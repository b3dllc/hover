#!/bin/bash
# Build wrapper for Hover! decompilation project.
#
# Default toolset (TOOLSET=vc21): the Visual C++ 2.1 compiler and MFC 3.1
# libraries (/workspace/msvc21x/MSVC20, extracted from VCPP-2.10.iso) with
# the Windows 95 SDK headers shipped by Visual C++ 2.2 (/workspace/msvc22/
# MSVC20/INCLUDE). Measured 2026-09-04 against the original hover.exe:
#   - VC++ 2.1 and 2.2 ship byte-identical compilers, linker and CRT
#     (C1/C1XX/C2/LINK/LIBCMT all dated 1995-01-13/20); only MFC differs.
#   - MFC 3.1 (2.1) matches the original's statically linked MFC code better
#     than MFC 3.2 (2.2): 232 vs 209 byte-exact library functions, 9 vs 22
#     drifted; VC++ 2.0 (MFC 3.0, older compiler and CRT) is far worse.
#   - The original's object layouts use the Windows 95 SDK structures
#     (JOYCAPS is 0x194 bytes, WM_HELP/WM_CONTEXTMENU etc.), which the 2.0/2.1
#     SDK headers lack, so the headers come from 2.2's INCLUDE.
#   - Game-code accuracy with this combination: 75.4% vs 75.2% with 2.2.
#
# Other toolsets: TOOLSET=msvc22 (VC++ 2.2 / MFC 3.2, the previous default),
# TOOLSET=msvc21 (pure 2.1 with its pre-Win95 headers; does not compile the
# current sources), TOOLSET=msvc20 (VC++ 2.0), TOOLSET=msvc420 (VC++ 4.2).
# SDKINC overrides the directory whose INCLUDE supplies the Win32 headers.

set -e

ROOT="$(cd "$(dirname "$0")" && pwd)"

TOOLSET="${TOOLSET:-vc21}"
case "$TOOLSET" in
  vc21)    MSVC="/workspace/msvc21x/MSVC20"; SDKINC="${SDKINC:-/workspace/msvc22/MSVC20}" ;;
  msvc22)  MSVC="/workspace/msvc22/MSVC20" ;;
  msvc21)  MSVC="/workspace/msvc21x/MSVC20" ;;
  msvc20)  MSVC="/workspace/msvc20x/MSVC20" ;;
  msvc420) MSVC="/workspace/msvc420" ;;
  *) echo "unknown TOOLSET: $TOOLSET" >&2; exit 1 ;;
esac

ZMSVC="Z:\\$(echo "$MSVC" | sed 's|^/||; s|/|\\|g')"

export WINEDEBUG=-all
export WINEPATH="${ZMSVC}\\bin"
# SDKINC: directory whose INCLUDE holds the Win32 SDK headers (defaults to the
# toolset's own; set to /workspace/msvc22/MSVC20 to pair the VC++ 2.1 compiler
# and MFC 3.1 with the Windows 95 SDK headers).
SDKINC="${SDKINC:-$MSVC}"
ZSDK="Z:\\$(echo "$SDKINC" | sed 's|^/||; s|/|\\|g')"
export INCLUDE="${ZSDK}\\include;${ZMSVC}\\mfc\\include"
export LIB="${ZMSVC}\\lib;${ZMSVC}\\mfc\\lib"

cd "${ROOT}"

mkdir -p build

# Always build from clean: the compiler appends its type records to the
# previous link's build/hover.pdb (/Fd build\), which makes incremental
# builds non-deterministic by up to 0.2 points of reccmp accuracy
# (notes/perfect-match.md rule 187).  A full build takes ~4 s.
rm -f build/*.obj build/hover.pdb
wine "${MSVC}/bin/NMAKE.EXE" /nologo "$@"
