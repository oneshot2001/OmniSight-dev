#!/bin/bash
#
# OMNISIGHT ML Training Pipeline
# End-to-end automated pipeline for training and deploying models
#

set -e  # Exit on error

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Logging functions
log_info() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

log_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

log_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

log_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Banner
print_banner() {
    echo ""
    echo "============================================================"
    echo "  OMNISIGHT ML Training Pipeline"
    echo "  Automated model training for ARTPEC-8 deployment"
    echo "============================================================"
    echo ""
}

# Configuration
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
TRAINING_DIR="$(dirname "$SCRIPT_DIR")"
MODELS_DIR="$(dirname "$TRAINING_DIR")"
DATA_DIR="$TRAINING_DIR/data"
CHECKPOINTS_DIR="$TRAINING_DIR/checkpoints"
OUTPUT_DIR="$MODELS_DIR/detection"

# Default parameters
SUBSET_SIZE=5000
VAL_SUBSET_SIZE=1000
EPOCHS=50
BATCH_SIZE=16
NUM_CALIBRATION_SAMPLES=100

# Parse arguments
DOWNLOAD_DATA=false
TRAIN_MODEL=false
CONVERT_TFLITE=false
BENCHMARK=false
FULL_PIPELINE=false

while [[ $# -gt 0 ]]; do
    case $1 in
        --download)
            DOWNLOAD_DATA=true
            shift
            ;;
        --train)
            TRAIN_MODEL=true
            shift
            ;;
        --convert)
            CONVERT_TFLITE=true
            shift
            ;;
        --benchmark)
            BENCHMARK=true
            shift
            ;;
        --all)
            FULL_PIPELINE=true
            DOWNLOAD_DATA=true
            TRAIN_MODEL=true
            CONVERT_TFLITE=true
            BENCHMARK=true
            shift
            ;;
        --subset-size)
            SUBSET_SIZE="$2"
            shift 2
            ;;
        --epochs)
            EPOCHS="$2"
            shift 2
            ;;
        --batch-size)
            BATCH_SIZE="$2"
            shift 2
            ;;
        --help)
            echo "Usage: $0 [OPTIONS]"
            echo ""
            echo "Options:"
            echo "  --download          Download and prepare COCO dataset"
            echo "  --train             Train detection model"
            echo "  --convert           Convert to TFLite INT8"
            echo "  --benchmark         Run performance benchmarks"
            echo "  --all               Run full pipeline (all steps)"
            echo "  --subset-size N     COCO subset size (default: 5000)"
            echo "  --epochs N          Training epochs (default: 50)"
            echo "  --batch-size N      Batch size (default: 16)"
            echo "  --help              Show this help message"
            echo ""
            echo "Examples:"
            echo "  $0 --all                    # Run full pipeline"
            echo "  $0 --download --train       # Download data and train"
            echo "  $0 --convert --benchmark    # Convert existing model and benchmark"
            exit 0
            ;;
        *)
            log_error "Unknown option: $1"
            exit 1
            ;;
    esac
done

# Check if at least one step is selected
if [ "$DOWNLOAD_DATA" = false ] && [ "$TRAIN_MODEL" = false ] && [ "$CONVERT_TFLITE" = false ] && [ "$BENCHMARK" = false ]; then
    log_error "No steps selected. Use --help to see available options."
    exit 1
fi

# Print banner
print_banner

# Check Python dependencies
log_info "Checking Python dependencies..."
python3 -c "import tensorflow, numpy, cv2, yaml, tqdm" 2>/dev/null || {
    log_error "Missing Python dependencies. Install with:"
    echo "  pip install -r $TRAINING_DIR/requirements.txt"
    exit 1
}
log_success "Python dependencies OK"

# Step 1: Download and prepare COCO dataset
if [ "$DOWNLOAD_DATA" = true ]; then
    log_info "Step 1/4: Downloading COCO dataset..."
    echo ""

    # Download train and val splits
    python3 "$SCRIPT_DIR/download_coco.py" \
        --data-dir "$DATA_DIR/coco" \
        --split both \
        --images \
        --annotations

    # Create subsets
    log_info "Creating training subset ($SUBSET_SIZE images)..."
    python3 "$SCRIPT_DIR/download_coco.py" \
        --data-dir "$DATA_DIR/coco" \
        --split train \
        --subset $SUBSET_SIZE

    log_info "Creating validation subset ($VAL_SUBSET_SIZE images)..."
    python3 "$SCRIPT_DIR/download_coco.py" \
        --data-dir "$DATA_DIR/coco" \
        --split val \
        --subset $VAL_SUBSET_SIZE

    log_success "Dataset prepared"
    echo ""
fi

# Step 2: Train model
if [ "$TRAIN_MODEL" = true ]; then
    log_info "Step 2/4: Training detection model..."
    echo ""

    # Train
    python3 "$SCRIPT_DIR/train_omnisight.py" \
        --epochs $EPOCHS \
        --batch-size $BATCH_SIZE

    log_success "Model training complete"
    echo ""
fi

# Step 3: Convert to TFLite INT8
if [ "$CONVERT_TFLITE" = true ]; then
    log_info "Step 3/4: Converting to TFLite INT8..."
    echo ""

    # Find latest trained model
    KERAS_MODEL="$OUTPUT_DIR/omnisight_detection.keras"

    if [ ! -f "$KERAS_MODEL" ]; then
        log_error "Trained model not found: $KERAS_MODEL"
        log_info "Please run with --train first"
        exit 1
    fi

    # Convert with INT8 quantization
    python3 "$SCRIPT_DIR/convert_tflite.py" \
        --model "$KERAS_MODEL" \
        --output "$OUTPUT_DIR/omnisight_detection_int8.tflite" \
        --quantize \
        --representative-data "$DATA_DIR/coco/subset_val2017/annotations.json" \
        --representative-images "$DATA_DIR/coco/subset_val2017/images" \
        --num-calibration-samples $NUM_CALIBRATION_SAMPLES

    log_success "TFLite conversion complete"
    echo ""
fi

# Step 4: Benchmark model
if [ "$BENCHMARK" = true ]; then
    log_info "Step 4/4: Benchmarking model..."
    echo ""

    # Find TFLite model
    TFLITE_MODEL="$OUTPUT_DIR/omnisight_detection_int8.tflite"

    if [ ! -f "$TFLITE_MODEL" ]; then
        log_error "TFLite model not found: $TFLITE_MODEL"
        log_info "Please run with --convert first"
        exit 1
    fi

    # Run benchmarks
    python3 "$SCRIPT_DIR/benchmark_model.py" \
        --model "$TFLITE_MODEL" \
        --all \
        --annotations "$DATA_DIR/coco/subset_val2017/annotations.json" \
        --images "$DATA_DIR/coco/subset_val2017/images" \
        --num-samples 100 \
        --num-runs 100 \
        --output "$OUTPUT_DIR/benchmark_results.json"

    log_success "Benchmarking complete"
    echo ""
fi

# Final summary
echo ""
echo "============================================================"
log_success "Pipeline Complete!"
echo "============================================================"
echo ""

if [ "$TRAIN_MODEL" = true ]; then
    echo "Trained model:    $OUTPUT_DIR/omnisight_detection.keras"
fi

if [ "$CONVERT_TFLITE" = true ]; then
    echo "TFLite INT8:      $OUTPUT_DIR/omnisight_detection_int8.tflite"
fi

if [ "$BENCHMARK" = true ]; then
    echo "Benchmark results: $OUTPUT_DIR/benchmark_results.json"
fi

echo ""
echo "Next steps:"
echo "  1. Review benchmark results"
echo "  2. Test on ARTPEC-8 camera hardware"
echo "  3. Deploy to OMNISIGHT application"
echo ""
