"""
Claude Flow API Endpoints

Phase 4: Enhanced distributed AI coordination endpoints.
"""

from flask import Blueprint, jsonify, request, current_app
import logging
import time

# Create blueprint
claude_flow_bp = Blueprint('claude_flow', __name__)

logger = logging.getLogger(__name__)


@claude_flow_bp.route('/status')
def get_status():
  """
  Get Claude Flow integration status.

  Returns:
    JSON with coordination status and health metrics
  """
  claude_flow_available = current_app.config.get('CLAUDE_FLOW_AVAILABLE', False)

  if not claude_flow_available:
    return jsonify({
      "enabled": False,
      "message": "Claude Flow not available - using fallback coordination"
    })

  try:
    swarm = current_app.config.get('SWARM_COORDINATOR')
    perf = current_app.config.get('PERFORMANCE_MONITOR')

    return jsonify({
      "enabled": True,
      "swarm": swarm.get_network_status() if swarm else {},
      "performance": perf.get_all_statistics() if perf else {},
      "timestamp": time.time()
    })

  except Exception as e:
    logger.error(f"Error getting Claude Flow status: {e}")
    return jsonify({"error": str(e)}), 500


@claude_flow_bp.route('/swarm/coordinate', methods=['POST'])
def coordinate_detection():
  """
  Coordinate detection event across camera network.

  Request Body:
    {
      "camera_id": "cam1",
      "detection_data": {
        "timestamp": 1234567890.0,
        "detections": [...],
        "threat_scores": [0.8, 0.3],
        "confidence": 0.85
      }
    }

  Returns:
    Consensus result from swarm
  """
  if not current_app.config.get('CLAUDE_FLOW_AVAILABLE'):
    return jsonify({"error": "Claude Flow not available"}), 503

  try:
    data = request.get_json()
    camera_id = data.get('camera_id')
    detection_data = data.get('detection_data', {})

    swarm = current_app.config.get('SWARM_COORDINATOR')
    if not swarm:
      return jsonify({"error": "Swarm coordinator not initialized"}), 500

    # Coordinate detection
    consensus = swarm.coordinate_detection(camera_id, detection_data)

    return jsonify({
      "success": True,
      "consensus": consensus,
      "timestamp": time.time()
    })

  except Exception as e:
    logger.error(f"Error coordinating detection: {e}")
    return jsonify({"error": str(e)}), 500


@claude_flow_bp.route('/timeline/predict', methods=['POST'])
def predict_timeline():
  """
  Generate neural timeline predictions.

  Request Body:
    {
      "current_state": {
        "timestamp": 1234567890.0,
        "detections": [...],
        "tracks": [...],
        "threat_level": 0.8
      },
      "historical_context": [...]
    }

  Returns:
    List of probable future timelines
  """
  if not current_app.config.get('CLAUDE_FLOW_AVAILABLE'):
    return jsonify({"error": "Claude Flow not available"}), 503

  try:
    data = request.get_json()
    current_state = data.get('current_state', {})
    historical_context = data.get('historical_context', [])

    timeline = current_app.config.get('TIMELINE_ENGINE')
    if not timeline:
      return jsonify({"error": "Timeline engine not initialized"}), 500

    # Predict futures
    predictions = timeline.predict_futures(current_state, historical_context)

    # Convert to JSON-serializable format
    predictions_json = [
      {
        "future_id": p.future_id,
        "probability": p.probability,
        "timeline_events": p.timeline_events,
        "confidence_score": p.confidence_score,
        "intervention_points": p.intervention_points,
        "timestamp": p.timestamp
      }
      for p in predictions
    ]

    return jsonify({
      "success": True,
      "predictions": predictions_json,
      "count": len(predictions),
      "timestamp": time.time()
    })

  except Exception as e:
    logger.error(f"Error predicting timeline: {e}")
    return jsonify({"error": str(e)}), 500


@claude_flow_bp.route('/federated/train', methods=['POST'])
def train_federated():
  """
  Train federated learning model on local data.

  Request Body:
    {
      "camera_id": "cam1",
      "model_name": "detector",
      "local_data": [...]
    }

  Returns:
    Training result with encrypted gradients
  """
  if not current_app.config.get('CLAUDE_FLOW_AVAILABLE'):
    return jsonify({"error": "Claude Flow not available"}), 503

  try:
    data = request.get_json()
    camera_id = data.get('camera_id')
    model_name = data.get('model_name')
    local_data = data.get('local_data', [])

    federated = current_app.config.get('FEDERATED_COORDINATOR')
    if not federated:
      return jsonify({"error": "Federated coordinator not initialized"}), 500

    # Train local model
    update = federated.train_local_model(camera_id, model_name, local_data)

    if update:
      return jsonify({
        "success": True,
        "update": {
          "camera_id": update.camera_id,
          "model_name": update.model_name,
          "gradients_hash": update.gradients_hash,
          "sample_count": update.sample_count,
          "timestamp": update.timestamp,
          "encrypted": update.encrypted
        }
      })
    else:
      return jsonify({"success": False, "error": "Training failed"}), 500

  except Exception as e:
    logger.error(f"Error training federated model: {e}")
    return jsonify({"error": str(e)}), 500


@claude_flow_bp.route('/federated/aggregate', methods=['POST'])
def aggregate_federated():
  """
  Aggregate federated learning updates across cameras.

  Request Body:
    {
      "model_name": "detector",
      "min_cameras": 2
    }

  Returns:
    Aggregation result with consensus
  """
  if not current_app.config.get('CLAUDE_FLOW_AVAILABLE'):
    return jsonify({"error": "Claude Flow not available"}), 503

  try:
    data = request.get_json()
    model_name = data.get('model_name')
    min_cameras = data.get('min_cameras')

    federated = current_app.config.get('FEDERATED_COORDINATOR')
    if not federated:
      return jsonify({"error": "Federated coordinator not initialized"}), 500

    # Aggregate global model
    result = federated.aggregate_global_model(model_name, min_cameras)

    return jsonify(result)

  except Exception as e:
    logger.error(f"Error aggregating federated model: {e}")
    return jsonify({"error": str(e)}), 500


@claude_flow_bp.route('/federated/status')
def get_federated_status():
  """Get federated learning status."""
  if not current_app.config.get('CLAUDE_FLOW_AVAILABLE'):
    return jsonify({"error": "Claude Flow not available"}), 503

  try:
    federated = current_app.config.get('FEDERATED_COORDINATOR')
    if not federated:
      return jsonify({"error": "Federated coordinator not initialized"}), 500

    status = federated.get_model_status()

    return jsonify({
      "success": True,
      "status": status,
      "timestamp": time.time()
    })

  except Exception as e:
    logger.error(f"Error getting federated status: {e}")
    return jsonify({"error": str(e)}), 500


@claude_flow_bp.route('/performance/metrics')
def get_performance_metrics():
  """Get current performance metrics."""
  if not current_app.config.get('CLAUDE_FLOW_AVAILABLE'):
    return jsonify({"error": "Claude Flow not available"}), 503

  try:
    perf = current_app.config.get('PERFORMANCE_MONITOR')
    if not perf:
      return jsonify({"error": "Performance monitor not initialized"}), 500

    stats = perf.get_all_statistics()

    return jsonify({
      "success": True,
      "metrics": stats,
      "timestamp": time.time()
    })

  except Exception as e:
    logger.error(f"Error getting performance metrics: {e}")
    return jsonify({"error": str(e)}), 500


@claude_flow_bp.route('/performance/benchmark', methods=['POST'])
def run_benchmark():
  """Run performance benchmark."""
  if not current_app.config.get('CLAUDE_FLOW_AVAILABLE'):
    return jsonify({"error": "Claude Flow not available"}), 503

  try:
    perf = current_app.config.get('PERFORMANCE_MONITOR')
    if not perf:
      return jsonify({"error": "Performance monitor not initialized"}), 500

    results = perf.run_benchmark()

    return jsonify({
      "success": True,
      "benchmark": results,
      "timestamp": time.time()
    })

  except Exception as e:
    logger.error(f"Error running benchmark: {e}")
    return jsonify({"error": str(e)}), 500


@claude_flow_bp.route('/performance/optimize', methods=['POST'])
def optimize_performance():
  """Automatically optimize performance."""
  if not current_app.config.get('CLAUDE_FLOW_AVAILABLE'):
    return jsonify({"error": "Claude Flow not available"}), 503

  try:
    perf = current_app.config.get('PERFORMANCE_MONITOR')
    if not perf:
      return jsonify({"error": "Performance monitor not initialized"}), 500

    optimizations = perf.optimize_performance()

    return jsonify({
      "success": True,
      "optimizations": optimizations,
      "count": len(optimizations),
      "timestamp": time.time()
    })

  except Exception as e:
    logger.error(f"Error optimizing performance: {e}")
    return jsonify({"error": str(e)}), 500


@claude_flow_bp.route('/performance/bottlenecks')
def detect_bottlenecks():
  """Detect performance bottlenecks."""
  if not current_app.config.get('CLAUDE_FLOW_AVAILABLE'):
    return jsonify({"error": "Claude Flow not available"}), 503

  try:
    perf = current_app.config.get('PERFORMANCE_MONITOR')
    if not perf:
      return jsonify({"error": "Performance monitor not initialized"}), 500

    bottlenecks = perf.detect_bottlenecks()

    return jsonify({
      "success": True,
      "bottlenecks": bottlenecks,
      "count": len(bottlenecks),
      "timestamp": time.time()
    })

  except Exception as e:
    logger.error(f"Error detecting bottlenecks: {e}")
    return jsonify({"error": str(e)}), 500


@claude_flow_bp.route('/performance/report')
def get_performance_report():
  """Get comprehensive performance report."""
  if not current_app.config.get('CLAUDE_FLOW_AVAILABLE'):
    return jsonify({"error": "Claude Flow not available"}), 503

  try:
    perf = current_app.config.get('PERFORMANCE_MONITOR')
    if not perf:
      return jsonify({"error": "Performance monitor not initialized"}), 500

    report = perf.get_performance_report()

    return jsonify({
      "success": True,
      "report": report,
      "timestamp": time.time()
    })

  except Exception as e:
    logger.error(f"Error generating performance report: {e}")
    return jsonify({"error": str(e)}), 500
