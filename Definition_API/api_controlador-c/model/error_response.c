#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "error_response.h"



static error_response_t *error_response_create_internal(
    char *error,
    char *message,
    char *expected,
    int received
    ) {
    error_response_t *error_response_local_var = malloc(sizeof(error_response_t));
    if (!error_response_local_var) {
        return NULL;
    }
    error_response_local_var->error = error;
    error_response_local_var->message = message;
    error_response_local_var->expected = expected;
    error_response_local_var->received = received;

    error_response_local_var->_library_owned = 1;
    return error_response_local_var;
}

__attribute__((deprecated)) error_response_t *error_response_create(
    char *error,
    char *message,
    char *expected,
    int received
    ) {
    return error_response_create_internal (
        error,
        message,
        expected,
        received
        );
}

void error_response_free(error_response_t *error_response) {
    if(NULL == error_response){
        return ;
    }
    if(error_response->_library_owned != 1){
        fprintf(stderr, "WARNING: %s() does NOT free objects allocated by the user\n", "error_response_free");
        return ;
    }
    listEntry_t *listEntry;
    if (error_response->error) {
        free(error_response->error);
        error_response->error = NULL;
    }
    if (error_response->message) {
        free(error_response->message);
        error_response->message = NULL;
    }
    if (error_response->expected) {
        free(error_response->expected);
        error_response->expected = NULL;
    }
    free(error_response);
}

cJSON *error_response_convertToJSON(error_response_t *error_response) {
    cJSON *item = cJSON_CreateObject();

    // error_response->error
    if (!error_response->error) {
        goto fail;
    }
    if(cJSON_AddStringToObject(item, "error", error_response->error) == NULL) {
    goto fail; //String
    }


    // error_response->message
    if (!error_response->message) {
        goto fail;
    }
    if(cJSON_AddStringToObject(item, "message", error_response->message) == NULL) {
    goto fail; //String
    }


    // error_response->expected
    if(error_response->expected) {
    if(cJSON_AddStringToObject(item, "expected", error_response->expected) == NULL) {
    goto fail; //String
    }
    }


    // error_response->received
    if(error_response->received) {
    if(cJSON_AddNumberToObject(item, "received", error_response->received) == NULL) {
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

error_response_t *error_response_parseFromJSON(cJSON *error_responseJSON){

    error_response_t *error_response_local_var = NULL;

    // error_response->error
    cJSON *error = cJSON_GetObjectItemCaseSensitive(error_responseJSON, "error");
    if (cJSON_IsNull(error)) {
        error = NULL;
    }
    if (!error) {
        goto end;
    }

    
    if(!cJSON_IsString(error))
    {
    goto end; //String
    }

    // error_response->message
    cJSON *message = cJSON_GetObjectItemCaseSensitive(error_responseJSON, "message");
    if (cJSON_IsNull(message)) {
        message = NULL;
    }
    if (!message) {
        goto end;
    }

    
    if(!cJSON_IsString(message))
    {
    goto end; //String
    }

    // error_response->expected
    cJSON *expected = cJSON_GetObjectItemCaseSensitive(error_responseJSON, "expected");
    if (cJSON_IsNull(expected)) {
        expected = NULL;
    }
    if (expected) { 
    if(!cJSON_IsString(expected) && !cJSON_IsNull(expected))
    {
    goto end; //String
    }
    }

    // error_response->received
    cJSON *received = cJSON_GetObjectItemCaseSensitive(error_responseJSON, "received");
    if (cJSON_IsNull(received)) {
        received = NULL;
    }
    if (received) { 
    if(!cJSON_IsNumber(received))
    {
    goto end; //Numeric
    }
    }


    error_response_local_var = error_response_create_internal (
        strdup(error->valuestring),
        strdup(message->valuestring),
        expected && !cJSON_IsNull(expected) ? strdup(expected->valuestring) : NULL,
        received ? received->valuedouble : 0
        );

    return error_response_local_var;
end:
    return NULL;

}
