# OMNISIGHT Deployment Guide

## Prerequisites

### Hardware Requirements

**Minimum:**
- Axis camera with ARTPEC-8 SoC
- 512MB RAM
- 100MB storage
- Network connectivity (for Swarm Intelligence)

**Recommended:**
- Axis ARTPEC-8 camera with DLPU
- 1GB RAM
- 200MB storage
- Gigabit Ethernet
- PoE+ (Power over Ethernet Plus)

**Compatible Cameras:**
- AXIS Q1656-LE (ARTPEC-8, 4K)
- AXIS M3068-P (ARTPEC-8, Panoramic)
- AXIS P3265-LVE (ARTPEC-8, Dome)
- AXIS P3267-LVE (ARTPEC-8, Dome, IR)
- Any AXIS OS 12.0+ camera with ARTPEC-8

### Software Requirements

**Development:**
- Docker 20.10+
- ACAP Native SDK 1.15+
- GCC 11+ (aarch64 cross-compiler)
- CMake 3.16+

**Runtime (on camera):**
- AXIS OS 12.0+
- ACAP Runtime 4.0+
- VDO API 3.0+
- Larod API 3.0+

## Build Process

### Step 1: Set Up Development Environment

```bash
# Clone repository
git clone https://github.com/oneshot2001/OmniSight-dev.git
cd OmniSight-dev

# Pull ACAP SDK Docker image
docker pull axisecp/acap-native-sdk:1.15-aarch64-ubuntu22.04

# Create build container
docker run --rm -it \
    -v $(pwd):/opt/app \
    -w /opt/app \
    axisecp/acap-native-sdk:1.15-aarch64-ubuntu22.04 \
    /bin/bash
```

### Step 2: Build OMNISIGHT

```bash
# Inside Docker container
mkdir -p build && cd build

# Configure with CMake
cmake .. \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_TOOLCHAIN_FILE=/opt/axis/acapsdk/sysroots/x86_64-pokysdk-linux/usr/share/cmake/axis-toolchain.cmake \
    -DCMAKE_INSTALL_PREFIX=/usr/local

# Build
make -j$(nproc)

# Run tests
make test

# Install to staging directory
make install DESTDIR=staging
```

### Step 3: Prepare ML Models

```bash
# Train models (or use pre-trained)
cd models/training/scripts

# Install Python dependencies
pip install -r requirements.txt

# Train detection model
python train_detection.py \
    --config ../configs/efficientnet_b4_detection.yaml \
    --dataset /path/to/dataset \
    --output ../checkpoints/detection_model

# Convert to TFLite
python convert_to_tflite.py \
    --model ../checkpoints/detection_model \
    --output ../../detection/efficientnet_b4_detection.tflite \
    --input-size 416 \
    --quantize INT8

# Validate model
python validate_model.py \
    --model ../../detection/efficientnet_b4_detection.tflite \
    --test-data /path/to/test_data
```

### Step 4: Create ACAP Package

```bash
# Create manifest.json
cat > manifest.json <<EOF
{
  "schemaVersion": "1.6.0",
  "acapPackageConf": {
    "setup": {
      "appName": "omnisight",
      "vendor": "OMNISIGHT",
      "version": "1.0.0",
      "appType": "application",
      "embeddedSdkVersion": "3.0",
      "architecture": "aarch64",
      "appId": "omnisight"
    },
    "configuration": {
      "httpConfig": [
        {
          "type": "transferConfig",
          "access": "admin",
          "name": "OMNISIGHT API"
        }
      ],
      "settingPage": "html/index.html",
      "httpContent": [
        {
          "access": "admin",
          "name": "OMNISIGHT Web Interface"
        }
      ]
    },
    "resources": {
      "memory": {
        "ram": 512
      },
      "storage": {
        "size": 100
      },
      "cpu": {
        "usage": 30
      }
    }
  }
}
EOF

# Package ACAP
eap-install.sh create \
    -m manifest.json \
    -s staging \
    -o omnisight_1_0_0_aarch64.eap
```

## Deployment

### Method 1: Web Interface

1. Open camera web interface: `http://camera-ip`
2. Login with admin credentials
3. Navigate to **Settings** → **Apps**
4. Click **Add app** or drag-and-drop `omnisight_1_0_0_aarch64.eap`
5. Wait for upload to complete
6. Click **Start** to launch OMNISIGHT

### Method 2: Command Line

```bash
# Upload ACAP package
curl -u root:password \
    -F "package=@omnisight_1_0_0_aarch64.eap" \
    http://camera-ip/axis-cgi/applications/upload.cgi

# Start application
curl -u root:password \
    "http://camera-ip/axis-cgi/applications/control.cgi?action=start&package=omnisight"

# Check status
curl -u root:password \
    "http://camera-ip/axis-cgi/applications/list.cgi"
```

### Method 3: SSH Deployment

```bash
# SSH into camera
ssh root@camera-ip

# Upload package (from local machine)
scp omnisight_1_0_0_aarch64.eap root@camera-ip:/tmp/

# On camera: Install
cd /tmp
systemd-run --unit=omnisight /usr/local/packages/omnisight/omnisight

# Check logs
journalctl -u omnisight -f
```

## Configuration

### Camera Configuration

Create `/etc/omnisight/config.json` on camera:

```json
{
  "camera": {
    "id": 1,
    "role": "internal",
    "position": {
      "x": 10.0,
      "y": 5.0,
      "z": 3.0
    },
    "fov": {
      "horizontal": 90.0,
      "vertical": 60.0
    }
  },
  "perception": {
    "fps": 30,
    "resolution": {
      "width": 1920,
      "height": 1080
    },
    "confidence_threshold": 0.5,
    "max_tracks": 100
  },
  "timeline": {
    "prediction_horizon_s": 60.0,
    "num_timelines": 3,
    "branching_enabled": true
  },
  "swarm": {
    "enabled": true,
    "mqtt_broker": "192.168.1.100",
    "mqtt_port": 1883,
    "privacy_level": "features",
    "federated_learning": true
  },
  "zones": [
    {
      "name": "Entrance",
      "x": 500,
      "y": 300,
      "radius": 100,
      "protected_event": "trespassing",
      "sensitivity": 0.9
    }
  ]
}
```

### MQTT Broker Setup

**Option 1: Mosquitto (Lightweight)**

```bash
# Install
sudo apt-get install mosquitto mosquitto-clients

# Configure /etc/mosquitto/mosquitto.conf
listener 1883 0.0.0.0
allow_anonymous false
password_file /etc/mosquitto/passwd

# Create user
sudo mosquitto_passwd -c /etc/mosquitto/passwd omnisight

# Start broker
sudo systemctl start mosquitto
sudo systemctl enable mosquitto

# Test
mosquitto_sub -h localhost -t "omnisight/#" -u omnisight -P password -v
```

**Option 2: HiveMQ (Enterprise)**

```bash
# Docker deployment
docker run -d \
    --name hivemq \
    -p 1883:1883 \
    -p 8080:8080 \
    -v $(pwd)/hivemq-config:/opt/hivemq/conf \
    hivemq/hivemq4

# Configure extensions for authentication, monitoring, etc.
```

### Network Topology Configuration

**Small Site (1-10 cameras):**
```json
{
  "topology": "star",
  "broker": "192.168.1.100:1883",
  "heartbeat_interval_ms": 5000,
  "track_share_interval_ms": 1000
}
```

**Medium Site (10-50 cameras):**
```json
{
  "topology": "star",
  "broker": "mqtt.site.local:1883",
  "heartbeat_interval_ms": 10000,
  "track_share_interval_ms": 2000,
  "enable_load_balancing": true
}
```

**Large Site (50+ cameras):**
```json
{
  "topology": "mesh",
  "brokers": [
    "mqtt1.site.local:1883",
    "mqtt2.site.local:1883",
    "mqtt3.site.local:1883"
  ],
  "heartbeat_interval_ms": 15000,
  "track_share_interval_ms": 3000,
  "enable_load_balancing": true,
  "enable_clustering": true
}
```

## Multi-Camera Deployment

### Step 1: Deploy Coordinator

```bash
# Deploy one camera as coordinator
./deploy.sh --camera 192.168.1.10 --role coordinator

# Coordinator runs federated learning aggregation
```

### Step 2: Deploy Edge Cameras

```bash
# Deploy edge cameras (perimeter)
for ip in 192.168.1.{11..14}; do
    ./deploy.sh --camera $ip --role edge --coordinator 192.168.1.10
done
```

### Step 3: Deploy Internal Cameras

```bash
# Deploy internal cameras
for ip in 192.168.1.{15..24}; do
    ./deploy.sh --camera $ip --role internal --coordinator 192.168.1.10
done
```

### Step 4: Configure Neighbors

```bash
# Auto-discover neighbors based on FOV overlap
./scripts/configure_neighbors.py \
    --cameras cameras.yaml \
    --min-overlap 0.2

# cameras.yaml contains camera positions and FOV
```

## Monitoring

### Health Checks

```bash
# Check application status
curl -u root:password \
    "http://camera-ip/axis-cgi/applications/list.cgi" | \
    jq '.data.list[] | select(.Name=="omnisight")'

# Check logs
ssh root@camera-ip "journalctl -u omnisight -n 100"

# Check resource usage
ssh root@camera-ip "top -b -n 1 | grep omnisight"
```

### Metrics Collection

```bash
# Prometheus exporter (optional)
# OMNISIGHT exposes metrics at /metrics

# Scrape configuration
scrape_configs:
  - job_name: 'omnisight'
    static_configs:
      - targets:
        - 'camera1:8080'
        - 'camera2:8080'
        # ... more cameras
```

### Grafana Dashboard

Import dashboard from `deployment/grafana/omnisight-dashboard.json`:

**Metrics:**
- Frames per second
- Detection latency
- Active tracks
- Events predicted
- Interventions triggered
- Network bandwidth
- Swarm health
- Model accuracy

## Troubleshooting

### Common Issues

**Issue: Application fails to start**
```bash
# Check logs
ssh root@camera-ip "journalctl -u omnisight -f"

# Common causes:
# - Missing ML model files
# - Insufficient resources
# - DLPU not available
# - VDO initialization failure

# Solution: Check resource limits in manifest.json
```

**Issue: High CPU usage**
```bash
# Reduce FPS or resolution
curl -u root:password -X POST \
    "http://camera-ip/axis-cgi/param.cgi?action=update&Perception.FPS=15"

# Disable Timeline Threading temporarily
curl -u root:password -X POST \
    "http://camera-ip/axis-cgi/param.cgi?action=update&Timeline.Enabled=false"
```

**Issue: Swarm not connecting**
```bash
# Test MQTT connection
mosquitto_pub -h mqtt-broker -t "test" -m "hello" -u omnisight -P password

# Check firewall
sudo ufw allow 1883/tcp

# Verify MQTT broker is running
sudo systemctl status mosquitto
```

**Issue: False positive detections**
```bash
# Increase confidence threshold
curl -u root:password -X POST \
    "http://camera-ip/axis-cgi/param.cgi?action=update&Perception.ConfidenceThreshold=0.7"

# Enable swarm consensus
curl -u root:password -X POST \
    "http://camera-ip/axis-cgi/param.cgi?action=update&Swarm.ConsensusEnabled=true"
```

## Security Best Practices

### 1. Network Security

- Use TLS/SSL for MQTT (port 8883)
- Enable firewall rules
- Segment camera network (VLAN)
- Use VPN for remote access

### 2. Authentication

- Change default passwords
- Use strong passwords (16+ chars)
- Enable two-factor authentication
- Rotate credentials regularly

### 3. Access Control

- Limit ACAP installation to admins
- Use role-based access control (RBAC)
- Audit access logs
- Disable unused services

### 4. Data Privacy

- Use PRIVACY_METADATA_ONLY for public areas
- Enable differential privacy for federated learning
- Comply with GDPR/CCPA regulations
- Document data retention policies

## Performance Tuning

### For Real-Time Performance

```json
{
  "perception": {
    "fps": 30,
    "resolution": {"width": 1920, "height": 1080},
    "use_dlpu": true
  },
  "timeline": {
    "prediction_horizon_s": 30.0,
    "num_timelines": 3
  }
}
```

### For Resource-Constrained Cameras

```json
{
  "perception": {
    "fps": 15,
    "resolution": {"width": 1280, "height": 720},
    "use_dlpu": true,
    "max_tracks": 50
  },
  "timeline": {
    "prediction_horizon_s": 30.0,
    "num_timelines": 1,
    "branching_enabled": false
  }
}
```

### For High-Accuracy

```json
{
  "perception": {
    "fps": 30,
    "resolution": {"width": 1920, "height": 1080},
    "confidence_threshold": 0.7,
    "use_feature_similarity": true
  },
  "swarm": {
    "enabled": true,
    "consensus_threshold": 0.8
  }
}
```

## Backup and Recovery

### Backup Configuration

```bash
# Backup camera configuration
ssh root@camera-ip "cat /etc/omnisight/config.json" > backup/camera_${CAMERA_ID}_config.json

# Backup ML models
ssh root@camera-ip "tar czf /tmp/models.tar.gz /usr/local/packages/omnisight/models"
scp root@camera-ip:/tmp/models.tar.gz backup/
```

### Restore Configuration

```bash
# Restore configuration
scp backup/camera_${CAMERA_ID}_config.json root@camera-ip:/etc/omnisight/config.json

# Restart OMNISIGHT
ssh root@camera-ip "systemctl restart omnisight"
```

## Scaling to Large Deployments

### 100+ Camera Network

**Architecture:**
```
├─ Regional Coordinators (3-5)
│  ├─ Camera Clusters (20-30 cameras each)
│  │  ├─ Edge Cameras
│  │  ├─ Internal Cameras
│  │  └─ Gateway Cameras
│  └─ Local MQTT Broker
└─ Central Coordinator
   ├─ Federated Learning Aggregation
   ├─ Global Model Distribution
   └─ Centralized Alerting
```

**Deployment:**
```bash
# Use orchestration tool (Ansible, Puppet, etc.)
ansible-playbook -i inventory.yaml deploy_omnisight.yaml

# Configure load balancing
./scripts/setup_load_balancing.sh --cameras cameras.yaml
```

## License

Copyright © 2024 OMNISIGHT Project. All rights reserved.
