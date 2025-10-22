/**
 * @file swarm_stub.c
 * @brief Stub implementation of Swarm Intelligence for testing
 *
 * Simulates swarm coordination without actual MQTT networking.
 */

#include "swarm.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <math.h>

// Stub swarm node structure
struct SwarmNode {
    SwarmConfig config;
    bool is_running;

    // Simulated neighbors
    NeighborInfo neighbors[5];
    uint32_t num_neighbors;

    // Buffered shared data
    SharedTrack shared_tracks[SWARM_MAX_SHARED_TRACKS];
    uint32_t num_shared_tracks;

    SharedEvent shared_events[100];
    uint32_t num_shared_events;

    // Statistics
    SwarmStats stats;

    // Track ID mapping
    uint64_t next_global_track_id;
};

// ============================================================================
// Utility Functions
// ============================================================================

static uint64_t get_current_time_ms(void) {
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    return (uint64_t)ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
}

static float randf(float min, float max) {
    return min + (float)rand() / RAND_MAX * (max - min);
}

/**
 * Create simulated neighbors
 */
static void create_simulated_neighbors(SwarmNode* node) {
    // Simulate 2-4 neighboring cameras
    node->num_neighbors = 2 + (rand() % 3);
    if (node->num_neighbors > 5) node->num_neighbors = 5;

    for (uint32_t i = 0; i < node->num_neighbors; i++) {
        NeighborInfo* neighbor = &node->neighbors[i];

        neighbor->camera_id = node->config.camera_id + i + 1;
        neighbor->geometry.x = randf(-20.0f, 20.0f);
        neighbor->geometry.y = randf(-20.0f, 20.0f);
        neighbor->geometry.z = 3.0f;
        neighbor->geometry.fov_horizontal = 90.0f;
        neighbor->geometry.fov_vertical = 60.0f;
        neighbor->geometry.coverage_radius = 15.0f;
        neighbor->overlap_area = randf(0.1f, 0.4f);
        neighbor->last_heartbeat_ms = get_current_time_ms();
        neighbor->is_online = true;
        neighbor->link_quality = randf(0.8f, 1.0f);
    }

    printf("[Swarm] Simulated %u neighboring cameras\n", node->num_neighbors);
}

// ============================================================================
// Public API Implementation
// ============================================================================

SwarmNode* swarm_init(const SwarmConfig* config) {
    if (!config) {
        return NULL;
    }

    printf("[Swarm] Initializing Swarm Intelligence stub...\n");

    SwarmNode* node = (SwarmNode*)calloc(1, sizeof(SwarmNode));
    if (!node) {
        return NULL;
    }

    node->config = *config;
    node->is_running = false;
    node->num_neighbors = 0;
    node->num_shared_tracks = 0;
    node->num_shared_events = 0;
    node->next_global_track_id = (uint64_t)config->camera_id * 1000000;

    // Initialize stats
    memset(&node->stats, 0, sizeof(SwarmStats));
    node->stats.network_health = 1.0f;

    // Create simulated neighbors
    create_simulated_neighbors(node);

    printf("[Swarm] ✓ Swarm Intelligence stub initialized\n");
    printf("[Swarm]   - Camera ID: %u\n", config->camera_id);
    printf("[Swarm]   - Role: %d\n", config->role);
    printf("[Swarm]   - Neighbors: %u\n", node->num_neighbors);
    printf("[Swarm]   - Simulated mode (no MQTT)\n");

    return node;
}

void swarm_destroy(SwarmNode* node) {
    if (!node) return;

    if (node->is_running) {
        swarm_stop(node);
    }

    free(node);
    printf("[Swarm] ✓ Swarm Intelligence destroyed\n");
}

bool swarm_start(SwarmNode* node) {
    if (!node || node->is_running) {
        return false;
    }

    printf("[Swarm] Starting swarm coordination (stub)...\n");

    node->is_running = true;
    node->stats.num_cameras = node->num_neighbors + 1;
    node->stats.num_online = node->num_neighbors;
    node->stats.num_neighbors = node->num_neighbors;

    printf("[Swarm] ✓ Swarm coordination active\n");
    return true;
}

void swarm_stop(SwarmNode* node) {
    if (!node || !node->is_running) {
        return;
    }

    printf("[Swarm] Stopping swarm coordination...\n");
    node->is_running = false;
    printf("[Swarm] ✓ Swarm coordination stopped\n");
}

uint32_t swarm_update_tracks(SwarmNode* node,
                              const TrackedObject* tracks,
                              uint32_t num_tracks,
                              uint64_t current_time_ms) {
    if (!node || !node->is_running || !tracks) {
        return 0;
    }

    // Convert tracks to shared tracks
    uint32_t num_shared = 0;
    for (uint32_t i = 0; i < num_tracks && i < SWARM_MAX_SHARED_TRACKS; i++) {
        SharedTrack* shared = &node->shared_tracks[i];

        shared->camera_id = node->config.camera_id;
        shared->track_id = tracks[i].track_id;
        shared->global_track_id = node->next_global_track_id++;

        shared->first_seen_ms = tracks[i].first_seen_ms;
        shared->last_seen_ms = tracks[i].last_seen_ms;
        shared->exit_time_ms = current_time_ms;

        // Convert to world coordinates (stub: just use normalized)
        shared->world_x = tracks[i].current_bbox.x * 10.0f;
        shared->world_y = tracks[i].current_bbox.y * 10.0f;
        shared->velocity_x = tracks[i].velocity_x * 10.0f;
        shared->velocity_y = tracks[i].velocity_y * 10.0f;
        shared->exit_direction = atan2f(shared->velocity_y, shared->velocity_x);

        // Copy features
        memcpy(shared->features, tracks[i].features, sizeof(shared->features));
        shared->feature_confidence = tracks[i].confidence;

        shared->object_class = tracks[i].class_id;
        shared->height_meters = 1.7f;  // Stub: assume average height

        shared->behaviors = tracks[i].behaviors;
        shared->threat_score = tracks[i].threat_score;

        // Predict next position
        shared->predicted_next_x = shared->world_x + shared->velocity_x;
        shared->predicted_next_y = shared->world_y + shared->velocity_y;
        shared->prediction_confidence = 0.8f;

        num_shared++;
    }

    node->num_shared_tracks = num_shared;
    node->stats.tracks_shared += num_shared;
    node->stats.messages_sent += num_shared;

    return num_shared;
}

bool swarm_share_event(SwarmNode* node,
                       const PredictedEvent* event,
                       const Timeline* timeline) {
    if (!node || !node->is_running || !event) {
        return false;
    }

    (void)timeline;  // Unused in stub

    if (node->num_shared_events >= 100) {
        // Evict oldest
        memmove(&node->shared_events[0], &node->shared_events[1],
                99 * sizeof(SharedEvent));
        node->num_shared_events = 99;
    }

    SharedEvent* shared = &node->shared_events[node->num_shared_events];
    shared->camera_id = node->config.camera_id;
    shared->event_id = node->num_shared_events;
    shared->global_event_id = (uint64_t)node->config.camera_id * 1000000 +
                              node->num_shared_events;
    shared->type = event->type;
    shared->severity = event->severity;
    shared->timestamp_ms = event->timestamp_ms;
    shared->probability = event->probability;
    shared->world_x = event->location_x * 10.0f;
    shared->world_y = event->location_y * 10.0f;
    shared->num_tracks = event->num_involved;
    for (uint32_t i = 0; i < event->num_involved; i++) {
        shared->global_track_ids[i] = event->involved_tracks[i];
    }

    // Simulate consensus
    shared->votes_for = 1 + (rand() % node->num_neighbors);
    shared->votes_against = node->num_neighbors - shared->votes_for;
    shared->consensus_confidence = (float)shared->votes_for /
                                   (shared->votes_for + shared->votes_against);

    node->num_shared_events++;
    node->stats.events_shared++;
    node->stats.messages_sent++;

    if (shared->consensus_confidence > 0.6f) {
        node->stats.consensus_reached++;
    }

    return true;
}

bool swarm_request_consensus(SwarmNode* node,
                              const SharedEvent* event,
                              uint32_t timeout_ms,
                              float* consensus_out) {
    if (!node || !node->is_running || !event) {
        return false;
    }

    (void)timeout_ms;  // Stub: immediate response

    // Simulate consensus voting
    uint32_t votes_for = 1 + (rand() % (node->num_neighbors + 1));
    uint32_t total_votes = node->num_neighbors + 1;
    float consensus = (float)votes_for / total_votes;

    if (consensus_out) {
        *consensus_out = consensus;
    }

    return consensus > 0.5f;
}

bool swarm_share_model_update(SwarmNode* node,
                               const ModelUpdate* update) {
    if (!node || !node->is_running || !update) {
        return false;
    }

    node->stats.model_updates_sent++;
    node->stats.messages_sent++;

    return true;
}

uint32_t swarm_get_neighbor_tracks(SwarmNode* node,
                                    SharedTrack* tracks,
                                    uint32_t max_tracks) {
    if (!node || !tracks) return 0;

    // Stub: Return some of our own tracks as if from neighbors
    uint32_t count = node->num_shared_tracks < max_tracks ?
                     node->num_shared_tracks : max_tracks;
    memcpy(tracks, node->shared_tracks, count * sizeof(SharedTrack));

    node->stats.tracks_received += count;
    return count;
}

uint32_t swarm_get_shared_events(SwarmNode* node,
                                  SharedEvent* events,
                                  uint32_t max_events) {
    if (!node || !events) return 0;

    uint32_t count = node->num_shared_events < max_events ?
                     node->num_shared_events : max_events;
    memcpy(events, node->shared_events, count * sizeof(SharedEvent));

    return count;
}

bool swarm_handoff_track(SwarmNode* node,
                          const SharedTrack* track,
                          uint32_t neighbor_id) {
    if (!node || !node->is_running || !track) {
        return false;
    }

    // Stub: Simulate successful handoff
    (void)neighbor_id;

    node->stats.successful_handoffs++;
    node->stats.messages_sent++;

    return true;
}

bool swarm_add_neighbor(SwarmNode* node,
                        const NeighborInfo* neighbor) {
    if (!node || !neighbor || node->num_neighbors >= 5) {
        return false;
    }

    node->neighbors[node->num_neighbors] = *neighbor;
    node->num_neighbors++;
    node->stats.num_neighbors = node->num_neighbors;

    return true;
}

void swarm_remove_neighbor(SwarmNode* node,
                           uint32_t camera_id) {
    if (!node) return;

    for (uint32_t i = 0; i < node->num_neighbors; i++) {
        if (node->neighbors[i].camera_id == camera_id) {
            // Shift remaining neighbors
            memmove(&node->neighbors[i], &node->neighbors[i + 1],
                    (node->num_neighbors - i - 1) * sizeof(NeighborInfo));
            node->num_neighbors--;
            node->stats.num_neighbors = node->num_neighbors;
            return;
        }
    }
}

void swarm_get_stats(const SwarmNode* node,
                     SwarmStats* stats) {
    if (!node || !stats) return;

    *stats = node->stats;

    // Update derived metrics
    stats->avg_network_latency_ms = randf(5.0f, 20.0f);
    stats->network_health = node->is_running ? randf(0.85f, 1.0f) : 0.0f;
}

float swarm_calculate_fov_overlap(const CameraGeometry* camera1,
                                   const CameraGeometry* camera2) {
    if (!camera1 || !camera2) return 0.0f;

    // Stub: Simple distance-based overlap calculation
    float dx = camera2->x - camera1->x;
    float dy = camera2->y - camera1->y;
    float distance = sqrtf(dx*dx + dy*dy);

    float combined_radius = camera1->coverage_radius + camera2->coverage_radius;

    if (distance >= combined_radius) {
        return 0.0f;
    }

    float overlap = 1.0f - (distance / combined_radius);
    return overlap;
}

bool swarm_image_to_world(const CameraGeometry* camera,
                           float image_x,
                           float image_y,
                           float* world_x,
                           float* world_y) {
    if (!camera || !world_x || !world_y) return false;

    // Stub: Simple projection (not geometrically accurate)
    float half_fov_h = camera->fov_horizontal / 2.0f * 3.14159f / 180.0f;
    float half_fov_v = camera->fov_vertical / 2.0f * 3.14159f / 180.0f;

    *world_x = camera->x + tanf(half_fov_h * (image_x - 0.5f)) *
               camera->coverage_radius;
    *world_y = camera->y + tanf(half_fov_v * (image_y - 0.5f)) *
               camera->coverage_radius;

    return true;
}
