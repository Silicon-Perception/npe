/**
 * NPE 4.0 — EPA (Encoding Pipeline Array) 编码管道阵列
 *
 * NPE-D 的正式实例化: 作为 NPE 架构内的第三个管道阵列
 *   - IPA (Image Pipeline Array)      : 图像亮度差分
 *   - CPA (Control Pipeline Array)    : 控制状态差分
 *   - EPA (Encoding Pipeline Array)   : 编码模式差分  ← 本文件
 *
 * EPA 的 NPE 本质:
 *   - 管道单元 = 每个被检测到的连通区域
 *   - 状态     = 当前编码模式 (RAW/RLE/RECT/POINT)
 *   - 输入     = 区域特征 (大小、值分布)
 *   - 判断变化 = 当前最优编码模式 vs 上一帧编码模式
 *   - 渗出     = 编码模式变化时输出"模式切换事件"
 *   - 阻挡     = 编码模式不变时仅输出数据, 不重传模式描述符
 *
 * MCU 兼容性:
 *   - 无 SIMD, 无浮点, 无乘除法 (仅比较/位移/赋值)
 *   - 通过 NPE_PLATFORM_MCU 切换为静态分配
 *   - 数据结构紧凑 (#pragma pack)
 *   - 编码模式决策仅用比较和阈值判断
 */

#ifndef NPE_EPA_H
#define NPE_EPA_H

#include "npe/internal/npe_types.h"
#include "npe/internal/npe_core.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ========== EPA 编码模式 ========== */

typedef enum {
    NPE_ENC_RAW   = 0,  /* 原始值流: 逐像素输出变化值 */
    NPE_ENC_RLE   = 1,  /* 游程编码: (值, 长度) 对 */
    NPE_ENC_RECT  = 2,  /* 矩形描述: (x, y, w, h, value) 区域内值一致 */
    NPE_ENC_POINT = 3,  /* 单点描述: (x, y, value) 零散点 */
} NPEEncodeMode;

/* ========== EPA 编码事件 (输出结构) ==========
 *
 * 一个事件描述一个区域的编码输出
 * 当 mode_changed=1 时, 接收方需更新该区域的解码模式
 * 当 mode_changed=0 时, 接收方沿用上一帧模式, 仅更新数据
 */
#pragma pack(push, 1)
typedef struct {
    uint8_t  mode;           /* NPEEncodeMode: 当前编码模式 */
    uint8_t  mode_changed;   /* 1=模式切换(渗出), 0=模式不变(阻挡) */
    uint16_t region_id;      /* 区域ID (EPA 管道索引) */
    uint16_t x;              /* 区域左上角 x */
    uint16_t y;              /* 区域左上角 y */
    uint16_t width;          /* 区域宽度 */
    uint16_t height;         /* 区域高度 */
    uint8_t  value;          /* 主值 (RECT) 或起始值 (RLE) */
    uint8_t  value_end;      /* 结束值 (RLE/渐变) */
    uint32_t pixel_count;    /* 区域内像素数 */
} NPEEncodeEvent;
#pragma pack(pop)

/* ========== EPA 配置 ========== */

typedef struct {
    uint16_t width;
    uint16_t height;
    uint16_t max_regions;       /* EPA 管道数上限 (区域数上限) */
    /* 编码模式决策阈值 (仅比较, 无计算) */
    uint16_t point_max_pixels;  /* 像素数 <= 此值 → POINT */
    uint16_t rect_min_pixels;   /* 像素数 >= 此值且值一致 → RECT */
    uint16_t rle_min_runs;      /* 段数 >= 此值 → RLE 优于 RAW */
} NPEEPAConfig;

/* ========== EPA 引擎 ==========
 *
 * 性能优化:
 *   - 预分配缓冲区, 避免每帧 malloc/free
 *   - 消除重复值一致性检查
 *   - 内联区域提取逻辑
 */

/* 预分配的 run 缓冲区 (避免每帧 malloc) */
#define NPE_EPA_MAX_RUNS  (NPE_MAX_WIDTH * NPE_MAX_HEIGHT / 4)

typedef struct {
    /* 底层 NPE Core: 复用状态差分基础设施 */
    NPEngineCore*   core;
    /* EPA 管道状态: 每个区域上一帧的编码模式 */
    NPEEncodeMode*  last_modes;
    /* 区域 ID 计数器 (用于 region_id 分配) */
    uint16_t        next_region_id;
    uint16_t        max_regions;
    uint16_t        width;
    uint16_t        height;
    /* 决策阈值 */
    uint16_t        point_max_pixels;
    uint16_t        rect_min_pixels;
    uint16_t        rle_min_runs;
    /* 预分配缓冲区 (性能优化: 避免每帧 malloc) */
    struct {
        uint16_t x_start;
        uint16_t x_end;
        uint16_t y;
        int32_t  region_id;
    }* runs_buf;        /* 水平段缓冲区 */
    uint32_t        runs_buf_size;
    struct {
        uint16_t x_min, x_max, y_min, y_max;
        uint32_t pixel_count;
    }* regions_buf;     /* 连通区域缓冲区 */
    uint32_t        regions_buf_size;
    NPERegionDesc*  descs_buf;  /* 描述符输出缓冲区 */
    uint16_t        descs_buf_size;
    /* 行索引 (加速区域合并, 避免全量搜索) */
    uint16_t*       row_start;   /* 每行第一个 run 的索引 */
    uint16_t        row_start_size;
} NPEEPA;

/* ========== EPA API ========== */

/* 创建 EPA 引擎
 * @param cfg  配置
 * @return EPA 实例 (失败返回 NULL)
 *
 * 内部创建 NPEngineCore (1D 拓扑, 域=uint8_t 编码模式)
 * MCU 模式下 max_regions 受 NPE_EPA_MCU_MAX_REGIONS 限制
 */
NPEEPA* npe_epa_create(const NPEEPAConfig* cfg);

void npe_epa_destroy(NPEEPA* epa);

void npe_epa_reset(NPEEPA* epa);

/* EPA 处理: 消费 IPA 的变化掩码, 输出编码事件流
 *
 * @param epa          EPA 引擎
 * @param ipa_core     IPA 引擎 (用于提取区域描述符, 复用 npe_core_extract_regions)
 * @param input        当前帧输入 (与 IPA 共享, 用于采样区域值)
 * @param events       输出事件数组
 * @param max_events   事件数组容量
 * @param event_count  输出事件数量
 * @return 0 成功, <0 错误
 *
 * 处理流程 (NPE 规则):
 *   1. 调用 npe_core_extract_regions 提取 IPA 的连通区域
 *   2. 对每个区域:
 *      a. 分析特征 (像素数, 值一致性)
 *      b. 决策最优编码模式 (仅比较)
 *      c. 与 last_modes[id] 比较
 *      d. 模式变化 → mode_changed=1 (渗出)
 *      e. 模式不变 → mode_changed=0 (阻挡)
 *      f. 更新 last_modes[id]
 */
int npe_epa_process(NPEEPA* epa,
                     NPEngineCore* ipa_core,
                     const NPE_STATE_TYPE* input,
                     NPEEncodeEvent* events,
                     uint16_t max_events,
                     uint16_t* event_count);

/* EPA 统计: 编码模式分布 (用于调试和优化) */
typedef struct {
    uint32_t total_regions;     /* 累计处理的区域数 */
    uint32_t mode_changed;      /* 累计模式切换次数 (渗出) */
    uint32_t mode_blocked;      /* 累计模式阻挡次数 */
    uint32_t raw_count;
    uint32_t rle_count;
    uint32_t rect_count;
    uint32_t point_count;
} NPEEPAStats;

void npe_epa_get_stats(const NPEEPA* epa, NPEEPAStats* stats);

#ifdef __cplusplus
}
#endif

#endif
