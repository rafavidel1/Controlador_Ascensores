cmake_minimum_required(VERSION 3.10)

# Declarar el proyecto
project(ElevatorSystemTests VERSION 1.0.0 LANGUAGES C)

# Mostrar información del proyecto
message(STATUS "Configurando proyecto: ${PROJECT_NAME} v${PROJECT_VERSION}")
message(STATUS "Directorio fuente: ${CMAKE_CURRENT_SOURCE_DIR}")
message(STATUS "Directorio build: ${CMAKE_CURRENT_BINARY_DIR}")

# Buscar CUnit
find_library(CUNIT_LIBRARY cunit)
find_path(CUNIT_INCLUDE_DIR CUnit/CUnit.h)

if(NOT CUNIT_LIBRARY OR NOT CUNIT_INCLUDE_DIR)
    message(FATAL_ERROR "CUnit not found. Install with: sudo apt-get install libcunit1-dev")
endif()

message(STATUS "CUnit found: ${CUNIT_LIBRARY}")
message(STATUS "CUnit include: ${CUNIT_INCLUDE_DIR}")

# Configurar estándares y opciones del compilador
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Buscar dependencias del proyecto
find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBCOAP REQUIRED libcoap-3-openssl)
pkg_check_modules(LIBCJSON REQUIRED libcjson)

# Habilitar testing
enable_testing()

# Función helper para crear tests con reportes
function(add_test_with_report test_name source_file)
    add_executable(${test_name} ${source_file})
    
    # Buscar directorios de includes
    set(SERVIDOR_CENTRAL_INC_DIR)
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../servidor_central/include")
        set(SERVIDOR_CENTRAL_INC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../servidor_central/include")
    elseif(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../../servidor_central/include")
        set(SERVIDOR_CENTRAL_INC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../servidor_central/include")
    endif()
    
    target_include_directories(${test_name} PRIVATE
        ${API_GATEWAY_INC_DIR}
        ${SERVIDOR_CENTRAL_INC_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CUNIT_INCLUDE_DIR}
        ${LIBCOAP_INCLUDE_DIRS}
        ${LIBCJSON_INCLUDE_DIRS}
    )
    
    target_link_libraries(${test_name}
        ${CUNIT_LIBRARY}
        ${LIBCOAP_LIBRARIES}
        ${LIBCJSON_LIBRARIES}
        elevator_system_lib
        test_mocks
        m      # Biblioteca matemática
        pthread # Biblioteca de threads
    )
    
    # Añadir test con reporte XML
    add_test(NAME ${test_name} 
             COMMAND ${test_name} --automated
             WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
    
    # Configurar propiedades del test
    set_tests_properties(${test_name} PROPERTIES
        TIMEOUT 30
        ENVIRONMENT "CUNIT_OUT_NAME=${test_name}"
    )
endfunction()

# Crear biblioteca estática con código fuente para testing
# Buscar archivos fuente en ubicaciones relativas posibles
set(API_GATEWAY_SRC_DIR)

# Buscar directorio de fuentes del API Gateway
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../api_gateway/src")
    set(API_GATEWAY_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../api_gateway/src")
elseif(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../../api_gateway/src")
    set(API_GATEWAY_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../api_gateway/src")
else()
    message(FATAL_ERROR "No se encontró el directorio src del API Gateway")
endif()

message(STATUS "Usando archivos fuente del API Gateway desde: ${API_GATEWAY_SRC_DIR}")

add_library(elevator_system_lib STATIC
    ${API_GATEWAY_SRC_DIR}/elevator_state_manager.c
    ${API_GATEWAY_SRC_DIR}/can_bridge.c
    ${API_GATEWAY_SRC_DIR}/api_handlers.c
)

# Buscar directorio de includes del API Gateway
set(API_GATEWAY_INC_DIR)
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../api_gateway/include")
    set(API_GATEWAY_INC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../api_gateway/include")
elseif(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../../api_gateway/include")
    set(API_GATEWAY_INC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../api_gateway/include")
else()
    message(FATAL_ERROR "No se encontró el directorio include del API Gateway")
endif()

target_include_directories(elevator_system_lib PUBLIC
    ${API_GATEWAY_INC_DIR}
    ${LIBCOAP_INCLUDE_DIRS}
    ${LIBCJSON_INCLUDE_DIRS}
)

target_link_libraries(elevator_system_lib
    ${LIBCOAP_LIBRARIES}
    ${LIBCJSON_LIBRARIES}
    m      # Biblioteca matemática
    pthread # Biblioteca de threads
)

# Crear directorio para mocks
add_subdirectory(mocks)

# Pruebas unitarias
add_test_with_report(test_elevator_state_manager unit/test_elevator_state_manager.c)
add_test_with_report(test_can_bridge unit/test_can_bridge.c)
add_test_with_report(test_api_handlers unit/test_api_handlers.c)
add_test_with_report(test_servidor_central unit/test_servidor_central.c)
add_test_with_report(test_psk_security unit/test_psk_security.c)

# Pruebas de integración
add_test_with_report(test_can_to_coap integration/test_can_to_coap.c)

# Nota: El reporte consolidado se genera automáticamente por run_all_tests.sh 