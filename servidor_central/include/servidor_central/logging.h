/**
 * @file logging.h
 * @brief Sistema de logging simplificado para el servidor central
 * @author Sistema de Ascensores
 * @date 2024
 * @version 1.0
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

// Definiciones de colores ANSI
#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

// Función auxiliar para obtener timestamp
static inline void get_timestamp(char *buffer, size_t size) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    struct tm *tm_info = localtime(&tv.tv_sec);
    strftime(buffer, size, "%H:%M:%S", tm_info);
    sprintf(buffer + strlen(buffer), ".%03d", (int)(tv.tv_usec / 1000));
}

// Macros de logging simplificadas
#define SRV_LOG_DEBUG(format, ...) do { \
    char timestamp[32]; \
    get_timestamp(timestamp, sizeof(timestamp)); \
    printf(ANSI_COLOR_BLUE "[DEBUG] %s " ANSI_COLOR_RESET format "\n", timestamp, ##__VA_ARGS__); \
    fflush(stdout); \
} while(0)

#define SRV_LOG_INFO(format, ...) do { \
    char timestamp[32]; \
    get_timestamp(timestamp, sizeof(timestamp)); \
    printf(ANSI_COLOR_GREEN "[INFO] %s " ANSI_COLOR_RESET format "\n", timestamp, ##__VA_ARGS__); \
    fflush(stdout); \
} while(0)

#define SRV_LOG_WARN(format, ...) do { \
    char timestamp[32]; \
    get_timestamp(timestamp, sizeof(timestamp)); \
    fprintf(stderr, ANSI_COLOR_YELLOW "[WARN] %s " ANSI_COLOR_RESET format "\n", timestamp, ##__VA_ARGS__); \
    fflush(stderr); \
} while(0)

#define SRV_LOG_ERROR(format, ...) do { \
    char timestamp[32]; \
    get_timestamp(timestamp, sizeof(timestamp)); \
    fprintf(stderr, ANSI_COLOR_RED "[ERROR] %s " ANSI_COLOR_RESET format "\n", timestamp, ##__VA_ARGS__); \
    fflush(stderr); \
} while(0)

#define SRV_LOG_CRIT(format, ...) do { \
    char timestamp[32]; \
    get_timestamp(timestamp, sizeof(timestamp)); \
    fprintf(stderr, ANSI_COLOR_MAGENTA "[CRIT] %s " ANSI_COLOR_RESET format "\n", timestamp, ##__VA_ARGS__); \
    fflush(stderr); \
} while(0)

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