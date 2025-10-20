/**
 * Main Dashboard Component
 * Shows real-time statistics and overview
 */

import React, { useState, useEffect } from 'react';
import { LineChart, Line, XAxis, YAxis, CartesianGrid, Tooltip, Legend, ResponsiveContainer } from 'recharts';
import './Dashboard.css';

function Dashboard({ api }) {
  const [statistics, setStatistics] = useState(null);
  const [tracks, setTracks] = useState([]);
  const [events, setEvents] = useState([]);
  const [fpsHistory, setFpsHistory] = useState([]);

  useEffect(() => {
    const fetchData = async () => {
      try {
        const [stats, tracksData, eventsData] = await Promise.all([
          api.getStatistics(),
          api.getTracks(),
          api.getEvents()
        ]);

        setStatistics(stats);
        setTracks(tracksData.tracks || []);
        setEvents(eventsData.events || []);

        // Add to FPS history
        if (stats.perception) {
          setFpsHistory(prev => {
            const newHistory = [...prev, {
              time: new Date().toLocaleTimeString(),
              fps: stats.perception.fps,
              latency: stats.perception.avg_latency_ms
            }];
            return newHistory.slice(-20); // Keep last 20 data points
          });
        }
      } catch (error) {
        console.error('Failed to fetch dashboard data:', error);
      }
    };

    fetchData();
    const interval = setInterval(fetchData, 1000); // Update every second

    return () => clearInterval(interval);
  }, [api]);

  if (!statistics) {
    return <div className="dashboard loading">Loading...</div>;
  }

  const { perception, timeline, swarm } = statistics;

  return (
    <div className="dashboard">
      <h2>System Dashboard</h2>

      {/* Key Metrics */}
      <div className="metrics-grid">
        <MetricCard
          title="Active Tracks"
          value={perception?.active_tracks || 0}
          icon="👥"
          trend={tracks.length > 10 ? 'up' : 'normal'}
        />
        <MetricCard
          title="Events Predicted"
          value={timeline?.events_predicted || 0}
          icon="⚠️"
          trend="normal"
        />
        <MetricCard
          title="Interventions"
          value={timeline?.interventions || 0}
          icon="🛡️"
          trend="normal"
        />
        <MetricCard
          title="Network Health"
          value={`${((swarm?.network_health || 0) * 100).toFixed(0)}%`}
          icon="🌐"
          trend={swarm?.network_health > 0.8 ? 'up' : 'down'}
        />
      </div>

      {/* Performance Chart */}
      <div className="chart-container">
        <h3>Performance Metrics</h3>
        <ResponsiveContainer width="100%" height={300}>
          <LineChart data={fpsHistory}>
            <CartesianGrid strokeDasharray="3 3" />
            <XAxis dataKey="time" />
            <YAxis yAxisId="left" />
            <YAxis yAxisId="right" orientation="right" />
            <Tooltip />
            <Legend />
            <Line yAxisId="left" type="monotone" dataKey="fps" stroke="#8884d8" name="FPS" />
            <Line yAxisId="right" type="monotone" dataKey="latency" stroke="#82ca9d" name="Latency (ms)" />
          </LineChart>
        </ResponsiveContainer>
      </div>

      {/* Active Tracks */}
      <div className="section">
        <h3>Active Tracks ({tracks.length})</h3>
        <div className="tracks-list">
          {tracks.slice(0, 10).map(track => (
            <div key={track.track_id} className={`track-card ${track.threat_score > 0.7 ? 'high-threat' : ''}`}>
              <div className="track-id">Track #{track.track_id}</div>
              <div className="track-details">
                <span>Position: ({track.x.toFixed(0)}, {track.y.toFixed(0)})</span>
                <span>Velocity: ({track.velocity_x.toFixed(1)}, {track.velocity_y.toFixed(1)})</span>
                <span>Confidence: {(track.confidence * 100).toFixed(0)}%</span>
              </div>
              <div className="track-threat">
                <ThreatMeter value={track.threat_score} />
                <span>Threat: {(track.threat_score * 100).toFixed(0)}%</span>
              </div>
              {track.behaviors && track.behaviors.length > 0 && (
                <div className="track-behaviors">
                  {track.behaviors.map(behavior => (
                    <span key={behavior} className="behavior-badge">{behavior}</span>
                  ))}
                </div>
              )}
            </div>
          ))}
        </div>
      </div>

      {/* Recent Events */}
      <div className="section">
        <h3>Recent Events ({events.length})</h3>
        <div className="events-list">
          {events.slice(0, 5).map((event, idx) => (
            <div key={idx} className={`event-card severity-${event.severity}`}>
              <div className="event-header">
                <span className="event-type">{event.type}</span>
                <span className="event-severity">{event.severity}</span>
              </div>
              <div className="event-details">
                <span>Probability: {(event.probability * 100).toFixed(0)}%</span>
                <span>Location: ({event.location.x.toFixed(0)}, {event.location.y.toFixed(0)})</span>
                <span>Time: {new Date(event.timestamp_ms).toLocaleTimeString()}</span>
              </div>
            </div>
          ))}
        </div>
      </div>

      {/* System Statistics */}
      <div className="stats-grid">
        <StatCard title="Perception" stats={perception} />
        <StatCard title="Timeline Threading™" stats={timeline} />
        <StatCard title="Swarm Intelligence" stats={swarm} />
      </div>
    </div>
  );
}

function MetricCard({ title, value, icon, trend }) {
  return (
    <div className={`metric-card trend-${trend}`}>
      <div className="metric-icon">{icon}</div>
      <div className="metric-content">
        <div className="metric-value">{value}</div>
        <div className="metric-title">{title}</div>
      </div>
    </div>
  );
}

function ThreatMeter({ value }) {
  const percentage = value * 100;
  let color = '#4caf50'; // Green
  if (percentage > 70) color = '#f44336'; // Red
  else if (percentage > 40) color = '#ff9800'; // Orange

  return (
    <div className="threat-meter">
      <div
        className="threat-meter-fill"
        style={{ width: `${percentage}%`, backgroundColor: color }}
      />
    </div>
  );
}

function StatCard({ title, stats }) {
  if (!stats) return null;

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
  return value;
}

export default Dashboard;
