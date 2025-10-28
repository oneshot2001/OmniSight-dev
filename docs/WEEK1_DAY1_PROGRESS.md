# Week 1, Day 1: ACAP SDK Environment Setup - COMPLETE ✅

**Date**: 2025-10-28
**Status**: Environment verified and ready for development
**Next**: Begin OMNISIGHT native implementation

---

## ✅ Accomplishments Today

### 1. **Environment Verification**
- ✅ Docker 28.3.0 running
- ✅ ACAP Native SDK examples cloned
- ✅ Build environment tested and working

### 2. **Reference Example Built Successfully**
**Built**: `web_server_rev_proxy_1_0_0_aarch64.eap`
**Size**: 379 KB
**Architecture**: aarch64 (ARM64 for ARTPEC-9)
**SDK Version**: 12.6.0

```bash
# Build command used:
docker build --tag acap-web-server:latest --build-arg ARCH=aarch64 .

# Result:
✅ Package created: web_server_rev_proxy_1_0_0_aarch64.eap (379 KB)
✅ Ready for deployment to Axis cameras
```

### 3. **Build Process Validated**
The Docker build successfully:
- Downloaded ACAP Native SDK (Ubuntu 24.04 based)
- Compiled CivetWeb library from source
- Cross-compiled C code for aarch64
- Packaged manifest.json and binaries into .eap
- Created deployment-ready package

---

## 📊 Key Learnings

### ACAP Build Environment:

**Docker Image Used:**
```
axisecp/acap-native-sdk:12.6.0-aarch64-ubuntu24.04
```

**Build Tools Included:**
- Cross-compiler for ARM (aarch64-linux-gnu-gcc)
- ACAP build tools (acap-build)
- CivetWeb source (compiled during build)
- pkg-config, make, standard dev tools

### Build Process Flow:

```
1. Pull ACAP SDK Docker image
2. Clone and compile CivetWeb library
3. Copy application source into container
4. Source ACAP environment setup
5. Run acap-build (compiles + packages)
6. Output: .eap file ready for deployment
```

### Package Contents (estimated):

```
web_server_rev_proxy_1_0_0_aarch64.eap (379 KB):
├── manifest.json           (~500 bytes)
├── package.conf           (~500 bytes)
├── web_server_rev_proxy   (~300 KB - binary)
├── libcivetweb.a          (~50 KB - static lib)
├── html/
│   ├── index.html
│   └── style.css
└── LICENSE
```

---

## 🎯 OMNISIGHT Adaptation Plan

### What We'll Copy from web-server Example:

1. **Dockerfile** - Build environment (modify for OMNISIGHT)
2. **Makefile** - Build configuration
3. **manifest.json** - ACAP configuration (update for OMNISIGHT)
4. **CivetWeb integration** - HTTP server setup
5. **Reverse proxy pattern** - Apache configuration

### What We'll Add for OMNISIGHT:

1. **Static file serving**:
   - html/ (dashboard from v0.2.4)
   - api/ (JSON files from v0.2.4)

2. **JSON generation** (Phase 2):
   - cJSON library integration
   - Dynamic API endpoints

3. **VDO/Larod integration** (Phase 3-4):
   - Video capture
   - ML inference

---

## 📁 Next Steps: Creating OMNISIGHT Native Structure

### Directory Structure to Create:

```
OmniSight-dev/
├── native/                  # NEW: Native C implementation
│   ├── app/
│   │   ├── omnisight.c     # Main application
│   │   ├── web_server.c    # CivetWeb HTTP server
│   │   ├── web_server.h    # Server header
│   │   ├── Makefile        # Build configuration
│   │   ├── manifest.json   # ACAP configuration
│   │   ├── html/           # Copy from app/html/
│   │   │   ├── index.html
│   │   │   └── style.css
│   │   └── api/            # Copy from app/api/
│   │       ├── health.json
│   │       ├── stats.json
│   │       └── ...
│   ├── Dockerfile          # ACAP SDK build environment
│   └── README.md           # Build instructions
├── app/                    # OLD: Python version (keep for reference)
├── scripts/                # Build helper scripts
└── docs/                   # Documentation
```

### Tasks for Tomorrow (Day 2):

1. **Create native/ directory structure**
2. **Copy web-server example as starting point**
3. **Modify manifest.json for OMNISIGHT**
4. **Copy HTML dashboard from app/html/**
5. **Copy JSON API files from app/api/**
6. **Update Dockerfile for OMNISIGHT**
7. **Test build**
8. **Deploy to P3285-LVR**

---

## 🔧 Technical Details

### Manifest.json Configuration:

```json
{
  "schemaVersion": "1.8.0",
  "acapPackageConf": {
    "setup": {
      "appName": "omnisight",
      "friendlyName": "OMNISIGHT - Precognitive Security",
      "vendor": "OMNISIGHT",
      "version": "0.3.0",
      "embeddedSdkVersion": "3.0",
      "runMode": "never",
      "architecture": "aarch64"
    },
    "configuration": {
      "settingPage": "index.html",
      "reverseProxy": [
        {
          "apiPath": "omnisight",
          "target": "http://localhost:8080",
          "access": "admin"
        }
      ]
    }
  }
}
```

**Key Changes from web-server example:**
- appName: `web_server_rev_proxy` → `omnisight`
- friendlyName: Added descriptive name
- vendor: Updated to OMNISIGHT
- version: `1.0.0` → `0.3.0`
- apiPath: `my_web_server` → `omnisight`
- target port: `2001` → `8080`
- settingPage: Added dashboard reference

---

## 📝 Build Commands Reference

### Build OMNISIGHT Native (when ready):

```bash
cd "/Users/matthewvisher/Omnisight dev/OmniSight-dev/native"

# Build for P3285-LVR (aarch64/ARTPEC-9)
docker build --tag omnisight-native:0.3.0 --build-arg ARCH=aarch64 .

# Extract .eap file
docker run --rm omnisight-native:0.3.0 cat /opt/app/omnisight_*.eap > omnisight_0_3_0_aarch64.eap

# Verify package
ls -lh omnisight_0_3_0_aarch64.eap
```

### Deploy to Camera:

```bash
# Via Web UI:
https://camera-ip/#settings/apps → Upload → Select .eap

# Via CLI:
curl -u root:password \
  -F 'package=@omnisight_0_3_0_aarch64.eap' \
  https://camera-ip/axis-cgi/applications/upload.cgi
```

---

## ✅ Success Criteria for Week 1

By end of Week 1, we should have:

1. **Package Built**:
   - ✅ omnisight_0_3_0_aarch64.eap created
   - ✅ Size: ~400-500 KB (similar to web-server example)

2. **Deployment Successful**:
   - ✅ Installs on P3285-LVR without errors
   - ✅ Service starts and stays running

3. **Dashboard Working**:
   - ✅ Accessible at: `https://camera-ip/local/omnisight/`
   - ✅ Loads HTML/CSS correctly
   - ✅ No "Service Unavailable" errors

4. **API Endpoints Working**:
   - ✅ `/api/health.json` returns static JSON
   - ✅ `/api/stats.json` returns static JSON
   - ✅ Test buttons work!

---

## 🎯 Current Status Summary

| Task | Status | Notes |
|------|--------|-------|
| Clone ACAP examples | ✅ Complete | At: `/Users/matthewvisher/Omnisight dev/acap-native-sdk-examples/` |
| Verify Docker environment | ✅ Complete | Docker 28.3.0, SDK 12.6.0 |
| Build reference example | ✅ Complete | 379 KB .eap created successfully |
| Understand build process | ✅ Complete | Dockerfile, Makefile, CivetWeb integration |
| Create OMNISIGHT structure | 🔄 Tomorrow | Day 2 task |
| First native build | 📅 Day 3 | After structure is ready |
| Deploy to camera | 📅 Day 4 | After successful build |

---

## 🚀 Tomorrow's Goals (Day 2)

1. Create `native/` directory with complete structure
2. Copy and modify web-server example files
3. Integrate OMNISIGHT HTML dashboard
4. Integrate OMNISIGHT API JSON files
5. Test local build
6. Prepare for camera deployment

**Estimated Time**: 4-6 hours of development

---

## 📚 References

- **ACAP SDK Docs**: https://axiscommunications.github.io/acap-documentation/
- **Web Server Example**: `/Users/matthewvisher/Omnisight dev/acap-native-sdk-examples/web-server/`
- **CivetWeb Docs**: https://github.com/civetweb/civetweb/blob/master/docs/UserManual.md
- **Implementation Plan**: [ACAP_NATIVE_IMPLEMENTATION_PLAN.md](ACAP_NATIVE_IMPLEMENTATION_PLAN.md)

---

**Day 1 Status: ✅ COMPLETE**

All environment setup and validation completed. Ready to begin OMNISIGHT native implementation tomorrow!
