#!/bin/bash
# Build OMNISIGHT v0.5.0 - Full React Dashboard with Proven Minimal Pattern

set -e

PROJECT_ROOT="/Users/matthewvisher/Omnisight dev/OmniSight-dev"
PACKAGE_DIR="$PROJECT_ROOT/package-v050"
OUTPUT_DIR="$PROJECT_ROOT/packages/v050"
FRONTEND_DIR="$PROJECT_ROOT/web/frontend"

echo "=========================================="
echo "OMNISIGHT v0.5.0 - Full Dashboard Build"
echo "Using Proven Minimal Pattern from Clean Build"
echo "=========================================="

# Clean directories
echo "Cleaning build directories..."
rm -rf "$PACKAGE_DIR"
rm -rf "$OUTPUT_DIR"
mkdir -p "$PACKAGE_DIR/html"
mkdir -p "$PACKAGE_DIR/lib"
mkdir -p "$OUTPUT_DIR"

# Build React frontend
echo ""
echo "Building React frontend..."
cd "$FRONTEND_DIR"
if [ ! -d "node_modules" ]; then
    echo "Installing npm dependencies..."
    npm install
fi

echo "Running production build..."
npm run build

# Copy React build to package
echo ""
echo "Copying React build to package..."
cp -r "$FRONTEND_DIR/dist/"* "$PACKAGE_DIR/html/"

# Create manifest.json (Schema 1.5 - proven to work)
echo ""
echo "Creating manifest.json (Schema 1.5)..."
cat > "$PACKAGE_DIR/manifest.json" <<'EOF'
{
  "schemaVersion": "1.5",
  "acapPackageConf": {
    "setup": {
      "appName": "omnisightv2",
      "friendlyName": "OMNISIGHT v2",
      "vendor": "OMNISIGHT",
      "vendorUrl": "https://github.com/oneshot2001/OmniSight-dev",
      "version": "0.5.0",
      "embeddedSdkVersion": "3.0",
      "runMode": "never",
      "architecture": "aarch64"
    },
    "configuration": {
      "settingPage": "index.html"
    }
  }
}
EOF

# Create package.conf (Minimal - proven to work)
echo "Creating package.conf..."
cat > "$PACKAGE_DIR/package.conf" <<'EOF'
PACKAGENAME="OMNISIGHT v2"
APPTYPE="aarch64"
APPNAME="omnisightv2"
APPMAJORVERSION="0"
APPMINORVERSION="5"
APPMICROVERSION="0"
APPGRP="sdk"
APPUSR="sdk"
APPOPTS=""
OTHERFILES=""
SETTINGSPAGEFILE="index.html"
STARTMODE="never"
EOF

# Create empty param.conf
echo "Creating param.conf..."
touch "$PACKAGE_DIR/param.conf"

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

# Create package
echo ""
echo "Creating .eap package..."
cd "$PACKAGE_DIR"
tar czf "$OUTPUT_DIR/OMNISIGHT_v2_0_5_0_aarch64.eap" .

# Get package size
PACKAGE_SIZE=$(du -h "$OUTPUT_DIR/OMNISIGHT_v2_0_5_0_aarch64.eap" | cut -f1)

echo ""
echo "=========================================="
echo "✅ v0.5.0 Build Complete!"
echo "=========================================="
echo ""
echo "Package: OMNISIGHT_v2_0_5_0_aarch64.eap"
echo "Size: $PACKAGE_SIZE"
echo "Location: $OUTPUT_DIR/"
echo ""
echo "Pattern Used:"
echo "  ✓ Schema 1.5 (proven to work)"
echo "  ✓ New app name: omnisightv2 (avoid cached configs)"
echo "  ✓ Minimal manifest (settingPage only)"
echo "  ✓ Minimal package.conf (SETTINGSPAGEFILE)"
echo "  ✓ Full React dashboard included"
echo "  ✓ No binary, no reverseProxy complexity"
echo ""
echo "Ready to deploy to P3285-LVE!"
echo "=========================================="
