# OMNISIGHT v0.2.0 Deployment Instructions

**Package**: `OMNISIGHT_-_Precognitive_Security_0.2.0_aarch64.eap`
**Size**: 16 KB
**Target**: Axis M4228-LVE (ARTPEC-8) or any ARTPEC-8/9 camera
**Date**: October 22, 2025

---

## 📦 What's in This Package

### Phase 2 Components
- ✅ **Flask REST API** - 10 endpoints with stub data responses
- ✅ **Reverse Proxy** - Apache integration via manifest schema 1.8.0
- ✅ **HTML Dashboard** - Web interface with system status
- ✅ **Python Dependencies** - Flask 3.0.0, Flask-CORS 4.0.0

### API Endpoints Included
1. `GET /api/health` - Health check
2. `GET /api/perception/status` - Perception module status
3. `GET /api/perception/detections` - Current detections (stub)
4. `GET /api/timeline/predictions` - Timeline predictions (stub)
5. `GET /api/timeline/history` - Historical events (stub)
6. `GET /api/swarm/network` - Swarm network status (stub)
7. `GET /api/swarm/cameras` - Camera list (stub)
8. `GET /api/config` - System configuration
9. `POST /api/config` - Update configuration
10. `GET /api/stats` - System statistics

### What's NOT Included (Coming in Phase 2.2+)
- ❌ VDO API integration (real video)
- ❌ Larod API integration (DLPU inference)
- ❌ IPC communication (C core ↔ Flask)
- ❌ Real object detection
- ❌ MQTT swarm communication

**This is a baseline test build** - All endpoints return stub data for API validation.

---

## 🚀 Deployment Steps

### Method 1: Web Interface (Recommended)

**Step 1: Access Camera**
1. Open web browser (Chrome, Edge, or Firefox)
2. Navigate to: `https://<your-camera-ip>`
3. Login with admin credentials
   - Username: `root`
   - Password: (your camera password)
4. Accept security warning (self-signed certificate)

**Step 2: Upload ACAP**
1. Go to: **Settings** → **Apps** → **Add app**
2. Click **"Browse"** button
3. Select: `OMNISIGHT_-_Precognitive_Security_0.2.0_aarch64.eap`
4. Click **"Install"**
5. Wait for upload (10-30 seconds)

**Step 3: Start Application**
1. Find **"OMNISIGHT - Precognitive Security"** in Apps list
2. Click **"Start"** button
3. Wait for status to change to **"Running"** (30-60 seconds)

**Step 4: Verify Installation**
1. Access dashboard: `https://<camera-ip>/local/omnisight/`
2. You should see:
   - Status: Active
   - Mode: Phase 2
   - Version: 0.2.0
   - 10 API endpoints listed

---

### Method 2: Command Line (Advanced)

**Prerequisites**:
- `curl` installed
- SSH access to camera (optional)

**Upload via curl**:
```bash
curl -k -u root:password \
  -F "file=@OMNISIGHT_-_Precognitive_Security_0.2.0_aarch64.eap" \
  https://<camera-ip>/axis-cgi/applications/upload.cgi
```

**Start via API** (after upload):
```bash
curl -k -u root:password \
  "https://<camera-ip>/axis-cgi/applications/control.cgi?action=start&package=omnisight"
```

---

## ✅ Verification Checklist

After deployment, verify these 5 items:

### 1. Application Status
**Check**: Camera web interface → Apps → OMNISIGHT status
**Expected**: Status shows "Running"

### 2. Dashboard Accessible
**Check**: `https://<camera-ip>/local/omnisight/`
**Expected**: HTML dashboard loads with:
- 🔮 OMNISIGHT header
- System Status showing "Active"
- 10 API endpoints listed
- Test buttons functional

### 3. Health Endpoint
**Check**: `https://<camera-ip>/local/omnisight/api/health`
**Expected** JSON response:
```json
{
  "status": "healthy",
  "version": "0.2.0",
  "timestamp": "2025-10-22T..."
}
```

### 4. Stats Endpoint
**Check**: `https://<camera-ip>/local/omnisight/api/stats`
**Expected**: JSON with system statistics:
```json
{
  "uptime_seconds": 86400,
  "memory": { "used_mb": 128, ... },
  "modules": {
    "perception": { "status": "running", "fps": 10.2, ... },
    ...
  }
}
```

### 5. Application Logs
**Check via SSH**:
```bash
ssh root@<camera-ip>
journalctl -u omnisight -n 50
```

**Expected** log messages:
```
[INFO] OMNISIGHT: Starting Flask API Server (Phase 2)...
[INFO] OMNISIGHT: Installing Flask dependencies...
[INFO] OMNISIGHT: Dependencies installed successfully
[INFO] OMNISIGHT: Flask API server starting on port 8080...
```

---

## 🧪 Testing Scenarios

### Scenario 1: Baseline Stability (5-10 minutes)

**Goal**: Verify ACAP starts and stays running

**Steps**:
1. Deploy ACAP as described above
2. Wait 10 minutes
3. Refresh dashboard - should still be accessible
4. Check logs for errors

**Success Criteria**:
- ✅ Dashboard remains accessible
- ✅ All API endpoints respond
- ✅ No ERROR messages in logs
- ✅ Application status stays "Running"

### Scenario 2: API Response Validation (10 minutes)

**Goal**: Test all 10 API endpoints

**Use PowerShell** (Windows):
```powershell
# Test all endpoints
$baseUrl = "https://<camera-ip>/local/omnisight/api"

Invoke-RestMethod "$baseUrl/health"
Invoke-RestMethod "$baseUrl/perception/status"
Invoke-RestMethod "$baseUrl/perception/detections"
Invoke-RestMethod "$baseUrl/timeline/predictions"
Invoke-RestMethod "$baseUrl/timeline/history"
Invoke-RestMethod "$baseUrl/swarm/network"
Invoke-RestMethod "$baseUrl/swarm/cameras"
Invoke-RestMethod "$baseUrl/config"
Invoke-RestMethod "$baseUrl/stats"
```

**Success Criteria**:
- ✅ All 10 endpoints return JSON (no 404/500 errors)
- ✅ Response time <100ms for each endpoint
- ✅ Data structure matches API documentation

### Scenario 3: Reverse Proxy Validation (5 minutes)

**Goal**: Verify Apache reverse proxy is working

**Test direct Flask access** (should fail):
```bash
curl http://<camera-ip>:8080/api/health
# Expected: Connection refused or timeout
```

**Test reverse proxy access** (should work):
```bash
curl https://<camera-ip>/local/omnisight/api/health
# Expected: {"status": "healthy", ...}
```

**Success Criteria**:
- ❌ Direct port 8080 access blocked (secure)
- ✅ Reverse proxy /local/omnisight/* works
- ✅ Apache handles TLS encryption

### Scenario 4: Log Collection (After testing)

**Goal**: Collect baseline performance logs

**Steps**:
1. SSH to camera: `ssh root@<camera-ip>`
2. Run collection script:
   ```bash
   # Create collection script (if not already created)
   curl -o /tmp/collect_logs.sh \
     https://raw.githubusercontent.com/oneshot2001/OmniSight-dev/main/scripts/collect_logs.sh
   chmod +x /tmp/collect_logs.sh

   # Run collection
   /tmp/collect_logs.sh
   ```
3. Download logs:
   ```powershell
   scp root@<camera-ip>:/tmp/omnisight_logs_*.tar.gz .
   ```

**What to send for analysis**:
- Log tarball (omnisight_logs_*.tar.gz)
- Screenshots of dashboard
- Test results summary

---

## 🔧 Troubleshooting

### Issue: ACAP Won't Start

**Symptoms**: Status shows "Stopped" or "Failed"

**Check**:
```bash
ssh root@<camera-ip>
journalctl -u omnisight -n 100
```

**Common Causes**:
1. **Python dependencies failed to install**
   - Solution: Check internet connectivity on camera
   - Workaround: Pre-install Flask manually via SSH

2. **Port 8080 already in use**
   - Check: `lsof -i :8080`
   - Solution: Stop conflicting app or change OMNISIGHT port

3. **Insufficient memory**
   - Check: `free -m`
   - Solution: Stop other ACAPs to free memory

### Issue: Dashboard Shows 404

**Symptoms**: `https://<camera-ip>/local/omnisight/` not found

**Check**:
1. **ACAP running**?
   ```bash
   systemctl status omnisight
   ```

2. **Reverse proxy configured**?
   ```bash
   cat /usr/local/packages/omnisight/manifest.json | grep reverseProxy
   ```

3. **Flask server started**?
   ```bash
   lsof -i :8080
   ```

**Solution**:
- Restart ACAP via camera web interface
- Check logs for Flask startup errors

### Issue: API Returns Stub Data Forever

**Expected Behavior**: This is NORMAL for v0.2.0!

**Explanation**: This Phase 2 build contains only stub data responses. Real data requires:
- Phase 2.2: VDO integration (real video)
- Phase 2.3: Larod integration (DLPU inference)
- Phase 2.4: IPC implementation (C ↔ Python)

**When will I get real data?**:
- After we implement VDO + Larod + IPC based on your test logs
- Estimated: v0.3.0 or v0.4.0

### Issue: Can't Download Logs

**PowerShell SCP fails**:
```powershell
# Install OpenSSH client (Windows 10/11)
Add-WindowsCapability -Online -Name OpenSSH.Client

# Or use WinSCP GUI
# Download from: https://winscp.net/
```

**PuTTY/SSH connection refused**:
- Enable SSH on camera: Settings → System → Plain Config → SSH → Enabled
- Check firewall allows port 22

---

## 📊 Expected Performance (v0.2.0 Baseline)

Since this build uses stub data, performance should be excellent:

| Metric | Expected Value | Notes |
|--------|----------------|-------|
| **API Response Time** | <20ms | Stub data is instant |
| **CPU Usage** | <10% | No video processing |
| **Memory Usage** | 50-80 MB | Flask + Python overhead |
| **Startup Time** | 30-60 seconds | First run installs deps |
| **Stability** | 100% uptime | No complex processing |

**If you see different values**, it indicates a problem - send logs for analysis!

---

## 📝 What to Document

After running v0.2.0 for 30-60 minutes, please document:

### Test Results Template

```markdown
# OMNISIGHT v0.2.0 Test Results

**Camera**: Axis M4228-LVE
**Firmware**: AXIS OS 12.6.97
**Test Date**: 2025-10-22
**Tester**: [Your Name]
**Test Duration**: 60 minutes

## Deployment
- Upload time: 15 seconds
- Installation time: 45 seconds
- First startup time: 60 seconds (dependencies installed)
- Dashboard accessible: ✅ YES
- API responsive: ✅ YES

## API Testing
- Health endpoint: ✅ PASS (8ms response)
- Perception status: ✅ PASS (10ms response)
- Perception detections: ✅ PASS (12ms response)
- Timeline predictions: ✅ PASS (15ms response)
- Timeline history: ✅ PASS (11ms response)
- Swarm network: ✅ PASS (9ms response)
- Swarm cameras: ✅ PASS (10ms response)
- Config GET: ✅ PASS (7ms response)
- Config POST: ✅ PASS (18ms response)
- Stats: ✅ PASS (13ms response)

**Average response time**: 11.3ms ✅ (target: <100ms)

## System Performance
- CPU usage: 8% ✅ (target: <40%)
- Memory usage: 65 MB ✅ (target: <256 MB)
- Uptime: 60 minutes ✅ (no crashes)
- Application status: Running ✅

## Issues Encountered
[None / List any issues]

## Logs Collected
- File: omnisight_logs_20251022_1530.tar.gz
- Size: 45 KB
- Attached: ✅ YES

## Overall Result
✅ PASS - Ready for Phase 2.2+ (VDO/Larod/IPC implementation)

## Notes
[Any additional observations]
```

---

## 🎯 Success Criteria

**v0.2.0 baseline testing is successful if**:

1. ✅ ACAP deploys and starts without errors
2. ✅ Dashboard accessible via browser
3. ✅ All 10 API endpoints respond with JSON
4. ✅ Average API response time <100ms
5. ✅ CPU usage <15%
6. ✅ Memory usage <100 MB
7. ✅ Runs stable for 60+ minutes
8. ✅ Logs collected successfully
9. ✅ No ERROR messages in logs
10. ✅ Reverse proxy routing works

**If all 10 criteria pass**, we can proceed with confidence to Phase 2.2+ (VDO/Larod/IPC implementation)!

---

## 📬 Sending Results

After testing, please send:

### Option 1: GitHub Issue
1. Create issue: https://github.com/oneshot2001/OmniSight-dev/issues
2. Title: "v0.2.0 Test Results - M4228-LVE"
3. Paste test results template (filled in)
4. Attach log tarball

### Option 2: Direct File Sharing
1. Upload logs to Google Drive / Dropbox
2. Share link in conversation
3. Include test results summary

### What I'll Do Next
1. Analyze your logs for any issues
2. Validate baseline performance
3. Implement VDO API based on your hardware
4. Implement Larod API for DLPU
5. Implement IPC for C ↔ Python communication
6. Build v0.3.0+ with real integrations
7. Send you improved package for next test iteration

---

**Ready to deploy?** Follow the deployment steps above and let me know how it goes! 🚀
