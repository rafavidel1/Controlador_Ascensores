/**
 * @file main.c
 * @brief Servidor Central del Sistema de Control de Ascensores
 * @author Sistema de Control de Ascensores
 * @date 2025
 * @version 2.0
 * 
 * Este archivo implementa el servidor central que gestiona la asignación
 * de tareas a ascensores en el sistema distribuido de control. Sus funciones principales:
 * 
 * - **Servidor CoAP DTLS**: Configuración de servidor CoAP con seguridad DTLS-PSK
 * - **Gestión de solicitudes**: Procesamiento de peticiones de piso y cabina
 * - **Algoritmos de asignación**: Lógica para asignar ascensores a tareas
 * - **Generación de IDs únicos**: Creación de identificadores únicos para tareas
 * - **Respuestas JSON**: Envío de respuestas estructuradas a los gateways
 * - **Logging**: Sistema de registro detallado para monitoreo y debugging
 * 
 * **Endpoints CoAP soportados:**
 * - `/peticion_piso`: Solicitudes de llamada de piso desde botones externos
 * - `/peticion_cabina`: Solicitudes de cabina desde interior de ascensores
 * 
 * **Algoritmo de asignación:**
 * Utiliza el algoritmo de proximidad inteligente implementado en select_optimal_elevator():
 * - Filtra ascensores disponibles (disponible=true)
 * - Calcula distancia absoluta desde piso_origen a cada ascensor
 * - Selecciona todos los ascensores con distancia mínima
 * - En caso de empate, selecciona aleatoriamente para distribuir carga
 * - Garantiza asignación óptima basada en proximidad
 * 
 * **Seguridad:**
 * Utiliza DTLS-PSK (Pre-Shared Key) para autenticación y cifrado de
 * comunicaciones con los API Gateways.
 * 
 * @see servidor_central/logging.h
 * @see servidor_central/dtls_common_config.h
 * @see coap3/coap.h
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h> // Para unlink, aunque ya no lo usaremos para DB
#include <sys/time.h> // Para gettimeofday (generar IDs únicos)
#include <arpa/inet.h> // Para inet_pton
#include <errno.h> // <--- AÑADIDO PARA errno
#include <limits.h> // Para INT_MAX
#include <time.h> // Para time() y srand()
#include <math.h> // Para abs() (aunque también está en stdlib.h)

#include <coap3/coap.h>
// #include <sqlite3.h> // REMOVED
#include <cJSON.h> // Keep for parsing new payloads. Corrected include path.

// #include "servidor_central/database_manager.h" // REMOVED
// #include "servidor_central/coap_server.h" // REMOVED (if not used otherwise)
// #include "servidor_central/config.h" // REMOVED as it may not exist or be needed
#include "servidor_central/logging.h" 
#include "servidor_central/dtls_common_config.h" // <--- AÑADIDO para PSK_SERVER_HINT, PSK_CLIENT_IDENTITY, PSK_KEY

/**
 * @brief Ruta del recurso CoAP para peticiones de piso
 * 
 * Define la ruta del endpoint CoAP que maneja las solicitudes de
 * llamada de piso (floor calls) desde botones externos.
 */
#define RESOURCE_FLOOR_CALL "peticion_piso"

/**
 * @brief Ruta del recurso CoAP para peticiones de cabina
 * 
 * Define la ruta del endpoint CoAP que maneja las solicitudes de
 * cabina (cabin requests) desde el interior de los ascensores.
 */
#define RESOURCE_CABIN_REQUEST "peticion_cabina"

/**
 * @brief Dirección IP de escucha del servidor
 * 
 * Dirección IP en la que el servidor CoAP escuchará conexiones.
 * "0.0.0.0" indica que escuchará en todas las interfaces disponibles.
 */
#define SERVER_IP "0.0.0.0"

/**
 * @brief Puerto de escucha del servidor CoAP DTLS
 * 
 * Puerto en el que el servidor CoAP con DTLS escuchará conexiones.
 * El puerto 5684 es el puerto estándar para CoAP sobre DTLS.
 */
#define SERVER_PORT "5684"

/**
 * @brief Bandera global para controlar el bucle principal del servidor
 * 
 * Esta variable se establece a 0 por el manejador de señal SIGINT
 * para indicar que el servidor debe terminar de manera elegante.
 * 
 * @see handle_sigint()
 */
static int running = 1;

// sqlite3 *db; // REMOVED

/**
 * @brief Manejador de señal para SIGINT (Ctrl+C)
 * @param signum Número de señal recibida (se espera SIGINT)
 * 
 * Establece la bandera running a 0 para señalar al bucle principal
 * del servidor que debe terminar, permitiendo una terminación elegante.
 * 
 * Esta función es registrada como manejador de SIGINT en main() y
 * proporciona una forma limpia de cerrar el servidor.
 */
void handle_sigint(int signum) {
    SRV_LOG_WARN("Received SIGINT, shutting down...");
    running = 0;
}

/**
 * @brief Genera un ID único para una tarea de ascensor
 * @param task_id_out Buffer donde se almacenará el ID generado
 * @param len Tamaño del buffer de salida
 * 
 * Esta función genera un identificador único para tareas de ascensor
 * basado en el timestamp actual del sistema. El formato del ID es:
 * "T_{segundos_unix}{milisegundos}"
 * 
 * Ejemplo: "T_1640995200123" donde:
 * - T_: Prefijo identificador de tarea
 * - 1640995200: Segundos desde epoch Unix
 * - 123: Milisegundos (3 dígitos)
 * 
 * Esta implementación garantiza unicidad temporal pero no es
 * thread-safe. Para entornos multihilo se recomienda añadir
 * sincronización o usar contadores atómicos adicionales.
 * 
 * @note El buffer de salida debe tener al menos 32 caracteres
 * @see gettimeofday()
 */
void generate_unique_task_id(char *task_id_out, size_t len) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    snprintf(task_id_out, len, "T_%ld%03ld", (long)tv.tv_sec, (long)(tv.tv_usec / 1000));
}

/**
 * @brief Encuentra el ascensor más cercano para una llamada de piso
 * @param elevadores_estado Array JSON con el estado de todos los ascensores
 * @param piso_origen Piso desde donde se realiza la llamada
 * @param direccion_llamada Dirección solicitada ("up" o "down")
 * @return ID del ascensor asignado (debe liberarse con free()) o NULL si no hay ascensores disponibles
 * 
 * Esta función implementa un algoritmo de asignación inteligente que selecciona
 * el ascensor más cercano al piso de origen de la llamada. En caso de empate
 * en distancia, selecciona aleatoriamente entre los candidatos empatados.
 * 
 * **Algoritmo de selección:**
 * 1. Filtra ascensores disponibles (disponible == true)
 * 2. Calcula distancia absoluta entre piso_actual y piso_origen
 * 3. Encuentra la distancia mínima
 * 4. Si hay múltiples ascensores con distancia mínima, selecciona aleatoriamente
 * 
 * **Criterios de disponibilidad:**
 * - Campo "disponible" debe ser true
 * - Campo "id_ascensor" debe ser string válido
 * - Campo "piso_actual" debe ser número válido
 * 
 * **Ejemplo:**
 * - Ascensores en pisos: [3, 6, 8, 10]
 * - Llamada desde piso 0
 * - Distancias: [3, 6, 8, 10]
 * - Selecciona ascensor en piso 3 (distancia mínima = 3)
 * 
 * @see hnd_floor_call()
 * @see cJSON_IsArray()
 */
static char* select_optimal_elevator(cJSON *elevadores_estado, int piso_origen, const char *direccion_llamada) {
    if (!cJSON_IsArray(elevadores_estado)) {
        SRV_LOG_ERROR("elevadores_estado no es un array válido");
        return NULL;
    }

    int array_size = cJSON_GetArraySize(elevadores_estado);
    if (array_size == 0) {
        SRV_LOG_WARN("Array elevadores_estado está vacío");
        return NULL;
    }

    // Arrays para almacenar candidatos con distancia mínima
    char **closest_elevators = malloc(array_size * sizeof(char*));
    int *closest_floors = malloc(array_size * sizeof(int));
    int closest_count = 0;
    int min_distance = INT_MAX;

    SRV_LOG_DEBUG("Analizando %d ascensores para llamada desde piso %d", array_size, piso_origen);

    // Primera pasada: encontrar la distancia mínima
    for (int i = 0; i < array_size; i++) {
        cJSON *elevator = cJSON_GetArrayItem(elevadores_estado, i);
        if (!elevator) continue;

        cJSON *j_id_ascensor = cJSON_GetObjectItemCaseSensitive(elevator, "id_ascensor");
        cJSON *j_piso_actual = cJSON_GetObjectItemCaseSensitive(elevator, "piso_actual");
        cJSON *j_disponible = cJSON_GetObjectItemCaseSensitive(elevator, "disponible");

        // Validar campos obligatorios
        if (!cJSON_IsString(j_id_ascensor) || !cJSON_IsNumber(j_piso_actual) || !cJSON_IsBool(j_disponible)) {
            SRV_LOG_WARN("Ascensor %d: campos inválidos o faltantes", i);
            continue;
        }

        // Solo considerar ascensores disponibles
        if (!cJSON_IsTrue(j_disponible)) {
            SRV_LOG_DEBUG("Ascensor %s no disponible (disponible=false)", j_id_ascensor->valuestring);
            continue;
        }

        int piso_actual = j_piso_actual->valueint;
        int distance = abs(piso_actual - piso_origen);

        SRV_LOG_DEBUG("Ascensor %s: piso_actual=%d, distancia=%d", 
                     j_id_ascensor->valuestring, piso_actual, distance);

        if (distance < min_distance) {
            min_distance = distance;
        }
    }

    if (min_distance == INT_MAX) {
        SRV_LOG_WARN("No se encontraron ascensores disponibles");
        free(closest_elevators);
        free(closest_floors);
        return NULL;
    }

    SRV_LOG_DEBUG("Distancia mínima encontrada: %d", min_distance);

    // Segunda pasada: recopilar todos los ascensores con distancia mínima
    for (int i = 0; i < array_size; i++) {
        cJSON *elevator = cJSON_GetArrayItem(elevadores_estado, i);
        if (!elevator) continue;

        cJSON *j_id_ascensor = cJSON_GetObjectItemCaseSensitive(elevator, "id_ascensor");
        cJSON *j_piso_actual = cJSON_GetObjectItemCaseSensitive(elevator, "piso_actual");
        cJSON *j_disponible = cJSON_GetObjectItemCaseSensitive(elevator, "disponible");

        if (!cJSON_IsString(j_id_ascensor) || !cJSON_IsNumber(j_piso_actual) || !cJSON_IsBool(j_disponible)) {
            continue;
        }

        if (!cJSON_IsTrue(j_disponible)) {
            continue;
        }

        int piso_actual = j_piso_actual->valueint;
        int distance = abs(piso_actual - piso_origen);

        if (distance == min_distance) {
            closest_elevators[closest_count] = strdup(j_id_ascensor->valuestring);
            closest_floors[closest_count] = piso_actual;
            closest_count++;
            SRV_LOG_DEBUG("Candidato %d: %s (piso %d, distancia %d)", 
                         closest_count, j_id_ascensor->valuestring, piso_actual, distance);
        }
    }

    char *selected_elevator = NULL;
    if (closest_count > 0) {
        if (closest_count == 1) {
            SRV_LOG_INFO("Ascensor más cercano: %s (piso %d, distancia %d)", 
                        closest_elevators[0], closest_floors[0], min_distance);
            selected_elevator = closest_elevators[0];
        } else {
            // Selección aleatoria entre candidatos empatados
            srand((unsigned int)time(NULL));
            int random_index = rand() % closest_count;
            SRV_LOG_INFO("Empate en distancia (%d). Seleccionando aleatoriamente: %s (piso %d) entre %d candidatos", 
                        min_distance, closest_elevators[random_index], closest_floors[random_index], closest_count);
            
            selected_elevator = closest_elevators[random_index];
            
            // Liberar los otros candidatos no seleccionados
            for (int i = 0; i < closest_count; i++) {
                if (i != random_index) {
                    free(closest_elevators[i]);
                }
            }
        }
    }

    free(closest_elevators);
    free(closest_floors);
    return selected_elevator;
}

// --- STUBBED CoAP Handlers (to be refactored) ---

/**
 * @brief Manejador CoAP para solicitudes de llamada de piso
 * @param resource Recurso CoAP que recibió la solicitud
 * @param session Sesión CoAP del cliente que envió la solicitud
 * @param request PDU de la solicitud CoAP recibida
 * @param query Parámetros de consulta de la URI (no utilizado)
 * @param response PDU de respuesta CoAP a enviar al cliente
 * 
 * Esta función procesa las solicitudes de llamada de piso (floor calls)
 * recibidas desde los API Gateways. Implementa el algoritmo de asignación
 * de ascensores para atender llamadas desde botones externos.
 * 
 * **Formato JSON esperado:**
 * ```json
 * {
 *   "id_edificio": "E1",
 *   "piso_origen_llamada": 5,
 *   "direccion_llamada": "SUBIENDO",
 *   "elevadores_estado": [
 *     {
 *       "id_ascensor": "E1A1",
 *       "piso_actual": 3,
 *       "estado_puerta": "CERRADA",
 *       "disponible": true,
 *       "tarea_actual_id": null,
 *       "destino_actual": null
 *     }
 *   ]
 * }
 * ```
 * 
 * **Respuesta JSON:**
 * ```json
 * {
 *   "tarea_id": "T_1640995200123",
 *   "ascensor_asignado_id": "E1A1"
 * }
 * ```
 * 
 * **Algoritmo de asignación:**
 * Utiliza el algoritmo de proximidad inteligente implementado en select_optimal_elevator():
 * - Filtra ascensores disponibles (disponible=true)
 * - Calcula distancia absoluta desde piso_origen a cada ascensor
 * - Selecciona todos los ascensores con distancia mínima
 * - En caso de empate, selecciona aleatoriamente para distribuir carga
 * - Garantiza asignación óptima basada en proximidad
 * 
 * @see select_optimal_elevator()
 * @see generate_unique_task_id()
 * @see RESOURCE_FLOOR_CALL
 */
static void hnd_floor_call(coap_resource_t *resource, coap_session_t *session,
                           const coap_pdu_t *request, const coap_string_t *query,
                           coap_pdu_t *response)
{
    const coap_str_const_t *uri_path_fc = coap_resource_get_uri_path(resource);
    if (uri_path_fc) {
        SRV_LOG_INFO("Received request on /%.*s (Peticion Piso)", (int)uri_path_fc->length, uri_path_fc->s);
    } else {
        SRV_LOG_INFO("Received request on /[unknown_path] (Peticion Piso)");
    }

    const uint8_t *data;
    size_t data_len;
    if (coap_get_data(request, &data_len, &data)) {
        SRV_LOG_DEBUG("Floor Call Payload: %.*s", (int)data_len, (char*)data);

        cJSON *json_payload = cJSON_ParseWithLength((const char*)data, data_len);
        if (!json_payload) {
            SRV_LOG_ERROR("Error parsing JSON payload: %s", cJSON_GetErrorPtr());
            coap_pdu_set_code(response, COAP_RESPONSE_CODE_BAD_REQUEST);
            cJSON *error_json = cJSON_CreateObject();
            cJSON_AddStringToObject(error_json, "error", "Invalid JSON payload");
            cJSON_AddStringToObject(error_json, "details", cJSON_GetErrorPtr());
            char *error_str = cJSON_PrintUnformatted(error_json);
            coap_add_option(response, COAP_OPTION_CONTENT_FORMAT, coap_encode_var_safe( (uint8_t[2]){0}, 2, COAP_MEDIATYPE_APPLICATION_JSON), (uint8_t[2]){0});
            coap_add_data(response, strlen(error_str), (const uint8_t*)error_str);
            cJSON_Delete(error_json);
            free(error_str);
            return;
        }

        cJSON *j_id_edificio = cJSON_GetObjectItemCaseSensitive(json_payload, "id_edificio");
        cJSON *j_piso_origen_llamada = cJSON_GetObjectItemCaseSensitive(json_payload, "piso_origen_llamada");
        cJSON *j_direccion_llamada = cJSON_GetObjectItemCaseSensitive(json_payload, "direccion_llamada");
        cJSON *j_elevadores_estado = cJSON_GetObjectItemCaseSensitive(json_payload, "elevadores_estado");

        if (!cJSON_IsString(j_id_edificio) || !cJSON_IsNumber(j_piso_origen_llamada) ||
            !cJSON_IsString(j_direccion_llamada) || !cJSON_IsArray(j_elevadores_estado)) {
            SRV_LOG_ERROR("Missing or invalid fields in JSON payload for floor call (expected id_edificio, piso_origen_llamada, direccion_llamada, elevadores_estado).");
            coap_pdu_set_code(response, COAP_RESPONSE_CODE_BAD_REQUEST);
            cJSON *error_json = cJSON_CreateObject();
            cJSON_AddStringToObject(error_json, "error", "Missing or invalid fields in JSON payload for floor call.");
            cJSON_AddStringToObject(error_json, "expected_fields", "id_edificio (string), piso_origen_llamada (number), direccion_llamada (string), elevadores_estado (array)");
            char *error_str = cJSON_PrintUnformatted(error_json);
            coap_add_option(response, COAP_OPTION_CONTENT_FORMAT, coap_encode_var_safe( (uint8_t[2]){0}, 2, COAP_MEDIATYPE_APPLICATION_JSON), (uint8_t[2]){0});
            coap_add_data(response, strlen(error_str), (const uint8_t*)error_str);
            cJSON_Delete(error_json);
            free(error_str);
            cJSON_Delete(json_payload);
            return;
        }

        char *id_edificio = j_id_edificio->valuestring;
        int piso_origen = j_piso_origen_llamada->valueint;
        char *direccion_llamada = j_direccion_llamada->valuestring;

        SRV_LOG_INFO("Floor call from Edificio '%s', Piso Origen Llamada %d, Direccion '%s'", id_edificio, piso_origen, direccion_llamada);

        // Usar algoritmo de asignación inteligente basado en proximidad
        char *assigned_elevator_id = select_optimal_elevator(j_elevadores_estado, piso_origen, direccion_llamada);

        if (assigned_elevator_id) {
            char task_id[32];
            generate_unique_task_id(task_id, sizeof(task_id));
            SRV_LOG_INFO("Assigning task %s to elevator %s for floor call from piso %d (Edificio: %s)", 
                        task_id, assigned_elevator_id, piso_origen, id_edificio);

            cJSON *response_json = cJSON_CreateObject();
            cJSON_AddStringToObject(response_json, "tarea_id", task_id);
            cJSON_AddStringToObject(response_json, "ascensor_asignado_id", assigned_elevator_id);

            char *response_str = cJSON_PrintUnformatted(response_json);
            coap_pdu_set_code(response, COAP_RESPONSE_CODE_CONTENT);
            coap_add_option(response, COAP_OPTION_CONTENT_FORMAT, coap_encode_var_safe( (uint8_t[2]){0}, 2, COAP_MEDIATYPE_APPLICATION_JSON), (uint8_t[2]){0});
            coap_add_data(response, strlen(response_str), (const uint8_t *)response_str);

            cJSON_Delete(response_json);
            free(response_str);
            free(assigned_elevator_id); // Liberar el ID asignado
        } else {
            SRV_LOG_WARN("No elevators available for floor call from edificio '%s', piso %d", id_edificio, piso_origen);
            coap_pdu_set_code(response, COAP_RESPONSE_CODE_SERVICE_UNAVAILABLE);
            cJSON *error_json = cJSON_CreateObject();
            cJSON_AddStringToObject(error_json, "error", "No elevators available at the moment.");
            cJSON_AddStringToObject(error_json, "edificio", id_edificio);
            cJSON_AddNumberToObject(error_json, "piso_origen", piso_origen);
            char *error_str = cJSON_PrintUnformatted(error_json);
            coap_add_option(response, COAP_OPTION_CONTENT_FORMAT, coap_encode_var_safe( (uint8_t[2]){0}, 2, COAP_MEDIATYPE_APPLICATION_JSON), (uint8_t[2]){0});
            coap_add_data(response, strlen(error_str), (const uint8_t*)error_str);
            cJSON_Delete(error_json);
            free(error_str);
        }
        cJSON_Delete(json_payload);

    } else {
        SRV_LOG_ERROR("Received floor call request with no payload.");
        coap_pdu_set_code(response, COAP_RESPONSE_CODE_BAD_REQUEST);
        cJSON *error_json = cJSON_CreateObject();
        cJSON_AddStringToObject(error_json, "error", "Missing payload for floor call request.");
        char *error_str = cJSON_PrintUnformatted(error_json);
        coap_add_option(response, COAP_OPTION_CONTENT_FORMAT, coap_encode_var_safe( (uint8_t[2]){0}, 2, COAP_MEDIATYPE_APPLICATION_JSON), (uint8_t[2]){0});
        coap_add_data(response, strlen(error_str), (const uint8_t*)error_str);
        cJSON_Delete(error_json);
        free(error_str);
    }
}

/**
 * @brief Manejador CoAP para solicitudes de cabina
 * @param resource Recurso CoAP que recibió la solicitud
 * @param session Sesión CoAP del cliente que envió la solicitud
 * @param request PDU de la solicitud CoAP recibida
 * @param query Parámetros de consulta de la URI (no utilizado)
 * @param response PDU de respuesta CoAP a enviar al cliente
 * 
 * Esta función procesa las solicitudes de cabina (cabin requests)
 * recibidas desde los API Gateways. Maneja solicitudes de destino
 * realizadas desde el interior de los ascensores.
 * 
 * **Formato JSON esperado:**
 * ```json
 * {
 *   "id_edificio": "E1",
 *   "solicitando_ascensor_id": "E1A1",
 *   "piso_destino_solicitud": 8,
 *   "elevadores_estado": [
 *     {
 *       "id_ascensor": "E1A1",
 *       "piso_actual": 3,
 *       "estado_puerta": "CERRADA",
 *       "disponible": true,
 *       "tarea_actual_id": null,
 *       "destino_actual": null
 *     }
 *   ]
 * }
 * ```
 * 
 * **Respuesta JSON:**
 * ```json
 * {
 *   "tarea_id": "T_1640995200456",
 *   "ascensor_asignado_id": "E1A1"
 * }
 * ```
 * 
 * **Algoritmo de asignación:**
 * Para solicitudes de cabina, el ascensor asignado es siempre
 * el mismo que realizó la solicitud (auto-asignación). Esto
 * es lógico ya que la solicitud proviene del interior del ascensor.
 * 
 * **Validaciones realizadas:**
 * - Formato JSON válido
 * - Presencia de campos obligatorios
 * - Tipos de datos correctos
 * - Array de estado de ascensores válido
 * 
 * @see generate_unique_task_id()
 * @see RESOURCE_CABIN_REQUEST
 */
static void hnd_cabin_request(coap_resource_t *resource, coap_session_t *session,
                              const coap_pdu_t *request, const coap_string_t *query,
                              coap_pdu_t *response)
{
    const coap_str_const_t *uri_path_cr = coap_resource_get_uri_path(resource);
    if (uri_path_cr) {
        SRV_LOG_INFO("Received request on /%.*s (Peticion Cabina)", (int)uri_path_cr->length, uri_path_cr->s);
    } else {
        SRV_LOG_INFO("Received request on /[unknown_path] (Peticion Cabina)");
    }

    const uint8_t *data;
    size_t data_len;
    if (coap_get_data(request, &data_len, &data)) {
        SRV_LOG_DEBUG("Cabin Request Payload: %.*s", (int)data_len, (char*)data);

        cJSON *json_payload = cJSON_ParseWithLength((const char*)data, data_len);
        if (!json_payload) {
            SRV_LOG_ERROR("Error parsing JSON payload for cabin request: %s", cJSON_GetErrorPtr());
            coap_pdu_set_code(response, COAP_RESPONSE_CODE_BAD_REQUEST);
            cJSON *error_json = cJSON_CreateObject();
            cJSON_AddStringToObject(error_json, "error", "Invalid JSON payload for cabin request");
            cJSON_AddStringToObject(error_json, "details", cJSON_GetErrorPtr());
            char *error_str = cJSON_PrintUnformatted(error_json);
            coap_add_option(response, COAP_OPTION_CONTENT_FORMAT, coap_encode_var_safe( (uint8_t[2]){0}, 2, COAP_MEDIATYPE_APPLICATION_JSON), (uint8_t[2]){0});
            coap_add_data(response, strlen(error_str), (const uint8_t*)error_str);
            cJSON_Delete(error_json);
            free(error_str);
            return;
        }

        cJSON *j_id_edificio = cJSON_GetObjectItemCaseSensitive(json_payload, "id_edificio");
        cJSON *j_solicitando_ascensor_id = cJSON_GetObjectItemCaseSensitive(json_payload, "solicitando_ascensor_id");
        cJSON *j_piso_destino_solicitud = cJSON_GetObjectItemCaseSensitive(json_payload, "piso_destino_solicitud");
        cJSON *j_elevadores_estado = cJSON_GetObjectItemCaseSensitive(json_payload, "elevadores_estado"); // Though not directly used for assignment logic here

        if (!cJSON_IsString(j_id_edificio) || !cJSON_IsString(j_solicitando_ascensor_id) ||
            !cJSON_IsNumber(j_piso_destino_solicitud) || !cJSON_IsArray(j_elevadores_estado)) {
            SRV_LOG_ERROR("Missing or invalid fields in JSON payload for cabin request (expected id_edificio, solicitando_ascensor_id, piso_destino_solicitud, elevadores_estado).");
            coap_pdu_set_code(response, COAP_RESPONSE_CODE_BAD_REQUEST);
            cJSON *error_json = cJSON_CreateObject();
            cJSON_AddStringToObject(error_json, "error", "Missing or invalid fields in JSON payload for cabin request.");
            cJSON_AddStringToObject(error_json, "expected_fields", "id_edificio (string), solicitando_ascensor_id (string), piso_destino_solicitud (number), elevadores_estado (array)");
            char *error_str = cJSON_PrintUnformatted(error_json);
            coap_add_option(response, COAP_OPTION_CONTENT_FORMAT, coap_encode_var_safe( (uint8_t[2]){0}, 2, COAP_MEDIATYPE_APPLICATION_JSON), (uint8_t[2]){0});
            coap_add_data(response, strlen(error_str), (const uint8_t*)error_str);
            cJSON_Delete(error_json);
            free(error_str);
            cJSON_Delete(json_payload);
            return;
        }

        char *id_edificio = j_id_edificio->valuestring;
        char *solicitando_ascensor_id = j_solicitando_ascensor_id->valuestring;
        int piso_destino_solicitud = j_piso_destino_solicitud->valueint;

        SRV_LOG_INFO("Cabin request from Edificio '%s', Ascensor '%s' to Piso Destino %d", 
                     id_edificio, solicitando_ascensor_id, piso_destino_solicitud);

        // For a cabin request, the assigned elevator is the one making the request.
        char task_id[32];
        generate_unique_task_id(task_id, sizeof(task_id));
        SRV_LOG_INFO("Assigning task %s to elevator %s for its cabin request to piso %d", 
                     task_id, solicitando_ascensor_id, piso_destino_solicitud);

        cJSON *response_json = cJSON_CreateObject();
        cJSON_AddStringToObject(response_json, "tarea_id", task_id);
        cJSON_AddStringToObject(response_json, "ascensor_asignado_id", solicitando_ascensor_id); // Assign to self

        char *response_str = cJSON_PrintUnformatted(response_json);
        coap_pdu_set_code(response, COAP_RESPONSE_CODE_CONTENT);
        coap_add_option(response, COAP_OPTION_CONTENT_FORMAT, coap_encode_var_safe( (uint8_t[2]){0}, 2, COAP_MEDIATYPE_APPLICATION_JSON), (uint8_t[2]){0});
        coap_add_data(response, strlen(response_str), (const uint8_t *)response_str);

        cJSON_Delete(response_json);
        free(response_str);
        cJSON_Delete(json_payload);

    } else {
        SRV_LOG_ERROR("Received cabin request with no payload.");
        coap_pdu_set_code(response, COAP_RESPONSE_CODE_BAD_REQUEST);
        cJSON *error_json = cJSON_CreateObject();
        cJSON_AddStringToObject(error_json, "error", "Missing payload for cabin request.");
        char *error_str = cJSON_PrintUnformatted(error_json);
        coap_add_option(response, COAP_OPTION_CONTENT_FORMAT, coap_encode_var_safe( (uint8_t[2]){0}, 2, COAP_MEDIATYPE_APPLICATION_JSON), (uint8_t[2]){0});
        coap_add_data(response, strlen(error_str), (const uint8_t*)error_str);
        cJSON_Delete(error_json);
        free(error_str);
    }
}

// REMOVED: preguntar_reestablecer_db()
// REMOVED: hnd_process_elevator_request()

int main(int argc, char **argv) {
    // REMOVED: preguntar_reestablecer_db();

    coap_context_t  *ctx = NULL;
    coap_address_t   serv_addr;
    coap_resource_t *r_floor_call = NULL;
    coap_resource_t *r_cabin_request = NULL;
    // coap_resource_t *r_arrival_notification = NULL; // Eliminado
    // REMOVED: coap_resource_t *r_process_elevator = NULL;

    signal(SIGINT, handle_sigint);
    SRV_LOG_INFO(ANSI_COLOR_GREEN "--- Servidor Central Ascensores CoAP (Stateless Dispatcher) ---" ANSI_COLOR_RESET);

    coap_set_log_level(LOG_DEBUG);
    coap_startup();
    SRV_LOG_INFO("libCoAP initialized.");

    // REMOVED: Database initialization block

    coap_address_init(&serv_addr);
    serv_addr.addr.sin.sin_family = AF_INET;
    if (inet_pton(AF_INET, SERVER_IP, &serv_addr.addr.sin.sin_addr) != 1) {
        SRV_LOG_ERROR("CRITICAL: Failed to convert server IP address '%s'. Error: %s. Exiting.", SERVER_IP, strerror(errno));
        coap_cleanup();
        return EXIT_FAILURE;
    }
    serv_addr.addr.sin.sin_port = htons(atoi(SERVER_PORT));

    ctx = coap_new_context(NULL);
    if (!ctx) {
        SRV_LOG_ERROR("Servidor Central: Cannot create CoAP context. Exiting.");
        coap_cleanup(); 
        return EXIT_FAILURE;
    }

    // Configurar PSK para el contexto del servidor usando la clave y el hint definidos
    if (!coap_context_set_psk(ctx, PSK_SERVER_HINT, (const uint8_t *)PSK_KEY, strlen(PSK_KEY))) {
        SRV_LOG_ERROR("Error: No se pudo configurar la información PSK del servidor (coap_context_set_psk falló). Error: %s. Asegúrate de que libcoap esté compilada con soporte DTLS-PSK.", strerror(errno));
    } else {
        SRV_LOG_INFO("Información PSK del servidor configurada mediante coap_context_set_psk (hint: %s).", PSK_SERVER_HINT);
    }

    coap_endpoint_t *endpoint = coap_new_endpoint(ctx, &serv_addr, COAP_PROTO_DTLS);
    if (!endpoint) {
        SRV_LOG_ERROR("CRITICAL: Failed to create CoAP server endpoint on DTLS %s:%s. Error: %s. Is address/port in use or DTLS setup failed? Exiting.", SERVER_IP, SERVER_PORT, strerror(errno));
        coap_free_context(ctx);
        coap_cleanup();
        return EXIT_FAILURE;
    }
    SRV_LOG_INFO("CoAP server listening on DTLS %s:%s", SERVER_IP, SERVER_PORT);

    // --- Register Resources (stateless versions) ---
    r_floor_call = coap_resource_init(coap_make_str_const(RESOURCE_FLOOR_CALL), 0);
    if (!r_floor_call) {
        SRV_LOG_ERROR("Failed to init resource /%s. Exiting.", RESOURCE_FLOOR_CALL);
        goto finish;
    }
    coap_register_handler(r_floor_call, COAP_REQUEST_POST, hnd_floor_call);
    coap_add_resource(ctx, r_floor_call);
    SRV_LOG_INFO("Registered resource: POST /%s", RESOURCE_FLOOR_CALL);

    r_cabin_request = coap_resource_init(coap_make_str_const(RESOURCE_CABIN_REQUEST), 0);
    if (!r_cabin_request) {
        SRV_LOG_ERROR("Failed to init resource /%s. Exiting.", RESOURCE_CABIN_REQUEST);
        goto finish;
    }
    coap_register_handler(r_cabin_request, COAP_REQUEST_POST, hnd_cabin_request);
    coap_add_resource(ctx, r_cabin_request);
    SRV_LOG_INFO("Registered resource: POST /%s", RESOURCE_CABIN_REQUEST);

    // r_arrival_notification = coap_resource_init(coap_make_str_const(RESOURCE_ARRIVAL), 0); // Eliminado
    // if (!r_arrival_notification) { // Eliminado
    //     SRV_LOG_ERROR("Failed to init resource /%s. Exiting.", RESOURCE_ARRIVAL); // Eliminado
    //     goto finish; // Eliminado
    // } // Eliminado
    // coap_register_handler(r_arrival_notification, COAP_REQUEST_POST, hnd_arrival_notification); // Eliminado
    // coap_add_resource(ctx, r_arrival_notification); // Eliminado
    // SRV_LOG_INFO("Registered resource: POST /%s", RESOURCE_ARRIVAL); // Eliminado

    // REMOVED: r_process_elevator resource

    SRV_LOG_INFO(ANSI_COLOR_GREEN "Stateless CoAP dispatcher server started. Waiting for requests... (Ctrl+C to stop)" ANSI_COLOR_RESET);

    while (running) {
        int result = coap_io_process(ctx, 1000);
        if (result < 0) {
            SRV_LOG_ERROR("Error in coap_io_process: %d. Shutting down.", result);
            running = 0;
        }
    }

finish:
    SRV_LOG_WARN("Shutting down CoAP server...");
    if (ctx) {
        coap_free_context(ctx);
        SRV_LOG_INFO("CoAP context freed.");
    }
    // REMOVED: db_close(db);
    coap_cleanup();
    SRV_LOG_INFO("libCoAP cleaned up.");
    SRV_LOG_INFO(ANSI_COLOR_GREEN "Server exited cleanly." ANSI_COLOR_RESET);
    return EXIT_SUCCESS;
} 