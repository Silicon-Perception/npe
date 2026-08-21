# NPP SDK 快速上手教程
> Copyright (c) 2026 吴金辉 (Jinhui Wu). All rights reserved.

---
## 快速开始

### 步骤1：包含头文件
```c
#include <npp.h>
```

### 步骤2：创建会话
```c
npp_session_t* session;
npp_session_cfg_t cfg = {
    .mode = NPP_MODE_NETWORK,
    .transport = NPP_TRANSPORT_UDP,
    .udp_port = 5000,
};
npp_session_create(&session, &cfg);
```

### 步骤3：连接对端
```c
npp_session_connect(session, "192.168.1.100", 5001);
```

### 步骤4：发送数据
```c
uint8_t data[] = {0x01, 0x02, 0x03, 0x04};
npp_pipe_write(session, 0, data, sizeof(data));
```

### 步骤5：接收数据
```c
static void on_data(uint32_t pipe_id, const uint8_t* data, uint32_t len, void* user_data) {
    printf("收到管道 %u 的数据，长度 %u\n", pipe_id, len);
}
npp_pipe_on_data(session, 0, on_data, NULL);
```

---

## 📁 完整示例代码
查看 `examples/` 目录：
- `quick_start.c`：基础示例
- `example_sensor.c`：传感器示例

---

## 🔗 更多文档
- `FAQ.md`：常见问题解答
- `INTEGRATION_GUIDE.md`：集成指南
- `PROTOCOL_OVERVIEW.md`：协议概述
