/**
 * Mock Data for Demo Mode
 * Used when API backend is unavailable
 */

export const mockSystemStatus = {
  status: "active",
  mode: "demo",
  version: "0.5.1",
  uptime_seconds: 3600,
  fps: 24.5,
  latency_ms: 12.3,
  camera_id: "P3285-LVE",
  modules: {
    perception: { status: "active", fps: 24.5 },
    timeline: { status: "active", predictions: 3 },
    swarm: { status: "active", cameras: 1 }
  }
};

export const mockDetections = {
  timestamp: new Date().toISOString(),
  frame_number: 1234,
  detections: [
    {
      id: 1,
      class: "person",
      confidence: 0.95,
      bbox: { x: 120, y: 80, width: 60, height: 140 },
      track_id: 42
    },
    {
      id: 2,
      class: "vehicle",
      confidence: 0.88,
      bbox: { x: 300, y: 200, width: 120, height: 80 },
      track_id: 17
    }
  ]
};

export const mockTimelinePredictions = {
  timestamp: new Date().toISOString(),
  current_state: {
    tracked_objects: 2,
    active_tracks: 2,
    scene_complexity: 0.4
  },
  predictions: [
    {
      future_id: "A",
      probability: 0.65,
      time_horizon_seconds: 300,
      predicted_events: [
        {
          type: "person_loitering",
          location: { x: 150, y: 120 },
          confidence: 0.72,
          time_offset_seconds: 120
        }
      ]
    },
    {
      future_id: "B",
      probability: 0.25,
      time_horizon_seconds: 300,
      predicted_events: [
        {
          type: "vehicle_parking",
          location: { x: 350, y: 220 },
          confidence: 0.68,
          time_offset_seconds: 180
        }
      ]
    },
    {
      future_id: "C",
      probability: 0.10,
      time_horizon_seconds: 300,
      predicted_events: [
        {
          type: "crowd_gathering",
          location: { x: 200, y: 150 },
          confidence: 0.55,
          time_offset_seconds: 240
        }
      ]
    }
  ]
};

export const mockEvents = [
  {
    id: 1,
    timestamp: new Date(Date.now() - 300000).toISOString(),
    type: "person_loitering",
    severity: "medium",
    confidence: 0.78,
    location: { x: 150, y: 120 },
    description: "Person detected in restricted area for extended period",
    track_id: 42
  },
  {
    id: 2,
    timestamp: new Date(Date.now() - 180000).toISOString(),
    type: "object_left_behind",
    severity: "high",
    confidence: 0.85,
    location: { x: 220, y: 180 },
    description: "Unattended object detected",
    track_id: null
  },
  {
    id: 3,
    timestamp: new Date(Date.now() - 60000).toISOString(),
    type: "perimeter_breach",
    severity: "high",
    confidence: 0.92,
    location: { x: 400, y: 250 },
    description: "Object crossed security perimeter",
    track_id: 17
  }
];

export const mockSwarmNetwork = {
  topology: "mesh",
  cameras: [
    {
      id: "P3285-LVE",
      status: "active",
      ip: "192.168.1.100",
      role: "coordinator",
      uptime_seconds: 3600,
      connections: 0
    }
  ],
  total_cameras: 1,
  active_connections: 0,
  health_score: 1.0
};

export const mockThreatHeatmap = {
  timestamp: new Date().toISOString(),
  resolution: { width: 1920, height: 1080 },
  cells: generateHeatmapCells()
};

function generateHeatmapCells() {
  const cells = [];
  const gridSize = 20;

  for (let y = 0; y < gridSize; y++) {
    for (let x = 0; x < gridSize; x++) {
      // Create some hotspots
      let threat_level = 0;

      // Hotspot 1: Bottom left
      if (x < 5 && y > 15) {
        threat_level = Math.random() * 0.8 + 0.2;
      }
      // Hotspot 2: Top right
      else if (x > 15 && y < 5) {
        threat_level = Math.random() * 0.6 + 0.1;
      }
      // Random low-level activity
      else {
        threat_level = Math.random() * 0.3;
      }

      cells.push({
        x,
        y,
        threat_level,
        event_count: Math.floor(threat_level * 10)
      });
    }
  }

  return cells;
}

export const mockConfig = {
  perception: {
    detection_threshold: 0.5,
    tracking_enabled: true,
    max_tracks: 50,
    fps_limit: 30
  },
  timeline: {
    prediction_horizon_seconds: 300,
    min_confidence: 0.6,
    max_futures: 3
  },
  swarm: {
    enabled: false,
    mqtt_broker: "localhost:1883",
    sync_interval_seconds: 5
  }
};
