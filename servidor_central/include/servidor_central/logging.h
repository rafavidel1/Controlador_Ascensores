/**
 * @file logging.h
 * @brief Sistema de Logging para el Servidor Central
 * @author Sistema de Control de Ascensores
 * @date 2025
 * @version 2.0
 * 
 * Este archivo define las macros de logging utilizadas por el servidor central
 * para generar mensajes de depuración, información, advertencias y errores
 * con formato consistente y colores ANSI.
 * 
 * **Características:**
 * - Colores ANSI para diferenciación visual de niveles
 * - Macros variádicas compatibles con printf
 * - Prefijos consistentes para identificación de componente
 * - Flush automático para salida inmediata
 * 
 * **Niveles de logging:**
 * - INFO: Información general del sistema
 * - WARN: Advertencias que no afectan funcionalidad crítica
 * - ERROR: Errores que requieren atención
 * - DEBUG: Información detallada para depuración
 * - DB: Operaciones específicas de base de datos (legacy)
 * 
 * @see api_gateway/logging_gw.h
 */
#ifndef LOGGING_H
#define LOGGING_H

#include <stdio.h>

/**
 * @defgroup ansi_colors Códigos de Color ANSI
 * @brief Definiciones de códigos de escape ANSI para colorear la salida de terminal
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

/** @brief Color magenta ANSI para información de depuración */
#define ANSI_COLOR_MAGENTA "\x1b[35m"

/** @brief Color cian ANSI para información general */
#define ANSI_COLOR_CYAN    "\x1b[36m"

/** @brief Reset de color ANSI para volver al color por defecto */
#define ANSI_COLOR_RESET   "\x1b[0m"

/** @} */ // end of ansi_colors group

/**
 * @defgroup logging_macros Macros de Logging
 * @brief Macros para generar mensajes de log con formato consistente
 * @{
 */

/**
 * @brief Macro para mensajes informativos del servidor
 * @param fmt Formato de mensaje (estilo printf)
 * @param ... Argumentos variables para el formato
 * 
 * Genera mensajes de información general con prefijo [INFO] en color cian.
 * Utilizado para eventos normales del sistema y confirmaciones de operaciones.
 */
#define SRV_LOG_INFO(fmt, ...)  printf(ANSI_COLOR_CYAN "[INFO]  " fmt ANSI_COLOR_RESET "\n", ##__VA_ARGS__)

/**
 * @brief Macro para mensajes de advertencia del servidor
 * @param fmt Formato de mensaje (estilo printf)
 * @param ... Argumentos variables para el formato
 * 
 * Genera mensajes de advertencia con prefijo [WARN] en color amarillo.
 * Utilizado para situaciones que requieren atención pero no impiden el funcionamiento.
 */
#define SRV_LOG_WARN(fmt, ...)  printf(ANSI_COLOR_YELLOW "[WARN]  " fmt ANSI_COLOR_RESET "\n", ##__VA_ARGS__)

/**
 * @brief Macro para mensajes de error del servidor
 * @param fmt Formato de mensaje (estilo printf)
 * @param ... Argumentos variables para el formato
 * 
 * Genera mensajes de error con prefijo [ERROR] en color rojo.
 * Utilizado para errores que requieren atención inmediata o afectan la funcionalidad.
 */
#define SRV_LOG_ERROR(fmt, ...) printf(ANSI_COLOR_RED "[ERROR] " fmt ANSI_COLOR_RESET "\n", ##__VA_ARGS__)

/**
 * @brief Macro para mensajes de depuración del servidor
 * @param fmt Formato de mensaje (estilo printf)
 * @param ... Argumentos variables para el formato
 * 
 * Genera mensajes de depuración con prefijo [DEBUG] en color magenta.
 * Utilizado para información detallada durante el desarrollo y troubleshooting.
 */
#define SRV_LOG_DEBUG(fmt, ...) printf(ANSI_COLOR_MAGENTA "[DEBUG] " fmt ANSI_COLOR_RESET "\n", ##__VA_ARGS__)

/**
 * @brief Macro para mensajes de operaciones de base de datos (legacy)
 * @param fmt Formato de mensaje (estilo printf)
 * @param ... Argumentos variables para el formato
 * 
 * Genera mensajes específicos de base de datos con prefijo [DB] en color azul.
 * Mantenido para compatibilidad con versiones anteriores que utilizaban base de datos.
 */
#define SRV_LOG_DB(fmt, ...)    printf(ANSI_COLOR_BLUE "[DB]    " fmt ANSI_COLOR_RESET "\n", ##__VA_ARGS__)

/** @} */ // end of logging_macros group

#endif // LOGGING_H 