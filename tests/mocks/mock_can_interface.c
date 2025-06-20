#include "mock_can_interface.h"
#include <stdlib.h>
#include <string.h>
#include "api_gateway/elevator_state_manager.h"

// Variables globales del mock CAN
static simulated_can_frame_t sent_frames[MAX_MOCK_CAN_FRAMES];
static int sent_frame_count = 0;
static simulated_can_frame_t received_frames[MAX_MOCK_CAN_FRAMES];
static int received_frame_count = 0;
static bool can_should_fail = false;

// Mock de la variable global managed_elevator_group
elevator_group_state_t managed_elevator_group;

// Mock de la función get_or_create_central_server_dtls_session
coap_session_t* get_or_create_central_server_dtls_session(coap_context_t *ctx) {
    // Para las pruebas, simplemente retornamos un puntero mock
    return (coap_session_t*)0x12345678; // Puntero mock
}

// Implementación del mock
void mock_can_send_frame(simulated_can_frame_t* frame) {
    if (can_should_fail || !frame) {
        return;
    }
    
    if (sent_frame_count < MAX_MOCK_CAN_FRAMES) {
        sent_frames[sent_frame_count] = *frame;
        sent_frame_count++;
    }
}

bool mock_can_receive_frame(simulated_can_frame_t* frame) {
    if (can_should_fail || !frame || received_frame_count == 0) {
        return false;
    }
    
    *frame = received_frames[0];
    
    // Mover frames restantes hacia adelante
    for (int i = 1; i < received_frame_count; i++) {
        received_frames[i-1] = received_frames[i];
    }
    received_frame_count--;
    
    return true;
}

// Funciones de control del mock
void mock_can_reset(void) {
    memset(sent_frames, 0, sizeof(sent_frames));
    memset(received_frames, 0, sizeof(received_frames));
    sent_frame_count = 0;
    received_frame_count = 0;
    can_should_fail = false;
    
    // Inicializar el mock del grupo de ascensores
    memset(&managed_elevator_group, 0, sizeof(elevator_group_state_t));
    strcpy(managed_elevator_group.edificio_id_str_grupo, "E1");
    managed_elevator_group.num_elevadores_en_grupo = 4;
    
    // Inicializar ascensores mock
    for (int i = 0; i < 4; i++) {
        elevator_status_t *elevator = &managed_elevator_group.ascensores[i];
        snprintf(elevator->ascensor_id, sizeof(elevator->ascensor_id), "E1A%d", i + 1);
        snprintf(elevator->id_edificio_str, sizeof(elevator->id_edificio_str), "E1");
        elevator->piso_actual = 1;
        elevator->ocupado = false;
        elevator->estado_puerta_enum = DOOR_CLOSED;
        elevator->direccion_movimiento_enum = STOPPED;
        elevator->destino_actual = -1;
        strcpy(elevator->tarea_actual_id, "");
    }
}

void mock_can_set_fail_mode(bool should_fail) {
    can_should_fail = should_fail;
}

void mock_can_queue_received_frame(simulated_can_frame_t* frame) {
    if (frame && received_frame_count < MAX_MOCK_CAN_FRAMES) {
        received_frames[received_frame_count] = *frame;
        received_frame_count++;
    }
}

simulated_can_frame_t* mock_can_get_sent_frame(int index) {
    if (index >= 0 && index < sent_frame_count) {
        return &sent_frames[index];
    }
    return NULL;
}

int mock_can_get_sent_frame_count(void) {
    return sent_frame_count;
}

int mock_can_get_received_frame_count(void) {
    return received_frame_count;
}

// Funciones helper para crear frames de prueba
simulated_can_frame_t mock_can_create_floor_call_frame(int floor, int direction) {
    simulated_can_frame_t frame = {
        .id = 0x100,
        .dlc = 2,
        .data = {floor, direction}
    };
    return frame;
}

simulated_can_frame_t mock_can_create_cabin_request_frame(int elevator_idx, int target_floor) {
    simulated_can_frame_t frame = {
        .id = 0x200,
        .dlc = 2,
        .data = {elevator_idx, target_floor}
    };
    return frame;
}

simulated_can_frame_t mock_can_create_arrival_frame(int elevator_idx, int current_floor) {
    simulated_can_frame_t frame = {
        .id = 0x300,
        .dlc = 2,
        .data = {elevator_idx, current_floor}
    };
    return frame;
} 