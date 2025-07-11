/**
 * @file dtls_common_config.h
 * @brief Configuración común para DTLS en el servidor central
 * @author Sistema de Control de Ascensores
 * @version 2.0
 * @date 2025
 * 
 * @details Este archivo contiene las configuraciones y constantes necesarias
 * para la implementación de DTLS seguro en el servidor central.
 * Define los parámetros de seguridad, timeouts y configuraciones de sesión.
 * 
 * @see server_functions.h
 * @see psk_validator.h
 */

#ifndef DTLS_COMMON_CONFIG_H
#define DTLS_COMMON_CONFIG_H

#include <openssl/ssl.h>
#include <openssl/err.h>
#include <coap3/coap.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Configuración de seguridad DTLS
 * 
 * @details Esta estructura contiene todos los parámetros necesarios para
 * configurar la seguridad DTLS del servidor central.
 */
typedef struct {
    char *auth_file;            /**< Ruta al archivo de configuración de autenticación */
    int auth_timeout;           /**< Timeout de sesión de autenticación en segundos */
    int dtls_mtu;               /**< MTU para DTLS en bytes */
    int retransmit_timeout;     /**< Timeout de retransmisión en segundos */
    int max_connections;        /**< Número máximo de conexiones simultáneas */
    int session_cache_size;     /**< Tamaño del caché de sesiones */
} dtls_config_t;

/**
 * @brief Configuración por defecto para DTLS
 * 
 * @details Valores recomendados para un entorno de producción:
 * - Timeout de 30 segundos para sesiones de autenticación
 * - MTU de 1280 bytes (estándar CoAP)
 * - Timeout de retransmisión de 2 segundos
 * - Máximo 100 conexiones simultáneas
 * - Caché de 50 sesiones
 */
#define DEFAULT_DTLS_CONFIG { \
    .auth_file = "/app/psk_keys.txt", \
    .auth_timeout = 30, \
    .dtls_mtu = 1280, \
    .retransmit_timeout = 2, \
    .max_connections = 100, \
    .session_cache_size = 50 \
}

/**
 * @brief Puerto por defecto para DTLS
 * 
 * @details El puerto 5684 es el estándar para CoAP sobre DTLS según RFC 7252
 */
#define DEFAULT_DTLS_PORT 5684

/**
 * @brief Tamaño máximo de credenciales
 * 
 * @details Las credenciales pueden tener hasta 128 caracteres
 */
#define MAX_AUTH_LENGTH 128

/**
 * @brief Tamaño máximo de identidad
 * 
 * @details Las identidades pueden tener hasta 64 caracteres
 */
#define MAX_AUTH_IDENTITY_LENGTH 64

/**
 * @brief Número de credenciales disponibles
 * 
 * @details El sistema incluye múltiples credenciales únicas para máxima seguridad
 */
#define NUM_AUTH_CREDENTIALS 15000

/**
 * @brief Inicializa la configuración SSL para DTLS
 * 
 * @param[out] ssl_ctx Contexto SSL a inicializar
 * @param[in] config Configuración DTLS a aplicar
 * 
 * @return 0 en caso de éxito, -1 en caso de error
 * 
 * @details Esta función configura el contexto SSL para DTLS:
 * - Inicializa la biblioteca OpenSSL
 * - Crea el contexto SSL con método DTLS
 * - Configura los parámetros de seguridad
 * - Establece los timeouts y límites de conexión
 * - Configura el caché de sesiones
 * 
 * @note Debe ser llamada antes de crear sesiones SSL
 * @see cleanup_ssl_context
 */
int init_ssl_context(SSL_CTX **ssl_ctx, const dtls_config_t *config);

/**
 * @brief Limpia y libera el contexto SSL
 * 
 * @param[in,out] ssl_ctx Contexto SSL a liberar
 * 
 * @details Esta función libera todos los recursos asociados al contexto SSL:
 * - Libera el contexto SSL
 * - Limpia el caché de sesiones
 * 
 * @note Debe ser llamada al finalizar para evitar memory leaks
 * @see init_ssl_context
 */
void cleanup_ssl_context(SSL_CTX *ssl_ctx);

/**
 * @brief Configura una sesión CoAP con DTLS
 * 
 * @param[in] ctx Contexto CoAP del servidor
 * @param[in] ssl_ctx Contexto SSL configurado
 * @param[in] port Puerto de escucha
 * 
 * @return 0 en caso de éxito, -1 en caso de error
 * 
 * @details Esta función configura la sesión CoAP para DTLS:
 * - Crea el endpoint de escucha DTLS
 * - Configura los parámetros de sesión
 * - Establece los callbacks de autenticación
 * - Configura los timeouts de sesión
 * 
 * @note Debe ser llamada después de inicializar el contexto SSL
 */
int setup_dtls_session(coap_context_t *ctx, SSL_CTX *ssl_ctx, int port);

/**
 * @brief Callback para autenticación del servidor
 * 
 * @param[in] ssl Conexión SSL
 * @param[in] identity Identidad del cliente
 * @param[in] credentials Credenciales del cliente
 * @param[in] max_credentials_len Longitud máxima de credenciales
 * 
 * @return 1 si la autenticación es exitosa, 0 en caso contrario
 * 
 * @details Este callback es llamado durante el handshake DTLS:
 * - Recibe la identidad y credenciales del cliente
 * - Valida las credenciales contra el archivo de configuración
 * - Retorna el resultado de la validación
 * - Configura las credenciales en la sesión SSL
 * 
 * @note Esta función es llamada automáticamente por OpenSSL
 * @see validate_psk_key
 */
int psk_server_callback(SSL *ssl, const char *identity, unsigned char *credentials, 
                       unsigned int max_credentials_len);

/**
 * @brief Valida credenciales contra el archivo de configuración
 * 
 * @param[in] identity Identidad del cliente
 * @param[in] credentials Credenciales a validar
 * @param[in] auth_file Ruta al archivo de configuración de autenticación
 * 
 * @return 1 si las credenciales son válidas, 0 en caso contrario
 * 
 * @details Esta función valida la autenticación:
 * - Lee el archivo de configuración de autenticación
 * - Busca la identidad del cliente
 * - Compara las credenciales proporcionadas con las almacenadas
 * - Retorna el resultado de la validación
 * 
 * @note El archivo debe contener pares identity:credentials
 * @see psk_server_callback
 */
int validate_psk_key(const char *identity, const char *credentials, const char *auth_file);

/**
 * @brief Obtiene la configuración DTLS desde variables de entorno
 * 
 * @param[out] config Configuración DTLS a llenar
 * 
 * @return 0 en caso de éxito, -1 en caso de error
 * 
 * @details Esta función lee la configuración DTLS desde variables de entorno:
 * - DTLS_AUTH_FILE: Ruta al archivo de configuración de autenticación
 * - DTLS_TIMEOUT: Timeout de sesión en segundos
 * - DTLS_MTU: MTU para DTLS en bytes
 * - DTLS_RETRANSMIT_TIMEOUT: Timeout de retransmisión
 * - DTLS_MAX_CONNECTIONS: Número máximo de conexiones
 * - DTLS_SESSION_CACHE_SIZE: Tamaño del caché de sesiones
 * 
 * @note Si una variable no está definida, usa el valor por defecto
 */
int get_dtls_config_from_env(dtls_config_t *config);

/**
 * @brief Configura los parámetros de sesión DTLS
 * 
 * @param[in] ssl Conexión SSL a configurar
 * @param[in] config Configuración DTLS a aplicar
 * 
 * @return 0 en caso de éxito, -1 en caso de error
 * 
 * @details Esta función configura los parámetros de una sesión DTLS:
 * - Establece el timeout de sesión
 * - Configura el MTU para DTLS
 * - Establece el timeout de retransmisión
 * - Configura los parámetros de seguridad
 * 
 * @note Debe ser llamada después de crear la sesión SSL
 */
int configure_dtls_session(SSL *ssl, const dtls_config_t *config);

#ifdef __cplusplus
}
#endif

#endif /* DTLS_COMMON_CONFIG_H */ 