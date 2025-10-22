# OMNISIGHT Development Container
# Based on ACAP Native SDK for Axis camera development

FROM axisecp/acap-native-sdk:1.15-aarch64-ubuntu22.04

# Set working directory
WORKDIR /opt/app

# Install additional development tools
RUN apt-get update && apt-get install -y \
    # Build essentials
    build-essential \
    cmake \
    ninja-build \
    pkg-config \
    # Version control
    git \
    git-lfs \
    # Python for ML model conversion
    python3 \
    python3-pip \
    python3-dev \
    # Network tools
    curl \
    wget \
    net-tools \
    iputils-ping \
    # Debugging tools
    gdb \
    gdbserver \
    valgrind \
    strace \
    # Additional libraries
    libssl-dev \
    libcurl4-openssl-dev \
    libmosquitto-dev \
    libjson-c-dev \
    # Cleanup
    && rm -rf /var/lib/apt/lists/*

# Install Python packages for ML workflow
RUN pip3 install --no-cache-dir \
    numpy \
    tensorflow==2.14.0 \
    onnx \
    onnx-tf \
    tensorflowjs \
    Pillow \
    opencv-python-headless \
    pyyaml \
    requests

# Install Rust toolchain for core components
RUN curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh -s -- -y \
    && . $HOME/.cargo/env \
    && rustup target add aarch64-unknown-linux-gnu

# Add Rust to PATH
ENV PATH="/root/.cargo/bin:${PATH}"

# Set up ACAP environment variables
ENV ACAP_SDK_VERSION=1.15
ENV CROSS_COMPILE=aarch64-linux-gnu-
ENV ARCH=aarch64
ENV SDKTARGETSYSROOT=/opt/axis/acapsdk/sysroots/aarch64

# Create project directories
RUN mkdir -p /opt/app/src \
    /opt/app/models \
    /opt/app/tests \
    /opt/app/build \
    /opt/app/package

# Set up build scripts (if they exist)
# COPY scripts/* /usr/local/bin/

# Expose ports for development
# 8080 - Application web interface
# 1883 - MQTT (Swarm Intelligence)
# 5000 - Debug/Development server
EXPOSE 8080 1883 5000

# Default command
CMD ["/bin/bash"]
