#!/usr/bin/env python3
"""Original-function sizes for reccmp.

reccmp does not know how long a function is in the *original* binary: unless
told, it reads as many original bytes as our recompiled function occupies, so
whenever ours is shorter the comparison silently truncates the original (and
whenever ours is longer it reads into the next function). compare.py calls
install() after reccmp's analysis to register a size for every matched
original function, derived by linear disassembly of hover.exe's .text: MSVC
pads between functions with int3 (0xCC), so a function ends at the first
int3 that follows a ret/jmp (capped at the next marker address).

Standalone: `origsize.py 0xVA` prints the derived size of one function.
"""
import bisect
import struct
import sys

_ORIG = None  # (text_start, text_bytes, sorted starts)


def _load(path):
    data = open(path, "rb").read()
    pe = struct.unpack_from("<I", data, 0x3C)[0]
    nsec = struct.unpack_from("<H", data, pe + 6)[0]
    opt = struct.unpack_from("<H", data, pe + 20)[0]
    base = struct.unpack_from("<I", data, pe + 24 + 28)[0]
    for i in range(nsec):
        o = pe + 24 + opt + 40 * i
        if data[o:o + 8].rstrip(b"\0") == b".text":
            vsz, va, rsz, raw = struct.unpack_from("<IIII", data, o + 8)
            return base + va, data[raw:raw + min(vsz, rsz)]
    raise RuntimeError("no .text section")


class OrigSizes:
    """Function sizes by linear disassembly: a function ends at the first
    int3 that follows a ret or an unconditional jmp (MSVC pads between
    functions with int3), capped at the next known function start. A plain
    int3-run scan is not enough because 0xCC also occurs inside
    instructions (e.g. as the displacement of [ebp-0x34])."""

    def __init__(self, path, known_starts=()):
        from capstone import CS_ARCH_X86, CS_MODE_32, Cs

        self.text_start, self.text = _load(path)
        self.md = Cs(CS_ARCH_X86, CS_MODE_32)
        self.starts = sorted(set(known_starts) | {self.text_start + len(self.text)})

    def size(self, addr):
        i = bisect.bisect_right(self.starts, addr)
        if i >= len(self.starts):
            return None
        cap = self.starts[i]
        o = addr - self.text_start
        if o < 0 or cap - self.text_start > len(self.text):
            return None
        end = None
        after_exit = False
        prologue = None  # address of a `push ebp` seen right after a ret/jmp
        thunk = None     # address of a `mov eax,[ecx]` seen right after one
        for ins in self.md.disasm(self.text[o:cap - self.text_start], addr):
            if ins.mnemonic == "int3":
                if after_exit:
                    end = ins.address
                    break
                continue
            # Modules without int3 padding (the statically linked libraries):
            # a `push ebp / mov ebp, esp` prologue right after a ret/jmp
            # starts the next function.
            if prologue is not None:
                if ins.mnemonic == "mov" and ins.op_str == "ebp, esp":
                    end = prologue
                    break
                prologue = None
            elif after_exit and ins.mnemonic == "push" and ins.op_str == "ebp":
                prologue = ins.address
            # ... and so does a compiler-generated vtable forwarding thunk
            # (`mov eax, [ecx] / jmp dword ptr [eax+N]`), which is what
            # MSVC packs behind the last real function of a module (e.g.
            # hover.exe:0x43dd1f, right after Hover_sub_43DC40).
            if thunk is not None:
                if ins.mnemonic == "jmp" and ins.op_str.startswith(
                        "dword ptr [eax"):
                    end = thunk
                    break
                thunk = None
            elif after_exit and ins.mnemonic == "mov" and \
                    ins.op_str == "eax, dword ptr [ecx]":
                thunk = ins.address
            after_exit = ins.mnemonic in ("ret", "jmp")
        if end is None:
            end = cap
            # strip trailing padding when capped by the next start
            while end > addr and self.text[end - self.text_start - 1] == 0xCC:
                end -= 1
        return end - addr


def install(compare, orig_path):
    """Register original sizes for every matched function in a reccmp
    Compare object (call after compare.run())."""
    from reccmp.types import ImageId

    matches = list(compare.get_functions())
    sizes = OrigSizes(orig_path, (m.orig_addr for m in matches))
    with compare._db.batch() as batch:  # pylint: disable=protected-access
        for m in matches:
            s = sizes.size(m.orig_addr)
            ours = m.any_size(ImageId.RECOMP) or 0
            # Sanity cap: if the derived size dwarfs our function the end
            # heuristic has failed (no padding, data after the ret, ...);
            # leave reccmp's default rather than compare against garbage.
            if s and s <= max(4 * ours, ours + 1024):
                batch.set(ImageId.ORIG, m.orig_addr, size=s)


if __name__ == "__main__":
    va = int(sys.argv[1], 16)
    print(OrigSizes("/workspace/hover/hover.exe", [va]).size(va))
