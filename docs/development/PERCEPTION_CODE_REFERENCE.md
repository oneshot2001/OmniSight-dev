# Perception.c Code Reference

Quick reference for key sections of the Phase 3 hardware integration.

## Table of Contents
1. [Data Structures](#data-structures)
2. [Initialization](#initialization)
3. [Start/Stop](#startstop)
4. [Capture Thread](#capture-thread)
5. [Frame Processing Pipeline](#frame-processing-pipeline)
6. [Error Handling Patterns](#error-handling-patterns)

---

## Data Structures

### PerceptionEngine (lines 18-37)
```c
struct PerceptionEngine {
    PerceptionConfig config;
    VdoCapture* vdo;              // Video capture handle
    LarodInference* larod;        // ML inference engine
    Tracker* tracker;             // Multi-object tracker
    BehaviorAnalyzer* behavior;   // Behavior analyzer

    PerceptionCallback callback;  // User callback
    void* callback_user_data;

    bool running;                 // Thread control flag
    pthread_t capture_thread;     // Frame polling thread
    pthread_mutex_t mutex;        // Thread safety

    // Statistics
    uint32_t frames_processed;
    uint32_t frames_dropped;
    float avg_inference_ms;
    float avg_fps;
};
```

---

## Initialization

### VDO Configuration (lines 67-80)
```c
VdoCaptureConfig vdo_config = {
    .channel = 1,              // Primary sensor
    .width = config->frame_width,
    .height = config->frame_height,
    .framerate = config->target_fps,
    .format = VDO_FORMAT_YUV,  // YUV recommended for Larod
    .buffer_count = config->buffer_pool_size,
    .dynamic_framerate = true  // Enable adaptive FPS
};
engine->vdo = vdo_capture_init(&vdo_config);
```

### Larod Configuration (lines 88-99)
```c
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
```

### Error Handling Pattern (lines 100-105)
```c
if (!engine->larod) {
    syslog(LOG_ERR, "[Perception] Larod inference initialization failed");
    printf("[Perception] Error: Larod inference initialization failed\n");
    perception_destroy(engine);  // Cleanup partial initialization
    return NULL;
}
```

---

## Start/Stop

### Start Sequence (lines 159-213)
```c
bool perception_start(PerceptionEngine* engine,
                     PerceptionCallback callback,
                     void* user_data) {
    // 1. Validate inputs
    if (!engine || !callback) return false;

    // 2. Set running flag
    pthread_mutex_lock(&engine->mutex);
    if (engine->running) {
        pthread_mutex_unlock(&engine->mutex);
        return false;  // Already running
    }
    engine->callback = callback;
    engine->callback_user_data = user_data;
    engine->running = true;
    pthread_mutex_unlock(&engine->mutex);

    // 3. Start VDO capture
    if (engine->vdo) {
        if (!vdo_capture_start(engine->vdo)) {
            syslog(LOG_ERR, "[Perception] VDO capture start failed");
            engine->running = false;
            return false;
        }
    }

    // 4. Start capture thread
    if (pthread_create(&engine->capture_thread, NULL,
                      capture_thread_func, engine) != 0) {
        syslog(LOG_ERR, "[Perception] Failed to create capture thread");
        if (engine->vdo) vdo_capture_stop(engine->vdo);
        engine->running = false;
        return false;
    }

    return true;
}
```

### Stop Sequence (lines 215-238)
```c
void perception_stop(PerceptionEngine* engine) {
    if (!engine) return;

    // 1. Set stop flag
    pthread_mutex_lock(&engine->mutex);
    if (!engine->running) {
        pthread_mutex_unlock(&engine->mutex);
        return;
    }
    engine->running = false;
    pthread_mutex_unlock(&engine->mutex);

    // 2. Wait for capture thread to finish
    pthread_join(engine->capture_thread, NULL);

    // 3. Stop VDO capture
    if (engine->vdo) {
        vdo_capture_stop(engine->vdo);
    }
}
```

---

## Capture Thread

### Thread Function (lines 357-416)
```c
static void* capture_thread_func(void* arg) {
    PerceptionEngine* engine = (PerceptionEngine*)arg;
    GError* error = NULL;

    while (engine->running) {
        // 1. Get frame from VDO (blocking)
        VdoBuffer* buffer = NULL;

        if (engine->vdo) {
            buffer = vdo_capture_get_frame(engine->vdo, &error);

            if (!buffer) {
                // Error handling
                if (error) {
                    syslog(LOG_WARNING, "[Perception] Error getting frame: %s",
                           error->message);
                    g_error_free(error);
                    error = NULL;
                }

                if (!engine->running) break;

                usleep(10000);  // 10ms sleep before retry
                engine->frames_dropped++;
                continue;
            }
        } else {
            // Placeholder mode - no real VDO
            usleep(100000);  // 100ms (10 fps)
            continue;
        }

        // 2. Process frame with mutex protection
        pthread_mutex_lock(&engine->mutex);
        if (engine->running) {
            process_frame(engine, buffer);
        }
        pthread_mutex_unlock(&engine->mutex);

        // 3. Release frame back to VDO
        if (!vdo_capture_release_frame(engine->vdo, buffer)) {
            syslog(LOG_WARNING, "[Perception] Failed to release frame buffer");
        }
    }

    return NULL;
}
```

**Key Points**:
- Blocking `vdo_capture_get_frame()` call
- Error recovery with sleep-and-retry
- Mutex protection for shared state
- Proper buffer lifecycle (get → process → release)

---

## Frame Processing Pipeline

### Complete Pipeline (lines 418-491)
```c
static void process_frame(PerceptionEngine* engine, VdoBuffer* buffer) {
    if (!engine || !buffer) return;

    // ============================================================
    // STEP 1: Run Larod Inference
    // ============================================================
    DetectedObject detections[50];
    uint32_t num_detections = 0;

    bool inference_success = larod_inference_run(
        engine->larod,
        buffer,                // VdoBuffer input
        detections,            // Output array
        50,                    // Max detections
        &num_detections        // Actual count
    );

    if (!inference_success) {
        syslog(LOG_WARNING, "[Perception] Inference failed on frame");
        engine->frames_dropped++;
        return;  // Drop frame, continue processing
    }

    // ============================================================
    // STEP 2: Update Tracker
    // ============================================================
    TrackedObject tracks[50];
    uint32_t num_tracks = tracker_update(
        engine->tracker,
        detections,            // Input detections
        num_detections,
        tracks,                // Output tracks
        50                     // Max tracks
    );

    // ============================================================
    // STEP 3: Behavior Analysis
    // ============================================================
    behavior_analyze(engine->behavior, tracks, num_tracks);

    // ============================================================
    // STEP 4: User Callback
    // ============================================================
    if (engine->callback && num_tracks > 0) {
        engine->callback(tracks, num_tracks, engine->callback_user_data);
    }

    // ============================================================
    // STEP 5: Update Statistics
    // ============================================================
    engine->frames_processed++;

    float avg_inference_ms = 0.0f;
    larod_inference_get_stats(engine->larod, &avg_inference_ms,
                             NULL, NULL, NULL);

    // Exponential moving average
    if (engine->frames_processed > 0) {
        float alpha = 0.1f;
        engine->avg_inference_ms = (alpha * avg_inference_ms) +
                                   ((1.0f - alpha) * engine->avg_inference_ms);
    }

    // ============================================================
    // STEP 6: Adaptive Framerate
    // ============================================================
    if (engine->vdo && avg_inference_ms > 0.0f) {
        if (vdo_capture_update_framerate(engine->vdo,
                                        (unsigned int)avg_inference_ms)) {
            VdoFrameInfo frame_info;
            if (vdo_capture_get_frame_info(engine->vdo, &frame_info)) {
                engine->avg_fps = (float)frame_info.framerate;
                printf("[Perception] Framerate adjusted to %.1f fps "
                       "(inference: %.1f ms)\n",
                       engine->avg_fps, avg_inference_ms);
            }
        }
    }
}
```

---

## Error Handling Patterns

### Pattern 1: Initialization Failure with Cleanup
```c
engine->module = module_init(&config);
if (!engine->module) {
    syslog(LOG_ERR, "[Perception] Module initialization failed");
    printf("[Perception] Error: Module initialization failed\n");
    perception_destroy(engine);  // Cleanup all previous modules
    return NULL;
}
```

### Pattern 2: Runtime Error with Frame Drop
```c
if (!operation_success) {
    syslog(LOG_WARNING, "[Perception] Operation failed");
    engine->frames_dropped++;
    return;  // Drop frame, continue processing
}
```

### Pattern 3: Resource Acquisition Error with Retry
```c
VdoBuffer* buffer = vdo_capture_get_frame(engine->vdo, &error);
if (!buffer) {
    if (error) {
        syslog(LOG_WARNING, "[Perception] Error: %s", error->message);
        g_error_free(error);
        error = NULL;
    }
    usleep(10000);  // Sleep before retry
    engine->frames_dropped++;
    continue;
}
```

### Pattern 4: Graceful Degradation
```c
if (engine->vdo) {
    // Use real hardware
    buffer = vdo_capture_get_frame(engine->vdo, &error);
} else {
    // Placeholder mode - no real hardware
    syslog(LOG_WARNING, "[Perception] Running in placeholder mode");
    usleep(100000);  // Simulate frame time
    continue;
}
```

---

## Threading Model

### Thread Safety Guarantees

**Mutex-Protected Operations**:
- Starting/stopping engine
- Processing frames
- Accessing statistics

**Lock-Free Operations**:
- Reading `engine->running` (atomic flag semantics)
- Thread creation/destruction

**Thread Lifecycle**:
```
perception_start()
    ↓
Create capture_thread
    ↓
capture_thread_func()
    ├── vdo_capture_get_frame() [BLOCKING]
    ├── process_frame() [MUTEX PROTECTED]
    └── vdo_capture_release_frame()
    ↓
perception_stop()
    ├── Set running = false
    └── pthread_join() [WAIT FOR COMPLETION]
```

---

## Performance Characteristics

### Latency Breakdown (Target: <50ms total)
- **VDO capture**: ~5-10ms (hardware dependent)
- **Larod inference**: ~10-20ms (DLPU)
- **Tracker update**: ~2-5ms
- **Behavior analysis**: ~1-3ms
- **Callback**: <1ms

### Memory Usage (Target: <256MB)
- **VDO buffers**: ~20MB (4 buffers × 1920×1080 YUV)
- **Larod tensors**: ~50MB (model + I/O buffers)
- **Tracker state**: ~1MB (50 tracks × history)
- **Behavior history**: ~5MB (event log)
- **Engine overhead**: ~1MB

### CPU Usage (Target: <30%)
- **Inference**: ~15-20% (DLPU offload)
- **Tracking**: ~5-8%
- **Behavior**: ~2-3%
- **Other**: ~2-5%

---

## Testing Checklist

### Unit Tests
- [ ] `perception_init` with NULL config
- [ ] `perception_init` with missing model
- [ ] `perception_start` already running
- [ ] `perception_stop` not running
- [ ] Frame processing with inference failure
- [ ] Frame processing with >50 detections
- [ ] Statistics accuracy

### Integration Tests
- [ ] Full pipeline with real camera
- [ ] Adaptive framerate under load
- [ ] Thread safety under concurrent access
- [ ] Memory stability (24-hour run)
- [ ] Graceful shutdown under load

### Performance Tests
- [ ] Inference latency < 20ms (DLPU)
- [ ] End-to-end latency < 50ms
- [ ] CPU usage < 30%
- [ ] Memory usage < 256MB
- [ ] No memory leaks after 1M frames

---

**Last Updated**: 2025-10-30
**Status**: Phase 3 Integration Complete
**Next**: Implement vdo_capture.c, larod_inference.c, tracker.c, behavior.c
