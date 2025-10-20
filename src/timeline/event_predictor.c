/**
 * @file event_predictor.c
 * @brief Event prediction implementation for Timeline Threading
 *
 * Predicts specific security events based on:
 * - Predicted trajectories
 * - Behavior patterns
 * - Threat scores
 * - Historical incident data
 * - Scene context
 */

#include "event_predictor.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdio.h>
#include <time.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/**
 * Internal event predictor state
 */
struct EventPredictor {
    EventPredictorConfig config;
    SceneContext scene;

    // Statistics
    struct {
        uint64_t num_predictions;
        uint64_t events_by_type[10];  // Count per EventType
        float avg_confidence;
    } stats;
};

// ============================================================================
// Utility Functions
// ============================================================================

static float distance(float x1, float y1, float x2, float y2) {
    float dx = x2 - x1;
    float dy = y2 - y1;
    return sqrtf(dx*dx + dy*dy);
}

static float clamp(float value, float min, float max) {
    if (value < min) return min;
    if (value > max) return max;
    return value;
}

/**
 * Get current timestamp in milliseconds
 */
static uint64_t get_current_time_ms() {
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    return (uint64_t)ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
}

/**
 * Calculate historical risk for a location
 */
static float calculate_historical_risk(
    const EventPredictor* predictor,
    float x,
    float y,
    EventType event_type
) {
    float risk = 0.0f;
    uint64_t current_time = get_current_time_ms();

    // Check incident history
    for (uint32_t i = 0; i < predictor->scene.num_incidents; i++) {
        const auto* incident = &predictor->scene.incident_history[i];

        // Only consider relevant event types
        if (incident->type != event_type && incident->type != EVENT_TYPE_OTHER) {
            continue;
        }

        // Calculate spatial proximity
        float dist = distance(x, y, incident->location_x, incident->location_y);
        float spatial_factor = 1.0f / (1.0f + dist / 100.0f);  // Decay with distance

        // Calculate temporal decay (incidents become less relevant over time)
        uint64_t age_ms = current_time - incident->timestamp_ms;
        float age_days = age_ms / (1000.0f * 60 * 60 * 24);
        float temporal_factor = expf(-age_days / 30.0f);  // 30-day decay

        risk += spatial_factor * temporal_factor;
    }

    return clamp(risk, 0.0f, 1.0f);
}

/**
 * Check if location is in a protected zone
 */
static bool is_in_protected_zone(
    const EventPredictor* predictor,
    float x,
    float y,
    EventType event_type,
    float* sensitivity
) {
    for (uint32_t i = 0; i < predictor->scene.num_zones; i++) {
        const auto* zone = &predictor->scene.zones[i];

        // Check if zone protects against this event type
        if (zone->protected_event != event_type && zone->protected_event != EVENT_TYPE_OTHER) {
            continue;
        }

        float dist = distance(x, y, zone->x, zone->y);
        if (dist < zone->radius) {
            if (sensitivity) {
                *sensitivity = zone->sensitivity;
            }
            return true;
        }
    }

    return false;
}

// ============================================================================
// Event Prediction Functions
// ============================================================================

bool event_predict_loitering(
    EventPredictor* predictor,
    const PredictedTrajectory* trajectory,
    PredictedEvent* event
) {
    if (!predictor || !trajectory || !event) {
        return false;
    }

    // Analyze trajectory for loitering patterns
    // Loitering = staying in small area for extended time

    if (trajectory->num_predictions < 10) {
        return false;  // Not enough data
    }

    // Calculate area covered by trajectory
    float min_x = INFINITY, max_x = -INFINITY;
    float min_y = INFINITY, max_y = -INFINITY;

    for (uint32_t i = 0; i < trajectory->num_predictions; i++) {
        float x = trajectory->predictions[i].x;
        float y = trajectory->predictions[i].y;

        if (x < min_x) min_x = x;
        if (x > max_x) max_x = x;
        if (y < min_y) min_y = y;
        if (y > max_y) max_y = y;
    }

    float area_width = max_x - min_x;
    float area_height = max_y - min_y;
    float area = area_width * area_height;

    // Calculate time span
    uint64_t time_span_ms = trajectory->predictions[trajectory->num_predictions - 1].timestamp_ms -
                           trajectory->predictions[0].timestamp_ms;

    // Loitering detection: small area for long time
    float loitering_threshold_area = 1000.0f;  // pixels^2
    bool is_loitering = (area < loitering_threshold_area) &&
                       (time_span_ms > predictor->config.loitering_threshold_ms);

    if (!is_loitering) {
        return false;
    }

    // Calculate center of loitering area
    float center_x = (min_x + max_x) / 2;
    float center_y = (min_y + max_y) / 2;

    // Build predicted event
    memset(event, 0, sizeof(PredictedEvent));
    event->type = EVENT_TYPE_LOITERING;
    event->timestamp_ms = trajectory->predictions[trajectory->num_predictions / 2].timestamp_ms;
    event->location_x = center_x;
    event->location_y = center_y;
    event->num_involved_tracks = 1;
    event->involved_tracks[0] = trajectory->track_id;

    // Calculate probability
    float base_prob = 0.6f;
    float history_factor = calculate_historical_risk(predictor, center_x, center_y,
                                                     EVENT_TYPE_LOITERING);
    float context_factor = predictor->scene.time_of_day_risk;

    event->probability = clamp(
        base_prob * (1.0f + history_factor * 0.5f) * context_factor,
        0.0f, 1.0f
    );

    // Calculate severity
    event->severity = event_calculate_severity(predictor, event);

    predictor->stats.num_predictions++;
    predictor->stats.events_by_type[EVENT_TYPE_LOITERING]++;

    return true;
}

bool event_predict_theft(
    EventPredictor* predictor,
    const PredictedTrajectory* trajectories,
    uint32_t num_trajectories,
    PredictedEvent* event
) {
    if (!predictor || !trajectories || !event || num_trajectories == 0) {
        return false;
    }

    // Theft pattern detection:
    // 1. Approach to protected area
    // 2. Loitering/concealment behavior
    // 3. Rapid exit (optional: with accomplices)

    for (uint32_t i = 0; i < num_trajectories; i++) {
        const PredictedTrajectory* traj = &trajectories[i];

        if (traj->num_predictions < 10) continue;

        // Check for approach to protected zone
        float sensitivity = 0.0f;
        bool enters_zone = false;
        uint32_t entry_step = 0;

        for (uint32_t step = 0; step < traj->num_predictions; step++) {
            if (is_in_protected_zone(predictor,
                                    traj->predictions[step].x,
                                    traj->predictions[step].y,
                                    EVENT_TYPE_THEFT,
                                    &sensitivity)) {
                enters_zone = true;
                entry_step = step;
                break;
            }
        }

        if (!enters_zone) continue;

        // Check for suspicious behaviors
        bool has_loitering = (traj->predictions[entry_step].behaviors & BEHAVIOR_LOITERING) != 0;
        bool has_concealing = (traj->predictions[entry_step].behaviors & BEHAVIOR_CONCEALING) != 0;
        float threat_score = traj->predictions[entry_step].threat_score;

        // Check for rapid exit after zone entry
        bool rapid_exit = false;
        if (entry_step + 10 < traj->num_predictions) {
            float exit_speed = sqrtf(
                traj->predictions[entry_step + 10].vx * traj->predictions[entry_step + 10].vx +
                traj->predictions[entry_step + 10].vy * traj->predictions[entry_step + 10].vy
            );
            rapid_exit = exit_speed > 50.0f;  // Fast movement
        }

        // Theft likelihood calculation
        float theft_likelihood = 0.0f;
        theft_likelihood += has_loitering ? 0.3f : 0.0f;
        theft_likelihood += has_concealing ? 0.4f : 0.0f;
        theft_likelihood += rapid_exit ? 0.2f : 0.0f;
        theft_likelihood += threat_score * 0.3f;
        theft_likelihood *= sensitivity;  // Zone sensitivity

        if (theft_likelihood < predictor->config.theft_proximity_threshold) {
            continue;
        }

        // Check for accomplices
        uint32_t num_accomplices = 0;
        uint32_t accomplice_ids[3];

        for (uint32_t j = 0; j < num_trajectories && num_accomplices < 3; j++) {
            if (j == i) continue;

            const PredictedTrajectory* other = &trajectories[j];

            // Check if other trajectory is nearby at entry time
            if (entry_step < other->num_predictions) {
                float dist = distance(
                    traj->predictions[entry_step].x,
                    traj->predictions[entry_step].y,
                    other->predictions[entry_step].x,
                    other->predictions[entry_step].y
                );

                if (dist < 100.0f) {  // Within 100 pixels
                    accomplice_ids[num_accomplices++] = other->track_id;
                }
            }
        }

        // Build predicted event
        memset(event, 0, sizeof(PredictedEvent));
        event->type = EVENT_TYPE_THEFT;
        event->timestamp_ms = traj->predictions[entry_step].timestamp_ms;
        event->location_x = traj->predictions[entry_step].x;
        event->location_y = traj->predictions[entry_step].y;
        event->num_involved_tracks = 1 + num_accomplices;
        event->involved_tracks[0] = traj->track_id;
        for (uint32_t k = 0; k < num_accomplices; k++) {
            event->involved_tracks[k + 1] = accomplice_ids[k];
        }

        // Probability increases with accomplices
        float accomplice_factor = 1.0f + num_accomplices * 0.2f;
        float history_factor = calculate_historical_risk(
            predictor,
            event->location_x,
            event->location_y,
            EVENT_TYPE_THEFT
        );

        event->probability = clamp(
            theft_likelihood * accomplice_factor * (1.0f + history_factor * 0.5f),
            0.0f, 1.0f
        );

        event->severity = event_calculate_severity(predictor, event);

        predictor->stats.num_predictions++;
        predictor->stats.events_by_type[EVENT_TYPE_THEFT]++;

        return true;
    }

    return false;
}

bool event_predict_assault(
    EventPredictor* predictor,
    const PredictedTrajectory* trajectories,
    uint32_t num_trajectories,
    PredictedEvent* event
) {
    if (!predictor || !trajectories || !event || num_trajectories < 2) {
        return false;
    }

    // Assault pattern detection:
    // 1. Rapid approach to person
    // 2. Following behavior
    // 3. Aggressive posture/movement

    for (uint32_t i = 0; i < num_trajectories; i++) {
        const PredictedTrajectory* traj1 = &trajectories[i];

        for (uint32_t j = i + 1; j < num_trajectories; j++) {
            const PredictedTrajectory* traj2 = &trajectories[j];

            // Check for rapid approach between two objects
            float initial_dist = distance(
                traj1->predictions[0].x, traj1->predictions[0].y,
                traj2->predictions[0].x, traj2->predictions[0].y
            );

            // Find minimum distance in future
            float min_dist = initial_dist;
            uint32_t min_dist_step = 0;
            uint32_t max_steps = traj1->num_predictions < traj2->num_predictions ?
                                traj1->num_predictions : traj2->num_predictions;

            for (uint32_t step = 1; step < max_steps; step++) {
                float dist = distance(
                    traj1->predictions[step].x, traj1->predictions[step].y,
                    traj2->predictions[step].x, traj2->predictions[step].y
                );

                if (dist < min_dist) {
                    min_dist = dist;
                    min_dist_step = step;
                }
            }

            // Check for rapid approach
            float approach_speed = (initial_dist - min_dist) / (min_dist_step * 1.0f);
            bool is_rapid_approach = approach_speed > predictor->config.assault_velocity_threshold;

            if (!is_rapid_approach || min_dist > 50.0f) {
                continue;  // Not close enough or not approaching
            }

            // Check for following behavior
            bool is_following = false;
            float avg_dist = 0.0f;
            for (uint32_t step = min_dist_step; step < max_steps; step++) {
                float dist = distance(
                    traj1->predictions[step].x, traj1->predictions[step].y,
                    traj2->predictions[step].x, traj2->predictions[step].y
                );
                avg_dist += dist;
            }
            avg_dist /= (max_steps - min_dist_step);
            is_following = avg_dist < 80.0f;  // Stays close

            // Check threat scores
            float threat1 = traj1->predictions[min_dist_step].threat_score;
            float threat2 = traj2->predictions[min_dist_step].threat_score;
            float max_threat = threat1 > threat2 ? threat1 : threat2;

            // Calculate assault likelihood
            float assault_likelihood = 0.0f;
            assault_likelihood += is_rapid_approach ? 0.4f : 0.0f;
            assault_likelihood += is_following ? 0.3f : 0.0f;
            assault_likelihood += max_threat * 0.5f;

            if (assault_likelihood < 0.5f) {
                continue;
            }

            // Build predicted event
            memset(event, 0, sizeof(PredictedEvent));
            event->type = EVENT_TYPE_ASSAULT;
            event->timestamp_ms = traj1->predictions[min_dist_step].timestamp_ms;
            event->location_x = (traj1->predictions[min_dist_step].x +
                                traj2->predictions[min_dist_step].x) / 2;
            event->location_y = (traj1->predictions[min_dist_step].y +
                                traj2->predictions[min_dist_step].y) / 2;
            event->num_involved_tracks = 2;
            event->involved_tracks[0] = traj1->track_id;
            event->involved_tracks[1] = traj2->track_id;

            float history_factor = calculate_historical_risk(
                predictor,
                event->location_x,
                event->location_y,
                EVENT_TYPE_ASSAULT
            );
            float time_factor = predictor->scene.time_of_day_risk;

            event->probability = clamp(
                assault_likelihood * (1.0f + history_factor * 0.3f) * time_factor,
                0.0f, 1.0f
            );

            event->severity = event_calculate_severity(predictor, event);

            predictor->stats.num_predictions++;
            predictor->stats.events_by_type[EVENT_TYPE_ASSAULT]++;

            return true;
        }
    }

    return false;
}

bool event_predict_collision(
    EventPredictor* predictor,
    const PredictedTrajectory* trajectories,
    uint32_t num_trajectories,
    PredictedEvent* event
) {
    if (!predictor || !trajectories || !event || num_trajectories < 2) {
        return false;
    }

    // Check all pairs of trajectories for potential collisions
    for (uint32_t i = 0; i < num_trajectories; i++) {
        for (uint32_t j = i + 1; j < num_trajectories; j++) {
            uint64_t collision_time = 0;
            float collision_x = 0, collision_y = 0;

            if (trajectory_detect_collision(
                &trajectories[i],
                &trajectories[j],
                predictor->config.collision_distance_threshold,
                &collision_time,
                &collision_x,
                &collision_y
            )) {
                // Collision predicted
                memset(event, 0, sizeof(PredictedEvent));
                event->type = EVENT_TYPE_COLLISION;
                event->timestamp_ms = collision_time;
                event->location_x = collision_x;
                event->location_y = collision_y;
                event->num_involved_tracks = 2;
                event->involved_tracks[0] = trajectories[i].track_id;
                event->involved_tracks[1] = trajectories[j].track_id;

                // Probability based on confidence of both trajectories
                event->probability = (trajectories[i].overall_confidence +
                                     trajectories[j].overall_confidence) / 2;

                event->severity = event_calculate_severity(predictor, event);

                predictor->stats.num_predictions++;
                predictor->stats.events_by_type[EVENT_TYPE_COLLISION]++;

                return true;
            }
        }
    }

    return false;
}

bool event_predict_trespassing(
    EventPredictor* predictor,
    const PredictedTrajectory* trajectory,
    PredictedEvent* event
) {
    if (!predictor || !trajectory || !event) {
        return false;
    }

    // Check if trajectory enters any protected zone
    for (uint32_t step = 0; step < trajectory->num_predictions; step++) {
        float sensitivity = 0.0f;
        if (is_in_protected_zone(predictor,
                                 trajectory->predictions[step].x,
                                 trajectory->predictions[step].y,
                                 EVENT_TYPE_TRESPASSING,
                                 &sensitivity)) {
            // Trespassing predicted
            memset(event, 0, sizeof(PredictedEvent));
            event->type = EVENT_TYPE_TRESPASSING;
            event->timestamp_ms = trajectory->predictions[step].timestamp_ms;
            event->location_x = trajectory->predictions[step].x;
            event->location_y = trajectory->predictions[step].y;
            event->num_involved_tracks = 1;
            event->involved_tracks[0] = trajectory->track_id;

            event->probability = trajectory->predictions[step].confidence * sensitivity;

            event->severity = event_calculate_severity(predictor, event);

            predictor->stats.num_predictions++;
            predictor->stats.events_by_type[EVENT_TYPE_TRESPASSING]++;

            return true;
        }
    }

    return false;
}

// ============================================================================
// Public API Implementation
// ============================================================================

EventPredictor* event_predictor_init(const EventPredictorConfig* config) {
    if (!config) {
        fprintf(stderr, "[EventPredict] ERROR: NULL config\n");
        return NULL;
    }

    EventPredictor* predictor = calloc(1, sizeof(EventPredictor));
    if (!predictor) {
        fprintf(stderr, "[EventPredict] ERROR: Failed to allocate memory\n");
        return NULL;
    }

    // Copy configuration
    memcpy(&predictor->config, config, sizeof(EventPredictorConfig));

    // Copy scene context if provided
    if (config->scene) {
        memcpy(&predictor->scene, config->scene, sizeof(SceneContext));
    } else {
        memset(&predictor->scene, 0, sizeof(SceneContext));
    }

    // Initialize statistics
    memset(&predictor->stats, 0, sizeof(predictor->stats));

    printf("[EventPredict] Initialized (%u zones, %u incidents)\n",
           predictor->scene.num_zones, predictor->scene.num_incidents);

    return predictor;
}

void event_predictor_destroy(EventPredictor* predictor) {
    if (!predictor) return;
    free(predictor);
}

uint32_t event_predictor_predict(
    EventPredictor* predictor,
    const PredictedTrajectory* trajectories,
    uint32_t num_trajectories,
    PredictedEvent* events,
    uint32_t max_events
) {
    if (!predictor || !trajectories || !events || max_events == 0) {
        return 0;
    }

    uint32_t num_events = 0;

    // Try to predict each event type
    PredictedEvent candidate;

    // Collision (check first - highest priority)
    if (event_predict_collision(predictor, trajectories, num_trajectories, &candidate)) {
        if (num_events < max_events) {
            events[num_events++] = candidate;
        }
    }

    // Assault
    if (event_predict_assault(predictor, trajectories, num_trajectories, &candidate)) {
        if (num_events < max_events) {
            events[num_events++] = candidate;
        }
    }

    // Theft
    if (event_predict_theft(predictor, trajectories, num_trajectories, &candidate)) {
        if (num_events < max_events) {
            events[num_events++] = candidate;
        }
    }

    // Loitering and trespassing (per trajectory)
    for (uint32_t i = 0; i < num_trajectories && num_events < max_events; i++) {
        if (event_predict_loitering(predictor, &trajectories[i], &candidate)) {
            if (num_events < max_events) {
                events[num_events++] = candidate;
            }
        }

        if (event_predict_trespassing(predictor, &trajectories[i], &candidate)) {
            if (num_events < max_events) {
                events[num_events++] = candidate;
            }
        }
    }

    return num_events;
}

SeverityLevel event_calculate_severity(
    EventPredictor* predictor,
    const PredictedEvent* event
) {
    if (!predictor || !event) {
        return SEVERITY_LOW;
    }

    // Base severity by event type
    float severity_score = 0.0f;

    switch (event->type) {
        case EVENT_TYPE_ASSAULT:
        case EVENT_TYPE_THEFT:
            severity_score = 0.8f;
            break;

        case EVENT_TYPE_COLLISION:
        case EVENT_TYPE_TRESPASSING:
            severity_score = 0.6f;
            break;

        case EVENT_TYPE_LOITERING:
        case EVENT_TYPE_SUSPICIOUS_BEHAVIOR:
            severity_score = 0.4f;
            break;

        default:
            severity_score = 0.3f;
            break;
    }

    // Adjust by probability
    severity_score *= event->probability;

    // Adjust by context
    severity_score *= (1.0f + predictor->scene.time_of_day_risk * 0.3f);

    // Adjust by historical incidents
    float history_risk = calculate_historical_risk(
        predictor,
        event->location_x,
        event->location_y,
        event->type
    );
    severity_score *= (1.0f + history_risk * 0.2f);

    // Map to severity level
    if (severity_score > 0.8f) return SEVERITY_CRITICAL;
    if (severity_score > 0.6f) return SEVERITY_HIGH;
    if (severity_score > 0.4f) return SEVERITY_MEDIUM;
    return SEVERITY_LOW;
}

void event_predictor_update_context(
    EventPredictor* predictor,
    const PredictedEvent* event
) {
    if (!predictor || !event) return;

    // Add to incident history
    if (predictor->scene.num_incidents < 100) {
        uint32_t idx = predictor->scene.num_incidents++;
        predictor->scene.incident_history[idx].type = event->type;
        predictor->scene.incident_history[idx].timestamp_ms = event->timestamp_ms;
        predictor->scene.incident_history[idx].location_x = event->location_x;
        predictor->scene.incident_history[idx].location_y = event->location_y;
    } else {
        // Circular buffer: overwrite oldest
        static uint32_t oldest_idx = 0;
        predictor->scene.incident_history[oldest_idx].type = event->type;
        predictor->scene.incident_history[oldest_idx].timestamp_ms = event->timestamp_ms;
        predictor->scene.incident_history[oldest_idx].location_x = event->location_x;
        predictor->scene.incident_history[oldest_idx].location_y = event->location_y;
        oldest_idx = (oldest_idx + 1) % 100;
    }
}
