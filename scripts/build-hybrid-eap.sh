#!/bin/bash
# OMNISIGHT Hybrid API ACAP Package Builder (v0.2.1)
# Creates lightweight .eap package using Python http.server (no dependencies)
# NO DOCKER REQUIRED - Builds directly on macOS/Linux

set -e  # Exit on error

# Configuration
VERSION="0.2.2"
APP_NAME="omnisight"
FRIENDLY_NAME="OMNISIGHT - Precognitive Security"
PACKAGE_NAME="OMNISIGHT_-_Precognitive_Security_${VERSION//./_}_aarch64.eap"

# Paths
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
PACKAGE_DIR="$PROJECT_ROOT/package-hybrid"

echo "============================================"
echo "OMNISIGHT Hybrid API ACAP Builder v${VERSION}"
echo "============================================"
echo "Project Root: $PROJECT_ROOT"
echo "Package Dir:  $PACKAGE_DIR"
echo "Package:      $PACKAGE_NAME"
echo "============================================"

# Clean previous builds
echo "Cleaning previous builds..."
rm -rf "$PACKAGE_DIR"
rm -f "$PROJECT_ROOT/$PACKAGE_NAME"

# Create package directory structure
echo "Creating package structure..."
mkdir -p "$PACKAGE_DIR"/{html,api/perception,api/timeline,api/swarm,api/config}

# Copy HTML dashboard
echo "Copying HTML dashboard..."
cp "$PROJECT_ROOT/package-phase2/html/index.html" "$PACKAGE_DIR/html/"

# Copy all JSON API files
echo "Copying JSON API files..."
cp "$PROJECT_ROOT/app/api/health.json" "$PACKAGE_DIR/api/"
cp "$PROJECT_ROOT/app/api/stats.json" "$PACKAGE_DIR/api/"
cp "$PROJECT_ROOT/app/api/perception/"*.json "$PACKAGE_DIR/api/perception/"
cp "$PROJECT_ROOT/app/api/timeline/"*.json "$PACKAGE_DIR/api/timeline/"
cp "$PROJECT_ROOT/app/api/swarm/"*.json "$PACKAGE_DIR/api/swarm/"
cp "$PROJECT_ROOT/app/api/config/"*.json "$PACKAGE_DIR/api/config/"

# Copy hybrid server Python script
echo "Copying hybrid server..."
cp "$PROJECT_ROOT/app/hybrid_server.py" "$PACKAGE_DIR/"

# Create launcher script
echo "Creating launcher script..."
cat > "$PACKAGE_DIR/omnisight" << 'LAUNCHER_EOF'
#!/bin/sh
# OMNISIGHT Hybrid API Server Launcher
# Uses built-in Python http.server - NO DEPENDENCIES REQUIRED

# Set environment variables
export OMNISIGHT_HTML_DIR="/usr/local/packages/omnisight/html"
export OMNISIGHT_API_DIR="/usr/local/packages/omnisight/api"
export PORT=8080

# Change to package directory
cd /usr/local/packages/omnisight

# Start hybrid server
exec python3 hybrid_server.py
LAUNCHER_EOF

chmod +x "$PACKAGE_DIR/omnisight"

# Copy manifest and package.conf
echo "Creating manifest and package.conf..."
cp "$SCRIPT_DIR/manifest-hybrid.json" "$PACKAGE_DIR/manifest.json"
cp "$SCRIPT_DIR/package-hybrid.conf" "$PACKAGE_DIR/package.conf"

# Copy LICENSE (create minimal if not exists)
echo "Copying LICENSE..."
if [ -f "$PROJECT_ROOT/LICENSE" ]; then
    cp "$PROJECT_ROOT/LICENSE" "$PACKAGE_DIR/"
else
    echo "MIT License - OMNISIGHT Project" > "$PACKAGE_DIR/LICENSE"
fi

# Create tarball
echo "Creating tarball..."
cd "$PACKAGE_DIR"
tar -czf "../$PACKAGE_NAME" *

# Get package size
PACKAGE_SIZE=$(stat -f%z "../$PACKAGE_NAME" 2>/dev/null || stat -c%s "../$PACKAGE_NAME")
PACKAGE_SIZE_KB=$((PACKAGE_SIZE / 1024))

echo ""
echo "============================================"
echo "✓ Package built successfully!"
echo "============================================"
echo "Package: $PACKAGE_NAME"
echo "Size:    ${PACKAGE_SIZE_KB} KB"
echo "Location: $PROJECT_ROOT/$PACKAGE_NAME"
echo "============================================"
echo ""
echo "Package contents:"
ls -lh "$PACKAGE_DIR" | tail -n +2

echo ""
echo "JSON API files:"
find "$PACKAGE_DIR/api" -name "*.json" -exec basename {} \; | sort

echo ""
echo "Ready for deployment to AXIS camera!"
echo "============================================"
