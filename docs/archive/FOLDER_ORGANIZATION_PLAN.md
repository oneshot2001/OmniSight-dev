# OMNISIGHT Folder Organization Plan

**Date:** October 31, 2025
**Purpose:** Clean up project structure for better maintainability

## Current Issues

1. **70+ markdown files in root directory** - Hard to find documentation
2. **Multiple .eap files in root** - Unclear which is current
3. **Temporary build directories** - build-phase4, build-phase4-fixed, etc.
4. **Duplicate packages** - native/, output/, and root all have .eap files
5. **Unclear documentation structure** - Phase docs scattered
6. **Old test scripts in root** - Should be in tests/ or scripts/

## Proposed Organization

```
omnisight/
├── README.md                          # Main project README
├── CLAUDE.md                          # AI assistant instructions
├── QUICK_TROUBLESHOOTING.md           # Quick reference
├── LICENSE
├── .gitignore
├── Makefile
├── docker-compose.yml
├── Dockerfile
│
├── docs/                              # 📚 ALL DOCUMENTATION
│   ├── README.md                      # Documentation index
│   ├── guides/                        # User guides
│   │   ├── QUICKSTART.md
│   │   ├── INSTALLATION.md
│   │   ├── DEPLOYMENT.md
│   │   ├── TESTING_AND_DEPLOYMENT.md
│   │   └── WINDOWS_TESTING_GUIDE.md
│   ├── development/                   # Developer docs
│   │   ├── DEVELOPMENT.md
│   │   ├── STUB_BUILD.md
│   │   └── API_ENDPOINTS.md
│   ├── acap/                          # ACAP platform docs
│   │   ├── ACAP_PACKAGING.md
│   │   ├── ACAP_INSIGHTS_FROM_PANDOSME.md
│   │   └── HARDWARE_COMPATIBILITY.md
│   ├── phase1/                        # Phase 1 docs
│   │   ├── PHASE1_STATUS.md
│   │   └── STUB_BUILD_COMPLETE.md
│   ├── phase2/                        # Phase 2 docs
│   │   ├── PHASE2_ROADMAP.md
│   │   ├── PHASE2_IPC_IMPLEMENTATION.md
│   │   ├── PHASE2_CODE_REVIEW.md
│   │   └── PHASE2_TASK1_COMPLETE.md
│   ├── phase3/                        # Phase 3 docs
│   │   ├── PHASE3_IMPLEMENTATION_PLAN.md
│   │   ├── PHASE3_STATUS.md
│   │   ├── PHASE3_DEPLOYMENT_SUCCESS.md
│   │   └── tasks/
│   │       ├── PHASE3_TASK_1_1_VDO_CAPTURE.md
│   │       ├── PHASE3_TASK_1_2_LAROD_INFERENCE.md
│   │       ├── PHASE3_TASK_1_3_OBJECT_TRACKING.md
│   │       ├── PHASE3_TASK_1_4_BEHAVIOR_ANALYSIS.md
│   │       └── PHASE3_TASK_3_1_MQTT_CLIENT.md
│   ├── phase4/                        # Phase 4 docs (CURRENT)
│   │   ├── PHASE4_CLAUDE_FLOW_IMPLEMENTATION.md
│   │   ├── PHASE4_PYTHON_DEPENDENCY_ISSUE.md ⚠️ IMPORTANT
│   │   ├── PHASE4_DEPLOYMENT_GUIDE.md
│   │   ├── PHASE4_V2_DEPLOYMENT_SUMMARY.md
│   │   └── PHASE4_COMPLETION_SUMMARY.md
│   ├── troubleshooting/               # Troubleshooting docs
│   │   ├── DEPLOYMENT_FIX.md
│   │   ├── LOG_COLLECTION_GUIDE.md
│   │   └── deployment-fixes/
│   │       ├── V0.2.1_FAILURE_ANALYSIS.md
│   │       ├── V0.2.3_PYTHON_DETECTION_FIX.md
│   │       ├── V0.2.4_SIMPLIFIED_LAUNCHER_FIX.md
│   │       └── V0.3.0_DEPLOYMENT_ANALYSIS.md
│   └── archive/                       # Old/superseded docs
│       ├── DEPLOYMENT_INSTRUCTIONS_V0.2.0.md
│       ├── FLASK_API_TEST_RESULTS.md
│       ├── CLAUDE_FLOW_INTEGRATION_SPEC.md
│       └── old-phase-docs/
│
├── scripts/                           # 🔧 BUILD & DEPLOYMENT SCRIPTS
│   ├── README.md                      # Script documentation
│   ├── build-phase4-v3-native-eap.sh  # ⭐ CURRENT RECOMMENDED
│   ├── build-stub.sh
│   ├── deprecated/                    # Old scripts (with warnings)
│   │   ├── build-eap.sh
│   │   ├── build-phase2-eap.sh
│   │   ├── build-phase4-fixed-eap.sh
│   │   └── build-phase4-v2-eap.sh
│   └── testing/                       # Test scripts
│       ├── test-flask-api.sh
│       ├── test-ipc-integration.sh
│       ├── validate_flask_app.sh
│       └── QUICK_API_TEST.sh
│
├── packages/                          # 📦 BUILT PACKAGES
│   ├── current/
│   │   └── OMNISIGHT_-_Precognitive_Security_042_aarch64.eap ⭐
│   ├── phase2/
│   │   ├── OMNISIGHT_-_Precognitive_Security_0_2_3_aarch64.eap
│   │   └── OMNISIGHT_-_Precognitive_Security_0_2_4_aarch64.eap
│   ├── phase3/
│   │   ├── OMNISIGHT_-_Stub_Demo_0_3_1_aarch64.eap
│   │   ├── OMNISIGHT_Stub_0_3_0-stub_aarch64.eap
│   │   └── OMNISIGHT_WORKING_0_3_1_aarch64.eap
│   └── phase4/
│       ├── OMNISIGHT_-_Precognitive_Security_040_aarch64.eap (failed)
│       ├── OMNISIGHT_-_Precognitive_Security_041_aarch64.eap (failed)
│       └── OMNISIGHT_-_Precognitive_Security_042_aarch64.eap (current)
│
├── src/                               # 💻 SOURCE CODE
│   ├── core/
│   ├── perception/
│   ├── timeline/
│   ├── swarm/
│   └── ipc/
│
├── app/                               # 🐍 PYTHON API SERVER
│   ├── server.py
│   ├── api/
│   ├── coordinator/
│   └── ipc/
│
├── web/                               # 🌐 WEB INTERFACE
│   └── frontend/
│       ├── src/
│       └── public/
│
├── tests/                             # 🧪 TESTS
│   ├── unit/
│   ├── integration/
│   └── phase4/
│
├── models/                            # 🤖 ML MODELS
│   ├── perception/
│   └── timeline/
│
├── config/                            # ⚙️ CONFIGURATION
│   └── default.conf
│
└── build/                             # 🏗️ BUILD ARTIFACTS (gitignored)
    ├── stub/
    └── phase4/
```

## Cleanup Actions

### 1. Create New Directory Structure

```bash
mkdir -p docs/{guides,development,acap,phase1,phase2,phase3,phase4,troubleshooting,archive}
mkdir -p docs/phase3/tasks
mkdir -p docs/troubleshooting/deployment-fixes
mkdir -p scripts/{deprecated,testing}
mkdir -p packages/{current,phase2,phase3,phase4}
mkdir -p build
```

### 2. Move Documentation

**Guides:**
- QUICKSTART.md → docs/guides/
- INSTALLATION.md → docs/guides/
- DEPLOYMENT.md → docs/guides/
- TESTING_AND_DEPLOYMENT.md → docs/guides/
- WINDOWS_TESTING_GUIDE.md → docs/guides/
- STUB_DEPLOYMENT_GUIDE.md → docs/guides/

**Development:**
- DEVELOPMENT.md → docs/development/
- STUB_BUILD.md → docs/development/
- API_ENDPOINTS.md → docs/development/
- PERCEPTION_CODE_REFERENCE.md → docs/development/
- DOCUMENTATION_CLEANUP_SUMMARY.md → docs/development/

**ACAP:**
- ACAP_PACKAGING.md → docs/acap/
- ACAP_INSIGHTS_FROM_PANDOSME.md → docs/acap/
- HARDWARE_COMPATIBILITY.md → docs/acap/

**Phase 1:**
- PHASE1_STATUS.md → docs/phase1/
- STUB_BUILD_COMPLETE.md → docs/phase1/

**Phase 2:**
- PHASE2_*.md → docs/phase2/

**Phase 3:**
- PHASE3_*.md → docs/phase3/
- PHASE3_TASK_*.md → docs/phase3/tasks/

**Phase 4:**
- PHASE4_*.md → docs/phase4/

**Troubleshooting:**
- DEPLOYMENT_FIX.md → docs/troubleshooting/
- LOG_COLLECTION_GUIDE.md → docs/troubleshooting/
- docs/V0.*.md → docs/troubleshooting/deployment-fixes/
- docs/DEPLOYMENT_FAILURE_ANALYSIS.md → docs/troubleshooting/deployment-fixes/

**Archive:**
- DEPLOYMENT_INSTRUCTIONS_V0.2.0.md → docs/archive/
- FLASK_API_TEST_RESULTS.md → docs/archive/
- CLAUDE_FLOW_INTEGRATION_SPEC.md → docs/archive/
- PERCEPTION_PHASE3_INTEGRATION.md → docs/archive/

### 3. Move Packages

**Current:**
- output/OMNISIGHT_-_Precognitive_Security_042_aarch64.eap → packages/current/

**Phase 2:**
- OMNISIGHT_-_Precognitive_Security_0_2_*.eap → packages/phase2/

**Phase 3:**
- OMNISIGHT_*_0_3_*.eap → packages/phase3/
- native/*.eap → packages/phase3/

**Phase 4:**
- output/OMNISIGHT_-_Precognitive_Security_04*.eap → packages/phase4/

### 4. Move Scripts

**Deprecated:**
- scripts/build-eap.sh → scripts/deprecated/
- scripts/build-phase2-eap.sh → scripts/deprecated/
- scripts/build-phase4-fixed-eap.sh → scripts/deprecated/
- scripts/build-phase4-v2-eap.sh → scripts/deprecated/
- scripts/build-hybrid-eap.sh → scripts/deprecated/
- scripts/build-stub-eap*.sh → scripts/deprecated/

**Testing:**
- test-flask-api.sh → scripts/testing/
- test-ipc-integration.sh → scripts/testing/
- validate_flask_app.sh → scripts/testing/
- QUICK_API_TEST.sh → scripts/testing/

### 5. Clean Up Temporary Directories

**Remove:**
- build-phase4/
- build-phase4-fixed/
- build-phase4-v2/
- package/
- package-hybrid/
- package-phase2/
- package-phase4-v3/
- temp-check/
- temp-extract/

**Move build artifacts to build/ (gitignored):**
- build-stub/ → build/stub/

### 6. Keep in Root

**Essential files only:**
- README.md
- CLAUDE.md
- QUICK_TROUBLESHOOTING.md
- LICENSE
- .gitignore
- Makefile
- docker-compose.yml
- Dockerfile
- manifest.json
- package.conf

## Benefits

1. ✅ **Clear navigation** - Logical folder structure
2. ✅ **Easy to find docs** - Organized by phase and purpose
3. ✅ **Clean root directory** - Only essential files
4. ✅ **Archived old versions** - Keep history but not in the way
5. ✅ **Obvious current build** - packages/current/
6. ✅ **Deprecated scripts isolated** - scripts/deprecated/
7. ✅ **Better git history** - Less clutter in commits
8. ✅ **Easier onboarding** - New developers find things quickly

## Implementation Order

1. Create new directory structure
2. Move documentation files
3. Move package files
4. Reorganize scripts
5. Clean up temporary directories
6. Update README.md with new structure
7. Update CLAUDE.md with new paths
8. Update .gitignore
9. Test builds still work
10. Commit changes

## Estimated Time

- Planning: ✅ Complete
- Execution: ~30 minutes
- Testing: ~15 minutes
- Documentation: ~15 minutes

**Total:** ~1 hour

## Post-Cleanup Validation

- [ ] All builds work from scripts/
- [ ] Documentation is findable
- [ ] Root directory has <15 files
- [ ] No broken links in docs
- [ ] CLAUDE.md updated
- [ ] README.md updated
- [ ] .gitignore updated
