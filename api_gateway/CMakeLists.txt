cmake_minimum_required(VERSION 3.10)
project(ApiGateway C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED True)

# Find LibCoAP using pkg-config
find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBCOAP REQUIRED libcoap-3-openssl)

# Find cJSON using pkg-config
pkg_check_modules(LIBCJSON REQUIRED libcjson)

# Source files for the executable
set(API_GATEWAY_SOURCES
    src/main.c
    src/api_handlers.c
    src/elevator_state_manager.c
    src/can_bridge.c
    src/mi_simulador_ascensor.c
    src/simulation_loader.c
    src/execution_logger.c
)

# Add the executable
add_executable(api_gateway ${API_GATEWAY_SOURCES})

# Crear ejecutable adicional con puerto dinámico para pruebas de carga
add_executable(api_gateway_dynamic
    src/main_dynamic_port.c
    src/api_handlers.c
    src/can_bridge.c
    src/elevator_state_manager.c
    src/mi_simulador_ascensor.c
    src/simulation_loader.c
    src/execution_logger.c
)

# Configure include directories and link libraries for the target
if(LIBCOAP_FOUND)
    message(STATUS "Found LibCoAP: YES")
    message(STATUS "LibCoAP include directories: ${LIBCOAP_INCLUDE_DIRS}")
    message(STATUS "LibCoAP library directories: ${LIBCOAP_LIBRARY_DIRS}")
    message(STATUS "LibCoAP libraries: ${LIBCOAP_LIBRARIES}")
    message(STATUS "LibCoAP CFLAGS: ${LIBCOAP_CFLAGS}")
    message(STATUS "LibCoAP LDFLAGS: ${LIBCOAP_LDFLAGS}")

    # Configuración para api_gateway original
    target_include_directories(api_gateway PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${LIBCOAP_INCLUDE_DIRS}
        ${LIBCJSON_INCLUDE_DIRS}
    )

    if(LIBCOAP_CFLAGS)
      string(REPLACE " " ";" LIBCOAP_CFLAGS_LIST ${LIBCOAP_CFLAGS})
      target_compile_options(api_gateway PRIVATE ${LIBCOAP_CFLAGS_LIST})
    endif()

    # Add library search paths for the target
    target_link_directories(api_gateway PRIVATE ${LIBCOAP_LIBRARY_DIRS})
    
    # Link the api_gateway executable against LibCoAP libraries
    target_link_libraries(api_gateway PRIVATE 
        ${LIBCOAP_LIBRARIES}
        ${LIBCJSON_LIBRARIES}
    )

    # Configuración para api_gateway_dynamic
    target_include_directories(api_gateway_dynamic PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${LIBCOAP_INCLUDE_DIRS}
        ${LIBCJSON_INCLUDE_DIRS}
    )

    if(LIBCOAP_CFLAGS)
      target_compile_options(api_gateway_dynamic PRIVATE ${LIBCOAP_CFLAGS_LIST})
    endif()

    target_link_directories(api_gateway_dynamic PRIVATE ${LIBCOAP_LIBRARY_DIRS})

    target_link_libraries(api_gateway_dynamic PRIVATE 
        ${LIBCOAP_LIBRARIES}
        ${LIBCJSON_LIBRARIES}
    )

else()
    message(FATAL_ERROR "LibCoAP (libcoap-3-openssl) not found by pkg-config. Please check installation and PKG_CONFIG_PATH.")
endif()

# Optional: Install step
# install(TARGETS api_gateway DESTINATION bin) 