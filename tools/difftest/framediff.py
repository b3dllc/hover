#!/usr/bin/env python3
"""Differential frame test: compare capture sets from two binaries.

    tools/framediff.py PREFIX_A PREFIX_B [--grid N] [--out DIR] [--regions]

For every stage both prefixes captured (PREFIX_<stage>.png), reports the
number of differing pixels (ImageMagick AE) and the RMSE over the whole
frame, then an N x N grid of per-cell differing-pixel counts so a hotspot is
visible without hand-picking rectangles.  With --out, writes a highlighted
diff image per stage.  Run it first on two captures of the SAME binary to
learn the noise floor (robots move, timers tick) before reading A-vs-B.
"""
import os
import re
import subprocess
import sys


def run(*cmd):
    p = subprocess.run(cmd, capture_output=True, text=True)
    return (p.stdout + p.stderr).strip()


def metric(a, b, kind):
    out = run("compare", "-metric", kind, a, b, "null:")
    m = re.search(r"([\d.]+(?:e[+-]?\d+)?)", out)
    return float(m.group(1)) if m else float("nan")


def size(a):
    return run("identify", "-format", "%w %h", a).split()


# Window-relative regions as fractions (x0, y0, x1, y1).  The start position
# varies run to run, so the 3D view and the mirror/minimap cannot be compared
# pixel for pixel; the HUD panels either side of them can.
REGIONS = {
    "hud-left":   (0.00, 0.05, 0.30, 0.22),   # flags box
    "hud-right":  (0.70, 0.05, 1.00, 0.22),   # score
    "dash-left":  (0.00, 0.74, 0.37, 1.00),   # gauges A/S/D
    "dash-right": (0.63, 0.74, 1.00, 1.00),   # speed dial, bars
    "view":       (0.00, 0.22, 1.00, 0.74),   # 3D view (histogram only)
}


def crop(img, w, h, frac, dst):
    x0, y0, x1, y1 = frac
    geom = "%dx%d+%d+%d" % (int((x1 - x0) * w), int((y1 - y0) * h), int(x0 * w), int(y0 * h))
    run("convert", img, "-crop", geom, "+repage", dst)


def histogram(img):
    out = run("convert", img, "-format", "%c", "histogram:info:-")
    hist = {}
    for line in out.splitlines():
        m = re.match(r"\s*(\d+):\s*\(([^)]*)\)", line)
        if m:
            hist[m.group(2)] = hist.get(m.group(2), 0) + int(m.group(1))
    return hist


def hist_distance(ha, hb):
    tot = max(1, sum(ha.values()) + sum(hb.values()))
    keys = set(ha) | set(hb)
    return sum(abs(ha.get(k, 0) - hb.get(k, 0)) for k in keys) / tot


def main():
    args = [a for a in sys.argv[1:] if not a.startswith("--")]
    grid = 4
    out = None
    if "--grid" in sys.argv:
        grid = int(sys.argv[sys.argv.index("--grid") + 1])
        args = [a for a in args if a != str(grid)]
    if "--out" in sys.argv:
        out = sys.argv[sys.argv.index("--out") + 1]
        args = [a for a in args if a != out]
        os.makedirs(out, exist_ok=True)
    pa, pb = args[0], args[1]
    stages = []
    for f in sorted(os.listdir(os.path.dirname(pa) or ".")):
        base = os.path.basename(pa) + "_"
        if f.startswith(base) and f.endswith(".png"):
            st = f[len(base):-4]
            if os.path.exists("%s_%s.png" % (pb, st)):
                stages.append(st)
    for st in stages:
        a, b = "%s_%s.png" % (pa, st), "%s_%s.png" % (pb, st)
        wa, wb = size(a), size(b)
        if wa != wb:
            print("%-8s size differs: %s vs %s" % (st, "x".join(wa), "x".join(wb)))
            continue
        w, h = int(wa[0]), int(wa[1])
        ae = metric(a, b, "AE")
        rmse = metric(a, b, "RMSE")
        print("%-8s %dx%d  differing pixels %6.0f (%.2f%%)  rmse %.4f"
              % (st, w, h, ae, 100.0 * ae / (w * h), rmse))
        rows = []
        for gy in range(grid):
            cells = []
            for gx in range(grid):
                cw, ch = w // grid, h // grid
                geom = "%dx%d+%d+%d" % (cw, ch, gx * cw, gy * ch)
                ca, cb = "/tmp/_fd_a.png", "/tmp/_fd_b.png"
                run("convert", a, "-crop", geom, "+repage", ca)
                run("convert", b, "-crop", geom, "+repage", cb)
                cells.append(metric(ca, cb, "AE"))
            rows.append(cells)
        for r in rows:
            print("         " + " ".join("%6.0f" % c for c in r))
        if out:
            run("compare", a, b, "-highlight-color", "red", "-lowlight-color", "white",
                os.path.join(out, "diff_%s.png" % st))
        if "--regions" in sys.argv:
            for name, frac in REGIONS.items():
                ca, cb = "/tmp/_fd_ra.png", "/tmp/_fd_rb.png"
                crop(a, w, h, frac, ca)
                crop(b, w, h, frac, cb)
                if name == "view":
                    print("         %-11s colour-histogram distance %.3f  (0 = identical palette use)"
                          % (name, hist_distance(histogram(ca), histogram(cb))))
                else:
                    cw, ch = size(ca)
                    aer = metric(ca, cb, "AE")
                    print("         %-11s differing pixels %6.0f (%.2f%%)"
                          % (name, aer, 100.0 * aer / (int(cw) * int(ch))))


main()
