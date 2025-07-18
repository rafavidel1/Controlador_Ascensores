/*
 * emergency_type.h
 *
 * Tipos de emergencia que el sistema puede procesar 
 */

#ifndef _emergency_type_H_
#define _emergency_type_H_

#include <string.h>
#include "../external/cJSON.h"
#include "../include/list.h"
#include "../include/keyValuePair.h"
#include "../include/binary.h"

typedef struct emergency_type_t emergency_type_t;


// Enum  for emergency_type

typedef enum { api_gateway_coap_para_sistema_de_ascensores_emergency_type__NULL = 0, api_gateway_coap_para_sistema_de_ascensores_emergency_type__EMERGENCY_STOP, api_gateway_coap_para_sistema_de_ascensores_emergency_type__POWER_FAILURE, api_gateway_coap_para_sistema_de_ascensores_emergency_type__PEOPLE_TRAPPED, api_gateway_coap_para_sistema_de_ascensores_emergency_type__MECHANICAL_FAILURE, api_gateway_coap_para_sistema_de_ascensores_emergency_type__FIRE_ALARM } api_gateway_coap_para_sistema_de_ascensores_emergency_type__e;

char* emergency_type_emergency_type_ToString(api_gateway_coap_para_sistema_de_ascensores_emergency_type__e emergency_type);

api_gateway_coap_para_sistema_de_ascensores_emergency_type__e emergency_type_emergency_type_FromString(char* emergency_type);

cJSON *emergency_type_convertToJSON(api_gateway_coap_para_sistema_de_ascensores_emergency_type__e emergency_type);

api_gateway_coap_para_sistema_de_ascensores_emergency_type__e emergency_type_parseFromJSON(cJSON *emergency_typeJSON);

#endif /* _emergency_type_H_ */

