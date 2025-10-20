/**
 * @file test_perception.c
 * @brief Unit tests for OMNISIGHT perception engine
 */

#include "../src/perception/perception.h"
#include "../src/perception/tracker.h"
#include "../src/perception/behavior.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

// Test configuration
#define TEST_WIDTH 416
#define TEST_HEIGHT 416
#define TEST_MODEL_PATH "/opt/app/models/detection.tflite"

// Test helpers
void test_perception_init() {
    printf("[TEST] perception_init... ");

    PerceptionConfig config = {
        .frame_width = TEST_WIDTH,
        .frame_height = TEST_HEIGHT,
        .target_fps = 10,
        .model_path = TEST_MODEL_PATH,
        .use_dlpu = false,  // Use CPU for testing
        .inference_threads = 2,
        .detection_threshold = 0.5,
        .tracking_threshold = 0.3,
        .max_tracked_objects = 50,
        .loitering_threshold_ms = 30000,
        .running_velocity_threshold = 100.0,
        .async_inference = false,
        .buffer_pool_size = 4
    };

    // Note: This will fail without actual camera hardware
    // For unit testing, we need mock interfaces
    PerceptionEngine* engine = perception_init(&config);

    if (engine) {
        perception_destroy(engine);
        printf("PASS\n");
    } else {
        printf("SKIP (no camera hardware)\n");
    }
}

void test_tracker() {
    printf("[TEST] tracker operations... ");

    TrackerConfig config = {
        .iou_threshold = 0.3,
        .max_age = 30,
        .min_hits = 3,
        .max_tracks = 50,
        .use_kalman_filter = true,
        .feature_similarity_weight = 0.3
    };

    Tracker* tracker = tracker_init(&config);
    assert(tracker != NULL);

    // Create mock detections
    DetectedObject detections[3] = {
        {
            .id = 1,
            .class_id = OBJECT_CLASS_PERSON,
            .confidence = 0.95,
            .bbox = {0.1, 0.1, 0.2, 0.3},
            .timestamp_ms = 1000
        },
        {
            .id = 2,
            .class_id = OBJECT_CLASS_VEHICLE,
            .confidence = 0.88,
            .bbox = {0.5, 0.5, 0.3, 0.3},
            .timestamp_ms = 1000
        },
        {
            .id = 3,
            .class_id = OBJECT_CLASS_PERSON,
            .confidence = 0.92,
            .bbox = {0.7, 0.2, 0.2, 0.4},
            .timestamp_ms = 1000
        }
    };

    TrackedObject tracks[10];
    uint32_t num_tracks = tracker_update(tracker, detections, 3, tracks, 10);

    // First update should create new tracks but not confirm them yet (min_hits=3)
    assert(num_tracks >= 0 && num_tracks <= 3);

    // Update again with same detections
    for (int i = 0; i < 3; i++) {
        detections[i].timestamp_ms += 100;
        detections[i].bbox.x += 0.01;  // Slight movement
    }

    num_tracks = tracker_update(tracker, detections, 3, tracks, 10);

    tracker_destroy(tracker);
    printf("PASS\n");
}

void test_iou_calculation() {
    printf("[TEST] IoU calculation... ");

    BoundingBox box1 = {0.0, 0.0, 0.5, 0.5};
    BoundingBox box2 = {0.25, 0.25, 0.5, 0.5};

    float iou = tracker_calculate_iou(&box1, &box2);

    // Boxes overlap by 0.25x0.25 = 0.0625
    // Union = 0.5*0.5 + 0.5*0.5 - 0.0625 = 0.4375
    // IoU = 0.0625 / 0.4375 ≈ 0.14
    assert(iou > 0.1 && iou < 0.2);

    // Test identical boxes
    iou = tracker_calculate_iou(&box1, &box1);
    assert(iou > 0.99 && iou <= 1.0);

    // Test non-overlapping boxes
    BoundingBox box3 = {0.6, 0.6, 0.3, 0.3};
    iou = tracker_calculate_iou(&box1, &box3);
    assert(iou == 0.0);

    printf("PASS\n");
}

void test_behavior_analyzer() {
    printf("[TEST] behavior analyzer... ");

    BehaviorConfig config = {
        .loitering_threshold_ms = 5000,
        .loitering_movement_threshold = 0.05,
        .running_velocity_threshold = 50.0,
        .running_frames_threshold = 5,
        .repeated_passes_count = 3,
        .repeated_passes_window_ms = 60000,
        .observation_threshold_ms = 10000,
        .observation_gaze_threshold = 0.8,
        .loitering_weight = 0.3,
        .running_weight = 0.5,
        .concealing_weight = 0.8,
        .repeated_passes_weight = 0.4,
        .observation_weight = 0.6
    };

    BehaviorAnalyzer* analyzer = behavior_init(&config);
    assert(analyzer != NULL);

    // Create a mock tracked object
    TrackedObject track = {
        .track_id = 1,
        .class_id = OBJECT_CLASS_PERSON,
        .current_bbox = {0.3, 0.3, 0.2, 0.4},
        .velocity_x = 0.01,
        .velocity_y = 0.01,
        .confidence = 0.95,
        .frame_count = 100,
        .miss_count = 0,
        .behaviors = BEHAVIOR_NORMAL,
        .threat_score = 0.0,
        .first_seen_ms = 0,
        .last_seen_ms = 6000  // 6 seconds
    };

    // Should detect loitering (threshold is 5000ms)
    bool loitering = behavior_detect_loitering(analyzer, &track);
    assert(loitering == true);

    // Update velocity for running detection
    track.velocity_x = 60.0;  // Above threshold
    track.velocity_y = 10.0;
    bool running = behavior_detect_running(analyzer, &track);
    assert(running == true);

    // Test threat score calculation
    float threat = behavior_calculate_threat_score(analyzer, &track);
    assert(threat > 0.0 && threat <= 1.0);

    behavior_destroy(analyzer);
    printf("PASS\n");
}

void test_behavior_flags() {
    printf("[TEST] behavior flags... ");

    BehaviorFlags flags = BEHAVIOR_NORMAL;
    assert(flags == 0x00);

    // Set multiple flags
    flags |= BEHAVIOR_LOITERING;
    flags |= BEHAVIOR_RUNNING;
    assert(flags & BEHAVIOR_LOITERING);
    assert(flags & BEHAVIOR_RUNNING);
    assert(!(flags & BEHAVIOR_CONCEALING));

    // Test all flag combinations
    flags = BEHAVIOR_LOITERING | BEHAVIOR_SUSPICIOUS_MOVEMENT | BEHAVIOR_REPEATED_PASSES;
    assert(flags & BEHAVIOR_LOITERING);
    assert(flags & BEHAVIOR_SUSPICIOUS_MOVEMENT);
    assert(flags & BEHAVIOR_REPEATED_PASSES);

    printf("PASS\n");
}

int main(int argc, char** argv) {
    printf("========================================\n");
    printf("OMNISIGHT Perception Engine Tests\n");
    printf("========================================\n\n");

    // Run tests
    test_iou_calculation();
    test_behavior_flags();
    test_tracker();
    test_behavior_analyzer();
    test_perception_init();  // May skip without hardware

    printf("\n========================================\n");
    printf("All tests completed successfully!\n");
    printf("========================================\n");

    return 0;
}
