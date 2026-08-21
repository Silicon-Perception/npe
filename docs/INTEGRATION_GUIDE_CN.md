# NPP SDK 集成指南

## 快速开始（5 分钟）

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

### 什么是管道（Pipe）？

**管道**是会话内的独立数据通道。每个管道：
- 有唯一的 `pipe_id`（0-511）
- 维护自己的状态（最后已知值）
- 独立于其他管道传输

### 什么是会话（Session）？

**会话**是两个端点之间的逻辑连接。一个会话：
- 包含多个管道
- 管理连接生命周期
- 处理 SYNC/恢复

### 什么是蓄水池（Reservoir）？

**蓄水池**是一种批处理机制：
- 累积多个管道变化
- 在单帧中发送
- 减少每帧开销

## API 用法

### 基本模式

```c
#include <npp.h>

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

// 3. 连接
npp_session_connect(session);

// 4. 写入数据（变化时自动发送）
npp_pipe_write(session, 0, data, len);

// 5. 轮询事件
while (1) {
    npp_poll(session);
}
```

### 处理管道变化

```c
// 为管道 #0 注册回调
npp_on_change(session, 0, [](uint32_t pipe_id, const uint8_t* data, uint32_t len) {
    printf("管道 %u 变化: %.*s\n", pipe_id, len, data);
});
```

### 会话事件

```c
// 注册会话事件回调
npp_on_session_event(session, NPP_EVENT_CONNECTED, on_connected);
npp_on_session_event(session, NPP_EVENT_DISCONNECTED, on_disconnected);
npp_on_session_event(session, NPP_EVENT_SYNC_COMPLETE, on_sync_complete);
```

## 连接状态

```
IDLE → CONNECTING → ACTIVE ↔ STALE
              ↑           │
              └── SYNC ───┘
```

| 事件 | 操作 |
|------|------|
| NPP_EVENT_CONNECTED | 开始发送数据 |
| NPP_EVENT_DISCONNECTED | 停止发送，等待重连 |
| NPP_EVENT_SYNC_COMPLETE | 状态一致，恢复正常运行 |

## 错误处理

| 错误码 | 含义 | 操作 |
|--------|------|------|
| NPP_OK | 成功 | 继续 |
| NPP_ERR_TIMEOUT | 操作超时 | 重试或重连 |
| NPP_ERR_INVALID | 参数无效 | 检查配置 |
| NPP_ERR_CRC | CRC 不匹配 | 帧损坏，下次 SYNC 恢复 |
| NPP_ERR_AUTH | 认证失败 | 检查加密密钥 |
| NPP_ERR_NO_MEM | 内存不足 | 减少管道数 |

## 网络行为

### 带宽占用

| 场景 | 带宽 | 说明 |
|------|------|------|
| 静态（无变化） | 0 | 静默即证明 |
| 低活动（10% 管道） | ~50 字节/帧 | 96 传感器，10Hz |
| 高活动（全部管道） | ~400 字节/帧 | 满吞吐量 |

### 恢复行为

1. **网络中断 < 超时**：自动重连，无数据丢失
2. **网络中断 > 超时**：标记为 STALE，重连时全量 SYNC
3. **服务端重启**：客户端检测超时，发起 SYNC

## 安全配置

### 明文模式（默认）

```c
npp_session_config_t cfg = {
    .transport = NPP_TRANSPORT_UDP,
    .flags = NPP_FLAG_PLAINTEXT  // 不加密
};
```

### 加密模式

```c
npp_session_config_t cfg = {
    .transport = NPP_TRANSPORT_UDP,
    .flags = NPP_FLAG_ENCRYPTED,
    .key = your_aes_key,  // 16 字节 AES-128
    .key_len = 16
};
```

### 密钥交换

生产环境请使用 ECDH 密钥交换：

```c
// 生成临时密钥对
npp_keypair_t keypair;
npp_generate_keypair(&keypair);

// 与对端交换公钥（带外）
// 会话会自动派生共享密钥
```

## 故障排除

### 常见问题

| 症状 | 原因 | 解决方案 |
|------|------|----------|
| 收不到数据 | 防火墙阻止 UDP | 开放端口 9999/UDP |
| 延迟高 | 蓄水池过大 | 减小蓄水池间隔 |
| 状态不一致 | SYNC 失败 | 检查网络，增加超时 |
| 内存增长 | 管道未清理 | 定期调用 npp_session_gc() |

### 调试模式

启用调试日志：

```c
npp_set_log_level(NPP_LOG_DEBUG);
npp_set_log_handler(my_log_handler);
```

### 抓包

用 tcpdump 捕获 NPP 流量：

```bash
sudo tcpdump -i any port 9999 -w npp_capture.pcap
```

用 Wireshark 分析（NPP 解析器在 `tools/wireshark/` 中）。

## 平台特定说明

### Linux

```bash
# 编译共享库版本
gcc -o app app.c -I./include -L./lib -lnpp -Wl,-rpath,./lib
```

### macOS

```bash
# 使用 .dylib 动态链接
gcc -o app app.c -I./include -L./lib -lnpe.4
```

### MCU（嵌入式）

```c
// MCU 仅使用静态库
// 内存限制：将 MAX_PIPES 减少到 64
#define NPP_MAX_PIPES 64
```

## 性能调优

### 蓄水池配置

```c
npp_session_config_t cfg = {
    .reservoir_interval_ms = 50,  // 批处理窗口（默认：100ms）
    .reservoir_max_frames = 4     // 每批最大帧数（默认：8）
};
```

| 间隔 | 延迟 | 效率 | 适用场景 |
|------|------|------|----------|
| 10ms | 低 | 较低 | 实时控制 |
| 100ms | 中等 | 均衡 | 通用 IoT |
| 500ms | 较高 | 最高 | 低功耗传感器 |

### MTU 考虑

- 默认最大帧：1400 字节（避免 IP 分片）
- 对于更大 MTU 的网络（如回环），可增加：
  ```c
  cfg.max_frame_size = 8192;  // 用于本地网络
  ```

## 线程安全与内存管理

### 线程安全

| 函数 | 线程安全 | 说明 |
|------|----------|------|
| npp_session_create() | ✅ 是 | 每会话调用一次 |
| npp_session_destroy() | ✅ 是 | 每会话调用一次 |
| npp_pipe_write() | ✅ 是 | 可从多线程调用 |
| npp_poll() | ❌ 否 | 每会话单线程 |
| npp_on_change() | ✅ 是 | connect 前注册 |

**推荐模式**:
- 一个线程用于 `npp_poll()`（事件循环）
- 任何线程都可调用 `npp_pipe_write()`
- 在 `npp_session_connect()` 前注册回调

### 内存管理

| 对象 | 分配者 | 释放者 |
|------|--------|--------|
| npp_session_t | npp_session_create() | npp_session_destroy() |
| npp_session_config_t | 调用者（栈） | 调用者 |
| 管道数据（写） | 调用者 | 调用者（npp_pipe_write 返回后） |
| 管道数据（读） | SDK（内部缓冲） | SDK（下次 poll 或 destroy 时） |

**重要**: 
- `npp_pipe_write()` 立即复制数据 — 调用者返回后即可释放
- 读取回调提供的数据指针仅在回调执行期间有效
- 如果需要在回调后保留数据，请复制

### 回调上下文

| 回调 | 执行线程 | 可阻塞 |
|------|----------|--------|
| npp_on_change() | npp_poll() 线程 | ❌ 否 |
| npp_on_session_event() | npp_poll() 线程 | ❌ 否 |

**重要**:
- 回调在 `npp_poll()` 线程中执行
- 不要在回调内调用 `npp_poll()`
- 不要在回调中执行长时间操作
- 使用队列将工作延迟到其他线程

## 获取帮助

- Gitee Issues: https://gitee.com/Silicon-Perception/npe/issues
- GitHub Issues: https://github.com/Silicon-Perception/npe/issues
- 邮箱: alphache@163.com
- 协议规范: [PROTOCOL_OVERVIEW_CN.md](./PROTOCOL_OVERVIEW_CN.md)
