/*
 * server_response.h
 *
 * Respuesta del servidor central con asignación de ascensor.
 */

#ifndef _server_response_H_
#define _server_response_H_

#include <string.h>
#include "../external/cJSON.h"
#include "../include/list.h"
#include "../include/keyValuePair.h"
#include "../include/binary.h"

typedef struct server_response_t server_response_t;




typedef struct server_response_t {
    char *ascensor_asignado_id; // string
    char *tarea_id; // string
    int piso_destino_asignado; //numeric
    int tiempo_estimado_llegada; //numeric

    int _library_owned; // Is the library responsible for freeing this object?
} server_response_t;

__attribute__((deprecated)) server_response_t *server_response_create(
    char *ascensor_asignado_id,
    char *tarea_id,
    int piso_destino_asignado,
    int tiempo_estimado_llegada
);

void server_response_free(server_response_t *server_response);

server_response_t *server_response_parseFromJSON(cJSON *server_responseJSON);

cJSON *server_response_convertToJSON(server_response_t *server_response);

#endif /* _server_response_H_ */

