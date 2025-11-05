#!/bin/bash
# Test Flask API endpoints with IPC integration
# Requires: C stub running and Flask server running

set -e

echo "============================================"
echo "OMNISIGHT Flask API Integration Test"
echo "============================================"
echo ""

BASE_URL="http://localhost:8080"

# Colors
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Test function
test_endpoint() {
  local endpoint=$1
  local name=$2

  echo -n "Testing $name... "

  response=$(curl -s -w "\n%{http_code}" "$BASE_URL$endpoint")
  body=$(echo "$response" | head -n -1)
  code=$(echo "$response" | tail -n 1)

  if [ "$code" == "200" ]; then
    echo -e "${GREEN}✓${NC} (HTTP $code)"
    # Pretty print JSON if jq available
    if command -v jq &> /dev/null; then
      echo "$body" | jq '.' | head -20
    else
      echo "$body" | head -5
    fi
    echo ""
  elif [ "$code" == "503" ]; then
    echo -e "${YELLOW}⚠${NC} (HTTP $code - Service Unavailable)"
    echo "$body" | head -5
    echo ""
  else
    echo -e "${RED}✗${NC} (HTTP $code)"
    echo "$body"
    echo ""
  fi
}

echo "Testing System Endpoints"
echo "------------------------"
test_endpoint "/api/health" "Health Check"
test_endpoint "/api/status" "Core Status"
test_endpoint "/api/stats" "System Stats"
echo ""

echo "Testing Perception Endpoints"
echo "-----------------------------"
test_endpoint "/api/perception/status" "Perception Status"
test_endpoint "/api/perception/detections" "Detections"
test_endpoint "/api/perception/tracks" "Tracks"
echo ""

echo "Testing Timeline Endpoints"
echo "--------------------------"
test_endpoint "/api/timeline/status" "Timeline Status"
test_endpoint "/api/timeline/predictions" "Predictions"
echo ""

echo "Testing Swarm Endpoints"
echo "-----------------------"
test_endpoint "/api/swarm/status" "Swarm Status"
test_endpoint "/api/swarm/network" "Network Topology"
echo ""

echo "============================================"
echo "API Test Complete"
echo "============================================"
echo ""
echo "If you see HTTP 503 errors, make sure:"
echo "  1. C stub is running: ./build-stub/omnisight --demo"
echo "  2. JSON files exist: ls /tmp/omnisight_*.json"
echo "  3. Data is fresh: cat /tmp/omnisight_status.json"
echo ""
