/*
 * emergency_response.h
 *
 * Respuesta a una solicitud de emergencia
 */

#ifndef _emergency_response_H_
#define _emergency_response_H_

#include <string.h>
#include "../external/cJSON.h"
#include "../include/list.h"
#include "../include/keyValuePair.h"
#include "../include/binary.h"

typedef struct emergency_response_t emergency_response_t;


// Enum PROTOCOLOACTIVADO for emergency_response

typedef enum  { api_gateway_coap_para_sistema_de_ascensores_emergency_response_PROTOCOLOACTIVADO_NULL = 0, api_gateway_coap_para_sistema_de_ascensores_emergency_response_PROTOCOLOACTIVADO_RESCUE_PROTOCOL, api_gateway_coap_para_sistema_de_ascensores_emergency_response_PROTOCOLOACTIVADO_MAINTENANCE_ALERT, api_gateway_coap_para_sistema_de_ascensores_emergency_response_PROTOCOLOACTIVADO_FIRE_EVACUATION, api_gateway_coap_para_sistema_de_ascensores_emergency_response_PROTOCOLOACTIVADO_POWER_BACKUP } api_gateway_coap_para_sistema_de_ascensores_emergency_response_PROTOCOLOACTIVADO_e;

char* emergency_response_protocolo_activado_ToString(api_gateway_coap_para_sistema_de_ascensores_emergency_response_PROTOCOLOACTIVADO_e protocolo_activado);

api_gateway_coap_para_sistema_de_ascensores_emergency_response_PROTOCOLOACTIVADO_e emergency_response_protocolo_activado_FromString(char* protocolo_activado);



typedef struct emergency_response_t {
    char *emergencia_id; // string
    api_gateway_coap_para_sistema_de_ascensores_emergency_response_PROTOCOLOACTIVADO_e protocolo_activado; //enum
    int tiempo_respuesta_estimado; //numeric
    list_t *servicios_notificados; //primitive container
    list_t *ascensores_redirection; //primitive container

    int _library_owned; // Is the library responsible for freeing this object?
} emergency_response_t;

__attribute__((deprecated)) emergency_response_t *emergency_response_create(
    char *emergencia_id,
    api_gateway_coap_para_sistema_de_ascensores_emergency_response_PROTOCOLOACTIVADO_e protocolo_activado,
    int tiempo_respuesta_estimado,
    list_t *servicios_notificados,
    list_t *ascensores_redirection
);

void emergency_response_free(emergency_response_t *emergency_response);

emergency_response_t *emergency_response_parseFromJSON(cJSON *emergency_responseJSON);

cJSON *emergency_response_convertToJSON(emergency_response_t *emergency_response);

#endif /* _emergency_response_H_ */

