#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "emergency_response.h"


char* emergency_response_protocolo_activado_ToString(api_gateway_coap_para_sistema_de_ascensores_emergency_response_PROTOCOLOACTIVADO_e protocolo_activado) {
    char* protocolo_activadoArray[] =  { "NULL", "RESCUE_PROTOCOL", "MAINTENANCE_ALERT", "FIRE_EVACUATION", "POWER_BACKUP" };
    return protocolo_activadoArray[protocolo_activado];
}

api_gateway_coap_para_sistema_de_ascensores_emergency_response_PROTOCOLOACTIVADO_e emergency_response_protocolo_activado_FromString(char* protocolo_activado){
    int stringToReturn = 0;
    char *protocolo_activadoArray[] =  { "NULL", "RESCUE_PROTOCOL", "MAINTENANCE_ALERT", "FIRE_EVACUATION", "POWER_BACKUP" };
    size_t sizeofArray = sizeof(protocolo_activadoArray) / sizeof(protocolo_activadoArray[0]);
    while(stringToReturn < sizeofArray) {
        if(strcmp(protocolo_activado, protocolo_activadoArray[stringToReturn]) == 0) {
            return stringToReturn;
        }
        stringToReturn++;
    }
    return 0;
}

static emergency_response_t *emergency_response_create_internal(
    char *emergencia_id,
    api_gateway_coap_para_sistema_de_ascensores_emergency_response_PROTOCOLOACTIVADO_e protocolo_activado,
    int tiempo_respuesta_estimado,
    list_t *servicios_notificados,
    list_t *ascensores_redirection
    ) {
    emergency_response_t *emergency_response_local_var = malloc(sizeof(emergency_response_t));
    if (!emergency_response_local_var) {
        return NULL;
    }
    emergency_response_local_var->emergencia_id = emergencia_id;
    emergency_response_local_var->protocolo_activado = protocolo_activado;
    emergency_response_local_var->tiempo_respuesta_estimado = tiempo_respuesta_estimado;
    emergency_response_local_var->servicios_notificados = servicios_notificados;
    emergency_response_local_var->ascensores_redirection = ascensores_redirection;

    emergency_response_local_var->_library_owned = 1;
    return emergency_response_local_var;
}

__attribute__((deprecated)) emergency_response_t *emergency_response_create(
    char *emergencia_id,
    api_gateway_coap_para_sistema_de_ascensores_emergency_response_PROTOCOLOACTIVADO_e protocolo_activado,
    int tiempo_respuesta_estimado,
    list_t *servicios_notificados,
    list_t *ascensores_redirection
    ) {
    return emergency_response_create_internal (
        emergencia_id,
        protocolo_activado,
        tiempo_respuesta_estimado,
        servicios_notificados,
        ascensores_redirection
        );
}

void emergency_response_free(emergency_response_t *emergency_response) {
    if(NULL == emergency_response){
        return ;
    }
    if(emergency_response->_library_owned != 1){
        fprintf(stderr, "WARNING: %s() does NOT free objects allocated by the user\n", "emergency_response_free");
        return ;
    }
    listEntry_t *listEntry;
    if (emergency_response->emergencia_id) {
        free(emergency_response->emergencia_id);
        emergency_response->emergencia_id = NULL;
    }
    if (emergency_response->servicios_notificados) {
        list_ForEach(listEntry, emergency_response->servicios_notificados) {
            free(listEntry->data);
        }
        list_freeList(emergency_response->servicios_notificados);
        emergency_response->servicios_notificados = NULL;
    }
    if (emergency_response->ascensores_redirection) {
        list_ForEach(listEntry, emergency_response->ascensores_redirection) {
            free(listEntry->data);
        }
        list_freeList(emergency_response->ascensores_redirection);
        emergency_response->ascensores_redirection = NULL;
    }
    free(emergency_response);
}

cJSON *emergency_response_convertToJSON(emergency_response_t *emergency_response) {
    cJSON *item = cJSON_CreateObject();

    // emergency_response->emergencia_id
    if (!emergency_response->emergencia_id) {
        goto fail;
    }
    if(cJSON_AddStringToObject(item, "emergencia_id", emergency_response->emergencia_id) == NULL) {
    goto fail; //String
    }


    // emergency_response->protocolo_activado
    if (api_gateway_coap_para_sistema_de_ascensores_emergency_response_PROTOCOLOACTIVADO_NULL == emergency_response->protocolo_activado) {
        goto fail;
    }
    if(cJSON_AddStringToObject(item, "protocolo_activado", emergency_response_protocolo_activado_ToString(emergency_response->protocolo_activado)) == NULL)
    {
    goto fail; //Enum
    }


    // emergency_response->tiempo_respuesta_estimado
    if (!emergency_response->tiempo_respuesta_estimado) {
        goto fail;
    }
    if(cJSON_AddNumberToObject(item, "tiempo_respuesta_estimado", emergency_response->tiempo_respuesta_estimado) == NULL) {
    goto fail; //Numeric
    }


    // emergency_response->servicios_notificados
    if (!emergency_response->servicios_notificados) {
        goto fail;
    }
    cJSON *servicios_notificados = cJSON_AddArrayToObject(item, "servicios_notificados");
    if(servicios_notificados == NULL) {
        goto fail; //primitive container
    }

    listEntry_t *servicios_notificadosListEntry;
    list_ForEach(servicios_notificadosListEntry, emergency_response->servicios_notificados) {
    if(cJSON_AddStringToObject(servicios_notificados, "", servicios_notificadosListEntry->data) == NULL)
    {
        goto fail;
    }
    }


    // emergency_response->ascensores_redirection
    if(emergency_response->ascensores_redirection) {
    cJSON *ascensores_redirection = cJSON_AddArrayToObject(item, "ascensores_redirection");
    if(ascensores_redirection == NULL) {
        goto fail; //primitive container
    }

    listEntry_t *ascensores_redirectionListEntry;
    list_ForEach(ascensores_redirectionListEntry, emergency_response->ascensores_redirection) {
    if(cJSON_AddStringToObject(ascensores_redirection, "", ascensores_redirectionListEntry->data) == NULL)
    {
        goto fail;
    }
    }
    }

    return item;
fail:
    if (item) {
        cJSON_Delete(item);
    }
    return NULL;
}

emergency_response_t *emergency_response_parseFromJSON(cJSON *emergency_responseJSON){

    emergency_response_t *emergency_response_local_var = NULL;

    // define the local list for emergency_response->servicios_notificados
    list_t *servicios_notificadosList = NULL;

    // define the local list for emergency_response->ascensores_redirection
    list_t *ascensores_redirectionList = NULL;

    // emergency_response->emergencia_id
    cJSON *emergencia_id = cJSON_GetObjectItemCaseSensitive(emergency_responseJSON, "emergencia_id");
    if (cJSON_IsNull(emergencia_id)) {
        emergencia_id = NULL;
    }
    if (!emergencia_id) {
        goto end;
    }

    
    if(!cJSON_IsString(emergencia_id))
    {
    goto end; //String
    }

    // emergency_response->protocolo_activado
    cJSON *protocolo_activado = cJSON_GetObjectItemCaseSensitive(emergency_responseJSON, "protocolo_activado");
    if (cJSON_IsNull(protocolo_activado)) {
        protocolo_activado = NULL;
    }
    if (!protocolo_activado) {
        goto end;
    }

    api_gateway_coap_para_sistema_de_ascensores_emergency_response_PROTOCOLOACTIVADO_e protocolo_activadoVariable;
    
    if(!cJSON_IsString(protocolo_activado))
    {
    goto end; //Enum
    }
    protocolo_activadoVariable = emergency_response_protocolo_activado_FromString(protocolo_activado->valuestring);

    // emergency_response->tiempo_respuesta_estimado
    cJSON *tiempo_respuesta_estimado = cJSON_GetObjectItemCaseSensitive(emergency_responseJSON, "tiempo_respuesta_estimado");
    if (cJSON_IsNull(tiempo_respuesta_estimado)) {
        tiempo_respuesta_estimado = NULL;
    }
    if (!tiempo_respuesta_estimado) {
        goto end;
    }

    
    if(!cJSON_IsNumber(tiempo_respuesta_estimado))
    {
    goto end; //Numeric
    }

    // emergency_response->servicios_notificados
    cJSON *servicios_notificados = cJSON_GetObjectItemCaseSensitive(emergency_responseJSON, "servicios_notificados");
    if (cJSON_IsNull(servicios_notificados)) {
        servicios_notificados = NULL;
    }
    if (!servicios_notificados) {
        goto end;
    }

    
    cJSON *servicios_notificados_local = NULL;
    if(!cJSON_IsArray(servicios_notificados)) {
        goto end;//primitive container
    }
    servicios_notificadosList = list_createList();

    cJSON_ArrayForEach(servicios_notificados_local, servicios_notificados)
    {
        if(!cJSON_IsString(servicios_notificados_local))
        {
            goto end;
        }
        list_addElement(servicios_notificadosList , strdup(servicios_notificados_local->valuestring));
    }

    // emergency_response->ascensores_redirection
    cJSON *ascensores_redirection = cJSON_GetObjectItemCaseSensitive(emergency_responseJSON, "ascensores_redirection");
    if (cJSON_IsNull(ascensores_redirection)) {
        ascensores_redirection = NULL;
    }
    if (ascensores_redirection) { 
    cJSON *ascensores_redirection_local = NULL;
    if(!cJSON_IsArray(ascensores_redirection)) {
        goto end;//primitive container
    }
    ascensores_redirectionList = list_createList();

    cJSON_ArrayForEach(ascensores_redirection_local, ascensores_redirection)
    {
        if(!cJSON_IsString(ascensores_redirection_local))
        {
            goto end;
        }
        list_addElement(ascensores_redirectionList , strdup(ascensores_redirection_local->valuestring));
    }
    }


    emergency_response_local_var = emergency_response_create_internal (
        strdup(emergencia_id->valuestring),
        protocolo_activadoVariable,
        tiempo_respuesta_estimado->valuedouble,
        servicios_notificadosList,
        ascensores_redirection ? ascensores_redirectionList : NULL
        );

    return emergency_response_local_var;
end:
    if (servicios_notificadosList) {
        listEntry_t *listEntry = NULL;
        list_ForEach(listEntry, servicios_notificadosList) {
            free(listEntry->data);
            listEntry->data = NULL;
        }
        list_freeList(servicios_notificadosList);
        servicios_notificadosList = NULL;
    }
    if (ascensores_redirectionList) {
        listEntry_t *listEntry = NULL;
        list_ForEach(listEntry, ascensores_redirectionList) {
            free(listEntry->data);
            listEntry->data = NULL;
        }
        list_freeList(ascensores_redirectionList);
        ascensores_redirectionList = NULL;
    }
    return NULL;

}
