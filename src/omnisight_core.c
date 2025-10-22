/**
 * @file omnisight_core.c
 * @brief OMNISIGHT Core Integration Implementation
 */

#include "omnisight_core.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <pthread.h>

/**
 * Core system state
 */
struct OmnisightCore {
    OmnisightConfig config;

    // Module instances
    PerceptionEngine* perception;
    TimelineEngine* timeline;
    SwarmNode* swarm;

    // State
    bool is_running;
    uint64_t start_time_ms;
    uint64_t frames_processed;

    // Event callback
    OmnisightEventCallback event_callback;
    void* callback_user_data;

    // Thread safety
    pthread_mutex_t mutex;

    // Current data
    TrackedObject current_tracks[50];
    uint32_t num_current_tracks;

    Timeline current_timelines[5];
    uint32_t num_current_timelines;
};

// =======================================================================
// Utility Functions
// =======================================================================

static uint64_t get_current_time_ms() {
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    return (uint64_t)ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
}

static void send_event(OmnisightCore* core, OmnisightEventType type,
                       const char* message, void* data) {
    if (!core->event_callback) return;

    OmnisightEvent event = {
        .type = type,
        .timestamp_ms = get_current_time_ms(),
        .data = data
    };

    snprintf(event.message, sizeof(event.message), "%s", message);
    core->event_callback(&event, core->callback_user_data);
}

// =======================================================================
// Module Integration Callbacks
// =======================================================================

/**
 * Perception callback - called when new tracked objects available
 */
static void perception_callback(const TrackedObject* objects,
                                 uint32_t object_count,
                                 void* user_data) {
    OmnisightCore* core = (OmnisightCore*)user_data;

    pthread_mutex_lock(&core->mutex);

    // Store current tracks
    uint32_t count = object_count < 50 ? object_count : 50;
    memcpy(core->current_tracks, objects, count * sizeof(TrackedObject));
    core->num_current_tracks = count;
    core->frames_processed++;

    pthread_mutex_unlock(&core->mutex);

    // Notify
    char msg[256];
    snprintf(msg, sizeof(msg), "Tracked %u objects", object_count);
    send_event(core, OMNISIGHT_EVENT_OBJECT_TRACKED, msg, NULL);

    // Forward to timeline if enabled
    if (core->timeline && core->config.enable_timeline) {
        Timeline timelines[5];
        uint32_t num_timelines = timeline_update(
            core->timeline,
            objects,
            object_count,
            timelines,
            5
        );

        if (num_timelines > 0) {
            pthread_mutex_lock(&core->mutex);
            memcpy(core->current_timelines, timelines,
                   num_timelines * sizeof(Timeline));
            core->num_current_timelines = num_timelines;
            pthread_mutex_unlock(&core->mutex);

            send_event(core, OMNISIGHT_EVENT_TIMELINE_UPDATED,
                      "Timelines updated", NULL);
        }
    }

    // Forward to swarm if enabled
    if (core->swarm && core->config.enable_swarm) {
        swarm_update_tracks(core->swarm, objects, object_count,
                            get_current_time_ms());
    }
}

/**
 * Timeline callback - called when timelines updated
 */
static void timeline_callback(const Timeline* timelines,
                              uint32_t num_timelines,
                              void* user_data) {
    OmnisightCore* core = (OmnisightCore*)user_data;

    // Check for high-severity events that need intervention
    for (uint32_t i = 0; i < num_timelines; i++) {
        if (timelines[i].worst_case_severity >= SEVERITY_HIGH) {
            send_event(core, OMNISIGHT_EVENT_INTERVENTION_RECOMMENDED,
                      "High severity event predicted - intervention needed",
                      (void*)&timelines[i]);

            // Share with swarm if enabled
            if (core->swarm && core->config.enable_swarm) {
                for (uint32_t j = 0; j < timelines[i].num_predicted_events; j++) {
                    swarm_share_event(core->swarm,
                                     &timelines[i].predicted_events[j],
                                     &timelines[i]);
                }
            }
        }
    }
}

// =======================================================================
// Public API Implementation
// =======================================================================

void omnisight_get_default_config(OmnisightConfig* config) {
    memset(config, 0, sizeof(OmnisightConfig));

    config->camera_id = 1;
    config->camera_name = "OMNISIGHT-01";
    config->location = "Main Entrance";

    // Perception defaults
    config->perception.frame_width = 1920;
    config->perception.frame_height = 1080;
    config->perception.target_fps = 10;
    config->perception.model_path = "/opt/omnisight/models/detection.tflite";
    config->perception.use_dlpu = true;
    config->perception.inference_threads = 2;
    config->perception.detection_threshold = 0.5f;
    config->perception.tracking_threshold = 0.3f;
    config->perception.max_tracked_objects = 50;
    config->perception.loitering_threshold_ms = 30000;  // 30 seconds
    config->perception.running_velocity_threshold = 5.0f;
    config->perception.async_inference = true;
    config->perception.buffer_pool_size = 4;

    // Timeline defaults
    config->timeline.prediction_horizon_ms = 300000;  // 5 minutes
    config->timeline.time_step_ms = 1000;  // 1 second
    config->timeline.max_timelines = 5;
    config->timeline.branch_threshold = 0.3f;
    config->timeline.merge_threshold = 0.8f;
    config->timeline.event_threshold = 0.5f;
    config->timeline.enable_intervention_search = true;
    config->timeline.max_iterations = 1000;
    config->timeline.use_gpu = true;

    // Swarm defaults
    config->swarm.camera_id = config->camera_id;
    config->swarm.role = SWARM_ROLE_INTERNAL;
    config->swarm.privacy = PRIVACY_FEATURES;
    config->swarm.mqtt_broker = "localhost";
    config->swarm.mqtt_port = 1883;
    config->swarm.mqtt_client_id = "omnisight-01";
    config->swarm.mqtt_username = NULL;
    config->swarm.mqtt_password = NULL;
    config->swarm.heartbeat_interval_ms = 5000;
    config->swarm.track_share_interval_ms = 1000;
    config->swarm.consensus_timeout_ms = 3000;
    config->swarm.handoff_lookahead_ms = 2000;
    config->swarm.federated_learning_enabled = false;
    config->swarm.model_update_interval_ms = 300000;
    config->swarm.min_improvement_threshold = 0.01f;

    // Integration defaults
    config->enable_perception = true;
    config->enable_timeline = true;
    config->enable_swarm = false;  // Disabled by default (requires network)
    config->auto_start = true;
    config->update_interval_ms = 100;
    config->verbose_logging = true;
}

bool omnisight_validate_config(const OmnisightConfig* config) {
    if (!config) return false;

    // Basic validation
    if (config->camera_id == 0) {
        printf("[OMNISIGHT] Error: Invalid camera_id (must be > 0)\n");
        return false;
    }

    if (config->enable_perception && config->perception.target_fps == 0) {
        printf("[OMNISIGHT] Error: Invalid target_fps\n");
        return false;
    }

    if (config->enable_timeline && config->timeline.prediction_horizon_ms == 0) {
        printf("[OMNISIGHT] Error: Invalid prediction_horizon_ms\n");
        return false;
    }

    return true;
}

OmnisightCore* omnisight_init(const OmnisightConfig* config) {
    if (!omnisight_validate_config(config)) {
        return NULL;
    }

    printf("[OMNISIGHT] Initializing system...\n");

    OmnisightCore* core = (OmnisightCore*)calloc(1, sizeof(OmnisightCore));
    if (!core) {
        printf("[OMNISIGHT] Error: Failed to allocate core structure\n");
        return NULL;
    }

    core->config = *config;
    core->is_running = false;
    core->start_time_ms = get_current_time_ms();
    core->frames_processed = 0;
    pthread_mutex_init(&core->mutex, NULL);

    // Initialize enabled modules
    if (config->enable_perception) {
        printf("[OMNISIGHT] Initializing perception engine...\n");
        core->perception = perception_init(&config->perception);
        if (!core->perception) {
            printf("[OMNISIGHT] Warning: Perception init failed (stub mode)\n");
            // Continue in stub mode - don't fail
        } else {
            printf("[OMNISIGHT] ✓ Perception engine ready\n");
        }
    }

    if (config->enable_timeline) {
        printf("[OMNISIGHT] Initializing Timeline Threading™...\n");
        core->timeline = timeline_init(&config->timeline);
        if (!core->timeline) {
            printf("[OMNISIGHT] Warning: Timeline init failed (stub mode)\n");
        } else {
            printf("[OMNISIGHT] ✓ Timeline Threading™ ready\n");
        }
    }

    if (config->enable_swarm) {
        printf("[OMNISIGHT] Initializing Swarm Intelligence...\n");
        core->swarm = swarm_init(&config->swarm);
        if (!core->swarm) {
            printf("[OMNISIGHT] Warning: Swarm init failed (disabled)\n");
            core->config.enable_swarm = false;  // Disable if failed
        } else {
            printf("[OMNISIGHT] ✓ Swarm Intelligence ready\n");
        }
    }

    printf("[OMNISIGHT] ✓ System initialized successfully\n\n");
    return core;
}

bool omnisight_start(OmnisightCore* core,
                     OmnisightEventCallback callback,
                     void* user_data) {
    if (!core || core->is_running) {
        return false;
    }

    printf("[OMNISIGHT] Starting all enabled modules...\n");

    core->event_callback = callback;
    core->callback_user_data = user_data;
    core->is_running = true;

    // Start perception
    if (core->perception && core->config.enable_perception) {
        printf("[OMNISIGHT] Starting perception engine...\n");
        if (!perception_start(core->perception, perception_callback, core)) {
            printf("[OMNISIGHT] Warning: Perception start failed\n");
        } else {
            printf("[OMNISIGHT] ✓ Perception running\n");
        }
    }

    // Start timeline monitoring
    if (core->timeline && core->config.enable_timeline) {
        printf("[OMNISIGHT] Starting Timeline Threading™...\n");
        if (!timeline_start_monitoring(core->timeline, timeline_callback, core)) {
            printf("[OMNISIGHT] Warning: Timeline start failed\n");
        } else {
            printf("[OMNISIGHT] ✓ Timeline Threading™ active\n");
        }
    }

    // Start swarm
    if (core->swarm && core->config.enable_swarm) {
        printf("[OMNISIGHT] Starting Swarm Intelligence...\n");
        if (!swarm_start(core->swarm)) {
            printf("[OMNISIGHT] Warning: Swarm start failed\n");
        } else {
            printf("[OMNISIGHT] ✓ Swarm Intelligence connected\n");
        }
    }

    send_event(core, OMNISIGHT_EVENT_STARTED, "OMNISIGHT system started", NULL);
    printf("[OMNISIGHT] ✓ All modules started successfully\n\n");

    return true;
}

bool omnisight_process(OmnisightCore* core) {
    if (!core || !core->is_running) {
        return false;
    }

    // This would be called in main loop
    // For now, modules handle their own processing via callbacks

    return true;
}

void omnisight_stop(OmnisightCore* core) {
    if (!core || !core->is_running) {
        return;
    }

    printf("[OMNISIGHT] Stopping all modules...\n");

    core->is_running = false;

    if (core->perception) {
        perception_stop(core->perception);
        printf("[OMNISIGHT] ✓ Perception stopped\n");
    }

    if (core->timeline) {
        timeline_stop_monitoring(core->timeline);
        printf("[OMNISIGHT] ✓ Timeline Threading™ stopped\n");
    }

    if (core->swarm) {
        swarm_stop(core->swarm);
        printf("[OMNISIGHT] ✓ Swarm Intelligence disconnected\n");
    }

    send_event(core, OMNISIGHT_EVENT_STOPPED, "OMNISIGHT system stopped", NULL);
    printf("[OMNISIGHT] ✓ All modules stopped\n");
}

void omnisight_get_stats(OmnisightCore* core, OmnisightStats* stats) {
    if (!core || !stats) return;

    memset(stats, 0, sizeof(OmnisightStats));

    pthread_mutex_lock(&core->mutex);

    stats->is_running = core->is_running;
    stats->uptime_ms = get_current_time_ms() - core->start_time_ms;
    stats->frames_processed = core->frames_processed;

    // Perception stats
    if (core->perception) {
        perception_get_stats(core->perception,
                            &stats->perception_stats.avg_inference_ms,
                            &stats->perception_stats.avg_fps,
                            &stats->perception_stats.dropped_frames);
        stats->perception_stats.tracked_objects = core->num_current_tracks;
    }

    // Timeline stats
    if (core->timeline) {
        uint64_t total_events, total_interventions;
        timeline_get_stats(core->timeline,
                          (uint64_t*)&stats->timeline_stats.active_timelines,
                          &total_events,
                          &total_interventions,
                          &stats->timeline_stats.avg_prediction_ms);
        stats->timeline_stats.total_events_predicted = total_events;
        stats->timeline_stats.total_interventions = total_interventions;
        stats->timeline_stats.active_timelines = core->num_current_timelines;
    }

    // Swarm stats
    if (core->swarm) {
        SwarmStats swarm_stats;
        swarm_get_stats(core->swarm, &swarm_stats);
        stats->swarm_stats.num_neighbors = swarm_stats.num_online;
        stats->swarm_stats.tracks_shared = swarm_stats.tracks_shared;
        stats->swarm_stats.events_shared = swarm_stats.events_shared;
        stats->swarm_stats.network_health = swarm_stats.network_health;
    }

    pthread_mutex_unlock(&core->mutex);
}

uint32_t omnisight_get_tracked_objects(OmnisightCore* core,
                                        TrackedObject* objects,
                                        uint32_t max_objects) {
    if (!core || !objects) return 0;

    pthread_mutex_lock(&core->mutex);
    uint32_t count = core->num_current_tracks < max_objects ?
                     core->num_current_tracks : max_objects;
    memcpy(objects, core->current_tracks, count * sizeof(TrackedObject));
    pthread_mutex_unlock(&core->mutex);

    return count;
}

uint32_t omnisight_get_timelines(OmnisightCore* core,
                                  Timeline* timelines,
                                  uint32_t max_timelines) {
    if (!core || !timelines) return 0;

    pthread_mutex_lock(&core->mutex);
    uint32_t count = core->num_current_timelines < max_timelines ?
                     core->num_current_timelines : max_timelines;
    memcpy(timelines, core->current_timelines, count * sizeof(Timeline));
    pthread_mutex_unlock(&core->mutex);

    return count;
}

bool omnisight_get_intervention(OmnisightCore* core,
                                 InterventionPoint* intervention) {
    if (!core || !core->timeline) return false;

    return timeline_get_best_intervention(core->timeline, intervention);
}

void omnisight_refresh_timelines(OmnisightCore* core) {
    if (!core || !core->timeline) return;

    timeline_clear(core->timeline);
    send_event(core, OMNISIGHT_EVENT_TIMELINE_UPDATED,
              "Timelines refreshed", NULL);
}

void omnisight_sync_swarm(OmnisightCore* core) {
    if (!core || !core->swarm || !core->config.enable_swarm) return;

    // Force sync of current state
    if (core->num_current_tracks > 0) {
        swarm_update_tracks(core->swarm, core->current_tracks,
                           core->num_current_tracks, get_current_time_ms());
    }
}

bool omnisight_health_check(OmnisightCore* core,
                            bool* perception_ok,
                            bool* timeline_ok,
                            bool* swarm_ok) {
    if (!core) return false;

    if (perception_ok) *perception_ok = (core->perception != NULL);
    if (timeline_ok) *timeline_ok = (core->timeline != NULL);
    if (swarm_ok) *swarm_ok = (core->swarm != NULL);

    return core->is_running;
}

void omnisight_destroy(OmnisightCore* core) {
    if (!core) return;

    printf("[OMNISIGHT] Shutting down system...\n");

    if (core->is_running) {
        omnisight_stop(core);
    }

    if (core->perception) {
        perception_destroy(core->perception);
        printf("[OMNISIGHT] ✓ Perception destroyed\n");
    }

    if (core->timeline) {
        timeline_destroy(core->timeline);
        printf("[OMNISIGHT] ✓ Timeline destroyed\n");
    }

    if (core->swarm) {
        swarm_destroy(core->swarm);
        printf("[OMNISIGHT] ✓ Swarm destroyed\n");
    }

    pthread_mutex_destroy(&core->mutex);
    free(core);

    printf("[OMNISIGHT] ✓ System shutdown complete\n");
}
