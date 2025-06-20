cmake_minimum_required(VERSION 3.10)

# Buscar CUnit
find_library(CUNIT_LIBRARY cunit)
find_path(CUNIT_INCLUDE_DIR CUnit/CUnit.h)

if(NOT CUNIT_LIBRARY OR NOT CUNIT_INCLUDE_DIR)
    message(FATAL_ERROR "CUnit not found. Install with: sudo apt-get install libcunit1-dev")
endif()

message(STATUS "CUnit found: ${CUNIT_LIBRARY}")
message(STATUS "CUnit include: ${CUNIT_INCLUDE_DIR}")

# Buscar dependencias del proyecto
find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBCOAP REQUIRED libcoap-3-openssl)
pkg_check_modules(LIBCJSON REQUIRED libcjson)

# Función helper para crear tests con reportes
function(add_test_with_report test_name source_file)
    add_executable(${test_name} ${source_file})
    
    target_include_directories(${test_name} PRIVATE
        ${CMAKE_SOURCE_DIR}/api_gateway/include
        ${CMAKE_SOURCE_DIR}/servidor_central/include
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
    )
    
    # Crear directorio de reportes
    file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/test_reports)
    
    # Añadir test con reporte XML
    add_test(NAME ${test_name} 
             COMMAND ${test_name} --automated
             WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/test_reports)
    
    # Configurar propiedades del test
    set_tests_properties(${test_name} PROPERTIES
        TIMEOUT 30
        ENVIRONMENT "CUNIT_OUT_NAME=${test_name}"
    )
endfunction()

# Crear biblioteca estática con código fuente para testing
add_library(elevator_system_lib STATIC
    ${CMAKE_SOURCE_DIR}/api_gateway/src/elevator_state_manager.c
    ${CMAKE_SOURCE_DIR}/api_gateway/src/can_bridge.c
    ${CMAKE_SOURCE_DIR}/api_gateway/src/api_handlers.c
)

target_include_directories(elevator_system_lib PUBLIC
    ${CMAKE_SOURCE_DIR}/api_gateway/include
    ${LIBCOAP_INCLUDE_DIRS}
    ${LIBCJSON_INCLUDE_DIRS}
)

target_link_libraries(elevator_system_lib
    ${LIBCOAP_LIBRARIES}
    ${LIBCJSON_LIBRARIES}
)

# Crear directorio para mocks
add_subdirectory(mocks)

# Pruebas unitarias
add_test_with_report(test_elevator_state_manager unit/test_elevator_state_manager.c)
add_test_with_report(test_can_bridge unit/test_can_bridge.c)
add_test_with_report(test_api_handlers unit/test_api_handlers.c)
add_test_with_report(test_servidor_central unit/test_servidor_central.c)

# Pruebas de integración
add_test_with_report(test_can_to_coap integration/test_can_to_coap.c)

# Nota: El reporte consolidado se genera automáticamente por run_all_tests.sh 