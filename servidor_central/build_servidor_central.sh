#!/bin/bash

# =============================
# SERVIDOR CENTRAL BUILD SCRIPT
# =============================
# 100% Autónomo - Instala todas las dependencias automáticamente
# Sin prerrequisitos manuales - Zero configuration
# NO incluye SQLite - Solo algoritmo inteligente en memoria

# Colores para la salida
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # Sin color

# Directorios base
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BASE_DIR="$SCRIPT_DIR"
BUILD_DIR="$BASE_DIR/build"

log_message() {
    local color=$1
    local message=$2
    echo -e "${color}[$(date '+%H:%M:%S')] ${message}${NC}"
}

# Función para ejecutar comandos con logging
run_command() {
    local cmd="$1"
    local desc="$2"
    
    log_message $BLUE "$desc"
    if eval "$cmd"; then
        log_message $GREEN "✅ $desc - Completado"
        return 0
    else
        log_message $RED "❌ $desc - Error"
        return 1
    fi
}

# Función para verificar si un comando existe
command_exists() {
    command -v "$1" >/dev/null 2>&1
}

# Función para verificar si una librería está instalada
library_exists() {
    pkg-config --exists "$1" 2>/dev/null
}

# Función para instalar dependencias básicas del sistema
install_system_dependencies() {
    log_message $BLUE "🔧 Instalando dependencias básicas del sistema..."
    
    # Actualizar repositorios
    run_command "sudo apt-get update" "Actualizando repositorios de paquetes"
    
    # Instalar herramientas básicas de compilación
    run_command "sudo apt-get install -y build-essential cmake gcc make pkg-config git" "Instalando herramientas de compilación básicas"
    
    # Instalar herramientas adicionales para libcoap
    run_command "sudo apt-get install -y libtool autoconf automake wget ca-certificates" "Instalando herramientas adicionales"
    
    # Instalar OpenSSL
    run_command "sudo apt-get install -y libssl-dev" "Instalando OpenSSL"
    
    # Instalar cJSON
    run_command "sudo apt-get install -y libcjson-dev" "Instalando cJSON"
    
    # Instalar json-c como alternativa
    run_command "sudo apt-get install -y libjson-c-dev" "Instalando json-c"
    
    # Instalar libcurl (para comunicaciones HTTP si es necesario)
    run_command "sudo apt-get install -y libcurl4-openssl-dev" "Instalando libcurl"
    
    # Limpiar cache de apt
    run_command "sudo apt-get clean && sudo apt-get autoclean" "Limpiando cache de paquetes"
    
    log_message $GREEN "✅ Todas las dependencias básicas instaladas correctamente"
}

# Función para instalar libcoap desde fuente
install_libcoap() {
    log_message $BLUE "🔧 Instalando libcoap desde fuente..."
    
    # Crear directorio temporal
    local temp_dir="/tmp/libcoap-build-$$"
    mkdir -p "$temp_dir"
    
    # Clonar repositorio
    run_command "git clone --depth=1 https://github.com/obgm/libcoap.git $temp_dir" "Clonando repositorio libcoap"
    
    # Cambiar al directorio
    cd "$temp_dir" || { log_message $RED "Error: No se pudo cambiar al directorio temporal"; return 1; }
    
    # Compilar e instalar
    run_command "./autogen.sh" "Ejecutando autogen.sh"
    run_command "./configure --prefix=/usr/local --enable-dtls --with-openssl --disable-doxygen --disable-manpages" "Configurando libcoap"
    run_command "make -j$(nproc)" "Compilando libcoap"
    run_command "sudo make install" "Instalando libcoap"
    run_command "sudo ldconfig" "Actualizando cache de librerías"
    
    # Limpiar directorio temporal
    cd / && rm -rf "$temp_dir"
    
    # Configurar variables de entorno
    export PKG_CONFIG_PATH="/usr/local/lib/pkgconfig:$PKG_CONFIG_PATH"
    export LD_LIBRARY_PATH="/usr/local/lib:$LD_LIBRARY_PATH"
    
    log_message $GREEN "✅ libcoap instalado correctamente en /usr/local/"
}

# Función principal de verificación e instalación de dependencias
install_all_dependencies() {
    log_message $BLUE "🚀 === INSTALACIÓN AUTOMÁTICA DE DEPENDENCIAS ==="
    log_message $YELLOW "ℹ️  Nota: Este servidor NO usa SQLite - Solo algoritmo inteligente en memoria"
    
    # 1. Verificar e instalar dependencias básicas
    if ! command_exists cmake || ! command_exists make || ! command_exists pkg-config || ! command_exists gcc; then
        log_message $YELLOW "Algunas dependencias básicas no están instaladas. Instalando..."
        install_system_dependencies
    else
        log_message $GREEN "✅ Dependencias básicas del sistema verificadas"
    fi
    
    # 2. Verificar e instalar libcoap
    if ! library_exists "libcoap-3"; then
        log_message $YELLOW "libcoap no está instalado. Instalando desde fuente..."
        install_libcoap
    else
        local version=$(pkg-config --modversion libcoap-3 2>/dev/null)
        log_message $GREEN "✅ libcoap ya está instalado (versión: $version)"
        # Configurar variables de entorno por si acaso
        export PKG_CONFIG_PATH="/usr/local/lib/pkgconfig:$PKG_CONFIG_PATH"
        export LD_LIBRARY_PATH="/usr/local/lib:$LD_LIBRARY_PATH"
    fi
    
    # 3. Verificar cJSON
    if ! library_exists "libcjson"; then
        log_message $YELLOW "cJSON no está instalado. Instalando..."
        run_command "sudo apt-get install -y libcjson-dev" "Instalando cJSON"
    else
        local version=$(pkg-config --modversion libcjson 2>/dev/null)
        log_message $GREEN "✅ cJSON ya está instalado (versión: $version)"
    fi
    
    # 4. Verificar OpenSSL
    if ! library_exists "openssl"; then
        log_message $YELLOW "OpenSSL no está instalado. Instalando..."
        run_command "sudo apt-get install -y libssl-dev" "Instalando OpenSSL"
    else
        local version=$(pkg-config --modversion openssl 2>/dev/null)
        log_message $GREEN "✅ OpenSSL ya está instalado (versión: $version)"
    fi
    
    # 5. Verificar json-c (opcional)
    if ! library_exists "json-c"; then
        log_message $YELLOW "json-c no está instalado. Instalando..."
        run_command "sudo apt-get install -y libjson-c-dev" "Instalando json-c"
    else
        local version=$(pkg-config --modversion json-c 2>/dev/null)
        log_message $GREEN "✅ json-c ya está instalado (versión: $version)"
    fi
    
    # 6. Verificar libcurl (opcional)
    if ! library_exists "libcurl"; then
        log_message $YELLOW "libcurl no está instalado. Instalando..."
        run_command "sudo apt-get install -y libcurl4-openssl-dev" "Instalando libcurl"
    else
        local version=$(pkg-config --modversion libcurl 2>/dev/null)
        log_message $GREEN "✅ libcurl ya está instalado (versión: $version)"
    fi
    
    log_message $GREEN "🎉 === TODAS LAS DEPENDENCIAS INSTALADAS CORRECTAMENTE ==="
    log_message $GREEN "✅ Algoritmo inteligente en memoria - Sin base de datos"
}

# Función para compilar el proyecto
build_project() {
    log_message $BLUE "🔨 === COMPILANDO SERVIDOR CENTRAL ==="
    
    # Crear directorio build
    if [ ! -d "$BUILD_DIR" ]; then
        mkdir -p "$BUILD_DIR"
    fi
    
    cd "$BUILD_DIR" || { log_message $RED "Error: No se pudo cambiar al directorio $BUILD_DIR."; exit 1; }
    
    # Limpiar directorio build
    run_command "rm -rf $BUILD_DIR/*" "Limpiando directorio build"
    
    # Configurar variables de entorno
    export PKG_CONFIG_PATH="/usr/local/lib/pkgconfig:$PKG_CONFIG_PATH"
    export LD_LIBRARY_PATH="/usr/local/lib:$LD_LIBRARY_PATH"
    
    # Ejecutar cmake
    run_command "cmake -S .. -B ." "Ejecutando cmake"
    
    # Solucionar clock skew - sincronizar timestamps
    run_command "find . -name 'Makefile*' -o -name '*.make' -o -name '*.cmake' | xargs -r touch" "Sincronizando timestamps para evitar clock skew"
    
    # Pequeña pausa para asegurar consistencia de timestamps
    sleep 1
    
    # Ejecutar make
    run_command "make servidor_central" "Compilando Servidor Central"
    
    # Copiar archivos necesarios
    if [ -f "../psk_keys.txt" ]; then
        run_command "cp ../psk_keys.txt ." "Copiando archivo de claves PSK"
    fi
    
    # Verificar que el ejecutable existe
    if [ -f "$BUILD_DIR/servidor_central" ]; then
        log_message $GREEN "✅ Ejecutable servidor_central creado exitosamente"
        log_message $GREEN "📍 Ubicación: $BUILD_DIR/servidor_central"
    else
        log_message $RED "❌ Error: No se pudo crear el ejecutable servidor_central"
        exit 1
    fi
    
    log_message $GREEN "🎉 === COMPILACIÓN COMPLETADA EXITOSAMENTE ==="
}

# Función para ejecutar el servidor central
run_servidor_central() {
    log_message $BLUE "🚀 === EJECUTANDO SERVIDOR CENTRAL ==="
    
    cd "$BUILD_DIR" || { log_message $RED "Error: No se pudo cambiar al directorio $BUILD_DIR."; exit 1; }
    
    # Configurar variables de entorno
    export PKG_CONFIG_PATH="/usr/local/lib/pkgconfig:$PKG_CONFIG_PATH"
    export LD_LIBRARY_PATH="/usr/local/lib:$LD_LIBRARY_PATH"
    
    log_message $GREEN "🎯 Ejecutando Servidor Central..."
    log_message $GREEN "🧠 Algoritmo inteligente activado - Sin base de datos"
    log_message $YELLOW "Para detener el servidor, presiona Ctrl+C"
    log_message $YELLOW "Para desplegar en Kubernetes, usa: ./deploy.sh"
    
    # Ejecutar el servidor central
    ./servidor_central
}

# Función para verificar minikube
check_minikube() {
    log_message $BLUE "🔍 Verificando disponibilidad de minikube..."
    
    if command_exists minikube; then
        local status=$(minikube status --format='{{.Host}}' 2>/dev/null)
        if [ "$status" = "Running" ]; then
            log_message $GREEN "✅ minikube está ejecutándose"
        else
            log_message $YELLOW "⚠️  minikube está instalado pero no está ejecutándose"
            log_message $YELLOW "💡 Para iniciarlo: minikube start"
        fi
    else
        log_message $YELLOW "⚠️  minikube no está instalado"
        log_message $YELLOW "💡 Para instalarlo: curl -LO https://storage.googleapis.com/minikube/releases/latest/minikube-linux-amd64"
        log_message $YELLOW "💡 sudo install minikube-linux-amd64 /usr/local/bin/minikube"
    fi
    
    if command_exists kubectl; then
        log_message $GREEN "✅ kubectl está disponible"
    else
        log_message $YELLOW "⚠️  kubectl no está instalado"
        log_message $YELLOW "💡 Para instalarlo: sudo apt-get install -y kubectl"
    fi
}

# FUNCIÓN PRINCIPAL
main() {
    log_message $BLUE "🌟 === SERVIDOR CENTRAL BUILD SCRIPT - 100% AUTÓNOMO ==="
    log_message $BLUE "🔧 Sin prerrequisitos manuales - Instalación automática de dependencias"
    log_message $BLUE "🧠 Algoritmo inteligente en memoria - Sin base de datos SQLite"
    
    # Verificar que estamos en el directorio correcto
    if [ ! -f "CMakeLists.txt" ]; then
        log_message $RED "Error: Este script debe ejecutarse desde el directorio servidor_central"
        exit 1
    fi
    
    # Paso 1: Instalar todas las dependencias automáticamente
    install_all_dependencies
    
    # Paso 2: Compilar el proyecto
    build_project
    
    # Paso 3: Verificar herramientas de Kubernetes
    check_minikube
    
    # Paso 4: Mostrar opciones de ejecución
    log_message $BLUE "🚀 === LISTO PARA EJECUTAR ==="
    log_message $GREEN "✅ Servidor Central compilado exitosamente"
    log_message $GREEN "📍 Ejecutable disponible en: $BUILD_DIR/servidor_central"
    log_message $GREEN "🎯 Para ejecutar manualmente: $BUILD_DIR/servidor_central"
    log_message $GREEN "🐳 Para desplegar en Kubernetes: ./deploy.sh"
    
    echo
    log_message $YELLOW "Opciones de ejecución:"
    log_message $YELLOW "  [1] Ejecutar servidor localmente"
    log_message $YELLOW "  [2] Desplegar en Kubernetes (requiere minikube)"
    log_message $YELLOW "  [3] Solo compilar, no ejecutar"
    echo -n "Selecciona una opción (1-3): "
    read -r choice
    
    case $choice in
        1)
            run_servidor_central
            ;;
        2)
            if command_exists minikube && command_exists kubectl; then
                log_message $GREEN "🐳 Desplegando en Kubernetes..."
                if [ -f "./deploy.sh" ]; then
                    ./deploy.sh
                else
                    log_message $RED "❌ No se encontró ./deploy.sh"
                fi
            else
                log_message $RED "❌ minikube y kubectl son necesarios para desplegar en Kubernetes"
            fi
            ;;
        3)
            log_message $GREEN "👍 Servidor Central listo para ejecutar cuando quieras"
            ;;
        *)
            log_message $GREEN "👍 Servidor Central listo para ejecutar cuando quieras"
            ;;
    esac
}

# Ejecutar función principal
main "$@" 