/**
 * @file psk_validator.c
 * @brief Implementación del validador de credenciales para autenticación DTLS
 * @author Sistema de Control de Ascensores
 * @version 2.0
 * @date 2025
 * 
 * @details Este archivo implementa el sistema de validación de credenciales
 * utilizado en la autenticación DTLS del servidor central. El sistema
 * carga credenciales desde un archivo de configuración y proporciona funciones para
 * validar y obtener credenciales de forma determinística basada en la identidad
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
 * @brief Estructura para almacenar las credenciales válidas
 * 
 * @details Esta estructura tiene todas las credenciales válidas
 * cargadas desde el archivo de configuración. Utiliza un array dinámico para
 * almacenar las credenciales como strings.
 */
typedef struct {
    char** keys;        /**< Array de punteros a credenciales */
    int count;          /**< Número actual de credenciales cargadas */
    int capacity;       /**< Capacidad total del array */
} psk_valid_keys_t;

/**
 * @brief Variable global que almacena las credenciales válidas
 * 
 * @details Esta variable global mantiene el estado del validador de credenciales.
 * Contiene todas las credenciales cargadas desde el archivo de configuración.
 */
static psk_valid_keys_t g_valid_keys = {NULL, 0, 0};

/**
 * @brief Inicializa el validador de credenciales
 * 
 * @param[in] keys_file_path Ruta al archivo de configuración de autenticación
 * 
 * @return 0 si se inicializó correctamente, -1 en caso de error
 * 
 * @details Esta función inicializa el sistema de validación de credenciales:
 * - Abre el archivo de configuración especificado
 * - Cuenta el número de líneas en el archivo
 * - Asigna memoria dinámica para almacenar las credenciales
 * - Lee todas las credenciales del archivo y las almacena en memoria
 * - Cierra el archivo después de la carga
 * 
 * @note El archivo debe contener una credencial por línea
 * @note La función maneja automáticamente la memoria dinámica
 * @see psk_validator_cleanup
 */
int psk_validator_init(const char* keys_file_path) {
    FILE* file = fopen(keys_file_path, "r");
    if (!file) {
        fprintf(stderr, "Error: No se pudo abrir el archivo de configuración de autenticación: %s\n", keys_file_path);
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
        fprintf(stderr, "Error: No se pudo asignar memoria para las credenciales válidas\n");
        fclose(file);
        return -1;
    }
    
    // Volver al inicio del archivo
    rewind(file);
    
    // Leer credenciales
    int index = 0;
    while (fgets(buffer, sizeof(buffer), file) && index < line_count) {
        // Remover salto de línea
        size_t len = strlen(buffer);
        if (len > 0 && buffer[len-1] == '\n') {
            buffer[len-1] = '\0';
        }
        
        // Asignar memoria para la credencial
        g_valid_keys.keys[index] = strdup(buffer);
        if (!g_valid_keys.keys[index]) {
            fprintf(stderr, "Error: No se pudo asignar memoria para la credencial válida %d\n", index);
            fclose(file);
            return -1;
        }
        index++;
    }
    
    g_valid_keys.count = index;
    fclose(file);
    
    printf("Validador de credenciales: Cargadas %d credenciales válidas desde %s\n", g_valid_keys.count, keys_file_path);
    return 0;
}

/**
 * @brief Valida si unas credenciales están en la lista de credenciales válidas
 * 
 * @param[in] key Credenciales a validar
 * @param[in] key_len Longitud de las credenciales en bytes
 * 
 * @return 1 si las credenciales son válidas, 0 si no lo son
 * 
 * @details Esta función valida unas credenciales contra la lista de credenciales válidas:
 * - Verifica que haya credenciales cargadas en memoria
 * - Compara las credenciales proporcionadas con cada credencial almacenada
 * - Utiliza comparación exacta de longitud y contenido
 * - Retorna el resultado de la validación
 * 
 * @note La búsqueda es lineal O(n) donde n es el número de credenciales
 * @note La función es thread-safe para lecturas concurrentes
 * @see psk_validator_get_key_for_identity
 */
int psk_validator_check_key(const char* key, size_t key_len) {
    if (g_valid_keys.count == 0) {
        fprintf(stderr, "Error: No hay credenciales válidas cargadas\n");
        return 0;
    }
    
    // Buscar las credenciales en la lista de credenciales válidas
    for (int i = 0; i < g_valid_keys.count; i++) {
        if (strlen(g_valid_keys.keys[i]) == key_len && 
            strncmp(g_valid_keys.keys[i], key, key_len) == 0) {
            return 1; // Credenciales válidas encontradas
        }
    }
    
    return 0; // Credenciales no encontradas
}

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
 * - Calcula un hash determinístico de la identidad
 * - Usa el hash como índice para seleccionar unas credenciales
 * - Copia las credenciales seleccionadas al buffer proporcionado
 * - Verifica que el buffer tenga espacio suficiente
 * 
 * @note La selección es determinística: misma identidad = mismas credenciales
 * @note El algoritmo usa hash simple para distribución uniforme
 * @see psk_validator_check_key
 */
int psk_validator_get_key_for_identity(const char* identity, uint8_t* key_buffer, size_t buffer_size) {
    if (g_valid_keys.count == 0) {
        fprintf(stderr, "Error: No hay credenciales válidas cargadas\n");
        return -1;
    }
    
    // Usar la identidad como seed para selección determinística
    unsigned int seed = 0;
    for (size_t i = 0; i < strlen(identity); i++) {
        seed = seed * 31 + identity[i];
    }
    
    // Seleccionar credenciales basadas en el hash de la identidad
    int selected_index = seed % g_valid_keys.count;
    const char* selected_key = g_valid_keys.keys[selected_index];
    
    // Copiar credenciales al buffer
    size_t key_len = strlen(selected_key);
    if (key_len >= buffer_size) {
        fprintf(stderr, "Error: Buffer insuficiente para las credenciales\n");
        return -1;
    }
    
    memcpy(key_buffer, selected_key, key_len);
    key_buffer[key_len] = '\0'; // Asegurar terminación null
    
    return 0;
}

/**
 * @brief Obtiene credenciales por índice específico
 * 
 * @param[in] index Índice de las credenciales en el archivo (0-based)
 * @param[out] key_buffer Buffer donde se almacenarán las credenciales
 * @param[in] buffer_size Tamaño del buffer en bytes
 * 
 * @return 0 si se obtuvo correctamente, -1 en caso de error
 * 
 * @details Esta función obtiene credenciales por su posición en el array:
 * - Valida que el índice esté dentro del rango válido
 * - Obtiene las credenciales del array de credenciales cargadas
 * - Copia las credenciales al buffer proporcionado
 * - Verifica que el buffer tenga espacio suficiente
 * 
 * @note El índice debe estar entre 0 y count-1
 * @see psk_validator_get_key_for_identity
 */
int psk_validator_get_key_by_index(int index, uint8_t* key_buffer, size_t buffer_size) {
    if (g_valid_keys.count == 0) {
        fprintf(stderr, "Error: No hay credenciales válidas cargadas\n");
        return -1;
    }
    
    if (index < 0 || index >= g_valid_keys.count) {
        fprintf(stderr, "Error: Índice de credenciales fuera de rango: %d\n", index);
        return -1;
    }
    
    const char* selected_key = g_valid_keys.keys[index];
    size_t key_len = strlen(selected_key);
    
    if (key_len >= buffer_size) {
        fprintf(stderr, "Error: Buffer insuficiente para las credenciales\n");
        return -1;
    }
    
    memcpy(key_buffer, selected_key, key_len);
    key_buffer[key_len] = '\0'; // Asegurar terminación null
    
    return 0;
}

/**
 * @brief Libera los recursos del validador de credenciales
 * 
 * @details Esta función limpia todos los recursos asociados al validador:
 * - Libera cada credencial individual del array
 * - Libera el array de punteros a credenciales
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
 * @brief Obtiene el número total de credenciales disponibles
 * 
 * @return Número de credenciales en el archivo
 * 
 * @details Esta función retorna el número total de credenciales que han sido
 * cargadas desde el archivo de configuración.
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
 * @details Esta función verifica si el validador de credenciales ha sido
 * inicializado correctamente y está listo para su uso.
 * 
 * @note Útil para verificar el estado antes de usar otras funciones
 */
int psk_validator_is_initialized(void) {
    return (g_valid_keys.keys != NULL && g_valid_keys.count > 0) ? 1 : 0;
} 