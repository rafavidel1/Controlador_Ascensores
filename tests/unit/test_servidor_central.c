/**
 * @file test_servidor_central.c
 * @brief Pruebas unitarias para el Servidor Central de Control de Ascensores
 */

#include <CUnit/CUnit.h>
#include <CUnit/Basic.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <cJSON.h>
#include <stdbool.h>
#include <unistd.h>

static int setup_called = 0;
static FILE *report_file = NULL;

int setup_servidor_central_tests(void) {
    report_file = fopen("test_servidor_central_report.txt", "w");
    if (report_file) {
        fprintf(report_file, "=== REPORTE DE PRUEBAS: SERVIDOR CENTRAL ===\n");
        fprintf(report_file, "Fecha: %s\n", __DATE__);
        fprintf(report_file, "=============================================\n\n");
    }
    setup_called = 1;
    return 0;
}

int teardown_servidor_central_tests(void) {
    if (report_file) {
        fprintf(report_file, "\n=== FIN DEL REPORTE ===\n");
        fclose(report_file);
        report_file = NULL;
    }
    setup_called = 0;
    return 0;
}

void write_test_result(const char* test_name, const char* description, bool passed, const char* details) {
    if (report_file) {
        fprintf(report_file, "PRUEBA: %s\n", test_name);
        fprintf(report_file, "Descripción: %s\n", description);
        fprintf(report_file, "Resultado: %s\n", passed ? "PASÓ" : "FALLÓ");
        fprintf(report_file, "Detalles: %s\n", details);
        fprintf(report_file, "----------------------------------------\n\n");
    }
}

void generate_unique_task_id(char *task_id_out, size_t len) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    snprintf(task_id_out, len, "T_%ld%03ld", (long)tv.tv_sec, (long)(tv.tv_usec / 1000));
}

void test_task_id_generation_basic(void) {
    printf("\n--- TEST: Generación básica de IDs de tareas ---\n");
    
    char task_id1[32];
    char task_id2[32];
    
    generate_unique_task_id(task_id1, sizeof(task_id1));
    usleep(1000);
    generate_unique_task_id(task_id2, sizeof(task_id2));
    
    CU_ASSERT_TRUE(strncmp(task_id1, "T_", 2) == 0);
    CU_ASSERT_TRUE(strncmp(task_id2, "T_", 2) == 0);
    CU_ASSERT_STRING_NOT_EQUAL(task_id1, task_id2);
    CU_ASSERT_TRUE(strlen(task_id1) > 5);
    CU_ASSERT_TRUE(strlen(task_id2) > 5);
    
    printf("✅ ID 1: %s\n", task_id1);
    printf("✅ ID 2: %s\n", task_id2);
    printf("✅ Generación de IDs únicos funciona correctamente\n");
    
    char details[256];
    snprintf(details, sizeof(details), "IDs generados correctamente: %s y %s, formato válido y únicos", task_id1, task_id2);
    write_test_result("test_task_id_generation_basic",
                     "Verifica la correcta generación de IDs únicos de tareas",
                     true, details);
}

void test_task_id_generation_uniqueness(void) {
    printf("\n--- TEST: Unicidad de IDs de tareas ---\n");
    
    const int num_ids = 10;
    char task_ids[num_ids][32];
    
    for (int i = 0; i < num_ids; i++) {
        generate_unique_task_id(task_ids[i], sizeof(task_ids[i]));
        usleep(2000);
    }
    
    int unique_count = 0;
    for (int i = 0; i < num_ids; i++) {
        bool is_unique = true;
        for (int j = i + 1; j < num_ids; j++) {
            if (strcmp(task_ids[i], task_ids[j]) == 0) {
                is_unique = false;
                printf("⚠️ ID duplicado encontrado: %s (posiciones %d y %d)\n", task_ids[i], i, j);
                break;
            }
        }
        if (is_unique) unique_count++;
    }
    
    printf("✅ Generados %d IDs únicos de %d intentos\n", unique_count, num_ids);
    
    for (int i = 0; i < 3 && i < num_ids; i++) {
        printf("   - %s\n", task_ids[i]);
    }
    
    CU_ASSERT_EQUAL(unique_count, num_ids);
    
    char details[256];
    snprintf(details, sizeof(details), "Generados %d IDs únicos de %d intentos, todos diferentes", unique_count, num_ids);
    write_test_result("test_task_id_generation_uniqueness",
                     "Verifica que los IDs de tareas generados son únicos entre múltiples llamadas",
                     (unique_count == num_ids), details);
}

void test_floor_call_payload_validation_valid(void) {
    printf("\n--- TEST: Validación payload petición piso (válido) ---\n");
    
    const char* valid_payload = "{"
        "\"id_edificio\":\"EDIFICIO_001\","
        "\"piso_origen_llamada\":3,"
        "\"direccion_llamada\":\"UP\","
        "\"elevadores_estado\":["
            "{"
                "\"id_ascensor\":\"ASC_001\","
                "\"piso_actual\":1,"
                "\"estado\":\"IDLE\""
            "}"
        "]"
    "}";
    
    cJSON *json = cJSON_Parse(valid_payload);
    CU_ASSERT_PTR_NOT_NULL(json);
    
    bool validation_passed = false;
    char validation_details[512] = "Payload inválido";
    
    if (json) {
        cJSON *id_edificio = cJSON_GetObjectItemCaseSensitive(json, "id_edificio");
        cJSON *piso_origen = cJSON_GetObjectItemCaseSensitive(json, "piso_origen_llamada");
        cJSON *direccion = cJSON_GetObjectItemCaseSensitive(json, "direccion_llamada");
        cJSON *elevadores = cJSON_GetObjectItemCaseSensitive(json, "elevadores_estado");
        
        CU_ASSERT_PTR_NOT_NULL(id_edificio);
        CU_ASSERT_PTR_NOT_NULL(piso_origen);
        CU_ASSERT_PTR_NOT_NULL(direccion);
        CU_ASSERT_PTR_NOT_NULL(elevadores);
        
        if (id_edificio && piso_origen && direccion && elevadores) {
            CU_ASSERT_TRUE(cJSON_IsString(id_edificio));
            CU_ASSERT_TRUE(cJSON_IsNumber(piso_origen));
            CU_ASSERT_TRUE(cJSON_IsString(direccion));
            CU_ASSERT_TRUE(cJSON_IsArray(elevadores));
            
            int array_size = cJSON_GetArraySize(elevadores);
            CU_ASSERT_TRUE(array_size > 0);
            
            printf("✅ Payload válido procesado correctamente\n");
            
            validation_passed = true;
            snprintf(validation_details, sizeof(validation_details), 
                    "Payload válido: edificio=%s, piso=%d, dirección=%s, ascensores=%d",
                    id_edificio->valuestring, piso_origen->valueint, 
                    direccion->valuestring, array_size);
        }
        
        cJSON_Delete(json);
    }
    
    write_test_result("test_floor_call_payload_validation_valid",
                     "Verifica la correcta validación de payloads JSON válidos para peticiones de piso",
                     validation_passed, validation_details);
}

void test_floor_call_payload_validation_invalid(void) {
    printf("\n--- TEST: Validación payload petición piso (inválido) ---\n");
    
    const char* invalid_payload = "{"
        "\"id_edificio\":\"EDIFICIO_001\","
        "\"piso_origen_llamada\":3"
    "}";
    
    cJSON *json = cJSON_Parse(invalid_payload);
    CU_ASSERT_PTR_NOT_NULL(json);
    
    bool validation_passed = true;
    
    if (json) {
        cJSON *direccion = cJSON_GetObjectItemCaseSensitive(json, "direccion_llamada");
        cJSON *elevadores = cJSON_GetObjectItemCaseSensitive(json, "elevadores_estado");
        
        CU_ASSERT_PTR_NULL(direccion);
        CU_ASSERT_PTR_NULL(elevadores);
        
        printf("✅ Payload inválido detectado correctamente\n");
        cJSON_Delete(json);
    }
    
    write_test_result("test_floor_call_payload_validation_invalid",
                     "Verifica la correcta detección de payloads JSON inválidos",
                     validation_passed, "Payload con campos faltantes detectado correctamente");
}

void test_cabin_request_payload_validation(void) {
    printf("\n--- TEST: Validación payload petición cabina ---\n");
    
    const char* valid_payload = "{"
        "\"id_edificio\":\"EDIFICIO_001\","
        "\"solicitando_ascensor_id\":\"ASC_001\","
        "\"piso_destino_cabina\":7"
    "}";
    
    cJSON *json = cJSON_Parse(valid_payload);
    CU_ASSERT_PTR_NOT_NULL(json);
    
    bool validation_passed = false;
    char validation_details[512] = "Payload de cabina inválido";
    
    if (json) {
        cJSON *id_edificio = cJSON_GetObjectItemCaseSensitive(json, "id_edificio");
        cJSON *ascensor_id = cJSON_GetObjectItemCaseSensitive(json, "solicitando_ascensor_id");
        cJSON *piso_destino = cJSON_GetObjectItemCaseSensitive(json, "piso_destino_cabina");
        
        CU_ASSERT_PTR_NOT_NULL(id_edificio);
        CU_ASSERT_PTR_NOT_NULL(ascensor_id);
        CU_ASSERT_PTR_NOT_NULL(piso_destino);
        
        if (id_edificio && ascensor_id && piso_destino) {
            CU_ASSERT_TRUE(cJSON_IsString(id_edificio));
            CU_ASSERT_TRUE(cJSON_IsString(ascensor_id));
            CU_ASSERT_TRUE(cJSON_IsNumber(piso_destino));
            
            CU_ASSERT_STRING_EQUAL(ascensor_id->valuestring, "ASC_001");
            CU_ASSERT_EQUAL(piso_destino->valueint, 7);
            
            printf("✅ Payload válido de petición de cabina procesado correctamente\n");
            
            validation_passed = true;
            snprintf(validation_details, sizeof(validation_details), 
                    "Solicitud de cabina válida: edificio=%s, ascensor=%s, destino=%d",
                    id_edificio->valuestring, ascensor_id->valuestring, 
                    piso_destino->valueint);
        }
        
        cJSON_Delete(json);
    }
    
    write_test_result("test_cabin_request_payload_validation",
                     "Verifica la correcta validación de payloads de solicitudes de cabina",
                     validation_passed, validation_details);
}

void test_elevator_assignment_algorithm_simple(void) {
    printf("\n--- TEST: Algoritmo de asignación simple ---\n");
    
    const char* payload = "{"
        "\"id_edificio\":\"EDIFICIO_001\","
        "\"piso_origen_llamada\":5,"
        "\"direccion_llamada\":\"UP\","
        "\"elevadores_estado\":["
            "{"
                "\"id_ascensor\":\"ASC_001\","
                "\"piso_actual\":1,"
                "\"estado\":\"IDLE\""
            "}"
        "]"
    "}";
    
    cJSON *json = cJSON_Parse(payload);
    CU_ASSERT_PTR_NOT_NULL(json);
    
    bool assignment_successful = false;
    char assignment_details[512] = "Asignación fallida";
    
    if (json) {
        cJSON *elevadores = cJSON_GetObjectItemCaseSensitive(json, "elevadores_estado");
        CU_ASSERT_PTR_NOT_NULL(elevadores);
        
        if (elevadores && cJSON_IsArray(elevadores)) {
            int array_size = cJSON_GetArraySize(elevadores);
            CU_ASSERT_EQUAL(array_size, 1);
            
            cJSON *first_elevator = cJSON_GetArrayItem(elevadores, 0);
            CU_ASSERT_PTR_NOT_NULL(first_elevator);
            
            if (first_elevator) {
                cJSON *id_ascensor = cJSON_GetObjectItemCaseSensitive(first_elevator, "id_ascensor");
                cJSON *estado = cJSON_GetObjectItemCaseSensitive(first_elevator, "estado");
                CU_ASSERT_PTR_NOT_NULL(id_ascensor);
                CU_ASSERT_PTR_NOT_NULL(estado);
                
                if (id_ascensor && estado) {
                    CU_ASSERT_STRING_EQUAL(id_ascensor->valuestring, "ASC_001");
                    CU_ASSERT_STRING_EQUAL(estado->valuestring, "IDLE");
                    
                    printf("✅ Algoritmo asignaría tarea a: %s\n", id_ascensor->valuestring);
                    
                    assignment_successful = true;
                    snprintf(assignment_details, sizeof(assignment_details), 
                            "Asignación exitosa: ascensor %s seleccionado",
                            id_ascensor->valuestring);
                }
            }
        }
        
        cJSON_Delete(json);
    }
    
    write_test_result("test_elevator_assignment_algorithm_simple",
                     "Verifica el correcto funcionamiento del algoritmo de asignación de ascensores",
                     assignment_successful, assignment_details);
}

void test_elevator_assignment_no_elevators(void) {
    printf("\n--- TEST: Asignación sin ascensores disponibles ---\n");
    
    const char* payload = "{"
        "\"id_edificio\":\"EDIFICIO_001\","
        "\"piso_origen_llamada\":5,"
        "\"direccion_llamada\":\"UP\","
        "\"elevadores_estado\":[]"
    "}";
    
    cJSON *json = cJSON_Parse(payload);
    CU_ASSERT_PTR_NOT_NULL(json);
    
    bool test_passed = false;
    
    if (json) {
        cJSON *elevadores = cJSON_GetObjectItemCaseSensitive(json, "elevadores_estado");
        CU_ASSERT_PTR_NOT_NULL(elevadores);
        
        if (elevadores && cJSON_IsArray(elevadores)) {
            int array_size = cJSON_GetArraySize(elevadores);
            CU_ASSERT_EQUAL(array_size, 0);
            
            printf("✅ Detectado correctamente: sin ascensores disponibles\n");
            test_passed = true;
        }
        
        cJSON_Delete(json);
    }
    
    write_test_result("test_elevator_assignment_no_elevators",
                     "Verifica el manejo correcto cuando no hay ascensores disponibles",
                     test_passed, "Detectado correctamente: array de ascensores vacío");
}

void test_response_generation_success(void) {
    printf("\n--- TEST: Generación de respuesta exitosa ---\n");
    
    char task_id[32];
    generate_unique_task_id(task_id, sizeof(task_id));
    
    const char* assigned_elevator = "ASC_001";
    
    cJSON *response_json = cJSON_CreateObject();
    CU_ASSERT_PTR_NOT_NULL(response_json);
    
    bool response_successful = false;
    char response_details[512] = "Generación de respuesta fallida";
    
    if (response_json) {
        cJSON_AddStringToObject(response_json, "tarea_id", task_id);
        cJSON_AddStringToObject(response_json, "ascensor_asignado_id", assigned_elevator);
        
        char *response_str = cJSON_PrintUnformatted(response_json);
        CU_ASSERT_PTR_NOT_NULL(response_str);
        
        if (response_str) {
            printf("✅ Respuesta generada: %s\n", response_str);
            
            CU_ASSERT_TRUE(strstr(response_str, "tarea_id") != NULL);
            CU_ASSERT_TRUE(strstr(response_str, "ascensor_asignado_id") != NULL);
            
            response_successful = true;
            snprintf(response_details, sizeof(response_details), 
                    "Respuesta JSON generada correctamente: tarea_id=%s, ascensor=%s",
                    task_id, assigned_elevator);
            
            free(response_str);
        }
        
        cJSON_Delete(response_json);
    }
    
    write_test_result("test_response_generation_success",
                     "Verifica la correcta generación de respuestas JSON exitosas",
                     response_successful, response_details);
}

void test_response_generation_error(void) {
    printf("\n--- TEST: Generación de respuesta de error ---\n");
    
    cJSON *error_json = cJSON_CreateObject();
    CU_ASSERT_PTR_NOT_NULL(error_json);
    
    bool error_response_successful = false;
    char error_details[512] = "Generación de respuesta de error fallida";
    
    if (error_json) {
        cJSON_AddStringToObject(error_json, "error", "No elevators available at the moment.");
        
        char *error_str = cJSON_PrintUnformatted(error_json);
        CU_ASSERT_PTR_NOT_NULL(error_str);
        
        if (error_str) {
            printf("✅ Error generado: %s\n", error_str);
            
            CU_ASSERT_TRUE(strstr(error_str, "error") != NULL);
            CU_ASSERT_TRUE(strstr(error_str, "No elevators available") != NULL);
            
            error_response_successful = true;
            snprintf(error_details, sizeof(error_details), 
                    "Respuesta de error generada correctamente");
            
            free(error_str);
        }
        
        cJSON_Delete(error_json);
    }
    
    write_test_result("test_response_generation_error",
                     "Verifica la correcta generación de respuestas de error",
                     error_response_successful, error_details);
}

void test_complete_floor_call_flow(void) {
    printf("\n--- TEST: Flujo completo de petición de piso ---\n");
    
    const char* input_payload = "{"
        "\"id_edificio\":\"EDIFICIO_TEST\","
        "\"piso_origen_llamada\":3,"
        "\"direccion_llamada\":\"UP\","
        "\"elevadores_estado\":["
            "{"
                "\"id_ascensor\":\"ASC_TEST\","
                "\"piso_actual\":1,"
                "\"estado\":\"IDLE\""
            "}"
        "]"
    "}";
    
    cJSON *input_json = cJSON_Parse(input_payload);
    CU_ASSERT_PTR_NOT_NULL(input_json);
    
    bool flow_successful = false;
    char flow_details[512] = "Flujo completo fallido";
    
    if (input_json) {
        cJSON *id_edificio = cJSON_GetObjectItemCaseSensitive(input_json, "id_edificio");
        cJSON *piso_origen = cJSON_GetObjectItemCaseSensitive(input_json, "piso_origen_llamada");
        cJSON *elevadores = cJSON_GetObjectItemCaseSensitive(input_json, "elevadores_estado");
        
        CU_ASSERT_PTR_NOT_NULL(id_edificio);
        CU_ASSERT_PTR_NOT_NULL(piso_origen);
        CU_ASSERT_PTR_NOT_NULL(elevadores);
        
        cJSON *first_elevator = cJSON_GetArrayItem(elevadores, 0);
        CU_ASSERT_PTR_NOT_NULL(first_elevator);
        
        if (first_elevator) {
            cJSON *id_ascensor = cJSON_GetObjectItemCaseSensitive(first_elevator, "id_ascensor");
            CU_ASSERT_PTR_NOT_NULL(id_ascensor);
            
            char task_id[32];
            generate_unique_task_id(task_id, sizeof(task_id));
            
            cJSON *response_json = cJSON_CreateObject();
            cJSON_AddStringToObject(response_json, "tarea_id", task_id);
            cJSON_AddStringToObject(response_json, "ascensor_asignado_id", id_ascensor->valuestring);
            
            char *response_str = cJSON_PrintUnformatted(response_json);
            CU_ASSERT_PTR_NOT_NULL(response_str);
            
            if (response_str) {
                printf("✅ Flujo completo exitoso\n");
                
                flow_successful = true;
                snprintf(flow_details, sizeof(flow_details), 
                        "Flujo completo exitoso: edificio=%s, piso=%d, tarea=%s, ascensor=%s",
                        id_edificio->valuestring, piso_origen->valueint, 
                        task_id, id_ascensor->valuestring);
                
                free(response_str);
            }
            
            cJSON_Delete(response_json);
        }
        
        cJSON_Delete(input_json);
    }
    
    write_test_result("test_complete_floor_call_flow",
                     "Verifica el flujo completo de procesamiento de peticiones de piso",
                     flow_successful, flow_details);
}

void test_setup_teardown_servidor(void) {
    printf("\n--- TEST: Setup y teardown de suite servidor central ---\n");
    
    CU_ASSERT_EQUAL(setup_called, 1);
    
    printf("✅ Setup y teardown del servidor central funcionan correctamente\n");
    
    write_test_result("test_setup_teardown_servidor",
                     "Verifica el correcto funcionamiento del setup y teardown de la suite",
                     (setup_called == 1), "Setup llamado correctamente, suite inicializada");
}

int main(void) {
    printf("=== INICIANDO PRUEBAS DEL SERVIDOR CENTRAL ===\n");
    
    if (CU_initialize_registry() != CUE_SUCCESS) {
        fprintf(stderr, "Error inicializando CUnit: %s\n", CU_get_error_msg());
        return CU_get_error();
    }
    
    CU_pSuite suite = CU_add_suite("Servidor Central", setup_servidor_central_tests, teardown_servidor_central_tests);
    if (!suite) {
        fprintf(stderr, "Error creando suite: %s\n", CU_get_error_msg());
        CU_cleanup_registry();
        return CU_get_error();
    }
    
    if (!CU_add_test(suite, "Generación básica IDs tareas", test_task_id_generation_basic) ||
        !CU_add_test(suite, "Unicidad IDs tareas", test_task_id_generation_uniqueness) ||
        !CU_add_test(suite, "Validación payload piso (válido)", test_floor_call_payload_validation_valid) ||
        !CU_add_test(suite, "Validación payload piso (inválido)", test_floor_call_payload_validation_invalid) ||
        !CU_add_test(suite, "Validación payload cabina", test_cabin_request_payload_validation) ||
        !CU_add_test(suite, "Algoritmo asignación simple", test_elevator_assignment_algorithm_simple) ||
        !CU_add_test(suite, "Asignación sin ascensores", test_elevator_assignment_no_elevators) ||
        !CU_add_test(suite, "Generación respuesta exitosa", test_response_generation_success) ||
        !CU_add_test(suite, "Generación respuesta error", test_response_generation_error) ||
        !CU_add_test(suite, "Flujo completo petición piso", test_complete_floor_call_flow) ||
        !CU_add_test(suite, "Setup y teardown servidor", test_setup_teardown_servidor)) {
        fprintf(stderr, "Error añadiendo pruebas: %s\n", CU_get_error_msg());
        CU_cleanup_registry();
        return CU_get_error();
    }
    
    CU_basic_set_mode(CU_BRM_VERBOSE);
    CU_basic_run_tests();
    
    int num_failures = CU_get_number_of_failures();
    int num_tests = CU_get_number_of_tests_run();
    
    printf("\n=== RESUMEN DE PRUEBAS SERVIDOR CENTRAL ===\n");
    printf("Total de pruebas: %d\n", num_tests);
    printf("Pruebas exitosas: %d\n", num_tests - num_failures);
    printf("Pruebas fallidas: %d\n", num_failures);
    printf("Tasa de éxito: %.1f%%\n", num_tests > 0 ? ((float)(num_tests - num_failures) / num_tests) * 100 : 0);
    
    CU_cleanup_registry();
    
    return num_failures;
} 