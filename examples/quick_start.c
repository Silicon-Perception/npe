/*
 * Copyright (c) 2026 吴金辉 (Jinhui Wu). All rights reserved.
 * NPP 30秒上手极简示例：5行代码实现网络通信
 */

#include <stdio.h>
#include "npp_object.h"

// 步骤1：定义网络对象（你要传输的数据）
NPP_OBJECT(temp_sensor)
    NPP_PROPERTY(float, temperature)  // 温度
    NPP_PROPERTY(float, humidity)     // 湿度
NPP_OBJECT_END

// 步骤2：定义数据变化回调
void on_temperature_change(float old_val, float new_val, void* user_data) {
    printf("温度变化: %.1f℃ -> %.1f℃\n", old_val, new_val);
    // 这里可以写自己的业务逻辑，比如控制设备、告警等
}

int main() {
    // 步骤3：初始化NPP会话
    npp_session_t* session;
    npp_session_cfg_t cfg = {
        .mode = NPP_MODE_NETWORK,
        .transport = NPP_TRANSPORT_UDP,
        .udp_port = 5000,
    };
    npp_session_create(&session, &cfg);
    
    // 步骤4：绑定对象到会话
    temp_sensor sensor;
    npp_object_bind(&sensor, session);
    npp_property_set_callback(&sensor, temperature, on_temperature_change, NULL);
    
    // 步骤5：使用数据，自动检测变化并传输
    printf("=== NPP极简示例 ===\n");
    npp_property_set(&sensor, temperature, 25.0f);
    npp_property_set(&sensor, humidity, 60.0f);
    
    // 模拟数据变化，自动触发传输
    for (int i = 0; i < 5; i++) {
        float temp = 25.0f + i * 0.5f;
        npp_property_set(&sensor, temperature, temp); // 只有变化时会自动传输
    }
    
    // 清理
    npp_session_destroy(session);
    return 0;
}
