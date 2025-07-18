#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "emergency_type.h"


char* emergency_type_emergency_type_ToString(api_gateway_coap_para_sistema_de_ascensores_emergency_type__e emergency_type) {
    char *emergency_typeArray[] =  { "NULL", "EMERGENCY_STOP", "POWER_FAILURE", "PEOPLE_TRAPPED", "MECHANICAL_FAILURE", "FIRE_ALARM" };
    return emergency_typeArray[emergency_type];
}

api_gateway_coap_para_sistema_de_ascensores_emergency_type__e emergency_type_emergency_type_FromString(char* emergency_type) {
    int stringToReturn = 0;
    char *emergency_typeArray[] =  { "NULL", "EMERGENCY_STOP", "POWER_FAILURE", "PEOPLE_TRAPPED", "MECHANICAL_FAILURE", "FIRE_ALARM" };
    size_t sizeofArray = sizeof(emergency_typeArray) / sizeof(emergency_typeArray[0]);
    while(stringToReturn < sizeofArray) {
        if(strcmp(emergency_type, emergency_typeArray[stringToReturn]) == 0) {
            return stringToReturn;
        }
        stringToReturn++;
    }
    return 0;
}

cJSON *emergency_type_convertToJSON(api_gateway_coap_para_sistema_de_ascensores_emergency_type__e emergency_type) {
    cJSON *item = cJSON_CreateObject();
    if(cJSON_AddStringToObject(item, "emergency_type", emergency_type_emergency_type_ToString(emergency_type)) == NULL) {
        goto fail;
    }
    return item;
fail:
    cJSON_Delete(item);
    return NULL;
}

api_gateway_coap_para_sistema_de_ascensores_emergency_type__e emergency_type_parseFromJSON(cJSON *emergency_typeJSON) {
    if(!cJSON_IsString(emergency_typeJSON) || (emergency_typeJSON->valuestring == NULL)) {
        return 0;
    }
    return emergency_type_emergency_type_FromString(emergency_typeJSON->valuestring);
}
