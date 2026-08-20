/*
 * Copyright (c) 2026 吴金辉 (Jinhui Wu). All rights reserved.
 * Licensed under the NPP SDK Semi-Open Source License.
 * See LICENSE file for details.
 * 
 * NPP SDK v2.0 - Unified Header
 * Based on NPE (Natural Pipeline Engine) architecture
 */
#ifndef NPP_H
#define NPP_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Version info */
#define NPP_VERSION_MAJOR 2
#define NPP_VERSION_MINOR 0
#define NPP_VERSION_PATCH 0
#define NPP_VERSION_STRING "2.0.0"

/*=============================================================================
 * Core Types & Constants
 *============================================================================*/

/* Error codes */
typedef enum {
    NPP_OK = 0,
    NPP_ERR_INVALID_PARAM = -1,
    NPP_ERR_NO_MEMORY = -2,
    NPP_ERR_TIMEOUT = -3,
    NPP_ERR_NOT_SUPPORTED = -4,
    NPP_ERR_NETWORK = -5,
    NPP_ERR_AUTH_FAILED = -6,
    NPP_ERR_MAX_RETRY = -7,
    NPP_ERR_RESERVOIR_FULL = -8,
} npp_err_t;

/* Pipe type */
typedef enum {
    NPP_PIPE_TYPE_HARDWARE = 0x01,
    NPP_PIPE_TYPE_SOFTWARE = 0x02
} npp_pipe_type_t;

/* Connection strategy */
typedef enum {
    NPP_STRATEGY_ON_DEMAND = 0,
    NPP_STRATEGY_KEEPALIVE = 1
} npp_connection_strategy_t;

/* Wake type */
typedef enum {
    NPP_WAKE_TYPE_DATA_CHANGE = 0,
    NPP_WAKE_TYPE_EXTERNAL_EVENT = 1,
    NPP_WAKE_TYPE_PEER_REQUEST = 2,
    NPP_WAKE_TYPE_MULTICAST = 3,
    NPP_WAKE_TYPE_UNICAST = 4
} npp_wake_type_t;

/* Session mode */
typedef enum {
    NPP_MODE_LOCAL = 0,
    NPP_MODE_NETWORK = 1,
} npp_session_mode_t;

/* Transport type */
typedef enum {
    NPP_TRANSPORT_UDP = 0,
    NPP_TRANSPORT_TCP = 1,
    NPP_TRANSPORT_RF = 2,
} npp_transport_type_t;

/* Reservoir mode */
typedef enum {
    NPP_RESERVOIR_MODE_VOLUME = 0,    /* Trigger on accumulated changes */
    NPP_RESERVOIR_MODE_LEVEL = 1,     /* Trigger on watched pipe change */
    NPP_RESERVOIR_MODE_TIMER = 2,     /* Trigger on time interval */
} npp_reservoir_mode_t;

/* Color tags */
typedef enum {
    NPP_COLOR_NONE = 0,
    NPP_COLOR_PRIORITY = 1,           /* QoS level */
    NPP_COLOR_SECURITY = 2,           /* Strong encryption required */
    NPP_COLOR_AUDIT = 3,              /* Must be logged */
    NPP_COLOR_ROUTE = 4,              /* Routing directives */
} npp_color_tag_t;

/* Log level */
typedef enum {
    NPP_LOG_DEBUG = 0,
    NPP_LOG_INFO = 1,
    NPP_LOG_WARN = 2,
    NPP_LOG_ERROR = 3,
    NPP_LOG_FATAL = 4
} npp_log_level_t;

/*=============================================================================
 * Core Handles
 *============================================================================*/

typedef struct npp_session_t npp_session_t;
typedef struct npp_profile_t npp_profile_t;
typedef struct npp_wake_pipe_t npp_wake_pipe_t;
typedef struct npp_server_t npp_server_t;
typedef struct npp_schema_t npp_schema_t;
typedef struct npp_reservoir_t npp_reservoir_t;
typedef struct npp_hopping_ctx_t npp_hopping_ctx_t;
typedef struct npp_coloring_ctx_t npp_coloring_ctx_t;

/*=============================================================================
 * Callback Types
 *============================================================================*/

typedef void (*npp_wake_cb_t)(npp_wake_type_t type, uint32_t profile_id, void* user_data);
typedef void (*npp_property_changed_cb_t)(double new_value, double old_value, void* user_data);
typedef void (*npp_log_cb_t)(npp_log_level_t level, const char* msg, void* user_data);
typedef void (*npp_error_cb_t)(int error_code, const char* error_msg, void* user_data);

/*=============================================================================
 * Session Configuration
 *============================================================================*/

typedef struct {
    npp_session_mode_t mode;
    npp_transport_type_t transport;
    uint16_t udp_port;
    uint16_t tcp_port;
    const char* server_addr;
} npp_session_cfg_t;

/*=============================================================================
 * Frame Format
 *============================================================================*/

typedef struct {
    uint32_t profile_id;
    uint32_t frame_id;
    uint8_t* bitmap;
    uint32_t bitmap_len;
    uint8_t* diff_data;
    uint32_t diff_data_len;
} npp_frame_t;

/*=============================================================================
 * Core API - Session Management
 *============================================================================*/

int npp_session_create(npp_session_t** session, npp_session_cfg_t* cfg);
int npp_session_destroy(npp_session_t* session);
int npp_session_connect(npp_session_t* session);
int npp_session_deploy(void* obj, int is_sender);

/*=============================================================================
 * Core API - Pipe Operations
 *============================================================================*/

int npp_pipe_write(npp_session_t* session, uint32_t pipe_id, void* data, uint32_t len);
int npp_pipe_read(npp_session_t* session, uint32_t pipe_id, void* data, uint32_t* len);

/*=============================================================================
 * Core API - Frame Operations
 *============================================================================*/

int npp_frame_parse(npp_frame_t* frame, uint8_t* data, uint32_t len);
int npp_frame_serialize(npp_frame_t* frame, uint8_t* data, uint32_t* len);

/*=============================================================================
 * Core API - Wake Pipe
 *============================================================================*/

npp_err_t npp_wake_pipe_init(npp_wake_pipe_t* pipe, npp_connection_strategy_t strategy);
npp_err_t npp_wake_pipe_register_profile(npp_wake_pipe_t* pipe, uint32_t profile_id, float threshold);
npp_err_t npp_wake_pipe_set_callback(npp_wake_pipe_t* pipe, npp_wake_cb_t cb, void* user_data);
npp_err_t npp_wake_pipe_start_listen(npp_wake_pipe_t* pipe);
npp_err_t npp_wake_pipe_stop_listen(npp_wake_pipe_t* pipe);
npp_err_t npp_wake_pipe_trigger_wake(npp_wake_pipe_t* pipe, npp_wake_type_t type, uint32_t profile_id);
npp_err_t npp_wake_pipe_handle_frame(npp_wake_pipe_t* pipe, npp_wake_type_t type, uint32_t addr, uint32_t group_id);

/*=============================================================================
 * Core API - Utility
 *============================================================================*/

uint32_t npp_hash32(const void* data, uint32_t len);
const char* npp_version_string(void);

#ifdef __cplusplus
}
#endif

#endif /* NPP_H */
