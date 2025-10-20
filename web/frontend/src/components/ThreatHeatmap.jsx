/**
 * Threat Heatmap Component
 * Visualizes spatial threat distribution across camera field of view
 */

import React, { useState, useEffect, useRef } from 'react';
import './ThreatHeatmap.css';

function ThreatHeatmap({ api }) {
  const [heatmapData, setHeatmapData] = useState(null);
  const [tracks, setTracks] = useState([]);
  const [events, setEvents] = useState([]);
  const [timeRange, setTimeRange] = useState(60); // seconds
  const [showTracks, setShowTracks] = useState(true);
  const [showEvents, setShowEvents] = useState(true);
  const canvasRef = useRef(null);

  useEffect(() => {
    const fetchData = async () => {
      try {
        const [heatmap, tracksData, eventsData] = await Promise.all([
          api.getHeatmap(timeRange),
          api.getTracks(),
          api.getEvents()
        ]);

        setHeatmapData(heatmap);
        setTracks(tracksData.tracks || []);
        setEvents(eventsData.events || []);
      } catch (error) {
        console.error('Failed to fetch heatmap data:', error);
      }
    };

    fetchData();
    const interval = setInterval(fetchData, 2000);

    return () => clearInterval(interval);
  }, [api, timeRange]);

  useEffect(() => {
    if (canvasRef.current && heatmapData) {
      drawHeatmap();
    }
  }, [heatmapData, tracks, events, showTracks, showEvents]);

  const drawHeatmap = () => {
    const canvas = canvasRef.current;
    if (!canvas) return;

    const ctx = canvas.getContext('2d');
    const width = canvas.width;
    const height = canvas.height;

    // Clear canvas
    ctx.fillStyle = '#0a0a1e';
    ctx.fillRect(0, 0, width, height);

    // Draw grid
    drawGrid(ctx, width, height);

    // Draw heatmap
    if (heatmapData && heatmapData.grid) {
      drawHeatmapGrid(ctx, width, height, heatmapData.grid);
    }

    // Draw events
    if (showEvents && events.length > 0) {
      drawEventMarkers(ctx, width, height, events);
    }

    // Draw tracks
    if (showTracks && tracks.length > 0) {
      drawTrackMarkers(ctx, width, height, tracks);
    }

    // Draw legend
    drawLegend(ctx, width, height);
  };

  const drawGrid = (ctx, width, height) => {
    ctx.strokeStyle = '#2a2a4e';
    ctx.lineWidth = 1;

    // Vertical lines
    for (let x = 0; x <= width; x += width / 10) {
      ctx.beginPath();
      ctx.moveTo(x, 0);
      ctx.lineTo(x, height);
      ctx.stroke();
    }

    // Horizontal lines
    for (let y = 0; y <= height; y += height / 10) {
      ctx.beginPath();
      ctx.moveTo(0, y);
      ctx.lineTo(width, y);
      ctx.stroke();
    }
  };

  const drawHeatmapGrid = (ctx, width, height, grid) => {
    const cellWidth = width / grid.width;
    const cellHeight = height / grid.height;

    for (let y = 0; y < grid.height; y++) {
      for (let x = 0; x < grid.width; x++) {
        const idx = y * grid.width + x;
        const value = grid.values[idx] || 0;

        if (value > 0) {
          const color = getThreatColor(value);
          ctx.fillStyle = color;
          ctx.fillRect(x * cellWidth, y * cellHeight, cellWidth, cellHeight);
        }
      }
    }
  };

  const drawEventMarkers = (ctx, width, height, events) => {
    events.forEach(event => {
      const x = event.location_x * width;
      const y = event.location_y * height;
      const severity = event.severity || 1;

      // Draw outer glow
      const gradient = ctx.createRadialGradient(x, y, 0, x, y, 30);
      gradient.addColorStop(0, getSeverityColor(severity, 0.6));
      gradient.addColorStop(1, getSeverityColor(severity, 0));
      ctx.fillStyle = gradient;
      ctx.fillRect(x - 30, y - 30, 60, 60);

      // Draw marker
      ctx.fillStyle = getSeverityColor(severity, 1);
      ctx.beginPath();
      ctx.arc(x, y, 8, 0, 2 * Math.PI);
      ctx.fill();

      // Draw border
      ctx.strokeStyle = '#ffffff';
      ctx.lineWidth = 2;
      ctx.stroke();

      // Draw icon
      ctx.fillStyle = '#ffffff';
      ctx.font = '12px Arial';
      ctx.textAlign = 'center';
      ctx.textBaseline = 'middle';
      ctx.fillText('⚠️', x, y);
    });
  };

  const drawTrackMarkers = (ctx, width, height, tracks) => {
    tracks.forEach(track => {
      const x = track.bbox_x * width;
      const y = track.bbox_y * height;
      const threat = track.threat_score || 0;

      // Draw track trail if available
      if (track.history && track.history.length > 1) {
        ctx.strokeStyle = getThreatColor(threat / 100, 0.3);
        ctx.lineWidth = 2;
        ctx.beginPath();
        track.history.forEach((point, idx) => {
          const px = point.x * width;
          const py = point.y * height;
          if (idx === 0) {
            ctx.moveTo(px, py);
          } else {
            ctx.lineTo(px, py);
          }
        });
        ctx.stroke();
      }

      // Draw track marker
      const color = getThreatColor(threat / 100, 0.8);
      ctx.fillStyle = color;
      ctx.beginPath();
      ctx.arc(x, y, 6, 0, 2 * Math.PI);
      ctx.fill();

      // Draw track ID
      if (threat > 50) {
        ctx.fillStyle = '#ffffff';
        ctx.font = 'bold 10px Arial';
        ctx.textAlign = 'center';
        ctx.fillText(`#${track.track_id}`, x, y - 12);
      }
    });
  };

  const drawLegend = (ctx, width, height) => {
    const legendWidth = 200;
    const legendHeight = 100;
    const x = width - legendWidth - 20;
    const y = 20;

    // Background
    ctx.fillStyle = 'rgba(26, 26, 62, 0.9)';
    ctx.fillRect(x, y, legendWidth, legendHeight);
    ctx.strokeStyle = '#3a3a5e';
    ctx.lineWidth = 2;
    ctx.strokeRect(x, y, legendWidth, legendHeight);

    // Title
    ctx.fillStyle = '#e0e0e0';
    ctx.font = 'bold 12px Arial';
    ctx.textAlign = 'left';
    ctx.fillText('Threat Level', x + 10, y + 20);

    // Gradient bar
    const barWidth = 180;
    const barHeight = 20;
    const barX = x + 10;
    const barY = y + 30;

    const gradient = ctx.createLinearGradient(barX, 0, barX + barWidth, 0);
    gradient.addColorStop(0, '#10b981');
    gradient.addColorStop(0.33, '#f59e0b');
    gradient.addColorStop(0.66, '#ef4444');
    gradient.addColorStop(1, '#991b1b');

    ctx.fillStyle = gradient;
    ctx.fillRect(barX, barY, barWidth, barHeight);

    // Labels
    ctx.font = '10px Arial';
    ctx.fillStyle = '#a0a0a0';
    ctx.textAlign = 'left';
    ctx.fillText('Low', barX, barY + barHeight + 15);
    ctx.textAlign = 'center';
    ctx.fillText('Medium', barX + barWidth / 2, barY + barHeight + 15);
    ctx.textAlign = 'right';
    ctx.fillText('Critical', barX + barWidth, barY + barHeight + 15);
  };

  const getThreatColor = (value, alpha = 1) => {
    if (value < 0.33) {
      return `rgba(16, 185, 129, ${alpha})`; // Green
    } else if (value < 0.66) {
      return `rgba(245, 158, 11, ${alpha})`; // Orange
    } else {
      return `rgba(239, 68, 68, ${alpha})`; // Red
    }
  };

  const getSeverityColor = (severity, alpha = 1) => {
    const colors = {
      0: `rgba(99, 102, 241, ${alpha})`,
      1: `rgba(16, 185, 129, ${alpha})`,
      2: `rgba(245, 158, 11, ${alpha})`,
      3: `rgba(239, 68, 68, ${alpha})`,
      4: `rgba(153, 27, 27, ${alpha})`
    };
    return colors[severity] || colors[1];
  };

  if (!heatmapData) {
    return (
      <div className="threat-heatmap loading">
        <h2>Threat Heatmap</h2>
        <p>Loading heatmap data...</p>
      </div>
    );
  }

  const stats = heatmapData.statistics || {};

  return (
    <div className="threat-heatmap">
      <div className="heatmap-header">
        <h2>Threat Heatmap - Spatial Risk Analysis</h2>
        <div className="heatmap-controls">
          <label>
            Time Range:
            <select value={timeRange} onChange={(e) => setTimeRange(parseInt(e.target.value))}>
              <option value={30}>Last 30 seconds</option>
              <option value={60}>Last 1 minute</option>
              <option value={300}>Last 5 minutes</option>
              <option value={600}>Last 10 minutes</option>
              <option value={1800}>Last 30 minutes</option>
            </select>
          </label>
          <label>
            <input
              type="checkbox"
              checked={showTracks}
              onChange={(e) => setShowTracks(e.target.checked)}
            />
            Show Tracks
          </label>
          <label>
            <input
              type="checkbox"
              checked={showEvents}
              onChange={(e) => setShowEvents(e.target.checked)}
            />
            Show Events
          </label>
        </div>
      </div>

      <div className="heatmap-content">
        <div className="heatmap-canvas-container">
          <canvas
            ref={canvasRef}
            width={800}
            height={600}
            className="heatmap-canvas"
          />
        </div>

        <div className="heatmap-stats">
          <h3>Statistics</h3>
          <div className="stats-grid">
            <StatCard
              label="Hotspots"
              value={stats.hotspot_count || 0}
              icon="🔥"
              color="#ef4444"
            />
            <StatCard
              label="Avg Threat"
              value={((stats.average_threat || 0) * 100).toFixed(0) + '%'}
              icon="📊"
              color="#f59e0b"
            />
            <StatCard
              label="Max Threat"
              value={((stats.max_threat || 0) * 100).toFixed(0) + '%'}
              icon="⚠️"
              color="#ef4444"
            />
            <StatCard
              label="Safe Zones"
              value={stats.safe_zone_count || 0}
              icon="✅"
              color="#10b981"
            />
          </div>

          {stats.hotspots && stats.hotspots.length > 0 && (
            <div className="hotspots-list">
              <h4>Active Hotspots</h4>
              {stats.hotspots.map((hotspot, idx) => (
                <HotspotCard key={idx} hotspot={hotspot} />
              ))}
            </div>
          )}
        </div>
      </div>
    </div>
  );
}

function StatCard({ label, value, icon, color }) {
  return (
    <div className="stat-card">
      <div className="stat-icon" style={{ color }}>{icon}</div>
      <div className="stat-content">
        <div className="stat-label">{label}</div>
        <div className="stat-value" style={{ color }}>{value}</div>
      </div>
    </div>
  );
}

function HotspotCard({ hotspot }) {
  return (
    <div className="hotspot-card">
      <div className="hotspot-location">
        <span className="hotspot-coords">
          ({hotspot.x?.toFixed(2)}, {hotspot.y?.toFixed(2)})
        </span>
        <span className="hotspot-threat" style={{ color: getThreatColorForValue(hotspot.threat) }}>
          {(hotspot.threat * 100).toFixed(0)}% threat
        </span>
      </div>
      <div className="hotspot-info">
        <span>{hotspot.event_count || 0} events</span>
        <span>{hotspot.track_count || 0} tracks</span>
      </div>
    </div>
  );
}

function getThreatColorForValue(value) {
  if (value < 0.33) return '#10b981';
  if (value < 0.66) return '#f59e0b';
  return '#ef4444';
}

export default ThreatHeatmap;
