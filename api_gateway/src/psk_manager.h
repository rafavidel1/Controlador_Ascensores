/**
 * @file psk_manager.h
 * @brief Gestor de claves PSK para API Gateway
 * @author Sistema de Control de Ascensores
 * @date 2025
 * @version 1.0
 * 
 * Este archivo define las funciones para gestionar claves PSK
 * desde un archivo de claves predefinidas.
 */

#ifndef PSK_MANAGER_H
#define PSK_MANAGER_H

#include <stddef.h>

/**
 * @brief Inicializa el gestor de claves PSK
 * @param keys_file_path Ruta al archivo de claves PSK
 * @return 0 si se inicializó correctamente, -1 en caso de error
 */
int psk_manager_init(const char* keys_file_path);

/**
 * @brief Obtiene una clave PSK aleatoria del archivo
 * @param key_buffer Buffer donde se almacenará la clave
 * @param buffer_size Tamaño del buffer
 * @return 0 si se obtuvo correctamente, -1 en caso de error
 */
int psk_manager_get_random_key(char* key_buffer, size_t buffer_size);

/**
 * @brief Obtiene la primera clave PSK del archivo como fallback
 * @param key_buffer Buffer donde se almacenará la clave
 * @param buffer_size Tamaño del buffer
 * @return 0 si se obtuvo correctamente, -1 en caso de error
 */
int psk_manager_get_first_key(char* key_buffer, size_t buffer_size);

/**
 * @brief Obtiene una clave PSK determinística basada en la identidad
 * @param identity Identidad del cliente
 * @param key_buffer Buffer donde se almacenará la clave
 * @param buffer_size Tamaño del buffer
 * @return 0 si se obtuvo correctamente, -1 en caso de error
 */
int psk_manager_get_deterministic_key(const char* identity, char* key_buffer, size_t buffer_size);

/**
 * @brief Libera los recursos del gestor de claves PSK
 */
void psk_manager_cleanup(void);

#endif // PSK_MANAGER_H 