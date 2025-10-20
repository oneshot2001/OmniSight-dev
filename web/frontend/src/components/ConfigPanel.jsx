/**
 * Configuration Panel Component
 * Live configuration interface for OMNISIGHT system parameters
 */

import React, { useState, useEffect } from 'react';
import './ConfigPanel.css';

function ConfigPanel({ api }) {
  const [config, setConfig] = useState(null);
  const [unsavedChanges, setUnsavedChanges] = useState(false);
  const [saving, setSaving] = useState(false);
  const [message, setMessage] = useState(null);

  useEffect(() => {
    fetchConfig();
  }, [api]);

  const fetchConfig = async () => {
    try {
      const data = await api.getConfig();
      setConfig(data);
      setUnsavedChanges(false);
    } catch (error) {
      console.error('Failed to fetch configuration:', error);
      showMessage('Failed to load configuration', 'error');
    }
  };

  const handleChange = (section, key, value) => {
    setConfig(prev => ({
      ...prev,
      [section]: {
        ...prev[section],
        [key]: value
      }
    }));
    setUnsavedChanges(true);
  };

  const handleSave = async () => {
    setSaving(true);
    try {
      await api.updateConfig(config);
      setUnsavedChanges(false);
      showMessage('Configuration saved successfully', 'success');
    } catch (error) {
      console.error('Failed to save configuration:', error);
      showMessage('Failed to save configuration', 'error');
    } finally {
      setSaving(false);
    }
  };

  const handleReset = () => {
    fetchConfig();
    showMessage('Configuration reset', 'info');
  };

  const showMessage = (text, type) => {
    setMessage({ text, type });
    setTimeout(() => setMessage(null), 3000);
  };

  if (!config) {
    return (
      <div className="config-panel loading">
        <h2>Configuration</h2>
        <p>Loading configuration...</p>
      </div>
    );
  }

  return (
    <div className="config-panel">
      <div className="config-header">
        <h2>System Configuration</h2>
        <div className="config-actions">
          {unsavedChanges && (
            <span className="unsaved-indicator">⚠️ Unsaved changes</span>
          )}
          <button
            className="btn btn-secondary"
            onClick={handleReset}
            disabled={!unsavedChanges || saving}
          >
            Reset
          </button>
          <button
            className="btn btn-primary"
            onClick={handleSave}
            disabled={!unsavedChanges || saving}
          >
            {saving ? 'Saving...' : 'Save Changes'}
          </button>
        </div>
      </div>

      {message && (
        <div className={`config-message ${message.type}`}>
          {message.text}
        </div>
      )}

      <div className="config-sections">
        {/* Perception Engine Configuration */}
        <ConfigSection title="Perception Engine" icon="👁️">
          <ConfigField
            label="Target FPS"
            type="number"
            value={config.perception?.fps || 30}
            onChange={(v) => handleChange('perception', 'fps', parseInt(v))}
            min={1}
            max={60}
            help="Target frames per second for video processing"
          />
          <ConfigField
            label="Confidence Threshold"
            type="range"
            value={config.perception?.confidence_threshold || 0.5}
            onChange={(v) => handleChange('perception', 'confidence_threshold', parseFloat(v))}
            min={0}
            max={1}
            step={0.05}
            help="Minimum confidence score for object detection (0.0-1.0)"
          />
          <ConfigField
            label="Max Tracks"
            type="number"
            value={config.perception?.max_tracks || 100}
            onChange={(v) => handleChange('perception', 'max_tracks', parseInt(v))}
            min={1}
            max={500}
            help="Maximum number of simultaneous tracks"
          />
          <ConfigField
            label="Enable Behavior Analysis"
            type="checkbox"
            checked={config.perception?.enable_behavior || true}
            onChange={(v) => handleChange('perception', 'enable_behavior', v)}
            help="Analyze behaviors like loitering, running, etc."
          />
        </ConfigSection>

        {/* Timeline Threading Configuration */}
        <ConfigSection title="Timeline Threading™" icon="🔮">
          <ConfigField
            label="Prediction Horizon (seconds)"
            type="number"
            value={config.timeline?.prediction_horizon || 60}
            onChange={(v) => handleChange('timeline', 'prediction_horizon', parseInt(v))}
            min={10}
            max={300}
            help="How far into the future to predict (10-300 seconds)"
          />
          <ConfigField
            label="Number of Timelines"
            type="number"
            value={config.timeline?.num_timelines || 3}
            onChange={(v) => handleChange('timeline', 'num_timelines', parseInt(v))}
            min={1}
            max={10}
            help="Number of probable futures to generate (1-10)"
          />
          <ConfigField
            label="Update Interval (ms)"
            type="number"
            value={config.timeline?.update_interval_ms || 1000}
            onChange={(v) => handleChange('timeline', 'update_interval_ms', parseInt(v))}
            min={100}
            max={5000}
            step={100}
            help="How often to update timeline predictions"
          />
          <ConfigField
            label="Minimum Event Probability"
            type="range"
            value={config.timeline?.min_event_probability || 0.3}
            onChange={(v) => handleChange('timeline', 'min_event_probability', parseFloat(v))}
            min={0}
            max={1}
            step={0.05}
            help="Minimum probability to report an event (0.0-1.0)"
          />
        </ConfigSection>

        {/* Swarm Intelligence Configuration */}
        <ConfigSection title="Swarm Intelligence" icon="🌐">
          <ConfigField
            label="Privacy Level"
            type="select"
            value={config.swarm?.privacy_level || 'metadata'}
            onChange={(v) => handleChange('swarm', 'privacy_level', v)}
            options={[
              { value: 'metadata', label: 'Metadata Only (Highest Privacy)' },
              { value: 'features', label: 'Features (Balanced)' },
              { value: 'full', label: 'Full Analytics (Lowest Privacy)' }
            ]}
            help="Level of data sharing between cameras"
          />
          <ConfigField
            label="Enable Federated Learning"
            type="checkbox"
            checked={config.swarm?.enable_federated_learning || false}
            onChange={(v) => handleChange('swarm', 'enable_federated_learning', v)}
            help="Collaboratively improve ML models without sharing raw data"
          />
          <ConfigField
            label="Consensus Threshold"
            type="range"
            value={config.swarm?.consensus_threshold || 0.6}
            onChange={(v) => handleChange('swarm', 'consensus_threshold', parseFloat(v))}
            min={0.5}
            max={1}
            step={0.05}
            help="Agreement required for multi-camera event validation (0.5-1.0)"
          />
          <ConfigField
            label="MQTT Broker"
            type="text"
            value={config.swarm?.mqtt_broker || 'localhost:1883'}
            onChange={(v) => handleChange('swarm', 'mqtt_broker', v)}
            help="MQTT broker address (host:port)"
          />
          <ConfigField
            label="Enable Track Handoff"
            type="checkbox"
            checked={config.swarm?.enable_track_handoff || true}
            onChange={(v) => handleChange('swarm', 'enable_track_handoff', v)}
            help="Share tracks when objects move between camera FOVs"
          />
        </ConfigSection>

        {/* Intervention Settings */}
        <ConfigSection title="Intervention Settings" icon="🛡️">
          <ConfigField
            label="Auto-Execute Interventions"
            type="checkbox"
            checked={config.intervention?.auto_execute || false}
            onChange={(v) => handleChange('intervention', 'auto_execute', v)}
            help="Automatically execute recommended interventions"
          />
          <ConfigField
            label="Minimum Severity for Auto-Execute"
            type="select"
            value={config.intervention?.min_severity || 'high'}
            onChange={(v) => handleChange('intervention', 'min_severity', v)}
            options={[
              { value: 'low', label: 'Low' },
              { value: 'medium', label: 'Medium' },
              { value: 'high', label: 'High' },
              { value: 'critical', label: 'Critical' }
            ]}
            help="Minimum event severity for automatic intervention"
          />
          <ConfigField
            label="Notification Email"
            type="email"
            value={config.intervention?.notification_email || ''}
            onChange={(v) => handleChange('intervention', 'notification_email', v)}
            help="Email address for critical alerts"
          />
        </ConfigSection>

        {/* Storage and Logging */}
        <ConfigSection title="Storage & Logging" icon="💾">
          <ConfigField
            label="Log Level"
            type="select"
            value={config.logging?.level || 'info'}
            onChange={(v) => handleChange('logging', 'level', v)}
            options={[
              { value: 'debug', label: 'Debug' },
              { value: 'info', label: 'Info' },
              { value: 'warning', label: 'Warning' },
              { value: 'error', label: 'Error' }
            ]}
            help="Logging verbosity level"
          />
          <ConfigField
            label="Event Retention (days)"
            type="number"
            value={config.storage?.event_retention_days || 30}
            onChange={(v) => handleChange('storage', 'event_retention_days', parseInt(v))}
            min={1}
            max={365}
            help="How long to keep event data"
          />
          <ConfigField
            label="Enable Video Recording"
            type="checkbox"
            checked={config.storage?.enable_video_recording || false}
            onChange={(v) => handleChange('storage', 'enable_video_recording', v)}
            help="Record video clips for significant events"
          />
          <ConfigField
            label="Max Storage (GB)"
            type="number"
            value={config.storage?.max_storage_gb || 100}
            onChange={(v) => handleChange('storage', 'max_storage_gb', parseInt(v))}
            min={1}
            max={1000}
            help="Maximum storage allocation for OMNISIGHT"
          />
        </ConfigSection>
      </div>
    </div>
  );
}

function ConfigSection({ title, icon, children }) {
  const [collapsed, setCollapsed] = useState(false);

  return (
    <div className="config-section">
      <div className="section-header" onClick={() => setCollapsed(!collapsed)}>
        <div className="section-title">
          <span className="section-icon">{icon}</span>
          <h3>{title}</h3>
        </div>
        <span className="collapse-icon">{collapsed ? '▶' : '▼'}</span>
      </div>
      {!collapsed && (
        <div className="section-content">
          {children}
        </div>
      )}
    </div>
  );
}

function ConfigField({ label, type, value, checked, onChange, options, help, ...props }) {
  const renderInput = () => {
    switch (type) {
      case 'checkbox':
        return (
          <input
            type="checkbox"
            checked={checked}
            onChange={(e) => onChange(e.target.checked)}
            className="config-checkbox"
          />
        );

      case 'select':
        return (
          <select
            value={value}
            onChange={(e) => onChange(e.target.value)}
            className="config-select"
          >
            {options.map(opt => (
              <option key={opt.value} value={opt.value}>
                {opt.label}
              </option>
            ))}
          </select>
        );

      case 'range':
        return (
          <div className="range-input">
            <input
              type="range"
              value={value}
              onChange={(e) => onChange(e.target.value)}
              className="config-range"
              {...props}
            />
            <span className="range-value">{value}</span>
          </div>
        );

      default:
        return (
          <input
            type={type}
            value={value}
            onChange={(e) => onChange(e.target.value)}
            className="config-input"
            {...props}
          />
        );
    }
  };

  return (
    <div className="config-field">
      <div className="field-header">
        <label className="field-label">{label}</label>
        {help && (
          <span className="field-help" title={help}>ℹ️</span>
        )}
      </div>
      {renderInput()}
      {help && type === 'range' && (
        <div className="field-description">{help}</div>
      )}
    </div>
  );
}

export default ConfigPanel;
