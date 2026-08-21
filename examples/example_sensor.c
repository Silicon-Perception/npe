/*
 * Copyright (c) 2026 吴金辉 (Jinhui Wu). All rights reserved.
 * NPP SDK Sensor Simulation Example
 *
 * Simulates a temperature/humidity sensor that:
 * - Sends data periodically via NPP pipes
 * - Demonstrates pipe read/write
 * - Shows session lifecycle
 */

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <npp.h>

/* Simulated sensor data */
typedef struct {
    float temperature;
    float humidity;
} sensor_data_t;

/* Callback for receiving commands on pipe #10 */
static void on_command_received(uint32_t pipe_id, const uint8_t* data, uint32_t len, void* user_data) {
    (void)user_data;
    printf("Command on pipe %u: %.*s\n", pipe_id, (int)len, (char*)data);
}

int main(int argc, char* argv[]) {
    (void)argc; (void)argv;

    printf("=== NPP Sensor Simulation ===\n");

    /* 1. Configure session */
    npp_session_config_t cfg = {
        .transport = NPP_TRANSPORT_UDP,
        .local_port = 8889,
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

    /* 3. Register callback for command pipe (#10) */
    npp_pipe_on_data(session, 10, on_command_received, NULL);

    /* 4. Connect */
    npp_session_connect(session);
    printf("Connected (port %d)\n", cfg.local_port);

    /* 5. Simulate sensor readings on pipes 0-1 */
    srand((unsigned)time(NULL));
    sensor_data_t sensors[2];

    for (int i = 0; i < 5; i++) {
        /* Simulate temperature reading on pipe 0 */
        sensors[0].temperature = 20.0f + (rand() % 150) / 10.0f;
        npp_pipe_write(session, 0, (const uint8_t*)&sensors[0].temperature, sizeof(float));

        /* Simulate humidity reading on pipe 1 */
        sensors[0].humidity = 40.0f + (rand() % 400) / 10.0f;
        npp_pipe_write(session, 1, (const uint8_t*)&sensors[0].humidity, sizeof(float));

        printf("Iteration %d: temp=%.1f humidity=%.1f\n",
               i + 1, sensors[0].temperature, sensors[0].humidity);
    }

    /* 6. Cleanup */
    npp_session_disconnect(session);
    npp_session_destroy(session);

    printf("=== Example completed ===\n");
    return 0;
}
