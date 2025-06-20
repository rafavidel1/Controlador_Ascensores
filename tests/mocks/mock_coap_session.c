#include "mock_coap_session.h"
#include <stdlib.h>
#include <string.h>

// Variables globales para el mock
static mock_coap_session_t mock_sessions[MAX_MOCK_SESSIONS];
static int mock_session_count = 0;
static bool mock_session_should_fail = false;

// Implementación del mock
coap_session_t* mock_coap_new_client_session_psk(
    coap_context_t *ctx,
    const coap_address_t *local_if,
    const coap_address_t *server,
    coap_proto_t proto,
    const char *identity,
    const uint8_t *key,
    unsigned key_len) {
    
    if (mock_session_should_fail) {
        return NULL;
    }
    
    if (mock_session_count >= MAX_MOCK_SESSIONS) {
        return NULL;
    }
    
    mock_coap_session_t *mock_session = &mock_sessions[mock_session_count++];
    mock_session->session_id = mock_session_count;
    mock_session->is_established = true;
    mock_session->proto = proto;
    
    if (identity) {
        strncpy(mock_session->identity, identity, sizeof(mock_session->identity) - 1);
        mock_session->identity[sizeof(mock_session->identity) - 1] = '\0';
    }
    
    if (key && key_len > 0) {
        mock_session->key_len = key_len < sizeof(mock_session->key) ? key_len : sizeof(mock_session->key);
        memcpy(mock_session->key, key, mock_session->key_len);
    }
    
    // Retornar puntero mock (cast del ID)
    return (coap_session_t*)(uintptr_t)mock_session->session_id;
}

coap_session_state_t mock_coap_session_get_state(const coap_session_t *session) {
    int session_id = (int)(uintptr_t)session;
    
    for (int i = 0; i < mock_session_count; i++) {
        if (mock_sessions[i].session_id == session_id) {
            return mock_sessions[i].is_established ? 
                COAP_SESSION_STATE_ESTABLISHED : COAP_SESSION_STATE_NONE;
        }
    }
    
    return COAP_SESSION_STATE_NONE;
}

void mock_coap_session_release(coap_session_t *session) {
    int session_id = (int)(uintptr_t)session;
    
    for (int i = 0; i < mock_session_count; i++) {
        if (mock_sessions[i].session_id == session_id) {
            mock_sessions[i].is_established = false;
            break;
        }
    }
}

// Funciones de control del mock
void mock_coap_session_reset(void) {
    memset(mock_sessions, 0, sizeof(mock_sessions));
    mock_session_count = 0;
    mock_session_should_fail = false;
}

void mock_coap_session_set_fail_mode(bool should_fail) {
    mock_session_should_fail = should_fail;
}

mock_coap_session_t* mock_coap_session_get_info(int session_id) {
    for (int i = 0; i < mock_session_count; i++) {
        if (mock_sessions[i].session_id == session_id) {
            return &mock_sessions[i];
        }
    }
    return NULL;
}

int mock_coap_session_get_count(void) {
    return mock_session_count;
} 