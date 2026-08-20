/**
 * NPE 4.3 — 量子随机数接口预留 (QRNG API)
 *
 * 设计哲学:
 *   NPP跳频安全依赖PRNG (xorshift128), 但PRNG是确定性的
 *   量子随机数发生器 (QRNG) 提供真随机性, 不可预测
 *   预留接口, 未来硬件QRNG可用时直接接入
 *
 * 应用场景:
 *   1. 跳频种子增强: PRNG种子 + QRNG熵 → 不可预测跳频序列
 *   2. 密钥协商: QRNG生成会话密钥, 前向安全
 *   3. 蓄水池触发: QRNG随机化触发阈值, 防流量分析
 *   4. 审计随机挑战: QRNG生成审计随机数, 防重放
 *
 * 后端抽象:
 *   BACKEND_QRNG_SOFTWARE  - 软件伪随机 (当前, /dev/urandom)
 *   BACKEND_QRNG_HARDWARE   - 硬件QRNG (未来, 如Intel RDRAND)
 *   BACKEND_QRNG_QUANTUM    - 量子QRNG (未来, 如光子QRNG芯片)
 *   BACKEND_QRNG_NETWORK    - 网络QRNG服务 (未来, 如NIST随机信标)
 *
 * 约束:
 *   - 接口稳定, 后端可插拔
 *   - 软件后端保证可用性 (即使无QRNG硬件)
 *   - 硬件后端不可用时自动降级
 *
 * 专利边界: 接口设计属于NPE/NPP, QRNG硬件实现属于第三方
 */

#ifndef NPE_QRNG_H
#define NPE_QRNG_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ========== QRNG 后端类型 ========== */

typedef enum {
    NPE_QRNG_SOFTWARE  = 0,  /* 软件伪随机 (/dev/urandom, LCG) */
    NPE_QRNG_HARDWARE   = 1,  /* 硬件随机 (RDRAND, ARM RNDR) */
    NPE_QRNG_QUANTUM    = 2,  /* 量子随机 (光子QRNG, 预留) */
    NPE_QRNG_NETWORK    = 3,  /* 网络随机信标 (NIST, 预留) */
    NPE_QRNG_HYBRID     = 4,  /* 混合 (多源异或, 最高安全性) */
} npe_qrng_backend_t;

/* ========== QRNG 上下文 ========== */

typedef struct {
    npe_qrng_backend_t backend;     /* 当前后端 */
    uint8_t  available;              /* 后端是否可用 */
    uint8_t  quality_level;          /* 随机质量 (0-100, 100=量子级) */
    uint64_t bytes_generated;        /* 累计生成字节数 */
    uint64_t seed;                   /* 软件后端种子 */
} npe_qrng_ctx_t;

/* ========== API ========== */

/**
 * 初始化QRNG上下文
 * @param preferred 期望后端 (不可用时自动降级)
 * @return 0=成功, <0=错误
 */
int npe_qrng_init(npe_qrng_ctx_t* ctx, npe_qrng_backend_t preferred);

/**
 * 检测可用后端
 * @return 可用后端位掩码 (bit n = backend n 可用)
 */
uint32_t npe_qrng_detect_backends(void);

/**
 * 生成随机字节
 * @param ctx  QRNG上下文
 * @param buf  输出缓冲区
 * @param len  字节数
 * @return 0=成功, <0=错误
 */
int npe_qrng_generate(npe_qrng_ctx_t* ctx, uint8_t* buf, size_t len);

/**
 * 生成32位随机数 (跳频种子用)
 */
uint32_t npe_qrng_generate_u32(npe_qrng_ctx_t* ctx);

/**
 * 生成64位随机数 (密钥协商用)
 */
uint64_t npe_qrng_generate_u64(npe_qrng_ctx_t* ctx);

/**
 * 混合熵源: PRNG种子 + QRNG熵 → 增强种子
 * @param prng_seed  PRNG种子 (确定性)
 * @param qrng_ctx   QRNG上下文 (真随机)
 * @return 增强后的种子 (不可预测)
 */
uint32_t npe_qrng_enhance_seed(uint32_t prng_seed, npe_qrng_ctx_t* qrng_ctx);

/**
 * 获取后端名称
 */
const char* npe_qrng_backend_name(npe_qrng_backend_t backend);

/**
 * 销毁QRNG上下文
 */
void npe_qrng_destroy(npe_qrng_ctx_t* ctx);

#ifdef __cplusplus
}
#endif

#endif
