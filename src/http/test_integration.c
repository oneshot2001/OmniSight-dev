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

    // Create configuration with correct field names and module configs
    OmnisightConfig config = {
        .camera_id = 1,
        .camera_name = "Test Camera",
        .location = "Local Test",

        // Perception module config
        .perception = {
            .frame_width = 1920,
            .frame_height = 1080,
            .target_fps = 25,
            .model_path = "/opt/models/omnisight.onnx",
            .use_dlpu = false,
            .inference_threads = 2,
            .detection_threshold = 0.5,
            .tracking_threshold = 0.3,
            .max_tracked_objects = 50,
            .loitering_threshold_ms = 30000,
            .running_velocity_threshold = 3.0,
            .async_inference = true,
            .buffer_pool_size = 4
        },

        // Timeline module config
        .timeline = {
            .prediction_horizon_ms = 300000,  // 5 minutes
            .time_step_ms = 1000,             // 1 second
            .max_timelines = 5,
            .branch_threshold = 0.3,
            .merge_threshold = 0.8,
            .event_threshold = 0.5,
            .enable_intervention_search = true,
            .max_iterations = 100,
            .use_gpu = false
        },

        // Swarm module config
        .swarm = {
            .camera_id = 1,
            .role = 1,  // SWARM_ROLE_INTERNAL
            .geometry = {
                .x = 0.0, .y = 0.0, .z = 3.0,
                .pan = 0.0, .tilt = 0.0, .zoom = 1.0,
                .fov_horizontal = 90.0,
                .fov_vertical = 60.0,
                .coverage_radius = 20.0
            },
            .privacy = 1,  // PRIVACY_FEATURES
            .mqtt_broker = "localhost",
            .mqtt_port = 1883,
            .mqtt_client_id = "omnisight_test",
            .mqtt_username = NULL,
            .mqtt_password = NULL,
            .heartbeat_interval_ms = 5000,
            .track_share_interval_ms = 1000,
            .consensus_timeout_ms = 2000,
            .handoff_lookahead_ms = 3000,
            .federated_learning_enabled = false,
            .model_update_interval_ms = 60000,
            .min_improvement_threshold = 0.01
        },

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

    // Create HTTP server with local web root for testing
    const char* web_root = "build-integration-test/html";
    HTTPServer* server = http_server_create(core, port, web_root);
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
