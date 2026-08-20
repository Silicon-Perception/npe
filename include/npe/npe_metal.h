#ifndef NPE_METAL_H
#define NPE_METAL_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * NPE Metal GPU 加速接口
 * 
 * 功能: 将 NPE 计算密集型操作 (IPA/Filter) 卸载到 GPU
 * 平台: macOS (Metal), iOS
 * 后端: 当 Metal 不可用时自动回退到 CPU
 */

/** GPU 上下文句柄 */
typedef struct npe_metal_context* npe_metal_context_t;

/** GPU 缓冲区描述 */
typedef struct {
    void* gpu_buffer;       /**< Metal MTLBuffer 指针 */
    uint8_t* cpu_mapping;   /**< CPU 映射 (可直接读写) */
    size_t size;            /**< 缓冲区大小 (bytes) */
} npe_gpu_buffer_t;

/** 初始化 Metal 上下文 */
npe_metal_context_t npe_metal_init(void);

/** 销毁 Metal 上下文 */
void npe_metal_destroy(npe_metal_context_t ctx);

/** 检查 Metal 是否可用 */
int npe_metal_available(void);

/**
 * 在 GPU 上执行 IPA (像素变化检测)
 * 
 * @param ctx      Metal 上下文
 * @param prev     上一帧灰度数据 (uint8)
 * @param curr     当前帧灰度数据 (uint8)
 * @param mask_out 输出掩码缓冲区 (uint8, 0=无变化, 1=有变化)
 * @param width    图像宽度
 * @param height   图像高度
 * @param threshold 变化阈值
 * @return 变化像素数量, -1 失败
 */
int32_t npe_metal_ipa(
    npe_metal_context_t ctx,
    const uint8_t* prev,
    const uint8_t* curr,
    uint8_t* mask_out,
    uint32_t width,
    uint32_t height,
    uint32_t threshold
);

/**
 * 在 GPU 上执行 IPA + Filter 管道
 * 
 * @param ctx        Metal 上下文
 * @param prev       上一帧灰度数据
 * @param curr       当前帧灰度数据
 * @param mask_out   输出掩码 (经 Flood + Energy 过滤)
 * @param width      图像宽度
 * @param height     图像高度
 * @param threshold  变化阈值
 * @param energy_threshold 能量过滤阈值 (0=关闭)
 * @return 变化像素数量, -1 失败
 */
int32_t npe_metal_pipeline(
    npe_metal_context_t ctx,
    const uint8_t* prev,
    const uint8_t* curr,
    uint8_t* mask_out,
    uint32_t width,
    uint32_t height,
    uint32_t threshold,
    uint32_t energy_threshold
);

/**
 * 预分配 GPU 缓冲区 (零拷贝模式)
 * 分配后 cpu_mapping 可直接读写, 数据自动同步到 GPU
 */
npe_gpu_buffer_t npe_metal_create_buffer(
    npe_metal_context_t ctx,
    size_t size
);

/** 释放 GPU 缓冲区 */
void npe_metal_destroy_buffer(
    npe_metal_context_t ctx,
    npe_gpu_buffer_t* buf
);

/**
 * 使用预分配缓冲区执行 IPA (零拷贝)
 * 
 * @param ctx        Metal 上下文
 * @param prev_buf   预分配的上一帧缓冲区
 * @param curr_buf   预分配的当前帧缓冲区
 * @param mask_buf   预分配的输出掩码缓冲区
 * @param width      图像宽度
 * @param height     图像高度
 * @param threshold  变化阈值
 * @return 变化像素数量, -1 失败
 */
int32_t npe_metal_ipa_zero_copy(
    npe_metal_context_t ctx,
    npe_gpu_buffer_t* prev_buf,
    npe_gpu_buffer_t* curr_buf,
    npe_gpu_buffer_t* mask_buf,
    uint32_t width,
    uint32_t height,
    uint32_t threshold
);

#ifdef __cplusplus
}
#endif

#endif /* NPE_METAL_H */