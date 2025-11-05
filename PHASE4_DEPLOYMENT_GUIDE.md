# Phase 4 Deployment & Testing Guide

> **Version**: v0.7.0
> **Target**: Axis P3285-LVE (ARTPEC-8)
> **Status**: Ready for Hardware Testing

## Overview

This guide provides complete instructions for deploying and testing OMNISIGHT v0.7.0 on the P3285-LVE camera. The package includes the native C HTTP server with React dashboard, ready for production deployment.

## Prerequisites

### Camera Requirements
- **Model**: Axis P3285-LVE (ARTPEC-8 or ARTPEC-9)
- **Firmware**: AXIS OS 12.0+ (tested on 12.6.97)
- **Memory**: 512MB RAM minimum
- **Storage**: 100MB free space
- **Network**: Camera accessible via network

### Package Requirements
- **File**: `OMNISIGHT_0_7_0_aarch64.eap`
- **Size**: 171KB (manifest + React) or ~800KB (with ARM binary)
- **Location**: `packages/OMNISIGHT_0_7_0_aarch64.eap`

### Access Requirements
- Camera admin credentials
- Network access to camera IP
- Web browser (Chrome, Firefox, Safari, Edge)

## Quick Start

### 1. Build Complete Package (Docker)

```bash
# Build with ARM binary
cd /Users/matthewvisher/Omnisight\ dev/OmniSight-dev
docker-compose run omnisight-dev ./scripts/build-phase4-v070.sh

# Verify build
ls -lh packages/OMNISIGHT_0_7_0_aarch64.eap
```

**Expected Output**: Package with ~800KB size (includes ARM binary)

### 2. Upload to Camera (Web Interface)

1. Open camera web interface: `https://camera-ip`
2. Login with admin credentials
3. Navigate: **Settings → Apps → Add**
4. Click **Browse** and select `OMNISIGHT_0_7_0_aarch64.eap`
5. Click **Upload**
6. Wait for installation (10-30 seconds)
7. Click **Start** if app doesn't auto-start
8. Click **Open** to access dashboard

**Expected Result**: Dashboard opens in new tab

### 3. Verify Installation

Access the dashboard:
- **URL**: `https://camera-ip/local/omnisight/`
- **Expected**: React dashboard loads
- **Status**: Shows "Active", FPS ~25, modules running

Test API endpoints:
```bash
# Health check
curl -k https://camera-ip/local/omnisight/api/health

# Expected: {"status":"healthy","version":"0.7.0"}

# Status
curl -k https://camera-ip/local/omnisight/api/status

# Expected: Full system status with modules
```

## Detailed Deployment Steps

### Method 1: Web Interface (Recommended)

**Step 1: Prepare Package**
```bash
# Ensure you have the latest package
cd /Users/matthewvisher/Omnisight\ dev/OmniSight-dev
ls -lh packages/OMNISIGHT_0_7_0_aarch64.eap
```

**Step 2: Access Camera**
1. Open browser
2. Navigate to `https://camera-ip`
3. Accept SSL certificate warning (if any)
4. Login with credentials

**Step 3: Upload App**
1. Click **Settings** (gear icon)
2. Select **Apps** from sidebar
3. Click **Add** button (+ icon)
4. In dialog:
   - Click **Browse**
   - Select `OMNISIGHT_0_7_0_aarch64.eap`
   - Click **Upload**

**Step 4: Monitor Upload**
- Progress bar shows upload status
- Installation happens automatically
- App appears in list after ~10-30 seconds

**Step 5: Start App**
- If status shows "Stopped", click **Start**
- Wait for status to change to "Running"
- **Open** button should appear

**Step 6: Access Dashboard**
- Click **Open** button
- New tab opens with dashboard
- Verify dashboard loads properly

### Method 2: Command Line (Advanced)

**Upload via curl**:
```bash
curl -k -u admin:password \
  -F 'package=@packages/OMNISIGHT_0_7_0_aarch64.eap' \
  https://camera-ip/axis-cgi/applications/upload.cgi
```

**Start app**:
```bash
curl -k -u admin:password \
  "https://camera-ip/axis-cgi/applications/control.cgi?action=start&package=omnisight"
```

**Check status**:
```bash
curl -k -u admin:password \
  "https://camera-ip/axis-cgi/applications/list.cgi"
```

### Method 3: SSH Direct Install (Expert)

**Step 1: Enable SSH on camera**
- Settings → System → Plain Config
- Add: `root.Network.SSH.Enabled: yes`

**Step 2: Copy package**:
```bash
scp packages/OMNISIGHT_0_7_0_aarch64.eap root@camera-ip:/tmp/
```

**Step 3: Install**:
```bash
ssh root@camera-ip
cd /tmp
eap-install.sh install OMNISIGHT_0_7_0_aarch64.eap
systemctl start omnisight.service
```

## Testing Checklist

### Basic Functionality Tests

#### 1. Dashboard Load Test ✓
- [ ] Dashboard URL accessible: `https://camera-ip/local/omnisight/`
- [ ] Page loads without errors
- [ ] React components render correctly
- [ ] No console errors in browser DevTools

#### 2. Static Assets Test ✓
- [ ] HTML loads (index.html)
- [ ] JavaScript bundle loads (580KB)
- [ ] CSS stylesheet loads (37KB)
- [ ] Page styling correct
- [ ] No 404 errors for assets

#### 3. API Endpoints Test ✓
Test each endpoint:

```bash
# Base URL
BASE="https://camera-ip/local/omnisight"

# Health check
curl -k "$BASE/api/health"
# Expected: {"status":"healthy","version":"0.7.0"}

# Status
curl -k "$BASE/api/status"
# Expected: {"status":"active","mode":"acap",...}

# Stats
curl -k "$BASE/api/stats"
# Expected: {"uptime_seconds":123,"memory_usage_mb":128,...}

# Perception
curl -k "$BASE/api/perception/status"
curl -k "$BASE/api/perception/detections"

# Timeline
curl -k "$BASE/api/timeline/predictions"

# Swarm
curl -k "$BASE/api/swarm/network"

# Config
curl -k "$BASE/api/config"
```

#### 4. Module Status Test ✓
- [ ] Perception module: Active, FPS ~25
- [ ] Timeline module: Active, predictions generating
- [ ] Swarm module: Active, local camera visible

#### 5. Performance Test ✓
- [ ] API response time: <100ms
- [ ] Dashboard load time: <3 seconds
- [ ] Memory usage: <256MB
- [ ] CPU usage: <30%

### Advanced Tests

#### 6. Stability Test ✓
- [ ] App stays running for 1+ hours
- [ ] No memory leaks observed
- [ ] Automatic restart on crash (runMode: respawn)
- [ ] No error accumulation in logs

#### 7. Network Test ✓
- [ ] CORS headers present on API responses
- [ ] Concurrent requests handled correctly
- [ ] Dashboard accessible from multiple browsers
- [ ] API endpoints accessible via reverse proxy

#### 8. Log Review ✓
Check app logs:
```bash
# Via web interface
Settings → Apps → OMNISIGHT → Logs → App log

# Via SSH
ssh root@camera-ip
journalctl -u omnisight.service -f
```

Look for:
- [ ] No error messages
- [ ] Modules initialized successfully
- [ ] HTTP server started on port 8080
- [ ] Web root configured correctly

## Expected Behavior

### On First Start

**Console Output** (in logs):
```
Initializing OMNISIGHT core...
[OMNISIGHT] Initializing system...
[OMNISIGHT] Initializing perception engine...
[Perception] ✓ Stub engine initialized
[OMNISIGHT] ✓ Perception engine ready
[OMNISIGHT] Initializing Timeline Threading™...
[Timeline] ✓ Timeline Threading™ stub initialized
[OMNISIGHT] ✓ Timeline Threading™ ready
[OMNISIGHT] Initializing Swarm Intelligence...
[Swarm] ✓ Swarm Intelligence stub initialized
[OMNISIGHT] ✓ Swarm Intelligence ready
Creating HTTP server...
Web root: /usr/local/packages/omnisight/html
Starting HTTP server...
HTTP server started on port 8080
```

### Dashboard Display

**Header**:
- Title: "OMNISIGHT - Precognitive Security"
- Version: v0.7.0
- Status indicator: Green (Active)

**Modules Section**:
- Perception: Active, FPS 25.0, Detections 3
- Timeline: Active, Predictions 2
- Swarm: Active, Cameras 1

**Stats Section**:
- Uptime: Incrementing
- Memory: ~128MB
- CPU: ~15%
- FPS: 25.0

### API Responses

**GET /api/health**:
```json
{
  "status": "healthy",
  "version": "0.7.0"
}
```

**GET /api/status**:
```json
{
  "status": "active",
  "mode": "acap",
  "version": "0.7.0",
  "uptime_seconds": 123,
  "fps": 25.0,
  "latency_ms": 0.0,
  "modules": {
    "perception": {"status": "active", "fps": 25.0, "detections": 3},
    "timeline": {"status": "active", "predictions": 2},
    "swarm": {"status": "active", "cameras": 1}
  }
}
```

## Troubleshooting

### Issue: "Open" Button Not Appearing

**Symptoms**: App installed, running, but no "Open" button in camera UI

**Causes**:
1. `settingPage` not configured in manifest
2. HTML files not in correct location
3. Manifest validation failed

**Solutions**:
```bash
# Check manifest
tar -tzf OMNISIGHT_0_7_0_aarch64.eap | grep manifest.json
tar -xzf OMNISIGHT_0_7_0_aarch64.eap manifest.json
cat manifest.json | grep settingPage

# Should show: "settingPage": "index.html"

# Check HTML files
tar -tzf OMNISIGHT_0_7_0_aarch64.eap | grep html

# Should show:
# ./html/index.html
# ./html/assets/...
```

**Fix**: Rebuild package with correct manifest:
```bash
./scripts/build-phase4-v070.sh
```

### Issue: Dashboard Shows 404 or Blank Page

**Symptoms**: "Open" button works, but page is blank or shows 404

**Causes**:
1. HTML files missing from package
2. Binary not starting correctly
3. Web root path incorrect

**Solutions**:
```bash
# Check if binary is running
ssh root@camera-ip
ps aux | grep omnisight

# Check binary can start manually
/usr/local/packages/omnisight/omnisight --port 8080

# Check web root exists
ls -la /usr/local/packages/omnisight/html/

# Should show: index.html and assets/
```

**Fix**: Verify HTML files in package:
```bash
tar -tzf OMNISIGHT_0_7_0_aarch64.eap | grep "\.html\|\.js\|\.css"
```

### Issue: API Endpoints Return 404

**Symptoms**: Dashboard loads, but shows "No data" or errors in console

**Causes**:
1. Binary not compiled with API handlers
2. Reverse proxy not configured
3. Port 8080 not accessible

**Solutions**:
```bash
# Test API directly (bypassing reverse proxy)
ssh root@camera-ip
curl http://localhost:8080/api/health

# If this works, reverse proxy issue
# Check Apache config

# Test from outside camera
curl -k https://camera-ip/local/omnisight/api/health

# If 404, check reverseProxy in manifest
```

**Fix**: Ensure reverseProxy configured in manifest:
```json
"reverseProxy": [{
  "apiPath": "api",
  "target": "http://localhost:8080",
  "access": "admin"
}]
```

### Issue: App Crashes or Won't Start

**Symptoms**: Status shows "Stopped", clicking Start does nothing

**Causes**:
1. Binary incompatible with camera architecture
2. Missing dependencies
3. Permission issues
4. Port 8080 already in use

**Solutions**:
```bash
# Check logs
ssh root@camera-ip
journalctl -u omnisight.service -n 50

# Check architecture
file /usr/local/packages/omnisight/omnisight
# Should show: ELF 64-bit LSB executable, ARM aarch64

# Check dependencies
ldd /usr/local/packages/omnisight/omnisight
# Should show: "statically linked" (no dependencies)

# Check port availability
netstat -tlnp | grep 8080
```

**Fix**: Rebuild with proper ARM compiler:
```bash
docker-compose run omnisight-dev ./scripts/build-phase4-v070.sh
```

### Issue: High Memory Usage

**Symptoms**: Memory usage exceeds 256MB, app killed by system

**Causes**:
1. Memory leaks in C code
2. Too many objects tracked
3. React assets too large

**Solutions**:
```bash
# Monitor memory over time
ssh root@camera-ip
watch -n 5 'ps aux | grep omnisight'

# Check resource limits in manifest
cat /usr/local/packages/omnisight/manifest.json | grep -A5 resources
```

**Fix**: Adjust memory limits in manifest:
```json
"resources": {
  "memory": {
    "ram": 256,      // Increase base
    "ramMax": 512    // Increase max
  }
}
```

### Issue: Slow Performance

**Symptoms**: API responses slow (>1 second), dashboard laggy

**Causes**:
1. Debug symbols not stripped
2. Non-optimized compilation
3. Too much logging
4. Camera CPU overloaded

**Solutions**:
```bash
# Check binary size
ssh root@camera-ip
ls -lh /usr/local/packages/omnisight/omnisight
# Should be <5MB if stripped

# Check CPU usage
top -b -n 1 | grep omnisight

# Disable verbose logging
# Edit config to set verbose_logging = false
```

**Fix**: Rebuild with optimizations:
```bash
# Ensure -O2 and strip flags in build script
aarch64-linux-gnu-gcc -O2 ... -o omnisight ...
aarch64-linux-gnu-strip omnisight
```

## Validation Tests

### Acceptance Criteria

All tests must pass for deployment approval:

#### Functional Tests
- [x] Package builds successfully
- [x] Package uploads to camera
- [x] App starts without errors
- [ ] "Open" button appears
- [ ] Dashboard loads in <3 seconds
- [ ] All 8 API endpoints respond
- [ ] Module statuses show "Active"
- [ ] Stats update in real-time

#### Performance Tests
- [ ] API response time: <100ms average
- [ ] Memory usage: <200MB stable
- [ ] CPU usage: <20% average
- [ ] No memory leaks over 1 hour
- [ ] Handles 10+ concurrent requests

#### Reliability Tests
- [ ] Runs continuously for 24 hours
- [ ] Survives camera reboot
- [ ] Restarts automatically on crash
- [ ] No errors in logs after 1 hour

## Next Steps After Successful Deployment

### Phase 5: Hardware Integration
1. **Replace Perception Stub**:
   - Integrate VDO API for real video capture
   - Process frames from camera sensor
   - Run actual object detection

2. **Replace Timeline Stub**:
   - Implement real trajectory prediction
   - Use Kalman filtering for motion tracking
   - Generate actual event predictions

3. **Enable Swarm Intelligence**:
   - Implement MQTT client
   - Connect to broker
   - Share tracking data between cameras

4. **Add Larod Integration**:
   - Use DLPU for ML inference
   - Accelerate perception with hardware
   - Achieve <20ms inference time

### Phase 6: Production Deployment
1. Test on multiple camera models
2. Load testing with high object counts
3. Network stress testing
4. Long-term stability validation
5. Security audit and hardening

## Support

### Log Collection
If issues persist, collect logs:
```bash
# App log
ssh root@camera-ip
journalctl -u omnisight.service > /tmp/omnisight-app.log

# System log
dmesg > /tmp/omnisight-system.log

# Apache log
cat /var/log/apache2/error.log > /tmp/apache-error.log

# Copy logs
scp root@camera-ip:/tmp/omnisight-*.log .
```

### Package Info
```bash
# Verify package contents
tar -tzf OMNISIGHT_0_7_0_aarch64.eap

# Check manifest
tar -xzf OMNISIGHT_0_7_0_aarch64.eap manifest.json
cat manifest.json | python3 -m json.tool
```

### Debugging Tools
```bash
# Test API locally on camera
ssh root@camera-ip
curl http://localhost:8080/api/health

# Check binary
file /usr/local/packages/omnisight/omnisight
ldd /usr/local/packages/omnisight/omnisight

# Monitor real-time
watch -n 1 'curl -s http://localhost:8080/api/status | python3 -m json.tool'
```

## Summary

This guide covers complete deployment and testing of OMNISIGHT v0.7.0 on P3285-LVE cameras. Follow the Quick Start for standard deployments, or use detailed steps for troubleshooting.

**Key Success Indicators**:
- ✅ Package uploads successfully
- ✅ "Open" button appears
- ✅ Dashboard loads and displays data
- ✅ API endpoints respond correctly
- ✅ System runs stably for 24+ hours

**Ready for Phase 5**: Once all validation tests pass, proceed with hardware integration for real video processing.

---

**Document Version**: 1.0
**Last Updated**: 2025-11-05
**Target Firmware**: AXIS OS 12.0+
