# P3265-LVE Deployment Fix

**Date**: November 3, 2025
**Camera Model**: Axis P3265-LVE
**Issue**: Wrong package deployed (v0.4.1 instead of v0.4.2)

## Problem Identified

The system log shows you deployed **v0.4.1** (Python-dependent) instead of **v0.4.2** (Native):

```
2025-10-31T15:46:09.091 [ INFO ] upload.cgi[57613]: Uploaded /tmp/OMNISIGHT_-_Precognitive_Security_041_aarch64.eap
```

And then it failed with the same Python error:
```
2025-10-31T15:47:20.215 [ INFO ] omnisight[58577]: /usr/local/packages/omnisight/omnisight: exec: line 34: python3: not found
```

## Root Cause

**P3265-LVE does NOT have Python 3 installed** (same as P3285-LVE).

Both ARTPEC-9 cameras lack Python 3:
- P3285-LVE (ARTPEC-9) - No Python 3 ❌
- P3265-LVE (ARTPEC-9) - No Python 3 ❌

## Solution

Deploy the correct package: **v0.4.2 (Phase 4 V3 Native)**

### Package Location

```bash
packages/current/OMNISIGHT_-_Precognitive_Security_042_aarch64.eap
```

### Deployment Steps

#### Method 1: Via Web Interface (Recommended)

1. **Uninstall v0.4.1**:
   - Go to camera Settings → Apps
   - Find "OMNISIGHT - Phase 4 V3 Native"
   - Click the three dots (⋮) → Remove
   - Confirm removal

2. **Install v0.4.2**:
   - Click "+ Add app"
   - Click "Browse" and select:
     ```
     packages/current/OMNISIGHT_-_Precognitive_Security_042_aarch64.eap
     ```
   - Click "Install"
   - Wait for installation to complete

3. **Start the Application**:
   - Toggle should appear next to OMNISIGHT
   - Click toggle to ON
   - **"Open" button should now appear** ✅

4. **Access Web Interface**:
   - Click the yellow "Open" button
   - Dashboard should load showing:
     - Status: Active
     - Mode: Stub/Demo
     - Version: 0.4.2
     - Server: Native (No Python dependency)

#### Method 2: Via CLI (Alternative)

```bash
# SSH into camera
ssh root@<camera-ip>

# Uninstall old version
acapctl uninstall omnisight

# Upload new package (from your local machine)
scp packages/current/OMNISIGHT_-_Precognitive_Security_042_aarch64.eap root@<camera-ip>:/tmp/

# Install via SSH (on camera)
acapctl install /tmp/OMNISIGHT_-_Precognitive_Security_042_aarch64.eap

# Start application
acapctl start omnisight

# Check status
acapctl status omnisight
```

## What v0.4.2 Does Differently

**v0.4.1 (OLD - FAILED)**:
```bash
#!/bin/sh
# Phase 4 Flask server
python3 server.py  # ← FAILS: python3 not found
```

**v0.4.2 (NEW - WORKS)**:
```bash
#!/bin/sh
# Phase 4 V3 Native

# Try Python 3 if available
if command -v python3 >/dev/null 2>&1; then
  exec python3 -m http.server 8080
fi

# Try Python 2 if available
if command -v python >/dev/null 2>&1; then
  exec python -m SimpleHTTPServer 8080
fi

# Fallback: Keep running (Apache serves static files)
while true; do sleep 3600; done
```

The v0.4.2 wrapper script:
1. **Tries Python 3** (if available)
2. **Falls back to Python 2** (if available)
3. **Final fallback**: Just keeps running, letting Apache serve static files

This means v0.4.2 works on **ALL cameras**, regardless of Python availability.

## Expected Result After Deploying v0.4.2

✅ **Installation succeeds**
✅ **Application starts** (toggle shows ON)
✅ **"Open" button appears** (settingPage configured)
✅ **Web interface loads** (static HTML served)
✅ **No Python errors** in system log

## Camera Compatibility Summary

| Camera Model | ARTPEC | Python 3 | Works with v0.4.1 | Works with v0.4.2 |
|--------------|--------|----------|-------------------|-------------------|
| **M4228-LVE** | ARTPEC-8 | ✅ Yes | ✅ Yes | ✅ Yes |
| **P3285-LVE** | ARTPEC-9 | ❌ No | ❌ No | ✅ Yes |
| **P3265-LVE** | ARTPEC-9 | ❌ No | ❌ No | ✅ Yes |

## Verification Steps

After deploying v0.4.2, verify success:

### 1. Check System Log
No errors like:
```
python3: not found  # ← Should NOT see this
Failed at step EXEC  # ← Should NOT see this
```

### 2. Check Application Status
```bash
acapctl status omnisight
# Should show: Running
```

### 3. Check Web Interface
- Open button appears: ✅
- Dashboard loads: ✅
- Shows "Version: 0.4.2": ✅
- Shows "Server: Native": ✅
- Shows warning about Python resolved: ✅

## Troubleshooting

### If Still No "Open" Button After v0.4.2

1. **Check manifest.json has settingPage**:
   ```bash
   ssh root@<camera-ip>
   cat /usr/local/packages/omnisight/manifest.json | grep settingPage
   # Should show: "settingPage": "index.html"
   ```

2. **Check index.html exists**:
   ```bash
   ls -la /usr/local/packages/omnisight/html/index.html
   # Should exist
   ```

3. **Check service status**:
   ```bash
   systemctl status sdkomnisight.service
   # Should be: active (running)
   ```

4. **Check system log**:
   ```bash
   tail -100 /var/log/messages | grep omnisight
   # Should NOT see "python3: not found"
   ```

## Related Documentation

- **Root Cause Analysis**: [docs/troubleshooting/deployment-fixes/PHASE4_PYTHON_DEPENDENCY_ISSUE.md](docs/troubleshooting/deployment-fixes/PHASE4_PYTHON_DEPENDENCY_ISSUE.md)
- **Quick Troubleshooting**: [docs/troubleshooting/QUICK_TROUBLESHOOTING.md](docs/troubleshooting/QUICK_TROUBLESHOOTING.md)
- **ACAP Packaging Guide**: [docs/acap/ACAP_PACKAGING.md](docs/acap/ACAP_PACKAGING.md)

## Next Steps

1. **Deploy v0.4.2** to P3265-LVE using steps above
2. **Verify "Open" button appears**
3. **Test web interface** functionality
4. **Update P3285-LVE** to v0.4.2 as well (if not already done)
5. **Document P3265-LVE** in camera compatibility matrix

---

**Key Takeaway**: Both P3285-LVE and P3265-LVE (ARTPEC-9 cameras) lack Python 3. Always use v0.4.2 (Native) for ARTPEC-9 cameras.
