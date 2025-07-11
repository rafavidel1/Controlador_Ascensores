/**
 * @file api_common_defs.h
 * @brief Definiciones Comunes para el API Gateway
 * @author Sistema de Control de Ascensores
 * @date 2025
 * @version 2.0
 * 
 * Este archivo define constantes y tipos de datos comunes utilizados
 * por todos los componentes del API Gateway del sistema de control
 * de ascensores.
 * 
 * **Constantes definidas:**
 * - Longitudes máximas para strings de identificadores
 * - Longitudes para IDs de tareas
 * - Tamaños de buffers para strings de estado
 * 
 * Las definiciones aquí centralizan valores utilizados en múltiples
 * módulos para mantener consistencia y facilitar el mantenimiento.
 * 
 * @see elevator_state_manager.h
 * @see api_handlers.h
 * @see logging_gw.h
 */
#ifndef API_COMMON_DEFS_H
#define API_COMMON_DEFS_H

#include "api_gateway/logging_gw.h"

/**
 * @brief Longitud máxima para strings de identificación
 * 
 * Define el tamaño máximo en caracteres para identificadores como:
 * - IDs de ascensores (ej: "E1A1", "E2A5")
 * - IDs de edificios (ej: "E1", "TOWER_A")
 * - Identificadores de clientes en general
 * 
 * Incluye espacio para el carácter nulo terminador.
 */
#define ID_STRING_MAX_LEN 25

/**
 * @brief Longitud máxima para IDs de tareas
 * 
 * Define el tamaño máximo en caracteres para identificadores
 * de tareas asignadas por el servidor central (ej: "T_1640995200123").
 * 
 * Incluye espacio para el carácter nulo terminador.
 */
#define TASK_ID_MAX_LEN 32

/**
 * @brief Longitud máxima para strings de estado
 * 
 * Define el tamaño máximo en caracteres para representaciones
 * en string de estados del sistema como:
 * - Estados de puertas ("ABIERTA", "CERRADA", "ABRIENDO")
 * - Direcciones de movimiento ("SUBIENDO", "BAJANDO")
 * - Estados de disponibilidad
 * 
 * Incluye espacio para el carácter nulo terminador.
 */
#define STATUS_STRING_MAX_LEN 16

/**
 * @note Las macros de logging y códigos de color ANSI están definidas
 *       en logging_gw.h para mantener la separación de responsabilidades.
 * 
 * @see logging_gw.h
 */

#endif // API_COMMON_DEFS_H 