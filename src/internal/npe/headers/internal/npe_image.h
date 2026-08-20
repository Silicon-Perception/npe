/**
 * NPE Image Adapter — 图像处理适配器
 *
 * 优化版: 适配 Core 的位压缩掩码，提供高效的掩码访问。
 */

#ifndef NPE_IMAGE_H
#define NPE_IMAGE_H

#include "npe/internal/npe_core.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint16_t width;
    uint16_t height;
    uint8_t  threshold;

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
} NPEImageConfig;

typedef struct {
    uint16_t width;
    uint16_t height;
    uint32_t total_pixels;
} NPEImageContext;

typedef struct {
    NPEngineCore* core;
    NPEImageConfig config;
    NPEImageContext ctx;
} NPEImage;

/* ========== ImageNPE API ========== */

NPEImage* npe_image_create(const NPEImageConfig* cfg);
void npe_image_destroy(NPEImage* img);

/**
 * 处理一帧图像
 * @param img           适配器
 * @param frame         输入帧 (uint8 灰度图)
 * @param changed_data  输出变化像素数据 (NULL 可不传)
 * @param change_mask   输出变化掩码 (bool 数组, NULL 可不传)
 * @param change_count  输出变化像素数量
 * @return              0 成功, -1 失败
 */
int npe_image_process(NPEImage* img,
                       const uint8_t* frame,
                       uint8_t* changed_data,
                       bool* change_mask,
                       uint32_t* change_count);

/**
 * 获取原始位压缩掩码 (零拷贝，直接访问内部缓冲区)
 * 用于高性能场景，避免 bool 数组转换开销
 */
const npemask_t* npe_image_get_mask(NPEImage* img);

/**
 * 将位压缩掩码转换为 bool 数组
 * 仅在需要时调用，避免不必要的转换
 */
void npe_image_mask_to_bool(NPEImage* img, bool* out_mask);

void npe_image_reset(NPEImage* img);
NPEngineCore* npe_image_get_core(NPEImage* img);

#ifdef __cplusplus
}
#endif

#endif
