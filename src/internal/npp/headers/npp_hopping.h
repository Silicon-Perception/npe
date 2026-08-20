#ifndef NPP_HOPPING_H
#define NPP_HOPPING_H

#include "npp_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * NPP 2.0: Stateless Pipe Frequency Hopping
 *
 * Inspired by HopSync: both sender and receiver independently derive
 * the pipe ID mapping from a shared secret + time epoch.
 *
 * Even if an attacker fully decrypts the frame, they cannot determine
 * which pipe corresponds to which semantic meaning (temperature, humidity, etc.)
 * because the mapping changes every epoch.
 */

#define NPP_HOPPING_KEY_LEN   32
#define NPP_HOPPING_EPOCH_MS  100  /* Default epoch: 100ms */

/* Hopping context (shared by sender and receiver) */
typedef struct {
    uint8_t  secret_key[NPP_HOPPING_KEY_LEN];  /* Shared secret */
    uint32_t pipe_count;                        /* Total logical pipes */
    uint32_t epoch_ms;                           /* Epoch duration in ms */
    uint32_t current_epoch;                      /* Current epoch number */
} npp_hopping_ctx_t;

/*
 * Initialize hopping context with shared secret.
 * Both sides must use the same secret and pipe_count.
 */
int npp_hopping_init(npp_hopping_ctx_t* ctx, const uint8_t* secret,
                       int secret_len, uint32_t pipe_count, uint32_t epoch_ms);

/*
 * Get current epoch based on time.
 * epoch = (current_time_ms / epoch_ms)
 */
uint32_t npp_hopping_get_epoch(const npp_hopping_ctx_t* ctx, uint64_t time_ms);

/*
 * Derive pipe mapping for given epoch.
 * logical_pipe[i] → physical_pipe[mapping[i]]
 *
 * Uses HMAC-SHA256(secret, epoch) as PRNG seed, then Fisher-Yates shuffle.
 */
void npp_hopping_get_mapping(const npp_hopping_ctx_t* ctx, uint32_t epoch,
                                uint16_t* mapping, uint16_t count);

/*
 * Remap a single logical pipe ID to physical pipe ID.
 */
uint16_t npp_hopping_logical_to_physical(const npp_hopping_ctx_t* ctx,
                                            uint32_t epoch, uint16_t logical_id);

/*
 * Remap a single physical pipe ID to logical pipe ID.
 */
uint16_t npp_hopping_physical_to_logical(const npp_hopping_ctx_t* ctx,
                                            uint32_t epoch, uint16_t physical_id);

/*
 * Remap a bitmap from logical to physical space.
 * in_bitmap:  original bitmap in logical pipe order
 * out_bitmap: remapped bitmap in physical pipe order
 * pipe_count: total pipes
 */
int npp_hopping_remap_bitmap(const npp_hopping_ctx_t* ctx, uint32_t epoch,
                                const uint8_t* in_bitmap, uint8_t* out_bitmap,
                                uint16_t pipe_count, uint16_t bitmap_bytes);

/*
 * Reverse remap: physical bitmap → logical bitmap.
 */
int npp_hopping_unmap_bitmap(const npp_hopping_ctx_t* ctx, uint32_t epoch,
                                const uint8_t* in_bitmap, uint8_t* out_bitmap,
                                uint16_t pipe_count, uint16_t bitmap_bytes);

/*
 * Remap pipe IDs in change list.
 * Each entry in change_list is a logical pipe index; this converts to physical.
 */
int npp_hopping_remap_pipes(const npp_hopping_ctx_t* ctx, uint32_t epoch,
                                uint16_t* pipe_ids, uint16_t count);

/*
 * Destroy hopping context (zero out secret).
 */
void npp_hopping_destroy(npp_hopping_ctx_t* ctx);

#ifdef __cplusplus
}
#endif

#endif
