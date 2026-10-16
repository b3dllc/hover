#!/bin/bash
# Build, refresh the inputs tools/equiv/equiv.py needs, and check every
# non-matching function for functional equivalence with hover.exe.
#
#   tools/equiv/run.sh [equiv.py args]     e.g. --runs 128, or 0x40f7f0 0x402250
#
# Writes build/equiv.json (per-function results) and prints one line per
# function; see tools/equiv/README.md for how to read it.
set -e
cd "$(dirname "$0")/../.."
PY="${RECCMP_PYTHON:-/workspace/tools/venv/bin/python}"
./build.sh > /dev/null
"$PY" tools/compare.py --json build/compare.json > /dev/null 2>&1
"$PY" tools/equiv/dump_matches.py build/matches.json > /dev/null 2>&1
[ -s build/winpops.json ] || "$PY" tools/equiv/winpops.py > build/winpops.json
exec "$PY" tools/equiv/equiv.py --json build/equiv.json "$@"
