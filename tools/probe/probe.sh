#!/bin/bash
# Compile a codegen probe snippet with MSVC 2.2 under Wine and disassemble it.
#
# Usage: tools/probe/probe.sh <file.c> [/O2]     (default flags: /O2 /MT /GX)
#
# Give the probe file as a path relative to the current directory (e.g.
# tools/probe/foo.cpp): CL.EXE treats an absolute /path as a command-line
# option and reports "missing source filename".
#
# The disassembly lands on stdout; edit the probe file, rerun, and compare
# against the original bytes (objdump -d -M intel --start-address=0xADDR
# /workspace/hover/hover.exe).
set -e
SRC="$1"
OPT="${2:-/O2}"
export WINEDEBUG=-all
export WINEPATH='Z:\workspace\msvc22\MSVC20\bin'
export INCLUDE='Z:\workspace\msvc22\MSVC20\include;Z:\workspace\msvc22\MSVC20\mfc\include'
export LIB='Z:\workspace\msvc22\MSVC20\lib'
OBJ="$(mktemp -d)/probe.obj"
wine /workspace/msvc22/MSVC20/BIN/CL.EXE /nologo /MT /W3 /GX "$OPT" /c "$SRC" /Fo"$OBJ"
objdump -d -M intel "$OBJ"
