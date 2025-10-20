/**
 * @file vdo_capture.h
 * @brief VDO (Video Device Object) capture module for OMNISIGHT
 *
 * Handles video stream capture from Axis camera using VDO API
 * Optimized for ARTPEC-8 hardware acceleration
 */

#ifndef OMNISIGHT_VDO_CAPTURE_H
#define OMNISIGHT_VDO_CAPTURE_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct VdoCapture VdoCapture;

/**
 * Video format specification
 */
typedef enum {
    VDO_FORMAT_YUV420,
    VDO_FORMAT_NV12,
    VDO_FORMAT_RGB,
    VDO_FORMAT_BGR,
} VdoFormat;

/**
 * Frame buffer structure
 */
typedef struct {
    uint8_t* data;           // Frame data
    uint32_t width;          // Frame width
    uint32_t height;         // Frame height
    uint32_t stride;         // Row stride in bytes
    VdoFormat format;        // Pixel format
    uint64_t timestamp_ms;   // Capture timestamp
    uint32_t sequence;       // Frame sequence number
    void* vdo_buffer;        // Internal VDO buffer handle
} VdoFrame;

/**
 * VDO capture configuration
 */
typedef struct {
    uint32_t width;          // Desired frame width (0 = use camera default)
    uint32_t height;         // Desired frame height (0 = use camera default)
    uint32_t framerate;      // Target framerate (0 = use camera default)
    VdoFormat format;        // Output pixel format
    uint32_t buffer_count;   // Number of buffers in pool
    bool use_hardware_accel; // Use hardware color conversion
    uint32_t channel;        // VDO channel number
} VdoCaptureConfig;

/**
 * Frame callback function
 */
typedef void (*VdoFrameCallback)(const VdoFrame* frame, void* user_data);

/**
 * Initialize VDO capture
 *
 * @param config Capture configuration
 * @return VDO capture instance, NULL on failure
 */
VdoCapture* vdo_capture_init(const VdoCaptureConfig* config);

/**
 * Start video capture
 *
 * @param capture VDO capture instance
 * @param callback Function called for each captured frame
 * @param user_data User data passed to callback
 * @return true on success, false on failure
 */
bool vdo_capture_start(
    VdoCapture* capture,
    VdoFrameCallback callback,
    void* user_data
);

/**
 * Stop video capture
 *
 * @param capture VDO capture instance
 */
void vdo_capture_stop(VdoCapture* capture);

/**
 * Release a frame buffer back to the pool
 *
 * Must be called after processing each frame
 *
 * @param capture VDO capture instance
 * @param frame Frame to release
 */
void vdo_capture_release_frame(VdoCapture* capture, const VdoFrame* frame);

/**
 * Get the next frame (synchronous/blocking mode)
 *
 * Alternative to callback-based capture
 *
 * @param capture VDO capture instance
 * @param timeout_ms Timeout in milliseconds (0 = infinite)
 * @return Frame pointer, NULL on timeout or error
 */
VdoFrame* vdo_capture_get_frame(VdoCapture* capture, uint32_t timeout_ms);

/**
 * Get capture statistics
 *
 * @param capture VDO capture instance
 * @param frames_captured Total frames captured
 * @param frames_dropped Number of dropped frames
 * @param current_fps Current capture FPS
 */
void vdo_capture_get_stats(
    VdoCapture* capture,
    uint64_t* frames_captured,
    uint64_t* frames_dropped,
    float* current_fps
);

/**
 * Destroy VDO capture and free resources
 *
 * @param capture VDO capture instance
 */
void vdo_capture_destroy(VdoCapture* capture);

#ifdef __cplusplus
}
#endif

#endif // OMNISIGHT_VDO_CAPTURE_H
