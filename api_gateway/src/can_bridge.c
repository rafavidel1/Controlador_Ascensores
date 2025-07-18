/**
 * @file can_bridge.c
 * @brief Implementación del Puente CAN-CoAP para el API Gateway
 * @author Sistema de Control de Ascensores
 * @date 2025
 * @version 2.0
 * 
 * Este archivo implementa el puente de comunicación entre el protocolo CAN
 * (Controller Area Network) y CoAP (Constrained Application Protocol) para
 * el sistema de control de ascensores. Sus funciones principales:
 * 
 * - **Procesamiento de frames CAN**: Interpretación de mensajes CAN entrantes
 * - **Conversión CAN-CoAP**: Transformación de solicitudes CAN a CoAP
 * - **Gestión de trackers**: Seguimiento de solicitudes originadas por CAN
 * - **Respuestas CAN**: Envío de respuestas del servidor central vía CAN
 * - **Simulación**: Interfaz con simulador de red CAN para testing
 * - **Logging**: Registro detallado de operaciones para debugging
 * 
 * Tipos de mensajes CAN soportados:
 * - **0x100**: Llamadas de piso (floor calls) con dirección
 * - **0x200**: Solicitudes de cabina (cabin requests) con destino
 * - **0x300**: Notificaciones de llegada de ascensores
 * - **0x400**: Llamadas de emergencia desde ascensores
 * 
 * El puente mantiene un buffer circular de trackers para correlacionar
 * respuestas del servidor central con las solicitudes CAN originales.
 * 
 * @see can_bridge.h
 * @see api_handlers.h
 * @see elevator_state_manager.h
 * @see coap_config.h
 */

#include "api_gateway/can_bridge.h"
#include "api_gateway/api_handlers.h" // Para api_request_tracker_t, forward_request_to_central_server, get_or_create_central_server_dtls_session
#include "api_gateway/elevator_state_manager.h" // Para las enums y structs de estado, y elevator_group_to_json_for_server

#include "api_gateway/execution_logger.h" // Sistema de logging de ejecuciones

#include <coap3/coap.h> 
#include <stdio.h>
#include <string.h>
#include <stdlib.h> // Para atoi
#include <arpa/inet.h> // <--- AÑADIR PARA inet_pton
#include <time.h> // Para time(), gmtime(), strftime()

/**
 * @brief Longitud máxima de datos en un frame CAN estándar
 * 
 * Define el número máximo de bytes de datos que puede contener
 * un frame CAN estándar según la especificación CAN 2.0.
 */
#define CAN_MAX_DATA_LEN 8

/**
 * @brief Función helper para logging de tokens CoAP
 * @param prefix Prefijo para el mensaje de log
 * @param token Token CoAP a imprimir en formato hexadecimal
 * 
 * Esta función convierte un token CoAP binario a su representación
 * hexadecimal para facilitar el debugging y seguimiento de solicitudes.
 * Limita la impresión a 8 bytes máximo para evitar logs excesivamente largos.
 */
static void log_coap_token(const char *prefix, coap_bin_const_t token) {
    if (!token.s) {
        LOG_DEBUG_GW("%s: Token es NULL", prefix);
        return;
    }
    char token_hex_str[8 * 2 + 1]; // Suficiente para 8 bytes de token + null
    size_t len_to_print = token.length > 8 ? 8 : token.length; // No imprimir más de 8 bytes
    for (size_t i = 0; i < len_to_print; ++i) {
        sprintf(&token_hex_str[i * 2], "%02x", token.s[i]);
    }
    token_hex_str[len_to_print * 2] = '\0';
    LOG_DEBUG_GW("%s: (len %zu) %s", prefix, token.length, token_hex_str);
}

/**
 * @brief Callback registrado para enviar frames CAN a la simulación
 * 
 * Puntero a función que se utiliza para enviar frames CAN de respuesta
 * al simulador. Se registra mediante ag_can_bridge_register_send_callback().
 * 
 * @see ag_can_bridge_register_send_callback()
 * @see can_send_callback_t
 */
static can_send_callback_t send_to_simulation_callback = NULL;

/**
 * @brief Número máximo de trackers CAN simultáneos
 * 
 * Define el tamaño del buffer circular utilizado para almacenar
 * información de solicitudes originadas por CAN que están pendientes
 * de respuesta del servidor central.
 */
#define MAX_CAN_ORIGIN_TRACKERS 10

/**
 * @brief Buffer circular de trackers para solicitudes CAN
 * 
 * Array que almacena información de correlación entre solicitudes
 * CAN enviadas al servidor central y sus respuestas esperadas.
 * Se gestiona como un buffer circular para reutilizar entradas.
 * 
 * @see can_origin_tracker_t
 */
static can_origin_tracker_t can_trackers[MAX_CAN_ORIGIN_TRACKERS];

/**
 * @brief Índice del siguiente tracker CAN disponible
 * 
 * Índice que apunta a la siguiente posición disponible en el buffer
 * circular can_trackers. Se incrementa módulo MAX_CAN_ORIGIN_TRACKERS.
 */
static int next_can_tracker_idx = 0;

/**
 * @brief Estado del grupo de ascensores gestionado
 * 
 * Referencia externa al estado global del grupo de ascensores
 * definido en main.c. Se utiliza para acceder a información
 * del edificio y ascensores al procesar mensajes CAN.
 * 
 * @see main.c
 * @see elevator_group_state_t
 */
extern elevator_group_state_t managed_elevator_group;

/**
 * @brief Almacena un tracker CAN para correlación de respuestas
 * @param token Token CoAP de la solicitud enviada al servidor central
 * @param can_id ID del frame CAN original
 * @param req_type Tipo de solicitud (floor call, cabin request, etc.)
 * @param target_floor Piso destino de la tarea
 * @param ref_floor Piso de referencia (origen para floor calls)
 * @param elevator_id_if_cabin ID del ascensor (solo para cabin requests)
 * 
 * Esta función almacena información de correlación en el buffer circular
 * de trackers CAN. Crea una copia del token CoAP para evitar problemas
 * de memoria y libera automáticamente tokens anteriores si es necesario.
 * 
 * @see find_can_tracker()
 * @see can_origin_tracker_t
 */
static void store_can_tracker(coap_bin_const_t token, uint32_t can_id, 
                              gw_request_type_t req_type, int target_floor, int ref_floor,
                              const char* elevator_id_if_cabin) {
    if (token.length == 0 || token.s == NULL) return;

    can_origin_tracker_t *tracker = &can_trackers[next_can_tracker_idx];
    if (tracker->coap_token.s) { // Liberar token anterior si existe
        coap_free(tracker->coap_token.s);
    }
    tracker->coap_token.length = token.length;
    tracker->coap_token.s = (uint8_t *)coap_malloc(token.length);
    if (!tracker->coap_token.s) {
        LOG_ERROR_GW("[CAN_Bridge] Error al asignar memoria para token en tracker CAN.");
        tracker->coap_token.length = 0;
        return;
    }
    memcpy(tracker->coap_token.s, token.s, token.length);
    tracker->original_can_id = can_id;
    tracker->request_type = req_type;
    tracker->target_floor_for_task = target_floor;
    tracker->call_reference_floor = ref_floor;
    if (elevator_id_if_cabin) {
        strncpy(tracker->requesting_elevator_id_if_cabin, elevator_id_if_cabin, ID_STRING_MAX_LEN - 1);
        tracker->requesting_elevator_id_if_cabin[ID_STRING_MAX_LEN - 1] = '\0';
    } else {
        tracker->requesting_elevator_id_if_cabin[0] = '\0';
    }

    log_coap_token("[CAN_Bridge] Stored token for CAN tracker", token);
    LOG_DEBUG_GW("[CAN_Bridge] Stored CAN ID 0x%X at index %d", can_id, next_can_tracker_idx);
    next_can_tracker_idx = (next_can_tracker_idx + 1) % MAX_CAN_ORIGIN_TRACKERS;
}

/**
 * @brief Busca un tracker CAN por token CoAP
 * @param token Token CoAP a buscar en los trackers almacenados
 * @return Puntero al tracker encontrado, o NULL si no se encuentra
 * 
 * Esta función busca en el buffer circular de trackers CAN un tracker
 * que corresponda al token CoAP especificado. Se utiliza para correlacionar
 * respuestas del servidor central con solicitudes CAN originales.
 * 
 * La búsqueda se realiza comparando tanto la longitud como el contenido
 * binario del token. No remueve el tracker de la lista (a diferencia
 * de find_and_remove_central_request_tracker).
 * 
 * @see store_can_tracker()
 * @see can_origin_tracker_t
 */
can_origin_tracker_t* find_can_tracker(coap_bin_const_t token) {
    log_coap_token("[CAN_Bridge] Finding token for CAN tracker", token);
    if (token.length == 0 || token.s == NULL) return NULL;
    for (int i = 0; i < MAX_CAN_ORIGIN_TRACKERS; ++i) {
        if (can_trackers[i].coap_token.length == token.length && 
            can_trackers[i].coap_token.s && 
            memcmp(can_trackers[i].coap_token.s, token.s, token.length) == 0) {
            // LOG_DEBUG_GW("[CAN_Bridge] Found CAN tracker for token (len %zu) at index %d", token.length, i);
            return &can_trackers[i];
        }
    }
    // LOG_DEBUG_GW("[CAN_Bridge] No CAN tracker found for token (len %zu)", token.length);
    return NULL;
}

/**
 * @brief Inicializa el puente CAN-CoAP
 * 
 * Esta función inicializa el sistema de puente CAN-CoAP, preparando
 * todas las estructuras de datos necesarias para el funcionamiento.
 * 
 * Operaciones realizadas:
 * - Limpia el callback de envío CAN
 * - Libera memoria de tokens almacenados en trackers
 * - Inicializa a cero el buffer de trackers CAN
 * - Resetea el índice del buffer circular
 * 
 * Debe llamarse una vez al inicio del programa antes de procesar
 * cualquier frame CAN o registrar callbacks.
 * 
 * @see ag_can_bridge_register_send_callback()
 * @see ag_can_bridge_process_incoming_frame()
 */
void ag_can_bridge_init(void) {
    LOG_INFO_GW("[CAN_Bridge] Inicializando el puente CAN simulado.");
    send_to_simulation_callback = NULL;
    for (int i = 0; i < MAX_CAN_ORIGIN_TRACKERS; ++i) {
        if (can_trackers[i].coap_token.s) {
            coap_free(can_trackers[i].coap_token.s);
        }
        memset(&can_trackers[i], 0, sizeof(can_origin_tracker_t));
    }
    next_can_tracker_idx = 0;
}

/**
 * @brief Registra el callback para envío de frames CAN al simulador
 * @param callback Función callback que será llamada para enviar frames CAN
 * 
 * Esta función registra una función callback que será utilizada por el
 * puente CAN-CoAP para enviar frames CAN de respuesta al simulador.
 * 
 * El callback debe implementar la interfaz can_send_callback_t y será
 * invocado cuando el puente necesite enviar respuestas del servidor
 * central de vuelta al sistema CAN simulado.
 * 
 * @note Solo se puede registrar un callback a la vez. Llamadas posteriores
 *       sobrescriben el callback anterior.
 * 
 * @see can_send_callback_t
 * @see ag_can_bridge_send_response_frame()
 */
void ag_can_bridge_register_send_callback(can_send_callback_t callback) {
    LOG_INFO_GW("[CAN_Bridge] Registrando callback para enviar frames CAN a la simulación.");
    send_to_simulation_callback = callback;
}

// Declaración adelantada de una función helper similar a forward_request_to_central_server
// pero adaptada para orígenes CAN.
static void forward_can_originated_request_to_central_server(
    coap_context_t *ctx,
    uint32_t original_can_id,
    const char *central_server_path,
    const char *log_tag_param,
    gw_request_type_t request_type_param,
    int origin_floor_param,
    int target_floor_for_task_param,
    const char* requesting_elevator_id_cabin_param,
    movement_direction_enum_t requested_direction_floor_param
); // Definición más abajo

// Declaración adelantada de función específica para emergencias
static void forward_can_emergency_request_to_central_server(
    coap_context_t *ctx,
    uint32_t original_can_id,
    const char *central_server_path,
    const char *log_tag_param,
    const char *elevator_id,
    int emergency_floor,
    api_gateway_coap_para_sistema_de_ascensores_emergency_type__e emergency_type,
    const char *description,
    const char *timestamp
); // Definición más abajo


/**
 * @brief Procesa un frame CAN entrante y lo convierte a solicitud CoAP
 * @param frame Puntero al frame CAN simulado a procesar
 * @param coap_ctx Contexto CoAP para enviar solicitudes al servidor central
 * 
 * Esta función es el punto de entrada principal para el procesamiento
 * de frames CAN entrantes. Interpreta el contenido del frame según
 * el esquema de mensajes CAN definido y genera las solicitudes CoAP
 * correspondientes al servidor central.
 * 
 * **Tipos de frames CAN soportados:**
 * - **0x100 - Llamada de piso**: 
 *   - data[0]: Piso origen (0-255)
 *   - data[1]: Dirección (0=UP, 1=DOWN)
 * - **0x200 - Solicitud de cabina**:
 *   - data[0]: Índice del ascensor (0-based)
 *   - data[1]: Piso destino (0-255)
 * - **0x300 - Notificación de llegada**:
 *   - data[0]: Índice del ascensor (0-based)
 *   - data[1]: Piso actual (0-255)
 * 
 * Para cada frame válido, la función:
 * 1. Valida el formato y longitud de datos
 * 2. Extrae los parámetros específicos del tipo de mensaje
 * 3. Genera una solicitud CoAP al servidor central
 * 4. Almacena un tracker para correlacionar la respuesta
 * 
 * @note Los IDs CAN y formato de datos deben adaptarse según
 *       el esquema específico del sistema de ascensores
 * 
 * @see forward_can_originated_request_to_central_server()
 * @see store_can_tracker()
 * @see simulated_can_frame_t
 */
void ag_can_bridge_process_incoming_frame(simulated_can_frame_t* frame, coap_context_t *coap_ctx) {
    if (!frame) {
        LOG_ERROR_GW("[CAN_Bridge] Frame CAN simulado nulo recibido.");
        return;
    }
    if (!coap_ctx) {
        LOG_ERROR_GW("[CAN_Bridge] Contexto CoAP nulo al procesar frame CAN.");
        return;
    }

    LOG_INFO_GW("[CAN_Bridge] Procesando frame CAN ID: 0x%X, DLC: %d", frame->id, frame->dlc);

    // --- Lógica de ejemplo para interpretar IDs CAN ---
    // --- ¡DEBES ADAPTAR ESTO A TU ESQUEMA DE MENSAJES CAN! ---
    switch (frame->id) {
        case 0x100: // Ejemplo: Llamada de piso
            if (frame->dlc >= 2) {
                int piso_origen = frame->data[0];
                movement_direction_enum_t direccion = (frame->data[1] == 0) ? MOVING_UP : MOVING_DOWN; // 0=UP, 1=DOWN
                LOG_INFO_GW("[CAN_Bridge] Llamada de piso CAN: Piso %d, Dirección %s", piso_origen, movement_direction_to_string(direccion));
                
                forward_can_originated_request_to_central_server(
                    coap_ctx, frame->id,
                    getenv("FLOOR_CALL_RESOURCE"), 
                    "CAN_FloorCall", 
                    GW_REQUEST_TYPE_FLOOR_CALL, 
                    piso_origen, 
                    piso_origen, // Para floor call, el target inicial es el mismo piso origen
                    NULL, // No aplica elevator_id para el tracker aquí (es floor call)
                    direccion);
            } else {
                LOG_WARN_GW("[CAN_Bridge] Frame CAN 0x100 (Llamada Piso) con DLC insuficiente: %d", frame->dlc);
            }
            break;

        case 0x200: // Ejemplo: Solicitud de cabina
            if (frame->dlc >= 2) {
                char elevator_id_str[atoi(getenv("ID_STRING_MAX_LEN"))];
                int elevator_number = frame->data[0] + 1;
                int max_building_id_len = atoi(getenv("ID_STRING_MAX_LEN")) - 1 /*null*/ - 1 /*A*/ - 3 /*NNN for elevator number*/;
                if (max_building_id_len < 1) max_building_id_len = 1; // ensure at least 1 char for building id part

                snprintf(elevator_id_str, sizeof(elevator_id_str), "%.*sA%d", 
                         max_building_id_len,
                         managed_elevator_group.edificio_id_str_grupo, 
                         elevator_number);
                int piso_destino = frame->data[1];
                LOG_INFO_GW("[CAN_Bridge] Solicitud de cabina CAN: Ascensor %s (idx %d), Piso Destino %d", elevator_id_str, frame->data[0], piso_destino);

                forward_can_originated_request_to_central_server(
                    coap_ctx, frame->id,
                    getenv("CABIN_REQUEST_RESOURCE"), 
                    "CAN_CabinReq", 
                    GW_REQUEST_TYPE_CABIN_REQUEST, 
                    -1, // No aplica origin_floor para cabin request aquí como ref_floor para el tracker (podría ser el actual del elevador)
                    piso_destino, 
                    elevator_id_str, // requesting_elevator_id_cabin_param
                    DIRECTION_UNKNOWN);
            } else {
                LOG_WARN_GW("[CAN_Bridge] Frame CAN 0x200 (Solicitud Cabina) con DLC insuficiente: %d", frame->dlc);
            }
            break;
        
        case 0x300: // Ejemplo: Notificación de llegada (esto lo maneja la simulación interna, pero si viniera de CAN)
            if (frame->dlc >= 2) {
                char elevator_id_str[atoi(getenv("ID_STRING_MAX_LEN"))];
                int elevator_number = frame->data[0] + 1;
                int max_building_id_len = atoi(getenv("ID_STRING_MAX_LEN")) - 1 /*null*/ - 1 /*A*/ - 3 /*NNN for elevator number*/;
                if (max_building_id_len < 1) max_building_id_len = 1; // ensure at least 1 char for building id part
                
                snprintf(elevator_id_str, sizeof(elevator_id_str), "%.*sA%d", 
                         max_building_id_len,
                         managed_elevator_group.edificio_id_str_grupo, 
                         elevator_number);
                int piso_actual = frame->data[1];
                // Opcional: door_state frame->data[2]
                LOG_INFO_GW("[CAN_Bridge] Notificación de llegada CAN: Ascensor %s, Piso %d", elevator_id_str, piso_actual);
                
                // Actualizar estado local directamente
                bool found = false;
                for (int i = 0; i < managed_elevator_group.num_elevadores_en_grupo; ++i) {
                    elevator_status_t *elevator = &managed_elevator_group.ascensores[i];
                    if (strcmp(elevator->ascensor_id, elevator_id_str) == 0) {
                        found = true;
                        LOG_INFO_GW("StateMgr: Ascensor %s llegó al piso %d. (Piso anterior: %d, Destino tarea: %d)", 
                                    elevator->ascensor_id, piso_actual, elevator->piso_actual, elevator->destino_actual);
                        
                        // Registrar movimiento del ascensor en el logger
                        exec_logger_log_elevator_moved(elevator->ascensor_id, elevator->piso_actual, piso_actual, 
                                                      movement_direction_to_string(elevator->direccion_movimiento_enum));
                        
                        elevator->piso_actual = piso_actual;

                        if (elevator->destino_actual == piso_actual) {
                            LOG_INFO_GW("StateMgr: Ascensor %s completó tarea %s en piso %d.", 
                                        elevator->ascensor_id, 
                                        elevator->tarea_actual_id[0] != '\0' ? elevator->tarea_actual_id : "N/A", 
                                        piso_actual);
                            
                            // Registrar completación de tarea en el logger
                            if (elevator->tarea_actual_id[0] != '\0') {
                                exec_logger_log_task_completed(elevator->tarea_actual_id, elevator->ascensor_id, piso_actual);
                            }
                            
                            elevator->estado_puerta_enum = DOOR_OPEN;
                            elevator->ocupado = false;
                            elevator->tarea_actual_id[0] = '\0'; // Limpiar ID de tarea
                            elevator->destino_actual = -1;        // Limpiar destino
                            elevator->direccion_movimiento_enum = STOPPED;
                            
                            LOG_INFO_GW("[CAN_Bridge] Tarea completada por %s (vía CAN). Se notificará al servidor.", elevator_id_str);
                        } else {
                            LOG_WARN_GW("StateMgr: Ascensor %s llegó a piso %d, pero su destino final es %d. No se completa tarea aún.",
                                        elevator->ascensor_id, piso_actual, elevator->destino_actual);
                        }
                        break;
                    }
                }

                if (!found) {
                    LOG_ERROR_GW("StateMgr: notify_arrival - Ascensor con ID '%s' no encontrado en el grupo.", elevator_id_str);
                }
            } else {
                 LOG_WARN_GW("[CAN_Bridge] Frame CAN 0x300 (Notif. Llegada) con DLC insuficiente: %d", frame->dlc);
            }
            break;

        case 0x400: // Nuevo: Llamada de emergencia
            if (frame->dlc >= 3) { // Necesitamos al menos 3 bytes de datos
                // Decodificar datos del frame CAN
                int elevator_index = frame->data[0];  // Índice del ascensor (0-based)
                int emergency_floor = frame->data[1]; // Piso donde está el ascensor
                int emergency_type_int = frame->data[2]; // Tipo de emergencia
                
                // Construir ID del ascensor
                char elevator_id_str[atoi(getenv("ID_STRING_MAX_LEN"))];
                int max_building_id_len = atoi(getenv("ID_STRING_MAX_LEN")) - 1 /*null*/ - 1 /*A*/ - 3 /*NNN for elevator number*/;
                if (max_building_id_len < 1) max_building_id_len = 1; // ensure at least 1 char for building id part
                
                snprintf(elevator_id_str, sizeof(elevator_id_str), "%.*sA%d", 
                         max_building_id_len,
                         managed_elevator_group.edificio_id_str_grupo, 
                         elevator_index + 1);
                
                // Convertir tipo de emergencia (ajustar offset +1 porque enum empieza en 1, no 0)
                api_gateway_coap_para_sistema_de_ascensores_emergency_type__e emergency_type;
                switch (emergency_type_int) {
                    case 0: emergency_type = api_gateway_coap_para_sistema_de_ascensores_emergency_type__EMERGENCY_STOP; break;
                    case 1: emergency_type = api_gateway_coap_para_sistema_de_ascensores_emergency_type__POWER_FAILURE; break;
                    case 2: emergency_type = api_gateway_coap_para_sistema_de_ascensores_emergency_type__PEOPLE_TRAPPED; break;
                    case 3: emergency_type = api_gateway_coap_para_sistema_de_ascensores_emergency_type__MECHANICAL_FAILURE; break;
                    case 4: emergency_type = api_gateway_coap_para_sistema_de_ascensores_emergency_type__FIRE_ALARM; break;
                    default: emergency_type = api_gateway_coap_para_sistema_de_ascensores_emergency_type__NULL; break;
                }
                
                // Generar timestamp
                char timestamp_str[32];
                time_t now = time(NULL);
                struct tm *utc_time = gmtime(&now);
                strftime(timestamp_str, sizeof(timestamp_str), "%Y-%m-%dT%H:%M:%SZ", utc_time);
                
                // Crear descripción
                char description[512];
                snprintf(description, sizeof(description), 
                         "Emergencia reportada por ascensor %s en piso %d", 
                         elevator_id_str, emergency_floor);
                
                LOG_WARN_GW("[CAN_Bridge] 🚨 EMERGENCIA CAN: Ascensor %s, Piso %d, Tipo %s", 
                           elevator_id_str, emergency_floor, emergency_type_to_string(emergency_type));
                
                // Usar implementación especial para emergencias que llena todos los campos
                forward_can_emergency_request_to_central_server(
                    coap_ctx, frame->id,
                    getenv("EMERGENCY_CALL_RESOURCE") ?: "/llamada_emergencia",
                    "CAN_Emergency", 
                    elevator_id_str,
                    emergency_floor,
                    emergency_type,
                    description,
                    timestamp_str);
            } else {
                LOG_WARN_GW("[CAN_Bridge] Frame CAN 0x400 (Emergencia) con DLC insuficiente: %d", frame->dlc);
            }
            break;

        default:
            LOG_WARN_GW("[CAN_Bridge] ID de frame CAN simulado desconocido: 0x%X", frame->id);
            break;
    }
}

/**
 * @brief Envía una respuesta (traducida de CoAP) como un frame CAN simulado a la simulación.
 */
void ag_can_bridge_send_response_frame(uint32_t original_can_id, coap_pdu_code_t response_code, cJSON* server_response_json) {
    if (!send_to_simulation_callback) {
        LOG_WARN_GW("[CAN_Bridge] Callback de envío a simulación no registrado. No se puede enviar respuesta CAN.");
        return;
    }

    simulated_can_frame_t response_frame;
    memset(&response_frame, 0, sizeof(simulated_can_frame_t));
    response_frame.dlc = 0; // Initialize DLC

    bool is_success_code = (COAP_RESPONSE_CLASS(response_code) == 2);

    if (!server_response_json) {
        LOG_WARN_GW("[CAN_Bridge] JSON de respuesta del servidor nulo.");
        if (is_success_code) {
            LOG_ERROR_GW("[CAN_Bridge] JSON nulo pero código CoAP de éxito (0x%X)! Enviando error genérico CAN.", response_code);
        }
        response_frame.id = 0xFE; // Generic error CAN ID from GW
        response_frame.data[0] = original_can_id & 0xFF; // Original CAN ID LSB
        response_frame.data[1] = 0x01; // Error code: 1 = Missing/Null JSON from server
        response_frame.dlc = 2;
        send_to_simulation_callback(&response_frame);
        return;
    }

    // Check for "error" field in JSON, or if CoAP code indicates error
    cJSON *j_error = cJSON_GetObjectItemCaseSensitive(server_response_json, "error");
    if (j_error || !is_success_code) {
        response_frame.id = 0xFE; // Generic error CAN ID from GW
        response_frame.data[0] = original_can_id & 0xFF; // Original CAN ID LSB
        response_frame.data[1] = 0x02; // Error code: 2 = Server reported error or CoAP error code
        if (cJSON_IsString(j_error) && j_error->valuestring != NULL) {
            LOG_WARN_GW("[CAN_Bridge] Servidor Central reportó error: %s. CoAP code: 0x%X", j_error->valuestring, response_code);
            // Potentially copy part of the error message to data[2] onwards if space and useful
        } else {
            LOG_WARN_GW("[CAN_Bridge] Error de CoAP (code 0x%X) o JSON de error no estándar del Servidor Central.", response_code);
        }
        response_frame.dlc = 2;
    } else {
        // --- Lógica para respuesta exitosa (como antes) ---
        cJSON *j_ascensor_asignado_id = cJSON_GetObjectItemCaseSensitive(server_response_json, "ascensor_asignado_id");
        cJSON *j_tarea_id = cJSON_GetObjectItemCaseSensitive(server_response_json, "tarea_id");

        response_frame.id = original_can_id + 1; // Example: Success CAN ID

        uint8_t current_dlc = 0;
        if (cJSON_IsString(j_ascensor_asignado_id) && j_ascensor_asignado_id->valuestring != NULL) {
            const char* assigned_id_str = j_ascensor_asignado_id->valuestring;
            // Attempt to extract elevator index number (e.g., from "E1A1" -> 0, "E1A2" -> 1)
            // This logic assumes a pattern like BuildingPrefix + 'A' + Number
            const char *num_part = strrchr(assigned_id_str, 'A'); // Find last 'A'
            if (num_part && *(num_part + 1) != '\0') { // Corrected: single backslash for null terminator
                int elevator_num = atoi(num_part + 1);
                if (elevator_num > 0) { // atoi returns 0 on error or for "A0"
                    response_frame.data[current_dlc++] = (uint8_t)(elevator_num - 1);
                } else {
                    LOG_WARN_GW("[CAN_Bridge] No se pudo extraer número válido de ascensor de: %s", assigned_id_str);
                    response_frame.data[current_dlc++] = 0xFF; // Error/unknown elevator index
                }
            } else {
                LOG_WARN_GW("[CAN_Bridge] Formato de ID de ascensor no esperado: %s", assigned_id_str);
                response_frame.data[current_dlc++] = 0xFF; // Error/unknown elevator index
            }
        } else {
            LOG_WARN_GW("[CAN_Bridge] 'ascensor_asignado_id' no encontrado o no es string en JSON de éxito.");
            response_frame.data[current_dlc++] = 0xFF; // Indicate missing assigned elevator
        }

        if (cJSON_IsString(j_tarea_id) && j_tarea_id->valuestring != NULL) {
            strncpy((char*)&response_frame.data[current_dlc], j_tarea_id->valuestring, CAN_MAX_DATA_LEN - current_dlc);
            // Ensure null termination if it fits, though CAN data isn't typically C-strings
            if (strlen(j_tarea_id->valuestring) < (CAN_MAX_DATA_LEN - current_dlc)) {
                 response_frame.data[current_dlc + strlen(j_tarea_id->valuestring)] = '\0'; // Corrected: single backslash for null terminator // if space allows
            }
            current_dlc += strnlen((char*)&response_frame.data[current_dlc], CAN_MAX_DATA_LEN - current_dlc);
        } else {
            LOG_WARN_GW("[CAN_Bridge] 'tarea_id' no encontrado o no es string en JSON de éxito.");
            // No specific byte for missing task_id, but DLC will be smaller
        }
        
        response_frame.dlc = current_dlc;
        if (response_frame.dlc == 0) { // If somehow nothing was added
             LOG_WARN_GW("[CAN_Bridge] Respuesta de éxito pero no se generaron datos CAN para ID original 0x%X. Enviando error CAN.", original_can_id);
             response_frame.id = 0xFE; 
             response_frame.data[0] = original_can_id & 0xFF;
             response_frame.data[1] = 0x03; // Error code: 3 = Failed to parse success JSON
             response_frame.dlc = 2;
        }
    }
    
    LOG_INFO_GW("[CAN_Bridge] Enviando respuesta CAN ID: 0x%X, DLC: %d. Datos: %02X %02X %02X...", 
                response_frame.id, response_frame.dlc, 
                response_frame.dlc > 0 ? response_frame.data[0] : 0,
                response_frame.dlc > 1 ? response_frame.data[1] : 0,
                response_frame.dlc > 2 ? response_frame.data[2] : 0);
    send_to_simulation_callback(&response_frame);
}

// Helper para enviar solicitudes CoAP originadas por CAN
// Similar a forward_request_to_central_server en api_handlers.c
// pero sin la parte del cliente CoAP original y con seguimiento CAN.
static void
forward_can_originated_request_to_central_server(
    coap_context_t *ctx,
    uint32_t original_can_id,
    const char *central_server_path,
    const char *log_tag_param,
    gw_request_type_t request_type_param,
    int origin_floor_param,
    int target_floor_for_task_param,
    const char* requesting_elevator_id_cabin_param,
    movement_direction_enum_t requested_direction_floor_param
) {
    if (!central_server_path || !log_tag_param) {
        LOG_ERROR_GW("[CAN_Fwd] Error: central_server_path o log_tag_param es NULL.");
        return;
    }
     if (!ctx) {
        LOG_ERROR_GW(ANSI_COLOR_RED "[%s] Error: No se pudo obtener contexto CoAP." ANSI_COLOR_RESET "\n", log_tag_param);
        return;
    }

    LOG_INFO_GW(ANSI_COLOR_YELLOW "[%s] Gateway (Origen CAN ID: 0x%X): Preparando solicitud para Servidor Central." ANSI_COLOR_RESET "\n", log_tag_param, original_can_id);

    // ---- Detalles de la solicitud (para JSON) ----
    api_request_details_for_json_t json_details; // Reutilizamos la struct de api_handlers
    memset(&json_details, 0, sizeof(api_request_details_for_json_t));
    switch (request_type_param) {
        case GW_REQUEST_TYPE_FLOOR_CALL:
            json_details.origin_floor_fc = origin_floor_param;
            json_details.direction_fc = requested_direction_floor_param;
            break;
        case GW_REQUEST_TYPE_CABIN_REQUEST:
            if (requesting_elevator_id_cabin_param) {
                strncpy(json_details.requesting_elevator_id_cr, requesting_elevator_id_cabin_param, ID_STRING_MAX_LEN - 1);
                json_details.requesting_elevator_id_cr[ID_STRING_MAX_LEN - 1] = '\0';
            }
            json_details.target_floor_cr = target_floor_for_task_param;
            break;
        // GW_REQUEST_TYPE_EMERGENCY_CALL usa función específica forward_can_emergency_request_to_central_server
        default: break;
    }
    
    // ---- Generar Payload JSON ----
    char *json_payload_str = NULL;
    // Usamos &managed_elevator_group que es extern y está definida en main.c
    cJSON *json_payload_obj = elevator_group_to_json_for_server(&managed_elevator_group, request_type_param, &json_details);
    if (!json_payload_obj) {
        LOG_ERROR_GW(ANSI_COLOR_RED "[%s] Error: Fallo al generar JSON para origen CAN." ANSI_COLOR_RESET "\n", log_tag_param);
        return; // No hay tracker que liberar aquí, es estático.
    }
    json_payload_str = cJSON_PrintUnformatted(json_payload_obj);
    cJSON_Delete(json_payload_obj);
    if (!json_payload_str) {
        LOG_ERROR_GW(ANSI_COLOR_RED "[%s] Error: Fallo al convertir JSON a string para origen CAN." ANSI_COLOR_RESET "\n", log_tag_param);
        return; // No hay tracker que liberar aquí
    }
    LOG_DEBUG_GW("[%s] Payload para Servidor Central (Origen CAN ID: 0x%X): %s", log_tag_param, original_can_id, json_payload_str);

    // ---- Sesión con el servidor central (DTLS) ----
    // La sesión es global y se obtiene/crea a través de get_or_create_central_server_dtls_session
    coap_session_t *session_to_central = get_or_create_central_server_dtls_session(ctx);
    if (!session_to_central) {
        LOG_ERROR_GW(ANSI_COLOR_RED "[%s] Error creando/obteniendo sesión DTLS con servidor central para origen CAN." ANSI_COLOR_RESET "\n", log_tag_param);
        free(json_payload_str);
        return; // No hay tracker que liberar, y la sesión global se gestiona internamente
    }

    // ---- Crear PDU para enviar al servidor central ----
    coap_pdu_t *pdu_to_central = coap_new_pdu(COAP_MESSAGE_CON, COAP_REQUEST_CODE_POST, session_to_central);
    if (!pdu_to_central) {
        LOG_ERROR_GW(ANSI_COLOR_RED "[%s] Error creando PDU para servidor central (origen CAN)." ANSI_COLOR_RESET "\n", log_tag_param);
        free(json_payload_str);
        // La sesión global no se libera aquí
        return;
    }

    // ---- Añadir Token NUEVO a la PDU para el servidor central ----
    uint8_t token_data[8]; 
    size_t token_length = sizeof(token_data);
    coap_session_new_token(session_to_central, &token_length, token_data);
    
    if (!coap_add_token(pdu_to_central, token_length, token_data)) {
         LOG_WARN_GW(ANSI_COLOR_YELLOW "[%s] Advertencia: Fallo al añadir NUEVO token a PDU (origen CAN)." ANSI_COLOR_RESET "\n", log_tag_param);
    }
    coap_bin_const_t pdu_token_to_central = coap_pdu_get_token(pdu_to_central);

    // Guardar el tracker para esta solicitud CAN
    store_can_tracker(pdu_token_to_central, original_can_id, request_type_param, target_floor_for_task_param, origin_floor_param, requesting_elevator_id_cabin_param);

    // ---- Añadir Opciones de URI (Uri-Path) ----
    char qualified_target_path[256];
    const char* path_to_use = central_server_path;
    
    // Limpiar la cadena de caracteres de control y espacios
    char cleaned_path[256];
    size_t cleaned_len = 0;
    for (size_t i = 0; path_to_use[i] != '\0' && i < sizeof(cleaned_path) - 1; i++) {
        if (path_to_use[i] != '\r' && path_to_use[i] != '\n' && path_to_use[i] != '\t') {
            cleaned_path[cleaned_len++] = path_to_use[i];
        }
    }
    cleaned_path[cleaned_len] = '\0';
    
    // Eliminar espacios al final
    while (cleaned_len > 0 && (cleaned_path[cleaned_len-1] == ' ' || cleaned_path[cleaned_len-1] == '\t')) {
        cleaned_path[--cleaned_len] = '\0';
    }
    
    if (cleaned_path[0] == '/') {
        strncpy(qualified_target_path, cleaned_path, sizeof(qualified_target_path) - 1);
        qualified_target_path[sizeof(qualified_target_path) - 1] = '\0';
    } else {
        // Verificar que hay espacio suficiente para "/" + cleaned_path + null terminator
        size_t cleaned_len = strlen(cleaned_path);
        size_t max_path_len = sizeof(qualified_target_path) - 1; // Reservar espacio para null terminator
        if (cleaned_len + 1 <= max_path_len) { // +1 para el "/"
            int result = snprintf(qualified_target_path, sizeof(qualified_target_path), "/%s", cleaned_path);
            if (result >= (int)sizeof(qualified_target_path)) {
                // Truncación ocurrió, asegurar terminación nula
                qualified_target_path[sizeof(qualified_target_path) - 1] = '\0';
                LOG_WARN_GW("[%s] Warning: Path truncado en qualified_target_path", log_tag_param);
            }
        } else {
            // Si es demasiado largo, truncar manualmente
            strncpy(qualified_target_path, "/", sizeof(qualified_target_path) - 1);
            size_t remaining_space = sizeof(qualified_target_path) - 2; // -2 para "/" y null terminator
            strncat(qualified_target_path, cleaned_path, remaining_space);
            qualified_target_path[sizeof(qualified_target_path) - 1] = '\0';
            LOG_WARN_GW("[%s] Warning: Path demasiado largo, truncado", log_tag_param);
        }
    }

    coap_uri_t *uri_obj = coap_new_uri((const uint8_t *)qualified_target_path, strlen(qualified_target_path));
    if (uri_obj) {
        coap_optlist_t *optlist_head = NULL;
        // Usar la dirección remota de la sesión DTLS con el servidor central
        const coap_address_t *remote_addr = coap_session_get_addr_remote(session_to_central);
        if (remote_addr) {
            if (coap_uri_into_optlist(uri_obj, remote_addr, &optlist_head, 1) == 1) {
                coap_add_optlist_pdu(pdu_to_central, &optlist_head);
                coap_delete_optlist(optlist_head);
            } else {
                LOG_ERROR_GW(ANSI_COLOR_RED "[%s] Error creando lista de opciones desde URI (origen CAN): %s" ANSI_COLOR_RESET "\n", log_tag_param, qualified_target_path);
            }
        } else {
            LOG_ERROR_GW(ANSI_COLOR_RED "[%s] Error obteniendo dirección remota de la sesión DTLS para URI (origen CAN)." ANSI_COLOR_RESET "\n", log_tag_param);
        }
        coap_delete_uri(uri_obj);
    } else {
         LOG_ERROR_GW(ANSI_COLOR_RED "[%s] Error creando objeto URI desde path (origen CAN): %s" ANSI_COLOR_RESET "\n", log_tag_param, qualified_target_path);
    }

    // ---- Añadir Opción Content-Format (application/json) ----
    uint8_t ct_buf[2]; // Suficiente para codificar application/json
    coap_add_option(pdu_to_central, COAP_OPTION_CONTENT_FORMAT, 
                    coap_encode_var_safe(ct_buf, sizeof(ct_buf), COAP_MEDIATYPE_APPLICATION_JSON), ct_buf);

    // ---- Añadir Payload JSON ----
    if (json_payload_str && strlen(json_payload_str) > 0) {
        if (!coap_add_data(pdu_to_central, strlen(json_payload_str), (const uint8_t *)json_payload_str)) {
            LOG_ERROR_GW(ANSI_COLOR_RED "[%s] Error: añadiendo payload JSON a PDU (origen CAN)." ANSI_COLOR_RESET "\n", log_tag_param);
            coap_delete_pdu(pdu_to_central); // Libera PDU y su token interno.
            // El tracker CAN permanece en su array, no se libera con free.
            free(json_payload_str);
            return;
        }
    }

    // ---- Verificar estado de sesión antes de enviar ----
    coap_session_state_t session_state = coap_session_get_state(session_to_central);
    if (session_state != COAP_SESSION_STATE_ESTABLISHED) {
        LOG_ERROR_GW(ANSI_COLOR_RED "[%s] Error: Sesión DTLS no establecida (estado: %d). No se puede enviar petición." ANSI_COLOR_RESET "\n", log_tag_param, session_state);
        coap_delete_pdu(pdu_to_central);
        free(json_payload_str);
        return;
    }
    
    // ---- Enviar PDU ----
    LOG_INFO_GW(ANSI_COLOR_CYAN "[%s] Gateway (Origen CAN ID: 0x%X) -> Central: Enviando solicitud..." ANSI_COLOR_RESET "\n", log_tag_param, original_can_id);
    
    // Registrar petición CoAP en el logger
    char method[] = "POST";
    exec_logger_log_coap_sent(method, qualified_target_path, json_payload_str);
    
    free(json_payload_str); // Payload copiado a la PDU

    if (coap_send(session_to_central, pdu_to_central) == COAP_INVALID_MID) {
        LOG_ERROR_GW(ANSI_COLOR_RED "[%s] Error: enviando petición a servidor central (origen CAN)." ANSI_COLOR_RESET "\n", log_tag_param);
        // PDU es liberada por coap_send en error.
        // El tracker CAN permanece en su array, se sobrescribirá eventualmente.
        // La sesión DTLS global NO se libera aquí.
        // No hay 'tracker' dinámico que liberar aquí como en api_handlers.c: free(tracker); es INCORRECTO para can_bridge.
    } else {
        LOG_INFO_GW(ANSI_COLOR_GREEN "[%s] Gateway (Origen CAN ID: 0x%X) -> Central: Solicitud enviada, esperando rsp..." ANSI_COLOR_RESET "\n", log_tag_param, original_can_id);
        // El tracker CAN está almacenado. La respuesta se asociará a través del token.
        // La sesión DTLS global no se libera aquí.
    }
}

// Función específica para emergencias que llena todos los campos necesarios
static void forward_can_emergency_request_to_central_server(
    coap_context_t *ctx,
    uint32_t original_can_id,
    const char *central_server_path,
    const char *log_tag_param,
    const char *elevator_id,
    int emergency_floor,
    api_gateway_coap_para_sistema_de_ascensores_emergency_type__e emergency_type,
    const char *description,
    const char *timestamp
) {
    if (!central_server_path || !log_tag_param || !elevator_id || !description || !timestamp) {
        LOG_ERROR_GW("[CAN_Emergency] Error: Parámetros NULL en función de emergencia.");
        return;
    }
    if (!ctx) {
        LOG_ERROR_GW("[%s] Error: No se pudo obtener contexto CoAP.", log_tag_param);
        return;
    }

    LOG_INFO_GW("[%s] Gateway (Emergencia CAN ID: 0x%X): Preparando solicitud de emergencia para Servidor Central.", log_tag_param, original_can_id);

    // Crear estructura de detalles para emergencia
    api_request_details_for_json_t json_details;
    memset(&json_details, 0, sizeof(api_request_details_for_json_t));
    
    // Llenar campos específicos de emergencia
    strncpy(json_details.emergency_elevator_id, elevator_id, ID_STRING_MAX_LEN - 1);
    json_details.emergency_elevator_id[ID_STRING_MAX_LEN - 1] = '\0';
    json_details.emergency_floor = emergency_floor;
    strncpy(json_details.emergency_description, description, sizeof(json_details.emergency_description) - 1);
    json_details.emergency_description[sizeof(json_details.emergency_description) - 1] = '\0';
    strncpy(json_details.emergency_timestamp, timestamp, sizeof(json_details.emergency_timestamp) - 1);
    json_details.emergency_timestamp[sizeof(json_details.emergency_timestamp) - 1] = '\0';
    json_details.emergency_type = emergency_type;
    
    // Generar Payload JSON para emergencia
    char *json_payload_str = NULL;
    cJSON *json_payload_obj = elevator_group_to_json_for_server(&managed_elevator_group, GW_REQUEST_TYPE_EMERGENCY_CALL, &json_details);
    if (!json_payload_obj) {
        LOG_ERROR_GW("[%s] Error: Fallo al generar JSON para emergencia CAN.", log_tag_param);
        return;
    }
    json_payload_str = cJSON_PrintUnformatted(json_payload_obj);
    cJSON_Delete(json_payload_obj);
    if (!json_payload_str) {
        LOG_ERROR_GW("[%s] Error: Fallo al convertir JSON de emergencia a string.", log_tag_param);
        return;
    }
    LOG_DEBUG_GW("[%s] Payload de emergencia para Servidor Central (CAN ID: 0x%X): %s", log_tag_param, original_can_id, json_payload_str);

    // Sesión con el servidor central (DTLS)
    coap_session_t *session_to_central = get_or_create_central_server_dtls_session(ctx);
    if (!session_to_central) {
        LOG_ERROR_GW("[%s] Error creando/obteniendo sesión DTLS con servidor central para emergencia CAN.", log_tag_param);
        free(json_payload_str);
        return;
    }

    // Crear PDU para enviar al servidor central
    coap_pdu_t *pdu_to_central = coap_new_pdu(COAP_MESSAGE_CON, COAP_REQUEST_CODE_POST, session_to_central);
    if (!pdu_to_central) {
        LOG_ERROR_GW("[%s] Error creando PDU para servidor central (emergencia CAN).", log_tag_param);
        free(json_payload_str);
        return;
    }

    // Añadir Token NUEVO a la PDU para el servidor central
    uint8_t token_data[8]; 
    size_t token_length = sizeof(token_data);
    coap_session_new_token(session_to_central, &token_length, token_data);
    
    if (!coap_add_token(pdu_to_central, token_length, token_data)) {
         LOG_WARN_GW("[%s] Advertencia: Fallo al añadir NUEVO token a PDU (emergencia CAN).", log_tag_param);
    }
    coap_bin_const_t pdu_token_to_central = coap_pdu_get_token(pdu_to_central);

    // Guardar el tracker para esta solicitud de emergencia CAN
    store_can_tracker(pdu_token_to_central, original_can_id, GW_REQUEST_TYPE_EMERGENCY_CALL, emergency_floor, emergency_floor, elevator_id);

    // Añadir Opciones de URI (Uri-Path)
    char qualified_target_path[256];
    const char* path_to_use = central_server_path;
    
    if (path_to_use[0] == '/') {
        strncpy(qualified_target_path, path_to_use, sizeof(qualified_target_path) - 1);
        qualified_target_path[sizeof(qualified_target_path) - 1] = '\0';
    } else {
        snprintf(qualified_target_path, sizeof(qualified_target_path), "/%s", path_to_use);
    }

    coap_uri_t *uri_obj = coap_new_uri((const uint8_t *)qualified_target_path, strlen(qualified_target_path));
    if (uri_obj) {
        coap_optlist_t *optlist_head = NULL;
        const coap_address_t *remote_addr = coap_session_get_addr_remote(session_to_central);
        if (remote_addr) {
            if (coap_uri_into_optlist(uri_obj, remote_addr, &optlist_head, 1) == 1) {
                coap_add_optlist_pdu(pdu_to_central, &optlist_head);
                coap_delete_optlist(optlist_head);
            } else {
                LOG_ERROR_GW("[%s] Error creando lista de opciones desde URI (emergencia CAN): %s", log_tag_param, qualified_target_path);
            }
        } else {
            LOG_ERROR_GW("[%s] Error obteniendo dirección remota de la sesión DTLS para URI (emergencia CAN).", log_tag_param);
        }
        coap_delete_uri(uri_obj);
    } else {
         LOG_ERROR_GW("[%s] Error creando objeto URI desde path (emergencia CAN): %s", log_tag_param, qualified_target_path);
    }

    // Añadir Opción Content-Format (application/json)
    uint8_t ct_buf[2];
    coap_add_option(pdu_to_central, COAP_OPTION_CONTENT_FORMAT, 
                    coap_encode_var_safe(ct_buf, sizeof(ct_buf), COAP_MEDIATYPE_APPLICATION_JSON), ct_buf);

    // Añadir Payload JSON
    if (json_payload_str && strlen(json_payload_str) > 0) {
        if (!coap_add_data(pdu_to_central, strlen(json_payload_str), (const uint8_t *)json_payload_str)) {
            LOG_ERROR_GW("[%s] Error: añadiendo payload JSON a PDU (emergencia CAN).", log_tag_param);
            coap_delete_pdu(pdu_to_central);
            free(json_payload_str);
            return;
        }
    }

    // Verificar estado de sesión antes de enviar
    coap_session_state_t session_state = coap_session_get_state(session_to_central);
    if (session_state != COAP_SESSION_STATE_ESTABLISHED) {
        LOG_ERROR_GW("[%s] Error: Sesión DTLS no establecida (estado: %d). No se puede enviar petición de emergencia.", log_tag_param, session_state);
        coap_delete_pdu(pdu_to_central);
        free(json_payload_str);
        return;
    }
    
    // Enviar PDU
    LOG_INFO_GW("[%s] Gateway (Emergencia CAN ID: 0x%X) -> Central: Enviando solicitud de emergencia...", log_tag_param, original_can_id);
    
    // Registrar petición CoAP en el logger
    char method[] = "POST";
    exec_logger_log_coap_sent(method, qualified_target_path, json_payload_str);
    
    free(json_payload_str);

    if (coap_send(session_to_central, pdu_to_central) == COAP_INVALID_MID) {
        LOG_ERROR_GW("[%s] Error: enviando petición de emergencia a servidor central (CAN).", log_tag_param);
    } else {
        LOG_INFO_GW("[%s] Gateway (Emergencia CAN ID: 0x%X) -> Central: Solicitud de emergencia enviada, esperando respuesta...", log_tag_param, original_can_id);
    }
} 