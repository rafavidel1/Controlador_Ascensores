#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "server_response.h"



static server_response_t *server_response_create_internal(
    char *ascensor_asignado_id,
    char *tarea_id,
    int piso_destino_asignado,
    int tiempo_estimado_llegada
    ) {
    server_response_t *server_response_local_var = malloc(sizeof(server_response_t));
    if (!server_response_local_var) {
        return NULL;
    }
    server_response_local_var->ascensor_asignado_id = ascensor_asignado_id;
    server_response_local_var->tarea_id = tarea_id;
    server_response_local_var->piso_destino_asignado = piso_destino_asignado;
    server_response_local_var->tiempo_estimado_llegada = tiempo_estimado_llegada;

    server_response_local_var->_library_owned = 1;
    return server_response_local_var;
}

__attribute__((deprecated)) server_response_t *server_response_create(
    char *ascensor_asignado_id,
    char *tarea_id,
    int piso_destino_asignado,
    int tiempo_estimado_llegada
    ) {
    return server_response_create_internal (
        ascensor_asignado_id,
        tarea_id,
        piso_destino_asignado,
        tiempo_estimado_llegada
        );
}

void server_response_free(server_response_t *server_response) {
    if(NULL == server_response){
        return ;
    }
    if(server_response->_library_owned != 1){
        fprintf(stderr, "WARNING: %s() does NOT free objects allocated by the user\n", "server_response_free");
        return ;
    }
    listEntry_t *listEntry;
    if (server_response->ascensor_asignado_id) {
        free(server_response->ascensor_asignado_id);
        server_response->ascensor_asignado_id = NULL;
    }
    if (server_response->tarea_id) {
        free(server_response->tarea_id);
        server_response->tarea_id = NULL;
    }
    free(server_response);
}

cJSON *server_response_convertToJSON(server_response_t *server_response) {
    cJSON *item = cJSON_CreateObject();

    // server_response->ascensor_asignado_id
    if (!server_response->ascensor_asignado_id) {
        goto fail;
    }
    if(cJSON_AddStringToObject(item, "ascensor_asignado_id", server_response->ascensor_asignado_id) == NULL) {
    goto fail; //String
    }


    // server_response->tarea_id
    if (!server_response->tarea_id) {
        goto fail;
    }
    if(cJSON_AddStringToObject(item, "tarea_id", server_response->tarea_id) == NULL) {
    goto fail; //String
    }


    // server_response->piso_destino_asignado
    if (!server_response->piso_destino_asignado) {
        goto fail;
    }
    if(cJSON_AddNumberToObject(item, "piso_destino_asignado", server_response->piso_destino_asignado) == NULL) {
    goto fail; //Numeric
    }


    // server_response->tiempo_estimado_llegada
    if(server_response->tiempo_estimado_llegada) {
    if(cJSON_AddNumberToObject(item, "tiempo_estimado_llegada", server_response->tiempo_estimado_llegada) == NULL) {
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

server_response_t *server_response_parseFromJSON(cJSON *server_responseJSON){

    server_response_t *server_response_local_var = NULL;

    // server_response->ascensor_asignado_id
    cJSON *ascensor_asignado_id = cJSON_GetObjectItemCaseSensitive(server_responseJSON, "ascensor_asignado_id");
    if (cJSON_IsNull(ascensor_asignado_id)) {
        ascensor_asignado_id = NULL;
    }
    if (!ascensor_asignado_id) {
        goto end;
    }

    
    if(!cJSON_IsString(ascensor_asignado_id))
    {
    goto end; //String
    }

    // server_response->tarea_id
    cJSON *tarea_id = cJSON_GetObjectItemCaseSensitive(server_responseJSON, "tarea_id");
    if (cJSON_IsNull(tarea_id)) {
        tarea_id = NULL;
    }
    if (!tarea_id) {
        goto end;
    }

    
    if(!cJSON_IsString(tarea_id))
    {
    goto end; //String
    }

    // server_response->piso_destino_asignado
    cJSON *piso_destino_asignado = cJSON_GetObjectItemCaseSensitive(server_responseJSON, "piso_destino_asignado");
    if (cJSON_IsNull(piso_destino_asignado)) {
        piso_destino_asignado = NULL;
    }
    if (!piso_destino_asignado) {
        goto end;
    }

    
    if(!cJSON_IsNumber(piso_destino_asignado))
    {
    goto end; //Numeric
    }

    // server_response->tiempo_estimado_llegada
    cJSON *tiempo_estimado_llegada = cJSON_GetObjectItemCaseSensitive(server_responseJSON, "tiempo_estimado_llegada");
    if (cJSON_IsNull(tiempo_estimado_llegada)) {
        tiempo_estimado_llegada = NULL;
    }
    if (tiempo_estimado_llegada) { 
    if(!cJSON_IsNumber(tiempo_estimado_llegada))
    {
    goto end; //Numeric
    }
    }


    server_response_local_var = server_response_create_internal (
        strdup(ascensor_asignado_id->valuestring),
        strdup(tarea_id->valuestring),
        piso_destino_asignado->valuedouble,
        tiempo_estimado_llegada ? tiempo_estimado_llegada->valuedouble : 0
        );

    return server_response_local_var;
end:
    return NULL;

}
