# OMNISIGHT Web Interface

Real-time monitoring and control dashboard for OMNISIGHT precognitive security system.

## Features

### 📊 Dashboard
- **Real-time metrics**: FPS, latency, active tracks, network health
- **Performance charts**: Live FPS and latency graphs
- **Active tracks display**: Current tracked objects with threat scores
- **Recent events**: Latest predicted events with severity levels
- **System statistics**: Detailed stats for all components

### ⚠️ Event Monitor
- **Real-time event stream**: WebSocket-powered live updates
- **Event filtering**: Filter by severity (Critical, High, Medium, Low)
- **Detailed event cards**: Expandable cards with full event details
- **Visual indicators**: Color-coded severity levels and icons
- **Time tracking**: Shows time until predicted event or time since occurrence

### 🔮 Timeline Threading™ Viewer
- **Multiple future timelines**: Visualize 3-5 probable futures
- **Confidence scores**: See probability of each timeline
- **Event predictions**: View events predicted on each timeline
- **Intervention recommendations**: See suggested actions to prevent incidents

### 🔥 Threat Heatmap
- **Spatial threat visualization**: See high-threat areas on camera view
- **Real-time updates**: Heatmap updates as threats change
- **Historical data**: View threat patterns over time
- **Interactive**: Click on areas to see detailed threat information

### 🌐 Camera Network View
- **Network topology**: Visualize camera connections
- **FOV overlap**: See which cameras share field of view
- **Online status**: Monitor which cameras are active
- **Swarm health**: View network health metrics

### ⚙️ Configuration Panel
- **Live configuration**: Adjust parameters without restarting
- **Perception settings**: FPS, confidence threshold, max tracks
- **Timeline settings**: Prediction horizon, number of timelines
- **Swarm settings**: Privacy level, federated learning

## Architecture

```
┌─────────────────────────────────────────────────────────┐
│                    Frontend (React)                      │
│  ┌─────────────┐  ┌─────────────┐  ┌─────────────┐    │
│  │  Dashboard  │  │   Events    │  │  Timeline   │    │
│  └─────────────┘  └─────────────┘  └─────────────┘    │
│  ┌─────────────┐  ┌─────────────┐  ┌─────────────┐    │
│  │  Heatmap    │  │   Network   │  │   Config    │    │
│  └─────────────┘  └─────────────┘  └─────────────┘    │
└─────────────────────────────────────────────────────────┘
                          ↕
            HTTP REST API + WebSocket
                          ↕
┌─────────────────────────────────────────────────────────┐
│              API Server (C + libmicrohttpd)             │
│  ┌──────────────────────────────────────────────────┐  │
│  │  REST Endpoints  │  WebSocket  │  JSON Builder  │  │
│  └──────────────────────────────────────────────────┘  │
└─────────────────────────────────────────────────────────┘
                          ↕
┌─────────────────────────────────────────────────────────┐
│                  OMNISIGHT Components                    │
│  Perception Engine • Timeline Threading • Swarm         │
└─────────────────────────────────────────────────────────┘
```

## Installation

### Prerequisites

**Frontend:**
- Node.js 18+
- npm or yarn

**Backend:**
- libmicrohttpd (HTTP server)
- libwebsockets (WebSocket support)

### Frontend Setup

```bash
cd web/frontend

# Install dependencies
npm install

# Development mode (hot reload)
npm run dev

# Production build
npm run build

# Preview production build
npm run preview
```

The frontend will be available at `http://localhost:5173` (dev) or `http://localhost:4173` (preview).

### Backend Setup

```bash
cd web/api

# Install dependencies (Ubuntu/Debian)
sudo apt-get install libmicrohttpd-dev libwebsockets-dev

# Build API server
gcc -o omnisight_api \
    omnisight_api.c \
    ../../src/perception/*.c \
    ../../src/timeline/*.c \
    ../../src/swarm/*.c \
    -I../../src/perception \
    -I../../src/timeline \
    -I../../src/swarm \
    -lmicrohttpd \
    -lwebsockets \
    -lpthread \
    -lm

# Run API server
./omnisight_api
```

The API server will listen on:
- HTTP: `http://localhost:8080`
- WebSocket: `ws://localhost:8081`

## API Endpoints

### REST API

**GET /api/status**
```json
{
  "status": "running",
  "uptime_seconds": 12345,
  "camera_id": 1,
  "fps": 30.0,
  "latency_ms": 18.5
}
```

**GET /api/statistics**
```json
{
  "perception": {
    "frames_processed": 10000,
    "active_tracks": 15,
    "fps": 30.0,
    "avg_latency_ms": 18.5
  },
  "timeline": {
    "num_updates": 5000,
    "events_predicted": 50,
    "interventions": 10,
    "avg_confidence": 0.85
  },
  "swarm": {
    "num_neighbors": 5,
    "tracks_shared": 1000,
    "tracks_received": 800,
    "network_health": 0.95
  }
}
```

**GET /api/tracks**
```json
{
  "timestamp_ms": 1234567890,
  "tracks": [
    {
      "track_id": 1,
      "x": 500, "y": 300,
      "width": 50, "height": 100,
      "velocity_x": 5.0, "velocity_y": 2.0,
      "confidence": 0.95,
      "threat_score": 0.3,
      "behaviors": ["loitering"]
    }
  ]
}
```

**GET /api/events**
```json
{
  "events": [
    {
      "event_id": 1,
      "type": "trespassing",
      "severity": "high",
      "probability": 0.85,
      "timestamp_ms": 1234567890,
      "location": {"x": 500, "y": 300},
      "involved_tracks": [1, 2]
    }
  ]
}
```

### WebSocket

**Connect to**: `ws://localhost:8081/ws/events`

**Message types**:
```json
{
  "type": "event",
  "data": { /* PredictedEvent */ }
}

{
  "type": "track_update",
  "data": { /* TrackedObjects */ }
}

{
  "type": "intervention",
  "data": { /* InterventionPoint */ }
}
```

## Development

### Project Structure

```
web/
├── api/                    # Backend API server
│   ├── omnisight_api.h     # API header
│   └── omnisight_api.c     # API implementation
├── frontend/               # React frontend
│   ├── src/
│   │   ├── App.jsx         # Main app component
│   │   ├── App.css         # Main styles
│   │   ├── components/     # React components
│   │   │   ├── Dashboard.jsx
│   │   │   ├── EventMonitor.jsx
│   │   │   ├── TimelineViewer.jsx
│   │   │   ├── ThreatHeatmap.jsx
│   │   │   ├── CameraNetwork.jsx
│   │   │   └── ConfigPanel.jsx
│   │   └── utils/
│   │       └── api.js      # API client utility
│   ├── public/             # Static assets
│   └── package.json        # NPM dependencies
└── README.md               # This file
```

### Adding New Features

**New React Component:**
```jsx
// components/MyFeature.jsx
import React, { useState, useEffect } from 'react';
import './MyFeature.css';

function MyFeature({ api }) {
  const [data, setData] = useState(null);

  useEffect(() => {
    // Fetch data from API
    api.getMyData().then(setData);
  }, [api]);

  return (
    <div className="my-feature">
      {/* Your UI */}
    </div>
  );
}

export default MyFeature;
```

**New API Endpoint:**
```c
// In omnisight_api.c
static char* handle_my_endpoint(ApiServer* server) {
    char* json = malloc(1000);
    sprintf(json, "{\"data\":\"value\"}");
    return json;
}

// Add to request handler
if (strcmp(url, "/api/myendpoint") == 0) {
    response = handle_my_endpoint(server);
}
```

## Deployment

### Production Build

```bash
# Build frontend
cd web/frontend
npm run build

# This creates web/frontend/dist/ with optimized static files
```

### Serve with OMNISIGHT

The built frontend can be served directly by the API server:

```c
ApiServerConfig config = {
    .http_port = 8080,
    .websocket_port = 8081,
    .web_root = "/path/to/web/frontend/dist",
    .enable_cors = true
};
```

### Docker Deployment

```dockerfile
# Frontend build stage
FROM node:18 AS frontend-build
WORKDIR /app/web/frontend
COPY web/frontend/package*.json ./
RUN npm install
COPY web/frontend/ ./
RUN npm run build

# Backend build stage
FROM gcc:12 AS backend-build
WORKDIR /app
COPY . .
RUN apt-get update && apt-get install -y libmicrohttpd-dev libwebsockets-dev
RUN make build

# Production stage
FROM ubuntu:22.04
COPY --from=backend-build /app/build/omnisight /usr/local/bin/
COPY --from=frontend-build /app/web/frontend/dist /var/www/omnisight
EXPOSE 8080 8081
CMD ["omnisight"]
```

## Customization

### Themes

Edit `App.css` CSS variables:

```css
:root {
  --primary-color: #6366f1;    /* Change primary color */
  --secondary-color: #8b5cf6;  /* Change secondary color */
  --bg-dark: #0f172a;          /* Change background */
}
```

### Update Intervals

Adjust refresh rates in components:

```javascript
// In Dashboard.jsx
const interval = setInterval(fetchData, 1000); // 1 second

// In EventMonitor.jsx
const websocket = new WebSocket('ws://localhost:8081/ws/events');
```

## Troubleshooting

**Frontend won't connect to API:**
- Check API server is running: `curl http://localhost:8080/api/status`
- Check CORS is enabled in API config
- Verify WebSocket port is open: `telnet localhost 8081`

**WebSocket disconnecting:**
- Check firewall rules
- Increase timeout in libwebsockets config
- Monitor server logs for errors

**High latency:**
- Reduce update intervals
- Enable data compression
- Use production build (not dev mode)

**Memory leaks:**
- Check JSON strings are freed after sending
- Verify WebSocket clients are properly closed
- Monitor with valgrind: `valgrind --leak-check=full ./omnisight_api`

## License

Copyright © 2024 OMNISIGHT Project. All rights reserved.
