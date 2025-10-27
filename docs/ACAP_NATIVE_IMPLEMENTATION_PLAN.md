# ACAP Native SDK Implementation Plan

**Date**: 2025-10-27
**Decision**: Implement OMNISIGHT using ACAP Native SDK (C/C++)
**Approach**: Reverse Proxy + CivetWeb web server
**Target**: P3285-LVR (ARTPEC-9) and M4228-LVE (ARTPEC-8)

---

## Architecture Overview

### Chosen Approach: Reverse Proxy + CivetWeb

Based on Axis ACAP Native SDK examples, we'll use:

**Component Stack:**
```
Client Browser
    ↓
Apache Web Server (built-in on camera)
    ↓ (reverse proxy at /local/omnisight/)
CivetWeb (lightweight C HTTP server, port 8080)
    ↓
OMNISIGHT Application (C code)
    ↓
Static Files (HTML, CSS, JSON) + Dynamic APIs
```

**Why This Approach:**
1. ✅ Matches our v0.2.x architecture (we already designed for this!)
2. ✅ CivetWeb is lightweight (~100 KB) and proven
3. ✅ Easy to serve static files AND dynamic content
4. ✅ Works on all ARTPEC versions
5. ✅ Official Axis example exists (web-server)

**Alternative Considered (FastCGI):**
- More tightly integrated with Apache
- Better for simple CGI scripts
- ❌ More complex for serving static files
- ❌ Harder to implement JSON APIs

---

## Phase 1: Basic Web Server (Week 1)

### Goal:
Replicate v0.2.4 functionality using native C code instead of Python

### Deliverables:
1. ✅ Docker-based build environment
2. ✅ Basic ACAP application that starts/stops
3. ✅ CivetWeb embedded server running on port 8080
4. ✅ Serve static HTML dashboard
5. ✅ Serve static JSON API files
6. ✅ Reverse proxy configuration
7. ✅ Package as .eap and deploy to P3285-LVR

### Tasks:

#### Day 1: Environment Setup
- [x] Clone ACAP Native SDK examples
- [ ] Set up Docker build environment
- [ ] Build "web-server" example successfully
- [ ] Deploy example to P3285-LVR to verify environment

**Commands:**
```bash
# Clone ACAP examples
git clone https://github.com/AxisCommunications/acap-native-sdk-examples.git

# Build web-server example
cd acap-native-sdk-examples/web-server
docker build --tag acap-web-server:latest --build-arg ARCH=aarch64 .

# Extract .eap file
docker cp $(docker create acap-web-server:latest):/opt/app ./web_server_aarch64.eap
```

#### Day 2: OMNISIGHT Native Structure
- [ ] Create `native/` directory in OMNISIGHT repo
- [ ] Copy web-server example as starting point
- [ ] Modify manifest.json for OMNISIGHT
- [ ] Create basic main.c with CivetWeb initialization
- [ ] Build and verify compilation

**Directory Structure:**
```
OmniSight-dev/
├── native/               # NEW: Native C implementation
│   ├── app/
│   │   ├── manifest.json
│   │   ├── omnisight.c
│   │   ├── web_server.c
│   │   ├── web_server.h
│   │   ├── html/         # Copy from hybrid version
│   │   └── api/          # Copy from hybrid version
│   ├── Dockerfile
│   ├── Makefile
│   └── README.md
├── app/                  # OLD: Python version (reference)
├── scripts/              # Keep build scripts
└── docs/                 # Documentation
```

#### Day 3: Static File Serving
- [ ] Implement static file serving in C
- [ ] Serve HTML dashboard from `/html/index.html`
- [ ] Serve JSON files from `/api/*.json`
- [ ] Test all endpoints locally
- [ ] Verify MIME types (text/html, application/json)

**Key Functions:**
```c
// web_server.c
int start_web_server(int port);
int serve_static_file(struct mg_connection *conn, const char *path);
int handle_request(struct mg_connection *conn, void *user_data);
void stop_web_server();
```

#### Day 4: Integration & Testing
- [ ] Add reverse proxy configuration to manifest.json
- [ ] Build complete .eap package
- [ ] Deploy to P3285-LVR
- [ ] Test dashboard access
- [ ] Test API endpoints
- [ ] Verify no "Service Unavailable" errors!

**Expected Result:**
```
✅ https://camera-ip/local/omnisight/ → Dashboard loads
✅ Click "Test Health" → Returns {"status": "healthy"}
✅ Click "Test Stats" → Returns stats JSON
✅ Service stays running (no crashes)
```

#### Day 5: Documentation & Cleanup
- [ ] Document build process
- [ ] Create deployment guide
- [ ] Add troubleshooting section
- [ ] Commit to GitHub
- [ ] Tag as v0.3.0 (first native version)

---

## Phase 2: Dynamic API Endpoints (Week 2)

### Goal:
Add dynamic data generation (still stub data, but generated at runtime)

### Deliverables:
1. ✅ JSON generation in C
2. ✅ Dynamic health/stats endpoints
3. ✅ Timestamp in responses
4. ✅ Basic error handling

### Tasks:

#### Day 1: JSON Library Integration
- [ ] Add cJSON library (lightweight JSON library for C)
- [ ] Create JSON utility functions
- [ ] Test JSON generation for health endpoint

**cJSON Integration:**
```c
#include "cJSON.h"

char* generate_health_response() {
    cJSON *root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "status", "healthy");
    cJSON_AddNumberToObject(root, "uptime", get_uptime());
    cJSON_AddNumberToObject(root, "timestamp", time(NULL));

    char *json_string = cJSON_Print(root);
    cJSON_Delete(root);
    return json_string;
}
```

#### Day 2-3: Implement Dynamic Endpoints
- [ ] `/api/health` - dynamic with timestamp
- [ ] `/api/stats` - dynamic with system stats
- [ ] `/api/perception/status` - stub data
- [ ] `/api/timeline/status` - stub data
- [ ] `/api/swarm/status` - stub data

#### Day 4-5: Testing & Refinement
- [ ] Load testing (100+ requests)
- [ ] Memory leak testing (valgrind)
- [ ] Error handling improvements
- [ ] Deploy and validate on camera

---

## Phase 3: VDO API Integration (Week 3)

### Goal:
Connect to actual camera video stream using VDO API

### Deliverables:
1. ✅ VDO API integration
2. ✅ Real video frame capture
3. ✅ Frame metadata in API responses
4. ✅ Stub object detection on frames

### Tasks:

#### Day 1-2: VDO API Setup
- [ ] Study Axis VDO API documentation
- [ ] Initialize VDO stream connection
- [ ] Capture single frame
- [ ] Extract frame metadata (resolution, timestamp, format)

**VDO API Integration:**
```c
#include <vdo-frame.h>
#include <vdo-stream.h>

VdoStream* init_video_stream() {
    VdoStreamSettings settings = {...};
    return vdo_stream_new(&settings);
}

VdoFrame* capture_frame(VdoStream *stream) {
    return vdo_stream_get_frame(stream);
}
```

#### Day 3: Frame Processing
- [ ] Convert frame to format usable by Larod
- [ ] Add frame info to API responses
- [ ] Update perception status endpoint

#### Day 4-5: Testing & Documentation
- [ ] Verify frame capture works
- [ ] Test on both cameras
- [ ] Document VDO integration
- [ ] Tag as v0.4.0

---

## Phase 4: Larod API Integration (Week 4)

### Goal:
Use Larod (DLPU) for actual ML inference

### Deliverables:
1. ✅ Larod API integration
2. ✅ Load pre-trained model
3. ✅ Run inference on video frames
4. ✅ Return actual detection results

### Tasks:

#### Day 1-2: Larod Setup
- [ ] Study Larod API documentation
- [ ] Initialize Larod engine
- [ ] Load simple object detection model
- [ ] Run test inference

**Larod API Integration:**
```c
#include <larod.h>

larodConnection* init_larod() {
    larodConnection *conn;
    larodCreateConnection(&conn);
    return conn;
}

larodModel* load_model(larodConnection *conn) {
    // Load pre-trained model
}
```

#### Day 3-4: Inference Pipeline
- [ ] Connect VDO → Larod pipeline
- [ ] Process frames through model
- [ ] Extract detection results
- [ ] Update API with real detections

#### Day 5: Testing & Validation
- [ ] Verify detections work
- [ ] Performance testing (FPS)
- [ ] Deploy and validate
- [ ] Tag as v0.5.0

---

## Technical Specifications

### Build Environment:

**Docker Image:**
```dockerfile
FROM axisecp/acap-native-sdk:latest-aarch64-ubuntu

# Install dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    pkg-config \
    libcivetweb-dev

WORKDIR /opt/app
COPY . .

# Build
RUN make clean && make
```

**Makefile:**
```makefile
TARGET = omnisight
ARCH = aarch64
CC = $(ARCH)-linux-gnu-gcc
CFLAGS = -Wall -O2
LIBS = -lcivetweb -lcjson -lpthread

SOURCES = omnisight.c web_server.c
OBJECTS = $(SOURCES:.c=.o)

$(TARGET): $(OBJECTS)
    $(CC) $(OBJECTS) $(LIBS) -o $(TARGET)

%.o: %.c
    $(CC) $(CFLAGS) -c $< -o $@

clean:
    rm -f $(TARGET) $(OBJECTS)
```

### Manifest.json:

```json
{
  "schemaVersion": "1.8.0",
  "acapPackageConf": {
    "setup": {
      "appName": "omnisight",
      "friendlyName": "OMNISIGHT - Precognitive Security",
      "vendor": "OMNISIGHT",
      "version": "0.3.0",
      "embeddedSdkVersion": "3.0",
      "runMode": "respawn",
      "architecture": "aarch64"
    },
    "configuration": {
      "settingPage": "index.html",
      "reverseProxy": [
        {
          "apiPath": "omnisight",
          "target": "http://localhost:8080",
          "access": "admin"
        }
      ]
    }
  }
}
```

### CivetWeb Configuration:

```c
const char *server_options[] = {
    "document_root", "/usr/local/packages/omnisight/html",
    "listening_ports", "8080",
    "num_threads", "4",
    "access_control_allow_origin", "*",
    NULL
};

struct mg_context *ctx = mg_start(NULL, 0, server_options);
```

---

## File Sizes Comparison

| Version | Type | Size | Components |
|---------|------|------|------------|
| v0.2.4 | Python | 9.4 KB | Python script + JSON |
| v0.3.0 | Native C | ~50 KB | Binary + JSON + CivetWeb |
| v0.4.0 | C + VDO | ~80 KB | + VDO integration |
| v0.5.0 | C + VDO + Larod | ~150 KB | + ML model (separate) |

Still much smaller than a bundled Python approach (which would be 50+ MB).

---

## Success Criteria

### Phase 1 (Native Web Server):
- ✅ Builds successfully in Docker
- ✅ Deploys to P3285-LVR without errors
- ✅ Service starts and stays running
- ✅ Dashboard loads at `https://camera-ip/local/omnisight/`
- ✅ Test buttons return valid JSON
- ✅ No "Service Unavailable" errors
- ✅ Memory usage < 10 MB

### Phase 2 (Dynamic APIs):
- ✅ Endpoints return current timestamp
- ✅ Health check shows uptime
- ✅ Stats show actual system metrics
- ✅ No memory leaks over 24 hours

### Phase 3 (VDO Integration):
- ✅ Successfully captures video frames
- ✅ Frame metadata in API responses
- ✅ Performance: ≥10 FPS

### Phase 4 (Larod Integration):
- ✅ Model loads successfully
- ✅ Inference runs on frames
- ✅ Detection results in API
- ✅ Performance: ≥5 FPS

---

## Risk Mitigation

### Risk 1: CivetWeb Not Available
**Mitigation**: Use libmicrohttpd (also lightweight, proven)

### Risk 2: ACAP SDK Learning Curve
**Mitigation**: Start with Axis examples, copy proven patterns

### Risk 3: VDO/Larod API Complexity
**Mitigation**: Tackle in separate phases, use Axis examples

### Risk 4: Cross-Compilation Issues
**Mitigation**: Use official Docker images, test frequently

---

## References

### Official Resources:
- **ACAP Native SDK**: https://github.com/AxisCommunications/acap-native-sdk-examples
- **Web Server Example**: https://github.com/AxisCommunications/acap-native-sdk-examples/tree/main/web-server
- **ACAP Documentation**: https://axiscommunications.github.io/acap-documentation/
- **VDO API**: https://axiscommunications.github.io/acap-documentation/docs/api/vdo-api.html
- **Larod API**: https://axiscommunications.github.io/acap-documentation/docs/api/larod-api.html

### Libraries:
- **CivetWeb**: https://github.com/civetweb/civetweb
- **cJSON**: https://github.com/DaveGamble/cJSON

---

## Timeline Summary

| Week | Phase | Deliverable | Ready for Testing |
|------|-------|-------------|-------------------|
| 1 | Basic Web Server | v0.3.0 | ✅ Day 4 |
| 2 | Dynamic APIs | v0.3.1 | ✅ Day 5 |
| 3 | VDO Integration | v0.4.0 | ✅ Day 5 |
| 4 | Larod Integration | v0.5.0 | ✅ Day 5 |

**Total Timeline**: 4 weeks from start to production-ready with ML inference

**First Deployable Version**: Week 1, Day 4 (native web server working)

---

## Next Immediate Steps

1. **Today**: Set up build environment
2. **Today**: Build web-server example
3. **Today**: Deploy example to P3285-LVR to verify
4. **Tomorrow**: Start OMNISIGHT native implementation
5. **This Week**: Complete Phase 1 (basic web server)

Ready to begin implementation! 🚀
