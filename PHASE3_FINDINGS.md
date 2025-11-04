# OMNISIGHT Phase 3 Findings - Python Availability Issue

**Date**: November 4, 2025
**Camera**: Axis P3285-LVE (ARTPEC-9)
**Firmware**: AXIS OS 12.6.97

## Critical Discovery

**The P3285-LVE camera does not have Python 3 installed.**

### Test Results:

**v0.6.0 (Flask API):**
- ❌ Error: `python3: not found`
- Issue: Hardcoded `python3` command

**v0.6.1 (Fixed Python Path):**
- ❌ Error: `ERROR: Python 3 not found!`
- Checked: `/usr/bin/python3`, `/usr/bin/python`
- Result: Neither exists on this camera

### Log Evidence:

```
2025-11-04T16:04:35.340 [ INFO ] omnisightv2[86419]: ERROR: Python 3 not found!
2025-11-04T16:04:35.340 [ INFO ] omnisightv2[86419]: Checked: /usr/bin/python3, /usr/bin/python
```

## Analysis

### Python Support in AXIS OS

According to Axis documentation:
- **Python support varies** by camera model and firmware
- **Not all ACAP-enabled cameras** include Python runtime
- **ARTPEC-8/9 cameras** may or may not have Python pre-installed
- **M4228-LVE** (ARTPEC-8) - May have Python (not tested)
- **P3285-LVE** (ARTPEC-9) - **CONFIRMED: No Python**

### Implications for OMNISIGHT

**Flask API Backend (Phase 3):**
- ❌ Cannot run on P3285-LVE
- ❌ Requires Python 3 runtime
- ❌ Requires Flask, Flask-CORS packages

**Static Dashboard (v0.5.1):**
- ✅ Works perfectly on P3285-LVE
- ✅ No Python required
- ✅ Demo mode with mock data
- ✅ Full UI functionality
- ✅ Proven stable (1+ hour runtime)

## Successful Version: v0.5.1 (Demo Mode)

### What Works:

**Package**: `OMNISIGHT_v2_DEMO_MODE_0_5_1.eap` (170KB)

**Features:**
- ✅ Schema 1.5 (simple, proven)
- ✅ Static HTML/JS/CSS dashboard
- ✅ No backend server required
- ✅ Mock data in JavaScript
- ✅ All 6 dashboard tabs functional
- ✅ Open button appears reliably
- ✅ Runs indefinitely without errors

**Test Results:**
- Deployed: Multiple times successfully
- Runtime: 1+ hour stable
- Open button: 100% success rate
- Dashboard: Fully functional
- Memory: Stable, no leaks
- Errors: None (only license warnings - non-critical)

### Why It Works:

1. **No Server Process**: Just static files served by Apache
2. **No Dependencies**: Pure HTML/JS/CSS
3. **No Python**: Doesn't need any runtime
4. **No IPC**: Mock data embedded in JavaScript
5. **Minimal Pattern**: Schema 1.5 + settingPage

## Path Forward: Three Options

### Option A: Static Dashboard Only (Recommended for P3285-LVE)

**Use v0.5.1 as the production version for P3285-LVE.**

**Pros:**
- ✅ Works perfectly right now
- ✅ No dependencies
- ✅ Proven stable
- ✅ Great for demos/presentations
- ✅ Full UI showcase

**Cons:**
- ⚠️ No real data (mock only)
- ⚠️ No backend processing
- ⚠️ Can't save configs

**Best For:**
- Product demonstrations
- UI/UX showcases
- Client presentations
- Testing dashboard design
- P3285-LVE cameras specifically

### Option B: Compiled C Binary (For Python-less Cameras)

**Build a native C/C++ binary that serves HTTP.**

**Approach:**
1. Use **microhttpd** or **mongoose** C library
2. Compile for aarch64 with ACAP SDK
3. Serve static files + API endpoints
4. Direct IPC with C core modules

**Pros:**
- ✅ No Python dependency
- ✅ Native performance
- ✅ Single binary deployment
- ✅ Direct C integration
- ✅ Works on any ACAP camera

**Cons:**
- ⚠️ Complex cross-compilation
- ⚠️ Larger development effort
- ⚠️ More debugging complexity
- ⚠️ HTTP library integration needed

**Effort**: 2-3 weeks

### Option C: Target Different Camera Model (For Flask Backend)

**Deploy Flask version to M4228-LVE or other Python-enabled cameras.**

**Requirements:**
1. Find Axis camera with Python 3 pre-installed
2. Verify Flask package availability
3. Test v0.6.x series

**Pros:**
- ✅ Flask backend works as designed
- ✅ Real API endpoints
- ✅ Dynamic configuration
- ✅ Python ecosystem benefits

**Cons:**
- ⚠️ Hardware dependency
- ⚠️ Not universal solution
- ⚠️ May need different firmware

**Status**: Requires hardware testing

## Recommendations

### For Immediate Use:

**Deploy v0.5.1 (Demo Mode) to P3285-LVE**
- Stable, proven, works perfectly
- Great for demonstrations
- Full UI functionality
- Zero maintenance

### For Future Development:

**Priority 1: Document v0.5.1 as "OMNISIGHT Demo Edition"**
- Position as product showcase version
- Create marketing materials
- Use for client demos
- Perfect for trade shows

**Priority 2: Investigate Compiled Binary Approach**
- Research **mongoose** or **microhttpd**
- Prototype simple HTTP server in C
- Test cross-compilation with ACAP SDK
- Build proof-of-concept

**Priority 3: Test Flask on Different Camera**
- Try M4228-LVE (if available)
- Check for Python availability
- Verify Flask can be installed
- Document results

### For Phase 3 Revival (If Compiled Binary Route):

**OMNISIGHT v0.7.0 - Native Binary Edition**

**Architecture:**
```
┌─────────────────────────────────────────┐
│         P3285-LVE Camera                │
│  ┌───────────────────────────────────┐  │
│  │      OMNISIGHT ACAP Package       │  │
│  │                                   │  │
│  │  ┌─────────────────────────────┐  │  │
│  │  │   Apache (settingPage)      │  │  │
│  │  │   Serves: html/index.html   │  │  │
│  │  └─────────────────────────────┘  │  │
│  │                                   │  │
│  │  ┌─────────────────────────────┐  │  │
│  │  │  omnisight_server (Binary)  │  │  │
│  │  │  - Compiled C/C++           │  │  │
│  │  │  - Mongoose HTTP server     │  │  │
│  │  │  - Port: 8080               │  │  │
│  │  │  - API endpoints            │  │  │
│  │  └──────────┬──────────────────┘  │  │
│  │             │ Direct calls        │  │
│  │  ┌──────────▼──────────────────┐  │  │
│  │  │   C Core Modules            │  │  │
│  │  │   - perception.c            │  │  │
│  │  │   - timeline.c              │  │  │
│  │  │   - swarm.c                 │  │  │
│  │  └─────────────────────────────┘  │  │
│  └───────────────────────────────────┘  │
└─────────────────────────────────────────┘
```

**Benefits:**
- Works on ALL ACAP cameras (no Python needed)
- Native performance
- Direct C integration
- Single binary deployment
- Smaller package size

**Libraries to Consider:**
1. **mongoose** - Embedded web server (MIT license)
   - HTTP server in single C file
   - WebSocket support
   - JSON parsing
   - ~50KB binary size

2. **microhttpd** - GNU libmicrohttpd
   - More features
   - Well-documented
   - Active development

3. **civetweb** - Fork of mongoose
   - Modern C/C++
   - Good documentation

## Conclusion

**For P3285-LVE specifically:**
- ✅ v0.5.1 (Demo Mode) is the **working, stable, production-ready** version
- ❌ v0.6.x (Flask API) cannot run without Python
- 🔄 Future: Native C binary server (v0.7.0+)

**Status Summary:**
- **Phase 1**: ✅ Complete (C stubs)
- **Phase 2**: ✅ Complete (React dashboard)
- **Phase 3**: ⚠️ Blocked on Python dependency
  - **Workaround**: Use v0.5.1 (static demo)
  - **Future**: Compile native HTTP server

**Current Production Version for P3285-LVE:**
**OMNISIGHT v0.5.1 - Demo Mode**
- Package: `OMNISIGHT_v2_DEMO_MODE_0_5_1.eap` (170KB)
- Status: Production-ready for demo/showcase use
- Tested: 1+ hour stable runtime
- Open button: 100% success rate
