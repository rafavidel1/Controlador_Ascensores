# 🏢 API Gateway CoAP - Sistema de Control de Ascensores

[![Build Status](https://img.shields.io/badge/build-passing-brightgreen.svg)](https://github.com/user/repo)
[![Version](https://img.shields.io/badge/version-2.0-blue.svg)](https://github.com/user/repo/releases)
[![Language](https://img.shields.io/badge/language-C-blue.svg)](https://en.wikipedia.org/wiki/C_(programming_language))
[![Protocol](https://img.shields.io/badge/protocol-CoAP%2FDTLS--PSK-orange.svg)](https://tools.ietf.org/html/rfc7252)
[![License](https://img.shields.io/badge/license-MIT-green.svg)](LICENSE)
[![Documentation](https://img.shields.io/badge/docs-complete-brightgreen.svg)](docs/)

> **Gateway inteligente y seguro para la gestión distribuida de sistemas de ascensores mediante CoAP/DTLS-PSK**

## 📋 Tabla de Contenidos

- [🎯 Descripción General](#-descripción-general)
- [✨ Características Principales](#-características-principales)
- [🏗️ Arquitectura del Sistema](#️-arquitectura-del-sistema)
- [🚀 Inicio Rápido](#-inicio-rápido)
- [⚙️ Instalación y Configuración](#️-instalación-y-configuración)
- [📖 Guía de Uso](#-guía-de-uso)
- [🔧 API y Configuración](#-api-y-configuración)
- [🧪 Testing y Simulación](#-testing-y-simulación)
- [📊 Logging y Monitoreo](#-logging-y-monitoreo)
- [🔒 Seguridad](#-seguridad)
- [🐛 Solución de Problemas](#-solución-de-problemas)
- [🤝 Contribución](#-contribución)
- [📄 Licencia](#-licencia)

## 🎯 Descripción General

El **API Gateway CoAP** es un componente crítico del Sistema de Control de Ascensores que actúa como intermediario inteligente entre los controladores CAN de ascensores y el servidor central de asignación. Implementa un puente bidireccional CAN-CoAP con comunicación segura DTLS-PSK.

### 🎭 Casos de Uso

- **🏢 Gestión de Edificios**: Control centralizado de múltiples ascensores
- **🔄 Traducción de Protocolos**: Puente entre CAN y CoAP/DTLS
- **📊 Monitoreo en Tiempo Real**: Estado completo de ascensores
- **🧪 Testing y Simulación**: Entorno de pruebas integrado
- **📈 Análisis de Rendimiento**: Logging detallado y métricas

## ✨ Características Principales

### 🔄 **Puente CAN-CoAP Bidireccional**
- ✅ Procesamiento de frames CAN simulados (0x100, 0x200, 0x300)
- ✅ Conversión automática a solicitudes CoAP estructuradas
- ✅ Sistema de correlación con tokens únicos
- ✅ Buffer circular para gestión eficiente de trackers

### 🏢 **Gestión Avanzada de Estado**
- ✅ Estado completo de ascensores (posición, puertas, tareas, dirección)
- ✅ Asignación automática de tareas desde servidor central
- ✅ Simulación realista de movimiento
- ✅ Serialización JSON optimizada para servidor central

### 🔐 **Comunicación Segura DTLS-PSK**
- ✅ **DTLS 1.2 con Pre-Shared Keys (PSK)** y autenticación mutua
- ✅ **Sistema de claves determinístico** basado en identidad del cliente
- ✅ **Gestión de sesiones optimizada** con reconexión automática
- ✅ **Validación de claves** contra archivo de 15,000 claves pre-generadas
- ✅ **Timeouts configurados** para máxima estabilidad de conexión
- ✅ **Manejo robusto de errores** de red y reconexión automática

### 🧪 **Sistema de Simulación Integrado**
- ✅ 100 edificios con 10 peticiones cada uno (5,504 líneas de datos JSON)
- ✅ Selección aleatoria de edificios por ejecución
- ✅ Simulación de llamadas de piso y solicitudes de cabina
- ✅ Callback system para respuestas CAN

### 📊 **Logging y Reportes Profesionales**
- ✅ Archivos Markdown organizados por fecha/hora
- ✅ Generación automática de PDFs con pandoc
- ✅ Estadísticas de rendimiento y métricas
- ✅ Registro completo de eventos CoAP/CAN

## 🏗️ Arquitectura del Sistema

### 📐 Diagrama de Componentes

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                           API GATEWAY                                       │
├─────────────────────────────────────────────────────────────────────────────┤
│  ┌─────────────────┐    ┌─────────────────┐    ┌─────────────────────────┐  │
│  │   CAN Bridge    │    │  API Handlers   │    │  Elevator State Manager │  │
│  │  (can_bridge.c) │◄──►│(api_handlers.c) │◄──►│(elevator_state_manager.c)│  │
│  │                 │    │                 │    │                         │  │
│  │ • Procesa CAN   │    │ • Maneja CoAP   │    │ • Estado de ascensores  │  │
│  │ • Simula frames │    │ • Valida JSON   │    │ • Asignación de tareas  │  │
│  │ • Correlación   │    │ • DTLS-PSK      │    │ • Serialización JSON    │  │
│  └─────────────────┘    └─────────────────┘    └─────────────────────────┘  │
│                                   │                                         │
│                                   ▼                                         │
│  ┌─────────────────────────────────────────────────────────────────────┐    │
│  │                        Main Loop (main.c)                          │    │
│  │ • Simulación de movimiento • Gestión de eventos • Control DTLS     │    │
│  └─────────────────────────────────────────────────────────────────────┘    │
│                                   │                                         │
│  ┌─────────────────────────────────────────────────────────────────────┐    │
│  │                    Sistema de Logging (execution_logger.c)         │    │
│  │ • Logs Markdown • Reportes PDF • Estadísticas • Métricas           │    │
│  └─────────────────────────────────────────────────────────────────────┘    │
└─────────────────────────────────────────────────────────────────────────────┘
                                   │ CoAP/DTLS-PSK
                                   ▼
┌─────────────────────────────────────────────────────────────────────────────┐
│                         SERVIDOR CENTRAL                                   │
│                    • /peticion_piso • /peticion_cabina                     │
└─────────────────────────────────────────────────────────────────────────────┘
```

### 🔄 Flujo de Comunicación

```
Simulador -> Gateway: Frame CAN (0x100/0x200)
Gateway -> Estado: Consultar ascensores
Estado -> Gateway: Estado actual JSON
Gateway -> Servidor: POST /peticion_piso (DTLS-PSK)
Servidor -> Gateway: Respuesta con asignación
Gateway -> Estado: Actualizar ascensor
Gateway -> Simulador: Frame CAN respuesta
```

### 🗂️ Estructura de Directorios

```
api_gateway/
├── 📁 src/                     # Código fuente principal
│   ├── 🔧 main.c               # Punto de entrada y bucle principal
│   ├── 🌐 api_handlers.c       # Manejadores CoAP y DTLS
│   ├── 🏢 elevator_state_manager.c  # Gestión de estado
│   ├── 🔄 can_bridge.c         # Puente CAN-CoAP
│   ├── 🧪 mi_simulador_ascensor.c   # Simulador integrado
│   ├── 📊 execution_logger.c   # Sistema de logging
│   └── 📋 simulation_loader.c  # Carga de datos JSON
├── 📁 include/api_gateway/     # Headers y definiciones
│   ├── 🔧 api_handlers.h
│   ├── 🏢 elevator_state_manager.h
│   ├── 🔄 can_bridge.h
│   ├── ⚙️ coap_config.h        # Configuración CoAP
│   ├── 🔐 dtls_common_config.h # Configuración DTLS-PSK y PSK Manager
│   ├── 📊 execution_logger.h
│   ├── 🎨 logging_gw.h         # Macros de logging con colores
│   └── 📋 simulation_loader.h
├── 📁 logs/                    # Logs organizados por fecha
├── 🔨 CMakeLists.txt          # Configuración de compilación
├── 📜 build_api_gateway.sh    # Script de compilación
├── 📄 generate_pdf_report.sh  # Generación de reportes
├── 📊 simulation_data.json    # Datos de simulación (100 edificios)
└── 📖 README.md               # Este archivo
```

## 🚀 Inicio Rápido

### ⚡ Ejecución Rápida

```bash
# 1. Clonar el repositorio
git clone <repository-url>
cd api_gateway

# 2. Compilar y ejecutar automáticamente
./build_api_gateway.sh

# 3. Ver logs generados
ls -la logs/$(date +%Y-%m-%d)/

# 4. Generar reporte PDF del último log
./generate_pdf_report.sh --latest
```

### 🐳 Usando Docker (Recomendado)

```bash
# Construir imagen
docker build -t api-gateway .

# Ejecutar contenedor
docker run -it --name api-gateway-test api-gateway

# Ver logs
docker exec api-gateway-test ls /app/logs/
```

## ⚙️ Instalación y Configuración

### 📋 Requisitos del Sistema

| Componente | Versión Mínima | Propósito |
|------------|----------------|-----------|
| **GCC** | 7.0+ | Compilador C con soporte C99 |
| **CMake** | 3.10+ | Sistema de construcción |
| **libcoap** | 4.3.0+ | Biblioteca CoAP con DTLS |
| **cJSON** | 1.7.0+ | Manipulación de JSON |
| **OpenSSL** | 1.1.1+ | Soporte criptográfico para DTLS |
| **pandoc** | 2.0+ | Generación de PDFs (opcional) |

### 🔧 Instalación de Dependencias

#### Ubuntu/Debian
```bash
# Dependencias del sistema
sudo apt-get update
sudo apt-get install -y build-essential cmake pkg-config
sudo apt-get install -y libcjson-dev libssl-dev
sudo apt-get install -y git wget ca-certificates

# Instalar pandoc para reportes PDF
sudo apt-get install -y pandoc

# Compilar libcoap desde fuente
cd ../Librerias/libcoap
./autogen.sh
./configure --prefix=/usr/local --enable-dtls --with-openssl --disable-doxygen --disable-manpages
make -j$(nproc) && sudo make install && sudo ldconfig
```

#### CentOS/RHEL
```bash
# Dependencias del sistema
sudo yum groupinstall "Development Tools"
sudo yum install cmake pkg-config
sudo yum install libcjson-devel openssl-devel
sudo yum install git wget ca-certificates

# Instalar pandoc para reportes PDF
sudo yum install pandoc

# Compilar libcoap desde fuente
cd ../Librerias/libcoap
./autogen.sh
./configure --prefix=/usr/local --enable-dtls --with-openssl --disable-doxygen --disable-manpages
make -j$(nproc) && sudo make install && sudo ldconfig
```

### 🔨 Compilación

```bash
# Crear directorio de build
mkdir build && cd build

# Configurar CMake
cmake -DBUILD_API_GATEWAY=ON -DCMAKE_BUILD_TYPE=Release ..

# Compilar
make -j$(nproc)

# Instalar (opcional)
sudo make install
```

### ⚙️ Configuración

#### 🔐 Configuración DTLS-PSK

El gateway utiliza el mismo sistema de claves PSK que el servidor central:

```bash
# Archivo de claves PSK (compartido con servidor central)
psk_keys.txt

# Formato de las claves:
# client_id:psk_key
# Ejemplo:
# gateway_001:abc123def456
# gateway_002:xyz789uvw012
```

#### 🌍 Variables de Entorno

```bash
# Configuración del servidor central
export SERVIDOR_CENTRAL_HOST=192.168.49.2  # IP asignada por MetalLB
export SERVIDOR_CENTRAL_PUERTO=5684

# Configuración DTLS
export DTLS_PSK_FILE=psk_keys.txt
export DTLS_TIMEOUT=30
export DTLS_MTU=1280

# Configuración de logging
export LOG_LEVEL=INFO
export LOG_DIR=logs
export GENERATE_PDF_REPORTS=true
```

#### 📁 Estructura de Archivos

```
api_gateway/
├── 📁 src/                    # Código fuente
├── 📁 include/api_gateway/    # Headers
├── 📁 logs/                   # Logs organizados por fecha
├── 🔨 CMakeLists.txt          # Configuración de build
├── 📜 build_api_gateway.sh    # Script de compilación
├── 📄 generate_pdf_report.sh  # Generación de reportes
├── 📊 simulation_data.json    # Datos de simulación
└── 📖 README.md               # Este archivo
```

## 📖 Guía de Uso

### 🚀 Ejecución Básica

```bash
# Compilar y ejecutar
./build_api_gateway.sh

# Ver logs en tiempo real
tail -f logs/$(date +%Y-%m-%d)/api_gateway_$(date +%H-%M-%S).md
```

### 🧪 Simulación Completa

```bash
# Ejecutar simulación con 100 edificios
./api_gateway

# Ver estadísticas
cat logs/$(date +%Y-%m-%d)/estadisticas.txt

# Generar reporte PDF
./generate_pdf_report.sh --latest
```

### 📊 Monitoreo en Tiempo Real

```bash
# Ver logs del último ejecución
ls -la logs/$(date +%Y-%m-%d)/

# Ver estadísticas de rendimiento
cat logs/$(date +%Y-%m-%d)/estadisticas.txt

# Ver conexiones DTLS activas
netstat -an | grep 5684
```

## 🔧 API y Configuración

### 📡 Endpoints CoAP

| Endpoint | Método | Descripción | Autenticación |
|----------|--------|-------------|---------------|
| `/peticion_piso` | POST | Solicitar asignación de ascensor | DTLS-PSK |
| `/peticion_cab` | POST | Solicitar ascensor específico | DTLS-PSK |

### 🔄 Frames CAN Procesados

| Frame ID | Tipo | Descripción | Payload |
|----------|------|-------------|---------|
| `0x100` | Llamada de piso | Solicitud de ascensor | `{piso_origen, piso_destino}` |
| `0x200` | Estado de cabina | Información de ascensor | `{ascensor_id, estado, piso_actual}` |
| `0x300` | Confirmación | Respuesta de asignación | `{tarea_id, ascensor_asignado}` |

### 📝 Formato de Peticiones JSON

```json
{
  "edificio_id": "edificio_001",
  "piso_origen": 5,
  "piso_destino": 10,
  "prioridad": "normal",
  "timestamp": 1640995200
}
```

### 📤 Formato de Respuestas JSON

```json
{
  "status": "success",
  "ascensor_asignado": "ascensor_003",
  "tiempo_estimado": 45,
  "tarea_id": "tarea_12345",
  "timestamp": 1640995200
}
```

## 🧪 Testing y Simulación

### 🧪 Tests Unitarios

```bash
# Ejecutar tests unitarios
cd tests/unit
make test

# Verificar cobertura
make coverage
```

### 🔗 Tests de Integración

```bash
# Ejecutar tests de integración
cd tests/integration
./run_integration_tests.sh
```

### 🧪 Simulación Automática

```bash
# Ejecutar simulación completa
./api_gateway

# Ver resultados
ls -la logs/$(date +%Y-%m-%d)/

# Generar reporte
./generate_pdf_report.sh --latest
```

## 📊 Logging y Monitoreo

### 📈 Sistema de Logs

```bash
# Estructura de logs
logs/
├── 2024-01-15/
│   ├── api_gateway_10-30-00.md
│   ├── estadisticas.txt
│   ├── metricas.json
│   └── reporte_10-30-00.pdf
└── 2024-01-16/
    └── ...
```

### 📊 Métricas en Tiempo Real

```bash
# Ver métricas de rendimiento
cat logs/$(date +%Y-%m-%d)/estadisticas.txt

# Ver conexiones DTLS
netstat -an | grep 5684

# Ver uso de memoria
ps aux | grep api_gateway
```

### 📄 Generación de Reportes

```bash
# Generar reporte PDF del último log
./generate_pdf_report.sh --latest

# Generar reporte de fecha específica
./generate_pdf_report.sh --date 2024-01-15

# Generar todos los reportes
./generate_pdf_report.sh --all
```

## 🔒 Seguridad

### 🔑 Sistema de Claves PSK

El gateway utiliza el mismo sistema de claves PSK que el servidor central:

```bash
# Archivo de claves PSK
psk_keys.txt

# Verificación de claves
./verify_psk_keys.sh
```

### 🔒 Configuración DTLS

```c
// Configuración DTLS-PSK
#define DTLS_PSK_FILE "psk_keys.txt"
#define DTLS_TIMEOUT 30
#define DTLS_MTU 1280
#define DTLS_RETRANSMIT_TIMEOUT 2
```

### 🛡️ Medidas de Seguridad

- ✅ **Cifrado de extremo a extremo** con DTLS 1.2
- ✅ **Autenticación mutua** mediante PSK
- ✅ **Validación de claves** contra archivo pre-generado
- ✅ **Timeouts optimizados** para prevenir ataques
- ✅ **Manejo robusto de errores** de red
- ✅ **Reconexión automática** en caso de fallos

## 🐛 Solución de Problemas

### 🔍 Problemas Comunes

<details>
<summary><strong>Error: Connection refused</strong></summary>

```bash
# Verificar que el servidor central esté ejecutándose
kubectl get pods -l app=servidor-central
kubectl get svc servidor-central-service

# Verificar conectividad
telnet 192.168.49.2 5684
```

</details>

<details>
<summary><strong>Error: DTLS handshake failed</strong></summary>

```bash
# Verificar archivo de claves PSK
ls -la psk_keys.txt

# Verificar configuración DTLS
cat include/api_gateway/dtls_common_config.h
```

</details>

<details>
<summary><strong>Error: JSON parsing failed</strong></summary>

```bash
# Verificar formato JSON
cat simulation_data.json | jq .

# Verificar codificación
file simulation_data.json
```

</details>

### 🛠️ Herramientas de Debugging

```bash
# Ver logs detallados
tail -f logs/$(date +%Y-%m-%d)/api_gateway_*.md

# Ver conexiones de red
netstat -an | grep 5684

# Ver uso de recursos
top -p $(pgrep api_gateway)

# Ver logs del sistema
journalctl -u api-gateway -f
```

## 🤝 Contribución

### 📝 Guías de Contribución

1. **Fork** el repositorio
2. **Crear** una rama para tu feature (`git checkout -b feature/nueva-funcionalidad`)
3. **Commit** tus cambios (`git commit -am 'Agregar nueva funcionalidad'`)
4. **Push** a la rama (`git push origin feature/nueva-funcionalidad`)
5. **Crear** un Pull Request

### 🧪 Testing

```bash
# Ejecutar todos los tests
./run_all_tests.sh

# Verificar cobertura
make coverage
```

### 📋 Checklist de Contribución

- [ ] Tests unitarios pasando
- [ ] Tests de integración pasando
- [ ] Documentación actualizada
- [ ] Código siguiendo estándares
- [ ] Configuración DTLS-PSK verificada
- [ ] Logs generados correctamente

---

## 📄 Licencia

Este proyecto está bajo la Licencia MIT. Ver el archivo [LICENSE](../LICENSE) para más detalles.

---

**🏢 API Gateway CoAP** - Gateway inteligente y seguro para la gestión distribuida de sistemas de ascensores mediante CoAP/DTLS-PSK. 