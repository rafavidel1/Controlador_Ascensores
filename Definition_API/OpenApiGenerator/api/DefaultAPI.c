#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include "DefaultAPI.h"

#define MAX_NUMBER_LENGTH 16
#define MAX_BUFFER_LENGTH 4096


// Enviar una solicitud de destino desde la cabina de un ascensor
//
// Procesa una solicitud de destino desde el interior de una cabina de ascensor, enviando el estado actual de todos los ascensores gestionados. La solicitud se envía al servidor central a través de CoAP sobre DTLS-PSK. 
//
server_response_t*
DefaultAPI_peticionCabinaPost(apiClient_t *apiClient, cabin_request_t *cabin_request)
{
    list_t    *localVarQueryParameters = NULL;
    list_t    *localVarHeaderParameters = NULL;
    list_t    *localVarFormParameters = NULL;
    list_t *localVarHeaderType = list_createList();
    list_t *localVarContentType = list_createList();
    char      *localVarBodyParameters = NULL;
    size_t     localVarBodyLength = 0;

    // clear the error code from the previous api call
    apiClient->response_code = 0;

    // create the path
    char *localVarPath = strdup("/peticion_cabina");





    // Body Param
    cJSON *localVarSingleItemJSON_cabin_request = NULL;
    if (cabin_request != NULL)
    {
        //not string, not binary
        localVarSingleItemJSON_cabin_request = cabin_request_convertToJSON(cabin_request);
        localVarBodyParameters = cJSON_Print(localVarSingleItemJSON_cabin_request);
        localVarBodyLength = strlen(localVarBodyParameters);
    }
    list_addElement(localVarHeaderType,"application/json"); //produces
    list_addElement(localVarContentType,"application/json"); //consumes
    apiClient_invoke(apiClient,
                    localVarPath,
                    localVarQueryParameters,
                    localVarHeaderParameters,
                    localVarFormParameters,
                    localVarHeaderType,
                    localVarContentType,
                    localVarBodyParameters,
                    localVarBodyLength,
                    "POST");

    // uncomment below to debug the error response
    //if (apiClient->response_code == 200) {
    //    printf("%s\n","Respuesta exitosa del servidor central con la asignación del ascensor.");
    //}
    // uncomment below to debug the error response
    //if (apiClient->response_code == 400) {
    //    printf("%s\n","Error en la solicitud (e.g., formato JSON inválido o campos faltantes).");
    //}
    // uncomment below to debug the error response
    //if (apiClient->response_code == 500) {
    //    printf("%s\n","Error interno en el servidor central.");
    //}
    //nonprimitive not container
    server_response_t *elementToReturn = NULL;
    if(apiClient->response_code >= 200 && apiClient->response_code < 300) {
        cJSON *DefaultAPIlocalVarJSON = cJSON_Parse(apiClient->dataReceived);
        elementToReturn = server_response_parseFromJSON(DefaultAPIlocalVarJSON);
        cJSON_Delete(DefaultAPIlocalVarJSON);
        if(elementToReturn == NULL) {
            // return 0;
        }
    }

    //return type
    if (apiClient->dataReceived) {
        free(apiClient->dataReceived);
        apiClient->dataReceived = NULL;
        apiClient->dataReceivedLen = 0;
    }
    
    
    
    list_freeList(localVarHeaderType);
    list_freeList(localVarContentType);
    free(localVarPath);
    if (localVarSingleItemJSON_cabin_request) {
        cJSON_Delete(localVarSingleItemJSON_cabin_request);
        localVarSingleItemJSON_cabin_request = NULL;
    }
    free(localVarBodyParameters);
    return elementToReturn;
end:
    free(localVarPath);
    return NULL;

}

// Enviar una solicitud de llamada de ascensor desde un piso
//
// Procesa una solicitud de llamada de ascensor desde un piso específico, enviando el estado actual de todos los ascensores gestionados. La solicitud se envía al servidor central a través de CoAP sobre DTLS-PSK. 
//
server_response_t*
DefaultAPI_peticionPisoPost(apiClient_t *apiClient, floor_call_request_t *floor_call_request)
{
    list_t    *localVarQueryParameters = NULL;
    list_t    *localVarHeaderParameters = NULL;
    list_t    *localVarFormParameters = NULL;
    list_t *localVarHeaderType = list_createList();
    list_t *localVarContentType = list_createList();
    char      *localVarBodyParameters = NULL;
    size_t     localVarBodyLength = 0;

    // clear the error code from the previous api call
    apiClient->response_code = 0;

    // create the path
    char *localVarPath = strdup("/peticion_piso");





    // Body Param
    cJSON *localVarSingleItemJSON_floor_call_request = NULL;
    if (floor_call_request != NULL)
    {
        //not string, not binary
        localVarSingleItemJSON_floor_call_request = floor_call_request_convertToJSON(floor_call_request);
        localVarBodyParameters = cJSON_Print(localVarSingleItemJSON_floor_call_request);
        localVarBodyLength = strlen(localVarBodyParameters);
    }
    list_addElement(localVarHeaderType,"application/json"); //produces
    list_addElement(localVarContentType,"application/json"); //consumes
    apiClient_invoke(apiClient,
                    localVarPath,
                    localVarQueryParameters,
                    localVarHeaderParameters,
                    localVarFormParameters,
                    localVarHeaderType,
                    localVarContentType,
                    localVarBodyParameters,
                    localVarBodyLength,
                    "POST");

    // uncomment below to debug the error response
    //if (apiClient->response_code == 200) {
    //    printf("%s\n","Respuesta exitosa del servidor central con la asignación del ascensor.");
    //}
    // uncomment below to debug the error response
    //if (apiClient->response_code == 400) {
    //    printf("%s\n","Error en la solicitud (e.g., formato JSON inválido o campos faltantes).");
    //}
    // uncomment below to debug the error response
    //if (apiClient->response_code == 500) {
    //    printf("%s\n","Error interno en el servidor central.");
    //}
    //nonprimitive not container
    server_response_t *elementToReturn = NULL;
    if(apiClient->response_code >= 200 && apiClient->response_code < 300) {
        cJSON *DefaultAPIlocalVarJSON = cJSON_Parse(apiClient->dataReceived);
        elementToReturn = server_response_parseFromJSON(DefaultAPIlocalVarJSON);
        cJSON_Delete(DefaultAPIlocalVarJSON);
        if(elementToReturn == NULL) {
            // return 0;
        }
    }

    //return type
    if (apiClient->dataReceived) {
        free(apiClient->dataReceived);
        apiClient->dataReceived = NULL;
        apiClient->dataReceivedLen = 0;
    }
    
    
    
    list_freeList(localVarHeaderType);
    list_freeList(localVarContentType);
    free(localVarPath);
    if (localVarSingleItemJSON_floor_call_request) {
        cJSON_Delete(localVarSingleItemJSON_floor_call_request);
        localVarSingleItemJSON_floor_call_request = NULL;
    }
    free(localVarBodyParameters);
    return elementToReturn;
end:
    free(localVarPath);
    return NULL;

}

