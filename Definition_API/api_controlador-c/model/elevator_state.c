#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "elevator_state.h"



static elevator_state_t *elevator_state_create_internal(
    char *id_ascensor,
    int piso_actual,
    api_gateway_coap_para_sistema_de_ascensores_door_state__e estado_puerta,
    int disponible,
    char *tarea_actual_id,
    int destino_actual
    ) {
    elevator_state_t *elevator_state_local_var = malloc(sizeof(elevator_state_t));
    if (!elevator_state_local_var) {
        return NULL;
    }
    elevator_state_local_var->id_ascensor = id_ascensor;
    elevator_state_local_var->piso_actual = piso_actual;
    elevator_state_local_var->estado_puerta = estado_puerta;
    elevator_state_local_var->disponible = disponible;
    elevator_state_local_var->tarea_actual_id = tarea_actual_id;
    elevator_state_local_var->destino_actual = destino_actual;

    elevator_state_local_var->_library_owned = 1;
    return elevator_state_local_var;
}

__attribute__((deprecated)) elevator_state_t *elevator_state_create(
    char *id_ascensor,
    int piso_actual,
    api_gateway_coap_para_sistema_de_ascensores_door_state__e estado_puerta,
    int disponible,
    char *tarea_actual_id,
    int destino_actual
    ) {
    return elevator_state_create_internal (
        id_ascensor,
        piso_actual,
        estado_puerta,
        disponible,
        tarea_actual_id,
        destino_actual
        );
}

void elevator_state_free(elevator_state_t *elevator_state) {
    if(NULL == elevator_state){
        return ;
    }
    if(elevator_state->_library_owned != 1){
        fprintf(stderr, "WARNING: %s() does NOT free objects allocated by the user\n", "elevator_state_free");
        return ;
    }
    listEntry_t *listEntry;
    if (elevator_state->id_ascensor) {
        free(elevator_state->id_ascensor);
        elevator_state->id_ascensor = NULL;
    }
    if (elevator_state->tarea_actual_id) {
        free(elevator_state->tarea_actual_id);
        elevator_state->tarea_actual_id = NULL;
    }
    free(elevator_state);
}

cJSON *elevator_state_convertToJSON(elevator_state_t *elevator_state) {
    cJSON *item = cJSON_CreateObject();

    // elevator_state->id_ascensor
    if (!elevator_state->id_ascensor) {
        goto fail;
    }
    if(cJSON_AddStringToObject(item, "id_ascensor", elevator_state->id_ascensor) == NULL) {
    goto fail; //String
    }


    // elevator_state->piso_actual
    if (!elevator_state->piso_actual) {
        goto fail;
    }
    if(cJSON_AddNumberToObject(item, "piso_actual", elevator_state->piso_actual) == NULL) {
    goto fail; //Numeric
    }


    // elevator_state->estado_puerta
    if (api_gateway_coap_para_sistema_de_ascensores_door_state__NULL == elevator_state->estado_puerta) {
        goto fail;
    }
    cJSON *estado_puerta_local_JSON = door_state_convertToJSON(elevator_state->estado_puerta);
    if(estado_puerta_local_JSON == NULL) {
        goto fail; // custom
    }
    cJSON_AddItemToObject(item, "estado_puerta", estado_puerta_local_JSON);
    if(item->child == NULL) {
        goto fail;
    }


    // elevator_state->disponible
    if (!elevator_state->disponible) {
        goto fail;
    }
    if(cJSON_AddBoolToObject(item, "disponible", elevator_state->disponible) == NULL) {
    goto fail; //Bool
    }


    // elevator_state->tarea_actual_id
    if(elevator_state->tarea_actual_id) {
    if(cJSON_AddStringToObject(item, "tarea_actual_id", elevator_state->tarea_actual_id) == NULL) {
    goto fail; //String
    }
    }


    // elevator_state->destino_actual
    if(elevator_state->destino_actual) {
    if(cJSON_AddNumberToObject(item, "destino_actual", elevator_state->destino_actual) == NULL) {
    goto fail; //Numeric
    }
    }

    return item;
fail:
    if (item) {
        cJSON_Delete(item);
    }
    return NULL;
}

elevator_state_t *elevator_state_parseFromJSON(cJSON *elevator_stateJSON){

    elevator_state_t *elevator_state_local_var = NULL;

    // define the local variable for elevator_state->estado_puerta
    api_gateway_coap_para_sistema_de_ascensores_door_state__e estado_puerta_local_nonprim = 0;

    // elevator_state->id_ascensor
    cJSON *id_ascensor = cJSON_GetObjectItemCaseSensitive(elevator_stateJSON, "id_ascensor");
    if (cJSON_IsNull(id_ascensor)) {
        id_ascensor = NULL;
    }
    if (!id_ascensor) {
        goto end;
    }

    
    if(!cJSON_IsString(id_ascensor))
    {
    goto end; //String
    }

    // elevator_state->piso_actual
    cJSON *piso_actual = cJSON_GetObjectItemCaseSensitive(elevator_stateJSON, "piso_actual");
    if (cJSON_IsNull(piso_actual)) {
        piso_actual = NULL;
    }
    if (!piso_actual) {
        goto end;
    }

    
    if(!cJSON_IsNumber(piso_actual))
    {
    goto end; //Numeric
    }

    // elevator_state->estado_puerta
    cJSON *estado_puerta = cJSON_GetObjectItemCaseSensitive(elevator_stateJSON, "estado_puerta");
    if (cJSON_IsNull(estado_puerta)) {
        estado_puerta = NULL;
    }
    if (!estado_puerta) {
        goto end;
    }

    
    estado_puerta_local_nonprim = door_state_parseFromJSON(estado_puerta); //custom

    // elevator_state->disponible
    cJSON *disponible = cJSON_GetObjectItemCaseSensitive(elevator_stateJSON, "disponible");
    if (cJSON_IsNull(disponible)) {
        disponible = NULL;
    }
    if (!disponible) {
        goto end;
    }

    
    if(!cJSON_IsBool(disponible))
    {
    goto end; //Bool
    }

    // elevator_state->tarea_actual_id
    cJSON *tarea_actual_id = cJSON_GetObjectItemCaseSensitive(elevator_stateJSON, "tarea_actual_id");
    if (cJSON_IsNull(tarea_actual_id)) {
        tarea_actual_id = NULL;
    }
    if (tarea_actual_id) { 
    if(!cJSON_IsString(tarea_actual_id) && !cJSON_IsNull(tarea_actual_id))
    {
    goto end; //String
    }
    }

    // elevator_state->destino_actual
    cJSON *destino_actual = cJSON_GetObjectItemCaseSensitive(elevator_stateJSON, "destino_actual");
    if (cJSON_IsNull(destino_actual)) {
        destino_actual = NULL;
    }
    if (destino_actual) { 
    if(!cJSON_IsNumber(destino_actual))
    {
    goto end; //Numeric
    }
    }


    elevator_state_local_var = elevator_state_create_internal (
        strdup(id_ascensor->valuestring),
        piso_actual->valuedouble,
        estado_puerta_local_nonprim,
        disponible->valueint,
        tarea_actual_id && !cJSON_IsNull(tarea_actual_id) ? strdup(tarea_actual_id->valuestring) : NULL,
        destino_actual ? destino_actual->valuedouble : 0
        );

    return elevator_state_local_var;
end:
    if (estado_puerta_local_nonprim) {
        estado_puerta_local_nonprim = 0;
    }
    return NULL;

}
