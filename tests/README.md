# Sistema de Pruebas - Control de Ascensores

Este directorio contiene el sistema completo de pruebas para el proyecto de control de ascensores.

## 🚀 Inicio Rápido

### 1. Ejecutar todas las pruebas (recomendado)
```bash
cd tests/
./run_all_tests.sh
```

### 2. Ejecución con limpieza automática
```bash
./run_all_tests.sh --clean         # Limpiar build anterior y ejecutar
```

### 3. Opciones disponibles
```bash
./run_all_tests.sh --help          # Mostrar ayuda
./run_all_tests.sh --clean         # Limpiar build anterior
./run_all_tests.sh --clean-only    # Solo limpiar sin ejecutar
./run_all_tests.sh --verbose       # Salida detallada
./run_all_tests.sh --no-reports    # No generar reportes
```

## 📦 Dependencias

### Instalación Automática
**El script instala automáticamente todas las dependencias necesarias:**
- **CMake** (>= 3.10)
- **GCC** o **Clang**
- **pkg-config**
- **build-essential**
- **libcoap-3-dev** - Comunicación CoAP
- **libcjson-dev** - Parsing JSON
- **libcunit1-dev** - Framework de pruebas
- **libssl-dev** - Seguridad SSL/TLS
- **libc6-dev** - Biblioteca estándar C
- **libtool**
- **autotools-dev**
- **automake**

### Verificación Manual (opcional)
Si necesitas verificar qué dependencias están instaladas:
```bash
./run_all_tests.sh --verbose
```

## 🧪 Tipos de Pruebas

### Pruebas Unitarias (5 módulos)
- **test_elevator_state_manager** - Gestión de estado de ascensores
- **test_can_bridge** - Puente CAN-CoAP
- **test_api_handlers** - Manejadores de API
- **test_servidor_central** - Servidor central
- **test_psk_security** - Seguridad PSK-DTLS

### Pruebas de Integración (1 módulo)
- **test_can_to_coap** - Integración CAN a CoAP completa

## 📊 Reportes Generados

Los reportes se generan automáticamente en `tests/temp-build-tests/` después de la ejecución. Se recomienda usar `./run_all_tests.sh --clean` para obtener reportes actualizados.

### Estructura de Reportes
```
tests/temp-build-tests/
├── reporte_consolidado.txt              # Reporte principal con resumen ejecutivo
├── test_results.json                    # Datos estructurados en JSON
├── test_api_handlers_report.txt         # Reporte: Manejadores de API
├── test_can_bridge_report.txt           # Reporte: Puente CAN
├── test_can_to_coap_report.txt          # Reporte: Integración CAN-CoAP
├── test_elevator_state_manager_report.txt # Reporte: Gestor de Estado de Ascensores
├── test_psk_security_report.txt         # Reporte: Seguridad PSK-DTLS
└── test_servidor_central_report.txt     # Reporte: Servidor Central
```

### Descripción de Reportes

- **`reporte_consolidado.txt`** - Reporte principal con resumen ejecutivo, estadísticas globales y detalles completos de todas las pruebas
- **`test_results.json`** - Datos estructurados para integración con herramientas de CI/CD
- **Reportes individuales** - Detalles específicos de cada módulo con análisis técnico detallado

### Recomendaciones de Uso

```bash
# Ejecución recomendada (limpia builds anteriores)
./run_all_tests.sh --clean

# Los reportes se generarán en temp-build-tests/
ls -la temp-build-tests/*report*.txt
cat temp-build-tests/reporte_consolidado.txt
```

## 🛠️ Estructura del Proyecto

```
tests/
├── run_all_tests.sh           # Script principal (instala dependencias automáticamente)
├── CMakeLists.txt             # Configuración CMake
├── clean_temp_build.sh        # Script de limpieza
├── unit/                      # Pruebas unitarias (5 módulos)
│   ├── test_elevator_state_manager.c
│   ├── test_can_bridge.c
│   ├── test_api_handlers.c
│   ├── test_servidor_central.c
│   └── test_psk_security.c
├── integration/               # Pruebas de integración (1 módulo)
│   └── test_can_to_coap.c
├── mocks/                     # Mocks para pruebas
│   ├── CMakeLists.txt
│   ├── mock_can_interface.c
│   └── mock_can_interface.h
└── temp-build-tests/          # Build temporal (auto-generado)
    ├── reporte_consolidado.txt
    ├── test_results.json
    └── [reportes individuales]
```

## 🔍 Solución de Problemas

### Error: "Permission denied"
```bash
chmod +x run_all_tests.sh
```

### Error: "Source directory does not contain CMakeLists.txt"
- Asegúrate de estar en el directorio `tests/`
- Verifica que el archivo `CMakeLists.txt` exista en `tests/`

### Error: "Dependencies not found"
El script instala automáticamente todas las dependencias. Si persiste el error:
```bash
./run_all_tests.sh --verbose
```

## 📝 Logs y Debugging

### Ejecución con detalles
```bash
./run_all_tests.sh --verbose
```

### Verificar instalación de dependencias
```bash
./run_all_tests.sh --help
```

### Limpiar build problemático
```bash
./run_all_tests.sh --clean-only
./run_all_tests.sh
```

## 🎯 Criterios de Éxito

Las pruebas pasan exitosamente cuando:
- ✅ El script instala automáticamente todas las dependencias
- ✅ El código compila sin errores
- ✅ Las pruebas unitarias ejecutan correctamente
- ✅ Las pruebas de integración funcionan
- ✅ Los reportes se generan correctamente en `temp-build-tests/`

## 🔄 Integración Continua

Este sistema de pruebas está diseñado para:
- Ejecución automática en pipelines CI/CD
- Instalación automática de dependencias
- Generación de reportes en formatos múltiples
- Detección automática de problemas de configuración

## 📞 Soporte

Si encuentras problemas:
1. Ejecuta `./run_all_tests.sh --verbose` para diagnóstico
2. Limpia el build con `./run_all_tests.sh --clean`
3. Revisa los reportes en `temp-build-tests/` 
4. Verifica que tienes permisos de ejecución: `chmod +x run_all_tests.sh` 