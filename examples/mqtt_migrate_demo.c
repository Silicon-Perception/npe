/*
 * Copyright (c) 2026 吴金辉 (Jinhui Wu). All rights reserved.
 * MQTT迁移示例：工业温湿度传感器从MQTT迁移到NPP
 * 这个示例展示如何用迁移工具把现有MQTT项目快速迁移到NPP
 */

#include <stdio.h>
#include "tools/mqtt_migrate.h"

// 原MQTT项目的业务逻辑，迁移后业务代码完全不需要修改
float temperature = 25.0f;
float humidity = 60.0f;

void sensor_read() {
    // 模拟传感器读取
    temperature += 0.1f;
    if (temperature > 40.0f) temperature = 20.0f;
    humidity += 0.5f;
    if (humidity > 80.0f) humidity = 40.0f;
}

int main() {
    printf("=== 工业传感器MQTT迁移NPP示例 ===\n\n");
    
    // 步骤1：配置迁移参数
    mqtt_migrate_config_t config = {
        .broker_addr = "mqtt.iot.aliyun.com",
        .broker_port = 1883,
        .client_id = "sensor_001",
        .npp_session = NULL,
    };
    mqtt_migrate_init(&config);
    
    // 步骤2：添加原有的MQTT主题映射
    mqtt_migrate_add_topic("sensor/001/temperature", MQTT_TOPIC_SINGLE, 1);
    mqtt_migrate_add_topic("sensor/001/humidity", MQTT_TOPIC_SINGLE, 1);
    mqtt_migrate_add_topic("sensor/001/config", MQTT_TOPIC_COMMAND, 0);
    
    // 步骤3：生成NPP代码
    mqtt_migrate_generate_code("generated/npp_sensor_code.c");
    
    // 步骤4：生成迁移对比报告
    mqtt_migrate_generate_report("generated/migration_report.md");
    
    printf("\n迁移完成！\n");
    printf("生成文件：\n");
    printf("1. generated/npp_sensor_code.c - 可直接编译运行的NPP代码\n");
    printf("2. generated/migration_report.md - 迁移对比报告\n");
    
    // 清理
    mqtt_migrate_cleanup();
    return 0;
}
