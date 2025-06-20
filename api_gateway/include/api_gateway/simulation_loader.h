/**
 * @file simulation_loader.h
 * @brief Sistema de Carga y Ejecución de Simulaciones de Ascensores
 * @author Sistema de Control de Ascensores
 * @date 2025
 * @version 1.0
 * 
 * Este archivo define las funciones para cargar y ejecutar simulaciones
 * de ascensores desde archivos JSON. Permite probar el sistema con
 * múltiples escenarios de edificios y peticiones variadas.
 * 
 * **Funcionalidades principales:**
 * - Carga de datos de simulación desde JSON
 * - Selección aleatoria de edificios
 * - Ejecución secuencial de peticiones
 * - Gestión de memoria para datos de simulación
 * 
 * **Formato de datos esperado:**
 * El archivo JSON debe contener un array de edificios, cada uno con
 * un ID único y un conjunto de peticiones específicas.
 * 
 * @see mi_simulador_ascensor.h
 * @see cJSON
 */
#ifndef SIMULATION_LOADER_H
#define SIMULATION_LOADER_H

#include <coap3/coap.h>
#include <cjson/cJSON.h>
#include <stdbool.h>

/**
 * @defgroup simulation_data Estructuras de Datos de Simulación
 * @brief Estructuras para almacenar datos de simulación
 * @{
 */

/**
 * @brief Tipos de peticiones de simulación
 * 
 * Enumeración que define los tipos de peticiones que pueden
 * ser ejecutadas durante la simulación de ascensores.
 */
typedef enum {
    PETICION_LLAMADA_PISO,    /**< Llamada de piso desde botón externo */
    PETICION_SOLICITUD_CABINA /**< Solicitud desde interior de cabina */
} tipo_peticion_t;

/**
 * @brief Estructura para una petición individual
 * 
 * Contiene toda la información necesaria para ejecutar una
 * petición específica durante la simulación.
 */
typedef struct {
    tipo_peticion_t tipo;         /**< Tipo de petición */
    
    // Para llamadas de piso
    int piso_origen;              /**< Piso desde el cual se llama */
    char direccion[8];            /**< Dirección: "up" o "down" */
    
    // Para solicitudes de cabina
    int indice_ascensor;          /**< Índice del ascensor (0-based) */
    int piso_destino;             /**< Piso destino solicitado */
} peticion_simulacion_t;

/**
 * @brief Estructura para un edificio de simulación
 * 
 * Representa un edificio completo con su ID único y
 * conjunto de peticiones asociadas.
 */
typedef struct {
    char id_edificio[16];         /**< ID único del edificio (ej: "E001") */
    peticion_simulacion_t *peticiones; /**< Array de peticiones */
    int num_peticiones;           /**< Número de peticiones en el array */
} edificio_simulacion_t;

/**
 * @brief Estructura principal de datos de simulación
 * 
 * Contiene todos los edificios cargados desde el archivo JSON
 * y proporciona acceso a los datos de simulación.
 */
typedef struct {
    edificio_simulacion_t *edificios; /**< Array de edificios */
    int num_edificios;                 /**< Número total de edificios */
    bool datos_cargados;              /**< Indica si los datos fueron cargados correctamente */
} datos_simulacion_t;

/** @} */ // end of simulation_data group

/**
 * @defgroup simulation_functions Funciones de Simulación
 * @brief Funciones para cargar y ejecutar simulaciones
 * @{
 */

/**
 * @brief Carga los datos de simulación desde un archivo JSON
 * @param archivo_json Ruta al archivo JSON con los datos de simulación
 * @param datos Puntero a la estructura donde almacenar los datos cargados
 * @return true si la carga fue exitosa, false en caso de error
 * 
 * Esta función lee un archivo JSON que contiene edificios y sus peticiones
 * asociadas, parseando la información y almacenándola en estructuras C
 * para su posterior ejecución.
 * 
 * **Formato JSON esperado:**
 * ```json
 * {
 *   "edificios": [
 *     {
 *       "id_edificio": "E001",
 *       "peticiones": [
 *         {"tipo": "llamada_piso", "piso_origen": 0, "direccion": "up"},
 *         {"tipo": "solicitud_cabina", "indice_ascensor": 0, "piso_destino": 5}
 *       ]
 *     }
 *   ]
 * }
 * ```
 * 
 * @see liberar_datos_simulacion()
 * @see cJSON_Parse()
 */
bool cargar_datos_simulacion(const char *archivo_json, datos_simulacion_t *datos);

/**
 * @brief Libera la memoria utilizada por los datos de simulación
 * @param datos Puntero a la estructura de datos a liberar
 * 
 * Esta función libera toda la memoria dinámica asignada durante
 * la carga de datos de simulación, incluyendo arrays de edificios
 * y peticiones.
 * 
 * @see cargar_datos_simulacion()
 */
void liberar_datos_simulacion(datos_simulacion_t *datos);

/**
 * @brief Selecciona un edificio aleatorio de los datos cargados
 * @param datos Puntero a los datos de simulación cargados
 * @return Puntero al edificio seleccionado, NULL si no hay datos
 * 
 * Esta función utiliza un generador de números aleatorios para
 * seleccionar un edificio de la lista cargada. La selección es
 * uniforme entre todos los edificios disponibles.
 * 
 * @note Inicializa el generador aleatorio con el tiempo actual
 * @see srand()
 * @see rand()
 */
edificio_simulacion_t* seleccionar_edificio_aleatorio(datos_simulacion_t *datos);

/**
 * @brief Ejecuta todas las peticiones de un edificio
 * @param edificio Puntero al edificio cuyas peticiones ejecutar
 * @param ctx Contexto CoAP para el procesamiento de peticiones
 * @return Número de peticiones ejecutadas exitosamente
 * 
 * Esta función ejecuta secuencialmente todas las peticiones de un
 * edificio específico, con pausas entre peticiones para permitir
 * el procesamiento de respuestas CoAP.
 * 
 * **Proceso de ejecución:**
 * 1. Configura el ID del edificio en el sistema
 * 2. Ejecuta cada petición según su tipo
 * 3. Procesa I/O CoAP entre peticiones
 * 4. Proporciona logging detallado del progreso
 * 
 * @see simular_llamada_de_piso_via_can()
 * @see simular_solicitud_cabina_via_can()
 * @see coap_io_process()
 */
int ejecutar_peticiones_edificio(edificio_simulacion_t *edificio, coap_context_t *ctx);

/**
 * @brief Convierte una cadena de dirección a enum
 * @param direccion_str Cadena con la dirección ("up" o "down")
 * @return Valor del enum correspondiente
 * 
 * Función auxiliar para convertir las direcciones en formato texto
 * del JSON a los valores enum utilizados por el simulador.
 * 
 * @see movement_direction_enum_t
 */
int convertir_direccion_string(const char *direccion_str);

/** @} */ // end of simulation_functions group

#endif // SIMULATION_LOADER_H 