#!/bin/bash
# Build OMNISIGHT v0.7.0 - Phase 4 Native C HTTP Server + React Dashboard
#
# This script builds the complete ACAP package with:
# - Native C binary (Mongoose HTTP server + OMNISIGHT core)
# - React dashboard (static HTML/CSS/JS)
# - Unified API + Frontend in single binary
#
# Requirements:
# - Docker (for ARM cross-compilation)
# - Node.js (for React build)

set -e

PROJECT_ROOT="/Users/matthewvisher/Omnisight dev/OmniSight-dev"
PACKAGE_DIR="$PROJECT_ROOT/package-v070"
OUTPUT_DIR="$PROJECT_ROOT/packages"
FRONTEND_DIR="$PROJECT_ROOT/web/frontend"
BUILD_DIR="$PROJECT_ROOT/build-phase4-acap"

echo "=========================================="
echo "OMNISIGHT v0.7.0 - Phase 4 Build"
echo "Native C HTTP Server + React Dashboard"
echo "=========================================="
echo ""

# Clean directories
echo "Cleaning build directories..."
rm -rf "$PACKAGE_DIR"
rm -rf "$BUILD_DIR"
mkdir -p "$PACKAGE_DIR/html"
mkdir -p "$PACKAGE_DIR/lib"
mkdir -p "$BUILD_DIR"
mkdir -p "$OUTPUT_DIR"

echo "✓ Directories cleaned"
echo ""

# Build React frontend
echo "Building React frontend..."
echo "--------------------"
cd "$FRONTEND_DIR"

if [ ! -d "node_modules" ]; then
    echo "Installing npm dependencies..."
    npm install
fi

echo "Running production build..."
npm run build

if [ ! -d "dist" ]; then
    echo "❌ Error: React build failed - dist/ directory not found"
    exit 1
fi

echo "✓ React build complete"
echo ""

# Copy React build to package
echo "Copying React dist to package..."
cp -r "$FRONTEND_DIR/dist/"* "$PACKAGE_DIR/html/"

# Verify files copied
if [ ! -f "$PACKAGE_DIR/html/index.html" ]; then
    echo "❌ Error: index.html not found in package"
    exit 1
fi

ASSET_COUNT=$(ls -1 "$PACKAGE_DIR/html/assets" | wc -l | tr -d ' ')
echo "✓ Copied React files: index.html + $ASSET_COUNT assets"
echo ""

# Build native binary (requires Docker with ACAP SDK)
echo "Building native C binary for ARM..."
echo "--------------------"
echo "⚠️  ARM cross-compilation requires ACAP SDK in Docker"
echo ""

# Check if we're in Docker or have the SDK
if command -v aarch64-linux-gnu-gcc &> /dev/null; then
    echo "✓ Found ARM compiler, building binary..."

    cd "$PROJECT_ROOT"

    # Compile for ARM
    aarch64-linux-gnu-gcc -O2 -Wall -Wextra \
        -I"$PROJECT_ROOT/src" \
        -I"$PROJECT_ROOT/src/perception" \
        -I"$PROJECT_ROOT/src/timeline" \
        -I"$PROJECT_ROOT/src/swarm" \
        -I"$PROJECT_ROOT/src/http" \
        -DMG_ENABLE_PACKED_FS=0 \
        -DOMNISIGHT_STUB_BUILD=1 \
        -DOMNISIGHT_VERSION=\"0.7.0\" \
        -D_GNU_SOURCE \
        -o "$BUILD_DIR/omnisight" \
        "$PROJECT_ROOT/src/main.c" \
        "$PROJECT_ROOT/src/omnisight_core.c" \
        "$PROJECT_ROOT/src/http/http_server_simple.c" \
        "$PROJECT_ROOT/src/http/mongoose.c" \
        "$PROJECT_ROOT/src/perception/perception_stub.c" \
        "$PROJECT_ROOT/src/timeline/timeline_stub.c" \
        "$PROJECT_ROOT/src/swarm/swarm_stub.c" \
        -lpthread -lm -static

    if [ -f "$BUILD_DIR/omnisight" ]; then
        # Strip binary to reduce size
        aarch64-linux-gnu-strip "$BUILD_DIR/omnisight"

        BINARY_SIZE=$(du -h "$BUILD_DIR/omnisight" | cut -f1)
        echo "✓ Binary built: omnisight ($BINARY_SIZE)"

        # Copy binary to package
        cp "$BUILD_DIR/omnisight" "$PACKAGE_DIR/omnisight"
        echo "✓ Binary copied to package"
    else
        echo "❌ Error: Binary compilation failed"
        exit 1
    fi
else
    echo "⚠️  ARM compiler not found"
    echo ""
    echo "To build the ARM binary, run this script in Docker:"
    echo "  docker-compose run omnisight-dev ./scripts/build-phase4-v070.sh"
    echo ""
    echo "Or build locally with x86 binary (for testing manifest only):"
    echo "  ./scripts/build-integration-test.sh"
    echo "  cp build-integration-test/test_integration package-v070/omnisight"
    echo ""
    echo "Continuing without binary (manifest-only package)..."
fi

echo ""

# Create manifest.json (Schema 1.8.0 with reverseProxy)
echo "Creating manifest.json..."
cat > "$PACKAGE_DIR/manifest.json" <<'EOF'
{
  "schemaVersion": "1.8.0",
  "acapPackageConf": {
    "setup": {
      "appName": "omnisight",
      "friendlyName": "OMNISIGHT - Precognitive Security",
      "vendor": "OMNISIGHT",
      "vendorUrl": "https://github.com/oneshot2001/OmniSight-dev",
      "version": "0.7.0",
      "appId": 1001,
      "embeddedSdkVersion": "3.0",
      "user": {
        "username": "sdk",
        "group": "sdk"
      },
      "architecture": "aarch64",
      "runMode": "respawn",
      "runOptions": ""
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
    },
    "resources": {
      "memory": {
        "ram": 128,
        "ramMax": 256
      }
    }
  }
}
EOF

echo "✓ Created manifest.json (Schema 1.8.0)"
echo ""

# Create package.conf
echo "Creating package.conf..."
cat > "$PACKAGE_DIR/package.conf" <<'EOF'
PACKAGENAME="OMNISIGHT"
APPTYPE="aarch64"
APPNAME="omnisight"
APPMAJORVERSION="0"
APPMINORVERSION="7"
APPMICROVERSION="0"
APPGRP="sdk"
APPUSR="sdk"
APPOPTS=""
OTHERFILES="omnisight html"
SETTINGSPAGEFILE="index.html"
STARTMODE="respawn"
EOF

echo "✓ Created package.conf"
echo ""

# Create param.conf
echo "Creating param.conf..."
touch "$PACKAGE_DIR/param.conf"
echo "✓ Created param.conf"
echo ""

# Create LICENSE
echo "Creating LICENSE..."
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

echo "✓ Created LICENSE"
echo ""

# Create startup script
echo "Creating startup script..."
cat > "$PACKAGE_DIR/omnisight_start.sh" <<'STARTEOF'
#!/bin/sh
# OMNISIGHT Startup Script

# Set web root to ACAP installation directory
WEB_ROOT="/usr/local/packages/omnisight/html"

# Start OMNISIGHT server on port 8080
exec /usr/local/packages/omnisight/omnisight --port 8080 --web-root "$WEB_ROOT"
STARTEOF

chmod +x "$PACKAGE_DIR/omnisight_start.sh"
echo "✓ Created startup script"
echo ""

# Create package
echo "Creating .eap package..."
cd "$PACKAGE_DIR"
tar czf "$OUTPUT_DIR/OMNISIGHT_0_7_0_aarch64.eap" .

if [ ! -f "$OUTPUT_DIR/OMNISIGHT_0_7_0_aarch64.eap" ]; then
    echo "❌ Error: Package creation failed"
    exit 1
fi

# Get package info
PACKAGE_SIZE=$(du -h "$OUTPUT_DIR/OMNISIGHT_0_7_0_aarch64.eap" | cut -f1)
BINARY_STATUS="NOT INCLUDED"
if [ -f "$PACKAGE_DIR/omnisight" ]; then
    BINARY_STATUS="INCLUDED"
fi

echo ""
echo "=========================================="
echo "✅ v0.7.0 Build Complete!"
echo "=========================================="
echo ""
echo "Package: OMNISIGHT_0_7_0_aarch64.eap"
echo "Size: $PACKAGE_SIZE"
echo "Location: $OUTPUT_DIR/"
echo ""
echo "Package Contents:"
echo "  ✓ React Dashboard (HTML/CSS/JS)"
echo "  ✓ Manifest (Schema 1.8.0 with reverseProxy)"
echo "  ✓ Binary: $BINARY_STATUS"
echo ""
echo "Architecture:"
echo "  - Single C binary (Mongoose + OMNISIGHT core)"
echo "  - Serves API endpoints (/api/*)"
echo "  - Serves static files (React dashboard)"
echo "  - Port: 8080"
echo "  - Mode: respawn (continuous operation)"
echo ""

if [ "$BINARY_STATUS" = "NOT INCLUDED" ]; then
    echo "⚠️  Next Steps:"
    echo "  1. Build ARM binary in Docker:"
    echo "     docker-compose run omnisight-dev ./scripts/build-phase4-v070.sh"
    echo "  2. Or deploy manifest-only for testing"
    echo ""
fi

echo "Ready to deploy to P3285-LVE (ARTPEC-8)!"
echo "=========================================="
