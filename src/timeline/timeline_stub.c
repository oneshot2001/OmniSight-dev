/**
 * @file timeline_stub.c
 * @brief Stub implementation of Timeline Threading™ for testing
 *
 * Simulates precognitive timeline prediction without full ML models.
 */

#include "timeline.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <math.h>

// Stub engine structure
struct TimelineEngine {
    TimelineConfig config;
    TimelineCallback callback;
    void* callback_user_data;

    // Active timelines
    Timeline timelines[5];
    uint32_t num_timelines;

    // Statistics
    uint64_t total_updates;
    uint64_t total_events_predicted;
    uint64_t total_interventions;
    float avg_prediction_ms;

    bool monitoring_active;
};

// ============================================================================
// Utility Functions
// ============================================================================

static uint64_t get_current_time_ms(void) {
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    return (uint64_t)ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
}

static float randf(float min, float max) {
    return min + (float)rand() / RAND_MAX * (max - min);
}

/**
 * Simulate event prediction based on track behavior
 */
static EventType predict_event_from_behavior(BehaviorFlags behaviors, float threat_score) {
    if (behaviors & BEHAVIOR_LOITERING && threat_score > 0.6f) {
        return EVENT_THEFT;
    } else if (behaviors & BEHAVIOR_RUNNING) {
        return EVENT_SUSPICIOUS_BEHAVIOR;
    } else if (behaviors & BEHAVIOR_SUSPICIOUS_MOVEMENT) {
        return EVENT_TRESPASSING;
    } else if (behaviors & BEHAVIOR_CONCEALING) {
        return EVENT_THEFT;
    }

    if (threat_score > 0.5f) {
        return EVENT_SUSPICIOUS_BEHAVIOR;
    }

    return EVENT_NONE;
}

/**
 * Generate simulated timeline from tracked objects
 */
static void generate_simulated_timeline(Timeline* timeline,
                                         const TrackedObject* tracks,
                                         uint32_t num_tracks,
                                         const TimelineConfig* config) {
    static uint32_t timeline_id_counter = 1;

    memset(timeline, 0, sizeof(Timeline));

    timeline->timeline_id = timeline_id_counter++;
    timeline->overall_probability = randf(0.6f, 0.9f);
    timeline->prediction_start_ms = get_current_time_ms();
    timeline->prediction_end_ms = timeline->prediction_start_ms +
                                   config->prediction_horizon_ms;
    timeline->created_ms = get_current_time_ms();
    timeline->last_updated_ms = timeline->created_ms;
    timeline->num_nodes = 1;
    timeline->tree_depth = 1;

    // Generate predicted events based on track behaviors
    timeline->num_predicted_events = 0;
    timeline->num_interventions = 0;
    timeline->worst_case_severity = SEVERITY_NONE;
    timeline->total_threat_score = 0.0f;
    timeline->incident_prevented = false;

    for (uint32_t i = 0; i < num_tracks && timeline->num_predicted_events < 50; i++) {
        const TrackedObject* track = &tracks[i];

        // Skip normal behavior
        if (track->behaviors == BEHAVIOR_NORMAL && track->threat_score < 0.3f) {
            continue;
        }

        // Predict event
        EventType event_type = predict_event_from_behavior(track->behaviors,
                                                           track->threat_score);
        if (event_type == EVENT_NONE) {
            continue;
        }

        PredictedEvent* event = &timeline->predicted_events[timeline->num_predicted_events];
        event->type = event_type;
        event->timestamp_ms = get_current_time_ms() + (rand() % config->prediction_horizon_ms);
        event->probability = randf(0.5f, 0.9f);

        // Determine severity
        if (track->threat_score > 0.7f) {
            event->severity = SEVERITY_HIGH;
        } else if (track->threat_score > 0.5f) {
            event->severity = SEVERITY_MEDIUM;
        } else {
            event->severity = SEVERITY_LOW;
        }

        if (event->severity > timeline->worst_case_severity) {
            timeline->worst_case_severity = event->severity;
        }

        event->num_involved = 1;
        event->involved_tracks[0] = track->track_id;
        event->location_x = track->current_bbox.x;
        event->location_y = track->current_bbox.y;

        // Generate description
        const char* event_name = timeline_event_to_string(event_type);
        snprintf(event->description, sizeof(event->description),
                 "%s predicted at (%.2f, %.2f) with %.0f%% confidence",
                 event_name, event->location_x, event->location_y,
                 event->probability * 100);

        timeline->total_threat_score += track->threat_score;
        timeline->num_predicted_events++;

        // Generate intervention if severity is high enough
        if (event->severity >= SEVERITY_MEDIUM &&
            timeline->num_interventions < 20) {

            InterventionPoint* intervention =
                &timeline->interventions[timeline->num_interventions];

            intervention->timestamp_ms = event->timestamp_ms -
                                        (config->prediction_horizon_ms / 10);  // 10% before event
            intervention->effectiveness = randf(0.7f, 0.95f);
            intervention->cost = randf(0.1f, 0.5f);
            intervention->prevented_event = *event;

            // Recommend intervention type based on event
            if (event->type == EVENT_THEFT || event->type == EVENT_VANDALISM) {
                intervention->type = INTERVENTION_ALERT_SECURITY;
                snprintf(intervention->recommendation, sizeof(intervention->recommendation),
                         "Alert security personnel to area (%.2f, %.2f)",
                         event->location_x, event->location_y);
            } else if (event->type == EVENT_TRESPASSING) {
                intervention->type = INTERVENTION_ACTIVATE_SPEAKER;
                snprintf(intervention->recommendation, sizeof(intervention->recommendation),
                         "Activate audio warning at (%.2f, %.2f)",
                         event->location_x, event->location_y);
            } else {
                intervention->type = INTERVENTION_INCREASE_LIGHTING;
                snprintf(intervention->recommendation, sizeof(intervention->recommendation),
                         "Increase lighting in monitored area");
            }

            timeline->num_interventions++;
        }
    }
}

// ============================================================================
// Public API Implementation
// ============================================================================

TimelineEngine* timeline_init(const TimelineConfig* config) {
    if (!config) {
        return NULL;
    }

    printf("[Timeline] Initializing Timeline Threading™ stub...\n");

    TimelineEngine* engine = (TimelineEngine*)calloc(1, sizeof(TimelineEngine));
    if (!engine) {
        return NULL;
    }

    engine->config = *config;
    engine->num_timelines = 0;
    engine->total_updates = 0;
    engine->total_events_predicted = 0;
    engine->total_interventions = 0;
    engine->avg_prediction_ms = 35.0f;
    engine->monitoring_active = false;

    printf("[Timeline] ✓ Timeline Threading™ stub initialized\n");
    printf("[Timeline]   - Prediction horizon: %u ms (%.1f min)\n",
           config->prediction_horizon_ms,
           config->prediction_horizon_ms / 60000.0f);
    printf("[Timeline]   - Max timelines: %u\n", config->max_timelines);
    printf("[Timeline]   - Simulated mode (predictive algorithms stubbed)\n");

    return engine;
}

uint32_t timeline_update(TimelineEngine* engine,
                          const TrackedObject* tracks,
                          uint32_t num_tracks,
                          Timeline* timelines,
                          uint32_t max_timelines) {
    if (!engine || !tracks || num_tracks == 0 || !timelines) {
        return 0;
    }

    uint64_t start_time = get_current_time_ms();

    // Generate 1-3 timeline branches based on track count
    uint32_t num_generated = 1 + (num_tracks / 3);
    if (num_generated > engine->config.max_timelines) {
        num_generated = engine->config.max_timelines;
    }
    if (num_generated > max_timelines) {
        num_generated = max_timelines;
    }

    for (uint32_t i = 0; i < num_generated; i++) {
        generate_simulated_timeline(&timelines[i], tracks, num_tracks,
                                     &engine->config);

        // Store in engine
        if (i < 5) {
            engine->timelines[i] = timelines[i];
        }
    }

    engine->num_timelines = num_generated;
    engine->total_updates++;

    // Count total events and interventions
    for (uint32_t i = 0; i < num_generated; i++) {
        engine->total_events_predicted += timelines[i].num_predicted_events;
        engine->total_interventions += timelines[i].num_interventions;
    }

    // Update prediction time stat
    uint64_t end_time = get_current_time_ms();
    float prediction_time = (float)(end_time - start_time);
    engine->avg_prediction_ms = engine->avg_prediction_ms * 0.9f +
                                prediction_time * 0.1f;

    return num_generated;
}

uint32_t timeline_get_timelines(TimelineEngine* engine,
                                 Timeline* timelines,
                                 uint32_t max_timelines) {
    if (!engine || !timelines) return 0;

    uint32_t count = engine->num_timelines < max_timelines ?
                     engine->num_timelines : max_timelines;
    memcpy(timelines, engine->timelines, count * sizeof(Timeline));

    return count;
}

bool timeline_get_by_id(TimelineEngine* engine,
                        uint32_t timeline_id,
                        Timeline* timeline) {
    if (!engine || !timeline) return false;

    for (uint32_t i = 0; i < engine->num_timelines; i++) {
        if (engine->timelines[i].timeline_id == timeline_id) {
            *timeline = engine->timelines[i];
            return true;
        }
    }

    return false;
}

bool timeline_get_best_intervention(TimelineEngine* engine,
                                     InterventionPoint* intervention) {
    if (!engine || !intervention) return false;

    // Find timeline with highest severity
    SeverityLevel max_severity = SEVERITY_NONE;
    int best_timeline_idx = -1;

    for (uint32_t i = 0; i < engine->num_timelines; i++) {
        if (engine->timelines[i].worst_case_severity > max_severity) {
            max_severity = engine->timelines[i].worst_case_severity;
            best_timeline_idx = i;
        }
    }

    if (best_timeline_idx == -1 || max_severity < SEVERITY_MEDIUM) {
        return false;  // No intervention needed
    }

    // Return first intervention from worst timeline
    Timeline* worst_timeline = &engine->timelines[best_timeline_idx];
    if (worst_timeline->num_interventions > 0) {
        *intervention = worst_timeline->interventions[0];
        return true;
    }

    return false;
}

uint32_t timeline_apply_intervention(TimelineEngine* engine,
                                      const InterventionPoint* intervention,
                                      Timeline* new_timelines,
                                      uint32_t max_timelines) {
    if (!engine || !intervention || !new_timelines) return 0;

    // Stub: Just mark incidents as prevented in existing timelines
    (void)max_timelines;

    for (uint32_t i = 0; i < engine->num_timelines; i++) {
        engine->timelines[i].incident_prevented = true;
        engine->timelines[i].total_threat_score *= 0.5f;  // Reduce threat
    }

    return engine->num_timelines;
}

bool timeline_start_monitoring(TimelineEngine* engine,
                                TimelineCallback callback,
                                void* user_data) {
    if (!engine) return false;

    engine->callback = callback;
    engine->callback_user_data = user_data;
    engine->monitoring_active = true;

    printf("[Timeline] ✓ Monitoring started\n");
    return true;
}

void timeline_stop_monitoring(TimelineEngine* engine) {
    if (!engine) return;

    engine->monitoring_active = false;
    printf("[Timeline] ✓ Monitoring stopped\n");
}

void timeline_clear(TimelineEngine* engine) {
    if (!engine) return;

    engine->num_timelines = 0;
    memset(engine->timelines, 0, sizeof(engine->timelines));
}

void timeline_get_stats(TimelineEngine* engine,
                        uint64_t* total_timelines,
                        uint64_t* total_events,
                        uint64_t* total_interventions,
                        float* avg_prediction_ms) {
    if (!engine) return;

    if (total_timelines) *total_timelines = engine->total_updates;
    if (total_events) *total_events = engine->total_events_predicted;
    if (total_interventions) *total_interventions = engine->total_interventions;
    if (avg_prediction_ms) *avg_prediction_ms = engine->avg_prediction_ms;
}

void timeline_destroy(TimelineEngine* engine) {
    if (!engine) return;

    if (engine->monitoring_active) {
        timeline_stop_monitoring(engine);
    }

    free(engine);
    printf("[Timeline] ✓ Timeline Threading™ destroyed\n");
}

int timeline_export_json(const Timeline* timeline,
                         char* buffer,
                         size_t buffer_size) {
    if (!timeline || !buffer) return -1;

    int written = snprintf(buffer, buffer_size,
        "{\"timeline_id\":%u,\"probability\":%.2f,\"events\":%u,\"interventions\":%u}",
        timeline->timeline_id,
        timeline->overall_probability,
        timeline->num_predicted_events,
        timeline->num_interventions);

    return written < (int)buffer_size ? written : -1;
}

const char* timeline_event_to_string(EventType event_type) {
    switch (event_type) {
        case EVENT_LOITERING: return "Loitering";
        case EVENT_THEFT: return "Theft";
        case EVENT_ASSAULT: return "Assault";
        case EVENT_VANDALISM: return "Vandalism";
        case EVENT_TRESPASSING: return "Trespassing";
        case EVENT_SUSPICIOUS_BEHAVIOR: return "Suspicious Behavior";
        case EVENT_COLLISION: return "Collision";
        case EVENT_FALL: return "Fall";
        case EVENT_ABANDONED_OBJECT: return "Abandoned Object";
        case EVENT_CROWD_FORMATION: return "Crowd Formation";
        default: return "Unknown";
    }
}

const char* timeline_intervention_to_string(InterventionType intervention_type) {
    switch (intervention_type) {
        case INTERVENTION_ALERT_SECURITY: return "Alert Security";
        case INTERVENTION_ACTIVATE_SPEAKER: return "Activate Speaker";
        case INTERVENTION_INCREASE_LIGHTING: return "Increase Lighting";
        case INTERVENTION_POSITION_GUARD: return "Position Guard";
        case INTERVENTION_LOCK_DOOR: return "Lock Door";
        case INTERVENTION_NOTIFY_POLICE: return "Notify Police";
        case INTERVENTION_DISPLAY_WARNING: return "Display Warning";
        default: return "None";
    }
}
