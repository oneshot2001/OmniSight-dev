/**
 * @file json_export.h
 * @brief JSON Export Utilities for IPC
 *
 * Simple JSON file-based IPC for Phase 2.
 * Exports OMNISIGHT data to JSON files for Flask API consumption.
 */

#ifndef OMNISIGHT_JSON_EXPORT_H
#define OMNISIGHT_JSON_EXPORT_H

#include "../omnisight_core.h"
#include "../perception/perception.h"
#include "../timeline/timeline.h"
#include "../swarm/swarm.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * JSON file paths
 */
#define JSON_EXPORT_STATS "/tmp/omnisight_stats.json"
#define JSON_EXPORT_STATUS "/tmp/omnisight_status.json"
#define JSON_EXPORT_DETECTIONS "/tmp/omnisight_detections.json"
#define JSON_EXPORT_TRACKS "/tmp/omnisight_tracks.json"
#define JSON_EXPORT_TIMELINES "/tmp/omnisight_timelines.json"

/**
 * Export system statistics to JSON
 *
 * @param stats System statistics
 * @return true on success
 */
bool json_export_stats(const OmnisightStats* stats);

/**
 * Export system status to JSON
 *
 * @param status Status string ("running", "stopped", "error")
 * @param uptime_ms System uptime in milliseconds
 * @return true on success
 */
bool json_export_status(const char* status, uint64_t uptime_ms);

/**
 * Export detections to JSON
 *
 * @param objects Array of detected objects
 * @param num_objects Number of objects
 * @param frame_id Frame ID
 * @return true on success
 */
bool json_export_detections(
    const DetectedObject* objects,
    uint32_t num_objects,
    uint64_t frame_id
);

/**
 * Export tracked objects to JSON
 *
 * @param tracks Array of tracked objects
 * @param num_tracks Number of tracks
 * @return true on success
 */
bool json_export_tracks(
    const TrackedObject* tracks,
    uint32_t num_tracks
);

/**
 * Export timeline predictions to JSON
 *
 * @param timelines Array of timelines
 * @param num_timelines Number of timelines
 * @return true on success
 */
bool json_export_timelines(
    const Timeline* timelines,
    uint32_t num_timelines
);

/**
 * Get current timestamp in milliseconds
 *
 * @return Milliseconds since epoch
 */
uint64_t json_get_timestamp_ms(void);

#ifdef __cplusplus
}
#endif

#endif // OMNISIGHT_JSON_EXPORT_H
