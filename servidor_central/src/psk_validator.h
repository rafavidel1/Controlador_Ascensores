/**
 * @file psk_validator.h
 * @brief Validador de claves PSK para Servidor Central
 * @author Sistema de Control de Ascensores
 * @date 2025
 * @version 1.0
 * 
 * Este archivo define las funciones para validar claves PSK
 * desde un archivo de claves predefinidas.
 */

#ifndef PSK_VALIDATOR_H
#define PSK_VALIDATOR_H

#include <stddef.h>
#include <stdint.h>

/**
 * @brief Inicializa el validador de claves PSK
 * @param keys_file_path Ruta al archivo de claves PSK
 * @return 0 si se inicializó correctamente, -1 en caso de error
 */
int psk_validator_init(const char* keys_file_path);

/**
 * @brief Valida si una clave PSK está en la lista de claves válidas
 * @param key Clave PSK a validar
 * @param key_len Longitud de la clave
 * @return 1 si la clave es válida, 0 si no lo es
 */
int psk_validator_check_key(const char* key, size_t key_len);

/**
 * @brief Obtiene una clave PSK válida para una identidad específica
 * @param identity Identidad del cliente
 * @param key_buffer Buffer donde se almacenará la clave
 * @param buffer_size Tamaño del buffer
 * @return 0 si se obtuvo correctamente, -1 en caso de error
 */
int psk_validator_get_key_for_identity(const char* identity, uint8_t* key_buffer, size_t buffer_size);

/**
 * @brief Obtiene una clave PSK por índice específico
 * @param index Índice de la clave en el archivo
 * @param key_buffer Buffer donde se almacenará la clave
 * @param buffer_size Tamaño del buffer
 * @return 0 si se obtuvo correctamente, -1 en caso de error
 */
int psk_validator_get_key_by_index(int index, uint8_t* key_buffer, size_t buffer_size);

/**
 * @brief Libera los recursos del validador de claves PSK
 */
void psk_validator_cleanup(void);

#endif // PSK_VALIDATOR_H 