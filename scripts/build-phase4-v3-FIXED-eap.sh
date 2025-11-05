#!/bin/bash
#
# Phase 4 V3 FIXED Build Script - Static Files via Apache httpConfig
#
# ROOT CAUSE: reverseProxy expects a running HTTP server on port 8080
# SOLUTION: Use httpConfig to serve static files directly via Apache
#

set -e

PROJECT_ROOT="/Users/matthewvisher/Omnisight dev/OmniSight-dev"
PHASE2_BASE="$PROJECT_ROOT/packages/phase3/OMNISIGHT_WORKING_0_3_1_aarch64.eap"
PACKAGE_DIR="$PROJECT_ROOT/package-phase4-v3-fixed"
OUTPUT_DIR="$PROJECT_ROOT/packages/phase4"

echo "================================================"
echo "OMNISIGHT Phase 4 V3 FIXED ACAP Package Builder"
echo "Static Files via Apache httpConfig"
echo "================================================"
echo ""
echo "Root Cause: reverseProxy needs HTTP server on port 8080"
echo "Solution: Use httpConfig to serve static files via Apache"
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
echo "Step 3: Creating manifest with httpConfig..."
cat > "$PACKAGE_DIR/manifest.json" <<'EOF'
{
  "schemaVersion": "1.8.0",
  "acapPackageConf": {
    "setup": {
      "appName": "omnisight",
      "friendlyName": "OMNISIGHT - Phase 4 V3 Fixed",
      "vendor": "OMNISIGHT",
      "vendorUrl": "https://github.com/oneshot2001/OmniSight-dev",
      "embeddedSdkVersion": "3.0",
      "version": "0.4.3",
      "appId": 1005,
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
echo "  ✓ Manifest created with httpConfig (runMode: never)"

echo ""
echo "Step 4: Creating enhanced HTML dashboard..."
cat > "$PACKAGE_DIR/html/index.html" <<'HTMLEOF'
<!DOCTYPE html>
<html>
<head>
  <title>OMNISIGHT v0.4.3 - Phase 4 V3 Fixed</title>
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
    .logo {
      font-size: 48px;
      font-weight: bold;
      background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
      -webkit-background-clip: text;
      -webkit-text-fill-color: transparent;
      background-clip: text;
      margin-bottom: 10px;
    }
    .version {
      color: #666;
      font-size: 14px;
    }
    .status-card {
      background: #f8f9fa;
      border-radius: 12px;
      padding: 30px;
      margin: 20px 0;
    }
    .status-title {
      font-size: 24px;
      font-weight: 600;
      color: #333;
      margin-bottom: 20px;
    }
    .status-grid {
      display: grid;
      grid-template-columns: repeat(auto-fit, minmax(250px, 1fr));
      gap: 20px;
    }
    .status-item {
      background: white;
      padding: 20px;
      border-radius: 8px;
      border-left: 4px solid #667eea;
    }
    .status-label {
      color: #666;
      font-size: 12px;
      text-transform: uppercase;
      letter-spacing: 1px;
      margin-bottom: 8px;
    }
    .status-value {
      font-size: 24px;
      font-weight: bold;
      color: #333;
    }
    .success { color: #28a745; }
    .warning { color: #ffc107; }
    .info { color: #17a2b8; }
    .message {
      background: #e7f3ff;
      border-left: 4px solid #2196F3;
      padding: 20px;
      border-radius: 8px;
      margin: 20px 0;
    }
    .message-title {
      font-weight: bold;
      color: #1976d2;
      margin-bottom: 10px;
    }
    .features {
      display: grid;
      grid-template-columns: repeat(auto-fit, minmax(300px, 1fr));
      gap: 20px;
      margin-top: 30px;
    }
    .feature-card {
      background: white;
      border: 2px solid #e0e0e0;
      border-radius: 12px;
      padding: 24px;
      transition: all 0.3s ease;
    }
    .feature-card:hover {
      border-color: #667eea;
      box-shadow: 0 4px 12px rgba(102, 126, 234, 0.2);
      transform: translateY(-2px);
    }
    .feature-icon {
      font-size: 32px;
      margin-bottom: 12px;
    }
    .feature-title {
      font-size: 18px;
      font-weight: 600;
      color: #333;
      margin-bottom: 8px;
    }
    .feature-desc {
      color: #666;
      font-size: 14px;
      line-height: 1.6;
    }
  </style>
</head>
<body>
  <div class="container">
    <div class="header">
      <div class="logo">OMNISIGHT</div>
      <div class="version">Phase 4 V3 Fixed - Version 0.4.3</div>
    </div>

    <div class="message">
      <div class="message-title">✅ Deployment Successful!</div>
      <div>
        This version uses Apache httpConfig to serve static files directly, eliminating the need for Python or a running HTTP server process. The web interface is now available via the camera's web UI.
      </div>
    </div>

    <div class="status-card">
      <div class="status-title">System Status</div>
      <div class="status-grid">
        <div class="status-item">
          <div class="status-label">Mode</div>
          <div class="status-value info">Static Demo</div>
        </div>
        <div class="status-item">
          <div class="status-label">Server Type</div>
          <div class="status-value success">Apache httpConfig</div>
        </div>
        <div class="status-item">
          <div class="status-label">Python Required</div>
          <div class="status-value success">No</div>
        </div>
        <div class="status-item">
          <div class="status-label">Camera Compatibility</div>
          <div class="status-value success">Universal</div>
        </div>
      </div>
    </div>

    <div class="status-card">
      <div class="status-title">Core Features</div>
      <div class="features">
        <div class="feature-card">
          <div class="feature-icon">👁️</div>
          <div class="feature-title">Perception Engine</div>
          <div class="feature-desc">Real-time object detection and tracking with behavioral analysis</div>
        </div>
        <div class="feature-card">
          <div class="feature-icon">⏱️</div>
          <div class="feature-title">Timeline Threading™</div>
          <div class="feature-desc">Multi-future prediction with 3-5 probable timelines and confidence scores</div>
        </div>
        <div class="feature-card">
          <div class="feature-icon">🤝</div>
          <div class="feature-title">Swarm Intelligence</div>
          <div class="feature-desc">Multi-camera coordination with privacy-preserving federated learning</div>
        </div>
      </div>
    </div>

    <div class="status-card">
      <div class="status-title">Deployment Details</div>
      <div class="status-grid">
        <div class="status-item">
          <div class="status-label">Camera Model</div>
          <div class="status-value">P3285-LVE</div>
        </div>
        <div class="status-item">
          <div class="status-label">ARTPEC Version</div>
          <div class="status-value">ARTPEC-9</div>
        </div>
        <div class="status-item">
          <div class="status-label">Package Size</div>
          <div class="status-value">~300KB</div>
        </div>
        <div class="status-item">
          <div class="status-label">Schema Version</div>
          <div class="status-value">1.8.0</div>
        </div>
      </div>
    </div>

    <div class="message">
      <div class="message-title">📌 Next Steps</div>
      <div>
        <strong>Phase 5 Development:</strong> This is a static demonstration. Full functionality requires integration with ACAP hardware APIs (VDO, Larod) and implementation of real perception algorithms. Contact the development team for Phase 5 roadmap and timeline.
      </div>
    </div>
  </div>
</body>
</html>
HTMLEOF
echo "  ✓ Enhanced HTML dashboard created"

echo ""
echo "Step 5: Creating package archive..."
cd "$PACKAGE_DIR"
tar czf "$OUTPUT_DIR/OMNISIGHT_-_Precognitive_Security_043_aarch64.eap" ./*
echo "  ✓ Package archive created"

cd "$PROJECT_ROOT"

echo ""
echo "================================================"
echo "✓ Phase 4 V3 FIXED Package Build Complete!"
echo "================================================"
echo ""
echo "Package: $OUTPUT_DIR/OMNISIGHT_-_Precognitive_Security_043_aarch64.eap"
SIZE=$(du -h "$OUTPUT_DIR/OMNISIGHT_-_Precognitive_Security_043_aarch64.eap" | cut -f1)
echo "Size: $SIZE"
echo "Version: 0.4.3"
echo ""
echo "CRITICAL FIX:"
echo "  ✓ Changed from reverseProxy to httpConfig"
echo "  ✓ runMode changed from 'respawn' to 'never'"
echo "  ✓ Static files served directly by Apache"
echo "  ✓ No HTTP server process needed"
echo "  ✓ 'Open' button should now appear!"
echo ""
echo "Ready for deployment to P3285-LVE!"
