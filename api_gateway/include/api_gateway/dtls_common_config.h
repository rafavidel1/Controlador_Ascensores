/**
 * @file dtls_common_config.h
 * @brief Configuraciones DTLS-PSK para el API Gateway
 * @author Sistema de Control de Ascensores
 * @date 2025
 * @version 2.0
 * 
 * Este archivo define las configuraciones de seguridad DTLS-PSK (Pre-Shared Key)
 * utilizadas por el API Gateway para autenticarse con el servidor central.
 * 
 * **Configuraciones incluidas:**
 * - Identidad del cliente gateway para presentar al servidor
 * - Clave PSK compartida para autenticación mutua
 * - Configuraciones de seguridad DTLS
 * 
 * **Seguridad:**
 * Las credenciales definidas aquí deben mantenerse sincronizadas con las
 * configuraciones del servidor central para garantizar la autenticación mutua.
 * 
 * **Flujo de autenticación:**
 * 1. Gateway presenta IDENTITY_TO_PRESENT_TO_SERVER al servidor
 * 2. Servidor valida la identidad contra PSK_CLIENT_IDENTITY
 * 3. Ambos usan KEY_FOR_SERVER/PSK_KEY para cifrado
 * 4. Se establece canal DTLS seguro
 * 
 * @see servidor_central/dtls_common_config.h
 * @see coap_config.h
 */
#ifndef DTLS_COMMON_CONFIG_GW_H
#define DTLS_COMMON_CONFIG_GW_H

/**
 * @brief Identidad que el Gateway presenta al Servidor Central
 * 
 * Esta identidad es enviada por el API Gateway al servidor central
 * durante el handshake DTLS-PSK para identificarse como cliente autorizado.
 * 
 * Debe coincidir exactamente con PSK_CLIENT_IDENTITY configurado
 * en el servidor central.
 * 
 * @see servidor_central/dtls_common_config.h::PSK_CLIENT_IDENTITY
 */
#define IDENTITY_TO_PRESENT_TO_SERVER "Gateway_Client_001"

/**
 * @brief Clave PSK compartida para autenticación con el servidor
 * 
 * Clave secreta compartida utilizada por el API Gateway para autenticarse
 * con el servidor central. Esta clave debe ser idéntica a la configurada
 * en el servidor central.
 * 
 * **CRÍTICO:** Esta clave debe ser idéntica en:
 * - API Gateway: KEY_FOR_SERVER
 * - Servidor Central: PSK_KEY
 * 
 * La seguridad del sistema depende de mantener esta clave secreta
 * y sincronizada entre ambos componentes.
 * 
 * @warning Mantener esta clave segura y no exponerla en logs o código público
 * @see servidor_central/dtls_common_config.h::PSK_KEY
 */
#define KEY_FOR_SERVER "SecretGatewayServidorCentralKey"

/**
 * @brief Longitud de la clave PSK en bytes
 * 
 * Longitud en bytes de la clave PSK definida en KEY_FOR_SERVER.
 * Utilizada para operaciones criptográficas que requieren conocer
 * el tamaño exacto de la clave.
 * 
 * @note Se calcula automáticamente basado en KEY_FOR_SERVER
 */
#define PSK_KEY_LENGTH (sizeof(KEY_FOR_SERVER) - 1)

/**
 * @brief Longitud máxima permitida para identidades PSK
 * 
 * Tamaño máximo del buffer para almacenar identidades PSK.
 * Debe ser suficiente para contener IDENTITY_TO_PRESENT_TO_SERVER
 * más el terminador nulo.
 */
#define PSK_IDENTITY_MAX_LENGTH 64

/**
 * @brief Longitud máxima permitida para claves PSK
 * 
 * Tamaño máximo del buffer para almacenar claves PSK.
 * Debe ser suficiente para contener KEY_FOR_SERVER más
 * margen para futuras expansiones.
 */
#define PSK_KEY_MAX_LENGTH 128

#endif // DTLS_COMMON_CONFIG_GW_H 