# OMNISIGHT API Endpoints Reference

Complete reference for all REST API endpoints in OMNISIGHT Phase 2.

## Base URL

When deployed to camera:
```
https://<camera-ip>/local/omnisight/api/
```

When running locally:
```
http://localhost:8080/api/
```

## Authentication

Phase 2: No authentication (development mode)
Phase 3+: Will use Axis camera authentication via reverse proxy

## Response Format

All endpoints return JSON responses with appropriate HTTP status codes:

- `200 OK`: Successful request
- `400 Bad Request`: Invalid request data
- `404 Not Found`: Endpoint not found
- `500 Internal Server Error`: Server error

## Endpoints

### System

| Endpoint | Method | Description |
|----------|--------|-------------|
| `/api/health` | GET | Health check for monitoring |
| `/api/stats` | GET | System statistics and metrics |

### Perception

| Endpoint | Method | Description |
|----------|--------|-------------|
| `/api/perception/status` | GET | Perception module status and FPS |
| `/api/perception/detections` | GET | Current frame detections with bounding boxes |

### Timeline

| Endpoint | Method | Description |
|----------|--------|-------------|
| `/api/timeline/predictions` | GET | Active timeline predictions |
| `/api/timeline/history` | GET | Past events and outcomes |

### Swarm

| Endpoint | Method | Description |
|----------|--------|-------------|
| `/api/swarm/network` | GET | Network topology and health |
| `/api/swarm/cameras` | GET | List of cameras in swarm |

### Configuration

| Endpoint | Method | Description |
|----------|--------|-------------|
| `/api/config` | GET | Current system configuration |
| `/api/config` | POST | Update configuration |

## Detailed Endpoint Documentation

### GET /api/health

Health check endpoint for monitoring system availability.

**Request:**
```bash
curl https://camera-ip/local/omnisight/api/health
```

**Response:**
```json
{
  "status": "healthy",
  "version": "0.2.0",
  "timestamp": "2025-10-22T16:00:00Z"
}
```

---

### GET /api/stats

Returns comprehensive system statistics including uptime, resource usage, and module status.

**Request:**
```bash
curl https://camera-ip/local/omnisight/api/stats
```

**Response:**
```json
{
  "uptime_seconds": 86400,
  "memory": {
    "used_mb": 128,
    "allocated_mb": 256,
    "usage_percent": 50
  },
  "cpu": {
    "usage_percent": 35,
    "cores": 4
  },
  "modules": {
    "perception": {
      "status": "running",
      "fps": 10.2,
      "frames_processed": 367200
    },
    "timeline": {
      "status": "running",
      "active_predictions": 3,
      "total_predictions": 1420
    },
    "swarm": {
      "status": "running",
      "connected_cameras": 3,
      "messages_sent": 5234,
      "messages_received": 4891
    }
  },
  "timestamp": "2025-10-22T16:00:00Z"
}
```

---

### GET /api/perception/status

Returns perception module status and performance metrics.

**Request:**
```bash
curl https://camera-ip/local/omnisight/api/perception/status
```

**Response:**
```json
{
  "enabled": true,
  "fps": 10.2,
  "resolution": "1920x1080",
  "objects_tracked": 3,
  "module": "perception",
  "last_update": "2025-10-22T16:00:00Z"
}
```

---

### GET /api/perception/detections

Returns current frame detections with bounding boxes and tracking IDs.

**Request:**
```bash
curl https://camera-ip/local/omnisight/api/perception/detections
```

**Response:**
```json
{
  "frame_id": 12345,
  "timestamp": "2025-10-22T16:00:00Z",
  "detections": [
    {
      "id": 1,
      "type": "person",
      "bbox": [100, 200, 50, 150],
      "confidence": 0.95,
      "track_id": "T001"
    },
    {
      "id": 2,
      "type": "vehicle",
      "bbox": [400, 300, 120, 80],
      "confidence": 0.87,
      "track_id": "T002"
    }
  ]
}
```

**Bounding Box Format:** `[x, y, width, height]` in pixels

---

### GET /api/timeline/predictions

Returns active timeline predictions with probable future events.

**Request:**
```bash
curl https://camera-ip/local/omnisight/api/timeline/predictions
```

**Response:**
```json
{
  "active_timelines": 3,
  "predictions": [
    {
      "timeline_id": "TL001",
      "probability": 0.75,
      "horizon_seconds": 300,
      "events": [
        {
          "type": "TRESPASSING",
          "severity": "medium",
          "time_offset": 45,
          "location": [500, 600],
          "confidence": 0.82
        },
        {
          "type": "LOITERING",
          "severity": "low",
          "time_offset": 120,
          "location": [520, 610],
          "confidence": 0.68
        }
      ]
    }
  ]
}
```

**Event Types:**
- `TRESPASSING`: Unauthorized area entry
- `LOITERING`: Prolonged presence
- `VEHICLE_STOPPED`: Stopped vehicle
- `NORMAL_ACTIVITY`: Normal behavior

**Severity Levels:** `none`, `low`, `medium`, `high`, `critical`

---

### GET /api/timeline/history

Returns historical events with prediction accuracy data.

**Request:**
```bash
curl https://camera-ip/local/omnisight/api/timeline/history
```

**Response:**
```json
{
  "total_events": 156,
  "time_range": {
    "start": "2025-10-22T00:00:00Z",
    "end": "2025-10-22T16:00:00Z"
  },
  "events": [
    {
      "event_id": "E001",
      "timestamp": "2025-10-22T14:30:00Z",
      "predicted_type": "TRESPASSING",
      "actual_type": "TRESPASSING",
      "prediction_accuracy": 0.95,
      "timeline_id": "TL045",
      "severity": "medium"
    }
  ]
}
```

---

### GET /api/swarm/network

Returns swarm network topology and health status.

**Request:**
```bash
curl https://camera-ip/local/omnisight/api/swarm/network
```

**Response:**
```json
{
  "local_camera_id": "CAM001",
  "neighbors": [
    {
      "id": "CAM002",
      "distance": 25.5,
      "signal_strength": -45,
      "shared_tracks": 2,
      "last_seen": "2025-10-22T16:00:00Z",
      "status": "online"
    }
  ],
  "network_health": "good",
  "total_cameras": 3,
  "mqtt_connected": true
}
```

**Network Health Values:** `excellent`, `good`, `degraded`, `poor`

---

### GET /api/swarm/cameras

Returns list of all cameras in the swarm network.

**Request:**
```bash
curl https://camera-ip/local/omnisight/api/swarm/cameras
```

**Response:**
```json
{
  "cameras": [
    {
      "id": "CAM001",
      "name": "Front Entrance",
      "location": [0, 0],
      "status": "online",
      "role": "master",
      "uptime_seconds": 86400
    },
    {
      "id": "CAM002",
      "name": "Parking Lot",
      "location": [25.5, 0],
      "status": "online",
      "role": "worker",
      "uptime_seconds": 82800
    }
  ],
  "total_count": 2
}
```

**Camera Roles:** `master`, `worker`
**Camera Status:** `online`, `offline`, `degraded`

---

### GET /api/config

Returns current system configuration.

**Request:**
```bash
curl https://camera-ip/local/omnisight/api/config
```

**Response:**
```json
{
  "perception": {
    "enabled": true,
    "fps_target": 10,
    "resolution": "1920x1080",
    "confidence_threshold": 0.7
  },
  "timeline": {
    "enabled": true,
    "horizon_seconds": 300,
    "max_timelines": 5,
    "prediction_threshold": 0.5
  },
  "swarm": {
    "enabled": true,
    "mqtt_broker": "localhost",
    "mqtt_port": 1883,
    "discovery_enabled": true
  },
  "system": {
    "log_level": "INFO",
    "max_memory_mb": 256
  }
}
```

---

### POST /api/config

Updates system configuration (partial updates supported).

**Request:**
```bash
curl -X POST https://camera-ip/local/omnisight/api/config \
  -H "Content-Type: application/json" \
  -d '{
    "perception": {
      "enabled": false
    }
  }'
```

**Response:**
```json
{
  "success": true,
  "message": "Configuration updated successfully",
  "config": {
    "perception": {
      "enabled": false,
      "fps_target": 10,
      "resolution": "1920x1080",
      "confidence_threshold": 0.7
    },
    "timeline": { ... },
    "swarm": { ... },
    "system": { ... }
  }
}
```

**Error Response (400):**
```json
{
  "success": false,
  "error": "No configuration data provided"
}
```

---

## Rate Limiting

Phase 2: No rate limiting
Phase 3+: Will implement rate limiting per IP/user

## CORS

CORS is enabled for development. All origins are allowed in Phase 2.

## Testing

Test all endpoints with curl:

```bash
# Health check
curl http://localhost:8080/api/health

# Get perception status
curl http://localhost:8080/api/perception/status

# Get detections
curl http://localhost:8080/api/perception/detections

# Get predictions
curl http://localhost:8080/api/timeline/predictions

# Get swarm network
curl http://localhost:8080/api/swarm/network

# Get system stats
curl http://localhost:8080/api/stats

# Update config
curl -X POST http://localhost:8080/api/config \
  -H "Content-Type: application/json" \
  -d '{"perception":{"fps_target":15}}'
```

## Notes

- All timestamps are in ISO 8601 format with UTC timezone (Z suffix)
- Phase 2 returns stub data for demonstration
- Phase 3+ will connect to C core for real data via IPC
- Bounding boxes use pixel coordinates relative to video frame
- Confidence values range from 0.0 to 1.0

---

**Document Version:** 1.0.0
**Last Updated:** 2025-10-22
**OMNISIGHT Phase:** 2
