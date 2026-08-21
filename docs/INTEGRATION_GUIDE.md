# NPP SDK Integration Guide

## Quick Start (5 minutes)

### 1. Download SDK

```bash
# Clone from Gitee
git clone https://gitee.com/Silicon-Perception/npe.git
cd npe

# Or download release package from:
# https://gitee.com/Silicon-Perception/npe/releases
```

### 2. Build and Run

```bash
# Compile with pre-compiled library
gcc -o demo examples/quick_start.c -I./include -L./lib -lnpp

# Run
./demo
```

## Protocol Concepts

### What is a Pipe?

A **pipe** is an independent data channel within a session. Each pipe:
- Has a unique `pipe_id` (0-511)
- Maintains its own state (last known value)
- Transmits independently of other pipes

### What is a Session?

A **session** is a logical connection between two endpoints. A session:
- Contains multiple pipes
- Manages connection lifecycle
- Handles SYNC/recovery

### What is the Reservoir?

The **reservoir** is a batching mechanism that:
- Accumulates multiple pipe changes
- Sends them in a single frame
- Reduces per-frame overhead

## API Usage

### Basic Pattern

```c
#include <npp.h>

// 1. Configure session
npp_session_config_t cfg = {
    .transport = NPP_TRANSPORT_UDP,
    .local_port = 8888,
    .remote_addr = "192.168.1.100",
    .remote_port = 9999
};

// 2. Create session
npp_session_t* session;
npp_session_create(&session, &cfg);

// 3. Connect
npp_session_connect(session);

// 4. Write data (auto-transmits on change)
npp_pipe_write(session, 0, data, len);

// 5. Poll for events
while (1) {
    npp_poll(session);
}
```

### Handling Pipe Changes

```c
// Register callback for pipe #0
npp_on_change(session, 0, [](uint32_t pipe_id, const uint8_t* data, uint32_t len) {
    printf("Pipe %u changed: %.*s\n", pipe_id, len, data);
});
```

### Session Events

```c
// Register session event callbacks
npp_on_session_event(session, NPP_EVENT_CONNECTED, on_connected);
npp_on_session_event(session, NPP_EVENT_DISCONNECTED, on_disconnected);
npp_on_session_event(session, NPP_EVENT_SYNC_COMPLETE, on_sync_complete);
```

## Connection States

```
IDLE → CONNECTING → ACTIVE ↔ STALE
              ↑           │
              └── SYNC ───┘
```

| Event | What to Do |
|-------|------------|
| NPP_EVENT_CONNECTED | Start sending data |
| NPP_EVENT_DISCONNECTED | Stop sending, wait for reconnect |
| NPP_EVENT_SYNC_COMPLETE | State is consistent, resume normal operation |

## Error Handling

| Error Code | Meaning | Action |
|------------|---------|--------|
| NPP_OK | Success | Continue |
| NPP_ERR_TIMEOUT | Operation timed out | Retry or reconnect |
| NPP_ERR_INVALID | Invalid parameter | Check configuration |
| NPP_ERR_CRC | CRC mismatch | Frame corrupted, will recover on next SYNC |
| NPP_ERR_AUTH | Authentication failed | Check encryption keys |
| NPP_ERR_NO_MEM | Out of memory | Reduce pipe count |

## Network Behavior

### Bandwidth Usage

| Scenario | Bandwidth | Notes |
|----------|-----------|-------|
| Static (no changes) | 0 | Silence is proof |
| Low activity (10% pipes) | ~50 bytes/frame | 96 sensors, 10Hz |
| High activity (all pipes) | ~400 bytes/frame | Full throughput |

### Recovery Behavior

1. **Network outage < timeout**: Automatic reconnection, no data loss
2. **Network outage > timeout**: Marked STALE, full SYNC on reconnect
3. **Server restart**: Client detects timeout, initiates SYNC

## Security Configuration

### Unencrypted Mode (default)

```c
npp_session_config_t cfg = {
    .transport = NPP_TRANSPORT_UDP,
    .flags = NPP_FLAG_PLAINTEXT  // No encryption
};
```

### Encrypted Mode

```c
npp_session_config_t cfg = {
    .transport = NPP_TRANSPORT_UDP,
    .flags = NPP_FLAG_ENCRYPTED,
    .key = your_aes_key,  // 16 bytes for AES-128
    .key_len = 16
};
```

### Key Exchange

For production use, use ECDH key exchange:

```c
// Generate ephemeral key pair
npp_keypair_t keypair;
npp_generate_keypair(&keypair);

// Exchange public keys with peer (out-of-band)
// Session will derive shared secret automatically
```

## Troubleshooting

### Common Issues

| Symptom | Cause | Solution |
|---------|-------|----------|
| No data received | Firewall blocking UDP | Open port 9999/UDP |
| High latency | Reservoir too large | Reduce reservoir interval |
| State inconsistent | SYNC failed | Check network, increase timeout |
| Memory growing | Pipes not cleaned up | Call npp_session_gc() periodically |

### Debug Mode

Enable debug logging:

```c
npp_set_log_level(NPP_LOG_DEBUG);
npp_set_log_handler(my_log_handler);
```

### Packet Capture

To capture NPP traffic with tcpdump:

```bash
sudo tcpdump -i any port 9999 -w npp_capture.pcap
```

Analyze with Wireshark (NPP dissector available in `tools/wireshark/`).

## Platform-Specific Notes

### Linux

```bash
# Build shared library version
gcc -o app app.c -I./include -L./lib -lnpp -Wl,-rpath,./lib
```

### macOS

```bash
# Use .dylib for dynamic linking
gcc -o app app.c -I./include -L./lib -lnpe.4
```

### MCU (Embedded)

```c
// MCU uses static library only
// Memory constraints: reduce MAX_PIPES to 64
#define NPP_MAX_PIPES 64
```

## Performance Tuning

### Reservoir Configuration

```c
npp_session_config_t cfg = {
    .reservoir_interval_ms = 50,  // Batch window (default: 100ms)
    .reservoir_max_frames = 4     // Max frames per batch (default: 8)
};
```

| Interval | Latency | Efficiency | Use Case |
|----------|---------|------------|----------|
| 10ms | Low | Lower | Real-time control |
| 100ms | Medium | Balanced | General IoT |
| 500ms | Higher | Highest | Low-power sensors |

### MTU Considerations

- Default max frame: 1400 bytes (avoids IP fragmentation)
- For networks with larger MTU (e.g., loopback), increase:
  ```c
  cfg.max_frame_size = 8192;  // For local networks
  ```

## Thread Safety and Memory Management

### Thread Safety

| Function | Thread Safe | Notes |
|----------|-------------|-------|
| npp_session_create() | ✅ Yes | Call once per session |
| npp_session_destroy() | ✅ Yes | Call once per session |
| npp_pipe_write() | ✅ Yes | Can be called from multiple threads |
| npp_poll() | ❌ No | Single thread per session |
| npp_on_change() | ✅ Yes | Register before connect |

**Recommended pattern**:
- One thread for `npp_poll()` (event loop)
- Any thread can call `npp_pipe_write()`
- Register callbacks before `npp_session_connect()`

### Memory Management

| Object | Allocated By | Freed By |
|--------|--------------|----------|
| npp_session_t | npp_session_create() | npp_session_destroy() |
| npp_session_config_t | Caller (stack) | Caller |
| Pipe data (write) | Caller | Caller (after npp_pipe_write returns) |
| Pipe data (read) | SDK (internal buffer) | SDK (on next poll or destroy) |

**Important**: 
- `npp_pipe_write()` copies the data immediately — caller can free after return
- Read callbacks provide data pointer valid only during callback execution
- Copy data if you need it after callback returns

### Callback Context

| Callback | Executed In | Can Block |
|----------|-------------|-----------|
| npp_on_change() | npp_poll() thread | ❌ No |
| npp_on_session_event() | npp_poll() thread | ❌ No |

**Important**:
- Callbacks execute in the `npp_poll()` thread
- Do NOT call `npp_poll()` from within a callback
- Do NOT perform long-running operations in callbacks
- Use queues to defer work to other threads

## Getting Help

- GitHub Issues: https://github.com/Silicon-Perception/npe/issues
- Gitee Issues: https://gitee.com/Silicon-Perception/npe/issues
- Email: alphache@163.com
- Protocol Spec: [PROTOCOL_OVERVIEW.md](./PROTOCOL_OVERVIEW.md)
