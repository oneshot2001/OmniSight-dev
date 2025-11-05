/**
 * @file behavior_analysis.h
 * @brief OMNISIGHT Behavior Analysis - Threat Detection and Scoring
 *
 * Analyzes tracked object behaviors to identify suspicious patterns:
 * - Loitering (extended dwell time in area)
 * - Running (high velocity movement)
 * - Unusual movement patterns
 * - Trajectory anomalies
 *
 * Copyright (C) 2025 OMNISIGHT
 */

#ifndef OMNISIGHT_BEHAVIOR_ANALYSIS_H
#define OMNISIGHT_BEHAVIOR_ANALYSIS_H

#include "perception.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// Forward declarations
typedef struct BehaviorAnalyzer BehaviorAnalyzer;

/**
 * Position history entry for trajectory analysis
 */
typedef struct {
    float x;                    // Normalized X position [0,1]
    float y;                    // Normalized Y position [0,1]
    uint64_t timestamp_ms;      // Timestamp
    float velocity;             // Instantaneous velocity
} PositionHistory;

/**
 * Track history for behavior analysis
 */
typedef struct {
    uint32_t track_id;
    PositionHistory positions[60];  // Last 60 positions (6 seconds @ 10 FPS)
    uint32_t num_positions;
    uint64_t first_seen_ms;
    uint64_t last_update_ms;
} TrackHistory;

/**
 * Behavior analyzer configuration
 */
typedef struct {
    // Loitering detection
    float loitering_dwell_time_ms;      // Dwell time threshold (default: 30000ms)
    float loitering_velocity_threshold; // Max velocity for loitering (default: 0.5 m/s)
    float loitering_radius_meters;      // Area radius for dwell (default: 2.0m)

    // Running detection
    float running_velocity_threshold;   // Min velocity for running (default: 3.0 m/s)
    float running_duration_ms;          // Sustained duration required (default: 1000ms)

    // Unusual movement
    float zigzag_threshold;             // Direction change threshold (default: 45 degrees)
    uint32_t zigzag_count_threshold;    // Number of changes for unusual (default: 5)

    // Zone analysis (for future use)
    bool enable_zone_analysis;
    uint32_t num_zones;

    // Calibration for camera (meters per normalized unit)
    float meters_per_normalized_unit;   // Scene calibration (default: 10.0m)

    // Threat scoring weights
    float weight_loitering;             // Weight for loitering (default: 0.3)
    float weight_running;               // Weight for running (default: 0.4)
    float weight_unusual_movement;      // Weight for unusual movement (default: 0.5)
    float weight_dwell_time;            // Weight for extended presence (default: 0.2)
} BehaviorAnalyzerConfig;

/**
 * Initialize behavior analyzer
 *
 * @param config Analyzer configuration
 * @return Pointer to initialized analyzer, NULL on failure
 */
BehaviorAnalyzer* behavior_analyzer_init(const BehaviorAnalyzerConfig* config);

/**
 * Analyze tracked object behavior
 *
 * Updates behavior flags and threat score based on track history.
 *
 * @param analyzer Behavior analyzer instance
 * @param track Tracked object to analyze (modified in-place)
 */
void behavior_analyzer_analyze(BehaviorAnalyzer* analyzer,
                               TrackedObject* track);

/**
 * Update track history with new position
 *
 * Maintains rolling window of positions for trajectory analysis.
 *
 * @param analyzer Behavior analyzer instance
 * @param track_id Track identifier
 * @param bbox Current bounding box
 * @param timestamp_ms Current timestamp
 */
void behavior_analyzer_update_history(BehaviorAnalyzer* analyzer,
                                      uint32_t track_id,
                                      const BoundingBox* bbox,
                                      uint64_t timestamp_ms);

/**
 * Get track history for a specific track
 *
 * @param analyzer Behavior analyzer instance
 * @param track_id Track identifier
 * @param history Output track history (caller-allocated)
 * @return true if history found, false otherwise
 */
bool behavior_analyzer_get_history(BehaviorAnalyzer* analyzer,
                                   uint32_t track_id,
                                   TrackHistory* history);

/**
 * Clear history for a specific track
 *
 * Called when track is deleted from tracker.
 *
 * @param analyzer Behavior analyzer instance
 * @param track_id Track identifier
 */
void behavior_analyzer_clear_history(BehaviorAnalyzer* analyzer,
                                     uint32_t track_id);

/**
 * Reset all track histories
 *
 * @param analyzer Behavior analyzer instance
 */
void behavior_analyzer_reset(BehaviorAnalyzer* analyzer);

/**
 * Get analyzer statistics
 *
 * @param analyzer Behavior analyzer instance
 * @param total_tracks_analyzed Total tracks analyzed
 * @param loitering_detections Loitering detections count
 * @param running_detections Running detections count
 * @param unusual_movement_detections Unusual movement detections count
 */
void behavior_analyzer_get_stats(BehaviorAnalyzer* analyzer,
                                 uint32_t* total_tracks_analyzed,
                                 uint32_t* loitering_detections,
                                 uint32_t* running_detections,
                                 uint32_t* unusual_movement_detections);

/**
 * Destroy behavior analyzer
 *
 * @param analyzer Behavior analyzer instance
 */
void behavior_analyzer_destroy(BehaviorAnalyzer* analyzer);

/**
 * Calculate Euclidean distance between two positions
 *
 * @param x1 First position X
 * @param y1 First position Y
 * @param x2 Second position X
 * @param y2 Second position Y
 * @return Distance in normalized coordinates
 */
static inline float position_distance(float x1, float y1, float x2, float y2) {
    float dx = x2 - x1;
    float dy = y2 - y1;
    return sqrtf(dx * dx + dy * dy);
}

/**
 * Calculate velocity magnitude
 *
 * @param vx Velocity X component
 * @param vy Velocity Y component
 * @return Velocity magnitude
 */
static inline float velocity_magnitude(float vx, float vy) {
    return sqrtf(vx * vx + vy * vy);
}

/**
 * Convert normalized coordinates to meters
 *
 * @param normalized_distance Distance in normalized coordinates [0,1]
 * @param meters_per_unit Calibration factor
 * @return Distance in meters
 */
static inline float normalized_to_meters(float normalized_distance,
                                         float meters_per_unit) {
    return normalized_distance * meters_per_unit;
}

#ifdef __cplusplus
}
#endif

#endif // OMNISIGHT_BEHAVIOR_ANALYSIS_H
