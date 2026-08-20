/**
 * NPE 4.2 — 管道阈值自适应 (Hebb 学习)
 *
 * 设计哲学:
 *   阈值 = RO膜孔径 (自然能力, 非人为干预)
 *   Hebb规则: "一起激发的管道, 阈值一起调整"
 *   - 频繁渗出(活跃)的管道 → 阈值升高 (降低敏感度, 过滤噪声)
 *   - 频繁阻挡(静止)的管道 → 阈值降低 (提高敏感度, 捕捉微变化)
 *
 * 约束:
 *   - O(1) 每管道 (自然能力)
 *   - 不覆盖用户手动设置的阈值 (manual flag)
 *   - 定点运算 (MCU友好, 无浮点)
 *   - 不修改NPE核心引擎逻辑 (外围扩展)
 *
 * 专利边界: 属于NPE引擎范畴 (阈值是NPE的自然能力参数)
 */

#ifndef NPE_ADAPTIVE_H
#define NPE_ADAPTIVE_H

#include "npe/internal/npe_types.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ========== 自适应阈值上下文 ========== */

typedef struct {
    uint16_t* thresholds;        /* 每管道自适应阈值 */
    uint8_t*  manual_flags;      /* 1=手动设置, 不自适应 */
    uint16_t* activity_ema;       /* 活动度EMA (Q8定点: 0-256 = 0.0-1.0) */
    uint32_t  pipe_count;

    uint16_t  base_threshold;     /* 基准阈值 (初始值) */
    uint16_t  min_threshold;      /* 阈值下限 */
    uint16_t  max_threshold;      /* 阈值上限 */
    uint8_t   learning_rate;      /* 学习率 (Q8: 0-256, 如 16=0.0625) */
    uint8_t   target_activity;    /* 目标活动度 (Q8: 如 26≈10%) */
    uint8_t   ema_alpha;          /* EMA平滑系数 (Q8: 如 32=0.125) */
} NPEAdaptiveThreshold;

/* ========== API ========== */

/**
 * 创建自适应阈值上下文
 * @param pipe_count     管道数量
 * @param base_threshold 基准阈值
 * @param min_threshold  最小阈值
 * @param max_threshold  最大阈值
 */
NPEAdaptiveThreshold* npe_adaptive_create(uint32_t pipe_count,
                                            uint16_t base_threshold,
                                            uint16_t min_threshold,
                                            uint16_t max_threshold);

void npe_adaptive_destroy(NPEAdaptiveThreshold* ctx);

/**
 * 判断管道是否应渗出 (Hebb自适应阈值)
 * @param ctx    自适应上下文
 * @param index  管道索引
 * @param input  输入值
 * @param state  当前状态值
 * @return true=应渗出(变化), false=应阻挡(不变)
 */
bool npe_adaptive_should_exude(const NPEAdaptiveThreshold* ctx,
                                uint32_t index,
                                uint8_t input, uint8_t state);

/**
 * 更新管道活动度 (在渗出/阻挡后调用)
 * Hebb学习规则: 根据活动度调整阈值
 * @param ctx    自适应上下文
 * @param index  管道索引
 * @param exuded 是否渗出 (true=渗出, false=阻挡)
 */
void npe_adaptive_update(NPEAdaptiveThreshold* ctx,
                          uint32_t index, bool exuded);

/**
 * 手动设置阈值 (标记为manual, 不再自适应)
 */
void npe_adaptive_set_manual(NPEAdaptiveThreshold* ctx,
                              uint32_t index, uint16_t threshold);

/**
 * 解除手动标记 (恢复自适应)
 */
void npe_adaptive_clear_manual(NPEAdaptiveThreshold* ctx, uint32_t index);

/**
 * 获取当前阈值
 */
uint16_t npe_adaptive_get_threshold(const NPEAdaptiveThreshold* ctx,
                                      uint32_t index);

/**
 * 获取管道活动度 (Q8定点: 0-256 = 0.0-1.0)
 */
uint16_t npe_adaptive_get_activity(const NPEAdaptiveThreshold* ctx,
                                     uint32_t index);

/**
 * 批量自适应处理: 遍历所有管道, 根据变化掩码更新阈值
 * @param ctx         自适应上下文
 * @param change_mask 变化掩码 (1=渗出, 0=阻挡)
 * @param pipe_count  管道数量
 */
void npe_adaptive_batch_update(NPEAdaptiveThreshold* ctx,
                                const uint8_t* change_mask,
                                uint32_t pipe_count);

/**
 * 重置所有阈值到基准值
 */
void npe_adaptive_reset(NPEAdaptiveThreshold* ctx);

#ifdef __cplusplus
}
#endif

#endif
