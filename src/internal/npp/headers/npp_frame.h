#ifndef NPP_FRAME_H
#define NPP_FRAME_H

#include "npp_types.h"

#ifdef __cplusplus
extern "C" {
#endif

#pragma pack(push, 1)

/* NPP Frame Header (16 bytes) */
typedef struct {
    uint16_t sync;
    uint8_t  frame_type;
    uint8_t  version;
    uint32_t frame_id;
    uint16_t schema_id;
    uint16_t reserved;
    uint32_t payload_len;
} npp_frame_header_t;

/* DATA frame payload header */
typedef struct {
    uint32_t change_count;
    uint32_t bitmap_offset;
    uint32_t values_offset;
    uint32_t bitmap_bytes;
} npp_data_header_t;

/* SYNC frame payload header */
typedef struct {
    uint32_t total_pipes;
    uint32_t values_offset;
} npp_sync_header_t;

/* REQ_SYNC frame payload */
typedef struct {
    uint32_t requested_from;
    uint32_t requested_to;
    uint8_t  reason;
    uint8_t  reserved[3];
} npp_req_sync_payload_t;

#pragma pack(pop)

#define NPP_FRAME_HEADER_SIZE  sizeof(npp_frame_header_t)
#define NPP_DATA_HEADER_SIZE   sizeof(npp_data_header_t)
#define NPP_SYNC_HEADER_SIZE   sizeof(npp_sync_header_t)

/* Frame encode/decode API */
uint16_t npp_frame_encode_data(uint8_t* buffer, uint32_t frame_id,
                                uint16_t schema_id,
                                const uint8_t* bitmap, uint32_t bitmap_bytes,
                                const uint8_t* values, uint32_t values_bytes,
                                uint32_t change_count);

uint16_t npp_frame_encode_sync(uint8_t* buffer, uint32_t frame_id,
                                uint16_t schema_id,
                                const uint8_t* all_values, uint32_t total_bytes);

uint16_t npp_frame_encode_req_sync(uint8_t* buffer, uint32_t frame_id,
                                    uint32_t from, uint32_t to, uint8_t reason);

int      npp_frame_decode_header(const uint8_t* buffer, uint16_t len,
                                  npp_frame_header_t* header);

int      npp_frame_decode_data(const uint8_t* buffer, uint16_t len,
                                npp_frame_header_t* header,
                                npp_data_header_t* data_hdr,
                                uint8_t** bitmap, uint8_t** values);

int      npp_frame_decode_sync(const uint8_t* buffer, uint16_t len,
                                npp_frame_header_t* header,
                                npp_sync_header_t* sync_hdr,
                                uint8_t** values);

#ifdef __cplusplus
}
#endif

#endif