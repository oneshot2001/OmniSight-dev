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

struct PerceptionEngine {
    PerceptionConfig config;
    VdoCapture* vdo;
    LarodInference* larod;
    Tracker* tracker;
    BehaviorAnalyzer* behavior;

    PerceptionCallback callback;
    void* callback_user_data;

    bool running;
    pthread_mutex_t mutex;

    // Statistics
    uint32_t frames_processed;
    uint32_t frames_dropped;
    float avg_inference_ms;
    float avg_fps;
};

// Forward declarations
static void frame_callback(const VdoFrame* frame, void* user_data);
static void process_frame(PerceptionEngine* engine, const VdoFrame* frame);

// ============================================================================
// Public API Implementation
// ============================================================================

PerceptionEngine* perception_init(const PerceptionConfig* config) {
    if (!config) {
        return NULL;
    }

    PerceptionEngine* engine = (PerceptionEngine*)calloc(1, sizeof(PerceptionEngine));
    if (!engine) {
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
        .width = config->frame_width,
        .height = config->frame_height,
        .framerate = config->target_fps,
        .format = VDO_FORMAT_RGB,
        .buffer_count = config->buffer_pool_size,
        .use_hardware_accel = true,
        .channel = 0
    };

    engine->vdo = vdo_capture_init(&vdo_config);
    if (!engine->vdo) {
        printf("[Perception] Warning: VDO capture initialization failed (placeholder mode)\n");
    }

    // Initialize Larod inference
    LarodInferenceConfig larod_config = {
        .model_path = config->model_path,
        .chip = config->use_dlpu ? LAROD_CHIP_DLPU : LAROD_CHIP_CPU,
        .num_threads = config->inference_threads,
        .async_mode = config->async_inference,
        .timeout_ms = 1000
    };

    engine->larod = larod_inference_init(&larod_config);
    if (!engine->larod) {
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
        if (!vdo_capture_start(engine->vdo, frame_callback, engine)) {
            printf("[Perception] Warning: VDO capture start failed (placeholder mode)\n");
        }
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

static void frame_callback(const VdoFrame* frame, void* user_data) {
    PerceptionEngine* engine = (PerceptionEngine*)user_data;

    if (!engine || !frame) {
        return;
    }

    pthread_mutex_lock(&engine->mutex);

    if (engine->running) {
        process_frame(engine, frame);
    }

    pthread_mutex_unlock(&engine->mutex);
}

static void process_frame(PerceptionEngine* engine, const VdoFrame* frame) {
    // TODO: Full implementation
    // This placeholder shows the processing pipeline

    (void)frame;  // Suppress unused warning

    // Step 1: Run ML inference
    // const void* inputs[] = {frame->data};
    // void* outputs[3];  // boxes, scores, classes
    // larod_inference_run(engine->larod, inputs, 1, outputs, 3);

    // Step 2: Parse detections
    DetectedObject detections[50];
    uint32_t num_detections = 0;  // Would be populated from inference results

    // Step 3: Update tracker
    TrackedObject tracks[50];
    uint32_t num_tracks = tracker_update(
        engine->tracker,
        detections,
        num_detections,
        tracks,
        50
    );

    // Step 4: Analyze behaviors
    behavior_analyze(engine->behavior, tracks, num_tracks);

    // Step 5: Call user callback
    if (engine->callback && num_tracks > 0) {
        engine->callback(tracks, num_tracks, engine->callback_user_data);
    }

    // Update statistics
    engine->frames_processed++;
}
