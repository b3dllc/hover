"""Static analysis of one function for the differential emulator.

Recursive descent over the function's bytes gives its instructions and basic
blocks.  A stack-depth dataflow then fixes, for every call site, how many
argument bytes the callee pops (`ret n`).  The emulator needs that to resume
the caller correctly after stubbing a call.  Direct callees and Win32 imports
have a known pop count; for indirect calls (virtual methods, function
pointers) the pop count is solved so that the depth agrees at every join
point and is zero at every `ret`.
"""
import capstone
from capstone import x86_const as X

_cs = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
_cs.detail = True

UNKNOWN = object()


class Insn:
    __slots__ = ("addr", "size", "mn", "ops", "op_str", "next", "succ",
                 "kind", "target", "mem", "imm", "regs")

    def __init__(self, ci):
        self.addr = ci.address
        self.size = ci.size
        self.mn = ci.mnemonic
        self.op_str = ci.op_str
        self.next = ci.address + ci.size
        self.ops = []
        for o in ci.operands:
            if o.type == X.X86_OP_REG:
                self.ops.append(("reg", ci.reg_name(o.reg)))
            elif o.type == X.X86_OP_IMM:
                self.ops.append(("imm", o.imm & 0xFFFFFFFF))
            elif o.type == X.X86_OP_MEM:
                m = o.mem
                self.ops.append((
                    "mem",
                    ci.reg_name(m.base) if m.base else None,
                    ci.reg_name(m.index) if m.index else None,
                    m.scale,
                    m.disp & 0xFFFFFFFF,
                    ci.reg_name(m.segment) if m.segment else None,
                    o.size,
                ))
        self.succ = []
        self.kind = None
        self.target = None


def decode(img, addr):
    data = img.read(addr, 16)
    ci = next(_cs.disasm(data, addr), None)
    return Insn(ci) if ci is not None else None


class FuncInfo:
    """Instructions, blocks and per-call-site stack facts of one function."""

    def __init__(self, img, start, size):
        self.img = img
        self.start = start
        self.end = start + size
        self.insns = {}
        self.blocks = set()
        self.calls = {}      # return address -> CallSite
        self.tailjumps = {}  # jmp address -> target
        self.error = None
        self._descend()

    def inside(self, a):
        return self.start <= a < self.end

    def block_starts_in(self, lo, hi):
        if not hasattr(self, "_sorted_blocks"):
            self._sorted_blocks = sorted(self.blocks)
        import bisect
        i = bisect.bisect_left(self._sorted_blocks, lo)
        out = []
        while i < len(self._sorted_blocks) and self._sorted_blocks[i] < hi:
            out.append(self._sorted_blocks[i])
            i += 1
        return out

    def _descend(self):
        work = [self.start]
        self.blocks.add(self.start)
        while work:
            a = work.pop()
            while self.inside(a) and a not in self.insns:
                ins = decode(self.img, a)
                if ins is None:
                    break
                self.insns[a] = ins
                mn = ins.mn
                if mn in ("ret", "retf"):
                    ins.kind = "ret"
                    break
                if mn in ("int3", "hlt", "ud2"):
                    ins.kind = "stop"
                    break
                if mn == "jmp":
                    op = ins.ops[0]
                    if op[0] == "imm":
                        t = op[1]
                        if self.inside(t):
                            ins.kind = "jmp"
                            ins.succ = [t]
                            self.blocks.add(t)
                            work.append(t)
                        else:
                            ins.kind = "tail"
                            ins.target = t
                            self.tailjumps[a] = t
                    elif op[0] == "mem" and op[2] is not None and op[3] == 4 and op[1] is None:
                        # jump table: jmp [index*4 + table]
                        ins.kind = "switch"
                        table = op[4]
                        i = 0
                        while i < 512:
                            t = self.img.u32(table + 4 * i)
                            if t is None or not self.inside(t):
                                break
                            ins.succ.append(t)
                            self.blocks.add(t)
                            work.append(t)
                            i += 1
                        # the table itself is data inside the function
                        for k in range(table, table + 4 * i):
                            self.insns.setdefault(k, None)
                    else:
                        ins.kind = "tail"  # jmp through memory / register
                        self.tailjumps[a] = None
                    break
                if mn.startswith("j") or mn.startswith("loop") or mn == "jecxz":
                    t = ins.ops[0][1]
                    ins.kind = "jcc"
                    ins.succ = [t, ins.next]
                    self.blocks.add(t)
                    self.blocks.add(ins.next)
                    if self.inside(t):
                        work.append(t)
                    a = ins.next
                    continue
                if mn == "call":
                    ins.kind = "call"
                    op = ins.ops[0]
                    ins.succ = [ins.next]
                    self.blocks.add(ins.next)
                    if op[0] == "imm":
                        ins.target = op[1]
                        if self.inside(op[1]):
                            # an EH unwind funclet inside the function's own
                            # bytes: executed, not stubbed
                            ins.kind = "icall"
                            self.blocks.add(op[1])
                            work.append(op[1])
                            a = ins.next
                            continue
                    self.calls[ins.next] = CallSite(ins)
                    a = ins.next
                    continue
                ins.succ = [ins.next]
                a = ins.next
        self.insns = {a: i for a, i in self.insns.items() if i is not None}
        self.blocks &= set(self.insns)

    # ------------------------------------------------------------------
    def solve_stack(self, callee_pop):
        """Fill CallSite.pop for every call site.  callee_pop(site) returns a
        known pop count or UNKNOWN; unknown ones are solved by consistency."""
        sites = list(self.calls.values())
        for s in sites:
            s.pop = callee_pop(s)
            s.known = s.pop is not UNKNOWN
        unknown = [s for s in sites if not s.known]
        for s in unknown:
            s.pop = s.heuristic = self._pushes_before(s)
        ok = self._dataflow(set())
        if ok is True or not unknown:
            self.stack_ok = ok is True
            return
        # search: vary the unknown sites one at a time, then in pairs
        cands = sorted({0, 4, 8, 12, 16, 20, 24, 28, 32, 36, 40})
        for s in unknown:
            base = s.pop
            for c in cands:
                s.pop = c
                if self._dataflow(set()) is True:
                    self.stack_ok = True
                    return
            s.pop = base
        if len(unknown) <= 6:
            for i, s in enumerate(unknown):
                for t in unknown[i + 1:]:
                    bs, bt = s.pop, t.pop
                    for c in cands:
                        for d in cands:
                            s.pop, t.pop = c, d
                            if self._dataflow(set()) is True:
                                self.stack_ok = True
                                return
                    s.pop, t.pop = bs, bt
        self.stack_ok = False

    def _pushes_before(self, site):
        """Bytes pushed by the pushes immediately preceding a call in its
        block (other non-stack instructions in between are skipped)."""
        n = 0
        a = site.insn.addr
        # walk backwards by re-decoding forward from the block start
        start = max(b for b in self.blocks if b <= a)
        seq = []
        x = start
        while x < a and x in self.insns:
            seq.append(self.insns[x])
            x = self.insns[x].next
        # in the entry block, a push of a callee-saved register nothing has
        # written yet saves the caller's value (prologue), not an argument
        saves = set()
        if start == self.start:
            written = set()
            for ins in seq:
                r = ins.ops[0][1] if ins.ops and ins.ops[0][0] == "reg" else None
                if ins.mn == "push" and r in ("ebx", "esi", "edi", "ebp") and r not in written:
                    saves.add(ins.addr)
                elif r is not None and ins.mn not in ("push", "cmp", "test"):
                    written.add(r)
        for ins in reversed(seq):
            if ins.addr in saves:
                continue
            if ins.mn == "push":
                n += 4
            elif ins.kind == "call" or ins.mn in ("pop", "add", "sub") and ins.ops and ins.ops[0] == ("reg", "esp"):
                break
            elif ins.mn == "pop":
                break
        return n

    def _dataflow(self, _):
        """Propagate (depth, ebp_depth).  True if consistent, else a message."""
        state = {self.start: (0, None)}
        work = [self.start]
        self.depth = state
        while work:
            a = work.pop()
            ins = self.insns.get(a)
            if ins is None:
                continue
            d, eb = state[a]
            r = self._effect(ins, d, eb)
            if isinstance(r, str):
                return r
            if r is None:
                continue
            nd, neb = r
            if nd < 0 and ins.kind != "ret":
                return f"negative depth at {a:#x}"
            for s in ins.succ:
                if not self.inside(s) or s not in self.insns:
                    continue
                if s in state:
                    if state[s][0] != nd:
                        return f"depth conflict at {s:#x}: {state[s][0]} vs {nd}"
                else:
                    state[s] = (nd, neb)
                    work.append(s)
        return True

    def _effect(self, ins, d, eb):
        mn, ops = ins.mn, ins.ops
        if ins.kind == "ret":
            if d != 0:
                return f"ret at depth {d} ({ins.addr:#x})"
            return None
        if ins.kind == "tail":
            if d != 0:
                return f"tail jump at depth {d} ({ins.addr:#x})"
            return None
        if ins.kind == "stop":
            return None
        if mn == "push":
            return d + (2 if ops and ops[0][0] == "mem" and ops[0][6] == 2 else 4), eb
        if mn == "pushal":
            return d + 32, eb
        if mn in ("pushfd", "pushf"):
            return d + 4, eb
        if mn == "pop":
            return d - 4, eb
        if mn == "popal":
            return d - 32, eb
        if mn in ("popfd", "popf"):
            return d - 4, eb
        if mn == "leave":
            if eb is None:
                return f"leave without frame ({ins.addr:#x})"
            return eb - 4, eb
        if ins.kind == "call":
            site = self.calls[ins.next]
            return d - site.pop, eb
        if ins.kind == "icall":
            return d, eb
        if ops and ops[0] == ("reg", "esp"):
            if mn == "sub" and ops[1][0] == "imm":
                return d + _signed(ops[1][1]), eb
            if mn == "add" and ops[1][0] == "imm":
                return d - _signed(ops[1][1]), eb
            if mn == "mov" and ops[1] == ("reg", "ebp"):
                if eb is None:
                    return f"mov esp,ebp without frame ({ins.addr:#x})"
                return eb, eb
            if mn == "lea" and ops[1][0] == "mem":
                base, disp = ops[1][1], _signed(ops[1][4])
                if base == "esp":
                    return d - disp, eb
                if base == "ebp" and eb is not None:
                    return eb - disp, eb
            if mn in ("cmp", "test"):
                return d, eb
            return f"unmodelled esp write {mn} {ins.op_str} ({ins.addr:#x})"
        if ops and ops[0] == ("reg", "ebp") and mn == "mov" and len(ops) > 1 and ops[1] == ("reg", "esp"):
            return d, d
        return d, eb


class CallSite:
    __slots__ = ("insn", "pop", "known", "heuristic", "ret", "arity")

    def __init__(self, insn):
        self.insn = insn
        self.ret = insn.next
        self.pop = None
        self.known = False
        self.heuristic = None
        self.arity = None


def _signed(v):
    return v - (1 << 32) if v & 0x80000000 else v


def ret_pop(img, addr, limit=3000):
    """Pop count of the first `ret` reached by recursive descent from addr."""
    seen, work, n = set(), [addr], 0
    while work and n < limit:
        a = work.pop(0)
        while a not in seen and n < limit:
            seen.add(a)
            n += 1
            ins = decode(img, a)
            if ins is None:
                break
            if ins.mn == "ret":
                return ins.ops[0][1] if ins.ops else 0
            if ins.mn in ("int3", "hlt", "ud2"):
                break
            if ins.mn == "jmp":
                if ins.ops[0][0] == "imm":
                    work.append(ins.ops[0][1])
                break
            if ins.mn.startswith("j"):
                work.append(ins.ops[0][1])
            a = ins.next
    return None
