/*
 * cabin_request.h
 *
 * Solicitud de destino desde la cabina de un ascensor.
 */

#ifndef _cabin_request_H_
#define _cabin_request_H_

#include <string.h>
#include "../external/cJSON.h"
#include "../include/list.h"
#include "../include/keyValuePair.h"
#include "../include/binary.h"

typedef struct cabin_request_t cabin_request_t;

#include "elevator_state.h"



typedef struct cabin_request_t {
    char *id_edificio; // string
    char *solicitando_ascensor_id; // string
    int piso_destino_solicitud; //numeric
    list_t *elevadores_estado; //nonprimitive container

    int _library_owned; // Is the library responsible for freeing this object?
} cabin_request_t;

__attribute__((deprecated)) cabin_request_t *cabin_request_create(
    char *id_edificio,
    char *solicitando_ascensor_id,
    int piso_destino_solicitud,
    list_t *elevadores_estado
);

void cabin_request_free(cabin_request_t *cabin_request);

cabin_request_t *cabin_request_parseFromJSON(cJSON *cabin_requestJSON);

cJSON *cabin_request_convertToJSON(cabin_request_t *cabin_request);

#endif /* _cabin_request_H_ */

