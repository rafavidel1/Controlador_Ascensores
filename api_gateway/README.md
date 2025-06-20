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

### 🔐 **Comunicación Segura**
- ✅ DTLS 1.2 con Pre-Shared Keys (PSK)
- ✅ Reutilización de sesiones para optimización
- ✅ Gestión automática de conexiones
- ✅ Manejo robusto de errores de red

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
│   ├── 🔐 dtls_common_config.h # Configuración DTLS-PSK
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
sudo apt-get install -y build-essential cmake pkg-config git
sudo apt-get install -y libcjson-dev libssl-dev

# Instalar libcoap desde fuente (recomendado)
git clone https://github.com/obgm/libcoap.git /tmp/libcoap
cd /tmp/libcoap
./autogen.sh
./configure --prefix=/usr/local --enable-dtls --with-openssl
make -j$(nproc) && sudo make install && sudo ldconfig

# Para generación de PDFs (opcional)
sudo apt-get install -y pandoc texlive-xetex
```

#### CentOS/RHEL
```bash
# Habilitar EPEL
sudo yum install -y epel-release

# Dependencias
sudo yum groupinstall -y "Development Tools"
sudo yum install -y cmake3 pkg-config openssl-devel

# Compilar cJSON y libcoap desde fuente
# (seguir instrucciones similares a Ubuntu)
```

### ⚙️ Configuración

#### 1. Configuración de Red

Editar `include/api_gateway/coap_config.h`:

```c
// Configuración del servidor central
#define CENTRAL_SERVER_IP "192.168.1.100"    // IP del servidor
#define CENTRAL_SERVER_PORT "5684"            // Puerto DTLS
#define GW_LISTEN_PORT "5683"                 // Puerto de escucha

// Recursos CoAP
#define CENTRAL_SERVER_FLOOR_CALL_PATH "/peticion_piso"
#define CENTRAL_SERVER_CABIN_REQUEST_PATH "/peticion_cabina"
```

#### 2. Configuración DTLS-PSK

Editar `include/api_gateway/dtls_common_config.h`:

```c
// Credenciales DTLS (deben coincidir con el servidor)
#define IDENTITY_TO_PRESENT_TO_SERVER "Gateway_Client_001"
#define KEY_FOR_SERVER "SecretGatewayServidorCentralKey"
```

#### 3. Variables de Entorno

```bash
# Configuración opcional via variables de entorno
export GATEWAY_LOG_LEVEL=DEBUG
export GATEWAY_SERVER_IP=192.168.1.100
export GATEWAY_SERVER_PORT=5684
export GATEWAY_BUILDING_ID=EDIFICIO_PROD
```

## 📖 Guía de Uso

### 🎮 Modo Básico

```bash
# Ejecución estándar
./api_gateway

# Con puerto personalizado
./api_gateway 6000

# Con configuración específica
./api_gateway --building-id "EDIFICIO_A" --elevators 4 --floors 15
```

### 🧪 Modo Simulación

```bash
# Simulación con edificio aleatorio
./api_gateway --demo-mode

# Simulación acelerada
./api_gateway --fast-simulation

# Solo testing de conexión
./api_gateway --test-connection
```

### 📊 Modo Debugging

```bash
# Logs detallados
./api_gateway --verbose --log-level debug

# Con salida a archivo
./api_gateway --log-file gateway.log

# Modo interactivo
./api_gateway --interactive
```

### 🔄 Opciones de Línea de Comandos

| Opción | Descripción | Ejemplo |
|--------|-------------|---------|
| `--help` | Muestra ayuda completa | `./api_gateway --help` |
| `--version` | Información de versión | `./api_gateway --version` |
| `--building-id <ID>` | ID del edificio | `--building-id "E001"` |
| `--elevators <N>` | Número de ascensores | `--elevators 6` |
| `--floors <N>` | Número de pisos | `--floors 20` |
| `--server-ip <IP>` | IP del servidor central | `--server-ip 10.0.0.1` |
| `--server-port <PORT>` | Puerto del servidor | `--server-port 5685` |
| `--verbose` | Salida detallada | `--verbose` |
| `--demo-mode` | Modo demostración | `--demo-mode` |
| `--test-connection` | Solo test de conexión | `--test-connection` |

## 🔧 API y Configuración

### 🌐 Endpoints CoAP

#### Servidor Central (Salientes)

| Endpoint | Método | Propósito | Payload |
|----------|--------|-----------|---------|
| `/peticion_piso` | POST | Llamadas de piso | JSON con estado de ascensores |
| `/peticion_cabina` | POST | Solicitudes de cabina | JSON con estado de ascensores |

#### Gateway (Entrantes - Deprecados)

| Endpoint | Método | Estado | Descripción |
|----------|--------|--------|-------------|
| `/llamada_piso_gw` | POST | ⚠️ Deprecado | Usar simulación CAN |
| `/solicitud_cabina_gw` | POST | ⚠️ Deprecado | Usar simulación CAN |

### 📋 Formato de Mensajes

#### Payload para Llamada de Piso

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

#### Payload para Solicitud de Cabina

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

#### Respuesta del Servidor Central

```json
{
  "tarea_id": "T_1234567890",
  "ascensor_asignado_id": "EDIFICIO_TESTA1",
  "piso_destino": 7,
  "prioridad": "NORMAL",
  "tiempo_estimado": 45
}
```

### 🔄 Protocolo CAN Simulado

#### Tipos de Frames

| ID CAN | Tipo | DLC | Datos | Descripción |
|--------|------|-----|-------|-------------|
| `0x100` | Floor Call | 2 | `[piso, dirección]` | Llamada de piso |
| `0x200` | Cabin Request | 2 | `[ascensor_idx, piso_destino]` | Solicitud de cabina |
| `0x300` | Arrival | 2 | `[ascensor_idx, piso_actual]` | Notificación de llegada |
| `0x101` | Floor Response | 1-8 | `[ascensor_idx, tarea_id...]` | Respuesta a llamada |
| `0x201` | Cabin Response | 1-8 | `[confirmación, tarea_id...]` | Respuesta a solicitud |
| `0xFE` | Error | 2 | `[can_id_original, error_code]` | Error del gateway |

#### Ejemplo de Intercambio CAN

```
Simulador -> Gateway: ID=0x100, DLC=2, Data=[03, 00]  // Llamada piso 3, dirección UP
Gateway -> Servidor: POST /peticion_piso (JSON con estado)
Servidor -> Gateway: 200 OK (JSON con asignación)
Gateway -> Simulador: ID=0x101, DLC=8, Data=[01, T_, 1_, 2_, 3_, 4_, 5_, 6_]  // Ascensor 1, tarea T_123456
```

## 🧪 Testing y Simulación

### 🎯 Sistema de Simulación

El API Gateway incluye un sistema de simulación completo con:

- **100 edificios únicos** con configuraciones variadas
- **10 peticiones por edificio** (mix de floor calls y cabin requests)
- **Selección aleatoria** de edificio por ejecución
- **5,504 líneas de datos JSON** para testing exhaustivo

#### Estructura de Datos de Simulación

```json
{
  "edificios": [
    {
      "id_edificio": "E001",
      "peticiones": [
        {
          "tipo": "llamada_piso",
          "piso_origen": 0,
          "direccion": "up"
        },
        {
          "tipo": "solicitud_cabina",
          "indice_ascensor": 0,
          "piso_destino": 5
        }
      ]
    }
  ]
}
```

### 🧪 Casos de Prueba

#### Test de Conexión DTLS

```bash
# Verificar conectividad con servidor central
./api_gateway --test-connection

# Salida esperada:
# [INFO-GW] Probando conexión DTLS con servidor central...
# [INFO-GW] ✓ Sesión DTLS establecida correctamente
# [INFO-GW] ✓ Handshake PSK completado
# [INFO-GW] ✓ Test de conectividad exitoso
```

#### Test de Simulación Básica

```bash
# Ejecutar simulación con logging detallado
./api_gateway --demo-mode --verbose

# Verificar logs generados
ls -la logs/$(date +%Y-%m-%d)/
cat logs/$(date +%Y-%m-%d)/ejecucion_*.md
```

#### Test de Carga

```bash
# Ejecutar múltiples instancias (requiere puertos diferentes)
for i in {7000..7010}; do
  ./api_gateway $i &
done

# Monitorear procesos
ps aux | grep api_gateway
```

### 📊 Métricas de Rendimiento

El sistema registra automáticamente:

- **Latencia de solicitudes CoAP** (tiempo de respuesta)
- **Throughput del sistema** (peticiones/segundo)
- **Tasa de éxito** (respuestas exitosas vs errores)
- **Utilización de ascensores** (tiempo ocupado vs libre)
- **Eficiencia de correlación** (trackers utilizados vs disponibles)

## 📊 Logging y Monitoreo

### 📝 Sistema de Logging

#### Estructura de Archivos

```
logs/
├── 2025-01-17/
│   ├── ejecucion_14-30-25-123.md    # Log de ejecución
│   ├── ejecucion_14-30-25-123.pdf   # Reporte PDF generado
│   └── ejecucion_15-45-10-456.md
└── 2025-01-18/
    └── ejecucion_09-15-30-789.md
```

#### Contenido de Logs

Cada archivo de log incluye:

1. **📋 Resumen Ejecutivo**
   - Información del sistema y configuración
   - Métricas de rendimiento
   - Estado de la ejecución

2. **📊 Registro de Eventos**
   - Eventos CAN enviados/recibidos
   - Solicitudes CoAP y respuestas
   - Asignaciones y completaciones de tareas
   - Movimientos de ascensores

3. **📈 Estadísticas Finales**
   - Duración total de ejecución
   - Número de peticiones procesadas
   - Tasa de éxito y errores
   - Throughput del sistema

#### Generación de Reportes PDF

```bash
# Convertir el log más reciente
./generate_pdf_report.sh --latest

# Convertir todos los logs
./generate_pdf_report.sh --all

# Convertir un archivo específico
./generate_pdf_report.sh logs/2025-01-17/ejecucion_14-30-25-123.md
```

### 🎨 Niveles de Logging

| Nivel | Color | Propósito | Ejemplo |
|-------|-------|-----------|---------|
| `INFO` | 🟢 Verde | Información general | Conexión establecida |
| `DEBUG` | 🔵 Azul | Depuración detallada | Token CoAP recibido |
| `WARN` | 🟡 Amarillo | Advertencias | Timeout de conexión |
| `ERROR` | 🔴 Rojo | Errores | Fallo de autenticación |
| `CRIT` | 🟣 Magenta | Errores críticos | Servidor no disponible |

### 📊 Monitoreo en Tiempo Real

```bash
# Seguir logs en tiempo real
tail -f logs/$(date +%Y-%m-%d)/ejecucion_*.md

# Filtrar solo errores
./api_gateway --verbose 2>&1 | grep -E "(ERROR|CRIT)"

# Métricas de red
./api_gateway --verbose 2>&1 | grep -E "(CoAP|DTLS)"
```

## 🔒 Seguridad

### 🔐 DTLS-PSK (Pre-Shared Key)

El sistema utiliza DTLS 1.2 con claves pre-compartidas para:

- **🔒 Autenticación mutua** entre gateway y servidor
- **🛡️ Cifrado end-to-end** de toda la comunicación
- **🔑 Gestión de claves** simplificada sin PKI
- **⚡ Rendimiento optimizado** con reutilización de sesiones

#### Configuración de Seguridad

```c
// Gateway presenta esta identidad al servidor
#define IDENTITY_TO_PRESENT_TO_SERVER "Gateway_Client_001"

// Clave compartida (debe coincidir en servidor y gateway)
#define KEY_FOR_SERVER "SecretGatewayServidorCentralKey"
```

### 🛡️ Mejores Prácticas de Seguridad

1. **🔄 Rotación de Claves**
   ```bash
   # Cambiar claves PSK periódicamente
   # Actualizar en gateway y servidor simultáneamente
   ```

2. **🚫 Validación de Entrada**
   ```c
   // Todos los payloads JSON son validados
   // Límites estrictos en tamaños de mensaje
   // Sanitización de datos de entrada
   ```

3. **📊 Auditoría**
   ```bash
   # Todos los eventos de seguridad se registran
   grep -E "(DTLS|PSK|AUTH)" logs/*/ejecucion_*.md
   ```

4. **🔒 Aislamiento de Red**
   ```bash
   # Ejecutar en red aislada o VPN
   # Firewall con puertos específicos (5683, 5684)
   ```

### 🚨 Indicadores de Seguridad

| Evento | Nivel | Acción Recomendada |
|--------|-------|-------------------|
| Fallo de autenticación DTLS | 🔴 CRIT | Verificar claves PSK |
| Timeout de handshake | 🟡 WARN | Verificar conectividad |
| Sesión DTLS cerrada inesperadamente | 🟡 WARN | Revisar logs del servidor |
| Múltiples fallos de conexión | 🔴 ERROR | Posible ataque, revisar firewall |

## 🐛 Solución de Problemas

### ❓ Problemas Comunes

#### 1. Error de Compilación: libcoap no encontrada

```bash
# Síntoma
CMake Error: Could not find libcoap-3-openssl

# Solución
sudo apt-get install pkg-config
# O compilar libcoap desde fuente (ver sección de instalación)
```

#### 2. Error DTLS: Handshake Failed

```bash
# Síntoma
[ERROR-GW] DTLS handshake failed with server

# Verificaciones
1. Comprobar que el servidor central está ejecutándose
2. Verificar que las claves PSK coinciden
3. Comprobar conectividad de red (ping, telnet)
4. Revisar configuración de firewall
```

#### 3. No se generan logs

```bash
# Síntoma
Directorio logs/ vacío después de ejecución

# Solución
1. Verificar permisos de escritura: chmod 755 .
2. Crear directorio manualmente: mkdir -p logs
3. Verificar espacio en disco: df -h
```

#### 4. Simulación no ejecuta peticiones

```bash
# Síntoma
[SIM_ASCENSOR] No se pudieron cargar datos de simulación

# Solución
1. Verificar que simulation_data.json existe
2. Comprobar formato JSON: jq . simulation_data.json
3. Verificar permisos de lectura: chmod 644 simulation_data.json
```

### 🔍 Debugging Avanzado

#### Habilitar Debugging de libcoap

```bash
export COAP_LOG_LEVEL=7  # Máximo nivel de debug
./api_gateway --verbose
```

#### Análisis de Red con Wireshark

```bash
# Capturar tráfico CoAP/DTLS
sudo tcpdump -i any -w gateway_traffic.pcap port 5683 or port 5684

# Analizar con wireshark
wireshark gateway_traffic.pcap
```

#### Debugging de Memoria

```bash
# Ejecutar con valgrind
valgrind --leak-check=full --show-leak-kinds=all ./api_gateway

# Ejecutar con AddressSanitizer
gcc -fsanitize=address -g -o api_gateway_debug src/*.c
./api_gateway_debug
```

#### Logs de Sistema

```bash
# Revisar logs del sistema
sudo journalctl -u api-gateway --since "1 hour ago"

# Logs de red
sudo netstat -tlnp | grep 5683
sudo ss -tlnp | grep 5684
```

### 📞 Obtener Ayuda

| Problema | Recurso | Comando |
|----------|---------|---------|
| Uso general | Ayuda integrada | `./api_gateway --help` |
| Bugs | Issues en GitHub | Crear issue con logs |
| Configuración | Documentación | Ver sección de configuración |
| Rendimiento | Profiling | `perf record ./api_gateway` |

## 🤝 Contribución

### 🛠️ Configuración de Desarrollo

```bash
# 1. Fork del repositorio
git clone https://github.com/tu-usuario/api-gateway.git
cd api-gateway

# 2. Crear rama de desarrollo
git checkout -b feature/nueva-funcionalidad

# 3. Configurar hooks de pre-commit
cp scripts/pre-commit .git/hooks/
chmod +x .git/hooks/pre-commit
```

### 📝 Estándares de Código

#### Estilo de Código C

```c
// ✅ Correcto: Documentación Doxygen
/**
 * @brief Procesa un frame CAN entrante
 * @param frame Puntero al frame CAN a procesar
 * @param ctx Contexto CoAP para envío de solicitudes
 * @return true si se procesó exitosamente, false en caso de error
 */
bool process_can_frame(const can_frame_t *frame, coap_context_t *ctx);

// ✅ Correcto: Nombres descriptivos
static void handle_floor_call_response(const coap_pdu_t *response);

// ❌ Incorrecto: Nombres genéricos
static void handle_resp(const coap_pdu_t *r);
```

#### Mensajes de Commit

```bash
# ✅ Formato correcto
feat(can_bridge): añadir soporte para frames 0x400
fix(dtls): corregir leak de memoria en sesiones
docs(readme): actualizar instrucciones de instalación

# ❌ Formato incorrecto
Fixed bug
Update code
```

### 🧪 Testing

```bash
# Ejecutar tests unitarios
make test

# Ejecutar tests de integración
./scripts/integration_tests.sh

# Verificar cobertura de código
gcov src/*.c
lcov --capture --directory . --output-file coverage.info
```

### 📋 Checklist de Pull Request

- [ ] ✅ Código sigue estándares de estilo
- [ ] 📝 Documentación actualizada
- [ ] 🧪 Tests añadidos/actualizados
- [ ] 🔄 CI/CD pasa sin errores
- [ ] 📊 Cobertura de código mantenida
- [ ] 🔒 Revisión de seguridad completada

## 📄 Licencia

```
MIT License

Copyright (c) 2025 Sistema de Control de Ascensores

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
```

---

<div align="center">

**🏢 API Gateway CoAP v2.0**

*Sistema de Control de Ascensores - Trabajo de Fin de Grado*

[![GitHub](https://img.shields.io/badge/GitHub-Repository-blue.svg)](https://github.com/user/repo)
[![Documentation](https://img.shields.io/badge/Docs-Complete-brightgreen.svg)](docs/)
[![Support](https://img.shields.io/badge/Support-Available-orange.svg)](mailto:support@example.com)

*Desarrollado con ❤️ para la gestión inteligente de ascensores*

</div> 