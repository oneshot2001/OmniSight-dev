#!/bin/bash
# Build OMNISIGHT v0.6.0 - Phase 3: Flask API Backend

set -e

PROJECT_ROOT="/Users/matthewvisher/Omnisight dev/OmniSight-dev"
PACKAGE_DIR="$PROJECT_ROOT/package-v060"
OUTPUT_DIR="$PROJECT_ROOT/packages/v060"
FRONTEND_DIR="$PROJECT_ROOT/web/frontend"
APP_DIR="$PROJECT_ROOT/app"

echo "=========================================="
echo "OMNISIGHT v0.6.0 - Phase 3 Build"
echo "Flask API + C Core Integration"
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
if [ ! -d "$FRONTEND_DIR/dist" ]; then
    echo "Building React frontend..."
    cd "$FRONTEND_DIR"
    npm run build
fi
cp -r "$FRONTEND_DIR/dist/"* "$PACKAGE_DIR/html/"

# Copy Flask app (simplified version)
echo ""
echo "Copying Flask API server..."
cp "$APP_DIR/server_acap.py" "$PACKAGE_DIR/app/server.py"
cp "$APP_DIR/requirements.txt" "$PACKAGE_DIR/app/"

# Create shell wrapper script
echo ""
echo "Creating shell wrapper..."
cat > "$PACKAGE_DIR/omnisightv2" <<'EOF'
#!/bin/sh
# OMNISIGHT v0.6.0 Entry Point
# Starts Flask API server on port 8080

echo "Starting OMNISIGHT Flask API Server..."
cd /usr/local/packages/omnisightv2
exec python3 app/server.py
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
      "version": "0.6.0",
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
APPMICROVERSION="0"
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
tar czf "$OUTPUT_DIR/OMNISIGHT_v2_FLASK_API_0_6_0.eap" .

# Get package size
PACKAGE_SIZE=$(du -h "$OUTPUT_DIR/OMNISIGHT_v2_FLASK_API_0_6_0.eap" | cut -f1)

echo ""
echo "=========================================="
echo "✅ v0.6.0 Build Complete!"
echo "=========================================="
echo ""
echo "Package: OMNISIGHT_v2_FLASK_API_0_6_0.eap"
echo "Size: $PACKAGE_SIZE"
echo "Location: $OUTPUT_DIR/"
echo ""
echo "Phase 3 Features:"
echo "  ✓ Schema 1.8.0 with reverseProxy"
echo "  ✓ Flask API server (port 8080)"
echo "  ✓ Shell wrapper for Python execution"
echo "  ✓ runMode: respawn (Flask stays running)"
echo "  ✓ JSON-based IPC with C stubs"
echo "  ✓ Full React dashboard"
echo ""
echo "Next Steps:"
echo "  1. Deploy to P3285-LVE"
echo "  2. Verify Open button appears"
echo "  3. Check Flask is running"
echo "  4. Test API endpoints"
echo ""
echo "Ready for deployment!"
echo "=========================================="
