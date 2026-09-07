#!/usr/bin/env python3
"""Name the original's data addresses for reccmp (the data half of libmatch).

`tools/libmatch.py` gave the original's statically linked MFC/CRT *functions*
the mangled symbols of the same functions in our build, so a library call
scores as a match on both sides (notes/perfect-match.md rule 239). References
to *data* were left behind: the original has no symbols, so

    mov ecx, <OFFSET4>          (original)
    mov ecx, ?g_strReadFileError@@3VCString@@A   (ours)

is still scored as a difference even where the two instructions are the same
instruction. Two kinds of address are affected:

* library data - the MFC/CRT globals our code and the now-named library
  functions both touch (`__adjust_fdiv`, `_afxCoreState`, MFC's canned
  exception strings, ...);
* our own data whose `// GLOBAL: HOVER 0x...` marker is missing or impossible:
  every vtable (`??_7CFoo@@6B@`), every `CRuntimeClass` (`?classCFoo@...`) and
  every file-scope `static` (rule 204: a `static` is not in the linker map's
  PUBLICS, so vc2map can never resolve its marker).

This tool writes `tools/orig_data.csv`, a second reccmp data source in the
same shape as `orig_library.csv`: `address,type,symbol,size` rows that give an
original data address the mangled symbol of our data object. reccmp's
match_symbols() pairs the two, and because a matched entity is one row with
one name, both sides of the instruction then print the identical text.

How the matching works
----------------------
The original's data has no names, but it does have *users*: every function
pair that libmatch already established is a pair of instruction streams in
which the i-th data reference on one side is the i-th data reference on the
other.

1. Pairs. `libmatch.Matcher` is run as a library: its result is
   orig-function -> our-function for the annotated game functions (the
   `// FUNCTION:` markers, the seeds) *and* for the ~980 library functions it
   identified by content.

2. Alignment. Both bodies are rendered the way libmatch renders them for its
   caller votes - absolute data addresses collapsed to `d`, string addresses
   to the string, calls to the callee's pair id - and aligned with difflib.
   Instructions difflib calls equal are the same instruction. A same-length
   `replace` run is looked at again with libmatch's string guesses undone,
   because libmatch renders a data pointer as whatever text the bytes at the
   far end happen to spell: the head of a CRuntimeClass reads as text about
   as often as it does not, so `push <classCFlag>` comes out as `push s:0TL`
   in the original and as `push d` in ours and the two never line up.

3. Operands. The address operands of an instruction are read from the
   relocation table, not from the disassembly text: a 4-byte field inside the
   instruction that carries a base relocation *is* an absolute address, and
   the fields come out in the same order on both sides. This drops integer
   literals that merely look like addresses.

4. Candidates. Our address is resolved against the linker map (any section
   but .text): the symbol that contains it, plus the displacement into it.
   Each occurrence is one vote for (our symbol -> original address).

5. Consistency. A reference to an object's *base* is the evidence that
   counts. A reference *into* one is much weaker: a file-scope `static` has
   no map symbol, so our object's spacing in the map runs over the statics
   that follow it, and the two images do not order those alike - so an
   interior vote only counts when it agrees with the base votes, and decides
   alone only when a symbol is never referenced at its base. The base votes
   are normally unanimous. Two rules break a tie: a name that carries the
   original address it was found at (`g_dwUnk_4A47E0`) is believed over the
   votes, and otherwise a clear plurality (two votes and twice the runner-up)
   wins. Finally the map has to be 1:1 - a symbol claimed by two original
   addresses, or an address claimed by two symbols, is dropped - and an
   address one of our `// GLOBAL:` markers already names is left to the
   marker.

6. Content check. The two objects are then compared byte by byte, with each
   relocated field replaced by what it points at (a matched function's pair
   id, or the string's text) so that a pointer compares equal across the
   relink. A difference in a field where both sides are known - a vtable slot
   holding two different functions, say - rejects the pair; unknown on either
   side is no evidence and passes. Only a prefix is compared: our object's
   "size" is the distance to the next symbol in the map, which for all but
   the largest objects runs past its real end into the next, unnamed static.
   A table of code pointers (a vtable) is compared in full, and the two
   tables must also be the same length.

Sizes: the row carries our object's map spacing, the same number reccmp
already uses for the recompiled side, so that an interior reference renders
identically on both (`?g_lvlParams@@3QAUtagLVL_PARAMS@@A+0x18 (OFFSET)`)
instead of one side naming it and the other printing `<OFFSETn>` - which
costs a line that used to match. The only clipping is against the next row:
two named objects may not overlap.

Effect on the report is a change of *scoring*, not of our code: the
instruction was always the same instruction, reccmp simply could not say so.
Vtables become matched VTABLE entities as a side effect, which is what makes
`tools/compare.py --vtable` useful for the first time; they are not
functions, so `tools/compare.py` keeps them out of the function listing.

Residual risk: the same as libmatch's - if our code references a *different*
object than the original does, at an instruction that is otherwise identical,
and that object is referenced nowhere else, then the vote is unopposed, the
content check passes, and a real difference is hidden. Every accepted name is
cross-checked against all of its reference sites, so the mistake would have
to be systematic. The strongest evidence that it is not: 184 of the proposals
land on an address one of our hand-written `// GLOBAL:` markers already
asserts, and not one of them contradicts its marker.

Usage:
    tools/datamatch.py                 # report (no files written)
    tools/datamatch.py --write         # regenerate tools/orig_data.csv
    tools/datamatch.py --show 0x4b6f18 # explain one original address
"""

import argparse
import collections
import difflib
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(ROOT / "tools"))

import libmatch  # noqa: E402  (needs the path above)
import vc2map  # noqa: E402

CSV_OUT = ROOT / "tools" / "orig_data.csv"

# Sections of the linker map that hold data we are willing to name. 1 is
# .text (libmatch's business) and 5 is .idata, whose import slots reccmp
# matches by itself.
DATA_SECTIONS = (2, 3, 4)

# Symbols we never emit:
#   ??_C@   string literals - reccmp already matches those by content, and
#           claiming one would only take the match away from that pass.
#   __imp_ / \177  import thunks and NULL_THUNK_DATA.
#   ___x?_? the CRT startup array boundaries. Naming these in the original
#           switches on reccmp's CRT startup analysis for it, which invents a
#           FUNCTION entity for each of the original's 87 C++ dynamic
#           initializers - 79 of them unmatched, which inflates the
#           Implemented denominator and buries Progress for a couple of
#           `push` instructions in _WinMainCRTStartup.
SKIP_SYMBOL = re.compile(r"^(\?\?_C@|__imp_|\x7f|___x[icpt]_[az]$)")

# reccmp's CvdumpNode.set_decorated calls any symbol that does not begin with
# '?' but contains '@' a __stdcall function, whatever section it lives in.
# That is wrong for the exception-handling tables the compiler emits into
# .rdata (`__TI4?AVCFileException@@`, `__CT??_R0?AV...`): if we matched one,
# reccmp would disassemble the table as code and count it in the report as an
# implemented function. Leave them unnamed.
LOOKS_LIKE_FUNCTION = re.compile(r"^[^?].*@")

# Our own reverse-engineered names carry the original address of the object
# they stand for: `?g_dwUnk_4A47E0@@3KA` is the global at 0x4a47e0.
ADDRESS_IN_NAME = re.compile(r"_(4[0-9A-F]{5})(?:@|$)")

# Largest object we bother to compare byte by byte.
MAX_CONTENT = 8192
# ... and how much of one is compared when it is not a table of pointers.
CHECK_PREFIX = 32


class Symbols:
    """Our data objects, from the linker map: address -> (name, size)."""

    def __init__(self, map_symbols, exact=None):
        exact = exact or {}
        by_section = collections.defaultdict(list)
        for sym in map_symbols:
            if sym["section"] in DATA_SECTIONS:
                by_section[sym["section"]].append(sym)
        self.starts = []
        self.info = {}
        for syms in by_section.values():
            syms.sort(key=lambda s: s["addr"])
            for cur, nxt in zip(syms, syms[1:] + [None]):
                # The last symbol of a section has no successor; give it a
                # nominal size, it is only used as an upper bound.
                size = (nxt["addr"] - cur["addr"]) if nxt else 4
                # A vtable's real length, where we know it, rather than the
                # map spacing - the same number vc2map gives reccmp for the
                # recompiled side, so that the two sides agree.
                size = min(size, exact.get(cur["addr"], size))
                if cur["addr"] in self.info:
                    continue
                self.info[cur["addr"]] = (cur["name"], size)
                self.starts.append(cur["addr"])
        self.starts.sort()

    def containing(self, addr):
        """(base, name, size) of the object holding `addr`, or None."""
        i = _bisect(self.starts, addr)
        if i == 0:
            return None
        base = self.starts[i - 1]
        name, size = self.info[base]
        if addr - base >= size:
            return None
        return base, name, size


def _bisect(sorted_list, value):
    lo, hi = 0, len(sorted_list)
    while lo < hi:
        mid = (lo + hi) // 2
        if sorted_list[mid] <= value:
            lo = mid + 1
        else:
            hi = mid
    return lo


def reloc_operands(image, addr, size):
    """Per instruction, the absolute addresses its relocated fields hold.

    Walking the relocation table instead of the operand text means we see
    exactly the fields the linker fixed up - real addresses, in the order
    they appear in the instruction - and nothing else.
    """
    off = addr - image.text_start
    buf = image.text[off:off + size]
    out = []
    for ins in libmatch.md.disasm(buf, addr):
        values = []
        for site in range(ins.address, ins.address + ins.size):
            if site in image.relocs:
                value = image.u32(site)
                if value is not None:
                    values.append(value)
        out.append(values)
    return out


class DataMatcher:
    def __init__(self, verbose=False):
        self.verbose = verbose
        self.lib = libmatch.Matcher(verbose=False)
        self.lib.run()
        self.symbols = Symbols(
            self.lib.map_symbols,
            vc2map.exact_vtable_sizes(self.lib.map_symbols, libmatch.OUR_EXE),
        )
        # Original addresses our `// GLOBAL:` markers already name.
        markers = vc2map.scan_markers(ROOT / "src")
        vc2map.resolve_markers(markers, self.lib.map_symbols)
        self.marked = {
            m["orig_addr"]: m["sym"]["name"]
            for m in markers
            if m["kind"] == "GLOBAL" and "sym" in m
        }
        # (our symbol, displacement into it) -> Counter of original addresses.
        self.votes = collections.defaultdict(collections.Counter)
        self.sites = collections.defaultdict(list)
        self.seen_orig = set()
        self.stats = collections.Counter()
        self.rejected = collections.Counter()

    def log(self, *args):
        if self.verbose:
            print(*args, file=sys.stderr)

    # -- vote collection ----------------------------------------------------
    def collect(self):
        orig, ours = self.lib.orig, self.lib.ours
        for orig_addr, our_addr in sorted(self.lib.pairs.items()):
            if orig_addr not in orig.tmpl or our_addr not in ours.tmpl:
                continue
            left = orig.stream(orig_addr, self.lib.orig_id, libmatch.VOTE_DEPTH)
            right = ours.stream(our_addr, self.lib.ours_id, libmatch.VOTE_DEPTH)
            lref = reloc_operands(orig.image, orig_addr, orig.ext[orig_addr])
            rref = reloc_operands(ours.image, our_addr, ours.ext[our_addr])
            if len(lref) != len(left) or len(rref) != len(right):
                continue  # disassembly disagreed with the template; skip
            lblind = [self._blind(orig.image, s, r) for s, r in zip(left, lref)]
            rblind = [self._blind(ours.image, s, r) for s, r in zip(right, rref)]
            matcher = difflib.SequenceMatcher(None, left, right, autojunk=False)
            for tag, i1, i2, j1, j2 in matcher.get_opcodes():
                # 'equal' is libmatch's own alignment. A 'replace' run of the
                # same length is worth a second look: libmatch renders a data
                # pointer as the string it seems to point at, and the byte
                # soup at the head of a CRuntimeClass reads as text about as
                # often as it does not - so `push <classCFlag>` renders as
                # `push s:0TL` in the original and as `push d` in ours, and
                # the two never line up. Compare those blind to the guess.
                if tag == "equal":
                    pairs = zip(range(i1, i2), range(j1, j2))
                elif tag == "replace" and i2 - i1 == j2 - j1:
                    pairs = [
                        (i, j)
                        for i, j in zip(range(i1, i2), range(j1, j2))
                        if lblind[i] == rblind[j]
                    ]
                else:
                    continue
                for i, j in pairs:
                    self._vote(lref[i], rref[j], orig_addr, our_addr)

    @staticmethod
    def _blind(image, line, refs):
        """The rendered instruction with libmatch's string guesses undone."""
        for value in refs:
            token = image.classify(value)
            if isinstance(token, str) and token.startswith("s:"):
                line = line.replace(token, "d", 1)
        return line

    def _vote(self, left, right, orig_fn, our_fn):
        if len(left) != len(right) or not left:
            return
        for a, b in zip(left, right):
            if self.lib.orig.image.in_text(a) or self.lib.ours.image.in_text(b):
                continue  # a code pointer: libmatch's business
            self.seen_orig.add(a)
            found = self.symbols.containing(b)
            if found is None:
                self.stats["no_symbol"] += 1
                continue
            base, name, _size = found
            if SKIP_SYMBOL.match(name) or LOOKS_LIKE_FUNCTION.match(name):
                self.stats["skipped_symbol"] += 1
                continue
            self.votes[(name, b - base)][a] += 1
            self.sites[name].append((orig_fn, our_fn, a, b - base))

    # -- content check ------------------------------------------------------
    def _pointer_token(self, image, value, is_orig):
        """What a relocated field points at, in terms both images share."""
        if image.in_text(value):
            if is_orig:
                our = self.lib.pairs.get(value)
                return "f:%x" % our if our is not None else "?"
            return "f:%x" % value if value in self.lib.rev else "?"
        text = image.classify(value)
        if isinstance(text, str) and text.startswith("s:"):
            return text
        return "?"  # an unnamed data pointer tells us nothing yet

    def _normalize(self, image, addr, size, is_orig):
        """One token per *byte*, so the two sides stay in step.

        A relocated field fills its four bytes with the same token, which
        means a pointer on one side and plain data on the other is seen as
        the structural difference it is.
        """
        out = []
        pos = 0
        while pos < size:
            site = addr + pos
            if site in image.relocs and pos + 4 <= size:
                value = image.u32(site)
                token = ("?" if value is None
                         else self._pointer_token(image, value, is_orig))
                out.extend([token] * 4)
                pos += 4
            else:
                byte = image.read(site, 1)
                out.append("b%02x" % (byte[0] if byte else 0))
                pos += 1
        return out

    def pointer_run(self, image, addr, size):
        """Length of the run of code pointers at `addr` - a vtable's length."""
        pos = 0
        while pos + 4 <= size and addr + pos in image.relocs:
            value = image.u32(addr + pos)
            if value is None or not image.in_text(value):
                break
            pos += 4
        return pos

    def content_ok(self, orig_base, our_base, size):
        """Do the two objects hold the same thing?

        Only a prefix is compared. Our object's "size" is the distance to the
        next symbol in the linker map, which for anything but the largest
        objects runs well past its real end and into the next (unnamed)
        static - and the two images do not lay those out alike, so comparing
        the whole span rejects almost everything. A wrong pair disagrees
        immediately, so a prefix is enough; for a table of code pointers (a
        vtable) the prefix is the whole table, and the two tables must also
        be the same length.
        """
        size = min(size, MAX_CONTENT)
        run_ours = self.pointer_run(self.lib.ours.image, our_base, size)
        if run_ours:
            run_orig = self.pointer_run(self.lib.orig.image, orig_base, size)
            if run_orig != run_ours:
                return False
        # A pointer table ends where the run ends; anything past that is the
        # next, unnamed object and the two images do not order those alike.
        length = run_ours if run_ours else min(size, CHECK_PREFIX)
        ours = self._normalize(self.lib.ours.image, our_base, length, False)
        orig = self._normalize(self.lib.orig.image, orig_base, length, True)
        for a, b in zip(orig, ours):
            if a == "?" or b == "?":
                continue  # unknown on one side: no evidence either way
            if a.startswith("s:") != b.startswith("s:"):
                # One side's pointer target happens to read as text and the
                # other's does not: that is the string heuristic guessing,
                # not a difference (MFC's CRuntimeClass::m_pBaseClass points
                # at another CRuntimeClass whose bytes can spell a word).
                continue
            if a != b:
                return False
        return True

    # -- acceptance ---------------------------------------------------------
    def _pick(self, name, base_votes):
        """Which original address does this symbol name, if any?

        Unanimity is the usual case. Two other rules break a tie:

        * The names we gave the game's own globals carry the original
          address they were found at (`g_dwUnk_4A47E0`). That is a human
          decision from outside this tool, so it wins outright - and if the
          votes never propose it, the symbol is dropped rather than guessed.
        * Otherwise a clear plurality (at least two votes and at least twice
          the runner-up) decides. MFC's interchangeable exception strings are
          the case in point: they are `mov ecx, <global>` in a run of
          near-identical throw helpers, so a single mis-aligned helper can
          cast one stray vote for its neighbour.
        """
        hint = ADDRESS_IN_NAME.search(name)
        if hint is not None:
            want = int(hint.group(1), 16)
            return want if want in base_votes else None
        ranked = base_votes.most_common()
        if len(ranked) == 1:
            return ranked[0][0]
        if ranked[0][1] >= 2 and ranked[0][1] >= 2 * ranked[1][1]:
            return ranked[0][0]
        return None

    def proposals(self):
        """One proposed original base per symbol, with its vote count.

        A reference *into* an object (displacement != 0) is much weaker
        evidence than a reference to its base: a file-scope `static` has no
        map symbol, so our object's map spacing runs over the statics that
        follow it, and the original orders its data differently - which makes
        a displacement mean different things on the two sides. So the base
        references decide, and an interior reference is only counted when it
        agrees with them. A symbol that is *only* ever referenced from the
        inside falls back to its interior votes, and must be unanimous.
        """
        by_symbol = collections.defaultdict(collections.Counter)
        interior = collections.defaultdict(collections.Counter)
        for (name, disp), counter in self.votes.items():
            target = by_symbol[name] if disp == 0 else interior[name]
            for addr, n in counter.items():
                target[addr - disp] += n
        out = {}
        for name in set(by_symbol) | set(interior):
            base_votes = by_symbol.get(name) or collections.Counter()
            if base_votes:
                orig_base = self._pick(name, base_votes)
                if orig_base is None:
                    self.log("ambiguous base for %s: %s"
                             % (name, {hex(a): n for a, n in base_votes.items()}))
                    self.rejected["ambiguous_base"] += 1
                    continue
                votes = base_votes[orig_base] + interior[name][orig_base]
            else:
                inner = interior[name]
                if len(inner) > 1:
                    self.rejected["interior_only_ambiguous"] += 1
                    continue
                orig_base = next(iter(inner))
                votes = inner[orig_base]
            out[name] = (orig_base, votes)
        return out

    def accept(self):
        self.rejected = collections.Counter()
        rows = []
        proposed = self.proposals()
        # An original address may not be claimed by two symbols.
        claimed = collections.Counter(base for base, _ in proposed.values())
        for name in sorted(proposed):
            orig_base, votes = proposed[name]
            if claimed[orig_base] > 1:
                self.rejected["address_reused"] += 1
                self.log("0x%x claimed by several symbols (%s)" % (orig_base, name))
                continue
            if orig_base in self.marked:
                if self.marked[orig_base] != name:
                    self.rejected["marker_conflict"] += 1
                    self.log("marker says %s, votes say %s at 0x%x"
                             % (self.marked[orig_base], name, orig_base))
                else:
                    self.rejected["already_marked"] += 1
                continue
            our_base, size = self._our_object(name)
            if our_base is None:
                self.rejected["no_object"] += 1
                continue
            if not self.content_ok(orig_base, our_base, size):
                self.rejected["content"] += 1
                self.log("content differs at 0x%x (%s)" % (orig_base, name))
                continue
            rows.append([orig_base, name, size, votes])
        # The size we emit is our object's map spacing, the same number
        # reccmp already uses for the recompiled side, so that an interior
        # reference renders identically on the two sides (`name+0x38
        # (OFFSET)`) instead of one side naming it and the other printing
        # `<OFFSETn>` - the latter costs a line that used to match. The only
        # clipping is against the next row: two named objects may not overlap.
        rows.sort()
        for row, nxt in zip(rows, rows[1:]):
            if row[0] + row[2] > nxt[0]:
                row[2] = nxt[0] - row[0]
                self.rejected["size_clipped"] += 1
        return rows

    def _our_object(self, name):
        if not hasattr(self, "_by_name"):
            self._by_name = {}
            for base, (sym_name, size) in self.symbols.info.items():
                self._by_name.setdefault(sym_name, (base, size))
        return self._by_name.get(name, (None, None))


def write_csv(rows):
    with open(CSV_OUT, "w", encoding="utf-8", newline="\n") as f:
        f.write(
            "# Names for the original's data - MFC/CRT globals, vtables,\n"
            "# CRuntimeClass objects and the file-scope statics a `// GLOBAL:`\n"
            "# marker cannot reach (rule 204). Generated by tools/datamatch.py\n"
            "# (see its docstring). 'symbol' is the mangled name in our build;\n"
            "# reccmp matches the two by symbol, so this file survives a\n"
            "# rebuild. 'size' lets an interior reference print as 'name+N'.\n"
            "address,type,symbol,size\n"
        )
        for addr, name, size, _votes in rows:
            f.write(f"0x{addr:x},global,{name},{size}\n")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--write", action="store_true", help="write the CSV")
    parser.add_argument("--show", help="explain one original address")
    parser.add_argument("-v", "--verbose", action="store_true")
    args = parser.parse_args()

    matcher = DataMatcher(verbose=args.verbose)
    matcher.collect()
    rows = matcher.accept()

    if args.show:
        wanted = args.show if args.show.startswith("?") else int(args.show, 16)
        for row in rows:
            if wanted in (row[0], row[1]):
                print("0x%x -> %s (size %d, %d votes)"
                      % (row[0], row[1], row[2], row[3]))
                for orig_fn, our_fn, a, disp in matcher.sites[row[1]]:
                    print("   in 0x%x/0x%x: orig 0x%x, displacement %d"
                          % (orig_fn, our_fn, a, disp))
                return 0
        print("not emitted; votes mentioning it:")
        for (name, disp), counter in sorted(matcher.votes.items()):
            if wanted == name or wanted in counter:
                print("   %s+%d: %s" % (name, disp,
                                        {hex(a): n for a, n in counter.items()}))
        return 0

    print("function pairs: %d" % len(matcher.lib.pairs))
    print("original data addresses referenced: %d" % len(matcher.seen_orig))
    print("candidates: %d" % len(matcher.votes))
    print("rejected: %s" % dict(matcher.rejected))
    print("emitted: %d" % len(rows))
    kinds = collections.Counter()
    for _addr, name, _size, _votes in rows:
        if name.startswith("??_7"):
            kinds["vtable"] += 1
        elif "CRuntimeClass" in name:
            kinds["CRuntimeClass"] += 1
        elif "@@" in name:
            kinds["C++ global"] += 1
        else:
            kinds["C global"] += 1
    print("  by kind: %s" % dict(kinds))

    if args.write:
        write_csv(rows)
        print("wrote %s" % CSV_OUT)
    return 0


if __name__ == "__main__":
    sys.exit(main())
