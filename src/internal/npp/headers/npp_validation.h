#ifndef NPP_VALIDATION_H
#define NPP_VALIDATION_H

#include "npp_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * NPP 2.0: Pipe Validation
 *
 * Four-layer validation:
 * L1: CRC32 (frame-level, in npp_frame2.c)
 * L2: Cross-pipe consistency (pipe A = f(pipe B))
 * L3: Topology consistency (neighbor similarity)
 * L4: Silent validation (NPP-unique: verify "unchanged" pipes)
 *
 * L1 is implemented in npp_frame2.c (npp_crc32).
 * This module implements L2, L3, L4.
 */

/* L2: Cross-pipe consistency check */
typedef struct {
    uint16_t source_pipe;     /* The pipe whose value is a function of others */
    uint16_t ref_pipe_a;      /* First reference pipe */
    uint16_t ref_pipe_b;      /* Second reference pipe (0xFFFF = unused) */
    uint8_t  check_type;      /* 0=sum, 1=diff, 2=avg, 3=custom */
    double   tolerance;       /* Acceptable deviation */
} npp_cross_check_t;

typedef struct {
    npp_cross_check_t* checks;
    uint16_t check_count;
} npp_validation_ctx_t;

/*
 * Initialize validation context with cross-pipe checks.
 */
int npp_validation_init(npp_validation_ctx_t* ctx,
                         const npp_cross_check_t* checks, uint16_t count);

/*
 * L2: Run cross-pipe consistency checks.
 * values: array of all pipe values (double[])
 * count: number of pipes
 * Returns NPP_OK if all pass, NPP_ERR_VALIDATION if any fail.
 * failed_pipe: set to the first failing pipe index (or 0xFFFF if none).
 */
int npp_validation_check_cross(const npp_validation_ctx_t* ctx,
                                const double* values, uint16_t count,
                                uint16_t* failed_pipe);

/*
 * L3: Topology consistency check.
 * For grid layouts, check that each pipe's value is within k*stddev
 * of its neighbors.
 *
 * values:   array of all pipe values
 * width:    grid width
 * height:   grid height
 * k:        number of stddevs for threshold
 * suspect_bitmap: output bitmap marking suspicious pipes (1=suspect)
 * Returns number of suspicious pipes found.
 */
uint16_t npp_validation_check_topology(const double* values,
                                         uint16_t width, uint16_t height,
                                         double k,
                                         uint8_t* suspect_bitmap);

/*
 * L4: Silent validation (NPP-unique).
 * Compare local state against a received SYNC frame.
 * Any pipe where local version > remote version means we're ahead.
 * Any pipe where local version < remote version means we're behind.
 * Both indicate potential issues.
 *
 * local_values:   our current pipe values
 * remote_values:  values from SYNC frame
 * local_versions: our version numbers
 * remote_versions: versions from SYNC frame
 * count: number of pipes
 *
 * Returns NPP_OK if states match, NPP_ERR_VALIDATION if mismatch.
 * mismatch_count: set to number of mismatched pipes.
 */
int npp_validation_check_silent(const double* local_values,
                                  const double* remote_values,
                                  const uint32_t* local_versions,
                                  const uint32_t* remote_versions,
                                  uint16_t count,
                                  uint16_t* mismatch_count);

/*
 * CRDT merge: merge remote state into local.
 * For each pipe: if remote version > local version, accept remote value.
 *
 * local_values:   our values (will be updated in-place)
 * remote_values:  incoming values
 * local_versions: our versions (will be updated in-place)
 * remote_versions: incoming versions
 * count: number of pipes
 *
 * Returns number of pipes that were updated.
 */
uint16_t npp_validation_crdt_merge(double* local_values,
                                      const double* remote_values,
                                      uint32_t* local_versions,
                                      const uint32_t* remote_versions,
                                      uint16_t count);

/*
 * Destroy validation context.
 */
void npp_validation_destroy(npp_validation_ctx_t* ctx);

#ifdef __cplusplus
}
#endif

#endif
