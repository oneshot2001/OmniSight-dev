#!/bin/bash

# Build HTTP server test directly with gcc (bypassing CMake issues)
# Phase 4 - v0.7.0 HTTP server validation

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
BUILD_DIR="$PROJECT_ROOT/build-http-test"

echo "========================================"
echo "OMNISIGHT HTTP Server Test Build"
echo "========================================"

# Clean previous build
rm -rf "$BUILD_DIR"
mkdir -p "$BUILD_DIR"

echo "Building HTTP server test..."

# Compile all source files
gcc -Wall -Wextra -O2 \
    -I"$PROJECT_ROOT/src" \
    -I"$PROJECT_ROOT/src/perception" \
    -I"$PROJECT_ROOT/src/timeline" \
    -I"$PROJECT_ROOT/src/swarm" \
    -I"$PROJECT_ROOT/src/http" \
    -DMG_ENABLE_PACKED_FS=0 \
    -DOMNISIGHT_STUB_BUILD=1 \
    -o "$BUILD_DIR/test_http_server" \
    "$PROJECT_ROOT/src/http/test_http_server.c" \
    "$PROJECT_ROOT/src/http/http_server.c" \
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
echo "Executable: $BUILD_DIR/test_http_server"
echo ""
echo "Run with:"
echo "  $BUILD_DIR/test_http_server [port]"
echo ""
echo "Default port: 8080"
echo ""
