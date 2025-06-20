#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "door_state.h"


char* door_state_door_state_ToString(api_gateway_coap_para_sistema_de_ascensores_door_state__e door_state) {
    char *door_stateArray[] =  { "NULL", "CERRADA", "ABIERTA", "ABRIENDO", "CERRANDO" };
    return door_stateArray[door_state];
}

api_gateway_coap_para_sistema_de_ascensores_door_state__e door_state_door_state_FromString(char* door_state) {
    int stringToReturn = 0;
    char *door_stateArray[] =  { "NULL", "CERRADA", "ABIERTA", "ABRIENDO", "CERRANDO" };
    size_t sizeofArray = sizeof(door_stateArray) / sizeof(door_stateArray[0]);
    while(stringToReturn < sizeofArray) {
        if(strcmp(door_state, door_stateArray[stringToReturn]) == 0) {
            return stringToReturn;
        }
        stringToReturn++;
    }
    return 0;
}

cJSON *door_state_convertToJSON(api_gateway_coap_para_sistema_de_ascensores_door_state__e door_state) {
    cJSON *item = cJSON_CreateObject();
    if(cJSON_AddStringToObject(item, "door_state", door_state_door_state_ToString(door_state)) == NULL) {
        goto fail;
    }
    return item;
fail:
    cJSON_Delete(item);
    return NULL;
}

api_gateway_coap_para_sistema_de_ascensores_door_state__e door_state_parseFromJSON(cJSON *door_stateJSON) {
    if(!cJSON_IsString(door_stateJSON) || (door_stateJSON->valuestring == NULL)) {
        return 0;
    }
    return door_state_door_state_FromString(door_stateJSON->valuestring);
}
