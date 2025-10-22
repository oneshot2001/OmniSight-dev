/**
 * @file ipc_server.h
 * @brief OMNISIGHT IPC Server - C side of inter-process communication
 *
 * This module handles IPC between the C core and Python Flask API server.
 * It uses a hybrid approach:
 * - Shared memory for high-frequency data (detections, tracks, timelines)
 * - Named pipes for commands (Python → C)
 * - JSON files for status/stats (C → Python)
 * - Semaphores for synchronization
 *
 * Performance targets:
 * - Read latency: <1ms for shared memory
 * - Command latency: <100ms
 * - Memory overhead: <1MB
 *
 * See docs/IPC_ARCHITECTURE.md for complete specification.
 */

#ifndef OMNISIGHT_IPC_SERVER_H
#define OMNISIGHT_IPC_SERVER_H

#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>
#include <semaphore.h>
#include "../perception/perception.h"
#include "../timeline/timeline.h"
#include "../swarm/swarm.h"

#ifdef __cplusplus
extern "C" {
#endif

// Forward declaration
typedef struct IPCServer IPCServer;

/**
 * Shared memory region names
 */
#define OMNISIGHT_SHM_DETECTIONS "/omnisight_detections"
#define OMNISIGHT_SHM_TRACKS "/omnisight_tracks"
#define OMNISIGHT_SHM_TIMELINES "/omnisight_timelines"

/**
 * Semaphore names
 */
#define OMNISIGHT_SEM_FRAME_READY "/omnisight_frame_ready"
#define OMNISIGHT_SEM_TRACKS_READY "/omnisight_tracks_ready"
#define OMNISIGHT_SEM_TIMELINES_READY "/omnisight_timelines_ready"

/**
 * Named pipe paths
 */
#define OMNISIGHT_PIPE_CMD "/tmp/omnisight_cmd"
#define OMNISIGHT_PIPE_ACK "/tmp/omnisight_cmd_ack"

/**
 * JSON file paths
 */
#define OMNISIGHT_JSON_STATS "/tmp/omnisight_stats.json"
#define OMNISIGHT_JSON_STATUS "/tmp/omnisight_status.json"

/**
 * Protocol constants
 */
#define OMNISIGHT_MAGIC 0x4F4D4E49  // "OMNI"
#define OMNISIGHT_VERSION 1
#define MAX_DETECTIONS_PER_FRAME 50
#define MAX_TRACKED_OBJECTS 50
#define MAX_TIMELINES 5
#define MAX_EVENTS_PER_TIMELINE 20
#define MAX_INTERVENTIONS_PER_TIMELINE 10

/**
 * Detection frame structure (shared memory)
 *
 * Size: ~5,000 bytes
 * Writer: C core (perception engine)
 * Readers: Python Flask (multiple concurrent readers OK)
 * Update frequency: 10 Hz
 */
typedef struct {
  // Header
  uint32_t magic;           // 0x4F4D4E49 ("OMNI")
  uint32_t version;         // Protocol version (1)
  uint64_t frame_id;        // Monotonic frame counter
  uint64_t timestamp_ms;    // Milliseconds since epoch

  // Detections
  uint32_t num_detections;  // Number of valid detections
  DetectedObject objects[MAX_DETECTIONS_PER_FRAME];

  // Metadata
  uint32_t checksum;        // CRC32 of above data
  uint32_t reserved[3];     // Future use
} __attribute__((packed)) DetectionFrame;

/**
 * Tracked objects frame structure (shared memory)
 *
 * Size: ~8,000 bytes
 * Writer: C core (perception engine)
 * Readers: Python Flask (multiple concurrent readers OK)
 * Update frequency: 10 Hz
 */
typedef struct {
  // Header
  uint32_t magic;
  uint32_t version;
  uint64_t timestamp_ms;

  // Tracks
  uint32_t num_tracks;
  TrackedObject tracks[MAX_TRACKED_OBJECTS];

  // Metadata
  uint32_t checksum;
  uint32_t reserved[3];
} __attribute__((packed)) TrackedObjectsFrame;

/**
 * Timeline snapshot for IPC (flattened from tree structure)
 */
typedef struct {
  uint32_t timeline_id;
  float overall_probability;
  uint64_t prediction_start_ms;
  uint64_t prediction_end_ms;

  // Events
  uint32_t num_events;
  PredictedEvent events[MAX_EVENTS_PER_TIMELINE];

  // Interventions
  uint32_t num_interventions;
  InterventionPoint interventions[MAX_INTERVENTIONS_PER_TIMELINE];

  // Outcome
  SeverityLevel worst_case_severity;
  float total_threat_score;
} __attribute__((packed)) TimelineSnapshot;

/**
 * Timelines frame structure (shared memory)
 *
 * Size: ~20,000 bytes
 * Writer: C core (timeline engine)
 * Readers: Python Flask (multiple concurrent readers OK)
 * Update frequency: 1 Hz
 */
typedef struct {
  // Header
  uint32_t magic;
  uint32_t version;
  uint64_t timestamp_ms;

  // Timelines
  uint32_t num_timelines;
  TimelineSnapshot timelines[MAX_TIMELINES];

  // Metadata
  uint32_t checksum;
  uint32_t reserved[3];
} __attribute__((packed)) TimelinesFrame;

/**
 * Command types (named pipe)
 */
typedef enum {
  IPC_CMD_UNKNOWN = 0,
  IPC_CMD_PING,
  IPC_CMD_CONFIG_UPDATE,
  IPC_CMD_REFRESH_TIMELINES,
  IPC_CMD_SYNC_SWARM,
  IPC_CMD_SHUTDOWN
} IPCCommandType;

/**
 * Command callback function
 *
 * Called by IPC server when command received from Python.
 *
 * @param cmd_type Command type
 * @param data Command data (JSON string)
 * @param user_data User-provided context
 * @return true if command processed successfully
 */
typedef bool (*IPCCommandCallback)(
  IPCCommandType cmd_type,
  const char* data,
  void* user_data
);

/**
 * IPC server configuration
 */
typedef struct {
  // Command handling
  IPCCommandCallback command_callback;
  void* callback_user_data;

  // Performance tuning
  uint32_t stats_update_interval_ms;  // Default: 1000
  bool enable_checksums;               // Default: true
  bool double_buffer;                  // Default: true

  // Error handling
  bool auto_recovery;                  // Auto-recreate failed resources
  uint32_t command_timeout_ms;         // Default: 1000
} IPCServerConfig;

/**
 * Initialize IPC server
 *
 * Creates shared memory regions, semaphores, named pipes, and starts
 * command handler thread.
 *
 * @param config Server configuration
 * @return IPC server instance, NULL on failure
 */
IPCServer* ipc_server_init(const IPCServerConfig* config);

/**
 * Start IPC server
 *
 * Begins command processing thread.
 *
 * @param server IPC server instance
 * @return true on success
 */
bool ipc_server_start(IPCServer* server);

/**
 * Stop IPC server
 *
 * Stops command processing thread.
 *
 * @param server IPC server instance
 */
void ipc_server_stop(IPCServer* server);

/**
 * Publish detection frame to shared memory
 *
 * Updates shared memory with new detections and signals waiting readers.
 * Thread-safe, can be called from perception callback.
 *
 * @param server IPC server instance
 * @param objects Array of detected objects
 * @param num_objects Number of objects
 * @return true on success
 */
bool ipc_server_publish_detections(
  IPCServer* server,
  const DetectedObject* objects,
  uint32_t num_objects
);

/**
 * Publish tracked objects to shared memory
 *
 * Updates shared memory with current tracks and signals waiting readers.
 * Thread-safe, can be called from perception callback.
 *
 * @param server IPC server instance
 * @param tracks Array of tracked objects
 * @param num_tracks Number of tracks
 * @return true on success
 */
bool ipc_server_publish_tracks(
  IPCServer* server,
  const TrackedObject* tracks,
  uint32_t num_tracks
);

/**
 * Publish timeline predictions to shared memory
 *
 * Updates shared memory with current timelines and signals waiting readers.
 * Thread-safe, can be called from timeline callback.
 *
 * @param server IPC server instance
 * @param timelines Array of timeline snapshots
 * @param num_timelines Number of timelines
 * @return true on success
 */
bool ipc_server_publish_timelines(
  IPCServer* server,
  const Timeline* timelines,
  uint32_t num_timelines
);

/**
 * Update statistics JSON file
 *
 * Writes current system statistics to JSON file atomically.
 * Safe to call from any thread.
 *
 * @param server IPC server instance
 * @param stats OmnisightStats structure
 * @return true on success
 */
bool ipc_server_update_stats(
  IPCServer* server,
  const OmnisightStats* stats
);

/**
 * Update status JSON file
 *
 * Writes current system status to JSON file atomically.
 * Typically called during state transitions.
 *
 * @param server IPC server instance
 * @param status Status string ("starting", "running", "stopping", "error")
 * @return true on success
 */
bool ipc_server_update_status(
  IPCServer* server,
  const char* status
);

/**
 * Send command acknowledgment
 *
 * Sends acknowledgment back to Python via named pipe.
 * Internal use by command handler.
 *
 * @param server IPC server instance
 * @param request_id Request ID from command
 * @param success Command success status
 * @param message Optional message
 * @return true on success
 */
bool ipc_server_send_ack(
  IPCServer* server,
  const char* request_id,
  bool success,
  const char* message
);

/**
 * Get IPC server statistics
 *
 * @param server IPC server instance
 * @param frames_published Total detection frames published
 * @param tracks_published Total track frames published
 * @param timelines_published Total timeline updates published
 * @param commands_processed Total commands processed
 * @param errors Total errors encountered
 */
void ipc_server_get_stats(
  IPCServer* server,
  uint64_t* frames_published,
  uint64_t* tracks_published,
  uint64_t* timelines_published,
  uint64_t* commands_processed,
  uint64_t* errors
);

/**
 * Destroy IPC server
 *
 * Stops threads, closes all resources, and frees memory.
 *
 * @param server IPC server instance
 */
void ipc_server_destroy(IPCServer* server);

/**
 * Get default IPC server configuration
 *
 * @param config Output configuration
 */
void ipc_server_get_default_config(IPCServerConfig* config);

/**
 * Utility: Calculate CRC32 checksum
 *
 * @param data Data buffer
 * @param size Data size in bytes
 * @return CRC32 checksum
 */
uint32_t ipc_crc32(const void* data, size_t size);

/**
 * Utility: Get current timestamp in milliseconds
 *
 * @return Milliseconds since epoch
 */
uint64_t ipc_get_timestamp_ms(void);

/**
 * Utility: Parse command type from JSON string
 *
 * @param json JSON command string
 * @return Command type
 */
IPCCommandType ipc_parse_command_type(const char* json);

#ifdef __cplusplus
}
#endif

#endif // OMNISIGHT_IPC_SERVER_H
