/**
 * @file server_functions.h
 * @brief Funciones Auxiliares del Servidor Central
 * @author Sistema de Control de Ascensores
 * @date 2025
 * @version 2.0
 * 
 * Este archivo define las funciones auxiliares utilizadas por el servidor central
 * para operaciones comunes como generación de identificadores únicos y
 * utilidades de procesamiento.
 * 
 * **Funciones incluidas:**
 * - Generación de IDs únicos para tareas
 * - Utilidades de tiempo y timestamp
 * - Funciones helper para procesamiento de solicitudes
 * 
 * Las funciones definidas aquí son utilizadas por los manejadores CoAP
 * del servidor central para generar respuestas consistentes y únicas.
 * 
 * @see servidor_central/main.c
 * @see logging.h
 */
#ifndef SERVER_FUNCTIONS_H
#define SERVER_FUNCTIONS_H

#include <stddef.h>

/**
 * @brief Genera un ID único para una tarea de ascensor
 * @param task_id_out Buffer donde se escribirá el ID generado
 * @param len Tamaño del buffer de salida
 * 
 * Esta función genera un identificador único para tareas de ascensor
 * basado en el timestamp actual del sistema. El formato del ID es:
 * "T_{segundos_unix}{milisegundos}"
 * 
 * **Ejemplo de salida:** "T_1640995200123"
 * - T_: Prefijo identificador de tarea
 * - 1640995200: Segundos desde epoch Unix
 * - 123: Milisegundos (3 dígitos)
 * 
 * **Características:**
 * - Garantiza unicidad temporal
 * - Formato legible y ordenable
 * - Compatible con sistemas de logging
 * - Longitud predecible (máximo 32 caracteres)
 * 
 * @note El buffer de salida debe tener al menos 32 caracteres
 * @note Esta implementación no es thread-safe para entornos multihilo
 * 
 * @warning Para entornos multihilo se recomienda añadir sincronización
 *          o usar contadores atómicos adicionales
 * 
 * @see gettimeofday()
 * @see main.c::hnd_floor_call()
 * @see main.c::hnd_cabin_request()
 */
void generate_unique_task_id(char *task_id_out, size_t len);

#endif // SERVER_FUNCTIONS_H 