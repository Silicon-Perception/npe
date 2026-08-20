/**
 * NPE 4.0 — 管道阵列 API
 *
 * 管道 (Pipeline): NPE 核心数据结构
 *   - 每个管道保持一个状态值
 *   - 输入变化时"渗出"数据
 *   - 输入不变时"阻挡"数据
 */

#ifndef NPE_PIPELINE_H
#define NPE_PIPELINE_H

#include "npe/internal/npe_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint16_t id;
    uint8_t  state;
    uint8_t  last_input;
    bool     state_initialized;
    uint64_t exude_count;
    uint64_t block_count;
} NPEPipeline;

typedef struct {
    NPEPipeline* pipelines;
    uint32_t count;
    uint16_t width;
    uint16_t height;
    uint8_t  channels;
} NPEPipelineArray;

NPEPipelineArray* npe_pa_create(uint16_t width, uint16_t height, uint8_t channels);
void              npe_pa_destroy(NPEPipelineArray* pa);

bool npe_pa_exude(NPEPipelineArray* pa, uint32_t index, uint8_t input, uint8_t* output);
void npe_pa_block(NPEPipelineArray* pa, uint32_t index, uint8_t input);
void npe_pa_reset(NPEPipelineArray* pa);

uint64_t npe_pa_get_stats(const NPEPipelineArray* pa, uint64_t* exudes, uint64_t* blocks);

#ifdef __cplusplus
}
#endif

#endif
