/**
 * @file vdo_capture.h
 * @brief OMNISIGHT VDO Video Capture Module
 *
 * Handles video stream capture from Axis camera using VDO API.
 * Provides YUV frames for Larod ML inference.
 *
 * Copyright (C) 2025 OMNISIGHT
 * Based on Axis ACAP Native SDK examples
 */

#ifndef OMNISIGHT_VDO_CAPTURE_H
#define OMNISIGHT_VDO_CAPTURE_H

#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>

#include <glib.h>
#include <vdo-stream.h>
#include <vdo-types.h>
#include <vdo-frame.h>
#include <vdo-buffer.h>
#include <vdo-error.h>

#ifdef __cplusplus
extern "C" {
#endif

// Forward declaration
typedef struct VdoCapture VdoCapture;

/**
 * VDO capture configuration
 */
typedef struct {
    unsigned int channel;        // Input channel (1 = primary sensor)
    unsigned int width;          // Requested frame width
    unsigned int height;         // Requested frame height
    double framerate;            // Target framerate (fps)
    VdoFormat format;            // Video format (VDO_FORMAT_YUV recommended)
    unsigned int buffer_count;   // Number of buffers (2-5)
    bool dynamic_framerate;      // Allow framerate adjustment
} VdoCaptureConfig;

/**
 * Frame metadata from VDO
 */
typedef struct {
    unsigned int width;
    unsigned int height;
    unsigned int pitch;          // Bytes per row
    VdoFormat format;
    double framerate;
    unsigned int rotation;       // Camera rotation (0, 90, 180, 270)
    uint64_t timestamp_ms;
} VdoFrameInfo;

/**
 * Initialize VDO capture with specified configuration
 *
 * @param config VDO capture configuration
 * @return VdoCapture instance or NULL on failure
 */
VdoCapture* vdo_capture_init(const VdoCaptureConfig* config);

/**
 * Start VDO stream capture
 *
 * Begins streaming video frames from the camera sensor.
 *
 * @param capture VdoCapture instance
 * @return true on success, false on failure
 */
bool vdo_capture_start(VdoCapture* capture);

/**
 * Stop VDO stream capture
 *
 * @param capture VdoCapture instance
 */
void vdo_capture_stop(VdoCapture* capture);

/**
 * Get next frame from VDO stream (blocking)
 *
 * Blocks until a frame is available or error occurs.
 * Caller must release frame with vdo_capture_release_frame().
 *
 * @param capture VdoCapture instance
 * @param error Optional GError pointer for error details
 * @return VdoBuffer containing frame data, NULL on error
 */
VdoBuffer* vdo_capture_get_frame(VdoCapture* capture, GError** error);

/**
 * Release frame buffer back to VDO
 *
 * Must be called after processing each frame from vdo_capture_get_frame().
 *
 * @param capture VdoCapture instance
 * @param buffer VdoBuffer to release
 * @return true on success, false on failure
 */
bool vdo_capture_release_frame(VdoCapture* capture, VdoBuffer* buffer);

/**
 * Flush all buffered frames
 *
 * Discards all frames in the VDO buffer queue.
 * Useful after framerate changes or long processing delays.
 *
 * @param capture VdoCapture instance
 */
void vdo_capture_flush_frames(VdoCapture* capture);

/**
 * Get current frame metadata
 *
 * @param capture VdoCapture instance
 * @param info Output structure for frame metadata
 * @return true on success, false on failure
 */
bool vdo_capture_get_frame_info(VdoCapture* capture, VdoFrameInfo* info);

/**
 * Update framerate dynamically
 *
 * Adjusts stream framerate based on processing time.
 * Only works if dynamic_framerate was enabled in config.
 *
 * @param capture VdoCapture instance
 * @param inference_time_ms Average inference time in milliseconds
 * @return true if framerate was changed, false otherwise
 */
bool vdo_capture_update_framerate(VdoCapture* capture, unsigned int inference_time_ms);

/**
 * Get file descriptor for polling
 *
 * Can be used with poll() or select() for asynchronous frame availability.
 *
 * @param capture VdoCapture instance
 * @return File descriptor or -1 on error
 */
int vdo_capture_get_fd(VdoCapture* capture);

/**
 * Get VDO stream object (for advanced usage)
 *
 * @param capture VdoCapture instance
 * @return VdoStream pointer or NULL
 */
VdoStream* vdo_capture_get_stream(VdoCapture* capture);

/**
 * Destroy VDO capture instance and free resources
 *
 * Automatically stops stream if running.
 *
 * @param capture VdoCapture instance
 */
void vdo_capture_destroy(VdoCapture* capture);

/**
 * Get statistics
 *
 * @param capture VdoCapture instance
 * @param frames_captured Total frames captured
 * @param frames_dropped Number of dropped frames
 * @param avg_framerate Average framerate
 */
void vdo_capture_get_stats(VdoCapture* capture,
                           uint64_t* frames_captured,
                           uint64_t* frames_dropped,
                           double* avg_framerate);

#ifdef __cplusplus
}
#endif

#endif // OMNISIGHT_VDO_CAPTURE_H
