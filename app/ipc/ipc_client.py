"""
OMNISIGHT IPC Client - Python side of inter-process communication

This module provides a Python interface to read data from the C core via IPC.
It uses:
- Shared memory (mmap + posix_ipc) for high-frequency data
- Named pipes for sending commands
- JSON files for reading status/stats

Performance targets:
- Read latency: <1ms for shared memory
- Command latency: <100ms

See docs/IPC_ARCHITECTURE.md for complete specification.
"""

import mmap
import os
import struct
import json
import time
import logging
from typing import Optional, Dict, List, Tuple
from dataclasses import dataclass

try:
    import posix_ipc
    HAS_POSIX_IPC = True
except ImportError:
    HAS_POSIX_IPC = False
    logging.warning("posix_ipc not available, shared memory IPC disabled")

# IPC paths (must match C side in ipc_server.h)
SHM_DETECTIONS = "/omnisight_detections"
SHM_TRACKS = "/omnisight_tracks"
SHM_TIMELINES = "/omnisight_timelines"

SEM_FRAME_READY = "/omnisight_frame_ready"
SEM_TRACKS_READY = "/omnisight_tracks_ready"
SEM_TIMELINES_READY = "/omnisight_timelines_ready"

PIPE_CMD = "/tmp/omnisight_cmd"
PIPE_ACK = "/tmp/omnisight_cmd_ack"

JSON_STATS = "/tmp/omnisight_stats.json"
JSON_STATUS = "/tmp/omnisight_status.json"

# Protocol constants
OMNISIGHT_MAGIC = 0x4F4D4E49  # "OMNI"
OMNISIGHT_VERSION = 1


@dataclass
class Detection:
    """Single detected object."""
    id: int
    class_id: int
    confidence: float
    bbox: Tuple[float, float, float, float]  # (x, y, width, height)
    timestamp_ms: int


@dataclass
class TrackedObject:
    """Tracked object across frames."""
    track_id: int
    class_id: int
    bbox: Tuple[float, float, float, float]
    velocity: Tuple[float, float]
    confidence: float
    behaviors: int
    threat_score: float
    first_seen_ms: int
    last_seen_ms: int


@dataclass
class Timeline:
    """Timeline prediction snapshot."""
    timeline_id: int
    probability: float
    prediction_start_ms: int
    prediction_end_ms: int
    events: List[Dict]
    interventions: List[Dict]
    worst_case_severity: int
    total_threat_score: float


class OmnisightIPCClient:
    """
    Client for reading OMNISIGHT C core data via IPC.

    Usage:
        client = OmnisightIPCClient()
        detections = client.get_detections()
        tracks = client.get_tracks()
        timelines = client.get_timelines()
        stats = client.get_stats()
    """

    def __init__(self):
        """Initialize IPC client."""
        self.logger = logging.getLogger("omnisight.ipc")

        # Cached data (returned when no new data available)
        self._last_detections = None
        self._last_tracks = None
        self._last_timelines = None

        # TODO Phase 2.1: Initialize shared memory and semaphores
        self._setup_shared_memory()

    def _setup_shared_memory(self):
        """Initialize shared memory and semaphores."""
        if not HAS_POSIX_IPC:
            self.logger.error("posix_ipc not available, IPC disabled")
            return

        # TODO Phase 2.1: Open shared memory regions
        # try:
        #     # Open detection shared memory
        #     self.shm_detections = posix_ipc.SharedMemory(SHM_DETECTIONS)
        #     self.mapfile_detections = mmap.mmap(
        #         self.shm_detections.fd,
        #         self.shm_detections.size
        #     )
        #
        #     # Open tracks shared memory
        #     self.shm_tracks = posix_ipc.SharedMemory(SHM_TRACKS)
        #     self.mapfile_tracks = mmap.mmap(
        #         self.shm_tracks.fd,
        #         self.shm_tracks.size
        #     )
        #
        #     # Open timelines shared memory
        #     self.shm_timelines = posix_ipc.SharedMemory(SHM_TIMELINES)
        #     self.mapfile_timelines = mmap.mmap(
        #         self.shm_timelines.fd,
        #         self.shm_timelines.size
        #     )
        #
        #     # Open semaphores
        #     self.sem_frame_ready = posix_ipc.Semaphore(SEM_FRAME_READY)
        #     self.sem_tracks_ready = posix_ipc.Semaphore(SEM_TRACKS_READY)
        #     self.sem_timelines_ready = posix_ipc.Semaphore(SEM_TIMELINES_READY)
        #
        #     self.logger.info("IPC: Shared memory initialized")
        # except Exception as e:
        #     self.logger.error(f"IPC: Failed to initialize shared memory: {e}")

        self.logger.info("IPC: Shared memory setup (stub)")

    def get_detections(self) -> Optional[Dict]:
        """
        Get latest detection frame.
        Returns cached data if no new frame available.

        Returns:
            Dict with keys:
                - frame_id: int
                - timestamp: int (ms)
                - detections: List[Detection]
            or None if never received data
        """
        # TODO Phase 2.1: Implement detection reading
        # 1. Check for new data (non-blocking semaphore wait)
        #    try:
        #        self.sem_frame_ready.acquire(timeout=0)
        #        new_data = True
        #    except posix_ipc.BusyError:
        #        new_data = False
        #
        # 2. If new data available, read from shared memory
        #    if new_data:
        #        frame = self._read_detection_frame()
        #        if frame and self._validate_checksum(frame):
        #            self._last_detections = frame
        #        else:
        #            self.logger.warning("IPC: Invalid detection frame")
        #
        # 3. Return cached or new data
        #    return self._last_detections

        # Stub: return placeholder
        if self._last_detections is None:
            self._last_detections = {
                'frame_id': 0,
                'timestamp': int(time.time() * 1000),
                'detections': []
            }
        return self._last_detections

    def get_tracks(self) -> Optional[Dict]:
        """
        Get latest tracked objects.
        Returns cached data if no new frame available.

        Returns:
            Dict with keys:
                - timestamp: int (ms)
                - tracks: List[TrackedObject]
            or None if never received data
        """
        # TODO Phase 2.1: Implement track reading
        # Similar to get_detections() but for TrackedObjectsFrame
        # - Check sem_tracks_ready
        # - Read from shm_tracks
        # - Validate checksum
        # - Cache result

        # Stub
        if self._last_tracks is None:
            self._last_tracks = {
                'timestamp': int(time.time() * 1000),
                'tracks': []
            }
        return self._last_tracks

    def get_timelines(self) -> Optional[Dict]:
        """
        Get current timeline predictions.
        Returns cached data if no new predictions available.

        Returns:
            Dict with keys:
                - timestamp: int (ms)
                - timelines: List[Timeline]
            or None if never received data
        """
        # TODO Phase 2.1: Implement timeline reading
        # Similar pattern but for TimelinesFrame
        # - Check sem_timelines_ready
        # - Read from shm_timelines
        # - Parse TimelineSnapshot structures
        # - Convert to Python dict/Timeline objects

        # Stub
        if self._last_timelines is None:
            self._last_timelines = {
                'timestamp': int(time.time() * 1000),
                'timelines': []
            }
        return self._last_timelines

    def _read_detection_frame(self) -> Optional[Dict]:
        """Read and parse DetectionFrame from shared memory."""
        # TODO Phase 2.1: Implement frame parsing
        # 1. Seek to start of shared memory
        #    self.mapfile_detections.seek(0)
        #
        # 2. Read header (32 bytes)
        #    header_data = self.mapfile_detections.read(32)
        #    magic, version, frame_id, timestamp_ms, num_detections = struct.unpack(
        #        'IIQQQ', header_data)
        #
        # 3. Validate magic number
        #    if magic != OMNISIGHT_MAGIC:
        #        return None
        #
        # 4. Read detections array
        #    detections = []
        #    for _ in range(num_detections):
        #        det_data = self.mapfile_detections.read(SIZE_OF_DETECTED_OBJECT)
        #        detection = self._parse_detection(det_data)
        #        detections.append(detection)
        #
        # 5. Return parsed frame
        #    return {
        #        'frame_id': frame_id,
        #        'timestamp': timestamp_ms,
        #        'detections': detections
        #    }

        return None  # Stub

    def _parse_detection(self, data: bytes) -> Detection:
        """Parse DetectedObject struct from bytes."""
        # TODO Phase 2.1: Parse detection struct
        # DetectedObject layout:
        # - uint32_t id
        # - uint32_t class_id
        # - float confidence
        # - BoundingBox bbox (4 floats)
        # - float features[128]
        # - uint64_t timestamp_ms
        #
        # Use struct.unpack() to extract fields
        pass

    def _validate_checksum(self, frame: Dict) -> bool:
        """Validate CRC32 checksum of frame."""
        # TODO Phase 2.1: Implement checksum validation
        # 1. Extract checksum from frame
        # 2. Calculate CRC32 over frame data
        # 3. Compare
        #
        # import zlib
        # expected = frame.get('checksum', 0)
        # if expected == 0:
        #     return True  # Checksums disabled
        # actual = zlib.crc32(frame_bytes) & 0xffffffff
        # return actual == expected

        return True  # Stub: always valid

    def get_stats(self) -> Optional[Dict]:
        """
        Read system statistics from JSON file.

        Returns:
            Dict with stats or None if file not available
        """
        # TODO Phase 2.3: Implement JSON file reading
        # try:
        #     with open(JSON_STATS, 'r') as f:
        #         stats = json.load(f)
        #     return stats
        # except FileNotFoundError:
        #     self.logger.warning("Stats file not found")
        #     return None
        # except json.JSONDecodeError as e:
        #     self.logger.error(f"Invalid stats JSON: {e}")
        #     return None

        # Stub
        return {
            'timestamp': int(time.time() * 1000),
            'uptime_ms': 60000,
            'perception': {
                'enabled': True,
                'fps': 10.0,
                'objects_tracked': 0
            },
            'timeline': {
                'enabled': True,
                'active_timelines': 0
            },
            'swarm': {
                'enabled': True,
                'num_neighbors': 0
            }
        }

    def get_status(self) -> Optional[Dict]:
        """
        Read system status from JSON file.

        Returns:
            Dict with status or None if file not available
        """
        # TODO Phase 2.3: Implement status file reading
        # Similar to get_stats()

        # Stub
        return {
            'timestamp': int(time.time() * 1000),
            'status': 'running',
            'pid': os.getpid(),
            'version': '0.2.0'
        }

    def send_command(self, command_type: str, data: Dict,
                    timeout: float = 1.0) -> bool:
        """
        Send command to C core and wait for acknowledgment.

        Args:
            command_type: Command type string (e.g., 'CONFIG_UPDATE')
            data: Command-specific data dict
            timeout: Acknowledgment timeout in seconds

        Returns:
            True if command succeeded, False otherwise
        """
        # TODO Phase 2.2: Implement command sending
        # import uuid
        #
        # 1. Build command JSON
        #    request_id = str(uuid.uuid4())
        #    command = {
        #        'type': command_type,
        #        'timestamp': int(time.time() * 1000),
        #        'request_id': request_id,
        #        'data': data
        #    }
        #
        # 2. Write to command pipe
        #    try:
        #        with open(PIPE_CMD, 'w') as f:
        #            json.dump(command, f)
        #    except Exception as e:
        #        self.logger.error(f"Failed to send command: {e}")
        #        return False
        #
        # 3. Wait for acknowledgment
        #    start = time.time()
        #    while time.time() - start < timeout:
        #        try:
        #            with open(PIPE_ACK, 'r') as f:
        #                ack = json.load(f)
        #                if ack.get('request_id') == request_id:
        #                    return ack.get('status') == 'success'
        #        except:
        #            pass
        #        time.sleep(0.01)  # 10ms poll
        #
        # 4. Timeout
        #    self.logger.warning(f"Command timeout: {command_type}")
        #    return False

        self.logger.info(f"IPC: Command sent (stub): {command_type}")
        return True

    def health_check(self) -> Dict:
        """
        Check IPC health status.

        Returns:
            Dict with health indicators:
                - shm_accessible: bool
                - data_fresh: bool
                - c_core_responsive: bool
        """
        health = {
            'shm_accessible': False,
            'data_fresh': False,
            'c_core_responsive': False
        }

        # TODO Phase 2.1: Implement health check
        # 1. Try to read detections
        #    detections = self.get_detections()
        #    health['shm_accessible'] = detections is not None
        #
        # 2. Check data freshness (within 5 seconds)
        #    if detections:
        #        age = time.time() * 1000 - detections['timestamp']
        #        health['data_fresh'] = age < 5000
        #
        # 3. Send PING command
        #    health['c_core_responsive'] = self.send_command('PING', {}, timeout=0.5)

        # Stub: assume healthy
        health['shm_accessible'] = True
        health['data_fresh'] = True
        health['c_core_responsive'] = True

        return health

    def close(self):
        """Close all IPC resources."""
        # TODO Phase 2.1: Cleanup resources
        # - Close mmap files
        # - Close shared memory
        # - Close semaphores
        # Don't unlink (C side owns the resources)

        self.logger.info("IPC: Client closed")


# Example usage
if __name__ == "__main__":
    logging.basicConfig(level=logging.INFO)

    client = OmnisightIPCClient()

    # Test reading
    detections = client.get_detections()
    print(f"Detections: {detections}")

    tracks = client.get_tracks()
    print(f"Tracks: {tracks}")

    stats = client.get_stats()
    print(f"Stats: {stats}")

    # Test health check
    health = client.health_check()
    print(f"Health: {health}")

    # Test command
    success = client.send_command('CONFIG_UPDATE', {
        'detection_threshold': 0.7
    })
    print(f"Command success: {success}")

    client.close()
