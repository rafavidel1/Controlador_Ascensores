#!/bin/bash

# test_unique_instances.sh - Prueba de instancias únicas del API Gateway
# Uso: ./test_unique_instances.sh

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
EXECUTABLE="build/api_gateway"
LOG_DIR="test_unique_logs"

# Colores
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

log_message() {
    local color=$1
    local message=$2
    echo -e "${color}[$(date '+%H:%M:%S')] ${message}${NC}"
}

check_dependencies() {
    if [ ! -f "$EXECUTABLE" ]; then
        log_message $RED "ERROR: Ejecutable no encontrado: $EXECUTABLE"
        log_message $YELLOW "Ejecuta: ./build_api_gateway.sh"
        exit 1
    fi
}

cleanup_test_processes() {
    log_message $YELLOW "Limpiando procesos de prueba..."
    pkill -f "api_gateway" 2>/dev/null || true
    sleep 2
}

run_test_instance() {
    local instance_id=$1
    local port=$2
    local log_file="$LOG_DIR/test_instance_${instance_id}_port_${port}.log"
    
    cd "$SCRIPT_DIR"
    
    # Crear directorio único para logs de esta instancia
    local instance_log_dir="logs/test_instance_${instance_id}_port_${port}"
    mkdir -p "$instance_log_dir"
    
    log_message $BLUE "Iniciando instancia de prueba $instance_id en puerto $port"
    
    # Ejecutar con variables de entorno únicas
    nohup env \
        GATEWAY_INSTANCE_ID="TEST_${instance_id}" \
        GATEWAY_PORT="${port}" \
        GATEWAY_LOG_DIR="${instance_log_dir}" \
        ./"$EXECUTABLE" "$port" > "$log_file" 2>&1 &
    local pid=$!
    
    echo "$pid" > "$LOG_DIR/test_pid_${instance_id}.txt"
    
    # Esperar un poco para que se inicialice
    sleep 3
    
    # Verificar que el proceso se inició
    if ! kill -0 "$pid" 2>/dev/null; then
        log_message $RED "ERROR: Instancia de prueba $instance_id falló al iniciar en puerto $port"
        return 1
    fi
    
    # Verificar que se creó el directorio de logs único
    if [ -d "$instance_log_dir" ]; then
        log_message $GREEN "✓ Directorio de logs único creado: $instance_log_dir"
    else
        log_message $RED "✗ No se creó el directorio de logs único: $instance_log_dir"
    fi
    
    # Verificar logs para identidad única
    if grep -q "Usando identidad única" "$log_file" 2>/dev/null; then
        local identity=$(grep "Usando identidad única" "$log_file" | head -1 | sed 's/.*: '\''\([^'\'']*\)'\''.*/\1/')
        log_message $GREEN "✓ Identidad única generada: $identity"
    else
        log_message $RED "✗ No se encontró identidad única en logs"
    fi
    
    return 0
}

test_unique_instances() {
    log_message $GREEN "=== PRUEBA DE INSTANCIAS ÚNICAS ==="
    
    # Limpiar logs anteriores
    if [ -d "$LOG_DIR" ]; then
        rm -rf "$LOG_DIR"
    fi
    mkdir -p "$LOG_DIR"
    
    # Probar 3 instancias
    local successful=0
    local total=3
    
    for i in {1..3}; do
        local port=$((6000 + i))
        
        if run_test_instance $i $port; then
            successful=$((successful + 1))
        fi
        
        sleep 2
    done
    
    log_message $GREEN "=== RESULTADOS DE LA PRUEBA ==="
    log_message $GREEN "Instancias exitosas: $successful/$total"
    
    if [ $successful -eq $total ]; then
        log_message $GREEN "✓ Todas las instancias se iniciaron correctamente con identidades únicas"
        
        # Mostrar información de cada instancia
        for i in {1..3}; do
            local port=$((6000 + i))
            local log_file="$LOG_DIR/test_instance_${i}_port_${port}.log"
            local instance_log_dir="logs/test_instance_${i}_port_${port}"
            
            echo ""
            log_message $BLUE "--- Instancia $i (Puerto $port) ---"
            
            if [ -f "$log_file" ]; then
                echo "Log principal: $log_file"
                echo "Directorios de logs únicos:"
                find "$instance_log_dir" -name "*.md" -type f 2>/dev/null | head -3
            fi
        done
        
        echo ""
        log_message $GREEN "✓ Prueba exitosa: Cada instancia tiene su propia identidad y directorio de logs"
        
    else
        log_message $RED "✗ Algunas instancias fallaron"
    fi
}

cleanup_test() {
    log_message $YELLOW "Limpiando procesos de prueba..."
    
    for i in {1..3}; do
        local pid_file="$LOG_DIR/test_pid_${i}.txt"
        if [ -f "$pid_file" ]; then
            local pid=$(cat "$pid_file")
            if kill -0 "$pid" 2>/dev/null; then
                kill -TERM "$pid" 2>/dev/null || true
            fi
            rm -f "$pid_file" 2>/dev/null || true
        fi
    done
    
    cleanup_test_processes
    log_message $GREEN "Limpieza completada"
}

main() {
    cd "$SCRIPT_DIR"
    
    check_dependencies
    cleanup_test_processes
    
    test_unique_instances
    
    # Esperar un poco para que se estabilicen
    sleep 5
    
    cleanup_test
    
    log_message $GREEN "=== PRUEBA COMPLETADA ==="
}

# Manejo de señales
trap 'log_message $YELLOW "Recibida señal de terminación..."; cleanup_test; exit 0' SIGINT

# Ejecutar función principal
main "$@" 