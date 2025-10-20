/**
 * Camera Network Component
 * Visualizes camera network topology and swarm intelligence status
 */

import React, { useState, useEffect, useRef } from 'react';
import './CameraNetwork.css';

function CameraNetwork({ api }) {
  const [cameras, setCameras] = useState([]);
  const [selectedCamera, setSelectedCamera] = useState(null);
  const [networkStats, setNetworkStats] = useState(null);
  const [viewMode, setViewMode] = useState('topology'); // 'topology' or 'list'
  const canvasRef = useRef(null);

  useEffect(() => {
    const fetchData = async () => {
      try {
        const data = await api.getCameras();
        setCameras(data.cameras || []);
        setNetworkStats(data.statistics || {});

        // Auto-select first camera if none selected
        if (!selectedCamera && data.cameras && data.cameras.length > 0) {
          setSelectedCamera(data.cameras[0].id);
        }
      } catch (error) {
        console.error('Failed to fetch camera network data:', error);
      }
    };

    fetchData();
    const interval = setInterval(fetchData, 3000);

    return () => clearInterval(interval);
  }, [api]);

  useEffect(() => {
    if (canvasRef.current && cameras.length > 0 && viewMode === 'topology') {
      drawNetworkTopology();
    }
  }, [cameras, selectedCamera, viewMode]);

  const drawNetworkTopology = () => {
    const canvas = canvasRef.current;
    if (!canvas) return;

    const ctx = canvas.getContext('2d');
    const width = canvas.width;
    const height = canvas.height;

    // Clear canvas
    ctx.fillStyle = '#0a0a1e';
    ctx.fillRect(0, 0, width, height);

    // Calculate positions for cameras (circular layout)
    const centerX = width / 2;
    const centerY = height / 2;
    const radius = Math.min(width, height) * 0.35;

    const positions = cameras.map((_, idx) => {
      const angle = (idx / cameras.length) * 2 * Math.PI - Math.PI / 2;
      return {
        x: centerX + radius * Math.cos(angle),
        y: centerY + radius * Math.sin(angle)
      };
    });

    // Draw connections (FOV overlap)
    cameras.forEach((camera, idx) => {
      if (camera.neighbors && camera.neighbors.length > 0) {
        camera.neighbors.forEach(neighborId => {
          const neighborIdx = cameras.findIndex(c => c.id === neighborId);
          if (neighborIdx !== -1) {
            drawConnection(
              ctx,
              positions[idx].x,
              positions[idx].y,
              positions[neighborIdx].x,
              positions[neighborIdx].y,
              camera.status === 'online' && cameras[neighborIdx].status === 'online'
            );
          }
        });
      }
    });

    // Draw cameras
    cameras.forEach((camera, idx) => {
      drawCamera(
        ctx,
        positions[idx].x,
        positions[idx].y,
        camera,
        camera.id === selectedCamera
      );
    });

    // Draw legend
    drawLegend(ctx, width, height);
  };

  const drawConnection = (ctx, x1, y1, x2, y2, active) => {
    ctx.strokeStyle = active ? 'rgba(16, 185, 129, 0.4)' : 'rgba(99, 102, 241, 0.2)';
    ctx.lineWidth = active ? 2 : 1;
    ctx.setLineDash(active ? [] : [5, 5]);
    ctx.beginPath();
    ctx.moveTo(x1, y1);
    ctx.lineTo(x2, y2);
    ctx.stroke();
    ctx.setLineDash([]);
  };

  const drawCamera = (ctx, x, y, camera, isSelected) => {
    const size = 40;

    // Draw FOV cone (if selected)
    if (isSelected && camera.fov_angle) {
      const fovLength = 80;
      const fovAngle = (camera.fov_angle * Math.PI) / 180;
      const cameraAngle = (camera.orientation * Math.PI) / 180;

      ctx.fillStyle = 'rgba(99, 102, 241, 0.2)';
      ctx.beginPath();
      ctx.moveTo(x, y);
      ctx.arc(
        x,
        y,
        fovLength,
        cameraAngle - fovAngle / 2,
        cameraAngle + fovAngle / 2
      );
      ctx.closePath();
      ctx.fill();
    }

    // Draw camera node
    const statusColor = getCameraStatusColor(camera.status);

    // Outer ring (selection indicator)
    if (isSelected) {
      ctx.strokeStyle = '#6366f1';
      ctx.lineWidth = 3;
      ctx.beginPath();
      ctx.arc(x, y, size / 2 + 5, 0, 2 * Math.PI);
      ctx.stroke();
    }

    // Camera circle
    const gradient = ctx.createRadialGradient(x, y, 0, x, y, size / 2);
    gradient.addColorStop(0, lightenColor(statusColor, 0.3));
    gradient.addColorStop(1, statusColor);
    ctx.fillStyle = gradient;
    ctx.beginPath();
    ctx.arc(x, y, size / 2, 0, 2 * Math.PI);
    ctx.fill();

    // Border
    ctx.strokeStyle = '#2a2a4e';
    ctx.lineWidth = 2;
    ctx.stroke();

    // Camera icon
    ctx.fillStyle = '#ffffff';
    ctx.font = '20px Arial';
    ctx.textAlign = 'center';
    ctx.textBaseline = 'middle';
    ctx.fillText('📹', x, y);

    // Camera name
    ctx.fillStyle = '#e0e0e0';
    ctx.font = 'bold 12px Arial';
    ctx.fillText(camera.name || camera.id, x, y + size / 2 + 15);

    // Status indicator
    ctx.fillStyle = statusColor;
    ctx.beginPath();
    ctx.arc(x + size / 3, y - size / 3, 6, 0, 2 * Math.PI);
    ctx.fill();
  };

  const drawLegend = (ctx, width, height) => {
    const legendX = 20;
    const legendY = height - 120;

    // Background
    ctx.fillStyle = 'rgba(26, 26, 62, 0.9)';
    ctx.fillRect(legendX, legendY, 180, 100);
    ctx.strokeStyle = '#3a3a5e';
    ctx.lineWidth = 2;
    ctx.strokeRect(legendX, legendY, 180, 100);

    // Title
    ctx.fillStyle = '#e0e0e0';
    ctx.font = 'bold 12px Arial';
    ctx.textAlign = 'left';
    ctx.fillText('Status Legend', legendX + 10, legendY + 20);

    // Status items
    const statuses = [
      { label: 'Online', color: '#10b981' },
      { label: 'Offline', color: '#6b7280' },
      { label: 'Warning', color: '#f59e0b' }
    ];

    statuses.forEach((status, idx) => {
      const y = legendY + 40 + idx * 20;

      ctx.fillStyle = status.color;
      ctx.beginPath();
      ctx.arc(legendX + 15, y, 5, 0, 2 * Math.PI);
      ctx.fill();

      ctx.fillStyle = '#e0e0e0';
      ctx.font = '11px Arial';
      ctx.fillText(status.label, legendX + 30, y + 4);
    });
  };

  const getCameraStatusColor = (status) => {
    switch (status) {
      case 'online':
        return '#10b981';
      case 'offline':
        return '#6b7280';
      case 'warning':
        return '#f59e0b';
      default:
        return '#6366f1';
    }
  };

  const lightenColor = (color, amount) => {
    // Simple color lightening (for gradient)
    return color;
  };

  const handleCanvasClick = (e) => {
    const canvas = canvasRef.current;
    const rect = canvas.getBoundingClientRect();
    const x = e.clientX - rect.left;
    const y = e.clientY - rect.top;

    // Check if click is on a camera
    const centerX = canvas.width / 2;
    const centerY = canvas.height / 2;
    const radius = Math.min(canvas.width, canvas.height) * 0.35;

    cameras.forEach((camera, idx) => {
      const angle = (idx / cameras.length) * 2 * Math.PI - Math.PI / 2;
      const cameraX = centerX + radius * Math.cos(angle);
      const cameraY = centerY + radius * Math.sin(angle);

      const distance = Math.sqrt((x - cameraX) ** 2 + (y - cameraY) ** 2);
      if (distance < 20) {
        setSelectedCamera(camera.id);
      }
    });
  };

  const selectedCameraData = cameras.find(c => c.id === selectedCamera);

  return (
    <div className="camera-network">
      <div className="network-header">
        <h2>Camera Network - Swarm Intelligence</h2>
        <div className="network-controls">
          <div className="view-mode-selector">
            <button
              className={viewMode === 'topology' ? 'active' : ''}
              onClick={() => setViewMode('topology')}
            >
              🕸️ Topology
            </button>
            <button
              className={viewMode === 'list' ? 'active' : ''}
              onClick={() => setViewMode('list')}
            >
              📋 List
            </button>
          </div>
        </div>
      </div>

      <div className="network-stats">
        <StatCard
          label="Total Cameras"
          value={cameras.length}
          icon="📹"
          color="#6366f1"
        />
        <StatCard
          label="Online"
          value={cameras.filter(c => c.status === 'online').length}
          icon="✅"
          color="#10b981"
        />
        <StatCard
          label="Network Health"
          value={`${((networkStats?.health || 0) * 100).toFixed(0)}%`}
          icon="🌐"
          color={networkStats?.health > 0.8 ? '#10b981' : '#f59e0b'}
        />
        <StatCard
          label="Messages/sec"
          value={networkStats?.messages_per_sec || 0}
          icon="📡"
          color="#6366f1"
        />
      </div>

      {viewMode === 'topology' ? (
        <div className="network-content">
          <div className="network-canvas-container">
            <canvas
              ref={canvasRef}
              width={700}
              height={600}
              className="network-canvas"
              onClick={handleCanvasClick}
            />
          </div>

          {selectedCameraData && (
            <div className="camera-details">
              <CameraDetailsPanel camera={selectedCameraData} />
            </div>
          )}
        </div>
      ) : (
        <div className="network-list">
          <CameraList
            cameras={cameras}
            selectedCamera={selectedCamera}
            onSelectCamera={setSelectedCamera}
          />
        </div>
      )}
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

function CameraDetailsPanel({ camera }) {
  return (
    <div className="camera-details-panel">
      <div className="camera-details-header">
        <h3>{camera.name || camera.id}</h3>
        <span className={`camera-status ${camera.status}`}>
          {camera.status}
        </span>
      </div>

      <div className="camera-info">
        <InfoRow label="Camera ID" value={camera.id} />
        <InfoRow label="Model" value={camera.model || 'Unknown'} />
        <InfoRow label="Location" value={camera.location || 'Not set'} />
        <InfoRow label="IP Address" value={camera.ip || 'N/A'} />
        <InfoRow label="FOV Angle" value={camera.fov_angle ? `${camera.fov_angle}°` : 'N/A'} />
        <InfoRow label="Orientation" value={camera.orientation ? `${camera.orientation}°` : 'N/A'} />
      </div>

      <div className="camera-metrics">
        <h4>Performance</h4>
        <MetricRow label="FPS" value={camera.fps || 0} />
        <MetricRow label="Latency" value={`${camera.latency_ms || 0}ms`} />
        <MetricRow label="Active Tracks" value={camera.active_tracks || 0} />
        <MetricRow label="Events Detected" value={camera.events_detected || 0} />
      </div>

      <div className="camera-neighbors">
        <h4>Connected Cameras ({camera.neighbors?.length || 0})</h4>
        {camera.neighbors && camera.neighbors.length > 0 ? (
          <div className="neighbors-list">
            {camera.neighbors.map(neighborId => (
              <div key={neighborId} className="neighbor-tag">
                📹 {neighborId}
              </div>
            ))}
          </div>
        ) : (
          <p className="no-neighbors">No FOV overlap with other cameras</p>
        )}
      </div>

      <div className="camera-swarm-stats">
        <h4>Swarm Statistics</h4>
        <MetricRow label="Tracks Shared" value={camera.tracks_shared || 0} />
        <MetricRow label="Tracks Received" value={camera.tracks_received || 0} />
        <MetricRow label="Handoffs" value={camera.handoffs || 0} />
        <MetricRow label="Consensus Events" value={camera.consensus_events || 0} />
      </div>
    </div>
  );
}

function InfoRow({ label, value }) {
  return (
    <div className="info-row">
      <span className="info-label">{label}</span>
      <span className="info-value">{value}</span>
    </div>
  );
}

function MetricRow({ label, value }) {
  return (
    <div className="metric-row">
      <span className="metric-label">{label}</span>
      <span className="metric-value">{value}</span>
    </div>
  );
}

function CameraList({ cameras, selectedCamera, onSelectCamera }) {
  return (
    <div className="camera-list-view">
      {cameras.map(camera => (
        <div
          key={camera.id}
          className={`camera-list-item ${camera.id === selectedCamera ? 'selected' : ''} ${camera.status}`}
          onClick={() => onSelectCamera(camera.id)}
        >
          <div className="camera-list-icon">📹</div>
          <div className="camera-list-info">
            <div className="camera-list-name">{camera.name || camera.id}</div>
            <div className="camera-list-meta">
              <span>{camera.location || 'Unknown location'}</span>
              <span>•</span>
              <span>{camera.ip || 'No IP'}</span>
            </div>
          </div>
          <div className="camera-list-stats">
            <div className="list-stat">
              <span className="list-stat-value">{camera.active_tracks || 0}</span>
              <span className="list-stat-label">Tracks</span>
            </div>
            <div className="list-stat">
              <span className="list-stat-value">{camera.neighbors?.length || 0}</span>
              <span className="list-stat-label">Neighbors</span>
            </div>
          </div>
          <div className={`camera-list-status ${camera.status}`}>
            {camera.status}
          </div>
        </div>
      ))}
    </div>
  );
}

export default CameraNetwork;
