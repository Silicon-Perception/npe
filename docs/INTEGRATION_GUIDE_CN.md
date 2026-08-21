# NPP SDK 集成指南

## 快速开始 (5 分钟)

### 1. 下载 SDK

```bash
# 从 Gitee 克隆
git clone https://gitee.com/Silicon-Perception/npe.git
cd npe

# 或从 Release 页面下载：
# https://gitee.com/Silicon-Perception/npe/releases
```

### 2. 编译运行

```bash
# 使用预编译库编译
gcc -o demo examples/quick_start.c -I./include -L./lib -lnpp

# 运行
./demo
```

## 协议概念

### 什么是管道 (Pipe)？

**管道**是会话内的独立数据通道。每个管道：
- 有唯一的 `pipe_id` (0-511)
- 维护自身状态（最后已知值）
- 独立于其他管道传输

### 什么是会话 (Session)？

**会话**是两个端点之间的逻辑连接。会话：
- 包含多个管道
- 管理连接生命周期
- 处理 SYNC/恢复

### 什么是蓄水池 (Reservoir)？

**蓄水池**是批量交付机制：
- 累积多个数据变化
- 一次性发送
- 减少每帧开销

## API 使用

### 基础模式

```c
#include <npp.h>

// 管道数据接收回调
void on_data(uint32_t pipe_id, const uint8_t* data, uint32_t len, void* user_data) {
    printf("管道 %u 收到 %u 字节\n", pipe_id, len);
}

int main() {
    // 1. 配置会话
    npp_session_config_t cfg = {
        .transport = NPP_TRANSPORT_UDP,
        .local_port = 8888,
        .remote_addr = "192.168.1.100",
        .remote_port = 9999
    };

    // 2. 创建会话
    npp_session_t* session;
    npp_session_create(&session, &cfg);

    // 3. 注册管道 #0 的回调
    npp_pipe_on_data(session, 0, on_data, NULL);

    // 4. 连接
    npp_session_connect(session);

    // 5. 写入数据（变化时自动传输）
    uint8_t data[] = {1, 2, 3, 4};
    npp_pipe_write(session, 0, data, sizeof(data));

    // 6. 清理
    npp_session_disconnect(session);
    npp_session_destroy(session);
    return 0;
}
```

### 处理管道变化

```c
// 为特定管道注册回调
npp_pipe_on_data(session, 0, [](uint32_t pipe_id, const uint8_t* data, uint32_t len, void* user_data) {
    printf("管道 %u 变化: %.*s\n", pipe_id, len, data);
}, NULL);
```

### 蓄水池使用

```c
// 创建蓄水池
npp_reservoir_config_t res_cfg = {
    .pipe_id = 0,
    .threshold = 100.0f,
    .timeout_ms = 100
};
npp_reservoir_t* reservoir;
npp_reservoir_create(&reservoir, &res_cfg);

// 添加数据（达到阈值自动交付）
npp_reservoir_add(reservoir, data, len);

// 或手动触发交付
npp_reservoir_flush(reservoir);
```

### 服务端模式

```c
// 创建服务端
npp_server_config_t srv_cfg = {
    .transport = NPP_TRANSPORT_UDP,
    .port = 9999,
    .max_clients = 100
};
npp_server_t* server;
npp_server_create(&server, &srv_cfg);

// 启动监听
npp_server_start(server);

// 广播到所有客户端
uint8_t msg[] = "Hello all!";
npp_server_broadcast(server, msg, sizeof(msg));

// 停止并清理
npp_server_stop(server);
npp_server_destroy(server);
```

## 连接状态

```
IDLE → CONNECTING → ACTIVE
              ↑         │
              └─────────┘ (自动重连)
```

## 错误处理

| 错误码 | 含义 | 操作 |
|--------|------|------|
| NPP_OK | 成功 | 继续 |
| NPP_ERR_INVALID_PARAM | 无效参数 | 检查配置 |
| NPP_ERR_NO_MEMORY | 内存不足 | 减少管道数 |
| NPP_ERR_TIMEOUT | 超时 | 重试或重连 |
| NPP_ERR_NETWORK | 网络错误 | 检查连通性 |
| NPP_ERR_AUTH_FAILED | 认证失败 | 检查加密密钥 |

## 日志

```c
// 设置日志回调
npp_set_log_callback([](npp_log_level_t level, const char* message) {
    printf("[NPP] %s\n", message);
});

// 设置错误回调
npp_set_error_callback([](npp_err_t error, const char* message, void* user_data) {
    fprintf(stderr, "NPP 错误 %d: %s\n", error, message);
});
```

## 网络行为

### 带宽占用

| 场景 | 带宽 | 说明 |
|------|------|------|
| 静态（无变化） | 0 | 沉默即证明 |
| 低活跃（10% 管道） | ~50 字节/帧 | 96 个传感器，10Hz |
| 高活跃（全部管道） | ~400 字节/帧 | 全吞吐量 |

### 恢复行为

1. **断网 < 超时**：自动重连，无数据丢失
2. **断网 > 超时**：重连后全量 SYNC
3. **服务端重启**：客户端检测到超时，主动 SYNC

## 安全

如需生产环境加密：
- 联系 alphache@163.com 获取商业授权加密版
- 当前开源版本支持明文模式

## 故障排除

### 常见问题

| 症状 | 原因 | 解决方案 |
|------|------|----------|
| 收不到数据 | 防火墙阻止 UDP | 开放端口 9999/UDP |
| 连接失败 | 地址错误 | 检查 IP 和端口 |
| 内存增长 | 会话未清理 | 调用 npp_session_destroy() |

### 抓包

使用 tcpdump 捕获 NPP 流量：

```bash
sudo tcpdump -i any port 9999 -w npp_capture.pcap
```

## 平台特定说明

### Linux

```bash
# 使用静态库编译
gcc -o app app.c -I./include -L./lib -lnpp
```

### macOS

```bash
# 使用静态库编译
gcc -o app app.c -I./include -L./lib -lnpp
```

### MCU（嵌入式）

```c
// MCU 仅使用静态库
// 内存受限：减少 MAX_PIPES 到 64
#define NPP_MAX_PIPES 64
```

## 性能调优

### 蓄水池配置

```c
npp_reservoir_config_t cfg = {
    .pipe_id = 0,
    .threshold = 100.0f,    // 交付阈值
    .timeout_ms = 100        // 最大批量窗口
};
```

| 超时 | 延迟 | 效率 | 场景 |
|------|------|------|------|
| 10ms | 低 | 较低 | 实时控制 |
| 100ms | 中 | 平衡 | 通用物联网 |
| 500ms | 较高 | 最高 | 低功耗传感器 |

## 获取帮助

- Gitee Issues: https://gitee.com/Silicon-Perception/npe/issues
- GitHub Issues: https://github.com/Silicon-Perception/npe/issues
- 邮件: alphache@163.com
- 协议规范: [PROTOCOL_OVERVIEW_CN.md](./PROTOCOL_OVERVIEW_CN.md)
