/**
 * @file trajectory_predictor.c
 * @brief Trajectory prediction implementation for Timeline Threading
 *
 * Implements multiple motion models for predicting future object trajectories:
 * - Constant velocity model (simple, fast)
 * - Kalman filter model (smooth, handles noise)
 * - Social force model (models interactions between objects)
 * - ML-based model (most accurate, uses learned patterns)
 */

#include "trajectory_predictor.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdio.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/**
 * Internal trajectory predictor state
 */
struct TrajectoryPredictor {
    TrajectoryPredictorConfig config;

    // Statistics
    struct {
        uint64_t num_predictions;
        uint64_t num_branches;
        float avg_confidence;
    } stats;

    // ML model handle (placeholder for TFLite interpreter)
    void* ml_model;
};

/**
 * Simple Kalman filter for trajectory smoothing
 */
typedef struct {
    // State: [x, y, vx, vy]
    float state[4];

    // Covariance matrix (4x4, stored as 16 elements)
    float P[16];

    // Process noise
    float Q[16];

    // Measurement noise
    float R[4];
} KalmanFilter;

// ============================================================================
// Utility Functions
// ============================================================================

static float clamp(float value, float min, float max) {
    if (value < min) return min;
    if (value > max) return max;
    return value;
}

static float distance(float x1, float y1, float x2, float y2) {
    float dx = x2 - x1;
    float dy = y2 - y1;
    return sqrtf(dx*dx + dy*dy);
}

// ============================================================================
// Kalman Filter Implementation
// ============================================================================

static void kalman_init(KalmanFilter* kf, float x, float y, float vx, float vy) {
    // Initialize state
    kf->state[0] = x;
    kf->state[1] = y;
    kf->state[2] = vx;
    kf->state[3] = vy;

    // Initialize covariance (identity * 10)
    memset(kf->P, 0, sizeof(kf->P));
    kf->P[0] = kf->P[5] = kf->P[10] = kf->P[15] = 10.0f;

    // Process noise (small, tuned for smooth motion)
    memset(kf->Q, 0, sizeof(kf->Q));
    kf->Q[0] = kf->Q[5] = 1.0f;      // Position noise
    kf->Q[10] = kf->Q[15] = 0.1f;    // Velocity noise

    // Measurement noise
    kf->R[0] = kf->R[1] = 2.0f;      // Position measurement noise
    kf->R[2] = kf->R[3] = 0.5f;      // Velocity measurement noise
}

static void kalman_predict(KalmanFilter* kf, float dt) {
    // State transition: x' = x + vx*dt, y' = y + vy*dt
    kf->state[0] += kf->state[2] * dt;
    kf->state[1] += kf->state[3] * dt;

    // Update covariance: P' = F*P*F^T + Q
    // Simplified for constant velocity model
    for (int i = 0; i < 16; i++) {
        kf->P[i] += kf->Q[i];
    }
}

static void kalman_update(KalmanFilter* kf, float x, float y, float vx, float vy) {
    // Kalman gain (simplified)
    float K[4] = {0.3f, 0.3f, 0.2f, 0.2f};  // Tuned values

    // Update state with measurement
    kf->state[0] += K[0] * (x - kf->state[0]);
    kf->state[1] += K[1] * (y - kf->state[1]);
    kf->state[2] += K[2] * (vx - kf->state[2]);
    kf->state[3] += K[3] * (vy - kf->state[3]);
}

// ============================================================================
// Motion Model Implementations
// ============================================================================

/**
 * Constant velocity model
 * Simplest model: assumes object continues at current velocity
 */
static void predict_constant_velocity(
    const TrackedObject* track,
    uint32_t num_steps,
    float dt,
    PredictedState* predictions
) {
    float x = track->box.x + track->box.width / 2;
    float y = track->box.y + track->box.height / 2;
    float vx = track->velocity_x;
    float vy = track->velocity_y;

    for (uint32_t i = 0; i < num_steps; i++) {
        x += vx * dt;
        y += vy * dt;

        predictions[i].timestamp_ms = track->last_seen_ms + (uint64_t)((i + 1) * dt * 1000);
        predictions[i].x = x;
        predictions[i].y = y;
        predictions[i].vx = vx;
        predictions[i].vy = vy;
        predictions[i].confidence = 1.0f / (1.0f + 0.1f * i);  // Decreases with time
        predictions[i].behaviors = track->behaviors;
        predictions[i].threat_score = track->threat_score;
    }
}

/**
 * Kalman filter model
 * Smoother predictions with noise handling
 */
static void predict_kalman(
    const TrackedObject* track,
    uint32_t num_steps,
    float dt,
    PredictedState* predictions
) {
    KalmanFilter kf;

    float x = track->box.x + track->box.width / 2;
    float y = track->box.y + track->box.height / 2;

    kalman_init(&kf, x, y, track->velocity_x, track->velocity_y);

    for (uint32_t i = 0; i < num_steps; i++) {
        kalman_predict(&kf, dt);

        predictions[i].timestamp_ms = track->last_seen_ms + (uint64_t)((i + 1) * dt * 1000);
        predictions[i].x = kf.state[0];
        predictions[i].y = kf.state[1];
        predictions[i].vx = kf.state[2];
        predictions[i].vy = kf.state[3];
        predictions[i].confidence = 1.0f / (1.0f + 0.05f * i);  // Slower decay than CV
        predictions[i].behaviors = track->behaviors;
        predictions[i].threat_score = track->threat_score;
    }
}

/**
 * Social force model
 * Models interactions between objects (attraction/repulsion)
 */
static void predict_social_force(
    const TrackedObject* track,
    const TrackedObject* other_tracks,
    uint32_t num_other_tracks,
    uint32_t num_steps,
    float dt,
    PredictedState* predictions
) {
    float x = track->box.x + track->box.width / 2;
    float y = track->box.y + track->box.height / 2;
    float vx = track->velocity_x;
    float vy = track->velocity_y;

    for (uint32_t i = 0; i < num_steps; i++) {
        // Self-propulsion force (goal-directed)
        float desired_speed = sqrtf(vx*vx + vy*vy);
        float fx = vx;  // Continue in current direction
        float fy = vy;

        // Interaction forces with other objects
        for (uint32_t j = 0; j < num_other_tracks; j++) {
            if (other_tracks[j].track_id == track->track_id) continue;

            float ox = other_tracks[j].box.x + other_tracks[j].box.width / 2;
            float oy = other_tracks[j].box.y + other_tracks[j].box.height / 2;

            float dx = x - ox;
            float dy = y - oy;
            float dist = sqrtf(dx*dx + dy*dy);

            if (dist < 0.01f) dist = 0.01f;  // Avoid division by zero

            // Repulsion force (inversely proportional to distance)
            float repulsion_strength = 100.0f / (dist * dist);
            fx += repulsion_strength * dx / dist;
            fy += repulsion_strength * dy / dist;
        }

        // Update velocity with forces (damping factor)
        float damping = 0.9f;
        vx = damping * vx + 0.1f * fx;
        vy = damping * vy + 0.1f * fy;

        // Update position
        x += vx * dt;
        y += vy * dt;

        predictions[i].timestamp_ms = track->last_seen_ms + (uint64_t)((i + 1) * dt * 1000);
        predictions[i].x = x;
        predictions[i].y = y;
        predictions[i].vx = vx;
        predictions[i].vy = vy;
        predictions[i].confidence = 1.0f / (1.0f + 0.08f * i);
        predictions[i].behaviors = track->behaviors;
        predictions[i].threat_score = track->threat_score;
    }
}

/**
 * ML-based prediction (placeholder for TFLite model)
 * Most accurate, uses learned patterns from historical data
 */
static void predict_ml(
    TrajectoryPredictor* predictor,
    const TrackedObject* track,
    const SceneContext* context,
    uint32_t num_steps,
    float dt,
    PredictedState* predictions
) {
    // TODO: Implement TFLite model inference when ML model is ready
    // For now, fall back to Kalman filter
    predict_kalman(track, num_steps, dt, predictions);

    // Adjust confidence based on historical accuracy
    for (uint32_t i = 0; i < num_steps; i++) {
        predictions[i].confidence *= 0.9f;  // ML model typically more confident
    }
}

// ============================================================================
// Public API Implementation
// ============================================================================

TrajectoryPredictor* trajectory_predictor_init(const TrajectoryPredictorConfig* config) {
    if (!config) {
        fprintf(stderr, "[TrajPredict] ERROR: NULL config\n");
        return NULL;
    }

    TrajectoryPredictor* predictor = calloc(1, sizeof(TrajectoryPredictor));
    if (!predictor) {
        fprintf(stderr, "[TrajPredict] ERROR: Failed to allocate memory\n");
        return NULL;
    }

    // Copy configuration
    memcpy(&predictor->config, config, sizeof(TrajectoryPredictorConfig));

    // Initialize statistics
    predictor->stats.num_predictions = 0;
    predictor->stats.num_branches = 0;
    predictor->stats.avg_confidence = 0.0f;

    // TODO: Load ML model if configured
    predictor->ml_model = NULL;

    printf("[TrajPredict] Initialized (model=%d, horizon=%.1fs)\n",
           config->motion_model, config->prediction_horizon_s);

    return predictor;
}

void trajectory_predictor_destroy(TrajectoryPredictor* predictor) {
    if (!predictor) return;

    // TODO: Unload ML model if loaded

    free(predictor);
}

bool trajectory_predict_single(
    TrajectoryPredictor* predictor,
    const TrackedObject* track,
    const TrackedObject* other_tracks,
    uint32_t num_other_tracks,
    const SceneContext* context,
    PredictedTrajectory* trajectory
) {
    if (!predictor || !track || !trajectory) {
        fprintf(stderr, "[TrajPredict] ERROR: NULL parameter\n");
        return false;
    }

    // Calculate number of prediction steps
    float dt = predictor->config.prediction_step_s;
    uint32_t num_steps = (uint32_t)(predictor->config.prediction_horizon_s / dt);
    if (num_steps > 300) num_steps = 300;  // Safety limit

    trajectory->track_id = track->track_id;
    trajectory->num_predictions = num_steps;

    // Choose motion model
    switch (predictor->config.motion_model) {
        case MOTION_MODEL_CONSTANT_VELOCITY:
            predict_constant_velocity(track, num_steps, dt, trajectory->predictions);
            break;

        case MOTION_MODEL_KALMAN:
            predict_kalman(track, num_steps, dt, trajectory->predictions);
            break;

        case MOTION_MODEL_SOCIAL_FORCE:
            if (other_tracks && num_other_tracks > 0) {
                predict_social_force(track, other_tracks, num_other_tracks,
                                   num_steps, dt, trajectory->predictions);
            } else {
                // Fall back to Kalman if no other tracks
                predict_kalman(track, num_steps, dt, trajectory->predictions);
            }
            break;

        case MOTION_MODEL_ML:
            predict_ml(predictor, track, context, num_steps, dt, trajectory->predictions);
            break;

        default:
            fprintf(stderr, "[TrajPredict] ERROR: Unknown motion model %d\n",
                   predictor->config.motion_model);
            return false;
    }

    // Calculate overall confidence
    float sum_confidence = 0.0f;
    for (uint32_t i = 0; i < num_steps; i++) {
        sum_confidence += trajectory->predictions[i].confidence;
    }
    trajectory->overall_confidence = sum_confidence / num_steps;

    // Update statistics
    predictor->stats.num_predictions++;
    predictor->stats.avg_confidence =
        (predictor->stats.avg_confidence * (predictor->stats.num_predictions - 1) +
         trajectory->overall_confidence) / predictor->stats.num_predictions;

    return true;
}

uint32_t trajectory_predict_branches(
    TrajectoryPredictor* predictor,
    const TrackedObject* track,
    uint32_t num_branches,
    const TrackedObject* other_tracks,
    uint32_t num_other_tracks,
    const SceneContext* context,
    PredictedTrajectory* trajectories
) {
    if (!predictor || !track || !trajectories || num_branches == 0) {
        fprintf(stderr, "[TrajPredict] ERROR: Invalid parameters\n");
        return 0;
    }

    if (num_branches > 5) {
        num_branches = 5;  // Limit to 5 branches
    }

    // First trajectory: most likely (using configured model)
    if (!trajectory_predict_single(predictor, track, other_tracks,
                                   num_other_tracks, context, &trajectories[0])) {
        return 0;
    }

    if (num_branches == 1) {
        return 1;
    }

    // Generate alternative branches by perturbing velocity
    float vx = track->velocity_x;
    float vy = track->velocity_y;
    float speed = sqrtf(vx*vx + vy*vy);

    for (uint32_t b = 1; b < num_branches; b++) {
        // Create modified track for alternative trajectory
        TrackedObject modified_track = *track;

        // Perturb direction (20 degree increments)
        float angle = atan2f(vy, vx);
        float angle_delta = (M_PI / 9.0f) * (b % 2 == 0 ? 1 : -1) * ((b + 1) / 2);
        float new_angle = angle + angle_delta;

        modified_track.velocity_x = speed * cosf(new_angle);
        modified_track.velocity_y = speed * sinf(new_angle);

        // Predict alternative trajectory
        if (!trajectory_predict_single(predictor, &modified_track, other_tracks,
                                      num_other_tracks, context, &trajectories[b])) {
            continue;
        }

        // Reduce confidence for alternative branches
        trajectories[b].overall_confidence *= 0.7f / b;
        for (uint32_t i = 0; i < trajectories[b].num_predictions; i++) {
            trajectories[b].predictions[i].confidence *= 0.7f / b;
        }
    }

    // Update statistics
    predictor->stats.num_branches += num_branches;

    return num_branches;
}

bool trajectory_detect_collision(
    const PredictedTrajectory* traj1,
    const PredictedTrajectory* traj2,
    float collision_distance,
    uint64_t* collision_time_ms,
    float* collision_x,
    float* collision_y
) {
    if (!traj1 || !traj2) {
        return false;
    }

    // Check each time step for collision
    uint32_t max_steps = traj1->num_predictions < traj2->num_predictions ?
                        traj1->num_predictions : traj2->num_predictions;

    for (uint32_t i = 0; i < max_steps; i++) {
        float dist = distance(
            traj1->predictions[i].x, traj1->predictions[i].y,
            traj2->predictions[i].x, traj2->predictions[i].y
        );

        if (dist < collision_distance) {
            // Collision detected
            if (collision_time_ms) {
                *collision_time_ms = traj1->predictions[i].timestamp_ms;
            }
            if (collision_x && collision_y) {
                *collision_x = (traj1->predictions[i].x + traj2->predictions[i].x) / 2;
                *collision_y = (traj1->predictions[i].y + traj2->predictions[i].y) / 2;
            }
            return true;
        }
    }

    return false;
}

bool trajectory_check_zone_entry(
    const PredictedTrajectory* trajectory,
    float zone_x,
    float zone_y,
    float zone_radius,
    uint64_t* entry_time_ms,
    float* entry_confidence
) {
    if (!trajectory) {
        return false;
    }

    for (uint32_t i = 0; i < trajectory->num_predictions; i++) {
        float dist = distance(
            trajectory->predictions[i].x, trajectory->predictions[i].y,
            zone_x, zone_y
        );

        if (dist < zone_radius) {
            // Zone entry detected
            if (entry_time_ms) {
                *entry_time_ms = trajectory->predictions[i].timestamp_ms;
            }
            if (entry_confidence) {
                *entry_confidence = trajectory->predictions[i].confidence;
            }
            return true;
        }
    }

    return false;
}

float trajectory_calculate_time_to_location(
    const PredictedTrajectory* trajectory,
    float target_x,
    float target_y,
    float proximity_threshold
) {
    if (!trajectory) {
        return -1.0f;
    }

    float min_distance = INFINITY;
    uint32_t closest_step = 0;

    for (uint32_t i = 0; i < trajectory->num_predictions; i++) {
        float dist = distance(
            trajectory->predictions[i].x, trajectory->predictions[i].y,
            target_x, target_y
        );

        if (dist < min_distance) {
            min_distance = dist;
            closest_step = i;
        }

        // If within threshold, return time
        if (dist < proximity_threshold) {
            uint64_t delta_ms = trajectory->predictions[i].timestamp_ms -
                              trajectory->predictions[0].timestamp_ms;
            return delta_ms / 1000.0f;
        }
    }

    // Never reaches location within horizon
    return -1.0f;
}

void trajectory_predictor_get_stats(
    const TrajectoryPredictor* predictor,
    uint64_t* num_predictions,
    uint64_t* num_branches,
    float* avg_confidence
) {
    if (!predictor) return;

    if (num_predictions) *num_predictions = predictor->stats.num_predictions;
    if (num_branches) *num_branches = predictor->stats.num_branches;
    if (avg_confidence) *avg_confidence = predictor->stats.avg_confidence;
}
