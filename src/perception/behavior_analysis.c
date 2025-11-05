/**
 * @file behavior_analysis.c
 * @brief OMNISIGHT Behavior Analysis Implementation
 *
 * Detects suspicious behaviors through trajectory and velocity analysis:
 * - Loitering: Extended dwell time in small area
 * - Running: High sustained velocity
 * - Unusual Movement: Erratic trajectory patterns
 * - Threat Scoring: Weighted combination of behaviors
 *
 * Copyright (C) 2025 OMNISIGHT
 */

#include "behavior_analysis.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <syslog.h>
#include <pthread.h>

#define MAX_TRACK_HISTORIES 100
#define POSITION_HISTORY_SIZE 60  // 6 seconds @ 10 FPS

/**
 * Track history storage
 */
typedef struct {
    uint32_t track_id;
    PositionHistory positions[POSITION_HISTORY_SIZE];
    uint32_t num_positions;
    uint32_t head;  // Circular buffer head
    uint64_t first_seen_ms;
    uint64_t last_update_ms;
    bool active;

    // Cached behavior state
    bool is_loitering;
    bool is_running;
    bool has_unusual_movement;
    float cached_threat_score;
} InternalTrackHistory;

/**
 * Behavior analyzer instance
 */
struct BehaviorAnalyzer {
    BehaviorAnalyzerConfig config;
    InternalTrackHistory histories[MAX_TRACK_HISTORIES];
    uint32_t num_histories;

    // Statistics
    uint32_t total_tracks_analyzed;
    uint32_t loitering_detections;
    uint32_t running_detections;
    uint32_t unusual_movement_detections;

    pthread_mutex_t mutex;
};

// Internal helper functions
static InternalTrackHistory* get_or_create_history(BehaviorAnalyzer* analyzer,
                                                   uint32_t track_id);
static void detect_loitering(BehaviorAnalyzer* analyzer,
                            InternalTrackHistory* history,
                            bool* is_loitering);
static void detect_running(BehaviorAnalyzer* analyzer,
                          InternalTrackHistory* history,
                          bool* is_running);
static void detect_unusual_movement(BehaviorAnalyzer* analyzer,
                                   InternalTrackHistory* history,
                                   bool* has_unusual);
static float calculate_threat_score(BehaviorAnalyzer* analyzer,
                                   InternalTrackHistory* history,
                                   BehaviorFlags behaviors);
static float calculate_direction_change(float x1, float y1,
                                       float x2, float y2,
                                       float x3, float y3);

// ============================================================================
// Public API Implementation
// ============================================================================

BehaviorAnalyzer* behavior_analyzer_init(const BehaviorAnalyzerConfig* config) {
    if (!config) {
        syslog(LOG_ERR, "[Behavior] Invalid configuration");
        return NULL;
    }

    BehaviorAnalyzer* analyzer = calloc(1, sizeof(BehaviorAnalyzer));
    if (!analyzer) {
        syslog(LOG_ERR, "[Behavior] Failed to allocate analyzer");
        return NULL;
    }

    analyzer->config = *config;
    analyzer->num_histories = 0;
    analyzer->total_tracks_analyzed = 0;
    analyzer->loitering_detections = 0;
    analyzer->running_detections = 0;
    analyzer->unusual_movement_detections = 0;

    if (pthread_mutex_init(&analyzer->mutex, NULL) != 0) {
        syslog(LOG_ERR, "[Behavior] Failed to initialize mutex");
        free(analyzer);
        return NULL;
    }

    syslog(LOG_INFO, "[Behavior] Initialized analyzer");
    syslog(LOG_INFO, "[Behavior] Loitering: dwell=%.1fs, vel=%.2fm/s, radius=%.1fm",
           config->loitering_dwell_time_ms / 1000.0f,
           config->loitering_velocity_threshold,
           config->loitering_radius_meters);
    syslog(LOG_INFO, "[Behavior] Running: vel=%.2fm/s, duration=%.1fs",
           config->running_velocity_threshold,
           config->running_duration_ms / 1000.0f);

    return analyzer;
}

void behavior_analyzer_analyze(BehaviorAnalyzer* analyzer,
                               TrackedObject* track) {
    if (!analyzer || !track) {
        return;
    }

    pthread_mutex_lock(&analyzer->mutex);

    InternalTrackHistory* history = get_or_create_history(analyzer, track->track_id);
    if (!history) {
        pthread_mutex_unlock(&analyzer->mutex);
        return;
    }

    // Reset behavior flags
    track->behaviors = BEHAVIOR_NORMAL;

    // Detect loitering
    bool is_loitering = false;
    detect_loitering(analyzer, history, &is_loitering);
    if (is_loitering) {
        track->behaviors |= BEHAVIOR_LOITERING;
        if (!history->is_loitering) {
            analyzer->loitering_detections++;
            history->is_loitering = true;
            syslog(LOG_INFO, "[Behavior] Track %u: LOITERING detected",
                   track->track_id);
        }
    } else {
        history->is_loitering = false;
    }

    // Detect running
    bool is_running = false;
    detect_running(analyzer, history, &is_running);
    if (is_running) {
        track->behaviors |= BEHAVIOR_RUNNING;
        if (!history->is_running) {
            analyzer->running_detections++;
            history->is_running = true;
            syslog(LOG_INFO, "[Behavior] Track %u: RUNNING detected",
                   track->track_id);
        }
    } else {
        history->is_running = false;
    }

    // Detect unusual movement
    bool has_unusual = false;
    detect_unusual_movement(analyzer, history, &has_unusual);
    if (has_unusual) {
        track->behaviors |= BEHAVIOR_SUSPICIOUS_MOVEMENT;
        if (!history->has_unusual_movement) {
            analyzer->unusual_movement_detections++;
            history->has_unusual_movement = true;
            syslog(LOG_INFO, "[Behavior] Track %u: UNUSUAL MOVEMENT detected",
                   track->track_id);
        }
    } else {
        history->has_unusual_movement = false;
    }

    // Calculate threat score
    track->threat_score = calculate_threat_score(analyzer, history, track->behaviors);
    history->cached_threat_score = track->threat_score;

    pthread_mutex_unlock(&analyzer->mutex);
}

void behavior_analyzer_update_history(BehaviorAnalyzer* analyzer,
                                      uint32_t track_id,
                                      const BoundingBox* bbox,
                                      uint64_t timestamp_ms) {
    if (!analyzer || !bbox) {
        return;
    }

    pthread_mutex_lock(&analyzer->mutex);

    InternalTrackHistory* history = get_or_create_history(analyzer, track_id);
    if (!history) {
        pthread_mutex_unlock(&analyzer->mutex);
        return;
    }

    // Calculate center position
    float x = bbox->x + bbox->width / 2.0f;
    float y = bbox->y + bbox->height / 2.0f;

    // Calculate velocity if we have previous position
    float velocity = 0.0f;
    if (history->num_positions > 0) {
        uint32_t prev_idx = (history->head + POSITION_HISTORY_SIZE - 1) % POSITION_HISTORY_SIZE;
        PositionHistory* prev = &history->positions[prev_idx];

        float dx = x - prev->x;
        float dy = y - prev->y;
        float dt_sec = (timestamp_ms - prev->timestamp_ms) / 1000.0f;

        if (dt_sec > 0.0f) {
            float dist_normalized = sqrtf(dx * dx + dy * dy);
            float dist_meters = normalized_to_meters(dist_normalized,
                                                     analyzer->config.meters_per_normalized_unit);
            velocity = dist_meters / dt_sec;
        }
    }

    // Add to circular buffer
    PositionHistory* pos = &history->positions[history->head];
    pos->x = x;
    pos->y = y;
    pos->timestamp_ms = timestamp_ms;
    pos->velocity = velocity;

    history->head = (history->head + 1) % POSITION_HISTORY_SIZE;
    if (history->num_positions < POSITION_HISTORY_SIZE) {
        history->num_positions++;
    }

    if (history->first_seen_ms == 0) {
        history->first_seen_ms = timestamp_ms;
    }
    history->last_update_ms = timestamp_ms;

    pthread_mutex_unlock(&analyzer->mutex);
}

bool behavior_analyzer_get_history(BehaviorAnalyzer* analyzer,
                                   uint32_t track_id,
                                   TrackHistory* history) {
    if (!analyzer || !history) {
        return false;
    }

    pthread_mutex_lock(&analyzer->mutex);

    bool found = false;
    for (uint32_t i = 0; i < MAX_TRACK_HISTORIES; i++) {
        if (analyzer->histories[i].active &&
            analyzer->histories[i].track_id == track_id) {

            InternalTrackHistory* internal = &analyzer->histories[i];
            history->track_id = track_id;
            history->num_positions = internal->num_positions;
            history->first_seen_ms = internal->first_seen_ms;
            history->last_update_ms = internal->last_update_ms;

            // Copy positions in chronological order
            for (uint32_t j = 0; j < internal->num_positions; j++) {
                uint32_t idx = (internal->head + POSITION_HISTORY_SIZE - internal->num_positions + j) %
                              POSITION_HISTORY_SIZE;
                history->positions[j] = internal->positions[idx];
            }

            found = true;
            break;
        }
    }

    pthread_mutex_unlock(&analyzer->mutex);
    return found;
}

void behavior_analyzer_clear_history(BehaviorAnalyzer* analyzer,
                                     uint32_t track_id) {
    if (!analyzer) {
        return;
    }

    pthread_mutex_lock(&analyzer->mutex);

    for (uint32_t i = 0; i < MAX_TRACK_HISTORIES; i++) {
        if (analyzer->histories[i].active &&
            analyzer->histories[i].track_id == track_id) {
            memset(&analyzer->histories[i], 0, sizeof(InternalTrackHistory));
            analyzer->histories[i].active = false;
            break;
        }
    }

    pthread_mutex_unlock(&analyzer->mutex);
}

void behavior_analyzer_reset(BehaviorAnalyzer* analyzer) {
    if (!analyzer) {
        return;
    }

    pthread_mutex_lock(&analyzer->mutex);

    memset(analyzer->histories, 0, sizeof(analyzer->histories));
    analyzer->num_histories = 0;
    analyzer->total_tracks_analyzed = 0;
    analyzer->loitering_detections = 0;
    analyzer->running_detections = 0;
    analyzer->unusual_movement_detections = 0;

    syslog(LOG_INFO, "[Behavior] Reset complete");

    pthread_mutex_unlock(&analyzer->mutex);
}

void behavior_analyzer_get_stats(BehaviorAnalyzer* analyzer,
                                 uint32_t* total_tracks_analyzed,
                                 uint32_t* loitering_detections,
                                 uint32_t* running_detections,
                                 uint32_t* unusual_movement_detections) {
    if (!analyzer) {
        return;
    }

    pthread_mutex_lock(&analyzer->mutex);

    if (total_tracks_analyzed) {
        *total_tracks_analyzed = analyzer->total_tracks_analyzed;
    }
    if (loitering_detections) {
        *loitering_detections = analyzer->loitering_detections;
    }
    if (running_detections) {
        *running_detections = analyzer->running_detections;
    }
    if (unusual_movement_detections) {
        *unusual_movement_detections = analyzer->unusual_movement_detections;
    }

    pthread_mutex_unlock(&analyzer->mutex);
}

void behavior_analyzer_destroy(BehaviorAnalyzer* analyzer) {
    if (!analyzer) {
        return;
    }

    pthread_mutex_lock(&analyzer->mutex);
    pthread_mutex_unlock(&analyzer->mutex);
    pthread_mutex_destroy(&analyzer->mutex);

    free(analyzer);

    syslog(LOG_INFO, "[Behavior] Destroyed");
}

// ============================================================================
// Internal Helper Functions
// ============================================================================

static InternalTrackHistory* get_or_create_history(BehaviorAnalyzer* analyzer,
                                                   uint32_t track_id) {
    // Search for existing history
    for (uint32_t i = 0; i < MAX_TRACK_HISTORIES; i++) {
        if (analyzer->histories[i].active &&
            analyzer->histories[i].track_id == track_id) {
            return &analyzer->histories[i];
        }
    }

    // Find free slot
    for (uint32_t i = 0; i < MAX_TRACK_HISTORIES; i++) {
        if (!analyzer->histories[i].active) {
            memset(&analyzer->histories[i], 0, sizeof(InternalTrackHistory));
            analyzer->histories[i].active = true;
            analyzer->histories[i].track_id = track_id;
            analyzer->total_tracks_analyzed++;
            return &analyzer->histories[i];
        }
    }

    // No free slots - reuse oldest
    uint32_t oldest_idx = 0;
    uint64_t oldest_time = analyzer->histories[0].last_update_ms;

    for (uint32_t i = 1; i < MAX_TRACK_HISTORIES; i++) {
        if (analyzer->histories[i].last_update_ms < oldest_time) {
            oldest_time = analyzer->histories[i].last_update_ms;
            oldest_idx = i;
        }
    }

    syslog(LOG_WARNING, "[Behavior] Reusing history slot (track %u evicted)",
           analyzer->histories[oldest_idx].track_id);

    memset(&analyzer->histories[oldest_idx], 0, sizeof(InternalTrackHistory));
    analyzer->histories[oldest_idx].active = true;
    analyzer->histories[oldest_idx].track_id = track_id;
    analyzer->total_tracks_analyzed++;

    return &analyzer->histories[oldest_idx];
}

static void detect_loitering(BehaviorAnalyzer* analyzer,
                            InternalTrackHistory* history,
                            bool* is_loitering) {
    *is_loitering = false;

    if (history->num_positions < 10) {
        return;  // Need minimum history
    }

    // Calculate dwell time
    uint64_t dwell_time = history->last_update_ms - history->first_seen_ms;
    if (dwell_time < analyzer->config.loitering_dwell_time_ms) {
        return;  // Not enough time elapsed
    }

    // Check if track has stayed within small radius
    // Get earliest and latest positions
    uint32_t earliest_idx = (history->head + POSITION_HISTORY_SIZE - history->num_positions) %
                           POSITION_HISTORY_SIZE;
    uint32_t latest_idx = (history->head + POSITION_HISTORY_SIZE - 1) % POSITION_HISTORY_SIZE;

    float start_x = history->positions[earliest_idx].x;
    float start_y = history->positions[earliest_idx].y;

    // Check all positions are within radius of start
    float max_distance = 0.0f;
    for (uint32_t i = 0; i < history->num_positions; i++) {
        uint32_t idx = (history->head + POSITION_HISTORY_SIZE - history->num_positions + i) %
                      POSITION_HISTORY_SIZE;
        float dist = position_distance(start_x, start_y,
                                      history->positions[idx].x,
                                      history->positions[idx].y);
        if (dist > max_distance) {
            max_distance = dist;
        }
    }

    float max_distance_meters = normalized_to_meters(max_distance,
                                                     analyzer->config.meters_per_normalized_unit);

    if (max_distance_meters > analyzer->config.loitering_radius_meters) {
        return;  // Moved too far from start
    }

    // Check average velocity is low
    float avg_velocity = 0.0f;
    uint32_t velocity_samples = 0;

    for (uint32_t i = 0; i < history->num_positions; i++) {
        uint32_t idx = (history->head + POSITION_HISTORY_SIZE - history->num_positions + i) %
                      POSITION_HISTORY_SIZE;
        avg_velocity += history->positions[idx].velocity;
        velocity_samples++;
    }

    if (velocity_samples > 0) {
        avg_velocity /= velocity_samples;
    }

    if (avg_velocity > analyzer->config.loitering_velocity_threshold) {
        return;  // Moving too fast
    }

    // All criteria met - loitering detected
    *is_loitering = true;
}

static void detect_running(BehaviorAnalyzer* analyzer,
                          InternalTrackHistory* history,
                          bool* is_running) {
    *is_running = false;

    if (history->num_positions < 5) {
        return;  // Need minimum history
    }

    // Check if sustained high velocity
    uint32_t high_velocity_count = 0;
    uint64_t first_high_velocity_time = 0;
    uint64_t last_high_velocity_time = 0;

    for (uint32_t i = 0; i < history->num_positions; i++) {
        uint32_t idx = (history->head + POSITION_HISTORY_SIZE - history->num_positions + i) %
                      POSITION_HISTORY_SIZE;

        if (history->positions[idx].velocity >= analyzer->config.running_velocity_threshold) {
            high_velocity_count++;
            if (first_high_velocity_time == 0) {
                first_high_velocity_time = history->positions[idx].timestamp_ms;
            }
            last_high_velocity_time = history->positions[idx].timestamp_ms;
        }
    }

    if (high_velocity_count < 3) {
        return;  // Not sustained
    }

    // Check duration of high velocity
    uint64_t duration = last_high_velocity_time - first_high_velocity_time;
    if (duration < analyzer->config.running_duration_ms) {
        return;  // Not sustained long enough
    }

    // Running detected
    *is_running = true;
}

static void detect_unusual_movement(BehaviorAnalyzer* analyzer,
                                   InternalTrackHistory* history,
                                   bool* has_unusual) {
    *has_unusual = false;

    if (history->num_positions < 6) {
        return;  // Need minimum history for direction changes
    }

    // Count significant direction changes (zigzag pattern)
    uint32_t direction_changes = 0;

    for (uint32_t i = 2; i < history->num_positions; i++) {
        uint32_t idx1 = (history->head + POSITION_HISTORY_SIZE - history->num_positions + i - 2) %
                       POSITION_HISTORY_SIZE;
        uint32_t idx2 = (history->head + POSITION_HISTORY_SIZE - history->num_positions + i - 1) %
                       POSITION_HISTORY_SIZE;
        uint32_t idx3 = (history->head + POSITION_HISTORY_SIZE - history->num_positions + i) %
                       POSITION_HISTORY_SIZE;

        float angle = calculate_direction_change(
            history->positions[idx1].x, history->positions[idx1].y,
            history->positions[idx2].x, history->positions[idx2].y,
            history->positions[idx3].x, history->positions[idx3].y
        );

        // Direction change > threshold (e.g., 45 degrees)
        if (fabsf(angle) > analyzer->config.zigzag_threshold) {
            direction_changes++;
        }
    }

    // Unusual if many direction changes
    if (direction_changes >= analyzer->config.zigzag_count_threshold) {
        *has_unusual = true;
    }
}

static float calculate_threat_score(BehaviorAnalyzer* analyzer,
                                   InternalTrackHistory* history,
                                   BehaviorFlags behaviors) {
    float score = 0.0f;

    // Behavior-based scoring
    if (behaviors & BEHAVIOR_LOITERING) {
        score += analyzer->config.weight_loitering;
    }
    if (behaviors & BEHAVIOR_RUNNING) {
        score += analyzer->config.weight_running;
    }
    if (behaviors & BEHAVIOR_SUSPICIOUS_MOVEMENT) {
        score += analyzer->config.weight_unusual_movement;
    }

    // Time-based escalation (longer presence = higher threat)
    if (history->first_seen_ms > 0 && history->last_update_ms > 0) {
        uint64_t dwell_time = history->last_update_ms - history->first_seen_ms;
        float dwell_minutes = dwell_time / 60000.0f;

        // Escalate by weight_dwell_time per minute (capped at 1.0)
        score += analyzer->config.weight_dwell_time * dwell_minutes;
    }

    // Clamp to [0.0, 1.0]
    if (score > 1.0f) {
        score = 1.0f;
    }

    return score;
}

static float calculate_direction_change(float x1, float y1,
                                       float x2, float y2,
                                       float x3, float y3) {
    // Calculate angle between vectors (p1→p2) and (p2→p3)
    float dx1 = x2 - x1;
    float dy1 = y2 - y1;
    float dx2 = x3 - x2;
    float dy2 = y3 - y2;

    // Calculate angles
    float angle1 = atan2f(dy1, dx1);
    float angle2 = atan2f(dy2, dx2);

    // Angle difference
    float diff = angle2 - angle1;

    // Normalize to [-180, 180] degrees
    while (diff > M_PI) diff -= 2.0f * M_PI;
    while (diff < -M_PI) diff += 2.0f * M_PI;

    // Convert to degrees
    return diff * 180.0f / M_PI;
}
