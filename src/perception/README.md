# OMNISIGHT Perception Engine

The perception engine is the computer vision foundation of OMNISIGHT, responsible for detecting, tracking, and analyzing objects in real-time video streams from Axis cameras.

## Overview

The perception engine processes video at 10 FPS, detecting and tracking objects using DLPU-accelerated deep learning models. It provides the foundation for Timeline Threading™ by:

1. **Capturing video** from camera via VDO API
2. **Detecting objects** using TensorFlow Lite models on ARTPEC-8 DLPU
3. **Tracking objects** across frames to maintain identity
4. **Analyzing behaviors** to identify suspicious activities
5. **Calculating threat scores** for Timeline Threading

## Architecture

```
┌─────────────────────────────────────────────────────┐
│                Perception Engine                     │
├─────────────────────────────────────────────────────┤
│                                                       │
│  ┌──────────────┐    ┌──────────────┐              │
│  │ VDO Capture  │───→│   Larod ML   │              │
│  │   Module     │    │  Inference   │              │
│  └──────────────┘    └──────────────┘              │
│         │                    │                       │
│         └────────┬───────────┘                       │
│                  ↓                                    │
│          ┌──────────────┐                           │
│          │   Tracker    │                           │
│          │   (IoU +     │                           │
│          │   Kalman)    │                           │
│          └──────────────┘                           │
│                  │                                    │
│                  ↓                                    │
│          ┌──────────────┐                           │
│          │  Behavior    │                           │
│          │  Analyzer    │                           │
│          └──────────────┘                           │
│                  │                                    │
│                  ↓                                    │
│         TrackedObject[]                              │
│         + Behaviors                                   │
│         + Threat Scores                              │
└─────────────────────────────────────────────────────┘
```

## Components

### 1. VDO Capture (`vdo_capture.h/c`)
Handles video stream capture from the camera.

**Features:**
- Hardware-accelerated format conversion
- Efficient buffer pooling
- Zero-copy where possible
- Automatic frame rate control

**Usage:**
```c
VdoCaptureConfig config = {
    .width = 416,
    .height = 416,
    .framerate = 10,
    .format = VDO_FORMAT_RGB,
    .buffer_count = 4,
    .use_hardware_accel = true
};

VdoCapture* capture = vdo_capture_init(&config);
vdo_capture_start(capture, frame_callback, user_data);
```

### 2. Larod Inference (`larod_inference.h/c`)
ML inference wrapper for ARTPEC-8 DLPU.

**Features:**
- TensorFlow Lite model loading
- Automatic DLPU utilization
- INT8 quantization support
- Preprocessing pipeline
- Async inference support

**Model Requirements (ARTPEC-8):**
- INT8 per-tensor quantization
- Input size multiple of 32 (e.g., 416x416, 608x608)
- Batch size = 1
- TFLite format

**Usage:**
```c
LarodInferenceConfig config = {
    .model_path = "/opt/app/models/detection.tflite",
    .chip = LAROD_CHIP_DLPU,
    .num_threads = 4,
    .async_mode = false
};

LarodInference* inference = larod_inference_init(&config);
larod_inference_run(inference, inputs, 1, outputs, 2);
```

### 3. Tracker (`tracker.h/c`)
Multi-object tracking across frames.

**Algorithm:**
- IoU-based matching with Hungarian algorithm
- Kalman filter for motion prediction
- Feature similarity for re-identification
- Track lifecycle management

**Usage:**
```c
TrackerConfig config = {
    .iou_threshold = 0.3,
    .max_age = 30,
    .min_hits = 3,
    .max_tracks = 50,
    .use_kalman_filter = true
};

Tracker* tracker = tracker_init(&config);
uint32_t num_tracks = tracker_update(tracker, detections, num_detections, tracks, 50);
```

### 4. Behavior Analyzer (`behavior.h/c`)
Detects suspicious behaviors and calculates threat scores.

**Behaviors Detected:**
- **Loitering**: Staying in area beyond threshold
- **Running**: Velocity exceeds threshold
- **Repeated Passes**: Multiple passes through area
- **Extended Observation**: Prolonged gaze at specific area
- **Concealing**: Suspicious body movements (future)

**Usage:**
```c
BehaviorConfig config = {
    .loitering_threshold_ms = 30000,  // 30 seconds
    .running_velocity_threshold = 100.0,
    .repeated_passes_count = 3,
    .repeated_passes_window_ms = 60000
};

BehaviorAnalyzer* analyzer = behavior_init(&config);
behavior_analyze(analyzer, tracks, num_tracks);
```

### 5. Perception Engine (`perception.h/c`)
Main orchestrator tying everything together.

**Usage:**
```c
PerceptionConfig config = {
    .frame_width = 416,
    .frame_height = 416,
    .target_fps = 10,
    .model_path = "/opt/app/models/detection.tflite",
    .use_dlpu = true,
    .detection_threshold = 0.5,
    .max_tracked_objects = 50
};

PerceptionEngine* engine = perception_init(&config);
perception_start(engine, perception_callback, user_data);

// In callback:
void perception_callback(const TrackedObject* objects, uint32_t count, void* data) {
    for (uint32_t i = 0; i < count; i++) {
        printf("Track %u: class=%d, threat=%.2f, behaviors=0x%x\n",
               objects[i].track_id,
               objects[i].class_id,
               objects[i].threat_score,
               objects[i].behaviors);
    }
}
```

## Performance Targets

| Metric | Target | Status |
|--------|--------|--------|
| Inference latency | <20ms | TBD |
| Throughput | 10 FPS | TBD |
| Detection accuracy | 95%+ | TBD |
| Tracking accuracy | 90%+ | TBD |
| Memory usage | <256MB | TBD |
| CPU usage | <20% | TBD |

## Building

```bash
# From project root
mkdir build && cd build
cmake ..
make omnisight_perception

# With testing
cmake -DBUILD_TESTING=ON ..
make
make test
```

## Testing

```bash
# Unit tests
cd tests
./test_perception

# Integration test with camera
./test_perception_camera --camera-ip 192.168.1.100
```

## Models

The perception engine uses the following models:

### Object Detection
- **Model**: EfficientNet-B4 (modified for ARTPEC-8)
- **Input**: 416x416x3 RGB (INT8)
- **Output**:
  - Bounding boxes: [N, 4] (x, y, w, h)
  - Classes: [N, 6] (person, vehicle, face, package, animal, other)
  - Scores: [N, 1]
- **Location**: `models/perception/efficientnet_b4_detection.tflite`

### Feature Extraction
- **Model**: MobileNet-V3-Small
- **Input**: Cropped object (128x128x3 RGB)
- **Output**: Feature vector [128]
- **Location**: `models/perception/mobilenet_v3_features.tflite`

## Integration with Timeline Threading™

The perception engine provides the following to Timeline Threading:

1. **Tracked Objects**: Current state and history
2. **Behavior Flags**: Detected suspicious behaviors
3. **Threat Scores**: Probability of incident
4. **Feature Vectors**: For identity matching
5. **Trajectories**: Motion history for prediction

## Optimization Notes

### ARTPEC-8 Specific
- Use per-tensor quantization (not per-channel)
- Prefer regular convolutions over depthwise
- 3x3 kernels are optimal
- Use stride 2 for downsampling
- Filters should be multiple of 6
- ReLU activation fuses with conv

### Performance Tips
- Reuse buffers to avoid allocation
- Use hardware color space conversion
- Batch preprocessing when possible
- Profile with `larod-benchmark`

## Known Limitations

1. Max 50 simultaneous tracks (configurable)
2. INT8 quantization may reduce accuracy slightly
3. Running detection requires calibration per camera
4. Feature extraction adds ~5ms per object

## Future Enhancements

- [ ] Person re-identification across cameras
- [ ] Pose estimation for advanced behavior analysis
- [ ] Facial recognition for access control
- [ ] License plate recognition for vehicles
- [ ] Anomaly detection for unusual patterns

## Dependencies

- **VDO**: Video Device Object API (ACAP SDK)
- **Larod**: ML inference API (ACAP SDK)
- **OpenCV**: 4.8+ (ARTPEC-optimized from SDK)
- **GLib**: 2.0+

## References

- [ACAP Computer Vision Guide](../../docs/axis-acap/Vision/)
- [ARTPEC-8 Optimization Tips](../../docs/axis-acap/Vision/optimization-tips.md)
- [Axis Model Zoo](https://github.com/AxisCommunications/axis-model-zoo)
- [ACAP SDK Examples](https://github.com/AxisCommunications/acap-native-sdk-examples)
