#ifndef API_HANDLERS_H
#define API_HANDLERS_H

#include <coap3/coap.h>                 // For coap_session_t, coap_pdu_t, etc.
#include <signal.h>                     // For sig_atomic_t
#include "api_common_defs.h" // Ensuring this is just the filename
#include "elevator_state_manager.h" // For movement_direction_enum_t, and gw_request_type_t now moved here

// typedef enum gw_request_type_t is MOVED to elevator_state_manager.h

/**
 * @brief Structure to track client requests forwarded to the central server.
 *
 * This structure holds information about an original client request that the API Gateway
 * has forwarded to the central server. It is used to correlate the server's response
 * back to the original client session and to manage state or context related to the request.
 */
typedef struct api_request_tracker_t {
    coap_session_t *original_elevator_session;  /**< Session of the original elevator client. */
    coap_mid_t original_mid;                    /**< CoAP Message ID of the original request. */
    coap_binary_t original_token;               /**< CoAP token of the original request (dynamically allocated). */
    char *log_tag;                              /**< Tag for logging purposes (dynamically allocated). */

    // New fields for detailed request tracking
    gw_request_type_t request_type;             /**< Type of the original request. */
    int origin_floor;                           /**< For floor calls: the floor where the call originated. */
    int target_floor_for_task;                  /**< The floor the assigned elevator should go to. */
    char requesting_elevator_id_cabin[ID_STRING_MAX_LEN]; /**< For cabin requests: ID of the elevator. */
    movement_direction_enum_t requested_direction_floor; /**< For floor calls: UP/DOWN. */

    // Potentially other fields like timestamp, retry count, etc.
} api_request_tracker_t;

// Forward declaration de coap_session_t y coap_context_t para la función helper
// Esto evita tener que incluir coap_session.h y coap_net.h aquí si no son necesarios para otras declaraciones.
struct coap_session_t;
struct coap_context_t;

// Declaración de la función helper para la gestión de sesiones DTLS (definida en main.c)
coap_session_t* get_or_create_central_server_dtls_session(struct coap_context_t *ctx);

// --- Signal Handler ---
void handle_sigint_gw(int signum);

// --- Response Handler (for responses FROM Central Server TO Gateway) ---
coap_response_t hnd_central_server_response_gw(coap_session_t *session_to_central,
                                             const coap_pdu_t *sent_to_central,
                                             const coap_pdu_t *received_from_central,
                                             const coap_mid_t mid_to_central);

// --- Resource Handlers (for requests FROM Clients TO Gateway) ---

/**
 * @brief Handler for legacy elevator API requests (e.g., /peticion_ascensor).
 * Currently logs and discards such requests.
 */
void hnd_elevator_api_request_gw(coap_resource_t *resource,
                               coap_session_t *elevator_session,
                               const coap_pdu_t *elevator_request_pdu,
                               const coap_string_t *query,
                               coap_pdu_t *response_placeholder);

/**
 * @brief Handler for cabin requests from elevator clients.
 * Path: GW_CABIN_REQUEST_PATH (e.g., /solicitud_cabina_gw)
 * Parses query params: elevator_id, target_floor
 */
void hnd_cabin_request_from_elevator_gw(coap_resource_t *resource,
                                      coap_session_t *elevator_session,
                                      const coap_pdu_t *elevator_request_pdu,
                                      const coap_string_t *query,
                                      coap_pdu_t *response_placeholder);

/**
 * @brief Handler for floor calls from elevator clients/external.
 * Path: GW_FLOOR_CALL_PATH (e.g., /llamada_piso_gw)
 * Parses query params: floor, dir (UP/DOWN)
 */
void hnd_floor_call_from_elevator_gw(coap_resource_t *resource,
                                     coap_session_t *elevator_session, 
                                     const coap_pdu_t *elevator_request_pdu, 
                                     const coap_string_t *query, 
                                     coap_pdu_t *response_placeholder);

// Note: hnd_arrival_notification_from_elevator_gw has been removed as the gateway
// now simulates arrivals and sends its own notifications to the central server.

extern volatile sig_atomic_t quit_main_loop; // Defined in main.c

#endif // API_HANDLERS_H 