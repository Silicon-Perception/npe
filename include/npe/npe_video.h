#ifndef NPE_VIDEO_H
#define NPE_VIDEO_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define NPE_VIDEO_MAX_WIDTH  1920
#define NPE_VIDEO_MAX_HEIGHT 1080

typedef struct NPEVideoEncoder_s NPEVideoEncoder;
typedef struct NPEVideoDecoder_s NPEVideoDecoder;

typedef struct {
    uint16_t width;
    uint16_t height;
    uint8_t  fps;
    uint8_t  bits_per_pixel;
    uint8_t  threshold;
    uint8_t  sampling_mode;
    uint8_t  default_value;
    uint8_t  enable_default;

    uint8_t  enable_energy;
    uint8_t  enable_direction;
    uint8_t  enable_decay;
    uint8_t  enable_flood;
    uint8_t  enable_dilate;
    uint8_t  energy_threshold;
    uint8_t  decay_shift;
    uint8_t  flood_iterations;
    uint8_t  spatial_radius;

    uint8_t  enable_epa;
    uint16_t epa_max_regions;
    uint16_t epa_point_max_pixels;
    uint16_t epa_rect_min_pixels;

    uint8_t  enable_perceptual;

    uint16_t max_delta_depth;
    uint8_t  enable_state_fingerprint;
} NPEVideoConfig;

typedef struct {
    uint32_t ipa_changes;
    uint32_t filter_perceptual;
    uint32_t filter_default;
    uint32_t filter_sampling;
    uint16_t epa_events;
    uint16_t epa_mode_changed;

    uint32_t bytes_header;
    uint32_t bytes_control;
    uint32_t bytes_palette;
    uint32_t bytes_regions;
    uint32_t bytes_mask;
    uint32_t bytes_bitstream;
    uint32_t bytes_total;

    double   compression_ratio;
    uint8_t  is_keyframe;
    uint8_t  frame_type;
} NPEVideoStats;

NPEVideoEncoder* npe_video_encoder_create(const NPEVideoConfig* cfg);
void npe_video_encoder_destroy(NPEVideoEncoder* enc);
int npe_video_encoder_process(NPEVideoEncoder* enc, const uint8_t* frame, uint8_t* packet, uint32_t packet_cap, uint32_t* packet_size);
void npe_video_encoder_reset(NPEVideoEncoder* enc);
int npe_video_encoder_set_config(NPEVideoEncoder* enc, const NPEVideoConfig* cfg);
const NPEVideoStats* npe_video_encoder_get_stats(const NPEVideoEncoder* enc);

/* ========== 控制通道API (编码器) ========== */

typedef struct {
    uint8_t  msg_type;          /* 消息类型 */
    uint8_t  reason;            /* 失步原因 */
    uint16_t target_frame;      /* 目标帧ID */
    uint16_t current_frame;     /* 当前帧ID */
    uint16_t state_fingerprint; /* 状态指纹 */
} NPEControlMessage;

typedef struct {
    uint8_t  is_healthy;        /* 1=健康, 0=不健康 */
    uint8_t  reason;            /* 失步原因 (0=健康) */
    uint16_t last_frame_id;     /* 最近接收帧ID */
    uint16_t expected_frame;    /* 期望下一帧ID */
    uint16_t state_fingerprint; /* 当前状态指纹 */
    uint8_t  consecutive_errors;/* 连续错误数 */
} NPEHealthState;

int npe_video_encoder_handle_control(NPEVideoEncoder* enc, const NPEControlMessage* msg);
uint8_t npe_video_encoder_need_sync(const NPEVideoEncoder* enc);
void npe_video_encoder_clear_sync(NPEVideoEncoder* enc);

/* ========== 控制通道API (解码器) ========== */

void npe_video_decoder_set_health_check(NPEVideoDecoder* dec, uint8_t enable);
const NPEHealthState* npe_video_decoder_get_health(const NPEVideoDecoder* dec);
int npe_video_decoder_build_sync_request(NPEVideoDecoder* dec, 
                                        uint8_t* buffer, uint32_t capacity,
                                        uint32_t* out_size);
uint8_t npe_video_decoder_has_pending_sync(const NPEVideoDecoder* dec);
void npe_video_decoder_clear_sync_request(NPEVideoDecoder* dec);

/* 调试/测试API: 手动设置失步状态 */
void npe_video_decoder_set_desynced(NPEVideoDecoder* dec, uint8_t is_desynced);

NPEVideoDecoder* npe_video_decoder_create(const NPEVideoConfig* cfg);
void npe_video_decoder_destroy(NPEVideoDecoder* dec);
int npe_video_decoder_process(NPEVideoDecoder* dec, const uint8_t* packet, uint32_t packet_size, uint8_t* frame, uint32_t frame_size);
void npe_video_decoder_reset(NPEVideoDecoder* dec);
int npe_video_decoder_set_config(NPEVideoDecoder* dec, const NPEVideoConfig* cfg);

#ifdef __cplusplus
}
#endif

#endif