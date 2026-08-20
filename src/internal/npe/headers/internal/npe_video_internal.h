/**
 * NPE 4.0 — 视频适配器主模块
 *
 * 整合所有优化层:
 *   IPA引擎 → 过滤层(感知+默认值+采样) → EPA区域编码 → 位流打包 → 视频协议
 */

#ifndef NPE_VIDEO_INTERNAL_H
#define NPE_VIDEO_INTERNAL_H

#include "npe/internal/npe_types.h"
#include "npe/internal/npe_core.h"
#include "npe/internal/npe_image.h"
#include "npe/internal/npe_filter.h"
#include "npe/internal/npe_epa.h"
#include "npe/internal/npe_bitstream.h"
#include "npe/internal/npe_video_protocol.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint16_t width;
    uint16_t height;
    uint8_t  fps;
    uint8_t  bits_per_pixel;

    uint8_t  enable_energy;
    uint8_t  enable_direction;
    uint8_t  enable_decay;
    uint8_t  enable_flood;
    uint8_t  enable_dilate;
    uint8_t  energy_threshold;
    uint8_t  decay_shift;
    uint8_t  flood_iterations;
    uint8_t  spatial_radius;
    uint8_t  neighborhood_mode;

    uint8_t  enable_perceptual;
    uint8_t  enable_default;
    uint8_t  default_value;
    uint8_t  enable_sampling;
    uint8_t  sampling_mode;

    uint8_t  enable_epa;
    uint8_t  enable_mode_diff;
    uint16_t epa_max_regions;
    uint16_t epa_point_max_pixels;
    uint16_t epa_rect_min_pixels;
    uint32_t compensate_threshold;

    uint8_t  threshold;

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

typedef struct {
    NPEImage*       ipa;
    NPEFilter*      filter;
    NPEEPA*         epa;

    NPEVideoConfig  config;

    uint8_t*        bitstream_buf;
    uint32_t        bitstream_cap;
    NPEEncodeEvent* events_buf;
    uint16_t        events_cap;
    NPERegionDesc*  regions_buf;
    uint16_t        regions_cap;
    
    /* 预分配工作缓冲区 (性能优化: 避免每帧 malloc) */
    uint8_t*        changed_values_buf;   /* IPA 输出: 变化值 */
    bool*           mask_bool_buf;        /* IPA 输出: 变化布尔掩码 */
    uint8_t*        prev_state_buf;       /* IPA 输入: 上一帧状态备份 */
    uint32_t        work_buf_size;        /* 缓冲区容量 (像素数) */

    NPEVideoStats   stats;
    uint16_t        frame_id;
    uint16_t        delta_depth;
    
    /* 控制通道: 接收端同步请求 */
    uint8_t         force_sync_frame;
    NPEControlMessage pending_control;
    uint8_t         has_pending_control;
} NPEVideo;

NPEVideo* npe_video_create(const NPEVideoConfig* cfg);
void npe_video_destroy(NPEVideo* v);
int npe_video_set_config(NPEVideo* v, const NPEVideoConfig* cfg);
int npe_video_set_palette(NPEVideo* v, const NPEPaletteEntry* palette, uint8_t size);
int npe_video_process(NPEVideo* v, const uint8_t* input, uint8_t* output, uint32_t output_cap, uint32_t* output_size);
void npe_video_reset(NPEVideo* v);
const NPEVideoStats* npe_video_get_stats(const NPEVideo* v);
void npe_video_default_config(NPEVideoConfig* cfg);

/* ========== 控制通道API ========== */

/* 编码器: 接收并处理来自解码器的控制请求 */
int npe_video_handle_control(NPEVideo* v, const NPEControlMessage* msg);

/* 编码器: 检查是否需要发送同步帧 */
uint8_t npe_video_need_sync_frame(const NPEVideo* v);

/* 编码器: 清除同步帧请求标志 */
void npe_video_clear_sync_request(NPEVideo* v);

#ifdef __cplusplus
}
#endif

#endif