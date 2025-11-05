/**
 * @file mqtt_client.h
 * @brief OMNISIGHT MQTT 5.0 Client for Swarm Communication
 *
 * Provides multi-camera communication for track sharing, event distribution,
 * and federated learning via MQTT 5.0 protocol with Eclipse Mosquitto.
 *
 * Copyright (C) 2025 OMNISIGHT
 * Based on Axis ACAP Native SDK message-broker example
 */

#ifndef OMNISIGHT_MQTT_CLIENT_H
#define OMNISIGHT_MQTT_CLIENT_H

#include <stdint.h>
#include <stdbool.h>

#include "../perception/perception.h"
#include "../perception/tracking.h"
#include "../timeline/timeline.h"

#ifdef __cplusplus
extern "C" {
#endif

// Forward declaration
typedef struct MqttClient MqttClient;

/**
 * MQTT client configuration
 */
typedef struct {
    const char* camera_id;          // Unique camera identifier (from Axis serial)
    const char* broker_host;        // MQTT broker hostname/IP
    int broker_port;                // MQTT broker port (default: 1883)
    int keepalive_seconds;          // Keepalive interval (default: 60)
    bool clean_session;             // Clean session flag (default: false)
    const char* username;           // Optional authentication username
    const char* password;           // Optional authentication password
    bool use_tls;                   // Enable TLS/SSL (default: false)
    const char* ca_cert_path;       // Path to CA certificate for TLS
} MqttClientConfig;

/**
 * Global position in world coordinates (meters)
 */
typedef struct {
    float x;                        // X coordinate in world space (meters)
    float y;                        // Y coordinate in world space (meters)
    float z;                        // Z coordinate (elevation, optional)
} GlobalPosition;

/**
 * Track message (published to swarm)
 */
typedef struct {
    char camera_id[64];             // Source camera ID
    uint32_t track_id;              // Track ID on source camera
    GlobalPosition position;        // Global world position
    float velocity_x;               // Velocity X component (m/s)
    float velocity_y;               // Velocity Y component (m/s)
    char object_class[32];          // Object class (person, vehicle, etc.)
    float confidence;               // Detection confidence (0.0-1.0)
    uint64_t timestamp_ms;          // Timestamp (milliseconds since epoch)
    BehaviorFlags behaviors;        // Detected behaviors
    float threat_score;             // Threat score (0.0-1.0)
} TrackMessage;

/**
 * Event message (published to swarm)
 */
typedef struct {
    char camera_id[64];             // Source camera ID
    uint32_t event_id;              // Event ID on source camera
    EventType event_type;           // Type of event
    GlobalPosition position;        // Event location
    uint32_t track_id;              // Associated track ID
    float probability;              // Event probability (0.0-1.0)
    EventSeverity severity;         // Event severity level
    uint64_t predicted_time_ms;     // When event is predicted to occur
    uint64_t timestamp_ms;          // Prediction timestamp
} EventMessage;

/**
 * Model weights message (for federated learning)
 */
typedef struct {
    char camera_id[64];             // Source camera ID
    uint32_t version;               // Model version number
    float* weights;                 // Weight array
    size_t num_weights;             // Number of weights
    uint64_t timestamp_ms;          // Update timestamp
} ModelWeightsMessage;

/**
 * Consensus message (multi-camera event validation)
 */
typedef struct {
    uint32_t event_id;              // Global event ID
    char initiating_camera[64];     // Camera that detected the event
    uint32_t num_confirming;        // Number of cameras confirming
    char confirming_cameras[10][64]; // List of confirming cameras
    float aggregated_confidence;    // Combined confidence
    uint64_t timestamp_ms;          // Consensus timestamp
} ConsensusMessage;

/**
 * Callback for receiving track updates from other cameras
 *
 * @param track Received track message
 * @param user_data User-provided context pointer
 */
typedef void (*TrackReceivedCallback)(const TrackMessage* track, void* user_data);

/**
 * Callback for receiving event predictions from other cameras
 *
 * @param event Received event message
 * @param user_data User-provided context pointer
 */
typedef void (*EventReceivedCallback)(const EventMessage* event, void* user_data);

/**
 * Callback for receiving model weight updates (federated learning)
 *
 * @param weights Received model weights
 * @param user_data User-provided context pointer
 */
typedef void (*ModelWeightsReceivedCallback)(const ModelWeightsMessage* weights,
                                             void* user_data);

/**
 * Callback for receiving consensus messages
 *
 * @param consensus Received consensus message
 * @param user_data User-provided context pointer
 */
typedef void (*ConsensusReceivedCallback)(const ConsensusMessage* consensus,
                                          void* user_data);

/**
 * Callback for connection status changes
 *
 * @param connected true if connected, false if disconnected
 * @param user_data User-provided context pointer
 */
typedef void (*ConnectionStatusCallback)(bool connected, void* user_data);

/**
 * Initialize MQTT client
 *
 * Creates MQTT client instance with specified configuration.
 * Does not connect to broker (call mqtt_client_connect).
 *
 * @param config Client configuration
 * @return MqttClient instance or NULL on failure
 */
MqttClient* mqtt_client_init(const MqttClientConfig* config);

/**
 * Connect to MQTT broker
 *
 * Establishes connection to broker and subscribes to swarm topics.
 * Async operation with automatic reconnection handling.
 * Starts network loop thread for message processing.
 *
 * @param client MqttClient instance
 * @return true on successful connection initiation, false on failure
 */
bool mqtt_client_connect(MqttClient* client);

/**
 * Disconnect from MQTT broker
 *
 * Gracefully disconnects from broker and stops network loop thread.
 *
 * @param client MqttClient instance
 */
void mqtt_client_disconnect(MqttClient* client);

/**
 * Check if client is connected
 *
 * @param client MqttClient instance
 * @return true if connected to broker, false otherwise
 */
bool mqtt_client_is_connected(const MqttClient* client);

/**
 * Set track received callback
 *
 * @param client MqttClient instance
 * @param callback Callback function
 * @param user_data User-provided context pointer
 */
void mqtt_client_set_track_callback(MqttClient* client,
                                    TrackReceivedCallback callback,
                                    void* user_data);

/**
 * Set event received callback
 *
 * @param client MqttClient instance
 * @param callback Callback function
 * @param user_data User-provided context pointer
 */
void mqtt_client_set_event_callback(MqttClient* client,
                                    EventReceivedCallback callback,
                                    void* user_data);

/**
 * Set model weights received callback
 *
 * @param client MqttClient instance
 * @param callback Callback function
 * @param user_data User-provided context pointer
 */
void mqtt_client_set_model_callback(MqttClient* client,
                                    ModelWeightsReceivedCallback callback,
                                    void* user_data);

/**
 * Set consensus received callback
 *
 * @param client MqttClient instance
 * @param callback Callback function
 * @param user_data User-provided context pointer
 */
void mqtt_client_set_consensus_callback(MqttClient* client,
                                        ConsensusReceivedCallback callback,
                                        void* user_data);

/**
 * Set connection status callback
 *
 * @param client MqttClient instance
 * @param callback Callback function
 * @param user_data User-provided context pointer
 */
void mqtt_client_set_connection_callback(MqttClient* client,
                                         ConnectionStatusCallback callback,
                                         void* user_data);

/**
 * Publish track update to swarm
 *
 * Publishes track state to omnisight/swarm/tracks/{camera_id}/{track_id}.
 * Uses QoS 0 for real-time performance.
 *
 * @param client MqttClient instance
 * @param track Track message to publish
 * @return true on successful publish, false on failure
 */
bool mqtt_client_publish_track(MqttClient* client, const TrackMessage* track);

/**
 * Publish event prediction to swarm
 *
 * Publishes event to omnisight/swarm/events/{camera_id}/{event_id}.
 * Uses QoS 1 for reliable delivery.
 *
 * @param client MqttClient instance
 * @param event Event message to publish
 * @return true on successful publish, false on failure
 */
bool mqtt_client_publish_event(MqttClient* client, const EventMessage* event);

/**
 * Publish model weights update (federated learning)
 *
 * Publishes model weights to omnisight/swarm/models/{camera_id}/weights.
 * Uses QoS 2 for exactly-once delivery.
 *
 * @param client MqttClient instance
 * @param weights Model weights to publish
 * @return true on successful publish, false on failure
 */
bool mqtt_client_publish_model_weights(MqttClient* client,
                                       const ModelWeightsMessage* weights);

/**
 * Publish consensus vote
 *
 * Publishes consensus message to omnisight/swarm/consensus/{event_id}.
 * Uses QoS 1 for reliable delivery.
 *
 * @param client MqttClient instance
 * @param consensus Consensus message to publish
 * @return true on successful publish, false on failure
 */
bool mqtt_client_publish_consensus(MqttClient* client,
                                   const ConsensusMessage* consensus);

/**
 * Get MQTT client statistics
 *
 * @param client MqttClient instance
 * @param messages_sent Total messages sent
 * @param messages_received Total messages received
 * @param bytes_sent Total bytes sent
 * @param bytes_received Total bytes received
 * @param reconnects Number of reconnection attempts
 */
void mqtt_client_get_stats(const MqttClient* client,
                           uint64_t* messages_sent,
                           uint64_t* messages_received,
                           uint64_t* bytes_sent,
                           uint64_t* bytes_received,
                           uint32_t* reconnects);

/**
 * Get last error message
 *
 * @param client MqttClient instance
 * @return Last error message or NULL if no error
 */
const char* mqtt_client_get_last_error(const MqttClient* client);

/**
 * Destroy MQTT client and free resources
 *
 * Automatically disconnects if still connected.
 *
 * @param client MqttClient instance
 */
void mqtt_client_destroy(MqttClient* client);

#ifdef __cplusplus
}
#endif

#endif // OMNISIGHT_MQTT_CLIENT_H
