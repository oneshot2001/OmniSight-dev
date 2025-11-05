# OMNISIGHT Phase 3 Deployment Guide

**Version:** 0.4.0
**Status:** Hardware Integration Ready
**Last Updated:** October 2024

## Overview

Phase 3 represents the complete OMNISIGHT system with full hardware integration:
- **VDO API** for real-time video capture from ARTPEC-8/9 cameras
- **Larod API** for DLPU-accelerated ML inference
- **MQTT** for multi-camera swarm coordination
- **Real algorithms** for tracking, prediction, and federated learning

This guide covers building, deploying, and monitoring the Phase 3 hardware-enabled system.

---

## Prerequisites

### Hardware Requirements

**Minimum Camera Specifications:**
- **Processor:** ARTPEC-8 or ARTPEC-9
- **Firmware:** AXIS OS 12.0 or later
- **RAM:** 2GB minimum (4GB recommended for swarm mode)
- **Storage:** 4GB available space (for application + model)
- **Network:** 100Mbps Ethernet with MQTT broker access

**Tested Cameras:**
- Axis M4228-LVE (ARTPEC-8)
- Axis Q1656-DLE (ARTPEC-9)
- Any ARTPEC-8/9 camera with AXIS OS 12.0+

### Development Environment

**Required:**
- Docker Desktop (20.10+)
- ACAP Native SDK 1.15 (automatically provisioned in Docker)
- Git
- 8GB+ RAM on host machine
- 20GB+ free disk space

**Optional:**
- MQTT broker (Mosquitto recommended)
- Multiple cameras for swarm testing
- GPU for local model training

---

## Building the Phase 3 Package

### Step 1: Start Docker Development Environment

```bash
cd /path/to/OmniSight-dev
docker-compose up -d omnisight-dev
docker-compose exec omnisight-dev bash
```

You should now be inside the container at `/opt/app`.

### Step 2: Run the Phase 3 Build Script

```bash
cd /opt/app
./scripts/build-phase3-eap.sh
```

**Build Process:**
1. Configures CMake with `ENABLE_HARDWARE_APIS=ON`
2. Cross-compiles for ARM aarch64 using ACAP SDK toolchain
3. Links VDO, Larod, GLib, pthread, and MQTT libraries
4. Strips debug symbols to reduce package size
5. Packages Flask API server with IPC layer
6. Creates ML model placeholder
7. Generates startup script with hardware initialization
8. Creates `.eap` package with manifest schema 1.8.0

**Expected Output:**
```
================================================
✓ Phase 3 Package Created Successfully!
================================================

📦 Package: OMNISIGHT_-_Hardware_0.4.0_aarch64.eap
📊 Size: ~15-20MB (without model)
📁 Location: /opt/app/OMNISIGHT_-_Hardware_0.4.0_aarch64.eap
```

### Step 3: Extract Package from Docker

```bash
# From your host machine (not inside Docker):
docker cp omnisight-dev:/opt/app/OMNISIGHT_-_Hardware_0.4.0_aarch64.eap ./
```

---

## Preparing the ML Model

### Model Requirements

**Format:** TensorFlow Lite (`.tflite`)

**Input Specification:**
- Shape: `[1, 416, 416, 3]`
- Type: FLOAT32
- Normalization: [0.0, 1.0] (RGB image)
- Preprocessing: Resize and normalize in VDO capture layer

**Output Specification (YOLO-style):**
- Bounding boxes: `[1, N, 4]` (x, y, w, h normalized)
- Confidence scores: `[1, N, 1]` (object confidence)
- Class probabilities: `[1, N, num_classes]`

**Performance:**
- Size: <100MB recommended for DLPU
- Quantization: INT8 or FP16 for best ARTPEC performance
- Target inference time: <20ms per frame

### Model Conversion

If you have a trained PyTorch or TensorFlow model:

```bash
# Inside Docker container
cd /opt/app
python3 scripts/convert_model.py \
    --input models/trained_model.pt \
    --output models/omnisight_detection.tflite \
    --quantize int8
```

Or use TensorFlow directly:

```python
import tensorflow as tf

# Load your model
model = tf.keras.models.load_model('trained_model.h5')

# Convert to TFLite
converter = tf.lite.TFLiteConverter.from_keras_model(model)
converter.optimizations = [tf.lite.Optimize.DEFAULT]
converter.target_spec.supported_types = [tf.int8]

tflite_model = converter.convert()

# Save
with open('omnisight_detection.tflite', 'wb') as f:
    f.write(tflite_model)
```

### Model Validation

Test model inference speed on your camera:

```bash
# SSH to camera
ssh root@<camera-ip>

# Test with Larod
larod-client -i omnisight_detection.tflite -c 10
```

Expected output: `<20ms` per inference.

---

## Deployment to Camera

### Method 1: Web Interface (Recommended)

1. **Open Camera Web Interface:**
   ```
   https://<camera-ip>
   ```

2. **Navigate to Apps:**
   - Click "Settings" → "Apps"

3. **Upload ACAP Package:**
   - Click "Add app"
   - Select `OMNISIGHT_-_Hardware_0.4.0_aarch64.eap`
   - Click "Upload"

4. **Wait for Installation:**
   - Installation takes 30-60 seconds
   - Status changes to "Installed"

5. **Start Application:**
   - Click the "Start" button
   - Status changes to "Running"

6. **Access Dashboard:**
   ```
   https://<camera-ip>/local/omnisight/
   ```

### Method 2: Command Line (cURL)

```bash
# Upload package
curl -k --anyauth -u root:<password> \
  -F "package=@OMNISIGHT_-_Hardware_0.4.0_aarch64.eap" \
  https://<camera-ip>/axis-cgi/applications/upload.cgi

# Start application
curl -k --anyauth -u root:<password> \
  "https://<camera-ip>/axis-cgi/applications/control.cgi?action=start&package=omnisight"

# Check status
curl -k --anyauth -u root:<password> \
  "https://<camera-ip>/axis-cgi/applications/list.cgi"
```

### Method 3: ACAP SDK Deployment Tool

```bash
# From Docker container
cd /opt/app
make deploy CAMERA_IP=<camera-ip> CAMERA_USER=root CAMERA_PASS=<password>
```

---

## Uploading the ML Model

**CRITICAL:** The model must be uploaded separately after ACAP installation.

### Step 1: Create Model Directory (if needed)

```bash
ssh root@<camera-ip>
mkdir -p /usr/local/packages/omnisight/models
```

### Step 2: Upload Model

```bash
scp omnisight_detection.tflite root@<camera-ip>:/usr/local/packages/omnisight/models/
```

### Step 3: Verify Upload

```bash
ssh root@<camera-ip>
ls -lh /usr/local/packages/omnisight/models/

# Expected output:
# -rw-r--r-- 1 root root 45M Oct 30 10:00 omnisight_detection.tflite
```

### Step 4: Restart OMNISIGHT

```bash
# Via web interface: Stop → Start
# Or via CLI:
curl -k --anyauth -u root:<password> \
  "https://<camera-ip>/axis-cgi/applications/control.cgi?action=restart&package=omnisight"
```

### Step 5: Verify Model Loading

Check system health endpoint:

```bash
curl -k --anyauth -u root:<password> \
  "https://<camera-ip>/local/omnisight/api/health" | jq .

# Expected response:
{
  "status": "healthy",
  "model_loaded": true,
  "model_path": "/usr/local/packages/omnisight/models/omnisight_detection.tflite",
  "vdo_active": true,
  "larod_active": true,
  "mqtt_connected": true
}
```

---

## Monitoring and Logs

### Real-Time System Logs (syslog)

```bash
# SSH to camera
ssh root@<camera-ip>

# Follow OMNISIGHT logs
tail -f /var/log/messages | grep OMNISIGHT

# Expected log messages:
# OMNISIGHT: Starting Phase 3 (Hardware Integration)...
# OMNISIGHT: VDO stream initialized: 1920x1080 @ 10fps
# OMNISIGHT: Larod model loaded: omnisight_detection.tflite
# OMNISIGHT: DLPU inference engine ready (<20ms avg)
# OMNISIGHT: MQTT connected to broker
# OMNISIGHT: Swarm coordinator started
# OMNISIGHT: Core started (PID: 1234)
# OMNISIGHT: Flask API server starting on port 8080...
```

### System Stats API

Monitor performance in real-time:

```bash
# Get system statistics
curl -k --anyauth -u root:<password> \
  "https://<camera-ip>/local/omnisight/api/stats" | jq .
```

**Response:**
```json
{
  "uptime_seconds": 3600,
  "fps": 9.8,
  "avg_inference_ms": 18.5,
  "tracked_objects": 3,
  "timeline_predictions": 5,
  "swarm_cameras": 4,
  "memory_usage_mb": 420,
  "cpu_usage_percent": 28
}
```

### Performance Metrics Dashboard

Create a monitoring loop:

```bash
#!/bin/bash
while true; do
  clear
  echo "=== OMNISIGHT Performance Monitor ==="
  curl -sk "https://root:password@<camera-ip>/local/omnisight/api/stats" | jq .
  sleep 5
done
```

### Perception Engine Status

Check object detection and tracking:

```bash
curl -k --anyauth -u root:<password> \
  "https://<camera-ip>/local/omnisight/api/perception/status" | jq .
```

**Response:**
```json
{
  "active": true,
  "fps": 9.8,
  "avg_inference_ms": 18.5,
  "detected_objects": 5,
  "tracked_objects": 3,
  "dropped_frames": 2,
  "vdo_resolution": "1920x1080",
  "larod_backend": "artpec8-dlpu"
}
```

### Timeline Predictions

Get current timeline predictions:

```bash
curl -k --anyauth -u root:<password> \
  "https://<camera-ip>/local/omnisight/api/timeline/predictions" | jq .
```

### Swarm Network Status

View multi-camera coordination:

```bash
curl -k --anyauth -u root:<password> \
  "https://<camera-ip>/local/omnisight/api/swarm/network" | jq .
```

---

## MQTT Configuration (Swarm Mode)

### Prerequisites

- MQTT broker accessible to all cameras (Mosquitto recommended)
- Network connectivity between cameras
- Broker hostname/IP and port

### Configure MQTT Broker

**On camera:**

```bash
ssh root@<camera-ip>
vi /usr/local/packages/omnisight/config.json
```

**Edit configuration:**

```json
{
  "mqtt": {
    "broker": "mqtt://mosquitto-server.local",
    "port": 1883,
    "username": "omnisight",
    "password": "secure-password",
    "topics": {
      "tracks": "omnisight/tracks",
      "events": "omnisight/events",
      "consensus": "omnisight/consensus"
    }
  }
}
```

**Restart OMNISIGHT:**

```bash
curl -k --anyauth -u root:<password> \
  "https://<camera-ip>/axis-cgi/applications/control.cgi?action=restart&package=omnisight"
```

### Verify MQTT Connection

```bash
# Check health endpoint
curl -k --anyauth -u root:<password> \
  "https://<camera-ip>/local/omnisight/api/health" | jq .mqtt_connected

# Should return: true
```

### Test Swarm Coordination

Deploy to multiple cameras and verify track sharing:

```bash
# Camera 1
curl -k "https://root:pass@camera1/local/omnisight/api/swarm/network" | jq .

# Expected: Lists other cameras in swarm
{
  "swarm_id": "omnisight-swarm-1",
  "camera_count": 4,
  "cameras": [
    {"id": "camera1", "status": "active"},
    {"id": "camera2", "status": "active"},
    {"id": "camera3", "status": "active"},
    {"id": "camera4", "status": "active"}
  ]
}
```

---

## Performance Validation

### Expected Performance Metrics

| Metric | Target | Acceptable Range |
|--------|--------|------------------|
| **Inference Speed** | <20ms | 15-25ms |
| **FPS** | 10 fps | 8-12 fps |
| **Memory Usage** | <512MB | 400-550MB |
| **CPU Usage** | <30% | 25-35% |
| **Prediction Horizon** | 5 minutes | 3-7 minutes |
| **False Positive Rate** | <3% | 2-5% |
| **Network Bandwidth** | <500Kbps | 400-600Kbps |

### Run Performance Tests

```bash
# 10-minute burn-in test
./scripts/performance-test.sh https://<camera-ip> 600

# Expected output:
# Avg inference: 18.2ms
# Avg FPS: 9.9
# Peak memory: 485MB
# Avg CPU: 27%
# Dropped frames: 0.02%
```

### Load Testing

Simulate high-traffic scenarios:

```bash
# Multiple concurrent API requests
ab -n 1000 -c 10 "https://root:pass@<camera-ip>/local/omnisight/api/stats"

# Check for performance degradation
```

---

## Troubleshooting

### Issue: Model Not Loading

**Symptoms:**
- `/api/health` shows `"model_loaded": false`
- Logs: "WARNING: ML model not found"

**Solution:**
1. Verify model exists:
   ```bash
   ssh root@<camera-ip>
   ls -lh /usr/local/packages/omnisight/models/omnisight_detection.tflite
   ```
2. Check file permissions:
   ```bash
   chmod 644 /usr/local/packages/omnisight/models/*.tflite
   ```
3. Verify TFLite format:
   ```bash
   file omnisight_detection.tflite
   # Should output: TensorFlow Lite model
   ```
4. Restart application

---

### Issue: VDO Capture Fails

**Symptoms:**
- Logs: "ERROR: VDO stream initialization failed"
- `/api/perception/status` shows `"active": false`

**Solution:**
1. Check camera firmware version:
   ```bash
   ssh root@<camera-ip>
   cat /etc/os-release | grep VERSION
   # Must be AXIS OS 12.0+
   ```
2. Verify VDO permissions in manifest:
   ```json
   "permissions": {
     "video": {
       "streamResolution": {"width": 1920, "height": 1080}
     }
   }
   ```
3. Check for conflicting ACAP apps:
   ```bash
   curl -k --anyauth -u root:pass \
     "https://<camera-ip>/axis-cgi/applications/list.cgi"
   ```
4. Restart camera if needed

---

### Issue: Larod Inference Slow

**Symptoms:**
- Inference time >30ms
- FPS drops below 7
- Logs: "WARNING: Inference timeout"

**Solution:**
1. Verify DLPU is being used:
   ```bash
   # Check Larod backend
   curl "https://<camera-ip>/local/omnisight/api/perception/status" | jq .larod_backend
   # Should return: "artpec8-dlpu" or "artpec9-dlpu"
   ```
2. Optimize model:
   - Reduce input resolution (try 320x320)
   - Apply INT8 quantization
   - Prune unused layers
3. Check model size:
   ```bash
   du -h /usr/local/packages/omnisight/models/*.tflite
   # Should be <100MB
   ```
4. Verify camera isn't overheating:
   ```bash
   cat /sys/class/thermal/thermal_zone0/temp
   # Should be <80000 (80°C)
   ```

---

### Issue: MQTT Connection Fails

**Symptoms:**
- `/api/health` shows `"mqtt_connected": false`
- Logs: "ERROR: MQTT broker unreachable"

**Solution:**
1. Verify broker is accessible:
   ```bash
   ssh root@<camera-ip>
   ping mqtt-broker.local
   telnet mqtt-broker.local 1883
   ```
2. Check MQTT configuration:
   ```bash
   cat /usr/local/packages/omnisight/config.json | grep mqtt
   ```
3. Test with mosquitto_pub:
   ```bash
   mosquitto_pub -h mqtt-broker.local -t test -m "hello"
   ```
4. Verify firewall rules:
   ```bash
   iptables -L | grep 1883
   ```

---

### Issue: High Memory Usage

**Symptoms:**
- Memory usage >600MB
- Camera becomes unresponsive
- Logs: "WARNING: Low memory"

**Solution:**
1. Check for memory leaks:
   ```bash
   # Monitor memory over time
   watch -n 5 'curl -sk "https://root:pass@<camera-ip>/local/omnisight/api/stats" | jq .memory_usage_mb'
   ```
2. Reduce tracked objects:
   ```bash
   # Update config
   curl -X POST "https://<camera-ip>/local/omnisight/api/config" \
     -d '{"max_tracked_objects": 10}'
   ```
3. Disable unused features:
   ```bash
   # Disable swarm if not needed
   curl -X POST "https://<camera-ip>/local/omnisight/api/config" \
     -d '{"swarm_enabled": false}'
   ```
4. Restart application periodically (via cron)

---

## Multi-Camera Deployment

### Deployment Strategy

**For 2-10 cameras:**
1. Deploy to all cameras sequentially
2. Configure MQTT broker connection on each
3. Wait 5 minutes for swarm convergence
4. Verify network topology via `/api/swarm/network`

**For 10+ cameras:**
1. Deploy in batches of 10
2. Use hierarchical MQTT topics per building/floor
3. Implement regional coordinators
4. Monitor bandwidth usage

### Automated Deployment Script

Create `cameras.txt`:
```
192.168.1.100
192.168.1.101
192.168.1.102
```

Run batch deployment:

```bash
#!/bin/bash
for camera in $(cat cameras.txt); do
  echo "Deploying to $camera..."
  curl -k --anyauth -u root:password \
    -F "package=@OMNISIGHT_-_Hardware_0.4.0_aarch64.eap" \
    "https://$camera/axis-cgi/applications/upload.cgi"

  sleep 10

  curl -k --anyauth -u root:password \
    "https://$camera/axis-cgi/applications/control.cgi?action=start&package=omnisight"
done
```

---

## Upgrading from Phase 2

If you already have Phase 2 deployed:

### Step 1: Backup Configuration

```bash
ssh root@<camera-ip>
cp -r /usr/local/packages/omnisight/config.json /tmp/omnisight-config-backup.json
```

### Step 2: Stop Phase 2

```bash
curl -k --anyauth -u root:<password> \
  "https://<camera-ip>/axis-cgi/applications/control.cgi?action=stop&package=omnisight"
```

### Step 3: Uninstall Phase 2

```bash
curl -k --anyauth -u root:<password> \
  "https://<camera-ip>/axis-cgi/applications/control.cgi?action=remove&package=omnisight"
```

### Step 4: Install Phase 3

Follow standard deployment steps above.

### Step 5: Restore Configuration

```bash
ssh root@<camera-ip>
cp /tmp/omnisight-config-backup.json /usr/local/packages/omnisight/config.json
```

### Step 6: Upload Model

Follow ML model upload steps.

---

## Uninstalling

### Complete Removal

```bash
# Stop application
curl -k --anyauth -u root:<password> \
  "https://<camera-ip>/axis-cgi/applications/control.cgi?action=stop&package=omnisight"

# Remove application
curl -k --anyauth -u root:<password> \
  "https://<camera-ip>/axis-cgi/applications/control.cgi?action=remove&package=omnisight"

# Clean up data (optional)
ssh root@<camera-ip>
rm -rf /usr/local/packages/omnisight
rm -f /tmp/omnisight*
```

---

## Support and Resources

### Documentation
- **Project README:** `/README.md`
- **Phase 1 Status:** `/PHASE1_STATUS.md`
- **Phase 2 Roadmap:** `/PHASE2_ROADMAP.md`
- **ACAP Packaging:** `/ACAP_PACKAGING.md`
- **Web Interface:** `/web/README.md`
- **API Documentation:** `/app/README.md`

### External Resources
- **Axis ACAP Developer Guide:** https://developer.axis.com/acap
- **ACAP Native SDK:** https://github.com/AxisCommunications/acap-sdk
- **VDO API Documentation:** https://developer.axis.com/vapix/api/vdo
- **Larod API Documentation:** https://developer.axis.com/acap/larod
- **MQTT Protocol:** https://mqtt.org/

### Getting Help
- **GitHub Issues:** https://github.com/oneshot2001/OmniSight-dev/issues
- **Axis Developer Forum:** https://www.axis.com/developer-community
- **Project Discussions:** Check repository for Q&A

---

## Next Steps

After successful deployment:

1. **Baseline Performance:** Run 24-hour test to establish performance baseline
2. **Tune Configuration:** Adjust detection thresholds, FPS, resolution for your use case
3. **Train Custom Model:** Fine-tune on your specific environment and threat scenarios
4. **Scale to Swarm:** Deploy to multiple cameras and test coordination
5. **Monitor Long-Term:** Set up automated monitoring and alerting
6. **Iterate:** Collect feedback and improve detection accuracy

---

**Phase 3 deployment complete. OMNISIGHT is now running with full hardware integration.**
