/**
 * @file can_bridge.h
 * @brief Puente CAN-CoAP para el API Gateway
 * @author Sistema de Control de Ascensores
 * @date 2025
 * @version 2.0
 * 
 * Este archivo define la interfaz del puente de comunicación entre el protocolo CAN
 * (Controller Area Network) y CoAP (Constrained Application Protocol) para
 * el sistema de control de ascensores.
 * 
 * **Funcionalidades principales:**
 * - Procesamiento de frames CAN entrantes desde simuladores
 * - Conversión de mensajes CAN a solicitudes CoAP
 * - Gestión de trackers para correlación de respuestas
 * - Envío de respuestas del servidor central vía CAN
 * - Interfaz con simuladores de ascensores
 * 
 * **Tipos de mensajes CAN soportados:**
 * - 0x100: Llamadas de piso (floor calls) con dirección
 * - 0x200: Solicitudes de cabina (cabin requests) con destino
 * - 0x300: Notificaciones de llegada de ascensores
 * - 0x400: Llamadas de emergencia desde ascensores
 * 
 * El puente mantiene un buffer circular de trackers para correlacionar
 * respuestas del servidor central con las solicitudes CAN originales.
 * 
 * @see can_bridge.c
 * @see elevator_state_manager.h
 * @see api_handlers.h
 */
#ifndef CAN_BRIDGE_H
#define CAN_BRIDGE_H

#include <stdint.h>
#include <stdbool.h>
#include <coap3/coap.h> // Para coap_bin_const_t y otros tipos CoAP
#include <cjson/cJSON.h>  // Para cJSON tipo
#include "api_gateway/elevator_state_manager.h" // Para gw_request_type_t
#include "api_gateway/api_common_defs.h"    // Para ID_STRING_MAX_LEN

/**
 * @brief Estructura para un frame CAN simulado
 * 
 * Representa un frame CAN estándar con ID, datos y longitud.
 * Utilizada para la comunicación entre el simulador de ascensores
 * y el API Gateway.
 */
typedef struct {
    uint32_t id;      ///< CAN ID (identificador del mensaje)
    uint8_t data[8]; ///< Datos CAN (hasta 8 bytes según estándar CAN)
    uint8_t dlc;     ///< Data Length Code (0-8, número de bytes válidos)
} simulated_can_frame_t;

/**
 * @brief Tracker para correlacionar solicitudes CAN con respuestas CoAP
 * 
 * Estructura utilizada para asociar un token CoAP de una solicitud
 * enviada al servidor central con la información original del frame CAN
 * que la originó. Permite correlacionar respuestas para enviar
 * confirmaciones apropiadas vía CAN.
 */
typedef struct {
    coap_binary_t coap_token;        ///< Token de la solicitud CoAP enviada al servidor central
    uint32_t original_can_id;        ///< ID del frame CAN original que originó la solicitud
    gw_request_type_t request_type;  ///< Tipo de solicitud original (floor call, cabin request)
    int target_floor_for_task;       ///< Piso destino de la tarea asignada
    int call_reference_floor;        ///< Piso origen de la llamada (para floor calls)
    char requesting_elevator_id_if_cabin[ID_STRING_MAX_LEN]; ///< ID del ascensor si fue cabin request
} can_origin_tracker_t;

/**
 * @brief Callback para envío de frames CAN al simulador
 * @param frame El frame CAN simulado a enviar al simulador del ascensor
 * 
 * Tipo de función callback que debe implementar el simulador de ascensores
 * para recibir frames CAN de respuesta del API Gateway. El simulador
 * registra una función de este tipo usando ag_can_bridge_register_send_callback().
 * 
 * **Responsabilidades del callback:**
 * - Procesar el frame CAN recibido
 * - Interpretar códigos de respuesta y datos
 * - Actualizar estado del simulador según corresponda
 * - Generar logging apropiado para debugging
 * 
 * @see ag_can_bridge_register_send_callback()
 * @see ag_can_bridge_send_response_frame()
 */
typedef void (*can_send_callback_t)(simulated_can_frame_t* frame);

/**
 * @brief Inicializa el puente CAN simulado
 * 
 * Esta función inicializa el sistema de puente CAN-CoAP, preparando
 * todas las estructuras de datos necesarias para el funcionamiento.
 * 
 * **Operaciones realizadas:**
 * - Limpia el callback de envío CAN
 * - Libera memoria de tokens almacenados en trackers
 * - Inicializa a cero el buffer de trackers CAN
 * - Resetea el índice del buffer circular
 * 
 * Debe llamarse una vez al inicio del programa antes de procesar
 * cualquier frame CAN o registrar callbacks.
 * 
 * @see ag_can_bridge_register_send_callback()
 * @see ag_can_bridge_process_incoming_frame()
 */
void ag_can_bridge_init(void);

/**
 * @brief Registra la función de callback que la API Gateway usará para enviar frames CAN
 * @param callback Puntero a la función de callback implementada por la simulación
 * 
 * Esta función registra una función callback que será utilizada por el
 * puente CAN-CoAP para enviar frames CAN de respuesta al simulador.
 * 
 * El callback debe implementar la interfaz can_send_callback_t y será
 * invocado cuando el puente necesite enviar respuestas del servidor
 * central de vuelta al sistema CAN simulado.
 * 
 * **Características:**
 * - Solo se puede registrar un callback a la vez
 * - Llamadas posteriores sobrescriben el callback anterior
 * - El callback debe ser thread-safe si se usa en entorno multihilo
 * 
 * @note El callback debe permanecer válido durante toda la vida del programa
 * 
 * @see can_send_callback_t
 * @see ag_can_bridge_send_response_frame()
 */
void ag_can_bridge_register_send_callback(can_send_callback_t callback);

/**
 * @brief Procesa un frame CAN simulado entrante desde la simulación de ascensores
 * @param frame Puntero al frame CAN simulado recibido
 * @param coap_context El contexto CoAP de la API Gateway, necesario para enviar solicitudes
 * 
 * Esta función es el punto de entrada principal para el procesamiento
 * de frames CAN entrantes. Interpreta el contenido del frame según
 * el esquema de mensajes CAN definido y genera las solicitudes CoAP
 * correspondientes al servidor central.
 * 
 * **Tipos de frames CAN soportados:**
 * - **0x100 - Llamada de piso**: 
 *   - data[0]: Piso origen (0-255)
 *   - data[1]: Dirección (0=UP, 1=DOWN)
 * - **0x200 - Solicitud de cabina**:
 *   - data[0]: Índice del ascensor (0-based)
 *   - data[1]: Piso destino (0-255)
 * - **0x300 - Notificación de llegada**:
 *   - data[0]: Índice del ascensor (0-based)
 *   - data[1]: Piso actual (0-255)
 * - **0x400 - Llamada de emergencia**:
 *   - data[0]: Índice del ascensor (0-based)
 *   - data[1]: Piso donde está el ascensor en emergencia
 *   - data[2]: Tipo de emergencia (enum)
 * 
 * **Procesamiento realizado:**
 * 1. Validación del formato y longitud de datos
 * 2. Extracción de parámetros específicos del tipo de mensaje
 * 3. Generación de solicitud CoAP al servidor central
 * 4. Almacenamiento de tracker para correlación de respuesta
 * 
 * @note Los IDs CAN y formato de datos deben adaptarse según
 *       el esquema específico del sistema de ascensores
 * 
 * @see forward_can_originated_request_to_central_server()
 * @see store_can_tracker()
 * @see simulated_can_frame_t
 */
void ag_can_bridge_process_incoming_frame(simulated_can_frame_t* frame, struct coap_context_t *coap_context);

/**
 * @brief Busca un tracker de origen CAN basado en un token CoAP
 * @param token El token CoAP de la respuesta recibida del servidor central
 * @return Puntero al can_origin_tracker_t si se encuentra, NULL en caso contrario
 * 
 * Esta función busca en el buffer circular de trackers CAN un tracker
 * que corresponda al token CoAP especificado. Se utiliza para correlacionar
 * respuestas del servidor central con solicitudes CAN originales.
 * 
 * **Características de la búsqueda:**
 * - Comparación binaria exacta del token (longitud + contenido)
 * - Búsqueda lineal en buffer circular
 * - No remueve el tracker de la lista (a diferencia de find_and_remove_central_request_tracker)
 * - Thread-safe para lecturas concurrentes
 * 
 * **Uso típico:**
 * 1. Se recibe respuesta del servidor central con token
 * 2. Se busca el tracker correspondiente
 * 3. Se extrae información del frame CAN original
 * 4. Se genera respuesta CAN apropiada
 * 
 * @see store_can_tracker()
 * @see can_origin_tracker_t
 * @see ag_can_bridge_send_response_frame()
 */
can_origin_tracker_t* find_can_tracker(coap_bin_const_t token);

/**
 * @brief Envía una respuesta (traducida de CoAP) como un frame CAN simulado a la simulación
 * @param original_can_id El ID del frame CAN original que originó esta respuesta
 * @param response_code El código de respuesta CoAP recibido del servidor central
 * @param server_response_json El objeto cJSON que contiene la respuesta del servidor central (puede ser NULL)
 * 
 * Esta función convierte una respuesta CoAP del servidor central en un frame CAN
 * simulado que se envía de vuelta al simulador de ascensores. Maneja tanto
 * respuestas exitosas como códigos de error.
 * 
 * **Procesamiento de respuestas exitosas:**
 * - Extrae ascensor_asignado_id y tarea_id del JSON
 * - Convierte ID de ascensor a índice numérico
 * - Construye frame CAN con información de asignación
 * - ID de respuesta: original_can_id + 1
 * 
 * **Procesamiento de errores:**
 * - Detecta códigos de error CoAP o campo "error" en JSON
 * - Genera frame CAN de error con ID 0xFE
 * - Incluye código de error específico en data[1]
 * 
 * **Códigos de error definidos:**
 * - 0x01: JSON faltante o nulo del servidor
 * - 0x02: Servidor reportó error o código CoAP de error
 * - 0x03: Fallo al parsear JSON de éxito
 * 
 * La función utiliza el callback registrado para enviar el frame
 * al simulador de ascensores.
 * 
 * @see ag_can_bridge_register_send_callback()
 * @see can_send_callback_t
 * @see simulated_can_frame_t
 */
void ag_can_bridge_send_response_frame(uint32_t original_can_id, coap_pdu_code_t response_code, cJSON* server_response_json);

#endif // CAN_BRIDGE_H 