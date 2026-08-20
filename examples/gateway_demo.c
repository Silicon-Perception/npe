/*
 * Copyright (c) 2026 吴金辉 (Jinhui Wu). All rights reserved.
 * NPP↔TCP/IP网关示例：展示NPP设备如何无缝接入现有互联网
 */

#include <stdio.h>
#include <unistd.h>
#include "gateway/npp_gateway.h"

int main() {
    printf("=== NPP↔TCP/IP网关示例 ===\n\n");
    printf("网关功能：\n");
    printf("1. NPP设备可以通过网关接入现有TCP/IP网络\n");
    printf("2. 现有TCP设备可以通过网关接入NPP网络\n");
    printf("3. 双向自动转发，对两端设备透明\n\n");
    
    // 配置网关参数
    npp_gateway_config_t config = {
        .npp_port = 5000,         // NPP协议端口
        .tcp_port = 8080,         // TCP/IP转接端口
        .bind_addr = "0.0.0.0",   // 绑定所有地址
        .max_clients = 32,        // 最大32个客户端
    };
    
    // 初始化网关
    if (npp_gateway_init(&config) != 0) {
        printf("网关初始化失败\n");
        return -1;
    }
    
    printf("网关启动成功！\n");
    printf("NPP设备请连接到: npp://localhost:5000\n");
    printf("TCP设备请连接到: tcp://localhost:8080\n");
    printf("按Ctrl+C停止网关\n\n");
    
    // 启动网关（阻塞）
    npp_gateway_start();
    
    // 清理
    npp_gateway_cleanup();
    return 0;
}
