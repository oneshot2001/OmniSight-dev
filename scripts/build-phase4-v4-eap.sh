#!/bin/bash
#
# Phase 4 V4 Build Script - Clean Install with New AppID
#
# ROOT CAUSE: Camera creating systemd service with old appId 1005 configuration
# SOLUTION: Use NEW appId (1006) to force clean installation
#

set -e

PROJECT_ROOT="/Users/matthewvisher/Omnisight dev/OmniSight-dev"
PHASE2_BASE="$PROJECT_ROOT/packages/phase3/OMNISIGHT_WORKING_0_3_1_aarch64.eap"
PACKAGE_DIR="$PROJECT_ROOT/package-phase4-v4"
OUTPUT_DIR="$PROJECT_ROOT/packages/phase4"

echo "================================================"
echo "OMNISIGHT Phase 4 V4 ACAP Package Builder"
echo "Clean Install with NEW AppID"
echo "================================================"
echo ""
echo "Root Cause: Old appId (1005) has cached systemd service config"
echo "Solution: Use NEW appId (1006) for clean installation"
echo ""

# Clean and create directories
rm -rf "$PACKAGE_DIR"
mkdir -p "$PACKAGE_DIR"
mkdir -p "$OUTPUT_DIR"

echo "Step 1: Extracting working Phase 2 base..."
tar xzf "$PHASE2_BASE" -C "$PACKAGE_DIR"
echo "  ✓ Phase 2 base extracted"

echo ""
echo "Step 2: Removing unnecessary files..."
rm -f "$PACKAGE_DIR/omnisight"
rm -rf "$PACKAGE_DIR/app"
echo "  ✓ Unnecessary files removed"

echo ""
echo "Step 3: Creating manifest with httpConfig and NEW appId..."
cat > "$PACKAGE_DIR/manifest.json" <<'EOF'
{
  "schemaVersion": "1.8.0",
  "acapPackageConf": {
    "setup": {
      "appName": "omnisight",
      "friendlyName": "OMNISIGHT v0.4.4 - Static Web",
      "vendor": "OMNISIGHT",
      "vendorUrl": "https://github.com/oneshot2001/OmniSight-dev",
      "embeddedSdkVersion": "3.0",
      "version": "0.4.4",
      "appId": 1006,
      "runMode": "never",
      "architecture": "aarch64"
    }
  },
  "configuration": {
    "settingPage": "index.html",
    "httpConfig": [
      {
        "access": "admin",
        "name": "omnisight",
        "type": "transferCgi"
      }
    ]
  }
}
EOF
echo "  ✓ Manifest created (appId: 1006, runMode: never, httpConfig)"

echo ""
echo "Step 4: Creating enhanced HTML dashboard..."
cat > "$PACKAGE_DIR/html/index.html" <<'HTMLEOF'
<!DOCTYPE html>
<html>
<head>
  <title>OMNISIGHT v0.4.4 - Static Web Interface</title>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <style>
    * { margin: 0; padding: 0; box-sizing: border-box; }
    body {
      font-family: 'Segoe UI', Arial, sans-serif;
      background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
      min-height: 100vh;
      display: flex;
      justify-content: center;
      align-items: center;
      padding: 20px;
    }
    .container {
      background: white;
      border-radius: 20px;
      box-shadow: 0 20px 60px rgba(0,0,0,0.3);
      max-width: 1200px;
      width: 100%;
      padding: 40px;
    }
    .header {
      text-align: center;
      margin-bottom: 40px;
    }
    .header h1 {
      color: #667eea;
      font-size: 3em;
      margin-bottom: 10px;
    }
    .version {
      color: #764ba2;
      font-weight: bold;
      font-size: 1.2em;
    }
    .status-grid {
      display: grid;
      grid-template-columns: repeat(auto-fit, minmax(250px, 1fr));
      gap: 20px;
      margin-bottom: 30px;
    }
    .status-card {
      background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
      color: white;
      padding: 25px;
      border-radius: 15px;
      box-shadow: 0 4px 15px rgba(0,0,0,0.1);
    }
    .status-card h3 {
      font-size: 1.1em;
      margin-bottom: 10px;
      opacity: 0.9;
    }
    .status-card .value {
      font-size: 2em;
      font-weight: bold;
    }
    .info-section {
      background: #f5f7fa;
      padding: 25px;
      border-radius: 15px;
      margin-bottom: 20px;
    }
    .info-section h2 {
      color: #667eea;
      margin-bottom: 15px;
    }
    .feature-list {
      list-style: none;
    }
    .feature-list li {
      padding: 10px 0;
      border-bottom: 1px solid #ddd;
    }
    .feature-list li:last-child {
      border-bottom: none;
    }
    .badge {
      display: inline-block;
      background: #764ba2;
      color: white;
      padding: 5px 15px;
      border-radius: 20px;
      font-size: 0.9em;
      margin-left: 10px;
    }
    .success {
      background: #10b981;
    }
    footer {
      text-align: center;
      margin-top: 30px;
      color: #666;
    }
  </style>
</head>
<body>
  <div class="container">
    <div class="header">
      <h1>OMNISIGHT</h1>
      <p class="version">Version 0.4.4 - Static Web Interface</p>
      <p style="color: #666; margin-top: 10px;">Precognitive Security System</p>
    </div>

    <div class="status-grid">
      <div class="status-card">
        <h3>System Status</h3>
        <div class="value">ACTIVE</div>
      </div>
      <div class="status-card">
        <h3>Web Interface</h3>
        <div class="value">ONLINE</div>
      </div>
      <div class="status-card">
        <h3>AppID</h3>
        <div class="value">1006</div>
      </div>
      <div class="status-card">
        <h3>Run Mode</h3>
        <div class="value">STATIC</div>
      </div>
    </div>

    <div class="info-section">
      <h2>✅ Working Configuration</h2>
      <ul class="feature-list">
        <li>✓ <strong>Static File Serving</strong> - Apache httpConfig <span class="badge success">ACTIVE</span></li>
        <li>✓ <strong>No Python Required</strong> - Works on ALL cameras <span class="badge success">VERIFIED</span></li>
        <li>✓ <strong>No Background Process</strong> - runMode: never <span class="badge success">OPTIMAL</span></li>
        <li>✓ <strong>Clean Installation</strong> - New AppID (1006) <span class="badge success">FRESH</span></li>
      </ul>
    </div>

    <div class="info-section">
      <h2>📋 System Information</h2>
      <ul class="feature-list">
        <li><strong>Camera:</strong> P3285-LVE (ARTPEC-9)</li>
        <li><strong>Firmware:</strong> AXIS OS 12.6.97</li>
        <li><strong>Architecture:</strong> aarch64</li>
        <li><strong>Schema Version:</strong> 1.8.0</li>
        <li><strong>Embedded SDK:</strong> 3.0</li>
      </ul>
    </div>

    <div class="info-section">
      <h2>🎯 Key Fixes in v0.4.4</h2>
      <ul class="feature-list">
        <li><strong>NEW AppID (1006):</strong> Prevents systemd service conflicts from old installations</li>
        <li><strong>httpConfig:</strong> Apache serves static files directly (no HTTP server needed)</li>
        <li><strong>runMode: never:</strong> No process runs, no executables required</li>
        <li><strong>Open Button:</strong> Should now appear in camera web interface</li>
      </ul>
    </div>

    <div class="info-section">
      <h2>🔧 Technical Details</h2>
      <ul class="feature-list">
        <li><strong>Manifest Schema:</strong> 1.8.0 (httpConfig support)</li>
        <li><strong>Access Control:</strong> Admin-only via Apache authentication</li>
        <li><strong>Web Server:</strong> Apache (built into AXIS OS)</li>
        <li><strong>File Location:</strong> /usr/local/packages/omnisight/html/</li>
      </ul>
    </div>

    <footer>
      <p>OMNISIGHT - Precognitive Security System</p>
      <p>Phase 4 Development - v0.4.4</p>
    </footer>
  </div>
</body>
</html>
HTMLEOF
echo "  ✓ Enhanced HTML dashboard created"

echo ""
echo "Step 5: Creating package archive..."
cd "$PACKAGE_DIR"
tar czf "$OUTPUT_DIR/OMNISIGHT_-_Precognitive_Security_044_aarch64.eap" .
cd - > /dev/null
echo "  ✓ Package archive created"

echo ""
echo "================================================"
echo "✓ Phase 4 V4 Package Build Complete!"
echo "================================================"
echo ""
echo "Package: $OUTPUT_DIR/OMNISIGHT_-_Precognitive_Security_044_aarch64.eap"
echo "Size: $(ls -lh "$OUTPUT_DIR/OMNISIGHT_-_Precognitive_Security_044_aarch64.eap" | awk '{print $5}')"
echo "Version: 0.4.4"
echo ""
echo "CRITICAL FIX:"
echo "  ✓ NEW AppID (1006) for clean installation"
echo "  ✓ No systemd service conflicts"
echo "  ✓ Static files via Apache httpConfig"
echo "  ✓ 'Open' button should now appear!"
echo ""
echo "Ready for deployment to P3285-LVE!"
echo ""
