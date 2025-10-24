#!/bin/bash
# OMNISIGHT Phase 2 ACAP Package Builder
# Creates .eap package with Flask API for M4228-LVE testing
# NO DOCKER REQUIRED - Builds directly on macOS/Linux

set -e  # Exit on error

echo "================================================"
echo "OMNISIGHT Phase 2 ACAP Package Builder"
echo "Flask API + Reverse Proxy (v0.2.0)"
echo "================================================"
echo ""

# Get the directory where this script is located
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"

# Directories
PACKAGE_DIR="$PROJECT_ROOT/package-phase2"
OUTPUT_DIR="$PROJECT_ROOT"

# Colors
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo "Project root: $PROJECT_ROOT"
echo ""

# Step 1: Clean and create package directory
echo "Step 1: Creating package structure..."
rm -rf "$PACKAGE_DIR"
mkdir -p "$PACKAGE_DIR"
mkdir -p "$PACKAGE_DIR/html"
mkdir -p "$PACKAGE_DIR/app"
mkdir -p "$PACKAGE_DIR/app/api"

# Step 2: Copy Flask application
echo ""
echo "Step 2: Copying Flask API server..."

# Copy Python app files
cp "$PROJECT_ROOT/app/server.py" "$PACKAGE_DIR/app/"
cp "$PROJECT_ROOT/app/requirements.txt" "$PACKAGE_DIR/app/"
cp -r "$PROJECT_ROOT/app/api" "$PACKAGE_DIR/app/"

echo "  ✓ Copied server.py"
echo "  ✓ Copied requirements.txt"
echo "  ✓ Copied API modules (5 files)"

# Step 3: Create launcher script
echo ""
echo "Step 3: Creating ACAP launcher..."

cat > "$PACKAGE_DIR/omnisight" << 'LAUNCHER_EOF'
#!/bin/sh
# OMNISIGHT Phase 2 - Flask API Server Launcher

echo "OMNISIGHT: Starting Flask API Server (Phase 2)..."

# Set environment variables
export OMNISIGHT_HTML_DIR="/usr/local/packages/omnisight/html"
export PYTHONPATH="/usr/local/packages/omnisight:$PYTHONPATH"
export PORT=8080

# Change to app directory
cd /usr/local/packages/omnisight/app

# Install dependencies on first run
if [ ! -f /tmp/omnisight-deps-installed ]; then
    echo "OMNISIGHT: Installing Flask dependencies..."
    pip3 install --no-cache-dir Flask==3.0.0 Flask-CORS==4.0.0 Werkzeug==3.0.1 || {
        echo "ERROR: Failed to install dependencies"
        exit 1
    }
    touch /tmp/omnisight-deps-installed
    echo "OMNISIGHT: Dependencies installed successfully"
fi

# Start Flask server
echo "OMNISIGHT: Flask API server starting on port $PORT..."
echo "OMNISIGHT: Dashboard: https://<camera-ip>/local/omnisight/"
echo "OMNISIGHT: API: https://<camera-ip>/local/omnisight/api/"

exec python3 server.py
LAUNCHER_EOF

chmod +x "$PACKAGE_DIR/omnisight"
echo "  ✓ Created executable launcher script"

# Step 4: Copy manifest
echo ""
echo "Step 4: Creating manifest.json..."

cp "$PROJECT_ROOT/scripts/manifest-camera.json" "$PACKAGE_DIR/manifest.json"
echo "  ✓ Copied manifest (schema 1.8.0, reverse proxy configured)"

# Step 5: Create LICENSE
echo ""
echo "Step 5: Creating LICENSE..."

cat > "$PACKAGE_DIR/LICENSE" << 'LICENSE_EOF'
MIT License

Copyright (c) 2024-2025 OMNISIGHT Project

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
LICENSE_EOF

echo "  ✓ Created MIT LICENSE"

# Step 6: Create HTML interface
echo ""
echo "Step 6: Creating web interface..."

cat > "$PACKAGE_DIR/html/index.html" << 'HTML_EOF'
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>OMNISIGHT - Precognitive Security</title>
    <style>
        * { margin: 0; padding: 0; box-sizing: border-box; }
        body {
            font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, sans-serif;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            color: #333;
            min-height: 100vh;
            display: flex;
            align-items: center;
            justify-content: center;
            padding: 20px;
        }
        .container {
            background: white;
            border-radius: 20px;
            padding: 40px;
            max-width: 900px;
            width: 100%;
            box-shadow: 0 20px 60px rgba(0,0,0,0.3);
        }
        h1 {
            color: #667eea;
            margin-bottom: 10px;
            font-size: 2.5em;
        }
        .tagline {
            color: #666;
            font-size: 1.2em;
            margin-bottom: 30px;
        }
        .status {
            background: #f0f4ff;
            border-left: 4px solid #667eea;
            padding: 20px;
            margin: 20px 0;
            border-radius: 8px;
        }
        .status h3 {
            color: #667eea;
            margin-bottom: 15px;
        }
        .metric {
            display: inline-block;
            margin: 10px 30px 10px 0;
        }
        .metric-label {
            font-size: 0.85em;
            color: #666;
            display: block;
            margin-bottom: 5px;
        }
        .metric-value {
            font-size: 1.8em;
            font-weight: bold;
            color: #667eea;
        }
        .api-info {
            background: #fff9e6;
            border-left: 4px solid #f39c12;
            padding: 20px;
            margin: 20px 0;
            border-radius: 8px;
        }
        .api-info h3 {
            color: #f39c12;
            margin-bottom: 15px;
        }
        .endpoint {
            font-family: 'Courier New', monospace;
            font-size: 0.9em;
            color: #555;
            margin: 8px 0;
            padding: 8px 12px;
            background: white;
            border-radius: 4px;
        }
        .endpoint strong {
            color: #2ecc71;
            margin-right: 10px;
        }
        .features {
            margin-top: 30px;
        }
        .features h3 {
            margin-bottom: 15px;
            color: #333;
        }
        .feature {
            margin: 12px 0;
            padding-left: 30px;
            position: relative;
            line-height: 1.6;
        }
        .feature:before {
            content: "✓";
            position: absolute;
            left: 0;
            color: #4caf50;
            font-weight: bold;
            font-size: 1.3em;
        }
        .version {
            text-align: center;
            color: #999;
            margin-top: 30px;
            padding-top: 20px;
            border-top: 1px solid #eee;
            font-size: 0.9em;
        }
        .test-btn {
            display: inline-block;
            background: #667eea;
            color: white;
            padding: 12px 24px;
            border-radius: 6px;
            text-decoration: none;
            margin: 10px 10px 0 0;
            font-weight: 500;
            transition: background 0.3s;
        }
        .test-btn:hover {
            background: #5568d3;
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>🔮 OMNISIGHT</h1>
        <div class="tagline">Precognitive Security System</div>

        <div class="status">
            <h3>System Status</h3>
            <div class="metric">
                <span class="metric-label">Status</span>
                <span class="metric-value">Active</span>
            </div>
            <div class="metric">
                <span class="metric-label">Mode</span>
                <span class="metric-value">Phase 2</span>
            </div>
            <div class="metric">
                <span class="metric-label">Version</span>
                <span class="metric-value">0.2.0</span>
            </div>
        </div>

        <div class="api-info">
            <h3>📡 REST API Endpoints</h3>
            <div class="endpoint"><strong>GET</strong> /api/health</div>
            <div class="endpoint"><strong>GET</strong> /api/perception/status</div>
            <div class="endpoint"><strong>GET</strong> /api/perception/detections</div>
            <div class="endpoint"><strong>GET</strong> /api/timeline/predictions</div>
            <div class="endpoint"><strong>GET</strong> /api/timeline/history</div>
            <div class="endpoint"><strong>GET</strong> /api/swarm/network</div>
            <div class="endpoint"><strong>GET</strong> /api/swarm/cameras</div>
            <div class="endpoint"><strong>GET</strong> /api/config</div>
            <div class="endpoint"><strong>POST</strong> /api/config</div>
            <div class="endpoint"><strong>GET</strong> /api/stats</div>

            <div style="margin-top: 15px;">
                <a href="api/health" class="test-btn">Test Health Endpoint</a>
                <a href="api/stats" class="test-btn">Test Stats Endpoint</a>
            </div>
        </div>

        <div class="features">
            <h3>🚀 Active Modules</h3>
            <div class="feature"><strong>Perception Engine</strong> - Object detection and tracking (stub data)</div>
            <div class="feature"><strong>Timeline Threading™</strong> - Predicting 3-5 probable futures (stub data)</div>
            <div class="feature"><strong>Swarm Intelligence</strong> - Multi-camera coordination framework</div>
        </div>

        <div class="features">
            <h3>✨ Phase 2 Features</h3>
            <div class="feature">Flask REST API with 10 endpoints</div>
            <div class="feature">Reverse proxy integration (Apache)</div>
            <div class="feature">JSON response format</div>
            <div class="feature">CORS support for development</div>
            <div class="feature">Real-time status reporting</div>
            <div class="feature">Configuration management</div>
        </div>

        <div class="version">
            <strong>OMNISIGHT v0.2.0</strong> | Phase 2 Development Build<br>
            <small>Flask API Server with stub data responses</small><br>
            <small>ARTPEC-8/9 Compatible | Tested on M4228-LVE</small><br>
            <small>© 2024-2025 OMNISIGHT Project | MIT License</small>
        </div>
    </div>
</body>
</html>
HTML_EOF

echo "  ✓ Created HTML dashboard"

# Step 7: Create package contents summary
echo ""
echo "Step 7: Package contents:"
echo "========================================="
find "$PACKAGE_DIR" -type f | sed "s|$PACKAGE_DIR/||" | sort
echo "========================================="

# Count files
FILE_COUNT=$(find "$PACKAGE_DIR" -type f | wc -l | tr -d ' ')
echo "Total files: $FILE_COUNT"

# Step 8: Create tarball (manual .eap creation since we don't have ACAP SDK on macOS)
echo ""
echo "Step 8: Creating .eap package..."

# Read version from manifest
VERSION=$(grep '"version"' "$PACKAGE_DIR/manifest.json" | sed 's/.*: *"\([^"]*\)".*/\1/')
PACKAGE_NAME="OMNISIGHT_-_Precognitive_Security_${VERSION}_aarch64.eap"

# Create tarball in the package directory
cd "$PACKAGE_DIR"
tar -czf "../$PACKAGE_NAME" *

# Move back
cd "$PROJECT_ROOT"

if [ -f "$PACKAGE_NAME" ]; then
    PACKAGE_SIZE=$(du -h "$PACKAGE_NAME" | cut -f1)

    echo ""
    echo "================================================"
    echo "${GREEN}✓ Package created successfully!${NC}"
    echo "================================================"
    echo ""
    echo "📦 Package: $PACKAGE_NAME"
    echo "📊 Size: $PACKAGE_SIZE"
    echo "📁 Location: $PROJECT_ROOT/$PACKAGE_NAME"
    echo ""
    echo "📋 What's Included:"
    echo "  ✓ Flask API server (10 endpoints)"
    echo "  ✓ Reverse proxy manifest (schema 1.8.0)"
    echo "  ✓ HTML dashboard"
    echo "  ✓ Python dependencies: Flask 3.0.0, Flask-CORS"
    echo "  ✓ API modules: perception, timeline, swarm, config, system"
    echo ""
    echo "🚀 Next Steps:"
    echo ""
    echo "  1. Deploy to M4228-LVE camera:"
    echo "     - Open browser: https://camera-ip"
    echo "     - Go to: Settings → Apps → Add app"
    echo "     - Upload: $PACKAGE_NAME"
    echo "     - Click 'Start'"
    echo ""
    echo "  2. Access dashboard:"
    echo "     https://camera-ip/local/omnisight/"
    echo ""
    echo "  3. Test API endpoints:"
    echo "     https://camera-ip/local/omnisight/api/health"
    echo "     https://camera-ip/local/omnisight/api/perception/status"
    echo "     https://camera-ip/local/omnisight/api/stats"
    echo ""
    echo "  4. Run for 30-60 minutes, then collect logs:"
    echo "     See: LOG_COLLECTION_GUIDE.md"
    echo ""
    echo "================================================"
    echo ""
else
    echo "ERROR: Package creation failed"
    exit 1
fi
