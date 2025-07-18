#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include "DefaultAPI.h"

#define MAX_NUMBER_LENGTH 16
#define MAX_BUFFER_LENGTH 4096


// Procesar llamadas de emergencia desde ascensores
//
// Procesa solicitudes de emergencia originadas desde cabinas o controladores de ascensores. Activa protocolos de emergencia y notifica a servicios de mantenimiento y sistemas de seguridad.  Tipos de emergencia soportados: - EMERGENCY_STOP: Botón de parada activado - POWER_FAILURE: Fallo de alimentación detectado - PEOPLE_TRAPPED: Personas atrapadas en cabina - MECHANICAL_FAILURE: Fallo mecánico del ascensor - FIRE_ALARM: Alarma de incendios activada 
//
emergency_response_t*
DefaultAPI_llamadaEmergenciaPost(apiClient_t *apiClient, emergency_call_request_t *emergency_call_request)
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
    char *localVarPath = strdup("/llamada_emergencia");





    // Body Param
    cJSON *localVarSingleItemJSON_emergency_call_request = NULL;
    if (emergency_call_request != NULL)
    {
        //not string, not binary
        localVarSingleItemJSON_emergency_call_request = emergency_call_request_convertToJSON(emergency_call_request);
        localVarBodyParameters = cJSON_Print(localVarSingleItemJSON_emergency_call_request);
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
    //    printf("%s\n","Solicitud de emergencia procesada exitosamente");
    //}
    // uncomment below to debug the error response
    //if (apiClient->response_code == 400) {
    //    printf("%s\n","Error en la solicitud");
    //}
    // uncomment below to debug the error response
    //if (apiClient->response_code == 401) {
    //    printf("%s\n","No autorizado");
    //}
    // uncomment below to debug the error response
    //if (apiClient->response_code == 500) {
    //    printf("%s\n","Error interno del servidor");
    //}
    //nonprimitive not container
    emergency_response_t *elementToReturn = NULL;
    if(apiClient->response_code >= 200 && apiClient->response_code < 300) {
        cJSON *DefaultAPIlocalVarJSON = cJSON_Parse(apiClient->dataReceived);
        elementToReturn = emergency_response_parseFromJSON(DefaultAPIlocalVarJSON);
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
    if (localVarSingleItemJSON_emergency_call_request) {
        cJSON_Delete(localVarSingleItemJSON_emergency_call_request);
        localVarSingleItemJSON_emergency_call_request = NULL;
    }
    free(localVarBodyParameters);
    return elementToReturn;
end:
    free(localVarPath);
    return NULL;

}

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
    //    printf("%s\n","Error en la solicitud - JSON inválido, campos faltantes o piso fuera de rango.");
    //}
    // uncomment below to debug the error response
    //if (apiClient->response_code == 401) {
    //    printf("%s\n","No autorizado - Sesión DTLS no establecida correctamente.");
    //}
    // uncomment below to debug the error response
    //if (apiClient->response_code == 415) {
    //    printf("%s\n","Tipo de contenido no soportado - Se requiere application/json.");
    //}
    // uncomment below to debug the error response
    //if (apiClient->response_code == 500) {
    //    printf("%s\n","Error interno en el servidor central durante procesamiento de solicitud.");
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
    //    printf("%s\n","Error en la solicitud - JSON inválido, campos faltantes o piso fuera de rango.");
    //}
    // uncomment below to debug the error response
    //if (apiClient->response_code == 401) {
    //    printf("%s\n","No autorizado - Sesión DTLS no establecida correctamente.");
    //}
    // uncomment below to debug the error response
    //if (apiClient->response_code == 415) {
    //    printf("%s\n","Tipo de contenido no soportado - Se requiere application/json.");
    //}
    // uncomment below to debug the error response
    //if (apiClient->response_code == 500) {
    //    printf("%s\n","Error interno en el servidor central durante procesamiento de solicitud.");
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

