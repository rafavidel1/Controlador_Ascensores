# 🏢 Sistema de Control de Ascensores - CoAP/DTLS-PSK

[![Build Status](https://img.shields.io/badge/build-passing-brightgreen.svg)](https://github.com/user/repo)
[![Version](https://img.shields.io/badge/version-2.0-blue.svg)](https://github.com/user/repo/releases)
[![Language](https://img.shields.io/badge/language-C-blue.svg)](https://en.wikipedia.org/wiki/C_(programming_language))
[![Protocol](https://img.shields.io/badge/protocol-CoAP%2FDTLS--PSK-orange.svg)](https://tools.ietf.org/html/rfc7252)

> **Sistema distribuido para la gestión inteligente de ascensores mediante comunicación segura CoAP/DTLS-PSK con compilación y ejecución 100% automatizada**

## 📋 Tabla de Contenidos

- [🎯 Descripción General](#-descripción-general)
- [🏗️ Arquitectura del Sistema](#️-arquitectura-del-sistema)
- [🚀 Inicio Rápido - 100% Automatizado](#-inicio-rápido---100-automatizado)
- [📦 Componentes del Sistema](#-componentes-del-sistema)
- [🧪 Testing Automatizado](#-testing-automatizado)
- [🔒 Seguridad DTLS-PSK](#-seguridad-dtls-psk)
- [📊 Monitoreo y Logging](#-monitoreo-y-logging)
- [🐛 Solución de Problemas](#-solución-de-problemas)

## 🎯 Descripción General

El **Sistema de Control de Ascensores** es una solución distribuida de alta disponibilidad diseñada para la gestión inteligente y eficiente de sistemas de ascensores en edificios modernos. Implementa comunicación segura mediante **CoAP/DTLS-PSK** y proporciona capacidades de monitoreo en tiempo real, análisis de rendimiento y escalabilidad horizontal.

### 🌟 Características Clave

- **⚡ Compilación Automática**: Scripts automatizados para build completo
- **🚀 Ejecución Zero-Config**: Configuración automática y ejecución inmediata
- **🔒 Seguridad DTLS-PSK**: Comunicación cifrada end-to-end
- **📊 Algoritmo Inteligente**: Asignación optimizada de ascensores con posición en tiempo real
- **🧪 Testing Completo**: 34 tests unitarios + tests de integración automatizados
- **📈 Simulación Masiva**: 100 edificios, 1000 peticiones de prueba

## 🏗️ Arquitectura del Sistema

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                           SISTEMA DE CONTROL DE ASCENSORES                  │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                             │
│  ┌─────────────────┐    CoAP/DTLS-PSK    ┌─────────────────────────────┐   │
│  │                 │◄──────────────────►│                             │   │
│  │   API GATEWAY   │    Puerto 5684      │      SERVIDOR CENTRAL       │   │
│  │                 │                     │                             │   │
│  │ • Puerto 5683   │                     │ • Asignación Inteligente    │   │
│  │ • Puente CAN    │                     │ • Algoritmos de Optimización│   │
│  │ • Estado Local  │                     │ • Persistencia SQLite       │   │
│  │ • Simulación    │                     │ • Kubernetes Ready          │   │
│  └─────────────────┘                     └─────────────────────────────┘   │
│           │                                           │                     │
│           │ Frames CAN                               │ Kubernetes          │
│           ▼                                           ▼                     │
│  ┌─────────────────┐                     ┌─────────────────────────────┐   │
│  │   SIMULADOR     │                     │       MINIKUBE              │   │
│  │   ASCENSORES    │                     │                             │   │
│  │                 │                     │ • Auto-despliegue           │   │
│  │ • 100 Edificios │                     │ • Scaling automático        │   │
│  │ • 1000 Peticiones│                    │ • LoadBalancer              │   │
│  │ • Datos JSON    │                     │ • Monitoring                │   │
│  └─────────────────┘                     └─────────────────────────────┘   │
│                                                                             │
└─────────────────────────────────────────────────────────────────────────────┘
```

## 🚀 Inicio Rápido - 100% Automatizado

### ⚡ Prerequisitos (Solo una vez)

```bash
# Ubuntu/Debian
sudo apt-get update
sudo apt-get install -y build-essential cmake pkg-config git
sudo apt-get install -y libcjson-dev libssl-dev sqlite3 libsqlite3-dev

# Para servidor central (si usas Kubernetes)
# Instalar minikube: https://minikube.sigs.k8s.io/docs/start/
curl -LO https://storage.googleapis.com/minikube/releases/latest/minikube-linux-amd64
sudo install minikube-linux-amd64 /usr/local/bin/minikube
minikube start
```

### 🎯 Compilación Automática (Zero-Config)

```bash
# Clonar y compilar 
git clone <repository-url>
cd sistema-control-ascensores

# 1. Compilar API Gateway (incluye libcoap automáticamente)
cd api_gateway
./build_api_gateway.sh           # ✅ Compila TODO automáticamente

# 2. Compilar Servidor Central
cd ../servidor_central
./build_servidor_central.sh      # ✅ Compila TODO automáticamente

# 3. Compilar y ejecutar Tests
cd ../tests
./run_all_tests.sh               # ✅ Ejecuta 34 tests automáticamente
```

### 🚀 Ejecución Automatizada

#### Opción 1: Despliegue en Kubernetes 

```bash
# Despliegue automático en minikube
cd servidor_central
./deploy.sh                      # ✅ Despliega automáticamente en K8s

# Luego ejecutar API Gateway
cd ../api_gateway
./api_gateway                    # ✅ Se conecta al servidor en K8s
```

#### Opción 2: Simulación Masiva (Testing)

```bash
# Ejecutar 100 API Gateways simultáneos
cd api_gateway
./run_100_api_gateways.sh        # ✅ Crea 100 instancias automáticamente

# Ver logs en tiempo real
tail -f logs/$(date +%Y-%m-%d)/*.md
```

### 🎯 Verificación Automática

```bash
# Verificar que todo funciona correctamente
cd tests
./run_all_tests.sh               # ✅ 34 tests pasan automáticamente

# Ver reportes automáticos
cd ../api_gateway
ls -la logs/$(date +%Y-%m-%d)/   # ✅ Reportes PDF/Markdown automáticos
```

## 📦 Componentes del Sistema

### 🔄 **API Gateway** (`api_gateway/`)
- **Compilación**: `./build_api_gateway.sh` (automática)
- **Ejecución**: `./api_gateway` (zero-config)
- **Funciones**:
  - Puente CAN-CoAP automático
  - Estado local de ascensores en tiempo real
  - Simulación integrada de 100 edificios
  - Logging automático con reportes PDF

### 🏢 **Servidor Central** (`servidor_central/`)
- **Compilación**: `./build_servidor_central.sh` (automática)
- **Ejecución**: `./servidor_central` o `./deploy.sh` (K8s)
- **Funciones**:
  - Algoritmo inteligente de asignación de ascensores
  - Considera posición actual y destino en tiempo real
  - Persistencia automática en SQLite
  - Escalabilidad horizontal con Kubernetes

### 🧪 **Testing Suite** (`tests/`)
- **Ejecución**: `./run_all_tests.sh` (automática)
- **Incluye**:
  - 34 tests unitarios
  - Tests de integración CoAP/DTLS
  - Mocks inteligentes
  - Reportes automáticos de cobertura

## 🧪 Testing Automatizado

```bash
# Ejecutar TODOS los tests automáticamente
cd tests
./run_all_tests.sh

# Salida esperada:
# ✅ Running 34 unit tests...
# ✅ Running integration tests...
# ✅ Running DTLS handshake tests...
# ✅ ALL TESTS PASSED!
# ✅ Coverage reports generated automatically
```

## 🔒 Seguridad DTLS-PSK

### 🔐 **Configuración Automática**
- **Claves PSK**: 15,000 claves pre-generadas en `psk_keys.txt`
- **Identidades**: Formato automático `Gateway_Client_XXXX`
- **Algoritmo**: Clave determinística basada en identidad del cliente
- **Timeouts**: Configuración automática para evitar timeouts

### 🛡️ **Autenticación Mutua**
- El servidor valida la identidad del cliente
- El cliente valida el certificado del servidor
- Cifrado end-to-end automático
- Gestión de sesiones DTLS optimizada

## 📊 Monitoreo y Logging

### 📈 **Reportes Automáticos**
- **Ubicación**: `api_gateway/logs/YYYY-MM-DD/`
- **Formatos**: Markdown (`.md`) y PDF (automático)
- **Contenido**: Timestamped execution logs con métricas
- **Frecuencia**: Automática cada ejecución

### 📊 **Algoritmo Inteligente**
- **Posición en tiempo real**: El servidor recibe la posición actual del ascensor durante el movimiento
- **Optimización de ruta**: Considera ascensores en movimiento para optimizar asignaciones
- **Eficiencia calculada**: Algoritmo que evalúa distancia, disponibilidad y ruta
- **Logging detallado**: Cada decisión del algoritmo se registra automáticamente

## 🐛 Solución de Problemas

### 🔍 **Problemas Comunes**

#### Error: "libcoap not found"
```bash
# Solución automática
cd api_gateway
./build_api_gateway.sh  # ✅ Instala libcoap automáticamente
```

#### Error: "DTLS handshake failed"
```bash
# Solución automática
cd servidor_central
./build_servidor_central.sh  # ✅ Configura DTLS automáticamente
```

#### Error: "Port already in use"
```bash
# Solución automática
cd api_gateway
./api_gateway 6000       # ✅ Usa puerto personalizado automáticamente
```

### 📋 **Logs Automáticos**
- **API Gateway**: `api_gateway/logs/YYYY-MM-DD/ejecucion_HH-MM-SS-mmm.md`
- **Servidor Central**: Output en consola con colores automáticos
- **Tests**: `tests/temp-build-tests/` con reportes automáticos

### 🔄 **Scripts de Diagnóstico**
- **API Gateway**: `./build_api_gateway.sh` incluye diagnóstico automático
- **Servidor Central**: `./build_servidor_central.sh` incluye validación automática
- **Tests**: `./run_all_tests.sh` incluye verificación automática

## 💡 Uso Avanzado

### 🎯 **Configuración Personalizada**
```bash
# Variables de entorno 
export GW_LISTEN_PORT=6000
export CENTRAL_SERVER_IP=192.168.1.100
export ENABLE_NETWORK_DEBUG=1

# Luego ejecutar normalmente
./api_gateway  # ✅ Usa configuración personalizada automáticamente
```

### 📊 **Análisis de Rendimiento**
```bash
# Simulación de carga masiva
cd api_gateway
./run_100_api_gateways.sh  # ✅ 100 instancias simultáneas

