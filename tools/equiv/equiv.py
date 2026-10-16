#!/usr/bin/env python3
"""Differential emulation: is each recompiled function a functional match?

Byte similarity cannot prove that a non-matching function behaves like the
original.  This runs the original function (in hover.exe) and its recompiled
twin (in build/hover.exe) side by side under Unicorn, on many identical
pseudo-random inputs, and compares everything a caller could observe:

  - every call it makes, in order: the callee (canonicalised to the
    original's address or import name), `this` for thiscall callees, and the
    stack arguments;
  - every write to non-stack memory, grouped by the calls they precede (a
    store cannot legally move across an opaque call);
  - the return value (eax, or st0 for float/double functions) and the number
    of bytes popped by `ret n`;
  - faults (null-page access, divide by zero) and escapes.

Inputs are deterministic functions of (seed, address): `this`, the stack
arguments, every global the two binaries share (reccmp's matched data), every
heap page touched (mapped lazily on first access), and every stubbed callee's
return value.  Values are drawn from a mix weighted towards what branches test:
zero, small and negative integers, pointers, floats, and the immediates the two
functions themselves compare against.  Callees are never executed (each
function is checked in isolation, assuming its callees match), except pure CRT
helpers such as __ftol and the 64-bit arithmetic routines.

    /workspace/tools/venv/bin/python tools/equiv/equiv.py [--runs N] [--all] [0xADDR ...]

Needs build/matches.json (tools/equiv/dump_matches.py) and build/winpops.json
(tools/equiv/winpops.py); tools/equiv/run.sh regenerates both.
"""
import argparse
import gc
import json
import os
from multiprocessing import TimeoutError as mp_TimeoutError
import re
import sys
import zlib
from bisect import bisect_right, insort
from pathlib import Path

import numpy as np
import pefile
from unicorn import (UC_ARCH_X86, UC_HOOK_BLOCK, UC_HOOK_CODE, UC_HOOK_INTR, UC_HOOK_MEM_READ,
                     UC_HOOK_MEM_FETCH_UNMAPPED, UC_HOOK_MEM_READ_UNMAPPED,
                     UC_HOOK_MEM_WRITE, UC_HOOK_MEM_WRITE_UNMAPPED, UC_MODE_32,
                     Uc, UcError)
from unicorn import x86_const as R

sys.path.insert(0, str(Path(__file__).resolve().parent))
sys.path.insert(0, str(Path(__file__).resolve().parent.parent))
import analysis  # noqa: E402
from analysis import UNKNOWN, FuncInfo  # noqa: E402

ROOT = Path(__file__).resolve().parent.parent.parent

# ---------------------------------------------------------------- layout
NULL_END = 0x10000
IMG_LO, IMG_HI = 0x400000, 0x500000
# pointers walked a little outside an image (an arena cursor below its
# base, say) still compare as pointers
NEAR_LO, NEAR_HI = 0x300000, 0x600000
IMPBASE = 0x600000            # fake addresses of imported functions
TIB = 0x700000
STACK_LO, STACK_HI = 0x800000, 0x900000
ENTRY_ESP = 0x8F0000
RET_MAGIC = 0x9F0000
SENT_LO, SENT_HI = 0xEE000000, 0xF0000000
UNTRANSLATED = 0xEE000000     # ours points into its image at nothing matched
EMPTYSTR = 0xEE5E0000         # an unmatched pointer to a NUL byte ("")
LOCAL = 0xEF5AFE00            # a pointer into the function's own frame
HEAP_LO, HEAP_SPAN = 0x10000000, 0x60000000
STACK_FILL = 0xCC
# The null page stays unmapped: a small value used as a pointer would make
# unrelated objects overlap there, which no real input does.
FORBIDDEN = ((0, NULL_END), (IMG_LO, 0xA00000), (SENT_LO, SENT_HI))  # GDT 0x6F0000 lies inside

EXEC_NAMES = re.compile(r"^_?_(ftol|alldiv|allmul|allrem|allshl|allshr|aulldiv|"
                        r"aullrem|aullshr|chkstk|alloca_probe|CI\w+|"
                        r"adj_f\w+|safe_f\w+)$")  # + the Pentium FDIV workarounds
WINPOP_EXTRA = {"USER32.DLL::DEFWINDOWPROCA": 16, "KERNEL32.DLL::EXITTHREAD": 4,
                "KERNEL32.DLL::FATALAPPEXITA": 8}
MAX_INSNS = 100_000
PARAM_MASKS = {}   # orig function -> per-argument masks (see _param_masks)
# (import, argument index) -> bytes the API writes through that pointer
OUT_SIZES = {
    ("USER32.DLL::GetClientRect", 1): 16, ("USER32.DLL::GetWindowRect", 1): 16,
    ("USER32.DLL::GetCursorPos", 0): 8, ("GDI32.DLL::GetTextExtentPointA", 3): 8,
    ("GDI32.DLL::GetTextMetricsA", 1): 56, ("USER32.DLL::GetUpdateRect", 1): 16,
    ("GDI32.DLL::GetClipBox", 1): 16, ("USER32.DLL::CopyRect", 0): 16,
    ("ADVAPI32.DLL::RegCreateKeyExA", 7): 4, ("ADVAPI32.DLL::RegCreateKeyExA", 8): 4,
    ("ADVAPI32.DLL::RegOpenKeyExA", 4): 4, ("ADVAPI32.DLL::RegQueryValueExA", 3): 4,
    ("ADVAPI32.DLL::RegQueryValueExA", 4): 4, ("ADVAPI32.DLL::RegQueryValueExA", 5): 4,
    ("USER32.DLL::GetDlgItemInt", 2): 4, ("USER32.DLL::SendMessageA", 2): 4,
    ("USER32.DLL::SendMessageA", 3): 4, ("USER32.DLL::SendDlgItemMessageA", 3): 4,
    ("USER32.DLL::SendDlgItemMessageA", 4): 4, ("USER32.DLL::GetWindowPlacement", 1): 44,
    ("WINMM.DLL::joyGetPos", 1): 16, ("WINMM.DLL::joyGetPosEx", 1): 52,
    ("WINMM.DLL::waveOutGetDevCapsA", 1): 52, ("WINMM.DLL::joyGetDevCapsA", 1): 404,
    ("KERNEL32.DLL::GetLocalTime", 0): 16, ("KERNEL32.DLL::GetSystemTime", 0): 16,
    ("KERNEL32.DLL::GetSystemInfo", 0): 36, ("KERNEL32.DLL::GlobalMemoryStatus", 0): 32,
    ("KERNEL32.DLL::GetStartupInfoA", 0): 68, ("WINMM.DLL::waveOutGetPosition", 1): 12,
    ("WINMM.DLL::mmioDescend", 1): 20, ("WINMM.DLL::midiOutGetDevCapsA", 1): 52,
    ("KERNEL32.DLL::ReadFile", 3): 4, ("USER32.DLL::SetRect", 0): 16,
    ("USER32.DLL::SetRectEmpty", 0): 16, ("USER32.DLL::IntersectRect", 0): 16,
    ("USER32.DLL::UnionRect", 0): 16, ("ADVAPI32.DLL::RegOpenKeyA", 2): 4,
    ("ADVAPI32.DLL::RegQueryValueA", 3): 4,
}
# (import, argument index) -> bytes the API reads through that pointer (a
# struct passed by address; in-out ones such as ClientToScreen's POINT are
# here, not in OUT_SIZES).  Any other frame pointer an import gets is
# compared up to its first NUL (a string), unless OUT_BUFS says the API only
# writes it.
IN_SIZES = {
    ("USER32.DLL::ClientToScreen", 1): 8, ("USER32.DLL::ScreenToClient", 1): 8,
    ("USER32.DLL::AdjustWindowRect", 0): 16, ("USER32.DLL::AdjustWindowRectEx", 0): 16,
    ("USER32.DLL::InvalidateRect", 1): 16, ("USER32.DLL::ValidateRect", 1): 16,
    ("USER32.DLL::FillRect", 1): 16, ("USER32.DLL::FrameRect", 1): 16,
    ("USER32.DLL::DrawTextA", 3): 16, ("GDI32.DLL::ExtTextOutA", 4): 16,
    ("USER32.DLL::OffsetRect", 0): 16, ("USER32.DLL::InflateRect", 0): 16,
    ("USER32.DLL::IntersectRect", 1): 16, ("USER32.DLL::IntersectRect", 2): 16,
    ("USER32.DLL::UnionRect", 1): 16, ("USER32.DLL::UnionRect", 2): 16,
    ("USER32.DLL::CopyRect", 1): 16, ("USER32.DLL::PtInRect", 0): 16,
    ("USER32.DLL::IsRectEmpty", 0): 16, ("USER32.DLL::EqualRect", 0): 16,
    ("USER32.DLL::EqualRect", 1): 16, ("USER32.DLL::MapWindowPoints", 2): 16,
    ("USER32.DLL::SetWindowPlacement", 1): 44, ("USER32.DLL::RegisterClassA", 0): 40,
    ("GDI32.DLL::CreatePalette", 0): 4, ("GDI32.DLL::CreateFontIndirectA", 0): 60,
}
# (import, argument index) -> bytes written, from the raw stack arguments
OUT_SIZED = {
    ("GDI32.DLL::GetPaletteEntries", 3): lambda a: 4 * a[2],
    ("GDI32.DLL::GetSystemPaletteEntries", 3): lambda a: 4 * a[2],
    ("USER32.DLL::GetWindowTextA", 1): lambda a: a[2],
    ("USER32.DLL::GetDlgItemTextA", 2): lambda a: a[3],
    ("USER32.DLL::LoadStringA", 2): lambda a: a[3],
    ("KERNEL32.DLL::GetModuleFileNameA", 1): lambda a: a[2],
    ("KERNEL32.DLL::GetCurrentDirectoryA", 1): lambda a: a[0],
    ("KERNEL32.DLL::ReadFile", 1): lambda a: a[2],
    ("WINMM.DLL::mmioRead", 1): lambda a: a[2],
}
# (import, argument index) -> bytes read, from the raw stack arguments
IN_SIZED = {
    ("GDI32.DLL::SetPaletteEntries", 3): lambda a: 4 * a[2],
    ("GDI32.DLL::AnimatePalette", 3): lambda a: 4 * a[2],
    ("GDI32.DLL::Polygon", 1): lambda a: 8 * a[2],
    ("GDI32.DLL::Polyline", 1): lambda a: 8 * a[2],
    ("KERNEL32.DLL::WriteFile", 1): lambda a: a[2],
    ("ADVAPI32.DLL::RegSetValueExA", 4): lambda a: a[5],
    ("ADVAPI32.DLL::RegSetValueA", 3): lambda a: a[4],
}
OUT_BUFS = {
    ("USER32.DLL::GetWindowTextA", 1), ("USER32.DLL::GetDlgItemTextA", 2),
    ("USER32.DLL::wsprintfA", 0), ("KERNEL32.DLL::lstrcpyA", 0),
    ("KERNEL32.DLL::GetModuleFileNameA", 1), ("KERNEL32.DLL::GetPrivateProfileStringA", 4),
    ("USER32.DLL::LoadStringA", 2), ("KERNEL32.DLL::GetCurrentDirectoryA", 1),
    ("ADVAPI32.DLL::RegQueryValueA", 2), ("USER32.DLL::SetRect", 0),
    ("USER32.DLL::SetRectEmpty", 0), ("USER32.DLL::IntersectRect", 0),
    ("USER32.DLL::UnionRect", 0), ("USER32.DLL::CopyRect", 0),
    ("KERNEL32.DLL::ReadFile", 1), ("KERNEL32.DLL::ReadFile", 3),
    ("WINMM.DLL::mmioRead", 1), ("WINMM.DLL::mmioDescend", 1),
    ("WINMM.DLL::waveOutGetPosition", 1), ("KERNEL32.DLL::GlobalMemoryStatus", 0),
    ("KERNEL32.DLL::GetSystemInfo", 0), ("KERNEL32.DLL::GetStartupInfoA", 0),
    ("GDI32.DLL::GetPaletteEntries", 3), ("GDI32.DLL::GetSystemPaletteEntries", 3),
    ("KERNEL32.DLL::FormatMessageA", 4), ("WINMM.DLL::midiOutGetDevCapsA", 1),
}
# imports whose result is a small count or size in practice: a random dword
# runs the caller's loops for billions of iterations (every run times out)
RET_CHOICES = {
    "GDI32.DLL::GetDeviceCaps": (0, 1, 2, 8, 16, 20, 24, 96, 256, 0xFFFFFFFF),
    "USER32.DLL::GetSystemMetrics": (0, 1, 2, 16, 32, 480, 640),
    "GDI32.DLL::GetPaletteEntries": (0, 1, 16, 256), "GDI32.DLL::GetSystemPaletteEntries": (0, 1, 16, 256),
    "KERNEL32.DLL::lstrlenA": (0, 1, 2, 7, 12, 31),
    "USER32.DLL::GetWindowTextA": (0, 1, 5, 12), "USER32.DLL::GetDlgItemTextA": (0, 1, 5, 12),
    "USER32.DLL::LoadStringA": (0, 1, 5, 12),
    "WINMM.DLL::waveOutGetNumDevs": (0, 1, 2), "WINMM.DLL::midiOutGetNumDevs": (0, 1, 2),
    "WINMM.DLL::joyGetNumDevs": (0, 1, 2),
}
GAME_PTD = 16      # bytes compared behind a frame pointer passed to a game function


class Ptd:
    """A pointer into the caller's frame passed to a callee, with what it
    points at: {offset: (None, byte, window)} for the bytes the function
    itself wrote there (stack fill and stub outputs are not evidence).
    `both`: compare only the leading bytes both sides wrote (a game
    callee's buffer may be a pure output, and the window may reach a
    neighbouring local)."""
    __slots__ = ("v", "buf", "both")

    def __init__(self, v, buf, both):
        self.v, self.buf, self.both = v, buf, both

    def __eq__(self, o):
        if isinstance(o, V):
            return self.v == o
        if not isinstance(o, Ptd) or self.v != o.v:
            return False
        if self.both or o.both:
            # the run of bytes from the pointer itself that both sides
            # wrote: the object's own leading fields, not a neighbouring
            # local past an unwritten gap (each compiler lays out its frame)
            keys = set()
            while len(keys) in self.buf and len(keys) in o.buf:
                keys.add(len(keys))
        else:
            keys = set(self.buf) | set(o.buf)
        for k in keys:
            x, y = self.buf.get(k), o.buf.get(k)
            if x is None or y is None or not _byte_eq(x, y):
                return False
        return True

    def __ne__(self, o):
        return not self.__eq__(o)

    __hash__ = None

    def __repr__(self):
        body = "".join(f"{e[1]:02x}" if k in self.buf else ".." for k, e in sorted(self.buf.items())[:24])
        return f"&local{{{body}}}"


def forbidden(a):
    return any(lo <= a < hi for lo, hi in FORBIDDEN)


# ---------------------------------------------------------------- images
class Image:
    def __init__(self, path):
        pe = pefile.PE(str(path), fast_load=False)
        self.base = pe.OPTIONAL_HEADER.ImageBase
        self.size = (pe.OPTIONAL_HEADER.SizeOfImage + 0xFFF) & ~0xFFF
        mem = bytearray(pe.get_memory_mapped_image())
        mem.extend(b"\0" * (self.size - len(mem)))
        self.mem = mem
        self.frozen = bytes(mem)
        self.iat = {}
        for imp in pe.DIRECTORY_ENTRY_IMPORT:
            dll = imp.dll.decode().upper()
            for s in imp.imports:
                if s.name:
                    self.iat[s.address] = f"{dll}::{s.name.decode()}"
        self.sections = [(s.Name.rstrip(b"\0").decode(), self.base + s.VirtualAddress,
                          s.Misc_VirtualSize, s.Characteristics) for s in pe.sections]
        self.relocs = set()
        for blk in getattr(pe, "DIRECTORY_ENTRY_BASERELOC", []):
            for e in blk.entries:
                if e.type == 3:
                    self.relocs.add(self.base + e.rva)
        self._sig = {}

    def writable(self, a):
        for name, va, size, ch in self.sections:
            if va <= a < va + size:
                return bool(ch & 0x80000000)
        return False

    def section(self, a):
        for name, va, size, ch in self.sections:
            if va <= a < va + size:
                return name
        return None

    def in_code(self, a):
        for name, va, size, ch in self.sections:
            if va <= a < va + size:
                return bool(ch & 0x20000000)
        return False

    def signature(self, addr):
        """Relocation-independent fingerprint of the code at addr, used to
        identify a library function one image names and the other does not."""
        if addr in self._sig:
            return self._sig[addr]
        insns = {}
        work = [addr]
        while work and len(insns) < 96:
            a = work.pop()
            while a not in insns and self.inside(a) and len(insns) < 96:
                ins = analysis.decode(self, a)
                if ins is None:
                    break
                raw = bytearray(self.read(a, ins.size))
                for k in range(ins.size - 3):
                    if a + k in self.relocs:
                        raw[k:k + 4] = b"RRRR"
                if ins.mn in ("call", "jmp") and raw[:1] in (b"\xe8", b"\xe9"):
                    raw[1:] = b"C" * (ins.size - 1)
                insns[a] = bytes(raw)
                if ins.mn in ("ret", "int3"):
                    break
                if ins.mn == "jmp":
                    if ins.ops[0][0] == "imm" and abs(ins.ops[0][1] - addr) < 0x2000:
                        work.append(ins.ops[0][1])
                    break
                if ins.mn.startswith("j") and ins.ops[0][0] == "imm":
                    work.append(ins.ops[0][1])
                a = ins.next
        sig = zlib.crc32(b"".join(insns[k] for k in sorted(insns)))
        self._sig[addr] = sig
        return sig

    def inside(self, a):
        return self.base <= a < self.base + self.size

    def read(self, a, n):
        o = a - self.base
        if o < 0 or o >= self.size:
            return b"\0" * n
        return bytes(self.mem[o:o + n])

    def u32(self, a):
        o = a - self.base
        if o < 0 or o + 4 > self.size:
            return None
        return int.from_bytes(self.mem[o:o + 4], "little")

    def cstring(self, a, limit=256):
        o = a - self.base
        if o < 0 or o >= self.size:
            return None
        end = self.mem.find(b"\0", o, o + limit)
        if end <= o:
            return None
        s = bytes(self.mem[o:end])
        if not all(32 <= c < 127 or c in (9, 10, 13) for c in s):
            return None
        return s


# ---------------------------------------------------------------- matches
class World:
    """Both images plus everything known about how they correspond."""

    def __init__(self):
        self.orig = Image(_orig_path())
        self.ours = Image(ROOT / "build" / "hover.exe")
        rows = json.loads((ROOT / "build" / "matches.json").read_text())
        pops = json.loads((ROOT / "build" / "winpops.json").read_text())
        self.winpop = {k.upper(): v for k, v in pops.items()}
        self.winpop.update(WINPOP_EXTRA)

        self.funcs = []           # matched functions (orig, recomp, osize, rsize, name)
        self.r2o_func, self.o2r_func = {}, {}
        self.rsize, self.osize = {}, {}
        self.oname, self.rname = {}, {}
        self.thunk_o, self.thunk_r = {}, {}
        ents = []
        self.data = []            # matched data: (orig, recomp, size)
        self.ostarts = set()
        for x in rows:
            o, r, t = x["orig"], x["recomp"], x["type"]
            if o is not None and x["name"]:
                self.oname.setdefault(o, x["name"])
            if o is not None and t in (1, 12):
                self.ostarts.add(o)
            if r is not None and x["name"]:
                self.rname.setdefault(r, x["name"])
            if t == 12:
                if o is not None:
                    self.thunk_o[o] = _impname(x["name"])
                if r is not None:
                    self.thunk_r[r] = _impname(x["name"])
                continue
            if o is None or r is None:
                if r is not None and x["recomp_size"]:
                    self.rsize.setdefault(r, x["recomp_size"])
                continue
            size = x["recomp_size"] or x["orig_size"] or 1
            if t == 1:
                self.r2o_func[r] = o
                self.o2r_func[o] = r
                self.osize[o] = x["orig_size"]
                self.rsize[r] = x["recomp_size"]
                if not x["library"]:
                    self.funcs.append((o, r, x["orig_size"], x["recomp_size"], x["name"]))
            if t == 2 or t is None:
                self.data.append((o, r, min(x["orig_size"] or size, x["recomp_size"] or size)))
            ents.append((r, max(size, 1), o))
        ents.sort()
        self.ent_r = ents
        self.ent_r_starts = [e[0] for e in ents]
        ents_o = sorted((o, size_o) for (r, size_o, o) in ents)
        osizes = {}
        for x in rows:
            if x["orig"] is not None and x["recomp"] is not None and x["type"] != 12:
                osizes[x["orig"]] = max(x["orig_size"] or x["recomp_size"] or 1, 1)
        self.ent_o = sorted(osizes.items())
        self.ent_o_starts = [e[0] for e in self.ent_o]
        self.data.sort()
        V.DELTAS = frozenset(o - r for o, r, _ in self.data if o is not None and r is not None)

        # mangled recomp names (thiscall detection) from the linker map
        import vc2map
        self.mangled_r = {s["addr"]: s["name"] for s in vc2map.parse_map(ROOT / "build" / "hover.map")}
        self.exec_o = {a for a, n in self.oname.items() if EXEC_NAMES.match(n or "")}
        self.exec_r = {a for a, n in self.mangled_r.items() if EXEC_NAMES.match(n or "")}
        self.exec_r |= {a for a, n in self.rname.items() if EXEC_NAMES.match(n or "")}
        self._str_cache = {}
        self.rettype = _rettypes()
        self._pop_cache = {}

    # -- translation of recomp values into the original's address space
    def _in_ent_r(self, v):
        i = bisect_right(self.ent_r_starts, v) - 1
        if i >= 0:
            r, size, o = self.ent_r[i]
            if v < r + size:
                return o + (v - r)
        return None

    def next_start(self, side, a):
        """The first known function start after a, on that side."""
        if not hasattr(self, "_starts"):
            # functions only: the original's LABEL entities (__ehhandler,
            # __Unwind funclets) sit inside the functions they belong to
            so = sorted(set(self.ostarts) | set(self.o2r_func) | set(self.thunk_o))
            sr = sorted(a for a in set(self.mangled_r) | set(self.r2o_func) | set(self.thunk_r)
                        if self.ours.in_code(a))
            self._starts = (so, sr)
        lst = self._starts[side]
        i = bisect_right(lst, a)
        return lst[i] if i < len(lst) else a + 0x100

    def entity(self, side, a):
        """(start, size) of the matched global holding a, on that side."""
        if side == 0:
            i = bisect_right(self.ent_o_starts, a) - 1
            if i >= 0 and a < self.ent_o[i][0] + self.ent_o[i][1]:
                return self.ent_o[i]
            return None
        i = bisect_right(self.ent_r_starts, a) - 1
        if i >= 0 and a < self.ent_r[i][0] + self.ent_r[i][1]:
            return self.ent_r[i][0], self.ent_r[i][1]
        return None

    def o2r_data(self, a):
        """Recomp address of the original's global byte a, if matched."""
        if not hasattr(self, "_data_o"):
            self._data_o = sorted((o, r, size) for o, r, size in self.data)
            self._data_os = [x[0] for x in self._data_o]
        i = bisect_right(self._data_os, a) - 1
        if i >= 0:
            o, r, size = self._data_o[i]
            if a < o + max(size, 4):
                return r + (a - o)
        return None

    def in_ent_o(self, v):
        i = bisect_right(self.ent_o_starts, v) - 1
        return i >= 0 and v < self.ent_o[i][0] + self.ent_o[i][1]

    def canon_orig(self, v):
        """An original-image pointer as compared: itself, except that an
        unmatched pointer to a NUL byte is any empty string."""
        if self.orig.inside(v) and (not self.in_ent_o(v) or self.in_ent_o(v - 1)) \
                and self.orig.section(v) in (".rdata", ".data") \
                and v % 4 == 0 and self.orig.u32(v) == 0 and self.orig.read(v - 1, 1) == b"\0":
            return EMPTYSTR
        return v

    def _shallow(self, d):
        """Translate one dword of a pointer table: a matched entity, or a
        pointer to a string that occurs exactly once in the original (the
        class name in a CRuntimeClass, say)."""
        t = self._in_ent_r(d)
        if t is not None or not self.ours.inside(d):
            return t
        s = self.ours.cstring(d)
        if s is not None and len(s) >= 2:
            hits = _findall(self.orig.mem, b"\0" + s + b"\0")
            if len(hits) == 1:
                return self.orig.base + hits[0] + 1
        return None

    def _table(self, img, a):
        out = []
        while len(out) < 64:
            d = img.u32(a + 4 * len(out))
            if d is None or not img.in_code(d):
                break
            out.append(d)
        return out

    def _best_table(self, v, cands):
        """Several original pointer tables fit the first slots of ours (a
        derived class shares its base's slots): score each over the whole
        table - every translatable slot must agree, and the lengths must be
        equal - and accept a unique best."""
        mine = self._table(self.ours, v)
        scored = []
        for c in cands:
            theirs = self._table(self.orig, c)
            if len(theirs) != len(mine):
                continue
            good = 0
            for d, e in zip(mine, theirs):
                t = self._in_ent_r(d)
                if t is None:
                    # an unannotated slot: the same code is still evidence
                    if self.ours.signature(d) == self.orig.signature(e):
                        good += 1
                    continue
                if t != e:
                    good = -1
                    break
                good += 1
            if good > 0:
                scored.append((good, c))
        scored.sort(reverse=True)
        if len(scored) == 1 or (len(scored) > 1 and scored[0][0] > scored[1][0]):
            return scored[0][1]
        return None

    def r2o(self, v):
        if not self.ours.inside(v):
            return v
        if v in self._str_cache:
            return self._str_cache[v]
        out = self._r2o(v)
        self._str_cache[v] = out
        return out

    def _r2o(self, v):
        t = self._in_ent_r(v)
        if t is not None and (self._in_ent_r(v - 1) is None or not self.ours.writable(v)
                              or self.ours.u32(v) != 0):
            return t
        if self.ours.in_code(v):
            return UNTRANSLATED | (v & 0xFFFFF)
        if self.ours.section(v) == ".data" and v % 4 == 0 and self.ours.u32(v) == 0 \
                and self.ours.read(v - 1, 1) == b"\0":
            return EMPTYSTR  # an unmatched "" literal (VC++ 2.1 puts strings in .data)
        s = self.ours.cstring(v)
        if s is not None and len(s) >= 2:
            hits = _findall(self.orig.mem, b"\0" + s + b"\0")
            if len(hits) == 1:
                return self.orig.base + hits[0] + 1
        # a wide (UTF-16) string literal
        raw = self.ours.read(v, 256)
        end = next((i for i in range(0, 254, 2) if raw[i:i + 2] == b"\0\0"), None)
        if end and end >= 4 and all(32 <= raw[i] < 127 and raw[i + 1] == 0 for i in range(0, end, 2)):
            hits = _findall(self.orig.mem, raw[:end + 2])
            hits = [h for h in hits if h % 2 == 0]
            if len(hits) == 1:
                return self.orig.base + hits[0]
        # plain data (a table of help IDs, say): the same bytes, once
        for n in (32, 16):
            blob = self.ours.read(v, n)
            if blob.count(0) < n // 2:
                hits = _findall(self.orig.mem, blob)
                if len(hits) == 1:
                    return self.orig.base + hits[0]
        # vtables and other pointer tables: the same dwords after translation
        # (untranslatable image pointers are wildcards, plain numbers must
        # match); eight dwords, so a vtable whose first slots point at
        # unannotated virtuals is still anchored by a later one
        n = max(1, min(8, len(self._table(self.ours, v)) or 4))
        dws = [self.ours.u32(v + 4 * k) or 0 for k in range(n)]
        tr = [self._shallow(d) for d in dws]
        if any(t is not None for t in tr):
            rx = b"".join(re.escape(t.to_bytes(4, "little")) if t is not None
                          else (b"...." if self.ours.inside(d) else re.escape(d.to_bytes(4, "little")))
                          for d, t in zip(dws, tr))
            hits = [m.start() for m in re.finditer(rx, self.orig.frozen, re.DOTALL) if m.start() % 4 == 0]
            if len(hits) == 1:
                return self.orig.base + hits[0]
            if 1 < len(hits) <= 64 and self.ours.in_code(dws[0]):
                best = self._best_table(v, [self.orig.base + h for h in hits])
                if best is not None:
                    return best
        # pointer arithmetic off a matched global (an index past its end)
        i = bisect_right(self.ent_r_starts, v) - 1
        if i >= 0 and self.ours.writable(v):
            r, size, o = self.ent_r[i]
            if v - r < 0x10000 and self.ours.writable(r):
                return o + (v - r)
        return UNTRANSLATED | (v & 0xFFFFF)

    def callee_id(self, side, t):
        """Canonical identity of a call target."""
        img = self.orig if side == 0 else self.ours
        if IMPBASE <= t < IMPBASE + 16 * len(self.impnames) and t % 16 == 0:
            return ("imp", self.impnames[(t - IMPBASE) // 16])
        if img.inside(t):
            thunks = self.thunk_o if side == 0 else self.thunk_r
            if t in thunks:
                return ("imp", thunks[t])
            if side == 0:
                if t in self.o2r_func:
                    return ("f", t)
                return ("lib", self.orig.signature(t))
            if t in self.r2o_func:
                return ("f", self.r2o_func[t])
            return ("lib", self.ours.signature(t))
        return ("ind", t)

    def is_exec(self, side, t):
        """Run this callee instead of stubbing it (pure CRT helpers).  The
        original names fewer of them, so an unnamed original function whose
        code fingerprint equals one of ours counts too."""
        if side == 1:
            return t in self.exec_r
        if t in self.exec_o:
            return True
        if t in self.o2r_func or not self.orig.inside(t):
            return False
        if not hasattr(self, "_exec_sigs"):
            self._exec_sigs = {self.ours.signature(a) for a in self.exec_r}
        return self.orig.signature(t) in self._exec_sigs

    def is_dtor(self, o):
        r = self.o2r_func.get(o)
        m = self.mangled_r.get(r) if r is not None else None
        if m:
            return m.startswith("??1")
        return "::~" in (self.oname.get(o) or "")

    def is_thiscall(self, ident):
        if ident[0] == "f":
            r = self.o2r_func.get(ident[1])
            m = self.mangled_r.get(r) if r is not None else None
            if m and "@@" in m:
                tail = m.split("@@", 1)[1]
                return len(tail) >= 3 and tail[0] in "ABEFIJMNQRUV" and tail[2] == "E"
            name = self.oname.get(ident[1]) or ""
            return "::" in name
        return ident[0] == "ind"

    def pop_of(self, side, t):
        """Callee-popped bytes for a direct target, or UNKNOWN."""
        key = (side, t)
        if key in self._pop_cache:
            return self._pop_cache[key]
        ident = self.callee_id(side, t)
        if ident[0] == "imp":
            v = self.winpop.get(ident[1].upper(), UNKNOWN)
            v = UNKNOWN if v is None else v
        else:
            img = self.orig if side == 0 else self.ours
            v = analysis.ret_pop(img, t) if img.inside(t) else None
            v = UNKNOWN if v is None else v
        self._pop_cache[key] = v
        return v

    def setup_imports(self):
        names = sorted(set(self.orig.iat.values()) | set(self.ours.iat.values()))
        self.impnames = names
        self.impaddr = {n: IMPBASE + 16 * i for i, n in enumerate(names)}


def _orig_path():
    """The original hover.exe, as configured for reccmp in reccmp-user.yml."""
    m = re.search(r"^\s*path:\s*(.+?)\s*$", (ROOT / "reccmp-user.yml").read_text(), re.M)
    p = Path(m.group(1)) if m else Path("../hover/hover.exe")
    return p if p.is_absolute() else (ROOT / p).resolve()


def _impname(n):
    # "KERNEL32.dll::lstrlenA" -> "KERNEL32.DLL::lstrlenA"
    if n and "::" in n:
        d, f = n.split("::", 1)
        return f"{d.upper()}::{f}"
    return n


def _findall(hay, needle):
    out, i = [], hay.find(needle)
    while i != -1 and len(out) < 2:
        out.append(i)
        i = hay.find(needle, i + 1)
    return out


def _param_masks(decl):
    """Per-parameter value masks from a declaration: a WORD/short/BYTE/char
    parameter is pushed as a dword whose upper bits are undefined."""
    if "(" not in decl:
        return None
    body = decl.split("(", 1)[1]
    depth, cur, parts = 0, "", []
    for ch in body:
        if ch == "(":
            depth += 1
        elif ch == ")":
            if depth == 0:
                break
            depth -= 1
        if ch == "," and depth == 0:
            parts.append(cur)
            cur = ""
        else:
            cur += ch
    else:
        return None  # the declaration continues on later lines
    parts.append(cur)
    masks = []
    for prm in parts:
        prm = prm.strip()
        if not prm or prm == "void":
            continue
        words = re.findall(r"\w+|[*&]", prm)
        if "*" in words or "&" in words:
            masks.append(0xFFFFFFFF)
        elif any(w in ("WORD", "short", "USHORT", "WCHAR", "ATOM") for w in words):
            masks.append(0xFFFF)
        elif any(w in ("BYTE", "char", "UCHAR", "bool") for w in words):
            masks.append(0xFF)
        else:
            masks.append(0xFFFFFFFF)
    return masks


def _rettypes():
    """orig address -> 'void' | 'float' | 'int' from the source declarations."""
    import vc2map
    out = {}
    for m in vc2map.scan_markers(ROOT / "src"):
        if m["kind"] in ("FUNCTION", "STUB"):
            lines = Path(m["file"]).read_text(encoding="utf-8").splitlines()
            full = " ".join(l.strip() for l in lines[m["decl_line"] - 1:m["decl_line"] + 7])
            pm = _param_masks(full)
            if pm is not None and any(x != 0xFFFFFFFF for x in pm):
                PARAM_MASKS[m["orig_addr"]] = pm
        if m["kind"] not in ("FUNCTION", "STUB", "LIBRARY", "TEMPLATE", "SYNTHETIC"):
            continue
        decl, cls, name = m["decl"], m["cls"], m["name"]
        if cls is not None and name == cls:
            out[m["orig_addr"]] = "int"          # constructors return this
            continue
        if name.startswith("~"):
            out[m["orig_addr"]] = "void"
            continue
        key = f"{cls}::{name}" if cls else name
        pre = decl.split(key, 1)[0] if key in decl else decl.split("(", 1)[0].rsplit(name, 1)[0]
        words = [w for w in re.split(r"\s+", pre.replace("*", " * ").strip()) if w and w not in (
            "virtual", "static", "inline", "__cdecl", "__stdcall", "WINAPI", "CALLBACK",
            "afx_msg", "PASCAL", "extern", "const", "__inline", "AFXAPI", "FAR", "NEAR")]
        if words == ["void"]:
            out[m["orig_addr"]] = "void"
        elif "*" not in words and any(w in ("float", "double") for w in words):
            out[m["orig_addr"]] = "float"
        else:
            out[m["orig_addr"]] = "int"
    return out


# ---------------------------------------------------------------- inputs
def _force_order(e):
    return (str(e[-1]).startswith("val:"), repr(e))


def h32(*parts):
    return zlib.crc32(repr(parts).encode()) & 0xFFFFFFFF


def gen_dwords(seed, key, n, dictv):
    rng = np.random.default_rng([seed & 0xFFFFFFFF, key & 0xFFFFFFFF, (key >> 32) & 0xFFFFFFFF])
    cls = rng.integers(0, 100, n)
    full = rng.integers(0, 1 << 32, n, dtype=np.uint64)
    small = rng.integers(1, 9, n).astype(np.uint64)
    med = rng.integers(9, 1024, n).astype(np.uint64)
    neg = ((1 << 32) - rng.integers(1, 17, n)).astype(np.uint64)
    ptr = (HEAP_LO + (rng.integers(0, HEAP_SPAN >> 4, n) << 4)).astype(np.uint64)
    flt = rng.uniform(-512.0, 512.0, n).astype(np.float32).view(np.uint32).astype(np.uint64)
    if dictv:
        dv = np.array(dictv, dtype=np.uint64)[rng.integers(0, len(dictv), n)]
    else:
        dv = small
    out = np.where(cls < 12, 0,
          np.where(cls < 28, small,
          np.where(cls < 44, dv,
          np.where(cls < 60, ptr,
          np.where(cls < 72, med,
          np.where(cls < 82, neg,
          np.where(cls < 90, flt, full))))))).astype(np.uint64)
    bad = np.zeros(n, dtype=bool)
    for lo, hi in FORBIDDEN[1:]:
        bad |= (out >= lo) & (out < hi)
    out = np.where(bad, out ^ 0x40000000, out)
    return out.astype(np.uint32)


def gen_pointer(seed, key):
    return int(HEAP_LO + (h32(seed, "ptr", key) % (HEAP_SPAN >> 4)) * 16)


def gen_forced(seed, key, kind, side=0):
    """A learned value: a valid pointer, a nonzero divisor, an index inside
    a table of n entries, or a directed value ("val:X", or "val:X:Y" for an
    original address X whose counterpart in our image is Y)."""
    if kind == "nz":
        return 1 + h32(seed, "nz", key) % 997
    if kind.startswith("idx:"):
        return h32(seed, "idx", key) % int(kind[4:])
    if kind.startswith("val:"):
        vals = kind[4:].split(":")
        return int(vals[side if len(vals) > 1 else 0], 16) & 0xFFFFFFFF
    return gen_pointer(seed, key)


# ---------------------------------------------------------------- values
class V:
    """A compared value.  Two values are equal if their raw numbers are equal
    or their canonical forms are: a number in the image's address range may be
    a pointer (compare translated) or just a number (compare raw)."""
    __slots__ = ("raw", "alt")

    def __init__(self, raw, alt):
        self.raw, self.alt = raw, alt

    DELTAS = frozenset()   # orig - recomp address of every matched global

    def __eq__(self, o):
        if not isinstance(o, V):
            return False
        if self.raw == o.raw or self.alt == o.alt:
            return True
        # a pointer walked past the end of a matched global: equal if the two
        # sit at the same offset from some matched pair
        near = NEAR_LO, NEAR_HI
        return ((near[0] <= self.raw < near[1] or near[0] <= o.raw < near[1])
                and ((self.raw - o.raw) in V.DELTAS or (o.raw - self.raw) in V.DELTAS))

    def __ne__(self, o):
        return not self.__eq__(o)

    __hash__ = None

    def __repr__(self):
        w = 2 if self.raw < 256 and self.alt < 256 else 0
        if self.raw == self.alt:
            return f"{self.raw:#0{w}x}" if w else hex(self.raw)
        return f"{self.raw:#x}~{self.alt:#x}"


# ---------------------------------------------------------------- one run
class Run:
    def __init__(self, world, side, fi, seed, dictv, rettype, ptrs=frozenset(), ptr_rets=frozenset(),
                 ptr_pcs=None, inl=None):
        self.w = world
        self.inl = inl or {}          # inlined callee start -> FuncInfo
        self.record_cmps = False
        self.insns = dict(fi.insns)
        self.callsites = dict(fi.calls)
        for sub in self.inl.values():
            self.insns.update(sub.insns)
            self.callsites.update(sub.calls)
        self.budget = MAX_INSNS
        self.ptr_pcs = ptr_pcs or {}  # load pc -> kind: what this instruction loads must be valid
        self.forced = []              # (address, kind) overridden on the fly via ptr_pcs
        # (address, key, kind): must hold gen_forced(key, kind); applied in a
        # fixed order with branch-directed values ("val:") last, so a value
        # forced onto a location also learned as a pointer wins the same
        # way in every process (set order varies with string hashing)
        self.ptrs = sorted(ptrs, key=_force_order)
        self.ptr_rets = sorted(ptr_rets, key=_force_order)  # (callee, occurrence, kind)
        self.recent = []              # last dword reads: (address, pc, ordinal)
        self.nreads = 0
        self.last_ret = None          # (callee, occurrence, value) of the last stub
        self.rets_seen = []           # every stub's (callee, occurrence, value)
        self.ptr_source = None        # where a null-page base pointer came from
        self.side = side
        self.img = world.orig if side == 0 else world.ours
        self.fi = fi
        self.seed = seed
        self.dictv = dictv
        self.rettype = rettype
        self.events = []
        self.stack_dirty = set()      # frame bytes this function wrote
        self.writes = []
        self.occ = {}
        self.blocks = set()
        self.in_exec = False
        self.stop_reason = None
        self.uc = None
        self.writer = None   # canonical address -> pc of last write (trace mode)
        self.wild = None     # (pc, address) of the first wild image access
        self.sites = []      # call-site pc of each call event (trace output)
        self.known = []      # sorted pointer-like values seen (see explained)
        self.pages = 0       # heap pages mapped so far
        self._hooks = []
        self.cmps = {}       # cmp/test pc -> (source of operand 0, operand 1 value)
        self.read_base = {}  # address -> base register value of its last load

    # -- canonicalisation
    def canon_val(self, v):
        if STACK_LO <= v < ENTRY_ESP:
            return V(LOCAL, LOCAL)
        if self.side == 1:
            return V(v, self.w.r2o(v) if self.w.ours.inside(v) else v)
        return V(v, self.w.canon_orig(v) if self.w.orig.inside(v) else v)

    def canon_addr(self, a):
        # a write address translates only when it lies inside a matched
        # global; any other image address (random pointer arithmetic) is
        # compared raw
        if self.side == 1 and self.w.ours.inside(a) and self.w.ours.writable(a):
            t = self.w._in_ent_r(a)
            return a if t is None else t
        return a

    def pointee(self, uc, p, ident, i, raw_args=()):
        """A frame pointer argument with what this function stored behind it."""
        v = self.canon_val(p)
        if ident[0] == "imp":
            key = (ident[1], i)
            if key in OUT_SIZES or key in OUT_BUFS:
                return v
            n = IN_SIZES.get(key)
            if n is None and key in IN_SIZED:
                try:
                    n = max(0, min(0x4000, IN_SIZED[key](raw_args)))
                except IndexError:
                    n = None
            if n is None:
                # a string: through its NUL
                n = 0
                while n < 260 and (p + n) in self.stack_dirty:
                    n += 1
                    if uc.mem_read(p + n - 1, 1)[0] == 0:
                        break
            both = False
        else:
            n, both = GAME_PTD, True
        n = min(n, ENTRY_ESP - p)
        raw = bytes(uc.mem_read(p, n)) if n > 0 else b""
        dirty = self.stack_dirty
        buf = {}
        for k in range(n):
            if p + k not in dirty:
                continue
            w0 = (p + k) & ~3
            win = None
            if all(w0 + j in dirty for j in range(4)) and w0 >= p and w0 + 4 <= p + n:
                dv = int.from_bytes(raw[w0 - p:w0 - p + 4], "little")
                win = ((p + k) - w0, self.canon_val(dv))
            buf[k] = (None, raw[k], win)
        return Ptd(v, buf, both)

    def drop_dead_vptr(self, uc, this):
        """A vptr stored at [this] just before calling a destructor on this
        is dead: every destructor of a class with virtuals stores its own
        vptr first.  VC++ emits the store only in a destructor it generated
        itself (rule 487), so it is not observable and not compared."""
        try:
            v = int.from_bytes(uc.mem_read(this, 4), "little")
        except UcError:
            return
        if not (self.img.inside(v) and self.img.section(v) == ".rdata"):
            return
        self.writes = [(a, n) for a, n in self.writes if not (this <= a and a + n <= this + 4)]
        # a store before earlier calls is just as dead if none of them could
        # see the object: member destructors get this+k, not this
        for i in range(len(self.events) - 1, -1, -1):
            ev = self.events[i]
            if ev[0] != "call":
                break
            if (ev[2] is not None and ev[2].raw == this) or any(isinstance(x, V) and x.raw == this for x in ev[3]):
                break
            kept = tuple(e for e in ev[4] if not (this <= e[1][0] < this + 4 and e[1][2] is not None
                                                  and self.img.section(e[1][2][1].raw) == ".rdata"))
            if len(kept) != len(ev[4]):
                self.events[i] = ev[:4] + (kept,)

    def flush(self):
        """Final contents of every byte written since the last event.

        Bytes are read back from memory (Unicorn's write-hook value is stale
        for read-modify-write instructions such as `and [mem], imm`).  A byte
        that lies in a fully written 4-byte window whose value points into the
        image also carries that pointer, canonicalised, so two pointers to the
        same global compare equal however the stores were split or aligned
        (packed structs put pointers at odd offsets).
        Entries: canonical address -> (raw address, byte, window) with window
        None or (offset of the byte in the window, V of the window)."""
        written = set()
        for a, size in self.writes:
            written.update(range(a, a + size))
        self.writes = []
        if not written:
            return ()
        uc = self.uc
        mem = {}
        for b in written:
            try:
                mem[b] = uc.mem_read(b, 1)[0]
            except UcError:
                pass
        img = self.img
        win = {}
        for b in sorted(mem):
            # prefer an aligned window whose value is an image address, then
            # any image address, then a near-image one: a pointer followed
            # by a small field must not be read one byte off
            best = None
            for k in range(4):
                w = b - k
                if not all(w + i in mem for i in range(4)):
                    continue
                v = mem[w] | mem[w + 1] << 8 | mem[w + 2] << 16 | mem[w + 3] << 24
                if img.inside(v) or STACK_LO <= v < ENTRY_ESP:
                    rank = 0 if w % 4 == 0 else 1
                elif NEAR_LO <= v < NEAR_HI:
                    rank = 2
                else:
                    continue
                if best is None or rank < best[0]:
                    best = (rank, k, v)
            if best is not None:
                win[b] = (best[1], self.canon_val(best[2]))
        delta = {}
        for b, byte in mem.items():
            delta[self.canon_addr(b)] = (b, byte, win.get(b))
        return tuple(sorted(delta.items()))

    # -- memory
    def map_lazy(self, uc, a):
        page = a & ~0xFFF
        if forbidden(page):
            return False
        self.pages += 1
        if self.pages > 8192:  # 32 MB of heap: a runaway loop, not evidence
            if self.wild is None:
                self.wild = (uc.reg_read(R.UC_X86_REG_EIP), a)
            return False
        vals = gen_dwords(self.seed, page, 1024, self.dictv)
        uc.mem_map(page, 0x1000)
        uc.mem_write(page, vals.tobytes())
        for p, key, kind in self.ptrs:
            if page - 3 <= p < page + 0x1000:
                # bytes of a forced dword may straddle the page boundary
                raw = gen_forced(self.seed, key, kind, self.side).to_bytes(4, "little")
                for i in range(4):
                    if page <= p + i < page + 0x1000:
                        uc.mem_write(p + i, raw[i:i + 1])
        return True

    def note_value(self, v):
        if v >= NULL_END and not forbidden(v):
            insort(self.known, v)

    def explained(self, a):
        """Is a heap address within reach of a pointer the run has seen?  An
        address that is not came from arithmetic on a random number - an
        index into an image array, typically - and lands at unrelated places
        on the two sides."""
        if STACK_LO <= a < STACK_HI or TIB <= a < TIB + 0x1000 or self.img.inside(a):
            return True
        i = bisect_right(self.known, a) - 1
        return i >= 0 and a - self.known[i] < 0x100000

    def check_heap(self, uc, addr, pc=None):
        if self.wild is not None or self.in_exec:
            return
        pc = uc.reg_read(R.UC_X86_REG_EIP) if pc is None else pc
        if not self.explained(addr):
            self.wild = (pc, addr)
            self.learn_index(uc, pc)
            return
        # a table access ([idx*s + table]) whose index carried it out of
        # the image altogether: the two tables sit at different addresses,
        # so the two sides land on different heap bytes
        ins = self.insns.get(pc)
        if ins is not None:
            for op in ins.ops:
                if op[0] == "mem" and op[2] is not None and self.img.inside(op[4]) \
                        and self.w.entity(self.side, op[4]) is not None:
                    self.wild = (pc, addr)
                    self.learn_index(uc, pc)
                    return

    def learn_index(self, uc, pc):
        """A table access (absolute displacement + index register) that left
        its table: trace the index to its source and force it into range."""
        ins = self.insns.get(pc)
        if ins is None or self.ptr_source is not None:
            return
        for op in ins.ops:
            if op[0] != "mem" or op[1] is not None or op[2] is None or op[2] not in _REG:
                continue
            if not self.img.inside(op[4]):
                continue
            ent = self.w.entity(self.side, op[4])
            if ent is None:
                continue
            n = max(1, (ent[0] + ent[1] - op[4]) // max(1, op[3]))
            src = self.find_source(uc, uc.reg_read(_REG[op[2]]))
            if src is not None:
                self.ptr_source = src + (f"idx:{n}",)
            return

    def on_read(self, uc, access, addr, size, value, _):
        if self.in_exec:
            return
        if not self.img.inside(addr):
            self.check_heap(uc, addr)
        if size != 4:
            return
        try:
            self.note_value(int.from_bytes(uc.mem_read(addr, 4), "little"))
        except UcError:
            pass
        pc = uc.reg_read(R.UC_X86_REG_EIP)
        kind = self.ptr_pcs.get(pc)
        if kind is not None:
            try:
                v = int.from_bytes(uc.mem_read(addr, 4), "little")
            except UcError:
                v = None
            bad = v is None or (v == 0 if kind == "nz" else (v < NULL_END or forbidden(v)))
            if bad:
                key = self.canon_addr(addr)
                uc.mem_write(addr, gen_forced(self.seed, key, kind, self.side).to_bytes(4, "little"))
                self.forced.append((addr, kind))
        self.nreads += 1
        self.recent.append((addr, pc, self.nreads))
        if len(self.recent) > 256:
            del self.recent[:128]
        ins = self.insns.get(pc)
        if ins is not None:
            for op in ins.ops:
                if op[0] == "mem" and op[1] in _REG and op[2] is None and op[1] != "esp":
                    self.read_base[addr] = uc.reg_read(_REG[op[1]])
                    break

    def find_source(self, uc, v, slack=0):
        """The most recent dword read (or stub return) that produced v.
        slack: a null-page base may be a small value plus a field offset
        (`lea edx, [eax+0x94]` then `mov eax, [edx]`), so for a fault the
        source may have produced up to `slack` less."""
        for d in ((0,) if not slack else (0, None)):
            # any stub's return, latest first (a pointer one call returned
            # may be used after later calls: GetView, then EnterCriticalSection)
            src_ret, when = None, -1
            for r in reversed(self.rets_seen):
                if r[2] == v if d == 0 else 0 < v - r[2] <= slack:
                    src_ret, when = ("ret",) + r[:2], r[3]
                    break
            for a, pc, k in reversed(self.recent):
                if k <= when:
                    break       # the stub returned it after this read
                try:
                    x = int.from_bytes(uc.mem_read(a, 4), "little")
                except UcError:
                    continue
                if x == v if d == 0 else (x < NULL_END and 0 < v - x <= slack):
                    return ("mem", a, pc)
            if src_ret is not None:
                return src_ret
        return None

    def ensure(self, uc, a, n=4):
        for p in {a & ~0xFFF, (a + n - 1) & ~0xFFF}:
            try:
                uc.mem_read(p, 1)
            except UcError:
                if not self.map_lazy(uc, p):
                    return False
        return True

    def u32(self, uc, a):
        if not self.ensure(uc, a):
            return None
        return int.from_bytes(uc.mem_read(a, 4), "little")

    # -- hooks
    def on_unmapped(self, uc, access, addr, size, value, _):
        if addr == RET_MAGIC:
            return False
        if self.map_lazy(uc, addr):
            return True
        if IMG_LO <= addr < IMG_HI and self.wild is None:
            # the image range past this binary's end (the two images differ
            # in size): a random pointer, not a fault of the code
            self.wild = (uc.reg_read(R.UC_X86_REG_EIP), addr)
        self.stop_reason = ("fault", "null" if addr < NULL_END else "forbidden")
        if addr < NULL_END:
            # which register held the bad base, and where did it come from?
            ins = self.insns.get(uc.reg_read(R.UC_X86_REG_EIP))
            if ins is not None:
                for op in ins.ops:
                    if op[0] == "mem" and op[1] is not None and op[1] in _REG:
                        v = uc.reg_read(_REG[op[1]])
                        src = self.find_source(uc, v, slack=0x1000)
                        self.ptr_source = src and src + ("ptr",)
                        break
        return False

    def on_img_access(self, uc, access, addr, size, value, _):
        self.check_img(uc, uc.reg_read(R.UC_X86_REG_EIP), addr)

    def check_img(self, uc, pc, addr):
        """A random pointer or index walking into one binary's private layout
        makes the two sides read or overwrite different things, and the run
        proves nothing.  Wild: a register-based address in the image outside
        every matched global or in its code, or an array access (absolute
        displacement plus a register) that leaves the global it names."""
        if self.in_exec or self.wild is not None:
            return
        img = self.img
        ins = self.insns.get(pc)
        if ins is None:
            return
        # a base register loaded from heap memory holds a random number
        # (an unaligned read can compose one in image range): whatever it
        # reaches in the image means different things on the two sides
        for op in ins.ops:
            if op[0] == "mem" and op[1] in _REG and op[1] != "esp":
                v = uc.reg_read(_REG[op[1]])
                if NEAR_LO <= v < NEAR_HI:
                    src = self.find_source(uc, v)
                    if src is not None and src[0] == "mem" and not img.inside(src[1]) \
                            and not (ENTRY_ESP <= src[1] < STACK_HI):
                        self.wild = (pc, addr)
                        return
        for op in ins.ops:
            if op[0] != "mem" or op[5] is not None:
                continue
            base, index, disp = op[1], op[2], op[4]
            if base is None and index is None:
                return  # a plain global reference
            if img.inside(disp):
                ent = self.w.entity(self.side, disp)
                if ent is None or ent[0] <= addr < ent[0] + ent[1]:
                    return  # indexing stays inside the global it names
                self.wild = (pc, addr)
                self.learn_index(uc, pc)
                return
        if not img.writable(addr) and img.section(addr) != ".text":
            return  # .rdata, .idata: real contents, same meaning in both
        ent = self.w.entity(self.side, addr)
        if ent is not None and img.writable(addr):
            return
        self.wild = (pc, addr)

    def on_lea(self, uc, addr, size, _):
        """`lea r, [idx*s + table]`: a pointer into a table, used later at an
        offset ([r+0x2d0]).  An index past the table's end makes it point
        into whatever global follows, which differs between the images."""
        if self.in_exec or self.wild is not None:
            return
        op = self.insns[addr].ops[1]
        _, base, index, scale, disp, seg, _sz = op
        if index not in _REG or (base is not None and base not in _REG):
            return
        ea = disp + uc.reg_read(_REG[index]) * scale
        if base is not None:
            ea += uc.reg_read(_REG[base])
        ea &= 0xFFFFFFFF
        ent = self.w.entity(self.side, disp)
        if ent is not None and not (ent[0] <= ea <= ent[0] + ent[1]):
            # a 1-based index folds its -1 into the displacement, which then
            # names the global before the table: the table starts just after
            nxt = self.w.entity(self.side, ent[0] + ent[1])
            if nxt is not None and nxt[0] - disp <= 2 * scale and nxt[0] <= ea <= nxt[0] + nxt[1]:
                return
            self.wild = (addr, ea)
            self.learn_index(uc, addr)

    def on_write(self, uc, access, addr, size, value, _):
        if STACK_LO <= addr < STACK_HI:
            if not self.in_exec:
                # stores only: argument pushes and return addresses are
                # stack traffic, not data the function put in its frame
                ins = self.insns.get(uc.reg_read(R.UC_X86_REG_EIP))
                if ins is None or ins.mn not in ("push", "call", "pushfd", "pushal"):
                    self.stack_dirty.update(range(addr, addr + size))
            return
        if TIB <= addr < TIB + 0x1000:
            return
        if not self.img.inside(addr):
            self.check_heap(uc, addr)
        if self.in_exec:
            return
        self.writes.append((addr, size))
        if self.writer is not None:
            pc = uc.reg_read(R.UC_X86_REG_EIP)
            for i in range(size):
                self.writer[self.canon_addr(addr + i)] = pc

    def on_block(self, uc, addr, size, _):
        # Unicorn's translation blocks run through join points, so credit
        # every static block that starts inside this one.
        self.in_exec = False
        self.blocks.add(addr)
        for b in self.fi.block_starts_in(addr, addr + size):
            self.blocks.add(b)

    def on_outside(self, uc, addr, size, _):
        if self.in_exec or addr == RET_MAGIC or addr in self.insns:
            return
        self.stop_reason = ("escape", self.w.callee_id(self.side, addr))
        uc.emu_stop()

    def chain(self, uc, v, depth=0):
        """Where a pointer value came from, as a root location plus the
        offsets of the loads that led from it: v = [..[[root] + o1] + o2..]."""
        src = self.find_source(uc, v)
        if src is None or src[0] != "mem":
            return None
        loc = src[1]
        bv = self.read_base.get(loc)
        stable = ENTRY_ESP <= loc < STACK_HI or self.img.inside(loc)
        if bv is None or stable or depth >= 3:
            return (loc, ())
        sub = self.chain(uc, bv, depth + 1)
        if sub is None:
            return (loc, ())
        return (sub[0], sub[1] + (((loc - bv) & 0xFFFFFFFF),))

    def on_cmp(self, uc, addr, size, _):
        """Remember where the first operand of a comparison came from and
        what it was compared against, for branch-directed inputs."""
        if self.in_exec or addr in self.cmps:
            return
        ins = self.insns.get(addr)
        if ins is None or len(ins.ops) != 2:
            return
        a, b = ins.ops
        rel = None
        if a[0] == "mem":
            loc = self.operand_addr(uc, a)
            src = ("mem", loc) if loc is not None else None
            if loc is not None and a[1] in _REG and a[2] is None and a[1] != "esp":
                bv = uc.reg_read(_REG[a[1]])
                ch = self.chain(uc, bv)
                if ch is not None:
                    rel = (ch[0], ch[1] + (((loc - bv) & 0xFFFFFFFF),))
        elif a[0] == "reg" and a[1] in _REG:
            src = self.find_source(uc, uc.reg_read(_REG[a[1]]))
        else:
            return
        if b[0] == "imm":
            other = b[1]
        elif b[0] == "reg" and b[1] in _REG:
            other = uc.reg_read(_REG[b[1]]) if b != a else None
        elif b[0] == "mem":
            loc = self.operand_addr(uc, b)
            other = self.u32(uc, loc) if loc is not None else None
        else:
            return
        if src is not None:
            self.cmps[addr] = (src, other, ins.mn, rel)

    def operand_addr(self, uc, op):
        _, base, index, scale, disp, seg, _sz = op
        if seg is not None:
            return None
        a = disp
        if base:
            if base not in _REG:
                return None
            a += uc.reg_read(_REG[base])
        if index:
            if index not in _REG:
                return None
            a += uc.reg_read(_REG[index]) * scale
        a &= 0xFFFFFFFF
        if STACK_LO <= a < ENTRY_ESP:
            return None  # a local: no input to force
        return a

    def on_intr(self, uc, intno, _):
        self.stop_reason = ("intr", intno)
        if intno == 0:
            # divide by zero: find where the zero divisor came from
            ins = self.insns.get(uc.reg_read(R.UC_X86_REG_EIP))
            if ins is not None and ins.mn in ("div", "idiv") and ins.ops and ins.ops[0][0] == "reg" \
                    and ins.ops[0][1] in _REG:
                v = uc.reg_read(_REG[ins.ops[0][1]])
                src = self.find_source(uc, v)
                self.ptr_source = src and src + ("nz",)
            elif ins is not None and ins.mn in ("div", "idiv") and ins.ops and ins.ops[0][0] == "mem":
                op = ins.ops[0]
                a = op[4] + (uc.reg_read(_REG[op[1]]) if op[1] in _REG else 0) \
                    + (uc.reg_read(_REG[op[2]]) * op[3] if op[2] in _REG else 0)
                self.ptr_source = ("mem", a & 0xFFFFFFFF, uc.reg_read(R.UC_X86_REG_EIP), "nz")
        uc.emu_stop()

    def operand_target(self, uc, ins):
        op = ins.ops[0]
        if op[0] == "imm":
            return op[1]
        if op[0] == "reg":
            return uc.reg_read(_REG[op[1]])
        _, base, index, scale, disp, seg, _sz = op
        a = disp
        if base:
            a += uc.reg_read(_REG[base])
        if index:
            a += uc.reg_read(_REG[index]) * scale
        a &= 0xFFFFFFFF
        if self.img.inside(a):
            if a in self.img.iat:
                return self.w.impaddr[self.img.iat[a]]
            # the stub reads the slot itself, past the memory hooks
            self.check_img(uc, ins.addr, a)
            return self.img.u32(a)
        if a < NULL_END:
            # a slot read through a (near-)null pointer: the same fault and
            # pointer learning as a load would get
            if base and base in _REG:
                src = self.find_source(uc, uc.reg_read(_REG[base]), slack=0x1000)
                self.ptr_source = src and src + ("ptr",)
            return None
        self.check_heap(uc, a, ins.addr)
        return self.u32(uc, a)

    def on_call(self, uc, addr, size, _):
        if self.in_exec:
            return
        ins = self.insns[addr]
        t = self.operand_target(uc, ins)
        if t is not None and t in self.inl:
            return  # runs inline (call or funclet tail jump); its own call sites are hooked
        if t is not None and ins.ops[0][0] != "imm" and self.img.inside(t) and self.wild is None:
            # an indirect call into the image at no known function start is
            # a random number used as a code pointer (an unaligned heap read
            # can compose one): it names unrelated code in the two images
            starts = self.w.ostarts if self.side == 0 else self.w.mangled_r
            if t not in starts and t not in (self.w.o2r_func if self.side == 0 else self.w.r2o_func) \
                    and t not in (self.w.thunk_o if self.side == 0 else self.w.thunk_r):
                self.wild = (addr, t)
        if t is None:
            self.stop_reason = ("fault", "null")
            uc.emu_stop()
            return
        if self.w.is_exec(self.side, t) and ins.kind == "call":
            self.in_exec = True
            return
        esp = uc.reg_read(R.UC_X86_REG_ESP)
        ident = self.w.callee_id(self.side, t)
        if ins.kind == "call":
            site = self.callsites[ins.next]
            pop = site.pop
            nargs = pop // 4 if pop else (site.arity or 0)
            argbase = esp
        else:  # tail jump: the return address is already on the stack
            pop = self.w.pop_of(self.side, t)
            pop = 0 if pop is UNKNOWN else pop
            nargs = pop // 4
            argbase = esp + 4
        args = []
        raw_args = [self.u32(uc, argbase + 4 * k) or 0 for k in range(nargs)]
        masks = PARAM_MASKS.get(ident[1]) if ident[0] == "f" else None
        for i in range(nargs):
            v = self.u32(uc, argbase + 4 * i)
            if v is not None and masks and i < len(masks):
                v &= masks[i]
            if v is not None and STACK_LO <= v < ENTRY_ESP - 4 and v >= esp:
                args.append(self.pointee(uc, v, ident, i, raw_args))
                continue
            args.append(self.canon_val(v) if v is not None else None)
        if ident[0] == "ind":
            # a virtual call reads its slot through the vptr stored at [ecx];
            # anything else (function tables, callback structs) is a plain
            # function pointer, where ecx is scratch
            op = ins.ops[0]
            thiscall = (op[0] == "mem" and op[1] is not None and op[2] is None
                        and self.u32(uc, uc.reg_read(R.UC_X86_REG_ECX)) == uc.reg_read(_REG[op[1]]))
        else:
            thiscall = self.w.is_thiscall(ident)
        ecx = self.canon_val(uc.reg_read(R.UC_X86_REG_ECX)) if thiscall else None
        if thiscall and ident[0] == "f" and self.w.is_dtor(ident[1]):
            self.drop_dead_vptr(uc, uc.reg_read(R.UC_X86_REG_ECX))
        self.events.append(("call", ident, ecx, tuple(args), self.flush()))
        self.sites.append(addr)
        # out-parameters: a pointer into the caller's frame gets a value
        # written through it (one dword, or a known struct's size), the
        # same on both sides - otherwise the caller reads stack fill
        for i in range(nargs):
            p = self.u32(uc, argbase + 4 * i)
            if p is not None and STACK_LO <= p < ENTRY_ESP - 4:
                if ident[0] == "imp":
                    # an API writes only through its documented outputs; an
                    # input buffer (RegSetValueEx's data) must stay as it is
                    size = OUT_SIZES.get((ident[1], i))
                    if size is None and (ident[1], i) in OUT_SIZED:
                        try:
                            size = max(0, min(0x4000, OUT_SIZED[(ident[1], i)](raw_args))) & ~3
                        except IndexError:
                            size = None
                        if size:
                            # bulk data (palettes, file contents): uniform
                            # bytes, not the zero-heavy mix of scalar inputs
                            rng = np.random.default_rng(h32(self.seed, "outbuf", ident, self.occ.get(ident, 0), i))
                            uc.mem_write(p, rng.integers(0, 256, size, dtype=np.uint8).tobytes())
                            continue
                    if not size:
                        continue
                else:
                    size = 4
                vals = gen_dwords(self.seed, h32("out", ident, self.occ.get(ident, 0), i),
                                  size // 4, self.dictv)
                uc.mem_write(p, vals.tobytes())
        n = self.occ.get(ident, 0)
        self.occ[ident] = n + 1
        rv = gen_dwords(self.seed, h32("ret", ident, n), 2, self.dictv)
        eax = int(rv[0])
        if ident[0] == "imp" and ident[1] in RET_CHOICES:
            ch = RET_CHOICES[ident[1]]
            eax = ch[h32(self.seed, "retc", ident, n) % len(ch)]
        for (rid, rn, kind) in self.ptr_rets:
            if rid == ident and rn == n:
                eax = gen_forced(self.seed, h32("ret", ident, n), kind, self.side)
        self.last_ret = (ident, n, eax)
        self.rets_seen.append(self.last_ret + (self.nreads,))
        self.note_value(eax)
        self.note_value(int(rv[1]))
        uc.reg_write(R.UC_X86_REG_EAX, eax)
        uc.reg_write(R.UC_X86_REG_EDX, int(rv[1]))
        if ins.kind == "call":
            uc.reg_write(R.UC_X86_REG_ESP, esp + pop)
            uc.reg_write(R.UC_X86_REG_EIP, ins.next)
        else:
            back = self.u32(uc, esp)
            uc.reg_write(R.UC_X86_REG_ESP, esp + 4 + pop)
            uc.reg_write(R.UC_X86_REG_EIP, back)

    def _hook(self, uc, *args, **kw):
        self._hooks.append(uc.hook_add(*args, **kw))

    def release(self):
        """Drop the hooks: they hold this Run, which holds the Uc, and the
        cycle would keep every emulator instance alive until memory ran out."""
        if self.uc is not None:
            for h in self._hooks:
                try:
                    self.uc.hook_del(h)
                except UcError:
                    pass
            self._hooks = []

    # -- driver
    def run(self, entry_regs, entry_stack, globals_fill):
        uc = Uc(UC_ARCH_X86, UC_MODE_32)
        self.uc = uc
        img = self.img
        uc.mem_map(img.base, img.size)
        uc.mem_write(img.base, bytes(img.mem))
        for slot, name in img.iat.items():
            uc.mem_write(slot, self.w.impaddr[name].to_bytes(4, "little"))
        for a, data in globals_fill:
            uc.mem_write(a, data)
        uc.mem_map(STACK_LO, STACK_HI - STACK_LO)
        uc.mem_write(STACK_LO, bytes([STACK_FILL]) * (STACK_HI - STACK_LO))
        uc.mem_map(TIB, 0x1000)
        uc.mem_write(TIB, (0xFFFFFFFF).to_bytes(4, "little") + (STACK_HI).to_bytes(4, "little")
                     + (STACK_LO).to_bytes(4, "little") + b"\0" * 12 + TIB.to_bytes(4, "little"))
        _set_fs(uc, TIB)
        esp = ENTRY_ESP
        uc.mem_write(esp, RET_MAGIC.to_bytes(4, "little") + entry_stack)
        for reg, v in entry_regs.items():
            uc.reg_write(reg, v)
            self.note_value(v)
        for i in range(0, len(entry_stack), 4):
            self.note_value(int.from_bytes(entry_stack[i:i + 4], "little"))
        for p, key, kind in self.ptrs:
            if kind == "ptr":
                self.note_value(gen_forced(self.seed, key, kind, self.side))
        uc.reg_write(R.UC_X86_REG_ESP, esp)
        uc.reg_write(R.UC_X86_REG_FPCW, 0x027F)

        self._hook(uc, UC_HOOK_MEM_READ_UNMAPPED | UC_HOOK_MEM_WRITE_UNMAPPED | UC_HOOK_MEM_FETCH_UNMAPPED,
                    self.on_unmapped)
        for p, key, kind in self.ptrs:
            if img.inside(p) or ENTRY_ESP <= p < STACK_HI:
                uc.mem_write(p, gen_forced(self.seed, key, kind, self.side).to_bytes(4, "little"))
        self._hook(uc, UC_HOOK_MEM_READ, self.on_read, begin=NULL_END, end=IMG_LO - 1)
        self._hook(uc, UC_HOOK_MEM_READ, self.on_read, begin=ENTRY_ESP + 4, end=ENTRY_ESP + 0x44)
        self._hook(uc, UC_HOOK_MEM_READ, self.on_read, begin=0xA00000, end=SENT_LO - 1)
        self._hook(uc, UC_HOOK_MEM_READ, self.on_read, begin=SENT_HI, end=0xFFFFFFFF)
        self._hook(uc, UC_HOOK_MEM_READ, self.on_read, begin=img.base, end=img.base + img.size - 1)
        self._hook(uc, UC_HOOK_MEM_WRITE, self.on_write)
        self._hook(uc, UC_HOOK_MEM_READ | UC_HOOK_MEM_WRITE, self.on_img_access,
                    begin=img.base, end=img.base + img.size - 1)
        self._hook(uc, UC_HOOK_BLOCK, self.on_block, begin=self.fi.start, end=self.fi.end - 1)
        self._hook(uc, UC_HOOK_CODE, self.on_outside, begin=0, end=self.fi.start - 1)
        self._hook(uc, UC_HOOK_CODE, self.on_outside, begin=self.fi.end, end=0xFFFFFFFF)
        self._hook(uc, UC_HOOK_INTR, self.on_intr)
        for a, ins in self.insns.items():
            if ins.kind == "call" or (ins.kind == "tail"):
                self._hook(uc, UC_HOOK_CODE, self.on_call, begin=a, end=a)
            elif ins.mn == "lea" and len(ins.ops) == 2 and ins.ops[1][0] == "mem" \
                    and ins.ops[1][2] is not None and img.inside(ins.ops[1][4]):
                self._hook(uc, UC_HOOK_CODE, self.on_lea, begin=a, end=a)
            elif self.record_cmps and ins.mn in ("cmp", "test") and a in self.fi.insns:
                self._hook(uc, UC_HOOK_CODE, self.on_cmp, begin=a, end=a)
        try:
            uc.emu_start(self.fi.start, RET_MAGIC, count=self.budget)
        except UcError as e:
            if self.stop_reason is None and uc.reg_read(R.UC_X86_REG_EIP) != RET_MAGIC:
                self.stop_reason = ("ucerror", str(e))
        eip = uc.reg_read(R.UC_X86_REG_EIP)
        if self.stop_reason is not None:
            self.events.append(self.stop_reason + (self.flush(),))
        elif eip == RET_MAGIC:
            ret = None
            if self.rettype == "int":
                ret = self.canon_val(uc.reg_read(R.UC_X86_REG_EAX))
            elif self.rettype == "float":
                ret = _st0(uc)
            popped = uc.reg_read(R.UC_X86_REG_ESP) - esp - 4
            # callee-saved registers must come back as they went in
            lost = tuple(n for n, r in (("ebx", R.UC_X86_REG_EBX), ("esi", R.UC_X86_REG_ESI),
                                        ("edi", R.UC_X86_REG_EDI), ("ebp", R.UC_X86_REG_EBP))
                         if uc.reg_read(r) != entry_regs[r])
            if lost and self.wild is None:
                # compiled code restores what it saved unless an index ran
                # off a stack array into the saved registers: undefined
                # behaviour on that input, so the run is no evidence
                self.wild = (eip, "clobbers " + " ".join(lost))
            self.events.append(("ret", ret, popped, self.flush()))
        else:
            self.events.append(("timeout",))
        self.release()
        return self.events


GDT = 0x6F0000


def _gdt_entry(base, limit, access, flags):
    import struct
    d = (limit & 0xFFFF) | ((base & 0xFFFFFF) << 16) | (access << 40) \
        | (((limit >> 16) & 0xF) << 48) | (flags << 52) | (((base >> 24) & 0xFF) << 56)
    return struct.pack("<Q", d)


def _set_fs(uc, base):
    """Point FS at the fake TIB through a GDT data descriptor (Unicorn ignores
    FS_BASE writes in 32-bit mode).  Once a GDT exists the other data
    segments need flat 32-bit descriptors too, or SS falls back to 16 bits."""
    data = 0x80 | 0x10 | 0x02           # present, data, writable, ring 0
    uc.mem_map(GDT, 0x1000)
    uc.mem_write(GDT + 8, _gdt_entry(base, 0xFFF, data, 0x4))
    uc.mem_write(GDT + 16, _gdt_entry(0, 0xFFFFF, data, 0xC))
    uc.reg_write(R.UC_X86_REG_GDTR, (0, GDT, 0xFF, 0))
    for seg in (R.UC_X86_REG_SS, R.UC_X86_REG_DS, R.UC_X86_REG_ES, R.UC_X86_REG_GS):
        uc.reg_write(seg, 2 << 3)
    uc.reg_write(R.UC_X86_REG_FS, 1 << 3)


_REG = {n: getattr(R, f"UC_X86_REG_{n.upper()}") for n in
        ("eax", "ebx", "ecx", "edx", "esi", "edi", "ebp", "esp", "ax", "bx", "cx", "dx", "si", "di")}


def _st0(uc):
    sw = uc.reg_read(R.UC_X86_REG_FPSW)
    top = (sw >> 11) & 7
    v = uc.reg_read(R.UC_X86_REG_FP0 + 0)  # FPn registers are ST-relative in unicorn
    return v if not isinstance(v, tuple) else tuple(v)


# ---------------------------------------------------------------- per function
class Pair:
    def __init__(self, world, o, r, osize, rsize, name):
        self.w = world
        self.o, self.r, self.name = o, r, name
        self.fo = FuncInfo(world.orig, o, osize)
        self.fr = FuncInfo(world.ours, r, rsize)
        for side, fi in ((0, self.fo), (1, self.fr)):
            fi.solve_stack(lambda s, side=side: self._site_pop(side, s))
            for s in fi.calls.values():
                s.arity = _cleanup_arity(fi, s) if not s.pop else s.pop // 4
        self.rettype = world.rettype.get(o, "int")
        self.ptr_pcs = ({}, {})
        # small callees neither side has a match for (per-file copies of
        # inline functions, compiler-generated destructors): run them inline
        # on both sides instead of stubbing, so what they do is compared
        self.inl = (self._inlinable(0, self.fo), self._inlinable(1, self.fr))
        # a matched callee only one side calls was expanded inline by the
        # other side's compiler (hover.exe's /Ob2 modules, rule 21): run it
        # inline where it is called, so both expansions are compared.  The
        # call sets include calls inside callees already run inline, and the
        # rule repeats until nothing changes (nested inline expansions).
        def calls_of(side):
            fis = [self.fo if side == 0 else self.fr] + list(self.inl[side].values())
            out = []
            for fi in fis:
                out += [s.insn.ops[0][1] for s in fi.calls.values() if s.insn.ops[0][0] == "imm"]
            return out
        onesided = (set(), set())
        nested = ({}, {})   # one-sided expansion -> the small callees it brought in
        for _round in range(8):
            ids = [{self.w.callee_id(side, t) for t in calls_of(side)} for side in (0, 1)]
            changed = False
            for side in (0, 1):
                # withdraw a one-sided expansion once the other side is seen
                # calling the same function after all
                for t in list(onesided[side]):
                    if self.w.callee_id(side, t) in ids[1 - side]:
                        del self.inl[side][t]
                        onesided[side].discard(t)
                        for k in nested[side].pop(t, ()):
                            self.inl[side].pop(k, None)
                        changed = True
                img = self.w.orig if side == 0 else self.w.ours
                for t in calls_of(side):
                    ident = self.w.callee_id(side, t)
                    if ident[0] != "f" or ident in ids[1 - side] or t in self.inl[side]:
                        continue
                    size = self.w.osize.get(t) if side == 0 else self.w.rsize.get(t)
                    if not size or size > 0x400:
                        continue
                    sub = FuncInfo(img, t, size)
                    sub.solve_stack(lambda cs, side=side, sub=sub: self._site_pop_any(side, cs, sub))
                    for cs in sub.calls.values():
                        cs.arity = _cleanup_arity(sub, cs) if not cs.pop else cs.pop // 4
                    self.inl[side][t] = sub
                    onesided[side].add(t)
                    # its own small unmatched callees and unwind-funclet tail
                    # jumps run inline too, as they would in the other
                    # side's expansion (~CObject reached from a destructor)
                    before = set(self.inl[side])
                    self._inlinable(side, sub, 1, self.inl[side])
                    nested[side][t] = set(self.inl[side]) - before
                    changed = True
            if not changed:
                break
        imms = set()
        for fi in (self.fo, self.fr):
            for ins in fi.insns.values():
                for op in ins.ops:
                    if op[0] == "imm" and not (IMG_LO <= op[1] < IMG_HI):
                        imms.update({op[1], (op[1] + 1) & 0xFFFFFFFF, (op[1] - 1) & 0xFFFFFFFF})
                    elif op[0] == "mem" and op[4] < 0x10000:
                        pass
        self.dictv = sorted(imms)[:256]

    def _inlinable(self, side, fi, depth=0, found=None):
        found = {} if found is None else found
        img = self.w.orig if side == 0 else self.w.ours
        targets = [s.insn.ops[0][1] for s in fi.calls.values() if s.insn.ops[0][0] == "imm"]
        targets += [t for t in fi.tailjumps.values() if t is not None]  # unwind funclets
        for t in targets:
            if t in found or not img.inside(t) or self.w.is_exec(side, t):
                continue
            ident = self.w.callee_id(side, t)
            if ident[0] not in ("lib", "f"):
                continue
            # an unnamed callee ends at the next known function at the latest
            sub = FuncInfo(img, t, min(0x100, self.w.next_start(side, t) - t))
            if not any(i.kind == "ret" for i in sub.insns.values()):
                continue
            if ident[0] == "f":
                # a matched callee runs inline only if it is a small leaf (an
                # inline accessor or constructor one compiler expanded and
                # the other called): its effects are then compared as the
                # caller's on both sides, whichever way each side compiled it
                span = max(a + i.size for a, i in sub.insns.items()) - t
                if sub.calls or sub.tailjumps or span > 0x40:
                    continue
            sub.end = max(a + i.size for a, i in sub.insns.items())
            sub.solve_stack(lambda c, side=side: self._site_pop_any(side, c, sub))
            for c in sub.calls.values():
                c.arity = _cleanup_arity(sub, c) if not c.pop else c.pop // 4
            found[t] = sub
            if depth < 2:
                self._inlinable(side, sub, depth + 1, found)
        return found

    def _site_pop_any(self, side, s, fi):
        op = s.insn.ops[0]
        img = self.w.orig if side == 0 else self.w.ours
        if op[0] == "imm":
            return self.w.pop_of(side, op[1])
        if op[0] == "mem" and op[1] is None and op[2] is None and op[4] in img.iat:
            v = self.w.winpop.get(img.iat[op[4]].upper())
            return UNKNOWN if v is None else v
        return UNKNOWN

    def _site_pop(self, side, s):
        ins = s.insn
        op = ins.ops[0]
        img = self.w.orig if side == 0 else self.w.ours
        if op[0] == "imm":
            exec_set = self.w.exec_o if side == 0 else self.w.exec_r
            if op[1] in exec_set and "chkstk" in (self.w.oname.get(op[1]) if side == 0 else self.w.mangled_r.get(op[1], "")) :
                return -_last_mov_eax(self.fo if side == 0 else self.fr, ins)
            return self.w.pop_of(side, op[1])
        if op[0] == "mem" and op[1] is None and op[2] is None and op[4] in img.iat:
            v = self.w.winpop.get(img.iat[op[4]].upper())
            return UNKNOWN if v is None else v
        if op[0] == "reg":
            slot = _reg_iat(self.fo if side == 0 else self.fr, op[1], ins.addr)
            if slot in img.iat:
                v = self.w.winpop.get(img.iat[slot].upper())
                return UNKNOWN if v is None else v
        return UNKNOWN

    def initial_value(self, seed, loc, forced, stack):
        """The dword a heap or argument location holds at the start of a run
        of this seed: a learned value if one is forced there, else what the
        generator puts there."""
        for p, key, kind in forced:
            if p == loc and not kind.startswith("val:"):
                return gen_forced(seed, key, kind)
        if ENTRY_ESP + 4 <= loc < ENTRY_ESP + 4 + len(stack):
            o = loc - ENTRY_ESP - 4
            return int.from_bytes(stack[o:o + 4], "little")
        if self.w.orig.inside(loc):
            # a matched global holds what globals_fill puts there (seeded
            # by the entity's own address); anything else its image bytes
            for o, r, size in self.w.data:
                if o <= loc < o + min(size, 0x10000) - 3:
                    vals = gen_dwords(seed, o, (min(size, 0x10000) + 3) // 4 + 1, self.dictv).tobytes()
                    return int.from_bytes(vals[loc - o:loc - o + 4], "little")
            return self.w.orig.u32(loc)
        if forbidden(loc & ~0xFFF) or forbidden((loc + 3) & ~0xFFF):
            return None
        out = 0
        for i in range(4):
            b = loc + i
            for p, key, kind in forced:
                if p <= b < p + 4:
                    byte = (gen_forced(seed, key, kind) >> (8 * (b - p))) & 0xFF
                    break
            else:
                vals = gen_dwords(seed, b & ~0xFFF, 1024, self.dictv)
                byte = (int(vals[(b & 0xFFF) // 4]) >> (8 * (b & 3))) & 0xFF
            out |= byte << (8 * i)
        return out

    def globals_fill(self, seed):
        fo, fr = [], []
        for o, r, size in self.w.data:
            size = min(size, 0x10000)
            vals = gen_dwords(seed, o, (size + 3) // 4, self.dictv).tobytes()[:size]
            fo.append((o, vals))
            fr.append((r, vals))
        return fo, fr

    def run_seed(self, seed, writers=None, extra=None, base=None):
        """Run both sides on one seed.  A run that stops on a null-page access
        is retried with the value's source (a heap/global dword, a stub's
        return, a stack argument) forced to a valid pointer on both sides:
        the input generator learns which locations are pointers."""
        stack = gen_dwords(seed, h32("args"), 16, self.dictv).tobytes()
        regs = gen_dwords(seed, h32("regs"), 6, self.dictv)
        this = int(HEAP_LO + (h32(seed, "this") % (HEAP_SPAN >> 4)) * 16)
        entry = {R.UC_X86_REG_ECX: this, R.UC_X86_REG_EAX: int(regs[0]), R.UC_X86_REG_EDX: int(regs[1]),
                 R.UC_X86_REG_EBX: int(regs[2]), R.UC_X86_REG_ESI: int(regs[3]),
                 R.UC_X86_REG_EDI: int(regs[4]), R.UC_X86_REG_EBP: int(regs[5])}
        go, gr = self.globals_fill(seed)
        # image addresses are per side (both binaries load at 0x400000);
        # heap and stack addresses are common
        # base: the learned inputs a previous run of this seed settled on
        ptrs = (set(base[0][0]), set(base[0][1])) if base else (set(), set())
        rets = set(base[1]) if base else set()
        rels = []
        for e in extra or ():
            if e[0] == "mem":
                ptrs[0].add((e[1], e[3], e[4]))
                ptrs[1].add((e[2], e[3], e[4]))
            elif e[0] == "rel":
                rels.append(e)
            else:
                rets.add((e[1], e[2], e[4]))
        self.unsettled = False
        for attempt in range(16):
            for _, loc, offs, _x, kind in rels:
                v = self.initial_value(seed, loc, ptrs[0], stack)
                for off in offs[:-1]:
                    if v is None:
                        break
                    v = self.initial_value(seed, (v + off) & 0xFFFFFFFF, ptrs[0], stack)
                if v is not None:
                    a = (v + offs[-1]) & 0xFFFFFFFF
                    for side in (0, 1):
                        ptrs[side].add((a, a, kind))
            ro = Run(self.w, 0, self.fo, seed, self.dictv, self.rettype, frozenset(ptrs[0]), frozenset(rets),
                     self.ptr_pcs[0], self.inl[0])
            rr = Run(self.w, 1, self.fr, seed, self.dictv, self.rettype, frozenset(ptrs[1]), frozenset(rets),
                     self.ptr_pcs[1], self.inl[1])
            ro.record_cmps = True
            ro.budget = rr.budget = getattr(self, "budget", MAX_INSNS)
            wr = None
            if writers is not None:
                ro.writer, rr.writer = {}, {}
                wr = [ro.writer, rr.writer]
            eo = ro.run(entry, stack, go)
            er = rr.run(entry, stack, gr)
            if eo[-1][0] == "timeout" or er[-1][0] == "timeout":
                break  # a run that hits the budget teaches nothing more
            new = False
            for run in (ro, rr):
                learned = [(a, k) for a, k in run.forced]
                src = run.ptr_source
                if src is not None and src[0] == "mem":
                    learned.append((src[1], src[-1]))
                    # (forcing every value a load instruction produces, by pc,
                    # was tried and abandoned: a forced `next` pointer is
                    # never null, so list walks never end)
                elif src is not None and src[0] == "ret" and (src[1], src[2], src[-1]) not in rets:
                    rets.add((src[1], src[2], src[-1]))
                    new = True
                for a, kind in learned:
                    if IMG_LO <= a < IMG_HI:
                        if run.side == 0:
                            pair = (a, self.w.o2r_data(a))
                        else:
                            t = self.w._in_ent_r(a)
                            pair = (t, a if t is not None else None)
                        if None in pair:
                            continue  # an unmatched global: cannot force both copies
                    else:
                        pair = (a, a)
                    key = pair[0] if pair[0] is not None else pair[1]
                    for side in (0, 1):
                        if pair[side] is not None and (pair[side], key, kind) not in ptrs[side]:
                            ptrs[side].add((pair[side], key, kind))
                            new = True
            if not new:
                break
            # (a wild access that taught an index range is retried too)
        else:
            # still learning when the attempts ran out: the two sides may
            # have seen different inputs, so this run proves nothing
            self.unsettled = True
        if writers is not None:
            writers.extend(wr)
        self.last_wild = (ro.wild, rr.wild)
        self.last_sites = (ro.sites, rr.sites)
        self.last_cmps = ro.cmps
        self.last_forced = (ptrs, rets)
        return eo, er, ro.blocks, rr.blocks


def _directed(pair, covered, cmp_log):
    """Inputs aimed at uncovered blocks: for each conditional jump from a
    covered block to an uncovered one, force the compared value's source
    to the constant it is compared against (and one either side).
    Yields (seed, extra) for run_seed."""
    fo = pair.fo
    preds = {}
    for a, ins in fo.insns.items():
        if ins.kind == "jcc":
            for t in ins.succ:
                preds.setdefault(t, []).append(a)
    order = sorted(fo.insns)
    seen = set()
    for blk in sorted(fo.blocks - covered):
        for j in preds.get(blk, ()):
            # the comparison feeding this jump: the nearest cmp/test before it
            i = order.index(j)
            flag = None
            for k in range(i - 1, max(i - 6, -1), -1):
                ins = fo.insns[order[k]]
                if ins.mn in ("cmp", "test"):
                    flag = order[k]
                    break
                if ins.kind is not None or ins.mn.startswith(("add", "sub", "and", "or", "xor", "inc", "dec")):
                    break
            if flag is None or flag not in cmp_log:
                continue
            seed, (src, other, mn, rel) = cmp_log[flag]
            if mn == "test":
                vals = (0, 1, 0xFFFFFFFF)
            elif other is None:
                continue
            else:
                vals = (other, other + 1, other - 1)
            for v in vals:
                e = _force(pair, src, rel, v)
                if e is None or (seed, e) in seen:
                    continue
                seen.add((seed, e))
                yield seed, [e]


def _force(pair, src, rel, v):
    """The run_seed `extra` entry that makes a comparison's traced input v."""
    v &= 0xFFFFFFFF
    kind = f"val:{v:x}"
    if IMG_LO <= v < IMG_HI:
        # an address of the original (a function compared against a stored
        # callback, a global): our image holds its counterpart
        r = pair.w.o2r_func.get(v)
        if r is None:
            r = pair.w.o2r_data(v)
        if r is not None:
            kind = f"val:{v:x}:{r:x}"
    if rel is not None and not (IMG_LO <= rel[0] < IMG_HI):
        # through a pointer: re-derive the address from the pointer's
        # current value on every attempt, since pointer learning may move
        # the object
        return ("rel", rel[0], rel[1], None, kind)
    if src[0] == "mem":
        a = src[1]
        if IMG_LO <= a < IMG_HI:
            ra = pair.w.o2r_data(a)
            if ra is None:
                return None
        else:
            ra = a
        return ("mem", a, ra, a, kind)
    return ("ret", src[1], src[2], None, kind)


def _boundary(pair, cmp_log):
    """Inputs on the boundary of every comparison the original made: the
    traced operand forced equal to what it is compared against, the one
    value where `<` and `<=` (or `>` and `>=`) part ways."""
    for flag in sorted(cmp_log):
        seed, (src, other, mn, rel) = cmp_log[flag]
        if mn != "cmp" or other is None:
            continue
        e = _force(pair, src, rel, other)
        if e is not None:
            yield seed, [e]


def _cleanup_arity(fi, s):
    ins = fi.insns.get(s.ret)
    if ins is None:
        return None
    if ins.mn == "add" and ins.ops[0] == ("reg", "esp") and ins.ops[1][0] == "imm":
        return ins.ops[1][1] // 4
    n = 0
    while ins is not None and ins.mn == "pop" and ins.ops[0] in (("reg", "ecx"), ("reg", "edx")):
        n += 1
        ins = fi.insns.get(ins.next)
    return n or None


def _last_mov_eax(fi, call_ins):
    best = 0
    for a, ins in fi.insns.items():
        if a < call_ins.addr and ins.mn == "mov" and ins.ops[0] == ("reg", "eax") and ins.ops[1][0] == "imm":
            if a > best:
                best = a
    return fi.insns[best].ops[1][1] if best else 0


def _reg_iat(fi, reg, at):
    """The IAT slot a callee-saved register holds at `call reg` (address
    `at`): VC++ hoists a repeatedly called import into ebx/esi/edi/ebp with
    `mov reg, [slot]`.  None unless every definition reaching the call is
    such a load of the same slot."""
    if reg not in ("ebx", "esi", "edi", "ebp"):
        return None

    def gen(ins):
        ops = ins.ops
        if (ops and ops[0] == ("reg", reg) and ins.mn not in ("cmp", "test", "push", "call", "jmp")) \
                or (ins.mn == "xchg" and len(ops) == 2 and ops[1] == ("reg", reg)):
            return ins.addr
        return None

    reach = {fi.start: frozenset(["entry"])}
    work = [fi.start]
    while work:
        x = work.pop()
        ins = fi.insns.get(x)
        if ins is None:
            continue
        g = gen(ins)
        out = frozenset([g]) if g is not None else reach[x]
        for t in (ins.succ if ins.succ else [ins.next]) if ins.kind != "ret" else []:
            if t not in fi.insns:
                continue
            old = reach.get(t)
            nw = out if old is None else old | out
            if nw != old:
                reach[t] = nw
                work.append(t)
    slots = set()
    for d in reach.get(at, ()):
        ins = fi.insns.get(d) if d != "entry" else None
        if ins is None or ins.mn != "mov" or len(ins.ops) != 2 or ins.ops[1][0] != "mem" \
                or ins.ops[1][1] is not None or ins.ops[1][2] is not None:
            return None
        slots.add(ins.ops[1][4])
    return slots.pop() if len(slots) == 1 else None


def _byte_eq(x, y):
    if x[1] == y[1]:
        return True
    return x[2] is not None and y[2] is not None and x[2][0] == y[2][0] and x[2][1] == y[2][1]


def delta_eq(da, db):
    """Write sets are equal if every written byte pairs up: by canonical
    address, or (for image addresses reached by out-of-bounds pointer
    arithmetic) by raw addresses offset like some matched global pair."""
    a, b = dict(da), dict(db)
    for k in list(a):
        if k in b and _byte_eq(a[k], b[k]):
            del a[k], b[k]
    if not a and not b:
        return True
    if len(a) != len(b):
        return False
    rb = {e[0]: e for e in b.values()}
    for e in a.values():
        raw = e[0]
        hit = None
        cands = (0,) + tuple(V.DELTAS) if IMG_LO <= raw < IMG_HI else (0,)
        for d in cands:
            f = rb.get(raw - d)
            if f is not None and _byte_eq(e, f):
                hit = raw - d
                break
        if hit is None:
            return False
        del rb[hit]
    return True


def _is_delta(x):
    return isinstance(x, tuple) and x and isinstance(x[0], tuple) and len(x[0]) == 2 \
        and isinstance(x[0][1], tuple) and len(x[0][1]) == 3 or x == ()


def ev_eq(a, b):
    if a is None or b is None or a[0] != b[0] or len(a) != len(b):
        return False
    if a[0] == "call" and len(a[3]) != len(b[3]):
        # a cdecl callee's arity is read off the caller's stack cleanup,
        # which one side may defer: compare the arguments both sides know
        n = min(len(a[3]), len(b[3]))
        a = a[:3] + (a[3][:n],) + a[4:]
        b = b[:3] + (b[3][:n],) + b[4:]
    for x, y in zip(a, b):
        if _is_delta(x) and _is_delta(y):
            if not delta_eq(x, y):
                return False
        elif isinstance(x, tuple) and isinstance(y, tuple) and not x and not y:
            continue
        elif x != y:
            return False
    return True


def first_diff(eo, er):
    for i, (a, b) in enumerate(zip(eo, er)):
        if not ev_eq(a, b):
            return i
    if len(eo) != len(er):
        return min(len(eo), len(er))
    return None


def describe(ev):
    if ev is None:
        return "(none)"
    kind = ev[0]
    def fmt(x):
        if isinstance(x, V):
            return repr(x)
        if isinstance(x, int):
            return hex(x)
        if isinstance(x, tuple) and len(x) == 2 and isinstance(x[0], str):
            return f"{x[0]}:{fmt(x[1]) if isinstance(x[1], int) else x[1]}"
        return str(x)
    if kind == "call":
        _, ident, ecx, args, delta = ev
        return f"call {fmt(ident)} this={fmt(ecx) if ecx is not None else '-'} args=[{', '.join(fmt(a) for a in args)}] writes={_fmt_delta(delta)}"
    if kind == "ret":
        _, ret, popped, delta = ev
        return f"ret value={fmt(ret) if ret is not None else '-'} pop={popped} writes={_fmt_delta(delta)}"
    return " ".join(fmt(x) if not isinstance(x, tuple) or not x or not isinstance(x[0], tuple) else _fmt_delta(x) for x in ev)


def _fmt_delta(delta, limit=10):
    """Runs of consecutive written bytes as addr=hexbytes."""
    if not delta:
        return "{}"
    runs, cur = [], None
    for k, e in delta:
        if cur and k == cur[0] + len(cur[1]):
            cur[1].append(e[1])
        else:
            cur = [k, [e[1]]]
            runs.append(cur)
    parts = [f"{k:#x}={bytes(bs).hex()}" for k, bs in runs[:limit]]
    if len(runs) > limit:
        parts.append(f"...+{len(runs) - limit}")
    return "{" + " ".join(parts) + "}"


def delta_diff(da, db):
    """Human summary of how two write sets differ."""
    a, b = dict(da), dict(db)
    only_a = sorted((k, a[k]) for k in a if k not in b)
    only_b = sorted((k, b[k]) for k in b if k not in a)
    diff = sorted(k for k in a if k in b and not _byte_eq(a[k], b[k]))
    out = []
    if only_a:
        out.append("orig-only " + _fmt_delta(only_a, 6))
    if only_b:
        out.append("ours-only " + _fmt_delta(only_b, 6))
    if diff:
        out.append("differ " + _fmt_delta([(k, a[k]) for k in diff], 6) + " vs "
                   + _fmt_delta([(k, b[k]) for k in diff], 6))
    return "; ".join(out)


def check(world, pair, runs, directed=48, early=False, boundary=32):
    """Run the pair on `runs` random seeds, then up to `directed` branch-
    directed ones.  early: stop at the first divergence (mutation testing
    only needs to know whether there is one).  When most runs hit the
    instruction budget, the check is redone with a 5x budget (a function
    whose single pass is long, not a runaway loop on a random count)."""
    res = _check(world, pair, runs, directed, early, boundary)
    if res.get("runs") and res.get("timeouts", 0) * 2 > res["runs"] and not res.get("error"):
        pair.budget = MAX_INSNS * 5
        res2 = _check(world, pair, runs, directed, early, boundary)
        res2["budget"] = pair.budget
        if res2.get("timeouts", 0) < res["timeouts"] or res2.get("diffs"):
            return res2
    return res


def _check(world, pair, runs, directed, early, boundary):
    res = {"orig": f"{pair.o:#x}", "recomp": f"{pair.r:#x}", "name": pair.name, "runs": 0, "diffs": 0,
           "timeouts": 0, "first": None, "stack_ok": [pair.fo.stack_ok, pair.fr.stack_ok]}
    bo, br = set(), set()
    cmp_log = {}
    forced_of = {}
    for seed in range(runs):
        if early and res["diffs"]:
            break
        if seed % 8 == 7:
            gc.collect()
        try:
            eo, er, b1, b2 = pair.run_seed(seed)
        except Exception as e:  # harness failure, not a verdict
            res["error"] = f"{type(e).__name__}: {e}"
            break
        forced_of[seed] = pair.last_forced
        for pc, rec in pair.last_cmps.items():
            cmp_log.setdefault(pc, (seed, rec))
        if pair.last_wild == (None, None):
            # coverage counts only runs that are evidence
            bo |= b1
            br |= b2
        if pair.unsettled:
            # every attempt gives both sides the same inputs, so the last
            # one is evidence like any other run; just count it
            res["unsettled"] = res.get("unsettled", 0) + 1
        wo, wr = pair.last_wild
        if wo is not None or wr is not None:
            # a random pointer walked into an image: the run is no evidence
            # either way (one-sided ones are counted for review)
            res["wild"] = res.get("wild", 0) + 1
            if (wo is None) != (wr is None):
                res["wild1"] = res.get("wild1", 0) + 1
            continue
        res["runs"] += 1
        if eo[-1][0] == "timeout" or er[-1][0] == "timeout":
            res["timeouts"] += 1
            # compare only the common prefix of call events
            n = min(len(eo), len(er)) - 1
            eo, er = eo[:n], er[:n]
        elif eo[-1][0] in ("fault", "intr") and er[-1][0] in ("fault", "intr"):
            # both stopped on what a random input did (a wild pointer into an
            # image, a divide by zero): only the calls before it are evidence
            res["faults"] = res.get("faults", 0) + 1
            eo, er = eo[:-1], er[:-1]
        i = first_diff(eo, er)
        if i is not None:
            res["diffs"] += 1
            if res["first"] is None:
                a = eo[i] if i < len(eo) else None
                b = er[i] if i < len(er) else None
                info = {"seed": seed, "event": i, "orig": describe(a), "ours": describe(b)}
                if a and b and a[0] == b[0] and not delta_eq(a[-1], b[-1]):
                    info["writes"] = delta_diff(a[-1], b[-1])
                res["first"] = info
    # branch-directed inputs for what the random ones missed
    if directed and not (early and res["diffs"]):
        budget = directed
        tried = set()

        def frontier():
            # recomputed after every run that reached something new
            while True:
                progress = False
                for seed, extra in list(_directed(pair, set(bo), cmp_log)):
                    key = (seed, repr(extra))
                    if key in tried:
                        continue
                    tried.add(key)
                    before = len(bo)
                    yield seed, extra
                    if len(bo) > before:
                        progress = True
                        break
                if not progress:
                    return

        for seed, extra in frontier():
            if budget <= 0 or (early and res["diffs"]):
                break
            budget -= 1
            base = forced_of.get(seed)
            if isinstance(seed, tuple):
                seed = seed[0]
            try:
                eo, er, b1, b2 = pair.run_seed(seed, extra=extra, base=base)
            except Exception as e:
                res["directed_error"] = f"{type(e).__name__}: {e}"
                continue
            res["directed"] = res.get("directed", 0) + 1
            wo, wr = pair.last_wild
            if wo is not None or wr is not None:
                continue
            new_cov = bool(b1 - bo)
            bo |= b1
            br |= b2
            if eo[-1][0] == "timeout" or er[-1][0] == "timeout":
                n = min(len(eo), len(er)) - 1
                eo, er = eo[:n], er[:n]
            elif eo[-1][0] in ("fault", "intr") and er[-1][0] in ("fault", "intr"):
                eo, er = eo[:-1], er[:-1]
            res["runs"] += 1
            i = first_diff(eo, er)
            if i is not None:
                res["diffs"] += 1
                if res["first"] is None:
                    a = eo[i] if i < len(eo) else None
                    b = er[i] if i < len(er) else None
                    res["first"] = {"seed": seed, "extra": repr(extra), "event": i,
                                    "orig": describe(a), "ours": describe(b)}
            if new_cov:
                # later rounds build on this run's inputs
                dseed = (seed, len(forced_of))
                forced_of[dseed] = pair.last_forced
                for pc, rec in pair.last_cmps.items():
                    cmp_log.setdefault(pc, (dseed, rec))
    # boundary inputs: every comparison the original made, at equality
    if boundary and not (early and res["diffs"]):
        for seed, extra in list(_boundary(pair, cmp_log))[:boundary]:
            if early and res["diffs"]:
                break
            base = forced_of.get(seed)
            if isinstance(seed, tuple):
                seed = seed[0]
            try:
                eo, er, b1, b2 = pair.run_seed(seed, extra=extra, base=base)
            except Exception as e:
                res["directed_error"] = f"{type(e).__name__}: {e}"
                continue
            if pair.last_wild != (None, None):
                continue
            res["boundary"] = res.get("boundary", 0) + 1
            bo |= b1
            br |= b2
            if eo[-1][0] == "timeout" or er[-1][0] == "timeout":
                n = min(len(eo), len(er)) - 1
                eo, er = eo[:n], er[:n]
            elif eo[-1][0] in ("fault", "intr") and er[-1][0] in ("fault", "intr"):
                eo, er = eo[:-1], er[:-1]
            res["runs"] += 1
            i = first_diff(eo, er)
            if i is not None:
                res["diffs"] += 1
                if res["first"] is None:
                    a = eo[i] if i < len(eo) else None
                    b = er[i] if i < len(er) else None
                    res["first"] = {"seed": seed, "extra": repr(extra), "event": i,
                                    "orig": describe(a), "ours": describe(b)}
    res["cov"] = [len(bo & pair.fo.blocks), len(pair.fo.blocks), len(br & pair.fr.blocks), len(pair.fr.blocks)]
    # block starts, and which of them no conclusive run reached (semdiff.py)
    res["blocks"] = [sorted(pair.fo.blocks), sorted(pair.fr.blocks)]
    res["uncovered"] = [sorted(pair.fo.blocks - bo), sorted(pair.fr.blocks - br)]
    pair.covered = (bo, br)
    return res


_WORLD = None


RUNNING = ROOT / "build" / "equiv-running"


def _pool_results(pool, todo):
    """Results from pool workers, unordered.  A worker that dies (OOM-killed
    by a function that makes Unicorn map without bound) loses its task, and
    multiprocessing.Pool would then wait for it forever; each worker notes
    the task it is on in RUNNING/<pid>, so a vanished worker's task is
    reported as a harness error instead."""
    it = pool.imap_unordered(_worker, todo, chunksize=1)
    left = len(todo)
    while left:
        try:
            res = it.next(timeout=30)
        except mp_TimeoutError:
            for f in RUNNING.glob("*"):
                if not Path(f"/proc/{f.name}").exists():
                    o, r, name = f.read_text().split(" ", 2)
                    f.unlink()
                    left -= 1
                    yield {"orig": o, "recomp": r, "name": name, "error": "worker died (out of memory?)"}
            continue
        left -= 1
        yield res


def _worker(args):
    o, r, osz, rsz, name, runs = args
    note = RUNNING / str(os.getpid())
    RUNNING.mkdir(parents=True, exist_ok=True)   # datadiff/mutate call in directly
    note.write_text(f"{o:#x} {r:#x} {name}")
    try:
        return _work1(o, r, osz, rsz, name, runs)
    finally:
        note.unlink(missing_ok=True)


def _work1(o, r, osz, rsz, name, runs):
    global _WORLD
    if _WORLD is None:
        _WORLD = World()
        _WORLD.setup_imports()
    try:
        pair = Pair(_WORLD, o, r, osz, rsz, name)
    except Exception as e:
        return {"orig": f"{o:#x}", "recomp": f"{r:#x}", "name": name, "error": f"setup {type(e).__name__}: {e}"}
    return check(_WORLD, pair, runs)


def main():
    ap = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    ap.add_argument("addrs", nargs="*")
    ap.add_argument("--runs", type=int, default=100)
    ap.add_argument("--all", action="store_true", help="include functions that already match 100%%")
    ap.add_argument("--json", help="write per-function results here")
    ap.add_argument("-j", "--jobs", type=int, default=os.cpu_count())
    ap.add_argument("-v", "--verbose", action="store_true")
    ap.add_argument("--trace", type=int, metavar="SEED",
                    help="print every event of both sides for one seed of one function")
    a = ap.parse_args()
    if a.trace is not None:
        return trace(a.addrs[0], a.trace)

    world = World()
    world.setup_imports()
    want = {int(x, 16) for x in a.addrs}
    acc = {}
    cmp_json = ROOT / "build" / "compare.json"
    if cmp_json.exists():
        for x in json.loads(cmp_json.read_text())["data"]:
            acc[int(x["address"], 16)] = x["matching"]
    todo = []
    for o, r, osz, rsz, name in world.funcs:
        if want and o not in want:
            continue
        if not want and not a.all and acc.get(o, 0) >= 1.0:
            continue
        if not osz or not rsz:
            continue
        todo.append((o, r, osz, rsz, name, a.runs))
    todo.sort()
    results = []
    # results so far, one JSON object per line, so a killed run keeps them
    partial = open(a.json + ".partial", "w") if a.json else None

    def got(res):
        results.append(res)
        _print(res, a.verbose, acc)
        if partial:
            partial.write(json.dumps(res) + "\n")
            partial.flush()

    if a.jobs > 1 and len(todo) > 1:
        import multiprocessing as mp
        RUNNING.mkdir(exist_ok=True)
        for f in RUNNING.glob("*"):
            f.unlink()
        with mp.Pool(a.jobs) as pool:
            for res in _pool_results(pool, todo):
                got(res)
    else:
        global _WORLD
        _WORLD = world
        for t in todo:
            got(_work1(*t))
    results.sort(key=lambda x: x["orig"])
    bad = [x for x in results if x.get("diffs") or x.get("error")]
    print(f"\n{len(results)} functions checked, {len(results) - len(bad)} equivalent on every run, "
          f"{sum(1 for x in results if x.get('diffs'))} diverge, {sum(1 for x in results if x.get('error'))} harness errors")
    if a.json:
        Path(a.json).write_text(json.dumps(results, indent=1))


def trace(addr, seed):
    world = World()
    world.setup_imports()
    if ":" in addr:
        # an unmatched pair, orig:ours, each bounded by the next known start
        o, r = (int(x, 16) for x in addr.split(":"))
        f = (o, r, min(world.next_start(0, o) - o, 0x400), min(world.next_start(1, r) - r, 0x400),
             f"unmatched {o:#x}")
    else:
        o = int(addr, 16)
        f = next(x for x in world.funcs if x[0] == o)
    pair = Pair(world, *f)
    for side, fi in (("orig", pair.fo), ("ours", pair.fr)):
        unk = [(hex(s.insn.addr), s.pop) for s in fi.calls.values() if not s.known]
        print(f"{side} {fi.start:#x}..{fi.end:#x} stack_ok={fi.stack_ok} solved indirect pops={unk}")
    wr = []
    eo, er, bo, br = pair.run_seed(seed, wr)
    n = max(len(eo), len(er))
    so, sr = pair.last_sites
    for i in range(n):
        a = eo[i] if i < len(eo) else None
        b = er[i] if i < len(er) else None
        mark = "  " if ev_eq(a, b) else "!!"
        at_o = f"@{so[i]:#x}" if i < len(so) else ""
        at_r = f"@{sr[i]:#x}" if i < len(sr) else ""
        print(f"{mark} {i:3} orig{at_o}: {describe(a)}")
        if not ev_eq(a, b):
            print(f"       ours{at_r}: {describe(b)}")
            if a and b and a[0] == b[0] and not delta_eq(a[-1], b[-1]):
                print(f"       {delta_diff(a[-1], b[-1])}")
                da, db = dict(a[-1]), dict(b[-1])
                shown = 0
                for k in sorted(set(da) | set(db)):
                    ea, eb = da.get(k), db.get(k)
                    if ea is None or eb is None or not _byte_eq(ea, eb):
                        po, pr = wr[0].get(k), wr[1].get(k)
                        print(f"       {k:#x}: orig {ea and hex(ea[1])} {ea and ea[2]} written at {po and hex(po)}, "
                              f"ours {eb and hex(eb[1])} {eb and eb[2]} written at {pr and hex(pr)}")
                        shown += 1
                        if shown >= 8:
                            break


def _print(res, verbose, acc):
    o = int(res["orig"], 16)
    pct = f"{acc.get(o, 0) * 100:5.1f}%" if acc else ""
    if res.get("error"):
        print(f"{res['orig']} {pct} ERROR {res['name']}: {res['error']}")
        return
    cov = res["cov"]
    tag = "DIFF" if res["diffs"] else ("ok  " if res["runs"] >= 16 else "weak")
    line = (f"{res['orig']} {pct} {tag} {res['diffs']:3}/{res['runs']} cov {cov[0]}/{cov[1]} {cov[2]}/{cov[3]}"
            f"{' T' + str(res['timeouts']) if res['timeouts'] else ''}"
            f"{' W' + str(res['wild']) if res.get('wild') else ''}"
            f"{'(' + str(res['wild1']) + ' one-sided)' if res.get('wild1') else ''}"
            f"{' F' + str(res['faults']) if res.get('faults') else ''}"
            f"{' U' + str(res['unsettled']) if res.get('unsettled') else ''}"
            f"{' D' + str(res['directed']) if res.get('directed') else ''}"
            f"{' B' + str(res['boundary']) if res.get('boundary') else ''}"
            f"{'' if all(res['stack_ok']) else ' stack?'} {res['name']}")
    print(line, flush=True)
    if res["diffs"]:
        f = res["first"]
        print(f"      seed {f['seed']} event {f['event']}:\n        orig: {f['orig']}\n        ours: {f['ours']}")
        if f.get("writes"):
            print(f"        {f['writes']}")


if __name__ == "__main__":
    main()
