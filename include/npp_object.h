/*
 * Copyright (c) 2026 吴金辉 (Jinhui Wu). All rights reserved.
 * Licensed under the NPP SDK Semi-Open Source License.
 * See LICENSE file for details.
 */
#ifndef NPP_OBJECT_H
#define NPP_OBJECT_H

#include "npp.h"

#ifdef __cplusplus
extern "C" {
#endif

/* 最大属性数量 */
#define NPP_MAX_PROPERTIES 64

/* NPP对象定义宏 */
#define NPP_OBJECT(name) \
    typedef struct { \
        npp_session_t* _session; \
        uint32_t _pipe_map[NPP_MAX_PROPERTIES]; \
        uint32_t _property_count;

#define NPP_PROPERTY(type, name) \
    type _current_##name; \
    type _last_##name; \
    float _threshold_##name; \
    npp_property_changed_cb_t _on_change_##name; \
    void* _on_change_user_data_##name;

#define NPP_OBJECT_END \
    } name;

/* 对象API */
npp_err_t npp_object_init(void* obj, const char* name, npp_profile_t* profile);
npp_err_t npp_set(void* obj, const char* property, double value);
double   npp_get(void* obj, const char* property);
npp_err_t npp_on_change(void* obj, const char* property, npp_property_changed_cb_t cb, void* user_data);
npp_err_t npp_poll(void* obj);
npp_err_t npp_session_deploy(void* obj, int is_sender);

#ifdef __cplusplus
}
#endif

#endif /* NPP_OBJECT_H */