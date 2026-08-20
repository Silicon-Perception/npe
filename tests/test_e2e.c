#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include "core/wake_pipe.h"
#include "multicast/multicast.h"
#include "transport/udp.h"

#define TEST_PORT 9090
#define TEST_GROUP_ID 2001
#define TEST_PROFILE_ID 1001

/* 接收端进程 */
int receiver_process() {
    printf("📶 接收端启动...\n");
    
    /* 初始化UDP */
    npp_udp_handle_t udp_handle;
    npp_udp_config_t udp_config = {
        .local_ip = "127.0.0.1",
        .local_port = TEST_PORT,
        .remote_ip = "127.0.0.1",
        .remote_port = TEST_PORT + 1,
        .timeout_ms = 5000
    };
    if (npp_udp_init(&udp_handle, udp_config) != NPP_OK) {
        printf("❌ 接收端UDP初始化失败\n");
        return 1;
    }
    
    /* 初始化唤醒管道 */
    npp_wake_pipe_t wake_pipe;
    npp_wake_pipe_init(&wake_pipe, NPP_STRATEGY_ON_DEMAND);
    npp_wake_pipe_register_profile(&wake_pipe, TEST_PROFILE_ID, 0.1f);
    
    /* 创建多播组并加入 */
    npp_multicast_create_group(&wake_pipe, TEST_GROUP_ID, 0x12345678);
    npp_multicast_join_group(&wake_pipe, TEST_GROUP_ID, getpid());
    
    /* 接收数据 */
    uint8_t buf[1024];
    size_t recv_len;
    int recv_count = 0;
    
    printf("📶 接收端等待数据...\n");
    for (int i = 0; i < 10; i++) {
        if (npp_udp_recv(&udp_handle, buf, sizeof(buf), &recv_len) == NPP_OK && recv_len > 0) {
            recv_count++;
            printf("📶 收到数据包，长度: %zu字节\n", recv_len);
            
            /* 处理多播唤醒帧 */
            if (recv_len >= sizeof(npp_multicast_wake_frame_t)) {
                npp_multicast_handle_frame(&wake_pipe, (npp_multicast_wake_frame_t*)buf, recv_len);
            }
        }
        usleep(100000);
    }
    
    printf("📶 接收端共收到%d个数据包\n", recv_count);
    npp_udp_close(&udp_handle);
    return recv_count > 0 ? 0 : 1;
}

/* 发送端进程 */
int sender_process() {
    printf("📤 发送端启动...\n");
    sleep(1); /* 等待接收端就绪 */
    
    /* 初始化UDP */
    npp_udp_handle_t udp_handle;
    npp_udp_config_t udp_config = {
        .local_ip = "127.0.0.1",
        .local_port = TEST_PORT + 1,
        .remote_ip = "127.0.0.1",
        .remote_port = TEST_PORT,
        .timeout_ms = 5000
    };
    if (npp_udp_init(&udp_handle, udp_config) != NPP_OK) {
        printf("❌ 发送端UDP初始化失败\n");
        return 1;
    }
    
    /* 初始化唤醒管道 */
    npp_wake_pipe_t wake_pipe;
    npp_wake_pipe_init(&wake_pipe, NPP_STRATEGY_ON_DEMAND);
    npp_wake_pipe_register_profile(&wake_pipe, TEST_PROFILE_ID, 0.1f);
    
    /* 创建多播组并发送唤醒 */
    npp_multicast_create_group(&wake_pipe, TEST_GROUP_ID, 0x12345678);
    npp_multicast_send_wake(&wake_pipe, TEST_GROUP_ID);
    
    /* 发送测试数据 */
    uint8_t test_data[] = "NPP E2E Test Data";
    for (int i = 0; i < 5; i++) {
        npp_udp_send(&udp_handle, test_data, sizeof(test_data));
        printf("📤 发送数据包%d\n", i + 1);
        usleep(200000);
    }
    
    npp_udp_close(&udp_handle);
    return 0;
}

int main() {
    printf("===== NPP SDK 端到端测试 =====\n");
    
    pid_t pid = fork();
    if (pid == 0) {
        /* 子进程：接收端 */
        exit(receiver_process());
    } else if (pid > 0) {
        /* 父进程：发送端 */
        int sender_ret = sender_process();
        
        /* 等待接收端结束 */
        int status;
        waitpid(pid, &status, 0);
        int receiver_ret = WEXITSTATUS(status);
        
        if (sender_ret == 0 && receiver_ret == 0) {
            printf("\n🎉 端到端测试通过！UDP传输+多播唤醒流程正常\n");
            return 0;
        } else {
            printf("\n❌ 端到端测试失败，发送端返回:%d，接收端返回:%d\n", sender_ret, receiver_ret);
            return 1;
        }
    } else {
        printf("❌ fork失败\n");
        return 1;
    }
}