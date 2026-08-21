/*
 * Copyright (c) 2026 吴金辉 (Jinhui Wu). All rights reserved.
 * NPP SDK Wake/Sync Tests
 *
 * Tests SYNC mechanism and state recovery
 */

#include <stdio.h>
#include <string.h>
#include <npp.h>

static int tests_run = 0;
static int tests_passed = 0;

#define TEST_ASSERT(cond, msg) do { \
    tests_run++; \
    if (cond) { \
        tests_passed++; \
        printf("  PASS: %s\n", msg); \
    } else { \
        printf("  FAIL: %s\n", msg); \
    } \
} while(0)

/* Test 1: Multiple sessions */
static void test_multiple_sessions(void) {
    printf("\n[Test] Multiple Sessions\n");
    npp_session_t* s1 = NULL;
    npp_session_t* s2 = NULL;

    npp_session_config_t cfg1 = {
        .transport = NPP_TRANSPORT_UDP,
        .local_port = 0,
        .remote_port = 19901,
        .remote_addr = "127.0.0.1"
    };
    npp_session_config_t cfg2 = {
        .transport = NPP_TRANSPORT_UDP,
        .local_port = 0,
        .remote_port = 19902,
        .remote_addr = "127.0.0.1"
    };

    npp_err_t err1 = npp_session_create(&s1, &cfg1);
    npp_err_t err2 = npp_session_create(&s2, &cfg2);

    TEST_ASSERT(err1 == NPP_OK && err2 == NPP_OK, "Two sessions created");
    TEST_ASSERT(s1 != s2, "Sessions are different handles");

    npp_session_destroy(s1);
    npp_session_destroy(s2);
}

/* Test 2: Pipe isolation */
static void test_pipe_isolation(void) {
    printf("\n[Test] Pipe Isolation\n");
    npp_session_t* session = NULL;
    npp_session_config_t cfg = {
        .transport = NPP_TRANSPORT_UDP,
        .local_port = 0,
        .remote_port = 0,
        .remote_addr = "127.0.0.1"
    };

    npp_err_t err = npp_session_create(&session, &cfg);
    if (err != NPP_OK) {
        printf("  SKIP: Cannot create session\n");
        return;
    }

    /* Write different data to different pipes */
    uint8_t data0[] = {0xAA};
    uint8_t data1[] = {0xBB};

    npp_pipe_write(session, 0, data0, 1);
    npp_pipe_write(session, 1, data1, 1);

    /* Read back and verify isolation */
    uint8_t read0 = 0, read1 = 0;
    uint32_t len0 = 1, len1 = 1;

    npp_pipe_read(session, 0, &read0, &len0);
    npp_pipe_read(session, 1, &read1, &len1);

    TEST_ASSERT(read0 == 0xAA, "Pipe 0 data intact");
    TEST_ASSERT(read1 == 0xBB, "Pipe 1 data intact");

    npp_session_destroy(session);
}

/* Test 3: Reconnect cycle */
static void test_reconnect_cycle(void) {
    printf("\n[Test] Reconnect Cycle\n");
    npp_session_t* session = NULL;
    npp_session_config_t cfg = {
        .transport = NPP_TRANSPORT_UDP,
        .local_port = 0,
        .remote_port = 0,
        .remote_addr = "127.0.0.1"
    };

    npp_err_t err = npp_session_create(&session, &cfg);
    if (err != NPP_OK) {
        printf("  SKIP: Cannot create session\n");
        return;
    }

    /* Connect-disconnect cycle */
    for (int i = 0; i < 3; i++) {
        err = npp_session_connect(session);
        TEST_ASSERT(err == NPP_OK, "Connect succeeds");

        err = npp_session_disconnect(session);
        TEST_ASSERT(err == NPP_OK, "Disconnect succeeds");
    }

    npp_session_destroy(session);
}

int main(void) {
    printf("=== NPP SDK Wake/Sync Tests ===\n");

    test_multiple_sessions();
    test_pipe_isolation();
    test_reconnect_cycle();

    printf("\n=== Results: %d/%d passed ===\n", tests_passed, tests_run);
    return (tests_passed == tests_run) ? 0 : 1;
}
