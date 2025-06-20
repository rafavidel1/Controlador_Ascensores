# DefaultAPI

All URIs are relative to *http://localhost*

Method | HTTP request | Description
------------- | ------------- | -------------
[**DefaultAPI_peticionCabinaPost**](DefaultAPI.md#DefaultAPI_peticionCabinaPost) | **POST** /peticion_cabina | Enviar una solicitud de destino desde la cabina de un ascensor
[**DefaultAPI_peticionPisoPost**](DefaultAPI.md#DefaultAPI_peticionPisoPost) | **POST** /peticion_piso | Enviar una solicitud de llamada de ascensor desde un piso


# **DefaultAPI_peticionCabinaPost**
```c
// Enviar una solicitud de destino desde la cabina de un ascensor
//
// Procesa una solicitud de destino desde el interior de una cabina de ascensor, enviando el estado actual de todos los ascensores gestionados. La solicitud se envía al servidor central a través de CoAP sobre DTLS-PSK. 
//
server_response_t* DefaultAPI_peticionCabinaPost(apiClient_t *apiClient, cabin_request_t *cabin_request);
```

### Parameters
Name | Type | Description  | Notes
------------- | ------------- | ------------- | -------------
**apiClient** | **apiClient_t \*** | context containing the client configuration |
**cabin_request** | **[cabin_request_t](cabin_request.md) \*** |  | 

### Return type

[server_response_t](server_response.md) *


### Authorization

No authorization required

### HTTP request headers

 - **Content-Type**: application/json
 - **Accept**: application/json

[[Back to top]](#) [[Back to API list]](../README.md#documentation-for-api-endpoints) [[Back to Model list]](../README.md#documentation-for-models) [[Back to README]](../README.md)

# **DefaultAPI_peticionPisoPost**
```c
// Enviar una solicitud de llamada de ascensor desde un piso
//
// Procesa una solicitud de llamada de ascensor desde un piso específico, enviando el estado actual de todos los ascensores gestionados. La solicitud se envía al servidor central a través de CoAP sobre DTLS-PSK. 
//
server_response_t* DefaultAPI_peticionPisoPost(apiClient_t *apiClient, floor_call_request_t *floor_call_request);
```

### Parameters
Name | Type | Description  | Notes
------------- | ------------- | ------------- | -------------
**apiClient** | **apiClient_t \*** | context containing the client configuration |
**floor_call_request** | **[floor_call_request_t](floor_call_request.md) \*** |  | 

### Return type

[server_response_t](server_response.md) *


### Authorization

No authorization required

### HTTP request headers

 - **Content-Type**: application/json
 - **Accept**: application/json

[[Back to top]](#) [[Back to API list]](../README.md#documentation-for-api-endpoints) [[Back to Model list]](../README.md#documentation-for-models) [[Back to README]](../README.md)

