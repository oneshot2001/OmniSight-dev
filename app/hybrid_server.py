#!/usr/bin/env python3
"""
OMNISIGHT Hybrid API Server (v0.2.1)

Lightweight HTTP server using Python's built-in http.server module.
Serves static JSON files as API responses - no external dependencies required.

This approach works on AXIS cameras because:
- Uses only built-in Python modules (http.server, json, os)
- No pip installation required
- No Flask or external packages needed
- Compatible with any ARTPEC-8/9 camera with Python 3

Directory structure:
/usr/local/packages/omnisight/
├── html/
│   └── index.html              # Web dashboard
├── api/
│   ├── health.json             # /api/health
│   ├── stats.json              # /api/stats
│   ├── perception/
│   │   ├── status.json         # /api/perception/status
│   │   ├── detections.json     # /api/perception/detections
│   │   └── tracks.json         # /api/perception/tracks
│   ├── timeline/
│   │   ├── status.json         # /api/timeline/status
│   │   ├── predictions.json    # /api/timeline/predictions
│   │   └── futures.json        # /api/timeline/futures
│   ├── swarm/
│   │   ├── status.json         # /api/swarm/status
│   │   ├── cameras.json        # /api/swarm/cameras
│   │   └── sync.json           # /api/swarm/sync
│   └── config/
│       ├── perception.json     # /api/config/perception
│       ├── timeline.json       # /api/config/timeline
│       └── swarm.json          # /api/config/swarm
└── hybrid_server.py            # This file
"""

import http.server
import socketserver
import os
import json
from datetime import datetime
from urllib.parse import urlparse, parse_qs

# Configuration
PORT = int(os.environ.get('PORT', 8080))
HTML_DIR = os.environ.get('OMNISIGHT_HTML_DIR', '/usr/local/packages/omnisight/html')
API_DIR = os.environ.get('OMNISIGHT_API_DIR', '/usr/local/packages/omnisight/api')

class OmnisightHandler(http.server.SimpleHTTPRequestHandler):
    """
    Custom HTTP handler that serves:
    - HTML files from html/ directory
    - JSON API responses from api/ directory
    - Adds CORS headers for cross-origin requests
    """

    def __init__(self, *args, **kwargs):
        # Set the directory to serve files from
        super().__init__(*args, directory='/usr/local/packages/omnisight', **kwargs)

    def end_headers(self):
        """Add CORS headers to all responses"""
        self.send_header('Access-Control-Allow-Origin', '*')
        self.send_header('Access-Control-Allow-Methods', 'GET, POST, OPTIONS')
        self.send_header('Access-Control-Allow-Headers', 'Content-Type')
        super().end_headers()

    def do_GET(self):
        """Handle GET requests"""
        parsed_path = urlparse(self.path)
        path = parsed_path.path

        # Route API requests
        if path.startswith('/api/'):
            self.serve_api(path)
        # Route dashboard requests
        elif path == '/' or path == '/index.html':
            self.serve_file(os.path.join(HTML_DIR, 'index.html'), 'text/html')
        # Route other files
        else:
            super().do_GET()

    def do_OPTIONS(self):
        """Handle OPTIONS requests for CORS preflight"""
        self.send_response(200)
        self.end_headers()

    def serve_api(self, path):
        """
        Serve API responses from JSON files

        Maps URL paths to JSON files:
        /api/health          -> api/health.json
        /api/stats           -> api/stats.json
        /api/perception/status -> api/perception/status.json
        """
        # Remove /api/ prefix and add .json extension
        json_path = path.replace('/api/', 'api/') + '.json'
        json_file = os.path.join('/usr/local/packages/omnisight', json_path)

        if os.path.exists(json_file):
            self.serve_file(json_file, 'application/json')
        else:
            # Return 404 with JSON error
            self.send_response(404)
            self.send_header('Content-Type', 'application/json')
            self.end_headers()
            error = {
                'error': 'Not Found',
                'message': f'API endpoint {path} not found',
                'timestamp': datetime.utcnow().isoformat() + 'Z'
            }
            self.wfile.write(json.dumps(error, indent=2).encode())

    def serve_file(self, filepath, content_type):
        """Serve a file with the specified content type"""
        try:
            with open(filepath, 'rb') as f:
                content = f.read()

            self.send_response(200)
            self.send_header('Content-Type', content_type)
            self.send_header('Content-Length', len(content))
            self.end_headers()
            self.wfile.write(content)
        except FileNotFoundError:
            self.send_response(404)
            self.end_headers()
        except Exception as e:
            self.send_response(500)
            self.end_headers()
            self.wfile.write(str(e).encode())

    def log_message(self, format, *args):
        """Override to customize logging format"""
        timestamp = datetime.now().strftime('%Y-%m-%d %H:%M:%S')
        print(f"[{timestamp}] {self.address_string()} - {format % args}")


def main():
    """Start the OMNISIGHT hybrid API server"""

    print("=" * 60)
    print("OMNISIGHT Hybrid API Server v0.2.1")
    print("=" * 60)
    print(f"Port:        {PORT}")
    print(f"HTML Dir:    {HTML_DIR}")
    print(f"API Dir:     {API_DIR}")
    print("=" * 60)
    print(f"\nStarting server on 0.0.0.0:{PORT}...")
    print(f"Dashboard:   http://localhost:{PORT}/")
    print(f"Health API:  http://localhost:{PORT}/api/health")
    print("\nPress Ctrl+C to stop the server.")
    print("=" * 60)

    # Create server
    with socketserver.TCPServer(("", PORT), OmnisightHandler) as httpd:
        try:
            httpd.serve_forever()
        except KeyboardInterrupt:
            print("\n\nShutting down server...")
            httpd.shutdown()
            print("Server stopped.")


if __name__ == '__main__':
    main()
