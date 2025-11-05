/**
 * OMNISIGHT HTTP Server - Simple Test
 *
 * Minimal test without OMNISIGHT core dependencies
 * Tests only the HTTP routing and basic responses
 *
 * Phase 4 - v0.7.0
 */

#include "mongoose.h"
#include <stdio.h>
#include <signal.h>
#include <stdbool.h>

static volatile bool g_running = true;

static void signal_handler(int signum) {
    (void)signum;
    printf("\nShutting down...\n");
    g_running = false;
}

static void event_handler(struct mg_connection *c, int ev, void *ev_data) {
    if (ev == MG_EV_HTTP_MSG) {
        struct mg_http_message *hm = (struct mg_http_message *)ev_data;

        // Simple routing using mg_match()
        if (mg_match(hm->uri, mg_str("/api/health"), NULL)) {
            mg_http_reply(c, 200,
                         "Content-Type: application/json\r\n",
                         "{\"status\":\"healthy\",\"version\":\"0.7.0\"}\n");
        }
        else if (mg_match(hm->uri, mg_str("/api/status"), NULL)) {
            mg_http_reply(c, 200,
                         "Content-Type: application/json\r\n",
                         "{\"status\":\"active\",\"mode\":\"test\"}\n");
        }
        else {
            mg_http_reply(c, 404,
                         "Content-Type: application/json\r\n",
                         "{\"error\":\"Not found\"}\n");
        }
    }
}

int main(void) {
    struct mg_mgr mgr;
    const char *port = "8081";  // Use 8081 to avoid conflicts

    signal(SIGINT, signal_handler);

    mg_mgr_init(&mgr);

    printf("Starting HTTP server on http://localhost:%s\n", port);
    printf("\n");
    printf("Test endpoints:\n");
    printf("  curl http://localhost:%s/api/health\n", port);
    printf("  curl http://localhost:%s/api/status\n", port);
    printf("\n");
    printf("Press Ctrl+C to stop\n\n");

    char url[64];
    snprintf(url, sizeof(url), "http://0.0.0.0:%s", port);
    struct mg_connection *c = mg_http_listen(&mgr, url, event_handler, NULL);
    if (!c) {
        fprintf(stderr, "Failed to start server\n");
        return 1;
    }

    while (g_running) {
        mg_mgr_poll(&mgr, 1000);
    }

    mg_mgr_free(&mgr);
    printf("Shutdown complete\n");

    return 0;
}
