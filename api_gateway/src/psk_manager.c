/**
 * @file psk_manager.c
 * @brief Implementación del gestor de claves PSK
 * @author Sistema de Control de Ascensores
 * @date 2025
 * @version 1.0
 */

#include "psk_manager.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/**
 * @brief Estructura para almacenar las claves PSK
 * 
 * Esta estructura gestiona dinámicamente un array de claves PSK
 * cargadas desde un archivo de configuración.
 */
typedef struct {
    char** keys;      ///< Array dinámico de strings con las claves PSK
    int count;        ///< Número actual de claves cargadas
    int capacity;     ///< Capacidad máxima del array (para futuras expansiones)
} psk_keys_t;

static psk_keys_t g_psk_keys = {NULL, 0, 0};

int psk_manager_init(const char* keys_file_path) {
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
    g_psk_keys.capacity = line_count;
    g_psk_keys.keys = malloc(line_count * sizeof(char*));
    if (!g_psk_keys.keys) {
        fprintf(stderr, "Error: No se pudo asignar memoria para las claves PSK\n");
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
        g_psk_keys.keys[index] = strdup(buffer);
        if (!g_psk_keys.keys[index]) {
            fprintf(stderr, "Error: No se pudo asignar memoria para la clave %d\n", index);
            fclose(file);
            return -1;
        }
        index++;
    }
    
    g_psk_keys.count = index;
    fclose(file);
    
    // Inicializar generador de números aleatorios
    srand(time(NULL));
    
    printf("PSK Manager: Cargadas %d claves desde %s\n", g_psk_keys.count, keys_file_path);
    return 0;
}

int psk_manager_get_random_key(char* key_buffer, size_t buffer_size) {
    if (g_psk_keys.count == 0) {
        fprintf(stderr, "Error: No hay claves PSK cargadas\n");
        return -1;
    }
    
    // Intentar hasta 5 veces obtener una clave válida
    int max_attempts = 5;
    for (int attempt = 0; attempt < max_attempts; attempt++) {
        // Seleccionar clave aleatoria
        int random_index = rand() % g_psk_keys.count;
        const char* selected_key = g_psk_keys.keys[random_index];
        
        // Verificar que la clave no esté vacía
        if (selected_key && strlen(selected_key) > 0) {
            // Copiar clave al buffer
            size_t key_len = strlen(selected_key);
            if (key_len >= buffer_size) {
                fprintf(stderr, "Error: Buffer insuficiente para la clave PSK\n");
                return -1;
            }
            
            strcpy(key_buffer, selected_key);
            return 0;
        }
        
        // Si la clave está vacía, intentar con la siguiente
        fprintf(stderr, "Warning: Clave PSK vacía en índice %d, reintentando...\n", random_index);
    }
    
    fprintf(stderr, "Error: No se pudo obtener una clave PSK válida después de %d intentos\n", max_attempts);
    return -1;
}

int psk_manager_get_first_key(char* key_buffer, size_t buffer_size) {
    if (g_psk_keys.count == 0) {
        fprintf(stderr, "Error: No hay claves PSK cargadas\n");
        return -1;
    }
    
    // Obtener la primera clave
    const char* first_key = g_psk_keys.keys[0];
    
    // Copiar clave al buffer
    size_t key_len = strlen(first_key);
    if (key_len >= buffer_size) {
        fprintf(stderr, "Error: Buffer insuficiente para la clave PSK\n");
        return -1;
    }
    
    strcpy(key_buffer, first_key);
    return 0;
}

int psk_manager_get_deterministic_key(const char* identity, char* key_buffer, size_t buffer_size) {
    if (g_psk_keys.count == 0) {
        fprintf(stderr, "Error: No hay claves PSK cargadas\n");
        return -1;
    }
    
    // Usar la identidad como seed para selección determinística
    unsigned int seed = 0;
    for (size_t i = 0; i < strlen(identity); i++) {
        seed = seed * 31 + identity[i];
    }
    
    // Seleccionar clave basada en el hash de la identidad
    int selected_index = seed % g_psk_keys.count;
    const char* selected_key = g_psk_keys.keys[selected_index];
    
    // Copiar clave al buffer
    size_t key_len = strlen(selected_key);
    if (key_len >= buffer_size) {
        fprintf(stderr, "Error: Buffer insuficiente para la clave PSK\n");
        return -1;
    }
    
    strcpy(key_buffer, selected_key);
    return 0;
}

void psk_manager_cleanup(void) {
    if (g_psk_keys.keys) {
        for (int i = 0; i < g_psk_keys.count; i++) {
            free(g_psk_keys.keys[i]);
        }
        free(g_psk_keys.keys);
        g_psk_keys.keys = NULL;
        g_psk_keys.count = 0;
        g_psk_keys.capacity = 0;
    }
} 