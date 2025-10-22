/**
 * Main Dashboard Component (Phase 2 Enhanced)
 * Shows real-time statistics and overview with Phase 2 API integration
 */

import React, { useState, useEffect } from 'react';
import { LineChart, Line, XAxis, YAxis, CartesianGrid, Tooltip, Legend, ResponsiveContainer } from 'recharts';
import './Dashboard.css';

function Dashboard({ api }) {
  const [statistics, setStatistics] = useState(null);
  const [detections, setDetections] = useState([]);
  const [predictions, setPredictions] = useState(null);
  const [networkStatus, setNetworkStatus] = useState(null);
  const [fpsHistory, setFpsHistory] = useState([]);
  const [isLoading, setIsLoading] = useState(true);
  const [lastUpdate, setLastUpdate] = useState(null);

  useEffect(() => {
    const fetchData = async () => {
      try {
        // Fetch all data in parallel using Phase 2 endpoints
        const [statsData, detectionsData, predictionsData, networkData] = await Promise.all([
          api.getStatistics().catch(() => null),
          api.getDetections().catch(() => ({ detections: [] })),
          api.getPredictions().catch(() => null),
          api.getSwarmNetwork().catch(() => null)
        ]);

        setStatistics(statsData);
        setDetections(detectionsData.detections || []);
        setPredictions(predictionsData);
        setNetworkStatus(networkData);
        setLastUpdate(new Date());
        setIsLoading(false);

        // Add to FPS history
        if (statsData?.perception) {
          setFpsHistory(prev => {
            const newHistory = [...prev, {
              time: new Date().toLocaleTimeString(),
              fps: statsData.perception.fps,
              latency: statsData.perception.avg_latency_ms
            }];
            return newHistory.slice(-30); // Keep last 30 data points
          });
        }
      } catch (error) {
        console.error('Failed to fetch dashboard data:', error);
        setIsLoading(false);
      }
    };

    fetchData();
    const interval = setInterval(fetchData, 1000); // Update every second

    return () => clearInterval(interval);
  }, [api]);

  if (isLoading) {
    return (
      <div className="dashboard loading">
        <div className="loading-spinner"></div>
        <p>Connecting to OMNISIGHT...</p>
      </div>
    );
  }

  if (!statistics) {
    return (
      <div className="dashboard error">
        <div className="error-icon">⚠️</div>
        <h3>Unable to connect to API</h3>
        <p>Please ensure the OMNISIGHT backend is running</p>
      </div>
    );
  }

  const { perception, timeline, swarm } = statistics;

  return (
    <div className="dashboard">
      <div className="dashboard-header">
        <h2>System Dashboard</h2>
        <div className="last-update">
          Last updated: {lastUpdate?.toLocaleTimeString()}
        </div>
      </div>

      {/* Key Metrics */}
      <div className="metrics-grid">
        <MetricCard
          title="Active Detections"
          value={detections.length}
          icon="👁️"
          subtitle={`${perception?.frames_processed || 0} frames processed`}
          trend={detections.length > 5 ? 'up' : 'normal'}
        />
        <MetricCard
          title="Timeline Predictions"
          value={predictions?.active_timelines || 0}
          icon="🔮"
          subtitle={`${timeline?.events_predicted || 0} events predicted`}
          trend="normal"
        />
        <MetricCard
          title="Swarm Cameras"
          value={networkStatus?.neighbors?.length || 0}
          icon="📷"
          subtitle={`Health: ${((networkStatus?.network_health || 0) * 100).toFixed(0)}%`}
          trend={networkStatus?.network_health > 0.8 ? 'up' : 'down'}
        />
        <MetricCard
          title="System FPS"
          value={perception?.fps?.toFixed(1) || '0.0'}
          icon="⚡"
          subtitle={`Latency: ${perception?.avg_latency_ms?.toFixed(1) || '0'}ms`}
          trend={perception?.fps > 25 ? 'up' : 'down'}
        />
      </div>

      {/* Performance Chart */}
      <div className="chart-container">
        <h3>Performance Metrics (Real-time)</h3>
        {fpsHistory.length > 0 ? (
          <ResponsiveContainer width="100%" height={300}>
            <LineChart data={fpsHistory}>
              <CartesianGrid strokeDasharray="3 3" stroke="#333" />
              <XAxis dataKey="time" stroke="#999" />
              <YAxis yAxisId="left" stroke="#8884d8" />
              <YAxis yAxisId="right" orientation="right" stroke="#82ca9d" />
              <Tooltip
                contentStyle={{
                  backgroundColor: '#1a1a1a',
                  border: '1px solid #333',
                  borderRadius: '4px'
                }}
              />
              <Legend />
              <Line
                yAxisId="left"
                type="monotone"
                dataKey="fps"
                stroke="#8884d8"
                name="FPS"
                strokeWidth={2}
                dot={false}
              />
              <Line
                yAxisId="right"
                type="monotone"
                dataKey="latency"
                stroke="#82ca9d"
                name="Latency (ms)"
                strokeWidth={2}
                dot={false}
              />
            </LineChart>
          </ResponsiveContainer>
        ) : (
          <div className="chart-loading">Collecting performance data...</div>
        )}
      </div>

      <div className="dashboard-content">
        {/* Current Detections */}
        <div className="section detections-section">
          <h3>
            Current Detections ({detections.length})
            <span className="section-subtitle">Live from perception engine</span>
          </h3>
          {detections.length > 0 ? (
            <div className="detections-grid">
              {detections.slice(0, 12).map((detection, idx) => (
                <DetectionCard key={idx} detection={detection} />
              ))}
            </div>
          ) : (
            <div className="empty-state">
              <span className="empty-icon">🔍</span>
              <p>No active detections</p>
            </div>
          )}
        </div>

        {/* Timeline Predictions Summary */}
        <div className="section predictions-section">
          <h3>
            Timeline Predictions
            <span className="section-subtitle">Active future timelines</span>
          </h3>
          {predictions?.predictions && predictions.predictions.length > 0 ? (
            <div className="predictions-list">
              {predictions.predictions.slice(0, 3).map((pred, idx) => (
                <PredictionCard key={idx} prediction={pred} />
              ))}
            </div>
          ) : (
            <div className="empty-state">
              <span className="empty-icon">✨</span>
              <p>No timeline predictions active</p>
            </div>
          )}
        </div>

        {/* Swarm Network Status */}
        <div className="section network-section">
          <h3>
            Swarm Network
            <span className="section-subtitle">Camera mesh topology</span>
          </h3>
          {networkStatus ? (
            <div className="network-info">
              <div className="network-local">
                <div className="camera-node local">
                  <div className="camera-icon">📷</div>
                  <div className="camera-label">
                    <strong>{networkStatus.local_camera_id || 'LOCAL'}</strong>
                    <span>This Camera</span>
                  </div>
                </div>
              </div>
              <div className="network-neighbors">
                {networkStatus.neighbors?.map((neighbor, idx) => (
                  <div key={idx} className="camera-node neighbor">
                    <div className="camera-icon">📷</div>
                    <div className="camera-details">
                      <strong>{neighbor.id}</strong>
                      <div className="neighbor-stats">
                        <span>Distance: {neighbor.distance?.toFixed(1)}m</span>
                        <span>Signal: {neighbor.signal_strength}dBm</span>
                        <span>Shared: {neighbor.shared_tracks} tracks</span>
                      </div>
                    </div>
                  </div>
                ))}
              </div>
              <div className="network-health">
                <label>Network Health</label>
                <div className="health-bar">
                  <div
                    className="health-fill"
                    style={{
                      width: `${(networkStatus.network_health || 0) * 100}%`,
                      backgroundColor: getHealthColor(networkStatus.network_health)
                    }}
                  ></div>
                </div>
                <span>{networkStatus.network_health || 'Unknown'}</span>
              </div>
            </div>
          ) : (
            <div className="empty-state">
              <span className="empty-icon">🌐</span>
              <p>Swarm network not initialized</p>
            </div>
          )}
        </div>
      </div>

      {/* System Statistics */}
      <div className="stats-grid">
        <StatCard title="Perception Engine" stats={perception} />
        <StatCard title="Timeline Threading™" stats={timeline} />
        <StatCard title="Swarm Intelligence" stats={swarm} />
      </div>
    </div>
  );
}

// ============================================================================
// Sub-components
// ============================================================================

function MetricCard({ title, value, icon, subtitle, trend }) {
  return (
    <div className={`metric-card trend-${trend}`}>
      <div className="metric-icon">{icon}</div>
      <div className="metric-content">
        <div className="metric-value">{value}</div>
        <div className="metric-title">{title}</div>
        {subtitle && <div className="metric-subtitle">{subtitle}</div>}
      </div>
    </div>
  );
}

function DetectionCard({ detection }) {
  const confidencePercent = (detection.confidence * 100).toFixed(0);
  const threatLevel = detection.threat_score
    ? detection.threat_score > 0.7 ? 'high' : detection.threat_score > 0.4 ? 'medium' : 'low'
    : 'low';

  return (
    <div className={`detection-card threat-${threatLevel}`}>
      <div className="detection-header">
        <span className="detection-type">{detection.type || 'Unknown'}</span>
        <span className="detection-id">#{detection.id}</span>
      </div>
      <div className="detection-bbox">
        <svg viewBox="0 0 100 100" className="bbox-visual">
          <rect
            x={detection.bbox?.[0] / 19.2 || 10}
            y={detection.bbox?.[1] / 10.8 || 10}
            width={detection.bbox?.[2] / 19.2 || 20}
            height={detection.bbox?.[3] / 10.8 || 20}
            fill="none"
            stroke="currentColor"
            strokeWidth="2"
          />
        </svg>
      </div>
      <div className="detection-details">
        <div className="detail-row">
          <label>Confidence</label>
          <div className="confidence-bar">
            <div
              className="confidence-fill"
              style={{ width: `${confidencePercent}%` }}
            ></div>
            <span>{confidencePercent}%</span>
          </div>
        </div>
        {detection.track_id && (
          <div className="detail-row">
            <label>Track ID</label>
            <span className="track-id">{detection.track_id}</span>
          </div>
        )}
      </div>
    </div>
  );
}

function PredictionCard({ prediction }) {
  const probability = (prediction.probability * 100).toFixed(0);
  const eventCount = prediction.events?.length || 0;
  const severity = getSeverityFromEvents(prediction.events);

  return (
    <div className={`prediction-card severity-${severity}`}>
      <div className="prediction-header">
        <span className="prediction-id">{prediction.timeline_id}</span>
        <span className={`prediction-probability prob-${getProbClass(prediction.probability)}`}>
          {probability}%
        </span>
      </div>
      <div className="prediction-info">
        <div className="info-item">
          <label>Events</label>
          <span>{eventCount}</span>
        </div>
        <div className="info-item">
          <label>Horizon</label>
          <span>{prediction.horizon_seconds}s</span>
        </div>
        <div className="info-item">
          <label>Severity</label>
          <span className={`severity-badge severity-${severity}`}>
            {severity}
          </span>
        </div>
      </div>
      {prediction.events && prediction.events.length > 0 && (
        <div className="prediction-events">
          <h5>Key Event:</h5>
          <div className="event-preview">
            <span className="event-type">{prediction.events[0].type}</span>
            <span className="event-time">in {prediction.events[0].time_offset}s</span>
          </div>
        </div>
      )}
    </div>
  );
}

function StatCard({ title, stats }) {
  if (!stats) {
    return (
      <div className="stat-card empty">
        <h4>{title}</h4>
        <p className="stat-empty">No data available</p>
      </div>
    );
  }

  return (
    <div className="stat-card">
      <h4>{title}</h4>
      <ul>
        {Object.entries(stats).map(([key, value]) => (
          <li key={key}>
            <span className="stat-key">{formatKey(key)}:</span>
            <span className="stat-value">{formatValue(value)}</span>
          </li>
        ))}
      </ul>
    </div>
  );
}

// ============================================================================
// Helper Functions
// ============================================================================

function formatKey(key) {
  return key.replace(/_/g, ' ').replace(/\b\w/g, l => l.toUpperCase());
}

function formatValue(value) {
  if (typeof value === 'number') {
    if (Number.isInteger(value)) {
      return value.toLocaleString();
    } else {
      return value.toFixed(2);
    }
  }
  return value?.toString() || 'N/A';
}

function getHealthColor(health) {
  if (health > 0.8) return '#4caf50';
  if (health > 0.5) return '#ff9800';
  return '#f44336';
}

function getSeverityFromEvents(events) {
  if (!events || events.length === 0) return 'low';
  const severities = events.map(e => e.severity || 'low');
  if (severities.includes('CRITICAL') || severities.includes('critical')) return 'critical';
  if (severities.includes('HIGH') || severities.includes('high')) return 'high';
  if (severities.includes('MEDIUM') || severities.includes('medium')) return 'medium';
  return 'low';
}

function getProbClass(probability) {
  if (probability > 0.7) return 'high';
  if (probability > 0.4) return 'medium';
  return 'low';
}

export default Dashboard;
