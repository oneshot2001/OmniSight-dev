/**
 * @file perception.c
 * @brief Main perception engine implementation for OMNISIGHT
 *
 * Orchestrates VDO capture, ML inference, tracking, and behavior analysis
 */

#include "perception.h"
#include "vdo_capture.h"
#include "larod_inference.h"
#include "tracker.h"
#include "behavior.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <syslog.h>
#include <glib.h>

struct PerceptionEngine {
    PerceptionConfig config;
    VdoCapture* vdo;
    LarodInference* larod;
    Tracker* tracker;
    BehaviorAnalyzer* behavior;

    PerceptionCallback callback;
    void* callback_user_data;

    bool running;
    pthread_t capture_thread;
    pthread_mutex_t mutex;

    // Statistics
    uint32_t frames_processed;
    uint32_t frames_dropped;
    float avg_inference_ms;
    float avg_fps;
};

// Forward declarations
static void* capture_thread_func(void* arg);
static void process_frame(PerceptionEngine* engine, VdoBuffer* buffer);

// ============================================================================
// Public API Implementation
// ============================================================================

PerceptionEngine* perception_init(const PerceptionConfig* config) {
    if (!config) {
        syslog(LOG_ERR, "[Perception] NULL config provided");
        return NULL;
    }

    PerceptionEngine* engine = (PerceptionEngine*)calloc(1, sizeof(PerceptionEngine));
    if (!engine) {
        syslog(LOG_ERR, "[Perception] Failed to allocate engine memory");
        return NULL;
    }

    engine->config = *config;
    engine->running = false;
    engine->frames_processed = 0;
    engine->frames_dropped = 0;
    engine->avg_inference_ms = 0.0f;
    engine->avg_fps = 0.0f;

    pthread_mutex_init(&engine->mutex, NULL);

    // Initialize VDO capture
    VdoCaptureConfig vdo_config = {
        .channel = 1,  // Primary sensor
        .width = config->frame_width,
        .height = config->frame_height,
        .framerate = config->target_fps,
        .format = VDO_FORMAT_YUV,  // YUV recommended for Larod
        .buffer_count = config->buffer_pool_size,
        .dynamic_framerate = true
    };

    engine->vdo = vdo_capture_init(&vdo_config);
    if (!engine->vdo) {
        syslog(LOG_WARNING, "[Perception] VDO capture initialization failed (placeholder mode)");
        printf("[Perception] Warning: VDO capture initialization failed (placeholder mode)\n");
    }

    // Initialize Larod inference
    LarodInferenceConfig larod_config = {
        .model_path = config->model_path,
        .device_name = config->use_dlpu ? "dlpu" : "cpu",
        .width = config->frame_width,
        .height = config->frame_height,
        .input_format = VDO_FORMAT_YUV,
        .confidence_threshold = config->detection_threshold,
        .max_detections = config->max_tracked_objects
    };

    engine->larod = larod_inference_init(&larod_config);
    if (!engine->larod) {
        syslog(LOG_ERR, "[Perception] Larod inference initialization failed");
        printf("[Perception] Error: Larod inference initialization failed\n");
        perception_destroy(engine);
        return NULL;
    }

    // Initialize tracker
    TrackerConfig tracker_config = {
        .iou_threshold = config->tracking_threshold,
        .max_age = 30,
        .min_hits = 3,
        .max_tracks = config->max_tracked_objects,
        .use_kalman_filter = true,
        .feature_similarity_weight = 0.3f
    };

    engine->tracker = tracker_init(&tracker_config);
    if (!engine->tracker) {
        syslog(LOG_ERR, "[Perception] Tracker initialization failed");
        printf("[Perception] Error: Tracker initialization failed\n");
        perception_destroy(engine);
        return NULL;
    }

    // Initialize behavior analyzer
    BehaviorConfig behavior_config = {
        .loitering_threshold_ms = config->loitering_threshold_ms,
        .loitering_movement_threshold = 0.05f,
        .running_velocity_threshold = config->running_velocity_threshold,
        .running_frames_threshold = 5,
        .repeated_passes_count = 3,
        .repeated_passes_window_ms = 60000,
        .observation_threshold_ms = 10000,
        .observation_gaze_threshold = 0.8f,
        .loitering_weight = 0.3f,
        .running_weight = 0.5f,
        .concealing_weight = 0.8f,
        .repeated_passes_weight = 0.4f,
        .observation_weight = 0.6f
    };

    engine->behavior = behavior_init(&behavior_config);
    if (!engine->behavior) {
        syslog(LOG_ERR, "[Perception] Behavior analyzer initialization failed");
        printf("[Perception] Error: Behavior analyzer initialization failed\n");
        perception_destroy(engine);
        return NULL;
    }

    printf("[Perception] Engine initialized successfully\n");
    printf("[Perception] Frame size: %ux%u @ %u FPS\n",
           config->frame_width, config->frame_height, config->target_fps);
    printf("[Perception] Model: %s\n", config->model_path);
    printf("[Perception] Using %s for inference\n",
           config->use_dlpu ? "DLPU" : "CPU");

    return engine;
}

bool perception_start(
    PerceptionEngine* engine,
    PerceptionCallback callback,
    void* user_data
) {
    if (!engine || !callback) {
        return false;
    }

    pthread_mutex_lock(&engine->mutex);

    if (engine->running) {
        pthread_mutex_unlock(&engine->mutex);
        return false;  // Already running
    }

    engine->callback = callback;
    engine->callback_user_data = user_data;
    engine->running = true;

    pthread_mutex_unlock(&engine->mutex);

    // Start VDO capture
    if (engine->vdo) {
        if (!vdo_capture_start(engine->vdo)) {
            syslog(LOG_ERR, "[Perception] VDO capture start failed");
            printf("[Perception] Error: VDO capture start failed\n");
            pthread_mutex_lock(&engine->mutex);
            engine->running = false;
            pthread_mutex_unlock(&engine->mutex);
            return false;
        }
        syslog(LOG_INFO, "[Perception] VDO capture started");
        printf("[Perception] VDO capture started\n");
    } else {
        syslog(LOG_WARNING, "[Perception] No VDO capture available (placeholder mode)");
        printf("[Perception] Warning: No VDO capture available (placeholder mode)\n");
    }

    // Start capture thread for frame polling
    if (pthread_create(&engine->capture_thread, NULL, capture_thread_func, engine) != 0) {
        syslog(LOG_ERR, "[Perception] Failed to create capture thread");
        printf("[Perception] Error: Failed to create capture thread\n");
        if (engine->vdo) {
            vdo_capture_stop(engine->vdo);
        }
        pthread_mutex_lock(&engine->mutex);
        engine->running = false;
        pthread_mutex_unlock(&engine->mutex);
        return false;
    }

    printf("[Perception] Engine started\n");
    return true;
}

void perception_stop(PerceptionEngine* engine) {
    if (!engine) {
        return;
    }

    pthread_mutex_lock(&engine->mutex);

    if (!engine->running) {
        pthread_mutex_unlock(&engine->mutex);
        return;
    }

    engine->running = false;

    pthread_mutex_unlock(&engine->mutex);

    // Wait for capture thread to finish
    pthread_join(engine->capture_thread, NULL);

    // Stop VDO capture
    if (engine->vdo) {
        vdo_capture_stop(engine->vdo);
    }

    printf("[Perception] Engine stopped\n");
}

void perception_destroy(PerceptionEngine* engine) {
    if (!engine) {
        return;
    }

    perception_stop(engine);

    if (engine->vdo) {
        vdo_capture_destroy(engine->vdo);
    }

    if (engine->larod) {
        larod_inference_destroy(engine->larod);
    }

    if (engine->tracker) {
        tracker_destroy(engine->tracker);
    }

    if (engine->behavior) {
        behavior_destroy(engine->behavior);
    }

    pthread_mutex_destroy(&engine->mutex);

    free(engine);

    printf("[Perception] Engine destroyed\n");
}

uint32_t perception_process_frame(
    PerceptionEngine* engine,
    const uint8_t* frame_data,
    uint32_t width,
    uint32_t height,
    DetectedObject* objects,
    uint32_t max_objects
) {
    if (!engine || !frame_data || !objects) {
        return 0;
    }

    // TODO: Implement frame processing
    // 1. Preprocess image for model
    // 2. Run inference
    // 3. Post-process detections
    // 4. Return detected objects

    (void)width;
    (void)height;
    (void)max_objects;

    printf("[Perception] Single frame processing not yet implemented\n");
    return 0;
}

uint32_t perception_get_tracked_objects(
    PerceptionEngine* engine,
    TrackedObject* objects,
    uint32_t max_objects
) {
    if (!engine || !objects) {
        return 0;
    }

    pthread_mutex_lock(&engine->mutex);
    uint32_t count = tracker_get_tracks(engine->tracker, objects, max_objects);
    pthread_mutex_unlock(&engine->mutex);

    return count;
}

void perception_update_behavior_params(
    PerceptionEngine* engine,
    uint32_t loitering_ms,
    float running_threshold
) {
    if (!engine) {
        return;
    }

    pthread_mutex_lock(&engine->mutex);

    engine->config.loitering_threshold_ms = loitering_ms;
    engine->config.running_velocity_threshold = running_threshold;

    // Update behavior analyzer config
    BehaviorConfig config;
    config.loitering_threshold_ms = loitering_ms;
    config.running_velocity_threshold = running_threshold;
    behavior_update_config(engine->behavior, &config);

    pthread_mutex_unlock(&engine->mutex);
}

void perception_get_stats(
    PerceptionEngine* engine,
    float* avg_inference_ms,
    float* avg_fps,
    uint32_t* dropped_frames
) {
    if (!engine) {
        return;
    }

    pthread_mutex_lock(&engine->mutex);

    if (avg_inference_ms) *avg_inference_ms = engine->avg_inference_ms;
    if (avg_fps) *avg_fps = engine->avg_fps;
    if (dropped_frames) *dropped_frames = engine->frames_dropped;

    pthread_mutex_unlock(&engine->mutex);
}

// ============================================================================
// Internal Functions
// ============================================================================

/**
 * Capture thread function - polls VDO for frames and processes them
 */
static void* capture_thread_func(void* arg) {
    PerceptionEngine* engine = (PerceptionEngine*)arg;
    GError* error = NULL;

    printf("[Perception] Capture thread started\n");

    while (engine->running) {
        // Get next frame from VDO (blocking)
        VdoBuffer* buffer = NULL;

        if (engine->vdo) {
            buffer = vdo_capture_get_frame(engine->vdo, &error);

            if (!buffer) {
                if (error) {
                    syslog(LOG_WARNING, "[Perception] Error getting frame: %s", error->message);
                    printf("[Perception] Error getting frame: %s\n", error->message);
                    g_error_free(error);
                    error = NULL;
                }

                // Check if we should continue
                if (!engine->running) {
                    break;
                }

                // Brief sleep before retry to avoid busy-wait
                usleep(10000);  // 10ms
                engine->frames_dropped++;
                continue;
            }
        } else {
            // Placeholder mode - no real VDO
            usleep(100000);  // 100ms (10 fps)
            continue;
        }

        // Process the frame
        pthread_mutex_lock(&engine->mutex);
        if (engine->running) {
            process_frame(engine, buffer);
        }
        pthread_mutex_unlock(&engine->mutex);

        // Release frame back to VDO
        if (!vdo_capture_release_frame(engine->vdo, buffer)) {
            syslog(LOG_WARNING, "[Perception] Failed to release frame buffer");
            printf("[Perception] Warning: Failed to release frame buffer\n");
        }
    }

    printf("[Perception] Capture thread stopped\n");
    return NULL;
}

/**
 * Process a single frame through the perception pipeline
 */
static void process_frame(PerceptionEngine* engine, VdoBuffer* buffer) {
    if (!engine || !buffer) {
        return;
    }

    // Step 1: Run Larod inference on VdoBuffer
    DetectedObject detections[50];
    uint32_t num_detections = 0;

    bool inference_success = larod_inference_run(
        engine->larod,
        buffer,
        detections,
        50,
        &num_detections
    );

    if (!inference_success) {
        syslog(LOG_WARNING, "[Perception] Inference failed on frame");
        printf("[Perception] Warning: Inference failed on frame\n");
        engine->frames_dropped++;
        return;
    }

    // Step 2: Update tracker with detections
    TrackedObject tracks[50];
    uint32_t num_tracks = tracker_update(
        engine->tracker,
        detections,
        num_detections,
        tracks,
        50
    );

    // Step 3: Analyze behaviors on tracked objects
    behavior_analyze(engine->behavior, tracks, num_tracks);

    // Step 4: Call user callback with tracked objects
    if (engine->callback && num_tracks > 0) {
        engine->callback(tracks, num_tracks, engine->callback_user_data);
    }

    // Step 5: Update statistics
    engine->frames_processed++;

    // Get inference performance stats
    float avg_inference_ms = 0.0f;
    larod_inference_get_stats(engine->larod, &avg_inference_ms, NULL, NULL, NULL);

    // Update rolling average
    if (engine->frames_processed > 0) {
        float alpha = 0.1f;  // Exponential moving average weight
        engine->avg_inference_ms = (alpha * avg_inference_ms) +
                                   ((1.0f - alpha) * engine->avg_inference_ms);
    } else {
        engine->avg_inference_ms = avg_inference_ms;
    }

    // Step 6: Adaptive framerate adjustment
    if (engine->vdo && avg_inference_ms > 0.0f) {
        if (vdo_capture_update_framerate(engine->vdo, (unsigned int)avg_inference_ms)) {
            VdoFrameInfo frame_info;
            if (vdo_capture_get_frame_info(engine->vdo, &frame_info)) {
                engine->avg_fps = (float)frame_info.framerate;
                printf("[Perception] Framerate adjusted to %.1f fps (inference: %.1f ms)\n",
                       engine->avg_fps, avg_inference_ms);
            }
        }
    }
}
