# OMNISIGHT ML Model Training Guide

Complete guide for training, converting, and deploying object detection models for ARTPEC-8 cameras.

## Table of Contents

- [Overview](#overview)
- [Requirements](#requirements)
- [Quick Start](#quick-start)
- [Detailed Workflow](#detailed-workflow)
- [Training Configuration](#training-configuration)
- [Performance Targets](#performance-targets)
- [Troubleshooting](#troubleshooting)

## Overview

The OMNISIGHT ML pipeline trains lightweight object detection models optimized for Axis ARTPEC-8/9 DLPU deployment with:

- **INT8 quantization** for 4x smaller models and faster inference
- **ARTPEC-8 optimizations** (regular convolutions, per-tensor quantization)
- **COCO dataset** for robust object detection
- **Automated pipeline** from download to deployment

### Model Architecture

- **Backbone**: Efficient CNN optimized for DLPU (regular convolutions, not depthwise)
- **Head**: Detection head with bounding boxes and class predictions
- **Input**: 416×416 RGB images (INT8)
- **Output**: Object detections [x1, y1, x2, y2, class, score]
- **Classes**: 6 categories (person, vehicle, face, package, animal, other)

## Requirements

### Software Dependencies

```bash
# Python 3.8+
python3 --version

# Install requirements
cd models/training
pip install -r requirements.txt
```

#### requirements.txt
```
tensorflow>=2.12.0
numpy>=1.24.0
opencv-python>=4.8.0
PyYAML>=6.0
tqdm>=4.66.0
matplotlib>=3.7.0
```

### Hardware Requirements

**Training:**
- CPU: 4+ cores recommended
- RAM: 16GB+ (for full COCO dataset, 8GB for subsets)
- Storage: 50GB+ (full COCO) or 10GB (subset)
- GPU: Optional but highly recommended (10x faster)

**Inference (ARTPEC-8 camera):**
- ARTPEC-8/9 SoC with DLPU
- RAM: 512MB (model uses <100MB)
- CPU: <30% per camera stream

## Quick Start

### 1-Command Full Pipeline

Train a model from scratch in one command:

```bash
cd models/training/scripts
./train_pipeline.sh --all
```

This will:
1. Download COCO dataset subset (5,000 training + 1,000 validation images)
2. Train detection model (50 epochs, ~2-4 hours on GPU)
3. Convert to TFLite INT8
4. Run performance benchmarks

**Estimated time:**
- With GPU: 2-4 hours
- CPU only: 12-24 hours

### Individual Steps

Run specific pipeline steps:

```bash
# Download data only
./train_pipeline.sh --download

# Train only (requires downloaded data)
./train_pipeline.sh --train

# Convert existing model
./train_pipeline.sh --convert

# Benchmark existing TFLite model
./train_pipeline.sh --benchmark
```

### Custom Configuration

```bash
# Train with custom parameters
./train_pipeline.sh --all \
    --subset-size 10000 \
    --epochs 100 \
    --batch-size 32
```

## Detailed Workflow

### Step 1: Download COCO Dataset

Download and prepare COCO dataset:

```bash
cd models/training/scripts

# Download full dataset (~25GB)
python3 download_coco.py \
    --data-dir ../data/coco \
    --split both \
    --images \
    --annotations

# Create subset for faster training
python3 download_coco.py \
    --data-dir ../data/coco \
    --split train \
    --subset 5000

python3 download_coco.py \
    --data-dir ../data/coco \
    --split val \
    --subset 1000

# Print dataset statistics
python3 download_coco.py \
    --data-dir ../data/coco \
    --stats
```

**Dataset structure:**
```
data/coco/
├── train2017/              # Full training images (118K)
├── val2017/                # Full validation images (5K)
├── annotations/            # COCO annotations
├── subset_train2017/       # Training subset
│   ├── images/             # 5,000 images
│   └── annotations.json
└── subset_val2017/         # Validation subset
    ├── images/             # 1,000 images
    └── annotations.json
```

### Step 2: Train Detection Model

Train model on COCO subset:

```bash
python3 train_omnisight.py \
    --epochs 50 \
    --batch-size 16 \
    --lr 0.001
```

**Training output:**
- Model checkpoints: `training/checkpoints/`
- Final model: `models/detection/omnisight_detection.keras`
- SavedModel: `models/detection/saved_model/`
- Training history: `models/detection/training_history.json`

**Monitoring training:**

```bash
# View training progress
tensorboard --logdir training/checkpoints/logs
```

**Training tips:**
- Start with smaller subset (1,000-5,000 images) for faster iteration
- Use GPU for 10x faster training
- Monitor validation loss to avoid overfitting
- Typical training: 50 epochs, ~2-4 hours on GPU

### Step 3: Convert to TFLite INT8

Convert trained model to TFLite with INT8 quantization:

```bash
python3 convert_tflite.py \
    --model ../detection/omnisight_detection.keras \
    --output ../detection/omnisight_detection_int8.tflite \
    --quantize \
    --representative-data ../data/coco/subset_val2017/annotations.json \
    --representative-images ../data/coco/subset_val2017/images \
    --num-calibration-samples 100
```

**Why INT8 quantization?**
- **4x smaller** models (~25MB → ~7MB)
- **4x faster** inference on DLPU
- **Lower power** consumption
- **No significant accuracy loss** (typically <2%)

**ARTPEC-8 optimizations:**
- Per-tensor quantization (not per-channel)
- Input/output: INT8
- Optimized for DLPU operators

### Step 4: Benchmark and Validate

Test model performance:

```bash
python3 benchmark_model.py \
    --model ../detection/omnisight_detection_int8.tflite \
    --all \
    --annotations ../data/coco/subset_val2017/annotations.json \
    --images ../data/coco/subset_val2017/images \
    --num-samples 100 \
    --num-runs 100 \
    --output ../detection/benchmark_results.json
```

**Benchmarks:**
- **Latency**: Mean, P50, P95, P99 inference time
- **Memory**: Model size and runtime memory usage
- **Compatibility**: ARTPEC-8 DLPU checks
- **Validation**: Accuracy on test dataset

**Expected results (on development machine, not camera):**
- Latency: 10-30ms (CPU), 2-5ms (GPU)
- Memory: 7-10MB model, 50-80MB total
- Validation: >95% inference success rate

**On ARTPEC-8 camera (actual deployment):**
- Target latency: <15ms @ 30 FPS
- Target memory: <100MB
- Target accuracy: mAP >0.70

## Training Configuration

### Default Configuration

Edit `training/configs/efficientnet_b4_detection.yaml`:

```yaml
model:
  name: omnisight_detection
  input_size: [416, 416, 3]
  num_classes: 6

training:
  batch_size: 16
  epochs: 50
  learning_rate: 0.001
  lr_decay_factor: 0.5
  lr_decay_epochs: 10

data:
  train_annotations: data/coco/subset_train2017/annotations.json
  train_images: data/coco/subset_train2017/images
  val_annotations: data/coco/subset_val2017/annotations.json
  val_images: data/coco/subset_val2017/images

output:
  checkpoint_dir: training/checkpoints
  model_dir: models/detection
```

### Hyperparameter Tuning

**For faster training (development):**
```bash
./train_pipeline.sh --all \
    --subset-size 1000 \
    --epochs 20 \
    --batch-size 8
```

**For higher accuracy (production):**
```bash
./train_pipeline.sh --all \
    --subset-size 20000 \
    --epochs 100 \
    --batch-size 32
```

**For limited GPU memory:**
```bash
./train_pipeline.sh --train \
    --batch-size 4  # Smaller batch size
```

### Data Augmentation

Built-in augmentations (in `coco_dataset.py`):
- Random horizontal flip (50%)
- Random brightness adjustment (±20%)
- Random contrast adjustment (±20%)

To add more augmentations, edit `coco_dataset.py::_augment_image()`.

## Performance Targets

### ARTPEC-8 Camera Targets

| Metric | Target | Typical |
|--------|--------|---------|
| **Latency (P95)** | <15ms | 8-12ms |
| **FPS** | 30 | 30-60 |
| **Memory** | <100MB | 60-80MB |
| **Model Size** | <15MB | 7-10MB |
| **mAP** | >0.70 | 0.72-0.78 |
| **CPU Usage** | <30% | 15-25% |

### Development Machine Benchmarks

Typical performance on development machines:

**CPU (Intel i7 / Apple M1):**
- Latency: 15-30ms
- FPS: 30-60

**GPU (NVIDIA RTX 3060 / Apple M1 GPU):**
- Latency: 2-5ms
- FPS: 200-500

**Note**: Development benchmarks are indicative only. Always validate on actual ARTPEC-8 camera hardware.

## Model Deployment

After training and conversion, deploy to OMNISIGHT:

1. **Copy TFLite model to ACAP:**
   ```bash
   cp models/detection/omnisight_detection_int8.tflite \
      <ACAP_ROOT>/models/detection/
   ```

2. **Update model path in code:**
   Edit `src/perception/perception.c`:
   ```c
   larod_config.model_path = "models/detection/omnisight_detection_int8.tflite";
   ```

3. **Rebuild and deploy ACAP:**
   ```bash
   make package
   make deploy CAMERA_IP=192.168.1.100
   ```

4. **Test on camera:**
   ```bash
   make monitor CAMERA_IP=192.168.1.100
   ```

## Troubleshooting

### Out of Memory (OOM)

**Symptoms**: Training crashes with "Out of Memory" error

**Solutions**:
- Reduce batch size: `--batch-size 4` or `--batch-size 8`
- Use smaller dataset subset: `--subset-size 1000`
- Close other applications
- Use smaller input size (edit config: `input_size: [320, 320, 3]`)

### Slow Training

**Symptoms**: Training takes many hours

**Solutions**:
- Use GPU if available (10x faster)
- Reduce dataset size: `--subset-size 1000`
- Reduce epochs for initial testing: `--epochs 10`
- Increase batch size if you have GPU memory: `--batch-size 32`

### Poor Validation Accuracy

**Symptoms**: val_loss not decreasing or accuracy <90%

**Solutions**:
- Train longer: `--epochs 100`
- Use larger dataset: `--subset-size 10000`
- Check for data quality issues
- Adjust learning rate: `--lr 0.0005`
- Enable more augmentation

### TFLite Conversion Failed

**Symptoms**: Conversion errors or incompatible ops

**Solutions**:
- Verify model loads correctly in Keras first
- Check that all ops are TFLite-compatible
- Try without quantization first: `--no-quantize`
- Provide representative dataset for better quantization

### Model Too Large

**Symptoms**: Model >15MB after quantization

**Solutions**:
- Ensure INT8 quantization is enabled
- Reduce model complexity (edit architecture in `train_omnisight.py`)
- Use smaller input size

### Poor Performance on Camera

**Symptoms**: Latency >20ms or low FPS on camera

**Solutions**:
- Verify INT8 quantization: `--quantize`
- Check ARTPEC-8 compatibility with benchmark tool
- Reduce input size: 320×320 instead of 416×416
- Optimize model architecture (use regular convolutions, not depthwise)

## Advanced Topics

### Custom Categories

To train on custom object categories, edit `coco_dataset.py::_create_category_mapping()`:

```python
omnisight_categories = {
    'person': 0,
    'vehicle': 1,
    'custom_object': 2,  # Add your category
    # ...
}
```

### Transfer Learning

To fine-tune a pre-trained model:

```python
# In train_omnisight.py, load existing weights
model = keras.models.load_model('pretrained_model.keras')

# Freeze early layers
for layer in model.layers[:20]:
    layer.trainable = False

# Train with lower learning rate
trainer.config['training']['learning_rate'] = 0.0001
```

### Multi-GPU Training

For multi-GPU training, edit `train_omnisight.py`:

```python
strategy = tf.distribute.MirroredStrategy()

with strategy.scope():
    model = build_model()
    model.compile(...)
```

## Resources

- **COCO Dataset**: https://cocodataset.org
- **TensorFlow Lite**: https://www.tensorflow.org/lite
- **ACAP Documentation**: https://axiscommunications.github.io/acap-documentation/
- **ARTPEC-8 DLPU Guide**: See `docs/axis-acap/ARTPEC-8/`

## Next Steps

After training your model:

1. ✅ Benchmark on development machine
2. ✅ Deploy to ARTPEC-8 camera
3. ✅ Test with real camera video
4. ✅ Monitor performance metrics
5. ✅ Iterate and improve

For deployment instructions, see [INSTALLATION.md](../INSTALLATION.md).

---

**Questions or Issues?**

- Check [Troubleshooting](#troubleshooting) section
- Review training logs and error messages
- Test with smaller subset first
- Validate on ARTPEC-8 hardware
