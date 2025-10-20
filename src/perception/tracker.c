/**
 * @file tracker.c
 * @brief Multi-object tracking implementation for OMNISIGHT
 *
 * Implements IoU-based tracking with Kalman filtering
 */

#include "tracker.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <float.h>

#define MAX_TRACKS 100
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

/**
 * Kalman filter state for single track
 */
typedef struct {
    float x, y, w, h;      // Position and size
    float vx, vy;          // Velocity
    float P[6][6];         // Covariance matrix
} KalmanState;

/**
 * Internal track representation
 */
typedef struct {
    uint32_t track_id;
    ObjectClass class_id;
    BoundingBox bbox;
    BoundingBox predicted_bbox;
    float velocity_x;
    float velocity_y;
    float confidence;
    uint32_t hits;           // Number of detections matched
    uint32_t miss_count;
    uint32_t frame_count;
    BehaviorFlags behaviors;
    float threat_score;
    uint64_t first_seen_ms;
    uint64_t last_seen_ms;
    float features[128];
    KalmanState kalman;
    bool active;
} InternalTrack;

/**
 * Tracker structure
 */
struct Tracker {
    TrackerConfig config;
    InternalTrack tracks[MAX_TRACKS];
    uint32_t next_track_id;
    uint64_t total_tracks_created;
    uint64_t total_tracks_lost;
    uint32_t active_count;
};

// Forward declarations
static void kalman_init(KalmanState* k, const BoundingBox* bbox);
static void kalman_predict(KalmanState* k);
static void kalman_update(KalmanState* k, const BoundingBox* bbox);
static void kalman_get_state(const KalmanState* k, BoundingBox* bbox, float* vx, float* vy);

// ============================================================================
// Public API Implementation
// ============================================================================

Tracker* tracker_init(const TrackerConfig* config) {
    if (!config) {
        return NULL;
    }

    Tracker* tracker = (Tracker*)calloc(1, sizeof(Tracker));
    if (!tracker) {
        return NULL;
    }

    tracker->config = *config;
    tracker->next_track_id = 1;
    tracker->total_tracks_created = 0;
    tracker->total_tracks_lost = 0;
    tracker->active_count = 0;

    // Initialize all tracks as inactive
    for (uint32_t i = 0; i < MAX_TRACKS; i++) {
        tracker->tracks[i].active = false;
    }

    return tracker;
}

uint32_t tracker_update(
    Tracker* tracker,
    const DetectedObject* detections,
    uint32_t num_detections,
    TrackedObject* tracks,
    uint32_t max_tracks
) {
    if (!tracker || !detections || !tracks) {
        return 0;
    }

    // Step 1: Predict positions for all active tracks
    for (uint32_t i = 0; i < MAX_TRACKS; i++) {
        if (!tracker->tracks[i].active) continue;

        if (tracker->config.use_kalman_filter) {
            kalman_predict(&tracker->tracks[i].kalman);
            kalman_get_state(
                &tracker->tracks[i].kalman,
                &tracker->tracks[i].predicted_bbox,
                &tracker->tracks[i].velocity_x,
                &tracker->tracks[i].velocity_y
            );
        } else {
            // Simple linear prediction
            tracker->tracks[i].predicted_bbox = tracker->tracks[i].bbox;
            tracker->tracks[i].predicted_bbox.x += tracker->tracks[i].velocity_x;
            tracker->tracks[i].predicted_bbox.y += tracker->tracks[i].velocity_y;
        }
    }

    // Step 2: Compute IoU matrix between predictions and detections
    float iou_matrix[MAX_TRACKS][num_detections];
    bool matched_detection[num_detections];
    bool matched_track[MAX_TRACKS];

    memset(matched_detection, 0, sizeof(matched_detection));
    memset(matched_track, 0, sizeof(matched_track));

    for (uint32_t i = 0; i < MAX_TRACKS; i++) {
        if (!tracker->tracks[i].active) continue;

        for (uint32_t j = 0; j < num_detections; j++) {
            float iou = tracker_calculate_iou(
                &tracker->tracks[i].predicted_bbox,
                &detections[j].bbox
            );

            // Add feature similarity if available
            if (tracker->config.feature_similarity_weight > 0.0f) {
                float feature_sim = tracker_calculate_feature_similarity(
                    tracker->tracks[i].features,
                    detections[j].features,
                    128
                );
                iou = iou * (1.0f - tracker->config.feature_similarity_weight) +
                      feature_sim * tracker->config.feature_similarity_weight;
            }

            iou_matrix[i][j] = iou;
        }
    }

    // Step 3: Match tracks to detections (greedy assignment)
    for (uint32_t iter = 0; iter < num_detections; iter++) {
        float best_iou = tracker->config.iou_threshold;
        int best_track = -1;
        int best_detection = -1;

        for (uint32_t i = 0; i < MAX_TRACKS; i++) {
            if (!tracker->tracks[i].active || matched_track[i]) continue;

            for (uint32_t j = 0; j < num_detections; j++) {
                if (matched_detection[j]) continue;

                if (iou_matrix[i][j] > best_iou) {
                    best_iou = iou_matrix[i][j];
                    best_track = i;
                    best_detection = j;
                }
            }
        }

        if (best_track >= 0 && best_detection >= 0) {
            // Update matched track
            InternalTrack* track = &tracker->tracks[best_track];
            const DetectedObject* det = &detections[best_detection];

            track->bbox = det->bbox;
            track->class_id = det->class_id;
            track->confidence = det->confidence;
            track->hits++;
            track->miss_count = 0;
            track->frame_count++;
            track->last_seen_ms = det->timestamp_ms;

            // Update Kalman filter
            if (tracker->config.use_kalman_filter) {
                kalman_update(&track->kalman, &det->bbox);
                kalman_get_state(
                    &track->kalman,
                    &track->bbox,
                    &track->velocity_x,
                    &track->velocity_y
                );
            } else {
                // Simple velocity update
                track->velocity_x = det->bbox.x - track->bbox.x;
                track->velocity_y = det->bbox.y - track->bbox.y;
            }

            // Update features (exponential moving average)
            for (int k = 0; k < 128; k++) {
                track->features[k] = 0.9f * track->features[k] + 0.1f * det->features[k];
            }

            matched_track[best_track] = true;
            matched_detection[best_detection] = true;
        } else {
            break;  // No more matches possible
        }
    }

    // Step 4: Handle unmatched tracks (increase miss count)
    for (uint32_t i = 0; i < MAX_TRACKS; i++) {
        if (!tracker->tracks[i].active) continue;

        if (!matched_track[i]) {
            tracker->tracks[i].miss_count++;

            // Delete track if too many misses
            if (tracker->tracks[i].miss_count >= tracker->config.max_age) {
                tracker->tracks[i].active = false;
                tracker->active_count--;
                tracker->total_tracks_lost++;
            }
        }
    }

    // Step 5: Create new tracks for unmatched detections
    for (uint32_t j = 0; j < num_detections; j++) {
        if (matched_detection[j]) continue;

        // Find free slot
        int free_slot = -1;
        for (uint32_t i = 0; i < MAX_TRACKS; i++) {
            if (!tracker->tracks[i].active) {
                free_slot = i;
                break;
            }
        }

        if (free_slot >= 0 && tracker->active_count < tracker->config.max_tracks) {
            InternalTrack* track = &tracker->tracks[free_slot];
            const DetectedObject* det = &detections[j];

            track->track_id = tracker->next_track_id++;
            track->class_id = det->class_id;
            track->bbox = det->bbox;
            track->predicted_bbox = det->bbox;
            track->velocity_x = 0.0f;
            track->velocity_y = 0.0f;
            track->confidence = det->confidence;
            track->hits = 1;
            track->miss_count = 0;
            track->frame_count = 1;
            track->behaviors = BEHAVIOR_NORMAL;
            track->threat_score = 0.0f;
            track->first_seen_ms = det->timestamp_ms;
            track->last_seen_ms = det->timestamp_ms;
            memcpy(track->features, det->features, sizeof(track->features));
            track->active = true;

            if (tracker->config.use_kalman_filter) {
                kalman_init(&track->kalman, &det->bbox);
            }

            tracker->active_count++;
            tracker->total_tracks_created++;
        }
    }

    // Step 6: Export confirmed tracks (hits >= min_hits)
    uint32_t output_count = 0;
    for (uint32_t i = 0; i < MAX_TRACKS && output_count < max_tracks; i++) {
        if (!tracker->tracks[i].active) continue;
        if (tracker->tracks[i].hits < tracker->config.min_hits) continue;

        InternalTrack* track = &tracker->tracks[i];
        TrackedObject* out = &tracks[output_count++];

        out->track_id = track->track_id;
        out->class_id = track->class_id;
        out->current_bbox = track->bbox;
        out->predicted_bbox = track->predicted_bbox;
        out->velocity_x = track->velocity_x;
        out->velocity_y = track->velocity_y;
        out->confidence = track->confidence;
        out->frame_count = track->frame_count;
        out->miss_count = track->miss_count;
        out->behaviors = track->behaviors;
        out->threat_score = track->threat_score;
        out->first_seen_ms = track->first_seen_ms;
        out->last_seen_ms = track->last_seen_ms;
        memcpy(out->features, track->features, sizeof(out->features));
    }

    return output_count;
}

uint32_t tracker_get_tracks(
    Tracker* tracker,
    TrackedObject* tracks,
    uint32_t max_tracks
) {
    if (!tracker || !tracks) {
        return 0;
    }

    uint32_t count = 0;
    for (uint32_t i = 0; i < MAX_TRACKS && count < max_tracks; i++) {
        if (!tracker->tracks[i].active) continue;
        if (tracker->tracks[i].hits < tracker->config.min_hits) continue;

        InternalTrack* track = &tracker->tracks[i];
        TrackedObject* out = &tracks[count++];

        out->track_id = track->track_id;
        out->class_id = track->class_id;
        out->current_bbox = track->bbox;
        out->predicted_bbox = track->predicted_bbox;
        out->velocity_x = track->velocity_x;
        out->velocity_y = track->velocity_y;
        out->confidence = track->confidence;
        out->frame_count = track->frame_count;
        out->miss_count = track->miss_count;
        out->behaviors = track->behaviors;
        out->threat_score = track->threat_score;
        out->first_seen_ms = track->first_seen_ms;
        out->last_seen_ms = track->last_seen_ms;
        memcpy(out->features, track->features, sizeof(out->features));
    }

    return count;
}

bool tracker_get_track_by_id(
    Tracker* tracker,
    uint32_t track_id,
    TrackedObject* track
) {
    if (!tracker || !track) {
        return false;
    }

    for (uint32_t i = 0; i < MAX_TRACKS; i++) {
        if (tracker->tracks[i].active && tracker->tracks[i].track_id == track_id) {
            InternalTrack* t = &tracker->tracks[i];

            track->track_id = t->track_id;
            track->class_id = t->class_id;
            track->current_bbox = t->bbox;
            track->predicted_bbox = t->predicted_bbox;
            track->velocity_x = t->velocity_x;
            track->velocity_y = t->velocity_y;
            track->confidence = t->confidence;
            track->frame_count = t->frame_count;
            track->miss_count = t->miss_count;
            track->behaviors = t->behaviors;
            track->threat_score = t->threat_score;
            track->first_seen_ms = t->first_seen_ms;
            track->last_seen_ms = t->last_seen_ms;
            memcpy(track->features, t->features, sizeof(track->features));

            return true;
        }
    }

    return false;
}

bool tracker_remove_track(Tracker* tracker, uint32_t track_id) {
    if (!tracker) {
        return false;
    }

    for (uint32_t i = 0; i < MAX_TRACKS; i++) {
        if (tracker->tracks[i].active && tracker->tracks[i].track_id == track_id) {
            tracker->tracks[i].active = false;
            tracker->active_count--;
            return true;
        }
    }

    return false;
}

void tracker_clear(Tracker* tracker) {
    if (!tracker) {
        return;
    }

    for (uint32_t i = 0; i < MAX_TRACKS; i++) {
        tracker->tracks[i].active = false;
    }

    tracker->active_count = 0;
}

void tracker_get_stats(
    Tracker* tracker,
    uint32_t* active_tracks,
    uint64_t* total_tracks,
    uint64_t* lost_tracks
) {
    if (!tracker) {
        return;
    }

    if (active_tracks) *active_tracks = tracker->active_count;
    if (total_tracks) *total_tracks = tracker->total_tracks_created;
    if (lost_tracks) *lost_tracks = tracker->total_tracks_lost;
}

void tracker_destroy(Tracker* tracker) {
    if (tracker) {
        free(tracker);
    }
}

float tracker_calculate_iou(const BoundingBox* box1, const BoundingBox* box2) {
    if (!box1 || !box2) {
        return 0.0f;
    }

    float x1_min = box1->x;
    float y1_min = box1->y;
    float x1_max = box1->x + box1->width;
    float y1_max = box1->y + box1->height;

    float x2_min = box2->x;
    float y2_min = box2->y;
    float x2_max = box2->x + box2->width;
    float y2_max = box2->y + box2->height;

    // Calculate intersection
    float inter_x_min = MAX(x1_min, x2_min);
    float inter_y_min = MAX(y1_min, y2_min);
    float inter_x_max = MIN(x1_max, x2_max);
    float inter_y_max = MIN(y1_max, y2_max);

    if (inter_x_max <= inter_x_min || inter_y_max <= inter_y_min) {
        return 0.0f;  // No intersection
    }

    float inter_area = (inter_x_max - inter_x_min) * (inter_y_max - inter_y_min);

    // Calculate union
    float area1 = box1->width * box1->height;
    float area2 = box2->width * box2->height;
    float union_area = area1 + area2 - inter_area;

    if (union_area <= 0.0f) {
        return 0.0f;
    }

    return inter_area / union_area;
}

float tracker_calculate_feature_similarity(
    const float* features1,
    const float* features2,
    uint32_t feature_dim
) {
    if (!features1 || !features2 || feature_dim == 0) {
        return 0.0f;
    }

    // Cosine similarity
    float dot_product = 0.0f;
    float norm1 = 0.0f;
    float norm2 = 0.0f;

    for (uint32_t i = 0; i < feature_dim; i++) {
        dot_product += features1[i] * features2[i];
        norm1 += features1[i] * features1[i];
        norm2 += features2[i] * features2[i];
    }

    norm1 = sqrtf(norm1);
    norm2 = sqrtf(norm2);

    if (norm1 < 1e-6f || norm2 < 1e-6f) {
        return 0.0f;
    }

    float similarity = dot_product / (norm1 * norm2);

    // Clamp to [0, 1]
    return MAX(0.0f, MIN(1.0f, (similarity + 1.0f) / 2.0f));
}

// ============================================================================
// Kalman Filter Implementation
// ============================================================================

static void kalman_init(KalmanState* k, const BoundingBox* bbox) {
    memset(k, 0, sizeof(KalmanState));

    k->x = bbox->x + bbox->width / 2.0f;
    k->y = bbox->y + bbox->height / 2.0f;
    k->w = bbox->width;
    k->h = bbox->height;
    k->vx = 0.0f;
    k->vy = 0.0f;

    // Initialize covariance matrix (simplified)
    for (int i = 0; i < 6; i++) {
        for (int j = 0; j < 6; j++) {
            k->P[i][j] = (i == j) ? 10.0f : 0.0f;
        }
    }
}

static void kalman_predict(KalmanState* k) {
    // Simple constant velocity model
    k->x += k->vx;
    k->y += k->vy;

    // Increase uncertainty
    for (int i = 0; i < 6; i++) {
        k->P[i][i] += 0.1f;
    }
}

static void kalman_update(KalmanState* k, const BoundingBox* bbox) {
    float measured_x = bbox->x + bbox->width / 2.0f;
    float measured_y = bbox->y + bbox->height / 2.0f;
    float measured_w = bbox->width;
    float measured_h = bbox->height;

    // Kalman gain (simplified, assuming measurement noise is low)
    float K = 0.5f;

    // Update state
    k->x = k->x + K * (measured_x - k->x);
    k->y = k->y + K * (measured_y - k->y);
    k->w = k->w + K * (measured_w - k->w);
    k->h = k->h + K * (measured_h - k->h);

    // Update velocity
    k->vx = measured_x - k->x;
    k->vy = measured_y - k->y;

    // Reduce uncertainty
    for (int i = 0; i < 6; i++) {
        k->P[i][i] *= (1.0f - K);
    }
}

static void kalman_get_state(const KalmanState* k, BoundingBox* bbox, float* vx, float* vy) {
    bbox->x = k->x - k->w / 2.0f;
    bbox->y = k->y - k->h / 2.0f;
    bbox->width = k->w;
    bbox->height = k->h;

    if (vx) *vx = k->vx;
    if (vy) *vy = k->vy;
}
