#!/usr/bin/env bash
set -euo pipefail

cd "$(dirname "$0")/.."

# Configure with AMBuild (creates objdir folder)
pdm run python configure.py

# Build from the objdir folder
(cd objdir && pdm run ambuild)

echo
echo "=== Build Complete ==="
echo "Static library: objdir/src/cs2-kit.lib"
