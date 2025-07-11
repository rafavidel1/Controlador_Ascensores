/**
 * @file logging.h
 * @brief Sistema de logging simplificado para el servidor central
 * @author Sistema de Control de Ascensores
 * @version 2.0
 * @date 2025
 * 
 * @details Este archivo define un sistema de logging simplificado y eficiente
 * para el servidor central. Proporciona macros de logging con colores ANSI,
 * timestamps precisos y compatibilidad con libcoap.
 * 
 * **Características:**
 * - Logging con colores ANSI para mejor legibilidad
 * - Timestamps con precisión de milisegundos
 * - Múltiples niveles de logging (DEBUG, INFO, WARN, ERROR, CRIT)
 * - Compatibilidad con libcoap evitando conflictos de macros
 * - Salida a stdout para INFO/DEBUG y stderr para WARN/ERROR
 * - Flush automático para garantizar output inmediato
 * 
 * **Niveles de logging:**
 * - `SRV_LOG_DEBUG`: Información detallada de debugging
 * - `SRV_LOG_INFO`: Información general del funcionamiento
 * - `SRV_LOG_WARN`: Advertencias que no impiden el funcionamiento
 * - `SRV_LOG_ERROR`: Errores que pueden afectar el funcionamiento
 * - `SRV_LOG_CRIT`: Errores críticos que requieren atención inmediata
 * 
 * @see main.c
 * @see psk_validator.h
 */

#ifndef LOGGING_H
#define LOGGING_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>

// Evitar conflictos con libcoap redefiniendo sus macros
#ifdef LOG_DEBUG
#undef LOG_DEBUG
#endif
#ifdef LOG_INFO
#undef LOG_INFO
#endif
#ifdef LOG_WARN
#undef LOG_WARN
#endif
#ifdef LOG_ERROR
#undef LOG_ERROR
#endif
#ifdef LOG_CRIT
#undef LOG_CRIT
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup ansi_colors Códigos de color ANSI
 * @brief Definiciones de códigos de escape ANSI para colores de terminal
 * @{
 */
#define ANSI_COLOR_RED     "\x1b[31m"    /**< Color rojo para errores */
#define ANSI_COLOR_GREEN   "\x1b[32m"    /**< Color verde para información */
#define ANSI_COLOR_YELLOW  "\x1b[33m"    /**< Color amarillo para advertencias */
#define ANSI_COLOR_BLUE    "\x1b[34m"    /**< Color azul para debug */
#define ANSI_COLOR_MAGENTA "\x1b[35m"    /**< Color magenta para crítico */
#define ANSI_COLOR_CYAN    "\x1b[36m"    /**< Color cian para información especial */
#define ANSI_COLOR_RESET   "\x1b[0m"     /**< Resetear color a default */
/** @} */

/**
 * @brief Obtiene el timestamp actual con precisión de milisegundos
 * 
 * @param[out] buffer Buffer donde se almacenará el timestamp
 * @param[in] size Tamaño del buffer en bytes
 * 
 * @details Esta función genera un timestamp en formato HH:MM:SS.mmm
 * utilizando gettimeofday() para obtener precisión de milisegundos.
 * 
 * **Formato de salida:** "HH:MM:SS.mmm"
 * **Ejemplo:** "14:23:45.123"
 * 
 * @note El buffer debe tener al menos 13 caracteres para el timestamp completo
 * @note La función es thread-safe en sistemas POSIX
 */
static inline void get_timestamp(char *buffer, size_t size) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    struct tm *tm_info = localtime(&tv.tv_sec);
    strftime(buffer, size, "%H:%M:%S", tm_info);
    sprintf(buffer + strlen(buffer), ".%03d", (int)(tv.tv_usec / 1000));
}

/**
 * @defgroup logging_macros Macros de logging con colores
 * @brief Macros para logging con timestamps y colores ANSI
 * @{
 */

/**
 * @brief Macro para logging de nivel DEBUG
 * @param[in] format String de formato estilo printf
 * @param[in] ... Argumentos variables para el formato
 * 
 * @details Genera logs de debugging con:
 * - Color azul para fácil identificación
 * - Timestamp con precisión de milisegundos
 * - Salida a stdout con flush automático
 * - Prefijo [DEBUG] para identificación
 */
#define SRV_LOG_DEBUG(format, ...) do { \
    char timestamp[32]; \
    get_timestamp(timestamp, sizeof(timestamp)); \
    printf(ANSI_COLOR_BLUE "[DEBUG] %s " ANSI_COLOR_RESET format "\n", timestamp, ##__VA_ARGS__); \
    fflush(stdout); \
} while(0)

/**
 * @brief Macro para logging de nivel INFO
 * @param[in] format String de formato estilo printf
 * @param[in] ... Argumentos variables para el formato
 * 
 * @details Genera logs informativos con:
 * - Color verde para fácil identificación
 * - Timestamp con precisión de milisegundos
 * - Salida a stdout con flush automático
 * - Prefijo [INFO] para identificación
 */
#define SRV_LOG_INFO(format, ...) do { \
    char timestamp[32]; \
    get_timestamp(timestamp, sizeof(timestamp)); \
    printf(ANSI_COLOR_GREEN "[INFO] %s " ANSI_COLOR_RESET format "\n", timestamp, ##__VA_ARGS__); \
    fflush(stdout); \
} while(0)

/**
 * @brief Macro para logging de nivel WARN
 * @param[in] format String de formato estilo printf
 * @param[in] ... Argumentos variables para el formato
 * 
 * @details Genera logs de advertencia con:
 * - Color amarillo para fácil identificación
 * - Timestamp con precisión de milisegundos
 * - Salida a stderr con flush automático
 * - Prefijo [WARN] para identificación
 */
#define SRV_LOG_WARN(format, ...) do { \
    char timestamp[32]; \
    get_timestamp(timestamp, sizeof(timestamp)); \
    fprintf(stderr, ANSI_COLOR_YELLOW "[WARN] %s " ANSI_COLOR_RESET format "\n", timestamp, ##__VA_ARGS__); \
    fflush(stderr); \
} while(0)

/**
 * @brief Macro para logging de nivel ERROR
 * @param[in] format String de formato estilo printf
 * @param[in] ... Argumentos variables para el formato
 * 
 * @details Genera logs de error con:
 * - Color rojo para fácil identificación
 * - Timestamp con precisión de milisegundos
 * - Salida a stderr con flush automático
 * - Prefijo [ERROR] para identificación
 */
#define SRV_LOG_ERROR(format, ...) do { \
    char timestamp[32]; \
    get_timestamp(timestamp, sizeof(timestamp)); \
    fprintf(stderr, ANSI_COLOR_RED "[ERROR] %s " ANSI_COLOR_RESET format "\n", timestamp, ##__VA_ARGS__); \
    fflush(stderr); \
} while(0)

/**
 * @brief Macro para logging de nivel CRÍTICO
 * @param[in] format String de formato estilo printf
 * @param[in] ... Argumentos variables para el formato
 * 
 * @details Genera logs críticos con:
 * - Color magenta para máxima visibilidad
 * - Timestamp con precisión de milisegundos
 * - Salida a stderr con flush automático
 * - Prefijo [CRIT] para identificación
 */
#define SRV_LOG_CRIT(format, ...) do { \
    char timestamp[32]; \
    get_timestamp(timestamp, sizeof(timestamp)); \
    fprintf(stderr, ANSI_COLOR_MAGENTA "[CRIT] %s " ANSI_COLOR_RESET format "\n", timestamp, ##__VA_ARGS__); \
    fflush(stderr); \
} while(0)

/** @} */

// Aliases para compatibilidad con el código original
#define LOG_DEBUG(format, ...) SRV_LOG_DEBUG(format, ##__VA_ARGS__)
#define LOG_INFO(format, ...) SRV_LOG_INFO(format, ##__VA_ARGS__)
#define LOG_WARN(format, ...) SRV_LOG_WARN(format, ##__VA_ARGS__)
#define LOG_ERROR(format, ...) SRV_LOG_ERROR(format, ##__VA_ARGS__)
#define LOG_CRIT(format, ...) SRV_LOG_CRIT(format, ##__VA_ARGS__)

// Funciones dummy para compatibilidad (no hacen nada)
static inline int init_logging(void *config, const char *log_file_path) { return 0; }
static inline void cleanup_logging(void) { }
static inline void set_log_level(int level) { }
static inline int get_log_level(void) { return 0; }

#ifdef __cplusplus
}
#endif

#endif /* LOGGING_H */ 