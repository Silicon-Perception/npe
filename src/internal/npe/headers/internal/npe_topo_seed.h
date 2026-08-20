/**
 * NPE 4.2 — 拓扑特征跳频种子 (Topology-Feature Hopping Seed)
 *
 * 设计哲学:
 *   NPE管道的空间拓扑蕴含独特特征 → 提取为跳频种子
 *   即使密钥泄露, 拓扑特征仍提供额外熵源
 *
 *   管道[42]的邻居是 [41, 43, 142, 242] → 特征哈希 → 跳频种子
 *   不同拓扑布局 → 不同种子 → 不同跳频序列
 *
 * 拓扑特征来源:
 *   1. 坐标特征: (x, y) 位置哈希
 *   2. 邻居特征: 邻居数量 + 邻居ID哈希
 *   3. 区域特征: 所属区域ID
 *   4. 度特征: 管道连接度 (入度/出度)
 *
 * 约束:
 *   - O(1) 每管道 (特征提取是局部操作)
 *   - 不修改NPE核心引擎 (读取拓扑信息)
 *
 * 专利边界: NPE拓扑特征是NPE自然能力, 跳频种子是NPE→NPP桥接
 */

#ifndef NPE_TOPO_SEED_H
#define NPE_TOPO_SEED_H

#include "npe/internal/npe_types.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ========== 拓扑特征描述符 ========== */

typedef struct {
    uint32_t pipe_index;       /* 管道索引 */
    uint16_t x;                /* X坐标 (2D网格) */
    uint16_t y;                /* Y坐标 (2D网格) */
    uint8_t  neighbor_count;   /* 邻居数量 */
    uint8_t  degree;            /* 连接度 */
    uint16_t region_id;        /* 所属区域 */
    uint32_t coord_hash;       /* 坐标哈希 */
    uint32_t neighbor_hash;    /* 邻居特征哈希 */
    uint32_t feature_hash;     /* 综合特征哈希 */
} NPETopoFeature;

typedef struct {
    NPETopoFeature* features;
    uint32_t        count;
    uint16_t        width;
    uint16_t        height;
} NPETopoFeatureSet;

/* ========== API ========== */

/**
 * 为2D网格拓扑提取特征
 * @param width  网格宽度
 * @param height 网格高度
 */
NPETopoFeatureSet* npe_topo_seed_extract_grid(uint16_t width, uint16_t height);

/**
 * 为任意拓扑提取特征
 * @param pipe_count    管道数
 * @param get_neighbors 邻居查询函数
 * @param context       上下文
 */
NPETopoFeatureSet* npe_topo_seed_extract_custom(
    uint32_t pipe_count,
    uint8_t (*get_neighbors)(uint32_t idx, uint32_t* nbrs, uint8_t max_n, void* ctx),
    void* context);

void npe_topo_seed_destroy(NPETopoFeatureSet* set);

/**
 * 计算管道的跳频种子
 * @param feature 管道拓扑特征
 * @param epoch   时间epoch
 * @return 32位跳频种子
 */
uint32_t npe_topo_seed_compute(const NPETopoFeature* feature, uint32_t epoch);

/**
 * 批量计算跳频种子
 * @param set     特征集
 * @param epoch   时间epoch
 * @param seeds   输出种子数组 (需count个元素)
 */
void npe_topo_seed_compute_batch(const NPETopoFeatureSet* set,
                                   uint32_t epoch,
                                   uint32_t* seeds);

/**
 * 获取管道的特征哈希 (不含epoch, 用于静态标识)
 */
uint32_t npe_topo_seed_get_hash(const NPETopoFeature* feature);

#ifdef __cplusplus
}
#endif

#endif
