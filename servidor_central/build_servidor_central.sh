#!/bin/bash

# Colores para la salida
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # Sin color

# Directorios base - usando rutas relativas
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BASE_DIR="$SCRIPT_DIR"
BUILD_DIR="$BASE_DIR/build"

log_message() {
    local color=$1
    local message=$2
    echo -e "${color}[$(date '+%H:%M:%S')] ${message}${NC}"
}

# Función para verificar comandos
check_command() {
    if ! command -v $1 &> /dev/null; then
        log_message $RED "Error: $1 no está instalado."
        return 1
    fi
    return 0
}

# Función para verificar librerías instaladas
check_library() {
    local lib_name=$1
    local pkg_name=$2
    local install_cmd=$3
    
    if pkg-config --exists "$pkg_name" 2>/dev/null; then
        local version=$(pkg-config --modversion "$pkg_name" 2>/dev/null)
        log_message $GREEN "$lib_name ya está instalada (versión: $version)"
        return 0
    else
        log_message $RED "$lib_name no está instalada"
        log_message $YELLOW "Comando sugerido para instalar: $install_cmd"
        echo -n "¿Deseas instalar $lib_name automáticamente? (s/n): "
        read answer
        if [ "$answer" = "s" ] || [ "$answer" = "S" ]; then
            eval $install_cmd
            if [ $? -eq 0 ]; then
                log_message $GREEN "$lib_name instalada correctamente"
                return 0
            else
                log_message $RED "Error al instalar $lib_name"
                return 1
            fi
        else
            return 1
        fi
    fi
}

# Función para instalar libcoap (adaptada del Dockerfile)
install_libcoap() {
    log_message $BLUE "Instalando dependencias necesarias para libcoap..."
    
    # Instalar dependencias de compilación
    sudo apt-get update && sudo apt-get install -y \
        build-essential \
        cmake \
        gcc \
        make \
        pkg-config \
        libtool \
        autoconf \
        automake \
        git \
        libssl-dev \
        libcjson-dev \
        libcurl4-openssl-dev \
        libjson-c-dev \
        wget \
        ca-certificates \
        && sudo apt-get clean && sudo apt-get autoclean
    
    if [ $? -ne 0 ]; then
        log_message $RED "Error instalando dependencias para libcoap"
        return 1
    fi

    log_message $BLUE "Clonando y compilando libcoap..."
    # Instalar libcoap desde fuente (igual que en Dockerfile)
    git clone --depth=1 https://github.com/obgm/libcoap.git /tmp/libcoap && \
        cd /tmp/libcoap && \
        ./autogen.sh && \
        ./configure --prefix=/usr/local --enable-dtls --with-openssl --disable-doxygen --disable-manpages && \
        make -j$(nproc) && \
        sudo make install && \
        sudo ldconfig && \
        rm -rf /tmp/libcoap
    
    if [ $? -eq 0 ]; then
        log_message $GREEN "libcoap instalado correctamente en /usr/local/"
        # Configurar variables de entorno
        export PKG_CONFIG_PATH=/usr/local/lib/pkgconfig:$PKG_CONFIG_PATH
        export LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH
        return 0
    else
        log_message $RED "Error al instalar libcoap"
        return 1
    fi
}

# Verificar dependencias básicas
log_message $BLUE "Verificando dependencias básicas..."
failed_deps=0

if ! check_command cmake; then
    log_message $YELLOW "Instalar con: sudo apt-get install cmake"
    failed_deps=$((failed_deps + 1))
fi

if ! check_command make; then
    log_message $YELLOW "Instalar con: sudo apt-get install make"
    failed_deps=$((failed_deps + 1))
fi

if ! check_command pkg-config; then
    log_message $YELLOW "Instalar con: sudo apt-get install pkg-config"
    failed_deps=$((failed_deps + 1))
fi

if ! check_command gcc; then
    log_message $YELLOW "Instalar con: sudo apt-get install build-essential"
    failed_deps=$((failed_deps + 1))
fi

if [ $failed_deps -gt 0 ]; then
    log_message $RED "Se encontraron $failed_deps dependencias faltantes. Instálalas antes de continuar."
    exit 1
fi

log_message $GREEN "Dependencias básicas verificadas correctamente"

# Verificar librerías específicas
log_message $BLUE "Verificando librerías específicas..."

# 1. Verificar libcoap
log_message $BLUE "Comprobando libcoap..."
if ! check_library "libcoap" "libcoap-3" "install_libcoap"; then
    log_message $RED "No se pudo instalar libcoap. No se puede continuar con la compilación."
    exit 1
fi

# 2. Verificar cJSON
log_message $BLUE "Comprobando cJSON..."
if ! check_library "cJSON" "libcjson" "sudo apt-get install -y libcjson-dev"; then
    log_message $RED "No se pudo instalar cJSON. No se puede continuar con la compilación."
    exit 1
fi

# 3. Verificar json-c (alternativa)
log_message $BLUE "Comprobando json-c..."
if ! check_library "json-c" "json-c" "sudo apt-get install -y libjson-c-dev"; then
    log_message $YELLOW "json-c no está disponible, pero cJSON ya está instalado."
fi

# 4. Verificar OpenSSL
log_message $BLUE "Comprobando OpenSSL..."
if ! check_library "OpenSSL" "openssl" "sudo apt-get install -y libssl-dev"; then
    log_message $RED "No se pudo instalar OpenSSL. No se puede continuar con la compilación."
    exit 1
fi

# 5. Verificar SQLite3 (necesario para servidor central)
log_message $BLUE "Comprobando SQLite3..."
if ! check_library "SQLite3" "sqlite3" "sudo apt-get install -y libsqlite3-dev"; then
    log_message $RED "No se pudo instalar SQLite3. Es necesario para el servidor central."
    exit 1
fi

# 6. Verificar libcurl (necesario para comunicaciones HTTP)
log_message $BLUE "Comprobando libcurl..."
if ! check_library "libcurl" "libcurl" "sudo apt-get install -y libcurl4-openssl-dev"; then
    log_message $RED "No se pudo instalar libcurl. Es necesario para comunicaciones HTTP."
    exit 1
fi

# Exportar variables de entorno para libcoap
export PKG_CONFIG_PATH=/usr/local/lib/pkgconfig:$PKG_CONFIG_PATH
export LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH

log_message $GREEN "Todas las librerías verificadas correctamente"

# 2. Crear y navegar al directorio build
log_message $BLUE "Navegando al directorio $BUILD_DIR..."
if [ ! -d "$BUILD_DIR" ]; then
    mkdir -p "$BUILD_DIR"
fi
cd "$BUILD_DIR" || { log_message $RED "Error: No se pudo cambiar al directorio $BUILD_DIR."; exit 1; }

# 3. Limpiar el directorio build
log_message $BLUE "Limpiando el directorio $BUILD_DIR..."
rm -rf "$BUILD_DIR"/*
if [ $? -eq 0 ]; then
    log_message $GREEN "Directorio build limpio."
else
    log_message $RED "Error al limpiar el directorio build."
    exit 1
fi

# 4. Ejecutar cmake
log_message $BLUE "Ejecutando cmake..."
cmake -S .. -B . -DCMAKE_BUILD_TYPE=Release
if [ $? -eq 0 ]; then
    log_message $GREEN "cmake ejecutado correctamente."
else
    log_message $RED "Error al ejecutar cmake."
    exit 1
fi

# 5. Ejecutar make para el servidor central
log_message $BLUE "Ejecutando make servidor_central..."
make servidor_central
if [ $? -eq 0 ]; then
    log_message $GREEN "make ejecutado correctamente."
else
    log_message $RED "Error al ejecutar make."
    exit 1
fi

# 6. Verificar existencia del ejecutable servidor_central
SERVIDOR_CENTRAL_EXEC=$(find "$BUILD_DIR" -type f -name servidor_central -executable | head -n 1)
if [ -n "$SERVIDOR_CENTRAL_EXEC" ]; then
    log_message $GREEN "Ejecutable servidor_central encontrado en $SERVIDOR_CENTRAL_EXEC."
else
    log_message $RED "Error: No se encontró el ejecutable servidor_central en $BUILD_DIR."
    exit 1
fi

# 7. Mostrar información de compilación (sin ejecutar)
log_message $GREEN "=== COMPILACIÓN DEL SERVIDOR CENTRAL COMPLETADA EXITOSAMENTE ==="
log_message $BLUE "Ejecutable disponible en: $SERVIDOR_CENTRAL_EXEC"
log_message $BLUE "Para ejecutar manualmente: $SERVIDOR_CENTRAL_EXEC"
log_message $BLUE "Para desplegar en Kubernetes: ./deploy.sh"
log_message $YELLOW "IMPORTANTE: Antes de usar el servidor central, asegúrate de:"
log_message $YELLOW "  1. Desplegar con ./deploy.sh"
log_message $GREEN "El servidor central está listo para despliegue." 