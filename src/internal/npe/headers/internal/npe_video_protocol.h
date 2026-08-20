/**
 * NPE 4.0 — 视频协议层
 *
 * 双通道设计:
 *   - Data Channel: [帧头] [控制段(可选)] [位流像素数据]
 *   - Control Channel: 健康评估反馈、同步请求
 *
 * 核心哲学:
 *   - 关键帧同步是控制问题，而非机制问题
 *   - 接收端主动评估健康状态，按需请求同步
 *   - 避免定期推送关键帧造成的带宽浪费
 *
 * 帧结构:
 *   [帧头] [控制段(可选)] [调色板段(可选)] [EPA区域描述符段] [位流像素数据]
 *
 * 零计算: 仅位移/比较/赋值
 * MCU兼容: 紧凑结构, 无动态内存
 */

#ifndef NPE_VIDEO_PROTOCOL_H
#define NPE_VIDEO_PROTOCOL_H

#include "npe/internal/npe_types.h"
#include "npe/internal/npe_core.h"
#include "npe/internal/npe_epa.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ========== 帧类型 ========== */

typedef enum {
    NPE_VFRAME_FULL       = 0x01,  /* 完整帧 (含所有段) */
    NPE_VFRAME_CONTROL    = 0x02,  /* 仅控制段变化 */
    NPE_VFRAME_PALETTE    = 0x03,  /* 仅调色板变化 */
    NPE_VFRAME_DATA       = 0x04,  /* 仅数据帧 (区域+位流) */
    NPE_VFRAME_COMPACT    = 0x05,  /* 紧凑帧 (仅位流, 无区域描述符) */
    NPE_VFRAME_SYNC       = 0x06,  /* 同步帧 (管道状态快照) */
    NPE_VFRAME_CTRL_REQ   = 0x07,  /* 控制请求帧 (接收端→发送端) */
} NPEVideoFrameType;

/* ========== 段标志 (位掩码) ========== */

#define NPE_SEG_CONTROL    0x01  /* 含控制段 */
#define NPE_SEG_PALETTE    0x02  /* 含调色板段 */
#define NPE_SEG_REGIONS    0x04  /* 含EPA区域描述符段 */
#define NPE_SEG_BITSTREAM  0x08  /* 含位流像素数据段 */
#define NPE_SEG_SYNC       0x10  /* 含同步帧数据段 */
#define NPE_SEG_FINGERPRINT 0x20 /* 含状态指纹段 */
#define NPE_SEG_CTRL_DATA  0x40  /* 含控制请求数据段 */
#define NPE_SEG_MASK       0x80  /* 含变化掩码段 */

/* ========== 控制消息类型 ========== */

typedef enum {
    NPE_CTRL_SYNC_REQUEST    = 0x01,  /* 请求同步帧 (失步恢复) */
    NPE_CTRL_HEALTH_REPORT   = 0x02,  /* 健康报告 (周期性状态) */
    NPE_CTRL_CONFIG_CHANGE   = 0x03,  /* 请求配置变更 */
    NPE_CTRL_ACK             = 0x04,  /* 确认消息 */
} NPEControlMsgType;

/* ========== 失步原因码 ========== */

typedef enum {
    NPE_DESYNC_NONE          = 0x00,  /* 健康 */
    NPE_DESYNC_FRAME_GAP     = 0x01,  /* 帧ID不连续 (丢帧) */
    NPE_DESYNC_FINGERPRINT   = 0x02,  /* 状态指纹不匹配 */
    NPE_DESYNC_DECODE_ERROR  = 0x03,  /* 位流解码错误 */
    NPE_DESYNC_TIMEOUT       = 0x04,  /* 超时 (长时间无数据) */
    NPE_DESYNC_INIT          = 0x05,  /* 初始状态 (请求首帧) */
} NPEDesyncReason;

/* ========== 视频帧头 (紧凑) ========== */

#pragma pack(push, 1)
typedef struct {
    uint16_t sync;           /* 同步字 0xAA55 */
    uint16_t frame_id;       /* 帧ID */
    uint8_t  frame_type;     /* NPEVideoFrameType */
    uint8_t  seg_flags;      /* 段标志 (NPE_SEG_*) */
    uint16_t total_size;     /* 整帧总字节数 (不含帧头) */
} NPEVideoHeader;
#pragma pack(pop)

#define NPE_VIDEO_HEADER_SIZE  sizeof(NPEVideoHeader)

/* ========== 同步帧数据段 ========== */

#pragma pack(push, 1)
typedef struct {
    uint16_t delta_count;    /* 自上次同步以来的delta帧数 */
    uint16_t state_fingerprint; /* 管道状态指纹 (CRC16) */
} NPEVideoSyncData;
#pragma pack(pop)

#define NPE_VIDEO_SYNC_DATA_SIZE  sizeof(NPEVideoSyncData)

/* ========== 控制段 (CPA雏形) ========== */

#pragma pack(push, 1)
typedef struct {
    uint16_t width;          /* 分辨率宽 */
    uint16_t height;         /* 分辨率高 */
    uint8_t  fps;            /* 帧率 (1-60) */
    uint8_t  bits_per_pixel; /* 位深 (1-8) */
    uint8_t  sampling_mode;  /* 采样模式 */
    uint8_t  filter_flags;   /* 过滤层开关 (bit0=感知, bit1=默认值, bit2=采样) */
    uint8_t  default_value;  /* 默认值 (背景色) */
    uint8_t  palette_size;   /* 调色板项数 (0=无变化) */
    uint16_t threshold;      /* NPE引擎阈值 */
    uint8_t  engine_flags;   /* NPE引擎功能开关 (bit0=energy, bit1=direction, bit2=decay, bit3=flood, bit4=dilate) */
    uint8_t  reserved[2];    /* 预留 */
} NPEVideoControl;
#pragma pack(pop)

#define NPE_VIDEO_CONTROL_SIZE  sizeof(NPEVideoControl)

/* ========== 调色板项 ========== */

#pragma pack(push, 1)
typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
} NPEPaletteEntry;
#pragma pack(pop)

#define NPE_PALETTE_ENTRY_SIZE  sizeof(NPEPaletteEntry)

/* ========== 视频帧打包器 ========== */

typedef struct {
    uint8_t* buffer;          /* 输出缓冲区 */
    uint32_t  capacity;        /* 缓冲区容量 */
    uint32_t  pos;             /* 当前写入位置 */
} NPEVideoPacker;

/* ========== API ========== */

/* 初始化打包器 */
void npe_vp_init(NPEVideoPacker* pk, uint8_t* buffer, uint32_t capacity);

/* 写入帧头 */
int npe_vp_write_header(NPEVideoPacker* pk, uint16_t frame_id,
                         uint8_t frame_type, uint8_t seg_flags);

/* 写入控制段 */
int npe_vp_write_control(NPEVideoPacker* pk, const NPEVideoControl* ctrl);

/* 写入调色板段 */
int npe_vp_write_palette(NPEVideoPacker* pk, const NPEPaletteEntry* palette, uint8_t size);

/* 写入EPA区域描述符段 */
int npe_vp_write_regions(NPEVideoPacker* pk, const NPERegionDesc* descs, uint16_t count);

/* 写入位流数据段 */
int npe_vp_write_bitstream(NPEVideoPacker* pk, const uint8_t* bitstream, uint32_t size);

/* 写入变化掩码段 (位压缩: 1bit/pixel, 1=变化, 0=未变化) */
int npe_vp_write_mask(NPEVideoPacker* pk, const bool* mask, uint32_t total_pixels, uint32_t* mask_bytes);

/* 写入同步帧数据段 */
int npe_vp_write_sync_data(NPEVideoPacker* pk, const NPEVideoSyncData* sync_data,
                           const uint8_t* state_data, uint32_t state_size);

/* 写入状态指纹段 */
int npe_vp_write_fingerprint(NPEVideoPacker* pk, uint16_t fingerprint);

/* 完成打包: 回填帧头的total_size, 返回总字节数 */
uint32_t npe_vp_finalize(NPEVideoPacker* pk);

/* ========== 解包器 ========== */

typedef struct {
    const uint8_t* buffer;
    uint32_t  size;
    uint32_t  pos;
    NPEVideoHeader header;
} NPEVideoUnpacker;

void npe_vu_init(NPEVideoUnpacker* up, const uint8_t* buffer, uint32_t size);

/* 解析帧头 */
int npe_vu_read_header(NPEVideoUnpacker* up);

/* 读取控制段 */
int npe_vu_read_control(NPEVideoUnpacker* up, NPEVideoControl* ctrl);

/* 读取调色板段 */
int npe_vu_read_palette(NPEVideoUnpacker* up, NPEPaletteEntry* palette, uint8_t max_size, uint8_t* size);

/* 读取区域描述符段 */
int npe_vu_read_regions(NPEVideoUnpacker* up, NPERegionDesc* descs, uint16_t max_count, uint16_t* count);

/* 读取位流数据段 */
int npe_vu_read_bitstream(NPEVideoUnpacker* up, uint8_t* bitstream, uint32_t max_size, uint32_t* size);

/* 读取变化掩码段 (位压缩: 1bit/pixel, 1=变化, 0=未变化) */
int npe_vu_read_mask(NPEVideoUnpacker* up, bool* mask, uint32_t total_pixels);

/* 读取同步帧数据段 */
int npe_vu_read_sync_data(NPEVideoUnpacker* up, NPEVideoSyncData* sync_data);

/* 读取状态指纹段 */
int npe_vu_read_fingerprint(NPEVideoUnpacker* up, uint16_t* fingerprint);

/* 读取同步帧状态数据 */
int npe_vu_read_sync_state(NPEVideoUnpacker* up, uint8_t* state_data, uint32_t max_size, uint32_t* size);

/* ========== 控制消息结构 ========== */

#pragma pack(push, 1)
typedef struct {
    uint8_t  msg_type;        /* NPEControlMsgType */
    uint8_t  reason;          /* NPEDesyncReason (同步请求时使用) */
    uint16_t target_frame;    /* 目标帧ID (请求同步的帧) */
    uint16_t current_frame;   /* 当前接收的最新帧ID */
    uint16_t state_fingerprint; /* 本地计算的状态指纹 */
} NPEControlMessage;
#pragma pack(pop)

#define NPE_CONTROL_MSG_SIZE  sizeof(NPEControlMessage)

/* ========== 解码器健康状态 ========== */

typedef struct {
    uint8_t  is_healthy;      /* 1=健康, 0=不健康 */
    uint8_t  reason;          /* NPEDesyncReason */
    uint16_t last_frame_id;   /* 最后收到的帧ID */
    uint16_t expected_frame;  /* 期望的下一帧ID */
    uint16_t state_fingerprint; /* 当前状态指纹 */
    uint8_t  consecutive_errors; /* 连续错误计数 */
} NPEHealthState;

/* ========== 控制通道API ========== */

/* 解码器: 评估健康状态，判断是否需要请求同步 */
int npe_decoder_health_assess(const NPEHealthState* state);

/* 解码器: 构造同步请求消息 */
int npe_vp_build_sync_request(uint8_t* buffer, uint32_t capacity,
                               const NPEControlMessage* msg, uint32_t* out_size);

/* 编码器: 解析控制请求消息 */
int npe_vu_parse_control_request(const uint8_t* buffer, uint32_t size,
                                 NPEControlMessage* msg);

/* 编码器: 处理控制请求 (返回1表示需要发送同步帧) */
int npe_encoder_handle_control(void* encoder, const NPEControlMessage* msg);

#ifdef __cplusplus
}
#endif

#endif
