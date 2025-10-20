/**
 * Timeline Viewer Component
 * Visualizes multiple future timelines with predictions and interventions
 */

import React, { useState, useEffect } from 'react';
import './TimelineViewer.css';

function TimelineViewer({ api }) {
  const [timelines, setTimelines] = useState([]);
  const [selectedTimeline, setSelectedTimeline] = useState(0);
  const [timeHorizon, setTimeHorizon] = useState(60); // seconds

  useEffect(() => {
    const fetchTimelines = async () => {
      try {
        const data = await api.getTimelines();
        setTimelines(data.timelines || []);
      } catch (error) {
        console.error('Failed to fetch timelines:', error);
      }
    };

    fetchTimelines();
    const interval = setInterval(fetchTimelines, 2000); // Update every 2 seconds

    return () => clearInterval(interval);
  }, [api]);

  if (timelines.length === 0) {
    return (
      <div className="timeline-viewer loading">
        <h2>Timeline Threading™</h2>
        <p>Waiting for timeline predictions...</p>
      </div>
    );
  }

  const timeline = timelines[selectedTimeline] || timelines[0];

  return (
    <div className="timeline-viewer">
      <div className="timeline-header">
        <h2>Timeline Threading™ - Precognitive Prediction</h2>
        <div className="timeline-controls">
          <label>
            Time Horizon:
            <select value={timeHorizon} onChange={(e) => setTimeHorizon(parseInt(e.target.value))}>
              <option value={30}>30 seconds</option>
              <option value={60}>1 minute</option>
              <option value={120}>2 minutes</option>
              <option value={300}>5 minutes</option>
            </select>
          </label>
        </div>
      </div>

      {/* Timeline Selector */}
      <div className="timeline-selector">
        <h3>Probable Futures ({timelines.length})</h3>
        <div className="timeline-tabs">
          {timelines.map((t, idx) => (
            <button
              key={idx}
              className={`timeline-tab ${selectedTimeline === idx ? 'active' : ''} severity-${getSeverity(t)}`}
              onClick={() => setSelectedTimeline(idx)}
            >
              <div className="tab-header">
                <span className="tab-label">Timeline {idx + 1}</span>
                <span className="tab-probability">{(t.probability * 100).toFixed(0)}%</span>
              </div>
              <div className="tab-info">
                <span className="tab-events">{t.events?.length || 0} events</span>
                <span className={`tab-severity severity-${getSeverity(t)}`}>
                  {getSeverityLabel(getSeverity(t))}
                </span>
              </div>
            </button>
          ))}
        </div>
      </div>

      {/* Timeline Visualization */}
      <div className="timeline-content">
        <div className="timeline-details">
          <div className="timeline-info-card">
            <h3>Timeline {selectedTimeline + 1}</h3>
            <div className="timeline-stats">
              <div className="stat">
                <label>Probability</label>
                <div className="probability-bar">
                  <div
                    className="probability-fill"
                    style={{ width: `${timeline.probability * 100}%` }}
                  ></div>
                  <span className="probability-text">{(timeline.probability * 100).toFixed(1)}%</span>
                </div>
              </div>
              <div className="stat">
                <label>Events Predicted</label>
                <span className="stat-value">{timeline.events?.length || 0}</span>
              </div>
              <div className="stat">
                <label>Interventions Available</label>
                <span className="stat-value">{timeline.interventions?.length || 0}</span>
              </div>
              <div className="stat">
                <label>Overall Severity</label>
                <span className={`stat-value severity-${getSeverity(timeline)}`}>
                  {getSeverityLabel(getSeverity(timeline))}
                </span>
              </div>
            </div>
          </div>

          {/* Timeline Graph */}
          <div className="timeline-graph">
            <h3>Event Timeline</h3>
            <div className="timeline-axis">
              <div className="timeline-track">
                <div className="timeline-now">NOW</div>
                {timeline.events?.map((event, idx) => (
                  <TimelineEvent
                    key={idx}
                    event={event}
                    timeHorizon={timeHorizon}
                  />
                ))}
                {timeline.interventions?.map((intervention, idx) => (
                  <TimelineIntervention
                    key={idx}
                    intervention={intervention}
                    timeHorizon={timeHorizon}
                  />
                ))}
              </div>
              <div className="timeline-scale">
                {[...Array(5)].map((_, i) => (
                  <div key={i} className="time-marker">
                    {Math.round((timeHorizon * i) / 4)}s
                  </div>
                ))}
              </div>
            </div>
          </div>

          {/* Event Details */}
          <div className="timeline-events">
            <h3>Predicted Events</h3>
            {timeline.events && timeline.events.length > 0 ? (
              <div className="events-list">
                {timeline.events.map((event, idx) => (
                  <EventCard key={idx} event={event} />
                ))}
              </div>
            ) : (
              <p className="no-events">No events predicted in this timeline</p>
            )}
          </div>

          {/* Intervention Recommendations */}
          <div className="timeline-interventions">
            <h3>Intervention Recommendations</h3>
            {timeline.interventions && timeline.interventions.length > 0 ? (
              <div className="interventions-list">
                {timeline.interventions.map((intervention, idx) => (
                  <InterventionCard key={idx} intervention={intervention} />
                ))}
              </div>
            ) : (
              <p className="no-interventions">No interventions recommended</p>
            )}
          </div>
        </div>
      </div>
    </div>
  );
}

// Timeline Event Component
function TimelineEvent({ event, timeHorizon }) {
  const timeUntil = (event.timestamp_ms - Date.now()) / 1000;
  const position = Math.max(0, Math.min(100, (timeUntil / timeHorizon) * 100));

  return (
    <div
      className={`timeline-marker event-marker severity-${event.severity}`}
      style={{ left: `${position}%` }}
      title={`${event.description} (${timeUntil.toFixed(0)}s)`}
    >
      <div className="marker-icon">⚠️</div>
      <div className="marker-label">{getEventIcon(event.type)}</div>
    </div>
  );
}

// Timeline Intervention Component
function TimelineIntervention({ intervention, timeHorizon }) {
  const timeUntil = (intervention.timestamp_ms - Date.now()) / 1000;
  const position = Math.max(0, Math.min(100, (timeUntil / timeHorizon) * 100));

  return (
    <div
      className="timeline-marker intervention-marker"
      style={{ left: `${position}%` }}
      title={`${intervention.recommendation} (${timeUntil.toFixed(0)}s)`}
    >
      <div className="marker-icon">🛡️</div>
      <div className="marker-label">{getInterventionIcon(intervention.type)}</div>
    </div>
  );
}

// Event Card Component
function EventCard({ event }) {
  const timeUntil = (event.timestamp_ms - Date.now()) / 1000;

  return (
    <div className={`event-card severity-${event.severity}`}>
      <div className="event-header">
        <div className="event-icon">{getEventIcon(event.type)}</div>
        <div className="event-title">
          <h4>{event.type}</h4>
          <span className="event-time">
            {timeUntil > 0 ? `in ${timeUntil.toFixed(0)}s` : `${Math.abs(timeUntil).toFixed(0)}s ago`}
          </span>
        </div>
        <div className={`event-severity severity-${event.severity}`}>
          {getSeverityLabel(event.severity)}
        </div>
      </div>
      <div className="event-body">
        <p className="event-description">{event.description}</p>
        <div className="event-details">
          <div className="detail">
            <label>Probability</label>
            <span>{(event.probability * 100).toFixed(0)}%</span>
          </div>
          <div className="detail">
            <label>Location</label>
            <span>({event.location_x?.toFixed(2)}, {event.location_y?.toFixed(2)})</span>
          </div>
          {event.involved_tracks && event.num_involved > 0 && (
            <div className="detail">
              <label>Tracks Involved</label>
              <span>{event.involved_tracks.slice(0, event.num_involved).join(', ')}</span>
            </div>
          )}
        </div>
      </div>
    </div>
  );
}

// Intervention Card Component
function InterventionCard({ intervention }) {
  const timeUntil = (intervention.timestamp_ms - Date.now()) / 1000;

  return (
    <div className="intervention-card">
      <div className="intervention-header">
        <div className="intervention-icon">{getInterventionIcon(intervention.type)}</div>
        <div className="intervention-title">
          <h4>{intervention.type}</h4>
          <span className="intervention-time">
            {timeUntil > 0 ? `Act in ${timeUntil.toFixed(0)}s` : `Expired`}
          </span>
        </div>
        <div className="intervention-effectiveness">
          <span className="effectiveness-label">Effectiveness</span>
          <span className="effectiveness-value">{(intervention.effectiveness * 100).toFixed(0)}%</span>
        </div>
      </div>
      <div className="intervention-body">
        <p className="intervention-recommendation">{intervention.recommendation}</p>
        <div className="intervention-details">
          <div className="detail">
            <label>Prevents</label>
            <span className={`prevented-event severity-${intervention.prevented_event?.severity}`}>
              {intervention.prevented_event?.type || 'Unknown'}
            </span>
          </div>
          <div className="detail">
            <label>Cost/Effort</label>
            <div className="cost-bar">
              <div
                className="cost-fill"
                style={{ width: `${intervention.cost * 100}%` }}
              ></div>
            </div>
          </div>
        </div>
      </div>
    </div>
  );
}

// Helper Functions
function getSeverity(timeline) {
  if (!timeline.events || timeline.events.length === 0) return 'low';
  const maxSeverity = Math.max(...timeline.events.map(e => e.severity || 0));
  if (maxSeverity >= 4) return 'critical';
  if (maxSeverity >= 3) return 'high';
  if (maxSeverity >= 2) return 'medium';
  return 'low';
}

function getSeverityLabel(severity) {
  const labels = {
    'critical': 'Critical',
    'high': 'High',
    'medium': 'Medium',
    'low': 'Low',
    0: 'None',
    1: 'Low',
    2: 'Medium',
    3: 'High',
    4: 'Critical'
  };
  return labels[severity] || 'Unknown';
}

function getEventIcon(eventType) {
  const icons = {
    'EVENT_LOITERING': '🚶',
    'EVENT_THEFT': '🏃',
    'EVENT_ASSAULT': '⚔️',
    'EVENT_VANDALISM': '🔨',
    'EVENT_TRESPASSING': '🚫',
    'EVENT_SUSPICIOUS_BEHAVIOR': '👁️',
    'EVENT_COLLISION': '💥',
    'EVENT_FALL': '🤕',
    'EVENT_ABANDONED_OBJECT': '📦',
    'EVENT_CROWD_FORMATION': '👥'
  };
  return icons[eventType] || '⚠️';
}

function getInterventionIcon(interventionType) {
  const icons = {
    'INTERVENTION_ALERT_SECURITY': '🚨',
    'INTERVENTION_ACTIVATE_SPEAKER': '📢',
    'INTERVENTION_INCREASE_LIGHTING': '💡',
    'INTERVENTION_POSITION_GUARD': '💂',
    'INTERVENTION_LOCK_DOOR': '🔒',
    'INTERVENTION_NOTIFY_POLICE': '🚔',
    'INTERVENTION_DISPLAY_WARNING': '⚠️'
  };
  return icons[interventionType] || '🛡️';
}

export default TimelineViewer;
