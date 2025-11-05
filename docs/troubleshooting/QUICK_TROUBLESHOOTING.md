# OMNISIGHT Quick Troubleshooting Guide

## 🚨 No "Open" Button After Installation

**Symptoms:**
- Application installs successfully
- Shows as installed in camera Apps page
- Toggle switch present but no "Open" button
- Application may show as "Off" or fail to start

**Most Common Cause:** Python dependency issue

**Quick Fix:**
```bash
# Use the universal native build
cd /Users/matthewvisher/Omnisight\ dev/OmniSight-dev
./scripts/build-phase4-v3-native-eap.sh

# Deploy: output/OMNISIGHT_-_Precognitive_Security_042_aarch64.eap
```

**Why it happens:** P3285-LVE (ARTPEC-9) cameras don't have Python 3

**Full details:** See [PHASE4_PYTHON_DEPENDENCY_ISSUE.md](PHASE4_PYTHON_DEPENDENCY_ISSUE.md)

---

## 🔍 How to Diagnose

### Step 1: Check System Logs

**Via Camera Web Interface:**
1. Navigate to Settings → System Options → Support
2. Click "System Log" or "Create Server Report"
3. Look for these errors:

**Python not found:**
```
python3: not found
Failed at step EXEC spawning /usr/local/packages/omnisight/omnisight
```
→ **Solution:** Use Phase 4 V3 (Native) build

**Missing files:**
```
Failed setting ownership of /usr/local/packages/omnisight.tmp/package.conf
```
→ **Solution:** Rebuild with updated script (v0.4.2+)

**Exec format error:**
```
Exec format error
```
→ **Solution:** Binary compiled for wrong architecture

### Step 2: Check Via SSH

```bash
# SSH to camera
ssh root@<camera-ip>

# Check if Python 3 exists
which python3

# Check application status
systemctl status sdkomnisight.service

# Check recent logs
journalctl -u sdkomnisight.service -n 50

# Check files installed
ls -la /usr/local/packages/omnisight/
```

---

## 🎯 Quick Reference: Which Build to Use

| Symptom | Use This Build |
|---------|----------------|
| **Any new deployment** | Phase 4 V3 Native (`build-phase4-v3-native-eap.sh`) |
| **P3285-LVE camera** | Phase 4 V3 Native |
| **M4228-LVE camera** | Phase 4 V3 Native |
| **Unknown camera** | Phase 4 V3 Native |
| **"python3: not found" error** | Phase 4 V3 Native |
| **No Open button** | Phase 4 V3 Native |

**Universal Rule:** Always use Phase 4 V3 Native unless you have a specific reason not to.

---

## 📋 Deployment Checklist

Before deploying:
- [ ] Built with `build-phase4-v3-native-eap.sh`
- [ ] Package is `v0.4.2` or later
- [ ] File size is ~308 KB

After deploying:
- [ ] Application shows as "Installed"
- [ ] **"Open" button appears** ← KEY CHECK
- [ ] Can toggle application ON
- [ ] Service shows as "Running"
- [ ] Can click "Open" and see dashboard

If any fail:
1. Download system logs from camera
2. Check for `python3: not found` error
3. Rebuild with Phase 4 V3 Native
4. Redeploy

---

## 🔧 Common Fixes

### Fix 1: Python Not Found
**Error:** `python3: not found`
**Solution:** Use Phase 4 V3 Native build
```bash
./scripts/build-phase4-v3-native-eap.sh
```

### Fix 2: Missing Config Files
**Error:** `Failed setting ownership of package.conf`
**Solution:** Use v0.4.0-fixed or later
```bash
./scripts/build-phase4-v3-native-eap.sh  # Has all configs
```

### Fix 3: Application Won't Stop
**Error:** Stop button doesn't work
**Solution:** V3 uses proper STARTMODE="respawn"
```bash
# Via SSH if needed
ssh root@<camera-ip>
systemctl stop sdkomnisight.service
```

### Fix 4: Port Already in Use
**Error:** Flask can't bind to port 8080
**Solution:** V3 falls back gracefully
```bash
# Check what's using port 8080
netstat -tulpn | grep 8080
```

---

## 📞 Getting Help

### Information to Collect

If you need to report an issue, collect:

1. **Camera model and firmware:**
   ```bash
   # Via camera web interface: Settings → About
   # Or via SSH:
   cat /etc/syslog-ng/syslog-ng.conf | grep DEVICE
   ```

2. **System logs:**
   - Download via Settings → System Options → Support → Create Server Report

3. **Application logs:**
   ```bash
   journalctl -u sdkomnisight.service -n 100 > /tmp/omnisight.log
   ```

4. **Package info:**
   - Which build script used
   - Package version (e.g., v0.4.2)
   - Package size

5. **Screenshot:**
   - Camera Apps page showing installation status

### Where to Look

- **Root cause analysis:** [PHASE4_PYTHON_DEPENDENCY_ISSUE.md](PHASE4_PYTHON_DEPENDENCY_ISSUE.md)
- **Build documentation:** [CLAUDE.md](CLAUDE.md#acap-packages)
- **Full README:** [README.md](README.md)
- **Cleanup summary:** [DOCUMENTATION_CLEANUP_SUMMARY.md](DOCUMENTATION_CLEANUP_SUMMARY.md)

---

## ✅ Success Indicators

You've successfully deployed when:

1. ✅ Package uploads without errors
2. ✅ Application shows "Installed" status
3. ✅ **"Open" button is visible**
4. ✅ Can toggle application ON
5. ✅ Service shows "Running" status
6. ✅ Clicking "Open" loads web dashboard
7. ✅ Dashboard shows system status
8. ✅ Can toggle application OFF successfully

---

## 🚀 Quick Start (TL;DR)

```bash
# 1. Build the universal package
cd /Users/matthewvisher/Omnisight\ dev/OmniSight-dev
./scripts/build-phase4-v3-native-eap.sh

# 2. Deploy via camera web interface
# Upload: output/OMNISIGHT_-_Precognitive_Security_042_aarch64.eap

# 3. Verify "Open" button appears

# 4. Click "Open" to access dashboard
```

**If no "Open" button:** Check system logs for `python3: not found` error

**Solution:** You already used the right build! Check [PHASE4_PYTHON_DEPENDENCY_ISSUE.md](PHASE4_PYTHON_DEPENDENCY_ISSUE.md) for additional diagnostics.

---

Last Updated: October 31, 2025
Current Version: v0.4.2 (Phase 4 V3 Native)
