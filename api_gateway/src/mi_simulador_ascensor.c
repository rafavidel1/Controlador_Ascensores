/**
 * @file mi_simulador_ascensor.c
 * @brief Simulador de Ascensores para Testing del API Gateway
 * @author Sistema de Control de Ascensores
 * @date 2025
 * @version 2.0
 * 
 * Este archivo implementa un simulador de ascensores que genera eventos CAN
 * simulados para probar la funcionalidad del API Gateway. Sus funciones principales:
 * 
 * - **Simulación de eventos CAN**: Generación de frames CAN simulados
 * - **Callback de respuesta**: Procesamiento de respuestas del gateway
 * - **Integración con puente CAN**: Interfaz con el sistema CAN-CoAP
 * - **Testing automatizado**: Secuencias de prueba predefinidas
 * - **Carga desde JSON**: Sistema de simulación basado en archivos de datos
 * 
 * **Tipos de eventos simulados:**
 * - Llamadas de piso (floor calls) desde botones externos
 * - Solicitudes de cabina (cabin requests) desde interior del ascensor
 * - Notificaciones de llegada a destinos
 * 
 * El simulador utiliza el contexto CoAP global del API Gateway para
 * procesar eventos y recibir respuestas del servidor central.
 * 
 * **Sistema de simulación JSON:**
 * Cada ejecución del API Gateway carga un archivo JSON con 100 edificios,
 * selecciona uno aleatoriamente y ejecuta sus 10 peticiones secuencialmente.
 * 
 * @see can_bridge.h
 * @see elevator_state_manager.h
 * @see simulation_loader.h
 * @see api_common_defs.h
 */
#include "api_gateway/can_bridge.h"
#include "api_gateway/elevator_state_manager.h"
#include "api_gateway/simulation_loader.h"
#include "api_gateway/execution_logger.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <time.h> // Para manejo de tiempo en simulación no-bloqueante
#include <stdbool.h> // Para manejo de booleanos en simulación no-bloqueante

/**
 * @brief Contexto CoAP global del API Gateway
 * 
 * Referencia externa al contexto CoAP principal definido en main.c
 * del API Gateway. Utilizado para procesar eventos CoAP y enviar
 * solicitudes al servidor central.
 * 
 * @see api_gateway/main.c
 */
extern coap_context_t *g_coap_context;

/**
 * @brief Grupo de ascensores gestionado
 * 
 * Referencia externa al grupo de ascensores principal definido en
 * main.c. Se utiliza para configurar el ID del edificio durante
 * la simulación.
 * 
 * @see elevator_group_state_t
 */
extern elevator_group_state_t managed_elevator_group;

/**
 * @brief Datos de simulación cargados desde JSON
 * 
 * Variable global que almacena todos los edificios y peticiones
 * cargados desde el archivo de simulación JSON.
 */
static datos_simulacion_t datos_simulacion_global;

// Variables globales para manejo de simulación no-bloqueante
static bool simulacion_activa = false;
static int peticion_actual_index = 0;
static edificio_simulacion_t *edificio_actual = NULL;
static time_t tiempo_ultima_peticion = 0;
static const int INTERVALO_PETICIONES_MS = 2000; // 2 segundos entre peticiones

/**
 * @brief Callback para recibir frames CAN de respuesta del gateway
 * @param frame Puntero al frame CAN simulado recibido del gateway
 * 
 * Esta función actúa como callback registrado en el puente CAN para
 * recibir y procesar las respuestas del API Gateway. Interpreta
 * diferentes tipos de frames CAN de respuesta:
 * 
 * **Tipos de frames procesados:**
 * - 0x101: Respuesta a llamada de piso (0x100)
 * - 0x201: Respuesta a solicitud de cabina (0x200)
 * - 0xFE: Error genérico del gateway
 * 
 * **Información extraída:**
 * - Índice del ascensor asignado
 * - ID de tarea (parcial, limitado por tamaño CAN)
 * - Códigos de error y diagnóstico
 * 
 * La función proporciona logging detallado para debugging y
 * verificación del comportamiento del sistema.
 * 
 * @see ag_can_bridge_register_send_callback()
 * @see simulated_can_frame_t
 */
void mi_simulador_recibe_can_gw(simulated_can_frame_t* frame) {
    if (!frame) return;

    printf("[SIM_ASCENSOR] Recibido frame CAN de GW: ID=0x%X, DLC=%d, Datos: %02X %02X %02X %02X %02X %02X %02X %02X\n",
           frame->id,
           frame->dlc,
           frame->dlc > 0 ? frame->data[0] : 0,
           frame->dlc > 1 ? frame->data[1] : 0,
           frame->dlc > 2 ? frame->data[2] : 0,
           frame->dlc > 3 ? frame->data[3] : 0,
           frame->dlc > 4 ? frame->data[4] : 0,
           frame->dlc > 5 ? frame->data[5] : 0,
           frame->dlc > 6 ? frame->data[6] : 0,
           frame->dlc > 7 ? frame->data[7] : 0);

    // Registrar frame CAN recibido en el logger
    char description[128];
    if (frame->id == 0x101) {
        snprintf(description, sizeof(description), "Respuesta de llamada de piso del Gateway");
    } else if (frame->id == 0x201) {
        snprintf(description, sizeof(description), "Respuesta de solicitud de cabina del Gateway");
    } else if (frame->id == 0xFE) {
        snprintf(description, sizeof(description), "Error reportado por el Gateway");
    } else {
        snprintf(description, sizeof(description), "Frame de respuesta desconocido del Gateway");
    }
    exec_logger_log_can_received(frame->id, frame->dlc, frame->data, description);

    // Interpretar el frame CAN de respuesta (ejemplo básico)
    if (frame->id == 0x101) { // Respuesta a llamada de piso (0x100)
        if (frame->dlc >= 1) {
            int ascensor_idx_asignado = frame->data[0]; // Asume que el byte 0 es el índice del ascensor
            printf("    Simulador -> Respuesta de llamada de piso: Ascensor (índice %d) asignado.\n", ascensor_idx_asignado);
            if (frame->dlc > 1) {
                char tarea_id_parcial[8] = {0};
                int len_tarea = (frame->dlc - 1 < 7) ? (frame->dlc - 1) : 7;
                memcpy(tarea_id_parcial, &frame->data[1], len_tarea);
                printf("    Simulador -> Tarea ID (parcial): %s\n", tarea_id_parcial);
            }
        }
    } else if (frame->id == 0x201) { // Respuesta a solicitud de cabina (0x200)
         if (frame->dlc >= 1) {
            // Asumimos que el primer byte podría ser confirmación o un dato
            // y los siguientes el ID de tarea.
            printf("    Simulador -> Respuesta de solicitud de cabina: Datos[0]=%02X\n", frame->data[0]);
            if (frame->dlc > 1) {
                char tarea_id_parcial[8] = {0};
                int len_tarea = (frame->dlc - 1 < 7) ? (frame->dlc - 1) : 7;
                memcpy(tarea_id_parcial, &frame->data[1], len_tarea);
                printf("    Simulador -> Tarea ID (parcial): %s\n", tarea_id_parcial);
            }
        }
    } else if (frame->id == 0xFE) { // Error genérico de GW
        printf("    Simulador -> GW reportó un error. CAN ID Original (LSB): 0x%02X, Código Error GW: 0x%02X\n", frame->data[0], frame->data[1]);
    } else {
        printf("    Simulador -> ID de frame CAN de respuesta desconocido: 0x%X\n", frame->id);
    }
}

/**
 * @brief Inicializa el simulador de ascensores
 * 
 * Esta función configura el simulador registrando el callback de respuesta
 * CAN en el puente CAN-CoAP del API Gateway y cargando los datos de simulación
 * desde el archivo JSON.
 * 
 * **Operaciones realizadas:**
 * - Registro del callback de respuesta CAN
 * - Carga de datos de simulación desde JSON
 * - Inicialización de estructuras internas
 * - Configuración de logging del simulador
 * 
 * @see ag_can_bridge_register_send_callback()
 * @see cargar_datos_simulacion()
 * @see mi_simulador_recibe_can_gw()
 */
void inicializar_mi_simulacion_ascensor(void) {
    ag_can_bridge_register_send_callback(mi_simulador_recibe_can_gw);
    printf("[SIM_ASCENSOR] Simulador de ascensor inicializado y callback CAN registrado.\n");

    // Cargar datos de simulación desde JSON
    const char *archivo_simulacion = "simulation_data.json";
    printf("[SIM_ASCENSOR] Intentando cargar datos de simulación desde: %s\n", archivo_simulacion);
    
    if (cargar_datos_simulacion(archivo_simulacion, &datos_simulacion_global)) {
        printf("[SIM_ASCENSOR] Datos de simulación cargados exitosamente desde %s\n", archivo_simulacion);
        printf("[SIM_ASCENSOR] Edificios cargados: %d, Datos válidos: %s\n", 
               datos_simulacion_global.num_edificios, 
               datos_simulacion_global.datos_cargados ? "Sí" : "No");
    } else {
        printf("[SIM_ASCENSOR] Advertencia: No se pudieron cargar datos de simulación. Usando simulación básica.\n");
    }
    
    printf("\n");
}

/**
 * @brief Simula una llamada de piso vía CAN
 * @param piso_origen Piso desde el cual se realiza la llamada
 * @param direccion Dirección solicitada (MOVING_UP o MOVING_DOWN)
 * 
 * Esta función genera un frame CAN simulado que representa una llamada
 * de piso desde un botón externo. El frame se envía al puente CAN-CoAP
 * del API Gateway para su procesamiento.
 * 
 * **Formato del frame CAN:**
 * - ID: 0x100 (identificador para llamadas de piso)
 * - data[0]: Piso origen (0-255)
 * - data[1]: Dirección (0=UP, 1=DOWN)
 * - DLC: 2 bytes
 * 
 * **Validaciones:**
 * - Verifica disponibilidad del contexto CoAP
 * - Valida parámetros de entrada
 * 
 * @see ag_can_bridge_process_incoming_frame()
 * @see movement_direction_enum_t
 */
void simular_llamada_de_piso_via_can(int piso_origen, movement_direction_enum_t direccion) {
    if (!g_coap_context) {
        printf("[SIM_ASCENSOR] Error: Contexto CoAP de Gateway no disponible.\n");
        return;
    }

    printf("[SIM_ASCENSOR] Enviando LLAMADA DE PISO a GW (vía CAN): Piso %d, Dir %s\n", 
           piso_origen, (direccion == MOVING_UP) ? "SUBIR" : "BAJAR");
    simulated_can_frame_t frame;
    frame.id = 0x100; // ID CAN para llamada de piso
    frame.data[0] = (uint8_t)piso_origen;
    frame.data[1] = (direccion == MOVING_UP) ? 0 : 1; // 0 para UP, 1 para DOWN
    frame.dlc = 2;

    // Registrar frame CAN en el logger
    char description[128];
    snprintf(description, sizeof(description), "Llamada de piso desde piso %d, dirección %s", 
             piso_origen, (direccion == MOVING_UP) ? "SUBIR" : "BAJAR");
    exec_logger_log_can_sent(frame.id, frame.dlc, frame.data, description);

    ag_can_bridge_process_incoming_frame(&frame, g_coap_context);
}

/**
 * @brief Simula una solicitud de cabina vía CAN
 * @param indice_ascensor Índice del ascensor que realiza la solicitud (0-based)
 * @param piso_destino Piso destino solicitado
 * 
 * Esta función genera un frame CAN simulado que representa una solicitud
 * de cabina desde el interior de un ascensor. El frame se envía al puente
 * CAN-CoAP del API Gateway para su procesamiento.
 * 
 * **Formato del frame CAN:**
 * - ID: 0x200 (identificador para solicitudes de cabina)
 * - data[0]: Índice del ascensor (0-based, ej: 0 para E1A1)
 * - data[1]: Piso destino (0-255)
 * - DLC: 2 bytes
 * 
 * **Validaciones:**
 * - Verifica disponibilidad del contexto CoAP
 * - Valida parámetros de entrada
 * 
 * @note El índice del ascensor se mapea a IDs como E1A1, E1A2, etc.
 * 
 * @see ag_can_bridge_process_incoming_frame()
 * @see simulated_can_frame_t
 */
void simular_solicitud_cabina_via_can(int indice_ascensor, int piso_destino) {
    if (!g_coap_context) {
        printf("[SIM_ASCENSOR] Error: Contexto CoAP de Gateway no disponible.\n");
        return;
    }
    printf("[SIM_ASCENSOR] Enviando SOLICITUD DE CABINA a GW (vía CAN): Ascensor idx %d, Piso Destino %d\n", 
           indice_ascensor, piso_destino);
    simulated_can_frame_t frame;
    frame.id = 0x200; // ID CAN para solicitud de cabina
    frame.data[0] = (uint8_t)indice_ascensor; // ej: 0 para el primer ascensor (E1A1)
    frame.data[1] = (uint8_t)piso_destino;
    frame.dlc = 2;

    // Registrar frame CAN en el logger
    char description[128];
    snprintf(description, sizeof(description), "Solicitud de cabina desde ascensor índice %d al piso %d", 
             indice_ascensor, piso_destino);
    exec_logger_log_can_sent(frame.id, frame.dlc, frame.data, description);

    ag_can_bridge_process_incoming_frame(&frame, g_coap_context);
}

/**
 * @brief Simula una llamada de emergencia enviando un frame CAN al API Gateway
 * 
 * @param[in] indice_ascensor Índice del ascensor en emergencia (0-based)
 * @param[in] piso_actual Piso actual donde está el ascensor
 * @param[in] tipo_emergencia Tipo de emergencia como string
 * 
 * @details Genera un frame CAN con ID 0x400 que será procesado por el 
 * bridge CAN para crear una solicitud CoAP al servidor central.
 */
void simular_emergencia_via_can(int indice_ascensor, int piso_actual, const char* tipo_emergencia) {
    if (!g_coap_context) {
        printf("[SIM_ASCENSOR] Error: Contexto CoAP de Gateway no disponible.\n");
        return;
    }
    
    printf("[SIM_ASCENSOR] 🚨 Enviando EMERGENCIA a GW (vía CAN): Ascensor idx %d, Piso %d, Tipo: %s\n", 
           indice_ascensor, piso_actual, tipo_emergencia);
    
    simulated_can_frame_t frame;
    frame.id = 0x400; // ID CAN para emergencias
    frame.data[0] = (uint8_t)indice_ascensor; // Índice del ascensor (0-based)
    frame.data[1] = (uint8_t)piso_actual;     // Piso actual del ascensor
    
    // Convertir tipo de emergencia a enumeración
    uint8_t tipo_enum = 0; // EMERGENCY_STOP por defecto
    if (strcmp(tipo_emergencia, "EMERGENCY_STOP") == 0) {
        tipo_enum = 0;
    } else if (strcmp(tipo_emergencia, "POWER_FAILURE") == 0) {
        tipo_enum = 1;
    } else if (strcmp(tipo_emergencia, "PEOPLE_TRAPPED") == 0) {
        tipo_enum = 2;
    } else if (strcmp(tipo_emergencia, "MECHANICAL_FAILURE") == 0) {
        tipo_enum = 3;
    } else if (strcmp(tipo_emergencia, "FIRE_ALARM") == 0) {
        tipo_enum = 4;
    }
    
    frame.data[2] = tipo_enum;               // Tipo de emergencia (enumeración)
    frame.dlc = 3;

    // Registrar frame CAN en el logger
    char description[256];
    snprintf(description, sizeof(description), 
             "🚨 EMERGENCIA: %s desde ascensor índice %d en piso %d", 
             tipo_emergencia, indice_ascensor, piso_actual);
    exec_logger_log_can_sent(frame.id, frame.dlc, frame.data, description);

    ag_can_bridge_process_incoming_frame(&frame, g_coap_context);
}

/**
 * @brief Ejecuta una secuencia de eventos simulados de ascensor desde JSON de forma no-bloqueante
 * 
 * Esta función reemplaza la simulación bloqueante anterior. Ahora la simulación
 * se ejecuta de forma incremental, permitiendo que el main loop procese I/O
 * y ejecute la simulación de movimiento entre peticiones.
 * 
 * **Cambios principales:**
 * - Simulación no-bloqueante (una petición por llamada)
 * - Control regresa al main loop para permitir movimiento de ascensores
 * - Intervalo configurable entre peticiones
 * - Estado global para rastrear progreso
 * 
 * **Proceso de simulación:**
 * 1. Inicialización: selecciona edificio y configura estado
 * 2. Ejecución incremental: una petición por llamada
 * 3. El main loop ejecuta tanto I/O como simulación de movimiento
 * 4. Finalización automática al completar todas las peticiones
 * 
 * @note Esta función debe llamarse desde el main loop para ser no-bloqueante
 * @see simulate_elevator_group_step() - se ejecuta en paralelo
 */
void simular_eventos_ascensor(void) {
    printf("\n[SIM_ASCENSOR] === INICIANDO SIMULACIÓN NO-BLOQUEANTE DE EVENTOS CAN ===\n");

    // Verificar si hay datos de simulación cargados
    if (datos_simulacion_global.datos_cargados && datos_simulacion_global.num_edificios > 0) {
        printf("[SIM_ASCENSOR] Configurando simulación desde JSON con %d edificios disponibles\n", 
               datos_simulacion_global.num_edificios);

        // Seleccionar edificio aleatorio
        edificio_actual = seleccionar_edificio_aleatorio(&datos_simulacion_global);
        if (!edificio_actual) {
            printf("[SIM_ASCENSOR] Error: No se pudo seleccionar edificio. Usando simulación básica.\n");
            goto simulacion_basica;
        }

        // Configurar el ID del edificio en el sistema
        init_elevator_group(&managed_elevator_group, edificio_actual->id_edificio, 4, 14);

        printf("[SIM_ASCENSOR] Sistema configurado para edificio: %s\n", managed_elevator_group.edificio_id_str_grupo);
        printf("[SIM_ASCENSOR] Ascensores disponibles: %sA1, %sA2, %sA3, %sA4\n", 
               edificio_actual->id_edificio, edificio_actual->id_edificio, 
               edificio_actual->id_edificio, edificio_actual->id_edificio);
        printf("[SIM_ASCENSOR] Simulación NO-BLOQUEANTE: %d peticiones con %dms entre cada una\n", 
               edificio_actual->num_peticiones, INTERVALO_PETICIONES_MS);

        // Registrar inicio de simulación
        exec_logger_log_simulation_start(edificio_actual->id_edificio, edificio_actual->num_peticiones);

        // Activar simulación no-bloqueante
        simulacion_activa = true;
        peticion_actual_index = 0;
        tiempo_ultima_peticion = time(NULL);

        printf("[SIM_ASCENSOR] ✅ Simulación no-bloqueante activada. El main loop manejará las peticiones.\n");

    } else {
        printf("[SIM_ASCENSOR] No hay datos de simulación JSON. Usando simulación básica hardcodeada.\n");

    simulacion_basica:
        // Simulación básica original (mantener como fallback síncrono)
        printf("[SIM_ASCENSOR] Ejecutando simulación básica de 2 peticiones...\n");

        // Simulación 1: Llamada desde el piso 2 para subir
        simular_llamada_de_piso_via_can(2, MOVING_UP);

        // Esperar y procesar eventos CoAP
        printf("[SIM_ASCENSOR] Pausando y procesando I/O CoAP por ~2 segundos...\n");
        for (int i = 0; i < 20; ++i) { // 20 * 100ms = 2 segundos
            if (g_coap_context) {
                coap_io_process(g_coap_context, 100);
            } else {
                usleep(100000); // 100ms
            }
        }

        // Simulación 2: Solicitud desde la cabina del ascensor 0 al piso 5
        simular_solicitud_cabina_via_can(0, 5);
    }
    
    printf("[SIM_ASCENSOR] === CONFIGURACIÓN DE SIMULACIÓN COMPLETADA ===\n\n");
}

/**
 * @brief Procesa la siguiente petición de la simulación no-bloqueante
 * 
 * Esta función debe llamarse desde el main loop para ejecutar las peticiones
 * de forma incremental, permitiendo que el simulador de movimiento funcione
 * en paralelo.
 * 
 * @return true si la simulación continúa, false si ha terminado
 */
bool procesar_siguiente_peticion_simulacion(void) {
    if (!simulacion_activa || !edificio_actual) {
        return false; // Simulación no activa o no configurada
    }

    // Verificar si han pasado suficientes milisegundos desde la última petición
    time_t tiempo_actual = time(NULL);
    if ((tiempo_actual - tiempo_ultima_peticion) < (INTERVALO_PETICIONES_MS / 1000)) {
        return true; // Aún no es tiempo para la siguiente petición
    }

    // Verificar si hemos completado todas las peticiones
    if (peticion_actual_index >= edificio_actual->num_peticiones) {
        printf("[SIM_ASCENSOR] === FIN SIMULACIÓN NO-BLOQUEANTE DEL EDIFICIO %s ===\n", edificio_actual->id_edificio);
        printf("[SIM_ASCENSOR] Peticiones ejecutadas exitosamente: %d/%d\n", 
               peticion_actual_index, edificio_actual->num_peticiones);
        
        // Registrar fin de simulación
        exec_logger_log_simulation_end(peticion_actual_index, edificio_actual->num_peticiones);
        
        // Desactivar simulación
        simulacion_activa = false;
        edificio_actual = NULL;
        peticion_actual_index = 0;
        
        return false; // Simulación terminada
    }

    // Ejecutar la petición actual
    peticion_simulacion_t *peticion = &edificio_actual->peticiones[peticion_actual_index];

    printf("[SIM_ASCENSOR] --- Petición %d/%d (NO-BLOQUEANTE) ---\n", 
           peticion_actual_index + 1, edificio_actual->num_peticiones);

    if (peticion->tipo == PETICION_LLAMADA_PISO) {
        printf("[SIM_ASCENSOR] Ejecutando llamada de piso: Piso %d, Dirección %s\n", 
               peticion->piso_origen, peticion->direccion);

        movement_direction_enum_t direccion = convertir_direccion_string(peticion->direccion);
        simular_llamada_de_piso_via_can(peticion->piso_origen, direccion);

    } else if (peticion->tipo == PETICION_SOLICITUD_CABINA) {
        printf("[SIM_ASCENSOR] Ejecutando solicitud de cabina: Ascensor %d, Destino piso %d\n", 
               peticion->indice_ascensor, peticion->piso_destino);

        simular_solicitud_cabina_via_can(peticion->indice_ascensor, peticion->piso_destino);

    } else if (peticion->tipo == PETICION_LLAMADA_EMERGENCIA) {
        printf("[SIM_ASCENSOR] 🚨 Ejecutando llamada de emergencia: %s en %s\n", 
               peticion->tipo_emergencia, peticion->ascensor_id_emergencia);
        printf("[SIM_ASCENSOR]    Piso actual: %d, Descripción: %s\n", 
               peticion->piso_actual_emergencia, peticion->descripcion_emergencia);

        // Simular emergencia vía CAN (usar el índice del ascensor extraído del ID)
        int ascensor_index = peticion->indice_ascensor; // Ya se extrae en simulation_loader
        simular_emergencia_via_can(ascensor_index, peticion->piso_actual_emergencia, peticion->tipo_emergencia);

    } else {
        printf("[SIM_ASCENSOR] Advertencia: Tipo de petición desconocido: %d\n", peticion->tipo);
        // Continuar con la siguiente petición
    }

    // Actualizar estado para la siguiente petición
    peticion_actual_index++;
    tiempo_ultima_peticion = tiempo_actual;

    return true; // Simulación continúa
} 