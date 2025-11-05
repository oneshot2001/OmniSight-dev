/**
 * @file object_tracking.h
 * @brief OMNISIGHT Object Tracking - SORT Algorithm Implementation
 *
 * Simple Online and Realtime Tracking (SORT) with Kalman filtering
 * for multi-object tracking across video frames.
 *
 * Tracks detected objects to generate persistent IDs and motion vectors.
 *
 * Copyright (C) 2025 OMNISIGHT
 */

#ifndef OMNISIGHT_OBJECT_TRACKING_H
#define OMNISIGHT_OBJECT_TRACKING_H

#include "perception.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// Forward declarations
typedef struct ObjectTracker ObjectTracker;

/**
 * Tracker configuration
 */
typedef struct {
    uint32_t max_tracks;              // Maximum simultaneous tracks
    float iou_threshold;              // IoU threshold for matching (0.3)
    uint32_t max_age;                 // Frames to keep track without detection (30)
    uint32_t min_hits;                // Hits required to confirm track (3)
    float velocity_smoothing;         // Velocity smoothing factor (0.3)
} TrackerConfig;

/**
 * Initialize object tracker
 *
 * @param config Tracker configuration
 * @return Pointer to initialized tracker, NULL on failure
 */
ObjectTracker* object_tracker_init(const TrackerConfig* config);

/**
 * Update tracker with new detections
 *
 * Associates detections with existing tracks using IoU matching
 * and Hungarian algorithm. Creates new tracks for unmatched detections.
 *
 * @param tracker Object tracker instance
 * @param detections Array of detected objects
 * @param num_detections Number of detections
 * @param timestamp_ms Current timestamp in milliseconds
 */
void object_tracker_update(ObjectTracker* tracker,
                           const DetectedObject* detections,
                           uint32_t num_detections,
                           uint64_t timestamp_ms);

/**
 * Get currently active tracks
 *
 * Returns only tracks that have been confirmed (min_hits reached)
 * and are still being detected (age < max_age).
 *
 * @param tracker Object tracker instance
 * @param tracks Output array for tracked objects (caller-allocated)
 * @param max_tracks Maximum tracks to return
 * @return Number of active tracks
 */
uint32_t object_tracker_get_tracks(ObjectTracker* tracker,
                                   TrackedObject* tracks,
                                   uint32_t max_tracks);

/**
 * Get all tracks including tentative and lost
 *
 * @param tracker Object tracker instance
 * @param tracks Output array for tracked objects (caller-allocated)
 * @param max_tracks Maximum tracks to return
 * @return Number of total tracks
 */
uint32_t object_tracker_get_all_tracks(ObjectTracker* tracker,
                                       TrackedObject* tracks,
                                       uint32_t max_tracks);

/**
 * Reset tracker
 *
 * Deletes all existing tracks and resets track ID counter
 *
 * @param tracker Object tracker instance
 */
void object_tracker_reset(ObjectTracker* tracker);

/**
 * Get tracker statistics
 *
 * @param tracker Object tracker instance
 * @param total_tracks Total tracks created since initialization
 * @param active_tracks Currently active tracks
 * @param confirmed_tracks Confirmed tracks (hit min_hits threshold)
 */
void object_tracker_get_stats(ObjectTracker* tracker,
                              uint32_t* total_tracks,
                              uint32_t* active_tracks,
                              uint32_t* confirmed_tracks);

/**
 * Destroy tracker and free resources
 *
 * @param tracker Object tracker instance
 */
void object_tracker_destroy(ObjectTracker* tracker);

/**
 * Calculate Intersection over Union (IoU) between two bounding boxes
 *
 * @param a First bounding box
 * @param b Second bounding box
 * @return IoU value [0.0, 1.0]
 */
float bbox_iou(const BoundingBox* a, const BoundingBox* b);

/**
 * Calculate area of bounding box
 *
 * @param bbox Bounding box
 * @return Area in normalized coordinates
 */
static inline float bbox_area(const BoundingBox* bbox) {
    return bbox->width * bbox->height;
}

/**
 * Check if bounding box is valid
 *
 * @param bbox Bounding box to validate
 * @return true if valid, false otherwise
 */
static inline bool bbox_is_valid(const BoundingBox* bbox) {
    return (bbox->width > 0.0f && bbox->height > 0.0f &&
            bbox->x >= 0.0f && bbox->y >= 0.0f &&
            bbox->x + bbox->width <= 1.0f &&
            bbox->y + bbox->height <= 1.0f);
}

#ifdef __cplusplus
}
#endif

#endif // OMNISIGHT_OBJECT_TRACKING_H
