# ACAP Development Insights from pandosme and Axis Examples

> Analysis Date: 2025-10-22
> Source: Fred Juhlin (pandosme) - Axis Developer
> Repository: https://github.com/pandosme/make_acap

## Overview

Fred Juhlin (pandosme) is an Axis developer who maintains excellent ACAP examples and templates. This document compares our current OMNISIGHT implementation with his patterns and official Axis recommendations.

## Key Findings

### 1. Manifest Schema Versions

**Our Current Approach:**
- Schema: `1.5` (ACAP_PACKAGING.md)
- Works with AXIS OS 12.6.97
- Basic configuration

**Pandosme's Approach:**
- Schema: `1.7.1` to `1.7.3` (most examples)
- Latest official: `1.8.0` (reverse proxy examples)
- More advanced features available

**Recommendation:**
✅ Consider upgrading to schema 1.7.3 or 1.8.0 for Phase 2 to access newer features like reverse proxy.

### 2. Web Server Implementation

**Our Current Approach (v0.1.8):**
```json
{
  "schemaVersion": "1.5",
  "setup": {
    "runMode": "respawn",
    "appId": 1001
  },
  "configuration": {
    "settingPage": "index.html"
  }
}
```

Uses: Python SimpleHTTPServer on port 8080

**Pandosme's Approach:**
```json
{
  "schemaVersion": "1.7.1",
  "setup": {
    "runMode": "once",
    "friendlyName": "Base",
    "appName": "base",
    "vendor": "Fred Juhlin",
    "embeddedSdkVersion": "3.0"
  },
  "configuration": {
    "settingPage": "index.html",
    "httpConfig": [
      {"name": "app", "access": "admin", "type": "fastCgi"},
      {"name": "settings", "access": "admin", "type": "fastCgi"}
    ]
  }
}
```

Uses: FastCGI endpoints for dynamic API calls

**Official Axis Reverse Proxy Approach (Schema 1.8.0):**
```json
{
  "schemaVersion": "1.8.0",
  "setup": {
    "appName": "web_server_rev_proxy",
    "runMode": "never"
  },
  "configuration": {
    "reverseProxy": [
      {
        "apiPath": "my_web_server",
        "target": "http://localhost:2001",
        "access": "admin"
      }
    ]
  }
}
```

Uses: Apache reverse proxy to internal web server (CivetWeb, etc.)

### 3. Comparison Matrix

| Feature | OMNISIGHT v0.1.8 | Pandosme Pattern | Axis Reverse Proxy |
|---------|------------------|------------------|-------------------|
| **Schema Version** | 1.5 | 1.7.1 | 1.8.0 |
| **Web Server** | Python HTTP | FastCGI | CivetWeb + Proxy |
| **Dynamic APIs** | ❌ No | ✅ Yes | ✅ Yes |
| **Authentication** | Via Apache | Via Apache | Via Apache |
| **TLS** | Via Apache | Via Apache | Via Apache |
| **Custom Port** | 8080 | N/A (Apache) | Internal only |
| **Complexity** | Low | Medium | Medium-High |
| **Best For** | Static UI | Dynamic APIs | Full web apps |

### 4. runMode Options Explained

**Our Current:** `"runMode": "respawn"`
- Restart if process exits
- Needed for our Python HTTP server to stay running
- ✅ Correct for our use case

**Pandosme Uses:** `"runMode": "once"`
- Restart only on device reboot
- Used when FastCGI handles requests (no persistent process needed)
- Good for event-driven applications

**Reverse Proxy Uses:** `"runMode": "never"`
- No automatic restart
- Used when web server is started manually or by other means
- Interesting: Can still serve web pages via reverse proxy!

### 5. appId Discovery

**Our Implementation:**
```json
"appId": 1001
```

**Important Finding:**
- appId MUST be numeric integer (not string) ✅ We got this right!
- This was our critical fix in v0.1.1
- Pandosme examples confirm: numeric values only

### 6. Missing Fields We Should Consider

**Fields Pandosme Uses That We Don't:**

1. **friendlyName**
   ```json
   "friendlyName": "OMNISIGHT Precognitive Security"
   ```
   - Displayed in camera UI (nicer than technical appName)
   - We should add this!

2. **embeddedSdkVersion**
   ```json
   "embeddedSdkVersion": "3.0"
   ```
   - Ensures compatibility with older firmware
   - Recommended by Axis for all new apps

3. **vendorUrl**
   ```json
   "vendorUrl": "https://github.com/oneshot2001/OmniSight-dev"
   ```
   - Provides link to documentation/support
   - Professional touch

### 7. Web Server Evolution Path

**Phase 1 (Current - ✅ WORKING):**
- Python SimpleHTTPServer
- Static HTML interface
- `runMode: "respawn"`
- Perfect for demo/stub phase

**Phase 2 Option A - FastCGI (Pandosme Pattern):**
```json
"httpConfig": [
  {"name": "api/perception", "access": "admin", "type": "fastCgi"},
  {"name": "api/timeline", "access": "admin", "type": "fastCgi"},
  {"name": "api/swarm", "access": "admin", "type": "fastCgi"}
]
```
- Pros: Native ACAP approach, well-documented
- Cons: C-based, more complex

**Phase 2 Option B - Reverse Proxy (Modern Axis):**
```json
"reverseProxy": [
  {
    "apiPath": "omnisight_api",
    "target": "http://localhost:8080",
    "access": "admin"
  }
]
```
- Pros: Keep Python/any web framework, modern approach
- Cons: Requires schema 1.8.0, more setup

### 8. Project Structure Comparison

**Pandosme's Template:**
```
.
├── app/
│   ├── ACAP.c/h          # SDK abstraction layer
│   ├── main.c            # Application entry point
│   ├── settings/
│   │   ├── events.json
│   │   └── settings.json
│   ├── html/
│   │   ├── index.html
│   │   └── js/
│   └── manifest.json
├── Dockerfile
```

**Our Current:**
```
.
├── src/                  # Source code (stub implementations)
├── scripts/
│   ├── build-eap.sh
│   └── manifest-camera.json
├── html/
│   ├── index.html
│   └── img/
├── Dockerfile
├── docker-compose.yml
```

**Recommendations:**
- ✅ Our structure is fine for Phase 1
- Consider adopting `settings/` directory for Phase 2
- Add `events.json` when we implement real event detection

### 9. Build Process Comparison

**Pandosme's Build:**
```bash
docker build --build-arg ARCH=aarch64 --tag acap .
docker cp $(docker create acap):/opt/app ./build
cp build/*.eap .
```

**Our Current Build:**
```bash
./scripts/build-eap.sh
```

**Key Difference:**
- Pandosme uses Axis SDK Docker images
- We use custom Docker + acap-build tool
- Both valid approaches

### 10. Recommended Improvements for Phase 2

Based on pandosme's patterns and Axis best practices:

#### Immediate (Low Effort, High Value):

1. **Add Missing Manifest Fields:**
   ```json
   {
     "schemaVersion": "1.7.3",
     "acapPackageConf": {
       "setup": {
         "appName": "omnisight",
         "friendlyName": "OMNISIGHT Precognitive Security",
         "vendor": "OMNISIGHT",
         "vendorUrl": "https://github.com/oneshot2001/OmniSight-dev",
         "embeddedSdkVersion": "3.0",
         "version": "0.2.0",
         "appId": 1001,
         "architecture": "aarch64",
         "runMode": "respawn"
       }
     }
   }
   ```

2. **Add Dynamic User (Security):**
   ```json
   "user": {
     "username": "dynamic",
     "group": "addon",
     "secondaryGroups": ["storage"]
   }
   ```

#### Phase 2A - Keep Python, Add APIs:

Use reverse proxy pattern (schema 1.8.0):
```json
"configuration": {
  "settingPage": "index.html",
  "reverseProxy": [
    {
      "apiPath": "api",
      "target": "http://localhost:8080",
      "access": "admin"
    }
  ]
}
```

Then implement Python Flask/FastAPI for real APIs:
- `/api/perception/status`
- `/api/timeline/predictions`
- `/api/swarm/network`

#### Phase 2B - FastCGI Pattern:

Follow pandosme's C-based FastCGI approach:
- More native to ACAP
- Better performance
- More complex to implement

### 11. Key Lessons Learned

1. **appId Numeric** ✅ We discovered this through troubleshooting
2. **runMode for Web Servers** ✅ We discovered "respawn" requirement
3. **Schema Versions** - We can upgrade for more features
4. **Reverse Proxy** - Modern alternative to our current approach
5. **FastCGI** - Pandosme's preferred pattern for APIs
6. **Dynamic User** - Better security than static sdk user

### 12. Validation of Our Approach

**What We Did Right:**
- ✅ appId is numeric (critical fix in v0.1.1)
- ✅ runMode: "respawn" for persistent web server
- ✅ Python server is platform-independent
- ✅ Lightweight package (2.8KB)
- ✅ Web interface works perfectly

**What We Could Improve:**
- ⚠️ Schema version (1.5 → 1.7.3 or 1.8.0)
- ⚠️ Missing friendlyName, embeddedSdkVersion, vendorUrl
- ⚠️ No dynamic APIs yet (Phase 2)
- ⚠️ Static sdk user instead of dynamic user

## Conclusion

Our current OMNISIGHT v0.1.8 implementation is **solid and working** for Phase 1 stub/demo purposes. The Python SimpleHTTPServer approach was the right choice for getting a working web interface quickly.

For Phase 2, we should:

1. **Upgrade schema to 1.7.3 or 1.8.0**
2. **Add missing manifest fields** (friendlyName, embeddedSdkVersion, vendorUrl)
3. **Implement dynamic user** for better security
4. **Choose API approach:**
   - **Option A:** Reverse proxy + Python Flask (easier, modern)
   - **Option B:** FastCGI + C (native, pandosme pattern)

Pandosme's repositories and the official Axis examples provide excellent reference implementations we can learn from as we build out real functionality.

## Resources

- **Pandosme GitHub:** https://github.com/pandosme
- **make_acap Template:** https://github.com/pandosme/make_acap
- **Axis ACAP Examples:** https://github.com/AxisCommunications/acap-native-sdk-examples
- **Manifest Schema Docs:** https://developer.axis.com/acap/develop/manifest-schemas/
- **Reverse Proxy Guide:** https://developer.axis.com/acap/develop/web-server-via-reverse-proxy/

---

**Analysis by:** Claude Code
**Date:** 2025-10-22
**Status:** Phase 1 Complete, Phase 2 Recommendations Ready
