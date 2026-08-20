#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <sys/time.h>
#include "core/wake_pipe.h"
#include "multicast/multicast.h"
#include "adaptive/adaptive.h"

/* 测试多画像唤醒管理 */
void test_multi_profile_wake() {
    printf("测试多画像唤醒管理...\n");
    npp_wake_pipe_t pipe;
    npp_wake_pipe_init(&pipe, NPP_STRATEGY_ON_DEMAND);
    
    /* 注册多个画像 */
    npp_wake_pipe_register_profile(&pipe, 1001, 0.1f);
    npp_wake_pipe_register_profile(&pipe, 1002, 0.2f);
    npp_wake_pipe_register_profile(&pipe, 1003, 0.3f);
    
    /* 测试单画像唤醒 */
    npp_wake_pipe_trigger_wake(&pipe, NPP_WAKE_TYPE_DATA_CHANGE, 1001);
    
    /* 测试多画像并发唤醒 */
    npp_wake_pipe_trigger_wake(&pipe, NPP_WAKE_TYPE_DATA_CHANGE, 1002);
    npp_wake_pipe_trigger_wake(&pipe, NPP_WAKE_TYPE_DATA_CHANGE, 1003);
    
    /* 测试无效画像过滤 */
    npp_wake_pipe_trigger_wake(&pipe, NPP_WAKE_TYPE_DATA_CHANGE, 9999);
    
    printf("✅ 多画像唤醒管理测试通过\n");
}

/* 测试唤醒失败退避机制 */
void test_wake_backoff() {
    printf("测试唤醒失败退避机制...\n");
    npp_wake_pipe_t pipe;
    npp_wake_pipe_init(&pipe, NPP_STRATEGY_ON_DEMAND);
    
    wake_backoff_ctx_t* ctx = &pipe.backoff_ctx;
    
    /* 测试首次失败 */
    npp_wake_backoff_fail(ctx);
    assert(ctx->fail_count == 1);
    assert(ctx->next_retry_ms == 1000);
    
    /* 测试连续失败退避 */
    npp_wake_backoff_fail(ctx);
    assert(ctx->fail_count == 2);
    assert(ctx->next_retry_ms == 2000);
    
    npp_wake_backoff_fail(ctx);
    assert(ctx->fail_count == 3);
    assert(ctx->next_retry_ms == 4000);
    
    /* 测试超过最大重试次数 */
    npp_wake_backoff_fail(ctx);
    npp_wake_backoff_fail(ctx);
    assert(ctx->fail_count == 5);
    assert(ctx->is_deep_sleep == 1);
    assert(npp_wake_backoff_can_retry(ctx) == 0);
    
    /* 测试成功重置 */
    npp_wake_backoff_success(ctx);
    assert(ctx->fail_count == 0);
    assert(ctx->is_deep_sleep == 0);
    assert(npp_wake_backoff_can_retry(ctx) == 1);
    
    printf("✅ 唤醒失败退避机制测试通过\n");
}

/* 测试多播唤醒 */
void test_multicast_wake() {
    printf("测试多播唤醒...\n");
    npp_wake_pipe_t pipe;
    npp_wake_pipe_init(&pipe, NPP_STRATEGY_ON_DEMAND);
    
    /* 创建多播组 */
    npp_multicast_create_group(&pipe, 2001, 0x12345678);
    npp_multicast_join_group(&pipe, 2001, 0x00000001);
    npp_multicast_join_group(&pipe, 2001, 0x00000002);
    
    /* 测试发送多播唤醒 */
    npp_multicast_send_wake(&pipe, 2001);
    
    /* 测试接收有效多播帧 */
    npp_multicast_wake_frame_t frame = {
        .frame_type = 0x03,
        .wake_addr = 0x12345678,
        .group_id = 2001,
        .ttl = 1,
        .checksum = 0
    };
    frame.checksum = npp_multicast_calc_checksum(&frame);
    npp_multicast_handle_frame(&pipe, &frame, sizeof(frame));
    
    /* 测试过滤非本组帧 */
    frame.group_id = 9999;
    frame.checksum = npp_multicast_calc_checksum(&frame);
    npp_multicast_handle_frame(&pipe, &frame, sizeof(frame));
    
    printf("✅ 多播唤醒测试通过\n");
}

/* 测试自适应调整 */
void test_adaptive_adjust() {
    printf("测试自适应调整...\n");
    npp_wake_pipe_t pipe;
    npp_wake_pipe_init(&pipe, NPP_STRATEGY_ON_DEMAND);
    
    /* 先初始化传感器模板 */
    npp_adaptive_init(&pipe, NPP_ADAPTIVE_TEMPLATE_SENSOR);
    npp_wake_pipe_register_profile(&pipe, 1001, 0.1f);
    
    /* 记录初始阈值 */
    float init_threshold = pipe.profile_routes[0].wake_threshold;
    
    /* 模拟误唤醒 */
    for (int i = 0; i < 3; i++) {
        npp_adaptive_report_wake(&pipe, 0);
    }
    
    /* 验证阈值调整 */
    npp_adaptive_stats_t stats;
    npp_adaptive_get_stats(&pipe, &stats);
    assert(stats.current_threshold > init_threshold);
    
    printf("✅ 自适应调整测试通过，阈值从%.3f调整为%.3f\n", init_threshold, stats.current_threshold);
}

/* 测试软件唤醒延迟 */
void test_software_wake_latency() {
    printf("测试软件唤醒延迟...\n");
    npp_wake_pipe_t pipe;
    npp_wake_pipe_init(&pipe, NPP_STRATEGY_ON_DEMAND);
    
    struct timeval start, end;
    gettimeofday(&start, NULL);
    
    /* 模拟唤醒流程 */
    npp_wake_pipe_trigger_wake(&pipe, NPP_WAKE_TYPE_DATA_CHANGE, 1001);
    
    gettimeofday(&end, NULL);
    long latency_us = (end.tv_sec - start.tv_sec) * 1000000 + (end.tv_usec - start.tv_usec);
    
    assert(latency_us < 20000); /* 小于20ms */
    printf("✅ 软件唤醒延迟测试通过，延迟: %ldus\n", latency_us);
}

int main() {
    printf("===== NPP SDK 单元测试 =====\n");
    
    test_multi_profile_wake();
    test_wake_backoff();
    test_multicast_wake();
    test_adaptive_adjust();
    test_software_wake_latency();
    
    printf("\n🎉 所有测试用例全部通过！\n");
    return 0;
}