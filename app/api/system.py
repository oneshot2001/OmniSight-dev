"""
System API Blueprint
Handles system statistics and health endpoints
"""

from flask import Blueprint, jsonify, current_app

system_bp = Blueprint('system', __name__)


@system_bp.route('/stats', methods=['GET'])
def get_stats():
  """
  Returns system statistics and performance metrics

  Returns:
    JSON with comprehensive system stats
  """
  ipc = current_app.config['IPC_CLIENT']
  data = ipc.get_stats()

  if not data:
    return jsonify({
      "error": "Stats not available",
      "message": "C core may not be running"
    }), 503

  return jsonify(data)


@system_bp.route('/health', methods=['GET'])
def health_check():
  """
  Health check endpoint for monitoring

  Returns:
    JSON with health status and IPC health
  """
  ipc = current_app.config['IPC_CLIENT']
  health = ipc.health_check()

  # Determine overall status
  if all(health.values()):
    status = "healthy"
    code = 200
  elif health.get('c_core_running'):
    status = "degraded"
    code = 200
  else:
    status = "unhealthy"
    code = 503

  return jsonify({
    "status": status,
    "version": "0.2.0",
    "ipc": health
  }), code


@system_bp.route('/status', methods=['GET'])
def get_status():
  """
  Returns C core status

  Returns:
    JSON with core status information
  """
  ipc = current_app.config['IPC_CLIENT']
  data = ipc.get_status()

  if not data:
    return jsonify({
      "error": "Status not available",
      "message": "C core may not be running"
    }), 503

  return jsonify(data)
