/**
 * OMNISIGHT API Client
 * Handles communication with backend REST API
 */

export class OmnisightAPI {
  constructor(baseUrl = 'http://localhost:8080') {
    this.baseUrl = baseUrl;
  }

  async request(endpoint, options = {}) {
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
      console.error(`API request failed: ${endpoint}`, error);
      throw error;
    }
  }

  // GET /api/status
  async getStatus() {
    return this.request('/api/status');
  }

  // GET /api/statistics
  async getStatistics() {
    return this.request('/api/statistics');
  }

  // GET /api/tracks
  async getTracks() {
    return this.request('/api/tracks');
  }

  // GET /api/events
  async getEvents() {
    return this.request('/api/events');
  }

  // GET /api/interventions
  async getInterventions() {
    return this.request('/api/interventions');
  }

  // GET /api/cameras
  async getCameras() {
    return this.request('/api/cameras');
  }

  // GET /api/heatmap
  async getHeatmap() {
    return this.request('/api/heatmap');
  }

  // GET /api/config
  async getConfig() {
    return this.request('/api/config');
  }

  // POST /api/config
  async updateConfig(config) {
    return this.request('/api/config', {
      method: 'POST',
      body: JSON.stringify(config),
    });
  }

  // WebSocket connection for real-time updates
  connectWebSocket(url = 'ws://localhost:8081/ws/events', callbacks = {}) {
    const ws = new WebSocket(url);

    ws.onopen = () => {
      console.log('WebSocket connected');
      callbacks.onOpen?.();
    };

    ws.onmessage = (event) => {
      try {
        const data = JSON.parse(event.data);
        callbacks.onMessage?.(data);
      } catch (error) {
        console.error('Failed to parse WebSocket message', error);
      }
    };

    ws.onerror = (error) => {
      console.error('WebSocket error', error);
      callbacks.onError?.(error);
    };

    ws.onclose = () => {
      console.log('WebSocket disconnected');
      callbacks.onClose?.();
    };

    return ws;
  }
}
