/**
 * OMNISIGHT HTTP Server - Native C Implementation
 *
 * Uses Mongoose embedded web server for REST API and static file serving.
 * Provides all API endpoints for perception, timeline, and swarm modules.
 *
 * Phase 4 - v0.7.0
 */

#include "http_server.h"
#include "mongoose.h"
#include "../omnisight_core.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// HTTP server state
struct HTTPServer {
    struct mg_mgr mgr;
    OmnisightCore* core;
    int port;
    bool running;
};

// Global server instance
static HTTPServer* g_server = NULL;

// Forward declarations
static void event_handler(struct mg_connection *c, int ev, void *ev_data);
static void handle_api_health(struct mg_connection *c, struct mg_http_message *hm);
static void handle_api_status(struct mg_connection *c, struct mg_http_message *hm);
static void handle_api_stats(struct mg_connection *c, struct mg_http_message *hm);
static void handle_perception_status(struct mg_connection *c, struct mg_http_message *hm);
static void handle_perception_detections(struct mg_connection *c, struct mg_http_message *hm);
static void handle_timeline_predictions(struct mg_connection *c, struct mg_http_message *hm);
static void handle_timeline_history(struct mg_connection *c, struct mg_http_message *hm);
static void handle_swarm_network(struct mg_connection *c, struct mg_http_message *hm);
static void handle_swarm_cameras(struct mg_connection *c, struct mg_http_message *hm);
static void handle_config_get(struct mg_connection *c, struct mg_http_message *hm);
static void handle_config_post(struct mg_connection *c, struct mg_http_message *hm);
static void handle_404(struct mg_connection *c, struct mg_http_message *hm);

// JSON utility functions
static char* get_current_timestamp(void);
static void send_json_response(struct mg_connection *c, int status_code, const char* json);
static void send_error_response(struct mg_connection *c, int status_code, const char* error_msg);

/**
 * Create HTTP server instance
 */
HTTPServer* http_server_create(OmnisightCore* core, int port) {
    HTTPServer* server = (HTTPServer*)malloc(sizeof(HTTPServer));
    if (!server) {
        fprintf(stderr, "Failed to allocate HTTP server\n");
        return NULL;
    }

    server->core = core;
    server->port = port;
    server->running = false;

    mg_mgr_init(&server->mgr);

    g_server = server;

    return server;
}

/**
 * Start HTTP server
 */
bool http_server_start(HTTPServer* server) {
    if (!server) return false;

    char addr[64];
    snprintf(addr, sizeof(addr), "http://0.0.0.0:%d", server->port);

    struct mg_connection *c = mg_http_listen(&server->mgr, addr, event_handler, server);
    if (!c) {
        fprintf(stderr, "Failed to start HTTP server on port %d\n", server->port);
        return false;
    }

    server->running = true;

    printf("HTTP server started on port %d\n", server->port);
    printf("Endpoints:\n");
    printf("  GET  /api/health\n");
    printf("  GET  /api/status\n");
    printf("  GET  /api/stats\n");
    printf("  GET  /api/perception/status\n");
    printf("  GET  /api/perception/detections\n");
    printf("  GET  /api/timeline/predictions\n");
    printf("  GET  /api/timeline/history\n");
    printf("  GET  /api/swarm/network\n");
    printf("  GET  /api/swarm/cameras\n");
    printf("  GET  /api/config\n");
    printf("  POST /api/config\n");

    return true;
}

/**
 * Process HTTP server events
 */
void http_server_poll(HTTPServer* server, int timeout_ms) {
    if (!server || !server->running) return;
    mg_mgr_poll(&server->mgr, timeout_ms);
}

/**
 * Stop HTTP server
 */
void http_server_stop(HTTPServer* server) {
    if (!server) return;

    server->running = false;
    printf("HTTP server stopped\n");
}

/**
 * Destroy HTTP server
 */
void http_server_destroy(HTTPServer* server) {
    if (!server) return;

    http_server_stop(server);
    mg_mgr_free(&server->mgr);
    free(server);

    if (g_server == server) {
        g_server = NULL;
    }
}

/**
 * Check if server is running
 */
bool http_server_is_running(const HTTPServer* server) {
    return server && server->running;
}

// ============================================================================
// HTTP Event Handler and Router
// ============================================================================

/**
 * Main event handler for all HTTP connections
 */
static void event_handler(struct mg_connection *c, int ev, void *ev_data) {
    if (ev == MG_EV_HTTP_MSG) {
        struct mg_http_message *hm = (struct mg_http_message *)ev_data;

        // Add CORS headers
        mg_printf(c, "Access-Control-Allow-Origin: *\r\n");
        mg_printf(c, "Access-Control-Allow-Methods: GET, POST, OPTIONS\r\n");
        mg_printf(c, "Access-Control-Allow-Headers: Content-Type\r\n");

        // Handle OPTIONS preflight requests
        if (mg_vcmp(&hm->method, "OPTIONS") == 0) {
            mg_http_reply(c, 204, "Access-Control-Allow-Origin: *\r\n", "");
            return;
        }

        // Route API requests
        if (mg_http_match_uri(hm, "/api/health")) {
            handle_api_health(c, hm);
        }
        else if (mg_http_match_uri(hm, "/api/status")) {
            handle_api_status(c, hm);
        }
        else if (mg_http_match_uri(hm, "/api/stats")) {
            handle_api_stats(c, hm);
        }
        else if (mg_http_match_uri(hm, "/api/perception/status")) {
            handle_perception_status(c, hm);
        }
        else if (mg_http_match_uri(hm, "/api/perception/detections")) {
            handle_perception_detections(c, hm);
        }
        else if (mg_http_match_uri(hm, "/api/timeline/predictions")) {
            handle_timeline_predictions(c, hm);
        }
        else if (mg_http_match_uri(hm, "/api/timeline/history")) {
            handle_timeline_history(c, hm);
        }
        else if (mg_http_match_uri(hm, "/api/swarm/network")) {
            handle_swarm_network(c, hm);
        }
        else if (mg_http_match_uri(hm, "/api/swarm/cameras")) {
            handle_swarm_cameras(c, hm);
        }
        else if (mg_http_match_uri(hm, "/api/config")) {
            if (mg_vcmp(&hm->method, "GET") == 0) {
                handle_config_get(c, hm);
            } else if (mg_vcmp(&hm->method, "POST") == 0) {
                handle_config_post(c, hm);
            } else {
                handle_404(c, hm);
            }
        }
        else {
            // Serve static files from /html directory (for React dashboard)
            struct mg_http_serve_opts opts = {
                .root_dir = "/usr/local/packages/omnisightv2/html",
                .ssi_pattern = NULL,
                .extra_headers = "Access-Control-Allow-Origin: *\r\n"
            };
            mg_http_serve_dir(c, hm, &opts);
        }
    }
}

// ============================================================================
// API Endpoint Handlers
// ============================================================================

/**
 * GET /api/health - Health check
 */
static void handle_api_health(struct mg_connection *c, struct mg_http_message *hm) {
    (void)hm; // Unused parameter

    const char* json = "{\"status\":\"healthy\",\"version\":\"0.7.0\"}";
    send_json_response(c, 200, json);
}

/**
 * GET /api/status - System status
 */
static void handle_api_status(struct mg_connection *c, struct mg_http_message *hm) {
    (void)hm; // Unused parameter

    if (!g_server || !g_server->core) {
        send_error_response(c, 500, "Server not initialized");
        return;
    }

    OmnisightStatus status = omnisight_get_status(g_server->core);

    char* json = mg_mprintf(
        "{"
        "\"status\":\"%s\","
        "\"mode\":\"acap\","
        "\"version\":\"0.7.0\","
        "\"uptime_seconds\":%d,"
        "\"fps\":%.1f,"
        "\"latency_ms\":%.1f,"
        "\"camera_id\":\"P3285-LVE\","
        "\"modules\":{"
            "\"perception\":{\"status\":\"active\",\"fps\":%.1f},"
            "\"timeline\":{\"status\":\"active\",\"predictions\":3},"
            "\"swarm\":{\"status\":\"active\",\"cameras\":1}"
        "}"
        "}",
        status.status,
        status.uptime_seconds,
        status.fps,
        status.latency_ms,
        status.fps
    );

    send_json_response(c, 200, json);
    free(json);
}

/**
 * GET /api/stats - System statistics
 */
static void handle_api_stats(struct mg_connection *c, struct mg_http_message *hm) {
    (void)hm; // Unused parameter

    if (!g_server || !g_server->core) {
        send_error_response(c, 500, "Server not initialized");
        return;
    }

    OmnisightStatus status = omnisight_get_status(g_server->core);

    char* json = mg_mprintf(
        "{"
        "\"uptime_seconds\":%d,"
        "\"memory_usage_mb\":128,"
        "\"cpu_usage_percent\":15.5,"
        "\"fps\":%.1f"
        "}",
        status.uptime_seconds,
        status.fps
    );

    send_json_response(c, 200, json);
    free(json);
}

/**
 * GET /api/perception/status - Perception module status
 */
static void handle_perception_status(struct mg_connection *c, struct mg_http_message *hm) {
    (void)hm; // Unused parameter

    if (!g_server || !g_server->core) {
        send_error_response(c, 500, "Server not initialized");
        return;
    }

    PerceptionState state = perception_get_state(g_server->core->perception);

    char* json = mg_mprintf(
        "{"
        "\"status\":\"active\","
        "\"fps\":%.1f,"
        "\"detections_count\":%d,"
        "\"inference_time_ms\":%.1f"
        "}",
        state.fps,
        state.detection_count,
        15.2 // Placeholder - add to PerceptionState if needed
    );

    send_json_response(c, 200, json);
    free(json);
}

/**
 * GET /api/perception/detections - Current detections
 */
static void handle_perception_detections(struct mg_connection *c, struct mg_http_message *hm) {
    (void)hm; // Unused parameter

    if (!g_server || !g_server->core) {
        send_error_response(c, 500, "Server not initialized");
        return;
    }

    PerceptionState state = perception_get_state(g_server->core->perception);
    char* timestamp = get_current_timestamp();

    // Build JSON array of detections
    struct mg_str json_str = {0};
    json_str.ptr = mg_mprintf("{\"timestamp\":\"%s\",\"detections\":[", timestamp);
    json_str.len = strlen(json_str.ptr);

    for (int i = 0; i < state.detection_count && i < MAX_DETECTIONS; i++) {
        const Detection* det = &state.detections[i];
        char* det_json = mg_mprintf(
            "%s{"
            "\"object_id\":%d,"
            "\"class\":\"%s\","
            "\"confidence\":%.2f,"
            "\"bbox\":{\"x\":%d,\"y\":%d,\"width\":%d,\"height\":%d},"
            "\"velocity\":{\"x\":%.2f,\"y\":%.2f}"
            "}",
            (i > 0) ? "," : "",
            det->object_id,
            det->class_name,
            det->confidence,
            det->bbox.x, det->bbox.y, det->bbox.width, det->bbox.height,
            det->velocity.x, det->velocity.y
        );

        char* new_json = mg_mprintf("%s%s", json_str.ptr, det_json);
        free((void*)json_str.ptr);
        free(det_json);
        json_str.ptr = new_json;
        json_str.len = strlen(new_json);
    }

    char* final_json = mg_mprintf("%s]}", json_str.ptr);
    free((void*)json_str.ptr);
    free(timestamp);

    send_json_response(c, 200, final_json);
    free(final_json);
}

/**
 * GET /api/timeline/predictions - Timeline predictions
 */
static void handle_timeline_predictions(struct mg_connection *c, struct mg_http_message *hm) {
    (void)hm; // Unused parameter

    if (!g_server || !g_server->core) {
        send_error_response(c, 500, "Server not initialized");
        return;
    }

    TimelineState state = timeline_get_state(g_server->core->timeline);

    // Build JSON array of predictions
    struct mg_str json_str = {0};
    json_str.ptr = mg_mprintf("{\"predictions\":[");
    json_str.len = strlen(json_str.ptr);

    for (int i = 0; i < state.prediction_count; i++) {
        const TimelinePrediction* pred = &state.predictions[i];

        char* pred_json = mg_mprintf(
            "%s{"
            "\"future_id\":\"%s\","
            "\"probability\":%.2f,"
            "\"time_horizon_seconds\":%d,"
            "\"predicted_events\":[]"
            "}",
            (i > 0) ? "," : "",
            pred->future_id,
            pred->probability,
            pred->time_horizon_seconds
        );

        char* new_json = mg_mprintf("%s%s", json_str.ptr, pred_json);
        free((void*)json_str.ptr);
        free(pred_json);
        json_str.ptr = new_json;
        json_str.len = strlen(new_json);
    }

    char* final_json = mg_mprintf("%s]}", json_str.ptr);
    free((void*)json_str.ptr);

    send_json_response(c, 200, final_json);
    free(final_json);
}

/**
 * GET /api/timeline/history - Timeline event history
 */
static void handle_timeline_history(struct mg_connection *c, struct mg_http_message *hm) {
    (void)hm; // Unused parameter

    // Placeholder - implement when timeline history tracking is added
    const char* json = "{\"events\":[]}";
    send_json_response(c, 200, json);
}

/**
 * GET /api/swarm/network - Swarm network status
 */
static void handle_swarm_network(struct mg_connection *c, struct mg_http_message *hm) {
    (void)hm; // Unused parameter

    if (!g_server || !g_server->core) {
        send_error_response(c, 500, "Server not initialized");
        return;
    }

    SwarmState state = swarm_get_state(g_server->core->swarm);

    char* json = mg_mprintf(
        "{"
        "\"network_status\":\"%s\","
        "\"cameras\":["
            "{"
            "\"camera_id\":\"P3285-LVE\","
            "\"status\":\"active\","
            "\"latency_ms\":5.2"
            "}"
        "]"
        "}",
        state.network_status
    );

    send_json_response(c, 200, json);
    free(json);
}

/**
 * GET /api/swarm/cameras - All cameras in swarm
 */
static void handle_swarm_cameras(struct mg_connection *c, struct mg_http_message *hm) {
    (void)hm; // Unused parameter

    const char* json =
        "{"
        "\"cameras\":["
            "{"
            "\"id\":\"P3285-LVE\","
            "\"ip\":\"192.168.1.100\","
            "\"status\":\"active\""
            "}"
        "]"
        "}";

    send_json_response(c, 200, json);
}

/**
 * GET /api/config - Get configuration
 */
static void handle_config_get(struct mg_connection *c, struct mg_http_message *hm) {
    (void)hm; // Unused parameter

    const char* json =
        "{"
        "\"perception\":{\"fps_target\":25,\"confidence_threshold\":0.7},"
        "\"timeline\":{\"prediction_horizon\":300,\"max_futures\":5},"
        "\"swarm\":{\"mqtt_broker\":\"localhost:1883\",\"enabled\":false}"
        "}";

    send_json_response(c, 200, json);
}

/**
 * POST /api/config - Update configuration
 */
static void handle_config_post(struct mg_connection *c, struct mg_http_message *hm) {
    (void)hm; // Unused parameter

    // TODO: Parse request body and update configuration
    const char* json = "{\"status\":\"updated\"}";
    send_json_response(c, 200, json);
}

/**
 * 404 handler
 */
static void handle_404(struct mg_connection *c, struct mg_http_message *hm) {
    (void)hm; // Unused parameter

    send_error_response(c, 404, "Endpoint not found");
}

// ============================================================================
// JSON Utility Functions
// ============================================================================

/**
 * Get current timestamp in ISO 8601 format
 */
static char* get_current_timestamp(void) {
    time_t now = time(NULL);
    struct tm* tm_info = gmtime(&now);

    char* timestamp = (char*)malloc(32);
    strftime(timestamp, 32, "%Y-%m-%dT%H:%M:%SZ", tm_info);

    return timestamp;
}

/**
 * Send JSON response with proper headers
 */
static void send_json_response(struct mg_connection *c, int status_code, const char* json) {
    mg_http_reply(c, status_code,
                  "Content-Type: application/json\r\n"
                  "Access-Control-Allow-Origin: *\r\n",
                  "%s", json);
}

/**
 * Send error response
 */
static void send_error_response(struct mg_connection *c, int status_code, const char* error_msg) {
    char* json = mg_mprintf("{\"error\":\"%s\"}", error_msg);
    send_json_response(c, status_code, json);
    free(json);
}
