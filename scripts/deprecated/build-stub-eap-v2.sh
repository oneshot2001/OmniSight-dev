#!/bin/bash
# OMNISIGHT Stub ACAP Package Builder v2
# Uses official acap-build tool for proper ACAP formatting

set -e  # Exit on error

# Configuration
VERSION="0.3.1-stub"
APP_NAME="omnisight"
FRIENDLY_NAME="OMNISIGHT - Stub Demo"

# Paths
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
BUILD_DIR="$PROJECT_ROOT/build-stub"
PACKAGE_DIR="$PROJECT_ROOT/package-stub-v2"
OUTPUT_DIR="$PROJECT_ROOT/output"

echo "============================================"
echo "OMNISIGHT Stub ACAP Builder v2 (acap-build)"
echo "============================================"
echo "Project Root: $PROJECT_ROOT"
echo "Build Dir:    $BUILD_DIR"
echo "Package Dir:  $PACKAGE_DIR"
echo "============================================"

# Step 1: Build stub binary if needed
if [ ! -f "$BUILD_DIR/omnisight" ]; then
    echo "Building stub binary..."
    cd "$PROJECT_ROOT"
    ./scripts/build-stub.sh
fi

if [ ! -f "$BUILD_DIR/omnisight" ]; then
    echo "ERROR: Stub binary not found at $BUILD_DIR/omnisight"
    exit 1
fi

echo "✓ Stub binary found ($(stat -f%z "$BUILD_DIR/omnisight" 2>/dev/null || stat -c%s "$BUILD_DIR/omnisight") bytes)"

# Step 2: Clean and create package structure
echo ""
echo "Creating package structure..."
rm -rf "$PACKAGE_DIR"
mkdir -p "$PACKAGE_DIR/html"
mkdir -p "$OUTPUT_DIR"

# Step 3: Copy stub binary as main executable
echo "Copying stub binary..."
cp "$BUILD_DIR/omnisight" "$PACKAGE_DIR/"
chmod +x "$PACKAGE_DIR/omnisight"
echo "✓ Stub binary copied"

# Step 4: Copy HTML dashboard
echo ""
echo "Copying HTML dashboard..."
if [ -f "$PROJECT_ROOT/package/html/index.html" ]; then
    cp "$PROJECT_ROOT/package/html/index.html" "$PACKAGE_DIR/html/"
    [ -d "$PROJECT_ROOT/package/html/img" ] && cp -r "$PROJECT_ROOT/package/html/img" "$PACKAGE_DIR/html/" || true
    echo "✓ HTML dashboard copied"
else
    # Create minimal HTML
    cat > "$PACKAGE_DIR/html/index.html" << 'EOF'
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>OMNISIGHT Stub</title>
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
            max-width: 600px;
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
        <div class="tagline">Stub Demo Build</div>

        <div class="status">
            <h3>System Status</h3>
            <p><strong>Status:</strong> Deployed Successfully</p>
            <p><strong>Version:</strong> 0.3.1-stub</p>
        </div>

        <div>
            <h3>Stub Modules</h3>
            <div class="feature">Perception Engine (simulated)</div>
            <div class="feature">Timeline Threading (simulated)</div>
            <div class="feature">Swarm Intelligence (simulated)</div>
        </div>

        <div class="version">
            OMNISIGHT v0.3.1-stub<br>
            Deployment Test Build
        </div>
    </div>
</body>
</html>
EOF
    echo "✓ Created minimal HTML dashboard"
fi

# Step 5: Create manifest (matching working package format)
echo ""
echo "Creating manifest..."
cat > "$PACKAGE_DIR/manifest.json" << EOF
{
  "schemaVersion": "1.8.0",
  "acapPackageConf": {
    "setup": {
      "appName": "$APP_NAME",
      "friendlyName": "$FRIENDLY_NAME",
      "vendor": "OMNISIGHT",
      "version": "$VERSION",
      "embeddedSdkVersion": "3.0",
      "runMode": "never",
      "architecture": "aarch64"
    },
    "configuration": {
      "settingPage": "html/index.html"
    }
  }
}
EOF
echo "✓ Manifest created (schema 1.8.0)"

# Step 6: Create LICENSE
echo ""
echo "Creating LICENSE..."
cat > "$PACKAGE_DIR/LICENSE" << 'EOF'
MIT License - OMNISIGHT Stub
EOF
echo "✓ LICENSE created"

# Step 7: Create package.conf (required by acap-build)
echo ""
echo "Creating package.conf..."
cat > "$PACKAGE_DIR/package.conf" << EOF
APPNAME="$APP_NAME"
APPGRP="applications"
APPTYPE="executable"
LICENSEPAGE=""
LICENSEFILE="LICENSE"
APPID=1001
VENDOR="OMNISIGHT"
VENDORHOMEPAGELINK=""
OTHERFILES=""
HTTPCGIPATHS=""
EOF
echo "✓ package.conf created"

# Step 8: Show package contents
echo ""
echo "Package contents:"
echo "----------------------------------------"
find "$PACKAGE_DIR" -type f | sed "s|$PACKAGE_DIR/||" | sort
echo "----------------------------------------"
echo "Total size: $(du -sh "$PACKAGE_DIR" | cut -f1)"

# Step 9: Build .eap using acap-build
echo ""
echo "Building .eap package with acap-build..."
echo "Sourcing ACAP SDK environment..."

# Source SDK environment and build
. /opt/axis/acapsdk/environment-setup-cortexa53-crypto-poky-linux

cd "$PACKAGE_DIR"

# Clean any previous build artifacts
rm -f *.eap 2>/dev/null || true

# Use acap-build with no-build option (we already built the binary)
echo "Running acap-build..."
acap-build \
    --build no-build \
    --disable-manifest-validation \
    .

# Find the generated .eap file
EAP_FILE=$(ls *.eap 2>/dev/null | head -1)

if [ -z "$EAP_FILE" ]; then
    echo "ERROR: No .eap file generated"
    exit 1
fi

# Move to output directory
mv "$EAP_FILE" "$OUTPUT_DIR/"

# Get package size
PACKAGE_SIZE=$(stat -f%z "$OUTPUT_DIR/$EAP_FILE" 2>/dev/null || stat -c%s "$OUTPUT_DIR/$EAP_FILE")
PACKAGE_SIZE_KB=$((PACKAGE_SIZE / 1024))

echo ""
echo "============================================"
echo "✓ Package built successfully with acap-build!"
echo "============================================"
echo "Package: $EAP_FILE"
echo "Size:    ${PACKAGE_SIZE_KB} KB"
echo "Location: $OUTPUT_DIR/$EAP_FILE"
echo "============================================"
echo ""
echo "This package was built with official acap-build tool"
echo "and should be compatible with P3285-LVE camera."
echo ""
echo "Next steps:"
echo ""
echo "1. Copy package to host machine:"
echo "   docker cp omnisight-dev:/opt/app/output/$EAP_FILE ./"
echo ""
echo "2. Deploy to camera via web UI:"
echo "   https://camera-ip → Settings → Apps → Upload"
echo ""
echo "3. Access web interface:"
echo "   https://camera-ip/local/$APP_NAME/html/index.html"
echo ""
echo "============================================"
