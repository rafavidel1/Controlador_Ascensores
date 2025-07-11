# 🏢 Servidor Central - Sistema de Control de Ascensores

[![Build Status](https://img.shields.io/badge/build-passing-brightgreen.svg)](https://github.com/user/repo)
[![Protocol](https://img.shields.io/badge/protocol-CoAP%2FDTLS--PSK-orange.svg)](https://tools.ietf.org/html/rfc7252)
[![Kubernetes](https://img.shields.io/badge/deployment-Kubernetes-blue.svg)](https://kubernetes.io/)
[![Algorithm](https://img.shields.io/badge/algorithm-Intelligent-green.svg)](https://en.wikipedia.org/wiki/Elevator_algorithm)

> **Servidor central inteligente para asignación optimizada de ascensores con algoritmo avanzado en tiempo real y despliegue automático en Kubernetes - 100% automatizado**

## 📋 Tabla de Contenidos

- [🎯 Descripción General](#-descripción-general)
- [🚀 Inicio Rápido - 100% Automatizado](#-inicio-rápido---100-automatizado)
- [🏗️ Arquitectura del Servidor](#️-arquitectura-del-servidor)
- [🧠 Algoritmo Inteligente](#-algoritmo-inteligente)
- [🔒 Seguridad DTLS-PSK](#-seguridad-dtls-psk)
- [🐳 Despliegue en Kubernetes](#-despliegue-en-kubernetes)
- [📊 Persistencia y Base de Datos](#-persistencia-y-base-de-datos)
- [🐛 Solución de Problemas](#-solución-de-problemas)

## 🎯 Descripción General

El **Servidor Central** es el cerebro del sistema de control de ascensores. Implementa un algoritmo inteligente que considera la posición en tiempo real de los ascensores para optimizar asignaciones, con persistencia SQLite y despliegue automático en Kubernetes mediante scripts zero-config.

### 🌟 Características Clave

- **⚡ Compilación Automática**: `./build_servidor_central.sh` - Todo en un comando
- **🚀 Despliegue Zero-Config**: `./deploy.sh` - Kubernetes automático
- **🧠 Algoritmo Inteligente**: Considera posición actual y destino en tiempo real
- **🔒 Seguridad DTLS-PSK**: Comunicación cifrada con autenticación mutua
- **📊 Algoritmo en Tiempo Real**: Asignación óptima basada en datos actuales
- **🐳 Kubernetes Ready**: Escalabilidad horizontal automática

## 🚀 Inicio Rápido - 100% Automatizado

### ⚡ Prerequisitos (100% Autónomos - Sin instalación manual)

```bash
# ✅ NINGÚN PREREQUISITO MANUAL NECESARIO
# El script build_servidor_central.sh instala TODAS las dependencias automáticamente:
# - build-essential, cmake, gcc, make, pkg-config, git
# - libcoap (compilado desde fuente)
# - OpenSSL, cJSON, json-c, libcurl
# - Configuración automática de variables de entorno
# - Corrección automática de clock skew
# - Algoritmo inteligente en memoria (SIN SQLite)

# Para Kubernetes (opcional):
# El script verifica minikube/kubectl y guía la instalación si es necesario

### 🎯 Compilación Automática 

```bash
# Compilar TODO automáticamente
./build_servidor_central.sh

# Salida esperada:
# ✅ Checking dependencies...
# ✅ Configuring CMake...
# ✅ Building Servidor Central...
# ✅ Build completed successfully!
# ✅ Executable created: ./servidor_central
```


#### Despliegue en Kubernetes 

```bash
# Despliegue automático en minikube
./deploy.sh

# Salida esperada:
# ✅ Building Docker image...
# ✅ Loading image to minikube...
# ✅ Applying Kubernetes manifests...
# ✅ Deployment created successfully!
# ✅ Service exposed on NodePort
# ✅ Server available at: http://192.168.49.2:5684
```

### 🎯 Verificación Automática

```bash
# Verificar despliegue K8s
kubectl get pods -l app=servidor-central

# Salida esperada:
# NAME                                READY   STATUS    RESTARTS
# servidor-central-7d4f8b6c8d-xyz123  1/1     Running   0

# Ver logs en tiempo real
kubectl logs -f deployment/servidor-central

# Probar conectividad
curl -k https://192.168.49.2:5684/status
```

### 📊 **Endpoints Automáticos**

| Endpoint | Método | Descripción | Procesamiento |
|----------|--------|-------------|---------------|
| `/peticion_piso` | POST | Llamada desde piso | ✅ Algoritmo inteligente automático |
| `/peticion_cabina` | POST | Solicitud desde cabina | ✅ Optimización de ruta automática |

## 🧠 Algoritmo Inteligente

### 🎯 **Algoritmo con Posición en Tiempo Real**

El servidor central recibe la posición actualizada de cada ascensor durante el movimiento:

```json
{
  "elevadores_estado": [{
    "id_ascensor": "EDI1A1",
    "piso_actual": 3,        // ✅ Posición actualizada en tiempo real
    "destino_actual": 7,     // ✅ Durante movimiento
    "disponible": false,
    "estado_puerta": "CERRADA"
  }]
}
```

### 🤖 **Lógica de Optimización Automática**

```bash
# El algoritmo considera automáticamente:
# ✅ Distancia desde posición actual (no estática)
# ✅ Dirección de movimiento del ascensor
# ✅ Compatibilidad de ruta (puede recoger en el camino)
# ✅ Tiempo estimado de llegada real
# ✅ Eficiencia de la asignación
```

### 📊 **Scoring Automático**

```c
// Algoritmo de scoring (automático)
float score = 0.0;

// Distancia (actualizada en tiempo real)
int distance = abs(piso_actual - piso_origen);  // piso_actual viene del API Gateway
if (distance == 0) score += 100.0;  // Mismo piso
else score += (50.0 / distance);   // Más cerca = mejor score

// Compatibilidad de ruta (ascensor en movimiento)
if (va_subiendo && piso_actual <= piso_origen && piso_origen <= destino_actual) {
    score += 30.0;  // Puede recoger en el camino
}

// Eficiencia total
return score;
```

### 🔄 **Logging Automático del Algoritmo**

```bash
# Logs automáticos del algoritmo
[INFO] Evaluando ascensor EDI1A1: piso_actual=3, destino_actual=7, score=85.5
[INFO] Evaluando ascensor EDI1A2: piso_actual=1, destino_actual=-1, score=92.0
[INFO] Mejor ascensor seleccionado: EDI1A2 (score: 92.0)
[INFO] Tarea T_1640995200123 asignada a EDI1A2
```

## 🔒 Seguridad DTLS-PSK

### 🔐 **Configuración Automática**

```bash
# Validación automática de PSK
cat psk_keys.txt | wc -l  # ✅ 15,000 claves
grep "Gateway_Client_" psk_keys.txt | head -5

# Salida esperada:
# Gateway_Client_0001 41414141...
# Gateway_Client_0002 42424242...
# Gateway_Client_0003 43434343...
# Gateway_Client_0004 44444444...
# Gateway_Client_0005 45454545...
```

### 🛡️ **Autenticación Mutua Automática**

```bash
# El servidor valida automáticamente:
# ✅ Identidad del cliente (Gateway_Client_XXXX)
# ✅ Clave PSK correspondiente
# ✅ Estado de la sesión DTLS
# ✅ Timeouts y reconexiones
```

### 🔄 **Gestión de Sesiones**

```c
// Validación automática de sesión
if (coap_session_get_state(session) != COAP_SESSION_STATE_ESTABLISHED) {
    // Respuesta automática de error 401
    return COAP_RESPONSE_CODE_UNAUTHORIZED;
}
```

## 🐳 Despliegue en Kubernetes

### 🚀 **Despliegue Completamente Automático**

```bash
# Un solo comando para desplegar todo
./deploy.sh

# El script hace automáticamente:
# ✅ Construye imagen Docker
# ✅ Carga imagen a minikube
# ✅ Aplica manifiestos K8s
# ✅ Configura servicios
# ✅ Expone puertos
# ✅ Verifica despliegue
```

### 🔧 **Configuración Automática**

```yaml
# kustomize/deployment.yaml (aplicado automáticamente)
apiVersion: apps/v1
kind: Deployment
metadata:
  name: servidor-central
spec:
  replicas: 1
  selector:
    matchLabels:
      app: servidor-central
  template:
    metadata:
      labels:
        app: servidor-central
    spec:
      containers:
      - name: servidor-central
        image: servidor-central:latest
        ports:
        - containerPort: 5684
        resources:
          requests:
            memory: "64Mi"
            cpu: "250m"
          limits:
            memory: "128Mi"
            cpu: "500m"
```

### 📊 **Monitoreo Automático**

```bash
# Comandos de monitoreo automático
kubectl get pods -l app=servidor-central -w  # Watch pods
kubectl logs -f deployment/servidor-central  # Logs en tiempo real
kubectl top pods -l app=servidor-central     # Uso de recursos

# Métricas automáticas
kubectl get hpa servidor-central-hpa         # Horizontal Pod Autoscaler
```

### 🔄 **Scaling Automático**

```yaml
# kustomize/hpa.yaml (aplicado automáticamente)
apiVersion: autoscaling/v2
kind: HorizontalPodAutoscaler
metadata:
  name: servidor-central-hpa
spec:
  scaleTargetRef:
    apiVersion: apps/v1
    kind: Deployment
    name: servidor-central
  minReplicas: 1
  maxReplicas: 10
  metrics:
  - type: Resource
    resource:
      name: cpu
      target:
        type: Utilization
        averageUtilization: 70
```

## 🐛 Solución de Problemas

### 🔍 **Problemas Comunes**

#### Error: "minikube not found"
```bash
# Instalar minikube (prerequisito obligatorio)
curl -LO https://storage.googleapis.com/minikube/releases/latest/minikube-linux-amd64
sudo install minikube-linux-amd64 /usr/local/bin/minikube

# Iniciar minikube
minikube start

# Verificar
kubectl get nodes
```

#### Error: "Docker image build failed"
```bash
# Solución automática
./build_servidor_central.sh  # ✅ Compila primero

# Luego desplegar
./deploy.sh  # ✅ Construye imagen automáticamente
```

#### Error: "Port 5684 already in use"
```bash
# Verificar procesos
sudo netstat -tlnp | grep :5684
sudo kill -9 <PID>

# O usar Kubernetes (recomendado)
./deploy.sh  # ✅ Maneja puertos automáticamente
```

#### Error: "DTLS handshake failed"
```bash
# Verificar claves PSK
ls -la psk_keys.txt
wc -l psk_keys.txt  # Debe ser 15,000 líneas

# Verificar configuración
grep "Gateway_Client_" psk_keys.txt | head -5
```

### 📋 **Logs de Diagnóstico**

```bash
# Logs locales
./servidor_central > server.log 2>&1 &
tail -f server.log

# Logs en Kubernetes
kubectl logs -f deployment/servidor-central

# Logs de eventos K8s
kubectl get events --sort-by=.metadata.creationTimestamp
```

### 🔄 **Comandos de Diagnóstico**

```bash
# Verificar estado completo
kubectl get all -l app=servidor-central

# Verificar configuración
kubectl describe deployment servidor-central

# Verificar recursos
kubectl top pods -l app=servidor-central

# Verificar conectividad
kubectl exec -it deployment/servidor-central -- netstat -tlnp
```

## 💡 Uso Avanzado

### 🎯 **Variables de Entorno**

```bash
# Configuración personalizada (antes del despliegue)
export DTLS_PSK_FILE="custom_psk_keys.txt"
export ALGORITHM_MODE="intelligent"  # Modo de algoritmo inteligente
export COAP_LISTEN_PORT=5685

# Luego desplegar
./deploy.sh  # ✅ Usa configuración personalizada
```

### 📊 **Análisis de Rendimiento**

```bash
# Métricas de Kubernetes
kubectl top pods -l app=servidor-central

# Logs de algoritmo
kubectl logs -f deployment/servidor-central | grep "score"
```


