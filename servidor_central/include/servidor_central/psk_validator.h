/**
 * @file psk_validator.h
 * @brief Validador de claves PSK para autenticación DTLS
 * @author Sistema de Control de Ascensores
 * @version 2.0
 * @date 2025
 * 
 * @details Este archivo define las funciones para validar y gestionar claves PSK
 * (Pre-Shared Keys) utilizadas en la autenticación DTLS-PSK del servidor central.
 * El sistema utiliza un archivo de 15,000 claves únicas pre-generadas para
 * garantizar la seguridad de las comunicaciones.
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
 * @brief Inicializa el validador de claves PSK
 * 
 * @param[in] keys_file_path Ruta al archivo de claves PSK
 * 
 * @return 0 si se inicializó correctamente, -1 en caso de error
 * 
 * @details Esta función inicializa el sistema de validación de claves PSK:
 * - Abre y lee el archivo de claves PSK
 * - Carga las claves en memoria para acceso rápido
 * - Valida el formato del archivo de claves
 * - Configura el sistema de búsqueda de claves
 * 
 * @note El archivo de claves debe contener pares identity:key
 * @note Debe ser llamada antes de usar cualquier función de validación
 * @see psk_validator_cleanup
 */
int psk_validator_init(const char* keys_file_path);

/**
 * @brief Valida si una clave PSK está en la lista de claves válidas
 * 
 * @param[in] key Clave PSK a validar
 * @param[in] key_len Longitud de la clave en bytes
 * 
 * @return 1 si la clave es válida, 0 si no lo es
 * 
 * @details Esta función valida una clave PSK contra la lista de claves válidas:
 * - Busca la clave en la tabla hash de claves cargadas
 * - Compara la clave proporcionada con las almacenadas
 * - Retorna el resultado de la validación
 * 
 * @note La búsqueda es O(1) gracias al uso de tabla hash
 * @see psk_validator_get_key_for_identity
 */
int psk_validator_check_key(const char* key, size_t key_len);

/**
 * @brief Obtiene una clave PSK válida para una identidad específica
 * 
 * @param[in] identity Identidad del cliente
 * @param[out] key_buffer Buffer donde se almacenará la clave
 * @param[in] buffer_size Tamaño del buffer en bytes
 * 
 * @return 0 si se obtuvo correctamente, -1 en caso de error
 * 
 * @details Esta función obtiene la clave PSK correspondiente a una identidad:
 * - Busca la identidad en el archivo de claves
 * - Copia la clave al buffer proporcionado
 * - Verifica que el buffer tenga espacio suficiente
 * - Retorna el resultado de la operación
 * 
 * @note El buffer debe tener al menos MAX_PSK_LENGTH bytes
 * @see psk_validator_check_key
 */
int psk_validator_get_key_for_identity(const char* identity, uint8_t* key_buffer, size_t buffer_size);

/**
 * @brief Obtiene una clave PSK por índice específico
 * 
 * @param[in] index Índice de la clave en el archivo (0-based)
 * @param[out] key_buffer Buffer donde se almacenará la clave
 * @param[in] buffer_size Tamaño del buffer en bytes
 * 
 * @return 0 si se obtuvo correctamente, -1 en caso de error
 * 
 * @details Esta función obtiene una clave PSK por su posición en el archivo:
 * - Lee la línea correspondiente al índice
 * - Parsea la clave del formato identity:key
 * - Copia la clave al buffer proporcionado
 * - Valida que el índice esté dentro del rango válido
 * 
 * @note El índice debe estar entre 0 y NUM_PSK_KEYS-1
 * @see psk_validator_get_key_for_identity
 */
int psk_validator_get_key_by_index(int index, uint8_t* key_buffer, size_t buffer_size);

/**
 * @brief Libera los recursos del validador de claves PSK
 * 
 * @details Esta función limpia todos los recursos asociados al validador:
 * - Cierra el archivo de claves si está abierto
 * - Libera la memoria de la tabla hash de claves
 * - Resetea el estado del validador
 * - Libera cualquier buffer interno
 * 
 * @note Debe ser llamada al finalizar para evitar memory leaks
 * @see psk_validator_init
 */
void psk_validator_cleanup(void);

/**
 * @brief Obtiene el número total de claves PSK disponibles
 * 
 * @return Número de claves PSK en el archivo
 * 
 * @details Esta función retorna el número total de claves PSK que han sido
 * cargadas desde el archivo de claves.
 * 
 * @note Solo es válida después de llamar a psk_validator_init
 */
int psk_validator_get_key_count(void);

/**
 * @brief Verifica si el validador está inicializado
 * 
 * @return 1 si está inicializado, 0 en caso contrario
 * 
 * @details Esta función verifica si el validador de claves PSK ha sido
 * inicializado correctamente y está listo para su uso.
 * 
 * @note Útil para verificar el estado antes de usar otras funciones
 */
int psk_validator_is_initialized(void);

#ifdef __cplusplus
}
#endif

#endif /* PSK_VALIDATOR_H */ 