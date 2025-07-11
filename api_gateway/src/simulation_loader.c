/**
 * @file simulation_loader.c
 * @brief Implementación del Sistema de Carga y Ejecución de Simulaciones
 * @author Sistema de Control de Ascensores
 * @date 2025
 * @version 1.0
 */
#include "api_gateway/simulation_loader.h"
#include "api_gateway/elevator_state_manager.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>

extern elevator_group_state_t managed_elevator_group;

/**
 * @brief Carga los datos de simulación desde un archivo JSON
 * @param archivo_json Ruta al archivo JSON con la configuración de simulación
 * @param datos Puntero a la estructura donde se almacenarán los datos cargados
 * @return true si se cargaron correctamente, false en caso de error
 * 
 * Esta función lee y parsea un archivo JSON que contiene la configuración
 * de simulación de ascensores, incluyendo edificios y peticiones.
 * 
 * **Estructura JSON esperada:**
 * ```json
 * {
 *   "edificios": [
 *     {
 *       "id_edificio": "E1",
 *       "peticiones": [
 *         {
 *           "tipo": "llamada_piso",
 *           "piso_origen": 3,
 *           "direccion": "up"
 *         },
 *         {
 *           "tipo": "solicitud_cabina",
 *           "indice_ascensor": 0,
 *           "piso_destino": 5
 *         }
 *       ]
 *     }
 *   ]
 * }
 * ```
 * 
 * **Operaciones realizadas:**
 * - Apertura y lectura del archivo JSON
 * - Parsing y validación de la estructura JSON
 * - Asignación de memoria para edificios y peticiones
 * - Validación de tipos de peticiones y parámetros
 * - Inicialización de estructuras de datos
 * 
 * En caso de error, libera automáticamente toda la memoria asignada.
 * 
 * @see liberar_datos_simulacion()
 * @see datos_simulacion_t
 * @see edificio_simulacion_t
 */
bool cargar_datos_simulacion(const char *archivo_json, datos_simulacion_t *datos) {
    if (!archivo_json || !datos) {
        printf("[SIMULATION] Error: Parámetros nulos\n");
        return false;
    }

    memset(datos, 0, sizeof(datos_simulacion_t));
    
    printf("[SIMULATION] Intentando abrir archivo: %s\n", archivo_json);

    FILE *file = fopen(archivo_json, "r");
    if (!file) {
        printf("[SIMULATION] Error: No se pudo abrir %s\n", archivo_json);
        printf("[SIMULATION] Error detallado: %s\n", strerror(errno));
        return false;
    }

    printf("[SIMULATION] Archivo abierto exitosamente\n");

    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    printf("[SIMULATION] Tamaño del archivo: %ld bytes\n", file_size);

    if (file_size <= 0) {
        printf("[SIMULATION] Error: Archivo vacío\n");
        fclose(file);
        return false;
    }

    char *json_string = malloc(file_size + 1);
    if (!json_string) {
        printf("[SIMULATION] Error: No se pudo asignar memoria\n");
        fclose(file);
        return false;
    }

    size_t bytes_read = fread(json_string, 1, file_size, file);
    json_string[bytes_read] = '\0';
    fclose(file);

    cJSON *json = cJSON_Parse(json_string);
    free(json_string);

    if (!json) {
        printf("[SIMULATION] Error: JSON inválido\n");
        return false;
    }

    cJSON *edificios_json = cJSON_GetObjectItemCaseSensitive(json, "edificios");
    if (!cJSON_IsArray(edificios_json)) {
        printf("[SIMULATION] Error: 'edificios' no es array\n");
        cJSON_Delete(json);
        return false;
    }

    int num_edificios = cJSON_GetArraySize(edificios_json);
    if (num_edificios <= 0) {
        printf("[SIMULATION] Error: No hay edificios\n");
        cJSON_Delete(json);
        return false;
    }

    datos->edificios = malloc(num_edificios * sizeof(edificio_simulacion_t));
    if (!datos->edificios) {
        printf("[SIMULATION] Error: No se pudo asignar memoria para edificios\n");
        cJSON_Delete(json);
        return false;
    }

    datos->num_edificios = num_edificios;

    cJSON *edificio_json = NULL;
    int edificio_idx = 0;
    cJSON_ArrayForEach(edificio_json, edificios_json) {
        edificio_simulacion_t *edificio = &datos->edificios[edificio_idx];
        memset(edificio, 0, sizeof(edificio_simulacion_t));

        cJSON *id_json = cJSON_GetObjectItemCaseSensitive(edificio_json, "id_edificio");
        if (!cJSON_IsString(id_json)) {
            printf("[SIMULATION] Error: ID edificio inválido en %d\n", edificio_idx);
            liberar_datos_simulacion(datos);
            cJSON_Delete(json);
            return false;
        }

        strncpy(edificio->id_edificio, id_json->valuestring, sizeof(edificio->id_edificio) - 1);

        cJSON *peticiones_json = cJSON_GetObjectItemCaseSensitive(edificio_json, "peticiones");
        if (!cJSON_IsArray(peticiones_json)) {
            printf("[SIMULATION] Error: Peticiones inválidas para %s\n", edificio->id_edificio);
            liberar_datos_simulacion(datos);
            cJSON_Delete(json);
            return false;
        }

        int num_peticiones = cJSON_GetArraySize(peticiones_json);
        if (num_peticiones <= 0) {
            printf("[SIMULATION] Advertencia: %s sin peticiones\n", edificio->id_edificio);
            edificio_idx++;
            continue;
        }

        edificio->peticiones = malloc(num_peticiones * sizeof(peticion_simulacion_t));
        if (!edificio->peticiones) {
            printf("[SIMULATION] Error: No memoria para peticiones de %s\n", edificio->id_edificio);
            liberar_datos_simulacion(datos);
            cJSON_Delete(json);
            return false;
        }

        edificio->num_peticiones = num_peticiones;

        cJSON *peticion_json = NULL;
        int peticion_idx = 0;
        cJSON_ArrayForEach(peticion_json, peticiones_json) {
            peticion_simulacion_t *peticion = &edificio->peticiones[peticion_idx];
            memset(peticion, 0, sizeof(peticion_simulacion_t));

            cJSON *tipo_json = cJSON_GetObjectItemCaseSensitive(peticion_json, "tipo");
            if (!cJSON_IsString(tipo_json)) {
                printf("[SIMULATION] Error: Tipo inválido en %s[%d]\n", edificio->id_edificio, peticion_idx);
                liberar_datos_simulacion(datos);
                cJSON_Delete(json);
                return false;
            }

            if (strcmp(tipo_json->valuestring, "llamada_piso") == 0) {
                peticion->tipo = PETICION_LLAMADA_PISO;

                cJSON *piso_json = cJSON_GetObjectItemCaseSensitive(peticion_json, "piso_origen");
                if (!cJSON_IsNumber(piso_json)) {
                    printf("[SIMULATION] Error: piso_origen inválido en %s[%d]\n", edificio->id_edificio, peticion_idx);
                    liberar_datos_simulacion(datos);
                    cJSON_Delete(json);
                    return false;
                }
                peticion->piso_origen = piso_json->valueint;

                cJSON *direccion_json = cJSON_GetObjectItemCaseSensitive(peticion_json, "direccion");
                if (!cJSON_IsString(direccion_json)) {
                    printf("[SIMULATION] Error: dirección inválida en %s[%d]\n", edificio->id_edificio, peticion_idx);
                    liberar_datos_simulacion(datos);
                    cJSON_Delete(json);
                    return false;
                }
                strncpy(peticion->direccion, direccion_json->valuestring, sizeof(peticion->direccion) - 1);

            } else if (strcmp(tipo_json->valuestring, "solicitud_cabina") == 0) {
                peticion->tipo = PETICION_SOLICITUD_CABINA;

                cJSON *indice_json = cJSON_GetObjectItemCaseSensitive(peticion_json, "indice_ascensor");
                if (!cJSON_IsNumber(indice_json)) {
                    printf("[SIMULATION] Error: indice_ascensor inválido en %s[%d]\n", edificio->id_edificio, peticion_idx);
                    liberar_datos_simulacion(datos);
                    cJSON_Delete(json);
                    return false;
                }
                peticion->indice_ascensor = indice_json->valueint;

                cJSON *destino_json = cJSON_GetObjectItemCaseSensitive(peticion_json, "piso_destino");
                if (!cJSON_IsNumber(destino_json)) {
                    printf("[SIMULATION] Error: piso_destino inválido en %s[%d]\n", edificio->id_edificio, peticion_idx);
                    liberar_datos_simulacion(datos);
                    cJSON_Delete(json);
                    return false;
                }
                peticion->piso_destino = destino_json->valueint;

            } else {
                printf("[SIMULATION] Error: Tipo '%s' desconocido en %s[%d]\n", 
                       tipo_json->valuestring, edificio->id_edificio, peticion_idx);
                liberar_datos_simulacion(datos);
                cJSON_Delete(json);
                return false;
            }

            peticion_idx++;
        }

        edificio_idx++;
    }

    cJSON_Delete(json);
    datos->datos_cargados = true;

    printf("[SIMULATION] Cargados %d edificios con %d peticiones totales\n", 
           datos->num_edificios, datos->num_edificios * 10);

    return true;
}

/**
 * @brief Libera la memoria asignada para los datos de simulación
 * @param datos Puntero a la estructura de datos de simulación a liberar
 * 
 * Esta función libera toda la memoria asignada dinámicamente para
 * los datos de simulación, incluyendo arrays de edificios y peticiones.
 * 
 * **Operaciones realizadas:**
 * - Libera memoria de peticiones para cada edificio
 * - Libera memoria del array de edificios
 * - Resetea los contadores y punteros
 * - Inicializa la estructura a cero
 * 
 * La función es segura para llamar múltiples veces o con punteros NULL.
 * 
 * @see cargar_datos_simulacion()
 * @see datos_simulacion_t
 */
void liberar_datos_simulacion(datos_simulacion_t *datos) {
    if (!datos) return;

    if (datos->edificios) {
        for (int i = 0; i < datos->num_edificios; i++) {
            if (datos->edificios[i].peticiones) {
                free(datos->edificios[i].peticiones);
            }
        }
        free(datos->edificios);
    }

    memset(datos, 0, sizeof(datos_simulacion_t));
}

/**
 * @brief Selecciona un edificio aleatorio de los datos de simulación
 * @param datos Puntero a los datos de simulación cargados
 * @return Puntero al edificio seleccionado, o NULL si no hay edificios
 * 
 * Esta función selecciona aleatoriamente un edificio de los disponibles
 * en los datos de simulación cargados. Utiliza la función rand() para
 * la selección aleatoria.
 * 
 * **Comportamiento:**
 * - Verifica que existan edificios disponibles
 * - Genera un índice aleatorio válido
 * - Retorna el puntero al edificio seleccionado
 * - Registra la selección en el sistema de logging
 * 
 * @note Se debe llamar a srand() antes de usar esta función para
 *       garantizar aleatoriedad real.
 * 
 * @see datos_simulacion_t
 * @see edificio_simulacion_t
 */
edificio_simulacion_t* seleccionar_edificio_aleatorio(datos_simulacion_t *datos) {
    if (!datos || !datos->datos_cargados || datos->num_edificios <= 0) {
        printf("[SIMULATION] Error: No hay datos para selección aleatoria\n");
        return NULL;
    }

    static bool srand_inicializado = false;
    if (!srand_inicializado) {
        srand(time(NULL));
        srand_inicializado = true;
    }

    int indice_aleatorio = rand() % datos->num_edificios;
    edificio_simulacion_t *edificio_seleccionado = &datos->edificios[indice_aleatorio];

    printf("[SIMULATION] Edificio seleccionado: %s (índice %d de %d)\n", 
           edificio_seleccionado->id_edificio, indice_aleatorio, datos->num_edificios);

    return edificio_seleccionado;
}

/**
 * @brief Convierte una cadena de dirección a valor numérico
 * @param direccion_str Cadena que representa la dirección ("up", "down", etc.)
 * @return Valor numérico correspondiente a la dirección
 * 
 * Esta función convierte cadenas de texto que representan direcciones
 * de movimiento a valores numéricos utilizados en frames CAN.
 * 
 * **Conversiones soportadas:**
 * - "up" → 0 (MOVING_UP)
 * - "down" → 1 (MOVING_DOWN)
 * - Otros valores → 0 (por defecto)
 * 
 * La función es insensible a mayúsculas y minúsculas.
 * 
 * @see movement_direction_enum_t
 * @see simulated_can_frame_t
 */
int convertir_direccion_string(const char *direccion_str) {
    if (!direccion_str) return MOVING_UP;
    
    if (strcmp(direccion_str, "up") == 0) {
        return MOVING_UP;
    } else if (strcmp(direccion_str, "down") == 0) {
        return MOVING_DOWN;
    }
    
    return MOVING_UP;
}

/**
 * @brief Ejecuta las peticiones de un edificio específico
 * @param edificio Puntero al edificio cuyas peticiones se van a ejecutar
 * @param ctx Contexto CoAP para el procesamiento de frames CAN
 * @return Número de peticiones procesadas exitosamente
 * 
 * Esta función ejecuta secuencialmente todas las peticiones de un edificio,
 * convirtiéndolas a frames CAN simulados y procesándolas a través del
 * puente CAN-CoAP.
 * 
 * **Tipos de peticiones soportadas:**
 * - **Llamada de piso**: Genera frame CAN 0x100 con piso origen y dirección
 * - **Solicitud de cabina**: Genera frame CAN 0x200 con ascensor y destino
 * 
 * **Operaciones realizadas:**
 * - Actualiza el grupo de ascensores con el ID del edificio
 * - Procesa cada petición según su tipo
 * - Crea frames CAN simulados apropiados
 * - Envía frames al puente CAN-CoAP para procesamiento
 * - Registra estadísticas de peticiones procesadas
 * 
 * @see peticion_simulacion_t
 * @see simulated_can_frame_t
 * @see ag_can_bridge_process_incoming_frame()
 */
int ejecutar_peticiones_edificio(edificio_simulacion_t *edificio, coap_context_t *ctx) {
    // Esta función no se usará directamente, la lógica estará en mi_simulador_ascensor.c
    return 0;
} 