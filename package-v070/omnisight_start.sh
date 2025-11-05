#!/bin/sh
# OMNISIGHT Startup Script

# Set web root to ACAP installation directory
WEB_ROOT="/usr/local/packages/omnisight/html"

# Start OMNISIGHT server on port 8080
exec /usr/local/packages/omnisight/omnisight --port 8080 --web-root "$WEB_ROOT"
