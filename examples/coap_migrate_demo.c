/*
 * Copyright (c) 2026 吴金辉 (Jinhui Wu). All rights reserved.
 * CoAP迁移示例：智能家居设备从CoAP迁移到NPP
 * 这个示例展示如何用迁移工具把现有CoAP项目快速迁移到NPP
 */

#include <stdio.h>
#include "tools/coap_migrate.h"

// 原CoAP项目的业务逻辑，迁移后业务代码完全不需要修改
float indoor_temp = 22.0f;
float indoor_humidity = 55.0f;
uint8_t light_state = 0;
char device_config[64] = "default";

void sensor_read() {
    // 模拟传感器读取
    indoor_temp += 0.2f;
    if (indoor_temp > 30.0f) indoor_temp = 18.0f;
    indoor_humidity += 0.3f;
    if (indoor_humidity > 70.0f) indoor_humidity = 40.0f;
}

int main() {
    printf("=== 智能家居CoAP迁移NPP示例 ===\n\n");
    
    // 步骤1：配置迁移参数
    coap_migrate_config_t config = {
        .server_addr = "coap.smart home.com",
        .server_port = 5683,
        .npp_session = NULL,
    };
    coap_migrate_init(&config);
    
    // 步骤2：添加原有的CoAP资源映射
    coap_migrate_add_resource("/sensor/temperature", COAP_RESOURCE_SENSOR, 1); // 开启Observe
    coap_migrate_add_resource("/sensor/humidity", COAP_RESOURCE_SENSOR, 1);
    coap_migrate_add_resource("/actuator/light", COAP_RESOURCE_ACTUATOR, 0);
    coap_migrate_add_resource("/config", COAP_RESOURCE_CONFIG, 0);
    
    // 步骤3：生成NPP代码
    coap_migrate_generate_code("generated/npp_smart_home_code.c");
    
    // 步骤4：生成迁移对比报告
    coap_migrate_generate_report("generated/coap_migration_report.md");
    
    printf("\n迁移完成！\n");
    printf("生成文件：\n");
    printf("1. generated/npp_smart_home_code.c - 可直接编译运行的NPP代码\n");
    printf("2. generated/coap_migration_report.md - 迁移对比报告\n");
    
    // 清理
    coap_migrate_cleanup();
    return 0;
}
