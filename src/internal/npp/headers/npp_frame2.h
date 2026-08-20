#ifndef NPP_FRAME2_H
#define NPP_FRAME2_H

#include "npp_types.h"

#ifdef __cplusplus
extern "C" {
#endif

#pragma pack(push, 1)

/*
 * NPP 2.0 Frame Header (16 bytes, backward compatible)
 *
 * v1.0 layout: sync(2) + type(1) + ver(1) + frame_id(4) + schema_id(2) + reserved(2) + payload_len(4)
 * v2.0 layout: sync(2) + type(1) + ver(1) + frame_id(4) + schema_id(2) + flags(2)  + payload_len(4)
 *
 * The "reserved" field in v1 becomes "flags" in v2.
 * v1 frames have reserved=0, which equals flags=0 (no optional blocks).
 * => Perfect backward compatibility.
 */
typedef struct {
    uint16_t sync;          /* 0x4E50 ("NP" in little-endian) */
    uint8_t  frame_type;   /* NPP_FRAME_DATA / SYNC / REQ_SYNC ... */
    uint8_t  version;      /* 0x01 = v1.0, 0x02 = v2.0 */
    uint32_t frame_id;     /* Monotonic frame sequence number */
    uint16_t schema_id;     /* Schema hash ID */
    uint16_t flags;        /* NPP_FLAG_* bitmask (was "reserved" in v1) */
    uint32_t payload_len;  /* Base payload length (excludes optional blocks) */
} npp_frame2_header_t;

/*
 * Optional block header (4 bytes)
 * Every optional block starts with this header.
 * Unknown blocks can be skipped by reading block_len.
 */
typedef struct {
    uint16_t block_type;   /* Which flag this block corresponds to (NPP_FLAG_*) */
    uint16_t block_len;    /* Length of block_data (excludes this header) */
} npp_optional_block_t;

/* CRC32 block (4 bytes data) */
typedef struct {
    uint32_t crc32;        /* CRC32 of entire frame (header + payload + blocks, excluding this CRC) */
} npp_crc32_block_t;

/* Hopping block */
typedef struct {
    uint32_t hop_epoch;    /* Time window number for PRNG seed */
    uint16_t hop_count;    /* Number of pipes that were remapped */
} npp_hopping_block_t;

/* Validation block */
typedef struct {
    uint8_t  validation_type;  /* npp_validation_type_t */
    uint8_t  reserved;
    uint16_t pipe_mask_len;    /* Bytes in the pipe mask */
    /* Followed by: uint8_t pipe_mask[pipe_mask_len] + uint32_t checksum */
} npp_validation_block_t;

/* Extension block */
typedef struct {
    uint16_t extension_id;     /* Extension identifier */
    uint16_t extension_ver;    /* Extension version */
    uint16_t pipe_offset;      /* Where extension pipes start */
    uint16_t pipe_count;       /* How many pipes this extension adds */
} npp_extension_block_t;

/* Coloring block */
typedef struct {
    uint8_t  color_tag;        /* npp_color_tag_t */
    uint8_t  color_value;      /* Tag value (e.g. priority level) */
    uint16_t pipe_count;       /* Number of pipes with this color */
    /* Followed by: uint16_t pipe_ids[pipe_count] */
} npp_coloring_block_t;

/* Region subscription block */
typedef struct {
    uint16_t region_id;        /* Region identifier */
    uint16_t start_pipe;       /* First pipe in region */
    uint16_t end_pipe;         /* Last pipe in region */
} npp_region_block_t;

/* Timestamp block */
typedef struct {
    uint64_t timestamp_us;     /* Microsecond precision timestamp */
} npp_timestamp_block_t;

#pragma pack(pop)

#define NPP_FRAME2_HEADER_SIZE  sizeof(npp_frame2_header_t)
#define NPP_OPTIONAL_BLOCK_SIZE sizeof(npp_optional_block_t)

/*
 * Encode a v2.0 DATA frame with optional blocks.
 *
 * buffer:         output buffer
 * frame_id:       frame sequence number
 * schema_id:      schema hash
 * flags:          NPP_FLAG_* bitmask
 * bitmap:         change bitmap
 * bitmap_bytes:   bitmap size
 * values:         changed values
 * values_bytes:   values size
 * change_count:   number of changed pipes
 *
 * Returns total frame size, or 0 on error.
 */
uint16_t npp_frame2_encode_data(uint8_t* buffer, uint32_t buffer_len,
                                 uint32_t frame_id, uint16_t schema_id,
                                 uint16_t flags,
                                 const uint8_t* bitmap, uint32_t bitmap_bytes,
                                 const uint8_t* values, uint32_t values_bytes,
                                 uint32_t change_count);

/*
 * Encode a v2.0 SYNC frame (CRDT-style with version numbers).
 * If flags & NPP_FLAG_VERSIONED, each pipe value is followed by a uint32_t version.
 */
uint16_t npp_frame2_encode_sync(uint8_t* buffer, uint32_t buffer_len,
                                 uint32_t frame_id, uint16_t schema_id,
                                 uint16_t flags,
                                 const uint8_t* all_values, uint32_t total_bytes,
                                 const uint32_t* versions, uint32_t version_count);

/*
 * Append CRC32 optional block to an encoded frame.
 * Must be called AFTER all other blocks are appended.
 * Updates payload_len and flags in the header.
 */
int npp_frame2_append_crc32(uint8_t* buffer, uint32_t* frame_len,
                             uint32_t buffer_len);

/*
 * Append timestamp block.
 */
int npp_frame2_append_timestamp(uint8_t* buffer, uint32_t* frame_len,
                                  uint32_t buffer_len, uint64_t timestamp_us);

/*
 * Append hopping block.
 */
int npp_frame2_append_hopping(uint8_t* buffer, uint32_t* frame_len,
                                uint32_t buffer_len,
                                uint32_t hop_epoch, uint16_t hop_count);

/*
 * Append validation block.
 */
int npp_frame2_append_validation(uint8_t* buffer, uint32_t* frame_len,
                                   uint32_t buffer_len,
                                   uint8_t validation_type,
                                   const uint8_t* pipe_mask, uint16_t mask_len,
                                   uint32_t checksum);

/*
 * Append extension block.
 */
int npp_frame2_append_extension(uint8_t* buffer, uint32_t* frame_len,
                                  uint32_t buffer_len,
                                  uint16_t extension_id, uint16_t extension_ver,
                                  uint16_t pipe_offset, uint16_t pipe_count);

/*
 * Append coloring block.
 */
int npp_frame2_append_coloring(uint8_t* buffer, uint32_t* frame_len,
                                 uint32_t buffer_len,
                                 uint8_t color_tag, uint8_t color_value,
                                 const uint16_t* pipe_ids, uint16_t pipe_count);

/*
 * Decode v2.0 frame header (also works with v1.0 frames).
 */
int npp_frame2_decode_header(const uint8_t* buffer, uint16_t len,
                               npp_frame2_header_t* header);

/*
 * Find an optional block by type in a decoded frame.
 * Returns pointer to the block data (after npp_optional_block_t), or NULL.
 * Sets *block_len to the block data length.
 */
const uint8_t* npp_frame2_find_block(const uint8_t* frame, uint16_t frame_len,
                                       uint16_t block_type, uint16_t* block_len);

/*
 * Verify CRC32 of a frame.
 * Returns NPP_OK if valid, NPP_ERR_CRC if mismatch.
 */
int npp_frame2_verify_crc32(const uint8_t* frame, uint16_t frame_len);

/*
 * CRC32 computation (IEEE 802.3 polynomial).
 */
uint32_t npp_crc32(const uint8_t* data, uint32_t len);

#ifdef __cplusplus
}
#endif

#endif
