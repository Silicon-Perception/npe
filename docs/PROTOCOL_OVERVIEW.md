# NPP Protocol Overview

## 1. Protocol Layering

NPP operates at the **Application Layer**, running on top of existing transport protocols:

```
┌─────────────────────────────────────────┐
│  Application Layer: NPP (this protocol) │
├─────────────────────────────────────────┤
│  Transport Layer: UDP / TCP             │
├─────────────────────────────────────────┤
│  Network Layer: IP                      │
└─────────────────────────────────────────┘
```

NPP does NOT replace UDP/TCP. It provides:
- State synchronization semantics
- Change-only transmission
- Pipe multiplexing

## 2. Frame Structure

### 2.1 Frame Layout

```
 0                   1                   2                   3
 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
├─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┤
│Ver│ Flags │           Session ID            │    Frame Seq    │
├─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┤
│         Pipe Bitmap (variable)          │      Payload      │
├─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┤
│              CRC-32 Checksum             │     Auth Tag     │
└─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┘
```

### 2.2 Field Descriptions

| Field | Size | Description |
|-------|------|-------------|
| Ver | 4 bits | Protocol version (current: 2) |
| Flags | 4 bits | Frame type, encryption flags |
| Session ID | 24 bits | Unique session identifier |
| Frame Seq | 16 bits | Sequence number for ordering |
| Pipe Bitmap | variable | Which pipes have changed data |
| Payload | variable | Pipe values (only changed ones) |
| CRC-32 | 32 bits | Integrity checksum |
| Auth Tag | 32 bits | Authentication (if encrypted) |

## 3. Reliability Model

### 3.1 Delivery Guarantees

| Guarantee | Level | Notes |
|-----------|-------|-------|
| At-most-once | Default | No retry, fire-and-forget |
| At-least-once | With SYNC | Full state sync on reconnect |
| Ordered | Per-pipe | Pipes are independent |

### 3.2 Sync Mechanism

```
Client                          Server
  │                               │
  │◄──────── SYNC_REQ ────────────│  "I need full state"
  │                               │
  │────────── SYNC_RESP ─────────►│  "Here is full state"
  │                               │
  │◄─────── Normal Frames ────────│  "Incremental updates"
```

- **SYNC** is client-initiated full state request
- **SYNC_RESP** contains all current pipe values
- Timeout: configurable (default 5s)
- Retry: exponential backoff, max 3 attempts

### 3.3 Loss Behavior

| Scenario | Behavior |
|----------|----------|
| Single frame lost | Only affected pipes miss one update |
| Network partition | Silent until timeout, then REQ_SYNC |
| Reconnection | Automatic full state sync |

## 4. Connection Lifecycle

```
┌──────────┐     connect()      ┌──────────┐
│ IDLE     │ ─────────────────► │CONNECTING│
└──────────┘                    └──────────┘
     ▲                              │
     │                          success
     │                              ▼
     │    disconnect()       ┌──────────┐     timeout     ┌────────┐
     │ ◄──────────────────── │  ACTIVE  │ ─────────────► │ STALE  │
     │                       └──────────┘                 └────────┘
     │                           │                            │
     │                           │◄───── REQ_SYNC ────────────┘
     │                           │      (auto recovery)
     └───────────────────────────┘
```

| State | Description | Transitions |
|-------|-------------|-------------|
| IDLE | No connection | → CONNECTING on connect() |
| CONNECTING | Handshake in progress | → ACTIVE on success |
| ACTIVE | Normal operation | → STALE on timeout |
| STALE | Connection lost, data stale | → ACTIVE on SYNC success |

## 5. Security Model

### 5.1 Threat Model

NPP defends against:
- ✅ Eavesdropping (encryption)
- ✅ Tampering (authentication tag)
- ✅ Replay attacks (sequence numbers + timestamp)
- ⚠️ DoS (limited - application-layer rate limiting recommended)

### 5.2 Security Features

| Feature | Implementation | Notes |
|---------|---------------|-------|
| Encryption | AES-128-GCM | Per-session key |
| Authentication | HMAC-SHA256 | Per-frame |
| Key Exchange | ECDH P-256 | On session establishment |
| Replay Protection | 64-bit nonce | Per-frame, sliding window |

### 5.3 Multicast Security

- Group key distributed via unicast session
- Key rotation: configurable interval (default 1 hour)
- New members receive current group key on join

## 6. Performance Characteristics

### 6.1 Overhead Analysis

| Component | Bytes | Notes |
|-----------|-------|-------|
| Frame header | 12 | Fixed |
| Pipe bitmap | 8-64 | 1 bit per pipe (max 512 pipes) |
| Auth tag | 4 | If encrypted |
| **Total overhead** | **24-88** | Per frame |

### 6.2 Capacity Limits

| Limit | Value | Notes |
|-------|-------|-------|
| Max pipes per session | 512 | Configurable at compile time |
| Max frame size | 1400 bytes | Avoids IP fragmentation |
| Max sessions | OS limited | Typically 10K+ |
| Sync state size | ~2KB | For 512 pipes |

## 7. Interoperability

### 7.1 Wire Format

NPP wire format is **stable since v2.0**. v2.x clients can communicate with v2.x servers.

### 7.2 Ports

| Protocol | Port | IANA Status |
|----------|------|-------------|
| NPP/UDP | 9999 | Not registered (private use) |
| NPP/TCP | 9999 | Not registered |

### 7.3 Implementing a Compatible Client

To implement an NPP-compatible client, you need to:
1. Follow the frame format specification
2. Implement SYNC/SYNC_RESP handshake
3. Implement CRC-32 verification
4. Implement encryption (if using secure mode)

Reference implementation: NPP SDK (this repository)
