/**
 * @file swarm.h
 * @brief OMNISIGHT Swarm Intelligence API
 *
 * Enables camera-to-camera coordination without sharing raw video.
 *
 * Key Features:
 * - Privacy-preserving feature sharing (not raw video)
 * - Federated learning for model updates
 * - Multi-camera event consensus
 * - Track handoff between cameras
 * - Distributed threat assessment
 * - Network topology management
 *
 * Communication Protocol: MQTT 5.0
 * Privacy: Only metadata, features, and model updates shared
 */

#ifndef OMNISIGHT_SWARM_H
#define OMNISIGHT_SWARM_H

#include <stdint.h>
#include <stdbool.h>
#include "../perception/perception.h"
#include "../timeline/timeline.h"

#ifdef __cplusplus
extern "C" {
#endif

// ============================================================================
// Forward Declarations
// ============================================================================

typedef struct SwarmNode SwarmNode;
typedef struct SwarmNetwork SwarmNetwork;

// ============================================================================
// Constants
// ============================================================================

#define SWARM_MAX_CAMERAS 100       // Maximum cameras in network
#define SWARM_MAX_NEIGHBORS 20      // Maximum direct neighbors per camera
#define SWARM_FEATURE_DIM 128       // Feature vector dimension for re-ID
#define SWARM_MAX_SHARED_TRACKS 50  // Max shared tracks per update

// ============================================================================
// Enums
// ============================================================================

/**
 * Camera role in swarm
 */
typedef enum {
    SWARM_ROLE_EDGE,        // Edge camera (perimeter)
    SWARM_ROLE_INTERNAL,    // Internal camera (center)
    SWARM_ROLE_GATEWAY,     // Gateway camera (entry/exit)
    SWARM_ROLE_COORDINATOR  // Coordinator (aggregates intelligence)
} SwarmRole;

/**
 * Message type in swarm protocol
 */
typedef enum {
    MSG_TYPE_HEARTBEAT,         // Periodic heartbeat
    MSG_TYPE_TRACK_SHARE,       // Share track info with neighbors
    MSG_TYPE_TRACK_HANDOFF,     // Transfer track to another camera
    MSG_TYPE_EVENT_SHARE,       // Share predicted event
    MSG_TYPE_CONSENSUS_REQUEST, // Request consensus on event
    MSG_TYPE_CONSENSUS_VOTE,    // Vote on event consensus
    MSG_TYPE_MODEL_UPDATE,      // Federated learning model update
    MSG_TYPE_THREAT_ALERT,      // Broadcast critical threat
    MSG_TYPE_TOPOLOGY_UPDATE    // Network topology change
} MessageType;

/**
 * Privacy level for shared data
 */
typedef enum {
    PRIVACY_METADATA_ONLY,      // Only bbox, timestamp, class
    PRIVACY_FEATURES,           // + Feature vectors (no images)
    PRIVACY_FULL_ANALYTICS      // + All analytics (still no raw video)
} PrivacyLevel;

// ============================================================================
// Data Structures
// ============================================================================

/**
 * Camera position and field of view
 */
typedef struct {
    float x, y, z;              // World coordinates (meters)
    float pan, tilt, zoom;      // PTZ parameters
    float fov_horizontal;       // Horizontal FOV (degrees)
    float fov_vertical;         // Vertical FOV (degrees)
    float coverage_radius;      // Effective coverage (meters)
} CameraGeometry;

/**
 * Shared track information (privacy-preserving)
 */
typedef struct {
    // Identity
    uint32_t camera_id;         // Source camera
    uint32_t track_id;          // Track ID at source
    uint64_t global_track_id;   // Global unique ID

    // Temporal
    uint64_t first_seen_ms;     // First detection time
    uint64_t last_seen_ms;      // Last detection time
    uint64_t exit_time_ms;      // When left this camera's FOV

    // Spatial (world coordinates)
    float world_x, world_y;     // Position in world coordinates
    float velocity_x, velocity_y; // Velocity in world coordinates
    float exit_direction;       // Direction when exiting FOV (radians)

    // Appearance (privacy-preserving features, NOT raw images)
    float features[SWARM_FEATURE_DIM];  // Re-identification features
    float feature_confidence;   // Feature quality [0.0, 1.0]

    // Semantics
    uint32_t object_class;      // Person, vehicle, etc.
    float height_meters;        // Estimated real-world height

    // Behavior
    BehaviorFlags behaviors;    // Behavior flags
    float threat_score;         // Threat assessment [0.0, 1.0]

    // Trajectory prediction (for handoff)
    float predicted_next_x;     // Predicted next position
    float predicted_next_y;
    float prediction_confidence;
} SharedTrack;

/**
 * Shared event information
 */
typedef struct {
    // Identity
    uint32_t camera_id;         // Source camera
    uint32_t event_id;          // Event ID at source
    uint64_t global_event_id;   // Global unique ID

    // Event details
    EventType type;             // Event type
    SeverityLevel severity;     // Severity level
    uint64_t timestamp_ms;      // When event will/did occur
    float probability;          // Event probability [0.0, 1.0]

    // Location (world coordinates)
    float world_x, world_y;     // Event location

    // Involved tracks
    uint32_t num_tracks;        // Number of involved tracks
    uint64_t global_track_ids[4]; // Global track IDs

    // Consensus
    uint32_t votes_for;         // Cameras agreeing
    uint32_t votes_against;     // Cameras disagreeing
    float consensus_confidence; // Consensus strength [0.0, 1.0]
} SharedEvent;

/**
 * Model update for federated learning
 */
typedef struct {
    uint32_t camera_id;         // Source camera
    uint64_t timestamp_ms;      // Update timestamp
    uint32_t model_version;     // Model version

    // Gradient updates (simplified for embedded)
    uint32_t num_params;        // Number of parameters
    float* param_updates;       // Parameter updates (allocated separately)
    float learning_rate;        // Learning rate used

    // Training statistics
    uint32_t num_samples;       // Samples used for training
    float loss;                 // Training loss
    float accuracy;             // Validation accuracy
} ModelUpdate;

/**
 * Swarm message
 */
typedef struct {
    MessageType type;           // Message type
    uint32_t source_id;         // Source camera ID
    uint32_t dest_id;           // Destination (0 = broadcast)
    uint64_t timestamp_ms;      // Message timestamp
    uint32_t sequence_number;   // For ordering/deduplication

    // Message payload (union for different message types)
    union {
        SharedTrack track;
        SharedEvent event;
        ModelUpdate model;
        struct {
            uint32_t camera_id;
            bool is_online;
            float cpu_load;
            float memory_usage;
        } heartbeat;
    } payload;
} SwarmMessage;

/**
 * Neighbor camera information
 */
typedef struct {
    uint32_t camera_id;         // Neighbor camera ID
    CameraGeometry geometry;    // Camera position and FOV
    float overlap_area;         // FOV overlap with this camera (0-1)
    uint64_t last_heartbeat_ms; // Last heartbeat received
    bool is_online;             // Online status
    float link_quality;         // Communication quality [0.0, 1.0]
} NeighborInfo;

/**
 * Swarm node configuration
 */
typedef struct {
    uint32_t camera_id;         // This camera's unique ID
    SwarmRole role;             // Camera role in swarm
    CameraGeometry geometry;    // This camera's geometry
    PrivacyLevel privacy;       // Privacy level for sharing

    // Network configuration
    const char* mqtt_broker;    // MQTT broker address
    uint16_t mqtt_port;         // MQTT port (default 1883)
    const char* mqtt_client_id; // MQTT client ID
    const char* mqtt_username;  // MQTT username (optional)
    const char* mqtt_password;  // MQTT password (optional)

    // Communication settings
    uint32_t heartbeat_interval_ms; // Heartbeat frequency
    uint32_t track_share_interval_ms; // Track sharing frequency
    uint32_t consensus_timeout_ms;   // Consensus decision timeout
    uint32_t handoff_lookahead_ms;   // Track handoff prediction time

    // Federated learning
    bool federated_learning_enabled;
    uint32_t model_update_interval_ms; // Model update frequency
    float min_improvement_threshold;   // Min improvement to share update
} SwarmConfig;

/**
 * Swarm network statistics
 */
typedef struct {
    uint32_t num_cameras;           // Cameras in network
    uint32_t num_online;            // Online cameras
    uint32_t num_neighbors;         // Direct neighbors

    uint64_t messages_sent;         // Total messages sent
    uint64_t messages_received;     // Total messages received
    uint64_t bytes_sent;            // Total bytes sent
    uint64_t bytes_received;        // Total bytes received

    uint32_t tracks_shared;         // Tracks shared
    uint32_t tracks_received;       // Tracks received from others
    uint32_t successful_handoffs;   // Successful track handoffs

    uint32_t events_shared;         // Events shared
    uint32_t consensus_reached;     // Events with consensus

    uint32_t model_updates_sent;    // Model updates sent
    uint32_t model_updates_received; // Model updates received

    float avg_network_latency_ms;  // Average message latency
    float network_health;           // Overall health [0.0, 1.0]
} SwarmStats;

// ============================================================================
// Public API
// ============================================================================

/**
 * Initialize swarm node
 *
 * @param config Swarm configuration
 * @return Swarm node handle, or NULL on error
 */
SwarmNode* swarm_init(const SwarmConfig* config);

/**
 * Destroy swarm node
 *
 * @param node Swarm node to destroy
 */
void swarm_destroy(SwarmNode* node);

/**
 * Start swarm communications
 *
 * @param node Swarm node
 * @return true on success, false on error
 */
bool swarm_start(SwarmNode* node);

/**
 * Stop swarm communications
 *
 * @param node Swarm node
 */
void swarm_stop(SwarmNode* node);

/**
 * Update swarm with current tracks
 *
 * Shares tracks with neighbors based on privacy level.
 * Handles track handoffs when tracks exit FOV.
 *
 * @param node Swarm node
 * @param tracks Current tracked objects
 * @param num_tracks Number of tracks
 * @param current_time_ms Current timestamp
 * @return Number of tracks shared
 */
uint32_t swarm_update_tracks(
    SwarmNode* node,
    const TrackedObject* tracks,
    uint32_t num_tracks,
    uint64_t current_time_ms
);

/**
 * Share predicted event with network
 *
 * Broadcasts event to neighbors for consensus.
 *
 * @param node Swarm node
 * @param event Predicted event
 * @param timeline Timeline containing event
 * @return true on success, false on error
 */
bool swarm_share_event(
    SwarmNode* node,
    const PredictedEvent* event,
    const Timeline* timeline
);

/**
 * Request consensus on event
 *
 * Polls neighbors for agreement on predicted event.
 *
 * @param node Swarm node
 * @param event Event to validate
 * @param timeout_ms Timeout for consensus
 * @param consensus_out Consensus result
 * @return true if consensus reached, false otherwise
 */
bool swarm_request_consensus(
    SwarmNode* node,
    const SharedEvent* event,
    uint32_t timeout_ms,
    float* consensus_out
);

/**
 * Share model update (federated learning)
 *
 * @param node Swarm node
 * @param update Model update
 * @return true on success, false on error
 */
bool swarm_share_model_update(
    SwarmNode* node,
    const ModelUpdate* update
);

/**
 * Get received tracks from neighbors
 *
 * Retrieves tracks shared by neighboring cameras.
 * Useful for re-identification and track association.
 *
 * @param node Swarm node
 * @param tracks Output buffer for tracks
 * @param max_tracks Maximum tracks to retrieve
 * @return Number of tracks retrieved
 */
uint32_t swarm_get_neighbor_tracks(
    SwarmNode* node,
    SharedTrack* tracks,
    uint32_t max_tracks
);

/**
 * Get shared events from network
 *
 * @param node Swarm node
 * @param events Output buffer for events
 * @param max_events Maximum events to retrieve
 * @return Number of events retrieved
 */
uint32_t swarm_get_shared_events(
    SwarmNode* node,
    SharedEvent* events,
    uint32_t max_events
);

/**
 * Attempt track handoff to neighbor
 *
 * Transfers track to neighboring camera when object exits FOV.
 *
 * @param node Swarm node
 * @param track Track to handoff
 * @param neighbor_id Target neighbor camera ID
 * @return true if handoff accepted, false otherwise
 */
bool swarm_handoff_track(
    SwarmNode* node,
    const SharedTrack* track,
    uint32_t neighbor_id
);

/**
 * Add/update neighbor camera
 *
 * @param node Swarm node
 * @param neighbor Neighbor information
 * @return true on success, false on error
 */
bool swarm_add_neighbor(
    SwarmNode* node,
    const NeighborInfo* neighbor
);

/**
 * Remove neighbor camera
 *
 * @param node Swarm node
 * @param camera_id Neighbor camera ID
 */
void swarm_remove_neighbor(
    SwarmNode* node,
    uint32_t camera_id
);

/**
 * Get network statistics
 *
 * @param node Swarm node
 * @param stats Output statistics
 */
void swarm_get_stats(
    const SwarmNode* node,
    SwarmStats* stats
);

/**
 * Calculate FOV overlap between two cameras
 *
 * Used to determine which cameras should be neighbors.
 *
 * @param camera1 First camera geometry
 * @param camera2 Second camera geometry
 * @return Overlap ratio [0.0, 1.0]
 */
float swarm_calculate_fov_overlap(
    const CameraGeometry* camera1,
    const CameraGeometry* camera2
);

/**
 * Convert local coordinates to world coordinates
 *
 * @param camera Camera geometry
 * @param image_x Image X coordinate (pixels)
 * @param image_y Image Y coordinate (pixels)
 * @param world_x Output world X (meters)
 * @param world_y Output world Y (meters)
 * @return true on success, false on error
 */
bool swarm_image_to_world(
    const CameraGeometry* camera,
    float image_x,
    float image_y,
    float* world_x,
    float* world_y
);

#ifdef __cplusplus
}
#endif

#endif // OMNISIGHT_SWARM_H
