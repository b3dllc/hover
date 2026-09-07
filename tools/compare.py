#!/usr/bin/env python3
"""Compare the recompiled hover.exe against the original using reccmp.

Wrapper around reccmp's comparison tools with one workaround: the PDB
produced by MSVC 4.2 attributes a function's start address to a line slightly
*above* the function signature (observed 1-3 lines, apparently the end of the
preceding code region). reccmp's LinesDb.find_function only searches the line
range of the function itself, so the lookup fails. We patch find_function to
retry a few lines above when the exact search finds nothing.

Usage (run from anywhere; the script cd's to the repository root):

    tools/compare.py [reccmp-reccmp args]   compare annotated functions
    tools/compare.py --data [args]          compare annotated globals (datacmp)
    tools/compare.py --vtable [args]        compare annotated vtables

Examples:

    tools/compare.py                        # summary of all function matches
    tools/compare.py --verbose 0x41c100     # asm diff for one function
    tools/compare.py --data                 # check g_lvlParams initializers
    tools/compare.py --lib                  # include the library functions
"""

import os
import sys
from pathlib import Path

from reccmp.compare.lines import LinesDb

_orig_find_function = LinesDb.find_function

# How far above the function signature the MSVC 4.2 PDB may place the
# function's first line entry.
_LINE_MARGIN = 4


def _find_function_with_margin(self, local_path, line_start, line_end=None, *, folded=False):
    if line_end is None:
        line_end = line_start
    # Exact range first, then the margin above, quietly.
    for start, end in (
        (line_start, line_end),
        (max(1, line_start - _LINE_MARGIN), line_start - 1),
    ):
        possible = set(self.search_line(local_path, start, end, start_only=True))
        if len(possible) == 1:
            return next(iter(possible))
    # Fall back to the original implementation for standard error reporting.
    return _orig_find_function(self, local_path, line_start, line_end, folded=folded)


LinesDb.find_function = _find_function_with_margin


def main():
    os.chdir(Path(__file__).resolve().parent.parent)

    # The bundled cvdump.exe cannot read the PDB 2.00 files written by the
    # VC++ 2.2 linker, so reccmp's symbol/line data is synthesized from the
    # linker map file and our source markers instead (see vc2map.py).
    import vc2map

    vc2map.install()

    # reccmp reads as many original bytes as our function occupies unless it
    # knows the original size; register sizes derived from the original's
    # layout so comparisons are neither truncated nor run into the neighbour
    # (see origsize.py).
    import origsize
    from reccmp.compare.core import Compare

    # reccmp only names a float constant when nothing else in the database
    # already covers its address. In the original that is always true (we have
    # no symbols there), but in our build every .rdata float sits inside the
    # *max size* of the preceding linker-map symbol, so the recompiled side of
    # every FP instruction is printed as an unresolved <OFFSETn> and scored as
    # a diff even when the constant is identical (cf. rule 31). Only a symbol
    # with a real, known size should mask a float.
    import reccmp.compare.core as _reccmp_core
    from reccmp.analysis import find_float_consts as _find_float_consts
    from reccmp.types import EntityType as _EntityType

    def _create_analysis_floats(db, img_id, binfile):
        with db.batch() as batch:
            for addr, size, value in _find_float_consts(binfile):
                if db.exists(img_id, addr):
                    continue
                prev = db.get(img_id, addr, exact=False)
                if prev is not None:
                    base = prev.addr(img_id)
                    declared = prev.get("size")
                    if declared is not None and addr - base < declared:
                        continue
                batch.set(
                    img_id,
                    addr,
                    type=_EntityType.FLOAT,
                    name=str(value),
                    size=size,
                )

    _reccmp_core.create_analysis_floats = _create_analysis_floats

    # Same problem for string constants. reccmp sizes a recompiled string
    # from its linker-map symbol, whose "size" is the distance to the next
    # symbol and therefore includes the linker's alignment padding, so the
    # name it builds for our side carries one or two trailing NULs
    # ("SoundSettings\x00\x00") while the original's, read with
    # read_string, does not - and every `push <string>` scores as a diff.
    # Trailing NULs are never part of a C string's identity: drop them on
    # both sides before the name is built.
    # Strict effective matches: reccmp's "effective match" (the asterisk,
    # counted as 100% in the totals) forgives instruction relocation, which
    # lets two adjacent argument pushes trade places unnoticed.  Push order
    # is the argument order under stdcall/cdecl, so a moved push is never
    # harmless (it hid the swapped Sub_410F20 arguments, PR #140).  Refuse to
    # relocate push lines unless RECCMP_LAX_PUSH=1 is set.
    import reccmp.compare.asm.fixes as _reccmp_fixes
    if os.environ.get("RECCMP_LAX_PUSH") != "1":
        _orig_relocate = _reccmp_fixes.relocate_instructions

        def _strict_relocate(codes, orig_asm, recomp_asm):
            moved = _orig_relocate(codes, orig_asm, recomp_asm)
            # A push that only moved past non-push instructions keeps the
            # argument order; only a changed push *sequence* is an argument
            # swap, and then no push relocation may be forgiven.
            pushes = lambda asm: [l for l in asm if l.lstrip().startswith("push")]
            if pushes(orig_asm) == pushes(recomp_asm):
                return moved
            return {j for j in moved if not recomp_asm[j].lstrip().startswith("push")}

        _reccmp_fixes.relocate_instructions = _strict_relocate

    import reccmp.compare.db as _reccmp_db
    import reccmp.compare.analyze as _reccmp_analyze
    import reccmp.compare.ingest as _reccmp_ingest

    _orig_entity_name_from_string = _reccmp_db.entity_name_from_string

    def _entity_name_from_string(text, wide: bool = False) -> str:
        return _orig_entity_name_from_string(text.rstrip("\x00"), wide)

    _reccmp_db.entity_name_from_string = _entity_name_from_string
    _reccmp_analyze.entity_name_from_string = _entity_name_from_string
    _reccmp_ingest.entity_name_from_string = _entity_name_from_string

    _orig_run = Compare.run

    def _run_with_orig_sizes(self):
        _orig_run(self)
        origsize.install(self, self.orig_bin.filepath)

    Compare.run = _run_with_orig_sizes

    args = sys.argv[1:]
    tool = "asmcmp"
    for flag, name in (("--data", "datacmp"), ("--vtable", "vtable")):
        if flag in args:
            args.remove(flag)
            tool = name

    # tools/orig_library.csv names the original's statically linked MFC/CRT
    # functions (see tools/libmatch.py) so that a library call scores as a
    # match on both sides. Those functions are not our code, so they stay out
    # of the report unless --lib is given.
    if tool == "asmcmp":
        if "--lib" in args:
            args.remove("--lib")
        elif "--nolib" not in args:
            args.append("--nolib")

        # tools/orig_data.csv (tools/datamatch.py) names the original's
        # vtables, so reccmp now matches a hundred of them - and asmcmp's
        # compare_all() prints one line per matched *entity*, vtables
        # included. A vtable is not a function, contributes nothing to
        # Implemented/Accuracy/Progress, and has its own report:
        # `tools/compare.py --vtable`. Keep this listing to functions.
        _compare_all = Compare.compare_all

        def _functions_only(self):
            for diff in _compare_all(self):
                if diff.match_type != _EntityType.VTABLE:
                    yield diff

        Compare.compare_all = _functions_only

    sys.argv = [f"reccmp-{tool}", "--target", "HOVER", *args]

    if tool == "datacmp":
        from reccmp.tools.datacmp import main as tool_main
    elif tool == "vtable":
        from reccmp.tools.vtable import main as tool_main
    else:
        from reccmp.tools.asmcmp import main as tool_main

    return tool_main()


if __name__ == "__main__":
    sys.exit(main() or 0)
