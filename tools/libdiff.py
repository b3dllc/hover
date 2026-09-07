#!/usr/bin/env python3
"""Byte-compare library functions in the original hover.exe against the same
functions in our MSVC 2.2/MFC 3.2 build, to measure library version drift.

Input: tools/libdiff-fuzzy.tsv (orig VA, byte size, mangled name) — one row
per library function located in the original by the coverage fingerprint
analysis. Our build's addresses come from build/hover.map.

Categories per function:
  exact      - normalized instruction streams are identical
  drift      - same instruction count, <=6 differing instructions
  different  - everything else (length mismatch or large diff count)

This measures *drift* between library versions; to name the original's
library functions for reccmp (a superset of the same matching, kept 1:1 and
verified by call graph), see tools/libmatch.py.

Usage: /workspace/tools/venv/bin/python tools/libdiff.py
"""
import re
import struct
import sys
from pathlib import Path

from capstone import CS_ARCH_X86, CS_MODE_32, Cs

ROOT = Path(__file__).resolve().parent.parent
_USER_YML = (ROOT / "reccmp-user.yml").read_text(encoding="utf-8")
_ORIG_PATH = re.search(r"^\s*path:\s*(\S+)\s*$", _USER_YML, re.M)
ORIG = Path(_ORIG_PATH.group(1)) if _ORIG_PATH else ROOT.parent / "hover" / "hover.exe"
OURS = ROOT / "build" / "hover.exe"
MAP = ROOT / "build" / "hover.map"
TSV = ROOT / "tools" / "libdiff-fuzzy.tsv"

DRIFT_THRESHOLD = 6


def load_pe(path):
    d = open(path, "rb").read()
    pe = struct.unpack("<I", d[0x3C : 0x40])[0]
    nsec = struct.unpack("<H", d[pe + 6 : pe + 8])[0]
    optsz = struct.unpack("<H", d[pe + 20 : pe + 22])[0]
    imgbase = struct.unpack("<I", d[pe + 24 + 28 : pe + 24 + 32])[0]
    sec = pe + 24 + optsz
    secs = []
    for i in range(nsec):
        o = sec + i * 40
        vsz, va, rsz, roff = struct.unpack("<IIII", d[o + 8 : o + 24])
        secs.append((imgbase + va, vsz, roff))
    return d, secs


def read_va(d, secs, va, size):
    for va0, vsz, roff in secs:
        if va0 <= va < va0 + vsz:
            return d[roff + va - va0 : roff + va - va0 + size]
    return b""


def parse_map(map_path):
    syms = {}
    for line in open(map_path, errors="ignore"):
        m = re.match(
            r"\s+[0-9A-Fa-f]{4}:[0-9A-Fa-f]{8}\s+(\S+)\s+([0-9A-Fa-f]{8})\s", line
        )
        if m:
            syms.setdefault(m.group(1), int(m.group(2), 16))
    return syms


ADDR = re.compile(r"0x[0-9a-f]{5,}")
md = Cs(CS_ARCH_X86, CS_MODE_32)


def disasm_norm(buf, va):
    """Instruction list with absolute addresses stripped."""
    return [ADDR.sub("A", f"{i.mnemonic} {i.op_str}") for i in md.disasm(buf, va)]


def trim_padding(insns):
    while insns and insns[-1] == "int3 ":
        insns.pop()
    return insns


def main():
    ours_syms = parse_map(MAP)
    orig_d, orig_s = load_pe(ORIG)
    ours_d, ours_s = load_pe(OURS)

    cats = {"exact": 0, "drift": [], "different": [], "nomap": 0}
    for line in open(TSV):
        if line.startswith("#"):
            continue
        va_s, size_s, name = line.rstrip("\n").split("\t")
        va, size = int(va_s, 16), int(size_s)
        if name not in ours_syms:
            cats["nomap"] += 1
            continue
        o = trim_padding(disasm_norm(read_va(orig_d, orig_s, va, size), va))
        u = trim_padding(
            disasm_norm(read_va(ours_d, ours_s, ours_syms[name], size), ours_syms[name])
        )
        if o == u:
            cats["exact"] += 1
            continue
        ndiff = sum(1 for a, b in zip(o, u) if a != b) + abs(len(o) - len(u))
        if len(o) == len(u) and ndiff <= DRIFT_THRESHOLD:
            cats["drift"].append((va, name, ndiff))
        else:
            cats["different"].append((va, name, len(o), len(u), ndiff))

    print("exact:", cats["exact"], " nomap:", cats["nomap"])
    print("drift (same length, small diffs):", len(cats["drift"]))
    for va, name, nd in cats["drift"]:
        print(f"  0x{va:x} {name} diffs={nd}")
    print("different:", len(cats["different"]))
    for va, name, lo, lu, nd in cats["different"]:
        print(f"  0x{va:x} {name[:60]} insns {lo}/{lu} diffs={nd}")


if __name__ == "__main__":
    sys.exit(main())
