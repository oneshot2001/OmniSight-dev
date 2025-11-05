/**
 * OMNISIGHT HTTP Server - Simplified Implementation
 *
 * Uses Mongoose embedded web server with correct OMNISIGHT core API.
 * This version correctly interfaces with omnisight_get_stats() and related functions.
 *
 * Phase 4 - v0.7.0
 */

#include "http_server.h"
#include "mongoose.h"
#include "../omnisight_core.h"
#include "../perception/perception.h"
#include "../timeline/timeline.h"
#include "../swarm/swarm.h"
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
    char web_root[256];  // Path to web root for static files
};

// Global server instance
static HTTPServer* g_server = NULL;

// Forward declarations
static void event_handler(struct mg_connection *c, int ev, void *ev_data);
static void handle_api_health(struct mg_connection *c);
static void handle_api_status(struct mg_connection *c);
static void handle_api_stats(struct mg_connection *c);
static void handle_perception_status(struct mg_connection *c);
static void handle_perception_detections(struct mg_connection *c);
static void handle_timeline_predictions(struct mg_connection *c);
static void handle_swarm_network(struct mg_connection *c);
static void handle_config_get(struct mg_connection *c);
static void send_json(struct mg_connection *c, int status, const char* json);
static void send_error(struct mg_connection *c, int status, const char* msg);

/**
 * Create HTTP server instance
 */
HTTPServer* http_server_create(OmnisightCore* core, int port, const char* web_root) {
    HTTPServer* server = (HTTPServer*)malloc(sizeof(HTTPServer));
    if (!server) {
        fprintf(stderr, "Failed to allocate HTTP server\n");
        return NULL;
    }

    server->core = core;
    server->port = port;
    server->running = false;

    // Set web root (default for ACAP if not specified)
    if (web_root) {
        snprintf(server->web_root, sizeof(server->web_root), "%s", web_root);
    } else {
        snprintf(server->web_root, sizeof(server->web_root), "/usr/local/packages/omnisightv2/html");
    }

    mg_mgr_init(&server->mgr);
    g_server = server;

    printf("Web root: %s\n", server->web_root);

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

static void event_handler(struct mg_connection *c, int ev, void *ev_data) {
    if (ev == MG_EV_HTTP_MSG) {
        struct mg_http_message *hm = (struct mg_http_message *)ev_data;

        // Handle OPTIONS (CORS preflight)
        if (mg_strcmp(hm->method, mg_str("OPTIONS")) == 0) {
            mg_http_reply(c, 204, "Access-Control-Allow-Origin: *\r\n", "");
            return;
        }

        // Route API requests
        if (mg_match(hm->uri, mg_str("/api/health"), NULL)) {
            handle_api_health(c);
        }
        else if (mg_match(hm->uri, mg_str("/api/status"), NULL)) {
            handle_api_status(c);
        }
        else if (mg_match(hm->uri, mg_str("/api/stats"), NULL)) {
            handle_api_stats(c);
        }
        else if (mg_match(hm->uri, mg_str("/api/perception/status"), NULL)) {
            handle_perception_status(c);
        }
        else if (mg_match(hm->uri, mg_str("/api/perception/detections"), NULL)) {
            handle_perception_detections(c);
        }
        else if (mg_match(hm->uri, mg_str("/api/timeline/predictions"), NULL)) {
            handle_timeline_predictions(c);
        }
        else if (mg_match(hm->uri, mg_str("/api/swarm/network"), NULL)) {
            handle_swarm_network(c);
        }
        else if (mg_match(hm->uri, mg_str("/api/config"), NULL)) {
            handle_config_get(c);
        }
        else {
            // Serve static files from configured web root
            struct mg_http_serve_opts opts = {
                .root_dir = g_server->web_root,
                .extra_headers = "Access-Control-Allow-Origin: *\r\n"
            };
            mg_http_serve_dir(c, hm, &opts);
        }
    }
}

// ============================================================================
// API Endpoint Handlers
// ============================================================================

static void handle_api_health(struct mg_connection *c) {
    send_json(c, 200, "{\"status\":\"healthy\",\"version\":\"0.7.0\"}");
}

static void handle_api_status(struct mg_connection *c) {
    if (!g_server || !g_server->core) {
        send_error(c, 500, "Server not initialized");
        return;
    }

    OmnisightStats stats;
    omnisight_get_stats(g_server->core, &stats);

    char* json = mg_mprintf(
        "{"
        "\"status\":\"active\","
        "\"mode\":\"acap\","
        "\"version\":\"0.7.0\","
        "\"uptime_seconds\":%lu,"
        "\"fps\":%.1f,"
        "\"latency_ms\":%.1f,"
        "\"modules\":{"
            "\"perception\":{\"status\":\"active\",\"fps\":%.1f,\"detections\":%u},"
            "\"timeline\":{\"status\":\"active\",\"predictions\":%u},"
            "\"swarm\":{\"status\":\"active\",\"cameras\":1}"
        "}"
        "}",
        (unsigned long)(stats.uptime_ms / 1000),
        stats.perception_stats.avg_fps,
        stats.perception_stats.avg_inference_ms,
        stats.perception_stats.avg_fps,
        stats.perception_stats.tracked_objects,
        stats.timeline_stats.active_timelines
    );

    send_json(c, 200, json);
    free(json);
}

static void handle_api_stats(struct mg_connection *c) {
    if (!g_server || !g_server->core) {
        send_error(c, 500, "Server not initialized");
        return;
    }

    OmnisightStats stats;
    omnisight_get_stats(g_server->core, &stats);

    char* json = mg_mprintf(
        "{"
        "\"uptime_seconds\":%lu,"
        "\"memory_usage_mb\":128,"
        "\"cpu_usage_percent\":15.5,"
        "\"fps\":%.1f"
        "}",
        (unsigned long)(stats.uptime_ms / 1000),
        stats.perception_stats.avg_fps
    );

    send_json(c, 200, json);
    free(json);
}

static void handle_perception_status(struct mg_connection *c) {
    if (!g_server || !g_server->core) {
        send_error(c, 500, "Server not initialized");
        return;
    }

    OmnisightStats stats;
    omnisight_get_stats(g_server->core, &stats);

    char* json = mg_mprintf(
        "{"
        "\"status\":\"active\","
        "\"fps\":%.1f,"
        "\"detections_count\":%u,"
        "\"inference_time_ms\":%.1f"
        "}",
        stats.perception_stats.avg_fps,
        stats.perception_stats.tracked_objects,
        stats.perception_stats.avg_inference_ms
    );

    send_json(c, 200, json);
    free(json);
}

static void handle_perception_detections(struct mg_connection *c) {
    if (!g_server || !g_server->core) {
        send_error(c, 500, "Server not initialized");
        return;
    }

    // Get tracked objects from core
    TrackedObject objects[50];
    uint32_t count = omnisight_get_tracked_objects(g_server->core, objects, 50);

    // Build JSON using simple string concatenation
    char* json = mg_mprintf("{\"detections\":[");

    for (uint32_t i = 0; i < count; i++) {
        const TrackedObject* obj = &objects[i];

        char* obj_json = mg_mprintf(
            "%s{"
            "\"object_id\":%u,"
            "\"class\":\"object\","
            "\"confidence\":%.2f,"
            "\"bbox\":{\"x\":%.0f,\"y\":%.0f,\"width\":%.0f,\"height\":%.0f},"
            "\"velocity\":{\"x\":%.2f,\"y\":%.2f},"
            "\"threat_score\":%.2f"
            "}",
            (i > 0) ? "," : "",
            obj->track_id,
            obj->confidence,
            obj->current_bbox.x, obj->current_bbox.y,
            obj->current_bbox.width, obj->current_bbox.height,
            obj->velocity_x, obj->velocity_y,
            obj->threat_score
        );

        char* new_json = mg_mprintf("%s%s", json, obj_json);
        free(json);
        free(obj_json);
        json = new_json;
    }

    char* final_json = mg_mprintf("%s]}", json);
    free(json);

    send_json(c, 200, final_json);
    free(final_json);
}

static void handle_timeline_predictions(struct mg_connection *c) {
    if (!g_server || !g_server->core) {
        send_error(c, 500, "Server not initialized");
        return;
    }

    // Get timelines from core
    Timeline timelines[5];
    uint32_t count = omnisight_get_timelines(g_server->core, timelines, 5);

    // Build JSON using simple string concatenation
    char* json = mg_mprintf("{\"predictions\":[");

    for (uint32_t i = 0; i < count; i++) {
        const Timeline* tl = &timelines[i];
        (void)tl; // Unused for now

        char* tl_json = mg_mprintf(
            "%s{"
            "\"future_id\":\"%c\","
            "\"probability\":%.2f,"
            "\"time_horizon_seconds\":300,"
            "\"predicted_events\":[]"
            "}",
            (i > 0) ? "," : "",
            'A' + i,
            0.5f + (i * 0.1f)  // Placeholder probabilities
        );

        char* new_json = mg_mprintf("%s%s", json, tl_json);
        free(json);
        free(tl_json);
        json = new_json;
    }

    char* final_json = mg_mprintf("%s]}", json);
    free(json);

    send_json(c, 200, final_json);
    free(final_json);
}

static void handle_swarm_network(struct mg_connection *c) {
    if (!g_server || !g_server->core) {
        send_error(c, 500, "Server not initialized");
        return;
    }

    OmnisightStats stats;
    omnisight_get_stats(g_server->core, &stats);

    char* json = mg_mprintf(
        "{"
        "\"network_status\":\"connected\","
        "\"cameras\":["
            "{"
            "\"camera_id\":\"local\","
            "\"status\":\"active\","
            "\"latency_ms\":5.2"
            "}"
        "]"
        "}"
    );

    send_json(c, 200, json);
    free(json);
}

static void handle_config_get(struct mg_connection *c) {
    send_json(c, 200,
        "{"
        "\"perception\":{\"fps_target\":25,\"confidence_threshold\":0.7},"
        "\"timeline\":{\"prediction_horizon\":300,\"max_futures\":5},"
        "\"swarm\":{\"mqtt_broker\":\"localhost:1883\",\"enabled\":false}"
        "}"
    );
}

// ============================================================================
// Utility Functions
// ============================================================================

static void send_json(struct mg_connection *c, int status, const char* json) {
    mg_http_reply(c, status,
                  "Content-Type: application/json\r\n"
                  "Access-Control-Allow-Origin: *\r\n",
                  "%s", json);
}

static void send_error(struct mg_connection *c, int status, const char* msg) {
    char* json = mg_mprintf("{\"error\":\"%s\"}", msg);
    send_json(c, status, json);
    free(json);
}
