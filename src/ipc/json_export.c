/**
 * @file json_export.c
 * @brief JSON Export Utilities Implementation
 */

#include "json_export.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

/**
 * Get current timestamp in milliseconds
 */
uint64_t json_get_timestamp_ms(void) {
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    return (uint64_t)ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
}

/**
 * Write JSON to file atomically
 * Writes to temp file first, then renames to avoid partial reads
 */
static bool write_json_atomic(const char* path, const char* json) {
    char temp_path[512];
    snprintf(temp_path, sizeof(temp_path), "%s.tmp", path);

    FILE* f = fopen(temp_path, "w");
    if (!f) {
        return false;
    }

    fprintf(f, "%s", json);
    fclose(f);

    // Atomic rename
    if (rename(temp_path, path) != 0) {
        unlink(temp_path);
        return false;
    }

    return true;
}

/**
 * Export system statistics to JSON
 */
bool json_export_stats(const OmnisightStats* stats) {
    if (!stats) return false;

    char json[4096];
    snprintf(json, sizeof(json),
        "{\n"
        "  \"timestamp\": %llu,\n"
        "  \"uptime_ms\": %llu,\n"
        "  \"frames_processed\": %llu,\n"
        "  \"is_running\": %s,\n"
        "  \"perception\": {\n"
        "    \"enabled\": true,\n"
        "    \"fps\": %.1f,\n"
        "    \"inference_ms\": %.1f,\n"
        "    \"objects_tracked\": %u,\n"
        "    \"dropped_frames\": %u\n"
        "  },\n"
        "  \"timeline\": {\n"
        "    \"enabled\": true,\n"
        "    \"active_timelines\": %u,\n"
        "    \"events_predicted\": %llu,\n"
        "    \"interventions\": %llu,\n"
        "    \"prediction_ms\": %.1f\n"
        "  },\n"
        "  \"swarm\": {\n"
        "    \"enabled\": true,\n"
        "    \"num_neighbors\": %u,\n"
        "    \"tracks_shared\": %u,\n"
        "    \"events_shared\": %u,\n"
        "    \"network_health\": %.2f\n"
        "  }\n"
        "}\n",
        json_get_timestamp_ms(),
        stats->uptime_ms,
        stats->frames_processed,
        stats->is_running ? "true" : "false",
        stats->perception_stats.avg_fps,
        stats->perception_stats.avg_inference_ms,
        stats->perception_stats.tracked_objects,
        stats->perception_stats.dropped_frames,
        stats->timeline_stats.active_timelines,
        stats->timeline_stats.total_events_predicted,
        stats->timeline_stats.total_interventions,
        stats->timeline_stats.avg_prediction_ms,
        stats->swarm_stats.num_neighbors,
        stats->swarm_stats.tracks_shared,
        stats->swarm_stats.events_shared,
        stats->swarm_stats.network_health
    );

    return write_json_atomic(JSON_EXPORT_STATS, json);
}

/**
 * Export system status to JSON
 */
bool json_export_status(const char* status, uint64_t uptime_ms) {
    if (!status) return false;

    char json[1024];
    snprintf(json, sizeof(json),
        "{\n"
        "  \"timestamp\": %llu,\n"
        "  \"status\": \"%s\",\n"
        "  \"uptime_ms\": %llu,\n"
        "  \"version\": \"0.2.0\",\n"
        "  \"pid\": %d\n"
        "}\n",
        json_get_timestamp_ms(),
        status,
        uptime_ms,
        getpid()
    );

    return write_json_atomic(JSON_EXPORT_STATUS, json);
}

/**
 * Convert object class to string
 */
static const char* class_to_string(ObjectClass class_id) {
    switch (class_id) {
        case OBJECT_CLASS_PERSON: return "person";
        case OBJECT_CLASS_VEHICLE: return "vehicle";
        case OBJECT_CLASS_FACE: return "face";
        case OBJECT_CLASS_ANIMAL: return "animal";
        case OBJECT_CLASS_PACKAGE: return "package";
        default: return "unknown";
    }
}

/**
 * Convert behavior flags to JSON array
 */
static void behaviors_to_json(uint32_t behaviors, char* out, size_t size) {
    out[0] = '\0';
    bool first = true;

    if (behaviors & BEHAVIOR_LOITERING) {
        strncat(out, first ? "\"loitering\"" : ", \"loitering\"", size - strlen(out) - 1);
        first = false;
    }
    if (behaviors & BEHAVIOR_RUNNING) {
        strncat(out, first ? "\"running\"" : ", \"running\"", size - strlen(out) - 1);
        first = false;
    }
    if (behaviors & BEHAVIOR_CONCEALING) {
        strncat(out, first ? "\"concealing\"" : ", \"concealing\"", size - strlen(out) - 1);
        first = false;
    }
    if (behaviors & BEHAVIOR_SUSPICIOUS_MOVEMENT) {
        strncat(out, first ? "\"suspicious_movement\"" : ", \"suspicious_movement\"", size - strlen(out) - 1);
        first = false;
    }
    if (behaviors & BEHAVIOR_REPEATED_PASSES) {
        strncat(out, first ? "\"repeated_passes\"" : ", \"repeated_passes\"", size - strlen(out) - 1);
        first = false;
    }
    if (behaviors & BEHAVIOR_EXTENDED_OBSERVATION) {
        strncat(out, first ? "\"extended_observation\"" : ", \"extended_observation\"", size - strlen(out) - 1);
        first = false;
    }
}

/**
 * Export detections to JSON
 */
bool json_export_detections(
    const DetectedObject* objects,
    uint32_t num_objects,
    uint64_t frame_id) {

    if (!objects) return false;

    char json[16384];  // 16KB buffer
    int offset = 0;

    offset += snprintf(json + offset, sizeof(json) - offset,
        "{\n"
        "  \"timestamp\": %llu,\n"
        "  \"frame_id\": %llu,\n"
        "  \"num_detections\": %u,\n"
        "  \"detections\": [\n",
        json_get_timestamp_ms(),
        frame_id,
        num_objects
    );

    for (uint32_t i = 0; i < num_objects && i < 50; i++) {
        const DetectedObject* obj = &objects[i];

        offset += snprintf(json + offset, sizeof(json) - offset,
            "    {\n"
            "      \"id\": %u,\n"
            "      \"class\": \"%s\",\n"
            "      \"confidence\": %.2f,\n"
            "      \"bbox\": [%.1f, %.1f, %.1f, %.1f],\n"
            "      \"timestamp_ms\": %llu\n"
            "    }%s\n",
            obj->id,
            class_to_string(obj->class_id),
            obj->confidence,
            obj->bbox.x, obj->bbox.y, obj->bbox.width, obj->bbox.height,
            obj->timestamp_ms,
            (i < num_objects - 1) ? "," : ""
        );
    }

    offset += snprintf(json + offset, sizeof(json) - offset,
        "  ]\n"
        "}\n"
    );

    return write_json_atomic(JSON_EXPORT_DETECTIONS, json);
}

/**
 * Export tracked objects to JSON
 */
bool json_export_tracks(
    const TrackedObject* tracks,
    uint32_t num_tracks) {

    if (!tracks) return false;

    char json[16384];  // 16KB buffer
    int offset = 0;

    offset += snprintf(json + offset, sizeof(json) - offset,
        "{\n"
        "  \"timestamp\": %llu,\n"
        "  \"num_tracks\": %u,\n"
        "  \"tracks\": [\n",
        json_get_timestamp_ms(),
        num_tracks
    );

    for (uint32_t i = 0; i < num_tracks && i < 50; i++) {
        const TrackedObject* track = &tracks[i];

        char behaviors_json[256];
        behaviors_to_json(track->behaviors, behaviors_json, sizeof(behaviors_json));

        offset += snprintf(json + offset, sizeof(json) - offset,
            "    {\n"
            "      \"track_id\": %u,\n"
            "      \"class\": \"%s\",\n"
            "      \"confidence\": %.2f,\n"
            "      \"bbox\": [%.1f, %.1f, %.1f, %.1f],\n"
            "      \"velocity\": [%.2f, %.2f],\n"
            "      \"behaviors\": [%s],\n"
            "      \"threat_score\": %.2f,\n"
            "      \"first_seen_ms\": %llu,\n"
            "      \"last_seen_ms\": %llu\n"
            "    }%s\n",
            track->track_id,
            class_to_string(track->class_id),
            track->confidence,
            track->current_bbox.x, track->current_bbox.y, track->current_bbox.width, track->current_bbox.height,
            track->velocity_x, track->velocity_y,
            behaviors_json,
            track->threat_score,
            track->first_seen_ms,
            track->last_seen_ms,
            (i < num_tracks - 1) ? "," : ""
        );
    }

    offset += snprintf(json + offset, sizeof(json) - offset,
        "  ]\n"
        "}\n"
    );

    return write_json_atomic(JSON_EXPORT_TRACKS, json);
}

/**
 * Convert event type to string
 */
static const char* event_type_to_string(EventType type) {
    switch (type) {
        case EVENT_THEFT: return "theft";
        case EVENT_TRESPASSING: return "trespassing";
        case EVENT_SUSPICIOUS_BEHAVIOR: return "suspicious_behavior";
        case EVENT_LOITERING: return "loitering";
        case EVENT_ASSAULT: return "assault";
        case EVENT_VANDALISM: return "vandalism";
        case EVENT_FALL: return "fall";
        case EVENT_COLLISION: return "collision";
        case EVENT_CROWD_FORMATION: return "crowd_formation";
        case EVENT_ABANDONED_OBJECT: return "abandoned_object";
        default: return "unknown";
    }
}

/**
 * Convert severity to string
 */
static const char* severity_to_string(SeverityLevel level) {
    switch (level) {
        case SEVERITY_LOW: return "low";
        case SEVERITY_MEDIUM: return "medium";
        case SEVERITY_HIGH: return "high";
        case SEVERITY_CRITICAL: return "critical";
        default: return "unknown";
    }
}

/**
 * Export timeline predictions to JSON
 */
bool json_export_timelines(
    const Timeline* timelines,
    uint32_t num_timelines) {

    if (!timelines) return false;

    char json[32768];  // 32KB buffer
    int offset = 0;

    offset += snprintf(json + offset, sizeof(json) - offset,
        "{\n"
        "  \"timestamp\": %llu,\n"
        "  \"num_timelines\": %u,\n"
        "  \"timelines\": [\n",
        json_get_timestamp_ms(),
        num_timelines
    );

    for (uint32_t i = 0; i < num_timelines && i < 5; i++) {
        const Timeline* tl = &timelines[i];

        offset += snprintf(json + offset, sizeof(json) - offset,
            "    {\n"
            "      \"timeline_id\": %u,\n"
            "      \"probability\": %.2f,\n"
            "      \"num_events\": %u,\n"
            "      \"events\": [\n",
            tl->timeline_id,
            tl->overall_probability,
            tl->num_predicted_events
        );

        // Export events
        for (uint32_t j = 0; j < tl->num_predicted_events && j < 20; j++) {
            const PredictedEvent* ev = &tl->predicted_events[j];

            offset += snprintf(json + offset, sizeof(json) - offset,
                "        {\n"
                "          \"type\": \"%s\",\n"
                "          \"severity\": \"%s\",\n"
                "          \"probability\": %.2f,\n"
                "          \"time_until_ms\": %llu,\n"
                "          \"description\": \"%s\"\n"
                "        }%s\n",
                event_type_to_string(ev->type),
                severity_to_string(ev->severity),
                ev->probability,
                ev->timestamp_ms,
                ev->description,
                (j < tl->num_predicted_events - 1) ? "," : ""
            );
        }

        offset += snprintf(json + offset, sizeof(json) - offset,
            "      ],\n"
            "      \"num_interventions\": %u,\n"
            "      \"interventions\": [\n",
            tl->num_interventions
        );

        // Export interventions
        for (uint32_t j = 0; j < tl->num_interventions && j < 10; j++) {
            const InterventionPoint* iv = &tl->interventions[j];

            offset += snprintf(json + offset, sizeof(json) - offset,
                "        {\n"
                "          \"timestamp_ms\": %llu,\n"
                "          \"effectiveness\": %.2f,\n"
                "          \"recommendation\": \"%s\"\n"
                "        }%s\n",
                iv->timestamp_ms,
                iv->effectiveness,
                iv->recommendation,
                (j < tl->num_interventions - 1) ? "," : ""
            );
        }

        offset += snprintf(json + offset, sizeof(json) - offset,
            "      ]\n"
            "    }%s\n",
            (i < num_timelines - 1) ? "," : ""
        );
    }

    offset += snprintf(json + offset, sizeof(json) - offset,
        "  ]\n"
        "}\n"
    );

    return write_json_atomic(JSON_EXPORT_TIMELINES, json);
}
