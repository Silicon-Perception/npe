# NPP SDK - Lightweight IoT Communication Protocol Stack

> Pure C-language implementation of an open-source network communication protocol stack, featuring incremental transmission design for low-power, low-bandwidth IoT scenarios. NPP protocol targets sensor networks, edge computing, and similar use cases with simple API and efficient incremental transmission capabilities.

<div align="center">

[![License: MIT](https://img.shields.io/badge/License%20(Open%20Source)-MIT-yellow.svg)](LICENSE)
[![C](https://img.shields.io/badge/C-99-blue.svg)](https://en.wikipedia.org/wiki/C99)
[![Platform](https://img.shields.io/badge/Platform-Linux%20%7C%20macOS%20%7C%20Windows%20%7C%20MCU-green.svg)]()
[![Version](https://img.shields.io/badge/Version-1.0.260821.1-blue.svg)](VERSION)
[![Build](https://img.shields.io/badge/Build-CMake-blue.svg)]()
[![DOI](https://img.shields.io/badge/DOI-10.5281%2Fzenodo.21844224-blue.svg)](https://zenodo.org/doi/10.5281/zenodo.21844224)
[![ORCID](https://img.shields.io/badge/ORCID-0009--0007--6411--7426-green)](https://orcid.org/0009-0007-6411-7426)

[English](./README.md) | [中文](./README_CN.md)

</div>

---

## 🎯 Core Concept: Pipeline Architecture

NPP is based on NPE (Natural Pipeline Engine) architecture, transforming network communication from "computation-driven" to "comparison-driven".

![Pipeline Design](images/a0.gif)
*Frame splitting into pipeline format: Each data stream is decomposed into independent "pipelines", each self-maintaining its state*

---

## ⚡ Communication Paradigm Comparison

### Traditional Polling Approach
![Traditional Communication](images/a1.gif)
- Periodic data queries, even when most data hasn't changed
- Connection management overhead
- CPU involved in packet processing

### NPP Approach
![NPP Communication](images/a2.gif)
- Incremental transmission: only changed data is transmitted
- No connection state management
- Simple compare-and-update operations

**NPP Design Features:**
- **Silence is proof**: No data change indicates device is online (no explicit heartbeat needed)
- **Incremental updates**: Core operations focus on detecting and transmitting changes
- **Zero computation**: Core operations are comparison and assignment, no multipliers, dividers, transforms, or entropy coding

---

## ✨ Core Features Showcase

### 🏗️ Pipeline Architecture
![Pipeline Design](images/a0.gif)
*Frame splitting into pipeline format: Each data stream is decomposed into independent "pipelines", each self-maintaining its state*

### 💧 Reservoir Mechanism
![Reservoir](images/a3.gif)
*Batch delivery mechanism: Accumulates multiple changes and delivers them at once, saving frame header overhead and improving transmission efficiency*

### 📡 Pipe Frequency Hopping
![Pipe Frequency Hopping](images/a4.gif)
*Pipe-level frequency hopping: Automatically switches pipe transmission channel in case of interference, ensuring reliable communication*

### 🔍 Auditability
![Auditability](images/a5.gif)
*Complete audit trail: All data changes are traceable, ensuring transparency and verifiability*

### 🎨 Pipe Coloring
![Pipe Coloring](images/a6.gif)
*Data flow visualization: Color-coded pipe states for intuitive monitoring of network health*

---

## ✨ Core Advantages

### 🔓 Semi-Open-Source Mode: Balancing Ecosystem and IP Protection
- **Closed-source core**: Core protocol layer, wake-up scheduling, adaptive algorithms — protected intellectual property
- **Open-source upper layer**: Minimal API, sample code, documentation
- **Business-friendly**: Open-source parts under MIT license, closed-source parts available for commercial license

### ⚡ C Language Implementation: Performance and Cross-Platform
- **Native performance**: Pure C implementation
- **Ultra-lightweight**: Minimum runtime <64KB ROM / 8KB RAM, runs on 8-bit MCUs to 64-bit servers
- **Seamless integration**: Compatible with all C/C++ environments, embeddable in Linux/RTOS/Bare-Metal

### 🌐 Paradigm Innovation
- **Silence is proof**: No data change means link is silent — itself proof of device online
- **Self-healing state**: Built-in SYNC/REQ_SYNC mechanism, automatic full-state sync after reconnection
- **Semantic-level fault tolerance**: UDP frame loss affects only single field, not entire system
- **Native multicast**: Wake multiple devices simultaneously, distribute single frame to thousands of terminals

---

## 🏗️ Protocol Layering

NPP operates at the **Application Layer**, running on top of existing transport protocols:

```
┌─────────────────────────────────────────┐
│  Application Layer: NPP (this protocol) │  ← You are here
├─────────────────────────────────────────┤
│  Transport Layer: UDP / TCP             │
├─────────────────────────────────────────┤
│  Network Layer: IP                      │
└─────────────────────────────────────────┘
```

NPP does **NOT** replace UDP/TCP. It provides:
- **State synchronization**: Change-only transmission, no redundant data
- **Pipe multiplexing**: Up to 512 independent data channels per session
- **Self-healing**: Automatic full-state SYNC after reconnection

See [Protocol Overview](./docs/PROTOCOL_OVERVIEW.md) for frame format, reliability model, and security details.

---

## 🛠️ Quick Start

### Basic Example: Session and Pipe I/O

```c
#include <npp.h>

int main() {
    /* 1. Configure session */
    npp_session_config_t cfg = {
        .transport = NPP_TRANSPORT_UDP,
        .local_port = 8888,
        .remote_port = 9999,
        .remote_addr = "192.168.1.100"
    };

    /* 2. Create session */
    npp_session_t* session;
    npp_session_create(&session, &cfg);
    npp_session_connect(session);

    /* 3. Write data to pipe #0 (auto-transmits on change) */
    uint8_t data[] = {1, 2, 3, 4};
    npp_pipe_write(session, 0, data, sizeof(data));

    /* 4. Cleanup */
    npp_session_disconnect(session);
    npp_session_destroy(session);
    return 0;
}
```

Compile and run:
**Linux/macOS:**
```bash
gcc -o my_app my_app.c -I./include -L./lib -lnpp
./my_app
```

**Windows (Visual Studio Developer Command Prompt):**
```cmd
cl my_app.c /I./include /link /LIBPATH:./lib npp.lib
my_app.exe
```

---

## 📊 Performance Characteristics

### Design Goals

- **Incremental transmission**: Only changed data is transmitted
- **Low overhead**: Minimal protocol overhead for small data packets
- **Flexible**: Supports various IoT and embedded scenarios

### Bandwidth Efficiency (Typical Scenarios)

| Scenario | Polling Approach | NPP | Notes |
|----------|-----------------|-----|-------|
| Static (no changes) | Periodic queries | Near zero | No data change = minimal traffic |
| Low activity | Periodic queries | Significantly lower | Only changed data transmitted |
| High activity | Periodic queries | Lower | Incremental updates reduce overhead |

*Actual results depend on specific use case, network conditions, and configuration.*

### Test Coverage

| Module | Tests | Status |
|--------|-------|--------|
| Basic API | 8 | ✅ Passing |
| Session Lifecycle | 5 | ✅ Passing |
| Pipe I/O | 6 | ✅ Passing |

---

## 🧩 Supported Scenarios

- **IoT**: Smart home, industrial sensors, agricultural monitoring, energy monitoring
- **Edge Computing**: Edge node communication, local data sync, low-power WAN
- **Embedded Devices**: MCU communication, device-to-device direct connection, RTOS systems
- **Distributed Systems**: Node state sync, service discovery, configuration distribution
- **Weak Network Environments**: Satellite communication, underground pipelines, high-speed mobile scenarios

---

## 📁 Project Structure

```
npe/
├── include/              // Public headers (open source, MIT license)
│   └── npp.h             // Main public API
├── lib/                  // Pre-compiled binaries (closed source, commercial license)
│   ├── libnpp.a          // NPP protocol static library
│   ├── libnpe.a          // NPE engine static library
│   └── README.md         // Library documentation
├── tests/                // Unit tests (open source, MIT license)
│   ├── test_e2e.c        // Basic API tests
│   └── test_wake.c       // Session/pipe tests
├── examples/             // Sample code (open source, MIT license)
│   ├── quick_start.c     // Basic session example
│   └── example_sensor.c  // Sensor simulation
├── docs/                 // Documentation
│   ├── PROTOCOL_OVERVIEW.md    // Protocol specification
│   ├── INTEGRATION_GUIDE.md    // Integration guide
│   ├── PROTOCOL_OVERVIEW_CN.md // Chinese protocol spec
│   ├── INTEGRATION_GUIDE_CN.md // Chinese integration guide
│   ├── GETTING_STARTED.md      // Quick start tutorial
│   └── FAQ.md                  // FAQ
├── images/               // Demo GIFs and images
├── CMakeLists.txt        // Build configuration
├── CONTRIBUTING.md       // Contribution guide
└── LICENSE               // Semi-open-source license
```

---

## 🤝 Contributing & Ecosystem

We welcome developers to participate in NPP ecosystem building:
1. Submit bug reports and suggestions
2. Contribute sample code and demos
3. Participate in upper-layer application development

Note: Core closed-source parts do not accept external PRs; contributions to open-source parts only.

---

## 📜 License

This project uses **semi-open-source license**:
- Open-source parts (include/tests/examples/docs) follow MIT license
- Closed-source parts (core implementation) require commercial license
- See [LICENSE](./LICENSE) for details
- Copyright: Jinhui Wu, ORCID: 0009-0007-6411-7426

## 📄 Citation

If you use this project in your research, please cite:

```bibtex
@software{npp_sdk_2026,
  author       = {Jinhui Wu},
  title        = {NPP SDK: High-Performance Network Communication Engine Based on NPE Architecture},
  month        = aug,
  year         = 2026,
  publisher    = {Zenodo},
  doi          = {10.5281/zenodo.21844224},
  url          = {https://zenodo.org/doi/10.5281/zenodo.21844224}
}
```

---

## 📧 Contact

- Primary Repository (China): https://gitee.com/Silicon-Perception/npe
- Mirror (Global): https://github.com/Silicon-Perception/npe
- Commercial License: alphache@163.com

## 📖 Documentation

| Document | Description |
|----------|-------------|
| [Integration Guide](./docs/INTEGRATION_GUIDE.md) | 5-minute tutorial, API usage, troubleshooting |
| [Protocol Overview](./docs/PROTOCOL_OVERVIEW.md) | Frame format, reliability, security, performance |
| [Getting Started](./docs/GETTING_STARTED.md) | Quick start tutorial |
| [FAQ](./docs/FAQ.md) | Frequently asked questions |
| [中文文档](./docs/INTEGRATION_GUIDE_CN.md) | 中文版集成指南 |
| [中文协议规范](./docs/PROTOCOL_OVERVIEW_CN.md) | 中文版协议规范 |

---

> Based on NPE architecture innovation, NPP SDK provides a more efficient solution for network communication.
