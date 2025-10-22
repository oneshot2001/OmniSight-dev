#!/usr/bin/env python3
"""
OMNISIGHT Flask API Server

REST API server for OMNISIGHT Precognitive Security System.
Provides endpoints for perception, timeline predictions, swarm intelligence,
configuration, and system statistics.

Phase 2: Currently returns stub data. Will be connected to C core via IPC.
"""

from flask import Flask, send_from_directory, jsonify
from flask_cors import CORS
import logging
import os

# Import API blueprints
from api.perception import perception_bp
from api.timeline import timeline_bp
from api.swarm import swarm_bp
from api.config import config_bp
from api.system import system_bp

# Initialize Flask app
app = Flask(__name__)

# Enable CORS for development (allows frontend to call APIs from different origin)
CORS(app)

# Configure logging
logging.basicConfig(
  level=logging.INFO,
  format='%(asctime)s - %(name)s - %(levelname)s - %(message)s'
)
logger = logging.getLogger(__name__)

# Register API blueprints
app.register_blueprint(perception_bp, url_prefix='/api/perception')
app.register_blueprint(timeline_bp, url_prefix='/api/timeline')
app.register_blueprint(swarm_bp, url_prefix='/api/swarm')
app.register_blueprint(config_bp, url_prefix='/api/config')
app.register_blueprint(system_bp, url_prefix='/api')

# Determine HTML directory path
# When packaged: /usr/local/packages/omnisight/html
# When developing: ../html relative to app/ directory
HTML_DIR = os.environ.get('OMNISIGHT_HTML_DIR',
                          os.path.join(os.path.dirname(os.path.dirname(__file__)), 'html'))


@app.route('/')
def index():
  """
  Serve the main HTML interface
  """
  return send_from_directory(HTML_DIR, 'index.html')


@app.route('/<path:path>')
def static_files(path):
  """
  Serve static files (CSS, JS, images)
  """
  return send_from_directory(HTML_DIR, path)


@app.errorhandler(404)
def not_found(e):
  """
  Handle 404 errors with JSON response
  """
  return jsonify({
    "error": "Not Found",
    "message": "The requested resource was not found",
    "status": 404
  }), 404


@app.errorhandler(500)
def internal_error(e):
  """
  Handle 500 errors with JSON response
  """
  logger.error(f"Internal server error: {str(e)}")
  return jsonify({
    "error": "Internal Server Error",
    "message": "An internal error occurred",
    "status": 500
  }), 500


if __name__ == '__main__':
  port = int(os.environ.get('PORT', 8080))
  logger.info(f"Starting OMNISIGHT Flask API Server on port {port}")
  logger.info(f"Serving HTML from: {HTML_DIR}")
  logger.info(f"API endpoints available at: http://localhost:{port}/api/")

  # Run Flask server
  # Use threaded=True for concurrent request handling
  app.run(
    host='0.0.0.0',
    port=port,
    debug=False,
    threaded=True
  )
