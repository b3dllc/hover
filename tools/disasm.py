#!/usr/bin/env python3
"""Disassemble a function of the original hover.exe.

Usage: /workspace/tools/venv/bin/python tools/disasm.py 0xVA [maxbytes]

Prints capstone Intel-syntax disassembly starting at VA until the first
int3 padding that follows a ret/jmp (i.e. the end of the function), or
maxbytes. Use this instead of `tools/compare.py --verbose`, which sizes the
original function by the *recompiled* one and truncates when ours is shorter.
"""
import struct
import sys

from capstone import CS_ARCH_X86, CS_MODE_32, Cs

ORIG = "/workspace/hover/hover.exe"


def section_map(data):
    pe = struct.unpack_from("<I", data, 0x3C)[0]
    nsec = struct.unpack_from("<H", data, pe + 6)[0]
    opt = struct.unpack_from("<H", data, pe + 20)[0]
    base = struct.unpack_from("<I", data, pe + 24 + 28)[0]
    secs = []
    for i in range(nsec):
        o = pe + 24 + opt + 40 * i
        vsz, va, rsz, raw = struct.unpack_from("<IIII", data, o + 8)
        secs.append((base + va, vsz, raw))
    return secs


def main():
    va = int(sys.argv[1], 16)
    n = int(sys.argv[2], 0) if len(sys.argv) > 2 else 0x4000
    data = open(ORIG, "rb").read()
    off = None
    for start, vsz, raw in section_map(data):
        if start <= va < start + vsz:
            off = raw + va - start
    if off is None:
        sys.exit(f"{va:#x} is not inside any section")
    md = Cs(CS_ARCH_X86, CS_MODE_32)
    prev = None
    for ins in md.disasm(data[off:off + n], va):
        if ins.mnemonic == "int3":
            if prev is not None and prev.mnemonic in ("ret", "jmp"):
                break
            continue
        print(f"{ins.address:08x}  {ins.bytes.hex():24s} {ins.mnemonic} {ins.op_str}")
        prev = ins


if __name__ == "__main__":
    main()
