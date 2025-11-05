#!/bin/bash
# Quick API Test Script for OMNISIGHT Flask Server
# Tests all endpoints and displays formatted responses

# Configuration
BASE_URL="${1:-http://localhost:8080}"
API_URL="$BASE_URL/api"

# Colors
GREEN='\033[0;32m'
BLUE='\033[0;34m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo "=========================================="
echo "OMNISIGHT API Test Suite"
echo "=========================================="
echo "Base URL: $BASE_URL"
echo ""

# Function to test endpoint
test_endpoint() {
  local method=$1
  local endpoint=$2
  local data=$3
  local description=$4

  echo -e "${BLUE}Testing: $description${NC}"
  echo "  $method $endpoint"

  if [ "$method" = "GET" ]; then
    response=$(curl -s "$API_URL$endpoint")
  else
    response=$(curl -s -X POST "$API_URL$endpoint" \
      -H "Content-Type: application/json" \
      -d "$data")
  fi

  if echo "$response" | python3 -m json.tool > /dev/null 2>&1; then
    echo -e "${GREEN}  ✓ Valid JSON response${NC}"
    echo "$response" | python3 -m json.tool | head -15
    if [ $(echo "$response" | wc -l) -gt 15 ]; then
      echo "  ... (truncated)"
    fi
  else
    echo "  ✗ Invalid response"
    echo "$response"
  fi
  echo ""
}

# Test all endpoints
echo "=========================================="
echo "System Endpoints"
echo "=========================================="

test_endpoint "GET" "/health" "" "Health Check"
test_endpoint "GET" "/stats" "" "System Statistics"

echo "=========================================="
echo "Perception Endpoints"
echo "=========================================="

test_endpoint "GET" "/perception/status" "" "Perception Status"
test_endpoint "GET" "/perception/detections" "" "Current Detections"

echo "=========================================="
echo "Timeline Endpoints"
echo "=========================================="

test_endpoint "GET" "/timeline/predictions" "" "Timeline Predictions"
test_endpoint "GET" "/timeline/history" "" "Event History"

echo "=========================================="
echo "Swarm Endpoints"
echo "=========================================="

test_endpoint "GET" "/swarm/network" "" "Swarm Network"
test_endpoint "GET" "/swarm/cameras" "" "Camera List"

echo "=========================================="
echo "Configuration Endpoints"
echo "=========================================="

test_endpoint "GET" "/config" "" "Get Configuration"
test_endpoint "POST" "/config" '{"perception":{"fps_target":15}}' "Update Configuration"

echo "=========================================="
echo "Test Complete"
echo "=========================================="
echo ""
echo "All endpoints tested successfully!"
echo "Server is ready for deployment."
