#!/bin/bash
#
# OMNISIGHT Phase 4 ACAP Package Builder V2
# Based on working Phase 2 structure with Phase 4 Python coordinators
#

set -e  # Exit on error

echo "================================================"
echo "OMNISIGHT Phase 4 ACAP Package Builder V2"
echo "Based on Working Phase 2 Structure"
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
VERSION="0.4.1"
PACKAGE_NAME="OMNISIGHT_-_Precognitive_Security_${VERSION//./}_aarch64"

# Directories
BUILD_DIR="$PROJECT_ROOT/build-phase4-v2"
PACKAGE_DIR="$BUILD_DIR/package"
OUTPUT_DIR="$PROJECT_ROOT/output"

echo ""
echo "Step 1: Extracting working Phase 2 base..."
rm -rf "$BUILD_DIR"
mkdir -p "$PACKAGE_DIR"
mkdir -p "$OUTPUT_DIR"

# Extract the working Phase 2 package as base
if [ -f "$PROJECT_ROOT/OMNISIGHT_WORKING_0_3_1_aarch64.eap" ]; then
  cd "$PACKAGE_DIR"
  tar xzf "$PROJECT_ROOT/OMNISIGHT_WORKING_0_3_1_aarch64.eap"
  cd "$PROJECT_ROOT"
  echo "  ✓ Phase 2 base extracted"
else
  echo "  ✗ Working Phase 2 package not found!"
  exit 1
fi

echo ""
echo "Step 2: Removing Phase 2 specific files..."
# Remove the old binary (we'll use Python instead)
rm -f "$PACKAGE_DIR/omnisight" 2>/dev/null || true
# Keep: package.conf, param.conf, LICENSE, manifest.json, html/

echo "  ✓ Old binary removed"

echo ""
echo "Step 3: Adding Phase 4 Python coordinators..."
# Create app directory
mkdir -p "$PACKAGE_DIR/app"

# Copy coordinator modules
cp -r "$PROJECT_ROOT/app/coordinator" "$PACKAGE_DIR/app/"
echo "  ✓ Coordinators copied"

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
echo "Step 4: Creating wrapper startup script..."
# Create a wrapper that launches Python Flask server
cat > "$PACKAGE_DIR/omnisight" <<'WRAPPEREOF'
#!/bin/sh
#
# OMNISIGHT Phase 4 Wrapper Script
# Launches Flask server with Phase 4 coordinators
#

# Log startup
echo "OMNISIGHT Phase 4: Starting Flask server..."

# Set Python path
export PYTHONPATH=/usr/local/packages/omnisight/app:$PYTHONPATH

# Set HTML directory
export OMNISIGHT_HTML_DIR=/usr/local/packages/omnisight/html

# Navigate to app directory
cd /usr/local/packages/omnisight/app || exit 1

# Install dependencies quietly
if command -v pip3 >/dev/null 2>&1; then
  pip3 install --quiet --no-warn-script-location Flask Flask-CORS 2>/dev/null || true
fi

# Check for claude-flow
if command -v npx >/dev/null 2>&1; then
  CF_VER=$(npx claude-flow@alpha --version 2>/dev/null || echo "n/a")
  echo "OMNISIGHT: Claude Flow status: $CF_VER"
else
  echo "OMNISIGHT: Claude Flow not available (using fallback)"
fi

# Start Flask server
echo "OMNISIGHT: Launching Flask on port 8080..."
exec python3 server.py
WRAPPEREOF

chmod +x "$PACKAGE_DIR/omnisight"
echo "  ✓ Wrapper script created"

echo ""
echo "Step 5: Updating configuration files..."

# Update package.conf for v0.4.1
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
APPMICROVERSION="1"
APPGRP="sdk"
APPUSR="sdk"
APPOPTS=""
OTHERFILES=""
SETTINGSPAGEFILE="index.html"
SETTINGSPAGETEXT=""
VENDORHOMEPAGELINK=''
PREUPGRADESCRIPT=""
POSTINSTALLSCRIPT=""
STARTMODE="respawn"
HTTPCGIPATHS=""
PKGEOF
echo "  ✓ package.conf updated (v0.4.1, STARTMODE=respawn)"

# Update manifest.json for v0.4.1
cat > "$PACKAGE_DIR/manifest.json" <<'MANEOF'
{
  "schemaVersion": "1.8.0",
  "acapPackageConf": {
    "setup": {
      "appName": "omnisight",
      "friendlyName": "OMNISIGHT - Phase 4 Claude Flow",
      "vendor": "OMNISIGHT",
      "vendorUrl": "https://github.com/oneshot2001/OmniSight-dev",
      "embeddedSdkVersion": "3.0",
      "version": "0.4.1",
      "appId": 1004,
      "runMode": "respawn",
      "runOptions": "/usr/local/packages/omnisight/omnisight",
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
MANEOF
echo "  ✓ manifest.json updated"

echo ""
echo "Step 6: Updating HTML dashboard..."
cat > "$PACKAGE_DIR/html/index.html" <<'HTMLEOF'
<!DOCTYPE html>
<html>
<head>
  <title>OMNISIGHT Phase 4</title>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <style>
    * { margin: 0; padding: 0; box-sizing: border-box; }
    body {
      font-family: 'Segoe UI', Arial, sans-serif;
      background: linear-gradient(135deg, #1a1a2e 0%, #16213e 100%);
      color: #fff;
      padding: 20px;
      min-height: 100vh;
    }
    .container { max-width: 1200px; margin: 0 auto; }
    h1 {
      font-size: 32px;
      margin-bottom: 10px;
      background: linear-gradient(90deg, #00ff88, #00d4ff);
      -webkit-background-clip: text;
      -webkit-text-fill-color: transparent;
      background-clip: text;
    }
    .subtitle { color: #888; margin-bottom: 30px; font-size: 14px; }
    .card {
      background: rgba(255,255,255,0.05);
      border: 1px solid rgba(255,255,255,0.1);
      border-radius: 12px;
      padding: 24px;
      margin-bottom: 20px;
      backdrop-filter: blur(10px);
    }
    .card h2 {
      color: #00ff88;
      margin-bottom: 16px;
      font-size: 20px;
      display: flex;
      align-items: center;
      gap: 10px;
    }
    .metrics {
      display: grid;
      grid-template-columns: repeat(auto-fit, minmax(200px, 1fr));
      gap: 16px;
      margin-top: 16px;
    }
    .metric {
      background: rgba(0,255,136,0.1);
      border-left: 3px solid #00ff88;
      padding: 12px;
      border-radius: 6px;
    }
    .metric-label { font-size: 12px; color: #888; margin-bottom: 4px; }
    .metric-value {
      font-size: 24px;
      font-weight: bold;
      color: #00ff88;
    }
    .feature-list {
      list-style: none;
      margin-top: 12px;
    }
    .feature-list li {
      padding: 8px 0;
      border-bottom: 1px solid rgba(255,255,255,0.05);
      display: flex;
      align-items: center;
      gap: 10px;
    }
    .feature-list li:last-child { border-bottom: none; }
    .status-dot {
      width: 8px;
      height: 8px;
      border-radius: 50%;
      background: #00ff88;
      animation: pulse 2s infinite;
    }
    @keyframes pulse {
      0%, 100% { opacity: 1; }
      50% { opacity: 0.5; }
    }
    .endpoints { margin-top: 12px; }
    .endpoint {
      background: rgba(0,0,0,0.2);
      padding: 8px 12px;
      margin: 4px 0;
      border-radius: 4px;
      font-family: monospace;
      font-size: 13px;
    }
    .endpoint a {
      color: #00d4ff;
      text-decoration: none;
    }
    .endpoint a:hover { text-decoration: underline; }
    .loading { color: #ff8800; }
    .error { color: #ff0088; }
    .success { color: #00ff88; }
  </style>
</head>
<body>
  <div class="container">
    <h1>🧠 OMNISIGHT Phase 4</h1>
    <div class="subtitle">Precognitive Security System with Claude Flow Integration</div>

    <div class="card">
      <h2>📊 System Status</h2>
      <div class="metrics">
        <div class="metric">
          <div class="metric-label">Version</div>
          <div class="metric-value">0.4.1</div>
        </div>
        <div class="metric">
          <div class="metric-label">Mode</div>
          <div class="metric-value">Phase 4</div>
        </div>
        <div class="metric">
          <div class="metric-label">Claude Flow</div>
          <div class="metric-value" id="claude-status">Loading...</div>
        </div>
        <div class="metric">
          <div class="metric-label">Health</div>
          <div class="metric-value" id="health-status">Checking...</div>
        </div>
      </div>
    </div>

    <div class="card">
      <h2>🚀 Phase 4 Features</h2>
      <ul class="feature-list">
        <li><span class="status-dot"></span><strong>Enhanced Swarm Coordination</strong> - Adaptive topology with Byzantine fault tolerance</li>
        <li><span class="status-dot"></span><strong>Neural Timeline Predictions</strong> - 27+ AI models for future analysis</li>
        <li><span class="status-dot"></span><strong>Federated Learning</strong> - Privacy-preserving distributed learning</li>
        <li><span class="status-dot"></span><strong>Performance Monitoring</strong> - Real-time optimization and bottleneck detection</li>
      </ul>
    </div>

    <div class="card">
      <h2>🔌 API Endpoints</h2>
      <div class="endpoints">
        <div class="endpoint"><a href="/api/claude-flow/status">/api/claude-flow/status</a> - Integration status</div>
        <div class="endpoint"><a href="/api/claude-flow/performance/metrics">/api/claude-flow/performance/metrics</a> - Performance metrics</div>
        <div class="endpoint"><a href="/api/claude-flow/performance/report">/api/claude-flow/performance/report</a> - Full report</div>
        <div class="endpoint"><a href="/api/system/stats">/api/system/stats</a> - System statistics</div>
      </div>
    </div>
  </div>

  <script>
    // Check Claude Flow status
    fetch('/api/claude-flow/status')
      .then(r => r.json())
      .then(data => {
        const status = document.getElementById('claude-status');
        status.textContent = data.enabled ? 'Active' : 'Fallback';
        status.className = data.enabled ? 'success' : 'loading';
      })
      .catch(e => {
        document.getElementById('claude-status').textContent = 'Error';
        document.getElementById('claude-status').className = 'error';
      });

    // Check system health
    fetch('/api/health')
      .then(r => r.json())
      .then(data => {
        const health = document.getElementById('health-status');
        health.textContent = data.status === 'ok' ? 'Online' : 'Degraded';
        health.className = data.status === 'ok' ? 'success' : 'loading';
      })
      .catch(e => {
        document.getElementById('health-status').textContent = 'Offline';
        document.getElementById('health-status').className = 'error';
      });
  </script>
</body>
</html>
HTMLEOF
echo "  ✓ Enhanced HTML dashboard created"

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
echo "✓ Phase 4 V2 Package Build Complete!"
echo "================================================"
echo ""
echo "Package: $OUTPUT_DIR/${PACKAGE_NAME}.eap"
echo "Size: $PACKAGE_SIZE"
echo "Version: $VERSION"
echo ""
echo "Changes from V1:"
echo "  ✓ Based on working Phase 2 structure"
echo "  ✓ Wrapper script instead of shell script"
echo "  ✓ STARTMODE=respawn in package.conf"
echo "  ✓ Enhanced HTML dashboard with live status"
echo "  ✓ Proper process management for stop/start"
echo ""
echo "Phase 4 Features:"
echo "  • Enhanced Swarm Coordination"
echo "  • Neural Timeline Predictions (27+ models)"
echo "  • Federated Learning (privacy-preserving)"
echo "  • Performance Monitoring"
echo ""
echo "Ready for deployment to P3285-LVE!"
echo ""
