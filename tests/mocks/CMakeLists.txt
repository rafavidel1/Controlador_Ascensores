# Biblioteca de mocks para testing
add_library(test_mocks STATIC
    mock_coap_session.c
    mock_can_interface.c
    mock_execution_logger.c
)

target_include_directories(test_mocks PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../../api_gateway/include
    ${LIBCOAP_INCLUDE_DIRS}
    ${LIBCJSON_INCLUDE_DIRS}
)

target_link_libraries(test_mocks
    ${LIBCOAP_LIBRARIES}
    ${LIBCJSON_LIBRARIES}
) 