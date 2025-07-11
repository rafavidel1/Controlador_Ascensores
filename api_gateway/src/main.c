/**
 * @file main.c
 * @brief Punto de entrada principal del API Gateway del Sistema de Control de Ascensores
 * @author Sistema de Control de Ascensores
 * @date 2025
 * @version 2.0
 * 
 * Este archivo implementa el API Gateway que actúa como intermediario entre
 * los controladores CAN de ascensores y el servidor central. Sus funciones principales:
 * 
 * - **Inicialización CoAP**: Configuración del contexto CoAP con soporte DTLS
 * - **Gestión de Recursos**: Registro de endpoints CoAP para recibir solicitudes
 * - **Puente CAN-CoAP**: Transformación de mensajes CAN a solicitudes CoAP
 * - **Gestión de Estado**: Mantenimiento del estado local de ascensores
 * - **Comunicación Segura**: Establecimiento de sesiones DTLS con servidor central
 * - **Simulación**: Simulación de movimiento de ascensores para testing
 * - **Manejo de Señales**: Terminación elegante con SIGINT
 * 
 * El gateway procesa dos tipos principales de solicitudes:
 * - Llamadas de piso (floor calls) desde botones externos
 * - Solicitudes de cabina (cabin requests) desde interior del ascensor
 * 
 * @see api_handlers.h
 * @see elevator_state_manager.h
 * @see can_bridge.h
 * @see coap_config.h
 */

#include <stdio.h>    // For standard I/O (printf, fprintf)
#include <stdlib.h>   // For EXIT_FAILURE, EXIT_SUCCESS, atoi, srand, time
#include <string.h>   // For strerror (potentially)
#include <signal.h>   // For signal, sig_atomic_t
#include <errno.h>    // For errno (potentially)

// Network headers - ensure these are available on your system
#include <sys/socket.h> // For AF_INET
#include <netinet/in.h> // For sockaddr_in, htons
#include <arpa/inet.h>  // For inet_pton
#include <time.h>     // For time() used in srand()
#include <stdbool.h>  // For bool type (simulación no-bloqueante)

#include <coap3/coap.h> // Incluir este paraguas primero
#include <coap3/coap_address.h> // Para coap_address_to_str
#include <coap3/coap_event.h>   // Para coap_event_str

#include "api_gateway/api_handlers.h"      // Corrected path for Declarations for CoAP handlers
#include "api_gateway/elevator_state_manager.h" // <--- NUEVO INCLUDE

// NUEVA INCLUSIÓN PARA EL PUENTE CAN
#include "api_gateway/can_bridge.h"

// Include cJSON for payload generation
#include <cJSON.h> 

#include "api_gateway/logging_gw.h"
#include "dotenv.h"
#include <unistd.h> // Para getpid()
#include "psk_manager.h" // Gestor de claves PSK

// Función para generar identidad única para cada instancia
static char* generate_unique_identity() {
    static char identity[64];
    pid_t pid = getpid();
    time_t now = time(NULL);
    snprintf(identity, sizeof(identity), "Gateway_Client_%d_%ld", (int)pid, now);
    return identity;
}

// Función para generar clave PSK determinística basada en la identidad
static char* generate_unique_psk_key() {
    static char psk_key[128];
    
    // Generar identidad única para esta instancia
    char* unique_identity = generate_unique_identity();
    
    // Obtener clave determinística basada en la identidad
    // Usa el mismo algoritmo que el servidor central
    if (psk_manager_get_deterministic_key(unique_identity, psk_key, sizeof(psk_key)) == 0) {
        LOG_DEBUG_GW("[PSK] Clave determinística para identidad '%s': %s", unique_identity, psk_key);
        return psk_key;
    }
    
    // Si falla, intentar obtener clave aleatoria con reintentos
    int max_retries = 10;
    int retry_count = 0;
    
    while (retry_count < max_retries) {
        if (psk_manager_get_random_key(psk_key, sizeof(psk_key)) == 0) {
            LOG_DEBUG_GW("[PSK] Clave aleatoria seleccionada: %s", psk_key);
            return psk_key;
        }
        
        retry_count++;
        LOG_WARN_GW("[PSK] Error obteniendo clave aleatoria (intento %d/%d), reintentando...", retry_count, max_retries);
        
        // Pequeña pausa entre reintentos
        usleep(100000); // 100ms
    }
    
    // Si todos los reintentos fallan, usar la primera clave del archivo como fallback
    LOG_ERROR_GW("[PSK] Todos los reintentos fallaron. Usando primera clave del archivo como fallback.");
    if (psk_manager_get_first_key(psk_key, sizeof(psk_key)) == 0) {
        LOG_INFO_GW("[PSK] Usando primera clave como fallback: %s", psk_key);
        return psk_key;
    }
    
    // Último recurso: usar una clave hardcodeada que sabemos que está en la lista
    LOG_ERROR_GW("[PSK] Error crítico: no se pudo obtener ninguna clave PSK válida");
    strncpy(psk_key, "GatewayKey_00001", sizeof(psk_key) - 1);
    psk_key[sizeof(psk_key) - 1] = '\0';
    return psk_key;
}
#include "api_gateway/execution_logger.h" // Sistema de logging de ejecuciones

/**
 * @brief Bandera global para controlar el bucle principal de la aplicación
 * 
 * Esta bandera se establece a 1 por el manejador de señal SIGINT (handle_sigint_gw)
 * para indicar que la aplicación debe terminar de manera elegante.
 * 
 * @see handle_sigint_gw()
 */
volatile sig_atomic_t quit_main_loop = 0;

/**
 * @brief Estado global del grupo de ascensores gestionado por este gateway
 * 
 * Contiene el estado completo de todos los ascensores del edificio
 * gestionado por este API Gateway, incluyendo posiciones actuales,
 * tareas asignadas, estados de puertas y disponibilidad.
 * 
 * @see elevator_group_state_t
 * @see elevator_state_manager.h
 */
elevator_group_state_t managed_elevator_group;

// --- Forward declaration for the new simulation function ---
static void simulate_elevator_group_step(coap_context_t *ctx, elevator_group_state_t *group);
// --- End Forward declaration ---

// --- Prototipos para el simulador de ascensor (normalmente en mi_simulador_ascensor.h) ---
void inicializar_mi_simulacion_ascensor(void); // No necesita ctx si usamos g_coap_context
void simular_eventos_ascensor(void);
bool procesar_siguiente_peticion_simulacion(void); // Nueva función no-bloqueante
// --- Fin Prototipos simulador ---

/**
 * @brief Contexto CoAP global para la simulación y gestión de sesiones
 * 
 * Contexto principal utilizado para todas las operaciones CoAP,
 * incluyendo la simulación de ascensores y la gestión de sesiones DTLS.
 */
coap_context_t  *g_coap_context = NULL;

// --- Inicio: Gestión de Sesión DTLS Global para Servidor Central ---

/**
 * @brief Sesión DTLS global con el servidor central
 * 
 * Mantiene la conexión segura DTLS con el servidor central
 * para el envío de solicitudes de asignación de ascensores.
 * Se reutiliza para múltiples solicitudes para eficiencia.
 */
coap_session_t *g_dtls_session_to_central_server = NULL;

/**
 * @brief Contexto CoAP para la gestión de sesiones DTLS
 * 
 * Contexto específico utilizado para el manejo de eventos
 * de sesiones DTLS con el servidor central.
 */
static coap_context_t *g_coap_context_for_session_mgnt = NULL;

/**
 * @brief Manejador de eventos CoAP para sesiones DTLS
 * @param session Sesión CoAP que generó el evento
 * @param event Tipo de evento ocurrido
 * @return 0 si el evento se procesó correctamente
 * 
 * Esta función maneja eventos relacionados con sesiones DTLS,
 * especialmente cierres de conexión y errores. Limpia automáticamente
 * la sesión global cuando se detecta que se ha cerrado o ha fallado.
 * 
 * Eventos manejados:
 * - COAP_EVENT_DTLS_CLOSED: Sesión DTLS cerrada
 * - COAP_EVENT_DTLS_ERROR: Error en sesión DTLS
 * - COAP_EVENT_SESSION_CLOSED: Sesión cerrada
 * - COAP_EVENT_SESSION_FAILED: Sesión falló
 */
static int event_handler_gw(coap_session_t *session, coap_event_t event) {
    // Manejar eventos de establecimiento de sesión
    if (event == COAP_EVENT_DTLS_CONNECTED || event == COAP_EVENT_SESSION_CONNECTED) {
        if (g_dtls_session_to_central_server && session == g_dtls_session_to_central_server) {
            LOG_INFO_GW("[EventHandlerGW] Sesión DTLS global (0x%p) establecida exitosamente con servidor central.", (void*)g_dtls_session_to_central_server);
        } else {
            LOG_DEBUG_GW("[EventHandlerGW] Sesión DTLS (0x%p) establecida (no es la sesión global).", (void*)session);
        }
        return 0;
    }
    
    // Manejar eventos de cierre/error de sesión
    if (event == COAP_EVENT_DTLS_CLOSED || event == COAP_EVENT_DTLS_ERROR || 
        event == COAP_EVENT_SESSION_CLOSED || event == COAP_EVENT_SESSION_FAILED) {
        
        char addr_buffer[INET6_ADDRSTRLEN + 8] = "unknown_peer"; // Default
        const coap_address_t *remote_addr = coap_session_get_addr_remote(session);
        if (remote_addr) {
            // Intentar usar coap_address_to_str si está disponible, si no, omitir esta parte detallada.
            // Para compilar sin la función, comentamos su uso por ahora.
            /* size_t len = coap_address_to_str(addr_buffer, sizeof(addr_buffer), remote_addr);
            if (len == 0) { 
                strcpy(addr_buffer, "addr_error");
            } */
            // Si no podemos convertir la dirección a string fácilmente, al menos sabemos que la sesión terminó.
            // Podríamos intentar imprimir IP y puerto manualmente si fuera crítico.
            LOG_WARN_GW("[EventHandlerGW] Evento DTLS/Sesión %d para sesión. Verifique peer manualmente si es necesario.", event);
        } else {
            LOG_WARN_GW("[EventHandlerGW] Evento DTLS/Sesión %d para sesión con peer desconocido.", event);
        }

        // Comentamos el uso de coap_event_str también para asegurar la compilación
        /* const char* event_text = coap_event_str(event);
        LOG_WARN_GW("[EventHandlerGW] Evento %d (%s) para sesión con %s.", 
                    event, event_text ? event_text : "UNKNOWN_EVENT_STR", addr_buffer); */

        if (g_dtls_session_to_central_server && session == g_dtls_session_to_central_server) {
            LOG_INFO_GW("[EventHandlerGW] La sesión DTLS global con el servidor central (0x%p) se cerró o tuvo un error (evento %d). Se limpiará.", (void*)g_dtls_session_to_central_server, event);
            coap_session_release(g_dtls_session_to_central_server); // Liberar nuestra referencia
            g_dtls_session_to_central_server = NULL;
        } else if (session != g_dtls_session_to_central_server) {
            LOG_WARN_GW("[EventHandlerGW] Evento %d para una sesión DTLS NO global (0x%p).", event, (void*)session);
            // No gestionamos la liberación de sesiones no globales aquí, el dueño debe hacerlo.
        }
    }
    
    // Manejar otros eventos de sesión
    if (event == COAP_EVENT_DTLS_CONNECTED) {
        if (g_dtls_session_to_central_server && session == g_dtls_session_to_central_server) {
            LOG_DEBUG_GW("[EventHandlerGW] Sesión DTLS global (0x%p) conectando con servidor central...", (void*)g_dtls_session_to_central_server);
        }
    }
    
    return 0;
}

/**
 * @brief Obtiene o crea una sesión DTLS con el servidor central
 * @param ctx Contexto CoAP a utilizar para la sesión
 * @return Puntero a la sesión DTLS establecida, o NULL en caso de error
 * 
 * Esta función implementa un patrón singleton para la gestión de sesiones DTLS
 * con el servidor central. Reutiliza sesiones existentes cuando están activas
 * y crea nuevas sesiones cuando es necesario.
 * 
 * Comportamiento:
 * 1. Si existe una sesión activa y establecida, la reutiliza
 * 2. Si la sesión existe pero no está establecida, la libera y crea una nueva
 * 3. Si no existe sesión, crea una nueva con configuración DTLS-PSK
 * 4. Registra el manejador de eventos para gestión automática de la sesión
 * 
 * La función utiliza las siguientes configuraciones:
 * - IP del servidor: CENTRAL_SERVER_IP
 * - Puerto del servidor: CENTRAL_SERVER_PORT  
 * - Identidad PSK: IDENTITY_TO_PRESENT_TO_SERVER
 * - Clave PSK: KEY_FOR_SERVER
 * 
 * @see event_handler_gw()
 * @see coap_config.h
 */
coap_session_t* get_or_create_central_server_dtls_session(coap_context_t *ctx) {
    static int creating_session = 0; // Evitar crear múltiples sesiones simultáneamente
    
    if (!ctx) {
        LOG_ERROR_GW("[SessionHelper] Contexto CoAP es NULL. No se puede obtener/crear sesión.");
        return NULL;
    }
    
    // Si ya estamos creando una sesión, esperar
    if (creating_session) {
        LOG_DEBUG_GW("[SessionHelper] Ya se está creando una sesión. Esperando...");
        int wait_count = 0;
        while (creating_session && wait_count < 50) { // Esperar hasta 5 segundos
            coap_io_process(ctx, 100);
            wait_count++;
        }
        if (g_dtls_session_to_central_server && 
            coap_session_get_state(g_dtls_session_to_central_server) == COAP_SESSION_STATE_ESTABLISHED) {
            return g_dtls_session_to_central_server;
        }
    }

    // Verificar si ya tenemos una sesión establecida y válida
    if (g_dtls_session_to_central_server != NULL && 
        coap_session_get_state(g_dtls_session_to_central_server) == COAP_SESSION_STATE_ESTABLISHED) {
        LOG_DEBUG_GW("[SessionHelper] Reutilizando sesión DTLS-PSK establecida (0x%p) con servidor central.", (void*)g_dtls_session_to_central_server);
        return g_dtls_session_to_central_server;
    }

    // Si la sesión existe pero está en estado CONNECTING, esperar un poco
    if (g_dtls_session_to_central_server != NULL && 
        coap_session_get_state(g_dtls_session_to_central_server) == COAP_SESSION_STATE_CONNECTING) {
        LOG_INFO_GW("[SessionHelper] Sesión DTLS-PSK (0x%p) está conectando. Esperando establecimiento...", (void*)g_dtls_session_to_central_server);
        
        // Esperar hasta 5 segundos para que se establezca la sesión
        int max_wait_attempts = 50; // 50 * 100ms = 5 segundos
        int wait_count = 0;
        
        while (wait_count < max_wait_attempts) {
            coap_io_process(ctx, 100); // Procesar eventos por 100ms
            
            if (coap_session_get_state(g_dtls_session_to_central_server) == COAP_SESSION_STATE_ESTABLISHED) {
                LOG_INFO_GW("[SessionHelper] Sesión DTLS-PSK (0x%p) establecida exitosamente después de %d intentos.", 
                           (void*)g_dtls_session_to_central_server, wait_count);
                return g_dtls_session_to_central_server;
            } else if (coap_session_get_state(g_dtls_session_to_central_server) == COAP_SESSION_STATE_NONE) {
                LOG_WARN_GW("[SessionHelper] Sesión DTLS-PSK (0x%p) falló durante la conexión. Creando nueva sesión.", 
                           (void*)g_dtls_session_to_central_server);
                break;
            }
            
            wait_count++;
        }
        
        if (wait_count >= max_wait_attempts) {
            LOG_WARN_GW("[SessionHelper] Timeout esperando establecimiento de sesión DTLS-PSK (0x%p). Creando nueva sesión.", 
                       (void*)g_dtls_session_to_central_server);
        }
    }

    // Si la sesión existe pero no está establecida, liberarla antes de crear una nueva.
    if (g_dtls_session_to_central_server != NULL) {
        LOG_INFO_GW("[SessionHelper] Liberando sesión DTLS-PSK existente (0x%p) para crear una nueva.", (void*)g_dtls_session_to_central_server);
        coap_session_release(g_dtls_session_to_central_server);
        g_dtls_session_to_central_server = NULL;
    }

    LOG_INFO_GW("[SessionHelper] Creando NUEVA sesión DTLS-PSK con servidor central.");
    creating_session = 1; // Marcar que estamos creando una sesión
    coap_address_t central_server_addr;
    coap_address_init(&central_server_addr);
    central_server_addr.addr.sin.sin_family = AF_INET;
    
    // Obtener y limpiar la IP del servidor central
    const char* server_ip_raw = getenv("CENTRAL_SERVER_IP") ?: "192.168.49.2";
    char server_ip[16];
    strncpy(server_ip, server_ip_raw, sizeof(server_ip) - 1);
    server_ip[sizeof(server_ip) - 1] = '\0';
    
    // Remover caracteres de nueva línea
    char *newline_server = strchr(server_ip, '\n');
    if (newline_server) *newline_server = '\0';
    newline_server = strchr(server_ip, '\r');
    if (newline_server) *newline_server = '\0';
    
    LOG_INFO_GW("[SessionHelper] Debug - server_ip = '%s' (longitud: %d)", server_ip, (int)strlen(server_ip));
    LOG_INFO_GW("[SessionHelper] Debug - CENTRAL_SERVER_IP env var = '%s'", getenv("CENTRAL_SERVER_IP") ?: "NULL");
    
    if (inet_pton(AF_INET, server_ip, &central_server_addr.addr.sin.sin_addr) <= 0) {
        LOG_ERROR_GW("[SessionHelper] Error convirtiendo IP del servidor central: %s", server_ip);
        return NULL;
    }
    
    // Obtener y limpiar el puerto del servidor central
    const char* server_port_raw = getenv("CENTRAL_SERVER_PORT") ?: "5684";
    char server_port_str[8];
    strncpy(server_port_str, server_port_raw, sizeof(server_port_str) - 1);
    server_port_str[sizeof(server_port_str) - 1] = '\0';
    
    // Remover caracteres de nueva línea
    char *newline_server_port = strchr(server_port_str, '\n');
    if (newline_server_port) *newline_server_port = '\0';
    newline_server_port = strchr(server_port_str, '\r');
    if (newline_server_port) *newline_server_port = '\0';
    
    central_server_addr.addr.sin.sin_port = htons(atoi(server_port_str));

    // Generar identidad y clave únicas para esta instancia
    char* unique_identity = generate_unique_identity();
    char* unique_psk_key = generate_unique_psk_key();
    
    // Establecer variables de entorno únicas para esta instancia
    setenv("IDENTITY_TO_PRESENT_TO_SERVER", unique_identity, 1);
    setenv("KEY_FOR_SERVER", unique_psk_key, 1);
    
    LOG_INFO_GW("[SessionHelper] Usando identidad única: '%s'", unique_identity);
    
    g_dtls_session_to_central_server = coap_new_client_session_psk(ctx,
                                                                   NULL, // local_if
                                                                   &central_server_addr,
                                                                   COAP_PROTO_DTLS,
                                                                   unique_identity,
                                                                   (const uint8_t *)unique_psk_key,
                                                                   strlen(unique_psk_key));

    if (!g_dtls_session_to_central_server) {
        LOG_ERROR_GW("[SessionHelper] Error creando NUEVA sesión DTLS-PSK con servidor central. Identity: '%s'", unique_identity);
        return NULL;
    }
    
    LOG_INFO_GW("[SessionHelper] NUEVA Sesión DTLS-PSK (0x%p) creada con servidor central. Identity: '%s'", (void*)g_dtls_session_to_central_server, unique_identity);
    coap_session_reference(g_dtls_session_to_central_server); // Tomamos una referencia explícita
    
    // Guardar el contexto para el event handler (si no lo hemos hecho ya o si cambia)
    if (g_coap_context_for_session_mgnt != ctx) {
        g_coap_context_for_session_mgnt = ctx;
        coap_register_event_handler(g_coap_context_for_session_mgnt, event_handler_gw);
        LOG_DEBUG_GW("[SessionHelper] Manejador de eventos CoAP registrado para la gestión de sesiones DTLS.");
    }

    // Esperar a que la nueva sesión se establezca
    LOG_INFO_GW("[SessionHelper] Esperando establecimiento de nueva sesión DTLS-PSK (0x%p)...", (void*)g_dtls_session_to_central_server);
    
    int max_wait_attempts = 50; // 50 * 100ms = 5 segundos
    int wait_count = 0;
    
    while (wait_count < max_wait_attempts) {
        coap_io_process(ctx, 100); // Procesar eventos por 100ms
        
        if (coap_session_get_state(g_dtls_session_to_central_server) == COAP_SESSION_STATE_ESTABLISHED) {
            LOG_INFO_GW("[SessionHelper] Nueva sesión DTLS-PSK (0x%p) establecida exitosamente después de %d intentos.", 
                       (void*)g_dtls_session_to_central_server, wait_count);
            creating_session = 0; // Desmarcar que estamos creando una sesión
            return g_dtls_session_to_central_server;
        } else if (coap_session_get_state(g_dtls_session_to_central_server) == COAP_SESSION_STATE_NONE) {
            LOG_ERROR_GW("[SessionHelper] Nueva sesión DTLS-PSK (0x%p) falló durante la conexión.", 
                       (void*)g_dtls_session_to_central_server);
            coap_session_release(g_dtls_session_to_central_server);
            g_dtls_session_to_central_server = NULL;
            creating_session = 0; // Desmarcar que estamos creando una sesión
            return NULL;
        }
        
        wait_count++;
    }
    
    LOG_ERROR_GW("[SessionHelper] Timeout esperando establecimiento de nueva sesión DTLS-PSK (0x%p).", 
               (void*)g_dtls_session_to_central_server);
    coap_session_release(g_dtls_session_to_central_server);
    g_dtls_session_to_central_server = NULL;
    creating_session = 0; // Desmarcar que estamos creando una sesión
    return NULL;
}
// --- Fin: Gestión de Sesión DTLS Global para Servidor Central ---

/**
 * @brief Simula un paso de movimiento para todos los ascensores del grupo
 * @param ctx Contexto CoAP para logging y operaciones
 * @param group Grupo de ascensores a simular
 * 
 * Esta función implementa la lógica de simulación de movimiento de ascensores.
 * Se ejecuta periódicamente para actualizar el estado de todos los ascensores
 * que tienen tareas asignadas.
 * 
 * Funcionalidades implementadas:
 * - **Cierre de puertas**: Cierra puertas antes del movimiento
 * - **Determinación de dirección**: Calcula dirección basada en destino
 * - **Simulación de movimiento**: Mueve ascensores piso por piso
 * - **Detección de llegada**: Detecta cuando un ascensor llega a su destino
 * - **Completado de tareas**: Notifica llegadas y libera ascensores
 * 
 * La simulación maneja los siguientes estados:
 * - Ascensores ocupados con destino válido
 * - Movimiento hacia arriba (MOVING_UP)
 * - Movimiento hacia abajo (MOVING_DOWN)
 * - Llegada a destino y liberación
 * 
 * @see elevator_group_state_t
 * @see elevator_status_t
 */
static void 
simulate_elevator_group_step(coap_context_t *ctx, elevator_group_state_t *group) {
    // Agregar logging detallado para diagnóstico
    static int debug_counter = 0;
    debug_counter++;
    
    // Log cada 10 iteraciones para no saturar los logs
    if (debug_counter % 10 == 0) {
        LOG_DEBUG_GW("[SimStep] === DIAGNÓSTICO SIMULADOR === Iteración %d", debug_counter);
        LOG_DEBUG_GW("[SimStep] Grupo: %s, Ascensores: %d", 
                    group ? group->edificio_id_str_grupo : "NULL",
                    group ? group->num_elevadores_en_grupo : 0);
    }
    
    if (!ctx || !group) {
        LOG_ERROR_GW("[SimStep] Error: ctx o group es NULL.");
        return;
    }

    int ascensores_ocupados = 0;
    int ascensores_moviendo = 0;
    
    for (int i = 0; i < group->num_elevadores_en_grupo; ++i) {
        elevator_status_t *elevator = &group->ascensores[i];
        
        // Contar ascensores ocupados
        if (elevator->ocupado) {
            ascensores_ocupados++;
        }
        
        // Log detallado cada 10 iteraciones
        if (debug_counter % 10 == 0) {
            LOG_DEBUG_GW("[SimStep] Ascensor %s: Piso=%d, Destino=%d, Ocupado=%s, Tarea=%s", 
                        elevator->ascensor_id,
                        elevator->piso_actual,
                        elevator->destino_actual,
                        elevator->ocupado ? "SÍ" : "NO",
                        elevator->tarea_actual_id[0] != '\0' ? elevator->tarea_actual_id : "NINGUNA");
        }

        if (elevator->ocupado && elevator->destino_actual != -1) {
            ascensores_moviendo++;
            
            if (elevator->piso_actual != elevator->destino_actual) {
                // Simulate door closing if it was open before movement
                if (elevator->estado_puerta_enum != DOOR_CLOSED) {
                    LOG_DEBUG_GW("[SimStep] Ascensor %s cerrando puertas en piso %d para moverse.", elevator->ascensor_id, elevator->piso_actual);
                    elevator->estado_puerta_enum = DOOR_CLOSED; 
                    // In a more complex sim, add DOOR_CLOSING state and delay
                }

                // Determine direction if somehow not set (should be set by assign_task)
                if (elevator->direccion_movimiento_enum == STOPPED || elevator->direccion_movimiento_enum == DIRECTION_UNKNOWN) {
                    if (elevator->destino_actual > elevator->piso_actual) {
                        elevator->direccion_movimiento_enum = MOVING_UP;
                    } else if (elevator->destino_actual < elevator->piso_actual) {
                        elevator->direccion_movimiento_enum = MOVING_DOWN;
                    }
                }

                // Simulate movement
                if (elevator->direccion_movimiento_enum == MOVING_UP) {
                    elevator->piso_actual++;
                    LOG_INFO_GW("[SimStep] Ascensor %s SUBE a piso %d (Destino: %d, Tarea: %s)", 
                                elevator->ascensor_id, elevator->piso_actual, elevator->destino_actual, elevator->tarea_actual_id);
                } else if (elevator->direccion_movimiento_enum == MOVING_DOWN) {
                    elevator->piso_actual--;
                    LOG_INFO_GW("[SimStep] Ascensor %s BAJA a piso %d (Destino: %d, Tarea: %s)", 
                                elevator->ascensor_id, elevator->piso_actual, elevator->destino_actual, elevator->tarea_actual_id);
                } else {
                     // Should not happen if moving, but good to log
                    LOG_WARN_GW("[SimStep] Ascensor %s ocupado con destino %d pero dirección %s. No se mueve.", 
                                elevator->ascensor_id, elevator->destino_actual, movement_direction_to_string(elevator->direccion_movimiento_enum));
                }

                // Check for arrival
                if (elevator->piso_actual == elevator->destino_actual) {
                    LOG_INFO_GW("[SimStep] Ascensor %s LLEGÓ a destino %d.", elevator->ascensor_id, elevator->destino_actual);
                    
                    LOG_INFO_GW("StateMgr: Ascensor %s completó tarea %s en piso %d.", 
                                elevator->ascensor_id, 
                                elevator->tarea_actual_id[0] != '\0' ? elevator->tarea_actual_id : "N/A", 
                                elevator->piso_actual);
                    
                    // Registrar completación de tarea en el logger
                    if (elevator->tarea_actual_id[0] != '\0') {
                        exec_logger_log_task_completed(elevator->tarea_actual_id, elevator->ascensor_id, elevator->piso_actual);
                    }
                    
                    elevator->estado_puerta_enum = DOOR_OPEN;
                    elevator->ocupado = false;
                    elevator->tarea_actual_id[0] = '\0'; // Limpiar ID de tarea
                    elevator->destino_actual = -1;        // Limpiar destino
                    elevator->direccion_movimiento_enum = STOPPED;
                    
                    LOG_INFO_GW("[SimStep] Tarea completada por %s.", elevator->ascensor_id);
                }
            } else { // Already at destination, but still marked occupied? 
                     // This could happen if a task was to the current floor.
                LOG_DEBUG_GW("[SimStep] Ascensor %s está ocupado y en su destino %d. Verificando si la tarea debe completarse.", 
                             elevator->ascensor_id, elevator->destino_actual);
                
                LOG_INFO_GW("StateMgr: Ascensor %s completó tarea %s en piso %d.", 
                            elevator->ascensor_id, 
                            elevator->tarea_actual_id[0] != '\0' ? elevator->tarea_actual_id : "N/A", 
                            elevator->piso_actual);
                
                // Registrar completación de tarea en el logger
                if (elevator->tarea_actual_id[0] != '\0') {
                    exec_logger_log_task_completed(elevator->tarea_actual_id, elevator->ascensor_id, elevator->piso_actual);
                }
                
                elevator->estado_puerta_enum = DOOR_OPEN;
                elevator->ocupado = false;
                elevator->tarea_actual_id[0] = '\0'; // Limpiar ID de tarea
                elevator->destino_actual = -1;        // Limpiar destino
                elevator->direccion_movimiento_enum = STOPPED;
                
                LOG_INFO_GW("[SimStep] Tarea completada por %s (estaba en destino).", elevator->ascensor_id);
            }
        } // end if (elevator->ocupado && elevator->destino_actual != -1)
    } // end for each elevator
    
    // Log estadísticas cada 10 iteraciones
    if (debug_counter % 10 == 0) {
        LOG_DEBUG_GW("[SimStep] ESTADÍSTICAS: Ocupados=%d, Moviendo=%d, Total=%d", 
                    ascensores_ocupados, ascensores_moviendo, group->num_elevadores_en_grupo);
    }
}

/**
 * @brief Main function for the API Gateway.
 *
 * Initializes libcoap, sets up the CoAP server endpoint for listening to elevator clients,
 * registers CoAP resource handlers, and enters the main I/O processing loop.
 * The gateway listens for client requests, forwards them to a central server, and
 * relays responses back to the clients.
 *
 * @param argc Número de argumentos de línea de comandos
 * @param argv Array de argumentos de línea de comandos
 *             argv[1] (opcional): Puerto de escucha (por defecto usa GW_LISTEN_PORT)
 *
 * @return EXIT_SUCCESS on successful execution and shutdown, EXIT_FAILURE on error.
 */
int main(int argc, char *argv[]) {
    printf("API Gateway: Intentando cargar gateway.env...\n");
    if (env_load("gateway.env", true) != 0) {
        printf("API Gateway: Error cargando gateway.env\n");
    } else {
        printf("API Gateway: gateway.env cargado exitosamente\n");
    }
    coap_context_t  *ctx = NULL;      // CoAP context
    coap_address_t   listen_addr;    // Address for the gateway to listen on
    int result;                       // Result of CoAP I/O operations
    // Obtener y limpiar el puerto
    const char* port_raw = getenv("GW_LISTEN_PORT") ?: "5683";
    char port_str[8];
    strncpy(port_str, port_raw, sizeof(port_str) - 1);
    port_str[sizeof(port_str) - 1] = '\0';
    
    // Remover caracteres de nueva línea
    char *newline_port = strchr(port_str, '\n');
    if (newline_port) *newline_port = '\0';
    newline_port = strchr(port_str, '\r');
    if (newline_port) *newline_port = '\0';
    
    int listen_port = atoi(port_str); // Puerto por defecto
    
    // Debug: mostrar valores cargados
    printf("API Gateway: GW_LISTEN_IP = '%s'\n", getenv("GW_LISTEN_IP") ?: "NULL");
    printf("API Gateway: GW_LISTEN_PORT = '%s'\n", getenv("GW_LISTEN_PORT") ?: "NULL");

    // Procesar argumentos de línea de comandos
    if (argc > 1) {
        int custom_port = atoi(argv[1]);
        if (custom_port >= 1024 && custom_port <= 65535) {
            listen_port = custom_port;
            printf("API Gateway: Usando puerto personalizado %d\n", listen_port);
        } else {
            fprintf(stderr, "Error: Puerto debe estar entre 1024 y 65535. Recibido: %s\n", argv[1]);
            printf("Uso: %s [puerto_escucha]\n", argv[0]);
            return EXIT_FAILURE;
        }
    } else {
        printf("API Gateway: Usando puerto por defecto %d\n", listen_port);
        printf("Uso: %s [puerto_escucha] (opcional)\n", argv[0]);
    }

    // Register the signal handler for SIGINT (Ctrl+C) for graceful shutdown.
    // Uses handle_sigint_gw from api_handlers.c
    signal(SIGINT, handle_sigint_gw); 

    // Seed the random number generator (if any CoAP operations rely on randomness, e.g. MIDs/Tokens if not handled by libcoap)
    srand(time(NULL)); 

    // Initialize the CoAP library stack.
    coap_startup();

    // INICIALIZAR EL PUENTE CAN SIMULADO
    ag_can_bridge_init();
    // NOTA: Tu simulación de ascensor C deberá llamar a 
    // ag_can_bridge_register_send_callback(tu_funcion_callback_can);
    // en algún momento después de esto y antes de enviar datos.

    // Prepare the CoAP address for the API Gateway to listen on.
    coap_address_init(&listen_addr);
    listen_addr.addr.sin.sin_family = AF_INET; // IPv4
    
    // Convert the listen IP string (from coap_config.h) to a network address.
    const char* listen_ip_raw = getenv("GW_LISTEN_IP") ?: "0.0.0.0";
    
    // Limpiar caracteres de nueva línea del valor
    char listen_ip[16];
    strncpy(listen_ip, listen_ip_raw, sizeof(listen_ip) - 1);
    listen_ip[sizeof(listen_ip) - 1] = '\0';
    
    // Remover caracteres de nueva línea
    char *newline = strchr(listen_ip, '\n');
    if (newline) *newline = '\0';
    newline = strchr(listen_ip, '\r');
    if (newline) *newline = '\0';
    

    
    if (inet_pton(AF_INET, listen_ip, &listen_addr.addr.sin.sin_addr) != 1) {
        fprintf(stderr, "API Gateway: Error converting listen IP address '%s'. Check GW_LISTEN_IP in gateway.env. Error: %s\n", listen_ip, strerror(errno));
        coap_cleanup(); // Cleanup libcoap before exiting
        return EXIT_FAILURE;
    }
    // Set the listen port (usar el puerto personalizado o por defecto)
    listen_addr.addr.sin.sin_port = htons(listen_port);

    // Create a new CoAP context.
    // The context is the main container for all CoAP activities.
    ctx = coap_new_context(NULL); // Passing NULL uses default CoAP I/O settings
    if (!ctx) {
        fprintf(stderr, "API Gateway: Error creating CoAP context.\n");
        coap_cleanup();
        return EXIT_FAILURE;
    }
    g_coap_context = ctx; // Asignar al global para que el simulador lo use
    g_coap_context_for_session_mgnt = ctx; // Asignar al inicio
    coap_register_event_handler(ctx, event_handler_gw); // Registrar el manejador de eventos para el contexto principal
    LOG_DEBUG_GW("[Main] Manejador de eventos CoAP global registrado.");

    // Register the global response handler for client requests made BY THIS API Gateway.
    // This handler (hnd_central_server_response_gw) will process responses from the Central Server.
    coap_register_response_handler(ctx, hnd_central_server_response_gw);

    // Create the CoAP server endpoint where the API Gateway will listen for client requests.
    coap_endpoint_t *endpoint = coap_new_endpoint(ctx, &listen_addr, COAP_PROTO_UDP);
    if (!endpoint) {
        fprintf(stderr, "API Gateway: Error creating listen endpoint on port %d. Is the address/port already in use?\n", listen_port);
        coap_free_context(ctx); // Free the context before cleaning up libcoap
        coap_cleanup();
        return EXIT_FAILURE;
    }

    printf("API Gateway: Listening on %s:%d for CoAP messages (UDP).\n"            
           "(Ctrl+C to quit)\n", 
           listen_ip, listen_port);

    // Inicializar el estado del grupo de ascensores
    // NOTA: La inicialización se hará desde la simulación JSON, no aquí
    // Por ejemplo, Edificio E1 con 4 ascensores y 14 plantas
    // Los IDs de los ascensores serán E1A1, E1A2, E1A3, E1A4
    init_elevator_group(&managed_elevator_group, "E1", 4, 14); 
    LOG_INFO_GW("API Gateway: Grupo de %d ascensores para edificio '%s' inicializado.", 
                managed_elevator_group.num_elevadores_en_grupo, 
                managed_elevator_group.edificio_id_str_grupo);

    // Inicializar gestor de claves PSK
    if (psk_manager_init("psk_keys.txt") != 0) {
        LOG_WARN_GW("[Main] No se pudo inicializar el gestor de claves PSK. Continuando con clave fija.");
    } else {
        LOG_INFO_GW("[Main] Gestor de claves PSK inicializado correctamente.");
    }

    // Inicialización de la simulación de ascensor (registrará su callback CAN)
    inicializar_mi_simulacion_ascensor();

    // Inicializar sistema de logging de ejecuciones
    if (!exec_logger_init()) {
        LOG_WARN_GW("[Main] No se pudo inicializar el sistema de logging de ejecuciones. Continuando sin logging.");
    }

    // Simular algunos eventos de ascensor una vez que todo está listo
    simular_eventos_ascensor();

    // Main I/O processing loop.
    // This loop continuously processes incoming CoAP messages and handles I/O operations.
    // It will run until quit_main_loop is set by the signal handler.
    while (!quit_main_loop) {
        // Process CoAP I/O for up to 100 milliseconds (0.1 seconds).
        // This function handles sending, receiving, and retransmissions.
        // Reducido de 500ms a 100ms para permitir simulación más frecuente
        result = coap_io_process(ctx, 100); 
        if (result < 0) {
            fprintf(stderr, "API Gateway: Error in coap_io_process. Shutting down.\n");
            break; // Exit loop on critical error
        }
        // result >= 0: milliseconds spent in I/O, or 0 if no I/O was ready within the timeout.

        // --- Procesar siguiente petición de simulación no-bloqueante ---
        procesar_siguiente_peticion_simulacion();

        // --- Simulate elevator group step (movimiento ascensor) ---
        simulate_elevator_group_step(ctx, &managed_elevator_group);

        // --- Procesamiento de frames CAN simulados (si los hubiera encolados o por sondeo) ---
        // Si tu simulación C llama directamente a ag_can_bridge_process_incoming_frame(),
        // no necesitas un sondeo explícito aquí a menos que tengas un buffer intermedio.
        // Por ejemplo, si tu simulación C se ejecuta en un hilo separado y encola frames:
        // process_any_queued_can_frames(ctx);
    }

    printf("API Gateway: Shutting down...\n");
    
    // Finalizar gestor de claves PSK
    psk_manager_cleanup();
    
    // Finalizar sistema de logging de ejecuciones
    exec_logger_finish();
    
    if (g_dtls_session_to_central_server) {
        LOG_INFO_GW("[Main] Liberando sesión DTLS global con servidor central (0x%p) al salir.", (void*)g_dtls_session_to_central_server);
        coap_session_release(g_dtls_session_to_central_server);
        g_dtls_session_to_central_server = NULL;
    }
    coap_free_context(ctx); // Release all resources associated with the CoAP context.
    g_coap_context = NULL;
    g_coap_context_for_session_mgnt = NULL;
    coap_cleanup();         // Release global CoAP library resources.

    return EXIT_SUCCESS;
} 