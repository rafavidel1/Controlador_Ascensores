/**
 * @file test_can_bridge.c
 * @brief Pruebas unitarias para el Puente de Comunicación CAN
 * @author Sistema de Control de Ascensores
 * @date 2025
 * @version 1.0
 * 
 * Este archivo contiene las pruebas unitarias para verificar el correcto
 * funcionamiento del puente de comunicación CAN, incluyendo:
 * - Inicialización del puente CAN
 * - Envío de tramas CAN
 * - Recepción de tramas CAN
 * - Manejo de errores de comunicación
 * - Procesamiento de múltiples tramas
 * - Validación de datos de tramas
 * 
 * @see can_bridge.h
 * @see api_gateway/can_bridge.c
 */

#include <CUnit/CUnit.h>
#include <CUnit/Basic.h>
#include <CUnit/Automated.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "api_gateway/can_bridge.h"
#include "../mocks/mock_can_interface.h"

/**
 * @brief Variable global para el estado del setup de la suite
 * 
 * Indica si la función de setup ha sido llamada correctamente.
 * Se utiliza para verificar que la inicialización de la suite funciona.
 */
static FILE *report_file = NULL;

/**
 * @brief Función de setup para la suite de pruebas del puente CAN
 * @return 0 si el setup es exitoso, código de error en caso contrario
 * 
 * Esta función se ejecuta antes de cada suite de pruebas.
 * Inicializa el entorno necesario para las pruebas del puente CAN,
 * incluyendo la apertura del archivo de reporte y el reset de mocks.
 */
int setup_can_bridge_tests(void) {
    mock_can_reset();
    
    // Abrir archivo de reporte si no existe
    if (!report_file) {
        report_file = fopen("test_can_bridge_report.txt", "w");
        if (report_file) {
            time_t now = time(NULL);
            fprintf(report_file, "=== REPORTE DE PRUEBAS: PUENTE CAN ===\n");
            fprintf(report_file, "Fecha: %s\n", ctime(&now));
            fprintf(report_file, "======================================\n\n");
        }
    }
    
    return 0;
}

/**
 * @brief Función de teardown para la suite de pruebas del puente CAN
 * @return 0 si el teardown es exitoso, código de error en caso contrario
 * 
 * Esta función se ejecuta después de cada suite de pruebas.
 * Limpia el entorno y libera recursos utilizados durante las pruebas,
 * incluyendo el cierre del archivo de reporte.
 */
int teardown_can_bridge_tests(void) {
    mock_can_reset();
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

// Test: Inicialización del puente CAN
void test_can_bridge_init(void) {
    char details[512];
    bool test_passed = true;
    
    // Ejecutar función bajo prueba
    ag_can_bridge_init();
    
    // Verificar que no hay frames pendientes después de la inicialización
    int sent_count = mock_can_get_sent_frame_count();
    int received_count = mock_can_get_received_frame_count();
    
    if (sent_count != 0) {
        test_passed = false;
        snprintf(details, sizeof(details), "Frames enviados después de init: esperado 0, obtenido %d", sent_count);
    } else if (received_count != 0) {
        test_passed = false;
        snprintf(details, sizeof(details), "Frames recibidos después de init: esperado 0, obtenido %d", received_count);
    } else {
        snprintf(details, sizeof(details), "Puente CAN inicializado correctamente sin frames pendientes");
    }
    
    write_test_result("test_can_bridge_init", 
                     "Verifica la correcta inicialización del puente CAN",
                     test_passed, details);
    
    // Assertions de CUnit
    CU_ASSERT_EQUAL(sent_count, 0);
    CU_ASSERT_EQUAL(received_count, 0);
}

// Test: Procesamiento de frame de llamada de piso
void test_process_floor_call_frame(void) {
    char details[512];
    bool test_passed = true;
    
    // Crear frame de llamada de piso
    simulated_can_frame_t frame = mock_can_create_floor_call_frame(3, 0); // Piso 3, UP
    
    // Verificar estructura del frame
    if (frame.id != 0x100) {
        test_passed = false;
        snprintf(details, sizeof(details), "ID de frame incorrecto: esperado 0x100, obtenido 0x%X", frame.id);
    } else if (frame.dlc != 2) {
        test_passed = false;
        snprintf(details, sizeof(details), "DLC incorrecto: esperado 2, obtenido %d", frame.dlc);
    } else if (frame.data[0] != 3) {
        test_passed = false;
        snprintf(details, sizeof(details), "Piso incorrecto: esperado 3, obtenido %d", frame.data[0]);
    } else if (frame.data[1] != 0) {
        test_passed = false;
        snprintf(details, sizeof(details), "Dirección incorrecta: esperado 0 (UP), obtenido %d", frame.data[1]);
    } else {
        snprintf(details, sizeof(details), 
                "Frame de llamada de piso creado correctamente: ID=0x%X, piso=%d, dirección=%d", 
                frame.id, frame.data[0], frame.data[1]);
    }
    
    write_test_result("test_process_floor_call_frame", 
                     "Verifica el procesamiento de frames de llamada de piso",
                     test_passed, details);
    
    // Assertions de CUnit
    CU_ASSERT_EQUAL(frame.id, 0x100);
    CU_ASSERT_EQUAL(frame.dlc, 2);
    CU_ASSERT_EQUAL(frame.data[0], 3);
    CU_ASSERT_EQUAL(frame.data[1], 0);
}

// Test: Procesamiento de frame de solicitud de cabina
void test_process_cabin_request_frame(void) {
    char details[512];
    bool test_passed = true;
    
    // Crear frame de solicitud de cabina
    simulated_can_frame_t frame = mock_can_create_cabin_request_frame(1, 7); // Ascensor 1, piso 7
    
    // Verificar estructura del frame
    if (frame.id != 0x200) {
        test_passed = false;
        snprintf(details, sizeof(details), "ID de frame incorrecto: esperado 0x200, obtenido 0x%X", frame.id);
    } else if (frame.dlc != 2) {
        test_passed = false;
        snprintf(details, sizeof(details), "DLC incorrecto: esperado 2, obtenido %d", frame.dlc);
    } else if (frame.data[0] != 1) {
        test_passed = false;
        snprintf(details, sizeof(details), "Índice de ascensor incorrecto: esperado 1, obtenido %d", frame.data[0]);
    } else if (frame.data[1] != 7) {
        test_passed = false;
        snprintf(details, sizeof(details), "Piso destino incorrecto: esperado 7, obtenido %d", frame.data[1]);
    } else {
        snprintf(details, sizeof(details), 
                "Frame de solicitud de cabina creado correctamente: ID=0x%X, ascensor=%d, destino=%d", 
                frame.id, frame.data[0], frame.data[1]);
    }
    
    write_test_result("test_process_cabin_request_frame", 
                     "Verifica el procesamiento de frames de solicitud de cabina",
                     test_passed, details);
    
    // Assertions de CUnit
    CU_ASSERT_EQUAL(frame.id, 0x200);
    CU_ASSERT_EQUAL(frame.dlc, 2);
    CU_ASSERT_EQUAL(frame.data[0], 1);
    CU_ASSERT_EQUAL(frame.data[1], 7);
}

// Test: Envío de respuesta CAN
void test_send_response_frame(void) {
    char details[512];
    bool test_passed = true;
    
    // Crear respuesta JSON simulada
    cJSON *response_json = cJSON_CreateObject();
    cJSON_AddStringToObject(response_json, "tarea_id", "T_123");
    cJSON_AddStringToObject(response_json, "ascensor_asignado_id", "E1A1");
    
    // Simular envío de respuesta
    simulated_can_frame_t response_frame = {
        .id = 0x100 | 0x80, // ID original con bit de respuesta
        .dlc = 8,
        .data = {0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF} // Datos simulados
    };
    
    mock_can_send_frame(&response_frame);
    
    // Verificar que se envió el frame
    int sent_count = mock_can_get_sent_frame_count();
    simulated_can_frame_t *sent_frame = mock_can_get_sent_frame(0);
    
    if (sent_count != 1) {
        test_passed = false;
        snprintf(details, sizeof(details), "Número de frames enviados incorrecto: esperado 1, obtenido %d", sent_count);
    } else if (!sent_frame) {
        test_passed = false;
        snprintf(details, sizeof(details), "No se pudo recuperar el frame enviado");
    } else if (sent_frame->id != (0x100 | 0x80)) {
        test_passed = false;
        snprintf(details, sizeof(details), "ID de respuesta incorrecto: esperado 0x%X, obtenido 0x%X", 
                0x100 | 0x80, sent_frame->id);
    } else {
        snprintf(details, sizeof(details), 
                "Frame de respuesta enviado correctamente: ID=0x%X, DLC=%d", 
                sent_frame->id, sent_frame->dlc);
    }
    
    write_test_result("test_send_response_frame", 
                     "Verifica el envío correcto de frames de respuesta CAN",
                     test_passed, details);
    
    // Assertions de CUnit
    CU_ASSERT_EQUAL(sent_count, 1);
    CU_ASSERT_PTR_NOT_NULL(sent_frame);
    if (sent_frame) {
        CU_ASSERT_EQUAL(sent_frame->id, 0x100 | 0x80);
    }
    
    cJSON_Delete(response_json);
}

// Test: Manejo de errores en envío CAN
void test_can_send_error_handling(void) {
    char details[512];
    bool test_passed = true;
    
    // Resetear el mock antes de la prueba
    mock_can_reset();
    
    // Configurar mock para fallar
    mock_can_set_fail_mode(true);
    
    // Intentar enviar frame
    simulated_can_frame_t frame = mock_can_create_floor_call_frame(5, 1);
    mock_can_send_frame(&frame);
    
    // Verificar que no se envió nada
    int sent_count = mock_can_get_sent_frame_count();
    
    if (sent_count != 0) {
        test_passed = false;
        snprintf(details, sizeof(details), "Se enviaron frames cuando debería haber fallado: count=%d", sent_count);
    } else {
        snprintf(details, sizeof(details), "Manejo de errores correcto: no se enviaron frames en modo fallo");
    }
    
    // Restaurar modo normal
    mock_can_set_fail_mode(false);
    
    write_test_result("test_can_send_error_handling", 
                     "Verifica el manejo correcto de errores en envío CAN",
                     test_passed, details);
    
    // Assertions de CUnit
    CU_ASSERT_EQUAL(sent_count, 0);
}

// Test: Recepción de múltiples frames
void test_multiple_frame_reception(void) {
    char details[512];
    bool test_passed = true;
    
    // Crear múltiples frames para recepción
    simulated_can_frame_t frame1 = mock_can_create_floor_call_frame(2, 0);
    simulated_can_frame_t frame2 = mock_can_create_cabin_request_frame(0, 5);
    simulated_can_frame_t frame3 = mock_can_create_arrival_frame(1, 3);
    
    // Encolar frames para recepción
    mock_can_queue_received_frame(&frame1);
    mock_can_queue_received_frame(&frame2);
    mock_can_queue_received_frame(&frame3);
    
    // Verificar que se encolaron correctamente
    int queued_count = mock_can_get_received_frame_count();
    
    if (queued_count != 3) {
        test_passed = false;
        snprintf(details, sizeof(details), "Número de frames encolados incorrecto: esperado 3, obtenido %d", queued_count);
    } else {
        // Recibir frames uno por uno
        simulated_can_frame_t received_frame;
        bool received1 = mock_can_receive_frame(&received_frame);
        bool received2 = mock_can_receive_frame(&received_frame);
        bool received3 = mock_can_receive_frame(&received_frame);
        bool received4 = mock_can_receive_frame(&received_frame); // Este debería fallar
        
        if (!received1 || !received2 || !received3) {
            test_passed = false;
            snprintf(details, sizeof(details), "Error al recibir frames: %d, %d, %d", received1, received2, received3);
        } else if (received4) {
            test_passed = false;
            snprintf(details, sizeof(details), "Se recibió un frame cuando no debería haber más");
        } else {
            snprintf(details, sizeof(details), "Recepción múltiple correcta: 3 frames procesados, cola vacía");
        }
    }
    
    write_test_result("test_multiple_frame_reception", 
                     "Verifica la recepción correcta de múltiples frames CAN",
                     test_passed, details);
    
    // Assertions de CUnit
    CU_ASSERT_EQUAL(queued_count, 3);
    
    simulated_can_frame_t received_frame;
    CU_ASSERT_TRUE(mock_can_receive_frame(&received_frame));
    CU_ASSERT_TRUE(mock_can_receive_frame(&received_frame));
    CU_ASSERT_TRUE(mock_can_receive_frame(&received_frame));
    CU_ASSERT_FALSE(mock_can_receive_frame(&received_frame)); // Cola vacía
}

// Función para cerrar el archivo de reporte
void close_report_file(void) {
    if (report_file) {
        fprintf(report_file, "\n=== FIN DEL REPORTE ===\n");
        fclose(report_file);
        report_file = NULL;
    }
}

// Suite de pruebas
CU_pSuite add_can_bridge_tests(void) {
    CU_pSuite suite = CU_add_suite("CANBridge", 
                                   setup_can_bridge_tests, 
                                   teardown_can_bridge_tests);
    
    if (suite == NULL) return NULL;
    
    if (CU_add_test(suite, "test_can_bridge_init", 
                    test_can_bridge_init) == NULL ||
        CU_add_test(suite, "test_process_floor_call_frame", 
                    test_process_floor_call_frame) == NULL ||
        CU_add_test(suite, "test_process_cabin_request_frame", 
                    test_process_cabin_request_frame) == NULL ||
        CU_add_test(suite, "test_send_response_frame", 
                    test_send_response_frame) == NULL ||
        CU_add_test(suite, "test_can_send_error_handling", 
                    test_can_send_error_handling) == NULL ||
        CU_add_test(suite, "test_multiple_frame_reception", 
                    test_multiple_frame_reception) == NULL) {
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
    if (add_can_bridge_tests() == NULL) {
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
    printf("\n=== RESUMEN DE PRUEBAS: PUENTE CAN ===\n");
    printf("Pruebas ejecutadas: %u\n", CU_get_number_of_tests_run());
    printf("Fallos: %u\n", CU_get_number_of_failures());
    printf("Reporte detallado guardado en: test_can_bridge_report.txt\n");
    
    CU_cleanup_registry();
    return (CU_get_number_of_failures() == 0) ? 0 : 1;
} 