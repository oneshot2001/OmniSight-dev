# OMNISIGHT Testing and Deployment Options

## Your Question

**Does Axis have a testing environment to test Omnisight, or is the best way to do it on Axis ARTPEC 8/9 hardware? If hardware is the best way, how do I get it on the hardware to test? How do I load the OMNISIGHT ACAP to test?**

## Short Answer

**No, Axis does not provide a virtual testing environment or simulator for ACAP applications.** The best (and only reliable) way to test OMNISIGHT is on **actual Axis hardware** with an ARTPEC-8/9 chipset. However, you can develop and test much of the functionality using **stub implementations** (which you now have working!).

## ✅ Package Build Status

**OMNISIGHT .eap package successfully created!**
- **File**: `OMNISIGHT_Precognitive_Security_0_1_0_aarch64.eap`
- **Size**: 20KB
- **Location**: Project root directory
- **Build Command**: `./scripts/build-eap.sh` (run in Docker)

---

## Testing Options

### Option 1: Stub Development (✅ Currently Working)

**What you have now:**
- Full stub implementation of all three modules (Perception, Timeline, Swarm)
- Runs in Docker without hardware
- Validates architecture and integration
- Perfect for development and feature iteration

**Limitations:**
- No real video capture (simulated objects only)
- No real ML inference (random walk simulation)
- No hardware acceleration testing
- Can't validate actual camera performance

**Use cases:**
- Algorithm development
- Integration testing
- Architecture validation
- Demonstration and training
- CI/CD pipeline

### Option 2: ACAP SDK Docker Environment (⚠️ Limited)

**What it provides:**
- Cross-compilation toolchain for aarch64
- ACAP SDK libraries and headers
- Build and package creation tools

**What it does NOT provide:**
- VDO (video capture) API - Needs real camera
- Larod (ML inference) API - Needs DLPU hardware
- Actual ARTPEC-8 DLPU acceleration
- Real camera sensors and optics
- Network camera features

**Limitations:**
```
┌─────────────────────────────────────┐
│  ACAP SDK Docker Container          │
│  ✅ Compile code                    │
│  ✅ Link libraries                  │
│  ✅ Create .eap package             │
│  ❌ Run VDO APIs                    │
│  ❌ Run Larod APIs                  │
│  ❌ Test ML inference               │
│  ❌ Capture video                   │
└─────────────────────────────────────┘
```

### Option 3: Real Axis Hardware (🎯 Required for Production)

**This is the ONLY way to fully test:**
- Video capture via VDO API
- ML inference via Larod API
- DLPU hardware acceleration
- Real-world performance metrics
- Actual camera integration
- End-to-end functionality

---

## Getting Hardware

### Recommended Cameras for OMNISIGHT

From [DEPLOYMENT.md](DEPLOYMENT.md#hardware-requirements), compatible ARTPEC-8 cameras:

| Model | Type | Key Features | Price Range |
|-------|------|--------------|-------------|
| **AXIS Q1656-LE** | Bullet | 4K, Outdoor, ARTPEC-8, DLPU | $600-800 |
| **AXIS M3068-P** | Panoramic | 360°, Indoor, ARTPEC-8 | $700-900 |
| **AXIS P3265-LVE** | Dome | Outdoor, Vandal-resistant, ARTPEC-8 | $500-700 |
| **AXIS P3267-LVE** | Dome | IR, Outdoor, ARTPEC-8, DLPU | $600-800 |

### Where to Buy

1. **Axis Partner Portal** (Best for developers)
   - https://www.axis.com/partners
   - Developer discount programs available
   - Technical support included

2. **Authorized Distributors**
   - ADI Global
   - Graybar
   - Anixter/WESCO
   - ScanSource

3. **Online Retailers**
   - B&H Photo Video
   - Amazon Business
   - CDW
   - Newegg Business

### Development Program

**Axis Developer Partnership:**
- Sign up at https://www.axis.com/partners/acap-partner-program
- Get developer discounts on cameras
- Access to technical support
- Early access to new features
- Marketing support for your ACAP

---

## Deployment Process

### Overview

```
┌────────────────┐      ┌─────────────────┐      ┌────────────────┐
│  Development   │      │  Build & Package │      │  Deploy to     │
│  Environment   │  →   │  (.eap file)     │  →   │  Camera        │
│  (Your code)   │      │                  │      │  (Test/Prod)   │
└────────────────┘      └─────────────────┘      └────────────────┘
```

### Step 1: Build for Hardware

Currently you have stub implementations. To build for real hardware:

```bash
# Enter Docker container
docker-compose exec omnisight-dev bash
cd /opt/app

# Build with CMake (for hardware)
mkdir -p build-hw && cd build-hw
cmake .. \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_TOOLCHAIN_FILE=/opt/axis/acapsdk/sysroots/x86_64-pokysdk-linux/usr/share/cmake/axis-toolchain.cmake

make -j4
```

**Note:** This will fail right now because you need to replace stubs with real VDO/Larod implementations.

### Step 2: Create ACAP Package (.eap file)

The `.eap` file is an ACAP application package that can be installed on cameras.

```bash
# Create the package
eap-install.sh create \
  -m manifest.json \
  -s build-hw/staging \
  -o omnisight_1_0_0_aarch64.eap
```

**What's in manifest.json:**
- Application name and version
- Resource requirements (RAM, CPU, storage)
- Permissions needed
- Web interface configuration
- API endpoints

See [DEPLOYMENT.md](DEPLOYMENT.md#step-4-create-acap-package) for full manifest example.

### Step 3: Deploy to Camera

You have **three methods** to load OMNISIGHT onto a camera:

#### Method A: Web Interface (Easiest)

1. Open browser: `https://192.168.1.100` (your camera IP)
2. Login with admin credentials
3. Go to **Settings → Apps**
4. Click **Add** button
5. Select `omnisight_1_0_0_aarch64.eap` file
6. Click **Install**
7. Click **Start**

```
┌─────────────────────────────────────────┐
│  Camera Web Interface                   │
│                                         │
│  Settings                               │
│  ├── System                             │
│  ├── Video                              │
│  ├── Audio                              │
│  └── 📱 Apps  ← Click here              │
│      ├── Installed apps                 │
│      └── ➕ Add app  ← Upload .eap here │
└─────────────────────────────────────────┘
```

#### Method B: Command Line (Scriptable)

```bash
# Set camera credentials
CAMERA_IP="192.168.1.100"
CAMERA_USER="root"
CAMERA_PASS="yourpassword"

# Upload ACAP package
curl -u $CAMERA_USER:$CAMERA_PASS \
  -F "package=@omnisight_1_0_0_aarch64.eap" \
  https://$CAMERA_IP/axis-cgi/applications/upload.cgi

# Start application
curl -u $CAMERA_USER:$CAMERA_PASS \
  "https://$CAMERA_IP/axis-cgi/applications/control.cgi?action=start&package=omnisight"

# Check status
curl -u $CAMERA_USER:$CAMERA_PASS \
  "https://$CAMERA_IP/axis-cgi/applications/list.cgi"
```

**Advantages:**
- Can script for multiple cameras
- Easy automation
- CI/CD integration

#### Method C: SSH (Advanced)

```bash
# SSH into camera
ssh root@192.168.1.100

# Upload from local machine
scp omnisight_1_0_0_aarch64.eap root@192.168.1.100:/tmp/

# On camera: Extract and install
cd /tmp
# ACAP packages are installed automatically via web interface or API
# Manual installation not recommended

# View logs
journalctl -u omnisight -f
```

### Step 4: Monitor and Debug

```bash
# Check application status
curl -u root:password \
  "https://192.168.1.100/axis-cgi/applications/list.cgi" | jq

# View logs
ssh root@192.168.1.100 "journalctl -u omnisight -n 100"

# Check resource usage
ssh root@192.168.1.100 "top -b -n 1 | grep omnisight"

# Access OMNISIGHT web interface
open https://192.168.1.100/local/omnisight/
```

---

## Current Status & Next Steps

### ✅ What You Have Now

1. **Working stub implementation**
   - All three modules functional
   - Integration layer complete
   - Can develop and test without hardware
   - Perfect for demonstrations

2. **Build environment**
   - Docker container with ACAP SDK
   - Cross-compilation toolchain
   - All development tools

3. **Documentation**
   - STUB_BUILD.md - How to build stubs
   - PHASE1_STATUS.md - What's complete
   - DEPLOYMENT.md - Full deployment guide

### 🔨 What You Need for Hardware Testing

1. **Axis Camera** with ARTPEC-8
   - Recommended: AXIS Q1656-LE or P3267-LVE
   - Cost: ~$600-800
   - Where: Axis partner program or authorized distributor

2. **Replace Stubs with Real Implementations**

   Need to implement:
   ```
   src/perception/vdo_capture.c      - Video capture
   src/perception/larod_inference.c  - ML inference
   src/perception/tracker.c          - Object tracking
   src/timeline/trajectory_predictor.c - Predictions
   src/swarm/mqtt_client.c           - Network communication
   ```

3. **ML Model**
   - Trained TensorFlow Lite model
   - Quantized for DLPU (INT8)
   - See [models/TRAINING.md](models/TRAINING.md)

4. **MQTT Broker** (for swarm testing)
   - Mosquitto or HiveMQ
   - Can run in Docker or cloud

### 📋 Recommended Path Forward

**Option A: Continue Stub Development** (No hardware needed yet)
- Implement more realistic simulation
- Add test scenarios
- Develop algorithm prototypes
- Perfect for MVP demonstration

**Option B: Acquire Hardware for Integration** (Production path)
1. Purchase one ARTPEC-8 camera (~$600)
2. Implement VDO and Larod integration
3. Deploy and test on real hardware
4. Iterate based on real-world performance
5. Scale to multiple cameras

**Option C: Hybrid Approach** (Recommended)
- Continue stub development for rapid iteration
- Acquire 1-2 cameras for hardware validation
- Test critical paths on hardware
- Use stubs for non-hardware features

---

## Frequently Asked Questions

### Q: Can I test without buying a camera?

**A:** You can develop and test most functionality with stubs (which you have now), but you cannot test:
- Real video capture
- ML inference performance
- DLPU acceleration
- Actual detection accuracy
- End-to-end latency
- Camera-specific features

### Q: Do I need multiple cameras to test swarm intelligence?

**A:** No, you can test swarm with:
- 1 camera + stub nodes (simulated neighbors)
- 2-3 cameras for real multi-camera testing
- Full swarm needs 5+ cameras for meaningful results

### Q: Can I use older Axis cameras (ARTPEC-7)?

**A:** No. OMNISIGHT requires:
- ARTPEC-8 or ARTPEC-9 chipset
- DLPU for ML acceleration
- AXIS OS 12.0+
- VDO API 3.0+
- Larod API 3.0+

### Q: How long does deployment take?

**A:**
- Upload .eap: ~30 seconds
- Installation: ~1 minute
- First start: ~10 seconds
- Total: **~2 minutes per camera**

### Q: Can I update the ACAP without uninstalling?

**A:** Yes:
```bash
# Upload new version
curl -u root:password -F "package=@omnisight_1_0_1_aarch64.eap" \
  https://camera-ip/axis-cgi/applications/upload.cgi

# Application will restart with new version
```

### Q: What if I don't have a network?

**A:** You can:
- Connect camera directly to laptop via Ethernet
- Use camera's default IP: `192.168.0.90`
- Or set camera to DHCP and check router

### Q: Can I access the camera remotely for testing?

**A:** Yes, but setup needed:
- VPN to camera network (recommended)
- Port forwarding (security risk)
- Axis cloud services (requires subscription)
- SSH tunneling

---

## Summary

### Testing Environment: No Virtual Option

❌ Axis does **NOT** provide:
- Virtual camera simulator
- Emulated ARTPEC environment
- Software-only testing platform

✅ You **CAN** use:
- Stub implementations (you have this!)
- ACAP SDK for building
- Partial testing in Docker

### Hardware Required For:

| Feature | Stub OK? | Hardware Needed? |
|---------|----------|------------------|
| Algorithm development | ✅ Yes | ❌ No |
| Integration testing | ✅ Yes | ❌ No |
| Architecture validation | ✅ Yes | ❌ No |
| Video capture | ❌ No | ✅ Yes |
| ML inference | ❌ No | ✅ Yes |
| DLPU acceleration | ❌ No | ✅ Yes |
| Performance testing | ❌ No | ✅ Yes |
| Production deployment | ❌ No | ✅ Yes |

### Deployment Process

1. **Build** → Create `.eap` package
2. **Upload** → Web UI, curl, or SSH
3. **Install** → Automatic via camera
4. **Start** → Launch application
5. **Monitor** → Check logs and metrics

### Cost to Get Started

- **1 ARTPEC-8 camera**: ~$600-800
- **Axis developer program**: Free
- **Development tools**: Free (ACAP SDK)
- **MQTT broker**: Free (Mosquitto)

**Total hardware cost**: ~$600-800 for minimal testing setup

### Your Current Position

You're in a **great place**:
- ✅ Full stub implementation working
- ✅ Can develop without hardware
- ✅ Architecture validated
- ✅ Build environment ready
- 📋 Next: Acquire camera + implement hardware APIs

---

## Additional Resources

- **ACAP Documentation**: https://developer.axis.com/acap/
- **Partner Program**: https://www.axis.com/partners/acap-partner-program
- **Developer Support**: https://www.axis.com/developer-community
- **Sample Code**: https://github.com/AxisCommunications/acap-native-sdk-examples
- **Your Deployment Guide**: [DEPLOYMENT.md](DEPLOYMENT.md)
- **Your Development Guide**: [DEVELOPMENT.md](DEVELOPMENT.md)

---

**Bottom Line**: You need real Axis hardware to fully test OMNISIGHT. The good news is you've built a solid stub implementation that lets you develop most features without hardware. When you're ready, one $600-800 camera will unlock full testing capabilities.
