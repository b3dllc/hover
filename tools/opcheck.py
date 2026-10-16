#!/usr/bin/env python3
"""Operand invariants: member offsets, constants and data references.

Companion to tools/callcheck.py (rule 353).  Where that compares the multiset
of call targets per function, this compares three more properties that are
independent of register allocation and frame layout, and that a wrong source
line changes while a merely different one does not:

  disp   the displacement of every memory operand whose base is not the frame
         (`[esi + 0x88]` -> `disp+0x88`): a member or array-element offset.
         A wrong field is a wrong displacement.
  imm    constants in push / cmp / test / and / or / xor / imul / shifts /
         constant stores / non-zero register loads, and add/sub by absolute
         value (so `add eax,-0x10` and `sub eax,0x10` agree).  Excludes the
         stack adjustments and `ret n`.
  data   named data operands reccmp resolved on both sides: globals, strings,
         vtables, float constants (`g_dwUnk_4A1D50 (DATA)` etc.).

    tools/compare.py --json /tmp/x.json && tools/opcheck.py /tmp/x.json
        [--only disp|imm|data] [--presence] [--summary] [--all] [0xADDR ...]

--presence keeps only properties one side has and the other lacks entirely;
count-only differences (3 reads of a global vs 4) are nearly always CSE or a
hoisted address and are the bulk of the noise.

Known noise, so nobody over-reads a hit: a hoisted address (`lea eax,[esi+0x88]`
then `[eax+0x10]`) splits one displacement into two; inlining depth moves a
callee's operands into the caller on one side; the `xor r,r` / `mov r,0` and
`inc` / `add 1` idioms are folded, others are not.  Unresolved `<OFFSETn>`
operands on the original side are soft (shown with --all).
"""
import json
import os
import re
import sys
from collections import Counter

MEM = re.compile(r"\[([^\]]*)\]")
FRAME = re.compile(r"\b(ebp|esp)\b")
FRAME_NOEBP = re.compile(r"\besp\b")
# ebp written as data: the function has no frame pointer and ebp is a plain
# register there (rule 362), so [ebp+N] is a member access, not a local
EBP_AS_GPR = re.compile(r"^(mov|lea|add|sub|xor|and|or|inc|dec|imul|shl|sar|shr)\s+ebp\b(?!,\s*esp\b)")
DISP = re.compile(r"([-+])\s*(0x[0-9a-f]+|\d+)\s*$")
IMM = re.compile(r"(?<![\w\]])(-?0x[0-9a-f]+|-?\d+)\b")
NAMED = re.compile(r"^(.+?)\s*\((DATA|STRING|VTABLE|FLOAT|POINTER)\)$")
UNRESOLVED = re.compile(r"<OFFSET\d+>")

IMM_OPS = {"push", "cmp", "test", "and", "or", "xor", "imul", "shl", "shr",
           "sar", "rol", "ror", "mov", "add", "sub", "adc", "sbb", "lea"}


def tokens(text):
    """(mnemonic, [operands]) from a disassembly line, minus annotations."""
    t = text.strip()
    t = re.sub(r"\s*\(.*?\)\s*$", "", t) if t.endswith(")") and "(" in t and not t.endswith("))") else t
    parts = t.split(None, 1)
    if len(parts) == 1:
        return parts[0], []
    return parts[0], [o.strip() for o in parts[1].split(",")]


def hexnorm(v):
    n = int(v, 16) if v.lower().startswith(("0x", "-0x")) else int(v)
    return n


def props(text, frame=FRAME):
    """Return (hard_keys, soft_keys) for one instruction."""
    hard, soft = [], []
    raw = text.strip()
    if UNRESOLVED.search(raw):
        soft.append("unresolved")
    # named data operands are annotated inline by reccmp: `..., g_x (DATA)`
    for m in re.finditer(r"([\w:?@$`'.~<>\-]+)\s*\((DATA|STRING|VTABLE|FLOAT|POINTER)\)", raw):
        hard.append("data:%s" % m.group(1))
    mn, ops = tokens(raw)
    if mn in ("ret", "jmp", "call") or mn.startswith("j"):
        return hard, soft
    # displacements
    for m in MEM.finditer(raw):
        inner = m.group(1)
        if frame.search(inner):
            continue
        d = DISP.search(inner)
        if d:
            n = int(d.group(2), 16) if d.group(2).startswith("0x") else int(d.group(2))
            if d.group(1) == "-":
                n = -n
            if n != 0:
                hard.append("disp%+#x" % n)
    # immediates
    if mn in IMM_OPS and ops:
        # stack adjustments are frame facts, not constants; `add ebp,N` only
        # when ebp is the frame register on this side (rule 367)
        if mn in ("add", "sub", "lea") and (ops[0] == "esp" or
                                            (ops[0] == "ebp" and frame is FRAME)):
            return hard, soft
        if mn == "lea":
            return hard, soft
        stripped = MEM.sub("[]", raw.split(None, 1)[1] if " " in raw else "")
        last = stripped.split(",")[-1].strip()
        im = re.fullmatch(r"-?0x[0-9a-f]+|-?\d+", last)
        if im:
            n = hexnorm(last)
            if mn in ("add", "sub", "adc", "sbb"):
                n = abs(n)
            if mn == "mov" and n == 0 and not ops[0].startswith("["):
                pass  # xor idiom
            else:
                hard.append("imm:%s:%#x" % ("add" if mn in ("add", "sub") else mn, n))
        elif mn in ("inc", "dec"):
            hard.append("imm:add:0x1")
    return hard, soft


def collect(entity):
    orig, recomp = Counter(), Counter()
    # first pass: does either side use ebp as an ordinary register?
    gpr = {"orig": False, "recomp": False}
    for hunk in entity.get("diff") or []:
        for block in hunk[1]:
            for side, rows in block.items():
                for row in rows:
                    if EBP_AS_GPR.match(row[1].strip()):
                        for who in (("orig", "recomp") if side == "both" else (side,)):
                            gpr[who] = True
    frame_of = {k: (FRAME_NOEBP if v else FRAME) for k, v in gpr.items()}
    for hunk in entity.get("diff") or []:
        for block in hunk[1]:
            for side, rows in block.items():
                for row in rows:
                    who = "orig" if side == "orig" else "recomp" if side == "recomp" else "orig"
                    hard, soft = props(row[1], frame_of[who])
                    keys = hard + ["~" + s for s in soft]
                    for k in keys:
                        if side in ("both", "orig"):
                            orig[k] += 1
                        if side in ("both", "recomp"):
                            recomp[k] += 1
    return orig, recomp


def main():
    args = [a for a in sys.argv[1:] if not a.startswith("--")]
    flags = [a for a in sys.argv[1:] if a.startswith("--")]
    only = None
    for f in flags:
        if f.startswith("--only"):
            only = f.split("=", 1)[1] if "=" in f else None
    if "--only" in flags:
        i = sys.argv.index("--only")
        only = sys.argv[i + 1]
        args = [a for a in args if a != only]
    data = json.load(open(args[0]))["data"]
    want = [int(a, 16) for a in args[1:]]

    sizes = {}
    try:
        sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
        import origsize
        addrs = [int(i["address"], 16) for i in data if i.get("address")]
        sizer = origsize.OrigSizes("/workspace/hover/hover.exe", known_starts=addrs)
        sizes = {a: sizer.size(a) or 0 for a in addrs}
    except Exception:
        pass

    findings = []
    cat_tot = Counter()
    for it in data:
        if "matching" not in it or not it.get("diff"):
            continue
        addr = int(it["address"], 16)
        if want and addr not in want:
            continue
        orig, recomp = collect(it)
        missing, extra = orig - recomp, recomp - orig
        if "--presence" in flags:
            # keep only properties that one side has and the other lacks
            # ENTIRELY: a count difference (3 reads vs 4) is usually CSE or a
            # hoisted address, a presence difference is usually a wrong line
            missing = Counter({k: v for k, v in missing.items() if recomp.get(k, 0) == 0})
            extra = Counter({k: v for k, v in extra.items() if orig.get(k, 0) == 0})
        if "--all" not in flags:
            missing = Counter({k: v for k, v in missing.items() if not k.startswith("~")})
            extra = Counter({k: v for k, v in extra.items() if not k.startswith("~")})
        if only:
            missing = Counter({k: v for k, v in missing.items() if k.startswith(only)})
            extra = Counter({k: v for k, v in extra.items() if k.startswith(only)})
        if not missing and not extra:
            continue
        for k in list(missing) + list(extra):
            cat_tot[k.split(":")[0].rstrip("+-0123456789xabcdef") or k[:4]] += 1
        findings.append((sizes.get(addr, 0), it["address"], it["name"], it["matching"], missing, extra))

    findings.sort(key=lambda f: -f[0])
    if "--summary" in flags:
        print("functions flagged: %d" % len(findings))
        for c, n in cat_tot.most_common():
            print("  %-6s %d" % (c, n))
        return
    for size, a, name, ratio, missing, extra in findings:
        print("%s  %s  %d bytes, %.1f%% matched" % (a, name, size, 100 * ratio))
        for k, n in sorted(missing.items()):
            print("    original has, we do not:  %s%s" % (k, "" if n == 1 else " x%d" % n))
        for k, n in sorted(extra.items()):
            print("    we have, original does not: %s%s" % (k, "" if n == 1 else " x%d" % n))
    print("\n%d of %d compared functions differ" % (len(findings), sum(1 for i in data if "matching" in i)))


main()
