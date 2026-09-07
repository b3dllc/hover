#!/bin/bash
# Build the project and compare the result against the original hover.exe
# using reccmp (https://github.com/isledecomp/reccmp).
#
# Requires a Python environment with reccmp installed; set RECCMP_PYTHON to
# the interpreter to use (default: /workspace/tools/venv/bin/python).
#
# Usage:
#   ./compare.sh [args]         build, then compare annotated functions
#   ./compare.sh --data [args]  build, then compare annotated globals
#
# Extra arguments are passed through to the underlying reccmp tool, e.g.
#   ./compare.sh --verbose 0x41c100

set -e
cd "$(dirname "$0")"

RECCMP_PYTHON="${RECCMP_PYTHON:-/workspace/tools/venv/bin/python}"

if [ ! -x "$RECCMP_PYTHON" ]; then
	echo "error: reccmp python not found at $RECCMP_PYTHON" >&2
	echo "set RECCMP_PYTHON or install reccmp into a venv (pip install reccmp)" >&2
	exit 1
fi

./build.sh
"$RECCMP_PYTHON" tools/compare.py "$@"
