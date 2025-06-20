/**
 * @file execution_logger.h
 * @brief Sistema de Logging de Ejecuciones para Resultados de TFG
 * @author Sistema de Control de Ascensores
 * @date 2025
 * @version 1.0
 * 
 * Este módulo implementa un sistema de logging avanzado que genera
 * archivos Markdown con el flujo completo de comunicación del API Gateway.
 * 
 * **Características:**
 * - Archivos organizados por fecha (carpetas por día)
 * - Formato Markdown para visualización bonita
 * - Captura de eventos CoAP, CAN y simulación
 * - Estadísticas automáticas de rendimiento
 * - Timestamps precisos para cada evento
 * 
 * **Estructura de archivos:**
 * ```
 * api_gateway/logs/YYYY-MM-DD/ejecucion_HH-MM-SS.md
 * ```
 */

#ifndef EXECUTION_LOGGER_H
#define EXECUTION_LOGGER_H

#include <stdio.h>
#include <stdbool.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

// ============================================================================
// CONSTANTES Y CONFIGURACIÓN
// ============================================================================

#define MAX_LOG_PATH 512        ///< Longitud máxima de rutas de archivos
#define MAX_LOG_MESSAGE 1024    ///< Longitud máxima de mensajes de log
#define MAX_EVENT_DESCRIPTION 256 ///< Longitud máxima de descripciones de eventos

// ============================================================================
// TIPOS DE EVENTOS
// ============================================================================

/**
 * @brief Tipos de eventos que se pueden registrar
 */
typedef enum {
    LOG_EVENT_SYSTEM_START,     ///< Inicio del sistema
    LOG_EVENT_SYSTEM_END,       ///< Fin del sistema
    LOG_EVENT_SIMULATION_START, ///< Inicio de simulación
    LOG_EVENT_SIMULATION_END,   ///< Fin de simulación
    LOG_EVENT_BUILDING_SELECTED,///< Edificio seleccionado para simulación
    LOG_EVENT_CAN_SENT,         ///< Frame CAN enviado
    LOG_EVENT_CAN_RECEIVED,     ///< Frame CAN recibido
    LOG_EVENT_COAP_SENT,        ///< Mensaje CoAP enviado
    LOG_EVENT_COAP_RECEIVED,    ///< Mensaje CoAP recibido
    LOG_EVENT_TASK_ASSIGNED,    ///< Tarea asignada a ascensor
    LOG_EVENT_ELEVATOR_MOVED,   ///< Ascensor se movió
    LOG_EVENT_TASK_COMPLETED,   ///< Tarea completada
    LOG_EVENT_ERROR             ///< Error del sistema
} log_event_type_t;

/**
 * @brief Estructura de un evento de log
 */
typedef struct {
    log_event_type_t type;                          ///< Tipo de evento
    time_t timestamp;                               ///< Timestamp del evento
    char description[MAX_EVENT_DESCRIPTION];       ///< Descripción del evento
    char details[MAX_LOG_MESSAGE];                 ///< Detalles adicionales
} log_event_t;

/**
 * @brief Estadísticas de ejecución
 */
typedef struct {
    int total_can_frames_sent;      ///< Total de frames CAN enviados
    int total_can_frames_received;  ///< Total de frames CAN recibidos
    int total_coap_requests;        ///< Total de peticiones CoAP
    int total_coap_responses;       ///< Total de respuestas CoAP
    int total_tasks_assigned;       ///< Total de tareas asignadas
    int total_tasks_completed;      ///< Total de tareas completadas
    int total_elevator_movements;   ///< Total de movimientos de ascensores
    int total_errors;               ///< Total de errores
    double execution_duration_sec;  ///< Duración total en segundos
    char building_id[32];           ///< ID del edificio simulado
    int building_requests;          ///< Número de peticiones del edificio
} execution_stats_t;

// ============================================================================
// FUNCIONES PÚBLICAS
// ============================================================================

/**
 * @brief Inicializa el sistema de logging de ejecuciones
 * @return true si se inicializó correctamente, false en caso de error
 * 
 * Esta función:
 * - Crea las carpetas necesarias por fecha
 * - Genera el nombre del archivo de log
 * - Inicializa las estructuras de estadísticas
 * - Escribe el header del archivo Markdown
 */
bool exec_logger_init(void);

/**
 * @brief Finaliza el sistema de logging y cierra el archivo
 * 
 * Esta función:
 * - Calcula estadísticas finales
 * - Escribe el resumen de ejecución
 * - Cierra el archivo de log
 * - Limpia recursos
 */
void exec_logger_finish(void);

/**
 * @brief Registra un evento en el log
 * @param type Tipo de evento
 * @param description Descripción breve del evento
 * @param details Detalles adicionales (puede ser NULL)
 */
void exec_logger_log_event(log_event_type_t type, const char* description, const char* details);

/**
 * @brief Registra el inicio de la simulación
 * @param building_id ID del edificio seleccionado
 * @param num_requests Número de peticiones a ejecutar
 */
void exec_logger_log_simulation_start(const char* building_id, int num_requests);

/**
 * @brief Registra el fin de la simulación
 * @param successful_requests Número de peticiones exitosas
 * @param total_requests Número total de peticiones
 */
void exec_logger_log_simulation_end(int successful_requests, int total_requests);

/**
 * @brief Registra un frame CAN enviado
 * @param can_id ID del frame CAN
 * @param dlc Longitud de datos
 * @param data Datos del frame
 * @param description Descripción del evento
 */
void exec_logger_log_can_sent(unsigned int can_id, int dlc, const unsigned char* data, const char* description);

/**
 * @brief Registra un frame CAN recibido
 * @param can_id ID del frame CAN
 * @param dlc Longitud de datos
 * @param data Datos del frame
 * @param description Descripción del evento
 */
void exec_logger_log_can_received(unsigned int can_id, int dlc, const unsigned char* data, const char* description);

/**
 * @brief Registra una petición CoAP enviada
 * @param method Método CoAP (GET, POST, etc.)
 * @param uri URI del recurso
 * @param payload Payload de la petición (puede ser NULL)
 */
void exec_logger_log_coap_sent(const char* method, const char* uri, const char* payload);

/**
 * @brief Registra una respuesta CoAP recibida
 * @param code Código de respuesta CoAP
 * @param payload Payload de la respuesta (puede ser NULL)
 */
void exec_logger_log_coap_received(const char* code, const char* payload);

/**
 * @brief Registra la asignación de una tarea
 * @param task_id ID de la tarea
 * @param elevator_id ID del ascensor asignado
 * @param target_floor Piso destino
 */
void exec_logger_log_task_assigned(const char* task_id, const char* elevator_id, int target_floor);

/**
 * @brief Registra el movimiento de un ascensor
 * @param elevator_id ID del ascensor
 * @param from_floor Piso origen
 * @param to_floor Piso destino
 * @param direction Dirección del movimiento
 */
void exec_logger_log_elevator_moved(const char* elevator_id, int from_floor, int to_floor, const char* direction);

/**
 * @brief Registra la completación de una tarea
 * @param task_id ID de la tarea completada
 * @param elevator_id ID del ascensor que completó la tarea
 * @param final_floor Piso final donde se completó
 */
void exec_logger_log_task_completed(const char* task_id, const char* elevator_id, int final_floor);

/**
 * @brief Registra un error del sistema
 * @param error_code Código de error
 * @param error_message Mensaje de error
 */
void exec_logger_log_error(const char* error_code, const char* error_message);

/**
 * @brief Obtiene las estadísticas actuales de ejecución
 * @return Puntero a las estadísticas (solo lectura)
 */
const execution_stats_t* exec_logger_get_stats(void);

/**
 * @brief Verifica si el logger está activo
 * @return true si está activo, false si no
 */
bool exec_logger_is_active(void);

#ifdef __cplusplus
}
#endif

#endif // EXECUTION_LOGGER_H 