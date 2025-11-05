# OMNISIGHT Stub Package Deployment Guide

**Package**: `OMNISIGHT_Stub_0_3_0-stub_aarch64.eap` (23 KB)
**Purpose**: Test ACAP deployment workflow with stub implementation
**Target**: Axis P3285-LVE camera (or any ARTPEC-8/9 camera)

---

## What's in This Package

This is a **stub/demo** build for testing the deployment process. It contains:

- **Stub Binary** (`omnisight`): 53KB executable with simulated modules
  - Perception Engine (random walk object tracking simulation)
  - Timeline Threading (multi-future predictions)
  - Swarm Intelligence (mock network coordination)

- **Web Dashboard**: Simple HTML status page showing stub is running

- **No Hardware APIs**: This build does NOT use VDO/Larod/MQTT - it's for deployment testing only

---

## Deployment Methods

### Method 1: Web Interface (Recommended)

**Steps**:

1. Open your camera's web interface:
   ```
   https://<camera-ip>
   ```

2. Log in (default: root/password)

3. Navigate to: **Settings → Apps → Add**

4. Click **Browse** and select:
   ```
   OMNISIGHT_Stub_0_3_0-stub_aarch64.eap
   ```

5. Click **Install**

6. Wait for installation (5-10 seconds)

7. Click **Open** to view the web interface

**Expected URL**:
```
https://<camera-ip>/local/omnisight/
```

---

### Method 2: Command Line (cURL)

**From your local machine**:

```bash
curl -k --anyauth -u root:<password> \
  -F 'package=@OMNISIGHT_Stub_0_3_0-stub_aarch64.eap' \
  https://<camera-ip>/axis-cgi/applications/upload.cgi
```

**Example with real IP**:
```bash
curl -k --anyauth -u root:mypassword \
  -F 'package=@OMNISIGHT_Stub_0_3_0-stub_aarch64.eap' \
  https://192.168.1.100/axis-cgi/applications/upload.cgi
```

**Expected Response**:
```
OK
```

---

### Method 3: SSH Direct Installation

**Steps**:

1. Copy package to camera:
   ```bash
   scp OMNISIGHT_Stub_0_3_0-stub_aarch64.eap root@<camera-ip>:/tmp/
   ```

2. SSH into camera:
   ```bash
   ssh root@<camera-ip>
   ```

3. Install via ACAP API:
   ```bash
   cd /tmp
   /usr/bin/acap-install OMNISIGHT_Stub_0_3_0-stub_aarch64.eap
   ```

---

## Verification

### Check Installation Status

**Via Web Interface**:
1. Go to **Settings → Apps**
2. Look for "OMNISIGHT - Stub Demo" in the list
3. Status should be "Running" or "Stopped"

**Via SSH**:
```bash
ssh root@<camera-ip>
ls -la /usr/local/packages/omnisight/
```

**Expected output**:
```
drwxr-xr-x 3 root root 4096 Oct 30 15:37 .
drwxr-xr-x 5 root root 4096 Oct 30 15:37 ..
drwxr-xr-x 2 root root 4096 Oct 30 15:37 html
-rwxr-xr-x 1 root root 53704 Oct 30 15:37 omnisight
```

### Test Web Interface

**Browser**:
```
https://<camera-ip>/local/omnisight/
```

**Expected**:
- Beautiful gradient purple/blue page
- "OMNISIGHT - Stub Demo Build" title
- Status showing "Running (Stub Mode)"
- Three stub modules listed

### Check Application Logs

**Via SSH**:
```bash
ssh root@<camera-ip>
tail -f /var/log/messages | grep omnisight
```

**Expected**: (if stub binary runs on startup)
```
[OMNISIGHT] Stub modules initialized
[OMNISIGHT] Perception: Simulating object tracking
[OMNISIGHT] Timeline: Generating predictions
[OMNISIGHT] Swarm: Mock network active
```

**Note**: Current stub has `runMode: "never"` so it won't start automatically. Web interface still works.

---

## What This Tests

✅ **ACAP Package Format**: Verifies .eap structure is valid
✅ **Manifest Validation**: Confirms manifest.json is correct
✅ **File Deployment**: Tests copying files to `/usr/local/packages/omnisight/`
✅ **Web Interface**: Validates HTTP config and HTML serving
✅ **Camera Compatibility**: Confirms aarch64 architecture works

❌ **Does NOT Test**:
- Real video capture (VDO API)
- ML inference (Larod/DLPU)
- Object tracking (SORT algorithm)
- Multi-camera coordination (MQTT)

---

## Uninstallation

### Via Web Interface:
1. **Settings → Apps**
2. Find "OMNISIGHT - Stub Demo"
3. Click **Remove**

### Via SSH:
```bash
ssh root@<camera-ip>
/usr/bin/acap-uninstall omnisight
```

### Manual Cleanup (if needed):
```bash
ssh root@<camera-ip>
rm -rf /usr/local/packages/omnisight
```

---

## Troubleshooting

### Problem: "Installation failed"

**Possible Causes**:
1. Corrupted .eap file
2. Insufficient storage on camera
3. Incompatible firmware version

**Solutions**:
```bash
# Check firmware version
ssh root@<camera-ip>
cat /etc/fw_version
# Requires: AXIS OS 12.0+

# Check disk space
df -h /usr/local
# Requires: >50MB free
```

### Problem: "App installed but web interface shows 404"

**Cause**: Package extracted but HTTP config not applied

**Solution**:
1. Restart the app from web interface
2. Or restart camera:
   ```bash
   ssh root@<camera-ip>
   reboot
   ```

### Problem: Stub binary doesn't run

**Cause**: `runMode: "never"` in manifest (intended for testing)

**Solution**: This is expected - stub only tests deployment, not execution. Web interface should still work.

**To Enable Auto-Start** (not recommended for stub):
Edit manifest.json before building:
```json
"runMode": "respawn"
```

---

## Next Steps After Successful Stub Deployment

Once stub deployment is verified:

1. **Acquire ACAP Native SDK** with cross-compiler:
   ```bash
   docker pull axisecp/acap-native-sdk:1.15-aarch64
   ```

2. **Build Hardware Package**:
   ```bash
   docker-compose exec omnisight-dev bash
   cd /opt/app
   ./scripts/build-phase3-eap.sh
   ```

3. **Deploy Hardware Build** following same methods

4. **Test Real Features**:
   - Live video capture at 10-30 FPS
   - DLPU-accelerated inference (<20ms)
   - Object tracking with SORT algorithm
   - Timeline predictions with real events

---

## Package Details

**Manifest Configuration**:
```json
{
  "schemaVersion": "1.5",
  "acapPackageConf": {
    "setup": {
      "appName": "omnisight",
      "friendlyName": "OMNISIGHT - Stub Demo",
      "version": "0.3.0-stub",
      "runMode": "never"
    },
    "configuration": {
      "httpConfig": [
        {
          "access": "viewer",
          "name": "OMNISIGHT Stub",
          "path": "html/index.html",
          "type": "transferRx"
        }
      ]
    }
  }
}
```

**Contents**:
- `omnisight` (53KB) - Stub binary
- `html/index.html` (3.8KB) - Web dashboard
- `html/img/icon.png` - Placeholder icon
- `manifest.json` - ACAP manifest
- `LICENSE` - MIT-style license
- `package.conf` - Package metadata

**Total Size**: 23 KB (compressed)

---

## Support

**Documentation**:
- Full deployment guide: [PHASE3_BUILD_AND_DEPLOY.md](PHASE3_BUILD_AND_DEPLOY.md)
- Build status: [PHASE3_BUILD_STATUS.md](PHASE3_BUILD_STATUS.md)
- Phase 3 overview: [PHASE3_STATUS.md](PHASE3_STATUS.md)

**Camera Information**:
- Axis ACAP Documentation: https://developer.axis.com/acap
- ACAP Native SDK: https://github.com/AxisCommunications/acap-native-sdk

---

**Last Updated**: October 30, 2025
**Package Version**: 0.3.0-stub
**Build System**: Docker + GCC (direct compilation, no ACAP SDK needed)
