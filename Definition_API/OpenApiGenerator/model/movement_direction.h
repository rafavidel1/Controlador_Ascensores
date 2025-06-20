/*
 * movement_direction.h
 *
 * Dirección de movimiento del ascensor: - SUBIENDO: Ascensor moviéndose hacia pisos superiores - BAJANDO: Ascensor moviéndose hacia pisos inferiores   - PARADO: Ascensor detenido en un piso 
 */

#ifndef _movement_direction_H_
#define _movement_direction_H_

#include <string.h>
#include "../external/cJSON.h"
#include "../include/list.h"
#include "../include/keyValuePair.h"
#include "../include/binary.h"

typedef struct movement_direction_t movement_direction_t;


// Enum  for movement_direction

typedef enum { api_gateway_coap_para_sistema_de_ascensores_movement_direction__NULL = 0, api_gateway_coap_para_sistema_de_ascensores_movement_direction__SUBIENDO, api_gateway_coap_para_sistema_de_ascensores_movement_direction__BAJANDO, api_gateway_coap_para_sistema_de_ascensores_movement_direction__PARADO } api_gateway_coap_para_sistema_de_ascensores_movement_direction__e;

char* movement_direction_movement_direction_ToString(api_gateway_coap_para_sistema_de_ascensores_movement_direction__e movement_direction);

api_gateway_coap_para_sistema_de_ascensores_movement_direction__e movement_direction_movement_direction_FromString(char* movement_direction);

cJSON *movement_direction_convertToJSON(api_gateway_coap_para_sistema_de_ascensores_movement_direction__e movement_direction);

api_gateway_coap_para_sistema_de_ascensores_movement_direction__e movement_direction_parseFromJSON(cJSON *movement_directionJSON);

#endif /* _movement_direction_H_ */

