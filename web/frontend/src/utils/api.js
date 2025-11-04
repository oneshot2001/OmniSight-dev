/**
 * OMNISIGHT API Client
 * Handles communication with backend REST API (Phase 2)
 *
 * Enhanced for Flask/Python backend with Phase 2 endpoint structure
 * Includes demo mode fallback when API is unavailable
 */

import * as mockData from './mockData.js';

export class OmnisightAPI {
  constructor(baseUrl = 'http://localhost:8080') {
    this.baseUrl = baseUrl;
    this.wsUrl = 'ws://localhost:8081';
    this.reconnectAttempts = 0;
    this.maxReconnectAttempts = 5;
    this.demoMode = false;
    this.checkDemoMode();
  }

  /**
   * Check if API is available, enable demo mode if not
   */
  async checkDemoMode() {
    try {
      const controller = new AbortController();
      const timeoutId = setTimeout(() => controller.abort(), 2000);

      const response = await fetch(`${this.baseUrl}/api/health`, {
        signal: controller.signal
      });

      clearTimeout(timeoutId);
      this.demoMode = !response.ok;
    } catch (error) {
      console.log('API unavailable, enabling demo mode');
      this.demoMode = true;
    }
  }

  async request(endpoint, options = {}) {
    // If demo mode, return mock data
    if (this.demoMode) {
      return this.getMockData(endpoint);
    }

    const url = `${this.baseUrl}${endpoint}`;

    try {
      const response = await fetch(url, {
        ...options,
        headers: {
          'Content-Type': 'application/json',
          ...options.headers,
        },
      });

      if (!response.ok) {
        throw new Error(`HTTP error! status: ${response.status}`);
      }

      return await response.json();
    } catch (error) {
      console.error(`API request failed: ${endpoint}, switching to demo mode`, error);
      this.demoMode = true;
      return this.getMockData(endpoint);
    }
  }

  /**
   * Get mock data based on endpoint
   */
  getMockData(endpoint) {
    const mockResponses = {
      '/api/status': mockData.mockSystemStatus,
      '/api/health': { status: 'demo' },
      '/api/stats': mockData.mockSystemStatus,
      '/api/perception/status': mockData.mockSystemStatus.modules.perception,
      '/api/perception/detections': mockData.mockDetections,
      '/api/timeline/predictions': mockData.mockTimelinePredictions,
      '/api/timeline/history': { events: [] },
      '/api/swarm/network': mockData.mockSwarmNetwork,
      '/api/swarm/cameras': mockData.mockSwarmNetwork.cameras,
      '/api/events': mockData.mockEvents,
      '/api/config': mockData.mockConfig,
      '/api/heatmap': mockData.mockThreatHeatmap,
      '/api/tracks': mockData.mockDetections.detections,
      '/api/timelines': mockData.mockTimelinePredictions,
      '/api/interventions': [],
      '/api/cameras': mockData.mockSwarmNetwork.cameras,
      '/api/statistics': mockData.mockSystemStatus
    };

    // Check if endpoint starts with any of our mock endpoints
    for (const [key, value] of Object.entries(mockResponses)) {
      if (endpoint.startsWith(key)) {
        return Promise.resolve(value);
      }
    }

    // Default response
    return Promise.resolve({ status: 'demo', data: null });
  }

  isDemoMode() {
    return this.demoMode;
  }

  // ========================================================================
  // Perception Endpoints (Phase 2)
  // ========================================================================

  /**
   * GET /api/perception/status
   * Returns perception engine status and performance
   */
  async getPerceptionStatus() {
    return this.request('/api/perception/status');
  }

  /**
   * GET /api/perception/detections
   * Returns current frame detections with bounding boxes
   */
  async getDetections() {
    return this.request('/api/perception/detections');
  }

  // ========================================================================
  // Timeline Endpoints (Phase 2)
  // ========================================================================

  /**
   * GET /api/timeline/predictions
   * Returns active timeline predictions
   */
  async getPredictions() {
    return this.request('/api/timeline/predictions');
  }

  /**
   * GET /api/timeline/history
   * Returns past events and outcomes
   */
  async getTimelineHistory() {
    return this.request('/api/timeline/history');
  }

  // ========================================================================
  // Swarm Network Endpoints (Phase 2)
  // ========================================================================

  /**
   * GET /api/swarm/network
   * Returns swarm network topology and health
   */
  async getSwarmNetwork() {
    return this.request('/api/swarm/network');
  }

  /**
   * GET /api/swarm/cameras
   * Returns list of cameras in swarm
   */
  async getSwarmCameras() {
    return this.request('/api/swarm/cameras');
  }

  // ========================================================================
  // Configuration Endpoints (Phase 2)
  // ========================================================================

  /**
   * GET /api/config
   * Returns current configuration
   */
  async getConfig() {
    return this.request('/api/config');
  }

  /**
   * POST /api/config
   * Updates configuration
   */
  async updateConfig(config) {
    return this.request('/api/config', {
      method: 'POST',
      body: JSON.stringify(config),
    });
  }

  // ========================================================================
  // Statistics Endpoint (Phase 2)
  // ========================================================================

  /**
   * GET /api/stats
   * Returns comprehensive system statistics
   */
  async getStats() {
    return this.request('/api/stats');
  }

  // ========================================================================
  // Legacy Endpoints (maintain backward compatibility)
  // ========================================================================

  async getStatus() {
    return this.request('/api/status');
  }

  async getStatistics() {
    return this.request('/api/statistics');
  }

  async getTracks() {
    return this.request('/api/tracks');
  }

  async getEvents() {
    return this.request('/api/events');
  }

  async getInterventions() {
    return this.request('/api/interventions');
  }

  async getCameras() {
    return this.request('/api/cameras');
  }

  async getHeatmap(timeRange = 60) {
    return this.request(`/api/heatmap?timeRange=${timeRange}`);
  }

  async getTimelines() {
    return this.request('/api/timelines');
  }

  // ========================================================================
  // WebSocket Connection (Real-time updates)
  // ========================================================================

  /**
   * Connect to WebSocket for real-time updates
   *
   * @param {string} url WebSocket URL (default: ws://localhost:8081/ws/events)
   * @param {Object} callbacks Event callbacks
   * @param {Function} callbacks.onOpen Called when connection opens
   * @param {Function} callbacks.onMessage Called when message received
   * @param {Function} callbacks.onError Called on error
   * @param {Function} callbacks.onClose Called when connection closes
   * @returns {WebSocket} WebSocket instance
   */
  connectWebSocket(url = null, callbacks = {}) {
    const wsUrl = url || `${this.wsUrl}/ws/events`;
    const ws = new WebSocket(wsUrl);

    ws.onopen = () => {
      console.log('✅ WebSocket connected');
      this.reconnectAttempts = 0;
      callbacks.onOpen?.();
    };

    ws.onmessage = (event) => {
      try {
        const data = JSON.parse(event.data);
        callbacks.onMessage?.(data);
      } catch (error) {
        console.error('❌ Failed to parse WebSocket message', error);
      }
    };

    ws.onerror = (error) => {
      console.error('❌ WebSocket error', error);
      callbacks.onError?.(error);
    };

    ws.onclose = () => {
      console.log('🔌 WebSocket disconnected');
      callbacks.onClose?.();

      // Auto-reconnect logic
      if (this.reconnectAttempts < this.maxReconnectAttempts) {
        this.reconnectAttempts++;
        const delay = Math.min(1000 * Math.pow(2, this.reconnectAttempts), 30000);
        console.log(`🔄 Reconnecting in ${delay / 1000}s... (attempt ${this.reconnectAttempts}/${this.maxReconnectAttempts})`);

        setTimeout(() => {
          this.connectWebSocket(wsUrl, callbacks);
        }, delay);
      } else {
        console.error('❌ Max reconnect attempts reached');
      }
    };

    return ws;
  }

  /**
   * Create a polling fallback when WebSocket is unavailable
   *
   * @param {Function} fetchCallback Function to call on each poll
   * @param {number} interval Polling interval in ms (default: 1000)
   * @returns {Object} Polling controller with stop() method
   */
  createPollingFallback(fetchCallback, interval = 1000) {
    let active = true;
    let timeoutId = null;

    const poll = async () => {
      if (!active) return;

      try {
        await fetchCallback();
      } catch (error) {
        console.error('Polling error:', error);
      }

      if (active) {
        timeoutId = setTimeout(poll, interval);
      }
    };

    // Start polling
    poll();

    return {
      stop: () => {
        active = false;
        if (timeoutId) {
          clearTimeout(timeoutId);
        }
      }
    };
  }

  /**
   * Health check - test if API is reachable
   *
   * @returns {Promise<boolean>} True if API is healthy
   */
  async healthCheck() {
    try {
      const response = await fetch(`${this.baseUrl}/api/status`, {
        method: 'GET',
        timeout: 5000,
      });
      return response.ok;
    } catch (error) {
      return false;
    }
  }

  /**
   * Get API base URL
   *
   * @returns {string} Base URL
   */
  getBaseUrl() {
    return this.baseUrl;
  }

  /**
   * Set API base URL
   *
   * @param {string} url New base URL
   */
  setBaseUrl(url) {
    this.baseUrl = url;
  }
}

// Export default instance
export default new OmnisightAPI();
