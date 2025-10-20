/**
 * @file behavior.c
 * @brief Behavior analysis implementation for OMNISIGHT
 *
 * Detects suspicious behaviors and calculates threat scores
 */

#include "behavior.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdio.h>

#define MAX_EVENTS 1000
#define MAX_TRACK_HISTORY 100

/**
 * Track history entry for pattern detection
 */
typedef struct {
    uint32_t track_id;
    BoundingBox positions[MAX_TRACK_HISTORY];
    uint64_t timestamps_ms[MAX_TRACK_HISTORY];
    uint32_t entry_count;
    uint32_t current_index;
} TrackHistory;

/**
 * Behavior analyzer structure
 */
struct BehaviorAnalyzer {
    BehaviorConfig config;
    BehaviorEvent events[MAX_EVENTS];
    uint32_t event_count;
    uint32_t event_write_index;
    TrackHistory histories[MAX_TRACKS];
    uint64_t total_events;
    float running_avg_threat;
    uint32_t high_threat_count;
};

// Forward declarations
static void update_track_history(BehaviorAnalyzer* analyzer, const TrackedObject* track);
static TrackHistory* get_track_history(BehaviorAnalyzer* analyzer, uint32_t track_id);
static void add_behavior_event(
    BehaviorAnalyzer* analyzer,
    uint32_t track_id,
    BehaviorFlags behavior,
    float confidence,
    uint64_t start_time_ms,
    uint64_t duration_ms,
    float threat_contribution
);

// ============================================================================
// Public API Implementation
// ============================================================================

BehaviorAnalyzer* behavior_init(const BehaviorConfig* config) {
    if (!config) {
        return NULL;
    }

    BehaviorAnalyzer* analyzer = (BehaviorAnalyzer*)calloc(1, sizeof(BehaviorAnalyzer));
    if (!analyzer) {
        return NULL;
    }

    analyzer->config = *config;
    analyzer->event_count = 0;
    analyzer->event_write_index = 0;
    analyzer->total_events = 0;
    analyzer->running_avg_threat = 0.0f;
    analyzer->high_threat_count = 0;

    // Initialize track histories
    for (int i = 0; i < MAX_TRACKS; i++) {
        analyzer->histories[i].track_id = 0;
        analyzer->histories[i].entry_count = 0;
        analyzer->histories[i].current_index = 0;
    }

    return analyzer;
}

uint32_t behavior_analyze(
    BehaviorAnalyzer* analyzer,
    TrackedObject* tracks,
    uint32_t num_tracks
) {
    if (!analyzer || !tracks) {
        return 0;
    }

    uint32_t events_detected = 0;
    analyzer->high_threat_count = 0;
    float total_threat = 0.0f;

    for (uint32_t i = 0; i < num_tracks; i++) {
        TrackedObject* track = &tracks[i];

        // Update track history
        update_track_history(analyzer, track);

        // Reset behavior flags
        track->behaviors = BEHAVIOR_NORMAL;

        // Detect loitering
        if (behavior_detect_loitering(analyzer, track)) {
            track->behaviors |= BEHAVIOR_LOITERING;
            events_detected++;
        }

        // Detect running
        if (behavior_detect_running(analyzer, track)) {
            track->behaviors |= BEHAVIOR_RUNNING;
            events_detected++;
        }

        // Detect repeated passes
        if (behavior_detect_repeated_passes(analyzer, track)) {
            track->behaviors |= BEHAVIOR_REPEATED_PASSES;
            events_detected++;
        }

        // Calculate threat score
        track->threat_score = behavior_calculate_threat_score(analyzer, track);
        total_threat += track->threat_score;

        if (track->threat_score > 0.7f) {
            analyzer->high_threat_count++;
        }
    }

    // Update running average threat score
    if (num_tracks > 0) {
        analyzer->running_avg_threat =
            0.9f * analyzer->running_avg_threat +
            0.1f * (total_threat / num_tracks);
    }

    return events_detected;
}

uint32_t behavior_get_events(
    BehaviorAnalyzer* analyzer,
    BehaviorEvent* events,
    uint32_t max_events
) {
    if (!analyzer || !events) {
        return 0;
    }

    uint32_t count = 0;
    uint32_t events_to_copy = (analyzer->event_count < max_events) ?
                              analyzer->event_count : max_events;

    for (uint32_t i = 0; i < events_to_copy; i++) {
        events[count++] = analyzer->events[i];
    }

    return count;
}

float behavior_calculate_threat_score(
    BehaviorAnalyzer* analyzer,
    const TrackedObject* track
) {
    if (!analyzer || !track) {
        return 0.0f;
    }

    float score = 0.0f;

    // Loitering contribution
    if (track->behaviors & BEHAVIOR_LOITERING) {
        uint64_t duration = track->last_seen_ms - track->first_seen_ms;
        float loitering_severity = (float)duration / (float)analyzer->config.loitering_threshold_ms;
        loitering_severity = fminf(loitering_severity, 1.0f);
        score += loitering_severity * analyzer->config.loitering_weight;
    }

    // Running contribution
    if (track->behaviors & BEHAVIOR_RUNNING) {
        float velocity = sqrtf(track->velocity_x * track->velocity_x +
                              track->velocity_y * track->velocity_y);
        float running_severity = velocity / analyzer->config.running_velocity_threshold;
        running_severity = fminf(running_severity, 1.0f);
        score += running_severity * analyzer->config.running_weight;
    }

    // Concealing contribution
    if (track->behaviors & BEHAVIOR_CONCEALING) {
        score += analyzer->config.concealing_weight;
    }

    // Repeated passes contribution
    if (track->behaviors & BEHAVIOR_REPEATED_PASSES) {
        score += analyzer->config.repeated_passes_weight;
    }

    // Extended observation contribution
    if (track->behaviors & BEHAVIOR_EXTENDED_OBSERVATION) {
        score += analyzer->config.observation_weight;
    }

    // Suspicious movement contribution
    if (track->behaviors & BEHAVIOR_SUSPICIOUS_MOVEMENT) {
        score += 0.3f;  // Fixed weight for suspicious movement
    }

    // Clamp to [0, 1]
    return fminf(score, 1.0f);
}

bool behavior_detect_loitering(
    BehaviorAnalyzer* analyzer,
    const TrackedObject* track
) {
    if (!analyzer || !track) {
        return false;
    }

    // Check if object has been present long enough
    uint64_t duration_ms = track->last_seen_ms - track->first_seen_ms;
    if (duration_ms < analyzer->config.loitering_threshold_ms) {
        return false;
    }

    // Check if movement is minimal
    TrackHistory* history = get_track_history(analyzer, track->track_id);
    if (!history || history->entry_count < 10) {
        return false;
    }

    // Calculate total movement
    float total_movement = 0.0f;
    uint32_t start_idx = history->current_index;

    for (uint32_t i = 1; i < history->entry_count && i < MAX_TRACK_HISTORY; i++) {
        uint32_t idx = (start_idx - i + MAX_TRACK_HISTORY) % MAX_TRACK_HISTORY;
        uint32_t prev_idx = (start_idx - i + 1 + MAX_TRACK_HISTORY) % MAX_TRACK_HISTORY;

        float dx = history->positions[idx].x - history->positions[prev_idx].x;
        float dy = history->positions[idx].y - history->positions[prev_idx].y;
        total_movement += sqrtf(dx * dx + dy * dy);
    }

    float avg_movement = total_movement / fminf(history->entry_count, MAX_TRACK_HISTORY);

    return avg_movement < analyzer->config.loitering_movement_threshold;
}

bool behavior_detect_running(
    BehaviorAnalyzer* analyzer,
    const TrackedObject* track
) {
    if (!analyzer || !track) {
        return false;
    }

    float velocity = sqrtf(track->velocity_x * track->velocity_x +
                          track->velocity_y * track->velocity_y);

    // Convert to pixels/second (assuming track velocity is in normalized coords/frame at 10fps)
    float velocity_pixels_per_sec = velocity * 416.0f * 10.0f;  // 416 = frame width, 10 = fps

    if (velocity_pixels_per_sec < analyzer->config.running_velocity_threshold) {
        return false;
    }

    // Check if velocity is sustained for multiple frames
    return track->frame_count >= analyzer->config.running_frames_threshold;
}

bool behavior_detect_repeated_passes(
    BehaviorAnalyzer* analyzer,
    const TrackedObject* track
) {
    if (!analyzer || !track) {
        return false;
    }

    TrackHistory* history = get_track_history(analyzer, track->track_id);
    if (!history || history->entry_count < analyzer->config.repeated_passes_count) {
        return false;
    }

    // Count how many times object entered a specific zone
    // Simplified: count direction reversals
    uint32_t direction_changes = 0;
    float prev_direction = 0.0f;

    uint32_t start_idx = history->current_index;
    for (uint32_t i = 1; i < history->entry_count && i < MAX_TRACK_HISTORY; i++) {
        uint32_t idx = (start_idx - i + MAX_TRACK_HISTORY) % MAX_TRACK_HISTORY;
        uint32_t prev_idx = (start_idx - i + 1 + MAX_TRACK_HISTORY) % MAX_TRACK_HISTORY;

        float dx = history->positions[idx].x - history->positions[prev_idx].x;
        float current_direction = (dx > 0) ? 1.0f : -1.0f;

        if (i > 1 && current_direction != prev_direction) {
            direction_changes++;
        }

        prev_direction = current_direction;
    }

    return direction_changes >= analyzer->config.repeated_passes_count;
}

void behavior_update_config(
    BehaviorAnalyzer* analyzer,
    const BehaviorConfig* config
) {
    if (analyzer && config) {
        analyzer->config = *config;
    }
}

void behavior_clear_history(BehaviorAnalyzer* analyzer) {
    if (!analyzer) {
        return;
    }

    for (int i = 0; i < MAX_TRACKS; i++) {
        analyzer->histories[i].entry_count = 0;
        analyzer->histories[i].current_index = 0;
    }

    analyzer->event_count = 0;
    analyzer->event_write_index = 0;
}

void behavior_get_stats(
    BehaviorAnalyzer* analyzer,
    uint64_t* total_events,
    float* avg_threat_score,
    uint32_t* high_threat_count
) {
    if (!analyzer) {
        return;
    }

    if (total_events) *total_events = analyzer->total_events;
    if (avg_threat_score) *avg_threat_score = analyzer->running_avg_threat;
    if (high_threat_count) *high_threat_count = analyzer->high_threat_count;
}

void behavior_destroy(BehaviorAnalyzer* analyzer) {
    if (analyzer) {
        free(analyzer);
    }
}

int behavior_to_string(
    BehaviorFlags behavior,
    char* buffer,
    size_t buffer_size
) {
    if (!buffer || buffer_size == 0) {
        return 0;
    }

    buffer[0] = '\0';
    int written = 0;

    if (behavior == BEHAVIOR_NORMAL) {
        return snprintf(buffer, buffer_size, "NORMAL");
    }

    if (behavior & BEHAVIOR_LOITERING) {
        written += snprintf(buffer + written, buffer_size - written, "LOITERING ");
    }
    if (behavior & BEHAVIOR_RUNNING) {
        written += snprintf(buffer + written, buffer_size - written, "RUNNING ");
    }
    if (behavior & BEHAVIOR_CONCEALING) {
        written += snprintf(buffer + written, buffer_size - written, "CONCEALING ");
    }
    if (behavior & BEHAVIOR_SUSPICIOUS_MOVEMENT) {
        written += snprintf(buffer + written, buffer_size - written, "SUSPICIOUS_MOVEMENT ");
    }
    if (behavior & BEHAVIOR_REPEATED_PASSES) {
        written += snprintf(buffer + written, buffer_size - written, "REPEATED_PASSES ");
    }
    if (behavior & BEHAVIOR_EXTENDED_OBSERVATION) {
        written += snprintf(buffer + written, buffer_size - written, "EXTENDED_OBSERVATION ");
    }

    // Remove trailing space
    if (written > 0 && buffer[written - 1] == ' ') {
        buffer[written - 1] = '\0';
        written--;
    }

    return written;
}

// ============================================================================
// Internal Helper Functions
// ============================================================================

static void update_track_history(BehaviorAnalyzer* analyzer, const TrackedObject* track) {
    TrackHistory* history = NULL;

    // Find existing history or create new one
    for (int i = 0; i < MAX_TRACKS; i++) {
        if (analyzer->histories[i].track_id == track->track_id) {
            history = &analyzer->histories[i];
            break;
        }
        if (analyzer->histories[i].track_id == 0) {
            history = &analyzer->histories[i];
            history->track_id = track->track_id;
            history->entry_count = 0;
            history->current_index = 0;
            break;
        }
    }

    if (!history) {
        return;  // No space for new history
    }

    // Add current position to history
    uint32_t idx = history->current_index;
    history->positions[idx] = track->current_bbox;
    history->timestamps_ms[idx] = track->last_seen_ms;

    history->current_index = (history->current_index + 1) % MAX_TRACK_HISTORY;
    if (history->entry_count < MAX_TRACK_HISTORY) {
        history->entry_count++;
    }
}

static TrackHistory* get_track_history(BehaviorAnalyzer* analyzer, uint32_t track_id) {
    for (int i = 0; i < MAX_TRACKS; i++) {
        if (analyzer->histories[i].track_id == track_id) {
            return &analyzer->histories[i];
        }
    }
    return NULL;
}

static void add_behavior_event(
    BehaviorAnalyzer* analyzer,
    uint32_t track_id,
    BehaviorFlags behavior,
    float confidence,
    uint64_t start_time_ms,
    uint64_t duration_ms,
    float threat_contribution
) {
    if (!analyzer) {
        return;
    }

    uint32_t idx = analyzer->event_write_index;
    BehaviorEvent* event = &analyzer->events[idx];

    event->track_id = track_id;
    event->behavior = behavior;
    event->confidence = confidence;
    event->start_time_ms = start_time_ms;
    event->duration_ms = duration_ms;
    event->threat_contribution = threat_contribution;

    analyzer->event_write_index = (analyzer->event_write_index + 1) % MAX_EVENTS;

    if (analyzer->event_count < MAX_EVENTS) {
        analyzer->event_count++;
    }

    analyzer->total_events++;
}
