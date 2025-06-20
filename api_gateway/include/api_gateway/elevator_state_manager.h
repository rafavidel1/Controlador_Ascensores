/**
 * @file elevator_state_manager.h
 * @brief Gestor de Estado de Ascensores para el API Gateway
 * @author Sistema de Control de Ascensores
 * @date 2025
 * @version 2.0
 * 
 * Este header define las estructuras de datos y funciones para la gestión
 * del estado de ascensores en el API Gateway. Proporciona:
 * 
 * - **Estructuras de estado**: Definiciones para ascensores individuales y grupos
 * - **Enumeraciones**: Estados de puertas, direcciones de movimiento, tipos de solicitud
 * - **Funciones de gestión**: Inicialización, asignación de tareas, notificaciones
 * - **Serialización JSON**: Conversión de estado a formato para servidor central
 * - **Utilidades**: Funciones helper para conversión de enums a strings
 * 
 * El gestor mantiene información completa de cada ascensor incluyendo:
 * - Identificación única y edificio
 * - Posición actual y destino
 * - Estado de puertas y dirección de movimiento
 * - Tarea asignada y estado de ocupación
 * 
 * @see elevator_state_manager.c
 * @see api_handlers.h
 * @see api_common_defs.h
 */

#ifndef ELEVATOR_STATE_MANAGER_H
#define ELEVATOR_STATE_MANAGER_H

#include <cJSON.h>
#include <stdbool.h> // Para bool

/**
 * @brief Número máximo de ascensores por gateway
 * 
 * Define el límite máximo de ascensores que puede gestionar
 * un solo API Gateway. Ajustar según los requisitos del sistema.
 */
#define MAX_ELEVATORS_PER_GATEWAY 6

/**
 * @brief Longitud máxima para strings de identificación
 * 
 * Define el tamaño máximo para IDs de ascensores, edificios
 * y otros identificadores del sistema.
 */
#define ID_STRING_MAX_LEN 25

/**
 * @brief Longitud máxima para IDs de tareas
 * 
 * Define el tamaño máximo para identificadores de tareas
 * asignadas por el servidor central.
 */
#define TASK_ID_MAX_LEN 32

/**
 * @brief Longitud máxima para strings de estado
 * 
 * Define el tamaño máximo para strings que representan
 * estados de puertas, direcciones, etc.
 */
#define STATUS_STRING_MAX_LEN 16

/**
 * @brief Enumeración de estados de puertas de ascensor
 * 
 * Define los posibles estados de las puertas de un ascensor
 * para control y monitoreo del sistema.
 */
typedef enum {
    DOOR_CLOSED,    ///< Puertas completamente cerradas
    DOOR_OPEN,      ///< Puertas completamente abiertas
    DOOR_OPENING,   ///< Puertas en proceso de apertura
    DOOR_CLOSING,   ///< Puertas en proceso de cierre
    DOOR_UNKNOWN    ///< Estado de puertas desconocido
} door_state_enum_t;

/**
 * @brief Enumeración de direcciones de movimiento de ascensor
 * 
 * Define las posibles direcciones de movimiento de un ascensor
 * para control de tráfico y optimización de rutas.
 */
typedef enum {
    MOVING_UP,        ///< Ascensor moviéndose hacia arriba
    MOVING_DOWN,      ///< Ascensor moviéndose hacia abajo
    STOPPED,          ///< Ascensor detenido
    DIRECTION_UNKNOWN ///< Dirección de movimiento desconocida
} movement_direction_enum_t;

/**
 * @brief Enumeración de tipos de solicitud del gateway
 * 
 * Define los tipos de solicitudes que el gateway puede procesar
 * y reenviar al servidor central. Utilizado para serialización JSON
 * y gestión de trackers.
 */
typedef enum {
    GW_REQUEST_TYPE_UNKNOWN = 0,    ///< Tipo de solicitud desconocido
    GW_REQUEST_TYPE_FLOOR_CALL,     ///< Llamada de piso (botón externo)
    GW_REQUEST_TYPE_CABIN_REQUEST   ///< Solicitud de cabina (botón interno)
} gw_request_type_t;

/**
 * @brief Estado de un ascensor individual
 * 
 * Estructura que contiene toda la información de estado
 * de un ascensor específico gestionado por el gateway.
 */
typedef struct {
    char ascensor_id[ID_STRING_MAX_LEN];                ///< ID único del ascensor (ej: "E1A1")
    char id_edificio_str[ID_STRING_MAX_LEN];           ///< ID del edificio (ej: "E1")
    int piso_actual;                                    ///< Piso actual del ascensor
    door_state_enum_t estado_puerta_enum;              ///< Estado actual de las puertas
    char tarea_actual_id[TASK_ID_MAX_LEN];             ///< ID de la tarea asignada por el servidor central
    int destino_actual;                                 ///< Piso objetivo de la tarea actual (-1 si no hay)
    movement_direction_enum_t direccion_movimiento_enum; ///< Dirección de movimiento actual
    bool ocupado;                                       ///< True si el ascensor está asignado a una tarea
} elevator_status_t;

/**
 * @brief Estado del grupo de ascensores gestionado por el gateway
 * 
 * Estructura que contiene el estado completo de todos los ascensores
 * de un edificio gestionados por un API Gateway específico.
 */
typedef struct {
    elevator_status_t ascensores[MAX_ELEVATORS_PER_GATEWAY]; ///< Array de ascensores del grupo
    int num_elevadores_en_grupo;                             ///< Número de ascensores en el grupo
    char edificio_id_str_grupo[ID_STRING_MAX_LEN];          ///< ID del edificio gestionado
} elevator_group_state_t;

/**
 * @brief Detalles específicos de solicitud para serialización JSON
 * 
 * Estructura que contiene información específica del tipo de solicitud
 * para incluir en el payload JSON enviado al servidor central.
 */
typedef struct {
    // Para Floor Call (GW_REQUEST_TYPE_FLOOR_CALL)
    int origin_floor_fc;                                ///< Piso origen de la llamada
    movement_direction_enum_t direction_fc;             ///< Dirección solicitada

    // Para Cabin Request (GW_REQUEST_TYPE_CABIN_REQUEST)
    char requesting_elevator_id_cr[ID_STRING_MAX_LEN];  ///< ID del ascensor solicitante
    int target_floor_cr;                                ///< Piso destino solicitado

    // Otros tipos de solicitud pueden añadir sus campos aquí si es necesario.
} api_request_details_for_json_t;

// --- Prototipos de Funciones ---

/**
 * @brief Inicializa el estado de un grupo de ascensores
 * @param group Puntero al grupo de ascensores a inicializar
 * @param edificio_id_str El ID del edificio (ej: "E1")
 * @param num_elevadores El número de ascensores en este grupo
 * @param num_pisos El número total de pisos en el edificio
 * 
 * Esta función inicializa completamente un grupo de ascensores con
 * la configuración especificada, estableciendo IDs únicos, posiciones
 * iniciales y estados por defecto para todos los ascensores.
 * 
 * @see elevator_group_state_t
 * @see elevator_status_t
 */
void init_elevator_group(elevator_group_state_t *group, const char* edificio_id_str, int num_elevadores, int num_pisos);

/**
 * @brief Serializa el estado del grupo de ascensores a JSON para el servidor central
 * @param group Puntero al estado del grupo de ascensores
 * @param request_type El tipo de la solicitud original que motiva este payload
 * @param details Puntero a estructura con detalles específicos de la solicitud (puede ser NULL)
 * @return Puntero a objeto cJSON que representa el payload, o NULL en caso de error
 * 
 * Esta función convierte el estado completo del grupo de ascensores y los
 * detalles de la solicitud específica a un objeto JSON formateado como
 * espera el servidor central para procesamiento de asignaciones.
 * 
 * El llamador es responsable de liberar el objeto cJSON con cJSON_Delete().
 * 
 * @see gw_request_type_t
 * @see api_request_details_for_json_t
 */
cJSON* elevator_group_to_json_for_server(const elevator_group_state_t *group, 
                                           gw_request_type_t request_type, 
                                           const api_request_details_for_json_t* details);

/**
 * @brief Actualiza el estado de un ascensor tras recibir asignación de tarea
 * @param group Puntero al grupo de ascensores
 * @param elevator_id_to_update ID del ascensor a actualizar
 * @param task_id El nuevo ID de tarea asignado
 * @param target_floor El piso destino para esta tarea
 * @param current_request_floor El piso donde se originó la solicitud
 * 
 * Esta función actualiza el estado de un ascensor específico después de
 * recibir una asignación de tarea del servidor central. Establece la tarea,
 * destino, dirección de movimiento y marca el ascensor como ocupado.
 * 
 * @see elevator_status_t
 */
void assign_task_to_elevator(elevator_group_state_t *group, const char* elevator_id_to_update, const char* task_id, int target_floor, int current_request_floor);

/**
 * @brief Convierte un estado de puerta a su representación en string
 * @param state Estado de puerta a convertir
 * @return String representando el estado de la puerta
 * 
 * Función helper para convertir enumeraciones de estado de puerta
 * a strings legibles para JSON y logging.
 * 
 * @see door_state_enum_t
 */
const char* door_state_to_string(door_state_enum_t state);

/**
 * @brief Convierte una dirección de movimiento a su representación en string
 * @param direction Dirección de movimiento a convertir
 * @return String representando la dirección de movimiento
 * 
 * Función helper para convertir enumeraciones de dirección de movimiento
 * a strings legibles para JSON y logging.
 * 
 * @see movement_direction_enum_t
 */
const char* movement_direction_to_string(movement_direction_enum_t direction);

#endif // ELEVATOR_STATE_MANAGER_H 