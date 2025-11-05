/**
 * OMNISIGHT HTTP Server Test
 *
 * Simple test program to validate HTTP server functionality locally.
 * Tests basic endpoints and verifies JSON responses.
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

// Global flag for graceful shutdown
static volatile bool g_running = true;

/**
 * Signal handler for SIGINT (Ctrl+C)
 */
static void signal_handler(int signum) {
    (void)signum;
    printf("\nReceived signal, shutting down...\n");
    g_running = false;
}

/**
 * Print test instructions
 */
static void print_instructions(int port) {
    printf("\n");
    printf("========================================\n");
    printf("OMNISIGHT HTTP Server Test\n");
    printf("========================================\n");
    printf("\n");
    printf("Server started on http://localhost:%d\n", port);
    printf("\n");
    printf("Test with curl commands:\n");
    printf("\n");
    printf("  Health Check:\n");
    printf("    curl http://localhost:%d/api/health\n", port);
    printf("\n");
    printf("  System Status:\n");
    printf("    curl http://localhost:%d/api/status\n", port);
    printf("\n");
    printf("  Statistics:\n");
    printf("    curl http://localhost:%d/api/stats\n", port);
    printf("\n");
    printf("  Perception Status:\n");
    printf("    curl http://localhost:%d/api/perception/status\n", port);
    printf("\n");
    printf("  Perception Detections:\n");
    printf("    curl http://localhost:%d/api/perception/detections\n", port);
    printf("\n");
    printf("  Timeline Predictions:\n");
    printf("    curl http://localhost:%d/api/timeline/predictions\n", port);
    printf("\n");
    printf("  Swarm Network:\n");
    printf("    curl http://localhost:%d/api/swarm/network\n", port);
    printf("\n");
    printf("  Configuration:\n");
    printf("    curl http://localhost:%d/api/config\n", port);
    printf("\n");
    printf("Press Ctrl+C to stop server\n");
    printf("\n");
}

int main(int argc, char* argv[]) {
    int port = 8080;

    // Parse command line arguments
    if (argc > 1) {
        port = atoi(argv[1]);
        if (port <= 0 || port > 65535) {
            fprintf(stderr, "Invalid port number: %s\n", argv[1]);
            fprintf(stderr, "Usage: %s [port]\n", argv[0]);
            return 1;
        }
    }

    // Set up signal handler
    signal(SIGINT, signal_handler);

    printf("Initializing OMNISIGHT core...\n");

    // Create OMNISIGHT core instance
    OmnisightCore* core = omnisight_init();
    if (!core) {
        fprintf(stderr, "Failed to initialize OMNISIGHT core\n");
        return 1;
    }

    printf("Starting OMNISIGHT core...\n");

    // Start core modules
    if (!omnisight_start(core)) {
        fprintf(stderr, "Failed to start OMNISIGHT core\n");
        omnisight_shutdown(core);
        return 1;
    }

    printf("Creating HTTP server...\n");

    // Create HTTP server
    HTTPServer* server = http_server_create(core, port);
    if (!server) {
        fprintf(stderr, "Failed to create HTTP server\n");
        omnisight_stop(core);
        omnisight_shutdown(core);
        return 1;
    }

    printf("Starting HTTP server on port %d...\n", port);

    // Start HTTP server
    if (!http_server_start(server)) {
        fprintf(stderr, "Failed to start HTTP server\n");
        http_server_destroy(server);
        omnisight_stop(core);
        omnisight_shutdown(core);
        return 1;
    }

    print_instructions(port);

    // Main event loop
    while (g_running && http_server_is_running(server)) {
        // Poll HTTP server (100ms timeout)
        http_server_poll(server, 100);

        // Update OMNISIGHT core
        omnisight_update(core);

        // Small sleep to prevent CPU spinning
        usleep(10000); // 10ms
    }

    printf("\nShutting down...\n");

    // Cleanup
    http_server_stop(server);
    http_server_destroy(server);
    omnisight_stop(core);
    omnisight_shutdown(core);

    printf("Shutdown complete\n");

    return 0;
}
