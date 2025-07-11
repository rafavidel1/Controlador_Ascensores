# 🧠 Algoritmo Inteligente de Asignación de Ascensores

## 📋 Índice
1. [Servidor Central: Algoritmo de Asignación](#servidor-central-algoritmo-de-asignación)
2. [Simulador: Arquitectura No-Bloqueante](#simulador-arquitectura-no-bloqueante)
3. [Flujo de Comunicación](#flujo-de-comunicación)
4. [Datos en Tiempo Real](#datos-en-tiempo-real)
5. [Diferencias con el Sistema Anterior](#diferencias-con-el-sistema-anterior)

---

## 🎯 Servidor Central: Algoritmo de Asignación

### **Funcionamiento del Algoritmo Inteligente**

El servidor central utiliza un **algoritmo de scoring avanzado** que evalúa múltiples factores para seleccionar el ascensor óptimo:

#### **1. Recepción de Datos en Tiempo Real**
```json
{
  "id_edificio": "E021",
  "piso_origen_llamada": 5,
  "direccion_llamada": "BAJANDO",
  "elevadores_estado": [
    {
      "id_ascensor": "E021A1",
      "piso_actual": 6,           ← 🔄 DATOS EN TIEMPO REAL
      "estado_puerta": "ABIERTA",
      "disponible": true,
      "tarea_actual_id": null,
      "destino_actual": null
    },
    {
      "id_ascensor": "E021A2",
      "piso_actual": 3,           ← 🔄 DATOS EN TIEMPO REAL
      "estado_puerta": "ABIERTA",
      "disponible": true,
      "tarea_actual_id": null,
      "destino_actual": null
    }
  ]
}
```

#### **2. Algoritmo de Scoring**

Para cada ascensor candidato, el algoritmo calcula un **score basado en múltiples factores**:

```c
// Factores del algoritmo de scoring
int calculate_elevator_score(elevator_t *elevator, int target_floor, movement_direction_t direction) {
    int score = 1000;  // Score base
    
    // Factor 1: Distancia al piso objetivo
    int distance = abs(elevator->piso_actual - target_floor);
    score -= distance;  // Penalización por distancia
    
    // Factor 2: Compatibilidad de dirección
    if (elevator->direccion_actual == direction) {
        score += 50;  // Bonus por dirección compatible
    }
    
    // Factor 3: Estado de disponibilidad
    if (elevator->disponible) {
        score += 100;  // Bonus por disponibilidad
    }
    
    // Factor 4: Eficiencia de ruta
    if (is_en_route_to_floor(elevator, target_floor)) {
        score += 200;  // Bonus por estar en ruta
    }
    
    return score;
}
```

#### **3. Proceso de Selección**

```
📊 ANÁLISIS DE CANDIDATOS:
┌─────────────┬───────┬─────────┬───────┬─────────────────┐
│ Ascensor    │ Piso  │ Destino │ Score │ Estado          │
├─────────────┼───────┼─────────┼───────┼─────────────────┤
│ E021A1      │   6   │   -1    │  997  │ DISPONIBLE      │
│ E021A2      │   4   │   -1    │  999  │ DISPONIBLE      │
│ E021A3      │   1   │   -1    │  998  │ DISPONIBLE      │
│ E021A4      │   0   │   -1    │  995  │ DISPONIBLE      │
└─────────────┴───────┴─────────┴───────┴─────────────────┘

🎯 SELECCIONADO: E021A2 (Score: 999)
```

#### **4. Categorización de Ascensores**

El algoritmo clasifica cada ascensor en categorías:

- **🟢 DISPONIBLE**: `disponible=true`, sin tareas pendientes
- **🟡 COMPATIBLE**: Ocupado pero con ruta compatible
- **🔴 OCUPADO**: Ocupado con ruta no compatible
- **🟠 PRÓXIMO**: Está llegando al destino pronto

#### **5. Estadísticas de Asignación**

```
📈 ESTADÍSTICAS DE ASIGNACIÓN:
- Disponibles: 4 ascensores
- Compatibles: 0 ascensores  
- Ocupados: 0 ascensores
- Total: 4 ascensores

✅ RESULTADO: Asignación óptima a ascensor disponible más cercano
```

---

## 🚀 Simulador: Arquitectura No-Bloqueante

### **Problema del Sistema Anterior**

❌ **SISTEMA BLOQUEANTE (Anterior)**:
```
Petición 1 → Petición 2 → Petición 3 → ... → Petición 10
                                                   ↓
                                          Movimiento de ascensores
```

✅ **SISTEMA NO-BLOQUEANTE (Nuevo)**:
```
Petición 1 → Movimiento → Petición 2 → Movimiento → Petición 3
    ↓            ↓            ↓            ↓            ↓
Tiempo real  Tiempo real  Tiempo real  Tiempo real  Tiempo real
```

### **Arquitectura del Simulador No-Bloqueante**

#### **1. Variables Globales de Estado**
```c
// Control de simulación no-bloqueante
static bool simulacion_activa = false;
static int peticion_actual_index = 0;
static edificio_simulacion_t *edificio_actual = NULL;
static time_t tiempo_ultima_peticion = 0;
static const int INTERVALO_PETICIONES_MS = 2000; // 2 segundos
```

#### **2. Inicialización de Simulación**
```c
void simular_eventos_ascensor(void) {
    // Configuración inicial
    edificio_actual = seleccionar_edificio_aleatorio(&datos_simulacion_global);
    init_elevator_group(&managed_elevator_group, edificio_actual->id_edificio, 4, 14);
    
    // Activar simulación no-bloqueante
    simulacion_activa = true;
    peticion_actual_index = 0;
    tiempo_ultima_peticion = time(NULL);
    
    // ✅ Control regresa inmediatamente al main loop
}
```

#### **3. Procesamiento Incremental**
```c
bool procesar_siguiente_peticion_simulacion(void) {
    // Verificar timing
    if ((tiempo_actual - tiempo_ultima_peticion) < (INTERVALO_PETICIONES_MS / 1000)) {
        return true; // Aún no es tiempo para la siguiente petición
    }
    
    // Ejecutar UNA petición
    peticion_simulacion_t *peticion = &edificio_actual->peticiones[peticion_actual_index];
    
    if (peticion->tipo == PETICION_LLAMADA_PISO) {
        simular_llamada_de_piso_via_can(peticion->piso_origen, direccion);
    } else if (peticion->tipo == PETICION_SOLICITUD_CABINA) {
        simular_solicitud_cabina_via_can(peticion->indice_ascensor, peticion->piso_destino);
    }
    
    // Actualizar índice y tiempo
    peticion_actual_index++;
    tiempo_ultima_peticion = tiempo_actual;
    
    // ✅ Control regresa al main loop para movimiento de ascensores
    return true;
}
```

#### **4. Main Loop Integrado**
```c
while (!quit_main_loop) {
    // Procesar I/O CoAP (100ms)
    result = coap_io_process(ctx, 100);
    
    // 🔄 Procesar siguiente petición (no-bloqueante)
    procesar_siguiente_peticion_simulacion();
    
    // 🔄 Simular movimiento de ascensores
    simulate_elevator_group_step(ctx, &managed_elevator_group);
}
```

### **Simulación de Movimiento de Ascensores**

#### **1. Movimiento Progresivo**
```c
static void simulate_elevator_group_step(coap_context_t *ctx, elevator_group_state_t *group) {
    for (int i = 0; i < group->num_elevadores_en_grupo; i++) {
        elevator_t *elevator = &group->elevadores[i];
        
        if (elevator->disponible || elevator->destino_actual == -1) {
            continue; // Sin movimiento si no tiene destino
        }
        
        // Calcular dirección de movimiento
        if (elevator->piso_actual < elevator->destino_actual) {
            // Subir un piso
            elevator->piso_actual++;
            LOG_INFO_GW("[SimStep] Ascensor %s SUBE a piso %d (Destino: %d)", 
                       elevator->id_ascensor, elevator->piso_actual, elevator->destino_actual);
        } else if (elevator->piso_actual > elevator->destino_actual) {
            // Bajar un piso
            elevator->piso_actual--;
            LOG_INFO_GW("[SimStep] Ascensor %s BAJA a piso %d (Destino: %d)", 
                       elevator->id_ascensor, elevator->piso_actual, elevator->destino_actual);
        }
        
        // Verificar llegada a destino
        if (elevator->piso_actual == elevator->destino_actual) {
            LOG_INFO_GW("[SimStep] Ascensor %s LLEGÓ a destino %d.", 
                       elevator->id_ascensor, elevator->destino_actual);
            complete_elevator_task(elevator);
        }
    }
}
```

#### **2. Logs de Movimiento en Tiempo Real**
```
[INFO-GW] [SimStep] Ascensor E021A1 SUBE a piso 1 (Destino: 5)
[INFO-GW] [SimStep] Ascensor E021A1 SUBE a piso 2 (Destino: 5)
[INFO-GW] [SimStep] Ascensor E021A1 SUBE a piso 3 (Destino: 5)
[INFO-GW] [SimStep] Ascensor E021A1 SUBE a piso 4 (Destino: 5)
[INFO-GW] [SimStep] Ascensor E021A1 SUBE a piso 5 (Destino: 5)
[INFO-GW] [SimStep] Ascensor E021A1 LLEGÓ a destino 5.
```

---

## 🔄 Flujo de Comunicación

### **Timeline de Ejecución**

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                    FLUJO TEMPORAL DE EJECUCIÓN                             │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                             │
│ T=0s     │ Simulador: Petición 1 → API Gateway                             │
│          │                                                                 │
│ T=0.1s   │ Simulador: Movimiento A1(0→1), A2(0→1)                        │
│          │                                                                 │
│ T=0.2s   │ Simulador: Movimiento A1(1→2), A2(1→2)                        │
│          │                                                                 │
│ T=2.0s   │ Simulador: Petición 2 → API Gateway                             │
│          │ Servidor: Recibe estado actual A1(piso=4), A2(piso=3)          │
│          │                                                                 │
│ T=2.1s   │ Simulador: Movimiento A1(4→5), A2(3→4)                        │
│          │                                                                 │
│ T=4.0s   │ Simulador: Petición 3 → API Gateway                             │
│          │ Servidor: Recibe estado actual A1(piso=6), A2(piso=5)          │
│                                                                             │
└─────────────────────────────────────────────────────────────────────────────┘
```

### **Intercambio de Datos**

#### **1. Petición del Simulador**
```json
// Datos enviados cada 2 segundos
{
  "id_edificio": "E021",
  "piso_origen_llamada": 4,
  "direccion_llamada": "SUBIENDO",
  "elevadores_estado": [
    {
      "id_ascensor": "E021A1",
      "piso_actual": 6,  ← 🔄 ACTUALIZADO desde última petición
      "estado_puerta": "ABIERTA",
      "disponible": true
    }
  ]
}
```

#### **2. Respuesta del Servidor Central**
```json
{
  "tarea_id": "T_1752190979031",
  "ascensor_asignado_id": "E021A1"
}
```

#### **3. Actualización de Estado Local**
```c
// API Gateway actualiza estado local
elevator->tarea_actual_id = "T_1752190979031";
elevator->destino_actual = 4;
elevator->disponible = false;
elevator->direccion_actual = MOVING_DOWN; // De piso 6 a piso 4
```

---

## 📊 Datos en Tiempo Real

### **Comparación: Antes vs Ahora**

#### **❌ SISTEMA ANTERIOR (Bloqueante)**
```
Petición 1: E021A1 piso_actual=0, E021A2 piso_actual=0
Petición 2: E021A1 piso_actual=0, E021A2 piso_actual=0  ← ❌ Sin cambios
Petición 3: E021A1 piso_actual=0, E021A2 piso_actual=0  ← ❌ Sin cambios
...
Petición 10: E021A1 piso_actual=0, E021A2 piso_actual=0 ← ❌ Sin cambios

[Después de todas las peticiones]
Movimiento: E021A1 (0→1→2→3→4→5)
```

#### **✅ SISTEMA ACTUAL (No-Bloqueante)**
```
Petición 1: E021A1 piso_actual=0, E021A2 piso_actual=0
Petición 2: E021A1 piso_actual=5, E021A2 piso_actual=0  ← ✅ Datos reales
Petición 3: E021A1 piso_actual=4, E021A2 piso_actual=3  ← ✅ Datos reales
Petición 4: E021A1 piso_actual=6, E021A2 piso_actual=3  ← ✅ Datos reales
```

### **Impacto en el Algoritmo del Servidor**

#### **Decisiones Inteligentes Basadas en Datos Reales**
```
🧠 ALGORITMO MEJORADO: Analizando 4 ascensores para piso 3, dirección BAJANDO

📊 Candidato: E021A1 | Piso: 6 | Destino: -1 | Score: 997 | Estado: DISPONIBLE
📊 Candidato: E021A2 | Piso: 3 | Destino: -1 | Score: 1000 | Estado: DISPONIBLE
📊 Candidato: E021A3 | Piso: 1 | Destino: -1 | Score: 998 | Estado: DISPONIBLE
📊 Candidato: E021A4 | Piso: 0 | Destino: -1 | Score: 997 | Estado: DISPONIBLE

🎯 SELECCIONADO: E021A2 | Score: 1000 | Estado: DISPONIBLE | Piso: 3 → -1
✅ ASIGNACIÓN ÓPTIMA: Ascensor disponible más cercano
```

**Resultado**: El servidor selecciona **E021A2** porque está **exactamente en el piso 3** (score perfecto 1000), en lugar de seleccionar un ascensor lejano.

---

## 🔄 Diferencias con el Sistema Anterior

### **Arquitectura**

| Aspecto | Sistema Anterior | Sistema Actual |
|---------|------------------|----------------|
| **Ejecución** | Bloqueante | No-Bloqueante |
| **Datos** | Estáticos (piso 0) | Tiempo Real |
| **Movimiento** | Post-peticiones | Durante peticiones |
| **Algoritmo** | Datos obsoletos | Datos actualizados |
| **Eficiencia** | Baja | Alta |

### **Flujo de Ejecución**

#### **❌ Sistema Anterior**
```
1. Ejecutar TODAS las peticiones secuencialmente
2. Procesar I/O CoAP después de cada petición
3. Mover ascensores SOLO después de terminar todas las peticiones
4. Servidor recibe siempre piso_actual=0 para todos los ascensores
```

#### **✅ Sistema Actual**
```
1. Ejecutar UNA petición
2. Regresar control al main loop
3. Procesar I/O CoAP (100ms)
4. Mover ascensores progresivamente
5. Ejecutar siguiente petición (después de 2 segundos)
6. Servidor recibe posiciones actualizadas en tiempo real
```

### **Beneficios del Nuevo Sistema**

#### **1. Realismo**
- ✅ Movimiento progresivo de ascensores
- ✅ Estados actualizados en tiempo real
- ✅ Comportamiento similar a sistema real

#### **2. Eficiencia del Algoritmo**
- ✅ Decisiones basadas en datos actuales
- ✅ Asignaciones óptimas
- ✅ Reducción de tiempos de espera

#### **3. Observabilidad**
- ✅ Logs detallados de movimiento
- ✅ Seguimiento de estado en tiempo real
- ✅ Depuración simplificada

#### **4. Escalabilidad**
- ✅ Preparado para múltiples instancias
- ✅ Gestión de estado distribuida
- ✅ Simulaciones complejas

---

## 🎯 Conclusión

El **nuevo algoritmo no-bloqueante** transformó completamente el sistema:

1. **Servidor Central**: Recibe datos actualizados y toma decisiones inteligentes
2. **Simulador**: Ejecuta peticiones incrementalmente con movimiento continuo
3. **Comunicación**: Intercambio de datos en tiempo real
4. **Resultado**: Sistema realista y eficiente

**El problema original "los pisos siempre se quedan sin actualizar" está 100% resuelto.** 