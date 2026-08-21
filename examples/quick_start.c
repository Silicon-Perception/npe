/*
 * Copyright (c) 2026 吴金辉 (Jinhui Wu). All rights reserved.
 * NPP SDK Quick Start Example
 *
 * This example demonstrates basic NPP session usage:
 * - Create a UDP session
 * - Connect to a remote peer
 * - Write data to a pipe
 * - Read data from a pipe
 * - Handle incoming data via callback
 */

#include <stdio.h>
#include <string.h>
#include <npp.h>

/* Callback for received pipe data */
static void on_pipe_data(uint32_t pipe_id, const uint8_t* data, uint32_t len, void* user_data) {
    (void)user_data;
    printf("Received on pipe %u: %.*s\n", pipe_id, (int)len, (char*)data);
}

int main(int argc, char* argv[]) {
    (void)argc; (void)argv;

    /* 1. Configure session */
    npp_session_config_t cfg = {
        .transport = NPP_TRANSPORT_UDP,
        .local_port = 8888,
        .remote_port = 9999,
        .remote_addr = "127.0.0.1"
    };

    /* 2. Create session */
    npp_session_t* session = NULL;
    npp_err_t err = npp_session_create(&session, &cfg);
    if (err != NPP_OK) {
        printf("Failed to create session: %d\n", err);
        return 1;
    }

    /* 3. Register callback for pipe #0 */
    err = npp_pipe_on_data(session, 0, on_pipe_data, NULL);
    if (err != NPP_OK) {
        printf("Failed to register callback: %d\n", err);
        npp_session_destroy(session);
        return 1;
    }

    /* 4. Connect */
    err = npp_session_connect(session);
    if (err != NPP_OK) {
        printf("Failed to connect: %d\n", err);
        npp_session_destroy(session);
        return 1;
    }

    printf("Session connected. Version: %s\n", npp_version_string());

    /* 5. Write data to pipe #0 (will auto-transmit if changed) */
    const char* msg = "Hello, NPP!";
    err = npp_pipe_write(session, 0, (const uint8_t*)msg, strlen(msg));
    if (err != NPP_OK) {
        printf("Failed to write: %d\n", err);
    }

    /* 6. Cleanup */
    npp_session_disconnect(session);
    npp_session_destroy(session);

    printf("Example completed.\n");
    return 0;
}
