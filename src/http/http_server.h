/**
 * OMNISIGHT HTTP Server - Native C Implementation
 *
 * Header file for Mongoose-based HTTP server
 * Provides REST API and static file serving
 *
 * Phase 4 - v0.7.0
 */

#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H

#include <stdbool.h>

// Forward declarations
typedef struct OmnisightCore OmnisightCore;
typedef struct HTTPServer HTTPServer;

/**
 * Create HTTP server instance
 *
 * @param core Pointer to OMNISIGHT core instance
 * @param port Port number to listen on (typically 8080)
 * @param web_root Path to web root directory for static files (or NULL for default)
 * @return Pointer to HTTPServer instance, or NULL on failure
 */
HTTPServer* http_server_create(OmnisightCore* core, int port, const char* web_root);

/**
 * Start HTTP server
 *
 * Begins listening for HTTP connections on configured port.
 * Server runs in non-blocking mode - must call http_server_poll() regularly.
 *
 * @param server Pointer to HTTPServer instance
 * @return true if server started successfully, false otherwise
 */
bool http_server_start(HTTPServer* server);

/**
 * Process HTTP server events
 *
 * Must be called regularly to handle incoming HTTP requests.
 * Typically called in main event loop with 100-1000ms timeout.
 *
 * @param server Pointer to HTTPServer instance
 * @param timeout_ms Timeout in milliseconds for event polling
 */
void http_server_poll(HTTPServer* server, int timeout_ms);

/**
 * Stop HTTP server
 *
 * Stops accepting new connections but does not free resources.
 * Use http_server_destroy() to fully clean up.
 *
 * @param server Pointer to HTTPServer instance
 */
void http_server_stop(HTTPServer* server);

/**
 * Destroy HTTP server
 *
 * Stops server and frees all resources.
 *
 * @param server Pointer to HTTPServer instance
 */
void http_server_destroy(HTTPServer* server);

/**
 * Check if server is running
 *
 * @param server Pointer to HTTPServer instance
 * @return true if server is running, false otherwise
 */
bool http_server_is_running(const HTTPServer* server);

#endif // HTTP_SERVER_H
