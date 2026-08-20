#ifndef NPP_TYPES_H
#define NPP_TYPES_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define NPP_VERSION_MAJOR 2
#define NPP_VERSION_MINOR 0
#define NPP_VERSION_PATCH 0
#define NPP_VERSION_STRING "2.0.0"

#define NPP_MAX_PIPES     65536
#define NPP_MAX_CLIENTS   1024
#define NPP_DEFAULT_PORT  8888
#define NPP_MAX_FRAME_SIZE (1024 * 1024)

/* NPP 2.0: Frame flags for optional blocks */
#define NPP_FLAG_CRC32        0x0001  /* L1: CRC32 checksum block */
#define NPP_FLAG_HOPPING      0x0002  /* Pipe ID hopping block */
#define NPP_FLAG_VALIDATION   0x0004  /* Cross-validation block */
#define NPP_FLAG_RESERVOIR    0x0008  /* Reservoir batch marker */
#define NPP_FLAG_EXTENSION    0x0010  /* Schema extension block */
#define NPP_FLAG_COLORING     0x0020  /* Pipe coloring tags */
#define NPP_FLAG_REGION       0x0040  /* Region subscription mask */
#define NPP_FLAG_VERSIONED    0x0080  /* CRDT version numbers */
#define NPP_FLAG_TIMESTAMP    0x0100  /* Precise timestamp block */

/* Frame types */
typedef enum {
    NPP_FRAME_SYNC       = 0x01,
    NPP_FRAME_DATA       = 0x02,
    NPP_FRAME_REQ_SYNC   = 0x03,
    NPP_FRAME_ACK        = 0x04,
    NPP_FRAME_HEARTBEAT  = 0x05,
    NPP_FRAME_ERROR      = 0x06,
} npp_frame_type_t;

/* Data types for pipe values */
typedef enum {
    NPP_DATA_U8   = 0,
    NPP_DATA_F32  = 1,
    NPP_DATA_F64  = 2,
    NPP_DATA_U32  = 3,
    NPP_DATA_U64  = 4,
} npp_data_type_t;

/* Pipe categories */
typedef enum {
    NPP_CAT_SESSION = 0,
    NPP_CAT_FRAME   = 1,
    NPP_CAT_DATA    = 2,
} npp_pipe_category_t;

/* Layout types */
typedef enum {
    NPP_LAYOUT_FREE  = 0,
    NPP_LAYOUT_GRID  = 1,
    NPP_LAYOUT_TILED = 2,
} npp_layout_type_t;

/* Transport types */
typedef enum {
    NPP_TRANSPORT_TCP = 0,
    NPP_TRANSPORT_UDP = 1,
    NPP_TRANSPORT_SHM = 2,
} npp_transport_type_t;

/* Crypto types */
typedef enum {
    NPP_CRYPTO_NONE     = 0,
    NPP_CRYPTO_CHACHA20 = 1,
    NPP_CRYPTO_AES_GCM  = 2,
    NPP_CRYPTO_CUSTOM   = 3,
    NPP_CRYPTO_SM4      = 4,
} npp_crypto_type_t;

/* Return codes */
typedef enum {
    NPP_OK              = 0,
    NPP_ERR_INVALID     = -1,
    NPP_ERR_MEMORY      = -2,
    NPP_ERR_TIMEOUT     = -3,
    NPP_ERR_NETWORK     = -4,
    NPP_ERR_PROTOCOL    = -5,
    NPP_ERR_CRYPTO      = -6,
    NPP_ERR_SCHEMA      = -7,
    NPP_ERR_BACKEND     = -8,
    NPP_ERR_CRC         = -9,   /* NPP 2.0: CRC mismatch */
    NPP_ERR_HOPPING     = -10,  /* NPP 2.0: Hopping sync failure */
    NPP_ERR_EXTENSION   = -11,  /* NPP 2.0: Unknown extension */
    NPP_ERR_RESERVOIR   = -12,  /* NPP 2.0: Reservoir overflow */
    NPP_ERR_VALIDATION  = -13,  /* NPP 2.0: Validation failed */
} npp_status_t;

/* NPP 2.0: Reservoir trigger modes */
typedef enum {
    NPP_RESERVOIR_WATER_VOLUME = 0,  /* Trigger when buffer count >= capacity */
    NPP_RESERVOIR_WATER_LEVEL  = 1,  /* Trigger when watch pipe changes */
    NPP_RESERVOIR_TIMER        = 2,  /* Trigger on time interval */
} npp_reservoir_mode_t;

/* NPP 2.0: Validation types */
typedef enum {
    NPP_VALIDATION_CRC32      = 0,  /* L1: Frame-level CRC32 */
    NPP_VALIDATION_CROSS      = 1,  /* L2: Cross-pipe consistency */
    NPP_VALIDATION_TOPOLOGY   = 2,  /* L3: Spatial neighbor consistency */
    NPP_VALIDATION_SILENT     = 3,  /* L4: Silent state verification */
} npp_validation_type_t;

/* NPP 2.0: Pipe coloring tags */
typedef enum {
    NPP_COLOR_NONE       = 0,
    NPP_COLOR_PRIORITY   = 1,  /* QoS priority tag */
    NPP_COLOR_SECURITY   = 2,  /* Strong encryption tag */
    NPP_COLOR_AUDIT      = 3,  /* Audit trail tag */
    NPP_COLOR_ROUTE      = 4,  /* Routing directive tag */
} npp_color_tag_t;

/* Forward declarations */
typedef struct npp_session_s  npp_session_t;
typedef struct npp_server_s   npp_server_t;
typedef struct npp_group_s    npp_group_t;
typedef struct npp_output_s   npp_output_t;

#ifdef __cplusplus
}
#endif

#endif