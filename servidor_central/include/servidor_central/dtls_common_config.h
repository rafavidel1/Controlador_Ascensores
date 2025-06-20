/**
 * @file dtls_common_config.h
 * @brief Configuraciones DTLS-PSK para el Servidor Central
 * @author Sistema de Control de Ascensores
 * @date 2025
 * @version 2.0
 * 
 * Este archivo define las configuraciones de seguridad DTLS-PSK (Pre-Shared Key)
 * utilizadas por el servidor central para autenticar y cifrar comunicaciones
 * con los API Gateways.
 * 
 * **Configuraciones incluidas:**
 * - Identidades de clientes esperadas
 * - Claves PSK compartidas
 * - Hints del servidor para selección de credenciales
 * 
 * **Seguridad:**
 * Las claves definidas aquí deben mantenerse sincronizadas con las
 * configuraciones de los API Gateways para garantizar la autenticación mutua.
 * 
 * @see api_gateway/dtls_common_config.h
 * @see coap_config.h
 */
#ifndef DTLS_COMMON_CONFIG_SC_H
#define DTLS_COMMON_CONFIG_SC_H

/**
 * @brief Identidad PSK esperada del cliente API Gateway
 * 
 * Define la identidad que el servidor central espera recibir
 * de los API Gateways durante el handshake DTLS-PSK.
 * 
 * Esta identidad debe coincidir exactamente con la configurada
 * en el API Gateway (IDENTITY_TO_PRESENT_TO_SERVER).
 */
#define PSK_CLIENT_IDENTITY "Gateway_Client_001"

/**
 * @brief Clave PSK compartida para autenticación
 * 
 * Clave secreta compartida utilizada para autenticar y cifrar
 * las comunicaciones DTLS entre el servidor central y los API Gateways.
 * 
 * **CRÍTICO:** Esta clave debe ser idéntica en:
 * - Servidor Central: PSK_KEY
 * - API Gateway: KEY_FOR_SERVER
 * 
 * @warning Mantener esta clave segura y sincronizada entre componentes
 */
#define PSK_KEY "SecretGatewayServidorCentralKey"

/**
 * @brief Hint del servidor para selección de credenciales
 * 
 * Hint enviado por el servidor central a los clientes durante
 * el handshake DTLS-PSK para ayudar en la selección de credenciales.
 * 
 * Útil cuando los clientes manejan múltiples identidades/claves
 * y necesitan seleccionar las apropiadas para este servidor.
 */
#define PSK_SERVER_HINT "ServidorCentralHint"

#endif // DTLS_COMMON_CONFIG_SC_H 