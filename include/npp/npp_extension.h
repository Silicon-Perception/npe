#ifndef NPP_EXTENSION_H
#define NPP_EXTENSION_H

#include "npp_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * NPP 2.0: Schema Extension (Base + Overlay model)
 *
 * Base Schema is immutable (schema_id locked at creation).
 * Extensions can be dynamically registered at runtime:
 * - Each extension has its own extension_id + version
 * - Extension pipes are appended after base pipes
 * - Old clients ignore unknown extensions (forward compatible)
 *
 * Frame header's extension_mask indicates which extensions are active.
 */

#define NPP_MAX_EXTENSIONS 32

/* Extension definition */
typedef struct {
    uint16_t extension_id;      /* Unique extension ID */
    uint16_t version;           /* Extension version */
    uint16_t pipe_offset;       /* Where extension pipes start (base_count + ...) */
    uint16_t pipe_count;        /* How many pipes this extension adds */
    char     name[32];          /* Extension name */
    uint8_t  active;            /* Is this extension currently active? */
} npp_extension_def_t;

/* Extension manager */
typedef struct {
    uint16_t base_pipe_count;   /* Base schema pipe count (immutable) */
    uint32_t base_schema_id;    /* Base schema ID (immutable) */
    npp_extension_def_t extensions[NPP_MAX_EXTENSIONS];
    uint16_t extension_count;
    uint16_t total_pipe_count;  /* base + all active extensions */
} npp_extension_ctx_t;

/*
 * Initialize extension context with base schema.
 */
int npp_extension_init(npp_extension_ctx_t* ctx, uint16_t base_pipe_count,
                         uint32_t base_schema_id);

/*
 * Register a new extension.
 * pipe_offset is auto-calculated (base_count + sum of previous extensions).
 */
int npp_extension_register(npp_extension_ctx_t* ctx,
                             uint16_t extension_id, uint16_t version,
                             uint16_t pipe_count, const char* name);

/*
 * Unregister an extension (pipes become inactive).
 */
int npp_extension_unregister(npp_extension_ctx_t* ctx, uint16_t extension_id);

/*
 * Get extension by ID.
 */
const npp_extension_def_t* npp_extension_get(const npp_extension_ctx_t* ctx,
                                                uint16_t extension_id);

/*
 * Get total pipe count (base + active extensions).
 */
uint16_t npp_extension_total_pipes(const npp_extension_ctx_t* ctx);

/*
 * Check if a pipe ID belongs to an extension.
 * Returns extension index, or -1 if base pipe.
 */
int npp_extension_find_pipe(const npp_extension_ctx_t* ctx, uint16_t pipe_id);

/*
 * Get extension mask (bitmap of active extension IDs).
 */
uint32_t npp_extension_get_mask(const npp_extension_ctx_t* ctx);

/*
 * Get list of active extensions.
 */
uint16_t npp_extension_get_list(const npp_extension_ctx_t* ctx,
                                   npp_extension_def_t* out_list, uint16_t max_count);

#ifdef __cplusplus
}
#endif

#endif
