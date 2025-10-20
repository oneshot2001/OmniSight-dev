# OMNISIGHT ACAP Installation Guide

Complete guide for installing OMNISIGHT as an ACAP application on Axis ARTPEC-8/9 cameras.

## Table of Contents

1. [Prerequisites](#prerequisites)
2. [Supported Cameras](#supported-cameras)
3. [Quick Start](#quick-start)
4. [Detailed Installation](#detailed-installation)
5. [Build from Source](#build-from-source)
6. [Deployment Methods](#deployment-methods)
7. [Configuration](#configuration)
8. [Verification](#verification)
9. [Troubleshooting](#troubleshooting)
10. [Uninstallation](#uninstallation)

## Prerequisites

### Development Machine Requirements

- **OS**: Linux (Ubuntu 20.04+ recommended) or macOS with Docker
- **Docker**: 20.10 or later
- **Storage**: 10GB free space for SDK and build artifacts
- **Network**: Internet connection for downloading SDK

### Camera Requirements

**Hardware:**
- Axis camera with ARTPEC-8 or ARTPEC-9 SoC
- Minimum 512MB RAM (1GB recommended)
- Minimum 100MB available storage
- Network connectivity (Ethernet)

**Software:**
- AXIS OS 11.0+ (ARTPEC-8) or 12.0+ (ARTPEC-9)
- ACAP Runtime 4.0+
- Camera firmware up to date

### Access Requirements

- Admin credentials for camera web interface
- SSH access (optional, for debugging)
- Network access to camera (HTTP/HTTPS)

## Supported Cameras

### ARTPEC-8 Cameras
- AXIS Q1656-LE Box Camera
- AXIS M3068-P Mini Dome
- AXIS P3265-LVE Dome Camera
- AXIS P3267-LVE Dome Camera with IR
- AXIS Q3536-LVE Dome Camera
- AXIS Q3538-LVE Dome Camera

### ARTPEC-9 Cameras
- AXIS Q1700-LE License Plate Camera
- AXIS P3267-LVE-3 Dome Camera
- AXIS Q3819-PVE Panoramic Camera
- Future ARTPEC-9 models

## Quick Start

### Using Pre-built Package

1. **Download the latest release:**
```bash
wget https://github.com/oneshot2001/OmniSight-dev/releases/latest/download/omnisight_1_0_0_aarch64.eap
```

2. **Open camera web interface:**
```
http://<camera-ip>
```

3. **Navigate to Apps:**
```
Settings → Apps → Add app
```

4. **Upload and start:**
- Click "Choose file" and select `omnisight_1_0_0_aarch64.eap`
- Click "Install"
- Click "Start" to run OMNISIGHT

## Detailed Installation

### Step 1: Download ACAP Native SDK

```bash
# Pull the SDK Docker image for ARTPEC-8/9 (aarch64)
docker pull axisecp/acap-native-sdk:1.15-aarch64-ubuntu22.04

# Verify the image
docker images | grep acap-native-sdk
```

### Step 2: Clone OMNISIGHT Repository

```bash
git clone https://github.com/oneshot2001/OmniSight-dev.git
cd OmniSight-dev
```

### Step 3: Build the ACAP Package

```bash
# Run build in Docker container
docker run --rm -v $(pwd):/opt/app \
    axisecp/acap-native-sdk:1.15-aarch64-ubuntu22.04 \
    make -C /opt/app package

# This creates: omnisight_1_0_0_aarch64.eap
```

### Step 4: Install on Camera

See [Deployment Methods](#deployment-methods) below for installation options.

## Build from Source

### Complete Build Process

1. **Enter build environment:**
```bash
docker run --rm -it \
    -v $(pwd):/opt/app \
    -w /opt/app \
    axisecp/acap-native-sdk:1.15-aarch64-ubuntu22.04 \
    /bin/bash
```

2. **Set up cross-compilation (inside container):**
```bash
# Source the SDK environment
source /opt/axis/acapsdk/environment-setup-aarch64-poky-linux

# Verify compiler
echo $CC
# Output: aarch64-poky-linux-gcc
```

3. **Build components:**
```bash
# Create build directory
mkdir -p build && cd build

# Configure with CMake
cmake .. \
    -DCMAKE_TOOLCHAIN_FILE=$OECORE_TARGET_SYSROOT/usr/share/cmake/OEToolchainConfig.cmake \
    -DCMAKE_BUILD_TYPE=Release \
    -DARTPEC_VERSION=8  # or 9 for ARTPEC-9

# Build
make -j$(nproc)

# Strip debug symbols (reduce size)
$STRIP omnisight
```

4. **Prepare ML models:**
```bash
# Convert TensorFlow model to TFLite
python3 models/training/scripts/convert_to_tflite.py \
    --model models/checkpoints/detection.h5 \
    --output models/detection/detection.tflite \
    --quantize INT8 \
    --artpec 8
```

5. **Create package structure:**
```bash
mkdir -p package/omnisight
cp build/omnisight package/omnisight/
cp models/detection/*.tflite package/omnisight/models/
cp -r web/frontend/dist package/omnisight/html/
cp manifest.json package/omnisight/
```

6. **Package the ACAP:**
```bash
cd package
tar czf ../omnisight_1_0_0_aarch64.eap omnisight/
cd ..
```

### Architecture-Specific Builds

**For ARTPEC-8:**
```bash
export CFLAGS="-march=armv8-a -mtune=cortex-a53 -O3"
export ARTPEC_VERSION=8
```

**For ARTPEC-9:**
```bash
export CFLAGS="-march=armv8.2-a -mtune=cortex-a77 -O3"
export ARTPEC_VERSION=9
```

## Deployment Methods

### Method 1: Web Interface (Recommended)

1. **Access camera web interface:**
   - Open browser: `http://<camera-ip>`
   - Login with admin credentials

2. **Navigate to Apps:**
   - Go to: **Settings** → **Apps**
   - Or direct URL: `http://<camera-ip>/#settings/apps`

3. **Install OMNISIGHT:**
   - Click **"Add app"** or **"+"** button
   - Click **"Choose file"**
   - Select `omnisight_1_0_0_aarch64.eap`
   - Click **"Install"**
   - Wait for upload (progress bar)

4. **Start the application:**
   - Find OMNISIGHT in apps list
   - Click **"Start"**
   - Verify status shows "Running"

### Method 2: VAPIX API (Automated)

```bash
# Set camera credentials
CAMERA_IP="192.168.1.100"
USERNAME="root"
PASSWORD="your-password"

# Upload package
curl -k --anyauth -u $USERNAME:$PASSWORD \
    -F "packfil=@omnisight_1_0_0_aarch64.eap" \
    "http://$CAMERA_IP/axis-cgi/applications/upload.cgi"

# Install
curl -k --anyauth -u $USERNAME:$PASSWORD \
    "http://$CAMERA_IP/axis-cgi/applications/control.cgi?action=install&package=omnisight"

# Start
curl -k --anyauth -u $USERNAME:$PASSWORD \
    "http://$CAMERA_IP/axis-cgi/applications/control.cgi?action=start&package=omnisight"

# Verify
curl -k --anyauth -u $USERNAME:$PASSWORD \
    "http://$CAMERA_IP/axis-cgi/applications/list.cgi" | grep omnisight
```

### Method 3: SSH Installation (Development)

```bash
# Copy package to camera
scp omnisight_1_0_0_aarch64.eap root@$CAMERA_IP:/tmp/

# SSH into camera
ssh root@$CAMERA_IP

# Extract package
cd /usr/local/packages
mkdir -p omnisight
cd omnisight
tar xzf /tmp/omnisight_1_0_0_aarch64.eap

# Create systemd service
cat > /etc/systemd/system/acap-omnisight.service << EOF
[Unit]
Description=OMNISIGHT Precognitive Security
After=network.target

[Service]
Type=simple
ExecStart=/usr/local/packages/omnisight/omnisight
Restart=always
User=sdk

[Install]
WantedBy=multi-user.target
EOF

# Start service
systemctl daemon-reload
systemctl enable acap-omnisight
systemctl start acap-omnisight
```

### Method 4: Batch Deployment (Multiple Cameras)

Create `deploy.sh`:
```bash
#!/bin/bash

# Camera list
CAMERAS=(
    "192.168.1.100"
    "192.168.1.101"
    "192.168.1.102"
)

USERNAME="root"
PASSWORD="your-password"
PACKAGE="omnisight_1_0_0_aarch64.eap"

for CAMERA_IP in "${CAMERAS[@]}"; do
    echo "Deploying to $CAMERA_IP..."

    # Upload
    curl -k --anyauth -u $USERNAME:$PASSWORD \
        -F "packfil=@$PACKAGE" \
        "http://$CAMERA_IP/axis-cgi/applications/upload.cgi"

    # Install and start
    curl -k --anyauth -u $USERNAME:$PASSWORD \
        "http://$CAMERA_IP/axis-cgi/applications/control.cgi?action=install&package=omnisight"

    curl -k --anyauth -u $USERNAME:$PASSWORD \
        "http://$CAMERA_IP/axis-cgi/applications/control.cgi?action=start&package=omnisight"

    echo "✓ Deployed to $CAMERA_IP"
done
```

## Configuration

### Initial Configuration

1. **Access OMNISIGHT web interface:**
```
http://<camera-ip>:8080
```

2. **Configure via API:**
```bash
curl -X POST http://$CAMERA_IP:8080/api/config \
    -H "Content-Type: application/json" \
    -d '{
        "camera_id": 1,
        "perception": {
            "fps": 30,
            "confidence_threshold": 0.5
        },
        "timeline": {
            "prediction_horizon_s": 60.0,
            "num_timelines": 3
        },
        "swarm": {
            "mqtt_broker": "192.168.1.50",
            "privacy_level": "features"
        }
    }'
```

3. **Configure via camera parameters:**
```bash
# Set parameters
curl -k --anyauth -u $USERNAME:$PASSWORD \
    "http://$CAMERA_IP/axis-cgi/param.cgi?action=update&OMNISIGHT.Perception.FPS=30"

# Get parameters
curl -k --anyauth -u $USERNAME:$PASSWORD \
    "http://$CAMERA_IP/axis-cgi/param.cgi?action=list&group=OMNISIGHT"
```

### Configuration Files on Camera

**Main config:** `/usr/local/packages/omnisight/config.json`
```json
{
    "camera_id": 1,
    "role": "internal",
    "perception": {
        "fps": 30,
        "resolution": {
            "width": 1920,
            "height": 1080
        }
    }
}
```

**Edit via SSH:**
```bash
ssh root@$CAMERA_IP
vi /usr/local/packages/omnisight/config.json
systemctl restart acap-omnisight
```

## Verification

### 1. Check Installation Status

**Via Web Interface:**
- Go to Settings → Apps
- OMNISIGHT should show "Running"

**Via API:**
```bash
curl -k --anyauth -u $USERNAME:$PASSWORD \
    "http://$CAMERA_IP/axis-cgi/applications/list.cgi" | jq '.data[] | select(.Name=="omnisight")'
```

**Via SSH:**
```bash
ssh root@$CAMERA_IP
systemctl status acap-omnisight
ps aux | grep omnisight
```

### 2. Check Logs

```bash
# View logs via SSH
ssh root@$CAMERA_IP
journalctl -u acap-omnisight -f

# View logs via API
curl -k --anyauth -u $USERNAME:$PASSWORD \
    "http://$CAMERA_IP/axis-cgi/systemlog.cgi?appname=omnisight"
```

### 3. Check Resource Usage

```bash
# CPU and memory
ssh root@$CAMERA_IP
top -b -n 1 | grep omnisight

# Storage
df -h /usr/local/packages/omnisight
```

### 4. Test Functionality

```bash
# Test API endpoints
curl http://$CAMERA_IP:8080/api/status
curl http://$CAMERA_IP:8080/api/statistics
curl http://$CAMERA_IP:8080/api/tracks

# Test WebSocket
wscat -c ws://$CAMERA_IP:8081/ws/events
```

## Troubleshooting

### Common Issues and Solutions

**1. Installation fails with "Insufficient resources"**
```bash
# Solution: Free up space
ssh root@$CAMERA_IP
rm -rf /tmp/*
apt-get clean
```

**2. Application won't start**
```bash
# Check logs for errors
journalctl -u acap-omnisight -n 50

# Common causes:
# - Missing ML models → Verify models/ directory
# - DLPU not available → Check with: larod-client -l
# - Port conflict → Change ports in config.json
```

**3. Low FPS or high latency**
```bash
# Reduce resolution or FPS
curl -X POST http://$CAMERA_IP:8080/api/config \
    -d '{"perception": {"fps": 15, "resolution": {"width": 1280, "height": 720}}}'
```

**4. DLPU not detected**
```bash
# Check DLPU availability
ssh root@$CAMERA_IP
larod-client -l

# Output should show:
# Chip: ARTPEC-8 DLPU
# Status: Available
```

**5. Memory issues**
```bash
# Check memory usage
free -m

# Reduce memory usage
# - Lower max_tracks in config
# - Disable timeline branching
# - Reduce swarm buffer sizes
```

### Debug Mode

Enable debug logging:
```bash
ssh root@$CAMERA_IP
echo "debug" > /usr/local/packages/omnisight/debug
systemctl restart acap-omnisight
tail -f /var/log/messages | grep omnisight
```

## Uninstallation

### Via Web Interface

1. Go to Settings → Apps
2. Find OMNISIGHT
3. Click "Stop"
4. Click "Remove" or trash icon

### Via API

```bash
# Stop
curl -k --anyauth -u $USERNAME:$PASSWORD \
    "http://$CAMERA_IP/axis-cgi/applications/control.cgi?action=stop&package=omnisight"

# Remove
curl -k --anyauth -u $USERNAME:$PASSWORD \
    "http://$CAMERA_IP/axis-cgi/applications/control.cgi?action=remove&package=omnisight"
```

### Via SSH

```bash
ssh root@$CAMERA_IP

# Stop service
systemctl stop acap-omnisight
systemctl disable acap-omnisight

# Remove files
rm -rf /usr/local/packages/omnisight
rm -f /etc/systemd/system/acap-omnisight.service
systemctl daemon-reload
```

## Support

### Logs Location
- Application logs: `/var/log/messages`
- ACAP logs: `journalctl -u acap-omnisight`
- Web interface logs: `/var/log/lighttpd/access.log`

### Debug Information
When reporting issues, include:
1. Camera model and firmware version
2. OMNISIGHT version
3. Error logs from journalctl
4. Output of `larod-client -l`
5. Memory and CPU usage

### Resources
- [AXIS ACAP Documentation](https://www.axis.com/developer-community/acap)
- [OMNISIGHT GitHub](https://github.com/oneshot2001/OmniSight-dev)
- [AXIS Developer Community](https://www.axis.com/developer-community)

## License

Copyright © 2024 OMNISIGHT Project. All rights reserved.