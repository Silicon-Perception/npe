#ifndef NPP_SCHEMA_H
#define NPP_SCHEMA_H

#include "npp_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Pipe definition */
typedef struct {
    uint16_t             index;
    const char*          name;
    npp_data_type_t      data_type;
    npp_pipe_category_t  category;
    double               threshold;
} npp_pipe_def_t;

/* Layout definition */
typedef struct {
    npp_layout_type_t type;
    uint16_t          total_pipes;
    uint16_t          width;
    uint16_t          height;
    uint16_t          tile_size;
    uint8_t           has_neighbors;
} npp_layout_t;

/* Schema definition */
typedef struct {
    npp_layout_t    layout;
    npp_pipe_def_t* pipes;
    uint16_t        pipe_count;
    uint32_t        schema_id;
    char            version[32];
} npp_schema_t;

/* Schema management API */
npp_schema_t* npp_schema_create(const npp_pipe_def_t* pipes, uint16_t count, 
                                  const npp_layout_t* layout, const char* version);
void          npp_schema_destroy(npp_schema_t* schema);
uint32_t      npp_schema_hash(const npp_schema_t* schema);
int           npp_schema_validate(const npp_schema_t* schema);

/* Helper: create a simple 1D schema */
npp_schema_t* npp_schema_create_1d(uint16_t count, const char** names, 
                                    npp_data_type_t type, double threshold);

/* Helper: create a 2D grid schema (e.g., for images) */
npp_schema_t* npp_schema_create_grid(uint16_t width, uint16_t height, 
                                      npp_data_type_t type, double threshold);

#ifdef __cplusplus
}
#endif

#endif