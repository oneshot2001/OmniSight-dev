# OMNISIGHT Dashboard - Testing Guide

## Quick Start

### 1. Install Dependencies

```bash
cd /Users/matthewvisher/Omnisight\ dev/OmniSight-dev/web/frontend
npm install
```

### 2. Start Development Server

```bash
npm run dev
```

Dashboard will be available at: `http://localhost:5173`

### 3. Mock API Server (Optional)

If the backend API is not running, you can create a simple mock server for testing:

**Create `mock-api.js` in the frontend directory:**

```javascript
// Simple mock API server for dashboard testing
import express from 'express';
import cors from 'cors';

const app = express();
app.use(cors());
app.use(express.json());

// Mock data generators
function randomBetween(min, max) {
  return Math.random() * (max - min) + min;
}

function generateDetections(count = 5) {
  return Array.from({ length: count }, (_, i) => ({
    id: i + 1,
    type: ['person', 'vehicle', 'object'][Math.floor(Math.random() * 3)],
    bbox: [
      randomBetween(100, 1000),
      randomBetween(100, 700),
      randomBetween(50, 200),
      randomBetween(100, 300)
    ],
    confidence: randomBetween(0.7, 0.99),
    track_id: `T${String(i + 1).padStart(3, '0')}`,
    threat_score: randomBetween(0, 0.9)
  }));
}

function generatePredictions(count = 3) {
  return Array.from({ length: count }, (_, i) => ({
    timeline_id: `TL${String(i + 1).padStart(3, '0')}`,
    probability: randomBetween(0.4, 0.95),
    horizon_seconds: [60, 120, 300][i % 3],
    events: [
      {
        type: ['TRESPASSING', 'LOITERING', 'THEFT'][Math.floor(Math.random() * 3)],
        severity: ['low', 'medium', 'high'][Math.floor(Math.random() * 3)],
        time_offset: randomBetween(10, 200),
        location: [randomBetween(100, 1000), randomBetween(100, 700)],
        confidence: randomBetween(0.6, 0.95)
      }
    ]
  }));
}

// Endpoints
app.get('/api/status', (req, res) => {
  res.json({
    status: 'running',
    uptime_seconds: Math.floor(Date.now() / 1000) % 10000,
    camera_id: 1,
    fps: randomBetween(28, 31),
    latency_ms: randomBetween(15, 25)
  });
});

app.get('/api/statistics', (req, res) => {
  res.json({
    perception: {
      frames_processed: Math.floor(randomBetween(10000, 50000)),
      active_tracks: Math.floor(randomBetween(3, 15)),
      fps: randomBetween(28, 31),
      avg_latency_ms: randomBetween(15, 25)
    },
    timeline: {
      num_updates: Math.floor(randomBetween(1000, 5000)),
      events_predicted: Math.floor(randomBetween(10, 100)),
      interventions: Math.floor(randomBetween(5, 20)),
      avg_confidence: randomBetween(0.7, 0.95)
    },
    swarm: {
      num_neighbors: Math.floor(randomBetween(2, 8)),
      tracks_shared: Math.floor(randomBetween(500, 2000)),
      tracks_received: Math.floor(randomBetween(400, 1800)),
      network_health: randomBetween(0.7, 0.99)
    }
  });
});

app.get('/api/perception/detections', (req, res) => {
  res.json({
    frame_id: Math.floor(randomBetween(10000, 99999)),
    timestamp: new Date().toISOString(),
    detections: generateDetections(Math.floor(randomBetween(2, 8)))
  });
});

app.get('/api/timeline/predictions', (req, res) => {
  res.json({
    active_timelines: 3,
    predictions: generatePredictions(3)
  });
});

app.get('/api/swarm/network', (req, res) => {
  const neighborCount = Math.floor(randomBetween(2, 6));
  res.json({
    local_camera_id: 'CAM001',
    neighbors: Array.from({ length: neighborCount }, (_, i) => ({
      id: `CAM${String(i + 2).padStart(3, '0')}`,
      distance: randomBetween(10, 100),
      signal_strength: -randomBetween(30, 80),
      shared_tracks: Math.floor(randomBetween(1, 10))
    })),
    network_health: randomBetween(0.7, 0.99)
  });
});

const PORT = 8080;
app.listen(PORT, () => {
  console.log(`Mock API server running on http://localhost:${PORT}`);
  console.log('Dashboard can now fetch data from this mock server');
});
```

**Run the mock server:**

```bash
# Install express if needed
npm install express cors

# Run mock server
node mock-api.js
```

## Testing Checklist

### Visual Tests

- [ ] **Header displays correctly**
  - Logo and title visible
  - Connection status indicator
  - FPS and latency metrics

- [ ] **Metric cards render**
  - All 4 cards visible
  - Icons display
  - Values update
  - Subtitles show

- [ ] **Performance chart works**
  - Chart renders
  - Data points appear
  - Dual Y-axis visible
  - Updates over time

- [ ] **Detection section**
  - Detection cards display
  - Bounding box visualization
  - Confidence bars work
  - Threat levels color-coded

- [ ] **Predictions section**
  - Prediction cards show
  - Probability displays
  - Event info visible
  - Severity indicators

- [ ] **Network section**
  - Local camera shown
  - Neighbor cameras listed
  - Health meter displays
  - Stats visible

- [ ] **Statistics cards**
  - All 3 cards render
  - Data populates
  - Formatting correct

### Functional Tests

- [ ] **Real-time updates**
  - Data refreshes every 1s
  - No visible lag
  - Smooth transitions

- [ ] **Error handling**
  - Shows loading state initially
  - Displays error if API down
  - Graceful degradation

- [ ] **Responsive design**
  - Works on desktop (1920x1080)
  - Works on tablet (768x1024)
  - Works on mobile (375x667)

- [ ] **Navigation**
  - Can switch between views
  - Active view highlighted
  - Dashboard loads by default

### Performance Tests

- [ ] **Load time**
  - Initial load < 2s
  - Data fetch < 500ms
  - Chart renders < 200ms

- [ ] **Memory usage**
  - No memory leaks
  - FPS history pruned correctly
  - Browser stays responsive

- [ ] **Network efficiency**
  - Parallel fetching works
  - Failed requests don't block
  - Retry logic functions

## Browser Testing

Test in the following browsers:

- [ ] Chrome (latest)
- [ ] Firefox (latest)
- [ ] Safari (latest)
- [ ] Edge (latest)
- [ ] Mobile Safari (iOS)
- [ ] Mobile Chrome (Android)

## Common Issues & Solutions

### Issue: "Unable to connect to API"

**Solution**: Ensure the backend API server is running on port 8080, or start the mock API server.

```bash
# Check if port 8080 is in use
lsof -i :8080

# Start mock API
node mock-api.js
```

### Issue: Chart not rendering

**Solution**: Check if recharts is installed:

```bash
npm install recharts
```

### Issue: WebSocket connection fails

**Solution**: WebSocket is optional for the dashboard (uses polling). If you need WebSocket, ensure it's running on port 8081.

### Issue: CORS errors

**Solution**: Backend must send CORS headers:

```javascript
// Express example
app.use((req, res, next) => {
  res.header('Access-Control-Allow-Origin', '*');
  res.header('Access-Control-Allow-Headers', 'Content-Type');
  next();
});
```

### Issue: Performance lag

**Solution**: Reduce update frequency in Dashboard.jsx:

```javascript
// Change from 1000ms to 2000ms
const interval = setInterval(fetchData, 2000);
```

## Expected Behavior

### On Load

1. Shows "Connecting to OMNISIGHT..." loading screen
2. Fetches data from all endpoints in parallel
3. Renders dashboard with initial data
4. Starts 1-second refresh interval

### During Operation

1. Metric cards update every second
2. Performance chart adds new data point every second
3. Detection cards refresh with new detections
4. Predictions update when timelines change
5. Network status shows live neighbor information

### On Error

1. If API is down: Shows "Unable to connect to API" message
2. If individual endpoint fails: Shows empty state for that section
3. Continues trying to fetch data every second
4. Recovers automatically when API comes back online

## Advanced Testing

### Load Testing

Simulate high detection count:

```javascript
// In mock-api.js, increase detection count
detections: generateDetections(50) // Instead of 8
```

### Stress Testing

Rapid updates:

```javascript
// In Dashboard.jsx, reduce interval
const interval = setInterval(fetchData, 100); // 10 updates/sec
```

### Network Simulation

Test with throttled network:

1. Open Chrome DevTools
2. Go to Network tab
3. Select "Slow 3G" throttling
4. Verify dashboard still functions

## Deployment Testing

### Build for Production

```bash
npm run build
```

Verify:
- [ ] Build completes without errors
- [ ] dist/ directory created
- [ ] Assets optimized (check file sizes)

### Preview Production Build

```bash
npm run preview
```

Visit `http://localhost:4173` and verify all features work.

## Accessibility Testing

- [ ] Keyboard navigation works
- [ ] Screen reader compatible
- [ ] Color contrast meets WCAG AA
- [ ] Alt text for visual elements
- [ ] Focus indicators visible

## Documentation

After testing, update:

- [ ] API endpoint documentation with actual response formats
- [ ] Component prop types
- [ ] Error handling procedures
- [ ] Deployment instructions

## Success Criteria

Dashboard is considered production-ready when:

1. All visual tests pass
2. All functional tests pass
3. Works on all major browsers
4. Responsive on all screen sizes
5. No console errors
6. Performance metrics met:
   - Initial load < 2s
   - Data fetch < 500ms
   - Smooth 60fps animations
7. Graceful error handling
8. Documentation complete

---

**Testing Status**: Ready for QA
**Last Updated**: 2025-10-22
**Tested By**: [Your Name]
