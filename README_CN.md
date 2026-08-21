# NPP SDK - 轻量级物联网通信协议栈

> 纯 C 语言实现的开源网络通信协议栈，采用增量传输设计，适用于低功耗、低带宽的物联网场景。NPP 协议面向传感器网络、边缘计算等场景，提供简洁的 API 和高效的增量传输能力。

<div align="center">

[![License: MIT](https://img.shields.io/badge/许可证-MIT-yellow.svg)](LICENSE)
[![C](https://img.shields.io/badge/C-99-blue.svg)](https://en.wikipedia.org/wiki/C99)
[![Platform](https://img.shields.io/badge/平台-Linux%20%7C%20macOS%20%7C%20Windows%20%7C%20MCU-green.svg)]()
[![Version](https://img.shields.io/badge/版本-1.0.260821.1-blue.svg)](VERSION)
[![Build](https://img.shields.io/badge/构建-CMake-blue.svg)]()
[![DOI](https://img.shields.io/badge/DOI-10.5281%2Fzenodo.21844224-blue.svg)](https://zenodo.org/doi/10.5281/zenodo.21844224)
[![ORCID](https://img.shields.io/badge/ORCID-0009--0007--6411--7426-green)](https://orcid.org/0009-0007-6411-7426)

[English](./README.md) | [中文](./README_CN.md)

</div>

---

## 🎯 核心理念：管道架构

NPP 基于 NPE（自然管道引擎）架构，将网络通从"计算驱动"转变为"比较驱动"。

![管道设计](images/a0.gif)
*帧拆分为管道格式：每个数据流分解为独立的"管道"，各自维护自身状态*

---

## ⚡ 通信范式对比

### 传统轮询方式
![传统通信](images/a1.gif)
- 定期查询数据，即使大部分数据未变化
- 连接管理开销
- CPU 参与数据包处理

### NPP 方式
![NPP 通信](images/a2.gif)
- 增量传输：仅传输变化的数据
- 无连接状态管理
- 简单的比较和更新操作

**NPP 设计特点：**
- **沉默即证明**：无数据变化即表示设备在线（无需显式心跳包）
- **增量更新**：核心操作专注于检测和传输变化
- **零计算**：核心操作为比较和赋值，不使用乘法器、除法器、变换运算和熵编码

---

## ✨ 核心特性展示

### 🏗️ 管道架构
![管道设计](images/a0.gif)
*帧拆分为管道格式：每个数据流分解为独立的"管道"，各自维护自身状态*

### 💧 蓄水池机制
![蓄水池](images/a3.gif)
*批量交付机制：累积多个变化后一次性交付，节省帧头开销，提高传输效率*

### 📡 管道跳频
![管道跳频](images/a4.gif)
*管道级跳频：受到干扰时自动切换管道传输频道，保证通可靠*

### 🔍 可审计性
![可审计性](images/a5.gif)
*完整审计追踪：所有数据变化可追溯，确保透明可验证*

### 🎨 管道着色
![管道着色](images/a6.gif)
*数据流可视化：颜色编码的管道状态，直观监控网络健康*

---

## ✨ 核心优势

### 🔓 半开源模式：平衡生态与 IP 保护
- **闭源核心**：核心协议层、唤醒调度、自适应算法 — 保护知识产权
- **开源上层**：极简 API、示例代码、文档
- **商业友好**：开源部分 MIT 协议，闭源部分可商业授权

### ⚡ C 语言实现：性能与跨平台
- **原生性能**：纯 C 实现
- **超轻量**：最小运行 <64KB ROM / 8KB RAM，从 8 位 MCU 到 64 位服务器均可运行
- **无缝集成**：兼容所有 C/C++ 环境，可嵌入 Linux/RTOS/裸机

### 🌐 范式创新
- **沉默即证明**：无数据变化即链路沉默 — 本身即是设备在线的证明
- **状态自愈**：内置 SYNC/REQ_SYNC 机制，重连后自动全量同步
- **语义级容错**：UDP 丢帧只影响单个字段，不影响整个系统
- **原生多播**：一次唤醒多个设备，单帧分发到数千终端

---

## 🏗️ 协议分层

NPP 工作在**应用层**，运行在现有传输协议之上：

```
┌─────────────────────────────────────────┐
│  应用层: NPP (本协议)                    │  ← 你在这里
├─────────────────────────────────────────┤
│  传输层: UDP / TCP                      │
├─────────────────────────────────────────┤
│  网络层: IP                             │
└─────────────────────────────────────────┘
```

NPP **不替代** UDP/TCP。它提供：
- **状态同步**：仅变化传输，无冗余数据
- **管道复用**：每会话最多 512 个独立数据通道
- **自愈能力**：重连后自动全量 SYNC

详见 [协议规范](./docs/PROTOCOL_OVERVIEW_CN.md)。

---

## 🛠️ 快速上手

### 基础示例：会话与管道 I/O

```c
#include <npp.h>

int main() {
    /* 1. 配置会话 */
    npp_session_config_t cfg = {
        .transport = NPP_TRANSPORT_UDP,
        .local_port = 8888,
        .remote_port = 9999,
        .remote_addr = "192.168.1.100"
    };

    /* 2. 创建会话 */
    npp_session_t* session;
    npp_session_create(&session, &cfg);
    npp_session_connect(session);

    /* 3. 写入管道 #0 数据（变化时自动传输） */
    uint8_t data[] = {1, 2, 3, 4};
    npp_pipe_write(session, 0, data, sizeof(data));

    /* 4. 清理 */
    npp_session_disconnect(session);
    npp_session_destroy(session);
    return 0;
}
```

编译运行：
```bash
# Linux / macOS
gcc -o my_app my_app.c -I./include -L./lib -lnpp
./my_app

# Windows (Visual Studio)
# 1. 打开 "Developer Command Prompt for VS"
# 2. mkdir build && cd build
# 3. cmake .. -G "Visual Studio 16 2019" -A x64
# 4. cmake --build . --config Release
# 5. 输出: build/Release/npp.lib
```

---

## 📊 性能特征

### 设计目标

- **增量传输**：仅传输变化的数据
- **低开销**：小数据包场景下协议开销小
- **灵活**：支持多种物联网和嵌入式场景

### 带宽效率（典型场景）

| 场景 | 轮询方式 | NPP | 说明 |
|------|----------|-----|------|
| 静态（无变化） | 定期查询 | 几乎为零 | 无数据变化 = 最小流量 |
| 低活跃 | 定期查询 | 显著降低 | 仅传输变化的数据 |
| 高活跃 | 定期查询 | 降低 | 增量更新减少开销 |

*实际效果取决于具体使用场景、网络环境和配置。*

### 测试覆盖

| 模块 | 测试数 | 状态 |
|------|--------|------|
| 基础 API | 8 | ✅ 通过 |
| 会话生命周期 | 5 | ✅ 通过 |
| 管道 I/O | 6 | ✅ 通过 |

---

## 🧩 支持场景

- **物联网**：智能家居、工业传感器、农业监控、能源监控
- **边缘计算**：边缘节点通、本地数据同步、低功耗广域网
- **嵌入式设备**：MCU 通、设备直连、RTOS 系统
- **分布式系统**：节点状态同步、服务发现、配置分发
- **弱网环境**：卫星通、地下管道、高速移动场景

---

## 📁 项目结构

```
npe/
├── include/              // 公共头文件（开源，MIT 协议）
│   └── npp.h             // 主公共 API
├── lib/                  // 预编译二进制（闭源，商业授权）
│   ├── libnpp.a          // NPP 协议静态库
│   ├── libnpe.a          // NPE 引擎静态库
│   └── README.md         // 库文档
├── tests/                // 单元测试（开源，MIT 协议）
│   ├── test_e2e.c        // 基础 API 测试
│   └── test_wake.c       // 会话/管道测试
├── examples/             // 示例代码（开源，MIT 协议）
│   ├── quick_start.c     // 基础会话示例
│   └── example_sensor.c  // 传感器模拟
├── docs/                 // 文档
│   ├── PROTOCOL_OVERVIEW.md    // 协议规范
│   ├── INTEGRATION_GUIDE.md    // 集成指南
│   ├── PROTOCOL_OVERVIEW_CN.md // 中文版协议规范
│   ├── INTEGRATION_GUIDE_CN.md // 中文版集成指南
│   ├── GETTING_STARTED.md      // 快速开始教程
│   └── FAQ.md                  // 常见问题
├── images/               // 演示 GIF 和图片
├── CMakeLists.txt        // 构建配置
├── CONTRIBUTING.md       // 贡献指南
└── LICENSE               // 半开源协议
```

---

## 🤝 贡献与生态

欢迎开发者参与 NPP 生态建设：
1. 提交 bug 报告和建议
2. 贡献示例代码和演示
3. 参与上层应用开发

注意：核心闭源部分不接受外部 PR，仅接受开源部分的贡献。

---

## 📜 许可证

本项目采用**半开源协议**：
- 开源部分（include/tests/examples/docs）遵循 MIT 协议
- 闭源部分（核心实现）需要商业授权
- 详见 [LICENSE](./LICENSE)
- 版权: 吴金辉, ORCID: 0009-0007-6411-7426

## 📄 引用

如果您在研究中使用了本项目，请引用：

```bibtex
@software{npp_sdk_2026,
  author       = {吴金辉},
  title        = {NPP SDK: 基于 NPE 架构的高性能网络通信引擎},
  month        = aug,
  year         = 2026,
  publisher    = {Zenodo},
  doi          = {10.5281/zenodo.21844224},
  url          = {https://zenodo.org/doi/10.5281/zenodo.21844224}
}
```

---

## 📧 联系

- 主仓库（国内）: https://gitee.com/Silicon-Perception/npe
- 镜像（全球）: https://github.com/Silicon-Perception/npe
- 商业授权: alphache@163.com

## 📖 文档

| 文档 | 说明 |
|------|------|
| [集成指南](./docs/INTEGRATION_GUIDE_CN.md) | 5 分钟教程、API 使用、故障排除 |
| [协议规范](./docs/PROTOCOL_OVERVIEW_CN.md) | 帧格式、可靠性、安全、性能 |
| [快速开始](./docs/GETTING_STARTED.md) | 快速上手教程 |
| [常见问题](./docs/FAQ.md) | 常见问题解答 |
| [English Docs](./docs/INTEGRATION_GUIDE.md) | English integration guide |
| [Protocol Spec](./docs/PROTOCOL_OVERVIEW.md) | English protocol specification |

---

> 基于 NPE 架构创新，NPP SDK 为网络通提供更高效的解决方案。
