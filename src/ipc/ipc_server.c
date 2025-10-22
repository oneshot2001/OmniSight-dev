/**
 * @file ipc_server.c
 * @brief OMNISIGHT IPC Server - Implementation skeleton
 *
 * This is a skeleton implementation with detailed TODOs for Phase 2 development.
 * See docs/IPC_ARCHITECTURE.md for complete specification.
 */

#include "ipc_server.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <errno.h>
#include <time.h>

/**
 * IPC server internal structure
 */
struct IPCServer {
  // Configuration
  IPCServerConfig config;

  // Shared memory
  int detections_fd;
  DetectionFrame* detections_ptr;
  int tracks_fd;
  TrackedObjectsFrame* tracks_ptr;
  int timelines_fd;
  TimelinesFrame* timelines_ptr;

  // Semaphores
  sem_t* frame_ready_sem;
  sem_t* tracks_ready_sem;
  sem_t* timelines_ready_sem;

  // Named pipes
  int cmd_fd;
  int ack_fd;

  // Command handler thread
  pthread_t cmd_thread;
  bool running;

  // Statistics
  uint64_t frames_published;
  uint64_t tracks_published;
  uint64_t timelines_published;
  uint64_t commands_processed;
  uint64_t errors;

  // Synchronization
  pthread_mutex_t stats_mutex;

  // Double buffering
  uint32_t detections_write_index;
  uint32_t tracks_write_index;
  uint32_t timelines_write_index;
};

// Forward declarations
static void* ipc_command_handler_thread(void* arg);
static bool ipc_create_shared_memory(IPCServer* server);
static bool ipc_create_semaphores(IPCServer* server);
static bool ipc_create_pipes(IPCServer* server);
static void ipc_cleanup_resources(IPCServer* server);

/**
 * Get default configuration
 */
void ipc_server_get_default_config(IPCServerConfig* config) {
  memset(config, 0, sizeof(IPCServerConfig));
  config->stats_update_interval_ms = 1000;
  config->enable_checksums = true;
  config->double_buffer = true;
  config->auto_recovery = true;
  config->command_timeout_ms = 1000;
}

/**
 * Initialize IPC server
 */
IPCServer* ipc_server_init(const IPCServerConfig* config) {
  IPCServer* server = calloc(1, sizeof(IPCServer));
  if (!server) {
    return NULL;
  }

  // Copy configuration
  memcpy(&server->config, config, sizeof(IPCServerConfig));

  // Initialize mutex
  pthread_mutex_init(&server->stats_mutex, NULL);

  // TODO Phase 2.1: Create shared memory regions
  // - Call ipc_create_shared_memory(server)
  // - Handle errors and cleanup on failure
  if (!ipc_create_shared_memory(server)) {
    fprintf(stderr, "IPC: Failed to create shared memory\n");
    ipc_server_destroy(server);
    return NULL;
  }

  // TODO Phase 2.1: Create semaphores
  // - Call ipc_create_semaphores(server)
  // - Initialize to 0 (no data available initially)
  if (!ipc_create_semaphores(server)) {
    fprintf(stderr, "IPC: Failed to create semaphores\n");
    ipc_server_destroy(server);
    return NULL;
  }

  // TODO Phase 2.2: Create named pipes
  // - Call ipc_create_pipes(server)
  // - Set non-blocking mode on write end
  if (!ipc_create_pipes(server)) {
    fprintf(stderr, "IPC: Failed to create pipes\n");
    ipc_server_destroy(server);
    return NULL;
  }

  // TODO Phase 2.3: Initialize status file
  // - Write initial status: "starting"
  // - Include PID, version, IPC paths
  ipc_server_update_status(server, "starting");

  printf("IPC: Server initialized successfully\n");
  return server;
}

/**
 * Start IPC server
 */
bool ipc_server_start(IPCServer* server) {
  if (!server) return false;

  server->running = true;

  // TODO Phase 2.2: Start command handler thread
  // - pthread_create(&server->cmd_thread, NULL, ipc_command_handler_thread, server)
  // - Handle thread creation errors
  if (pthread_create(&server->cmd_thread, NULL,
                    ipc_command_handler_thread, server) != 0) {
    fprintf(stderr, "IPC: Failed to start command handler thread\n");
    server->running = false;
    return false;
  }

  // TODO Phase 2.3: Update status file
  ipc_server_update_status(server, "running");

  printf("IPC: Server started\n");
  return true;
}

/**
 * Stop IPC server
 */
void ipc_server_stop(IPCServer* server) {
  if (!server) return;

  // TODO Phase 2.2: Stop command handler thread
  // - Set server->running = false
  // - pthread_join(server->cmd_thread, NULL)
  server->running = false;
  if (server->cmd_thread) {
    pthread_join(server->cmd_thread, NULL);
  }

  // TODO Phase 2.3: Update status file
  ipc_server_update_status(server, "stopped");

  printf("IPC: Server stopped\n");
}

/**
 * Publish detection frame
 */
bool ipc_server_publish_detections(
  IPCServer* server,
  const DetectedObject* objects,
  uint32_t num_objects) {

  if (!server || !objects) return false;

  // TODO Phase 2.1: Implement detection publishing
  // 1. Get current buffer (handle double-buffering if enabled)
  //    DetectionFrame* frame = &server->detections_ptr[server->detections_write_index];
  //
  // 2. Fill frame header
  //    frame->magic = OMNISIGHT_MAGIC;
  //    frame->version = OMNISIGHT_VERSION;
  //    frame->frame_id = server->frames_published;
  //    frame->timestamp_ms = ipc_get_timestamp_ms();
  //    frame->num_detections = num_objects;
  //
  // 3. Copy detection data
  //    memcpy(frame->objects, objects, num_objects * sizeof(DetectedObject));
  //
  // 4. Calculate checksum (if enabled)
  //    if (server->config.enable_checksums) {
  //      frame->checksum = ipc_crc32(frame, sizeof(DetectionFrame) - 4);
  //    }
  //
  // 5. Memory barrier to ensure writes complete
  //    __sync_synchronize();
  //
  // 6. Signal readers
  //    sem_post(server->frame_ready_sem);
  //
  // 7. Swap buffers (if double-buffering)
  //    server->detections_write_index = 1 - server->detections_write_index;
  //
  // 8. Update statistics
  //    pthread_mutex_lock(&server->stats_mutex);
  //    server->frames_published++;
  //    pthread_mutex_unlock(&server->stats_mutex);

  // Stub implementation
  pthread_mutex_lock(&server->stats_mutex);
  server->frames_published++;
  pthread_mutex_unlock(&server->stats_mutex);

  return true;
}

/**
 * Publish tracked objects
 */
bool ipc_server_publish_tracks(
  IPCServer* server,
  const TrackedObject* tracks,
  uint32_t num_tracks) {

  if (!server || !tracks) return false;

  // TODO Phase 2.1: Implement track publishing
  // Similar to ipc_server_publish_detections() but for TrackedObjectsFrame
  // Follow same pattern:
  // - Get buffer
  // - Fill header
  // - Copy data
  // - Checksum
  // - Memory barrier
  // - Signal
  // - Swap buffer
  // - Update stats

  // Stub implementation
  pthread_mutex_lock(&server->stats_mutex);
  server->tracks_published++;
  pthread_mutex_unlock(&server->stats_mutex);

  return true;
}

/**
 * Publish timeline predictions
 */
bool ipc_server_publish_timelines(
  IPCServer* server,
  const Timeline* timelines,
  uint32_t num_timelines) {

  if (!server || !timelines) return false;

  // TODO Phase 2.1: Implement timeline publishing
  // Challenge: Need to flatten Timeline tree structure into TimelineSnapshot
  //
  // For each timeline:
  // 1. Create TimelineSnapshot
  //    snapshot.timeline_id = timeline->timeline_id;
  //    snapshot.overall_probability = timeline->overall_probability;
  //    ...
  //
  // 2. Copy events array
  //    snapshot.num_events = timeline->num_predicted_events;
  //    memcpy(snapshot.events, timeline->predicted_events, ...);
  //
  // 3. Copy interventions array
  //    snapshot.num_interventions = timeline->num_interventions;
  //    memcpy(snapshot.interventions, timeline->interventions, ...);
  //
  // 4. Write to shared memory buffer
  //    TimelinesFrame* frame = &server->timelines_ptr[...];
  //    frame->timelines[i] = snapshot;
  //
  // 5. Checksum, signal, swap as before

  // Stub implementation
  pthread_mutex_lock(&server->stats_mutex);
  server->timelines_published++;
  pthread_mutex_unlock(&server->stats_mutex);

  return true;
}

/**
 * Update statistics JSON file
 */
bool ipc_server_update_stats(
  IPCServer* server,
  const OmnisightStats* stats) {

  if (!server || !stats) return false;

  // TODO Phase 2.3: Implement atomic JSON write
  // 1. Build JSON string
  //    char json[4096];
  //    snprintf(json, sizeof(json),
  //      "{"
  //      "\"timestamp\": %llu,"
  //      "\"uptime_ms\": %llu,"
  //      "\"perception\": {"
  //      "  \"enabled\": true,"
  //      "  \"fps\": %.1f,"
  //      "  \"objects_tracked\": %u"
  //      "},"
  //      "...",
  //      ipc_get_timestamp_ms(),
  //      stats->uptime_ms,
  //      stats->perception_stats.avg_fps,
  //      stats->perception_stats.tracked_objects
  //    );
  //
  // 2. Write to temporary file
  //    FILE* f = fopen("/tmp/omnisight_stats.json.tmp", "w");
  //    fprintf(f, "%s", json);
  //    fclose(f);
  //
  // 3. Atomic rename
  //    rename("/tmp/omnisight_stats.json.tmp", OMNISIGHT_JSON_STATS);
  //
  // This ensures Python never reads partial JSON

  return true;
}

/**
 * Update status JSON file
 */
bool ipc_server_update_status(
  IPCServer* server,
  const char* status) {

  if (!server || !status) return false;

  // TODO Phase 2.3: Implement status file write
  // Similar to update_stats but simpler JSON:
  // {
  //   "timestamp": 1698765432000,
  //   "status": "running",
  //   "pid": 12345,
  //   "version": "0.2.0",
  //   "ipc": {
  //     "shm_detections": "/omnisight_detections",
  //     ...
  //   }
  // }

  return true;
}

/**
 * Send acknowledgment
 */
bool ipc_server_send_ack(
  IPCServer* server,
  const char* request_id,
  bool success,
  const char* message) {

  if (!server || !request_id) return false;

  // TODO Phase 2.2: Implement acknowledgment send
  // 1. Build JSON ack
  //    char ack_json[1024];
  //    snprintf(ack_json, sizeof(ack_json),
  //      "{"
  //      "\"request_id\": \"%s\","
  //      "\"status\": \"%s\","
  //      "\"timestamp\": %llu,"
  //      "\"message\": \"%s\""
  //      "}",
  //      request_id,
  //      success ? "success" : "error",
  //      ipc_get_timestamp_ms(),
  //      message ? message : ""
  //    );
  //
  // 2. Write to ack pipe (non-blocking)
  //    write(server->ack_fd, ack_json, strlen(ack_json));

  return true;
}

/**
 * Get statistics
 */
void ipc_server_get_stats(
  IPCServer* server,
  uint64_t* frames_published,
  uint64_t* tracks_published,
  uint64_t* timelines_published,
  uint64_t* commands_processed,
  uint64_t* errors) {

  if (!server) return;

  pthread_mutex_lock(&server->stats_mutex);
  if (frames_published) *frames_published = server->frames_published;
  if (tracks_published) *tracks_published = server->tracks_published;
  if (timelines_published) *timelines_published = server->timelines_published;
  if (commands_processed) *commands_processed = server->commands_processed;
  if (errors) *errors = server->errors;
  pthread_mutex_unlock(&server->stats_mutex);
}

/**
 * Destroy IPC server
 */
void ipc_server_destroy(IPCServer* server) {
  if (!server) return;

  // Stop server if running
  if (server->running) {
    ipc_server_stop(server);
  }

  // TODO Phase 2.1: Cleanup all resources
  // - Close and unlink shared memory
  // - Close and unlink semaphores
  // - Close and unlink pipes
  // - Destroy mutex
  // - Free memory
  ipc_cleanup_resources(server);

  pthread_mutex_destroy(&server->stats_mutex);
  free(server);

  printf("IPC: Server destroyed\n");
}

/**
 * Command handler thread
 */
static void* ipc_command_handler_thread(void* arg) {
  IPCServer* server = (IPCServer*)arg;
  char buffer[4096];

  printf("IPC: Command handler thread started\n");

  while (server->running) {
    // TODO Phase 2.2: Implement command handling loop
    // 1. Read from command pipe with timeout
    //    ssize_t n = read(server->cmd_fd, buffer, sizeof(buffer) - 1);
    //
    // 2. If data received, parse JSON
    //    if (n > 0) {
    //      buffer[n] = '\0';
    //
    //      // Parse command type and request_id
    //      IPCCommandType cmd_type = ipc_parse_command_type(buffer);
    //      const char* request_id = ...; // Extract from JSON
    //
    //      // Call callback
    //      bool success = false;
    //      if (server->config.command_callback) {
    //        success = server->config.command_callback(
    //          cmd_type, buffer, server->config.callback_user_data);
    //      }
    //
    //      // Send acknowledgment
    //      ipc_server_send_ack(server, request_id, success, NULL);
    //
    //      // Update stats
    //      pthread_mutex_lock(&server->stats_mutex);
    //      server->commands_processed++;
    //      pthread_mutex_unlock(&server->stats_mutex);
    //    }
    //
    // 3. Sleep briefly to avoid busy-wait
    //    usleep(10000);  // 10ms

    usleep(100000);  // 100ms (stub)
  }

  printf("IPC: Command handler thread stopped\n");
  return NULL;
}

/**
 * Create shared memory regions
 */
static bool ipc_create_shared_memory(IPCServer* server) {
  // TODO Phase 2.1: Implement shared memory creation
  //
  // For each region (detections, tracks, timelines):
  // 1. Open/create shared memory
  //    server->detections_fd = shm_open(
  //      OMNISIGHT_SHM_DETECTIONS,
  //      O_CREAT | O_RDWR,
  //      0666
  //    );
  //    if (server->detections_fd < 0) {
  //      perror("shm_open");
  //      return false;
  //    }
  //
  // 2. Set size (double if double-buffering)
  //    size_t size = sizeof(DetectionFrame);
  //    if (server->config.double_buffer) size *= 2;
  //    ftruncate(server->detections_fd, size);
  //
  // 3. Map to memory
  //    server->detections_ptr = mmap(
  //      NULL, size,
  //      PROT_READ | PROT_WRITE,
  //      MAP_SHARED,
  //      server->detections_fd, 0
  //    );
  //    if (server->detections_ptr == MAP_FAILED) {
  //      perror("mmap");
  //      return false;
  //    }
  //
  // 4. Initialize memory
  //    memset(server->detections_ptr, 0, size);
  //
  // Repeat for tracks and timelines

  return true;  // Stub
}

/**
 * Create semaphores
 */
static bool ipc_create_semaphores(IPCServer* server) {
  // TODO Phase 2.1: Implement semaphore creation
  //
  // For each semaphore (frame_ready, tracks_ready, timelines_ready):
  // 1. Open/create semaphore
  //    server->frame_ready_sem = sem_open(
  //      OMNISIGHT_SEM_FRAME_READY,
  //      O_CREAT,
  //      0666,
  //      0  // Initial value: 0 (no data available)
  //    );
  //    if (server->frame_ready_sem == SEM_FAILED) {
  //      perror("sem_open");
  //      return false;
  //    }
  //
  // 2. Reset to 0 in case of stale value from previous run
  //    while (sem_trywait(server->frame_ready_sem) == 0);
  //
  // Repeat for other semaphores

  return true;  // Stub
}

/**
 * Create named pipes
 */
static bool ipc_create_pipes(IPCServer* server) {
  // TODO Phase 2.2: Implement named pipe creation
  //
  // 1. Create FIFOs (ignore error if already exists)
  //    mkfifo(OMNISIGHT_PIPE_CMD, 0666);
  //    mkfifo(OMNISIGHT_PIPE_ACK, 0666);
  //
  // 2. Open command pipe for reading (non-blocking)
  //    server->cmd_fd = open(OMNISIGHT_PIPE_CMD, O_RDONLY | O_NONBLOCK);
  //    if (server->cmd_fd < 0) {
  //      perror("open cmd pipe");
  //      return false;
  //    }
  //
  // 3. Open ack pipe for writing
  //    server->ack_fd = open(OMNISIGHT_PIPE_ACK, O_WRONLY);
  //    if (server->ack_fd < 0) {
  //      perror("open ack pipe");
  //      return false;
  //    }

  return true;  // Stub
}

/**
 * Cleanup resources
 */
static void ipc_cleanup_resources(IPCServer* server) {
  // TODO Phase 2.1: Implement resource cleanup
  //
  // Shared memory:
  // - munmap(server->detections_ptr, ...)
  // - close(server->detections_fd)
  // - shm_unlink(OMNISIGHT_SHM_DETECTIONS)
  //
  // Semaphores:
  // - sem_close(server->frame_ready_sem)
  // - sem_unlink(OMNISIGHT_SEM_FRAME_READY)
  //
  // Pipes:
  // - close(server->cmd_fd)
  // - close(server->ack_fd)
  // - unlink(OMNISIGHT_PIPE_CMD)
  // - unlink(OMNISIGHT_PIPE_ACK)
}

/**
 * Calculate CRC32 checksum
 */
uint32_t ipc_crc32(const void* data, size_t size) {
  // TODO Phase 2.1: Implement CRC32
  // Use standard CRC32 algorithm or library (e.g., zlib)
  // For now, return 0 (checksum disabled in stub)
  return 0;
}

/**
 * Get current timestamp in milliseconds
 */
uint64_t ipc_get_timestamp_ms(void) {
  struct timespec ts;
  clock_gettime(CLOCK_REALTIME, &ts);
  return (uint64_t)ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
}

/**
 * Parse command type from JSON
 */
IPCCommandType ipc_parse_command_type(const char* json) {
  // TODO Phase 2.2: Implement JSON parsing
  // Use simple string search or JSON library (cJSON)
  //
  // if (strstr(json, "\"type\":\"PING\"")) return IPC_CMD_PING;
  // if (strstr(json, "\"type\":\"CONFIG_UPDATE\"")) return IPC_CMD_CONFIG_UPDATE;
  // ...

  return IPC_CMD_UNKNOWN;  // Stub
}
