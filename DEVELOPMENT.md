# OMNISIGHT Development Guide

This document provides detailed instructions for setting up and developing the OMNISIGHT precognitive security system.

## Table of Contents
- [Prerequisites](#prerequisites)
- [Environment Setup](#environment-setup)
- [Development Workflow](#development-workflow)
- [Architecture Overview](#architecture-overview)
- [Building and Testing](#building-and-testing)
- [Deployment](#deployment)
- [Troubleshooting](#troubleshooting)

## Prerequisites

### Required Software
- **Docker Desktop** 4.0+ (with Docker Compose)
- **Git** 2.30+
- **Make** (GNU Make 4.0+)
- **8GB+ RAM** for development environment
- **20GB+ free disk space**

### Optional
- **Axis camera** with ARTPEC-8 chipset for hardware testing
- **Python 3.8+** for model training/conversion (outside Docker)
- **Visual Studio Code** with Remote-Containers extension

### ACAP SDK Version
This project uses **ACAP Native SDK 1.15** which supports:
- AXIS OS 12.0+
- ACAP version 12 (aligned with AXIS OS)
- ARTPEC-8 with DLPU support

## Environment Setup

### 1. Clone the Repository
```bash
git clone https://github.com/oneshot2001/OmniSight-dev.git
cd OmniSight-dev
```

### 2. Environment Configuration
Create a `.env` file for local configuration:
```bash
cat > .env << EOF
# Camera connection settings
CAMERA_IP=192.168.1.100
CAMERA_USER=root
CAMERA_PASS=yourpassword

# Development settings
OMNISIGHT_ENV=development
DEBUG=1
LOG_LEVEL=debug

# Model settings
MODEL_QUANTIZATION=int8
INFERENCE_THREADS=4
EOF
```

### 3. Build Development Environment
```bash
# Build the Docker development container
make docker-build

# This will:
# - Pull ACAP Native SDK base image
# - Install development tools (GCC, CMake, Python)
# - Set up Rust toolchain
# - Configure cross-compilation for aarch64
```

### 4. Start Development Environment
```bash
# Start all services (development container + MQTT broker)
make dev

# Or manually with docker-compose
docker-compose up -d
```

### 5. Enter Development Container
```bash
# Open a shell in the development container
make shell

# Inside the container, verify setup:
aarch64-linux-gnu-gcc --version
cmake --version
python3 --version
rustc --version
```

## Development Workflow

### Project Structure
```
omnisight/
├── src/
│   ├── core/           # Application framework and lifecycle
│   ├── perception/     # Vision processing and object detection
│   ├── timeline/       # Timeline Threading™ engine
│   ├── swarm/          # Swarm Intelligence networking
│   ├── nlp/            # Natural Language Processing
│   └── platform/       # ACAP platform integration (Larod, VDO)
├── models/             # ML models (TFLite format)
├── tests/              # Unit and integration tests
├── scripts/            # Build and deployment scripts
├── config/             # Configuration files
└── docs/               # Documentation
```

### Daily Development Cycle

#### 1. Start Development Environment
```bash
# Start containers
make dev

# Enter shell
make shell
```

#### 2. Make Code Changes
Edit files in your local `src/` directory - changes are live-mounted in the container.

#### 3. Build and Test
```bash
# Inside container or via make commands:

# Build the application
make build

# Run tests
make test

# Run specific test suite
make test-perception
make test-timeline
make test-swarm
```

#### 4. Package and Deploy
```bash
# Create ACAP package
make package

# Deploy to camera (requires .env configuration)
make deploy

# Or just install existing package
make install CAMERA_IP=192.168.1.100
```

#### 5. Monitor and Debug
```bash
# View logs from camera
make logs

# Check application status
make status

# Access camera's syslog
curl -k --anyauth -u root:pass \
  https://192.168.1.100/axis-cgi/systemlog.cgi
```

## Architecture Overview

### Core Components

#### 1. Perception Engine (`src/perception/`)
- **Purpose**: Object detection, tracking, and feature extraction
- **Technologies**: OpenCV, TensorFlow Lite, Larod API
- **Models**: EfficientNet-B4 (optimized for ARTPEC-8)
- **Performance Target**: <20ms per frame at 10fps

#### 2. Timeline Threading™ (`src/timeline/`)
- **Purpose**: Predict 3-5 probable futures with confidence scores
- **Technologies**: Transformer-based trajectory prediction
- **Horizon**: 30 seconds to 5 minutes
- **Output**: Probability tree with intervention points

#### 3. Swarm Intelligence (`src/swarm/`)
- **Purpose**: Multi-camera threat sharing and collective learning
- **Technologies**: MQTT 5.0, Federated Learning
- **Privacy**: No raw video sharing, only threat signatures
- **Latency**: <100ms propagation across network

#### 4. Natural Language Interface (`src/nlp/`)
- **Purpose**: Conversational control and queries
- **Technologies**: Quantized Llama 3B (INT4)
- **Capabilities**: 1000+ commands, 7-day context memory
- **Latency**: <200ms response time

#### 5. Platform Integration (`src/platform/`)
- **Purpose**: ACAP SDK integration (Larod, VDO, Events)
- **APIs Used**:
  - Larod - ML inference acceleration
  - VDO - Video capture and processing
  - Event - Event generation and handling
  - Parameter - Configuration management

### Data Flow
```
Camera → VDO API → Perception Engine → Timeline Engine → Decision Layer
                          ↓                    ↓
                    Swarm Network      Intervention Actions
                          ↓                    ↓
                    Global Intelligence   Event Generation
```

## Building and Testing

### Build Targets

#### Full Build
```bash
make build ARCH=aarch64
```

#### Component-Specific Builds
```bash
# Build perception engine only
cd src/perception && cmake . && make

# Build timeline engine
cd src/timeline && cargo build --target aarch64-unknown-linux-gnu
```

#### Clean Build
```bash
make clean      # Remove build artifacts
make clean-all  # Remove all including Docker volumes
```

### Testing Strategy

#### Unit Tests
```bash
# Run all unit tests
make test

# Run with verbose output
docker exec -it omnisight-dev \
  bash -c "cd tests && python3 -m pytest -v -s"
```

#### Integration Tests
```bash
# Test perception pipeline
make test-perception

# Test timeline prediction
make test-timeline

# Test swarm communication
make test-swarm
```

#### Camera Testing
```bash
# Deploy to test camera
make deploy CAMERA_IP=192.168.1.100

# Monitor real-time logs
make logs

# Run integration tests on camera
make test-camera
```

### Model Development

#### Training Models
```bash
# Train perception model (outside Docker)
cd models/perception
python train.py --dataset ./data --epochs 100

# Train timeline prediction model
cd models/timeline
python train_trajectory.py --config config.yaml
```

#### Convert Models to TFLite
```bash
# Use Docker container for conversion
make convert-model

# Or manually
docker run --rm -v $(pwd)/models:/models omnisight-dev \
  python3 scripts/convert_model.py \
    --input models/perception/model.h5 \
    --output models/perception/model.tflite \
    --quantize int8
```

#### Deploy Models
Models should be placed in `models/` directory and will be packaged with the ACAP application.

## Deployment

### Development Deployment
```bash
# Quick deploy to development camera
make deploy CAMERA_IP=192.168.1.100

# With credentials
make deploy CAMERA_IP=192.168.1.100 \
  CAMERA_USER=root \
  CAMERA_PASS=yourpassword
```

### Production Packaging
```bash
# Create production package
make package

# Output: package/omnisight_0.1.0_aarch64.eap
```

### Camera Installation

#### Via Web Interface
1. Open camera web interface: `https://192.168.1.100`
2. Navigate to **Apps** → **Add app**
3. Upload `omnisight_0.1.0_aarch64.eap`
4. Start application

#### Via CLI
```bash
# Upload package
curl -k --anyauth -u root:pass \
  -F file=@package/omnisight_0.1.0_aarch64.eap \
  https://192.168.1.100/axis-cgi/applications/upload.cgi

# Start application
curl -k --anyauth -u root:pass \
  "https://192.168.1.100/axis-cgi/applications/control.cgi?action=start&package=omnisight"
```

### Verify Installation
```bash
# Check application status
make status

# View logs
make logs

# Access web interface
open https://192.168.1.100/local/omnisight/
```

## Troubleshooting

### Docker Issues

**Problem**: Docker build fails with network errors
```bash
# Solution: Use Docker BuildKit
export DOCKER_BUILDKIT=1
make docker-build
```

**Problem**: Permission denied accessing Docker
```bash
# Solution: Add user to docker group
sudo usermod -aG docker $USER
newgrp docker
```

### Build Issues

**Problem**: Cross-compilation fails
```bash
# Solution: Verify SDK installation
docker run --rm axisecp/acap-native-sdk:1.15-aarch64-ubuntu22.04 \
  aarch64-linux-gnu-gcc --version
```

**Problem**: Linking errors with libraries
```bash
# Solution: Check library paths in CMakeLists.txt
# Ensure SDKTARGETSYSROOT is set correctly
echo $SDKTARGETSYSROOT
```

### Camera Connection Issues

**Problem**: Cannot connect to camera
```bash
# Verify camera is reachable
ping 192.168.1.100

# Check camera web interface
curl -k https://192.168.1.100

# Verify credentials
curl -k --anyauth -u root:pass \
  https://192.168.1.100/axis-cgi/basicdeviceinfo.cgi
```

**Problem**: Installation fails
```bash
# Check camera firmware version (requires AXIS OS 12.0+)
curl -k --anyauth -u root:pass \
  https://192.168.1.100/axis-cgi/basicdeviceinfo.cgi | grep FirmwareVersion

# Check available storage
curl -k --anyauth -u root:pass \
  https://192.168.1.100/axis-cgi/systeminfo.cgi
```

### Runtime Issues

**Problem**: Application crashes on startup
```bash
# Check logs
make logs

# Check resource usage
curl -k --anyauth -u root:pass \
  https://192.168.1.100/axis-cgi/applications/list.cgi
```

**Problem**: Model inference fails
```bash
# Verify DLPU availability
# Check camera has ARTPEC-8 chip
# Ensure model is correctly quantized (INT8)
```

## Development Best Practices

### Code Style
- **C++**: Follow Google C++ Style Guide
- **Rust**: Use `rustfmt` for formatting
- **Python**: PEP 8 compliance

### Git Workflow
```bash
# Create feature branch
git checkout -b feature/timeline-threading

# Make changes and commit
git add .
git commit -m "Implement Timeline Threading prediction engine"

# Push to remote
git push origin feature/timeline-threading

# Create Pull Request on GitHub
```

### Performance Profiling
```bash
# Enable profiling build
make build CMAKE_BUILD_TYPE=RelWithDebInfo

# Use gdbserver for remote debugging
gdbserver :2345 /opt/app/build/omnisight

# Connect from host
gdb-multiarch -ex "target remote 192.168.1.100:2345"
```

## Resources

### Documentation
- [OMNISIGHT PRD](docs/axis-acap/Markdown%20versions/omnisight-prd.md)
- [Axis ACAP Documentation](docs/axis-acap/)
- [ACAP Native SDK Examples](https://github.com/AxisCommunications/acap-native-sdk-examples)

### Support
- **Issues**: https://github.com/oneshot2001/OmniSight-dev/issues
- **Axis Developer Community**: https://axis.com/developer-community
- **ACAP Documentation**: https://developer.axis.com/acap/

## Next Steps

1. **Week 1-2**: Implement core perception engine
2. **Week 3-4**: Build Timeline Threading prototype
3. **Week 5-6**: Develop Swarm Intelligence protocol
4. **Week 7-8**: Integrate Natural Language Processing
5. **Week 9-10**: Performance optimization
6. **Week 11-12**: ISC West demo preparation

---

**Let's build the future of security!**
