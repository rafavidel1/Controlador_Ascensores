#include <stdlib.h>
#include <stdio.h>
#include "../include/apiClient.h"
#include "../include/list.h"
#include "../external/cJSON.h"
#include "../include/keyValuePair.h"
#include "../include/binary.h"
#include "../model/cabin_request.h"
#include "../model/emergency_call_request.h"
#include "../model/emergency_response.h"
#include "../model/error_response.h"
#include "../model/floor_call_request.h"
#include "../model/server_response.h"


// Procesar llamadas de emergencia desde ascensores
//
// Procesa solicitudes de emergencia originadas desde cabinas o controladores de ascensores. Activa protocolos de emergencia y notifica a servicios de mantenimiento y sistemas de seguridad.  Tipos de emergencia soportados: - EMERGENCY_STOP: Botón de parada activado - POWER_FAILURE: Fallo de alimentación detectado - PEOPLE_TRAPPED: Personas atrapadas en cabina - MECHANICAL_FAILURE: Fallo mecánico del ascensor - FIRE_ALARM: Alarma de incendios activada 
//
emergency_response_t*
DefaultAPI_llamadaEmergenciaPost(apiClient_t *apiClient, emergency_call_request_t *emergency_call_request);


// Enviar una solicitud de destino desde la cabina de un ascensor
//
// Procesa una solicitud de destino desde el interior de una cabina de ascensor, enviando el estado actual de todos los ascensores gestionados. La solicitud se envía al servidor central a través de CoAP sobre DTLS-PSK. 
//
server_response_t*
DefaultAPI_peticionCabinaPost(apiClient_t *apiClient, cabin_request_t *cabin_request);


// Enviar una solicitud de llamada de ascensor desde un piso
//
// Procesa una solicitud de llamada de ascensor desde un piso específico, enviando el estado actual de todos los ascensores gestionados. La solicitud se envía al servidor central a través de CoAP sobre DTLS-PSK. 
//
server_response_t*
DefaultAPI_peticionPisoPost(apiClient_t *apiClient, floor_call_request_t *floor_call_request);


