#!/bin/bash
# OMNISIGHT Stub ACAP Package Builder
# Creates .eap package for stub binary deployment testing

set -e  # Exit on error

# Configuration
VERSION="0.3.0-stub"
APP_NAME="omnisight"
FRIENDLY_NAME="OMNISIGHT - Stub Demo"
PACKAGE_NAME="OMNISIGHT_Stub_${VERSION//./_}_aarch64.eap"

# Paths
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
BUILD_DIR="$PROJECT_ROOT/build-stub"
PACKAGE_DIR="$PROJECT_ROOT/package-stub"
OUTPUT_DIR="$PROJECT_ROOT/output"

echo "============================================"
echo "OMNISIGHT Stub ACAP Builder v${VERSION}"
echo "============================================"
echo "Project Root: $PROJECT_ROOT"
echo "Build Dir:    $BUILD_DIR"
echo "Package Dir:  $PACKAGE_DIR"
echo "Package:      $PACKAGE_NAME"
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

# Step 3: Copy stub binary
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
    # Create minimal HTML if original not found
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
            <p><strong>Status:</strong> Running (Stub Mode)</p>
            <p><strong>Version:</strong> 0.3.0-stub</p>
        </div>

        <div class="features">
            <h3>Stub Modules Active</h3>
            <div class="feature">Perception Engine (simulated)</div>
            <div class="feature">Timeline Threading (simulated)</div>
            <div class="feature">Swarm Intelligence (simulated)</div>
        </div>

        <div class="version">
            OMNISIGHT v0.3.0-stub | Deployment Test Build<br>
            © 2024-2025 OMNISIGHT Project
        </div>
    </div>
</body>
</html>
EOF
    echo "✓ Created minimal HTML dashboard"
fi

# Step 5: Create manifest
echo ""
echo "Creating manifest..."
cat > "$PACKAGE_DIR/manifest.json" << EOF
{
  "schemaVersion": "1.5",
  "acapPackageConf": {
    "setup": {
      "appName": "$APP_NAME",
      "embeddedSdkVersion": "3.0",
      "friendlyName": "$FRIENDLY_NAME",
      "runMode": "never",
      "vendor": "OMNISIGHT Project",
      "version": "$VERSION"
    },
    "configuration": {
      "httpConfig": [
        {
          "access": "viewer",
          "name": "OMNISIGHT Stub",
          "path": "html/index.html",
          "type": "transferRx"
        }
      ]
    }
  }
}
EOF
echo "✓ Manifest created"

# Step 6: Create LICENSE
echo ""
echo "Creating LICENSE..."
cat > "$PACKAGE_DIR/LICENSE" << 'EOF'
Copyright (c) 2024-2025 OMNISIGHT Project

Permission is hereby granted to use this software on Axis Communications
cameras for evaluation and development purposes.

This is stub/demo software for testing deployment workflows.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
EOF

# Copy LICENSE to project root (eap-create.sh requirement)
cp "$PACKAGE_DIR/LICENSE" "$PROJECT_ROOT/LICENSE"
echo "✓ LICENSE created"

# Step 7: Create package.conf (for acap-build)
echo ""
echo "Creating package.conf..."
cat > "$PACKAGE_DIR/package.conf" << EOF
APPNAME="$APP_NAME"
APPTYPE="stub"
APPID=1001
VENDOR="OMNISIGHT Project"
EOF
echo "✓ package.conf created"

# Step 8: Show package contents
echo ""
echo "Package contents:"
echo "----------------------------------------"
find "$PACKAGE_DIR" -type f | sed "s|$PACKAGE_DIR/||" | sort
echo "----------------------------------------"
echo "Total size: $(du -sh "$PACKAGE_DIR" | cut -f1)"

# Step 9: Build .eap package using tar
echo ""
echo "Building .eap package..."
cd "$PACKAGE_DIR"

# Create tarball
tar -czf "../$PACKAGE_NAME" *

# Move to output directory
mv "../$PACKAGE_NAME" "$OUTPUT_DIR/"

# Get package size
PACKAGE_SIZE=$(stat -f%z "$OUTPUT_DIR/$PACKAGE_NAME" 2>/dev/null || stat -c%s "$OUTPUT_DIR/$PACKAGE_NAME")
PACKAGE_SIZE_KB=$((PACKAGE_SIZE / 1024))

echo ""
echo "============================================"
echo "✓ Package built successfully!"
echo "============================================"
echo "Package: $PACKAGE_NAME"
echo "Size:    ${PACKAGE_SIZE_KB} KB"
echo "Location: $OUTPUT_DIR/$PACKAGE_NAME"
echo "============================================"
echo ""
echo "Next steps:"
echo ""
echo "1. Copy package to host machine:"
echo "   docker cp omnisight-dev:/opt/app/output/$PACKAGE_NAME ./"
echo ""
echo "2. Deploy to camera (via web UI):"
echo "   https://camera-ip → Settings → Apps → Upload"
echo ""
echo "3. Or deploy via CLI:"
echo "   curl -k --anyauth -u root:password \\"
echo "     -F 'package=@$PACKAGE_NAME' \\"
echo "     https://camera-ip/axis-cgi/applications/upload.cgi"
echo ""
echo "4. Access web interface:"
echo "   https://camera-ip/local/$APP_NAME/"
echo ""
echo "============================================"
