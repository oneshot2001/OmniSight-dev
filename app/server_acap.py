#!/usr/bin/env python3
"""
OMNISIGHT Flask API Server - ACAP Edition
Simplified for deployment on Axis cameras (Phase 3)
"""

from flask import Flask, jsonify, request
from flask_cors import CORS
import json
import os
import logging
import time
from datetime import datetime

# Configure logging
logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s [%(levelname)s] %(name)s: %(message)s'
)
logger = logging.getLogger('omnisight')

# Initialize Flask app
app = Flask(__name__)
CORS(app)

# Configuration
IPC_DIR = '/tmp/omnisight'
PORT = 8080

# Create IPC directory if it doesn't exist
os.makedirs(IPC_DIR, exist_ok=True)

# ============================================================================
# IPC Helper Functions
# ============================================================================

def read_json_file(filename, default=None):
    """Read JSON data from IPC directory"""
    filepath = os.path.join(IPC_DIR, filename)
    try:
        if os.path.exists(filepath):
            with open(filepath, 'r') as f:
                return json.load(f)
        else:
            logger.warning(f"File not found: {filepath}, using default")
            return default if default is not None else {}
    except Exception as e:
        logger.error(f"Error reading {filename}: {e}")
        return default if default is not None else {}

def write_json_file(filename, data):
    """Write JSON data to IPC directory"""
    filepath = os.path.join(IPC_DIR, filename)
    try:
        with open(filepath, 'w') as f:
            json.dump(data, f, indent=2)
        return True
    except Exception as e:
        logger.error(f"Error writing {filename}: {e}")
        return False

# ============================================================================
# System Endpoints
# ============================================================================

@app.route('/api/health', methods=['GET'])
def health():
    """Health check endpoint"""
    return jsonify({
        'status': 'healthy',
        'timestamp': datetime.now().isoformat(),
        'mode': 'acap'
    })

@app.route('/api/status', methods=['GET'])
def status():
    """Get overall system status"""
    perception = read_json_file('perception_state.json', {})
    timeline = read_json_file('timeline_state.json', {})
    swarm = read_json_file('swarm_state.json', {})

    return jsonify({
        'status': 'active',
        'mode': 'acap',
        'version': '0.6.0',
        'timestamp': datetime.now().isoformat(),
        'uptime_seconds': int(time.time() - app.start_time),
        'fps': perception.get('fps', 0),
        'latency_ms': perception.get('latency_ms', 0),
        'camera_id': os.environ.get('AXIS_CAMERA_ID', 'unknown'),
        'modules': {
            'perception': {
                'status': perception.get('status', 'active'),
                'fps': perception.get('fps', 0)
            },
            'timeline': {
                'status': timeline.get('status', 'active'),
                'predictions': len(timeline.get('predictions', []))
            },
            'swarm': {
                'status': swarm.get('status', 'active'),
                'cameras': len(swarm.get('cameras', []))
            }
        }
    })

@app.route('/api/stats', methods=['GET'])
def stats():
    """Get system statistics (alias for status)"""
    return status()

# ============================================================================
# Perception Endpoints
# ============================================================================

@app.route('/api/perception/status', methods=['GET'])
def perception_status():
    """Get perception module status"""
    data = read_json_file('perception_state.json', {})
    return jsonify({
        'status': data.get('status', 'active'),
        'fps': data.get('fps', 0),
        'detections_count': len(data.get('detections', [])),
        'tracks_count': data.get('tracks_count', 0)
    })

@app.route('/api/perception/detections', methods=['GET'])
def perception_detections():
    """Get current frame detections"""
    data = read_json_file('perception_state.json', {})
    return jsonify({
        'timestamp': data.get('timestamp', datetime.now().isoformat()),
        'frame_number': data.get('frame_number', 0),
        'detections': data.get('detections', [])
    })

@app.route('/api/perception/tracks', methods=['GET'])
def perception_tracks():
    """Get active object tracks"""
    data = read_json_file('perception_state.json', {})
    return jsonify({
        'timestamp': data.get('timestamp', datetime.now().isoformat()),
        'tracks': data.get('tracks', [])
    })

# ============================================================================
# Timeline Endpoints
# ============================================================================

@app.route('/api/timeline/predictions', methods=['GET'])
def timeline_predictions():
    """Get timeline predictions"""
    data = read_json_file('timeline_state.json', {})
    return jsonify({
        'timestamp': data.get('timestamp', datetime.now().isoformat()),
        'current_state': data.get('current_state', {}),
        'predictions': data.get('predictions', [])
    })

@app.route('/api/timeline/history', methods=['GET'])
def timeline_history():
    """Get timeline history"""
    data = read_json_file('timeline_state.json', {})
    return jsonify({
        'events': data.get('history', [])
    })

@app.route('/api/timeline/interventions', methods=['GET'])
def timeline_interventions():
    """Get recommended interventions"""
    data = read_json_file('timeline_state.json', {})
    return jsonify({
        'interventions': data.get('interventions', [])
    })

# ============================================================================
# Swarm Endpoints
# ============================================================================

@app.route('/api/swarm/network', methods=['GET'])
def swarm_network():
    """Get swarm network topology"""
    data = read_json_file('swarm_state.json', {})
    return jsonify({
        'topology': data.get('topology', 'standalone'),
        'cameras': data.get('cameras', []),
        'total_cameras': len(data.get('cameras', [])),
        'active_connections': data.get('active_connections', 0),
        'health_score': data.get('health_score', 1.0)
    })

@app.route('/api/swarm/cameras', methods=['GET'])
def swarm_cameras():
    """Get list of cameras in swarm"""
    data = read_json_file('swarm_state.json', {})
    return jsonify(data.get('cameras', []))

@app.route('/api/swarm/sync', methods=['POST'])
def swarm_sync():
    """Trigger manual swarm synchronization"""
    logger.info("Manual swarm sync triggered")
    return jsonify({'status': 'sync_initiated'})

# ============================================================================
# Events Endpoint
# ============================================================================

@app.route('/api/events', methods=['GET'])
def events():
    """Get recent security events"""
    timeline = read_json_file('timeline_state.json', {})
    return jsonify(timeline.get('events', []))

# ============================================================================
# Configuration Endpoints
# ============================================================================

@app.route('/api/config', methods=['GET'])
def get_config():
    """Get current configuration"""
    config = read_json_file('config.json', {
        'perception': {
            'detection_threshold': 0.5,
            'tracking_enabled': True,
            'max_tracks': 50,
            'fps_limit': 30
        },
        'timeline': {
            'prediction_horizon_seconds': 300,
            'min_confidence': 0.6,
            'max_futures': 3
        },
        'swarm': {
            'enabled': False,
            'mqtt_broker': 'localhost:1883',
            'sync_interval_seconds': 5
        }
    })
    return jsonify(config)

@app.route('/api/config', methods=['POST'])
def update_config():
    """Update configuration"""
    try:
        new_config = request.get_json()
        if write_json_file('config.json', new_config):
            logger.info("Configuration updated")
            return jsonify({'status': 'success', 'config': new_config})
        else:
            return jsonify({'status': 'error', 'message': 'Failed to write config'}), 500
    except Exception as e:
        logger.error(f"Error updating config: {e}")
        return jsonify({'status': 'error', 'message': str(e)}), 500

# ============================================================================
# Legacy Endpoints (for compatibility)
# ============================================================================

@app.route('/api/statistics', methods=['GET'])
def statistics():
    """Legacy statistics endpoint"""
    return status()

@app.route('/api/tracks', methods=['GET'])
def tracks():
    """Legacy tracks endpoint"""
    return perception_tracks()

@app.route('/api/timelines', methods=['GET'])
def timelines():
    """Legacy timelines endpoint"""
    return timeline_predictions()

@app.route('/api/cameras', methods=['GET'])
def cameras():
    """Legacy cameras endpoint"""
    return swarm_cameras()

@app.route('/api/heatmap', methods=['GET'])
def heatmap():
    """Get threat heatmap data"""
    # For now, return empty heatmap
    # TODO: Implement heatmap calculation
    return jsonify({
        'timestamp': datetime.now().isoformat(),
        'resolution': {'width': 1920, 'height': 1080},
        'cells': []
    })

# ============================================================================
# Main
# ============================================================================

if __name__ == '__main__':
    # Store start time
    app.start_time = time.time()

    logger.info(f"Starting OMNISIGHT Flask API Server (ACAP Edition)")
    logger.info(f"IPC Directory: {IPC_DIR}")
    logger.info(f"Listening on port {PORT}")

    # Run Flask server
    app.run(
        host='0.0.0.0',
        port=PORT,
        debug=False,
        threaded=True
    )
