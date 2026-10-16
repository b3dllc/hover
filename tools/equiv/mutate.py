#!/usr/bin/env python3
"""Mutation testing: how good is equiv.py at catching a wrong function?

A clean "equivalent" verdict is only as strong as the harness's ability to
notice a difference.  This measures it: small bugs are planted directly in
our compiled function (build/hover.exe, patched in memory - no rebuild), and
each mutant is checked against the original exactly as equiv.py checks the
real function.  A mutant the harness reports as divergent is *killed*; one
it passes *survives*.

Mutants, each a single instruction in a block the unmutated check covered:

  bound   a conditional jump moved across its boundary (jl <-> jle,
          jb <-> jbe, ...): `<` written for `<=`
  negate  a conditional jump inverted (je <-> jne, ...)
  imm     an immediate operand changed by one (a constant, an argument)
  store   a store to non-stack memory removed (a dropped assignment)
  field   a memory operand's displacement moved by 4 (the wrong member)
  swap    two adjacent pushes before a call exchanged (argument order)

A survivor is not necessarily a harness miss: some mutants are equivalent
(a boundary the values never reach, a store the function overwrites).

    /workspace/tools/venv/bin/python tools/equiv/mutate.py [--per N] [-j N] [--json OUT] [0xADDR ...]
"""
import argparse
import json
import os
import random
import sys
from bisect import bisect_right
from collections import Counter, defaultdict
from pathlib import Path

import capstone

sys.path.insert(0, str(Path(__file__).resolve().parent))
import equiv  # noqa: E402

_cs = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
_cs.detail = True

BOUND = {0x7C: 0x7E, 0x7E: 0x7C, 0x7D: 0x7F, 0x7F: 0x7D, 0x72: 0x76, 0x76: 0x72, 0x73: 0x77, 0x77: 0x73}
STACK = ("esp", "ebp")


def _mem_ops(ci):
    out = []
    for o in ci.operands:
        if o.type == capstone.x86.X86_OP_MEM:
            out.append((ci.reg_name(o.mem.base) if o.mem.base else None,
                        ci.reg_name(o.mem.index) if o.mem.index else None, o.mem.disp))
    return out


def mutants(img, fi, covered):
    """Every applicable single-instruction mutant: (kind, addr, patch bytes at addr)."""
    blocks = sorted(fi.blocks)
    cov = set(covered)

    def is_cov(a):
        i = bisect_right(blocks, a) - 1
        return i >= 0 and blocks[i] in cov

    out = []
    addrs = sorted(fi.insns)
    for n, a in enumerate(addrs):
        ins = fi.insns[a]
        if not is_cov(a):
            continue
        raw = bytearray(img.read(a, ins.size))
        relocs = {k for k in range(ins.size) if any(a + k - j in img.relocs for j in range(4))}
        ci = next(_cs.disasm(bytes(raw), a), None)
        if ci is None:
            continue
        mn = ci.mnemonic
        # conditional jumps
        if mn.startswith("j") and mn != "jmp":
            if raw[0] in BOUND:
                p = bytearray(raw); p[0] = BOUND[raw[0]]
                out.append(("bound", a, bytes(p)))
            elif raw[0] == 0x0F and raw[1] - 0x10 in BOUND:
                p = bytearray(raw); p[1] = BOUND[raw[1] - 0x10] + 0x10
                out.append(("bound", a, bytes(p)))
            if 0x70 <= raw[0] <= 0x7F:
                p = bytearray(raw); p[0] ^= 1
                out.append(("negate", a, bytes(p)))
            elif raw[0] == 0x0F and 0x80 <= raw[1] <= 0x8F:
                p = bytearray(raw); p[1] ^= 1
                out.append(("negate", a, bytes(p)))
            continue
        # SEH frame bookkeeping (registration in fs:[0], the -1 try level
        # pushed under the handler) only matters when an exception unwinds
        if any(o.type == capstone.x86.X86_OP_MEM and o.mem.segment for o in ci.operands):
            continue
        if mn == "push" and raw[:2] == b"\x6a\xff" and n + 1 < len(addrs) and img.read(addrs[n + 1], 1) == b"\x68":
            continue
        if mn in ("call", "jmp"):
            continue
        mems = _mem_ops(ci)
        on_stack = any(b in STACK for b, i, d in mems)
        regs_w = [ci.reg_name(o.reg) for o in ci.operands if o.type == capstone.x86.X86_OP_REG]
        # immediates (not relocated addresses, not stack-pointer arithmetic,
        # not the SEH try-level at [ebp-4])
        if ci.imm_size and ci.imm_offset and not (relocs & set(range(ci.imm_offset, ci.imm_offset + ci.imm_size))):
            if not ("esp" in regs_w and mn in ("add", "sub")) and not (on_stack and mems and mems[0][2] == -4):
                k, sz = ci.imm_offset, ci.imm_size
                v = int.from_bytes(raw[k:k + sz], "little")
                p = bytearray(raw)
                p[k:k + sz] = ((v + 1) % (1 << (8 * sz))).to_bytes(sz, "little")
                out.append(("imm", a, bytes(p)))
        # stores to non-stack memory
        if mn in ("mov", "movsx", "movzx") and mems and ci.operands[0].type == capstone.x86.X86_OP_MEM \
                and not on_stack:
            out.append(("store", a, b"\x90" * ins.size))
        # the wrong field
        if mems and not on_stack and mems[0][0] is not None and ci.disp_size == 1 and ci.disp_offset \
                and mn not in ("lea",):
            k = ci.disp_offset
            d = raw[k]
            if d + 4 < 0x80:
                p = bytearray(raw); p[k] = d + 4
                out.append(("field", a, bytes(p)))
        # argument order: two adjacent pushes of equal length, different bytes
        if mn == "push" and n + 1 < len(addrs):
            b = addrs[n + 1]
            nxt = fi.insns[b]
            # (arguments only: the pushes must run straight into a call)
            k = n + 2
            while k < len(addrs) and fi.insns[addrs[k]].mn == "push":
                k += 1
            feeds_call = k < len(addrs) and fi.insns[addrs[k]].mn == "call"
            if b == a + ins.size and nxt.mn == "push" and nxt.size == ins.size and is_cov(b) and feeds_call:
                rb = img.read(b, nxt.size)
                if rb != bytes(raw) and not relocs and not any(b + k in img.relocs for k in range(nxt.size)):
                    out.append(("swap", a, rb + bytes(raw)))
    return out


_W = None


def _work(args):
    global _W
    o, r, osz, rsz, name, per, runs, seed = args
    if _W is None:
        _W = equiv.World()
        _W.setup_imports()
        equiv._WORLD = _W
    w = _W
    try:
        base = equiv.Pair(w, o, r, osz, rsz, name)
        res0 = equiv.check(w, base, runs)
    except Exception as e:
        return {"orig": o, "name": name, "error": f"{type(e).__name__}: {e}"}
    if res0.get("diffs") or res0.get("error") or res0.get("runs", 0) < 8:
        return {"orig": o, "name": name, "skipped": "baseline not clean"}
    cands = mutants(w.ours, base.fr, base.covered[1])
    rng = random.Random(seed ^ o)
    bykind = defaultdict(list)
    for m in cands:
        bykind[m[0]].append(m)
    picks = []
    kinds = sorted(bykind)
    rng.shuffle(kinds)
    # spread the budget over the kinds this function offers
    while len(picks) < per and any(bykind[k] for k in kinds):
        for k in kinds:
            if bykind[k] and len(picks) < per:
                picks.append(bykind[k].pop(rng.randrange(len(bykind[k]))))
    out = []
    mem = w.ours.mem
    for kind, a, patch in picks:
        off = a - w.ours.base
        saved = bytes(mem[off:off + len(patch)])
        mem[off:off + len(patch)] = patch
        w.ours._sig.clear()
        w._pop_cache.clear()
        try:
            pair = equiv.Pair(w, o, r, osz, rsz, name)
            res = equiv.check(w, pair, runs, early=True)
            verdict = "error" if res.get("error") else ("killed" if res.get("diffs") else "survived")
            first = res.get("first")
        except Exception as e:
            verdict, first = "error", f"{type(e).__name__}: {e}"
        finally:
            mem[off:off + len(patch)] = saved
            w.ours._sig.clear()
            w._pop_cache.clear()
        out.append({"kind": kind, "addr": a, "patch": patch.hex(), "orig_bytes": saved.hex(),
                    "verdict": verdict, "first": first})
    return {"orig": o, "name": name, "mutants": out, "available": len(cands)}


def main():
    ap = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    ap.add_argument("addrs", nargs="*")
    ap.add_argument("--per", type=int, default=6, help="mutants per function")
    ap.add_argument("--runs", type=int, default=64)
    ap.add_argument("--seed", type=int, default=1)
    ap.add_argument("-j", "--jobs", type=int, default=os.cpu_count())
    ap.add_argument("--json")
    ap.add_argument("--all", action="store_true", help="include functions that already match 100%%")
    a = ap.parse_args()
    world = equiv.World()
    want = {int(x, 16) for x in a.addrs}
    acc = {}
    cmp_json = equiv.ROOT / "build" / "compare.json"
    if cmp_json.exists():
        for x in json.loads(cmp_json.read_text())["data"]:
            acc[int(x["address"], 16)] = x["matching"]
    todo = []
    for o, r, osz, rsz, name in world.funcs:
        if want and o not in want:
            continue
        if not want and not a.all and acc.get(o, 0) >= 1.0:
            continue
        if osz and rsz:
            todo.append((o, r, osz, rsz, name, a.per, a.runs, a.seed))
    todo.sort()
    results = []
    tally = Counter()
    bykind = defaultdict(Counter)

    def show(res):
        results.append(res)
        ms = res.get("mutants") or []
        for m in ms:
            tally[m["verdict"]] += 1
            bykind[m["kind"]][m["verdict"]] += 1
        s = " ".join(f"{m['kind']}:{m['verdict'][0].upper()}" for m in ms)
        print(f"{res['orig']:#x} {s or res.get('skipped') or res.get('error', '')}  {res['name']}", flush=True)

    if a.jobs > 1 and len(todo) > 1:
        import multiprocessing as mp
        with mp.Pool(a.jobs) as pool:
            for res in pool.imap_unordered(_work, todo, chunksize=1):
                show(res)
    else:
        for t in todo:
            show(_work(t))
    print()
    for k in sorted(bykind):
        c = bykind[k]
        n = c["killed"] + c["survived"]
        print(f"  {k:7} {c['killed']:4}/{n:<4} killed ({100 * c['killed'] / max(n, 1):.1f}%)"
              + (f", {c['error']} errors" if c["error"] else ""))
    n = tally["killed"] + tally["survived"]
    print(f"{tally['killed']}/{n} mutants killed ({100 * tally['killed'] / max(n, 1):.1f}%), "
          f"{tally['error']} harness errors, over {sum(1 for r in results if r.get('mutants'))} functions")
    if a.json:
        results.sort(key=lambda x: x["orig"])
        Path(a.json).write_text(json.dumps(results, indent=1))


if __name__ == "__main__":
    main()
