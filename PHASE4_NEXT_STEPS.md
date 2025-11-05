# Phase 4: Next Steps

> **Current Status**: Step 4 Complete (67% overall)
> **Last Updated**: 2025-11-05
> **Version**: v0.7.0

## Summary

Phase 4 has successfully delivered a **native C HTTP server with integrated React dashboard**. Steps 1-4 are complete and committed to GitHub. The package is ready for hardware testing.

### Completed Steps ✅

- ✅ **Step 1**: Mongoose HTTP server integration (3 hours)
- ✅ **Step 2**: 8 REST API endpoints implemented (4 hours)
- ✅ **Step 3**: Static file serving for React dashboard (2 hours)
- ✅ **Step 4**: ACAP build script automation (2 hours)

### Current Package Status

**Package Created**: `packages/OMNISIGHT_0_7_0_aarch64.eap`
- **Size**: 171KB (manifest + React dashboard only)
- **Missing**: ARM binary (requires Docker build)
- **Works**: ✓ Manifest valid, ✓ React assets included
- **Blocker**: Docker daemon not running on development machine

## Immediate Next Steps

### 1. Build ARM Binary (Step 5a)

**Requirement**: Docker daemon must be running

**Command**:
```bash
# Start Docker daemon
# Then run:
docker-compose up -d omnisight-dev
docker-compose run --rm omnisight-dev ./scripts/build-phase4-v070.sh
```

**Expected Output**:
- Complete package: `OMNISIGHT_0_7_0_aarch64.eap` (~800KB with binary)
- ARM binary: `omnisight` (~200KB, aarch64, static linked)
- Build time: ~30 seconds

**What This Produces**:
```
packages/OMNISIGHT_0_7_0_aarch64.eap (~800KB)
├── omnisight                 # ARM binary (NEW)
├── manifest.json             # ACAP config
├── package.conf              # Package metadata
├── omnisight_start.sh        # Startup script
├── LICENSE                   # MIT license
└── html/                     # React dashboard (617KB)
    ├── index.html
    └── assets/
        ├── index-*.js        # React bundle
        └── index-*.css       # Styles
```

### 2. Deploy to Camera (Step 5b)

**Requirement**: P3285-LVE camera with AXIS OS 12.0+

**Method 1: Web Interface** (Recommended)
```
1. Navigate to camera IP → Settings → Apps
2. Click "Add" button
3. Upload OMNISIGHT_0_7_0_aarch64.eap
4. Wait for installation (10-20 seconds)
5. Click "Open" button to access dashboard
```

**Method 2: CLI**
```bash
curl -k -u root:password \
  -F 'package=@packages/OMNISIGHT_0_7_0_aarch64.eap' \
  https://camera-ip/axis-cgi/applications/upload.cgi
```

**Method 3: SSH**
```bash
scp packages/OMNISIGHT_0_7_0_aarch64.eap root@camera-ip:/tmp/
ssh root@camera-ip
cd /tmp
acap-install OMNISIGHT_0_7_0_aarch64.eap
```

### 3. Basic Validation (Step 5c)

**Test Checklist**:
1. ✓ Application appears in camera Apps list
2. ✓ "Open" button present
3. ✓ Dashboard loads when clicked
4. ✓ All 8 API endpoints respond
5. ✓ React assets load (HTML/CSS/JS)
6. ✓ No errors in browser console
7. ✓ System log shows successful startup
8. ✓ Memory usage <256MB

**Quick API Test**:
```bash
# From camera or local network:
curl http://camera-ip/local/omnisight/api/health
curl http://camera-ip/local/omnisight/api/status
curl http://camera-ip/local/omnisight/api/perception/detections
```

**Expected Response Format**:
```json
{
  "status": "active",
  "version": "0.7.0",
  "modules": {
    "perception": "running",
    "timeline": "running",
    "swarm": "running"
  }
}
```

### 4. Advanced Testing (Step 5d)

**Performance Metrics**:
- API response time: <100ms per request
- Dashboard load time: <3 seconds
- Memory usage: stable at ~128MB
- CPU usage: <30% average
- 24-hour uptime: no crashes

**Full Test Suite**:
See [PHASE4_DEPLOYMENT_GUIDE.md](PHASE4_DEPLOYMENT_GUIDE.md) for:
- 8 basic functionality tests
- 8 advanced performance tests
- Troubleshooting guide
- Acceptance criteria

### 5. Documentation (Step 5e)

After successful hardware validation:
- Update PHASE4_SUMMARY.md with test results
- Create PHASE4_STEP5_COMPLETE.md
- Document any issues encountered
- Update README.md with v0.7.0 status

## What Works Right Now

**Without Docker build** (current state):
- ✅ Manifest-only package (171KB) is valid
- ✅ React dashboard assets included
- ✅ Can be uploaded to camera
- ❌ Binary missing (app won't start)

**With Docker build** (after Step 5a):
- ✅ Complete deployable package (~800KB)
- ✅ ARM binary included and executable
- ✅ Full functionality on camera
- ✅ Dashboard + API both operational

## Alternative: Manual ARM Build

If Docker unavailable, build can be done manually with ACAP SDK:

```bash
# On a Linux x86_64 machine with ACAP SDK installed:
export ARCH=aarch64
export OECORE_TARGET_SYSROOT=/opt/axis/sdk/sysroots/aarch64
export CC=aarch64-linux-gnu-gcc

aarch64-linux-gnu-gcc -O2 -Wall -Wextra \
  -I./src -I./src/perception -I./src/timeline -I./src/swarm -I./src/http \
  -DMG_ENABLE_PACKED_FS=0 -DOMNISIGHT_STUB_BUILD=1 \
  -DOMNISIGHT_VERSION=\"0.7.0\" -D_GNU_SOURCE \
  -o omnisight \
  src/main.c src/omnisight_core.c \
  src/http/http_server_simple.c src/http/mongoose.c \
  src/perception/perception_stub.c \
  src/timeline/timeline_stub.c \
  src/swarm/swarm_stub.c \
  -lpthread -lm -static

aarch64-linux-gnu-strip omnisight

# Copy to package-v070/
cp omnisight package-v070/

# Create .eap
cd package-v070
tar czf ../packages/OMNISIGHT_0_7_0_aarch64.eap .
```

## Success Criteria

Phase 4 will be considered **100% complete** when:

- [x] All 4 implementation steps done (Steps 1-4)
- [ ] ARM binary built successfully
- [ ] Package deployed to camera
- [ ] Dashboard accessible via "Open" button
- [ ] All 8 API endpoints functional on camera
- [ ] Basic tests pass (8/8)
- [ ] Advanced tests pass (8/8)
- [ ] 24-hour stability test passes
- [ ] Documentation updated with results

**Current Progress**: 67% (4/6 steps)

## After Phase 4

Once Phase 4 is complete, Phase 5 will begin:

### Phase 5: Hardware Integration

**Goal**: Replace stub implementations with real camera integration

**Tasks**:
1. **VDO API Integration** - Real video capture from camera
2. **Larod API Integration** - ML inference on DLPU
3. **Real Tracker** - Kalman filtering for trajectories
4. **MQTT Client** - Multi-camera communication
5. **Timeline Algorithms** - Actual future prediction
6. **Federated Learning** - Privacy-preserving swarm ML

**Estimated Timeline**: 2-3 weeks

**Success Criteria**: Real-time object detection and prediction operational

## Resources

- **Deployment Guide**: [PHASE4_DEPLOYMENT_GUIDE.md](PHASE4_DEPLOYMENT_GUIDE.md)
- **Phase 4 Summary**: [PHASE4_SUMMARY.md](PHASE4_SUMMARY.md)
- **Build Script**: [scripts/build-phase4-v070.sh](scripts/build-phase4-v070.sh)
- **Package**: [packages/OMNISIGHT_0_7_0_aarch64.eap](packages/OMNISIGHT_0_7_0_aarch64.eap)

## Contact

For issues or questions:
- GitHub Issues: https://github.com/oneshot2001/OmniSight-dev/issues
- Axis ACAP Docs: https://developer.axis.com/acap

---

**Phase 4 Status**: Implementation complete, awaiting hardware testing
**Blocker**: Docker daemon not running (required for ARM cross-compilation)
**Action Required**: Start Docker, run build script, deploy to camera
