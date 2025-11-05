/**
 * @file vdo_capture.c
 * @brief OMNISIGHT VDO Video Capture Implementation
 *
 * Handles video stream capture from Axis camera using VDO API.
 * Based on Axis ACAP Native SDK examples.
 *
 * Copyright (C) 2025 OMNISIGHT
 */

#include "vdo_capture.h"

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <syslog.h>
#include <poll.h>
#include <sys/time.h>

#include <vdo-channel.h>
#include <vdo-map.h>

// Maximum number of VDO buffers (VDO limitation)
#define MAX_VDO_BUFFERS 5

// Default buffer count
#define DEFAULT_BUFFER_COUNT 3

/**
 * VDO Capture internal structure
 */
struct VdoCapture {
    // Configuration
    VdoCaptureConfig config;

    // VDO objects
    VdoStream* stream;
    VdoChannel* channel;

    // Stream info (updated dynamically)
    VdoFrameInfo frame_info;

    // State
    bool running;
    int fd;

    // Framerate adaptation
    unsigned int current_frametime_ms;
    unsigned int total_inference_time_ms;
    unsigned int inference_frame_count;

    // Statistics
    uint64_t frames_captured;
    uint64_t frames_dropped;

    // Thread safety
    pthread_mutex_t mutex;
};

// ============================================================================
// Helper Functions
// ============================================================================

/**
 * Get current time in milliseconds
 */
static uint64_t get_time_ms(void) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (uint64_t)tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

/**
 * Calculate optimal framerate based on inference time
 */
static void calculate_framerate(VdoCapture* capture, unsigned int inference_ms) {
    double new_framerate = capture->config.framerate;
    unsigned int new_frametime_ms = 0;

    // Adaptive framerate thresholds (from Axis examples)
    if (inference_ms > 201) {
        new_framerate = 1.0;
        new_frametime_ms = 1001;
    } else if (inference_ms < 34) {
        new_framerate = 30.0;
        new_frametime_ms = 34;
    } else if (inference_ms < 41) {
        new_framerate = 25.0;
        new_frametime_ms = 41;
    } else if (inference_ms < 51) {
        new_framerate = 20.0;
        new_frametime_ms = 51;
    } else if (inference_ms < 67) {
        new_framerate = 15.0;
        new_frametime_ms = 67;
    } else if (inference_ms < 101) {
        new_framerate = 10.0;
        new_frametime_ms = 101;
    } else if (inference_ms < 201) {
        new_framerate = 5.0;
        new_frametime_ms = 201;
    }

    // Don't exceed configured target framerate
    if (new_framerate > capture->config.framerate) {
        new_framerate = capture->config.framerate;
        new_frametime_ms = (unsigned int)(1000.0 / capture->config.framerate);
    }

    capture->frame_info.framerate = new_framerate;
    capture->current_frametime_ms = new_frametime_ms;
}

/**
 * Choose best stream resolution from VDO channel capabilities
 */
static bool choose_stream_resolution(VdoChannel* channel,
                                     unsigned int requested_width,
                                     unsigned int requested_height,
                                     VdoFormat format,
                                     unsigned int* chosen_width,
                                     unsigned int* chosen_height) {
    GError* error = NULL;
    VdoResolutionSet* resolutions = NULL;
    VdoMap* filter = vdo_map_new();

    if (!filter) {
        syslog(LOG_ERR, "[VDO] Failed to create resolution filter map");
        return false;
    }

    // Filter by format
    vdo_map_set_uint32(filter, "format", format);
    vdo_map_set_string(filter, "select", "all");

    resolutions = vdo_channel_get_resolutions(channel, filter, &error);
    g_object_unref(filter);

    if (!resolutions || resolutions->count == 0) {
        syslog(LOG_ERR, "[VDO] No resolutions available for format %u", format);
        if (error) {
            syslog(LOG_ERR, "[VDO] Error: %s", error->message);
            g_error_free(error);
        }
        if (resolutions) g_free(resolutions);
        return false;
    }

    // Find smallest resolution that fits requested size
    ssize_t best_idx = -1;
    unsigned int best_area = UINT32_MAX;

    for (size_t i = 0; i < resolutions->count; i++) {
        VdoResolution* res = &resolutions->resolutions[i];
        if (res->width >= requested_width && res->height >= requested_height) {
            unsigned int area = res->width * res->height;
            if (area < best_area) {
                best_idx = i;
                best_area = area;
            }
        }
    }

    // If no match found, use requested size (VDO will choose closest)
    if (best_idx >= 0) {
        *chosen_width = resolutions->resolutions[best_idx].width;
        *chosen_height = resolutions->resolutions[best_idx].height;
        syslog(LOG_INFO, "[VDO] Selected resolution: %ux%u", *chosen_width, *chosen_height);
    } else {
        *chosen_width = requested_width;
        *chosen_height = requested_height;
        syslog(LOG_WARNING, "[VDO] No exact match, using requested: %ux%u",
               requested_width, requested_height);
    }

    g_free(resolutions);
    return true;
}

// ============================================================================
// Public API Implementation
// ============================================================================

VdoCapture* vdo_capture_init(const VdoCaptureConfig* config) {
    if (!config) {
        syslog(LOG_ERR, "[VDO] NULL configuration provided");
        return NULL;
    }

    syslog(LOG_INFO, "[VDO] Initializing video capture...");
    syslog(LOG_INFO, "[VDO]   Channel: %u", config->channel);
    syslog(LOG_INFO, "[VDO]   Resolution: %ux%u", config->width, config->height);
    syslog(LOG_INFO, "[VDO]   Framerate: %.1f fps", config->framerate);
    syslog(LOG_INFO, "[VDO]   Format: %u (YUV=%d)", config->format, VDO_FORMAT_YUV);

    VdoCapture* capture = (VdoCapture*)calloc(1, sizeof(VdoCapture));
    if (!capture) {
        syslog(LOG_ERR, "[VDO] Failed to allocate VdoCapture: %s", strerror(errno));
        return NULL;
    }

    // Copy configuration
    capture->config = *config;

    // Clamp buffer count
    if (capture->config.buffer_count == 0) {
        capture->config.buffer_count = DEFAULT_BUFFER_COUNT;
    } else if (capture->config.buffer_count > MAX_VDO_BUFFERS) {
        capture->config.buffer_count = MAX_VDO_BUFFERS;
    }

    // Initialize state
    capture->running = false;
    capture->fd = -1;
    capture->current_frametime_ms = (unsigned int)(1000.0 / config->framerate);
    capture->frames_captured = 0;
    capture->frames_dropped = 0;
    pthread_mutex_init(&capture->mutex, NULL);

    // Get VDO channel
    GError* error = NULL;
    VdoMap* channel_desc = vdo_map_new();
    if (!channel_desc) {
        syslog(LOG_ERR, "[VDO] Failed to create channel descriptor");
        free(capture);
        return NULL;
    }

    vdo_map_set_uint32(channel_desc, "input", config->channel);
    capture->channel = vdo_channel_get_ex(channel_desc, &error);
    g_object_unref(channel_desc);

    if (!capture->channel) {
        syslog(LOG_ERR, "[VDO] Failed to get channel %u: %s",
               config->channel, error ? error->message : "unknown");
        if (error) g_error_free(error);
        pthread_mutex_destroy(&capture->mutex);
        free(capture);
        return NULL;
    }

    // Choose best resolution
    unsigned int chosen_width = config->width;
    unsigned int chosen_height = config->height;

    if (!choose_stream_resolution(capture->channel,
                                  config->width,
                                  config->height,
                                  config->format,
                                  &chosen_width,
                                  &chosen_height)) {
        syslog(LOG_WARNING, "[VDO] Resolution selection failed, using requested size");
    }

    // Create VDO stream settings
    VdoMap* stream_settings = vdo_map_new();
    if (!stream_settings) {
        syslog(LOG_ERR, "[VDO] Failed to create stream settings");
        g_object_unref(capture->channel);
        pthread_mutex_destroy(&capture->mutex);
        free(capture);
        return NULL;
    }

    vdo_map_set_uint32(stream_settings, "input", config->channel);
    vdo_map_set_uint32(stream_settings, "format", config->format);
    vdo_map_set_uint32(stream_settings, "width", chosen_width);
    vdo_map_set_uint32(stream_settings, "height", chosen_height);
    vdo_map_set_double(stream_settings, "framerate", config->framerate);
    vdo_map_set_uint32(stream_settings, "buffer.count", capture->config.buffer_count);
    vdo_map_set_uint32(stream_settings, "buffer.strategy", VDO_BUFFER_STRATEGY_INFINITE);

    if (config->dynamic_framerate) {
        vdo_map_set_boolean(stream_settings, "dynamic.framerate", TRUE);
    }

    // Create VDO stream
    capture->stream = vdo_stream_new_ex(stream_settings, &error);
    g_object_unref(stream_settings);

    if (!capture->stream) {
        syslog(LOG_ERR, "[VDO] Failed to create stream: %s",
               error ? error->message : "unknown");
        if (error) g_error_free(error);
        g_object_unref(capture->channel);
        pthread_mutex_destroy(&capture->mutex);
        free(capture);
        return NULL;
    }

    // Update frame info with actual stream properties
    capture->frame_info.width = chosen_width;
    capture->frame_info.height = chosen_height;
    capture->frame_info.format = config->format;
    capture->frame_info.framerate = config->framerate;
    capture->frame_info.rotation = 0;

    // Get stream info map for pitch
    VdoMap* stream_info = vdo_stream_get_info(capture->stream);
    if (stream_info) {
        vdo_map_get_uint32(stream_info, "pitch", &capture->frame_info.pitch);
        g_object_unref(stream_info);
    } else {
        // Estimate pitch (YUV format assumption)
        capture->frame_info.pitch = chosen_width;
    }

    syslog(LOG_INFO, "[VDO] ✓ Video capture initialized");
    syslog(LOG_INFO, "[VDO]   Actual resolution: %ux%u",
           capture->frame_info.width, capture->frame_info.height);
    syslog(LOG_INFO, "[VDO]   Pitch: %u bytes", capture->frame_info.pitch);
    syslog(LOG_INFO, "[VDO]   Buffer count: %u", capture->config.buffer_count);

    return capture;
}

bool vdo_capture_start(VdoCapture* capture) {
    if (!capture || capture->running) {
        return false;
    }

    syslog(LOG_INFO, "[VDO] Starting video stream...");

    GError* error = NULL;
    if (!vdo_stream_start(capture->stream, &error)) {
        syslog(LOG_ERR, "[VDO] Failed to start stream: %s",
               error ? error->message : "unknown");
        if (error) g_error_free(error);
        return false;
    }

    // Get file descriptor for polling
    capture->fd = vdo_stream_get_fd(capture->stream);
    if (capture->fd < 0) {
        syslog(LOG_ERR, "[VDO] Failed to get stream file descriptor");
        vdo_stream_stop(capture->stream);
        return false;
    }

    capture->running = true;

    syslog(LOG_INFO, "[VDO] ✓ Video stream started (fd=%d)", capture->fd);
    return true;
}

void vdo_capture_stop(VdoCapture* capture) {
    if (!capture || !capture->running) {
        return;
    }

    syslog(LOG_INFO, "[VDO] Stopping video stream...");

    GError* error = NULL;
    if (!vdo_stream_stop(capture->stream)) {
        syslog(LOG_WARNING, "[VDO] Error stopping stream");
    }

    capture->running = false;
    capture->fd = -1;

    syslog(LOG_INFO, "[VDO] ✓ Video stream stopped");
}

VdoBuffer* vdo_capture_get_frame(VdoCapture* capture, GError** error) {
    if (!capture || !capture->running) {
        if (error) {
            *error = g_error_new_literal(g_quark_from_static_string("vdo-capture"),
                                        1, "Capture not running");
        }
        return NULL;
    }

    VdoBuffer* buffer = vdo_stream_get_buffer(capture->stream, error);

    if (buffer) {
        pthread_mutex_lock(&capture->mutex);
        capture->frames_captured++;
        capture->frame_info.timestamp_ms = get_time_ms();
        pthread_mutex_unlock(&capture->mutex);
    }

    return buffer;
}

bool vdo_capture_release_frame(VdoCapture* capture, VdoBuffer* buffer) {
    if (!capture || !buffer) {
        return false;
    }

    GError* error = NULL;
    bool result = vdo_stream_buffer_unref(capture->stream, &buffer, &error);

    if (!result) {
        if (vdo_error_is_expected(&error)) {
            // Expected error (e.g., rotation change) - not critical
            g_clear_error(&error);
        } else {
            syslog(LOG_ERR, "[VDO] Unexpected error releasing buffer: %s",
                   error ? error->message : "unknown");
            if (error) g_error_free(error);
        }
    }

    return result;
}

void vdo_capture_flush_frames(VdoCapture* capture) {
    if (!capture || !capture->running) {
        return;
    }

    syslog(LOG_INFO, "[VDO] Flushing buffered frames...");

    GError* error = NULL;
    VdoBuffer* buffer;

    // Read and discard all buffered frames
    while ((buffer = vdo_stream_get_buffer(capture->stream, &error)) != NULL) {
        vdo_stream_buffer_unref(capture->stream, &buffer, NULL);

        pthread_mutex_lock(&capture->mutex);
        capture->frames_dropped++;
        pthread_mutex_unlock(&capture->mutex);
    }

    if (error) {
        if (!vdo_error_is_expected(&error)) {
            syslog(LOG_WARNING, "[VDO] Error during flush: %s", error->message);
        }
        g_error_free(error);
    }
}

bool vdo_capture_get_frame_info(VdoCapture* capture, VdoFrameInfo* info) {
    if (!capture || !info) {
        return false;
    }

    pthread_mutex_lock(&capture->mutex);
    *info = capture->frame_info;
    pthread_mutex_unlock(&capture->mutex);

    return true;
}

bool vdo_capture_update_framerate(VdoCapture* capture, unsigned int inference_time_ms) {
    if (!capture || !capture->config.dynamic_framerate || !capture->running) {
        return false;
    }

    pthread_mutex_lock(&capture->mutex);

    // Update running average
    capture->total_inference_time_ms += inference_time_ms;
    capture->inference_frame_count++;

    unsigned int avg_inference_ms = capture->total_inference_time_ms /
                                   capture->inference_frame_count;

    // Calculate new framerate
    unsigned int old_frametime = capture->current_frametime_ms;
    calculate_framerate(capture, avg_inference_ms);

    pthread_mutex_unlock(&capture->mutex);

    // Apply framerate change if needed
    if (old_frametime != capture->current_frametime_ms) {
        GError* error = NULL;
        if (!vdo_stream_set_framerate(capture->stream,
                                      capture->frame_info.framerate,
                                      &error)) {
            syslog(LOG_ERR, "[VDO] Failed to change framerate: %s",
                   error ? error->message : "unknown");
            if (error) g_error_free(error);
            return false;
        }

        syslog(LOG_INFO, "[VDO] Framerate adjusted to %.1f fps (inference avg: %u ms)",
               capture->frame_info.framerate, avg_inference_ms);

        // Flush stale frames
        vdo_capture_flush_frames(capture);

        return true;
    }

    return false;
}

int vdo_capture_get_fd(VdoCapture* capture) {
    return capture ? capture->fd : -1;
}

VdoStream* vdo_capture_get_stream(VdoCapture* capture) {
    return capture ? capture->stream : NULL;
}

void vdo_capture_destroy(VdoCapture* capture) {
    if (!capture) {
        return;
    }

    syslog(LOG_INFO, "[VDO] Destroying video capture...");

    // Stop if running
    if (capture->running) {
        vdo_capture_stop(capture);
    }

    // Release VDO objects
    if (capture->stream) {
        g_object_unref(capture->stream);
    }

    if (capture->channel) {
        g_object_unref(capture->channel);
    }

    // Cleanup
    pthread_mutex_destroy(&capture->mutex);
    free(capture);

    syslog(LOG_INFO, "[VDO] ✓ Video capture destroyed");
}

void vdo_capture_get_stats(VdoCapture* capture,
                           uint64_t* frames_captured,
                           uint64_t* frames_dropped,
                           double* avg_framerate) {
    if (!capture) {
        return;
    }

    pthread_mutex_lock(&capture->mutex);

    if (frames_captured) {
        *frames_captured = capture->frames_captured;
    }

    if (frames_dropped) {
        *frames_dropped = capture->frames_dropped;
    }

    if (avg_framerate) {
        *avg_framerate = capture->frame_info.framerate;
    }

    pthread_mutex_unlock(&capture->mutex);
}
