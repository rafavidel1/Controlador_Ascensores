FROM ubuntu:22.04

# Instalar dependencias necesarias
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

# Ejecutar el servidor
CMD ["./build/servidor_central"]