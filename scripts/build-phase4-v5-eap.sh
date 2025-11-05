#!/bin/bash
#
# Phase 4 V5 Build Script - Remove package.conf, Use ONLY manifest.json
#
# ROOT CAUSE: package.conf might be conflic ting with manifest.json
# SOLUTION: Remove package.conf entirely, use only manifest.json (schema 1.8.0)
#

set -e

PROJECT_ROOT="/Users/matthewvisher/Omnisight dev/OmniSight-dev"
PHASE2_BASE="$PROJECT_ROOT/packages/phase3/OMNISIGHT_WORKING_0_3_1_aarch64.eap"
PACKAGE_DIR="$PROJECT_ROOT/package-phase4-v5"
OUTPUT_DIR="$PROJECT_ROOT/packages/phase4"

echo "================================================"
echo "OMNISIGHT Phase 4 V5 ACAP Package Builder"
echo "manifest.json ONLY (No package.conf)"
echo "================================================"
echo ""
echo "Root Cause: package.conf may conflict with manifest.json"
echo "Solution: Remove package.conf, rely solely on manifest.json"
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
rm -f "$PACKAGE_DIR/package.conf"  # REMOVE package.conf!
rm -f "$PACKAGE_DIR/param.conf"    # Remove param.conf too
echo "  ✓ Removed: omnisight binary, app/, package.conf, param.conf"

echo ""
echo "Step 3: Creating manifest with httpConfig (NO settingPage!)..."
cat > "$PACKAGE_DIR/manifest.json" <<'EOF'
{
  "schemaVersion": "1.8.0",
  "acapPackageConf": {
    "setup": {
      "appName": "omnisight",
      "friendlyName": "OMNISIGHT v0.4.5",
      "vendor": "OMNISIGHT",
      "vendorUrl": "https://github.com/oneshot2001/OmniSight-dev",
      "embeddedSdkVersion": "3.0",
      "version": "0.4.5",
      "appId": 1007,
      "runMode": "never",
      "architecture": "aarch64"
    }
  },
  "configuration": {
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
echo "  ✓ Manifest created (NO package.conf, NO settingPage)"

echo ""
echo "Step 4: Creating index.html in html/omnisight/..."
mkdir -p "$PACKAGE_DIR/html/omnisight"
cat > "$PACKAGE_DIR/html/omnisight/index.html" <<'HTMLEOF'
<!DOCTYPE html>
<html>
<head>
  <title>OMNISIGHT v0.4.5</title>
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
      max-width: 900px;
      width: 100%;
      padding: 40px;
    }
    .header {
      text-align: center;
      margin-bottom: 30px;
    }
    .header h1 {
      color: #667eea;
      font-size: 2.5em;
      margin-bottom: 10px;
    }
    .version {
      color: #764ba2;
      font-weight: bold;
      font-size: 1.1em;
    }
    .status-grid {
      display: grid;
      grid-template-columns: repeat(auto-fit, minmax(200px, 1fr));
      gap: 15px;
      margin-bottom: 25px;
    }
    .status-card {
      background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
      color: white;
      padding: 20px;
      border-radius: 12px;
      text-align: center;
    }
    .status-card h3 {
      font-size: 0.9em;
      margin-bottom: 8px;
      opacity: 0.9;
    }
    .status-card .value {
      font-size: 1.5em;
      font-weight: bold;
    }
    .info-section {
      background: #f5f7fa;
      padding: 20px;
      border-radius: 12px;
      margin-bottom: 15px;
    }
    .info-section h2 {
      color: #667eea;
      margin-bottom: 12px;
      font-size: 1.3em;
    }
    .feature-list {
      list-style: none;
    }
    .feature-list li {
      padding: 8px 0;
      border-bottom: 1px solid #ddd;
    }
    .feature-list li:last-child {
      border-bottom: none;
    }
    .badge {
      display: inline-block;
      background: #764ba2;
      color: white;
      padding: 3px 10px;
      border-radius: 15px;
      font-size: 0.85em;
      margin-left: 8px;
    }
    .success {
      background: #10b981;
    }
    footer {
      text-align: center;
      margin-top: 25px;
      color: #666;
      font-size: 0.9em;
    }
  </style>
</head>
<body>
  <div class="container">
    <div class="header">
      <h1>OMNISIGHT</h1>
      <p class="version">Version 0.4.5</p>
      <p style="color: #666; margin-top: 8px;">Precognitive Security System</p>
    </div>

    <div class="status-grid">
      <div class="status-card">
        <h3>Web Interface</h3>
        <div class="value">ONLINE</div>
      </div>
      <div class="status-card">
        <h3>AppID</h3>
        <div class="value">1007</div>
      </div>
      <div class="status-card">
        <h3>Schema</h3>
        <div class="value">1.8.0</div>
      </div>
      <div class="status-card">
        <h3>Run Mode</h3>
        <div class="value">STATIC</div>
      </div>
    </div>

    <div class="info-section">
      <h2>✅ v0.4.5 Configuration</h2>
      <ul class="feature-list">
        <li>✓ <strong>NO package.conf</strong> - Only manifest.json <span class="badge success">CLEAN</span></li>
        <li>✓ <strong>NO settingPage</strong> - Prevents auto-linking <span class="badge success">TEST</span></li>
        <li>✓ <strong>httpConfig</strong> - transferCgi type <span class="badge success">ACTIVE</span></li>
        <li>✓ <strong>NEW AppID (1007)</strong> - Fresh installation <span class="badge success">UNIQUE</span></li>
      </ul>
    </div>

    <div class="info-section">
      <h2>🔧 Technical Details</h2>
      <ul class="feature-list">
        <li><strong>Path:</strong> /local/omnisight/omnisight/</li>
        <li><strong>Access:</strong> Admin only (Apache auth)</li>
        <li><strong>Files:</strong> /usr/local/packages/omnisight/html/omnisight/</li>
        <li><strong>Method:</strong> Apache transferCgi</li>
      </ul>
    </div>

    <div class="info-section">
      <h2>🎯 Key Changes in v0.4.5</h2>
      <ul class="feature-list">
        <li><strong>Removed package.conf:</strong> Eliminates conflicts with manifest.json</li>
        <li><strong>Removed settingPage:</strong> Testing if this enables Open button</li>
        <li><strong>httpConfig only:</strong> No reverse proxy configuration</li>
        <li><strong>Path structure:</strong> html/omnisight/index.html for transferCgi</li>
      </ul>
    </div>

    <footer>
      <p>OMNISIGHT - Precognitive Security System</p>
      <p>Phase 4 Development - v0.4.5 (manifest-only)</p>
    </footer>
  </div>
</body>
</html>
HTMLEOF
echo "  ✓ index.html created in html/omnisight/"

echo ""
echo "Step 5: Creating package archive..."
cd "$PACKAGE_DIR"
tar czf "$OUTPUT_DIR/OMNISIGHT_v045_aarch64.eap" .
cd - > /dev/null
echo "  ✓ Package archive created"

echo ""
echo "================================================"
echo "✓ Phase 4 V5 Package Build Complete!"
echo "================================================"
echo ""
echo "Package: $OUTPUT_DIR/OMNISIGHT_v045_aarch64.eap"
echo "Size: $(ls -lh "$OUTPUT_DIR/OMNISIGHT_v045_aarch64.eap" | awk '{print $5}')"
echo "Version: 0.4.5"
echo ""
echo "CRITICAL CHANGES:"
echo "  ✓ Removed package.conf entirely"
echo "  ✓ Removed settingPage from manifest"
echo "  ✓ NEW AppID (1007) for clean install"
echo "  ✓ httpConfig with transferCgi type"
echo "  ✓ Files in html/omnisight/ directory"
echo ""
echo "Access URL: http://<camera-ip>/local/omnisight/omnisight/"
echo ""
echo "Ready for deployment to P3285-LVE!"
echo ""
