#!/bin/bash

# Colores para la salida
GREEN='\033[0;32m'
RED='\033[0;31m'
NC='\033[0m' # Sin color

# Directorios base
BASE_DIR="/mnt/d/AATFG/API/Codigo/api_gateway"
BUILD_DIR="$BASE_DIR/build"

# Función para verificar comandos
check_command() {
  if ! command -v $1 &> /dev/null; then
    echo -e "${RED}Error: $1 no está instalado. Por favor, instálalo.${NC}"
    exit 1
  fi
}

# Función para instalar libcoap
install_libcoap() {
  echo "Instalando dependencias necesarias para libcoap..."
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
    && sudo rm -rf /var/lib/apt/lists/*

  echo "Clonando y compilando libcoap..."
  git clone --depth=1 https://github.com/obgm/libcoap.git /tmp/libcoap && \
    cd /tmp/libcoap && \
    ./autogen.sh && \
    ./configure --prefix=/usr/local --enable-dtls --with-openssl --disable-doxygen --disable-manpages && \
    make -j$(nproc) && \
    sudo make install && \
    sudo ldconfig && \
    rm -rf /tmp/libcoap
  if [ $? -eq 0 ]; then
    echo -e "${GREEN}libcoap instalado correctamente en /usr/local/.${NC}"
  else
    echo -e "${RED}Error al instalar libcoap.${NC}"
    exit 1
  fi
}

# Verificar dependencias básicas
echo "Verificando dependencias..."
check_command cmake
check_command make
check_command pkg-config

# 1. Comprobar si libcoap está instalada
echo "Comprobando libcoap..."
if pkg-config --modversion libcoap-3 &> /dev/null; then
  echo -e "${GREEN}libcoap ya está instalada (versión: $(pkg-config --modversion libcoap-3)).${NC}"
else
  echo -e "${RED}libcoap no está instalada.${NC}"
  echo -n "¿Deseas instalar libcoap desde el código fuente? (s/n): "
  read answer
  if [ "$answer" = "s" ] || [ "$answer" = "S" ]; then
    install_libcoap
  else
    echo -e "${RED}No se instaló libcoap. No se puede continuar con la compilación.${NC}"
    exit 1
  fi
fi

# 2. Crear y navegar al directorio build
echo "Navegando al directorio $BUILD_DIR..."
if [ ! -d "$BUILD_DIR" ]; then
  mkdir -p "$BUILD_DIR"
fi
cd "$BUILD_DIR" || { echo -e "${RED}Error: No se pudo cambiar al directorio $BUILD_DIR.${NC}"; exit 1; }

# 3. Limpiar el directorio build
echo "Limpiando el directorio $BUILD_DIR..."
rm -rf "$BUILD_DIR"/*
if [ $? -eq 0 ]; then
  echo -e "${GREEN}Directorio build limpio.${NC}"
else
  echo -e "${RED}Error al limpiar el directorio build.${NC}"
  exit 1
fi

# 4. Ejecutar cmake
echo "Ejecutando cmake..."
cmake -S .. -B .
if [ $? -eq 0 ]; then
  echo -e "${GREEN}cmake ejecutado correctamente.${NC}"
else
  echo -e "${RED}Error al ejecutar cmake.${NC}"
  exit 1
fi

# 5. Ejecutar make para el target api_gateway
echo "Ejecutando make api_gateway..."
make api_gateway
if [ $? -eq 0 ]; then
  echo -e "${GREEN}make ejecutado correctamente.${NC}"
else
  echo -e "${RED}Error al ejecutar make.${NC}"
  exit 1
fi

# 5.1. Copiar archivo de datos de simulación
echo "Copiando archivo de datos de simulación..."
if [ -f "$BASE_DIR/simulation_data.json" ]; then
  cp "$BASE_DIR/simulation_data.json" "$BUILD_DIR/simulation_data.json"
  if [ $? -eq 0 ]; then
    echo -e "${GREEN}Archivo simulation_data.json copiado al directorio build.${NC}"
  else
    echo -e "${RED}Advertencia: No se pudo copiar simulation_data.json.${NC}"
  fi
else
  echo -e "${RED}Advertencia: No se encontró simulation_data.json en $BASE_DIR.${NC}"
fi

# 6. Verificar existencia del ejecutable api_gateway
API_GATEWAY_EXEC=$(find "$BUILD_DIR" -type f -name api_gateway -executable | head -n 1)
if [ -n "$API_GATEWAY_EXEC" ]; then
  echo -e "${GREEN}Ejecutable api_gateway encontrado en $API_GATEWAY_EXEC.${NC}"
else
  echo -e "${RED}Error: No se encontró el ejecutable api_gateway en $BUILD_DIR.${NC}"
  exit 1
fi

# 7. Ejecutar api_gateway
echo "Ejecutando $API_GATEWAY_EXEC..."
"$API_GATEWAY_EXEC"