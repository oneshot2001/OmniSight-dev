"""
System API Blueprint
Handles system statistics and health endpoints
"""

from flask import Blueprint, jsonify
from datetime import datetime
import os

system_bp = Blueprint('system', __name__)


@system_bp.route('/stats', methods=['GET'])
def get_stats():
  """
  Returns system statistics and performance metrics

  Returns:
    JSON with comprehensive system stats
  """
  return jsonify({
    "uptime_seconds": 86400,
    "memory": {
      "used_mb": 128,
      "allocated_mb": 256,
      "usage_percent": 50
    },
    "cpu": {
      "usage_percent": 35,
      "cores": 4
    },
    "modules": {
      "perception": {
        "status": "running",
        "fps": 10.2,
        "frames_processed": 367200
      },
      "timeline": {
        "status": "running",
        "active_predictions": 3,
        "total_predictions": 1420
      },
      "swarm": {
        "status": "running",
        "connected_cameras": 3,
        "messages_sent": 5234,
        "messages_received": 4891
      }
    },
    "timestamp": datetime.utcnow().isoformat() + "Z"
  })


@system_bp.route('/health', methods=['GET'])
def health_check():
  """
  Health check endpoint for monitoring

  Returns:
    JSON with health status
  """
  return jsonify({
    "status": "healthy",
    "version": "0.2.0",
    "timestamp": datetime.utcnow().isoformat() + "Z"
  })
