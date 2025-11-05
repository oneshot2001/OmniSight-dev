# OMNISIGHT - Precognitive Security System

**Tagline**: "See Everything. Understand Everything. Prevent Everything."

OMNISIGHT is a revolutionary edge AI application for Axis cameras that transforms passive security cameras into active guardians through precognitive intelligence, swarm collaboration, and natural language understanding.

---

## ⚠️ QUICK START - Camera Compatibility

**IMPORTANT:** Not all Axis cameras have Python 3! Use the correct build for your camera:

| Your Camera | Build Command | Package Version |
|-------------|---------------|-----------------|
| **P3285-LVE (ARTPEC-9)** | `./scripts/build-phase4-v3-native-eap.sh` | v0.4.2 (Native) ✅ |
| **M4228-LVE (ARTPEC-8)** | `./scripts/build-phase4-v3-native-eap.sh` | v0.4.2 (Native) ✅ |
| **Unknown/New Camera** | `./scripts/build-phase4-v3-native-eap.sh` | v0.4.2 (Native) ✅ |

**Key Issue Resolved:** Phase 4 V3 (Native) works on ALL cameras without requiring Python.

📖 **Full Details:** See [PHASE4_PYTHON_DEPENDENCY_ISSUE.md](PHASE4_PYTHON_DEPENDENCY_ISSUE.md)

---

## Core Innovations

### Timeline Threading™
Industry's first precognitive security system showing 3-5 probable futures with confidence scores, enabling intervention before incidents occur.

### Swarm Intelligence
Network effect where cameras share threat signatures and learn collectively without sharing raw video data (privacy-preserving federated learning).

### Natural Language Interface
Conversational AI for intuitive camera control and investigation - no complex UIs, just talk to your security system.

### Autonomous Operations
Self-configuring, self-learning edge AI that becomes operational in <30 seconds from unboxing.

## Project Structure

```
omnisight/
├── src/                      # Source code
│   ├── core/                 # Core application framework
│   ├── perception/           # Vision and object detection
│   ├── timeline/             # Timeline Threading™ engine
│   ├── swarm/                # Swarm intelligence & networking
│   ├── nlp/                  # Natural language processing
│   └── platform/             # ACAP platform integration
├── models/                   # ML models (TFLite, ONNX)
├── tests/                    # Unit and integration tests
├── scripts/                  # Build and deployment scripts
├── config/                   # Configuration files
├── docs/                     # Documentation
│   ├── axis-acap/            # Axis development documentation
│   └── README.md             # Documentation index
├── Dockerfile                # ACAP SDK container
├── Makefile                  # Build system
└── manifest.json             # ACAP application manifest
```

## Technology Stack

### Platform
- **Target**: AXIS OS 12.0+ (ACAP 4.0+)
- **Hardware**: ARTPEC-8 with DLPU
- **Runtime**: ACAP Native SDK 4.0

### Languages
- **Rust**: Core safety-critical components
- **C++**: ML pipeline and performance-critical code
- **Python**: Model training and conversion tools

### ML Framework
- **TensorFlow Lite**: On-device inference
- **ONNX Runtime**: Model compatibility
- **Larod API**: Axis ML acceleration

### Key Libraries
- **OpenCV 4.8**: Computer vision (ARTPEC optimized)
- **MQTT 5.0**: Swarm communication
- **libcurl**: HTTP/REST APIs

## Performance Targets

| Metric | Requirement | Status |
|--------|-------------|--------|
| Inference Speed | <20ms per frame | Planning |
| Prediction Horizon | 5 minutes | Planning |
| False Positive Rate | <3% | Planning |
| Memory Footprint | <512MB | Planning |
| CPU Usage | <30% | Planning |
| Network Bandwidth | <500Kbps | Planning |

## Getting Started

### Prerequisites
- Docker Desktop installed
- Git
- 8GB+ RAM for development
- (Optional) Axis camera with ARTPEC-8 for testing

### Development Setup

1. Clone the repository:
```bash
git clone https://github.com/oneshot2001/OmniSight-dev.git
cd OmniSight-dev
```

2. Set up ACAP SDK Docker environment:
```bash
docker pull axisecp/acap-native-sdk:latest
```

3. Build the development container:
```bash
make build-env
```

4. Start development:
```bash
make dev
```

### Building the Application

```bash
# Build for target architecture
make build ARCH=aarch64

# Build package for deployment
make package

# Install on camera
make install CAMERA_IP=192.168.1.100
```

## Development Workflow

### Phase 1: Foundation (Weeks 1-4)
- [x] Project setup and documentation
- [x] Git repository and GitHub integration
- [ ] Docker development environment
- [ ] Core perception engine
- [ ] Basic Timeline Threading (30-second horizon)
- [ ] Swarm protocol v1

### Phase 2: Intelligence (Weeks 5-8)
- [ ] Extended Timeline Threading (5-minute horizon)
- [ ] Full swarm intelligence
- [ ] Conversational AI integration
- [ ] Self-configuration system

### Phase 3: Platform (Weeks 9-12)
- [ ] Developer SDK alpha
- [ ] Enterprise features
- [ ] Performance optimization
- [ ] ISC West demo preparation

## Key Features (PRD Reference)

### CR-001: Timeline Threading™
- 30 seconds to 5 minutes prediction horizon
- 3-5 parallel probability paths
- Confidence scoring 0-100%
- Real-time intervention suggestions

### CR-002: Swarm Intelligence
- <100ms threat propagation
- Federated learning without video sharing
- Multi-camera consensus
- Global threat database

### CR-003: Natural Language Interface
- 95% intent accuracy
- 7-day context memory
- Proactive communication
- 12 language support

### CR-004: Autonomous Operations
- <30 second setup
- 72-hour baseline learning
- Self-optimization
- 99.99% uptime

## Documentation

### Product & Requirements
- [Product Requirements Document](docs/axis-acap/Markdown%20versions/omnisight-prd.md)
- [Axis ACAP Documentation](docs/README.md)

### Development Guides
- [ACAP Packaging Guide](ACAP_PACKAGING.md) - Complete guide for building and deploying .eap packages
- [Phase 1 Status](PHASE1_STATUS.md) - Current development progress
- [Testing & Deployment](TESTING_AND_DEPLOYMENT.md) - Hardware requirements and deployment procedures
- [Stub Build Guide](STUB_BUILD.md) - Building and testing stub implementations

### Coming Soon
- [Architecture Design](docs/ARCHITECTURE.md)
- [API Reference](docs/API.md)

## Testing

```bash
# Run all tests
make test

# Run specific test suite
make test-perception
make test-timeline
make test-swarm

# Run on actual camera (requires camera connection)
make test-camera CAMERA_IP=192.168.1.100
```

## Contributing

This is a private development project. For team members:

1. Create a feature branch: `git checkout -b feature/your-feature`
2. Make changes and commit: `git commit -am 'Add feature'`
3. Push to branch: `git push origin feature/your-feature`
4. Create Pull Request

## Deployment

### Requirements
- Axis camera with ARTPEC-8 chipset
- AXIS OS 12.0 or later
- Minimum 2GB RAM, 4GB storage
- Network connectivity for swarm features

### Installation
```bash
# Package the application
make package

# Upload to camera via web interface or CLI
curl -k --anyauth -u root:pass -F file=@omnisight.eap \
  https://192.168.1.100/axis-cgi/applications/upload.cgi
```

## License

Proprietary - All Rights Reserved

## Contact & Support

- **GitHub Issues**: https://github.com/oneshot2001/OmniSight-dev/issues
- **Documentation**: See [docs/README.md](docs/README.md)

---

**OMNISIGHT: Every Camera Becomes Omniscient**

*Ship Date: 12 Weeks | Destiny: Inevitable | Impact: Civilization-Scale*
