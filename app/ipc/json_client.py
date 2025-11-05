"""
OMNISIGHT JSON-based IPC Client

Simple JSON file-based IPC for Phase 2.
Reads data exported by C core from /tmp/omnisight_*.json files.
"""

import json
import os
import time
import logging
from typing import Optional, Dict

logger = logging.getLogger("omnisight.ipc.json")


class OmnisightJSONClient:
    """
    Client for reading OMNISIGHT C core data via JSON files.

    Usage:
        client = OmnisightJSONClient()
        detections = client.get_detections()
        tracks = client.get_tracks()
        timelines = client.get_timelines()
        stats = client.get_stats()
    """

    def __init__(self):
        """Initialize JSON IPC client."""
        self.json_dir = "/tmp"
        self._last_detections = None
        self._last_tracks = None
        self._last_timelines = None
        self._last_stats = None
        self._last_status = None

        logger.info("JSON IPC client initialized")

    def _read_json_file(self, filename: str, max_age_ms: int = 5000) -> Optional[Dict]:
        """
        Read and parse JSON file with freshness check.

        Args:
            filename: Name of JSON file in /tmp/
            max_age_ms: Maximum age in milliseconds before considering stale

        Returns:
            Parsed JSON dict or None if file missing/invalid/stale
        """
        filepath = os.path.join(self.json_dir, filename)

        try:
            # Check if file exists
            if not os.path.exists(filepath):
                logger.debug(f"JSON file not found: {filename}")
                return None

            # Read and parse JSON
            with open(filepath, 'r') as f:
                data = json.load(f)

            # Check data freshness
            timestamp = data.get('timestamp', 0)
            if timestamp == 0:
                logger.warning(f"No timestamp in {filename}")
                return data  # Return anyway, might be valid

            age_ms = time.time() * 1000 - timestamp
            if age_ms > max_age_ms:
                logger.warning(f"{filename} data is stale ({int(age_ms)}ms old)")
                return None

            return data

        except json.JSONDecodeError as e:
            logger.error(f"Invalid JSON in {filename}: {e}")
            return None
        except Exception as e:
            logger.error(f"Error reading {filename}: {e}")
            return None

    def get_detections(self) -> Optional[Dict]:
        """
        Get latest detection frame from JSON file.

        Returns:
            Dict with keys:
                - frame_id: int
                - timestamp: int (ms)
                - num_detections: int
                - detections: List[Dict]
            or cached data if file not available/stale, or None if never received
        """
        data = self._read_json_file("omnisight_detections.json")
        if data:
            self._last_detections = data
        return self._last_detections

    def get_tracks(self) -> Optional[Dict]:
        """
        Get latest tracked objects from JSON file.

        Returns:
            Dict with keys:
                - timestamp: int (ms)
                - num_tracks: int
                - tracks: List[Dict]
            or cached data if file not available/stale, or None if never received
        """
        data = self._read_json_file("omnisight_tracks.json")
        if data:
            self._last_tracks = data
        return self._last_tracks

    def get_timelines(self) -> Optional[Dict]:
        """
        Get current timeline predictions from JSON file.

        Returns:
            Dict with keys:
                - timestamp: int (ms)
                - num_timelines: int
                - timelines: List[Dict]
            or cached data if file not available/stale, or None if never received
        """
        data = self._read_json_file("omnisight_timelines.json")
        if data:
            self._last_timelines = data
        return self._last_timelines

    def get_stats(self) -> Optional[Dict]:
        """
        Get system statistics from JSON file.

        Returns:
            Dict with comprehensive system stats or cached data/None
        """
        data = self._read_json_file("omnisight_stats.json", max_age_ms=10000)  # 10s for stats
        if data:
            self._last_stats = data
        return self._last_stats

    def get_status(self) -> Optional[Dict]:
        """
        Get system status from JSON file.

        Returns:
            Dict with status info or cached data/None
        """
        data = self._read_json_file("omnisight_status.json", max_age_ms=10000)  # 10s for status
        if data:
            self._last_status = data
        return self._last_status

    def health_check(self) -> Dict:
        """
        Check IPC health status.

        Returns:
            Dict with health indicators:
                - json_accessible: bool
                - data_fresh: bool
                - c_core_running: bool
        """
        health = {
            'json_accessible': False,
            'data_fresh': False,
            'c_core_running': False
        }

        # Check if we can read status file
        status = self.get_status()
        if status:
            health['json_accessible'] = True
            health['c_core_running'] = status.get('status') == 'running'

        # Check data freshness
        stats = self.get_stats()
        if stats:
            age_ms = time.time() * 1000 - stats.get('timestamp', 0)
            health['data_fresh'] = age_ms < 5000

        return health

    def close(self):
        """Close client (no-op for JSON-based IPC)."""
        logger.info("JSON IPC client closed")


# Example usage
if __name__ == "__main__":
    logging.basicConfig(level=logging.INFO)

    client = OmnisightJSONClient()

    # Test reading
    print("\n=== Testing JSON IPC Client ===\n")

    detections = client.get_detections()
    print(f"Detections: {json.dumps(detections, indent=2) if detections else 'None'}\n")

    tracks = client.get_tracks()
    print(f"Tracks: {json.dumps(tracks, indent=2) if tracks else 'None'}\n")

    timelines = client.get_timelines()
    print(f"Timelines: {json.dumps(timelines, indent=2) if timelines else 'None'}\n")

    stats = client.get_stats()
    print(f"Stats: {json.dumps(stats, indent=2) if stats else 'None'}\n")

    status = client.get_status()
    print(f"Status: {json.dumps(status, indent=2) if status else 'None'}\n")

    # Test health check
    health = client.health_check()
    print(f"Health: {json.dumps(health, indent=2)}\n")

    client.close()
