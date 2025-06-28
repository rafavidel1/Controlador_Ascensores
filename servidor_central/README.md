# 🏢 Servidor Central - Sistema de Gestión de Ascensores

[![Build Status](https://img.shields.io/badge/build-passing-brightgreen.svg)](https://github.com/user/repo)
[![Tests](https://img.shields.io/badge/tests-11%2F11%20passing-brightgreen.svg)](./tests/)
[![Coverage](https://img.shields.io/badge/coverage-100%25-brightgreen.svg)](./tests/)
[![Language](https://img.shields.io/badge/language-C-blue.svg)](https://en.wikipedia.org/wiki/C_(programming_language))
[![Protocol](https://img.shields.io/badge/protocol-CoAP%2FDTLS--PSK-orange.svg)](https://tools.ietf.org/html/rfc7252)
[![License](https://img.shields.io/badge/license-MIT-green.svg)](LICENSE)

> **Servidor central de alta disponibilidad para la gestión inteligente de sistemas de ascensores multi-edificio con comunicación segura CoAP/DTLS-PSK**

## 📋 Tabla de Contenidos

- [🎯 Descripción General](#-descripción-general)
- [✨ Características Principales](#-características-principales)
- [🏗️ Arquitectura del Sistema](#️-arquitectura-del-sistema)
- [🚀 Inicio Rápido](#-inicio-rápido)
- [📦 Instalación](#-instalación)
- [⚙️ Configuración](#️-configuración)
- [🔌 API Endpoints](#-api-endpoints)
- [💾 Base de Datos](#-base-de-datos)
- [🧪 Testing](#-testing)
- [🔐 Seguridad](#-seguridad)
- [📊 Monitorización](#-monitorización)
- [🐛 Solución de Problemas](#-solución-de-problemas)
- [📚 Documentación](#-documentación)
- [🤝 Contribución](#-contribución)

## 🎯 Descripción General

El **Servidor Central** es el núcleo del sistema de gestión de ascensores, diseñado para manejar múltiples edificios con alta disponibilidad y rendimiento. Implementa algoritmos inteligentes de asignación de ascensores y proporciona una API CoAP segura con autenticación DTLS-PSK.

### 🎯 Casos de Uso

- **Edificios Comerciales**: Gestión de ascensores en centros comerciales y oficinas
- **Complejos Residenciales**: Coordinación de ascensores en edificios residenciales
- **Hospitales**: Gestión crítica de ascensores con prioridades especiales
- **Hoteles**: Optimización del servicio de ascensores para huéspedes

## ✨ Características Principales

### 🏢 **Gestión Multi-Edificio**
- Soporte para múltiples edificios con configuraciones independientes
- Escalabilidad horizontal para cientos de ascensores
- Aislamiento de lógica por edificio

### 🧠 **Algoritmos Inteligentes**
- Asignación óptima basada en distancia y carga
- Balanceo de carga automático
- Minimización de tiempo de espera

### 💾 **Persistencia Robusta**
- Base de datos SQLite integrada
- Estado en tiempo real sincronizado
- Sistema de logs completo

### 🔐 **Seguridad Avanzada**
- Cifrado DTLS-PSK de extremo a extremo
- Autenticación por Pre-Shared Keys
- Verificación de integridad de mensajes

### 📊 **Monitorización**
- Métricas en tiempo real
- Sistema de logs estructurado
- Herramientas de debugging integradas

## 🏗️ Arquitectura del Sistema

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                           SERVIDOR CENTRAL                                 │
├─────────────────────────────────────────────────────────────────────────────┤
│  ┌─────────────────┐    ┌─────────────────┐    ┌─────────────────────────┐  │
│  │  CoAP Handlers  │    │  JSON Processor │    │   Assignment Engine    │  │
│  │                 │◄──►│                 │◄──►│                         │  │
│  │ • /peticion_piso│    │ • Validation    │    │ • Optimal Algorithm     │  │
│  │ • /peticion_cab │    │ • Parsing       │    │ • Distance Calculation  │  │
│  │ • DTLS-PSK      │    │ • Generation    │    │ • Load Balancing        │  │
│  └─────────────────┘    └─────────────────┘    └─────────────────────────┘  │
│                                   │                                         │
│                                   ▼                                         │
│  ┌─────────────────────────────────────────────────────────────────────┐    │
│  │                    Database Manager (SQLite)                       │    │
│  │ • edificios • ascensores_estado • configuraciones • logs           │    │
│  └─────────────────────────────────────────────────────────────────────┘    │
└─────────────────────────────────────────────────────────────────────────────┘
                                   ▲
                                   │ CoAP/DTLS-PSK
                                   │
┌─────────────────────────────────────────────────────────────────────────────┐
│                         API GATEWAYS                                       │
│              Gateway A          Gateway B          Gateway C               │
│            (Edificio 1)       (Edificio 2)       (Edificio 3)             │
└─────────────────────────────────────────────────────────────────────────────┘
```

### 🔄 Flujo de Procesamiento

1. **Recepción**: API Gateway envía solicitud CoAP/DTLS
2. **Validación**: Verificación de autenticación y formato JSON
3. **Procesamiento**: Ejecución de algoritmos de asignación
4. **Persistencia**: Actualización de estado en base de datos
5. **Respuesta**: Generación de respuesta estructurada

## 🚀 Inicio Rápido

### Prerrequisitos

```bash
# Ubuntu/Debian
sudo apt-get update
sudo apt-get install -y build-essential cmake pkg-config
sudo apt-get install -y libcjson-dev libsqlite3-dev libssl-dev
```

### Compilación Express

```bash
# Clonar repositorio
git clone https://github.com/usuario/sistema-ascensores.git
cd sistema-ascensores

# Compilar libcoap (requerido)
cd Librerias/libcoap
./autogen.sh && ./configure --enable-dtls --with-openssl
make -j$(nproc) && sudo make install && sudo ldconfig

# Compilar servidor central
cd ../../
mkdir build && cd build
cmake -DBUILD_SERVIDOR_CENTRAL=ON -DCMAKE_BUILD_TYPE=Release ..
make -j$(nproc)
```

### Ejecución

```bash
# Ejecutar servidor
./servidor_central/servidor_central

# Verificar funcionamiento
curl -X POST http://localhost:5684/.well-known/core
```

## 📦 Instalación

### 🔧 Dependencias del Sistema

<details>
<summary><strong>Ubuntu/Debian</strong></summary>

```bash
# Paquetes base
sudo apt-get update
sudo apt-get install -y \
    build-essential \
    cmake \
    pkg-config \
    git \
    autotools-dev \
    automake \
    libtool

# Dependencias de desarrollo
sudo apt-get install -y \
    libcjson-dev \
    libsqlite3-dev \
    libssl-dev \
    libcoap-2-dev
```
</details>

<details>
<summary><strong>CentOS/RHEL</strong></summary>

```bash
# Paquetes base
sudo yum groupinstall -y "Development Tools"
sudo yum install -y cmake pkg-config git

# Dependencias específicas
sudo yum install -y \
    cjson-devel \
    sqlite-devel \
    openssl-devel
```
</details>

### 📚 Compilación de libcoap

```bash
cd Librerias/libcoap

# Configuración automática
./autogen.sh

# Configuración con DTLS
./configure \
    --prefix=/usr/local \
    --enable-dtls \
    --with-openssl \
    --enable-shared \
    --enable-static

# Compilación e instalación
make -j$(nproc)
sudo make install
sudo ldconfig

# Verificación
pkg-config --modversion libcoap-3-openssl
```

### 🏗️ Compilación del Proyecto

```bash
# Crear directorio de compilación
mkdir -p build
cd build

# Configuración CMake
cmake \
    -DBUILD_SERVIDOR_CENTRAL=ON \
    -DCMAKE_BUILD_TYPE=Release \
    -DENABLE_TESTING=ON \
    -DENABLE_COVERAGE=OFF \
    ..

# Compilación
make -j$(nproc)

# Verificación de binarios
ls -la servidor_central/servidor_central
```

## ⚙️ Configuración

### 🌐 Configuración de Red

```c
// servidor_central/include/servidor_central/config.h
#define SERVER_IP "127.0.0.1"          // IP del servidor
#define SERVER_PORT 5684                // Puerto CoAP estándar
#define MAX_CONCURRENT_SESSIONS 50      // Sesiones simultáneas
#define COAP_MAX_PDU_SIZE 1024         // Tamaño máximo de PDU
```

### 🔐 Configuración DTLS-PSK

```c
// servidor_central/include/servidor_central/dtls_config.h
#define DTLS_PSK_HINT "central_server"         // Hint del servidor
#define DTLS_PSK_KEY "secreto_compartido_2024" // Clave compartida
#define DTLS_PSK_KEY_LEN 22                    // Longitud de clave
#define DTLS_TIMEOUT_MS 5000                   // Timeout DTLS
```

### 💾 Configuración de Base de Datos

```c
// servidor_central/include/servidor_central/database_config.h
#define DATABASE_FILE "elevators.db"    // Archivo de base de datos
#define DB_TIMEOUT_MS 3000             // Timeout de operaciones
#define MAX_DB_CONNECTIONS 10          // Conexiones máximas
#define BACKUP_INTERVAL_HOURS 24       // Intervalo de backup
```

### 🎛️ Opciones de Línea de Comandos

```bash
# Mostrar ayuda
./servidor_central --help

# Configuración personalizada
./servidor_central \
    --ip "0.0.0.0" \
    --port 5685 \
    --database "custom.db" \
    --max-sessions 100

# Modo verbose para debugging
./servidor_central --verbose

# Modo de prueba
./servidor_central --test-mode
```

## 🔌 Endpoints

### 📍 Endpoint: `POST /peticion_piso`

**Descripción**: Procesa solicitudes de llamada desde piso

**Payload de Entrada**:
```json
{
  "id_edificio": "EDIFICIO_TEST",
  "piso_origen_llamada": 3,
  "direccion_llamada": "SUBIENDO",
  "elevadores_estado": [
    {
      "id_ascensor": "EDIFICIO_TESTA1",
      "piso_actual": 0,
      "estado_puerta": "CERRADA",
      "disponible": true,
      "tarea_actual_id": null,
      "destino_actual": null
    }
  ]
}
```

**Respuesta Exitosa** (`2.01 Created`):
```json
{
  "mensaje": "Solicitud de piso procesada exitosamente",
  "ascensor_asignado_id": "EDIFICIO_TESTA1",
  "tarea_id": "T_1749908537046",
  "piso_destino": 3,
  "tiempo_estimado": 30
}
```

### 🚪 Endpoint: `POST /peticion_cabina`

**Descripción**: Procesa solicitudes desde interior de cabina

**Payload de Entrada**:
```json
{
  "id_edificio": "EDIFICIO_TEST",
  "solicitando_ascensor_id": "EDIFICIO_TESTA1",
  "piso_destino_solicitud": 7,
  "elevadores_estado": [
    {
      "id_ascensor": "EDIFICIO_TESTA1",
      "piso_actual": 3,
      "estado_puerta": "CERRADA",
      "disponible": false,
      "tarea_actual_id": "T_1234567890",
      "destino_actual": 7
    }
  ]
}
```

**Respuesta Exitosa** (`2.01 Created`):
```json
{
  "mensaje": "Solicitud de cabina procesada exitosamente",
  "tarea_id": "T_1749908537047",
  "confirmacion": "Destino registrado para ascensor EDIFICIO_TESTA1"
}
```

### 📊 Códigos de Respuesta

| Código | Estado | Descripción |
|--------|---------|-------------|
| `2.01` | Created | Solicitud procesada exitosamente |
| `4.00` | Bad Request | JSON malformado o campos faltantes |
| `4.04` | Not Found | Edificio no encontrado en base de datos |
| `5.00` | Internal Server Error | Error interno del servidor |
| `5.03` | Service Unavailable | No hay ascensores disponibles |

### 🧪 Testing de Endpoints

```bash
# Probar con coap-client
coap-client -m post \
    -T "application/json" \
    -e '{"id_edificio":"TEST","piso_origen_llamada":3,"direccion_llamada":"SUBIENDO","elevadores_estado":[]}' \
    coap://127.0.0.1:5684/peticion_piso

# Con DTLS-PSK
coap-client -k "secreto_compartido_2024" \
    -u "gateway_client" \
    -m post \
    -T "application/json" \
    -e '{"test":"payload"}' \
    coaps://127.0.0.1:5684/peticion_piso
```

## 💾 Base de Datos

### 📊 Esquema de Base de Datos

#### Tabla `edificios`
```sql
CREATE TABLE edificios (
    id_edificio TEXT PRIMARY KEY,
    num_plantas INTEGER NOT NULL,
    num_ascensores_total INTEGER NOT NULL,
    fecha_creacion DATETIME DEFAULT CURRENT_TIMESTAMP,
    activo INTEGER DEFAULT 1
);
```

#### Tabla `ascensores_estado`
```sql
CREATE TABLE ascensores_estado (
    ascensor_id TEXT PRIMARY KEY,
    edificio_id TEXT NOT NULL,
    piso_actual INTEGER NOT NULL DEFAULT 0,
    estado_puerta TEXT NOT NULL DEFAULT 'CERRADA',
    disponible INTEGER NOT NULL DEFAULT 1,
    tarea_actual_id TEXT,
    destino_actual INTEGER,
    ultima_actualizacion DATETIME DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (edificio_id) REFERENCES edificios(id_edificio)
);
```

#### Tabla `logs_operaciones`
```sql
CREATE TABLE logs_operaciones (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    timestamp DATETIME DEFAULT CURRENT_TIMESTAMP,
    edificio_id TEXT,
    ascensor_id TEXT,
    operacion TEXT,
    detalles TEXT,
    resultado TEXT
);
```
### 📊 Cobertura de Código

Las pruebas cubren:
- ✅ **Generación de IDs**: Unicidad y formato correcto
- ✅ **Validación JSON**: Payloads válidos e inválidos
- ✅ **Algoritmos de asignación**: Lógica básica y optimizada
- ✅ **Generación de respuestas**: Formato correcto
- ✅ **Flujo completo**: Integración end-to-end
- ✅ **Manejo de errores**: Casos de error y recuperación

## 🔐 Seguridad

### 🔑 DTLS-PSK Implementation

El servidor implementa **DTLS-PSK** (Datagram Transport Layer Security with Pre-Shared Keys) para asegurar todas las comunicaciones:

```c
/**
 * @brief Callback de validación PSK
 * Valida la identidad del cliente y proporciona la clave compartida
 */
static unsigned int psk_server_callback(
    coap_session_t *session,
    const uint8_t *hint, size_t hint_len,
    const uint8_t *identity, size_t identity_len,
    uint8_t *key, size_t max_key_len
) {
    // Validar identidad del cliente
    if (identity_len == strlen("gateway_client") &&
        memcmp(identity, "gateway_client", identity_len) == 0) {
        
        if (max_key_len >= psk_key_len) {
            memcpy(key, psk_key, psk_key_len);
            return psk_key_len;
        }
    }
    
    return 0; // Fallo de autenticación
}
```

### 🛡️ Características de Seguridad

- **Cifrado de extremo a extremo**: Todos los datos están cifrados
- **Autenticación mutua**: Cliente y servidor se autentican
- **Integridad de mensajes**: Verificación de integridad automática
- **Protección contra replay**: Prevención de ataques de repetición

### 🔧 Configuración de Seguridad

```bash
# Generar nueva clave PSK (recomendado para producción)
openssl rand -hex 16

# Configurar claves en ambos extremos
# servidor_central/include/servidor_central/dtls_config.h
# api_gateway/include/api_gateway/dtls_common_config.h
```

## 📊 Monitorización

### 📈 Métricas en Tiempo Real

```bash
# Estado del servidor
sqlite3 elevators.db "SELECT * FROM ascensores_estado;"

# Conexiones activas
ss -tulpn | grep 5684

# Uso de recursos
htop -p $(pgrep servidor_central)
```

### 📝 Sistema de Logs

```bash
# Logs del sistema
tail -f /var/log/servidor_central.log

# Logs de base de datos
sqlite3 elevators.db "SELECT * FROM logs_operaciones ORDER BY timestamp DESC LIMIT 10;"

# Habilitar logging detallado
export SQLITE_DEBUG=1
./servidor_central --verbose
```

### 🔍 Herramientas de Debugging

```bash
# Con GDB
gdb ./servidor_central
(gdb) set args --verbose
(gdb) run

# Con Valgrind (detección de memory leaks)
valgrind --leak-check=full ./servidor_central

# Análisis de rendimiento
perf record ./servidor_central
perf report
```

## 🐛 Solución de Problemas

### ❌ Errores Comunes

<details>
<summary><strong>Error: "Database connection failed"</strong></summary>

```bash
# Verificar permisos
ls -la elevators.db
chmod 664 elevators.db

# Verificar integridad
sqlite3 elevators.db "PRAGMA integrity_check;"

# Recrear si está corrupta
rm elevators.db && ./servidor_central
```
</details>

<details>
<summary><strong>Error: "DTLS handshake failed"</strong></summary>

```bash
# Verificar configuración PSK
grep -r "DTLS_PSK" include/servidor_central/

# Verificar sincronización de claves
diff servidor_central/include/servidor_central/dtls_config.h \
     api_gateway/include/api_gateway/dtls_common_config.h

# Probar sin DTLS (debugging)
./servidor_central --no-dtls
```
</details>

<details>
<summary><strong>Error: "Port already in use"</strong></summary>

```bash
# Identificar proceso
sudo netstat -tulpn | grep 5684
sudo lsof -i :5684

# Terminar proceso
sudo kill -9 <PID>

# Usar puerto alternativo
./servidor_central --port 5685
```
</details>

### 🔧 Comandos de Diagnóstico

```bash
# Verificar dependencias
pkg-config --exists libcoap-3-openssl && echo "✅ libcoap OK"
pkg-config --exists libcjson && echo "✅ cJSON OK"
sqlite3 --version && echo "✅ SQLite OK"

# Test de conectividad
coap-client -m get coap://127.0.0.1:5684/.well-known/core

# Verificar base de datos
sqlite3 elevators.db ".tables"
sqlite3 elevators.db "SELECT COUNT(*) FROM edificios;"
```

## 📚 Documentación

### 📖 Documentación del Código

Todo el código está documentado usando **Doxygen**:

```bash
# Generar documentación
doxygen Doxyfile

# Ver documentación
firefox docs/html/index.html
```

### 🏗️ Algoritmos de Asignación

<details>
<summary><strong>Algoritmo Básico por Proximidad</strong></summary>

```c
/**
 * @brief Algoritmo de asignación básico por proximidad
 * Selecciona el ascensor disponible más cercano al piso objetivo
 */
int basic_assignment_algorithm(elevator_info_t* elevators, int count, int target_floor) {
    int best_elevator = -1;
    int min_distance = INT_MAX;
    
    for (int i = 0; i < count; i++) {
        if (elevators[i].available) {
            int distance = abs(elevators[i].current_floor - target_floor);
            if (distance < min_distance) {
                min_distance = distance;
                best_elevator = i;
            }
        }
    }
    
    return best_elevator;
}
```
</details>

### 📋 Especificaciones Técnicas

- **Lenguaje**: C (C99)
- **Protocolo**: CoAP (RFC 7252)
- **Seguridad**: DTLS-PSK (RFC 4279)
- **Base de Datos**: SQLite 3.x
- **Formato de Datos**: JSON (RFC 7159)
- **Arquitectura**: Cliente-Servidor

---
