#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "emergency_call_request.h"



static emergency_call_request_t *emergency_call_request_create_internal(
    char *id_edificio,
    char *ascensor_id_emergencia,
    api_gateway_coap_para_sistema_de_ascensores_emergency_type__e tipo_emergencia,
    int piso_actual_emergencia,
    char *descripcion_emergencia,
    char *timestamp_emergencia,
    list_t *elevadores_estado
    ) {
    emergency_call_request_t *emergency_call_request_local_var = malloc(sizeof(emergency_call_request_t));
    if (!emergency_call_request_local_var) {
        return NULL;
    }
    emergency_call_request_local_var->id_edificio = id_edificio;
    emergency_call_request_local_var->ascensor_id_emergencia = ascensor_id_emergencia;
    emergency_call_request_local_var->tipo_emergencia = tipo_emergencia;
    emergency_call_request_local_var->piso_actual_emergencia = piso_actual_emergencia;
    emergency_call_request_local_var->descripcion_emergencia = descripcion_emergencia;
    emergency_call_request_local_var->timestamp_emergencia = timestamp_emergencia;
    emergency_call_request_local_var->elevadores_estado = elevadores_estado;

    emergency_call_request_local_var->_library_owned = 1;
    return emergency_call_request_local_var;
}

__attribute__((deprecated)) emergency_call_request_t *emergency_call_request_create(
    char *id_edificio,
    char *ascensor_id_emergencia,
    api_gateway_coap_para_sistema_de_ascensores_emergency_type__e tipo_emergencia,
    int piso_actual_emergencia,
    char *descripcion_emergencia,
    char *timestamp_emergencia,
    list_t *elevadores_estado
    ) {
    return emergency_call_request_create_internal (
        id_edificio,
        ascensor_id_emergencia,
        tipo_emergencia,
        piso_actual_emergencia,
        descripcion_emergencia,
        timestamp_emergencia,
        elevadores_estado
        );
}

void emergency_call_request_free(emergency_call_request_t *emergency_call_request) {
    if(NULL == emergency_call_request){
        return ;
    }
    if(emergency_call_request->_library_owned != 1){
        fprintf(stderr, "WARNING: %s() does NOT free objects allocated by the user\n", "emergency_call_request_free");
        return ;
    }
    listEntry_t *listEntry;
    if (emergency_call_request->id_edificio) {
        free(emergency_call_request->id_edificio);
        emergency_call_request->id_edificio = NULL;
    }
    if (emergency_call_request->ascensor_id_emergencia) {
        free(emergency_call_request->ascensor_id_emergencia);
        emergency_call_request->ascensor_id_emergencia = NULL;
    }
    if (emergency_call_request->descripcion_emergencia) {
        free(emergency_call_request->descripcion_emergencia);
        emergency_call_request->descripcion_emergencia = NULL;
    }
    if (emergency_call_request->timestamp_emergencia) {
        free(emergency_call_request->timestamp_emergencia);
        emergency_call_request->timestamp_emergencia = NULL;
    }
    if (emergency_call_request->elevadores_estado) {
        list_ForEach(listEntry, emergency_call_request->elevadores_estado) {
            elevator_state_free(listEntry->data);
        }
        list_freeList(emergency_call_request->elevadores_estado);
        emergency_call_request->elevadores_estado = NULL;
    }
    free(emergency_call_request);
}

cJSON *emergency_call_request_convertToJSON(emergency_call_request_t *emergency_call_request) {
    cJSON *item = cJSON_CreateObject();

    // emergency_call_request->id_edificio
    if (!emergency_call_request->id_edificio) {
        goto fail;
    }
    if(cJSON_AddStringToObject(item, "id_edificio", emergency_call_request->id_edificio) == NULL) {
    goto fail; //String
    }


    // emergency_call_request->ascensor_id_emergencia
    if (!emergency_call_request->ascensor_id_emergencia) {
        goto fail;
    }
    if(cJSON_AddStringToObject(item, "ascensor_id_emergencia", emergency_call_request->ascensor_id_emergencia) == NULL) {
    goto fail; //String
    }


    // emergency_call_request->tipo_emergencia
    if (api_gateway_coap_para_sistema_de_ascensores_emergency_type__NULL == emergency_call_request->tipo_emergencia) {
        goto fail;
    }
    cJSON *tipo_emergencia_local_JSON = emergency_type_convertToJSON(emergency_call_request->tipo_emergencia);
    if(tipo_emergencia_local_JSON == NULL) {
        goto fail; // custom
    }
    cJSON_AddItemToObject(item, "tipo_emergencia", tipo_emergencia_local_JSON);
    if(item->child == NULL) {
        goto fail;
    }


    // emergency_call_request->piso_actual_emergencia
    if (!emergency_call_request->piso_actual_emergencia) {
        goto fail;
    }
    if(cJSON_AddNumberToObject(item, "piso_actual_emergencia", emergency_call_request->piso_actual_emergencia) == NULL) {
    goto fail; //Numeric
    }


    // emergency_call_request->descripcion_emergencia
    if(emergency_call_request->descripcion_emergencia) {
    if(cJSON_AddStringToObject(item, "descripcion_emergencia", emergency_call_request->descripcion_emergencia) == NULL) {
    goto fail; //String
    }
    }


    // emergency_call_request->timestamp_emergencia
    if (!emergency_call_request->timestamp_emergencia) {
        goto fail;
    }
    if(cJSON_AddStringToObject(item, "timestamp_emergencia", emergency_call_request->timestamp_emergencia) == NULL) {
    goto fail; //Date-Time
    }


    // emergency_call_request->elevadores_estado
    if (!emergency_call_request->elevadores_estado) {
        goto fail;
    }
    cJSON *elevadores_estado = cJSON_AddArrayToObject(item, "elevadores_estado");
    if(elevadores_estado == NULL) {
    goto fail; //nonprimitive container
    }

    listEntry_t *elevadores_estadoListEntry;
    if (emergency_call_request->elevadores_estado) {
    list_ForEach(elevadores_estadoListEntry, emergency_call_request->elevadores_estado) {
    cJSON *itemLocal = elevator_state_convertToJSON(elevadores_estadoListEntry->data);
    if(itemLocal == NULL) {
    goto fail;
    }
    cJSON_AddItemToArray(elevadores_estado, itemLocal);
    }
    }

    return item;
fail:
    if (item) {
        cJSON_Delete(item);
    }
    return NULL;
}

emergency_call_request_t *emergency_call_request_parseFromJSON(cJSON *emergency_call_requestJSON){

    emergency_call_request_t *emergency_call_request_local_var = NULL;

    // define the local variable for emergency_call_request->tipo_emergencia
    api_gateway_coap_para_sistema_de_ascensores_emergency_type__e tipo_emergencia_local_nonprim = 0;

    // define the local list for emergency_call_request->elevadores_estado
    list_t *elevadores_estadoList = NULL;

    // emergency_call_request->id_edificio
    cJSON *id_edificio = cJSON_GetObjectItemCaseSensitive(emergency_call_requestJSON, "id_edificio");
    if (cJSON_IsNull(id_edificio)) {
        id_edificio = NULL;
    }
    if (!id_edificio) {
        goto end;
    }

    
    if(!cJSON_IsString(id_edificio))
    {
    goto end; //String
    }

    // emergency_call_request->ascensor_id_emergencia
    cJSON *ascensor_id_emergencia = cJSON_GetObjectItemCaseSensitive(emergency_call_requestJSON, "ascensor_id_emergencia");
    if (cJSON_IsNull(ascensor_id_emergencia)) {
        ascensor_id_emergencia = NULL;
    }
    if (!ascensor_id_emergencia) {
        goto end;
    }

    
    if(!cJSON_IsString(ascensor_id_emergencia))
    {
    goto end; //String
    }

    // emergency_call_request->tipo_emergencia
    cJSON *tipo_emergencia = cJSON_GetObjectItemCaseSensitive(emergency_call_requestJSON, "tipo_emergencia");
    if (cJSON_IsNull(tipo_emergencia)) {
        tipo_emergencia = NULL;
    }
    if (!tipo_emergencia) {
        goto end;
    }

    
    tipo_emergencia_local_nonprim = emergency_type_parseFromJSON(tipo_emergencia); //custom

    // emergency_call_request->piso_actual_emergencia
    cJSON *piso_actual_emergencia = cJSON_GetObjectItemCaseSensitive(emergency_call_requestJSON, "piso_actual_emergencia");
    if (cJSON_IsNull(piso_actual_emergencia)) {
        piso_actual_emergencia = NULL;
    }
    if (!piso_actual_emergencia) {
        goto end;
    }

    
    if(!cJSON_IsNumber(piso_actual_emergencia))
    {
    goto end; //Numeric
    }

    // emergency_call_request->descripcion_emergencia
    cJSON *descripcion_emergencia = cJSON_GetObjectItemCaseSensitive(emergency_call_requestJSON, "descripcion_emergencia");
    if (cJSON_IsNull(descripcion_emergencia)) {
        descripcion_emergencia = NULL;
    }
    if (descripcion_emergencia) { 
    if(!cJSON_IsString(descripcion_emergencia) && !cJSON_IsNull(descripcion_emergencia))
    {
    goto end; //String
    }
    }

    // emergency_call_request->timestamp_emergencia
    cJSON *timestamp_emergencia = cJSON_GetObjectItemCaseSensitive(emergency_call_requestJSON, "timestamp_emergencia");
    if (cJSON_IsNull(timestamp_emergencia)) {
        timestamp_emergencia = NULL;
    }
    if (!timestamp_emergencia) {
        goto end;
    }

    
    if(!cJSON_IsString(timestamp_emergencia) && !cJSON_IsNull(timestamp_emergencia))
    {
    goto end; //DateTime
    }

    // emergency_call_request->elevadores_estado
    cJSON *elevadores_estado = cJSON_GetObjectItemCaseSensitive(emergency_call_requestJSON, "elevadores_estado");
    if (cJSON_IsNull(elevadores_estado)) {
        elevadores_estado = NULL;
    }
    if (!elevadores_estado) {
        goto end;
    }

    
    cJSON *elevadores_estado_local_nonprimitive = NULL;
    if(!cJSON_IsArray(elevadores_estado)){
        goto end; //nonprimitive container
    }

    elevadores_estadoList = list_createList();

    cJSON_ArrayForEach(elevadores_estado_local_nonprimitive,elevadores_estado )
    {
        if(!cJSON_IsObject(elevadores_estado_local_nonprimitive)){
            goto end;
        }
        elevator_state_t *elevadores_estadoItem = elevator_state_parseFromJSON(elevadores_estado_local_nonprimitive);

        list_addElement(elevadores_estadoList, elevadores_estadoItem);
    }


    emergency_call_request_local_var = emergency_call_request_create_internal (
        strdup(id_edificio->valuestring),
        strdup(ascensor_id_emergencia->valuestring),
        tipo_emergencia_local_nonprim,
        piso_actual_emergencia->valuedouble,
        descripcion_emergencia && !cJSON_IsNull(descripcion_emergencia) ? strdup(descripcion_emergencia->valuestring) : NULL,
        strdup(timestamp_emergencia->valuestring),
        elevadores_estadoList
        );

    return emergency_call_request_local_var;
end:
    if (tipo_emergencia_local_nonprim) {
        tipo_emergencia_local_nonprim = 0;
    }
    if (elevadores_estadoList) {
        listEntry_t *listEntry = NULL;
        list_ForEach(listEntry, elevadores_estadoList) {
            elevator_state_free(listEntry->data);
            listEntry->data = NULL;
        }
        list_freeList(elevadores_estadoList);
        elevadores_estadoList = NULL;
    }
    return NULL;

}
