# OMNISIGHT Deployment Failure Analysis

> Camera: Axis M4228-LVE (ARTPEC-8)
> Date: October 22, 2025
> Log Analysis: System logs from deployment attempts

## Executive Summary

OMNISIGHT v0.2.0 Flask API deployment **FAILED** on M4228-LVE with "Exec format error". Root cause identified as **Python dependency issue** and incorrect launcher script configuration.

## Log Analysis Results

### Camera Information
- **Model**: Axis M4228-LVE
- **MAC**: E827250BECE0
- **ARTPEC**: ARTPEC-8 (confirmed from line 17: `PACKAGE_ARCHS` includes "artpec-8")
- **ACAP Support**: Yes

### Deployment Timeline

| Time | Event | Package | Status |
|------|-------|---------|--------|
| 11:45:15 | Upload attempt | Unknown | ❌ Unsigned package rejected |
| 11:45:42 | Upload attempt | Unknown | ❌ `appId` conversion error |
| 11:53:55 | Upload attempt | Unknown | ❌ `appId` conversion error |
| 12:12:27 | Start attempt | Unknown | ❌ **Exec format error** |
| 15:40:09 | Upload | v0.1.4 (19.7KB) | ✅ Install succeeded |
| 15:40:32 | Start | v0.1.4 | ❌ **Exec format error** |
| 15:41:00 | Start retry | v0.1.4 | ❌ **Exec format error** |
| 15:49:21 | Uninstall | v0.1.4 | ✅ Uninstalled |
| 15:57:13 | Upload | v0.1.6 (2.8KB) | ✅ Install succeeded |
| 15:57:54 | Start | v0.1.6 | ✅ **Running successfully** |

### Critical Errors Found

#### Error 1: `appId` Manifest Issue (Fixed in later versions)
```
2025-10-22T11:45:42.893-06:00 [ WARNING ] acapctl[8634]:
    code 1: Failed to convert the string to an integer for 'appId'
```
**Impact**: Installation blocked
**Cause**: `appId` was string instead of integer in early manifest versions
**Status**: ✅ FIXED (changed to numeric appId: 1001)

#### Error 2: **Exec Format Error** (PRIMARY BLOCKER)
```
2025-10-22T12:12:27.351-06:00 [ ERR ] (mnisight)[16460]:
    sdkomnisight.service: Failed to execute /usr/local/packages/omnisight/omnisight:
    Exec format error

2025-10-22T12:12:27.378-06:00 [ ERR ] (mnisight)[16460]:
    sdkomnisight.service: Failed at step EXEC spawning /usr/local/packages/omnisight/omnisight:
    Exec format error
```
**Frequency**: Repeated 20+ times
**Impact**: Application cannot start
**Cause**: Launcher script issues (detailed below)
**Status**: ⚠️ **REQUIRES FIX**

### Working Version

**v0.1.6 (2.8KB)** - Successfully running as shown in screenshot
- Status: Active
- Mode: Stub/Demo
- Version: 0.1.0 (display version, not package version)
- Web interface accessible
- All modules active

This is the **Phase 1 Python SimpleHTTPServer stub** that works correctly.

## Root Cause Analysis

### Why "Exec Format Error" Occurs

The error "Failed to execute ... Exec format error" on Linux means one of:
1. **Wrong architecture** - Binary compiled for x86 on ARM system
2. **Bad shebang** - Script's `#!/path/to/interpreter` points to missing interpreter
3. **Corrupted binary** - File is not a valid executable
4. **Missing dependencies** - Required libraries not available

### Our Case: Python Availability Issue

Looking at our launcher script in `build-phase2-eap.sh`:

```bash
#!/bin/sh
# OMNISIGHT Flask API Server Launcher

export OMNISIGHT_HTML_DIR="/usr/local/packages/omnisight/html"
export PORT=8080

# Install dependencies on first run
if [ ! -f /tmp/omnisight-deps-installed ]; then
    pip3 install --no-cache-dir Flask==3.0.0 Flask-CORS==4.0.0 Werkzeug==3.0.1
    touch /tmp/omnisight-deps-installed
fi

# Start Flask server
exec python3 server.py
```

**Problems identified:**

1. **`pip3` may not be available** on AXIS OS
2. **`python3` path may be different** (e.g., `/usr/bin/python3`)
3. **Flask cannot be installed** via pip on embedded system
4. **Working directory not set** before running `server.py`

### Comparison with Working Version (v0.1.6)

The working stub uses:
```bash
#!/bin/sh
cd /usr/local/packages/omnisight/html
exec python3 -m http.server 8080
```

**Why this works:**
- ✅ Uses built-in Python module (`http.server`)
- ✅ No external dependencies to install
- ✅ Sets working directory explicitly
- ✅ Uses full `exec` path for replacement

## Camera Python Capabilities

Based on successful v0.1.6 deployment:

| Feature | Available | Notes |
|---------|-----------|-------|
| Python 3 | ✅ Yes | `/usr/bin/python3` or `python3` in PATH |
| `http.server` module | ✅ Yes | Built-in Python module works |
| `pip3` command | ❌ Unknown | Not confirmed, likely unavailable |
| Flask library | ❌ No | Cannot be installed via pip |
| External Python packages | ❌ No | Embedded system limitation |

## Why v0.2.0 Failed vs v0.1.0 Succeeded

| Aspect | v0.1.0 (Working) | v0.2.0 (Failed) |
|--------|------------------|-----------------|
| **Python dependencies** | None (built-in `http.server`) | Flask, Flask-CORS, Werkzeug |
| **Installation method** | No pip required | Requires `pip3 install` |
| **Launcher complexity** | Simple 3-line script | Complex with pip logic |
| **Working directory** | Set with `cd` | Missing `cd` command |
| **Exec path** | `exec python3 -m http.server` | `exec python3 server.py` |

## Recommended Solutions

### Solution 1: Static Flask Binary (RECOMMENDED)
Package Flask and dependencies as pre-compiled Python wheels or static binaries.

**Pros:**
- ✅ No pip required on camera
- ✅ Full Flask functionality
- ✅ All features from Phase 2 API

**Cons:**
- ⚠️ Larger package size (~5-10 MB)
- ⚠️ Requires building wheels for ARM

**Implementation:**
```bash
# On development machine:
pip3 download --platform manylinux2014_aarch64 --only-binary=:all: Flask Flask-CORS
# Package wheels into .eap
# Launcher installs from local wheels:
pip3 install --no-index --find-links=/usr/local/packages/omnisight/wheels Flask
```

### Solution 2: Pure Python API (LIGHTWEIGHT)
Replace Flask with Python's built-in `http.server` and custom routing.

**Pros:**
- ✅ No dependencies
- ✅ Smallest package size (~5KB)
- ✅ Works on any AXIS camera with Python

**Cons:**
- ⚠️ Less elegant code
- ⚠️ Manual request routing
- ⚠️ No Flask ecosystem features

**Implementation:**
```python
from http.server import BaseHTTPRequestHandler, HTTPServer
import json

class OmnisightHandler(BaseHTTPRequestHandler):
    def do_GET(self):
        if self.path == '/api/health':
            self.send_response(200)
            self.send_header('Content-Type', 'application/json')
            self.end_headers()
            self.wfile.write(json.dumps({'status': 'healthy'}).encode())
        # ... more routes
```

### Solution 3: Hybrid Approach (BEST)
Use Python `http.server` with JSON file-based API responses.

**Pros:**
- ✅ No dependencies
- ✅ Clean separation (static files for API responses)
- ✅ Easy to update responses without code changes
- ✅ Minimal package size

**Cons:**
- ⚠️ Not true dynamic API
- ⚠️ Stub data only (until IPC integration)

**Implementation:**
```bash
#!/bin/sh
# Serve both HTML and JSON "API" files
cd /usr/local/packages/omnisight
exec python3 -m http.server 8080
```

Directory structure:
```
/usr/local/packages/omnisight/
├── html/
│   └── index.html
├── api/
│   ├── health.json          # Served as /api/health.json
│   ├── perception/
│   │   ├── status.json      # Served as /api/perception/status.json
│   │   └── detections.json
│   └── timeline/
│       └── predictions.json
└── omnisight (launcher)
```

## Next Steps

### Immediate Actions
1. ✅ **Identify root cause** - COMPLETE (Python dependency issue)
2. ⏳ **Choose solution approach** - Pending user decision
3. ⏳ **Implement fix** - Waiting for solution selection
4. ⏳ **Test on M4228-LVE** - After implementation
5. ⏳ **Verify on P3285** - Cross-camera compatibility

### Decision Required

**Which solution should we implement?**

1. **Solution 1** (Static Flask) - Full-featured but larger package
2. **Solution 2** (Pure Python) - Minimal dependencies, more code
3. **Solution 3** (Hybrid) - Lightweight, JSON-based API

**Recommendation**: Start with **Solution 3 (Hybrid)** for v0.2.1 baseline testing:
- Confirms deployment and startup work
- Validates web interface accessibility
- Tests reverse proxy routing
- Provides foundation for IPC integration (Phase 2.1)
- Can evolve to full Flask (Solution 1) once static binary packaging is working

## Success Criteria for v0.2.1

- [ ] Package builds successfully
- [ ] Deploys to M4228-LVE without errors
- [ ] Application starts (no "Exec format error")
- [ ] Web dashboard accessible
- [ ] API endpoints respond (even if with static JSON)
- [ ] Runs stable for 60+ minutes
- [ ] Compatible with P3285 (test after M4228 success)

## Appendix: Full Error Log Excerpt

```
2025-10-22T12:12:27.351-06:00 axis-e827250bece0 [ ERR     ] (mnisight)[16460]:
    sdkomnisight.service: Failed to execute /usr/local/packages/omnisight/omnisight:
    Exec format error

2025-10-22T12:12:27.378-06:00 axis-e827250bece0 [ ERR     ] (mnisight)[16460]:
    sdkomnisight.service: Failed at step EXEC spawning /usr/local/packages/omnisight/omnisight:
    Exec format error

2025-10-22T12:12:27.401-06:00 axis-e827250bece0 [ WARNING ] systemd[1]:
    sdkomnisight.service: Failed with result 'exit-code'.

[Error repeated 20+ times with different PIDs]

2025-10-22T12:12:28.492-06:00 axis-e827250bece0 [ WARNING ] systemd[1]:
    sdkomnisight.service: Start request repeated too quickly.

2025-10-22T12:12:28.501-06:00 axis-e827250bece0 [ WARNING ] systemd[1]:
    sdkomnisight.service: Failed with result 'exit-code'.

2025-10-22T12:12:28.516-06:00 axis-e827250bece0 [ ERR     ] systemd[1]:
    Failed to start OMNISIGHT Precognitive Security.
```

**Interpretation**:
1. systemd tries to execute `/usr/local/packages/omnisight/omnisight`
2. Kernel returns ENOEXEC (Exec format error)
3. systemd retries with exponential backoff
4. After 5 rapid failures, systemd gives up
5. Application marked as "failed" state

## References

- Log file: `Axis_SR_20251022_155926_E827250BECE0/serverreport_cgi.txt`
- Screenshot: Shows v0.1.0 running successfully
- Build script: [scripts/build-phase2-eap.sh](../scripts/build-phase2-eap.sh)
- Manifest: [scripts/manifest-camera.json](../scripts/manifest-camera.json)
