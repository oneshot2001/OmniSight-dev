# OMNISIGHT Phase 3 Quick Start

**One-page guide for building and deploying Phase 3 hardware integration.**

---

## Build Phase 3 Package

```bash
# Start Docker
docker-compose up -d omnisight-dev
docker-compose exec omnisight-dev bash

# Inside Docker container:
cd /opt/app
./scripts/build-phase3-eap.sh

# Extract package (from host):
docker cp omnisight-dev:/opt/app/OMNISIGHT_-_Hardware_0.4.0_aarch64.eap ./
```

---

## Deploy to Camera

### Option 1: Web Interface
1. Navigate to `https://<camera-ip>`
2. Go to **Settings → Apps → Add app**
3. Upload `OMNISIGHT_-_Hardware_0.4.0_aarch64.eap`
4. Click **Start**

### Option 2: Command Line
```bash
# Upload
curl -k --anyauth -u root:<password> \
  -F "package=@OMNISIGHT_-_Hardware_0.4.0_aarch64.eap" \
  https://<camera-ip>/axis-cgi/applications/upload.cgi

# Start
curl -k --anyauth -u root:<password> \
  "https://<camera-ip>/axis-cgi/applications/control.cgi?action=start&package=omnisight"
```

---

## Upload ML Model

```bash
# Create directory
ssh root@<camera-ip>
mkdir -p /usr/local/packages/omnisight/models

# Upload model
scp omnisight_detection.tflite \
  root@<camera-ip>:/usr/local/packages/omnisight/models/

# Restart OMNISIGHT
curl -k --anyauth -u root:<password> \
  "https://<camera-ip>/axis-cgi/applications/control.cgi?action=restart&package=omnisight"
```

---

## Verify Deployment

### Check Health
```bash
curl -k "https://root:<password>@<camera-ip>/local/omnisight/api/health" | jq .
```

**Expected:**
```json
{
  "status": "healthy",
  "model_loaded": true,
  "vdo_active": true,
  "larod_active": true,
  "mqtt_connected": true
}
```

### Monitor Performance
```bash
curl -k "https://root:<password>@<camera-ip>/local/omnisight/api/stats" | jq .
```

**Expected:**
```json
{
  "fps": 9.8,
  "avg_inference_ms": 18.5,
  "memory_usage_mb": 420,
  "cpu_usage_percent": 28
}
```

### View Logs
```bash
ssh root@<camera-ip>
tail -f /var/log/messages | grep OMNISIGHT
```

**Expected logs:**
```
OMNISIGHT: Starting Phase 3 (Hardware Integration)...
OMNISIGHT: VDO stream initialized: 1920x1080 @ 10fps
OMNISIGHT: Larod model loaded: omnisight_detection.tflite
OMNISIGHT: DLPU inference engine ready (<20ms avg)
OMNISIGHT: Core started (PID: 1234)
OMNISIGHT: Flask API server starting on port 8080...
```

---

## Access Dashboard

Open in browser:
```
https://<camera-ip>/local/omnisight/
```

---

## Performance Targets

| Metric | Target | Acceptable |
|--------|--------|------------|
| Inference | <20ms | 15-25ms |
| FPS | 10 | 8-12 |
| Memory | <512MB | 400-550MB |
| CPU | <30% | 25-35% |

---

## Troubleshooting

### Model not loading
```bash
ls -lh /usr/local/packages/omnisight/models/
chmod 644 /usr/local/packages/omnisight/models/*.tflite
```

### VDO fails
- Check firmware: Must be AXIS OS 12.0+
- Verify no conflicting ACAP apps

### Larod slow
- Use INT8 quantization
- Reduce model size (<100MB)
- Check DLPU backend: `jq .larod_backend /api/perception/status`

### MQTT disconnected
```bash
ping mqtt-broker.local
telnet mqtt-broker.local 1883
```

---

## Multi-Camera Deployment

Create `cameras.txt`:
```
192.168.1.100
192.168.1.101
192.168.1.102
```

Batch deploy:
```bash
for camera in $(cat cameras.txt); do
  curl -k -u root:pass \
    -F "package=@OMNISIGHT_-_Hardware_0.4.0_aarch64.eap" \
    "https://$camera/axis-cgi/applications/upload.cgi"

  curl -k -u root:pass \
    "https://$camera/axis-cgi/applications/control.cgi?action=start&package=omnisight"

  sleep 10
done
```

---

## Uninstall

```bash
# Stop
curl -k -u root:pass \
  "https://<camera-ip>/axis-cgi/applications/control.cgi?action=stop&package=omnisight"

# Remove
curl -k -u root:pass \
  "https://<camera-ip>/axis-cgi/applications/control.cgi?action=remove&package=omnisight"

# Clean data (optional)
ssh root@<camera-ip>
rm -rf /usr/local/packages/omnisight
```

---

**For detailed instructions, see `PHASE3_DEPLOYMENT_GUIDE.md`**
