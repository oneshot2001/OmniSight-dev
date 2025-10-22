/**
 * @file timeline.h
 * @brief Timeline Threading™ - Precognitive prediction engine for OMNISIGHT
 *
 * The Timeline Threading engine predicts 3-5 probable futures by analyzing:
 * - Object trajectories and velocities
 * - Behavioral patterns and threat scores
 * - Environmental context and constraints
 * - Historical incident patterns
 *
 * Output: Multiple timeline branches with confidence scores and intervention points
 *
 * Performance targets:
 * - Prediction horizon: 30 seconds to 5 minutes
 * - Timeline generation: <50ms
 * - Accuracy: 80% @ 30s, 60% @ 5min
 */

#ifndef OMNISIGHT_TIMELINE_H
#define OMNISIGHT_TIMELINE_H

#include "../perception/perception.h"
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// Forward declarations
typedef struct TimelineEngine TimelineEngine;
typedef struct Timeline Timeline;
typedef struct TimelineNode TimelineNode;
typedef struct InterventionPoint InterventionPoint;

/**
 * Event types that can occur in timelines
 */
typedef enum {
    EVENT_NONE = 0,
    EVENT_LOITERING,
    EVENT_THEFT,
    EVENT_ASSAULT,
    EVENT_VANDALISM,
    EVENT_TRESPASSING,
    EVENT_SUSPICIOUS_BEHAVIOR,
    EVENT_COLLISION,
    EVENT_FALL,
    EVENT_ABANDONED_OBJECT,
    EVENT_CROWD_FORMATION,
    EVENT_MAX
} EventType;

/**
 * Intervention types that can prevent incidents
 */
typedef enum {
    INTERVENTION_NONE = 0,
    INTERVENTION_ALERT_SECURITY,
    INTERVENTION_ACTIVATE_SPEAKER,
    INTERVENTION_INCREASE_LIGHTING,
    INTERVENTION_POSITION_GUARD,
    INTERVENTION_LOCK_DOOR,
    INTERVENTION_NOTIFY_POLICE,
    INTERVENTION_DISPLAY_WARNING,
    INTERVENTION_MAX
} InterventionType;

/**
 * Timeline outcome severity
 */
typedef enum {
    SEVERITY_NONE = 0,
    SEVERITY_LOW,
    SEVERITY_MEDIUM,
    SEVERITY_HIGH,
    SEVERITY_CRITICAL
} SeverityLevel;

/**
 * Predicted position at a future time
 */
typedef struct {
    uint64_t timestamp_ms;      // Predicted time
    float x, y;                 // Predicted position (normalized)
    float vx, vy;               // Predicted velocity
    float confidence;           // Prediction confidence [0.0, 1.0]
    BehaviorFlags behaviors;    // Predicted behaviors
    float threat_score;         // Predicted threat level
} PredictedState;

/**
 * A single event prediction in a timeline
 */
typedef struct {
    EventType type;             // Type of event
    uint64_t timestamp_ms;      // When event occurs
    float probability;          // Probability of occurrence [0.0, 1.0]
    SeverityLevel severity;     // Event severity
    uint32_t involved_tracks[4]; // Track IDs involved
    uint32_t num_involved;      // Number of tracks
    float location_x, location_y; // Event location
    char description[256];      // Human-readable description
} PredictedEvent;

/**
 * Intervention point in timeline
 */
struct InterventionPoint {
    uint64_t timestamp_ms;           // When to intervene
    InterventionType type;           // Type of intervention
    float effectiveness;             // Expected effectiveness [0.0, 1.0]
    float cost;                      // Cost/effort required [0.0, 1.0]
    PredictedEvent prevented_event;  // Event this would prevent
    char recommendation[256];        // Human-readable recommendation
};

/**
 * Node in timeline tree (represents a decision/branch point)
 */
struct TimelineNode {
    uint64_t timestamp_ms;           // Time of this node
    uint32_t node_id;                // Unique node ID

    // State at this point
    uint32_t num_tracks;
    PredictedState track_states[50]; // Predicted states of all tracks

    // Events at this time
    uint32_t num_events;
    PredictedEvent events[10];

    // Branching
    uint32_t num_children;           // Number of possible futures
    struct TimelineNode* children[5]; // Child timelines (max 5 branches)
    float branch_probability[5];     // Probability of each branch

    // Parent node
    struct TimelineNode* parent;
};

/**
 * Complete timeline branch from present to future
 */
struct Timeline {
    uint32_t timeline_id;            // Unique timeline ID
    float overall_probability;       // Overall probability [0.0, 1.0]
    uint64_t prediction_start_ms;    // When prediction starts
    uint64_t prediction_end_ms;      // How far into future

    // Timeline tree
    TimelineNode* root;              // Root node (current state)
    uint32_t num_nodes;              // Total nodes in timeline
    uint32_t tree_depth;             // Depth of timeline tree

    // Aggregated predictions
    uint32_t num_predicted_events;
    PredictedEvent predicted_events[50];

    // Intervention opportunities
    uint32_t num_interventions;
    InterventionPoint interventions[20];

    // Outcome assessment
    SeverityLevel worst_case_severity;
    float total_threat_score;        // Integrated threat over time
    bool incident_prevented;         // If intervention applied

    // Metadata
    uint64_t created_ms;
    uint64_t last_updated_ms;
};

/**
 * Timeline engine configuration
 */
typedef struct {
    // Prediction parameters
    uint32_t prediction_horizon_ms;  // How far to predict (default: 300000 = 5min)
    uint32_t time_step_ms;           // Prediction granularity (default: 1000 = 1s)
    uint32_t max_timelines;          // Maximum branches (default: 5)

    // Branching thresholds
    float branch_threshold;          // Min probability for branch (default: 0.3)
    float merge_threshold;           // Similarity threshold for merging (default: 0.8)

    // Event detection
    float event_threshold;           // Min probability for event (default: 0.5)
    bool enable_intervention_search; // Find intervention points

    // Performance
    uint32_t max_iterations;         // Max prediction iterations
    bool use_gpu;                    // Use ML model on DLPU (future)
} TimelineConfig;

/**
 * Timeline callback for real-time updates
 */
typedef void (*TimelineCallback)(
    const Timeline* timelines,
    uint32_t num_timelines,
    void* user_data
);

/**
 * Initialize Timeline Threading engine
 *
 * @param config Engine configuration
 * @return Timeline engine instance, NULL on failure
 */
TimelineEngine* timeline_init(const TimelineConfig* config);

/**
 * Update timeline predictions with current track data
 *
 * Call this each frame with updated tracked objects from perception engine
 *
 * @param engine Timeline engine instance
 * @param tracks Current tracked objects
 * @param num_tracks Number of tracked objects
 * @param timelines Output array for generated timelines
 * @param max_timelines Maximum timelines to generate
 * @return Number of timelines generated
 */
uint32_t timeline_update(
    TimelineEngine* engine,
    const TrackedObject* tracks,
    uint32_t num_tracks,
    Timeline* timelines,
    uint32_t max_timelines
);

/**
 * Get current active timelines
 *
 * @param engine Timeline engine instance
 * @param timelines Output array for timelines
 * @param max_timelines Maximum timelines to return
 * @return Number of timelines returned
 */
uint32_t timeline_get_timelines(
    TimelineEngine* engine,
    Timeline* timelines,
    uint32_t max_timelines
);

/**
 * Get timeline by ID
 *
 * @param engine Timeline engine instance
 * @param timeline_id Timeline ID to retrieve
 * @param timeline Output timeline
 * @return true if found, false otherwise
 */
bool timeline_get_by_id(
    TimelineEngine* engine,
    uint32_t timeline_id,
    Timeline* timeline
);

/**
 * Get best intervention for preventing worst timeline
 *
 * @param engine Timeline engine instance
 * @param intervention Output intervention point
 * @return true if intervention found, false if no action needed
 */
bool timeline_get_best_intervention(
    TimelineEngine* engine,
    InterventionPoint* intervention
);

/**
 * Apply intervention and recalculate timelines
 *
 * Simulates applying an intervention and updates timeline predictions
 *
 * @param engine Timeline engine instance
 * @param intervention Intervention to apply
 * @param new_timelines Output array for updated timelines
 * @param max_timelines Maximum timelines to generate
 * @return Number of new timelines generated
 */
uint32_t timeline_apply_intervention(
    TimelineEngine* engine,
    const InterventionPoint* intervention,
    Timeline* new_timelines,
    uint32_t max_timelines
);

/**
 * Start real-time timeline monitoring
 *
 * Automatically updates timelines as new track data arrives
 *
 * @param engine Timeline engine instance
 * @param callback Function called with updated timelines
 * @param user_data User data passed to callback
 * @return true on success
 */
bool timeline_start_monitoring(
    TimelineEngine* engine,
    TimelineCallback callback,
    void* user_data
);

/**
 * Stop real-time monitoring
 *
 * @param engine Timeline engine instance
 */
void timeline_stop_monitoring(TimelineEngine* engine);

/**
 * Clear all timelines and reset engine
 *
 * @param engine Timeline engine instance
 */
void timeline_clear(TimelineEngine* engine);

/**
 * Get timeline engine statistics
 *
 * @param engine Timeline engine instance
 * @param total_timelines Total timelines generated
 * @param total_events Total events predicted
 * @param total_interventions Total interventions suggested
 * @param avg_prediction_ms Average prediction time
 */
void timeline_get_stats(
    TimelineEngine* engine,
    uint64_t* total_timelines,
    uint64_t* total_events,
    uint64_t* total_interventions,
    float* avg_prediction_ms
);

/**
 * Destroy timeline engine and free resources
 *
 * @param engine Timeline engine instance
 */
void timeline_destroy(TimelineEngine* engine);

/**
 * Export timeline to JSON format
 *
 * @param timeline Timeline to export
 * @param buffer Output buffer
 * @param buffer_size Buffer size
 * @return Number of bytes written, or -1 on error
 */
int timeline_export_json(
    const Timeline* timeline,
    char* buffer,
    size_t buffer_size
);

/**
 * Get human-readable event description
 *
 * @param event_type Event type
 * @return Event description string
 */
const char* timeline_event_to_string(EventType event_type);

/**
 * Get human-readable intervention description
 *
 * @param intervention_type Intervention type
 * @return Intervention description string
 */
const char* timeline_intervention_to_string(InterventionType intervention_type);

#ifdef __cplusplus
}
#endif

#endif // OMNISIGHT_TIMELINE_H
