/**
 * @file timeline.c
 * @brief Timeline Threading™ engine implementation
 *
 * Core innovation of OMNISIGHT: Predicts 3-5 probable futures
 * 30 seconds to 5 minutes ahead with confidence scores.
 *
 * Architecture:
 * - Trajectory prediction for each tracked object
 * - Branch prediction for alternative futures
 * - Event prediction on each timeline
 * - Intervention recommendation
 */

#include "timeline.h"
#include "trajectory_predictor.h"
#include "event_predictor.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <math.h>

/**
 * Timeline node structure (internal)
 */
struct TimelineNode {
    uint32_t node_id;
    uint64_t timestamp_ms;
    float probability;

    // Predicted states at this node
    uint32_t num_states;
    PredictedState states[100];

    // Events predicted at this node
    uint32_t num_events;
    PredictedEvent events[20];

    // Branching
    uint32_t num_children;
    struct TimelineNode* children[5];  // Max 5 branches

    // Parent node (NULL for root)
    struct TimelineNode* parent;
};

/**
 * Timeline structure (internal)
 */
struct Timeline {
    uint32_t timeline_id;
    float overall_probability;

    // Tree structure
    TimelineNode* root;
    uint32_t total_nodes;

    // Predicted events on this timeline
    uint32_t num_predicted_events;
    PredictedEvent predicted_events[50];

    // Recommended interventions
    uint32_t num_interventions;
    InterventionPoint interventions[20];
};

/**
 * Timeline Threading engine state
 */
struct TimelineEngine {
    TimelineConfig config;

    // Subsystems
    TrajectoryPredictor* trajectory_predictor;
    EventPredictor* event_predictor;

    // Active timelines
    uint32_t num_timelines;
    Timeline* timelines[10];  // Max 10 timelines

    // Statistics
    struct {
        uint64_t num_updates;
        uint64_t total_events_predicted;
        uint64_t total_interventions;
        float avg_timeline_confidence;
    } stats;

    // Node pool for efficient allocation
    TimelineNode* node_pool;
    uint32_t node_pool_size;
    uint32_t next_free_node;
};

// ============================================================================
// Utility Functions
// ============================================================================

static uint64_t get_current_time_ms() {
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    return (uint64_t)ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
}

static float clamp(float value, float min, float max) {
    if (value < min) return min;
    if (value > max) return max;
    return value;
}

// ============================================================================
// Timeline Node Management
// ============================================================================

static TimelineNode* timeline_node_create(TimelineEngine* engine) {
    if (engine->next_free_node >= engine->node_pool_size) {
        fprintf(stderr, "[Timeline] ERROR: Node pool exhausted\n");
        return NULL;
    }

    TimelineNode* node = &engine->node_pool[engine->next_free_node++];
    memset(node, 0, sizeof(TimelineNode));
    node->node_id = engine->next_free_node - 1;

    return node;
}

static void timeline_node_destroy_recursive(TimelineNode* node) {
    if (!node) return;

    for (uint32_t i = 0; i < node->num_children; i++) {
        timeline_node_destroy_recursive(node->children[i]);
    }

    // Note: nodes are in pool, don't free individually
}

static void timeline_node_add_child(TimelineNode* parent, TimelineNode* child) {
    if (!parent || !child) return;

    if (parent->num_children >= 5) {
        fprintf(stderr, "[Timeline] WARNING: Max branches exceeded\n");
        return;
    }

    parent->children[parent->num_children++] = child;
    child->parent = parent;
}

// ============================================================================
// Timeline Management
// ============================================================================

static Timeline* timeline_create(TimelineEngine* engine, uint32_t id) {
    Timeline* timeline = calloc(1, sizeof(Timeline));
    if (!timeline) {
        fprintf(stderr, "[Timeline] ERROR: Failed to allocate timeline\n");
        return NULL;
    }

    timeline->timeline_id = id;
    timeline->overall_probability = 1.0f;
    timeline->root = timeline_node_create(engine);
    timeline->total_nodes = 1;

    if (!timeline->root) {
        free(timeline);
        return NULL;
    }

    timeline->root->timestamp_ms = get_current_time_ms();
    timeline->root->probability = 1.0f;

    return timeline;
}

static void timeline_destroy(Timeline* timeline) {
    if (!timeline) return;

    timeline_node_destroy_recursive(timeline->root);
    free(timeline);
}

// ============================================================================
// Timeline Building
// ============================================================================

/**
 * Build timeline tree by predicting trajectories and events
 */
static bool timeline_build_tree(
    TimelineEngine* engine,
    Timeline* timeline,
    const TrackedObject* tracks,
    uint32_t num_tracks,
    uint64_t current_time_ms
) {
    // Predict trajectories for each track
    PredictedTrajectory trajectories[100];
    uint32_t num_trajectories = 0;

    for (uint32_t i = 0; i < num_tracks && num_trajectories < 100; i++) {
        if (!trajectory_predict_single(
            engine->trajectory_predictor,
            &tracks[i],
            tracks,
            num_tracks,
            engine->config.scene_context,
            &trajectories[num_trajectories]
        )) {
            continue;
        }
        num_trajectories++;
    }

    if (num_trajectories == 0) {
        return false;
    }

    // Update root node with current predicted states
    TimelineNode* root = timeline->root;
    root->num_states = 0;

    for (uint32_t i = 0; i < num_trajectories && root->num_states < 100; i++) {
        if (trajectories[i].num_predictions > 0) {
            root->states[root->num_states++] = trajectories[i].predictions[0];
        }
    }

    // Predict events on this timeline
    root->num_events = event_predictor_predict(
        engine->event_predictor,
        trajectories,
        num_trajectories,
        root->events,
        20
    );

    // Store events in timeline
    timeline->num_predicted_events = 0;
    for (uint32_t i = 0; i < root->num_events && timeline->num_predicted_events < 50; i++) {
        timeline->predicted_events[timeline->num_predicted_events++] = root->events[i];
    }

    // Create branch timelines if configured
    if (engine->config.num_timelines > 1 && engine->config.branching_enabled) {
        // For now, create linear timeline (branching is complex)
        // TODO: Implement full branching based on decision points
    }

    // Calculate overall timeline probability
    float sum_confidence = 0.0f;
    for (uint32_t i = 0; i < num_trajectories; i++) {
        sum_confidence += trajectories[i].overall_confidence;
    }
    timeline->overall_probability = sum_confidence / num_trajectories;

    return true;
}

// ============================================================================
// Intervention Recommendation
// ============================================================================

/**
 * Recommend best intervention to prevent predicted events
 */
static bool timeline_recommend_interventions(
    TimelineEngine* engine,
    Timeline* timeline,
    uint64_t current_time_ms
) {
    timeline->num_interventions = 0;

    // Analyze predicted events and recommend interventions
    for (uint32_t i = 0; i < timeline->num_predicted_events; i++) {
        const PredictedEvent* event = &timeline->predicted_events[i];

        // Only intervene for high-severity events
        if (event->severity < SEVERITY_MEDIUM) {
            continue;
        }

        if (timeline->num_interventions >= 20) {
            break;
        }

        InterventionPoint* intervention = &timeline->interventions[timeline->num_interventions++];
        memset(intervention, 0, sizeof(InterventionPoint));

        intervention->event_id = i;
        intervention->trigger_time_ms = current_time_ms;
        intervention->event_time_ms = event->timestamp_ms;
        intervention->location_x = event->location_x;
        intervention->location_y = event->location_y;
        intervention->target_track_id = event->involved_tracks[0];
        intervention->success_probability = event->probability * 0.8f;  // Assume 80% intervention success

        // Recommend intervention type based on event severity and type
        switch (event->severity) {
            case SEVERITY_CRITICAL:
                intervention->intervention = INTERVENTION_ALERT_SECURITY;
                intervention->urgency = URGENCY_IMMEDIATE;
                break;

            case SEVERITY_HIGH:
                if (event->type == EVENT_TYPE_ASSAULT || event->type == EVENT_TYPE_THEFT) {
                    intervention->intervention = INTERVENTION_ALERT_SECURITY;
                    intervention->urgency = URGENCY_HIGH;
                } else {
                    intervention->intervention = INTERVENTION_ACTIVATE_SPEAKER;
                    intervention->urgency = URGENCY_HIGH;
                }
                break;

            case SEVERITY_MEDIUM:
                intervention->intervention = INTERVENTION_ACTIVATE_SPEAKER;
                intervention->urgency = URGENCY_MEDIUM;
                break;

            default:
                intervention->intervention = INTERVENTION_LOG_ONLY;
                intervention->urgency = URGENCY_LOW;
                break;
        }

        // Calculate time to act
        intervention->time_to_act_ms = event->timestamp_ms - current_time_ms;
    }

    return timeline->num_interventions > 0;
}

// ============================================================================
// Public API Implementation
// ============================================================================

TimelineEngine* timeline_init(const TimelineConfig* config) {
    if (!config) {
        fprintf(stderr, "[Timeline] ERROR: NULL config\n");
        return NULL;
    }

    TimelineEngine* engine = calloc(1, sizeof(TimelineEngine));
    if (!engine) {
        fprintf(stderr, "[Timeline] ERROR: Failed to allocate engine\n");
        return NULL;
    }

    // Copy configuration
    memcpy(&engine->config, config, sizeof(TimelineConfig));

    // Initialize trajectory predictor
    TrajectoryPredictorConfig traj_config = {
        .motion_model = MOTION_MODEL_KALMAN,
        .prediction_horizon_s = config->prediction_horizon_s,
        .prediction_step_s = 1.0f,
        .max_branch_depth = 3
    };
    engine->trajectory_predictor = trajectory_predictor_init(&traj_config);
    if (!engine->trajectory_predictor) {
        free(engine);
        return NULL;
    }

    // Initialize event predictor
    EventPredictorConfig event_config = {
        .loitering_threshold_ms = 30000,  // 30 seconds
        .theft_proximity_threshold = 0.5f,
        .assault_velocity_threshold = 50.0f,
        .collision_distance_threshold = 30.0f,
        .trajectory_weight = 0.3f,
        .behavior_weight = 0.3f,
        .context_weight = 0.2f,
        .history_weight = 0.2f,
        .scene = config->scene_context
    };
    engine->event_predictor = event_predictor_init(&event_config);
    if (!engine->event_predictor) {
        trajectory_predictor_destroy(engine->trajectory_predictor);
        free(engine);
        return NULL;
    }

    // Allocate node pool
    engine->node_pool_size = 1000;  // Support 1000 nodes
    engine->node_pool = calloc(engine->node_pool_size, sizeof(TimelineNode));
    if (!engine->node_pool) {
        event_predictor_destroy(engine->event_predictor);
        trajectory_predictor_destroy(engine->trajectory_predictor);
        free(engine);
        return NULL;
    }
    engine->next_free_node = 0;

    // Initialize statistics
    memset(&engine->stats, 0, sizeof(engine->stats));

    printf("[Timeline] Timeline Threading™ engine initialized\n");
    printf("[Timeline] Horizon: %.1fs, Timelines: %u, Branching: %s\n",
           config->prediction_horizon_s,
           config->num_timelines,
           config->branching_enabled ? "enabled" : "disabled");

    return engine;
}

void timeline_destroy(TimelineEngine* engine) {
    if (!engine) return;

    // Destroy all timelines
    for (uint32_t i = 0; i < engine->num_timelines; i++) {
        timeline_destroy(engine->timelines[i]);
    }

    // Destroy subsystems
    event_predictor_destroy(engine->event_predictor);
    trajectory_predictor_destroy(engine->trajectory_predictor);

    // Free node pool
    free(engine->node_pool);

    free(engine);
}

uint32_t timeline_update(
    TimelineEngine* engine,
    const TrackedObject* tracks,
    uint32_t num_tracks,
    uint64_t current_time_ms,
    Timeline** timelines_out
) {
    if (!engine || !tracks || num_tracks == 0) {
        return 0;
    }

    // Clear old timelines
    for (uint32_t i = 0; i < engine->num_timelines; i++) {
        timeline_destroy(engine->timelines[i]);
        engine->timelines[i] = NULL;
    }
    engine->num_timelines = 0;

    // Reset node pool
    engine->next_free_node = 0;

    // Create timelines
    uint32_t num_timelines_to_create = engine->config.num_timelines;
    if (num_timelines_to_create > 10) {
        num_timelines_to_create = 10;
    }

    for (uint32_t i = 0; i < num_timelines_to_create; i++) {
        Timeline* timeline = timeline_create(engine, i);
        if (!timeline) {
            continue;
        }

        // Build timeline tree
        if (!timeline_build_tree(engine, timeline, tracks, num_tracks, current_time_ms)) {
            timeline_destroy(timeline);
            continue;
        }

        // Recommend interventions
        timeline_recommend_interventions(engine, timeline, current_time_ms);

        engine->timelines[engine->num_timelines++] = timeline;

        // Update statistics
        engine->stats.total_events_predicted += timeline->num_predicted_events;
        engine->stats.total_interventions += timeline->num_interventions;
    }

    // Update statistics
    engine->stats.num_updates++;

    // Calculate average confidence
    float sum_confidence = 0.0f;
    for (uint32_t i = 0; i < engine->num_timelines; i++) {
        sum_confidence += engine->timelines[i]->overall_probability;
    }
    engine->stats.avg_timeline_confidence = sum_confidence / engine->num_timelines;

    // Return timelines
    if (timelines_out) {
        for (uint32_t i = 0; i < engine->num_timelines; i++) {
            timelines_out[i] = engine->timelines[i];
        }
    }

    return engine->num_timelines;
}

bool timeline_get_best_intervention(
    TimelineEngine* engine,
    InterventionPoint* intervention
) {
    if (!engine || !intervention) {
        return false;
    }

    // Find highest priority intervention across all timelines
    InterventionPoint* best = NULL;
    float best_score = 0.0f;

    for (uint32_t i = 0; i < engine->num_timelines; i++) {
        Timeline* timeline = engine->timelines[i];

        for (uint32_t j = 0; j < timeline->num_interventions; j++) {
            InterventionPoint* candidate = &timeline->interventions[j];

            // Score = probability * urgency * timeline_confidence
            float score = candidate->success_probability *
                         (float)candidate->urgency *
                         timeline->overall_probability;

            if (score > best_score) {
                best_score = score;
                best = candidate;
            }
        }
    }

    if (!best) {
        return false;
    }

    // Copy best intervention
    memcpy(intervention, best, sizeof(InterventionPoint));
    return true;
}

uint32_t timeline_get_events(
    const Timeline* timeline,
    PredictedEvent* events,
    uint32_t max_events
) {
    if (!timeline || !events || max_events == 0) {
        return 0;
    }

    uint32_t num_to_copy = timeline->num_predicted_events < max_events ?
                          timeline->num_predicted_events : max_events;

    memcpy(events, timeline->predicted_events, num_to_copy * sizeof(PredictedEvent));
    return num_to_copy;
}

uint32_t timeline_get_interventions(
    const Timeline* timeline,
    InterventionPoint* interventions,
    uint32_t max_interventions
) {
    if (!timeline || !interventions || max_interventions == 0) {
        return 0;
    }

    uint32_t num_to_copy = timeline->num_interventions < max_interventions ?
                          timeline->num_interventions : max_interventions;

    memcpy(interventions, timeline->interventions, num_to_copy * sizeof(InterventionPoint));
    return num_to_copy;
}

float timeline_get_probability(const Timeline* timeline) {
    return timeline ? timeline->overall_probability : 0.0f;
}

uint32_t timeline_get_id(const Timeline* timeline) {
    return timeline ? timeline->timeline_id : 0;
}

void timeline_get_stats(
    const TimelineEngine* engine,
    uint64_t* num_updates,
    uint64_t* total_events,
    uint64_t* total_interventions,
    float* avg_confidence
) {
    if (!engine) return;

    if (num_updates) *num_updates = engine->stats.num_updates;
    if (total_events) *total_events = engine->stats.total_events_predicted;
    if (total_interventions) *total_interventions = engine->stats.total_interventions;
    if (avg_confidence) *avg_confidence = engine->stats.avg_timeline_confidence;
}
