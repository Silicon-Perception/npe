# NPP SDK - High-Performance Network Communication Engine Based on NPE Architecture

> Pure C-language implementation of a semi-open-source network communication protocol stack, replacing traditional full transmission with incremental transmission paradigm, significantly reducing computing and bandwidth overhead.

<div align="center">

[![License: MIT](https://img.shields.io/badge/License%20(Open%20Source)-MIT-yellow.svg)](LICENSE)
[![C](https://img.shields.io/badge/C-99-blue.svg)](https://en.wikipedia.org/wiki/C99)
[![Platform](https://img.shields.io/badge/Platform-Linux%20%7C%20macOS%20%7C%20MCU-green.svg)]()
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

## ⚡ Traditional vs NPP: Communication Paradigm Comparison

### Traditional Network Communication
![Traditional Communication](images/a1.gif)
- **Full transmission**: Complete data transmitted every frame, even if 99% of data remains unchanged
- **Periodic heartbeat**: Continuous keep-alive packets even when device is idle, consuming bandwidth and power
- **Computation-intensive**: CPU participates in packet assembly/disassembly, verification/retransmission

### NPP Network Communication
![NPP Communication](images/a2.gif)
- **Incremental transmission**: Only changed pipeline data is transmitted; zero bandwidth when static
- **Silence is proof**: No data change = device online, no heartbeat needed
- **Comparison-driven**: Core operations are only comparison and assignment, no multiplication/division

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
- **Open-source upper layer**: Minimal API, NPP Object encapsulation, test cases, sample code, migration tools
- **Business-friendly**: Open-source parts under MIT license, closed-source parts available for commercial license

### ⚡ C Language Implementation: Performance and Cross-Platform
- **Native performance**: Pure C implementation, 100x faster than Python/Java
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

## 🔥 Core Superpowers

NPP SDK encapsulates complex network communication into simple, powerful capabilities:

### 🚀 Zero-Config Deployment
```c
// Just 3 lines of code to establish a complete network connection
npp_object_init(&sensor, "sensor_001", NULL);
npp_session_deploy(&sensor, 1);
// Done! All communication handled automatically
```

### ⚡ Silence is Proof (Zero Bandwidth Heartbeat)
- No data change = link completely silent
- No heartbeat packets, no keep-alive traffic
- **95%+ bandwidth savings** in static scenarios
- Itself proof of device online

### 🔄 State Self-Healing
- Built-in SYNC/REQ_SYNC mechanism
- Automatic full-state sync after reconnection
- **<100ms** recovery time (vs 1-10s for traditional TCP)

### 📡 Native Multicast
- One wake-up for multiple devices
- Single frame to thousands of terminals
- **10x development efficiency** vs extra implementation needed

### 💧 Smart Batch Delivery (Reservoir)
- Accumulates multiple changes, delivers at once
- Saves frame header overhead
- Automatic adaptation to network conditions

### 📡 Pipe Frequency Hopping
- Pipe-level dynamic frequency hopping
- Automatically switches channel on interference
- Ensures reliable communication

### 🔍 Full Auditability
- All data changes traceable
- Transparent and verifiable
- Compliant with industrial/financial requirements

---

## 🛠️ Quick Start

### 30-Second Demo: Define Network Object, Automatic Data Sync

```c
/* 1. Define network object, as simple as defining a struct */
NPP_OBJECT(EnvironmentSensor) {
    NPP_PROPERTY(float, temperature);  // Temperature, change threshold 0.1℃
    NPP_PROPERTY(float, humidity);     // Humidity, change threshold 1%
    NPP_PROPERTY(uint32_t, pm25);      // PM2.5, change threshold 1μg/m³
} NPP_OBJECT_END;

int main() {
    /* 2. Initialize object and NPP session */
    EnvironmentSensor sensor;
    npp_object_init(&sensor, "sensor_001", NULL);
    
    /* 3. Register property change callback, auto-trigger network send */
    npp_on_change(&sensor, "temperature", on_temp_changed, NULL);
    
    /* 4. Deploy session, automatic pipe setup and connection establishment */
    npp_session_deploy(&sensor, 1); // 1=sender, 0=receiver
    
    /* 5. Normal read/write, NPP handles all communication logic automatically */
    npp_set(&sensor, "temperature", 25.5f); // Auto-send if change exceeds threshold
    npp_set(&sensor, "humidity", 60.0f);
    
    while(1) {
        npp_poll(&sensor); // Handle network events, state sync
        sleep(1);
    }
    return 0;
}
```

> Developers don't need to worry about heartbeat packets, retransmission, packet assembly, bandwidth optimization — NPP handles everything automatically.

---

## 📊 Performance Metrics (Verified Test Data)

### Compression Performance

| Scene | Traditional JSON | NPP SDK | Compression Ratio |
|-------|-----------------|---------|-------------------|
| Static (no change) | ~990B | ~12B | **82x** |
| Low frequency (10% change) | ~990B | ~50B | **20x** |
| Medium frequency (30% change) | ~990B | ~150B | **6.6x** |
| High frequency (70% change) | ~990B | ~350B | **2.8x** |

### Bandwidth Savings

| Pipe Type | Traditional MQTT | NPP | Savings |
|-----------|-----------------|-----|---------|
| Session-level (18 pipes) | Every frame | Silent after first frame | **99%** |
| Frame-level (12 pipes) | Every frame | Only on change | **70-90%** |
| Data-level (32 pipes) | Every frame | Only on change | **60-80%** |
| **Total** | **~560B** | **~20-100B** | **60-90%** |

### Test Coverage

| Module | Test Cases | Pass Rate |
|--------|-----------|-----------|
| Schema Tests | 4 | 100% |
| Frame Tests | 3 | 100% |
| Crypto Tests | 2 | 100% |
| Backend Tests | 3 | 100% |
| Session Tests | 5 | 100% |
| Server Tests | 5 | 100% |
| Hopping Tests | 2 | 100% |
| Reservoir Tests | 2 | 100% |
| Coloring Tests | 1 | 100% |
| **Total** | **186 assertions** | **100%** |

> Data source: NPP SDK v2.0 official test report, 64-channel sensor real-time demo verified.

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
npp-sdk/
├── include/              // Public headers (open source, MIT license)
├── lib/                  // Pre-compiled binaries (closed source, commercial license)
│   ├── libnpp.a          // Static library
│   └── README.md         // Download instructions
├── tests/                // Unit tests (open source, MIT license)
├── examples/             // Sample code (open source, MIT license)
├── tools/                // Protocol migration tools (open source, MIT license)
├── docs/                 // Design documents (open source, CC-BY license)
├── scripts/              // Build scripts
│   ├── build_release.sh  // Build pre-compiled binaries
│   └── check_release.sh  // Pre-release check
├── MAINTENANCE.md        // Maintenance manual
├── RELEASE_CHECKLIST.md  // Release checklist
├── CONTRIBUTING.md       // Contribution guide
└── LICENSE               // Semi-open-source license
```

---

## 🚀 Quick Start

### 1. Download SDK

```bash
git clone https://gitee.com/Silicon-Perception/npe.git
cd npe
```

### 2. Use Pre-compiled Library

```c
// Only need to include one header file!
#include <npp.h>

int main() {
    // Create session configuration
    npp_session_config_t cfg = {
        .transport = NPP_TRANSPORT_UDP,
        .local_port = 8888,
        .remote_port = 9999,
        .remote_addr = "192.168.1.100"
    };
    
    // Create session
    npp_session_t* session;
    npp_session_create(&session, &cfg);
    npp_session_connect(session);
    
    // Write data
    uint8_t data[] = {1, 2, 3, 4};
    npp_pipe_write(session, 0, data, sizeof(data));
    
    // Cleanup
    npp_session_destroy(session);
    return 0;
}
```

### 3. Compile & Run

```bash
gcc -o my_app my_app.c -I./include -L./lib -lnpp
./my_app
```

---

## 🤝 Contributing & Ecosystem

We welcome developers to participate in NPP ecosystem building:
1. Submit bug reports and suggestions
2. Contribute sample code and demos
3. Submit protocol migration rules to support more traditional protocol conversions
4. Participate in upper-layer application development

Note: Core closed-source parts do not accept external PRs; contributions to open-source parts only.

---

## 📜 License

This project uses **semi-open-source license**:
- Open-source parts (include/tests/examples/tools/docs) follow MIT license
- Closed-source parts (src/core implementation) require commercial license
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
- Ecosystem Discussion: https://gitee.com/Silicon-Perception/npe/discussions

## 📖 Documentation

| Document | Description |
|----------|-------------|
| [Quick Start](./docs/INTEGRATION_GUIDE.md) | 5-minute tutorial, API usage, troubleshooting |
| [Protocol Overview](./docs/PROTOCOL_OVERVIEW.md) | Frame format, reliability, security, performance |
| [System Design](./docs/system-design.md) | Architecture details |
| [Changelog](./CHANGELOG.md) | Version history |
| [中文文档](./docs/INTEGRATION_GUIDE_CN.md) | 中文版集成指南 |

<!--
---

## 💝 Support This Project

If NPP SDK is helpful to you, please consider supporting us:

| Alipay | WeChat Pay |
|--------|------------|
| ![Alipay](images/alipay_pay.png) | ![WeChat](images/wechat_pay.png) |
-->


---

> Based on NPE architecture innovation, NPP SDK provides a more efficient solution for network communication.
