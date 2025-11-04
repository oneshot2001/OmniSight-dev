#!/bin/bash
# Build OMNISIGHT v0.6.1 - Phase 3: Fixed Python Path

set -e

PROJECT_ROOT="/Users/matthewvisher/Omnisight dev/OmniSight-dev"
PACKAGE_DIR="$PROJECT_ROOT/package-v061"
OUTPUT_DIR="$PROJECT_ROOT/packages/v061"
FRONTEND_DIR="$PROJECT_ROOT/web/frontend"
APP_DIR="$PROJECT_ROOT/app"

echo "=========================================="
echo "OMNISIGHT v0.6.1 - Phase 3 (Fixed)"
echo "Corrected Python Path for AXIS OS"
echo "=========================================="

# Clean directories
echo ""
echo "Cleaning build directories..."
rm -rf "$PACKAGE_DIR"
rm -rf "$OUTPUT_DIR"
mkdir -p "$PACKAGE_DIR/html"
mkdir -p "$PACKAGE_DIR/app"
mkdir -p "$PACKAGE_DIR/lib"
mkdir -p "$OUTPUT_DIR"

# Copy React build
echo ""
echo "Copying React dashboard..."
cp -r "$FRONTEND_DIR/dist/"* "$PACKAGE_DIR/html/"

# Copy Flask app
echo ""
echo "Copying Flask API server..."
cp "$APP_DIR/server_acap.py" "$PACKAGE_DIR/app/server.py"
cp "$APP_DIR/requirements.txt" "$PACKAGE_DIR/app/"

# Create shell wrapper script with corrected Python path
echo ""
echo "Creating shell wrapper (FIXED Python path)..."
cat > "$PACKAGE_DIR/omnisightv2" <<'EOF'
#!/bin/sh
# OMNISIGHT v0.6.1 Entry Point
# Starts Flask API server on port 8080

echo "Starting OMNISIGHT Flask API Server..."

# Find Python 3 - AXIS OS typically has it at /usr/bin/python or /usr/bin/python3
PYTHON=""
if [ -x "/usr/bin/python3" ]; then
    PYTHON="/usr/bin/python3"
elif [ -x "/usr/bin/python" ]; then
    # Check if it's Python 3
    if /usr/bin/python --version 2>&1 | grep -q "Python 3"; then
        PYTHON="/usr/bin/python"
    fi
fi

if [ -z "$PYTHON" ]; then
    echo "ERROR: Python 3 not found!"
    echo "Checked: /usr/bin/python3, /usr/bin/python"
    exit 1
fi

echo "Using Python: $PYTHON"
cd /usr/local/packages/omnisightv2
exec $PYTHON app/server.py
EOF
chmod +x "$PACKAGE_DIR/omnisightv2"

# Create manifest.json (Schema 1.8.0 with reverseProxy)
echo ""
echo "Creating manifest.json (Schema 1.8.0)..."
cat > "$PACKAGE_DIR/manifest.json" <<'MFEOF'
{
  "schemaVersion": "1.8.0",
  "acapPackageConf": {
    "setup": {
      "appName": "omnisightv2",
      "friendlyName": "OMNISIGHT v2",
      "vendor": "OMNISIGHT",
      "vendorUrl": "https://github.com/oneshot2001/OmniSight-dev",
      "version": "0.6.1",
      "embeddedSdkVersion": "3.0",
      "runMode": "respawn",
      "architecture": "aarch64"
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
}
MFEOF

# Create package.conf
echo "Creating package.conf..."
cat > "$PACKAGE_DIR/package.conf" <<'PKGEOF'
PACKAGENAME="OMNISIGHT v2"
APPTYPE="aarch64"
APPNAME="omnisightv2"
APPMAJORVERSION="0"
APPMINORVERSION="6"
APPMICROVERSION="1"
APPGRP="sdk"
APPUSR="sdk"
APPOPTS=""
OTHERFILES=""
SETTINGSPAGEFILE="index.html"
STARTMODE="respawn"
PKGEOF

# Create param.conf
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
tar czf "$OUTPUT_DIR/OMNISIGHT_v2_FLASK_FIXED_0_6_1.eap" .

# Get package size
PACKAGE_SIZE=$(du -h "$OUTPUT_DIR/OMNISIGHT_v2_FLASK_FIXED_0_6_1.eap" | cut -f1)

echo ""
echo "=========================================="
echo "✅ v0.6.1 Build Complete (FIXED)!"
echo "=========================================="
echo ""
echo "Package: OMNISIGHT_v2_FLASK_FIXED_0_6_1.eap"
echo "Size: $PACKAGE_SIZE"
echo "Location: $OUTPUT_DIR/"
echo ""
echo "FIX Applied:"
echo "  ✓ Python path detection logic"
echo "  ✓ Checks /usr/bin/python3 first"
echo "  ✓ Falls back to /usr/bin/python (if Python 3)"
echo "  ✓ Error message if Python not found"
echo ""
echo "This should start Flask successfully!"
echo "=========================================="
