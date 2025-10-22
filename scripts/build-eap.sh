#!/bin/bash
# OMNISIGHT ACAP Package Builder
# Creates .eap package for deployment to Axis cameras

set -e  # Exit on error

echo "================================================"
echo "OMNISIGHT ACAP Package Builder"
echo "================================================"
echo ""

# Directories
PROJECT_ROOT="/opt/app"
BUILD_DIR="$PROJECT_ROOT/build-stub"
PACKAGE_DIR="$PROJECT_ROOT/package"
OUTPUT_DIR="$PROJECT_ROOT/output"

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Step 1: Build the application
echo "Step 1: Building OMNISIGHT..."
cd "$PROJECT_ROOT"

if [ ! -d "$BUILD_DIR" ]; then
    echo "${YELLOW}Build directory doesn't exist. Running build script...${NC}"
    ./scripts/build-stub.sh
fi

if [ ! -f "$BUILD_DIR/omnisight" ]; then
    echo "${RED}ERROR: omnisight executable not found${NC}"
    echo "Please run ./scripts/build-stub.sh first"
    exit 1
fi

echo "${GREEN}✓ Binary built${NC}"

# Step 2: Create package directory structure
echo ""
echo "Step 2: Creating package directory structure..."
rm -rf "$PACKAGE_DIR"
mkdir -p "$PACKAGE_DIR"
mkdir -p "$PACKAGE_DIR/html"
mkdir -p "$PACKAGE_DIR/html/img"
mkdir -p "$PACKAGE_DIR/lib"
mkdir -p "$OUTPUT_DIR"

# Step 3: Create a minimal Python web server
echo "Step 3: Creating minimal web server..."

# Create a simple Python HTTP server that serves the HTML directory
cat > "$PACKAGE_DIR/omnisight" << 'EOF'
#!/usr/bin/env python3
import http.server
import socketserver
import os

# Change to the html directory
os.chdir('/usr/local/packages/omnisight/html')

# Simple HTTP server on port 8080
PORT = 8080
Handler = http.server.SimpleHTTPRequestHandler

with socketserver.TCPServer(("", PORT), Handler) as httpd:
    print(f"OMNISIGHT web server running on port {PORT}")
    httpd.serve_forever()
EOF
chmod +x "$PACKAGE_DIR/omnisight"
echo "  ✓ Created Python web server"

# Copy manifest (use camera-compatible manifest for ACAP packaging)
cp "$PROJECT_ROOT/scripts/manifest-camera.json" "$PACKAGE_DIR/manifest.json"
echo "  ✓ Copied manifest.json"

# Step 4: Create LICENSE file (in both package dir and project root for eap-create.sh)
echo "Step 4: Creating LICENSE..."
cat > "$PACKAGE_DIR/LICENSE" << 'EOF'
Copyright (c) 2024-2025 OMNISIGHT Project

Permission is hereby granted to use this software on Axis Communications
cameras for evaluation and development purposes.

This is proprietary software. Redistribution and commercial use require
explicit permission from the copyright holder.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
EOF

# eap-create.sh expects LICENSE in current working directory
cp "$PACKAGE_DIR/LICENSE" "$PROJECT_ROOT/LICENSE"
echo "  ✓ Created LICENSE (package and project root)"

# Step 5: Create basic HTML interface
echo "Step 5: Creating web interface..."
cat > "$PACKAGE_DIR/html/index.html" << 'EOF'
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
            max-width: 800px;
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
            margin-bottom: 10px;
        }
        .metric {
            display: inline-block;
            margin: 10px 20px 10px 0;
        }
        .metric-label {
            font-size: 0.9em;
            color: #666;
        }
        .metric-value {
            font-size: 1.5em;
            font-weight: bold;
            color: #667eea;
        }
        .features {
            margin-top: 30px;
        }
        .feature {
            margin: 15px 0;
            padding-left: 30px;
            position: relative;
        }
        .feature:before {
            content: "✓";
            position: absolute;
            left: 0;
            color: #4caf50;
            font-weight: bold;
            font-size: 1.2em;
        }
        .version {
            text-align: center;
            color: #999;
            margin-top: 30px;
            font-size: 0.9em;
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>OMNISIGHT</h1>
        <div class="tagline">Precognitive Security System</div>

        <div class="status">
            <h3>System Status</h3>
            <div class="metric">
                <div class="metric-label">Status</div>
                <div class="metric-value">Active</div>
            </div>
            <div class="metric">
                <div class="metric-label">Mode</div>
                <div class="metric-value">Stub/Demo</div>
            </div>
            <div class="metric">
                <div class="metric-label">Version</div>
                <div class="metric-value">0.1.0</div>
            </div>
        </div>

        <div class="features">
            <h3>Active Modules</h3>
            <div class="feature">Perception Engine - Object detection and tracking</div>
            <div class="feature">Timeline Threading™ - Predicting 3-5 probable futures</div>
            <div class="feature">Swarm Intelligence - Multi-camera coordination (stub)</div>
        </div>

        <div class="features">
            <h3>Features</h3>
            <div class="feature">Real-time object tracking simulation</div>
            <div class="feature">Predictive timeline generation</div>
            <div class="feature">Event detection and classification</div>
            <div class="feature">Threat assessment scoring</div>
        </div>

        <div class="version">
            OMNISIGHT v0.1.0 | Development Build<br>
            © 2024-2025 OMNISIGHT Project
        </div>
    </div>
</body>
</html>
EOF
echo "  ✓ Created HTML interface"

# Create placeholder icon
cat > "$PACKAGE_DIR/html/img/icon.png" << 'EOF'
This would be a PNG file in production
EOF

# Step 6: Create CGI script in html directory (will be accessible)
echo "Step 6: Creating CGI index page..."

# Create index.cgi in html directory
cat > "$PACKAGE_DIR/html/index.cgi" << 'EOF'
#!/bin/sh
echo "Content-Type: text/html"
echo ""
cat <<'HTMLEOF'
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
            max-width: 800px;
            width: 100%;
            box-shadow: 0 20px 60px rgba(0,0,0,0.3);
        }
        h1 { color: #667eea; margin-bottom: 10px; font-size: 2.5em; }
        .tagline { color: #666; font-size: 1.2em; margin-bottom: 30px; }
        .status {
            background: #f0f4ff;
            border-left: 4px solid #667eea;
            padding: 20px;
            margin: 20px 0;
            border-radius: 8px;
        }
        .metric { display: inline-block; margin: 10px 20px 10px 0; }
    </style>
</head>
<body>
    <div class="container">
        <h1>OMNISIGHT</h1>
        <div class="tagline">Precognitive Security System - Demo Mode</div>
        <div class="status">
            <h3>System Status</h3>
            <div class="metric">Status: Active (Stub/Demo)</div>
            <div class="metric">Mode: Static Web Interface</div>
            <div class="metric">Version: 0.1.3</div>
        </div>
        <p><strong>Note:</strong> This is a static demo version. Full functionality requires hardware integration.</p>
    </div>
</body>
</html>
HTMLEOF
EOF
chmod +x "$PACKAGE_DIR/html/index.cgi"
echo "  ✓ Created CGI index page in html/"

# Step 7: Show package contents
echo ""
echo "Step 7: Package contents:"
echo "----------------------------------------"
find "$PACKAGE_DIR" -type f -o -type d | sort
echo "----------------------------------------"

# Step 8: Build .eap package
echo ""
echo "Step 8: Building .eap package..."
echo "This uses the ACAP SDK acap-build tool..."

# Clean any leftover files from previous builds
rm -f "$PROJECT_ROOT"/*.eap 2>/dev/null || true
find "$PACKAGE_DIR" -type f -name 'tmp*' -delete 2>/dev/null || true
find "$PACKAGE_DIR" -type f -name '*.eap' -delete 2>/dev/null || true
find "$PACKAGE_DIR" -type f -name 'package.conf' -delete 2>/dev/null || true
find "$PACKAGE_DIR" -type f -name 'param.conf' -delete 2>/dev/null || true
find "$PACKAGE_DIR" -type f -name 'cgi.conf' -delete 2>/dev/null || true

# Change to package directory (eap-create.sh expects to run from there)
cd "$PACKAGE_DIR"

# Source ACAP SDK environment
echo "  Sourcing ACAP SDK environment..."
. /opt/axis/acapsdk/environment-setup-cortexa53-crypto-poky-linux

# Use acap-build with no-build option since we already built
# Disable manifest validation since we have extra fields for documentation
acap-build \
    --build no-build \
    --disable-manifest-validation \
    .

# The .eap file is created in the package directory
if [ -f "$PACKAGE_DIR"/*.eap ]; then
    EAP_FILE=$(ls "$PACKAGE_DIR"/*.eap | head -1)
    EAP_NAME=$(basename "$EAP_FILE")

    # Move to output directory
    mv "$EAP_FILE" "$OUTPUT_DIR/"

    echo ""
    echo "================================================"
    echo "${GREEN}✓ Package created successfully!${NC}"
    echo "================================================"
    echo ""
    echo "Package: $OUTPUT_DIR/$EAP_NAME"
    echo "Size: $(du -h "$OUTPUT_DIR/$EAP_NAME" | cut -f1)"
    echo ""
    echo "Next steps:"
    echo "  1. Copy to your local machine:"
    echo "     docker cp omnisight-dev-1:/opt/app/output/$EAP_NAME ./"
    echo ""
    echo "  2. Deploy to camera:"
    echo "     - Web UI: https://camera-ip → Settings → Apps → Upload"
    echo "     - CLI: curl -u root:pass -F 'package=@$EAP_NAME' https://camera-ip/axis-cgi/applications/upload.cgi"
    echo ""
else
    echo "${RED}ERROR: .eap file not created${NC}"
    echo "Check the acap-build output above for errors"
    exit 1
fi
