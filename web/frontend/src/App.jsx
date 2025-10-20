/**
 * OMNISIGHT Web Interface
 * Main Application Component
 */

import React, { useState, useEffect } from 'react';
import './App.css';
import Dashboard from './components/Dashboard';
import CameraNetwork from './components/CameraNetwork';
import EventMonitor from './components/EventMonitor';
import ThreatHeatmap from './components/ThreatHeatmap';
import TimelineViewer from './components/TimelineViewer';
import ConfigPanel from './components/ConfigPanel';
import { OmnisightAPI } from './utils/api';

function App() {
  const [currentView, setCurrentView] = useState('dashboard');
  const [systemStatus, setSystemStatus] = useState(null);
  const [isConnected, setIsConnected] = useState(false);

  const api = new OmnisightAPI('http://localhost:8080');

  useEffect(() => {
    // Fetch system status on mount
    const fetchStatus = async () => {
      try {
        const status = await api.getStatus();
        setSystemStatus(status);
        setIsConnected(true);
      } catch (error) {
        console.error('Failed to fetch status:', error);
        setIsConnected(false);
      }
    };

    fetchStatus();
    const interval = setInterval(fetchStatus, 5000); // Update every 5 seconds

    return () => clearInterval(interval);
  }, []);

  const renderView = () => {
    switch (currentView) {
      case 'dashboard':
        return <Dashboard api={api} />;
      case 'network':
        return <CameraNetwork api={api} />;
      case 'events':
        return <EventMonitor api={api} />;
      case 'heatmap':
        return <ThreatHeatmap api={api} />;
      case 'timeline':
        return <TimelineViewer api={api} />;
      case 'config':
        return <ConfigPanel api={api} />;
      default:
        return <Dashboard api={api} />;
    }
  };

  return (
    <div className="app">
      {/* Header */}
      <header className="app-header">
        <div className="header-logo">
          <h1>🔮 OMNISIGHT</h1>
          <span className="tagline">Precognitive Security</span>
        </div>

        <div className="header-status">
          <span className={`status-indicator ${isConnected ? 'connected' : 'disconnected'}`}>
            {isConnected ? '● Connected' : '○ Disconnected'}
          </span>
          {systemStatus && (
            <>
              <span className="status-item">
                FPS: {systemStatus.fps?.toFixed(1)}
              </span>
              <span className="status-item">
                Latency: {systemStatus.latency_ms?.toFixed(1)}ms
              </span>
              <span className="status-item">
                Uptime: {formatUptime(systemStatus.uptime_seconds)}
              </span>
            </>
          )}
        </div>
      </header>

      {/* Navigation */}
      <nav className="app-nav">
        <button
          className={currentView === 'dashboard' ? 'active' : ''}
          onClick={() => setCurrentView('dashboard')}
        >
          📊 Dashboard
        </button>
        <button
          className={currentView === 'events' ? 'active' : ''}
          onClick={() => setCurrentView('events')}
        >
          ⚠️ Events
        </button>
        <button
          className={currentView === 'timeline' ? 'active' : ''}
          onClick={() => setCurrentView('timeline')}
        >
          🔮 Timeline Threading™
        </button>
        <button
          className={currentView === 'heatmap' ? 'active' : ''}
          onClick={() => setCurrentView('heatmap')}
        >
          🔥 Threat Heatmap
        </button>
        <button
          className={currentView === 'network' ? 'active' : ''}
          onClick={() => setCurrentView('network')}
        >
          🌐 Camera Network
        </button>
        <button
          className={currentView === 'config' ? 'active' : ''}
          onClick={() => setCurrentView('config')}
        >
          ⚙️ Configuration
        </button>
      </nav>

      {/* Main Content */}
      <main className="app-main">
        {renderView()}
      </main>

      {/* Footer */}
      <footer className="app-footer">
        <span>OMNISIGHT v1.0.0</span>
        <span>Timeline Threading™ • Swarm Intelligence • Privacy-Preserving</span>
        <span>Camera ID: {systemStatus?.camera_id || 'N/A'}</span>
      </footer>
    </div>
  );
}

function formatUptime(seconds) {
  if (!seconds) return '0s';

  const hours = Math.floor(seconds / 3600);
  const minutes = Math.floor((seconds % 3600) / 60);
  const secs = seconds % 60;

  if (hours > 0) {
    return `${hours}h ${minutes}m`;
  } else if (minutes > 0) {
    return `${minutes}m ${secs}s`;
  } else {
    return `${secs}s`;
  }
}

export default App;
