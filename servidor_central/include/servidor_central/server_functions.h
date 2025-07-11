/**
 * @file server_functions.h
 * @brief Funciones principales del servidor central de control de ascensores
 * @author Sistema de Control de Ascensores
 * @version 2.0
 * @date 2025
 * @details Este archivo contiene las declaraciones de las funciones principales
 * del servidor central que maneja la comunicación CoAP/DTLS y la gestión
 * de asignación inteligente de ascensores en tiempo real.
 * 
 * @see main.c
 * @see dtls_common_config.h
 * @see logging.h
 */

#ifndef SERVER_FUNCTIONS_H
#define SERVER_FUNCTIONS_H

#include <coap3/coap.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Estructura para manejar el contexto del servidor
 * 
 * @details Esta estructura contiene todos los elementos necesarios para
 * el funcionamiento del servidor central, incluyendo el contexto CoAP,
 * la configuración DTLS y el estado del servidor. El sistema es completamente
 * stateless y procesa cada petición independientemente.
 */
typedef struct {
    coap_context_t *ctx;        /**< Contexto CoAP del servidor */
    SSL_CTX *ssl_ctx;          /**< Contexto SSL para DTLS */
    int running;               /**< Flag de estado del servidor (1=activo, 0=detenido) */
    char *auth_file;           /**< Ruta al archivo de configuración de autenticación */
    int port;                  /**< Puerto de escucha del servidor */
} server_context_t;

/**
 * @brief Inicializa el contexto del servidor
 * 
 * @param[out] server_ctx Puntero a la estructura de contexto del servidor
 * @param[in] port Puerto en el que escuchará el servidor
 * @param[in] auth_file Ruta al archivo de configuración de autenticación
 * 
 * @return 0 en caso de éxito, -1 en caso de error
 * 
 * @details Esta función inicializa todos los componentes necesarios para
 * el funcionamiento del servidor:
 * - Configura el contexto CoAP
 * - Inicializa la configuración DTLS
 * - Configura los recursos CoAP
 * - Prepara el algoritmo inteligente stateless
 * 
 * @note La función debe ser llamada antes de iniciar el servidor
 * @see cleanup_server_context
 */
int init_server_context(server_context_t *server_ctx, int port, const char *auth_file);

/**
 * @brief Limpia y libera los recursos del contexto del servidor
 * 
 * @param[in,out] server_ctx Puntero a la estructura de contexto del servidor
 * 
 * @details Esta función libera todos los recursos asociados al servidor:
 * - Cierra el contexto CoAP
 * - Libera la configuración SSL
 * - Limpia el sistema de validación de autenticación
 * 
 * @note Debe ser llamada al finalizar el servidor para evitar memory leaks
 * @see init_server_context
 */
void cleanup_server_context(server_context_t *server_ctx);

/**
 * @brief Inicia el bucle principal del servidor
 * 
 * @param[in] server_ctx Puntero a la estructura de contexto del servidor
 * 
 * @return 0 en caso de éxito, -1 en caso de error
 * 
 * @details Esta función inicia el bucle principal del servidor que:
 * - Escucha conexiones entrantes
 * - Procesa solicitudes CoAP
 * - Maneja la autenticación DTLS segura
 * - Ejecuta el algoritmo inteligente de asignación de ascensores
 * - Responde a los clientes con asignaciones optimizadas
 * 
 * @note Esta función es bloqueante y solo retorna cuando el servidor se detiene
 * @see init_server_context
 */
int run_server(server_context_t *server_ctx);

/**
 * @brief Configura los recursos CoAP del servidor
 * 
 * @param[in] ctx Contexto CoAP del servidor
 * 
 * @return 0 en caso de éxito, -1 en caso de error
 * 
 * @details Esta función registra los endpoints CoAP disponibles:
 * - /peticion_piso: Para solicitudes de asignación de ascensor
 * - /peticion_cabina: Para solicitudes específicas de cabina
 * - /.well-known/core: Para descubrimiento de recursos
 * 
 * @note Debe ser llamada después de inicializar el contexto CoAP
 */
int setup_coap_resources(coap_context_t *ctx);

/**
 * @brief Valida las credenciales contra el archivo de configuración
 * 
 * @param[in] identity Identidad del cliente
 * @param[in] credentials Credenciales proporcionadas por el cliente
 * @param[in] auth_file Ruta al archivo de configuración de autenticación
 * 
 * @return 1 si las credenciales son válidas, 0 en caso contrario
 * 
 * @details Esta función valida la autenticación DTLS:
 * - Lee el archivo de configuración de autenticación
 * - Busca la identidad del cliente
 * - Compara las credenciales proporcionadas con las almacenadas
 * - Retorna el resultado de la validación
 * 
 * @note El archivo debe contener pares identity:credentials
 * @see psk_validator.h
 */
int validate_psk(const char *identity, const char *credentials, const char *auth_file);

/**
 * @brief Procesa una solicitud de asignación de ascensor
 * 
 * @param[in] session Sesión CoAP del cliente
 * @param[in] request Petición CoAP recibida
 * @param[in] response Respuesta CoAP a enviar
 * 
 * @return 0 en caso de éxito, -1 en caso de error
 * 
 * @details Esta función procesa las solicitudes de asignación:
 * - Valida el formato JSON de la petición
 * - Analiza el estado actual de los ascensores recibido en el payload
 * - Ejecuta el algoritmo inteligente de asignación óptima
 * - Genera la respuesta JSON con la asignación
 * - Procesa todo de forma stateless sin persistencia
 * 
 * @note Esta función es llamada automáticamente por el handler CoAP
 */
int handle_floor_request(coap_session_t *session, coap_pdu_t *request, 
                        coap_pdu_t *response);

/**
 * @brief Procesa una solicitud específica de cabina
 * 
 * @param[in] session Sesión CoAP del cliente
 * @param[in] request Petición CoAP recibida
 * @param[in] response Respuesta CoAP a enviar
 * 
 * @return 0 en caso de éxito, -1 en caso de error
 * 
 * @details Esta función procesa las solicitudes de cabina específica:
 * - Valida que el ascensor solicitado esté disponible en el payload
 * - Verifica que el ascensor pueda atender la solicitud
 * - Auto-asigna la tarea al ascensor específico
 * - Genera la respuesta de confirmación
 * - Procesa todo de forma stateless
 * 
 * @note Esta función es llamada automáticamente por el handler CoAP
 */
int handle_cabin_request(coap_session_t *session, coap_pdu_t *request, 
                        coap_pdu_t *response);

/**
 * @brief Ejecuta el algoritmo inteligente de asignación de ascensores
 * 
 * @param[in] edificio_id ID del edificio
 * @param[in] piso_origen Piso de origen de la solicitud
 * @param[in] piso_destino Piso de destino de la solicitud
 * @param[out] ascensor_asignado ID del ascensor asignado
 * @param[out] tiempo_estimado Tiempo estimado de llegada en segundos
 * 
 * @return 0 en caso de éxito, -1 en caso de error
 * 
 * @details Este algoritmo implementa la lógica de asignación inteligente:
 * - Analiza todos los ascensores disponibles recibidos en el payload
 * - Calcula la distancia de cada ascensor al piso de origen
 * - Considera la dirección de movimiento y posición actual
 * - Evalúa compatibilidad de ruta para ascensores ocupados
 * - Selecciona el ascensor que minimice el tiempo de espera
 * - Retorna la asignación óptima basada en datos recibidos en tiempo real
 * 
 * @note El algoritmo prioriza ascensores libres sobre ocupados
 * @note Considera ascensores en movimiento que pueden recoger en ruta
 * @note Procesa todo de forma stateless usando solo datos del payload
 */
int assign_elevator(const char *edificio_id, int piso_origen, int piso_destino,
                   char **ascensor_asignado, int *tiempo_estimado);

#ifdef __cplusplus
}
#endif

#endif /* SERVER_FUNCTIONS_H */ 