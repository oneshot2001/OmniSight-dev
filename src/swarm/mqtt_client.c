/**
 * @file mqtt_client.c
 * @brief OMNISIGHT MQTT 5.0 Client Implementation
 *
 * Production MQTT client using Eclipse Mosquitto library for multi-camera
 * swarm communication. Supports track sharing, event distribution, and
 * federated learning with QoS 0/1/2.
 *
 * Copyright (C) 2025 OMNISIGHT
 * Based on Axis ACAP Native SDK and Eclipse Mosquitto documentation
 */

#include "mqtt_client.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <syslog.h>
#include <errno.h>
#include <time.h>
#include <mosquitto.h>
#include <jansson.h>  // JSON serialization

/**
 * MQTT client internal structure
 */
struct MqttClient {
    // Configuration
    char camera_id[64];
    char broker_host[256];
    int broker_port;
    int keepalive_seconds;
    bool clean_session;
    char username[128];
    char password[128];
    bool use_tls;
    char ca_cert_path[256];

    // Mosquitto handle
    struct mosquitto* mosq;
    bool connected;

    // Callbacks
    TrackReceivedCallback on_track_received;
    EventReceivedCallback on_event_received;
    ModelWeightsReceivedCallback on_model_received;
    ConsensusReceivedCallback on_consensus_received;
    ConnectionStatusCallback on_connection_status;
    void* callback_user_data;

    // Statistics
    uint64_t messages_sent;
    uint64_t messages_received;
    uint64_t bytes_sent;
    uint64_t bytes_received;
    uint32_t reconnects;

    // Thread safety
    pthread_mutex_t publish_mutex;
    pthread_mutex_t stats_mutex;

    // Error tracking
    char last_error[256];
};

// Topic prefix
#define TOPIC_PREFIX "omnisight/swarm"

// ============================================================================
// Forward Declarations (Internal Functions)
// ============================================================================

static void on_connect(struct mosquitto* mosq, void* obj, int reason_code);
static void on_disconnect(struct mosquitto* mosq, void* obj, int reason_code);
static void on_message(struct mosquitto* mosq, void* obj,
                      const struct mosquitto_message* message);
static void on_publish(struct mosquitto* mosq, void* obj, int mid);
static void on_subscribe(struct mosquitto* mosq, void* obj, int mid,
                        int qos_count, const int* granted_qos);

static char* serialize_track_message(const TrackMessage* track);
static char* serialize_event_message(const EventMessage* event);
static char* serialize_model_weights_message(const ModelWeightsMessage* weights);
static char* serialize_consensus_message(const ConsensusMessage* consensus);

static bool deserialize_track_message(const char* json, TrackMessage* track);
static bool deserialize_event_message(const char* json, EventMessage* event);
static bool deserialize_model_weights_message(const char* json,
                                              ModelWeightsMessage* weights);
static bool deserialize_consensus_message(const char* json,
                                         ConsensusMessage* consensus);

// ============================================================================
// Public API Implementation
// ============================================================================

MqttClient* mqtt_client_init(const MqttClientConfig* config) {
    if (!config || !config->camera_id || !config->broker_host) {
        syslog(LOG_ERR, "[MQTT] Invalid configuration");
        return NULL;
    }

    // Initialize mosquitto library (thread-safe)
    static bool mosquitto_initialized = false;
    if (!mosquitto_initialized) {
        mosquitto_lib_init();
        mosquitto_initialized = true;
        syslog(LOG_INFO, "[MQTT] Mosquitto library initialized");
    }

    MqttClient* client = calloc(1, sizeof(MqttClient));
    if (!client) {
        syslog(LOG_ERR, "[MQTT] Failed to allocate memory");
        return NULL;
    }

    // Copy configuration
    strncpy(client->camera_id, config->camera_id, sizeof(client->camera_id) - 1);
    strncpy(client->broker_host, config->broker_host, sizeof(client->broker_host) - 1);
    client->broker_port = config->broker_port;
    client->keepalive_seconds = config->keepalive_seconds;
    client->clean_session = config->clean_session;

    if (config->username) {
        strncpy(client->username, config->username, sizeof(client->username) - 1);
    }
    if (config->password) {
        strncpy(client->password, config->password, sizeof(client->password) - 1);
    }

    client->use_tls = config->use_tls;
    if (config->ca_cert_path) {
        strncpy(client->ca_cert_path, config->ca_cert_path,
               sizeof(client->ca_cert_path) - 1);
    }

    // Initialize mutexes
    pthread_mutex_init(&client->publish_mutex, NULL);
    pthread_mutex_init(&client->stats_mutex, NULL);

    // Create mosquitto instance
    client->mosq = mosquitto_new(config->camera_id, config->clean_session, client);
    if (!client->mosq) {
        syslog(LOG_ERR, "[MQTT] Failed to create mosquitto instance");
        pthread_mutex_destroy(&client->publish_mutex);
        pthread_mutex_destroy(&client->stats_mutex);
        free(client);
        return NULL;
    }

    // Set callbacks
    mosquitto_connect_callback_set(client->mosq, on_connect);
    mosquitto_disconnect_callback_set(client->mosq, on_disconnect);
    mosquitto_message_callback_set(client->mosq, on_message);
    mosquitto_publish_callback_set(client->mosq, on_publish);
    mosquitto_subscribe_callback_set(client->mosq, on_subscribe);

    // Set authentication if provided
    if (client->username[0] != '\0') {
        mosquitto_username_pw_set(client->mosq, client->username,
                                 client->password[0] != '\0' ? client->password : NULL);
    }

    // Set TLS if enabled
    if (client->use_tls && client->ca_cert_path[0] != '\0') {
        int rc = mosquitto_tls_set(client->mosq, client->ca_cert_path,
                                   NULL, NULL, NULL, NULL);
        if (rc != MOSQ_ERR_SUCCESS) {
            syslog(LOG_ERR, "[MQTT] Failed to configure TLS: %s",
                   mosquitto_strerror(rc));
            snprintf(client->last_error, sizeof(client->last_error),
                    "TLS configuration failed: %s", mosquitto_strerror(rc));
        }
    }

    syslog(LOG_INFO, "[MQTT] Client initialized (camera=%s, broker=%s:%d)",
           client->camera_id, client->broker_host, client->broker_port);

    return client;
}

bool mqtt_client_connect(MqttClient* client) {
    if (!client || !client->mosq) {
        return false;
    }

    // Connect to broker (async)
    int rc = mosquitto_connect_async(client->mosq, client->broker_host,
                                     client->broker_port,
                                     client->keepalive_seconds);
    if (rc != MOSQ_ERR_SUCCESS) {
        syslog(LOG_ERR, "[MQTT] Connection failed: %s", mosquitto_strerror(rc));
        snprintf(client->last_error, sizeof(client->last_error),
                "Connection failed: %s", mosquitto_strerror(rc));
        return false;
    }

    // Start network loop thread
    rc = mosquitto_loop_start(client->mosq);
    if (rc != MOSQ_ERR_SUCCESS) {
        syslog(LOG_ERR, "[MQTT] Failed to start network loop: %s",
               mosquitto_strerror(rc));
        snprintf(client->last_error, sizeof(client->last_error),
                "Failed to start network loop: %s", mosquitto_strerror(rc));
        return false;
    }

    syslog(LOG_INFO, "[MQTT] Connecting to %s:%d...",
           client->broker_host, client->broker_port);

    return true;
}

void mqtt_client_disconnect(MqttClient* client) {
    if (!client || !client->mosq) {
        return;
    }

    if (client->connected) {
        mosquitto_disconnect(client->mosq);
        client->connected = false;
    }

    // Stop network loop
    mosquitto_loop_stop(client->mosq, false);

    syslog(LOG_INFO, "[MQTT] Disconnected");
}

bool mqtt_client_is_connected(const MqttClient* client) {
    return client ? client->connected : false;
}

void mqtt_client_set_track_callback(MqttClient* client,
                                    TrackReceivedCallback callback,
                                    void* user_data) {
    if (client) {
        client->on_track_received = callback;
        client->callback_user_data = user_data;
    }
}

void mqtt_client_set_event_callback(MqttClient* client,
                                    EventReceivedCallback callback,
                                    void* user_data) {
    if (client) {
        client->on_event_received = callback;
        client->callback_user_data = user_data;
    }
}

void mqtt_client_set_model_callback(MqttClient* client,
                                    ModelWeightsReceivedCallback callback,
                                    void* user_data) {
    if (client) {
        client->on_model_received = callback;
        client->callback_user_data = user_data;
    }
}

void mqtt_client_set_consensus_callback(MqttClient* client,
                                        ConsensusReceivedCallback callback,
                                        void* user_data) {
    if (client) {
        client->on_consensus_received = callback;
        client->callback_user_data = user_data;
    }
}

void mqtt_client_set_connection_callback(MqttClient* client,
                                         ConnectionStatusCallback callback,
                                         void* user_data) {
    if (client) {
        client->on_connection_status = callback;
        client->callback_user_data = user_data;
    }
}

bool mqtt_client_publish_track(MqttClient* client, const TrackMessage* track) {
    if (!client || !client->mosq || !track || !client->connected) {
        return false;
    }

    // Serialize track to JSON
    char* json = serialize_track_message(track);
    if (!json) {
        syslog(LOG_ERR, "[MQTT] Failed to serialize track message");
        return false;
    }

    // Build topic: omnisight/swarm/tracks/{camera_id}/{track_id}
    char topic[256];
    snprintf(topic, sizeof(topic), "%s/tracks/%s/%u",
             TOPIC_PREFIX, track->camera_id, track->track_id);

    pthread_mutex_lock(&client->publish_mutex);

    // Publish with QoS 0 (real-time, fire and forget)
    int rc = mosquitto_publish(client->mosq, NULL, topic, strlen(json), json, 0, false);

    if (rc == MOSQ_ERR_SUCCESS) {
        pthread_mutex_lock(&client->stats_mutex);
        client->messages_sent++;
        client->bytes_sent += strlen(json);
        pthread_mutex_unlock(&client->stats_mutex);
    } else {
        syslog(LOG_ERR, "[MQTT] Publish track failed: %s", mosquitto_strerror(rc));
        snprintf(client->last_error, sizeof(client->last_error),
                "Publish failed: %s", mosquitto_strerror(rc));
    }

    pthread_mutex_unlock(&client->publish_mutex);

    free(json);
    return (rc == MOSQ_ERR_SUCCESS);
}

bool mqtt_client_publish_event(MqttClient* client, const EventMessage* event) {
    if (!client || !client->mosq || !event || !client->connected) {
        return false;
    }

    // Serialize event to JSON
    char* json = serialize_event_message(event);
    if (!json) {
        syslog(LOG_ERR, "[MQTT] Failed to serialize event message");
        return false;
    }

    // Build topic: omnisight/swarm/events/{camera_id}/{event_id}
    char topic[256];
    snprintf(topic, sizeof(topic), "%s/events/%s/%u",
             TOPIC_PREFIX, event->camera_id, event->event_id);

    pthread_mutex_lock(&client->publish_mutex);

    // Publish with QoS 1 (at least once delivery)
    int rc = mosquitto_publish(client->mosq, NULL, topic, strlen(json), json, 1, false);

    if (rc == MOSQ_ERR_SUCCESS) {
        pthread_mutex_lock(&client->stats_mutex);
        client->messages_sent++;
        client->bytes_sent += strlen(json);
        pthread_mutex_unlock(&client->stats_mutex);
    } else {
        syslog(LOG_ERR, "[MQTT] Publish event failed: %s", mosquitto_strerror(rc));
        snprintf(client->last_error, sizeof(client->last_error),
                "Publish failed: %s", mosquitto_strerror(rc));
    }

    pthread_mutex_unlock(&client->publish_mutex);

    free(json);
    return (rc == MOSQ_ERR_SUCCESS);
}

bool mqtt_client_publish_model_weights(MqttClient* client,
                                       const ModelWeightsMessage* weights) {
    if (!client || !client->mosq || !weights || !client->connected) {
        return false;
    }

    // Serialize weights to JSON
    char* json = serialize_model_weights_message(weights);
    if (!json) {
        syslog(LOG_ERR, "[MQTT] Failed to serialize model weights message");
        return false;
    }

    // Build topic: omnisight/swarm/models/{camera_id}/weights
    char topic[256];
    snprintf(topic, sizeof(topic), "%s/models/%s/weights",
             TOPIC_PREFIX, weights->camera_id);

    pthread_mutex_lock(&client->publish_mutex);

    // Publish with QoS 2 (exactly once delivery)
    int rc = mosquitto_publish(client->mosq, NULL, topic, strlen(json), json, 2, false);

    if (rc == MOSQ_ERR_SUCCESS) {
        pthread_mutex_lock(&client->stats_mutex);
        client->messages_sent++;
        client->bytes_sent += strlen(json);
        pthread_mutex_unlock(&client->stats_mutex);
    } else {
        syslog(LOG_ERR, "[MQTT] Publish model weights failed: %s",
               mosquitto_strerror(rc));
        snprintf(client->last_error, sizeof(client->last_error),
                "Publish failed: %s", mosquitto_strerror(rc));
    }

    pthread_mutex_unlock(&client->publish_mutex);

    free(json);
    return (rc == MOSQ_ERR_SUCCESS);
}

bool mqtt_client_publish_consensus(MqttClient* client,
                                   const ConsensusMessage* consensus) {
    if (!client || !client->mosq || !consensus || !client->connected) {
        return false;
    }

    // Serialize consensus to JSON
    char* json = serialize_consensus_message(consensus);
    if (!json) {
        syslog(LOG_ERR, "[MQTT] Failed to serialize consensus message");
        return false;
    }

    // Build topic: omnisight/swarm/consensus/{event_id}
    char topic[256];
    snprintf(topic, sizeof(topic), "%s/consensus/%u",
             TOPIC_PREFIX, consensus->event_id);

    pthread_mutex_lock(&client->publish_mutex);

    // Publish with QoS 1 (at least once delivery)
    int rc = mosquitto_publish(client->mosq, NULL, topic, strlen(json), json, 1, false);

    if (rc == MOSQ_ERR_SUCCESS) {
        pthread_mutex_lock(&client->stats_mutex);
        client->messages_sent++;
        client->bytes_sent += strlen(json);
        pthread_mutex_unlock(&client->stats_mutex);
    } else {
        syslog(LOG_ERR, "[MQTT] Publish consensus failed: %s",
               mosquitto_strerror(rc));
        snprintf(client->last_error, sizeof(client->last_error),
                "Publish failed: %s", mosquitto_strerror(rc));
    }

    pthread_mutex_unlock(&client->publish_mutex);

    free(json);
    return (rc == MOSQ_ERR_SUCCESS);
}

void mqtt_client_get_stats(const MqttClient* client,
                           uint64_t* messages_sent,
                           uint64_t* messages_received,
                           uint64_t* bytes_sent,
                           uint64_t* bytes_received,
                           uint32_t* reconnects) {
    if (!client) {
        return;
    }

    pthread_mutex_lock((pthread_mutex_t*)&client->stats_mutex);

    if (messages_sent) *messages_sent = client->messages_sent;
    if (messages_received) *messages_received = client->messages_received;
    if (bytes_sent) *bytes_sent = client->bytes_sent;
    if (bytes_received) *bytes_received = client->bytes_received;
    if (reconnects) *reconnects = client->reconnects;

    pthread_mutex_unlock((pthread_mutex_t*)&client->stats_mutex);
}

const char* mqtt_client_get_last_error(const MqttClient* client) {
    return client ? client->last_error : NULL;
}

void mqtt_client_destroy(MqttClient* client) {
    if (!client) {
        return;
    }

    // Disconnect if still connected
    if (client->connected) {
        mqtt_client_disconnect(client);
    }

    // Destroy mosquitto instance
    if (client->mosq) {
        mosquitto_destroy(client->mosq);
    }

    // Destroy mutexes
    pthread_mutex_destroy(&client->publish_mutex);
    pthread_mutex_destroy(&client->stats_mutex);

    free(client);

    syslog(LOG_INFO, "[MQTT] Client destroyed");
}

// ============================================================================
// Mosquitto Callbacks
// ============================================================================

static void on_connect(struct mosquitto* mosq, void* obj, int reason_code) {
    MqttClient* client = (MqttClient*)obj;

    if (reason_code == 0) {
        // Connection successful
        client->connected = true;
        syslog(LOG_INFO, "[MQTT] Connected to broker");

        // Subscribe to all swarm topics
        char topic_tracks[256];
        char topic_events[256];
        char topic_models[256];
        char topic_consensus[256];

        snprintf(topic_tracks, sizeof(topic_tracks), "%s/tracks/+/+", TOPIC_PREFIX);
        snprintf(topic_events, sizeof(topic_events), "%s/events/+/+", TOPIC_PREFIX);
        snprintf(topic_models, sizeof(topic_models), "%s/models/+/weights", TOPIC_PREFIX);
        snprintf(topic_consensus, sizeof(topic_consensus), "%s/consensus/+", TOPIC_PREFIX);

        mosquitto_subscribe(mosq, NULL, topic_tracks, 0);
        mosquitto_subscribe(mosq, NULL, topic_events, 1);
        mosquitto_subscribe(mosq, NULL, topic_models, 2);
        mosquitto_subscribe(mosq, NULL, topic_consensus, 1);

        // Notify callback
        if (client->on_connection_status) {
            client->on_connection_status(true, client->callback_user_data);
        }
    } else {
        // Connection failed
        syslog(LOG_ERR, "[MQTT] Connection failed: %s",
               mosquitto_connack_string(reason_code));
        snprintf(client->last_error, sizeof(client->last_error),
                "Connection failed: %s", mosquitto_connack_string(reason_code));

        // Track reconnect attempts
        pthread_mutex_lock(&client->stats_mutex);
        client->reconnects++;
        pthread_mutex_unlock(&client->stats_mutex);
    }
}

static void on_disconnect(struct mosquitto* mosq, void* obj, int reason_code) {
    MqttClient* client = (MqttClient*)obj;

    client->connected = false;

    if (reason_code == 0) {
        syslog(LOG_INFO, "[MQTT] Disconnected (clean)");
    } else {
        syslog(LOG_WARNING, "[MQTT] Disconnected unexpectedly: %s",
               mosquitto_strerror(reason_code));

        // Track reconnect attempts
        pthread_mutex_lock(&client->stats_mutex);
        client->reconnects++;
        pthread_mutex_unlock(&client->stats_mutex);
    }

    // Notify callback
    if (client->on_connection_status) {
        client->on_connection_status(false, client->callback_user_data);
    }
}

static void on_message(struct mosquitto* mosq, void* obj,
                      const struct mosquitto_message* message) {
    MqttClient* client = (MqttClient*)obj;

    pthread_mutex_lock(&client->stats_mutex);
    client->messages_received++;
    client->bytes_received += message->payloadlen;
    pthread_mutex_unlock(&client->stats_mutex);

    // Parse topic to determine message type
    const char* topic = message->topic;
    const char* payload = (const char*)message->payload;

    if (strstr(topic, "/tracks/") != NULL) {
        // Track message
        TrackMessage track;
        if (deserialize_track_message(payload, &track)) {
            if (client->on_track_received) {
                client->on_track_received(&track, client->callback_user_data);
            }
        }
    } else if (strstr(topic, "/events/") != NULL) {
        // Event message
        EventMessage event;
        if (deserialize_event_message(payload, &event)) {
            if (client->on_event_received) {
                client->on_event_received(&event, client->callback_user_data);
            }
        }
    } else if (strstr(topic, "/models/") != NULL && strstr(topic, "/weights") != NULL) {
        // Model weights message
        ModelWeightsMessage weights;
        if (deserialize_model_weights_message(payload, &weights)) {
            if (client->on_model_received) {
                client->on_model_received(&weights, client->callback_user_data);
            }
            // Free weights array allocated during deserialization
            if (weights.weights) {
                free(weights.weights);
            }
        }
    } else if (strstr(topic, "/consensus/") != NULL) {
        // Consensus message
        ConsensusMessage consensus;
        if (deserialize_consensus_message(payload, &consensus)) {
            if (client->on_consensus_received) {
                client->on_consensus_received(&consensus, client->callback_user_data);
            }
        }
    }
}

static void on_publish(struct mosquitto* mosq, void* obj, int mid) {
    // Message delivery confirmed
    // (Can track delivery latency here if needed)
}

static void on_subscribe(struct mosquitto* mosq, void* obj, int mid,
                        int qos_count, const int* granted_qos) {
    syslog(LOG_DEBUG, "[MQTT] Subscription confirmed (mid=%d)", mid);
}

// ============================================================================
// JSON Serialization/Deserialization
// ============================================================================

static char* serialize_track_message(const TrackMessage* track) {
    json_t* root = json_object();

    json_object_set_new(root, "camera_id", json_string(track->camera_id));
    json_object_set_new(root, "track_id", json_integer(track->track_id));

    json_t* position = json_object();
    json_object_set_new(position, "x", json_real(track->position.x));
    json_object_set_new(position, "y", json_real(track->position.y));
    json_object_set_new(position, "z", json_real(track->position.z));
    json_object_set_new(root, "global_position", position);

    json_t* velocity = json_object();
    json_object_set_new(velocity, "x", json_real(track->velocity_x));
    json_object_set_new(velocity, "y", json_real(track->velocity_y));
    json_object_set_new(root, "velocity", velocity);

    json_object_set_new(root, "class", json_string(track->object_class));
    json_object_set_new(root, "confidence", json_real(track->confidence));
    json_object_set_new(root, "timestamp", json_integer(track->timestamp_ms));
    json_object_set_new(root, "behaviors", json_integer(track->behaviors));
    json_object_set_new(root, "threat_score", json_real(track->threat_score));

    char* json_str = json_dumps(root, JSON_COMPACT);
    json_decref(root);

    return json_str;
}

static char* serialize_event_message(const EventMessage* event) {
    json_t* root = json_object();

    json_object_set_new(root, "camera_id", json_string(event->camera_id));
    json_object_set_new(root, "event_id", json_integer(event->event_id));
    json_object_set_new(root, "event_type", json_integer(event->event_type));

    json_t* position = json_object();
    json_object_set_new(position, "x", json_real(event->position.x));
    json_object_set_new(position, "y", json_real(event->position.y));
    json_object_set_new(position, "z", json_real(event->position.z));
    json_object_set_new(root, "position", position);

    json_object_set_new(root, "track_id", json_integer(event->track_id));
    json_object_set_new(root, "probability", json_real(event->probability));
    json_object_set_new(root, "severity", json_integer(event->severity));
    json_object_set_new(root, "predicted_time", json_integer(event->predicted_time_ms));
    json_object_set_new(root, "timestamp", json_integer(event->timestamp_ms));

    char* json_str = json_dumps(root, JSON_COMPACT);
    json_decref(root);

    return json_str;
}

static char* serialize_model_weights_message(const ModelWeightsMessage* weights) {
    json_t* root = json_object();

    json_object_set_new(root, "camera_id", json_string(weights->camera_id));
    json_object_set_new(root, "version", json_integer(weights->version));
    json_object_set_new(root, "num_weights", json_integer(weights->num_weights));
    json_object_set_new(root, "timestamp", json_integer(weights->timestamp_ms));

    // Serialize weights array
    json_t* weights_array = json_array();
    for (size_t i = 0; i < weights->num_weights; i++) {
        json_array_append_new(weights_array, json_real(weights->weights[i]));
    }
    json_object_set_new(root, "weights", weights_array);

    char* json_str = json_dumps(root, JSON_COMPACT);
    json_decref(root);

    return json_str;
}

static char* serialize_consensus_message(const ConsensusMessage* consensus) {
    json_t* root = json_object();

    json_object_set_new(root, "event_id", json_integer(consensus->event_id));
    json_object_set_new(root, "initiating_camera", json_string(consensus->initiating_camera));
    json_object_set_new(root, "num_confirming", json_integer(consensus->num_confirming));

    json_t* cameras = json_array();
    for (uint32_t i = 0; i < consensus->num_confirming && i < 10; i++) {
        json_array_append_new(cameras, json_string(consensus->confirming_cameras[i]));
    }
    json_object_set_new(root, "confirming_cameras", cameras);

    json_object_set_new(root, "aggregated_confidence", json_real(consensus->aggregated_confidence));
    json_object_set_new(root, "timestamp", json_integer(consensus->timestamp_ms));

    char* json_str = json_dumps(root, JSON_COMPACT);
    json_decref(root);

    return json_str;
}

static bool deserialize_track_message(const char* json, TrackMessage* track) {
    json_error_t error;
    json_t* root = json_loads(json, 0, &error);
    if (!root) {
        syslog(LOG_ERR, "[MQTT] JSON parse error: %s", error.text);
        return false;
    }

    memset(track, 0, sizeof(TrackMessage));

    json_t* value;
    if ((value = json_object_get(root, "camera_id"))) {
        strncpy(track->camera_id, json_string_value(value), sizeof(track->camera_id) - 1);
    }
    if ((value = json_object_get(root, "track_id"))) {
        track->track_id = json_integer_value(value);
    }

    json_t* position = json_object_get(root, "global_position");
    if (position) {
        track->position.x = json_real_value(json_object_get(position, "x"));
        track->position.y = json_real_value(json_object_get(position, "y"));
        track->position.z = json_real_value(json_object_get(position, "z"));
    }

    json_t* velocity = json_object_get(root, "velocity");
    if (velocity) {
        track->velocity_x = json_real_value(json_object_get(velocity, "x"));
        track->velocity_y = json_real_value(json_object_get(velocity, "y"));
    }

    if ((value = json_object_get(root, "class"))) {
        strncpy(track->object_class, json_string_value(value), sizeof(track->object_class) - 1);
    }
    if ((value = json_object_get(root, "confidence"))) {
        track->confidence = json_real_value(value);
    }
    if ((value = json_object_get(root, "timestamp"))) {
        track->timestamp_ms = json_integer_value(value);
    }
    if ((value = json_object_get(root, "behaviors"))) {
        track->behaviors = json_integer_value(value);
    }
    if ((value = json_object_get(root, "threat_score"))) {
        track->threat_score = json_real_value(value);
    }

    json_decref(root);
    return true;
}

static bool deserialize_event_message(const char* json, EventMessage* event) {
    json_error_t error;
    json_t* root = json_loads(json, 0, &error);
    if (!root) {
        syslog(LOG_ERR, "[MQTT] JSON parse error: %s", error.text);
        return false;
    }

    memset(event, 0, sizeof(EventMessage));

    json_t* value;
    if ((value = json_object_get(root, "camera_id"))) {
        strncpy(event->camera_id, json_string_value(value), sizeof(event->camera_id) - 1);
    }
    if ((value = json_object_get(root, "event_id"))) {
        event->event_id = json_integer_value(value);
    }
    if ((value = json_object_get(root, "event_type"))) {
        event->event_type = json_integer_value(value);
    }

    json_t* position = json_object_get(root, "position");
    if (position) {
        event->position.x = json_real_value(json_object_get(position, "x"));
        event->position.y = json_real_value(json_object_get(position, "y"));
        event->position.z = json_real_value(json_object_get(position, "z"));
    }

    if ((value = json_object_get(root, "track_id"))) {
        event->track_id = json_integer_value(value);
    }
    if ((value = json_object_get(root, "probability"))) {
        event->probability = json_real_value(value);
    }
    if ((value = json_object_get(root, "severity"))) {
        event->severity = json_integer_value(value);
    }
    if ((value = json_object_get(root, "predicted_time"))) {
        event->predicted_time_ms = json_integer_value(value);
    }
    if ((value = json_object_get(root, "timestamp"))) {
        event->timestamp_ms = json_integer_value(value);
    }

    json_decref(root);
    return true;
}

static bool deserialize_model_weights_message(const char* json,
                                              ModelWeightsMessage* weights) {
    json_error_t error;
    json_t* root = json_loads(json, 0, &error);
    if (!root) {
        syslog(LOG_ERR, "[MQTT] JSON parse error: %s", error.text);
        return false;
    }

    memset(weights, 0, sizeof(ModelWeightsMessage));

    json_t* value;
    if ((value = json_object_get(root, "camera_id"))) {
        strncpy(weights->camera_id, json_string_value(value), sizeof(weights->camera_id) - 1);
    }
    if ((value = json_object_get(root, "version"))) {
        weights->version = json_integer_value(value);
    }
    if ((value = json_object_get(root, "num_weights"))) {
        weights->num_weights = json_integer_value(value);
    }
    if ((value = json_object_get(root, "timestamp"))) {
        weights->timestamp_ms = json_integer_value(value);
    }

    json_t* weights_array = json_object_get(root, "weights");
    if (weights_array && json_is_array(weights_array)) {
        size_t array_size = json_array_size(weights_array);
        weights->weights = malloc(array_size * sizeof(float));
        if (weights->weights) {
            for (size_t i = 0; i < array_size; i++) {
                weights->weights[i] = json_real_value(json_array_get(weights_array, i));
            }
        }
    }

    json_decref(root);
    return true;
}

static bool deserialize_consensus_message(const char* json,
                                         ConsensusMessage* consensus) {
    json_error_t error;
    json_t* root = json_loads(json, 0, &error);
    if (!root) {
        syslog(LOG_ERR, "[MQTT] JSON parse error: %s", error.text);
        return false;
    }

    memset(consensus, 0, sizeof(ConsensusMessage));

    json_t* value;
    if ((value = json_object_get(root, "event_id"))) {
        consensus->event_id = json_integer_value(value);
    }
    if ((value = json_object_get(root, "initiating_camera"))) {
        strncpy(consensus->initiating_camera, json_string_value(value),
                sizeof(consensus->initiating_camera) - 1);
    }
    if ((value = json_object_get(root, "num_confirming"))) {
        consensus->num_confirming = json_integer_value(value);
    }

    json_t* cameras = json_object_get(root, "confirming_cameras");
    if (cameras && json_is_array(cameras)) {
        size_t array_size = json_array_size(cameras);
        for (size_t i = 0; i < array_size && i < 10; i++) {
            const char* camera_id = json_string_value(json_array_get(cameras, i));
            if (camera_id) {
                strncpy(consensus->confirming_cameras[i], camera_id, 63);
            }
        }
    }

    if ((value = json_object_get(root, "aggregated_confidence"))) {
        consensus->aggregated_confidence = json_real_value(value);
    }
    if ((value = json_object_get(root, "timestamp"))) {
        consensus->timestamp_ms = json_integer_value(value);
    }

    json_decref(root);
    return true;
}
