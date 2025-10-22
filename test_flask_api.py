#!/usr/bin/env python3
"""
Test script for OMNISIGHT Flask API
Validates that all endpoints return expected data
"""

import sys
import os

# Add app directory to path
sys.path.insert(0, os.path.join(os.path.dirname(__file__), 'app'))

# Import Flask app
from server import app
import json

def test_endpoint(client, endpoint, method='GET', expected_keys=None):
    """Test a single endpoint and validate response"""
    print(f"\nTesting {method} {endpoint}...")

    if method == 'GET':
        response = client.get(endpoint)
    elif method == 'POST':
        response = client.post(endpoint,
                             data=json.dumps({"perception": {"enabled": False}}),
                             content_type='application/json')
    else:
        print(f"  SKIP: Unsupported method {method}")
        return False

    print(f"  Status: {response.status_code}")

    if response.status_code != 200:
        print(f"  FAIL: Expected 200, got {response.status_code}")
        return False

    try:
        data = json.loads(response.data)
        print(f"  Response: {json.dumps(data, indent=2)[:200]}...")

        if expected_keys:
            for key in expected_keys:
                if key not in data:
                    print(f"  FAIL: Missing expected key '{key}'")
                    return False

        print("  PASS")
        return True

    except json.JSONDecodeError:
        print("  FAIL: Invalid JSON response")
        return False


def main():
    """Run all endpoint tests"""
    print("=" * 60)
    print("OMNISIGHT Flask API Test Suite")
    print("=" * 60)

    # Create test client
    app.config['TESTING'] = True
    client = app.test_client()

    tests = [
        # System endpoints
        ('/api/health', 'GET', ['status', 'version']),
        ('/api/stats', 'GET', ['uptime_seconds', 'memory', 'cpu', 'modules']),

        # Perception endpoints
        ('/api/perception/status', 'GET', ['enabled', 'fps', 'resolution']),
        ('/api/perception/detections', 'GET', ['frame_id', 'timestamp', 'detections']),

        # Timeline endpoints
        ('/api/timeline/predictions', 'GET', ['active_timelines', 'predictions']),
        ('/api/timeline/history', 'GET', ['total_events', 'events']),

        # Swarm endpoints
        ('/api/swarm/network', 'GET', ['local_camera_id', 'neighbors', 'network_health']),
        ('/api/swarm/cameras', 'GET', ['cameras', 'total_count']),

        # Config endpoints
        ('/api/config', 'GET', ['perception', 'timeline', 'swarm']),
        ('/api/config', 'POST', ['success', 'config']),
    ]

    passed = 0
    failed = 0

    for endpoint, method, keys in tests:
        if test_endpoint(client, endpoint, method, keys):
            passed += 1
        else:
            failed += 1

    print("\n" + "=" * 60)
    print(f"Test Results: {passed} passed, {failed} failed")
    print("=" * 60)

    return 0 if failed == 0 else 1


if __name__ == '__main__':
    sys.exit(main())
