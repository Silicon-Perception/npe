/*
 * Copyright (c) 2026 吴金辉 (Jinhui Wu). All rights reserved.
 * Licensed under the NPP SDK Semi-Open Source License.
 * See LICENSE file for details.
 * 
 * NPP SDK v1.0 - Public API
 * Based on NPE (Natural Pipeline Engine) architecture
 * 
 * 本头文件仅暴露公共API接口，所有内部实现细节均已隐藏。
 * 用户只需包含此文件即可使用全部功能。
 */
#ifndef NPP_H
#define NPP_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/*=============================================================================
 * Version Info
 *============================================================================*/

#define NPP_VERSION_MAJOR 1
#define NPP_VERSION_MINOR 0
#define NPP_VERSION_PATCH 1
#define NPP_VERSION_DATE 260821
#define NPP_VERSION_STRING "1.0.260821.1"

/*=============================================================================
 * Opaque Handles (不透明句柄 - 用户无法访问内部结构)
 *============================================================================*/

typedef struct npp_session_t npp_session_t;        /* 会话句柄 */
typedef struct npp_server_t npp_server_t;          /* 服务端句柄 */
typedef struct npp_reservoir_t npp_reservoir_t;    /* 蓄水池句柄 */

/*=============================================================================
 * Public Error Codes
 *============================================================================*/

typedef enum {
    NPP_OK                    =  0,   /* 成功 */
    NPP_ERR_INVALID_PARAM     = -1,   /* 无效参数 */
    NPP_ERR_NO_MEMORY         = -2,   /* 内存不足 */
    NPP_ERR_TIMEOUT           = -3,   /* 超时 */
    NPP_ERR_NOT_SUPPORTED     = -4,   /* 不支持 */
    NPP_ERR_NETWORK           = -5,   /* 网络错误 */
    NPP_ERR_AUTH_FAILED       = -6,   /* 认证失败 */
    NPP_ERR_MAX_RETRY         = -7,   /* 超过最大重试次数 */
} npp_err_t;

/*=============================================================================
 * Public Configuration Enums
 *============================================================================*/

/* 传输类型 */
typedef enum {
    NPP_TRANSPORT_UDP = 0,
    NPP_TRANSPORT_TCP = 1,
} npp_transport_type_t;

/* 日志级别 */
typedef enum {
    NPP_LOG_INFO  = 0,
    NPP_LOG_WARN  = 1,
    NPP_LOG_ERROR = 2,
} npp_log_level_t;

/*=============================================================================
 * Public Configuration Structures
 *============================================================================*/

/* 会话配置 */
typedef struct {
    npp_transport_type_t transport;   /* 传输类型 */
    uint16_t local_port;              /* 本地端口 */
    uint16_t remote_port;             /* 远程端口 */
    const char* remote_addr;          /* 远程地址 */
} npp_session_config_t;

/* 服务端配置 */
typedef struct {
    npp_transport_type_t transport;   /* 传输类型 */
    uint16_t port;                    /* 监听端口 */
    uint32_t max_clients;             /* 最大客户端数 */
} npp_server_config_t;

/* 蓄水池配置 */
typedef struct {
    uint32_t pipe_id;                 /* 监控的管道ID */
    float threshold;                  /* 触发阈值 */
    uint32_t timeout_ms;              /* 超时时间(毫秒) */
} npp_reservoir_config_t;

/*=============================================================================
 * Callback Types
 *============================================================================*/

/* 数据变化回调 */
typedef void (*npp_on_data_cb_t)(uint32_t pipe_id, const uint8_t* data, uint32_t len, void* user_data);

/* 错误回调 */
typedef void (*npp_on_error_cb_t)(npp_err_t error, const char* message, void* user_data);

/* 日志回调 */
typedef void (*npp_on_log_cb_t)(npp_log_level_t level, const char* message);

/*=============================================================================
 * Core API - Session Management (会话管理)
 *============================================================================*/

/**
 * 创建会话
 * @param session 输出：会话句柄
 * @param config  会话配置
 * @return NPP_OK 成功，其他为错误码
 */
npp_err_t npp_session_create(npp_session_t** session, const npp_session_config_t* config);

/**
 * 销毁会话
 * @param session 会话句柄
 * @return NPP_OK 成功
 */
npp_err_t npp_session_destroy(npp_session_t* session);

/**
 * 连接到对端
 * @param session 会话句柄
 * @return NPP_OK 成功
 */
npp_err_t npp_session_connect(npp_session_t* session);

/**
 * 断开连接
 * @param session 会话句柄
 * @return NPP_OK 成功
 */
npp_err_t npp_session_disconnect(npp_session_t* session);

/*=============================================================================
 * Core API - Pipe Operations (管道操作)
 *============================================================================*/

/**
 * 写入管道数据
 * @param session 会话句柄
 * @param pipe_id 管道ID
 * @param data    数据缓冲区
 * @param len     数据长度
 * @return NPP_OK 成功
 */
npp_err_t npp_pipe_write(npp_session_t* session, uint32_t pipe_id, const uint8_t* data, uint32_t len);

/**
 * 读取管道数据
 * @param session 会话句柄
 * @param pipe_id 管道ID
 * @param data    数据缓冲区
 * @param len     输入输出：缓冲区大小/实际读取长度
 * @return NPP_OK 成功
 */
npp_err_t npp_pipe_read(npp_session_t* session, uint32_t pipe_id, uint8_t* data, uint32_t* len);

/**
 * 注册数据变化回调
 * @param session  会话句柄
 * @param pipe_id  管道ID
 * @param cb       回调函数
 * @param user_data 用户数据
 * @return NPP_OK 成功
 */
npp_err_t npp_pipe_on_data(npp_session_t* session, uint32_t pipe_id, npp_on_data_cb_t cb, void* user_data);

/*=============================================================================
 * Core API - Reservoir (蓄水池 - 批量交付)
 *============================================================================*/

/**
 * 创建蓄水池
 * @param reservoir 输出：蓄水池句柄
 * @param config    蓄水池配置
 * @return NPP_OK 成功
 */
npp_err_t npp_reservoir_create(npp_reservoir_t** reservoir, const npp_reservoir_config_t* config);

/**
 * 销毁蓄水池
 * @param reservoir 蓄水池句柄
 * @return NPP_OK 成功
 */
npp_err_t npp_reservoir_destroy(npp_reservoir_t* reservoir);

/**
 * 添加数据到蓄水池
 * @param reservoir 蓄水池句柄
 * @param data      数据缓冲区
 * @param len       数据长度
 * @return NPP_OK 成功，NPP_ERR_RESERVOIR_FULL 蓄水池已满（将触发交付）
 */
npp_err_t npp_reservoir_add(npp_reservoir_t* reservoir, const uint8_t* data, uint32_t len);

/**
 * 手动触发蓄水池交付
 * @param reservoir 蓄水池句柄
 * @return NPP_OK 成功
 */
npp_err_t npp_reservoir_flush(npp_reservoir_t* reservoir);

/*=============================================================================
 * Core API - Server (服务端)
 *============================================================================*/

/**
 * 创建服务端
 * @param server 输出：服务端句柄
 * @param config 服务端配置
 * @return NPP_OK 成功
 */
npp_err_t npp_server_create(npp_server_t** server, const npp_server_config_t* config);

/**
 * 销毁服务端
 * @param server 服务端句柄
 * @return NPP_OK 成功
 */
npp_err_t npp_server_destroy(npp_server_t* server);

/**
 * 启动服务端
 * @param server 服务端句柄
 * @return NPP_OK 成功
 */
npp_err_t npp_server_start(npp_server_t* server);

/**
 * 停止服务端
 * @param server 服务端句柄
 * @return NPP_OK 成功
 */
npp_err_t npp_server_stop(npp_server_t* server);

/**
 * 广播数据到所有客户端
 * @param server 服务端句柄
 * @param data   数据缓冲区
 * @param len    数据长度
 * @return NPP_OK 成功
 */
npp_err_t npp_server_broadcast(npp_server_t* server, const uint8_t* data, uint32_t len);

/*=============================================================================
 * Core API - Utility (工具函数)
 *============================================================================*/

/**
 * 设置日志回调
 * @param cb 回调函数
 */
void npp_set_log_callback(npp_on_log_cb_t cb);

/**
 * 设置错误回调
 * @param cb 回调函数
 */
void npp_set_error_callback(npp_on_error_cb_t cb);

/**
 * 获取版本字符串
 * @return 版本字符串，如 "1.0.260821.1"
 */
const char* npp_version_string(void);

#ifdef __cplusplus
}
#endif

#endif /* NPP_H */
