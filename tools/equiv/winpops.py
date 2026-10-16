#!/usr/bin/env python3
"""Callee-popped argument bytes (`ret n`) for every Win32 API hover.exe imports.

Win32 APIs are stdcall: the callee pops its arguments, so an emulator that
stubs an API call must know how many bytes to pop.  The undecorated import
names do not say, but Wine ships real i386 PE builds of every system DLL, and
each exported function ends in the same `ret n` the Windows original does.
This disassembles each export (following forwarders) up to its first `ret`.

    /workspace/tools/venv/bin/python tools/equiv/winpops.py > build/winpops.json
"""
import json
import sys
from pathlib import Path

import capstone
import pefile

WINE = Path("/usr/lib/i386-linux-gnu/wine/i386-windows")
_cs = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
_dlls = {}


def load(name):
    name = name.lower()
    if not name.endswith((".dll", ".drv")):
        name += ".dll"
    if name not in _dlls:
        path = WINE / name
        if not path.exists():
            _dlls[name] = None
        else:
            pe = pefile.PE(str(path))
            exports = {}
            for e in getattr(pe, "DIRECTORY_ENTRY_EXPORT", None).symbols if hasattr(pe, "DIRECTORY_ENTRY_EXPORT") else []:
                if e.name:
                    exports[e.name.decode()] = e
            iat = {}
            for imp in getattr(pe, "DIRECTORY_ENTRY_IMPORT", []):
                for t in imp.imports:
                    if t.name:
                        iat[t.address - pe.OPTIONAL_HEADER.ImageBase] = (imp.dll.decode(), t.name.decode())
            _dlls[name] = (pe, pe.get_memory_mapped_image(), exports, iat)
    return _dlls[name]


def ret_pop(image, rva, iat=None, base=0, limit=4000):
    """Pop count of the first ret reached by recursive descent from rva.
    A tail jump through the DLL's own import table is followed into the
    imported function (Wine's user32 jumps to win32u, kernel32 to kernelbase)."""
    seen, work, n = set(), [rva], 0
    while work and n < limit:
        a = work.pop(0)
        while a not in seen and 0 <= a < len(image) and n < limit:
            seen.add(a)
            n += 1
            ins = next(_cs.disasm(image[a : a + 16], a), None)
            if ins is None:
                break
            if ins.mnemonic == "ret":
                return int(ins.op_str, 16) if ins.op_str else 0
            if ins.mnemonic == "jmp" and ins.op_str.startswith("dword ptr [0x"):
                slot = int(ins.op_str[len("dword ptr ["):-1], 16) - base
                if iat and slot in iat:
                    return lookup(*iat[slot], depth=1)
                break
            if ins.mnemonic == "jmp":
                try:
                    work.append(int(ins.op_str, 16))
                except ValueError:
                    pass
                break
            if ins.mnemonic.startswith("j") or ins.mnemonic.startswith("loop"):
                try:
                    work.append(int(ins.op_str, 16))
                except ValueError:
                    pass
            if ins.mnemonic in ("int3", "hlt", "ud2"):
                break
            a += ins.size
    return None


def lookup(dll, func, depth=0):
    d = load(dll)
    if d is None or depth > 8:
        return None
    pe, image, exports, iat = d
    e = exports.get(func)
    if e is None:
        return None
    if e.forwarder:
        target = e.forwarder.decode()
        tdll, tfunc = target.rsplit(".", 1)
        return lookup(tdll, tfunc, depth + 1)
    return ret_pop(image, e.address, iat, pe.OPTIONAL_HEADER.ImageBase)


def main():
    out = {}
    for exe in sys.argv[1:] or ["/workspace/hover/hover.exe", "build/hover.exe"]:
        pe = pefile.PE(exe)
        for imp in pe.DIRECTORY_ENTRY_IMPORT:
            dll = imp.dll.decode()
            for s in imp.imports:
                if not s.name:
                    continue
                func = s.name.decode()
                key = f"{dll.upper()}::{func}"
                if key not in out:
                    out[key] = lookup(dll, func)
    json.dump(out, sys.stdout, indent=1, sort_keys=True)
    missing = [k for k, v in out.items() if v is None]
    print(f"{len(out)} imports, {len(missing)} unresolved: {missing}", file=sys.stderr)


if __name__ == "__main__":
    main()
