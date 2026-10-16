#!/usr/bin/env python3
"""Dump reccmp's entity database (every orig <-> recomp match, plus the
unmatched entities of both images) to JSON for tools/equiv/equiv.py.

Runs the same reccmp setup as tools/compare.py (vc2map bridge, origsize,
library and data CSVs) and writes the database instead of a report.

    /workspace/tools/venv/bin/python tools/equiv/dump_matches.py build/matches.json
"""
import json
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent.parent
sys.path.insert(0, str(ROOT / "tools"))

OUT = Path(sys.argv[1] if len(sys.argv) > 1 else ROOT / "build" / "matches.json").resolve()
sys.argv = [sys.argv[0]]

import compare  # noqa: E402  (tools/compare.py)
from reccmp.compare.core import Compare  # noqa: E402
from reccmp.types import ImageId  # noqa: E402


def _dump(self):
    rows = []
    for e in self._db.get_all():
        rows.append(
            {
                "orig": e.orig_addr,
                "recomp": e.recomp_addr,
                "type": e.entity_type,
                "name": e.best_name(),
                "orig_size": e.size(ImageId.ORIG) or e.get("size"),
                "recomp_size": e.size(ImageId.RECOMP) or e.get("size"),
                "library": bool(e.get("library")),
            }
        )
    OUT.write_text(json.dumps(rows))
    print(f"wrote {len(rows)} entities to {OUT}")
    sys.exit(0)


Compare.compare_all = _dump
compare.main()
