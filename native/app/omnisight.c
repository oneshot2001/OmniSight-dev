/**
 * OMNISIGHT - Precognitive Security System
 * Native ACAP Implementation using CivetWeb
 *
 * Copyright (C) 2025, OMNISIGHT
 * Based on Axis Communications ACAP Native SDK examples
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "civetweb.h"
#include <signal.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <time.h>
#include <unistd.h>

#define PORT "8080"
volatile sig_atomic_t application_running = 1;

__attribute__((noreturn)) __attribute__((format(printf, 1, 2))) static void
panic(const char* format, ...) {
    va_list arg;
    va_start(arg, format);
    vsyslog(LOG_ERR, format, arg);
    va_end(arg);
    exit(1);
}

static void stop_application(int status) {
    (void)status;
    application_running = 0;
}

/* Root handler - serves the main HTML dashboard */
static int root_handler(struct mg_connection* conn, void* cb_data __attribute__((unused))) {
    mg_send_file(conn, "html/index.html");
    return 1;
}

/* Health endpoint handler */
static int health_handler(struct mg_connection* conn, void* cb_data __attribute__((unused))) {
    mg_send_file(conn, "api/health.json");
    return 1;
}

/* Stats endpoint handler */
static int stats_handler(struct mg_connection* conn, void* cb_data __attribute__((unused))) {
    mg_send_file(conn, "api/stats.json");
    return 1;
}

/* Perception status handler */
static int perception_status_handler(struct mg_connection* conn, void* cb_data __attribute__((unused))) {
    mg_send_file(conn, "api/perception/status.json");
    return 1;
}

/* Perception detections handler */
static int perception_detections_handler(struct mg_connection* conn, void* cb_data __attribute__((unused))) {
    mg_send_file(conn, "api/perception/detections.json");
    return 1;
}

/* Perception tracks handler */
static int perception_tracks_handler(struct mg_connection* conn, void* cb_data __attribute__((unused))) {
    mg_send_file(conn, "api/perception/tracks.json");
    return 1;
}

/* Timeline status handler */
static int timeline_status_handler(struct mg_connection* conn, void* cb_data __attribute__((unused))) {
    mg_send_file(conn, "api/timeline/status.json");
    return 1;
}

/* Timeline predictions handler */
static int timeline_predictions_handler(struct mg_connection* conn, void* cb_data __attribute__((unused))) {
    mg_send_file(conn, "api/timeline/predictions.json");
    return 1;
}

/* Timeline futures handler */
static int timeline_futures_handler(struct mg_connection* conn, void* cb_data __attribute__((unused))) {
    mg_send_file(conn, "api/timeline/futures.json");
    return 1;
}

/* Swarm status handler */
static int swarm_status_handler(struct mg_connection* conn, void* cb_data __attribute__((unused))) {
    mg_send_file(conn, "api/swarm/status.json");
    return 1;
}

/* Swarm cameras handler */
static int swarm_cameras_handler(struct mg_connection* conn, void* cb_data __attribute__((unused))) {
    mg_send_file(conn, "api/swarm/cameras.json");
    return 1;
}

/* Swarm sync handler */
static int swarm_sync_handler(struct mg_connection* conn, void* cb_data __attribute__((unused))) {
    mg_send_file(conn, "api/swarm/sync.json");
    return 1;
}

/* Config perception handler */
static int config_perception_handler(struct mg_connection* conn, void* cb_data __attribute__((unused))) {
    mg_send_file(conn, "api/config/perception.json");
    return 1;
}

/* Config timeline handler */
static int config_timeline_handler(struct mg_connection* conn, void* cb_data __attribute__((unused))) {
    mg_send_file(conn, "api/config/timeline.json");
    return 1;
}

/* Config swarm handler */
static int config_swarm_handler(struct mg_connection* conn, void* cb_data __attribute__((unused))) {
    mg_send_file(conn, "api/config/swarm.json");
    return 1;
}

int main(void) {
    signal(SIGTERM, stop_application);
    signal(SIGINT, stop_application);

    mg_init_library(0);

    struct mg_callbacks callbacks = {0};
    const char* options[] =
        {"listening_ports", PORT, "request_timeout_ms", "10000", "error_log_file", "error.log", 0};

    struct mg_context* context = mg_start(&callbacks, 0, options);
    if (!context) {
        panic("Failed to start OMNISIGHT web server on port %s", PORT);
    }

    syslog(LOG_INFO, "OMNISIGHT server started on port %s", PORT);

    /* Register all endpoint handlers */
    mg_set_request_handler(context, "/", root_handler, 0);
    mg_set_request_handler(context, "/api/health", health_handler, 0);
    mg_set_request_handler(context, "/api/stats", stats_handler, 0);

    /* Perception endpoints */
    mg_set_request_handler(context, "/api/perception/status", perception_status_handler, 0);
    mg_set_request_handler(context, "/api/perception/detections", perception_detections_handler, 0);
    mg_set_request_handler(context, "/api/perception/tracks", perception_tracks_handler, 0);

    /* Timeline endpoints */
    mg_set_request_handler(context, "/api/timeline/status", timeline_status_handler, 0);
    mg_set_request_handler(context, "/api/timeline/predictions", timeline_predictions_handler, 0);
    mg_set_request_handler(context, "/api/timeline/futures", timeline_futures_handler, 0);

    /* Swarm endpoints */
    mg_set_request_handler(context, "/api/swarm/status", swarm_status_handler, 0);
    mg_set_request_handler(context, "/api/swarm/cameras", swarm_cameras_handler, 0);
    mg_set_request_handler(context, "/api/swarm/sync", swarm_sync_handler, 0);

    /* Config endpoints */
    mg_set_request_handler(context, "/api/config/perception", config_perception_handler, 0);
    mg_set_request_handler(context, "/api/config/timeline", config_timeline_handler, 0);
    mg_set_request_handler(context, "/api/config/swarm", config_swarm_handler, 0);

    syslog(LOG_INFO, "OMNISIGHT: All API endpoints registered");

    while (application_running) {
        sleep(1);
    }

    syslog(LOG_INFO, "OMNISIGHT: Shutting down gracefully");
    mg_stop(context);
    mg_exit_library();

    return EXIT_SUCCESS;
}
