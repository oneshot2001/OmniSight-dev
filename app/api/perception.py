"""
Perception API Blueprint
Handles object detection and tracking endpoints
"""

from flask import Blueprint, jsonify
from datetime import datetime

perception_bp = Blueprint('perception', __name__)


@perception_bp.route('/status', methods=['GET'])
def get_perception_status():
  """
  Returns perception module status and performance metrics

  Returns:
    JSON with enabled status, FPS, resolution, and object count
  """
  return jsonify({
    "enabled": True,
    "fps": 10.2,
    "resolution": "1920x1080",
    "objects_tracked": 3,
    "module": "perception",
    "last_update": datetime.utcnow().isoformat() + "Z"
  })


@perception_bp.route('/detections', methods=['GET'])
def get_detections():
  """
  Returns current frame detections with bounding boxes

  Returns:
    JSON with frame ID, timestamp, and list of detections
  """
  return jsonify({
    "frame_id": 12345,
    "timestamp": datetime.utcnow().isoformat() + "Z",
    "detections": [
      {
        "id": 1,
        "type": "person",
        "bbox": [100, 200, 50, 150],
        "confidence": 0.95,
        "track_id": "T001"
      },
      {
        "id": 2,
        "type": "vehicle",
        "bbox": [400, 300, 120, 80],
        "confidence": 0.87,
        "track_id": "T002"
      },
      {
        "id": 3,
        "type": "person",
        "bbox": [600, 250, 45, 140],
        "confidence": 0.92,
        "track_id": "T003"
      }
    ]
  })
