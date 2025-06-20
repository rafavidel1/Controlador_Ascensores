cmake_minimum_required(VERSION 3.10)
project(servidor_central C)

set(CMAKE_C_STANDARD 99)

# --- Find Libraries using pkg-config ---
find_package(PkgConfig REQUIRED)

# Find libCoAP
pkg_check_modules(LIBCOAP REQUIRED libcoap-3-openssl)

# Find cJSON (will be needed for parsing complex request payloads)
pkg_check_modules(CJSON REQUIRED libcjson)

# ---- START DEBUG MESSAGES ----
# message(STATUS "DEBUG: SQLite3 found (via pkg-config): ${SQLITE3_FOUND}")
# message(STATUS "DEBUG: SQLite3 include directories (via pkg-config): ${SQLITE3_INCLUDE_DIRS}")
# message(STATUS "DEBUG: SQLite3 libraries (via pkg-config): ${SQLITE3_LIBRARIES}")
message(STATUS "DEBUG: cJSON found (via pkg-config): ${CJSON_FOUND}")
message(STATUS "DEBUG: cJSON include directories (via pkg-config): ${CJSON_INCLUDE_DIRS}")
message(STATUS "DEBUG: cJSON libraries (via pkg-config): ${CJSON_LIBRARIES}")
# ---- END DEBUG MESSAGES ----

# Include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${LIBCOAP_INCLUDE_DIRS}
    # ${SQLITE3_INCLUDE_DIRS} # Remove SQLite3 include directory
    ${CJSON_INCLUDE_DIRS}   # Keep cJSON include directory
)

# Link directories (pkg-config usually handles this)
link_directories(
    ${LIBCOAP_LIBRARY_DIRS}
    # ${SQLITE3_LIBRARY_DIRS} # Remove SQLite3 library directory
    ${CJSON_LIBRARY_DIRS}
)

# Add executable
add_executable(servidor_central
    src/main.c
    # src/database_manager.c # Removed
)

# Link libraries
target_link_libraries(servidor_central
    PRIVATE
    ${LIBCOAP_LIBRARIES}
    # ${SQLITE3_LIBRARIES}   # Remove SQLite3
    ${CJSON_LIBRARIES}      # Keep cJSON
)

# Installation (optional, but good practice)
install(TARGETS servidor_central DESTINATION bin) 