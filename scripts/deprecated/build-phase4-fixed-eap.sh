#!/bin/bash
#
# OMNISIGHT Phase 4 ACAP Package Builder (FIXED)
# Claude Flow Integration with proper ACAP configuration files
#
# ⚠️ DEPRECATED: This version still requires Python 3!
# Use build-phase4-v3-native-eap.sh instead - works on all cameras.
#

set -e  # Exit on error

echo "================================================"
echo "OMNISIGHT Phase 4 ACAP Package Builder (FIXED)"
echo "Claude Flow Integration"
echo "================================================"
echo ""

# Determine project root
if [ -d "/opt/app" ]; then
  # Running in Docker
  PROJECT_ROOT="/opt/app"
else
  # Running on host
  PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
fi

echo "Project root: $PROJECT_ROOT"
cd "$PROJECT_ROOT"

# Version
VERSION="0.4.0"
PACKAGE_NAME="OMNISIGHT_-_Precognitive_Security_${VERSION//./}_aarch64"

# Directories
BUILD_DIR="$PROJECT_ROOT/build-phase4-fixed"
PACKAGE_DIR="$BUILD_DIR/package"
OUTPUT_DIR="$PROJECT_ROOT/output"

echo ""
echo "Step 1: Creating build and package structure..."
rm -rf "$BUILD_DIR"
mkdir -p "$PACKAGE_DIR"/{app,html}
mkdir -p "$OUTPUT_DIR"
echo "  ✓ Directories created"

echo ""
echo "Step 2: Copying Phase 4 Python coordinators..."
# Copy coordinator modules
cp -r "$PROJECT_ROOT/app/coordinator" "$PACKAGE_DIR/app/"
echo "  ✓ Coordinators copied (swarm, timeline, federated, performance)"

# Copy API modules
cp -r "$PROJECT_ROOT/app/api" "$PACKAGE_DIR/app/"
echo "  ✓ API modules copied"

# Copy IPC client
if [ -d "$PROJECT_ROOT/app/ipc" ]; then
  cp -r "$PROJECT_ROOT/app/ipc" "$PACKAGE_DIR/app/"
  echo "  ✓ IPC client copied"
fi

# Copy server.py
cp "$PROJECT_ROOT/app/server.py" "$PACKAGE_DIR/app/"
echo "  ✓ Flask server copied"

# Copy requirements.txt
cp "$PROJECT_ROOT/app/requirements.txt" "$PACKAGE_DIR/app/"
echo "  ✓ Requirements copied"

echo ""
echo "Step 3: Copying web dashboard..."
# Copy HTML dashboard
if [ -d "$PROJECT_ROOT/web/frontend/dist" ]; then
  cp -r "$PROJECT_ROOT/web/frontend/dist"/* "$PACKAGE_DIR/html/"
  echo "  ✓ React build copied"
elif [ -d "$PROJECT_ROOT/package/html" ]; then
  cp -r "$PROJECT_ROOT/package/html"/* "$PACKAGE_DIR/html/"
  echo "  ✓ HTML files copied"
else
  echo "  ⚠ No HTML found, creating placeholder..."
  cat > "$PACKAGE_DIR/html/index.html" <<'EOF'
<!DOCTYPE html>
<html>
<head>
  <title>OMNISIGHT Phase 4</title>
  <style>
    body { font-family: Arial; margin: 40px; background: #1a1a1a; color: #fff; }
    h1 { color: #00ff88; }
    .status { background: #2a2a2a; padding: 20px; border-radius: 8px; margin: 20px 0; }
    .metric { display: inline-block; margin: 10px 20px; }
    .value { font-size: 24px; font-weight: bold; color: #00ff88; }
  </style>
</head>
<body>
  <h1>🧠 OMNISIGHT Phase 4 - Claude Flow Integration</h1>

  <div class="status">
    <h2>System Status</h2>
    <div class="metric">
      <div>Version</div>
      <div class="value">0.4.0</div>
    </div>
    <div class="metric">
      <div>Mode</div>
      <div class="value">Phase 4 Demo</div>
    </div>
    <div class="metric">
      <div>Claude Flow</div>
      <div class="value" id="claude-status">Loading...</div>
    </div>
  </div>

  <div class="status">
    <h2>Phase 4 Features</h2>
    <ul>
      <li><strong>Enhanced Swarm Coordination</strong> - Adaptive topology with Byzantine FT</li>
      <li><strong>Neural Timeline Predictions</strong> - 27+ AI models for futures</li>
      <li><strong>Federated Learning</strong> - Privacy-preserving distributed learning</li>
      <li><strong>Performance Monitoring</strong> - Real-time optimization</li>
    </ul>
  </div>

  <div class="status">
    <h2>API Endpoints</h2>
    <ul>
      <li><a href="/api/claude-flow/status">/api/claude-flow/status</a> - Integration status</li>
      <li><a href="/api/claude-flow/performance/metrics">/api/claude-flow/performance/metrics</a> - Performance metrics</li>
      <li><a href="/api/claude-flow/performance/report">/api/claude-flow/performance/report</a> - Full report</li>
    </ul>
  </div>

  <script>
    // Check Claude Flow status
    fetch('/api/claude-flow/status')
      .then(r => r.json())
      .then(data => {
        document.getElementById('claude-status').textContent =
          data.enabled ? 'Active' : 'Fallback';
        document.getElementById('claude-status').style.color =
          data.enabled ? '#00ff88' : '#ff8800';
      })
      .catch(e => {
        document.getElementById('claude-status').textContent = 'Error';
        document.getElementById('claude-status').style.color = '#ff0088';
      });
  </script>
</body>
</html>
EOF
  echo "  ✓ Placeholder HTML created"
fi

echo ""
echo "Step 4: Creating startup script..."
cat > "$PACKAGE_DIR/omnisight-phase4.sh" <<'EOF'
#!/bin/sh
#
# OMNISIGHT Phase 4 Startup Script
# Initializes Flask server with Claude Flow coordinators
#

# Set Python path
export PYTHONPATH=/usr/local/packages/omnisight/app:$PYTHONPATH

# Set HTML directory
export OMNISIGHT_HTML_DIR=/usr/local/packages/omnisight/html

# Navigate to app directory
cd /usr/local/packages/omnisight/app

# Install Python dependencies (if pip available)
if command -v pip3 >/dev/null 2>&1; then
  pip3 install --quiet --no-warn-script-location -r requirements.txt 2>/dev/null || true
fi

# Check if claude-flow is available
if command -v npx >/dev/null 2>&1; then
  CLAUDE_FLOW_CHECK=$(npx claude-flow@alpha --version 2>/dev/null || echo "not-found")
  if [ "$CLAUDE_FLOW_CHECK" != "not-found" ]; then
    echo "OMNISIGHT: Claude Flow available (version: $CLAUDE_FLOW_CHECK)"
  else
    echo "OMNISIGHT: Claude Flow not available, using fallback coordination"
  fi
else
  echo "OMNISIGHT: npx not found, using fallback coordination"
fi

# Start Flask server
echo "OMNISIGHT: Starting Phase 4 Flask server with Claude Flow integration..."
exec python3 server.py
EOF
chmod +x "$PACKAGE_DIR/omnisight-phase4.sh"
echo "  ✓ Startup script created"

echo ""
echo "Step 5: Creating ACAP configuration files..."

# Create package.conf (REQUIRED by ACAP!)
cat > "$PACKAGE_DIR/package.conf" <<'PKGEOF'
PACKAGENAME="OMNISIGHT - Phase 4 Claude Flow"
APPTYPE="aarch64"
APPNAME="omnisight"
APPID=""
LICENSENAME="Available"
LICENSEPAGE="none"
VENDOR="OMNISIGHT"
REQEMBDEVVERSION="3.0"
APPMAJORVERSION="0"
APPMINORVERSION="4"
APPMICROVERSION="0"
APPGRP="sdk"
APPUSR="sdk"
APPOPTS=""
OTHERFILES=""
SETTINGSPAGEFILE="index.html"
SETTINGSPAGETEXT=""
VENDORHOMEPAGELINK=''
PREUPGRADESCRIPT=""
POSTINSTALLSCRIPT=""
STARTMODE="never"
HTTPCGIPATHS=""
PKGEOF
echo "  ✓ package.conf created"

# Create param.conf (empty but required)
touch "$PACKAGE_DIR/param.conf"
echo "  ✓ param.conf created"

# Create LICENSE file
cat > "$PACKAGE_DIR/LICENSE" <<'LICEOF'
MIT License

Copyright (c) 2025 OMNISIGHT

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
LICEOF
echo "  ✓ LICENSE created"

echo ""
echo "Step 6: Creating manifest.json..."
cat > "$PACKAGE_DIR/manifest.json" <<EOF
{
  "schemaVersion": "1.8.0",
  "acapPackageConf": {
    "setup": {
      "appName": "omnisight",
      "friendlyName": "OMNISIGHT - Phase 4 Claude Flow",
      "vendor": "OMNISIGHT",
      "vendorUrl": "https://github.com/oneshot2001/OmniSight-dev",
      "embeddedSdkVersion": "3.0",
      "version": "$VERSION",
      "appId": 1004,
      "runMode": "respawn",
      "runOptions": "/usr/local/packages/omnisight/omnisight-phase4.sh",
      "architecture": "aarch64"
    }
  },
  "configuration": {
    "settingPage": "index.html",
    "reverseProxy": [
      {
        "apiPath": "api",
        "target": "http://localhost:8080",
        "access": "admin"
      }
    ]
  }
}
EOF
echo "  ✓ Manifest created (Schema 1.8.0, appId: 1004)"

echo ""
echo "Step 7: Creating package archive..."
cd "$BUILD_DIR"

# Create tar.gz
tar czf "${PACKAGE_NAME}.tar.gz" -C package .
echo "  ✓ Package archive created"

# Rename to .eap
mv "${PACKAGE_NAME}.tar.gz" "$OUTPUT_DIR/${PACKAGE_NAME}.eap"

# Calculate size
PACKAGE_SIZE=$(ls -lh "$OUTPUT_DIR/${PACKAGE_NAME}.eap" | awk '{print $5}')

echo ""
echo "================================================"
echo "✓ Phase 4 Package Build Complete! (FIXED)"
echo "================================================"
echo ""
echo "Package: $OUTPUT_DIR/${PACKAGE_NAME}.eap"
echo "Size: $PACKAGE_SIZE"
echo "Version: $VERSION"
echo ""
echo "FIXED Issues:"
echo "  ✓ Added package.conf (REQUIRED by ACAP)"
echo "  ✓ Added param.conf (REQUIRED by ACAP)"
echo "  ✓ Added LICENSE file"
echo ""
echo "Phase 4 Features:"
echo "  • Enhanced Swarm Coordination (adaptive topology)"
echo "  • Neural Timeline Predictions (27+ models)"
echo "  • Federated Learning (privacy-preserving)"
echo "  • Performance Monitoring (real-time optimization)"
echo ""
echo "Next Steps:"
echo "  1. Deploy to P3285-LVE via web interface"
echo "  2. Access dashboard at http://camera-ip/local/omnisight/"
echo "  3. Test Claude Flow endpoints"
echo ""
