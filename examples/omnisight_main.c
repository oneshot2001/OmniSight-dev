/**
 * @file omnisight_main.c
 * @brief Complete OMNISIGHT application example
 *
 * Demonstrates full integration of all components:
 * - Perception Engine (VDO + Larod + Tracker + Behavior)
 * - Timeline Threading (Trajectory + Event + Intervention)
 * - Swarm Intelligence (MQTT + Federated Learning)
 */

#include "../src/perception/perception.h"
#include "../src/perception/vdo_capture.h"
#include "../src/perception/larod_inference.h"
#include "../src/timeline/timeline.h"
#include "../src/swarm/swarm.h"
#include "../src/swarm/federated_learning.h"

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <time.h>

// ============================================================================
// Global State
// ============================================================================

typedef struct {
    // Perception
    PerceptionEngine* perception;
    VdoCapture* vdo;
    LarodInference* larod;

    // Timeline
    TimelineEngine* timeline;

    // Swarm
    SwarmNode* swarm;
    FederatedLearner* fed_learner;

    // Runtime state
    bool running;
    uint64_t frame_count;
    uint64_t event_count;
    uint64_t intervention_count;
} OmnisightApp;

static OmnisightApp* g_app = NULL;

// ============================================================================
// Signal Handler
// ============================================================================

void signal_handler(int signum) {
    printf("\n[OMNISIGHT] Received signal %d, shutting down...\n", signum);
    if (g_app) {
        g_app->running = false;
    }
}

// ============================================================================
// Callbacks
// ============================================================================

/**
 * Perception callback - called when objects are detected
 */
void on_objects_detected(
    const DetectedObject* objects,
    uint32_t num_objects,
    uint64_t timestamp_ms,
    void* user_data
) {
    OmnisightApp* app = (OmnisightApp*)user_data;
    app->frame_count++;

    if (num_objects == 0) {
        return;  // No objects detected
    }

    printf("[Frame %lu] Detected %u objects\n", app->frame_count, num_objects);

    // Get tracked objects with behaviors
    TrackedObject tracks[100];
    uint32_t num_tracks = perception_get_tracked_objects(
        app->perception,
        tracks,
        100
    );

    if (num_tracks == 0) {
        return;
    }

    // Print tracked objects with threat scores
    for (uint32_t i = 0; i < num_tracks; i++) {
        if (tracks[i].threat_score > 0.5f) {
            printf("  Track %u: threat=%.2f behaviors=0x%x\n",
                   tracks[i].track_id,
                   tracks[i].threat_score,
                   tracks[i].behaviors);
        }
    }

    // Share tracks with swarm network
    if (app->swarm) {
        swarm_update_tracks(app->swarm, tracks, num_tracks, timestamp_ms);

        // Get tracks from neighbors for re-identification
        SharedTrack neighbor_tracks[50];
        uint32_t num_neighbor = swarm_get_neighbor_tracks(
            app->swarm,
            neighbor_tracks,
            50
        );

        if (num_neighbor > 0) {
            printf("  Received %u tracks from neighbors\n", num_neighbor);
        }
    }

    // Update Timeline Threading engine
    if (app->timeline) {
        Timeline* timelines[10];
        uint32_t num_timelines = timeline_update(
            app->timeline,
            tracks,
            num_tracks,
            timestamp_ms,
            timelines
        );

        if (num_timelines > 0) {
            // Get predicted events from most likely timeline
            PredictedEvent events[50];
            uint32_t num_events = timeline_get_events(
                timelines[0],
                events,
                50
            );

            for (uint32_t i = 0; i < num_events; i++) {
                app->event_count++;

                printf("  Event predicted: type=%d severity=%d prob=%.2f in %lums\n",
                       events[i].type,
                       events[i].severity,
                       events[i].probability,
                       events[i].timestamp_ms - timestamp_ms);

                // High severity events: request consensus from swarm
                if (events[i].severity >= SEVERITY_HIGH && app->swarm) {
                    swarm_share_event(app->swarm, &events[i], timelines[0]);

                    // In production, would wait for consensus responses
                    // For demo, assume 75% consensus
                    float consensus = 0.75f;

                    if (consensus > 0.7f) {
                        printf("    ✓ Event confirmed by network (%.0f%% consensus)\n",
                               consensus * 100);

                        // Get intervention recommendation
                        InterventionPoint intervention;
                        if (timeline_get_best_intervention(app->timeline, &intervention)) {
                            app->intervention_count++;

                            printf("    → Intervention: type=%d urgency=%d in %lums\n",
                                   intervention.intervention,
                                   intervention.urgency,
                                   intervention.time_to_act_ms);

                            // Execute intervention
                            execute_intervention(app, &intervention);
                        }
                    }
                }
            }
        }
    }
}

/**
 * Execute intervention action
 */
void execute_intervention(OmnisightApp* app, const InterventionPoint* intervention) {
    switch (intervention->intervention) {
        case INTERVENTION_ALERT_SECURITY:
            printf("    ⚠️  ALERTING SECURITY PERSONNEL\n");
            // In production: Send alert to security system
            break;

        case INTERVENTION_ACTIVATE_SPEAKER:
            printf("    🔊 ACTIVATING SPEAKER WARNING\n");
            // In production: Activate camera speaker
            break;

        case INTERVENTION_DISPATCH_PERSONNEL:
            printf("    🚨 DISPATCHING SECURITY TO LOCATION\n");
            // In production: Dispatch security via API
            break;

        case INTERVENTION_LOCK_DOORS:
            printf("    🔒 LOCKING DOORS\n");
            // In production: Integrate with access control system
            break;

        case INTERVENTION_CALL_EMERGENCY:
            printf("    🚑 CALLING EMERGENCY SERVICES\n");
            // In production: Automated emergency call
            break;

        default:
            printf("    📝 LOGGING EVENT\n");
            break;
    }
}

// ============================================================================
// Initialization
// ============================================================================

OmnisightApp* omnisight_init(uint32_t camera_id) {
    OmnisightApp* app = calloc(1, sizeof(OmnisightApp));
    if (!app) {
        fprintf(stderr, "[OMNISIGHT] Failed to allocate app\n");
        return NULL;
    }

    printf("[OMNISIGHT] Initializing camera %u...\n", camera_id);

    // Initialize VDO capture
    VdoCaptureConfig vdo_config = {
        .width = 1920,
        .height = 1080,
        .fps = 30,
        .format = VDO_FORMAT_YUV420,
        .buffer_count = 4
    };

    app->vdo = vdo_capture_init(&vdo_config);
    if (!app->vdo) {
        fprintf(stderr, "[OMNISIGHT] Failed to initialize VDO\n");
        goto error;
    }

    // Initialize Larod inference
    LarodInferenceConfig larod_config = {
        .model_path = "models/detection/efficientnet_b4_detection.tflite",
        .use_dlpu = true,
        .num_threads = 2,
        .input_width = 416,
        .input_height = 416,
        .input_channels = 3,
        .quantized = true
    };

    app->larod = larod_inference_init(&larod_config);
    if (!app->larod) {
        fprintf(stderr, "[OMNISIGHT] Failed to initialize Larod\n");
        goto error;
    }

    // Initialize Perception Engine
    PerceptionConfig perception_config = {
        .max_tracks = 100,
        .iou_threshold = 0.3f,
        .confidence_threshold = 0.5f,
        .max_age = 30,
        .callback = on_objects_detected,
        .user_data = app
    };

    app->perception = perception_init(&perception_config);
    if (!app->perception) {
        fprintf(stderr, "[OMNISIGHT] Failed to initialize perception\n");
        goto error;
    }

    // Initialize Timeline Threading
    SceneContext scene = {
        .num_zones = 1,
        .zones[0] = {
            .x = 500, .y = 300, .radius = 100,
            .protected_event = EVENT_TYPE_TRESPASSING,
            .sensitivity = 0.9f
        },
        .time_of_day_risk = 1.0f,
        .day_of_week_risk = 1.0f,
        .num_incidents = 0
    };

    TimelineConfig timeline_config = {
        .prediction_horizon_s = 60.0f,
        .update_interval_ms = 100,
        .num_timelines = 3,
        .branching_enabled = true,
        .scene_context = &scene
    };

    app->timeline = timeline_init(&timeline_config);
    if (!app->timeline) {
        fprintf(stderr, "[OMNISIGHT] Failed to initialize timeline\n");
        goto error;
    }

    // Initialize Swarm Intelligence
    SwarmConfig swarm_config = {
        .camera_id = camera_id,
        .role = SWARM_ROLE_INTERNAL,
        .geometry = {
            .x = 10.0f, .y = 5.0f, .z = 3.0f,
            .pan = 0.0f, .tilt = -20.0f, .zoom = 1.0f,
            .fov_horizontal = 90.0f,
            .fov_vertical = 60.0f,
            .coverage_radius = 15.0f
        },
        .privacy = PRIVACY_FEATURES,
        .mqtt_broker = "localhost",
        .mqtt_port = 1883,
        .mqtt_client_id = "omnisight_cam",
        .heartbeat_interval_ms = 5000,
        .track_share_interval_ms = 1000,
        .federated_learning_enabled = true
    };

    app->swarm = swarm_init(&swarm_config);
    if (!app->swarm) {
        fprintf(stderr, "[OMNISIGHT] Failed to initialize swarm\n");
        // Non-fatal: can run without swarm
    }

    // Initialize Federated Learner
    if (app->swarm) {
        LocalTrainingConfig fed_config = {
            .batch_size = 32,
            .num_epochs = 5,
            .learning_rate = 0.01f,
            .privacy = PRIVACY_DIFFERENTIAL,
            .gradient_clip_norm = 1.0f,
            .noise_multiplier = 0.1f,
            .privacy_budget = 1.0f
        };

        app->fed_learner = fl_learner_init(camera_id, &fed_config);
    }

    printf("[OMNISIGHT] Initialization complete\n");
    return app;

error:
    omnisight_destroy(app);
    return NULL;
}

void omnisight_destroy(OmnisightApp* app) {
    if (!app) return;

    printf("[OMNISIGHT] Shutting down...\n");

    if (app->fed_learner) {
        fl_learner_destroy(app->fed_learner);
    }

    if (app->swarm) {
        swarm_stop(app->swarm);
        swarm_destroy(app->swarm);
    }

    if (app->timeline) {
        timeline_destroy(app->timeline);
    }

    if (app->perception) {
        perception_stop(app->perception);
        perception_destroy(app->perception);
    }

    if (app->larod) {
        larod_inference_destroy(app->larod);
    }

    if (app->vdo) {
        vdo_capture_stop(app->vdo);
        vdo_capture_destroy(app->vdo);
    }

    free(app);

    printf("[OMNISIGHT] Shutdown complete\n");
}

// ============================================================================
// Main Loop
// ============================================================================

int omnisight_run(OmnisightApp* app) {
    if (!app) return -1;

    printf("[OMNISIGHT] Starting...\n");

    // Start components
    if (!perception_start(app->perception)) {
        fprintf(stderr, "[OMNISIGHT] Failed to start perception\n");
        return -1;
    }

    if (app->swarm) {
        if (!swarm_start(app->swarm)) {
            fprintf(stderr, "[OMNISIGHT] Failed to start swarm (non-fatal)\n");
        }
    }

    // Start VDO capture
    if (!vdo_capture_start(app->vdo, app->perception)) {
        fprintf(stderr, "[OMNISIGHT] Failed to start VDO capture\n");
        return -1;
    }

    app->running = true;

    printf("[OMNISIGHT] Running... (Press Ctrl+C to stop)\n");
    printf("============================================================\n");

    // Main loop - just monitor and print statistics
    uint64_t last_stats_time = 0;
    while (app->running) {
        sleep(5);  // Print stats every 5 seconds

        uint64_t current_time = time(NULL) * 1000;
        if (current_time - last_stats_time >= 5000) {
            printf("\n[OMNISIGHT] Statistics:\n");
            printf("  Frames: %lu\n", app->frame_count);
            printf("  Events predicted: %lu\n", app->event_count);
            printf("  Interventions: %lu\n", app->intervention_count);

            // Perception stats
            uint32_t active_tracks = 0;
            perception_get_stats(app->perception, &active_tracks, NULL, NULL);
            printf("  Active tracks: %u\n", active_tracks);

            // Timeline stats
            if (app->timeline) {
                uint64_t num_updates = 0;
                uint64_t total_events = 0;
                timeline_get_stats(app->timeline, &num_updates, &total_events,
                                  NULL, NULL);
                printf("  Timeline updates: %lu (events: %lu)\n",
                       num_updates, total_events);
            }

            // Swarm stats
            if (app->swarm) {
                SwarmStats swarm_stats;
                swarm_get_stats(app->swarm, &swarm_stats);
                printf("  Tracks shared: %u, received: %u\n",
                       swarm_stats.tracks_shared,
                       swarm_stats.tracks_received);
                printf("  Network health: %.0f%%\n",
                       swarm_stats.network_health * 100);
            }

            printf("------------------------------------------------------------\n");
            last_stats_time = current_time;
        }
    }

    return 0;
}

// ============================================================================
// Main Entry Point
// ============================================================================

int main(int argc, char* argv[]) {
    printf("\n");
    printf("╔════════════════════════════════════════════════════════════╗\n");
    printf("║            OMNISIGHT Precognitive Security                ║\n");
    printf("║                                                            ║\n");
    printf("║  Timeline Threading™ • Swarm Intelligence • Privacy       ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n");
    printf("\n");

    // Parse camera ID from command line
    uint32_t camera_id = 1;
    if (argc > 1) {
        camera_id = atoi(argv[1]);
    }

    // Install signal handlers
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    // Initialize application
    g_app = omnisight_init(camera_id);
    if (!g_app) {
        fprintf(stderr, "Failed to initialize OMNISIGHT\n");
        return 1;
    }

    // Run main loop
    int result = omnisight_run(g_app);

    // Cleanup
    omnisight_destroy(g_app);
    g_app = NULL;

    printf("\n");
    printf("[OMNISIGHT] Goodbye!\n");
    printf("\n");

    return result;
}
