# OMNISIGHT Hardware Compatibility

**Last Updated**: October 22, 2025
**Version**: 0.2.0

---

## Supported Hardware Platforms

### System-on-Chip (SoC) Requirements

OMNISIGHT is designed for Axis network cameras with the following SoCs:

#### ✅ ARTPEC-8 (Fully Supported)
- **DLPU**: Yes (Deep Learning Processing Unit)
- **Performance**: 10 FPS object detection
- **Inference**: <20ms per frame
- **Memory**: 256 MB recommended
- **Firmware**: AXIS OS 12.0 or later

#### ✅ ARTPEC-9 (Fully Supported)
- **DLPU**: Yes (Enhanced performance)
- **Performance**: 15-20 FPS object detection
- **Inference**: <15ms per frame
- **Memory**: 256 MB recommended
- **Firmware**: AXIS OS 12.6 or later

---

## Minimum Requirements

### Firmware
- **AXIS OS**: 12.0 or later
- **ACAP Version**: 4.0 or later
- **SDK**: ACAP SDK 3.5+

### Hardware Resources
- **CPU Cores**: 4 (quad-core)
- **RAM**: 512 MB minimum, 1 GB recommended
- **DLPU**: Required for real-time inference
- **Storage**: 50 MB for ACAP installation
- **Network**: Ethernet for MQTT swarm communication

---

## Tested Camera Models

While OMNISIGHT is SoC-agnostic (works on any ARTPEC-8/9 camera), it has been tested on:

### Primary Test Platform
- **Axis M4228-LVE**
  - SoC: ARTPEC-8
  - Firmware: AXIS OS 12.6.97
  - Status: ✅ Fully validated

### Additional Compatible Models
Any Axis camera with ARTPEC-8 or ARTPEC-9 SoC should work, including:

**ARTPEC-8 Models** (examples):
- M3xxx series
- M4xxx series
- P14xx series
- Q17xx series

**ARTPEC-9 Models** (examples):
- Latest M-series cameras
- Latest P-series cameras
- Latest Q-series cameras

> **Note**: Check your camera's specifications to verify SoC type. Look for "ARTPEC-8" or "ARTPEC-9" in the technical specifications.

---

## Feature Availability by SoC

| Feature | ARTPEC-8 | ARTPEC-9 |
|---------|----------|----------|
| Object Detection | ✅ 10 FPS | ✅ 15-20 FPS |
| Object Tracking | ✅ Full | ✅ Enhanced |
| Timeline Predictions | ✅ Full | ✅ Full |
| Swarm Intelligence | ✅ Full | ✅ Full |
| DLPU Acceleration | ✅ Yes | ✅ Enhanced |
| IPC Communication | ✅ Full | ✅ Full |
| Web Dashboard | ✅ Full | ✅ Full |

---

## Deployment Methods

### 1. Windows Laptop Workflow (Recommended for Testing)

**Prerequisites**:
- Windows 10/11
- Web browser (Chrome, Edge, Firefox)
- Network access to camera

**Steps**:
1. Open browser and navigate to camera IP: `https://<camera-ip>`
2. Login with admin credentials
3. Go to **Apps** → **Add app**
4. Upload `OMNISIGHT_-_Precognitive_Security_0_2_0_aarch64.eap`
5. Start the application
6. Access dashboard: `https://<camera-ip>/local/omnisight/`

### 2. Command-Line Deployment (Advanced)

**Prerequisites**:
- `curl` installed
- SSH access to camera

**Upload via curl**:
```bash
curl -k -u root:pass \
  -F "file=@OMNISIGHT_-_Precognitive_Security_0_2_0_aarch64.eap" \
  https://<camera-ip>/axis-cgi/applications/upload.cgi
```

**SSH deployment**:
```bash
scp OMNISIGHT_*.eap root@<camera-ip>:/usr/local/packages/
ssh root@<camera-ip> "systemctl restart omnisight"
```

---

## Performance Expectations

### ARTPEC-8 (M4228-LVE tested)

**Perception Engine**:
- Detection FPS: 10 Hz
- Inference latency: 15-20ms
- Max tracked objects: 50 concurrent

**Timeline Threading**:
- Prediction horizon: 5 minutes
- Active timelines: 3-5 concurrent
- Update frequency: 1 Hz

**Swarm Intelligence**:
- Max neighbor cameras: 10
- MQTT latency: <100ms
- Track sharing: Real-time

**System Resources**:
- CPU usage: 30-40%
- Memory usage: 128-256 MB
- Network bandwidth: 1-2 Mbps

### ARTPEC-9 (Projected)

**Perception Engine**:
- Detection FPS: 15-20 Hz
- Inference latency: 10-15ms
- Max tracked objects: 100 concurrent

**System Resources**:
- CPU usage: 25-35%
- Memory usage: 128-256 MB

---

## Limitations

### Current Release (v0.2.0)
- ⚠️ **DLPU models**: Limited to TensorFlow Lite models <10 MB
- ⚠️ **Swarm size**: Tested with up to 3 cameras (10 max recommended)
- ⚠️ **Video resolution**: Optimized for 1920x1080 (Full HD)
- ⚠️ **Network**: Requires stable Ethernet (Wi-Fi not recommended for swarm)

### Not Supported
- ❌ Cameras without DLPU (ARTPEC-7 and earlier)
- ❌ Non-Axis cameras
- ❌ AXIS OS versions below 12.0

---

## Verification Steps

After installing OMNISIGHT on your camera:

### 1. Check ACAP Status
```bash
# Via SSH
ssh root@<camera-ip>
systemctl status omnisight
```

### 2. Verify DLPU Access
```bash
# Check Larod API availability
larod-client list-models
```

### 3. Test API Endpoints
```bash
# From Windows laptop
curl https://<camera-ip>/local/omnisight/api/health
```

### 4. Monitor Logs
```bash
# Via SSH
journalctl -u omnisight -f
```

---

## Troubleshooting

### Common Issues

**ACAP won't start**:
- Check AXIS OS version (must be 12.0+)
- Verify DLPU availability: `cat /proc/cpuinfo | grep dlpu`
- Check storage space: `df -h`

**Low FPS performance**:
- Reduce video resolution to 1280x720
- Check CPU usage: `top`
- Verify DLPU is being used (check logs)

**API not accessible**:
- Check if reverse proxy is enabled
- Verify firewall settings on camera
- Test direct Flask port: `http://<camera-ip>:8080/api/health`

**Swarm not connecting**:
- Verify MQTT broker is reachable
- Check network connectivity between cameras
- Ensure cameras are on same subnet

---

## Support

For hardware-specific issues:
- Axis Support: https://www.axis.com/support
- ACAP Documentation: https://axiscommunications.github.io/acap-documentation/

For OMNISIGHT issues:
- GitHub Issues: https://github.com/oneshot2001/OmniSight-dev/issues

---

**Hardware Compatibility**: ARTPEC-8/9 SoC only
**Test Platform**: Axis M4228-LVE (ARTPEC-8, AXIS OS 12.6.97)
**Deployment**: Windows laptop via web interface (recommended)
