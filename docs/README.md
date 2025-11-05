# OMNISIGHT Documentation Index

This directory contains all technical documentation for the OMNISIGHT Precognitive Security System.

## 🚀 Quick Start

- **New to OMNISIGHT?** Start with [Installation Guide](guides/INSTALLATION_GUIDE.md)
- **Deployment Issue?** Check [Quick Troubleshooting](troubleshooting/QUICK_TROUBLESHOOTING.md)
- **Building Packages?** See [ACAP Packaging Guide](acap/ACAP_PACKAGING.md)

## ⚠️ Camera Compatibility

| Camera Model | ARTPEC | Python 3 | Recommended Package | Status |
|--------------|--------|----------|---------------------|---------|
| **M4228-LVE** | ARTPEC-8 | ✅ Yes | Phase 4 V3 (Native) v0.4.2 | Tested ✅ |
| **P3285-LVE** | ARTPEC-9 | ❌ No | Phase 4 V3 (Native) v0.4.2 | Tested ✅ |
| **Unknown** | Any | Any | Phase 4 V3 (Native) v0.4.2 | Universal ✅ |

**Always use**: `./scripts/build-phase4-v3-native-eap.sh` (works on all cameras)

## 📚 Documentation by Category

### User Guides (`guides/`)
- [Installation Guide](guides/INSTALLATION_GUIDE.md) - Complete installation instructions
- [Testing & Deployment](guides/TESTING_AND_DEPLOYMENT.md) - Hardware requirements and deployment
- [Quick Start Guide](guides/QUICK_START_GUIDE.md) - Get up and running fast
- [Timeline User Guide](guides/TIMELINE_USER_GUIDE.md) - Using Timeline Threading™
- [Swarm Intelligence Guide](guides/SWARM_INTELLIGENCE_GUIDE.md) - Multi-camera coordination
- [Stub Build Guide](guides/STUB_BUILD.md) - Building stub implementations

### Developer Documentation (`development/`)
- [IPC Architecture](development/IPC_ARCHITECTURE.md) - Inter-process communication design
- [IPC Implementation](development/IPC_IMPLEMENTATION_SUMMARY.md) - Implementation details
- [IPC Quick Start](development/IPC_QUICKSTART.md) - Get started with IPC
- [Architecture Document](development/ARCHITECTURE.md) - System architecture overview
- [Development Log](development/DEVELOPMENT_LOG.md) - Development history

### ACAP Platform (`acap/`)
- [ACAP Packaging Guide](acap/ACAP_PACKAGING.md) - Complete packaging guide (405 lines)
- [ACAP Insights from Pandosme](acap/ACAP_INSIGHTS_FROM_PANDOSME.md) - Best practices analysis
- [ACAP Native Implementation Plan](ACAP_NATIVE_IMPLEMENTATION_PLAN.md) - Native C implementation
- [Release Notes v0.2.1](RELEASE_NOTES_v0.2.1.md) - Phase 2 release notes

### Troubleshooting (`troubleshooting/`)
- [Quick Troubleshooting](troubleshooting/QUICK_TROUBLESHOOTING.md) - Fast diagnosis guide ⚡
- **Deployment Issues** (`troubleshooting/deployment-fixes/`)
  - [Phase 4 Python Dependency Issue](troubleshooting/deployment-fixes/PHASE4_PYTHON_DEPENDENCY_ISSUE.md) - Critical fix ⚠️
  - [Documentation Cleanup Summary](troubleshooting/deployment-fixes/DOCUMENTATION_CLEANUP_SUMMARY.md) - Change tracking
  - [Phase 4 P3285 LVE Testing](troubleshooting/deployment-fixes/PHASE4_P3285_LVE_TESTING.md) - P3285 test results

## 📅 Documentation by Phase

### Phase 1: Stub Implementation (`phase1/`)
- [Phase 1 Status](phase1/PHASE1_STATUS.md) - Implementation status (60% complete)
- [Phase 1 Summary](phase1/PHASE1_SUMMARY.md) - Overview and accomplishments

### Phase 2: API Server (`phase2/`)
- [Phase 2 Roadmap](phase2/PHASE2_ROADMAP.md) - 7-task implementation plan
- [Phase 2 Overview](phase2/PHASE2_OVERVIEW.md) - Flask API architecture
- [Phase 2 Summary](phase2/PHASE2_SUMMARY.md) - Progress summary
- [IPC Implementation](phase2/IPC_IMPLEMENTATION.md) - IPC layer details
- [Release Notes v0.2.1](RELEASE_NOTES_v0.2.1.md) - Phase 2 release

### Phase 3: Hardware Integration (`phase3/`)
- [Phase 3 VDO Integration](phase3/PHASE3_VDO_INTEGRATION.md) - Video capture integration
- [Phase 3 Larod Integration](phase3/PHASE3_LAROD_INTEGRATION.md) - ML inference via DLPU
- [Phase 3 Progress Summary](phase3/PHASE3_PROGRESS_SUMMARY.md) - Overall progress
- [Phase 3 Summary](phase3/PHASE3_SUMMARY.md) - Achievements and status
- **Tasks** (`phase3/tasks/`)
  - [Task 1: VDO API Integration](phase3/tasks/TASK1_VDO_API.md)
  - [Task 2: Larod API Integration](phase3/tasks/TASK2_LAROD_API.md)
  - [Task 3: Hybrid Approach](phase3/tasks/TASK3_HYBRID_APPROACH.md)
  - [Task 4: MQTT Communication](phase3/tasks/TASK4_MQTT_COMMUNICATION.md)
  - [Task 5: Optimization](phase3/tasks/TASK5_OPTIMIZATION.md)
  - [Task 6: Docker Testing](phase3/tasks/TASK6_DOCKER_TESTING.md)
  - [Task 7: Hardware Testing](phase3/tasks/TASK7_HARDWARE_TESTING.md)
  - [Task 8: Production Deployment](phase3/tasks/TASK8_PRODUCTION_DEPLOYMENT.md)

### Phase 4: Claude Flow Integration (`phase4/`)
- [Phase 4 Overview](phase4/PHASE4_OVERVIEW.md) - Distributed AI coordination
- [Phase 4 Coordination Architecture](phase4/PHASE4_COORDINATION_ARCHITECTURE.md) - Swarm design
- [Phase 4 Federated Learning](phase4/PHASE4_FEDERATED_LEARNING.md) - Privacy-preserving ML
- [Phase 4 Dashboard](phase4/PHASE4_DASHBOARD.md) - Interactive visualization
- [Phase 4 Timeline Engine](phase4/PHASE4_TIMELINE_ENGINE.md) - Neural timeline prediction
- [Phase 4 Summary](phase4/PHASE4_SUMMARY.md) - Progress and achievements
- [Phase 4 Python Dependency Issue](troubleshooting/deployment-fixes/PHASE4_PYTHON_DEPENDENCY_ISSUE.md) - Critical fix ⚠️

## 🏗️ Axis ACAP Platform Reference (`axis-acap/`)

### `/axis-acap/`
Complete Axis development documentation organized by topic:

#### Core Platform Documentation
- **ACAP/** - Core ACAP platform documentation
- **ACAP version 12/** - ACAP version 12 specific docs, API references, compatibility guides
- **Documentation/** - General Axis development guides and knowledge base articles

#### API & Integration
- **Vapix/** - VAPIX API documentation (device configuration, network video, applications)
- **AOA/** - Axis Object Analytics documentation
- **Amalytic & Metadata/** - Scene metadata and analytics data formats

#### Specialized Topics
- **Vision/** - Computer vision on-device, DLPU documentation, model optimization
- **Video Streaming and Recording/** - Video protocols, Zipstream, signed video
- **Scene Metadata/** - Scene metadata integration guides
- **Guidlines/** - Development and testing guidelines

#### Reference Materials
- **Axis White Papers/** - Technical white papers
- **Axis Files/** - Additional Axis resources
- **Research/** - Industry research and pain points
- **Markdown versions/** - PRD and strategy documents (omnisight-prd.md, neuraxis-prd.md, etc.)

## Key Documents for OMNISIGHT Development

### Must-Read for Getting Started
1. [ACAP version 12 Main](axis-acap/ACAP%20version%2012/) - Start here for ACAP 4.0+ development
2. [Computer Vision on Device](axis-acap/Vision/) - DLPU, model optimization, TensorFlow Lite
3. [Scene Metadata Integration](axis-acap/Scene%20Metadata/) - For swarm intelligence and data sharing
4. [VAPIX API](axis-acap/Vapix/) - Device control and configuration APIs

### Critical for OMNISIGHT Features
1. **Timeline Threading™ (Precognitive Engine)**
   - Vision/DLPU documentation for transformer models
   - Computer vision optimization guides

2. **Swarm Intelligence**
   - Amalytic & Metadata/Message Broker (MQTT)
   - Scene Metadata protocols

3. **Natural Language Interface**
   - ACAP API documentation
   - Integration patterns

4. **Performance Requirements**
   - Vision/optimization-tips
   - Vision/quantization guides
   - Recommended model architectures

## Product Requirements
- **omnisight-prd.md** - Main product requirements document (located in Markdown versions/)
- **neuraxis-prd.md** - Alternative PRD version
- **sentinel-core-strategy.md** - Core strategy document

## Usage Notes
- All documents are copied from the source iCloud Drive location
- Updated: October 19, 2025
- Organized for quick reference during development

## 🔧 Build Scripts & Tools

### Current Builds (Recommended)
- `./scripts/build-phase4-v3-native-eap.sh` - **Universal build** (works on all cameras) ✅
  - Package: v0.4.2 (308 KB)
  - No Python dependency
  - Tested on M4228-LVE and P3285-LVE

### Legacy Builds (Not Recommended)
- `./scripts/deprecated/build-phase2-eap.sh` - Requires Python 3 ⚠️
- `./scripts/deprecated/build-phase4-eap.sh` - Requires Python 3 ⚠️
- `./scripts/deprecated/build-phase4-fixed-eap.sh` - Requires Python 3 ⚠️
- `./scripts/deprecated/build-phase4-v2-eap.sh` - Requires Python 3 ⚠️

### Testing Tools
- `./scripts/testing/validate_flask_app.sh` - Validate Flask API server
- `./scripts/testing/QUICK_API_TEST.sh` - Quick API endpoint testing

## 📦 Packages

All pre-built packages are in `packages/`:

- `packages/current/` - Latest recommended package (v0.4.2)
- `packages/phase2/` - Phase 2 packages (Python required)
- `packages/phase3/` - Phase 3 packages (hardware integration)
- `packages/phase4/` - Phase 4 packages (various versions)

## 🚨 Common Issues & Solutions

### No "Open" Button After Installation
**Symptom**: App installs and shows as "Running" but no "Open" button appears

**Root Cause**: Camera doesn't have Python 3 installed (common on P3285-LVE/ARTPEC-9)

**Solution**: Use Phase 4 V3 (Native) build:
```bash
./scripts/build-phase4-v3-native-eap.sh
```

**Details**: [Phase 4 Python Dependency Issue](troubleshooting/deployment-fixes/PHASE4_PYTHON_DEPENDENCY_ISSUE.md)

### Application Won't Stop
**Symptom**: Toggle stays ON, app won't stop when requested

**Root Cause**: Service crashed but systemd keeps restarting it

**Solution**:
1. Check system logs for errors
2. Use Phase 4 V3 (Native) build
3. See [Quick Troubleshooting](troubleshooting/QUICK_TROUBLESHOOTING.md)

### Build Fails in Docker
**Symptom**: CMake errors, architecture mismatches

**Root Cause**: ACAP SDK forces ARM cross-compilation

**Solution**: Use `build-stub.sh` for local testing:
```bash
./scripts/build-stub.sh
./build-stub/omnisight --demo
```

## 📊 Project Status

**Current Phase**: Phase 4 V3 (Native) - Universal ARTPEC-8/9 Compatibility

**Version**: v0.4.2

**Status**: Production-ready for deployment

### Completion Status by Phase
- ✅ **Phase 1** (60% Complete): Stub implementations
- ✅ **Phase 2** (100% Complete): Flask API server
- ⏳ **Phase 3** (In Progress): Hardware integration (VDO, Larod, MQTT)
- ✅ **Phase 4** (100% Complete): Claude Flow coordination + Universal packaging

### Recent Milestones
- ✅ Phase 4 V3 (Native) deployed to P3285-LVE successfully (Oct 31, 2025)
- ✅ Solved Python dependency issue for ARTPEC-9 cameras
- ✅ Comprehensive documentation cleanup and organization
- ✅ Created universal build that works on all Axis cameras

## 📖 Archive

Historical documentation and old versions are in `archive/`:
- Project logs and weekly progress reports
- Deprecated documentation
- Old planning documents

## 🆘 Getting Help

1. **Quick Issues**: Check [Quick Troubleshooting](troubleshooting/QUICK_TROUBLESHOOTING.md)
2. **Deployment Issues**: See [troubleshooting/deployment-fixes/](troubleshooting/deployment-fixes/)
3. **Building Packages**: Read [ACAP Packaging Guide](acap/ACAP_PACKAGING.md)
4. **Phase-Specific**: Check relevant phase documentation above

## 🔗 External Resources

- **Axis Developer Portal**: https://developer.axis.com/
- **ACAP SDK**: https://github.com/AxisCommunications/acap-sdk
- **ACAP Examples**: https://github.com/AxisCommunications/acap-native-sdk-examples

## 📝 Next Steps

### For New Developers
1. Read [Installation Guide](guides/INSTALLATION_GUIDE.md)
2. Review [ACAP Packaging Guide](acap/ACAP_PACKAGING.md)
3. Build Phase 4 V3: `./scripts/build-phase4-v3-native-eap.sh`
4. Deploy to test camera

### For Existing Developers
1. Review [Phase 4 Python Dependency Issue](troubleshooting/deployment-fixes/PHASE4_PYTHON_DEPENDENCY_ISSUE.md)
2. Update all deployments to v0.4.2
3. Continue with Phase 3 hardware integration

### For Troubleshooting
1. Start with [Quick Troubleshooting](troubleshooting/QUICK_TROUBLESHOOTING.md)
2. Check [deployment-fixes/](troubleshooting/deployment-fixes/) for specific issues
3. Review camera-specific compatibility in Camera Compatibility Matrix above

---

**Last Updated**: October 31, 2025
**Documentation Version**: 2.0
**Project Version**: v0.4.2 (Phase 4 V3 Native)
