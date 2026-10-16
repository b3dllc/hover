#!/usr/bin/env python3
"""Patch bytes in a PE image by virtual address (test harness use only).

    tools/pepatch.py EXE 0xVA HEXBYTES [--expect HEXBYTES]

Converts the VA to a file offset through the section table, optionally
verifies the bytes currently there, and overwrites them in place.  Used by
the differential frame test to give both binaries the same rand() seed:
`call time` (E8 rel32) is the same length as `mov eax, imm32` (B8 imm32).
"""
import struct
import sys


def main():
    exe, va = sys.argv[1], int(sys.argv[2], 16)
    new = bytes.fromhex(sys.argv[3])
    expect = bytes.fromhex(sys.argv[sys.argv.index("--expect") + 1]) if "--expect" in sys.argv else None
    data = bytearray(open(exe, "rb").read())
    pe = struct.unpack_from("<I", data, 0x3C)[0]
    nsec = struct.unpack_from("<H", data, pe + 6)[0]
    opt = struct.unpack_from("<H", data, pe + 20)[0]
    base = struct.unpack_from("<I", data, pe + 24 + 28)[0]
    rva = va - base
    for i in range(nsec):
        o = pe + 24 + opt + 40 * i
        vsz, sva, rsz, raw = struct.unpack_from("<IIII", data, o + 8)
        if sva <= rva < sva + max(vsz, rsz):
            off = raw + (rva - sva)
            cur = bytes(data[off:off + len(new)])
            if expect is not None and cur != expect:
                sys.exit("at %#x found %s, expected %s" % (va, cur.hex(), expect.hex()))
            data[off:off + len(new)] = new
            open(exe, "wb").write(data)
            print("%s: %#x (file %#x) %s -> %s" % (exe, va, off, cur.hex(), new.hex()))
            return
    sys.exit("VA %#x not in any section" % va)


main()
