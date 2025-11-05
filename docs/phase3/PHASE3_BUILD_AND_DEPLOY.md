# Phase 3 Build and Deployment Instructions

**Status**: Ready for Hardware Deployment
**Target**: Axis P3285-LVE (ARTPEC-9 DLPU)
**Version**: 0.4.0

## Quick Summary

Phase 3 is **100% code complete** and ready for hardware testing:

✅ **VDO Capture**: vdo_capture.c/h (complete)
✅ **Larod Inference**: larod_inference.c/h (complete)
✅ **Object Tracking**: object_tracking.c/h, tracker.c/h (complete)
✅ **Behavior Analysis**: behavior_analysis.c/h, behavior.c/h (complete)
✅ **Integration Layer**: perception.c with full pipeline (complete)
✅ **Build System**: CMake + build script (complete)
✅ **ACAP Packaging**: manifest + deployment scripts (complete)

**Total Phase 3 Code**: ~150KB across 8 modules

---

## Prerequisites

### Hardware
- Axis P3285-LVE camera (ARTPEC-9) or compatible ARTPEC-8 camera
- Firmware: AXIS OS 12.0 or later
- Network access to camera
- Admin credentials

### Development Environment
- Docker Desktop running
- OMNISIGHT project cloned
- 2GB+ free disk space

### ML Model (Optional for Initial Testing)
- TensorFlow Lite model (.tflite file)
- Recommended: SSD MobileNet v2 COCO (300×300)
- Will upload separately after package deployment

---

## Build Process

### Step 1: Verify Environment

```bash
# Check Docker is running
docker ps

# Navigate to project
cd "/Users/matthewvisher/Omnisight dev/OmniSight-dev"

# Verify Phase 3 files exist
ls -lh scripts/build-phase3-eap.sh
ls -lh scripts/manifest-phase3.json
ls -lh src/perception/vdo_capture.c
ls -lh src/perception/larod_inference.c
```

### Step 2: Start Docker Environment

```bash
# Start ACAP SDK container
docker-compose up -d omnisight-dev

# Verify container is running
docker ps | grep omnisight-dev
```

### Step 3: Build Phase 3 Package

**Option A: Automatic Build (Recommended)**
```bash
# Run build script in Docker
docker-compose exec omnisight-dev bash -c "cd /opt/app && ./scripts/build-phase3-eap.sh"
```

**Option B: Manual Build**
```bash
# Enter Docker container
docker-compose exec omnisight-dev bash

# Inside container:
cd /opt/app
mkdir -p build-phase3
cd build-phase3

# Configure with CMake
cmake .. \
  -DCMAKE_TOOLCHAIN_FILE=/opt/axis/acapsdk/sysroots/x86_64-pokysdk-linux/usr/share/cmake/axis-toolchain.cmake \
  -DCMAKE_BUILD_TYPE=Release \
  -DENABLE_HARDWARE_APIS=ON

# Build
make -j$(nproc)

# Package
cd /opt/app
# Follow packaging steps from build script
```

### Step 4: Extract Package

```bash
# Exit Docker container (Ctrl+D or 'exit')

# Copy .eap package to host
docker cp omnisight-dev:/opt/app/OMNISIGHT_-_Hardware_0.4.0_aarch64.eap ./

# Verify package
ls -lh OMNISIGHT_-_Hardware_0.4.0_aarch64.eap
```

**Expected Output**: ~2-5MB .eap file

---

## Deployment to Camera

### Method 1: Web Interface (Easiest)

1. Open browser: `https://<camera-ip>`
2. Login with admin credentials
3. Navigate: **Settings → Apps**
4. Click **Add** button (+ icon)
5. Select: `OMNISIGHT_-_Hardware_0.4.0_aarch64.eap`
6. Click **Install**
7. Wait for installation (~30 seconds)
8. Click **Start**

**Web Interface Location**: `https://<camera-ip>/local/omnisight/`

### Method 2: Command Line

```bash
# Set camera details
CAMERA_IP="192.168.1.100"
CAMERA_USER="root"
CAMERA_PASS="your_password"

# Upload package
curl -k --anyauth -u $CAMERA_USER:$CAMERA_PASS \
  -F "package=@OMNISIGHT_-_Hardware_0.4.0_aarch64.eap" \
  https://$CAMERA_IP/axis-cgi/applications/upload.cgi

# Start application
curl -k --anyauth -u $CAMERA_USER:$CAMERA_PASS \
  "https://$CAMERA_IP/axis-cgi/applications/control.cgi?action=start&package=omnisight"

# Check status
curl -k --anyauth -u $CAMERA_USER:$CAMERA_PASS \
  "https://$CAMERA_IP/axis-cgi/applications/list.cgi"
```

### Method 3: SSH (Advanced)

```bash
# Copy package to camera
scp OMNISIGHT_-_Hardware_0.4.0_aarch64.eap root@<camera-ip>:/tmp/

# SSH into camera
ssh root@<camera-ip>

# Install via web interface or API (SSH installation not recommended)
```

---

## Post-Deployment Steps

### 1. Verify Installation

```bash
# Check application status
curl -k "https://root:password@<camera-ip>/local/omnisight/api/health" | jq .

# Expected response:
{
  "status": "healthy",
  "version": "0.4.0",
  "modules": {
    "perception": true,
    "timeline": true,
    "swarm": true
  },
  "ipc": {
    "c_core_running": true,
    "data_fresh": true,
    "json_accessible": true
  }
}
```

### 2. Check Syslog

```bash
# SSH into camera
ssh root@<camera-ip>

# Monitor OMNISIGHT logs
tail -f /var/log/messages | grep -E 'Perception|VDO|Larod|Tracker|Behavior'

# Expected output:
[Perception] Engine initialized successfully
[Perception] Frame size: 416x416 @ 10 FPS
[Perception] Model: /usr/local/packages/omnisight/models/detection.tflite
[Perception] Using DLPU for inference
[VDO] Stream started on channel 1
[Larod] Connected to Larod service
[Larod] Model loaded successfully on device: dlpu
```

### 3. Upload ML Model (If Not Included)

```bash
# Create models directory on camera
ssh root@<camera-ip> "mkdir -p /usr/local/packages/omnisight/models"

# Upload TensorFlow Lite model
scp ssd_mobilenet_v2_coco_quant.tflite \
  root@<camera-ip>:/usr/local/packages/omnisight/models/detection.tflite

# Verify upload
ssh root@<camera-ip> "ls -lh /usr/local/packages/omnisight/models/"

# Restart OMNISIGHT to load model
curl -k --anyauth -u root:password \
  "https://<camera-ip>/axis-cgi/applications/control.cgi?action=stop&package=omnisight"

curl -k --anyauth -u root:password \
  "https://<camera-ip>/axis-cgi/applications/control.cgi?action=start&package=omnisight"
```

### 4. Test API Endpoints

```bash
# System health
curl -k "https://root:password@<camera-ip>/local/omnisight/api/health"

# Perception status
curl -k "https://root:password@<camera-ip>/local/omnisight/api/perception/status"

# Active tracks
curl -k "https://root:password@<camera-ip>/local/omnisight/api/perception/tracks"

# Timeline predictions
curl -k "https://root:password@<camera-ip>/local/omnisight/api/timeline/predictions"
```

---

## Monitoring Performance

### Real-Time Monitoring

```bash
# SSH into camera
ssh root@<camera-ip>

# Monitor CPU/Memory
top | grep omnisight

# Expected:
# PID   CPU%  MEM%  COMMAND
# 1234  25%   180MB omnisight

# Check frame processing
journalctl -u omnisight -f

# Monitor inference time
curl -k "https://root:password@<camera-ip>/local/omnisight/api/perception/status" | jq .avg_inference_ms
```

### Performance Targets

| Metric | Target | Command to Check |
|--------|--------|------------------|
| **CPU Usage** | <30% | `top \| grep omnisight` |
| **Memory** | <512MB | `cat /proc/$(pidof omnisight)/status \| grep VmRSS` |
| **Inference Time** | <20ms | `curl .../api/perception/status \| jq .avg_inference_ms` |
| **FPS** | 10 fps | `curl .../api/perception/status \| jq .fps` |
| **Dropped Frames** | <1% | Check `frames_dropped` in stats |

---

## Troubleshooting

### Application Won't Start

**Symptom**: Package installs but doesn't start

**Solutions**:
```bash
# Check syslog for errors
ssh root@<camera-ip>
tail -100 /var/log/messages | grep -i error

# Common issues:
# 1. Missing ML model
ls /usr/local/packages/omnisight/models/
# Solution: Upload model as shown above

# 2. Insufficient memory
free -m
# Solution: Stop other ACAP apps or increase RAM allocation

# 3. VDO already in use
# Solution: Stop conflicting apps via web interface
```

### VDO Capture Fails

**Symptom**: "VDO capture initialization failed" in logs

**Solutions**:
```bash
# Check firmware version (must be 12.0+)
cat /etc/os-release

# Verify no other app is using video
curl -k --anyauth -u root:password \
  "https://<camera-ip>/axis-cgi/applications/list.cgi"

# Restart camera if needed
curl -k --anyauth -u root:password \
  "https://<camera-ip>/axis-cgi/restart.cgi"
```

### Larod Inference Slow (>20ms)

**Symptom**: Inference takes >20ms, FPS drops

**Solutions**:
1. **Verify DLPU is used** (not CPU fallback):
   ```bash
   curl .../api/perception/status | jq .larod_backend
   # Should show: "dlpu"
   ```

2. **Use INT8 quantized model**:
   - Convert with TensorFlow Lite post-training quantization
   - Expected speedup: 4-8x over FP32

3. **Reduce model size**:
   - Use MobileNet v2 instead of v3
   - Reduce input resolution (300×300 instead of 416×416)

4. **Check DLPU power**:
   ```bash
   # DLPU may need time to power up
   # Wait 60 seconds after first start
   ```

### No Detections

**Symptom**: Tracks count is always 0

**Solutions**:
```bash
# Check model path
ssh root@<camera-ip>
ls -lh /usr/local/packages/omnisight/models/

# Verify model format (must be .tflite)
file /usr/local/packages/omnisight/models/detection.tflite

# Check confidence threshold
curl .../api/perception/status | jq .confidence_threshold
# Default: 0.5 (try lowering to 0.3 for testing)

# Test with known objects in frame
# Point camera at people or vehicles
```

### High CPU Usage (>30%)

**Solutions**:
1. **Reduce framerate**:
   - Dynamic framerate should auto-adjust
   - Manually set lower target in config

2. **Optimize model**:
   - Use smaller model (MobileNet v2 Lite)
   - Reduce input resolution

3. **Check for memory leaks**:
   ```bash
   watch -n 1 'cat /proc/$(pidof omnisight)/status | grep VmRSS'
   # Memory should stabilize, not grow
   ```

---

## Upgrading from Phase 2

If you have Phase 2 (v0.2.x or v0.3.x) installed:

```bash
# Stop old version
curl -k --anyauth -u root:password \
  "https://<camera-ip>/axis-cgi/applications/control.cgi?action=stop&package=omnisight"

# Remove old version (optional - upload will overwrite)
curl -k --anyauth -u root:password \
  "https://<camera-ip>/axis-cgi/applications/control.cgi?action=remove&package=omnisight"

# Upload Phase 3 package
curl -k --anyauth -u root:password \
  -F "package=@OMNISIGHT_-_Hardware_0.4.0_aarch64.eap" \
  https://<camera-ip>/axis-cgi/applications/upload.cgi

# Start Phase 3
curl -k --anyauth -u root:password \
  "https://<camera-ip>/axis-cgi/applications/control.cgi?action=start&package=omnisight"
```

**Note**: Phase 3 uses different data paths. Old stub data is not compatible.

---

## Multi-Camera Deployment

For swarm mode with 3+ cameras:

### 1. Deploy to All Cameras

```bash
# Use same .eap package for all cameras
for IP in 192.168.1.100 192.168.1.101 192.168.1.102; do
  echo "Deploying to $IP..."
  curl -k --anyauth -u root:password \
    -F "package=@OMNISIGHT_-_Hardware_0.4.0_aarch64.eap" \
    https://$IP/axis-cgi/applications/upload.cgi
done
```

### 2. Configure MQTT Broker

```bash
# Edit configuration on each camera (future enhancement)
# For Phase 3, MQTT configuration is hardcoded
# Will be configurable in Phase 4
```

### 3. Verify Swarm Communication

```bash
# Check swarm status on each camera
for IP in 192.168.1.100 192.168.1.101 192.168.1.102; do
  echo "Camera $IP:"
  curl -k "https://root:password@$IP/local/omnisight/api/swarm/network" | jq .
done
```

---

## Next Steps After Deployment

### Phase 3 Validation Checklist

- [ ] Package builds successfully in Docker
- [ ] .eap installs on P3285-LVE camera
- [ ] Application starts without errors
- [ ] VDO captures frames at 10 FPS
- [ ] Larod inference runs on DLPU (<20ms)
- [ ] Objects are detected and tracked
- [ ] Behaviors are analyzed (loitering, running)
- [ ] API endpoints return live data
- [ ] Web dashboard displays real-time tracks
- [ ] Memory usage <512MB after 1 hour
- [ ] CPU usage <30% sustained

### Timeline Module (Week 10)
Once perception is validated:
- Implement trajectory prediction (Kalman filter)
- Implement event forecasting algorithms
- Generate 3-5 timeline branches

### Swarm Module (Week 11)
After timeline is working:
- Implement MQTT client for track sharing
- Implement federated learning framework
- Test multi-camera coordination

### Production Hardening (Week 12)
Final polish:
- Performance optimization
- Error recovery improvements
- Configuration management
- Load testing

---

## Support and Resources

### Documentation
- **Phase 3 Summary**: PHASE3_SUMMARY.md
- **Deployment Guide**: PHASE3_DEPLOYMENT_GUIDE.md
- **Quick Start**: PHASE3_QUICK_START.md
- **Implementation Plan**: PHASE3_IMPLEMENTATION_PLAN.md

### Logs and Debugging
- **System log**: `/var/log/messages` (on camera)
- **Application log**: `journalctl -u omnisight`
- **API health**: `https://<camera-ip>/local/omnisight/api/health`

### Common Commands
```bash
# Restart OMNISIGHT
curl -k --anyauth -u root:password \
  "https://<camera-ip>/axis-cgi/applications/control.cgi?action=restart&package=omnisight"

# View live logs
ssh root@<camera-ip> "tail -f /var/log/messages | grep OMNISIGHT"

# Check resource usage
ssh root@<camera-ip> "top -b -n 1 | grep omnisight"
```

---

## Success Metrics

**Phase 3 is considered successful when:**

1. ✅ Application deploys to camera without errors
2. ✅ VDO captures video at target framerate (10 FPS)
3. ✅ Larod inference meets <20ms target on DLPU
4. ✅ Object tracking maintains persistent IDs across frames
5. ✅ Behavior analysis detects loitering and running
6. ✅ Memory usage stays <512MB for 24 hours
7. ✅ CPU usage averages <30% under normal load
8. ✅ API endpoints return accurate real-time data
9. ✅ Web dashboard visualizes live tracks
10. ✅ System handles occlusions and lighting changes

---

**Phase 3 Build and Deployment Guide Complete**

All files are ready for hardware testing. The next step is to execute the build process and deploy to the P3285-LVE camera for validation.
