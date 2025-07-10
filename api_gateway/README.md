# 🔄 API Gateway - Sistema de Control de Ascensores

[![Build Status](https://img.shields.io/badge/build-passing-brightgreen.svg)](https://github.com/user/repo)
[![Protocol](https://img.shields.io/badge/protocol-CoAP%2FDTLS--PSK-orange.svg)](https://tools.ietf.org/html/rfc7252)
[![CAN Protocol](https://img.shields.io/badge/protocol-CAN-blue.svg)](https://en.wikipedia.org/wiki/CAN_bus)

> **Puente CAN-CoAP inteligente con comunicación DTLS-PSK y simulación integrada de 100 edificios - 100% automatizado**

## 📋 Tabla de Contenidos

- [🎯 Descripción General](#-descripción-general)
- [🚀 Inicio Rápido - 100% Automatizado](#-inicio-rápido---100-automatizado)
- [🏗️ Arquitectura del Gateway](#️-arquitectura-del-gateway)
- [📊 Simulación Masiva](#-simulación-masiva)
- [📈 Monitoreo y Logging](#-monitoreo-y-logging)
- [🔒 Seguridad DTLS-PSK](#-seguridad-dtls-psk)
- [🐛 Solución de Problemas](#-solución-de-problemas)

## 🎯 Descripción General

El **API Gateway** es un puente inteligente que traduce automáticamente entre protocolos CAN y CoAP, proporcionando comunicación segura con el servidor central mediante DTLS-PSK. Incluye simulación integrada de 100 edificios con 1000 peticiones de prueba y reportes automáticos.

### 🌟 Características Clave

- **⚡ Compilación Automática**: `./build_api_gateway.sh` - Todo en un comando
- **🚀 Ejecución Zero-Config**: `./api_gateway` - Funciona inmediatamente
- **🔄 Puente CAN-CoAP**: Traducción bidireccional automática
- **📊 Estado en Tiempo Real**: Posición de ascensores actualizada automáticamente
- **🏢 Simulación Masiva**: 100 edificios, 1000 peticiones simultáneas
- **📈 Reportes Automáticos**: Logs PDF/Markdown con timestamping

## 🚀 Inicio Rápido - 100% Automatizado

### ⚡ Prerequisitos (Solo una vez)

```bash
# Ubuntu/Debian
sudo apt-get update
sudo apt-get install -y build-essential cmake pkg-config git
sudo apt-get install -y libcjson-dev libssl-dev
```

### 🎯 Compilación Automática (Un Solo Comando)

```bash
# Compilar TODO automáticamente (incluye libcoap)
./build_api_gateway.sh

# Salida esperada:
# ✅ Checking and building libcoap...
# ✅ Configuring CMake...
# ✅ Building API Gateway...
# ✅ Building dynamic port version...
# ✅ Build completed successfully!
```

### 🚀 Ejecución Automática

#### Opción 1: Ejecución Estándar (Recomendada)

```bash
# Ejecutar con configuración automática
./api_gateway

# Salida esperada:
# ✅ API Gateway: gateway.env cargado exitosamente
# ✅ API Gateway: Usando puerto por defecto 5683
# ✅ StateMgr: Inicializando 6 ascensores para edificio 'EDI1'
# ✅ API Gateway: Puente CAN inicializado
# ✅ API Gateway: Servidor CoAP iniciado en 0.0.0.0:5683
# ✅ API Gateway: Simulación de ascensores iniciada
```

#### Opción 2: Puerto Personalizado

```bash
# Ejecutar en puerto específico
./api_gateway 6000

# Salida esperada:
# ✅ API Gateway: Usando puerto personalizado 6000
# ✅ [resto igual]
```

#### Opción 3: Simulación Masiva

```bash
# Ejecutar 100 instancias simultáneas
./run_100_api_gateways.sh

# Salida esperada:
# ✅ Starting 100 API Gateway instances...
# ✅ Instance 1 started on port 6000
# ✅ Instance 2 started on port 6001
# ✅ [...]
# ✅ All 100 instances started successfully!
```

## 🏗️ Arquitectura del Gateway

### 🔄 **Flujo de Datos Automático**

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                                API GATEWAY                                  │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                             │
│  ┌─────────────────┐    JSON/HTTP     ┌─────────────────────────────┐      │
│  │                 │◄──────────────►│                             │      │
│  │   SIMULADOR     │                 │        PUENTE CAN           │      │
│  │   ASCENSORES    │                 │                             │      │
│  │                 │                 │ • Frames CAN → CoAP         │      │
│  │ • 100 Edificios │                 │ • Tokens únicos             │      │
│  │ • 1000 Peticiones│                │ • Correlación automática    │      │
│  │ • Estado RT     │                 │ • Timeouts inteligentes     │      │
│  └─────────────────┘                 └─────────────────────────────┘      │
│           │                                           │                     │
│           │ Estado Local                             │ CoAP/DTLS-PSK       │
│           ▼                                           ▼                     │
│  ┌─────────────────┐                     ┌─────────────────────────────┐   │
│  │  GESTOR ESTADO  │                     │    CLIENTE CoAP/DTLS        │   │
│  │   ASCENSORES    │                     │                             │   │
│  │                 │                     │ • Conexión DTLS-PSK         │   │
│  │ • piso_actual   │                     │ • 15,000 claves PSK         │   │
│  │ • destino_actual│                     │ • Timeouts configurables    │   │
│  │ • estado_puerta │                     │ • Reconexión automática     │   │
│  │ • disponible    │                     │ • Validación de sesión      │   │
│  └─────────────────┘                     └─────────────────────────────┘   │
│                                                       │                     │
│                                                       │ Puerto 5684         │
│                                                       ▼                     │
│                                           ┌─────────────────────────────┐   │
│                                           │      SERVIDOR CENTRAL       │   │
│                                           │                             │   │
│                                           │ • Algoritmo inteligente     │   │
│                                           │ • Persistencia SQLite       │   │
│                                           │ • Kubernetes Ready          │   │
│                                           └─────────────────────────────┘   │
│                                                                             │
└─────────────────────────────────────────────────────────────────────────────┘
```

### 📊 **Gestión de Estado en Tiempo Real**

El API Gateway mantiene el estado actualizado de todos los ascensores y lo reporta automáticamente al servidor central:

```json
{
  "id_edificio": "EDI1",
  "elevadores_estado": [{
    "id_ascensor": "EDI1A1",
    "piso_actual": 3,        // ✅ Actualizado en tiempo real
    "destino_actual": 7,     // ✅ Durante movimiento
    "estado_puerta": "CERRADA",
    "disponible": false,
    "tarea_actual_id": "T_1640995200123"
  }]
}
```

## 📊 Simulación Masiva

### 🏢 **Configuración Automática**

```bash
# Archivo: simulation_data.json (generado automáticamente)
{
  "edificios": [
    {
      "id": "EDI1",
      "num_ascensores": 6,
      "num_pisos": 10,
      "peticiones_simuladas": 1000
    },
    // ... 99 edificios más
  ]
}
```

### 🚀 **Ejecución Masiva**

```bash
# Ejecutar 100 gateways simultáneos
./run_100_api_gateways.sh

# Configuración automática:
# ✅ Puertos: 6000-6099 (automático)
# ✅ Edificios: EDI1-EDI100 (automático)
# ✅ Claves PSK: Gateway_Client_0001-Gateway_Client_0100 (automático)
# ✅ Logs: mass_execution_logs/*.log (automático)
```

### 📈 **Monitoreo en Tiempo Real**

```bash
# Ver logs de todas las instancias
tail -f mass_execution_logs/*.log

# Ver reporte final
cat mass_execution_logs/final_report.txt

# Salida esperada:
# ✅ 100 instancias ejecutándose
# ✅ 100,000 peticiones procesadas
# ✅ 0 errores de conexión DTLS
# ✅ Tiempo promedio de respuesta: 45ms
```

## 📈 Monitoreo y Logging

### 📊 **Reportes Automáticos**

```bash
# Ubicación automática de logs
ls -la logs/$(date +%Y-%m-%d)/

# Estructura automática:
# ✅ ejecucion_HH-MM-SS-mmm.md    # Reporte principal
# ✅ ejecucion_HH-MM-SS-mmm.pdf   # Reporte PDF (si disponible)
# ✅ network_debug_HH-MM-SS.log   # Debug de red
# ✅ dtls_handshake_HH-MM-SS.log  # Debug DTLS
```

### 🔍 **Contenido de Reportes**

```markdown
# Reporte de Ejecución API Gateway
**Timestamp**: 2024-01-15 20:56:36.697
**Edificio**: EDI1
**Ascensores**: 6
**Puerto**: 5683

## Estadísticas de Ejecución
- ✅ Conexiones DTLS: 1,000
- ✅ Peticiones procesadas: 10,000
- ✅ Tareas asignadas: 8,500
- ✅ Errores: 0

## Detalles de Ascensores
- EDI1A1: Piso 3 → 7 (En movimiento)
- EDI1A2: Piso 1 (Disponible)
- [...]
```

### 📊 **Métricas Automáticas**

- **Latencia promedio**: Calculada automáticamente
- **Throughput**: Peticiones por segundo
- **Disponibilidad**: Porcentaje de tiempo activo
- **Errores DTLS**: Conteo automático de fallos

## 🔒 Seguridad DTLS-PSK

### 🔐 **Configuración Automática**

```bash
# Archivo: gateway.env (configuración automática)
DTLS_ACK_TIMEOUT_SECONDS=10
DTLS_ACK_RANDOM_FACTOR=2
DTLS_MAX_RETRANSMIT=8
DTLS_MTU_SIZE=1280
ENABLE_NETWORK_DEBUG=1
LOG_DTLS_HANDSHAKE=1
```

### 🛡️ **Validación Automática**

```bash
# El build script valida automáticamente:
# ✅ Archivo psk_keys.txt existe
# ✅ Claves PSK válidas
# ✅ Identidades correctas
# ✅ Configuración DTLS
```

### 🔄 **Gestión de Sesiones**

- **Reconexión automática**: Si se pierde la conexión DTLS
- **Timeouts configurables**: Evita timeouts en redes lentas
- **Validación de estado**: Verifica sesión antes de cada petición
- **Logs de debug**: Información detallada de handshake DTLS

## 🐛 Solución de Problemas

### 🔍 **Problemas Comunes**

#### Error: "libcoap not found"
```bash
# Solución automática
./build_api_gateway.sh  # ✅ Instala librerías automáticamente

# Verificación manual (si necesario)
ls -la ../libcoap-install-linux/
```

#### Error: "DTLS handshake failed"
```bash
# Solución automática
export ENABLE_NETWORK_DEBUG=1
export LOG_DTLS_HANDSHAKE=1
./api_gateway

# Verificar configuración
cat gateway.env | grep DTLS
```

#### Error: "Port already in use"
```bash
# Solución automática
./api_gateway 6000  # ✅ Puerto personalizado

# O usar puerto dinámico
./api_gateway_dynamic_port  # ✅ Puerto automático
```

#### Error: "No se puede conectar al servidor central/DTLS failed"
```bash
# Verificar servidor central
ping 192.168.49.2
curl -k https://192.168.49.2:5684/status

# Verificar configuración
cat gateway.env | grep CENTRAL_SERVER
```
#### Verificar que el servidor está correctamente deplegado


## 💡 Uso Avanzado

### 🎯 **Variables de Entorno**

```bash
# Configuración personalizada
export GW_LISTEN_PORT=6000
export GW_LISTEN_IP=192.168.1.100
export CENTRAL_SERVER_IP=192.168.1.200
export CENTRAL_SERVER_PORT=5684
export ENABLE_NETWORK_DEBUG=1
export LOG_DTLS_HANDSHAKE=1
export DTLS_ACK_TIMEOUT_SECONDS=15

# Luego ejecutar
./api_gateway  # ✅ Usa configuración personalizada
```

### 📊 **Análisis de Rendimiento**

```bash
# Benchmark automático
./run_100_api_gateways.sh benchmark

