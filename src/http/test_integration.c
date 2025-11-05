/**
 * OMNISIGHT HTTP Server Integration Test
 *
 * Tests HTTP server with full OMNISIGHT core integration.
 * Uses correct API calls for initialization and data access.
 *
 * Phase 4 - v0.7.0
 */

#include "http_server.h"
#include "../omnisight_core.h"
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <stdbool.h>

static volatile bool g_running = true;

static void signal_handler(int signum) {
    (void)signum;
    printf("\nShutting down...\n");
    g_running = false;
}

static void print_instructions(int port) {
    printf("\n========================================\n");
    printf("OMNISIGHT HTTP Server Integration Test\n");
    printf("========================================\n\n");
    printf("Server: http://localhost:%d\n\n", port);
    printf("Test endpoints:\n");
    printf("  curl http://localhost:%d/api/health\n", port);
    printf("  curl http://localhost:%d/api/status\n", port);
    printf("  curl http://localhost:%d/api/stats\n", port);
    printf("  curl http://localhost:%d/api/perception/status\n", port);
    printf("  curl http://localhost:%d/api/perception/detections\n", port);
    printf("  curl http://localhost:%d/api/timeline/predictions\n", port);
    printf("  curl http://localhost:%d/api/swarm/network\n", port);
    printf("\nPress Ctrl+C to stop\n\n");
}

int main(int argc, char* argv[]) {
    int port = 8081;  // Use 8081 to avoid conflicts

    if (argc > 1) {
        port = atoi(argv[1]);
        if (port <= 0 || port > 65535) {
            fprintf(stderr, "Invalid port: %s\n", argv[1]);
            return 1;
        }
    }

    signal(SIGINT, signal_handler);

    printf("Initializing OMNISIGHT core...\n");

    // Create configuration with correct field names
    OmnisightConfig config = {
        .camera_id = 1,
        .camera_name = "Test Camera",
        .location = "Local Test",
        .enable_perception = true,
        .enable_timeline = true,
        .enable_swarm = true,
        .auto_start = true,
        .update_interval_ms = 100,
        .verbose_logging = false
    };

    // Initialize core
    OmnisightCore* core = omnisight_init(&config);
    if (!core) {
        fprintf(stderr, "Failed to initialize OMNISIGHT core\n");
        return 1;
    }

    printf("Starting OMNISIGHT modules...\n");

    // Start modules (no event callback)
    if (!omnisight_start(core, NULL, NULL)) {
        fprintf(stderr, "Failed to start OMNISIGHT modules\n");
        omnisight_stop(core);
        free(core);
        return 1;
    }

    printf("Creating HTTP server...\n");

    // Create HTTP server
    HTTPServer* server = http_server_create(core, port);
    if (!server) {
        fprintf(stderr, "Failed to create HTTP server\n");
        omnisight_stop(core);
        free(core);
        return 1;
    }

    printf("Starting HTTP server...\n");

    // Start HTTP server
    if (!http_server_start(server)) {
        fprintf(stderr, "Failed to start HTTP server\n");
        http_server_destroy(server);
        omnisight_stop(core);
        free(core);
        return 1;
    }

    print_instructions(port);

    // Main event loop
    while (g_running && http_server_is_running(server)) {
        // Poll HTTP server
        http_server_poll(server, 100);

        // Process OMNISIGHT core
        omnisight_process(core);

        // Small sleep
        usleep(10000); // 10ms
    }

    printf("\nShutting down...\n");

    // Cleanup
    http_server_stop(server);
    http_server_destroy(server);
    omnisight_stop(core);
    free(core);

    printf("Shutdown complete\n");

    return 0;
}
