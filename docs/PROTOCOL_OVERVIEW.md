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

### 2.1 Byte Order

All multi-byte fields use **network byte order (big-endian)**.

### 2.2 Frame Layout

```
 0                   1                   2                   3
 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
├─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┤
│Ver│ Flags │           Session ID            │    Frame Seq    │
├─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┤
│         Pipe Bitmap (variable)          │      Payload      │
├─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┤
│              CRC-32 Checksum             │     Auth Tag      │
└─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┘
```

### 2.3 Field Descriptions

| Field | Size | Description |
|-------|------|-------------|
| Ver | 4 bits | Protocol version (current: 2) |
| Flags | 4 bits | See Flags bit definition below |
| Session ID | 24 bits | Unique session identifier |
| Frame Seq | 16 bits | Sequence number for ordering (wraps at 65535) |
| Pipe Bitmap | variable | Which pipes have changed data (see encoding below) |
| Payload | variable | Pipe values (only changed ones) |
| CRC-32 | 32 bits | Integrity checksum (IEEE 802.3 polynomial) |
| Auth Tag | 128 bits | AES-GCM authentication tag (only if encrypted) |

### 2.4 Flags Bit Definition

```
Bit 0 (LSB): Encryption flag
  0 = Plaintext
  1 = Encrypted (Auth Tag present)

Bit 1: SYNC flag
  0 = Normal data frame
  1 = SYNC request/response frame

Bit 2: Multicast flag
  0 = Unicast
  1 = Multicast (Session ID = group ID)

Bit 3: Reserved (must be 0)
```

### 2.5 Pipe Bitmap Encoding

The Pipe Bitmap uses **bitset encoding**:

- Maximum 512 pipes per session
- Bitmap size = ceil(max_pipes / 8) bytes
- Bit position = pipe_id (0-511)
- Value 1 = pipe has changed data in this frame
- Value 0 = pipe unchanged

**Example** (8 pipes, pipe 0, 2, 5 changed):
```
Pipe IDs:  7 6 5 4 3 2 1 0
Bitmap:    0 0 1 0 0 1 0 1  = 0x25 (1 byte)
```

**Example** (512 pipes):
- Bitmap size: 64 bytes
- Pipe 300 changed: byte 37 (300/8), bit 4 (300%8)

### 2.6 Auth Tag Clarification

| Mode | Auth Tag | Total Header |
|------|----------|--------------|
| Plaintext | Not present | 12 bytes + bitmap |
| Encrypted | 128 bits (16 bytes) | 28 bytes + bitmap |

The Auth Tag field in the frame diagram shows the logical position. In plaintext mode, this field is omitted.

## 3. Reliability Model

### 3.1 Delivery Guarantees

| Guarantee | Level | Notes |
|-----------|-------|-------|
| At-most-once | Default | No retry, fire-and-forget |
| At-least-once | With SYNC | Full state sync on reconnect |
| Ordered | Per-pipe | Pipes are independent |

### 3.2 Sequence Number Handling

- **Size**: 16 bits (0-65535)
- **Wraparound**: Sequence numbers wrap modulo 65536
- **Duplicate detection**: Sliding window of 32 frames
- **Out-of-order handling**:
  - Within window: Buffer and reorder
  - Outside window: Drop (will recover on next SYNC)
  - Per-pipe ordering: Each pipe maintains its own expected sequence

**High-speed consideration**: At 1000 frames/second, 16-bit sequence wraps in ~65 seconds. For high-speed applications, use SYNC more frequently or upgrade to v3 (negotiated via Ver field).

### 3.3 Sync Mechanism

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
- **Timeout**: Adaptive (default 5s initial)
- **Retry strategy**:
  - Initial interval: 1 second
  - Backoff factor: 2x
  - Maximum interval: 32 seconds
  - Maximum attempts: 3
  - Jitter: ±25% random to avoid thundering herd

### 3.4 Adaptive Timeout

The SYNC timeout adapts based on network conditions:

| Condition | Timeout Adjustment |
|-----------|-------------------|
| Successful sync within 1s | Decrease timeout by 10% (min 1s) |
| Sync timeout | Increase timeout by 50% (max 30s) |
| Packet loss > 5% | Increase timeout by 25% |

### 3.5 Loss Behavior

| Scenario | Behavior |
|----------|----------|
| Single frame lost | Only affected pipes miss one update |
| Network partition | Silent until timeout, then REQ_SYNC |
| Reconnection | Automatic full state sync |
| Sequence gap detected | Mark pipe as stale, recover on SYNC |

## 4. Congestion Control

### 4.1 SYNC Storm Prevention

When multiple devices reconnect simultaneously (e.g., after network outage):

| Mechanism | Description |
|-----------|-------------|
| Random jitter | SYNC delay = random(0, 100) ms |
| Exponential backoff | Retry interval doubles each attempt |
| Rate limit | Max 1 SYNC per 10 seconds per session |

### 4.2 Send Rate Limiting

| Parameter | Default | Description |
|-----------|---------|-------------|
| Token bucket size | 100 tokens | Burst capacity |
| Token refill rate | 10 tokens/second | Sustained rate |
| Max frame rate | 1000 frames/second | Hard limit |

### 4.3 Adaptive Behavior

| Network Condition | Response |
|-------------------|----------|
| High latency (>500ms) | Reduce frame rate by 50% |
| Packet loss > 10% | Enable forward error correction |
| Buffer overflow | Drop oldest non-SYNC frames |

## 5. Connection Lifecycle

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

## 6. Security Model

### 6.1 Threat Model

NPP defends against:
- ✅ Eavesdropping (encryption)
- ✅ Tampering (authentication tag)
- ✅ Replay attacks (sequence numbers + timestamp)
- ⚠️ DoS (limited - application-layer rate limiting recommended)

### 6.2 Security Features

| Feature | Implementation | Notes |
|---------|---------------|-------|
| Encryption | AES-128-GCM | Per-session key |
| Authentication | GCM auth tag | 128-bit tag |
| Key Exchange | ECDH P-256 | On session establishment |
| Replay Protection | 64-bit nonce | Per-frame, sliding window |

### 6.3 Multicast Security

- Group key distributed via unicast session
- Key rotation: configurable interval (default 1 hour)
- New members receive current group key on join
- Multicast frames use Flags bit 2 = 1

## 7. Performance Characteristics

### 7.1 Overhead Analysis

| Component | Bytes | Notes |
|-----------|-------|-------|
| Frame header | 12 | Fixed (Ver + Flags + Session ID + Seq) |
| Pipe bitmap | 8-64 | 1 bit per pipe (max 512 pipes) |
| Auth tag | 16 | If encrypted (AES-128-GCM) |
| CRC-32 | 4 | If not using transport checksum |
| **Total overhead (plaintext)** | **24-88** | Per frame |
| **Total overhead (encrypted)** | **40-104** | Per frame |

### 7.2 Capacity Limits

| Limit | Value | Notes |
|-------|-------|-------|
| Max pipes per session | 512 | Configurable at compile time |
| Max frame size | 1400 bytes | Avoids IP fragmentation |
| Max sessions | OS limited | Typically 10K+ |
| Sync state size | ~2KB | For 512 pipes |

## 8. Interoperability

### 8.1 Wire Format

NPP wire format is **stable**. Clients and servers with the same major version can interoperate.

### 8.2 Ports

| Protocol | Port | IANA Status |
|----------|------|-------------|
| NPP/UDP | 9999 | Not registered (private use) |
| NPP/TCP | 9999 | Not registered |

### 8.3 Implementing a Compatible Client

To implement an NPP-compatible client, you need to:
1. Follow the frame format specification
2. Implement SYNC/SYNC_RESP handshake
3. Implement CRC-32 verification
4. Implement encryption (if using secure mode)

Reference implementation: NPP SDK (this repository)

### 8.4 Wireshark Dissector

A Wireshark dissector for NPP is planned. For now, use:

```bash
# Capture NPP traffic
tcpdump -i any port 9999 -w npp_capture.pcap

# Decode as raw UDP/TCP payload
# Filter: udp.port == 9999 or tcp.port == 9999
```
