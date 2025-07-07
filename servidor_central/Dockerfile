# Multi-stage build para optimizar el tamaño de la imagen
FROM ubuntu:22.04 AS builder

# Instalar dependencias de compilación
RUN apt-get update && apt-get install -y \
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
    wget \
    ca-certificates \
    && apt-get clean && apt-get autoclean \
    && rm -rf /var/lib/apt/lists/*

# Instalar libcoap desde fuente
RUN git clone --depth=1 https://github.com/obgm/libcoap.git /tmp/libcoap && \
    cd /tmp/libcoap && \
    ./autogen.sh && \
    ./configure --prefix=/usr/local --enable-dtls --with-openssl --disable-doxygen --disable-manpages && \
    make -j$(nproc) && \
    make install && \
    ldconfig && \
    rm -rf /tmp/libcoap

# Exportar manualmente el path al archivo .pc de libcoap
ENV PKG_CONFIG_PATH=/usr/local/lib/pkgconfig
ENV LD_LIBRARY_PATH=/usr/local/lib

# Crear carpeta de trabajo
WORKDIR /app

# Copiar el código fuente del proyecto
COPY . .

# Crear carpeta de build y compilar
RUN cmake -B build -S . -DCMAKE_BUILD_TYPE=Release && \
    cmake --build build

# Segunda etapa: imagen de producción limpia
FROM ubuntu:22.04 AS production

# Instalar solo las dependencias de runtime necesarias
RUN apt-get update && apt-get install -y \
    libssl3 \
    libc6 \
    libcjson1 \
    ca-certificates \
    && apt-get clean && apt-get autoclean \
    && rm -rf /var/lib/apt/lists/*

# Copiar libcoap desde la etapa de builder
COPY --from=builder /usr/local/lib/libcoap* /usr/local/lib/
COPY --from=builder /usr/local/include/coap* /usr/local/include/
COPY --from=builder /usr/local/lib/pkgconfig/libcoap* /usr/local/lib/pkgconfig/

# Configurar el runtime environment
ENV LD_LIBRARY_PATH=/usr/local/lib

# Crear usuario no-root para seguridad
RUN groupadd -r appuser && useradd -r -g appuser appuser

# Crear directorio de trabajo
WORKDIR /app

# Copiar el binario compilado desde la etapa de builder
COPY --from=builder /app/build/servidor_central .

# Copiar archivos de configuración necesarios
COPY --from=builder /app/psk_keys.txt .

# Cambiar permisos y propietario
RUN chown appuser:appuser /app/servidor_central && \
    chown appuser:appuser /app/psk_keys.txt && \
    chmod +x /app/servidor_central

# Cambiar al usuario no-root
USER appuser

# Exponer puerto DTLS
EXPOSE 5684

# Ejecutar el servidor
CMD ["./servidor_central"]