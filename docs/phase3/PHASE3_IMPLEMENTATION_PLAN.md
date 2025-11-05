# OMNISIGHT Phase 3 Implementation Plan

**Created**: January 2025
**Status**: Ready to Begin
**Duration**: 4 weeks (Weeks 9-12)
**Hardware**: P3285-LVE (ARTPEC-9) - **VALIDATED**

## Executive Summary

Phase 2 (v0.3.1) successfully validated on P3285-LVE camera with excellent performance:
- ✅ Process stability: Multi-hour operation
- ✅ Resource usage: 10% CPU, 93MB RAM (far below targets)
- ✅ Web server: CivetWeb running on port 8080
- ✅ ACAP packaging: Working deployment pipeline

Phase 3 transitions from **stub/demo** to **real precognitive capabilities** by integrating:
1. **VDO API** - Real video capture from camera sensor
2. **Larod API** - Hardware-accelerated ML inference on DLPU
3. **MQTT 5.0** - Multi-camera swarm intelligence
4. **Real algorithms** - Trajectory prediction, event forecasting

## Verified Hardware Capabilities

**Camera**: AXIS P3285-LVE Dome Camera
**Chipset**: ARTPEC-9 (latest generation with enhanced DLPU)
**Firmware**: AXIS OS 12.6.97
**Serial**: E82725203C16

**Available APIs** (confirmed via ACAP SDK):
- ✅ VDO 4.x - Video capture and streaming
- ✅ Larod 3.x - ML inference with DLPU acceleration
- ✅ MQTT support - Via libmosquitto
- ✅ OpenCV 4.x - Computer vision primitives
- ✅ GLib/GObject - Event loops and data structures

## Architecture Overview

```
┌─────────────────────────────────────────────────────────────┐
│                    OMNISIGHT Phase 3                        │
├─────────────────────────────────────────────────────────────┤
│                                                             │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐     │
│  │  Perception  │  │   Timeline   │  │    Swarm     │     │
│  │              │  │              │  │              │     │
│  │  VDO Capture │  │  Trajectory  │  │ MQTT Client  │     │
│  │  Larod Infer │──│  Prediction  │──│   Federated  │     │
│  │  Tracking    │  │  Event Pred  │  │   Learning   │     │
│  └──────────────┘  └──────────────┘  └──────────────┘     │
│         │                 │                  │              │
│         └─────────────────┴──────────────────┘              │
│                          │                                  │
│                ┌─────────▼─────────┐                        │
│                │  OmniSight Core   │                        │
│                │  Integration      │                        │
│                └─────────┬─────────┘                        │
│                          │                                  │
│                ┌─────────▼─────────┐                        │
│                │   Flask API       │                        │
│                │   REST Endpoints  │                        │
│                └─────────┬─────────┘                        │
│                          │                                  │
│                ┌─────────▼─────────┐                        │
│                │  React Dashboard  │                        │
│                │  Timeline View    │                        │
│                └───────────────────┘                        │
└─────────────────────────────────────────────────────────────┘
```

## Week 9: Perception Module (Priority 1)

### Goals
Replace stub implementations with real hardware APIs for object detection and tracking.

### Task 1.1: VDO Video Capture Integration
**Duration**: 2 days
**Files to Create/Modify**:
- `src/perception/vdo_capture.c` (new)
- `src/perception/vdo_capture.h` (new)
- `src/perception/perception.c` (modify to use VDO)

**Reference Example**: `/acap-native-sdk-examples/vdo-larod/app/imgprovider.c`

**Implementation Steps**:

1. **VDO Stream Initialization**
```c
// vdo_capture.h
typedef struct {
    VdoStream* stream;
    VdoChannel* channel;
    unsigned int width;
    unsigned int height;
    VdoFormat format;
    double framerate;
    VdoBuffer* current_buffer;
    bool running;
    pthread_mutex_t buffer_mutex;
} VdoCapture;

VdoCapture* vdo_capture_init(unsigned int width, unsigned int height, double fps);
VdoBuffer* vdo_capture_get_frame(VdoCapture* capture, GError** error);
void vdo_capture_release_frame(VdoCapture* capture, VdoBuffer* buffer);
void vdo_capture_destroy(VdoCapture* capture);
```

2. **Stream Configuration**
- Use `VDO_FORMAT_YUV` for Larod compatibility
- Configure for ARTPEC-9 DLPU input requirements
- Set framerate to 30 FPS (adjustable based on inference time)
- Use channel 1 (primary sensor)

3. **Buffer Management**
- Implement buffer acquisition with timeout
- Handle rotation changes gracefully
- Flush stale frames before inference
- Proper cleanup and reference counting

**Success Criteria**:
- ✅ VDO stream starts successfully
- ✅ Frames arrive at 30 FPS
- ✅ YUV buffers have correct dimensions
- ✅ No memory leaks (24-hour test)
- ✅ Handles camera rotation/restart

### Task 1.2: Larod ML Inference Integration
**Duration**: 2-3 days
**Files to Create/Modify**:
- `src/perception/larod_inference.c` (new)
- `src/perception/larod_inference.h` (new)
- `src/perception/perception.c` (modify to use Larod)

**Reference Example**: `/acap-native-sdk-examples/vdo-larod/app/model.c`

**Implementation Steps**:

1. **Larod Context Initialization**
```c
// larod_inference.h
typedef struct {
    larodConnection* conn;
    larodModel* model;
    larodMap* inputMap;
    larodMap* outputMap;
    larodTensor** inputTensors;
    larodTensor** outputTensors;
    size_t numInputs;
    size_t numOutputs;

    // Cropping for aspect ratio
    unsigned int clip_x;
    unsigned int clip_y;
    unsigned int clip_w;
    unsigned int clip_h;

    // Performance tracking
    double avg_inference_ms;
} LarodInference;

LarodInference* larod_inference_init(const char* model_path, const char* device_name);
bool larod_inference_run(LarodInference* inf, VdoBuffer* vdo_buf, DetectedObject* objs, uint32_t* count);
void larod_inference_destroy(LarodInference* inference);
```

2. **Model Loading**
- Use SSD MobileNet v2 or YOLO for object detection
- Target device: `"dlpu"` (ARTPEC-9 DLPU chip)
- Model format: TensorFlow Lite (.tflite)
- Input: 300x300 RGB or 416x416 RGB
- Output: Bounding boxes + class IDs + confidence scores

3. **Preprocessing**
- Convert YUV to RGB if needed (Larod can do this)
- Center crop to maintain aspect ratio
- Normalize pixel values (0-255 → 0.0-1.0)

4. **Postprocessing**
```c
// Parse Larod output tensors to DetectedObject format
static void parse_detection_output(
    larodTensor** output_tensors,
    size_t num_outputs,
    DetectedObject* objects,
    uint32_t* num_objects
) {
    // Typical object detection model outputs:
    // tensor[0]: Bounding boxes [N, 4] (y1, x1, y2, x2 normalized)
    // tensor[1]: Class IDs [N]
    // tensor[2]: Confidence scores [N]
    // tensor[3]: Number of detections [1]

    // Filter by confidence threshold (>0.5)
    // Convert normalized coords to pixel coords
    // Assign unique IDs
}
```

**Success Criteria**:
- ✅ Larod connects to DLPU successfully
- ✅ Model loads without errors
- ✅ Inference runs at <20ms per frame
- ✅ Detections have >80% precision on test footage
- ✅ CPU usage stays <30%

### Task 1.3: Object Tracking Implementation
**Duration**: 3 days
**Files to Create/Modify**:
- `src/perception/tracker.c` (new)
- `src/perception/tracker.h` (new)
- `src/perception/perception.c` (integrate tracker)

**Reference Example**: `/acap-native-sdk-examples/using-opencv/` (for Kalman filter)

**Implementation Steps**:

1. **SORT (Simple Online and Realtime Tracking) Algorithm**
```c
// tracker.h
typedef struct {
    uint32_t id;
    BoundingBox bbox;
    float velocity_x;
    float velocity_y;
    uint32_t age;
    uint32_t hits;
    uint32_t time_since_update;

    // Kalman filter state
    float state[7];      // [x, y, s, r, vx, vy, vs]
    float covariance[7][7];

    // Re-ID features
    float features[128];
} Track;

typedef struct {
    Track* tracks;
    uint32_t num_tracks;
    uint32_t max_tracks;
    uint32_t next_id;
} Tracker;

Tracker* tracker_init(uint32_t max_tracks);
void tracker_update(Tracker* t, DetectedObject* dets, uint32_t num_dets);
TrackedObject* tracker_get_active_tracks(Tracker* t, uint32_t* count);
void tracker_destroy(Tracker* t);
```

2. **Kalman Filter for Motion Prediction**
- State vector: [center_x, center_y, scale, aspect_ratio, vx, vy, vs]
- Predict next position based on constant velocity model
- Update with new detections via Kalman gain

3. **Hungarian Algorithm for Association**
- Calculate IoU (Intersection over Union) matrix
- Use Hungarian algorithm to match detections to tracks
- Threshold: IoU > 0.3 for valid match

4. **Track Lifecycle Management**
- New track: 3 consecutive hits to confirm
- Lost track: 30 frames without detection → delete
- Track ID persistence across occlusions

**Success Criteria**:
- ✅ Track IDs persist across frames
- ✅ Handles occlusions (5-second gaps)
- ✅ Re-ID accuracy >85% on test footage
- ✅ No track ID switching (stable assignments)

### Task 1.4: Behavior Analysis Implementation
**Duration**: 2 days
**Files to Create/Modify**:
- `src/perception/behavior.c` (new)
- `src/perception/behavior.h` (new)

**Implementation Steps**:

1. **Behavior Detection Algorithms**
```c
// behavior.h
typedef enum {
    BEHAVIOR_NORMAL = 0x00,
    BEHAVIOR_LOITERING = 0x01,
    BEHAVIOR_RUNNING = 0x02,
    BEHAVIOR_UNUSUAL_MOVEMENT = 0x04,
    BEHAVIOR_ZONE_VIOLATION = 0x08
} BehaviorFlags;

void analyze_behavior(TrackedObject* track, const TrackHistory* history, BehaviorFlags* behaviors, float* threat_score);
```

2. **Loitering Detection**
- Track dwell time in 2x2 meter grid cells
- Threshold: >30 seconds in same cell → loitering
- Velocity < 0.5 m/s required

3. **Running Detection**
- Calculate velocity from position history
- Threshold: velocity > 3.0 m/s → running
- Sudden acceleration detection

4. **Threat Scoring**
```c
float calculate_threat_score(const TrackedObject* track, BehaviorFlags behaviors) {
    float score = 0.0f;

    if (behaviors & BEHAVIOR_LOITERING) score += 0.3f;
    if (behaviors & BEHAVIOR_RUNNING) score += 0.4f;
    if (behaviors & BEHAVIOR_UNUSUAL_MOVEMENT) score += 0.5f;
    if (behaviors & BEHAVIOR_ZONE_VIOLATION) score += 0.7f;

    // Time-based escalation
    float dwell_factor = track->last_seen_ms - track->first_seen_ms;
    score += dwell_factor / 120000.0f;  // +0.5 after 2 minutes

    return fminf(score, 1.0f);
}
```

**Success Criteria**:
- ✅ Loitering detected within 30 seconds
- ✅ Running accurately classified (>90%)
- ✅ False positive rate <5%
- ✅ Threat scores calibrated (0.0-1.0 range)

---

## Week 10: Timeline Module (Priority 2)

### Goals
Implement real trajectory prediction and event forecasting algorithms.

### Task 2.1: Trajectory Prediction with Kalman Filter
**Duration**: 2 days
**Files to Create/Modify**:
- `src/timeline/trajectory_predictor.c` (new)
- `src/timeline/trajectory_predictor.h` (new)
- `src/timeline/timeline.c` (integrate predictor)

**Implementation Steps**:

1. **Kalman Filter State Representation**
```c
// trajectory_predictor.h
typedef struct {
    float x, y;          // Position (meters)
    float vx, vy;        // Velocity (m/s)
    float ax, ay;        // Acceleration (m/s²)
    float covariance[6][6];  // State uncertainty
} KalmanState;

typedef struct {
    float x, y;
    float timestamp_sec;
    float probability;
} TrajectoryPoint;

void predict_trajectory(
    const TrackedObject* track,
    float horizon_sec,
    TrajectoryPoint* points,
    uint32_t* num_points
);
```

2. **Extended Kalman Filter (EKF)**
- State: [x, y, vx, vy, ax, ay]
- Prediction horizon: 5-30 seconds
- Account for human motion models (turning, stopping)

3. **Collision Probability Calculation**
```c
float calculate_collision_probability(
    const TrajectoryPoint* traj1,
    const TrajectoryPoint* traj2,
    uint32_t num_points
) {
    float min_distance = FLT_MAX;
    float collision_threshold = 0.5f;  // 0.5 meters

    for (uint32_t i = 0; i < num_points; i++) {
        float dx = traj1[i].x - traj2[i].x;
        float dy = traj1[i].y - traj2[i].y;
        float dist = sqrtf(dx*dx + dy*dy);

        if (dist < min_distance) min_distance = dist;
    }

    if (min_distance < collision_threshold) {
        return 1.0f - (min_distance / collision_threshold);
    }
    return 0.0f;
}
```

**Success Criteria**:
- ✅ Trajectory prediction ±10% error over 5 seconds
- ✅ Handles turning and stopping behaviors
- ✅ Probability calibrated to real outcomes
- ✅ Performance: <5ms per tracked object

### Task 2.2: Event Prediction Engine
**Duration**: 3 days
**Files to Create/Modify**:
- `src/timeline/event_predictor.c` (new)
- `src/timeline/event_predictor.h` (new)

**Implementation Steps**:

1. **Event Classification**
```c
// event_predictor.h
typedef enum {
    EVENT_THEFT = 0,
    EVENT_TRESPASSING,
    EVENT_SUSPICIOUS_BEHAVIOR,
    EVENT_LOITERING,
    EVENT_VIOLENCE,
    EVENT_COLLISION
} EventType;

typedef struct {
    EventType type;
    float probability;
    uint64_t predicted_time_ms;
    float severity;  // 0.0-1.0
    char description[256];
    uint32_t involved_track_ids[8];
    uint32_t num_involved_tracks;
} PredictedEvent;
```

2. **Pattern Recognition**
```c
// Trespassing detection
bool predict_trespassing(const TrackedObject* track, const Zone* restricted_zones, uint32_t num_zones) {
    // Project trajectory into restricted zones
    TrajectoryPoint predicted[30];
    uint32_t num_points;
    predict_trajectory(track, 10.0f, predicted, &num_points);

    for (uint32_t i = 0; i < num_points; i++) {
        for (uint32_t j = 0; j < num_zones; j++) {
            if (point_in_zone(&predicted[i], &restricted_zones[j])) {
                return true;
            }
        }
    }
    return false;
}
```

3. **Timeline Branching**
```c
typedef struct {
    uint32_t branch_id;
    float probability;
    PredictedEvent events[10];
    uint32_t num_events;
    uint64_t divergence_time_ms;
} TimelineBranch;

void generate_timeline_branches(
    const TrackedObject* tracks,
    uint32_t num_tracks,
    TimelineBranch* branches,
    uint32_t* num_branches
);
```

**Success Criteria**:
- ✅ 5 event types detected
- ✅ Prediction accuracy >70% (verified on test footage)
- ✅ 3-5 timeline branches per scenario
- ✅ Intervention points identified

---

## Week 11: Swarm Module (Priority 3)

### Goals
Implement multi-camera communication and federated learning.

### Task 3.1: MQTT 5.0 Client Integration
**Duration**: 2 days
**Files to Create/Modify**:
- `src/swarm/mqtt_client.c` (complete existing)
- `src/swarm/mqtt_client.h` (complete existing)
- `src/swarm/swarm.c` (integrate MQTT)

**Reference Example**: `/acap-native-sdk-examples/message-broker/`

**Implementation Steps**:

1. **MQTT Client Setup**
```c
// mqtt_client.h
typedef struct {
    struct mosquitto* mosq;
    char camera_id[64];
    char broker_host[256];
    int broker_port;
    bool connected;

    SwarmMessageCallback on_track_received;
    SwarmMessageCallback on_event_received;
    void* user_data;

    pthread_mutex_t publish_mutex;
} MqttClient;

MqttClient* mqtt_client_init(const char* camera_id, const char* broker, int port);
bool mqtt_client_connect(MqttClient* client);
bool mqtt_publish_track(MqttClient* client, const TrackedObject* track);
bool mqtt_publish_event(MqttClient* client, const PredictedEvent* event);
void mqtt_client_destroy(MqttClient* client);
```

2. **Topic Structure**
```
omnisight/swarm/tracks/{camera_id}/{track_id}
omnisight/swarm/events/{camera_id}/{event_id}
omnisight/swarm/models/{camera_id}/weights
omnisight/swarm/consensus/{event_id}
```

3. **Message Serialization (JSON)**
```c
// Track message format
{
    "camera_id": "P3285_E82725203C16",
    "track_id": 1234,
    "global_position": {"x": 10.5, "y": 3.2},
    "velocity": {"x": 1.2, "y": 0.0},
    "class": "person",
    "confidence": 0.92,
    "timestamp": 1736204567890
}
```

4. **QoS and Reliability**
- Track updates: QoS 0 (at most once, real-time)
- Event predictions: QoS 1 (at least once)
- Model updates: QoS 2 (exactly once)
- Clean session: false (persistent)
- Keepalive: 60 seconds

**Success Criteria**:
- ✅ MQTT connects to broker successfully
- ✅ Track updates published at 10 Hz
- ✅ Events received from 3+ cameras
- ✅ Handles broker disconnect/reconnect
- ✅ Latency <100ms for local network

### Task 3.2: Federated Learning Framework
**Duration**: 3 days
**Files to Create/Modify**:
- `src/swarm/federated_learning.c` (new)
- `src/swarm/federated_learning.h` (new)

**Implementation Steps**:

1. **Model Weight Representation**
```c
// federated_learning.h
typedef struct {
    float* weights;
    size_t num_weights;
    uint32_t version;
    uint64_t timestamp_ms;
    char camera_id[64];
} ModelWeights;

typedef struct {
    ModelWeights local_model;
    ModelWeights aggregated_model;

    float* gradients;
    size_t num_gradients;

    // Aggregation buffer
    float** neighbor_gradients;
    uint32_t num_neighbors;

    pthread_mutex_t update_mutex;
} FederatedLearner;
```

2. **Gradient Extraction**
```c
bool extract_gradients(
    const ModelWeights* old_weights,
    const ModelWeights* new_weights,
    float* gradients
) {
    for (size_t i = 0; i < old_weights->num_weights; i++) {
        gradients[i] = new_weights->weights[i] - old_weights->weights[i];
    }
    return true;
}
```

3. **Federated Averaging (FedAvg)**
```c
bool aggregate_gradients(
    const float** neighbor_gradients,
    size_t num_neighbors,
    size_t num_weights,
    float* aggregated_gradients
) {
    // Simple averaging (can add weighted averaging later)
    memset(aggregated_gradients, 0, num_weights * sizeof(float));

    for (size_t n = 0; n < num_neighbors; n++) {
        for (size_t i = 0; i < num_weights; i++) {
            aggregated_gradients[i] += neighbor_gradients[n][i];
        }
    }

    float scale = 1.0f / (float)num_neighbors;
    for (size_t i = 0; i < num_weights; i++) {
        aggregated_gradients[i] *= scale;
    }

    return true;
}
```

4. **Model Update Distribution**
```c
bool update_local_model(
    FederatedLearner* learner,
    const float* aggregated_gradients,
    float learning_rate
) {
    pthread_mutex_lock(&learner->update_mutex);

    for (size_t i = 0; i < learner->local_model.num_weights; i++) {
        learner->local_model.weights[i] += learning_rate * aggregated_gradients[i];
    }

    learner->local_model.version++;
    learner->local_model.timestamp_ms = get_current_time_ms();

    pthread_mutex_unlock(&learner->update_mutex);
    return true;
}
```

**Success Criteria**:
- ✅ Model weights shared across 3+ cameras
- ✅ Gradients aggregated correctly
- ✅ Detection accuracy improves over time
- ✅ Privacy preserved (no raw data shared)
- ✅ Convergence within 100 updates

---

## Week 12: Testing and Optimization

### Goals
Comprehensive testing, performance optimization, and production readiness.

### Task 4.1: Performance Profiling and Optimization
**Duration**: 2 days

**Tools and Methods**:

1. **CPU Profiling**
```bash
# On-camera profiling
ssh root@192.168.1.100
perf record -g -p $(pidof omnisight) -- sleep 60
perf report

# Identify hotspots
perf top -p $(pidof omnisight)
```

2. **Memory Leak Detection**
```bash
# Valgrind (in Docker dev environment)
valgrind --leak-check=full --track-origins=yes ./omnisight --demo

# On-camera monitoring
watch -n 1 'cat /proc/$(pidof omnisight)/status | grep -E "VmSize|VmRSS"'
```

3. **Larod Inference Optimization**
- Ensure DLPU chip is used (not CPU fallback)
- Use INT8 quantized models (4x faster)
- Batch inference if possible (multiple detections per frame)
- Optimize preprocessing (YUV→RGB conversion)

4. **Threading Optimization**
- Verify pipeline parallelism (VDO capture + Larod inference + tracking)
- Use lock-free queues for inter-thread communication
- Profile mutex contention with `perf lock`

**Performance Targets** (updated based on v0.3.1 results):

| Metric | Current (v0.3.1) | Phase 3 Target | Notes |
|--------|------------------|----------------|-------|
| CPU Usage | 10% | <30% | Includes ML inference |
| Memory | 93 MB | <512 MB | Includes model weights |
| Inference Time | N/A (stub) | <20ms | Per frame on DLPU |
| Framerate | N/A | 30 FPS | Stable at 1920x1080 |
| Network BW | N/A | <500 Kbps | Per camera (MQTT) |
| Startup Time | <2s | <5s | Including model load |

**Success Criteria**:
- ✅ All targets met or exceeded
- ✅ No memory leaks (24-hour run)
- ✅ No CPU bottlenecks
- ✅ Stable 30 FPS under load

### Task 4.2: Integration Testing
**Duration**: 3 days

**Test Scenarios**:

1. **Single Camera Operation**
```bash
# Test plan
1. Deploy v0.4.0 to P3285-LVE
2. Run for 30 minutes with live video
3. Verify:
   - Object detection working
   - Tracking IDs persistent
   - Timeline predictions generated
   - API endpoints returning data
   - Web dashboard updates
```

2. **Multi-Camera Swarm** (requires 3+ cameras)
```bash
# Swarm test plan
1. Deploy to 3 cameras in overlapping FOV
2. Walk through coverage area
3. Verify:
   - Track handoff between cameras
   - Global IDs assigned correctly
   - Events shared via MQTT
   - Consensus voting works
   - Federated learning updates
```

3. **Event Prediction Accuracy**
```bash
# Ground truth validation
1. Record 1 hour of test footage
2. Manually label events (loitering, running, etc.)
3. Run OMNISIGHT on footage
4. Calculate metrics:
   - Precision: TP / (TP + FP)
   - Recall: TP / (TP + FN)
   - F1 Score: 2 * (P * R) / (P + R)
   - Target: >70% F1 score
```

4. **Network Resilience**
```bash
# MQTT fault tolerance
1. Start swarm with MQTT broker
2. Disconnect broker during operation
3. Verify:
   - Local operation continues
   - Reconnect after 60 seconds
   - Queued messages delivered
   - No data loss
```

5. **Memory Leak Testing**
```bash
# 24-hour stress test
1. Deploy to camera
2. Enable continuous operation
3. Monitor every hour:
   - Memory usage (RSS, VSZ)
   - File descriptors
   - Thread count
   - CPU usage
4. Verify: No growth over 24 hours
```

**Test Infrastructure**:

```python
# tests/integration_test.py
import requests
import time
import json

class OmnisightTester:
    def __init__(self, camera_ip):
        self.base_url = f"http://{camera_ip}:8080/api"

    def test_perception(self):
        """Test perception endpoint returns detections"""
        resp = requests.get(f"{self.base_url}/perception/status")
        assert resp.status_code == 200
        data = resp.json()
        assert data['active_tracks'] >= 0
        assert 'avg_inference_ms' in data

    def test_timeline(self):
        """Test timeline predictions"""
        resp = requests.get(f"{self.base_url}/timeline/futures")
        assert resp.status_code == 200
        data = resp.json()
        assert 'timelines' in data
        assert len(data['timelines']) > 0

    def test_swarm(self):
        """Test swarm communication"""
        resp = requests.get(f"{self.base_url}/swarm/status")
        assert resp.status_code == 200
        data = resp.json()
        assert 'neighbor_count' in data

# Run tests
if __name__ == "__main__":
    tester = OmnisightTester("192.168.1.100")
    tester.test_perception()
    tester.test_timeline()
    tester.test_swarm()
    print("✅ All integration tests passed!")
```

**Success Criteria**:
- ✅ All test scenarios pass
- ✅ Event prediction accuracy >70%
- ✅ Multi-camera handoff <2 second latency
- ✅ Network resilience validated
- ✅ 24-hour stability confirmed

---

## Build System Updates

### CMakeLists.txt Modifications

**Add Larod and VDO dependencies**:

```cmake
# Phase 3: Hardware API libraries
find_package(PkgConfig REQUIRED)

pkg_check_modules(VDO REQUIRED vdo-stream vdo-types vdo-frame vdo-buffer)
pkg_check_modules(LAROD REQUIRED larod)
pkg_check_modules(GLIB REQUIRED glib-2.0 gobject-2.0)
pkg_check_modules(MOSQUITTO REQUIRED libmosquitto)

include_directories(
    ${VDO_INCLUDE_DIRS}
    ${LAROD_INCLUDE_DIRS}
    ${GLIB_INCLUDE_DIRS}
    ${MOSQUITTO_INCLUDE_DIRS}
)

target_link_libraries(omnisight_perception
    PRIVATE
        ${VDO_LIBRARIES}
        ${LAROD_LIBRARIES}
        ${GLIB_LIBRARIES}
)

target_link_libraries(omnisight_swarm
    PRIVATE
        ${MOSQUITTO_LIBRARIES}
)
```

### New Build Script: `scripts/build-phase3-eap.sh`

```bash
#!/bin/bash
# Build OMNISIGHT Phase 3 with hardware APIs

set -e

echo "Building OMNISIGHT Phase 3 (Hardware Integration)..."

# Create build directory
rm -rf build-phase3
mkdir -p build-phase3
cd build-phase3

# Cross-compile for ARTPEC-9
export ARCH=aarch64
export SDK=/opt/axis/acapsdk

cmake .. \
    -DCMAKE_TOOLCHAIN_FILE=../acap-toolchain.cmake \
    -DCMAKE_BUILD_TYPE=Release \
    -DENABLE_HARDWARE_APIS=ON \
    -DENABLE_VDO=ON \
    -DENABLE_LAROD=ON \
    -DENABLE_MQTT=ON

make -j$(nproc)

echo "✓ Phase 3 binary built: build-phase3/omnisight"

# Package as ACAP
cd ..
./scripts/package-acap.sh build-phase3/omnisight

echo "✓ ACAP package: OMNISIGHT_Hardware_0_4_0_aarch64.eap"
```

### manifest.json Updates

```json
{
  "schemaVersion": "1.8.0",
  "acapPackageConf": {
    "setup": {
      "appName": "OMNISIGHT_Hardware",
      "vendor": "OMNISIGHT",
      "version": "0.4.0",
      "friendlyName": "OMNISIGHT - Precognitive Security (Hardware)",
      "embeddedSdkVersion": "3.0",
      "runMode": "respawn",
      "user": {
        "username": "acap-omnisight",
        "group": "sdk"
      }
    },
    "configuration": {
      "settingPage": "html/settings.html",
      "httpConfig": [
        {
          "name": "api",
          "type": "transferEncoding",
          "access": "admin"
        }
      ]
    },
    "resources": {
      "memory": {
        "ram": 512
      }
    }
  }
}
```

---

## Testing Checklist

### Before Deployment
- [ ] Code compiled successfully in Docker
- [ ] Unit tests pass (if implemented)
- [ ] Static analysis clean (cppcheck, clang-tidy)
- [ ] Memory profiling clean (valgrind)
- [ ] ACAP package builds without errors

### After Deployment to Camera
- [ ] Application starts successfully
- [ ] VDO stream initializes (check syslog)
- [ ] Larod model loads (check syslog)
- [ ] MQTT connects to broker
- [ ] Web interface accessible
- [ ] API endpoints return data
- [ ] No errors in syslog

### Performance Validation
- [ ] CPU usage <30%
- [ ] Memory usage <512MB
- [ ] Inference time <20ms
- [ ] Framerate steady at 30 FPS
- [ ] Network bandwidth <500 Kbps

### Functional Validation
- [ ] Objects detected correctly
- [ ] Tracking IDs persistent
- [ ] Behaviors classified (loitering, running)
- [ ] Timeline predictions generated
- [ ] Events predicted accurately
- [ ] Threat scores calibrated

### Multi-Camera Validation (if available)
- [ ] Tracks shared via MQTT
- [ ] Events propagated to neighbors
- [ ] Consensus voting operational
- [ ] Model updates distributed
- [ ] Global IDs assigned correctly

---

## Risk Mitigation

### High-Risk Items

**1. Larod Model Compatibility**
- **Risk**: TensorFlow Lite model not compatible with ARTPEC-9 DLPU
- **Mitigation**:
  - Use Axis-provided model conversion tools
  - Test with reference models from acap-computer-vision-sdk-examples
  - Have CPU fallback option

**2. Performance Targets**
- **Risk**: Inference >20ms, can't maintain 30 FPS
- **Mitigation**:
  - Use INT8 quantized models
  - Reduce input resolution (300x300 instead of 416x416)
  - Adjust framerate dynamically based on inference time

**3. MQTT Broker Availability**
- **Risk**: No MQTT broker available for testing
- **Mitigation**:
  - Use local Mosquitto broker in Docker
  - Implement local-only mode (no swarm)
  - Test with public MQTT brokers (test.mosquitto.org)

### Medium-Risk Items

**4. Memory Constraints**
- **Risk**: Model + tracking exceeds 512MB limit
- **Mitigation**:
  - Use smaller models (MobileNet v2 instead of ResNet)
  - Limit max tracked objects to 50
  - Implement object eviction policies

**5. Multi-Camera Testing**
- **Risk**: Only 1 camera available
- **Mitigation**:
  - Simulate multiple cameras with mock MQTT publishers
  - Focus on single-camera operation first
  - Request additional cameras for Week 11

---

## Deliverables

### Code
- ✅ VDO capture module (`vdo_capture.c/h`)
- ✅ Larod inference module (`larod_inference.c/h`)
- ✅ Object tracker (`tracker.c/h`)
- ✅ Behavior analyzer (`behavior.c/h`)
- ✅ Trajectory predictor (`trajectory_predictor.c/h`)
- ✅ Event predictor (`event_predictor.c/h`)
- ✅ MQTT client (`mqtt_client.c/h`)
- ✅ Federated learner (`federated_learning.c/h`)

### Documentation
- ✅ Phase 3 implementation plan (this document)
- ✅ API documentation updates
- ✅ Performance benchmark report
- ✅ Integration test results
- ✅ Deployment guide updates

### Build Artifacts
- ✅ `OMNISIGHT_Hardware_0_4_0_aarch64.eap` (ACAP package)
- ✅ Build scripts updated
- ✅ Docker image with Phase 3 dependencies
- ✅ Test suite

### Test Results
- ✅ Single-camera test report (30 minutes)
- ✅ Performance profiling results
- ✅ Memory leak test results (24 hours)
- ✅ Event prediction accuracy report
- ✅ Multi-camera test report (if available)

---

## Next Steps After Phase 3

### Phase 4: Scale and Polish (Weeks 13-16)
1. **Performance Optimization**
   - Multi-threaded inference pipeline
   - GPU acceleration (if available)
   - Model compression (pruning, quantization)

2. **Enhanced Features**
   - Face detection and tracking
   - License plate recognition
   - Audio analysis (gunshot detection)
   - Anomaly detection (unusual events)

3. **User Interface**
   - Real-time timeline visualization
   - Interactive zone configuration
   - Alert management dashboard
   - Video playback with overlays

4. **Enterprise Features**
   - Multi-site deployment
   - Centralized monitoring
   - Alert routing and escalation
   - Integration with VMS (Video Management Systems)

---

## Appendix: Reference Documentation

### Official Axis ACAP Documentation
- VDO API: https://axiscommunications.github.io/acap-api/vdo/html/
- Larod API: https://axiscommunications.github.io/acap-api/larod/html/
- ACAP SDK: https://github.com/AxisCommunications/acap-native-sdk

### Local Examples
- VDO + Larod: `/acap-native-sdk-examples/vdo-larod/`
- Object Detection: `/acap-native-sdk-examples/object-detection/`
- OpenCV: `/acap-native-sdk-examples/using-opencv/`
- MQTT: `/acap-native-sdk-examples/message-broker/`

### OMNISIGHT Skills
- `/.claude/skills/omnisight-phase-3-integration.md`
- `/.claude/skills/axis-acap-development.md`
- `/.claude/skills/axis-examples-reference.md`

### Hardware Specifications
- P3285-LVE Datasheet: https://www.axis.com/products/axis-p3285-lve
- ARTPEC-9 Specifications: (Axis confidential)

---

**Ready to begin Phase 3 implementation!** 🚀

Let's build real precognitive capabilities!
