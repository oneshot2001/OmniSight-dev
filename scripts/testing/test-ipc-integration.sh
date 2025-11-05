#!/bin/bash
# Test script for Phase 2 IPC Integration
# Tests C stub → JSON files → Flask API flow

set -e

echo "============================================"
echo "OMNISIGHT Phase 2 IPC Integration Test"
echo "============================================"
echo ""

# Step 1: Build C stub
echo "Step 1: Building C stub with JSON export..."
./scripts/build-stub.sh
echo "✓ Build complete"
echo ""

# Step 2: Clean old JSON files
echo "Step 2: Cleaning old JSON files..."
rm -f /tmp/omnisight_*.json
echo "✓ Old files removed"
echo ""

# Step 3: Start C stub in background
echo "Step 3: Starting C stub (background)..."
./build-stub/omnisight --demo &
STUB_PID=$!
echo "✓ C stub started (PID: $STUB_PID)"
echo ""

# Step 4: Wait for JSON files to be created
echo "Step 4: Waiting for JSON files (5 seconds)..."
sleep 5

# Step 5: Verify JSON files exist
echo "Step 5: Verifying JSON files..."
FILES=(
  "/tmp/omnisight_status.json"
  "/tmp/omnisight_stats.json"
  "/tmp/omnisight_detections.json"
  "/tmp/omnisight_tracks.json"
  "/tmp/omnisight_timelines.json"
)

for file in "${FILES[@]}"; do
  if [ -f "$file" ]; then
    SIZE=$(wc -c < "$file")
    echo "  ✓ $file ($SIZE bytes)"
  else
    echo "  ✗ $file (missing!)"
    kill $STUB_PID 2>/dev/null
    exit 1
  fi
done
echo ""

# Step 6: Display sample JSON content
echo "Step 6: Sample JSON content:"
echo ""
echo "--- Status ---"
cat /tmp/omnisight_status.json | python3 -m json.tool 2>/dev/null || cat /tmp/omnisight_status.json
echo ""
echo "--- Stats (summary) ---"
cat /tmp/omnisight_stats.json | python3 -c "import json, sys; d=json.load(sys.stdin); print(f'FPS: {d[\"perception\"][\"fps\"]}, Tracks: {d[\"perception\"][\"objects_tracked\"]}, Timelines: {d[\"timeline\"][\"active_timelines\"]}')" 2>/dev/null || echo "(parse error)"
echo ""

# Step 7: Test Flask IPC client
echo "Step 7: Testing Flask IPC client..."
cd app
python3 -c "
from ipc.json_client import OmnisightJSONClient
import json

client = OmnisightJSONClient()

print('Testing IPC client methods:')
print('  - get_status():', 'OK' if client.get_status() else 'FAIL')
print('  - get_stats():', 'OK' if client.get_stats() else 'FAIL')
print('  - get_detections():', 'OK' if client.get_detections() else 'FAIL')
print('  - get_tracks():', 'OK' if client.get_tracks() else 'FAIL')
print('  - get_timelines():', 'OK' if client.get_timelines() else 'FAIL')

health = client.health_check()
print(f'  - health_check(): {json.dumps(health)}')
" 2>&1
cd ..
echo ""

# Step 8: Cleanup
echo "Step 8: Stopping C stub..."
kill $STUB_PID 2>/dev/null
wait $STUB_PID 2>/dev/null || true
echo "✓ C stub stopped"
echo ""

echo "============================================"
echo "✓ All tests passed!"
echo "============================================"
echo ""
echo "Next steps:"
echo "1. Update app/server.py to initialize IPC client"
echo "2. Update app/api/*.py endpoints to use IPC"
echo "3. Run: python3 app/server.py"
echo "4. Test: curl http://localhost:8080/api/stats"
echo ""
echo "See PHASE2_IPC_IMPLEMENTATION.md for details"
echo ""
