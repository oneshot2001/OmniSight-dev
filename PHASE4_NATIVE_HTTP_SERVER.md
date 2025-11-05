# Phase 4: Native C Binary HTTP Server

> **Status**: Planning
> **Target Version**: v0.7.0
> **Created**: 2025-11-04

## Overview

Phase 4 implements a native C binary HTTP server to replace the Flask backend approach (which failed due to missing Python on P3285-LVE). This provides a zero-dependency solution that works on any ACAP-compatible camera.

## Why Native C HTTP Server?

### Critical Discovery from Phase 3

P3285-LVE (ARTPEC-9) with AXIS OS 12.6.97 does **NOT** have Python runtime:
- ❌ `/usr/bin/python3` does not exist
- ❌ `/usr/bin/python` does not exist
- ❌ No Python interpreter anywhere on the system

### Advantages of Native C Approach

1. **Zero Dependencies**: No external runtimes required (Python, Node.js, etc.)
2. **Universal Compatibility**: Works on any AXIS camera with ACAP support
3. **Small Binary Size**: 50-200KB vs 5-10MB+ for Python/Node runtimes
4. **Direct Integration**: Native IPC with C core modules (no JSON file overhead)
5. **Performance**: Lower latency, lower memory footprint
6. **Single Process**: Simpler deployment and management

## Library Evaluation

### Option A: Mongoose Embedded Web Server ⭐ RECOMMENDED

**Repository**: https://github.com/cesanta/mongoose
**License**: GPL v2 / Commercial (dual license)
**Maturity**: Very mature (10+ years)
**Size**: ~50KB compiled

**Pros**:
- ✅ Single C file (`mongoose.c` + `mongoose.h`) - trivial to integrate
- ✅ Comprehensive HTTP server with routing
- ✅ WebSocket support (for future real-time updates)
- ✅ Built-in JSON parsing/generation
- ✅ Widely used in embedded systems
- ✅ Excellent documentation and examples
- ✅ Active development and maintenance

**Cons**:
- ⚠️ GPL v2 license (commercial license available for $599)
- ⚠️ Slightly larger binary than microhttpd

**Example Code**:
```c
#include "mongoose.h"

static void fn(struct mg_connection *c, int ev, void *ev_data) {
  if (ev == MG_EV_HTTP_MSG) {
    struct mg_http_message *hm = (struct mg_http_message *) ev_data;
    if (mg_http_match_uri(hm, "/api/status")) {
      mg_http_reply(c, 200, "Content-Type: application/json\r\n",
                    "{\"status\":\"active\",\"version\":\"0.7.0\"}\n");
    }
  }
}

int main(void) {
  struct mg_mgr mgr;
  mg_mgr_init(&mgr);
  mg_http_listen(&mgr, "http://0.0.0.0:8080", fn, NULL);
  for (;;) mg_mgr_poll(&mgr, 1000);
  mg_mgr_free(&mgr);
  return 0;
}
```

### Option B: GNU libmicrohttpd

**Repository**: https://www.gnu.org/software/libmicrohttpd/
**License**: LGPL v2.1+
**Maturity**: Very mature (GNU project)
**Size**: ~100KB compiled

**Pros**:
- ✅ LGPL license (more permissive than GPL)
- ✅ Very stable and well-tested
- ✅ Good performance
- ✅ Official GNU project

**Cons**:
- ❌ More complex API than Mongoose
- ❌ Larger binary size
- ❌ Requires separate library (not single-file)
- ❌ More difficult cross-compilation

### Option C: CivetWeb

**Repository**: https://github.com/civetweb/civetweb
**License**: MIT
**Maturity**: Fork of Mongoose, actively maintained
**Size**: ~200KB compiled

**Pros**:
- ✅ MIT license (most permissive)
- ✅ Modern C/C++ implementation
- ✅ Active community
- ✅ Good documentation

**Cons**:
- ❌ Larger binary than Mongoose
- ❌ More complex than Mongoose
- ❌ Requires C++ compiler for some features

## Recommended Approach: Mongoose

**Rationale**:
1. Simplest integration (single C file)
2. Smallest binary size (~50KB)
3. Perfect fit for embedded systems
4. WebSocket support for future real-time features
5. Built-in JSON handling
6. Proven track record in production

**License Consideration**:
- GPL v2 is acceptable for open-source OMNISIGHT project
- If commercialization needed later, commercial license is available ($599 one-time)

## Architecture Design

### System Overview

```
┌─────────────────────────────────────────────────────────┐
│ AXIS Camera (P3285-LVE)                                 │
│                                                          │
│  ┌────────────────────────────────────────────────────┐ │
│  │ ACAP Application (omnisightv2)                     │ │
│  │                                                     │ │
│  │  ┌──────────────────────────────────────────────┐  │ │
│  │  │ Mongoose HTTP Server (Port 8080)             │  │ │
│  │  │                                               │  │ │
│  │  │  GET /api/status          ────┐              │  │ │
│  │  │  GET /api/perception/*    ────┤              │  │ │
│  │  │  GET /api/timeline/*      ────┤ Route        │  │ │
│  │  │  GET /api/swarm/*         ────┤ Handlers     │  │ │
│  │  │  GET /api/config          ────┤              │  │ │
│  │  │  POST /api/config         ────┘              │  │ │
│  │  │                                               │  │ │
│  │  │  Static Files:                                │  │ │
│  │  │  GET /                    ─── index.html     │  │ │
│  │  │  GET /assets/*            ─── CSS/JS/images  │  │ │
│  │  └───────────────┬───────────────────────────────┘  │ │
│  │                  │ Direct Function Calls             │ │
│  │                  ↓                                    │ │
│  │  ┌──────────────────────────────────────────────┐  │ │
│  │  │ OMNISIGHT Core (omnisight_core.c)            │  │ │
│  │  │                                               │  │ │
│  │  │  ┌────────────┐  ┌─────────────┐  ┌────────┐ │  │ │
│  │  │  │ Perception │  │  Timeline   │  │ Swarm  │ │  │ │
│  │  │  │   Module   │  │   Module    │  │ Module │ │  │ │
│  │  │  └────────────┘  └─────────────┘  └────────┘ │  │ │
│  │  └──────────────────────────────────────────────┘  │ │
│  └─────────────────────────────────────────────────────┘ │
│                                                          │
│  ┌────────────────────────────────────────────────────┐ │
│  │ Apache (AXIS OS Built-in)                          │ │
│  │                                                     │ │
│  │  http://camera-ip/local/omnisightv2/              │ │
│  │         ↓                                          │ │
│  │  reverseProxy → http://localhost:8080              │ │
│  └────────────────────────────────────────────────────┘ │
└─────────────────────────────────────────────────────────┘
```

### HTTP Server Implementation

**File**: `src/http_server.c` + `src/http_server.h`

**Core Components**:
1. **Mongoose Integration**: Single-file embedded server
2. **API Route Handlers**: REST endpoints for all modules
3. **Static File Serving**: React dashboard assets
4. **JSON Serialization**: Convert C structs to JSON responses
5. **CORS Support**: Enable cross-origin requests
6. **WebSocket Support** (future): Real-time event streaming

### API Endpoints

All endpoints return JSON with proper `Content-Type: application/json` headers.

#### System Endpoints

```
GET /api/health
Response: {"status": "healthy", "version": "0.7.0"}

GET /api/status
Response: {
  "status": "active",
  "mode": "acap",
  "version": "0.7.0",
  "uptime_seconds": 3600,
  "fps": 24.5,
  "latency_ms": 12.3,
  "camera_id": "P3285-LVE",
  "modules": {
    "perception": {"status": "active", "fps": 24.5},
    "timeline": {"status": "active", "predictions": 3},
    "swarm": {"status": "active", "cameras": 1}
  }
}

GET /api/stats
Response: {
  "uptime_seconds": 3600,
  "memory_usage_mb": 128,
  "cpu_usage_percent": 15.5,
  "fps": 24.5
}
```

#### Perception Endpoints

```
GET /api/perception/status
Response: {
  "status": "active",
  "fps": 24.5,
  "detections_count": 5,
  "inference_time_ms": 15.2
}

GET /api/perception/detections
Response: {
  "timestamp": "2025-11-04T10:30:00Z",
  "detections": [
    {
      "object_id": 1,
      "class": "person",
      "confidence": 0.95,
      "bbox": {"x": 100, "y": 200, "width": 80, "height": 150},
      "velocity": {"x": 1.5, "y": 0.2}
    }
  ]
}
```

#### Timeline Endpoints

```
GET /api/timeline/predictions
Response: {
  "predictions": [
    {
      "future_id": "A",
      "probability": 0.65,
      "time_horizon_seconds": 300,
      "predicted_events": [
        {
          "event_type": "object_exit",
          "confidence": 0.75,
          "timestamp_offset": 120,
          "location": {"x": 150, "y": 300}
        }
      ]
    }
  ]
}

GET /api/timeline/history
Response: {
  "events": [
    {
      "timestamp": "2025-11-04T10:25:00Z",
      "event_type": "object_detected",
      "details": {"object_id": 1, "class": "person"}
    }
  ]
}
```

#### Swarm Endpoints

```
GET /api/swarm/network
Response: {
  "network_status": "connected",
  "cameras": [
    {
      "camera_id": "P3285-LVE",
      "status": "active",
      "latency_ms": 5.2
    }
  ]
}

GET /api/swarm/cameras
Response: {
  "cameras": [
    {
      "id": "P3285-LVE",
      "ip": "192.168.1.100",
      "status": "active"
    }
  ]
}
```

#### Configuration Endpoints

```
GET /api/config
Response: {
  "perception": {"fps_target": 25, "confidence_threshold": 0.7},
  "timeline": {"prediction_horizon": 300, "max_futures": 5},
  "swarm": {"mqtt_broker": "localhost:1883", "enabled": false}
}

POST /api/config
Request: {"perception": {"fps_target": 30}}
Response: {"status": "updated"}
```

### JSON Serialization Helper

**File**: `src/json_utils.c` + `src/json_utils.h`

**Purpose**: Convert C structs to JSON strings using Mongoose's `mg_json_*` functions

**Example**:
```c
// Convert OmnisightStatus struct to JSON
char* status_to_json(const OmnisightStatus* status) {
    return mg_mprintf(
        "{"
        "\"status\":\"%s\","
        "\"version\":\"%s\","
        "\"fps\":%.1f,"
        "\"uptime_seconds\":%d"
        "}",
        status->status,
        status->version,
        status->fps,
        status->uptime_seconds
    );
}

// In HTTP handler
static void handle_status(struct mg_connection *c, struct mg_http_message *hm) {
    OmnisightStatus status = omnisight_get_status(core);
    char* json = status_to_json(&status);
    mg_http_reply(c, 200, "Content-Type: application/json\r\n", "%s", json);
    free(json);
}
```

## Implementation Plan

### Step 1: Integrate Mongoose

**Tasks**:
1. Download mongoose.c and mongoose.h from GitHub
2. Add to `src/http/` directory
3. Update CMakeLists.txt to include mongoose
4. Create basic HTTP server scaffold in `src/http_server.c`
5. Test basic "Hello World" HTTP response

**Estimated Effort**: 2-4 hours

### Step 2: Implement API Endpoints

**Tasks**:
1. Create route handler table for all API endpoints
2. Implement JSON serialization helpers in `src/json_utils.c`
3. Add handlers for system endpoints (/api/health, /api/status)
4. Add handlers for perception endpoints
5. Add handlers for timeline endpoints
6. Add handlers for swarm endpoints
7. Add handlers for config endpoints
8. Test all endpoints locally

**Estimated Effort**: 1-2 days

### Step 3: Static File Serving

**Tasks**:
1. Configure Mongoose to serve static files from `/html/` directory
2. Copy React build output to package
3. Add CORS headers for development
4. Test dashboard loads correctly
5. Test API calls from React dashboard

**Estimated Effort**: 4-6 hours

### Step 4: ACAP Integration

**Tasks**:
1. Create main entry point that initializes both HTTP server and OMNISIGHT core
2. Add proper signal handling (SIGTERM, SIGINT)
3. Add logging to syslog
4. Create systemd service configuration in manifest.json
5. Test binary runs correctly in Docker ACAP environment

**Estimated Effort**: 1 day

### Step 5: Build Script and Packaging

**Tasks**:
1. Create `scripts/build-phase4-v070.sh`
2. Configure CMake for ACAP cross-compilation (aarch64)
3. Build React frontend
4. Compile native binary with mongoose
5. Create manifest.json with schema 1.8.0 + reverseProxy
6. Package everything into .eap file
7. Test package uploads and runs on P3285-LVE

**Estimated Effort**: 1 day

### Step 6: Testing and Validation

**Tasks**:
1. Deploy to P3285-LVE camera
2. Verify "Open" button appears
3. Verify dashboard loads
4. Test all API endpoints return correct data
5. Monitor system logs for errors
6. Verify stability over extended runtime (1+ hour)
7. Measure performance metrics (CPU, memory, latency)

**Estimated Effort**: 1-2 days

## Total Estimated Effort

**Development**: 4-6 days
**Testing**: 1-2 days
**Total**: 5-8 days

## Success Criteria

1. ✅ Binary compiles successfully for aarch64 architecture
2. ✅ Package installs on P3285-LVE without errors
3. ✅ "Open" button appears in camera web interface
4. ✅ Dashboard loads and displays correctly
5. ✅ All API endpoints return valid JSON responses
6. ✅ Dashboard can fetch real-time data from C core modules
7. ✅ Application runs stably for 1+ hour
8. ✅ Memory usage < 512MB
9. ✅ CPU usage < 30%
10. ✅ API response latency < 50ms

## Migration from v0.5.1 Demo Mode

**Changes for Users**:
- ✅ Dashboard UI remains identical (no changes)
- ✅ API structure remains identical (no changes)
- ✅ Demo mode still available as fallback
- ✅ Same .eap installation process

**Under the Hood**:
- ✅ Flask Python server → Mongoose C binary
- ✅ JSON file IPC → Direct function calls
- ✅ External Python dependency → Zero dependencies
- ✅ ~10MB runtime → ~50KB binary

**Upgrade Path**:
1. Build v0.7.0 package with native HTTP server
2. Test on P3285-LVE
3. If successful, deprecate v0.6.x Flask attempts
4. Keep v0.5.1 as demo-only reference implementation

## Risk Mitigation

### Risk 1: Cross-Compilation Issues
**Probability**: Medium
**Impact**: High
**Mitigation**: Use ACAP SDK Docker environment, test incremental builds, reference official ACAP examples

### Risk 2: Mongoose License Restrictions
**Probability**: Low
**Impact**: Medium
**Mitigation**: GPL v2 acceptable for open-source project, commercial license available if needed ($599)

### Risk 3: Binary Size Exceeds ACAP Limits
**Probability**: Low
**Impact**: Medium
**Mitigation**: Mongoose is only ~50KB, total binary should be < 500KB (well under typical limits)

### Risk 4: Performance Issues
**Probability**: Low
**Impact**: Medium
**Mitigation**: Mongoose proven in embedded systems, C native code is faster than Python

### Risk 5: WebSocket Instability
**Probability**: Medium (for future)
**Impact**: Low
**Mitigation**: WebSocket support optional, can launch without it if issues arise

## Future Enhancements (Post v0.7.0)

### Phase 4.1: WebSocket Real-Time Updates
- Implement WebSocket endpoint `/ws/events`
- Push real-time detection updates to dashboard
- Remove polling, reduce API calls by 90%

### Phase 4.2: Hardware API Integration
- Replace perception stubs with VDO API (real camera feed)
- Replace timeline stubs with Larod API (DLPU inference)
- Replace swarm stubs with MQTT client

### Phase 4.3: Performance Optimization
- Implement connection pooling
- Add response caching for frequently accessed endpoints
- Optimize JSON serialization
- Add gzip compression for large responses

### Phase 4.4: Security Hardening
- Add HTTPS support (TLS)
- Implement rate limiting
- Add authentication middleware
- Sanitize all user inputs

## References

- **Mongoose Documentation**: https://mongoose.ws/documentation/
- **ACAP SDK**: https://github.com/AxisCommunications/acap-native-sdk
- **ACAP Web Server Example**: https://github.com/AxisCommunications/acap-native-sdk-examples/tree/main/web-server
- **Phase 3 Findings**: PHASE3_FINDINGS.md
- **Phase 3 Architecture**: PHASE3_ARCHITECTURE.md

## Conclusion

Phase 4 with native C binary HTTP server is the optimal path forward for OMNISIGHT on P3285-LVE cameras. It eliminates the Python dependency discovered in Phase 3 while providing:

- ✅ Universal compatibility across all ACAP cameras
- ✅ Minimal resource footprint
- ✅ Direct integration with C core modules
- ✅ Production-ready reliability
- ✅ Future-proof architecture

**Recommended Next Steps**:
1. Review this plan with stakeholders
2. Begin Step 1: Integrate Mongoose
3. Develop incrementally with continuous testing
4. Deploy v0.7.0 to P3285-LVE for validation
