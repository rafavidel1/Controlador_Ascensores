#ifndef MOCK_COAP_SESSION_H
#define MOCK_COAP_SESSION_H

#include <coap3/coap.h>
#include <stdbool.h>
#include <stdint.h>

#define MAX_MOCK_SESSIONS 10

// Estructura para simular sesiones CoAP
typedef struct {
    int session_id;
    bool is_established;
    coap_proto_t proto;
    char identity[64];
    uint8_t key[64];
    unsigned key_len;
} mock_coap_session_t;

// Funciones mock que reemplazan las de libcoap
coap_session_t* mock_coap_new_client_session_psk(
    coap_context_t *ctx,
    const coap_address_t *local_if,
    const coap_address_t *server,
    coap_proto_t proto,
    const char *identity,
    const uint8_t *key,
    unsigned key_len);

coap_session_state_t mock_coap_session_get_state(const coap_session_t *session);
void mock_coap_session_release(coap_session_t *session);

// Funciones de control del mock
void mock_coap_session_reset(void);
void mock_coap_session_set_fail_mode(bool should_fail);
mock_coap_session_t* mock_coap_session_get_info(int session_id);
int mock_coap_session_get_count(void);

// Macros para reemplazar funciones reales en tests
#ifdef TESTING
#define coap_new_client_session_psk mock_coap_new_client_session_psk
#define coap_session_get_state mock_coap_session_get_state
#define coap_session_release mock_coap_session_release
#endif

#endif // MOCK_COAP_SESSION_H 