/**
 * @file omnisight_api.h
 * @brief REST API server for OMNISIGHT web interface
 *
 * Provides HTTP endpoints for:
 * - Real-time event streaming (WebSocket)
 * - Statistics and monitoring
 * - Configuration management
 * - Camera network status
 */

#ifndef OMNISIGHT_API_H
#define OMNISIGHT_API_H

#include <stdint.h>
#include <stdbool.h>
#include "../../src/perception/perception.h"
#include "../../src/timeline/timeline.h"
#include "../../src/swarm/swarm.h"

#ifdef __cplusplus
extern "C" {
#endif

// ============================================================================
// API Server Configuration
// ============================================================================

typedef struct {
    uint16_t http_port;           // HTTP server port (default: 8080)
    uint16_t websocket_port;      // WebSocket port (default: 8081)
    const char* web_root;         // Web root directory
    bool enable_cors;             // Enable CORS
    const char* allowed_origins;  // CORS allowed origins
    uint32_t max_connections;     // Max simultaneous connections
} ApiServerConfig;

// ============================================================================
// API Server Handle
// ============================================================================

typedef struct ApiServer ApiServer;

// ============================================================================
// API Endpoints
// ============================================================================

/**
 * GET /api/status
 * Returns overall system status
 *
 * Response:
 * {
 *   "status": "running",
 *   "uptime_seconds": 12345,
 *   "camera_id": 1,
 *   "fps": 30.0,
 *   "latency_ms": 18.5
 * }
 */

/**
 * GET /api/statistics
 * Returns detailed statistics
 *
 * Response:
 * {
 *   "perception": {
 *     "frames_processed": 10000,
 *     "active_tracks": 15,
 *     "fps": 30.0,
 *     "avg_latency_ms": 18.5
 *   },
 *   "timeline": {
 *     "num_updates": 5000,
 *     "events_predicted": 50,
 *     "interventions": 10,
 *     "avg_confidence": 0.85
 *   },
 *   "swarm": {
 *     "num_neighbors": 5,
 *     "tracks_shared": 1000,
 *     "tracks_received": 800,
 *     "network_health": 0.95
 *   }
 * }
 */

/**
 * GET /api/tracks
 * Returns current tracked objects
 *
 * Response:
 * {
 *   "timestamp_ms": 1234567890,
 *   "tracks": [
 *     {
 *       "track_id": 1,
 *       "x": 500, "y": 300, "width": 50, "height": 100,
 *       "velocity_x": 5.0, "velocity_y": 2.0,
 *       "confidence": 0.95,
 *       "threat_score": 0.3,
 *       "behaviors": ["loitering"]
 *     },
 *     ...
 *   ]
 * }
 */

/**
 * GET /api/events
 * Returns predicted events
 *
 * Response:
 * {
 *   "events": [
 *     {
 *       "event_id": 1,
 *       "type": "trespassing",
 *       "severity": "high",
 *       "probability": 0.85,
 *       "timestamp_ms": 1234567890,
 *       "location": {"x": 500, "y": 300},
 *       "involved_tracks": [1, 2]
 *     },
 *     ...
 *   ]
 * }
 */

/**
 * GET /api/interventions
 * Returns recommended interventions
 *
 * Response:
 * {
 *   "interventions": [
 *     {
 *       "intervention_id": 1,
 *       "type": "alert_security",
 *       "urgency": "high",
 *       "time_to_act_ms": 5000,
 *       "location": {"x": 500, "y": 300},
 *       "success_probability": 0.8
 *     },
 *     ...
 *   ]
 * }
 */

/**
 * GET /api/cameras
 * Returns camera network status
 *
 * Response:
 * {
 *   "cameras": [
 *     {
 *       "camera_id": 1,
 *       "role": "internal",
 *       "position": {"x": 10.0, "y": 5.0, "z": 3.0},
 *       "is_online": true,
 *       "last_heartbeat_ms": 1234567890,
 *       "active_tracks": 15,
 *       "fps": 30.0
 *     },
 *     ...
 *   ]
 * }
 */

/**
 * GET /api/heatmap
 * Returns threat heatmap data
 *
 * Response:
 * {
 *   "width": 1920,
 *   "height": 1080,
 *   "grid_size": 32,
 *   "data": [0.1, 0.2, 0.8, ...]  // Threat scores per grid cell
 * }
 */

/**
 * GET /api/config
 * Returns current configuration
 *
 * POST /api/config
 * Updates configuration
 *
 * Request:
 * {
 *   "perception": {
 *     "fps": 30,
 *     "confidence_threshold": 0.5
 *   },
 *   "timeline": {
 *     "prediction_horizon_s": 60.0
 *   }
 * }
 */

/**
 * WebSocket /ws/events
 * Real-time event stream
 *
 * Messages:
 * {
 *   "type": "event",
 *   "data": { ... }
 * }
 * {
 *   "type": "track_update",
 *   "data": { ... }
 * }
 * {
 *   "type": "intervention",
 *   "data": { ... }
 * }
 */

// ============================================================================
// Public API
// ============================================================================

/**
 * Initialize API server
 *
 * @param config Server configuration
 * @param perception Perception engine handle
 * @param timeline Timeline engine handle
 * @param swarm Swarm node handle
 * @return API server handle, or NULL on error
 */
ApiServer* api_server_init(
    const ApiServerConfig* config,
    PerceptionEngine* perception,
    TimelineEngine* timeline,
    SwarmNode* swarm
);

/**
 * Destroy API server
 *
 * @param server API server to destroy
 */
void api_server_destroy(ApiServer* server);

/**
 * Start API server
 *
 * Starts HTTP and WebSocket servers in separate threads.
 *
 * @param server API server
 * @return true on success, false on error
 */
bool api_server_start(ApiServer* server);

/**
 * Stop API server
 *
 * @param server API server
 */
void api_server_stop(ApiServer* server);

/**
 * Broadcast event to all connected WebSocket clients
 *
 * @param server API server
 * @param event_json JSON string of event
 */
void api_server_broadcast_event(
    ApiServer* server,
    const char* event_json
);

/**
 * Broadcast track update to all connected WebSocket clients
 *
 * @param server API server
 * @param tracks_json JSON string of tracks
 */
void api_server_broadcast_tracks(
    ApiServer* server,
    const char* tracks_json
);

/**
 * Get number of connected WebSocket clients
 *
 * @param server API server
 * @return Number of connected clients
 */
uint32_t api_server_get_num_clients(const ApiServer* server);

#ifdef __cplusplus
}
#endif

#endif // OMNISIGHT_API_H
