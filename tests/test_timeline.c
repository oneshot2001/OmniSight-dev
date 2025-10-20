/**
 * @file test_timeline.c
 * @brief Unit tests for Timeline Threading™ engine
 */

#include "../src/timeline/timeline.h"
#include "../src/timeline/trajectory_predictor.h"
#include "../src/timeline/event_predictor.h"
#include "../src/perception/perception.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include <time.h>

#define TEST_ASSERT(condition, message) \
    do { \
        if (!(condition)) { \
            fprintf(stderr, "FAIL: %s\n", message); \
            return false; \
        } \
    } while(0)

#define TEST_PASS(message) \
    do { \
        printf("PASS: %s\n", message); \
        return true; \
    } while(0)

// ============================================================================
// Test Utilities
// ============================================================================

static uint64_t get_current_time_ms() {
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    return (uint64_t)ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
}

static TrackedObject create_test_track(
    uint32_t id,
    float x, float y,
    float vx, float vy,
    BehaviorFlags behaviors,
    float threat_score
) {
    TrackedObject track;
    memset(&track, 0, sizeof(TrackedObject));

    track.track_id = id;
    track.box.x = x;
    track.box.y = y;
    track.box.width = 50;
    track.box.height = 100;
    track.confidence = 0.9f;
    track.velocity_x = vx;
    track.velocity_y = vy;
    track.behaviors = behaviors;
    track.threat_score = threat_score;
    track.last_seen_ms = get_current_time_ms();
    track.frames_tracked = 30;

    return track;
}

// ============================================================================
// Trajectory Predictor Tests
// ============================================================================

bool test_trajectory_predictor_init() {
    TrajectoryPredictorConfig config = {
        .motion_model = MOTION_MODEL_KALMAN,
        .prediction_horizon_s = 30.0f,
        .prediction_step_s = 1.0f,
        .max_branch_depth = 3
    };

    TrajectoryPredictor* predictor = trajectory_predictor_init(&config);
    TEST_ASSERT(predictor != NULL, "Trajectory predictor initialization");

    trajectory_predictor_destroy(predictor);
    TEST_PASS("Trajectory predictor init/destroy");
}

bool test_trajectory_constant_velocity() {
    TrajectoryPredictorConfig config = {
        .motion_model = MOTION_MODEL_CONSTANT_VELOCITY,
        .prediction_horizon_s = 10.0f,
        .prediction_step_s = 1.0f,
        .max_branch_depth = 3
    };

    TrajectoryPredictor* predictor = trajectory_predictor_init(&config);
    TEST_ASSERT(predictor != NULL, "Predictor created");

    // Create a track moving right at 10 pixels/second
    TrackedObject track = create_test_track(1, 100, 100, 10, 0, 0, 0.0f);

    PredictedTrajectory trajectory;
    bool result = trajectory_predict_single(
        predictor,
        &track,
        NULL,
        0,
        NULL,
        &trajectory
    );

    TEST_ASSERT(result, "Prediction succeeded");
    TEST_ASSERT(trajectory.num_predictions == 10, "Correct number of predictions");

    // Check first prediction (1 second ahead)
    float expected_x = 100 + 10 * 1;  // x + vx * t
    TEST_ASSERT(fabsf(trajectory.predictions[0].x - expected_x) < 0.1f,
               "First prediction X coordinate");

    // Check last prediction (10 seconds ahead)
    expected_x = 100 + 10 * 10;
    TEST_ASSERT(fabsf(trajectory.predictions[9].x - expected_x) < 0.1f,
               "Last prediction X coordinate");

    trajectory_predictor_destroy(predictor);
    TEST_PASS("Constant velocity prediction");
}

bool test_trajectory_branching() {
    TrajectoryPredictorConfig config = {
        .motion_model = MOTION_MODEL_KALMAN,
        .prediction_horizon_s = 10.0f,
        .prediction_step_s = 1.0f,
        .max_branch_depth = 3
    };

    TrajectoryPredictor* predictor = trajectory_predictor_init(&config);

    TrackedObject track = create_test_track(1, 100, 100, 10, 5, 0, 0.0f);

    PredictedTrajectory trajectories[5];
    uint32_t num_branches = trajectory_predict_branches(
        predictor,
        &track,
        3,  // Request 3 branches
        NULL,
        0,
        NULL,
        trajectories
    );

    TEST_ASSERT(num_branches == 3, "Correct number of branches");
    TEST_ASSERT(trajectories[0].overall_confidence > trajectories[1].overall_confidence,
               "First branch most confident");

    trajectory_predictor_destroy(predictor);
    TEST_PASS("Trajectory branching");
}

bool test_trajectory_collision_detection() {
    TrajectoryPredictorConfig config = {
        .motion_model = MOTION_MODEL_CONSTANT_VELOCITY,
        .prediction_horizon_s = 10.0f,
        .prediction_step_s = 1.0f,
        .max_branch_depth = 3
    };

    TrajectoryPredictor* predictor = trajectory_predictor_init(&config);

    // Two tracks moving towards each other
    TrackedObject track1 = create_test_track(1, 100, 100, 10, 0, 0, 0.0f);
    TrackedObject track2 = create_test_track(2, 150, 100, -10, 0, 0, 0.0f);

    PredictedTrajectory traj1, traj2;
    trajectory_predict_single(predictor, &track1, NULL, 0, NULL, &traj1);
    trajectory_predict_single(predictor, &track2, NULL, 0, NULL, &traj2);

    uint64_t collision_time = 0;
    float collision_x = 0, collision_y = 0;

    bool collision = trajectory_detect_collision(
        &traj1,
        &traj2,
        10.0f,  // Collision distance
        &collision_time,
        &collision_x,
        &collision_y
    );

    TEST_ASSERT(collision, "Collision detected");
    TEST_ASSERT(fabsf(collision_x - 125.0f) < 5.0f, "Collision X near midpoint");

    trajectory_predictor_destroy(predictor);
    TEST_PASS("Collision detection");
}

bool test_trajectory_zone_entry() {
    TrajectoryPredictorConfig config = {
        .motion_model = MOTION_MODEL_CONSTANT_VELOCITY,
        .prediction_horizon_s = 10.0f,
        .prediction_step_s = 1.0f,
        .max_branch_depth = 3
    };

    TrajectoryPredictor* predictor = trajectory_predictor_init(&config);

    // Track moving towards zone at (200, 100)
    TrackedObject track = create_test_track(1, 100, 100, 10, 0, 0, 0.0f);

    PredictedTrajectory trajectory;
    trajectory_predict_single(predictor, &track, NULL, 0, NULL, &trajectory);

    uint64_t entry_time = 0;
    float entry_confidence = 0.0f;

    bool enters = trajectory_check_zone_entry(
        &trajectory,
        200, 100,  // Zone center
        20.0f,     // Zone radius
        &entry_time,
        &entry_confidence
    );

    TEST_ASSERT(enters, "Zone entry detected");
    TEST_ASSERT(entry_confidence > 0.0f, "Entry confidence > 0");

    trajectory_predictor_destroy(predictor);
    TEST_PASS("Zone entry detection");
}

// ============================================================================
// Event Predictor Tests
// ============================================================================

bool test_event_predictor_init() {
    SceneContext scene;
    memset(&scene, 0, sizeof(SceneContext));

    EventPredictorConfig config = {
        .loitering_threshold_ms = 30000,
        .theft_proximity_threshold = 0.5f,
        .assault_velocity_threshold = 50.0f,
        .collision_distance_threshold = 30.0f,
        .trajectory_weight = 0.3f,
        .behavior_weight = 0.3f,
        .context_weight = 0.2f,
        .history_weight = 0.2f,
        .scene = &scene
    };

    EventPredictor* predictor = event_predictor_init(&config);
    TEST_ASSERT(predictor != NULL, "Event predictor initialization");

    event_predictor_destroy(predictor);
    TEST_PASS("Event predictor init/destroy");
}

bool test_event_predict_loitering() {
    SceneContext scene;
    memset(&scene, 0, sizeof(SceneContext));
    scene.time_of_day_risk = 1.0f;

    EventPredictorConfig config = {
        .loitering_threshold_ms = 5000,  // 5 seconds for testing
        .theft_proximity_threshold = 0.5f,
        .assault_velocity_threshold = 50.0f,
        .collision_distance_threshold = 30.0f,
        .trajectory_weight = 0.3f,
        .behavior_weight = 0.3f,
        .context_weight = 0.2f,
        .history_weight = 0.2f,
        .scene = &scene
    };

    EventPredictor* event_predictor = event_predictor_init(&config);

    TrajectoryPredictorConfig traj_config = {
        .motion_model = MOTION_MODEL_CONSTANT_VELOCITY,
        .prediction_horizon_s = 10.0f,
        .prediction_step_s = 1.0f,
        .max_branch_depth = 3
    };

    TrajectoryPredictor* traj_predictor = trajectory_predictor_init(&traj_config);

    // Create track with very low velocity (loitering)
    TrackedObject track = create_test_track(1, 100, 100, 0.1f, 0.1f,
                                            BEHAVIOR_LOITERING, 0.6f);

    PredictedTrajectory trajectory;
    trajectory_predict_single(traj_predictor, &track, NULL, 0, NULL, &trajectory);

    PredictedEvent event;
    bool detected = event_predict_loitering(event_predictor, &trajectory, &event);

    TEST_ASSERT(detected, "Loitering detected");
    TEST_ASSERT(event.type == EVENT_TYPE_LOITERING, "Correct event type");
    TEST_ASSERT(event.probability > 0.3f, "Reasonable probability");

    trajectory_predictor_destroy(traj_predictor);
    event_predictor_destroy(event_predictor);
    TEST_PASS("Loitering prediction");
}

bool test_event_predict_collision() {
    SceneContext scene;
    memset(&scene, 0, sizeof(SceneContext));

    EventPredictorConfig config = {
        .loitering_threshold_ms = 30000,
        .theft_proximity_threshold = 0.5f,
        .assault_velocity_threshold = 50.0f,
        .collision_distance_threshold = 30.0f,
        .trajectory_weight = 0.3f,
        .behavior_weight = 0.3f,
        .context_weight = 0.2f,
        .history_weight = 0.2f,
        .scene = &scene
    };

    EventPredictor* event_predictor = event_predictor_init(&config);

    TrajectoryPredictorConfig traj_config = {
        .motion_model = MOTION_MODEL_CONSTANT_VELOCITY,
        .prediction_horizon_s = 10.0f,
        .prediction_step_s = 1.0f,
        .max_branch_depth = 3
    };

    TrajectoryPredictor* traj_predictor = trajectory_predictor_init(&traj_config);

    // Two tracks on collision course
    TrackedObject track1 = create_test_track(1, 100, 100, 10, 0, 0, 0.3f);
    TrackedObject track2 = create_test_track(2, 150, 100, -10, 0, 0, 0.3f);

    PredictedTrajectory trajectories[2];
    trajectory_predict_single(traj_predictor, &track1, NULL, 0, NULL, &trajectories[0]);
    trajectory_predict_single(traj_predictor, &track2, NULL, 0, NULL, &trajectories[1]);

    PredictedEvent event;
    bool detected = event_predict_collision(event_predictor, trajectories, 2, &event);

    TEST_ASSERT(detected, "Collision detected");
    TEST_ASSERT(event.type == EVENT_TYPE_COLLISION, "Correct event type");
    TEST_ASSERT(event.num_involved_tracks == 2, "Two tracks involved");

    trajectory_predictor_destroy(traj_predictor);
    event_predictor_destroy(event_predictor);
    TEST_PASS("Collision prediction");
}

bool test_event_severity_calculation() {
    SceneContext scene;
    memset(&scene, 0, sizeof(SceneContext));
    scene.time_of_day_risk = 1.5f;  // High risk time

    EventPredictorConfig config = {
        .loitering_threshold_ms = 30000,
        .theft_proximity_threshold = 0.5f,
        .assault_velocity_threshold = 50.0f,
        .collision_distance_threshold = 30.0f,
        .trajectory_weight = 0.3f,
        .behavior_weight = 0.3f,
        .context_weight = 0.2f,
        .history_weight = 0.2f,
        .scene = &scene
    };

    EventPredictor* predictor = event_predictor_init(&config);

    // Create high-probability assault event
    PredictedEvent event;
    memset(&event, 0, sizeof(PredictedEvent));
    event.type = EVENT_TYPE_ASSAULT;
    event.probability = 0.9f;

    SeverityLevel severity = event_calculate_severity(predictor, &event);

    TEST_ASSERT(severity == SEVERITY_CRITICAL || severity == SEVERITY_HIGH,
               "Assault has high severity");

    event_predictor_destroy(predictor);
    TEST_PASS("Severity calculation");
}

// ============================================================================
// Timeline Engine Tests
// ============================================================================

bool test_timeline_engine_init() {
    SceneContext scene;
    memset(&scene, 0, sizeof(SceneContext));

    TimelineConfig config = {
        .prediction_horizon_s = 30.0f,
        .update_interval_ms = 100,
        .num_timelines = 3,
        .branching_enabled = true,
        .scene_context = &scene
    };

    TimelineEngine* engine = timeline_init(&config);
    TEST_ASSERT(engine != NULL, "Timeline engine initialization");

    timeline_destroy(engine);
    TEST_PASS("Timeline engine init/destroy");
}

bool test_timeline_update() {
    SceneContext scene;
    memset(&scene, 0, sizeof(SceneContext));
    scene.time_of_day_risk = 1.0f;

    TimelineConfig config = {
        .prediction_horizon_s = 30.0f,
        .update_interval_ms = 100,
        .num_timelines = 3,
        .branching_enabled = true,
        .scene_context = &scene
    };

    TimelineEngine* engine = timeline_init(&config);

    // Create test tracks
    TrackedObject tracks[2];
    tracks[0] = create_test_track(1, 100, 100, 10, 5, 0, 0.5f);
    tracks[1] = create_test_track(2, 200, 150, -5, 3, BEHAVIOR_LOITERING, 0.7f);

    Timeline* timelines[10];
    uint32_t num_timelines = timeline_update(
        engine,
        tracks,
        2,
        get_current_time_ms(),
        timelines
    );

    TEST_ASSERT(num_timelines > 0, "Timelines created");
    TEST_ASSERT(num_timelines <= 3, "Correct number of timelines");

    // Check that events were predicted
    uint64_t total_events = 0;
    for (uint32_t i = 0; i < num_timelines; i++) {
        PredictedEvent events[50];
        uint32_t n = timeline_get_events(timelines[i], events, 50);
        total_events += n;
    }

    printf("  Total events predicted: %lu\n", total_events);

    timeline_destroy(engine);
    TEST_PASS("Timeline update");
}

bool test_timeline_interventions() {
    SceneContext scene;
    memset(&scene, 0, sizeof(SceneContext));

    // Add a protected zone
    scene.num_zones = 1;
    scene.zones[0].x = 150;
    scene.zones[0].y = 150;
    scene.zones[0].radius = 50;
    scene.zones[0].protected_event = EVENT_TYPE_TRESPASSING;
    scene.zones[0].sensitivity = 0.9f;

    TimelineConfig config = {
        .prediction_horizon_s = 30.0f,
        .update_interval_ms = 100,
        .num_timelines = 3,
        .branching_enabled = true,
        .scene_context = &scene
    };

    TimelineEngine* engine = timeline_init(&config);

    // Track moving towards protected zone
    TrackedObject tracks[1];
    tracks[0] = create_test_track(1, 100, 150, 5, 0, 0, 0.6f);

    Timeline* timelines[10];
    uint32_t num_timelines = timeline_update(
        engine,
        tracks,
        1,
        get_current_time_ms(),
        timelines
    );

    // Get best intervention
    InterventionPoint intervention;
    bool has_intervention = timeline_get_best_intervention(engine, &intervention);

    if (has_intervention) {
        printf("  Intervention: type=%d, urgency=%d, time_to_act=%lums\n",
               intervention.intervention,
               intervention.urgency,
               intervention.time_to_act_ms);
    }

    timeline_destroy(engine);
    TEST_PASS("Timeline interventions");
}

bool test_timeline_statistics() {
    SceneContext scene;
    memset(&scene, 0, sizeof(SceneContext));

    TimelineConfig config = {
        .prediction_horizon_s = 30.0f,
        .update_interval_ms = 100,
        .num_timelines = 3,
        .branching_enabled = false,
        .scene_context = &scene
    };

    TimelineEngine* engine = timeline_init(&config);

    // Run multiple updates
    TrackedObject tracks[2];
    for (int i = 0; i < 5; i++) {
        tracks[0] = create_test_track(1, 100 + i*10, 100, 10, 0, 0, 0.5f);
        tracks[1] = create_test_track(2, 200 - i*10, 150, -10, 0, 0, 0.5f);

        timeline_update(engine, tracks, 2, get_current_time_ms(), NULL);
    }

    // Get statistics
    uint64_t num_updates = 0;
    uint64_t total_events = 0;
    uint64_t total_interventions = 0;
    float avg_confidence = 0.0f;

    timeline_get_stats(engine, &num_updates, &total_events,
                      &total_interventions, &avg_confidence);

    printf("  Updates: %lu, Events: %lu, Interventions: %lu, Avg confidence: %.2f\n",
           num_updates, total_events, total_interventions, avg_confidence);

    TEST_ASSERT(num_updates == 5, "Correct number of updates");

    timeline_destroy(engine);
    TEST_PASS("Timeline statistics");
}

// ============================================================================
// Main Test Runner
// ============================================================================

typedef bool (*TestFunction)();

typedef struct {
    const char* name;
    TestFunction func;
} TestCase;

int main() {
    printf("\n");
    printf("=============================================================\n");
    printf(" OMNISIGHT Timeline Threading™ Unit Tests\n");
    printf("=============================================================\n\n");

    TestCase tests[] = {
        // Trajectory predictor tests
        {"Trajectory Predictor Init", test_trajectory_predictor_init},
        {"Constant Velocity Prediction", test_trajectory_constant_velocity},
        {"Trajectory Branching", test_trajectory_branching},
        {"Collision Detection", test_trajectory_collision_detection},
        {"Zone Entry Detection", test_trajectory_zone_entry},

        // Event predictor tests
        {"Event Predictor Init", test_event_predictor_init},
        {"Loitering Prediction", test_event_predict_loitering},
        {"Collision Prediction", test_event_predict_collision},
        {"Severity Calculation", test_event_severity_calculation},

        // Timeline engine tests
        {"Timeline Engine Init", test_timeline_engine_init},
        {"Timeline Update", test_timeline_update},
        {"Timeline Interventions", test_timeline_interventions},
        {"Timeline Statistics", test_timeline_statistics},
    };

    int num_tests = sizeof(tests) / sizeof(TestCase);
    int passed = 0;
    int failed = 0;

    for (int i = 0; i < num_tests; i++) {
        printf("\n[%d/%d] Testing: %s\n", i+1, num_tests, tests[i].name);
        printf("-------------------------------------------------------------\n");

        if (tests[i].func()) {
            passed++;
        } else {
            failed++;
        }
    }

    printf("\n=============================================================\n");
    printf(" Test Results\n");
    printf("=============================================================\n");
    printf("Total:  %d\n", num_tests);
    printf("Passed: %d (%.1f%%)\n", passed, 100.0f * passed / num_tests);
    printf("Failed: %d\n", failed);
    printf("=============================================================\n\n");

    return failed == 0 ? 0 : 1;
}
