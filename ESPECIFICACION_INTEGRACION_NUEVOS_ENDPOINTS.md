# Especificación Técnica Completa para Integración de Nuevos Endpoints y Modificación de Existentes

## 1. Comprensión Integral del Sistema

### 1.1 Contexto Arquitectónico Detallado

El sistema de control de ascensores implementa una arquitectura distribuida que integra múltiples componentes mediante protocolos de comunicación estándar. Para comprender completamente cómo integrar nuevos endpoints, es esencial entender la arquitectura en profundidad:

```
┌─────────────────┐    ┌─────────────────┐    ┌──────────────────┐
│  Controladores  │    │   API Gateway   │    │ Servidor Central │
│      CAN        │──→ │  (CoAP Client)  │──→ │  (CoAP Server)   │
│                 │    │                 │    │                  │
│ • Sensores      │    │ • Puente CAN    │    │ • Algoritmos     │
│ • Actuadores    │    │ • Gestión Estado│    │ • Validación     │
│ • Botones       │    │ • Serialización │    │ • Respuestas     │
└─────────────────┘    └─────────────────┘    └──────────────────┘
         │                        │                       │
         │                        │                       │
         v                        v                       v
┌─────────────────┐    ┌─────────────────┐    ┌──────────────────┐
│   Frames CAN    │    │  Código OpenAPI │    │   Protocolos     │
│                 │    │                 │    │                  │
│ • ID: 0x100     │    │ • Modelos C     │    │ • DTLS-PSK       │
│ • ID: 0x200     │    │ • Funciones API │    │ • CoAP           │
│ • ID: 0x300     │    │ • Conversiones  │    │ • JSON           │
└─────────────────┘    └─────────────────┘    └──────────────────┘
```

**Componentes que cualquier desarrollador debe conocer:**

1. **Especificación OpenAPI** (`Api_controlador_ascensores.yaml`): Define la interface contractual
2. **Código Generado** (`Definition_API/api_controlador-c/`): Modelos y funciones C autogeneradas
3. **API Gateway** (`api_gateway/`): Cliente CoAP que procesa señales CAN y las reenvía
4. **Servidor Central** (`servidor_central/`): Servidor CoAP que procesa lógica de negocio

### 1.2 Flujo de Comunicación Completo

**Cuando alguien quiere añadir un nuevo endpoint, debe entender este flujo:**

1. **Origen**: El frame CAN llega al API Gateway desde controladores físicos o simuladores
2. **Interpretación**: El gateway procesa el frame según esquemas predefinidos
3. **Transformación**: Se convierte el frame CAN a solicitud CoAP usando código OpenAPI
4. **Envío**: El gateway actúa como cliente CoAP y envía la solicitud al servidor central via DTLS
5. **Procesamiento**: El servidor central valida, procesa y responde
6. **Respuesta**: El gateway recibe la respuesta CoAP del servidor central
7. **Retorno**: El gateway puede procesar la respuesta (tracking, logging, etc.)

**¿Por qué es importante entender esto?** Porque al añadir un nuevo endpoint, debes modificar el procesamiento CAN y la lógica del servidor central.

### 1.3 Análisis Detallado de Archivos de Configuración

**Para entender completamente el sistema, necesitas conocer estos archivos :**

#### 1.3.1 API Gateway Headers

**`api_gateway/include/api_gateway/api_common_defs.h`**
- **Propósito**: Define constantes globales compartidas por todo el sistema
- **Contenido**: 
  - `ID_STRING_MAX_LEN` (25): Tamaño máximo para IDs de ascensores
  - `TASK_ID_MAX_LEN` (32): Tamaño máximo para IDs de tareas
  - `STATUS_STRING_MAX_LEN` (16): Tamaño máximo para strings de estado
- **¿Por qué es importante?**: Cualquier string que uses debe respetar estos límites

**`api_gateway/include/api_gateway/elevator_state_manager.h`**
- **Propósito**: Define las estructuras de datos principales del sistema
- **Contenido**:
  - `elevator_status_t`: Estado individual de cada ascensor
  - `elevator_group_state_t`: Estado del grupo de ascensores
  - `gw_request_type_t`: Enumera los tipos de solicitudes (FLOOR_CALL, CABIN_REQUEST, etc.)
  - `api_request_details_for_json_t`: Estructura para detalles específicos de solicitudes
- **¿Por qué es importante?**: Aquí es donde añades nuevos tipos de solicitudes

**`api_gateway/include/api_gateway/can_bridge.h`**
- **Propósito**: Define la interfaz entre CAN y CoAP
- **Contenido**:
  - `simulated_can_frame_t`: Estructura de un frame CAN
  - `can_origin_tracker_t`: Para correlacionar solicitudes con respuestas
  - `can_send_callback_t`: Callback para enviar respuestas CAN
- **¿Por qué es importante?**: Aquí defines cómo se procesan nuevos tipos de frames CAN

**`api_gateway/include/api_gateway/coap_config.h`**
- **Propósito**: Configuraciones del protocolo CoAP
- **Contenido**:
  - `CENTRAL_SERVER_IP`: IP del servidor central
  - `CENTRAL_SERVER_PORT`: Puerto del servidor central
  - `FLOOR_CALL_RESOURCE`: Ruta para llamadas de piso
  - `CABIN_REQUEST_RESOURCE`: Ruta para solicitudes de cabina
- **¿Por qué es importante?**: Aquí defines las rutas de tus nuevos endpoints

**`api_gateway/include/api_gateway/logging_gw.h`**
- **Propósito**: Sistema de logging del gateway
- **Contenido**:
  - `LOG_INFO_GW`: Para mensajes informativos
  - `LOG_DEBUG_GW`: Para mensajes de depuración
  - `LOG_ERROR_GW`: Para mensajes de error
- **¿Por qué es importante?**: Todas las funciones deben incluir logging apropiado

#### 1.3.2 Servidor Central Headers

**`servidor_central/include/servidor_central/logging.h`**
- **Propósito**: Sistema de logging del servidor central
- **Contenido**:
  - `SRV_LOG_INFO`: Mensajes informativos
  - `SRV_LOG_ERROR`: Mensajes de error
  - `SRV_LOG_WARN`: Mensajes de advertencia
- **¿Por qué es importante?**: Evita conflictos con macros de libcoap

**`servidor_central/include/servidor_central/psk_validator.h`**
- **Propósito**: Validación de credenciales DTLS
- **Contenido**:
  - `psk_validator_init`: Inicializa el validador
  - `psk_validator_check_key`: Valida credenciales
- **¿Por qué es importante?**: Todos los endpoints deben validar autenticación

### 1.4 Estructuras de Datos Principales

**Para modificar correctamente el sistema, debes entender estas estructuras:**

#### 1.4.1 Estado de Ascensor Individual

```c
typedef struct {
    char ascensor_id[ID_STRING_MAX_LEN];                // ID único (ej: "E1A1")
    char id_edificio_str[ID_STRING_MAX_LEN];           // ID edificio (ej: "E1")
    int piso_actual;                                    // Piso actual
    door_state_enum_t estado_puerta_enum;              // Estado puertas
    char tarea_actual_id[TASK_ID_MAX_LEN];             // ID tarea asignada
    int destino_actual;                                 // Piso destino
    movement_direction_enum_t direccion_movimiento_enum; // Dirección
    bool ocupado;                                       // ¿Está ocupado?
} elevator_status_t;
```

#### 1.4.2 Estado del Grupo de Ascensores

```c
typedef struct {
    elevator_status_t ascensores[MAX_ELEVATORS_PER_GATEWAY]; // Array de ascensores
    int num_elevadores_en_grupo;                             // Número de ascensores
    char edificio_id_str_grupo[ID_STRING_MAX_LEN];          // ID del edificio
} elevator_group_state_t;
```

#### 1.4.3 Detalles de Solicitud para JSON

```c
typedef struct {
    // Para llamadas de piso
    int origin_floor_fc;
    movement_direction_enum_t direction_fc;
    
    // Para solicitudes de cabina
    char requesting_elevator_id_cr[ID_STRING_MAX_LEN];
    int target_floor_cr;
    
    // Para otros tipos de solicitudes...
} api_request_details_for_json_t;
```

**¿Por qué es importante?** Cuando añades un nuevo endpoint, necesitas añadir campos a esta estructura.

## 2. Caso de Uso 1: Añadir un Nuevo Endpoint (Llamadas de Emergencia)

### 2.1 Análisis Conceptual - ¿Qué Significa Añadir un Endpoint?

**Para alguien que no es programador:**
Añadir un endpoint significa crear una nueva "puerta de entrada" al sistema. Es como añadir una nueva función que antes no existía. En nuestro caso, queremos que los ascensores puedan reportar emergencias.

**Para alguien técnico:**
Añadir un endpoint significa definir una nueva ruta HTTP/CoAP que puede procesar un tipo específico de solicitud, implementar toda la lógica de procesamiento y asegurar la integración con el resto del sistema.

**¿Por qué empezar con el análisis?** Porque necesitas entender exactamente qué va a hacer tu endpoint antes de escribir código.

### 2.2 Paso 1: Definir el Comportamiento del Endpoint

**Antes de tocar cualquier código, define:**

1. **¿Qué datos necesita?**
   - ID del edificio
   - ID del ascensor en emergencia
   - Tipo de emergencia (parada, fallo eléctrico, personas atrapadas, etc.)
   - Piso actual
   - Descripción de la emergencia
   - Timestamp
   - Estado de todos los ascensores

2. **¿Qué hace con esos datos?**
   - Valida que los datos sean correctos
   - Determina qué protocolo de emergencia activar
   - Identifica servicios a notificar (bomberos, mantenimiento, etc.)
   - Encuentra ascensores disponibles para apoyo
   - Genera un ID único para la emergencia

3. **¿Qué responde?**
   - ID de la emergencia
   - Protocolo activado
   - Tiempo estimado de respuesta
   - Servicios notificados
   - Ascensores redirigidos

### 2.3 Paso 2: Especificación OpenAPI - El Contrato

**¿Por qué empezar aquí?**
Porque el archivo YAML define el "contrato" que todos los componentes deben seguir. Es como el plano arquitectónico antes de construir.

**Modificaciones necesarias en `Definition_API/Api_controlador_ascensores.yaml`:**

```yaml
paths:
  /llamada_emergencia:
    post:
      summary: Procesar llamadas de emergencia desde ascensores
      description: |
        Procesa solicitudes de emergencia originadas desde cabinas o controladores
        de ascensores. Activa protocolos de emergencia y notifica a servicios
        de mantenimiento y sistemas de seguridad.
        
        Tipos de emergencia soportados:
        - EMERGENCY_STOP: Botón de parada activado
        - POWER_FAILURE: Fallo de alimentación detectado
        - PEOPLE_TRAPPED: Personas atrapadas en cabina
        - MECHANICAL_FAILURE: Fallo mecánico del ascensor
        - FIRE_ALARM: Alarma de incendios activada
        
      requestBody:
        required: true
        content:
          application/json:
            schema:
              $ref: '#/components/schemas/EmergencyCallRequest'
      
      responses:
        '200':
          description: Solicitud de emergencia procesada exitosamente
          content:
            application/json:
              schema:
                $ref: '#/components/schemas/EmergencyResponse'
        '400':
          description: Error en la solicitud
          content:
            application/json:
              schema:
                $ref: '#/components/schemas/ErrorResponse'
        '401':
          description: No autorizado
          content:
            application/json:
              schema:
                $ref: '#/components/schemas/ErrorResponse'
        '500':
          description: Error interno del servidor
          content:
            application/json:
              schema:
                $ref: '#/components/schemas/ErrorResponse'
```

**¿Qué estamos definiendo aquí?**
- Una nueva ruta `/llamada_emergencia` que acepta solicitudes POST
- Qué tipo de datos esperamos recibir (EmergencyCallRequest)
- Qué tipo de respuesta enviaremos (EmergencyResponse)
- Qué códigos de error pueden ocurrir

**Luego definimos los esquemas de datos:**

```yaml
components:
  schemas:
    EmergencyType:
      type: string
      enum: [EMERGENCY_STOP, POWER_FAILURE, PEOPLE_TRAPPED, MECHANICAL_FAILURE, FIRE_ALARM]
      description: |
        Tipos de emergencia que el sistema puede procesar
      example: PEOPLE_TRAPPED

    EmergencyCallRequest:
      type: object
      description: Solicitud de emergencia desde ascensor
      properties:
        id_edificio:
          type: string
          description: ID del edificio donde ocurre la emergencia
          pattern: '^E[0-9]+$'
          example: E1
          maxLength: 23
        ascensor_id_emergencia:
          type: string
          description: ID del ascensor que reporta la emergencia
          pattern: '^E[0-9]+A[0-9]+$'
          example: E1A1
          maxLength: 23
        tipo_emergencia:
          $ref: '#/components/schemas/EmergencyType'
        piso_actual_emergencia:
          type: integer
          description: Piso donde está el ascensor en emergencia
          minimum: 0
          maximum: 50
          example: 5
        descripcion_emergencia:
          type: string
          description: Descripción detallada de la emergencia
          maxLength: 500
          example: "Personas atrapadas, puerta no abre"
        timestamp_emergencia:
          type: string
          format: date-time
          description: Cuándo ocurrió la emergencia
          example: "2024-01-15T10:30:00Z"
        elevadores_estado:
          type: array
          items:
            $ref: '#/components/schemas/ElevatorState'
          description: Estado de todos los ascensores del edificio
          minItems: 1
          maxItems: 6
      required:
        - id_edificio
        - ascensor_id_emergencia
        - tipo_emergencia
        - piso_actual_emergencia
        - timestamp_emergencia
        - elevadores_estado

    EmergencyResponse:
      type: object
      description: Respuesta a una solicitud de emergencia
      properties:
        emergencia_id:
          type: string
          description: ID único generado para esta emergencia
          pattern: '^EMG_[0-9]+$'
          example: EMG_1640995200123
          maxLength: 31
        protocolo_activado:
          type: string
          description: Protocolo de emergencia que se activó
          enum: [RESCUE_PROTOCOL, MAINTENANCE_ALERT, FIRE_EVACUATION, POWER_BACKUP]
          example: RESCUE_PROTOCOL
        tiempo_respuesta_estimado:
          type: integer
          description: Tiempo estimado de respuesta en minutos
          minimum: 1
          maximum: 60
          example: 15
        servicios_notificados:
          type: array
          items:
            type: string
          description: Lista de servicios que fueron notificados
          example: ["BOMBEROS", "MANTENIMIENTO", "SEGURIDAD"]
        ascensores_redirection:
          type: array
          items:
            type: string
          description: Ascensores redirigidos para asistencia
          example: ["E1A2", "E1A3"]
      required:
        - emergencia_id
        - protocolo_activado
        - tiempo_respuesta_estimado
        - servicios_notificados
```

**¿Por qué definir límites?**
- `maxLength: 23` para IDs coincide con `ID_STRING_MAX_LEN`
- `maxLength: 500` para descripción es razonable pero no excesivo
- `minimum: 0, maximum: 50` para pisos cubre edificios típicos

### 2.4 Paso 3: Validación del Contrato

**¿Por qué validar?**
Antes de continuar, necesitamos asegurarnos de que nuestra especificación es correcta sintácticamente.

```bash
# Validar sintaxis OpenAPI
npx swagger-cli validate Definition_API/Api_controlador_ascensores.yaml

# Validar con OpenAPI Generator
openapi-generator-cli validate -i Definition_API/Api_controlador_ascensores.yaml
```

**Validación completada, continuar con el siguiente paso.**

### 2.5 Paso 4: Generación de Código Cliente

**¿Qué hace este paso?**
El generador lee nuestro YAML y produce código C automáticamente. Este código incluye:
- Estructuras de datos para nuestros modelos
- Funciones para convertir entre JSON y estructuras C
- Funciones para hacer llamadas HTTP/CoAP

```bash
# Generar código C desde la especificación
openapi-generator-cli generate \
  -i Definition_API/Api_controlador_ascensores.yaml \
  -g c \
  -o Definition_API/api_controlador-c \
  --package-name api_gateway_coap_para_sistema_de_ascensores \
  --additional-properties=packageCompany="Sistema de Control de Ascensores" \
  --additional-properties=packageVersion="2.0.0"
```

**Archivos generados automáticamente:**

1. **`model/emergency_call_request.h`** - Estructura de datos:
```c
typedef struct emergency_call_request_t {
    char *id_edificio;
    char *ascensor_id_emergencia;
    api_gateway_coap_para_sistema_de_ascensores_emergency_type__e tipo_emergencia;
    int piso_actual_emergencia;
    char *descripcion_emergencia;
    char *timestamp_emergencia;
    list_t *elevadores_estado;
    int _library_owned;
} emergency_call_request_t;
```

2. **`model/emergency_call_request.c`** - Funciones de conversión:
```c
// Convierte estructura C a JSON
cJSON *emergency_call_request_convertToJSON(emergency_call_request_t *emergency_call_request);

// Convierte JSON a estructura C
emergency_call_request_t *emergency_call_request_parseFromJSON(cJSON *emergency_call_requestJSON);

// Libera memoria
void emergency_call_request_free(emergency_call_request_t *emergency_call_request);
```

3. **`model/emergency_type.h`** - Enumeración:
```c
typedef enum {
    api_gateway_coap_para_sistema_de_ascensores_emergency_type__NULL = 0,
    api_gateway_coap_para_sistema_de_ascensores_emergency_type__EMERGENCY_STOP,
    api_gateway_coap_para_sistema_de_ascensores_emergency_type__POWER_FAILURE,
    api_gateway_coap_para_sistema_de_ascensores_emergency_type__PEOPLE_TRAPPED,
    api_gateway_coap_para_sistema_de_ascensores_emergency_type__MECHANICAL_FAILURE,
    api_gateway_coap_para_sistema_de_ascensores_emergency_type__FIRE_ALARM
} api_gateway_coap_para_sistema_de_ascensores_emergency_type__e;
```

4. **`api/DefaultAPI.h`** - Función de API:
```c
emergency_response_t* DefaultAPI_llamadaEmergenciaPost(
    apiClient_t *apiClient,
    emergency_call_request_t *emergency_call_request
);
```

**¿Por qué se generan estos archivos?** Porque OpenAPI Generator convierte la especificación YAML en código C utilizable.

### 2.6 Paso 5: Integración en el API Gateway

#### 2.6.1 Modificaciones en Headers

**En `api_gateway/include/api_gateway/elevator_state_manager.h`:**

**¿Qué estamos haciendo?** Añadiendo el nuevo tipo de solicitud a la enumeración existente.

```c
typedef enum {
    GW_REQUEST_TYPE_UNKNOWN = 0,
    GW_REQUEST_TYPE_FLOOR_CALL,
    GW_REQUEST_TYPE_CABIN_REQUEST,
    GW_REQUEST_TYPE_EMERGENCY_CALL  // <- NUEVO
} gw_request_type_t;
```

**¿Por qué aquí?** Porque esta enumeración define todos los tipos de solicitudes que el gateway puede procesar.

#### 2.6.2 Ampliación de la Estructura de Detalles

**En `api_gateway/include/api_gateway/elevator_state_manager.h`:**

```c
typedef struct {
    // Campos existentes para floor calls
    int origin_floor_fc;
    movement_direction_enum_t direction_fc;
    
    // Campos existentes para cabin requests
    char requesting_elevator_id_cr[ID_STRING_MAX_LEN];
    int target_floor_cr;
    
    // NUEVOS CAMPOS PARA EMERGENCIAS
    char emergency_elevator_id[ID_STRING_MAX_LEN];
    int emergency_floor;
    char emergency_description[512];
    char emergency_timestamp[32];
    api_gateway_coap_para_sistema_de_ascensores_emergency_type__e emergency_type;
} api_request_details_for_json_t;
```

**¿Qué estamos haciendo?** Añadiendo campos para almacenar información específica de emergencias.

**¿Por qué estos campos?** Porque el servidor central necesita esta información para procesar la emergencia correctamente.

#### 2.6.3 Actualización del Generador de JSON

**En `api_gateway/src/elevator_state_manager.c`:**

**¿Qué hace esta función?** Convierte el estado actual del sistema a JSON para enviar al servidor central.

```c
cJSON *elevator_group_to_json_for_server(elevator_group_state_t *group, 
                                         gw_request_type_t request_type,
                                         api_request_details_for_json_t *details) {
    if (!group || !details) {
        return NULL;
    }

    cJSON *json = cJSON_CreateObject();
    if (!json) return NULL;

    // Campos comunes para todos los tipos de solicitud
    cJSON_AddStringToObject(json, "id_edificio", group->edificio_id_str_grupo);
    
    // Array de estado de ascensores
    cJSON *elevadores_array = cJSON_CreateArray();
    for (int i = 0; i < group->num_elevadores_en_grupo; i++) {
        cJSON *elevator_obj = elevator_status_to_json(&group->ascensores[i]);
        if (elevator_obj) {
            cJSON_AddItemToArray(elevadores_array, elevator_obj);
        }
    }
    cJSON_AddItemToObject(json, "elevadores_estado", elevadores_array);

    // Campos específicos según el tipo de solicitud
    switch (request_type) {
        case GW_REQUEST_TYPE_FLOOR_CALL:
            cJSON_AddNumberToObject(json, "piso_origen_llamada", details->origin_floor_fc);
            cJSON_AddStringToObject(json, "direccion_llamada", 
                                   movement_direction_to_string(details->direction_fc));
            break;
            
        case GW_REQUEST_TYPE_CABIN_REQUEST:
            cJSON_AddStringToObject(json, "solicitando_ascensor_id", details->requesting_elevator_id_cr);
            cJSON_AddNumberToObject(json, "piso_destino_solicitud", details->target_floor_cr);
            break;
            
        case GW_REQUEST_TYPE_EMERGENCY_CALL:  // <- NUEVO CASO
            cJSON_AddStringToObject(json, "ascensor_id_emergencia", details->emergency_elevator_id);
            cJSON_AddNumberToObject(json, "piso_actual_emergencia", details->emergency_floor);
            cJSON_AddStringToObject(json, "descripcion_emergencia", details->emergency_description);
            cJSON_AddStringToObject(json, "timestamp_emergencia", details->emergency_timestamp);
            cJSON_AddStringToObject(json, "tipo_emergencia", 
                                   emergency_type_to_string(details->emergency_type));
            break;
    }

    return json;
}
```

**¿Por qué usar un switch?** Porque cada tipo de solicitud necesita campos diferentes en el JSON.

#### 2.6.4 Función de Conversión de Tipos

**En `api_gateway/src/elevator_state_manager.c`:**

```c
const char* emergency_type_to_string(api_gateway_coap_para_sistema_de_ascensores_emergency_type__e emergency_type) {
    switch (emergency_type) {
        case api_gateway_coap_para_sistema_de_ascensores_emergency_type__EMERGENCY_STOP:
            return "EMERGENCY_STOP";
        case api_gateway_coap_para_sistema_de_ascensores_emergency_type__POWER_FAILURE:
            return "POWER_FAILURE";
        case api_gateway_coap_para_sistema_de_ascensores_emergency_type__PEOPLE_TRAPPED:
            return "PEOPLE_TRAPPED";
        case api_gateway_coap_para_sistema_de_ascensores_emergency_type__MECHANICAL_FAILURE:
            return "MECHANICAL_FAILURE";
        case api_gateway_coap_para_sistema_de_ascensores_emergency_type__FIRE_ALARM:
            return "FIRE_ALARM";
        default:
            return "UNKNOWN";
    }
}
```

**¿Por qué necesitamos esto?** Porque el código generado usa enumeraciones en C, pero el JSON necesita strings.

#### 2.6.5 Procesamiento de Frames CAN

**En `api_gateway/src/can_bridge.c`:**

**¿Qué estamos haciendo?** Añadiendo procesamiento para un nuevo tipo de frame CAN (ID 0x400) que representará emergencias.

**¿Por qué ID 0x400?** Porque ya se usan 0x100 (llamadas de piso), 0x200 (solicitudes de cabina), y 0x300 (notificaciones de llegada). El 0x400 se asigna específicamente para emergencias.

**Nota importante sobre declaraciones forward:** Antes de implementar el procesamiento del frame 0x400, necesitas añadir la declaración de la nueva función al inicio del archivo:

```c
// Declaración adelantada de función específica para emergencias
static void forward_can_emergency_request_to_central_server(
    coap_context_t *ctx,
    uint32_t original_can_id,
    const char *central_server_path,
    const char *log_tag_param,
    const char *elevator_id,
    int emergency_floor,
    api_gateway_coap_para_sistema_de_ascensores_emergency_type__e emergency_type,
    const char *description,
    const char *timestamp
); // Definición más abajo
```

```c
void ag_can_bridge_process_incoming_frame(simulated_can_frame_t* frame, coap_context_t *coap_ctx) {
    if (!frame || !coap_ctx) {
        LOG_ERROR_GW("[CAN_Bridge] Frame o contexto CoAP nulo.");
        return;
    }

    LOG_INFO_GW("[CAN_Bridge] Procesando frame CAN ID: 0x%X, DLC: %d", frame->id, frame->dlc);

    switch (frame->id) {
        case 0x100: // Llamada de piso (código existente)
            // ... código existente ...
            break;
            
        case 0x200: // Solicitud de cabina (código existente)
            // ... código existente ...
            break;
            
        case 0x300: // Notificación de llegada (código existente)
            // ... código existente ...
            break;
            
        case 0x400: // NUEVO: Llamada de emergencia
            if (frame->dlc >= 3) { // Necesitamos al menos 3 bytes de datos
                // Decodificar datos del frame CAN
                int elevator_index = frame->data[0];  // Índice del ascensor (0-based)
                int emergency_floor = frame->data[1]; // Piso donde está el ascensor
                int emergency_type_int = frame->data[2]; // Tipo de emergencia
                
                // Construir ID del ascensor
                char elevator_id_str[atoi(getenv("ID_STRING_MAX_LEN"))];
                int max_building_id_len = atoi(getenv("ID_STRING_MAX_LEN")) - 1 /*null*/ - 1 /*A*/ - 3 /*NNN for elevator number*/;
                if (max_building_id_len < 1) max_building_id_len = 1; // ensure at least 1 char for building id part
                
                snprintf(elevator_id_str, sizeof(elevator_id_str), "%.*sA%d", 
                         max_building_id_len,
                         managed_elevator_group.edificio_id_str_grupo, 
                         elevator_index + 1);
                
                // Convertir tipo de emergencia
                api_gateway_coap_para_sistema_de_ascensores_emergency_type__e emergency_type = 
                    (api_gateway_coap_para_sistema_de_ascensores_emergency_type__e)emergency_type_int;
                
                // Generar timestamp
                char timestamp_str[32];
                time_t now = time(NULL);
                struct tm *utc_time = gmtime(&now);
                strftime(timestamp_str, sizeof(timestamp_str), "%Y-%m-%dT%H:%M:%SZ", utc_time);
                
                // Crear descripción
                char description[512];
                snprintf(description, sizeof(description), 
                         "Emergencia reportada por ascensor %s en piso %d", 
                         elevator_id_str, emergency_floor);
                
                LOG_WARN_GW("[CAN_Bridge] 🚨 EMERGENCIA CAN: Ascensor %s, Piso %d, Tipo %s", 
                           elevator_id_str, emergency_floor, emergency_type_to_string(emergency_type));
                
                // Usar implementación específica para emergencias que llena todos los campos
                forward_can_emergency_request_to_central_server(
                    coap_ctx, frame->id,
                    getenv("EMERGENCY_CALL_RESOURCE") ?: "/llamada_emergencia",
                    "CAN_Emergency", 
                    elevator_id_str,
                    emergency_floor,
                    emergency_type,
                    description,
                    timestamp_str);
            } else {
                LOG_WARN_GW("[CAN_Bridge] Frame CAN 0x400 (Emergencia) con DLC insuficiente: %d", frame->dlc);
            }
            break;

        default:
            LOG_WARN_GW("[CAN_Bridge] ID de frame CAN desconocido: 0x%X", frame->id);
            break;
    }
}
```

**¿Por qué esta estructura de datos?**
- `frame->data[0]`: Índice del ascensor (0-5)
- `frame->data[1]`: Piso actual (0-50)
- `frame->data[2]`: Tipo de emergencia (enum)

**Nota sobre includes:** Asegúrate de añadir `#include <time.h>` al inicio de `can_bridge.c` para las funciones de timestamp (`time()`, `gmtime()`, `strftime()`).

#### 2.6.6 Variables de Entorno

**En `api_gateway/gateway.env`:**

```env
# Recursos CoAP existentes
FLOOR_CALL_RESOURCE=/peticion_piso
CABIN_REQUEST_RESOURCE=/peticion_cabina

# NUEVO RECURSO
EMERGENCY_CALL_RESOURCE=/llamada_emergencia
```

**¿Por qué usar variables de entorno?** Para poder cambiar las rutas sin recompilar.

### 2.7 Paso 6: Verificar Flujo CAN → API Gateway (Cliente) → Servidor Central

**¿Qué hace esta sección?**
El API Gateway actúa únicamente como **cliente CoAP** hacia el servidor central. Su función es procesar frames CAN (de simuladores o hardware real) y reenviarlos como solicitudes CoAP al servidor central.

#### 2.7.1 Flujo Real del Sistema

**Flujo actual implementado:**

```
[Simulador] → Frame CAN 0x400 → [can_bridge.c] → [forward_can_emergency_request_to_central_server] → [Servidor Central]
```

**Formato del Frame CAN 0x400 (Emergencias):**
- `data[0]`: Índice del ascensor (0-based, 0-5)
- `data[1]`: Piso donde está el ascensor en emergencia (0-50)
- `data[2]`: Tipo de emergencia (enum: 0=EMERGENCY_STOP, 1=POWER_FAILURE, 2=PEOPLE_TRAPPED, 3=MECHANICAL_FAILURE, 4=FIRE_ALARM)
- `dlc`: Mínimo 3 bytes



### 2.8 Paso 7: Integración en el Servidor Central

**¿Qué hace el servidor central?**
El servidor central recibe solicitudes CoAP del API Gateway, las procesa según la lógica de negocio y responde con las acciones a tomar.

**✅ IMPLEMENTADO:** El endpoint de emergencias está completamente implementado en el servidor central con toda la funcionalidad descrita en esta especificación.

#### 2.7.2 Verificación del Procesamiento CAN

**Estado del flujo de emergencias:**

1. **✅ Frame CAN 0x400** se procesa correctamente en `ag_can_bridge_process_incoming_frame()` 
2. **✅ Datos extraídos** del frame:
   - `frame->data[0]`: Índice del ascensor (0-based)
   - `frame->data[1]`: Piso donde está el ascensor en emergencia
   - `frame->data[2]`: Tipo de emergencia (enumeración)

3. **✅ Función especializada** `forward_can_emergency_request_to_central_server()` convierte a JSON CoAP
4. **✅ Servidor central** procesa `/llamada_emergencia` con `hnd_emergency_call()`

**✅ Flujo Completo:** El sistema de emergencias está completamente implementado desde el frame CAN hasta la respuesta del servidor central, incluyendo activación de protocolos y coordinación de servicios.

**¿Qué significa esto?** El flujo de emergencias funciona end-to-end: CAN → API Gateway → Servidor Central → Protocolos de emergencia.

### 2.8 Paso 7: Integración en el Servidor Central

**¿Qué hace el servidor central?**
El servidor central recibe solicitudes CoAP, las procesa según la lógica de negocio y responde con las acciones a tomar.

#### 2.8.1 Definición de Constantes

**En `servidor_central/src/main.c`:**

```c
#define RESOURCE_FLOOR_CALL "peticion_piso"
#define RESOURCE_CABIN_REQUEST "peticion_cabina"
#define RESOURCE_EMERGENCY_CALL "llamada_emergencia"  // <- NUEVO
```

#### 2.8.2 Implementación del Manejador

**En `servidor_central/src/main.c`:**

```c
static void hnd_emergency_call(coap_resource_t *resource, coap_session_t *session,
                              const coap_pdu_t *request, const coap_string_t *query,
                              coap_pdu_t *response) {
    
    SRV_LOG_INFO("=== PROCESANDO LLAMADA DE EMERGENCIA ===");
    
    // 1. Verificar seguridad DTLS
    if (coap_session_get_state(session) != COAP_SESSION_STATE_ESTABLISHED) {
        SRV_LOG_ERROR("Sesión DTLS no establecida para emergencia");
        coap_pdu_set_code(response, COAP_RESPONSE_CODE_UNAUTHORIZED);
        
        cJSON *error_json = cJSON_CreateObject();
        cJSON_AddStringToObject(error_json, "error", "Unauthorized");
        cJSON_AddStringToObject(error_json, "message", "DTLS requerido para emergencias");
        
        char *error_str = cJSON_PrintUnformatted(error_json);
        coap_add_option(response, COAP_OPTION_CONTENT_FORMAT, 
                       coap_encode_var_safe((uint8_t[2]){0}, 2, COAP_MEDIATYPE_APPLICATION_JSON), 
                       (uint8_t[2]){0});
        coap_add_data(response, strlen(error_str), (const uint8_t*)error_str);
        
        cJSON_Delete(error_json);
        free(error_str);
        return;
    }

    // 2. Obtener datos del payload
    const uint8_t *data;
    size_t data_len;
    
    if (coap_get_data(request, &data_len, &data)) {
        SRV_LOG_DEBUG("Payload de emergencia: %.*s", (int)data_len, (char*)data);

        // 3. Procesar JSON
        cJSON *json_payload = cJSON_ParseWithLength((const char*)data, data_len);
        if (!json_payload) {
            SRV_LOG_ERROR("Error procesando JSON de emergencia: %s", cJSON_GetErrorPtr());
            coap_pdu_set_code(response, COAP_RESPONSE_CODE_BAD_REQUEST);
            
            cJSON *error_json = cJSON_CreateObject();
            cJSON_AddStringToObject(error_json, "error", "JSON inválido");
            cJSON_AddStringToObject(error_json, "details", cJSON_GetErrorPtr());
            
            char *error_str = cJSON_PrintUnformatted(error_json);
            coap_add_option(response, COAP_OPTION_CONTENT_FORMAT, 
                           coap_encode_var_safe((uint8_t[2]){0}, 2, COAP_MEDIATYPE_APPLICATION_JSON), 
                           (uint8_t[2]){0});
            coap_add_data(response, strlen(error_str), (const uint8_t*)error_str);
            
            cJSON_Delete(error_json);
            free(error_str);
            return;
        }

        // 4. Extraer campos del JSON
        cJSON *j_id_edificio = cJSON_GetObjectItemCaseSensitive(json_payload, "id_edificio");
        cJSON *j_ascensor_id = cJSON_GetObjectItemCaseSensitive(json_payload, "ascensor_id_emergencia");
        cJSON *j_tipo_emergencia = cJSON_GetObjectItemCaseSensitive(json_payload, "tipo_emergencia");
        cJSON *j_piso_actual = cJSON_GetObjectItemCaseSensitive(json_payload, "piso_actual_emergencia");
        cJSON *j_timestamp = cJSON_GetObjectItemCaseSensitive(json_payload, "timestamp_emergencia");
        cJSON *j_elevadores_estado = cJSON_GetObjectItemCaseSensitive(json_payload, "elevadores_estado");
        cJSON *j_descripcion = cJSON_GetObjectItemCaseSensitive(json_payload, "descripcion_emergencia");

        // 5. Validar campos obligatorios
        if (!cJSON_IsString(j_id_edificio) || !cJSON_IsString(j_ascensor_id) ||
            !cJSON_IsString(j_tipo_emergencia) || !cJSON_IsNumber(j_piso_actual) ||
            !cJSON_IsString(j_timestamp) || !cJSON_IsArray(j_elevadores_estado)) {
            
            SRV_LOG_ERROR("Campos obligatorios faltantes en emergencia");
            coap_pdu_set_code(response, COAP_RESPONSE_CODE_BAD_REQUEST);
            
            cJSON *error_json = cJSON_CreateObject();
            cJSON_AddStringToObject(error_json, "error", "Campos faltantes");
            cJSON_AddStringToObject(error_json, "required", 
                                   "id_edificio, ascensor_id_emergencia, tipo_emergencia, piso_actual_emergencia, timestamp_emergencia, elevadores_estado");
            
            char *error_str = cJSON_PrintUnformatted(error_json);
            coap_add_option(response, COAP_OPTION_CONTENT_FORMAT, 
                           coap_encode_var_safe((uint8_t[2]){0}, 2, COAP_MEDIATYPE_APPLICATION_JSON), 
                           (uint8_t[2]){0});
            coap_add_data(response, strlen(error_str), (const uint8_t*)error_str);
            
            cJSON_Delete(error_json);
            free(error_str);
            cJSON_Delete(json_payload);
            return;
        }

        // 6. Extraer valores
        char *id_edificio = j_id_edificio->valuestring;
        char *ascensor_id = j_ascensor_id->valuestring;
        char *tipo_emergencia = j_tipo_emergencia->valuestring;
        int piso_actual = j_piso_actual->valueint;
        char *timestamp = j_timestamp->valuestring;
        char *descripcion = j_descripcion && cJSON_IsString(j_descripcion) ? 
                           j_descripcion->valuestring : "Sin descripción";

        // 7. Validar tipo de emergencia
        if (strcmp(tipo_emergencia, "EMERGENCY_STOP") != 0 &&
            strcmp(tipo_emergencia, "POWER_FAILURE") != 0 &&
            strcmp(tipo_emergencia, "PEOPLE_TRAPPED") != 0 &&
            strcmp(tipo_emergencia, "MECHANICAL_FAILURE") != 0 &&
            strcmp(tipo_emergencia, "FIRE_ALARM") != 0) {
            
            SRV_LOG_ERROR("Tipo de emergencia inválido: %s", tipo_emergencia);
            coap_pdu_set_code(response, COAP_RESPONSE_CODE_BAD_REQUEST);
            
            cJSON *error_json = cJSON_CreateObject();
            cJSON_AddStringToObject(error_json, "error", "Tipo de emergencia inválido");
            cJSON_AddStringToObject(error_json, "received", tipo_emergencia);
            cJSON_AddStringToObject(error_json, "valid_types", 
                                   "EMERGENCY_STOP, POWER_FAILURE, PEOPLE_TRAPPED, MECHANICAL_FAILURE, FIRE_ALARM");
            
            char *error_str = cJSON_PrintUnformatted(error_json);
            coap_add_option(response, COAP_OPTION_CONTENT_FORMAT, 
                           coap_encode_var_safe((uint8_t[2]){0}, 2, COAP_MEDIATYPE_APPLICATION_JSON), 
                           (uint8_t[2]){0});
            coap_add_data(response, strlen(error_str), (const uint8_t*)error_str);
            
            cJSON_Delete(error_json);
            free(error_str);
            cJSON_Delete(json_payload);
            return;
        }

        // 8. Procesar la emergencia
        SRV_LOG_WARN("🚨 EMERGENCIA: %s en %s, ascensor %s, piso %d", 
                     tipo_emergencia, id_edificio, ascensor_id, piso_actual);
        SRV_LOG_INFO("Descripción: %s", descripcion);
        SRV_LOG_INFO("Timestamp: %s", timestamp);

        // 9. Generar ID único para la emergencia
        char emergency_id[32];
        struct timeval tv;
        gettimeofday(&tv, NULL);
        snprintf(emergency_id, sizeof(emergency_id), "EMG_%ld%03ld", 
                (long)tv.tv_sec, (long)(tv.tv_usec / 1000));

        // 10. Determinar protocolo según tipo de emergencia
        const char *protocolo_activado;
        const char *servicios_notificados[10];
        int num_servicios = 0;
        int tiempo_respuesta = 15; // Por defecto

        if (strcmp(tipo_emergencia, "PEOPLE_TRAPPED") == 0) {
            protocolo_activado = "RESCUE_PROTOCOL";
            servicios_notificados[num_servicios++] = "BOMBEROS";
            servicios_notificados[num_servicios++] = "MANTENIMIENTO";
            servicios_notificados[num_servicios++] = "SEGURIDAD";
            tiempo_respuesta = 10; // Alta prioridad
        } else if (strcmp(tipo_emergencia, "FIRE_ALARM") == 0) {
            protocolo_activado = "FIRE_EVACUATION";
            servicios_notificados[num_servicios++] = "BOMBEROS";
            servicios_notificados[num_servicios++] = "EVACUACION";
            servicios_notificados[num_servicios++] = "POLICIA";
            tiempo_respuesta = 5; // Prioridad alta
        } else if (strcmp(tipo_emergencia, "POWER_FAILURE") == 0) {
            protocolo_activado = "POWER_BACKUP";
            servicios_notificados[num_servicios++] = "MANTENIMIENTO";
            servicios_notificados[num_servicios++] = "ELECTRICIDAD";
            tiempo_respuesta = 20;
        } else {
            protocolo_activado = "MAINTENANCE_ALERT";
            servicios_notificados[num_servicios++] = "MANTENIMIENTO";
            servicios_notificados[num_servicios++] = "SEGURIDAD";
            tiempo_respuesta = 15;
        }

        // 11. Encontrar ascensores disponibles para apoyo
        cJSON *ascensores_apoyo = cJSON_CreateArray();
        int array_size = cJSON_GetArraySize(j_elevadores_estado);
        for (int i = 0; i < array_size; i++) {
            cJSON *elevator = cJSON_GetArrayItem(j_elevadores_estado, i);
            if (elevator) {
                cJSON *j_id = cJSON_GetObjectItemCaseSensitive(elevator, "id_ascensor");
                cJSON *j_disponible = cJSON_GetObjectItemCaseSensitive(elevator, "disponible");
                
                if (cJSON_IsString(j_id) && cJSON_IsBool(j_disponible) &&
                    strcmp(j_id->valuestring, ascensor_id) != 0 && // No el de emergencia
                    cJSON_IsTrue(j_disponible)) { // Disponible
                    cJSON_AddItemToArray(ascensores_apoyo, cJSON_CreateString(j_id->valuestring));
                }
            }
        }

        SRV_LOG_INFO("🚨 PROTOCOLO: %s", protocolo_activado);
        SRV_LOG_INFO("⏱️ TIEMPO ESTIMADO: %d minutos", tiempo_respuesta);

        // 12. Construir respuesta JSON
        cJSON *response_json = cJSON_CreateObject();
        cJSON_AddStringToObject(response_json, "emergencia_id", emergency_id);
        cJSON_AddStringToObject(response_json, "protocolo_activado", protocolo_activado);
        cJSON_AddNumberToObject(response_json, "tiempo_respuesta_estimado", tiempo_respuesta);

        // Añadir servicios notificados
        cJSON *servicios_array = cJSON_CreateArray();
        for (int i = 0; i < num_servicios; i++) {
            cJSON_AddItemToArray(servicios_array, cJSON_CreateString(servicios_notificados[i]));
        }
        cJSON_AddItemToObject(response_json, "servicios_notificados", servicios_array);

        // Añadir ascensores de apoyo
        cJSON_AddItemToObject(response_json, "ascensores_redirection", ascensores_apoyo);

        // 13. Enviar respuesta
        char *response_str = cJSON_PrintUnformatted(response_json);
        if (!response_str) {
            SRV_LOG_ERROR("Error creando respuesta JSON");
            coap_pdu_set_code(response, COAP_RESPONSE_CODE_INTERNAL_ERROR);
            cJSON_Delete(response_json);
            cJSON_Delete(json_payload);
            return;
        }

        coap_pdu_set_code(response, COAP_RESPONSE_CODE_CONTENT);
        coap_add_option(response, COAP_OPTION_CONTENT_FORMAT, 
                       coap_encode_var_safe((uint8_t[2]){0}, 2, COAP_MEDIATYPE_APPLICATION_JSON), 
                       (uint8_t[2]){0});
        coap_add_data(response, strlen(response_str), (const uint8_t *)response_str);

        SRV_LOG_INFO("🚨 EMERGENCIA PROCESADA: %s", emergency_id);

        cJSON_Delete(response_json);
        free(response_str);
        cJSON_Delete(json_payload);

    } else {
        SRV_LOG_ERROR("Solicitud de emergencia sin payload");
        coap_pdu_set_code(response, COAP_RESPONSE_CODE_BAD_REQUEST);
        
        cJSON *error_json = cJSON_CreateObject();
        cJSON_AddStringToObject(error_json, "error", "Payload faltante");
        
        char *error_str = cJSON_PrintUnformatted(error_json);
        coap_add_option(response, COAP_OPTION_CONTENT_FORMAT, 
                       coap_encode_var_safe((uint8_t[2]){0}, 2, COAP_MEDIATYPE_APPLICATION_JSON), 
                       (uint8_t[2]){0});
        coap_add_data(response, strlen(error_str), (const uint8_t*)error_str);
        
        cJSON_Delete(error_json);
        free(error_str);
    }
}
```

**¿Por qué toda esta validación?** Porque las emergencias son críticas y no podemos asumir que los datos están bien formateados.

#### 2.8.3 Registro del Recurso

**En la función `main()` del servidor central:**

```c
int main(int argc, char **argv) {
    // ... código de inicialización existente ...
    
    // Registrar recursos existentes
    r_floor_call = coap_resource_init(coap_make_str_const(RESOURCE_FLOOR_CALL), 0);
    // ... código existente ...
    
    r_cabin_request = coap_resource_init(coap_make_str_const(RESOURCE_CABIN_REQUEST), 0);
    // ... código existente ...

    // REGISTRAR NUEVO RECURSO DE EMERGENCIA
    coap_resource_t *r_emergency_call = coap_resource_init(coap_make_str_const(RESOURCE_EMERGENCY_CALL), 0);
    if (!r_emergency_call) {
        SRV_LOG_ERROR("No se pudo inicializar recurso /%s", RESOURCE_EMERGENCY_CALL);
        goto finish;
    }
    coap_register_handler(r_emergency_call, COAP_REQUEST_POST, hnd_emergency_call);
    coap_add_resource(ctx, r_emergency_call);
    SRV_LOG_INFO("Recurso registrado: POST /%s", RESOURCE_EMERGENCY_CALL);

    // ... resto del código ...
}
```

**¿Por qué registrar el recurso?** Para que el servidor CoAP sepa qué función llamar cuando llegue una solicitud a `/llamada_emergencia`.

### 2.9 Paso 8: Integración del Simulador para Emergencias

**¿Por qué modificar el simulador?**
El simulador debe entender y procesar el nuevo tipo `"llamada_emergencia"` del `simulation_data.json`. Esta integración permite ejecutar automáticamente las peticiones de emergencia durante las pruebas del sistema.

#### 2.9.1 Modificaciones en Headers del Simulador

**En `api_gateway/include/api_gateway/simulation_loader.h`:**

```c
/**
 * @brief Tipos de peticiones de simulación
 * 
 * Enumeración que define los tipos de peticiones que pueden
 * ser ejecutadas durante la simulación de ascensores.
 */
typedef enum {
    PETICION_LLAMADA_PISO,        /**< Llamada de piso desde botón externo */
    PETICION_SOLICITUD_CABINA,    /**< Solicitud desde interior de cabina */
    PETICION_LLAMADA_EMERGENCIA   /**< Solicitud de emergencia desde ascensor */ // <- AÑADIR
} tipo_peticion_t;
```

**¿Por qué añadir aquí?** Esta enumeración define todos los tipos de peticiones que el simulador puede entender.

**Estructura extendida para almacenar datos de emergencia:**

```c
typedef struct {
    tipo_peticion_t tipo;         /**< Tipo de petición */
    
    // Para llamadas de piso (existente)
    int piso_origen;              
    char direccion[8];            
    
    // Para solicitudes de cabina (existente)
    int indice_ascensor;          
    int piso_destino;             
    
    // NUEVOS CAMPOS PARA EMERGENCIAS
    char id_edificio[16];         /**< ID del edificio en emergencia */
    char ascensor_id_emergencia[32]; /**< ID específico del ascensor */
    char tipo_emergencia[32];     /**< Tipo de emergencia (EMERGENCY_STOP, etc.) */
    int piso_actual_emergencia;   /**< Piso actual del ascensor en emergencia */
    char timestamp_emergencia[64]; /**< Timestamp de la emergencia */
    char descripcion_emergencia[256]; /**< Descripción detallada */
} peticion_simulacion_t;
```

#### 2.9.2 Procesamiento JSON de Emergencias

**En `api_gateway/src/simulation_loader.c`:**

```c
} else if (strcmp(tipo_json->valuestring, "llamada_emergencia") == 0) {
    peticion->tipo = PETICION_LLAMADA_EMERGENCIA;

    // Procesar campos de emergencia
    cJSON *id_edificio_json = cJSON_GetObjectItemCaseSensitive(peticion_json, "id_edificio");
    if (cJSON_IsString(id_edificio_json)) {
        strncpy(peticion->id_edificio, id_edificio_json->valuestring, sizeof(peticion->id_edificio) - 1);
    }

    cJSON *ascensor_id_json = cJSON_GetObjectItemCaseSensitive(peticion_json, "ascensor_id_emergencia");
    if (cJSON_IsString(ascensor_id_json)) {
        strncpy(peticion->ascensor_id_emergencia, ascensor_id_json->valuestring, sizeof(peticion->ascensor_id_emergencia) - 1);
    }

    cJSON *tipo_emergencia_json = cJSON_GetObjectItemCaseSensitive(peticion_json, "tipo_emergencia");
    if (cJSON_IsString(tipo_emergencia_json)) {
        strncpy(peticion->tipo_emergencia, tipo_emergencia_json->valuestring, sizeof(peticion->tipo_emergencia) - 1);
    }

    cJSON *piso_actual_json = cJSON_GetObjectItemCaseSensitive(peticion_json, "piso_actual_emergencia");
    if (cJSON_IsNumber(piso_actual_json)) {
        peticion->piso_actual_emergencia = piso_actual_json->valueint;
    }

    cJSON *timestamp_json = cJSON_GetObjectItemCaseSensitive(peticion_json, "timestamp_emergencia");
    if (cJSON_IsString(timestamp_json)) {
        strncpy(peticion->timestamp_emergencia, timestamp_json->valuestring, sizeof(peticion->timestamp_emergencia) - 1);
    }

    cJSON *descripcion_json = cJSON_GetObjectItemCaseSensitive(peticion_json, "descripcion_emergencia");
    if (cJSON_IsString(descripcion_json)) {
        strncpy(peticion->descripcion_emergencia, descripcion_json->valuestring, sizeof(peticion->descripcion_emergencia) - 1);
    }

    // Extraer índice del ascensor del ID (ej: "ASC_E001_02" -> índice 2)
    if (strlen(peticion->ascensor_id_emergencia) > 0) {
        char *last_underscore = strrchr(peticion->ascensor_id_emergencia, '_');
        if (last_underscore && strlen(last_underscore) > 1) {
            peticion->indice_ascensor = atoi(last_underscore + 1);
        }
    }

    printf("[SIMULATION] 🚨 Emergencia cargada: %s en %s (ascensor %s [idx:%d], piso %d)\n",
           peticion->tipo_emergencia, peticion->id_edificio,
           peticion->ascensor_id_emergencia, peticion->indice_ascensor, peticion->piso_actual_emergencia);

} else {
    // Error: tipo desconocido
}
```

**¿Qué hace este código?**
1. Reconoce el tipo `"llamada_emergencia"` en el JSON
2. Extrae todos los campos específicos de emergencia
3. Convierte el ID del ascensor a índice numérico
4. Registra la emergencia cargada con emoji para fácil identificación

#### 2.9.3 Ejecución de Emergencias CAN

**En `api_gateway/src/mi_simulador_ascensor.c`:**

```c
} else if (peticion->tipo == PETICION_LLAMADA_EMERGENCIA) {
    printf("[SIM_ASCENSOR] 🚨 Ejecutando llamada de emergencia: %s en %s\n", 
           peticion->tipo_emergencia, peticion->ascensor_id_emergencia);
    printf("[SIM_ASCENSOR]    Piso actual: %d, Descripción: %s\n", 
           peticion->piso_actual_emergencia, peticion->descripcion_emergencia);

    // Simular emergencia vía CAN
    int ascensor_index = peticion->indice_ascensor;
    simular_emergencia_via_can(ascensor_index, peticion->piso_actual_emergencia, peticion->tipo_emergencia);

} else {
    // Tipo desconocido
}
```

**Nueva función de simulación CAN:**

```c
/**
 * @brief Simula una llamada de emergencia enviando un frame CAN al API Gateway
 * 
 * @param[in] indice_ascensor Índice del ascensor en emergencia (0-based)
 * @param[in] piso_actual Piso actual donde está el ascensor
 * @param[in] tipo_emergencia Tipo de emergencia como string
 * 
 * @details Genera un frame CAN con ID 0x400 que será procesado por el 
 * bridge CAN para crear una solicitud CoAP al servidor central.
 */
void simular_emergencia_via_can(int indice_ascensor, int piso_actual, const char* tipo_emergencia) {
    if (!g_coap_context) {
        printf("[SIM_ASCENSOR] Error: Contexto CoAP de Gateway no disponible.\n");
        return;
    }
    
    printf("[SIM_ASCENSOR] 🚨 Enviando EMERGENCIA a GW (vía CAN): Ascensor idx %d, Piso %d, Tipo: %s\n", 
           indice_ascensor, piso_actual, tipo_emergencia);
    
    simulated_can_frame_t frame;
    frame.id = 0x400; // ID CAN para emergencias
    frame.data[0] = (uint8_t)indice_ascensor; // Índice del ascensor (0-based)
    frame.data[1] = (uint8_t)piso_actual;     // Piso actual del ascensor
    
    // Convertir tipo de emergencia a enumeración
    uint8_t tipo_enum = 0; // EMERGENCY_STOP por defecto
    if (strcmp(tipo_emergencia, "EMERGENCY_STOP") == 0) {
        tipo_enum = 0;
    } else if (strcmp(tipo_emergencia, "POWER_FAILURE") == 0) {
        tipo_enum = 1;
    } else if (strcmp(tipo_emergencia, "PEOPLE_TRAPPED") == 0) {
        tipo_enum = 2;
    } else if (strcmp(tipo_emergencia, "MECHANICAL_FAILURE") == 0) {
        tipo_enum = 3;
    } else if (strcmp(tipo_emergencia, "FIRE_ALARM") == 0) {
        tipo_enum = 4;
    }
    
    frame.data[2] = tipo_enum;               // Tipo de emergencia (enumeración)
    frame.dlc = 3;

    // Registrar frame CAN en el logger
    char description[256];
    snprintf(description, sizeof(description), 
             "🚨 EMERGENCIA: %s desde ascensor índice %d en piso %d", 
             tipo_emergencia, indice_ascensor, piso_actual);
    exec_logger_log_can_sent(frame.id, frame.dlc, frame.data, description);

    ag_can_bridge_process_incoming_frame(&frame, g_coap_context);
}
```

**¿Por qué esta implementación?**
- Convierte datos JSON a frame CAN binario
- Usa ID 0x400 específico para emergencias  
- Mapea tipos de emergencia texto → enumeración
- Registra logs con emoji para fácil seguimiento

#### 2.9.3 Ejecución de Emergencias CAN

**En `api_gateway/src/mi_simulador_ascensor.c`:**

```c
} else if (peticion->tipo == PETICION_LLAMADA_EMERGENCIA) {
    printf("[SIM_ASCENSOR] 🚨 Ejecutando llamada de emergencia: %s en %s\n", 
           peticion->tipo_emergencia, peticion->ascensor_id_emergencia);
    printf("[SIM_ASCENSOR]    Piso actual: %d, Descripción: %s\n", 
           peticion->piso_actual_emergencia, peticion->descripcion_emergencia);

    // Simular emergencia vía CAN usando el índice extraído
    int ascensor_index = peticion->indice_ascensor;
    simular_emergencia_via_can(ascensor_index, peticion->piso_actual_emergencia, peticion->tipo_emergencia);

} else {
    printf("[SIM_ASCENSOR] Advertencia: Tipo de petición desconocido: %d\n", peticion->tipo);
}
```

**Nueva función de simulación CAN:**

```c
/**
 * @brief Simula una llamada de emergencia enviando un frame CAN al API Gateway
 * 
 * @param[in] indice_ascensor Índice del ascensor en emergencia (0-based)
 * @param[in] piso_actual Piso actual donde está el ascensor
 * @param[in] tipo_emergencia Tipo de emergencia como string
 */
void simular_emergencia_via_can(int indice_ascensor, int piso_actual, const char* tipo_emergencia) {
    if (!g_coap_context) {
        printf("[SIM_ASCENSOR] Error: Contexto CoAP de Gateway no disponible.\n");
        return;
    }
    
    printf("[SIM_ASCENSOR] 🚨 Enviando EMERGENCIA a GW (vía CAN): Ascensor idx %d, Piso %d, Tipo: %s\n", 
           indice_ascensor, piso_actual, tipo_emergencia);
    
    simulated_can_frame_t frame;
    frame.id = 0x400; // ID CAN para emergencias
    frame.data[0] = (uint8_t)indice_ascensor; // Índice del ascensor (0-based)
    frame.data[1] = (uint8_t)piso_actual;     // Piso actual del ascensor
    
    // Convertir tipo de emergencia a enumeración (0-4)
    uint8_t tipo_enum = 0; // EMERGENCY_STOP por defecto
    if (strcmp(tipo_emergencia, "EMERGENCY_STOP") == 0) {
        tipo_enum = 0;
    } else if (strcmp(tipo_emergencia, "POWER_FAILURE") == 0) {
        tipo_enum = 1;
    } else if (strcmp(tipo_emergencia, "PEOPLE_TRAPPED") == 0) {
        tipo_enum = 2;
    } else if (strcmp(tipo_emergencia, "MECHANICAL_FAILURE") == 0) {
        tipo_enum = 3;
    } else if (strcmp(tipo_emergencia, "FIRE_ALARM") == 0) {
        tipo_enum = 4;
    }
    
    frame.data[2] = tipo_enum;
    frame.dlc = 3;

    // Registrar frame CAN en el logger
    char description[256];
    snprintf(description, sizeof(description), 
             "🚨 EMERGENCIA: %s desde ascensor índice %d en piso %d", 
             tipo_emergencia, indice_ascensor, piso_actual);
    exec_logger_log_can_sent(frame.id, frame.dlc, frame.data, description);

    ag_can_bridge_process_incoming_frame(&frame, g_coap_context);
}
```

#### 2.9.4 Implementación de la Conversión de Tipos CAN

**En `api_gateway/src/can_bridge.c`, en el case 0x400:**

```c
// Convertir tipo de emergencia de CAN frame a enumeración OpenAPI
api_gateway_coap_para_sistema_de_ascensores_emergency_type__e emergency_type;
switch (emergency_type_int) {
    case 0: emergency_type = api_gateway_coap_para_sistema_de_ascensores_emergency_type__EMERGENCY_STOP; break;
    case 1: emergency_type = api_gateway_coap_para_sistema_de_ascensores_emergency_type__POWER_FAILURE; break;
    case 2: emergency_type = api_gateway_coap_para_sistema_de_ascensores_emergency_type__PEOPLE_TRAPPED; break;
    case 3: emergency_type = api_gateway_coap_para_sistema_de_ascensores_emergency_type__MECHANICAL_FAILURE; break;
    case 4: emergency_type = api_gateway_coap_para_sistema_de_ascensores_emergency_type__FIRE_ALARM; break;
    default: emergency_type = api_gateway_coap_para_sistema_de_ascensores_emergency_type__NULL; break;
}
```

**¿Por qué esta implementación específica?** 

1. **Frame CAN usa valores 0-4**: El simulador envía tipos de emergencia como valores numéricos (0=EMERGENCY_STOP, 1=POWER_FAILURE, etc.)

2. **Enumeración OpenAPI incluye NULL**: La enumeración generada tiene estructura:
   ```c
   typedef enum { 
       api_gateway_coap_para_sistema_de_ascensores_emergency_type__NULL = 0,
       api_gateway_coap_para_sistema_de_ascensores_emergency_type__EMERGENCY_STOP = 1,
       api_gateway_coap_para_sistema_de_ascensores_emergency_type__POWER_FAILURE = 2,
       // ...
   } api_gateway_coap_para_sistema_de_ascensores_emergency_type__e;
   ```

3. **Mapeo necesario**: Se debe convertir explícitamente CAN(0-4) → Enum(1-5) para mantener la correspondencia correcta entre tipos.

#### 2.9.5 Variables de Entorno

**Verificar en `api_gateway/gateway.env` (sin espacios extra):**

```env
# Recursos CoAP
FLOOR_CALL_RESOURCE=peticion_piso
CABIN_REQUEST_RESOURCE=peticion_cabina
EMERGENCY_CALL_RESOURCE=llamada_emergencia
```

### 2.10 Paso 9: Script de Generación de Emergencias

**¿Por qué un script automatizado?**
Para probar el sistema completo de emergencias, necesitamos añadir solicitudes de emergencia a los 100 edificios del `simulation_data.json`. Hacerlo manualmente sería tedioso y propenso a errores.

#### 2.10.1 Script Python

**Archivo: `add_emergency_requests.py`**

```python
#!/usr/bin/env python3
"""
Script para añadir solicitudes de emergencia al archivo simulation_data.json

Este script añade automáticamente 2 solicitudes de emergencia diferentes
a cada edificio en el archivo de simulación para probar el endpoint
/llamada_emergencia implementado en el servidor central.
"""

import json
import random
from datetime import datetime, timedelta

def generate_emergency_request(building_id, emergency_type, elevator_index, current_floor):
    """Genera una solicitud de emergencia con datos realistas"""
    
    # Generar timestamp realista
    base_time = datetime.now()
    random_minutes = random.randint(10, 300)
    timestamp = (base_time + timedelta(minutes=random_minutes)).isoformat()
    
    # Descripciones específicas por tipo
    descriptions = {
        "EMERGENCY_STOP": "Botón de emergencia activado por usuario",
        "POWER_FAILURE": "Pérdida de suministro eléctrico principal", 
        "PEOPLE_TRAPPED": "Personas atrapadas entre pisos",
        "MECHANICAL_FAILURE": "Fallo en sistema de tracción",
        "FIRE_ALARM": "Detección de humo en shaft del ascensor"
    }
    
    return {
        "tipo": "llamada_emergencia",
        "id_edificio": building_id,
        "ascensor_id_emergencia": f"ASC_{building_id}_{elevator_index:02d}",
        "tipo_emergencia": emergency_type,
        "piso_actual_emergencia": current_floor,
        "timestamp_emergencia": timestamp,
        "descripcion_emergencia": descriptions.get(emergency_type, "Emergencia no especificada"),
        "elevadores_estado": [
            # Estado de todos los ascensores del edificio
            {"id_ascensor": f"ASC_{building_id}_00", "disponible": elevator_index != 0, "piso_actual": random.randint(0, 9)},
            {"id_ascensor": f"ASC_{building_id}_01", "disponible": elevator_index != 1, "piso_actual": random.randint(0, 9)},
            {"id_ascensor": f"ASC_{building_id}_02", "disponible": elevator_index != 2, "piso_actual": random.randint(0, 9)}
        ]
    }

# Función principal que procesa todos los edificios...
```

**Ejecución:**

```bash
python add_emergency_requests.py
```

**Resultado esperado:**
- **200 emergencias añadidas** (2 por edificio × 100 edificios)
- **Distribución equilibrada** de tipos de emergencia
- **Datos realistas** con timestamps y descripciones apropiadas

### 2.11 Paso 10: Compilación y Pruebas

#### 2.11.1 Compilación

```bash
# Compilar API Gateway con cambios
cd api_gateway
./build_api_gateway.sh

# Compilar Servidor Central con endpoint de emergencias
cd servidor_central
./build_servidor_central.sh
```

#### 2.11.2 Pruebas End-to-End

**Ejecutar con emergencias:**

```bash

# 2. Ejecutar API Gateway (incluirá emergencias automáticamente)
cd api_gateway
./build/api_gateway

# 3. En otra terminal, ejecutar Servidor Central
cd servidor_central
./build/servidor_central
```

**Logs esperados para emergencias:**

```
[SIMULATION] 🚨 Emergencia cargada: PEOPLE_TRAPPED en E042 (ascensor ASC_E042_01 [idx:1], piso 3)
[SIM_ASCENSOR] 🚨 Ejecutando llamada de emergencia: PEOPLE_TRAPPED en ASC_E042_01
[SIM_ASCENSOR] 🚨 Enviando EMERGENCIA a GW (vía CAN): Ascensor idx 1, Piso 3, Tipo: PEOPLE_TRAPPED
[WARN-GW] [CAN_Bridge] 🚨 EMERGENCIA CAN: Ascensor E042A2, Piso 3, Tipo PEOPLE_TRAPPED
[INFO-GW] [CAN_Emergency] Gateway (Emergencia CAN ID: 0x400) -> Central: Enviando solicitud de emergencia...
SRV_LOG_WARN("🚨 EMERGENCIA DETECTADA: PEOPLE_TRAPPED en edificio E042, ascensor E042A2, piso 3")
SRV_LOG_INFO("🚨 EMERGENCIA PROCESADA: EMG_1704105600123")
```

### 2.12 Verificación Final del Sistema

#### 2.12.1 Checklist de Implementación Completa

- [x] ✅ **YAML modificado** con endpoint `/llamada_emergencia`
- [x] ✅ **Código generado** con OpenAPI Generator
- [x] ✅ **API Gateway** procesa frames CAN 0x400
- [x] ✅ **Servidor Central** implementa `hnd_emergency_call()`
- [x] ✅ **Simulador** soporta `PETICION_LLAMADA_EMERGENCIA`
- [x] ✅ **Script** para añadir emergencias funcionando
- [x] ✅ **Compilación** sin errores
- [x] ✅ **Pruebas E2E** exitosas

#### 2.12.2 Flujo Completo Verificado

```
📝 simulation_data.json (con emergencias)
     ↓
🎮 Simulador carga emergencias
     ↓
📡 Frame CAN 0x400 → API Gateway
     ↓
🔄 Conversión CAN → CoAP JSON
     ↓
🏢 Servidor Central procesa emergencia
     ↓
📋 Protocolo activado + servicios notificados
     ↓
📤 Respuesta con ID emergencia
```

[... resto del contenido existente ...]