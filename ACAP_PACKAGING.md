# ACAP Packaging Guide for Axis Cameras

This document explains how to build and deploy OMNISIGHT as an ACAP (Axis Camera Application Platform) package for Axis cameras.

## Prerequisites

- Docker and Docker Compose installed
- Axis camera with ARTPEC-8 or ARTPEC-9 chipset (e.g., M4228-LVE)
- Camera firmware: AXIS OS 12 or later
- Access to camera web interface (admin credentials)

## Overview

The OMNISIGHT ACAP package is built inside a Docker container that includes the Axis ACAP SDK. The build process creates a `.eap` file that can be uploaded directly to Axis cameras.

## Build Process

### Step 1: Start the Docker Environment

```bash
cd "/path/to/OmniSight-dev"
docker-compose up -d omnisight-dev
```

### Step 2: Build the ACAP Package

```bash
docker-compose exec omnisight-dev bash -c "cd /opt/app && ./scripts/build-eap.sh"
```

This script:
1. Creates a Python-based web server (lightweight HTTP server)
2. Packages HTML interface files
3. Generates manifest.json with ACAP configuration
4. Creates LICENSE file
5. Builds the .eap package using ACAP SDK tools

### Step 3: Copy Package to Local Machine

```bash
docker cp omnisight-dev:/opt/app/output/OMNISIGHT_0_1_8_aarch64.eap ./
```

The package will be copied to your project root directory.

## Package Structure

The .eap file contains:
- `omnisight` - Python HTTP server (serves web interface)
- `manifest.json` - ACAP application metadata and configuration
- `package.conf` - Generated package configuration
- `LICENSE` - Application license
- `html/` - Web interface files
  - `index.html` - Main interface page
  - `img/` - Images and icons

## Deployment to Camera

### Method 1: Web Interface (Recommended)

1. Open camera web interface: `https://your-camera-ip/`
2. Log in with admin credentials
3. Navigate to: **Settings → Apps**
4. Click **Add** or **Upload**
5. Select the `.eap` file (e.g., `OMNISIGHT_0_1_8_aarch64.eap`)
6. Click **Install**
7. Wait for installation to complete
8. The app should start automatically
9. Click **Open** to view the web interface

### Method 2: Command Line (Quick)

```bash
curl -k -u root:password \
  -F 'package=@OMNISIGHT_0_1_8_aarch64.eap' \
  https://camera-ip/axis-cgi/applications/upload.cgi
```

### Method 3: SSH Upload (Advanced)

```bash
# Upload package
scp OMNISIGHT_0_1_8_aarch64.eap root@camera-ip:/tmp/

# SSH into camera
ssh root@camera-ip

# Install package
cd /tmp
packagemanager install OMNISIGHT_0_1_8_aarch64.eap

# Start application
packagemanager start omnisight
```

## Manifest Configuration

The `manifest.json` file defines the ACAP application:

```json
{
  "schemaVersion": "1.5",
  "acapPackageConf": {
    "setup": {
      "appName": "omnisight",
      "appId": 1001,
      "version": "0.1.8",
      "architecture": "aarch64",
      "runMode": "respawn",
      "friendlyName": "OMNISIGHT"
    },
    "configuration": {
      "settingPage": "index.html"
    },
    "resources": {
      "memory": {
        "ram": 128,
        "ramMax": 256
      }
    }
  }
}
```

### Key Configuration Fields

- **appId**: Must be numeric integer (not a string like "com.example.app")
- **runMode**:
  - `"respawn"` - Application runs continuously, restarts on failure
  - `"never"` - Application doesn't run (static files only - NOT SUPPORTED for web interfaces)
- **architecture**: `"aarch64"` for ARTPEC-8/9 cameras
- **settingPage**: Path to main web interface file

## Critical Lessons Learned

### Issue 1: Invalid Manifest - appId Must Be Numeric

**Error**: `Failed to convert the string to an integer for 'appId'`

**Cause**: Using string format like `"com.omnisight.precognitive"`

**Solution**: Use numeric ID like `1001`

```json
// ❌ WRONG
"appId": "com.omnisight.precognitive"

// ✅ CORRECT
"appId": 1001
```

### Issue 2: Exec Format Error

**Error**: `Failed to execute /usr/local/packages/omnisight/omnisight: Exec format error`

**Cause**: Including x86_64 binary instead of ARM binary, or non-executable script

**Solution**: Use proper shell script or Python script with shebang:

```python
#!/usr/bin/env python3
import http.server
# ... server code
```

### Issue 3: Web Interface Not Loading with runMode: "never"

**Error**: "Couldn't find page" when clicking Open

**Cause**: Axis cameras require a running process to serve web pages. Static-only ACAPs (`runMode: "never"`) cannot serve HTTP content.

**Solution**: Include a lightweight web server (Python HTTP server) with `runMode: "respawn"`

### Issue 4: Extra Manifest Fields Cause Validation Errors

**Error**: `Additional properties are not allowed ('acapMetadata', 'capabilities' were unexpected)`

**Cause**: Including documentation fields not in ACAP schema

**Solution**: Use only official ACAP schema fields, or disable validation during build:

```bash
acap-build --disable-manifest-validation
```

## Web Server Implementation

The OMNISIGHT ACAP uses a minimal Python HTTP server:

```python
#!/usr/bin/env python3
import http.server
import socketserver
import os

os.chdir('/usr/local/packages/omnisight/html')

PORT = 8080
Handler = http.server.SimpleHTTPRequestHandler

with socketserver.TCPServer(("", PORT), Handler) as httpd:
    print(f"OMNISIGHT web server running on port {PORT}")
    httpd.serve_forever()
```

This approach:
- ✅ Minimal resource usage (~2-3 MB package size)
- ✅ No external dependencies (Python included in Axis OS)
- ✅ Automatic restart on failure (`runMode: "respawn"`)
- ✅ Serves static HTML, CSS, JavaScript files

## Verifying Installation

### Check Application Status

Via camera web interface:
1. Go to **Settings → Apps**
2. Find "OMNISIGHT" in the list
3. Status should show "Running"

### Check System Logs

```bash
# SSH into camera
ssh root@camera-ip

# View ACAP logs
journalctl -u sdkomnisight.service -f

# Check if web server is running
ps aux | grep omnisight
```

### Test Web Interface

Click the **Open** button next to OMNISIGHT in the Apps list, or navigate directly:
```
https://camera-ip:8080/
```

## Troubleshooting

### Package Upload Fails

**Check**: Ensure package size is reasonable (should be ~3KB for current stub version)
```bash
ls -lh OMNISIGHT_0_1_8_aarch64.eap
```

### Application Won't Start

**Check logs**:
```bash
ssh root@camera-ip
journalctl -u sdkomnisight.service -n 50
```

Common issues:
- Python not available (should be in AXIS OS 12+)
- Port 8080 already in use
- Insufficient memory

### Web Interface Shows "Page Not Found"

**Verify files are in package**:
```bash
tar -tzf OMNISIGHT_0_1_8_aarch64.eap | grep html
```

Should show:
```
html/
html/index.html
html/img/
html/img/icon.png
```

## Current Limitations (Phase 1 - Stub Version)

This version is a **demonstration/development build**:

- ❌ No actual camera video integration (VDO API)
- ❌ No ML inference (Larod API)
- ❌ No MQTT swarm communication
- ❌ No configuration settings
- ✅ Web interface works
- ✅ ACAP packaging validated
- ✅ Deployment pipeline working

## Next Steps (Phase 2 - Hardware Integration)

To enable full functionality:

1. **Integrate VDO API** - Connect to camera video stream
2. **Integrate Larod API** - Use DLPU for ML inference
3. **Enable MQTT** - Swarm intelligence communication
4. **Add Configuration UI** - User settings and controls
5. **Implement Real Algorithms** - Replace stub implementations

See [PHASE1_STATUS.md](PHASE1_STATUS.md) for detailed development roadmap.

## Build Script Reference

The main build script is located at: `scripts/build-eap.sh`

Key steps it performs:
1. Create package directory structure
2. Generate Python web server script
3. Copy manifest and HTML files
4. Create LICENSE file
5. Source ACAP SDK environment
6. Run `acap-build` to create .eap package
7. Move .eap to output directory

## Hardware Requirements

### Minimum Requirements
- **Camera**: Axis camera with ARTPEC-8 or ARTPEC-9
- **Firmware**: AXIS OS 12.0 or later
- **RAM**: 128 MB (current stub version)
- **Storage**: 10 MB

### Recommended Hardware for Production
- **Camera**: AXIS Q1656-LE or AXIS P3267-LVE
- **Chipset**: ARTPEC-8 with DLPU (Deep Learning Processing Unit)
- **RAM**: 256+ MB
- **Storage**: 100+ MB

## Supported Cameras

Any Axis camera with ARTPEC-8 or ARTPEC-9:
- AXIS M42xx series (e.g., M4228-LVE) ✅
- AXIS P32xx series
- AXIS Q16xx series
- AXIS Q35xx series

Check camera specifications at: https://www.axis.com/products/online-guide

## References

- [Axis ACAP Documentation](https://www.axis.com/developer-community/acap)
- [ACAP SDK on GitHub](https://github.com/AxisCommunications/acap-sdk)
- OMNISIGHT Build Documentation: [STUB_BUILD.md](STUB_BUILD.md)
- Testing Guide: [TESTING_AND_DEPLOYMENT.md](TESTING_AND_DEPLOYMENT.md)

---

**Last Updated**: October 22, 2025
**OMNISIGHT Version**: 0.1.8 (Stub/Demo)
**Author**: OMNISIGHT Development Team
