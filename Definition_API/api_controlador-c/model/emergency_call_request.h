/*
 * emergency_call_request.h
 *
 * Solicitud de emergencia desde ascensor
 */

#ifndef _emergency_call_request_H_
#define _emergency_call_request_H_

#include <string.h>
#include "../external/cJSON.h"
#include "../include/list.h"
#include "../include/keyValuePair.h"
#include "../include/binary.h"

typedef struct emergency_call_request_t emergency_call_request_t;

#include "elevator_state.h"
#include "emergency_type.h"



typedef struct emergency_call_request_t {
    char *id_edificio; // string
    char *ascensor_id_emergencia; // string
    api_gateway_coap_para_sistema_de_ascensores_emergency_type__e tipo_emergencia; //referenced enum
    int piso_actual_emergencia; //numeric
    char *descripcion_emergencia; // string
    char *timestamp_emergencia; //date time
    list_t *elevadores_estado; //nonprimitive container

    int _library_owned; // Is the library responsible for freeing this object?
} emergency_call_request_t;

__attribute__((deprecated)) emergency_call_request_t *emergency_call_request_create(
    char *id_edificio,
    char *ascensor_id_emergencia,
    api_gateway_coap_para_sistema_de_ascensores_emergency_type__e tipo_emergencia,
    int piso_actual_emergencia,
    char *descripcion_emergencia,
    char *timestamp_emergencia,
    list_t *elevadores_estado
);

void emergency_call_request_free(emergency_call_request_t *emergency_call_request);

emergency_call_request_t *emergency_call_request_parseFromJSON(cJSON *emergency_call_requestJSON);

cJSON *emergency_call_request_convertToJSON(emergency_call_request_t *emergency_call_request);

#endif /* _emergency_call_request_H_ */

