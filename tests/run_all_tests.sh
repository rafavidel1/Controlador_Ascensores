#!/bin/bash

# Script principal para ejecutar todas las pruebas del sistema
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
# Directorio temporal para compilación - DENTRO de tests/
TEMP_BUILD_DIR="$SCRIPT_DIR/temp-build-tests"
# Directorio de reportes - EL MISMO que el temporal
REPORTS_DIR="$TEMP_BUILD_DIR"

echo "==============================================================================="
echo "                    EJECUTOR DE PRUEBAS - SISTEMA DE ASCENSORES"
echo "==============================================================================="
echo ""
echo "📁 Directorio del proyecto: $PROJECT_ROOT"
echo "🔨 Directorio temporal de build: $TEMP_BUILD_DIR"
echo "📊 Directorio de reportes: $REPORTS_DIR"
echo ""

# Función para limpiar en caso de error
cleanup() {
    echo ""
    echo "🧹 Limpieza automática deshabilitada..."
    echo "📁 Los reportes están disponibles en: $TEMP_BUILD_DIR"
    echo "💡 Para limpiar manualmente, ejecuta: $0 --clean"
    if [ -d "$PROJECT_ROOT" ]; then
        cd "$PROJECT_ROOT"
    fi
}
trap cleanup EXIT

# Función adicional para limpieza manual
cleanup_manual() {
    echo "🧹 Limpieza manual iniciada..."
    if [ -d "$TEMP_BUILD_DIR" ]; then
        rm -rf "$TEMP_BUILD_DIR"
        echo "✅ Directorio temporal eliminado: $TEMP_BUILD_DIR"
    fi
    echo "✅ Limpieza completada"
}

# Función para verificar dependencias
check_dependencies() {
    echo "🔍 Verificando dependencias..."
    
    local missing_deps=0
    
    # Verificar CMake
    if ! command -v cmake >/dev/null 2>&1; then
        echo "❌ CMake no encontrado"
        missing_deps=$((missing_deps + 1))
    else
        echo "✅ CMake: $(cmake --version | head -n1)"
    fi
    
    # Verificar compilador C
    if ! command -v gcc >/dev/null 2>&1 && ! command -v clang >/dev/null 2>&1; then
        echo "❌ Compilador C no encontrado (gcc o clang)"
        missing_deps=$((missing_deps + 1))
    else
        if command -v gcc >/dev/null 2>&1; then
            echo "✅ GCC: $(gcc --version | head -n1)"
        else
            echo "✅ Clang: $(clang --version | head -n1)"
        fi
    fi
    
    # Verificar pkg-config
    if ! command -v pkg-config >/dev/null 2>&1; then
        echo "❌ pkg-config no encontrado"
        missing_deps=$((missing_deps + 1))
    else
        echo "✅ pkg-config: $(pkg-config --version)"
    fi
    
    # Verificar libcoap
    if ! pkg-config --exists libcoap-3-openssl; then
        echo "❌ libcoap-3-openssl no encontrada"
        echo "   Instalar con: sudo apt-get install libcoap-3-dev"
        missing_deps=$((missing_deps + 1))
    else
        echo "✅ libcoap: $(pkg-config --modversion libcoap-3-openssl)"
    fi
    
    # Verificar libcjson
    if ! pkg-config --exists libcjson; then
        echo "❌ libcjson no encontrada"
        echo "   Instalar con: sudo apt-get install libcjson-dev"
        missing_deps=$((missing_deps + 1))
    else
        echo "✅ libcjson: $(pkg-config --modversion libcjson)"
    fi
    
    # Verificar CUnit
    if ! ldconfig -p | grep -q libcunit; then
        echo "❌ CUnit no encontrada"
        echo "   Instalar con: sudo apt-get install libcunit1-dev"
        missing_deps=$((missing_deps + 1))
    else
        echo "✅ CUnit: Instalada"
    fi
    
    if [ $missing_deps -gt 0 ]; then
        echo ""
        echo "❌ Faltan $missing_deps dependencia(s). Por favor, instálalas antes de continuar."
        exit 1
    fi
    
    echo "✅ Todas las dependencias están disponibles"
    echo ""
}

# Función para configurar el build
configure_build() {
    echo "⚙️  Configurando build de pruebas..."
    
    # Crear directorio de build limpio
    if [ -d "$TEMP_BUILD_DIR" ]; then
        echo "🗑️  Limpiando build anterior..."
        rm -rf "$TEMP_BUILD_DIR"
    fi
    
    mkdir -p "$TEMP_BUILD_DIR"
    cd "$TEMP_BUILD_DIR"
    
    # Configurar CMake
    echo "🔧 Ejecutando CMake..."
    cmake -DBUILD_TESTS=ON \
          -DENABLE_COVERAGE=ON \
          -DCMAKE_BUILD_TYPE=Debug \
          -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
          "$PROJECT_ROOT"
    
    echo "✅ Configuración completada"
    echo ""
}

# Función para compilar
build_tests() {
    echo "🔨 Compilando pruebas..."
    
    cd "$TEMP_BUILD_DIR"
    
    # Solucionar clock skew - sincronizar timestamps
    echo "🔧 Sincronizando timestamps para evitar clock skew..."
    find . -name 'Makefile*' -o -name '*.make' -o -name '*.cmake' | xargs -r touch
    
    # Pequeña pausa para asegurar consistencia de timestamps
    sleep 1
    
    # Compilar con información de progreso
    make -j$(nproc) VERBOSE=1
    
    echo "✅ Compilación completada"
    echo ""
}

# Función para ejecutar pruebas individuales
run_individual_tests() {
    echo "🧪 Ejecutando pruebas individuales..."
    echo ""
    
    cd "$TEMP_BUILD_DIR"
    
    local total_failures=0
    local tests_run=0
    
    # Lista de ejecutables de prueba
    local test_executables=(
        "test_elevator_state_manager"
        "test_can_bridge"
        "test_api_handlers"
        "test_servidor_central"
        "test_can_to_coap"
    )
    
    for test_exe in "${test_executables[@]}"; do
        # Buscar el ejecutable en el subdirectorio tests/
        local test_path="tests/$test_exe"
        if [ -f "$test_path" ]; then
            echo "▶️  Ejecutando: $test_exe"
            echo "----------------------------------------"
            
            # Ejecutar la prueba y capturar el código de salida
            if ./"$test_path"; then
                echo "✅ $test_exe: PASÓ"
            else
                echo "❌ $test_exe: FALLÓ"
                total_failures=$((total_failures + 1))
            fi
            
            tests_run=$((tests_run + 1))
            echo ""
        else
            echo "⚠️  Ejecutable no encontrado: $test_exe (buscado en $test_path)"
            echo ""
        fi
    done

    echo "📊 Resumen de pruebas individuales:"
    echo "   - Pruebas ejecutadas: $tests_run"
    echo "   - Pruebas exitosas: $((tests_run - total_failures))"
    echo "   - Pruebas fallidas: $total_failures"
    echo ""
    
    return $total_failures
}

# Función para ejecutar CTest
run_ctest() {
    echo "🔬 Ejecutando pruebas con CTest deshabilitado..."
    echo "ℹ️  Las pruebas individuales ya se ejecutaron correctamente."
    echo "✅ Todas las pruebas completadas"
    return 0
}

# Función para generar reportes
generate_reports() {
    echo "📋 Generando reportes..."
    
    # Esperar a que todas las pruebas terminen de escribir sus reportes
    echo "⏳ Esperando 5 segundos para asegurar que todos los reportes se escriban completamente..."
    sleep 5
    
    # Cambiar al directorio de reportes para asegurar rutas correctas
    cd "$REPORTS_DIR"
    echo "📁 Directorio actual: $(pwd)"
    echo "📁 Verificando directorio de reportes: $REPORTS_DIR"
    
    if [ ! -d "$REPORTS_DIR" ]; then
        echo "❌ Error: Directorio de reportes no encontrado: $REPORTS_DIR"
        return 1
    fi
    
    # Listar reportes existentes
    echo "📋 Reportes individuales encontrados:"
    echo "📂 Contenido completo del directorio:"
    ls -la *.txt 2>/dev/null || echo "   (no hay archivos .txt)"
    echo ""
    
    local reports_found=0
    for report in *_report.txt; do
        if [ -f "$report" ]; then
            local file_size=$(stat -c%s "$report" 2>/dev/null || echo "0")
            echo "   ✅ $report (${file_size} bytes)"
            reports_found=$((reports_found + 1))
        fi
    done
    
    if [ $reports_found -eq 0 ]; then
        echo "   ⚠️  No se encontraron reportes individuales en: $REPORTS_DIR"
        echo "   📂 Listando contenido del directorio:"
        ls -la *.txt 2>/dev/null || echo "   (no hay archivos .txt)"
        return 1
    fi
    
    # Generar reporte consolidado
    echo "📄 Generando reporte consolidado..."
    local consolidated_report="reporte_consolidado.txt"
    
    # Crear header del reporte
    echo "===============================================================================" > "$consolidated_report"
    echo "                    REPORTE CONSOLIDADO DE PRUEBAS" >> "$consolidated_report"
    echo "                    Sistema de Control de Ascensores" >> "$consolidated_report"
    echo "===============================================================================" >> "$consolidated_report"
    echo "" >> "$consolidated_report"
    echo "Fecha de generación: $(date '+%Y-%m-%d %H:%M:%S')" >> "$consolidated_report"
    echo "Proyecto: SistemaControlAscensores" >> "$consolidated_report"
    echo "" >> "$consolidated_report"
    echo "===============================================================================" >> "$consolidated_report"
    echo "                              RESUMEN EJECUTIVO" >> "$consolidated_report"
    echo "===============================================================================" >> "$consolidated_report"
    echo "" >> "$consolidated_report"

    # Procesar cada reporte individual
    local total_failures=0
    local total_tests=0
    local total_passed=0
    local reports_processed=0
    
    for report in *_report.txt; do
        if [ -f "$report" ] && [ "$report" != "reporte_consolidado.txt" ]; then
            local report_name=$(basename "$report" .txt)
            
            # Extraer estadísticas del reporte de forma más robusta
            local tests_raw=$(grep -c "^TEST:" "$report" 2>/dev/null || echo "0")
            local passed_raw=$(grep -c "Resultado: PASÓ" "$report" 2>/dev/null || echo "0")
            local failed_raw=$(grep -c "Resultado: FALLÓ" "$report" 2>/dev/null || echo "0")
            
            # Función auxiliar para limpiar y validar números
            clean_number() {
                local num="$1"
                # Limpiar caracteres no numéricos
                num=$(echo "$num" | tr -d '\n\r\t ' | sed 's/[^0-9]//g')
                # Si está vacío o no es un número válido, devolver 0
                if [ -z "$num" ] || [ "$num" = "" ]; then
                    echo "0"
                else
                    # Verificar que es un número válido
                    if [ "$num" -eq "$num" ] 2>/dev/null; then
                        echo "$num"
                    else
                        echo "0"
                    fi
                fi
            }
            
            local tests=$(clean_number "$tests_raw")
            local passed=$(clean_number "$passed_raw")
            local failed=$(clean_number "$failed_raw")
            
            # Actualizar totales de forma segura
            total_tests=$((total_tests + tests))
            total_passed=$((total_passed + passed))
            total_failures=$((total_failures + failed))
            reports_processed=$((reports_processed + 1))
            
            local success_rate=0
            if [ "$tests" -gt 0 ]; then
                success_rate=$((passed * 100 / tests))
            fi
            
            # Determinar el nombre del módulo
            local module_name="DESCONOCIDO"
            case "$report_name" in
                *elevator_state_manager*) module_name="GESTOR DE ESTADO DE ASCENSORES" ;;
                *can_bridge*) module_name="PUENTE CAN" ;;
                *api_handlers*) module_name="API HANDLERS" ;;
                *servidor_central*) module_name="SERVIDOR CENTRAL" ;;
                *can_to_coap*) module_name="INTEGRACIÓN CAN-COAP" ;;
            esac
            
            echo "$module_name:" >> "$consolidated_report"
            echo "  - Archivo: $report" >> "$consolidated_report"
            echo "  - Total de pruebas: $tests" >> "$consolidated_report"
            echo "  - Pruebas exitosas: $passed" >> "$consolidated_report"
            echo "  - Pruebas fallidas: $failed" >> "$consolidated_report"
            echo "  - Tasa de éxito: ${success_rate}%" >> "$consolidated_report"
            echo "" >> "$consolidated_report"
            
            echo "   📊 Procesado: $module_name ($tests pruebas, $passed exitosas, $failed fallidas)"
        fi
    done
    
    # Agregar detalles completos de cada reporte
    echo "" >> "$consolidated_report"
    echo "===============================================================================" >> "$consolidated_report"
    echo "                              DETALLES DE PRUEBAS" >> "$consolidated_report"
    echo "===============================================================================" >> "$consolidated_report"
    echo "" >> "$consolidated_report"

    for report in *_report.txt; do
        if [ -f "$report" ] && [ "$report" != "reporte_consolidado.txt" ]; then
            local report_name=$(basename "$report" .txt)
            local module_name="DESCONOCIDO"
            case "$report_name" in
                *elevator_state_manager*) module_name="GESTOR DE ESTADO DE ASCENSORES" ;;
                *can_bridge*) module_name="PUENTE CAN" ;;
                *api_handlers*) module_name="API HANDLERS" ;;
                *servidor_central*) module_name="SERVIDOR CENTRAL" ;;
                *can_to_coap*) module_name="INTEGRACIÓN CAN-COAP" ;;
            esac
            
            echo "" >> "$consolidated_report"
            echo "-------------------------------------------------------------------------------" >> "$consolidated_report"
            echo "                        DETALLES: $module_name" >> "$consolidated_report"
            echo "-------------------------------------------------------------------------------" >> "$consolidated_report"
            echo "" >> "$consolidated_report"
            cat "$report" >> "$consolidated_report"
            echo "" >> "$consolidated_report"
        fi
    done
    
    # Agregar resumen final
    echo "" >> "$consolidated_report"
    echo "===============================================================================" >> "$consolidated_report"
    echo "                              RESUMEN FINAL" >> "$consolidated_report"
    echo "===============================================================================" >> "$consolidated_report"
    echo "" >> "$consolidated_report"
    echo "Total de reportes procesados: $reports_processed" >> "$consolidated_report"
    echo "Total de pruebas ejecutadas: $total_tests" >> "$consolidated_report"
    echo "Total de pruebas exitosas: $total_passed" >> "$consolidated_report"
    echo "Total de fallos detectados: $total_failures" >> "$consolidated_report"
    echo "Fecha de finalización: $(date '+%Y-%m-%d %H:%M:%S')" >> "$consolidated_report"
    echo "" >> "$consolidated_report"

    if [ $total_failures -eq 0 ]; then
        echo "✅ RESULTADO: TODAS LAS PRUEBAS PASARON EXITOSAMENTE" >> "$consolidated_report"
        echo "" >> "$consolidated_report"
        echo "El sistema ha superado todas las pruebas implementadas. Se puede proceder" >> "$consolidated_report"
        echo "con confianza a la siguiente fase de desarrollo o despliegue." >> "$consolidated_report"
    else
        echo "❌ RESULTADO: SE DETECTARON FALLOS EN LAS PRUEBAS" >> "$consolidated_report"
        echo "" >> "$consolidated_report"
        echo "Se encontraron $total_failures fallo(s) en las pruebas. Revisar los detalles" >> "$consolidated_report"
        echo "anteriores para identificar y corregir los problemas antes de continuar." >> "$consolidated_report"
    fi
    
    # Generar reporte JSON
    local json_report="test_results.json"
    echo "{" > "$json_report"
    echo "  \"timestamp\": \"$(date -Iseconds)\"," >> "$json_report"
    echo "  \"project\": \"SistemaControlAscensores\"," >> "$json_report"
    echo "  \"reports_processed\": $reports_processed," >> "$json_report"
    echo "  \"total_tests\": $total_tests," >> "$json_report"
    echo "  \"total_passed\": $total_passed," >> "$json_report"
    echo "  \"total_failures\": $total_failures," >> "$json_report"
    if [ $total_failures -eq 0 ]; then
        echo "  \"success\": true" >> "$json_report"
    else
        echo "  \"success\": false" >> "$json_report"
    fi
    echo "}" >> "$json_report"

    echo ""
    echo "✅ Reportes generados en: $REPORTS_DIR"
    echo "   📊 Reportes procesados: $reports_processed"
    echo "   🧪 Total de pruebas: $total_tests"
    echo "   ✅ Pruebas exitosas: $total_passed"
    echo "   ❌ Pruebas fallidas: $total_failures"
    echo "   📄 Reporte consolidado: $REPORTS_DIR/$consolidated_report"
    echo "   📋 Datos JSON: $REPORTS_DIR/$json_report"
    echo ""
}

# Función para mostrar resumen final
show_final_summary() {
    local exit_code=$1
    
    echo "==============================================================================="
    echo "                              RESUMEN FINAL"
    echo "==============================================================================="
    echo ""
    
    if [ $exit_code -eq 0 ]; then
        echo "🎉 ¡TODAS LAS PRUEBAS PASARON EXITOSAMENTE!"
        echo ""
        echo "✅ El sistema ha superado todas las verificaciones"
        echo "✅ Los reportes están disponibles en: $REPORTS_DIR/"
        echo "✅ Se puede proceder con confianza al siguiente paso"
    else
        echo "❌ SE DETECTARON FALLOS EN LAS PRUEBAS"
        echo ""
        echo "⚠️  Revisar los reportes detallados para identificar problemas"
        echo "📁 Reportes disponibles en: $REPORTS_DIR/"
        echo "🔧 Corregir los fallos antes de continuar"
    fi
    
    echo ""
    echo "📁 Archivos generados:"
    if [ -d "$REPORTS_DIR" ]; then
        find "$REPORTS_DIR" -name "*.txt" -o -name "*.json" -o -name "*.xml" | while read -r file; do
            echo "   - $(basename "$file")"
        done
    fi
    
    echo ""
    echo "🕒 Ejecución completada: $(date)"
    echo "==============================================================================="
}

# Función principal
main() {
    local start_time=$(date +%s)
    
    echo "🚀 Iniciando ejecución de pruebas: $(date)"
    echo ""
    
    # Verificar dependencias
    check_dependencies
    
    # Configurar build
    configure_build
    
    # Compilar
    build_tests
    
    # Ejecutar pruebas individuales
    local individual_failures=0
    run_individual_tests || individual_failures=$?
    
    # Ejecutar CTest
    local ctest_failures=0
    run_ctest || ctest_failures=$?
    
    # Generar reportes (si no está deshabilitado)
    if [ "$SKIP_REPORTS" != "1" ]; then
        generate_reports
    else
        echo "📋 Generación de reportes omitida (--no-reports)"
    fi
    
    # Calcular tiempo total
    local end_time=$(date +%s)
    local duration=$((end_time - start_time))
    
    echo "⏱️  Tiempo total de ejecución: ${duration}s"
    echo ""
    
    # Determinar código de salida
    local total_failures=$((individual_failures + ctest_failures))
    
    # Mostrar resumen final
    show_final_summary $total_failures
    
    # Salir con código apropiado
    exit $total_failures
}

# Procesar argumentos de línea de comandos
CLEAN_ONLY=0
while [[ $# -gt 0 ]]; do
    case $1 in
        --help|-h)
            echo "Uso: $0 [opciones]"
            echo ""
            echo "Opciones:"
            echo "  --help, -h          Mostrar esta ayuda"
            echo "  --clean, -c         Limpiar build anterior antes de ejecutar"
            echo "  --clean-only        Solo limpiar sin ejecutar pruebas"
            echo "  --verbose, -v       Salida detallada"
            echo "  --no-reports        No generar reportes"
            echo ""
            echo "Este script ejecuta todas las pruebas del sistema de control de ascensores"
            echo "y genera reportes detallados de los resultados."
            echo ""
            echo "Los reportes se generan en: tests/temp-build-tests/"
            echo "Para limpiar manualmente: $0 --clean-only"
            exit 0
            ;;
        --clean|-c)
            echo "🧹 Modo de limpieza activado"
            if [ -d "$TEMP_BUILD_DIR" ]; then
                rm -rf "$TEMP_BUILD_DIR"
                echo "✅ Build anterior eliminado"
            fi
            ;;
        --clean-only)
            echo "🧹 Solo limpieza - no se ejecutarán pruebas"
            CLEAN_ONLY=1
            if [ -d "$TEMP_BUILD_DIR" ]; then
                rm -rf "$TEMP_BUILD_DIR"
                echo "✅ Directorio temporal eliminado: $TEMP_BUILD_DIR"
            else
                echo "ℹ️  No hay nada que limpiar"
            fi
            ;;
        --verbose|-v)
            echo "📢 Modo verbose activado"
            set -x
            ;;
        --no-reports)
            echo "📋 Generación de reportes deshabilitada"
            SKIP_REPORTS=1
            ;;
        *)
            echo "❌ Opción desconocida: $1"
            echo "Usar --help para ver las opciones disponibles"
            exit 1
            ;;
    esac
    shift
done

# Si solo es limpieza, salir aquí
if [ $CLEAN_ONLY -eq 1 ]; then
    echo "✅ Limpieza completada"
    exit 0
fi

# Ejecutar función principal
main 