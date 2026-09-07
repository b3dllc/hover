# Path to 100% byte match

Why: percentage similarity cannot prove the decompilation is faithful. Only
byte equality with the original can. This note records what stands between us
and a byte-exact build, measured 2026-08-20, and the strategy to get there.

## Where we stand

- 2026-09-04: 363 functions annotated (3 stubs), overall reccmp accuracy 69%,
  49 functions at 100%; measured 2026-08-20: 347 annotated, 68%. A handful of
  functions are at `100.00%*` (reccmp "effective match": identical instruction
  stream modulo register allocation, cmp operand swaps, and reordering).
- Section-level: `.rsrc` is already exactly the original size (0x24370);
  `.text` is 0x441ec vs the original's 0x5e56e because ~65% of game code is
  still stubbed; both binaries carry a `.reloc` section.
- Incremental build + diff cycle is ~5 seconds (`./build.sh` then
  `tools/compare.py --verbose 0xADDR`), so trial-and-error on individual
  functions is cheap.

## Diff taxonomy (what actually differs today)

Looking at verbose diffs of near-miss functions, all differences fall into
four classes:

1. **String-pool layout.** The most common diff by far: `push "WavMix32"` in
   the original vs `push "WavMix32\x00\x00"` in ours. Both pools pad literals
   to 4 bytes; the difference is pool *order* (string N+1 differs because our
   module's first-use order differs). This is a data-layout artifact, not a
   codegen bug: it resolves only when the module is fully implemented *and*
   functions appear in the same order as the original. Per-function, these
   diffs are expected and acceptable for now.

2. **Register-allocation cascades.** The original keeps a value in `ebx` where
   we use `edi` (e.g. Hover_sub_439900). This is driven by variable live
   ranges, i.e. by source structure — fixing it means matching the original's
   construct, not flags. reccmp tolerates pure register substitution in its
   effective-match check, so this class matters mostly for final byte equality.

3. **cmp operand orientation.** VC 2.2 emits `cmp a,b / jcc` vs the swapped
   `cmp b,a / jcc'` depending on the *construct*, not on source operand order
   (the compiler canonicalizes `a < b` and `b > a` identically). Measured with
   the probe harness (tools/probe):
   - `if (i < n) { do ... while (i < n); }`: entry check emitted reversed
     (`cmp n,i / jle`); folds to `test n,n / jle` when the bound is in a
     register and the counter is provably 0.
   - `for (i = 0; i < n; ...)`: back-edge and post-loop checks emitted in
     natural order (`cmp i,n / jl`), entry folds when possible.
   - /O1 vs /O2 vs /O2b0/b1 makes no difference to this behavior.
   reccmp's effective-match rules tolerate cmp+jcc swaps, so chase these only
   for exact (non-asterisk) matching.

4. **Library drift.** *Resolved 2026-09-04: the original links MFC 3.1 (VC++
   2.1), whose compiler/CRT are byte-identical to 2.2's; the build now uses
   the 2.1 compiler and MFC with the Win95 SDK headers (see build.sh). Every
   MFC function that drifted under 3.2 is exact under 3.1; the 76 "different"
   entries left are identical under 2.0/2.1/2.2 and are therefore
   misidentified fingerprints from the coverage scan, not drift. The
   optimizer differences below (rules 6, 9, 11, 13 and the frame gaps) are
   consequently NOT version drift: the original used the same compiler, so
   they must come from compiler flags or constructs still unknown.*
   Original text: Our MFC 3.2/CRT (VC++ 2.2) vs whatever minor version the
   original linked. Measured with tools/libdiff.py over the 336 "fuzzy twin"
   functions from the coverage analysis:
   - 207/336 are byte-exact already (the fuzzy rating was a fingerprint
     artifact — e.g. `__adj_fdiv_r` is identical).
   - 22 show real 1-6-instruction drift (e.g. `CWinThread::IsIdleMessage`
     differs by one cmp operand order, i.e. the original's MFC source had the
     comparison written the other way — a genuine MFC minor-version
     difference).
   - The remainder are mostly CRT/EH-runtime functions with larger drift
     (`_WinMainCRTStartup`, `__FrameUnwindToState`, `?BuildCatchBlock@@...`),
     plus name-collision artifacts (duplicate `??1CPtrList@@` symbols).
   Note: coverage.md's claim "linker 2.55 (VC++ 2.1)" is unproven — our VC++
   2.2 ISO ships LINK 2.55 too, so the PE stamp does not distinguish 2.1 from
   2.2. The small MFC source drift suggests the original may be MFC 3.1; the
   linker stamp cannot tell us.

## Milestones

**A. Per-function effective match (100%\*) — game code.**
For every game function, the instruction stream must match modulo addresses,
register substitution, and cmp swaps. Workflow:
1. `tools/compare.py --verbose 0xADDR` shows the residual diffs.
2. Classify each diff: string-push (ignore for now), regalloc, cmp-swap
   (tolerated), or a genuine logic/codegen difference.
3. For codegen differences, use the probe harness (`tools/probe/probe.sh
   file.c`) to test which C construct produces the original pattern, then
   rewrite the source with that construct.
4. Record every new compiler rule discovered in this file (append below) so
   the same pattern is never reverse-engineered twice.

**B. Layout match.**
Once all game functions in a module are implemented:
1. Order functions within each `.cpp` by original address (markers give us the
   original layout), and order `OBJS` in the Makefile so object order matches
   the original `.text` order (game code first: hover.exe lays out
   module-by-module).
2. The string pool order follows from first-use order in each module — this
   clears class-1 diffs and turns effective matches into exact matches.
3. Verify section sizes match (`.text`, `.rdata`, `.data`, `.idata`,
   `.reloc`), then byte-compare whole sections.

**C. Library drift resolution.**
Game code is fully under our control; precompiled lib bytes are not. Options,
in increasing effort:
1. Accept the ~22 small + ~dozen CRT/EH drifted functions as documented
   exceptions (list them from `tools/libdiff.py`).
2. Identify the original's exact MFC/CRT minor version from the drift
   fingerprint (e.g. IsIdleMessage's operand order) and, if it is MFC 3.1 /
   VC++ 2.1, source that toolchain.
3. Rebuild NAFXCW.LIB/LIBCMT.LIB from the shipped MFC/CRT sources
   (MFC/SRC, AFXCMN.MAK) with targeted reversions of the drifted functions —
   the diffs tell us exactly what the older source looked like.

## Open questions

- **0x439c20 entry-check construct**: the original gwb loop has an entry
  `cmp i,n / jge` that is *not* constant-folded even though `i=0` is visible
  and the bound is in a register. Neither if+do-while (folds, reversed) nor
  for-loop (folds, natural) reproduces it in context. Candidates not yet
  tried: guard reached from an outer-loop back-edge (defeats folding),
  strength-reduced `base[i]` indexing with a separate counter, or an
  intervening call between `i=0` and the check in the original source.
- **Exact original MFC/CRT version**: the drift set is small enough to serve
  as a fingerprint if we ever get access to MFC 3.0/3.1 libs for comparison.
- ~~**0x426f20 dead frame slots**~~ RESOLVED by rule 258: a `volatile`
  padding array (rule 80's shape, not the unreferenced-array probe this
  question originally cited) reproduces the six untouched dword slots.
- **reccmp string tolerance**: string-push diffs block `100%*` for
  string-using functions. Either patch our compare wrapper to normalize
  string-pool padding, or accept `<100%` on those functions until milestone B.
  (Currently unpatched; the two existing 100%* functions simply contain no
  string pushes.)

## Tooling

- `tools/probe/probe.sh file.c [/Oflags]` — compile a snippet with MSVC 2.2
  under Wine and disassemble it; the way to answer "what construct produced
  this pattern?" in seconds. `probe.c` / `probe2.c` hold the loop-construct
  probes used for the rules above.
- `tools/libdiff.py` (+ `libdiff-fuzzy.tsv`) — quantifies library drift
  between the original and our build's NAFXCW/LIBCMT code.
- `tools/libmatch.py` / `tools/datamatch.py` — recover names for the
  original's library *functions* and for its *data*; they write
  `tools/orig_library.csv` and `tools/orig_data.csv`, the two reccmp data
  sources listed in `reccmp-project.yml` (rules 239, 240). `--show` explains
  one address; `--write` regenerates.
- `tools/compare.py --verbose 0xADDR` — the per-function diff oracle.

## VC 2.2 codegen rulebook (append as discovered)

1. Relational conditions are canonicalized: source operand order (`a<b` vs
   `b>a`) and `!(a>=b)` spelling do not affect emitted code.
2. `if+do-while` loop guards emit the entry cmp with the *bound* as first
   operand (`cmp n,i / jle`-style); `for` loops emit natural order
   (`cmp i,n / jge`-style) at the back-edge and post-check.
3. A `i=0`-initialized counter compared against a register-resident bound is
   constant-folded (`test n,n`) at a for/if guard; a memory-resident bound is
   not folded (`cmp [mem], i`).
4. Register allocation is global over the (inlined) function; unrelated
   changes elsewhere in the function can flip allocation and cascade through
   every comparison. Match constructs first, then allocation follows.
5. Pointer equality against a DWORD argument: `(DWORD)p == dw` and
   `p == (T*)dw` compile to the same cmp but steer the argument-register
   allocation differently (casting the *member* to DWORD kept the natural
   `dwA→ecx, dwB→edx` order at hover.exe:0x404580; casting the argument to
   the member's pointer type swapped them). Probe both spellings.
6. A source-early `ret = 0` makes the return-value register a *known zero*:
   the next register null test is emitted as `cmp reg, eax` instead of
   `test reg, reg` (loop-entry guards and plain ifs alike; /O1 and /O2
   behave identically). The original never shows this peephole even where
   eax is provably zero (e.g. hover.exe:0x40458c `test edi,edi`), which
   points at a compiler-version peephole difference (VC 2.1/2.0 vs our 2.2)
   rather than a source-construct difference — no probed construct (while /
   for / if+do-while / early-return / late-init) reproduces the plain test
   together with eax reserved across the loop.
7. List-walk loops: a body-spelled advance test
   (`while (p && p->f == 0) { next = p->next; if (!next) break; p = next; }`)
   gets its back-edge null test constant-folded away (the compiler proves
   `p != 0` from the break check), emitting an unconditional `jmp` back-edge.
   The original's `mov p,next / test p,p / jne top` back-edge is reproduced by
   folding the advance into the condition:
   `while (p && p->f == 0 && p->next != 0) p = p->next;`
   (the compiler does not propagate the conjunct's non-null proof through the
   body assignment). do-while, for-increment, and goto-break spellings all
   normalize to the folded form.
8. The global allocator's register choice is sensitive to *first-assignment
   order* in the source, independent of where the load is finally scheduled:
   assigning a walk pointer before a group of stores gave it `edx`; assigning
   it after the same stores gave it `esi` (Hover_sub_4044A0). If the
   original's load sits late but its register looks "early-assigned", move the
   assignment statement up; the load may still schedule late.
9. A `p = 0` init whose value is only stored on a cold path may be deleted
   and rematerialized (`xor eax,eax`) at the cold block instead of kept live;
   the original sometimes keeps it live across the branch (dead `test` +
   re-`test` pattern). Not source-controllable in VC 2.2 — likely another
   2.1-vs-2.2 optimizer difference (see "Library drift").
10. Boolean results of signed comparisons on *DWORD-typed* fields cast to int
    (`b = ((int)a - (int)b) * (int)c + ... >= 0`) are emitted as branches
    (`mov r,1 / jns / xor r,r`); the same expression on `long`/`int` fields
    is emitted as `xor r,r / ... / setns rl`, which is what the original has
    in CSub406090::Sub_405B90 (0x405B90). Retyping CMerlinLine's coordinate
    and delta fields to `long` lifts that function from 29% to 50% but drops
    CSub410F20::Sub_410F20 (0x410F20) from 38% to 29%, so the retyping is
    left for a dedicated pass over every CMerlinLine user. Note the Makefile
    does not list gameobj.h/texheap.h as dependencies of every object that
    includes them: delete build/*.obj after header-type experiments.
11. VC 2.2 always folds an increment of a global WORD with an unused result
    (`g++`, `++g`, `g = g + 1`, `g += 1`, even temp-mediated
    `t = g; t++; g = t;`) into a single `inc word ptr [g]`. The original of
    CSub406090::Sub_405FA0 (0x405fa0) instead has `mov ax,[g]` hoisted above
    the guarding `je`, then `inc ax` / `mov [g],ax`. No source spelling,
    /O1, /O2 or /GB reproduces this with our compiler — suspected toolchain
    drift (cf. the MFC 3.1 / VC 2.1 fingerprint in the libdiff section).
12. To make the compiler emit a global load *between* a call and the
    `test eax,eax`/`je` that guards on its result (a hoisted-load pattern
    seen in the original, e.g. `call f` / `mov cx,[g]` / `test eax,eax` /
    `je`), assign the global to a local between the two statements:
    `n = f(); wOrd = g; if (n != 0 && h(wOrd)) ...`. Without the local the
    load is emitted after the branch.
13. Loop-carried offsets: whenever VC 2.2 is forced to keep a loop-carried
    integer offset in memory instead of a register, it *also* reassociates the
    index expression and hoists the now loop-invariant part out of the inner
    loop (`row + left` computed once per row into a fresh stack temp); and
    whenever the offset *is* in a register it folds `row + x` into a `lea`
    instead of evaluating `left + x + row` left to right. Measured on
    Hover_sub_411770 (0x411770) with ~20 spellings (statement splitting,
    parenthesization, operand order, declaration order, `register` hints,
    guard shapes): the pairing is absolute — register row base => LEA and no
    hoist; memory row base => reassociation + hoist. The original has the
    third combination (memory row base, strictly left-to-right evaluation, no
    hoist), so its shape is unreachable with our compiler and costs that
    function one stack slot, i.e. a 4-byte shift of every local reference.
    Suspected 2.1-vs-2.2 optimizer drift (cf. rules 6, 9 and 11).
    Corollary measured on the same function: `for (i = n; i > 0; i--)` behind
    an `if (n <= 0) return;` early-out still emits its own `test/jle` entry
    test (rule 3: the bound lives in memory, so the guard's proof is not
    propagated), and that spurious test happens to give the trip counter a
    stack slot of its own instead of coalescing it with `n` — which restores
    the original's frame size and therefore all fifteen argument offsets.
    Spelling the same loop as a guarded `do { } while (--i != 0);` matches the
    original's control flow instruction for instruction but leaves the frame
    4 bytes short: 40.40% versus 52.00% on reccmp.

14. Register allocation is stable per function but *not* per translation
    unit: adding an unrelated function to a .cpp re-shuffles the register
    choices of the other functions in the same file (measured by adding a
    four-line stub at the top of texheap.cpp - CSub402E00::Sub_402630 went
    29.0% -> 35.1%, Sub_4030F0 20.8% -> 25.2%, but CSub403980::Sub_403A60
    7.4% -> 2.8% and Hover_sub_406A80 39.5% -> 36.7%). The .obj is one
    optimizer unit, so expect this churn whenever a function is added to a
    file that already holds decompiled code, and re-measure the whole
    summary. Parking a new stub in stubs.cpp instead of next to its class
    avoids the churn when the collateral loss is not wanted.

15. Stack-slot order for *function-scope* locals follows the order of the
    first statement that touches them (first touched = highest offset in
    our build), not the declaration order; locals declared inside a nested
    block (e.g. an _asm macro's temporaries) are allocated above all
    function-scope scalars, in declaration order, block by block in source
    order, and arrays sit at the very top of the frame. Within a macro, the
    slot layout therefore follows the declaration order while the emitted
    store order follows the assignment order - splitting `long a = x;` into
    a declaration plus a separate assignment is the lever for matching both
    independently (used to line up the HOVER_MULDIV* argument temporaries in
    hover.exe:0x401260; it also lined up the four temporaries of the
    HOVER_DIST macro that `Hover_Distance` has since replaced, see rule 60).

16. `WORD` versus `short` for a stack local changes more than the signedness
    of the comparisons: at hover.exe:0x4045E0 the same clip test written
    with `(short)` casts over `WORD` locals produced `cmp reg, [mem]` with
    the operands the other way round from the original *and* the two locals
    in swapped slots, while declaring the locals `short` produced the
    original's `cmp [mem], reg` and slot order (95.0% -> 95.5%, and the
    whole clip block went from four diffs to none). When a stack local is
    only ever compared as signed, declare it `short`/`long` rather than
    casting a `WORD`/`DWORD` at every use (cf. rule 10).
17. `_asm` helpers are expanded inline. VC 2.2 *does* inline a function whose
    body is written entirely in `_asm` (labels inside are renamed per
    expansion, so the same helper can appear several times in one function -
    a macro cannot, since asm labels are function-scoped and would collide).
    Every expansion homes each parameter and the result in its own stack
    slot, because the `_asm` body references them by name; the slots run
    *downward* in parameter declaration order with the result temp last, the
    expansions are laid out top-down in source order, and the enclosing
    function's own scalars end up *below* all of them (arrays stay at the top
    of the frame). This is what produces the repeated five-slot temp blocks
    in the renderer's distance code (six expansions in hover.exe:0x4045E0,
    three in 0x406D30, one at 0x408A30 - see `Hover_Distance` in hover.h),
    including the 16x16-bit `mul ax` squares that no C spelling emits
    (`(unsigned short)a * (unsigned short)a` always yields `and`+`imul`).
    Corollary: a pointer copied from a pointer already proven non-NULL loses
    its own null test (`p = pPrev;` folds `test/je` away), but re-reading the
    *same memory expression* (`p = pObj->m_pUnk30;`) keeps the test while
    still CSE-ing the load - which is how hover.exe:0x406FF4's redundant
    `test edx,edx / je` is reproduced.
    Two things measured on 0x406D30 that are *not* source-controllable:
    (a) the original schedules the `cmp mem,0` of a following `if` *between*
    the load of a value and the `mov [obj+n],ax` that stores it; our compiler
    emits the store first (isolated probes do emit the original's order, so
    it is a scheduler difference, not a construct one), and (b) the
    zero-constant/walk-pointer register pair in a bubble loop comes out
    ecx/edx swapped no matter how the loop, its flag or its variables are
    spelled.
    Note also that adding a member declaration to a widely included class
    (CMerlinLine in gameobj.h) perturbs register allocation in *every*
    translation unit that includes it - 30-40 functions move by a few
    percent in both directions on any such change, so per-function scores
    elsewhere are only meaningful in aggregate (overall accuracy went
    68.74% -> 69.18% when 0x406D30 landed).

18. Measured on `CMerlinLine::Sub_401260` (0x401260), the segment/line
    intersection, whose arithmetic is a hand-written 64-bit muldiv:
    - **A 64/32 division is always inline asm.** `(__int64)a * b / c` (with
      or without a rounding term) compiles to a `__alldiv` call under VC
      2.2, never to `imul`/`idiv`, so every `idiv` on a 64-bit dividend in
      the original is an `_asm` block. When such a block must both take
      member expressions as inputs and feed its result into a C expression,
      write it as a macro that copies *all* arguments into block-scoped
      temporaries and ends with `_asm { ... }` followed by the C statement
      that consumes the result temp (see HOVER_MULDIVP/HOVER_MULDIVN in
      hover.h; a trailing C statement only parses after the braced form,
      because everything after a bare `_asm` to end of line is assembly).
    - **`this` loses to a plain local in the register allocator.** A method
      that reads a member twice (once for a computation, once for a result
      base) makes VC 2.2 spill the incoming `this` to a home slot *and* CSE
      that member into a second stack slot, growing the frame by 4 and
      shifting every local and argument reference. Copying `this` into a
      local pointer as the first statement and using it everywhere keeps the
      pointer in a register, kills the member CSE and restores the original
      frame (51% -> 82% here; the original clearly did the same, since it
      never touches a `this` home slot).
    - **Position inside the .cpp matters as much as rule 14's TU
      composition.** Moving this function one slot earlier in gameobj.cpp
      (before `CMerlinLine::Sub_4010C0` instead of after it) took it from
      73.9% to 82.5% *and* gave it the original's exact register assignment;
      the end of the file gave 83.0%. By contrast, every source-level
      rewrite of the arithmetic - operand order inside the products,
      statement order of the three cross products, splitting a product into
      its own statement, negating the whole expression - produced byte-identical
      code (VC 2.2 canonicalizes all of it). When the arithmetic is right
      but the registers are wrong, move the function inside the file rather
      than rewriting the expression, and re-measure the whole summary
      (function-order changes shuffle unrelated files too, apparently
      through the compiler's symbol tables: adding the declaration to
      gameobj.h alone flipped a register pair in hover.cpp).
    - **`else { return 0; }` is tail-merged, `if (x == 0) return 0;` is
      not.** Closing an if/else-if chain with a bare `else return 0;` makes
      VC 2.2 branch forward to an identical `return 0` epilogue emitted
      later (`je <far>`), while spelling the same test as the first
      statement *inside* the else block emits its own inline copy of the
      return and epilogue, which is what the original has (+5 points here).
    - **`t + p->m` is canonicalized.** With one stack temp and one
      `[reg+disp]` member operand, VC 2.2 always loads the *member* into
      eax and adds the temp, whichever way the source is written and
      whatever the signedness of the operands. The original loads the temp
      and adds the member, so this costs two instructions per use and is
      not source-controllable (suspected 2.1-vs-2.2 drift, cf. rules 6, 9,
      11 and 13).

19. **`double` members are copied with integer `mov` pairs, high dword
    first.** VC 2.2 never uses the FPU for a plain `double` member-to-member
    assignment at /O2: it emits `mov r,[src+4] / mov [dst+4],r` followed by
    `mov r,[src] / mov [dst],r`, interleaving the pairs of several
    assignments (probed with a six-field struct copy, and confirmed against
    hover.exe:0x425730, where the `theApp` fields at +0x1A0/+0x1A8/+0x1B0 are
    exchanged with a dialog's members). So a *pair* of dword moves whose
    offsets differ by 4, emitted high half first, is the fingerprint of a
    `double` field - not of two DWORDs, which are emitted in source order.

    Corollary for reading stack frames: in an EH-protected frame the three
    `push -1 / push handler / push eax` words put `esp` at `ebp-0xC`, so the
    `sub esp, N` covers `ebp-0xC-N .. ebp-0xD`, and that region holds *both*
    the C++ object(s) and the function's own scalars. `sizeof` of an object
    at `ebp-X` is therefore `X` minus the offset of the lowest local above
    it, not the whole frame - getting this wrong shifts every local by four
    bytes (measured on CHoverApp::OnOptionsCustomize and OnCmd34009, whose
    dialogs are 4 bytes smaller than the frame because a `POSITION` local
    sits at `ebp-0x10`).
20. Measured on the CMainFrame message map (hover.exe:0x4BA8D8) and its
    handlers:
    - **reccmp cannot score calls into the statically linked MFC/CRT.** The
      original's `call CWnd::Default` shows up as `call <OFFSET1>` and never
      matches our `call CWnd::Default`, so a handler whose whole body is
      `{ Default(); }` (hover.exe:0x40e2b0, 0x417af0) tops out at 50.00% and
      `{ m_dwUnkXX = 1; return Default(); }` (0x426b40) at 66.67% even when
      the instruction stream is otherwise identical. Do not chase these.
    - **The polarity of a ternary decides which arm falls through.**
      `::GetClientRect(pView == NULL ? m_hWnd : pView->m_hWnd, &rc)` emits
      `test/jne <pView arm>` with the `this` arm in the fallthrough, which is
      what the original has; spelling it `pView != NULL ? pView->m_hWnd :
      m_hWnd` emits the arms the other way round (+16 points on
      hover.exe:0x41dce0, +9 on 0x426a60). Rule 1 (source operand order is
      irrelevant) applies to relational *comparisons*, not to `?:` arms.
      Writing the same thing as a duplicated `if/else` with two calls is
      worse still (-1 point): VC 2.2 does not tail-merge the two calls, and
      the original's shape - the second argument pushed *before* the branch
      that selects the first - is not reachable from any spelling tried.
    - **A difference kept in a local is reused; an inline one is
      reassociated.** `cxWindow = rect.right - rect.left; if (cxWindow != cx
      ...)` keeps the width in a register and reuses it at the later
      `SetWindowPos` call, exactly like the original, while the inline
      `rect.bottom - rect.top != cy` is reassociated to `bottom - cy != top`
      (the original keeps the natural form). Same lever on 0x426c20 and
      0x41f480: assigning the two `SetWindowPos` dimensions to locals makes
      VC compute both into registers before the argument pushes (+5 points),
      instead of interleaving one of them with the pushes.
    - **`(a == 0) != (b == 0)`** compiles to the pair of `cmp r,1 / sbb r,r /
      neg r` zero-tests followed by `sub` and a `cmp r,1 / sbb r,r / inc r`,
      which is exactly hover.exe:0x415300's activation-changed flag; and
      swapping the *declaration* order of the two locals in that function
      (flag before pointer) swapped ebx/ebp to match the original (+18
      points). Cf. rule 8.
    - **Do not put a new free-function declaration in hover.h.** Adding
      `void Hover_sub_42D760(void*)` to hover.h (included by every .cpp)
      cost CMerlinLine::Sub_4010C0 its 100% match (down to 72.9%) and
      CGameObj::Sub_40D6E0 22 points; moving the same declaration into
      mainfrm.h, the only header that needs it, restored both. Class member
      declarations added to a widely included header are cheaper but not
      free (cf. rule 17's note on CMerlinLine).
21. **The original's CBumperView translation unit was compiled with
    automatic inlining** (`/Ob2`, i.e. `/Ox`-style), unlike our `/O2`
    (`/Ob1`) build. Measured on CBumperView's message map (hover.exe:
    0x4BB0C8): `CBumperView::Sub_40EE90(1)` (0x40EE90, the "suspend the game
    loop" transition) is expanded inline into five handlers - OnSize
    (0x4183E0), OnGamePause (0x413C70), OnEscape (0x428890), OnCmd34032
    (0x429230) and OnContextMenuMsg (0x428D90) - while still existing as an
    out-of-line function that CMainFrame::OnClose (0x41D6B0, a different
    module) calls normally. The same holds for `Hover_sub_401140`
    (0x401140), which OnCmd34030 (0x428A90) expands inline but OnClose
    calls; that also tells us the original's file split differs from ours
    (0x401140 must have lived in the CBumperView module).
    Since we cannot turn on `/Ob2` for one file without re-shuffling the
    whole build, reproduce such an expansion with a `do { ... } while (0)`
    macro whose early exits are `break`s (the inlined early returns only
    skip the expanded block, not the rest of the handler); see
    `HOVER_INLINE_SUB_40EE90_1` in bumpview.cpp. That macro is worth ~50-60
    points on each of those five functions (OnGamePause and OnEscape reach
    100%/97.9% with it).

    Two related observations from the same message map:
    - **Long `if / else if` chains lay their bodies out in reverse.** For
      CBumperView::OnKeyDown (0x40B540) the original emits test1, body1,
      test2, body2, test3, body3; VC 2.2 emits test1, test2, test3, body3,
      body2, body1 (identical instructions, different block order) for the
      same source. No spelling tried (operand order, extra `else`, nesting)
      changed it, and the instruction streams are otherwise identical, so
      this is block-placement drift, not a source difference.
    - **Passing a `CPoint` to an inline member that takes `POINT` by value
      creates a per-call 8-byte stack temp** (`mov [tmp],eax / mov eax,[tmp]
      / push eax`). The original's WM_CONTEXTMENU handler has seven of them,
      i.e. its hit tests were written `rc.PtInRect(point)` on `CRect`/
      `CPoint`, not `::PtInRect(&rc, point)` on `RECT`/`POINT`.

    Finally, note that annotating a global that lies *inside* another
    annotated global's span changes how reccmp resolves every operand above
    it: adding `// GLOBAL: HOVER 0x460970` to `g_pBumperDoc` (which aliases
    `theApp + 0x400`) gained ~0.05 points overall and lifted eight
    CBumperView handlers to 100%, but the same trick on `g_avkControls`
    (`theApp + 0x170`) cost 0.4 points overall, because every `theApp+N`
    reference above 0x170 then resolved against `g_avkControls` instead.
    Only annotate an aliasing global when it sits near the end of the
    enclosing object.

22. **A store between the `test` and the `jcc` needs a dead store in the
    taken arm.** `p->b = p->a; if ((long)p->a > 0) p->b = 0;` always emits
    `mov eax,[a] / mov [b],eax / test eax,eax / jle`, i.e. the store
    *before* the test. The original of `CBumperView::Sub_41E5B0`
    (0x41e5b0) has `mov eax,[a] / test eax,eax / mov [b],eax / jle` nine
    times over. The only spelling that reproduces it is an if/else whose
    two arms both start with the same store, so VC hoists it above the
    branch it just tested:
    `n = p->a; if (n > 0) { p->b = n; p->b = 0; } else { p->b = n; }`
    (see `HOVER_RESET_GAUGE` in bumpview.cpp; 67% -> 93%). Caveat: when the
    *last* such if/else is followed by fewer than ~4 instructions, VC tail-
    duplicates the join block instead of merging it - harmless in a long
    function, fatal in a short one. Note also that `> 0` on a `DWORD` field
    folds to `jne`; the signed `jle` needs a `long` temporary (cf. rule 16).

23. **Identifying the CRT string/copy helper behind a call.** VC 2.2 at /O2
    expands `memcpy` inline even with a *variable* length
    (`mov ecx,n / shr ecx,2 / rep movsd / mov ecx,n / and ecx,3 /
    rep movsb`), and `strcpy`/`strcat` inline as `repne scasb` (strlen) plus
    the same `rep movsd/movsb` pair - a `strcat` being recognisable by the
    *second* `repne scasb` over the destination followed by `dec edi`. So a
    real `call` with (dst, src, len) is *not* memcpy: hover.exe:0x43ea30 is
    `strncpy` (copies, stops at NUL, zero-pads, returns dst), which has no
    intrinsic and is always called. Getting this wrong costs ~10
    instructions of inline expansion (CBumperView::Sub_408DF0, 0x408df0:
    57% -> 77%). Two more library shapes from the same function:
    hover.exe:0x43ea54 is `_fcvt` (double, int, int*, int* - the hand-rolled
    "%.1f" formatter the original uses instead of sprintf) and 0x448a0c is
    `_itoa`.

24. **The Pentium-FDIV workaround is unconditional and not source-visible.**
    Every `double` division compiles to `pushfd / cmp __adjust_fdiv,0 /
    jne / fdivp st(1) / jmp / push eax / mov al,0xD / call __adj_fdiv_r /
    pop eax / popfd`. Likewise `(double)someDWORD` is `mov [tmp],eax /
    mov [tmp+4],0 / fild qword ptr [tmp]` (unsigned widened to `__int64`
    through a stack temp), and a `double` local that feeds exactly one
    later use is kept on the x87 stack with no frame slot at all, while a
    second one is spilled. None of this is worth chasing.

25. **`if (x == 0) return f();` emits the tail call inline; the inverted
    spelling puts it at the end.** VC 2.2 turns the early-out into
    `jne <skip> / jmp f` right where it is written, but
    `if (x != 0) { ...; return 1; } return f();` emits `je <end>` over the
    whole body with the `jmp f` as the last instruction of the function -
    which is what hover.exe:0x413710 (`CSub425050::Sub_413710`) has
    (82% -> 100%). Complements rule 18's tail-merge observation.

    Also from the same batch: assigning `&m_dwUnkNNN` to a local `DWORD*`
    and storing through `p[0..3]` is what produces the original's
    `add ecx,0xNNN` + four `[ecx+0..0xc]` stores instead of four
    `[this+0xNNN]` stores (CBumperView::Sub_427E90, 0x427e90: 12% -> 100%);
    the same idiom already appears in Sub_4274B0.

26. **VC 2.2 always reassociates `p->a - p->b - t` into `(p->a - p->b) - t`
    and CSEs the difference.** Measured on `CMainFrame::Sub_41D850`
    (0x41d850), whose original computes `abs(rc->right - nZoneCX -
    rc->left)` strictly left to right while recomputing `rc->right -
    rc->left` inline at two later comparisons. Every non-volatile spelling
    of the abs expression - statement splitting, `-=` chains,
    reparenthesizing as `right - (n + left)`, swapping the operands -
    compiles to the same reassociated form, and because `right - left` then
    occurs three times VC spills it (and the matching `bottom - top`) to two
    extra stack slots, growing the frame by 8 and shifting every
    `esp`-relative displacement in the function. Reading the fields through
    a `(volatile RECT*)` cast at the *other* use sites is the only lever
    that restores the original's recomputation: 68.30% -> 87.53%.

27. **Where a local C++ object is declared decides whether the return value
    is evaluated before or after its destructor.** An object declared
    mid-function but at *function* scope makes VC 2.2 evaluate `return
    m_x;` into a stack temp, run the destructor, then reload the temp
    (`mov [ebp-0x14],eax / call <unwind funclet> / mov eax,[ebp-0x14]`),
    which is what hover.exe:0x4253d0 has. Wrapping the same object in an
    inner `{ }` block runs the destructor first and reads the member
    afterwards, and costs the frame one slot - so the position of the
    return-value temp in the epilogue is the fingerprint that tells the two
    apart. (Declaring the object mid-function is also what puts its
    constructor call in the middle of the instruction stream instead of the
    prologue.)

28. **Refinement of rule 20's "duplicated if/else is worse".** That holds
    when the two calls differ in several arguments. When only the *first*
    argument differs - `pWnd->SetWindowPos(cond ? &CWnd::wndTopMost :
    &CWnd::wndNoTopMost, 0, 0, cx, cy, 0)` - the ternary emits the inlined
    `GetSafeHwnd()` once after the select, while the original duplicates it
    per arm. Spelling it as an if/else with two full `SetWindowPos` calls
    reproduces the original exactly: VC 2.2 cross-jumps the identical
    argument tail (last push + call) and duplicates only the pointer
    selection (+5 points on 0x4253d0).

29. **Reading zero-test idioms in the original.** `cmp r,1 / sbb r,r /
    neg r` is `(r == 0)`; `cmp r,1 / sbb r,r / inc r` is `(r != 0)`; and
    `mov r,0 / cmp [g],1 / adc r,-1 / and r,X` is `g ? X : 0`. All three are
    what VC 2.2 emits for the plain C spelling, so they need no special
    source form (used in 0x4253d0, 0x425600 and 0x41d850). Not
    source-controllable in the other direction: whether the compiler
    materializes the zero of a `cond ? V : 0` *before* the branch (ours) or
    rematerializes it in the cold block (the original) - cf. rule 9.

30. **A member load can be pulled above a group of global stores by
    assigning it to a local first** (complements rule 12): the original of
    `CHoverApp::Sub_41C8E0` loads `pDlg->m_hWnd` before the three global
    stores that precede its `UpdateWindow` call, which
    `HWND h = pDlg->m_hWnd; g1 = ...; g2 = ...; ::UpdateWindow(h);`
    reproduces while `g1 = ...; ...; pDlg->UpdateWindow();` emits the load
    after them.

31. **reccmp scores an unresolved symbol operand as a diff.** A
    `// GLOBAL: HOVER 0x...` marker only helps when the variable reaches
    the linker map - a file-scope `static` (e.g. `g_dwUnk_4C483C` in
    mainfrm.cpp) does not, and the marker just adds a "0 map candidates"
    warning. Calls into the statically linked MFC/CRT can never be scored
    (rule 20), which caps a pure constructor like `CSub424720::CSub424720`
    (0x424720) at 81.40% even though its instruction stream is identical.

32. **Statement order across a call decides whether a memory operand is
    reloaded.** In `CMainFrame::PreCreateWindow` (0x41ddf0) the original
    stores `cs.x`/`cs.y` into the local rectangle *before* `GetSystemInfo`
    and then reloads `cs.x`/`cs.y` from the CREATESTRUCT again in both arms
    of the branch that follows. Writing the two assignments after the call
    keeps the values live in registers and emits a `jmp` around one arm;
    moving them in front of the call reproduces the original's reloads and
    its branch shape exactly (+9 points, 85.91% -> 97.96%). The general
    form: a value read before a call and used after it is reloaded, one
    read only after the call is kept in a register.

33. **An empty virtual override is not noise.** `ret n` bodies in the
    original (e.g. `CBumperDoc::Serialize` at 0x426f10, three bytes) are
    real overrides that keep an MFC base implementation out of the vtable
    slot; declaring the member and giving it an empty body is a 100% match.

34. **Destructors folded into `??_G`.** MSVC 2.2 inlines a short
    user-written destructor into the scalar deleting destructor it
    generates, so classes like `CHoverApp`, `CMainFrame` and `CBumperDoc`
    have no separate `??1` in the original - the vtable slot 1 address *is*
    the whole destructor. A `// FUNCTION:` marker on our `~C...()` therefore
    pairs our `??1` against the original's `??_G` and can never exceed
    ~70-90%: the `test byte ptr [ebp+8],1 / delete this / return this` tail
    is scored as a diff even when every instruction of the body matches.
    Read those percentages as "body matches, wrapper unscoreable".
35. **A member value stored and then used twice must live in an explicit
    local.** `m_a = m_b; if (p) { p->x = (WORD)m_b; p->y = (WORD)m_b + K; }`
    makes VC 2.2 reload `m_b` before the second use (it CSEs only the
    first); `DWORD t = m_b; m_a = t; ... p->x = (WORD)t; p->y = (WORD)t + K;`
    keeps it in `eax` throughout. Took `CPlayer::Sub_4087F0` (0x408990)
    from 81% to 100%.

36. **`void D::f() { B::f(); }` is emitted as a bare `jmp B::f`.** VC 2.2
    tail-jumps a void member function whose whole body is one call with
    the same `this` and no arguments, so the original's many one-
    instruction vtable slots (0x4089D0, 0x4089F0, 0x42C050, 0x42C240,
    0x42C3B0, 0x40D2E0, 0x41B2C0, 0x422620, ...) are simply inline
    forwarders in the source, not thunks. All of them match at 100%.

37. **State transitions the original inlines need the target computed
    into a local first.** The robot AI writes its state switch as
    `pNext = &parent->m_objNNN; parent->m_pState->Leave(); parent->m_pState
    = pNext; pNext->Enter();` - the `lea` for `pNext` is emitted *before*
    the `Leave` call. Spelling it as three statements that each re-derive
    `&parent->m_objNNN` re-loads the parent after the call and costs 5-8
    instructions per site (+57 points on 0x40BC90, +33 on 0x42ED90, +31 on
    0x4150F0).

38. **Two adjacent `if (c) return K;` guards share one epilogue only when
    written as `if (c1 || c2) return K;`.** Written separately, VC 2.2
    duplicates the whole `xor eax,eax` + register pops + `ret n` tail per
    guard, which on a small function with four callee-saved registers is a
    dozen instructions of diff (+12 points on 0x42ED00, +26 on 0x40CB40).

39. **A struct-returning helper's *return value* and its output buffer are
    not interchangeable.** `CGameObj::Sub_403880(adwOut)` returns `adwOut`;
    the original sometimes reads the result through the returned pointer
    (`mov ecx,[eax] / mov edx,[eax+4]`) and sometimes out of the stack
    slots (`mov ecx,[esp+0x10]`). Matching that choice is worth several
    instructions - use `pdw = obj->Sub_403880(adw); f(pdw[0], pdw[1]);` for
    the first form and `obj->Sub_403880(adw); f(adw[0], adw[1]);` for the
    second (0x40A790 vs 0x40BB40).

40. **Nested collider-cell loops stay `for` loops.** Rewriting
    `CPodObj::Sub_408820` (0x413180) into rule 3's if + do-while form -
    which the reversed `cmp bound, counter` orientation of all four of its
    loop tests suggests - *halves* the score (65% -> 35%): the do-while
    body gets a different register assignment for the two counters. The
    reversed orientation there comes from somewhere else; the plain
    `for (i = lo; i <= hi; i++)` pair is still the best match.
41. **A vtable slot that is a bare `jmp` to the base implementation is a
    forwarding override**, i.e. `void CDerived::F() { CBase::F(); }`. VC 2.2
    tail-calls it, so the whole function is one 5-byte `jmp` (verified with
    tools/probe: a void no-argument forwarder becomes `jmp`, one with
    arguments keeps the pushes and a `call`). The maze-object hierarchy is
    full of them - CPodObj, every pod subclass, CFloorObj and its pads and
    CWall together contribute 34 such slots, and all 34 come out at 100.00%
    from that spelling. Do not mistake them for linker thunks: a class that
    really inherits the base's implementation gets the base function's
    address in the slot directly (e.g. CPodObj slot 9), so a `jmp` in the
    slot always means the source declared and defined an override.

42. **Corollary of rule 10 for the `xor/setns` boolean.** When the members
    feeding a signed comparison are `DWORD`, casting them to `long` at the
    use site is *not* enough - VC 2.2 still emits `mov r,1 / jns / xor r,r`.
    Copying each member into a `long` local first (and typing pointer
    parameters `const long*` rather than casting `DWORD*` reads) is what
    produces the original's `xor eax,eax / ... / setns al`. Measured on
    `CSub40D320::Sub_40D2F0` (0x40d2f0): 16.00% -> 100.00% with no other
    change. Cheap enough to try on any comparison-returning helper whose
    fields are DWORD in the reconstruction.

43. **`if/else` arm order follows the source, like rule 20's ternary
    polarity.** For the gauge clamp of `CSub422C80::Sub_42A9F0` (0x42a9f0)
    the original emits `jl <zero path>` with the saturating store in the
    fallthrough; `if (n < lim) { ...zero path... } else { m = lim; }` emits
    the two blocks the other way round, and spelling the same test as
    `if (n >= lim) { m = lim; } else { ...zero path... }` reproduces the
    original's placement (+4 points). Rule 1 (operand order is
    canonicalized) applies to the comparison, never to which arm is laid
    out first.

44. **An overloaded constructor cannot be scored.** `tools/compare.py`
    resolves markers through the linker map by decorated name prefix, so
    two constructors of the same class (`CSub42AE90(WORD,WORD)` at
    0x42AE90 and `CSub42AE90(double)` at 0x42A9D0) both match
    `??0CSub42AE90@@` and are reported as "2 map candidates" and left
    unresolved. Overloaded *methods* are fine; only same-name constructors
    collide. Either accept the two unscored functions or give one of them
    a distinct name.
45. **Every translation unit compiled earlier perturbs the codegen of the
    ones after it.** All objects share one `/Fd build\` PDB, so the type
    indices a .cpp gets depend on what was compiled before it, and VC 2.2's
    register allocation shifts with them - in files whose own sources and
    headers did not change at all. Adding optionsdlg.cpp (and editing
    hover.cpp/stubs.cpp/bumperdoc.cpp, which are compiled earlier) moved 91
    of the 499 previously scored functions, e.g. `CSub403850::Sub_403850`
    (texheap.cpp, 0x403850) 100% -> 37% and `CSub410F20::Sub_410F20`
    28% -> 77%. Reordering the new file inside the Makefile's `OBJS` list -
    which is the order nmake compiles in - shifts the whole-binary accuracy
    by ~0.1 points on its own, so try that before rewriting anything when a
    new file makes unrelated functions regress. The .obj bytes are not
    reproducible either (debug-info timestamps): compare the linked code,
    never md5sums, when chasing this.

46. **`HIWORD()`/`LOWORD()` on a value the original reads out of memory.**
    hover.exe's parameter dialogs read a 16.16 fixed-point dword as
    `movsx r, word ptr [tmp+2]` for the integer part and `and r, 0xffff`
    on the register copy for the fraction. `(short)HIWORD(x)` on a plain
    local compiles to `shr`/`movsx r,ax` instead; a union with a `short`
    high member (`FIXED1616` in optionsdlg.cpp), assigned from the global
    and then read as `fx.s.sInt` / `(fx.dw & 0xFFFF)`, reproduces the
    original exactly (+40 points on 0x430b80 and 0x431d10).

47. **`= 0.0` on a double member is not a memset.** A constructor that
    zeroes an array of doubles one element at a time compiles to a pair of
    `mov dword ptr [r+n], 0` per element in VC 2.2 (with the halves of
    neighbouring elements scheduled out of order), which is what hover.exe
    does; `memset(m_a, 0, sizeof(m_a))` becomes `rep stosd` and scores
    ~36%. Write the assignments out (+59 points on 0x430780/0x4319a0).

48. **`CArchive` buffer accesses: cache the cursor, store the destination
    first.** Every Merlin `Serialize` in hover.exe reads and writes the
    archive through MFC's inline `CArchive::operator<</>>` bodies, but the
    original keeps `m_lpBufCur` live in one register across a whole run of
    reads or writes, while the literal MFC spelling (`ar << w` / `ar >> w`)
    makes VC 2.2 reload it after every access - it cannot prove that the
    store through the cursor, or the caller's store into the member, does
    not alias the `m_lpBufCur` member itself. Two source levers reproduce
    the original (both in `CMerlinArchive`, texture.h):
    - **Copy the cursor into a local** before using it
      (`p = m_lpBufCur; *(WORD*)p = w; m_lpBufCur = p + 2;`), which kills
      the reload *inside* the helper. Worth ~15 points per Serialize.
    - **Take the destination by pointer** so the member store happens
      *before* the cursor store (`void ReadWord(WORD* pw)` rather than
      `WORD ReadWord()`): with the cursor stored last, VC knows the
      register still equals `m_lpBufCur` and the next access's bounds check
      reuses it. Switching the four-word reads of `CMerlinLine::Serialize`
      to the pointer form took it from 41.8% to 72.4%, `CMerlinStatic::
      Serialize` from 50.1% to 63.5% and `CMerlinLocation::Serialize` from
      42.9% to 68.3%. The value-returning form is still the right one where
      the result is widened before it is stored
      (`pEntry->m_dwUnk08 = (short)pAr->ReadWord();` in
      `CMerlinTexture::Serialize`), which is what the original has there.
    - For single **bytes** the original advances the cursor first and reads
      behind it (`mov ecx,[esi+0x1c] / inc ecx / mov [esi+0x1c],ecx /
      mov al,[ecx-1]`), i.e. `m_lpBufCur++; return m_lpBufCur[-1];`.

    Still unexplained: all six original `Serialize` overrides reserve a
    4-byte frame slot (`sub esp, 4`, `sub esp, 0xc` for CMerlinBSP) that
    holds the trailing-comment `BYTE` local and/or the `short` count, and
    our build materializes neither (the inline `ar >> by` store is
    dead-code-eliminated together with its slot, and the count stays in a
    register). The missing `sub esp, N` / `add esp, N` pair is two
    instructions, but it shifts reccmp's whole alignment, so these
    functions score 20-30 points below the similarity their instruction
    streams actually have. `volatile BYTE by` allocates the slot but adds a
    store the original does not have.

65. **Re-read a global inside the call arm to reproduce the original's
    reload.** Two shapes from the MIDI stream code, both worth a lot:
    - A global *object* pointer used before and after a call: writing
      `p = g_p; ... if (c) { f(p->x); ... } ... p->y = 1;` keeps `p` live
      across the call, so VC 2.2 gives it a callee-saved register and the
      function pays an extra `push`/`pop` pair. Adding `p = g_p;` as the
      last statement *inside* the call arm makes `p` dead across the call,
      which lets the allocator keep it in `ecx` and reload it exactly where
      the original does (`Hover_sub_438440`, the MOM_DONE callback:
      32.00% -> 100.00%). Reading `g_p->m` at every use instead is worse
      (32.00% -> 32.00%, one reload per use, because the intervening stores
      may alias).
    - A global *function pointer* called in a loop: `pfn = g_pfn;` at the
      top of the loop body leaves the load *inside* the loop and makes the
      back edge jump to it; assigning it once before the loop and again
      inside the call arm (`if (pfn) { r = pfn(...); pfn = g_pfn; }`)
      produces the original's rotated shape - load hoisted into the
      preheader, reload after the call, back edge past the load
      (`CSub425050::Sub_438300`: 92.17% -> 97.25%).
    Corollary: `if (pfn != NULL) r = pfn(a); else r = ERR;` on a *local*
    copy emits `mov reg,[g] / test reg,reg / ... / call reg`, while using
    the global directly emits `cmp dword ptr [g],0 / ... / call dword ptr
    [g]`. The original always has the register form, i.e. it copied the
    resolved entry point into a local first (65.40% -> 85.71% on 0x438300).

66. **A `&&` chain that produces a *value* must be split to get the
    branchless form.** `b = (p != NULL && p->wMagic == MAGIC);` compiles to
    `cmp word ptr [p+n], MAGIC / mov eax,1 / je / xor eax,eax`, but the
    original of `Hover_sub_43A860` has the 16-bit zero-test idiom
    `mov ax,[p+n] / sub ax,MAGIC / cmp ax,1 / sbb eax,eax / neg eax`
    (rule 29's `(r == 0)` shape). Probed: only the two-statement spelling
    `b = 0; if (p != NULL) b = (p->wMagic == MAGIC);` emits it - the
    short-circuit expression and the `(WORD)(x - MAGIC) == 0` rewrite both
    give the cmp-against-memory form. Same holds for a `WORD` temporary.

67. **WNDCLASS-style struct fills follow *statement* order, and a global
    read after a call is reloaded.** `Hover_sub_43DC40` fills its
    `WNDCLASS` in an order that is not the declaration order (hCursor,
    hIcon, lpszMenuName, lpszClassName, hbrBackground, style, hInstance,
    cbWndExtra, cbClsExtra, lpfnWndProc); writing the assignments in that
    order reproduces the original's interleaving of the stores with the
    `LoadCursor`/`GetStockObject` argument pushes exactly. The same
    reordering also fixes `wc.hInstance = g_hInst;`: written *before* the
    two API calls VC forwards the value it just stored into the global
    (`mov [wc+0x10], edi`), written *after* them it has to reload
    (`mov eax,[g_hInst]`), which is what the original does - the general
    form of rule 32 for globals. 57.67% -> 96.88% together with the
    tooling fix below.

68. **Tooling: `tools/origsize.py` now also ends a function at a vtable
    forwarding thunk.** MSVC packs compiler-generated `mov eax,[ecx] /
    jmp dword ptr [eax+N]` thunks (and EH-protected library constructors)
    immediately behind the last real function of a module with no `int3`
    padding, so the linear end-detection ran past `Hover_sub_43DC40`
    (0x43dc40) into 136 bytes of MFC `CDC` thunks and reccmp scored the
    function against them. `size()` now treats that two-instruction thunk
    pattern, when it follows a `ret`/`jmp`, as the start of the next
    function (223 bytes instead of 359 for 0x43dc40). Worth remembering
    when any function's reported original size looks suspiciously larger
    than ours: check `tools/disasm.py` for what really follows its `ret`.
69. **Mark the real `??1` when the original has one; `??_G` only when it
    does not.** Rule 34 says a destructor marker pairs our `??1` against
    the original's scalar deleting destructor, but half the maze-object
    classes do have a separate out-of-line `??1` as well - it is simply
    never referenced by a `call`, so a call-target scan misses it. Find it
    as the *`jmp` target of the class's EH unwind funclets*: every
    `mov ecx,[ebp-N] / jmp X` thunk that a constructor or a deleting
    destructor calls names the destructor of the object being unwound.
    That is how `CWall::~CWall` turned out to be hover.exe:0x40E650 (90.5%)
    rather than the vtable slot 0x417BA0, and `CMerlinObject::~CMerlinObject`
    hover.exe:0x4021E0 (82.1%) rather than 0x42D620. A class gets its own
    out-of-line `??1` exactly when something builds it on the *stack* (the
    collider's temporary `CWall`s, the loader's Merlin objects); the pool
    classes, which only ever exist behind `new`, have none and must carry
    the `??_G` marker instead. Measured with the `??_G` marker on an
    otherwise empty pod destructor: 65.31% (the EH prologue, the vtable
    store and the base call match; the `test byte ptr [ebp+8],1` /
    `operator delete` / `return this` tail is scored as a diff), 69-74%
    when the destructor has a body of its own.

    Corollary for the `??_G` of a class that *does* have a `??1`
    (`CPodObj` 0x42B000, `CFloorObj` 0x4219D0, `CGameObj` 0x42ADB0,
    `CCollider`/`CRegionMatrix` 0x41B6C0/0x42C7A0, `CMerlinObject`
    0x42D620): our build emits that function too, but no marker can name
    it - `tools/vc2map.py` derives the mangled name from the declaration
    under the marker, and `??_G...` is not a spelling any declaration has.
    Those functions stay unannotated until the marker syntax grows a way
    to name a compiler-generated symbol.

70. **`tools/origsize.py` reports EH funclets as functions.** Its linear
    scan ends a function at the first `int3` after a `ret`/`jmp`, so an
    unwind funclet (`mov ecx,[ebp-N] / jmp <dtor>`, 8-24 bytes, referenced
    only from the EH tables in .rdata) is sized as if it ran to the end of
    the *next* real function. Every "function" in the mazeobj range whose
    first instruction is `mov`/`lea ecx, [ebp-N]` followed by a `jmp`, or
    that starts with `cmp dword ptr [ebp-N], 0`, is such a false start:
    0x421173, 0x42254D, 0x4227A7, 0x412342, 0x4123EB, 0x4124F2, 0x40F2AC,
    0x40F3C9, 0x4116E5, 0x417952 and 0x4176A8 all sit *inside* functions
    that are already decompiled (0x421070, 0x422470, 0x4226D0, 0x412250,
    0x40EF30, 0x411600, 0x417860, 0x415450). 0x4176A8 is worth a second
    look for a different reason: it is a *catch* funclet (it ends with
    `mov eax, <resume address> / ret`), i.e. the level loader at 0x415450
    really does have a `try`/`catch` around the texture-loading block.

71. **The fixed-point helpers come in a by-pointer flavour.** The two
    CPlayer wall-bounce helpers (hover.exe:0x41AB10 and 0x421B50) do not
    use the `HOVER_FIXMUL`/`HOVER_FIXMULW` shape of the rest of the game;
    every product there is written
    `mov [tmp],val / lea r,[tmp] / mov [ptmp],r / mov ecx,[ptmp] /
    mov eax,[ecx] / imul [btmp] / shrd / mov [ecx],eax / mov eax,[tmp]`,
    i.e. the accumulator is reached through a pointer temporary. An
    `_asm` helper taking `DWORD*` (`Hover_FixMulP` / `Hover_FixMulWP` in
    hover.h) reproduces that instruction for instruction and is worth
    ~18 points on 0x421B50 (41.9% -> 59.8%). Two caveats:
    - the trailing C statement of a macro only parses after a *braced*
      `_asm { }` block (cf. rule 18), so the `_asm` helper function is the
      cleaner form here;
    - the original's frame is still 8 bytes larger than ours: each
      expansion leaves one dword slot completely untouched (hover.exe:
      0x41AB10 never writes `[esp-0x14]` or `[esp-0x20]`). Neither an
      unreferenced local in the helper nor a third macro temporary
      allocates it under VC 2.1, and the 8-byte shift is what keeps
      0x41AB10 at 38.5% although its instruction stream matches.

72. **`ar >> by` and `pAr->ReadByte(&by)` are different functions in the
    original.** The trailing-comment skip loop of `CMerlinObject::
    Serialize` (0x411B10) expands MFC's inline `CArchive::operator>>(BYTE&)`,
    while `CMerlinDynamic::Serialize` (0x42CC40) and `CMerlinBSP::
    Serialize` (0x41BC40) `call` an out-of-line copy at hover.exe:0x42CF30
    that advances the cursor before the read and returns `this` - i.e. the
    original declared a `CMerlinArchive` overload of `operator>>` and
    defined it in the maze-object module. Ours is spelled
    `CMerlinArchive& CMerlinArchive::ReadByte(BYTE*)` because vc2map's
    marker parser cannot name an operator (80.95%, the residual being the
    unscoreable `CArchive::FillBuffer` call and one register swap).
    Only those two loops call it: the single-byte reads in the same two
    functions (`*(BYTE*)&m_dwUnk60 = pAr->ReadByte()` and friends) are
    inlined by the original (`inc edx / mov [esi+0x1c],edx / mov al,
    [edx-1]`), so the inline `BYTE ReadByte()` overload is right there and
    both spellings have to coexist. Spelling the CMerlinBSP loop `ar >> by`
    instead cost the `call`, the `lea` of the byte's slot and 4 bytes of
    frame (`add esp,8` for `add esp,0xc`).
49. **`#pragma optimize("", off)` is a real source construct in hover.exe.**
    `CRobotPlayer::Sub_409350` (0x409350, the robot steering pass and the
    biggest function in the game at 4405 bytes) is not optimized code: it
    has an `ebp` frame, every temporary homed on the stack, a five-byte
    `jmp $+5` at the end of most statements, and unreachable
    "materialize the boolean, then test it" tails after every inlined
    comparison - while *still* expanding inline functions and still
    calling the fixed-point helpers (0x4038E0/0x40A490/0x40B4F0) that the
    optimizer inlines everywhere else. That combination is exactly
    `/Od /Ob1`, and inside our `/O2` build it is reproduced by wrapping
    the function in `#pragma optimize("", off)` / `#pragma optimize("",
    on)` (probed: `/O2` + the pragma is byte-identical to `/Od /Ob1` on
    the same source, and the surrounding functions are unaffected).
    Corollaries measured on the same function:
    - Each inlined call gets its arguments homed in fresh stack slots, a
      result slot of its own, and a `jmp` to the join point. To get the
      original's slot *count* an inline wrapper must therefore copy its
      result into a local (`dwR = dwB; return dwR;`) and an inline
      in-place operator must copy its argument into one
      (`DWORD dwT = dwA; m_dwValue += dwT;`) - without those the compiler
      reuses the parameter slot and the frame comes out short, which
      shifts every `ebp` displacement in the function.
    - A comparison written as one expression short-circuits into a single
      `cmp`; the original's per-operand structure (one `cmp`, one dead
      `cmp temp,0` tail and two `jmp`s per leaf) needs each leaf to be its
      own inline function, with the composite `a < b` spelled
      `!IsGt(a, b) && !IsEq(a, b)`.
    - An in-place `-=` in the original emits `xor eax,eax / sub eax,arg /
      neg eax / sub [dst],eax`, i.e. its source really was a double
      negation (`m -= -(-x)`); a plain `m -= x` is two instructions
      shorter.

50. **The fixed-point macros have pointer-destination variants.** Beside
    the `mov eax, dwA / imul dwB / shrd / mov dwA, eax` form the
    HOVER_FIXMUL* macros already reproduced, hover.exe's physics code
    (0x41A6B0, 0x41AB10, 0x421B50) reaches the destination through a
    stack temporary holding its *address*: `mov ecx, _pA / mov eax, [ecx]
    / imul _dwB / shrd eax, edx, 0x10 / mov [ecx], eax`, with `_pA`
    allocated above `_dwB` and stored after it. `HOVER_FIXMULP` and
    `HOVER_FIXMULWP` in hover.h are that form; using the plain macros at
    those sites costs three instructions per expansion.

51. **An `_asm`-bodied `__thiscall` helper needs `this` copied into a
    local.** The out-of-line fixed-point helpers (0x4038E0, 0x40A490,
    0x40B4F0) are one-slot frames that home `this` (`sub esp,4 / mov
    [esp],ecx`), read the argument straight out of the caller's slot
    (`imul dword ptr [esp+8]`) and return `this` in eax. `_asm` cannot
    name `this`, and writing the body in C with the HOVER_FIXMUL* macros
    allocates a second temporary; `CFixed* p = this; _asm { ... mov
    ecx, p ... } return p;` reproduces both helpers exactly (100.00% on
    0x40A490 and 0x40B4F0).
55. **VC 2.2's inline assembler cannot see through a C++ reference, and
    hover.exe's fixed-point macros take their destination by pointer.**
    `_asm mov eax, r` where `r` is a `DWORD&` loads *the reference
    variable*, i.e. the address, and `_asm mov r, eax` overwrites it, so
    `HOVER_FIXMUL(ref, x)` silently corrupts the reference and never
    touches the member. `CSub422F50::Sub_40FEE0` did exactly that with
    `DWORD& dwProjBase = *(DWORD*)&m_awUnk006C[8]`, and the recompiled game
    page-faulted at the next `mov ebp,[edx]` through the clobbered
    reference. The original shows what the source really was: every such
    update goes `lea ecx,[esi+0x7c] / mov [esp+N],ecx / mov ecx,[esp+N] /
    mov eax,[ecx] / ... / mov [ecx],eax`, i.e. a `DWORD*` in a stack slot
    of its own, with the second operand copied into a second slot right
    after it. `HOVER_FIXMULP`/`HOVER_FIXDIVPP` (hover.h) reproduce that:
    ```
    DWORD* _pdwA = (pdwA);   /* stored first */
    DWORD  _dwB  = (dwB);
    _asm mov ecx, _pdwA
    _asm mov eax, [ecx]
    ...
    _asm mov [ecx], eax
    ```
    Assigning the address to a *reused* `DWORD* pdw` local right before
    each pair of "store then scale" statements also reproduces the
    original's habit of re-emitting the `lea` for every use instead of
    keeping one pointer live (0x40fee0: 44.7% -> 60.0%, 0x427a80: 30.1% ->
    49.4%, and both stopped crashing). Where the original stores straight
    to the member (`mov [esi+0x78], eax`) keep the member expression - only
    the macro arguments need the pointer.

56. **Consecutive parameters copied into consecutive members are a struct
    assignment.** `CSub422F50::Sub_40FEE0` starts with
    `lea edx,[ecx+0x5c] / lea ecx,[esp+0x8c] / mov eax,[ecx] /
    mov ebx,[ecx+4] / mov ebp,[ecx+8] / mov eax,[ecx+0xc]` and four stores
    through `edx`: VC 2.2 only addresses both sides through a pointer like
    that for a whole-object copy. Four separate `m_dwUnk5C = dwLeft; ...`
    statements compile to eight `[esp+N]`/`[esi+N]` moves instead. Spelling
    it `*(RECT*)&m_dwUnk5C = *(RECT*)&dwLeft;` (the four parameters are
    contiguous in the frame) reproduces the original exactly.

57. **Corollary of rule 31: a `// GLOBAL: HOVER 0x...` marker is worth
    several points on every function that touches the global.** reccmp
    prints an unresolved operand as `<OFFSETn>` on the original side and
    `?name@@3KA (UNK)` on ours and scores it as a diff. Adding the marker
    above `g_dwUnk_46049C` alone took `CHoverApp::Sub_425AD0` from 63.6% to
    a 100% effective match, and markers for `g_dwUnk_46048C`,
    `g_dwUnk_4C57C4/C8/CC/D0` and `g_pLoadDlg_4C57D4` were worth ~3 points
    each on `Sub_425B20`, `Sub_4282F0` and `Hover_sub_40EB50`. The markers
    are comments, so they never change codegen - mark every global a new
    function references.

58. **Which constructor a compiler-generated static initializer inlines.**
    A global object with a constructor is built by the CRT initializer
    function of its translation unit (`_$$1000`, reached only from the
    `.CRT$XC*` table, so it has no call sites). Whether the constructor is
    inlined there is decided by *where it is defined*, not by its cost:
    with `/O2` (i.e. `/Ob1`) VC 2.1/2.2 expands only functions defined
    inside the class body and leaves a constructor defined out of line in
    the .cpp as a call - which is exactly hover.exe:0x433370, where the
    beginner human parameter block is filled with eleven folded `mov
    imm32` stores while the other five blocks are five
    `push`-eleven-doubles + `call 0x425290` sequences. Two more measured
    facts from tools/probe:
    - An *inline* constructor is expanded only up to an expansion budget:
      six global objects of a class whose inline constructor stores eleven
      `(long)(d * 65536.0)` values all inline, but from the seventh on
      only the first five expand and the rest call the out-of-line copy
      the compiler emitted. So "one inline block followed by calls" cannot
      be produced by a single inline constructor - it needs two
      constructors (here an inline 16.16 `long` one, called with `FIX16()`
      constants, and the out-of-line `double` one).
    - `(long)(1.0 * 65536.0)` *is* folded at compile time inside an inline
      expansion (`mov dword ptr [g], 0x10000`), but it also counts as a
      constant initializer: `DWORD a[11] = { FIX16(1.0), ... };` goes into
      `.data` with no initializer code at all. Runtime stores of folded
      fixed-point constants therefore prove the block is a class object
      with an inline constructor, not an initialized array.
    The floating-point constants pushed for the out-of-line calls come out
    in reverse argument order, high dword first (`push 0x40080000 / push
    0` for 3.0), and the `mov ecx, <object>` for the `this` pointer is
    scheduled into the middle of the pushes.

59. **`// GLOBAL:` markers work on constructed objects too** (tooling).
    `tools/vc2map.py`'s `_GLOBAL_DEF` regex only recognized `name;` and
    `name = ...` definitions, so a marker above `CPlayerParams
    g_HBegParams(...);` aborted the comparison. It now also accepts
    `name(`. Marking the nine parameter blocks was worth +45 points on
    `CHoverApp::Sub_425870` (55.0% -> 100%), +10.6 on
    `CHoverApp::OnOptionsCustomize`, +6 on each of the three registry
    loaders and +5.8 on each parameter dialog's OnSaveSettings - see
    rule 57.

60. **The same `_asm` block as a macro and as an inline function emit the
    same code - what matters is the order the arguments are stored.** The
    renderer's distance helper existed in two spellings (a `HOVER_DIST`
    macro whose four `long` temporaries were declared and assigned in
    separate statements, and the inline `Hover_Distance`), each used at the
    call sites where it had measured better. They differ only in argument
    order: the original stores the *second* point first (0x4046a6 and
    0x406d89 both go `mov [esp+0x54], y2 / mov [esp+0x5c], x2 /
    mov [esp+0x50], y1 / mov [esp+0x58], x1`), which the macro reproduces as
    `HOVER_DIST(r, hitX, hitY, lineX, lineY)` and the function as
    `Hover_Distance(lineX, lineY, hitX, hitY)`; mapping the arguments
    positionally instead reverses the four stores and costs ~10 points.
    With the right mapping the two forms are indistinguishable: converting
    all six expansions in `Hover_sub_4045E0` from the macro to the function
    left its reccmp diff line-for-line identical (95.60% both ways), so the
    macro was deleted. Prefer the inline function: its `_asm` labels are
    renamed per expansion by the compiler (rule 17), so it needs no
    `__LINE__` label-pasting, and one helper cannot drift from the other.

61. **`CDWordArray::GetAt(i)` and `arr[i]` are not interchangeable in a
    loop**, even though both inline to `m_pData[i]` in release. Measured on
    `CSub40B800::~CSub40B800` (0x422cb0), whose tail is a
    "find `this` in the registry, swap with the last entry, shrink" loop:
    - `arr[i]` (the non-const `operator[]`, i.e. `ElementAt`, an lvalue)
      makes VC 2.2 hoist the `this` home slot into a callee-saved register
      across the loop (`mov esi,[ebp-0x10]` in the preheader), which then
      pushes the loop counter into ebx and the swap temporary into eax;
    - `arr.GetAt(i)` (an rvalue) leaves `this` re-loaded inside the loop, and
      the whole loop then lands on the original's exact register assignment
      (edi = i*4, esi = i, ebx = temp, `mov edx,[ebp-0x10]` per iteration).
    The price is that the GetAt form is *rotated* into a top-tested loop
    (single `cmp/jle` at the head, `jmp` back-edge) while the original is
    bottom-tested with a duplicated test; no spelling (`for`, `while`,
    guarded `do/while`, `continue`, body-`i++`, a temp for the element)
    changes that, and the guarded `do/while` spelling actually brings the
    `this` hoist back. Net: GetAt is worth ~8 points here (81.7% -> 89.4%).
    Same function, two more data points: a global assigned to a local right
    before its guard (`pEngine = g_pUnk_4C56DC; if (pEngine != NULL && ...)`)
    is what produces the original's `mov ecx,[g] / test ecx,ecx` with ecx
    reused as the later call argument, instead of `cmp [g],0` plus a second
    load (rule 12's lever, +4.6 points); and `g == this` on a `void*` global
    always emits `mov eax,[this-slot] / cmp [g],eax`, never the original's
    `mov eax,[g] / cmp eax,[this-slot]` - operand order, casts to the class
    pointer type, casts of both sides to DWORD and a local for the global all
    canonicalize to the same thing (suspected 2.1-vs-2.2 drift, cf. rules 6,
    9, 11, 13 and 18).
    Finally, rule 18's "position inside the .cpp matters" applies with full
    force to this function: identical source scored 82.0% where the class's
    other members sit in gameobj.cpp, 86.6% parked in stubs.cpp, and 89.4%
    at the very end of gameobj.cpp - the end-of-file position is the one that
    reproduces the original's loop register assignment.

73. **Tooling: reccmp could not name the float constants in *our* binary.**
    `create_analysis_floats` skips an address when anything in the entity
    database already "intersects" it, and `intersects` counts an entity's
    *max size* (the distance to the next symbol) as well as its real size.
    The original has no symbols in `.rdata`, so its FP operands were all
    named (`fmul qword ptr [10000.0 (FLOAT)]`), but in our build every float
    pool sits inside the max size of the preceding linker-map symbol, so our
    side printed `<OFFSETn>` and reccmp scored an identical `fmul` as a diff
    (cf. rule 31 for the same effect on globals). `tools/compare.py` now
    replaces `reccmp.compare.core.create_analysis_floats` with a copy that
    only lets an entity with a *declared* size mask a float. Worth +0.22
    points overall with no source change: the three parameter dialogs'
    `OnInitDialog` went 75/63/50% -> 95/96/95%, their write-back helpers
    51% -> 75%, `CPlayerParams::CPlayerParams` 55% -> 78%,
    `Hover_sub_438540` 67% -> 78% and the eight `CBumperDoc::OnCmd340NN`
    handlers to 100.00%. Note the residual on the write-back helpers is the
    unscoreable `call __ftol` (rule 20), not a real diff.

74. **A member array copied element-wise into a stack buffer gets software-
    pipelined; a pointer local stops it.** `Hover_sub_42E420` (0x42e420)
    writes twelve permuted `theApp.m_avkControls[i]` into a `DWORD[12]`
    registry buffer. Written as `adw[k] = theApp.m_avkControls[i];` VC 2.1
    knows the two objects cannot alias and runs the loads one or two
    elements ahead of the stores (`mov ecx,[+368] / mov edx,[+376] /
    mov [esp+8],eax / mov eax,[+396] / mov [esp+0xc],ecx / ...`); the
    original has strict `mov eax,[src] / mov [dst],eax` pairs cycling
    eax/ecx/edx. Reading the source array through a `const DWORD*` local
    (`pvk = theApp.m_avkControls; adw[k] = pvk[i];`) reproduces the original
    exactly (67.92% -> 96.23%). Same family as rule 35: naming the value
    changes what the scheduler is allowed to move.

75. **Where a `for` counter is *declared* decides its register, even when
    the compiler strength-reduces it away.** In `Hover_sub_42DC50`
    (0x42dc50) the key-name lookup `for (int i = 0; g_aKeyNames[i].nID; i++)`
    put the counter in edi and the strength-reduced table pointer in esi,
    the other way round from the original; hoisting the declaration to
    function scope (`int i;` next to the other locals) swaps them and takes
    the function from 81.82% to 90.91%. The general form of rule 8: for a
    function whose only remaining diff is a swapped callee-saved pair, try
    every permutation of the local declarations - a five-local sweep of
    `CSub42D910::Sub_42DB60` (0x42db60, 120 builds, ~20 s each) found one
    ordering worth +8.6 points over the natural one.

76. **The parameter dialogs' `OnInitDialog` order-of-evaluation depends on
    the function's position in the .cpp, not on the expression.** All three
    debug dialogs run the same `FIX_TO_DBL` expansion (rule 46), but only
    `CSub430780::OnInitDialog` emitted the original's schedule (integer term
    first); `CSub4319A0::OnInitDialog` and `CSub432A50::OnInitDialog` emitted
    the fraction term first, and *no* spelling of the macro changes that -
    swapping the two products in the source is a no-op in the real build
    (in an isolated probe it does flip, and the direction of the flip even
    depends on the number of expansions, so the probe cannot be trusted
    here). Moving each `OnInitDialog` *after* its class's fixed-point
    write-back helper (`Sub_432660`, `Sub_432DB0`) restored the original's
    schedule: 50.50% -> 75.32% and 63.66% -> 82.58% before the float-naming
    fix of rule 73, 94.92% and 96.07% after it. Cf. rules 18 and 45.

77. **Two zero-materialization cases that are *not* source-controllable**
    (more evidence for rules 9 and 29). `CSub42E790::OnInitDialog`
    (0x42ea00) uses eight constant zeros in one block - two `cmp [g],0`,
    three `push 0` and three `mov [this+N],0`. The original keeps them all
    as immediates and spends ebx on the `EnableWindow` import; our build
    hoists the zero into ebx, spends esi on the import and needs a third
    callee-saved register for `this`, which is the whole 37.65% diff.
    Probed without effect: `::EnableWindow(m_hWnd, FALSE)` vs
    `CWnd::EnableWindow(FALSE)`, splitting the `&&` into nested ifs, and
    four positions for the function inside the .cpp.
    Likewise `CSub42E790::OnOK` (0x42eb40) is a pure 4-byte frame shift: the
    original reserves 0x10 and lays out hKey, dwDisposition, the 4-byte
    registry buffer and *one dead dword*, while ours reserves 0xc and puts
    the buffer at the top of the frame (rule 15). Neither declaring the
    `BYTE[4]` at function scope nor replacing it with a `DWORD` written
    through a `BYTE*` reproduces the original's layout.

78. **VC 2.1's optimizer silently gives up on a very large function.**
    `CBumperDoc::OnOpenDocument` (0x415450, 9230 bytes) is unoptimized
    code in hover.exe - `this` and every temporary homed on the stack, a
    `jmp $+5` at the end of most statements - and so is *our* build of the
    same source at `/O2`. `#pragma optimize("", off)` (rule 49) around it
    is a complete no-op: the .obj is byte-identical with and without it.
    So do not read a `jmp $+5`-riddled function as proof of the pragma;
    check whether the function is simply too big for C2's optimizer first.
    Handy consequence: an `/Od` listing is a statement-by-statement
    transcript of the original source, which is what made the loader's
    control flow, its local set and even its `try`/`catch` recoverable.

79. **`/Od` stack-slot order is not source-controllable.** In the same
    function neither declaration order (reversing the whole block changed
    the frame by nothing and the score by 0.4 points), nor scope nesting,
    nor first-use order predicts which local lands where; even *renaming*
    a padding array moved every slot (`adwFramePad` -> `adwPad` was worth
    0.6 points), which points at C2 walking its identifier hash table.
    What *is* controllable, and what dominates reccmp's score on a large
    `/Od` function, is the **total frame size**: the `this` home slot and
    the whole compiler-temporary region are laid out from the bottom of
    the frame upwards, so getting `sub esp, N` exactly right lines up the
    `this` references (57 of them here) and ~80 temporaries at once.
    Measured on 0x415450: same source, frame 0x3f4 = 42.3%, frame 0x44c
    (the original's) = 55.1%.

80. **Reserve the missing frame bytes with a placeholder while the
    remaining temporaries are unknown.** A plain `DWORD adwPad[N];` with
    one store works at `/Od` (0x415450, 0x44c); at `/O2` the dead local is
    deleted and the array has to be `volatile` (`CBumperDoc::Sub_412250`
    0x412250: 0x24 bytes short, 45.6% -> 48.6%; `CMerlinObject::Serialize`
    0x411b10: the 4-byte slot of rule 48, 86.0% -> 91.2%, leaving only the
    five unscoreable MFC calls). Label them clearly - they are not
    variables of the original, only a stand-in for temporaries our
    expressions do not yet generate.

81. **One name buffer per code path.** `CBumperDoc::Sub_412250`'s frame
    holds four separate `char[0x80]` arrays at ebp-0xf4/-0x174/-0x1f4/
    -0x274, one inside each of its four `do/while` search loops, while the
    `CString` temporary at ebp-0x74 is shared by all four. A single
    function-scope `szName` puts every buffer 0x180 bytes too high. When
    consecutive 0x80-aligned frame offsets appear in mutually exclusive
    branches, they are per-block declarations, not one reused array.

82. **`while (s > 0) { ...; s-- }` on a `short` counter keeps the counter
    16-bit; the original widens it.** hover.exe's Merlin comment-skip loops
    test the `short` (`test cx,cx / jle`) and then run an `int` down
    counter (`movsx edi,cx` ... `dec edi / jne`). Only the two-variable
    spelling reproduces both halves:
    `sCount = (short)pAr->ReadWord(); if (sCount > 0) { i = sCount; do ar
    >> by; while (--i); }` - `for (i = sCount; i > 0; i--)` widens but
    tests `edi`, and the plain `while` loop emits `dec di`
    (0x411b10: 76.7% -> 81.0% -> 86.0%).

83. **A 16-bit `sar` is a `(short)` cast around the shift.** In
    `CMerlinTexture::Serialize` the span accumulator is
    `sar ax,cl / movsx edx,ax` twice; `((short)wX >> nLevel)` gives the
    int-promoted 32-bit shift, `(short)((short)wX >> nLevel)` gives the
    original's pair (+0.7 points).

84. **`this`-relative `lea` into a member CString means a call on that
    member.** `CMerlinObject::CMerlinObject` (0x401030) ends with
    `lea ecx,[eax+4] / ... / call <lib>` after the vtable store; that call
    is `m_strUnk04.Empty()`, which our reconstruction omitted entirely
    (72.7% -> 79.4%). Worth checking every constructor whose tail has a
    `lea ecx,[this+N]` into a CString member.

85. **A dllimport call target is CSE'd into a callee-saved register the
    moment one is free for the whole function.** With four `SetRect`
    calls and a spare register VC 2.1 emits `mov esi,[__imp__SetRect]`
    once and `call esi` at every site; occupy all four callee-saved
    registers across the calls (probed with four live `int`s) and it
    falls back to the original's `call dword ptr [__imp__SetRect]`.
    `Hover_sub_41F540` (0x41f540) calls SetRect 97 times, so this one
    decision is 97 scored diffs: driving its three waypoint loops with a
    plain `int` counter (which needs `ebp` across the calls) instead of
    the memory-resident `volatile` one removes it, +14 points. The
    balance is fragile - anything that changes the translation unit can
    make the allocator spill the counter and take `esi` back (see rule
    74), so re-check the `call esi` count after every edit to the file.

86. **A scalar declared beside an array lands *below* it in the frame.**
    VC 2.1 puts aggregates at the top of the frame and scalars underneath
    (probed: `volatile int j; volatile int nm[2];` gives j at [esp+0],
    nm at [esp+4]/[esp+8]), while three separate `volatile int`s are laid
    out in declaration order upwards. hover.exe:0x41f540's frame is
    exactly that shape - a loop temporary at [esp+0x10] with the two
    paired coordinate temporaries above it at [esp+0x14]/[esp+0x18] - and
    reproducing it with three scalars slides the pair 4 bytes down, which
    mis-scores all 241 of their accesses. Spell the pair as a two-element
    volatile array (with `#define`s to keep the names) and the scalar as
    a separate volatile, and the whole frame lines up.

87. **A new *type* in a .cpp is as disruptive as a new function
    (extension of rule 45).** Reconstructing the 16.16 constants of
    `CHoverApp::CHoverApp` with a local `typedef union { DWORD dw; struct
    { WORD wFrac; short sInt; } s; }` flipped the register allocation of
    all of hover.cpp: `Hover_sub_41F540` lost 20 points (its SetRect
    import CSE came back, rule 73), `Hover_sub_439560` 23 and
    `Hover_sub_438CA0` its 100% match. Writing the same accesses as
    `*(WORD*)&dw` / `*((short*)&dw + 1)` macros keeps the PDB type list
    unchanged and every other function where it was. Prefer casts to a
    new typedef in a file whose other functions already match well.

88. **`CHoverApp::InitInstance` (0x41c9e0) is another `#pragma
    optimize("", off)` function** (cf. rule 49): `this` is homed at
    [ebp-0xe4] and reloaded before every member access, each of the four
    device-context handles gets its own frame slot and is compared
    against memory rather than tested in a register, and no import
    pointer is cached. The pragma alone is worth +16 points.
    Two things about it that are *not* reproduced yet:
    - The body is wrapped in a `try`/`catch` whose handler is the catch
      funclet at hover.exe:0x41d3c2 (`mov eax,[ebp-0xc0] / push eax /
      call Hover_sub_4262B0 / add esp,4 / mov eax,<resume> / ret`, rule
      70's funclet shape). That is why the EH state runs 0/1 instead of
      -1/0 and why the prologue reserves the extra `sub esp, 4` esp-save
      slot at [ebp-0x10]. Spelling it out (and the `#include "texmisc.h"`
      it needs) costs ~80 points of register churn elsewhere in the file,
      so it is left as a comment for now.
    - VC 2.1 does not lay out /Od locals in declaration order the way the
      original does (hover.exe has hdcScreen at [ebp-0x14] and the four
      HDCs contiguously below it; ours scatters them over the frame
      whatever the declaration order or grouping). Unexplained.

89. **The sound code's fixed-point helpers are the by-pointer flavour.**
    `Hover_sub_40B6E0` (0x40b6e0) and `Hover_sub_41B7A0` (0x41b7a0) build
    every product with `HOVER_FIXMULP` (rule 50/55), not `HOVER_FIXMUL`,
    and keep the sine values and the running dot product in memory
    (`volatile`), which is what shrinks their register save list to the
    original's single `push esi`: 11.5% -> 31.5% and 25.4% -> 42.7%.
    0x40b6e0 also derives its second sine index from the first
    (`eax = angle - 0x80` then `ecx = eax + 0x80`), gives each of the four
    products a destination temporary of its own, and re-reads the two
    velocity arguments when it forms the side component instead of
    subtracting in place.

90. **Size the local C++ object before you name it.** A local MFC object's
    class is pinned by the distance between its `lea ecx,[ebp-N]` and the
    next stack slot the function actually uses: `CClientDC` is 0x14 bytes,
    `CPaintDC` 0x54, `CPen`/`CBrush`/`CFont`/`CMenu` 8, `CDC` 0x10. At
    hover.exe:0x407430 the WM_COPYDATA `COPYDATASTRUCT` lives at
    object+0x14, so the DC there is a `CClientDC` (GetDC/ReleaseDC), not
    the `CPaintDC` we had - which also matches the caller: 0x407430 is the
    present pass of the render state machine, not a WM_PAINT handler.
    Corollary: the *number of EH states* the prologue walks through counts
    constructors, not objects. Three `mov [ebp-4],0/1/2` around three vptr
    stores into the same slot is one object whose base classes each have a
    destructor (CObject/CGdiObject/CPalette at hover.exe:0x410CE0), i.e. a
    default construction; a single state per object is an out-of-line
    constructor call, i.e. `CBrush brush(color);` rather than
    `CBrush brush; brush.CreateSolidBrush(color);` (hover.exe:0x427F40,
    +36 points).

91. **MFC 3.1 defines CDC::BitBlt and CDC::StretchBlt inline**
    (AFXWIN1.INL), so every blit site in the original is the GDI import
    plus an inlined `pSrcDC->GetSafeHdc()` (the `cmp this,-0xB4` null test)
    reading the destination handle out of the DC object's `m_hDC`. Written
    that way each expansion also homes its *computed* arguments in stack
    temporaries of its own, in argument order at descending addresses,
    stored in reverse argument order - which is what the 0x224-byte frame
    of hover.exe:0x407430 is full of. Spelling the same blits as
    `::StretchBlt(dc.m_hDC, ...)` produces neither the temporaries nor
    their order. Same lever, weaker, for ordinary calls: passing a pair of
    struct fields through block locals makes VC home them (the six
    dashboard element calls in the same function).

92. **`#pragma optimize("", off)` is detectable in one grep.** Rule 49's
    /Od functions are exactly the ones whose bytes contain `e900000000`
    (`jmp $+5`); over CBumperView's 57 functions only 0x41ECE0 does, and
    wrapping it in the pragma took it from 25.2% to 47.0% on its own.
    Inside such a function the other /Od tells are readable as source: a
    pointer materialised into a fresh slot per statement is an inline
    accessor's return value, `mov [ebp-4],0` mid-function is the entry to
    a `try` block, and a three-dword EH prologue (`push -1` / `push
    <ehhandler>` / `push fs:[0]`, then `sub esp,4` and `mov [ebp-0x10],
    esp`) is C++ EH - four dwords with a scope table would be
    `__try`/`__except`. The catch funclet sits after the epilogue and
    names the handler: `mov eax,[ebp-0x2c] / push eax / call f / ret` is
    `catch (CException* e) { f(e); }`.

93. **An `if/else` whose else arm repeats the code after the `if` is a
    shared tail.** CBumperView::Sub_40D0A0 (0x40D0A0) looked like
    `if (flag) { draw(); } else { shadow(); shadow(); draw(); }`; the
    original is `if (!flag) { shadow(); shadow(); } draw();` - one `jmp`
    into the common tail instead of a duplicated call sequence. Rewriting
    it that way took the function from 26.4% to a 100% match. Whenever a
    diff shows the original's two arms converging *before* the last call
    of the taken arm, hoist that call out of the `if`.

94. **Reading a group of far struct fields through a local pointer.** When
    the original materialises a base (`lea ecx,[ebp+0x1B0]`) and then uses
    `[ecx]`, `[ecx+4]`, `[ecx+8]` for four consecutive fields at an offset
    over 127, the source read them through a pointer local
    (`RECT* prc = &pZone->m_rcViewport; ... prc->left, prc->top, ...`);
    naming the fields off the struct pointer emits four 32-bit
    displacements instead (+3 points per site on hover.exe:0x410510, and a
    `DWORD*` over an adjacent width/height pair is what keeps the original's
    `mov edx,[edi] / mov ecx,[edi+4]` live across four calls).

95. **A pointer-destination `_asm` multiply whose destination is a plain
    copy of another local is miscompiled.** `HOVER_FIXMULP` (rule 50/55)
    only ever writes its destination through the pointer temporary inside
    the `_asm` block, and VC 2.1 does not model that write: given
    `dwSpin = dwTangent; HOVER_FIXMULP(dwSpin, k); m += dwSpin;` it
    copy-propagates `dwTangent` over every later read of `dwSpin`, deletes
    the initialising store, and the expansion then multiplies an
    *uninitialised* stack slot while the member gets the unscaled value.
    `CPlayer::Sub_41AB10` (0x41ab10) shipped that way - the wall-bounce spin
    and turn feedback were applied unscaled. Any *memory* initialiser (a
    member, a table entry, `*p`) is safe; only a local-to-local copy is not,
    and the fix that also matches the original's code is
    `dwSpin = *(volatile DWORD*)&dwTangent;`, which emits exactly the
    load/store pair the original has (0x41ab10: 33.1% -> 36.6%,
    0x40a9f0: also needed for its two velocity components). Audit with a
    disassembly scan: after each `mov [ecx], eax` that ends an expansion the
    next stack read must be the slot the preceding `lea` took the address
    of.

96. **Reserving a frame slot for a local whose only store is dead.** Rule
    48 left the missing `sub esp, 4` of every Merlin `Serialize` override
    unexplained: the trailing-comment `BYTE by` gets a slot in the original
    even though MFC's inlined `operator>>(BYTE&)` store is dead-code
    eliminated, and VC 2.1 drops the slot with the store. A volatile
    self-assignment right after the read keeps the slot and emits *no*
    instruction of its own:
    ```
    ar >> by;
    *(volatile BYTE*)&by = by;
    ```
    (`volatile BYTE by;` does not compile against `operator>>`, and a
    volatile *read* does emit a load.) Worth 6 points on
    `CMerlinLine::Serialize` (72.4% -> 78.5%, its tail is now
    instruction-exact), 6 on `CMerlinLocation::Serialize` and ~19 on
    `CMerlinStatic::Serialize`. It only works where the store really is
    dead: in `CMerlinBSP::Serialize`, whose loop calls the out-of-line
    `ReadByte(BYTE*)`, the same line costs a real load and store.
    Companion fix for the same loops: the original counts down in a
    sign-extended register (`movsx ebx, cx` / `dec ebx` / `jne`), which
    needs an explicit `int` copy -
    `if (n > 0) { i = n; do { ... } while (--i != 0); }` - because
    `while (n > 0) { ...; n--; }` on a `short` emits a 16-bit `dec`.

97. **A `short` loop/index scalar re-extends at every use; a `long` does
    not.** `CPlayer::Sub_408550` (0x408550) reads the movement angle into a
    `short` and uses it twice (`(a + 0x80) & 0x1FF` and `a & 0x1FF`); VC 2.1
    keeps it in `cx` and emits `movsx`/`xor+mov ax,cx` per use, while the
    original sign-extends once into a 32-bit register. Declaring the local
    `long` and casting at the *read* (`lAngle = (long)(short)m_awUnk0194[5];`)
    reproduces the original's single `movsx ecx, word ptr [esi+0x19e]`
    (22.7% -> 41.1% together with the pointer multiplies, 51.4% in total).
    This is rule 16 for an index rather than a comparison operand.

98. **A cached member that must not be forwarded into a later
    read-modify-write.** `CPlayer::Sub_40ADE0` reads `m_dwUnk014C` into a
    local after a call and tests it twice, but still decrements the
    *member* (`dec dword ptr [esi+0x14c]`). Written as
    `lCount = (long)m_dwUnk014C;` VC 2.1 proves the two equal and turns the
    decrement into `dec edi` + a store back, which keeps the local alive to
    the end of the function and costs the whole tail its register
    assignment; `lCount = *(volatile long*)&m_dwUnk014C;` keeps them
    distinct, the member decrement comes back and the local dies early
    (+16 points on its own, 28.5% -> 52.8% for the function).

99. **Constructors zero their WORD blocks one store at a time.**
    `CPlayer::CPlayer` (0x41ac30) sets 36 `WORD` members to zero; a
    `for` loop over the arrays becomes a `rep stosd` that VC cannot
    schedule, while the original's individual `mov word ptr [eax+N], si`
    stores are hoisted *ahead of* the four `CObList` member constructions.
    Writing the assignments out took it from 50.5% to 71.0% - the same
    lesson as rule 47 for doubles.

100. **`strcmp` is a real call in hover.exe.** VC 2.1 expands `strcmp`
    inline at /O2 (a ~20-instruction byte loop ending in `sbb eax,eax /
    sbb eax,-1`); the original calls the CRT copy at hover.exe:0x43E87E at
    all six texture-name tests of `CMerlinStatic::Serialize`.
    `#pragma function(strcmp)` at the top of the .cpp restores the call
    (30.8% -> 36.1% on its own). Cf. rule 23 for the copy/length helpers.

101. **Open: the CPlayer physics frames are 0x18-0x7c bytes smaller than the
    original's.** Even with every multiply in the pointer-destination form
    and a destination temporary per expansion, `Sub_42B8E0` comes out 0x2c
    short of the original's 0xcc, `Sub_40ADE0` 0x18 short of 0x7c,
    `Sub_40A9F0` 0x2c short of 0x5c and `Sub_40B0B0` 0x7c short of 0xcc, so
    every displacement in them is wrong even where the instruction stream
    matches one for one. Two observations that should pin it down:
    - each expansion block in the original is *three* dwords, not two -
      `_dwB`, `_pA` and one the function never writes (0x60/0x6c/0x78/0x84
      in 0x40a9f0, 0x88..0x94 in 0x42b8e0), which is rule 71's mystery seen
      from the other side. Probed and rejected: an unreferenced third macro
      temporary, an inline `_asm` function with a declared return type, an
      inline `CFixed*`-returning method, `return *pA` with the result
      unused - none allocates the slot.
    - the originals also home almost every scalar and do their arithmetic
      in memory (`mov [esp+N],eax / sub [esp+N],ecx / mov eax,[esp+N]` for
      one subtraction in 0x40ab70), which is the fingerprint of a `CFixed`
      value class whose inline operators take their operands by pointer
      (rule 49's corollary), not of plain DWORD arithmetic. Reconstructing
      that class and rewriting the physics in it is probably what closes
      these functions, and it would explain the third slot as the
      operator's result temporary.

102. **A sparse dispatch on one member value is a `switch`, not an
    `if`/`else if` chain.** `CFloorObj::Sub_42AD60` (0x40c620) tests
    `m_dwUnkB0` against -1, 7 and 6 with three `cmp`/`je` at the top and
    lays the four bodies out *after* the dispatch. VC 2.2 emits an
    `if`/`else if` chain as test1/body1/test2/body2 (37.96%) whatever the
    arm order, but a `switch` with the same cases emits exactly the
    original's shape (69.68%, +32 points); the same lever is worth +4 on
    `CFloorObj::Sub_42AD40` (0x40d730). Two caveats measured on 0x40c620:
    the *source* order of the cases does not change anything (VC sorts the
    comparison chain itself - ours comes out ascending-unsigned, the
    original's is descending, which is the residual 4-instruction diff),
    and the last case in the original falls through into its body
    (`jne <default>`) where ours jumps to it (`je <body>`).

103. **The pool `operator new`s re-read their globals after `AddTail`.**
    Every `CXxx::operator new` in the maze-object module has the shape
    ```
    pObj = (CXxx*)g_dwHead;
    if (pObj == NULL) {
        g_dwHead = (DWORD)::operator new(g_dwCount * sizeof(CXxx));
        g_listBlocks.AddTail((void*)g_dwHead);
        nCount = g_dwCount;              /* both globals re-read */
        pObj   = (CXxx*)g_dwHead;        /* after the call        */
        for (i = 0; i < nCount - 1; i++)
            pObj[i].m_dwLink = (DWORD)&pObj[i + 1];
        pObj[nCount - 1].m_dwLink = 0;
    }
    g_dwHead = pObj->m_dwLink;
    return pObj;
    ```
    Assigning the allocation result to the local first (`pObj = ...;
    g_dwHead = (DWORD)pObj;`) keeps both values in registers across
    `AddTail` and costs ~50 points; hoisting the count into `nCount` is
    what turns the free-list loop's back edge into the original's
    `dec ecx / jne` instead of a per-iteration reload of the global.
    Measured: `??2CPodObj` 20.59% -> 93.75%, `??2CWall` 22.22% -> 79.41%,
    `??2CFloorObj` 12.50% -> 77.33%. Note the *array-indexed* body
    (`pObj[i].m_dwLink = (DWORD)&pObj[i+1]`) is what strength-reduces to
    the original's link-address walk; a hand-written pointer walk gives a
    different (object-pointer) induction variable and scores 16 points
    less.

104. **A pair of dwords read through an inline function that returns them
    by value has a three-part fingerprint.** `inline POINT F(const T* p)
    { return *(POINT*)&p->m_a; }` compiles to (a) a dead `add r, off` /
    `lea r,[base+off]` for the address, (b) an 8-byte stack temp written
    once per basic block with *both* halves (`mov [T],lo / mov [T+4],hi`),
    and (c) for every use of the *first* member a further one-dword temp
    (`mov [T2],lo / cmp x,[T2]`), while uses of the second member read the
    register directly. Wherever hover.exe shows that pattern - the collider
    cell counts, the CSub40D320 segment endpoints - the source called such
    an accessor at every use site instead of caching the pair in a local.
    Reproducing it took `CFloorObj::Sub_408820` (0x41a960) from 23.6% to
    43.9% *and* restored its 0x98-byte frame exactly. Caveat: VC 2.2 only
    materialises the 8-byte temp when the accessor's argument varies (a
    list walk); on a loop-invariant `&this->m_obj04` it forwards the loads
    and the temps disappear, which is why `CWall::Sub_408820` (0x40ef30)
    still has a 0x3C frame against the original's 0x1B8 (suspected
    2.1-vs-2.2 drift, cf. rules 6, 9, 11, 13 and 18).

105. **`CSub422C80::Sub_42A9F0` exists twice in hover.exe.** The gauge
    update is both an out-of-line method (0x42A9F0, called from the two
    `Sub_42AD50` handlers) and an inline expansion in the five per-frame
    `Sub_42AD30` handlers. Marking our method `inline` reproduces the
    expansions but deletes the out-of-line copy (its marker then resolves
    to nothing) and wrongly expands it in the `Sub_42AD50` handlers too, so
    the expansion is spelled as a `do { } while (0)` macro
    (`HOVER_INLINE_SUB_42A9F0` in mazeobj.cpp, rule 21's technique) whose
    gauge argument must be a local pointer, since the original computes the
    gauge address once. Worth +36 to +45 points on each of 0x40DE80,
    0x42A660, 0x42B070, 0x42C430 and 0x414370. The method body itself needs
    `HOVER_FIXMULWP` (rule 50), not a plain product: 22.22% -> 88.24%.

106. **Two guards that return the same constant share one epilogue only
    inside one `if` (rule 38), and two *different* tails must be
    duplicated.** Both directions appear in the maze objects:
    `CPodObj::Sub_42AD60` (0x40cb40) needs the two height tests written
    `if (lTop < lPodZ || lPodTop < lZ) return 0;` (63.3% -> 92.3%), while
    `CWall::Sub_42AD60` (0x40d4b0) and `CWall::Sub_411570` (0x40f7f0) need
    each proximity test to carry its *own* `m_dwUnk2C = 1; return 1;`
    (38.6% -> 78.5% and 45.2% -> 84.1%) because the original emits that
    tail twice. Closing such a function with `if (a) {...} else if (b)
    {...} return 0;` is what makes both arms share the single `return 0`.
    Two more levers from the same three functions: the `+ 0x40` height
    sums and the `movsx`ed `short` members must be hoisted into `long`
    locals before the first test (the original computes them all up
    front), and a member read again after an intervening call must be
    re-read from the object rather than cached in a local (rule 65).

107. **`strcmp` is called, not expanded, in `CMerlinDynamic::Serialize`.**
    VC 2.2 expands `strcmp` as a 17-instruction intrinsic; hover.exe:
    0x42CD6F pushes two arguments and does `add esp,8`. `#pragma
    function(strcmp)` in front of the function restores the call
    (38.28% -> 40.71%). In the same function the trailing-comment counter
    is an `int` (`movsx edi,cx` + `dec edi`), not a `short`.

108. **`w &= ~8` on a WORD member is a byte-wide AND in the original.**
    hover.exe:0x4225D9 has `and byte ptr [eax+0x24], 0xf7` where VC 2.2
    emits `and word ptr [eax+0x24], 0xfff7` for `m_wFlags &= ~8`. Writing
    `*(BYTE*)&m_wFlags &= ~8` reproduces it (+5.5 points on
    `CMapEraserPod::Sub_42AD40`, 0x422470).

109. **The level index is signed.** `LVLPARAM(g_dwUnk_46049C)` on the
    `DWORD` global compiles the macro's `< 16` test to `jb` and its `% 3`
    to `div`; hover.exe has `jl` and `cdq`/`idiv`, i.e. the index was an
    `int`. Casting at the use site (`LVLPARAM((int)g_dwUnk_46049C)`) is
    worth +18 points on `CFlag::Sub_42AD40` (0x41ae40).

110. **The pod constructors are expanded inline at their one `new` site and
    still exist out of line - and we cannot have both.** Every
    `CXxxPod::Sub_42AD40` allocates a replacement pod with `new CXxxPod`,
    and hover.exe expands the whole constructor there (vtable store,
    member inits, `wsprintf`, `Lookup`) inside an EH frame, while keeping
    the out-of-line constructor (e.g. CSlowPod::CSlowPod at 0x422630) for
    the other translation units that also allocate pods. Defining the
    constructor in the class body reproduces the expansion (+23 points on
    `CSlowPod::Sub_42AD40`, 0x4226d0) but reccmp can then no longer resolve
    the out-of-line copy - `[ERROR] Failed to find function symbol with
    filename and line: src/mazeobj.h:NNN`, because the surviving copy is a
    COMDAT emitted by texture.obj/bumpview.obj - so the constructor's own
    ~82% is lost and "Implemented" drops one function per class. Measured
    over all seven pod classes: 79.16% -> 78.69% overall and 768 -> 761
    implemented, i.e. an exact wash plus a metric regression, so the seven
    `Sub_42AD40` handlers are left at 54-60% with the missing EH frame.
    Turning `/Ob2` on for mazeobj.cpp is worse still (75.66%): it inlines
    the base-class forwarders too and kills 21 functions that are 100%
    today (cf. rule 41).

111. **The state-machine hand-over re-reads the parent for the store.** Every
    CRobotPlayer AI component ends with the same four statements, and the
    original's shape is
    ```
    pParent = m_pUnk88;            /* one load, kept in a register */
    pParent->m_dwUnk01FC = ...;    /* through that register */
    pNext = &pParent->m_objNNN;    /* the lea, before the Leave call */
    pParent->m_pUnk0220->Sub_42ECF0();
    m_pUnk88->m_pUnk0220 = pNext;  /* re-read after the call */
    pNext->Sub_42ECE0();
    ```
    Using the cached `pParent` for the *store* keeps it live across the
    call and costs a callee-saved register plus its push/pop; re-reading
    `m_pUnk88` there is what the original does (`mov ecx,[esi+0x88]` right
    after the vtable call). Worth +35 on hover.exe:0x40BFF0 (58% -> 93%),
    +15 on 0x40BB40, +19 on 0x40A790, and 2-3 points on 0x42EF40, 0x4150F0
    and 0x42ED90. Complements rule 37 (the `lea` before the Leave call).

112. **Two `return 0`s separated by a call share one epilogue only if the
    source has one `return 0`.** Rule 38 covers adjacent guards; when the
    guards straddle a call (`if (a) return 0; ...; if (b) return 0; ...;
    return 1;`) VC 2.1 emits the whole `xor eax,eax` + pops + `ret` twice,
    while the original has a single copy both guards branch to. Nesting the
    body and closing with one `return 0`
    (`if (!a) { ...; if (!b) { ...; return 1; } } return 0;`) reproduces it:
    +19 on hover.exe:0x40A790, +27 on 0x40BB40 (69% -> 97%).

113. **Pool `operator new` (CRobotPlayer 0x42C320, CMerlinDynamic 0x42CBC0,
    CRegionMatrix's node chunk 0x4086A0).** All three have the same shape,
    and four things have to be spelled the original's way:
    - the free-list head is loaded **once at the top** (`p = (T*)g_head;
      if (p == NULL) {...}`) and that same pointer is the return value;
    - the chunk base and the pool size are **re-read out of the globals**
      after the `AddTail` call, not carried in locals across it;
    - the free list is threaded with a `void**` walking the *link fields*
      (`ppLink = (void**)((BYTE*)p + LINKOFS); do { ppLink = (void**)
      ((BYTE*)ppLink + STRIDE); i--; *(void**)((BYTE*)ppLink - STRIDE) =
      (BYTE*)ppLink - STRIDE + LINKOFS'; } while (i != 0);`), which is what
      produces the `lea next / add p,stride / mov [p-stride],next` body;
    - the last entry's link is reached **by index** off the base
      (`pBlock[n - 1].m_pLink = NULL`), not through the walking pointer.
    Worth 19% -> 58%, 38% -> 60% and 50% -> 65% respectively. The node
    chunk is additionally a real `new REGIONNODE[n]`, i.e. the compiler's
    vector-new expansion (size, null test, backwards constructor loop,
    `= 0` on the null path) around an empty out-of-line constructor.

114. **A colour is packed with `+`, not `|`.** `r + (g << 8) + (b << 16)`
    emits `xor ecx,ecx / mov cl,b / shl ecx,8 / add ecx,g / shl ecx,8 /
    add ecx,r` (VC folds the disjoint ORs into ADDs and reassociates into
    a Horner chain), which is what hover.exe:0x419960 hands
    `GetNearestPaletteIndex`; `RGB()` and the equivalent `|` expression
    both give `shl 8` / `shl 0x10` / `or` / `or` instead (+8 points).

115. **`/Oa` is the only thing that turns a store loop into `rep stosd`.**
    `#pragma optimize("a", on)` plus `if (n > 0) { for (i = 0; i < n; i++)
    a[i] = v; for (i = 0; i < n; i++) b[i] = 0; }` reproduces
    hover.exe:0x419530's two `rep stosd` fills instruction for instruction,
    single guard and following `lea eax,[edx*4]` included; no loop spelling
    does it under plain `/O2`. It is still not worth turning on for that
    function: assuming no aliasing makes VC cache far more member values,
    the frame grows from 0x18 to 0x2c and every `esp` displacement shifts
    (58% -> 41%). Recorded in case a function turns up whose whole body is
    fills.

116. **Reading hover.exe:0x409350 (`#pragma optimize("", off)`, rule 49).**
    Two measurements for whoever finishes it: the original emits 111
    five-byte `jmp $+5` separators against our 39, and its frame is 0x234
    against our 0x168 - i.e. the original's source has roughly three times
    as many inline expansions as our reconstruction, and 51 more dword
    temporaries. Two of the pieces are known:
    - a pair of `mov word ptr [ebp-N],0` stores followed by a `jmp` is one
      expansion of an inline "zero a 16.16 value" helper, and two of those
      followed by a third `jmp` is an outer helper zeroing a point;
    - the original's `+=` expansion has **three** temporaries (`mov eax,
      [dwA] / mov [T],eax / mov eax,[T] / add [dwB],eax`) where ours has
      two. Neither `void AddEq(DWORD)` nor `{ DWORD t = a; m += t; }`
      produces the third slot - VC coalesces a parameter with a local that
      is a direct copy of it. Passing the addend **by value as a struct**
      (`void AddEq(CFixed f) { m_dwValue += f.m_dwValue; }`, called
      `((CFixed*)&dwB)->AddEq(*(CFixed*)&dwA)`) does: a by-value struct
      parameter is never coalesced with the caller's parameter home.

117. **`#pragma optimize("g", off)` is the second unoptimized-function
    spelling, and it is the one hover.exe's projection helper uses.**
    Rule 49 reproduces `CRobotPlayer::Sub_409350` with
    `#pragma optimize("", off)`, which also switches inlining off. The
    renderer's `CSub422F50::Sub_4055B0` (0x4055B0) has the same /Od frame -
    ebp, every value homed, a five-byte `jmp $+5` between blocks - but
    still expands inline functions and still `call`s CFixed::Sub_4038E0
    out of line, which is exactly `#pragma optimize("g", off)` (only the
    global optimizer off, /Ob1 kept). Measured on that function
    (9.59% -> 43.72%), with these sub-rules:
    - **The `jmp $+5` is not a statement separator, it is the end of an
      inline expansion** (plus one at the end of the function). Plain
      statements emit none, so counting the jmps counts the inline levels:
      `dwR = f(g(x))` with both inline is two jmps, a real `call` inside an
      inline is one.
    - **A reference parameter of an inline expansion folds away with no
      slot and no code** - `inline void Shl32(__int64& q)` gives exactly
      `mov cl,0x20 / mov eax,[q] / mov edx,[q+4] / call __allshl /
      mov [q],eax / mov [q+4],edx / jmp`. So does a scalar parameter whose
      argument is a plain, non-address-taken lvalue. A *class* passed by
      value gets one slot per expansion.
    - **Slot layout inside an expansion**: the last parameter is highest
      and the first lowest, the arguments are stored right to left, and the
      callee's own locals sit *above* the parameter block. Consecutive
      expansions descend in source order.
    - **Frame layout**: function-scope scalars run downward from `ebp-4` in
      declaration order, but an `__int64` declared at function scope is
      hoisted to the top of the frame; one declared in a nested block is
      allocated *below* all function-scope scalars (that is how
      hover.exe:0x405806's `llQ` sits at `ebp-0x14` under three longs).
      Mixing types perturbs the order: three `long`s came out in
      declaration order where a `DWORD` in the middle of them did not, so
      match the original's types before chasing its slot numbers.

118. **A small struct returned by value from an inline function is the
    renderer's point accessor.** hover.exe reads every world/projected
    point pair as an 8-byte value: `lea eax,[obj+N] / mov ecx,[eax] /
    mov edx,[eax+4]` into a *fresh* stack temporary at every use, with the
    unused half's store deleted, and the comparison reading straight out of
    that temporary (`mov [tmp],eax / cmp [tmp],ecx`). Probed: neither a
    plain member read nor a by-value POINT *parameter* does this (both
    forward the value into a register); only
    `inline POINT Hover_PtOf(const DWORD* p) { return *(const POINT*)p; }`
    does, because VC 2.1 treats the return slot as address-taken memory.
    Repeated identical expansions are CSE-ed by *reloading from the first
    temporary* rather than re-reading the member. Worth +22 points on
    `CSub422F50::Sub_404BB0` (0x404bb0, 12.51% -> 34.70%) and +8 on
    `Sub_401C00`. Its sibling idiom for a pair that is only ever read is a
    `POINT*` local over the two members (`lea`/`add` on the object pointer,
    then `[p]` and `[p+4]`), which is what Sub_401C00's four draw branches
    and Sub_404BB0's player triangle use.

119. **`max()` over two fixed-point values is a reference-returning inline.**
    hover.exe:0x405071 copies both candidates into temporaries, compares
    once and branches *twice* (`jl` then `je` to the same label), `lea`s
    the winner's address and dereferences it. The two branches mean the
    source really is `a < b || a == b`; the lea/deref means the helper
    returns a reference to one of its by-value parameters (a pointer select
    over two caller-side copies produces the same stream without the
    C4173 warning).

120. **`abs()` is the branchless `cdq / xor eax,edx / sub eax,edx`.** An
    `if (v < 0) v = -v;` compiles to `jns`/`neg` instead; the line
    rasteriser's two deltas (hover.exe:0x401988) use the intrinsic
    (24.80% -> 37.50% on `CSub422F50::Sub_401980`).

121. **A `volatile` pointer over a member group stops VC CSE-ing it into
    stack temporaries** (the read-side counterpart of rule 26). The clip
    tester `CSub422F50::Sub_401520` (0x401520) re-reads its four clip-rect
    members and both endpoint coordinates from memory at every comparison;
    ours CSE-ed them into six stack slots and grew the frame from the
    original's 4 bytes to 0x18. Declaring `volatile DWORD* pdwClip =
    &m_dwUnk00;` / `volatile long* pB = plB;` and indexing through them
    restored the 4-byte frame and took the function from 52% to 71%.

122. **Two more pointer-flavour fixed-point macros** (file-local in
    bumpview.cpp, cf. rules 50 and 55): `HOVER_FIXDIVPL(dwA, dwB)` is the
    divide counterpart of `HOVER_FIXMULP` (destination lvalue reached
    through a pointer temporary), and `HOVER_FIXMULWPR(dwR, dwA, wB)` is
    the three-temporary form the camera-basis setup uses - source value,
    destination pointer and 16-bit factor each in a slot of its own, the
    destination written *through* the pointer so the constant source is not
    folded away (`volatile` on the value temp is what keeps VC from
    deleting it). `CSub423030::Sub_4140E0` (0x4140e0) goes 17.69% -> 49.44%
    with them and matches the original instruction for instruction modulo
    two dead slots the original leaves in its frame.

123. **A frame slot the original spills but our allocator enregisters can
    be forced with `volatile` on the local.** `CSub403980::Sub_403A60`
    (0x403a60), the span walker, keeps five values live across the drawer
    calls but has only four callee-saved registers, so exactly one has to
    live in the frame. The original spills *both* span-end temporaries
    (`nOldX1` at `[esp+0x10]`, `nNewX1` at `[esp+0x14]`) and keeps the new
    span cursor in `ebp`; our build did the opposite and put the cursor in
    memory, which turns every `movsx r,[ebp]` / `add ebp,4` into a reload
    plus `add dword ptr [esp+N],4` and misaligns the whole function.
    Nothing source-level moved the allocator - not `register`, not
    declaration order, not moving the assignment before or after the copy
    loop, not folding the two temporaries into an `int[2]` (arrays are
    never enregistered, and that scored 8 points *lower* than volatile).
    Declaring the two temporaries `volatile int` inside the loop body is
    what reproduces the original's frame and register assignment: 8.25% ->
    31.04%. The cost is one extra reload per use where the original still
    had the value in a register; the win is that every pointer-cursor
    instruction lines up. Declaring the four span coordinates inside the
    `do { }` body rather than at function scope is worth another 2 points
    on its own.

124. **`(WORD)g_someDword` loads 32 bits; `*(WORD*)&g_someDword` loads 16.**
    Sub_403A60's index-fill loops read the current column out of the DWORD
    global `g_dwUnk_4A4534` as `mov di, word ptr [0x4a4534]`, and that is
    the *only* place in hover.exe that reads it 16 bits wide - everywhere
    else it is a plain dword. `wFill = (WORD)g_dwUnk_4A4534;` emits
    `mov edx, dword ptr [g]` and, worse, lets VC sink the load below the
    counter increment; `*pIdx = *(WORD*)&g_dwUnk_4A4534;` emits the
    original's 16-bit load in the original's position and needs no
    temporary at all (probed three spellings in tools/probe). The same
    probe pinned the companion rule: the byte step of a `WORD*` cursor
    (`pIdx += dwStep`) is hoisted out of the loop as `lea r,[step*2]` only
    when `dwStep` is a *local*; reading the global at each use emits
    `lea r,[r+step*2]` inside the loop instead.

125. **A global re-read after every call is a local, not a global
    reference.** The same function reloads `g_dwUnk_4A453C` (the span step)
    after each drawer call and keeps it in `eax` everywhere else. Writing
    `g_dwUnk_4A453C` at each use site emits `add r, dword ptr [g]` per use
    (no register); reading it into a local once before the loop *and*
    re-assigning that local as the statement right after each call
    reproduces the original exactly (this is rule 65's function-pointer
    shape applied to a scalar).

126. **Both span-append helpers read the buffer slot before bumping the
    count.** `CSub403980::Sub_403980` (0x403980) and `Sub_406D00`
    (0x406d00) end with `mov eax,[esi+0xc] / inc edx / mov [esi+0x14],edx /
    mov eax,[eax] / lea ecx,[eax+edx*4-4]`, i.e. the source loads
    `pNode->m_pUnk0C` into a `WORD**`, then increments the count, then
    dereferences and forms one cursor. Writing it the obvious way
    (`pw = *pNode->m_pUnk0C; pw[(n-1)*2] = ...`) emits the dereference
    before the increment and two indexed stores instead
    (0x406d00: 13.33% -> 40.00%).

127. **A frame slot written with the return code *and* with a mid-function
    temporary is one `volatile` variable, not two coalesced ones.**
    hover.exe:0x439e80 (WaveMixPlay) stores 0xC into [esp+0x14] in the
    prologue, 5/8/4 into it on every error path, the mixer play position
    into it while the wait list is drained, 0 into it at `Success:` and
    then reads it straight back (`mov [slot],0 / mov eax,[slot]` - the
    store-then-load of the same slot is the volatile fingerprint; a spilled
    local would keep the 0 in eax). VC 2.x does not colour stack slots, so
    the position temporary and the return code really are the same
    variable. Spelling it as one `volatile DWORD` reproduces the original's
    three-dword frame; two separate volatiles give four slots and score
    worse. Corollary found the same way: the initial value is
    MMSYSERR_HANDLEBUSY (0xC), which is what the re-entrancy guard at the
    top returns - a semantic bug in the reconstruction, which returned 0.
    Whole function 50.5% -> 56.9% together with rules 66 (the wave magic
    test as a ternary value) and 124.

128. **Declaring the "next" pointer *before* the "current" one swaps their
    registers.** `Hover_sub_438CA0` (0x438ca0) is a six-instruction
    free-list builder that the original writes with the walking pointer in
    ecx and the lookahead in eax; ours had them the other way round and
    scored 33%. Moving `DWORD* pNext;` above `DWORD* pNode;` - nothing
    else - makes it a 100% match. VC 2.x assigns scratch registers in
    declaration order, so when a small function's only diff is a two-way
    register swap, try permuting the declarations before anything else.

129. **`x += a * b` is not `x = a * b + x`.** VC compiles `+=` on a
    subscripted global to an in-place read-modify-write with the index
    folded into the addressing mode (`add [idx*8+g_tab+56], eax`), while
    the written-out form loads, multiplies, adds and stores through the
    element address the original already materialised with a `lea`. One
    such line in `CHoverApp::CHoverApp` (0x41c100) was worth 59.6% ->
    66.4%, because getting the shape wrong misaligns the whole 16-iteration
    loop that follows it.

130. **A 16-bit count parameter shows up as `cmp word ptr [esp+N],0`.**
    `Hover_sub_4389F0` (0x4389f0, the WaveMix mixing kernel) takes its
    frame count as a `WORD`: the original tests the parameter slot with a
    word compare and keeps the halving in 16 bits (`mov ax,[esp+0x38] /
    inc ax / shr ax,1`), where an `int` parameter gives `movzx`/32-bit
    arithmetic. Same tell in `Hover_sub_43B150` (0x43b150), whose device id
    is a `WORD` local (`mov ax,0` in one arm, `mov ax,[..+0x18]` in the
    other, `movzx eax,ax` at the push). 47.5% -> 55.9% and 65.3% -> 67.0%.
    In the same mixing kernel the one-source stereo path reads *both*
    channel volumes into locals before the count test instead of indexing
    the volume array inside the loop.

131. **`CHoverApp::InitInstance`'s try/catch is now in** (supersedes the
    open point in rule 88): `try { <whole body> } catch (CException* e)
    { Hover_sub_4262B0(e); } return TRUE;`, with `#include "texmisc.h"`.
    The EH prologue (the extra `sub esp,4` esp-save slot at [ebp-0x10], the
    0/1 EH state), the catch funclet at 0x41d3c2 and the whole epilogue
    then match instruction for instruction, and exceptions raised during
    startup are reported instead of escaping. It is only worth +0.1 points
    on the function itself, and the register churn it causes elsewhere in
    hover.cpp nets out to zero once the rest of this wave's changes are in
    (measured both ways: 84.54% accuracy either way), because two other
    things still cap the function: (a) ~64 instructions reference MFC globals/methods that
    reccmp cannot resolve in the original (`<OFFSET4>`/`<OFFSET5>`), and
    (b) the /Od frame layout. On (b): the original has *no* stack-slot
    reuse - hdcScreen at [ebp-0x14] with hdc1..hdc4 contiguous below it,
    every `return FALSE` with its own result temporary at [ebp-0xc4..-0xd0]
    and `this` at [ebp-0xe4] - while our VC allocates /Od slots in an order
    that is neither declaration order nor scope order and reuses them
    across blocks (measured: hdcScreen -0x20, hdc1 -0x18, hdc2 -0x58,
    hdc3 -0x68, hdc4 -0x70). Still unexplained; the frame is 0xcc against
    the original's 0xd8.

132. **A large part of what is left in the sound module is string padding,
    not code.** Every `push "<literal>"` in hover.cpp scores as a diff
    because our string pool pads each literal to a 4-byte boundary
    (`"WavMix32\x00\x00\x00"`) where the original packs them
    (`"WavMix32"`). It is a build-level difference (no `/Gf` in CFLAGS),
    not something a source change can reach, and it costs several points
    on every WaveMix function - worth checking before attributing their
    residual to codegen.

133. **Name the first term of a two-term sum; do not try to reorder it.**
    The three parameter dialogs' `FIX_TO_DBL` (rule 46) writes
    `floor(int * 10000.0 + frac * 0.152587890625 + 0.5) * 0.0001`. As one
    expression VC 2.1 always emits the *fraction* conversion first (the
    unsigned one, `fild qword`), while hover.exe always converts the
    integer part first. Turning the macro into
    `static __inline double FixToDbl(FIXED1616 fx)` whose body is
    `double dInt = (double)fx.s.sInt * 10000.0;` followed by the `floor`
    expression reproduces the original everywhere:
    `CSub430780::OnInitDialog` 64.05% -> 94.93%,
    `CSub4319A0::OnInitDialog` 63.94% -> 94.92%,
    `CSub432A50::OnInitDialog` 73.80% -> 96.07% (residual: the unscoreable
    `call floor`). The named temporary also makes the schedule independent
    of the function's position in the .cpp, so it *supersedes* the
    positional workaround of rule 76 - which had silently stopped working
    once other translation units changed (rule 45). Without the temporary,
    the same inline helper fixes only whichever of the three happens to sit
    in the lucky spot.

134. **VC 2.1 canonicalizes commutative operands, so swapping the source
    terms is a no-op.** Measured three ways on the same build: swapping the
    two products inside `FIX_TO_DBL`, swapping `(rc.left - rc.right) / 2`
    and `(rcView.right + rcView.left) / 2` in `CSub424720::OnInitDialog`
    (0x4247f0), and swapping the operands of the inner `+` - all produce a
    byte-identical .obj. Naming *both* halves in locals (`int a = ...;
    int b = ...; x = a + b;`) is also a no-op: the temporaries are folded
    back and re-canonicalized. What does move the order is rule 123's
    single named term or rule 125's scope change.

135. **Declaration *scope* is a stronger register/schedule lever than
    declaration order (rule 75).** Three measurements in optionsdlg.cpp:
    - `CSub42D910::Sub_42DB60` (0x42db60): the insertion sort's key had the
      strength-reduced `&avk[j]` pointer and `dwKey` in the opposite
      registers from the original, and a full 120-permutation sweep of the
      five function-scope declarations could not fix it. Moving one
      declaration into the loop body (`for (i = 1; ...) { DWORD dwKey =
      avk[i]; ... }`) swapped the pair: 81.03% -> 100.00%.
    - `CSub424720::OnInitDialog` (0x4247f0): moving `pView` and `rcView`
      into the `else` block that is their only user flipped the
      canonicalization of rule 124 in all three `x`/`y` expressions at
      once: 87.18% -> 99.15% (only the unscoreable `CDialog::OnInitDialog`
      left).
    - `Hover_sub_40EB50` (0x40eb50): the same move for the progress bar's
      rects, `CSize` and four `int`s, 91.43% -> 91.90%.
    So when a function's only remaining diffs are a swapped register pair
    or a swapped pair of commutative terms, try block scope for every local
    whose uses are confined to one block *before* sweeping declaration
    orders - it is one build instead of n!.

136. **An argument expression that hover.exe computes before the pushes is
    a statement of its own.** `CSub424720::OnInitDialog`'s splash-relative
    branch calls `::MoveWindow(m_hWnd, x, g_dwUnk_4A487C, cx, cy, FALSE)`.
    With the `x` expression written inline as the argument, VC evaluates it
    *after* pushing `cx`/`cy` and keeps them in caller-saved registers;
    hover.exe computes `cy`, `cx` and `x` first, which forces `cx`/`cy`
    into edi/ebx and interleaves the pushes with the `x` arithmetic.
    Assigning `x = g_dwUnk_4A4878 + (rc.left - rc.right) / 2;` on its own
    line before the call reproduces it (74.89% -> 87.18%). Cf. rule 15:
    naming a value is what lets the scheduler move it.

137. **A registry value buffer can be bigger than the value.**
    `CSub42E790::OnOK` (0x42eb40) reserves 0x10 where a `BYTE buf[4]`
    declared in the `if` block gives 0xc, and the buffer sits *above*
    `hKey`/`dwDisposition` instead of below them (rule 77 recorded this as
    "not source-controllable"). `BYTE buf[8]` at function scope - four
    bytes written, `4` passed as the size, as in the original - reproduces
    the frame and the slot order exactly: 61.54% -> 80.77%. When a `/O2`
    frame is exactly one slot too small, widening the array is worth trying
    before rule 80's placeholder.

138. **Tooling: the recompiled side's string names carry the linker map's
    padding NULs.** Same shape as rule 73's floats. reccmp sizes a string
    entity from the symbol it finds, and vc2map derives that size from the
    distance to the next map symbol, which includes the linker's alignment
    padding; `create_string_entities` then decodes `size - 1` bytes, so our
    `"SoundSettings"` becomes `"SoundSettings\x00\x00"` while the original,
    read with `read_string`, is plain. Every `push <string>` therefore
    scored as a diff. `tools/compare.py` now wraps
    `reccmp.compare.db.entity_name_from_string` to `rstrip("\x00")` (the
    original side can never show a trailing NUL, so nothing real is
    masked). Worth +0.66 points of whole-binary Accuracy with no source
    change: the three `OnDeleteSettings` 86.67% -> 100.00%, the three
    `OnSaveSettings` 90.38% -> 98.08%, `CSub42E790::OnOK` +4,
    `Hover_sub_42E420` -> 100.00%.

139. **The projection call takes a POINT by value.** `CSub422F50::Sub_404BB0`
    hands `Sub_4055B0` a whole point: at every call site the original
    copies both halves into a *fresh adjacent 8-byte slot pair*, then
    reloads them for the two pushes (hover.exe:0x404D01/0x404D14 and
    0x404D5B/0x404D6F). Probed (rule 118's sibling): a plain pair of
    scalar arguments never builds the pair, and a named `POINT` local
    filled field by field is coalesced away; only an **inline forwarder
    with a by-value POINT parameter**
    (`static inline void Hover_Project(CSub422F50*, DWORD*, POINT)`)
    produces it, because a by-value class parameter of an inline
    expansion always gets a slot of its own (rule 116). The same
    forwarder also fixes the `cmp [temp],reg / jge` direction of the
    first clip test.

140. **The `_asm`-through-pointer fixed-point macros defeat VC 2.2's copy
    propagation, and that is a miscompile, not a cosmetic.** `HOVER_FIXMULWP`
    writes its destination through `_pA`, which VC 2.2 does not follow: if
    another local was initialised from the same value, VC treats the two as
    equal *after* the asm block and reads the pre-multiply copy back
    (hover.exe:0x4050BC - our build compared and stored `dwWide` where the
    original uses the product). Declaring the destination `volatile` and
    casting it at the macro call (`HOVER_FIXMULWP(*(DWORD*)&dwCap2, 8)`)
    restores the reload and matches the original. Check every macro
    destination that shares an initialiser with another local.

141. **A local that is mutated in place between calls stays in memory only
    if its address is taken.** hover.exe's marker code adjusts the two
    corner points of the selection diamond with `sub dword ptr [esp+N],0xa`
    / `add dword ptr [esp+N],0x14` between four `Sub_401980` calls; plain
    `long` locals (or plain `POINT` members) are enregistered instead.
    Reaching them through `POINT*` locals (`pPtA = &ptA; pPtA->y -= 0xA;`)
    gives the original's memory read-modify-write *and* its exact frame
    size. `CSub422F50::Sub_4053A0` 47.49% -> 71.20%. Read the shared
    source once into scalars first, or the aliasing forces a re-read of
    the source pair at every store.

142. **Which side of a clip compare lands in the register is decided by the
    *bound*, not by the coordinate.** `cmp [coord], reg` (the original's
    shape) needs the viewport global copied into a local first
    (`lClip = (long)g_dwUnk_4A47BC; if (plPt[0] < lClip) ...`); writing the
    global inline makes VC hoist the *coordinate* into the register and
    emit `cmp [glob], reg` instead. A `volatile` pointer over the
    coordinates (rule 121) is not enough on its own - it only guarantees
    the re-read. Worth +5.5 points on `CSub422F50::Sub_404BB0`'s pod
    branch; the same change applied to the wall branch's `&&` pairs makes
    things worse, so measure per test.

143. **hover.exe constructs arrays with an explicit constructor call, not
    placement new.** `p->CSub419520::CSub419520();` (a VC 2.x extension)
    gives exactly `mov ecx,p / add p,size / call ctor / dec n / jns`;
    `new (p) T()` adds a `test/je` around the call that the original does
    not have. `CSub406090::Sub_4192F0` 65.57% -> 88.89% (with the
    allocation-failed tail spelled as the `else` arm so it stays out of
    line).

144. **Four consecutive same-type arguments that end up in a 16-byte member
    block are a by-value RECT.** The original reads them with one
    `lea ecx,[esp+N]` over the argument home followed by three loads, one
    store, one load and three stores (hover.exe:0x411D8B, 0x40FEFA,
    0x40FBB0). Two functions in bumpview.cpp were reconstructed as
    "ignore the rect arguments and copy the rect from somewhere else",
    which is a behaviour bug as well as a mismatch: check the `lea`'s
    displacement against the argument home before believing such a
    comment.

145. **Signed `% 4` compares.** `x % 4 == 0` is
    `cdq / xor / sub / and 3 / xor / sub / cmp eax,1 / sbb r,r / neg r`;
    `x % 4 == 1` puts a `dec eax` before the `cmp`. The extra `dec` is the
    quickest way to spot an off-by-one in a reconstructed alignment flag
    (it caught two in `CSub422F50::Sub_40FBB0`).

146. **Open: `xor r,r / setns r8` versus `mov r,1 / jns / xor r,r`.**
    hover.exe:0x405BC6 materialises four "which side of the line" booleans
    with `setns`; VC 2.2 emits the branch form as soon as a function needs
    four of them (two still give `setns`), and neither a named temporary,
    an inline helper nor a `?:` spelling changes that. Looks like 2.1
    -vs- 2.2 drift; it costs `CSub406090::Sub_405B90` about 8 instructions.

147. **VC 2.1 silently refuses to expand an inline that is one statement
    too long.** `CMerlinArchive::ReadWord(WORD*)` (guard, `p = m_lpBufCur`,
    `*pw = *(WORD*)p`, `m_lpBufCur = p + 2`) expands everywhere, but the
    value-returning `WORD ReadWord()` - the same body plus `w = ...` and
    `return w` - is emitted as a **call** plus a COMDAT in the linker map,
    with no diagnostic, while hover.exe expands it (0x42CE96 is the
    four-instruction expansion). It is a body-size limit, not a per-caller
    budget: the refusal does not depend on the call's position in the
    caller, on the callee's name, on `/Zi`, or on `#pragma function(...)`,
    and a small probe TU expands the identical function happily. Dropping
    the `BYTE* p` local (`w = *(WORD*)m_lpBufCur; m_lpBufCur += 2;`) puts
    it back under the limit and reproduces hover.exe's stream exactly.
    Whenever a diff shows `call <our own inline helper>` against an
    expansion in the original, `grep` the helper's mangled name in
    build/hover.map first - if it is there, the body is too big.

148. **A sparse dispatch whose comparison chain is not ascending is not a
    `switch`.** Refines rule 102. VC 2.1 always sorts a switch's chain
    ascending (probed: cases 6/7/-1 give `6, 7, -1` for an unsigned
    selector and `-1, 6, 7` for a signed one, whatever the source order,
    with a `default:` or without) *and lays the case bodies out in that
    same sorted order*. hover.exe:0x40C620 compares `-1`, then `7`, then
    `6` and falls through into the case 6 body, i.e. chain and bodies are
    in opposite orders, which no switch can produce. The shape is a nested
    chain of inverted tests
    ```
    if (m != -1) {
        if (m != 7) { if (m == 6) { <body 6> } return 0; }
        else        { <body 7> }
    } else          { <body -1> }
    ```
    - the innermost `if (m == 6) {...} return 0;` is what puts the shared
    `return 0` tail between body 6 and body 7 and makes the last test a
    `jne` into it. Worth 69.13% -> 74.28% on `CFloorObj::Sub_42AD60`.
    (`CFloorObj::Sub_42AD40`, 0x40d730, has the mirror shape - chain 6, 7
    with the case 7 body falling through - but there both spellings give
    the same code, so its switch was left alone.)

149. **Two guards on the same member with a call between them are two
    independent `if`s.** `CFlag::Sub_42AD60` (0x414470) tests
    `m_dwUnkCC == 0` and then `m_dwUnkCC == 1` with an `IsKindOf` call in
    between, and hover.exe re-reads the member for the second test
    (`cmp [ecx+0xcc],0` ... `cmp [edi+0xcc],1`, rule 65). Written as
    `if (a) {...} else if (b) {...}` VC caches the member in `eax` and
    compares the register twice; written as
    `if (m == 0 && p->IsKindOf(A)) return 0; if (m == 1 && p->IsKindOf(B))
    return 0;` it re-reads it. Dropping the `CPlayer* pPlayer` local at the
    same time (casting the parameter at each use) restores the original's
    register assignment - `this` in edi, the other player loaded from the
    parameter home inside each arm. 46.15% -> 87.88%.

150. **SUPERSEDED BY RULE 321 - the missing ingredient was inline `__asm`.
    VC 2.1 coalesces expression temporaries into one stack slot, so the
    `CWall::Sub_408820` frame cannot be reproduced.** The rule 104/118
    accessor pattern *is* reproduced by our build at hover.exe:0x40EF30 -
    the 8-byte POINT temp, the CSE'd reloads of both halves, the extra
    one-dword temp for every use of the first member and the direct
    register use of the second all come out instruction for instruction -
    but the original gives every expansion its **own** 8-byte slot
    (-0x130/-0x138/-0x140/-0x148 for four `Cells(pMgr).x` uses) where VC
    packs them all into one, hence the 0x1B8-byte frame against our 0x44
    and a wrong displacement on nearly every instruction. Probed and
    rejected: 16 expansions instead of 4 (still one slot), block-scoped
    `POINT` locals, a 3-dword return struct, a reference parameter, a
    struct with a copy constructor (the whole thing is optimised away),
    `/O1`, `/Ox`, `/Oxs`, `/Ogitb1`, `/Oglt`. Only two things give
    distinct slots: `/Og-` (`#pragma optimize("g", off)`), which the
    function cannot be using because it keeps `nDX` in `esi` across the
    whole body, and a returned class **with a destructor**, which drags in
    EH state stores and destructor calls the original does not have. Also
    unexplained in that function: hover.exe:0x40F267 is an `int 3` guarded
    by a `jns` - the only `jns/int 3` in the whole binary. Related negative
    result from the same family: the stack slots VC gives the three
    out-parameter locals of `CSub40D320::Sub_40C9B0` follow neither their
    declaration order nor their argument order (both permutations were
    measured on 0x40c620, 0x40d4b0 and 0x41a3a0 and changed nothing), so
    the remaining `lea [esp+N]` diffs in those four functions are not
    reachable from the source.

151. **Why the pod constructors cannot be both inlined and out of line
    (closing rule 110).** hover.exe's out-of-line `CXxxPod::CXxxPod` is
    reached from exactly two places - `CBumperDoc::OnOpenDocument`
    (texture.cpp, 0x415450, the unoptimized one) and the seven
    `CBumperView::OnCmd3401x` handlers (bumpview.cpp, e.g. 0x428830) -
    while `CXxxPod::Sub_42AD40` in mazeobj.cpp expands it. Measured on
    CSlowPod:
    - defining the constructor in the class body gives the expansion
      (`CSlowPod::Sub_42AD40` 57.58% -> 80.40%) but our VC also inlines it
      in bumpview.cpp, so `OnCmd34016` falls 100% -> 31.82%, OnOpenDocument
      loses 4 points, **no out-of-line COMDAT is emitted at all** (grep the
      map: `??0CSlowPod@@QAE@XZ` is absent), and the constructor's marker
      goes unresolved - a net loss;
    - defining it `inline` in mazeobj.cpp only (so the other TUs see just
      the declaration, which is the linkage the original evidently had)
      gives `texture.obj/bumpview.obj : error LNK2001: unresolved external
      symbol ??0CSlowPod@@QAE@XZ`, because VC emits an inline function's
      body only when a call in that TU could not be expanded.
    The missing ingredient is a call site inside mazeobj.cpp where VC
    declines to expand it (the original's texture.cpp had one because its
    huge OnOpenDocument is compiled unoptimized). Note the reccmp bridge
    resolves markers by mangled name out of build/hover.map and only scans
    `src/*.cpp`, so the marker itself is not the obstacle - a marker in a
    `#if 0` block in the .cpp resolves fine once the symbol exists.

152. **A zeroed pair of adjacent members can be a `POINT` copied out of an
    8-byte local.** `CSub422C30::CSub422C30` (0x422c30) has a `sub esp,8`
    frame it never reads, a `push esi` / `lea esi,[ecx+0xc]` and two zero
    stores through `esi`. That is not a pointer local (which enregisters
    into eax and needs no frame) but
    ```
    POINT ptZero;
    ptZero.x = 0;
    ptZero.y = 0;
    *(POINT*)&m_pUnk0C = ptZero;
    ```
    - the local supplies the 8 bytes, the struct assignment supplies the
    `lea` and the callee-saved register. 41.03% -> 100.00%. The same
    constructor also showed that hover.exe's constructors initialise far
    fewer members than a "safe" reconstruction does: the original clears
    only eight of CSub422C30's eighteen fields, and the extra stores we had
    were pure invention (rule 99's counterpart - when the emitted stores
    stop, so does the source).

153. **`CDC::GetTextExtent` is the renderer's text measurement, not
    `::GetTextExtentPointA`.** MFC 3.1 defines it inline as
    `::GetTextExtentPoint(m_hAttribDC, ...)` into a local `CSize` returned
    by value, so each call site emits (a) the *attribute* DC handle
    (`m_dcUnk00B4` + 0xBC, not `GetSafeHdc()`'s +0xB8) and (b) an 8-byte
    return temporary of its own that is written by the API and immediately
    read back (rule 118). Declaring the destination `CSize` *at* the call
    (`CSize sizeA = dc.GetTextExtent(psz, strlen(psz));`) elides the copy
    so the named local and the return slot are the same 8 bytes; declaring
    it at the top of the function costs a second 8-byte slot per string.
    `CBumperView::Sub_40CCE0` (0x40cce0) 46.5% -> 64.7% and
    `Sub_428120` (0x428120) 43.1% -> 70.4%.

154. **`n % 2` on a signed int is six instructions, `abs(n) & 1` is
    three.** `cdq / xor eax,edx / sub eax,edx / and eax,1 / xor eax,edx /
    sub eax,edx` is VC 2.1's signed remainder-by-2; the dashboard bitmap
    loaders round their scan pitch up with `if (nPitch % 2 != 0)
    nPitch++;`, which is what `CSub422BD0::Sub_411A00` (0x411a00, 49.7% ->
    67.4%) and `CSub422C30::Sub_411BB0` (0x411bb0, 46.3% -> 55.3%) need.
    Both also store the pitch into its member *before* and *after* the
    round-up - the first store is dead but the original keeps it.

155. **Indexing the zone table directly and caching a pointer into it are
    different addressing modes, and hover.exe uses both.** `((ZONE_DESCRIPTOR*)
    g_adwUnk_4609A0)[i].field` scales the index into a register and reaches
    every field as `[reg + g_adwUnk_4609A0 + fieldoffset]` (absolute base,
    one instruction per field); `pZone = ... + i; pZone->field` forms the
    pointer once and uses `[reg + fieldoffset]`. `Sub_407430`, `Sub_410510`
    and `Sub_4184B0` do the latter, but `OnContextMenuMsg` (0x428d90) does
    the former for all twenty-eight fields it reads (+1.4 points); the
    subscript form only survives if the base stays a cast of the array
    itself, not a variable.

156. **The /Od worker thread reaches `theApp` through an inline accessor.**
    Rule 92's "a pointer materialised into a fresh slot per statement is an
    inline accessor's return value" has a concrete instance in
    `CBumperView::Sub_41ECE0` (0x41ece0): every *store* into the app object
    is preceded by `mov [ebp-N], OFFSET theApp` with a slot of its own per
    statement, while every *read* is a plain `cmp [theApp+N],0`. A
    file-local `inline CHoverApp* HoverWorkerApp() { return &theApp; }`
    used on the seven store sites reproduces all five temporaries
    (57.2% -> 60.8%) at the cost of ~2 points of register churn in
    Sub_407430.

157. **Match the emitted argument order before believing a call.** Four of
    the wrong-operand bugs in this package were invisible at the C level
    and obvious in the push stream: `pfnProgress(step, 10)` had its two
    arguments swapped in `OnInitialUpdate`, `SelectPalette(&pal, FALSE)`
    was passing TRUE in `Sub_4184B0`, `CSub422C30::Sub_40E9A0`'s
    source-bitmap test was inverted (`m_dwUnk44 < i+1`, not `i+1 <
    m_dwUnk44`), and `Sub_41ECE0` called `SetEvent`/`PostMessage` where the
    original calls `ResetEvent`/`SendMessage`. When a diff shows the same
    call with a different constant or a swapped `push` pair, that is a
    behavioural bug, not codegen drift.

158. **The fixed-point wrappers take their destination as the *first*
    parameter.** In `CRobotPlayer::Sub_409350`'s `/Od` code every
    expansion of `FixMul`/`FixDiv`/`FixMulW`/`FixAdd`/`FixSub` homes the
    second argument in the *higher* slot and reaches the first one with
    `lea ecx` (`mov [T2],b / mov [T1],a / mov eax,[T2] / push eax /
    lea ecx,[T1] / call`), i.e. the shape is
    `inline DWORD FixMul(DWORD a, DWORD b) { ((CFixed*)&a)->Sub_4038E0(b);
    return a; }`. Writing the wrapper with `this` as the *second*
    parameter swaps the two `mov`s and the `lea`. Two more members of the
    same family are `Half()` (`mov eax,[t] / cdq / sub eax,edx /
    sar eax,1 / mov [t],eax`, a two-level expansion inside a by-value
    wrapper) and `Zero()` (two `mov word ptr [t],0`), and a point is
    zeroed by an outer wrapper that calls `Zero()` on both halves - three
    `jmp $+5`s per point. Negation and `labs()` are done in the wrapper
    itself (`a = -a; return a;`), one expansion each.

159. **A standalone `/Od` comparison materializes its result through an
    unreachable third `return`.** hover.exe:0x40a069, 0x40a1ce, 0x40a2ca
    and 0x4097e0 all read
    ```
    cmp .. / jcc Lelse / mov [R],1 / jmp Lend / Lelse: mov [R],0 /
    Lend: jmp Ldead+5 / mov eax,[U] / mov [R],eax / jmp / cmp [R],0 / jcc
    ```
    which is exactly
    `inline int IsLt(DWORD a, DWORD b) { int r; if ((long)a < (long)b)
    return 1; else return 0; return r; }` - the two `return`s write the
    result slot and the trailing `return r` of an *uninitialized* local
    leaves the dead copy behind. A leaf that is branched on immediately
    (inside `&&`) must NOT have that shape: `return (long)a > (long)b;`
    is what produces the original's bare `cmp`/`jcc` there.
    Related: VC 2.1 canonicalizes `K > x` with a literal `K` into
    `cmp [x],K`, so the original's `mov eax, 0x1000000 / cmp eax,[x]`
    needs the constant to come out of an inline: `FixOfInt(0x100) >
    (long)x` with `inline long FixOfInt(short n) { return (long)n << 16; }`
    reproduces it (the same helper explains hover.exe:0x40a1d3's
    `movsx / shl 0x10` against a non-constant).

160. **`/Od` orders named locals by C2's identifier hash, and a locals
    struct is how you take the order back.** Declaration order, scope
    nesting and first-use order have no effect at all (rule 79), but the
    *names* do: `DWORD aa,bb,cc,dd,ee,ff,gg,hh;` comes out in the frame as
    gg,dd,aa,hh,ee,bb,ff,cc while `a,b,c,d,...` and `a0,a1,a2,...` come out
    in declaration order. The order of any subset of a name pool is stable
    (measured on a 1700-name pool split into chunks sharing 400 ruler
    names), so a pool can be probed once and names picked to realize a
    wanted order - but the readable way is to put *every* local of the
    function into one `struct` and spell its members out in **reverse**
    frame order: a struct is laid out in declaration order from its own
    base, and a function whose only local is that struct puts the base at
    the bottom of the named region, so member N lands at
    `ebp - sizeof(struct) + offsetof(N)`. Any second local competes with
    the struct for `ebp-4` (name-hash again), so it has to be folded into
    the struct or renamed until it sorts below it. Worth 40.71% -> 64.34%
    on `CRobotPlayer::Sub_409350` (0x409350) once the frame size was
    already right; combined with rules 49/116/123/124 the function went
    32.10% -> 66.47%. Compiler *temporaries* below the named region are
    laid out bottom-up in source order (the first statement's temporaries
    sit just above the `this` home, the last statement's just below the
    named locals), so once the named region is pinned the temporaries fall
    into place by themselves as long as the expansion sequence matches.

161. **At `/O2` a pointer temporary over an array element is a *stack*
    slot, not a register.** `CSub410F20::Sub_419960` (0x419960) writes the
    palette ramp with `m_pUnk6C[i] |= m_pUnk6C[i] << 8;` twice; our
    reconstruction spelled it `pdw = &m_pUnk6C[i]; *pdw |= *pdw << 8;`
    because the original reuses the element address it has just computed
    (`mov [ecx+esi],eax / add ecx,esi / mov eax,[ecx] ...`). VC 2.1 spills
    that pointer, which grew the frame from the original's 0x18 to 0x1c
    and moved all three widened-BYTE parameter slots; indexing the member
    array directly gives the original's frame back and the same
    `add ecx,esi` sequence falls out of the address arithmetic anyway
    (56.05% -> 72.05%, together with computing `m_dwUnk8C / 2` on the spot
    at both uses instead of caching it in an `nHalf` local). Corollary of
    rule 79 for optimized code: when a frame is a few dwords too big, look
    for a local the original kept in a register before looking for missing
    statements.

162. **Declaration order picks which of two equally-live values gets the
    callee-saved register.** `CSub410F20::Sub_410F20` (0x410f20) keeps its
    two flag bits in `esi` and `edx` across the whole dispatch; whichever
    of `dwB1`/`dwB2` is declared *second* lands in `esi`. Adding an
    unrelated local anywhere else in texload.cpp flipped the pair and cost
    the function eleven points; swapping the two declarations put it back
    (84.94% -> 96.19%). Cheapest thing to try whenever a diff is nothing
    but two registers exchanged.

163. **Rule 101's third expansion slot is a by-value *class* argument that
    came out of an inline returning by value.** The physics multiplies own
    three stack dwords per expansion - `_dwB` (low), `_pA`, and one the
    function never writes (high) - and blocks descend 12 bytes apart in
    source order. Neither an unreferenced macro temporary, a `volatile`
    one, a taken-and-dropped address, an `_asm` member function nor an
    inline taking `CFixed` by value allocates it: VC 2.1 coalesces a
    by-value class parameter with the caller's lvalue. What does allocate
    it is handing the factor over as the *return value* of another inline
    (`MulP(&a, MakeFx(g))`, both inline, `MakeFx` returning the class by
    value): the return temporary is address-taken memory (rule 118) so the
    slot is reserved, but the value is forwarded straight into the callee's
    parameter home, so nothing is ever stored in it. `HOVER_FIXMULP3` in
    gameobj.cpp is that shape
    (`Hover_FixMulP3(&(dwA), Hover_FixArg(dwB))`) and it restores
    `CPlayer::Sub_41AB10`'s 0x34 frame exactly (36.6% -> 58.5%) and
    `Sub_40ADE0`'s 0x7c with rule 124's merge temporary (52.8% -> 58.4%).
    Two caveats: the *topmost* expansion's dead slot is sometimes absent in
    the original (0x40a9f0) and sometimes present with one more above it
    (0x40ade0), and the 16-bit `HOVER_FIXMULWP` expansions have **no**
    third slot at all - their factor lives in a WORD slot down with the
    other words (hover.exe:0x41a7a0), so leave those as they are.

164. **Fixed-point arithmetic done *in memory* is a free inline taking the
    destination by pointer, with the caller making the copy.** The
    integrators' `mov [T],a / sub [T],b / mov eax,[T]` (hover.exe:0x40AB8D,
    0x40B2DC, 0x40AC4C) is not a plain `a - b`. Probed at /O2: an inline
    taking the left operand by value and doing `p = &a; p->v -= b` folds
    back into registers, and so does a member function on `this`; only
    `inline void SubP(CFx* pa, DWORD b) { pa->v -= b; }` called on a copy
    the *caller* made (`CFx t = a; SubP(&t, b); return t;`) keeps the
    subtraction in memory. `*(volatile DWORD*)&t -= b` on a plain local
    emits the identical three instructions and needs no new type, which
    matters because of rule 132.

165. **`TRY`/`CATCH` is not what hover.exe's guarded blocks compile from.**
    MFC 3.1's `TRY` constructs an `AFX_EXCEPTION_LINK` (a real `call`)
    before the `try`; `CSub40B800::Sub_417860` (0x417860) has no such call,
    only the EH state transitions, and its handler does its own
    `IsKindOf(RUNTIME_CLASS(CFileException))` test (0x41795D) with a bare
    rethrow (`push 0 / push 0 / call __CxxThrowException`, 0x4179EC) on the
    mismatch arm. Writing it out as `try { } catch (CException* e) { if
    (e->IsKindOf(...)) { ...; return 0; } throw; }` is worth 53.5% -> 65.3%;
    three more levers take it to 82.4% and restore the 0x12c frame exactly:
    - the `wsprintf` is *outside* the try (that is what moves the
      `mov [ebp-4],0` state store down past it);
    - the length local is scoped to the enclosing `if`, not the function -
      that is what puts it *below* the 0x104 name buffer (rule 86 does not
      apply to a function-scope scalar here, which lands at `ebp-0x14`);
    - the buffer is released with `delete (BYTE*)m_pUnk04`, whose two
      argument temporaries are the frame's last two dwords;
      `::operator delete(m_pUnk04)` passes the value straight through.

166. **Constructors call `CString::Empty()` on strings they just
    default-constructed.** `CMerlinStatic::CMerlinStatic` (0x419b30)
    default-constructs its six texture-name CStrings in the member-init
    list and then calls hover.exe:0x44B9DE (`Empty`) on all six after the
    scalar block (0x419C11). Adding the six calls took it 67.4% -> 80.8%,
    i.e. instruction-exact modulo the unresolvable MFC call operands. Worth
    checking on every `IMPLEMENT_SERIAL` class that owns CStrings; the
    other constructors in gameobj.cpp do *not* have them.

167. **A member group reached through one pointer.** `CPlayer::Sub_42AD70`
    (0x42bfc0) does `add ecx, 0x180` once and then addresses the five
    proximity values as `[ecx]`, `[ecx+4]`, `[ecx+8]`, `[ecx+0xc]`,
    `[ecx+0x10]`; writing them as `this->m_dwUnk0180` etc. emits five
    `[ecx+0x18N]` displacements instead. A `DWORD* pdw = &m_dwUnk0180;`
    walk makes it an effective 100% match (from 89.4%). The same shape is
    what `CSub40B800::Sub_40B800` wants for `playParams.pMixWave`: reading
    the member back after the store makes VC cache `&m_pUnk08` in esi,
    reading the *struct field* the original reads does not.

168. **hover.exe:0x415070 and 0x41BB80 are MFC's `Construct(void*)`
    thunks, not the constructors.** Both take `this` on the *stack*
    (`mov ecx,[ebp+8]`), null-test it, call the base constructor, and
    `ret 4`; neither has a single `call` site - they are stored in the
    `CRuntimeClass` at 0x4C5644/0x4C566C. That is MFC 3.1's
    `IMPLEMENT_SERIAL` helper `void PASCAL X::Construct(void* p) { new(p)
    X; }` with the constructor expanded into it, so those two classes have
    no out-of-line `??0` in hover.exe at all and our `// FUNCTION:` markers
    on `CMerlinLocation::CMerlinLocation` / `CMerlinBSP::CMerlinBSP` can
    never exceed ~55% (the missing stack-`this` prologue and `ret 4` are
    scored as diffs). Cf. rules 34 and 69 for the destructor equivalent.
    **Superseded by rule 221**: the ceiling is not real - move the
    constructor into the class declaration and put the marker on a
    hand-expanded `Construct`, and both reach 77-91%.

169. **A guard whose body is the whole function puts its `return f(...)`
    tail last.** `CPlayer::Sub_41A6B0`'s two segment cases are written
    `if (dwOnSeg == 0) return Sub_421B50(...); <body>` in our source, which
    lays the call in the fall-through; hover.exe:0x41A70F jumps *forward*
    over the entire body to the call at 0x41A80E, i.e. the source is
    `if (dwOnSeg != 0) { <body>; return 1; } return Sub_421B50(...);`
    (54.8% -> 61.6%). Complements rules 25 and 112.

170. **A helper that returns its output buffer is read through the return
    value.** Every `Hover_sub_4386D0` call site in hover.exe reads the
    angle as `movsx ecx, word ptr [eax+2]` off the *returned* pointer, not
    out of the buffer it was handed (`[esp+N]`). Chaining the call into the
    use (`((short*)Hover_sub_4386D0(&dwOut, dy, dx))[1]`) is worth +2.3 on
    `CSub40D320::Sub_42C820`, +1.9 on `Sub_421B50`, +1.4 on `Sub_41A6B0`
    and +0.6 on `Sub_42B8E0`. Cf. rule 39.

171. **`*pd = *(double*)(m_lpBufCur - 8)` is the wrong spelling for the
    archive's double reader.** It comes out high dword first and re-reads
    `m_lpBufCur` between the two stores (VC cannot prove `*pd` does not
    alias the member). hover.exe:0x41BF0B advances the cursor into a local,
    reads both halves off it and stores the **low** dword first
    (70.8% -> 75.0% on `CMerlinArchive::ReadDouble`). Rule 19's high-first
    order is about copying a double *member*, not about this.

172. **gameobj.cpp's `CMerlinStatic::Serialize` is a 1427-byte lottery
    ticket (extension of rules 45/87).** Any new type or inline function in
    that translation unit re-rolls it between 37% and 70% - and 0x42BE50,
    0x42BFC0, 0x4128A0 and 0x401260 with it - which is enough to swing the
    whole-binary accuracy by +-0.06 on its own. Measure the *summary*, not
    the function, after every such edit: a genuine +7 on one function was
    twice cancelled by that one. Where a construct can be expressed with a
    macro over existing types instead of a new inline (rule 124's volatile
    compound assignment, for instance), prefer the macro.

173. **A two-word member pair read into one 8-byte slot is a POINT passed
    by value; the giveaway is left-to-right argument evaluation.** VC 2.x
    pushes arguments right to left and evaluates them the same way, so
    `f((short)p->m_wUnk14, (short)p->m_wUnk16, n)` emits `push n / movsx
    ...16 / push / movsx ...14 / push`. Where the original instead widens
    +0x14 *first*, spills it, and only then loads the third argument
    (hover.exe:0x412306, 0x415D20, 0x415E7D, 0x415FC0, 0x417DD3), the pair
    is one 8-byte value built ahead of the pushes: give the function a
    `POINT` first parameter and read the pair through a POINT-returning
    inline accessor. `CPlayer::Sub_41B2D0` (0x41b2d0) goes 92.59% ->
    100.00% that way; `CCollider::Sub_40F9B0`'s two call sites drive
    `CBumperDoc::Sub_412250` 48.6% -> 61.9%.

174. **A POINT-returning inline costs one extra 8-byte temporary in our
    build.** VC 2.1 does not elide the copy from the inline's named local
    into the caller's argument temporary, so every `p->GetPoint()` call is
    12 frame bytes and 6 instructions where the original spends 4 and 0
    (`mov eax,[T1] / mov ecx,[T1+4] / mov [T2],eax / mov [T2+4],ecx`
    behind the inline's return `jmp`). Initialising the target at its
    declaration, returning `CPoint(x, y)` and the MSVC named-return-value
    syntax were all probed without effect. When the pair is copied
    verbatim rather than widened, skip the accessor entirely and write
    `pt = *(POINT*)&obj->m_dwUnk08;`: that gives the original's
    `add eax,<off> / mov ecx,[eax] / mov eax,[eax+4] / mov [pt],ecx /
    mov [pt+4],eax` with no extra slot (0x415450's four wall-ring
    endpoints). The offsets only fold into one `add` when the accessor's
    `this` is the outer object, not the embedded struct.

175. **`jcc` onto the zero arm of a materialised bool is the *negation* of
    the source test.** When a comparison has to survive a destructor call
    VC emits `jcc L0 / mov [T],1 / jmp L1 / L0: mov [T],0 / L1:` and then
    branches on `[T]`. `je L0` therefore means the source said `!= 0` and
    `jne L0` means `== 0`. hover.exe:0x416B36/0x416BC8/0x416D14 all use
    `je`, i.e. the loader's three maze-face name tests are "does *not*
    end in PAD_/DECAL_" - our reconstruction had all three inverted.

176. **`if (x = f(...)) break;` keeps the test after the temporary's
    destructor.** Written as two statements the CString temporary of
    `f(CString(sz))` is destroyed before the comparison; written as one
    the original's shape appears - the `== NULL` result is materialised
    into a frame temporary (rule 125), the destructor runs, and only then
    is the temporary tested (hover.exe:0x4161FD, 0x416604).

177. **A loop whose failure arm is `jmp` *past* the loop is
    `if (cond) {...} else break;`, not `if (!cond) continue;`.** In
    0x415450's animation-chain loops the `je` target holds a single
    `jmp` to the instruction after the loop, and the fall-through arm
    ends with a `jmp` to the loop's back edge; ours skipped to the next
    iteration instead, which is a behaviour difference, not just codegen.

178. **A strength-reduced byte offset in a frame slot means the source
    indexes an array; a cached element pointer means it holds a pointer.**
    `CMerlinTexture::Serialize` (0x412980) keeps only `nLevel * 0x20` in a
    slot (`mov [esp+0x20],0` ... `add [esp+0x20],0x20`) and re-forms
    `m_pUnk28 + ofs` at every use, so the source is
    `m_pUnk28[nLevel].m_dwUnkNN`, not a `pEntry` local (13.6% -> 32.9%).

179. **The unoptimised level loader's `try` opens at its first
    statement.** `mov dword ptr [ebp-4],0` is 0x415450's very first
    instruction after the `this` home, so the try block encloses the
    progress counters and the cursor change too, not just the archives.

180. **An *unused* inline function in a shared header is as disruptive as
    a used one** (extension of rules 45/87). Adding `inline int
    LvlIndex(int)` to hover.h and leaving `LVLPARAM` a macro moved 130
    instructions inside `CBumperDoc::OnOpenDocument` (C2 stopped
    duplicating the loop-bound tail into both arms of the wrap
    conditional) and 0.11 points of overall Accuracy. Using the inline
    instead of the macro reproduces the original's `jmp $+5` at every
    LVLPARAM join (the inline's return jump, 8 sites) but costs the tail
    duplication in the nine pod loops, which is a net loss; the macro
    stays. `LVLPARAM((int)g_dwUnk_46049C)` is still needed - without the
    cast the wrap divides unsigned (`sub edx,edx / div`) and the loops
    compare unsigned (`jb`).

181. **reccmp's percentage is unstable on a function with many
    near-identical loops.** `SequenceMatcherWithPins` gets no pins unless
    the source carries `// LINE:` annotations, and on 0x415450 (nine
    identical `Sub_412250(new C...Pod, rand() % 4 == 0)` loops, 1919
    instructions, most of them carrying a mismatched `ebp` displacement)
    its unpinned diff drops one whole loop - 294 original instructions
    with no `+` counterpart - and the reported figure swings by 8-13
    points on edits that provably improve the code. Measure such a
    function by aligning the two disassemblies with the `ebp`
    displacements normalised away and counting the *shape* mismatches as
    well as trusting the headline number.

182. **`f(CString(sz))` versus `CString s(sz); f(s);`.** The temporary form
    passes the constructor's return value straight on (`push eax`) and its
    destructor runs *before* the caller tests the result; the named form
    re-takes the address (`lea ecx,[ebp-N] / push ecx`) and keeps the
    object alive to the end of the block. `CBumperDoc::Sub_417D40`
    (0x417d40) uses the temporary: 52.94% -> 69.84% from that one line
    (and 90.62% once its Sub_40F9B0 point stopped going through a
    POINT-returning inline, rule 124).

183. **A function that hands its own argument block to a callee as a
    coordinate pair has to read *every* coordinate back through the
    pointer.** `CSub422F50::Sub_401980` (0x401980) passes `&lX1` and
    `&lX2` to the clipper `Sub_401520(long*, long*)`, which writes both
    halves of each endpoint back. VC only marks `lX1`/`lX2` as aliased -
    their address is the one that was taken - so it keeps `lY1`/`lY2` in
    registers across the calls and the caller then works with the
    *unclipped* y. The original reloads all four out of the argument
    slots after the calls (`mov eax,[ebx+0x18] / sub eax,[esp+0x18] /
    sub ecx,[esp+0x20]`, hover.exe:0x4019D8) and stores the flipped y
    values back into them, so spell the whole body through `long* plA =
    &lX1; long* plB = &lX2;`. This is a correctness rule, not only a
    match rule: with the registers cached the minimap line walked off the
    bottom of the 512x384 bitmap and faulted. The pointer form costs
    1.3 points on this function (37.50% -> 36.22%, `this` lands in ebp
    instead of ebx); `((long*)&lX1)[1]` only at the two reads is worse
    still (23.53%, `this` spills).

184. **Whole-tree audit of the `_asm`-through-pointer macros (rules 95 and
    140), and how to run it again.** Compile the file with the project's
    flags plus `/FAcs` (source-interleaved listing, e.g. `cl /nologo /MT
    /W3 /GX /O2 /Zi ... /FAcs /Fa<dir>\ /c src\gameobj.cpp`) and scan the
    `.cod` for every `mov DWORD PTR [ecx], eax` that closes an expansion.
    Trace `ecx` back through the `_pA`/`_pdwA`/`$T` temporary to the `lea`
    that produced it, which names the destination slot (`_dwQ1$`); the
    site is correct only when (a) some instruction *before* the block
    stores into that slot and (b) the first later use of the value reads
    that same slot. `NO-INIT-STORE` (the initialising store was deleted
    and the `imul`/`idiv` reads garbage) and `NEVER-READ-AFTER` (the
    pre-multiply value was forwarded past the block) are the two shapes of
    the miscompile. The `.cod` listing beats disassembling hover.exe for
    this because it names the slots and interleaves the source lines.
    Running it over all 109 expansions in bumpview/gameobj/hover/mazeobj/
    texload found four more live cases beyond rules 95 and 140:
    `Hover_sub_40B6E0` (`dwV1 = dwVY`, the along-vector lost its sine
    scaling, 31.45% -> 47.20%), `Hover_sub_41B7A0` (`dwA = dwY` *and*
    `dwB = dwX`, `*pdwOut` was just `dwX + dwY`, 48.00% -> 53.16%),
    `CSub423030::Sub_4140E0` (`dwQ1 = dwNum1`, both view scale factors at
    +0x44/+0x48 got the undivided numerator, 49.44% -> 68.89%) and
    `CPlayer::Sub_40AB70` (`dwVal = 0x240000`, 39.76% -> 46.15%).
    Destinations that are *memory* - a member reached through `lea
    ecx,[esi+N]`, a by-value parameter's home, `*pdw` - are all clean:
    VC 2.1 reloads them after the block.

185. **A constant initialiser needs a `volatile` destination, not a
    volatile read.** Rule 95's `dwSpin = *(volatile DWORD*)&dwTangent`
    works because it breaks VC's equality between two locals, but
    `dwVal = 0x240000` is *constant* propagation, which survives that:
    the fix is `volatile DWORD dwVal;` plus a cast at the macro call
    (`HOVER_FIXDIVPP((DWORD*)&dwVal, dwDiff)`), the rule 140 spelling.
    Routing the constant through a fresh named temporary
    (`dwSeed = 0x240000; dwVal = *(volatile DWORD*)&dwSeed;`) also fixes
    it and is closer to the original's instruction stream
    (hover.exe:0x40abfc really does home the constant in a slot of its
    own), but the *new identifier* perturbs C2's symbol hashing for the
    whole translation unit and costs points in five other gameobj.cpp
    functions (`CMerlinLine::Sub_401260` 72.97% -> 62.30%,
    `CPlayer::Sub_42BE50` 70.71% -> 63.57%); with the volatile
    declaration those functions stay put or improve. Adding a `volatile`
    qualifier or a cast is free; adding a name is not.

186. **How to actually pair our destructor against the original's `??_G`
    (closing rules 34 and 69).** `tools/vc2map.py` now falls back to
    `??_GCls@@` when a `~Cls` marker finds no `??1Cls@@` in the map, so a
    destructor whose original address carries the
    `test byte ptr [ebp+8],1 / operator delete / ret 4` tail is fixed by
    **moving its body into the class declaration**: with no out-of-line
    definition VC emits no `??1` at all and the marker resolves to the
    scalar deleting destructor. The marker itself has to stay in a
    `src/*.cpp` file with a parsable declaration under it, so leave the
    old definition behind wrapped in `#if 0 ... #endif` with the marker
    *inside* the block (the scanner skips blank and `//` lines after a
    marker but not a `#if`, so the `#if 0` must come before the marker).
    Measured, all eleven empty pod destructors 65.31% -> 92.86%
    (`CInviciblePod` 0x4146C0, `CSpeedPod` 0x41A510, `CMapEraserPod`
    0x41B350, `CSlowPod` 0x41B3E0, `CSmokeBombPod` 0x421290, `CJumpPod`
    0x4213C0, `CTempWallPod` 0x4215D0, `CSledPad` 0x4217B0, `CFlag`
    0x421DB0, `CHoldPad` 0x421F30, `CFlagRemover` 0x422B20), and with a
    body: `CContainer` 0x41B230 73.85% -> 94.44%, `CBumperDoc` 0x41E0D0
    81.60% -> 90.91%, `CMainFrame` 0x41DFD0 71.23% -> 87.50%,
    `CHoverApp` 0x426660 67.74% -> 86.96%, `CMerlinTexture` 0x418A60
    60.38% -> 83.33%, `CMerlinStatic` 0x419CF0 70.80% -> 81.67%,
    `CRobotPlayer` 0x422250 69.92% -> 80.00%, `CBeacon` 0x424180
    69.84% -> 80.00%. Two caveats:
    - the class must not need an out-of-line `??1` for anything else.
      If a *derived* class's destructor calls it (embedded members,
      stack objects) VC emits the `??1` anyway, the marker binds to that
      and nothing changes - that is rule 69's case, and the fix there is
      to re-point the marker at the original's real `??1`
      (`CSub423670` 0x42F180 -> **0x423670** 65.31% -> 90.48%, `CPlayer`
      0x42C080 -> **0x421A90** 74.23% -> 86.67%; both `??1`s were
      unmarked and are the `jmp` target of the embedding class's unwind
      funclets);
    - if the class has a pool `operator delete` the original inlines it
      into the same `??_G` (`CBeacon`, `CRobotPlayer`), so that has to
      move into the class declaration too, or the deleting arm shows a
      `call ??3Cls@@SAXPAX@Z` against four inline stores. Keep the
      `DWORD dwHead = g_head; g_head = (DWORD)p; ((C*)p)->m_link =
      dwHead;` spelling: it is the one `CFloorObj::operator delete`
      (0x42B7E0, an out-of-line 100% effective match) confirms, and the
      store order is source-visible even though the eax/ecx assignment
      is not.
    An inline destructor in a *widely included* header is a rule-45/180
    lottery of its own: `CHoverApp`'s (hover.h, included by every TU)
    measured -0.23 Accuracy the first time and +0.10 the second, with
    nothing but other agents' unrelated edits in between. Measure, and
    re-measure once before rejecting one.

187. **Delete `build/hover.pdb` as well as `build/*.obj` before any
    measurement.** `CFLAGS` has `/Fd$(INTDIR)\` and `LFLAGS` has
    `/pdb:$(INTDIR)\hover.pdb`, i.e. the compiler appends its type records
    to the *linker's* PDB from the previous build, so `rm -f build/*.obj`
    alone leaves the next build's type indices - and with them VC's
    register allocation, rule 45 - dependent on what was linked last.
    Two builds of identical sources that way produced 285,639 differing
    bytes and Accuracy 86.38% vs 86.57%; with `rm -f build/*.obj
    build/hover.pdb` the .text is byte-identical (only the 66 bytes of PE
    and debug timestamps move) and the figure is reproducible. Every
    before/after number in this file that was taken the old way is worth
    +-0.2.

188. **Our destructors store the vtable pointer two instructions too
    late, in every single one of them.** hover.exe's `??1`/`??_G` emit
    `mov dword ptr [ecx], <vftable>` between the
    `push offset __ehhandler` and the `push eax` of the EH frame setup;
    VC 2.1 emits it after `mov [ebp-0x10], ecx`, i.e. after the whole
    frame is up. It costs exactly two scored instructions in each of the
    ~30 marked destructors (the reason the folded ones plateau at 80-95%
    rather than 100%) and is not reachable from the source: it is the
    same in an empty destructor, one with a body, one with member
    destructors and one with an inlined `operator delete`. Related and
    equally unexplained: `CMerlinTexture::~CMerlinTexture` (0x418A60) has
    two EH state stores in the original (`[ebp-4] = 0` around the body,
    then `-1`) where ours sets `-1` once, even though the bodies are the
    same single `dec dword ptr [g_dwUnk_4C4DA0]` with no call in scope.

189. **A nested scope's locals go *below* the enclosing scope's, and a
    `CString` local reserves eight bytes of padding above it.** These are
    the two facts rule 160's locals-struct technique was missing, and
    together they close the "unexplained" /Od frame of
    `CHoverApp::InitInstance` (rule 88's and 131's open point (b)). VC 2.1
    allocates the named locals of the outermost block first (highest
    addresses), then each nested block's, in nesting order downwards, and
    hashes only *within* one block; so a group that has to sit low in the
    frame is put in an extra `{ ... }`, and a group that has to sit even
    lower goes one block deeper again. Independently, a `CString`
    (probed: any 4-byte class with a constructor) occupies twelve bytes,
    the object at the bottom and eight bytes of dead padding above it -
    that is the hole at [ebp-0x34]/[ebp-0x30] in hover.exe:0x41c9e0, not
    two lost locals. Applying both, `InitInstance`'s whole named region
    from [ebp-0x14] down to [ebp-0xbc] reproduces exactly: the five HDCs +
    nColors + bPaletteDevice in a function-scope struct, `CString
    strMessage` in the `try`, the startup group in a block below it, and
    the MIDI caps buffer with its `MMRESULT` in the `for` body below
    that. 48.18% -> 79.85%.

190. **hover.exe:0x420d33 is not a bug.** `Hover_sub_41F540`'s second
    waypoint row does `imul ebp,[esp+0x14]`, which looks like the first
    row's bottom edge, but SetRect's `bottom` argument has already been
    pushed four instructions earlier (`push ecx` at 0x420d2a), so esp is
    four lower and the slot is the same `i+1` at [esp+0x10] that the first
    row multiplies by. Always re-base `[esp+N]` across intervening pushes
    before calling a stack slot the wrong variable.

191. **A member call through a struct member pointer costs an extra /Od
    temporary.** `L.pSplash->Create(IDD)` emits `mov eax,[L.pSplash] / mov
    [temp],eax / ... / mov ecx,[temp]` where the same call on a plain
    local emits `mov ecx,[pSplash]` (probed both ways with the same type,
    the same class and inside the same `try`). So a pointer that hover.exe
    calls a method on cannot be folded into a rule-160 locals struct - in
    `InitInstance` the splash dialog pointer had to stay a plain local and
    the group around it be split into two structs whose names hash above
    and below it. Worth 7 points there, because the extra temporary also
    pushes the `this` home down a slot and mis-scores its ~30 reads.

192. **The /Od byte swap `xor eax,eax / mov ah / mov al`.** The Windows
    version check in `CHoverApp::InitInstance` builds `major << 8 | minor`
    from the low word of `GetVersion()`. VC recognizes the pattern only in
    the *shift-then-mask* spelling and its operand order decides which
    half is written first: `((v << 8) & 0xff00) | ((v >> 8) & 0xff)` gives
    the original's `mov ah` then `mov al`, while `((v & 0xff) << 8) |
    ((v & 0xff00) >> 8)` and all the `MAKEWORD`/`LOBYTE`/`HIBYTE`
    spellings give `shl/or` or the reverse order. Which order a given
    spelling produces also depends on whether the operand is a plain local
    or a struct member and on whether the destination is `int` or `DWORD`,
    so probe it in the shape the function actually uses.

193. **The operand of a relational compare that lands in the register is
    the one declared first.** `for (i = 0; i < n; i++)` emits `mov eax,i /
    cmp n,eax / jbe` when `i` is declared before `n` and `mov eax,n / cmp
    i,eax / jae` when `n` is declared first (probed with plain locals and
    with struct members; declaration order wins over address order, and
    writing the comparison the other way round changes nothing because VC
    canonicalizes it, rule 134). hover.exe:0x420cf8's MIDI-device loop is
    the second shape, so its `nMidiDevices` was declared before its `i` -
    which a rule-160 locals struct cannot express, since the struct's
    declaration order is also its frame order.

194. **Open: which of three `volatile` locals gets the lowest frame
    slot.** `Hover_sub_439E80` (0x439e80) has exactly three, all written in
    the prologue; hover.exe puts the return-code one in the middle slot
    ([esp+4] before the pushes) and the flag that is written last at
    [esp+0]. Ours puts the return code at [esp+0]. Neither declaration
    order, nor assignment order, nor renaming any of the three (probed
    with 28 names) moves it, and a reduced probe always puts the
    first-written volatile at [esp+0]. Costs ~10 instructions there.

195. **C2's /Od local-ordering hash has 16 buckets and can be measured, so
    any frame can be pinned by name.** Locals of one block are laid out
    from `ebp-0x14` downwards in *bucket* order; inside a bucket the
    **later** declaration takes the **higher** slot (a push-front list).
    The bucket of a single-char name is `c % 16`, of a two-char `cc` name
    `5*c % 16`; longer names are not worth modelling - measure instead.
    Probe recipe (`tools/probe/probe.sh <file>.c /Od`): declare the ruler
    `aa,bb,...,pp` (buckets 5,10,15,4,9,14,3,8,13,2,7,12,1,6,11,0) first,
    then the candidate names, assign each a distinct constant and read the
    `mov dword ptr [ebp-N],K` offsets; each candidate lands immediately
    *before* the ruler name of its bucket. With that, pick names whose
    buckets are non-decreasing in the original's frame order (ties are
    free between two plain locals - just declare them in reverse frame
    order) and the whole frame falls into place. `CBumperDoc::
    OnOpenDocument` (0x415450, 1919 instructions) went 48.40% -> 74.19%
    on nothing but this plus rules 196-198: twelve names for the try
    block (`sDoc(0) pLoc(0) sHead(2) fileMaze(6) sDir(7) sK(7)
    arTexIn(8) nLev(10) fileTexture(11) pEntry(11) sTail(12)
    arMaze(15)`) and eighteen for the `if (m_dwUnk8498 == 0)` block
    (`pTexEnt(1) nK(3) pFloorObj(6) nSeq(6) nGroup(7) pLoc1(7)
    pDrone(7) pLoc2(8) pDrone2(9) pFrst(9) pLast(9) j1(9) pAfter(9)
    pSide1(9) pSide2(10) posIt(10) pSide3(11) pSide4(12)`).

196. **`try { }` is a block, so the loader declares *everything* inside
    it.** Locals that live in the function body get a pool of their own
    that is allocated before the try block's, which pushes the try
    block's `CFile`/`CArchive` objects below all the scalars. hover.exe
    interleaves them (fileMaze at ebp-0x34 between `nWalls` and `by`,
    arTex at ebp-0x178 between `sI` and `nLevel`), i.e. they are in the
    *same* pool as the scalars, which is only possible if the function
    body declares no locals at all and the try block opens at the first
    statement (rule 179). Objects with constructors are ordered by the
    same hash as everything else; their *declaration* order is fixed by
    execution order, so on a tie the later-constructed object takes the
    higher slot - which is exactly the order 0x415450 needs.

197. **A struct member used as an inline's `this`, or inside a reference
    argument's address arithmetic, costs a frame temporary; a plain local
    does not** (the same phenomenon as the sibling rule about member calls
    through struct-member pointers). Grouping locals into a struct to pin
    the frame (rule 195) is therefore not free: `pEntry[sI].peRed`
    compiles the archive extractor's reference argument straight into the
    store (`movsx ecx,[sI] / mov edx,[pEntry] / mov [edx+ecx*4],al`,
    hover.exe:0x4156FC) but `sTail.pEntry[sDir.sI].peRed` first builds the
    address in a temporary (`shl / add / mov [T],eax`, four extra slots),
    and `sLoc.pTexture->GetPoint()` homes `this` in a temporary where
    `pTexture->GetPoint()` does not. Keep every local that feeds an
    inline's `this` or an indexed reference argument *out* of the struct
    and give it its own hash-picked name.

198. **A named `POINT` local where the original has an expression
    temporary is fixed with a file-local `inline` whose parameter is the
    POINT.** hover.exe:0x41670A/0x416732/0x4167E4/0x41680C read each end
    of a pad's wall ring as one 8-byte value into a slot *below* the named
    region, so they are temporaries, and rule 174's `pt = *(POINT*)&...`
    spelling puts them in named slots instead (32 bytes of frame in the
    wrong pool). `static inline void SetSegment(CSub40D320* p, POINT ptA,
    POINT ptB) { p->Sub_422BF0((DWORD*)&ptA, (DWORD*)&ptB); }` reproduces
    the original exactly - the by-value parameters of an expanded inline
    *are* caller temporaries, they are filled right-to-left like real
    arguments, and argument 1 gets the lower slot. The ring-closed test
    is `PtAt(&tail) != PtAt(&head)` with `inline CPoint PtAt(const DWORD*
    pdw) { return *(CPoint*)pdw; }`: MFC's `CPoint::operator!=(POINT)`
    then fills the object temporary first and its by-value argument
    second, which is the order hover.exe uses. Without `inline` on the
    helper VC 2.1 emits a real call at /O2-with-bailout.

199. **Open on 0x415450 after rules 195-198 (74.19%, 1855 of 1919
    instructions aligned once ebp displacements are normalised):**
    (a) hover.exe has 73 `jmp $+5` and we emit 34. The 39 extra ones sit
    at statement ends all over the function (two after the two progress-
    counter stores at 0x415498, one after every `new` statement, two
    around each `_mbscmp` argument); `#pragma optimize("", off)` changes
    nothing (re-confirmed), so they are inline-expansion return jumps of
    accessors we have not reconstructed, not a flag difference.
    (b) `new CCollider(0x5A00, 0x5A00, 0x100)`: hover.exe:0x415CAA stores
    0x5a00 into two adjacent temporaries, emits the inline's return jump
    and only then pushes them - rule 173's signature, i.e. the collider's
    first parameter is a `POINT`/`CSize` from a POINT-returning inline,
    not two ints.
    (c) the wall-name tests materialise each `_mbscmp(...) != 0` into a
    BOOL temporary and take an extra temporary for the `operator LPCTSTR`
    `this`, while the third term (`m_dwUnkBC`) is copied *raw* into its
    temporary - so the first two terms are calls to a BOOL-returning
    inline predicate over the `CString` and only the third is a plain
    member test.
    (d) hover.exe:0x41677E `cmp dword ptr [ebp-0x44c],0 / je` is
    unreachable: 0x416779 jumps over it into the ring-closing body, and
    the slot is never written. It is the third, compile-time-true operand
    of the ring-closed condition, dead like the `|= 8` arms.

200. **The renderer's width/height pair is an inline accessor whose result
    is used as a whole, not two member reads** (rule 118, second flavour).
    hover.exe's six dashboard element calls in the present pass
    (`CBumperView::Sub_407430`, 0x407430) read the zone descriptor's base
    size as `mov eax,[edi] / mov edx,[edi+4] / mov [T],eax / mov [T+4],edx
    / mov edx,[T] / mov ebx,[T+4] / push ebx / push edx`: one 8-byte stack
    temporary per site, *both* halves stored and both read back. Measured
    spellings of `inline HOVER_ZONESIZE Hover_ZoneSize(const DWORD* p)
    { return *(const HOVER_ZONESIZE*)p; }`:
    - `f(bits, Hover_ZoneSize(p).cx, Hover_ZoneSize(p).cy)` - two temps per
      site chained by a copy (`mov [T2],[T1]`), only `.cx` stored;
    - `HOVER_ZONESIZE s = Hover_ZoneSize(p); f(bits, s.cx, s.cy);` - one
      8-byte temp per site with only the `.cx` store surviving; this is the
      shape that took 0x407430 from 29.42% to 43.61%;
    - a by-value `HOVER_ZONESIZE` *parameter* (`f(bits, Hover_ZoneSize(p))`)
      is the only spelling that stores both halves, and it is right for
      `Sub_410510`'s calls (+3.7 there, +5 on `Sub_40E9A0`) but wrong for
      0x407430 (-12.6), so the two call sites of the same callee want
      different shapes and the 3-argument signature wins overall.
    A plain `*(HOVER_ZONESIZE*)p` by-value argument (no accessor) produces
    no temporary at all.

201. **hover.exe:0x407430's frame is four regions and is all-or-nothing.**
    From the bottom: the named aggregates (`CClientDC` at ebp-0x230, the
    `COPYDATASTRUCT` at ebp-0x21c), then the 8-byte-aligned temporaries
    (the `fild qword` conversion temp and eleven 8-byte slots, six of them
    live), then the 4-byte temporary pool (ebp-0x1b0 up to ebp-0x44, 92
    slots), then three unused dwords, then the named scalars ebp-0x34 up to
    ebp-0x14. The 4-byte pool is filled *first-allocated-lowest*, so every
    displacement in it is a function of the pool's total slot count: the
    ten blit expansions' argument homes (5 for each `StretchBlt`, 3-4 for
    each `BitBlt`), the ten `GetSafeHdc` result temps (allocated last, at
    the top of the pool), the fixed-point expansions' 14 slots and one home
    for the final overlay blit. Our reconstruction reaches 60-73 of the 92,
    which is why the function sits at ~42% however faithful the individual
    statements are; the remaining shortfall is concentrated in the three
    waypoint-marker sites, where VC 2.x doubles the destination rectangle
    in place at push time (`push ebx / add ebx,ebx`) instead of homing it.

202. **Block-scope locals are allocated in the temporary pool, not with the
    function-scope scalars, and declaration order runs to *descending*
    addresses.** Declaring the five homed arguments of a blit expansion in
    argument order and assigning them in reverse (lowest address first)
    reproduces rule 91's store order exactly - measured on the waypoint
    `StretchBlt` at hover.exe:0x407adc, where
    `DWORD x2, y2, cx2, cy2, cySrc; cySrc = cy; cy2 = cy*2; cx2 = cx*2;
    y2 = y*2; x2 = x*2;` gives the original's
    `mov [T],ecx / add ecx,ecx / mov [T+4],ecx / lea ecx,[eax*2] / ...`
    instruction for instruction. Only four of the five land in the pool
    (the pure copy `cySrc = cy` is coalesced into the named-scalar area),
    and a `volatile DWORD[4]` home array does reserve the `BitBlt` side's
    four slots but scatters them, so this is not yet a complete recipe.

203. **An unreferenced local is always dropped; the second member of an
    8-byte struct is not.** VC 2.2 deletes an unreferenced `DWORD`,
    `DWORD[1]`, `volatile DWORD`, `__int64` or empty struct declared inside
    a fixed-point macro (rule 101's missing third slot), but
    `struct { DWORD dwUnused; DWORD* pA; }` with only `pA` written reserves
    both dwords and emits no instruction of its own. It is not the
    original's shape in 0x407430 - an 8-byte struct is allocated in the
    8-byte-aligned region, while the original's third slot sits in the
    4-byte pool immediately below the macro's two live slots - but it is a
    zero-cost way to reserve stack space when rule 80 would otherwise be
    needed. Beware: merely *defining* the struct type in the translation
    unit moved `CBumperView::Sub_4184B0` by 4.5 points and 0x407430 by 11
    (rule 45 amplified by rule 181's aligner instability).

204. **`// GLOBAL:` markers (rule 57/59) cannot resolve a `static`
    (internal-linkage) global.** `tools/vc2map.py` matches a marker's
    symbol against the linker's PUBLICS table, and a file-scope `static`
    array or variable never appears there regardless of the marker being
    syntactically correct - `vc2map` reports "0 map candidates" and the
    address is left as an unresolved `<OFFSETn>` on our side forever
    (tried on `optionsdlg.cpp`'s `s_anSkillNames`/`s_anMazeNames`, hover.exe
    0x4bed50/0x4bed60 - correct addresses, verified with
    `tools/disasm.py`, no effect). The marker only works on externally
    linked globals (plain `DWORD g_x;`, `CFoo g_x(...)`, etc.); a
    `static`-qualified table referenced by index from another function is
    not fixable this way and the residual `<OFFSETn>` diff on it should be
    treated as unscoreable, like rule 20's MFC/CRT calls. (Rechecked for
    rule 240: the map's separate "Static symbols" section holds only the
    compiler's `_$$` labels in .text, never a data static, so there is
    nothing for vc2map to resolve. What rule 240 *does* recover is the
    statics that happen to sit inside a public symbol's span, which now
    print as `?g_x@@3...A+0x38 (OFFSET)` on both sides.)

205. **Declaration/initialization order picks the register for a pair of
    plain scalars, not just comparison operands (generalizes rule 193).**
    `CBumperView::Sub_4184B0`'s zone-search loop (`int nZoneInts, nIndex,
    *pSearch; nZoneInts = ...; pSearch = g_adwUnk_4609A0; nIndex = 0;`) put
    the array pointer in eax and the loop index in ecx; the original has
    them swapped (index in eax, pointer in ecx). Declaring *and*
    initializing `nIndex` before `pSearch` (order of the first two
    statements, matching declaration order) swaps the pair to match the
    original with no other change (58.06% -> 60.55%). Reordering only the
    assignment statements without reordering the declarations had no
    effect - both must move together.

206. **A three-term reassociated sum's walking pointer is anchored on the
    *last*-written term, not the entry's start.** `Hover_sub_413730`
    (0x413730) matches a 4-byte palette entry's R/G/B bytes against a
    target colour with `g_awUnk_4C4DB0[255+nB-p[2]] + [...-p[1]] +
    [...-p[0]]` (rule "VC reassociates the three-term sum" already
    documented the write order). The reconstruction advanced the pointer
    to the entry's start (`pPal += 0x28`) and read `p[0]/p[1]/p[2]`
    forward; the original instead advances it two bytes *past* the start,
    to the last (blue) byte the sum touches, and reaches green/red with
    *negative* offsets: `pPal += 0x2A;` then `p[0]` (blue), `p[-1]`
    (green), `p[-2]` (red). Both spellings read the identical three bytes
    each iteration, but only the "anchor on the last term, negative-index
    the rest" shape reproduces the original's `mov bl,[edx-2]` /
    `[edx-1]` / `[edx]` addressing (58.72% -> 60.55%). Whenever a
    reassociated sum indexes the same base pointer at three adjacent
    offsets, try anchoring the pointer on the highest offset and reaching
    the others negatively before assuming the diff is unfixable register
    churn.

207. **For a callee-saved register shared between the `this` pointer and
    one stack parameter, declaration position (not statement/usage order)
    picks which one lands in which of {esi, ebx}.** `CFloorObj::Sub_42AD60`
    (0x40c620) takes one explicit `DWORD dwUnk` parameter and casts it to
    `CPlayer* pPlayer`, used throughout alongside many bare `this->m_...`
    accesses. Declaring `pPlayer` first among the locals (the natural
    order, since it is assigned in the first statement) put `this` in esi
    and `pPlayer` in ebx - backwards from the original, which keeps `this`
    in ebx and `pPlayer` in esi. Reordering the *statements* that use them
    (moving a `this->member = 0` before the `pPlayer = ...` assignment)
    changed nothing. Moving the `CPlayer* pPlayer;` declaration to the
    *end* of the local-variable block, after every other local, flipped
    the assignment to match the original (73.31% -> 90.35%, +17 points).
    Cheap thing to try whenever a diff opens with a `this` vs
    stack-parameter register swap that persists across statement
    reordering (contrast rule 162, where swapping declaration order of two
    ordinary same-type locals was the lever - here it was declaration
    *position relative to the rest of the block*, with type/usage held
    fixed).

208. **Rule 204 extends from one shadowed value to several: parameters
    that survive a call need a local shadow, not just the disputed one.**
    `CCollider::Sub_411600` (0x411600) takes four DWORD coordinate
    parameters; three (`dwX1`, `dwX2`, `dwY2`) are read again after the
    `Hover_sub_408A00` call while the fourth (`dwY1`) is not. Reading the
    raw stack parameters throughout put all three of the surviving values
    in the wrong callee-saved registers (68.97%). Introducing a local
    `DWORD` shadow for each of the three survivors, assigned once at the
    top and used everywhere after (`dwY1` left as a bare parameter, since
    the original does not keep it live either), reproduced the original's
    register choice for the one that matters most to the byte count
    (68.97% -> 87.07%, +18 points). Declaration order among the three
    shadows made no measured difference here (three orderings tried, two
    identical at 87.07% and one worse at 80.17%) - unlike rule 204's
    single-value case, so treat introducing the shadow as the lever and
    only fall back to reordering it if the first attempt does not move
    the number.

209. **A missing `// GLOBAL:` marker on a plain-`DWORD`/array global is easy
    to miss and worth an explicit sweep (extension of rule 57).** Several
    hover.cpp globals that sit right next to correctly-marked neighbours had
    no marker at all - `g_dwUnk_4C56E4`/`g_dwUnk_4C56E8` (a critical-section
    flag pair), `g_dwUnk_4C56FC`/`g_dwUnk_4C5AD4` (WaveMixPlay's reentrancy
    counter), `g_adwSinTable`/`g_awTanTable` (the fixed-point trig tables)
    and `g_pUnk_4C4738`/`g_pUnk_46003C` (ExitInstance's two `delete`
    targets). Adding the marker is a pure comment and changed no codegen,
    but it let reccmp resolve the operand on both sides instead of scoring
    it as a diff: `Hover_sub_422E30` 72.37% -> 76.32%, `Hover_sub_439E80`
    63.83% -> 64.44%, `Hover_sub_438540` 77.89% -> 82.11%,
    `CHoverApp::ExitInstance` 84.09% -> 88.64%. A quick way to find the rest
    of a file's unmarked globals: scan for a `// hover.exe:0x...` trailing
    comment on a declaration line whose *preceding* line is not a
    `// GLOBAL:` marker.

210. **Operate on the output pointer directly; do not name the value it
    would hold.** `Hover_sub_43C420`/`Hover_sub_43C4C0` build an in-memory
    dialog template and round `*pcbTemplate` up to a DWORD boundary. Written
    with a local (`cb = wcslen(...)*2 + 0x18; *pcbTemplate = cb; if (cb & 3)
    ...`) VC computes `cb` in a register and stores it once; the original
    reads and writes `*pcbTemplate` itself at every step (`mov [edx],eax` /
    `and eax,3` / `mov [edx],ecx` / `mov eax,[edx]` - a real reload before
    the size is used again). Deleting the local and writing
    `*pcbTemplate = wcslen(pszTitle) * 2 + 0x18; if ((*pcbTemplate & 3) !=
    0) *pcbTemplate = *pcbTemplate - (*pcbTemplate & 3) + 4;` reproduces the
    read-modify-write exactly: 57.39% -> 73.04% and 81.43% -> 86.52%. A
    second local that is genuinely reused later (`cbOld`, the old size
    used to offset into the reallocated block) has to stay - only the
    single-use running total goes.

211. **CORRECTED by rule 226 - this was wrong.** Originally claimed
    `Hover_sub_42D290` passes `NULL` for `RegCreateKeyExA`'s
    `lpdwDisposition` out-parameter. `tools/disasm.py 0x42d290 0x30` shows
    hover.exe computing and pushing a real stack address for it
    (`lea eax,[esp+4]` / `lea ecx,[esp+4]`, both non-null, both pushed) -
    the original does keep `&dwDisposition` even though it never reads the
    value back. Restoring the real local (`DWORD dwDisposition; ...,
    &hKey, &dwDisposition)`) alone regressed to exactly the "before" number
    this rule cites (71.70%) because of an unrelated 4-byte frame gap; see
    rule 226 for the actual fix (block-scoping `buf`/`bLevel` so their
    stack space is reused) that gets both right at once (100.00%*). Left
    in place, uncorrected, so a future pass does not "fix" 42D290 back to
    the wrong, merely-higher-scoring `NULL` a second time - always check
    the raw disassembly for what an argument's address computation
    actually targets before accepting a scoring delta as proof of the
    original's behavior.

212. **The "which local gets which register" swap (rule 128) also fires
    between an early flag/counter and an unrelated value already live in a
    register, not just between two pointers.** Five instances, all fixed by
    reordering *unrelated* declarations against each other with no other
    change:
    - `CSub425050::Sub_438300`: the implicit `this` and the `bOk` flag were
      swapped between esi/edi; declaring `int bOk;` *before* the other
      locals (it was declared last) put `this` and `bOk` in the original's
      registers: 70.64% -> 90.83%.
    - `Hover_sub_42D6A0`: `BOOL bDone` and the cached `sndPlaySoundA`
      import pointer were swapped; declaring `bDone` *after* `pOnce`/
      `pLoop` (it was declared first) fixed it and also let the loop's
      exit test read `bDone` straight from its register instead of
      reloading `g_hUnk_4C56F4` first: 82.64% -> 84.30%.
    - `Hover_sub_439900`, `Hover_sub_439180` and `Hover_sub_439A80` all
      have the shape `dwPos_or_similar = f(...); i = 0; while (i < nCount)
      ...`; moving the loop counter `i` so it sits on the *other* side of
      the value it competes with (before `dwPos` in one function, after
      `nCount` in the other two - there is no single universal direction)
      matches the original's ecx/edi assignment: 91.82% -> 100.00%*,
      88.08% -> 89.40%, 73.61% -> 75.00%. There is no shortcut for which
      side is correct short of trying both and re-measuring; the common
      thread is that the fix is *always* a pure declaration reorder of two
      locals that do not otherwise interact.

213. **A member cached across a call can go the other way from rule 65: the
    original sometimes caches where a naive reconstruction re-reads.**
    `Hover_sub_438F50` reads `pEngine->dwUnk0C` for a null check, then
    passes it to `Hover_sub_438840`, then again to `waveOutReset`. Written
    as `pEngine->dwUnk0C` at each site, VC reloads the member after the
    call instead of keeping the callee-saved register alive across it, the
    opposite of what hover.exe does here. Assigning `hWaveOut =
    pEngine->dwUnk0C` once and using the local at every site keeps it live
    across the call and reproduces the original instruction for instruction:
    82.88% -> 100.00%. Rule 65 and this one are the same lever pointed in
    opposite directions - check the disassembly's actual reload pattern
    before assuming either default is right.

214. **`#pragma function(isalnum, isspace)` is a no-op (extension of rule
    100/107): only some CRT routines are real intrinsics.** Under `/D
    _MBCS`, VC 2.2 always expands `isalnum`/`isspace` at `/O2` as an
    `__isctype`/`__pctype` MBCS-aware test; `#pragma function` on them
    produces `warning C4163: '...' : not available as an intrinsic
    function` and changes no code (measured on `Hover_sub_43B880`, still
    51.20%). `strcmp` and `isdigit`-family routines are genuine intrinsics
    where the pragma works (rules 100/107); `isalnum`/`isspace` are not.
    Do not spend a build cycle on this pairing again.

215. **Zero-initializing output locals earlier only helps if it also fixes
    the register that holds them.** `Hover_sub_43B150` stores `hMMio`/
    `hResData`/`pData` to `NULL` right before their first real use (deep in
    the function); hover.exe's frame layout suggests one of them is
    pre-zeroed at the very top instead. Moving all three up did make the
    session-pointer register match the original (`edi`, previously `esi`)
    but produced a *double* store into the wrong slot and dropped the
    function from 74.34% to 70.19% - reverted. A one-line reorder is not
    enough here; the frame is also 4 bytes larger than the original's
    (`sub esp, 0x84` vs `0x80`) and that has to be found first.

216. **A local-function improvement that regresses the file-wide summary
    must still be reverted (rule 45's cost, restated as a warning).**
    Rewriting `Hover_sub_438D60`'s per-block field stores to go through
    `*pp` directly instead of a cached `pBlock` local matched the
    original's re-read pattern and took the function from 78.67% to 85.19%,
    but the register churn it caused elsewhere in hover.cpp cost more than
    that gain: whole-file Accuracy went 86.95% -> 86.92%. Reverted. Always
    take the full `tools/compare.py` summary before and after, not just the
    one function's line - this is the second time in this file a real
    per-statement fix has had to be thrown away for that reason (cf. rule
    172).

217. **A dispatch on an enum with a gap in its case values is not
    compiled as a `switch` at all when it is written as a sequence of
    independent, non-exclusive `if`s.** `Hover_sub_4262B0` (0x4262b0)
    reports the message for a `CArchiveException`'s `m_cause`; hover.exe
    tests the cause against 0, 2, 3, 4, 5, 6, 7 (there is no case for
    `CArchiveException::generic == 1`) as `test/cmp` followed by `jne
    <next test>` with the assignment inline and *no* jump to a shared
    end - it keeps testing every remaining value even after a match,
    which is harmless because only one can be true. A real `switch`
    (even one whose cases are exactly this set) or an `else if` chain
    both compile to a jump table or to comparisons that jump *forward*
    to bodies grouped after the last test - neither matches. Write it as
    `if (m_cause == K1) msg = M1; if (m_cause == K2) msg = M2; ...` with
    no `else` and no `break`. Took 0x4262b0 from 64.90% to 74.25%
    together with the same fix (folding `pszOsError = NULL;` into its
    declaration made no further difference).

218. **A struct array that is mostly zeroed per element is `memset` first,
    then the loop sets every field anyway (including the ones already
    zero).** `CMerlinWorld::Sub_418F00` (0x418f00) fills a 256-entry
    `PALETTEENTRY` array with `{0, 0, 0, PC_NOCOLLAPSE}` per element; a
    single `for` loop writing all four fields compiles to a pure byte-
    store loop (rule 115: plain `/O2` does not turn a per-element store
    loop into `rep stosd` by itself), but hover.exe has a `rep stosd`
    zeroing the *entire* array before the same four-field loop runs -
    i.e. the source is `memset(logpal.palPalEntry, 0,
    sizeof(logpal.palPalEntry)); for (i = 0; i < 0x100; i++) { .peRed=0;
    .peGreen=0; .peBlue=0; .peFlags=PC_NOCOLLAPSE; }`, with the loop's
    zero stores kept even though they are redundant after the `memset` -
    the original doesn't elide them either. Took 0x418f00 from 77.55% to
    a 100% effective match (reccmp still shows the harmless `ebx`/`edi`
    register-name swap around `SelectPalette`/`DeleteObject`). When a
    per-element loop's diff shows a leading `rep stosd` only on the
    original's side, look for a struct where most fields are constant
    zero and add the explicit `memset` rather than trying to make the
    loop itself vectorize.

219. **A three-line array swap's load order picks which side needs a
    fourth register.** `CSub424BD0::Sub_433CF0` (0x433cf0) shuffles two
    `DWORD` array elements with `dwTemp = a[i]; a[i] = a[j]; a[j] =
    dwTemp;` - both spellings compute the same swap, but this one makes
    VC 2.2 hold `a[i]`, `a[j]` and the two array base addresses live at
    once, spilling into `ebp` and forcing every exit path in the whole
    function (including an unrelated early `return;` branch far above
    the swap) to push/pop a callee-saved register it never otherwise
    needs. Writing it the other way, `dwTemp = a[j]; a[j] = a[i]; a[i]
    = dwTemp;`, loads `a[j]` first and fits in `ebx`/`ecx`/`edx`/`eax`
    with no fourth register - +7.9 points here, purely from deleting the
    `push ebp`/`pop ebp` pair that had bracketed the *entire* function.
    When a diff's only difference on an unrelated early-exit path is an
    extra saved register, look for a swap or multi-temp statement
    elsewhere in the same function whose load order can be flipped.

220. **A ternary computed inline as a call argument does not force a
    reload of an earlier argument that reads the same member; a ternary
    computed into a local first does.** `CMainFrame::OnCmd34010`
    (0x426c20) calls `::ShowWindow(m_wndStatusBar.m_hWnd, (cond) ?
    SW_HIDE : SW_SHOW)` where `cond` itself calls `::GetWindowLong(
    m_wndStatusBar.m_hWnd, ...)` - the same member read twice in one
    call's argument list. Written as one expression, VC 2.2 CSEs the
    member across the whole statement and reuses one register for both
    reads despite the intervening `GetWindowLong` call; split into `int
    nShow = (cond) ? SW_HIDE : SW_SHOW; ::ShowWindow(m_wndStatusBar.
    m_hWnd, nShow);` it re-reads the member for the second (`ShowWindow`)
    call, matching hover.exe exactly - 63.41% to 96.30%, the whole
    function bar one instruction. The lever is the *statement boundary*
    between the two reads, not the repeated member-access spelling
    (already identical in both versions).

221. **A "constructor" that takes `this` on the stack, null-tests it and
    `ret 4`s is `IMPLEMENT_DYNCREATE`/`IMPLEMENT_SERIAL`'s
    `Construct(void*)` thunk with an *inline* constructor expanded into
    it - the class declared its constructor in the header.** MFC 3.1's
    `AFX.H` expands both macros to `void PASCAL X::Construct(void* p) {
    new(p) X; }` (line 452-465). `PASCAL` puts `p` at `[ebp+8]` and cleans
    it with `ret 4`; MFC's placement `operator new(size_t, void*)` can
    return its argument unchanged, so VC 2.1 guards the *entire*
    construction - base-class call, member constructors, vtable store,
    member initialisers - with `mov ecx,[ebp+8] / test ecx,ecx / je`.
    When `X::X()` is out of line the thunk merely `call`s `??0X`; when it
    is defined inside the class declaration the compiler expands the whole
    body into the thunk and hover.exe contains no `??0X` at all.  Exactly
    four functions in hover.exe have this shape - 0x4128F0
    (CMerlinTexture), 0x415070 (CMerlinLocation), 0x41BB80 (CMerlinBSP)
    and 0x41D600 (CMainFrame) - while CMerlinWorld, CMerlinObject,
    CMerlinLine, CBumperDoc and CBumperView, whose constructors are out of
    line, show the ordinary `this`-in-ecx form.  Verified with a probe:
    a `CFrameWnd`-derived class with `DECLARE_DYNCREATE`, an inline
    constructor zeroing four members and a `CStatusBar` member compiles to
    hover.exe:0x41D600 byte for byte.  The fix is therefore two-sided:
    move the constructor body into the class declaration, and hand-expand
    the `IMPLEMENT_*` macro so the generated `Construct` can carry the
    `// FUNCTION:` marker -
        _IMPLEMENT_RUNTIMECLASS(X, Base, wSchema, X::Construct)
        CArchive& AFXAPI operator>>(CArchive& ar, X* &pOb)   // SERIAL only
            { pOb = (X*) ar.ReadObject(RUNTIME_CLASS(X)); return ar; }
    with
        // FUNCTION: HOVER 0xaddr
        void PASCAL X::Construct(void* p)
        {
            new(p) X;
        }
    left where the constructor used to be.  Worth +39.6 (0x4128F0),
    +36.1 (0x415070), +31.0 (0x41BB80) and +15.8 (0x41D600); what is left
    is only the original-side library calls and vtable that reccmp cannot
    name.  Note the marker body must be a normal multi-line `{ ... }` -
    reccmp's own source parser silently drops a marker whose function body
    is written on one line, and the function vanishes from the report.
    This supersedes rule 168, which had recorded the same shape at
    0x415070/0x41BB80 as an unreachable ~55% ceiling.

222. **Sibling branches that each compute into a differently-named local
    before copying it into a shared result variable block the shared
    jump-to-store hover.exe uses.** `CPlayer::Sub_40ADE0` (0x40ade0)
    picks one of four `HOVER_FIXMULP3` variants of the same
    `*(DWORD*)&m_awUnk01D0[4]` load depending on two flag checks, then
    stores the result once at the end. Written with a distinct local per
    branch (`dwThrust1`/`dwThrust2`/`dwThrust3`, each copied into
    `dwThrust` right before falling through to the common store), VC 2.2
    duplicates the store into every branch instead of sharing it, so the
    "no multiply" branch (`0128==0 && 012C==0`) can no longer jump clear
    over the sibling branches' multiply code the way hover.exe does at
    0x40AE3C (`jne`/`jmp` skipping straight to the shared
    `*(DWORD*)&m_awUnk01D0[2] = dwThrust;`). Deleting the extra locals and
    having every branch assign the *same* `dwThrust` directly restores the
    shared store and the far jump: 58.36% -> 63.47%, no regression to the
    file summary. When a value is selected from several mutually-exclusive
    branches only to be copied into one shared variable right after, skip
    the copy and use the shared variable as the computation target in
    every branch.

223. **gameobj.cpp's four Merlin `Serialize` overrides are capped by a
    body-size threshold inside `texture.h`'s `CMerlinArchive`, out of
    reach from this file.** `CMerlinStatic::Serialize` (0x419e00, rule
    172's "lottery" function), `CMerlinBSP::Serialize` (0x41bc40),
    `CMerlinLine::Serialize` (0x414740) and `CMerlinLocation::Serialize`
    (0x41b940) all diff the same way on their storing side: hover.exe
    interleaves each `WriteWord` call's buffer-cursor reload with the
    *next* field's source read (`mov word ptr [eax],bx` / `add eax,2` /
    `mov bx,word ptr [edi+N]` / ... / `mov [esi+0x1c],eax`, one giant
    chain with no calls), while our build periodically calls
    `CArchive::Flush` out of line and does not interleave the same way -
    and, worse, `WriteDouble` doesn't inline into
    `CMerlinBSP::Serialize` at all even though `texture.h`'s own comment
    says hover.exe inlines it there. Both are consequences of
    `CMerlinArchive`'s inline method bodies in `texture.h` (statement
    count, local-pointer-vs-direct-member-store shape), which this
    package's rules forbid touching (`texture.h` is a header, out of
    scope for a gameobj.cpp pass). Confirmed by grepping the diff for
    `Flush`/`<OFFSET` hits (a dozen-plus per function) with no other
    divergence underneath. Treat all four as a shared ceiling: nothing in
    gameobj.cpp itself can move them further without a `texture.h` change
    in a different package.

224. **Logged negative results for gameobj.cpp, to save a future pass the
    build cycle.** Each of the following matches an already-documented
    rule's *shape* but made the named function (and sometimes the whole
    file) worse when tried here - the lever is not universal, so verify
    with a full `tools/compare.py` summary before keeping any of them
    again on these functions:
    - `CPlayer::Sub_40B0B0` (0x40b0b0): giving the `if (m_awUnk01AC[4/6]
      == 0)` guards local `DWORD* pFricX/pFricY` pointers (rule 210's
      shape) dropped 48.45% -> 46.90% and the file summary with it - the
      original only holds the pointer live *inside* each zero-check
      block and re-addresses the member directly everywhere else, so
      extending the pointer's scope is the wrong direction here.
    - `CPlayer::Sub_40A9F0` (0x40a9f0): hoisting the shared
      `dwLimit = *pdwParams;` above the `if (m_wUnk01F0 < 0)` that reads
      it in both branches (both branches loaded the identical address,
      textbook shared-load-before-branch) dropped 44.90% -> 42.64%
      despite hover.exe's disassembly appearing to do exactly that load
      order - something else in the min/max clamp shape absorbs the
      difference.
    - `CSub40D320::Sub_40C9B0` (0x40c9b0): swapping the declaration/init
      order of `lX`/`lY` (rule 205's shape, chasing an edi/esi swap)
      dropped 54.48% -> 53.10%.
    - `CPlayer::Sub_408550` (0x408550): rule 167's member-group pointer
      (`DWORD* pdw = &m_dwUnk0180; pdw[0]=...; pdw[1]=...;`, matching
      hover.exe's `lea edx,[esi+0x180]` two-store shape exactly) tanked
      52.70% -> 34.21% and the file summary - the values being stored are
      sign-extended 16-bit halves of a local (`((short*)&dwFrontX)[1]`),
      not a plain member copy, and going through a pointer changes how
      VC 2.2 handles that truncation.
    - `CHumanPlayer::Sub_42AD30` (0x409180): removing the one-shot
      `DWORD dwC`/`dwC2` locals and comparing `pView->m_dwUnk006C`/
      `m_dwUnk0078` directly in the `(x < 1) ? 1 : -1` ternaries (to try
      to recover hover.exe's branchless `cmp/sbb/and/dec` idiom, which
      our build compiles as a real branch either way) dropped
      82.35% -> 80.65%.
    - `CMerlinWorld::Sub_413DB0` (0x413db0): shadowing the `dwUnk`
      parameter into a local `DWORD dwObj` used at every site (rule
      208's shape, chasing an esi/edi loop-index swap) left the function
      unchanged at 60.87% but dropped the file summary anyway - confirms
      rule 172's warning that this file's summary can move from edits
      that don't touch Serialize at all, not just new types/inlines.
    - `CGameObj::~CGameObj` (0x40d220)/`CSub40B800::~CSub40B800`
      (0x422cb0): swapping the declaration order of the free-list
      unlink loop's `pNode`/`pFree` locals had no effect on either
      function (this loop's register assignment is not declaration-order
      sensitive here, unlike rule 205's cases).
    - `CSub40B800::Sub_40B800` (0x40b800): shadowing
      `playParams.pSession` into a local `SOUNDENGINE* pSession` checked
      by the `if (... == NULL) return 0;` guard (rule 213's shape, to
      stop VC from re-reading the global `g_pUnk_4C56DC` for the check)
      left the function unchanged at 71.97% but dropped the file
      summary - another instance of rule 172's instability.

225. **A pointer assigned to an output field and then reused as a call
    argument must be read back from the local, not the field (extension
    of rule 213).** `CSub425050::Sub_437F00` (0x437f00) does `pNode->lpData
    = (LPSTR)pNode + 0x40;` then calls `ReadFile(h, pNode->lpData, ...)`.
    Re-reading `pNode->lpData` for the `ReadFile` argument makes VC 2.2
    reload the member from memory (`mov edx,[ebp]`); the original keeps the
    pointer live in a register from the assignment through to the call.
    Introducing `LPSTR pData = (LPSTR)pNode + 0x40; pNode->lpData = pData;
    ReadFile(h, pData, ...);` and using the local at the call site instead
    of the member reproduces the original's register reuse exactly. Same
    function also needed two rule-207-style declaration moves (an
    early-declared `int bLoaded` flag competing with `this` for the
    esi/edi pair, and a late-declared `HMODULE hMod` competing with the
    cached `GetProcAddress` pointer for ebx/ebp) before this pointer-reuse
    fix became visible: 80.07% -> 82.19% -> 88.89% -> 99.65% across the
    three fixes, applied in that order.

226. **Rule 212's flag-vs-counter register swap is a broadly reliable
    first probe whenever a loop mixes a body-scoped boolean with the loop
    index.** `CSub424130::Sub_40BEF0` (0x40bef0) walks a beacon's already-
    visited stack with `while (i < nDepth) { bNew = (*pScan != x); ...; if
    (bNew == 0) break; }` - `int i; int bNew;` (declared in that order,
    matching first-use order) puts the pair in the compiler's non-default
    ebp/ebx assignment. Swapping the *declaration* order only (`int bNew;
    int i;`, no statement changes) matched hover.exe's assignment and took
    the function from 58.93% to 73.21% in one edit - the single largest
    point gain found by simple declaration reordering in this wave's
    package, and further confirmation that this lever is worth trying
    before anything more invasive whenever a diff shows a flag and a
    counter swapped across an entire loop body.

227. **A no-op-looking local shadow can still be worth keeping if it only
    helps other functions in the same translation unit (rule 14's
    corollary, stated as policy rather than just a caveat).**
    `CPlayer::Sub_41B0E0` (0x41b0e0) shadows `pt.x`/`pt.y` into locals
    before an early call; the shadowed function's own bytes are
    byte-for-byte identical before and after (61.29% unchanged), but the
    register-allocation reshuffle it causes elsewhere in texload.cpp raised
    the whole-file Accuracy from 87.22% to 87.24% - `CSub424130::Sub_40BEF0`
    gained further (73.21% -> 75.00%), `CSub4237A0::Sub_42AD30` and
    `CSub410F20::Sub_419530` ticked up, while `Hover_sub_40E730` (in the
    same package) and `CRobotPlayer::Sub_409350` (outside it) both
    regressed. Kept per the file-wide-summary rule (rules 14/216): measure
    the whole `tools/compare.py` summary, not just the touched function's
    line, and a net-positive file-wide change is worth taking even when it
    costs an individual function elsewhere and helps nothing about the
    function actually being edited.

228. **A handful of near-identical trivial constructors/destructors in
    this file sit at an unscoreable ceiling made entirely of unresolved
    `vc2map` symbols (extension of rule 20/204).** The seven `CSub423670`-
    subclass one-line constructors (`CSub423710`, `CSub4237A0`,
    `CSub4238D0`, `CSub423960`, `CSub423B80`, `CSub4235E0`, and
    `CSub410F20::CSub410F20`, all 91-92% similar) and two destructors
    (`CSub410F20::~CSub410F20` at 0x420f60, `CRobotPlayer::~CRobotPlayer`
    at 0x422250) have byte-identical codegen to hover.exe wherever the
    diff shows resolvable operands; every remaining mismatch is a vtable-
    pointer store or an `operator new`/`operator delete` call that
    `vc2map` reports against an unresolved `<OFFSETn>` on the original
    side instead of a name, so reccmp scores it as a raw byte diff even
    though the instruction is the same `mov [ecx], vftable` or
    `call ??3@YAXPAX@Z` on both sides. `CRobotPlayer::~CRobotPlayer` also
    has one further-unfixable class: the compiler's own EH-frame/vtable-
    reset scheduling in the destructor prologue differs in instruction
    order from the original in a way no source change reaches (it is
    entirely compiler-generated scaffolding, not user code). Confirmed by
    inspection, no edit attempted; treat this whole cluster as done.

229. **A flat run of `DBL_TO_FIX`/`FIX_TO_DBL`-style conversions (`fld` /
    `fmul` / `call __ftol`, or the reverse) is a hard ceiling, not a lever
    to keep pulling.** `CSub4319A0::Sub_432660` (0x432660, 33 back-to-back
    `g_R*Params.m_adw[n] = DBL_TO_FIX(m_adblParams[n]);` statements) and
    `CSub430780::Sub_4315E0` (0x4315e0, the same shape with 30
    conversions) each show *every single* mismatched line as `call
    __ftol` on the original's side against an unresolved `call <OFFSETn>`
    on ours - never a register, operand, or instruction-shape difference.
    Every other instruction in both functions (the `fld`/`fmul` sequence,
    the stores, the field offsets) is byte-identical; only the CALL
    instruction's encoded displacement to the shared CRT `__ftol` thunk
    differs, because `tools/vc2map.py` cannot disambiguate which of many
    textually-identical calls to the same helper corresponds to which
    call site once there are more than a handful in one function (compare
    rule 204's "0 map candidates" for statics - same tool limitation,
    different trigger). No source-level change moves this: the two
    functions above were tried with reversed comparison operands,
    declaration reordering, and local-caching before this pattern was
    recognized, all with zero effect. When a function's *entire* diff is
    a repeating `call <OFFSETn>` / `call __ftol` (or `__ftoul`, `_ftol2`,
    etc.) pair with no other marked line, stop immediately - it is at
    the same kind of ceiling as rule 20's unscoreable MFC/CRT calls, just
    reached through repetition rather than a single opaque call.

230. **A same-named call two lines apart is not always a scheduling
    artifact - check the IAT slot before assuming it is.** Most `call
    dword ptr [->DLL::X (IMPORT)]` vs `<OFFSETn>` diffs are just vc2map
    failing to resolve the original's side (rule 20/204's ceiling), but
    `Hover_sub_410CE0` (0x410ce0) genuinely calls `GetPaletteEntries`
    *twice* with identical arguments (`0, 0x100, pEntries`) rather than
    reading once and calling `SetPaletteEntries` the second time as our
    reconstruction had it. Confirmed by reading the retail exe's raw
    import table directly (`objdump -x`), not just the recompiled diff:
    IAT slot 0x4ca6d4 is `GetPaletteEntries`, 0x4ca6e4 (16 bytes further)
    is the unrelated `SetPaletteEntries` - the two calls are one table
    entry apart and easy to transpose by eye. The fix is a real behaviour
    change (`SetPaletteEntries` can broadcast `WM_PALETTECHANGED`;
    `GetPaletteEntries` never does) even though the data written back
    happens to be identical, so it was kept per the wave brief despite
    costing whole-file Accuracy elsewhere (see rule 222) - when a same-
    class import appears twice with the same three arguments, always
    check the IAT slot address before trusting that it is "the same call,
    just unresolved."

231. **This translation unit's per-function register choice can be
    perturbed by an edit anywhere else in the file, with no shared code
    path between the two functions (extends rule 45/181's "aligner
    instability", already named against `Sub_407430` in rule 203).**
    Changing a single call name in `Hover_sub_410CE0` (`SetPaletteEntries`
    -> `GetPaletteEntries`, same argument count, same line count via an
    inline comment) flipped `CBumperView::Sub_407430`'s `this`-pointer
    register from esi to edi throughout, dropping it from 41.41% to
    24.68% with zero source changes to that function - confirmed reproducible
    both with the extra fix as a 3-line comment block and as a same-line
    trailing comment, so it is not a line-number artifact; it is VC 2.2's
    single-pass codegen state carrying some residue across function
    boundaries within one translation unit. Whole-file Accuracy dropped
    87.15% -> 87.13% as a result. Per the wave brief, a verified
    behavioural bug (rule 221) was kept anyway rather than reverted for
    the aggregate number - rule 216's "revert if the summary drops" is
    the right default for a pure codegen-matching change, but does not
    apply when the change also fixes real program behaviour. Whenever a
    one-line fix to function A regresses an unrelated function B by
    double digits, check the map for `Sub_407430`-style fragility before
    concluding the fix itself is wrong - B's fragility, not A's fix, is
    usually the cause.

232. **The `wsprintfA(szName, "%s%02d", "PODxx_"/"FLGxx_", 0)` name buffer
    used by every pod/flag constructor is `char szName[0x34]`, not
    `[0x30]`.** All ten copies of this idiom in mazeobj.cpp (one per
    `CPodObj`/`CContainer`/`CFlag`/`CXxxPod` constructor) were declared
    with a 48-byte buffer, four bytes short of what the frame actually
    needs; the format string itself only ever produces 8 characters
    ("POD06_00" and friends), so the extra 4 bytes are pure reserved
    padding the original carries but never writes into, exactly like
    rule 203's zero-cost frame filler. The shortfall costs a whole
    `sub esp, 0x38` vs `0x34` frame-size diff and shifts every
    `lea [ebp-N]` after it by 4. Fixed on the three copies that are not
    documented dead ends: `CPodObj::CPodObj` (0x413050) 82.19% ->
    84.93%, `CContainer::CContainer` (0x41b160) 81.82% -> 87.27%,
    `CFlag::CFlag` (0x421cd0) 84.48% -> 89.66%. The other seven copies
    live in the `CXxxPod::CXxxPod` constructors that rule 151 already
    caps at their current match for an unrelated linkage reason (the
    constructor cannot be both inlined and out-of-line); this buffer-size
    fix was not applied there since those functions are out of scope,
    but a future pass revisiting rule 151 should try it too.

233. **Rule 215's "one of them is pre-zeroed" was right about the fix, wrong
    about needing all of them.** `Hover_sub_43B150` zeroes `hMMio`,
    `hResData` and `pData` together, deep in the function, right before the
    `dwFlags` dispatch that opens the wave source. hover.exe's frame is 4
    bytes smaller and stores one zero at function entry, before even the
    session-handle validation - moving *all three* up (rule 215's attempt)
    matched that early store but also produced a duplicate store into the
    wrong slot and dropped the function (74.34% -> 70.19%, reverted).
    Moving only `hMMio = NULL;` to the top (immediately after the entry
    trace call, before the `pEngine` check) and leaving `hResData`/`pData`
    zeroed at their original deep site reproduced the early store and the
    4-byte-smaller frame exactly: 74.34% -> 83.00%. When several locals are
    zeroed together and only one needs to move, try each candidate
    individually before concluding the group has to move as a unit.

234. **Not every "obvious" defensive initializer is in the original -
    a `theApp` member can rely on the global's static zero-init instead.**
    `Hover_sub_4232E0` never writes `theApp.m_bJoystick = 0` before the
    `joyGetDevCapsA` probe; it only stores 1 (probe succeeded) or 0 (probe
    succeeded but `joyGetPos` then failed) inside the `if`. When the probe
    fails outright, hover.exe leaves the member exactly as it was, relying
    on the object's `.bss` zero-init since this init function only runs
    once. Deleting our defensive `theApp.m_bJoystick = 0;` line above the
    `if` matched the missing store exactly: 77.82% -> 86.44% together with
    a declaration-order fix (rule 205/208 family: swapping `dwType`'s
    initializer before `cbData`'s). Before adding a "just in case" default
    assignment for a member later read unconditionally, check whether the
    original actually writes it on every path or is quietly trusting the
    zero-initialized global.

235. **Splitting a nested call pair into two statements can retarget which
    one's import thunk gets cached in a register.** `Hover_sub_438D60`
    writes `pBlock = (DWORD*)GlobalLock(GlobalAlloc(GMEM_DDESHARE,
    0x2030));` as one expression; hover.exe caches `GlobalAlloc`'s IAT
    pointer in `esi` (called through the register) and calls `GlobalLock`
    directly through `call dword ptr [->GlobalLock]` (its result used only
    once). Our compiler picked the opposite pairing - `GlobalLock` cached,
    `GlobalAlloc` indirect - from the single nested expression. Introducing
    an explicit `HGLOBAL hMem = GlobalAlloc(...); pBlock =
    (DWORD*)GlobalLock(hMem);` (declaring `hMem` with the function's other
    locals, assigning it in the loop body) flipped the caching to match:
    73.93% -> 81.52%. This is independent of rule 216's later store-through-
    pointer rewrite in the same function, which is still a confirmed
    regression and stays reverted. When two chained WinAPI calls show a
    swapped import-cache assignment, try naming the inner call's result
    before assuming the pairing is fixed by the compiler.

236. **A ternary assigned straight to a member store can need the opposite
    of rule 220: compute it into a local first.** `Hover_sub_43C8F0` (the
    WaveMix config-apply function) has `g_pUnk_4B6248->dwUnk1B0 = (cond) ?
    (DWORD)FnA : (DWORD)FnB;`. Written as one statement, our compiler
    stores the chosen function pointer with a separate immediate `mov`
    inside each branch (two stores, one per arm); hover.exe instead loads
    the chosen address into a register in each arm and does one shared
    store after the branches merge. Introducing `DWORD dwProc = (cond) ?
    (DWORD)FnA : (DWORD)FnB; member = dwProc;` reproduced the merged single
    store exactly: 96.57% -> 100.00%*. Rule 220 pulled a ternary *out* of a
    call argument into a local to force a re-read; here the same move
    (ternary into a named local) instead collapses two stores into one -
    check the actual instruction shape before assuming which direction a
    given ternary needs to move.

237. **Register-allocation cascades (rule 4) cross function boundaries in
    the same translation unit, not just within one function - re-check
    *neighboring* functions after every accepted change, not only the
    target and the whole-file summary.** Two cases in hover.cpp's WaveMix
    block (0x43axxx-0x43cxxx): rewriting `Hover_sub_43C4C0`'s field stores
    as pointer arithmetic (never itself a win, 68.09% -> 68.09%) dropped
    the unrelated `Hover_sub_43AA70` from 90.91% to 61.66% - reverted
    outright (rule 216 applies even though the *whole-file* summary had
    nudged up, since the specific regression landed on another function in
    this same package). Separately, the accepted `Hover_sub_438D60` fix
    (rule 223) and the accepted `Hover_sub_43C8F0` fix (rule 224) each
    knock `Hover_sub_43ABC0` down a further step (77.97% -> 73.90% ->
    64.65%) as a pure side effect - no edit was ever made to 0x43abc0
    itself, and several declaration-reorder attempts on it after the fact
    did not recover it. Both fixes are kept because they are net wins
    across the group (algebraically: bytes gained at the target function(s)
    exceed bytes lost at the bystander), but the loss should be logged
    against the bystander's own line in the final report, not hidden inside
    a rising whole-file percentage. When done with a wave, diff the whole
    package's scores against the wave's starting snapshot, not just each
    function's own before/after, to catch this.

238. **Non-overlapping same-scope locals need an explicit block per local
    to get their stack slots reused (corrects rule 211).**
    `Hover_sub_42D290` really does pass `&dwDisposition` to
    `RegCreateKeyExA` (rule 211 was wrong - see its corrected text).
    Restoring that real local by itself regressed the function to exactly
    rule 211's "before" score because our reconstruction still had a
    4-byte-larger frame than hover.exe's: `BYTE buf[5]` and `BYTE bLevel`
    are used one after the other (never simultaneously) but were both
    declared loose in the function body, so the compiler gave them
    separate, non-overlapping stack slots instead of reusing `buf`'s space
    for `bLevel`. Wrapping each in its own `{ }` block - `{ BYTE buf[5];
    ...; RegSetValueExA(...); }` then `{ BYTE bLevel = ...; RegSetValueExA
    (...); }` - let the compiler see their lifetimes as disjoint and reuse
    the slot, closing the frame gap and reaching 100.00%* (71.70% with the
    real `dwDisposition` alone). When two same-type-ish locals are used in
    sequence and never together, and the frame is a few bytes larger than
    hover.exe's, try scoping each into its own block rather than leaving
    them flat in the function body.

239. **Rule 20's ceiling is gone: calls into the statically linked MFC/CRT
    now score.** `tools/libmatch.py` identifies the original's library
    functions by matching them against the same functions in our build
    (identical fingerprints, then caller-alignment for the ones the two
    library builds compiled differently) and writes
    `tools/orig_library.csv`, a reccmp data source that gives each original
    library address our mangled symbol; reccmp matches the two by symbol,
    so `call CWnd::Default` now prints the same on both sides instead of
    `call <OFFSET1>` against `call CWnd::Default (FUNCTION)`. 976 original
    addresses are named, covering 1241 of the 1251 library call sites our
    annotated code makes. Effect on the report: accuracy 87.32% -> 89.45%,
    functions at 100% 400 -> 473, 292 functions improved and none
    regressed, with the Implemented denominator unchanged (the rows are
    marked `library`, and `tools/compare.py` passes `--nolib`, so library
    functions stay out of the score - `tools/compare.py --lib` includes
    them). Concretely: the `{ Default(); }` handlers of rule 20
    (0x40e2b0, 0x417af0) are at 100%, 0x426b40 is at 100%, and both
    `call __ftol` marathons of rule 229 (0x432660, 0x4315e0) are at 100%.
    What is *not* fixed: a call whose target we get wrong still scores as a
    difference (that is the point), the ~10 remaining unnamed library
    addresses (mostly `__adj_fdiv*` FDIV-bug stubs), and references to
    unannotated *data* - `mov ecx, <OFFSET4>` against `mov ecx,
    ?g_strReadFileError@@3VCString@@A` is the same problem for globals and
    still needs a `// GLOBAL:` marker. When a diff is now dominated by
    `<OFFSETn>` lines, check whether they are data rather than calls before
    declaring a ceiling. Regenerate the CSV with `tools/libmatch.py --write`
    if the set of linked library functions ever changes (the file keys on
    symbol names, so it survives ordinary rebuilds); `tools/libmatch.py
    --show 0xADDR` explains one address.

240. **Rule 237's register-allocation cascade also runs *positive*, and a
    two-lever fix on one function can be the trigger.** `Hover_sub_4385F0`
    (the vector-to-angle helper) needed two changes together: (1) `long
    lAngle = 0x4000;` at declaration materializes the default value at
    function entry; hover.exe defers that store to immediately before its
    one use (right after the `nFlags` sign-fixup ifs, before `if (lX !=
    lY)`) - moving the initializer down to match cost nothing on its own.
    (2) the four-way `if (nFlags==1) return a; else if (==2) return b;
    else if (==3) return c; return d;` compiles with a separate
    pop-registers/ret epilogue duplicated in every arm; collapsing it to
    one shared-return shape (`DWORD dwResult; if/else if/else assigning
    dwResult; return dwResult;`) did not, by itself, merge the epilogues
    (the arms still each end in their own pop/ret - VC's tail-duplication
    heuristic isn't reached by this alone), but combined with (1) it took
    the function 47.57% -> 48.65% *and*, via the same register-allocation
    cascade rule 237 describes, lifted two unrelated functions elsewhere
    in hover.cpp with no edits of their own: `CHoverApp::Sub_4253D0`
    71.84% -> 75.29% and `CHoverApp::Sub_425600` 83.67% -> 97.96%. Whereas
    rule 237's two logged cases were the cascade costing bytes, this is
    the same lever paying out for free elsewhere - always diff the whole
    package's scores after a change, not just the target function's own
    line, to catch cascades in either direction.

241. **A rule-207/237 register-cascade fix can be silently erased by later,
    unrelated commits to the same file, and the declaration-order lever
    that fixed it once does not necessarily fix it again.**
    `CFloorObj::Sub_42AD60` (0x40c620) was taken from 73.31% to 90.35% by
    commit c6edf88 (declare `CPlayer* pPlayer` last, rule 207); that source
    change is still present verbatim (diffed byte-for-byte against the
    commit), but the function now measures 74.28% again with the exact
    `this`/`pPlayer` <-> `ebx`/`esi` swap rule 207 fixed, because roughly a
    dozen later commits elsewhere in mazeobj.cpp (gauge-pointer caching in
    `CFlagRemover`/`CFlag`, the `wsprintfA` buffer-size fix rippling through
    every pod ctor's frame, `pPlayer`/`pMgr`-last fixes in sibling
    functions, etc. - see `git log c6edf88..HEAD -- src/mazeobj.cpp`) each
    nudged the file-wide register-allocator state per rule 237. Retried
    three declaration orders (pPlayer first, pPlayer last matching the
    commit, pPlayer buried mid-block with the three `bIn*`/`dwOut*` groups
    swapped around it) against the *current* file state and all three
    still measure 74.28% - the lever this rule number used to name is
    dead for this function until whatever upstream commit tipped it back
    is identified and revisited (out of scope for a mazeobj.cpp-only
    pass). When a rulebook entry claims a specific percentage for a
    specific function, re-measure before trusting it - large mazeobj.cpp
    passes since have moved the goalposts for at least this one address.

242. **Rule 223's `CMerlinArchive` inline-body-size ceiling (texture.h) also
    caps the *reading* side, not just `WriteWord`'s interleaving.**
    `CMerlinDynamic::Serialize` (0x42cc40) calls `pAr->ReadWord(&m_wUnkNN)`
    five times in a row (0x54/0x56/0x58/0x5A/0x5C); hover.exe expands all
    five (plus the trailing byte and count-word reads) as one unbroken
    FillBuffer-guarded chain with no calls at all until the final
    comment-byte loop. Disassembling our own `build/hover.exe` at this
    function's link address (`objdump -d -M intel --start-address=... 
    build/hover.exe`, since `tools/disasm.py` only reads the *original*)
    shows our build inlines the first *four* `ReadWord(&m_wUnkNN)` calls
    and then emits a real `call CMerlinArchive::ReadWord(WORD*)` for the
    fifth (`m_wUnk5C`) - contradicting rule 147's claim that the
    pointer-taking overload "expands everywhere" and "does not depend on
    the call's position in the caller": here it very much does, once
    enough other inlining has already happened earlier in the same
    function (the five `WriteWord` calls on the storing side stay fully
    inlined throughout, by contrast - a smaller/differently-shaped body).
    The storing side also reproduces rule 223's reordering symptom
    verbatim: hover.exe defers each field's cursor store-back
    (`mov [esi+0x1c],eax` / `add eax,2`) to just *before* the next field's
    raw load, while our build performs the store-back immediately after
    the write, which does not net out to the same byte count over the
    whole chain (measured +3 bytes across the five-field run, matching the
    3-byte `jne 0xe9`-vs-`jne 0xec` divergence right at the branch entry).
    Both symptoms are properties of `CMerlinArchive::ReadWord`/`WriteWord`'s
    bodies in texture.h, out of reach from mazeobj.cpp under this pass's
    header restriction - same documented ceiling as rule 223, now
    confirmed on the reading side too. No change kept; 76.48% is this
    function's ceiling until texture.h is in scope.

243. **Rule 148's inverted-if shape (the sibling function's fix) applies
    even when neither case value is -1.** `CFloorObj::Sub_42AD40`
    (0x40d730) dispatches on the same `m_dwUnkB0` as `Sub_42AD60`
    (case 6 a long nested-if body, case 7 four statements, default a
    two-statement fallthrough) but has only two real cases, not three,
    and rule 148 had marked it a false alarm ("both spellings give the
    same code, so its switch was left alone") - true only for
    switch-vs-ascending-if-chain; neither reproduces hover.exe here.
    hover.exe tests `!= 6` first: the nested `== 7` test and its short
    body fall straight through with no jump (reached directly when
    `!= 6`), while case 6's long body sits in the `else` arm, jumped to.
    A `switch` or an if-chain tested in ascending order (6 then 7) both
    place case 6's body first/closer instead, which is backwards -
    confirmed by testing three source orderings (`switch` case 6-then-7,
    `switch` case 7-then-6, and a plain ascending if-chain) that all
    produced byte-identical output regardless of source order, meaning
    VC 2.1 is choosing layout by something other than declaration order
    for this shape. Rewriting as `if (m_dwUnkB0 != 6) { if (m_dwUnkB0 ==
    7) {...} } else {...}` - i.e. explicitly nesting case 7 inside the
    "not 6" arm rather than listing it as a sibling case - reproduced the
    fallthrough/jump split exactly: 81.30% -> 82.93%. A same-scope
    tweak (moving the `lZ = pOther->m_dwUnk01A4` assignment out of the
    `else` block's top and inlining it directly into the `if (m_dwUnk90 <
    ...)` condition instead, to try to match the original's operand
    load order in that inner compare) regressed this function to 76.42%
    and was reverted - the member-then-parameter load order inside that
    nested compare is a separate, still-open mismatch, not fixable by
    relocating the `lZ` assignment.

244. **Open: `CSub40D320::Sub_40C9B0(pdwPts, &dwOutC, &dwOutB, &dwOutA)`'s
    three by-reference out-params get a consistently swapped stack-slot
    pair in every caller tried so far, and no declaration-order lever
    moves it.** `CTempWallPod::Sub_42AD60` (0x41a3a0, 78.23%) and
    `CWall::Sub_42AD60` (0x40d4b0, 79.23%) both call this exact
    four-argument member function 3-4 times each; at every call site
    hover.exe computes `lea eax,[esp+0x14] / lea ecx,[esp+0x18]` (low
    slot first) while our build computes `lea eax,[esp+0x18] / lea
    ecx,[esp+0x14]` (high slot first) for the same two out-params, and a
    third `&dwOutA`-equivalent slot is off by a step too (`[esp+0x20]`
    original vs `[esp+0x14]` ours, reused). Tried on both functions:
    swapping the declaration order of `dwOutA`/`dwOutB` (adjacent, most
    likely candidates by rule 205's pattern) - zero effect on either
    function's score, unlike rule 205/208's single-value shadows. Since
    the swap is identical at every call site regardless of which
    arguments precede it in the expression (a plain call, a `||`-chained
    condition, and a nested `pPlayer->Sub_403880(adwPos)` result used as
    the first argument), the lever is probably in `Sub_40C9B0`'s own
    definition or in how many of the three out-params are actually read
    back afterward (only `dwOutC` seems consumed a level up in some
    callers), not in caller-side declaration order - not chased further
    under this pass's per-function budget. `CFloorObj::Sub_42AD60`
    (0x40c620, rule 240) has the identical triple and the identical
    swapped-lea pattern in its own diff, for whatever that convergent
    evidence is worth.

245. **A shared macro's declaration-order lever helps some call sites and
    hurts others - net effect across the whole file decides, not any one
    caller.** `HOVER_INLINE_SUB_42A9F0`'s `_lIndex`/`_lSegs` locals
    (mazeobj.cpp:87-109) are used identically at five call sites (the
    per-frame `Sub_42AD30` handlers of `CSpeedPod`, `CSlowPod` and three
    others). Swapping the two locals' declaration order took
    `CSpeedPod::Sub_42AD30` (0x414370) from 75.36% to 82.61% but dropped
    `CSlowPod::Sub_42AD30` (0x42c430) from 86.96% to 73.91% and the whole-
    file summary with it (89.60%/89.41% -> 89.55%/89.36%) - reverted.
    Since the macro is textually substituted at each site, "declaration
    order" here really means "the order two temporaries are introduced
    relative to whatever surrounds them at that particular call site",
    which is apparently NOT uniform across the five - a fix at the macro
    definition can't be tuned per caller. Left as documented: the lever
    exists but is a net loss macro-wide: do not retry without also
    checking all five sites' scores together.

246. **Inlining a pointer-yielding call as a call argument (rules 213/225's
    "assign inside the argument" shape) is not a universal lever - it only
    matches when the original's argument-evaluation order actually needs
    it.** Two texload.cpp call sites duplicate the exact shape rule 225
    fixed elsewhere in this file (`pdwPos = pObj->Sub_403880(buf); Foo(...,
    pdwPos[0], pdwPos[1], ...)` rewritten as `Foo(..., (pdwPos =
    pObj->Sub_403880(buf))[0], pdwPos[1], ...)`), but produced byte-for-byte
    *identical* object code before and after the edit at both
    `CSub423670::Sub_40BA10` (0x40ba10, two call sites) and
    `CSub423670::Sub_42ED00` (0x42ed00): the original schedules the
    Sub_403880 call late (right before the last two argument pushes)
    regardless of whether the source states it as a separate preceding
    statement or as an inline argument sub-expression, and our compiler
    does the same in both spellings. Probe this lever, but don't assume it
    always does something - verify with a diff, not just the summary,
    since a neutral result here is easy to mistake for "already tried and
    failed" on a genuinely different function.

247. **The two 0x70/0x688-pool `operator new` allocators (rule 113) share a
    constant-multiply codegen quirk that declaration reordering does not
    reach.** `CRobotPlayer::operator new` (0x42c320, `0x688 *
    g_dwUnk_4C54D4`) and `CMerlinDynamic::operator new` (0x42cbc0, `0x70 *
    g_dwUnk_4C5624`) both show the *same* extra `mov ecx, eax` before the
    strength-reduced multiply sequence (`lea`/`shl`/`sub` chain) that the
    original does not have - the original computes the whole product
    in-place in one register, ours keeps a redundant copy alive throughout.
    Reordering the surrounding locals (`dwCount` vs `ppLink` vs `i`) had no
    effect on either function. Likely the same class of VC 2.1-vs-2.2
    optimizer drift as rules 9/11 rather than a source-construct
    difference; not chased further.

248. **`CSub425050::Sub_437F00` (0x437f00, rule 225's function, previously
    brought to 99.65%) is down to 90.81% as of this wave's starting
    snapshot** - a cross-function register-allocation cascade bystander
    effect (rule 237, previously only documented with hover.cpp examples)
    from unrelated changes elsewhere in texload.cpp's translation unit.
    All three of rule 225's fixes (the `pData` local, `bLoaded`'s
    early declaration, `hMod`'s late declaration) are still present
    unmodified; the regression is not local to this function. Confirms
    rule 237 applies across files/waves, not just within one PR's changes -
    when a function that a numbered rule brought to a high score shows up
    lower in a later package snapshot, check whether the rule's fix is
    still present before assuming it needs new work.

249. **When rule 45/135's file-wide churn undoes a scope fix, the second
    lever is declaration *order* inside the already-narrowed scope, not
    further narrowing.** `CSub424720::OnInitDialog` (0x4247f0) held at
    99.15% (rule 135: `pView`/`rcView` moved into the `else` block) until
    an unrelated later fix elsewhere in optionsdlg.cpp (declaring `i`/`j`
    before the pointer locals in `CSub42D910::Sub_42DB60`) shifted the
    whole file's register allocation again and dropped it to 88.03%.
    Re-scoping further (splitting `x`/`cx`/`cy` into their own per-branch
    locals instead of one set shared by all three assignments) alone made
    no difference; swapping the *order* of the two already-block-scoped
    declarations (`RECT rcView; CBumperView* pView;` instead of `pView`
    first) on top of it was what restored the 100% match. So when a
    function that previously matched regresses from unrelated churn, try
    the swap on its existing scoped block before assuming the fix stopped
    working - the lever is still order (rule 75), it just now has to be
    re-applied inside the block rule 135 already carved out, and the
    build has to be re-checked after every later commit that touches the
    same translation unit.

250. **Rule 239's other half: the original's *data* is named too, so a
    `mov ecx, <OFFSETn>` against a named global is no longer a diff.**
    `tools/datamatch.py` writes `tools/orig_data.csv`, a second reccmp data
    source (`address,type,symbol,size`, `type=global`) that gives an original
    data address the mangled symbol of our data object; reccmp matches the
    two by symbol, and because a matched entity is a single row with a single
    name, both sides then print the same text. It finds the pairs the way
    libmatch finds function pairs: for every function pair libmatch already
    has (the `// FUNCTION:` markers plus the ~980 identified library
    functions), the two bodies are aligned, and in each pair of aligned
    identical instructions the i-th *relocated* operand on one side is the
    i-th on the other. Our operand is resolved against the linker map to a
    symbol plus a displacement; the same displacement off the original's
    operand is one vote for that symbol's base. A name is emitted only if its
    base votes agree, if the map stays 1:1 both ways, and if the two objects'
    bytes agree with relocated fields replaced by what they point at (a
    vtable whose slots hold two different functions is rejected). 267
    original addresses are named: 102 vtables, 51 `CRuntimeClass` objects and
    114 globals. Effect on the report: accuracy 89.56% -> 89.87%, progress
    89.37% -> 89.68%, functions at 100% 473 -> 481, 97 improved, 2 changed
    for the worse, `<OFFSETn>` lines in the whole diff 332 -> 54. The two
    that went down (`CMerlinStatic::~CMerlinStatic` 93.33% -> 88.33%,
    `CRobotPlayer::~CRobotPlayer` 80.00% -> 75.38%) did not get worse: the
    `mov [ecx], CFoo::`vftable'` store now matches, which anchors difflib
    there and stops it from hiding a genuinely different prologue
    order behind a cheaper alignment. **Validation:** 184 of the tool's
    proposals land on an address one of our hand-written `// GLOBAL:` markers
    already asserts, and not one of them contradicts its marker; the emitted
    `??_7CFlag@@6B@` (0x4bc9f0) has 18 slots of which 14 hold a function
    whose own marker gives exactly the original address in that slot, and the
    emitted `?classCFlag@CFlag@@2UCRuntimeClass@@A` (0x4c5418) has the same
    class name, object size (256) and schema (0xffff) on both sides. What is
    *not* fixed: a `static` that our source never exports still has no symbol
    on *our* side either, so rule 204's dead end stands for the ones that do
    not fall inside a public symbol's span (checked: the linker map's "Static
    symbols" section holds only compiler-generated `_$$` labels in .text, no
    data); and the ~15 remaining `<OFFSETn>` calls are libmatch's residue.
    Side effects worth knowing: the original's vtables are now matched
    entities, which makes `tools/compare.py --vtable` useful for the first
    time (102 vtables, 3 matching) - `tools/compare.py` filters them out of
    the *function* listing, since a vtable is not a function and never enters
    Implemented/Accuracy/Progress; and `tools/compare.py --data` went from
    189 to 354 variables, still 0 issues. `tools/vc2map.py` now reads each
    `??_7` vtable's real length out of our image instead of estimating it
    from the map spacing (score-neutral, but it silences 94 of the 95 spurious "Recomp
    vtable is larger than orig vtable" warnings and stops the estimate from
    masking the constants that follow a vtable). Regenerate with
    `tools/datamatch.py --write` when the set of globals changes;
    `tools/datamatch.py --show 0x4c5418` (or `--show '?classCFlag@...'`)
    explains one address.

251. **The vtable pointer a constructor stores decides an MFC container's
    class - libmatch's name does not.** `CObList` and `CPtrList`,
    `CObArray` and `CPtrArray` and `CDWordArray` are near-template
    twins: their constructors, `AddTail`, `Find`, `SetSize` and the rest
    compile to identical instructions. libmatch's fingerprint erases
    absolute data addresses, so the *only* thing that separates a twin
    pair - the `mov [this], <vtable>` immediate - is invisible to it,
    and its twin pass assigns the bucket by caller vote; it named
    hover.exe:0x4491E4 `CPtrList::CPtrList(int)` when it is in fact
    `CObList::CObList(int)`.
    Never settle a member's type from a libmatch name alone. The method
    that does settle it, in order:
    (a) `tools/disasm.py <ctor>` and find the `call` or the
        `mov [this+off], <vtable>` for the member;
    (b) if it is a call, disassemble the callee and read the vtable
        constant it stores into `[this]` (MFC constructors store
        `CObject`'s vtable first and then their own, so take the *last*
        one);
    (c) name that vtable. `tools/orig_data.csv` names the ones our build
        also links (`??_7CObArray@@6B@` = 0x4C2340; 0x4C3098 is
        `??_7CPtrArray@@6B@`, NOT `CDWordArray`'s - see rule 349). For one
        it does not name, read vtable slot 0: it is
        `GetRuntimeClass`, `mov eax, <CRuntimeClass>; ret`, and the
        CRuntimeClass's first field is the class-name string
        (0x4C2118[0] -> 0x45E232 -> 0x4C7180 -> "CObList";
        0x4C2610[0] -> 0x45E36B -> 0x4C7238 -> "CPtrList"). A serialisable
        class's CRuntimeClass also carries an `m_pfnConstruct` thunk which
        calls the very constructor in question - a second, independent
        confirmation (0x45E210 calls 0x4491E4 with `push 0xA`).
    Settled this way: `CCollider::m_listUnk28`, `CFloorObj::m_list94`,
    `CPlayer::m_obList00B4/00D0/00EC/0108`, `CBumperDoc::m_list838C/83A8`
    (and so `CSub41E1E0`'s base) are all `CObList`; only the seven
    file-scope `g_listUnk_4A4*` globals are real `CPtrList`s.
    `CCollider::m_arrUnk48` is a `CObArray`, not a `CDWordArray`. Scan
    the whole call graph in one go rather than one member at a time -
    a rel32 sweep of .text for `call <ctor>` lists every member of the
    class at once (that is how `CFloorObj::m_list94` turned up).
    Two knock-ons to expect: retyping a member reshuffles the file
    (rule 45/87), which cost the two pool `operator new` overloads 18
    points until their `ppLink` declaration was moved last; and a
    mislabelled twin in `tools/orig_library.csv` must be corrected by
    hand at the same time, or the corrected call sites score *worse*
    than the wrong ones.

252. **Rule 251's twin problem also affects a file-scope global, and this
    one isn't even a twin ambiguity - the call targets are already
    distinct in `tools/orig_library.csv`.** `g_dwArrUnk_4A47F0` (declared
    `CDWordArray` in gameobj.h, defined in gameobj.cpp) registers each
    live `CGameObj*` at construction and destruction
    (`CSub40B800::CSub40B800`, 0x415380, and the matching removal at
    hover.exe:0x412bb0-ish teardown code). hover.exe calls
    `CObArray::SetAtGrow` at the 0x4153d8 registration site; our build,
    typed `CDWordArray`, calls `CDWordArray::SetAtGrow` at the same
    source line - two different, already-named library entry points, not
    a libmatch fingerprint collision. `hover.cpp:6404`'s teardown loop
    (`delete (CObject*)g_dwArrUnk_4A47F0[i]`) is also more at home with a
    `CObArray` of `CObject*` than a `CDWordArray` of raw `DWORD`s holding
    reinterpreted pointers. Not fixed here: `g_dwArrUnk_4A47F0`'s `extern
    CDWordArray` declaration lives in gameobj.h, out of reach for a
    gameobj.cpp-only pass, and every read site (`GetAt`/`operator[]`
    returning `DWORD` vs `CObject*&`) in both gameobj.cpp and hover.cpp
    would need the retype too - a headers-pass job, flagged for one.
    DONE: taken in the header pass, see rule 348.

253. **Rule 188 (destructors store their own vtable pointer two
    instructions too late, unreachably) also holds for constructors'
    *base-class* vtable store.** `CGameObj::CGameObj` (0x40d150) and
    `CSub40B800::CSub40B800` (0x415380) both show hover.exe storing the
    inherited `CObject::vftable` between `push offset __ehhandler` and
    `push eax` of the EH-frame setup, while our build stores it after
    `mov [ebp-0x10],ecx`, i.e. once the whole frame is up - the same gap,
    same place in the prologue, just on the constructor side instead of
    the destructor side. Untried here whether it is equally
    unreachable-from-source (rule 188 confirmed that for the destructor
    case across ~30 functions with varying bodies); logged as the same
    shape rather than re-proving it, since chasing it on `CGameObj`
    alone would mean touching a function order-sensitive to rule 172's
    `CMerlinStatic::Serialize` reroll for an instruction the destructor
    evidence says is compiler-scheduled, not source-controlled.

254. **Negative result: rule 222's `CPlayer::Sub_40ADE0` fix (58.36% ->
    63.47% on the function) re-rolls `CMerlinStatic::Serialize` down
    (83.54% -> 76.58%) even though it only deletes three now-redundant
    locals - no new type, no new inline.** This means rule 172's reroll
    trigger is broader than "any new type or inline function": a plain
    dead-local removal in `CPlayer::Sub_40ADE0` did it too, while an
    unrelated statement-order swap in `CGameObj::CGameObj` (tried the
    same session, see below) left Serialize untouched at 83.54% - so the
    trigger is not "any edit in the file" either. Net effect of the
    Sub_40ADE0 fix on the whole-binary summary: Accuracy 90.32% ->
    90.31%, a regression, so it was reverted per the keep-only-
    non-regressions rule despite being individually correct and
    rule-222-documented; a future pass revisiting `CMerlinStatic::Serialize`
    itself (texture.h, rule 223) could re-open this one.
    Also logged negative: swapping `CGameObj::CGameObj`'s
    `g_dwUnk_4A45A0++;`/`m_dwUnk34 = GetTickCount();` statement order (to
    match hover.exe's apparent global-increment-before-member-store
    sequencing at 0x40d1dd/0x40d1eb) made the function itself worse
    (83.64% -> 81.82%) with Serialize unaffected - the hypothesis that
    the two statements were simply written in the other order in the
    original was wrong; something else drives that global's scheduling.

255. **A debug-toggle member already declared in the header can silently
    gate a whole block that our source made unconditional.** In
    `CBumperView::Sub_402250` (0x402250) the per-frame render step -
    both `Sub_406C00`/`Sub_406090` calls for the primary view, the
    theApp.m_dwUnk01D0-guarded mirror-view pair, and the
    `m_obj00E8.Sub_405A10(this, pDoc)` call - is unconditional in our
    source but the original wraps the entire thing in
    `if (m_dwUnk0540 == 0)`. `m_dwUnk0540` was already a named member
    (the "no rectangle" debug toggle other functions in this file
    already read/write, always 0 by default) - it was simply never
    checked here, so the block behaved correctly only by coincidence
    (the toggle defaults off). Two more real bugs sat in the same
    function: the mirror-view `Sub_406C00` call added a spurious
    `+ 0x100` to its third argument (`m_dwUnk00E0`) that the original's
    disassembly does not have (only the fourth argument, `m_dwUnk00DC`,
    gets `+0x100` before the `&0x1FF` mask - confirmed by tracing the
    push order at both call sites byte for byte), and the
    `InvalidateRect` branch was missing the original's `m_dwUnk0080 = 0`
    reset afterward, plus an outright extra `GdiFlush()` call before
    `GetTickCount()` that has no counterpart anywhere in the original
    function's disassembly. Lesson: when a function reads a named
    "debug toggle" or "flag" member anywhere else in the file, grep for
    every place the *original* disassembly tests it (not just where our
    source already does) before trusting an unconditional block - a
    missing guard produces byte-identical results in the common case
    and only shows up as a diff, never a crash. 57.03% -> 59.53% (only a
    modest gain since the missing guard is usually not taken in
    practice; the fix is correctness, not primarily a score lever).

256. **`theApp`'s pointer members get the same declaration-position pointer-
    local caching rule 155 documents for the zone table, and the derived-
    pointer computation's *position* inside a same-base field-write run is
    a real reordering, not scheduling noise.** `CBumperView::Sub_427BB0`
    (0x427bb0) re-read `theApp.m_pUnk03EC` from the global on every one of
    eleven `BITMAPINFOHEADER` field writes; the original loads it once into
    a register and reuses it for the whole run (`pBmi = theApp.m_pUnk03EC;`
    reproduces this). Separately, the color-table pointer
    (`pColor = (BYTE*)&pBmi->bmiColors[0]`, i.e. `pBmi + 0x28`) is computed
    by the original *between* writing `biHeight` and the remaining four
    fields (`biClrImportant`, `biSizeImage`, `biClrUsed`, `biBitCount`) -
    not after all eleven fields as our source had it. Moving the pointer
    computation to its correct position in the field-write sequence (not
    just introducing the pointer local) was necessary: the pointer-local
    fix alone reproduced the *shape* (one load, incremental offsets) but
    regressed the function from 72.61% to 56.16% because the two writes
    that come after `pColor` in the original still came before it in ours,
    putting every subsequent field at the wrong relative byte offset in
    the diff; moving `pColor`'s statement to match the original's field
    order restored 72.60%. 52.69% -> 72.60%.

257. **Post-#172, texload.cpp's declaration-order register-pairing lever
    (rules 205-212/226) is mostly dead, and a recurring
    `Sub_403880`-then-`Sub_40C2B0` call shape looks like a real ceiling,
    not an unfound lever.** Two independent findings from a from-the-top
    pass over the file's 31 worst-matched functions after #172's
    `CObList`/`CObArray`/`CSize` retyping (which rule 241 already showed
    can retroactively kill a working declaration-order fix elsewhere in
    a file):
    (a) Fourteen declaration-order swap attempts across ten functions
        (`CSub410F20::Sub_419530`/`Sub_419960`, `CSub4237A0::Sub_42AD30`,
        `CCollider::CCollider`/`Sub_40F9B0`, `CBeacon::Sub_40C0E0`,
        `CSub4235E0::Sub_42ECE0`, `Hover_sub_413730`/`_40E730`,
        `CSub423710::Sub_42AD30`, `CRobotPlayer::Sub_4087D0`/
        `CRobotPlayer::CRobotPlayer`) each targeting an observed
        flag/counter or pointer/value register swap exactly like rules
        207/226 describe elsewhere in the codebase - only one (moving
        `CSub410F20::Sub_419530`'s `nTargetH` computation earlier,
        67.20% -> 68.38%) produced a net improvement; the rest were
        either no-ops (the compiler's register choice did not depend on
        the swapped pair at all) or regressions, several severe
        (`CRobotPlayer::CRobotPlayer` 80.95% -> 66.03%,
        `CRobotPlayer::Sub_4087F0` 86.79% -> 71.70%,
        `Hover_sub_413730` 60.55% -> 47.71%). Before spending a probe on
        this lever elsewhere in texload.cpp, expect it to fail more
        often than it works now; a positive result is the exception.
    (b) Five functions - `CSub423670::Sub_40A790` (0x40a790),
        `CSub423670::Sub_42ED00` (0x42ed00), `CSub423A90::Sub_42AD30`
        (0x40bcd0), `CSub4238D0::Sub_42AD30` (0x4150f0) and
        `CSub423B80::Sub_42AD30` (0x42ef40) - share one call shape: a
        `pObj->Sub_403880(buf)` call whose `[0]`/`[1]` result feeds,
        together with two `pParent->m_dwUnk01A4`-derived bounds, into a
        `CCollider::Sub_40C2B0`/`CSub423670::Sub_42ED00` call a few
        lines later. In every one of the five, hover.exe schedules the
        loads and pushes for that call in a different register/order
        than our build, and the two extra 0x40bcd0/0x42ef40/0x4150f0
        cases add a further, identical divergence: hover.exe never
        allocates a third callee-saved register for this shape (just
        esi/edi) while ours spills to a third (ebx). Rule 246 already
        found the "inline the pointer-yielding call as a call argument"
        lever byte-identical on two of these five (`Sub_40BA10`,
        `Sub_42ED00`'s own callers); this entry extends that finding to
        the shape itself, independent of how the call is spelled in
        source. No source-level rewording tried here (argument order,
        local caching, statement splitting) changed the codegen for
        this call shape - treat it as a ceiling from this VC 2.1/2.2
        version's call-argument scheduler for this particular
        call-then-call pattern, on the same footing as rules 229/247's
        hard ceilings, rather than a lever nobody has found yet.

258. **Resolves the "0x426f20 dead frame slots" open question: the earlier
    probe tried the wrong shape.** `CBumperDoc::OnCmd34002` (0x426f20) is
    the same missing-frame-space problem rule 80 already solves elsewhere -
    the open question's probe had only tried an *unreferenced* `int[6]`,
    which `/O2` deletes regardless, not rule 80's `volatile` array with one
    store. Adding `volatile DWORD adwPad[6]; adwPad[0] = 0;` among the
    locals reproduces the original's six untouched dword slots at
    `[ebp-0x28]..[ebp-0x14]` and the resulting frame size: 78.41% -> 85.88%.
    When an open question describes a frame gap and cites only a plain/
    unreferenced-array probe as having failed, retry it as a `volatile`
    array with a single store before concluding the gap is unreachable.

259. **Corrects rule 100/107: hover.exe:0x43E87E is `_mbscmp`, not `strcmp`
    - a real behavioural bug, not just a naming nit.** `Hover_sub_42DC50`
    (0x42dc50) looks up a key display name in `g_aKeyNames` with
    `_stricmp(strKey, strName) == 0` (case-insensitive); hover.exe instead
    calls hover.exe:0x43E87E at that site. Rule 100 named this same address
    "the CRT copy of strcmp" from an earlier, pre-libmatch pass, but
    `tools/orig_library.csv` labels it `__mbscmp`, and `tools/libmatch.py
    --show 0x43e87e` confirms the fingerprint match is exact and MBCS-aware:
    its very first instruction is `cmp dword ptr [_mbctype], 0`, a check
    plain `strcmp` never has. `_mbscmp` is the case-*sensitive* MBCS
    compare (not `_mbsicmp`/`_stricmp`), so the lookup hover.exe actually
    performs is case-sensitive, matching `texture.cpp`'s three existing
    `_mbscmp((const unsigned char*)(const char*)a, (const unsigned
    char*)(const char*)b)` call sites verbatim. Switching
    `Hover_sub_42DC50` to the same spelling took it 88.64% -> 89.77%, a
    genuine behaviour fix, but the register-allocation cascade (rule 237)
    it triggers elsewhere in optionsdlg.cpp cost two out-of-package
    functions their 100% match (`CSub424720::OnInitDialog` 100% -> 88.03%,
    `CSub42D910::Sub_42DBE0` 100%* -> 64.58%) and dropped the whole-file
    Accuracy/Progress by 0.05 each - kept anyway per rule 231 (a verified
    behavioural fix is not subject to rule 216's "revert on any drop").
    When a diff's call target is a named CRT/MFC function that contradicts
    an older, pre-libmatch rule's identification of the same address,
    trust `tools/libmatch.py --show` and the address's own first
    instructions over the older rule.

260. **Declaration order *inside* a nested block decides which callee-saved
    register each block-scope local gets, and it is worth more than any
    other lever on `CSub403980::Sub_403A60` (0x403a60).** The span walker's
    merge loop keeps four values in callee-saved registers - the node
    (`esi` in hover.exe), the new-span cursor `pNew`, the new span's head
    `nNewX0` and the old span's head `nOldX0`. Our build hands the
    block-scope span heads their registers *in declaration order*:
    whichever of `nOldX0`/`nNewX0` is declared first in the `do { }` body
    ranks first. Declaring the old-span pair first (`int nOldX0; volatile
    int nOldX1; int nNewX0; volatile int nNewX1;`) puts `nOldX0` in `edi`
    like hover.exe and took 0x403a60 from 28.13% to 34.77% in one step; the
    reverse order (rule 123's original spelling) scores 28.13%, and the
    seven other orders measured all land between 27.9% and 31.7%. Block
    *depth* does not enter into it: wrapping the new-span pair in an inner
    block that spans the whole `do { }` body left the build byte-identical
    at 34.77%, so /O2 ranks block-scope locals by textual declaration order
    however deeply they are nested - unlike the /Od frame layout of rule
    195, where nested-block locals sit below the enclosing ones. When a
    large /O2 function is a register permutation away from the original,
    permute the *declaration order of the block-scope temporaries* before
    concluding the allocator is out of reach - it is a one-line change with
    a double-digit payoff.

261. **How to read a whole-function register permutation: separate the
    function-long live range from the per-block ones, because only the
    former is usually wrong.** `Sub_403A60` compiles the same merge body
    twice (once per drawer), and the two copies get *different* registers
    even in hover.exe. Laying both builds out side by side:

        hover.exe  block 1: edi=nOldX0  esi=pNode  ebx=nNewX0  ebp=pNew
        ours       block 1: edi=nOldX0  esi=nNewX0 ebx=pNode   ebp=pNew
        hover.exe  block 2: edi=pNew    esi=pNode  ebx=nOldX0  ebp=nNewX0
        ours       block 2: edi=pNew    esi=nOldX0 ebx=pNode   ebp=nNewX0

    `pNode` is the only value live across the whole function; it is
    allocated once, and each block then fills the three registers left over
    in the fixed preference order edi, esi, ebx, ebp using its own ranking
    - (nOldX0, nNewX0, pNew) for block 1, (pNew, nOldX0, nNewX0) for block
    2. Those two per-block rankings are *identical* in hover.exe and in our
    build. Every one of the ~300 mismatched instructions therefore hangs
    off a single fact: the function-long `pNode` takes `ebx` in our build
    and `esi` in hover.exe, and each block's leftovers shift one slot to
    compensate. Do this decomposition before you start permuting: it turns
    "the allocator is completely different" into one variable, and it tells
    you which variables it is pointless to move.

262. **`Sub_403A60`'s residual mismatch is that one `pNode` register and it
    is a hard ceiling (rule 77 family).** Measured negatives, none of which
    moved `pNode` out of `ebx`: `register SPANNODE* pNode` (no effect);
    moving the `pNode` declaration to any position other than first among
    the function-scope locals (33.00% flat, from 34.77%); hoisting `nNewX0`
    to function scope, whether before `pNode` (25.91%) or immediately after
    it (12.62%) - either way it becomes live across *both* merge blocks and
    drags spills through the whole function; hoisting `nOldX0` to function
    scope (29.90%); giving each half its own node variable via a
    `HOVER_SPANMERGE_NODE(NODE, EMIT)` parameter so the live range does not
    span both blocks (15.95%); and a block-scope alias `SPANNODE* pN =
    pNode;` used through the merge body (17.05%) - VC copy-propagates `pN`
    away entirely, yet its *declaration* still counts in the block-local
    ranking of rule 260 and flips `nOldX0`/`nNewX0`. Our allocator ranks
    both block-scope span heads above the function-long pointer no matter
    how the pointer is spelled; hover.exe ranks the pointer between them.
    Treat the remaining ~65% as this ceiling, not as an unexplored lever -
    and note rule 14: this function's allocation also moves when unrelated
    functions are added to texheap.cpp, so the ceiling is a property of the
    whole .obj, not of the function's text alone.

263. **The span step is loaded once per merge block, and the second load is
    *not* eliminated by our build even though hover.exe has only one.**
    hover.exe loads `g_dwUnk_4A453C` into `eax` at 0x403aa9 - after the
    scratch-buffer copy loop of the *first* block, not at function entry -
    and the second block (0x403d38..) never reloads it outside the
    post-call reloads of rule 125. Writing `dwStep = g_dwUnk_4A453C;`
    before the copy loop (where our code had it) ties up `eax` through the
    copy loop and costs it its second scratch register; moving the
    assignment into the block, between the copy loop and `nNew = 0;`, frees
    `eax` again (our copy loop then opens with hover.exe's own `mov ax,
    word ptr [edx]`) and was worth 27.49% -> 28.13%. VC 2.x does *not* CSE
    the second block's load away, so a macro used for both blocks emits one
    extra `mov eax,[g]` that hover.exe does not have - one instruction, and
    the only known way to drop it is to stop sharing the macro. Also in the
    same preamble: `pDst = pNew;` must precede `dwCount = pNode->m_dwUnk14;`
    to get hover.exe's `mov edi,ebp / mov ecx,[esi+0x14]` order.

264. **What `volatile` on a spilled temporary actually costs, measured on
    `Sub_403A60`.** Rule 123 forces `nOldX1`/`nNewX1` into hover.exe's
    frame slots with `volatile int`, and that is still the best spelling
    known - but it is not free, and knowing the bill stops you chasing the
    difference as if it were a bug:

    - hover.exe reads the slot 16 bits wide wherever the value only feeds a
      `WORD` store (`mov cx, word ptr [esp+0x10]` at 0x403b69, 0x403bfc,
      0x403c3d, 0x403e49, 0x403edc, 0x403f1d). A `volatile int` must be
      read in full, so our build emits `mov ecx, dword ptr [esp+N]` at all
      six. Reaching past the qualifier with `*(WORD*)&nOldX1` does produce
      the narrow load, but taking the address of the volatile disturbs the
      allocation far more than the six operands it fixes: 25.25%.
    - hover.exe keeps the value in the register it just stored from (`mov
      [esp+0x10],edx / cmp ebx,edx`); we must re-read (`cmp esi,[esp+0x20]`),
      and where *both* operands are volatile that costs a whole extra
      instruction (0x403b18, 0x403bcd, 0x403de8, 0x403ead).

    Both are the price of the spill, not a wrong operand. Also, two
    block-scope `volatile int` pairs (one per macro expansion) never share
    a frame slot, which is where our `sub esp,0x20` against hover.exe's
    `0x1c` comes from, and with it every `mov ecx,[esp+0x38]` argument
    fetch reading `[esp+0x3c]`.

265. **Negatives recorded on `Sub_403A60` (0x403a60) so nobody repeats
    them.** (a) The two span-end temporaries must stay `volatile` *and*
    block-scope, one pair per merge block: plain `int` scores 18.42%, only
    `nOldX1` volatile 25.03%, only `nNewX1` volatile 23.37%. (b) Sharing one
    pair between the two blocks - either at function scope or in a block
    that encloses both - gives hover.exe's exact frame *layout* (`sub
    esp,0x18`, `this` at `[esp+0x24]`, `nOldX1`/`nNewX1` at
    `[esp+0x10]`/`[esp+0x14]`, `pOld` at `[esp+0x18]`, `nNew`/`nOld` at
    `[esp+0x1c]`/`[esp+0x20]`) but loses `pNew`'s `ebp`, and scores 15.95%
    against 34.77%. So does replacing the pair with a local `struct { int
    nOldX1; int nNewX1; } t;` (23.78%, or 23.03% with `volatile` members):
    two block-scope *structs* do share slots where two block-scope
    `volatile int`s do not, so that spelling alone reproduces hover.exe's
    `sub esp,0x1c` - and it still loses `pNew` its `ebp`. The 4-byte frame
    surplus and `pNew`'s register are a package deal; take the register.
    (c) `WORD w0,w1;` temporaries to force hover.exe's load-load-store-
    store copy loop: 25.69%. (d) Spelling the index cursor's step as
    `pIdx = (WORD*)((BYTE*)pIdx + dwStep*2)` to force hover.exe's hoisted
    `lea r,[step*2]` + spilled stride in the three fill loops that have no
    spare register costs 0x403a60 34.77% -> 31.67%, but through rule 14
    churn it lifts `CSub402E00::Sub_406740` 37.74% -> 40.55% and
    `Hover_sub_406A80` 32.41% -> 36.68%, for a net summary move of
    Progress 90.23% -> 90.24%. Left out here because it is churn, not
    structure, and the next edit anywhere in texheap.cpp re-rolls it - but
    re-measure the trade if all three functions are ever in one package.
    Probing the fill loop in isolation (tools/probe) shows all three
    spellings of `pIdx += dwStep` hoist the stride, so the inline
    `lea ecx,[ecx+eax*2]` our build emits inside the loop is pure register
    pressure, not a wrong spelling. (e) Rewriting the span-run list walk as
    `for (pNode = m_pUnk0C; pNode != 0; pNode = pNode->m_pUnk28)` instead of
    `if (pNode != 0) { do { ... } while (pNode != 0); }` leaves 0x403a60 at
    34.77% to the instruction but costs 0.02 of Progress elsewhere in the
    file. (f) The closing buffer swap's source order is irrelevant: VC
    schedules the three loads as `0xc`, `0x14`, `8` whether the source
    reads `ppA/ppB/dwCount` or `ppA/dwCount/ppB`, so leave it in hover.exe's
    order (`m_pUnk0C`, `m_pUnk08`, `m_dwUnk14`).

266. **`Sub_403A60`'s behaviour is already exact - the whole mismatch is
    allocation.** A statement-by-statement audit of all 456 instructions
    against `tools/disasm.py 0x403a60` found no behavioural divergence.
    Both merge blocks agree on every field offset (`m_dwUnk14`, `m_dwUnk18`,
    `m_pUnk08`, `m_pUnk0C`, `m_pUnk10`, `m_dwUnk1C`, `m_dwUnk24`,
    `m_pUnk28`); on both drawer argument lists (`Sub_406740(m_pUnk10[nX],
    nX)` - hover.exe pushes `nX` then the index word - and
    `Sub_402B10(m_pUnk10[nX], nX, m_dwUnk24, m_dwUnk1C)`, pushed
    `[esi+0x1c]`, `[esi+0x24]`, `nX`, `m_pUnk10[nX]`); on the sense and
    signedness of all fourteen comparisons in the merge body, including the
    two that our build emits with the operands swapped (`cmp esi,edx / jl`
    for hover.exe's `cmp edx,ebx / jg`, which is the same test); on the
    two-level entry guard (`if (m_dwUnk18 > 0)` outside the `do`, `if
    (m_dwUnk14 <= nNew) break;` as the first statement inside it, both
    landing on the same 0x403c5f); on the six emit/fill loop forms and
    their post-call `g_dwUnk_4A453C` reloads (rule 125); on the shared
    `pOld += 2; nOld++` tail at 0x403c49; on the strength-reduced
    `[edx+edi+2]` re-read in the old-span drain loop; and on the closing
    buffer swap (read `m_pUnk0C`, `m_pUnk08`, `m_dwUnk14`; write
    `m_pUnk0C`, `m_pUnk08`, `m_dwUnk18`, then `m_dwUnk14 = 0`). Since this
    function drives the drawer table in `CSub410F20`, that is the useful
    result even though the score is 34.77%: any texture-mapping fault on
    screen is not coming from the span walker's operands.

267. **Where you *set* a long-lived pointer local decides whether VC hoists
    an unrelated `&member` into the register it would otherwise leave
    free.** `CBumperView::Sub_410510` (0x410510) configures seven
    `CSub422C80::Sub_411D60` gauges with `(pZone->m_adwGaugeParams[i],
    (DWORD)&m_brush04xx, (DWORD)&m_pen04D0, (DWORD)&m_brush04E0)`; the
    original recomputes `lea ecx,[ebx+0x4d0]` at every one of the seven
    sites (hover.exe:0x410613, 0x410637, ...), because at that point `ebx`
    is `this`, `ebp` the zone descriptor, `esi` the DIB pixel base and
    `edi` the zone's width/height pointer - no callee-saved register is
    left. Setting the width/height pointer at its *first use* (before the
    `Sub_411BB0` sprite banks, which is where the original's `lea
    edi,[ebp+8]` appears) leaves one register free over the gauge block and
    VC hoists `&m_pen04D0` into it as a common subexpression, costing three
    instructions per site; moving the same assignment up to just after the
    palette-remap call - so the two pointers are live over the whole block
    - reproduces all seven `lea`s exactly and takes the function
    71.48% -> 76.29%. It also settles the frame at the original's 0x40 and
    puts the `GetClientRect` rectangle at the same +0x40. Generalisation:
    when a run of identical calls shows the original recomputing an address
    the recompile hoists, do not look for a different spelling of the
    argument - look for the variable whose live range the original has
    covering that run, and start it earlier.

268. **A width/height pair that both halves of a function read is an
    address-taken by-value struct, and it is what pads a frame by 8
    bytes.** Rule 200's `HOVER_ZONESIZE Hover_ZoneSize(const DWORD*)`
    accessor has a second use besides the per-call-site temporary: when the
    pair is read once and consumed by two later calls, `HOVER_ZONESIZE zone
    = Hover_ZoneSize(pdwSize);` makes VC treat the return slot as memory
    and store *both* halves into the frame (`mov [esp+0x18],ecx / mov
    [esp+0x1c],eax`, hover.exe:0x4109DD/0x4109E5) instead of keeping the
    height in `esi`. That frees `edi` again to be the scratch register of
    the rectangle copies that follow, and the two slots are exactly the
    difference between a 0x38-byte and the original's 0x40-byte frame
    (`Sub_410510`, 61.04% -> 65.10%). Two plain `DWORD` scalars will not
    do it: VC enregisters one of them.

269. **Six call sites that share one 8-byte temporary want one variable
    reassigned, not six block-scoped copies.** `Sub_410510`'s tail uploads
    the dashboard elements with six `Sub_40E9A0`/`Sub_40FCE0` calls that
    all read the render target's size pair; every site spills only the
    `.cx` half and reloads it (`mov [esp+0x34],ecx` / `mov eax,[esp+0x34]`,
    hover.exe:0x410BC1/0x410BCB) into the *same* frame slot. Six separate
    `{ HOVER_ZONESIZE size = Hover_ZoneSize(...); f(...); }` blocks and one
    `{ HOVER_ZONESIZE size; size = ...; f(...); size = ...; f(...); }`
    score the same on the function, but the six-block form costs the file's
    Accuracy and Progress 0.01 each in register churn elsewhere in
    bumpview.cpp, so the single reassigned variable is the one to keep
    (65.10% -> 70.27%). The reassignment loses the store/reload pair - VC
    copy-propagates out of an assigned struct but not out of an
    initialised one - which is the remaining ~2% of that function.

270. **Only one of two rectangle base pointers fits when the same call's
    first argument is a division.** `Sub_410510`'s two `Sub_411BB0` sprite
    banks each read eight fields out of two `RECT`s over 127 bytes into the
    zone descriptor, and the original reaches both through bases of their
    own (`lea edx,[ebp+0x1e0]` then `lea ecx,[ebp+0x1b0]`) - but it creates
    them *lazily inside the argument evaluation*, after `LVLPARAM`'s
    `idiv` has finished with eax/edx. A declared local is initialised at
    the head of its block instead, so with both declared VC spills the
    second across the `idiv` (and the divisor 3 with it: `mov [esp+0x2c],3
    / idiv dword ptr [esp+0x2c]`) and reloads it once per field. Declaring
    only one and naming the other group's four fields off `pZone` is worth
    +1.2 points over either extreme. Corollary for rule 94: the pointer-
    local spelling only pays when nothing between the declaration and the
    call needs eax/edx.

271. **Negatives measured on hover.exe:0x410510 (do not retry).**
    - Hoisting `LVLPARAM((int)g_dwUnk_46049C).m_dwUnk30` into a `DWORD`
      local before the two `Sub_411BB0` calls so the `idiv` precedes the
      rectangle bases: -2.2 points (the table load moves off 0x41075C).
      Spelling the index conditional out by hand (`int nLvl = (i < 16) ? i
      : (i % 3 + 15)`) instead of the `LVLPARAM` macro: -9.8 points.
    - Reaching the render target's `m_dwUnk04` as `pdwTarget[-4]` off a
      `DWORD* pdwTarget = &m_pUnk00D0->m_dwWidth` does *not* reproduce the
      original's `add eax,0x14` / `[eax-0x10]` / `[eax]` / `[eax+4]` group:
      VC folds the -0x10 back onto the unadjusted base and emits `[ecx+4]`.
      Naming the three members is a line shorter.
    - The `esi`/`edi` roles are not steerable from the source; rule 273
      explains why. Neither declaration order, nor the order of the two
      assignments, nor moving either assignment changes it.
    - Rewriting the zone selection as `&((ZONE_DESCRIPTOR*)
      g_adwUnk_4609A0)[g_dwUnk_460998]` (rule 155's subscript form) or
      casting the index to `int`/`long` changes nothing in the function.

272. **A width/height pair that both arrive as arguments of one Win32 call
    are separate statements when the original evaluates *both* before it
    pushes the call's first constant.** `CBumperView::Sub_410510`'s
    `PatBlt` subtracts `bottom-top` into eax and `right-left` into ecx at
    hover.exe:0x4109A9/0x4109AD, and only then pushes `BLACKNESS`
    (0x4109B5). A right-to-left argument evaluation of
    `::PatBlt(hdc, rc.left, rc.top, rc.right - rc.left,
    rc.bottom - rc.top, BLACKNESS)` can never do that: the constant is the
    last argument, so it is pushed first and the two subtractions are
    interleaved with the pushes that follow. Writing
    `nHeight = rc.bottom - rc.top; nWidth = rc.right - rc.left;` as two
    statements ahead of the call reproduces the block instruction for
    instruction (77.76% from 76.42%, the whole `GetClientRect`/`PatBlt`
    region goes to zero diff). Height is the first statement - the original
    computes bottom-top into eax before right-left into ecx - and the two
    locals cost no frame, they stay in registers. Generalise: whenever a
    computed argument is evaluated *earlier* than the argument order
    allows, it was a statement of its own in the source.

273. **VC 2.x hands `esi` to the highest-ranked register candidate in the
    function and `edi` to the second; a candidate you cannot outrank you
    cannot move.** In `Sub_410510` hover.exe keeps the DIB pixel base in
    `esi` and the zone width/height base in `edi`; every recompile has them
    swapped, and that single swap is most of the function's remaining diff
    (it renames `push esi`/`push edi` at nine sites, `[edi]`/`[esi]` at
    thirteen, and it changes which register is free as the scratch of the
    two composite rectangle copies). Measured ladder, by editing only the
    number of references to the size base and rebuilding:
    - size pointer as a *variable* with all 14 references: rank 1, takes
      `esi`; pixel base takes `edi` (76%..78% builds).
    - size pointer with 13 references (the first sprite bank's height read
      off the descriptor): still rank 1, still `esi`.
    - size pointer with 12, 10, 8 or 2 references: not allocated at all,
      and the register it frees is taken by the gauge block's
      `lea reg,[ebx+0x4d0]`/`[ebx+0x4e0]` address CSE - which also outranks
      the pixel base, so the pixel base *still* gets `edi` (66%, 65%,
      66%, 61% builds).
    - no size pointer variable at all: VC forms the size base as its own
      CSE at exactly the original's `lea edi,[ebp+8]` position and reads
      the first sprite bank's height as `[ebp+0xc]` like the original -
      but the gauge CSE takes `esi` and the pixel base again gets `edi`
      (71.93%).
    The pixel base is therefore never rank 1: something always outranks it,
    and with only two callee-saved registers left (ebx is `this`, ebp the
    zone descriptor) it can only ever be second. Reproducing hover.exe
    would need the gauge block to stop offering an address CSE, which no
    spelling of `(DWORD)&m_pen04D0` achieves. Do not spend more time on it;
    take the ~5 points it costs as the floor of this function.

274. **The corollary of rule 273: keep the long-lived pointer local at
    rank 1 deliberately.** In `Sub_410510` the size pointer's live range
    over the seven `Sub_411D60` gauge calls is the only thing stopping VC
    from hoisting `&m_pen04D0` into the second callee-saved register (rule
    260), and it holds that rank only while it has 13 or 14 references.
    Spelling the *first* sprite bank's height as `pZone->m_dwUnk0C` instead
    of `pdwSize[1]` drops it to 13, which is still rank 1, and matches the
    original's one absolute read (`mov edx,[ebp+0xc]`, hover.exe:0x41071E,
    with `lea edi,[ebp+8]` born one reference later at 0x410721):
    77.76% -> 77.93%. Dropping a second reference costs 11 points.

275. **`ptr + DWORD_index` where sizeof is 824 costs one instruction that
    no spelling removes.** hover.exe expands the multiply as
    `lea ecx,[eax+eax*2] / lea ecx,[eax+ecx*4] / shl ecx,3 / sub ecx,eax /
    lea reg,[ecx*8+base]` - accumulating in ecx and keeping the index in
    eax. Every recompile accumulates in eax and therefore needs a
    `mov ecx,eax` first. The divergence is identical in
    `CBumperView::Sub_410510` (0x410542) and `CBumperView::Sub_410380`
    (0x4103E3), so it is the expression, not the function. Tried and
    unchanged: the `&base[i]` subscript form (rule 155), casting the index
    to `int`/`long`, hoisting the index into an `int` local (that one also
    moves a frame slot and costs 0.34 of `Sub_410510`). What *is* worth
    copying is the statement shape: hover.exe assigns the base
    unconditionally (`mov ebp,g_adwUnk_4609A0` at 0x410526, before the
    `cmp`) and guards only the `+=`, so write
    `pZone = (ZONE_DESCRIPTOR*)g_adwUnk_4609A0; if (m_dwUnk0544 == 0)
    pZone += g_dwUnk_460998;` rather than an if/else over two full
    assignments.

276. **Negatives measured on hover.exe:0x410510 in wave 8 (do not retry).**
    - Computing the DIB pixel base before `FreeLibrary` so its `lea` is
      emitted first: -0.56, and it does not change the esi/edi roles
      (rule 273).
    - Assigning the size pointer before the pixel base, or declaring it
      after the pixel base: no change at all.
    - Declaring a base local for *both* sprite rectangle groups: -4.9
      (rule 270's spill). Declaring one for the second group and naming the
      first group's fields off `pZone` scores exactly the same as the
      reverse - the aligner matches the four based reads either way - so
      keep the first-group form rule 270 describes.
    - Dropping the base local for `m_rcViewport` and naming the four fields
      off `pZone`: -1.44 (VC then reads three of the four absolutely).
    - Making the four `Sub_411A00` digit banks read the size through the
      by-value accessor (which *does* fix their load order to the
      original's `mov edx,[edi] / mov ecx,[edi+4]`, i.e. memory order
      rather than right-to-left): -3.5, because the extra 8-byte local
      moves the composite passes' rectangle slots and costs 40 lines
      there. Reusing the composites' own `HOVER_ZONESIZE` for them: -5.2.
      Two plain `DWORD` scalars instead: -3.5. The digit banks' load order
      is not worth a frame slot.
    - Reaching the six element uploads' objects through a `CSub422C30*` /
      `CSub422BD0*` local so that `this` is computed before the argument
      pushes: -0.34, and VC folds the local away, so it does not produce
      the original's `mov [esp+0x34],ecx` / `mov eax,[esp+0x34]` spill of
      the width (rule 269 is still the last word on that pair).
    - Six separate `{ HOVER_ZONESIZE size = ...; f(...); }` blocks for the
      uploads: -0.34 on the function while raising the file's Accuracy and
      Progress by 0.01/0.02. The file-level move is an artefact of the
      comparator re-matching elsewhere, not of this function - judge a
      package's own function by its own percentage.

277. **hover.exe:0x410510's behavioural audit was redone in wave 8 and is
    still clean.** Re-checked against the disassembly instruction by
    instruction: the resource id (`0` or `g_dwUnk_460998`, `+0x81` after
    the arms join, 0x41056A-0x410578), `Hover_sub_453352(hInst, id,
    RT_BITMAP)`, `LoadResource`/`LockResource` with an
    `AfxThrowMemoryException` on each NULL, `SizeofResource(hInst,
    hRsrc)`, the inline memcpy, `FreeLibrary`, the pixel base
    `pBits + biSize + 0x400`, `Hover_sub_410CE0(pBits, pDibBits,
    &g_paletteUnk_4A1D50)`, the seven gauge parameter indices
    (0,2,2,1,3,5,4 at +0x40..+0x54) and their brushes (0x4E8, 0x4F8, 0x4F0,
    0x4E8, 0x4E8, 0x4E8, 0x4E8 with pen 0x4D0 and brush 0x4E0), the four
    `Sub_411920` markers (+0x2B8, +0x2C8, +0x2C0, +0x220), the two
    `Sub_411CF0` (+0x228, +0x230), the seven `Sub_411D10` (+0x288, +0x2A8,
    +0x2A8, +0x298, +0x304, +0x324, +0x314), `Sub_40FEE0(&m_dcUnk00B4,
    this, theApp.m_pBumperDoc, +0x258 rect)`, `Sub_411D80(&m_dcUnk00B4,
    pBits, +0x278 rect)`, `m_obj00E8.m_pUnk64 = &m_pen04D8`
    (`mov [ebx+0x14c],ecx`), `GetClientRect`/`PatBlt(BLACKNESS)`, both
    15-argument composites, the `Sub_401AD0` order (02E0, 0370, 0340, 0310,
    03A0, 03D0, 0400), the six uploads and the tail
    (`operator delete`, `theApp.m_dwUnk03B0`, `Sub_428120`, `Sub_410380`,
    `SelectPalette(pOldPal, TRUE)`). No divergence. What is left of the
    diff is register allocation and instruction scheduling only.

278. **`CSub402E00::Sub_404030` (0x404030): the one-pixel step is a *local*,
    and that single local is what makes the frame `sub esp, 0xc` instead of
    `sub esp, 8`.** The original's frame holds `this` at `[esp+0x10]`, the
    step at `[esp+0x14]` and the span index `i` at `[esp+0x18]`, and every
    argument therefore sits at `[esp+0x20/0x24/0x28]`. With
    `g_dwUnk_4A453C` spelled inline at all nine use sites our build folds
    the load into the operand (`sub edx, dword ptr [g_dwUnk_4A453C]`),
    needs no third slot, and shifts `this`, `i` and all three arguments by
    four bytes - which also mis-matches the six sunk `return` tails'
    `add esp, 0xc`. Declaring `DWORD dwStep;` *after* `DWORD i;` and
    assigning it once at entry restores the frame, puts `this` and `i` in
    the original's slots and lines up every argument reference. hover.exe
    keeps the value in `edx` (CSE'd at entry, re-materialised from the
    global after each call) where ours keeps it in its home slot; that
    residual is not reachable from source - see rule 280. 35.03% -> 40.85%
    together with rules 261 and 263.

279. **A `mov <reg>, [esp+<slot>]` of a local in the *entry* block, before
    any store to that slot, is the compiler loading a genuinely
    uninitialised local - reproduce it by leaving the local uninitialised,
    not by zeroing it.** `Sub_404030` loads `ebp` from `[esp+0x18]` at
    0x404049 and can reach its trailing `if (dwX1 >= dwX) return;` (0x4042af)
    along the path where an object's span count is <= 0, i.e. without ever
    assigning `dwX1`. Our `dwX1 = 0;` compiled to `xor ebp, ebp` and was a
    real behavioural difference from hover.exe (which compares against
    whatever the frame slot held). Deleting the initialiser reproduces the
    load exactly and raised the function 40.56% -> 40.85%. VC 2.x emits no
    C4700 for this shape (the variable is only *read* on one path), so a
    missing initialiser is not flagged by the build.

280. **Negative for `Sub_404030` (0x404030): the operand order of an integer
    `cmp` is not source-controllable under /O2.** hover.exe compares
    `cmp ebp, eax` / `cmp ebp, [esp+0x28]` (loop variable first) at every
    clip test; our build canonicalises to `cmp eax, ebp` /
    `cmp [esp+0x28], ebp`. Rewriting all six relational tests with their
    operands (and the relation) swapped - `dwX1 < dwX` as `dwX > dwX1`,
    `dwX1 >= dwXMax` as `dwXMax <= dwX1`, and so on - produced a
    *byte-identical* object: 40.85% before and after. The order is decided
    after the front end, so do not spend a build on it. (Measured
    separately: which side ends up in the register does move when an
    unrelated function in the same .cpp changes, i.e. it is rule 14 churn,
    not a source lever.)

281. **Negative for `Sub_404030` (0x404030): writing the object-advance out
    at every site reproduces hover.exe's eleven duplicated exits but scores
    much worse than one shared `L_advance`.** hover.exe repeats
    `mov edi, [edi+0x38]; test edi, edi; je <sunk tail>; jmp L_top` at all
    eleven "step to the next object" points, with eleven separate copies of
    `Sub_406D00(dwX, dwXMax); return;` sunk to the end of the function -
    417 instructions against our 293. Spelling that out in source (a
    `HOVER_NEXTOBJ` macro carrying the null test, the call and the `return`,
    plus a `goto L_top`) does emit eleven copies and brings the instruction
    count to 429, and it also removes the spurious `dwXMax` CSE into `ebx`,
    but our compiler folds each copy's back-edge (`jne L_top` with the tail
    falling through) instead of hover.exe's `je <tail>; jmp L_top` with the
    tail sunk, so the copies land in the wrong order and the score falls
    35.03% -> 32.15% (28.70% with rule 278's step local). A variant that
    duplicates only the advance and shares one `L_end:` label scores 25.77%.
    Our build *does* sink `return` blocks in general - the six
    `Sub_402630`/`Sub_402990`/`Sub_4066B0` tails come out in hover.exe's
    order and position - it just will not sink one whose `if` is followed
    by nothing but a backward `goto`. Keep the single shared advance label.

282. **`CSub402E00::Sub_4030F0` (0x4030f0): a pointer member that is tested
    and then stored is read *once* into a local in hover.exe.** The four
    edge-node appends guard on `pSrc->m_pUnk8C` / `m_pUnk90` / `m_pUnkA0`
    and then store the same member into the new node's `m_pUnk04`; hover.exe
    emits `mov edx, [ebx+0x8c]; test edx, edx; je ...` and later
    `mov [esi+4], edx`, while reading the member twice in source produced
    `cmp dword ptr [ebx+0x8c], edi` plus a reload. The same shape applies to
    `pObj2->m_pUnk30` at the end of the re-sort (hover.exe keeps it in the
    `pPrev2` slot across `m_pUnk3C = ...; m_pUnk38 = 0;` and then does
    `test eax, eax`). This is rule 155/256's caching rule in its smallest
    form: one test plus one store is already enough to make the original use
    a local. 59.30% -> 59.42%.

283. **Negative, and a lead: coalescing `Sub_4030F0`'s scan-value local with
    its first arithmetic temporary is worth +1.94% on that function and
    -9.3% spread over four others in the same .obj.** hover.exe gives
    `dwT1` and `dwVal` one shared slot at `[esp+0x24]` and `dwT2` its own at
    `[esp+0x18]`; our build keeps `dwT1` at `[esp+0x18]` and coalesces
    `dwVal` with `dwT2` at `[esp+0x28]`, and carries one slot more than
    hover.exe (`sub esp, 0x1c` against `sub esp, 0x18`) even though only six
    slots are ever referenced. Deleting `dwVal` and reusing `dwT1` for the
    scan value (their live ranges do not overlap) takes the function
    59.42% -> 61.36% and its instruction count from 517 to hover.exe's
    exact 515 - but rule 14's translation-unit churn then costs
    `CSub403980::Sub_403A60` 39.91% -> 34.15%, `Sub_404030` 40.85% ->
    36.90%, `CSub402E00::Sub_4066B0` 31.43% -> 28.57% and `Hover_sub_406A80`
    39.45% -> 38.75%, for a net Accuracy 90.56% -> 90.54%. Revisit once
    those neighbours are themselves close to matching; the frame is still
    one dword too big, and no local in the function accounts for
    `[esp+0x20]`.

284. **Negative refining rule 15: under /O2 the declaration order of
    function-scope scalars does not move their stack slots.** Swapping
    `DWORD dwT1; DWORD dwT2;` for `DWORD dwT2; DWORD dwT1;` in
    `Sub_4030F0` produced a byte-identical object (59.42% either way).
    Rule 15's declaration-order lever is an /Od and nested-block
    phenomenon; for /O2 function-scope scalars the slots follow the
    optimiser's liveness and coalescing decisions, so the levers that do
    work are removing a local (rule 283) or adding one (rule 278).

285. **Behavioural audit of `Sub_4030F0` (0x4030f0) and `Sub_404030`
    (0x404030): both are semantically exact against hover.exe.** Checked
    instruction by instruction across the whole of both functions: every
    field offset, list-end test, comparison direction (all the span clips
    are signed), argument order, the `HEAPNODE` stride of 0x18 and the
    `SPANENTRY` stride of 0x14, the sprite/static split on `m_dwUnk0C`, the
    front/back-face split on `m_dwUnk14` and which of
    `m_pUnk8C/90/9C/A0` each side merges against versus appends, the
    signed `sar`/`idiv` in the mip-shift loops, and the `-1` sentinel and
    `g_dwUnk_4A4568`/`g_dwUnk_4A4530` edge pointers in the edge-list pass.
    The only divergence found in this wave was `Sub_404030`'s `dwX1 = 0`
    (rule 279), now removed. The remaining ~40% of `Sub_404030` and ~40% of
    `Sub_4030F0` is register allocation (rule 77: `pObj` in `esi` where
    hover.exe uses `edi`; `pKey`/`pTarget` swapped between `eax` and `edx`
    in all eight span-merge expansions; a whole-register rotation in the
    edge-list loop) plus Pentium-pairing scheduling, not semantics.

286. **Every `/O2` VC 2.x frame is five regions, and two counts fix every
    displacement in it.** Reading `hover.exe:0x407430` slot by slot pins
    the general shape (rule 201 named the regions; this is the
    arithmetic). From the bottom of the frame upwards: local C++
    aggregates in declaration order (here `CClientDC` 0x14 bytes then the
    `COPYDATASTRUCT` 0xC), then an 8-byte-aligned temporary region of `M`
    slots (anything wider than a dword: `fild qword` conversion temps,
    by-value struct returns, `volatile DWORD` arrays, and any local
    `struct`), then a 4-byte temporary pool of `N` slots, then the
    named-scalar block, then a 4-byte gap under `ebp-0xC` that VC still
    uses for `WORD` temporaries (`ebp-0xe`). So

        sub esp = (aggregates) + 8*M + 4*N + (named block) + 4

    and, because the named block is anchored just below `ebp`, **the pool
    is anchored at its top**: pool slot `k`, counting the
    first-allocated slot as 0, always sits at `ebp - (0x44 + 4*(N-1-k))`
    when the named block is 0x30 bytes. The 8-byte region hangs below the
    pool, so its slots move with `N` *and* `M`, and the aggregates at the
    bottom move with both plus their own size. Practical consequences:
    (a) every `[ebp-N]` in the pool is a function of the *total* slot
    count, which is why 0x407430 is all-or-nothing; (b) padding the
    8-byte region (rule 80's `volatile DWORD adw[n]; adw[0]=0;` in a
    nested block) moves the aggregates without touching a single pool
    displacement, so it can fix a `CClientDC`/`CPaintDC` displacement on
    its own; (c) `M` never affects pool displacements, so do not chase it
    first. Measure `M` and `N` from the disassembly by taking the longest
    strictly-4-spaced run of `[ebp-N]` ending at the slot adjacent to the
    named block, then solving the equation above for `M`.

287. **The complete 92-slot inventory of hover.exe:0x407430's 4-byte
    pool** (`ebp-0x1b0` first-allocated up to `ebp-0x44`), measured
    instruction by instruction; `M` = 12 and the frame is 0x224. Groups in
    allocation order, each "a+b" being the `StretchBlt` (2x) and `BitBlt`
    (1x) arms of one `HOVER_PRESENT_BLIT`:

    - 5+3 `-0x1b0..-0x194`  background rect `m_rcUnk0220`
    - 5+3 `-0x190..-0x174`  background rect `m_rcUnk0230`
    - 5+3 `-0x170..-0x154`  `m_obj0430` (`m_rcUnk228`, `6*m_dwUnk28`, `m_dwUnk2C`)
    - 5+3 `-0x150..-0x134`  `m_obj0480` (`m_rcUnk230`, same extent)
    - 5+4 `-0x130..-0x110`  `m_obj0240` waypoint (markers[0..1], `m_dwUnk20/24`)
    - 5+4 `-0x10c..-0x0ec`  `m_obj0268` waypoint (markers[4..5])
    - 5+4 `-0x0e8..-0x0c8`  `m_obj0290` waypoint (markers[2..3])
    - 5+3 `-0x0c4..-0x0a8`  `m_obj02B8` (`m_rcUnk220`, `7*m_dwUnk18`, `m_dwUnk1C`)
    - 3,3,3 `-0x0a4..-0x084` the three `HOVER_FIXMULP` expansions
    - 1 `-0x080`            the `fiadd dword ptr` integer operand of the sqrt
    - 2 `-0x07c..-0x078`    `HOVER_FIXDIVLP` (pointer, then divisor)
    - 2 `-0x074..-0x070`    `HOVER_FIXMULWP` (unused slot, then pointer)
    - 1 `-0x06c`            the final overlay `StretchBlt`'s `width*2` home
    - 10 `-0x068..-0x044`   the ten homed `pSrcDC->GetSafeHdc()` results

    The 8-byte region holds the `fild qword` temp at `-0x210`, six live
    `HOVER_ZONESIZE` pairs at `-0x208/-0x200/-0x1f8/-0x1f0/-0x1e8/-0x1e0`
    and five slots `-0x1d8..-0x1b4` that are allocated and never written.
    The named block is 0x30 bytes (`ebp-0x40..ebp-0x11`) in both hover.exe
    and our build, but hover.exe leaves `-0x40/-0x3c/-0x38` unwritten while
    we fill them; the `WORD` operand of `HOVER_FIXMULWP` is at `ebp-0xe`
    in both.

288. **What our reconstruction of 0x407430 is still missing, itemised.**
    At commit `afb21de` (45.27%) the pool is `N` = 61 against hover.exe's
    92, i.e. every one of the 186 pool references, all 34 `CClientDC`
    references and the 5 `COPYDATASTRUCT` references carry a wrong
    displacement. The 31-slot deficit is: +27 the three waypoint sites,
    which home *nothing* in our build (rule 290); +1 each on the first,
    second and last `StretchBlt`, where hover.exe also homes the raw
    source operand whose register the doubling clobbers; +3 the never-
    written third slot of each `HOVER_FIXMULP`; +1 the same slot in
    `HOVER_FIXMULWP`; +1 the final overlay's `width*2`; and -4 because we
    home fourteen `GetSafeHdc` results where hover.exe homes ten. Only the
    waypoint homes have a source-level lever at all; the rest are register
    allocation. Note the sign of the last term: because the `GetSafeHdc`
    temps are allocated *last*, an excess there shifts every other slot,
    so even a perfect reconstruction of the sixteen blit groups cannot
    land the pool until that count is 10.

289. **Corrects rule 203 for hover.exe:0x407430: the fixed-point macro's
    pointer belongs in the 4-byte pool, and no spelling reserves the third
    slot.** hover.exe puts all three dwords of a `HOVER_FIXMULP`
    expansion in the pool - operand highest (`ebp-0x9c`), pointer below it
    (`ebp-0xa0`), never-written slot lowest (`ebp-0xa4`) - and stores the
    operand first. Rule 203's `struct { DWORD dwUnused; DWORD* pA; }` does
    reserve two dwords for free, but a `struct` is an aggregate, so it
    lands in the 8-byte region instead: all four expansions in 0x407430
    sat ~0x100 bytes away from hover.exe's displacements and `M` was 17
    rather than 13. Declaring the two macro locals as `DWORD _dwB;
    DWORD* _pA;` in that order (operand first, so it is stored first and
    gets the higher address per rule 202) puts them back in the pool and
    took the function 42.05% -> 45.27%. The third slot has no spelling
    that survives: an unreferenced `DWORD`, `volatile DWORD` or `DWORD[1]`
    is deleted (rule 203), a `DWORD*[2]` array whose element 1 is used
    reserves both dwords but in the aggregate region (40.62%), and an
    initialised pad costs an instruction. Leave a dead `DWORD _dwPad;`
    declaration in the macro as documentation and accept the missing slot.

290. **A blit's homed arguments are the inlined MFC function's *parameter*
    homes, allocated in parameter order at descending addresses with the
    enregistered ones dropped, so the count per site is register
    allocation, not source.** In hover.exe:0x407430's `BitBlt` sites the
    survivors are always a contiguous run of `CDC::BitBlt(x, y, nWidth,
    nHeight, pSrcDC, xSrc, ySrc, dwRop)` read in that order downwards: the
    background rects keep `x, y, nWidth` (`-0x194/-0x198/-0x19c`) and
    enregister the rest, the waypoint markers keep `nWidth, nHeight,
    xSrc, ySrc` (`-0x110/-0x114/-0x118/-0x11c`). `StretchBlt` sites home
    the four doubled destination values plus whichever raw source operand
    the doubling clobbers, which is why the raw one is always the
    *lowest* slot of its group. Measured spellings for the three waypoint
    sites, none of which reproduces hover.exe's 5+4:
    - block locals `DWORD x,y,cx,cy` then `HOVER_PRESENT_BLIT(x,y,cx,cy)`
      - 0 pool slots; VC doubles in place at push time (`push ebx /
      add ebx,ebx`) and spills the one clobbered local into the *named*
      block at `ebp-0x2c/-0x28/-0x24`. 45.27%, and still the best.
    - the descriptor fields passed straight to the macro - 2 slots per
      site, the other two doubled values re-read at push time (35.84%).
    - rule 202's `DWORD x2,y2,cx2,cy2,cySrc;` recipe - 5 slots for the
      `StretchBlt`, laid out exactly like hover.exe's (the lowest slot
      holds a re-read source operand, `cySrc` itself being coalesced), but
      only 1 for the `BitBlt`: 6 per site (41.44%); passing `_xSrc/_ySrc`
      as the destination arguments as well changes nothing (41.24%).
    - `volatile DWORD _adwPad[5]` / `[4]` per branch (rule 80/258) - the
      arrays are aggregates, so they land in the 8-byte region: the frame
      grows 0x1b0 -> 0x214 with `N` unchanged (31.76%). Rule 80 cannot add
      pool slots, only 8-byte-region slots.

291. **hover.exe:0x40820d tests the fourth gauge's two enable flags with
    opposite polarity.** `CBumperView::Sub_407430` draws the `m_obj0340`
    gauge when `pDoc+0x82B8` is non-zero *or* `pDoc+0x82B4` is **zero**:
    `cmp [eax+0x82b8],0 / jne body / cmp [eax+0x82b4],0 / jne past-the-
    body`. We had `!= 0 || != 0`, so the gauge was also drawn whenever the
    second flag alone was set and never suppressed when it was. This is
    the shape rule 157 warns about - both arms look identical in C and
    only the branch *targets* differ (one jumps into the block, the other
    over it). Whenever a two-test `||` guard compiles to two `jne`s with
    different destinations, one of the two tests is inverted.

292. **Aligner instability (rule 231) is worth up to four points on
    `CBumperView::Sub_407430` alone, so measure a codegen change against
    the whole-file summary and against a *dead* control.** Six spellings of
    the same two fixed-point macros, all semantically identical and all
    producing `N` = 61, scored 40.30%, 41.01%, 43.21% and 45.27% on
    0x407430 with whole-file Accuracy 90.44%, 90.44%, 90.41% and 90.42%;
    the only difference between the 43.21% and the 45.27% builds is a
    `DWORD _dwPad;` declaration that VC deletes. The collateral is just as
    large and just as arbitrary: the same edits moved
    `CBumperView::Sub_4282F0` +11.6, `CSub406090::CSub406090` +7.0 and
    `CBumperView::OnUpdateGamePause` -26.7 (a 45-instruction function that
    lost its `and eax,0x80000000` to a `shr eax,0x1f` fold and swapped
    esi/edi) with no source change to any of them. Corollary for wave
    briefs: on a large function in a large translation unit, a swing of a
    few points means nothing on its own - only a change that also moves
    the *structure* (a group of displacements, a call, a guard) is real.

293. **hover.exe uses *two* projection idioms in the object pass, and which
    one a call site gets is visible in the frame.** `CSub422F50::Sub_404BB0`
    (0x404bb0) calls `Sub_4055B0` seven times. The wall endpoints and the
    two temp-wall-pod rect corners go through the by-value POINT forwarder
    of rule 139 - 0x404e1e/0x404e2e and 0x404e3c/0x404e48 fill two fresh
    adjacent 8-byte slot pairs *before* the first of the pair of calls and
    reload them for the pushes - written `Hover_Project(this, &out,
    *(POINT*)&pRect[5])`. The three player-triangle calls do the opposite:
    0x40530e..0x405381 pushes the two coordinates straight out of
    `[esi+0x164]`/`[ebp]`/`[ebx]` with no argument temp at all, i.e. they
    name `Sub_4055B0` directly with `pPt->x, pPt->y`. The pod centre is the
    same bare form (0x404f81/0x404f8f read `adwPt` itself). Swapping the two
    idioms round is worth 38.32% -> 39.47%; each by-value site costs eight
    bytes of frame, so getting the set right is also what pins the frame at
    0x100.

294. **A `Hover_PtOf` expansion whose value is folded away still reserves
    its 8-byte slot, and the holes in the original's frame are how you
    count the expansions.** 0x404bb0's clip-test block owns eight 8-byte
    temporaries at [esp+0xc8]..[esp+0x100] for four `&&` pairs, of which
    two - [esp+0xd8] and [esp+0xc8] - are never written. Those are the
    *second* operands of the two y tests, whose comparison reads
    `[esi+0x10]` directly: VC deletes both stores of the return slot but
    keeps the slot. So the first y test is written
    `Hover_PtOf(&m_obj04.m_dwUnk00).y < g && Hover_PtOf(&m_obj04.m_dwUnk08).y
    < g` and only the *second* y test names `m_dwUnk0C` as a plain member
    (39.47% -> 40.21%); writing both with the member loses 8 bytes of frame
    and shifts every displacement above [esp+0xb0], writing both with the
    accessor scores higher on the function but costs more elsewhere. Count
    the holes before deciding.

295. **`*(POINT*)&m_dwUnk84 = pt;` before `*(POINT*)&m_dwUnk8C = pt;` - the
    dead `add esi, 0x8c` says which store came first.** 0x404bb0's wall
    "not visible" block `lea`s the +0x84 address, stores +0x8C/+0x90
    through plain `[esi+N]` displacements, computes `esi + 0x8c` and never
    uses it, then stores through the lea'd pointer. That is the shape of
    the +0x84 assignment being written *first* and VC hoisting its address:
    swapping our two statements into that order is worth +0.4 points. The
    dead `add` is the second statement's address computation, discarded
    once VC decided to reach it by displacement.

296. **Two more spill/re-read nits in 0x404bb0's player branch.** (a) The
    `dwT = max(dwT, dwCap2)` step reads the `HOVER_FIXMULWP` product *once*
    (`mov eax,[esp+0x18] / cmp [esp+0x14],eax / jge / mov [esp+0x14],eax`);
    because the destination has to be `volatile` for rule 140, naming it
    twice in the `if` emits two loads and two extra instructions - copy it
    into a plain local first (an already-dead one such as `dwWide` costs no
    frame). (b) `dwCapMax` lives in a frame slot, not a callee-saved
    register: 0x40519f and 0x4051eb re-read it out of [esp+0x30] for the
    two products that scale by it, so it needs rule 123's `volatile`.
    Together +0.7 points.

297. **A parameter that the original re-reads at every test is a
    `volatile`-qualified read of the parameter, and that is what emits
    `cmp dword ptr [esp+N], 0`.** `CSub423030::Sub_401E80` (0x401e80) tests
    `bFullUpdate` at 0x401f17, 0x402018 and 0x40211f, all three times
    against memory; ours cached it in a callee-saved register
    (`mov ebx,[esp+0x58]` once, then `test ebx,ebx`), which also stole the
    register the original spends on the source offsets. Probed: a plain
    `int` parameter read three times is always cached, and
    `*(volatile int*)&f != 0` always emits `cmp dword ptr [esp+N], 0` -
    never `mov`/`test`. Worth +0.8 on its own and it unblocks the register
    assignment of everything downstream.

298. **A blit's extents are locals of its branch, not argument
    expressions - but the destination DC must *not* be one in every
    branch.** 0x401e80's first `BitBlt` pair computes width, height and the
    destination object *before* the `GetSafeHdc` null test
    (0x401f1e..0x401f2b and 0x401f4b..0x401f74); ours computed them between
    the pushes. Declaring `long lWidth; long lHeight;` in each branch and
    assigning them first fixes the order. In the *dirty* branch the
    original also has `CDC* pDC = m_pUnk4C;` - that is what produces its
    dead `lea ebp,[esi+0x4c]` at 0x401f6a and the two spill slots
    [esp+0x14]/[esp+0x18] - but the *full-update* branch must keep
    `m_pUnk4C->m_hDC` in the call, because a `pDC` local there hoists the
    `mov eax,[ebx+4]` out of the push sequence (48.32% with it, 50.35%
    without). 46.18% -> 50.35% with rule 264.

299. **In a block that doubles a rectangle, declare the doubled locals
    *before* the raw ones.** 0x401e80's 2x present branches keep the four
    raw values in registers and spill all four doubled ones
    (0x402031..0x40205b stores height*2, width*2, top*2, left*2 through a
    single `ebp` scratch). VC gives the block's spill slots to the
    *earlier* declarations, so `long l2Left, l2Top, l2Width, l2Height;
    long lLeft, lTop, lWidth, lHeight;` spills the doubled group and
    enregisters the raw group; the other order spills one raw value and
    enregisters `l2Left` (50.35% -> 53.52%). Forcing the missing one with
    `volatile` (rule 123) instead is catastrophic here: 40.14%.

300. **A blit whose source and destination rectangles are the same
    rectangle still has two pairs of coordinate locals.** 0x401e80's 1x
    present branches copy the top into a second register (0x40212b
    `mov ebx,edx`) and spill the left (0x40213a `mov [esp+0x30],ebp`)
    before the blit, and the dirty branch has the matching dead
    `mov ebp,ecx / mov ebp,edx` pair and writes its copy into [esp+0x20];
    the four pushes then come two from the copies and two from the
    originals. One `lLeft`/`lTop` pair used for both argument pairs is a
    push short in each branch. Worth 53.52% -> 54.36%.

301. **Negatives on 0x404bb0's clip tests.** (a) Rule 142's "copy the
    viewport bound into a local first" applied to the *wall* branch's four
    `&&` pairs takes the function 40.33% -> 37.82%; the pairs already CSE
    the bound into a register because they use it twice, and the local only
    gets in the way. (b) The pod branch's four tests are still inverted
    (original `mov eax,[bound] / cmp [esp+0x88],eax / jl`, ours
    `mov eax,[coord] / cmp [bound],eax / jg`). VC 2.2 canonicalises
    `a < b` by loading the *left* operand into a register and comparing
    against the right, so operand order is not the lever; probed
    (`long* pl = (long*)p; lC = g; if (pl[0] < lC) return; ... use(pl[0],
    pl[1]);`) that a **non-volatile** `long*` local over the array, with
    the later use also going *through that pointer*, gives the original's
    form exactly in isolation - but the same spelling inside 0x404bb0 does
    not reproduce it, and dropping the `volatile` also lets VC CSE the
    coordinate across the pair. Unresolved; worth ~8 instructions.
    (c) Replacing the pod projection with a bare `Sub_4055B0(...,
    plPt[0], plPt[1], ...)` (which *is* what 0x404f81/0x404f8f do) shrinks
    the frame from 0x100 to 0xf8 and loses more than it gains - the
    original evidently still reserves that call's argument pair.

302. **Adding or removing a *local* in one function re-rolls the register
    allocation of later functions in the same translation unit, and the
    trigger is the local count, not the name (sharpens rules 231/237).**
    Every variant of the 0x401e80 work above knocks the same three
    bystanders in bumpview.cpp off their previous values -
    `CBumperView::OnUpdateGamePause` 97.78% -> 71.11% (esi/edi swapped
    throughout a 23-instruction function), `CBumperView::Sub_427F40`
    82.32% -> 58.01%, `CBumperView::Sub_407430` 44.16% -> 28.49% - with no
    source change to any of them. Measured: renaming every block local in
    0x401e80 (`l2Left` -> `xL` and so on) changes *nothing*, and reverting
    either one of the two first-blit branches to argument expressions
    leaves 0x412860/0x427f40 exactly where the full change put them, so it
    is the number of block-scoped locals VC has seen so far in the file,
    not their spelling or the line count. Whole-file Accuracy 90.45% ->
    90.40%, Progress 90.25% -> 90.20%, against +8.18 points on 0x401e80
    and +2.83 on 0x404bb0. Rule 216 says revert a pure codegen-matching
    change that lowers the summary; this one was kept and flagged instead,
    because all three bystanders are other wave-8 agents' packages and are
    being rewritten in parallel, so the measurement is not predictive of
    the merged file. Whoever merges should re-take the summary and revert
    the two 0x401e80 commits if it still drops.

303. **Separate "structure" from "register naming" before deciding a big
     function is fixable: disassemble both images and diff twice, once raw
     and once with every register rewritten to `R`.** `Hover_sub_41F540`
     (0x41f540, 983 instructions, 68.15%) splits cleanly into a
     straight-line prologue (0x41f540-0x420b92, 1111 original instructions)
     and three loops (0x420b93-0x420dc3, 170). Aligning the two images'
     instruction lists with `difflib.SequenceMatcher` after normalising
     the global base, the SetRect import slot (0x4caaa8 vs our 0x491c90 -
     normalise it or you count 97 phantom diffs) and the register names
     gives, for the current source: straight-line 797/1111 raw vs 872/1111
     normalised, loops 86/170 raw vs 115/170 normalised. So of this
     function's 407 mismatched instructions only ~104 are pure
     register-name choices; the other ~300 are ordering. That split is
     what tells you whether a rule-77 cascade is the whole story (it is
     not here) and it is cheap - ~40 lines of capstone plus difflib. Do it
     before spending a wave on declaration-order lottery tickets.

304. **A whole-function behavioural audit of a straight-line
     constant-initialiser is mechanisable: interpret the original and read
     the values straight out of our source.** `Hover_sub_41F540`'s
     straight-line body does 240 global stores and 90 `SetRect` calls with
     constant arguments. Walking hover.exe's disassembly with a 60-line
     interpreter (track `mov` reg/imm/[esp+N]/[global], `push`, `xor r,r`,
     and treat every 5-push `call` as one SetRect, decrementing an esp
     delta across the pushes so `[esp+0x14]` after two pushes resolves to
     the pre-push `[esp+0xc]` - rule 190's trap in interpreter form; skip
     the four prologue register pushes or they land in the argument list,
     and remember the pushes come out bottom/right/top/left/target) gives
     one event list; a second 40-line pass over our own C++ resolving the
     `n`/`m`/`tK` (or `CPoint`) chain gives the other. Done twice
     independently, in wave 8 and before it, the two lists are
     **identical**: every global store's target and value and every
     (target, left, top, right, bottom) tuple matches, and the seven
     `SetRect` calls in the three tail loops match operand for operand by
     hand, so the function has **no behavioural divergence at all** - its
     residual is entirely codegen. Worth doing for any function whose diff
     is dominated by `mov [global], reg` and constant pushes: it either
     finds a real bug or retires the question permanently. The by-product
     is just as useful - laying the resolved values out per zone is what
     exposed the twelve-pair regularity behind rule 309.

305. **hover.exe:0x41f540's `t1..t8` temporary chain is a stand-in, not the
     original's shape - see rule 309 for what the source really is.** The
     chain does buy back some of the original's four-deep live-value
     scheduling: in places the allocator agrees and the original matches
     instruction for instruction, e.g. hover.exe:0x41f764-0x41f7b1. Two
     measured negatives on top of the chain, both still valid: rewriting
     the region in the natural form `n = c1; m = c2; ZD(a) = n; ZD(b) = m;`
     (mechanically, by moving each `ZD(x) = tK;` store back to the line
     where `tK = n;` read the slot, which preserves the volatile access
     order exactly) drops it 68.15% -> 61.26%, because VC then keeps only
     two values live instead of four; doing the same for zone 5 alone gives
     63.58%. The natural form scores *better* on the rule-260 normalised
     metric (915/1111 vs 872/1111) purely because difflib aligns its very
     regular `mov R,[esp+0x14]` / `mov [G],R` alternation more greedily - a
     reminder that the normalised number diagnoses, it does not decide.

306. **Do not re-tune 0x41f540's temporary count: 8 is a measured local
     optimum and everything around it is worse.** Renaming the `t`
     temporaries is behaviour-preserving (verified with rule 304's
     interpreter: identical event lists) and changes nothing but register
     allocation, so it is a pure rule-77 lottery. Measured, all against the
     same file state: minimal live-range colouring (4 names) 66.25%; a
     round-robin pool of 5 = 67.76%, 6 = 65.55%, 7 = 67.42%, 8 = 68.15%
     (reproduces the committed source), 9 = 67.68%, 10 = 66.30%, 12 =
     66.59%; twelve random 8-colourings ranged 65.19%-67.99%. The spread is
     +/-1.5 points of noise with the current best at the top, so a future
     wave that "improves" this by renaming temporaries has almost certainly
     just resampled the noise.

307. **Corrects rule 86: `[esp+0x10]` in hover.exe:0x41f540 is not a
     declared local at all - it is the register allocator's spill slot for
     the waypoint loop's `i + 1`, and the loop closes with `i = j` rather
     than an `inc`.** At 0x420cf4 the original stores `i+1` there, reads it
     back at 0x420d01 for `(i+1) * p[-0x50]`, again at 0x420d33 (rule 190's
     re-based `[esp+0x14]`) for the second rectangle's right edge, and a
     third time at 0x420d48 into ebx, which is the loop's increment. The
     same block also spills the first rectangle's bottom edge into
     `[esp+0x14]` - i.e. into the CPoint temporary's own slot (rule 309),
     which is dead by then. That two-spill pressure is also why the
     original loads `[esi-0x50]` twice where ours CSEs it into one
     register. Reproducing it on top of the rule-262 stand-in cannot be
     kept: with `i = j;` / `do { } while (i < 10)` the frame grows to
     `sub esp,0x10` and the function collapses to 45.14% (rule 199); with a
     plain `for (i = 0; i < 10; i++)` around it the frame holds but the
     straight-line region loses 8 more instructions, for 67.65%-67.73% net.
     Swept against nine temporary-colourings (rule 306) and twelve random
     ones, the best variant is 67.99% - under the 68.15% baseline. On top
     of the *real* shape (rule 309) it is reproducible for free; see rule
     267 for how, and rule 311 for why it still cannot be committed.

308. **hover.exe:0x41f540's first loop walks a rolling `LPRECT` and reads
     the previous rectangle's `left` through it.** Each iteration does
     `lea ebx,[esi+0x188]` / SetRect / `add edx,[ebx]` (the next
     rectangle's left edge is `p[0] + pRc->left` of the one just written) /
     `lea ebx,[esi+0x198]` / SetRect / ... - one register serving as both
     the SetRect target and the source of the previous rectangle's left,
     reassigned only after being read. Our source recomputes each left from
     the zone pointer (`p[-0x338/4] + p[-0x1a0/4]`), one instruction more
     per site. Spelling the original's shape (`pRc = (RECT*)(p + 0x188/4);
     SetRect(pRc, ...); nL = p[0] + pRc->left; pRc = (RECT*)(p + 0x198/4);
     SetRect(pRc, nL, ...);`) does reproduce it - the loop region's
     normalised match goes 115/170 -> 120/170 - but the `int nL` it needs
     re-rolls the whole function's allocation and costs 5.4 points overall
     (68.15% -> 62.77%), and giving the third loop its own pointer pair
     (hover.exe puts the zone pointer in edi and the rect pointer in esi
     there, the opposite way round from the second loop) grows the frame
     and costs 22 (-> 46.03%). Both are logged as "known shape, unreachable
     at this file state" rather than as unfound levers.

309. **hover.exe:0x41f540's ~72 paired coordinate writes are single
     `CPoint`-valued assignments: `*(CPoint*)&zone[off] = CPoint(x, y);`.**
     This is the whole structure of the function's 1111-instruction
     straight-line body and it explains every oddity earlier waves papered
     over with `volatile` scalars and a rotating temporary chain (rules 86,
     262, 263). Three independent pieces of evidence. (a) *Shape*: every
     group's two globals are an adjacent dword pair, and the pairs are the
     same twelve offsets in all six zones, always written in the same
     order - 0x2b8, 0x2c0, 0x2c8, then the six scalars 0x40..0x54, then
     0x18, 0x20, 0x28, 0x58, 0x60, 0x68, 0x228, 0x230, 0x220. (b)
     *Mechanism*: VC 2.x does not forward a constant through an aggregate
     copy, so an 8-byte class temporary is stored to the stack and read
     back for every assignment - that, not `volatile`, is why the slots are
     memory-resident, and it is also why the *second* member is often
     const-propagated straight into the global (`mov [G+4], edx`) while the
     first is always reloaded. (c) *Signature instruction*: a temporary
     from an inline two-argument constructor writes its `y` from a register
     **late**, after two of the global stores that consumed it
     (hover.exe:0x41f71c `mov [esp+0x18], edx`). A named `POINT` local
     writes it early; only the constructed temporary reproduces the late
     write. `CSize` gives byte-identical code, so the class cannot be told
     apart - `CPoint` is the plausible name for coordinate pairs. Measured:
     rewriting the body this way takes the function 68.15% -> **84.21%**
     (the straight-line region goes to roughly 90% matched), against 72.8%
     for a named `POINT` local with member assignments and 62.0% for the
     same thing without the frame fix. Look for this pattern in any VC 2.x
     function whose diff is full of `mov [esp+N], imm` / `mov R,[esp+N]`
     pairs feeding adjacent globals.

310. **Frame-slot law for VC 2.1 /O2, measured on 0x41f540: named
     aggregates are allocated at the top of the frame, compiler temporaries
     below them, and `volatile` scalars at the bottom.** 0x41f540's frame
     is `sub esp,0xc` = an 8-byte class temporary at [esp+0x14]/[esp+0x18]
     plus one spill dword at [esp+0x10] (rule 307). To reproduce it from
     C++ you need exactly one extra memory-resident dword *below* the
     temporary, and the spelling matters: `volatile int j;` lands at
     [esp+0x10] and gives the original's offsets (84.2%), while
     `struct { int j; } t;`, `union { int j; } u;` or `volatile struct { int
     j; } t;` land at [esp+0x18] and push the temporary down to
     [esp+0x10]/[esp+0x14], costing every one of the ~85 slot references
     (65.5%). A one-element array `int a[1]` is enregistered and buys
     nothing; `int a[3]` is an aggregate and takes the top. The one
     non-`volatile` route that works is a genuine spill: naming all five of
     the waypoint loop's SetRect arguments as plain `int` locals raises the
     pressure enough that VC allocates the extra dword itself
     (`sub esp,0xc`, correct offsets) - but it rewrites the loop body and
     scores lower overall (70.3%-70.7%) than the `volatile` stand-in.

311. **0x41f540 is blocked by a downstream register cascade, not by its own
     codegen: every spelling that gets its frame layout right regresses
     `CHoverApp::Sub_4253D0` and/or `CHoverApp::Sub_425600` in the same
     file by more than the function gains on the unweighted-mean
     Accuracy.** Measured over ~120 builds (rules 266/267 x loop shape x
     declaration order x identifier names; names and line numbers have no
     effect at /O2, the cascade is deterministic):

     | 0x41f540 source                      | 41f540 | 4253d0 | 425600 | Accuracy |
     |--------------------------------------|--------|--------|--------|----------|
     | committed (rule 305 stand-in)        | 68.15  | 96.55  | 100.00*| 90.42    |
     | CPoint + `volatile int j`            | 84.21  | 72.00  | 83.67  | 90.40    |
     | CPoint + aggregate filler            | 67.63  | 93.10  | 100.00*| 90.41    |
     | CPoint + five named loop args        | 70.73  | 78.16  | 83.67  | 90.39    |
     | POINT local + `volatile int j`       | 72.97  | 96.55  | 79.59  | 90.40    |
     | POINT local + `int a[3]`             | 65.42  | 89.71  | 100.00*| 90.41    |
     | POINT local, no frame fix            | 62.03  | 96.55  | 100.00*| 90.41    |

     Both sinks are pure two-register swaps (esi/edi in 0x425600,
     ecx/edx + ebx/edi in 0x4253d0) in otherwise byte-identical functions -
     rule 77 at file scope. The best row is a real net gain in matched
     instructions (0x41f540 is 1281 original instructions, 0x4253d0 is 174
     and 0x425600 is 98: +206 -43 -16 = **+147 instructions matched**) and
     it loses only because Accuracy is the unweighted mean of 938 function
     percentages. It is left uncommitted under the "do not lower the
     summary" rule. Two consequences for whoever picks this up: (i) the
     cascade is almost certainly evidence that 0x41f540 is still only
     84% right - in the true source all three functions match at once, so
     finishing the loops (rules 264, 265, 269) may well release the sinks;
     (ii) the change is worth ~0.02 Accuracy but +147 instructions, so it
     should be taken as a package with a byte-weighted measurement, not
     judged on the summary line alone.

312. **0x41f540's three tail loops: shapes and register roles.** All three
     are `do { } while` over the zone pointer with the counter reset inside
     the outer body (`xor ebx,ebx` at 0x420cdc and `xor edx,edx` at
     0x420d68 are *inside* their outer loops, i.e. `i = 0;` is the first
     statement of the body). Both inner loops close with `i = j;` where
     `j = i + 1` was computed at the top and used for the right edge of
     each rectangle (rule 307) - never an `inc`. Loop 2 (waypoints, base
     +0x70) puts the zone pointer in **esi** and the rectangle pointer in
     **edi**; loop 3 (flag drop zones, base +0x1b0) puts them the other way
     round, zone pointer in **edi** and rectangle pointer in **esi**, which
     a single `int* p; RECT* pRc;` pair cannot express - giving loop 3 its
     own pointer pair grows the frame and costs 22 points (rule 308). Loop
     1 (base +0x28) walks a rolling `LPRECT` and reads the previous
     rectangle's `left` through it (`add edx,[ebx]`) rather than
     recomputing it from the zone pointer; reproducing that raises the
     loop region's normalised match 115/170 -> 120/170 but the `int` it
     needs re-rolls the whole function's allocation (rule 308).

313. **In `/Od` code the `jmp $+5` of an inline expansion sits *after* the
    expansion's value is consumed when the expansion materialises the value,
    and *in front of* the code when it folds into an lvalue.** Measured on
    `CRobotPlayer::Sub_409350` (0x409350, rule 49) with `tools/probe`:
    - `n = Whole(x);` with `inline int Whole(const DWORD& v) { return
      (int)(short)((short*)&v)[1]; }` gives `movsx eax, word ptr [x+2] /
      mov [n], eax / jmp $+5` - the jmp last;
    - `f(Whole(a), Whole(b), 0, 0)` gives `jmp $+5 / jmp $+5 / push 0 /
      push 0 / movsx eax,[b+2] / push eax / movsx eax,[a+2] / push eax /
      call` - both jmps *before* the pushes, because in an argument
      position the expansion is entered (and closed) while the value is
      still an lvalue and the `movsx` is made at the push;
    - the same helper written `inline short& Whole(DWORD& v)` folds even in
      an assignment (`f.sSteer = Whole(f.dwNeg);` is `jmp $+5 / mov ax,
      [x+2] / mov [sSteer], ax`), which is what hover.exe:0x409e3e and
      0x409e6a have.
    So counting jmps tells you the number of expansions (rule 117) and
    their *position* tells you which spelling produced them. The same
    reading identifies four more one-expansion helpers in that function:
    `((short*)&x)[1] += n`, `((short*)&x)[1] -= n`, `x &= 0x1FFFFFF` and
    the options-block accessor of rule 315. Worth 68.68% -> 72.20% on
    0x409350 (111 jmps in the original against our 80 before, 111 after).

314. **A `/Od` by-value wrapper has a result local exactly when its result
    is handed to another expansion, and none when it is stored.**
    hover.exe:0x409350 contains both spellings of the same multiply:
    `f.dwSquare = FixMul(v, v)` is `mov [b],v / mov [a],v / mov eax,[b] /
    push eax / lea ecx,[a] / call / mov eax,[a] / mov [f.dwSquare],eax /
    jmp` (no result slot, i.e. `{ ...; return dwA; }`), while the two
    squares inside `FixSqrt(FixSum(FixMul(y,y), FixMul(x,x)))` each get
    `mov eax,[a] / mov [r],eax` before their `jmp` (i.e. `{ ...; dwR = dwA;
    return dwR; }`). VC never inserts that copy by itself - probed with
    both shapes - so the reconstruction needs two helpers
    (`HoverFixMul`/`HoverFixMulR`, `HoverFixAdd`/`HoverFixAddR`). The same
    reading explains the two jmps after the speed sum: the sum is one level
    deeper than the sums that are stored (`HoverFixSum` is a trivial
    `return HoverFixAddR(a, b);`), and the square root's argument home is
    that wrapper's result slot rather than a slot of its own.
    Corollary: a trivial `return Inner(a);` wrapper emits no code at all
    and closes after the value is consumed, which is how a statement ends
    in two adjacent jmps.

315. **A member of a `theApp` block that is read several times in one
    function is read through an inline of its own.** In 0x409350 the turn
    limit `((DWORD*)theApp.m_pUnk0154)[3]` is read five times, and the
    original has a bare `jmp $+5` in front of every one of the five
    `mov eax, [theApp+0x154]` loads and nowhere else (hover.exe:0x409e7c,
    0x409e93, 0x409ea4, 0x409ebf and, with the whole-part read of rule 313,
    0x409df7/0x409dfc). `inline long HoverAppTurn() { return
    (long)((DWORD*)theApp.m_pUnk0154)[3]; }` reproduces all five: the body
    is a plain lvalue, so it folds and only the expansion end is left
    (71.05% -> 71.97%).

316. **Rule 159's materialising comparison leaf is a `?:`, not an
    `if`/`else` of `return`s, and VC 2.x always lays a `?: 1 : 0` out with
    the zero arm first.** The original's leaf is
    ```
    cmp [x],0 / jge Lelse / mov [R],1 / jmp Lendif / Lelse: mov [R],0 /
    Lendif: jmp Lend / mov eax,[U] / mov [R],eax / Lend:
    ```
    - one shared return jump, and both arms writing the *result* slot,
    which only a `?:` does (its temporary is the return slot). Written
    `if (c) return 1; else return 0; return nR;` VC emits a return jump per
    arm plus the else-skip (one instruction too many, and it also stops the
    `!=` composite of rule 159 from branching); written `if (c) nQ = 1;
    else nQ = 0; return nQ; return nR;` it emits `mov eax,[nQ] / mov
    [R],eax` for the return (two too many). `return c ? 1 : 0; return nR;`
    gets the count and the structure right - but VC canonicalises both
    `(x < 0) ? 1 : 0` and `(x >= 0) ? 0 : 1` to `jl` with the zero arm
    first, so the original's `jge` with the one arm first cannot be
    reproduced from source: three instructions per leaf (the jcc and the
    two `mov`s) stay mismatched at all four sites. Measured: `?:` 72.20%,
    `if/return` 70.96%, `if/assign` 71.05% with a bigger frame.

317. **Negative results on hover.exe:0x409350 (do not repeat).**
    - Expanding "16.16 from a whole number" *inside* each comparison leaf
      (so the leaves take the whole number, `FixLtI(0x100, x)`) removes the
      one stray expansion end in front of `mov eax, 0x1000000` but lets VC
      fold the constant into the compare (`cmp [x],0x1000000`), losing four
      instructions per site: 71.05% -> 68.34%.
    - Giving the square root's wrapper a `const DWORD&` parameter to stop
      it homing its argument binds the reference to the *rvalue* the sum
      returns, so VC materialises a temporary and reaches it with
      `lea/mov [t],eax/mov eax,[t]/mov eax,[eax]`: much worse than the one
      argument home it was meant to remove.
    - Publishing `m_wUnk01F0`/`m_wUnk01F4` through an inline that writes
      the whole part and then the fraction adds an expansion end the
      original does not have at four of its five sites: 71.97% -> 70.53%.
    - Giving `HoverFixOfInt` a result local (`long lR; lR = ...; return
      lR;`) costs 12 points on its own (72.20% -> 59.56%): one extra
      four-byte temporary moves the whole temporary region.
    - Removing the argument home at hover.exe:0x409593 (spelling the
      publish wrapper `return dwA;` there instead of the two-slot form)
      also moves the region: 72.20% -> 64.70%.

318. **The `/Od` temporary region of hover.exe:0x409350 is now the right
    size and the right count but not the right order - a measured open
    problem.** With the expansion set of rules 260/261 the frame is the
    original's 0x234 and the mapping from the original's temporary slots to
    ours is a *bijection* over 94 slots (script: pair up the operands of
    every otherwise-identical instruction), so no expansion is missing or
    extra; ~200 of the remaining 280 mismatched instructions are that
    permutation. What is known about the order: within one statement the
    original puts the parameter homes of the *innermost* expansions
    deepest (`FixMul`'s two homes at ebp-0x218..-0x224) and every
    expansion's *result* slot much higher (ebp-0x1f4..-0x200), with the
    square root's `double` local in between at ebp-0x214; our build groups
    each expansion's home and result together and puts the square root's
    temporaries deepest instead. A three-statement probe of the same shape
    lays temporaries out strictly downward in source order with each
    expansion's locals above its parameters, which does *not* explain the
    original, so the rule is still unknown. Anyone continuing here should
    start from that bijection rather than from the percentage.

319. **Header change that hover.exe:0x409350 needs and that no in-function
    edit can make.** At hover.exe:0x4097bd the original pushes the target
    point with `mov eax,[pt.x] / mov ecx,[pt.y] / push ecx / push eax`,
    i.e. `Hover_sub_40B6E0` takes the velocity as an eight-byte *by-value
    structure* in its first parameter, not as two DWORDs:
    `void Hover_sub_40B6E0(FIXPT pt, DWORD dwAngle, DWORD* pdwAlong,
    DWORD* pdwSide);` (still five pushed dwords, still `add esp,0x14`).
    Ours emits `mov eax,[pt.y] / push eax / mov eax,[pt.x] / push eax`.
    The declaration is `src/hover.h:943` and the function has four callers
    (texload.cpp and three in gameobj.cpp), so it is a whole-header change:
    left unmade, costs four instructions.

320. **Extends rule 231: it is the inline expansions a translation unit
    actually *emits*, not the declarations it contains, that perturb the
    register choice of unrelated functions earlier in the file.** Probed on
    texload.cpp: adding thirteen unused file-scope `inline` helpers before
    `CRobotPlayer::Sub_409350` leaves `CRegionMatrix::Sub_4086A0` (defined
    1600 lines *earlier*) byte-identical; using them inside 0x409350 flips
    that function's `this` register from ebp to edi and costs it 81.94% ->
    70.83%, with `CSub410F20::Sub_419530` losing 68.91% -> 66.45%.
    Renaming the helpers changes nothing, so it is not rule 195's
    identifier hash. On 0x409350 the trade is +6.08 points on the largest
    function in the game (4405 bytes) against -13.6 spread over two small
    ones, which the unweighted per-function mean that `tools/compare.py`
    reports as Accuracy/Progress scores as -0.01; kept per rule 231/259.

321. **Inline `__asm` anywhere in a function switches off VC's coalescing
    of expression temporaries into one stack slot - this is the missing
    ingredient in rule 150.** Without asm, every `f(p).x` sub-temporary of
    the rule 104/118 accessor pattern is folded onto a single slot; with
    one `_asm` statement in the function the same source gives each
    expansion its **own** slot at an 8-byte stride, exactly like
    hover.exe. Probe (`tools/probe/probe.sh <file>.cpp`, default /O2): two
    copies of `if (i >= Cells(p).x) i = Cells(p).x - 1; if (Cells(p).x <=
    j) j = Cells(p).x - 1; ...`, the second with a trailing `_asm int 3`.
    The first compiles to `mov [esp+0x10],eax / cmp [esp+0x10],ecx`
    four times (one slot, an esp frame); the second to `[ebp-0x48]`,
    `[ebp-0x40]`, `[ebp-0x38]`, `[ebp-0x30]` (four slots) with a full
    `push ebp / mov ebp,esp / sub esp,N / push ebx / push esi / push edi`
    prologue. So an original function whose frame is *far* larger than
    ours and which repeats `mov [ebp-N],eax / mov eax,[ebp-N]` at
    ever-changing displacements is telling you it contains inline
    assembly, not that it was built with a different compiler.
    `CWall::Sub_408820` (0x40ef30) went 18.40% -> 34.50% on nothing but
    restoring its two `__asm` fragments (rules 261 and 262), and its frame
    from 0x44 to 0x1b0 against the original's 0x1b8. Corollary: the
    `#pragma optimize("y", off)` such a function needs to get its `ebp`
    frame is redundant once the asm is there (measured: removing it changes
    no instruction), and `_asm` operand names must avoid the FPU register
    spellings - a `short sT` fails with C2404 "'ST' : illegal register".

322. **hover.exe:0x40F267 is a debug trap left in the shipping build.**
    `add dword ptr [ebp-0x64],eax / jns $+3 / int 3` - the only
    `jns`/`int 3` pair in the whole binary (rule 150 flagged it as
    unexplained). It is `nRow += nRowDir; if (nRow < 0) { _asm int 3 }`:
    the `jns` reads the flags of the `add`, so no separate `cmp` appears
    and the guard costs exactly two bytes. Only the column-walking branch
    of `CWall::Sub_408820` has it; the row-walking branch does not, which
    is what a hand-inserted trap looks like.

323. **The grid-crossing multiply in `CWall::Sub_408820` is a
    `HOVER_FIXMULWP`-shaped `__asm`, written out in the function body.**
    hover.exe emits `mov eax,fxSlope / lea ecx,[fxT] / mov [fxT],eax /
    mov [pFx],ecx / mov eax,nXBound / sub ax,word ptr nStartX /
    mov word ptr [sDelta],ax / mov ecx,[pFx] / movsx eax,word ptr [sDelta]
    / imul dword ptr [ecx] / mov [ecx],eax`, i.e. rule 71's pointer
    temporary with the one-operand `imul`. Spelled in C as
    `fxT.dw = fxSlope.dw; pFx = &fxT; sDelta = nXBound - nStartX;
    pFx->dw *= sDelta;` VC copy-propagates both temporaries away and emits
    a two-operand `imul reg, mem` with no `pFx` and no `sDelta` slot; the
    four `_asm` lines have to be written inline in the function body
    (not via the hover.h macro) so that `pFx` and `sDelta` are the
    function's own locals in its own frame. Note the 16-bit `sub ax, word
    ptr` comes for free from declaring `sDelta` as `short`, and the
    cross-coordinate is `((short*)&fxT)[1] + nStartY`, a `movsx` of the
    *high* half.

324. **A `DWORD` parameter that is really a pointer costs a hoisted address
    temporary inside a loop.** hover.exe:0x40EF30 reads its collider
    argument fresh at every use (`mov eax,[ebp+8] / cmp [eax+0x44],0`) and
    keeps no local for it - writing `CCollider* pMgr = (CCollider*)dwA;`
    instead puts the pointer in a local and costs a point (34.50% ->
    35.52% when the local is removed and every use spelled
    `((CCollider*)dwA)->...`). But inside the two walk loops VC then
    hoists the *address* of the tested member out of the cast expression:
    `mov ecx,[ebp+8] / add ecx,0x44 / mov [ebp-0x24],ecx /
    mov eax,[ebp-0x24] / cmp [eax],0` where hover.exe has just the two
    instructions. That is three extra instructions per loop **and one
    extra stack slot**, which shifts every displacement below it. It does
    not happen with a real `CCollider*` parameter, so the last few points
    of 0x40EF30 need `CGameObj::Sub_408820`'s vtable-wide signature
    retyped from `DWORD` to `CCollider*` - a header change across the
    whole maze-object hierarchy, deliberately not made.

325. **Negative: the /O2 stack-home order of `CWall::Sub_408820` is
    reachable by neither declaration order nor rule 195's /Od name hash.**
    Measured with two probes: (a) sixteen locals `aa..pp` plus candidates,
    each assigned then used, come out in strict **reverse declaration
    order** (last declared at `ebp-4`) at /O2, and (b) assigning them in
    the reverse order changes nothing - so in a simple function the rule is
    just "reverse declaration order", with 2-byte locals pooled at the top
    (`ebp-2`, `ebp-4`). In 0x40EF30 that rule does not hold: moving
    `nCol`/`nRow` from the middle of the declaration list to the front
    produced a byte-identical function. The order there is decided by the
    register allocator's spill order, which is not addressable from the
    source. What *is* addressable: block-scoping the per-branch temporaries
    (`fxSlope`, `pFx`, `fxT`, `sDelta`, the two bounds, the step and the
    direction, declared in that order in each branch) reproduces the
    original's relative slot order **exactly** from `ebp-0x24` downwards,
    including the way the two branches interleave with the `else` branch's
    copy 4 bytes shallower than the `if` branch's - but it also adds four
    slots above `ebp-0x24` (`nCol`, `nRow`, rule 324's address temp and
    branch 2's `nXBound`, which hover.exe keeps in `ebx`), shifting every
    displacement below by 0x10 and costing 3.5 points (35.52% -> 32.05%).
    Keep the temporaries at function scope until rule 324's header change
    removes the address temp.

326. **Negative: VC canonicalises comparison operand order and ternary
    polarity, so neither can be read back out of the original.**
    hover.exe:0x40EF30 clamps its two "A" bounds as `cmp nColA, Cells.x /
    jl` and its two "B" bounds the other way round as `cmp Cells.x, nColB /
    jg`, and writes its two direction constants as `mov eax,-1 / jle /
    mov eax,1` while the two step constants are `mov eax,nCell / jg /
    neg eax`. Writing the source to match (`if (Cells.x <= nColB)`,
    `(nEndY <= nStartY) ? -1 : 1`) produces byte-identical output to the
    naive spelling: VC normalises `A <= B` to `B >= A` and always loads the
    ternary's *true* value first. A small probe *does* honour
    `Cells(p).x <= j`, so the choice is register-pressure dependent, not a
    source-form question - do not spend time on it.

327. **hover.exe:0x40EF30 clamps the wrong variable, in both branches.**
    After `if (nColA < 0) nColA = 0;` the original's next guard is
    `cmp dword ptr [nColB],0 / jge / mov dword ptr [nColA],0` (0x40F102 in
    the column branch, 0x40F384 in the row branch, where `nColA` is `ecx`):
    the test reads the *end* column and the assignment writes the *start*
    column. It is a genuine bug in the shipped game (a wall whose far end
    is left of the grid resets its near end to column 0 instead), and our
    `if (nColB < 0) nColA = 0;` reproduces it verbatim in both branches -
    do not "fix" it. Everything else in that function's audit matches:
    argument order for `CCollider::Sub_40F8A0(nCol,nRow)` and
    `CRegionMatrix::Sub_4086A0(nCol,nRow,this)`, the `nDY==0 && nDX==0`
    degenerate case, the `abs(nDY) < abs(nDX)` split, the endpoint swap
    with `nDX`/`nDY` negation in the `else` arm of each branch, the
    `nRowA*nCell + max(nRowStep,0)` seed, the `nXBound += nCell` step and
    the shared `nCol = nColB; nRow = nRowB;` tail.

328. **VC 2.1's register allocation for one function changes when an
    *unrelated* function in the same .cpp changes.** Editing
    `CBumperDoc::OnOpenDocument` (texture.cpp) flipped the eax/ecx polarity
    of ten inlined `CMerlinArchive::ReadWord` bounds checks in
    `CMerlinTexture::Serialize` (0x412980) - whose source was untouched -
    for 43.01% -> 38.49%, and a later edit in the loader put it back to
    42.11%. It is the same lottery as rules 77/237 but across function
    boundaries: C2 carries state (identifier tables, the temp allocator's
    free lists) from one function to the next in a translation unit. Two
    consequences: (a) never quote a sibling function's percentage as
    evidence for or against an edit in another function of the same file -
    quote the summary; (b) a function whose score is a scheduling lottery
    (rule 336) should be re-rolled *after* the rest of its file is settled,
    since every neighbouring edit re-rolls it anyway.

329. **An expanded inline's by-value parameter and return-value
    temporaries are allocated at the very bottom of the /Od frame; a
    temporary the expression itself creates is allocated in the middle
    pool at its source position.** This is what rule 198's `SetSegment`
    inline got wrong: its two by-value `POINT` parameters reproduce
    hover.exe:0x416825's instruction shape but land at ebp-0x42c/-0x434
    instead of the original's ebp-0x338/-0x340, and they push the whole
    bottom of the frame (including the `this` home, 65 references) out of
    line. Calling the member directly with MSVC's address-of-class-rvalue
    extension - `pSide3->m_obj04.Sub_422BF0((DWORD*)&PtAt(tail),
    (DWORD*)&PtAt(head))`, where `PtAt` returns a `CPoint` by value -
    materialises both pairs in the middle pool at exactly the original's
    slots and cost nothing: `CBumperDoc::OnOpenDocument` 73.89% -> 85.90%
    in one edit. VC 2.1 accepts `&<class rvalue>` silently at /W3. The one
    thing this spelling does not reproduce is the fill *order*: hover.exe
    fills argument 2's temporary first (right-to-left, rule 198) while the
    address-of-rvalue form fills argument 1 first; both agree that argument
    1 takes the lower slot, so the two nine-instruction endpoint loads come
    out swapped (18 diff lines) and swapping the source arguments would
    swap the endpoints' meaning. Left as is.

330. **Where 0x415450's last 8 frame bytes go (rule 199 diagnosed, still
    open).** Our frame is 0x454 against hover.exe's 0x44c, which puts the
    `this` home at ebp-0x460 instead of ebp-0x458 and mismatches 65
    instructions. The count closes exactly: we allocate **six** dwords too
    many at the bottom, because each of the three `pLoc->GetPoint()`
    arguments costs *two* temporaries here - the inline's returned `POINT`
    is homed at the bottom (ebp-0x448..-0x45c) and then copied into
    `Sub_41B2D0`/`Sub_41B0E0`'s by-value argument temporary at
    ebp-0x2bc/-0x2c8/-0x2d4, where hover.exe builds the pair directly with
    two `movsx` stores and no copy - and **four** dwords too few in the
    middle pool, because hover.exe materialises three `operator LPCTSTR`
    results (ebp-0x3f0/-0x3fc/-0x404, each behind an inline return `jmp`,
    rule 199c) and one `_mbscmp(...) != 0` flag (ebp-0x3f4) that we fold
    into the use site. +6-4 = +2 dwords = the 8 bytes. Both halves have to
    be fixed together: removing only the doubled `GetPoint` temporaries
    would leave the frame 16 bytes too *small*. Spellings probed without
    effect: `CPoint((short)p->m_wUnk14, (short)p->m_wUnk16)` (moves the
    object into the middle pool but homes the two ctor arguments at the
    bottom instead - same total), `*&x.Left(n)` and casting through
    `(const CString&)` for the `operator LPCTSTR` temporary.

331. **The level loader's three maze-face name tests call `CString::Left`,
    not `Right` - a behavioural bug, and the way to tell the twins
    apart.** hover.exe:0x416af9/0x416b8b/0x416cd7 call 0x448a8d, which
    computes `nCount = min(nCount, m_nDataLength)` and then
    `AllocCopy(dest, nCount, 0, 0)`; `Right` differs only in passing
    `m_nDataLength - nCount` as the third argument (`nCopyIndex`), so
    reading that one push settles which twin an address is - as does
    `tools/libmatch.py --show 0x448a8d`, which matches our own
    `CString::Left` instruction for instruction. The loader therefore asks
    whether a face's name *starts* with `"PAD_"` (on `m_strUnk04`) or
    `"DECAL_"` (on `m_strUnk5C`); our reconstruction tested the tail, so
    every face took the wrong arm - walls were registered with the
    collider that should not have been, and `m_dwUnk38` was set on the
    wrong ones. Note MFC 3.1's `CString` is 12 bytes (`m_pchData`,
    `m_nDataLength`, `m_nAllocLength` as real members; the `CStringData`
    header only arrives in MFC 4), which is why `[ecx+4]` inside these
    functions is the length.

332. **hover.exe:0x451349 is `CGdiObject::Detach`, 0x451378 is
    `CGdiObject::DeleteObject`.** Detach saves `m_hObject`, removes it from
    the handle map, zeroes the member and returns the old handle - it
    contains no call to the `::DeleteObject` import; DeleteObject is the
    function immediately after it and *calls* Detach before
    `[0x4ca6b4]`. The level loader's palette swap
    (hover.exe:0x415823, before `CreatePalette`/`Attach`) is a Detach: the
    previous palette is leaked, not deleted, presumably because it may
    still be selected into a DC. We had `DeleteObject` there.

333. **A `&&` term that is a plain member test is spelled without
    `!= 0`.** hover.exe:0x416c14 copies `pFace->m_dwUnkBC` raw into the
    chain's temporary and tests the copy; writing the term as
    `m_dwUnkBC != 0` makes VC materialise a 1/0 BOOL through a
    `je/mov 1/jmp/mov 0` ladder first (five extra instructions).
    0x415450: 85.90% -> 86.13%. The rule is the mirror image of rule 175 -
    a *comparison* term is materialised as a bool, a bare integer term is
    materialised as its own value.

334. **`CMerlinTexture::Serialize`'s span pointer is taken off the texture
    heap cursor before the cursor advances.** hover.exe:0x412d27 forms
    `pPts` with `lea ebp,[eax+ecx*8]` from the `g_pUnk_461D00` value loaded
    at 0x412cfb, i.e. the value that was just stored into
    `m_pUnk28[nLevel].m_dwUnk1C`, and only then updates the global.
    Computing `pPts` after the update (which is what the obvious statement
    order gives) forces a reload of `m_dwUnk1C` and loses the block's
    cached `m_pUnk28 + nLevel*0x20` pointer: 38.49% -> 42.11%.

335. **Negatives on `CMerlinTexture::Serialize` (0x412980).** All three are
    locally *correct* reconstructions that lose more to the register
    cascade than they gain:
    (a) splitting `g_dwUnk_4C4DA8++` out of the progress-callback call
    (hover.exe:0x412ed5 increments it after `call ecx`, and the split also
    restores the `mov ecx,[0x4c4dac]` / `call ecx` CSE that our in-argument
    `++` kills) reproduces the tail exactly but flips the eax/ecx polarity
    of ten archive-read expansions: -62 aligned instructions;
    (b) the trailing-comment skip written as hover.exe has it -
    `short sCount = (short)ReadWord(); if (sCount > 0) { i = sCount; do
    ReadByte(); while (--i); }` - reproduces 0x412e42-0x412e4d instruction
    for instruction (the 16-bit `test cx,cx` before the `movsx`) and costs
    3-4 points through the same cascade, twice, in different file states;
    (c) the frame is 0x18 against hover.exe's 0x20 with the `this` home at
    [esp+4] instead of [esp+0xc], and it cannot be padded: a
    `volatile DWORD[n]` and separate `volatile DWORD` scalars both land at
    the *top* of the frame whatever their declaration position, so the
    frame grows without moving the home (43.01% -> 41.29%/37.12%/25.59%
    for three placements).

336. **0x412980's five mip-header reads are a scheduling difference, not a
    source-shape one.** hover.exe evaluates the destination's `this` and
    `m_pUnk28` loads *before* the inlined `ReadWord`'s cursor store and
    computes the next read's bounds check off the cursor register
    (`lea ecx,[edx+2]`) before the member store, so it needs 18
    instructions per read where we need 19 and align only half of them.
    Probed and rejected: a store-into-destination reader
    (`RD(pAr, &m_pUnk28[i].m_dwUnkNN)`) - the by-pointer argument is
    materialised with a `lea` into one register, which the original never
    does; explicit byte arithmetic `*(DWORD*)((BYTE*)m_pUnk28 + i*32 + 8)`
    - byte-for-byte identical to the array form; a local class with a
    static member function - VC 2.1 rejects it outright (C2601,
    "functions cannot be defined in local classes"). The remaining gap on
    this function is register allocation, not structure.

337. **Audit technique: diff the *multisets* of member offsets, immediates
    and resolved symbols instead of reading 1900 instructions twice.**
    For a big /Od function, extracting every `[reg + 0xNN]` displacement,
    every non-address immediate and every `(FUNCTION)`/`(DATA)`/`(STRING)`
    reference from both disassemblies and comparing the counts finds wrong
    fields, wrong constants, wrong allocation sizes and wrong callees
    mechanically. On `CBumperDoc::OnOpenDocument` the offset/immediate
    multisets differed in exactly two entries (the frame size, and one
    `0xa0` swallowed by our switch jump table being disassembled as code),
    and the symbol multiset in exactly one (`___CxxFrameHandler`), which is
    what certified the loader's field and argument reconstruction; the two
    behavioural faults of rules 263/264 came out of the symbol multiset as
    `CString::Left` vs `CString::Right` and `Detach` vs `DeleteObject`.

338. **Do not delete the now-unreferenced `g_dwUnk_4A1D10` (texture.cpp:63,
      texture.h:261), even though rule 331 made it dead.** It was a stand-in
      for `g_obArrUnk_4A1D08.m_nSize` and nothing reads it any more, so it
      looks like obvious tidy-up. Measured on master 8d65142: removing both
      the definition and the extern costs Accuracy 90.59% -> 90.55% and
      byte-weighted accuracy 81.41% -> 80.93%. An unreferenced file-scope
      `DWORD` still occupies a slot in our .data layout, and taking it away
      shifts the globals after it, which re-rolls every function in
      texture.cpp that addresses them. The same caution applies to any other
      dead global: measure before deleting, and prefer leaving it with a
      comment (rule 45 for types, this rule for data).

339. **Rule 309's `CPoint` rewrite of hover.exe:0x41f540 is now committed,
    and the trade it loses on the headline Accuracy is real but small.**
    The recipe is: `*(CPoint*)&zone[off] = CPoint(x, y);` for each of the
    twelve pairs per zone in the fixed order 0x2b8, 0x2c0, 0x2c8, the six
    scalars 0x40..0x54, then 0x18, 0x20, 0x28, 0x58, 0x60, 0x68, 0x228,
    0x230, 0x220, followed by 0x2d0; plus a `volatile int j` for the frame's
    third dword, used for the two waypoint rectangles' right edges so that
    it is not eliminated. 68.15% -> 84.21% on its own. It costs
    `CHoverApp::Sub_4253D0` 96.55% -> 72.00% and `CHoverApp::Sub_425600`
    99.00% -> 83.67% (rule 311's file-scope cascade), so the unweighted
    Accuracy moves 90.58% -> 90.56% while the byte-weighted accuracy moves
    80.92% -> 81.27%: +1008 original bytes matched against -182. **Judge
    changes to a 6 KB function with `tools/compare.py --json` piped through
    a size-weighted mean, not with the summary line** - the summary is an
    unweighted mean over 938 functions and cannot see a 4 %-point move on
    the largest function in the binary. Two later fixes (rule 340) took it
    to 87.79% / 81.37%.

340. **hover.exe:0x41f540's per-zone header writes the two split-line
    centers back to back, and zone 5 uses the same order as the rest.** The
    eight header stores are `0x00, 0x04, 0x10, 0x14, 0x08, 0x0c, 0x30,
    0x38` in every zone. Zones 4..0 compute `0x10` and `0x14` as
    `(prevzone[0x00] + W) / 2`; VC fills the first division's scheduling
    slot with nothing and the *second* division's slot with the `0x08`
    store, which is what tells you `0x08` comes after `0x14` in the source
    (with `0x08` written between them, VC puts it in the first division's
    slot instead). Fixing that alone is 84.21% -> 87.31%. Zone 5's centers
    are the constant 0x2710, so its stores come out in plain source order
    and an earlier wave read the order `0x04, 0x10, 0x00, 0x14, 0x08` off
    the emission; making zone 5 use the same order as the other five and
    letting the scheduler move `0x00`/`0x08` past the four register pushes
    reproduces the prologue exactly, 87.31% -> 87.79%. General lesson: when
    six copies of the same statement block exist, derive the source order
    from the copy with the *most* scheduling pressure, not the one that
    happens to come out in order.

341. **A `volatile` frame stand-in must be read or written or VC 2.1 drops
    it, and how it is read decides three separate things.** Measured on
    0x41f540 on top of rule 339: (a) an unreferenced `volatile int j;` is
    eliminated - the frame falls back to `sub esp,8`, the class temporary
    moves to [esp+0x10]/[esp+0x14] and the function scores 67.63%; (b) with
    `j = i + 1;` at the top of the waypoint loop and `j` used for both
    right edges, the frame is right and the loop's spill slot lands on the
    original's [esp+0x10] (87.79%); (c) closing either inner loop with
    `i = j;` - as a `do { } while`, or as the third clause of the `for` -
    makes the volatile read count in the inner loop tip VC into hoisting
    `dword ptr [SetRect]` into `esi` for the *whole function*, which breaks
    all ninety call sites and drops it to 59.5%-61.0%. So rule 312's
    `i = j` loop close is correct about the original and still
    unreachable: the original's [esp+0x10] is a genuine allocator spill of
    a plain `int`, and a plain `int j` (which VC keeps in a register) takes
    the frame back to `sub esp,8` and 48.2%.

342. **Negative, retested on top of rule 339: 0x41f540's rolling-`LPRECT`
    loop 1 (rule 308) still cannot be spelled.** Writing
    `pRc = (LPRECT)(p + 0x188/4); SetRect(pRc, ...); nL = p[0] + pRc->left;`
    needs one `int` for the carried left edge, and every way of providing
    it regresses the *straight-line* body, which is fifteen times bigger
    than the loop: a new file-scope-lifetime `int nL` gives 76.0%, the same
    `int` declared inside the `do` block gives 75.4%, and reusing the
    existing loop counter `i` (no new local at all, so not rule 302's
    "adding a local") gives 72.3% - all against 87.79% for leaving loop 1
    alone. Our form already recovers most of it anyway: VC CSEs
    `&p[0x188/4]` into `ebx` and reads the previous rectangle's left through
    it, costing two instructions per site rather than the one the original
    spends.

343. **Where 0x41f540's residual sits at 87.79%, so nobody re-derives it.**
    Of ~314 diff lines: 132 in the straight-line body, 58 in loop 1, 83 in
    loop 2, 41 in loop 3. The straight-line remainder is confined to zone 4
    and zone 0 (zones 5, 3, 2, 1 and the whole prologue match); both are
    *equal-length* hunks - 77 and 72 instructions on each side - in which
    VC picks the same registers as the original but reaches the steady-state
    rotation one pair later, so the `CPoint` temporary's `x` and `y` land in
    swapped registers for three or four pairs and the phase never
    re-syncs. That is rule 77 at its purest and no source-level lever was
    found for it. The loops' residual is scheduling: hover.exe evaluates all
    five `SetRect` arguments into registers and then pushes them in one run
    (spilling the first rectangle's bottom edge into the dead class
    temporary at [esp+0x14] to do it), while ours pushes each argument as it
    is computed and therefore never needs that spill.

344. **Rule 319's evidence is right and rule 319's change still loses:
    do not make `Hover_sub_40B6E0` take a by-value point.** The reading of
    the original is correct and holds at all four call sites - every one of
    them loads both halves of the velocity into registers and pushes them
    together, which is what VC emits for an eight-byte by-value structure
    and never for two DWORD arguments: `mov eax,[ebp-0x20] / mov ecx,
    [ebp-0x1c] / push ecx / push eax` at 0x4097bd, `mov ecx,[edi+4] / ...
    / mov eax,[edi] / push ecx / push eax` at 0x41ab50, and `lea eax,
    [esi+0x1ac] / mov ecx,[eax] / mov edx,[eax+4] / push edx / push ecx`
    twice in 0x42b8e0. Applied in full (a `FIXPT` typedef in hover.h, the
    declaration at hover.h:943, the definition at hover.cpp:5418 and all
    four call sites spelled `*(FIXPT*)&...`) it measures 81.86% ->
    81.65% byte-weighted, Accuracy 90.58% -> 90.41%: 0x41ab10 58.54 ->
    63.41, 0x40b6e0 47.20 -> 48.45, 0x42b8e0 38.31 -> 37.13, and the
    function it was supposed to help most, 0x409350, 72.20 -> 69.85. Two
    separate reasons, rules 345 and 346. No behavioural difference: the
    callee reads the same two dwords at the same frame offsets either way.

345. **VC /Od pushes an eight-byte by-value structure with a `lea` when
    the argument is a subobject at a non-zero offset, and without one
    otherwise - which is why rule 319's change cannot help
    hover.exe:0x409350.** Probed with `tools/probe/probe.sh ... /Od`: a
    plain local, a local *array* reached through a cast (`*(FIXPT*)adw`),
    and a struct member at offset **0** all give the original's four
    instructions, `mov eax,[lo] / mov ecx,[hi] / push ecx / push eax`; the
    same struct member at any non-zero offset gives five in the other
    register roles, `lea eax,[ebp-N] / mov ecx,[eax] / mov eax,[eax+4] /
    push eax / push ecx`. 0x409350's locals live inside the `ROBOTSTEER`
    frame-pinning struct of rules 305/310, so `f.adwTarget` is a non-zero
    offset subobject and the retype replaces four wrong instructions with
    five wrong ones. Freeing it would mean taking `adwTarget` out of that
    struct, which rules 306/317 measure as costing far more than it could
    return. `__int64` in place of the structure emits exactly the same
    four instructions in the plain-local case, so the pattern identifies
    "eight bytes by value", not "a structure".

346. **Adding *any* user-defined type to hover.h re-rolls register
    allocation program-wide, and on the current tree that roll costs 0.58
    byte-weighted points.** An unused, otherwise inert `typedef struct
    tagFIXPT { DWORD m_dwX; DWORD m_dwY; } FIXPT;` with nothing else
    changed measures 81.86% -> 81.28% and drops 0x409350 from 72.20% to
    70.02%. The identical score comes back for `DWORD x; DWORD y;` member
    names, for a one-`int` struct with an unrelated tag, and with the
    typedef moved from the middle of the header to just above the final
    `#endif`; fifteen lines of *comment* in the same place change nothing
    at all. So it is neither rule 195's identifier hash nor line numbers -
    it is the extra entry in each translation unit's type table, and it is
    position- and name-independent. The damage is chaotic (e.g.
    `CSub403850::Sub_403850` in texheap.cpp, which touches none of this,
    flips a register from edx to eax and falls 100% -> 37%), so the sign
    of the roll is luck; budget +/-0.5 points for it and never introduce a
    type into a shared header for a gain smaller than that. Measured
    against this control the rule 319 retype is worth +0.37 on its own,
    which is less than the roll it has to pay for.

347. **`POINT` is not a cheaper stand-in for rule 319's structure, and
    `__int64` is much worse.** `POINT` adds no new type (so it skips rule
    346's toll) but measures 81.20% byte-weighted - the signed `LONG`
    members change the callee's code (0x40b6e0 48.45% with `FIXPT` ->
    45.00% with `POINT`) and 0x42b8e0's whole register rotation shifts,
    taking it to 27.50%. `__int64` measures 80.47% with the same 27.50% on
    0x42b8e0. Both were tried because rule 346 made the by-value shape look
    free if no type had to be declared; neither is.

348. **Rule 252 taken: `g_dwArrUnk_4A47F0` is a `CObArray` of
    `CSub40B800*`, and the retype pays for itself - but only just, and
    not where you would expect.** The evidence held up exactly as rule
    252 stated, and it is not a twin ambiguity: `CSub40B800::CSub40B800`
    calls 0x4496c4 = `?SetAtGrow@CObArray@@QAEXHPAVCObject@@@Z` at
    hover.exe:0x4153d8, and the unregistration in
    `CSub40B800::~CSub40B800` calls 0x4496ed =
    `?RemoveAt@CObArray@@QAEXHH@Z` at hover.exe:0x422dd6 - two distinct,
    separately-named `CObArray` entry points that a `CDWordArray` could
    never reach. Renamed `g_dwArrUnk_4A47F0` -> `g_arrUnk_4A47F0` (the
    `dw` was the type claim) and made every site cast-free:
    `SetAtGrow(GetSize(), this)`, `GetAt(i) == this`,
    `arr[i] = arr[GetSize()-1]`, and in hover.cpp's teardown
    `delete g_arrUnk_4A47F0[i]` without its now-redundant `(CObject*)`.
    `CSub40B800` derives publicly from `CObject` at offset 0 (its
    constructor stores `CObject`'s vftable at `[this]`), so all those
    conversions are implicit and adjust nothing - no operator or
    overload changed behaviour, and `delete` still dispatches through
    vtable slot 1 with `push 1` exactly as at hover.exe:0x422f11. A
    second, separate bug fell out of the same disassembly: the removal
    loop called `SetSize(GetSize()-1, 1)` where hover.exe calls
    `RemoveAt(GetSize()-1)` - besides being the wrong entry point,
    `SetSize`'s second argument was also resetting `m_nGrowBy` to 1.
    Numbers: Accuracy 90.58% -> 90.59%, byte-weighted 81.8637% ->
    81.8823%, Progress 90.38% -> 90.40% (the last only after rule 349's
    table fix). The predicted instruction did appear:
    `CSub40B800::CSub40B800` 88.68% -> 90.57%, its
    `call CDWordArray::SetAtGrow` now `call CObArray::SetAtGrow`, with
    only rule 253's base-vtable store and one eax/edx choice left. But
    `CSub40B800::~CSub40B800` *fell* 88.48% -> 85.71% on a pure reroll:
    one byte of growth flips the second `g_pvUnk_4C56F0 == this` compare
    from `mov eax,[global]; cmp eax,[ebp-0x10]` to
    `mov eax,[ebp-0x10]; cmp [global],eax` and shifts three jump
    displacements. So the two functions the change actually touches net
    *negative* by weight (+3.7 bytes against -10.6); the whole gain is
    collateral luck elsewhere - `CHoverApp::Sub_425600` 83.67% -> 98.98%
    (now a 100% effective match, one `mov` scheduled earlier),
    `CMerlinLine::Serialize` 91.77% -> 95.57%,
    `CMerlinLocation::Serialize` 91.41% -> 87.73% (an eax/ecx allocation
    swap). Keep it because the call target is decisive evidence and the
    source is now honest, not because +0.019 byte-weighted means
    anything. Two spellings tried and rejected on the destructor loop:
    `g_arrUnk_4A47F0[i] == this` instead of `GetAt(i)` scores 85.3%, and
    the old `SetSize(GetSize()-1, 1)` scores 84.79%. The residual loop
    diff is register allocation (hover.exe uses edi for the byte offset
    and esi for `i`, ours the reverse) and loop rotation (hover.exe
    re-tests at the bottom, ours jumps back to the top test); no
    source-level lever was found for either.
    On the new-type toll (a bare unused typedef added to hover.h was
    measured at -0.58 byte-weighted in the same pass, because the extra
    type-table entry re-rolls register allocation in every TU that
    includes the header): this change pays none of it. It adds no
    `#include` and no new type - `CObArray` was already used four times
    in gameobj.h itself (`CMerlinObject::m_arrUnk14/28/3C/50`) - and it
    *removes* one, since this global was the last `CDWordArray` in the
    program. That shows in the collateral: lowering the mover threshold
    from 1.0 to 0.4 points still lists only the same five functions, two
    of them the ones the change targets, where a genuine new-header-type
    edit re-rolls broadly. So the reroll cost of a container retype is
    bounded by whether the new type is already in the header's type set;
    prefer retypes that reuse a type the header already names.

349. **hover.exe contains no `CDWordArray` at all - both tools tables
    mislabelled `CPtrArray` as one.** Retyping the last `CDWordArray` in
    our source dropped `array_d.obj` out of the link, which left
    0x449a45 in `tools/orig_library.csv` (named `??0CDWordArray@@QAE@XZ`)
    with no counterpart in our build and quietly cost 0.08 of Progress.
    Settled by rule 251's own method: 0x449a45 stores vtable 0x4C3098,
    whose slot 0 is `mov eax,0x4C7330; ret` at 0x45eaca, and
    CRuntimeClass 0x4C7330's first field points at the string
    **"CPtrArray"**. So 0x449a45 is `??0CPtrArray@@QAE@XZ` and 0x4C3098
    is `??_7CPtrArray@@6B@`. Independent confirmation at the only two
    call sites (hover.exe:0x453786 and 0x457b39, both inside MFC's own
    code): 0x453786 constructs the object with 0x449a45 and immediately
    calls 0x449b11 = `?SetSize@CPtrArray@@QAEXHH@Z` on the same `this` -
    one object, one class. Corrected one line in each of
    `tools/orig_library.csv` and `tools/orig_data.csv`; Implemented went
    back to 99.79% (938/940), Progress 90.30% -> 90.40%, and
    `compare.py --data` is still 354 variables / 0 issues. Practical
    consequence: **no member or global in this program is a
    `CDWordArray`** - the class's code and vtable are simply not in the
    binary - so a `CDWordArray` anywhere in our headers is wrong by
    construction, and the twin to disambiguate an array against is
    `CPtrArray`. Rule 251's parenthetical naming 0x4C3098 as
    `??_7CDWordArray@@6B@` was wrong and is corrected in place. General
    lesson: `orig_data.csv` and `orig_library.csv` are generated by the
    same fingerprinting rule 251 warns about, so a twin's *name* in
    those tables is exactly as untrustworthy as one from libmatch; only
    the vtable -> CRuntimeClass -> class-name-string chain settles it,
    and a library function that suddenly has no counterpart in our build
    is a good prompt to go and check one.

350. **Rule 324's retype is real and pays off in its own functions, but the
    shared-header toll cancels it: measured and not taken.** Retyping vtable
    slot 12 `Sub_408820`'s parameter from `DWORD` to `CCollider*` across the
    whole hierarchy (the two base declarations in gameobj.h, the four
    overrides in mazeobj.h, six definitions in gameobj.cpp/mazeobj.cpp and
    six call sites in gameobj.cpp/mazeobj.cpp/texload.cpp) does exactly what
    rule 324 predicts: in `CWall::Sub_408820` each walk loop's
    `mov ecx,[ebp+8] / add ecx,0x44 / mov [ebp-0x24],ecx / mov eax,[ebp-0x24]
    / cmp [eax],0` collapses to hover.exe's `mov ecx,[ebp+8] /
    cmp [ecx+0x44],0`, and the stack slot goes with it. The six slot-12
    bodies gain +63 original bytes (+0.028 pp byte-weighted):
    `CPlayer::Sub_408820` 37.97% -> 50.43%, `CPodObj::Sub_408820`
    66.13% -> 77.42%, `CTempWallPod::Sub_408820` 61.64% -> 72.60%,
    `CFloorObj::Sub_408820` 43.90% -> 45.97%. But `CCollider` lives in
    texload.h, which is the header that includes gameobj.h's classes, so
    gameobj.h needs a `class CCollider;` forward declaration - and a control
    that adds *only* that declaration and changes nothing else costs -94
    original bytes (-0.041 pp, 81.86% -> 81.82%) of pure register-allocation
    churn in translation units that never mention the type
    (`CSub413F10::DoDataExchange` 100% -> 84.62%, `CSub403980::Sub_403A60`
    38.32% -> 31.89%, against `CSub424720::OnInitDialog` 88.03% -> 100%).
    Net 81.86% -> 81.85% byte-weighted (Accuracy 90.58% -> 90.62%), so the
    change was reverted. This is the same toll the velocity-parameter wave
    measured on hover.h; it is charged per *type-table entry*, not per line:
    placing the declaration at the top of gameobj.h, next to `class CPlayer;`
    just above `CGameObj`, or writing no declaration at all and spelling the
    parameter `class CCollider*` inside the signature all produce
    byte-identical output. The retype itself is sound - if a later change
    needs `CCollider` visible in gameobj.h for another reason, take this one
    with it, because the toll is then already paid.

351. **Correction to rule 324: removing the hoisted address temp makes
    `CWall::Sub_408820` *worse*, because our frame is two slots too deep at
    the top, not one.** 35.52% -> 33.66%. hover.exe's frame is 0x1B8 and
    homes its named locals `[ebp-8] nCell`, `[ebp-0xC] this`,
    `[ebp-0x10] nDX`, keeping `nCol` and `nRow` deep at `[ebp-0x68]` and
    `[ebp-0x64]`. Ours puts `nCol` at `[ebp-8]` and `nRow` at `[ebp-0x10]`,
    which pushes `nCell`/`this`/`nDX` down to `-0xC`/`-0x14`/`-0x18` and
    every displacement below by 8 (frame 0x1C0 against 0x1B8). Deleting the
    address temp therefore took a slot out of the *middle* of the frame and
    moved the tail 4 further from the original instead of onto it. Rule 325's
    block-scoping of the per-branch temporaries is worth +0.4 once the
    address temp is gone (33.66% -> 34.04%, and it is what restores rule 325's
    predicted relative order below `ebp-0x24`), but nothing moves `nCol` and
    `nRow`: declaring them first, declaring them last, and block-scoping them
    into the degenerate case, both branches and the tail all give a
    byte-identical 34.04%. Those two slots are the whole remaining barrier
    for this function and, as rule 325 says, spill order is not addressable
    from the source.

352. **Corollary to rule 328: when a retype is forced on you, keep the
    surrounding statements in exactly their previous shape, because a
    neighbour's register-allocation flip is worth more than the retype.**
    The first, natural spelling of rule 350's change - parameter used
    directly in `CPlayer::Sub_408820` instead of its existing
    `CCollider* pCollider` local, temporaries left at function scope in
    `CWall::Sub_408820` - flipped two large neighbours in the same
    translation units: `CMerlinStatic::Serialize` (0x419E00, 1427 bytes,
    gameobj.cpp) swapped `esi`/`edi` throughout and fell 83.54% -> 49.79%,
    and `CFloorObj::Sub_42AD60` (0x40C620, 912 bytes, mazeobj.cpp) swapped
    `ebx`/`esi` and fell 91.00% -> 73.63%. Together that is more weighted
    bytes than the retype gained anywhere. Both were recovered by minimising
    the diff rather than by touching them: keeping the `pCollider` local and
    only changing what it is assigned from (`(CCollider*)dwA` -> the
    parameter) put 0x419E00 back to 76.58% with 0x408820 unchanged at
    50.43%, and block-scoping `nCol`/`nRow` put 0x40C620 back to 91.00%.
    Neither recovery changes an instruction in the function being edited, so
    when a big neighbour drops after a mechanical change, restore the
    neighbouring statements' exact previous shape before concluding the
    change itself is at fault.

353. **The call-target invariant: `tools/callcheck.py`, a second reproducible
      signal that is blind to register allocation and sensitive to
      semantics.** Match percentage is dominated by allocation, frame layout
      and scheduling, none of which can change behaviour, so a function can
      sit at 95% with a wrong call in it (rule 355 is exactly that) or at
      27% and be behaviourally perfect (rule 266). reccmp resolves call
      targets by name on BOTH sides, so per function we can compare the
      *multiset* of targets the original calls against the multiset ours
      calls. Position is ignored on purpose: a call that merely moved
      appears on both sides and cancels. Run it as
      `tools/compare.py --json /tmp/x.json && tools/callcheck.py /tmp/x.json`.
      Every difference is machine-checkable and re-checkable on every build,
      exactly like a byte mismatch, and it covers all 938 functions rather
      than the ones somebody has read. It found rule 354's real bug
      immediately. **Four noise classes must be normalised away or it is
      unusable** (all four are implemented): `call edi` where the original
      names an import, because VC hoists a repeatedly-called import into a
      register and which register is allocation; `call [ebp-0x1c]`, because
      our frames differ by design so the offset is meaningless; `<OFFSETn>`,
      an original target with no name in `tools/orig_library.csv` yet; and
      inlining depth, which is not filtered and must be read around - if the
      original inlines a callee, that callee's own calls appear in the
      caller on the original side and in a separate function on ours (all
      six `CXxxPod::Sub_42AD40` findings are mostly this). **Blind spots:**
      an indirect call is compared only by its vtable slot offset, a target
      mis-named in our tables compares wrong on both sides (and this project
      has had one, rule 349), and it sees nothing of arithmetic, constants
      or control flow. It complements the score; it does not replace it.

354. **Three of the ten pod handlers changed to `AddTail` in #166/#167 were
      wrong: the original adds to the HEAD of the player's pod list.**
      Found by rule 353's checker, verified positionally rather than by
      eye: hover.exe:0x42b285 is `push esi / mov ecx,[esi+0x88] /
      add ecx,0xd0 / call 0x44934b`, and 0x44934b is
      `CObList::AddHead`, sitting exactly where our
      `((CPlayer*)m_dwUnk88)->m_obList00D0.AddTail(this)` was, right after
      the `Sub_40D6E0(8,1,5,0)` sound call. Head in `CTempWallPod`
      (0x42b160), `CSmokeBombPod` (0x42a760) and `CJumpPod` (0x42aac0);
      tail is correct in `CSlowPod` (0x4226d0), `CSpeedPod` (0x421070) and
      `CInviciblePod` (0x4181e0); `CMapEraserPod` (0x422470) makes no list
      call at all. This reverses the iteration order of the player's pod
      list, so it is a real behavioural difference and it was a regression
      we introduced. 0x42a760 and 0x42aac0 66.0 -> 67.0, 0x42b160 65.0 ->
      65.7, Accuracy 90.59 -> 90.60. **Check the direction per handler
      before generalising a container fix across a family of functions.**

355. **`CWall::Sub_414D40` (0x414d40) binds its base call statically.**
      hover.exe:0x414d4a is `call 0x414a50` direct, where our unqualified
      `Sub_414A50(...)` dispatched through vtable slot 5. Spelling it
      `CGameObj::Sub_414A50(...)` forces the direct call and clears the
      rule 353 flag at no measurable cost (byte-weighted 81.89% either way;
      the function is 34 bytes, so its 58.8 -> 56.2 is about one byte).
      No class overrides `Sub_414A50` today, so behaviour is unchanged now
      - but the original is statically bound and ours was not, which would
      diverge the moment an override appeared.

356. **Known and deliberately not taken: seven emptiness tests call
      `strcmp` where hover.exe calls `__mbscmp`.** Six in
      `CMerlinStatic::Serialize` (gameobj.cpp) and one in
      `CMerlinDynamic::Serialize` (mazeobj.cpp); the original's six calls
      at 0x419e00 all go to 0x43e87e, which rule 253 established is
      `__mbscmp`. Switching them makes the calls match, confirmed in the
      diff, but the surrounding register allocation rerolls and
      `CMerlinStatic::Serialize` falls 83.5% -> 69.4%, byte-weighted
      81.89% -> 81.75%. **It cannot change behaviour**: every site compares
      against `""`, and `strcmp(s,"")` and `_mbscmp(s,"")` are zero for
      exactly the same inputs. The spelling is not the problem - a bare
      `extern "C"` prototype instead of `<mbstring.h>` scores identically,
      so this is not rule 346's header toll. Left as it is, flagged
      permanently by rule 353's checker, and worth retrying when the
      allocation around those loops is understood.

357. **What a clean-ish `callcheck` run looks like, so the next person does
      not re-triage from scratch.** At master 302e391 it reported 37
      functions with a comparable call-target difference. The bulk decompose
      into: inlining depth (an inlined base constructor or accessor moves
      its calls into the caller - `CHoverApp::CHoverApp`,
      `CMainFrame::Construct`, `CBumperDoc::CBumperDoc`, the
      `CMerlin*::Serialize` `CArchive::FillBuffer` entries, the pod
      handlers' `CMapStringToOb::Lookup` / `wsprintfA` / `CPodObj::CPodObj`);
      FP helper naming gaps (`__CIsqrt`, `__adj_fdiv_m64`,
      `__adj_fdivr_m32i`, `__adj_fptan`, `__isctype`), where the original's
      side is an unnamed target; and genuine leads. **The seven leads this
      run produced were then chased and only three were real** - that
      correction matters more than the list did. REAL and fixed: 0x433ee0
      was `OnInitDialog` wired as `OnCancel`, and `Hover_sub_43BAD0` called
      `_atoi` for `_atol` (both rule 359); `CBumperView::Sub_402250` was
      missing a `GdiFlush` (rule 360). FALSE POSITIVES, all one mechanism,
      now filtered by rule 358: the missing `RegOpenKeyExA`/`RegCloseKey` in
      `CSub424BD0::Sub_424C80`, the missing `CreateDIBSection` in
      `CSub423030::Sub_411D80`, the three extra Win32 calls in
      `CBumperView::OnDraw`, `GlobalAlloc` vs `GlobalLock` in
      `Hover_sub_438D60`, and `GlobalUnlock` vs
      `FindResourceA`/`LoadResource` in `Hover_sub_43B150`. **Hit rate three
      in eight: every finding is a lead to verify in the disassembly, never
      a defect on its own.**

358. **Two more noise classes the checker must filter, both the same
      mechanism: an import called through a register.** VC caches a
      repeatedly-called import's address in a callee-saved register, and
      which import gets that treatment differs between the two builds. It
      appears two ways. (a) `call edi` on one side against the named import
      on the other: cancel one named import per call-through-register on
      the opposite side. (b) `mov esi, dword ptr [KERNEL32.dll::GlobalAlloc]`
      on one side - the *load* of the address, which never appears as a
      named call at all - against `call dword ptr [->...GlobalAlloc]` on the
      other: scan the diff for those loads and cancel the matching named
      call opposite. Both are implemented. Together they took the flagged
      count from 36 to 25 and removed five of the seven leads above. **A
      named import reported as missing is the least trustworthy finding the
      checker produces**; look for a register load or a register call before
      believing it.

359. **Two functions the checker took to byte-exact, both already above
      85% matched.** `CSub424BD0`'s handler at 0x433ee0 is `OnInitDialog`,
      not `OnCancel`: hover.exe calls `CDialog::OnInitDialog` (0x44a9b0,
      confirmed by `libmatch.py --show`) and returns TRUE in eax, which a
      void `OnCancel` never would. Ours declared it `virtual void
      OnCancel()`, so it sat in the wrong vtable slot - the high-score
      dialog never ran its setup and ran it on cancel instead, a real
      behavioural bug. 85.71% -> 100.00%. `Hover_sub_43BAD0` calls `_atol`
      (0x43fec9), not `_atoi`: identical results on 32-bit, but it is the
      original's entry point. 95.65% -> 100.00%. This is rule 353's whole
      argument in two data points - the score could not see either.

360. **`CBumperView::Sub_402250` flushes the GDI batch after its black
      fill.** hover.exe:0x4024e3 calls `GdiFlush` immediately after the
      `PatBlt`; we did not call it at all. Everything after that point
      writes the DIB bits directly, so an unflushed fill can land on top of
      the frame drawn after it. Same class as the flush restored in #139.
      59.50% -> 59.81%.
