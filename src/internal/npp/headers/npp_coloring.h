#ifndef NPP_COLORING_H
#define NPP_COLORING_H

#include "npp_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * NPP 2.0: Pipe Coloring (Bypass Tagging)
 *
 * Inspired by physical bypass pipes that "dye" the water flowing through them.
 * Coloring pipes attach semantic tags to data without modifying values.
 *
 * Color types:
 * - PRIORITY: mark pipes with QoS level (affects transmission order)
 * - SECURITY: mark pipes requiring stronger encryption
 * - AUDIT:    mark pipes whose changes must be logged
 * - ROUTE:    mark pipes with routing directives
 */

#define NPP_MAX_COLORS 256

/* Color assignment */
typedef struct {
    uint16_t pipe_id;
    uint8_t  color_tag;    /* npp_color_tag_t */
    uint8_t  color_value;  /* Tag-specific value (e.g. priority level 0-3) */
} npp_color_assign_t;

/* Color context */
typedef struct {
    npp_color_assign_t colors[NPP_MAX_COLORS];
    uint16_t color_count;
} npp_coloring_ctx_t;

/*
 * Initialize coloring context.
 */
int npp_coloring_init(npp_coloring_ctx_t* ctx);

/*
 * Assign a color to a pipe.
 */
int npp_coloring_assign(npp_coloring_ctx_t* ctx, uint16_t pipe_id,
                          uint8_t color_tag, uint8_t color_value);

/*
 * Remove color from a pipe.
 */
int npp_coloring_remove(npp_coloring_ctx_t* ctx, uint16_t pipe_id);

/*
 * Get the color of a pipe.
 * Returns NPP_COLOR_NONE if pipe has no color.
 */
uint8_t npp_coloring_get_tag(const npp_coloring_ctx_t* ctx, uint16_t pipe_id);

/*
 * Get color value for a pipe.
 */
uint8_t npp_coloring_get_value(const npp_coloring_ctx_t* ctx, uint16_t pipe_id);

/*
 * Get all pipes with a specific color tag.
 * Returns count, fills out_ids with pipe IDs.
 */
uint16_t npp_coloring_get_pipes_by_tag(const npp_coloring_ctx_t* ctx,
                                            uint8_t color_tag,
                                            uint16_t* out_ids, uint16_t max_count);

/*
 * Get priority for a pipe (convenience function).
 * Returns color_value if tagged with PRIORITY, else 0.
 */
uint8_t npp_coloring_get_priority(const npp_coloring_ctx_t* ctx, uint16_t pipe_id);

/*
 * Check if a pipe requires audit logging.
 */
int npp_coloring_needs_audit(const npp_coloring_ctx_t* ctx, uint16_t pipe_id);

/*
 * Check if a pipe requires strong encryption.
 */
int npp_coloring_needs_strong_crypto(const npp_coloring_ctx_t* ctx, uint16_t pipe_id);

#ifdef __cplusplus
}
#endif

#endif
