# OMNISIGHT Phase 3 Architecture

**Version**: 0.6.0
**Status**: In Development
**Date**: November 4, 2025

## Overview

Phase 3 integrates the Flask API backend with the proven minimal ACAP packaging pattern from Phase 2, replacing demo mode with real data from C core modules.

## Architecture

```
┌─────────────────────────────────────────────────────────┐
│                    P3285-LVE Camera                     │
│  ┌───────────────────────────────────────────────────┐ │
│  │              OMNISIGHT ACAP Package               │ │
│  │                                                   │ │
│  │  ┌─────────────────────────────────────────────┐ │ │
│  │  │         Apache Web Server (ACAP)          │ │ │
│  │  │  - Serves static HTML/JS (index.html)      │ │ │
│  │  │  - Reverse proxy to Flask (/api/*)        │ │ │
│  │  └────────────┬──────────────────────────────┘ │ │
│  │               │                                 │ │
│  │  ┌────────────▼──────────────────────────────┐ │ │
│  │  │     Flask API Server (Python)             │ │ │
│  │  │  - Port: 8080                             │ │ │
│  │  │  - REST endpoints                         │ │ │
│  │  │  - JSON-based IPC client                  │ │ │
│  │  └────────────┬──────────────────────────────┘ │ │
│  │               │ JSON files                     │ │
│  │  ┌────────────▼──────────────────────────────┐ │ │
│  │  │     C Core Modules (Stubs)                │ │ │
│  │  │  - perception_stub.c                      │ │ │
│  │  │  - timeline_stub.c                        │ │ │
│  │  │  - swarm_stub.c                           │ │ │
│  │  │  - Write JSON to /tmp/omnisight/*.json    │ │ │
│  │  └───────────────────────────────────────────┘ │ │
│  └───────────────────────────────────────────────┘ │
└─────────────────────────────────────────────────────────┘
```

## Key Design Decisions

### 1. Proven Minimal Pattern (Keep What Works)

**From v0.5.1 Success:**
- ✅ Schema 1.5 base structure
- ✅ `settingPage: "index.html"`
- ✅ `SETTINGSPAGEFILE="index.html"`
- ✅ No binary executable (Python script instead)
- ✅ Clean app name

**New for Phase 3:**
- ⚡ Upgrade to Schema 1.8.0 (for reverseProxy support)
- ⚡ Add reverseProxy configuration
- ⚡ Change `runMode: "never"` → `"respawn"` (Flask must run)

### 2. Flask Server Execution

**Challenge**: ACAP expects a binary at `/usr/local/packages/omnisightv2/omnisightv2`

**Solution**: Create shell script wrapper:
```bash
#!/bin/sh
# /usr/local/packages/omnisightv2/omnisightv2
cd /usr/local/packages/omnisightv2
exec python3 app/server.py
```

**Benefits:**
- ✅ Satisfies ACAP systemd requirements
- ✅ Python 3 is built into AXIS OS
- ✅ No cross-compilation needed
- ✅ Easy debugging (just Python)

### 3. Reverse Proxy Configuration

**Manifest Configuration:**
```json
{
  "schemaVersion": "1.8.0",
  "configuration": {
    "settingPage": "index.html",
    "reverseProxy": [
      {
        "apiPath": "api",
        "target": "http://localhost:8080",
        "access": "admin"
      }
    ]
  }
}
```

**How It Works:**
1. User opens dashboard: Apache serves `html/index.html`
2. Dashboard calls `/api/status`: Apache proxies to `http://localhost:8080/api/status`
3. Flask receives request, reads JSON from C stubs, returns data
4. Dashboard receives real data, displays in UI

### 4. IPC: Flask ↔ C Core

**Phase 3 Approach: JSON Files (Simple)**

**Why JSON files:**
- ✅ No shared memory complexity
- ✅ No sockets/pipes to manage
- ✅ Easy debugging (just cat the files)
- ✅ Works perfectly for 1Hz update rate
- ✅ C stubs already write JSON

**File Structure:**
```
/tmp/omnisight/
├── perception_state.json    (Updated by perception_stub)
├── timeline_state.json      (Updated by timeline_stub)
└── swarm_state.json         (Updated by swarm_stub)
```

**C Stub Updates (Every 1 second):**
```c
void perception_update() {
  // Write latest detections
  FILE *f = fopen("/tmp/omnisight/perception_state.json", "w");
  fprintf(f, "{\"detections\": [...], \"fps\": 24.5}");
  fclose(f);
}
```

**Flask Reads:**
```python
def get_detections():
  with open('/tmp/omnisight/perception_state.json') as f:
    data = json.load(f)
  return jsonify(data)
```

## Package Structure

```
OMNISIGHT_v2_0_6_0_aarch64.eap
├── manifest.json              (Schema 1.8.0, reverseProxy)
├── package.conf               (STARTMODE="respawn")
├── param.conf                 (empty)
├── LICENSE
├── omnisightv2                (Shell script → python3 app/server.py)
├── html/
│   ├── index.html            (React build)
│   └── assets/               (CSS, JS)
├── app/
│   ├── server.py             (Flask entry point)
│   ├── requirements.txt      (Flask, Flask-CORS)
│   ├── api/                  (Endpoint blueprints)
│   └── ipc/                  (JSON file reader)
└── lib/                       (empty)
```

## API Endpoints

### System
- `GET /api/health` - Health check
- `GET /api/status` - System status from all modules

### Perception
- `GET /api/perception/status` - FPS, detections count
- `GET /api/perception/detections` - Current frame detections

### Timeline
- `GET /api/timeline/predictions` - Active predictions
- `GET /api/timeline/history` - Past events

### Swarm
- `GET /api/swarm/network` - Network topology
- `GET /api/swarm/cameras` - Camera list

### Configuration
- `GET /api/config` - Current config
- `POST /api/config` - Update config

## Implementation Plan

### Step 1: Update Flask Server for ACAP
- Simplify server.py (remove Claude Flow for now)
- Update IPC client to read from `/tmp/omnisight/`
- Add health check endpoint
- Test locally

### Step 2: Create Shell Wrapper
- Create `omnisightv2` shell script
- Make executable
- Test: `./omnisightv2` → Flask starts

### Step 3: Update ACAP Manifest
- Change schema to 1.8.0
- Add reverseProxy configuration
- Change runMode to "respawn"
- Update version to 0.6.0

### Step 4: Package Assembly
- Copy Flask app/ directory
- Copy shell wrapper as `omnisightv2`
- Copy React build to html/
- Create .eap package

### Step 5: Testing
- Test in Docker first (simulate ACAP environment)
- Deploy to P3285-LVE
- Verify:
  - Open button appears
  - Dashboard loads
  - API calls return real stub data
  - No "Demo Mode" badge

## Success Criteria

✅ **Package Installs**: Successfully installs on P3285-LVE
✅ **Open Button**: Appears in camera web UI
✅ **Flask Starts**: Python server runs and stays running
✅ **API Works**: Dashboard receives data from Flask
✅ **Real Data**: Stub data displayed (not mock data)
✅ **Stable**: Runs for 1+ hour without errors

## Risks & Mitigations

| Risk | Mitigation |
|------|------------|
| Python dependencies missing | Use only stdlib + Flask (included in AXIS OS) |
| Flask crashes | Add error handling, systemd respawn |
| Reverse proxy fails | Test with curl before deploying |
| JSON file permissions | Ensure /tmp/omnisight/ writable by sdk user |
| Port 8080 conflicts | Check camera doesn't use 8080 |

## Future (Phase 4+)

After Phase 3 is stable:
- Replace stubs with real VDO API (camera feed)
- Add Larod API (DLPU ML inference)
- Enable MQTT (swarm communication)
- Add Claude Flow coordinators
- Federated learning

## Notes

- **Keep v0.5.1**: Stable demo version (no backend)
- **v0.6.0**: First version with Flask backend
- **Focus**: Make it work, then make it fast
- **Testing**: Docker first, then camera deployment
