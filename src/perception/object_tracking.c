/**
 * @file object_tracking.c
 * @brief OMNISIGHT Object Tracking Implementation
 *
 * SORT (Simple Online and Realtime Tracking) algorithm with:
 * - Kalman filter for motion prediction
 * - Hungarian algorithm for detection-to-track assignment
 * - IoU-based distance metric
 *
 * Copyright (C) 2025 OMNISIGHT
 * Based on SORT paper by Bewley et al. (2016)
 */

#include "object_tracking.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <float.h>
#include <syslog.h>
#include <pthread.h>

// Kalman filter state size
#define STATE_DIM 7  // [x, y, s, r, vx, vy, vs]
#define MEAS_DIM 4   // [x, y, s, r]

/**
 * Kalman filter state for a single track
 */
typedef struct {
    float state[STATE_DIM];          // State vector [x, y, s, r, vx, vy, vs]
    float covariance[STATE_DIM][STATE_DIM];  // Covariance matrix
    bool initialized;
} KalmanFilter;

/**
 * Individual track in the tracker
 */
typedef struct {
    uint32_t id;                     // Unique track ID
    ObjectClass class_id;            // Object class
    KalmanFilter kf;                 // Kalman filter
    uint32_t hits;                   // Number of successful updates
    uint32_t age;                    // Frames since creation
    uint32_t time_since_update;      // Frames since last detection
    float confidence;                // Current confidence
    uint64_t first_seen_ms;          // First detection timestamp
    uint64_t last_seen_ms;           // Last update timestamp
    float features[128];             // Feature vector
} Track;

/**
 * Object tracker instance
 */
struct ObjectTracker {
    TrackerConfig config;
    Track* tracks;                   // Array of tracks
    uint32_t num_tracks;             // Current number of tracks
    uint32_t next_id;                // Next track ID to assign
    uint32_t total_tracks_created;   // Statistics
    pthread_mutex_t mutex;
};

// Internal helper functions
static void kalman_filter_init(KalmanFilter* kf, const BoundingBox* bbox);
static void kalman_filter_predict(KalmanFilter* kf);
static void kalman_filter_update(KalmanFilter* kf, const BoundingBox* bbox);
static void bbox_from_state(const float* state, BoundingBox* bbox);
static void bbox_to_measurement(const BoundingBox* bbox, float* measurement);
static float** hungarian_algorithm(float** cost_matrix, int rows, int cols);
static void create_cost_matrix(ObjectTracker* tracker,
                               const DetectedObject* detections,
                               uint32_t num_detections,
                               float*** cost_matrix,
                               uint32_t* matrix_rows,
                               uint32_t* matrix_cols);

// ============================================================================
// Public API Implementation
// ============================================================================

ObjectTracker* object_tracker_init(const TrackerConfig* config) {
    if (!config) {
        syslog(LOG_ERR, "[Tracker] Invalid configuration");
        return NULL;
    }

    ObjectTracker* tracker = calloc(1, sizeof(ObjectTracker));
    if (!tracker) {
        syslog(LOG_ERR, "[Tracker] Failed to allocate tracker");
        return NULL;
    }

    tracker->config = *config;
    tracker->tracks = calloc(config->max_tracks, sizeof(Track));
    if (!tracker->tracks) {
        syslog(LOG_ERR, "[Tracker] Failed to allocate tracks array");
        free(tracker);
        return NULL;
    }

    tracker->num_tracks = 0;
    tracker->next_id = 1;
    tracker->total_tracks_created = 0;

    if (pthread_mutex_init(&tracker->mutex, NULL) != 0) {
        syslog(LOG_ERR, "[Tracker] Failed to initialize mutex");
        free(tracker->tracks);
        free(tracker);
        return NULL;
    }

    syslog(LOG_INFO, "[Tracker] Initialized with max_tracks=%u, iou_threshold=%.2f",
           config->max_tracks, config->iou_threshold);

    return tracker;
}

void object_tracker_update(ObjectTracker* tracker,
                           const DetectedObject* detections,
                           uint32_t num_detections,
                           uint64_t timestamp_ms) {
    if (!tracker) {
        return;
    }

    pthread_mutex_lock(&tracker->mutex);

    // Step 1: Predict new locations for all tracks
    for (uint32_t i = 0; i < tracker->num_tracks; i++) {
        kalman_filter_predict(&tracker->tracks[i].kf);
        tracker->tracks[i].age++;
        tracker->tracks[i].time_since_update++;
    }

    // Step 2: Create cost matrix (IoU-based)
    float** cost_matrix = NULL;
    uint32_t matrix_rows = 0;
    uint32_t matrix_cols = 0;

    if (num_detections > 0 && tracker->num_tracks > 0) {
        create_cost_matrix(tracker, detections, num_detections,
                          &cost_matrix, &matrix_rows, &matrix_cols);
    }

    // Step 3: Solve assignment problem using Hungarian algorithm
    bool* matched_detections = calloc(num_detections, sizeof(bool));
    bool* matched_tracks = calloc(tracker->num_tracks, sizeof(bool));

    if (cost_matrix && matrix_rows > 0 && matrix_cols > 0) {
        float** assignment = hungarian_algorithm(cost_matrix, matrix_rows, matrix_cols);

        // Process assignments
        for (uint32_t i = 0; i < matrix_rows && i < tracker->num_tracks; i++) {
            for (uint32_t j = 0; j < matrix_cols && j < num_detections; j++) {
                if (assignment[i][j] == 1.0f) {
                    // Valid match - update track
                    kalman_filter_update(&tracker->tracks[i].kf, &detections[j].bbox);
                    tracker->tracks[i].hits++;
                    tracker->tracks[i].time_since_update = 0;
                    tracker->tracks[i].confidence = detections[j].confidence;
                    tracker->tracks[i].last_seen_ms = timestamp_ms;
                    tracker->tracks[i].class_id = detections[j].class_id;

                    // Update features (running average)
                    float alpha = tracker->config.velocity_smoothing;
                    for (uint32_t k = 0; k < 128; k++) {
                        tracker->tracks[i].features[k] =
                            alpha * detections[j].features[k] +
                            (1.0f - alpha) * tracker->tracks[i].features[k];
                    }

                    matched_detections[j] = true;
                    matched_tracks[i] = true;
                }
            }
            if (assignment[i]) {
                free(assignment[i]);
            }
        }
        if (assignment) {
            free(assignment);
        }
    }

    // Step 4: Create new tracks for unmatched detections
    for (uint32_t i = 0; i < num_detections; i++) {
        if (!matched_detections[i] && tracker->num_tracks < tracker->config.max_tracks) {
            // Create new track
            Track* new_track = &tracker->tracks[tracker->num_tracks];
            new_track->id = tracker->next_id++;
            new_track->class_id = detections[i].class_id;
            new_track->hits = 1;
            new_track->age = 1;
            new_track->time_since_update = 0;
            new_track->confidence = detections[i].confidence;
            new_track->first_seen_ms = timestamp_ms;
            new_track->last_seen_ms = timestamp_ms;

            // Initialize Kalman filter
            kalman_filter_init(&new_track->kf, &detections[i].bbox);

            // Copy features
            memcpy(new_track->features, detections[i].features, sizeof(new_track->features));

            tracker->num_tracks++;
            tracker->total_tracks_created++;

            syslog(LOG_DEBUG, "[Tracker] Created new track ID=%u, class=%d",
                   new_track->id, new_track->class_id);
        }
    }

    // Step 5: Delete old tracks
    uint32_t write_idx = 0;
    for (uint32_t i = 0; i < tracker->num_tracks; i++) {
        if (tracker->tracks[i].time_since_update < tracker->config.max_age) {
            // Keep track
            if (write_idx != i) {
                tracker->tracks[write_idx] = tracker->tracks[i];
            }
            write_idx++;
        } else {
            syslog(LOG_DEBUG, "[Tracker] Deleted track ID=%u (age=%u)",
                   tracker->tracks[i].id, tracker->tracks[i].age);
        }
    }
    tracker->num_tracks = write_idx;

    // Cleanup
    if (cost_matrix) {
        for (uint32_t i = 0; i < matrix_rows; i++) {
            free(cost_matrix[i]);
        }
        free(cost_matrix);
    }
    free(matched_detections);
    free(matched_tracks);

    pthread_mutex_unlock(&tracker->mutex);
}

uint32_t object_tracker_get_tracks(ObjectTracker* tracker,
                                   TrackedObject* tracks,
                                   uint32_t max_tracks) {
    if (!tracker || !tracks) {
        return 0;
    }

    pthread_mutex_lock(&tracker->mutex);

    uint32_t count = 0;
    for (uint32_t i = 0; i < tracker->num_tracks && count < max_tracks; i++) {
        Track* t = &tracker->tracks[i];

        // Only return confirmed tracks that are currently being tracked
        if (t->hits >= tracker->config.min_hits && t->time_since_update == 0) {
            TrackedObject* obj = &tracks[count];

            obj->track_id = t->id;
            obj->class_id = t->class_id;
            obj->confidence = t->confidence;
            obj->frame_count = t->age;
            obj->miss_count = t->time_since_update;
            obj->first_seen_ms = t->first_seen_ms;
            obj->last_seen_ms = t->last_seen_ms;

            // Current bounding box from Kalman state
            bbox_from_state(t->kf.state, &obj->current_bbox);

            // Predicted next position (one step ahead)
            KalmanFilter kf_copy = t->kf;
            kalman_filter_predict(&kf_copy);
            bbox_from_state(kf_copy.state, &obj->predicted_bbox);

            // Velocity from Kalman state
            obj->velocity_x = t->kf.state[4];  // vx
            obj->velocity_y = t->kf.state[5];  // vy

            // Features
            memcpy(obj->features, t->features, sizeof(obj->features));

            // Behavior flags (will be populated by behavior analysis module)
            obj->behaviors = BEHAVIOR_NORMAL;
            obj->threat_score = 0.0f;

            count++;
        }
    }

    pthread_mutex_unlock(&tracker->mutex);
    return count;
}

uint32_t object_tracker_get_all_tracks(ObjectTracker* tracker,
                                       TrackedObject* tracks,
                                       uint32_t max_tracks) {
    if (!tracker || !tracks) {
        return 0;
    }

    pthread_mutex_lock(&tracker->mutex);

    uint32_t count = 0;
    for (uint32_t i = 0; i < tracker->num_tracks && count < max_tracks; i++) {
        Track* t = &tracker->tracks[i];

        TrackedObject* obj = &tracks[count];

        obj->track_id = t->id;
        obj->class_id = t->class_id;
        obj->confidence = t->confidence;
        obj->frame_count = t->age;
        obj->miss_count = t->time_since_update;
        obj->first_seen_ms = t->first_seen_ms;
        obj->last_seen_ms = t->last_seen_ms;

        bbox_from_state(t->kf.state, &obj->current_bbox);

        KalmanFilter kf_copy = t->kf;
        kalman_filter_predict(&kf_copy);
        bbox_from_state(kf_copy.state, &obj->predicted_bbox);

        obj->velocity_x = t->kf.state[4];
        obj->velocity_y = t->kf.state[5];

        memcpy(obj->features, t->features, sizeof(obj->features));

        obj->behaviors = BEHAVIOR_NORMAL;
        obj->threat_score = 0.0f;

        count++;
    }

    pthread_mutex_unlock(&tracker->mutex);
    return count;
}

void object_tracker_reset(ObjectTracker* tracker) {
    if (!tracker) {
        return;
    }

    pthread_mutex_lock(&tracker->mutex);

    tracker->num_tracks = 0;
    tracker->next_id = 1;
    tracker->total_tracks_created = 0;

    syslog(LOG_INFO, "[Tracker] Reset complete");

    pthread_mutex_unlock(&tracker->mutex);
}

void object_tracker_get_stats(ObjectTracker* tracker,
                              uint32_t* total_tracks,
                              uint32_t* active_tracks,
                              uint32_t* confirmed_tracks) {
    if (!tracker) {
        return;
    }

    pthread_mutex_lock(&tracker->mutex);

    if (total_tracks) {
        *total_tracks = tracker->total_tracks_created;
    }

    if (active_tracks) {
        *active_tracks = tracker->num_tracks;
    }

    if (confirmed_tracks) {
        uint32_t confirmed = 0;
        for (uint32_t i = 0; i < tracker->num_tracks; i++) {
            if (tracker->tracks[i].hits >= tracker->config.min_hits) {
                confirmed++;
            }
        }
        *confirmed_tracks = confirmed;
    }

    pthread_mutex_unlock(&tracker->mutex);
}

void object_tracker_destroy(ObjectTracker* tracker) {
    if (!tracker) {
        return;
    }

    pthread_mutex_lock(&tracker->mutex);

    free(tracker->tracks);

    pthread_mutex_unlock(&tracker->mutex);
    pthread_mutex_destroy(&tracker->mutex);

    free(tracker);

    syslog(LOG_INFO, "[Tracker] Destroyed");
}

float bbox_iou(const BoundingBox* a, const BoundingBox* b) {
    if (!bbox_is_valid(a) || !bbox_is_valid(b)) {
        return 0.0f;
    }

    // Calculate intersection
    float x1 = fmaxf(a->x, b->x);
    float y1 = fmaxf(a->y, b->y);
    float x2 = fminf(a->x + a->width, b->x + b->width);
    float y2 = fminf(a->y + a->height, b->y + b->height);

    if (x2 < x1 || y2 < y1) {
        return 0.0f;  // No intersection
    }

    float intersection = (x2 - x1) * (y2 - y1);
    float area_a = bbox_area(a);
    float area_b = bbox_area(b);
    float union_area = area_a + area_b - intersection;

    if (union_area <= 0.0f) {
        return 0.0f;
    }

    return intersection / union_area;
}

// ============================================================================
// Kalman Filter Implementation
// ============================================================================

static void kalman_filter_init(KalmanFilter* kf, const BoundingBox* bbox) {
    memset(kf, 0, sizeof(KalmanFilter));

    // Initialize state: [x, y, s, r, vx, vy, vs]
    // x, y: center of bounding box
    // s: scale (area)
    // r: aspect ratio (width/height)
    // vx, vy, vs: velocities (initially 0)

    kf->state[0] = bbox->x + bbox->width / 2.0f;   // x (center)
    kf->state[1] = bbox->y + bbox->height / 2.0f;  // y (center)
    kf->state[2] = bbox->width * bbox->height;     // s (scale/area)
    kf->state[3] = bbox->width / (bbox->height + 1e-6f);  // r (aspect ratio)
    kf->state[4] = 0.0f;  // vx
    kf->state[5] = 0.0f;  // vy
    kf->state[6] = 0.0f;  // vs

    // Initialize covariance matrix (high uncertainty for velocities)
    for (int i = 0; i < STATE_DIM; i++) {
        for (int j = 0; j < STATE_DIM; j++) {
            kf->covariance[i][j] = (i == j) ? ((i < 4) ? 10.0f : 1000.0f) : 0.0f;
        }
    }

    kf->initialized = true;
}

static void kalman_filter_predict(KalmanFilter* kf) {
    if (!kf->initialized) {
        return;
    }

    // State transition: x' = x + vx, y' = y + vy, s' = s + vs
    kf->state[0] += kf->state[4];  // x += vx
    kf->state[1] += kf->state[5];  // y += vy
    kf->state[2] += kf->state[6];  // s += vs

    // Keep state within bounds
    kf->state[0] = fmaxf(0.0f, fminf(1.0f, kf->state[0]));
    kf->state[1] = fmaxf(0.0f, fminf(1.0f, kf->state[1]));
    kf->state[2] = fmaxf(0.001f, kf->state[2]);  // Minimum scale
    kf->state[3] = fmaxf(0.1f, fminf(10.0f, kf->state[3]));  // Aspect ratio limits

    // Process noise (increase uncertainty)
    for (int i = 0; i < STATE_DIM; i++) {
        kf->covariance[i][i] += (i < 4) ? 1.0f : 0.01f;
    }
}

static void kalman_filter_update(KalmanFilter* kf, const BoundingBox* bbox) {
    if (!kf->initialized || !bbox_is_valid(bbox)) {
        return;
    }

    // Measurement: [x, y, s, r]
    float measurement[MEAS_DIM];
    bbox_to_measurement(bbox, measurement);

    // Kalman gain (simplified - assumes high measurement confidence)
    float innovation[MEAS_DIM];
    for (int i = 0; i < MEAS_DIM; i++) {
        innovation[i] = measurement[i] - kf->state[i];
    }

    // Update state (weighted combination of prediction and measurement)
    float alpha = 0.7f;  // Measurement weight (higher = trust measurement more)
    kf->state[0] += alpha * innovation[0];  // x
    kf->state[1] += alpha * innovation[1];  // y
    kf->state[2] += alpha * innovation[2];  // s
    kf->state[3] += alpha * innovation[3];  // r

    // Update velocities based on innovation
    kf->state[4] = 0.3f * kf->state[4] + 0.7f * innovation[0];  // vx
    kf->state[5] = 0.3f * kf->state[5] + 0.7f * innovation[1];  // vy
    kf->state[6] = 0.3f * kf->state[6] + 0.7f * innovation[2];  // vs

    // Reduce uncertainty (measurement update)
    for (int i = 0; i < MEAS_DIM; i++) {
        kf->covariance[i][i] *= 0.5f;
    }
}

static void bbox_from_state(const float* state, BoundingBox* bbox) {
    float center_x = state[0];
    float center_y = state[1];
    float scale = state[2];
    float aspect_ratio = state[3];

    // Calculate width and height from scale and aspect ratio
    float height = sqrtf(scale / (aspect_ratio + 1e-6f));
    float width = aspect_ratio * height;

    bbox->x = center_x - width / 2.0f;
    bbox->y = center_y - height / 2.0f;
    bbox->width = width;
    bbox->height = height;

    // Clamp to [0, 1]
    bbox->x = fmaxf(0.0f, fminf(1.0f, bbox->x));
    bbox->y = fmaxf(0.0f, fminf(1.0f, bbox->y));
    bbox->width = fmaxf(0.01f, fminf(1.0f - bbox->x, bbox->width));
    bbox->height = fmaxf(0.01f, fminf(1.0f - bbox->y, bbox->height));
}

static void bbox_to_measurement(const BoundingBox* bbox, float* measurement) {
    measurement[0] = bbox->x + bbox->width / 2.0f;   // x (center)
    measurement[1] = bbox->y + bbox->height / 2.0f;  // y (center)
    measurement[2] = bbox->width * bbox->height;     // s (scale/area)
    measurement[3] = bbox->width / (bbox->height + 1e-6f);  // r (aspect ratio)
}

// ============================================================================
// Hungarian Algorithm (Simplified for SORT)
// ============================================================================

static float** hungarian_algorithm(float** cost_matrix, int rows, int cols) {
    // Simplified Hungarian algorithm for SORT tracking
    // Uses greedy assignment with IoU threshold

    float** assignment = calloc(rows, sizeof(float*));
    for (int i = 0; i < rows; i++) {
        assignment[i] = calloc(cols, sizeof(float));
    }

    bool* assigned_rows = calloc(rows, sizeof(bool));
    bool* assigned_cols = calloc(cols, sizeof(bool));

    // Greedy assignment: find best matches first
    for (int iter = 0; iter < rows && iter < cols; iter++) {
        float max_iou = -1.0f;
        int best_row = -1;
        int best_col = -1;

        // Find highest IoU unassigned pair
        for (int i = 0; i < rows; i++) {
            if (assigned_rows[i]) continue;

            for (int j = 0; j < cols; j++) {
                if (assigned_cols[j]) continue;

                if (cost_matrix[i][j] > max_iou) {
                    max_iou = cost_matrix[i][j];
                    best_row = i;
                    best_col = j;
                }
            }
        }

        // Assign if above threshold
        if (max_iou >= 0.3f && best_row >= 0 && best_col >= 0) {
            assignment[best_row][best_col] = 1.0f;
            assigned_rows[best_row] = true;
            assigned_cols[best_col] = true;
        } else {
            break;  // No more valid matches
        }
    }

    free(assigned_rows);
    free(assigned_cols);

    return assignment;
}

static void create_cost_matrix(ObjectTracker* tracker,
                               const DetectedObject* detections,
                               uint32_t num_detections,
                               float*** cost_matrix,
                               uint32_t* matrix_rows,
                               uint32_t* matrix_cols) {
    *matrix_rows = tracker->num_tracks;
    *matrix_cols = num_detections;

    *cost_matrix = malloc(tracker->num_tracks * sizeof(float*));
    for (uint32_t i = 0; i < tracker->num_tracks; i++) {
        (*cost_matrix)[i] = malloc(num_detections * sizeof(float));

        // Get predicted bbox from track
        BoundingBox track_bbox;
        bbox_from_state(tracker->tracks[i].kf.state, &track_bbox);

        // Calculate IoU with each detection
        for (uint32_t j = 0; j < num_detections; j++) {
            (*cost_matrix)[i][j] = bbox_iou(&track_bbox, &detections[j].bbox);
        }
    }
}
