"""
Swarm API Blueprint
Handles swarm intelligence and multi-camera networking
"""

from flask import Blueprint, jsonify

swarm_bp = Blueprint('swarm', __name__)


@swarm_bp.route('/network', methods=['GET'])
def get_network():
  """
  Returns swarm network topology and health status

  Returns:
    JSON with local camera ID, neighbors, and network health
  """
  return jsonify({
    "local_camera_id": "CAM001",
    "neighbors": [
      {
        "id": "CAM002",
        "distance": 25.5,
        "signal_strength": -45,
        "shared_tracks": 2,
        "last_seen": "2025-10-22T16:00:00Z",
        "status": "online"
      },
      {
        "id": "CAM003",
        "distance": 42.8,
        "signal_strength": -62,
        "shared_tracks": 1,
        "last_seen": "2025-10-22T16:00:02Z",
        "status": "online"
      }
    ],
    "network_health": "good",
    "total_cameras": 3,
    "mqtt_connected": True
  })


@swarm_bp.route('/cameras', methods=['GET'])
def get_cameras():
  """
  Returns list of all cameras in the swarm network

  Returns:
    JSON with array of camera details
  """
  return jsonify({
    "cameras": [
      {
        "id": "CAM001",
        "name": "Front Entrance",
        "location": [0, 0],
        "status": "online",
        "role": "master",
        "uptime_seconds": 86400
      },
      {
        "id": "CAM002",
        "name": "Parking Lot",
        "location": [25.5, 0],
        "status": "online",
        "role": "worker",
        "uptime_seconds": 82800
      },
      {
        "id": "CAM003",
        "name": "Back Entrance",
        "location": [0, 42.8],
        "status": "online",
        "role": "worker",
        "uptime_seconds": 79200
      }
    ],
    "total_count": 3
  })
