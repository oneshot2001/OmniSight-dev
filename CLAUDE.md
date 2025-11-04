# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

OMNISIGHT is a precognitive security system for Axis cameras running on the ACAP (Axis Camera Application Platform). It combines real-time object detection, multi-timeline prediction, and swarm intelligence across networked cameras.

**Current Status**: Phase 4 V3 (Native) - ARTPEC-9 Compatible (v0.4.2)
- Phase 1 Complete: Stub implementations for all three core modules
- Phase 2 Complete: Flask API server with full C-to-Python IPC integration
- Phase 3 Complete: Hybrid deployment with static API files
- **Phase 4 V3 (Current)**: Native HTTP server without Python dependency
- Phase 5 Next: Native C implementation of Phase 4 coordinators

## Quick Start (30 seconds)

### For Local Development

```bash
# 1. Start Docker environment
docker-compose up -d omnisight-dev

# 2. Enter container
docker exec -it omnisight-dev bash

# 3. Build stub implementation
./scripts/build-stub.sh

# 4. Test locally
./build-stub/omnisight --demo
```

### For Camera Deployment

```bash
# Universal build (works on ALL cameras)
./scripts/build-phase4-v3-native-eap.sh

# Upload via camera web interface:
# 1. Navigate to: http://<camera-ip>
# 2. Settings → Apps → Add
# 3. Upload the .eap file
# 4. Click "Open" to access OMNISIGHT
```

## Which Build Script Should I Use?

```
┌─ Do you have the camera hardware?
│
├─ NO → Use ./scripts/build-stub.sh
│        (For local development and testing)
│
└─ YES → Which camera model?
         │
         ├─ P3285-LVE (ARTPEC-9) → ./scripts/build-phase4-v3-native-eap.sh ✅
         │                          (No Python dependency - REQUIRED)
         │
         ├─ M4228-LVE (ARTPEC-8) → ./scripts/build-phase4-v3-native-eap.sh ✅
         │                          (Recommended - most compatible)
         │                          OR
         │                          ./scripts/build-phase2-eap.sh
         │                          (Advanced - requires Python 3)
         │
         └─ Unknown camera → ./scripts/build-phase4-v3-native-eap.sh ✅
                             (Safest choice - no dependencies)
```

## Technology Stack

### Core System (C)
- **Platform**: ACAP Native SDK 1.15 (aarch64)
- **Target Hardware**: ARTPEC-8/9 cameras (e.g., M4228-LVE, P3285-LVE)
- **Runtime**: AXIS OS 12.0+

### Web Interface
- **Server**: Native HTTP (no Python dependency) or Apache reverse proxy
- **Fallback**: Python SimpleHTTPServer if available
- **API**: Static JSON files (Phase 4 V3) or Flask endpoints (Phase 2)

### Web Frontend (JavaScript)
- **Framework**: React 18+
- **Build**: Vite
- **Styling**: TailwindCSS

## ⚠️ Camera Compatibility Matrix

| Camera Model | ARTPEC | Python 3 | Recommended Package | Status |
|--------------|--------|----------|---------------------|---------|
| **M4228-LVE** | ARTPEC-8 | ✅ Yes | Phase 4 V3 or Phase 2 | Tested |
| **P3285-LVE** | ARTPEC-9 | ❌ No | Phase 4 V3 (Native) | Tested ✅ |
| **Other ARTPEC-9** | ARTPEC-9 | ❌ Unknown | Phase 4 V3 (Native) | Safe choice |
| **Other ARTPEC-8** | ARTPEC-8 | ✅ Likely | Phase 4 V3 or Phase 2 | Unknown |

**Key Finding:** Not all Axis cameras have Python 3 installed! P3285-LVE (ARTPEC-9) requires the native build.

## Core Architecture

### Three-Module System

```
omnisight_core (src/omnisight_core.c/h)
├── Perception Module (src/perception/)
│   ├── Object detection and tracking
│   ├── Behavior analysis
│   └── Threat scoring
├── Timeline Module (src/timeline/)
│   ├── Multi-future predictions (3-5 timelines)
│   ├── Event classification
│   └── Intervention recommendations
└── Swarm Module (src/swarm/)
    ├── Multi-camera coordination
    ├── Track sharing
    └── Federated learning
```

Each module has:
- **Interface**: Public API in header file (`module.h`)
- **Stub**: Simulation implementation (`module_stub.c`)
- **Real**: Hardware implementation (`vdo_capture.c`, `larod_inference.c`, etc.) - Phase 3

### Communication Flow

```
Web Dashboard (React)
    ↓ HTTP REST API
Flask API Server (app/server.py)
    ↓ JSON File IPC (/tmp/omnisight_*.json @ 1Hz)
OMNISIGHT Core (C native binary)
    ↓
Axis Hardware APIs (VDO, Larod)
```

**IPC Architecture (Phase 2 Complete)**:
- C core exports 5 JSON files every second: stats, status, detections, tracks, timelines
- Flask reads JSON files with freshness checking (5s threshold)
- Atomic writes (temp file + rename) prevent partial reads
- Cache fallback for transient failures
- Performance: <5ms export overhead, <10ms API response time

## File Organization (Critical Paths)

```
OmniSight-dev/
├── src/                      # ALL C source code here
│   ├── main.c               # Entry point - START HERE for C changes
│   ├── omnisight_core.c/h   # Core integration layer
│   ├── ipc/                 # JSON export for Python communication
│   │   ├── json_export.c/h  # IPC implementation (500+ lines)
│   │   └── json_common.h    # Shared IPC definitions
│   ├── perception/          # Object detection module
│   │   ├── perception.h     # Public API
│   │   └── perception_stub.c # Stub implementation (350 lines)
│   ├── timeline/            # Prediction module
│   │   ├── timeline.h       # Public API
│   │   └── timeline_stub.c  # Stub implementation (400 lines)
│   └── swarm/               # Multi-camera coordination
│       ├── swarm.h          # Public API
│       └── swarm_stub.c     # Stub implementation (410 lines)
│
├── app/                      # ALL Python/Flask code here
│   ├── server.py            # Flask entry point - START HERE for API changes
│   ├── api/                 # REST endpoint implementations
│   │   ├── perception.py    # Perception endpoints
│   │   ├── timeline.py      # Timeline endpoints
│   │   ├── swarm.py         # Swarm endpoints
│   │   ├── system.py        # System stats/health
│   │   └── config.py        # Configuration management
│   └── ipc/                 # JSON client for C communication
│       └── json_client.py   # IPC client (200+ lines)
│
├── web/frontend/            # ALL React code here
│   ├── src/App.jsx          # React entry point - START HERE for UI changes
│   └── src/components/      # Dashboard components
│       ├── Dashboard.jsx
│       ├── EventMonitor.jsx
│       ├── TimelineViewer.jsx
│       ├── ThreatHeatmap.jsx
│       ├── CameraNetwork.jsx
│       └── ConfigPanel.jsx
│
├── scripts/                  # Build and deployment scripts
│   ├── build-stub.sh        # For local development (FASTEST)
│   ├── build-phase4-v3-native-eap.sh  # For camera deployment (RECOMMENDED)
│   ├── build-phase2-eap.sh  # Flask server (requires Python 3)
│   └── testing/             # Validation scripts
│       ├── test-flask-api.sh
│       ├── test-ipc-integration.sh
│       └── validate_flask_app.sh
│
├── docs/                     # All documentation (see docs/README.md)
│   ├── guides/              # How-to guides
│   ├── phase1-4/            # Phase documentation
│   ├── development/         # Architecture and design docs
│   └── troubleshooting/     # Common issues and fixes
│
├── native/                   # Native binary outputs
├── packages/                 # Output .eap files appear here
├── manifest.json            # ACAP manifest (critical for deployment)
├── Dockerfile               # Development container
├── docker-compose.yml       # Container orchestration
└── Makefile                 # High-level build automation
```

## Development Commands

### Docker Environment

Start the development container:
```bash
docker-compose up -d omnisight-dev
docker-compose exec omnisight-dev bash
```

Stop the development container:
```bash
docker-compose down
```

### Building

#### Stub Implementation (No Hardware Required)
```bash
# Inside Docker or on host
./scripts/build-stub.sh

# Run in demo mode
./build-stub/omnisight --demo
```

#### ACAP Packages

**⚠️ IMPORTANT: Python Dependency Issue**

P3285-LVE (ARTPEC-9) cameras **DO NOT have Python 3 installed**. Always use Phase 4 V3 (Native) for maximum compatibility.

**Phase 4 V3 (Native) - RECOMMENDED** (No Python dependency):
```bash
./scripts/build-phase4-v3-native-eap.sh
```

**Phase 2 Package** (Flask API server - requires Python 3):
```bash
# Only for M4228-LVE (ARTPEC-8) with Python 3
./scripts/build-phase2-eap.sh
```

**Phase 1 Package** (Python HTTP server, static HTML):
```bash
# Legacy - not recommended
./scripts/build-eap.sh
```

**Extract Package from Docker**:
```bash
docker cp omnisight-dev:/opt/app/output/<package-name>.eap ./
```

### Web Development

#### Frontend (React)
```bash
cd web/frontend
npm install
npm run dev        # Development server on :5173
npm run build      # Production build
```

#### API Server (Flask)
```bash
cd app
pip3 install -r requirements.txt
python3 server.py  # Runs on :8080

# Quick validation
./scripts/testing/validate_flask_app.sh
./scripts/testing/QUICK_API_TEST.sh
```

### Testing

#### Run Tests
```bash
# All tests
make test

# Module-specific
make test-perception
make test-timeline
make test-swarm
```

#### Integration Tests
```bash
# Test Flask API endpoints
cd scripts/testing
./test-flask-api.sh

# Test C-to-Flask IPC
./test-ipc-integration.sh

# Validate Flask application
./validate_flask_app.sh
```

#### Manual Testing
```bash
# Test stub locally
./build-stub/omnisight --demo

# Test API endpoints
curl http://localhost:8080/api/health
curl http://localhost:8080/api/stats
curl http://localhost:8080/api/perception/status
curl http://localhost:8080/api/timeline/predictions
```

### Deployment

**Upload to camera** (via web interface):
1. Navigate to camera IP → Settings → Apps
2. Click "Add" → Upload `.eap` file
3. Click "Open" to access web interface

**Upload via CLI**:
```bash
curl -k -u root:password \
  -F 'package=@<package-name>.eap' \
  https://<camera-ip>/axis-cgi/applications/upload.cgi
```

**Start/Stop Application**:
```bash
# Using Makefile
make start CAMERA_IP=192.168.1.100
make stop CAMERA_IP=192.168.1.100
make restart CAMERA_IP=192.168.1.100

# Or directly
curl -k -u root:pass "https://<camera-ip>/axis-cgi/applications/control.cgi?action=start&package=omnisight"
curl -k -u root:pass "https://<camera-ip>/axis-cgi/applications/control.cgi?action=stop&package=omnisight"
```

## REST API Endpoints (Phase 2+)

### System
- `GET /api/health` - Health check
- `GET /api/stats` - System statistics

### Perception
- `GET /api/perception/status` - Module status and FPS
- `GET /api/perception/detections` - Current frame detections
- `GET /api/perception/tracks` - Active object tracks

### Timeline
- `GET /api/timeline/predictions` - Multi-future predictions
- `GET /api/timeline/history` - Past events
- `GET /api/timeline/interventions` - Recommended actions

### Swarm
- `GET /api/swarm/network` - Network topology
- `GET /api/swarm/cameras` - All cameras in swarm
- `POST /api/swarm/sync` - Manual sync trigger

### Configuration
- `GET /api/config` - Current configuration
- `POST /api/config` - Update configuration

## Common Development Tasks

### Task: Add a new object detection type

**Files to modify:**
1. `src/perception/perception.h` - Add enum value
2. `src/perception/perception_stub.c` - Update detection logic
3. `app/api/perception.py` - No changes needed (auto-updates via IPC)
4. `web/frontend/src/components/Dashboard.jsx` - Add UI handling

**Steps:**
```bash
# 1. Edit files above
vim src/perception/perception.h
vim src/perception/perception_stub.c

# 2. Rebuild and test
./scripts/build-stub.sh
./build-stub/omnisight --demo

# 3. Deploy to camera
./scripts/build-phase4-v3-native-eap.sh
# Upload via camera web interface
```

### Task: Add a new API endpoint

**Files to modify:**
1. `app/api/<module>.py` - Add new route
2. `app/server.py` - Register blueprint (if new module)

**Steps:**
```bash
# 1. Edit API file
cd app
vim api/perception.py  # Example

# 2. Test locally
python3 server.py

# 3. Validate
curl http://localhost:8080/api/your-new-endpoint

# 4. Deploy
cd ..
./scripts/build-phase2-eap.sh  # If using Python
# OR
./scripts/build-phase4-v3-native-eap.sh  # If using static files
```

### Task: Modify the web dashboard

**Files to modify:**
1. `web/frontend/src/components/<Component>.jsx`
2. `web/frontend/src/App.jsx` (if adding new component)

**Steps:**
```bash
cd web/frontend

# Development mode (hot reload)
npm run dev
# Open http://localhost:5173

# Production build
npm run build

# Deploy
cd ../..
./scripts/build-phase4-v3-native-eap.sh
```

### Task: Update IPC data format

**Files to modify:**
1. `src/ipc/json_export.c` - Update C export logic
2. `app/ipc/json_client.py` - Update Python parsing (if needed)
3. `src/ipc/json_common.h` - Update shared constants

**Steps:**
```bash
# 1. Edit files
vim src/ipc/json_export.c

# 2. Rebuild
./scripts/build-stub.sh

# 3. Test IPC integration
./scripts/testing/test-ipc-integration.sh

# 4. Verify API still works
cd app
python3 server.py &
curl http://localhost:8080/api/stats
```

## Debugging & Troubleshooting

### Check if Application is Running on Camera

```bash
# SSH into camera (if enabled)
ssh root@<camera-ip>

# Check running processes
ps | grep omnisight

# View application logs
tail -f /var/log/syslog | grep omnisight

# Check application status
curl -k -u root:pass "https://<camera-ip>/axis-cgi/applications/list.cgi"
```

### Access Application via Browser

```
http://<camera-ip>/local/omnisight/
```

### View Application Logs

```bash
# From host machine
make logs CAMERA_IP=192.168.1.100

# Or directly
curl -k -u root:pass "https://<camera-ip>/axis-cgi/systemlog.cgi?appname=omnisight"
```

### Common Deployment Issues

| Symptom | Cause | Solution |
|---------|-------|----------|
| No "Open" button after install | Python missing (ARTPEC-9) | Use Phase 4 V3 native build |
| "Failed to convert appId" | appId is string not number | Fix manifest.json (use numeric ID) |
| Can't access web interface | runMode set to "never" | Set to "respawn" in manifest |
| Package upload fails | Wrong architecture | Rebuild for aarch64 |
| Application crashes on start | Missing dependencies | Check system logs, use native build |
| API returns stale data | C core not running | Check IPC files in /tmp/omnisight_*.json |
| Port conflicts in dev | Port 5000 conflicts (AirPlay) | Use port 5001 (already configured) |

### Verify IPC is Working

```bash
# Inside Docker container or on camera
ls -lh /tmp/omnisight_*.json

# Should show 5 files updated every second:
# omnisight_stats.json
# omnisight_status.json
# omnisight_detections.json
# omnisight_tracks.json
# omnisight_timelines.json

# Check file contents
cat /tmp/omnisight_stats.json | python3 -m json.tool
```

## ⚠️ Known Issues & Solutions

### Python Dependency (CRITICAL)

**Issue:** P3285-LVE (ARTPEC-9) cameras don't have Python 3 installed

**Error:** `python3: not found` when trying to start Flask server

**Symptom:** Application installs but no "Open" button appears

**Solution:** Use Phase 4 V3 (Native) build: `./scripts/build-phase4-v3-native-eap.sh`

**Affected Versions:**
- ❌ v0.4.0 - Failed install (missing config files)
- ❌ v0.4.1 - Installed but crashed (Python not found)
- ✅ v0.4.2 - Works without Python dependency

**Documentation:** See [docs/troubleshooting/deployment-fixes/PHASE4_PYTHON_DEPENDENCY_ISSUE.md](docs/troubleshooting/deployment-fixes/PHASE4_PYTHON_DEPENDENCY_ISSUE.md)

### CMake Cross-Compilation

**Issue:** ACAP SDK CMake forces ARM flags, breaking x86 builds

**Workaround:** Use `./scripts/build-stub.sh` for direct gcc compilation

**Status:** Phase 4 V3 doesn't require CMake

### Port Conflicts

**Issue:** Development server port 5000 conflicts with macOS AirPlay

**Solution:** Changed to port 5001 in development configuration

**Status:** Already fixed in docker-compose.yml

## Important Build System Notes

### Known CMake Issue
The ACAP SDK Docker image forces ARM cross-compilation flags even for x86_64 builds. This breaks standard CMake builds.

**Workaround**: Use `./scripts/build-stub.sh` which compiles with gcc directly.

### ACAP Manifest Critical Requirements

**appId must be numeric** (not string):
```json
"appId": 1001  // ✓ Correct
"appId": "com.omnisight"  // ✗ Wrong - causes deployment failure
```

**runMode for web servers**:
- `"respawn"` - Required for continuous processes (web servers)
- `"never"` - Cannot serve web pages (no running process)

**Schema versions**:
- `1.5` - Basic ACAP (Phase 1)
- `1.7.3` - FastCGI support
- `1.8.0` - Reverse proxy support (Phase 2+)

### Docker Volume Mounting
Build scripts must be in `scripts/` directory to be accessible inside the container. Build outputs go to `/opt/app/output/` inside container.

## Current Development Phase

### Phase 1 Complete (40% of total project)
- ✅ Stub implementations for all modules
- ✅ Integration layer working
- ✅ Build and packaging pipeline
- ✅ Static web interface
- ✅ Demo mode functional

### Phase 2 Complete (30% of total project)
- ✅ Flask API server structure
- ✅ React dashboard components
- ✅ Manifest upgraded to schema 1.8.0
- ✅ API endpoint implementation
- ✅ JSON-based IPC layer (C → Flask)
- ✅ Full integration testing passed
- ✅ All endpoints returning real stub data
- ✅ Performance validated (<10ms response times)

**Key Achievement**: C core now communicates with Flask API via JSON files, enabling real-time dashboard updates with stub data. System is fully operational and ready for hardware integration.

### Phase 3 Next (30% of total project - Hardware Integration)
Requires physical Axis camera:
- ⏳ VDO API for real video capture
- ⏳ Larod API for DLPU inference
- ⏳ MQTT swarm communication
- ⏳ Real object tracking algorithms
- ⏳ Real timeline prediction models
- ⏳ Federated learning implementation

## 📚 Documentation Organization

**All documentation is now organized in the `docs/` directory. See [docs/README.md](docs/README.md) for the complete index.**

### Quick Links

#### Getting Started
- [Installation Guide](docs/guides/INSTALLATION_GUIDE.md) - Complete installation instructions
- [Quick Troubleshooting](docs/troubleshooting/QUICK_TROUBLESHOOTING.md) - Fast diagnosis guide
- [ACAP Packaging Guide](docs/acap/ACAP_PACKAGING.md) - Complete packaging guide (405 lines)

#### Critical Issues & Solutions
- [Phase 4 Python Dependency Issue](docs/troubleshooting/deployment-fixes/PHASE4_PYTHON_DEPENDENCY_ISSUE.md) - **Must read for deployment** ⚠️
- [Documentation Cleanup Summary](docs/troubleshooting/deployment-fixes/DOCUMENTATION_CLEANUP_SUMMARY.md) - Recent changes

#### Phase Documentation
- **Phase 1**: [docs/phase1/PHASE1_STATUS.md](docs/phase1/PHASE1_STATUS.md) - Stub implementation status
- **Phase 2**: [docs/phase2/PHASE2_ROADMAP.md](docs/phase2/PHASE2_ROADMAP.md) - API server implementation
- **Phase 3**: [docs/phase3/PHASE3_PROGRESS_SUMMARY.md](docs/phase3/PHASE3_PROGRESS_SUMMARY.md) - Hardware integration
- **Phase 4**: [docs/phase4/PHASE4_OVERVIEW.md](docs/phase4/PHASE4_OVERVIEW.md) - Claude Flow coordination

#### Development Guides
- [IPC Architecture](docs/development/IPC_ARCHITECTURE.md) - Inter-process communication design
- [Architecture Document](docs/development/ARCHITECTURE.md) - System architecture overview
- [Stub Build Guide](docs/guides/STUB_BUILD.md) - Building stub implementations

#### Web Interface
- [web/README.md](web/README.md) - Web interface architecture
- [app/README.md](app/README.md) - Flask API documentation

### External Resources
- Axis ACAP Docs: https://developer.axis.com/acap
- ACAP SDK: https://github.com/AxisCommunications/acap-sdk
- Axis Developer Portal: https://developer.axis.com/

## Hardware Requirements

**For Phase 1-2 (Stub/API Development)**:
- Docker Desktop
- 8GB+ RAM
- No camera required

**For Phase 3 (Hardware Integration)**:
- Axis camera with ARTPEC-8 or ARTPEC-9
- AXIS OS 12.0+
- Minimum 2GB RAM, 4GB storage
- Network connectivity for swarm features

## Performance Targets (Phase 3)

| Metric | Target | Current Status |
|--------|--------|----------------|
| Inference Speed | <20ms per frame | Stub (no real inference) |
| Prediction Horizon | 5 minutes | Implemented in stub |
| False Positive Rate | <3% | Not measured (stub data) |
| Memory Footprint | <512MB | ~128MB (stub only) |
| CPU Usage | <30% | Not measured |
| Network Bandwidth | <500Kbps | Not measured |
| IPC Overhead | <5ms | ✅ Measured: 2-3ms |
| API Response Time | <50ms | ✅ Measured: 8-12ms |

## Working with Stubs

The stub implementations provide realistic simulation for development without hardware:

**Perception Stub** (`src/perception/perception_stub.c`):
- Simulates 1-3 people walking across field of view
- Occasional vehicle detections
- Realistic bounding boxes and trajectories
- Behavior detection (loitering, running)
- Threat scores (0-100)
- Updates at 30 FPS

**Timeline Stub** (`src/timeline/timeline_stub.c`):
- Generates 1-3 timeline branches
- Predicts events: THEFT, TRESPASSING, SUSPICIOUS_BEHAVIOR, LOITERING, VIOLENCE
- Severity scoring: low, medium, high, critical
- Intervention recommendations with confidence scores
- 5-minute prediction horizon

**Swarm Stub** (`src/swarm/swarm_stub.c`):
- Simulates 2-4 neighboring cameras
- Track sharing with global IDs
- Consensus voting on events
- Network health metrics
- Simulated MQTT communication

## Makefile Quick Reference

```bash
# Development
make dev              # Start dev environment
make shell            # Enter container shell

# Building
make build            # Build ACAP application
make package          # Create .eap package

# Deployment
make install CAMERA_IP=192.168.1.100   # Install on camera
make deploy          # Build + package + install
make start/stop/restart CAMERA_IP=...  # Control app

# Testing
make test            # Run all tests
make test-perception # Test perception module
make test-timeline   # Test timeline module
make test-swarm      # Test swarm module

# Monitoring
make logs CAMERA_IP=...    # View application logs
make status CAMERA_IP=...  # Check app status
make monitor CAMERA_IP=... # Real-time monitoring

# Cleanup
make clean           # Remove build artifacts
make clean-all       # Deep clean (includes Docker)
```
