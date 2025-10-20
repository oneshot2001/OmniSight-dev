/**
 * @file event_predictor.h
 * @brief Event prediction for Timeline Threading
 *
 * Predicts specific security events based on:
 * - Predicted trajectories
 * - Behavior patterns
 * - Threat scores
 * - Historical incident data
 * - Scene context
 */

#ifndef OMNISIGHT_EVENT_PREDICTOR_H
#define OMNISIGHT_EVENT_PREDICTOR_H

#include "timeline.h"
#include "trajectory_predictor.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct EventPredictor EventPredictor;

/**
 * Scene context for event prediction
 */
typedef struct {
    // Protected zones
    uint32_t num_zones;
    struct {
        float x, y, radius;      // Zone location and size
        EventType protected_event; // Event type to prevent in this zone
        float sensitivity;       // Detection sensitivity [0.0, 1.0]
    } zones[20];

    // Historical data
    uint32_t num_incidents;
    struct {
        EventType type;
        uint64_t timestamp_ms;
        float location_x, location_y;
    } incident_history[100];

    // Time-based patterns
    float time_of_day_risk;      // Risk multiplier based on time
    float day_of_week_risk;      // Risk multiplier based on day
} SceneContext;

/**
 * Event predictor configuration
 */
typedef struct {
    // Detection thresholds
    float loitering_threshold_ms;
    float theft_proximity_threshold;
    float assault_velocity_threshold;
    float collision_distance_threshold;

    // Weights for event scoring
    float trajectory_weight;
    float behavior_weight;
    float context_weight;
    float history_weight;

    // Scene context
    const SceneContext* scene;
} EventPredictorConfig;

/**
 * Initialize event predictor
 *
 * @param config Predictor configuration
 * @return Predictor instance, NULL on failure
 */
EventPredictor* event_predictor_init(const EventPredictorConfig* config);

/**
 * Predict events from trajectories
 *
 * @param predictor Event predictor instance
 * @param trajectories Predicted trajectories
 * @param num_trajectories Number of trajectories
 * @param events Output array for predicted events
 * @param max_events Maximum events to return
 * @return Number of events predicted
 */
uint32_t event_predictor_predict(
    EventPredictor* predictor,
    const PredictedTrajectory* trajectories,
    uint32_t num_trajectories,
    PredictedEvent* events,
    uint32_t max_events
);

/**
 * Predict loitering event
 *
 * @param predictor Event predictor instance
 * @param trajectory Predicted trajectory
 * @param event Output predicted event
 * @return true if loitering predicted
 */
bool event_predict_loitering(
    EventPredictor* predictor,
    const PredictedTrajectory* trajectory,
    PredictedEvent* event
);

/**
 * Predict theft event
 *
 * Detects patterns like:
 * - Approach to valuable area
 * - Concealment behavior
 * - Rapid exit
 *
 * @param predictor Event predictor instance
 * @param trajectories All trajectories (to detect accomplices)
 * @param num_trajectories Number of trajectories
 * @param event Output predicted event
 * @return true if theft predicted
 */
bool event_predict_theft(
    EventPredictor* predictor,
    const PredictedTrajectory* trajectories,
    uint32_t num_trajectories,
    PredictedEvent* event
);

/**
 * Predict assault event
 *
 * Detects patterns like:
 * - Rapid approach to person
 * - Following behavior
 * - Aggressive posture
 *
 * @param predictor Event predictor instance
 * @param trajectories All trajectories
 * @param num_trajectories Number of trajectories
 * @param event Output predicted event
 * @return true if assault predicted
 */
bool event_predict_assault(
    EventPredictor* predictor,
    const PredictedTrajectory* trajectories,
    uint32_t num_trajectories,
    PredictedEvent* event
);

/**
 * Predict collision event
 *
 * @param predictor Event predictor instance
 * @param trajectories All trajectories
 * @param num_trajectories Number of trajectories
 * @param event Output predicted event
 * @return true if collision predicted
 */
bool event_predict_collision(
    EventPredictor* predictor,
    const PredictedTrajectory* trajectories,
    uint32_t num_trajectories,
    PredictedEvent* event
);

/**
 * Predict trespassing event
 *
 * @param predictor Event predictor instance
 * @param trajectory Predicted trajectory
 * @param event Output predicted event
 * @return true if trespassing predicted
 */
bool event_predict_trespassing(
    EventPredictor* predictor,
    const PredictedTrajectory* trajectory,
    PredictedEvent* event
);

/**
 * Calculate event severity based on context
 *
 * @param predictor Event predictor instance
 * @param event Predicted event
 * @return Severity level
 */
SeverityLevel event_calculate_severity(
    EventPredictor* predictor,
    const PredictedEvent* event
);

/**
 * Update scene context (e.g., new incident occurred)
 *
 * @param predictor Event predictor instance
 * @param event Event that occurred
 */
void event_predictor_update_context(
    EventPredictor* predictor,
    const PredictedEvent* event
);

/**
 * Destroy event predictor
 *
 * @param predictor Predictor instance
 */
void event_predictor_destroy(EventPredictor* predictor);

#ifdef __cplusplus
}
#endif

#endif // OMNISIGHT_EVENT_PREDICTOR_H
