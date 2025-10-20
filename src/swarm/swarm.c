/**
 * @file swarm.c
 * @brief Swarm Intelligence implementation
 *
 * Enables privacy-preserving camera-to-camera coordination.
 */

#include "swarm.h"
#include "mqtt_client.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <pthread.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// ============================================================================
// Internal Structures
// ============================================================================

/**
 * Swarm node state
 */
struct SwarmNode {
    SwarmConfig config;
    MqttClient* mqtt;

    // Neighbors
    uint32_t num_neighbors;
    NeighborInfo neighbors[SWARM_MAX_NEIGHBORS];

    // Shared data buffers
    SharedTrack shared_tracks[SWARM_MAX_SHARED_TRACKS];
    uint32_t num_shared_tracks;

    SharedEvent shared_events[100];
    uint32_t num_shared_events;

    // Track handoff management
    uint64_t next_global_track_id;
    struct {
        uint32_t local_id;
        uint64_t global_id;
    } track_id_map[100];
    uint32_t num_track_mappings;

    // Statistics
    SwarmStats stats;

    // Threading
    pthread_mutex_t mutex;
    bool is_running;
};

// ============================================================================
// Utility Functions
// ============================================================================

static uint64_t get_current_time_ms() {
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    return (uint64_t)ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
}

static float clamp(float value, float min, float max) {
    if (value < min) return min;
    if (value > max) return max;
    return value;
}

static float distance_2d(float x1, float y1, float x2, float y2) {
    float dx = x2 - x1;
    float dy = y2 - y1;
    return sqrtf(dx*dx + dy*dy);
}

/**
 * Get or create global track ID
 */
static uint64_t swarm_get_global_track_id(SwarmNode* node, uint32_t local_id) {
    // Check if mapping exists
    for (uint32_t i = 0; i < node->num_track_mappings; i++) {
        if (node->track_id_map[i].local_id == local_id) {
            return node->track_id_map[i].global_id;
        }
    }

    // Create new mapping
    if (node->num_track_mappings >= 100) {
        // Evict oldest
        node->num_track_mappings = 99;
    }

    uint64_t global_id = node->next_global_track_id++;
    node->track_id_map[node->num_track_mappings].local_id = local_id;
    node->track_id_map[node->num_track_mappings].global_id = global_id;
    node->num_track_mappings++;

    return global_id;
}

// ============================================================================
// Message Handling
// ============================================================================

/**
 * Handle incoming MQTT message
 */
static void on_mqtt_message(
    const char* topic,
    const uint8_t* payload,
    uint32_t payload_len,
    void* user_data
) {
    SwarmNode* node = (SwarmNode*)user_data;

    pthread_mutex_lock(&node->mutex);

    // Parse message type from topic
    // Topic format: omnisight/{camera_id}/{message_type}

    if (strstr(topic, "/track_share")) {
        // Received shared track from neighbor
        if (payload_len == sizeof(SharedTrack)) {
            SharedTrack* track = (SharedTrack*)payload;

            // Add to shared tracks buffer
            if (node->num_shared_tracks < SWARM_MAX_SHARED_TRACKS) {
                node->shared_tracks[node->num_shared_tracks++] = *track;
                node->stats.tracks_received++;
            }
        }
    }
    else if (strstr(topic, "/event_share")) {
        // Received shared event from neighbor
        if (payload_len == sizeof(SharedEvent)) {
            SharedEvent* event = (SharedEvent*)payload;

            // Add to shared events buffer
            if (node->num_shared_events < 100) {
                node->shared_events[node->num_shared_events++] = *event;
            }
        }
    }
    else if (strstr(topic, "/heartbeat")) {
        // Update neighbor heartbeat
        if (payload_len >= sizeof(uint32_t)) {
            uint32_t camera_id = *(uint32_t*)payload;

            for (uint32_t i = 0; i < node->num_neighbors; i++) {
                if (node->neighbors[i].camera_id == camera_id) {
                    node->neighbors[i].last_heartbeat_ms = get_current_time_ms();
                    node->neighbors[i].is_online = true;
                    break;
                }
            }
        }
    }
    else if (strstr(topic, "/handoff")) {
        // Track handoff request
        // TODO: Handle track handoff acceptance
    }

    node->stats.messages_received++;
    node->stats.bytes_received += payload_len;

    pthread_mutex_unlock(&node->mutex);
}

/**
 * Handle MQTT connection status
 */
static void on_mqtt_status(MqttStatus status, void* user_data) {
    SwarmNode* node = (SwarmNode*)user_data;

    if (status == MQTT_STATUS_CONNECTED) {
        printf("[Swarm] Connected to MQTT broker\n");

        // Subscribe to relevant topics
        char topic[MQTT_MAX_TOPIC_LENGTH];

        // Subscribe to track sharing from neighbors
        snprintf(topic, sizeof(topic), "omnisight/+/track_share");
        mqtt_client_subscribe(node->mqtt, topic, MQTT_QOS_AT_LEAST_ONCE);

        // Subscribe to event sharing
        snprintf(topic, sizeof(topic), "omnisight/+/event_share");
        mqtt_client_subscribe(node->mqtt, topic, MQTT_QOS_AT_LEAST_ONCE);

        // Subscribe to heartbeats
        snprintf(topic, sizeof(topic), "omnisight/+/heartbeat");
        mqtt_client_subscribe(node->mqtt, topic, MQTT_QOS_AT_MOST_ONCE);

        // Subscribe to handoff requests
        snprintf(topic, sizeof(topic), "omnisight/%u/handoff", node->config.camera_id);
        mqtt_client_subscribe(node->mqtt, topic, MQTT_QOS_AT_LEAST_ONCE);
    }
    else if (status == MQTT_STATUS_DISCONNECTED) {
        printf("[Swarm] Disconnected from MQTT broker\n");
    }
}

// ============================================================================
// Public API Implementation
// ============================================================================

SwarmNode* swarm_init(const SwarmConfig* config) {
    if (!config) {
        fprintf(stderr, "[Swarm] ERROR: NULL config\n");
        return NULL;
    }

    SwarmNode* node = calloc(1, sizeof(SwarmNode));
    if (!node) {
        fprintf(stderr, "[Swarm] ERROR: Failed to allocate memory\n");
        return NULL;
    }

    // Copy configuration
    memcpy(&node->config, config, sizeof(SwarmConfig));

    // Initialize track ID generator
    node->next_global_track_id = ((uint64_t)config->camera_id << 32) | 1;

    // Initialize mutex
    pthread_mutex_init(&node->mutex, NULL);

    // Initialize statistics
    memset(&node->stats, 0, sizeof(SwarmStats));

    // Initialize MQTT client
    MqttConfig mqtt_config = {
        .broker_address = config->mqtt_broker,
        .broker_port = config->mqtt_port,
        .client_id = config->mqtt_client_id,
        .username = config->mqtt_username,
        .password = config->mqtt_password,
        .keepalive_interval_s = 60,
        .clean_session = true,
        .connect_timeout_s = 30,
        .use_tls = false,
        .on_message = on_mqtt_message,
        .on_status = on_mqtt_status,
        .user_data = node,
        .default_qos = MQTT_QOS_AT_LEAST_ONCE
    };

    node->mqtt = mqtt_client_init(&mqtt_config);
    if (!node->mqtt) {
        fprintf(stderr, "[Swarm] ERROR: Failed to initialize MQTT client\n");
        pthread_mutex_destroy(&node->mutex);
        free(node);
        return NULL;
    }

    printf("[Swarm] Node initialized (ID=%u, Role=%d)\n",
           config->camera_id, config->role);

    return node;
}

void swarm_destroy(SwarmNode* node) {
    if (!node) return;

    if (node->is_running) {
        swarm_stop(node);
    }

    mqtt_client_destroy(node->mqtt);
    pthread_mutex_destroy(&node->mutex);
    free(node);
}

bool swarm_start(SwarmNode* node) {
    if (!node) return false;

    if (!mqtt_client_connect(node->mqtt)) {
        fprintf(stderr, "[Swarm] ERROR: Failed to connect to MQTT broker\n");
        return false;
    }

    node->is_running = true;
    printf("[Swarm] Node started\n");

    return true;
}

void swarm_stop(SwarmNode* node) {
    if (!node) return;

    node->is_running = false;
    mqtt_client_disconnect(node->mqtt);

    printf("[Swarm] Node stopped\n");
}

uint32_t swarm_update_tracks(
    SwarmNode* node,
    const TrackedObject* tracks,
    uint32_t num_tracks,
    uint64_t current_time_ms
) {
    if (!node || !tracks || num_tracks == 0) {
        return 0;
    }

    if (!mqtt_client_is_connected(node->mqtt)) {
        return 0;
    }

    pthread_mutex_lock(&node->mutex);

    uint32_t num_shared = 0;

    // Share tracks based on privacy level
    for (uint32_t i = 0; i < num_tracks && num_shared < SWARM_MAX_SHARED_TRACKS; i++) {
        const TrackedObject* track = &tracks[i];

        // Create shared track
        SharedTrack shared;
        memset(&shared, 0, sizeof(SharedTrack));

        shared.camera_id = node->config.camera_id;
        shared.track_id = track->track_id;
        shared.global_track_id = swarm_get_global_track_id(node, track->track_id);

        shared.first_seen_ms = track->first_seen_ms;
        shared.last_seen_ms = track->last_seen_ms;

        // Convert image coordinates to world coordinates
        float image_x = track->box.x + track->box.width / 2;
        float image_y = track->box.y + track->box.height / 2;
        swarm_image_to_world(&node->config.geometry, image_x, image_y,
                            &shared.world_x, &shared.world_y);

        shared.velocity_x = track->velocity_x;
        shared.velocity_y = track->velocity_y;

        // Privacy-preserving features (simulated - would come from feature extractor)
        shared.feature_confidence = 0.8f;
        for (int j = 0; j < SWARM_FEATURE_DIM; j++) {
            shared.features[j] = ((float)rand() / RAND_MAX) * 2 - 1;  // Placeholder
        }

        shared.object_class = track->class_id;
        shared.height_meters = track->box.height / 100.0f;  // Rough estimate

        shared.behaviors = track->behaviors;
        shared.threat_score = track->threat_score;

        // Publish to MQTT
        char topic[MQTT_MAX_TOPIC_LENGTH];
        snprintf(topic, sizeof(topic), "omnisight/%u/track_share", node->config.camera_id);

        if (mqtt_client_publish(node->mqtt, topic,
                               (uint8_t*)&shared, sizeof(SharedTrack),
                               MQTT_QOS_AT_LEAST_ONCE, false)) {
            num_shared++;
            node->stats.tracks_shared++;
            node->stats.messages_sent++;
            node->stats.bytes_sent += sizeof(SharedTrack);
        }
    }

    pthread_mutex_unlock(&node->mutex);

    // Process incoming messages
    mqtt_client_process(node->mqtt, 0);

    return num_shared;
}

bool swarm_share_event(
    SwarmNode* node,
    const PredictedEvent* event,
    const Timeline* timeline
) {
    if (!node || !event) {
        return false;
    }

    if (!mqtt_client_is_connected(node->mqtt)) {
        return false;
    }

    pthread_mutex_lock(&node->mutex);

    // Create shared event
    SharedEvent shared;
    memset(&shared, 0, sizeof(SharedEvent));

    shared.camera_id = node->config.camera_id;
    shared.event_id = node->stats.events_shared;  // Use counter as event ID
    shared.global_event_id = ((uint64_t)node->config.camera_id << 32) | shared.event_id;

    shared.type = event->type;
    shared.severity = event->severity;
    shared.timestamp_ms = event->timestamp_ms;
    shared.probability = event->probability;

    // Convert to world coordinates
    swarm_image_to_world(&node->config.geometry,
                        event->location_x, event->location_y,
                        &shared.world_x, &shared.world_y);

    shared.num_tracks = event->num_involved_tracks;
    for (uint32_t i = 0; i < event->num_involved_tracks && i < 4; i++) {
        shared.global_track_ids[i] = swarm_get_global_track_id(node, event->involved_tracks[i]);
    }

    // Publish to MQTT
    char topic[MQTT_MAX_TOPIC_LENGTH];
    snprintf(topic, sizeof(topic), "omnisight/%u/event_share", node->config.camera_id);

    bool success = mqtt_client_publish(node->mqtt, topic,
                                      (uint8_t*)&shared, sizeof(SharedEvent),
                                      MQTT_QOS_AT_LEAST_ONCE, false);

    if (success) {
        node->stats.events_shared++;
        node->stats.messages_sent++;
        node->stats.bytes_sent += sizeof(SharedEvent);
    }

    pthread_mutex_unlock(&node->mutex);

    return success;
}

bool swarm_request_consensus(
    SwarmNode* node,
    const SharedEvent* event,
    uint32_t timeout_ms,
    float* consensus_out
) {
    if (!node || !event || !consensus_out) {
        return false;
    }

    // TODO: Implement consensus protocol
    // For now, simple majority voting

    *consensus_out = 0.7f;  // Placeholder
    node->stats.consensus_reached++;

    return true;
}

bool swarm_share_model_update(
    SwarmNode* node,
    const ModelUpdate* update
) {
    if (!node || !update) {
        return false;
    }

    if (!mqtt_client_is_connected(node->mqtt)) {
        return false;
    }

    // TODO: Implement federated learning message serialization
    // For now, just log

    printf("[Swarm] Model update shared (version=%u, samples=%u)\n",
           update->model_version, update->num_samples);

    return true;
}

uint32_t swarm_get_neighbor_tracks(
    SwarmNode* node,
    SharedTrack* tracks,
    uint32_t max_tracks
) {
    if (!node || !tracks || max_tracks == 0) {
        return 0;
    }

    pthread_mutex_lock(&node->mutex);

    uint32_t num_to_copy = node->num_shared_tracks < max_tracks ?
                          node->num_shared_tracks : max_tracks;

    memcpy(tracks, node->shared_tracks, num_to_copy * sizeof(SharedTrack));

    // Clear buffer after retrieval
    node->num_shared_tracks = 0;

    pthread_mutex_unlock(&node->mutex);

    return num_to_copy;
}

uint32_t swarm_get_shared_events(
    SwarmNode* node,
    SharedEvent* events,
    uint32_t max_events
) {
    if (!node || !events || max_events == 0) {
        return 0;
    }

    pthread_mutex_lock(&node->mutex);

    uint32_t num_to_copy = node->num_shared_events < max_events ?
                          node->num_shared_events : max_events;

    memcpy(events, node->shared_events, num_to_copy * sizeof(SharedEvent));

    // Clear buffer after retrieval
    node->num_shared_events = 0;

    pthread_mutex_unlock(&node->mutex);

    return num_to_copy;
}

bool swarm_handoff_track(
    SwarmNode* node,
    const SharedTrack* track,
    uint32_t neighbor_id
) {
    if (!node || !track) {
        return false;
    }

    if (!mqtt_client_is_connected(node->mqtt)) {
        return false;
    }

    // Publish handoff request to specific neighbor
    char topic[MQTT_MAX_TOPIC_LENGTH];
    snprintf(topic, sizeof(topic), "omnisight/%u/handoff", neighbor_id);

    bool success = mqtt_client_publish(node->mqtt, topic,
                                      (uint8_t*)track, sizeof(SharedTrack),
                                      MQTT_QOS_AT_LEAST_ONCE, false);

    if (success) {
        node->stats.successful_handoffs++;
    }

    return success;
}

bool swarm_add_neighbor(
    SwarmNode* node,
    const NeighborInfo* neighbor
) {
    if (!node || !neighbor) {
        return false;
    }

    pthread_mutex_lock(&node->mutex);

    // Check if already exists
    for (uint32_t i = 0; i < node->num_neighbors; i++) {
        if (node->neighbors[i].camera_id == neighbor->camera_id) {
            // Update existing
            node->neighbors[i] = *neighbor;
            pthread_mutex_unlock(&node->mutex);
            return true;
        }
    }

    // Add new neighbor
    if (node->num_neighbors >= SWARM_MAX_NEIGHBORS) {
        fprintf(stderr, "[Swarm] ERROR: Max neighbors reached\n");
        pthread_mutex_unlock(&node->mutex);
        return false;
    }

    node->neighbors[node->num_neighbors++] = *neighbor;
    node->stats.num_neighbors = node->num_neighbors;

    pthread_mutex_unlock(&node->mutex);

    printf("[Swarm] Neighbor added (ID=%u, overlap=%.2f)\n",
           neighbor->camera_id, neighbor->overlap_area);

    return true;
}

void swarm_remove_neighbor(
    SwarmNode* node,
    uint32_t camera_id
) {
    if (!node) return;

    pthread_mutex_lock(&node->mutex);

    for (uint32_t i = 0; i < node->num_neighbors; i++) {
        if (node->neighbors[i].camera_id == camera_id) {
            // Remove by shifting
            for (uint32_t j = i; j < node->num_neighbors - 1; j++) {
                node->neighbors[j] = node->neighbors[j + 1];
            }
            node->num_neighbors--;
            node->stats.num_neighbors = node->num_neighbors;
            break;
        }
    }

    pthread_mutex_unlock(&node->mutex);
}

void swarm_get_stats(
    const SwarmNode* node,
    SwarmStats* stats
) {
    if (!node || !stats) return;

    pthread_mutex_lock((pthread_mutex_t*)&node->mutex);
    *stats = node->stats;
    pthread_mutex_unlock((pthread_mutex_t*)&node->mutex);
}

float swarm_calculate_fov_overlap(
    const CameraGeometry* camera1,
    const CameraGeometry* camera2
) {
    if (!camera1 || !camera2) {
        return 0.0f;
    }

    // Simplified overlap calculation
    // In production, would use actual FOV geometry

    float dist = distance_2d(camera1->x, camera1->y, camera2->x, camera2->y);
    float max_dist = camera1->coverage_radius + camera2->coverage_radius;

    if (dist >= max_dist) {
        return 0.0f;  // No overlap
    }

    // Simple linear interpolation
    float overlap = 1.0f - (dist / max_dist);
    return clamp(overlap, 0.0f, 1.0f);
}

bool swarm_image_to_world(
    const CameraGeometry* camera,
    float image_x,
    float image_y,
    float* world_x,
    float* world_y
) {
    if (!camera || !world_x || !world_y) {
        return false;
    }

    // Simplified projection (assumes overhead view)
    // In production, would use full camera calibration matrix

    float scale = camera->coverage_radius / 1000.0f;  // Rough scale

    *world_x = camera->x + (image_x - 960) * scale;  // Assume 1920x1080 image
    *world_y = camera->y + (image_y - 540) * scale;

    return true;
}
