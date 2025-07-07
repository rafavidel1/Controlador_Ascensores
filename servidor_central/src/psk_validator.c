/**
 * @file psk_validator.c
 * @brief Implementación del validador de claves PSK
 * @author Sistema de Control de Ascensores
 * @date 2025
 * @version 1.0
 */

#include "psk_validator.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

// Estructura para almacenar las claves PSK válidas
typedef struct {
    char** keys;
    int count;
    int capacity;
} psk_valid_keys_t;

static psk_valid_keys_t g_valid_keys = {NULL, 0, 0};

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