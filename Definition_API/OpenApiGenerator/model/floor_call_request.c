#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "floor_call_request.h"



static floor_call_request_t *floor_call_request_create_internal(
    char *id_edificio,
    int piso_origen_llamada,
    api_gateway_coap_para_sistema_de_ascensores_movement_direction__e direccion_llamada,
    list_t *elevadores_estado
    ) {
    floor_call_request_t *floor_call_request_local_var = malloc(sizeof(floor_call_request_t));
    if (!floor_call_request_local_var) {
        return NULL;
    }
    floor_call_request_local_var->id_edificio = id_edificio;
    floor_call_request_local_var->piso_origen_llamada = piso_origen_llamada;
    floor_call_request_local_var->direccion_llamada = direccion_llamada;
    floor_call_request_local_var->elevadores_estado = elevadores_estado;

    floor_call_request_local_var->_library_owned = 1;
    return floor_call_request_local_var;
}

__attribute__((deprecated)) floor_call_request_t *floor_call_request_create(
    char *id_edificio,
    int piso_origen_llamada,
    api_gateway_coap_para_sistema_de_ascensores_movement_direction__e direccion_llamada,
    list_t *elevadores_estado
    ) {
    return floor_call_request_create_internal (
        id_edificio,
        piso_origen_llamada,
        direccion_llamada,
        elevadores_estado
        );
}

void floor_call_request_free(floor_call_request_t *floor_call_request) {
    if(NULL == floor_call_request){
        return ;
    }
    if(floor_call_request->_library_owned != 1){
        fprintf(stderr, "WARNING: %s() does NOT free objects allocated by the user\n", "floor_call_request_free");
        return ;
    }
    listEntry_t *listEntry;
    if (floor_call_request->id_edificio) {
        free(floor_call_request->id_edificio);
        floor_call_request->id_edificio = NULL;
    }
    if (floor_call_request->elevadores_estado) {
        list_ForEach(listEntry, floor_call_request->elevadores_estado) {
            elevator_state_free(listEntry->data);
        }
        list_freeList(floor_call_request->elevadores_estado);
        floor_call_request->elevadores_estado = NULL;
    }
    free(floor_call_request);
}

cJSON *floor_call_request_convertToJSON(floor_call_request_t *floor_call_request) {
    cJSON *item = cJSON_CreateObject();

    // floor_call_request->id_edificio
    if (!floor_call_request->id_edificio) {
        goto fail;
    }
    if(cJSON_AddStringToObject(item, "id_edificio", floor_call_request->id_edificio) == NULL) {
    goto fail; //String
    }


    // floor_call_request->piso_origen_llamada
    if (!floor_call_request->piso_origen_llamada) {
        goto fail;
    }
    if(cJSON_AddNumberToObject(item, "piso_origen_llamada", floor_call_request->piso_origen_llamada) == NULL) {
    goto fail; //Numeric
    }


    // floor_call_request->direccion_llamada
    if (api_gateway_coap_para_sistema_de_ascensores_movement_direction__NULL == floor_call_request->direccion_llamada) {
        goto fail;
    }
    cJSON *direccion_llamada_local_JSON = movement_direction_convertToJSON(floor_call_request->direccion_llamada);
    if(direccion_llamada_local_JSON == NULL) {
        goto fail; // custom
    }
    cJSON_AddItemToObject(item, "direccion_llamada", direccion_llamada_local_JSON);
    if(item->child == NULL) {
        goto fail;
    }


    // floor_call_request->elevadores_estado
    if (!floor_call_request->elevadores_estado) {
        goto fail;
    }
    cJSON *elevadores_estado = cJSON_AddArrayToObject(item, "elevadores_estado");
    if(elevadores_estado == NULL) {
    goto fail; //nonprimitive container
    }

    listEntry_t *elevadores_estadoListEntry;
    if (floor_call_request->elevadores_estado) {
    list_ForEach(elevadores_estadoListEntry, floor_call_request->elevadores_estado) {
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

floor_call_request_t *floor_call_request_parseFromJSON(cJSON *floor_call_requestJSON){

    floor_call_request_t *floor_call_request_local_var = NULL;

    // define the local variable for floor_call_request->direccion_llamada
    api_gateway_coap_para_sistema_de_ascensores_movement_direction__e direccion_llamada_local_nonprim = 0;

    // define the local list for floor_call_request->elevadores_estado
    list_t *elevadores_estadoList = NULL;

    // floor_call_request->id_edificio
    cJSON *id_edificio = cJSON_GetObjectItemCaseSensitive(floor_call_requestJSON, "id_edificio");
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

    // floor_call_request->piso_origen_llamada
    cJSON *piso_origen_llamada = cJSON_GetObjectItemCaseSensitive(floor_call_requestJSON, "piso_origen_llamada");
    if (cJSON_IsNull(piso_origen_llamada)) {
        piso_origen_llamada = NULL;
    }
    if (!piso_origen_llamada) {
        goto end;
    }

    
    if(!cJSON_IsNumber(piso_origen_llamada))
    {
    goto end; //Numeric
    }

    // floor_call_request->direccion_llamada
    cJSON *direccion_llamada = cJSON_GetObjectItemCaseSensitive(floor_call_requestJSON, "direccion_llamada");
    if (cJSON_IsNull(direccion_llamada)) {
        direccion_llamada = NULL;
    }
    if (!direccion_llamada) {
        goto end;
    }

    
    direccion_llamada_local_nonprim = movement_direction_parseFromJSON(direccion_llamada); //custom

    // floor_call_request->elevadores_estado
    cJSON *elevadores_estado = cJSON_GetObjectItemCaseSensitive(floor_call_requestJSON, "elevadores_estado");
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


    floor_call_request_local_var = floor_call_request_create_internal (
        strdup(id_edificio->valuestring),
        piso_origen_llamada->valuedouble,
        direccion_llamada_local_nonprim,
        elevadores_estadoList
        );

    return floor_call_request_local_var;
end:
    if (direccion_llamada_local_nonprim) {
        direccion_llamada_local_nonprim = 0;
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
