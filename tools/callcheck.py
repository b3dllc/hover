#!/usr/bin/env python3
"""Call-target invariant: every function must call the same things the original
calls, the same number of times.

Match percentage is dominated by register allocation and frame layout, which
cannot change behaviour.  The behavioural bugs this project keeps finding are a
different animal: a call to the near-twin of the right function (`CString::Right`
for `Left`, `_stricmp` for `_mbscmp`, `DeleteObject` for `Detach`, `SetSize` for
`RemoveAt`, `AddHead` for `AddTail`, the wrong array flavour of `SetAtGrow`).
Each of those is one instruction, so it barely dents the score, but it is a
plain difference in the set of things the function calls - mechanical, with no
judgement involved, and reproducible on every build.

reccmp resolves call targets by name on BOTH sides, so for each function we can
compare the multiset of targets the original calls against the multiset ours
calls.  A target we call fewer times than the original is a MISSING call; more
times is an EXTRA call.  Position and scheduling are ignored on purpose: a call
that merely moved appears on both sides and cancels.

    tools/compare.py --json /tmp/x.json && tools/callcheck.py /tmp/x.json

Options:
    --all       also list functions whose only differences are unresolved
                (raw-address) targets, which are usually a naming gap, not a bug
    --quiet     summary line only

Blind spots, stated so nobody over-trusts a clean run: an indirect call is
compared by its vtable offset only (the register is normalised away), a target
mis-named in tools/orig_library.csv compares wrong on both sides, and nothing
here sees arithmetic, constants or control flow.
"""
import json
import os
import re
import sys
from collections import Counter

CALL = re.compile(r"^call\s+(.*)$")
BARE_REG = re.compile(r"^[a-z]{2,3}$")
MEMCALL = re.compile(r"^(?:dword ptr\s*)?\[\s*([a-z]{2,3})\s*(?:([-+])\s*(0x[0-9a-f]+|\d+))?\s*\]$")
SCALED = re.compile(r"^(?:dword ptr\s*)?\[.*\*\d.*\]$")
UNRESOLVED = re.compile(r"^(<OFFSET\d+>|(?:0x)?[0-9a-f]{5,8})$")


def normalise(target):
    """Return (key, hard) - hard is False for forms that cannot be compared
    across builds because they encode register or frame allocation rather than
    the identity of the callee."""
    t = target.strip()
    t = re.sub(r"\s*\((FUNCTION|THUNK|LABEL|VTABLE|POINTER|STRING|FLOAT|DATA|IMPORT)\)$", "", t)
    if BARE_REG.match(t):
        # `call edi`: the callee's address is in a register, usually a hoisted
        # import.  Which register is allocation; the identity is invisible here.
        return "call-through-register", False
    if UNRESOLVED.match(t):
        # the original's target has no name in tools/orig_library.csv yet
        return "unnamed-target", False
    if SCALED.match(t):
        return "call-through-table", False
    m = MEMCALL.match(t)
    if m:
        base, sign, off = m.group(1), m.group(2) or "+", m.group(3) or "0"
        if not off.startswith("0x"):
            off = hex(int(off))
        if base in ("ebp", "esp") or sign == "-":
            # a local slot: our frames differ by design, so the offset is noise
            return "call-through-local", False
        return "vtable-slot[%s%s]" % (sign, off), True
    return t, True


HOIST = re.compile(r"^mov\s+[a-z]{2,3},\s*dword ptr \[(.*?)(?:\s*\(IMPORT\))?\]$")


def collect(entity):
    """Return (orig_multiset, recomp_multiset) of call targets in the diff,
    plus the imports each side loads into a register (`mov esi, [GlobalAlloc]`),
    which are then called as `call esi` and so never appear as a named call."""
    orig, recomp = Counter(), Counter()
    hoisted = {"orig": set(), "recomp": set()}
    for hunk in entity.get("diff") or []:
        for block in hunk[1]:
            for side, rows in block.items():
                for row in rows:
                    # 'both' rows are [orig_addr, text, recomp_addr]; one-sided
                    # rows are [addr, text]
                    text = row[1]
                    h = HOIST.match(text.strip())
                    if h:
                        name = h.group(1).strip()
                        for who in (("orig", "recomp") if side == "both" else (side,)):
                            hoisted[who].add(name)
                        continue
                    m = CALL.match(text.strip())
                    if not m:
                        continue
                    tgt, hard = normalise(m.group(1))
                    if not hard:
                        tgt = "~" + tgt
                    if side == "both":
                        orig[tgt] += 1
                        recomp[tgt] += 1
                    elif side == "orig":
                        orig[tgt] += 1
                    elif side == "recomp":
                        recomp[tgt] += 1
    return orig, recomp, hoisted


def main():
    args = [a for a in sys.argv[1:] if not a.startswith("--")]
    flags = {a for a in sys.argv[1:] if a.startswith("--")}
    data = json.load(open(args[0]))["data"]

    sizes = {}
    try:
        sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
        import origsize
        addrs = [int(i["address"], 16) for i in data if i.get("address")]
        sizer = origsize.OrigSizes("/workspace/hover/hover.exe", known_starts=addrs)
        for a in addrs:
            sizes[a] = sizer.size(a) or 0
    except Exception:
        pass

    findings = []
    for it in data:
        if "matching" not in it or not it.get("diff"):
            continue
        orig, recomp, hoisted = collect(it)
        missing = orig - recomp
        extra = recomp - orig
        if not missing and not extra:
            continue
        # A hoisted import cancels a register call on the other side: VC keeps
        # a repeatedly-called import in a register, so `call edi` on one side
        # and the named import on the other are the same call, and reporting
        # the named one as missing is an artefact of suppressing the other.
        # An import whose address the other side loaded into a register is
        # called through that register there, so it is the same call.
        for who, side_set in (("recomp", missing), ("orig", extra)):
            for name in hoisted[who]:
                for t in [t for t in list(side_set) if name in t]:
                    del side_set[t]
        for a, b in ((missing, extra), (extra, missing)):
            slots = b.get("~call-through-register", 0)
            for t in [t for t in list(a) if t.startswith("dword ptr [->")]:
                if slots <= 0:
                    break
                take = min(slots, a[t])
                a[t] -= take
                slots -= take
                if a[t] == 0:
                    del a[t]
            if slots != b.get("~call-through-register", 0):
                b["~call-through-register"] = slots

        hard = any(not t.startswith("~") for t in list(missing) + list(extra))
        if not hard and "--all" not in flags:
            continue
        if "--all" not in flags:
            missing = Counter({t: n for t, n in missing.items() if not t.startswith("~")})
            extra = Counter({t: n for t, n in extra.items() if not t.startswith("~")})
        addr = int(it["address"], 16)
        findings.append((sizes.get(addr, 0), it["address"], it["name"],
                         it["matching"], missing, extra))

    findings.sort(key=lambda f: -f[0])
    if "--quiet" not in flags:
        for size, addr, name, ratio, missing, extra in findings:
            print("%s  %s  %d bytes, %.1f%% matched" % (addr, name, size, 100 * ratio))
            for t, n in sorted(missing.items()):
                print("    original calls, we do not:  %s%s" % (t, "" if n == 1 else " x%d" % n))
            for t, n in sorted(extra.items()):
                print("    we call, original does not: %s%s" % (t, "" if n == 1 else " x%d" % n))
    print("\n%d of %d compared functions differ in a comparable call target%s"
          % (len(findings), sum(1 for i in data if "matching" in i),
             "" if "--all" in flags else " (--all also shows register, frame and unnamed forms)"))


main()
