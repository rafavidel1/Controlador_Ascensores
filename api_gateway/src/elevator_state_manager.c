/**
 * @file elevator_state_manager.c
 * @brief Implementación del Gestor de Estado de Ascensores del API Gateway
 * @author Sistema de Control de Ascensores
 * @date 2025
 * @version 2.0
 * 
 * Este archivo implementa la gestión completa del estado de ascensores
 * en el API Gateway, proporcionando funcionalidades para:
 * 
 * - **Inicialización de grupos**: Configuración inicial de grupos de ascensores
 * - **Gestión de estado**: Mantenimiento del estado actual de cada ascensor
 * - **Asignación de tareas**: Asignación de tareas a ascensores específicos
 * - **Notificaciones de llegada**: Procesamiento de llegadas a destinos
 * - **Serialización JSON**: Conversión del estado a formato JSON para servidor central
 * - **Utilidades de conversión**: Funciones helper para strings de estado
 * 
 * El gestor mantiene información completa de cada ascensor incluyendo:
 * - Posición actual (piso)
 * - Estado de puertas (abierta/cerrada/abriendo/cerrando)
 * - Dirección de movimiento (subiendo/bajando/parado)
 * - Tarea actual asignada
 * - Destino actual
 * - Estado de ocupación
 * 
 * @see elevator_state_manager.h
 * @see api_common_defs.h
 * @see api_handlers.h
 */

#include "api_gateway/elevator_state_manager.h"
#include "api_gateway/api_common_defs.h" // For LOG_INFO_GW etc.
#include "api_gateway/execution_logger.h" // Sistema de logging de ejecuciones
#include <string.h> // For strcpy, memset, snprintf
#include <stdio.h> // For snprintf

// --- Implementación de Funciones ---

/**
 * @brief Convierte un estado de puerta a su representación en string
 * @param state Estado de puerta a convertir
 * @return String representando el estado de la puerta
 * 
 * Esta función proporciona una representación legible del estado
 * de las puertas del ascensor para logging y serialización JSON.
 * 
 * Estados soportados:
 * - DOOR_CLOSED: "CERRADA"
 * - DOOR_OPEN: "ABIERTA"  
 * - DOOR_OPENING: "ABRIENDO"
 * - DOOR_CLOSING: "CERRANDO"
 * - Otros: "DESCONOCIDO"
 */
const char* door_state_to_string(door_state_enum_t state) {
    switch (state) {
        case DOOR_CLOSED: return "CERRADA";
        case DOOR_OPEN: return "ABIERTA";
        case DOOR_OPENING: return "ABRIENDO";
        case DOOR_CLOSING: return "CERRANDO";
        default: return "DESCONOCIDO";
    }
}

/**
 * @brief Convierte una dirección de movimiento a su representación en string
 * @param direction Dirección de movimiento a convertir
 * @return String representando la dirección de movimiento
 * 
 * Esta función proporciona una representación legible de la dirección
 * de movimiento del ascensor para logging y serialización JSON.
 * 
 * Direcciones soportadas:
 * - MOVING_UP: "SUBIENDO"
 * - MOVING_DOWN: "BAJANDO"
 * - STOPPED: "PARADO"
 * - Otros: "DESCONOCIDO"
 */
const char* movement_direction_to_string(movement_direction_enum_t direction) {
    switch (direction) {
        case MOVING_UP: return "SUBIENDO";
        case MOVING_DOWN: return "BAJANDO";
        case STOPPED: return "PARADO";
        default: return "DESCONOCIDO";
    }
}

/**
 * @brief Inicializa un grupo de ascensores con configuración específica
 * @param group Puntero al grupo de ascensores a inicializar
 * @param edificio_id_str ID del edificio al que pertenece el grupo
 * @param num_elevadores Número de ascensores en el grupo
 * @param num_pisos Número de pisos del edificio
 * 
 * Esta función inicializa completamente un grupo de ascensores con
 * la configuración especificada. Realiza las siguientes operaciones:
 * 
 * 1. **Validación**: Verifica parámetros de entrada válidos
 * 2. **Limpieza**: Inicializa la estructura a cero
 * 3. **Configuración del grupo**: Establece ID del edificio y número de ascensores
 * 4. **Inicialización individual**: Configura cada ascensor con:
 *    - ID único (formato: {edificio_id}A{numero})
 *    - Piso inicial: 0 (planta baja)
 *    - Puertas cerradas
 *    - Sin tarea asignada
 *    - Estado parado y disponible
 * 
 * @note El número de ascensores debe estar entre 1 y MAX_ELEVATORS_PER_GATEWAY
 * @see elevator_group_state_t
 * @see elevator_status_t
 */
void init_elevator_group(elevator_group_state_t *group, const char* edificio_id_str, int num_elevadores, int num_pisos) {
    if (!group || !edificio_id_str) {
        LOG_ERROR_GW("StateMgr: Error inicializando grupo, group o edificio_id_str es NULL.");
        return;
    }

    if (num_elevadores <= 0 || num_elevadores > MAX_ELEVATORS_PER_GATEWAY) {
        LOG_ERROR_GW("StateMgr: Número de elevadores (%d) inválido. Debe estar entre 1 y %d.", num_elevadores, MAX_ELEVATORS_PER_GATEWAY);
        group->num_elevadores_en_grupo = 0;
        return;
    }

    memset(group, 0, sizeof(elevator_group_state_t)); // Limpiar toda la estructura
    strncpy(group->edificio_id_str_grupo, edificio_id_str, ID_STRING_MAX_LEN -1);
    group->edificio_id_str_grupo[ID_STRING_MAX_LEN -1] = '\0'; // Asegurar null-termination
    group->num_elevadores_en_grupo = num_elevadores;

    LOG_INFO_GW("StateMgr: Inicializando %d ascensores para edificio '%s', %d pisos.", num_elevadores, edificio_id_str, num_pisos);

    for (int i = 0; i < num_elevadores; ++i) {
        elevator_status_t *elevator = &group->ascensores[i];
        snprintf(elevator->ascensor_id, ID_STRING_MAX_LEN, "%sA%d", edificio_id_str, i + 1);
        strncpy(elevator->id_edificio_str, edificio_id_str, ID_STRING_MAX_LEN - 1);
        elevator->id_edificio_str[ID_STRING_MAX_LEN - 1] = '\0';
        
        elevator->piso_actual = 0; // Todos empiezan en planta baja (piso 0)
        elevator->estado_puerta_enum = DOOR_CLOSED;
        elevator->tarea_actual_id[0] = '\0'; // Sin tarea
        elevator->destino_actual = -1; // Sin destino
        elevator->direccion_movimiento_enum = STOPPED;
        elevator->ocupado = false;

        LOG_DEBUG_GW("StateMgr: Ascensor %s inicializado: Piso %d, Puerta %s, Ocupado: %s", 
                     elevator->ascensor_id, 
                     elevator->piso_actual, 
                     door_state_to_string(elevator->estado_puerta_enum),
                     elevator->ocupado ? "Sí" : "No");
    }
}

cJSON* elevator_group_to_json_for_server(const elevator_group_state_t *group, 
                                           gw_request_type_t request_type, 
                                           const api_request_details_for_json_t* details) {
    if (!group) {
        LOG_ERROR_GW("StateMgr: elevator_group_to_json - group es NULL.");
        return NULL;
    }

    cJSON *root = cJSON_CreateObject();
    if (!root) {
        LOG_ERROR_GW("StateMgr: Failed to create root JSON object.");
        return NULL;
    }

    cJSON_AddStringToObject(root, "id_edificio", group->edificio_id_str_grupo);

    // Add request-specific details if provided
    if (details) {
        switch (request_type) {
            case GW_REQUEST_TYPE_FLOOR_CALL:
                cJSON_AddNumberToObject(root, "piso_origen_llamada", details->origin_floor_fc);
                cJSON_AddStringToObject(root, "direccion_llamada", movement_direction_to_string(details->direction_fc));
                // The target_floor_for_task for a floor call is origin_floor_fc itself.
                // This is implicitly handled by the server if it needs it for task assignment logic,
                // or the gateway already set it in the tracker for its own use.
                // For now, just sending what the server needs to identify the call.
                break;
            case GW_REQUEST_TYPE_CABIN_REQUEST:
                cJSON_AddStringToObject(root, "solicitando_ascensor_id", details->requesting_elevator_id_cr);
                cJSON_AddNumberToObject(root, "piso_destino_solicitud", details->target_floor_cr);
                break;
            case GW_REQUEST_TYPE_UNKNOWN:
            default:
                LOG_WARN_GW("StateMgr: Unknown or unhandled request type (%d) for adding specific JSON details.", request_type);
                break;
        }
    }

    cJSON *elevadores_array = cJSON_CreateArray();
    if (!elevadores_array) {
        LOG_ERROR_GW("StateMgr: Failed to create elevadores_estado JSON array.");
        cJSON_Delete(root);
        return NULL;
    }
    cJSON_AddItemToObject(root, "elevadores_estado", elevadores_array);

    for (int i = 0; i < group->num_elevadores_en_grupo; ++i) {
        const elevator_status_t *elevator = &group->ascensores[i];
        cJSON *elevator_json = cJSON_CreateObject();
        if (!elevator_json) {
            LOG_ERROR_GW("StateMgr: Failed to create JSON object for elevator %s.", elevator->ascensor_id);
            cJSON_Delete(root); // Clean up partially created JSON
            return NULL;
        }

        cJSON_AddStringToObject(elevator_json, "id_ascensor", elevator->ascensor_id);
        cJSON_AddNumberToObject(elevator_json, "piso_actual", elevator->piso_actual);
        cJSON_AddStringToObject(elevator_json, "estado_puerta", door_state_to_string(elevator->estado_puerta_enum));
        
        // El servidor espera "disponible", que es lo inverso de nuestro "ocupado".
        cJSON_AddBoolToObject(elevator_json, "disponible", !elevator->ocupado); 

        if (elevator->tarea_actual_id[0] != '\0') {
            cJSON_AddStringToObject(elevator_json, "tarea_actual_id", elevator->tarea_actual_id);
        } else {
            cJSON_AddNullToObject(elevator_json, "tarea_actual_id");
        }

        if (elevator->destino_actual != -1) {
            cJSON_AddNumberToObject(elevator_json, "destino_actual", elevator->destino_actual);
        } else {
            cJSON_AddNullToObject(elevator_json, "destino_actual");
        }
        // Nota: No estamos incluyendo "direccion_movimiento" en el payload para el servidor central
        // según la especificación actual del servidor. Si se necesita, se puede añadir.

        cJSON_AddItemToArray(elevadores_array, elevator_json);
    }

    return root;
}

void assign_task_to_elevator(elevator_group_state_t *group, const char* elevator_id_to_update, const char* task_id, int target_floor, int current_request_floor) {
    if (!group || !elevator_id_to_update || !task_id) {
        LOG_ERROR_GW("StateMgr: assign_task - Argumentos inválidos (NULL group, elevator_id o task_id).");
        return;
    }

    bool found = false;
    for (int i = 0; i < group->num_elevadores_en_grupo; ++i) {
        elevator_status_t *elevator = &group->ascensores[i];
        if (strcmp(elevator->ascensor_id, elevator_id_to_update) == 0) {
            strncpy(elevator->tarea_actual_id, task_id, TASK_ID_MAX_LEN - 1);
            elevator->tarea_actual_id[TASK_ID_MAX_LEN - 1] = '\0';
            elevator->destino_actual = target_floor;
            elevator->ocupado = true;

            // Determinar dirección de movimiento
            // Si el ascensor ya está en el piso de la solicitud, la dirección es hacia el target_floor
            // Si no, current_request_floor nos da el punto de partida para la nueva tarea.
            int reference_floor_for_direction = elevator->piso_actual;
            // Si el ascensor está siendo asignado a una tarea que no es donde está ahora mismo,
            // y current_request_floor es válido, usarlo como referencia.
            // Esto es útil si el ascensor estaba parado y se le asigna una llamada desde otro piso.
            // Sin embargo, en nuestro modelo, el ascensor es asignado y *luego* se movería.
            // Para simplificar: la dirección se basa en su piso_actual vs target_floor.
            // La lógica de si puede recoger current_request_floor en el camino sería más compleja.

            if (target_floor > elevator->piso_actual) {
                elevator->direccion_movimiento_enum = MOVING_UP;
            } else if (target_floor < elevator->piso_actual) {
                elevator->direccion_movimiento_enum = MOVING_DOWN;
            } else { // target_floor == elevator->piso_actual
                // Si el destino es el piso actual, podría estar abriendo puertas, o es una tarea instantánea.
                // Consideraremos que se detiene momentáneamente si no estaba ya parado.
                elevator->direccion_movimiento_enum = STOPPED; 
                // Aquí podríamos cambiar estado_puerta_enum a DOOR_OPENING si la lógica lo requiere.
            }

            LOG_INFO_GW("StateMgr: Tarea '%s' asignada a ascensor %s. Destino: piso %d. Piso actual: %d. Dirección: %s",
                        elevator->tarea_actual_id, 
                        elevator->ascensor_id,
                        elevator->destino_actual,
                        elevator->piso_actual,
                        movement_direction_to_string(elevator->direccion_movimiento_enum));
            
            // Registrar asignación de tarea en el logger
            exec_logger_log_task_assigned(elevator->tarea_actual_id, elevator->ascensor_id, elevator->destino_actual);
            
            found = true;
            break;
        }
    }

    if (!found) {
        LOG_ERROR_GW("StateMgr: assign_task - Ascensor con ID '%s' no encontrado en el grupo.", elevator_id_to_update);
    }
} 