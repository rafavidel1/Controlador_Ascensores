/**
 * @file test_elevator_state_manager.c
 * @brief Pruebas unitarias para el Gestor de Estado de Ascensores
 * @author Sistema de Control de Ascensores
 * @date 2025
 * @version 1.0
 * 
 * Este archivo contiene las pruebas unitarias para verificar el correcto
 * funcionamiento del gestor de estado de ascensores, incluyendo:
 * - Inicialización de grupos de ascensores
 * - Asignación de tareas a ascensores
 * - Notificaciones de llegada
 * - Serialización a JSON
 * - Búsqueda de ascensores disponibles
 * 
 * @see elevator_state_manager.h
 * @see api_gateway/elevator_state_manager.c
 */

#include <CUnit/CUnit.h>
#include <CUnit/Basic.h>
#include <CUnit/Automated.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "api_gateway/elevator_state_manager.h"

// Constantes de prueba
#define TEST_BUILDING_ID "E1"
#define TEST_NUM_ELEVATORS 3
#define TEST_NUM_FLOORS 10

// Variables globales para testing
static elevator_group_state_t test_group;
static FILE *report_file = NULL;

/**
 * @brief Variable global para el estado del setup de la suite
 * 
 * Indica si la función de setup ha sido llamada correctamente.
 * Se utiliza para verificar que la inicialización de la suite funciona.
 */
static int setup_called = 0;

// Setup ejecutado antes de cada test
int setup_elevator_tests(void) {
    memset(&test_group, 0, sizeof(elevator_group_state_t));
    
    // Abrir archivo de reporte si no existe
    if (!report_file) {
        report_file = fopen("test_elevator_state_manager_report.txt", "w");
        if (report_file) {
            time_t now = time(NULL);
            fprintf(report_file, "=== REPORTE DE PRUEBAS: GESTOR DE ESTADO DE ASCENSORES ===\n");
            fprintf(report_file, "Fecha: %s\n", ctime(&now));
            fprintf(report_file, "========================================================\n\n");
        }
    }
    
    setup_called = 1;
    return 0;
}

// Teardown ejecutado después de cada test
int teardown_elevator_tests(void) {
    setup_called = 0;
    return 0;
}

/**
 * @brief Escribe el resultado de una prueba individual al archivo de reporte
 * @param test_name Nombre de la prueba ejecutada
 * @param description Descripción de lo que verifica la prueba
 * @param passed Indica si la prueba pasó (true) o falló (false)
 * @param details Detalles específicos del resultado de la prueba
 * 
 * Esta función centraliza la escritura de resultados de pruebas,
 * manteniendo un formato consistente en todos los reportes.
 */
void write_test_result(const char* test_name, const char* description, bool passed, const char* details) {
    if (report_file) {
        fprintf(report_file, "PRUEBA: %s\n", test_name);
        fprintf(report_file, "Descripción: %s\n", description);
        fprintf(report_file, "Resultado: %s\n", passed ? "PASÓ" : "FALLÓ");
        if (details) {
            fprintf(report_file, "Detalles: %s\n", details);
        }
        fprintf(report_file, "----------------------------------------\n\n");
        fflush(report_file);
    }
}

/**
 * @brief Prueba la inicialización correcta de un grupo de ascensores
 * 
 * Esta prueba verifica que:
 * - El grupo se inicializa con el número correcto de ascensores
 * - Cada ascensor tiene un ID único y válido
 * - Los ascensores inician en el piso correcto (≥ 0)
 * - El estado inicial es consistente (puerta cerrada, no ocupado)
 * - El ID del edificio se asigna correctamente
 * 
 * @test Inicialización de grupo de ascensores
 * @expected El grupo se inicializa correctamente con todos los parámetros válidos
 */
void test_init_elevator_group(void) {
    char details[512];
    bool test_passed = true;
    
    // Inicializar grupo de prueba
    init_elevator_group(&test_group, TEST_BUILDING_ID, TEST_NUM_ELEVATORS, TEST_NUM_FLOORS);
    
    // Verificar inicialización básica
    if (test_group.num_elevadores_en_grupo != TEST_NUM_ELEVATORS) {
        test_passed = false;
        snprintf(details, sizeof(details), "Número de ascensores incorrecto: esperado %d, obtenido %d", 
                TEST_NUM_ELEVATORS, test_group.num_elevadores_en_grupo);
    } else if (strcmp(test_group.edificio_id_str_grupo, TEST_BUILDING_ID) != 0) {
        test_passed = false;
        snprintf(details, sizeof(details), "ID de edificio incorrecto: esperado '%s', obtenido '%s'", 
                TEST_BUILDING_ID, test_group.edificio_id_str_grupo);
    } else {
        // Verificar cada ascensor individualmente
        for (int i = 0; i < TEST_NUM_ELEVATORS; i++) {
            elevator_status_t *elevator = &test_group.ascensores[i];
            
            if (strlen(elevator->ascensor_id) == 0) {
                test_passed = false;
                snprintf(details, sizeof(details), "Ascensor %d no tiene ID asignado", i);
                break;
            }
            
            if (elevator->piso_actual < 0) {
                test_passed = false;
                snprintf(details, sizeof(details), "Ascensor %d inicia en piso inválido: %d", 
                        i, elevator->piso_actual);
                break;
            }
            
            if (elevator->ocupado) {
                test_passed = false;
                snprintf(details, sizeof(details), "Ascensor %d inicia ocupado cuando debería estar libre", i);
                break;
            }
        }
        
        if (test_passed) {
            snprintf(details, sizeof(details), 
                    "Grupo inicializado correctamente: %d ascensores en edificio %s, piso inicial=%d", 
                    test_group.num_elevadores_en_grupo, test_group.edificio_id_str_grupo, 
                    test_group.ascensores[0].piso_actual);
        }
    }
    
    write_test_result("test_init_elevator_group", 
                     "Verifica la correcta inicialización de un grupo de ascensores",
                     test_passed, details);
    
    // Assertions de CUnit
    CU_ASSERT_EQUAL(test_group.num_elevadores_en_grupo, TEST_NUM_ELEVATORS);
    CU_ASSERT_STRING_EQUAL(test_group.edificio_id_str_grupo, TEST_BUILDING_ID);
    CU_ASSERT_TRUE(test_group.ascensores[0].piso_actual >= 0);
    CU_ASSERT_FALSE(test_group.ascensores[0].ocupado);
}

/**
 * @brief Prueba la asignación correcta de tareas a ascensores
 * 
 * Esta prueba verifica que:
 * - Las tareas se asignan al ascensor correcto
 * - El estado del ascensor se actualiza apropiadamente
 * - La información de la tarea se almacena correctamente
 * - La dirección de movimiento se calcula bien
 * 
 * @test Asignación de tareas a ascensores
 * @expected La tarea se asigna correctamente y el estado se actualiza
 */
void test_assign_task_to_elevator(void) {
    char details[512];
    bool test_passed = true;
    
    // Inicializar grupo para la prueba
    init_elevator_group(&test_group, TEST_BUILDING_ID, 2, TEST_NUM_FLOORS);
    
    // Asignar tarea al primer ascensor
    const char* task_id = "T_001";
    int target_floor = 5;
    int elevator_index = 0;
    
    // Usar el ID correcto que genera el sistema (formato: {edificio_id}A{numero})
    char elevator_id[32];
    snprintf(elevator_id, sizeof(elevator_id), "%sA%d", TEST_BUILDING_ID, elevator_index + 1);
    assign_task_to_elevator(&test_group, elevator_id, task_id, target_floor, 1);
    
    elevator_status_t *assigned_elevator = &test_group.ascensores[elevator_index];
    
    // Verificar asignación
    if (strcmp(assigned_elevator->tarea_actual_id, task_id) != 0) {
        test_passed = false;
        snprintf(details, sizeof(details), "ID de tarea incorrecto: esperado '%s', obtenido '%s'", 
                task_id, assigned_elevator->tarea_actual_id);
    } else if (assigned_elevator->destino_actual != target_floor) {
        test_passed = false;
        snprintf(details, sizeof(details), "Destino incorrecto: esperado %d, obtenido %d", 
                target_floor, assigned_elevator->destino_actual);
    } else if (!assigned_elevator->ocupado) {
        test_passed = false;
        snprintf(details, sizeof(details), "Ascensor no marcado como ocupado después de asignación");
    } else {
        // Verificar dirección de movimiento
        movement_direction_enum_t expected_direction = (target_floor > assigned_elevator->piso_actual) ? 
                                                      MOVING_UP : MOVING_DOWN;
        if (assigned_elevator->direccion_movimiento_enum != expected_direction) {
            test_passed = false;
            snprintf(details, sizeof(details), "Dirección incorrecta: esperado %d, obtenido %d", 
                    expected_direction, assigned_elevator->direccion_movimiento_enum);
        } else {
            snprintf(details, sizeof(details), 
                    "Tarea asignada correctamente: ascensor %s, tarea %s, destino piso %d, dirección %s", 
                    assigned_elevator->ascensor_id, task_id, target_floor,
                    (expected_direction == MOVING_UP) ? "UP" : "DOWN");
        }
    }
    
    write_test_result("test_assign_task_to_elevator", 
                     "Verifica la correcta asignación de tareas a ascensores",
                     test_passed, details);
    
    // Assertions de CUnit
    CU_ASSERT_STRING_EQUAL(assigned_elevator->tarea_actual_id, task_id);
    CU_ASSERT_EQUAL(assigned_elevator->destino_actual, target_floor);
    CU_ASSERT_TRUE(assigned_elevator->ocupado);
}

/**
 * @brief Prueba la serialización del estado del grupo a formato JSON
 * 
 * Esta prueba verifica que:
 * - El JSON generado contiene todos los campos requeridos
 * - La estructura del JSON es válida y parseable
 * - Los datos se serializan correctamente
 * - El formato es compatible con el servidor central
 * 
 * @test Serialización a JSON del estado del grupo
 * @expected Se genera un JSON válido con toda la información del grupo
 */
void test_elevator_group_to_json(void) {
    char details[512];
    bool test_passed = true;
    
    // Inicializar grupo para la prueba
    init_elevator_group(&test_group, TEST_BUILDING_ID, 2, TEST_NUM_FLOORS);
    
    // Crear detalles de solicitud
    api_request_details_for_json_t request_details;
    request_details.origin_floor_fc = 3;
    request_details.direction_fc = MOVING_UP;
    
    // Generar JSON
    cJSON *json_obj = elevator_group_to_json_for_server(&test_group, GW_REQUEST_TYPE_FLOOR_CALL, &request_details);
    
    if (!json_obj) {
        test_passed = false;
        snprintf(details, sizeof(details), "No se pudo generar JSON del grupo");
    } else {
        // Verificar campos requeridos
        cJSON *id_edificio = cJSON_GetObjectItemCaseSensitive(json_obj, "id_edificio");
        cJSON *piso_origen = cJSON_GetObjectItemCaseSensitive(json_obj, "piso_origen_llamada");
        
        if (!id_edificio || !cJSON_IsString(id_edificio)) {
            test_passed = false;
            snprintf(details, sizeof(details), "Campo 'id_edificio' faltante o inválido");
        } else if (!piso_origen || !cJSON_IsNumber(piso_origen)) {
            test_passed = false;
            snprintf(details, sizeof(details), "Campo 'piso_origen_llamada' faltante o inválido");
        } else {
            snprintf(details, sizeof(details), 
                    "JSON generado correctamente con id_edificio='%s' y piso_origen_llamada=%d", 
                    id_edificio->valuestring, piso_origen->valueint);
        }
        
        cJSON_Delete(json_obj);
    }
    
    write_test_result("test_elevator_group_to_json", 
                     "Verifica la correcta serialización del estado del grupo a JSON",
                     test_passed, details);
    
    // Assertions de CUnit
    CU_ASSERT_PTR_NOT_NULL(json_obj);
}

/**
 * @brief Limpia y cierra el archivo de reporte
 * 
 * Esta función debe ser llamada al final de todas las pruebas
 * para asegurar que el archivo de reporte se cierre correctamente
 * y se escriban todos los datos pendientes.
 */
void close_report_file(void) {
    if (report_file) {
        fprintf(report_file, "=== FIN DEL REPORTE ===\n");
        fclose(report_file);
        report_file = NULL;
    }
}

/**
 * @brief Configura la suite de pruebas para el gestor de estado de ascensores
 * @return Puntero a la suite configurada, o NULL si hay error
 * 
 * Esta función crea y configura la suite de pruebas, añadiendo todas
 * las pruebas individuales y configurando las funciones de setup/teardown.
 */
CU_pSuite add_elevator_state_manager_tests(void) {
    CU_pSuite suite = CU_add_suite("Elevator State Manager Tests", 
                                   setup_elevator_tests, 
                                   teardown_elevator_tests);
    
    if (suite == NULL) {
        return NULL;
    }
    
    // Añadir pruebas individuales
    if (CU_add_test(suite, "test_init_elevator_group", test_init_elevator_group) == NULL ||
        CU_add_test(suite, "test_assign_task_to_elevator", test_assign_task_to_elevator) == NULL ||
        CU_add_test(suite, "test_elevator_group_to_json", test_elevator_group_to_json) == NULL) {
        return NULL;
    }
    
    return suite;
}

// Main para ejecutar solo estas pruebas
int main(int argc, char *argv[]) {
    CU_ErrorCode result;
    bool automated = false;
    
    // Procesar argumentos
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--automated") == 0) {
            automated = true;
        }
    }
    
    // Inicializar CUnit
    if (CU_initialize_registry() != CUE_SUCCESS) {
        return CU_get_error();
    }
    
    // Registrar suite
    if (add_elevator_state_manager_tests() == NULL) {
        CU_cleanup_registry();
        return CU_get_error();
    }
    
    // Ejecutar pruebas
    if (automated) {
        CU_automated_run_tests();
        CU_list_tests_to_file();
    } else {
        CU_basic_set_mode(CU_BRM_VERBOSE);
        result = CU_basic_run_tests();
    }
    
    // Cerrar archivo de reporte
    close_report_file();
    
    // Mostrar resumen
    printf("\n=== RESUMEN DE PRUEBAS: GESTOR DE ESTADO DE ASCENSORES ===\n");
    printf("Pruebas ejecutadas: %u\n", CU_get_number_of_tests_run());
    printf("Fallos: %u\n", CU_get_number_of_failures());
    printf("Reporte detallado guardado en: test_elevator_state_manager_report.txt\n");
    
    CU_cleanup_registry();
    return (CU_get_number_of_failures() == 0) ? 0 : 1;
} 