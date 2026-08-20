#ifndef NPP_SERVER_H
#define NPP_SERVER_H

#include "npp_types.h"
#include "npp_schema.h"
#include "npp_backend.h"
#include "npp_transport.h"
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#ifdef __cplusplus
extern "C" {
#endif

#define NPP_CLIENT_SEND_BUFFER (16 * 1024)

/* Client info */
typedef struct {
    uint32_t client_id;
    uint32_t last_frame_id;
    uint8_t  needs_sync;
    int      sock_fd;
    uint8_t  active;
    struct sockaddr_in addr;
    uint8_t  send_buffer[NPP_CLIENT_SEND_BUFFER];
    uint32_t send_buf_len;
    uint32_t last_activity;
    uint32_t frames_sent;
    uint8_t  priority;
} npp_client_info_t;

/* Server configuration */
typedef struct {
    const char*          listen_addr;
    uint16_t             listen_port;
    npp_transport_type_t transport_type;
    npp_backend_type_t   backend_type;
    uint16_t             max_clients;
    uint32_t             sync_cooldown_ms;
} npp_server_config_t;

/* Server structure */
struct npp_server_s {
    npp_schema_t*        schema;
    npp_backend_t*       backend;
    npp_transport_t*     transport;

    /* Client management */
    npp_client_info_t   clients[NPP_MAX_CLIENTS];
    uint16_t             client_count;
    uint16_t             max_clients;
    uint32_t             next_client_id;

    /* Frame management */
    uint32_t             frame_id;
    uint8_t*             sync_frame;
    uint32_t             sync_frame_size;

    /* Config */
    npp_server_config_t  config;
    int                  listen_fd;
    uint8_t              running;

    /* Multicast frame cache */
    uint8_t*             multicast_frame;
    uint32_t             multicast_frame_len;

    /* Event loop */
    pthread_t            event_thread;
    uint8_t              event_loop_running;

    /* Thread safety */
    pthread_mutex_t      lock;
};

/* Server lifecycle */
npp_server_t* npp_server_create(const npp_schema_t* schema,
                                 const npp_server_config_t* config);
void          npp_server_destroy(npp_server_t* server);

/* Server operations */
int  npp_server_start(npp_server_t* server);
void npp_server_stop(npp_server_t* server);

/* Broadcast: one Diff, N sends */
int  npp_server_broadcast(npp_server_t* server, const uint8_t* frame,
                           uint32_t frame_len);

/* Client management */
uint32_t npp_server_accept_client(npp_server_t* server);
int      npp_server_disconnect_client(npp_server_t* server, uint32_t client_id);

/* Get active client count */
uint32_t npp_server_get_active_clients(const npp_server_t* server);

/* Extended server API */
int  npp_server_run(npp_server_t* server);
void npp_server_stop_loop(npp_server_t* server);
int  npp_server_set_client_priority(npp_server_t* server, uint32_t client_id, uint8_t priority);
int  npp_server_get_client_info(npp_server_t* server, uint32_t client_id,
                                uint32_t* frames_sent, uint32_t* last_active);
const uint8_t* npp_server_get_multicast_frame(npp_server_t* server, uint32_t* out_len);
int  npp_server_kick_idle_clients(npp_server_t* server, uint32_t max_idle_sec);
int  npp_server_publish(npp_server_t* server, npp_schema_t* schema,
                        const double* ctrl_values, const double* data_values,
                        uint16_t ctrl_count, uint16_t data_count,
                        const uint8_t* changed_ctrl, const uint8_t* changed_data);

#ifdef __cplusplus
}
#endif

#endif