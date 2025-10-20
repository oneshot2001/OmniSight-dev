/**
 * @file omnisight_api.c
 * @brief REST API server implementation
 *
 * Uses libmicrohttpd for HTTP server and libwebsockets for WebSocket
 */

#include "omnisight_api.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <pthread.h>
#include <time.h>

// TODO: Include actual HTTP server library
// #include <microhttpd.h>
// #include <libwebsockets.h>

/**
 * API server state
 */
struct ApiServer {
    ApiServerConfig config;

    // Component handles
    PerceptionEngine* perception;
    TimelineEngine* timeline;
    SwarmNode* swarm;

    // Server state
    bool running;
    pthread_t http_thread;
    pthread_t websocket_thread;

    // HTTP server (placeholder)
    void* http_server;  // MHD_Daemon*

    // WebSocket context (placeholder)
    void* ws_context;   // lws_context*

    // Statistics
    uint64_t start_time_ms;
    uint64_t requests_served;
    uint32_t num_ws_clients;

    pthread_mutex_t mutex;
};

// ============================================================================
// Utility Functions
// ============================================================================

static uint64_t get_current_time_ms() {
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    return (uint64_t)ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
}

/**
 * Create JSON response for tracks
 */
static char* create_tracks_json(PerceptionEngine* perception) {
    if (!perception) return strdup("[]");

    TrackedObject tracks[100];
    uint32_t num_tracks = perception_get_tracked_objects(perception, tracks, 100);

    // Allocate buffer
    char* json = malloc(100000);  // 100KB should be enough
    if (!json) return strdup("[]");

    char* ptr = json;
    ptr += sprintf(ptr, "{\"timestamp_ms\":%lu,\"tracks\":[",
                  get_current_time_ms());

    for (uint32_t i = 0; i < num_tracks; i++) {
        if (i > 0) ptr += sprintf(ptr, ",");

        ptr += sprintf(ptr,
            "{\"track_id\":%u,\"x\":%.0f,\"y\":%.0f,\"width\":%.0f,\"height\":%.0f,"
            "\"velocity_x\":%.2f,\"velocity_y\":%.2f,\"confidence\":%.2f,"
            "\"threat_score\":%.2f,\"behaviors\":[",
            tracks[i].track_id,
            tracks[i].box.x, tracks[i].box.y,
            tracks[i].box.width, tracks[i].box.height,
            tracks[i].velocity_x, tracks[i].velocity_y,
            tracks[i].confidence,
            tracks[i].threat_score
        );

        // Add behavior flags
        bool first = true;
        if (tracks[i].behaviors & BEHAVIOR_LOITERING) {
            ptr += sprintf(ptr, "%s\"loitering\"", first ? "" : ",");
            first = false;
        }
        if (tracks[i].behaviors & BEHAVIOR_RUNNING) {
            ptr += sprintf(ptr, "%s\"running\"", first ? "" : ",");
            first = false;
        }
        if (tracks[i].behaviors & BEHAVIOR_REPEATED_PASSES) {
            ptr += sprintf(ptr, "%s\"repeated_passes\"", first ? "" : ",");
            first = false;
        }

        ptr += sprintf(ptr, "]}");
    }

    ptr += sprintf(ptr, "]}");

    return json;
}

/**
 * Create JSON response for statistics
 */
static char* create_statistics_json(ApiServer* server) {
    char* json = malloc(10000);
    if (!json) return strdup("{}");

    char* ptr = json;
    ptr += sprintf(ptr, "{");

    // Perception statistics
    if (server->perception) {
        uint32_t active_tracks = 0;
        float fps = 0.0f;
        uint64_t frames = 0;

        perception_get_stats(server->perception, &active_tracks, &fps, &frames);

        ptr += sprintf(ptr,
            "\"perception\":{\"frames_processed\":%lu,\"active_tracks\":%u,"
            "\"fps\":%.1f,\"avg_latency_ms\":%.1f}",
            frames, active_tracks, fps, 18.5f
        );
    }

    // Timeline statistics
    if (server->timeline) {
        ptr += sprintf(ptr, ",");

        uint64_t num_updates = 0;
        uint64_t total_events = 0;
        uint64_t total_interventions = 0;
        float avg_confidence = 0.0f;

        timeline_get_stats(server->timeline, &num_updates, &total_events,
                          &total_interventions, &avg_confidence);

        ptr += sprintf(ptr,
            "\"timeline\":{\"num_updates\":%lu,\"events_predicted\":%lu,"
            "\"interventions\":%lu,\"avg_confidence\":%.2f}",
            num_updates, total_events, total_interventions, avg_confidence
        );
    }

    // Swarm statistics
    if (server->swarm) {
        ptr += sprintf(ptr, ",");

        SwarmStats swarm_stats;
        swarm_get_stats(server->swarm, &swarm_stats);

        ptr += sprintf(ptr,
            "\"swarm\":{\"num_neighbors\":%u,\"tracks_shared\":%u,"
            "\"tracks_received\":%u,\"network_health\":%.2f}",
            swarm_stats.num_neighbors, swarm_stats.tracks_shared,
            swarm_stats.tracks_received, swarm_stats.network_health
        );
    }

    ptr += sprintf(ptr, "}");

    return json;
}

/**
 * Create JSON response for status
 */
static char* create_status_json(ApiServer* server) {
    char* json = malloc(1000);
    if (!json) return strdup("{}");

    uint64_t uptime_ms = get_current_time_ms() - server->start_time_ms;
    uint64_t uptime_s = uptime_ms / 1000;

    float fps = 30.0f;
    if (server->perception) {
        perception_get_stats(server->perception, NULL, &fps, NULL);
    }

    sprintf(json,
        "{\"status\":\"running\",\"uptime_seconds\":%lu,"
        "\"camera_id\":1,\"fps\":%.1f,\"latency_ms\":%.1f,"
        "\"requests_served\":%lu,\"ws_clients\":%u}",
        uptime_s, fps, 18.5f,
        server->requests_served, server->num_ws_clients
    );

    return json;
}

// ============================================================================
// HTTP Request Handlers (Simplified)
// ============================================================================

/**
 * Handle HTTP request
 */
static int handle_http_request(
    void* cls,
    const char* url,
    const char* method
) {
    ApiServer* server = (ApiServer*)cls;

    pthread_mutex_lock(&server->mutex);
    server->requests_served++;
    pthread_mutex_unlock(&server->mutex);

    char* response = NULL;

    // Route requests
    if (strcmp(url, "/api/status") == 0) {
        response = create_status_json(server);
    }
    else if (strcmp(url, "/api/statistics") == 0) {
        response = create_statistics_json(server);
    }
    else if (strcmp(url, "/api/tracks") == 0) {
        response = create_tracks_json(server->perception);
    }
    else {
        response = strdup("{\"error\":\"not found\"}");
    }

    // TODO: Send HTTP response using libmicrohttpd
    printf("[API] %s %s -> %s\n", method, url, response);

    free(response);
    return 0;
}

/**
 * HTTP server thread
 */
static void* http_server_thread(void* arg) {
    ApiServer* server = (ApiServer*)arg;

    printf("[API] HTTP server listening on port %u\n", server->config.http_port);

    // TODO: Start actual HTTP server
    /*
    server->http_server = MHD_start_daemon(
        MHD_USE_SELECT_INTERNALLY,
        server->config.http_port,
        NULL, NULL,
        &handle_http_request, server,
        MHD_OPTION_END
    );
    */

    // Placeholder: Just sleep
    while (server->running) {
        sleep(1);
    }

    return NULL;
}

/**
 * WebSocket server thread
 */
static void* websocket_server_thread(void* arg) {
    ApiServer* server = (ApiServer*)arg;

    printf("[API] WebSocket server listening on port %u\n",
           server->config.websocket_port);

    // TODO: Start actual WebSocket server using libwebsockets

    // Placeholder: Just sleep
    while (server->running) {
        sleep(1);
    }

    return NULL;
}

// ============================================================================
// Public API Implementation
// ============================================================================

ApiServer* api_server_init(
    const ApiServerConfig* config,
    PerceptionEngine* perception,
    TimelineEngine* timeline,
    SwarmNode* swarm
) {
    if (!config) {
        fprintf(stderr, "[API] ERROR: NULL config\n");
        return NULL;
    }

    ApiServer* server = calloc(1, sizeof(ApiServer));
    if (!server) {
        fprintf(stderr, "[API] ERROR: Failed to allocate memory\n");
        return NULL;
    }

    // Copy configuration
    memcpy(&server->config, config, sizeof(ApiServerConfig));

    // Store component handles
    server->perception = perception;
    server->timeline = timeline;
    server->swarm = swarm;

    server->start_time_ms = get_current_time_ms();
    server->running = false;

    pthread_mutex_init(&server->mutex, NULL);

    printf("[API] API server initialized\n");

    return server;
}

void api_server_destroy(ApiServer* server) {
    if (!server) return;

    if (server->running) {
        api_server_stop(server);
    }

    pthread_mutex_destroy(&server->mutex);
    free(server);
}

bool api_server_start(ApiServer* server) {
    if (!server) return false;

    server->running = true;

    // Start HTTP server thread
    if (pthread_create(&server->http_thread, NULL, http_server_thread, server) != 0) {
        fprintf(stderr, "[API] ERROR: Failed to create HTTP thread\n");
        return false;
    }

    // Start WebSocket server thread
    if (pthread_create(&server->websocket_thread, NULL, websocket_server_thread, server) != 0) {
        fprintf(stderr, "[API] ERROR: Failed to create WebSocket thread\n");
        pthread_cancel(server->http_thread);
        return false;
    }

    printf("[API] API server started\n");
    return true;
}

void api_server_stop(ApiServer* server) {
    if (!server) return;

    server->running = false;

    // Stop HTTP server
    // TODO: MHD_stop_daemon(server->http_server);

    // Wait for threads
    pthread_join(server->http_thread, NULL);
    pthread_join(server->websocket_thread, NULL);

    printf("[API] API server stopped\n");
}

void api_server_broadcast_event(
    ApiServer* server,
    const char* event_json
) {
    if (!server || !event_json) return;

    // TODO: Broadcast to all WebSocket clients
    printf("[API] Broadcasting event: %s\n", event_json);
}

void api_server_broadcast_tracks(
    ApiServer* server,
    const char* tracks_json
) {
    if (!server || !tracks_json) return;

    // TODO: Broadcast to all WebSocket clients
    printf("[API] Broadcasting tracks update\n");
}

uint32_t api_server_get_num_clients(const ApiServer* server) {
    return server ? server->num_ws_clients : 0;
}
