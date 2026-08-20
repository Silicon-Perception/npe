/**
 * NPE 4.0 — 过滤层
 *
 * 三个串行过滤器, 各有独立开关:
 *   层A: 人眼感知过滤 (perceptual) — 感知无差异的亮度变化剔除
 *   层B: 默认值过滤 (default)      — 背景色/主色调免传输
 *   层C: 出口采样 (sampling)        — 隔行/隔列/隔行+隔列
 *
 * 架构位置: IPA引擎之后, EPA编码之前
 *
 * 零计算原则:
 *   - 层A: 查表 + 比较
 *   - 层B: 仅比较
 *   - 层C: 掩码测试
 *
 * MCU 兼容: 无动态内存, 无SIMD
 */

#ifndef NPE_FILTER_H
#define NPE_FILTER_H

#include "npe/internal/npe_types.h"
#include "npe/internal/npe_core.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ========== 采样模式 (层C) ========== */

typedef enum {
    NPE_SAMPLE_NONE    = 0,  /* 全采样 */
    NPE_SAMPLE_INTERLACE = 1,  /* 隔行 (偶数行) */
    NPE_SAMPLE_COLUMN  = 2,  /* 隔列 (偶数列) */
    NPE_SAMPLE_BOTH    = 3,  /* 隔行+隔列 (棋盘格) */
} NPESampleMode;

/* ========== 过滤配置 ========== */

typedef struct {
    /* 层A: 人眼感知过滤 */
    uint8_t  enable_perceptual;     /* 开关: 0=关, 1=开 */
    /* 层B: 默认值过滤 */
    uint8_t  enable_default;        /* 开关: 0=关, 1=开 */
    uint8_t  default_value;         /* 默认值 (背景色) */
    /* 层C: 出口采样 */
    uint8_t  enable_sampling;       /* 开关: 0=关, 1=开 */
    uint8_t  sample_mode;           /* NPESampleMode */
} NPEFilterConfig;

/* ========== 过滤结果统计 ========== */

typedef struct {
    uint32_t input_count;       /* 过滤前变化像素数 */
    uint32_t after_perceptual;  /* 层A后 */
    uint32_t after_default;     /* 层B后 */
    uint32_t after_sampling;    /* 层C后 */
    uint32_t filtered_total;    /* 总过滤数 */
} NPEFilterStats;

/* ========== 过滤层上下文 ========== */

typedef struct {
    NPEFilterConfig config;
    uint16_t width;
    uint16_t height;
    /* 感知阈值表 (256项, 预设) */
    const uint8_t* perceptual_table;
    /* 采样掩码 (位图, 每像素1bit) */
    npemask_t* sample_mask;
    uint32_t   sample_mask_words;
    /* 统计 */
    NPEFilterStats stats;
} NPEFilter;

/* ========== API ========== */

/* 创建过滤层
 * @param cfg    配置 (各层开关)
 * @param width  图像宽度
 * @param height 图像高度
 */
NPEFilter* npe_filter_create(const NPEFilterConfig* cfg, uint16_t width, uint16_t height);

void npe_filter_destroy(NPEFilter* flt);

/* 重置统计 */
void npe_filter_reset(NPEFilter* flt);

/* 动态修改配置 (支持运行时开关切换) */
void npe_filter_set_config(NPEFilter* flt, const NPEFilterConfig* cfg);

/* 重建采样掩码 (采样模式变化时调用) */
void npe_filter_rebuild_sample_mask(NPEFilter* flt);

/* ========== 核心过滤 API ========== */

/* 对 IPA 输出的变化掩码进行过滤
 *
 * @param flt        过滤层
 * @param mask_read  IPA的变化掩码 (输入, 会被修改为过滤后的掩码)
 * @param input      当前帧输入 (用于层B默认值判断)
 * @param prev_state 上一帧亮度值 (用于层A感知判断, IPA处理前备份)
 * @param eng        IPA引擎 (用于获取total_units等元信息)
 *
 * 处理流程:
 *   1. 层A: 遍历变化像素, |input[i]-prev_state[i]| < 感知阈值 → 清除掩码位
 *   2. 层B: 遍历变化像素, input[i] == 默认值 → 清除掩码位
 *   3. 层C: 遍历所有位, 未采样位置 → 清除掩码位
 *
 * 注意: 直接修改 mask_read, 过滤后的掩码可直接用于EPA
 */
void npe_filter_apply(NPEFilter* flt, npemask_t* mask_read,
                       const NPE_STATE_TYPE* input,
                       const NPE_STATE_TYPE* prev_state,
                       NPEngineCore* eng);

/* ========== 感知阈值表预设 ========== */

/* 标准人眼感知阈值表 (Weber-Fechner近似)
 * 暗部阈值低 (1-2), 亮部阈值高 (4-8)
 */
extern const uint8_t npe_perceptual_table_standard[256];

/* 高灵敏度表 (阈值全1, 几乎不过滤) */
extern const uint8_t npe_perceptual_table_sensitive[256];

/* 低灵敏度表 (阈值较高, 激进过滤) */
extern const uint8_t npe_perceptual_table_aggressive[256];

#ifdef __cplusplus
}
#endif

#endif
