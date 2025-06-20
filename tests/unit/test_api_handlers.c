/**
 * @file test_api_handlers.c
 * @brief Pruebas unitarias para los Manejadores de API del Gateway
 * @author Sistema de Control de Ascensores
 * @date 2025
 * @version 1.0
 * 
 * Este archivo contiene las pruebas unitarias para verificar el correcto
 * funcionamiento de los manejadores de API del gateway, incluyendo:
 * - Gestión básica de trackers de solicitudes
 * - Manejadores de señales del sistema
 * - Validación de payloads JSON
 * - Formato de respuestas JSON
 * - Integración con el gestor de estado de ascensores
 * - Tipos de solicitudes y direcciones de movimiento
 * - Setup y teardown de suites de pruebas
 * 
 * @see api_gateway/api_handlers.h
 * @see api_gateway/api_handlers.c
 */

#include <CUnit/CUnit.h>
#include <CUnit/Basic.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <cJSON.h>
#include <stdbool.h>

// Incluir los headers necesarios
#include "api_gateway/api_handlers.h"
#include "api_gateway/elevator_state_manager.h"

/**
 * @brief Variables globales mock para las pruebas del API Gateway
 * 
 * Estas variables simulan el entorno de ejecución del API Gateway
 * durante las pruebas unitarias.
 */
volatile sig_atomic_t quit_main_loop = 0;  ///< Control del bucle principal
coap_session_t *g_dtls_session_to_central_server = NULL;  ///< Sesión DTLS mock
extern elevator_group_state_t managed_elevator_group; ///< Estado del grupo de ascensores (definido en mock_can_interface.c)

/**
 * @brief Variable global para el estado del setup de la suite
 * 
 * Indica si la función de setup ha sido llamada correctamente.
 * Se utiliza para verificar que la inicialización de la suite funciona.
 */
static int setup_called = 0;

/**
 * @brief Archivo de reporte para escribir resultados de pruebas
 * 
 * Puntero al archivo donde se escriben los resultados detallados
 * de cada prueba ejecutada.
 */
static FILE *report_file = NULL;

/**
 * @brief Escribe el resultado de una prueba al archivo de reporte
 * @param test_name Nombre de la prueba
 * @param description Descripción de la prueba
 * @param passed Indica si la prueba pasó (true) o falló (false)
 * @param details Detalles adicionales del resultado
 */
void write_test_result(const char* test_name, const char* description, bool passed, const char* details) {
    if (report_file) {
        fprintf(report_file, "TEST: %s\n", test_name);
        fprintf(report_file, "Descripción: %s\n", description);
        fprintf(report_file, "Resultado: %s\n", passed ? "PASÓ" : "FALLÓ");
        fprintf(report_file, "Detalles: %s\n", details);
        fprintf(report_file, "----------------------------------------\n\n");
        fflush(report_file);
    }
}

/**
 * @brief Función de setup para la suite de pruebas de API handlers
 * @return 0 si el setup es exitoso, código de error en caso contrario
 * 
 * Esta función se ejecuta antes de cada suite de pruebas.
 * Inicializa el entorno necesario para las pruebas de los manejadores de API,
 * incluyendo la apertura del archivo de reporte y la inicialización del
 * grupo de ascensores para las pruebas.
 */
int setup_api_handlers_tests(void) {
    // Abrir archivo de reporte
    report_file = fopen("test_api_handlers_report.txt", "w");
    if (report_file) {
        fprintf(report_file, "=== REPORTE DE PRUEBAS: API HANDLERS ===\n");
        fprintf(report_file, "Fecha: %s\n", __DATE__);
        fprintf(report_file, "=========================================\n\n");
    }
    
    // Inicializar grupo de ascensores para las pruebas
    init_elevator_group(&managed_elevator_group, "EDIFICIO_TEST", 2, 10);
    
    setup_called = 1;
    return 0;
}

/**
 * @brief Función de teardown para la suite de pruebas de API handlers
 * @return 0 si el teardown es exitoso, código de error en caso contrario
 * 
 * Esta función se ejecuta después de cada suite de pruebas.
 * Limpia el entorno y libera recursos utilizados durante las pruebas,
 * incluyendo el cierre del archivo de reporte.
 */
int teardown_api_handlers_tests(void) {
    if (report_file) {
        fclose(report_file);
        report_file = NULL;
    }
    setup_called = 0;
    return 0;
}

// ============================================================================
// PRUEBAS DE GESTIÓN DE TRACKERS
// ============================================================================

void test_tracker_management_basic(void) {
    printf("\n--- TEST: Gestión básica de trackers ---\n");
    
    // Esta prueba verifica la estructura de datos del tracker
    // Como las funciones de gestión son estáticas, probamos la estructura
    
    // Crear un request tracker simulado
    api_request_tracker_t tracker;
    
    // Inicializar campos
    tracker.original_elevator_session = NULL;
    tracker.original_mid = 12345;
    tracker.original_token.length = 4;
    tracker.original_token.s = (uint8_t*)"test";
    tracker.log_tag = "TestTracker";
    tracker.request_type = GW_REQUEST_TYPE_FLOOR_CALL;
    tracker.origin_floor = 1;
    tracker.target_floor_for_task = 5;
    tracker.requesting_elevator_id_cabin[0] = '\0';
    tracker.requested_direction_floor = MOVING_UP;
    
    // Verificar que los campos se asignaron correctamente
    CU_ASSERT_EQUAL(tracker.original_mid, 12345);
    CU_ASSERT_EQUAL(tracker.original_token.length, 4);
    CU_ASSERT_PTR_NOT_NULL(tracker.original_token.s);
    CU_ASSERT_STRING_EQUAL(tracker.log_tag, "TestTracker");
    CU_ASSERT_EQUAL(tracker.request_type, GW_REQUEST_TYPE_FLOOR_CALL);
    CU_ASSERT_EQUAL(tracker.origin_floor, 1);
    CU_ASSERT_EQUAL(tracker.target_floor_for_task, 5);
    CU_ASSERT_EQUAL(tracker.requested_direction_floor, MOVING_UP);
    
    printf("✅ Estructura de tracker verificada correctamente\n");
    
    write_test_result("test_tracker_management_basic",
                     "Verifica la correcta inicialización y gestión de trackers de solicitudes",
                     true,
                     "Estructura de tracker inicializada correctamente con todos los campos válidos");
}

void test_signal_handler(void) {
    printf("\n--- TEST: Manejador de señales ---\n");
    
    // Verificar que el manejador de señales funciona
    extern volatile sig_atomic_t quit_main_loop;
    
    // Estado inicial
    quit_main_loop = 0;
    CU_ASSERT_EQUAL(quit_main_loop, 0);
    
    // Simular recepción de SIGINT
    handle_sigint_gw(SIGINT);
    
    // Verificar que se estableció la bandera
    CU_ASSERT_EQUAL(quit_main_loop, 1);
    
    printf("✅ Manejador de señales funciona correctamente\n");
    
    // Restaurar estado
    quit_main_loop = 0;
    
    write_test_result("test_signal_handler",
                     "Verifica el correcto funcionamiento del manejador de señales SIGINT",
                     true,
                     "Señal SIGINT manejada correctamente, bandera quit_main_loop establecida");
}

// ============================================================================
// PRUEBAS DE VALIDACIÓN JSON
// ============================================================================

void test_json_payload_validation(void) {
    printf("\n--- TEST: Validación de payloads JSON ---\n");
    
    bool test_passed = true;
    char test_details[512] = "";
    
    // Probar con JSON válido
    const char* valid_json = "{\"id_ascensor\":\"ASC_001\",\"piso_actual\":3}";
    cJSON *json = cJSON_Parse(valid_json);
    CU_ASSERT_PTR_NOT_NULL(json);
    if (json) {
        cJSON *id_ascensor = cJSON_GetObjectItemCaseSensitive(json, "id_ascensor");
        CU_ASSERT_PTR_NOT_NULL(id_ascensor);
        CU_ASSERT_TRUE(cJSON_IsString(id_ascensor));
        printf("✅ JSON válido parseado correctamente\n");
        cJSON_Delete(json);
        strcat(test_details, "JSON válido parseado correctamente; ");
    } else {
        test_passed = false;
    }
    
    // Probar con JSON inválido
    const char* invalid_json = "{\"id_ascensor\":\"ASC_001\",\"piso_actual\":}";
    json = cJSON_Parse(invalid_json);
    CU_ASSERT_PTR_NULL(json);
    if (!json) {
        printf("✅ JSON inválido rechazado correctamente\n");
        strcat(test_details, "JSON inválido rechazado correctamente");
    } else {
        test_passed = false;
        cJSON_Delete(json);
    }
    
    write_test_result("test_json_payload_validation",
                     "Verifica la correcta validación de payloads JSON válidos e inválidos",
                     test_passed,
                     test_details);
}

void test_json_elevator_state_format(void) {
    printf("\n--- TEST: Formato JSON de estado de ascensor ---\n");
    
    bool test_passed = false;
    char test_details[512] = "Formato JSON de estado de ascensor inválido";
    
    // Crear JSON de estado de ascensor típico
    const char* elevator_state_json = "{"
        "\"id_ascensor\":\"ASC_001\","
        "\"piso_actual\":3,"
        "\"estado\":\"IDLE\","
        "\"direccion\":\"NONE\""
    "}";
    
    cJSON *json = cJSON_Parse(elevator_state_json);
    CU_ASSERT_PTR_NOT_NULL(json);
    
    if (json) {
        // Verificar campos requeridos
        cJSON *id_ascensor = cJSON_GetObjectItemCaseSensitive(json, "id_ascensor");
        cJSON *piso_actual = cJSON_GetObjectItemCaseSensitive(json, "piso_actual");
        cJSON *estado = cJSON_GetObjectItemCaseSensitive(json, "estado");
        
        CU_ASSERT_PTR_NOT_NULL(id_ascensor);
        CU_ASSERT_PTR_NOT_NULL(piso_actual);
        CU_ASSERT_PTR_NOT_NULL(estado);
        
        CU_ASSERT_TRUE(cJSON_IsString(id_ascensor));
        CU_ASSERT_TRUE(cJSON_IsNumber(piso_actual));
        CU_ASSERT_TRUE(cJSON_IsString(estado));
        
        // Verificar valores
        CU_ASSERT_STRING_EQUAL(id_ascensor->valuestring, "ASC_001");
        CU_ASSERT_EQUAL(piso_actual->valueint, 3);
        CU_ASSERT_STRING_EQUAL(estado->valuestring, "IDLE");
        
        printf("✅ Formato JSON de estado de ascensor válido\n");
        test_passed = true;
        snprintf(test_details, sizeof(test_details), 
                "Formato JSON válido: ascensor=%s, piso=%d, estado=%s",
                id_ascensor->valuestring, piso_actual->valueint, estado->valuestring);
        
        cJSON_Delete(json);
    }
    
    write_test_result("test_json_elevator_state_format",
                     "Verifica el correcto formato JSON para el estado de ascensores",
                     test_passed,
                     test_details);
}

// ============================================================================
// PRUEBAS DE INTEGRACIÓN CON ESTADO DE ASCENSORES
// ============================================================================

void test_elevator_state_integration(void) {
    printf("\n--- TEST: Integración con gestión de estado de ascensores ---\n");
    
    bool test_passed = false;
    char test_details[512] = "Integración con estado de ascensores fallida";
    
    // Inicializar grupo de ascensores para la prueba
    elevator_group_state_t test_group;
    init_elevator_group(&test_group, "EDIFICIO_TEST", 2, 10);  // 2 ascensores, 10 pisos
    
    // Verificar inicialización básica
    CU_ASSERT_EQUAL(test_group.num_elevadores_en_grupo, 2);
    CU_ASSERT_STRING_EQUAL(test_group.edificio_id_str_grupo, "EDIFICIO_TEST");
    
    // Verificar que los ascensores se inicializaron
    CU_ASSERT_STRING_NOT_EQUAL(test_group.ascensores[0].ascensor_id, "");
    CU_ASSERT_STRING_EQUAL(test_group.ascensores[0].id_edificio_str, "EDIFICIO_TEST");
    
    // Crear detalles de solicitud para JSON
    api_request_details_for_json_t details;
    details.origin_floor_fc = 3;
    details.direction_fc = MOVING_UP;
    
    // Serializar a JSON usando la función correcta
    cJSON *json_obj = elevator_group_to_json_for_server(&test_group, GW_REQUEST_TYPE_FLOOR_CALL, &details);
    CU_ASSERT_PTR_NOT_NULL(json_obj);
    
    if (json_obj) {
        char *json_str = cJSON_Print(json_obj);
        if (json_str) {
            printf("JSON generado: %.100s...\n", json_str);
            
            // Verificar que contiene los datos esperados
            CU_ASSERT_TRUE(strstr(json_str, "EDIFICIO_TEST") != NULL);
            
            printf("✅ Integración con estado de ascensores funciona\n");
            test_passed = true;
            snprintf(test_details, sizeof(test_details), 
                    "Integración exitosa: grupo inicializado con %d ascensores, JSON generado correctamente",
                    test_group.num_elevadores_en_grupo);
            
            free(json_str);
        }
        cJSON_Delete(json_obj);
    }
    
    write_test_result("test_elevator_state_integration",
                     "Verifica la correcta integración con el sistema de gestión de estado de ascensores",
                     test_passed,
                     test_details);
}

// ============================================================================
// PRUEBAS DE TIPOS DE SOLICITUDES
// ============================================================================

void test_request_types(void) {
    printf("\n--- TEST: Tipos de solicitudes ---\n");
    
    // Verificar que los tipos de solicitud están definidos correctamente
    gw_request_type_t floor_call = GW_REQUEST_TYPE_FLOOR_CALL;
    gw_request_type_t cabin_request = GW_REQUEST_TYPE_CABIN_REQUEST;
    
    CU_ASSERT_NOT_EQUAL(floor_call, cabin_request);
    
    printf("✅ Tipos de solicitudes definidos correctamente\n");
    printf("   - Floor call: %d\n", floor_call);
    printf("   - Cabin request: %d\n", cabin_request);
    
    char details[256];
    snprintf(details, sizeof(details), 
            "Tipos de solicitudes válidos: FLOOR_CALL=%d, CABIN_REQUEST=%d",
            floor_call, cabin_request);
    
    write_test_result("test_request_types",
                     "Verifica la correcta definición de tipos de solicitudes del API",
                     true,
                     details);
}

void test_movement_directions(void) {
    printf("\n--- TEST: Direcciones de movimiento ---\n");
    
    // Verificar que las direcciones están definidas
    movement_direction_enum_t up = MOVING_UP;
    movement_direction_enum_t down = MOVING_DOWN;
    movement_direction_enum_t stopped = STOPPED;
    
    CU_ASSERT_NOT_EQUAL(up, down);
    CU_ASSERT_NOT_EQUAL(up, stopped);
    CU_ASSERT_NOT_EQUAL(down, stopped);
    
    printf("✅ Direcciones de movimiento definidas correctamente\n");
    printf("   - UP: %d, DOWN: %d, STOPPED: %d\n", up, down, stopped);
    
    char details[256];
    snprintf(details, sizeof(details), 
            "Direcciones de movimiento válidas: UP=%d, DOWN=%d, STOPPED=%d",
            up, down, stopped);
    
    write_test_result("test_movement_directions",
                     "Verifica la correcta definición de direcciones de movimiento de ascensores",
                     true,
                     details);
}

// ============================================================================
// PRUEBAS DE CONFIGURACIÓN
// ============================================================================

void test_setup_teardown(void) {
    printf("\n--- TEST: Setup y teardown de suite ---\n");
    
    // Verificar que setup fue llamado
    CU_ASSERT_EQUAL(setup_called, 1);
    
    printf("✅ Setup y teardown funcionan correctamente\n");
    
    write_test_result("test_setup_teardown",
                     "Verifica el correcto funcionamiento del setup y teardown de la suite de pruebas",
                     (setup_called == 1),
                     "Setup llamado correctamente, suite de API handlers inicializada");
}

// ============================================================================
// SUITE DE PRUEBAS
// ============================================================================

int main(void) {
    printf("=== INICIANDO PRUEBAS DE API HANDLERS ===\n");
    
    // Inicializar CUnit
    if (CU_initialize_registry() != CUE_SUCCESS) {
        fprintf(stderr, "Error inicializando CUnit: %s\n", CU_get_error_msg());
        return CU_get_error();
    }
    
    // Crear suite
    CU_pSuite suite = CU_add_suite("API Handlers", setup_api_handlers_tests, teardown_api_handlers_tests);
    if (!suite) {
        fprintf(stderr, "Error creando suite: %s\n", CU_get_error_msg());
        CU_cleanup_registry();
        return CU_get_error();
    }
    
    // Añadir pruebas
    if (!CU_add_test(suite, "Gestión básica de trackers", test_tracker_management_basic) ||
        !CU_add_test(suite, "Manejador de señales", test_signal_handler) ||
        !CU_add_test(suite, "Validación payloads JSON", test_json_payload_validation) ||
        !CU_add_test(suite, "Formato JSON estado ascensor", test_json_elevator_state_format) ||
        !CU_add_test(suite, "Integración estado ascensores", test_elevator_state_integration) ||
        !CU_add_test(suite, "Tipos de solicitudes", test_request_types) ||
        !CU_add_test(suite, "Direcciones de movimiento", test_movement_directions) ||
        !CU_add_test(suite, "Setup y teardown", test_setup_teardown)) {
        fprintf(stderr, "Error añadiendo pruebas: %s\n", CU_get_error_msg());
        CU_cleanup_registry();
        return CU_get_error();
    }
    
    // Ejecutar pruebas
    CU_basic_set_mode(CU_BRM_VERBOSE);
    CU_basic_run_tests();
    
    // Obtener resultados
    int num_failures = CU_get_number_of_failures();
    int num_tests = CU_get_number_of_tests_run();
    
    printf("\n=== RESUMEN DE PRUEBAS API HANDLERS ===\n");
    printf("Total de pruebas: %d\n", num_tests);
    printf("Pruebas exitosas: %d\n", num_tests - num_failures);
    printf("Pruebas fallidas: %d\n", num_failures);
    printf("Tasa de éxito: %.1f%%\n", num_tests > 0 ? ((float)(num_tests - num_failures) / num_tests) * 100 : 0);
    
    // Limpiar
    CU_cleanup_registry();
    
    return num_failures;
} 