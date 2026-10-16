#!/usr/bin/env python3
"""Flow and type invariants: signedness, loops, and immediate arguments by
position.  Third companion to callcheck.py (rule 353) and opcheck.py (#196).

Properties compared per function, all independent of register allocation:

  jcc:<class>   conditional jumps by class - signed (jl/jge/jle/jg),
                unsigned (jb/jae/jbe/ja), equality (je/jne), flag (js/jo/jc..).
                The compiler swaps arms and operands freely but NEVER turns a
                signed compare into an unsigned one: a class-count difference
                is a type difference in the source (rule 10).
  sign:<op>     sar/shr, idiv/div, imul/mul, movsx/movzx, cdq/cwde/cbw, and
                setcc by class - the same argument: a signed shift, divide or
                extension is a fact about the operand's type.
  loop          conditional jumps with a negative relative displacement:
                loop back-edges (unconditional ones are unwind funclets).
                A different count is a loop the original has that we lack, or
                unrolling/tail duplication (rules 281, 376).
  arg:<callee>:<i>=<imm>
                each immediate pushed for a call, by argument position (0 =
                the push nearest the call = first argument).  A swapped or
                wrong constant argument is one of the classic bug shapes here
                (rule 140's Sub_410F20(detail,3) crash).  Register pushes are
                invisible; the original zeroing a register once and pushing it
                four times is a known noise class (rule 377).  The push
                accumulator resets on call, stack adjustment, jmp and ret.

    tools/compare.py --json /tmp/x.json && tools/flowcheck.py /tmp/x.json
        [--only jcc|sign|loop|arg] [--counts] [--summary] [0xADDR ...]

Default output is presence mode (a key one side has and the other lacks
entirely); --counts also reports count differences, which for jcc and sign
classes are meaningful and for arg are usually CSE.
"""
import json
import os
import re
import sys
from collections import Counter

JCC = {"jl": "signed", "jge": "signed", "jle": "signed", "jg": "signed",
       "jb": "unsigned", "jae": "unsigned", "jbe": "unsigned", "ja": "unsigned",
       "jnb": "unsigned", "jna": "unsigned",
       "je": "eq", "jne": "eq", "jz": "eq", "jnz": "eq",
       "js": "flag", "jns": "flag", "jo": "flag", "jno": "flag", "jc": "flag",
       "jnc": "flag", "jp": "flag", "jnp": "flag"}
SETCC = {"setl": "signed", "setge": "signed", "setle": "signed", "setg": "signed",
         "setb": "unsigned", "setae": "unsigned", "setbe": "unsigned", "seta": "unsigned",
         "sete": "eq", "setne": "eq"}
SIGNOPS = {"sar", "shr", "idiv", "div", "imul", "mul", "movsx", "movzx", "cdq", "cwde", "cbw"}
IMM = re.compile(r"^-?(0x[0-9a-f]+|\d+)$")
REL = re.compile(r"^(j[a-z]+)\s+(-?0x[0-9a-f]+)$")
CALL = re.compile(r"^call\s+(.*)$")
PUSH = re.compile(r"^push\s+(.*)$")
NAMED = re.compile(r"^(.*?)\s*\((STRING|DATA|FLOAT|POINTER|VTABLE)\)$")


def callee(t):
    t = t.strip()
    t = re.sub(r"\s*\((FUNCTION|THUNK|LABEL|IMPORT)\)$", "", t)
    if re.fullmatch(r"[a-z]{2,3}", t):
        return "reg"
    if "[" in t and ("ebp" in t or "esp" in t):
        return "local"
    return t


def props(rows):
    """rows: list of instruction texts in order for one side. Returns Counter."""
    c = Counter()
    pushes = []
    for text in rows:
        t = text.strip()
        mn = t.split(None, 1)[0] if t else ""
        rest = t[len(mn):].strip()
        if mn in JCC:
            c["jcc:" + JCC[mn]] += 1
        if mn in SETCC:
            c["sign:set" + SETCC[mn]] += 1
        if mn in SIGNOPS:
            c["sign:" + mn] += 1
        m = REL.match(t)
        if m and m.group(2).startswith("-") and m.group(1) != "jmp":
            # only CONDITIONAL back-edges: an unconditional backward jmp is
            # almost always an exception-unwind funclet or a shared epilogue
            c["loop"] += 1
        m = PUSH.match(t)
        if m:
            arg = m.group(1).strip()
            nm = NAMED.match(arg)
            if nm:
                pushes.append(nm.group(1).strip())
            elif IMM.match(arg):
                v = int(arg, 16) if "0x" in arg else int(arg)
                pushes.append("%#x" % (v & 0xffffffff) if v < 0 else "%#x" % v)
            else:
                pushes.append(None)
            continue
        m = CALL.match(t)
        if m:
            cal = callee(m.group(1))
            for i, p in enumerate(reversed(pushes)):
                if p is not None:
                    c["arg:%s:%d=%s" % (cal, i, p)] += 1
            pushes = []
            continue
        if (mn in ("add",) and rest.startswith("esp")) or mn in ("jmp", "ret"):
            # a stack adjustment ends an argument list; so does an
            # unconditional transfer - a branch that reaches a shared call
            # tail by jmp must not carry its pushes into the next call's
            # count (rule 385: tail-duplicated branches accumulated)
            pushes = []
    return c


def collect(entity):
    o_rows, r_rows = [], []
    for hunk in entity.get("diff") or []:
        for block in hunk[1]:
            for side, rows in block.items():
                for row in rows:
                    if side in ("both", "orig"):
                        o_rows.append(row[1])
                    if side in ("both", "recomp"):
                        r_rows.append(row[1])
    return props(o_rows), props(r_rows)


def main():
    argv = sys.argv[1:]
    only = None
    if "--only" in argv:
        i = argv.index("--only"); only = argv[i + 1]; del argv[i:i + 2]
    flags = {a for a in argv if a.startswith("--")}
    args = [a for a in argv if not a.startswith("--")]
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
    findings, cat = [], Counter()
    for it in data:
        if "matching" not in it or not it.get("diff"):
            continue
        addr = int(it["address"], 16)
        if want and addr not in want:
            continue
        orig, recomp = collect(it)
        missing, extra = orig - recomp, recomp - orig
        if "--counts" not in flags:
            missing = Counter({k: v for k, v in missing.items() if recomp.get(k, 0) == 0 or k.startswith(("jcc:", "sign:", "loop"))})
            extra = Counter({k: v for k, v in extra.items() if orig.get(k, 0) == 0 or k.startswith(("jcc:", "sign:", "loop"))})
        if only:
            missing = Counter({k: v for k, v in missing.items() if k.startswith(only)})
            extra = Counter({k: v for k, v in extra.items() if k.startswith(only)})
        if not missing and not extra:
            continue
        for k in list(missing) + list(extra):
            cat[k.split(":")[0]] += 1
        findings.append((sizes.get(addr, 0), it["address"], it["name"], it["matching"], missing, extra))
    findings.sort(key=lambda f: -f[0])
    if "--summary" in flags:
        print("functions flagged: %d" % len(findings))
        for c, n in cat.most_common():
            print("  %-5s %d" % (c, n))
        return
    for size, a, name, ratio, missing, extra in findings:
        print("%s  %s  %d bytes, %.1f%% matched" % (a, name, size, 100 * ratio))
        for k, n in sorted(missing.items()):
            print("    original has, we do not:  %s%s" % (k, "" if n == 1 else " x%d" % n))
        for k, n in sorted(extra.items()):
            print("    we have, original does not: %s%s" % (k, "" if n == 1 else " x%d" % n))
    print("\n%d of %d compared functions differ" % (len(findings), sum(1 for i in data if "matching" in i)))


main()
