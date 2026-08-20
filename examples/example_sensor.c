#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include "npp_object.h"
#include "core/wake_pipe.h"
#include "adaptive/adaptive.h"

/* 定义传感器对象 */
NPP_OBJECT(EnvironmentSensor) {
    NPP_PROPERTY(float, temperature);
    NPP_PROPERTY(float, humidity);
    NPP_PROPERTY(float, pressure);
} NPP_OBJECT_END;

/* 温度变化回调 */
void on_temperature_changed(float new_val, float old_val, void* user_data) {
    printf("🌡️ 温度变化: %.1f°C → %.1f°C\n", old_val, new_val);
    if (new_val > 35.0) {
        printf("⚠️ 高温警报！\n");
    } else if (new_val < 5.0) {
        printf("⚠️ 低温警报！\n");
    }
}

/* 湿度变化回调 */
void on_humidity_changed(float new_val, float old_val, void* user_data) {
    printf("💧 湿度变化: %.1f%% → %.1f%%\n", old_val, new_val);
}

/* 唤醒回调 */
void wake_callback(npp_wake_type_t type, uint32_t profile_id, void* user_data) {
    EnvironmentSensor* sensor = (EnvironmentSensor*)user_data;
    printf("🔔 设备唤醒，类型: %d，画像ID: %d\n", type, profile_id);
    
    /* 模拟读取传感器数据 */
    float new_temp = 20.0f + (rand() % 150) / 10.0f;
    float new_humidity = 40.0f + (rand() % 400) / 10.0f;
    
    npp_set(sensor, "temperature", new_temp);
    npp_set(sensor, "humidity", new_humidity);
}

int main() {
    srand(time(NULL));
    printf("===== NPP 传感器示例 =====\n");
    
    /* 初始化传感器对象 */
    EnvironmentSensor sensor;
    npp_object_init(&sensor, "env_sensor_001", NULL);
    
    /* 初始化唤醒管道 */
    npp_wake_pipe_t wake_pipe;
    npp_wake_pipe_init(&wake_pipe, NPP_STRATEGY_ON_DEMAND);
    npp_wake_pipe_register_profile(&wake_pipe, 1001, 0.1f);
    npp_wake_pipe_set_callback(&wake_pipe, wake_callback, &sensor);
    npp_adaptive_init(&wake_pipe, NPP_ADAPTIVE_TEMPLATE_SENSOR);
    npp_wake_pipe_start_listen(&wake_pipe);
    
    /* 注册属性变化回调 */
    npp_on_change(&sensor, "temperature", on_temperature_changed, NULL);
    npp_on_change(&sensor, "humidity", on_humidity_changed, NULL);
    
    /* 模拟运行 */
    for (int i = 0; i < 10; i++) {
        printf("\n--- 第%d次循环 ---\n", i + 1);
        npp_poll(&sensor);
        sleep(1);
    }
    
    printf("\n✅ 示例运行完成\n");
    return 0;
}