#!/bin/bash
#
# COMPLETE REBUILD - Minimal ACAP Package from Scratch
#
# Strategy: Build the absolute minimum ACAP package that will show an Open button
# Based on: Official Axis ACAP requirements and working v0.3.1 analysis
#

set -e

PROJECT_ROOT="/Users/matthewvisher/Omnisight dev/OmniSight-dev"
PACKAGE_DIR="$PROJECT_ROOT/package-minimal-clean"
OUTPUT_DIR="$PROJECT_ROOT/packages/clean"

echo "================================================"
echo "OMNISIGHT - COMPLETE REBUILD FROM SCRATCH"
echo "Minimal ACAP Package - Clean Build"
echo "================================================"
echo ""
echo "Strategy: Absolute minimum configuration"
echo "Goal: Make the Open button appear"
echo ""

# Clean and create directories
rm -rf "$PACKAGE_DIR"
rm -rf "$OUTPUT_DIR"
mkdir -p "$PACKAGE_DIR/html"
mkdir -p "$PACKAGE_DIR/lib"
mkdir -p "$OUTPUT_DIR"

echo "Step 1: Creating minimal manifest.json..."
cat > "$PACKAGE_DIR/manifest.json" <<'EOF'
{
  "schemaVersion": "1.5",
  "acapPackageConf": {
    "setup": {
      "appName": "omnisightclean",
      "friendlyName": "OMNISIGHT Clean",
      "vendor": "OMNISIGHT",
      "version": "1.0.0",
      "embeddedSdkVersion": "3.0",
      "runMode": "never",
      "architecture": "aarch64"
    },
    "configuration": {
      "settingPage": "index.html"
    }
  }
}
EOF
echo "  ✓ manifest.json created (schema 1.5, settingPage ONLY)"

echo ""
echo "Step 2: Creating minimal package.conf..."
cat > "$PACKAGE_DIR/package.conf" <<'EOF'
PACKAGENAME="OMNISIGHT Clean"
APPTYPE="aarch64"
APPNAME="omnisightclean"
APPID=""
LICENSENAME="Available"
LICENSEPAGE="none"
VENDOR="OMNISIGHT"
REQEMBDEVVERSION="3.0"
APPMAJORVERSION="1"
APPMINORVERSION="0"
APPMICROVERSION="0"
APPGRP="sdk"
APPUSR="sdk"
APPOPTS=""
OTHERFILES=""
SETTINGSPAGEFILE="index.html"
SETTINGSPAGETEXT=""
VENDORHOMEPAGELINK=""
PREUPGRADESCRIPT=""
POSTINSTALLSCRIPT=""
STARTMODE="never"
HTTPCGIPATHS=""
EOF
echo "  ✓ package.conf created (SETTINGSPAGEFILE='index.html')"

echo ""
echo "Step 3: Creating minimal param.conf..."
touch "$PACKAGE_DIR/param.conf"
echo "  ✓ param.conf created (empty)"

echo ""
echo "Step 4: Creating simple HTML page..."
cat > "$PACKAGE_DIR/html/index.html" <<'HTMLEOF'
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>OMNISIGHT - Clean Build</title>
    <style>
        body {
            font-family: Arial, sans-serif;
            max-width: 800px;
            margin: 50px auto;
            padding: 20px;
            background: #f5f5f5;
        }
        .container {
            background: white;
            padding: 40px;
            border-radius: 10px;
            box-shadow: 0 2px 10px rgba(0,0,0,0.1);
        }
        h1 {
            color: #333;
            margin-bottom: 20px;
        }
        .success {
            background: #4CAF50;
            color: white;
            padding: 20px;
            border-radius: 5px;
            text-align: center;
            font-size: 1.2em;
            margin: 20px 0;
        }
        .info {
            background: #e3f2fd;
            padding: 15px;
            border-radius: 5px;
            margin: 15px 0;
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>🎯 OMNISIGHT - Clean Build v1.0.0</h1>

        <div class="success">
            ✓ SUCCESS! The Open button worked!
        </div>

        <div class="info">
            <h3>Configuration Details:</h3>
            <ul>
                <li><strong>Schema:</strong> 1.5 (simplified)</li>
                <li><strong>settingPage:</strong> index.html</li>
                <li><strong>SETTINGSPAGEFILE:</strong> index.html</li>
                <li><strong>runMode:</strong> never</li>
                <li><strong>Package:</strong> Minimal clean build</li>
            </ul>
        </div>

        <div class="info">
            <h3>Build Strategy:</h3>
            <p>This is a <strong>minimal clean build</strong> with only the essential files:</p>
            <ul>
                <li>manifest.json (schema 1.5)</li>
                <li>package.conf</li>
                <li>param.conf (empty)</li>
                <li>html/index.html</li>
            </ul>
            <p><strong>NO binary, NO libraries, NO complex configuration.</strong></p>
        </div>

        <p style="text-align: center; margin-top: 30px; color: #666;">
            If you're reading this, the ACAP package loaded successfully<br>
            and the Open button appeared in the camera web interface!
        </p>
    </div>
</body>
</html>
HTMLEOF
echo "  ✓ index.html created"

echo ""
echo "Step 5: Creating LICENSE file..."
cat > "$PACKAGE_DIR/LICENSE" <<'LICEOF'
MIT License

Copyright (c) 2025 OMNISIGHT

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction.
LICEOF
echo "  ✓ LICENSE created"

echo ""
echo "Step 6: Creating empty lib directory placeholder..."
touch "$PACKAGE_DIR/lib/.keep"
echo "  ✓ lib/ directory created"

echo ""
echo "Step 7: Creating package archive..."
cd "$PACKAGE_DIR"
tar czf "$OUTPUT_DIR/OMNISIGHT_CLEAN_v1_0_0_aarch64.eap" .
cd - > /dev/null
echo "  ✓ Package created"

echo ""
echo "================================================"
echo "✓ CLEAN BUILD COMPLETE!"
echo "================================================"
echo ""
echo "Package: $OUTPUT_DIR/OMNISIGHT_CLEAN_v1_0_0_aarch64.eap"
echo "Size: $(ls -lh "$OUTPUT_DIR/OMNISIGHT_CLEAN_v1_0_0_aarch64.eap" | awk '{print $5}')"
echo ""
echo "FILES INCLUDED:"
echo "  - manifest.json (schema 1.5, settingPage)"
echo "  - package.conf (SETTINGSPAGEFILE)"
echo "  - param.conf (empty)"
echo "  - html/index.html"
echo "  - LICENSE"
echo "  - lib/ (empty directory)"
echo ""
echo "FILES NOT INCLUDED:"
echo "  - NO binary executable"
echo "  - NO app/ directory"
echo "  - NO complex configurations"
echo ""
echo "This is the MINIMAL package that should show an Open button."
echo "Ready for testing on P3285-LVE!"
echo ""
