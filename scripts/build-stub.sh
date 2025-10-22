#!/bin/bash
# OMNISIGHT Stub Build Script
# Compiles the application with stub implementations (no hardware dependencies)

set -e  # Exit on error

echo "================================================"
echo "OMNISIGHT Stub Build"
echo "================================================"
echo ""

# Build directories
BUILD_DIR="build-stub"
SRC_DIR="src"

# Compiler settings
CC="gcc"
CFLAGS="-Wall -Wextra -O2 -std=gnu11"
DEFINES="-D_GNU_SOURCE -DOMNISIGHT_VERSION=\\\"0.1.0\\\" -DOMNISIGHT_STUB_BUILD=1"
INCLUDES="-I./src -I./src/perception -I./src/timeline -I./src/swarm"
LIBS="-lm -lpthread"

# Create build directory
mkdir -p "$BUILD_DIR"

echo "Compiling modules..."

# Compile perception stub
echo "  - perception_stub.c"
$CC $CFLAGS $DEFINES $INCLUDES -c "$SRC_DIR/perception/perception_stub.c" -o "$BUILD_DIR/perception_stub.o"

# Compile timeline stub
echo "  - timeline_stub.c"
$CC $CFLAGS $DEFINES $INCLUDES -c "$SRC_DIR/timeline/timeline_stub.c" -o "$BUILD_DIR/timeline_stub.o"

# Compile swarm stub
echo "  - swarm_stub.c"
$CC $CFLAGS $DEFINES $INCLUDES -c "$SRC_DIR/swarm/swarm_stub.c" -o "$BUILD_DIR/swarm_stub.o"

# Compile core
echo "  - omnisight_core.c"
$CC $CFLAGS $DEFINES $INCLUDES -c "$SRC_DIR/omnisight_core.c" -o "$BUILD_DIR/omnisight_core.o"

# Compile main
echo "  - main.c"
$CC $CFLAGS $DEFINES $INCLUDES -c "$SRC_DIR/main.c" -o "$BUILD_DIR/main.o"

echo ""
echo "Linking..."

# Link all together
$CC -o "$BUILD_DIR/omnisight" \
    "$BUILD_DIR/main.o" \
    "$BUILD_DIR/omnisight_core.o" \
    "$BUILD_DIR/perception_stub.o" \
    "$BUILD_DIR/timeline_stub.o" \
    "$BUILD_DIR/swarm_stub.o" \
    $LIBS

echo ""
echo "================================================"
echo "✓ Build successful!"
echo "================================================"
echo ""
echo "Executable: $BUILD_DIR/omnisight"
echo "Size: $(du -h "$BUILD_DIR/omnisight" | cut -f1)"
echo ""
echo "Usage:"
echo "  ./$BUILD_DIR/omnisight          # Normal mode"
echo "  ./$BUILD_DIR/omnisight --demo   # Demo mode (no swarm)"
echo "  ./$BUILD_DIR/omnisight --help   # Show help"
echo ""
