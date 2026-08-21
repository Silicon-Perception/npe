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
- Accumulates multiple data changes
- Sends them in a single frame
- Reduces per-frame overhead

## API Usage

### Basic Pattern

```c
#include <npp.h>

// Callback for received pipe data
void on_data(uint32_t pipe_id, const uint8_t* data, uint32_t len, void* user_data) {
    printf("Pipe %u received %u bytes\n", pipe_id, len);
}

int main() {
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

    // 3. Register callback for pipe #0
    npp_pipe_on_data(session, 0, on_data, NULL);

    // 4. Connect
    npp_session_connect(session);

    // 5. Write data (auto-transmits on change)
    uint8_t data[] = {1, 2, 3, 4};
    npp_pipe_write(session, 0, data, sizeof(data));

    // 6. Cleanup
    npp_session_disconnect(session);
    npp_session_destroy(session);
    return 0;
}
```

### Handling Pipe Changes

```c
// Register callback for a specific pipe
npp_pipe_on_data(session, 0, [](uint32_t pipe_id, const uint8_t* data, uint32_t len, void* user_data) {
    printf("Pipe %u changed: %.*s\n", pipe_id, len, data);
}, NULL);
```

### Reservoir Usage

```c
// Create reservoir
npp_reservoir_config_t res_cfg = {
    .pipe_id = 0,
    .threshold = 100.0f,
    .timeout_ms = 100
};
npp_reservoir_t* reservoir;
npp_reservoir_create(&reservoir, &res_cfg);

// Add data (auto-flushes when threshold reached)
npp_reservoir_add(reservoir, data, len);

// Or manually flush
npp_reservoir_flush(reservoir);
```

### Server Mode

```c
// Create server
npp_server_config_t srv_cfg = {
    .transport = NPP_TRANSPORT_UDP,
    .port = 9999,
    .max_clients = 100
};
npp_server_t* server;
npp_server_create(&server, &srv_cfg);

// Start listening
npp_server_start(server);

// Broadcast to all clients
uint8_t msg[] = "Hello all!";
npp_server_broadcast(server, msg, sizeof(msg));

// Stop and cleanup
npp_server_stop(server);
npp_server_destroy(server);
```

## Connection States

```
IDLE → CONNECTING → ACTIVE
              ↑         │
              └─────────┘ (auto-reconnect)
```

## Error Handling

| Error Code | Meaning | Action |
|------------|---------|--------|
| NPP_OK | Success | Continue |
| NPP_ERR_INVALID_PARAM | Invalid parameter | Check configuration |
| NPP_ERR_NO_MEMORY | Out of memory | Reduce pipe count |
| NPP_ERR_TIMEOUT | Operation timed out | Retry or reconnect |
| NPP_ERR_NETWORK | Network error | Check connectivity |
| NPP_ERR_AUTH_FAILED | Authentication failed | Check encryption keys |

## Logging

```c
// Set log callback
npp_set_log_callback([](npp_log_level_t level, const char* message) {
    printf("[NPP] %s\n", message);
});

// Set error callback
npp_set_error_callback([](npp_err_t error, const char* message, void* user_data) {
    fprintf(stderr, "NPP Error %d: %s\n", error, message);
});
```

## Network Behavior

### Bandwidth Usage

| Scenario | Bandwidth | Notes |
|----------|-----------|-------|
| Static (no changes) | 0 | Silence is proof |
| Low activity (10% pipes) | ~50 bytes/frame | 96 sensors, 10Hz |
| High activity (all pipes) | ~400 bytes/frame | Full throughput |

### Recovery Behavior

1. **Network outage < timeout**: Automatic reconnection, no data loss
2. **Network outage > timeout**: Full SYNC on reconnect
3. **Server restart**: Client detects timeout, initiates SYNC

## Security

For production use with encryption:
- Contact alphache@163.com for commercial license with encryption features
- Current open-source release supports plaintext mode

## Troubleshooting

### Common Issues

| Symptom | Cause | Solution |
|---------|-------|----------|
| No data received | Firewall blocking UDP | Open port 9999/UDP |
| Connection fails | Wrong remote address | Check IP and port |
| Memory growing | Sessions not cleaned up | Call npp_session_destroy() |

### Packet Capture

To capture NPP traffic with tcpdump:

```bash
sudo tcpdump -i any port 9999 -w npp_capture.pcap
```

## Platform-Specific Notes

### Linux

```bash
# Build with static library
gcc -o app app.c -I./include -L./lib -lnpp
```

### macOS

```bash
# Build with static library
gcc -o app app.c -I./include -L./lib -lnpp
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
npp_reservoir_config_t cfg = {
    .pipe_id = 0,
    .threshold = 100.0f,    // Flush threshold
    .timeout_ms = 100        // Max batch window
};
```

| Timeout | Latency | Efficiency | Use Case |
|---------|---------|------------|----------|
| 10ms | Low | Lower | Real-time control |
| 100ms | Medium | Balanced | General IoT |
| 500ms | Higher | Highest | Low-power sensors |

## Getting Help

- Gitee Issues: https://gitee.com/Silicon-Perception/npe/issues
- GitHub Issues: https://github.com/Silicon-Perception/npe/issues
- Email: alphache@163.com
- Protocol Spec: [PROTOCOL_OVERVIEW.md](./PROTOCOL_OVERVIEW.md)
