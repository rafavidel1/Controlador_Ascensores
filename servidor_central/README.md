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

### 🔐 **Seguridad DTLS-PSK Avanzada**
- **Cifrado DTLS 1.2 con Pre-Shared Keys**: Comunicación segura de extremo a extremo
- **Sistema de claves determinístico**: 15,000 claves únicas basadas en identidad del cliente
- **Archivo de claves PSK**: `psk_keys.txt` con claves pre-generadas para sincronización perfecta
- **Gestión de sesiones DTLS**: Evita múltiples conexiones simultáneas y timeouts
- **Configuración por variables de entorno**: Sistema flexible y seguro de configuración
- **Validación automática**: Verificación de claves PSK y estado de conexiones

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

1. **Recepción**: API Gateway envía solicitud CoAP/DTLS-PSK
2. **Autenticación**: Verificación de clave PSK basada en identidad del cliente
3. **Validación**: Verificación de formato JSON y datos de entrada
4. **Procesamiento**: Ejecución de algoritmos de asignación optimizada
5. **Persistencia**: Actualización de estado en base de datos SQLite
6. **Respuesta**: Generación de respuesta estructurada con tarea asignada

## 🚀 Inicio Rápido

### 🐳 **Despliegue en Kubernetes (Recomendado)**

El sistema está optimizado para ejecutarse en Kubernetes con Minikube y MetalLB:

```bash
# 1. Desplegar automáticamente
./deploy.sh

# El script automáticamente:
# - Configura el entorno de Docker de Minikube
# - Verifica y construye la imagen Docker con libcjson1
# - Instala y configura MetalLB con IPAddressPool (192.168.49.2-192.168.49.10)
# - Despliega el servidor central con imagePullPolicy: Never
# - Asigna IP externa automáticamente

# 2. Verificar el despliegue
kubectl get pods
kubectl get svc
kubectl logs -f deployment/servidor-central-deployment

# 3. Acceder al servicio
# El servicio estará disponible en la IP asignada por MetalLB
```

### 🐳 **Docker Optimizado (Desarrollo)**

El proyecto incluye un **Dockerfile optimizado con multi-stage build**:

```bash
# Construir imagen optimizada
docker build -t servidor-central .

# Ejecutar contenedor
docker run -d \
  --name servidor-central \
  -p 5684:5684 \
  -v $(pwd)/data:/app/data \
  servidor-central

# Verificar funcionamiento
docker logs servidor-central
```

**Características del Docker optimizado:**
- ✅ **Multi-stage build**: Compilación y runtime separados
- ✅ **Dependencias mínimas**: Solo libcjson1, libssl3, libc6
- ✅ **Seguridad mejorada**: Usuario no-root, sin herramientas de compilación
- ✅ **Configuración por variables de entorno**: Sistema flexible
- ✅ **Archivo de claves PSK incluido**: `psk_keys.txt` copiado al contenedor

### 📦 **Instalación Local**

#### Prerrequisitos

```bash
# Ubuntu/Debian
sudo apt-get update
sudo apt-get install -y build-essential cmake pkg-config
sudo apt-get install -y libcjson-dev libsqlite3-dev libssl-dev
```

#### Compilación

```bash
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

#### Ejecución

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
sudo apt-get install -y build-essential cmake pkg-config
sudo apt-get install -y libcjson-dev libsqlite3-dev libssl-dev
sudo apt-get install -y git wget ca-certificates

# Compilar libcoap desde fuente
cd Librerias/libcoap
./autogen.sh
./configure --prefix=/usr/local --enable-dtls --with-openssl --disable-doxygen --disable-manpages
make -j$(nproc) && sudo make install && sudo ldconfig
```

</details>

<details>
<summary><strong>CentOS/RHEL</strong></summary>

```bash
# Paquetes base
sudo yum groupinstall "Development Tools"
sudo yum install cmake pkg-config
sudo yum install libcjson-devel sqlite-devel openssl-devel
sudo yum install git wget ca-certificates

# Compilar libcoap desde fuente
cd Librerias/libcoap
./autogen.sh
./configure --prefix=/usr/local --enable-dtls --with-openssl --disable-doxygen --disable-manpages
make -j$(nproc) && sudo make install && sudo ldconfig
```

</details>

### 🔨 Compilación

```bash
# Crear directorio de build
mkdir build && cd build

# Configurar CMake
cmake -DBUILD_SERVIDOR_CENTRAL=ON -DCMAKE_BUILD_TYPE=Release ..

# Compilar
make -j$(nproc)

# Instalar (opcional)
sudo make install
```

## ⚙️ Configuración

### 🔐 Configuración DTLS-PSK

El sistema utiliza un archivo de claves PSK pre-generadas:

```bash
# Archivo de claves PSK (15,000 claves únicas)
psk_keys.txt

# Formato de las claves:
# client_id:psk_key
# Ejemplo:
# gateway_001:abc123def456
# gateway_002:xyz789uvw012
```

### 🌍 Variables de Entorno

```bash
# Configuración del servidor
export SERVIDOR_PUERTO=5684
export SERVIDOR_HOST=0.0.0.0

# Configuración de base de datos
export DB_PATH=/app/data/servidor_central.db

# Configuración de logging
export LOG_LEVEL=INFO
export LOG_FILE=/app/logs/servidor_central.log

# Configuración DTLS
export DTLS_PSK_FILE=/app/psk_keys.txt
export DTLS_TIMEOUT=30
```

### 📁 Estructura de Archivos

```
servidor_central/
├── 📁 src/                    # Código fuente
├── 📁 include/                # Headers
├── 📁 kustomize/              # Configuración Kubernetes
│   ├── deployment.yaml        # Deployment con imagePullPolicy: Never
│   ├── service.yaml           # Service LoadBalancer
│   └── hpa.yaml              # Horizontal Pod Autoscaler
├── 🐳 Dockerfile              # Multi-stage build optimizado
├── 📜 deploy.sh               # Script de despliegue automatizado
├── 📜 metallb-config.yaml     # Configuración MetalLB (IPAddressPool)
├── 📜 psk_keys.txt            # 15,000 claves PSK pre-generadas
└── 📖 README.md               # Este archivo
```

## 🔌 API Endpoints

### 📡 Endpoints CoAP/DTLS-PSK

| Endpoint | Método | Descripción | Autenticación |
|----------|--------|-------------|---------------|
| `/peticion_piso` | POST | Solicitar asignación de ascensor | DTLS-PSK |
| `/peticion_cab` | POST | Solicitar ascensor específico | DTLS-PSK |
| `/.well-known/core` | GET | Descubrimiento de recursos | DTLS-PSK |

### 📝 Formato de Peticiones

```json
{
  "edificio_id": "edificio_001",
  "piso_origen": 5,
  "piso_destino": 10,
  "prioridad": "normal",
  "timestamp": 1640995200
}
```

### 📤 Formato de Respuestas

```json
{
  "status": "success",
  "ascensor_asignado": "ascensor_003",
  "tiempo_estimado": 45,
  "tarea_id": "tarea_12345",
  "timestamp": 1640995200
}
```

## 💾 Base de Datos

### 🗄️ Esquema SQLite

```sql
-- Tabla de edificios
CREATE TABLE edificios (
    id TEXT PRIMARY KEY,
    nombre TEXT NOT NULL,
    num_ascensores INTEGER DEFAULT 4,
    configuracion TEXT
);

-- Tabla de estado de ascensores
CREATE TABLE ascensores_estado (
    id TEXT PRIMARY KEY,
    edificio_id TEXT,
    piso_actual INTEGER DEFAULT 1,
    estado TEXT DEFAULT 'disponible',
    ultima_actualizacion TIMESTAMP,
    FOREIGN KEY (edificio_id) REFERENCES edificios(id)
);

-- Tabla de tareas asignadas
CREATE TABLE tareas (
    id TEXT PRIMARY KEY,
    edificio_id TEXT,
    ascensor_id TEXT,
    piso_origen INTEGER,
    piso_destino INTEGER,
    estado TEXT DEFAULT 'pendiente',
    timestamp TIMESTAMP,
    FOREIGN KEY (edificio_id) REFERENCES edificios(id),
    FOREIGN KEY (ascensor_id) REFERENCES ascensores_estado(id)
);
```

## 🧪 Testing

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

### 🐳 Tests de Contenedor

```bash
# Test de imagen Docker
docker build -t servidor-central-test .
docker run --rm servidor-central-test ./run_tests.sh
```

## 🔐 Seguridad

### 🔑 Sistema de Claves PSK

El sistema utiliza un archivo de 15,000 claves PSK pre-generadas:

```bash
# Generación de claves (ya incluido)
python3 generate_psk_keys.py

# Verificación de claves
./verify_psk_keys.sh
```

### 🔒 Configuración DTLS

```c
// Configuración DTLS-PSK
#define DTLS_PSK_FILE "/app/psk_keys.txt"
#define DTLS_TIMEOUT 30
#define DTLS_MTU 1280
#define DTLS_RETRANSMIT_TIMEOUT 2
```

### 🛡️ Medidas de Seguridad

- ✅ **Cifrado de extremo a extremo** con DTLS 1.2
- ✅ **Autenticación mutua** mediante PSK
- ✅ **Validación de claves** contra archivo pre-generado
- ✅ **Timeouts optimizados** para prevenir ataques
- ✅ **Usuario no-root** en contenedor Docker
- ✅ **Dependencias mínimas** para reducir superficie de ataque

## 📊 Monitorización

### 📈 Métricas en Tiempo Real

```bash
# Ver logs del servidor
kubectl logs -f deployment/servidor-central-deployment

# Ver métricas de recursos
kubectl top pods

# Ver estado de servicios
kubectl get svc
```

### 📊 Logs Estructurados

```json
{
  "timestamp": "2024-01-15T10:30:00Z",
  "level": "INFO",
  "component": "assignment_engine",
  "message": "Ascensor asignado: ascensor_003",
  "metadata": {
    "edificio_id": "edificio_001",
    "piso_origen": 5,
    "piso_destino": 10,
    "tiempo_procesamiento": 15
  }
}
```

## 🐛 Solución de Problemas

### 🔍 Problemas Comunes

<details>
<summary><strong>Error: ImagePullBackOff</strong></summary>

```bash
# Solución: Configurar entorno de Docker de Minikube
eval $(minikube docker-env)
docker build -t servidor-central .

# Verificar que imagePullPolicy: Never esté configurado
kubectl get deployment servidor-central-deployment -o yaml
```

</details>

<details>
<summary><strong>Error: libcjson.so.1 not found</strong></summary>

```bash
# Solución: Agregar libcjson1 al Dockerfile
RUN apt-get install -y libcjson1

# Reconstruir imagen
docker build -t servidor-central .
```

</details>

<details>
<summary><strong>Error: Service en estado pending</strong></summary>

```bash
# Solución: Verificar configuración de MetalLB
kubectl get ipaddresspools -n metallb-system
kubectl apply -f metallb-config.yaml
```

</details>

### 🛠️ Herramientas de Debugging

```bash
# Ver logs detallados
kubectl logs -f deployment/servidor-central-deployment

# Ver eventos del pod
kubectl describe pod <pod-name>

# Ver configuración del deployment
kubectl get deployment servidor-central-deployment -o yaml

# Ver estado de servicios
kubectl get svc -o wide
```

## 📚 Documentación

### 📖 Documentación Técnica

- [📋 Especificación CoAP/DTLS-PSK](./docs/coap-dtls-spec.md)
- [🏗️ Arquitectura del Sistema](./docs/architecture.md)
- [🔐 Guía de Seguridad](./docs/security.md)
- [📊 Guía de Monitorización](./docs/monitoring.md)

### 🔗 Enlaces Útiles

- [📦 Repositorio Principal](../README.md)
- [🔌 API Gateway](../api_gateway/README.md)
- [🧪 Tests](../tests/README.md)
- [📊 Monitorización](../monitoring/README.md)

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
- [ ] Imagen Docker construida correctamente

---

## 📄 Licencia

Este proyecto está bajo la Licencia MIT. Ver el archivo [LICENSE](../LICENSE) para más detalles.

---

**🏢 Sistema de Control de Ascensores** - Comunicación segura CoAP/DTLS-PSK para gestión inteligente de ascensores multi-edificio.
