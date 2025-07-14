cmake_minimum_required(VERSION 3.10)
project(ApiGateway C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED True)

# Find LibCoAP using pkg-config
find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBCOAP REQUIRED libcoap-3-openssl)

# Find cJSON using pkg-config
pkg_check_modules(LIBCJSON REQUIRED libcjson)

# Dotenv-c - Buscar en ubicaciones posibles
set(DOTENV_SRC)
set(DOTENV_INC)

# Buscar dotenv-c en ubicaciones estándar
find_path(DOTENV_ROOT_DIR
    NAMES "src/dotenv.c"
    PATHS
        "../dotenv-c"
        "../../dotenv-c"
        "${CMAKE_SOURCE_DIR}/../dotenv-c"
        "${CMAKE_SOURCE_DIR}/../../dotenv-c"
        "/usr/local/include"
        "/usr/include"
    DOC "Directorio raíz de dotenv-c"
)

# Si encontramos dotenv-c localmente, usar esos archivos
if(DOTENV_ROOT_DIR AND EXISTS "${DOTENV_ROOT_DIR}/src/dotenv.c")
    set(DOTENV_SRC "${DOTENV_ROOT_DIR}/src/dotenv.c")
    set(DOTENV_INC "${DOTENV_ROOT_DIR}/src")
    message(STATUS "Usando dotenv-c local: ${DOTENV_ROOT_DIR}")
# Si no, verificar si está instalado en el sistema
elseif(EXISTS "/usr/local/include/dotenv.h")
    # dotenv-c instalado en sistema - no necesitamos archivos fuente
    set(DOTENV_SRC)
    set(DOTENV_INC "/usr/local/include")
    message(STATUS "Usando dotenv-c del sistema: /usr/local/")
elseif(EXISTS "/usr/include/dotenv.h")
    set(DOTENV_SRC)
    set(DOTENV_INC "/usr/include")
    message(STATUS "Usando dotenv-c del sistema: /usr/")
else()
    message(FATAL_ERROR "No se encontró dotenv-c. Instala con 'sudo apt-get install libdotenv-dev' o ejecuta build_api_gateway.sh")
endif()

# Source files for the executable
set(API_GATEWAY_SOURCES
    src/main.c
    src/api_handlers.c
    src/elevator_state_manager.c
    src/can_bridge.c
    src/mi_simulador_ascensor.c
    src/simulation_loader.c
    src/execution_logger.c
    src/psk_manager.c
)

# Add the executable
if(DOTENV_SRC)
    add_executable(api_gateway ${API_GATEWAY_SOURCES} ${DOTENV_SRC})
else()
    add_executable(api_gateway ${API_GATEWAY_SOURCES})
endif()

# Crear ejecutable adicional con puerto dinámico para pruebas de carga
if(DOTENV_SRC)
    add_executable(api_gateway_dynamic
        src/main_dynamic_port.c
        src/api_handlers.c
        src/can_bridge.c
        src/elevator_state_manager.c
        src/mi_simulador_ascensor.c
        src/simulation_loader.c
        src/execution_logger.c
        ${DOTENV_SRC}
    )
else()
    add_executable(api_gateway_dynamic
        src/main_dynamic_port.c
        src/api_handlers.c
        src/can_bridge.c
        src/elevator_state_manager.c
        src/mi_simulador_ascensor.c
        src/simulation_loader.c
        src/execution_logger.c
    )
endif()

# Configure include directories and link libraries for the target
if(LIBCOAP_FOUND)
    message(STATUS "Found LibCoAP: YES")
    message(STATUS "LibCoAP include directories: ${LIBCOAP_INCLUDE_DIRS}")
    message(STATUS "LibCoAP library directories: ${LIBCOAP_LIBRARY_DIRS}")
    message(STATUS "LibCoAP libraries: ${LIBCOAP_LIBRARIES}")
    message(STATUS "LibCoAP CFLAGS: ${LIBCOAP_CFLAGS}")
    message(STATUS "LibCoAP LDFLAGS: ${LIBCOAP_LDFLAGS}")

    # Configuración para api_gateway original
    if(DOTENV_INC)
        target_include_directories(api_gateway PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/include
            ${LIBCOAP_INCLUDE_DIRS}
            ${LIBCJSON_INCLUDE_DIRS}
            ${DOTENV_INC}
        )
    else()
        target_include_directories(api_gateway PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/include
            ${LIBCOAP_INCLUDE_DIRS}
            ${LIBCJSON_INCLUDE_DIRS}
        )
    endif()

    if(LIBCOAP_CFLAGS)
      string(REPLACE " " ";" LIBCOAP_CFLAGS_LIST ${LIBCOAP_CFLAGS})
      target_compile_options(api_gateway PRIVATE ${LIBCOAP_CFLAGS_LIST})
    endif()

    # Add library search paths for the target
    target_link_directories(api_gateway PRIVATE ${LIBCOAP_LIBRARY_DIRS})
    
    # Link the api_gateway executable against LibCoAP libraries
    if(NOT DOTENV_SRC)
        # Si dotenv está instalado en sistema, linkear la librería
        target_link_libraries(api_gateway PRIVATE 
            ${LIBCOAP_LIBRARIES}
            ${LIBCJSON_LIBRARIES}
            dotenv
        )
    else()
        target_link_libraries(api_gateway PRIVATE 
            ${LIBCOAP_LIBRARIES}
            ${LIBCJSON_LIBRARIES}
        )
    endif()

    # Configuración para api_gateway_dynamic
    if(DOTENV_INC)
        target_include_directories(api_gateway_dynamic PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/include
            ${LIBCOAP_INCLUDE_DIRS}
            ${LIBCJSON_INCLUDE_DIRS}
            ${DOTENV_INC}
        )
    else()
        target_include_directories(api_gateway_dynamic PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/include
            ${LIBCOAP_INCLUDE_DIRS}
            ${LIBCJSON_INCLUDE_DIRS}
        )
    endif()

    if(LIBCOAP_CFLAGS)
      target_compile_options(api_gateway_dynamic PRIVATE ${LIBCOAP_CFLAGS_LIST})
    endif()

    target_link_directories(api_gateway_dynamic PRIVATE ${LIBCOAP_LIBRARY_DIRS})

    if(NOT DOTENV_SRC)
        # Si dotenv está instalado en sistema, linkear la librería
        target_link_libraries(api_gateway_dynamic PRIVATE 
            ${LIBCOAP_LIBRARIES}
            ${LIBCJSON_LIBRARIES}
            dotenv
        )
    else()
        target_link_libraries(api_gateway_dynamic PRIVATE 
            ${LIBCOAP_LIBRARIES}
            ${LIBCJSON_LIBRARIES}
        )
    endif()

    add_custom_command(TARGET api_gateway POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_CURRENT_SOURCE_DIR}/gateway.env
        $<TARGET_FILE_DIR:api_gateway>/gateway.env
    )

    add_custom_command(TARGET api_gateway POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_CURRENT_SOURCE_DIR}/psk_keys.txt
        $<TARGET_FILE_DIR:api_gateway>/psk_keys.txt
    )

    add_custom_command(TARGET api_gateway POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_CURRENT_SOURCE_DIR}/simulation_data.json
        $<TARGET_FILE_DIR:api_gateway>/simulation_data.json
    )

    add_custom_command(TARGET api_gateway_dynamic POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_CURRENT_SOURCE_DIR}/gateway.env
        $<TARGET_FILE_DIR:api_gateway_dynamic>/gateway.env
    )

    add_custom_command(TARGET api_gateway_dynamic POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_CURRENT_SOURCE_DIR}/psk_keys.txt
        $<TARGET_FILE_DIR:api_gateway_dynamic>/psk_keys.txt
    )

    add_custom_command(TARGET api_gateway_dynamic POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_CURRENT_SOURCE_DIR}/simulation_data.json
        $<TARGET_FILE_DIR:api_gateway_dynamic>/simulation_data.json
    )

else()
    message(FATAL_ERROR "LibCoAP (libcoap-3-openssl) not found by pkg-config. Please check installation and PKG_CONFIG_PATH.")
endif()

# Optional: Install step
# install(TARGETS api_gateway DESTINATION bin) 