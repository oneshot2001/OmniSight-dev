#!/bin/bash

# Build HTTP server integration test with full OMNISIGHT core
# Phase 4 - v0.7.0

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
BUILD_DIR="$PROJECT_ROOT/build-integration-test"

echo "========================================"
echo "OMNISIGHT HTTP Integration Test Build"
echo "========================================"

# Clean previous build
rm -rf "$BUILD_DIR"
mkdir -p "$BUILD_DIR"

echo "Building HTTP server integration test..."

# Compile with simplified http_server
gcc -Wall -Wextra -O2 \
    -I"$PROJECT_ROOT/src" \
    -I"$PROJECT_ROOT/src/perception" \
    -I"$PROJECT_ROOT/src/timeline" \
    -I"$PROJECT_ROOT/src/swarm" \
    -I"$PROJECT_ROOT/src/http" \
    -DMG_ENABLE_PACKED_FS=0 \
    -DOMNISIGHT_STUB_BUILD=1 \
    -DOMNISIGHT_VERSION=\"0.7.0\" \
    -D_GNU_SOURCE \
    -o "$BUILD_DIR/test_integration" \
    "$PROJECT_ROOT/src/http/test_integration.c" \
    "$PROJECT_ROOT/src/http/http_server_simple.c" \
    "$PROJECT_ROOT/src/http/mongoose.c" \
    "$PROJECT_ROOT/src/omnisight_core.c" \
    "$PROJECT_ROOT/src/perception/perception.c" \
    "$PROJECT_ROOT/src/perception/perception_stub.c" \
    "$PROJECT_ROOT/src/timeline/timeline.c" \
    "$PROJECT_ROOT/src/timeline/timeline_stub.c" \
    "$PROJECT_ROOT/src/swarm/swarm.c" \
    "$PROJECT_ROOT/src/swarm/swarm_stub.c" \
    -lpthread -lm

echo ""
echo "Build complete!"
echo ""
echo "Executable: $BUILD_DIR/test_integration"
echo ""
echo "Run with:"
echo "  $BUILD_DIR/test_integration [port]"
echo ""
echo "Default port: 8081"
echo ""
