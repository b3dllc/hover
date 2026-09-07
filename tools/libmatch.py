#!/usr/bin/env python3
"""Name the original's statically linked MFC/CRT functions for reccmp.

The original hover.exe has no symbols, so every call our code makes into MFC
or the CRT is printed by reccmp as `call <OFFSETn>` on the original side and
as `call CDialog::OnInitDialog (FUNCTION)` on ours: a scored difference even
when the surrounding code is byte-identical (notes/perfect-match.md rule 20).
Our build links the same libraries, so the functions themselves are present in
both images - only the names are missing on the original side.

This tool recovers them. It writes `tools/orig_library.csv`, a reccmp data
source (`data-sources:` in reccmp-project.yml) that gives each original
library address the *mangled symbol* of the matching function in our build.
reccmp's match_symbols() then pairs the two, so both sides of a call print the
identical name and the instruction scores as a match. The rows carry
`type=library`, which makes reccmp treat them as library code: `asmcmp
--nolib` (which tools/compare.py passes) keeps them out of the report, so the
Implemented/Accuracy/Progress numbers still describe our own code only.

The CSV stores symbols, not our addresses, so it survives a rebuild: our
library functions move whenever the game code changes size, but their names do
not. It only needs regenerating when the set of linked library functions
changes (a new MFC class used, a toolset change).

How the matching works
----------------------
Both images are analyzed identically, so that every decision is symmetric:

1. Function starts: recursive-descent disassembly from the annotated original
   functions / our map symbols, the entry point and every relocation that
   points into .text, minus switch jump-table targets (they point into the
   middle of a function) and minus pointer targets that are not preceded by a
   ret/jmp/int3. Direct call targets found during the descent are starts too.
   A function extends to the next start, trailing int3/nop trimmed.

2. Fingerprint: the instruction stream with everything that a relink changes
   removed - absolute data addresses become `d`, string addresses become the
   string, IAT slots become `imp:DLL!Name`, internal branches become an offset
   from the function start, and a call becomes the *callee's* fingerprint.
   Anything else, immediates included, is compared verbatim. Callee
   fingerprints are computed to several depths: depth 0 ignores the callees,
   depth k knows the call graph k levels down. Deeper is more precise but also
   more sensitive to a callee that the two library builds compiled
   differently, so the matcher tries the depths in turn.

3. Content pass: a (size, fingerprint) pair that occurs exactly once on each
   side is a match, tightest depth first. A matched function then fingerprints
   as a pair id - the same token on both sides - which makes its callers more
   specific, so the pass is repeated until nothing new appears.

4. Twin passes: MFC's near-template classes produce byte-identical twins
   (CObList::AddHead and CPtrList::AddHead are the same code), which share a
   bucket and cannot be told apart by content. They are assigned by caller
   votes: for every matched pair of callers, the two instruction streams are
   aligned (difflib) and a call aligned with a call votes for its two callees.
   Inside a tightest-depth bucket the members are byte-identical, so any
   assignment is harmless and one unopposed vote is enough; the looser pass
   (functions of the same shape that call different things) demands twice the
   runner-up's votes and a 90%-similar body.

5. Drift pass: the original's MFC/CRT is a slightly different build of the
   same source, so some functions differ by an instruction or two and never
   share a fingerprint. The same caller alignment proposes them - once with
   callee fingerprints (which keeps AddHead and AddTail apart) and once with
   every unknown callee rendered alike (which is the only way two *drifted*
   callees line up at all). A proposal is accepted only if the two bodies are
   at least 60% similar, the winner is 5 points clear of the runner-up, and
   the proposal is 1:1.

6. Verification: every accepted pair must be call-graph consistent - where the
   two functions call the same number of functions, the i-th callees must be
   each other's match wherever either is matched. Failures are dropped and the
   check repeats, so one bad name cannot spread through the votes.

Residual risk: a wrong name is possible where our code calls a *different*
library function than the original does at the same place and the two are
similar enough to pass the gates - that would hide a real difference. The
passes are ordered so that this can only happen for drifted functions (byte
identity does the rest), and every emitted pair was checked to agree on its
`ret N` stack cleanup.

Usage:
    tools/libmatch.py                 # report (no files written)
    tools/libmatch.py --write         # regenerate tools/orig_library.csv
    tools/libmatch.py --show 0x449377 # explain one original address
"""

import argparse
import collections
import difflib
import hashlib
import re
import struct
import sys
from pathlib import Path

from capstone import CS_ARCH_X86, CS_GRP_CALL, CS_GRP_JUMP, CS_MODE_32, Cs

ROOT = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(ROOT / "tools"))  # for vc2map


def _original_path():
    """The original hover.exe, as configured in reccmp-user.yml."""
    text = (ROOT / "reccmp-user.yml").read_text(encoding="utf-8")
    match = re.search(r"^\s*path:\s*(\S+)\s*$", text, re.M)
    if match:
        return Path(match.group(1))
    return ROOT.parent / "hover" / "hover.exe"


ORIG_EXE = _original_path()
OUR_EXE = ROOT / "build" / "hover.exe"
OUR_MAP = ROOT / "build" / "hover.map"
CSV_OUT = ROOT / "tools" / "orig_library.csv"

# The original's game code ends with Hover_sub_43DC40; everything above this
# belongs to the statically linked libraries. Nothing below it is ever named
# by this tool - those functions are reccmp's business (source markers).
LIB_START = 0x43DE00

# How deep the structural fingerprints go (h[0] ignores callees, h[k] knows
# the call graph k levels down).
HASH_ROUNDS = 3
# Depths tried when matching by content, tightest first. None = "every unknown
# callee looks alike", the loosest key, which still requires the match to be
# unique on both sides.
MATCH_DEPTHS = (2, 1, 0, None)
# Depth used when aligning two functions instruction by instruction. h[1]
# tells CObList::AddHead from CObList::AddTail (their bodies differ) without
# being thrown off by drift deeper in the call graph.
VOTE_DEPTH = 1
# Twin passes: (fingerprint depth, minimum votes, votes needed over the
# runner-up, minimum body similarity). The first pass resolves real twins
# (identical bytes, so any assignment within the bucket is harmless); the
# second one takes on look-alikes that differ in which unmatched function they
# call, and demands a clear majority and a near-identical body for those.
TWIN_PASSES = ((2, 1, 1.0, 0.0), (None, 2, 2.0, 0.9))

# Minimum body similarity for a drift match proposed by caller alignment.
DRIFT_SIMILARITY = 0.60
# ... and how far ahead of the runner-up it has to be.
DRIFT_MARGIN = 0.05

md = Cs(CS_ARCH_X86, CS_MODE_32)
md.detail = True

HEX = re.compile(r"0x[0-9a-f]+")


class Image:
    """Just enough PE for this tool: .text bytes, imports and relocations."""

    def __init__(self, path):
        data = open(path, "rb").read()
        self.data = data
        pe = struct.unpack_from("<I", data, 0x3C)[0]
        nsec = struct.unpack_from("<H", data, pe + 6)[0]
        optsz = struct.unpack_from("<H", data, pe + 20)[0]
        self.base = struct.unpack_from("<I", data, pe + 24 + 28)[0]
        self.entry = self.base + struct.unpack_from("<I", data, pe + 24 + 16)[0]
        self.sections = []
        for i in range(nsec):
            off = pe + 24 + optsz + 40 * i
            name = data[off:off + 8].rstrip(b"\0").decode()
            vsize, va, rsize, raw = struct.unpack_from("<IIII", data, off + 8)
            self.sections.append((name, self.base + va, vsize, raw, rsize))
            if name == ".text":
                self.text_start = self.base + va
                self.text = data[raw:raw + min(vsize, rsize)]
                self.text_end = self.text_start + len(self.text)
        self.image_end = max(va + vsz for _, va, vsz, _, _ in self.sections)
        datadir = pe + 24 + 96
        self._imp_rva = struct.unpack_from("<I", data, datadir + 8)[0]
        self._reloc_rva, self._reloc_size = struct.unpack_from(
            "<II", data, datadir + 40
        )
        self.iat = {}
        self._load_imports()
        self.relocs = self._load_relocs()
        self._strings = {}

    # -- raw access ---------------------------------------------------------
    def offset(self, va):
        for _, start, vsize, raw, rsize in self.sections:
            if start <= va < start + vsize:
                return raw + (va - start) if va - start < rsize else None
        return None

    def read(self, va, size):
        off = self.offset(va)
        return self.data[off:off + size] if off is not None else b""

    def u32(self, va):
        buf = self.read(va, 4)
        return struct.unpack("<I", buf)[0] if len(buf) == 4 else None

    def cstring(self, va, limit=96):
        off = self.offset(va)
        if off is None:
            return None
        end = self.data.find(b"\0", off, off + limit)
        return self.data[off:end] if end > off else None

    def in_text(self, va):
        return self.text_start <= va < self.text_end

    # -- tables -------------------------------------------------------------
    def _load_imports(self):
        pos = self.base + self._imp_rva
        while True:
            desc = self.read(pos, 20)
            if len(desc) < 20 or desc == b"\0" * 20:
                break
            ilt, _, _, name_rva, iat = struct.unpack("<IIIII", desc)
            dll = (self.cstring(self.base + name_rva) or b"?").decode()
            lookup = self.base + (ilt or iat)
            slot = self.base + iat
            while True:
                value = self.u32(lookup)
                if not value:
                    break
                if value & 0x80000000:
                    name = f"#{value & 0xFFFF}"
                else:
                    name = (self.cstring(self.base + value + 2) or b"?").decode()
                self.iat[slot] = f"{dll}!{name}"
                lookup += 4
                slot += 4
            pos += 20

    def _load_relocs(self):
        out = set()
        pos = self.base + self._reloc_rva
        end = pos + self._reloc_size
        while pos < end:
            header = self.read(pos, 8)
            if len(header) < 8:
                break
            page, size = struct.unpack("<II", header)
            if size < 8:
                break
            block = self.read(pos + 8, size - 8)
            for i in range(0, len(block) - 1, 2):
                (entry,) = struct.unpack_from("<H", block, i)
                if entry >> 12 == 3:
                    out.add(self.base + page + (entry & 0xFFF))
            pos += size
        return out

    # -- operand classification --------------------------------------------
    def classify(self, value):
        """Symbolic stand-in for an absolute address.

        None: the value is a code address (the caller resolves it).
        "":   the value is not an address at all; keep the literal."""
        if not self.base <= value < self.image_end:
            return ""
        if value in self.iat:
            return "imp:" + self.iat[value]
        if self.in_text(value):
            return None  # a code address; the caller resolves it
        if value not in self._strings:
            text = self.cstring(value)
            ok = text is not None and len(text) >= 3 and all(
                32 <= c < 127 or c in (9, 10, 13) for c in text
            )
            self._strings[value] = ("s:" + text.decode("ascii")) if ok else "d"
        return self._strings[value]


def reloc_text_targets(image):
    out = set()
    for site in image.relocs:
        value = image.u32(site)
        if value is not None and image.in_text(value):
            out.add(value)
    return out


def descend(image, seeds):
    """Recursive-descent disassembly. Returns (instruction addrs, call targets)."""
    seen = set()
    calls = set()
    stack = [s for s in seeds if image.in_text(s)]
    while stack:
        addr = stack.pop()
        while True:
            if addr in seen or not image.in_text(addr):
                break
            off = addr - image.text_start
            ins = next(md.disasm(image.text[off:off + 16], addr, 1), None)
            if ins is None:
                break
            seen.add(addr)
            target = None
            if ins.operands and ins.operands[0].type == 2:  # IMM
                if CS_GRP_CALL in ins.groups or CS_GRP_JUMP in ins.groups:
                    target = ins.operands[0].imm
            if target is not None and image.in_text(target):
                if CS_GRP_CALL in ins.groups:
                    calls.add(target)
                if target not in seen:
                    stack.append(target)
            if ins.mnemonic in ("ret", "retf", "iret", "jmp", "int3"):
                break
            addr += ins.size
    return seen, calls


def jump_table_targets(image, insn_addrs):
    """Case labels of `jmp dword ptr [reg*4 + table]`: not function starts."""
    out = set()
    for addr in insn_addrs:
        off = addr - image.text_start
        ins = next(md.disasm(image.text[off:off + 16], addr, 1), None)
        if ins is None or ins.mnemonic != "jmp" or not ins.operands:
            continue
        operand = ins.operands[0]
        if operand.type != 3:  # MEM
            continue
        table = operand.mem.disp & 0xFFFFFFFF
        if not image.in_text(table):
            continue
        while table in image.relocs:
            value = image.u32(table)
            if value is None or not image.in_text(value):
                break
            out.add(value)
            table += 4
    return out


def function_starts(image, seeds):
    pointed = reloc_text_targets(image)
    seen, calls = descend(image, set(seeds) | pointed | {image.entry})
    tables = jump_table_targets(image, seen)
    # Where does each instruction end? A pointer into .text is only a function
    # start if the previous instruction ended the previous function.
    ends = {}
    for addr in seen:
        off = addr - image.text_start
        ins = next(md.disasm(image.text[off:off + 16], addr, 1), None)
        if ins is not None:
            ends[addr + ins.size] = ins.mnemonic
    pointed = {
        t for t in pointed - tables
        if ends.get(t) in (None, "ret", "retf", "jmp", "int3")
    }
    return {
        a for a in (set(seeds) | {image.entry} | calls | pointed)
        if image.in_text(a)
    }


def extents(image, starts):
    """{start: size}, each function running up to the next start."""
    ordered = sorted(starts)
    out = {}
    for i, addr in enumerate(ordered):
        end = ordered[i + 1] if i + 1 < len(ordered) else image.text_end
        while end > addr and image.read(end - 1, 1) in (b"\xcc", b"\x90"):
            end -= 1
        if end > addr:
            out[addr] = end - addr
    return out


def analyze(image, addr, size):
    """Render one function as a list of (format string, code targets).

    The format string is the instruction with every absolute address replaced;
    `{}` marks a call/jump target whose name is filled in later.
    Returns None if the bytes do not disassemble into exactly `size` bytes.
    """
    off = addr - image.text_start
    buf = image.text[off:off + size]
    out = []
    calls = []
    pos = 0
    for ins in md.disasm(buf, addr):
        if ins.address - addr != pos:
            return None
        pos += ins.size
        mnemonic = ins.mnemonic
        if ins.operands and ins.operands[0].type == 2 and (
            CS_GRP_JUMP in ins.groups or CS_GRP_CALL in ins.groups
        ):
            target = ins.operands[0].imm
            if addr <= target < addr + size:
                out.append((f"{mnemonic} .{target - addr}", ()))
            elif image.in_text(target):
                calls.append(target)
                out.append((mnemonic + " @{}", (target,)))
            else:
                out.append((f"{mnemonic} {target:#x}", ()))
            continue
        text = ins.op_str
        pieces = []
        targets = []
        last = 0
        for match in HEX.finditer(text):
            value = int(match.group(0), 16)
            if value < 0x10000:
                continue
            name = image.classify(value)
            if name is None:
                calls.append(value)
                repl, tgt = "@{}", value
            elif name == "":
                continue  # not an address, keep the literal
            else:
                repl, tgt = name.replace("{", "{{").replace("}", "}}"), None
            pieces.append(_escape(text[last:match.start()]))
            pieces.append(repl)
            if tgt is not None:
                targets.append(tgt)
            last = match.end()
        if pieces:
            pieces.append(_escape(text[last:]))
            out.append((mnemonic + " " + "".join(pieces), tuple(targets)))
        else:
            out.append((mnemonic + " " + _escape(text), ()))
    if pos != size:
        return None
    return out, calls


def _escape(text):
    return text.replace("{", "{{").replace("}", "}}")


def digest(template, resolve):
    hasher = hashlib.blake2b(digest_size=12)
    for fmt, targets in template:
        hasher.update(fmt.format(*[resolve(t) for t in targets]).encode())
        hasher.update(b"\n")
    return hasher.digest()


def render(template, resolve):
    return [fmt.format(*[resolve(t) for t in targets]) for fmt, targets in template]


class Side:
    """One image plus its function inventory."""

    def __init__(self, image, seeds):
        self.image = image
        self.ext = extents(image, function_starts(image, seeds))
        self.tmpl = {}
        self.calls = {}
        for addr, size in self.ext.items():
            result = analyze(image, addr, size)
            if result is not None:
                self.tmpl[addr], self.calls[addr] = result
        self.hash = {}

    def refine(self, rounds=HASH_ROUNDS):
        """Structural hashes: h[k](f) summarizes f and its call graph k deep.

        h[0] ignores the callees entirely, h[1] knows each callee's body, and
        so on. The deeper the hash, the more precise - and the more sensitive
        to a callee that the two library builds compiled differently, which is
        why the matcher tries several depths."""
        self.hash = []
        previous = {}
        for _ in range(rounds):
            resolve = lambda t: previous.get(t, b"").hex() or "?"
            current = {
                addr: digest(template, resolve)
                for addr, template in self.tmpl.items()
            }
            self.hash.append(current)
            previous = current

    def _resolver(self, pair_id, depth):
        """`pair_id(target)` is the match id of a callee (the same number in
        both images) or None. depth None: unknown callees are all alike ("?",
        the loosest key)."""
        table = self.hash[depth] if depth is not None else {}

        def resolve(target):
            ident = pair_id(target)
            if ident is not None:
                return "M%x" % ident
            value = table.get(target)
            return value.hex() if value else "?"

        return resolve

    def key(self, addr, pair_id, depth):
        return (
            self.ext[addr],
            digest(self.tmpl[addr], self._resolver(pair_id, depth)),
        )

    def stream(self, addr, pair_id, depth=0):
        return render(self.tmpl[addr], self._resolver(pair_id, depth))


class Matcher:
    def __init__(self, verbose=False):
        self.verbose = verbose
        import vc2map  # local import: only needed for the marker seeds

        self.map_symbols = vc2map.parse_map(OUR_MAP)
        markers = vc2map.scan_markers(ROOT / "src")
        vc2map.resolve_markers(markers, self.map_symbols)
        self.seed = {
            m["orig_addr"]: m["sym"]["addr"]
            for m in markers
            if "sym" in m and m["kind"] != "GLOBAL"
        }
        self.name_at = {}
        self.obj_at = {}
        self.symbols_at = collections.Counter()
        for sym in self.map_symbols:
            if sym["section"] == 1:
                self.name_at.setdefault(sym["addr"], sym["name"])
                self.obj_at.setdefault(sym["addr"], sym.get("obj", ""))
                self.symbols_at[sym["addr"]] += 1
        self.orig = Side(Image(ORIG_EXE), self.seed.keys())
        self.ours = Side(
            Image(OUR_EXE), [s["addr"] for s in self.map_symbols if s["section"] == 1]
        )
        self.orig.refine()
        self.ours.refine()
        self.pairs = dict(self.seed)          # orig addr -> our addr
        self.rev = {v: k for k, v in self.pairs.items()}
        # A match id that means the same on both sides: our address.
        self.orig_id = self.pairs.get
        self.ours_id = lambda addr: addr if addr in self.rev else None
        self.stats = collections.Counter()

    def log(self, *args):
        if self.verbose:
            print(*args, file=sys.stderr)

    # -- passes -------------------------------------------------------------
    def buckets(self, depth):
        left = collections.defaultdict(list)
        right = collections.defaultdict(list)
        for addr in self.orig.tmpl:
            if addr not in self.pairs:
                left[self.orig.key(addr, self.orig_id, depth)].append(addr)
        for addr in self.ours.tmpl:
            if addr not in self.rev:
                right[self.ours.key(addr, self.ours_id, depth)].append(addr)
        return left, right

    def add(self, orig_addr, our_addr):
        self.pairs[orig_addr] = our_addr
        self.rev[our_addr] = orig_addr

    def exact_pass(self, depth):
        total = 0
        while True:
            left, right = self.buckets(depth)
            added = 0
            for key, orig_addrs in left.items():
                our_addrs = right.get(key)
                if our_addrs and len(orig_addrs) == 1 and len(our_addrs) == 1:
                    self.add(orig_addrs[0], our_addrs[0])
                    added += 1
            total += added
            self.log("  exact pass (depth %s): +%d" % (depth, added))
            if not added:
                break
        self.stats["exact"] += total
        return total

    def votes(self, depth=VOTE_DEPTH):
        """Callee proposals from aligned call sites in matched callers."""
        proposals = collections.defaultdict(collections.Counter)
        for orig_addr, our_addr in list(self.pairs.items()):
            if orig_addr not in self.orig.tmpl or our_addr not in self.ours.tmpl:
                continue
            left = self.orig.stream(orig_addr, self.orig_id, depth)
            right = self.ours.stream(our_addr, self.ours_id, depth)
            lt = self.orig.tmpl[orig_addr]
            rt = self.ours.tmpl[our_addr]
            blocks = difflib.SequenceMatcher(
                None, left, right, autojunk=False
            ).get_matching_blocks()
            for i, j, n in blocks:
                for k in range(n):
                    a, b = lt[i + k], rt[j + k]
                    if not a[0].startswith("call @") or a[0] != b[0]:
                        continue
                    if len(a[1]) != 1 or len(b[1]) != 1:
                        continue
                    at, bt = a[1][0], b[1][0]
                    if at in self.pairs or bt in self.rev:
                        continue
                    if at not in self.orig.ext or bt not in self.ours.ext:
                        continue
                    proposals[at][bt] += 1
        return proposals

    def similarity(self, orig_addr, our_addr):
        if orig_addr not in self.orig.tmpl or our_addr not in self.ours.tmpl:
            return 0.0
        left = self.orig.stream(orig_addr, self.orig_id, VOTE_DEPTH)
        right = self.ours.stream(our_addr, self.ours_id, VOTE_DEPTH)
        return difflib.SequenceMatcher(None, left, right, autojunk=False).ratio()

    def twin_pass(self, depth, min_votes=1, ratio=1.0, min_similarity=0.0):
        """Assign functions that share a fingerprint bucket, by caller votes.

        At the tightest depth a bucket with several members holds genuine
        twins - byte-identical functions such as CObList::AddHead and
        CPtrList::AddHead - and any assignment within it is harmless, so a
        single unopposed vote is enough. Looser buckets can hold functions
        that merely have the same shape, so those need a clear majority and a
        similar body as well.
        """
        added = 0
        left, right = self.buckets(depth)
        proposals = self.votes()
        for key, orig_addrs in left.items():
            our_addrs = right.get(key)
            if not our_addrs or (len(orig_addrs) == 1 and len(our_addrs) == 1):
                continue
            free = set(our_addrs)
            for orig_addr in orig_addrs:
                counter = proposals.get(orig_addr)
                if not counter:
                    continue
                choices = sorted(
                    ((n, u) for u, n in counter.items() if u in free), reverse=True
                )
                if not choices or choices[0][0] < min_votes:
                    continue
                runner_up = choices[1][0] if len(choices) > 1 else 0
                if choices[0][0] <= runner_up * ratio:
                    continue
                if min_similarity and self.similarity(orig_addr, choices[0][1]) < min_similarity:
                    continue
                self.add(orig_addr, choices[0][1])
                free.discard(choices[0][1])
                added += 1
        self.stats["twin"] += added
        self.log("  twin pass (depth %s): +%d" % (depth, added))
        return added

    def drift_pass(self):
        """Match functions that differ (a different build of the same library)
        through the call sites of an already matched caller."""
        added = 0
        while True:
            # A drifted callee does not render identically on the two sides,
            # so its call sites only line up in the loose rendering; take
            # proposals from both and let the similarity gate sort them out.
            proposals = collections.defaultdict(collections.Counter)
            for depth in (VOTE_DEPTH, None):
                for orig_addr, counter in self.votes(depth).items():
                    proposals[orig_addr].update(counter)
            scored = {}
            for orig_addr, counter in proposals.items():
                ranked = sorted(
                    ((self.similarity(orig_addr, u), u) for u in counter),
                    reverse=True,
                )
                if not ranked or ranked[0][0] < DRIFT_SIMILARITY:
                    continue
                if len(ranked) > 1 and ranked[0][0] - ranked[1][0] < DRIFT_MARGIN:
                    continue
                scored[orig_addr] = ranked[0]
            # 1:1 only
            claimed = collections.Counter(u for _, u in scored.values())
            round_added = 0
            for orig_addr, (_, our_addr) in scored.items():
                if claimed[our_addr] == 1 and our_addr not in self.rev:
                    self.add(orig_addr, our_addr)
                    round_added += 1
            self.log("  drift pass: +%d" % round_added)
            added += round_added
            if not round_added:
                break
        self.stats["drift"] += added

    def verify(self):
        """Drop pairs whose call graphs disagree.

        For a pair (A, U), walk the aligned instruction streams: wherever both
        sides call something and either callee is matched, the two callees must
        be each other's match. One bad name would otherwise spread through the
        votes.
        """
        dropped = 0
        while True:
            bad = []
            for orig_addr, our_addr in self.pairs.items():
                if orig_addr in self.seed:
                    continue
                if orig_addr not in self.orig.tmpl or our_addr not in self.ours.tmpl:
                    continue
                left = self.orig.calls[orig_addr]
                right = self.ours.calls[our_addr]
                if len(left) != len(right):
                    if self.similarity(orig_addr, our_addr) < DRIFT_SIMILARITY:
                        bad.append(orig_addr)
                    continue
                for at, bt in zip(left, right):
                    if at in self.pairs and self.pairs[at] != bt:
                        bad.append(orig_addr)
                        break
                    if bt in self.rev and self.rev[bt] != at:
                        bad.append(orig_addr)
                        break
            if not bad:
                break
            for orig_addr in bad:
                our_addr = self.pairs.pop(orig_addr)
                self.rev.pop(our_addr, None)
            dropped += len(bad)
            self.log("  verify: -%d" % len(bad))
        self.stats["dropped"] = dropped

    def run(self):
        # Tightest fingerprint first, then progressively looser ones: every
        # match makes the next round's fingerprints more specific.
        for depth in MATCH_DEPTHS:
            self.log("content, depth %s:" % depth)
            self.exact_pass(depth)
        self.log("twins:")
        for depth, min_votes, ratio, min_sim in TWIN_PASSES:
            while self.twin_pass(depth, min_votes, ratio, min_sim):
                for match_depth in MATCH_DEPTHS:
                    self.exact_pass(match_depth)
        self.log("drift:")
        self.drift_pass()
        for depth in MATCH_DEPTHS:
            self.exact_pass(depth)
        self.log("verify:")
        self.verify()

    # -- output -------------------------------------------------------------
    def rows(self):
        """(original address, mangled symbol) for library functions only."""
        by_name = collections.Counter(self.name_at.values())
        out = []
        for orig_addr, our_addr in sorted(self.pairs.items()):
            if orig_addr < LIB_START or orig_addr in self.seed:
                continue
            if orig_addr == self.orig.image.entry:
                continue  # reccmp matches the CRT entry point by itself
            name = self.name_at.get(our_addr)
            if name is None or by_name[name] > 1:
                continue  # no symbol, or one shared by several functions
            if self.symbols_at[our_addr] > 1:
                continue  # several symbols here: reccmp keeps only one, and
                          # we cannot tell which (e.g. ??_E and ??_G aliases)
            obj = self.obj_at.get(our_addr, "")
            if ":" not in obj or not obj.lower().endswith(".obj"):
                continue  # our own code, or an import thunk
            out.append((orig_addr, name))
        return out


def write_csv(rows):
    with open(CSV_OUT, "w", encoding="utf-8", newline="\n") as f:
        f.write(
            "# Names for the original's statically linked MFC 3.1 / CRT\n"
            "# functions, generated by tools/libmatch.py (see its docstring).\n"
            "# 'symbol' is the mangled name in our build; reccmp matches the\n"
            "# two by symbol, so this file survives a rebuild. 'library' keeps\n"
            "# these functions out of the report (asmcmp --nolib).\n"
            "address,type,symbol\n"
        )
        for addr, name in rows:
            f.write(f"0x{addr:x},library,{name}\n")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--write", action="store_true", help="write the CSV")
    parser.add_argument("--show", help="explain one original address")
    parser.add_argument("-v", "--verbose", action="store_true")
    args = parser.parse_args()

    matcher = Matcher(verbose=args.verbose or not args.show)
    matcher.run()

    if args.show:
        addr = int(args.show, 16)
        our = matcher.pairs.get(addr)
        print("orig 0x%x -> %s" % (addr, "unmatched" if our is None else
                                   "0x%x %s" % (our, matcher.name_at.get(our, "?"))))
        if our is not None:
            left = matcher.orig.stream(addr, matcher.orig_id, VOTE_DEPTH)
            right = matcher.ours.stream(our, matcher.ours_id, VOTE_DEPTH)
            for i in range(max(len(left), len(right))):
                a = left[i] if i < len(left) else ""
                b = right[i] if i < len(right) else ""
                print(("   " if a == b else " ! ") + a.ljust(46) + b)
        return 0

    rows = matcher.rows()
    print("orig functions %d, ours %d" % (len(matcher.orig.ext), len(matcher.ours.ext)))
    print("matched: exact %d, twins %d, drift %d, dropped %d"
          % (matcher.stats["exact"], matcher.stats["twin"],
             matcher.stats["drift"], matcher.stats["dropped"]))
    print("library addresses with a name: %d" % len(rows))

    # How much of what our code calls is covered?
    called = collections.Counter()
    for orig_addr in matcher.seed:
        for target in matcher.orig.calls.get(orig_addr, ()):
            if target >= LIB_START and target not in matcher.seed:
                called[target] += 1
    named = {a for a, _ in rows}
    covered = [t for t in called if t in named]
    print("library functions called by our code: %d (%d call sites)"
          % (len(called), sum(called.values())))
    print("  named: %d (%d call sites)"
          % (len(covered), sum(called[t] for t in covered)))
    missing = sorted(((n, a) for a, n in called.items() if a not in named),
                     reverse=True)[:10]
    if missing:
        print("  most-called without a name: "
              + ", ".join("0x%x (%d)" % (a, n) for n, a in missing))

    if args.write:
        write_csv(rows)
        print("wrote %s" % CSV_OUT)
    return 0


if __name__ == "__main__":
    sys.exit(main())
