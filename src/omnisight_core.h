/**
 * @file omnisight_core.h
 * @brief OMNISIGHT Core Integration Layer
 *
 * Coordinates all three modules (Perception, Timeline, Swarm) and manages
 * their interactions, data flow, and lifecycle.
 */

#ifndef OMNISIGHT_CORE_H
#define OMNISIGHT_CORE_H

#include "perception/perception.h"
#include "timeline/timeline.h"
#include "swarm/swarm.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// Forward declaration
typedef struct OmnisightCore OmnisightCore;

/**
 * OMNISIGHT system configuration
 */
typedef struct {
    // System-wide settings
    uint32_t camera_id;            // Unique camera ID in network
    const char* camera_name;       // Human-readable camera name
    const char* location;          // Camera location description

    // Module configurations
    PerceptionConfig perception;
    TimelineConfig timeline;
    SwarmConfig swarm;

    // Integration settings
    bool enable_perception;
    bool enable_timeline;
    bool enable_swarm;
    bool auto_start;

    // Performance
    uint32_t update_interval_ms;   // Main loop update interval
    bool verbose_logging;
} OmnisightConfig;

/**
 * System-wide statistics
 */
typedef struct {
    // Overall status
    bool is_running;
    uint64_t uptime_ms;
    uint64_t frames_processed;

    // Per-module stats
    struct {
        float avg_inference_ms;
        float avg_fps;
        uint32_t tracked_objects;
        uint32_t dropped_frames;
    } perception_stats;

    struct {
        uint32_t active_timelines;
        uint64_t total_events_predicted;
        uint64_t total_interventions;
        float avg_prediction_ms;
    } timeline_stats;

    struct {
        uint32_t num_neighbors;
        uint32_t tracks_shared;
        uint32_t events_shared;
        float network_health;
    } swarm_stats;

    // Integration metrics
    uint32_t perception_to_timeline;  // Tracks passed to timeline
    uint32_t timeline_to_swarm;       // Events shared to swarm
    uint32_t swarm_to_perception;     // Neighbor tracks received
} OmnisightStats;

/**
 * Event notification types
 */
typedef enum {
    OMNISIGHT_EVENT_STARTED,
    OMNISIGHT_EVENT_STOPPED,
    OMNISIGHT_EVENT_OBJECT_DETECTED,
    OMNISIGHT_EVENT_OBJECT_TRACKED,
    OMNISIGHT_EVENT_BEHAVIOR_DETECTED,
    OMNISIGHT_EVENT_TIMELINE_UPDATED,
    OMNISIGHT_EVENT_INTERVENTION_RECOMMENDED,
    OMNISIGHT_EVENT_SWARM_TRACK_RECEIVED,
    OMNISIGHT_EVENT_SWARM_EVENT_RECEIVED,
    OMNISIGHT_EVENT_CONSENSUS_REACHED,
    OMNISIGHT_EVENT_ERROR
} OmnisightEventType;

/**
 * Event notification structure
 */
typedef struct {
    OmnisightEventType type;
    uint64_t timestamp_ms;
    void* data;  // Event-specific data
    char message[256];
} OmnisightEvent;

/**
 * Event callback for notifications
 */
typedef void (*OmnisightEventCallback)(
    const OmnisightEvent* event,
    void* user_data
);

/**
 * Initialize OMNISIGHT core system
 *
 * @param config System configuration
 * @return Core instance, or NULL on failure
 */
OmnisightCore* omnisight_init(const OmnisightConfig* config);

/**
 * Start all enabled modules
 *
 * @param core OMNISIGHT core instance
 * @param callback Event notification callback (optional)
 * @param user_data User data for callback
 * @return true on success, false on failure
 */
bool omnisight_start(
    OmnisightCore* core,
    OmnisightEventCallback callback,
    void* user_data
);

/**
 * Stop all modules
 *
 * @param core OMNISIGHT core instance
 */
void omnisight_stop(OmnisightCore* core);

/**
 * Main processing loop (call periodically or run in thread)
 *
 * @param core OMNISIGHT core instance
 * @return true to continue running, false to stop
 */
bool omnisight_process(OmnisightCore* core);

/**
 * Get current system statistics
 *
 * @param core OMNISIGHT core instance
 * @param stats Output statistics structure
 */
void omnisight_get_stats(
    OmnisightCore* core,
    OmnisightStats* stats
);

/**
 * Get current tracked objects
 *
 * @param core OMNISIGHT core instance
 * @param objects Output array
 * @param max_objects Maximum objects to return
 * @return Number of objects returned
 */
uint32_t omnisight_get_tracked_objects(
    OmnisightCore* core,
    TrackedObject* objects,
    uint32_t max_objects
);

/**
 * Get current timelines
 *
 * @param core OMNISIGHT core instance
 * @param timelines Output array
 * @param max_timelines Maximum timelines to return
 * @return Number of timelines returned
 */
uint32_t omnisight_get_timelines(
    OmnisightCore* core,
    Timeline* timelines,
    uint32_t max_timelines
);

/**
 * Get best intervention recommendation
 *
 * @param core OMNISIGHT core instance
 * @param intervention Output intervention point
 * @return true if intervention recommended, false if none needed
 */
bool omnisight_get_intervention(
    OmnisightCore* core,
    InterventionPoint* intervention
);

/**
 * Manual trigger: Force timeline recalculation
 *
 * @param core OMNISIGHT core instance
 */
void omnisight_refresh_timelines(OmnisightCore* core);

/**
 * Manual trigger: Share current state with swarm
 *
 * @param core OMNISIGHT core instance
 */
void omnisight_sync_swarm(OmnisightCore* core);

/**
 * Get module health status
 *
 * @param core OMNISIGHT core instance
 * @param perception_ok Perception module status
 * @param timeline_ok Timeline module status
 * @param swarm_ok Swarm module status
 * @return true if all enabled modules healthy
 */
bool omnisight_health_check(
    OmnisightCore* core,
    bool* perception_ok,
    bool* timeline_ok,
    bool* swarm_ok
);

/**
 * Export system state to JSON
 *
 * @param core OMNISIGHT core instance
 * @param buffer Output buffer
 * @param buffer_size Buffer size
 * @return Number of bytes written, or -1 on error
 */
int omnisight_export_json(
    OmnisightCore* core,
    char* buffer,
    size_t buffer_size
);

/**
 * Destroy core system and free all resources
 *
 * @param core OMNISIGHT core instance
 */
void omnisight_destroy(OmnisightCore* core);

/**
 * Get default configuration
 *
 * @param config Output configuration with defaults
 */
void omnisight_get_default_config(OmnisightConfig* config);

/**
 * Validate configuration
 *
 * @param config Configuration to validate
 * @return true if valid, false otherwise
 */
bool omnisight_validate_config(const OmnisightConfig* config);

#ifdef __cplusplus
}
#endif

#endif // OMNISIGHT_CORE_H
