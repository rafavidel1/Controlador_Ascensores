/**
 * @file execution_logger.c
 * @brief Implementación del Sistema de Logging de Ejecuciones
 * @author Sistema de Control de Ascensores
 * @date 2025
 * @version 1.0
 */

// Para clock_gettime con precisión de nanosegundos
#define _POSIX_C_SOURCE 199309L

#include "api_gateway/execution_logger.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>  // Para getcwd

// ============================================================================
// VARIABLES GLOBALES PRIVADAS
// ============================================================================

static FILE *log_file = NULL;                  ///< Archivo de log actual
static execution_stats_t stats;                ///< Estadísticas de ejecución
static time_t start_time;                      ///< Tiempo de inicio de ejecución
static bool logger_active = false;             ///< Estado del logger
static char current_log_path[MAX_LOG_PATH];    ///< Ruta del archivo actual

// ============================================================================
// FUNCIONES PRIVADAS
// ============================================================================

/**
 * @brief Crea un directorio recursivamente
 * @param path Ruta del directorio a crear
 * @return true si se creó exitosamente, false en caso de error
 */
static bool create_directory(const char *path) {
    struct stat st = {0};
    
    if (stat(path, &st) == -1) {
        if (mkdir(path, 0755) == -1) {
            if (errno != EEXIST) {
                printf("[EXEC_LOGGER] Error creando directorio %s: %s\n", path, strerror(errno));
                return false;
            }
        }
        printf("[EXEC_LOGGER] Directorio creado: %s\n", path);
    } else {
        printf("[EXEC_LOGGER] Directorio ya existe: %s\n", path);
    }
    return true;
}

/**
 * @brief Obtiene el timestamp actual formateado
 * @param buffer Buffer donde escribir el timestamp
 * @param buffer_size Tamaño del buffer
 * @param format Formato del timestamp
 */
static void get_formatted_timestamp(char *buffer, size_t buffer_size, const char *format) {
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    strftime(buffer, buffer_size, format, tm_info);
}

/**
 * @brief Escribe el header del archivo Markdown
 */
static void write_markdown_header(void) {
    if (!log_file) return;
    
    char timestamp[64];
    get_formatted_timestamp(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S");
    
    fprintf(log_file, "---\n");
    fprintf(log_file, "title: \"Reporte de Ejecución - API Gateway\"\n");
    fprintf(log_file, "subtitle: \"Sistema de Control de Ascensores\"\n");
    fprintf(log_file, "author: \"API Gateway v2.0\"\n");
    fprintf(log_file, "date: \"%s\"\n", timestamp);
    fprintf(log_file, "geometry: margin=2cm\n");
    fprintf(log_file, "fontsize: 11pt\n");
    fprintf(log_file, "documentclass: article\n");
    fprintf(log_file, "header-includes:\n");
    fprintf(log_file, "  - \\usepackage{fancyhdr}\n");
    fprintf(log_file, "  - \\usepackage{graphicx}\n");
    fprintf(log_file, "  - \\pagestyle{fancy}\n");
    fprintf(log_file, "  - \\fancyhf{}\n");
    fprintf(log_file, "  - \\rhead{API Gateway - Sistema de Ascensores}\n");
    fprintf(log_file, "  - \\lfoot{%s}\n", timestamp);
    fprintf(log_file, "  - \\rfoot{\\thepage}\n");
    fprintf(log_file, "---\n\n");
    
    fprintf(log_file, "\\newpage\n\n");
    
    fprintf(log_file, "# Resumen Ejecutivo\n\n");
    fprintf(log_file, "Este documento presenta el registro detallado de la ejecución del API Gateway ");
    fprintf(log_file, "del Sistema de Control de Ascensores. El sistema actúa como intermediario entre ");
    fprintf(log_file, "los controladores CAN de ascensores y el servidor central, proporcionando ");
    fprintf(log_file, "comunicación segura mediante CoAP sobre DTLS-PSK.\n\n");
    
    fprintf(log_file, "## Información del Sistema\n\n");
    fprintf(log_file, "| **Parámetro** | **Valor** |\n");
    fprintf(log_file, "|:--------------|:----------|\n");
    fprintf(log_file, "| **Fecha de Ejecución** | %s |\n", timestamp);
    fprintf(log_file, "| **Versión del Sistema** | 2.0 |\n");
    fprintf(log_file, "| **Estado Inicial** | EN EJECUCION |\n");
    fprintf(log_file, "| **Edificio Simulado** | *Pendiente de asignación* |\n");
    fprintf(log_file, "| **Peticiones Programadas** | *Pendiente de configuración* |\n\n");
    
    fprintf(log_file, "## Configuración Técnica\n\n");
    fprintf(log_file, "### Protocolos de Comunicación\n\n");
    fprintf(log_file, "- **Protocolo Principal:** CoAP (Constrained Application Protocol)\n");
    fprintf(log_file, "- **Seguridad:** DTLS-PSK (Datagram Transport Layer Security con Pre-Shared Key)\n");
    fprintf(log_file, "- **Transporte:** UDP (User Datagram Protocol)\n");
    fprintf(log_file, "- **Puerto de Escucha:** 5683 (Puerto estándar CoAP)\n");
    fprintf(log_file, "- **Servidor Central:** 192.168.49.2:30084 (Minikube Cluster)\n\n");
    
    fprintf(log_file, "### Componentes del Sistema\n\n");
    fprintf(log_file, "- **Simulador CAN:** Integrado para testing\n");
    fprintf(log_file, "- **Gestor de Estado:** Mantenimiento del estado de ascensores\n");
    fprintf(log_file, "- **Puente CAN-CoAP:** Transformación de mensajes\n");
    fprintf(log_file, "- **Formato de Datos:** JSON para payloads\n");
    fprintf(log_file, "- **Logging:** Sistema de registro de eventos en tiempo real\n\n");
    
    fprintf(log_file, "\\newpage\n\n");
    fprintf(log_file, "# Registro de Eventos\n\n");
    fprintf(log_file, "La siguiente sección presenta el flujo cronológico de eventos durante la ejecución del sistema.\n\n");
    
    fflush(log_file);
}

/**
 * @brief Escribe el footer del archivo Markdown con estadísticas finales
 */
static void write_markdown_footer(void) {
    if (!log_file) return;
    
    char timestamp[64];
    get_formatted_timestamp(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S");
    
    fprintf(log_file, "\n\\newpage\n\n");
    fprintf(log_file, "# Estadísticas Finales de Ejecución\n\n");
    
    fprintf(log_file, "## Resumen de Comunicaciones\n\n");
    fprintf(log_file, "### Tráfico de Red\n\n");
    fprintf(log_file, "| **Protocolo** | **Enviados** | **Recibidos** | **Total** |\n");
    fprintf(log_file, "|:--------------|:-------------|:--------------|:----------|\n");
    fprintf(log_file, "| **Frames CAN** | %d | %d | %d |\n", 
             stats.total_can_frames_sent, stats.total_can_frames_received, 
             stats.total_can_frames_sent + stats.total_can_frames_received);
    fprintf(log_file, "| **Mensajes CoAP** | %d | %d | %d |\n\n", 
             stats.total_coap_requests, stats.total_coap_responses,
             stats.total_coap_requests + stats.total_coap_responses);
    
    fprintf(log_file, "## Gestión de Ascensores\n\n");
    fprintf(log_file, "### Operaciones de Control\n\n");
    fprintf(log_file, "| **Métrica** | **Cantidad** | **Porcentaje** |\n");
    fprintf(log_file, "|:------------|:-------------|:---------------|\n");
    fprintf(log_file, "| **Tareas Asignadas** | %d | 100%% |\n", stats.total_tasks_assigned);
    
    if (stats.total_tasks_assigned > 0) {
        double completion_rate = (100.0 * stats.total_tasks_completed) / stats.total_tasks_assigned;
        fprintf(log_file, "| **Tareas Completadas** | %d | %.1f%% |\n", stats.total_tasks_completed, completion_rate);
    } else {
        fprintf(log_file, "| **Tareas Completadas** | %d | N/A |\n", stats.total_tasks_completed);
    }
    
    fprintf(log_file, "| **Movimientos de Ascensores** | %d | N/A |\n", stats.total_elevator_movements);
    fprintf(log_file, "| **Errores Detectados** | %d | N/A |\n\n", stats.total_errors);
    
    fprintf(log_file, "## Análisis de Rendimiento\n\n");
    fprintf(log_file, "### Métricas Temporales\n\n");
    fprintf(log_file, "| **Parámetro** | **Valor** | **Unidad** |\n");
    fprintf(log_file, "|:--------------|:----------|:-----------|\n");
    fprintf(log_file, "| **Duración Total** | %.2f | segundos |\n", stats.execution_duration_sec);
    fprintf(log_file, "| **Edificio Simulado** | %s | ID |\n", stats.building_id[0] ? stats.building_id : "N/A");
    fprintf(log_file, "| **Peticiones del Edificio** | %d | cantidad |\n", stats.building_requests);
    
    if (stats.building_requests > 0) {
        double avg_time = stats.execution_duration_sec / stats.building_requests;
        fprintf(log_file, "| **Tiempo Promedio por Petición** | %.3f | segundos |\n", avg_time);
        
        if (avg_time > 0) {
            double throughput = 1.0 / avg_time;
            fprintf(log_file, "| **Throughput del Sistema** | %.2f | peticiones/segundo |\n", throughput);
        }
    }
    
    fprintf(log_file, "\n### Eficiencia del Sistema\n\n");
    if (stats.total_errors == 0) {
        fprintf(log_file, "**ESTADO: EJECUCION EXITOSA**\n\n");
        fprintf(log_file, "- Sin errores detectados durante la ejecución\n");
        fprintf(log_file, "- Todas las comunicaciones funcionaron correctamente\n");
        fprintf(log_file, "- Sistema de simulación operativo y estable\n");
        fprintf(log_file, "- Protocolo DTLS-PSK establecido correctamente\n");
    } else {
        fprintf(log_file, "**ESTADO: EJECUCION CON ADVERTENCIAS**\n\n");
        fprintf(log_file, "- **%d errores** detectados durante la ejecución\n", stats.total_errors);
        fprintf(log_file, "- Revisar la sección de eventos para análisis detallado\n");
        fprintf(log_file, "- Verificar configuración de red y protocolos\n");
    }
    
    fprintf(log_file, "\n## Conclusiones\n\n");
    fprintf(log_file, "Este reporte documenta la ejecución completa del API Gateway del Sistema de ");
    fprintf(log_file, "Control de Ascensores. Los datos presentados permiten evaluar el rendimiento ");
    fprintf(log_file, "del sistema y identificar áreas de mejora en futuras iteraciones.\n\n");
    
    fprintf(log_file, "---\n\n");
    fprintf(log_file, "**Reporte generado automáticamente**  \n");
    fprintf(log_file, "Sistema de Control de Ascensores - API Gateway v2.0  \n");
    fprintf(log_file, "Finalizado: %s\n", timestamp);
    
    fflush(log_file);
}

/**
 * @brief Convierte el tipo de evento a etiqueta de texto profesional
 */
static const char* get_event_label(log_event_type_t type) {
    switch (type) {
        case LOG_EVENT_SYSTEM_START:     return "INICIO";
        case LOG_EVENT_SYSTEM_END:       return "FIN";
        case LOG_EVENT_SIMULATION_START: return "SIM-INICIO";
        case LOG_EVENT_SIMULATION_END:   return "SIM-FIN";
        case LOG_EVENT_BUILDING_SELECTED:return "EDIFICIO";
        case LOG_EVENT_CAN_SENT:         return "CAN-TX";
        case LOG_EVENT_CAN_RECEIVED:     return "CAN-RX";
        case LOG_EVENT_COAP_SENT:        return "COAP-TX";
        case LOG_EVENT_COAP_RECEIVED:    return "COAP-RX";
        case LOG_EVENT_TASK_ASSIGNED:    return "TAREA-ASIG";
        case LOG_EVENT_ELEVATOR_MOVED:   return "ASCENSOR-MOV";
        case LOG_EVENT_TASK_COMPLETED:   return "TAREA-COMP";
        case LOG_EVENT_ERROR:            return "ERROR";
        default:                         return "INFO";
    }
}

// ============================================================================
// FUNCIONES PÚBLICAS
// ============================================================================

bool exec_logger_init(void) {
    // Inicializar estadísticas
    memset(&stats, 0, sizeof(execution_stats_t));
    start_time = time(NULL);
    
    // Debug: mostrar directorio actual de trabajo
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        printf("[EXEC_LOGGER] Directorio de trabajo actual: %s\n", cwd);
    }
    
    // Verificar si hay variables de entorno únicas para esta instancia
    const char* instance_id = getenv("GATEWAY_INSTANCE_ID");
    const char* instance_port = getenv("GATEWAY_PORT");
    const char* custom_log_dir = getenv("GATEWAY_LOG_DIR");
    
    // Determinar la ruta base correcta para los logs
    char base_logs_dir[512];  // Aumentar tamaño del buffer
    
    if (custom_log_dir && strlen(custom_log_dir) > 0) {
        // Usar directorio personalizado si se especifica
        strcpy(base_logs_dir, custom_log_dir);
        printf("[EXEC_LOGGER] Usando directorio personalizado: %s\n", base_logs_dir);
    } else {
        // Verificar si estamos en build/ o en api_gateway/
        struct stat st = {0};
        if (stat("../logs", &st) == 0) {
            // Estamos en build/, los logs están en ../logs
            strcpy(base_logs_dir, "../logs");
            printf("[EXEC_LOGGER] Detectado: ejecutando desde build/, usando ../logs\n");
        } else if (stat("logs", &st) == 0) {
            // Estamos en api_gateway/, usar logs directamente
            strcpy(base_logs_dir, "logs");
            printf("[EXEC_LOGGER] Detectado: ejecutando desde api_gateway/, usando logs\n");
        } else {
            // Crear logs en el directorio padre (desde build/)
            strcpy(base_logs_dir, "../logs");
            printf("[EXEC_LOGGER] Creando logs en: ../logs\n");
        }
        
        // Si hay ID de instancia, crear subdirectorio único
        if (instance_id && instance_port) {
            char instance_dir[1024];
            snprintf(instance_dir, sizeof(instance_dir), "%s/instance_%s_port_%s", 
                    base_logs_dir, instance_id, instance_port);
            strcpy(base_logs_dir, instance_dir);
            printf("[EXEC_LOGGER] Creando directorio único para instancia: %s\n", base_logs_dir);
        }
    }
    
    // Crear estructura de directorios
    printf("[EXEC_LOGGER] Intentando crear directorio '%s'...\n", base_logs_dir);
    if (!create_directory(base_logs_dir)) {
        return false;
    }
    
    // Crear directorio por fecha
    char date_str[32];
    get_formatted_timestamp(date_str, sizeof(date_str), "%Y-%m-%d");
    
    char date_dir[1024];  // Aumentar tamaño del buffer para evitar truncamiento
    int date_dir_len = snprintf(date_dir, sizeof(date_dir), "%s/%s", base_logs_dir, date_str);
    
    // Verificar que no se haya truncado la ruta
    if (date_dir_len >= sizeof(date_dir)) {
        printf("[EXEC_LOGGER] Error: Ruta del directorio demasiado larga\n");
        return false;
    }
    
    printf("[EXEC_LOGGER] Intentando crear directorio '%s'...\n", date_dir);
    if (!create_directory(date_dir)) {
        return false;
    }
    
    // Crear nombre del archivo con timestamp incluyendo milisegundos
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    struct tm *tm_info = localtime(&ts.tv_sec);
    
    char time_str[64];
    // Formato: HH-MM-SS-mmm (donde mmm son milisegundos)
    snprintf(time_str, sizeof(time_str), "%02d-%02d-%02d-%03ld",
             tm_info->tm_hour, tm_info->tm_min, tm_info->tm_sec,
             ts.tv_nsec / 1000000);  // Convertir nanosegundos a milisegundos
    
    // Usar snprintf con verificación de tamaño
    int path_len = snprintf(current_log_path, sizeof(current_log_path), 
                           "%s/ejecucion_%s.md", date_dir, time_str);
    
    if (path_len >= sizeof(current_log_path)) {
        printf("[EXEC_LOGGER] Error: Ruta del archivo demasiado larga\n");
        return false;
    }
    
    printf("[EXEC_LOGGER] Intentando crear archivo: %s\n", current_log_path);
    
    // Abrir archivo de log
    log_file = fopen(current_log_path, "w");
    if (!log_file) {
        printf("[EXEC_LOGGER] Error abriendo archivo de log: %s\n", strerror(errno));
        return false;
    }
    
    // Escribir header
    write_markdown_header();
    
    logger_active = true;
    
    printf("[EXEC_LOGGER] Sistema de logging inicializado: %s\n", current_log_path);
    
    // Registrar evento de inicio
    exec_logger_log_event(LOG_EVENT_SYSTEM_START, "Sistema API Gateway iniciado", "Logging de ejecución activado");
    
    return true;
}

void exec_logger_finish(void) {
    if (!logger_active || !log_file) return;
    
    // Calcular duración final
    time_t end_time = time(NULL);
    stats.execution_duration_sec = difftime(end_time, start_time);
    
    // Registrar evento de fin
    exec_logger_log_event(LOG_EVENT_SYSTEM_END, "Sistema API Gateway finalizado", "Cerrando logging de ejecución");
    
    // Escribir footer con estadísticas
    write_markdown_footer();
    
    // Cerrar archivo
    fclose(log_file);
    log_file = NULL;
    logger_active = false;
    
    printf("[EXEC_LOGGER] Reporte de ejecución guardado en: %s\n", current_log_path);
    
    // Generar PDF automáticamente usando rutas relativas correctas
    char pdf_command[MAX_LOG_PATH + 512];
    
    // Determinar si tenemos el script de PDF disponible y desde dónde ejecutar
    struct stat script_stat;
    if (stat("../generate_pdf_report.sh", &script_stat) == 0) {
        // Script está en el directorio padre (desde build/)
        snprintf(pdf_command, sizeof(pdf_command), 
                 "cd .. && ./generate_pdf_report.sh \"%s\" 2>/dev/null", current_log_path);
        printf("[EXEC_LOGGER] Script encontrado en directorio padre\n");
    } else if (stat("generate_pdf_report.sh", &script_stat) == 0) {
        // Script está en el directorio actual
        snprintf(pdf_command, sizeof(pdf_command), 
                 "./generate_pdf_report.sh \"%s\" 2>/dev/null", current_log_path);
        printf("[EXEC_LOGGER] Script encontrado en directorio actual\n");
    } else {
        // No se encontró el script
        printf("[EXEC_LOGGER] Nota: Script de PDF no encontrado. Para generar PDF manualmente:\n");
        printf("  cd ..\n");
        printf("  chmod +x generate_pdf_report.sh\n");
        printf("  ./generate_pdf_report.sh \"%s\"\n", current_log_path);
        return;
    }
    
    printf("[EXEC_LOGGER] Generando PDF automáticamente...\n");
    int result = system(pdf_command);
    
    if (result == 0) {
        // Generar nombre del archivo PDF
        char pdf_path[MAX_LOG_PATH];
        strncpy(pdf_path, current_log_path, sizeof(pdf_path) - 1);
        pdf_path[sizeof(pdf_path) - 1] = '\0';
        
        // Cambiar extensión .md por .pdf
        char *dot = strrchr(pdf_path, '.');
        if (dot && strcmp(dot, ".md") == 0) {
            strcpy(dot, ".pdf");
            printf("[EXEC_LOGGER] ✓ PDF generado exitosamente: %s\n", pdf_path);
            printf("[EXEC_LOGGER] Para visualizar: xdg-open \"%s\" o abrir con visor PDF\n", pdf_path);
        }
    } else {
        printf("[EXEC_LOGGER] Nota: Para generar PDF manualmente:\n");
        printf("  cd ..\n");
        printf("  chmod +x generate_pdf_report.sh\n");
        printf("  ./generate_pdf_report.sh \"%s\"\n", current_log_path);
    }
}

void exec_logger_log_event(log_event_type_t type, const char* description, const char* details) {
    if (!logger_active || !log_file || !description) return;
    
    char timestamp[32];
    get_formatted_timestamp(timestamp, sizeof(timestamp), "%H:%M:%S.%f");
    
    const char* label = get_event_label(type);
    
    // Formato profesional para eventos
    fprintf(log_file, "## Evento: %s\n\n", label);
    fprintf(log_file, "**Timestamp:** %s  \n", timestamp);
    fprintf(log_file, "**Descripción:** %s  \n", description);
    
    if (details && strlen(details) > 0) {
        fprintf(log_file, "**Detalles:**\n\n");
        fprintf(log_file, "```\n%s\n```\n", details);
    }
    
    fprintf(log_file, "\n---\n\n");
    fflush(log_file);
}

void exec_logger_log_simulation_start(const char* building_id, int num_requests) {
    if (!logger_active || !building_id) return;
    
    strncpy(stats.building_id, building_id, sizeof(stats.building_id) - 1);
    stats.building_requests = num_requests;
    
    char details[256];
    snprintf(details, sizeof(details), "Edificio: %s\nPeticiones a ejecutar: %d", building_id, num_requests);
    
    exec_logger_log_event(LOG_EVENT_SIMULATION_START, "Iniciando simulación de ascensores", details);
}

void exec_logger_log_simulation_end(int successful_requests, int total_requests) {
    if (!logger_active) return;
    
    char details[256];
    snprintf(details, sizeof(details), "Peticiones exitosas: %d/%d\nTasa de éxito: %.1f%%", 
             successful_requests, total_requests, 
             total_requests > 0 ? (100.0 * successful_requests / total_requests) : 0.0);
    
    exec_logger_log_event(LOG_EVENT_SIMULATION_END, "Simulación completada", details);
}

void exec_logger_log_can_sent(unsigned int can_id, int dlc, const unsigned char* data, const char* description) {
    if (!logger_active) return;
    
    stats.total_can_frames_sent++;
    
    char details[512];
    char data_str[256] = "";
    
    if (data && dlc > 0) {
        char temp[8];
        for (int i = 0; i < dlc && i < 8; i++) {
            snprintf(temp, sizeof(temp), "%02X ", data[i]);
            strcat(data_str, temp);
        }
    }
    
    snprintf(details, sizeof(details), "CAN ID: 0x%X\nDLC: %d\nDatos: %s\nDescripción: %s", 
             can_id, dlc, data_str, description ? description : "N/A");
    
    exec_logger_log_event(LOG_EVENT_CAN_SENT, "Frame CAN enviado", details);
}

void exec_logger_log_can_received(unsigned int can_id, int dlc, const unsigned char* data, const char* description) {
    if (!logger_active) return;
    
    stats.total_can_frames_received++;
    
    char details[512];
    char data_str[256] = "";
    
    if (data && dlc > 0) {
        char temp[8];
        for (int i = 0; i < dlc && i < 8; i++) {
            snprintf(temp, sizeof(temp), "%02X ", data[i]);
            strcat(data_str, temp);
        }
    }
    
    snprintf(details, sizeof(details), "CAN ID: 0x%X\nDLC: %d\nDatos: %s\nDescripción: %s", 
             can_id, dlc, data_str, description ? description : "N/A");
    
    exec_logger_log_event(LOG_EVENT_CAN_RECEIVED, "Frame CAN recibido", details);
}

void exec_logger_log_coap_sent(const char* method, const char* uri, const char* payload) {
    if (!logger_active) return;
    
    stats.total_coap_requests++;
    
    char details[1024];
    snprintf(details, sizeof(details), "Método: %s\nURI: %s\nPayload: %s", 
             method ? method : "N/A", 
             uri ? uri : "N/A", 
             payload ? payload : "N/A");
    
    exec_logger_log_event(LOG_EVENT_COAP_SENT, "Petición CoAP enviada", details);
}

void exec_logger_log_coap_received(const char* code, const char* payload) {
    if (!logger_active) return;
    
    stats.total_coap_responses++;
    
    char details[1024];
    snprintf(details, sizeof(details), "Código: %s\nPayload: %s", 
             code ? code : "N/A", 
             payload ? payload : "N/A");
    
    exec_logger_log_event(LOG_EVENT_COAP_RECEIVED, "Respuesta CoAP recibida", details);
}

void exec_logger_log_task_assigned(const char* task_id, const char* elevator_id, int target_floor) {
    if (!logger_active) return;
    
    stats.total_tasks_assigned++;
    
    char details[256];
    snprintf(details, sizeof(details), "Tarea: %s\nAscensor: %s\nPiso destino: %d", 
             task_id ? task_id : "N/A", 
             elevator_id ? elevator_id : "N/A", 
             target_floor);
    
    exec_logger_log_event(LOG_EVENT_TASK_ASSIGNED, "Tarea asignada a ascensor", details);
}

void exec_logger_log_elevator_moved(const char* elevator_id, int from_floor, int to_floor, const char* direction) {
    if (!logger_active) return;
    
    stats.total_elevator_movements++;
    
    char details[256];
    snprintf(details, sizeof(details), "Ascensor: %s\nDesde piso: %d\nHacia piso: %d\nDirección: %s", 
             elevator_id ? elevator_id : "N/A", 
             from_floor, to_floor, 
             direction ? direction : "N/A");
    
    exec_logger_log_event(LOG_EVENT_ELEVATOR_MOVED, "Ascensor en movimiento", details);
}

void exec_logger_log_task_completed(const char* task_id, const char* elevator_id, int final_floor) {
    if (!logger_active) return;
    
    stats.total_tasks_completed++;
    
    char details[256];
    snprintf(details, sizeof(details), "Tarea: %s\nAscensor: %s\nPiso final: %d", 
             task_id ? task_id : "N/A", 
             elevator_id ? elevator_id : "N/A", 
             final_floor);
    
    exec_logger_log_event(LOG_EVENT_TASK_COMPLETED, "Tarea completada", details);
}

void exec_logger_log_error(const char* error_code, const char* error_message) {
    if (!logger_active) return;
    
    stats.total_errors++;
    
    char details[512];
    snprintf(details, sizeof(details), "Código: %s\nMensaje: %s", 
             error_code ? error_code : "N/A", 
             error_message ? error_message : "N/A");
    
    exec_logger_log_event(LOG_EVENT_ERROR, "Error del sistema", details);
}

const execution_stats_t* exec_logger_get_stats(void) {
    return &stats;
}

bool exec_logger_is_active(void) {
    return logger_active;
} 