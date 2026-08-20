#ifndef NPP_TRANSPORT_H
#define NPP_TRANSPORT_H

#include "npp_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Transport interface */
typedef struct {
    int  (*connect)(void* ctx, const char* addr, uint16_t port);
    int  (*send)(void* ctx, const uint8_t* data, uint32_t len);
    int  (*recv)(void* ctx, uint8_t* buf, uint32_t max_len, uint32_t* recv_len);
    void (*close)(void* ctx);
    int  (*get_fd)(void* ctx);
} npp_transport_ops_t;

/* Transport structure */
typedef struct {
    npp_transport_type_t type;
    npp_transport_ops_t ops;
    void*                internal;
} npp_transport_t;

/* Transport factory */
npp_transport_t* npp_transport_create(npp_transport_type_t type);
void             npp_transport_destroy(npp_transport_t* transport);

/* Convenience functions */
int npp_transport_connect(npp_transport_t* t, const char* addr, uint16_t port);
int npp_transport_send(npp_transport_t* t, const uint8_t* data, uint32_t len);
int npp_transport_recv(npp_transport_t* t, uint8_t* buf, uint32_t max_len, 
                        uint32_t* recv_len);
void npp_transport_close(npp_transport_t* t);

#ifdef __cplusplus
}
#endif

#endif