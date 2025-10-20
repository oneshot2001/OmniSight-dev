/**
 * @file tracker.h
 * @brief Multi-object tracking for OMNISIGHT
 *
 * Implements tracking algorithm to maintain object identities across frames
 * Uses IoU-based matching with Kalman filtering for prediction
 */

#ifndef OMNISIGHT_TRACKER_H
#define OMNISIGHT_TRACKER_H

#include "perception.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Tracker Tracker;

/**
 * Tracker configuration
 */
typedef struct {
    float iou_threshold;          // IoU threshold for matching (0.3-0.5)
    uint32_t max_age;             // Frames before deleting lost track
    uint32_t min_hits;            // Min detections before confirming track
    uint32_t max_tracks;          // Maximum simultaneous tracks
    bool use_kalman_filter;       // Enable Kalman filtering
    float feature_similarity_weight; // Weight of feature similarity (0-1)
} TrackerConfig;

/**
 * Initialize tracker
 *
 * @param config Tracker configuration
 * @return Tracker instance, NULL on failure
 */
Tracker* tracker_init(const TrackerConfig* config);

/**
 * Update tracker with new detections
 *
 * @param tracker Tracker instance
 * @param detections Array of detected objects
 * @param num_detections Number of detections
 * @param tracks Output array for tracked objects
 * @param max_tracks Maximum tracks to return
 * @return Number of active tracks
 */
uint32_t tracker_update(
    Tracker* tracker,
    const DetectedObject* detections,
    uint32_t num_detections,
    TrackedObject* tracks,
    uint32_t max_tracks
);

/**
 * Get all active tracks
 *
 * @param tracker Tracker instance
 * @param tracks Output array for tracked objects
 * @param max_tracks Maximum tracks to return
 * @return Number of active tracks
 */
uint32_t tracker_get_tracks(
    Tracker* tracker,
    TrackedObject* tracks,
    uint32_t max_tracks
);

/**
 * Get specific track by ID
 *
 * @param tracker Tracker instance
 * @param track_id Track ID to retrieve
 * @param track Output tracked object
 * @return true if track exists, false otherwise
 */
bool tracker_get_track_by_id(
    Tracker* tracker,
    uint32_t track_id,
    TrackedObject* track
);

/**
 * Remove a track by ID
 *
 * @param tracker Tracker instance
 * @param track_id Track ID to remove
 * @return true if track was removed, false if not found
 */
bool tracker_remove_track(Tracker* tracker, uint32_t track_id);

/**
 * Clear all tracks
 *
 * @param tracker Tracker instance
 */
void tracker_clear(Tracker* tracker);

/**
 * Get tracker statistics
 *
 * @param tracker Tracker instance
 * @param active_tracks Number of active tracks
 * @param total_tracks Total tracks created
 * @param lost_tracks Number of lost tracks
 */
void tracker_get_stats(
    Tracker* tracker,
    uint32_t* active_tracks,
    uint64_t* total_tracks,
    uint64_t* lost_tracks
);

/**
 * Destroy tracker and free resources
 *
 * @param tracker Tracker instance
 */
void tracker_destroy(Tracker* tracker);

/**
 * Calculate IoU (Intersection over Union) between two bounding boxes
 *
 * @param box1 First bounding box
 * @param box2 Second bounding box
 * @return IoU value [0.0, 1.0]
 */
float tracker_calculate_iou(const BoundingBox* box1, const BoundingBox* box2);

/**
 * Calculate feature similarity between two feature vectors
 *
 * Uses cosine similarity
 *
 * @param features1 First feature vector
 * @param features2 Second feature vector
 * @param feature_dim Feature vector dimension
 * @return Similarity score [0.0, 1.0]
 */
float tracker_calculate_feature_similarity(
    const float* features1,
    const float* features2,
    uint32_t feature_dim
);

#ifdef __cplusplus
}
#endif

#endif // OMNISIGHT_TRACKER_H
