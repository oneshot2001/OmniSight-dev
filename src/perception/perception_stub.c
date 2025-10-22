/**
 * @file perception_stub.c
 * @brief Stub implementation of perception engine for testing without hardware
 *
 * This stub simulates the perception engine behavior to allow development
 * and testing without an Axis camera.
 */

#include "perception.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <math.h>

// Stub structure
struct PerceptionEngine {
    PerceptionConfig config;
    PerceptionCallback callback;
    void* callback_user_data;

    bool running;
    pthread_t thread;
    pthread_mutex_t mutex;

    // Simulation state
    uint32_t frame_count;
    uint32_t next_track_id;
    TrackedObject simulated_tracks[10];
    uint32_t num_simulated_tracks;

    // Statistics
    float avg_inference_ms;
    float avg_fps;
    uint32_t dropped_frames;
};

// ============================================================================
// Simulation Helpers
// ============================================================================

static uint64_t get_current_time_ms(void) {
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    return (uint64_t)ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
}

static float randf(float min, float max) {
    return min + (float)rand() / RAND_MAX * (max - min);
}

/**
 * Simulate a person walking across the field of view
 */
static void simulate_person_track(TrackedObject* track, uint32_t track_id, uint32_t frame) {
    track->track_id = track_id;
    track->class_id = OBJECT_CLASS_PERSON;
    track->confidence = randf(0.8f, 0.95f);
    track->frame_count = frame;
    track->miss_count = 0;

    // Simulate walking motion
    float progress = (frame % 100) / 100.0f;
    track->current_bbox.x = progress;
    track->current_bbox.y = 0.4f + sinf(progress * 6.28f) * 0.1f;
    track->current_bbox.width = 0.08f;
    track->current_bbox.height = 0.15f;

    // Predict next position
    track->predicted_bbox = track->current_bbox;
    track->predicted_bbox.x += 0.01f;

    // Velocity
    track->velocity_x = 0.01f;
    track->velocity_y = 0.0f;

    // Behavior detection
    track->behaviors = BEHAVIOR_NORMAL;
    if (progress > 0.3f && progress < 0.7f) {
        track->behaviors |= BEHAVIOR_LOITERING;
        track->threat_score = 0.3f;
    } else {
        track->threat_score = 0.1f;
    }

    // Timestamps
    track->first_seen_ms = get_current_time_ms() - (frame * 100);
    track->last_seen_ms = get_current_time_ms();

    // Generate dummy features
    for (int i = 0; i < 128; i++) {
        track->features[i] = randf(-1.0f, 1.0f);
    }
}

/**
 * Simulate a vehicle passing by
 */
static void simulate_vehicle_track(TrackedObject* track, uint32_t track_id, uint32_t frame) {
    track->track_id = track_id;
    track->class_id = OBJECT_CLASS_VEHICLE;
    track->confidence = randf(0.85f, 0.98f);
    track->frame_count = frame;
    track->miss_count = 0;

    // Fast horizontal movement
    float progress = (frame % 50) / 50.0f;
    track->current_bbox.x = progress;
    track->current_bbox.y = 0.6f;
    track->current_bbox.width = 0.12f;
    track->current_bbox.height = 0.08f;

    track->predicted_bbox = track->current_bbox;
    track->predicted_bbox.x += 0.02f;

    track->velocity_x = 0.02f;
    track->velocity_y = 0.0f;

    track->behaviors = BEHAVIOR_NORMAL;
    track->threat_score = 0.05f;

    track->first_seen_ms = get_current_time_ms() - (frame * 100);
    track->last_seen_ms = get_current_time_ms();

    for (int i = 0; i < 128; i++) {
        track->features[i] = randf(-1.0f, 1.0f);
    }
}

/**
 * Generate simulated tracked objects
 */
static void generate_simulated_tracks(PerceptionEngine* engine) {
    pthread_mutex_lock(&engine->mutex);

    engine->num_simulated_tracks = 0;

    // Simulate 1-3 people
    uint32_t num_people = 1 + (rand() % 3);
    for (uint32_t i = 0; i < num_people && engine->num_simulated_tracks < 10; i++) {
        simulate_person_track(
            &engine->simulated_tracks[engine->num_simulated_tracks],
            engine->next_track_id++,
            engine->frame_count
        );
        engine->num_simulated_tracks++;
    }

    // Occasionally simulate a vehicle
    if (rand() % 5 == 0 && engine->num_simulated_tracks < 10) {
        simulate_vehicle_track(
            &engine->simulated_tracks[engine->num_simulated_tracks],
            engine->next_track_id++,
            engine->frame_count
        );
        engine->num_simulated_tracks++;
    }

    pthread_mutex_unlock(&engine->mutex);
}

/**
 * Processing thread
 */
static void* processing_thread(void* arg) {
    PerceptionEngine* engine = (PerceptionEngine*)arg;

    printf("[Perception] Stub processing thread started\n");

    while (engine->running) {
        uint64_t frame_start = get_current_time_ms();

        // Generate simulated tracks
        generate_simulated_tracks(engine);

        // Call callback if registered
        if (engine->callback) {
            engine->callback(
                engine->simulated_tracks,
                engine->num_simulated_tracks,
                engine->callback_user_data
            );
        }

        engine->frame_count++;

        // Simulate processing time
        uint64_t frame_end = get_current_time_ms();
        float frame_time = (float)(frame_end - frame_start);

        // Update statistics
        engine->avg_inference_ms = engine->avg_inference_ms * 0.9f + frame_time * 0.1f;
        float target_frame_time = 1000.0f / engine->config.target_fps;
        engine->avg_fps = 1000.0f / fmaxf(frame_time, target_frame_time);

        // Sleep to maintain target FPS
        usleep((int)(target_frame_time * 1000));
    }

    printf("[Perception] Stub processing thread stopped\n");
    return NULL;
}

// ============================================================================
// Public API Implementation
// ============================================================================

PerceptionEngine* perception_init(const PerceptionConfig* config) {
    if (!config) {
        return NULL;
    }

    printf("[Perception] Initializing stub engine...\n");

    PerceptionEngine* engine = (PerceptionEngine*)calloc(1, sizeof(PerceptionEngine));
    if (!engine) {
        return NULL;
    }

    engine->config = *config;
    engine->running = false;
    engine->frame_count = 0;
    engine->next_track_id = 1000;
    engine->num_simulated_tracks = 0;
    engine->avg_inference_ms = 15.0f;
    engine->avg_fps = config->target_fps;
    engine->dropped_frames = 0;

    pthread_mutex_init(&engine->mutex, NULL);

    printf("[Perception] ✓ Stub engine initialized\n");
    printf("[Perception]   - Target FPS: %u\n", config->target_fps);
    printf("[Perception]   - Resolution: %ux%u\n",
           config->frame_width, config->frame_height);
    printf("[Perception]   - Simulated mode (no hardware)\n");

    return engine;
}

bool perception_start(PerceptionEngine* engine,
                      PerceptionCallback callback,
                      void* user_data) {
    if (!engine || engine->running) {
        return false;
    }

    printf("[Perception] Starting stub processing...\n");

    engine->callback = callback;
    engine->callback_user_data = user_data;
    engine->running = true;

    // Start processing thread
    if (pthread_create(&engine->thread, NULL, processing_thread, engine) != 0) {
        printf("[Perception] Error: Failed to create processing thread\n");
        engine->running = false;
        return false;
    }

    printf("[Perception] ✓ Stub processing started\n");
    return true;
}

void perception_stop(PerceptionEngine* engine) {
    if (!engine || !engine->running) {
        return;
    }

    printf("[Perception] Stopping stub processing...\n");

    engine->running = false;
    pthread_join(engine->thread, NULL);

    printf("[Perception] ✓ Stub processing stopped\n");
}

void perception_destroy(PerceptionEngine* engine) {
    if (!engine) {
        return;
    }

    if (engine->running) {
        perception_stop(engine);
    }

    pthread_mutex_destroy(&engine->mutex);
    free(engine);

    printf("[Perception] ✓ Stub engine destroyed\n");
}

uint32_t perception_process_frame(PerceptionEngine* engine,
                                   const uint8_t* frame_data,
                                   uint32_t width,
                                   uint32_t height,
                                   DetectedObject* objects,
                                   uint32_t max_objects) {
    if (!engine) return 0;

    // Stub: Just generate some detections
    (void)frame_data;
    (void)width;
    (void)height;

    uint32_t count = (rand() % 3) + 1;
    if (count > max_objects) count = max_objects;

    for (uint32_t i = 0; i < count; i++) {
        objects[i].id = i;
        objects[i].class_id = OBJECT_CLASS_PERSON;
        objects[i].confidence = randf(0.7f, 0.95f);
        objects[i].bbox.x = randf(0.0f, 0.8f);
        objects[i].bbox.y = randf(0.0f, 0.8f);
        objects[i].bbox.width = 0.1f;
        objects[i].bbox.height = 0.15f;
        objects[i].timestamp_ms = get_current_time_ms();
    }

    return count;
}

uint32_t perception_get_tracked_objects(PerceptionEngine* engine,
                                         TrackedObject* objects,
                                         uint32_t max_objects) {
    if (!engine || !objects) return 0;

    pthread_mutex_lock(&engine->mutex);
    uint32_t count = engine->num_simulated_tracks < max_objects ?
                     engine->num_simulated_tracks : max_objects;
    memcpy(objects, engine->simulated_tracks, count * sizeof(TrackedObject));
    pthread_mutex_unlock(&engine->mutex);

    return count;
}

void perception_update_behavior_params(PerceptionEngine* engine,
                                        uint32_t loitering_ms,
                                        float running_threshold) {
    if (!engine) return;

    engine->config.loitering_threshold_ms = loitering_ms;
    engine->config.running_velocity_threshold = running_threshold;
}

void perception_get_stats(PerceptionEngine* engine,
                          float* avg_inference_ms,
                          float* avg_fps,
                          uint32_t* dropped_frames) {
    if (!engine) return;

    if (avg_inference_ms) *avg_inference_ms = engine->avg_inference_ms;
    if (avg_fps) *avg_fps = engine->avg_fps;
    if (dropped_frames) *dropped_frames = engine->dropped_frames;
}
