"""
Timeline API Blueprint
Handles Timeline Threading predictions and history
"""

from flask import Blueprint, jsonify
from datetime import datetime

timeline_bp = Blueprint('timeline', __name__)


@timeline_bp.route('/predictions', methods=['GET'])
def get_predictions():
  """
  Returns active timeline predictions with probable future events

  Returns:
    JSON with active timeline count and prediction details
  """
  return jsonify({
    "active_timelines": 3,
    "predictions": [
      {
        "timeline_id": "TL001",
        "probability": 0.75,
        "horizon_seconds": 300,
        "events": [
          {
            "type": "TRESPASSING",
            "severity": "medium",
            "time_offset": 45,
            "location": [500, 600],
            "confidence": 0.82
          },
          {
            "type": "LOITERING",
            "severity": "low",
            "time_offset": 120,
            "location": [520, 610],
            "confidence": 0.68
          }
        ]
      },
      {
        "timeline_id": "TL002",
        "probability": 0.60,
        "horizon_seconds": 180,
        "events": [
          {
            "type": "NORMAL_ACTIVITY",
            "severity": "none",
            "time_offset": 30,
            "location": [600, 250],
            "confidence": 0.90
          }
        ]
      },
      {
        "timeline_id": "TL003",
        "probability": 0.40,
        "horizon_seconds": 240,
        "events": [
          {
            "type": "VEHICLE_STOPPED",
            "severity": "low",
            "time_offset": 60,
            "location": [400, 300],
            "confidence": 0.55
          }
        ]
      }
    ]
  })


@timeline_bp.route('/history', methods=['GET'])
def get_history():
  """
  Returns past events and their outcomes

  Returns:
    JSON with historical events and validation data
  """
  return jsonify({
    "total_events": 156,
    "time_range": {
      "start": "2025-10-22T00:00:00Z",
      "end": datetime.utcnow().isoformat() + "Z"
    },
    "events": [
      {
        "event_id": "E001",
        "timestamp": "2025-10-22T14:30:00Z",
        "predicted_type": "TRESPASSING",
        "actual_type": "TRESPASSING",
        "prediction_accuracy": 0.95,
        "timeline_id": "TL045",
        "severity": "medium"
      },
      {
        "event_id": "E002",
        "timestamp": "2025-10-22T13:15:00Z",
        "predicted_type": "NORMAL_ACTIVITY",
        "actual_type": "NORMAL_ACTIVITY",
        "prediction_accuracy": 0.88,
        "timeline_id": "TL044",
        "severity": "none"
      },
      {
        "event_id": "E003",
        "timestamp": "2025-10-22T12:00:00Z",
        "predicted_type": "LOITERING",
        "actual_type": "VEHICLE_STOPPED",
        "prediction_accuracy": 0.42,
        "timeline_id": "TL043",
        "severity": "low"
      }
    ]
  })
