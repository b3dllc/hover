#!/usr/bin/env python3
"""Static data: does every object the code references hold the same bytes?

equiv.py checks what functions *do*; this checks what they read.  Every
matched function pair is disassembled on both sides and the two instruction
streams are aligned; each relocated operand (an absolute address) in the
original then pairs with the one at the same place in ours.  A pair is a
claim that the two addresses name the same object, so its bytes are
compared:

  - the extent runs to the next address anything references (a relocation
    target anywhere in the image, or a matched entity) on either side,
    except where both sides have such a boundary at the same offset and it
    is itself a known pair (an array element referenced on its own);
  - a dword that is a relocation on one side must be one on the other, and
    the two pointers must name the same thing: the same matched function,
    or an object pair that is then compared in turn (a vtable, a table of
    strings, a CRuntimeClass);
  - every other byte must be equal.  .bss reads as zeros.

A code reference is checked too: the two operands must name the same
matched function.  Matched GLOBAL entities are compared over their size.

    /workspace/tools/venv/bin/python tools/equiv/datadiff.py [-v] [--lib] [--json OUT]
"""
import argparse
import difflib
import json
import sys
from bisect import bisect_right
from collections import defaultdict
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import equiv  # noqa: E402
from analysis import FuncInfo  # noqa: E402

MAX_EXTENT = 0x10000


def _tok(ins, img):
    """Alignment token: the mnemonic and operands with registers erased
    (register allocation differs between the sides) and relocated values
    erased (the two images lay out differently); displacements and plain
    immediates stay, so reordered stores to different fields do not pair."""
    parts = [ins.mn]
    for o in ins.ops:
        if o[0] == "reg":
            parts.append("r")
        elif o[0] == "imm":
            parts.append("A" if img.inside(o[1]) else f"{o[1]:x}")
        else:
            disp = o[4]
            parts.append("mA" if img.inside(disp) else f"m{disp:x}/{o[6]}")
    return " ".join(parts)


def _stream(img, fi):
    out = []
    for a in sorted(fi.insns):
        ins = fi.insns[a]
        tg = [img.u32(a + k) for k in range(ins.size - 3) if a + k in img.relocs]
        out.append((_tok(ins, img), tg, a))
    return out


class DataDiff:
    def __init__(self, world, lib=False):
        self.w = world
        self.O, self.R = world.orig, world.ours
        self.lib = lib
        self.anch = [self._anchors(self.O, 0), self._anchors(self.R, 1)]
        self.pairs = {}          # (o, r) -> first provenance
        self.code_bad = []       # code references naming different functions
        self.imp_bad = []        # import-table references naming different imports
        self._code_memo = {}
        self.emulated = []
        self._acc_ready = False
        self.votes = defaultdict(int)
        self.prov = {}
        self.dropped = 0
        self.game_refs = set()
        self.unaligned = 0

    def _anchors(self, img, side):
        s = set()
        for a in img.relocs:
            v = img.u32(a)
            if v is not None and img.inside(v):
                s.add(v)
        ents = self.w.ent_o if side == 0 else self.w.ent_r
        for e in ents:
            s.add(e[0])
            if e[1] > 1:
                s.add(e[0] + e[1])
        for name, va, size, ch in img.sections:
            s.add(va)
            s.add(va + size)
        return sorted(s)

    def _next_anchor(self, side, a):
        lst = self.anch[side]
        i = bisect_right(lst, a)
        return lst[i] if i < len(lst) else a + MAX_EXTENT

    # -- pairing references from code
    def collect(self):
        w = self.w
        funcs = [(o, r, w.osize[o], w.rsize[r], w.oname.get(o, "")) for r, o in w.r2o_func.items()
                 if w.osize.get(o) and w.rsize.get(r)]
        libs = {o for o, r, *_ in w.funcs}
        for o, r, osz, rsz, name in sorted(funcs):
            is_lib = o not in libs
            if is_lib and not self.lib:
                continue
            fo, fr = FuncInfo(self.O, o, osz), FuncInfo(self.R, r, rsz)
            so, sr = _stream(self.O, fo), _stream(self.R, fr)
            for x in so:
                self.game_refs.update(x[1])
            sm = difflib.SequenceMatcher(None, [x[0] for x in so], [x[0] for x in sr], autojunk=False)
            for tag, i1, i2, j1, j2 in sm.get_opcodes():
                if tag == "equal":
                    for x, y in zip(so[i1:i2], sr[j1:j2]):
                        if len(x[1]) == len(y[1]):
                            for vo, vr in zip(x[1], y[1]):
                                if fo.inside(vo) and fr.inside(vr):
                                    continue        # a label (SEH funclet, switch table)
                                self.votes[(vo, vr)] += 1
                                self.prov.setdefault((vo, vr), ("code", o, x[2], y[2], name))
                elif any(x[1] for x in so[i1:i2]) or any(y[1] for y in sr[j1:j2]):
                    self.unaligned += 1
        # Equal tokens can still pair the wrong operands (`mov esi, [IAT]`
        # hoisted for different imports).  A pair counts only when it is the
        # best-supported partner of both its addresses; the rest are
        # alignment noise (a real wrong-global read is the emulator's job).
        best_o, best_r = {}, {}
        for (vo, vr), n in self.votes.items():
            if n > best_o.get(vo, (0,))[0]:
                best_o[vo] = (n, vr)
            if n > best_r.get(vr, (0,))[0]:
                best_r[vr] = (n, vo)
        for (vo, vr), n in sorted(self.votes.items()):
            if best_o[vo][1] != vr or best_r[vr][1] != vo:
                self.dropped += 1
                continue
            self._ref(vo, vr, self.prov[(vo, vr)])

    def _thru(self, img, a, matched):
        """Follow an unmatched bare `jmp rel32` (a forwarding override such
        as hover.exe's CSub424720::DestroyWindow, which only jumps to
        CWnd::DestroyWindow) to the function it forwards to."""
        for _ in range(4):
            if a in matched or img.read(a, 1) != b"\xe9":
                break
            a = (a + 5 + int.from_bytes(img.read(a + 1, 4), "little", signed=True)) & 0xFFFFFFFF
        return a

    def _same_code(self, vo, vr, fo=None, fr=None):
        w = self.w
        if fo is not None and fo.inside(vo) and fr.inside(vr):
            return True                     # a label inside the function (SEH, switch)
        vo, vr = self._thru(self.O, vo, w.o2r_func), self._thru(self.R, vr, w.r2o_func)
        if vr in w.r2o_func:
            return w.r2o_func[vr] == vo
        if vo in w.o2r_func:
            return False
        ti_o, ti_r = w.thunk_o.get(vo), w.thunk_r.get(vr)
        if ti_o or ti_r:
            return ti_o == ti_r
        return self._same_unmatched(vo, vr)

    def _same_unmatched(self, vo, vr):
        """Neither side has a match (a compiler-generated destructor, a
        per-file copy of an inline function).  Both are walked straight-line
        to their ret; the instructions must be the same multiset of bytes
        outside relocations and call displacements (the scheduler may order
        them differently - a vptr store moved past the SEH setup), local
        labels must be local on both sides, and the calls must name the same
        targets in the same order (recursively)."""
        key = (vo, vr)
        if key in self._code_memo:
            return self._code_memo[key]
        self._code_memo[key] = True          # assume equal on recursion
        a_side = self._walk(self.O, vo)
        b_side = self._walk(self.R, vr)
        ok = a_side is not None and b_side is not None
        if ok:
            (ta, ca, da, ia), (tb, cb, db, ib) = a_side, b_side
            ok = sorted(ta) == sorted(tb) and len(ca) == len(cb) and ia == ib \
                and all(self._same_code(x, y) for x, y in zip(ca, cb))
            if ok:
                for po, pr in zip(da, db):
                    self._pair(po, pr, ("data", vo, vr))
        self._code_memo[key] = ok
        return ok

    def _walk(self, img, start):
        from analysis import decode
        toks, calls, data, imps = [], [], [], []
        a = start
        for _ in range(256):
            x = decode(img, a)
            if x is None:
                return None
            raw = bytearray(img.read(a, x.size))
            for k in range(x.size - 3):
                if (a + k) in img.relocs:
                    p = img.u32(a + k)
                    if img.in_code(p):
                        if start <= p < start + 0x400:
                            raw[k:k + 4] = b"LLLL"
                        else:
                            calls.append(p)
                            raw[k:k + 4] = b"CCCC"
                    elif p in img.iat:
                        imps.append(img.iat[p].upper())
                        raw[k:k + 4] = b"IIII"
                    else:
                        data.append(p)
                        raw[k:k + 4] = b"DDDD"
            if raw[:1] in (b"\xe8", b"\xe9") and x.ops and x.ops[0][0] == "imm":
                t = x.ops[0][1]
                if x.mn == "call" or not (start <= t < start + 0x400):
                    calls.append(t)
                    raw[1:] = b"C" * (x.size - 1)
                else:
                    raw[1:] = b"L" * (x.size - 1)
            elif x.mn.startswith("j") and x.ops and x.ops[0][0] == "imm":
                raw[1:] = b"L" * (x.size - 1)   # a short branch: scheduling moves it
            toks.append(bytes(raw))
            if x.mn in ("ret", "int3") or (x.mn == "jmp" and not raw[1:2] == b"L"):
                break
            a = x.next
        return toks, calls, data, imps

    def _emulated(self, vo, vr):
        """Two unmatched functions that differ in shape (the original's /Ob2
        inlined a base destructor into a deleting destructor, say): let the
        differential emulator decide."""
        w = self.w
        if vo in w.o2r_func or vr in w.r2o_func:
            return False
        key = ("emu", vo, vr)
        if key in self._code_memo:
            return self._code_memo[key]
        osz = min(w.next_start(0, vo) - vo, 0x400)
        rsz = min(w.next_start(1, vr) - vr, 0x400)
        equiv._WORLD = w
        # the return type is unknown: MFC 3's CRuntimeClass::m_pfnConstruct
        # is void, and a deleting destructor's result is never used
        w.rettype.setdefault(vo, "void")
        res = equiv._worker((vo, vr, osz, rsz, f"unmatched {vo:#x}", 32))
        ok = not res.get("error") and not res.get("diffs") and res.get("runs", 0) >= 8
        self.emulated.append((vo, vr, ok, res))
        self._code_memo[key] = ok
        return ok

    def _imp(self, a):
        n = self.O.iat.get(a)
        return n.upper() if n else None

    def _imp_r(self, a):
        n = self.R.iat.get(a)
        return n.upper() if n else None

    def _pair(self, po, pr, prov):
        if (po, pr) not in self.pairs:
            self.pairs[(po, pr)] = prov
            if hasattr(self, "queue"):
                self.queue.append((po, pr))

    def _ref(self, vo, vr, prov, fo=None, fr=None):
        O, R = self.O, self.R
        co, cr = O.in_code(vo), R.in_code(vr)
        if co or cr:
            if not (co and cr and (self._same_code(vo, vr, fo, fr) or self._emulated(vo, vr))):
                self.code_bad.append((vo, vr, prov))
            return
        if vo in O.iat or vr in R.iat:
            if self._imp(vo) != self._imp_r(vr):
                self.imp_bad.append((vo, vr, prov))
            return
        self._pair(vo, vr, prov)

    # -- comparing contents
    def _byte(self, img, a):
        if img.section(a) == ".bss":
            return 0
        return img.read(a, 1)[0]

    def extent(self, vo, vr):
        """(bytes to compare, offset of the last paired boundary merged in)."""
        known = self.pairs
        L = 0
        while L < MAX_EXTENT:
            no = self._next_anchor(0, vo + L) - vo
            nr = self._next_anchor(1, vr + L) - vr
            if no == nr and (vo + no, vr + nr) in known \
                    and self.O.section(vo + no) == self.O.section(vo):
                L = no
                continue
            return min(no, nr, MAX_EXTENT), L
        return MAX_EXTENT, L

    # -- how the code touches each address
    def _accesses(self, img, funcs):
        """target -> widest plain [disp] access, or None when the address is
        ever used otherwise (taken as a value, indexed, or stored in data):
        then the object may extend past what any one access shows."""
        acc = {}
        covered = set()
        for start, size in funcs:
            fi = FuncInfo(img, start, size)
            for a, ins in fi.insns.items():
                for k in range(ins.size - 3):
                    if (a + k) not in img.relocs:
                        continue
                    covered.add(a + k)
                    t = img.u32(a + k)
                    plain = [o for o in ins.ops if o[0] == "mem" and o[4] == t and o[1] is None and o[2] is None]
                    if plain and t in acc and acc[t] is None:
                        continue
                    if plain and ins.mn not in ("lea",):
                        acc[t] = max(acc.get(t) or 0, plain[0][6])
                    else:
                        acc[t] = None
        for a in img.relocs:
            if a not in covered:
                acc[img.u32(a)] = None
        return acc

    def scalar_size(self, vo, vr):
        if not hasattr(self, "_acc"):
            w = self.w
            fo = [(o, w.osize[o]) for o in w.o2r_func if w.osize.get(o)]
            fr = [(r, w.rsize[r]) for r in w.r2o_func if w.rsize.get(r)]
            self._acc = (self._accesses(self.O, fo), self._accesses(self.R, fr))
        a, b = self._acc[0].get(vo, None), self._acc[1].get(vr, None)
        if a is None or b is None:
            return None
        return max(a, b)

    def compare(self, vo, vr, L):
        """Mismatching ranges [(off, n, kind)] of the L bytes at vo / vr."""
        O, R = self.O, self.R
        bad = []
        k = 0
        while k < L:
            ro, rr = (vo + k) in O.relocs, (vr + k) in R.relocs
            if ro or rr:
                if not (ro and rr) or k + 4 > L:
                    bad.append((k, 4, "ptr/value"))
                else:
                    po, pr = O.u32(vo + k), R.u32(vr + k)
                    co, cr = O.in_code(po), R.in_code(pr)
                    if co or cr:
                        if not (co and cr and (self._same_code(po, pr) or self._emulated(po, pr))):
                            bad.append((k, 4, "code ptr"))
                    elif po in O.iat or pr in R.iat:
                        if self._imp(po) != self._imp_r(pr):
                            bad.append((k, 4, "import"))
                    else:
                        self._pair(po, pr, ("data", vo + k, vr + k))
                k += 4
                continue
            if self._byte(O, vo + k) != self._byte(R, vr + k):
                if bad and bad[-1][2] == "bytes" and bad[-1][0] + bad[-1][1] == k:
                    bad[-1] = (bad[-1][0], bad[-1][1] + 1, "bytes")
                else:
                    bad.append((k, 1, "bytes"))
            k += 1
        return bad

    def _natural1(self, img, a, L):
        """How much of the L bytes at a is plainly one object: a run of code
        pointers (a vtable, a message-map's handlers end elsewhere), or a
        NUL-terminated string padded to a dword."""
        if a in img.relocs and img.in_code(img.u32(a) or 0):
            n = 0
            while n < L and (a + n) in img.relocs and img.in_code(img.u32(a + n)):
                n += 4
            return n
        if img.section(a) == ".bss":
            return L
        raw = img.read(a, L)
        if any((a + k) in img.relocs for k in range(0, min(L, 48), 4)):
            # a table of AFX_MSGMAP_ENTRY (24 bytes) ends at its all-zero entry
            for k in range(0, L - 23, 24):
                if raw[k:k + 24] == bytes(24):
                    return k + 24
        end = raw.find(b"\0")
        if end >= 1 and all(32 <= c < 127 or c in (9, 10, 13) for c in raw[:end]) \
                and not any((a + k) in img.relocs for k in range(end)):
            return (end + 4) & ~3
        return L

    def natural(self, vo, vr, L, merged=0):
        n = max(self._natural1(self.O, vo, L), self._natural1(self.R, vr, L))
        if self._acc_ready:
            # a global the code only ever loads or stores whole: its size
            # is the widest access (of the last element merged in)
            last = (vo + merged, vr + merged)
            sz = self.scalar_size(*last)
            if sz is not None:
                n = min(n, merged + sz)
        return min(L, n)

    def run(self):
        self.collect()
        for o, r, size in self.w.data:
            # a global no game function names is the library's (CRT tables)
            if not self.lib and o not in self.game_refs:
                continue
            if not self.O.in_code(o) and (o, r) not in self.pairs:
                self.pairs[(o, r)] = ("entity", self.w.oname.get(o, ""))
        # an address game code names that no aligned instruction paired:
        # translate it through the matched global holding it, if any
        for v in sorted(self.game_refs):
            if not self.O.inside(v) or self.O.in_code(v) or v in self.O.iat:
                continue
            t = self.w.o2r_data(v)
            if t is not None and (v, t) not in self.pairs and v not in self._paired_o():
                self.pairs[(v, t)] = ("entity", self.name_o(v))
        # still unpaired: find ours by content (a vtable by its translated
        # slots, a string or constant by its bytes - equiv.World.r2o), over
        # every address our image's relocations name
        left = {v for v in self.game_refs if self.O.inside(v) and not self.O.in_code(v)
                and v not in self.O.iat} - {vo for vo, vr in self.pairs}
        if left:
            back = defaultdict(set)
            for a in self.R.relocs:
                t = self.R.u32(a)
                if t is not None and self.R.inside(t) and not self.R.in_code(t) and t not in self.R.iat:
                    o = self.w.r2o(t)
                    if o in left:
                        back[o].add(t)
            for o, ts in back.items():
                if len(ts) == 1:
                    self.pairs[(o, ts.pop())] = ("content", self.name_o(o))
        self.queue = list(self.pairs)
        self.results = []
        self.spans = []
        done = set()
        self._acc_ready = True
        while self.queue:
            vo, vr = self.queue.pop()
            if (vo, vr) in done:
                continue
            done.add((vo, vr))
            if self.O.section(vo) == ".bss" and self.R.section(vr) == ".bss":
                self.spans.append((vo, 1))
                continue
            L, merged = self.extent(vo, vr)
            self.spans.append((vo, L))
            bad = self.compare(vo, vr, L)
            if bad:
                nat = self.natural(vo, vr, L, merged)
                core = [b for b in bad if b[0] < nat]
                self.results.append({"orig": vo, "recomp": vr, "len": L, "nat": nat, "bad": bad,
                                     "tail": not core, "prov": self.pairs[(vo, vr)]})
        self.results.sort(key=lambda x: x["orig"])

    def _paired_o(self):
        if not hasattr(self, "_po"):
            self._po = {vo for vo, vr in self.pairs}
        return self._po

    # -- reporting
    def name_o(self, a):
        w = self.w
        e = w.entity(0, a)
        if e:
            n = w.oname.get(e[0], "")
            return n + (f"+{a - e[0]:#x}" if a != e[0] else "")
        return ""

    def show(self, verbose, tails=False):
        O, R = self.O, self.R
        for x in self.results:
            if x["tail"] and not tails:
                continue
            vo, vr, L = x["orig"], x["recomp"], x["len"]
            prov = x["prov"]
            if prov[0] == "code":
                src = f"from {prov[4] or hex(prov[1])} @{prov[2]:#x}"
            elif prov[0] == "data":
                src = f"via pointer at {prov[1]:#x}"
            else:
                src = f"entity {prov[1]}"
            nb = sum(b[1] for b in x["bad"] if tails or b[0] < x["nat"])
            print(f"{vo:#x} {O.section(vo)} <-> {vr:#x} {R.section(vr)} len {L:#x}: "
                  f"{nb} bytes differ {self.name_o(vo)} ({src})")
            if verbose:
                for off, n, kind in [b for b in x["bad"] if tails or b[0] < x["nat"]][:8]:
                    a = O.read(vo + off, min(n, 16)) if O.section(vo) != ".bss" else b"\0" * min(n, 16)
                    b = R.read(vr + off, min(n, 16)) if R.section(vr) != ".bss" else b"\0" * min(n, 16)
                    print(f"    +{off:#x} {kind:9} orig {a.hex(' ')}  ours {b.hex(' ')}")
        for vo, vr, prov in self.code_bad:
            print(f"code ref {vo:#x} {self.w.oname.get(vo, '')!r} <-> {vr:#x} "
                  f"{self.w.oname.get(self.w.r2o_func.get(vr), '') or self.w.rname.get(vr, '')!r} "
                  f"in {prov[4] or hex(prov[1])} @{prov[2]:#x}/{prov[3]:#x}")
        for vo, vr, ok, res in self.emulated:
            print(f"unmatched code {vo:#x} <-> {vr:#x}: emulated {res.get('runs', 0)} runs, "
                  f"{'equivalent' if ok else 'DIFF ' + str(res.get('first') or res.get('error'))[:120]}")
        for vo, vr, prov in self.imp_bad:
            print(f"import ref {self._imp(vo)} <-> {self._imp_r(vr) or hex(vr)} "
                  f"in {prov[4] or hex(prov[1])} @{prov[2]:#x}/{prov[3]:#x}")
        O = self.O
        want = {v for v in self.game_refs if O.inside(v) and not O.in_code(v) and v not in O.iat}
        got = {vo for vo, vr in self.pairs}
        # an address inside a compared object is covered too
        spans = sorted((vo, vo + n) for vo, n in self.spans)
        from bisect import bisect_right as _br
        starts = [x[0] for x in spans]

        def covered(v):
            if v in got:
                return True
            i = _br(starts, v) - 1
            return i >= 0 and v < spans[i][1]
        miss = sorted(v for v in want if not covered(v))
        self.missing = miss
        core = sum(1 for x in self.results if not x["tail"])
        print(f"\n{len(want) - len(miss)}/{len(want)} data addresses game code names are compared"
              + (f"; never paired: {' '.join(hex(v) for v in miss[:12])}{' ...' if len(miss) > 12 else ''}"
                 if miss else ""))
        print(f"{len(self.pairs)} object pairs, {core} differ (+{len(self.results) - core} only past "
              f"the object's natural end); {len(self.code_bad)} code and {len(self.imp_bad)} import "
              f"references disagree; {self.unaligned} unaligned hunks, {self.dropped} weaker pairings dropped")


def main():
    ap = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    ap.add_argument("-v", "--verbose", action="store_true")
    ap.add_argument("--lib", action="store_true", help="include library (MFC/CRT) functions")
    ap.add_argument("--tails", action="store_true",
                    help="also show differences past a vtable's or string's end")
    ap.add_argument("--json")
    a = ap.parse_args()
    world = equiv.World()
    world.setup_imports()
    d = DataDiff(world, lib=a.lib)
    d.run()
    d.show(a.verbose, a.tails)
    if a.json:
        Path(a.json).write_text(json.dumps({"diffs": d.results, "code": d.code_bad}, indent=1))


if __name__ == "__main__":
    main()
