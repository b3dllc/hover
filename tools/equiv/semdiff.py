#!/usr/bin/env python3
"""Static check for semantic differences hidden in non-matching code.

reccmp aligns each function's instructions with the original's.  Register
allocation, stack-slot numbering, branch offsets and instruction order may
all differ without changing behaviour; what must not differ is *what* the
code computes with.  For every non-matching function this collects the
semantic atoms of the instructions that appear on one side only:

  imm:N      an immediate operand (constants, loop bounds, flags)
  fld:+N     a structure field offset ([reg + N], not the frame)
  sym:NAME   a referenced global, function, string or vtable
  op:KIND    signedness-sensitive operations: movsx/movzx, sar/shr,
             idiv/div, imul/mul, cdq, and the condition family of every
             jcc/setcc (eq, signed, unsigned, sign, parity)

and reports atoms whose counts do not cancel between the two sides.  A
constant written as `cmp x, 7; jg` on one side and `cmp x, 8; jge` on the
other shows up as imm:7 vs imm:8, which the emulator's boundary forcing then
has to explain; a signed compare that became unsigned shows up as
op:signed vs op:unsigned.

With --equiv (the --json output of equiv.py, which records the basic
blocks no conclusive run reached) only instructions in uncovered blocks are
considered: everything else was executed and compared by the emulator.  -v
then lists those instructions for review.

Usage: semdiff.py [--json build/compare.json] [--equiv equiv.json ...] [-v] [addrs ...]
"""

import argparse
import json
import re
from bisect import bisect_right
from collections import Counter
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
LIBRARY_START = 0x43E400

COND = {
    "e": "eq", "z": "eq", "ne": "eq", "nz": "eq",
    "l": "signed", "nge": "signed", "ge": "signed", "nl": "signed",
    "g": "signed", "nle": "signed", "le": "signed", "ng": "signed",
    "b": "unsigned", "nae": "unsigned", "c": "unsigned", "ae": "unsigned", "nb": "unsigned", "nc": "unsigned",
    "a": "unsigned", "nbe": "unsigned", "be": "unsigned", "na": "unsigned",
    "s": "sign", "ns": "sign", "p": "parity", "np": "parity", "pe": "parity", "po": "parity",
    "o": "overflow", "no": "overflow",
}
SIGNED_OPS = {"movsx": "movsx", "movzx": "movzx", "sar": "sar", "shr": "shr", "idiv": "idiv",
              "div": "div", "imul": "imul", "mul": "mul", "cdq": "cdq", "cwde": "cwde", "cbw": "cbw"}
REGS = r"(?:e?[abcd]x|[abcd][lh]|e?[sd]i|e?[sb]p)"
FRAME = re.compile(r"\[(?:ebp|esp)\s*[-+]\s*(?:0x[0-9a-f]+|\d+)\]")
FIELD = re.compile(r"\[" + REGS + r"(?:\s*\+\s*" + REGS + r"(?:\s*\*\s*\d)?)?\s*([-+])\s*(0x[0-9a-f]+|\d+)\]")
SYM = re.compile(r"([^\s,\[\]][^,\[\]]*?)\s+\((?:DATA|FUNCTION|STRING|VTABLE|OFFSET|LABEL|POINTER|IMPORT|FLOAT)\)")
IMM = re.compile(r"(?:^|[\s,])(-?0x[0-9a-f]+|-?\d+)(?=$|[\s,])")


def atoms(text):
    text = text.split("\t")[0].strip()
    mn, _, ops = text.partition(" ")
    out = []
    if mn.startswith("j") and mn not in ("jmp",):
        c = COND.get(mn[1:])
        if c:
            out.append("op:" + c)
        return out  # the target offset is layout, not semantics
    if mn.startswith("set"):
        c = COND.get(mn[3:])
        if c:
            out.append("op:" + c)
    if mn in ("jmp", "call") and not SYM.search(ops):
        return out
    if mn in SIGNED_OPS:
        out.append("op:" + SIGNED_OPS[mn])
    for m in SYM.finditer(ops):
        name = re.sub(r"^(?:" + REGS + r"\s*\+\s*)+", "", m.group(1).strip()).lstrip("->")
        if name != "___CxxFrameHandler":
            out.append("sym:" + name)
    ops_ns = SYM.sub("", ops)
    ops_ns = FRAME.sub("", ops_ns)
    for m in FIELD.finditer(ops_ns):
        n = int(m.group(2), 0) * (-1 if m.group(1) == "-" else 1)
        out.append(f"fld:{n:+#x}")
    ops_nf = FIELD.sub("", ops_ns)
    ops_nf = re.sub(r"\[[^\]]*\]", "", ops_nf)
    if mn not in ("push",) or not ops.startswith(("e", "[")):
        for m in IMM.finditer(ops_nf):
            v = int(m.group(1), 0) & 0xFFFFFFFF
            if mn in ("add", "sub") and ops.startswith("esp"):
                continue  # stack adjustment: layout
            out.append(f"imm:{v:#x}")
    return out


def _in(blocks, uncovered, a):
    i = bisect_right(blocks, a) - 1
    return i >= 0 and blocks[i] in uncovered


def function_delta(entry, cov=None):
    """Atoms of one-sided lines that do not cancel, and the one-sided lines
    themselves.  With cov (equiv.py's blocks/uncovered), only lines in
    blocks no conclusive run reached."""
    o, r = Counter(), Counter()
    lines = []
    for _hdr, blocks in entry.get("diff") or []:
        for b in blocks:
            for side, key in ((0, "orig"), (1, "recomp")):
                for line in b.get(key, []):
                    if not line[0].startswith("0x"):
                        continue
                    if cov is not None and not _in(cov[0][side], cov[1][side], int(line[0], 16)):
                        continue
                    (o if side == 0 else r).update(atoms(line[1]))
                    lines.append((side, line[0], line[1]))
    return o - r, r - o, lines


def main():
    ap = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    ap.add_argument("addrs", nargs="*")
    ap.add_argument("--json", default=str(ROOT / "build" / "compare.json"))
    ap.add_argument("--equiv", nargs="*", default=[], help="equiv.py --json output(s)")
    ap.add_argument("-v", "--verbose", action="store_true")
    a = ap.parse_args()
    data = json.loads(Path(a.json).read_text())["data"]
    cov = {}
    for f in a.equiv:
        for x in json.loads(Path(f).read_text()):
            if "blocks" in x:
                cov[int(x["orig"], 16)] = (x["blocks"], [set(u) for u in x["uncovered"]])
    want = {int(x, 16) for x in a.addrs}
    n_fn = n_flag = n_lines = 0
    kinds = Counter()
    for e in data:
        addr = int(e["address"], 16)
        if e.get("matching", 0) >= 1 or addr >= LIBRARY_START or (want and addr not in want):
            continue
        if a.equiv and addr not in cov:
            continue  # not checked (e.g. matched by bytes)
        n_fn += 1
        oo, rr, lines = function_delta(e, cov.get(addr) if a.equiv else None)
        n_lines += len(lines)
        if not oo and not rr and not (a.equiv and lines):
            continue
        n_flag += 1
        for k in list(oo) + list(rr):
            kinds[k.split(":")[0]] += 1
        fmt = lambda c: " ".join(f"{k}" + (f"x{v}" if v > 1 else "") for k, v in sorted(c.items()))
        print(f"{e['address']} {e['matching'] * 100:5.1f}% {e['name']}")
        if oo:
            print(f"    orig only:  {fmt(oo)}")
        if rr:
            print(f"    ours only:  {fmt(rr)}")
        if a.verbose:
            for side, ad, text in lines:
                print(f"      {'-+'[side]} {ad} {text.split(chr(9))[0][:90]}")
    if a.equiv:
        print(f"\n{n_lines} one-sided instructions lie in blocks no conclusive run reached")
    print(f"\n{n_flag}/{n_fn} non-matching functions have atoms that do not cancel "
          f"({', '.join(f'{k} {v}' for k, v in kinds.most_common())})")


if __name__ == "__main__":
    main()
