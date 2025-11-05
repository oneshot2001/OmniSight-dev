"""
Perception API Blueprint
Handles object detection and tracking endpoints
"""

from flask import Blueprint, jsonify, current_app

perception_bp = Blueprint('perception', __name__)


@perception_bp.route('/status', methods=['GET'])
def get_perception_status():
  """
  Returns perception module status and performance metrics

  Returns:
    JSON with enabled status, FPS, resolution, and object count
  """
  ipc = current_app.config['IPC_CLIENT']
  stats = ipc.get_stats()

  if not stats:
    return jsonify({
      "error": "Stats not available",
      "message": "C core may not be running"
    }), 503

  perception_stats = stats.get('perception', {})
  return jsonify({
    "enabled": True,
    "fps": perception_stats.get('fps', 0),
    "resolution": "1920x1080",
    "objects_tracked": perception_stats.get('objects_tracked', 0),
    "inference_ms": perception_stats.get('inference_ms', 0),
    "dropped_frames": perception_stats.get('dropped_frames', 0),
    "module": "perception"
  })


@perception_bp.route('/detections', methods=['GET'])
def get_detections():
  """
  Returns current frame detections with bounding boxes

  Returns:
    JSON with frame ID, timestamp, and list of detections
  """
  ipc = current_app.config['IPC_CLIENT']
  data = ipc.get_detections()

  if not data:
    return jsonify({
      "error": "Detections not available",
      "message": "No detection data from C core"
    }), 503

  return jsonify(data)


@perception_bp.route('/tracks', methods=['GET'])
def get_tracks():
  """
  Returns currently tracked objects

  Returns:
    JSON with timestamp and list of tracked objects
  """
  ipc = current_app.config['IPC_CLIENT']
  data = ipc.get_tracks()

  if not data:
    return jsonify({
      "error": "Tracks not available",
      "message": "No tracking data from C core"
    }), 503

  return jsonify(data)
