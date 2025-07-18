/*
 * door_state.h
 *
 * Estado actual de las puertas del ascensor: - CERRADA: Puertas completamente cerradas y bloqueadas - ABIERTA: Puertas completamente abiertas - ABRIENDO: Puertas en proceso de apertura - CERRANDO: Puertas en proceso de cierre 
 */

#ifndef _door_state_H_
#define _door_state_H_

#include <string.h>
#include "../external/cJSON.h"
#include "../include/list.h"
#include "../include/keyValuePair.h"
#include "../include/binary.h"

typedef struct door_state_t door_state_t;


// Enum  for door_state

typedef enum { api_gateway_coap_para_sistema_de_ascensores_door_state__NULL = 0, api_gateway_coap_para_sistema_de_ascensores_door_state__CERRADA, api_gateway_coap_para_sistema_de_ascensores_door_state__ABIERTA, api_gateway_coap_para_sistema_de_ascensores_door_state__ABRIENDO, api_gateway_coap_para_sistema_de_ascensores_door_state__CERRANDO } api_gateway_coap_para_sistema_de_ascensores_door_state__e;

char* door_state_door_state_ToString(api_gateway_coap_para_sistema_de_ascensores_door_state__e door_state);

api_gateway_coap_para_sistema_de_ascensores_door_state__e door_state_door_state_FromString(char* door_state);

cJSON *door_state_convertToJSON(api_gateway_coap_para_sistema_de_ascensores_door_state__e door_state);

api_gateway_coap_para_sistema_de_ascensores_door_state__e door_state_parseFromJSON(cJSON *door_stateJSON);

#endif /* _door_state_H_ */

