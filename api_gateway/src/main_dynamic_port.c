/**
 * @file main_dynamic_port.c
 * @brief Versión del API Gateway que acepta puerto como parámetro
 * 
 * Esta versión permite ejecutar múltiples instancias del API Gateway
 * cada una escuchando en un puerto diferente para pruebas de carga.
 * 
 * Uso: ./api_gateway_dynamic [puerto_escucha]
 * Si no se especifica puerto, usa 5683 por defecto.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>

#include <coap3/coap.h>
#include <coap3/coap_address.h>
#include <coap3/coap_event.h>

#include "api_gateway/api_handlers.h"
#include "api_gateway/elevator_state_manager.h"
#include "api_gateway/can_bridge.h"
#include <cJSON.h>
#include "api_gateway/logging_gw.h"
#include "api_gateway/execution_logger.h"
#include "dotenv.h"

// Variables globales (igual que en main.c original)
volatile sig_atomic_t quit_main_loop = 0;
elevator_group_state_t managed_elevator_group;
coap_context_t *g_coap_context = NULL;
coap_session_t *g_dtls_session_to_central_server = NULL;
static coap_context_t *g_coap_context_for_session_mgnt = NULL;

// Puerto de escucha dinámico
static int dynamic_listen_port = 5683;

// Forward declarations
static void simulate_elevator_group_step(coap_context_t *ctx, elevator_group_state_t *group);
void inicializar_mi_simulacion_ascensor(void);
void simular_eventos_ascensor(void);

// Event handler (copiado del original)
static int event_handler_gw(coap_session_t *session, coap_event_t event) {
    if (event == COAP_EVENT_DTLS_CLOSED || event == COAP_EVENT_DTLS_ERROR || 
        event == COAP_EVENT_SESSION_CLOSED || event == COAP_EVENT_SESSION_FAILED) {
        
        LOG_WARN_GW("[EventHandlerGW] Evento DTLS/Sesión %d para sesión.", event);

        if (g_dtls_session_to_central_server && session == g_dtls_session_to_central_server) {
            LOG_INFO_GW("[EventHandlerGW] La sesión DTLS global con el servidor central se cerró. Se limpiará.");
            coap_session_release(g_dtls_session_to_central_server);
            g_dtls_session_to_central_server = NULL;
        }
    }
    return 0;
}

// Función para obtener sesión DTLS (copiada del original)
coap_session_t* get_or_create_central_server_dtls_session(coap_context_t *ctx) {
    if (!ctx) {
        LOG_ERROR_GW("[SessionHelper] Contexto CoAP es NULL.");
        return NULL;
    }

    // Si ya tenemos una sesión activa, verificar si sigue siendo válida
    if (g_dtls_session_to_central_server) {
        coap_session_state_t state = coap_session_get_state(g_dtls_session_to_central_server);
        if (state == COAP_SESSION_STATE_ESTABLISHED) {
            LOG_DEBUG_GW("[SessionHelper] Reutilizando sesión DTLS existente con servidor central.");
            return g_dtls_session_to_central_server;
        } else {
            LOG_INFO_GW("[SessionHelper] Sesión DTLS existente no está establecida (estado %d). Liberando y creando nueva.", state);
            coap_session_release(g_dtls_session_to_central_server);
            g_dtls_session_to_central_server = NULL;
        }
    }

    // Crear nueva sesión DTLS
    coap_address_t server_addr;
    coap_address_init(&server_addr);
    server_addr.addr.sin.sin_family = AF_INET;
    
    if (inet_pton(AF_INET, getenv("CENTRAL_SERVER_IP"), &server_addr.addr.sin.sin_addr) != 1) {
        LOG_ERROR_GW("[SessionHelper] Error convirtiendo IP del servidor central: %s", getenv("CENTRAL_SERVER_IP"));
        return NULL;
    }
    server_addr.addr.sin.sin_port = htons(atoi(getenv("CENTRAL_SERVER_PORT")));

    // Crear sesión DTLS con PSK
    g_dtls_session_to_central_server = coap_new_client_session_psk(
        ctx, NULL, &server_addr, COAP_PROTO_DTLS,
        getenv("IDENTITY_TO_PRESENT_TO_SERVER"),
        (const uint8_t*)getenv("KEY_FOR_SERVER"), strlen(getenv("KEY_FOR_SERVER"))
    );

    if (!g_dtls_session_to_central_server) {
        LOG_ERROR_GW("[SessionHelper] Error creando sesión DTLS con servidor central %s:%s", 
                     getenv("CENTRAL_SERVER_IP"), getenv("CENTRAL_SERVER_PORT"));
        return NULL;
    }

    LOG_INFO_GW("[SessionHelper] Nueva sesión DTLS creada con servidor central %s:%s", 
                getenv("CENTRAL_SERVER_IP"), getenv("CENTRAL_SERVER_PORT"));
    return g_dtls_session_to_central_server;
}

// Simulación (copiada del original)
static void simulate_elevator_group_step(coap_context_t *ctx, elevator_group_state_t *group) {
    if (!group || group->num_elevadores_en_grupo <= 0) {
        return;
    }

    for (int i = 0; i < group->num_elevadores_en_grupo; ++i) {
        elevator_status_t *elevator = &group->ascensores[i];

        if (elevator->ocupado && elevator->destino_actual != -1) {
            if (elevator->piso_actual != elevator->destino_actual) {
                // Cerrar puertas antes del movimiento
                if (elevator->estado_puerta_enum != DOOR_CLOSED) {
                    LOG_DEBUG_GW("[SimStep] Ascensor %s cerrando puertas en piso %d.", 
                                 elevator->ascensor_id, elevator->piso_actual);
                    elevator->estado_puerta_enum = DOOR_CLOSED;
                }

                // Determinar dirección
                if (elevator->direccion_movimiento_enum == STOPPED || 
                    elevator->direccion_movimiento_enum == DIRECTION_UNKNOWN) {
                    if (elevator->destino_actual > elevator->piso_actual) {
                        elevator->direccion_movimiento_enum = MOVING_UP;
                    } else if (elevator->destino_actual < elevator->piso_actual) {
                        elevator->direccion_movimiento_enum = MOVING_DOWN;
                    }
                }

                // Simular movimiento
                if (elevator->direccion_movimiento_enum == MOVING_UP) {
                    elevator->piso_actual++;
                    LOG_INFO_GW("[SimStep] Ascensor %s SUBE a piso %d (Destino: %d)", 
                                elevator->ascensor_id, elevator->piso_actual, elevator->destino_actual);
                } else if (elevator->direccion_movimiento_enum == MOVING_DOWN) {
                    elevator->piso_actual--;
                    LOG_INFO_GW("[SimStep] Ascensor %s BAJA a piso %d (Destino: %d)", 
                                elevator->ascensor_id, elevator->piso_actual, elevator->destino_actual);
                }

                // Verificar llegada
                if (elevator->piso_actual == elevator->destino_actual) {
                    LOG_INFO_GW("[SimStep] Ascensor %s LLEGÓ a destino %d.", 
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
                    
                    LOG_INFO_GW("[SimStep] Tarea completada por %s.", elevator->ascensor_id);
                }
            } else {
                // Ya en destino
                LOG_DEBUG_GW("[SimStep] Ascensor %s está ocupado y en su destino %d.", 
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
        }
    }
}

// Función principal modificada
int main(int argc, char *argv[]) {
    env_load("gateway.env", true); // Cargar variables de entorno desde gateway.env
    coap_context_t *ctx = NULL;
    coap_address_t listen_addr;
    int result;

    // Procesar argumentos de línea de comandos
    if (argc > 1) {
        dynamic_listen_port = atoi(argv[1]);
        if (dynamic_listen_port < 1024 || dynamic_listen_port > 65535) {
            fprintf(stderr, "Error: Puerto debe estar entre 1024 y 65535. Recibido: %d\n", dynamic_listen_port);
            return EXIT_FAILURE;
        }
        printf("API Gateway: Usando puerto dinámico %d\n", dynamic_listen_port);
    } else {
        printf("API Gateway: Usando puerto por defecto %d\n", dynamic_listen_port);
        printf("Uso: %s [puerto_escucha]\n", argv[0]);
    }

    // Registrar manejador de señales
    signal(SIGINT, handle_sigint_gw);
    srand(time(NULL));

    // Inicializar CoAP
    coap_startup();

    // Inicializar puente CAN
    ag_can_bridge_init();

    // Preparar dirección de escucha
    coap_address_init(&listen_addr);
    listen_addr.addr.sin.sin_family = AF_INET;
    
    if (inet_pton(AF_INET, getenv("GW_LISTEN_IP"), &listen_addr.addr.sin.sin_addr) != 1) {
        fprintf(stderr, "Error convirtiendo IP de escucha '%s': %s\n", getenv("GW_LISTEN_IP"), strerror(errno));
        coap_cleanup();
        return EXIT_FAILURE;
    }
    listen_addr.addr.sin.sin_port = htons(dynamic_listen_port);

    // Crear contexto CoAP
    ctx = coap_new_context(NULL);
    if (!ctx) {
        fprintf(stderr, "Error creando contexto CoAP.\n");
        coap_cleanup();
        return EXIT_FAILURE;
    }
    
    g_coap_context = ctx;
    g_coap_context_for_session_mgnt = ctx;
    coap_register_event_handler(ctx, event_handler_gw);
    coap_register_response_handler(ctx, hnd_central_server_response_gw);

    // Crear endpoint de escucha
    coap_endpoint_t *endpoint = coap_new_endpoint(ctx, &listen_addr, COAP_PROTO_UDP);
    if (!endpoint) {
        fprintf(stderr, "Error creando endpoint en puerto %d. ¿Puerto en uso?\n", dynamic_listen_port);
        coap_free_context(ctx);
        coap_cleanup();
        return EXIT_FAILURE;
    }

    printf("API Gateway: Escuchando en %s:%d para mensajes CoAP (UDP).\n", 
           getenv("GW_LISTEN_IP"), dynamic_listen_port);
    printf("(Ctrl+C para salir)\n");

    // Inicializar grupo de ascensores
    init_elevator_group(&managed_elevator_group, "E1", 4, 14);
    LOG_INFO_GW("API Gateway: Grupo de %d ascensores para edificio '%s' inicializado.", 
                managed_elevator_group.num_elevadores_en_grupo, 
                managed_elevator_group.edificio_id_str_grupo);

    // Inicializar simulación
    inicializar_mi_simulacion_ascensor();

    // Inicializar logging
    if (!exec_logger_init()) {
        LOG_WARN_GW("[Main] No se pudo inicializar el sistema de logging. Continuando sin logging.");
    }

    // Simular eventos
    simular_eventos_ascensor();

    // Bucle principal
    while (!quit_main_loop) {
        result = coap_io_process(ctx, 500);
        if (result < 0) {
            fprintf(stderr, "Error en coap_io_process. Cerrando.\n");
            break;
        }

        simulate_elevator_group_step(ctx, &managed_elevator_group);
    }

    printf("API Gateway: Cerrando...\n");
    
    // Limpieza
    exec_logger_finish();
    
    if (g_dtls_session_to_central_server) {
        LOG_INFO_GW("[Main] Liberando sesión DTLS global al salir.");
        coap_session_release(g_dtls_session_to_central_server);
        g_dtls_session_to_central_server = NULL;
    }
    
    coap_free_context(ctx);
    g_coap_context = NULL;
    g_coap_context_for_session_mgnt = NULL;
    coap_cleanup();

    return EXIT_SUCCESS;
} 