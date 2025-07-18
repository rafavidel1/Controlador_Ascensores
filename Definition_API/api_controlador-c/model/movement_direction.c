#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "movement_direction.h"


char* movement_direction_movement_direction_ToString(api_gateway_coap_para_sistema_de_ascensores_movement_direction__e movement_direction) {
    char *movement_directionArray[] =  { "NULL", "SUBIENDO", "BAJANDO", "PARADO" };
    return movement_directionArray[movement_direction];
}

api_gateway_coap_para_sistema_de_ascensores_movement_direction__e movement_direction_movement_direction_FromString(char* movement_direction) {
    int stringToReturn = 0;
    char *movement_directionArray[] =  { "NULL", "SUBIENDO", "BAJANDO", "PARADO" };
    size_t sizeofArray = sizeof(movement_directionArray) / sizeof(movement_directionArray[0]);
    while(stringToReturn < sizeofArray) {
        if(strcmp(movement_direction, movement_directionArray[stringToReturn]) == 0) {
            return stringToReturn;
        }
        stringToReturn++;
    }
    return 0;
}

cJSON *movement_direction_convertToJSON(api_gateway_coap_para_sistema_de_ascensores_movement_direction__e movement_direction) {
    cJSON *item = cJSON_CreateObject();
    if(cJSON_AddStringToObject(item, "movement_direction", movement_direction_movement_direction_ToString(movement_direction)) == NULL) {
        goto fail;
    }
    return item;
fail:
    cJSON_Delete(item);
    return NULL;
}

api_gateway_coap_para_sistema_de_ascensores_movement_direction__e movement_direction_parseFromJSON(cJSON *movement_directionJSON) {
    if(!cJSON_IsString(movement_directionJSON) || (movement_directionJSON->valuestring == NULL)) {
        return 0;
    }
    return movement_direction_movement_direction_FromString(movement_directionJSON->valuestring);
}
