#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "cabin_request.h"



static cabin_request_t *cabin_request_create_internal(
    char *id_edificio,
    char *solicitando_ascensor_id,
    int piso_destino_solicitud,
    list_t *elevadores_estado
    ) {
    cabin_request_t *cabin_request_local_var = malloc(sizeof(cabin_request_t));
    if (!cabin_request_local_var) {
        return NULL;
    }
    cabin_request_local_var->id_edificio = id_edificio;
    cabin_request_local_var->solicitando_ascensor_id = solicitando_ascensor_id;
    cabin_request_local_var->piso_destino_solicitud = piso_destino_solicitud;
    cabin_request_local_var->elevadores_estado = elevadores_estado;

    cabin_request_local_var->_library_owned = 1;
    return cabin_request_local_var;
}

__attribute__((deprecated)) cabin_request_t *cabin_request_create(
    char *id_edificio,
    char *solicitando_ascensor_id,
    int piso_destino_solicitud,
    list_t *elevadores_estado
    ) {
    return cabin_request_create_internal (
        id_edificio,
        solicitando_ascensor_id,
        piso_destino_solicitud,
        elevadores_estado
        );
}

void cabin_request_free(cabin_request_t *cabin_request) {
    if(NULL == cabin_request){
        return ;
    }
    if(cabin_request->_library_owned != 1){
        fprintf(stderr, "WARNING: %s() does NOT free objects allocated by the user\n", "cabin_request_free");
        return ;
    }
    listEntry_t *listEntry;
    if (cabin_request->id_edificio) {
        free(cabin_request->id_edificio);
        cabin_request->id_edificio = NULL;
    }
    if (cabin_request->solicitando_ascensor_id) {
        free(cabin_request->solicitando_ascensor_id);
        cabin_request->solicitando_ascensor_id = NULL;
    }
    if (cabin_request->elevadores_estado) {
        list_ForEach(listEntry, cabin_request->elevadores_estado) {
            elevator_state_free(listEntry->data);
        }
        list_freeList(cabin_request->elevadores_estado);
        cabin_request->elevadores_estado = NULL;
    }
    free(cabin_request);
}

cJSON *cabin_request_convertToJSON(cabin_request_t *cabin_request) {
    cJSON *item = cJSON_CreateObject();

    // cabin_request->id_edificio
    if (!cabin_request->id_edificio) {
        goto fail;
    }
    if(cJSON_AddStringToObject(item, "id_edificio", cabin_request->id_edificio) == NULL) {
    goto fail; //String
    }


    // cabin_request->solicitando_ascensor_id
    if (!cabin_request->solicitando_ascensor_id) {
        goto fail;
    }
    if(cJSON_AddStringToObject(item, "solicitando_ascensor_id", cabin_request->solicitando_ascensor_id) == NULL) {
    goto fail; //String
    }


    // cabin_request->piso_destino_solicitud
    if (!cabin_request->piso_destino_solicitud) {
        goto fail;
    }
    if(cJSON_AddNumberToObject(item, "piso_destino_solicitud", cabin_request->piso_destino_solicitud) == NULL) {
    goto fail; //Numeric
    }


    // cabin_request->elevadores_estado
    if (!cabin_request->elevadores_estado) {
        goto fail;
    }
    cJSON *elevadores_estado = cJSON_AddArrayToObject(item, "elevadores_estado");
    if(elevadores_estado == NULL) {
    goto fail; //nonprimitive container
    }

    listEntry_t *elevadores_estadoListEntry;
    if (cabin_request->elevadores_estado) {
    list_ForEach(elevadores_estadoListEntry, cabin_request->elevadores_estado) {
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

cabin_request_t *cabin_request_parseFromJSON(cJSON *cabin_requestJSON){

    cabin_request_t *cabin_request_local_var = NULL;

    // define the local list for cabin_request->elevadores_estado
    list_t *elevadores_estadoList = NULL;

    // cabin_request->id_edificio
    cJSON *id_edificio = cJSON_GetObjectItemCaseSensitive(cabin_requestJSON, "id_edificio");
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

    // cabin_request->solicitando_ascensor_id
    cJSON *solicitando_ascensor_id = cJSON_GetObjectItemCaseSensitive(cabin_requestJSON, "solicitando_ascensor_id");
    if (cJSON_IsNull(solicitando_ascensor_id)) {
        solicitando_ascensor_id = NULL;
    }
    if (!solicitando_ascensor_id) {
        goto end;
    }

    
    if(!cJSON_IsString(solicitando_ascensor_id))
    {
    goto end; //String
    }

    // cabin_request->piso_destino_solicitud
    cJSON *piso_destino_solicitud = cJSON_GetObjectItemCaseSensitive(cabin_requestJSON, "piso_destino_solicitud");
    if (cJSON_IsNull(piso_destino_solicitud)) {
        piso_destino_solicitud = NULL;
    }
    if (!piso_destino_solicitud) {
        goto end;
    }

    
    if(!cJSON_IsNumber(piso_destino_solicitud))
    {
    goto end; //Numeric
    }

    // cabin_request->elevadores_estado
    cJSON *elevadores_estado = cJSON_GetObjectItemCaseSensitive(cabin_requestJSON, "elevadores_estado");
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


    cabin_request_local_var = cabin_request_create_internal (
        strdup(id_edificio->valuestring),
        strdup(solicitando_ascensor_id->valuestring),
        piso_destino_solicitud->valuedouble,
        elevadores_estadoList
        );

    return cabin_request_local_var;
end:
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
