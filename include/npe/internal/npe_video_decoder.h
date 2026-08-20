#ifndef NPE_VIDEO_DECODER_H
#define NPE_VIDEO_DECODER_H

#include "npe/internal/npe_types.h"
#include "npe/internal/npe_video_protocol.h"
#include "npe/internal/npe_filter.h"
#include "npe/internal/npe_image.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint16_t width;
    uint16_t height;
    uint8_t  bits_per_pixel;
    uint8_t  fps;
    uint8_t  sampling_mode;
    uint8_t  filter_flags;
    uint8_t  default_value;
    uint8_t  palette_size;
    NPEPaletteEntry palette[256];
    uint8_t  enable_state_fingerprint;
    uint16_t threshold;
    uint8_t  engine_flags;
} NPEDecoderConfig;

typedef struct {
    NPEVideoUnpacker unpacker;
    
    NPEDecoderConfig config;
    uint8_t* frame_buffer;
    uint32_t frame_size;
    
    uint8_t* bitstream_buf;
    uint32_t bitstream_cap;
    
    NPERegionDesc* regions_buf;
    uint16_t regions_cap;
    
    NPEImage* dec_ipa;
    
    NPEVideoControl last_control;
    uint8_t control_valid;
    
    uint16_t frame_id;
    uint16_t state_fingerprint;
    uint8_t  is_desynced;
    uint8_t* sync_state_buf;
    uint32_t sync_state_cap;
    
    /* 健康评估状态 */
    NPEHealthState health;
    uint8_t  health_check_enabled;
    uint8_t  pending_sync_request;
    uint16_t next_expected_frame;
    
    /* 帧重建后指纹校验 */
    uint16_t recv_fingerprint;
    uint8_t  has_recv_fingerprint;
} NPEVideoDecoder;

NPEVideoDecoder* npe_decoder_create(const NPEDecoderConfig* cfg);
#define npe_decoder_create_internal npe_decoder_create

void npe_decoder_destroy(NPEVideoDecoder* dec);
#define npe_decoder_destroy_internal npe_decoder_destroy

int npe_decoder_process(NPEVideoDecoder* dec,
                         const uint8_t* packet, uint32_t packet_size,
                         uint8_t* output_frame, uint32_t output_size);
#define npe_decoder_process_internal npe_decoder_process

void npe_decoder_reset(NPEVideoDecoder* dec);
#define npe_decoder_reset_internal npe_decoder_reset

const NPEDecoderConfig* npe_decoder_get_config(const NPEVideoDecoder* dec);

int npe_decoder_update_config(NPEVideoDecoder* dec, const NPEVideoControl* ctrl);

void npe_decoder_set_enable_state_fingerprint(NPEVideoDecoder* dec, uint8_t enable);

/* ========== 健康评估API ========== */

/* 启用/禁用健康检查 */
void npe_decoder_set_health_check(NPEVideoDecoder* dec, uint8_t enable);

/* 获取当前健康状态 */
const NPEHealthState* npe_decoder_get_health(const NPEVideoDecoder* dec);

/* 构建同步请求消息 (供上层发送) */
int npe_decoder_build_sync_request(NPEVideoDecoder* dec, 
                                   uint8_t* buffer, uint32_t capacity,
                                   uint32_t* out_size);

/* 检查是否有待定的同步请求 */
uint8_t npe_decoder_has_pending_sync(const NPEVideoDecoder* dec);

/* 清除待定同步请求 (编码器响应后调用) */
void npe_decoder_clear_sync_request(NPEVideoDecoder* dec);

#ifdef __cplusplus
}
#endif

#endif