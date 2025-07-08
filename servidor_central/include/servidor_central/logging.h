/**
 * @file logging.h
 * @brief Sistema de logging estructurado para el servidor central
 * @author Sistema de Control de Ascensores
 * @version 2.0
 * @date 2025
 * 
 * @details Este archivo define el sistema de logging completo del servidor central,
 * incluyendo macros para diferentes niveles de log, funciones de inicialización
 * y configuración del sistema de logging.
 * 
 * @see server_functions.h
 * @see main.c
 */

#ifndef LOGGING_H
#define LOGGING_H

#include <stdio.h>
#include <time.h>
#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Niveles de logging disponibles
 * 
 * @details Define los diferentes niveles de logging del sistema,
 * ordenados de menor a mayor prioridad.
 */
typedef enum {
    LOG_LEVEL_DEBUG = 0,    /**< Información detallada para debugging */
    LOG_LEVEL_INFO = 1,     /**< Información general del sistema */
    LOG_LEVEL_WARN = 2,     /**< Advertencias que no impiden funcionamiento */
    LOG_LEVEL_ERROR = 3,    /**< Errores que afectan funcionalidad */
    LOG_LEVEL_CRIT = 4      /**< Errores críticos que pueden causar fallos */
} log_level_t;

/**
 * @brief Configuración del sistema de logging
 * 
 * @details Esta estructura contiene la configuración del sistema de logging,
 * incluyendo el nivel mínimo, el archivo de salida y las opciones de formato.
 */
typedef struct {
    log_level_t min_level;      /**< Nivel mínimo de logging */
    FILE *log_file;             /**< Archivo de salida para logs */
    int use_timestamps;         /**< 1 para incluir timestamps, 0 para no */
    int use_colors;             /**< 1 para colores en terminal, 0 para no */
    int use_thread_id;          /**< 1 para incluir ID de thread, 0 para no */
    char *log_format;           /**< Formato personalizado de log */
} logging_config_t;

/**
 * @brief Configuración por defecto del sistema de logging
 * 
 * @details Configuración recomendada para entorno de producción:
 * - Nivel mínimo: INFO
 * - Timestamps habilitados
 * - Colores habilitados en terminal
 * - ID de thread habilitado
 * - Formato estándar
 */
#define DEFAULT_LOGGING_CONFIG { \
    .min_level = LOG_LEVEL_INFO, \
    .log_file = NULL, \
    .use_timestamps = 1, \
    .use_colors = 1, \
    .use_thread_id = 1, \
    .log_format = NULL \
}

/**
 * @brief Inicializa el sistema de logging
 * 
 * @param[in] config Configuración del sistema de logging
 * @param[in] log_file_path Ruta al archivo de log (NULL para stdout)
 * 
 * @return 0 en caso de éxito, -1 en caso de error
 * 
 * @details Esta función inicializa el sistema de logging:
 * - Configura el nivel mínimo de logging
 * - Abre el archivo de log si se especifica
 * - Configura el formato de salida
 * - Inicializa las opciones de timestamp y colores
 * 
 * @note Debe ser llamada antes de usar cualquier macro de logging
 * @see cleanup_logging
 */
int init_logging(const logging_config_t *config, const char *log_file_path);

/**
 * @brief Limpia y cierra el sistema de logging
 * 
 * @details Esta función limpia los recursos del sistema de logging:
 * - Cierra el archivo de log si está abierto
 * - Libera memoria dinámica
 * - Resetea la configuración
 * 
 * @note Debe ser llamada al finalizar el programa
 * @see init_logging
 */
void cleanup_logging(void);

/**
 * @brief Establece el nivel mínimo de logging
 * 
 * @param[in] level Nuevo nivel mínimo de logging
 * 
 * @details Esta función permite cambiar dinámicamente el nivel mínimo
 * de logging durante la ejecución del programa.
 * 
 * @note Solo los logs con nivel >= al mínimo serán mostrados
 */
void set_log_level(log_level_t level);

/**
 * @brief Obtiene el nivel actual de logging
 * 
 * @return Nivel actual de logging
 * 
 * @details Esta función retorna el nivel mínimo de logging actualmente configurado.
 */
log_level_t get_log_level(void);

/**
 * @brief Función interna para escribir logs
 * 
 * @param[in] level Nivel del log
 * @param[in] file Archivo donde se originó el log
 * @param[in] line Línea donde se originó el log
 * @param[in] func Función donde se originó el log
 * @param[in] format Formato del mensaje
 * @param[in] ... Argumentos variables del mensaje
 * 
 * @details Esta función es llamada internamente por las macros de logging.
 * No debe ser llamada directamente desde el código.
 */
void _log_message(log_level_t level, const char *file, int line, const char *func,
                  const char *format, ...);

/**
 * @brief Macro para logging de debug
 * 
 * @param[in] format Formato del mensaje
 * @param[in] ... Argumentos variables del mensaje
 * 
 * @details Esta macro genera un log de nivel DEBUG con información detallada
 * útil para debugging del código.
 * 
 * @note Solo se genera si el nivel mínimo es DEBUG
 * @see LOG_LEVEL_DEBUG
 */
#define LOG_DEBUG(format, ...) \
    _log_message(LOG_LEVEL_DEBUG, __FILE__, __LINE__, __func__, format, ##__VA_ARGS__)

/**
 * @brief Macro para logging de información
 * 
 * @param[in] format Formato del mensaje
 * @param[in] ... Argumentos variables del mensaje
 * 
 * @details Esta macro genera un log de nivel INFO con información general
 * sobre el funcionamiento del sistema.
 * 
 * @note Solo se genera si el nivel mínimo es INFO o menor
 * @see LOG_LEVEL_INFO
 */
#define LOG_INFO(format, ...) \
    _log_message(LOG_LEVEL_INFO, __FILE__, __LINE__, __func__, format, ##__VA_ARGS__)

/**
 * @brief Macro para logging de advertencias
 * 
 * @param[in] format Formato del mensaje
 * @param[in] ... Argumentos variables del mensaje
 * 
 * @details Esta macro genera un log de nivel WARN para situaciones que
 * requieren atención pero no impiden el funcionamiento normal.
 * 
 * @note Solo se genera si el nivel mínimo es WARN o menor
 * @see LOG_LEVEL_WARN
 */
#define LOG_WARN(format, ...) \
    _log_message(LOG_LEVEL_WARN, __FILE__, __LINE__, __func__, format, ##__VA_ARGS__)

/**
 * @brief Macro para logging de errores
 * 
 * @param[in] format Formato del mensaje
 * @param[in] ... Argumentos variables del mensaje
 * 
 * @details Esta macro genera un log de nivel ERROR para errores que
 * afectan la funcionalidad pero no causan fallos críticos.
 * 
 * @note Solo se genera si el nivel mínimo es ERROR o menor
 * @see LOG_LEVEL_ERROR
 */
#define LOG_ERROR(format, ...) \
    _log_message(LOG_LEVEL_ERROR, __FILE__, __LINE__, __func__, format, ##__VA_ARGS__)

/**
 * @brief Macro para logging de errores críticos
 * 
 * @param[in] format Formato del mensaje
 * @param[in] ... Argumentos variables del mensaje
 * 
 * @details Esta macro genera un log de nivel CRIT para errores críticos
 * que pueden causar fallos en el sistema.
 * 
 * @note Solo se genera si el nivel mínimo es CRIT o menor
 * @see LOG_LEVEL_CRIT
 */
#define LOG_CRIT(format, ...) \
    _log_message(LOG_LEVEL_CRIT, __FILE__, __LINE__, __func__, format, ##__VA_ARGS__)

/**
 * @brief Macro para logging de inicio de función
 * 
 * @param[in] func_name Nombre de la función
 * @param[in] ... Argumentos adicionales opcionales
 * 
 * @details Esta macro genera un log de entrada a una función con información
 * sobre los parámetros de entrada.
 * 
 * @note Útil para debugging y trazabilidad
 */
#define LOG_FUNC_ENTER(func_name, ...) \
    LOG_DEBUG("Entering %s", func_name)

/**
 * @brief Macro para logging de salida de función
 * 
 * @param[in] func_name Nombre de la función
 * @param[in] return_value Valor de retorno (opcional)
 * 
 * @details Esta macro genera un log de salida de una función con información
 * sobre el valor de retorno.
 * 
 * @note Útil para debugging y trazabilidad
 */
#define LOG_FUNC_EXIT(func_name, return_value) \
    LOG_DEBUG("Exiting %s with return value %d", func_name, return_value)

/**
 * @brief Macro para logging de variables
 * 
 * @param[in] var_name Nombre de la variable
 * @param[in] var_value Valor de la variable
 * @param[in] format Formato para mostrar el valor
 * 
 * @details Esta macro genera un log con el valor de una variable específica.
 * Útil para debugging y monitoreo de variables.
 */
#define LOG_VAR(var_name, var_value, format) \
    LOG_DEBUG("%s = " format, var_name, var_value)

#ifdef __cplusplus
}
#endif

#endif /* LOGGING_H */ 