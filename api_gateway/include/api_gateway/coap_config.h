/**
 * @file coap_config.h
 * @brief Configuraciones CoAP para el API Gateway
 * @author Sistema de Control de Ascensores
 * @date 2025
 * @version 2.0
 * 
 * Este archivo define las configuraciones del protocolo CoAP (Constrained Application Protocol)
 * utilizadas por el API Gateway para comunicarse con el servidor central.
 * 
 * **Configuraciones incluidas:**
 * - Dirección IP y puerto del servidor central
 * - Dirección IP y puerto de escucha del gateway
 * - Rutas de recursos CoAP para diferentes tipos de solicitudes
 * - Configuraciones de timeout y reintentos
 * - Parámetros de conexión DTLS-PSK
 * 
 * **Recursos CoAP definidos:**
 * - `/peticion_piso`: Para solicitudes de llamada de piso
 * - `/peticion_cabina`: Para solicitudes desde cabina de ascensor
 * 
 * Las configuraciones aquí definidas deben coincidir con las rutas
 * y configuraciones del servidor central para garantizar la comunicación.
 * 
 * @see dtls_common_config.h
 * @see servidor_central/main.c
 */
#ifndef COAP_CONFIG_H
#define COAP_CONFIG_H

/**
 * @defgroup gateway_listen Configuración de Escucha del Gateway
 * @brief Configuraciones para el servidor CoAP del API Gateway
 * @{
 */

/**
 * @brief Dirección IP de escucha del API Gateway
 * 
 * Dirección IPv4 en la que el API Gateway escuchará conexiones CoAP
 * desde simuladores de ascensores y otros clientes.
 * 
 * @note "0.0.0.0" escucha en todas las interfaces disponibles
 */
#define GW_LISTEN_IP "0.0.0.0"

/**
 * @brief Puerto de escucha del API Gateway
 * 
 * Puerto UDP en el que el API Gateway escuchará conexiones CoAP
 * desde simuladores de ascensores y otros clientes.
 */
#define GW_LISTEN_PORT "5683"

/** @} */ // end of gateway_listen group

/**
 * @defgroup central_server Configuración del Servidor Central
 * @brief Configuraciones para conectar con el servidor central
 * @{
 */

/**
 * @brief Dirección IP del servidor central
 * 
 * Dirección IPv4 donde se ejecuta el servidor central de asignación
 * de ascensores. El API Gateway se conectará a esta dirección para
 * enviar solicitudes CoAP.
 * 
 * @note Cambiar a la IP real del servidor en entornos de producción
 */
#define CENTRAL_SERVER_IP "192.168.49.2"

/**
 * @brief Puerto del servidor central
 * 
 * Puerto UDP donde el servidor central escucha conexiones CoAP.
 * Debe coincidir con el puerto configurado en el servidor central.
 * 
 * @see servidor_central/main.c::SERVER_PORT
 */
#define CENTRAL_SERVER_PORT "5684"

/** @} */ // end of central_server group

/**
 * @defgroup coap_resources Recursos CoAP
 * @brief Definiciones de rutas de recursos CoAP
 * @{
 */

/**
 * @brief Ruta CoAP para solicitudes de llamada de piso
 * 
 * Recurso CoAP utilizado para enviar solicitudes de llamada de piso
 * al servidor central. Estas solicitudes se originan desde botones
 * externos en los pisos.
 * 
 * **Formato de payload JSON esperado:**
 * ```json
 * {
 *   "piso_origen": 3,
 *   "direccion_solicitada": "up"
 * }
 * ```
 */
#define FLOOR_CALL_RESOURCE "peticion_piso"

/**
 * @brief Ruta CoAP para solicitudes de cabina
 * 
 * Recurso CoAP utilizado para enviar solicitudes desde el interior
 * de los ascensores al servidor central. Estas solicitudes incluyen
 * el ID del ascensor y el piso destino.
 * 
 * **Formato de payload JSON esperado:**
 * ```json
 * {
 *   "ascensor_id": "E1A1",
 *   "piso_destino": 5
 * }
 * ```
 */
#define CABIN_REQUEST_RESOURCE "peticion_cabina"

/**
 * @brief Ruta completa CoAP para solicitudes de llamada de piso (CAN Bridge)
 * 
 * Ruta completa utilizada por el puente CAN para enviar solicitudes
 * de llamada de piso al servidor central. Incluye la barra inicial.
 * 
 * @see FLOOR_CALL_RESOURCE
 */
#define CENTRAL_SERVER_FLOOR_CALL_PATH "/" FLOOR_CALL_RESOURCE

/**
 * @brief Ruta completa CoAP para solicitudes de cabina (CAN Bridge)
 * 
 * Ruta completa utilizada por el puente CAN para enviar solicitudes
 * de cabina al servidor central. Incluye la barra inicial.
 * 
 * @see CABIN_REQUEST_RESOURCE
 */
#define CENTRAL_SERVER_CABIN_REQUEST_PATH "/" CABIN_REQUEST_RESOURCE

/** @} */ // end of coap_resources group

/**
 * @defgroup coap_timeouts Configuraciones de Timeout
 * @brief Configuraciones de tiempo para solicitudes CoAP
 * @{
 */

/**
 * @brief Timeout para solicitudes CoAP en milisegundos
 * 
 * Tiempo máximo de espera para recibir una respuesta del servidor central
 * antes de considerar la solicitud como fallida.
 * 
 * @note Ajustar según la latencia de red esperada en el entorno de despliegue
 */
#define COAP_REQUEST_TIMEOUT_MS 5000

/**
 * @brief Número máximo de reintentos para solicitudes CoAP
 * 
 * Cantidad de reintentos automáticos que realizará el cliente CoAP
 * si no recibe respuesta del servidor central dentro del timeout.
 * 
 * @see COAP_REQUEST_TIMEOUT_MS
 */
#define COAP_MAX_RETRIES 3

/** @} */ // end of coap_timeouts group

#endif // COAP_CONFIG_H 