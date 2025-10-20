# OMNISIGHT ML Models

This directory contains machine learning models for the OMNISIGHT perception engine, optimized for Axis ARTPEC-8 DLPU.

## Directory Structure

```
models/
├── detection/              # Object detection models
│   ├── efficientnet_b4_detection.tflite
│   └── model_config.json
├── features/               # Feature extraction models
│   ├── mobilenet_v3_features.tflite
│   └── model_config.json
├── trajectory/             # Trajectory prediction models (Timeline Threading)
│   ├── transformer_trajectory.tflite
│   └── model_config.json
├── training/               # Training infrastructure
│   ├── data/              # Training datasets
│   ├── scripts/           # Training scripts
│   ├── configs/           # Model configurations
│   └── checkpoints/       # Training checkpoints
├── converted/              # Intermediate conversion outputs
└── benchmarks/             # Performance benchmarks
```

## Models Overview

### 1. Object Detection Model
**Purpose:** Detect and classify objects in video frames

**Architecture:** EfficientNet-B4 (modified for ARTPEC-8)
- Input: 416x416x3 RGB (INT8)
- Output:
  - Bounding boxes: [N, 4] (x, y, w, h)
  - Classes: [N, 6] (person, vehicle, face, package, animal, other)
  - Scores: [N, 1] confidence scores

**Optimizations:**
- INT8 per-tensor quantization
- 3x3 convolutions (optimal for ARTPEC-8)
- Regular convolutions (not depthwise)
- Filters multiple of 6
- ReLU activation fusion

**Performance Target:**
- Inference: <15ms on DLPU
- mAP: >0.70
- Size: <30MB

### 2. Feature Extraction Model
**Purpose:** Extract feature vectors for object re-identification

**Architecture:** MobileNet-V3-Small
- Input: 128x128x3 RGB (INT8)
- Output: Feature vector [128] (FLOAT32)

**Use Case:**
- Re-identify objects across frames
- Track objects after occlusion
- Swarm Intelligence signature matching

**Performance Target:**
- Inference: <5ms per object on DLPU
- Feature distance accuracy: >0.85
- Size: <10MB

### 3. Trajectory Prediction Model
**Purpose:** Predict future trajectories for Timeline Threading™

**Architecture:** Transformer-based (8-layer, 256-dim)
- Input: Track history [30, 8] (position, velocity, features)
- Output: Predicted trajectories [5, 60] (5 timelines, 60 future frames)

**Use Case:**
- Timeline Threading engine
- Predict 3-5 probable futures
- 5-minute prediction horizon

**Performance Target:**
- Inference: <25ms on DLPU
- Prediction accuracy: >0.60 @ 5min
- Size: <200MB quantized

## ARTPEC-8 DLPU Requirements

### Hardware Specifications
- Chip: ARTPEC-8 with DLPU
- NPU Performance: ~1.5 TOPS INT8
- Memory: Shared with system
- Framework: TensorFlow Lite

### Optimization Guidelines

**Quantization:**
- Use INT8 per-tensor quantization (NOT per-channel)
- Calibration dataset: 100-500 representative images
- Post-training quantization preferred

**Model Architecture:**
- Input size: Multiple of 32 (e.g., 416x416, 608x608)
- Batch size: 1 only
- Kernel size: 3x3 optimal
- Stride: 2 for downsampling
- Filters: Multiple of 6
- Activation: ReLU (fuses with convolution)

**Operators:**
- Prefer: Conv2D, DepthwiseConv2D, MaxPool, AveragePool, ReLU
- Avoid: Complex operations that fall back to CPU
- Supported: See ARTPEC-8 operator table in docs

## Model Training

### Prerequisites
```bash
pip install -r training/requirements.txt
```

Required packages:
- TensorFlow 2.14+
- TensorFlow Lite
- TensorFlow Model Optimization Toolkit
- NumPy, Pillow, OpenCV
- Matplotlib (for visualization)

### Training Object Detection Model

```bash
cd training/scripts

# Prepare dataset
python prepare_dataset.py \
  --dataset-path ../data/coco \
  --output-path ../data/processed

# Train model
python train_detection.py \
  --config ../configs/efficientnet_b4.yaml \
  --epochs 100 \
  --batch-size 32 \
  --checkpoint-dir ../checkpoints/detection

# Convert to TFLite
python convert_to_tflite.py \
  --model ../checkpoints/detection/best_model.h5 \
  --output ../../detection/efficientnet_b4_detection.tflite \
  --quantize int8 \
  --calibration-data ../data/calibration
```

### Training Feature Extraction Model

```bash
cd training/scripts

# Train feature extractor
python train_features.py \
  --config ../configs/mobilenet_v3.yaml \
  --epochs 50 \
  --triplet-loss \
  --checkpoint-dir ../checkpoints/features

# Convert to TFLite
python convert_to_tflite.py \
  --model ../checkpoints/features/best_model.h5 \
  --output ../../features/mobilenet_v3_features.tflite \
  --quantize int8
```

## Model Conversion Pipeline

### 1. Export from Training Format
```python
# Save Keras model
model.save('model.h5')

# Or SavedModel format
tf.saved_model.save(model, 'saved_model/')
```

### 2. Convert to TFLite
```python
converter = tf.lite.TFLiteConverter.from_keras_model(model)

# Enable ARTPEC-8 optimizations
converter.optimizations = [tf.lite.Optimize.DEFAULT]
converter.target_spec.supported_ops = [tf.lite.OpsSet.TFLITE_BUILTINS_INT8]
converter.inference_input_type = tf.int8
converter.inference_output_type = tf.int8

# Representative dataset for quantization
def representative_dataset():
    for _ in range(100):
        yield [np.random.random((1, 416, 416, 3)).astype(np.float32)]

converter.representative_dataset = representative_dataset

# Convert
tflite_model = converter.convert()
```

### 3. Validate Model
```bash
python validate_model.py \
  --model ../../detection/efficientnet_b4_detection.tflite \
  --test-data ../data/test \
  --target artpec8
```

## Benchmarking

### On Camera Hardware
```bash
# Deploy model to camera
scp efficientnet_b4_detection.tflite root@192.168.1.100:/tmp/

# Run benchmark
ssh root@192.168.1.100 "larod-benchmark /tmp/efficientnet_b4_detection.tflite"
```

### Expected Performance

| Model | Size | Latency (DLPU) | Latency (CPU) | Memory |
|-------|------|----------------|---------------|--------|
| Detection | 28MB | 14ms | 180ms | 85MB |
| Features | 8MB | 4ms | 45ms | 25MB |
| Trajectory | 180MB | 22ms | 350ms | 150MB |

## Model Deployment

### 1. Copy to Application
```bash
cp detection/efficientnet_b4_detection.tflite \
   ../src/perception/models/
```

### 2. Update manifest.json
```json
{
  "resources": {
    "storage": {
      "persistent": 250  // Increased for models
    }
  }
}
```

### 3. Load in Application
```c
LarodInferenceConfig config = {
    .model_path = "/opt/app/models/efficientnet_b4_detection.tflite",
    .chip = LAROD_CHIP_DLPU,
    .num_threads = 4
};
LarodInference* inference = larod_inference_init(&config);
```

## Pre-trained Models

### Download Links
Coming soon - models will be hosted on:
- GitHub Releases
- Axis Model Zoo
- Hugging Face

### Quick Start (Placeholder Models)
For testing without training:
```bash
cd models/detection
wget https://github.com/oneshot2001/OmniSight-dev/releases/download/v0.1.0/efficientnet_b4_detection.tflite
```

## Training Datasets

### Recommended Datasets

**Object Detection:**
- COCO 2017 (person, vehicle classes)
- Open Images v7 (additional classes)
- Custom security footage (domain-specific)

**Feature Extraction:**
- Market-1501 (person re-ID)
- VeRi-776 (vehicle re-ID)
- Custom tracked objects

**Trajectory Prediction:**
- ETH/UCY pedestrian dataset
- Stanford Drone Dataset
- Custom surveillance trajectories

### Dataset Preparation
```bash
cd training/scripts

# Download COCO
python download_coco.py --output ../data/coco

# Prepare for training
python prepare_dataset.py \
  --dataset coco \
  --input ../data/coco \
  --output ../data/processed \
  --split 0.8/0.1/0.1
```

## Model Versioning

### Naming Convention
```
<model_type>_<architecture>_<version>_<date>.tflite

Examples:
- detection_efficientnet_b4_v1.0_20250120.tflite
- features_mobilenet_v3_v1.2_20250125.tflite
```

### Version History
Track in `model_versions.json`:
```json
{
  "detection": {
    "current": "v1.0",
    "versions": [
      {
        "version": "v1.0",
        "date": "2025-01-20",
        "metrics": {"mAP": 0.72, "latency_ms": 14},
        "notes": "Initial release"
      }
    ]
  }
}
```

## Troubleshooting

### Model Not Loading
- Check file path and permissions
- Verify TFLite format compatibility
- Check DLPU availability with `larod-client -l`

### Poor Performance
- Verify INT8 quantization
- Check calibration dataset quality
- Profile with `larod-benchmark`
- Review ARTPEC-8 optimization guidelines

### High Latency
- Reduce model complexity
- Optimize operator usage
- Enable hardware acceleration
- Check CPU fallback with profiler

## References

- [Axis Model Zoo](https://github.com/AxisCommunications/axis-model-zoo)
- [ACAP SDK Examples](https://github.com/AxisCommunications/acap-native-sdk-examples)
- [TensorFlow Lite Guide](https://www.tensorflow.org/lite/guide)
- [ARTPEC-8 Optimization](../../docs/axis-acap/Vision/optimization-tips.md)

## Support

For model training and optimization support:
- GitHub Issues: https://github.com/oneshot2001/OmniSight-dev/issues
- Documentation: [docs/axis-acap/Vision/](../../docs/axis-acap/Vision/)
