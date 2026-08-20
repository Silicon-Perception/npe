/**
 * NPE 4.0 — 协议 API
 *
 * NPE 协议帧结构:
 *   [帧头] [控制段] [图像数据段]
 *   - 仅传输渗出的变化数据，极低带宽
 */

#ifndef NPE_PROTOCOL_H
#define NPE_PROTOCOL_H

#include "npe/internal/npe_types.h"

#ifdef __cplusplus
extern "C" {
#endif

#pragma pack(push, 1)

typedef struct {
    uint16_t sync;
    uint16_t frame_id;
    uint8_t  frame_type;
    uint8_t  payload_len;
} NPEFrameHeader;

typedef struct {
    uint16_t pipeline_id;
    uint8_t  value;
} NPEPipelineUpdate;

typedef struct {
    uint16_t width;
    uint16_t height;
    uint8_t  color_format;
    uint8_t  color_mode;
    uint8_t  y_levels;
    uint8_t  uv_levels;
    uint8_t  sampling_mode;
    uint8_t  reserved[8];
} NPEControlData;

typedef struct {
    uint16_t x;
    uint16_t y;
    uint8_t  value;
} NPEImagePixel;

#pragma pack(pop)

#define NPE_FRAME_HEADER_SIZE  sizeof(NPEFrameHeader)
#define NPE_CONTROL_DATA_SIZE  sizeof(NPEControlData)
#define NPE_IMAGE_PIXEL_SIZE   sizeof(NPEImagePixel)

uint16_t npe_protocol_pack_control(uint8_t* buffer, uint16_t frame_id,
                                   const NPEControlData* ctrl);
uint16_t npe_protocol_pack_data(uint8_t* buffer, uint16_t frame_id,
                                const NPEImagePixel* pixels, uint16_t count);
int      npe_protocol_unpack_control(const uint8_t* buffer, uint16_t len,
                                     NPEControlData* ctrl, uint16_t* frame_id);
int      npe_protocol_unpack_data(const uint8_t* buffer, uint16_t len,
                                  NPEImagePixel* pixels, uint16_t max_count,
                                  uint16_t* count, uint16_t* frame_id);

#ifdef __cplusplus
}
#endif

#endif
