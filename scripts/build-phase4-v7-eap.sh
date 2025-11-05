#!/bin/bash
#
# Phase 4 V7 Build Script - Based on Working v0.3.1 Structure
#
# ROOT CAUSE: Open button requires settingPage in both manifest.json AND package.conf
# SOLUTION: Add settingPage back, but NO binary execution (dummy binary)
#

set -e

PROJECT_ROOT="/Users/matthewvisher/Omnisight dev/OmniSight-dev"
PHASE2_BASE="$PROJECT_ROOT/packages/phase3/OMNISIGHT_WORKING_0_3_1_aarch64.eap"
PACKAGE_DIR="$PROJECT_ROOT/package-phase4-v7"
OUTPUT_DIR="$PROJECT_ROOT/packages/phase4"

echo "================================================"
echo "OMNISIGHT Phase 4 V7 ACAP Package Builder"
echo "Based on Working v0.3.1 Structure"
echo "================================================"
echo ""
echo "Analysis: v0.3.1 had 'Open' button because:"
echo "  - settingPage defined in manifest.json"
echo "  - SETTINGSPAGEFILE defined in package.conf"
echo "  - Even with runMode='never', these trigger the button"
echo ""

# Clean and create directories
rm -rf "$PACKAGE_DIR"
mkdir -p "$PACKAGE_DIR"
mkdir -p "$OUTPUT_DIR"

echo "Step 1: Extracting working v0.3.1 base..."
tar xzf "$PHASE2_BASE" -C "$PACKAGE_DIR"
echo "  ✓ v0.3.1 base extracted"

echo ""
echo "Step 2: Removing reverseProxy (doesn't work without real server)..."
# Keep the binary, keep package.conf structure, just update versions

echo ""
echo "Step 3: Updating package.conf with v0.4.7..."
cat > "$PACKAGE_DIR/package.conf" <<'EOF'
PACKAGENAME="OMNISIGHT - Precognitive Security"
APPTYPE="aarch64"
APPNAME="omnisight"
APPID=""
LICENSENAME="Available"
LICENSEPAGE="none"
VENDOR="OMNISIGHT"
REQEMBDEVVERSION="3.0"
APPMAJORVERSION="0"
APPMINORVERSION="4"
APPMICROVERSION="7"
APPGRP="sdk"
APPUSR="sdk"
APPOPTS=""
OTHERFILES=""
SETTINGSPAGEFILE="index.html"
SETTINGSPAGETEXT=""
VENDORHOMEPAGELINK=""
PREUPGRADESCRIPT=""
POSTINSTALLSCRIPT=""
STARTMODE="never"
HTTPCGIPATHS="/local/omnisight/html/:omnisight"
EOF
echo "  ✓ package.conf updated (v0.4.7, SETTINGSPAGEFILE='index.html')"

echo ""
echo "Step 4: Updating manifest with settingPage..."
cat > "$PACKAGE_DIR/manifest.json" <<'MEOF'
{
  "schemaVersion": "1.8.0",
  "acapPackageConf": {
    "setup": {
      "appName": "omnisight",
      "friendlyName": "OMNISIGHT v0.4.7",
      "vendor": "OMNISIGHT",
      "vendorUrl": "https://github.com/oneshot2001/OmniSight-dev",
      "embeddedSdkVersion": "3.0",
      "version": "0.4.7",
      "appId": 1009,
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
MEOF
echo "  ✓ Manifest updated (v0.4.7, AppID 1009, WITH settingPage)"

echo ""
echo "Step 5: Updating HTML dashboard..."
cat > "$PACKAGE_DIR/html/index.html" <<'HTMLEOF'
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>OMNISIGHT v0.4.7 - OPEN BUTTON TEST</title>
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
        .success {
            background: #10b981;
            color: white;
            padding: 20px;
            border-radius: 12px;
            text-align: center;
            margin: 20px 0;
            font-size: 1.5em;
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
            color: #888;
            font-size: 0.9em;
        }
        .metric-value {
            color: #667eea;
            font-size: 1.8em;
            font-weight: bold;
        }
        .info {
            background: #fff9e6;
            border-left: 4px solid #fbbf24;
            padding: 15px;
            margin: 15px 0;
            border-radius: 8px;
        }
        .feature-list {
            list-style: none;
            padding: 0;
        }
        .feature-list li {
            padding: 10px 0;
            border-bottom: 1px solid #eee;
        }
        .feature-list li:last-child {
            border-bottom: none;
        }
        .badge {
            display: inline-block;
            background: #10b981;
            color: white;
            padding: 4px 12px;
            border-radius: 12px;
            font-size: 0.85em;
            margin-left: 10px;
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>🎯 OMNISIGHT v0.4.7</h1>
        <div class="tagline" style="color: #666; font-size: 1.2em; margin-bottom: 30px;">
            Precognitive Security System
        </div>

        <div class="success">
            ✓ OPEN BUTTON TEST - If you see this, the Open button worked!
        </div>

        <div class="status">
            <h3>📊 System Status</h3>
            <div class="metric">
                <div class="metric-label">Status</div>
                <div class="metric-value">ACTIVE</div>
            </div>
            <div class="metric">
                <div class="metric-label">Version</div>
                <div class="metric-value">0.4.7</div>
            </div>
            <div class="metric">
                <div class="metric-label">AppID</div>
                <div class="metric-value">1009</div>
            </div>
        </div>

        <div class="info">
            <h3 style="color: #d97706; margin-bottom: 10px;">🔑 Key Configuration</h3>
            <ul class="feature-list">
                <li>✓ <strong>settingPage:</strong> "index.html" in manifest.json <span class="badge">ENABLED</span></li>
                <li>✓ <strong>SETTINGSPAGEFILE:</strong> "index.html" in package.conf <span class="badge">ENABLED</span></li>
                <li>✓ <strong>Binary:</strong> Placeholder binary from v0.3.1 <span class="badge">PRESENT</span></li>
                <li>✓ <strong>runMode:</strong> "never" (binary won't execute) <span class="badge">SAFE</span></li>
            </ul>
        </div>

        <div class="status">
            <h3>📋 Based on Working v0.3.1</h3>
            <p>This version replicates the structure of v0.3.1 which had a working "Open" button:</p>
            <ul class="feature-list">
                <li><strong>Manifest:</strong> Schema 1.8.0 with settingPage</li>
                <li><strong>Package.conf:</strong> SETTINGSPAGEFILE defined</li>
                <li><strong>Binary:</strong> Included (but won't run with STARTMODE=never)</li>
                <li><strong>HTML:</strong> Static files served via httpConfig</li>
            </ul>
        </div>

        <div class="info">
            <h3 style="color: #d97706; margin-bottom: 10px;">ℹ️ Access Information</h3>
            <ul class="feature-list">
                <li><strong>Direct URL:</strong> http://&lt;camera-ip&gt;/local/omnisight/html/</li>
                <li><strong>Camera:</strong> P3285-LVE (ARTPEC-9)</li>
                <li><strong>Firmware:</strong> AXIS OS 12.6.97</li>
                <li><strong>Authentication:</strong> Camera admin credentials</li>
            </ul>
        </div>

        <div style="margin-top: 30px; padding: 20px; background: #f9fafb; border-radius: 12px; text-align: center;">
            <p style="color: #666; margin-bottom: 10px;">If you're seeing this page, the "Open" button successfully appeared and worked!</p>
            <p style="color: #10b981; font-weight: bold; font-size: 1.2em;">✓ SUCCESS!</p>
        </div>
    </div>
</body>
</html>
HTMLEOF
echo "  ✓ HTML updated with Open Button success message"

echo ""
echo "Step 6: Creating package archive..."
cd "$PACKAGE_DIR"
tar czf "$OUTPUT_DIR/OMNISIGHT_v047_aarch64.eap" .
cd - > /dev/null
echo "  ✓ Package archive created"

echo ""
echo "================================================"
echo "✓ Phase 4 V7 Package Build Complete!"
echo "================================================"
echo ""
echo "Package: $OUTPUT_DIR/OMNISIGHT_v047_aarch64.eap"
echo "Size: $(ls -lh "$OUTPUT_DIR/OMNISIGHT_v047_aarch64.eap" | awk '{print $5}')"
echo "Version: 0.4.7"
echo ""
echo "BASED ON WORKING v0.3.1:"
echo "  ✓ settingPage in manifest.json"
echo "  ✓ SETTINGSPAGEFILE in package.conf"
echo "  ✓ Binary included (from v0.3.1)"
echo "  ✓ runMode='never' (binary won't execute)"
echo "  ✓ httpConfig for static file serving"
echo ""
echo "EXPECTED RESULT: 'Open' button SHOULD appear!"
echo ""
echo "Ready for deployment to P3285-LVE!"
echo ""
