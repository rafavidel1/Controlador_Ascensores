/**
 * @file api_handlers.c
 * @brief Implementación de manejadores CoAP para el API Gateway
 * @author Sistema de Control de Ascensores
 * @date 2025
 * @version 2.0
 * 
 * Este archivo contiene la lógica para manejar:
 * - **Solicitudes entrantes**: Requests de clientes (ascensores) vía CoAP
 * - **Respuestas del servidor central**: Procesamiento de respuestas de asignación
 * - **Gestión de trackers**: Seguimiento de solicitudes pendientes
 * - **Manejo de señales**: Terminación elegante con SIGINT
 * - **Puente CAN-CoAP**: Integración con el sistema de comunicación CAN
 * 
 * Funcionalidades principales:
 * - Recepción de solicitudes de llamada de piso y cabina
 * - Reenvío de solicitudes al servidor central con estado de ascensores
 * - Gestión de sesiones DTLS-PSK para comunicación segura
 * - Tracking de solicitudes pendientes para correlación de respuestas
 * - Integración con el gestor de estado de ascensores
 * 
 * @see api_handlers.h
 * @see elevator_state_manager.h
 * @see can_bridge.h
 * @see coap_config.h
 */

#include <coap3/coap.h>       // Main libcoap header
#include <coap3/coap_pdu.h>   // PDU definitions
#include <coap3/coap_option.h> // Option definitions and helpers
#include <coap3/coap_uri.h>   // URI parsing functions like coap_uri_parse
#include <coap3/coap_session.h> // Include session definitions
#include <coap3/coap_net.h> // Include network/context definitions

#include "api_gateway/api_handlers.h" // Corrected path for Definiciones de nuestras funciones y estructuras
#include <api_gateway/coap_config.h> // Configuraciones de CoAP (IPs, puertos, etc.)
#include "api_gateway/elevator_state_manager.h" // Para gestionar estado y serializar a JSON
#include "api_gateway/dtls_common_config.h" // <--- AÑADIDO PARA DTLS PSK DEFINES

// NUEVA INCLUSIÓN PARA EL PUENTE CAN
#include "api_gateway/can_bridge.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <ctype.h> // Added for isprint
#include "api_gateway/execution_logger.h" // Sistema de logging de ejecuciones

/**
 * @brief Bandera para indicar si el bucle principal debe terminar
 * 
 * Esta variable se declara como extern en api_handlers.h y se define en main.c.
 * Es establecida por el manejador de señal (handle_sigint_gw) para detener
 * el bucle principal de eventos.
 * 
 * @see handle_sigint_gw()
 */
extern volatile sig_atomic_t quit_main_loop; // Se definirá en main.c

/**
 * @brief Estado del grupo de ascensores gestionado por este gateway
 * 
 * Declaración externa para acceder al estado del grupo de ascensores
 * gestionado en main.c. Contiene información completa de todos los
 * ascensores del edificio.
 * 
 * @see elevator_group_state_t
 * @see main.c
 */
extern elevator_group_state_t managed_elevator_group;

/**
 * @brief Manejador de señal para SIGINT (Ctrl+C)
 * @param signum Número de señal recibida (se espera SIGINT). No utilizado.
 * 
 * Establece la bandera quit_main_loop a 1 para señalar al bucle principal
 * de eventos que debe terminar, permitiendo una terminación elegante
 * del API Gateway.
 * 
 * Esta función es registrada como manejador de SIGINT en main.c y
 * proporciona una forma limpia de cerrar la aplicación.
 */
void handle_sigint_gw(int signum) {
    (void)signum;
    quit_main_loop = 1; 
}

// --- Inicio: Gestión de Sesión DTLS Global (declaración extern) ---
/**
 * @brief Sesión DTLS global con el servidor central
 * 
 * Declaración externa de la sesión DTLS definida en main.c.
 * Se utiliza para enviar solicitudes al servidor central de manera eficiente
 * reutilizando la misma conexión segura.
 * 
 * @see main.c
 * @see get_or_create_central_server_dtls_session()
 */
extern coap_session_t *g_dtls_session_to_central_server;
// --- Fin: Gestión de Sesión DTLS Global ---


// --- Inicio: Gestión de Trackers para solicitudes al Servidor Central ---

/**
 * @brief Número máximo de solicitudes pendientes al servidor central
 * 
 * Define el límite de solicitudes que pueden estar pendientes de respuesta
 * del servidor central simultáneamente. Ajustar según la capacidad del sistema.
 */
#define MAX_PENDING_REQUESTS_TO_CENTRAL 32

/**
 * @brief Entrada de tracker para solicitudes al servidor central
 * 
 * Estructura que asocia un token de solicitud enviada al servidor central
 * con los datos del tracker original que contiene información de la
 * solicitud inicial del ascensor.
 */
typedef struct {
    coap_bin_const_t token; ///< Token de la solicitud enviada AL SERVIDOR CENTRAL
    api_request_tracker_t *tracker_data; ///< Datos del tracker original (sesión del ascensor, token original, etc.)
} central_request_entry_t;

/**
 * @brief Array de solicitudes pendientes al servidor central
 * 
 * Almacena todas las solicitudes que han sido enviadas al servidor central
 * y están esperando respuesta. Se utiliza para correlacionar respuestas
 * con las solicitudes originales de los ascensores.
 */
static central_request_entry_t pending_central_requests[MAX_PENDING_REQUESTS_TO_CENTRAL];

/**
 * @brief Número actual de solicitudes pendientes al servidor central
 * 
 * Contador que mantiene el número de entradas activas en el array
 * pending_central_requests. Para un entorno multihilo, se necesitaría un mutex.
 */
static int num_pending_central_requests = 0;

/**
 * @brief Añade un tracker a la lista de solicitudes pendientes
 * @param token_to_central Token de la solicitud enviada al servidor central
 * @param tracker Datos del tracker original a asociar
 * 
 * Esta función almacena la asociación entre un token de solicitud al servidor
 * central y el tracker original que contiene información de la solicitud
 * del ascensor. Crea una copia del token para evitar problemas de memoria.
 * 
 * Si no hay espacio disponible o hay errores de memoria, libera automáticamente
 * el tracker para evitar fugas de memoria.
 * 
 * @see find_and_remove_central_request_tracker()
 */
static void add_central_request_tracker(coap_bin_const_t token_to_central, api_request_tracker_t *tracker) {
    if (num_pending_central_requests >= MAX_PENDING_REQUESTS_TO_CENTRAL) {
        LOG_ERROR_GW("[TrackerMgmt] Demasiadas solicitudes pendientes al servidor central. No se puede rastrear la nueva (tracker 0x%p).", (void*)tracker);
        // Liberar el tracker que no se puede almacenar para evitar fugas
        if (tracker) {
            if(tracker->original_token.s) coap_free((void*)tracker->original_token.s);
            if(tracker->log_tag) free(tracker->log_tag);
            if(tracker->original_elevator_session) coap_session_release(tracker->original_elevator_session);
            free(tracker);
        }
        return;
    }
    // Guardar una copia del token_to_central, ya que el original podría ser temporal
    uint8_t *token_copy_data = (uint8_t *)malloc(token_to_central.length);
    if (!token_copy_data) {
        LOG_ERROR_GW("[TrackerMgmt] Error al asignar memoria para la copia del token (tracker 0x%p).", (void*)tracker);
        if (tracker) { // Liberar tracker si no podemos copiar el token
            if(tracker->original_token.s) coap_free((void*)tracker->original_token.s);
            if(tracker->log_tag) free(tracker->log_tag);
            if(tracker->original_elevator_session) coap_session_release(tracker->original_elevator_session);
            free(tracker);
        }
        return;
    }
    memcpy(token_copy_data, token_to_central.s, token_to_central.length);
    
    pending_central_requests[num_pending_central_requests].token.length = token_to_central.length;
    pending_central_requests[num_pending_central_requests].token.s = token_copy_data; // Guardamos la copia
    pending_central_requests[num_pending_central_requests].tracker_data = tracker;
    num_pending_central_requests++;
    LOG_DEBUG_GW("[TrackerMgmt] Tracker (0x%p) añadido para token_to_central (len %zu). Pendientes: %d", 
                 (void*)tracker, token_to_central.length, num_pending_central_requests);
}

/**
 * @brief Encuentra y remueve un tracker de la lista de solicitudes pendientes
 * @param received_token Token recibido en la respuesta del servidor central
 * @return Puntero al tracker encontrado, o NULL si no se encuentra
 * 
 * Esta función busca en la lista de solicitudes pendientes un tracker
 * que corresponda al token recibido en una respuesta del servidor central.
 * Si lo encuentra, lo remueve de la lista y libera la memoria del token copiado.
 * 
 * La función compacta el array después de remover una entrada para mantener
 * la integridad de la estructura de datos. Es responsabilidad del llamador
 * liberar la memoria del tracker retornado.
 * 
 * @note En un entorno multihilo, esta función requeriría sincronización
 * @see add_central_request_tracker()
 */
static api_request_tracker_t* find_and_remove_central_request_tracker(coap_bin_const_t received_token) {
    for (int i = 0; i < num_pending_central_requests; i++) {
        if (coap_binary_equal(&pending_central_requests[i].token, &received_token)) {
            api_request_tracker_t *found_tracker = pending_central_requests[i].tracker_data;
            LOG_DEBUG_GW("[TrackerMgmt] Tracker (0x%p) encontrado para token (len %zu).", (void*)found_tracker, received_token.length);
            
            // Liberar la copia del token que almacenamos
            if (pending_central_requests[i].token.s) {
                free((void *)pending_central_requests[i].token.s); 
            }

            // Eliminar de la lista compactando (simple para array estático)
            for (int j = i; j < num_pending_central_requests - 1; j++) {
                pending_central_requests[j] = pending_central_requests[j + 1];
            }
            num_pending_central_requests--;
            LOG_DEBUG_GW("[TrackerMgmt] Tracker removido. Pendientes: %d", num_pending_central_requests);
            return found_tracker;
        }
    }
    // LOG_WARN_GW("[TrackerMgmt] No se encontró tracker para token (len %zu).", received_token.length);
    // Este WARN puede ser ruidoso si respuestas de CAN o send_arrival_update vienen aquí y no usan este sistema.
    return NULL;
}
// --- Fin: Gestión de Trackers ---

// ---- HELPER FUNCTION ----

/**
 * @brief Constructs and sends a CoAP request to the central server with the gateway's elevator group state.
 *
 * This function populates an api_request_tracker_t with details of the intended operation
 * (floor call, cabin request), serializes the current state of the managed elevator group to JSON,
 * and sends this as a CoAP request to the specified path on the central server.
 *
 * @param original_client_session The CoAP session from which the original request came (can be NULL if internally generated).
 * @param original_mid The CoAP MID from the original request (if applicable).
 * @param original_token_const The CoAP token from the original request (if applicable, will be duplicated).
 * @param central_server_path The target CoAP resource path on the central server.
 * @param log_tag_param A string tag for logging (e.g., "FloorCallGW", will be duplicated).
 * @param request_type_param The type of request being made (e.g., GW_REQUEST_TYPE_FLOOR_CALL).
 * @param origin_floor_param Floor of origin for a floor call.
 * @param target_floor_for_task_param The target floor for the task to be assigned by the server.
 * @param requesting_elevator_id_cabin_param ID of the elevator making a cabin request (can be NULL).
 * @param requested_direction_floor_param Direction for a floor call.
 */
static void
forward_request_to_central_server(
    coap_session_t *original_client_session, 
    coap_mid_t original_mid,
    coap_bin_const_t original_token_const,
    const char *central_server_path,
    const char *log_tag_param,
    gw_request_type_t request_type_param,
    int origin_floor_param,
    int target_floor_for_task_param,
    const char* requesting_elevator_id_cabin_param,
    movement_direction_enum_t requested_direction_floor_param
) {
    // ...Toda la implementación de esta función comentada
    // ... ya que los handlers que la usaban (hnd_cabin_request_from_elevator_gw, hnd_floor_call_from_elevator_gw)
    // ... han sido eliminados.
    // ... La lógica para enviar a central desde CAN está en can_bridge.c
    // ... La lógica para enviar /notificar_llegada está en main.c del gateway y construye su PDU.
}


// ---- RESPONSE HANDLER ----

/**
 * @brief Manejador de respuestas del servidor central
 * @param session_from_server Sesión CoAP desde la cual se recibió la respuesta
 * @param sent_to_central PDU que fue enviado al servidor central
 * @param received_from_central PDU recibido del servidor central
 * @param mid_from_server Message ID de la respuesta del servidor
 * @return COAP_RESPONSE_OK si la respuesta se procesó correctamente
 * 
 * Esta función procesa las respuestas recibidas del servidor central para
 * solicitudes de asignación de ascensores. Realiza las siguientes operaciones:
 * 
 * 1. **Validación**: Verifica que se recibió un PDU válido
 * 2. **Parsing JSON**: Extrae y parsea el payload JSON de la respuesta
 * 3. **Correlación**: Encuentra el tracker original usando el token
 * 4. **Procesamiento**: Extrae información de asignación (ascensor_asignado_id, tarea_id)
 * 5. **Notificación CAN**: Envía respuesta al controlador CAN correspondiente
 * 6. **Limpieza**: Libera recursos del tracker y memoria asociada
 * 
 * Maneja diferentes códigos de respuesta:
 * - 2.01 Created: Asignación exitosa
 * - 4.xx Client Error: Errores de solicitud
 * - 5.xx Server Error: Errores del servidor
 * 
 * @see add_central_request_tracker()
 * @see find_and_remove_central_request_tracker()
 * @see can_bridge.h
 */
coap_response_t
hnd_central_server_response_gw(coap_session_t *session_from_server,
                               const coap_pdu_t *sent_to_central,
                               const coap_pdu_t *received_from_central,
                               const coap_mid_t mid_from_server) {
    if (!received_from_central) {
        LOG_WARN_GW("[ResponseHandlerGW] Timeout o error: No se recibió PDU del Servidor Central.");
        // No podemos obtener token aquí para limpiar un tracker específico si no hay PDU.
        // Un sistema de timeouts para los trackers en pending_central_requests sería necesario para estos casos.
        // La sesión DTLS global se maneja por el event_handler.
        return COAP_RESPONSE_OK;
    }

    coap_pdu_code_t rcv_code = coap_pdu_get_code(received_from_central);
    LOG_INFO_GW("[ResponseHandlerGW] Servidor Central -> Gateway: Respuesta recibida (Code: %u.%02u). MID: %u",
                COAP_RESPONSE_CLASS(rcv_code), COAP_RESPONSE_CODE(rcv_code), mid_from_server);

    const uint8_t *data_from_central;
    size_t data_len_from_central;
    cJSON *json_response_from_central = NULL;

    if (coap_get_data(received_from_central, &data_len_from_central, &data_from_central)) {
        json_response_from_central = cJSON_ParseWithLength((const char*)data_from_central, data_len_from_central);
        if (!json_response_from_central) {
            LOG_WARN_GW("[ResponseHandlerGW] Payload de Servidor Central no es JSON válido o está vacío. Payload: %.*s", (int)data_len_from_central, data_from_central);
        } else {
            char* pretty_json = cJSON_Print(json_response_from_central);
            LOG_DEBUG_GW("[ResponseHandlerGW] JSON recibido de Servidor Central: %s", pretty_json);
            free(pretty_json);
        }
    } else {
        LOG_DEBUG_GW("[ResponseHandlerGW] Respuesta de Servidor Central no contenía payload.");
    }

    // Registrar respuesta CoAP en el logger
    char code_str[16];
    snprintf(code_str, sizeof(code_str), "%u.%02u", COAP_RESPONSE_CLASS(rcv_code), COAP_RESPONSE_CODE(rcv_code));
    if (data_from_central && data_len_from_central > 0) {
        char payload_str[256];
        size_t copy_len = data_len_from_central < sizeof(payload_str) - 1 ? data_len_from_central : sizeof(payload_str) - 1;
        memcpy(payload_str, data_from_central, copy_len);
        payload_str[copy_len] = '\0';
        exec_logger_log_coap_received(code_str, payload_str);
    } else {
        exec_logger_log_coap_received(code_str, NULL);
    }

    coap_bin_const_t received_token = coap_pdu_get_token(received_from_central);
    // Log del token recibido
    char token_hex_str_resp[17] = {0}; // Máx 8 bytes de token en hex + null
    if (received_token.s && received_token.length > 0) {
        size_t len_to_print_resp = received_token.length > 8 ? 8 : received_token.length;
        for (size_t i = 0; i < len_to_print_resp; ++i) {
            sprintf(&token_hex_str_resp[i * 2], "%02x", received_token.s[i]);
        }
        LOG_DEBUG_GW("[ResponseHandlerGW] Token recibido del servidor: (len %zu) %s", received_token.length, token_hex_str_resp);
    } else {
        LOG_DEBUG_GW("[ResponseHandlerGW] Token recibido del servidor: NULO o vacío.");
    }

    api_request_tracker_t *api_tracker = find_and_remove_central_request_tracker(received_token);
    can_origin_tracker_t *can_tracker = NULL;

    if (api_tracker) {
        const char* current_log_tag = api_tracker->log_tag ? api_tracker->log_tag : "ResponseHandlerGW_CoAP";
        LOG_DEBUG_GW("[%s] Tracker de API (0x%p) encontrado y removido para token %s.", current_log_tag, (void*)api_tracker, token_hex_str_resp);

        if (json_response_from_central) {
            cJSON *j_tarea_id = cJSON_GetObjectItemCaseSensitive(json_response_from_central, "tarea_id");
            cJSON *j_ascensor_asignado_id = cJSON_GetObjectItemCaseSensitive(json_response_from_central, "ascensor_asignado_id");

            if (cJSON_IsString(j_tarea_id) && j_tarea_id->valuestring != NULL &&
                cJSON_IsString(j_ascensor_asignado_id) && j_ascensor_asignado_id->valuestring != NULL) {
                LOG_INFO_GW("[%s] Servidor Central asignó tarea '%s' a ascensor '%s'. (CoAP Origin)", 
                            current_log_tag, j_tarea_id->valuestring, j_ascensor_asignado_id->valuestring);
                int call_reference_floor = (api_tracker->request_type == GW_REQUEST_TYPE_FLOOR_CALL) ? api_tracker->origin_floor : 0; 
                assign_task_to_elevator(&managed_elevator_group, 
                                        j_ascensor_asignado_id->valuestring, 
                                        j_tarea_id->valuestring, 
                                        api_tracker->target_floor_for_task, 
                                        call_reference_floor); 
            } else {
                LOG_WARN_GW("[%s] Respuesta JSON del servidor no contiene tarea_id o ascensor_asignado_id válidos. (CoAP Origin)", current_log_tag);
            }
        } else {
            LOG_WARN_GW("[%s] Respuesta del servidor sin payload JSON para asignación de tarea (CoAP Origin), o CoAP error sin JSON.", current_log_tag);
        }

        if (api_tracker->original_elevator_session) {
            coap_pdu_t *response_to_elevator = coap_new_pdu(coap_pdu_get_type(received_from_central), rcv_code, api_tracker->original_elevator_session);
            if (!response_to_elevator) {
                LOG_ERROR_GW(ANSI_COLOR_RED "[%s] Error: creando PDU para respuesta al cliente original CoAP." ANSI_COLOR_RESET "\n", current_log_tag);
            } else {
                coap_pdu_set_mid(response_to_elevator, api_tracker->original_mid);
                if (api_tracker->original_token.s && api_tracker->original_token.length > 0) {
                    if (!coap_add_token(response_to_elevator, api_tracker->original_token.length, api_tracker->original_token.s)) {
                         LOG_WARN_GW(ANSI_COLOR_YELLOW "[%s] Error: añadiendo token a PDU de respuesta al cliente CoAP." ANSI_COLOR_RESET "\n", current_log_tag);
                    }
                }
                coap_opt_iterator_t opt_iter_resp;
                coap_opt_t *option_resp;
                coap_option_iterator_init(received_from_central, &opt_iter_resp, COAP_OPT_ALL);
                while ((option_resp = coap_option_next(&opt_iter_resp))) {
                    if (!coap_add_option(response_to_elevator, opt_iter_resp.number, coap_opt_length(option_resp), coap_opt_value(option_resp))) { 
                        LOG_WARN_GW(ANSI_COLOR_YELLOW "[%s] Advertencia: No se pudo añadir opción %u a PDU de respuesta al cliente CoAP." ANSI_COLOR_RESET "\n", current_log_tag, opt_iter_resp.number);
                    }
                }
                if (data_from_central && data_len_from_central > 0) {
                    if (!coap_add_data(response_to_elevator, data_len_from_central, data_from_central)) {
                         LOG_ERROR_GW(ANSI_COLOR_RED "[%s] Error: añadiendo payload a PDU de respuesta al cliente CoAP." ANSI_COLOR_RESET "\n", current_log_tag);
                    }
                }
                LOG_INFO_GW(ANSI_COLOR_CYAN "[%s] Gateway -> Cliente Original CoAP: Reenviando respuesta del servidor..." ANSI_COLOR_RESET "\n", current_log_tag);
                if (coap_send(api_tracker->original_elevator_session, response_to_elevator) == COAP_INVALID_MID) {
                    LOG_ERROR_GW(ANSI_COLOR_RED "[%s] Error: enviando respuesta final al cliente original CoAP." ANSI_COLOR_RESET "\n", current_log_tag);
                } else {
                    LOG_INFO_GW(ANSI_COLOR_GREEN "[%s] Gateway -> Cliente Original CoAP: Respuesta reenviada exitosamente." ANSI_COLOR_RESET "\n", current_log_tag);
                }
            }
        } else {
             LOG_WARN_GW(ANSI_COLOR_YELLOW "[%s] API Tracker (0x%p) no tiene sesión original de ascensor. No se puede reenviar respuesta." ANSI_COLOR_RESET "\n", current_log_tag, (void*)api_tracker);
        }
        
        // Clean up the API tracker (ya fue removido de la lista)
        if(api_tracker->original_token.s) coap_free((void*)api_tracker->original_token.s);
        if(api_tracker->log_tag) free(api_tracker->log_tag);
        if(api_tracker->original_elevator_session) coap_session_release(api_tracker->original_elevator_session);
        free(api_tracker);

    } else {
        // No se encontró API tracker. Intentar buscar tracker de CAN.
        can_tracker = find_can_tracker(received_token); // find_can_tracker NO remueve de su lista

        if (can_tracker) {
            LOG_INFO_GW("[ResponseHandlerGW] Respuesta CoAP corresponde a una solicitud originada por CAN (ID: 0x%X). Token %s", can_tracker->original_can_id, token_hex_str_resp);
            
            bool is_success_code_class = (COAP_RESPONSE_CLASS(rcv_code) == 2);
            if (is_success_code_class && json_response_from_central) {
                cJSON *j_tarea_id = cJSON_GetObjectItemCaseSensitive(json_response_from_central, "tarea_id");
                cJSON *j_ascensor_asignado_id = cJSON_GetObjectItemCaseSensitive(json_response_from_central, "ascensor_asignado_id");

                if (cJSON_IsString(j_tarea_id) && j_tarea_id->valuestring != NULL &&
                    cJSON_IsString(j_ascensor_asignado_id) && j_ascensor_asignado_id->valuestring != NULL) {
                    
                    LOG_INFO_GW("[ResponseHandlerGW] Servidor Central (vía CAN origin) asignó tarea '%s' a ascensor '%s'. Actualizando estado local.", 
                                j_tarea_id->valuestring, j_ascensor_asignado_id->valuestring);

                    assign_task_to_elevator(&managed_elevator_group, 
                                            j_ascensor_asignado_id->valuestring, 
                                            j_tarea_id->valuestring, 
                                            can_tracker->target_floor_for_task, 
                                            can_tracker->call_reference_floor); 
                } else {
                    LOG_WARN_GW("[ResponseHandlerGW] Respuesta JSON (vía CAN origin) del servidor no contiene tarea_id o ascensor_asignado_id válidos para actualizar estado.");
                }
            }
            ag_can_bridge_send_response_frame(can_tracker->original_can_id, rcv_code, json_response_from_central);
            // El tracker CAN no se libera aquí, es parte de un buffer circular.
        } else {
            // No es un tracker de API y no es un tracker de CAN.
            // Podría ser una respuesta a una solicitud autoiniciada por el gateway (ej. send_arrival_update)
            // o una respuesta inesperada/tardía.
            LOG_WARN_GW("[ResponseHandlerGW] Respuesta del servidor con token %s no corresponde a ningún tracker conocido (ni API ni CAN). MID: %u. Quizás de send_arrival_update?", token_hex_str_resp, mid_from_server);
        }
    }
    
    if(json_response_from_central) {
        cJSON_Delete(json_response_from_central);
    }

    // Gestión de la sesión (NO liberar la sesión global aquí)
    // extern coap_session_t *g_dtls_session_to_central_server; // Ya está declarada arriba o en un scope visible
    if (session_from_server != g_dtls_session_to_central_server && session_from_server != NULL) {
         LOG_WARN_GW("[ResponseHandlerGW] La sesión de respuesta (0x%p) no es la global (0x%p). Liberándola.", (void*)session_from_server, (void*)g_dtls_session_to_central_server);
         coap_session_release(session_from_server);
    } else if (session_from_server == g_dtls_session_to_central_server) {
         LOG_DEBUG_GW("[ResponseHandlerGW] Respuesta recibida en la sesión DTLS global (0x%p). No se libera aquí.", (void*)session_from_server);
    }

    return COAP_RESPONSE_OK;
}


// ---- RESOURCE HANDLERS ---- 
// (Minor log changes in these handlers, mainly adding color/context)

void
hnd_elevator_api_request_gw(coap_resource_t *resource,
                            coap_session_t *elevator_session,
                            const coap_pdu_t *elevator_request_pdu,
                            const coap_string_t *query,
                            coap_pdu_t *response_placeholder) {
    // ... Implementación comentada
}

void
hnd_cabin_request_from_elevator_gw(coap_resource_t *resource,
                                   coap_session_t *elevator_session,
                                   const coap_pdu_t *elevator_request_pdu,
                                   const coap_string_t *query,
                                   coap_pdu_t *response_placeholder) {
    // ... Implementación comentada
}

void
hnd_floor_call_from_elevator_gw(coap_resource_t *resource,
                                  coap_session_t *elevator_session, 
                                  const coap_pdu_t *elevator_request_pdu, 
                                  const coap_string_t *query, 
                                  coap_pdu_t *response_placeholder) { 
    // ... Implementación comentada
}

