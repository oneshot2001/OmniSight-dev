# OMNISIGHT Log Collection Guide

**Purpose**: Collect system logs from M4228-LVE camera for performance analysis and optimization

**Last Updated**: October 22, 2025

---

## Overview

After running OMNISIGHT on your M4228-LVE camera, you'll need to collect system logs to analyze:
- Performance metrics (FPS, latency, CPU, memory)
- Error messages and warnings
- VDO capture statistics
- Larod inference performance
- IPC data flow metrics

This guide covers multiple methods for collecting logs from your Windows laptop.

---

## Method 1: Via Camera Web Interface (Easiest)

### Step 1: Access System Logs

1. **Open browser** and navigate to camera:
   ```
   https://192.168.1.100
   ```

2. **Login** with admin credentials

3. **Navigate to logs**:
   ```
   Settings → System → Logs → Application Logs
   ```

4. **Filter for OMNISIGHT**:
   - In the search/filter box, type: `omnisight`
   - Select time range (last hour, last day, etc.)

### Step 2: Download Logs

1. **Click "Download"** button (usually top-right)

2. **Save file** as:
   ```
   omnisight_logs_2025-10-22_15-30.txt
   ```

3. **Include timestamp** in filename for tracking

### Step 3: Download System Report

For comprehensive diagnostics:

1. **Navigate to**:
   ```
   Settings → System → Support → System Report
   ```

2. **Generate report** (takes 30-60 seconds)

3. **Download .zip file** containing:
   - System logs
   - Application logs
   - Performance metrics
   - Configuration files

---

## Method 2: Via SSH (Most Detailed)

### Prerequisites

- **SSH client**: PuTTY (Windows) or OpenSSH
- **SSH enabled** on camera
- **Root credentials**

### Download PuTTY (if needed)

1. Download from: https://www.chiark.greenend.org.uk/~sgtatham/putty/latest.html
2. Install `putty.exe`

### Step 1: Connect via SSH

**Using PuTTY**:
1. Open PuTTY
2. Host Name: `192.168.1.100`
3. Port: `22`
4. Connection Type: SSH
5. Click "Open"
6. Login: `root` / `<your password>`

**Using PowerShell** (Windows 10/11):
```powershell
ssh root@192.168.1.100
# Enter password when prompted
```

### Step 2: Collect OMNISIGHT Logs

Once connected via SSH, run these commands:

#### Basic Application Logs
```bash
# View real-time logs (Ctrl+C to stop)
journalctl -u omnisight -f

# Save last 1000 lines to file
journalctl -u omnisight -n 1000 > /tmp/omnisight_app.log

# Save logs from last 24 hours
journalctl -u omnisight --since "24 hours ago" > /tmp/omnisight_24h.log

# Save logs with timestamps
journalctl -u omnisight -o verbose > /tmp/omnisight_verbose.log
```

#### System Performance Logs
```bash
# CPU and memory usage
top -bn1 > /tmp/system_stats.log

# DLPU/Larod status
larod-client list-models > /tmp/larod_models.log
larod-client list-chips > /tmp/larod_chips.log

# Process list
ps aux | grep omnisight > /tmp/omnisight_processes.log

# Memory details
cat /proc/meminfo > /tmp/meminfo.log

# Disk usage
df -h > /tmp/disk_usage.log
```

#### IPC Status
```bash
# Shared memory
ls -lh /dev/shm/ > /tmp/shm_status.log

# Named pipes
ls -lh /tmp/omnisight* > /tmp/pipes_status.log

# Semaphores
ipcs -s > /tmp/semaphores.log
```

#### Network/MQTT Status (if swarm enabled)
```bash
# Network connections
netstat -tuln | grep 1883 > /tmp/mqtt_connections.log

# Ping other cameras
ping -c 5 192.168.1.101 > /tmp/camera2_ping.log
```

### Step 3: Download Log Files to Windows

**Method A: Using WinSCP (GUI)**:
1. Download WinSCP: https://winscp.net/
2. Install and open
3. Connect to `192.168.1.100` (SSH)
4. Navigate to `/tmp/`
5. Drag and drop log files to your PC

**Method B: Using SCP from PowerShell**:
```powershell
# Download single file
scp root@192.168.1.100:/tmp/omnisight_app.log C:\Users\YourName\Desktop\

# Download all log files
scp root@192.168.1.100:/tmp/omnisight*.log C:\Users\YourName\Desktop\logs\
scp root@192.168.1.100:/tmp/*.log C:\Users\YourName\Desktop\logs\
```

### Step 4: Create Combined Log Package

On camera (via SSH):
```bash
# Create logs directory
mkdir -p /tmp/omnisight_logs

# Copy all relevant logs
journalctl -u omnisight --since "1 hour ago" > /tmp/omnisight_logs/app.log
top -bn1 > /tmp/omnisight_logs/system_stats.log
larod-client list-models > /tmp/omnisight_logs/larod.log
ps aux > /tmp/omnisight_logs/processes.log
df -h > /tmp/omnisight_logs/disk.log
ipcs > /tmp/omnisight_logs/ipc.log

# Create timestamp
date > /tmp/omnisight_logs/timestamp.txt

# Create tarball
cd /tmp
tar -czf omnisight_logs_$(date +%Y%m%d_%H%M%S).tar.gz omnisight_logs/

# Download to Windows
# (Use WinSCP or scp command from PowerShell)
```

Then from Windows PowerShell:
```powershell
scp root@192.168.1.100:/tmp/omnisight_logs_*.tar.gz C:\Users\YourName\Desktop\
```

---

## Method 3: Automated Log Collection Script

I'll create a script you can run on the camera to collect everything automatically.

### Step 1: Create Collection Script

SSH to camera and create script:
```bash
cat > /tmp/collect_omnisight_logs.sh << 'EOF'
#!/bin/bash
# OMNISIGHT Log Collection Script
# Collects comprehensive logs for analysis

TIMESTAMP=$(date +%Y%m%d_%H%M%S)
LOG_DIR="/tmp/omnisight_logs_${TIMESTAMP}"
mkdir -p "${LOG_DIR}"

echo "OMNISIGHT Log Collection - ${TIMESTAMP}"
echo "========================================"

# Application logs
echo "Collecting application logs..."
journalctl -u omnisight --since "2 hours ago" > "${LOG_DIR}/app.log"
journalctl -u omnisight --since "2 hours ago" -o json > "${LOG_DIR}/app.json"

# System performance
echo "Collecting system performance..."
top -bn2 -d 1 > "${LOG_DIR}/top.log"
vmstat 1 5 > "${LOG_DIR}/vmstat.log"
iostat -x 1 5 > "${LOG_DIR}/iostat.log" 2>/dev/null || echo "iostat not available"

# Memory
echo "Collecting memory info..."
cat /proc/meminfo > "${LOG_DIR}/meminfo.log"
free -m > "${LOG_DIR}/free.log"

# Processes
echo "Collecting process info..."
ps aux > "${LOG_DIR}/processes.log"
ps aux | grep omnisight > "${LOG_DIR}/omnisight_processes.log"

# DLPU/Larod
echo "Collecting Larod info..."
larod-client list-models > "${LOG_DIR}/larod_models.log" 2>&1
larod-client list-chips > "${LOG_DIR}/larod_chips.log" 2>&1

# IPC
echo "Collecting IPC status..."
ls -lh /dev/shm/ > "${LOG_DIR}/shm.log" 2>&1
ls -lh /tmp/omnisight* > "${LOG_DIR}/pipes.log" 2>&1
ipcs -s > "${LOG_DIR}/semaphores.log" 2>&1
ipcs -m > "${LOG_DIR}/shared_memory.log" 2>&1

# Network
echo "Collecting network info..."
netstat -tuln > "${LOG_DIR}/netstat.log"
ifconfig > "${LOG_DIR}/ifconfig.log"

# Disk
echo "Collecting disk info..."
df -h > "${LOG_DIR}/disk.log"
du -sh /usr/local/packages/omnisight/* > "${LOG_DIR}/package_size.log" 2>&1

# Camera info
echo "Collecting camera info..."
cat /proc/cpuinfo > "${LOG_DIR}/cpuinfo.log"
uname -a > "${LOG_DIR}/uname.log"
cat /etc/os-release > "${LOG_DIR}/os_release.log" 2>&1

# Create metadata
echo "Creating metadata..."
cat > "${LOG_DIR}/metadata.txt" << METADATA
Collection Timestamp: ${TIMESTAMP}
Camera Model: $(cat /etc/axis-release 2>/dev/null | head -1)
Firmware: $(cat /etc/os-release 2>/dev/null | grep VERSION= | cut -d= -f2)
Uptime: $(uptime)
OMNISIGHT Version: $(journalctl -u omnisight -n 1 | grep "version" | tail -1)
METADATA

# Create tarball
echo "Creating archive..."
cd /tmp
tar -czf "omnisight_logs_${TIMESTAMP}.tar.gz" "omnisight_logs_${TIMESTAMP}/"

echo ""
echo "✅ Log collection complete!"
echo "📦 Archive: /tmp/omnisight_logs_${TIMESTAMP}.tar.gz"
echo ""
echo "Download with:"
echo "scp root@<camera-ip>:/tmp/omnisight_logs_${TIMESTAMP}.tar.gz ."
echo ""

# Cleanup old logs (keep last 5)
ls -t /tmp/omnisight_logs_*.tar.gz | tail -n +6 | xargs rm -f 2>/dev/null
EOF

chmod +x /tmp/collect_omnisight_logs.sh
```

### Step 2: Run Collection Script

```bash
/tmp/collect_omnisight_logs.sh
```

### Step 3: Download Results

From Windows PowerShell:
```powershell
scp root@192.168.1.100:/tmp/omnisight_logs_*.tar.gz C:\Users\YourName\Desktop\
```

---

## What to Collect for Each Test Phase

### Phase 1: Initial Deployment (Baseline)
**Collect**:
- ✅ Application startup logs
- ✅ Initial system stats
- ✅ Flask API startup messages

**Commands**:
```bash
journalctl -u omnisight --since "5 minutes ago" > baseline_startup.log
```

### Phase 2: VDO Testing (Video Capture)
**Collect**:
- ✅ VDO API logs
- ✅ Frame capture statistics
- ✅ FPS measurements over time

**Commands**:
```bash
# Run for 30 minutes during testing
journalctl -u omnisight -f > vdo_test_30min.log
# Ctrl+C when done
```

### Phase 3: DLPU Testing (Inference)
**Collect**:
- ✅ Larod API logs
- ✅ Inference latency measurements
- ✅ CPU/Memory usage during load
- ✅ DLPU utilization

**Commands**:
```bash
# Before test
larod-client list-models > dlpu_before.log

# During test (run in background)
top -b -d 5 > dlpu_performance.log &
TOP_PID=$!

# After test
kill $TOP_PID
larod-client list-models > dlpu_after.log
```

### Phase 4: IPC Testing (Data Flow)
**Collect**:
- ✅ IPC connection logs
- ✅ Shared memory stats
- ✅ API response times
- ✅ Data throughput metrics

**Commands**:
```bash
# Collect IPC status
ipcs -a > ipc_status.log

# Monitor shared memory
watch -n 1 'ls -lh /dev/shm/' > shm_monitor.log &
WATCH_PID=$!
sleep 300  # Run for 5 minutes
kill $WATCH_PID
```

### Phase 5: Long-term Stability (24+ hours)
**Collect**:
- ✅ Full system logs
- ✅ Memory leak detection
- ✅ Performance degradation tracking
- ✅ Error frequency analysis

**Commands**:
```bash
# Run automated collection every hour
while true; do
  /tmp/collect_omnisight_logs.sh
  sleep 3600  # 1 hour
done
```

---

## Log Analysis - What I'll Look For

When you send me logs, here's what I'll analyze:

### Performance Metrics
```
[INFO] Perception: FPS=10.2 Latency=15ms Objects=3
[INFO] Timeline: Predictions=2 Probability=0.75
[INFO] Swarm: Neighbors=2 Latency=85ms
```

**Analyzing for**:
- FPS stability (should stay 9-11 Hz)
- Latency trends (should be <20ms)
- Resource usage patterns

### Error Patterns
```
[ERROR] VDO: Frame dropped (buffer full)
[WARN] Larod: Inference timeout (25ms)
[ERROR] IPC: Semaphore wait failed
```

**Analyzing for**:
- Frequency of errors
- Error clustering (all at once vs. periodic)
- Root cause correlation

### Resource Usage
```
CPU: 45% (target <40%)
Memory: 280MB (target <256MB)
DLPU: 85% utilization (good)
```

**Analyzing for**:
- Resource bottlenecks
- Memory leaks (increasing over time)
- CPU spikes (what triggers them?)

### IPC Performance
```
[DEBUG] IPC: Frame published 12345 (0.15ms)
[DEBUG] IPC: Python read frame 12345 (0.22ms)
[DEBUG] IPC: Total latency 0.37ms (target <1ms)
```

**Analyzing for**:
- IPC latency breakdown
- Data flow bottlenecks
- Synchronization issues

---

## Log Formats and Structure

### Application Logs Format
```
2025-10-22 15:30:45.123 [INFO] OMNISIGHT: Module started
2025-10-22 15:30:45.234 [INFO] VDO: Video capture started (1920x1080 @ 10 FPS)
2025-10-22 15:30:45.345 [INFO] Larod: DLPU model loaded successfully
2025-10-22 15:30:45.456 [INFO] IPC: Server initialized
2025-10-22 15:30:45.567 [INFO] Flask: API server running on port 8080
```

### Performance Logs Format
```
Timestamp: 2025-10-22 15:31:00
CPU Usage: 35.2%
Memory: 210MB / 1024MB (20.5%)
FPS: 10.1
Latency: 16ms
Objects Tracked: 3
Active Timelines: 2
```

### Error Logs Format
```
2025-10-22 15:32:15.789 [ERROR] VDO: Frame capture failed
  Reason: Buffer overflow
  Details: 15 frames dropped in last minute
  Action: Reducing frame rate to 9 FPS
```

---

## Sharing Logs with Me

### Option 1: GitHub Issue (Public)
1. Create new issue: https://github.com/oneshot2001/OmniSight-dev/issues
2. Title: "Test Results - M4228-LVE - [Date]"
3. Attach log files (or paste if small)
4. Describe test scenario

### Option 2: Direct File Sharing (Private)
1. Upload to file sharing service:
   - Google Drive
   - Dropbox
   - WeTransfer
2. Share link in conversation

### Option 3: Paste in Chat (Small Logs)
For short logs (<1000 lines), you can paste directly:
```
Here are the startup logs:
[paste log content]
```

---

## Automated Analysis Report Template

After you send logs, I'll provide a report like this:

```markdown
# OMNISIGHT Test Analysis Report

**Test Date**: 2025-10-22
**Duration**: 1 hour
**Camera**: M4228-LVE (ARTPEC-8)
**Firmware**: AXIS OS 12.6.97
**OMNISIGHT Version**: 0.2.0

## Performance Summary

✅ **FPS**: 10.1 Hz avg (target: 10 Hz) - PASS
⚠️ **Latency**: 21ms avg (target: <20ms) - MARGINAL
✅ **CPU**: 35% avg (target: <40%) - PASS
⚠️ **Memory**: 270MB peak (target: <256MB) - MARGINAL

## Issues Detected

1. **Memory Usage Trending Up**
   - Started: 180MB
   - After 1h: 270MB
   - Trend: +90MB/hour
   - Diagnosis: Possible memory leak in detection buffer
   - Fix: Review buffer management in perception.c

2. **Latency Spikes**
   - 3 spikes to 35ms detected
   - Correlation: When 10+ objects in frame
   - Diagnosis: DLPU overload
   - Fix: Add object count throttling

## Optimizations Recommended

1. **Priority 1**: Fix memory leak (prevents long-term operation)
2. **Priority 2**: Add DLPU load balancing
3. **Priority 3**: Optimize IPC buffer size

## Next Test Iteration

I'll implement fixes and provide v0.2.1 for retesting.
Target improvements:
- Memory stable at 200MB
- Latency <18ms even with 10+ objects
```

---

## Quick Reference Commands

### Collect Everything (One Command)
```bash
/tmp/collect_omnisight_logs.sh
```

### Download to Windows
```powershell
scp root@192.168.1.100:/tmp/omnisight_logs_*.tar.gz C:\Users\YourName\Desktop\
```

### View Real-Time Logs
```bash
ssh root@192.168.1.100
journalctl -u omnisight -f
```

### Check Current Performance
```bash
ssh root@192.168.1.100
top -bn1 | grep omnisight
```

---

**Recommendation**: Run automated log collection script after each test phase, then send me the tarball for analysis!
