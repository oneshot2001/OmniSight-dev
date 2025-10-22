# OMNISIGHT Flask API Server

REST API server for OMNISIGHT Precognitive Security System Phase 2.

## Overview

This Flask application provides a REST API interface for the OMNISIGHT system, exposing endpoints for perception, timeline predictions, swarm intelligence, configuration, and system statistics.

**Current Status**: Phase 2 - Returns stub data for all endpoints. Will be connected to C core via IPC in later phases.

## Architecture

```
Flask API Server (Port 8080)
├── Static HTML serving (/)
├── API endpoints (/api/*)
│   ├── Perception (/api/perception/*)
│   ├── Timeline (/api/timeline/*)
│   ├── Swarm (/api/swarm/*)
│   ├── Config (/api/config)
│   └── System (/api/stats, /api/health)
└── Error handlers (404, 500)
```

## API Endpoints

### System Endpoints

#### GET /api/health
Health check endpoint for monitoring.

**Response:**
```json
{
  "status": "healthy",
  "version": "0.2.0",
  "timestamp": "2025-10-22T16:00:00Z"
}
```

#### GET /api/stats
Returns comprehensive system statistics.

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
    "perception": { "status": "running", "fps": 10.2 },
    "timeline": { "status": "running", "active_predictions": 3 },
    "swarm": { "status": "running", "connected_cameras": 3 }
  }
}
```

### Perception Endpoints

#### GET /api/perception/status
Returns perception module status and performance metrics.

**Response:**
```json
{
  "enabled": true,
  "fps": 10.2,
  "resolution": "1920x1080",
  "objects_tracked": 3
}
```

#### GET /api/perception/detections
Returns current frame detections with bounding boxes.

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
    }
  ]
}
```

### Timeline Endpoints

#### GET /api/timeline/predictions
Returns active timeline predictions with probable future events.

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
        }
      ]
    }
  ]
}
```

#### GET /api/timeline/history
Returns past events and their outcomes.

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
      "prediction_accuracy": 0.95
    }
  ]
}
```

### Swarm Endpoints

#### GET /api/swarm/network
Returns swarm network topology and health status.

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
      "status": "online"
    }
  ],
  "network_health": "good"
}
```

#### GET /api/swarm/cameras
Returns list of all cameras in the swarm network.

**Response:**
```json
{
  "cameras": [
    {
      "id": "CAM001",
      "name": "Front Entrance",
      "location": [0, 0],
      "status": "online",
      "role": "master"
    }
  ]
}
```

### Configuration Endpoints

#### GET /api/config
Returns current system configuration.

**Response:**
```json
{
  "perception": {
    "enabled": true,
    "fps_target": 10,
    "resolution": "1920x1080"
  },
  "timeline": {
    "enabled": true,
    "horizon_seconds": 300
  },
  "swarm": {
    "enabled": true,
    "mqtt_broker": "localhost"
  }
}
```

#### POST /api/config
Updates system configuration.

**Request Body:**
```json
{
  "perception": {
    "enabled": false
  }
}
```

**Response:**
```json
{
  "success": true,
  "message": "Configuration updated successfully",
  "config": { ... }
}
```

## Development

### Prerequisites

- Python 3.8+
- Flask 3.0.0
- Flask-CORS 4.0.0

### Installation

```bash
cd app
pip3 install -r requirements.txt
```

### Running Locally

```bash
cd app
python3 server.py
```

Server will start on http://localhost:8080

### Environment Variables

- `PORT`: Server port (default: 8080)
- `OMNISIGHT_HTML_DIR`: Path to HTML directory (default: ../html)

## Deployment

This Flask application is packaged into the OMNISIGHT .eap file by the build script:

```bash
./scripts/build-eap.sh
```

On camera deployment:
1. Flask dependencies are installed on first run
2. Server launches on port 8080
3. Axis reverse proxy routes /api/* to Flask
4. HTML interface served at /

## Project Structure

```
app/
├── server.py              # Main Flask application
├── requirements.txt       # Python dependencies
├── api/
│   ├── __init__.py
│   ├── perception.py      # Perception endpoints
│   ├── timeline.py        # Timeline endpoints
│   ├── swarm.py           # Swarm endpoints
│   ├── config.py          # Configuration endpoints
│   └── system.py          # System endpoints
└── README.md              # This file
```

## Future Enhancements

Phase 3+ will add:
- IPC communication with C core
- Real-time data from perception engine
- WebSocket support for live updates
- Authentication and authorization
- Request rate limiting
- Advanced error handling
- Database integration

## License

Copyright (c) 2024-2025 OMNISIGHT Project
