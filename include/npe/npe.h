#ifndef NPE_H
#define NPE_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define NPE_VERSION_MAJOR 4
#define NPE_VERSION_MINOR 1
#define NPE_VERSION_PATCH 0
#define NPE_VERSION_STRING "4.1.0"

#define NPE_MAX_UNITS  (1920 * 1080)

typedef uint64_t npemask_t;

/* NPEFlag 扩展: 数据类型
 *  0x01=NPE_DOMAIN_U8 (已有), 
 *  0x02=NPE_DOMAIN_F32 (新增, float)
 *  0x04=NPE_DOMAIN_F64 (新增, double)
 *  0x08=NPE_PARALLEL_ENABLE (新增, 启用OpenMP并行)
 */
#define NPE_DOMAIN_U8          0x01
#define NPE_DOMAIN_F32         0x02
#define NPE_DOMAIN_F64         0x04
#define NPE_PARALLEL_ENABLE    0x08

/* ========== Layer A: NPE Core API — 通用自然管道引擎 ========== */

typedef struct NPEngine_s NPEEngine;

/* 批处理项: 用于多引擎并行调度 */
typedef struct {
    NPEEngine* engine;       /* 引擎句柄 */
    const void* input;       /* 输入 (uint8_t/float/double, 依引擎类型) */
    void*       output;      /* 输出 (同输入类型) */
    uint32_t*   change_count;/* 变化计数输出 */
    int32_t     result;      /* 该任务的返回值 (0=成功) */
} NPEBatchTask;

typedef struct {
    uint32_t total_units;
    uint8_t  dimensions;
    uint16_t dim_size[3];
    uint8_t  flags;
} NPEConfig;

typedef struct {
    uint8_t enable_energy;
    uint8_t enable_direction;
    uint8_t enable_decay;
    uint8_t enable_flood;
    uint8_t enable_dilate;

    uint8_t energy_threshold;
    uint8_t decay_shift;
    uint8_t flood_iterations;
    uint8_t spatial_radius;
    uint16_t threshold;
} NPEEngineCapability;

typedef enum {
    NPE_DESC_POINT    = 0,
    NPE_DESC_RECT     = 1,
    NPE_DESC_RECT_GRAD = 2,
} NPEDescType;

#pragma pack(push, 1)
typedef struct {
    uint8_t  type;
    uint16_t x;
    uint16_t y;
    uint16_t width;
    uint16_t height;
    uint8_t  value;
    uint8_t  value_end;
    uint32_t pixel_count;
} NPERegionDesc;
#pragma pack(pop)

NPEEngine* npe_engine_create(const NPEConfig* cfg, const NPEEngineCapability* cap);
void npe_engine_destroy(NPEEngine* eng);
int npe_engine_process(NPEEngine* eng, const uint8_t* input, uint8_t* output, uint32_t* change_count);

/* float/double 类型原生支持: 避免 HMG 场景下类型转换开销 */
int npe_engine_process_f32(NPEEngine* eng, const float* input, float* output, uint32_t* change_count);
int npe_engine_process_f64(NPEEngine* eng, const double* input, double* output, uint32_t* change_count);

int npe_engine_extract_regions(NPEEngine* eng, const uint8_t* input, NPERegionDesc* descs, uint16_t max_descs, uint16_t* desc_count);
void npe_engine_reset(NPEEngine* eng);
void npe_engine_set_threshold(NPEEngine* eng, uint16_t threshold);
const npemask_t* npe_engine_get_mask(NPEEngine* eng);
uint32_t npe_engine_get_mask_words(NPEEngine* eng);

/* ========== 批处理 API: 多引擎并行调度 (天然多实例并行) ========== */

/**
 * 批量处理多个引擎任务, 内部使用 OpenMP 实现真正的多实例并行
 * 
 * @param tasks       任务数组, 每个任务包含引擎、输入输出缓冲区
 * @param task_count  任务数量 (建议: 4-16, 匹配 CPU 核心数)
 * @return 0 成功, <0 失败 (tasks[i].result 包含每个任务的单独返回值)
 *
 * 典型场景: HMG 多实例状态检测、多路视频流并行编码
 * 性能: 8核CPU下 8实例 ≈ 单实例 × 7.5x (近似线性加速)
 */
int npe_batch_process(NPEBatchTask* tasks, uint32_t task_count);

/* ========== 版本信息 ========== */

const char* npe_version_string(void);

static inline void npe_version(int* major, int* minor, int* patch) {
    if (major) *major = NPE_VERSION_MAJOR;
    if (minor) *minor = NPE_VERSION_MINOR;
    if (patch) *patch = NPE_VERSION_PATCH;
}

#ifdef __cplusplus
}
#endif

#endif