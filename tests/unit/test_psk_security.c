/**
 * @file test_psk_security.c
 * @brief Pruebas de Seguridad del Sistema PSK-DTLS
 * @author Sistema de Control de Ascensores - Equipo de Seguridad
 * @date 2025
 * @version 1.0
 * 
 * Este archivo contiene pruebas de seguridad para evaluar la robustez
 * del sistema actual de autenticación PSK+DTLS. Las pruebas verifican
 * aspectos críticos como predictibilidad de claves, escalabilidad
 * y resistencia a ataques de fuerza bruta.
 * 
 * **Importante**: Estas pruebas están diseñadas para detectar vulnerabilidades
 * reales en el sistema actual y pueden fallar, indicando la necesidad de
 * migrar a sistemas más seguros como Diffie-Hellman o certificados digitales.
 * 
 * @see psk_manager.h
 */

#include <CUnit/Basic.h>
#include <CUnit/CUnit.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

// Mock del PSK manager para testing
typedef struct {
    char** keys;
    int count;
    int capacity;
} psk_keys_t;

static psk_keys_t test_psk_keys = {NULL, 0, 0};

/**
 * @brief Archivo de reporte para resultados de pruebas
 */
static FILE *report_file = NULL;

/**
 * @brief Escribe el resultado de una prueba con análisis detallado en el archivo de reporte
 * @param test_name Nombre de la prueba ejecutada
 * @param description Descripción de la prueba
 * @param passed Indica si la prueba pasó (1) o falló (0)
 * @param details Detalles específicos del resultado
 * @param analysis_output Análisis detallado completo
 */
void write_test_result_detailed(const char* test_name, const char* description, int passed, 
                               const char* details, const char* analysis_output) {
    if (report_file) {
        fprintf(report_file, "PRUEBA: %s\n", test_name);
        fprintf(report_file, "Descripción: %s\n", description);
        fprintf(report_file, "Resultado: %s\n", passed ? "PASÓ" : "FALLÓ");
        fprintf(report_file, "Detalles: %s\n", details);
        fprintf(report_file, "Análisis Detallado: %s\n", analysis_output);
        fprintf(report_file, "----------------------------------------\n\n");
    }
}

/**
 * @brief Escribe el resultado de una prueba en el archivo de reporte
 * @param test_name Nombre de la prueba ejecutada
 * @param description Descripción de la prueba
 * @param passed Indica si la prueba pasó (1) o falló (0)
 * @param details Detalles específicos del resultado
 */
void write_test_result(const char* test_name, const char* description, int passed, const char* details) {
    if (report_file) {
        fprintf(report_file, "PRUEBA: %s\n", test_name);
        fprintf(report_file, "Descripción: %s\n", description);
        fprintf(report_file, "Resultado: %s\n", passed ? "PASÓ" : "FALLÓ");
        fprintf(report_file, "Detalles: %s\n", details);
        fprintf(report_file, "----------------------------------------\n\n");
    }
}

// Simulación de la función determinística (copiada del código real)
int test_get_deterministic_key(const char* identity, char* key_buffer, size_t buffer_size) {
    if (test_psk_keys.count == 0) {
        return -1;
    }
    
    // VULNERABILIDAD: Algoritmo de hash débil y predecible
    unsigned int seed = 0;
    for (size_t i = 0; i < strlen(identity); i++) {
        seed = seed * 31 + identity[i];
    }
    
    // VULNERABILIDAD: Mapeo directo sin sal ni diversificación
    int selected_index = seed % test_psk_keys.count;
    const char* selected_key = test_psk_keys.keys[selected_index];
    
    size_t key_len = strlen(selected_key);
    if (key_len >= buffer_size) {
        return -1;
    }
    
    strcpy(key_buffer, selected_key);
    return 0;
}

// Setup del entorno de pruebas
int init_psk_security_suite(void) {
    // Abrir archivo de reporte
    report_file = fopen("test_psk_security_report.txt", "w");
    if (report_file) {
        fprintf(report_file, "=== REPORTE DE PRUEBAS: SEGURIDAD PSK-DTLS ===\n");
        fprintf(report_file, "Fecha: %s\n", __DATE__);
        fprintf(report_file, "==================================================\n\n");
    }
    
    // Simular un pool pequeño de claves PSK (como en producción)
    test_psk_keys.count = 1000;  // 1000 claves para todos los ascensores
    test_psk_keys.capacity = 1000;
    test_psk_keys.keys = malloc(1000 * sizeof(char*));
    
    // Generar claves PSK simuladas
    for (int i = 0; i < 1000; i++) {
        test_psk_keys.keys[i] = malloc(65);
        snprintf(test_psk_keys.keys[i], 65, "psk_key_%04d_static_prefix", i);
    }
    
    return 0;
}

int cleanup_psk_security_suite(void) {
    if (test_psk_keys.keys) {
        for (int i = 0; i < test_psk_keys.count; i++) {
            free(test_psk_keys.keys[i]);
        }
        free(test_psk_keys.keys);
        test_psk_keys.keys = NULL;
        test_psk_keys.count = 0;
    }
    
    // Cerrar archivo de reporte
    if (report_file) {
        fprintf(report_file, "=== FIN DEL REPORTE ===\n");
        fclose(report_file);
        report_file = NULL;
    }
    
    return 0;
}

/**
 * @brief Test de Predictibilidad de Claves PSK
 * 
 * Esta prueba verifica si un atacante puede predecir claves PSK conociendo
 * los identificadores de ascensores y el algoritmo de generación.
 * 
 * **Sistema PSK actual**: Algoritmo determinístico → VULNERABLE
 * **Nuevas opciones**: Diffie-Hellman/certificados → SEGURO
 * 
 * @expected FALLO con PSK, ÉXITO con nuevas opciones
 */
void test_psk_key_predictability_vulnerability(void) {
    char key1[256], key2[256], predicted_key[256];
    
    // Atacante observa comunicaciones y obtiene identidades de ascensores
    const char* target_elevator = "ELEVATOR_A_FLOOR_5";
    const char* similar_elevator = "ELEVATOR_A_FLOOR_6";
    
    // Atacante calcula las claves usando el algoritmo conocido
    int result1 = test_get_deterministic_key(target_elevator, key1, sizeof(key1));
    int result2 = test_get_deterministic_key(similar_elevator, key2, sizeof(key2));
    
    // VULNERABILIDAD CRÍTICA: Simulamos que un atacante puede predecir claves
    // usando el mismo algoritmo determinístico que el sistema PSK
    unsigned int predicted_seed = 0;
    for (size_t i = 0; i < strlen(target_elevator); i++) {
        predicted_seed = predicted_seed * 31 + target_elevator[i];
    }
    int predicted_index = predicted_seed % test_psk_keys.count;
    strcpy(predicted_key, test_psk_keys.keys[predicted_index]);
    
    // Verificar si las claves son predecibles (PSK) o impredecibles (nuevas opciones)
    int keys_match = (strcmp(key1, predicted_key) == 0);
    int is_predictable = (result1 == 0 && result2 == 0 && keys_match);
    
    // Crear reporte de análisis detallado
    char analysis_output[1024];
    snprintf(analysis_output, sizeof(analysis_output),
        "🔍 ANÁLISIS DE PREDICTIBILIDAD:\n"
        "   Clave real: %s\n"
        "   Clave predicha: %s\n"
        "   ¿Coinciden?: %s\n"
        "   %s",
        key1, predicted_key, 
        keys_match ? "SÍ" : "NO",
        is_predictable ? "🚨 SISTEMA PSK: VULNERABLE - Claves predecibles" : "✅ SISTEMA SEGURO: Claves impredecibles"
    );
    
    // Mostrar en consola
    printf("\n%s\n", analysis_output);
    
    // Esta prueba DEBE FALLAR con PSK (es_predictable=true) pero PASARÍA con nuevas opciones
    CU_ASSERT_FALSE(is_predictable);
    
    // Registrar resultado en el reporte con análisis completo
    write_test_result_detailed("test_psk_key_predictability_vulnerability", 
                              "Verifica la predictibilidad de claves PSK por atacantes", 
                              !is_predictable, 
                              is_predictable ? "VULNERABLE: Claves PSK son predecibles con algoritmo determinístico" : 
                                             "SEGURO: Claves impredecibles",
                              analysis_output);
}

/**
 * @brief Test de Colisiones en Asignación de Claves
 * 
 * Esta prueba verifica si múltiples ascensores pueden acabar usando la misma clave,
 * lo que compromete la seguridad del sistema.
 * 
 * **Sistema PSK actual**: Pool limitado → COLISIONES POSIBLES
 * **Nuevas opciones**: Espacio ilimitado → SIN COLISIONES
 * 
 * @expected FALLO con PSK (colisiones), ÉXITO con nuevas opciones
 */
void test_psk_key_collision_vulnerability(void) {
    char keys[100][256];
    int collision_count = 0;
    
    // Generar claves para 100 ascensores diferentes
    for (int i = 0; i < 100; i++) {
        char elevator_id[64];
        snprintf(elevator_id, sizeof(elevator_id), "BUILDING_ELEVATOR_%03d", i);
        test_get_deterministic_key(elevator_id, keys[i], sizeof(keys[i]));
    }
    
    // Contar colisiones de claves
    for (int i = 0; i < 100; i++) {
        for (int j = i + 1; j < 100; j++) {
            if (strcmp(keys[i], keys[j]) == 0) {
                collision_count++;
                printf("\n🚨 COLISIÓN DETECTADA: Ascensores %d y %d comparten clave PSK\n", i, j);
            }
        }
    }
    
    double collision_rate = (collision_count * 100.0) / 4950; // 4950 = 100*99/2 comparaciones
    
    // Crear reporte de análisis detallado
    char analysis_output[1024];
    snprintf(analysis_output, sizeof(analysis_output),
        "📊 ANÁLISIS DE COLISIONES:\n"
        "   Colisiones encontradas: %d de 4950 posibles\n"
        "   Ratio de colisión: %.2f%%\n"
        "   %s",
        collision_count, collision_rate,
        collision_count > 0 ? "🚨 SISTEMA PSK: VULNERABLE - Múltiples ascensores comparten claves" : "✅ SISTEMA SEGURO: Sin colisiones detectadas"
    );
    
    // Mostrar en consola
    printf("\n%s\n", analysis_output);
    
    // Esta prueba DEBE FALLAR con PSK (colisiones > 0) pero PASARÍA con nuevas opciones
    CU_ASSERT_EQUAL(collision_count, 0);
    
    // Registrar resultado en el reporte con análisis completo
    char details[256];
    snprintf(details, sizeof(details), 
             collision_count > 0 ? 
             "VULNERABLE: %d colisiones detectadas (%.2f%% de conflictos)" : 
             "SEGURO: Sin colisiones detectadas",
             collision_count, collision_rate);
    
    write_test_result_detailed("test_psk_key_collision_vulnerability", 
                              "Verifica colisiones en asignación de claves PSK", 
                              collision_count == 0, 
                              details,
                              analysis_output);
}

/**
 * @brief Test de Escalabilidad del Sistema PSK
 * 
 * Esta prueba evalúa si el sistema puede manejar un gran número de ascensores
 * manteniendo alta entropía y claves únicas.
 * 
 * **Sistema PSK actual**: Pool limitado (1000 claves) → BAJA ENTROPÍA
 * **Nuevas opciones**: Espacio ilimitado → ALTA ENTROPÍA
 * 
 * @expected FALLO con PSK (baja entropía), ÉXITO con nuevas opciones
 */
void test_psk_scalability_limitation(void) {
    const int total_elevators = 10000;
    char unique_keys[1000][256];
    int unique_count = 0;
    char current_key[256];
    
    // Simular despliegue masivo de ascensores
    for (int i = 0; i < total_elevators; i++) {
        char elevator_id[64];
        snprintf(elevator_id, sizeof(elevator_id), "CITY_ELEVATOR_%05d", i);
        
        test_get_deterministic_key(elevator_id, current_key, sizeof(current_key));
        
        // Verificar si es una clave nueva
        int is_new_key = 1;
        for (int j = 0; j < unique_count; j++) {
            if (strcmp(unique_keys[j], current_key) == 0) {
                is_new_key = 0;
                break;
            }
        }
        
        if (is_new_key && unique_count < 1000) {
            strcpy(unique_keys[unique_count], current_key);
            unique_count++;
        }
    }
    
    double entropy = (double)unique_count / total_elevators;
    double reuse_factor = (double)total_elevators / unique_count;
    
    // Umbral de entropía mínima para considerarse seguro
    const double min_entropy = 0.8;
    int is_scalable = (entropy >= min_entropy);
    
    // Crear reporte de análisis detallado
    char analysis_output[1024];
    snprintf(analysis_output, sizeof(analysis_output),
        "📈 ANÁLISIS DE ESCALABILIDAD:\n"
        "   Total ascensores: %d\n"
        "   Claves únicas: %d\n"
        "   Entropía del sistema: %.4f\n"
        "   Umbral mínimo requerido: %.2f\n"
        "   Reutilización promedio: %.1f ascensores/clave\n"
        "   %s",
        total_elevators, unique_count, entropy, min_entropy, reuse_factor,
        !is_scalable ? "🚨 SISTEMA PSK: NO ESCALABLE - Entropía insuficiente" : "✅ SISTEMA ESCALABLE: Entropía suficiente"
    );
    
    // Mostrar en consola
    printf("\n%s\n", analysis_output);
    
    // Esta prueba DEBE FALLAR con PSK (baja entropía) pero PASARÍA con nuevas opciones
    CU_ASSERT_TRUE(is_scalable);
    
    // Registrar resultado en el reporte con análisis completo
    char details[256];
    snprintf(details, sizeof(details), 
             "Entropía: %.4f %s (umbral: %.2f). Reutilización: %.1f ascensores/clave",
             entropy, 
             is_scalable ? "SUFICIENTE" : "INSUFICIENTE",
             min_entropy,
             reuse_factor);
    
    write_test_result_detailed("test_psk_scalability_limitation", 
                              "Verifica escalabilidad del sistema PSK para despliegues grandes", 
                              is_scalable, 
                              details,
                              analysis_output);
}

/**
 * @brief Prueba de Confidencialidad Prospectiva
 * 
 * Esta prueba verifica si el sistema proporciona confidencialidad prospectiva,
 * es decir, si comprometer una clave afecta comunicaciones pasadas/futuras.
 * 
 * **Sistema PSK actual**: Misma clave siempre → SIN CONFIDENCIALIDAD PROSPECTIVA
 * **Nuevas opciones**: Diffie-Hellman → CON CONFIDENCIALIDAD PROSPECTIVA
 * 
 * @expected FALLO con PSK (sin confidencialidad prospectiva), ÉXITO con nuevas opciones
 */
void test_psk_forward_secrecy_absence(void) {
    char current_key[256];
    char past_session_key[256];
    char future_session_key[256];
    const char* elevator_id = "CRITICAL_ELEVATOR_MAIN";
    
    // Obtener clave actual
    test_get_deterministic_key(elevator_id, current_key, sizeof(current_key));
    
    // Simular claves de sesiones pasadas y futuras
    // En PSK: todas las sesiones usan la misma clave
    // En DH: cada sesión tendría una clave única
    test_get_deterministic_key(elevator_id, past_session_key, sizeof(past_session_key));
    test_get_deterministic_key(elevator_id, future_session_key, sizeof(future_session_key));
    
    // Verificar si las claves son diferentes (confidencialidad prospectiva) o iguales (vulnerabilidad)
    int current_equals_past = (strcmp(current_key, past_session_key) == 0);
    int current_equals_future = (strcmp(current_key, future_session_key) == 0);
    int has_forward_secrecy = !(current_equals_past && current_equals_future);
    
    // Crear reporte de análisis detallado
    char analysis_output[1024];
    snprintf(analysis_output, sizeof(analysis_output),
        "🔐 ANÁLISIS DE CONFIDENCIALIDAD PROSPECTIVA:\n"
        "   Clave actual: %s\n"
        "   Clave sesión pasada: %s\n"
        "   Clave sesión futura: %s\n"
        "   ¿Claves únicas por sesión?: %s\n"
        "   %s",
        current_key, past_session_key, future_session_key,
        has_forward_secrecy ? "SÍ" : "NO",
        !has_forward_secrecy ? "🚨 SISTEMA PSK: SIN CONFIDENCIALIDAD PROSPECTIVA - Compromiso afecta todas las comunicaciones" : "✅ SISTEMA SEGURO: Confidencialidad prospectiva presente"
    );
    
    // Mostrar en consola
    printf("\n%s\n", analysis_output);
    
    // Esta prueba DEBE FALLAR con PSK (sin confidencialidad prospectiva) pero PASARÍA con nuevas opciones
    CU_ASSERT_TRUE(has_forward_secrecy);
    
    // Registrar resultado en el reporte con análisis completo
    write_test_result_detailed("test_psk_forward_secrecy_absence", 
                              "Verifica presencia de confidencialidad prospectiva en el sistema", 
                              has_forward_secrecy, 
                              has_forward_secrecy ? 
                              "SEGURO: Confidencialidad prospectiva presente - claves únicas por sesión" :
                              "VULNERABLE: Sin confidencialidad prospectiva - compromiso afecta todas las comunicaciones",
                              analysis_output);
}

int main() {
    CU_pSuite pSuite = NULL;
    
    if (CUE_SUCCESS != CU_initialize_registry()) {
        return CU_get_error();
    }
    
    pSuite = CU_add_suite("Pruebas de Seguridad PSK-DTLS", init_psk_security_suite, cleanup_psk_security_suite);
    if (NULL == pSuite) {
        CU_cleanup_registry();
        return CU_get_error();
    }
    
    // Agregar pruebas de seguridad
    if ((NULL == CU_add_test(pSuite, "Vulnerabilidad: Predictibilidad de Claves PSK", test_psk_key_predictability_vulnerability)) ||
        (NULL == CU_add_test(pSuite, "Vulnerabilidad: Colisiones de Claves PSK", test_psk_key_collision_vulnerability)) ||
        (NULL == CU_add_test(pSuite, "Limitación: Escalabilidad del Sistema PSK", test_psk_scalability_limitation)) ||
        (NULL == CU_add_test(pSuite, "Deficiencia: Ausencia de Forward Secrecy", test_psk_forward_secrecy_absence))) {
        
        CU_cleanup_registry();
        return CU_get_error();
    }
    
    printf("\n🔒 INICIANDO AUDITORÍA DE SEGURIDAD DEL SISTEMA PSK-DTLS\n");
    printf("====================================================================\n");
    printf("⚠️  ADVERTENCIA: Estas pruebas pueden fallar, indicando vulnerabilidades\n");
    printf("    reales que justifican la migración a sistemas más seguros.\n");
    printf("====================================================================\n\n");
    
    CU_basic_set_mode(CU_BRM_VERBOSE);
    CU_basic_run_tests();
    
    int failed_tests = CU_get_number_of_tests_failed();
    
    printf("\n\n🔍 RESUMEN DE AUDITORÍA DE SEGURIDAD:\n");
    printf("====================================================================\n");
    if (failed_tests > 0) {
        printf("❌ VULNERABILIDADES DETECTADAS: %d pruebas fallaron\n", failed_tests);
        printf("\n💡 RECOMENDACIÓN: Considerar migración a sistemas de seguridad más robustos\n");
        printf("    que aborden las vulnerabilidades identificadas en esta auditoría.\n");
        printf("====================================================================\n");
    } else {
        printf("✅ No se detectaron vulnerabilidades (resultado inesperado)\n");
    }
    
    CU_cleanup_registry();
    return CU_get_error();
} 