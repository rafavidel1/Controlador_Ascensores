#ifndef MOCK_CAN_INTERFACE_H
#define MOCK_CAN_INTERFACE_H

#include "api_gateway/can_bridge.h"
#include "api_gateway/elevator_state_manager.h"
#include <coap3/coap.h>
#include <stdbool.h>

#define MAX_MOCK_CAN_FRAMES 50

// Declaraciones de variables globales mock
extern elevator_group_state_t managed_elevator_group;

// Declaraciones de funciones mock
coap_session_t* get_or_create_central_server_dtls_session(coap_context_t *ctx);

// Funciones mock para interfaz CAN
void mock_can_send_frame(simulated_can_frame_t* frame);
bool mock_can_receive_frame(simulated_can_frame_t* frame);

// Funciones de control del mock
void mock_can_reset(void);
void mock_can_set_fail_mode(bool should_fail);
void mock_can_queue_received_frame(simulated_can_frame_t* frame);

// Funciones de inspección
simulated_can_frame_t* mock_can_get_sent_frame(int index);
int mock_can_get_sent_frame_count(void);
int mock_can_get_received_frame_count(void);

// Funciones helper para crear frames de prueba
simulated_can_frame_t mock_can_create_floor_call_frame(int floor, int direction);
simulated_can_frame_t mock_can_create_cabin_request_frame(int elevator_idx, int target_floor);
simulated_can_frame_t mock_can_create_arrival_frame(int elevator_idx, int current_floor);

#endif // MOCK_CAN_INTERFACE_H 