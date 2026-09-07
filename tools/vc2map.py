#!/usr/bin/env python3
"""Bridge between reccmp and the Visual C++ 2.2 toolchain.

reccmp learns the recompiled binary's symbols and line numbers by running
Microsoft's cvdump.exe on the linker PDB. The cvdump bundled with reccmp
(version 14) cannot read the PDB 2.00 ("JG") files written by the VC++ 2.2
linker, so this module synthesizes an equivalent reccmp CvdumpParser from
two sources instead:

- build/hover.map: the linker map file gives every public symbol with its
  section, offset, and absolute address. This covers functions, globals,
  vtables and string constants.
- src/*.cpp: our own reccmp markers. The marker comment gives the original
  address, and the signature on the following line gives the qualified
  name, which is matched against the (mangled) map symbols. Each resolved
  marker becomes a synthetic LINES entry so reccmp's marker lookup keeps
  working.

compare.py installs the bridge by replacing reccmp.compare.core.Cvdump
with the fluent shim at the bottom of this file.
"""

import re
import struct
import sys
from pathlib import Path, PurePath, PureWindowsPath

from reccmp.cvdump.cvinfo import CvdumpTypeKey
from reccmp.cvdump.parser import (
    CvdumpParser,
    GdataEntry,
    LineValue,
    PublicsEntry,
    SizeRefEntry,
)
from reccmp.cvdump.symbols import SymbolsEntry


class BridgeError(Exception):
    pass


# ---------------------------------------------------------------------------
# Linker map file

_MAP_LINE = re.compile(
    r"^\s+([0-9A-Fa-f]{4}):([0-9A-Fa-f]{8})\s+(\S+)\s+([0-9A-Fa-f]{8})\s+"
    r"\S*\s*(\S*)"
)


def parse_map(map_path):
    """Parse a VC++ 2.x linker map file.

    Returns a list of dicts: section, offset, addr, name, obj (the map's
    Lib:Object column, e.g. "NAFXCW:wincore.obj" for library code)."""
    symbols = []
    in_publics = False
    with open(map_path, encoding="utf-8", errors="ignore") as f:
        for line in f:
            if "Publics by Value" in line:
                in_publics = True
                continue
            if not in_publics:
                continue
            match = _MAP_LINE.match(line)
            if match is None:
                continue
            symbols.append(
                {
                    "section": int(match.group(1), 16),
                    "offset": int(match.group(2), 16),
                    "name": match.group(3),
                    "addr": int(match.group(4), 16),
                    "obj": match.group(5),
                }
            )
    return symbols


# ---------------------------------------------------------------------------
# Marker scan

_MARKER = re.compile(r"^// (FUNCTION|STUB|GLOBAL): HOVER (0x[0-9a-fA-F]+)\s*$")

# Something that can follow a marker: a C++ signature or a global definition.
_METHOD_SIG = re.compile(r"(\w+)::(~?\w+|operator\s+new|operator\s+delete)\s*\(")
_FREE_SIG = re.compile(r"\b(\w+)\s*\(")
# A global definition: "TYPE name;", "TYPE name[N] = ...", and also
# "TYPE name(ctor args...)" for class objects with a constructor.
_GLOBAL_DEF = re.compile(r"\b(\w+)\s*(?:\[[^\]]*\])*\s*(?:=|;|\()")


def undecorated_prefix(kind, cls, name):
    """Expected mangled-name prefix for a marker's symbol."""
    if kind == "GLOBAL":
        return f"?{name}@@"
    if cls is not None:
        if name == cls:
            return f"??0{cls}@@"
        if name == "~" + cls:
            return f"??1{cls}@@"
        if name == "operator new":
            return f"??2{cls}@@"
        if name == "operator delete":
            return f"??3{cls}@@"
        return f"?{name}@{cls}@@"
    return f"?{name}@@"


def friendly_name(cls, name):
    return f"{cls}::{name}" if cls is not None else name


def scan_markers(src_root):
    """Walk src/*.cpp and yield marker dicts:
    kind, orig_addr, file, marker_line, cls, name (and global flag)."""
    markers = []
    for path in sorted(Path(src_root).glob("*.cpp")):
        lines = path.read_text(encoding="utf-8").splitlines()
        for i, line in enumerate(lines):
            m = _MARKER.match(line)
            if m is None:
                continue
            kind = m.group(1)
            # Find the declaration: the next line that is not blank/comment.
            j = i + 1
            while j < len(lines) and (
                not lines[j].strip() or lines[j].lstrip().startswith("//")
            ):
                j += 1
            if j >= len(lines):
                raise BridgeError(f"{path}:{i + 1}: marker without declaration")
            decl = lines[j].strip()

            cls = name = None
            if kind == "GLOBAL":
                gm = _GLOBAL_DEF.search(decl)
                if gm is not None:
                    name = gm.group(1)
            else:
                mm = _METHOD_SIG.search(decl)
                if mm is not None:
                    cls, name = mm.group(1), mm.group(2)
                else:
                    fm = _FREE_SIG.search(decl)
                    if fm is not None:
                        name = fm.group(1)
            if name is None:
                raise BridgeError(
                    f"{path}:{i + 1}: cannot parse declaration: {decl!r}"
                )

            markers.append(
                {
                    "kind": kind,
                    "orig_addr": int(m.group(2), 16),
                    "file": path,
                    "marker_line": i + 1,
                    "decl_line": j + 1,
                    "cls": cls,
                    "name": name,
                    "decl": decl,
                }
            )
    return markers


def resolve_markers(markers, map_symbols):
    """Match each marker to exactly one map symbol. Adds 'sym' to each."""
    errors = []
    for marker in markers:
        prefix = undecorated_prefix(marker["kind"], marker["cls"], marker["name"])
        # Functions live in section 1 (.text), data anywhere else.
        want_text = marker["kind"] != "GLOBAL"
        candidates = [
            s
            for s in map_symbols
            if s["name"].startswith(prefix) and (s["section"] == 1) == want_text
        ]
        if len(candidates) > 1 and marker["kind"] != "GLOBAL":
            # Overloaded functions share the prefix (e.g. two constructors):
            # narrow by parameter count — MSVC 2.x mangles a void parameter
            # list as ...XZ.
            zero_args = re.search(r"\(\s*(?:void)?\s*\)", marker["decl"])
            narrowed = [
                s
                for s in candidates
                if s["name"].endswith("XZ") == (zero_args is not None)
            ]
            if narrowed:
                candidates = narrowed
        if not candidates and marker["kind"] != "GLOBAL" and marker["cls"] is not None \
                and marker["name"] == "~" + marker["cls"]:
            # A destructor defined inline has no ??1 symbol of its own: the
            # compiler folds it into the scalar deleting destructor ??_G,
            # which is also what the original's vtable slot holds (rule 34).
            alt = f"??_G{marker['cls']}@@"
            candidates = [
                s for s in map_symbols if s["name"].startswith(alt) and s["section"] == 1
            ]
        if len(candidates) != 1:
            errors.append(
                "%s:%d: %d map candidates for %s (%s)"
                % (
                    marker["file"],
                    marker["marker_line"],
                    len(candidates),
                    friendly_name(marker["cls"], marker["name"]),
                    prefix,
                )
            )
            continue
        marker["sym"] = candidates[0]
    return errors


# ---------------------------------------------------------------------------
# Friendly names for display

_MEMBER = re.compile(r"^\?(\w+)@(\w+)@@")
_FREE = re.compile(r"^\?(\w+)@@")
_CTOR = re.compile(r"^\?\?0(\w+)@@")
_DTOR = re.compile(r"^\?\?(?:1|_G)(\w+)@@")


def undecorate(name):
    """Best-effort undecoration for the simple symbols in our own code.
    Anything we do not recognize is returned as-is."""
    for regex, fmt in (
        (_CTOR, "{0}::{0}"),
        (_DTOR, "{0}::~{0}"),
        (_MEMBER, "{1}::{0}"),
        (_FREE, "{0}"),
    ):
        m = regex.match(name)
        if m is not None:
            return fmt.format(*m.groups())
    return name


# ---------------------------------------------------------------------------
# Vtable sizes

_TEXT_SECTION = 1


def exact_vtable_sizes(map_symbols, exe_path):
    """{address: byte length} for every `??_7Foo@@6B@` in the image.

    A vtable's size is otherwise estimated from the distance to the next
    symbol in the linker map, which overshoots by the alignment padding and
    by any unnamed object in between. reccmp compares as many slots as that
    size allows and warns "Recomp vtable is larger than orig vtable" for
    every one of them; and the estimated size also masks the float and string
    constants that follow. The real length is simply the run of pointers into
    .text at the symbol's address.
    """
    try:
        from reccmp.formats import detect_image

        image = detect_image(filepath=str(exe_path))
    except Exception:  # not built yet, or an image reccmp cannot read
        return {}
    text_start = image.get_section_offset_by_index(_TEXT_SECTION)
    text_end = text_start + image.get_section_extent_by_index(_TEXT_SECTION)
    sizes = {}
    for sym in map_symbols:
        if not sym["name"].startswith("??_7"):
            continue
        size = 0
        while True:
            raw = image.read(sym["addr"] + size, 4)
            if raw is None or len(raw) != 4:
                break
            (value,) = struct.unpack("<I", raw)
            if not text_start <= value < text_end:
                break
            size += 4
        if size:
            sizes[sym["addr"]] = size
    return sizes


# ---------------------------------------------------------------------------
# Build the CvdumpParser


def build_parser(map_path, src_root):
    map_symbols = parse_map(map_path)
    markers = scan_markers(src_root)
    errors = resolve_markers(markers, map_symbols)
    for err in errors:
        print(f"[vc2map] {err}", file=sys.stderr)

    parser = CvdumpParser()

    # PUBLICS: every map symbol. Drives node creation, vtable/string/float
    # detection in CvdumpNode.set_decorated.
    for s in map_symbols:
        parser.publics.append(
            PublicsEntry(
                type="S_PUB32",
                section=s["section"],
                offset=s["offset"],
                flags=0,
                name=s["name"],
            )
        )

    # SECTION CONTRIBUTIONS: estimate sizes from the distance to the next
    # symbol in the same section, except for vtables, whose real length we
    # can read out of the image (see exact_vtable_sizes).
    exact = exact_vtable_sizes(map_symbols, PurePath(map_path).with_suffix(".exe"))
    by_section = {}
    for s in map_symbols:
        by_section.setdefault(s["section"], []).append(s)
    for section, syms in by_section.items():
        syms.sort(key=lambda s: s["offset"])
        for cur, nxt in zip(syms, syms[1:]):
            size = min(nxt["offset"] - cur["offset"],
                       exact.get(cur["addr"], 1 << 30))
            if size > 0:
                cur["est_size"] = size
                parser.sizerefs.append(
                    SizeRefEntry(
                        module=0,
                        section=section,
                        offset=cur["offset"],
                        size=size,
                    )
                )

    # SYMBOLS (S_GPROC32): friendly names + sizes for every .text symbol.
    for s in by_section.get(_TEXT_SECTION, []):
        parser.symbols.append(
            SymbolsEntry(
                type="S_GPROC32",
                section=s["section"],
                offset=s["offset"],
                size=s.get("est_size", 0),
                func_type=CvdumpTypeKey(0x74),  # T_INT4, unused
                name=undecorate(s["name"]),
            )
        )

    # GLOBALS: the two marked globals. Use a non-scalar type key so the
    # types lookup misses and the estimated size (map spacing) is used.
    resolved = [m for m in markers if "sym" in m]
    for m in resolved:
        if m["kind"] != "GLOBAL":
            continue
        parser.globals.append(
            GdataEntry(
                section=m["sym"]["section"],
                offset=m["sym"]["offset"],
                type=CvdumpTypeKey(0x1000),
                name=m["name"],
                is_global=True,
            )
        )

    # LINES: synthetic entries at the marker and declaration lines so
    # reccmp's LinesDb can resolve each marker to its map address.
    for m in resolved:
        if m["kind"] == "GLOBAL":
            continue
        key = PureWindowsPath(str(m["file"]).replace("/", "\\"))
        for line_no in (m["marker_line"], m["decl_line"]):
            parser.lines.setdefault(key, []).append(
                LineValue(line_no, m["sym"]["section"], m["sym"]["offset"])
            )

    n_func = sum(1 for m in resolved if m["kind"] != "GLOBAL")
    n_glob = sum(1 for m in resolved if m["kind"] == "GLOBAL")
    print(
        f"[vc2map] {len(map_symbols)} map symbols, resolved {n_func} functions"
        f" + {n_glob} globals from {len(markers)} markers"
        + (f" ({len(errors)} unresolved)" if errors else ""),
        file=sys.stderr,
    )
    return parser


# ---------------------------------------------------------------------------
# Shim installed in place of reccmp.compare.core.Cvdump


class MapCvdump:
    """Drop-in replacement for reccmp's Cvdump runner.

    The real Cvdump runs cvdump.exe on the PDB; we build the same
    CvdumpParser from the linker map file and our source markers."""

    def __init__(self, pdb_path: str) -> None:
        self._map_path = str(PurePath(pdb_path).with_suffix(".map"))

    # Fluent interface, all no-ops.
    def lines(self):
        return self

    def symbols(self):
        return self

    def globals(self):
        return self

    def publics(self):
        return self

    def section_contributions(self):
        return self

    def modules(self):
        return self

    def types(self):
        return self

    def run(self) -> CvdumpParser:
        return build_parser(self._map_path, "src")


def install():
    """Replace reccmp.compare.core.Cvdump with the map-file shim."""
    import reccmp.compare.core as core

    core.Cvdump = MapCvdump
