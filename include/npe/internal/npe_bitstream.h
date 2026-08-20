/**
 * NPE 4.0 — 位流打包层
 *
 * 功能:
 *   - 1-8bit 可变位深的位流读写
 *   - 消除 uint8_t 存储浪费 (2bit位深只占2bit, 非8bit)
 *   - 支持非字节对齐的位流打包
 *
 * 零计算原则:
 *   - 仅位移、掩码、赋值
 *   - 无乘除法, 无浮点
 *
 * MCU 兼容:
 *   - 无动态内存分配
 *   - 无 SIMD
 *   - 紧凑结构
 */

#ifndef NPE_BITSTREAM_H
#define NPE_BITSTREAM_H

#include "npe/internal/npe_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ========== 位流写入器 ========== */

typedef struct {
    uint8_t* buffer;       /* 输出缓冲区 */
    uint32_t  capacity;     /* 缓冲区容量(字节) */
    uint32_t  byte_pos;     /* 当前字节位置 */
    uint8_t   bit_pos;      /* 当前位位置 (0-7) */
    uint8_t   bits_per_val; /* 每个值的位深 (1-8) */
} NPEBitWriter;

/* ========== 位流读取器 ========== */

typedef struct {
    const uint8_t* buffer;
    uint32_t  size;
    uint32_t  byte_pos;
    uint8_t   bit_pos;
    uint8_t   bits_per_val;
} NPEBitReader;

/* ========== 位流写入 API ========== */

void npe_bw_init(NPEBitWriter* bw, uint8_t* buffer, uint32_t capacity, uint8_t bits_per_val);

/* 写入一个值 (位深由 bw->bits_per_val 决定)
 * @return 0 成功, -1 缓冲区满
 */
int npe_bw_write(NPEBitWriter* bw, uint8_t value);

/* 获取已写入的总字节数 (向上取整) */
uint32_t npe_bw_bytes_used(const NPEBitWriter* bw);

/* 获取已写入的总位数 */
uint32_t npe_bw_bits_used(const NPEBitWriter* bw);

/* ========== 位流读取 API ========== */

void npe_br_init(NPEBitReader* br, const uint8_t* buffer, uint32_t size, uint8_t bits_per_val);

/* 读取一个值
 * @return 0 成功, -1 缓冲区结束
 */
int npe_br_read(NPEBitReader* br, uint8_t* value);

/* 批量写入/读取 (优化版, 减少函数调用开销) */
uint32_t npe_bw_write_batch(NPEBitWriter* bw, const uint8_t* values, uint32_t count);
uint32_t npe_br_read_batch(NPEBitReader* br, uint8_t* values, uint32_t count);

/* ========== 工具函数 ========== */

/* 亮度级数 → 所需位深 (查表, 零计算)
 * levels: 1-256
 * return: 0-8 (0=无数据, 8=256级)
 */
uint8_t npe_bits_for_levels(uint16_t levels);

/* 位深 → 最大亮度级数 */
uint16_t npe_levels_for_bits(uint8_t bits);

/* 计算打包后字节数
 * count: 值数量, bits: 位深
 * return: 字节数 (向上取整)
 */
static inline uint32_t npe_packed_bytes(uint32_t count, uint8_t bits) {
    if (bits == 0) return 0;
    uint32_t total_bits = count * bits;
    return (total_bits + 7) >> 3;  /* (total_bits + 7) / 8, 用位移替代除法 */
}

#ifdef __cplusplus
}
#endif

#endif
