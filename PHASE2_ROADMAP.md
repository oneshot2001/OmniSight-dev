# OMNISIGHT Phase 2 Roadmap

**Phase**: Hardware Integration & Real APIs
**Timeline**: 2-3 weeks
**Status**: Planning
**Last Updated**: 2025-10-22

## Phase 2 Overview

Phase 1 delivered working stub implementations that validate the architecture. Phase 2 transitions from simulation to real camera integration with live video processing, ML inference, and swarm networking.

### Goals

1. ✅ Replace Python SimpleHTTPServer with Flask API server
2. ✅ Integrate Axis VDO API for real video capture
3. ✅ Integrate Axis Larod API for DLPU-accelerated inference
4. ✅ Enable MQTT swarm communication between cameras
5. ✅ Create interactive web dashboard with real-time data
6. ✅ Deploy and test on M4228-LVE (ARTPEC-8) hardware

## Architecture Evolution

### Phase 1 (Current)
```
┌─────────────────────────────────────┐
│   Python SimpleHTTPServer (8080)    │
│   - Serves static HTML only         │
│   - No dynamic data                 │
└─────────────────────────────────────┘
            ↓
┌─────────────────────────────────────┐
│   Static HTML Interface             │
│   - Status: Active                  │
│   - Mode: Stub/Demo                 │
└─────────────────────────────────────┘
```

### Phase 2 (Target)
```
┌──────────────────────────────────────────────────┐
│   Apache Reverse Proxy (AXIS OS)                 │
│   - Authentication & TLS                         │
│   - Routes /api/* to localhost:8080              │
└──────────────────────────────────────────────────┘
            ↓
┌──────────────────────────────────────────────────┐
│   Python Flask API Server (8080)                 │
│   - Serves HTML                                  │
│   - REST APIs:                                   │
│     • GET  /api/perception/status                │
│     • GET  /api/perception/detections            │
│     • GET  /api/timeline/predictions             │
│     • GET  /api/swarm/network                    │
│     • POST /api/config/update                    │
└──────────────────────────────────────────────────┘
            ↓
┌──────────────────────────────────────────────────┐
│   OMNISIGHT C Core (Native Binary)               │
│   - VDO: Real video from camera                  │
│   - Larod: DLPU ML inference                     │
│   - MQTT: Swarm communication                    │
└──────────────────────────────────────────────────┘
            ↓
┌──────────────────────────────────────────────────┐
│   Interactive Web Dashboard                      │
│   - Live video overlay with detections           │
│   - Timeline visualization (branching futures)   │
│   - Network graph of swarm cameras               │
│   - Real-time event feed                         │
│   - Configuration panel                          │
└──────────────────────────────────────────────────┘
```

## Implementation Plan

### Task 1: Upgrade to Reverse Proxy Architecture

**Estimated Time**: 2 hours
**Dependencies**: None
**Status**: Ready to start

#### Subtasks:

1.1 **Update manifest to schema 1.8.0**
- Change `schemaVersion` from `1.7.3` to `1.8.0`
- Add `reverseProxy` configuration:
  ```json
  "reverseProxy": [
    {
      "apiPath": "api",
      "target": "http://localhost:8080",
      "access": "admin"
    }
  ]
  ```
- Keep `settingPage: "index.html"` for main UI

1.2 **Update version to 0.2.0**
- Increment version: `0.1.9` → `0.2.0`
- Update package naming

1.3 **Test reverse proxy routing**
- Verify `/api/*` routes to Flask server
- Verify root serves HTML interface

**Deliverable**: OMNISIGHT_-_Precognitive_Security_0_2_0_aarch64.eap with reverse proxy

---

### Task 2: Implement Python Flask API Server

**Estimated Time**: 1 day
**Dependencies**: Task 1 complete
**Status**: Ready after Task 1

#### Subtasks:

2.1 **Create Flask application structure**
```
app/
├── server.py              # Main Flask application
├── api/
│   ├── __init__.py
│   ├── perception.py      # Perception endpoints
│   ├── timeline.py        # Timeline endpoints
│   └── swarm.py           # Swarm endpoints
├── static/
│   └── (HTML, CSS, JS)
└── requirements.txt
```

2.2 **Implement REST API endpoints**

**Perception APIs:**
```python
GET /api/perception/status
Response: {
  "enabled": true,
  "fps": 10.2,
  "resolution": "1920x1080",
  "objects_tracked": 3
}

GET /api/perception/detections
Response: {
  "frame_id": 12345,
  "timestamp": "2025-10-22T16:00:00Z",
  "detections": [
    {
      "id": 1,
      "type": "person",
      "bbox": [100, 200, 50, 150],
      "confidence": 0.95,
      "track_id": "T001"
    }
  ]
}
```

**Timeline APIs:**
```python
GET /api/timeline/predictions
Response: {
  "active_timelines": 3,
  "predictions": [
    {
      "timeline_id": "TL001",
      "probability": 0.75,
      "horizon_seconds": 300,
      "events": [
        {
          "type": "TRESPASSING",
          "severity": "medium",
          "time_offset": 45,
          "location": [500, 600],
          "confidence": 0.82
        }
      ]
    }
  ]
}
```

**Swarm APIs:**
```python
GET /api/swarm/network
Response: {
  "local_camera_id": "CAM001",
  "neighbors": [
    {
      "id": "CAM002",
      "distance": 25.5,
      "signal_strength": -45,
      "shared_tracks": 2
    }
  ],
  "network_health": "good"
}
```

2.3 **Implement IPC with C core**
- Shared memory for high-frequency data (detections)
- Named pipes for commands
- JSON files for configuration

2.4 **Add CORS and security headers**

**Deliverable**: Working Flask API server with all endpoints

---

### Task 3: Integrate Axis VDO API

**Estimated Time**: 2-3 days
**Dependencies**: Access to M4228-LVE camera
**Status**: Requires hardware

#### Subtasks:

3.1 **Study VDO API documentation**
- Review official Axis VDO examples
- Understand buffer management
- Learn stream configuration

3.2 **Implement VDO capture module**
File: `src/perception/vdo_capture.c`

```c
// Initialize VDO stream
vdo_error vdo_init(vdo_config_t* config) {
    // Set resolution (1920x1080)
    // Set frame rate (10 fps)
    // Configure format (YUV420)
    // Register frame callback
}

// Frame callback
void on_frame_ready(vdo_buffer_t* buffer) {
    // Convert to format for ML
    // Pass to Larod inference
    // Release buffer
}
```

3.3 **Handle video buffers efficiently**
- Zero-copy where possible
- Proper buffer release
- Memory leak prevention

3.4 **Test on camera hardware**
- Verify frame capture at 10 fps
- Check memory usage
- Monitor CPU usage

**Deliverable**: Real video frames from camera feeding perception engine

---

### Task 4: Integrate Axis Larod API

**Estimated Time**: 3-4 days
**Dependencies**: VDO integration complete, ML model ready
**Status**: Requires hardware + model

#### Subtasks:

4.1 **Select and convert ML model**
- Choose object detection model (YOLO, SSD, etc.)
- Convert to TensorFlow Lite format
- Optimize for ARTPEC-8 DLPU
- Test model accuracy

4.2 **Implement Larod inference module**
File: `src/perception/larod_inference.c`

```c
// Load model via Larod
larod_error larod_init(const char* model_path) {
    // Load TFLite model
    // Configure DLPU acceleration
    // Allocate input/output tensors
}

// Run inference
void larod_infer(uint8_t* frame_data, detection_t* outputs) {
    // Copy frame to input tensor
    // Run inference on DLPU
    // Parse output tensor to detections
    // Apply NMS (non-max suppression)
}
```

4.3 **Optimize for performance**
- Target <20ms inference time
- Minimize memory allocations
- Use DLPU efficiently

4.4 **Integrate with tracker**
- Feed detections to multi-object tracker
- Maintain track IDs
- Handle occlusions

**Deliverable**: Real object detection running on camera DLPU

---

### Task 5: Implement MQTT Swarm Communication

**Estimated Time**: 2-3 days
**Dependencies**: MQTT broker deployed
**Status**: Can start in parallel

#### Subtasks:

5.1 **Set up MQTT broker**
- Deploy Mosquitto or similar
- Configure topics structure:
  ```
  omnisight/cameras/{camera_id}/tracks
  omnisight/cameras/{camera_id}/events
  omnisight/swarm/consensus
  omnisight/swarm/heartbeat
  ```

5.2 **Implement MQTT client module**
File: `src/swarm/mqtt_client.c`

```c
// Connect to broker
mqtt_error mqtt_connect(mqtt_config_t* config) {
    // Connect with QoS 1
    // Set last will message
    // Subscribe to topics
}

// Publish track data
void mqtt_publish_track(track_t* track) {
    // Serialize track to JSON
    // Publish to tracks topic
    // Handle QoS
}

// Message callback
void on_message(const char* topic, const char* payload) {
    // Parse message
    // Update swarm state
    // Trigger consensus if needed
}
```

5.3 **Implement track sharing protocol**
- Serialize/deserialize track data
- Handle coordinate transformations
- Manage global track IDs

5.4 **Test with multiple cameras**
- Deploy to 2-3 M4228-LVE cameras
- Verify track propagation
- Measure latency (<100ms target)

**Deliverable**: Multi-camera swarm network with track sharing

---

### Task 6: Create Interactive Web Dashboard

**Estimated Time**: 2-3 days
**Dependencies**: Flask APIs working
**Status**: Can start after Task 2

#### Subtasks:

6.1 **Design dashboard layout**
```
┌────────────────────────────────────────────────┐
│  OMNISIGHT - Precognitive Security             │
├──────────────────┬─────────────────────────────┤
│                  │  Timeline Predictions       │
│   Live Video     │  ┌─────────────────────┐   │
│   + Detections   │  │ TL1: 75% - Trespass │   │
│                  │  │ TL2: 60% - Loiter   │   │
│                  │  │ TL3: 40% - Normal   │   │
│                  │  └─────────────────────┘   │
├──────────────────┴─────────────────────────────┤
│  Active Objects: 3  |  Threats: 1  |  FPS: 10 │
├────────────────────────────────────────────────┤
│  Swarm Network                                 │
│  [CAM001] ←→ [CAM002] ←→ [CAM003]             │
└────────────────────────────────────────────────┘
```

6.2 **Implement real-time updates**
- WebSocket or SSE for live data
- Update detection overlays
- Refresh timeline predictions
- Update network graph

6.3 **Add visualization components**
- Video canvas with bounding boxes
- Timeline tree visualization
- Network topology graph
- Event log/feed
- Statistics charts

6.4 **Implement configuration panel**
- Enable/disable modules
- Adjust detection thresholds
- Configure swarm settings
- Save/load configurations

**Deliverable**: Production-ready web interface

---

### Task 7: End-to-End Integration Testing

**Estimated Time**: 2-3 days
**Dependencies**: All previous tasks complete
**Status**: Final phase

#### Subtasks:

7.1 **Deploy to M4228-LVE camera**
- Upload complete .eap package
- Verify all modules start
- Check resource usage

7.2 **Test complete workflow**
- Video capture → Detection → Tracking
- Timeline predictions updating
- Swarm communication working
- Web dashboard showing live data

7.3 **Performance validation**
- Inference: <20ms per frame ✓
- FPS: 10 fps stable ✓
- Memory: <512MB ✓
- Network latency: <100ms ✓

7.4 **Bug fixes and optimization**
- Address any issues found
- Optimize bottlenecks
- Improve stability

**Deliverable**: Fully functional Phase 2 system

---

## Technical Specifications

### Hardware Requirements

- **Camera**: Axis M4228-LVE (ARTPEC-8)
- **Firmware**: AXIS OS 12.6.97 or later
- **Memory**: Minimum 512MB available
- **Storage**: 50MB for application + models
- **Network**: Ethernet with MQTT broker access

### Software Stack

**ACAP Package:**
- Schema: 1.8.0
- Architecture: aarch64
- SDK: ACAP Native SDK 3.0

**Languages:**
- C: Core perception, timeline, swarm modules
- Python 3: Flask API server and web interface
- JavaScript: Frontend dashboard (HTML/CSS/JS)

**Key Libraries:**
- VDO API: Video capture
- Larod API: ML inference
- Paho MQTT: Swarm networking
- Flask: Web API framework

**ML Framework:**
- TensorFlow Lite
- Model: TBD (YOLO/SSD/MobileNet)
- Acceleration: ARTPEC-8 DLPU

### API Endpoints Summary

| Endpoint | Method | Purpose |
|----------|--------|---------|
| `/api/perception/status` | GET | Module status and FPS |
| `/api/perception/detections` | GET | Current frame detections |
| `/api/timeline/predictions` | GET | Active timeline predictions |
| `/api/timeline/history` | GET | Past events and outcomes |
| `/api/swarm/network` | GET | Network topology and health |
| `/api/swarm/cameras` | GET | List of swarm cameras |
| `/api/config` | GET | Current configuration |
| `/api/config` | POST | Update configuration |
| `/api/stats` | GET | System statistics |

### File Structure (Phase 2)

```
omnisight/
├── src/                      # C modules
│   ├── perception/
│   │   ├── vdo_capture.c     # ✅ NEW: Real video
│   │   ├── larod_inference.c # ✅ NEW: Real ML
│   │   └── tracker.c         # ✅ NEW: Real tracking
│   ├── timeline/
│   │   └── predictor.c       # ✅ Enhanced algorithms
│   ├── swarm/
│   │   └── mqtt_client.c     # ✅ NEW: Real networking
│   └── main.c                # Updated for IPC
├── app/                      # ✅ NEW: Flask API
│   ├── server.py
│   ├── api/
│   ├── static/
│   └── templates/
├── html/                     # ✅ Enhanced UI
│   ├── index.html
│   ├── js/
│   │   ├── dashboard.js
│   │   ├── timeline.js
│   │   └── network.js
│   └── css/
├── models/                   # ✅ NEW: ML models
│   └── detector.tflite
└── scripts/
    └── build-eap.sh          # Updated for Flask
```

## Success Criteria

Phase 2 will be considered complete when:

1. ✅ Real video frames captured from M4228-LVE at 10 fps
2. ✅ Object detection running on DLPU with <20ms inference
3. ✅ Multi-object tracking maintains IDs across frames
4. ✅ Timeline predictions update based on real detections
5. ✅ MQTT swarm shares tracks between cameras <100ms
6. ✅ Web dashboard displays live data with <1 second latency
7. ✅ All REST APIs return correct real-time data
8. ✅ System runs stable for 24+ hours without crashes
9. ✅ Memory usage stays under 512MB
10. ✅ Complete .eap package deploys successfully

## Risks and Mitigations

### High Risk

**Risk**: DLPU inference too slow for 10 fps target
**Mitigation**:
- Optimize model (quantization, pruning)
- Reduce input resolution
- Skip frames if needed

**Risk**: MQTT latency exceeds 100ms
**Mitigation**:
- Deploy local broker on camera
- Use QoS 0 for real-time data
- Compress messages

### Medium Risk

**Risk**: Memory constraints on camera
**Mitigation**:
- Profile memory usage carefully
- Implement buffer pools
- Reduce model size

**Risk**: VDO/Larod API learning curve
**Mitigation**:
- Study official Axis examples
- Start with simple implementations
- Iterate and optimize

### Low Risk

**Risk**: Flask performance
**Mitigation**:
- Use async where needed
- Cache static responses
- Optimize IPC

## Phase 2 Timeline

**Week 1:**
- Tasks 1-2: Reverse proxy + Flask API (3 days)
- Task 3: VDO integration start (2 days)

**Week 2:**
- Task 3: VDO complete (2 days)
- Task 4: Larod integration (3 days)

**Week 3:**
- Task 5: MQTT swarm (2-3 days)
- Task 6: Web dashboard (2-3 days)
- Task 7: Integration testing (2 days)

**Total**: 2-3 weeks with hardware access

## Next Steps

1. **Immediate**: Begin Task 1 (manifest upgrade to reverse proxy)
2. **Order hardware**: Ensure M4228-LVE camera available for testing
3. **Set up MQTT broker**: Deploy Mosquitto for swarm testing
4. **Source ML model**: Research and select object detection model
5. **Create Phase 2 branch**: `git checkout -b phase-2-integration`

---

**Document Status**: Draft
**Approval Needed**: Yes
**Ready to Start**: Task 1 only (others require hardware)
