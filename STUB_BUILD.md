# OMNISIGHT Stub Build Documentation

## Overview

The stub build allows you to develop and test OMNISIGHT without access to Axis camera hardware. All three core modules have been implemented as stubs that simulate realistic behavior:

- **Perception Engine** - Simulates object detection and tracking
- **Timeline Threading™** - Simulates predictive timelines and event prediction
- **Swarm Intelligence** - Simulates multi-camera coordination (disabled in demo mode)

## Quick Start

### Building

```bash
# In Docker container:
docker-compose up -d
docker-compose exec omnisight-dev bash
cd /opt/app
./scripts/build-stub.sh
```

This creates the executable at: `build-stub/omnisight`

### Running

```bash
# Demo mode (recommended for testing - disables swarm)
./build-stub/omnisight --demo

# Normal mode (includes swarm simulation)
./build-stub/omnisight

# Show help
./build-stub/omnisight --help
```

## What the Stubs Simulate

### Perception Stub (`src/perception/perception_stub.c`)

Simulates:
- 1-3 people walking across field of view
- Occasional vehicles passing by
- Realistic bounding boxes and trajectories
- Feature vectors for tracking
- Behavior detection (loitering, running, etc.)
- Threat score calculation
- Runs at 10 FPS target

### Timeline Stub (`src/timeline/timeline_stub.c`)

Simulates:
- 1-3 timeline branches based on tracked objects
- Event prediction (theft, trespassing, suspicious behavior)
- Severity calculation (low, medium, high, critical)
- Intervention point recommendations
- Probability scoring for each timeline

### Swarm Stub (`src/swarm/swarm_stub.c`)

Simulates:
- 2-4 neighboring cameras
- Track sharing with global IDs
- Consensus voting on events
- Network statistics tracking
- FOV overlap calculation

## Architecture

```
omnisight (main executable)
├── omnisight_core.c       # Integration layer
├── perception_stub.c      # Simulated perception
├── timeline_stub.c        # Simulated predictions
└── swarm_stub.c          # Simulated coordination
```

### Integration Layer (`src/omnisight_core.c`)

The core layer coordinates all modules:
- Initializes and manages module lifecycle
- Passes data between modules via callbacks
- Aggregates statistics from all modules
- Thread-safe shared state management
- Event notification system

### Data Flow

```
Perception Stub
    ↓ (tracks objects)
Timeline Stub
    ↓ (predicts events)
Swarm Stub
    ↓ (shares globally)
Event Callbacks
    ↓
Main Application
```

## Files Created

### Stub Implementations
- `src/perception/perception_stub.c` (~350 lines)
- `src/timeline/timeline_stub.c` (~400 lines)
- `src/swarm/swarm_stub.c` (~410 lines)

### Integration
- `src/omnisight_core.h` (222 lines) - Core API
- `src/omnisight_core.c` (461 lines) - Core implementation
- `src/main.c` (246 lines) - Application entry point

### Build System
- `scripts/build-stub.sh` - Stub build script
- Updated `CMakeLists.txt` files (removed hardware dependencies)

## Example Output

When running `./build-stub/omnisight --demo`:

```
╔═══════════════════════════════════════════════════════════════╗
║     OMNISIGHT - Precognitive Security System v0.1.0           ║
║     "See Everything. Understand Everything."                  ║
║     "Prevent Everything."                                     ║
╚═══════════════════════════════════════════════════════════════╝

[INFO] Running in demo mode
[OMNISIGHT] Initializing system...
[Perception] ✓ Stub engine initialized
[Timeline] ✓ Timeline Threading™ stub initialized
[OMNISIGHT] ✓ System initialized successfully

[EVENT:OBJECT_TRACKED] Tracked 2 objects
[EVENT:TIMELINE_UPDATED] Timelines updated
[EVENT:PREDICTED_EVENT] Event predicted: THEFT (severity: 0.60)
...
```

## Development Workflow

1. **Make changes** to stub implementations or core
2. **Rebuild**: `./scripts/build-stub.sh`
3. **Test**: `./build-stub/omnisight --demo`
4. **Iterate**

## Transitioning to Real Implementation

When you have access to Axis camera hardware:

1. Replace stub files with real implementations:
   - `perception_stub.c` → `perception.c` (with VDO/Larod APIs)
   - `timeline_stub.c` → `timeline.c` (with real prediction algorithms)
   - `swarm_stub.c` → `swarm.c` (with MQTT connectivity)

2. Update CMakeLists.txt to link hardware libraries:
   - Add VDO, Larod, OpenCV dependencies
   - Use ACAP SDK toolchain
   - Enable ARM cross-compilation

3. The `omnisight_core` layer remains unchanged - it's hardware-agnostic

## Troubleshooting

### Build fails with ARM architecture errors

The ACAP SDK Docker image has ARM cross-compilation flags enabled by default. Use the provided `build-stub.sh` script which bypasses CMake and uses direct gcc commands.

### "Cannot access 'build-stub.sh': No such file or directory"

The script must be in the `scripts/` directory which is mounted in the Docker container. Files in the project root are not mounted.

### Application crashes or hangs

Check that you're running with `--demo` flag to disable swarm networking, which requires MQTT broker configuration.

## Performance Characteristics

- **Perception**: ~10 FPS simulation rate
- **Timeline**: <50ms prediction generation
- **Memory**: ~44KB executable size
- **CPU**: Minimal usage (simulation is lightweight)

## Next Steps

1. Add more realistic simulation scenarios
2. Create test suite for stub implementations
3. Add configuration file support
4. Implement statistics dashboard
5. Add visualization of timelines and predictions

## Notes

- Stubs use only standard C libraries (no hardware dependencies)
- All modules are thread-safe
- Statistics are aggregated from all modules
- Demo mode disables swarm to avoid networking requirements
- Perfect for development, testing, and demonstration without hardware
