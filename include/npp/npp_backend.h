#ifndef NPP_BACKEND_H
#define NPP_BACKEND_H

#include "npp_types.h"
#include "npp_schema.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declare */
struct npp_backend_s;

/* Output structure from backend */
struct npp_output_s {
    uint8_t*  bitmap;
    uint32_t  bitmap_size;
    uint32_t  change_count;
    uint8_t*  exuded_values;
    uint32_t  exuded_size;
    uint32_t  frame_id;
};

/* Backend type */
typedef enum {
    NPP_BACKEND_SOFTWARE = 0,
    NPP_BACKEND_HW_COPROC = 1,
    NPP_BACKEND_REMOTE = 2,
} npp_backend_type_t;

/* Backend interface */
typedef struct {
    int  (*init)(struct npp_backend_s* backend, const npp_schema_t* schema);
    void (*destroy)(struct npp_backend_s* backend);
    int  (*process)(struct npp_backend_s* backend, const void* input, 
                    npp_output_t* output);
    int  (*apply)(struct npp_backend_s* backend, const uint8_t* frame, 
                  uint32_t len);
    int  (*sync)(struct npp_backend_s* backend, uint8_t* buffer, 
                  uint32_t max_len, uint32_t* out_len);
    int  (*get_state)(struct npp_backend_s* backend, void* values, 
                      uint32_t max_count);
    int  (*reset)(struct npp_backend_s* backend);
} npp_backend_ops_t;

/* Backend structure */
typedef struct npp_backend_s {
    npp_backend_type_t type;
    npp_backend_ops_t  ops;
    void*              internal;
    const npp_schema_t* schema;
} npp_backend_t;

/* Backend factory */
npp_backend_t* npp_backend_create(npp_backend_type_t type, 
                                   const npp_schema_t* schema);
void           npp_backend_destroy(npp_backend_t* backend);

/* Software backend specific */
int npp_backend_software_init(npp_backend_t* backend, 
                               const npp_schema_t* schema);

/* Hardware coprocessor backend (stub for future) */
int npp_backend_coproc_init(npp_backend_t* backend, 
                             const npp_schema_t* schema,
                             const char* coproc_device);

/* Convenience wrappers */
int  npp_backend_process(npp_backend_t* backend, const void* input,
                          npp_output_t* output);
int  npp_backend_sync(npp_backend_t* backend, uint8_t* buffer,
                       uint32_t max_len, uint32_t* out_len);
int  npp_backend_apply(npp_backend_t* backend, const uint8_t* frame,
                        uint32_t len);
int  npp_backend_get_state(npp_backend_t* backend, void* values,
                            uint32_t max_count);
int  npp_backend_reset(npp_backend_t* backend);

#ifdef __cplusplus
}
#endif

#endif