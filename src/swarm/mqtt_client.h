/**
 * @file mqtt_client.h
 * @brief MQTT 5.0 client for Swarm Intelligence
 *
 * Lightweight MQTT client optimized for embedded systems.
 * Uses Eclipse Paho MQTT C library.
 */

#ifndef OMNISIGHT_MQTT_CLIENT_H
#define OMNISIGHT_MQTT_CLIENT_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// ============================================================================
// Forward Declarations
// ============================================================================

typedef struct MqttClient MqttClient;

// ============================================================================
// Constants
// ============================================================================

#define MQTT_MAX_TOPIC_LENGTH 256
#define MQTT_MAX_PAYLOAD_SIZE 65536  // 64KB max message size

// ============================================================================
// Enums
// ============================================================================

/**
 * MQTT QoS level
 */
typedef enum {
    MQTT_QOS_AT_MOST_ONCE = 0,   // Fire and forget
    MQTT_QOS_AT_LEAST_ONCE = 1,  // Guaranteed delivery, may duplicate
    MQTT_QOS_EXACTLY_ONCE = 2    // Guaranteed exactly once (expensive)
} MqttQoS;

/**
 * Connection status
 */
typedef enum {
    MQTT_STATUS_DISCONNECTED,
    MQTT_STATUS_CONNECTING,
    MQTT_STATUS_CONNECTED,
    MQTT_STATUS_DISCONNECTING,
    MQTT_STATUS_ERROR
} MqttStatus;

// ============================================================================
// Callbacks
// ============================================================================

/**
 * Message received callback
 *
 * @param topic Message topic
 * @param payload Message payload
 * @param payload_len Payload length
 * @param user_data User data passed to mqtt_client_init
 */
typedef void (*MqttMessageCallback)(
    const char* topic,
    const uint8_t* payload,
    uint32_t payload_len,
    void* user_data
);

/**
 * Connection status callback
 *
 * @param status New connection status
 * @param user_data User data passed to mqtt_client_init
 */
typedef void (*MqttStatusCallback)(
    MqttStatus status,
    void* user_data
);

// ============================================================================
// Data Structures
// ============================================================================

/**
 * MQTT client configuration
 */
typedef struct {
    // Broker
    const char* broker_address;   // MQTT broker address
    uint16_t broker_port;         // MQTT broker port (default 1883)

    // Client
    const char* client_id;        // Unique client ID
    const char* username;         // Username (optional)
    const char* password;         // Password (optional)

    // Connection
    uint32_t keepalive_interval_s; // Keepalive interval (default 60s)
    bool clean_session;           // Clean session flag
    uint32_t connect_timeout_s;   // Connection timeout

    // TLS/SSL (optional)
    bool use_tls;                 // Enable TLS
    const char* ca_cert_path;     // CA certificate path
    const char* client_cert_path; // Client certificate path
    const char* client_key_path;  // Client private key path

    // Callbacks
    MqttMessageCallback on_message;
    MqttStatusCallback on_status;
    void* user_data;              // User data for callbacks

    // Quality of Service
    MqttQoS default_qos;          // Default QoS for publish
} MqttConfig;

/**
 * MQTT client statistics
 */
typedef struct {
    uint64_t messages_published;  // Total messages published
    uint64_t messages_received;   // Total messages received
    uint64_t bytes_sent;          // Total bytes sent
    uint64_t bytes_received;      // Total bytes received
    uint64_t connect_attempts;    // Connection attempts
    uint64_t connection_errors;   // Connection errors
    uint64_t publish_failures;    // Publish failures
    float avg_latency_ms;         // Average round-trip latency
} MqttStats;

// ============================================================================
// Public API
// ============================================================================

/**
 * Initialize MQTT client
 *
 * @param config MQTT configuration
 * @return Client handle, or NULL on error
 */
MqttClient* mqtt_client_init(const MqttConfig* config);

/**
 * Destroy MQTT client
 *
 * @param client Client to destroy
 */
void mqtt_client_destroy(MqttClient* client);

/**
 * Connect to MQTT broker
 *
 * @param client MQTT client
 * @return true on success, false on error
 */
bool mqtt_client_connect(MqttClient* client);

/**
 * Disconnect from MQTT broker
 *
 * @param client MQTT client
 */
void mqtt_client_disconnect(MqttClient* client);

/**
 * Check if connected
 *
 * @param client MQTT client
 * @return true if connected, false otherwise
 */
bool mqtt_client_is_connected(const MqttClient* client);

/**
 * Get connection status
 *
 * @param client MQTT client
 * @return Connection status
 */
MqttStatus mqtt_client_get_status(const MqttClient* client);

/**
 * Subscribe to topic
 *
 * @param client MQTT client
 * @param topic Topic to subscribe (supports wildcards: +, #)
 * @param qos Quality of Service level
 * @return true on success, false on error
 */
bool mqtt_client_subscribe(
    MqttClient* client,
    const char* topic,
    MqttQoS qos
);

/**
 * Unsubscribe from topic
 *
 * @param client MQTT client
 * @param topic Topic to unsubscribe
 * @return true on success, false on error
 */
bool mqtt_client_unsubscribe(
    MqttClient* client,
    const char* topic
);

/**
 * Publish message
 *
 * @param client MQTT client
 * @param topic Topic to publish to
 * @param payload Message payload
 * @param payload_len Payload length
 * @param qos Quality of Service level
 * @param retain Retain flag
 * @return true on success, false on error
 */
bool mqtt_client_publish(
    MqttClient* client,
    const char* topic,
    const uint8_t* payload,
    uint32_t payload_len,
    MqttQoS qos,
    bool retain
);

/**
 * Publish string message
 *
 * @param client MQTT client
 * @param topic Topic to publish to
 * @param message String message (null-terminated)
 * @param qos Quality of Service level
 * @param retain Retain flag
 * @return true on success, false on error
 */
bool mqtt_client_publish_string(
    MqttClient* client,
    const char* topic,
    const char* message,
    MqttQoS qos,
    bool retain
);

/**
 * Process incoming messages
 *
 * Should be called periodically to handle incoming messages.
 * Non-blocking.
 *
 * @param client MQTT client
 * @param timeout_ms Timeout in milliseconds (0 = non-blocking)
 * @return Number of messages processed
 */
uint32_t mqtt_client_process(MqttClient* client, uint32_t timeout_ms);

/**
 * Get client statistics
 *
 * @param client MQTT client
 * @param stats Output statistics
 */
void mqtt_client_get_stats(const MqttClient* client, MqttStats* stats);

/**
 * Set last will and testament
 *
 * Message published when client disconnects unexpectedly.
 *
 * @param client MQTT client
 * @param topic Will topic
 * @param message Will message
 * @param qos Will QoS
 * @param retain Retain flag
 * @return true on success, false on error
 */
bool mqtt_client_set_will(
    MqttClient* client,
    const char* topic,
    const char* message,
    MqttQoS qos,
    bool retain
);

#ifdef __cplusplus
}
#endif

#endif // OMNISIGHT_MQTT_CLIENT_H
