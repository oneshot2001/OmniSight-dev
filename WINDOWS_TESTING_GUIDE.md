# OMNISIGHT Testing Guide - Windows Laptop Workflow

**Target Hardware**: Axis M4228-LVE (ARTPEC-8)
**Test Platform**: Windows 10/11 Laptop
**Last Updated**: October 22, 2025

---

## Overview

This guide covers the complete testing workflow for OMNISIGHT using a Windows laptop to interact with the Axis M4228-LVE camera.

---

## Prerequisites

### Hardware Setup
- ✅ **Axis M4228-LVE camera** (or any ARTPEC-8/9 camera)
- ✅ **Windows 10/11 laptop**
- ✅ **Network connection** (Ethernet or Wi-Fi) connecting laptop to camera

### Software Requirements
- ✅ **Web browser**: Chrome, Edge, or Firefox (recommended: Chrome)
- ✅ **Camera credentials**: Admin username/password
- ✅ **ACAP package**: `OMNISIGHT_-_Precognitive_Security_0_2_0_aarch64.eap`

### Network Configuration
- ✅ Camera and laptop on same network
- ✅ Know camera IP address (e.g., `192.168.1.100`)
- ✅ Can ping camera: `ping 192.168.1.100`

---

## Phase 1: Initial Deployment (5-10 minutes)

### Step 1: Access Camera Web Interface

1. **Open browser** and navigate to camera:
   ```
   https://192.168.1.100
   ```
   (Replace with your camera's IP)

2. **Login** with admin credentials
   - Username: `root`
   - Password: (your camera password)

3. **Accept security warning** (self-signed certificate)

### Step 2: Upload ACAP Package

1. In camera web interface, go to:
   ```
   Settings → Apps → Add app
   ```

2. **Click "Browse"** and select:
   ```
   OMNISIGHT_-_Precognitive_Security_0_2_0_aarch64.eap
   ```

3. **Click "Install"**

4. **Wait for upload** (10-30 seconds depending on network)

### Step 3: Start Application

1. Find **"OMNISIGHT - Precognitive Security"** in app list

2. **Click "Start"**

3. **Wait for initialization** (30-60 seconds)
   - Status should change to "Running"

### Step 4: Verify Installation

1. **Access dashboard** in browser:
   ```
   https://192.168.1.100/local/omnisight/
   ```

2. **You should see**:
   - Status: Active
   - Mode: Demo/Stub
   - Version: 0.2.0
   - Three modules listed: Perception, Timeline, Swarm

3. **Test API endpoint**:
   ```
   https://192.168.1.100/local/omnisight/api/health
   ```

   Expected response:
   ```json
   {
     "status": "healthy",
     "version": "0.2.0",
     "timestamp": "2025-10-22T..."
   }
   ```

✅ **Phase 1 Complete** - OMNISIGHT is running!

---

## Phase 2: VDO Testing (30-60 minutes)

**Goal**: Verify real video capture and object detection

### What to Monitor

#### Camera View
1. **Point camera** at area with activity:
   - People walking
   - Vehicles moving
   - Varying lighting conditions

2. **Let run for different scenarios**:
   - **5 minutes**: Daytime, good lighting
   - **5 minutes**: Low light / shadows
   - **5 minutes**: Multiple objects (3+ people)
   - **5 minutes**: Static scene (no motion)
   - **10 minutes**: Day→night transition (if available)

#### Dashboard Monitoring

Open dashboard: `https://192.168.1.100/local/omnisight/`

**Watch these metrics**:

1. **Active Detections**:
   - Should show real-time object count
   - Updates every 1 second
   - Should match what you see in camera view

2. **System FPS**:
   - Target: 10 FPS (ARTPEC-8)
   - Should stay stable
   - Watch latency: <20ms

3. **Perception Engine**:
   - Status: "running"
   - Frames processed: increasing counter
   - Objects tracked: matches detections

### API Testing

**Test perception endpoints** from browser or curl:

```bash
# Get current detections
https://192.168.1.100/local/omnisight/api/perception/detections

# Expected response:
{
  "frame_id": 12345,
  "timestamp": "2025-10-22T16:00:00Z",
  "detections": [
    {
      "id": 1,
      "type": "person",
      "bbox": [100, 200, 50, 150],
      "confidence": 0.95,
      "track_id": "T001"
    }
  ]
}
```

### What to Record

Create a test log documenting:

**Scenario 1: Daytime, Good Lighting**
- Time: 10:00 AM
- Lighting: Bright, outdoor
- Activity: 2 people walking
- FPS: 10.2
- Detections: Consistent, 95%+ confidence
- Issues: None

**Scenario 2: Low Light**
- Time: 7:00 PM
- Lighting: Dusk, shadows
- Activity: 1 person, 1 vehicle
- FPS: 9.8
- Detections: Person 85%, vehicle 90%
- Issues: Slight confidence drop in shadows

(Continue for each scenario...)

✅ **Phase 2 Complete** - VDO integration verified!

---

## Phase 3: Larod/DLPU Testing (1-2 hours)

**Goal**: Verify DLPU inference performance

### Monitor DLPU Usage

#### Via SSH (if available):
```bash
ssh root@192.168.1.100
top -bn1 | grep larod
```

#### Via API:
```bash
# Check system stats
https://192.168.1.100/local/omnisight/api/stats
```

Expected response includes:
```json
{
  "cpu": {
    "usage_percent": 35,
    "cores": 4
  },
  "modules": {
    "perception": {
      "status": "running",
      "fps": 10.2,
      "frames_processed": 36720
    }
  }
}
```

### Performance Targets (ARTPEC-8)

| Metric | Target | Acceptable | Poor |
|--------|--------|------------|------|
| Inference Latency | <15ms | <20ms | >20ms |
| Detection FPS | 10 Hz | 8-10 Hz | <8 Hz |
| CPU Usage | <35% | <45% | >45% |
| Memory Usage | <256 MB | <384 MB | >384 MB |

### Load Testing

**Light Load** (5 minutes):
- 1 object in frame
- Expected FPS: 10 Hz
- Expected latency: 10-15ms

**Medium Load** (10 minutes):
- 3-5 objects in frame
- Expected FPS: 9-10 Hz
- Expected latency: 15-18ms

**Heavy Load** (10 minutes):
- 10+ objects in frame
- Expected FPS: 8-9 Hz
- Expected latency: 18-20ms

### What to Record

**DLPU Performance Log**:

```
Test: Light Load (1 object)
Duration: 5 minutes
FPS: 10.2 Hz (stable)
Latency: 12ms avg, 15ms max
CPU: 30%
Memory: 180 MB
Result: ✅ PASS

Test: Medium Load (5 objects)
Duration: 10 minutes
FPS: 9.8 Hz (stable)
Latency: 17ms avg, 19ms max
CPU: 38%
Memory: 220 MB
Result: ✅ PASS

Test: Heavy Load (12 objects)
Duration: 10 minutes
FPS: 8.5 Hz (fluctuating)
Latency: 21ms avg, 25ms max
CPU: 45%
Memory: 280 MB
Result: ⚠️ MARGINAL (latency above target)
```

✅ **Phase 3 Complete** - DLPU inference validated!

---

## Phase 4: IPC/API Testing (30 minutes)

**Goal**: Verify data flow C core → IPC → Flask API

### Test All API Endpoints

From Windows browser or PowerShell:

```powershell
# PowerShell commands for testing

# 1. Health check
Invoke-RestMethod https://192.168.1.100/local/omnisight/api/health

# 2. Perception status
Invoke-RestMethod https://192.168.1.100/local/omnisight/api/perception/status

# 3. Detections
Invoke-RestMethod https://192.168.1.100/local/omnisight/api/perception/detections

# 4. Timeline predictions
Invoke-RestMethod https://192.168.1.100/local/omnisight/api/timeline/predictions

# 5. Swarm network
Invoke-RestMethod https://192.168.1.100/local/omnisight/api/swarm/network

# 6. System stats
Invoke-RestMethod https://192.168.1.100/local/omnisight/api/stats

# 7. Configuration
Invoke-RestMethod https://192.168.1.100/local/omnisight/api/config
```

### Verify Real-Time Updates

1. **Open dashboard** in browser:
   ```
   https://192.168.1.100/local/omnisight/
   ```

2. **Watch for live updates** (every 1 second):
   - Detection count changes
   - FPS metrics update
   - Timeline predictions appear
   - Performance chart updates

3. **Create activity in camera view**:
   - Walk in front of camera
   - Watch detection cards appear in dashboard
   - Verify bbox coordinates match object position

### API Response Time Testing

Test API performance (should be <50ms):

```powershell
# Measure API response time
Measure-Command {
  Invoke-RestMethod https://192.168.1.100/local/omnisight/api/perception/detections
}

# Expected: 10-30 milliseconds
```

✅ **Phase 4 Complete** - IPC and API validated!

---

## Phase 5: Timeline & Swarm Testing (Optional, if multiple cameras available)

### Timeline Predictions

1. **Create predictable patterns**:
   - Walk same path repeatedly
   - Stand in one location (loitering)
   - Approach restricted area

2. **Monitor predictions**:
   ```
   https://192.168.1.100/local/omnisight/api/timeline/predictions
   ```

3. **Check prediction accuracy**:
   - Horizon: 300 seconds (5 minutes)
   - Probability: >0.5 for valid predictions
   - Events: Should predict LOITERING, TRESPASSING, etc.

### Swarm Intelligence (requires 2+ cameras)

If you have multiple M4228-LVE cameras:

1. **Install OMNISIGHT on all cameras**

2. **Configure MQTT broker** (or use public broker for testing)

3. **Monitor swarm network**:
   ```
   https://192.168.1.100/local/omnisight/api/swarm/network
   ```

4. **Verify track sharing**:
   - Walk between camera coverage areas
   - Check if track ID persists across cameras
   - Verify <100ms propagation latency

---

## Logging and Diagnostics

### View OMNISIGHT Logs

**Via SSH**:
```bash
ssh root@192.168.1.100
journalctl -u omnisight -f
```

**Via Camera Web Interface**:
```
Settings → System → Logs → Application logs
```

### Common Log Messages

**Successful startup**:
```
[INFO] OMNISIGHT: Initializing Precognitive Security System
[INFO] VDO: Video capture started (1920x1080 @ 10 FPS)
[INFO] Larod: DLPU model loaded successfully
[INFO] IPC: Server initialized successfully
[INFO] Flask: API server running on port 8080
[INFO] OMNISIGHT: All modules operational
```

**Error indicators**:
```
[ERROR] VDO: Failed to open video stream
[ERROR] Larod: DLPU not available
[ERROR] IPC: Shared memory creation failed
[ERROR] Flask: Port 8080 already in use
```

---

## Performance Benchmarks

### Expected Performance (ARTPEC-8)

**Baseline (No Objects)**:
- FPS: 10 Hz
- CPU: 25%
- Memory: 150 MB
- Latency: <10ms

**Normal Load (3-5 Objects)**:
- FPS: 9-10 Hz
- CPU: 35%
- Memory: 200 MB
- Latency: 15-18ms

**Peak Load (10+ Objects)**:
- FPS: 8-9 Hz
- CPU: 40-45%
- Memory: 250 MB
- Latency: 18-22ms

---

## Recommended Testing Schedule

**Day 1 (2-3 hours)**:
- Phase 1: Deployment (30 min)
- Phase 2: VDO testing (1 hour)
- Phase 3: DLPU testing (1 hour)

**Day 2 (2 hours)**:
- Phase 4: API testing (30 min)
- Phase 5: Timeline testing (1 hour)
- Documentation (30 min)

**Day 3+ (Optional)**:
- Long-term stability testing (24+ hours)
- Swarm testing with multiple cameras
- Performance optimization

---

## Troubleshooting

### Dashboard Not Loading

**Check**:
1. ACAP status in camera web interface
2. Restart application if needed
3. Clear browser cache
4. Try different browser

**Solution**:
```bash
# Via SSH
ssh root@192.168.1.100
systemctl restart omnisight
```

### Low FPS Performance

**Check**:
1. CPU usage: `top`
2. DLPU status: `larod-client list-models`
3. Video resolution (reduce if needed)

**Solution**:
- Reduce resolution to 1280x720
- Check for other running ACAPs
- Restart camera if memory fragmented

### API Returns 404

**Check**:
1. Reverse proxy configuration in manifest
2. Flask server status
3. Firewall rules

**Solution**:
```bash
# Test direct Flask access
curl http://192.168.1.100:8080/api/health
```

---

## Test Results Template

Use this template to document your testing:

```markdown
# OMNISIGHT Test Results

**Camera**: Axis M4228-LVE
**Firmware**: AXIS OS 12.6.97
**OMNISIGHT Version**: 0.2.0
**Test Date**: 2025-10-22
**Tester**: [Your Name]

## Phase 1: Deployment
- Upload time: 15 seconds
- Start time: 45 seconds
- Dashboard accessible: ✅ YES
- API responsive: ✅ YES

## Phase 2: VDO Testing
- Average FPS: 10.1 Hz
- Detection accuracy: 92%
- Issues: None

## Phase 3: DLPU Testing
- Inference latency: 16ms avg
- CPU usage: 35%
- Memory usage: 210 MB
- Issues: None

## Phase 4: API Testing
- All endpoints: ✅ PASS
- Average response time: 12ms
- Real-time updates: ✅ WORKING

## Phase 5: Timeline/Swarm
- [Not tested / Results here]

## Overall Result
✅ PASS - Ready for production
```

---

## Next Steps After Testing

1. **Document findings** using template above
2. **Report issues** via GitHub if any problems
3. **Optimize** based on performance data
4. **Deploy to production** if all tests pass
5. **Monitor** long-term stability

---

**Testing Platform**: Windows 10/11 Laptop
**Target Hardware**: Axis M4228-LVE (ARTPEC-8)
**Estimated Time**: 3-6 hours total testing
**Success Criteria**: All 4 phases pass validation
