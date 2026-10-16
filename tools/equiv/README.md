# Differential emulation: functional equivalence per function

Byte similarity cannot show that a non-matching function *behaves* like the
original.  `equiv.py` runs each original function (in `hover.exe`) and its
recompiled twin (in `build/hover.exe`) side by side under
[Unicorn](https://www.unicorn-engine.org/) on many identical pseudo-random
inputs, and compares everything a caller could observe.

    tools/equiv/run.sh                     # build, refresh inputs, check every non-matching function
    tools/equiv/run.sh 0x40f7f0 0x402250   # just these (original addresses)
    tools/equiv/equiv.py --trace 3 0x40f7f0   # every event of both sides for seed 3

`run.sh` regenerates `build/compare.json` (reccmp scores, used to skip 100%
matches), `build/matches.json` (`dump_matches.py`: every orig <-> recomp
address pair reccmp knows) and, once, `build/winpops.json` (`winpops.py`:
callee-popped bytes of every Win32 import, read from Wine's i386 DLLs).
Needs `pip install unicorn numpy` in the reccmp venv.

## What is compared

Each function runs in isolation: every call it makes is intercepted at the
call instruction and *stubbed* (the callee is assumed to match - it has its
own entry).  Some callees run instead of being stubbed:

- pure CRT helpers (`__ftol`, `__CIsqrt`, the 64-bit arithmetic routines,
  the Pentium FDIV workarounds);
- small callees neither image has a match for (per-file copies of inline
  functions, compiler-generated destructors reached from unwind funclets)
  and small matched leaf functions;
- a matched callee only one side calls: the other side's compiler expanded
  it inline (hover.exe's `/Ob2` modules), so it runs inline where it is
  called and both expansions are compared.  This is decided to a fixpoint,
  so nested expansions (a derived constructor inlining its base) line up.

The observable behaviour is a list of events:

- **call**: the callee (canonicalised to the original's address, the import
  name, or a relocation-independent code fingerprint for unnamed library
  functions), `this` for thiscall callees (a virtual call is recognised by
  its vptr being the dword at `[ecx]`), and the stack arguments (a
  WORD/short/BYTE parameter compares only its declared width, read from
  the source declaration);
- the bytes written to non-stack memory since the previous event, attached
  to each call (a store cannot legally move across an opaque call);
- **ret**: eax (or st0 for float/double functions, from the source
  declaration; nothing for void), and the bytes popped by `ret n`;
- **fault** / **intr**: a stop on a null-page access or a divide by zero.

Values are compared raw *or* translated: a number inside our image may be a
pointer, so it also compares equal to the original's corresponding address
(matched entity, unique string, pointer table such as a vtable or
`CRuntimeClass` located by its translated slots, or an offset from a matched
global for a pointer walked past an array's end).

## Inputs

Everything the function reads is a deterministic function of (seed,
address): `this`, the stack arguments, every matched global, every heap page
(mapped lazily on first touch), and every stub's return value.  Values mix
zero, small and negative integers, floats, heap pointers, full-range
randoms, and the immediates the two functions themselves compare against.

The generator learns per seed: when a run faults on a null-page access,
the base register's value is traced back to the dword (or stub return)
that produced it, that location is forced to a valid pointer on both sides,
and the seed is rerun.  A zero divisor is forced nonzero, and an index that
runs a table access off the end of the table it names is forced into range,
the same way.  A stub writes a value through every argument that points
into the caller's frame (a dword, or a known API's struct), so out-parameters
are not left as stack fill.

After the random seeds, **branch-directed inputs** go after what they
missed: for each conditional jump from a covered block to an uncovered one,
the comparison feeding it is traced to its input (through a pointer, as an
offset from the pointer's source, so it survives pointer learning) and
forced to the compared constant and its neighbours; the frontier is
recomputed after every run that reaches something new (`D` in the output).

## Reading the output

    0x40f7f0  85.4% DIFF  11/64 cov 9/9 8/8 CWall::Sub_411570
          seed 2 event 1:
            orig: ret value=0x0 pop=16 writes={...}
            ours: ret value=0x1 pop=16 writes={...}

`DIFF n/m` - n of the m conclusive runs diverged; the first divergence is
shown.  `ok` - every conclusive run agreed (`weak` if fewer than 16 were
conclusive).  `cov a/b c/d` - basic blocks reached in the original and in
ours across all runs: a high ratio is what makes `ok` meaningful.  Flags:

- `T` runs hit the instruction budget (only the common prefix is compared);
- `F` runs where both sides faulted (only the calls before it count);
- `W` runs set aside as inconclusive because a random pointer or index
  walked into one binary's private layout (a register-based address outside
  every matched global, an array access leaving the global it names, a
  register loaded from heap memory used to reach into an image, or a heap
  access no pointer the run has seen explains); `(n one-sided)` when only
  one side did - worth a look;
- `U` runs whose learning had not settled when the attempts ran out (still
  compared: every attempt gives both sides the same inputs);
- `D` branch-directed runs;
- `stack?` the static stack-depth solver could not make the function's
  depths consistent, so stubbed callee pops may be wrong.

`--trace SEED ADDR` prints both event lists with call-site addresses and,
for a write that differs, the instruction that wrote each byte.
`--trace SEED 0xORIG:0xOURS` does the same for two functions neither image
has a match for (a compiler-generated destructor, say).

A pointer into the function's own frame passed to a callee (`&rc`, a local
string) is compared together with what the function stored behind it: the
struct an API reads (`IN_SIZES`: `ClientToScreen`'s POINT, `FillRect`'s
RECT), a string through its NUL for any other import argument not known to
be a pure output (`OUT_SIZES`, `OUT_BUFS`), and the first 16 bytes both
sides wrote for a game callee.  A vptr stored into an object just before a
destructor is called on it is dead (every destructor stores its own first)
and is not compared.

## Static data: `datadiff.py`

    tools/equiv/datadiff.py [-v] [--tails] [--lib] [--json OUT]

The emulator checks what functions do; this checks the data they read.
Every matched function pair is disassembled and the two instruction streams
aligned; each relocated operand pairs with the one at the same place on the
other side, naming the same object.  Each pair's bytes are compared up to the
next address anything references, pointers inside it must name the same
thing (the same matched function, or an object pair compared in turn: a
vtable's slots, a string table, a `CRuntimeClass`), and `.bss` reads as
zeros, so a global the original initialises and ours does not shows up.
Addresses no aligned instruction pairs are translated through the matched
global holding them, or found by content.  Two unmatched functions behind a
pointer (deleting destructors, `CRuntimeClass::m_pfnConstruct`) must have the
same instructions in any order, or else pass the emulator.  The last line
says how many of the data addresses game code names were compared.
It reads the same inputs as `equiv.py` (`build/matches.json` and
`build/winpops.json`); after a clean rebuild, run `tools/equiv/run.sh`
once or regenerate them by hand as that script does.

## Mutation testing: `mutate.py`

    tools/equiv/mutate.py [--per N] [-j N] [--json OUT] [0xADDR ...]

How good is an `ok`?  This plants single-instruction bugs in our compiled
function (in memory - no rebuild) and checks each mutant exactly as the real
function is checked: a conditional jump moved across its boundary
(`<` for `<=`) or inverted, an immediate changed by one, a non-stack store
removed, a displacement moved to the next field, two pushed arguments
swapped.  Only instructions in blocks the clean check covered are mutated.
A mutant reported divergent is *killed*.  A survivor is either an equivalent
mutant (a store the function overwrites, a boundary the values never reach)
or a blind spot of the harness - each one is worth a look.

## Unreached code: `semdiff.py`

    tools/equiv/semdiff.py --equiv build/equiv.json [-v] [0xADDR ...]

`equiv.py --json` records each function's basic blocks and the ones no
conclusive run reached.  `semdiff.py` takes reccmp's alignment of each
non-matching function (build/compare.json) and keeps only the instructions
that differ *and* lie in an unreached block: the only code the emulator
gives no evidence for.  `-v` lists them for review; the summary lines show
semantic atoms (constants, field offsets, symbols, signed/unsigned operations
and condition families) that do not cancel between the two sides.  Without
`--equiv` it scans every differing instruction, which is mostly noise
(register caching splits field offsets differently on the two sides).

`equiv.py --json OUT` also writes `OUT.partial`, one result per line as
they arrive, and a pool worker that dies (OOM) has its function reported as
a harness error instead of hanging the run.

## Limits

A clean run is strong evidence, not proof: blocks the inputs never reach are
unchecked (see `cov`), stubs model no callee side effects beyond writing
out-parameters, and floating-point results are compared bit for
bit, so a different but harmless evaluation order of x87 code shows up as a
divergence.  Globals that are not annotated on both sides (statics never
reach the map file) cannot be translated; a divergence involving one usually
means a missing `// GLOBAL:` marker.
