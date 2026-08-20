/**
 * NPE Core — 自然管道架构 (v4.1 深度优化版)
 *
 * NPE 不是一个图像处理算法，而是一个通用的时空计算架构。
 * 核心思想：管道单元保持状态 → 仅比较(相等/不等) → 变化才渗出 → 邻域自然感知
 * ⚠ NPE 从不计算差值 (delta/diff)，只做等值比较 + 渗出原始值。
 *
 * 优化:
 *   - 位压缩掩码 (uint64_t per 64 units)
 *   - 双缓冲切换 (flood/dilate 零 memcpy)
 *   - Fast Path: 2D 网格硬编码邻域 (消除函数指针)
 *   - Fast Path: uint8_t 域内联比较 (消除函数指针)
 *   - 线性扫描 (编译器可自动向量化)
 */

#ifndef NPE_CORE_H
#define NPE_CORE_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef NPE_STATE_TYPE
#define NPE_STATE_TYPE uint8_t
#endif

/* 位压缩掩码类型 */
#ifndef npemask_t
typedef uint64_t npemask_t;
#endif
#define NPE_MASK_BITS 64

/* 拓扑标志: 指示可使用的快速路径 */
#define NPE_TOPO_2D_GRID    0x01  /* 2D 网格布局, 可硬编码邻域 */
#define NPE_DOMAIN_U8       0x02  /* uint8_t 域, 可内联 abs_diff/cmp_ge */

/* ========== 前向声明 ========== */

struct NPEngineCore_s;
typedef struct NPEngineCore_s NPEngineCore;

/* ========== 核心数据结构 ========== */

/* NPEUnit: 管道单元
 *
 * bit位复用设计:
 *   state   = 亮度值 (管道比较用, 0-255, 独立字段不破坏引擎逻辑)
 *   packed  = bit位复用字段 [标记3bit][状态2bit][保留3bit]
 *     - 标记色 (3bit): 8种标记, 支持状态5-7映射 (project_memory要求)
 *     - 管道状态 (2bit): 未初始化/渗出/阻挡/衰减
 *     - 保留 (3bit): 架构预留 (未来NPE-E/F管道, 链路自适应)
 *
 * 传输时: state按位深打包成位流, packed随像素"免费"传输
 */
typedef struct {
    NPE_STATE_TYPE state;     /* 亮度值 (管道比较用) */
    uint8_t packed;           /* bit位复用: [标记3bit][状态2bit][保留3bit] */
    uint8_t initialized;
} NPEUnit;

/* packed 字段 bit 位定义 */
#define NPE_PACK_MARKER_BITS    3
#define NPE_PACK_STATE_BITS     2
#define NPE_PACK_RESERVED_BITS  3
#define NPE_PACK_MARKER_SHIFT   0
#define NPE_PACK_STATE_SHIFT    NPE_PACK_MARKER_BITS
#define NPE_PACK_RESERVED_SHIFT (NPE_PACK_MARKER_BITS + NPE_PACK_STATE_BITS)

#define NPE_PACK_MARKER_MASK    ((1 << NPE_PACK_MARKER_BITS) - 1)
#define NPE_PACK_STATE_MASK     ((1 << NPE_PACK_STATE_BITS) - 1)
#define NPE_PACK_RESERVED_MASK  ((1 << NPE_PACK_RESERVED_BITS) - 1)

/* 标记色枚举 (3bit, 8种) */
typedef enum {
    NPE_MARKER_NONE      = 0,  /* 无标记 */
    NPE_MARKER_DEFAULT   = 1,  /* 默认值 (背景色) */
    NPE_MARKER_REGION_A  = 2,  /* 区域A */
    NPE_MARKER_REGION_B  = 3,  /* 区域B */
    NPE_MARKER_REGION_C  = 4,  /* 区域C */
    NPE_MARKER_STATE_5   = 5,  /* 状态5 (project_memory: 状态5-7映射) */
    NPE_MARKER_STATE_6   = 6,  /* 状态6 */
    NPE_MARKER_STATE_7   = 7,  /* 状态7 */
} NPEMarker;

/* 管道状态枚举 (2bit, 4种) */
typedef enum {
    NPE_PIPE_UNINIT   = 0,  /* 未初始化 */
    NPE_PIPE_EXUDE    = 1,  /* 渗出 (变化, 传输) */
    NPE_PIPE_BLOCK    = 2,  /* 阻挡 (无变化, 不传) */
    NPE_PIPE_DECAY    = 3,  /* 衰减 */
} NPEPipeState;

/* packed 字段访问内联函数 */
static inline uint8_t npe_unit_get_marker(const NPEUnit* u) {
    return (u->packed >> NPE_PACK_MARKER_SHIFT) & NPE_PACK_MARKER_MASK;
}
static inline uint8_t npe_unit_get_pipe_state(const NPEUnit* u) {
    return (u->packed >> NPE_PACK_STATE_SHIFT) & NPE_PACK_STATE_MASK;
}
static inline uint8_t npe_unit_get_reserved(const NPEUnit* u) {
    return (u->packed >> NPE_PACK_RESERVED_SHIFT) & NPE_PACK_RESERVED_MASK;
}
static inline void npe_unit_set_marker(NPEUnit* u, uint8_t m) {
    u->packed = (uint8_t)((u->packed & ~(NPE_PACK_MARKER_MASK << NPE_PACK_MARKER_SHIFT)) |
                          ((m & NPE_PACK_MARKER_MASK) << NPE_PACK_MARKER_SHIFT));
}
static inline void npe_unit_set_pipe_state(NPEUnit* u, uint8_t s) {
    u->packed = (uint8_t)((u->packed & ~(NPE_PACK_STATE_MASK << NPE_PACK_STATE_SHIFT)) |
                          ((s & NPE_PACK_STATE_MASK) << NPE_PACK_STATE_SHIFT));
}
static inline void npe_unit_set_reserved(NPEUnit* u, uint8_t r) {
    u->packed = (uint8_t)((u->packed & ~(NPE_PACK_RESERVED_MASK << NPE_PACK_RESERVED_SHIFT)) |
                          ((r & NPE_PACK_RESERVED_MASK) << NPE_PACK_RESERVED_SHIFT));
}

typedef struct {
    uint32_t total_units;
    uint8_t  dimensions;
    uint16_t dim_size[3];
    uint8_t  flags;

    uint8_t  (*get_neighbors)(uint32_t idx, uint32_t* nbrs,
                              uint8_t max_n, void* context);
} NPETopology;

typedef struct {
    NPE_STATE_TYPE (*abs_diff)(NPE_STATE_TYPE a, NPE_STATE_TYPE b);
    bool (*cmp_ge)(NPE_STATE_TYPE a, NPE_STATE_TYPE b);
} NPEDomain;

typedef struct {
    uint8_t enable_energy;
    uint8_t enable_direction;
    uint8_t enable_decay;
    uint8_t enable_flood;
    uint8_t enable_dilate;
    uint8_t enable_compensate;  /* 补偿输出模式: 连通区域超阈值时用描述参数替代逐管道输出 */

    uint8_t energy_threshold;
    uint8_t decay_shift;
    uint8_t flood_iterations;
    uint8_t spatial_radius;
    uint32_t compensate_threshold;  /* 连通区域像素数超过此值时启动补偿输出 */
} NPECapability;

/* ========== 补偿输出描述符 ========== */

/* 描述符类型: 用少量参数描述一片连通变化区域 */
#ifndef NPE_DESC_POINT
typedef enum {
    NPE_DESC_POINT    = 0,  /* 单点: (x, y, value) */
    NPE_DESC_RECT     = 1,  /* 矩形: (x, y, w, h, value) — 区域内值一致 */
    NPE_DESC_RECT_GRAD = 2, /* 梯度矩形: (x, y, w, h, v0, v1) — 值从v0到v1渐变 */
} NPEDescType;
#endif

#pragma pack(push, 1)
#ifndef NPEREGION_DESC_DEFINED
#define NPEREGION_DESC_DEFINED
typedef struct {
    uint8_t  type;        /* NPEDescType */
    uint16_t x;           /* 区域左上角 x */
    uint16_t y;           /* 区域左上角 y */
    uint16_t width;       /* 区域宽度 */
    uint16_t height;      /* 区域高度 */
    uint8_t  value;       /* 区域值 (RECT模式) 或起始值 (GRAD模式) */
    uint8_t  value_end;   /* 结束值 (仅GRAD模式) */
    uint32_t pixel_count; /* 区域内像素数 */
} NPERegionDesc;
#endif
#pragma pack(pop)

/* ========== Core API ========== */

NPEngineCore* npe_core_create(const NPETopology* topo,
                               const NPEDomain* domain,
                               const NPECapability* cap,
                               void* topo_context);

void npe_core_destroy(NPEngineCore* eng);

int npe_core_process(NPEngineCore* eng,
                      const NPE_STATE_TYPE* input,
                      NPE_STATE_TYPE* output,
                      uint32_t* change_count);

/* f32/f64 原生处理: 变化检测(相等比较)按类型实现, 自然能力阶段完全共享
 * (能量/方向/洪泛/膨胀只操作 mask, 与数据类型无关)
 * ⚠ 仅比较 |输入 - 状态| ≥ eps  →  设置渗出位, 从不输出差值
 */
int npe_core_process_f32(NPEngineCore* eng,
                          const float* input, float* output,
                          uint32_t* change_count);
int npe_core_process_f64(NPEngineCore* eng,
                          const double* input, double* output,
                          uint32_t* change_count);

/* 补偿输出模式: 将变化掩码分析为区域描述符
 * @param eng           引擎
 * @param input         当前帧 (用于提取区域值)
 * @param descs         输出描述符数组
 * @param max_descs     描述符数组容量
 * @param desc_count    输出描述符数量
 * @return 0 成功
 */
int npe_core_extract_regions(NPEngineCore* eng,
                              const NPE_STATE_TYPE* input,
                              NPERegionDesc* descs,
                              uint16_t max_descs,
                              uint16_t* desc_count);

void npe_core_reset(NPEngineCore* eng);
void npe_core_set_threshold(NPEngineCore* eng, uint16_t threshold);

/* 位压缩掩码操作内联函数 */
static inline void npe_mask_set(npemask_t* mask, uint32_t idx) {
    mask[idx / NPE_MASK_BITS] |= (npemask_t)1 << (idx % NPE_MASK_BITS);
}

static inline void npe_mask_clear(npemask_t* mask, uint32_t idx) {
    mask[idx / NPE_MASK_BITS] &= ~((npemask_t)1 << (idx % NPE_MASK_BITS));
}

static inline bool npe_mask_test(const npemask_t* mask, uint32_t idx) {
    return (mask[idx / NPE_MASK_BITS] >> (idx % NPE_MASK_BITS)) & 1;
}

/* ========== 完整结构定义 ========== */
struct NPEngineCore_s {
    NPETopology    topo;
    NPEDomain      domain;
    NPECapability  cap;
    void*          topo_context;

    NPEUnit*       units;

    npemask_t*     mask_read;
    npemask_t*     mask_write;
    npemask_t*     mask_temp;

    /* f32/f64 原生状态缓存: 与 units 独立, 供 process_f32/f64 使用 */
    float*         f32_states;
    double*        f64_states;
    uint8_t*       f32_init;
    uint8_t*       f64_init;

    uint32_t       total_units;
    uint32_t       mask_words;
    uint32_t       frame;
    uint16_t       threshold;
    uint8_t        initialized;

    /* 预解析快速路径函数 */
    void (*fn_apply_energy)(NPEngineCore* eng);
    void (*fn_apply_direction)(NPEngineCore* eng);
    void (*fn_apply_flood)(NPEngineCore* eng);
    void (*fn_apply_dilate)(NPEngineCore* eng);
};

#ifdef __cplusplus
}
#endif

#endif
