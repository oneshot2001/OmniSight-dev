/**
 * @file behavior.h
 * @brief Behavior analysis for OMNISIGHT
 *
 * Analyzes tracked objects to detect suspicious behaviors:
 * - Loitering
 * - Running
 * - Concealing objects
 * - Repeated passes
 * - Extended observation
 *
 * Calculates threat scores for Timeline Threading™
 */

#ifndef OMNISIGHT_BEHAVIOR_H
#define OMNISIGHT_BEHAVIOR_H

#include "perception.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct BehaviorAnalyzer BehaviorAnalyzer;

/**
 * Behavior analysis configuration
 */
typedef struct {
    // Loitering detection
    uint32_t loitering_threshold_ms;     // Time in area before flagging
    float loitering_movement_threshold;   // Max movement (normalized)

    // Running detection
    float running_velocity_threshold;     // Min velocity (pixels/second)
    uint32_t running_frames_threshold;    // Consecutive frames

    // Repeated passes detection
    uint32_t repeated_passes_count;       // Number of passes
    uint32_t repeated_passes_window_ms;   // Time window

    // Extended observation
    uint32_t observation_threshold_ms;    // Time observing area
    float observation_gaze_threshold;     // Head orientation threshold

    // Threat scoring weights
    float loitering_weight;
    float running_weight;
    float concealing_weight;
    float repeated_passes_weight;
    float observation_weight;
} BehaviorConfig;

/**
 * Behavior event structure
 */
typedef struct {
    uint32_t track_id;           // Associated track
    BehaviorFlags behavior;      // Behavior type
    float confidence;            // Detection confidence
    uint64_t start_time_ms;      // When behavior started
    uint64_t duration_ms;        // Behavior duration
    float threat_contribution;   // Contribution to threat score
} BehaviorEvent;

/**
 * Initialize behavior analyzer
 *
 * @param config Behavior analysis configuration
 * @return Analyzer instance, NULL on failure
 */
BehaviorAnalyzer* behavior_init(const BehaviorConfig* config);

/**
 * Analyze tracked objects for behaviors
 *
 * Updates behavior flags and threat scores in tracked objects
 *
 * @param analyzer Behavior analyzer instance
 * @param tracks Array of tracked objects (modified in-place)
 * @param num_tracks Number of tracked objects
 * @return Number of behavior events detected
 */
uint32_t behavior_analyze(
    BehaviorAnalyzer* analyzer,
    TrackedObject* tracks,
    uint32_t num_tracks
);

/**
 * Get recent behavior events
 *
 * @param analyzer Behavior analyzer instance
 * @param events Output array for behavior events
 * @param max_events Maximum events to return
 * @return Number of events returned
 */
uint32_t behavior_get_events(
    BehaviorAnalyzer* analyzer,
    BehaviorEvent* events,
    uint32_t max_events
);

/**
 * Calculate threat score for a tracked object
 *
 * Combines multiple behavior indicators into single score
 *
 * @param analyzer Behavior analyzer instance
 * @param track Tracked object
 * @return Threat score [0.0, 1.0]
 */
float behavior_calculate_threat_score(
    BehaviorAnalyzer* analyzer,
    const TrackedObject* track
);

/**
 * Detect loitering behavior
 *
 * @param analyzer Behavior analyzer instance
 * @param track Tracked object
 * @return true if loitering detected
 */
bool behavior_detect_loitering(
    BehaviorAnalyzer* analyzer,
    const TrackedObject* track
);

/**
 * Detect running behavior
 *
 * @param analyzer Behavior analyzer instance
 * @param track Tracked object
 * @return true if running detected
 */
bool behavior_detect_running(
    BehaviorAnalyzer* analyzer,
    const TrackedObject* track
);

/**
 * Detect repeated passes behavior
 *
 * @param analyzer Behavior analyzer instance
 * @param track Tracked object
 * @return true if repeated passes detected
 */
bool behavior_detect_repeated_passes(
    BehaviorAnalyzer* analyzer,
    const TrackedObject* track
);

/**
 * Update behavior configuration
 *
 * @param analyzer Behavior analyzer instance
 * @param config New configuration
 */
void behavior_update_config(
    BehaviorAnalyzer* analyzer,
    const BehaviorConfig* config
);

/**
 * Clear behavior history
 *
 * @param analyzer Behavior analyzer instance
 */
void behavior_clear_history(BehaviorAnalyzer* analyzer);

/**
 * Get behavior analyzer statistics
 *
 * @param analyzer Behavior analyzer instance
 * @param total_events Total behavior events detected
 * @param avg_threat_score Average threat score across all tracks
 * @param high_threat_count Number of high-threat objects (>0.7)
 */
void behavior_get_stats(
    BehaviorAnalyzer* analyzer,
    uint64_t* total_events,
    float* avg_threat_score,
    uint32_t* high_threat_count
);

/**
 * Destroy behavior analyzer and free resources
 *
 * @param analyzer Behavior analyzer instance
 */
void behavior_destroy(BehaviorAnalyzer* analyzer);

/**
 * Get human-readable behavior description
 *
 * @param behavior Behavior flags
 * @param buffer Output buffer
 * @param buffer_size Buffer size
 * @return Number of characters written
 */
int behavior_to_string(
    BehaviorFlags behavior,
    char* buffer,
    size_t buffer_size
);

#ifdef __cplusplus
}
#endif

#endif // OMNISIGHT_BEHAVIOR_H
