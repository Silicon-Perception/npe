#ifndef NPP_SUBSCRIPTION_H
#define NPP_SUBSCRIPTION_H

#include "npp_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * NPP 2.0: Region Subscription Broadcast
 *
 * Inspired by NDN's content-based forwarding.
 * Each client subscribes to a range of pipes (a "region").
 * Server only sends changes for pipes the client cares about.
 *
 * This requires NPE's topology knowledge (which pipes belong to which region).
 */

#define NPP_MAX_SUBSCRIPTIONS 16

/* Subscription region */
typedef struct {
    uint16_t region_id;
    uint16_t start_pipe;    /* First pipe in region */
    uint16_t end_pipe;      /* Last pipe (inclusive) */
    uint8_t  priority;      /* Subscription priority (higher = more important) */
    uint8_t  active;        /* Is this subscription active? */
} npp_subscription_t;

/* Client subscription manager */
typedef struct {
    uint32_t client_id;
    npp_subscription_t subs[NPP_MAX_SUBSCRIPTIONS];
    uint8_t  sub_count;
    uint8_t  subscribe_all;  /* If 1, receive all pipes */
} npp_client_subscription_t;

/*
 * Initialize client subscription manager.
 */
int npp_subscriber_init(npp_client_subscription_t* sub, uint32_t client_id);

/*
 * Add a region subscription.
 */
int npp_subscriber_add_region(npp_client_subscription_t* sub,
                                uint16_t region_id, uint16_t start, uint16_t end,
                                uint8_t priority);

/*
 * Remove a region subscription.
 */
int npp_subscriber_remove_region(npp_client_subscription_t* sub,
                                   uint16_t region_id);

/*
 * Check if a client is interested in a specific pipe.
 */
int npp_subscriber_interested_in(const npp_client_subscription_t* sub,
                                    uint16_t pipe_id);

/*
 * Filter a bitmap: keep only bits for pipes the client is interested in.
 * in_bitmap:  original change bitmap
 * out_bitmap: filtered bitmap (only subscribed pipes)
 * pipe_count: total pipes
 * Returns number of pipes the client is interested in that changed.
 */
uint16_t npp_subscriber_filter_bitmap(const npp_client_subscription_t* sub,
                                         const uint8_t* in_bitmap,
                                         uint8_t* out_bitmap,
                                         uint16_t pipe_count);

/*
 * Set "subscribe all" mode.
 */
int npp_subscriber_set_all(npp_client_subscription_t* sub, uint8_t enable);

/*
 * Get subscription count.
 */
uint16_t npp_subscriber_get_count(const npp_client_subscription_t* sub);

#ifdef __cplusplus
}
#endif

#endif
