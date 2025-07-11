/**
 * @file psk_validator.h
 * @brief Validador de credenciales para autenticación DTLS
 * @author Sistema de Control de Ascensores
 * @version 2.0
 * @date 2025
 * 
 * @details Este archivo define las funciones para validar y gestionar credenciales
 * de autenticación utilizadas en la autenticación DTLS del servidor central.
 * El sistema utiliza un archivo de configuración con credenciales únicas
 * para garantizar la seguridad de las comunicaciones.
 * 
 * @see dtls_common_config.h
 * @see server_functions.h
 */

#ifndef PSK_VALIDATOR_H
#define PSK_VALIDATOR_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Inicializa el validador de credenciales
 * 
 * @param[in] keys_file_path Ruta al archivo de configuración de autenticación
 * 
 * @return 0 si se inicializó correctamente, -1 en caso de error
 * 
 * @details Esta función inicializa el sistema de validación de credenciales:
 * - Abre y lee el archivo de configuración de autenticación
 * - Carga las credenciales en memoria para acceso rápido
 * - Valida el formato del archivo de configuración
 * - Configura el sistema de búsqueda de credenciales
 * 
 * @note El archivo debe contener pares identity:credentials
 * @note Debe ser llamada antes de usar cualquier función de validación
 * @see psk_validator_cleanup
 */
int psk_validator_init(const char* keys_file_path);

/**
 * @brief Valida si unas credenciales están en la lista de credenciales válidas
 * 
 * @param[in] key Credenciales a validar
 * @param[in] key_len Longitud de las credenciales en bytes
 * 
 * @return 1 si las credenciales son válidas, 0 si no lo son
 * 
 * @details Esta función valida unas credenciales contra la lista de credenciales válidas:
 * - Busca las credenciales en la tabla hash de credenciales cargadas
 * - Compara las credenciales proporcionadas con las almacenadas
 * - Retorna el resultado de la validación
 * 
 * @note La búsqueda es O(1) gracias al uso de tabla hash
 * @see psk_validator_get_key_for_identity
 */
int psk_validator_check_key(const char* key, size_t key_len);

/**
 * @brief Obtiene credenciales válidas para una identidad específica
 * 
 * @param[in] identity Identidad del cliente
 * @param[out] key_buffer Buffer donde se almacenarán las credenciales
 * @param[in] buffer_size Tamaño del buffer en bytes
 * 
 * @return 0 si se obtuvo correctamente, -1 en caso de error
 * 
 * @details Esta función obtiene las credenciales correspondientes a una identidad:
 * - Busca la identidad en el archivo de configuración
 * - Copia las credenciales al buffer proporcionado
 * - Verifica que el buffer tenga espacio suficiente
 * - Retorna el resultado de la operación
 * 
 * @note El buffer debe tener al menos 128 bytes
 * @see psk_validator_check_key
 */
int psk_validator_get_key_for_identity(const char* identity, uint8_t* key_buffer, size_t buffer_size);

/**
 * @brief Obtiene credenciales por índice específico
 * 
 * @param[in] index Índice de las credenciales en el archivo (0-based)
 * @param[out] key_buffer Buffer donde se almacenarán las credenciales
 * @param[in] buffer_size Tamaño del buffer en bytes
 * 
 * @return 0 si se obtuvo correctamente, -1 en caso de error
 * 
 * @details Esta función obtiene credenciales por su posición en el archivo:
 * - Lee la línea correspondiente al índice
 * - Parsea las credenciales del formato identity:credentials
 * - Copia las credenciales al buffer proporcionado
 * - Valida que el índice esté dentro del rango válido
 * 
 * @note El índice debe estar entre 0 y el número total de credenciales-1
 * @see psk_validator_get_key_for_identity
 */
int psk_validator_get_key_by_index(int index, uint8_t* key_buffer, size_t buffer_size);

/**
 * @brief Libera los recursos del validador de credenciales
 * 
 * @details Esta función limpia todos los recursos asociados al validador:
 * - Cierra el archivo de configuración si está abierto
 * - Libera la memoria de la tabla hash de credenciales
 * - Resetea el estado del validador
 * - Libera cualquier buffer interno
 * 
 * @note Debe ser llamada al finalizar para evitar memory leaks
 * @see psk_validator_init
 */
void psk_validator_cleanup(void);

/**
 * @brief Obtiene el número total de credenciales disponibles
 * 
 * @return Número de credenciales en el archivo
 * 
 * @details Esta función retorna el número total de credenciales que han sido
 * cargadas desde el archivo de configuración.
 * 
 * @note Solo es válida después de llamar a psk_validator_init
 */
int psk_validator_get_key_count(void);

/**
 * @brief Verifica si el validador está inicializado
 * 
 * @return 1 si está inicializado, 0 en caso contrario
 * 
 * @details Esta función verifica si el validador de credenciales ha sido
 * inicializado correctamente y está listo para su uso.
 * 
 * @note Útil para verificar el estado antes de usar otras funciones
 */
int psk_validator_is_initialized(void);

#ifdef __cplusplus
}
#endif

#endif /* PSK_VALIDATOR_H */ 