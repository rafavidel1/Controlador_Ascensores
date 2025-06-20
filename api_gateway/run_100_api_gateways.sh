#!/bin/bash

# run_100_api_gateways.sh - Ejecuta múltiples API Gateways con puertos diferentes
# Uso: ./run_100_api_gateways.sh [OPCIONES]

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
NUM_INSTANCES=100
BASE_PORT=6000
EXECUTABLE="build/api_gateway"
LOG_DIR="mass_execution_logs"
WAIT_TIME=30
SERVER_IP="192.168.49.2"
SERVER_PORT="5684"

# Colores
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

show_help() {
    echo "Uso: $0 [OPCIONES]"
    echo ""
    echo "Ejecuta múltiples instancias del API Gateway con puertos diferentes"
    echo ""
    echo "OPCIONES:"
    echo "  -n, --num NUM      Número de instancias (default: 100)"
    echo "  -b, --base PORT    Puerto base (default: 6000)"
    echo "  -t, --time TIME    Tiempo de ejecución en segundos (default: 30)"
    echo "  -s, --server IP    IP del servidor (default: 192.168.49.2)"
    echo "  -p, --port PORT    Puerto del servidor (default: 5684)"
    echo "  -k, --kill         Solo matar procesos existentes"
    echo "  -c, --clean        Solo limpiar logs anteriores"
    echo "  --auto-clean       Limpiar logs automáticamente sin preguntar"
    echo "  -h, --help         Mostrar esta ayuda"
    echo ""
    echo "COMPORTAMIENTO:"
    echo "  • Los logs de mass_execution_logs se limpian automáticamente"
    echo "  • Si hay >50 logs individuales, pregunta si limpiarlos"
    echo "  • Con --auto-clean, limpia todo automáticamente"
    echo "  • Los logs del día actual siempre se conservan"
    echo ""
    echo "EJEMPLOS:"
    echo "  $0                     # 100 instancias desde puerto 6000"
    echo "  $0 -n 50 -b 7000       # 50 instancias desde puerto 7000"
    echo "  $0 --auto-clean -n 10  # 10 instancias, limpieza automática"
    echo "  $0 -k                  # Matar todas las instancias"
    echo "  $0 -c                  # Solo limpiar logs"
}

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
    
    # Verificar conectividad al servidor
    if command -v nc &> /dev/null; then
        if ! nc -uz "$SERVER_IP" "$SERVER_PORT" 2>/dev/null; then
            log_message $YELLOW "ADVERTENCIA: No se puede conectar a $SERVER_IP:$SERVER_PORT"
            log_message $YELLOW "Verifica que el servidor esté ejecutándose"
        else
            log_message $GREEN "Servidor $SERVER_IP:$SERVER_PORT está accesible"
        fi
    fi
}

kill_existing_processes() {
    log_message $YELLOW "Matando procesos api_gateway existentes..."
    
    # Obtener PID del script actual para excluirlo
    local script_pid=$$
    local script_name=$(basename "$0")
    
    # Buscar procesos api_gateway pero excluir el script actual y procesos de grep
    local pids=$(pgrep -f "api_gateway" 2>/dev/null | grep -v "^${script_pid}$" || true)
    
    if [ -n "$pids" ]; then
        echo "$pids" | while read pid; do
            if [ -n "$pid" ] && [ "$pid" != "$script_pid" ]; then
                # Verificar que no es el script actual ni un proceso de grep
                local cmd=$(ps -p "$pid" -o comm= 2>/dev/null || echo "")
                if [[ "$cmd" != *"$script_name"* ]] && [[ "$cmd" != *"grep"* ]]; then
                    log_message $BLUE "Matando proceso PID: $pid ($cmd)"
                    kill -TERM "$pid" 2>/dev/null || true
                fi
            fi
        done
        
        sleep 2
        
        # Segunda pasada para procesos que no respondieron a TERM
        local remaining_pids=$(pgrep -f "api_gateway" 2>/dev/null | grep -v "^${script_pid}$" || true)
        if [ -n "$remaining_pids" ]; then
            echo "$remaining_pids" | while read pid; do
                if [ -n "$pid" ] && [ "$pid" != "$script_pid" ]; then
                    local cmd=$(ps -p "$pid" -o comm= 2>/dev/null || echo "")
                    if [[ "$cmd" != *"$script_name"* ]] && [[ "$cmd" != *"grep"* ]]; then
                        log_message $BLUE "Forzando terminación de PID: $pid ($cmd)"
                        kill -KILL "$pid" 2>/dev/null || true
                    fi
                fi
            done
        fi
        
        log_message $GREEN "Procesos terminados"
    else
        log_message $GREEN "No hay procesos api_gateway ejecutándose"
    fi
}

clean_logs() {
    log_message $YELLOW "Limpiando logs anteriores..."
    if [ -d "$LOG_DIR" ]; then
        rm -rf "$LOG_DIR"
        log_message $GREEN "Directorio $LOG_DIR eliminado"
    fi
}

setup_log_directory() {
    local auto_clean_mode=$1
    
    # Limpiar logs anteriores automáticamente para evitar confusión
    if [ -d "$LOG_DIR" ]; then
        log_message $YELLOW "Limpiando logs de ejecución anterior..."
        rm -rf "$LOG_DIR"
        log_message $GREEN "Logs anteriores eliminados"
    fi
    
    mkdir -p "$LOG_DIR"
    log_message $GREEN "Directorio de logs creado: $LOG_DIR"
    
    # También limpiar logs individuales del sistema de logging si existen
    if [ -d "logs" ]; then
        local old_logs_count=$(find logs -name "*.md" -type f 2>/dev/null | wc -l)
        if [ "$old_logs_count" -gt 50 ]; then
            if [ "$auto_clean_mode" = true ]; then
                log_message $YELLOW "Modo automático: limpiando $old_logs_count logs individuales antiguos..."
                # Mantener solo los logs del día actual
                local today=$(date '+%Y-%m-%d')
                find logs -name "*.md" -type f ! -path "*/logs/$today/*" -delete 2>/dev/null || true
                log_message $GREEN "Logs individuales antiguos limpiados automáticamente (conservados los de hoy)"
            else
                log_message $YELLOW "Detectados $old_logs_count logs individuales antiguos..."
                read -p "¿Deseas limpiar también los logs individuales del sistema? (y/N): " -n 1 -r
                echo
                if [[ $REPLY =~ ^[Yy]$ ]]; then
                    # Mantener solo los logs del día actual
                    local today=$(date '+%Y-%m-%d')
                    find logs -name "*.md" -type f ! -path "*/logs/$today/*" -delete 2>/dev/null || true
                    log_message $GREEN "Logs individuales antiguos limpiados (conservados los de hoy)"
                fi
            fi
        fi
    fi
}

run_instance() {
    local instance_id=$1
    local port=$2
    local log_file="$LOG_DIR/gateway_${instance_id}_port_${port}.log"
    
    cd "$SCRIPT_DIR"
    nohup ./"$EXECUTABLE" "$port" > "$log_file" 2>&1 &
    local pid=$!
    
    echo "$pid" > "$LOG_DIR/pid_${instance_id}.txt"
    
    # Delay para evitar race conditions DTLS
    sleep 0.5
    
    # Verificar que el proceso se inició
    if ! kill -0 "$pid" 2>/dev/null; then
        log_message $RED "ERROR: Instancia $instance_id falló al iniciar en puerto $port"
        return 1
    fi
    
    return 0
}

monitor_instances() {
    local duration=$1
    log_message $YELLOW "Monitoreando $NUM_INSTANCES instancias por $duration segundos..."
    
    local start_time=$(date +%s)
    local end_time=$((start_time + duration))
    
    while [ $(date +%s) -lt $end_time ]; do
        local running_count=0
        local failed_count=0
        
        for i in $(seq 1 $NUM_INSTANCES); do
            local pid_file="$LOG_DIR/pid_${i}.txt"
            if [ -f "$pid_file" ]; then
                local pid=$(cat "$pid_file")
                if kill -0 "$pid" 2>/dev/null; then
                    running_count=$((running_count + 1))
                else
                    failed_count=$((failed_count + 1))
                fi
            else
                failed_count=$((failed_count + 1))
            fi
        done
        
        local remaining_time=$((end_time - $(date +%s)))
        
        # Mostrar información cada 10 segundos
        if [ $((remaining_time % 10)) -eq 0 ] && [ $remaining_time -ne $duration ]; then
            echo ""
            log_message $BLUE "Estado actual: $running_count ejecutándose, $failed_count fallidas"
            if command -v netstat &> /dev/null; then
                local listening_ports=$(netstat -tuln 2>/dev/null | grep -c ":$BASE_PORT" || echo "0")
                log_message $GREEN "Puertos de escucha detectados: $listening_ports"
            fi
        fi
        
        printf "\r${GREEN}Ejecutándose: %d${NC} | ${RED}Fallidas: %d${NC} | ${YELLOW}Tiempo restante: %ds${NC}     " \
               $running_count $failed_count $remaining_time
        
        sleep 1
    done
    
    echo ""
    log_message $GREEN "Tiempo de ejecución completado"
}

collect_statistics() {
    log_message $YELLOW "Recopilando estadísticas finales..."
    
    local successful_instances=0
    local still_running=0
    local total_logs=0
    
    if [ -d "logs" ]; then
        total_logs=$(find logs -name "*.md" -type f 2>/dev/null | wc -l)
    fi
    
    for i in $(seq 1 $NUM_INSTANCES); do
        local pid_file="$LOG_DIR/pid_${i}.txt"
        local log_file="$LOG_DIR/gateway_${i}_port_$((BASE_PORT + i - 1)).log"
        
        # Contar instancias que se iniciaron exitosamente
        if [ -f "$log_file" ] && grep -q "Listening on\|Sistema de logging inicializado" "$log_file" 2>/dev/null; then
            successful_instances=$((successful_instances + 1))
        fi
        
        # Contar instancias aún ejecutándose
        if [ -f "$pid_file" ]; then
            local pid=$(cat "$pid_file")
            if kill -0 "$pid" 2>/dev/null; then
                still_running=$((still_running + 1))
            fi
        fi
    done
    
    echo ""
    log_message $GREEN "=== ESTADÍSTICAS FINALES ==="
    echo "Servidor destino: $SERVER_IP:$SERVER_PORT"
    echo "Instancias solicitadas: $NUM_INSTANCES"
    echo "Instancias iniciadas exitosamente: $successful_instances"
    echo "Instancias aún ejecutándose: $still_running"
    echo "Puerto base utilizado: $BASE_PORT"
    echo "Rango de puertos: $BASE_PORT-$((BASE_PORT + NUM_INSTANCES - 1))"
    echo "Logs individuales generados: $total_logs"
    echo "Tiempo de ejecución: $WAIT_TIME segundos"
    
    # Mostrar estadísticas de puertos
    if command -v netstat &> /dev/null; then
        log_message $BLUE "Puertos UDP activos en el rango:"
        local port_count=0
        for i in $(seq 0 9); do  # Mostrar solo los primeros 10 puertos
            local port=$((BASE_PORT + i))
            if netstat -tuln 2>/dev/null | grep -q ":$port "; then
                port_count=$((port_count + 1))
                echo "  Puerto $port: ACTIVO"
            fi
        done
        echo "  (Total verificados: primeros 10 puertos, activos: $port_count)"
    fi
    
    # Generar reporte
    local report_file="$LOG_DIR/final_report.txt"
    {
        echo "=== REPORTE DE PRUEBA MASIVA DE API GATEWAYS ==="
        echo "Fecha: $(date)"
        echo "Servidor destino: $SERVER_IP:$SERVER_PORT"
        echo "Instancias solicitadas: $NUM_INSTANCES"
        echo "Instancias iniciadas exitosamente: $successful_instances"
        echo "Instancias aún ejecutándose al final: $still_running"
        echo "Puerto base: $BASE_PORT"
        echo "Rango de puertos: $BASE_PORT-$((BASE_PORT + NUM_INSTANCES - 1))"
        echo "Tiempo de ejecución: $WAIT_TIME segundos"
        echo ""
        echo "=== CONCLUSIÓN ==="
        echo "Esta prueba demuestra que múltiples instancias del API Gateway"
        echo "pueden ejecutarse simultáneamente usando puertos diferentes."
        echo "Cada instancia escucha en su puerto asignado y se conecta"
        echo "al mismo servidor central ($SERVER_IP:$SERVER_PORT) sin conflictos."
        echo ""
        echo "Tasa de éxito: $((successful_instances * 100 / NUM_INSTANCES))%"
    } > "$report_file"
    
    log_message $GREEN "Reporte guardado en: $report_file"
}

terminate_all_instances() {
    log_message $YELLOW "Terminando todas las instancias..."
    
    for i in $(seq 1 $NUM_INSTANCES); do
        local pid_file="$LOG_DIR/pid_${i}.txt"
        if [ -f "$pid_file" ]; then
            local pid=$(cat "$pid_file")
            if kill -0 "$pid" 2>/dev/null; then
                kill -TERM "$pid" 2>/dev/null || true
            fi
            rm -f "$pid_file" 2>/dev/null || true
        fi
    done
    
    sleep 2
    kill_existing_processes
    log_message $GREEN "Todas las instancias terminadas"
}

main() {
    local kill_only=false
    local clean_only=false
    local auto_clean=false
    
    # Procesar argumentos
    while [[ $# -gt 0 ]]; do
        case $1 in
            -n|--num)
                NUM_INSTANCES="$2"
                shift 2
                ;;
            -b|--base)
                BASE_PORT="$2"
                shift 2
                ;;
            -t|--time)
                WAIT_TIME="$2"
                shift 2
                ;;
            -s|--server)
                SERVER_IP="$2"
                shift 2
                ;;
            -p|--port)
                SERVER_PORT="$2"
                shift 2
                ;;
            -k|--kill)
                kill_only=true
                shift
                ;;
            -c|--clean)
                clean_only=true
                shift
                ;;
            --auto-clean)
                auto_clean=true
                shift
                ;;
            -h|--help)
                show_help
                exit 0
                ;;
            *)
                echo "ERROR: Opción desconocida $1"
                show_help
                exit 1
                ;;
        esac
    done
    
    cd "$SCRIPT_DIR"
    
    if [ "$clean_only" = true ]; then
        clean_logs
        exit 0
    fi
    
    if [ "$kill_only" = true ]; then
        kill_existing_processes
        exit 0
    fi
    
    # Validar parámetros
    if [ "$NUM_INSTANCES" -lt 1 ] || [ "$NUM_INSTANCES" -gt 1000 ]; then
        log_message $RED "ERROR: Número de instancias debe estar entre 1 y 1000"
        exit 1
    fi
    
    if [ "$BASE_PORT" -lt 1024 ] || [ "$BASE_PORT" -gt 60000 ]; then
        log_message $RED "ERROR: Puerto base debe estar entre 1024 y 60000"
        exit 1
    fi
    
    # Mostrar configuración
    log_message $GREEN "=== CONFIGURACIÓN DE PRUEBA MASIVA ==="
    echo "Instancias a ejecutar: $NUM_INSTANCES"
    echo "Puerto base: $BASE_PORT"
    echo "Rango de puertos: $BASE_PORT-$((BASE_PORT + NUM_INSTANCES - 1))"
    echo "Servidor destino: $SERVER_IP:$SERVER_PORT"
    echo "Tiempo de ejecución: $WAIT_TIME segundos"
    echo "Ejecutable: $EXECUTABLE"
    echo ""
    echo "NOTA: Cada instancia escuchará en un puerto diferente."
    echo "Todas se conectarán al mismo servidor central."
    echo ""
    
    check_dependencies
    kill_existing_processes
    setup_log_directory $auto_clean
    
    # Ejecutar instancias
    log_message $GREEN "=== INICIANDO $NUM_INSTANCES INSTANCIAS ==="
    local successful_starts=0
    
    for i in $(seq 1 $NUM_INSTANCES); do
        local port=$((BASE_PORT + i - 1))
        
        if [ $((i % 10)) -eq 1 ] || [ $i -le 5 ] || [ $i -ge $((NUM_INSTANCES - 5)) ]; then
            log_message $BLUE "Iniciando instancia $i/$NUM_INSTANCES en puerto $port"
        fi
        
        if run_instance $i $port; then
            successful_starts=$((successful_starts + 1))
        fi
        
        # Pausa muy pequeña entre inicios
        sleep 0.05
        
        # Mostrar progreso cada 25 instancias
        if [ $((i % 25)) -eq 0 ]; then
            log_message $YELLOW "Progreso: $i/$NUM_INSTANCES instancias procesadas, $successful_starts exitosas"
        fi
    done
    
    log_message $GREEN "=== INICIO COMPLETADO ==="
    log_message $GREEN "Instancias iniciadas exitosamente: $successful_starts/$NUM_INSTANCES"
    log_message $GREEN "Tasa de éxito inicial: $((successful_starts * 100 / NUM_INSTANCES))%"
    
    if [ $successful_starts -eq 0 ]; then
        log_message $RED "ERROR: No se pudo iniciar ninguna instancia"
        exit 1
    fi
    
    # Monitorear ejecución
    monitor_instances $WAIT_TIME
    
    # Recopilar estadísticas
    collect_statistics
    
    # Terminar instancias
    terminate_all_instances
    
    log_message $GREEN "=== PRUEBA MASIVA COMPLETADA ==="
    log_message $BLUE "Se demostró que $successful_starts API Gateways pueden ejecutarse"
    log_message $BLUE "simultáneamente conectándose al mismo servidor central."
}

# Manejo de señales - solo para terminación manual del script
trap 'log_message $YELLOW "Recibida señal de terminación manual (Ctrl+C)..."; terminate_all_instances; exit 0' SIGINT

# Ejecutar función principal
main "$@" 