#ifndef NPP_SESSION_H
#define NPP_SESSION_H

#include "npp_types.h"
#include "npp_schema.h"
#include "npp_backend.h"
#include "npp_crypto.h"
#include "npp_transport.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Session state */
typedef enum {
    NPP_SESSION_INIT       = 0,
    NPP_SESSION_CONNECTING = 1,
    NPP_SESSION_READY      = 2,
    NPP_SESSION_SYNCING    = 3,
    NPP_SESSION_ACTIVE     = 4,
    NPP_SESSION_ERROR      = 5,
    NPP_SESSION_CLOSED     = 6,
} npp_session_state_t;

/* Session configuration */
typedef struct {
    const char*            remote_addr;
    uint16_t               remote_port;
    npp_transport_type_t   transport_type;
    npp_backend_type_t     backend_type;
    npp_crypto_type_t      crypto_type;
    const uint8_t*         crypto_key;
    int                    crypto_key_len;
    uint32_t               health_check_interval;
    uint32_t               sync_cooldown_ms;
} npp_session_config_t;

/* Session structure */
struct npp_session_s {
    npp_session_state_t  state;
    npp_schema_t*        schema;
    npp_backend_t*       backend;
    npp_transport_t*     transport;
    npp_crypto_ctx_t     crypto;
    
    /* Frame management */
    uint32_t             local_frame_id;
    uint32_t             remote_frame_id;
    uint32_t             last_sync_frame;
    
    /* Health */
    uint8_t              needs_sync;
    uint32_t             last_recv_time;
    uint32_t             consecutive_timeouts;
    
    /* Buffers */
    uint8_t*             recv_buffer;
    uint32_t             recv_buffer_size;
    uint8_t*             send_buffer;
    uint32_t             send_buffer_size;
    uint8_t*             sync_frame;
    uint32_t             sync_frame_size;
    
    /* Config */
    npp_session_config_t  config;
};

/* Session lifecycle */
npp_session_t* npp_session_create(const npp_schema_t* schema,
                                    const npp_session_config_t* config);
void           npp_session_destroy(npp_session_t* session);

/* Session connection */
int  npp_session_connect(npp_session_t* session);
void npp_session_close(npp_session_t* session);

/* Data operations */
int  npp_session_tick(npp_session_t* session, const void* input, 
                       npp_output_t* output);
int  npp_session_apply(npp_session_t* session, const uint8_t* frame,
                        uint32_t len);
int  npp_session_sync(npp_session_t* session);

/* Receive loop (non-blocking) */
int  npp_session_recv(npp_session_t* session);

/* Health check */
int  npp_session_health_check(npp_session_t* session);

/* Getters */
npp_session_state_t npp_session_get_state(const npp_session_t* session);
uint32_t            npp_session_get_frame_id(const npp_session_t* session);

/* Event loop (blocking, for server) */
int  npp_session_run(npp_session_t* session);

#ifdef __cplusplus
}
#endif

#endif