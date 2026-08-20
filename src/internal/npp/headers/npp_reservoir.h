#ifndef NPP_RESERVOIR_H
#define NPP_RESERVOIR_H

#include "npp_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * NPP 2.0: Reservoir (Batch Delivery)
 *
 * Inspired by the physical reservoir: water accumulates until full,
 * then the gate opens and all water is released at once.
 *
 * Benefits:
 * - Multiple pipe changes merged into one frame → header overhead reduced
 * - Static periods → reservoir stays empty → zero transmission
 *   (works with NPE's block mechanism)
 *
 * Three trigger modes:
 * 1. Water Volume: trigger when accumulated changes >= capacity
 * 2. Water Level:  trigger when a watched "critical" pipe changes
 * 3. Timer:        trigger on time interval (periodic reporting)
 *
 * The "water level" itself is a computation result:
 * - level = total_inflow - total_outflow (pending changes)
 * - pressure = d(level)/dt (rate of change)
 * - Water hammer detection: sudden pressure spike = anomaly
 */

#define NPP_RESERVOIR_MAX_CHANGES 4096

/* Single pipe change record */
typedef struct {
    uint16_t pipe_id;       /* Which pipe changed */
    uint8_t  data[NPP_MAX_PIPES > 256 ? 8 : 8]; /* New value (double=8 bytes) */
    uint32_t timestamp;     /* When the change was recorded */
} npp_reservoir_entry_t;

/* Reservoir context */
typedef struct {
    npp_reservoir_entry_t entries[NPP_RESERVOIR_MAX_CHANGES];
    uint32_t count;                    /* Current water level (pending changes) */
    uint32_t capacity;                 /* Max entries before volume trigger */

    uint32_t total_inflow;            /* Total changes ever recorded */
    uint32_t total_outflow;           /* Total changes ever flushed */
    double   prev_level;              /* Previous water level (for pressure calc) */
    double   pressure;               /* Current pressure = d(level)/dt */

    npp_reservoir_mode_t mode;        /* Primary trigger mode */
    uint16_t watch_pipe;             /* Watched pipe for water-level trigger */
    uint32_t flush_interval_ms;      /* Timer trigger interval */
    uint32_t last_flush_time;        /* Last flush timestamp */

    uint32_t flush_count;            /* Number of flushes performed */
    double   avg_flush_size;         /* Average entries per flush */
} npp_reservoir_t;

/*
 * Initialize reservoir.
 * capacity: max changes before auto-flush (water volume mode)
 * mode: primary trigger mode
 */
int npp_reservoir_init(npp_reservoir_t* res, uint32_t capacity,
                         npp_reservoir_mode_t mode);

/*
 * Add a pipe change to the reservoir.
 * Returns NPP_OK, or NPP_ERR_RESERVOIR if reservoir is full (should flush).
 */
int npp_reservoir_add(npp_reservoir_t* res, uint16_t pipe_id,
                        const void* value, uint32_t timestamp);

/*
 * Check if reservoir should flush.
 * Returns 1 if flush needed, 0 if not.
 */
int npp_reservoir_should_flush(const npp_reservoir_t* res, uint32_t now);

/*
 * Flush: extract all accumulated changes.
 * Copies entries to out_buf, clears reservoir, updates statistics.
 * Returns number of entries flushed.
 */
uint32_t npp_reservoir_flush(npp_reservoir_t* res,
                                npp_reservoir_entry_t* out_buf,
                                uint32_t out_capacity, uint32_t now);

/*
 * Get current water level (pending change count).
 */
static inline double npp_reservoir_get_level(const npp_reservoir_t* res) {
    return res ? (double)res->count : 0.0;
}

/*
 * Get current pressure (rate of change).
 */
static inline double npp_reservoir_get_pressure(const npp_reservoir_t* res) {
    return res ? res->pressure : 0.0;
}

/*
 * Detect water hammer: sudden pressure spike.
 * threshold: pressure level that indicates hammer.
 * Returns 1 if hammer detected, 0 otherwise.
 */
int npp_reservoir_detect_hammer(const npp_reservoir_t* res, double threshold);

/*
 * Update pressure based on time delta.
 */
void npp_reservoir_update_pressure(npp_reservoir_t* res, uint32_t now);

/*
 * Reset reservoir (clear all entries without flushing).
 */
void npp_reservoir_reset(npp_reservoir_t* res);

/*
 * Get statistics.
 */
typedef struct {
    uint32_t total_inflow;
    uint32_t total_outflow;
    uint32_t flush_count;
    double   avg_flush_size;
    double   current_level;
    double   current_pressure;
} npp_reservoir_stats_t;

void npp_reservoir_get_stats(const npp_reservoir_t* res,
                                npp_reservoir_stats_t* stats);

#ifdef __cplusplus
}
#endif

#endif
