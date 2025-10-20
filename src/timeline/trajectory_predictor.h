/**
 * @file trajectory_predictor.h
 * @brief Trajectory prediction for Timeline Threading
 *
 * Predicts future positions and behaviors of tracked objects using:
 * - Kalman filtering
 * - Physics-based motion models
 * - Social force models (pedestrian interaction)
 * - ML-based trajectory prediction (future)
 */

#ifndef OMNISIGHT_TRAJECTORY_PREDICTOR_H
#define OMNISIGHT_TRAJECTORY_PREDICTOR_H

#include "../perception/perception.h"
#include "timeline.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct TrajectoryPredictor TrajectoryPredictor;

/**
 * Motion model types
 */
typedef enum {
    MOTION_CONSTANT_VELOCITY,    // Simple linear extrapolation
    MOTION_CONSTANT_ACCELERATION, // Include acceleration
    MOTION_KALMAN_FILTER,        // Kalman-based prediction
    MOTION_SOCIAL_FORCE,         // Social force model (pedestrians)
    MOTION_ML_MODEL              // ML-based (future)
} MotionModel;

/**
 * Trajectory predictor configuration
 */
typedef struct {
    MotionModel model;           // Motion model to use
    uint32_t history_length;     // Frames of history to use
    float uncertainty_growth;    // How fast uncertainty grows
    bool consider_interactions;  // Model object interactions
    bool consider_boundaries;    // Consider scene boundaries
    const char* ml_model_path;   // Path to ML model (if using)
} TrajectoryConfig;

/**
 * Complete predicted trajectory for one object
 */
typedef struct {
    uint32_t track_id;              // Track being predicted
    uint32_t num_predictions;       // Number of future states
    PredictedState predictions[300]; // Future states (5min @ 1s intervals)
    float overall_confidence;       // Overall trajectory confidence
} PredictedTrajectory;

/**
 * Initialize trajectory predictor
 *
 * @param config Predictor configuration
 * @return Predictor instance, NULL on failure
 */
TrajectoryPredictor* trajectory_predictor_init(const TrajectoryConfig* config);

/**
 * Predict trajectory for a single tracked object
 *
 * @param predictor Predictor instance
 * @param track Tracked object
 * @param history Previous positions (optional, can be NULL)
 * @param history_length Number of history entries
 * @param prediction_horizon_ms How far to predict
 * @param time_step_ms Prediction granularity
 * @param trajectory Output predicted trajectory
 * @return true on success
 */
bool trajectory_predict_single(
    TrajectoryPredictor* predictor,
    const TrackedObject* track,
    const TrackedObject* history,
    uint32_t history_length,
    uint32_t prediction_horizon_ms,
    uint32_t time_step_ms,
    PredictedTrajectory* trajectory
);

/**
 * Predict trajectories for multiple objects (considers interactions)
 *
 * @param predictor Predictor instance
 * @param tracks Current tracked objects
 * @param num_tracks Number of tracks
 * @param prediction_horizon_ms How far to predict
 * @param time_step_ms Prediction granularity
 * @param trajectories Output array for trajectories
 * @param max_trajectories Maximum trajectories to return
 * @return Number of trajectories generated
 */
uint32_t trajectory_predict_multiple(
    TrajectoryPredictor* predictor,
    const TrackedObject* tracks,
    uint32_t num_tracks,
    uint32_t prediction_horizon_ms,
    uint32_t time_step_ms,
    PredictedTrajectory* trajectories,
    uint32_t max_trajectories
);

/**
 * Predict multiple possible trajectories (branching scenarios)
 *
 * Generates alternative futures based on uncertainty and decisions
 *
 * @param predictor Predictor instance
 * @param track Tracked object
 * @param num_branches Number of alternative trajectories
 * @param prediction_horizon_ms How far to predict
 * @param time_step_ms Prediction granularity
 * @param trajectories Output array for alternative trajectories
 * @param probabilities Probability of each trajectory
 * @return Number of trajectories generated
 */
uint32_t trajectory_predict_branches(
    TrajectoryPredictor* predictor,
    const TrackedObject* track,
    uint32_t num_branches,
    uint32_t prediction_horizon_ms,
    uint32_t time_step_ms,
    PredictedTrajectory* trajectories,
    float* probabilities
);

/**
 * Detect collision between predicted trajectories
 *
 * @param traj1 First trajectory
 * @param traj2 Second trajectory
 * @param collision_threshold Distance threshold for collision
 * @param collision_time Output time of collision (ms from now)
 * @param collision_probability Output probability of collision
 * @return true if collision predicted
 */
bool trajectory_detect_collision(
    const PredictedTrajectory* traj1,
    const PredictedTrajectory* traj2,
    float collision_threshold,
    uint64_t* collision_time,
    float* collision_probability
);

/**
 * Detect if trajectory enters a zone
 *
 * @param trajectory Predicted trajectory
 * @param zone_x Zone center X (normalized)
 * @param zone_y Zone center Y (normalized)
 * @param zone_radius Zone radius (normalized)
 * @param entry_time Output time of zone entry (ms from now)
 * @param entry_probability Output probability of entry
 * @return true if zone entry predicted
 */
bool trajectory_detect_zone_entry(
    const PredictedTrajectory* trajectory,
    float zone_x,
    float zone_y,
    float zone_radius,
    uint64_t* entry_time,
    float* entry_probability
);

/**
 * Calculate trajectory similarity (for merging similar predictions)
 *
 * @param traj1 First trajectory
 * @param traj2 Second trajectory
 * @return Similarity score [0.0, 1.0]
 */
float trajectory_similarity(
    const PredictedTrajectory* traj1,
    const PredictedTrajectory* traj2
);

/**
 * Destroy trajectory predictor
 *
 * @param predictor Predictor instance
 */
void trajectory_predictor_destroy(TrajectoryPredictor* predictor);

#ifdef __cplusplus
}
#endif

#endif // OMNISIGHT_TRAJECTORY_PREDICTOR_H
