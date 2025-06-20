/**
 * @file logging_gw.h
 * @brief Sistema de Logging para el API Gateway
 * @author Sistema de Control de Ascensores
 * @date 2025
 * @version 2.0
 * 
 * Este archivo define las macros de logging utilizadas por el API Gateway
 * para generar mensajes de depuración, información, advertencias y errores
 * con formato consistente y colores ANSI.
 * 
 * **Características:**
 * - Colores ANSI para diferenciación visual de niveles
 * - Macros variádicas compatibles con printf
 * - Prefijo específico del gateway [*-GW] para identificación
 * - Flush automático para salida inmediata
 * - Separación entre stdout (info/debug) y stderr (warn/error)
 * 
 * **Niveles de logging:**
 * - INFO: Información general del gateway
 * - DEBUG: Información detallada para depuración
 * - WARN: Advertencias que no afectan funcionalidad crítica
 * - ERROR: Errores que requieren atención
 * - CRIT: Errores críticos que pueden preceder a una salida
 * 
 * @see servidor_central/logging.h
 * @see api_common_defs.h
 */
#ifndef LOGGING_GW_H
#define LOGGING_GW_H

#include <stdio.h> // Para printf

/**
 * @defgroup ansi_colors_gw Códigos de Color ANSI para Gateway
 * @brief Definiciones de códigos de escape ANSI para colorear la salida del gateway
 * @{
 */

/** @brief Color rojo ANSI para errores */
#define ANSI_COLOR_RED     "\x1b[31m"

/** @brief Color verde ANSI para información exitosa */
#define ANSI_COLOR_GREEN   "\x1b[32m"

/** @brief Color amarillo ANSI para advertencias */
#define ANSI_COLOR_YELLOW  "\x1b[33m"

/** @brief Color azul ANSI para información de depuración */
#define ANSI_COLOR_BLUE    "\x1b[34m"

/** @brief Color magenta ANSI para errores críticos */
#define ANSI_COLOR_MAGENTA "\x1b[35m"

/** @brief Color cian ANSI para información general */
#define ANSI_COLOR_CYAN    "\x1b[36m"

/** @brief Reset de color ANSI para volver al color por defecto */
#define ANSI_COLOR_RESET   "\x1b[0m"

/** @} */ // end of ansi_colors_gw group

/**
 * @defgroup logging_macros_gw Macros de Logging del Gateway
 * @brief Macros para generar mensajes de log con formato específico del gateway
 * @{
 */

/**
 * @brief Macro para mensajes informativos del gateway
 * @param format Formato de mensaje (estilo printf)
 * @param ... Argumentos variables para el formato
 * 
 * Genera mensajes de información general con prefijo [INFO-GW] en color verde.
 * Utilizado para eventos normales del gateway y confirmaciones de operaciones.
 * La salida se dirige a stdout con flush automático.
 */
#define LOG_INFO_GW(format, ...) \
    do { \
        printf(ANSI_COLOR_GREEN "[INFO-GW] " ANSI_COLOR_RESET format "\n", ##__VA_ARGS__); \
        fflush(stdout); \
    } while (0)

/**
 * @brief Macro para mensajes de depuración del gateway
 * @param format Formato de mensaje (estilo printf)
 * @param ... Argumentos variables para el formato
 * 
 * Genera mensajes de depuración con prefijo [DEBUG-GW] en color azul.
 * Utilizado para información detallada durante el desarrollo y troubleshooting.
 * La salida se dirige a stdout con flush automático.
 * 
 * @note Estos mensajes pueden compilarse condicionalmente en versiones de producción
 */
#define LOG_DEBUG_GW(format, ...) \
    do { \
        printf(ANSI_COLOR_BLUE "[DEBUG-GW] " ANSI_COLOR_RESET format "\n", ##__VA_ARGS__); \
        fflush(stdout); \
    } while (0)

/**
 * @brief Macro para mensajes de advertencia del gateway
 * @param format Formato de mensaje (estilo printf)
 * @param ... Argumentos variables para el formato
 * 
 * Genera mensajes de advertencia con prefijo [WARN-GW] en color amarillo.
 * Utilizado para situaciones que requieren atención pero no impiden el funcionamiento.
 * La salida se dirige a stderr con flush automático.
 */
#define LOG_WARN_GW(format, ...) \
    do { \
        fprintf(stderr, ANSI_COLOR_YELLOW "[WARN-GW] " ANSI_COLOR_RESET format "\n", ##__VA_ARGS__); \
        fflush(stderr); \
    } while (0)

/**
 * @brief Macro para mensajes de error del gateway
 * @param format Formato de mensaje (estilo printf)
 * @param ... Argumentos variables para el formato
 * 
 * Genera mensajes de error con prefijo [ERROR-GW] en color rojo.
 * Utilizado para errores que requieren atención inmediata o afectan la funcionalidad.
 * La salida se dirige a stderr con flush automático.
 */
#define LOG_ERROR_GW(format, ...) \
    do { \
        fprintf(stderr, ANSI_COLOR_RED "[ERROR-GW] " ANSI_COLOR_RESET format "\n", ##__VA_ARGS__); \
        fflush(stderr); \
    } while (0)

/**
 * @brief Macro para mensajes de error crítico del gateway
 * @param format Formato de mensaje (estilo printf)
 * @param ... Argumentos variables para el formato
 * 
 * Genera mensajes de error crítico con prefijo [CRIT-GW] en color magenta.
 * Utilizado para errores críticos que pueden preceder a una terminación del programa.
 * La salida se dirige a stderr con flush automático.
 * 
 * @warning Estos mensajes indican problemas graves que requieren atención inmediata
 */
#define LOG_CRIT_GW(format, ...) \
    do { \
        fprintf(stderr, ANSI_COLOR_MAGENTA "[CRIT-GW] " ANSI_COLOR_RESET format "\n", ##__VA_ARGS__); \
        fflush(stderr); \
    } while (0)

/** @} */ // end of logging_macros_gw group

#endif // LOGGING_GW_H 