/**
 * Real-Time Event Monitor
 * Shows predicted events as they happen
 */

import React, { useState, useEffect, useRef } from 'react';
import './EventMonitor.css';

function EventMonitor({ api }) {
  const [events, setEvents] = useState([]);
  const [filter, setFilter] = useState('all');
  const [ws, setWs] = useState(null);
  const eventsEndRef = useRef(null);

  useEffect(() => {
    // Connect to WebSocket for real-time events
    const websocket = new WebSocket('ws://localhost:8081/ws/events');

    websocket.onopen = () => {
      console.log('WebSocket connected');
    };

    websocket.onmessage = (event) => {
      const data = JSON.parse(event.data);

      if (data.type === 'event') {
        setEvents(prev => [data.data, ...prev].slice(0, 100)); // Keep last 100 events
        playNotificationSound(data.data.severity);
      }
    };

    websocket.onerror = (error) => {
      console.error('WebSocket error:', error);
    };

    websocket.onclose = () => {
      console.log('WebSocket disconnected');
      // Reconnect after 5 seconds
      setTimeout(() => {
        window.location.reload();
      }, 5000);
    };

    setWs(websocket);

    // Also fetch initial events
    api.getEvents().then(data => {
      setEvents(data.events || []);
    });

    return () => {
      websocket.close();
    };
  }, [api]);

  useEffect(() => {
    // Scroll to latest event
    eventsEndRef.current?.scrollIntoView({ behavior: 'smooth' });
  }, [events]);

  const filteredEvents = events.filter(event => {
    if (filter === 'all') return true;
    if (filter === 'high') return event.severity === 'high' || event.severity === 'critical';
    return event.severity === filter;
  });

  return (
    <div className="event-monitor">
      <div className="monitor-header">
        <h2>📡 Real-Time Event Monitor</h2>

        <div className="event-filters">
          <button
            className={filter === 'all' ? 'active' : ''}
            onClick={() => setFilter('all')}
          >
            All ({events.length})
          </button>
          <button
            className={filter === 'critical' ? 'active critical' : 'critical'}
            onClick={() => setFilter('critical')}
          >
            Critical ({events.filter(e => e.severity === 'critical').length})
          </button>
          <button
            className={filter === 'high' ? 'active high' : 'high'}
            onClick={() => setFilter('high')}
          >
            High ({events.filter(e => e.severity === 'high').length})
          </button>
          <button
            className={filter === 'medium' ? 'active medium' : 'medium'}
            onClick={() => setFilter('medium')}
          >
            Medium ({events.filter(e => e.severity === 'medium').length})
          </button>
        </div>
      </div>

      <div className="events-timeline">
        {filteredEvents.length === 0 ? (
          <div className="no-events">
            <span className="icon">✓</span>
            <p>No events detected</p>
            <p className="subtitle">System is monitoring...</p>
          </div>
        ) : (
          filteredEvents.map((event, idx) => (
            <EventCard key={idx} event={event} />
          ))
        )}
        <div ref={eventsEndRef} />
      </div>
    </div>
  );
}

function EventCard({ event }) {
  const [expanded, setExpanded] = useState(false);

  const getSeverityIcon = (severity) => {
    switch (severity) {
      case 'critical': return '🚨';
      case 'high': return '⚠️';
      case 'medium': return '⚡';
      case 'low': return 'ℹ️';
      default: return '📌';
    }
  };

  const getEventTypeIcon = (type) => {
    switch (type) {
      case 'trespassing': return '🚫';
      case 'theft': return '💰';
      case 'assault': return '👊';
      case 'loitering': return '⏱️';
      case 'collision': return '💥';
      case 'suspicious_behavior': return '👀';
      default: return '❓';
    }
  };

  const timeAgo = (timestamp) => {
    const now = Date.now();
    const diff = timestamp - now;

    if (diff > 0) {
      // Future event
      const seconds = Math.floor(diff / 1000);
      if (seconds < 60) return `in ${seconds}s`;
      const minutes = Math.floor(seconds / 60);
      if (minutes < 60) return `in ${minutes}m`;
      return `in ${Math.floor(minutes / 60)}h`;
    } else {
      // Past event
      const seconds = Math.floor(-diff / 1000);
      if (seconds < 60) return `${seconds}s ago`;
      const minutes = Math.floor(seconds / 60);
      if (minutes < 60) return `${minutes}m ago`;
      return `${Math.floor(minutes / 60)}h ago`;
    }
  };

  return (
    <div
      className={`event-card event-${event.severity} ${expanded ? 'expanded' : ''}`}
      onClick={() => setExpanded(!expanded)}
    >
      <div className="event-card-header">
        <div className="event-icons">
          <span className="severity-icon">{getSeverityIcon(event.severity)}</span>
          <span className="type-icon">{getEventTypeIcon(event.type)}</span>
        </div>

        <div className="event-main">
          <div className="event-title">
            <span className="event-type-label">{formatEventType(event.type)}</span>
            <span className={`severity-badge severity-${event.severity}`}>
              {event.severity}
            </span>
          </div>
          <div className="event-meta">
            <span>{timeAgo(event.timestamp_ms)}</span>
            <span>•</span>
            <span>Probability: {(event.probability * 100).toFixed(0)}%</span>
            {event.location && (
              <>
                <span>•</span>
                <span>Location: ({event.location.x.toFixed(0)}, {event.location.y.toFixed(0)})</span>
              </>
            )}
          </div>
        </div>

        <div className="event-probability">
          <CircularProgress value={event.probability} />
        </div>
      </div>

      {expanded && (
        <div className="event-card-details">
          <div className="detail-row">
            <span className="detail-label">Event ID:</span>
            <span className="detail-value">{event.event_id}</span>
          </div>
          <div className="detail-row">
            <span className="detail-label">Timestamp:</span>
            <span className="detail-value">
              {new Date(event.timestamp_ms).toLocaleString()}
            </span>
          </div>
          {event.involved_tracks && event.involved_tracks.length > 0 && (
            <div className="detail-row">
              <span className="detail-label">Involved Tracks:</span>
              <span className="detail-value">
                {event.involved_tracks.map(id => `#${id}`).join(', ')}
              </span>
            </div>
          )}
          {event.intervention && (
            <div className="intervention-recommendation">
              <h4>🛡️ Recommended Intervention</h4>
              <p>{formatIntervention(event.intervention)}</p>
            </div>
          )}
        </div>
      )}
    </div>
  );
}

function CircularProgress({ value }) {
  const percentage = value * 100;
  const circumference = 2 * Math.PI * 18; // radius = 18
  const offset = circumference - (percentage / 100) * circumference;

  let color = '#4caf50'; // Green
  if (percentage > 70) color = '#f44336'; // Red
  else if (percentage > 40) color = '#ff9800'; // Orange

  return (
    <svg className="circular-progress" width="50" height="50">
      <circle
        cx="25"
        cy="25"
        r="18"
        stroke="#e0e0e0"
        strokeWidth="4"
        fill="none"
      />
      <circle
        cx="25"
        cy="25"
        r="18"
        stroke={color}
        strokeWidth="4"
        fill="none"
        strokeDasharray={circumference}
        strokeDashoffset={offset}
        transform="rotate(-90 25 25)"
      />
      <text
        x="25"
        y="25"
        textAnchor="middle"
        dy=".3em"
        fontSize="10"
        fill={color}
      >
        {percentage.toFixed(0)}%
      </text>
    </svg>
  );
}

function formatEventType(type) {
  return type.replace(/_/g, ' ').replace(/\b\w/g, l => l.toUpperCase());
}

function formatIntervention(intervention) {
  const interventions = {
    alert_security: 'Alert Security Personnel',
    activate_speaker: 'Activate Warning Speaker',
    dispatch_personnel: 'Dispatch Security to Location',
    lock_doors: 'Lock Affected Doors',
    call_emergency: 'Call Emergency Services',
    notify_manager: 'Notify Security Manager'
  };
  return interventions[intervention] || intervention;
}

function playNotificationSound(severity) {
  // Play different sounds based on severity
  if (severity === 'critical' || severity === 'high') {
    // In browser, you could play an audio file
    // For now, just log
    console.log(`🔔 ${severity.toUpperCase()} event notification`);
  }
}

export default EventMonitor;
