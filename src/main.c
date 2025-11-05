/**
 * OMNISIGHT - Precognitive Security System
 * Main application entry point
 *
 * Copyright (c) 2025 OMNISIGHT
 */

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>

#include "omnisight_core.h"
#include "ipc/json_export.h"

// Global state
static volatile bool running = true;
static OmnisightCore* g_core = NULL;

/**
 * Signal handler for graceful shutdown
 */
static void signal_handler(int signum) {
    (void)signum;
    printf("\n\nReceived shutdown signal, cleaning up...\n");
    running = false;
}

/**
 * Print application banner
 */
static void print_banner(void) {
    printf("\n");
    printf("╔═══════════════════════════════════════════════════════════════╗\n");
    printf("║                                                                ║\n");
    printf("║     OMNISIGHT - Precognitive Security System v0.1.0           ║\n");
    printf("║                                                                ║\n");
    printf("║     \"See Everything. Understand Everything.\"                  ║\n");
    printf("║     \"Prevent Everything.\"                                     ║\n");
    printf("║                                                                ║\n");
    printf("╚═══════════════════════════════════════════════════════════════╝\n");
    printf("\n");
}

/**
 * Event callback for OMNISIGHT notifications
 */
static void event_callback(const OmnisightEvent* event, void* user_data) {
    (void)user_data;

    const char* type_str = "UNKNOWN";
    switch (event->type) {
        case OMNISIGHT_EVENT_STARTED:
            type_str = "STARTED";
            break;
        case OMNISIGHT_EVENT_STOPPED:
            type_str = "STOPPED";
            break;
        case OMNISIGHT_EVENT_OBJECT_DETECTED:
            type_str = "OBJECT_DETECTED";
            break;
        case OMNISIGHT_EVENT_OBJECT_TRACKED:
            type_str = "OBJECT_TRACKED";
            break;
        case OMNISIGHT_EVENT_BEHAVIOR_DETECTED:
            type_str = "BEHAVIOR_DETECTED";
            break;
        case OMNISIGHT_EVENT_TIMELINE_UPDATED:
            type_str = "TIMELINE_UPDATED";
            break;
        case OMNISIGHT_EVENT_INTERVENTION_RECOMMENDED:
            type_str = "⚠️  INTERVENTION";
            break;
        case OMNISIGHT_EVENT_SWARM_TRACK_RECEIVED:
            type_str = "SWARM_TRACK";
            break;
        case OMNISIGHT_EVENT_SWARM_EVENT_RECEIVED:
            type_str = "SWARM_EVENT";
            break;
        case OMNISIGHT_EVENT_CONSENSUS_REACHED:
            type_str = "CONSENSUS";
            break;
        case OMNISIGHT_EVENT_ERROR:
            type_str = "ERROR";
            break;
    }

    printf("[EVENT:%s] %s\n", type_str, event->message);
}

/**
 * Print system statistics
 */
static void print_stats(OmnisightCore* core) {
    OmnisightStats stats;
    omnisight_get_stats(core, &stats);

    printf("\n");
    printf("═══════════════════════════════════════════════════════════════\n");
    printf("                    OMNISIGHT STATUS                           \n");
    printf("═══════════════════════════════════════════════════════════════\n");
    printf("\n");

    printf("System:\n");
    printf("  Status:          %s\n", stats.is_running ? "RUNNING" : "STOPPED");
    printf("  Uptime:          %lu seconds\n", stats.uptime_ms / 1000);
    printf("  Frames:          %lu\n", stats.frames_processed);
    printf("\n");

    printf("Perception:\n");
    printf("  Tracked Objects: %u\n", stats.perception_stats.tracked_objects);
    printf("  Average FPS:     %.1f\n", stats.perception_stats.avg_fps);
    printf("  Inference Time:  %.1f ms\n", stats.perception_stats.avg_inference_ms);
    printf("  Dropped Frames:  %u\n", stats.perception_stats.dropped_frames);
    printf("\n");

    printf("Timeline Threading™:\n");
    printf("  Active Timelines:     %u\n", stats.timeline_stats.active_timelines);
    printf("  Events Predicted:     %lu\n", stats.timeline_stats.total_events_predicted);
    printf("  Interventions:        %lu\n", stats.timeline_stats.total_interventions);
    printf("  Prediction Time:      %.1f ms\n", stats.timeline_stats.avg_prediction_ms);
    printf("\n");

    if (stats.swarm_stats.num_neighbors > 0) {
        printf("Swarm Intelligence:\n");
        printf("  Neighbors:       %u\n", stats.swarm_stats.num_neighbors);
        printf("  Tracks Shared:   %u\n", stats.swarm_stats.tracks_shared);
        printf("  Events Shared:   %u\n", stats.swarm_stats.events_shared);
        printf("  Network Health:  %.0f%%\n", stats.swarm_stats.network_health * 100);
        printf("\n");
    }

    printf("═══════════════════════════════════════════════════════════════\n");
    printf("\n");
}

/**
 * Main application loop
 */
static void run_main_loop(OmnisightCore* core) {
    printf("[MAIN] OMNISIGHT is now operational\n");
    printf("[MAIN] Press Ctrl+C to stop | Type 'stats' for status\n");
    printf("[MAIN] Exporting JSON to /tmp/omnisight_*.json for Flask API\n\n");

    int iteration = 0;
    int stats_counter = 0;
    int json_export_counter = 0;

    // Initial status export
    json_export_status("running", 0);

    while (running) {
        // Process OMNISIGHT
        if (!omnisight_process(core)) {
            printf("[MAIN] Warning: Processing failed\n");
        }

        // Export JSON every second (10 iterations at 100ms)
        json_export_counter++;
        if (json_export_counter >= 10) {
            // Get current stats
            OmnisightStats stats;
            omnisight_get_stats(core, &stats);

            // Export stats
            json_export_stats(&stats);

            // Export status with uptime
            json_export_status("running", stats.uptime_ms);

            // Get and export current data
            TrackedObject tracks[50];
            uint32_t num_tracks;
            num_tracks = omnisight_get_tracked_objects(core, tracks, 50);
            if (num_tracks > 0) {
                json_export_tracks(tracks, num_tracks);
            }

            Timeline timelines[5];
            uint32_t num_timelines;
            num_timelines = omnisight_get_timelines(core, timelines, 5);
            if (num_timelines > 0) {
                json_export_timelines(timelines, num_timelines);
            }

            json_export_counter = 0;
        }

        // Check for interventions every 5 seconds
        if (iteration % 50 == 0) {
            InterventionPoint intervention;
            if (omnisight_get_intervention(core, &intervention)) {
                printf("\n");
                printf("⚠️  INTERVENTION RECOMMENDED!\n");
                printf("   Type: %s\n", intervention.recommendation);
                printf("   Effectiveness: %.0f%%\n", intervention.effectiveness * 100);
                printf("   Time: %lu ms\n", intervention.timestamp_ms);
                printf("\n");
            }
        }

        // Print stats every 30 seconds
        stats_counter++;
        if (stats_counter >= 300) {  // 30 seconds at 100ms intervals
            print_stats(core);
            stats_counter = 0;
        }

        iteration++;
        usleep(100000);  // 100ms
    }

    // Final status export
    json_export_status("stopped", 0);

    printf("\n[MAIN] Main loop terminated\n");
}

/**
 * Main entry point
 */
int main(int argc, char *argv[]) {
    int ret = EXIT_SUCCESS;

    // Print banner
    print_banner();

    // Parse command line arguments
    bool demo_mode = false;
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--demo") == 0) {
            demo_mode = true;
            printf("[INFO] Running in demo mode\n");
        } else if (strcmp(argv[i], "--help") == 0) {
            printf("Usage: %s [OPTIONS]\n\n", argv[0]);
            printf("Options:\n");
            printf("  --demo     Run in demo/simulation mode\n");
            printf("  --help     Show this help message\n");
            printf("\n");
            return EXIT_SUCCESS;
        }
    }

    printf("\n");

    // Set up signal handlers
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    // Get default configuration
    OmnisightConfig config;
    omnisight_get_default_config(&config);

    // Modify config for demo mode if needed
    if (demo_mode) {
        config.enable_swarm = false;  // Disable swarm in demo mode
        printf("[INFO] Swarm Intelligence disabled in demo mode\n");
    }

    // Initialize OMNISIGHT
    g_core = omnisight_init(&config);
    if (!g_core) {
        fprintf(stderr, "[ERROR] Failed to initialize OMNISIGHT\n");
        return EXIT_FAILURE;
    }

    // Start system
    if (!omnisight_start(g_core, event_callback, NULL)) {
        fprintf(stderr, "[ERROR] Failed to start OMNISIGHT\n");
        omnisight_destroy(g_core);
        return EXIT_FAILURE;
    }

    // Run main application loop
    run_main_loop(g_core);

    // Stop and cleanup
    omnisight_stop(g_core);
    omnisight_destroy(g_core);

    printf("\n[EXIT] OMNISIGHT terminated successfully\n\n");
    return ret;
}
