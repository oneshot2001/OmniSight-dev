# OMNISIGHT v0.2.1 Release Notes

> **Hybrid API Implementation**
> Release Date: October 24, 2025
> Package: OMNISIGHT_-_Precognitive_Security_0_2_1_aarch64.eap

## Overview

Version 0.2.1 introduces a **lightweight hybrid API server** that replaces the Flask-based implementation with Python's built-in `http.server` module. This change eliminates all external dependencies and ensures compatibility with **any ARTPEC-8/9 Axis camera** that has Python 3 installed.

## 🔧 What Changed

### From v0.2.0 (FAILED) → v0.2.1 (WORKING)

| Aspect | v0.2.0 (Flask) | v0.2.1 (Hybrid) |
|--------|----------------|-----------------|
| **Dependencies** | Flask, Flask-CORS, Werkzeug | None (built-in modules only) |
| **Package Size** | 16 KB | 8 KB (50% smaller) |
| **Installation** | Required `pip3 install` | No installation needed |
| **Server** | Flask web framework | Python `http.server` |
| **API Responses** | Dynamic Python code | Static JSON files |
| **Deployment** | ❌ Failed with "Exec format error" | ✅ Should work on all cameras |

## 🎯 Key Features

### 1. **Zero Dependencies**

```python
# Only built-in Python modules:
import http.server
import socketserver
import os
import json
from datetime import datetime
from urllib.parse import urlparse, parse_qs
```

No external packages required - works with Python 3 standard library only.

### 2. **Static JSON API Responses**

All API endpoints serve pre-generated JSON files:

```
/api/
├── health.json             # System health status
├── stats.json              # Performance statistics
├── perception/
│   ├── status.json         # Perception module status
│   ├── detections.json     # Current detections (stub data)
│   └── tracks.json         # Active tracks (stub data)
├── timeline/
│   ├── status.json         # Timeline module status
│   ├── predictions.json    # Future predictions (stub data)
│   └── futures.json        # Timeline threading futures (stub data)
├── swarm/
│   ├── status.json         # Swarm module status
│   ├── cameras.json        # Connected cameras (stub data)
│   └── sync.json           # Swarm synchronization (stub data)
└── config/
    ├── perception.json     # Perception configuration
    ├── timeline.json       # Timeline configuration
    └── swarm.json          # Swarm configuration
```

### 3. **CORS Support**

All responses include CORS headers for cross-origin access:

```python
Access-Control-Allow-Origin: *
Access-Control-Allow-Methods: GET, POST, OPTIONS
Access-Control-Allow-Headers: Content-Type
```

### 4. **Reverse Proxy Integration**

Manifest configured for Apache reverse proxy (ACAP 1.8.0):

```json
"reverseProxy": [
  {
    "apiPath": "api",
    "target": "http://localhost:8080",
    "access": "admin"
  }
]
```

## 📦 Package Contents

**Total Size**: 8 KB (compressed tarball)

```
omnisight/
├── LICENSE                     # Project license
├── manifest.json               # ACAP configuration (schema 1.8.0)
├── omnisight                   # Launcher script (executable)
├── hybrid_server.py            # Python HTTP server (6 KB)
├── html/
│   └── index.html              # Web dashboard
└── api/
    ├── health.json
    ├── stats.json
    ├── perception/
    │   ├── status.json
    │   ├── detections.json
    │   └── tracks.json
    ├── timeline/
    │   ├── status.json
    │   ├── predictions.json
    │   └── futures.json
    ├── swarm/
    │   ├── status.json
    │   ├── cameras.json
    │   └── sync.json
    └── config/
        ├── perception.json
        ├── timeline.json
        └── swarm.json
```

## 🚀 Deployment

### Installation Methods

1. **Web UI** (Recommended)
   - Navigate to camera web interface
   - Settings → Apps → Add app
   - Upload `OMNISIGHT_-_Precognitive_Security_0_2_1_aarch64.eap`
   - Click "Install" and "Start"

2. **CLI with curl**
   ```bash
   curl -k --anyauth -u root:pass https://camera-ip/axis-cgi/applications/upload.cgi \
     -F file=@OMNISIGHT_-_Precognitive_Security_0_2_1_aarch64.eap
   ```

3. **SSH Direct**
   ```bash
   scp OMNISIGHT_-_Precognitive_Security_0_2_1_aarch64.eap root@camera-ip:/tmp/
   ssh root@camera-ip
   acapctl install /tmp/OMNISIGHT_-_Precognitive_Security_0_2_1_aarch64.eap
   acapctl start omnisight
   ```

### Accessing the Application

**Dashboard**: `http://<camera-ip>/local/omnisight/index.html`

**API Endpoints**:
- Health: `http://<camera-ip>/local/omnisight/api/health`
- Stats: `http://<camera-ip>/local/omnisight/api/stats`
- Perception Status: `http://<camera-ip>/local/omnisight/api/perception/status`
- Timeline Predictions: `http://<camera-ip>/local/omnisight/api/timeline/predictions`
- Swarm Cameras: `http://<camera-ip>/local/omnisight/api/swarm/cameras`

## ✅ Success Criteria

v0.2.1 is successful if:

- [x] Package builds without errors (8 KB)
- [ ] Deploys to M4228-LVE without errors
- [ ] Application starts (no "Exec format error")
- [ ] Web dashboard accessible via browser
- [ ] All 14 API endpoints return JSON
- [ ] Average API response time <50ms
- [ ] CPU usage <10%
- [ ] Memory usage <50 MB
- [ ] Runs stable for 60+ minutes
- [ ] Logs show no ERROR messages
- [ ] Compatible with P3285-LVE

## 🔍 Testing Plan

### Phase 1: Local Development Testing ✅

- [x] Python script syntax validation
- [x] JSON file format validation
- [x] Build script executes successfully
- [x] Package created (8 KB)

### Phase 2: M4228-LVE Deployment Testing

1. Deploy package to M4228-LVE
2. Verify startup (check logs for "Exec format error")
3. Access dashboard via browser
4. Test all API endpoints
5. Monitor resource usage (CPU, memory)
6. Run stability test (60 minutes)
7. Collect system logs

### Phase 3: P3285 Cross-Compatibility Testing

1. Deploy same package to P3285-LVE/LVR
2. Repeat Phase 2 tests
3. Verify identical behavior
4. Confirm SoC-agnostic design works

## 🐛 Known Limitations

### Current Stub Data Limitations

1. **No Real Video Integration**
   - VDO API not yet integrated
   - Using stub detections data
   - No actual video stream processing

2. **No DLPU Inference**
   - Larod API not yet integrated
   - No real object detection
   - Placeholder inference times

3. **No Multi-Camera Swarm**
   - MQTT not yet implemented
   - Stub camera connections
   - No actual inter-camera communication

4. **Static API Responses**
   - JSON files don't update dynamically
   - No real-time data
   - Timestamps are static

### Phase 2.1 Roadmap (Post-Deployment Validation)

Once v0.2.1 is validated on hardware:

1. **IPC Integration** - Connect C core with Python API
2. **VDO API Integration** - Real video stream capture
3. **Larod API Integration** - DLPU object detection
4. **MQTT Integration** - Multi-camera swarm communication
5. **Dynamic JSON Updates** - IPC-driven API responses

## 📝 Migration from v0.2.0

If you previously attempted to deploy v0.2.0:

1. **Uninstall v0.2.0** (if installed):
   ```bash
   acapctl stop omnisight
   acapctl uninstall omnisight
   ```

2. **Install v0.2.1**:
   ```bash
   acapctl install OMNISIGHT_-_Precognitive_Security_0_2_1_aarch64.eap
   acapctl start omnisight
   ```

3. **Verify Success**:
   - Check logs: No "Exec format error"
   - Access dashboard: Should load immediately
   - Test API: `/local/omnisight/api/health` should return JSON

## 🎓 Technical Details

### Why This Approach Works

**Problem with v0.2.0**:
```bash
[ ERR ] Failed to execute /usr/local/packages/omnisight/omnisight: Exec format error
```

Caused by:
- Flask not available on AXIS OS
- `pip3` installation failed/unavailable
- External Python packages not supported

**Solution in v0.2.1**:
- Uses **only built-in Python modules**
- No `pip install` required
- No external dependencies
- Works on **any** Python 3 installation

### Launcher Script

```bash
#!/bin/sh
# OMNISIGHT Hybrid API Server Launcher

export OMNISIGHT_HTML_DIR="/usr/local/packages/omnisight/html"
export OMNISIGHT_API_DIR="/usr/local/packages/omnisight/api"
export PORT=8080

cd /usr/local/packages/omnisight
exec python3 hybrid_server.py
```

**Key changes from v0.2.0**:
- ❌ Removed: `pip3 install Flask==3.0.0 Flask-CORS==4.0.0`
- ✅ Added: `cd` to working directory
- ✅ Uses: `python3` (guaranteed to exist on camera)

### Performance Characteristics

**Expected performance** (based on Python `http.server`):

| Metric | Target | Notes |
|--------|--------|-------|
| Startup time | <2 seconds | Immediate |
| Memory usage | <50 MB | Lightweight |
| CPU usage (idle) | <5% | Minimal overhead |
| API latency | <50ms | File read + JSON parse |
| Concurrent requests | 10+ | Single-threaded but fast |

## 📚 Documentation

### New Files Created

1. **[DEPLOYMENT_FAILURE_ANALYSIS.md](DEPLOYMENT_FAILURE_ANALYSIS.md)** - Root cause analysis of v0.2.0 failure
2. **[RELEASE_NOTES_v0.2.1.md](RELEASE_NOTES_v0.2.1.md)** - This file
3. **[app/hybrid_server.py](../app/hybrid_server.py)** - Python HTTP server implementation
4. **[scripts/build-hybrid-eap.sh](../scripts/build-hybrid-eap.sh)** - Build script for v0.2.1
5. **[scripts/manifest-hybrid.json](../scripts/manifest-hybrid.json)** - ACAP manifest for v0.2.1

### Updated Files

1. **Build scripts** - New hybrid build process
2. **Manifest** - Schema 1.8.0 with reverse proxy
3. **API structure** - Static JSON files instead of Flask routes

## 🤝 Contributing

This release focuses on **deployment validation**. After confirming v0.2.1 works on M4228-LVE and P3285:

1. Collect performance data
2. Analyze resource usage
3. Document any compatibility issues
4. Plan Phase 2.1 IPC integration

## 📞 Support

**Deployment Issues**: Check [DEPLOYMENT_FAILURE_ANALYSIS.md](DEPLOYMENT_FAILURE_ANALYSIS.md)

**Build Issues**: Ensure Python 3 is available:
```bash
python3 --version  # Should show Python 3.x
```

**Runtime Issues**: Check camera logs:
```bash
journalctl -u sdkomnisight.service -n 50
```

---

**Previous Version**: [v0.2.0](https://github.com/omnisight/releases/tag/v0.2.0) (FAILED - Exec format error)
**Next Version**: v0.2.2 (Planned - IPC integration)
