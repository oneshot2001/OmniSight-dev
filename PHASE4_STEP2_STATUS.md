# Phase 4 Step 2 Status: API Endpoint Implementation

> **Status**: In Progress
> **Started**: 2025-11-04
> **Target Version**: v0.7.0

## Overview

Step 2 focuses on implementing full API endpoint logic with real OMNISIGHT core integration. This step builds on the successful Mongoose integration from Step 1.

## Progress Summary

### ✅ Completed

1. **Created Simplified HTTP Server** (`http_server_simple.c`)
   - Uses correct Mongoose API (`mg_match()` instead of `mg_http_match_uri()`)
   - Proper CORS handling
   - Static file serving configured
   - All endpoint routes implemented

2. **Created Integration Test Program** (`test_integration.c`)
   - Proper OMNISIGHT core initialization
   - HTTP server creation and lifecycle management
   - Event loop with both HTTP and core processing

3. **Build Infrastructure**
   - Created `scripts/build-integration-test.sh`
   - Direct gcc compilation (bypasses CMake issues)
   - Includes all module dependencies

### 🔄 In Progress

1. **API Data Structure Alignment**
   - Need to match `OmnisightStats` actual fields:
     - `perception_stats.avg_fps` (not `fps`)
     - `perception_stats.tracked_objects` (not `active_tracks`)
     - `timeline_stats.active_timelines` (not `timelines_generated`)

2. **Configuration Structure**
   - Need to use correct config fields:
     - `enable_perception` (not `perception_enabled`)
     - `enable_timeline` (not `timeline_enabled`)
     - `enable_swarm` (not `swarm_enabled`)

3. **Mongoose API Compatibility**
   - `mg_str` structure doesn't have `.ptr` member
   - Need to use Mongoose string functions correctly
   - `mg_vcmp()` function needs proper declaration

### ⏸️ Blocked Items

1. **Compilation Dependencies**
   - `perception.c` requires `glib.h` (GLib library)
   - `timeline.c` has struct redefinition issues
   - Need to use stub implementations only for now

## Current Code Status

### Files Created

1. **[src/http/http_server_simple.c](src/http/http_server_simple.c)** (442 lines)
   - Simplified server using correct API
   - All endpoint handlers implemented
   - Uses `omnisight_get_stats()`, `omnisight_get_tracked_objects()`, `omnisight_get_timelines()`

2. **[src/http/test_integration.c](src/http/test_integration.c)** (119 lines)
   - Full integration test
   - Correct core initialization
   - Proper event loop

3. **[scripts/build-integration-test.sh](scripts/build-integration-test.sh)**
   - Build script for integration testing
   - Links all modules correctly

### Compilation Errors to Fix

```
Error Categories:
1. Config struct field names (3 errors)
2. Stats struct field names (8 errors)
3. Mongoose mg_str usage (15 errors)
4. Module dependencies (glib.h, struct redefinitions)
```

## Next Actions

### Immediate (Today)

1. Fix `OmnisightConfig` initialization:
   ```c
   OmnisightConfig config = {
       .enable_perception = true,  // Not perception_enabled
       .enable_timeline = true,
       .enable_swarm = true
   };
   ```

2. Fix `OmnisightStats` field access:
   ```c
   stats.perception_stats.avg_fps  // Not fps
   stats.perception_stats.tracked_objects  // Not active_tracks
   stats.timeline_stats.active_timelines  // Not timelines_generated
   ```

3. Fix Mongoose string handling:
   - Use `mg_str()` helper correctly
   - Build JSON strings with proper memory management
   - Remove `.ptr` access

### Short-term (This Week)

1. Create stub-only build variant
   - Exclude `perception.c`, `timeline.c`, `swarm.c`
   - Use only stub implementations
   - Avoid GLib and other dependencies

2. Test all endpoints return valid JSON

3. Verify data accuracy from stubs

4. Create automated endpoint test script

## Architecture Decisions

### Why Simplified Server?

The original `http_server.c` used incorrect API calls throughout. Rather than fix 50+ errors, we created `http_server_simple.c` with:

- Correct Mongoose API usage
- Correct OMNISIGHT core API calls
- Simpler JSON building (less prone to errors)
- Focus on functionality over features

### Why Stub-Only Build?

Full module compilation requires:
- GLib (for VDO/Larod APIs)
- Proper header alignment
- ACAP SDK environment

For local testing, stub-only builds are:
- Faster to compile
- No external dependencies
- Sufficient for API validation
- Same data structures

## Success Criteria

### Step 2 Complete When:

- [ ] All endpoint handlers compile without errors
- [ ] Integration test builds successfully
- [ ] All endpoints return valid JSON
- [ ] Data matches omnisight_core state
- [ ] No memory leaks in JSON generation
- [ ] CORS headers work correctly
- [ ] Static file serving functional

### Test Requirements:

```bash
# All these should return valid JSON:
curl http://localhost:8081/api/health
curl http://localhost:8081/api/status
curl http://localhost:8081/api/stats
curl http://localhost:8081/api/perception/status
curl http://localhost:8081/api/perception/detections
curl http://localhost:8081/api/timeline/predictions
curl http://localhost:8081/api/swarm/network
curl http://localhost:8081/api/config
```

## Estimated Completion

- **Original Estimate**: 1-2 days
- **Time Spent**: 2 hours
- **Remaining**: 4-6 hours
- **Blockers**: API alignment, Mongoose string handling

## Key Learnings

1. **Always check actual struct definitions** - Don't assume field names
2. **Mongoose has version differences** - Check header for actual API
3. **Build incrementally** - Stub-only first, then full integration
4. **Test simple before complex** - `test_http_simple` validated routing before data

## Related Documentation

- [PHASE4_NATIVE_HTTP_SERVER.md](PHASE4_NATIVE_HTTP_SERVER.md) - Overall Phase 4 plan
- [src/omnisight_core.h](src/omnisight_core.h) - Core API reference
- [src/http/http_server_simple.c](src/http/http_server_simple.c) - Simplified implementation

## References

- Mongoose Documentation: https://mongoose.ws/documentation/
- OMNISIGHT Core API: `src/omnisight_core.h` lines 127-220
- Perception API: `src/perception/perception.h` lines 137-200
- Timeline API: `src/timeline/timeline.h` lines 280-360
- Swarm API: `src/swarm/swarm.h` lines 180-250

---

**Last Updated**: 2025-11-04
**Next Update**: After fixing compilation errors
