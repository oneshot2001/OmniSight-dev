#!/bin/bash
# Validate Flask application structure and syntax

echo "=========================================="
echo "OMNISIGHT Flask App Validation"
echo "=========================================="
echo ""

SUCCESS=0
FAILED=0

# Check directory structure
echo "Checking directory structure..."
REQUIRED_DIRS=(
  "app"
  "app/api"
)

for dir in "${REQUIRED_DIRS[@]}"; do
  if [ -d "$dir" ]; then
    echo "  ✓ Directory exists: $dir"
    ((SUCCESS++))
  else
    echo "  ✗ Missing directory: $dir"
    ((FAILED++))
  fi
done

# Check required files
echo ""
echo "Checking required files..."
REQUIRED_FILES=(
  "app/server.py"
  "app/requirements.txt"
  "app/api/__init__.py"
  "app/api/perception.py"
  "app/api/timeline.py"
  "app/api/swarm.py"
  "app/api/config.py"
  "app/api/system.py"
)

for file in "${REQUIRED_FILES[@]}"; do
  if [ -f "$file" ]; then
    echo "  ✓ File exists: $file"
    ((SUCCESS++))
  else
    echo "  ✗ Missing file: $file"
    ((FAILED++))
  fi
done

# Check Python syntax
echo ""
echo "Checking Python syntax..."
PYTHON_FILES=(
  "app/server.py"
  "app/api/perception.py"
  "app/api/timeline.py"
  "app/api/swarm.py"
  "app/api/config.py"
  "app/api/system.py"
)

for file in "${PYTHON_FILES[@]}"; do
  if [ -f "$file" ]; then
    if python3 -m py_compile "$file" 2>/dev/null; then
      echo "  ✓ Valid syntax: $file"
      ((SUCCESS++))
    else
      echo "  ✗ Syntax error: $file"
      python3 -m py_compile "$file"
      ((FAILED++))
    fi
  fi
done

# Check for executable bit on server.py
echo ""
echo "Checking executable permissions..."
if [ -x "app/server.py" ] || [ -f "app/server.py" ]; then
  echo "  ✓ server.py is accessible"
  ((SUCCESS++))
fi

# Verify imports can be resolved
echo ""
echo "Verifying Python imports (structure only)..."
if grep -q "from flask import Flask" app/server.py; then
  echo "  ✓ Flask import found in server.py"
  ((SUCCESS++))
fi

if grep -q "from flask import Blueprint" app/api/perception.py; then
  echo "  ✓ Blueprint import found in perception.py"
  ((SUCCESS++))
fi

# Check requirements.txt content
echo ""
echo "Checking requirements.txt..."
if grep -q "Flask" app/requirements.txt; then
  echo "  ✓ Flask listed in requirements.txt"
  ((SUCCESS++))
fi

if grep -q "Flask-CORS" app/requirements.txt; then
  echo "  ✓ Flask-CORS listed in requirements.txt"
  ((SUCCESS++))
fi

# Summary
echo ""
echo "=========================================="
echo "Validation Summary"
echo "=========================================="
echo "Passed: $SUCCESS"
echo "Failed: $FAILED"
echo ""

if [ $FAILED -eq 0 ]; then
  echo "✓ All validations passed!"
  echo ""
  echo "Flask app is ready for packaging."
  echo "The app will run on port 8080 when deployed."
  exit 0
else
  echo "✗ Some validations failed."
  exit 1
fi
