# OMNISIGHT IPC Quick Start Guide

**For Developers**: Fast track to understanding and implementing IPC

## TL;DR

We use **3 IPC methods**:
1. **Shared memory** (fast) - Detections, tracks, timelines
2. **Named pipes** (commands) - Config updates, control
3. **JSON files** (status) - Stats, debugging

Target latency: **<1ms** for reads.

## File Locations

```
/Users/matthewvisher/Omnisight dev/OmniSight-dev/
├── docs/
│   ├── IPC_ARCHITECTURE.md          ← Full specification (1060 lines)
│   ├── IPC_IMPLEMENTATION_SUMMARY.md ← Integration guide
│   └── IPC_QUICKSTART.md            ← This file
├── src/ipc/
│   ├── ipc_server.h                 ← C API (406 lines)
│   └── ipc_server.c                 ← C implementation skeleton (645 lines)
└── app/ipc/
    ├── __init__.py
    └── ipc_client.py                ← Python client (488 lines)
```

## 5-Minute Overview

### C Side (Publisher)

```c
#include "ipc/ipc_server.h"

// 1. Initialize
IPCServerConfig config;
ipc_server_get_default_config(&config);
config.command_callback = my_command_handler;

IPCServer* ipc = ipc_server_init(&config);
ipc_server_start(ipc);

// 2. Publish detections (every frame, 10 Hz)
ipc_server_publish_detections(ipc, detections, num_detections);
ipc_server_publish_tracks(ipc, tracks, num_tracks);

// 3. Publish timelines (every second, 1 Hz)
ipc_server_publish_timelines(ipc, timelines, num_timelines);

// 4. Update stats (every second)
ipc_server_update_stats(ipc, &stats);

// 5. Cleanup
ipc_server_stop(ipc);
ipc_server_destroy(ipc);
```

### Python Side (Consumer)

```python
from app.ipc import OmnisightIPCClient

# 1. Initialize
client = OmnisightIPCClient()

# 2. Read data (non-blocking, cached if no new data)
detections = client.get_detections()  # Returns: {'frame_id': 123, 'detections': [...]}
tracks = client.get_tracks()           # Returns: {'timestamp': 456, 'tracks': [...]}
timelines = client.get_timelines()     # Returns: {'timestamp': 789, 'timelines': [...]}

# 3. Read status
stats = client.get_stats()             # From JSON file
status = client.get_status()           # From JSON file

# 4. Send commands
success = client.send_command('CONFIG_UPDATE', {'threshold': 0.7})

# 5. Health check
health = client.health_check()  # {'shm_accessible': True, 'data_fresh': True, ...}

# 6. Cleanup
client.close()
```

## Data Flow

```
┌─────────────────────────────────────┐
│  C CORE (10 fps)                    │
│  - Detections                       │
│  - Tracks                           │
│  - Timelines                        │
└───────────┬─────────────────────────┘
            │ Write
            ▼
┌─────────────────────────────────────┐
│  SHARED MEMORY                      │
│  /omnisight_detections (5 KB)      │
│  /omnisight_tracks (8 KB)          │
│  /omnisight_timelines (20 KB)      │
└───────────┬─────────────────────────┘
            │ Read (<1ms)
            ▼
┌─────────────────────────────────────┐
│  PYTHON FLASK API                   │
│  GET /api/perception/detections     │
│  GET /api/perception/tracks         │
│  GET /api/timeline/predictions      │
└─────────────────────────────────────┘
```

## Key Data Structures

### Detection Frame (5 KB)
```c
struct DetectionFrame {
  uint32_t magic;           // "OMNI"
  uint64_t frame_id;
  uint64_t timestamp_ms;
  uint32_t num_detections;
  DetectedObject objects[50];
  uint32_t checksum;
};
```

### Python Parsed Result
```python
{
  'frame_id': 12345,
  'timestamp': 1698765432000,
  'detections': [
    {
      'id': 1,
      'class_id': 1,  # PERSON
      'confidence': 0.95,
      'bbox': (0.1, 0.2, 0.5, 0.8),
      'timestamp_ms': 1698765432000
    }
  ]
}
```

## Implementation Checklist

### Phase 2.1: Shared Memory (2-3 days)
- [ ] C: Implement `ipc_create_shared_memory()`
- [ ] C: Implement `ipc_server_publish_detections()`
- [ ] C: Implement `ipc_server_publish_tracks()`
- [ ] C: Implement `ipc_server_publish_timelines()`
- [ ] Python: Implement `_setup_shared_memory()`
- [ ] Python: Implement `get_detections()` with binary parsing
- [ ] Python: Implement `get_tracks()` with binary parsing
- [ ] Python: Implement `get_timelines()` with binary parsing
- [ ] Test: End-to-end detection flow
- [ ] Test: Verify <1ms latency
- [ ] Test: Memory leak check (valgrind)

### Phase 2.2: Named Pipes (1-2 days)
- [ ] C: Implement `ipc_create_pipes()`
- [ ] C: Implement `ipc_command_handler_thread()`
- [ ] C: Implement command parsing (JSON)
- [ ] C: Implement `ipc_server_send_ack()`
- [ ] Python: Implement `send_command()`
- [ ] Test: CONFIG_UPDATE command
- [ ] Test: Command timeout handling
- [ ] Test: Invalid JSON error handling

### Phase 2.3: JSON Files (1 day)
- [ ] C: Implement `ipc_server_update_stats()` (atomic write)
- [ ] C: Implement `ipc_server_update_status()`
- [ ] Python: Implement `get_stats()` with JSON parsing
- [ ] Python: Implement `get_status()` with JSON parsing
- [ ] Test: Atomic writes (no partial reads)
- [ ] Test: JSON parse error handling

### Phase 2.4: Integration (1 day)
- [ ] Integrate IPC into omnisight_core.c
- [ ] Create Flask API endpoints using IPC client
- [ ] Full system test (C → Python → HTTP)
- [ ] Load test (100 concurrent requests)
- [ ] Stress test (sustained 10 Hz for 24 hours)
- [ ] Performance profiling
- [ ] Documentation update

## Common Tasks

### Adding a New Data Type

**C Side:**
1. Define struct in `ipc_server.h`
2. Add shared memory region name
3. Add semaphore name
4. Implement `ipc_server_publish_xyz()` function

**Python Side:**
1. Add `get_xyz()` method to `OmnisightIPCClient`
2. Implement binary parsing for struct
3. Add caching logic

### Debugging IPC

```bash
# C side: Check resources
ls -la /dev/shm/omnisight_*        # Shared memory
ls -la /dev/shm/sem.omnisight_*    # Semaphores
ls -la /tmp/omnisight_*            # Pipes and JSON files

# Inspect shared memory
hexdump -C /dev/shm/omnisight_detections | head -20

# Monitor JSON files
watch -n 1 cat /tmp/omnisight_stats.json

# Python side: Enable debug logging
import logging
logging.basicConfig(level=logging.DEBUG)
```

### Performance Testing

```python
# Measure read latency
import time

client = OmnisightIPCClient()

latencies = []
for _ in range(10000):
    start = time.perf_counter()
    detections = client.get_detections()
    end = time.perf_counter()
    latencies.append((end - start) * 1000)  # ms

print(f"Mean: {sum(latencies)/len(latencies):.2f} ms")
print(f"P95: {sorted(latencies)[int(len(latencies)*0.95)]:.2f} ms")
print(f"P99: {sorted(latencies)[int(len(latencies)*0.99)]:.2f} ms")
```

## Dependencies

### Install Python IPC Library
```bash
pip install posix_ipc
```

### Install cJSON (C side)
```bash
# Ubuntu/Debian
sudo apt-get install libcjson-dev

# Or include in project
git submodule add https://github.com/DaveGamble/cJSON.git external/cjson
```

## Error Handling

### Common Errors

**SharedMemory not found (Python)**
```
posix_ipc.ExistentialError: No such file or directory
```
**Solution**: C core not started. Start C core first.

**Checksum mismatch**
```
IPC: Checksum validation failed
```
**Solution**: C/Python struct mismatch. Verify struct definitions match exactly.

**Pipe broken (Python)**
```
BrokenPipeError: [Errno 32] Broken pipe
```
**Solution**: C core crashed. Check C core logs, restart.

**Stale data (Python)**
```
IPC: Data older than 5 seconds
```
**Solution**: C core frozen. Check C core status, restart if needed.

## Performance Targets

| Metric | Target | How to Verify |
|--------|--------|---------------|
| Read latency | <1ms | Run latency test above |
| Command latency | <100ms | Send command, measure time to ack |
| Throughput | 10 fps sustained | Monitor for 1 hour, check dropped frames |
| Memory usage | <1 MB IPC overhead | Monitor RSS of both processes |
| Error rate | <0.1% | Count checksum failures / total reads |

## Resources

- **Full Architecture**: docs/IPC_ARCHITECTURE.md (1060 lines)
- **Implementation Guide**: docs/IPC_IMPLEMENTATION_SUMMARY.md
- **C API Reference**: src/ipc/ipc_server.h (406 lines, fully documented)
- **Python Client**: app/ipc/ipc_client.py (488 lines with TODOs)

## Questions?

1. Read docs/IPC_ARCHITECTURE.md sections 1-3 (Overview, Requirements, Methods)
2. Review data structures in src/ipc/ipc_server.h
3. Check TODOs in src/ipc/ipc_server.c and app/ipc/ipc_client.py
4. Follow implementation roadmap in docs/IPC_IMPLEMENTATION_SUMMARY.md

---

**Status**: Design Complete, Ready for Implementation
**Estimated Implementation Time**: 5-6 days
**Target Latency**: <1ms ✓
**Target Throughput**: 10 fps ✓
