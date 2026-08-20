/**
 * NPE 4.2 — 管道聚合层 (Aggregation Layer)
 *
 * 设计哲学:
 *   多管道汇流 → 聚合管道计算 → 单值输出
 *   物理类比: 多条支流汇集到蓄水池, 出口只输出统计值
 *
 *   pipe[0] ─┐
 *   pipe[1] ─┤
 *   pipe[2] ─┼─→ 聚合管道 ─→ MAX/MIN/AVG/SUM/COUNT
 *   pipe[3] ─┤
 *   ...     ─┘
 *
 * 带宽节省: N个原始管道 → 1个聚合值
 *
 * 约束:
 *   - O(K) 每聚合管道 (K=源管道数, 自然能力)
 *   - 零计算原则: 仅比较和累加, 无乘除
 *   - 不修改NPE核心引擎 (外围扩展层)
 *
 * 专利边界: 属于NPE引擎范畴 (管道拓扑操作)
 */

#ifndef NPE_AGGREGATE_H
#define NPE_AGGREGATE_H

#include "npe/internal/npe_types.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ========== 聚合操作类型 ========== */

typedef enum {
    NPE_AGG_MAX    = 0,  /* 最大值 */
    NPE_AGG_MIN    = 1,  /* 最小值 */
    NPE_AGG_SUM    = 2,  /* 求和 */
    NPE_AGG_AVG    = 3,  /* 平均值 (SUM/COUNT, 仅整数除法) */
    NPE_AGG_COUNT  = 4,  /* 非零计数 */
} NPEAggOp;

/* ========== 聚合管道 ========== */

#define NPE_AGG_MAX_SOURCES 256  /* 每个聚合管道最多256个源 */

typedef struct {
    NPEAggOp op;                            /* 聚合操作 */
    uint32_t source_indices[NPE_AGG_MAX_SOURCES]; /* 源管道索引 */
    uint32_t source_count;                  /* 源管道数量 */
    double   last_value;                     /* 上次聚合值 */
    uint8_t  initialized;                   /* 是否已初始化 */
    uint64_t update_count;                   /* 更新次数 */
} NPEAggPipe;

typedef struct {
    NPEAggPipe* pipes;
    uint32_t    count;       /* 当前聚合管道数 */
    uint32_t    capacity;    /* 最大容量 */
} NPEAggLayer;

/* ========== API ========== */

/**
 * 创建聚合层
 * @param capacity 最大聚合管道数
 */
NPEAggLayer* npe_aggregate_create(uint32_t capacity);

void npe_aggregate_destroy(NPEAggLayer* layer);

/**
 * 添加聚合管道
 * @param layer         聚合层
 * @param op            聚合操作
 * @param source_indices 源管道索引数组
 * @param source_count   源管道数量
 * @return >=0 聚合管道ID, <0 失败
 */
int npe_aggregate_add_pipe(NPEAggLayer* layer, NPEAggOp op,
                            const uint32_t* source_indices,
                            uint32_t source_count);

/**
 * 处理聚合层: 遍历所有聚合管道, 计算聚合值
 * @param layer        聚合层
 * @param pipe_values  原始管道值数组
 * @param pipe_count   原始管道数量
 * @param outputs      输出聚合值数组 (需capacity个元素)
 * @param change_mask  输出变化掩码 (1=聚合值变化, 0=不变)
 * @return 变化的聚合管道数
 */
uint32_t npe_aggregate_process(NPEAggLayer* layer,
                                 const double* pipe_values,
                                 uint32_t pipe_count,
                                 double* outputs,
                                 uint8_t* change_mask);

/**
 * 获取聚合管道的当前值
 */
double npe_aggregate_get_value(const NPEAggLayer* layer, uint32_t agg_id);

/**
 * 重置聚合层
 */
void npe_aggregate_reset(NPEAggLayer* layer);

#ifdef __cplusplus
}
#endif

#endif
