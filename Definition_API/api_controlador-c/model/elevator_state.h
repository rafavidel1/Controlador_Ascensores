/*
 * elevator_state.h
 *
 * Estado completo de un ascensor individual.
 */

#ifndef _elevator_state_H_
#define _elevator_state_H_

#include <string.h>
#include "../external/cJSON.h"
#include "../include/list.h"
#include "../include/keyValuePair.h"
#include "../include/binary.h"

typedef struct elevator_state_t elevator_state_t;

#include "door_state.h"



typedef struct elevator_state_t {
    char *id_ascensor; // string
    int piso_actual; //numeric
    api_gateway_coap_para_sistema_de_ascensores_door_state__e estado_puerta; //referenced enum
    int disponible; //boolean
    char *tarea_actual_id; // string
    int destino_actual; //numeric

    int _library_owned; // Is the library responsible for freeing this object?
} elevator_state_t;

__attribute__((deprecated)) elevator_state_t *elevator_state_create(
    char *id_ascensor,
    int piso_actual,
    api_gateway_coap_para_sistema_de_ascensores_door_state__e estado_puerta,
    int disponible,
    char *tarea_actual_id,
    int destino_actual
);

void elevator_state_free(elevator_state_t *elevator_state);

elevator_state_t *elevator_state_parseFromJSON(cJSON *elevator_stateJSON);

cJSON *elevator_state_convertToJSON(elevator_state_t *elevator_state);

#endif /* _elevator_state_H_ */

