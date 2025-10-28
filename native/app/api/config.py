"""
Configuration API Blueprint
Handles system configuration get/update endpoints
"""

from flask import Blueprint, jsonify, request

config_bp = Blueprint('config', __name__)

# In-memory config store (will be replaced with file-based or IPC in Phase 2)
current_config = {
  "perception": {
    "enabled": True,
    "fps_target": 10,
    "resolution": "1920x1080",
    "confidence_threshold": 0.7
  },
  "timeline": {
    "enabled": True,
    "horizon_seconds": 300,
    "max_timelines": 5,
    "prediction_threshold": 0.5
  },
  "swarm": {
    "enabled": True,
    "mqtt_broker": "localhost",
    "mqtt_port": 1883,
    "discovery_enabled": True
  },
  "system": {
    "log_level": "INFO",
    "max_memory_mb": 256
  }
}


@config_bp.route('', methods=['GET'])
def get_config():
  """
  Returns current system configuration

  Returns:
    JSON with complete configuration
  """
  return jsonify(current_config)


@config_bp.route('', methods=['POST'])
def update_config():
  """
  Updates system configuration

  Expects:
    JSON with configuration updates (partial or complete)

  Returns:
    JSON with updated configuration and status
  """
  try:
    updates = request.get_json()

    if not updates:
      return jsonify({
        "success": False,
        "error": "No configuration data provided"
      }), 400

    # Deep merge updates into current config
    for section, values in updates.items():
      if section in current_config and isinstance(values, dict):
        current_config[section].update(values)
      else:
        current_config[section] = values

    return jsonify({
      "success": True,
      "message": "Configuration updated successfully",
      "config": current_config
    })

  except Exception as e:
    return jsonify({
      "success": False,
      "error": str(e)
    }), 500
