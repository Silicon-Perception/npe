/*
 * Copyright (c) 2026 吴金辉 (Jinhui Wu). All rights reserved.
 * NPP SDK Basic Tests
 *
 * Tests public API from npp.h:
 * - Session create/destroy
 * - Pipe write/read
 * - Callback registration
 * - Version string
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

/* Test 1: Version string */
static void test_version(void) {
    printf("\n[Test] Version\n");
    const char* ver = npp_version_string();
    TEST_ASSERT(ver != NULL, "Version string not NULL");
    TEST_ASSERT(strstr(ver, "2.") != NULL, "Version starts with 2.");
}

/* Test 2: Session create/destroy */
static void test_session_lifecycle(void) {
    printf("\n[Test] Session Lifecycle\n");
    npp_session_t* session = NULL;
    npp_session_config_t cfg = {
        .transport = NPP_TRANSPORT_UDP,
        .local_port = 0,  /* Let OS assign */
        .remote_port = 0,
        .remote_addr = "127.0.0.1"
    };

    npp_err_t err = npp_session_create(&session, &cfg);
    TEST_ASSERT(err == NPP_OK, "Session create returns OK");
    TEST_ASSERT(session != NULL, "Session handle not NULL");

    err = npp_session_destroy(session);
    TEST_ASSERT(err == NPP_OK, "Session destroy returns OK");
}

/* Test 3: Pipe write/read */
static void test_pipe_write_read(void) {
    printf("\n[Test] Pipe Write/Read\n");
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

    /* Write to pipe 0 */
    uint8_t write_data[] = {1, 2, 3, 4, 5};
    err = npp_pipe_write(session, 0, write_data, sizeof(write_data));
    TEST_ASSERT(err == NPP_OK, "Pipe write returns OK");

    /* Read from pipe 0 */
    uint8_t read_data[16] = {0};
    uint32_t read_len = sizeof(read_data);
    err = npp_pipe_read(session, 0, read_data, &read_len);
    TEST_ASSERT(err == NPP_OK, "Pipe read returns OK");
    TEST_ASSERT(read_len == sizeof(write_data), "Read length matches");
    TEST_ASSERT(memcmp(write_data, read_data, read_len) == 0, "Read data matches");

    npp_session_destroy(session);
}

/* Test 4: Callback registration */
static void test_callback_registration(void) {
    printf("\n[Test] Callback Registration\n");
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

    err = npp_pipe_on_data(session, 0, NULL, NULL);
    TEST_ASSERT(err == NPP_OK, "Callback registration returns OK");

    npp_session_destroy(session);
}

/* Test 5: Error handling */
static void test_error_handling(void) {
    printf("\n[Test] Error Handling\n");

    /* NULL session should fail */
    uint8_t data[] = {1};
    npp_err_t err = npp_pipe_write(NULL, 0, data, 1);
    TEST_ASSERT(err == NPP_ERR_INVALID_PARAM, "NULL session returns INVALID_PARAM");

    /* NULL config should fail */
    npp_session_t* session = NULL;
    err = npp_session_create(&session, NULL);
    TEST_ASSERT(err == NPP_ERR_INVALID_PARAM, "NULL config returns INVALID_PARAM");
}

int main(void) {
    printf("=== NPP SDK Basic Tests ===\n");

    test_version();
    test_session_lifecycle();
    test_pipe_write_read();
    test_callback_registration();
    test_error_handling();

    printf("\n=== Results: %d/%d passed ===\n", tests_passed, tests_run);
    return (tests_passed == tests_run) ? 0 : 1;
}
