#!/bin/bash
# OMNISIGHT Phase 3 ACAP Package Builder
# Creates .eap package with HARDWARE INTEGRATION (VDO, Larod, MQTT)
# REQUIRES: Docker with ACAP SDK, actual Axis camera for deployment

set -e  # Exit on error

echo "================================================"
echo "OMNISIGHT Phase 3 ACAP Package Builder"
echo "Hardware Integration (VDO + Larod + MQTT)"
echo "================================================"
echo ""

# Get the directory where this script is located
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"

# Directories
BUILD_DIR="$PROJECT_ROOT/build-phase3"
PACKAGE_DIR="$PROJECT_ROOT/package-phase3"
OUTPUT_DIR="$PROJECT_ROOT"
MODELS_DIR="$PROJECT_ROOT/models"

# Colors
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
NC='\033[0m' # No Color

echo "Project root: $PROJECT_ROOT"
echo ""

# Check if running inside Docker
if [ ! -f "/.dockerenv" ]; then
    echo "${RED}ERROR: Phase 3 build must run inside ACAP SDK Docker container${NC}"
    echo ""
    echo "Run this command:"
    echo "  docker-compose up -d omnisight-dev"
    echo "  docker-compose exec omnisight-dev bash"
    echo "  cd /opt/app"
    echo "  ./scripts/build-phase3-eap.sh"
    echo ""
    exit 1
fi

# Step 1: Clean and create directories
echo "Step 1: Creating build and package structure..."
rm -rf "$BUILD_DIR" "$PACKAGE_DIR"
mkdir -p "$BUILD_DIR"
mkdir -p "$PACKAGE_DIR"
mkdir -p "$PACKAGE_DIR/html"
mkdir -p "$PACKAGE_DIR/app"
mkdir -p "$PACKAGE_DIR/app/api"
mkdir -p "$PACKAGE_DIR/models"
mkdir -p "$PACKAGE_DIR/lib"

echo "  ✓ Directories created"

# Step 2: Configure CMake with hardware APIs enabled
echo ""
echo "Step 2: Configuring CMake build with hardware APIs..."
cd "$BUILD_DIR"

# Source ACAP SDK environment
. /opt/axis/acapsdk/environment-setup-cortexa53-crypto-poky-linux

# Configure CMake for hardware build
cmake "$PROJECT_ROOT" \
    -DCMAKE_BUILD_TYPE=Release \
    -DENABLE_HARDWARE_APIS=ON \
    -DENABLE_VDO=ON \
    -DENABLE_LAROD=ON \
    -DENABLE_MQTT=ON \
    -DCMAKE_TOOLCHAIN_FILE=/opt/axis/acapsdk/sysroots/x86_64-pokysdk-linux/usr/share/cmake/OEToolchainConfig.cmake \
    -DCMAKE_INSTALL_PREFIX=/usr/local/packages/omnisight

echo "  ✓ CMake configured for hardware build"

# Step 3: Build native binaries
echo ""
echo "Step 3: Building OMNISIGHT native binaries..."
make -j$(nproc) omnisight

if [ ! -f "omnisight" ]; then
    echo "${RED}ERROR: omnisight binary not built${NC}"
    exit 1
fi

echo "  ✓ Native binary built successfully"

# Check binary architecture
file omnisight | grep -q "ARM aarch64" || {
    echo "${RED}ERROR: Binary is not ARM aarch64${NC}"
    file omnisight
    exit 1
}

echo "  ✓ Binary architecture verified: ARM aarch64"

# Step 4: Copy binaries and libraries
echo ""
echo "Step 4: Packaging binaries and libraries..."

# Copy main executable
cp omnisight "$PACKAGE_DIR/"
chmod +x "$PACKAGE_DIR/omnisight"

# Strip debug symbols to reduce size
$STRIP "$PACKAGE_DIR/omnisight" || echo "${YELLOW}Warning: Strip failed (non-critical)${NC}"

echo "  ✓ Binary size: $(du -h "$PACKAGE_DIR/omnisight" | cut -f1)"

# Copy required shared libraries (if any)
if ldd omnisight 2>/dev/null | grep -q "=>"; then
    echo "  Copying required libraries..."
    ldd omnisight | grep "=> /" | awk '{print $3}' | grep -v "^/lib" | while read lib; do
        if [ -f "$lib" ]; then
            cp "$lib" "$PACKAGE_DIR/lib/"
            echo "    - $(basename "$lib")"
        fi
    done
fi

# Step 5: Copy Flask application
echo ""
echo "Step 5: Packaging Flask API server..."

cp "$PROJECT_ROOT/app/server.py" "$PACKAGE_DIR/app/"
cp "$PROJECT_ROOT/app/requirements.txt" "$PACKAGE_DIR/app/"
cp -r "$PROJECT_ROOT/app/api" "$PACKAGE_DIR/app/"
cp -r "$PROJECT_ROOT/app/ipc" "$PACKAGE_DIR/app/" 2>/dev/null || true

echo "  ✓ Copied Flask API server"
echo "  ✓ Copied IPC layer"

# Step 6: Package ML model
echo ""
echo "Step 6: Packaging ML model..."

# Check for TFLite model
if [ -f "$MODELS_DIR/omnisight_detection.tflite" ]; then
    cp "$MODELS_DIR/omnisight_detection.tflite" "$PACKAGE_DIR/models/"
    echo "  ✓ Copied TFLite model: $(du -h "$MODELS_DIR/omnisight_detection.tflite" | cut -f1)"
else
    # Create placeholder for model
    cat > "$PACKAGE_DIR/models/README.txt" << 'MODEL_README'
OMNISIGHT ML Model Placeholder

The TensorFlow Lite model file should be uploaded separately to:
/usr/local/packages/omnisight/models/omnisight_detection.tflite

Model Requirements:
- Format: TensorFlow Lite (.tflite)
- Input: 416x416 RGB image (normalized [0.0, 1.0])
- Output: YOLO-style detection [boxes, scores, classes]
- Size: <100MB recommended for DLPU performance
- Quantization: INT8 or FP16 for best ARTPEC-8/9 performance

To upload model to camera:
1. SSH to camera: ssh root@<camera-ip>
2. Create directory: mkdir -p /usr/local/packages/omnisight/models
3. Upload model: scp omnisight_detection.tflite root@<camera-ip>:/usr/local/packages/omnisight/models/
4. Verify: ls -lh /usr/local/packages/omnisight/models/

Without the model, OMNISIGHT will fall back to stub detection data.
MODEL_README

    echo "  ${YELLOW}⚠ No model found - created placeholder${NC}"
    echo "  ${YELLOW}  Upload model separately to camera after deployment${NC}"
fi

# Step 7: Create startup script
echo ""
echo "Step 7: Creating startup script..."

cat > "$PACKAGE_DIR/start.sh" << 'STARTUP_EOF'
#!/bin/sh
# OMNISIGHT Phase 3 - Hardware Integration Startup Script

APP_DIR="/usr/local/packages/omnisight"
LOG_PREFIX="OMNISIGHT:"

echo "$LOG_PREFIX Starting Phase 3 (Hardware Integration)..."

# Set environment variables
export OMNISIGHT_HTML_DIR="$APP_DIR/html"
export OMNISIGHT_MODELS_DIR="$APP_DIR/models"
export OMNISIGHT_IPC_DIR="/tmp"
export PYTHONPATH="$APP_DIR:$PYTHONPATH"
export LD_LIBRARY_PATH="$APP_DIR/lib:$LD_LIBRARY_PATH"

# Check for ML model
if [ ! -f "$OMNISIGHT_MODELS_DIR/omnisight_detection.tflite" ]; then
    echo "$LOG_PREFIX WARNING: ML model not found at $OMNISIGHT_MODELS_DIR/omnisight_detection.tflite"
    echo "$LOG_PREFIX Will use fallback stub detection"
fi

# Install Python dependencies on first run
DEPS_MARKER="/tmp/omnisight-phase3-deps-installed"
if [ ! -f "$DEPS_MARKER" ]; then
    echo "$LOG_PREFIX Installing Flask dependencies..."
    pip3 install --no-cache-dir Flask==3.0.0 Flask-CORS==4.0.0 Werkzeug==3.0.1 || {
        echo "$LOG_PREFIX ERROR: Failed to install dependencies"
        exit 1
    }
    touch "$DEPS_MARKER"
    echo "$LOG_PREFIX Dependencies installed"
fi

# Start C core in background (writes JSON IPC files)
echo "$LOG_PREFIX Starting OMNISIGHT core with hardware APIs..."
$APP_DIR/omnisight --hardware &
CORE_PID=$!

# Give core time to initialize
sleep 2

# Check if core started successfully
if ! kill -0 $CORE_PID 2>/dev/null; then
    echo "$LOG_PREFIX ERROR: Core failed to start"
    exit 1
fi

echo "$LOG_PREFIX Core started (PID: $CORE_PID)"

# Start Flask API server (reads JSON IPC files)
echo "$LOG_PREFIX Starting Flask API server..."
cd "$APP_DIR/app"
exec python3 server.py
STARTUP_EOF

chmod +x "$PACKAGE_DIR/start.sh"
echo "  ✓ Created startup script"

# Step 8: Create manifest
echo ""
echo "Step 8: Creating manifest.json..."

cp "$SCRIPT_DIR/manifest-phase3.json" "$PACKAGE_DIR/manifest.json"
echo "  ✓ Copied Phase 3 manifest (schema 1.8.0)"

# Step 9: Create LICENSE
echo ""
echo "Step 9: Creating LICENSE..."

cat > "$PACKAGE_DIR/LICENSE" << 'LICENSE_EOF'
MIT License

Copyright (c) 2024-2025 OMNISIGHT Project

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
LICENSE_EOF

echo "  ✓ Created MIT LICENSE"

# Step 10: Create HTML interface
echo ""
echo "Step 10: Creating web interface..."

cat > "$PACKAGE_DIR/html/index.html" << 'HTML_EOF'
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>OMNISIGHT - Hardware Mode</title>
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
        .status.hardware {
            background: #e8f5e9;
            border-left-color: #4caf50;
        }
        .status h3 {
            color: #667eea;
            margin-bottom: 15px;
        }
        .status.hardware h3 {
            color: #4caf50;
        }
        .metric {
            display: inline-block;
            margin: 10px 30px 10px 0;
        }
        .metric-label {
            font-size: 0.85em;
            color: #666;
            display: block;
            margin-bottom: 5px;
        }
        .metric-value {
            font-size: 1.8em;
            font-weight: bold;
            color: #667eea;
        }
        .metric-value.hardware {
            color: #4caf50;
        }
        .warning {
            background: #fff3cd;
            border-left: 4px solid #ffc107;
            padding: 15px;
            margin: 20px 0;
            border-radius: 8px;
        }
        .warning h3 {
            color: #f39c12;
            margin-bottom: 10px;
        }
        .feature {
            margin: 12px 0;
            padding-left: 30px;
            position: relative;
            line-height: 1.6;
        }
        .feature:before {
            content: "✓";
            position: absolute;
            left: 0;
            color: #4caf50;
            font-weight: bold;
            font-size: 1.3em;
        }
        .version {
            text-align: center;
            color: #999;
            margin-top: 30px;
            padding-top: 20px;
            border-top: 1px solid #eee;
            font-size: 0.9em;
        }
        .badge {
            display: inline-block;
            background: #4caf50;
            color: white;
            padding: 4px 12px;
            border-radius: 12px;
            font-size: 0.8em;
            font-weight: bold;
            margin-left: 10px;
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>🔮 OMNISIGHT</h1>
        <div class="tagline">Precognitive Security System <span class="badge">HARDWARE MODE</span></div>

        <div class="status hardware">
            <h3>🚀 Phase 3: Hardware Integration Active</h3>
            <div class="metric">
                <span class="metric-label">Status</span>
                <span class="metric-value hardware">Live</span>
            </div>
            <div class="metric">
                <span class="metric-label">Mode</span>
                <span class="metric-value hardware">Hardware</span>
            </div>
            <div class="metric">
                <span class="metric-label">Version</span>
                <span class="metric-value hardware">0.4.0</span>
            </div>
        </div>

        <div class="warning" id="model-warning" style="display:none;">
            <h3>⚠️ Model Not Found</h3>
            <p>TensorFlow Lite model not detected. Using fallback stub detection.</p>
            <p><strong>Upload model to:</strong> /usr/local/packages/omnisight/models/omnisight_detection.tflite</p>
        </div>

        <div class="features">
            <h3>✨ Hardware Features Enabled</h3>
            <div class="feature"><strong>VDO API</strong> - Real-time video capture from ARTPEC-8/9</div>
            <div class="feature"><strong>Larod API</strong> - DLPU-accelerated ML inference (<20ms)</div>
            <div class="feature"><strong>MQTT Swarm</strong> - Multi-camera coordination and track sharing</div>
            <div class="feature"><strong>Object Tracking</strong> - Kalman filter-based multi-object tracking</div>
            <div class="feature"><strong>Timeline Threading™</strong> - Real-time prediction of 3-5 probable futures</div>
            <div class="feature"><strong>Federated Learning</strong> - Privacy-preserving model updates</div>
        </div>

        <div class="features">
            <h3>📊 Performance Targets</h3>
            <div class="feature">Inference Speed: <20ms per frame</div>
            <div class="feature">Prediction Horizon: 5 minutes</div>
            <div class="feature">False Positive Rate: <3%</div>
            <div class="feature">Memory Footprint: <512MB</div>
            <div class="feature">CPU Usage: <30%</div>
            <div class="feature">Network Bandwidth: <500Kbps</div>
        </div>

        <div class="version">
            <strong>OMNISIGHT v0.4.0</strong> | Phase 3 Hardware Build<br>
            <small>VDO + Larod + MQTT integration</small><br>
            <small>ARTPEC-8/9 Native | AXIS OS 12.0+</small><br>
            <small>© 2024-2025 OMNISIGHT Project | MIT License</small>
        </div>
    </div>

    <script>
        // Check if model exists via API
        fetch('/api/health')
            .then(r => r.json())
            .then(data => {
                if (data.model_loaded === false) {
                    document.getElementById('model-warning').style.display = 'block';
                }
            })
            .catch(() => {
                document.getElementById('model-warning').style.display = 'block';
            });
    </script>
</body>
</html>
HTML_EOF

echo "  ✓ Created HTML dashboard"

# Step 11: Create package
echo ""
echo "Step 11: Creating .eap package..."

# Read version from manifest
VERSION=$(grep '"version"' "$PACKAGE_DIR/manifest.json" | sed 's/.*: *"\([^"]*\)".*/\1/')
PACKAGE_NAME="OMNISIGHT_-_Hardware_${VERSION}_aarch64.eap"

# Use acap-build
cd "$PACKAGE_DIR"

# Source ACAP SDK environment again
. /opt/axis/acapsdk/environment-setup-cortexa53-crypto-poky-linux

acap-build \
    --build no-build \
    --disable-manifest-validation \
    .

# Move package to output
if [ -f *.eap ]; then
    EAP_FILE=$(ls *.eap | head -1)
    mv "$EAP_FILE" "$OUTPUT_DIR/$PACKAGE_NAME"

    PACKAGE_SIZE=$(du -h "$OUTPUT_DIR/$PACKAGE_NAME" | cut -f1)

    echo ""
    echo "================================================"
    echo "${GREEN}✓ Phase 3 Package Created Successfully!${NC}"
    echo "================================================"
    echo ""
    echo "📦 Package: $PACKAGE_NAME"
    echo "📊 Size: $PACKAGE_SIZE"
    echo "📁 Location: $OUTPUT_DIR/$PACKAGE_NAME"
    echo ""
    echo "📋 What's Included:"
    echo "  ✓ Native C binary with VDO/Larod/MQTT integration"
    echo "  ✓ Flask API server with JSON IPC"
    echo "  ✓ HTML dashboard"
    echo "  ✓ ML model placeholder (upload separately)"
    echo "  ✓ Startup script with hardware initialization"
    echo ""
    echo "🚀 Deployment:"
    echo "  See PHASE3_DEPLOYMENT_GUIDE.md for complete instructions"
    echo ""
    echo "⚠️  IMPORTANT: Upload ML model separately after deployment"
    echo "  scp model.tflite root@<camera-ip>:/usr/local/packages/omnisight/models/"
    echo ""
    echo "================================================"
else
    echo "${RED}ERROR: Package creation failed${NC}"
    exit 1
fi
