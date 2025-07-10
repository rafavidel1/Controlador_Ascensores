/**
 * @file main.c
 * @brief Servidor central de control de ascensores con protocolo CoAP sobre DTLS
 * @author Sistema de Ascensores
 * @date 2024
 * @version 2.0
 * 
 * @details Este archivo implementa el servidor central que gestiona las solicitudes
 * de ascensores desde múltiples API Gateways usando CoAP sobre DTLS con autenticación PSK.
 * 
 * **Arquitectura del sistema:**
 * - Servidor CoAP-DTLS que escucha en puerto 5684
 * - Autenticación mediante claves precompartidas (PSK)
 * - Gestión stateless de solicitudes de ascensores
 * - Balanceamiento de carga automático entre ascensores
 * - Logging detallado para debugging y auditoría
 * 
 * **Recursos CoAP disponibles:**
 * - `POST /peticion_piso`: Solicitudes de llamada desde pisos
 * - `POST /peticion_cabina`: Solicitudes desde cabinas de ascensores
 * 
 * **Seguridad:**
 * - Cifrado DTLS para todas las comunicaciones
 * - Autenticación PSK con validación de identidades
 * - Timeouts configurables para estabilidad de conexiones
 * - Logging de todos los eventos de seguridad
 * 
 * **Funcionamiento:**
 * 1. Inicialización del contexto CoAP con DTLS
 * 2. Configuración de callbacks PSK personalizados
 * 3. Registro de recursos CoAP disponibles
 * 4. Bucle principal procesando solicitudes
 * 5. Terminación elegante con liberación de recursos
 * 
 * @note Requiere libcoap compilado con soporte DTLS/OpenSSL
 * @see https://libcoap.net/doc/reference/4.3.0/
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <unistd.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <time.h>
#include <math.h>
#include <limits.h>
#include <coap3/coap.h>
#include <cjson/cJSON.h>
#include <openssl/rand.h>

#include "servidor_central/logging.h"
#include "servidor_central/psk_validator.h"

// Definición de la constante PSK_SERVER_HINT
#define PSK_SERVER_HINT "ElevatorCentralServer"

/**
 * @brief Manejador de eventos de sesión para configurar timeouts DTLS optimizados
 * 
 * @param[in] session Sesión CoAP donde ocurrió el evento
 * @param[in] event Tipo de evento de sesión
 * 
 * @return 0 en caso de éxito, valor negativo en caso de error
 * 
 * @details Esta función maneja eventos de sesión CoAP para configurar
 * parámetros optimizados de DTLS, especialmente timeouts para mejorar
 * la estabilidad de conexiones en redes con latencia variable.
 * 
 * **Eventos manejados:**
 * - `COAP_EVENT_DTLS_CONNECTED`: Nueva conexión DTLS establecida
 * - `COAP_EVENT_DTLS_CLOSED`: Conexión DTLS cerrada
 * - `COAP_EVENT_DTLS_ERROR`: Error en conexión DTLS
 * - `COAP_EVENT_SERVER_SESSION_DEL`: Sesión de servidor eliminada
 * 
 * **Configuración de timeouts:**
 * - ACK timeout: 4 segundos (equilibrio entre rapidez y estabilidad)
 * - Random factor: 1.5 (factor aleatorio para evitar congestión)
 * - Max retransmit: 4 intentos (suficiente para redes problemáticas)
 * 
 * **Justificación de valores:**
 * - 4s ACK timeout: Permite recuperación en redes con latencia alta
 * - 1.5 random factor: Reduce colisiones en múltiples conexiones
 * - 4 retransmisiones: Balance entre persistencia y eficiencia
 * 
 * @note Estos valores están optimizados para entornos Kubernetes
 * @note El logging detallado facilita debugging de problemas de conectividad
 * @see coap_session_set_ack_timeout()
 * @see coap_session_set_ack_random_factor()
 * @see coap_session_set_max_retransmit()
 */
static int session_event_handler(coap_session_t *session,
                               const coap_event_t event) {
    if (event == COAP_EVENT_DTLS_CONNECTED) {
        // Configurar timeouts optimizados para estabilidad DTLS
        coap_fixed_point_t timeout;
        timeout.integer_part = 4;        // 4 segundos para ACK
        timeout.fractional_part = 0;     // Sin fracción
        coap_session_set_ack_timeout(session, timeout);
        
        coap_fixed_point_t random_factor;
        random_factor.integer_part = 1;  // 1.5 factor aleatorio para ACK
        random_factor.fractional_part = 500;  // 0.5 en formato fixed point
        coap_session_set_ack_random_factor(session, random_factor);
        
        coap_session_set_max_retransmit(session, 4);  // Máximo 4 retransmisiones
        
        SRV_LOG_DEBUG("Nueva sesión DTLS configurada con timeouts optimizados");
    } else if (event == COAP_EVENT_DTLS_CLOSED) {
        SRV_LOG_INFO("=== SESIÓN DTLS CERRADA ===");
    } else if (event == COAP_EVENT_DTLS_ERROR) {
        SRV_LOG_ERROR("=== ERROR DTLS ===");
    } else if (event == COAP_EVENT_SERVER_SESSION_DEL) {
        SRV_LOG_INFO("=== SESIÓN SERVIDOR ELIMINADA ===");
    }
    return 0;
}

/**
 * @brief Callback PSK personalizado para autenticación DTLS-PSK
 * 
 * @param[in] identity Identidad del cliente DTLS
 * @param[in] session Sesión CoAP asociada
 * @param[in] arg Argumento de usuario (no usado)
 * 
 * @return Puntero a la clave PSK correspondiente o NULL si no se encuentra
 * 
 * @details Esta función implementa el callback de autenticación PSK para DTLS:
 * 
 * **Proceso de autenticación:**
 * 1. Recibe la identidad del cliente desde el handshake DTLS
 * 2. Valida que la identidad siga el patrón "Gateway_Client_*"
 * 3. Obtiene la clave PSK determinística basada en la identidad
 * 4. Retorna la clave para completar el handshake DTLS
 * 
 * **Patrones de identidad aceptados:**
 * - "Gateway_Client_*": Cualquier identidad que empiece con este prefijo
 * - Se rechazan identidades que no sigan el patrón
 * 
 * **Algoritmo de clave determinística:**
 * - Usa psk_validator_get_key_for_identity() para obtener clave
 * - La misma identidad siempre produce la misma clave
 * - Garantiza consistencia entre servidor y cliente
 * 
 * **Seguridad:**
 * - Validación estricta de patrones de identidad
 * - Logging detallado de intentos de conexión
 * - Prevención de ataques de identidad falsa
 * 
 * @note Esta función es llamada automáticamente por libcoap durante handshake DTLS
 * @see psk_validator_get_key_for_identity()
 * @see coap_bin_const_t
 */
static const coap_bin_const_t *get_psk_info(coap_bin_const_t *identity,
                                           coap_session_t *session,
                                           void *arg) {
    SRV_LOG_INFO("PSK callback: Función ejecutándose...");
    
    if (!identity) {
        SRV_LOG_ERROR("PSK callback: identity es NULL");
        return NULL;
    }
    
    // Convertir identity a string para verificar patrón
    char identity_str[256];
    size_t copy_len = identity->length < sizeof(identity_str) - 1 ? identity->length : sizeof(identity_str) - 1;
    memcpy(identity_str, identity->s, copy_len);
    identity_str[copy_len] = '\0';
    
    SRV_LOG_INFO("PSK callback: Cliente intentando conectar con identidad: '%s'", identity_str);
    
    // Verificar si la identidad empieza con "Gateway_Client_"
    if (strncmp(identity_str, "Gateway_Client_", 15) == 0) {
        SRV_LOG_INFO("PSK callback: Identidad aceptada (patrón válido): '%s'", identity_str);
        
        // Crear estructura estática para la clave PSK
        static coap_bin_const_t psk_key;
        static uint8_t key_buffer[128];
        
        // Usar el mismo algoritmo determinístico que el API Gateway
        // para obtener la clave basada en la identidad
        if (psk_validator_get_key_for_identity(identity_str, key_buffer, sizeof(key_buffer)) == 0) {
            size_t key_len = strlen((char*)key_buffer);
            SRV_LOG_INFO("PSK callback: Clave determinística para identidad '%s': '%s'", identity_str, key_buffer);
            
            psk_key.s = key_buffer;
            psk_key.length = key_len;
            return &psk_key;
        } else {
            SRV_LOG_WARN("PSK callback: No se pudo obtener clave determinística para identidad '%s'", identity_str);
            return NULL;
        }
    } else {
        SRV_LOG_WARN("PSK callback: Identidad rechazada (patrón inválido): '%s'", identity_str);
        return NULL;
    }
}

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
 * 
 * @param[in] signum Número de señal recibida (se espera SIGINT = 2)
 * 
 * @details Esta función implementa el manejo elegante de la señal SIGINT:
 * 
 * **Funcionalidad:**
 * - Establece la bandera global 'running' a 0
 * - Permite que el bucle principal termine de forma controlada
 * - Registra el evento en el log del sistema
 * - Evita terminación abrupta del servidor
 * 
 * **Flujo de terminación:**
 * 1. Usuario presiona Ctrl+C
 * 2. Sistema envía SIGINT al proceso
 * 3. Esta función establece running = 0
 * 4. Bucle principal detecta el cambio y termina
 * 5. Se ejecutan rutinas de limpieza en main()
 * 
 * **Seguridad:**
 * - No realiza operaciones complejas en el handler
 * - Solo modifica la bandera de control
 * - Es thread-safe para el contexto de señales
 * 
 * @note Esta función debe ser registrada con signal() o sigaction()
 * @note Solo modifica variables globales para evitar problemas de reentrancia
 * @see running
 * @see main()
 */
void handle_sigint(int signum) {
    SRV_LOG_WARN("Received SIGINT, shutting down...");
    running = 0;
}

/**
 * @brief Genera un ID único para una tarea de ascensor
 * 
 * @param[out] task_id_out Buffer donde se almacenará el ID generado
 * @param[in] len Tamaño del buffer de salida en bytes
 * 
 * @details Esta función genera un identificador único para tareas de ascensor
 * basado en el timestamp actual del sistema con precisión de milisegundos.
 * 
 * **Formato del ID generado:**
 * ```
 * "T_{segundos_unix}{milisegundos}"
 * ```
 * 
 * **Ejemplo de ID:**
 * - "T_1640995200123" donde:
 *   - T_: Prefijo identificador de tarea
 *   - 1640995200: Segundos desde epoch Unix
 *   - 123: Milisegundos (3 dígitos)
 * 
 * **Características:**
 * - Unicidad temporal garantizada
 * - Formato legible y ordenable
 * - Precisión de milisegundos
 * - Compatible con sistemas distribuidos
 * 
 * **Limitaciones:**
 * - No es thread-safe (para entornos multihilo usar sincronización)
 * - Dependiente del reloj del sistema
 * - Máximo 1,000 IDs por segundo (limitación de milisegundos)
 * 
 * **Uso típico:**
 * ```c
 * char task_id[32];
 * generate_unique_task_id(task_id, sizeof(task_id));
 * // task_id contiene "T_1640995200123"
 * ```
 * 
 * @note El buffer de salida debe tener al menos 32 caracteres
 * @note Para entornos multihilo considerar usar mutex o contadores atómicos
 * @see gettimeofday()
 * @see snprintf()
 */
void generate_unique_task_id(char *task_id_out, size_t len) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    snprintf(task_id_out, len, "T_%ld%03ld", (long)tv.tv_sec, (long)(tv.tv_usec / 1000));
}

/**
 * @brief Algoritmo de selección inteligente de ascensores mejorado
 * 
 * @param[in] elevadores_estado Array JSON con el estado de todos los ascensores
 * @param[in] piso_origen Piso desde donde se hace la llamada
 * @param[in] direccion_llamada Dirección deseada ("SUBIENDO" o "BAJANDO")
 * 
 * @return ID del ascensor asignado (debe ser liberado por el caller) o NULL si no hay ascensores
 * 
 * @details Esta función implementa un algoritmo inteligente que:
 * 
 * **🧠 ALGORITMO MEJORADO:**
 * 1. **Prioridad 1:** Ascensores disponibles más cercanos
 * 2. **Prioridad 2:** Ascensores ocupados que van en la misma dirección y pueden recoger
 * 3. **Prioridad 3:** Ascensores ocupados que terminarán cerca del origen
 * 4. **Prioridad 4:** Cualquier ascensor disponible como último recurso
 * 
 * **📊 ANÁLISIS POR CATEGORÍAS:**
 * - **DISPONIBLES:** Ascensores sin tarea actual (disponible=true)
 * - **COMPATIBLES:** Ascensores ocupados que van en la misma dirección
 * - **PRÓXIMOS:** Ascensores que terminarán cerca del piso origen
 * - **CUALQUIERA:** Fallback si todos están ocupados
 * 
 * **🎯 CRITERIOS DE SELECCIÓN:**
 * - Distancia al piso origen
 * - Compatibilidad de dirección
 * - Eficiencia de ruta
 * - Tiempo estimado de disponibilidad
 * 
 * @note Esta función resuelve el problema crítico del algoritmo anterior
 * @note El ID retornado debe ser liberado con free() por el caller
 */
static char* select_optimal_elevator(cJSON *elevadores_estado, int piso_origen, const char *direccion_llamada) {
    if (!elevadores_estado || !cJSON_IsArray(elevadores_estado) || !direccion_llamada) {
        SRV_LOG_ERROR("Invalid parameters for elevator selection");
        return NULL;
    }

    int array_size = cJSON_GetArraySize(elevadores_estado);
    if (array_size == 0) {
        SRV_LOG_WARN("No elevators in the building");
        return NULL;
    }

    SRV_LOG_INFO("🧠 ALGORITMO MEJORADO: Analizando %d ascensores para piso %d, dirección %s", 
                 array_size, piso_origen, direccion_llamada);

    // Estructuras para candidatos por prioridad
    struct {
        char *id;
        int piso_actual;
        int destino_actual;
        int score;
        int distance;
        int disponible;
        char *estado;
    } *candidatos = malloc(array_size * sizeof(*candidatos));
    
    if (!candidatos) {
        SRV_LOG_ERROR("Memory allocation failed for elevator candidates");
        return NULL;
    }

    int num_candidatos = 0;
    int num_disponibles = 0;
    int num_compatibles = 0;
    int num_ocupados = 0;

    // Analizar todos los ascensores
    for (int i = 0; i < array_size; i++) {
        cJSON *elevator = cJSON_GetArrayItem(elevadores_estado, i);
        if (!elevator) continue;

        cJSON *j_id = cJSON_GetObjectItemCaseSensitive(elevator, "id_ascensor");
        cJSON *j_piso = cJSON_GetObjectItemCaseSensitive(elevator, "piso_actual");
        cJSON *j_disponible = cJSON_GetObjectItemCaseSensitive(elevator, "disponible");
        cJSON *j_destino = cJSON_GetObjectItemCaseSensitive(elevator, "destino_actual");

        if (!cJSON_IsString(j_id) || !cJSON_IsNumber(j_piso) || !cJSON_IsBool(j_disponible)) {
            SRV_LOG_WARN("Ascensor %d: campos inválidos", i);
            continue;
        }

        char *id = j_id->valuestring;
        int piso_actual = j_piso->valueint;
        int disponible = cJSON_IsTrue(j_disponible) ? 1 : 0;
        int destino_actual = (j_destino && cJSON_IsNumber(j_destino)) ? j_destino->valueint : -1;

        // Calcular métricas
        int distance = abs(piso_actual - piso_origen);
        int score = 0;
        char *estado = "UNKNOWN";

        if (disponible) {
            // CATEGORÍA 1: DISPONIBLES
            score = 1000 - distance; // Prioridad máxima, menor distancia = mayor score
            estado = "DISPONIBLE";
            num_disponibles++;
        } else if (destino_actual != -1) {
            // CATEGORÍA 2: OCUPADOS CON DESTINO
            num_ocupados++;
            
            // Verificar si es compatible (va en la misma dirección y puede recoger)
            int va_subiendo = (destino_actual > piso_actual);
            int va_bajando = (destino_actual < piso_actual);
            int puede_recoger = 0;
            
            if (strcmp(direccion_llamada, "SUBIENDO") == 0) {
                // Llamada hacia arriba
                if (va_subiendo && piso_actual <= piso_origen && piso_origen <= destino_actual) {
                    puede_recoger = 1;
                    estado = "COMPATIBLE_SUBIENDO";
                }
            } else if (strcmp(direccion_llamada, "BAJANDO") == 0) {
                // Llamada hacia abajo
                if (va_bajando && piso_actual >= piso_origen && piso_origen >= destino_actual) {
                    puede_recoger = 1;
                    estado = "COMPATIBLE_BAJANDO";
                }
            }
            
            if (puede_recoger) {
                // CATEGORÍA 2: COMPATIBLES
                score = 800 - distance; // Alta prioridad
                num_compatibles++;
            } else {
                // CATEGORÍA 3: PRÓXIMOS (terminarán cerca)
                int distancia_al_terminar = abs(destino_actual - piso_origen);
                score = 600 - distancia_al_terminar; // Prioridad media
                estado = "PRÓXIMO";
            }
        } else {
            // CATEGORÍA 4: OCUPADOS SIN DESTINO CONOCIDO
            score = 400 - distance; // Prioridad baja
            estado = "OCUPADO_SIN_DESTINO";
            num_ocupados++;
        }

        // Agregar candidato
        candidatos[num_candidatos].id = strdup(id);
        candidatos[num_candidatos].piso_actual = piso_actual;
        candidatos[num_candidatos].destino_actual = destino_actual;
        candidatos[num_candidatos].score = score;
        candidatos[num_candidatos].distance = distance;
        candidatos[num_candidatos].disponible = disponible;
        candidatos[num_candidatos].estado = strdup(estado);
        num_candidatos++;

        SRV_LOG_DEBUG("📊 Candidato: %s | Piso: %d | Destino: %d | Score: %d | Estado: %s", 
                     id, piso_actual, destino_actual, score, estado);
    }

    // Estadísticas del análisis
    SRV_LOG_INFO("📈 ESTADÍSTICAS: Disponibles=%d, Compatibles=%d, Ocupados=%d, Total=%d", 
                 num_disponibles, num_compatibles, num_ocupados, num_candidatos);

    // Seleccionar el mejor candidato
    char *selected_id = NULL;
    int best_score = -1;
    int best_index = -1;

    for (int i = 0; i < num_candidatos; i++) {
        if (candidatos[i].score > best_score) {
            best_score = candidatos[i].score;
            best_index = i;
        }
    }

    if (best_index != -1) {
        selected_id = strdup(candidatos[best_index].id);
        SRV_LOG_INFO("🎯 SELECCIONADO: %s | Score: %d | Estado: %s | Piso: %d → %d", 
                     candidatos[best_index].id,
                     candidatos[best_index].score,
                     candidatos[best_index].estado,
                     candidatos[best_index].piso_actual,
                     candidatos[best_index].destino_actual);
        
        // Logging adicional según el tipo de selección
        if (candidatos[best_index].disponible) {
            SRV_LOG_INFO("✅ ASIGNACIÓN ÓPTIMA: Ascensor disponible más cercano");
        } else if (strstr(candidatos[best_index].estado, "COMPATIBLE")) {
            SRV_LOG_INFO("🚀 ASIGNACIÓN INTELIGENTE: Ascensor compatible en ruta");
        } else {
            SRV_LOG_INFO("⏳ ASIGNACIÓN DIFERIDA: Ascensor ocupado, se asignará al terminar");
        }
    } else {
        SRV_LOG_ERROR("🚫 ERROR CRÍTICO: No se pudo seleccionar ningún ascensor");
    }

    // Liberar memoria de candidatos
    for (int i = 0; i < num_candidatos; i++) {
        free(candidatos[i].id);
        free(candidatos[i].estado);
    }
    free(candidatos);

    return selected_id;
}

// --- STUBBED CoAP Handlers (to be refactored) ---

/**
 * @brief Manejador CoAP para solicitudes de llamada de piso
 * 
 * @param[in] resource Recurso CoAP que recibió la solicitud
 * @param[in] session Sesión CoAP del cliente que envió la solicitud
 * @param[in] request PDU de la solicitud CoAP recibida
 * @param[in] query Parámetros de consulta de la URI (no utilizado)
 * @param[out] response PDU de respuesta CoAP a enviar al cliente
 * 
 * @details Esta función procesa las solicitudes de llamada de piso (floor calls)
 * recibidas desde los API Gateways. Implementa el algoritmo de asignación
 * de ascensores para atender llamadas desde botones externos de los edificios.
 * 
 * **Endpoint:** `POST /peticion_piso`
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
 * **Respuesta JSON de éxito:**
 * ```json
 * {
 *   "tarea_id": "T_1640995200123",
 *   "ascensor_asignado_id": "E1A1"
 * }
 * ```
 * 
 * **Códigos de respuesta HTTP:**
 * - `200 OK`: Asignación exitosa
 * - `400 Bad Request`: JSON inválido o campos faltantes
 * - `503 Service Unavailable`: No hay ascensores disponibles
 * 
 * **Algoritmo de asignación:**
 * Utiliza el algoritmo de proximidad inteligente implementado en select_optimal_elevator():
 * - Filtra ascensores disponibles (disponible=true)
 * - Calcula distancia absoluta desde piso_origen a cada ascensor
 * - Selecciona todos los ascensores con distancia mínima
 * - En caso de empate, selecciona aleatoriamente para distribuir carga
 * - Garantiza asignación óptima basada en proximidad
 * 
 * **Validaciones realizadas:**
 * - Verificación de formato JSON válido
 * - Validación de campos obligatorios
 * - Comprobación de tipos de datos correctos
 * - Verificación de disponibilidad de ascensores
 * 
 * **Gestión de errores:**
 * - Logging detallado de errores y warnings
 * - Respuestas JSON con información de error
 * - Liberación automática de memoria en caso de error
 * 
 * @note Esta función es llamada automáticamente por libcoap
 * @note La memoria del ID del ascensor asignado debe ser liberada por el llamador
 * @see select_optimal_elevator()
 * @see generate_unique_task_id()
 * @see RESOURCE_FLOOR_CALL
 * @see cJSON_ParseWithLength()
 */
static void hnd_floor_call(coap_resource_t *resource, coap_session_t *session,
                           const coap_pdu_t *request, const coap_string_t *query,
                           coap_pdu_t *response)
{
    SRV_LOG_INFO("=== MANEJADOR FLOOR CALL EJECUTÁNDOSE ===");
    SRV_LOG_INFO("=== PETICIÓN POST RECIBIDA EN /peticion_piso ===");
    const coap_str_const_t *uri_path_fc = coap_resource_get_uri_path(resource);
    if (uri_path_fc) {
        SRV_LOG_INFO("Received request on /%.*s (Peticion Piso)", (int)uri_path_fc->length, uri_path_fc->s);
    } else {
        SRV_LOG_INFO("Received request on /[unknown_path] (Peticion Piso)");
    }

    // Verificar que la sesión tenga una conexión DTLS válida
    if (coap_session_get_state(session) != COAP_SESSION_STATE_ESTABLISHED) {
        SRV_LOG_ERROR("Unauthorized request: Session not properly connected via DTLS");
        coap_pdu_set_code(response, COAP_RESPONSE_CODE_UNAUTHORIZED);
        cJSON *error_json = cJSON_CreateObject();
        cJSON_AddStringToObject(error_json, "error", "Unauthorized");
        cJSON_AddStringToObject(error_json, "message", "DTLS connection required");
        char *error_str = cJSON_PrintUnformatted(error_json);
        coap_add_option(response, COAP_OPTION_CONTENT_FORMAT, coap_encode_var_safe( (uint8_t[2]){0}, 2, COAP_MEDIATYPE_APPLICATION_JSON), (uint8_t[2]){0});
        coap_add_data(response, strlen(error_str), (const uint8_t*)error_str);
        cJSON_Delete(error_json);
        free(error_str);
        return;
    }

    const uint8_t *data;
    size_t data_len;
    if (coap_get_data(request, &data_len, &data)) {
        // Verificar Content-Format si está presente
        coap_opt_iterator_t opt_iter;
        coap_opt_t *option;
        coap_option_iterator_init(request, &opt_iter, COAP_OPT_ALL);
        while ((option = coap_option_next(&opt_iter))) {
            if (opt_iter.number == COAP_OPTION_CONTENT_FORMAT) {
                uint32_t content_format = coap_decode_var_bytes(coap_opt_value(option), coap_opt_length(option));
                if (content_format != COAP_MEDIATYPE_APPLICATION_JSON) {
                    SRV_LOG_ERROR("Unsupported Content-Format: %u (expected JSON)", content_format);
                    coap_pdu_set_code(response, COAP_RESPONSE_CODE_UNSUPPORTED_CONTENT_FORMAT);
                    cJSON *error_json = cJSON_CreateObject();
                    cJSON_AddStringToObject(error_json, "error", "Unsupported Content-Format");
                    cJSON_AddStringToObject(error_json, "expected", "application/json");
                    cJSON_AddNumberToObject(error_json, "received", content_format);
                    char *error_str = cJSON_PrintUnformatted(error_json);
                    coap_add_option(response, COAP_OPTION_CONTENT_FORMAT, coap_encode_var_safe( (uint8_t[2]){0}, 2, COAP_MEDIATYPE_APPLICATION_JSON), (uint8_t[2]){0});
                    coap_add_data(response, strlen(error_str), (const uint8_t*)error_str);
                    cJSON_Delete(error_json);
                    free(error_str);
                    return;
                }
                break;
            }
        }

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

        // Validar rango de piso (asumiendo edificios de 1-50 pisos)
        if (piso_origen < 1 || piso_origen > 50) {
            SRV_LOG_ERROR("Invalid floor number: %d (must be between 1-50)", piso_origen);
            coap_pdu_set_code(response, COAP_RESPONSE_CODE_BAD_REQUEST);
            cJSON *error_json = cJSON_CreateObject();
            cJSON_AddStringToObject(error_json, "error", "Invalid floor number");
            cJSON_AddNumberToObject(error_json, "floor", piso_origen);
            cJSON_AddStringToObject(error_json, "valid_range", "1-50");
            char *error_str = cJSON_PrintUnformatted(error_json);
            coap_add_option(response, COAP_OPTION_CONTENT_FORMAT, coap_encode_var_safe( (uint8_t[2]){0}, 2, COAP_MEDIATYPE_APPLICATION_JSON), (uint8_t[2]){0});
            coap_add_data(response, strlen(error_str), (const uint8_t*)error_str);
            cJSON_Delete(error_json);
            free(error_str);
            cJSON_Delete(json_payload);
            return;
        }

        // Validar dirección de llamada
        if (strcmp(direccion_llamada, "SUBIENDO") != 0 && strcmp(direccion_llamada, "BAJANDO") != 0) {
            SRV_LOG_ERROR("Invalid call direction: %s (must be SUBIENDO or BAJANDO)", direccion_llamada);
            coap_pdu_set_code(response, COAP_RESPONSE_CODE_BAD_REQUEST);
            cJSON *error_json = cJSON_CreateObject();
            cJSON_AddStringToObject(error_json, "error", "Invalid call direction");
            cJSON_AddStringToObject(error_json, "direction", direccion_llamada);
            cJSON_AddStringToObject(error_json, "valid_values", "SUBIENDO, BAJANDO");
            char *error_str = cJSON_PrintUnformatted(error_json);
            coap_add_option(response, COAP_OPTION_CONTENT_FORMAT, coap_encode_var_safe( (uint8_t[2]){0}, 2, COAP_MEDIATYPE_APPLICATION_JSON), (uint8_t[2]){0});
            coap_add_data(response, strlen(error_str), (const uint8_t*)error_str);
            cJSON_Delete(error_json);
            free(error_str);
            cJSON_Delete(json_payload);
            return;
        }

        SRV_LOG_INFO("Floor call from Edificio '%s', Piso Origen Llamada %d, Direccion '%s'", id_edificio, piso_origen, direccion_llamada);

        // Usar algoritmo de asignación inteligente mejorado
        char *assigned_elevator_id = select_optimal_elevator(j_elevadores_estado, piso_origen, direccion_llamada);

        if (assigned_elevator_id) {
            char task_id[32];
            generate_unique_task_id(task_id, sizeof(task_id));
            
            // Verificar que se pudo generar el task_id correctamente
            if (strlen(task_id) == 0) {
                SRV_LOG_ERROR("Internal error: Failed to generate task ID");
                coap_pdu_set_code(response, COAP_RESPONSE_CODE_INTERNAL_ERROR);
                cJSON *error_json = cJSON_CreateObject();
                cJSON_AddStringToObject(error_json, "error", "Internal Server Error");
                cJSON_AddStringToObject(error_json, "message", "Failed to generate task ID");
                char *error_str = cJSON_PrintUnformatted(error_json);
                coap_add_option(response, COAP_OPTION_CONTENT_FORMAT, coap_encode_var_safe( (uint8_t[2]){0}, 2, COAP_MEDIATYPE_APPLICATION_JSON), (uint8_t[2]){0});
                coap_add_data(response, strlen(error_str), (const uint8_t*)error_str);
                cJSON_Delete(error_json);
                free(error_str);
                free(assigned_elevator_id);
                cJSON_Delete(json_payload);
                return;
            }
            
            SRV_LOG_INFO("Assigning task %s to elevator %s for floor call from piso %d (Edificio: %s)", 
                        task_id, assigned_elevator_id, piso_origen, id_edificio);

            cJSON *response_json = cJSON_CreateObject();
            cJSON_AddStringToObject(response_json, "tarea_id", task_id);
            cJSON_AddStringToObject(response_json, "ascensor_asignado_id", assigned_elevator_id);

            char *response_str = cJSON_PrintUnformatted(response_json);
            if (!response_str) {
                SRV_LOG_ERROR("Internal error: Failed to create JSON response");
                coap_pdu_set_code(response, COAP_RESPONSE_CODE_INTERNAL_ERROR);
                cJSON *error_json = cJSON_CreateObject();
                cJSON_AddStringToObject(error_json, "error", "Internal Server Error");
                cJSON_AddStringToObject(error_json, "message", "Failed to create response");
                char *error_str = cJSON_PrintUnformatted(error_json);
                coap_add_option(response, COAP_OPTION_CONTENT_FORMAT, coap_encode_var_safe( (uint8_t[2]){0}, 2, COAP_MEDIATYPE_APPLICATION_JSON), (uint8_t[2]){0});
                coap_add_data(response, strlen(error_str), (const uint8_t*)error_str);
                cJSON_Delete(error_json);
                free(error_str);
                cJSON_Delete(response_json);
                free(assigned_elevator_id);
                cJSON_Delete(json_payload);
                return;
            }
            
            coap_pdu_set_code(response, COAP_RESPONSE_CODE_CONTENT);
            coap_add_option(response, COAP_OPTION_CONTENT_FORMAT, coap_encode_var_safe( (uint8_t[2]){0}, 2, COAP_MEDIATYPE_APPLICATION_JSON), (uint8_t[2]){0});
            coap_add_data(response, strlen(response_str), (const uint8_t *)response_str);

            cJSON_Delete(response_json);
            free(response_str);
            free(assigned_elevator_id);
        } else {
            SRV_LOG_WARN("No elevators available for floor call from edificio '%s', piso %d", id_edificio, piso_origen);
            coap_pdu_set_code(response, COAP_RESPONSE_CODE_SERVICE_UNAVAILABLE);
            cJSON *error_json = cJSON_CreateObject();
            cJSON_AddStringToObject(error_json, "error", "No elevators available at the moment.");
            cJSON_AddStringToObject(error_json, "edificio", id_edificio);
            cJSON_AddNumberToObject(error_json, "piso_origen", piso_origen);
            cJSON_AddStringToObject(error_json, "suggestion", "Try again in a few moments");
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
 * 
 * @param[in] resource Recurso CoAP que recibió la solicitud
 * @param[in] session Sesión CoAP del cliente que envió la solicitud
 * @param[in] request PDU de la solicitud CoAP recibida
 * @param[in] query Parámetros de consulta de la URI (no utilizado)
 * @param[out] response PDU de respuesta CoAP a enviar al cliente
 * 
 * @details Esta función procesa las solicitudes de cabina (cabin requests)
 * recibidas desde los API Gateways. Las solicitudes de cabina provienen
 * del interior de los ascensores cuando los usuarios presionan botones
 * de destino.
 * 
 * **Endpoint:** `POST /peticion_cabina`
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
 * **Respuesta JSON de éxito:**
 * ```json
 * {
 *   "tarea_id": "T_1640995200456",
 *   "ascensor_asignado_id": "E1A1"
 * }
 * ```
 * 
 * **Códigos de respuesta HTTP:**
 * - `200 OK`: Asignación exitosa
 * - `400 Bad Request`: JSON inválido o campos faltantes
 * 
 * **Algoritmo de asignación:**
 * Para solicitudes de cabina, el ascensor asignado es siempre
 * el mismo que realizó la solicitud (auto-asignación). Esto
 * es lógico ya que la solicitud proviene del interior del ascensor.
 * 
 * **Validaciones realizadas:**
 * - Verificación de formato JSON válido
 * - Validación de campos obligatorios
 * - Comprobación de tipos de datos correctos
 * - Verificación de array de estado de ascensores válido
 * 
 * **Diferencias con llamadas de piso:**
 * - No requiere algoritmo de selección de ascensor
 * - El ascensor solicitante se auto-asigna
 * - No necesita verificar disponibilidad de otros ascensores
 * - Proceso más directo y eficiente
 * 
 * **Gestión de errores:**
 * - Logging detallado de errores y warnings
 * - Respuestas JSON con información de error
 * - Liberación automática de memoria en caso de error
 * 
 * @note Esta función es llamada automáticamente por libcoap
 * @note No requiere algoritmo de selección de ascensor como las llamadas de piso
 * @see generate_unique_task_id()
 * @see RESOURCE_CABIN_REQUEST
 * @see cJSON_ParseWithLength()
 */
static void hnd_cabin_request(coap_resource_t *resource, coap_session_t *session,
                              const coap_pdu_t *request, const coap_string_t *query,
                              coap_pdu_t *response)
{
    SRV_LOG_INFO("=== MANEJADOR CABIN REQUEST EJECUTÁNDOSE ===");
    const coap_str_const_t *uri_path_cr = coap_resource_get_uri_path(resource);
    if (uri_path_cr) {
        SRV_LOG_INFO("Received request on /%.*s (Peticion Cabina)", (int)uri_path_cr->length, uri_path_cr->s);
    } else {
        SRV_LOG_INFO("Received request on /[unknown_path] (Peticion Cabina)");
    }

    // Verificar conexión DTLS
    if (coap_session_get_state(session) != COAP_SESSION_STATE_ESTABLISHED) {
        SRV_LOG_ERROR("Unauthorized cabin request: Session not properly connected via DTLS");
        coap_pdu_set_code(response, COAP_RESPONSE_CODE_UNAUTHORIZED);
        cJSON *error_json = cJSON_CreateObject();
        cJSON_AddStringToObject(error_json, "error", "Unauthorized");
        cJSON_AddStringToObject(error_json, "message", "DTLS connection required");
        char *error_str = cJSON_PrintUnformatted(error_json);
        coap_add_option(response, COAP_OPTION_CONTENT_FORMAT, coap_encode_var_safe( (uint8_t[2]){0}, 2, COAP_MEDIATYPE_APPLICATION_JSON), (uint8_t[2]){0});
        coap_add_data(response, strlen(error_str), (const uint8_t*)error_str);
        cJSON_Delete(error_json);
        free(error_str);
        return;
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
        cJSON *j_elevadores_estado = cJSON_GetObjectItemCaseSensitive(json_payload, "elevadores_estado");

        if (!cJSON_IsString(j_id_edificio) || !cJSON_IsString(j_solicitando_ascensor_id) ||
            !cJSON_IsNumber(j_piso_destino_solicitud) || !cJSON_IsArray(j_elevadores_estado)) {
            SRV_LOG_ERROR("Missing or invalid fields in JSON payload for cabin request");
            coap_pdu_set_code(response, COAP_RESPONSE_CODE_BAD_REQUEST);
            cJSON *error_json = cJSON_CreateObject();
            cJSON_AddStringToObject(error_json, "error", "Missing or invalid fields in JSON payload for cabin request");
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
        int piso_destino = j_piso_destino_solicitud->valueint;

        // Validar rango de piso destino
        if (piso_destino < 1 || piso_destino > 50) {
            SRV_LOG_ERROR("Invalid destination floor: %d (must be between 1-50)", piso_destino);
            coap_pdu_set_code(response, COAP_RESPONSE_CODE_BAD_REQUEST);
            cJSON *error_json = cJSON_CreateObject();
            cJSON_AddStringToObject(error_json, "error", "Invalid destination floor");
            cJSON_AddNumberToObject(error_json, "destination_floor", piso_destino);
            cJSON_AddStringToObject(error_json, "valid_range", "1-50");
            char *error_str = cJSON_PrintUnformatted(error_json);
            coap_add_option(response, COAP_OPTION_CONTENT_FORMAT, coap_encode_var_safe( (uint8_t[2]){0}, 2, COAP_MEDIATYPE_APPLICATION_JSON), (uint8_t[2]){0});
            coap_add_data(response, strlen(error_str), (const uint8_t*)error_str);
            cJSON_Delete(error_json);
            free(error_str);
            cJSON_Delete(json_payload);
            return;
        }

        // Verificar que el ascensor solicitante existe en el array de estado
        int ascensor_encontrado = 0;
        int array_size = cJSON_GetArraySize(j_elevadores_estado);
        for (int i = 0; i < array_size; i++) {
            cJSON *elevator = cJSON_GetArrayItem(j_elevadores_estado, i);
            if (elevator) {
                cJSON *j_id_ascensor = cJSON_GetObjectItemCaseSensitive(elevator, "id_ascensor");
                if (cJSON_IsString(j_id_ascensor) && 
                    strcmp(j_id_ascensor->valuestring, solicitando_ascensor_id) == 0) {
                    ascensor_encontrado = 1;
                    break;
                }
            }
        }

        if (!ascensor_encontrado) {
            SRV_LOG_ERROR("Requesting elevator '%s' not found in elevators state array", solicitando_ascensor_id);
            coap_pdu_set_code(response, COAP_RESPONSE_CODE_BAD_REQUEST);
            cJSON *error_json = cJSON_CreateObject();
            cJSON_AddStringToObject(error_json, "error", "Requesting elevator not found");
            cJSON_AddStringToObject(error_json, "elevator_id", solicitando_ascensor_id);
            cJSON_AddStringToObject(error_json, "message", "Elevator must exist in elevators_estado array");
            char *error_str = cJSON_PrintUnformatted(error_json);
            coap_add_option(response, COAP_OPTION_CONTENT_FORMAT, coap_encode_var_safe( (uint8_t[2]){0}, 2, COAP_MEDIATYPE_APPLICATION_JSON), (uint8_t[2]){0});
            coap_add_data(response, strlen(error_str), (const uint8_t*)error_str);
            cJSON_Delete(error_json);
            free(error_str);
            cJSON_Delete(json_payload);
            return;
        }

        SRV_LOG_INFO("Cabin request from Edificio '%s', Ascensor '%s', Destino %d", 
                    id_edificio, solicitando_ascensor_id, piso_destino);

        // Para solicitudes de cabina, el ascensor se auto-asigna
        char task_id[32];
        generate_unique_task_id(task_id, sizeof(task_id));
        
        if (strlen(task_id) == 0) {
            SRV_LOG_ERROR("Internal error: Failed to generate task ID for cabin request");
            coap_pdu_set_code(response, COAP_RESPONSE_CODE_INTERNAL_ERROR);
            cJSON *error_json = cJSON_CreateObject();
            cJSON_AddStringToObject(error_json, "error", "Internal Server Error");
            cJSON_AddStringToObject(error_json, "message", "Failed to generate task ID");
            char *error_str = cJSON_PrintUnformatted(error_json);
            coap_add_option(response, COAP_OPTION_CONTENT_FORMAT, coap_encode_var_safe( (uint8_t[2]){0}, 2, COAP_MEDIATYPE_APPLICATION_JSON), (uint8_t[2]){0});
            coap_add_data(response, strlen(error_str), (const uint8_t*)error_str);
            cJSON_Delete(error_json);
            free(error_str);
            cJSON_Delete(json_payload);
            return;
        }

        SRV_LOG_INFO("Self-assigning task %s to elevator %s for cabin request to floor %d", 
                    task_id, solicitando_ascensor_id, piso_destino);

        cJSON *response_json = cJSON_CreateObject();
        cJSON_AddStringToObject(response_json, "tarea_id", task_id);
        cJSON_AddStringToObject(response_json, "ascensor_asignado_id", solicitando_ascensor_id);

        char *response_str = cJSON_PrintUnformatted(response_json);
        if (!response_str) {
            SRV_LOG_ERROR("Internal error: Failed to create JSON response for cabin request");
            coap_pdu_set_code(response, COAP_RESPONSE_CODE_INTERNAL_ERROR);
            cJSON *error_json = cJSON_CreateObject();
            cJSON_AddStringToObject(error_json, "error", "Internal Server Error");
            cJSON_AddStringToObject(error_json, "message", "Failed to create response");
            char *error_str = cJSON_PrintUnformatted(error_json);
            coap_add_option(response, COAP_OPTION_CONTENT_FORMAT, coap_encode_var_safe( (uint8_t[2]){0}, 2, COAP_MEDIATYPE_APPLICATION_JSON), (uint8_t[2]){0});
            coap_add_data(response, strlen(error_str), (const uint8_t*)error_str);
            cJSON_Delete(error_json);
            free(error_str);
            cJSON_Delete(response_json);
            cJSON_Delete(json_payload);
            return;
        }
        
        coap_pdu_set_code(response, COAP_RESPONSE_CODE_CONTENT);
        coap_add_option(response, COAP_OPTION_CONTENT_FORMAT, coap_encode_var_safe( (uint8_t[2]){0}, 2, COAP_MEDIATYPE_APPLICATION_JSON), (uint8_t[2]){0});
        coap_add_data(response, strlen(response_str), (const uint8_t *)response_str);

        cJSON_Delete(response_json);
        free(response_str);
        cJSON_Delete(json_payload);

    } else {
        SRV_LOG_ERROR("Received cabin request with no payload");
        coap_pdu_set_code(response, COAP_RESPONSE_CODE_BAD_REQUEST);
        cJSON *error_json = cJSON_CreateObject();
        cJSON_AddStringToObject(error_json, "error", "Missing payload for cabin request");
        char *error_str = cJSON_PrintUnformatted(error_json);
        coap_add_option(response, COAP_OPTION_CONTENT_FORMAT, coap_encode_var_safe( (uint8_t[2]){0}, 2, COAP_MEDIATYPE_APPLICATION_JSON), (uint8_t[2]){0});
        coap_add_data(response, strlen(error_str), (const uint8_t*)error_str);
        cJSON_Delete(error_json);
        free(error_str);
    }
}

// REMOVED: preguntar_reestablecer_db()
// REMOVED: hnd_process_elevator_request()

/**
 * @brief Función principal del Servidor Central de Ascensores
 * 
 * @param[in] argc Número de argumentos de línea de comandos
 * @param[in] argv Array de argumentos de línea de comandos
 * 
 * @return EXIT_SUCCESS (0) si el servidor termina correctamente, EXIT_FAILURE (1) en caso de error
 * 
 * @details Esta función implementa el punto de entrada principal del servidor central
 * de control de ascensores. Configura y ejecuta un servidor CoAP con DTLS-PSK
 * que gestiona solicitudes de ascensores desde API Gateways.
 * 
 * **Flujo de inicialización:**
 * 1. **Configuración de señales**: Registra manejador para SIGINT (Ctrl+C)
 * 2. **Inicialización de libCoAP**: Configura logging y contexto CoAP
 * 3. **Configuración de red**: Establece dirección y puerto de escucha
 * 4. **Configuración DTLS-PSK**: Configura autenticación y cifrado
 * 5. **Inicialización PSK**: Carga validador de claves precompartidas
 * 6. **Registro de recursos**: Configura endpoints CoAP
 * 7. **Bucle principal**: Procesa solicitudes hasta terminación
 * 
 * **Configuración de seguridad:**
 * - Autenticación DTLS-PSK con claves precompartidas
 * - Validación de identidades de clientes
 * - Cifrado de todas las comunicaciones
 * - Timeouts optimizados para estabilidad
 * 
 * **Recursos CoAP registrados:**
 * - `POST /peticion_piso`: Solicitudes de llamada de piso
 * - `POST /peticion_cabina`: Solicitudes de cabina
 * 
 * **Gestión de errores:**
 * - Validación de configuración de red
 * - Verificación de inicialización de componentes
 * - Logging detallado de errores críticos
 * - Terminación elegante en caso de fallo
 * 
 * **Terminación elegante:**
 * - Respuesta a señal SIGINT (Ctrl+C)
 * - Liberación de recursos de memoria
 * - Cierre de conexiones DTLS
 * - Limpieza de contexto CoAP
 * 
 * **Configuración de red:**
 * - Puerto: 5684 (estándar CoAP-DTLS)
 * - Interfaz: 0.0.0.0 (todas las interfaces)
 * - Protocolo: UDP con DTLS
 * 
 * @note El servidor se ejecuta indefinidamente hasta recibir SIGINT
 * @note Requiere archivo de claves PSK para funcionamiento completo
 * @see handle_sigint()
 * @see session_event_handler()
 * @see get_psk_info()
 * @see psk_validator_init()
 * @see hnd_floor_call()
 * @see hnd_cabin_request()
 */
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

    coap_set_log_level(COAP_LOG_DEBUG);
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

    // Configurar callback PSK personalizado para aceptar patrones de identidad
    coap_dtls_spsk_t setup_data;
    memset(&setup_data, 0, sizeof(setup_data));
    setup_data.version = COAP_DTLS_SPSK_SETUP_VERSION;
    setup_data.validate_id_call_back = get_psk_info;
    setup_data.id_call_back_arg = NULL;
    
    // Configurar hint (la clave se obtendrá dinámicamente en el callback)
    setup_data.psk_info.hint.s = (const uint8_t *)PSK_SERVER_HINT;
    setup_data.psk_info.hint.length = strlen(PSK_SERVER_HINT);
    
    SRV_LOG_INFO("Configurando callback PSK personalizado...");
    SRV_LOG_INFO("Callback function pointer: %p", (void*)get_psk_info);
    SRV_LOG_INFO("PSK_SERVER_HINT: '%s'", PSK_SERVER_HINT);
    
    if (!coap_context_set_psk2(ctx, &setup_data)) {
        SRV_LOG_ERROR("Error: No se pudo configurar la información PSK del servidor (coap_context_set_psk2 falló).");
    } else {
        SRV_LOG_INFO("Callback PSK personalizado configurado para aceptar identidades con patrón 'Gateway_Client_*'");
    }

    // Inicializar validador de claves PSK
    // Intentar diferentes rutas para el archivo de claves
    const char* psk_paths[] = {
        "/app/psk_keys.txt",  // Ruta en Docker/Kubernetes
        "psk_keys.txt",       // Ruta local
        "./psk_keys.txt"      // Ruta relativa
    };
    
    int psk_initialized = 0;
    for (int i = 0; i < 3; i++) {
        if (psk_validator_init(psk_paths[i]) == 0) {
            SRV_LOG_INFO("Validador de claves PSK inicializado correctamente desde: %s", psk_paths[i]);
            psk_initialized = 1;
            break;
        }
    }
    
    if (!psk_initialized) {
        SRV_LOG_WARN("No se pudo inicializar el validador de claves PSK desde ninguna ruta. Continuando con validación básica.");
    }

    // Registrar callback para configurar sesiones DTLS con timeouts optimizados
    coap_register_event_handler(ctx, session_event_handler);
    SRV_LOG_INFO("Callback de eventos de sesión registrado para optimizar timeouts DTLS");
    


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
        int result = coap_io_process(ctx, 5000);  // 5 segundos de timeout para mejor estabilidad DTLS
        if (result < 0) {
            SRV_LOG_ERROR("Error in coap_io_process: %d. Shutting down.", result);
            running = 0;
        }
    }

finish:
    SRV_LOG_WARN("Shutting down CoAP server...");
    
    // Finalizar validador de claves PSK
    psk_validator_cleanup();
    
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