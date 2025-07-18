/*
 * floor_call_request.h
 *
 * Solicitud de llamada de ascensor desde un piso.
 */

#ifndef _floor_call_request_H_
#define _floor_call_request_H_

#include <string.h>
#include "../external/cJSON.h"
#include "../include/list.h"
#include "../include/keyValuePair.h"
#include "../include/binary.h"

typedef struct floor_call_request_t floor_call_request_t;

#include "elevator_state.h"
#include "movement_direction.h"



typedef struct floor_call_request_t {
    char *id_edificio; // string
    int piso_origen_llamada; //numeric
    api_gateway_coap_para_sistema_de_ascensores_movement_direction__e direccion_llamada; //referenced enum
    list_t *elevadores_estado; //nonprimitive container

    int _library_owned; // Is the library responsible for freeing this object?
} floor_call_request_t;

__attribute__((deprecated)) floor_call_request_t *floor_call_request_create(
    char *id_edificio,
    int piso_origen_llamada,
    api_gateway_coap_para_sistema_de_ascensores_movement_direction__e direccion_llamada,
    list_t *elevadores_estado
);

void floor_call_request_free(floor_call_request_t *floor_call_request);

floor_call_request_t *floor_call_request_parseFromJSON(cJSON *floor_call_requestJSON);

cJSON *floor_call_request_convertToJSON(floor_call_request_t *floor_call_request);

#endif /* _floor_call_request_H_ */

