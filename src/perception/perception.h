/**
 * @file perception.h
 * @brief OMNISIGHT Perception Engine - Core computer vision and object detection
 *
 * The perception engine is responsible for:
 * - Video stream capture via VDO API
 * - Object detection using DLPU-accelerated models
 * - Object tracking across frames
 * - Feature extraction for Timeline Threading
 *
 * Performance targets:
 * - Inference: <20ms per frame
 * - Throughput: 10 fps
 * - Memory: <256MB
 */

#ifndef OMNISIGHT_PERCEPTION_H
#define OMNISIGHT_PERCEPTION_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// Forward declarations
typedef struct PerceptionEngine PerceptionEngine;
typedef struct DetectedObject DetectedObject;
typedef struct TrackedObject TrackedObject;

/**
 * Object classification types
 */
typedef enum {
    OBJECT_CLASS_UNKNOWN = 0,
    OBJECT_CLASS_PERSON,
    OBJECT_CLASS_VEHICLE,
    OBJECT_CLASS_FACE,
    OBJECT_CLASS_PACKAGE,
    OBJECT_CLASS_ANIMAL,
    OBJECT_CLASS_MAX
} ObjectClass;

/**
 * Behavioral flags for threat assessment
 */
typedef enum {
    BEHAVIOR_NORMAL = 0x00,
    BEHAVIOR_LOITERING = 0x01,
    BEHAVIOR_RUNNING = 0x02,
    BEHAVIOR_CONCEALING = 0x04,
    BEHAVIOR_SUSPICIOUS_MOVEMENT = 0x08,
    BEHAVIOR_REPEATED_PASSES = 0x10,
    BEHAVIOR_EXTENDED_OBSERVATION = 0x20,
} BehaviorFlags;

/**
 * Bounding box in normalized coordinates [0.0, 1.0]
 */
typedef struct {
    float x;      // Top-left X
    float y;      // Top-left Y
    float width;  // Width
    float height; // Height
} BoundingBox;

/**
 * Detected object in a single frame
 */
struct DetectedObject {
    uint32_t id;              // Unique detection ID
    ObjectClass class_id;     // Object classification
    float confidence;         // Detection confidence [0.0, 1.0]
    BoundingBox bbox;         // Bounding box
    float features[128];      // Feature vector for tracking/timeline
    uint64_t timestamp_ms;    // Detection timestamp
};

/**
 * Tracked object across multiple frames
 */
struct TrackedObject {
    uint32_t track_id;           // Unique track ID
    ObjectClass class_id;        // Object classification
    BoundingBox current_bbox;    // Current position
    BoundingBox predicted_bbox;  // Predicted next position
    float velocity_x;            // X velocity (pixels/frame)
    float velocity_y;            // Y velocity (pixels/frame)
    float confidence;            // Track confidence
    uint32_t frame_count;        // Frames since first detection
    uint32_t miss_count;         // Consecutive missed detections
    BehaviorFlags behaviors;     // Detected behaviors
    float threat_score;          // Calculated threat level [0.0, 1.0]
    uint64_t first_seen_ms;      // First detection timestamp
    uint64_t last_seen_ms;       // Last detection timestamp
    float features[128];         // Average feature vector
};

/**
 * Perception engine configuration
 */
typedef struct {
    // Video settings
    uint32_t frame_width;
    uint32_t frame_height;
    uint32_t target_fps;

    // Model settings
    const char* model_path;
    bool use_dlpu;
    uint32_t inference_threads;

    // Detection thresholds
    float detection_threshold;    // Minimum confidence
    float tracking_threshold;     // Minimum IoU for tracking
    uint32_t max_tracked_objects;

    // Behavior detection
    uint32_t loitering_threshold_ms;
    float running_velocity_threshold;

    // Performance
    bool async_inference;
    uint32_t buffer_pool_size;
} PerceptionConfig;

/**
 * Perception callback for detected/tracked objects
 */
typedef void (*PerceptionCallback)(
    const TrackedObject* objects,
    uint32_t object_count,
    void* user_data
);

/**
 * Initialize the perception engine
 *
 * @param config Engine configuration
 * @return Pointer to initialized engine, NULL on failure
 */
PerceptionEngine* perception_init(const PerceptionConfig* config);

/**
 * Start the perception engine
 *
 * Begins video capture and processing pipeline
 *
 * @param engine Perception engine instance
 * @param callback Function called for each processed frame
 * @param user_data User data passed to callback
 * @return true on success, false on failure
 */
bool perception_start(
    PerceptionEngine* engine,
    PerceptionCallback callback,
    void* user_data
);

/**
 * Stop the perception engine
 *
 * @param engine Perception engine instance
 */
void perception_stop(PerceptionEngine* engine);

/**
 * Destroy the perception engine and free resources
 *
 * @param engine Perception engine instance
 */
void perception_destroy(PerceptionEngine* engine);

/**
 * Process a single frame (for testing/manual control)
 *
 * @param engine Perception engine instance
 * @param frame_data Raw frame data (RGB)
 * @param width Frame width
 * @param height Frame height
 * @param objects Output array for detected objects
 * @param max_objects Maximum objects to return
 * @return Number of objects detected
 */
uint32_t perception_process_frame(
    PerceptionEngine* engine,
    const uint8_t* frame_data,
    uint32_t width,
    uint32_t height,
    DetectedObject* objects,
    uint32_t max_objects
);

/**
 * Get current tracked objects
 *
 * @param engine Perception engine instance
 * @param objects Output array for tracked objects
 * @param max_objects Maximum objects to return
 * @return Number of tracked objects
 */
uint32_t perception_get_tracked_objects(
    PerceptionEngine* engine,
    TrackedObject* objects,
    uint32_t max_objects
);

/**
 * Update behavior analysis parameters
 *
 * @param engine Perception engine instance
 * @param loitering_ms Milliseconds before flagging loitering
 * @param running_threshold Velocity threshold for running detection
 */
void perception_update_behavior_params(
    PerceptionEngine* engine,
    uint32_t loitering_ms,
    float running_threshold
);

/**
 * Get performance statistics
 *
 * @param engine Perception engine instance
 * @param avg_inference_ms Average inference time
 * @param avg_fps Average FPS
 * @param dropped_frames Number of dropped frames
 */
void perception_get_stats(
    PerceptionEngine* engine,
    float* avg_inference_ms,
    float* avg_fps,
    uint32_t* dropped_frames
);

#ifdef __cplusplus
}
#endif

#endif // OMNISIGHT_PERCEPTION_H
