/**
 * @file psk_validator.c
 * @brief Implementación del validador de claves PSK para autenticación DTLS
 * @author Sistema de Control de Ascensores
 * @version 2.0
 * @date 2025
 * 
 * @details Este archivo implementa el sistema de validación de claves PSK
 * utilizado en la autenticación DTLS-PSK del servidor central. El sistema
 * carga claves desde un archivo pre-generado y proporciona funciones para
 * validar y obtener claves de forma determinística basada en la identidad
 * del cliente.
 * 
 * @see psk_validator.h
 * @see dtls_common_config.h
 */

#include "servidor_central/psk_validator.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/**
 * @brief Estructura para almacenar las claves PSK válidas
 * 
 * @details Esta estructura mantiene en memoria todas las claves PSK válidas
 * cargadas desde el archivo de claves. Utiliza un array dinámico para
 * almacenar las claves como strings.
 */
typedef struct {
    char** keys;        /**< Array de punteros a claves PSK */
    int count;          /**< Número actual de claves cargadas */
    int capacity;       /**< Capacidad total del array */
} psk_valid_keys_t;

/**
 * @brief Variable global que almacena las claves PSK válidas
 * 
 * @details Esta variable global mantiene el estado del validador de claves PSK.
 * Contiene todas las claves cargadas desde el archivo de configuración.
 */
static psk_valid_keys_t g_valid_keys = {NULL, 0, 0};

/**
 * @brief Inicializa el validador de claves PSK
 * 
 * @param[in] keys_file_path Ruta al archivo de claves PSK
 * 
 * @return 0 si se inicializó correctamente, -1 en caso de error
 * 
 * @details Esta función inicializa el sistema de validación de claves PSK:
 * - Abre el archivo de claves PSK especificado
 * - Cuenta el número de líneas en el archivo
 * - Asigna memoria dinámica para almacenar las claves
 * - Lee todas las claves del archivo y las almacena en memoria
 * - Cierra el archivo después de la carga
 * 
 * @note El archivo de claves debe contener una clave por línea
 * @note La función maneja automáticamente la memoria dinámica
 * @see psk_validator_cleanup
 */
int psk_validator_init(const char* keys_file_path) {
    FILE* file = fopen(keys_file_path, "r");
    if (!file) {
        fprintf(stderr, "Error: No se pudo abrir el archivo de claves PSK: %s\n", keys_file_path);
        return -1;
    }
    
    // Contar líneas en el archivo
    int line_count = 0;
    char buffer[256];
    while (fgets(buffer, sizeof(buffer), file)) {
        line_count++;
    }
    
    // Inicializar estructura
    g_valid_keys.capacity = line_count;
    g_valid_keys.keys = malloc(line_count * sizeof(char*));
    if (!g_valid_keys.keys) {
        fprintf(stderr, "Error: No se pudo asignar memoria para las claves PSK válidas\n");
        fclose(file);
        return -1;
    }
    
    // Volver al inicio del archivo
    rewind(file);
    
    // Leer claves
    int index = 0;
    while (fgets(buffer, sizeof(buffer), file) && index < line_count) {
        // Remover salto de línea
        size_t len = strlen(buffer);
        if (len > 0 && buffer[len-1] == '\n') {
            buffer[len-1] = '\0';
        }
        
        // Asignar memoria para la clave
        g_valid_keys.keys[index] = strdup(buffer);
        if (!g_valid_keys.keys[index]) {
            fprintf(stderr, "Error: No se pudo asignar memoria para la clave válida %d\n", index);
            fclose(file);
            return -1;
        }
        index++;
    }
    
    g_valid_keys.count = index;
    fclose(file);
    
    printf("PSK Validator: Cargadas %d claves válidas desde %s\n", g_valid_keys.count, keys_file_path);
    return 0;
}

/**
 * @brief Valida si una clave PSK está en la lista de claves válidas
 * 
 * @param[in] key Clave PSK a validar
 * @param[in] key_len Longitud de la clave en bytes
 * 
 * @return 1 si la clave es válida, 0 si no lo es
 * 
 * @details Esta función valida una clave PSK contra la lista de claves válidas:
 * - Verifica que haya claves cargadas en memoria
 * - Compara la clave proporcionada con cada clave almacenada
 * - Utiliza comparación exacta de longitud y contenido
 * - Retorna el resultado de la validación
 * 
 * @note La búsqueda es lineal O(n) donde n es el número de claves
 * @note La función es thread-safe para lecturas concurrentes
 * @see psk_validator_get_key_for_identity
 */
int psk_validator_check_key(const char* key, size_t key_len) {
    if (g_valid_keys.count == 0) {
        fprintf(stderr, "Error: No hay claves PSK válidas cargadas\n");
        return 0;
    }
    
    // Buscar la clave en la lista de claves válidas
    for (int i = 0; i < g_valid_keys.count; i++) {
        if (strlen(g_valid_keys.keys[i]) == key_len && 
            strncmp(g_valid_keys.keys[i], key, key_len) == 0) {
            return 1; // Clave válida encontrada
        }
    }
    
    return 0; // Clave no encontrada
}

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
 * - Calcula un hash determinístico de la identidad
 * - Usa el hash como índice para seleccionar una clave
 * - Copia la clave seleccionada al buffer proporcionado
 * - Verifica que el buffer tenga espacio suficiente
 * 
 * @note La selección es determinística: misma identidad = misma clave
 * @note El algoritmo usa hash simple para distribución uniforme
 * @see psk_validator_check_key
 */
int psk_validator_get_key_for_identity(const char* identity, uint8_t* key_buffer, size_t buffer_size) {
    if (g_valid_keys.count == 0) {
        fprintf(stderr, "Error: No hay claves PSK válidas cargadas\n");
        return -1;
    }
    
    // Usar la identidad como seed para selección determinística
    unsigned int seed = 0;
    for (size_t i = 0; i < strlen(identity); i++) {
        seed = seed * 31 + identity[i];
    }
    
    // Seleccionar clave basada en el hash de la identidad
    int selected_index = seed % g_valid_keys.count;
    const char* selected_key = g_valid_keys.keys[selected_index];
    
    // Copiar clave al buffer
    size_t key_len = strlen(selected_key);
    if (key_len >= buffer_size) {
        fprintf(stderr, "Error: Buffer insuficiente para la clave PSK\n");
        return -1;
    }
    
    memcpy(key_buffer, selected_key, key_len);
    key_buffer[key_len] = '\0'; // Asegurar terminación null
    
    return 0;
}

/**
 * @brief Obtiene una clave PSK por índice específico
 * 
 * @param[in] index Índice de la clave en el archivo (0-based)
 * @param[out] key_buffer Buffer donde se almacenará la clave
 * @param[in] buffer_size Tamaño del buffer en bytes
 * 
 * @return 0 si se obtuvo correctamente, -1 en caso de error
 * 
 * @details Esta función obtiene una clave PSK por su posición en el array:
 * - Valida que el índice esté dentro del rango válido
 * - Obtiene la clave del array de claves cargadas
 * - Copia la clave al buffer proporcionado
 * - Verifica que el buffer tenga espacio suficiente
 * 
 * @note El índice debe estar entre 0 y count-1
 * @see psk_validator_get_key_for_identity
 */
int psk_validator_get_key_by_index(int index, uint8_t* key_buffer, size_t buffer_size) {
    if (g_valid_keys.count == 0) {
        fprintf(stderr, "Error: No hay claves PSK válidas cargadas\n");
        return -1;
    }
    
    if (index < 0 || index >= g_valid_keys.count) {
        fprintf(stderr, "Error: Índice de clave PSK fuera de rango: %d\n", index);
        return -1;
    }
    
    const char* selected_key = g_valid_keys.keys[index];
    size_t key_len = strlen(selected_key);
    
    if (key_len >= buffer_size) {
        fprintf(stderr, "Error: Buffer insuficiente para la clave PSK\n");
        return -1;
    }
    
    memcpy(key_buffer, selected_key, key_len);
    key_buffer[key_len] = '\0'; // Asegurar terminación null
    
    return 0;
}

/**
 * @brief Libera los recursos del validador de claves PSK
 * 
 * @details Esta función limpia todos los recursos asociados al validador:
 * - Libera cada clave individual del array
 * - Libera el array de punteros a claves
 * - Resetea los contadores y punteros
 * - Prepara el validador para una nueva inicialización
 * 
 * @note Debe ser llamada al finalizar para evitar memory leaks
 * @note Es seguro llamar esta función múltiples veces
 * @see psk_validator_init
 */
void psk_validator_cleanup(void) {
    if (g_valid_keys.keys) {
        for (int i = 0; i < g_valid_keys.count; i++) {
            free(g_valid_keys.keys[i]);
        }
        free(g_valid_keys.keys);
        g_valid_keys.keys = NULL;
        g_valid_keys.count = 0;
        g_valid_keys.capacity = 0;
    }
}

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
int psk_validator_get_key_count(void) {
    return g_valid_keys.count;
}

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
int psk_validator_is_initialized(void) {
    return (g_valid_keys.keys != NULL && g_valid_keys.count > 0) ? 1 : 0;
} 