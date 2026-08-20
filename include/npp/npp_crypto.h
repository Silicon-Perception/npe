#ifndef NPP_CRYPTO_H
#define NPP_CRYPTO_H

#include "npp_types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define NPP_CRYPTO_KEY_SIZE    32
#define NPP_CRYPTO_NONCE_SIZE  12

/* Crypto context */
typedef struct {
    npp_crypto_type_t type;
    uint8_t  key[NPP_CRYPTO_KEY_SIZE];
    uint8_t  nonce[NPP_CRYPTO_NONCE_SIZE];
    uint64_t counter;
    void*    impl_ctx;
} npp_crypto_ctx_t;

/* Crypto operations */
typedef struct {
    int  (*init)(npp_crypto_ctx_t* ctx, const uint8_t* key, int key_len);
    void (*destroy)(npp_crypto_ctx_t* ctx);
    int  (*encrypt)(npp_crypto_ctx_t* ctx, const uint8_t* plain, 
                    uint8_t* cipher, uint32_t len);
    int  (*decrypt)(npp_crypto_ctx_t* ctx, const uint8_t* cipher, 
                    uint8_t* plain, uint32_t len);
    int  (*set_nonce)(npp_crypto_ctx_t* ctx, const uint8_t* nonce);
} npp_crypto_ops_t;

/* Crypto API */
int  npp_crypto_init(npp_crypto_ctx_t* ctx, npp_crypto_type_t type, 
                     const uint8_t* key, int key_len);
void npp_crypto_destroy(npp_crypto_ctx_t* ctx);
int  npp_crypto_encrypt(npp_crypto_ctx_t* ctx, const uint8_t* plain,
                         uint8_t* cipher, uint32_t len);
int  npp_crypto_decrypt(npp_crypto_ctx_t* ctx, const uint8_t* cipher,
                         uint8_t* plain, uint32_t len);
int  npp_crypto_set_nonce(npp_crypto_ctx_t* ctx, const uint8_t* nonce);

/* Built-in crypto implementations */
int npp_crypto_chacha20_init(npp_crypto_ctx_t* ctx, const uint8_t* key, int key_len);
int npp_crypto_custom_init(npp_crypto_ctx_t* ctx, const uint8_t* key, int key_len);

#ifdef __cplusplus
}
#endif

#endif