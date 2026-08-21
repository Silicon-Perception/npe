# Changelog

All notable changes to NPP SDK will be documented in this file.

---

## [1.0.260821.1] - 2026-08-21

### Added
- **Core Architecture**: Pipeline-based network communication framework
- **NPP Object System**: Define network objects like structs, auto-sync properties
- **Wake Pipe Mechanism**: Zero-power listening, wake on change
- **Pipe Frequency Hopping**: HMAC-SHA256 based secure channel hopping
- **Reservoir**: Batch delivery with water hammer detection
- **Pipe Coloring**: Priority/Security/Audit/Route tagging
- **Session Management**: State machine, health check, SYNC synchronization
- **Server & Multicast**: One-to-many broadcast, native multicast support
- **Transport Layer**: UDP/TCP/RF abstraction
- **Migration Tools**: COAP and MQTT protocol migration examples

### Performance
- **82x compression ratio** (static scene, verified)
- **60-90% bandwidth savings** (verified with 64-channel sensor demo)
- **186 test assertions, 100% pass rate**

### Documentation
- README (English & Chinese)
- FAQ and Getting Started Guide
- 7 animated GIF demonstrations
- Test reports and design docs

---

## [v1.0.0] - 2026-07-30 (Internal)

### Added
- Initial NPE engine implementation
- IPA differential detection
- EPA region aggregation
- Skip-Value stream encoding
- Cross-platform HAL (CPU/SIMD/GPU/MCU)

---

*This project uses [Semantic Versioning](http://semver.org/).*
