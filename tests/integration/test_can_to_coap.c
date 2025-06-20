#include <CUnit/CUnit.h>
#include <CUnit/Basic.h>
#include <CUnit/Automated.h>
#include <coap3/coap.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "api_gateway/can_bridge.h"
#include "../mocks/mock_can_interface.h"
#include "../mocks/mock_coap_session.h"

// Variables globales para testing de integración
static coap_context_t *test_context = NULL;
static FILE *report_file = NULL;

// Setup ejecutado antes de cada test
int setup_integration_tests(void) {
    coap_startup();
    test_context = coap_new_context(NULL);
    mock_can_reset();
    mock_coap_session_reset();
    
    // Abrir archivo de reporte si no existe
    if (!report_file) {
        report_file = fopen("test_can_to_coap_report.txt", "w");
        if (report_file) {
            time_t now = time(NULL);
            fprintf(report_file, "=== REPORTE DE PRUEBAS: INTEGRACIÓN CAN-COAP ===\n");
            fprintf(report_file, "Fecha: %s\n", ctime(&now));
            fprintf(report_file, "===============================================\n\n");
        }
    }
    
    return (test_context != NULL) ? 0 : -1;
}

// Teardown ejecutado después de cada test
int teardown_integration_tests(void) {
    if (test_context) {
        coap_free_context(test_context);
        test_context = NULL;
    }
    mock_can_reset();
    mock_coap_session_reset();
    return 0;
}

// Helper para escribir resultados al reporte
void write_test_result(const char* test_name, const char* description, bool passed, const char* details) {
    if (report_file) {
        fprintf(report_file, "TEST: %s\n", test_name);
        fprintf(report_file, "Descripción: %s\n", description);
        fprintf(report_file, "Resultado: %s\n", passed ? "PASÓ" : "FALLÓ");
        if (details) {
            fprintf(report_file, "Detalles: %s\n", details);
        }
        fprintf(report_file, "----------------------------------------\n\n");
        fflush(report_file);
    }
}

// Test: Flujo completo CAN -> CoAP
void test_can_to_coap_flow(void) {
    char details[512];
    bool test_passed = true;
    
    // Inicializar puente CAN
    ag_can_bridge_init();
    
    // Crear frame CAN de llamada de piso
    simulated_can_frame_t frame = mock_can_create_floor_call_frame(5, 1); // Piso 5, DOWN
    
    // Simular recepción del frame
    mock_can_queue_received_frame(&frame);
    
    // Verificar que el frame se procesó correctamente
    int queued_count = mock_can_get_received_frame_count();
    
    if (queued_count != 1) {
        test_passed = false;
        snprintf(details, sizeof(details), "Frame no se encoló correctamente: esperado 1, obtenido %d", queued_count);
    } else {
        // Simular procesamiento del frame
        simulated_can_frame_t received_frame;
        bool received = mock_can_receive_frame(&received_frame);
        
        if (!received) {
            test_passed = false;
            snprintf(details, sizeof(details), "No se pudo recibir el frame encolado");
        } else if (received_frame.id != 0x100) {
            test_passed = false;
            snprintf(details, sizeof(details), "ID de frame incorrecto: esperado 0x100, obtenido 0x%X", received_frame.id);
        } else if (received_frame.data[0] != 5 || received_frame.data[1] != 1) {
            test_passed = false;
            snprintf(details, sizeof(details), "Datos de frame incorrectos: piso=%d, dirección=%d", 
                    received_frame.data[0], received_frame.data[1]);
        } else {
            snprintf(details, sizeof(details), 
                    "Flujo CAN->CoAP simulado correctamente: frame ID=0x%X procesado, piso=%d, dirección=%s", 
                    received_frame.id, received_frame.data[0], 
                    received_frame.data[1] == 1 ? "DOWN" : "UP");
        }
    }
    
    write_test_result("test_can_to_coap_flow", 
                     "Verifica el flujo completo de procesamiento CAN a CoAP",
                     test_passed, details);
    
    // Assertions de CUnit
    CU_ASSERT_EQUAL(frame.id, 0x100);
    CU_ASSERT_EQUAL(frame.data[0], 5);
    CU_ASSERT_EQUAL(frame.data[1], 1);
}

// Test: Creación de sesión CoAP
void test_coap_session_creation(void) {
    char details[512];
    bool test_passed = true;
    
    // Configurar dirección del servidor
    coap_address_t server_addr;
    coap_address_init(&server_addr);
    server_addr.addr.sin.sin_family = AF_INET;
    server_addr.addr.sin.sin_port = htons(5684);
    
    // Crear sesión mock
    coap_session_t *session = mock_coap_new_client_session_psk(
        test_context,
        NULL,
        &server_addr,
        COAP_PROTO_DTLS,
        "test_identity",
        (const uint8_t *)"test_key",
        8
    );
    
    if (!session) {
        test_passed = false;
        snprintf(details, sizeof(details), "No se pudo crear la sesión CoAP mock");
    } else {
        // Verificar estado de la sesión
        coap_session_state_t state = mock_coap_session_get_state(session);
        int session_count = mock_coap_session_get_count();
        
        if (state != COAP_SESSION_STATE_ESTABLISHED) {
            test_passed = false;
            snprintf(details, sizeof(details), "Estado de sesión incorrecto: esperado %d, obtenido %d", 
                    COAP_SESSION_STATE_ESTABLISHED, state);
        } else if (session_count != 1) {
            test_passed = false;
            snprintf(details, sizeof(details), "Número de sesiones incorrecto: esperado 1, obtenido %d", session_count);
        } else {
            snprintf(details, sizeof(details), 
                    "Sesión CoAP creada correctamente: estado=ESTABLISHED, protocolo=DTLS");
        }
        
        // Liberar sesión
        mock_coap_session_release(session);
    }
    
    write_test_result("test_coap_session_creation", 
                     "Verifica la creación correcta de sesiones CoAP",
                     test_passed, details);
    
    // Assertions de CUnit
    CU_ASSERT_PTR_NOT_NULL(session);
    if (session) {
        CU_ASSERT_EQUAL(mock_coap_session_get_state(session), COAP_SESSION_STATE_ESTABLISHED);
    }
}

// Test: Respuesta CoAP -> CAN
void test_coap_response_to_can(void) {
    char details[512];
    bool test_passed = true;
    
    // Crear respuesta JSON simulada del servidor
    cJSON *server_response = cJSON_CreateObject();
    cJSON_AddStringToObject(server_response, "tarea_id", "T_456");
    cJSON_AddStringToObject(server_response, "ascensor_asignado_id", "E1A2");
    cJSON_AddNumberToObject(server_response, "tiempo_estimado", 45);
    
    // Simular envío de respuesta CAN
    simulated_can_frame_t response_frame = {
        .id = 0x100 | 0x80, // ID con bit de respuesta
        .dlc = 8,
        .data = {0x01, 0x02, 0x2D, 0x00, 0x00, 0x00, 0x00, 0x00} // Datos codificados
    };
    
    mock_can_send_frame(&response_frame);
    
    // Verificar que se generó respuesta CAN
    int sent_count = mock_can_get_sent_frame_count();
    simulated_can_frame_t *sent_frame = mock_can_get_sent_frame(0);
    
    if (sent_count != 1) {
        test_passed = false;
        snprintf(details, sizeof(details), "Número de frames de respuesta incorrecto: esperado 1, obtenido %d", sent_count);
    } else if (!sent_frame) {
        test_passed = false;
        snprintf(details, sizeof(details), "No se pudo recuperar el frame de respuesta enviado");
    } else if (sent_frame->id != (0x100 | 0x80)) {
        test_passed = false;
        snprintf(details, sizeof(details), "ID de respuesta incorrecto: esperado 0x%X, obtenido 0x%X", 
                0x100 | 0x80, sent_frame->id);
    } else {
        snprintf(details, sizeof(details), 
                "Respuesta CoAP->CAN procesada correctamente: ID=0x%X, tarea=T_456, ascensor=E1A2, tiempo=45s", 
                sent_frame->id);
    }
    
    write_test_result("test_coap_response_to_can", 
                     "Verifica el procesamiento de respuestas CoAP a CAN",
                     test_passed, details);
    
    // Assertions de CUnit
    CU_ASSERT_PTR_NOT_NULL(server_response);
    CU_ASSERT_EQUAL(sent_count, 1);
    if (sent_frame) {
        CU_ASSERT_EQUAL(sent_frame->id, 0x100 | 0x80);
    }
    
    cJSON_Delete(server_response);
}

// Test: Manejo de errores de comunicación
void test_communication_error_handling(void) {
    char details[512];
    bool test_passed = true;
    
    // Resetear mocks antes de la prueba
    mock_can_reset();
    mock_coap_session_reset();
    
    // Configurar mock CoAP para fallar
    mock_coap_session_set_fail_mode(true);
    
    // Intentar crear sesión
    coap_address_t server_addr;
    coap_address_init(&server_addr);
    
    coap_session_t *session = mock_coap_new_client_session_psk(
        test_context,
        NULL,
        &server_addr,
        COAP_PROTO_DTLS,
        "test_identity",
        (const uint8_t *)"test_key",
        8
    );
    
    if (session != NULL) {
        test_passed = false;
        snprintf(details, sizeof(details), "La sesión se creó cuando debería haber fallado");
    } else {
        // Configurar mock CAN para fallar también
        mock_can_set_fail_mode(true);
        
        simulated_can_frame_t frame = mock_can_create_floor_call_frame(3, 0);
        mock_can_send_frame(&frame);
        
        int sent_count = mock_can_get_sent_frame_count();
        
        if (sent_count != 0) {
            test_passed = false;
            snprintf(details, sizeof(details), "Se enviaron frames CAN cuando debería haber fallado: count=%d", sent_count);
        } else {
            snprintf(details, sizeof(details), 
                    "Manejo de errores correcto: fallos de CoAP y CAN manejados apropiadamente");
        }
    }
    
    // Restaurar modo normal
    mock_coap_session_set_fail_mode(false);
    mock_can_set_fail_mode(false);
    
    write_test_result("test_communication_error_handling", 
                     "Verifica el manejo correcto de errores de comunicación",
                     test_passed, details);
    
    // Assertions de CUnit
    CU_ASSERT_PTR_NULL(session);
    CU_ASSERT_EQUAL(mock_can_get_sent_frame_count(), 0);
}

// Test: Flujo de múltiples solicitudes concurrentes
void test_concurrent_requests_flow(void) {
    char details[512];
    bool test_passed = true;
    
    // Resetear mocks antes de la prueba
    mock_can_reset();
    
    // Crear múltiples frames CAN
    simulated_can_frame_t frame1 = mock_can_create_floor_call_frame(2, 0);    // Piso 2, UP
    simulated_can_frame_t frame2 = mock_can_create_cabin_request_frame(0, 8); // Ascensor 0, piso 8
    simulated_can_frame_t frame3 = mock_can_create_floor_call_frame(6, 1);    // Piso 6, DOWN
    
    // Encolar múltiples solicitudes
    mock_can_queue_received_frame(&frame1);
    mock_can_queue_received_frame(&frame2);
    mock_can_queue_received_frame(&frame3);
    
    int initial_count = mock_can_get_received_frame_count();
    
    // Procesar todas las solicitudes
    int processed_count = 0;
    simulated_can_frame_t received_frame;
    
    while (mock_can_receive_frame(&received_frame)) {
        processed_count++;
        
        // Simular respuesta para cada solicitud
        simulated_can_frame_t response = {
            .id = received_frame.id | 0x80,
            .dlc = 4,
            .data = {processed_count, 0x01, 0x00, 0x00} // Respuesta simple
        };
        mock_can_send_frame(&response);
    }
    
    int final_received_count = mock_can_get_received_frame_count();
    int sent_responses = mock_can_get_sent_frame_count();
    
    if (initial_count != 3) {
        test_passed = false;
        snprintf(details, sizeof(details), "Número inicial de frames incorrecto: esperado 3, obtenido %d", initial_count);
    } else if (processed_count != 3) {
        test_passed = false;
        snprintf(details, sizeof(details), "Número de frames procesados incorrecto: esperado 3, obtenido %d", processed_count);
    } else if (final_received_count != 0) {
        test_passed = false;
        snprintf(details, sizeof(details), "Cola no se vació completamente: quedan %d frames", final_received_count);
    } else if (sent_responses != 3) {
        test_passed = false;
        snprintf(details, sizeof(details), "Número de respuestas enviadas incorrecto: esperado 3, obtenido %d", sent_responses);
    } else {
        snprintf(details, sizeof(details), 
                "Flujo concurrente procesado correctamente: 3 solicitudes recibidas y respondidas");
    }
    
    write_test_result("test_concurrent_requests_flow", 
                     "Verifica el manejo correcto de múltiples solicitudes concurrentes",
                     test_passed, details);
    
    // Assertions de CUnit
    CU_ASSERT_EQUAL(initial_count, 3);
    CU_ASSERT_EQUAL(processed_count, 3);
    CU_ASSERT_EQUAL(final_received_count, 0);
    CU_ASSERT_EQUAL(sent_responses, 3);
}

// Test: Validación de formato JSON
void test_json_format_validation(void) {
    char details[512];
    bool test_passed = true;
    
    // Crear JSON de solicitud válido
    cJSON *valid_request = cJSON_CreateObject();
    cJSON_AddStringToObject(valid_request, "id_edificio", "E1");
    cJSON_AddNumberToObject(valid_request, "piso_origen_llamada", 4);
    cJSON_AddStringToObject(valid_request, "direccion_llamada", "UP");
    cJSON_AddStringToObject(valid_request, "tipo_solicitud", "FLOOR_CALL");
    
    // Crear JSON de respuesta válido
    cJSON *valid_response = cJSON_CreateObject();
    cJSON_AddStringToObject(valid_response, "tarea_id", "T_789");
    cJSON_AddStringToObject(valid_response, "ascensor_asignado_id", "E1A3");
    cJSON_AddNumberToObject(valid_response, "tiempo_estimado", 30);
    cJSON_AddStringToObject(valid_response, "estado", "ASIGNADO");
    
    // Validar estructura del JSON de solicitud
    cJSON *id_edificio = cJSON_GetObjectItem(valid_request, "id_edificio");
    cJSON *piso_origen = cJSON_GetObjectItem(valid_request, "piso_origen_llamada");
    cJSON *direccion = cJSON_GetObjectItem(valid_request, "direccion_llamada");
    cJSON *tipo = cJSON_GetObjectItem(valid_request, "tipo_solicitud");
    
    if (!id_edificio || !cJSON_IsString(id_edificio)) {
        test_passed = false;
        snprintf(details, sizeof(details), "Campo 'id_edificio' faltante o tipo incorrecto");
    } else if (!piso_origen || !cJSON_IsNumber(piso_origen)) {
        test_passed = false;
        snprintf(details, sizeof(details), "Campo 'piso_origen_llamada' faltante o tipo incorrecto");
    } else if (!direccion || !cJSON_IsString(direccion)) {
        test_passed = false;
        snprintf(details, sizeof(details), "Campo 'direccion_llamada' faltante o tipo incorrecto");
    } else if (!tipo || !cJSON_IsString(tipo)) {
        test_passed = false;
        snprintf(details, sizeof(details), "Campo 'tipo_solicitud' faltante o tipo incorrecto");
    } else {
        // Validar estructura del JSON de respuesta
        cJSON *tarea_id = cJSON_GetObjectItem(valid_response, "tarea_id");
        cJSON *ascensor_id = cJSON_GetObjectItem(valid_response, "ascensor_asignado_id");
        
        if (!tarea_id || !cJSON_IsString(tarea_id)) {
            test_passed = false;
            snprintf(details, sizeof(details), "Campo 'tarea_id' faltante o tipo incorrecto en respuesta");
        } else if (!ascensor_id || !cJSON_IsString(ascensor_id)) {
            test_passed = false;
            snprintf(details, sizeof(details), "Campo 'ascensor_asignado_id' faltante o tipo incorrecto en respuesta");
        } else {
            snprintf(details, sizeof(details), 
                    "Formato JSON validado correctamente: solicitud y respuesta con estructura válida");
        }
    }
    
    write_test_result("test_json_format_validation", 
                     "Verifica la validación correcta del formato JSON",
                     test_passed, details);
    
    // Assertions de CUnit
    CU_ASSERT_PTR_NOT_NULL(valid_request);
    CU_ASSERT_PTR_NOT_NULL(valid_response);
    CU_ASSERT_PTR_NOT_NULL(id_edificio);
    CU_ASSERT_PTR_NOT_NULL(piso_origen);
    CU_ASSERT_TRUE(cJSON_IsString(id_edificio));
    CU_ASSERT_TRUE(cJSON_IsNumber(piso_origen));
    
    cJSON_Delete(valid_request);
    cJSON_Delete(valid_response);
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
CU_pSuite add_integration_tests(void) {
    CU_pSuite suite = CU_add_suite("Integration", 
                                   setup_integration_tests, 
                                   teardown_integration_tests);
    
    if (suite == NULL) return NULL;
    
    if (CU_add_test(suite, "test_can_to_coap_flow", 
                    test_can_to_coap_flow) == NULL ||
        CU_add_test(suite, "test_coap_session_creation", 
                    test_coap_session_creation) == NULL ||
        CU_add_test(suite, "test_coap_response_to_can", 
                    test_coap_response_to_can) == NULL ||
        CU_add_test(suite, "test_communication_error_handling", 
                    test_communication_error_handling) == NULL ||
        CU_add_test(suite, "test_concurrent_requests_flow", 
                    test_concurrent_requests_flow) == NULL ||
        CU_add_test(suite, "test_json_format_validation", 
                    test_json_format_validation) == NULL) {
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
    if (add_integration_tests() == NULL) {
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
    printf("\n=== RESUMEN DE PRUEBAS: INTEGRACIÓN CAN-COAP ===\n");
    printf("Pruebas ejecutadas: %u\n", CU_get_number_of_tests_run());
    printf("Fallos: %u\n", CU_get_number_of_failures());
    printf("Reporte detallado guardado en: test_can_to_coap_report.txt\n");
    
    // Limpiar CoAP
    coap_cleanup();
    
    CU_cleanup_registry();
    return (CU_get_number_of_failures() == 0) ? 0 : 1;
} 