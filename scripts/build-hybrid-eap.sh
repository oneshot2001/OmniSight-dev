#!/bin/bash
# OMNISIGHT Hybrid API ACAP Package Builder (v0.2.4)
# Creates lightweight .eap package using Python http.server (no dependencies)
# NO DOCKER REQUIRED - Builds directly on macOS/Linux
# v0.2.4: Simplified launcher - direct Python execution without command -v

set -e  # Exit on error

# Configuration
VERSION="0.2.4"
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
echo "Creating simplified launcher script..."
cat > "$PACKAGE_DIR/omnisight" << 'LAUNCHER_EOF'
#!/bin/sh
# OMNISIGHT Hybrid API Server Launcher v0.2.4
# Simplified: Tries Python paths directly without command -v
# Compatible with minimal shell environments (ARTPEC-8 and ARTPEC-9)

# Set environment variables
export OMNISIGHT_HTML_DIR="/usr/local/packages/omnisight/html"
export OMNISIGHT_API_DIR="/usr/local/packages/omnisight/api"
export PORT=8080

# Change to package directory
cd /usr/local/packages/omnisight || exit 1

# Try Python paths in order (first success wins)
# Using || to continue to next if current fails

echo "OMNISIGHT starting..." >&2

# Try /usr/bin/python3 first (ARTPEC-8)
if /usr/bin/python3 --version >/dev/null 2>&1; then
    echo "Using /usr/bin/python3" >&2
    exec /usr/bin/python3 hybrid_server.py
fi

# Try /usr/bin/python (ARTPEC-9)
if /usr/bin/python --version >/dev/null 2>&1; then
    echo "Using /usr/bin/python" >&2
    exec /usr/bin/python hybrid_server.py
fi

# Try python3 from PATH
if which python3 >/dev/null 2>&1; then
    echo "Using python3 from PATH" >&2
    exec python3 hybrid_server.py
fi

# Try python from PATH
if which python >/dev/null 2>&1; then
    echo "Using python from PATH" >&2
    exec python hybrid_server.py
fi

# If we get here, no Python was found
echo "ERROR: No Python interpreter found!" >&2
echo "Tried: /usr/bin/python3, /usr/bin/python, python3, python" >&2
exit 127
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
