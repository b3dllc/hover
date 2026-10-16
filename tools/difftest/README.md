# Differential frame test against the original binary

Runs the original `hover.exe` and our build under identical scripted input
and compares the captured frames pixel for pixel.  A difference is a
reproducible fault with a picture attached, and it catches semantic
divergence that no static comparison can see.

The game seeds `rand()` from `time()` at two sites (the maze loader picks the
level and the spawn points from it), so two runs never match by default.
`pepatch.py` rewrites the CRT `time()` body in a *test copy* of each binary
to return a constant, which makes every seed identical; nothing in the
shipped build is touched.

    # test copies (never the real binaries)
    cp -r /workspace/hover fd/orig; cp -r /workspace/hover fd/ours
    cp build/hover.exe fd/ours/hover.exe
    python3 pepatch.py fd/orig/hover.exe 0x43e9c2 b8393000008b4c240485c974028901c3   # _time in hover.exe
    python3 pepatch.py fd/ours/hover.exe 0x<our _time> b8393000008b4c240485c974028901c3  # from build/hover.map
    # capture (run under reap.py so Wine leaves no zombies; separate WINEPREFIX)
    WINEPREFIX=/tmp/wp8 python3 reap.py ./framecap.sh fd/ours fd/oursA
    WINEPREFIX=/tmp/wp8 python3 reap.py ./framecap.sh fd/orig fd/origA
    WINEPREFIX=/tmp/wp8 python3 reap.py ./framecap.sh fd/ours fd/oursB   # noise floor
    python3 framediff.py fd/oursA fd/oursB --regions        # what timing alone changes
    python3 framediff.py fd/oursA fd/origA --regions --out diffs

The patch bytes are `mov eax,0x3039; mov ecx,[esp+4]; test ecx,ecx; jz +2;
mov [ecx],eax; ret`, which honours `time(&t)` as well as `time(NULL)`.

Stages: `menu`, `start3`/`start10` (first in-game frames, static camera),
`turn` (eight taps of Right), `drive`/`drive8` (three seconds of Up, then
eight seconds later).  `menu`, `start3`, `start10` are deterministic; `turn`
differs by tens of pixels of key-timing jitter; the `drive` stages depend on
how far the craft travelled under Wine scheduling and only compare exactly
when that coincides, so read them against the noise floor.
