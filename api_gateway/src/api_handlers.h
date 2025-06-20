/**
 * @file api_handlers.h
 * @brief Declares CoAP handler functions and related data structures for the API Gateway.
 *
 * This file provides the interface for the CoAP NACK/response handler for client requests made by the gateway.
 */
#ifndef API_GATEWAY_API_HANDLERS_H
#define API_GATEWAY_API_HANDLERS_H

#include <coap3/coap.h>
#include "api_gateway/coap_config.h" // Asegurar que coap_config.h esté incluido
#include "api_gateway/elevator_state_manager.h" // Para enums y ID_STRING_MAX_LEN

// Resource paths the API Gateway would listen on - REMOVED FOR SIMPLIFICATION
// #define GW_ELEVATOR_LEGACY_PATH "peticion_ascensor"
// #define GW_CABIN_REQUEST_PATH "cabin_request_gw"
// #define GW_FLOOR_CALL_PATH "floor_call_gw"
// #define GW_ARRIVAL_NOTIFICATION_PATH "arrival_notification_gw" // Not used

// Tipo de solicitud originada/procesada por el Gateway
typedef enum {
    GW_REQUEST_TYPE_UNKNOWN,
    GW_REQUEST_TYPE_FLOOR_CALL,
    GW_REQUEST_TYPE_CABIN_REQUEST
} gw_request_type_t;

/**
 * @brief Tracks information from an original client (elevator) request OR a gateway-originated request.
 *
 * This structure is used to maintain state across the two-stage request process:
 * 1. Event (simulated client or internal) -> Gateway
 * 2. Gateway -> Central Server
 * It allows the Gateway to correctly respond to the original client (if any) or update its internal state
 * after receiving a response from the Central Server.
 */
typedef struct {
    coap_session_t *original_elevator_session; /**< The CoAP session with the original elevator client (NULL if gateway-originated internal event). */
    coap_binary_t original_token;              /**< The CoAP token from the original client's request (duplicated). */
    coap_mid_t original_mid;                   /**< The CoAP Message ID from the original client's request. */
    char *log_tag;                             /**< Tag for logging (e.g., "FloorCall", duplicated). */

    // Nuevos campos para la lógica de gestión de estado del Gateway
    gw_request_type_t request_type;            /**< Tipo de solicitud gestionada por el gateway. */
    int origin_floor;                          /**< Piso origen (para GW_REQUEST_TYPE_FLOOR_CALL). */
    int target_floor_for_task;                 /**< Piso destino para la tarea a asignar (puede ser piso_origen para floor_call o destino_cabina para cabin_request). */
    char requesting_elevator_id_cabin[ID_STRING_MAX_LEN]; /**< ID del ascensor (para GW_REQUEST_TYPE_CABIN_REQUEST). */
    movement_direction_enum_t requested_direction_floor; /**< Dirección solicitada (para GW_REQUEST_TYPE_FLOOR_CALL). */

} api_request_tracker_t;

/**
 * @brief Signal handler for SIGINT (Ctrl+C) to allow graceful shutdown.
 *
 * @param signum The signal number (unused).
 */
void handle_sigint_gw(int signum);

/**
 * @brief Handles responses received from the Central Server.
 *
 * This function is registered as a CoAP NACK/response handler for requests made by the API Gateway
 * (acting as a client) to the Central Server. It processes the Central Server's response
 * and forwards it to the original elevator client (if applicable) or updates internal state (e.g. for CAN originated).
 *
 * @param session_to_central The CoAP session between the API Gateway and the Central Server.
 * @param sent_to_central The PDU that the API Gateway sent to the Central Server (can be NULL).
 * @param received_from_central The PDU received from the Central Server.
 * @param mid_to_central The CoAP Message ID of the exchange with the Central Server.
 * @return COAP_RESPONSE_OK to indicate the response was handled.
 */
coap_response_t
hnd_central_server_response_gw(coap_session_t *session_to_central,
                               const coap_pdu_t *sent_to_central,
                               const coap_pdu_t *received_from_central,
                               const coap_mid_t mid_to_central);

// HANDLERS FOR GATEWAY'S OWN RESOURCES - REMOVED FOR SIMPLIFICATION
/*
void
hnd_elevator_api_request_gw(coap_resource_t *resource,
                            coap_session_t *elevator_session,
                            const coap_pdu_t *elevator_request_pdu,
                            const coap_string_t *query,
                            coap_pdu_t *response_placeholder);

void
hnd_cabin_request_from_elevator_gw(coap_resource_t *resource,
                                   coap_session_t *elevator_session,
                                   const coap_pdu_t *elevator_request_pdu,
                                   const coap_string_t *query,
                                   coap_pdu_t *response_placeholder);

void
hnd_floor_call_from_elevator_gw(coap_resource_t *resource,
                                  coap_session_t *elevator_session,
                                  const coap_pdu_t *elevator_request_pdu,
                                  const coap_string_t *query,
                                  coap_pdu_t *response_placeholder);

void
hnd_arrival_notification_from_elevator_gw(coap_resource_t *resource,
                                          coap_session_t *elevator_session,
                                          const coap_pdu_t *elevator_request_pdu,
                                          const coap_string_t *query,
                                          coap_pdu_t *response_placeholder);
*/

#endif // API_GATEWAY_API_HANDLERS_H 