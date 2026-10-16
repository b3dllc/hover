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

361. **`CBumperView::OnContextMenuMsg` called `::WinHelp` with the wrong
      command for six of its seven topics.** hover.exe:0x429165 is a
      shared tail all seven `if`/`else if` branches jump into after
      pushing their branch-specific `dwData`; it pushes the `uCommand`
      argument as a literal `8` (`HELP_CONTEXTPOPUP`) unconditionally, so
      every branch - not just the map one - opens the popup-style help
      window rather than switching the help file's topic index. We had
      `HELP_CONTEXT` (`1`) on six of the seven (waypoints, pods, score, the
      three gauges); only the map branch already matched. Found by
      opcheck.py's bundle-11 lead list flagging this function only for an
      unrelated `CMenu::\`vftable'` reference (rule 362's noise class); the
      `HELP_CONTEXT`/`HELP_CONTEXTPOPUP` mismatch had to be found by
      reading `tools/disasm.py 0x428d90` positionally and noticing the
      original's seven `::WinHelp` call sites collapse to *one* physical
      call (tail-merged), which only works if every branch pushes the same
      `uCommand` - a mechanical hint that source with per-branch constants
      differing only in a shared tail is worth rechecking literally.
      49.4% -> 50.5% (this function), byte-weighted 81.89% -> 81.90%.

362. **Session covering opcheck.py leads on eight `src/bumpview.cpp`
      functions (bundle 11): one real fix (rule 361), seven noise, hit
      rate one in eight.** All seven noise verdicts reduce to classes
      already named in rules 353/358 plus two siblings worth naming
      explicitly for the next opcheck run:
      - **Unnamed `<OFFSETn>` vtable, both sides write the same address.**
        `CBumperView::OnContextMenuMsg` (0x428d90): our inlined
        `CMenu::CMenu()` writes `CObject::\`vftable'` (the inlined base
        ctor) then `CMenu::\`vftable'\`` (its own), exactly mirroring
        hover.exe:0x428e3b/0x428e49 - but the original's second write
        resolves to `<OFFSET9>` because `tools/orig_data.csv` has no entry
        for CMenu's vtable (only its `CRuntimeClass` at 0x4c6a00 is
        named). Not fixed (nothing to fix, it's a naming gap in the
        tooling, not the source).
      - **A live register beats a hoisted address, in either direction.**
        `CSub406090::Sub_406090` (0x406090) and `CSub422F50::Sub_407160`
        (0x407160): a value just written to a global (`g_dwUnk_4A1D64`,
        `g_dwUnk_4A4794`) is still live in a register when the next
        statement needs it, so one side rereads it from memory and the
        other reuses the register - a pure CSE count difference
        (opcheck's own "3 reads vs 4" note), confirmed by re-tracing every
        instance positionally rather than trusting the raw counts.
      - **Hoisted address splits one displacement into two, both
        directions, repeatedly.** `CBumperView::Sub_4184B0` (0x4184b0,
        the `pZone->m_rcUnk210` RECT copy), `CSub406090::Sub_406090`
        (0x406090, the `pCam->m_dwUnk48`/`m_dwUnk4C` camera-position
        pairs, both directions in the same function), `CBumperView::
        CBumperView` (0x41e730, the `m_dwUnk0034`/`m_dwUnk00E4`/
        `m_dwUnk0518..0524` cluster hoisted through `this+0x518`) and
        `CBumperView::Sub_402250` (0x402250, the `m_obj0150` viewport RECT
        and the `pdwDest`/pdwCam D4/D8 pair, hoisted through whichever
        base happened to already be live). Every instance was checked
        member-by-member and value-by-value against the source before
        being written off - same fields, same values, only the choice of
        which pointer to keep live differs.
      - **`add`/`shl` vs `lea` is the same arithmetic in a different
        instruction.** A scale-by-4 index (`shl reg,2` vs `lea reg,
        [reg*4]`, seen in 0x4184b0, 0x406090 and 0x405ce0) and a
        sub-object's base address (`add ecx,0x1ac`/`0xb4` vs
        `lea ecx,[eax+0x1ac]`/`[eax+0xb4]`, seen in 0x402250 and 0x41e730)
        both fold into one instruction on one side and stay two on the
        other; opcheck's own classifier even splits identical arithmetic
        into different keys (`disp+N` for the `lea` form, `imm:shl:N`/
        `imm:add:N` for the separate-instruction form) because it reads
        addressing mode, not value.
      - **`ebp` reused as a plain GPR breaks the frame-register
        heuristic.** `CSub406090::Sub_405CE0` (0x405ce0) has no
        `mov ebp,esp` at all (FPO, `ebp` is just another callee-saved
        register); opcheck's `FRAME` regex excludes *any* operand
        containing `ebp` or `esp` on the theory that it is frame-relative,
        so whichever side happens to carry `this`/a node pointer in `ebp`
        has its member-offset accesses (`m_dwUnk30`, `m_dwUnk34`,
        `m_pUnk40`, every `SPANLINK` field write) silently dropped from
        that side's tally, and the matching `add ebp,0x18` (`sizeof
        SPANLINK`, pointer increment) is dropped too by the same
        `mn in ("add","sub","lea") and ops[0] in ("esp","ebp")` guard in
        `props()`. Every presence finding on this function - eleven of
        them - is this one mechanism, confirmed by walking the two
        register-shuffled sides side by side and checking each field
        offset and stored value individually. **New noise class for
        opcheck.py**, not yet filtered by the tool.
      - **`cmp x,0` vs `test x,x`, confirmed as the class the method
        section already names.** `CSub406090::Sub_405CE0` (0x405ce0),
        `CSub422F50::Sub_407160` (0x407160, `theApp.m_dwUnk01E0`) and
        `CBumperView::Sub_41ECE0` (0x41ece0, the state-dispatch jump
        table's implicit `<= 7` range check, which only one side needs at
        all).
      - **Switch lowered as a jump table vs. sequential compares, state
        constants verified correct.** `CBumperView::Sub_41ECE0`
        (0x41ece0): hover.exe builds *two* 8-slot jump tables here even
        though the function is `#pragma optimize("",off)` per rule 49 (a
        dense 0-7 switch gets a table regardless of optimization level);
        our build's own -O0 falls back to `cmp`/`je` chains instead,
        which is why we have `cmp dwState,1`/`,5` (first switch) and the
        checker sees no matching compares on the original side at all.
        Decoded both original jump tables byte-for-byte (file offsets
        0x1e36d and 0x1e479 in hover.exe) and matched every one of the
        16 slots to its handler by reading the target code: first switch
        slot 0 -> `m_dwUnk00A4`/`A8` (`ptArgs`), slot 1 -> `theApp+0x1c4`
        (`nStateArg`), slot 5 -> `m_dwUnk00B0` (`nScaleArg`), 2/3/4/6/7 ->
        the shared no-op exit; second switch slots 0-7 -> `Sub_4184B0`,
        `Sub_413C10`, `bContinue=FALSE`, `Sub_402250`, `Sub_410510`,
        `Sub_427A80`, `Sub_427BB0`, `Sub_427CC0` in that order. All match
        the source exactly - **the worker-thread state machine has no
        swapped constants this time**, despite the earlier history that
        made this worth checking case by case rather than trusting the
        60.7% score. The `data:theApp x2` finding on the same function is
        the address-materialization count noise already documented by
        rule 92 (this function's `#pragma optimize("",off)` makes every
        `HoverWorkerApp()` call re-derive `&theApp` into its own stack
        temp, and exactly how many separate accesses get that treatment
        is sensitive to inconsequential source shape).

363. **`CHoverApp::CHoverApp` (0x41c100): the level-parameter table's
      `m_dwUnk38` accumulates across levels, it does not self-reference.**
      Found via `opcheck.py`'s `disp-0x1` flag. hover.exe:0x41c3d0 is
      `lea eax,[esi-1]` (i.e. `i-1`), and that register alone feeds the
      *third* read in the statement's address chain (0x41c3ec-0x41c407:
      the first two reads, `m_dwUnk30` and `m_dwUnk44`, are indexed by
      `esi`/`i` itself). So the original is
      `LVLPARAM(i).m_dwUnk38 = LVLPARAM(i).m_dwUnk30 * LVLPARAM(i).m_dwUnk44
      + LVLPARAM(i - 1).m_dwUnk38;` - a running total seeded at level 0 and
      carried forward - where ours re-read the current level's own
      (not-yet-final) `m_dwUnk38`. Fixing the index clears the `disp-0x1`
      flag and moves the function 67.8% -> 71.8%; byte-weighted fell
      slightly (81.89% -> 81.82%) but this is a behavioural bug (wrong
      accumulated value for every level past the first), kept regardless.
      The function's other `opcheck` flags (`data:g_lvlParams x9`,
      `disp+0x34/38/3c/40/44/48/4c/50/54`, `+0x20/+0x3dc/+0x3f4/+0x3f8`,
      `imm:and:0x80000000`, our extra `+0x18/+0x1c/+0x3fc`) are all
      addressing-shape noise, not behaviour: the original hoists
      `&LVLPARAM(i)` into a register once per field before each store
      (`lea edi,[ecx*8+g_lvlParams]` then `mov [edi+0x44],ecx`) while our
      compile folds the same address into one scaled-index+offset operand
      (`mov [ecx*8+g_lvlParams+0x44],eax`); rule 129 already fixed one
      instance of this same shape (the `m_dwUnk38` store itself) by writing
      it out instead of `+=`, but VC 2.1's choice of which stores get the
      register hoist and which don't is scheduling-driven, not
      source-spelling-driven (rules 17-18) - not chased further here.
      `disp+0x3dc`/`+0x20` (original) vs our `+0x3fc` is the classic
      "hoisted address splits one displacement into two" noise (rule 337-
      class): the original computes `this+0x3dc` as a base, stores
      `m_pfnChangeDisplaySettings` there, then reuses that same base
      register with `+0x20` for `m_dwUnk03FC` (`this+0x3dc+0x20 ==
      this+0x3fc`), where we compute `this+0x3fc` directly.
      `imm:and:0x80000000` is a redundant-mask canonicalisation the
      original didn't fold (`(x&0x80000000)>>31` vs a bare `shr eax,0x1f`,
      same result) - same family as rules 6/9/11/13/18's 2.1-vs-our-
      compiler drift negatives.

364. **`Hover_sub_43B880` (0x43b880): `isalnum`/`isspace` are real CRT
      function calls in the original, not the `_MBCS` ctype.h macro.**
      hover.exe:0x43b896 and 0x43fe6e/0x43fe99 (`libmatch.py` can't name
      them, but the shape is unambiguous: `push byte; call; add esp,4`,
      unconditionally, no preceding test) call the functions directly, with
      no runtime check at all. Our `_MBCS` build's `<ctype.h>` expands
      `isalnum(c)`/`isspace(c)` inline as
      `__mb_cur_max>1 ? __isctype(c,mask) : (__pctype[c]&mask)`, adding a
      `___mb_cur_max`/`__pctype`/`__isctype` sequence hover.exe never has.
      Parenthesizing the call - `(isalnum)(*p)`, `(isspace)(*p)` - is the
      standard idiom to defeat a function-like macro and call the
      underlying declared CRT function instead; it reproduces the
      original's shape exactly (no behavioural change - both paths
      classify the same byte the same way for a single-byte locale).
      51.2% -> 88.2%; Accuracy 90.61% -> 90.66%, byte-weighted 81.89% ->
      81.94%. The residual `imm:cmp:0x0` flag is the `cmp x,0`/`test` idiom
      noise class.

365. **Three more `opcheck`/`callcheck` leads chased and found NOISE - all
      fold into rule 337's "diff the multisets" method, no new classes
      needed.** (a) `Hover_sub_41F540` (0x41f540): `imm:cmp:0x461d18/
      0x461d60/0x461ea0` are the three tail loops' one-past-end bounds
      (`g_adwUnk_4609A0 + bias + 6*0x338`, confirmed algebraically -
      0x4609a0+0x28+6*0x338 == 0x461d18 exactly, etc.) landing on whatever
      the *original's* linker happened to place next to that array; ours
      lands on different neighbours (`g_HBegParams+16`, `theApp+40/360`)
      because `g_adwUnk_4609A0` is a free global here and its neighbours
      are linker-order noise, not something a source change to this
      function can fix (moving the array into `theApp` to match would be a
      header change rippling into mainfrm.cpp/bumpview.cpp/texture.cpp,
      out of a single function's scope). `disp+0xa0`/`-0x190` vs our
      `+0x1a8`/`+0x90`/`-0x1a0` are the loop pointer's increment scheduled
      on the other side of a dependent `lea` (`p_old+0x1a8 == p_new-0x190`
      algebraically); this is the exact residual rules 306-312 already
      measured and left uncommitted as a cross-function register cascade
      (rule 311) - reconfirmed, not re-attempted. (b) `Hover_sub_439E80`
      (0x439e80): `imm:and:0xffffffca` vs our `imm:and:0xca` is a register-
      width choice on the same branchless `(flags&8)?'W':'!'` trick - the
      original does it in `eax` (`sbb eax,eax`/`and eax,0xffffffca`/
      `add eax,0x57`), ours in `al` (`sbb al,al`/`and al,0xca`/`add al,
      0x57`) followed by `movsx edx,al` before the push; both push the
      identical value. The `data:g_pUnk_4B6248` flag on the same function
      is a count-only difference (absent under `--presence`), which
      `opcheck.py`'s own docstring already calls CSE/hoisting noise. (c)
      `Hover_sub_43D3B0` (0x43d3b0): our extra `imm:add:0x17e` is the
      hoisted-address noise class from the other side - the original folds
      the pointer materialisation into a `lea eax,[ecx+0x17e]` and reuses
      `eax` bare for the nChannels clamp's two conditional stores, we
      re-derive the same pointer with a separate `add ecx,0x17e` after an
      offset-form read; the clamp's stored values (1, 2) are identical
      either way. No source changes taken for any of (a)-(c).

366. **`CSub40B800::Sub_40B800` (0x40b800) re-read a global its own local
      copy already held.** hover.exe:0x40b892 loads `g_pUnk_4C56DC` into
      edx once and keeps it live in that register (untouched by the
      inlined `strcpy`/`strcat` and the intervening string-literal
      operations) all the way to the `Hover_sub_43B150` call at 0x40b95a
      (`push edx`) - it never touches the global a second time. We passed
      `g_pUnk_4C56DC` again as that call's first argument instead of the
      already-captured `playParams.pSession`. `Hover_sub_43B150` never
      touches `this` or any global (rule 353's callcheck confirmed no
      side effects), so this cannot change behaviour today, but it is a
      real difference in what a program COULD observe (a concurrent
      change to the WaveMix session pointer between the two reads would
      diverge) and it is what the original does. Fixed by passing
      `playParams.pSession`; `tools/opcheck.py`'s `data:g_pUnk_4C56DC x2`
      finding drops to x1 - the remaining one is `if (playParams.pSession
      == NULL)` recompiling as `cmp [g_pUnk_4C56DC],0` instead of a
      register test, a dataflow/CSE choice on already-correct source, left
      as noise. Function match unchanged (74.7%); Accuracy 90.61% ->
      90.61%, byte-weighted 81.89% -> 81.89% (no measurable change, kept
      per rule for a faithfulness fix that does not cost byte-weighted).

367. **`CFlag::Sub_42AD40` (0x41ae40) needed a named local for a gauge
      pointer the original computes but never uses.** hover.exe:0x41b026
      stores `1` and `lCount` into `pView->m_obj02B8`'s two fields via
      plain `[eax+0x2b8]`/`[eax+0x2c0]` displacements, exactly like our
      source, and only afterward materialises `&pView->m_obj02B8` into eax
      (`add eax, 0x2b8`) even though the very next instruction overwrites
      eax with an unrelated global load - the pointer is dead on arrival.
      No behaviour differs either way. Declaring `CSub422BD0* pGauge2 =
      &pView->m_obj02B8;` and writing the two fields through it - matching
      the style of the sibling `CSub422C30* pGauge = &pView->m_obj0430;`
      block immediately above - reproduces the vestigial computation and
      clears the `disp+0x2b8` flag outright: 94.40% -> 95.94%, Accuracy
      90.61% -> 90.67%, byte-weighted 81.89% -> 81.90%. Read this as a
      companion to rule 337: naming an address-of-member local even when
      its only later use is the raw-offset form the compiler already
      picks can still matter, because VC 2.1 does not always eliminate the
      now-unused pointer materialisation.

368. **A ninth noise class for `tools/opcheck.py`/`tools/callcheck.py`:
      when our own function is fully inlined at every call site, the
      marker for its would-be out-of-line copy binds to whatever
      unrelated function's code the linker happened to place at that
      address, and both checkers then compare the wrong pair of
      functions.** `CSub42AE90::CSub42AE90(WORD, WORD)` (hover.exe:0x42ae90,
      22 bytes: `mov ax,[esp+8]` / `mov dx,[esp+4]` / two word stores) is a
      two-instruction inline the comment above it says the compiler "still
      emits ... out-of-line" - it no longer does in this build.
      `build/hover.map` has no `??0CSub42AE90@@QAE@GG@Z` symbol at all
      (only the `(double)` constructor's `??0CSub42AE90@@QAE@N@Z` at
      0x42a8b0), and `compare.py --verbose 0x42ae90` prints a `[WARNING]
      Match (42a9d0, 42a8b0) collides with previous staged match` and then
      diffs hover.exe's real (WORD,WORD) body against OUR (double) ctor's
      body (`mazeobj.cpp:3390`, `dbl * 65536.0`, `__ftol`) - hence the
      apparent missing `65536.0 (FLOAT)` and `__ftol` call, and the 13.3%
      score. The (WORD,WORD) ctor's actual logic is correct wherever it IS
      inlined (`m_wFrac`/`m_wInt` at +0x00/+0x02, matching the original's
      `[ecx]`/`[ecx+2]` stores in that exact order) - there is nothing to
      fix in the source, and nothing short of an unfaithful
      no-inline hack would force MSVC 2.1 to keep a two-instruction
      function out of line at every one of its call sites. Left as is;
      re-check after any edit that touches this file's inlining balance
      (rule 328).

369. **`CMerlinTexture::Serialize` (0x412980) leads from this run
      reproduce rules 328/334/335/336, not new bugs.** callcheck/opcheck
      flagged `add edi, 0x14` (original, missing from ours) at 0x412b48 -
      the fifth of rule 336's five inlined-`ReadWord` mip-header reads,
      already documented as a pure scheduling difference (18 vs 19
      instructions per read, "the remaining gap ... is register
      allocation, not structure"); `lea ebp,[eax+ecx*8]`/`shl ecx,3`
      around 0x412d16-0x412d30 - rule 334's already-applied pPts-before-
      global-update fix (current 42.11% match IS rule 334's landed value);
      and three `cmp reg, 0` where the original has `test reg,reg` - the
      checker's own documented cmp/test idiom class. Rule 335 already
      tried and rejected three targeted fixes in this exact area (all cost
      more to the register cascade than they gained); no new attempt was
      made here. Verdict NOISE for all of it, folding this run's findings
      back into the existing audit.

370. **Three more settled `opcheck` leads, folded in so they are not
      re-triaged.** (a) `CPlayer::Sub_40B0B0` (0x40b0b0): hover.exe
      materialises the literal `0x100` into a register (`mov ecx,0x100` /
      `mov di,0x100`) before two `labs(dx) </== 0x100 || labs(dy) </==
      0x100` checks that our code compiles as direct `cmp reg,0x100`
      immediates; both sides implement the exact same two-branch OR test
      (`jl`/`je` to the same target on both sides) and the register value
      is never stored anywhere - a register-vs-immediate materialisation
      choice, not a different comparison or a different field. (b)
      `CPlayer::Sub_40ADE0` (0x40ade0): the extra `imm:add:0x28` /
      missing `disp+0x28` is `tools/opcheck.py`'s own `add reg,N` filter
      exempting any destination named `ebp` (meant to skip stack-frame
      adjustments) firing on hover.exe:0x40af84's `add ebp,0x28`, which
      here is an ordinary pointer computation, not a frame op - the
      original recomputes `m_dwUnk0084+0x28` fresh via `[eax+0x28]`
      addressing after the `Hover_sub_41B830` call instead of reusing the
      cached pointer, and since that helper is a pure function of its two
      arguments (no access to `this` or any global), the value cannot
      differ either way. A checker limitation, not a bug; worth teaching
      `opcheck.py` to key the exemption off the source operand (`esp`/
      `ebp`) rather than the destination if this pattern recurs. (c)
      `CRobotPlayer::Sub_40BC10` (0x40bc10): the missing
      `?classCBeacon@CBeacon@@2UCRuntimeClass@@A` reference is
      `CBeacon::GetRuntimeClass()` (MFC's `DECLARE_DYNAMIC`/
      `IMPLEMENT_DYNAMIC` boilerplate, `mov eax,&classCBeacon`/`ret`, no
      stack-arg pop), laid out immediately after Sub_40BC10's `ret 4` in
      the original binary purely by link order; Sub_40BC10 itself (a
      10-entry ring buffer: store, increment, wrap at 10) is complete and
      correct, and one of its own callers does pass a `CBeacon*`
      (texload.cpp:1794), which is presumably why the two ended up
      adjacent upstream. Moving code between translation units to chase a
      linker-order coincidence is out of scope here. All three verdicts:
      NOISE.

371. **`CSub402E00::Sub_406740` (0x406740) wrote garbage-tinted texture
      fills: a real bug, found chasing rule 358's checkers on the texheap
      bundle.** The flat-fill path's two hand-written loops -
      `dwPat = (DWORD)bPix * 0x01010101;` then a dword loop and a byte
      loop both reading `dwPat` - made VC 2.1 build the replicated byte
      via a Horner shift-add chain that reused `eax` as a second copy of
      `bPix` *without clearing it first*: `eax` still held the upper 24
      bits of the `pView` pointer from an earlier `mov eax,edi`, only its
      low byte got overwritten with `bPix`, and three `shl 8; add eax`
      steps folded those pointer bits straight into the fill pattern
      (confirmed with `objdump` on the recompiled binary:
      `mov eax,edi` / `mov al,[ecx+ebx*4]` / `xor ecx,ecx` / `mov cl,al` /
      `shl ecx,8` / `add ecx,eax` x3). Every `rep stosd` of the dword
      portion therefore wrote pointer-tinted garbage instead of a pure
      `bPix,bPix,bPix,bPix` dword - this is the on-screen texture-mapping
      corruption the operand checkers exist to catch. hover.exe never
      does this: it builds the same replicated byte with `mov ch,bl` /
      `mov cl,bl` / `mov eax,ecx` / `shl eax,0x10` / `mov ax,cx`, using
      only the byte value, no pointer register involved. Fix: replace
      both loops with `memset(pDst, bPix, dwFill);`, the same
      nonzero-byte/runtime-length idiom already used at hover.cpp:2555.
      VC then inlines the correct word-doubling replicate and a clean
      `rep stosd` + `rep stosb`, matching hover.exe's approach with no
      poisoned register. Fixing this one block also cleared nearly every
      other opcheck key rule 358's run reported for the function
      (`g_dwUnk_461CFC`, `g_dwUnk_4A1D70`, `g_dwUnk_4A4534` x3,
      `g_dwUnk_453C`, `g_dwUnk_4A455C` x4, `shl 8` x3, `and 3`, `shr 2`):
      they were all downstream realignment artifacts of this single
      miscompiled block, not independent bugs - a function-sized version
      of rule 337's audit technique catching a real defect instead of
      noise. 0x406740: match 44.0% -> 36.8% (own-function score falls
      because the corrected, shorter codegen reschedules registers for
      the rest of the function); Accuracy 90.61% -> 90.59%,
      byte-weighted 81.89% -> 81.85%. Kept regardless per rule 353's own
      principle: a behavioural fix stands even when the percentage
      falls. One small residual key remains and is noise (next rule).

372. **Three more opcheck leads chased on the texheap bundle, all noise -
      recorded so nobody re-triages them.** `CSub402E00::Sub_402E00`
      (0x402e00): the `data:g_lineUnk_4A4498` / `imm:cmp:0x4a40a8` pair is
      a computed one-past-the-arena pointer
      (`g_abUnk_4A1D80 + (0x4A40A8 - 0x4A1D80)`) whose *value* correctly
      bounds-checks our own array in either binary; the disassembler just
      labels our build's copy with whatever global happens to sit at that
      address, because roughly 0x3F0 bytes of the original's globals
      between the arena's end and `g_lineUnk_4A4498` are not yet declared
      in texheap.cpp, so our linker packs the two adjacent. Rewriting the
      comparison as a hardcoded literal would make the disassembly match
      but would be actively wrong - our own link address for the arena is
      not guaranteed to be hover.exe's 0x4A1D80. Its `disp+0xb4`/`disp+0xb8`
      is rule 353's hoisted-address-split class (`lea ecx,[edi+0xac]` then
      `[ecx+8]`/`[ecx+0xc]` on one side, `[edi+0xb4]`/`[edi+0xb8]` directly
      on the other - the same field). Its `imm:shl:0x2` is a `lea
      eax,[ecx*4]` (original) vs `shl ebx,2` (ours) instruction-selection
      choice for the same `pObj->m_dwUnk90 * 0x14` multiply, driven by
      whether the multiplicand is still live in a register or reloaded
      from the member - same value either way. `CSub402E00::Sub_402B10`
      (0x402b10): `data:g_dwUnk_4A1D74` (x1) is a dead-store elimination -
      three of its four assignments in source are unconditionally
      overwritten with no intervening read or call, so VC drops all but
      the last observable one; `data:g_dwUnk_4A4A44` (x10) is CSE - the
      value used in every `HOVER_SINLUT(...) * g_dwUnk_4A4A44` multiply
      and the closing `Sub_403850` call is cached in a stack slot
      (`[esp+0x10]`) on our side and re-read from the global nine times on
      hover.exe's, confirmed by `objdump` (`imul eax,[esp+0x10]` x9 vs
      `imul eax,[0x4a4a44]` x9 in the original); `data:g_pUnk_4A1D68` (x3
      extra on our side) is the opposite direction - our build
      rematerialises the `pView` pointer from the global three times
      instead of keeping it live across the whole function the way
      hover.exe's `esi` does. None of the three can change behaviour:
      nothing writes any of these three globals between the relevant
      reads. `CSub402E00::Sub_402630` (0x402630): `data:g_dwUnk_4A4564` /
      `disp+0xc` is the pre-existing, deliberate omission already
      commented in source ("the original carries a dead cmp of m_dwUnk0C
      here"); `imm:cmp:0x0` (x3) is hover.exe keeping `bCovered` live in a
      register and testing it with `test eax,eax` (confirmed at
      hover.exe:0x402711) against our build spilling it to the stack and
      using `cmp dword ptr[slot],0` at the three `if (bCovered) return;`
      sites - same flag, different home. `disp+0x60` and `disp+0x64` (x6
      each) in the same function ARE a real CSE gap - hover.exe loads
      `pObj->m_dwUnk60`/`m_dwUnk64` once per branch group (2 and 3 loads
      total) and reuses the register across up to four sibling
      `Sub_403980` calls, where our build reloads the member at every
      individual call site (8 and 9 raw loads) - but caching each into a
      local `DWORD` before its branch group, tried and measured, cleared
      both opcheck keys yet *cost* accuracy (0x402630 34.7% -> 23.9%,
      byte-weighted 81.85% -> 81.78%) for a change that cannot alter
      behaviour (hover.exe's own caching proves `Sub_403980` never
      mutates these fields through `pObj`). Reverted per rule 353's other
      principle: a faithfulness-only change is kept only if byte-weighted
      does not fall. Left as direct member accesses.

373. **opcheck's frame-register filter has the same kind of blind spot
      callcheck's known-noise list already covers for calls (rule 353):
      it goes by register *name*, not role.** `FRAME = r"\b(ebp|esp)\b"`
      drops any `[reg+N]` operand whose base is spelled `ebp` or `esp`,
      on the reasoning that our stack frame differs from hover.exe's by
      design. That reasoning only holds when `ebp` is actually the frame
      pointer (a preceding `mov ebp,esp`). VC 2.1's /O2 frequently omits
      the frame pointer and allocates `ebp` as a fifth general-purpose
      register instead; when our build happens to put an object pointer
      there, a perfectly matching member access (confirmed against
      hover.exe instruction-for-instruction, e.g. `Sub_402B10`'s two
      `pView->m_dwUnk4C` reads and one `pView->m_dwUnk28` read, rule 372)
      gets silently dropped from *our* side only, manufacturing a
      one-sided "original has, we do not" that looks exactly like a real
      missing field. Before trusting a `disp+N` presence finding, check
      whether the base register on the flagged side is `ebp`/`esp` acting
      as a GP register (no `mov ebp,esp` in the function) rather than a
      real frame reference, the same way rule 358 says to check for a
      register-cached import before trusting a missing call.

374. **Open lead, not chased: `CSub402E00::Sub_404030` (0x404030) calls
      `Sub_402630` and `Sub_402990` directly; hover.exe's does not, and
      calls `CSub403980::Sub_406D00` ten more times than we do.** Found
      by rule 353's checker while chasing the texheap bundle (0x402630,
      0x402990) but the function itself is outside that bundle, so it
      was not fixed here. `Sub_404030` is the recursive span-list walker
      that dispatches to `Sub_402630`/`Sub_402990` (occlusion clip) or
      `m_pUnk00->Sub_406D00` (pass-through) per node; the net call-target
      difference is consistent with rule 353's inlining-depth noise class
      (hover.exe inlining `Sub_402630`/`Sub_402990` into `Sub_404030`
      would remove those calls and, if their bodies also reach
      `Sub_406D00` along some path, add exactly the kind of extra
      `Sub_406D00` calls seen here) but this was not verified
      positionally against hover.exe:0x404030 - only the multiset diff is
      established. Worth a dedicated pass with `tools/disasm.py 0x404030`
      before assuming either inlining or a genuine wrong-callee bug.

375. **Five `opcheck` leads on `src/gameobj.cpp`'s `CPlayer`/`CHumanPlayer`
      functions (bundle 12), all NOISE, hit rate zero in five - folded in
      so nobody re-triages them.** `callcheck.py` reported nothing new for
      any of the five.
      - **`CPlayer::Sub_41A6B0` (0x41a6b0, wall-bounce angle lookup):**
        `disp+0x20 x2` (original) vs our `disp+0x1c x2` is the hoisted-
        address-split class from the opposite direction of the usual case
        - our code holds `pSeg = &pWall->m_obj04` live and reads
        `pSeg->m_dwUnk1C` (`[pSeg+0x1c]`), hover.exe:0x41a734/0x41a85d
        instead reuses the still-live raw `pWall` pointer and reads
        `[pWall+0x20]` directly; `pWall+0x20 == (pWall+4)+0x1c` algebraically
        (`CSub40D320::m_dwUnk1C` is the struct's last member, at +0x1c
        within the +0x04 `m_obj04` sub-object), same field, same value.
        The extra `imm:add:0x4` is the companion `add`-vs-`lea` instruction-
        selection noise (rule 358's family) for the `pWall+4` pointer
        computation itself (hover.exe:0x41a6c7 `lea ebp,[ebx+4]`, ours an
        `add` on an already-loaded register); a pre-existing `disp+0x4`
        elsewhere in the function (the `pdwFront[1]` access passed to
        `Sub_421B50`) already cancels it in the presence report. No wrong
        field or wrong element index - both concerns the lead raised are
        the same known noise class, confirmed positionally at both call
        sites.
      - **`CHumanPlayer::Sub_42AD30` (0x409180, sound-cue swap latching
        `m_wUnk01F2`/`m_wUnk01F6`):** hover.exe:0x4091ff-0x409204 computes
        `((long)dwC < 1) ? 1 : -1` branchlessly (`cmp dwC,1` /
        `sbb eax,eax` / `and eax,2` / `dec eax`, giving 0-1=-1 or 2-1=1);
        our build compiles the identical ternary as a branch
        (`mov ax,0xffff` / `jge +4` / `mov ax,1`, giving -1 unless
        `dwC>=1` overwrites it to 1). Confirmed both give -1 when
        `dwC>=1` and 1 when `dwC<1` - same computation, same stored value,
        just branchless vs. branch-based instruction selection for the
        same ternary (compiler codegen choice, not source-controlled per
        rules 17-18's precedent). Occurs twice (`m_wUnk01F2` and
        `m_wUnk01F6`), both identical.
      - **`CPlayer::Sub_408550` (0x408550, front/back proximity point
        computation):** `disp+0x184`/`disp+0x18c` (ours, direct absolute
        member offsets) vs original's `disp+0x4 x2` is the classic
        hoisted-address split: hover.exe:0x4085ec/0x40863a compute
        `edx = this+0x180` / `this+0x188` once each, then store the paired
        field through `[edx]`/`[edx+4]` (`m_dwUnk0180`/`m_dwUnk0184`,
        `m_dwUnk0188`/`m_dwUnk018C`); our compile addresses each field
        independently from `this`. Same four fields, same stored values
        (`((short*)&dwFrontX)[1]` etc., matching the source exactly).
      - **`CPlayer::Sub_42BE50` (0x42be50, wall-proximity range check):**
        our extra `disp+0x190` is the same hoisted-address-split class
        again - hover.exe:0x42be6d reads `m_dwUnk0190` as `[edi+0x10]`
        reusing `edi = this+0x180` (already live from `pMine = &m_dwUnk0180`
        at 0x42be56), where our compile reads it directly as `this+0x190`;
        `this+0x180+0x10 == this+0x190` algebraically, and `pProx[4]` (the
        other addend of `lLimit`) is unaffected either way.
      - **`CPlayer::CPlayer` (0x41ac30):** the `disp+0xb4` is NOT rule
        253's inlined-base-ctor-vtable-store shape (checked explicitly,
        since the lead named that rule as a candidate) - it is a plain
        `this`-pointer reuse-vs-reload difference for the first `CObList`
        placement-construction call. hover.exe:0x41ac9d does
        `lea ecx,[eax+0xb4]` reusing `eax` (already loaded as `this` for
        the `m_awUnk0090` zeroing loop just above), while our compile
        reloads `this` fresh with `mov ecx,[ebp-0x10]` then `add ecx,0xb4`
        before the same `CObList::CObList(10)` call - both compute
        identical addresses for `m_obList00B4` (declared at gameobj.h:501,
        already a real `CObList` member; the other three placement calls
        at +0xd0/+0xec/+0x108 match byte-for-byte with no flag at all, so
        this is a one-off scheduling artifact on the first call only, not
        a systemic gap). The companion `data:CPlayer::\`vftable'` finding
        is rule 362's already-named "unnamed `<OFFSETn>` vtable" class:
        hover.exe:0x41ada8 stores the same address (`0x4bc910`) as a bare
        immediate because `tools/orig_data.csv` has no entry naming
        `CPlayer::\`vftable'` on the original side, while our recompiled
        side's copy of the same instruction resolves the symbol - both
        sides write the identical vtable pointer.

376. **Three verification leads chased, all NOISE; rule 374's open lead is
      now closed positionally.** `CSub410F20::Sub_419960` (0x419960) and
      `CSub403980::Sub_403980` (0x403980) are one displacement-splitting
      class in each direction; `CSub402E00::Sub_404030` (0x404030) settles
      rule 374 and turns out to be neither inlining nor a wrong callee.

      `Sub_419960`'s two `disp-0x4` (opcheck) are the second column-ramp
      loop's `m_pUnk6C[i]` store: hover.exe advances its byte index
      (`add esi,4` at 0x419a8f) *before* using it, then reaches back with
      `[ecx+esi-4]`/`lea eax,[esi+ecx-4]` to hit the current element; our
      build advances the index *after* the store, so both sides write the
      identical `m_pUnk6C[i]` - a hoisted-index instance of rule 353's "a
      hoisted address that splits one displacement into two".

      `Sub_403980`'s missing `disp+0xa0` (opcheck) is the same class,
      folded rather than split: hover.exe computes
      `pEnd = pCur + m_dwUnk00*2 + 0xA0` with the second `+= 0xA0` folded
      into `lea ebp,[eax+ebp*2+0xa0]` (0x4039de); our build performs both
      source `pCur += 0xA0` additions back-to-back before that `lea`
      (`add eax,0xa0` twice, early), so the value the `lea` needs is
      already baked into `eax` and it shows no `+0xa0` of its own. Same
      final value, different scheduling of the two `+= 0xA0` statements
      already in source - not source-controllable per rule 280.

      `Sub_404030`: positionally confirmed hover.exe calls `Sub_402630`
      and `Sub_402990` directly, never inlined - 3 static calls to each
      (0x4040e8/0x404333/0x40434d for `Sub_402630`, the matching offsets
      for `Sub_402990`) against our 4, because the original cross-jumps
      the "normal" outcome of the `if (dwX0 > dwX)` branch and its sibling
      into one shared call site (0x4040e8) that reuses whichever value the
      predecessor pushed, while the two "early-return" tails
      (`Sub_402630(pObj, dwX0/dwX, dwXMax); return;`) stay separate at
      0x404333/0x40434d. Net +1 per helper for us, exactly matching
      callcheck. The `Sub_406D00` x10 is rule 281's already-documented and
      deliberately-unreproduced tail duplication: hover.exe copies the
      "advance to next pObj; if null call `Sub_406D00(dwX,dwXMax)`;
      return" tail at all 11 `goto L_advance` predecessors (byte-identical
      copies at 0x4042d7, 0x4042f2, 0x40430d, 0x404368, 0x404383,
      0x40439e, 0x4043ef, 0x40440a, 0x404425, 0x404440, 0x404491) instead
      of our one shared `L_advance` label - net 11-1=10, exactly matching
      callcheck - and rule 281 already measured that reproducing the
      duplication regresses the function (35.03% -> 32.15% or worse), so
      the shared label stays. No source change made: this is call-target
      accounting from cross-jump merging and tail duplication, not a
      missing or wrong callee, consistent with rule 285's full behavioural
      audit of this function. **A fifth and sixth noise class for
      `tools/callcheck.py`, alongside rule 358's two:** a compiler
      cross-jump that merges two logically-distinct call sites (with
      different arguments reached via different predecessors) into one
      shared call, undercounting the original by one per merge; and tail
      duplication of a single shared exit across every branch that jumps
      to it, overcounting the original by (copies - 1). Both are visible
      only by reading the branches into the call site in the disassembly,
      not from the call multiset alone.

377. **Six-lead bundle across texmisc.cpp/highscrdlg.cpp/hover.cpp,
      opcheck.py only, all six settled NOISE - no source changes, so
      nobody re-triages them.** (a) `Hover_sub_4262B0` (0x4262b0,
      `imm:push:0x0` x4): hover.exe:0x4263b5-0x4263c3 is the
      `FormatMessage(FLAGS, NULL, dwError, 0, &pszOsError, 0, NULL)` call
      - four of its seven args are the literal 0, pushed as immediates.
      Ours zeroes `edx` once (`xor edx,edx`, reused right before it for
      `g_szUnk_460060[0]='\0'`) and pushes the *register* four times
      instead of four separate `push 0` immediates - same call, same
      values, a register-vs-immediate materialisation choice (rule
      353/370(a)'s class), not the `InitInstance` try/catch lead the
      handoff suggested chasing (that MFC boilerplate is unrelated to
      this leaf function's own `FormatMessage` call). (b)
      `CSub414550::Sub_414550` (0x414550, `disp-0x1` and `imm:push:0x0`):
      two independent instances of the same two noise classes in one
      function. The `push 0` is `pPalette->GetPaletteEntries(0, 0x100,
      aEntries)`'s `nStartIndex=0` argument - hover.exe zeroes `ebp` and
      pushes it, we push the literal `0` directly (mirror image of (a)'s
      direction, same class). The `disp-0x1` is `m_abTable[nLevel*0x100 +
      i] = ...`: hover.exe increments its loop counter (`ebx`) *after*
      using it to form the index (`add ecx,ebx` then `inc ebx`); our
      build increments its counter (`edi`) *before* forming the address
      and compensates with `lea ecx,[ebp+edi-1]` - algebraically
      identical (`ebp+(i+1)-1 == ebp+i`), a pure scheduling choice of
      when the increment happens relative to the address computation,
      confirmed by tracing both register sequences instruction-by-
      instruction. (c) `CSub424BD0::Sub_433F10` (0x433f10, `disp-0x1`):
      looked exactly like rule 363's "running total read from the
      previous entry" shape at first glance (the handoff's suggested
      lead) but is not - it is the `*p++ = '\0'` null-terminator idiom in
      the name-packing loop. hover.exe computes `edx = p+nLen` then
      `inc edx` (`edx = p+nLen+1`, the *next* iteration's `p`) and writes
      the terminator at `[edx-1]` (`= p+nLen`) afterward; our build
      writes the terminator first (`mov byte ptr[eax],0` at `eax =
      p+nLen`) and increments `eax` to `p+nLen+1` afterward - same byte
      written to the same address, same next-`p`, just terminator-then-
      increment vs increment-then-terminator-behind. No previous-entry
      read anywhere in this function; rule 363's shape does not recur
      here. (d) `Hover_sub_43C4C0` (0x43c4c0, `disp-0x2`/`imm:add:0x12`
      missing, `disp+0x10`/`disp+0xe`/`imm:add:0x16` extra): the
      DLGITEMTEMPLATE field writer for
      `style@0/extStyle@4/x@8/y@0xa/cx@0xc/cy@0xe/id@0x10/class@0x12/
      title@0x16` (not a wave-format header - the handoff's guess was
      wrong, this is `Hover_sub_43C420`'s in-memory dialog template
      builder). hover.exe hoists `ecx` to `pItem+0x12` once and reaches
      `cy`/`id` as `[ecx-4]`/`[ecx-2]`, then hoists again to `+0x16` and
      reaches `class`/`title` as `[ecx-4]`/`[ecx]`; our build addresses
      `cy`/`id`/`class`/`title` directly off the un-hoisted base
      (`+0xe`/`+0x10`/`+0x12`/`+0x16`) with one combined `add ecx,0x16`
      instead of two `add`s of `0x12`+`0x4`. Every field lands at the
      identical offset with the identical value on both sides - textbook
      hoisted-address-splits-one-displacement-into-two (rule 337/362/372
      class), confirmed field-by-field. (e) `Hover_sub_43B0A0` (0x43b0a0,
      `imm:adc:0x1`/`imm:cmp:0x1`): the source comment already flags this
      - hover.exe's `cmp ecx,1 / adc eax,-1 / and eax,ecx` is the
      branchless form of `(nResult != 0) ? nResult : NULL` (`eax` becomes
      0 when `ecx==0`, else all-ones, so the final `and` yields `ecx` or
      `0`); our build compiles the same ternary as `test eax,eax / jne
      +2 / xor eax,eax` - a branch instead of the carry-flag idiom, same
      result for every input, confirmed algebraically. (f)
      `Hover_sub_43ADF0`/`Hover_sub_43AD40` (0x43adf0, `disp-0x1` x2;
      0x43ad40, `disp-0x1`/`disp-0x2`): both are the same two mechanisms.
      First, the pre-advance-and-compensate pattern from (b): `Sub_43AD40`
      advances `pSrc`'s register early (`inc esi` / `add esi,2`) and
      reads the sample via `byte/word ptr [esi-1]`/`[esi-2]`, where
      hover.exe keeps a separate never-advanced register (`ebp`/`ecx`)
      and reads `[ebp]`/`[ecx]` directly - same address either way
      (`esi-1 == ebp`, `esi-2 == ecx`), and `Sub_43AD40`'s 8-bit path also
      has `sub al,0x80` (original) vs `add al,0x80` (ours), which are the
      same byte operation mod 256 (`-0x80 == +0x80` in one byte). Second,
      `Sub_43ADF0`'s two `disp-0x1` hits (`lea edx,[esi-1]` in both the
      8-bit and 16-bit interpolation loops) are a missed do-while
      conversion: hover.exe proves `for (j = nRatio - 1; j > 0; j--)`
      always runs at least once (the enclosing `if (nRatio > 1)` already
      guarantees `nRatio-1 >= 1`) and drops the loop-entry test entirely;
      our build computes `nRatio-1` the same way but keeps a `test
      edx,edx; jle` guard that can never actually fire. Same iteration
      count either way in all four functions; no source changes taken
      for (a)-(f).

378. **`CSub423030::CSub423030` (0x423030) zero-initialized three members
      hover.exe's constructor never touches.** `tools/disasm.py 0x423030`
      writes only +0x28, the m_bitmap/m_dc sub-object vtables at +0x2c/
      +0x34 (three chained trivial-base-ctor vtable stamps at +0x2c, then
      a real `call` for CDC's ctor at +0x34), +0x44, +0x46, +0x48, +0x4a
      and +0x58 - there is no store to +0x4c/+0x50/+0x54
      (`m_pUnk4C`/`m_dwUnk50`/`m_dwUnk54`) anywhere in the function. Our
      constructor zeroed all three, which `opcheck`'s presence check
      flagged as `disp+0x4c`/`+0x50`/`+0x54` on our side only. Confirmed
      harmless to drop by tracing every use: `Sub_411D80` writes all
      three fields (`m_pUnk4C = pInternalDC`, `m_dwUnk50`/`m_dwUnk54`
      from the source rect) before `Sub_401E80` - the only reader of any
      of them - ever runs, on the one instance (`m_obj00E8` in
      `CBumperView::Sub_410510`) that calls `Sub_401E80` at all, so the
      uninitialized state is never observed. Removing the three lines:
      0x423030 80.3% -> 87.1%; Accuracy 90.69% -> 90.70%, byte-weighted
      81.91% -> 81.92% (in isolation; see rule 379 for the combined
      number with that bundle's other fix in the same build).

379. **`CSub422F50::Sub_4053A0` (0x4053a0) dropped a vestigial address
      computation the original leaves lying around, same class as rule
      367.** hover.exe:0x405568-0x405574 reads `pSub[0x27]`/`pSub[0x28]`
      through absolute `[eax+0x9c]`/`[eax+0xa0]` displacements and then
      computes `add eax, 0x9c` (i.e. materializes `&pSub[0x27]`) even
      though eax is never used again before the next loop iteration
      reloads it fresh from the beacon array - the pointer is dead on
      arrival, exactly rule 367's pattern. `opcheck` flagged the missing
      `imm:add:0x9c` (the `disp+0x9c`/`+0xa0` reads already matched, so
      this was the one residual key). Naming
      `DWORD* pField = &pSub[0x27];` and reading `pField[0]`/`pField[1]`
      in the `Sub_401980(pPt[0], pPt[1], ...)` call instead of the raw
      `pSub[0x27]`/`pSub[0x28]` subscripts reproduces the address
      materialization with no behavioural change (same two field values
      either way - `pField` is never otherwise used). 0x4053a0:
      71.2% -> 75.5%. Combined with rule 378's fix in the same build:
      Accuracy 90.69% -> 90.72%, byte-weighted 81.91% -> 82.06%.

380. **Session covering opcheck leads on seven `src/bumpview.cpp`
      functions (bundle 12): two real fixes (rules 375/376), five noise,
      hit rate two in seven.** All five noise verdicts are the "hoisted
      address splits one displacement into two" class (rules 267/270/337/
      362) in one of its many shapes, confirmed positionally with
      `objdump` on our own `build/hover.exe` as well as
      `tools/disasm.py`/`compare.py --verbose` on hover.exe, so the next
      run does not re-triage them:
      - `CBumperView::Sub_410510` (0x410510): the `m_rcSpriteParams`/
        `m_rcComposite1`/`m_rcViewport` RECT reads (`disp+0x1bc`/`+0x1cc`/
        `+0x23c`/`+0x1d4`/`+0x1d8`/`+0x1e4`/`+0x1e8`/`+0x1ec`/`+0x1f4`/
        `+0x264`) are all instances of the class already exhaustively
        ladder-tested for this exact function by rules 267-277: whichever
        one field of a four-field RECT group is scheduled earliest in
        program order gets an absolute displacement off the unhoisted
        zone-descriptor base, the rest go through a `lea`-materialized
        pointer - which field draws the absolute form differs between
        the two compilations. The `imm:add:0x14 x2`/`disp-0x10` pair is
        rule 271's already-documented, already-tried-and-abandoned
        `pdwTarget[-4]` reproduction attempt for
        `m_pUnk00D0->m_dwUnk04`/`m_dwWidth`/`m_dwHeight` (confirmed again
        at hover.exe:0x410a21-0x410a37 and 0x410ab9-0x410ac8): naming the
        three members directly, as the source already does, is the
        established floor.
      - `CSub422F50::Sub_401C00` (0x401c00): the missing `disp+0x170`/
        `+0x90`/`+0xf8` and extra `disp+0x8 x2` are the same class applied
        to `POINT` pairs 8 bytes apart (`pPt1`/`pPt2` in the Wall,
        CTempWallPod and Player branches) - hover.exe reads one far field
        of each pair absolutely off the unadjusted `this` before forming
        the pair's own base, our build instead derives the second point's
        address as `pPt1_base + 8` (confirmed with `objdump` on our own
        build: `lea eax,[edi+8]` at both the Wall and CTempWallPod sites),
        which is why `+0x8` appears twice on our side only.
      - `CBumperView::Sub_427A80` (0x427a80): `imm:mov:0x1547a`/`0xc000`
        vs our `imm:add:0x1547a`/`imm:and:0xffff6b86`/`imm:cmp:0x1` is a
        branchless materialization of the `dwMul = (nUnk != 0) ? 0x1547a
        : 0xc000;` ternary (`cmp eax,1` / `sbb eax,eax` / `and
        eax,0xffff6b86` / `add eax,0x1547a`), same idiom family as rule
        365(b)'s `(flags&8)?'W':'!'` trick - checked algebraically:
        mask 0 gives 0x1547a, mask 0xffffffff gives 0xc000, so both sides
        compute the same constant. `imm:add:0x258`/`disp+0x258` is the
        hoisted-base class again for `pZone->m_rcViewport`.
      - `CSub422C30::Sub_411BB0` (0x411bb0) and `CSub422C30::Sub_40E9A0`
        (0x40e9a0): both read the same `m_rcSource2` RECT (+0x2c) and
        disagree on which fields go through a hoisted `lea this+0x2c`
        base and which go through a direct `this+0x30`/`+0x34`/`+0x38`
        displacement - confirmed with `objdump` on our own build for
        both functions (`Sub_411BB0` reads all four fields absolutely
        where hover.exe hoists a base; `Sub_40E9A0` hoists a base where
        hover.exe reads two fields, `+0x34`/`+0x38`, absolutely). Same
        class, opposite direction in each function, no behavioural
        difference (same struct, same fields).
      - `Hover_sub_410CE0` (0x410ce0): the `disp-0x5`/`-0x6`/`-0x7`/
        `imm:add:0x3` (ours only) is the RGBQUAD-remap loop's destination
        pointer pre-biased by 3 bytes before the per-iteration `+4`
        instead of hover.exe's un-biased `+4` with `-1`/`-2`/`-3`/`-4`
        offsets (confirmed with `objdump`: our build does `add esi,3`
        once before the loop, then `-5(esi)`/`-6(esi)`/`-7(esi)` per
        iteration where hover.exe does `-1(ebx)`/`-2(ebx)`/`-3(ecx)`/
        `-4(ecx)`) - same RGBQUAD fields (`rgbReserved`/`rgbRed`/
        `rgbGreen`/`rgbBlue`), same source values, different constant
        folded into the addressing arithmetic.

381. **`tools/flowcheck.py`'s `sign:shr` class found a materialised-vs-
      folded shift, not a signedness bug: `CMainFrame::OnCmd34010`
      (0x426c20) needs its own local so the compiler can't prove the
      shift is dead.** hover.exe:0x426c4e is `shr eax, 0x1c` between
      `and eax, 0x10000000` and the `cmp eax,1`/`sbb eax,eax`/`and eax,5`
      branchless select that implements `nShow = fVisible ? SW_HIDE :
      SW_SHOW`. That select only tests eax for zero, so the shift's exact
      magnitude never affects the result - shifting an isolated single
      bit down by any amount preserves its zero/nonzero-ness. Folding the
      shift straight into the ternary's condition let our compiler see
      that and drop the `shr` as dead code; giving the shifted value its
      own local (`int fVisible = (mask) >> 28; nShow = fVisible ? ... :
      ...;`) forces it to materialise the shift exactly as hover.exe
      does, because the store to a real local closes off the dead-code
      proof. 96.3% -> 100.0% match, byte-identical. Accuracy 90.72% ->
      90.73%, byte-weighted holds at 82.06%. **Generalise:** an
      `arg:<callee>:N=<imm>` or `sign:<op>` lead that vanishes only
      because a boolean-context expression let the optimizer prove a
      sub-computation doesn't affect the truth test is fixable by naming
      the sub-computation as its own local - try that before writing the
      lead off as scheduling noise.

382. **A byte read from the registry needs an explicit `(char)` cast to
      reach `CheckDlgButton` the way hover.exe does.**
      `CQuickHelpDialog::OnInitDialog` (0x434360) was missing
      `sign:movsx`: hover.exe:0x4343d7 is `movsx eax, byte ptr
      [esp+0x10]` immediately before the `CheckDlgButton` call, sign-
      extending the "QuickHelpFlag" registry byte rather than zero-
      extending it. The sibling accessor `CQuickHelpDialog::Sub_424F30`
      (0x424f30, rule-clean on this checker) already reads the identical
      byte through `return (int)(char)abFlag[0];` - OnInitDialog's
      `BYTE abSkip[8]` buffer was instead passed to `CheckDlgButton`
      un-cast, so our compiler zero-extended it (`BYTE` = `unsigned
      char`). Casting the call argument to `(char)abSkip[0]` reproduces
      the `movsx` and takes the function 92.1% -> 97.7% match (the
      remaining diff is the already-documented "QuickHelpFlag"
      string-size measurement artefact, unrelated to this fix). Accuracy
      90.72% -> 90.73%, byte-weighted holds at 82.06%.

383. **Four more `tools/flowcheck.py` leads chased on the mainfrm/
      optionsdlg/bumperdoc/texture bundle, all settled NOISE - folded in
      so the next run does not re-triage them.** (a)
      `CMainFrame::OnCreate` (0x41db90, `arg:CFrameWnd::OnCreate:5`
      present in the original, `:6` in ours): the position shift is a
      callee-saved register, not an extra argument. hover.exe re-reads
      `cxDigit` (the `GetTextExtent("0",1).cx` result) from its stack
      slot for both `SetPaneInfo` calls (`mov edx,[ebp-0x14] / mov
      [ebp-0x1c],edx / mov edx,[ebp-0x1c]`, a genuinely redundant
      reload-of-a-reload); our build instead keeps the same value live in
      `ebx` across both calls, which costs one extra callee-saved
      `push ebx`/`pop ebx` pair and shifts every `arg:` position after it
      by one. Confirmed by disassembling our own `build/hover.exe` with
      `objdump` and by testing an explicit `CSize size = ...; cxDigit =
      size.cx;` split (mirroring the working pattern at
      `bumpview.cpp:5037`) - it made no difference, so the register-vs-
      reload choice is the compiler's, not the source's. (b)
      `CSub413F10::OnOK` (0x413fe0, `arg:Hover_sub_401140:0=0x1` /
      `arg:CBumperView::Sub_4274B0:2=0x1`): the checker's own documented
      "a push made before a nested getter call is attributed to the
      getter" class. hover.exe pushes `Sub_4274B0`'s third argument
      (the literal `1`) at the very top of the `if` block, before the
      `theApp.m_dwUnk01D0 = m_nRearView;` store and before calling
      `Hover_sub_401140()` for `pdw` - a trivial, side-effect-free
      constant push that the original's scheduler hoisted early for no
      observable reason, since our build computes and pushes the same
      three arguments (`pdw[0]`, `pdw[1]`, `1`) with the getter called
      exactly once either way. Tried inlining the getter call twice
      directly into the argument list (`Sub_4274B0(Hover_sub_401140()->
      m_dwUnk00A4, Hover_sub_401140()->m_dwUnk00A8, 1)`) on the theory
      the original's single `call Hover_sub_401140` might come from CSE
      of two source-level calls - it does reproduce the early push
      position, but costs a second real `call` instruction (`Hover_
      sub_401140` is not pure - it dereferences a volatile pointer and
      calls a virtual `GetActiveView()` - so the compiler never CSEs it),
      dropping the function 89.3% -> 88.0%. Reverted. (c)
      `CBumperDoc::Sub_412650` (0x412650, `jcc:eq`, 56.8% matched): the
      redundant-check pattern `if (pos) RemoveAt(pos); if (pos) return
      TRUE;`, written out twice, is already our source's shape and
      matches hover.exe's for the *first* occurrence (both keep the
      second `if` as a real, provably-redundant branch); for the
      *second* (tail) occurrence, hover.exe still uses a real branch
      (tail-merged into the same "return TRUE" epilogue as the first),
      while our compiler folds the tail `if (pos) return TRUE; return
      FALSE;` into a branchless `cmp esi,1 / sbb eax,eax / inc eax` -
      one fewer `jcc:eq` for the same three-call, two-list, TRUE/FALSE
      result. Tried merging each pair into a single `if (pos) { RemoveAt
      (pos); return TRUE; }` block, matching the checker's own
      instinct - it removed the intentional redundant check by
      construction instead of by proof, tail-merged both returns into
      one epilogue the original doesn't share, and dropped the function
      to 40.5%. Reverted. (d) `CBumperDoc::OnOpenDocument` (0x415450,
      9230 bytes, one `jcc:eq` short over 48 jumps): rule 330's already-
      documented, already-probed gap - the `_mbscmp(...) != 0` flag at
      hover.exe's ebp-0x3f4 (materialised through a `je`/`mov 1`/`jmp`/
      `mov 0`/`cmp`/`je` ladder before the wall-name check) that our
      build folds into the use site instead. Rule 330 already tried and
      rejected several spellings for the two-part frame-layout gap this
      belongs to; nothing new was found here and no further attempt was
      made.

384. **`CBumperView::Sub_40CCE0` (0x40cce0): the first message string was
      wrapped in a null check the original never makes.** First pass over
      `tools/flowcheck.py`'s new `arg`/`jcc`/`sign`/`loop` invariants
      (9-lead bundle on `src/bumpview.cpp`). hover.exe:0x40CFC2-0x40CFC7
      draws `pszA` unconditionally (`mov ecx,[ebp+8]` straight into the
      `Sub_40D0A0` call, no preceding `cmp`/`je`); only `pszB`/`pszC`/
      `pszD` at `[ebp+0xc]`/`[ebp+0x10]`/`[ebp+0x14]` get a
      `cmp dword ptr [ebp+N],0 / je` guard, both in the measuring pass
      (rule confirmed by walking all four `GetTextExtent` sites) and the
      drawing pass. Our source wrapped the `rc.top += cyHalfSpacing;
      Sub_40D0A0(&rc, pszA, ...)` pair in `if (pszA != NULL)`, which is
      exactly the extra `jcc:eq` flowcheck flagged. Made the pszA draw
      unconditional, matching hover.exe's structure; the function's
      comment already documented "the first string is always present",
      so this callee contract was already known, just not carried through
      to the draw site. Kept as REAL (a genuine guard-sense difference,
      not merely a faithfulness choice) even though the numbers move
      slightly against it: 0x40cce0 69.0% -> 68.9%, Accuracy unchanged at
      90.72%, byte-weighted 82.06% -> 82.04%.

385. **Same bundle, eight leads settled NOISE, all reducing to two
      mechanisms already on file plus one worth naming for
      `tools/flowcheck.py` specifically: a tail-duplicated branch whose
      shared argument tail is reached by `jmp`, not `call`, defeats the
      checker's linear push-accumulator.** `flowcheck.py` resets its
      pending-`push` list only on `call` or `add esp,N`; a conditional
      branch that jumps *into* a shared tail (rather than falling through
      into it) leaves its own branch-local pushes in that list, so by the
      time the one physical `call` fires the checker has accumulated
      pushes from every branch that ever jumps there, not just the one
      that runs. This inflates `arg:<callee>:<i>` position numbers by
      exactly the pushed-item count of every other tail-duplicated
      branch, and can misattribute an unrelated branch's leftover
      immediate to a completely different, later, register-called
      import. Traced positionally with `tools/disasm.py` on all three
      instances - counted every push between calls by hand and confirmed
      each duplicate/shared push carries the identical value on both
      sides:
      - `CBumperView::OnContextMenuMsg` (0x428d90): all seven
        `::WinHelp(hwnd, "hover.hlp", HELP_CONTEXTPOPUP, dwData)` call
        sites (one per zone) tail-duplicate into hover.exe's *one*
        physical call at 0x429165-0x42916d; six of the seven dwData
        pushes (`0xa3c`/`0xa28`/`0xa46`/`0xa50`/`0xa5a`/`0xa64`) are each
        immediately followed by a `jmp`, so they stay live in the
        checker's push list until the seventh (map, `0xa32`) branch's
        `jne`-reached copy falls straight into the shared tail - which is
        why flowcheck sees `0xa32`/`0xa64` at positions 3/4 of the named
        `WinHelpA` import and `0xa3c` at position 3 of an unrelated
        register call. Verified all seven dwData values reach the
        correct zone in the correct source order by tracing every
        branch's own `call edi`/`jne`/`je` chain in `tools/disasm.py
        0x428d90`'s output - rule 361's fix from the prior pass is
        intact, nothing to change. Same function's `AppendMenuA`
        uFlags=0 and `CMenu::TrackPopupMenu`'s lpRect=NULL are rule
        377's register-zero-materialization class (`xor edi,edi` once,
        reused for both `push edi`, against the original's two separate
        `push 0`), and the `CMenu::vftable`/`<OFFSETn>` PtInRect-cached-
        register noise already named in rule 362 recurs unchanged.
      - `CSub423030::Sub_411D80` (0x411d80): the `Hover_sub_40E730` call's
        `rcDest.left`/`.top` (both source constant `0`) are written into
        two stack slots via a zeroed register (`mov [esp+0x18],edi` /
        `[esp+0x1c],edi`, hover.exe:0x411F24/0x411F2A - the frame this
        session's rule 378/379 already annotated) and read back with
        `mov`+`push` at the call site instead of a fresh `push 0`; same
        stack-vs-immediate materialization family as rule 377, just via
        memory instead of a register. The `CreateDIBSection` findings on
        the same function are rule 358(a)'s import-cached-in-a-register
        class (`mov ebp,[GDI32.dll::CreateDIBSection] / call ebp` on our
        side against the named import on the original's).
      - `CBumperView::Sub_407430` (0x407430): the ternary
        `(dwFlag != 0) ? &m_dcUnk00B4 : &dc` feeding
        `m_obj00E8.Sub_401E80(ptr, 0)` has hover.exe push the `0`
        bFullUpdate argument separately in *each* arm before a shared
        `push eax; lea ecx,[esi+0xe8]; call` join (0x407787 and 0x40778b,
        confirmed in `tools/disasm.py 0x407430`); our build hoists that
        `push 0` to a single copy reached by both arms. Same value on
        both sides, one fewer static instruction on ours - the class
        above, not a different argument.
      - `CSub422C30::Sub_40E9A0` (0x40e9a0): the `if (pThis->m_dwUnk44 >=
        i+1) {...} else {...}` selecting `pSrc`/`pSrcRect` inside the
        blit loop has hover.exe duplicate the *entire* 13-push
        `Hover_sub_40E730` argument tail into both arms (0x40ea16-0x40ea45
        falling through to a `jmp` over 0x40ea48-0x40ea85's twin copy),
        so the checker's accumulator counts 2 (pre-if pushes, including
        the `push 1` bUnk argument) + 13 + 13 = 28 pushed items before
        the one physical call - exactly why `1` (bUnk, the farthest-
        pushed/first-C-argument) lands at position 27 there against our
        single-copy position 14. Confirmed the pushed field values are
        identical in both arms field-by-field against source.
      - `CSub406090::Sub_405B90` (0x405b90) and `Hover_sub_411770`
        (0x411770) are not this mechanism: they reproduce rule 146 (VC
        2.1 emits a branch instead of `setns` once a function needs four
        or more sign-derived booleans - this exact function, "about 8
        instructions", already logged as open/unfixable) and rule 13
        (the `for` loop's un-provable `<= 0` guard re-tests on entry
        because the bound lives in memory - this exact function, ~20
        source spellings already tried, current shape is the deliberately
        chosen 52.00% winner) respectively. Both are named-function,
        already-exhausted toolchain drift, not re-triaged.
      Hit rate 1 real fix in 9 leads. Numbers after the one kept fix:
      Accuracy 90.72%, byte-weighted 82.04%.

386. **`CPlayer::Sub_408820` (0x408820) had two `for` loops the original
      compiles as `do`/`while`.** flowcheck's bundle-13 lead was two
      SIGNED jccs we have and hover.exe lacks. The source's own guard,
      `if (nMaxX < nMinX || nMaxY < nMinY) return 1;`, already proves both
      the X and Y ranges are non-empty, so hover.exe rotates both nested
      `for` loops into `do { ... } while (++n <= max)` with no separate
      entry test; our `for` loops kept an independent entry test on top
      of the guard, duplicating the range check (guard once, then each
      loop's own `for`-condition once more) for the extra jccs. Rewriting
      both loops as `do`/`while` (nX/nY initialized before the loop,
      `while (++nX <= nMaxX)`/`while (++nY <= nMaxY)`): 38.0% -> 42.1%;
      Accuracy 90.72% -> 90.73%, byte-weighted 82.06% -> 82.07%. Same
      class as rule 377(f)'s do-while conversion; see rule 390 for two
      cases in this same session where the identical technique made
      things worse instead (0x422cb0, 0x414d70) - check for collateral
      damage on other functions in the file before trusting this one.

387. **`CMerlinWorld::Sub_4128A0` (0x4128a0) has a dead sentinel check
      hover.exe never eliminated.** The found-path (reached only via the
      search loop's `je`) runs a `cmp eax,-1`/`je` that can never fire
      (`eax` is the matching loop index, always >= 0) before returning 0;
      the loop-exhausted and `nSize<=0` paths jump straight to
      `SetAtGrow` without ever materializing a -1 sentinel or touching
      that check. This is the sibling of 0x413db0/0x414300's search loops
      (rule 390), which DO use a real `i=-1` sentinel merged with a
      `goto Found`/`Found: if (i!=-1) return 0;` idiom - 0x4128a0's
      source used the same idiom, but in this instantiation the compiler
      only needed the runtime check on the found branch (where `i` isn't
      known at compile time) and folded it away on the not-found branch
      (where `i` is the literal -1 constant). Reproducing that exact
      asymmetry - `if (m_arrUnk28[i]==dwUnk) { if (i != -1) return 0; goto
      DoInsert; }` inside the loop, `i = -1;` never assigned, `DoInsert:`
      label right before `SetAtGrow` - beats the literal
      `goto Found; i = -1; Found: if (i != -1) return 0;` form (which
      forces a real `-1` materialization and check on BOTH paths and
      matches far worse): 44.8% -> 53.3%; byte-weighted 82.07% -> 82.08%.

388. **`CMerlinWorld::CMerlinWorld` (0x418ae0) called `.GetSize()` inline
      in a `for` condition where its own destructor (0x418c00, four
      identical loops, none flagged) caches it into a local first.**
      hover.exe compiles the destructor's four `for (i=0;i<n;i++)`
      delete-loops as countdown `dec reg`/`jne` loops because `n` is a
      local computed once before the loop; the constructor's otherwise
      identical loop wrote `g_obArrUnk_4A1D08.GetSize()` directly in the
      `for` condition, which our compiler treated as not proven
      loop-invariant and left as a forward `i < n` (signed `jl`)
      comparison instead - flowcheck's missing `jcc:eq` (the `jne`) and
      extra `jcc:signed` (the `jl`). Caching it (`n =
      g_obArrUnk_4A1D08.GetSize();` before the loop, matching the
      destructor's style) reproduces the countdown form: 87.4% -> 91.6%;
      Accuracy 90.74% -> 90.75%, byte-weighted 82.08% -> 82.09%. General
      lesson: when a sibling function (ctor/dtor, or another method
      touching the same array) already matches a loop over the same
      collection, its caching style is the first thing to try.

389. **`CFloorObj::Sub_411570` (0x413220): the early-return check ran
      only when the list entry was non-NULL; hover.exe runs it every
      iteration.** Source read `while (pos!=NULL) { pObj=GetNext(pos); if
      (pObj!=NULL) { bAny|=...; *pdwOut|=dwFlag; if (bAny!=0 &&
      *pdwOut!=0) return 1; } }` - nesting the break check inside the
      `if`. hover.exe's `test ebx,ebx`/`je`/`cmp [edi],0`/`je` pair back
      to the loop's `pos!=NULL` test is control flow shared by BOTH the
      null and non-null paths (confirmed positionally: a NULL `pObj`
      still reaches the ebx/edi check before looping, rather than jumping
      straight back to the top), so a run of NULL entries after a
      non-null one that already set both flags returns 1 immediately
      instead of continuing to scan. Moving the `if (bAny != 0 &&
      *pdwOut != 0) return 1;` outside the `if (pObj != NULL)` block
      (dwFlag/pdwOut are untouched either way) matches: 82.8% -> 98.0%;
      Accuracy 90.75% -> 90.77%, byte-weighted holds at 82.09%. Verified
      no collateral change to any other function in the file.

390. **Session covering flowcheck leads on an 8-item, four-file bundle
      (bundle 13): four real fixes (rules 381-384), one dead-code noise
      verdict folded into rule 387's fix, and four UNRESOLVED where a
      structurally-correct fix measurably lost overall despite clearing
      the flowcheck signal - reverted, recorded here so the next run
      doesn't re-attempt the same reproduction and re-discover the same
      loss the hard way.
      - `CSub40B800::~CSub40B800` (0x422cb0, missing `jcc:signed` +
        `loop`): the `for (i=0;i<g_arrUnk_4A47F0.GetSize();i++)` removal
        loop compiles as a test-at-top loop with an unconditional `jmp`
        back where hover.exe rotates it into a `dec esi`/`jne` do-while
        (guarded by a `GetSize()>0` pre-check, same class as rule 386).
        Restructuring to an explicit guarded `do`/`while` DOES rotate the
        loop (flowcheck clears) but the recompiler swaps which register
        holds the byte-offset vs the index counter relative to hover.exe
        (`esi`/`edi` reversed), so the function's OWN match falls (85.7%
        -> 85.3%) and byte-weighted falls (82.07% -> 82.06%); a plain
        `while` loop (no guard) left the register roles alone but also
        left the loop unrotated (no change at all, 85.7%). Reverted to
        the original `for` loop; still UNRESOLVED.
      - `CMerlinWorld::Sub_413DB0` / `Sub_414300` (0x413db0/0x414300,
        extra `jcc:eq`): both call `delete (CGameObj*)dwUnk` guarded by
        an explicit `dwUnk != 0` (0x414300) or `bFound != 0 && dwUnk !=
        0` (0x413db0) source-level check, but the compiler's own vtable
        deleting-destructor call ALSO inserts its own null check, so our
        binary has the null test twice (source-level `if` plus the
        delete's own guard) where hover.exe has it once - confirmed
        positionally, hover.exe's single `test edi,edi`/`je` at the
        delete call site is the compiler's OWN check, meaning hover.exe's
        source has NO explicit null guard on `dwUnk` at all (only the
        `bFound`/`nIdx>=0` guard on the array-removal side). Dropping the
        explicit `dwUnk != 0` guard and calling `delete` unconditionally
        does clear the extra `je` (confirmed - the flowcheck lead
        disappears) but the recompiler responds by no longer keeping
        `dwUnk` live in a callee-saved register across the whole
        function, costing far more match elsewhere: 0x413db0 62.3% ->
        35.6%, 0x414300 86.0% -> 55.0%. Reverted both; still UNRESOLVED,
        left for a pass with more register-allocation leverage (possibly
        forcing `dwUnk` into an explicit local read early, or matching
        hover.exe's exact variable-liveness shape some other way).
      - `CMerlinWorld::Sub_418FA0` (0x418fa0, missing `jcc:eq` + `jcc:
        signed` + `loop`): hover.exe has a genuinely dead
        `cmp edi,esi`/`jge`/`dec edx`/`jne` block right after the "copy
        game colors into the middle band" loop, unreachable in both the
        "band is empty" and "band ran" cases because `edi` is always
        already equal to `esi` by the time the check runs (the loop
        precomputes its own end index into `edi` rather than incrementing
        it) - the same shape as 0x4128a0's dead sentinel check (rule
        382). Reproducing it as a second, textually-identical `for (;
        i < nEnd; i++) aEntries[i].peFlags = PC_NOCOLLAPSE;` right after
        the first DOES clear the flowcheck lead, but costs the function
        its own match (66.9% -> 65.5%) AND causes collateral regressions
        in two unrelated functions later in the same translation unit
        (`CSub414550::Sub_414550` 32.3% -> 34.4%(*), `Hover_sub_422370`
        96.0% -> 72.0%(*) - net accuracy 90.75% -> 90.72%, byte-weighted
        82.09% -> 82.08%. Reverted; still UNRESOLVED. (*) one improved,
        one regressed, net still a loss once weighted by size - always
        diff the FULL per-function match table before/after a change
        that touches loop shape, not just the target address; a
        same-file collateral swing this large is a sign the change is
        shifting the compiler's layout/register decisions for later
        functions in the unit, not actually reproducing hover.exe.
      - `CSub4235E0::Sub_42AD30` (0x414d70, missing `jcc:eq` + `loop`):
        the beacon-scan `for (nLeft=nBeacons,i=0; nLeft>0; i++,nLeft--)`
        loop (already countdown-style, several `continue`s in the body)
        compiles with the same test-at-top/unconditional-`jmp`-back shape
        as rule 386 and 0x422cb0 above; hover.exe rotates it to `dec
        edi`/`jne`. Restructuring to an explicit guarded `do`/`while`
        (each `continue` replaced with `goto NextBeacon;` to a label
        right before the `i++`/`--nLeft` test, preserving the original's
        per-iteration update semantics) DOES rotate the loop and clear
        the flowcheck lead, and the target function's own match even
        ticks up slightly (64.3% -> 64.8%), but it triggers the same
        same-translation-unit collateral pattern as 0x418fa0: five other
        `src/texload.cpp` functions swing by measurable amounts (one as
        far as 92.3% -> 61.5%), net Accuracy 90.75% -> 90.69% and
        byte-weighted 82.09% -> 82.02%. Reverted; still UNRESOLVED. Two
        occurrences of the identical loop-rotation technique causing this
        exact same collateral pattern in one session (this and 0x422cb0)
        suggests the recompiler's register allocator is globally
        sensitive to total register pressure/stack-slot count changes
        within a translation unit in a way that a single isolated
        function-level fix cannot control; a future attempt should
        rebuild and diff the WHOLE file's function list before trusting
        any do-while rotation, not just the target address.
      - `CSub410F20::Sub_414A10` (0x414a10, 41 bytes, two `shr` we have
        that hover.exe lacks): both `memcpy(pBottom, pB, nCount *
        sizeof(DWORD))`/`memcpy(pTop, pA, nCount * sizeof(DWORD))` calls
        inline to a generic byte-count expansion (`shl eax,2` to bytes,
        then `shr ecx,2`+`rep movsd`+`and ecx,3`+`rep movsb` per call) on
        our side, where hover.exe's compiler recognized the `n *
        sizeof(DWORD)` pattern well enough to skip the byte round-trip
        entirely and emit `mov ecx,eax`/`rep movsd` directly with `eax`
        still holding `nCount` (dwords), never converting to bytes at
        all. Tried swapping the multiplication order (`sizeof(DWORD) *
        nCount`) - no change to the generated code, confirming this is
        the compiler's own memcpy-inlining heuristic rather than
        something addressable by rephrasing the call. Left UNRESOLVED
        (source unchanged); a future attempt might try forcing the count
        through a `DWORD` local before the multiply, or reordering the
        two memcpy calls, but this is most likely an inline-memcpy
        front-end quirk outside decompiled-source control, same
        character as rule 379's noted VC++ 2.1/2.2 codegen deltas.

391. **`CHoverApp::CHoverApp` (0x41c100): the six `LVLPARAM(i).m_dwUnkNN
      = ... * 9 / 10` (and `* 11 / 10`) divisions are signed (`cdq`+
      `idiv`) in hover.exe where our `DWORD` fields divide unsigned
      (`div`) - tried, reverted.** All six dividends
      (`m_dwUnk44`/`m_dwUnk50`/`m_dwUnk38`) are provably non-negative and
      well under 2^31 for every reachable level (0-15), so signed and
      unsigned division produce bit-identical results - a
      faithfulness-only difference, not a behavioural one. Casting each
      dividend to `(int)` (the header's `DWORD` fields are out of a
      single function's scope to change) does reproduce the `cdq`/`idiv`
      pair and clears the flowcheck `sign:` lead (0x41c100 71.8% ->
      73.8%), but the six extra `cdq` bytes shift every function
      compiled after it in `hover.cpp`, and `tools/compare.py`'s
      matcher (address/PDB-based, not purely symbol-based for every
      instruction - jump tables and nearby unnamed statics are
      apparently sensitive to this) mis-scores ~22 unrelated functions
      elsewhere in the file as a knock-on (some by 20-30 points either
      way, e.g. 0x41f540 87.7% -> 61.6%, 0x43c8c0 100% -> 58.8%),
      netting byte-weighted 82.06% -> 81.40% - confirmed by toggling the
      one-line diff back and forth twice, deterministic each time. Per
      the pass method, a faithfulness-only fix is kept only if
      byte-weighted does not fall; it fell, so this one was reverted.
      Left as a known, reproducible-on-demand fix if a future pass wants
      it and can absorb (or separately chase) the collateral drift.

392. **`Hover_sub_439E80` (0x439e80): truncated warning string, real
      fix; a claimed loop back-edge did not reproduce.** The
      `"!!!! adding to channel..."` diagnostic passed to
      `Hover_sub_438820` was missing hover.exe's trailing `"!!!!\n\r"`
      (positional match via `tools/disasm.py`/`compare.py --verbose`);
      fixing the literal clears the `arg:Hover_sub_438820:1` flowcheck
      lead with byte-weighted unchanged (82.06% -> 82.06%), 72.8% ->
      73.0%. The function's remaining `sign:movsx` lead is rule 365(b),
      already on record (register-width choice on the branchless
      `(flags&8)?'W':'!'` idiom, identical pushed value either way). A
      third symptom in this pass's bundle description (a conditional
      loop back-edge original has that we lack) does not reproduce
      against current source: `tools/flowcheck.py --only loop` reports
      0 differences for this address with the string fix in place.

393. **Wide-string arguments: hover.exe's un-annotated data gets read
      as ANSI by the comparison tooling, truncating to the first
      character since a UTF-16LE literal's second byte is a NUL -
      new noise class, mirror image of the "string size mis-measured"
      class in the `flowcheck.py`/`opcheck.py` docstrings.**
      `Hover_sub_43C5A0` (0x43c5a0): all twelve `arg:...:9="X"` /
      `arg:Hover_sub_43C420:6="W"` flowcheck leads are this. Confirmed
      with `tools/compare.py --verbose 0x43c5a0`: at the exact same
      push instruction (e.g. 0x43c5d0), the original is rendered as
      `push "&" (STRING)` and our recompiled build (which carries our
      own PDB type info, `LPCWSTR`) as `push L"&Ok" (WIDECHAR)` - same
      address, same bytes, different decode. `"&Ok"` as UTF-16LE is
      `26 00 4f 00 6b 00 00 00`; read as a NUL-terminated ANSI string
      that's just `"&"` (0x26, terminated by the very next byte). Every
      flagged string is exactly the first character of the
      corresponding wide title/button text in
      `Hover_sub_43C5A0`/`Hover_sub_43C420`'s calls - not a real
      argument difference, both sides push the address of the same wide
      string. No fix; nothing to change.

394. **`Hover_sub_439010` (0x439010): the `arg:CreateWindowExA:2=""`
      lead is a linker tail-merge coincidence, not NULL-vs-empty-string
      - NOISE (the "one-past-end pointer lands on a linker neighbour"
      class named in `flowcheck.py`'s own noise list).** hover.exe
      pushes `g_szUnk_4C47B0+12` (`g_szUnk_4C47B0` = `"winmm.dll"`,
      `texload.cpp:80`) for the window-name argument, not our literal
      `""`. Reading hover.exe's `.data` bytes directly at that address
      (`winmm.dll\0` then zero padding out to offset 16, where `"Temp"`
      starts) shows offset 12 is one of those zero-padding bytes - i.e.
      hover.exe's window name is *also* an empty string, just reached by
      pointing 2 bytes into another global's post-NUL padding instead of
      a dedicated `""` literal. Same observable behaviour (empty window
      title either way); not fixable from this function even if it were
      wrong, since the target is linker layout of an unrelated global.

395. **`Hover_sub_439560` (0x439560): REAL, fixed - `memset`'s length
      argument is WORD-truncated in hover.exe (`movzx edx,bx`) though
      every other use of the same computed length
      (`dwPos`/`pBuf`/`wRemain`) stays full-width.** Confirmed
      positionally with `compare.py --verbose 0x439560`: `ebx` = `dwMin
      - dwPos` computed once, `edx = (WORD)ebx` split off ONLY for the
      `rep stosd`/`stosb` byte count, `ebx` itself reused unmasked for
      the pointer/counter updates immediately after. Casting only the
      `memset()` argument, `memset(pBuf, 0x80, (WORD)dwMixLen)`,
      reproduces this with no behavioural change (`dwMixLen` is bounded
      to <= 0xffff here since it starts as `wRemain`, a `WORD`, and is
      only ever reduced from there). 85.0% -> 88.2%; byte-weighted
      unchanged at 82.06% despite the same ~16-function collateral
      drift pattern as rule 391 (this time it happened to net to zero)
      - kept, since it didn't fall.

396. **`Hover_sub_43AEC0` (0x43aec0): REAL, fixed - hover.exe computes
      `wChannels*wBytes` exactly once and keeps it live in `ebx` for the
      whole function (both the upsample and downsample branches, across
      the `Hover_sub_43ADF0`/`Hover_sub_43AD40` calls, and as the
      tail-copy loop counters) where our six separate source spellings
      of the same product each re-multiplied.** Confirmed with
      `tools/disasm.py 0x43aec0`: `imul ebx,[esp+0x10]` at 0x43af0b,
      unchanged through 0x43b064. Naming it `dwFrameBytes` and using it
      at all six sites clears `sign:imul x4`; 54.2% -> 66.1%,
      byte-weighted 82.06% -> 82.08% (net positive this time - same
      collateral-drift mechanism as rule 391, direction varies by
      function).

397. **`Hover_sub_4386D0` (0x4386d0): REAL, fixed - the tan-table
      binary search's break arm should read "<=" like its sibling in
      `Hover_sub_4385F0`, not "==".** hover.exe's `jbe` for this arm
      reuses the preceding arm's `cmp` flags (no intervening
      flag-setting instruction between the `jae` for "<" and this
      `jbe` - confirmed with `tools/disasm.py 0x4386d0`); spelling it
      "==" forces a fresh compare (`je`), which is what our build did.
      `Hover_sub_4385F0`'s near-identical search already spells this
      arm "<=" (both logically equivalent here, since the first arm
      already excludes "<"). Fixing it: 33.1% -> 34.3%, byte-weighted
      unchanged at 82.08%. Fixing this exposed a second, previously
      cancelling-out `jcc:eq` lead in the same function: the `if (lX <
      lY)` a few lines later compiles in hover.exe to a redundant
      `jg`+`je` pair off one `cmp` where our build (and
      `Hover_sub_4385F0`'s *identical* "if (lX < lY)" line, on both
      sides) uses a single `jge` - ruled NOISE, same class as rule
      363's "scheduling-driven, not source-spelling-driven": the
      sibling function proves this specific comparison doesn't need two
      jumps, so hover.exe's choice here is compiler-scheduling noise
      local to this compilation, not a source difference.

398. **`Hover_sub_439A80` (0x439a80): REAL, fixed - "i >= nCount", not
      "i == nCount", for the block-scan's out-of-blocks arm.** Same
      idiom already on record in `Hover_sub_439E80`'s comment for the
      identical scan pattern shared with `Hover_sub_439180` - this
      function just hadn't been brought in line with it. Clears both
      `jcc:eq` (ours) and `jcc:signed` (original's) in one edit, since
      `i` can only ever reach `nCount` by incrementing past it one step
      at a time (`==` and `>=` agree). 76.4% -> 77.8%, byte-weighted
      82.08% -> 82.09%.

399. **`Hover_sub_439180` (0x439180) and `Hover_sub_4393A0` (0x4393a0):
      each has one `loop`-class flowcheck lead (a conditional back-edge
      we have that hover.exe lacks) that does not reduce to a source
      difference - UNRESOLVED/NOISE, new sub-class: shared-epilogue /
      merge-point layout choice.** In both cases hover.exe reaches a
      shared piece of code (a `return 0;` epilogue for 0x439180's
      `for(;;){...; if(nErr==0) break;}` loop; the shared `for` setup
      for 0x4393a0's `nMode==1`/`nMode==2` arms) via two FORWARD jumps
      from different callers, while our build never merges the
      duplicate epilogue/setup code at all (0x439180: confirmed with
      `objdump` on our own build - the loop's `xor eax,eax;ret` at
      0x403caf and the function's other, later `xor eax,eax;ret` at
      0x403d1a are two separate, unmerged code blocks) or lays the
      shared block out such that one caller must jump backward into it
      (0x4393a0: `objdump` shows our `nMode==2` arm jumps backward into
      the middle of the `nMode==1` arm's inline code to reach the
      shared `for`-loop entry, where hover.exe places that entry after
      both arms so each reaches it with a forward jump). For 0x4393a0,
      rewriting the `else if (nMode != 2) {error}` as an explicit
      `else if (nMode == 2) ; else {error}` - a plausible source
      hypothesis for the three-way symmetric dispatch hover.exe's
      layout implies - was tried and produced byte-identical machine
      code for this function (objdump confirmed), so the layout choice
      is not driven by this spelling; reverted (no-op, but non-idiomatic
      style not worth leaving in). Both left as-is: this looks like
      MSVC 2.1's identical-code-folding/epilogue-sharing heuristic,
      which (per rule 363's precedent for register-hoist scheduling)
      is compilation-instance-driven, not reliably reproducible from a
      single function's source spelling.

400. **Session covering `tools/flowcheck.py` leads on nine
      `src/hover.cpp` functions (bundle 13): five real fixes (rules
      382, 385-388), one behavioural-equivalent fix tried and reverted
      for a net byte-weighted loss (rule 391), two noise verdicts (rules
      383-384), and one function with two UNRESOLVED loop-layout leads
      (rule 399) - hit rate five (kept) of nine, six of nine settled
      with a real source-level finding either way.** Confirms rule
      381's collateral-drift mechanism (a size change to one function
      shifting `tools/compare.py`'s match scores for ~15-25 unrelated
      functions elsewhere in the same object file, sometimes by 20-30
      points on an individual function) is not an isolated fluke: it
      recurred, in both directions, on two more fixes this session
      (rules 385, 386) - net effect varies per change and must be
      re-measured with the full-file `byte-weighted` number every time,
      never assumed from a single function's own before/after.

401. **Control, done properly this time: a `DWORD`->`int` field retype that
      touches no new type-table entry costs nothing, even in hover.h.**
      Rule 346 measured a -0.58 byte-weighted toll from adding a brand-new
      user-defined type (an unused `FIXPT` typedef) to hover.h, and warned
      never to introduce a shared-header type for a gain smaller than that.
      That toll does not apply to substituting one already-known fundamental
      type for another. Retyping `LVL_PARAMS::m_dwUnk28`/`m_dwUnk2C` from
      `DWORD` to `int` - chosen because every read site already spells an
      explicit `(int)` cast, so the change is 100% call-site-inert - and
      rebuilding measures Accuracy 90.77% and byte-weighted 82.1117%,
      bit-for-bit identical to the untouched baseline (`tools/compare.py`
      reports the exact same 938/940 Implemented and the exact same
      82.11% before rounding). `int` and `DWORD` are both already in every
      translation unit's type table before either field is touched, so there
      is no new entry for the compiler's per-TU numbering to absorb and
      nothing downstream to re-roll. Practical consequence for future header
      passes: the toll from rule 346 is specific to adding a type the
      program did not already use (a lesson rule 350 already drew for
      `class CCollider;` forward declarations); swapping the signedness or
      width of an *existing* fundamental-type field is free by this measure
      and should not be avoided on toll grounds. (Control reverted before
      the real change below was made; not committed.)

402. **Rule 391 taken on retry: `LVL_PARAMS`' ten signed fields
      (`m_dwUnk30`/`34`/`38`/`3C`/`40`/`44`/`48`/`4C`/`50`/`54`) retyped
      `DWORD` -> `int`, Accuracy 90.77% -> 90.78%, byte-weighted
      82.1117% -> 82.1123%.** Rule 391's own evidence (all six
      `* 9 / 10` / `* 11 / 10` divisions in `CHoverApp::CHoverApp`,
      0x41c100, compile to `cdq`/`idiv` in hover.exe) was re-confirmed
      directly in the disassembly before touching anything, including the
      two not shown in rule 391's text (`m_dwUnk40`/`m_dwUnk4C`, both
      `imul` by 9 then `cdq`/`idiv edi` with `edi=10` at 0x41c45c/0x41c4b0).
      Rule 391 only cast the six dividends at their point of use and reverted
      because that was measured to knock ~22 unrelated functions elsewhere
      in hover.cpp by 20-30 points each; this pass instead retyped the
      fields themselves (declaration in hover.h, both `FX_INT` stores at
      hover.cpp:568/571 now unneeded of their stray `(DWORD)` cast, the
      already-int-cast loop bounds at texture.cpp:914/917 with their now
      redundant `(int)` removed, mazeobj.cpp:1599's stray `(DWORD)` cast
      around `CObList::GetCount()` - which already returns `int`
      (AFXCOLL.H) - removed, and mazeobj.cpp:1554 given one new cast, see
      below). Result: only 11 functions move by more than a point (not
      ~22), the largest five being 0x438820 (+33.3, now byte-exact),
      0x43ad40 (+16.2), 0x43a860 (-11.5), 0x43abc0 (-6.6) and 0x43dbe0
      (+6.2) - see rule 403 for why this collateral is both smaller and a
      different kind of thing than rule 391's. `CHoverApp::CHoverApp`
      itself moves 71.83% -> 73.77%, clearing `tools/flowcheck.py`'s
      `sign:cdq x6` / `sign:idiv x6` / `sign:div x6` (extra) lead
      completely (confirmed both before and after with
      `tools/flowcheck.py --only sign --counts 0x41c100`). Independent
      confirmation from the read side, per this pass's addendum:
      `tools/flowcheck.py --only sign` on the four functions that read
      these fields without dividing them (`CFlag::Sub_42AD40` 0x41ae40,
      `CSub42F640::OnChangeLevel` 0x42fb60, `CBumperView::Sub_4282F0`
      0x4282f0, `CBumperDoc::OnOpenDocument` 0x415450) shows zero
      discrepancy in either direction before or after - consistent with
      `int`, and not evidence either way for `int` over `long` (both are
      32-bit and indistinguishable to this compiler in every expression
      here; `int` was picked over `long` because every existing reader
      cast is `(int)`, never `(long)`, and no reader assigns these fields
      to a `long`-typed local - the `long lLimit`/`lCount` locals in
      `CFlag::Sub_42AD40` belong to an unrelated gauge member, not to
      `LVL_PARAMS`). One new cast was required and is not stray:
      mazeobj.cpp:1554 compares `m_dwUnk30` (now `int`) against the
      untouched `DWORD m_dwUnk83E8`, which the compiler did not warn about
      before (both `DWORD`) and now flags as C4018 signed/unsigned
      mismatch at this project's `/W3`; hover.exe's own comparison at this
      site is a sign-agnostic `cmp`/`jne` equality test, so casting either
      side is behaviourally and, per the verbose diff, byte-identically
      inert - `(int)theApp.m_pBumperDoc->m_dwUnk83E8` was added to silence
      the warning without touching `m_dwUnk83E8`'s own (out-of-scope,
      unproven-signed) type. `tools/callcheck.py` and a whole-program
      `tools/flowcheck.py --counts` both report the identical function
      count differing before and after (25 and 65 respectively) - no
      overload, operator or call target changed anywhere in the program.
      Build introduces no warning beyond the two pre-existing ones. Kept:
      both metrics rise, on the real-codegen figure (rule 403).

403. **Correction to rule 391's collateral diagnosis: the ~22-function,
      20-30-point swings were real codegen caused by the dividend-cast
      trick's own byte-count mismatch, not `tools/compare.py` annotation
      drift - and a complete retype avoids almost all of it.** This pass's
      addendum asked whether rule 391's collateral was a measurement
      artefact (annotation drift: same bytes, different `<OFFSETn>`/name/
      jump-table rendering) rather than real codegen. Per rule 402 the
      collateral this time is only 11 functions, not ~22, and inspecting
      `tools/compare.py --verbose` before/after (saved in
      `notes/pass14-verbose/{before,after}.txt`) on all five largest movers
      shows every one is REAL codegen - different registers or different
      instruction ordering at the same offsets, never merely a different
      annotation on identical bytes: 0x438820 drops two loads-through-a-
      pointer (`mov ecx,[g_dwUnk_4C59BC]`/`test ecx,ecx`) for a direct
      `mov eax,[g_dwUnk_4C59BC]`/`test eax,eax` and becomes byte-exact;
      0x43a860 swaps `edi`/`ebx` throughout a sixteen-instruction free-list
      walk with no other change (a pure register-allocation flip, rule
      328/352's class); 0x43dbe0 swaps `ecx`/`edi` the same way. None of
      the five have a drifted `<OFFSETn>`, a differently-resolved global
      name, or a jump-table entry rendered differently - so the
      byte-weighted figure in rule 402 (82.1123%) already *is* the
      real-codegen figure, with nothing to subtract for drift. The
      difference from rule 391 is not the checker; it is the input. Rule
      391's dividend-only cast changed six `div`s to `cdq`+`idiv` (+6
      bytes) while leaving every other instruction in
      `CHoverApp::CHoverApp` exactly as compiled for `DWORD`, so the
      function's total byte count no longer matched what a genuinely
      `int`-typed field set would compile to; PDB/line-table driven
      matching (`tools/compare.py`/reccmp) then staged and scored later
      hover.cpp functions against a hover.exe layout that had shifted by
      an amount inconsistent with the local source, producing large,
      chaotic swings. Retyping the fields (this pass) makes
      `CHoverApp::CHoverApp` compile the way `int` fields naturally would
      throughout, so the shift downstream is the ordinary, small kind rule
      328/352 already documented for any change to an earlier function in
      the same translation unit - real, but an order of magnitude smaller.
      Lesson for tooling: nothing here indicates a `tools/compare.py`/reccmp
      bug to fix; a half-applied type change (source-level cast without a
      matching field retype) is simply a worse-conditioned input than a
      full one, and the METHOD step "make the change completely and
      consistently" (this pass's brief, step 2) is sufficient defence
      against it. No tooling change is recommended.

404. **Differential frame test: for one seed and level, our build's first
      in-game frames are PIXEL-IDENTICAL to hover.exe's.** `tools/difftest/`
      runs both binaries under the same scripted input and diffs the
      captured window. The game seeds `rand()` from `time()` at two sites
      (`CSub424BD0::Sub_433CF0` and the maze loader at hover.exe:0x4154D5,
      whose `rand()` picks the level and the spawn points), so two runs
      never match by default; `pepatch.py` rewrites the CRT `_time` body in
      TEST COPIES of both executables to return a constant (libfaketime
      hangs Wine's 32-bit client in every configuration tried - do not
      retry it). Measured at master d166dcf with seed 0x3039: the menu
      frame and the two static in-game frames (`start3`, `start10`: maze
      geometry, textures, floor, ceiling, sky, sprites, mirror, minimap,
      HUD) differ by ZERO pixels between our build and the original, and by
      zero between two runs of ours. The `turn` stage differs by 2-94
      pixels of key-timing jitter, the same magnitude ours-vs-ours. The
      `drive` stages depend on how far the craft travelled under Wine
      scheduling and are exact (0-16 px, the minimap marker and one mirror
      pixel) whenever that coincides. **What this certifies:** the whole
      frame pipeline - loader, BSP walk, span renderer, texture mapping,
      flat fills, sprites, dashboard composition - produces the original's
      bytes for this level from a static camera. **What it does not:**
      other levels and seeds (run it with other constants), behaviour over
      time (robot AI, physics, scoring; needs a deterministic tick, which
      `GetTickCount` is not), sound, dialogs other than the menu. Read the
      `drive` numbers only against the ours-vs-ours noise floor. **Extended 2026-09-13 at master 0aed06e over three more seeds
      (1, 0xff, 0x12345 - three further levels and spawn points): `menu`,
      `start3` and `start10` are pixel-identical to hover.exe at every seed.**
      The `turn` stage is jitter-limited, not a divergence: at seed 1 two
      runs of OUR OWN build differ wholesale at `turn` (108,806 px) while a
      matching pair of runs (oursB vs origB) differs by ZERO pixels there,
      so eight timed taps sometimes land as a different number of key
      events under Wine; a 28-pixel `start3` difference between two of our
      runs is a robot sprite caught one animation step apart (robots move on
      GetTickCount). Four seeds, four levels, zero pixels of difference in
      the static frames: the frame pipeline is certified across levels, and
      any future `turn`/`drive` comparison must be read against a same-binary
      repeat, never against a single run.

405. **`CSub406090::Sub_406090` (0x406090, 65.0%): full behavioural audit
      turns up nothing new to fix - every operand, field offset, constant,
      list end, call, and argument order was checked against the original
      and matches (the divisor 0x55, the 0x2AAAAB/0x1FFFFFF masks, the
      memset size, the sprite screen-space math, all sixteen jump-table
      handlers already verified by rule 362). The 12-byte frame growth
      (`sub esp,0x5c` vs our `sub esp,0x68`) traces to three extra stack
      slots (`ebp-0x24`/`-0x28`/`-0x2c`) the sprite-projection loop uses to
      cache `pCam->m_dwUnk48`/`m_dwUnk4C` and `nScale` before the
      `dy -= pCam->m_dwUnk4C` / `dx * nScale` arithmetic; the original
      keeps `pCam` (or the scalar) live in a register across the same span
      instead. Extends rule 362's "hoisted address splits one displacement
      into two, both directions" entry with the exact new slots. Tried
      reordering the block's `nDX;nDY;dx;dy;nScale;fDist` declarations
      (rule 195's 16-bucket hash theory) hoping for a different spill
      pattern: zero effect on the generated code, reverted. At the ceiling
      rule 362 already documented for this function.

406. **`CBumperView::Sub_41ECE0` (0x41ece0, 60.7%): full audit, no
      behavioural bugs; the new `opcheck`/`flowcheck` leads are all rule
      362's jump-table-vs-cmp-chain class.** `imm:cmp:0x1`/`0x5`,
      `jcc:unsigned`, `jcc:eq x3` and `loop x3` are the same "hover.exe
      builds an 8-slot jump table here, our build falls back to `cmp`/`je`
      chains" mechanism rule 362 already decoded byte-for-byte; nothing
      new to re-derive. The `arg:...CreateWindowExA:1="DisplayDibWindow"`/
      `:2=""` flags are a data-naming artifact, not a missing argument: the
      original's identical string bytes for both literals already live
      inside two *other* named globals (`g_szUnk_4C47B0+12`,
      `g_dwUnk_4C4CF0+16` per `tools/compare.py --verbose`) and are pushed
      as `OFFSET`s into them, so the checker only sees a `STRING` push on
      our side, where each literal gets its own fresh constant. Confirmed
      the pushed values are the correct strings by reading the push order
      and comparing against the source. Still at the ceiling rules 156 and
      362 documented.

407. **`CBumperView::Sub_402250` (0x402250, 59.8%): rules 255/360's fixes
      (the `m_dwUnk0540` guard, the mirror-view `+0x100` argument, the
      `m_dwUnk0080` reset, the single post-`PatBlt` `GdiFlush`) are all
      still present and correct - re-verified by reading the whole
      function. One new, minor, NOT-behavioural finding from `opcheck`'s
      `imm:mov:0x288`: hover.exe:0x402309 stores the `fxOffset.dw = 0x288`
      constant to a stack slot (`ebp-0x2c`) that is never read again
      (confirmed - no later instruction touches it), while our /O2 build's
      optimizer deletes that now-provably-dead store outright; the actual
      `fxArg.dw = fx.dw + 0x288` computation folds to the identical
      `add eax, 0x288` immediate on both sides, so the values are
      unaffected - a VC-version dead-store-elimination difference, not a
      bug. Tried declaring `fxOffset` `volatile` per rule 185's "a constant
      initialiser needs a volatile destination": this does force the store
      back, but a volatile destination also forces the *read* to become a
      real memory operand (`add eax, dword ptr [ebp-0x2c]`), where the
      original still uses the bare immediate - trading one mismatch for a
      different one, net regression 59.8% -> 59.7%, reverted. The
      remaining diff (the `edi`-vs-`ebx` register-role swap for the cached
      `pDoc`/base pointer, `disp+0x8`/`0xc`, `imm:add:0x1ac`, the
      `m_dwUnk00D4`/`00D8` camera pair) is rule 362's already-documented
      hoisted-address/register-choice noise for this exact function
      (explicitly named there: "the `m_obj0150` viewport RECT and the
      `pdwDest`/`pdwCam` D4/D8 pair").

408. **`CSub422F50::Sub_401520` (0x401520, 74.0%): full instruction-level
      re-audit of all four clip-edge cases (top/bottom/left/right) finds
      no wrong operand, offset, or constant anywhere.** Every line of the
      diff against the original is one of two shapes: a swapped-operand
      compare that tests the identical condition (`cmp ebp,[ecx]/jge` vs
      our `cmp [ecx],ebp/jle` - both "branch when `lX1 >= pdwClip[0]`"),
      or a difference in how the `B`-endpoint pointer is kept live
      (`ebx`/`edi` register vs a `[esp+0x1c]`/`[esp+0x10]` reload/spill).
      Confirms rule 121's `volatile` clip-pointer fix already captures the
      function's one real structural win (52% -> 71%, now 74% after later
      passes); the rest is -O0-under-/O2 scheduling noise in rule 362's
      family. Nothing new to fix.

409. **`CBumperView::Sub_40CCE0` (0x40cce0, 68.9%): re-confirmed rule 384's
      "pszA is drawn unconditionally" fix is present and correct**, and no
      `opcheck`/`flowcheck` leads exist for this function at all - the
      cleanest of this session's eight targets. At the ceiling rule 384
      already found.

410. **`CSub406090::Sub_405CE0` (0x405ce0, 53.8%): `opcheck`'s only two
      flags are both already-named classes for this exact function, not
      new leads.** `imm:cmp:0x0` is rule 362's `cmp x,0` vs `test x,x`
      class (explicitly listed for 0x405ce0 there); `imm:shl:0x2` is the
      `shl reg,2` vs `lea reg,[reg*4]` scale-by-4 fold (also explicitly
      listed for 0x405ce0 in rule 362). Rule 146's `setns`/branch drift is
      a different, sibling function (`Sub_405B90`) and does not apply
      here. No probe lever found; left unchanged per the wave brief's
      "only if a probe shows a lever" guidance.

411. **`CBumperView::Sub_4184B0` (0x4184b0, 68.2%): `opcheck`'s only two
      flags, `disp+0x214` and `imm:shl:0x2`, are rule 362's own examples
      for this exact function** (the `pZone->m_rcUnk210` RECT-copy
      hoisted-address split and the scale-by-4 `shl`/`lea` fold). A full
      read of the zone-lookup, render-target-recreation and dual-viewport
      setup skeleton against the original turned up no further
      behavioural divergence: the six-descriptor bottom-tested search
      loop, the 2x-stretch-mode 320x240 override, the render-target
      recreate-and-reselect sequence, and the `nSub`/`dwMidX`/`dwTopY`
      geometry all match field-for-field. At the ceiling rule 362 already
      documented.

412. **`CSub422F50::Sub_40FEE0` (0x40fee0, 60.1%): `flowcheck`'s
      `sign:movsx x2` finding is real but value-inert, and forcing it
      costs the whole file.** hover.exe explicitly sign-extends
      `fxX.s.sInt`/`fxY.s.sInt` to 32 bits (`movsx`) before the
      `sExtent = max(fxX.s.sInt, fxY.s.sInt)` comparison; our compiler
      proves the promotion unobservable for a signed compare of two
      equal-width operands and narrows it to a direct 16-bit
      `cmp word ptr/jle` - identical result on every input, a pure
      instruction-selection choice. Adding explicit `(int)` casts at the
      comparison to force the promotion did change our codegen, but the
      change rippled through the rest of `bumpview.cpp`'s translation
      unit: byte-weighted fell 82.11% -> 81.98% (Accuracy 90.78% ->
      90.74%) while 0x40fee0 itself stayed at 60.1%; reverted. Concrete
      instance of rule 390's warning: a two-token, functionally-inert
      change this far into the file can move the whole-table
      byte-weighted total by a tenth of a point, so it must always be
      measured against the full table, never the target function alone.

413. **`Hover_sub_439E80` (0x439e80, 73.0%): reconfirmed at its documented
     ceiling; one new negative recorded.** Full skeleton audit (original
     vs. ours, behavioural-first) turned up nothing beyond what rules 194
     and 365(b) already record, and `callcheck.py`/`opcheck.py --presence`/
     `flowcheck.py` agree: `opcheck` and `flowcheck` each report exactly
     the rule-365(b) `and:0xffffffca`/`and:0xca` register-width pair and
     nothing else, `callcheck` reports nothing for this function at all.
     New negative: rule 194 documents that the three-`volatile`-local
     frame keeps the return-code local out of hover.exe's middle slot no
     matter how the three are declared, assigned or renamed; the same
     insensitivity extends to a second, unrelated construct in the same
     function - swapping the append-arm's `if (pCur != NULL) {walk} else
     {empty}` to `if (pCur == NULL) {empty} else {walk}` (hover.exe places
     the non-empty/list-walk arm as the fallthrough and the empty arm
     out-of-line with a forward branch; our build does the opposite)
     produces byte-identical output before and after the swap - the
     fallthrough-arm choice here is also a pure `/O2` block-layout
     heuristic, not source-order-controllable. No behavioural differences
     found; no fix taken; 73.0% before and after, byte-weighted 82.11%
     unchanged.

414. **`Hover_sub_4389F0` (0x4389f0, 55.9%): the whole gap is one missing
     stack slot, and it resists every lever tried.** hover.exe's frame is
     `sub esp, 0x14` (five dword slots); ours is `sub esp, 0x10` (four),
     which shifts every `[esp+N]` parameter/local reference in the
     verbose diff by a constant 4 and is why the match looks so much worse
     than the actual instruction-level divergence (mechanically confirmed
     clean: `opcheck.py --presence` and `flowcheck.py` both report zero
     findings for this function; `callcheck.py` does not list it either).
     In the stereo/multi-source branch, hover.exe keeps `wOff` (word) and
     `wFrames` (word) in two **separate** dword-aligned slots 0x10E bytes
     apart; our build packs both words into one shared dword slot,
     accounting for the missing slot exactly (the other four slots -
     nAccR, the `i` counter, and a masked/widened copy of `wOff` cached
     before the inner loop - already match hover.exe's slot-for-slot,
     confirming the cached-widen local is not a missing temp, just
     unusually already present). Tried and reverted, each a full
     rebuild+compare: (a) introducing an explicit `DWORD dwOff` local
     holding the widened copy and using it in place of `wOff` inside the
     inner loop - compiles to byte-identical code for this function (the
     compiler already keeps the same value in a register regardless);
     byte-weighted ticked 82.11%->82.13% purely from `compare.py`'s
     matcher re-scoring four unrelated functions elsewhere in the file
     (rule 391's "matcher... apparently sensitive" phenomenon, confirmed
     by diffing the full before/after table - no real fix, so not kept).
     (b) `volatile WORD wOff` alone: 55.9%->36.8%, byte-weighted
     82.11%->82.05% - reverted. (c) `volatile WORD wFrames` alone:
     55.9%->49.9%, byte-weighted 82.11%->82.09% - reverted. (d) swapping
     the two locals' declaration order: no effect (expected at `/O2`,
     where stack-slot assignment for spilled scalars follows liveness/use
     shape, not declaration order). No source change kept; open for a
     future pass that wants to force the split (a hand-written pointer
     alias or a wrapping struct might separate the two words without the
     `volatile` regression, untried here).

415. **`Hover_sub_43B150` (0x43b150, 83.2%): confirmed rule 358 is the
     whole remaining gap.** `opcheck.py --presence` and `flowcheck.py`
     agree: flowcheck's only finding is the documented
     `arg:reg:2="WAVE"` vs. `arg:...FindResourceA:2="WAVE"` register-
     cached-import pair (rule 358(b)'s exact mechanism - the resource-type
     string is pushed through a cached `FindResourceA`/`LoadResource`
     register pair in hover.exe, through the named import in ours), and
     `callcheck.py` does not list the function. Manual read of the whole
     verbose diff found one more instance of the same mechanism at the
     tail cleanup (`if (pWave) {GlobalUnlock/GlobalFree...} if (pData)
     {GlobalUnlock/GlobalFree...} if (hResData) FreeResource...`):
     hover.exe loads `GlobalHandle` into `edi` and `GlobalFree` into `esi`
     once and reuses `call edi`/`call esi` across *both* the `pWave` and
     `pData` cleanup blocks (they are structurally identical calls),
     while our build reloads the import addresses per block. Same class,
     not independently fixable at the source level (which import gets the
     register-cache treatment, and for how long the register survives
     across blocks, is the linker/compiler's own scheduling choice - rule
     358 already documents this is not source-spelling-driven). No fix
     taken; 83.2% before and after.

416. **`CHoverApp::CHoverApp` (0x41c100, 73.8%): the constructor is
     unoptimized (`/Od`-shaped) code in hover.exe, same as
     `CHoverApp::InitInstance` - confirmed by three independent tells,
     and the naive fix (the pragma alone, without a rule-160/195 frame
     rebuild) regresses hard rather than helping.** Tells, all in the
     disassembly: (1) `this` is homed to a dedicated frame slot
     (`mov [ebp-0x14], ecx`) at entry, the same signature rule 79/rule
     274 already document for other `/Od` constructors; (2) a full SEH
     prologue/epilogue (`mov eax,fs:[0]` / three pushes / `mov fs:[0],esp`
     ... `mov fs:[0],ecx` at the tail) around a fixed `sub esp, 0x54`
     frame; (3) the clinching one - `LVLPARAM(i)`'s `((i)<16) ? (i) :
     ((i)%3+15)` index (already correctly spelled in `hover.h`) is
     **recomputed from scratch with a fresh `cmp`/`idiv` at every single
     field access** (nine times per loop iteration in the 0-15 loop,
     where `i<16` is a loop-invariant fact the compiler could trivially
     prove and never does), which no optimizing pass would leave
     un-CSE'd. The second loop (levels 16-19, `ecx` from 16 to 19) was
     mapped byte-for-byte: its "constant" source addresses
     (0x4c4558-0x4c457c) are not a separate table - they resolve exactly
     to `g_lvlParams[15]`'s own fields (`g_lvlParams` spans
     0x4c4000-0x4c4738, stride 0x58/88 bytes per rule confirmed by
     `21*0x58==0x738`; `0x4c4558-0x4c4000 == 15*0x58+0x30`, etc.), so our
     current `LVLPARAM(i).field = g_lvlParams[15].field` for `i=16..19`
     is the behaviourally correct copy (indices 16 and 17 get written,
     index 15 gets a harmless self-copy at `i=18`, index 16 gets written
     twice at `i=16` and `i=19` - all consistent with the `(i%3+15)`
     wraparound) - no behavioural bug there, only the recomputation shape
     differs. **Tried and reverted:** wrapping just the constructor body
     in `#pragma optimize("", off)` (matching `InitInstance`'s treatment)
     with no other change: 73.8% -> 15.8%, byte-weighted 82.11% ->
     81.66% - a severe regression, because every local's frame offset is
     now wrong (VC 2.1's `/Od` hash-bucket local order, rule 195, was not
     reproduced), confirming the pragma is necessary but nowhere near
     sufficient; the InitInstance precedent (rule 160's struct-grouping
     trick) is the correct next step, not attempted here for lack of
     budget. Frame data points recorded for that future pass, all
     ebp-relative in the original: `this` at `ebp-0x14`; SEH scope index
     at `ebp-4`, old handler chain at `ebp-0xc` (compiler-managed, not
     user locals); `SYSTEM_INFO sysInfo` spans `ebp-0x60` upward (its
     `dwProcessorType` field lands at `ebp-0x48`, i.e. `sysInfo+0x18`);
     the two `HOVER_FIXMULWP` macro expansions' internal `_pA`/`_wB`
     temps land at `ebp-0x2c`/`ebp-0x10` (first call, 1000) and
     `ebp-0x28`/`ebp-0xe` (second call, 2500); `dwProd1` at `ebp-0x1c`
     (low half) / `ebp-0x1a` (high half, read by `FX_INT`); the loop
     counter `i` stays in `esi` for the whole 0-15 loop rather than
     spilling to memory (so this is not *pure* `/Od` register discipline -
     `#pragma optimize("", off)` in VC 2.1 evidently still keeps simple
     loop induction variables in registers, worth confirming against
     `InitInstance`'s own loop, if it has one, before the next attempt).
     `opcheck.py --presence`/`flowcheck.py` on the (reverted, current)
     build report only the already-catalogued inlining-depth noise (rule
     357's `CWinApp::CWinApp` entry) and the `data:g_lvlParams`/`disp:`
     findings that are this same optimized-vs-unoptimized shape mismatch,
     not independent leads. No fix taken; 73.8% before and after.

417. **The rule that assigns a blit's frame homes in hover.exe:0x407430,
    with the complete per-expansion inventory.** Rule 290 named the shape;
    this pins it. For each inlined `CDC::BitBlt`/`CDC::StretchBlt`, the
    arguments that need memory get one 4-byte pool slot each, allocated in
    **parameter order at descending addresses** (parameter 1 highest, the
    enregistered parameters simply skipped, so the surviving slots are
    address-contiguous even when the parameters they belong to are not),
    and the stores are emitted lowest-address-first. Read off all sixteen
    expansions of hover.exe:0x407430:

    - `m_rcUnk0220` / `m_rcUnk0230` `StretchBlt` (0x407535, 0x4076??):
      `x, y, nWidth, nHeight, xSrc` - the four doubled values plus the raw
      `left`, which is homed because the doubling scratch (`eax`) clobbered
      it; `left` is then re-read from the object for `x = left*2`.
    - the same two `BitBlt` arms (0x4075e4): `x, y, nWidth`; `nHeight`
      (`bottom-top`, computed in `ebx`) stays in a register.
    - `m_obj0430` / `m_obj0480` `StretchBlt` (0x4077fe): `x, y, nWidth,
      nHeight, nSrcWidth` - here the raw operand that survives in the pool
      is `6*m_dwUnk28`, re-computed (`lea eax,[eax+eax*2] / shl eax,2`)
      for the doubled `nWidth`.
    - the same two `BitBlt` arms (0x4078b0): `x, y, nHeight`; `nWidth`
      (`6*m_dwUnk28`) stays in `ebx`.
    - the three waypoint `StretchBlt`s (0x407adc): `x, y, nWidth, nHeight,
      nSrcHeight`.
    - the three waypoint `BitBlt`s (0x407b7b): `nWidth, nHeight, xSrc,
      ySrc` - **four**, and the destination pair is pushed from the
      registers the values were stored from.
    - `m_obj02B8`: 5 + 3 in the same pattern.

    So every `BitBlt` site homes exactly **three** of its four values and
    leaves one in a register, and which one is left is arbitrary (the
    background rects leave `nHeight`, `m_obj0430` leaves `nWidth`) - except
    the waypoints, which home all four. The `StretchBlt` sites always home
    five: the four doubled destination values plus whichever raw source
    operand the doubling clobbered, and that one is always the *lowest*
    slot of the group.

418. **Why the waypoint `BitBlt` homes four and we can only reach one: VC
    2.x's copy propagation, and it cannot be spelled around.** At those
    three sites hover.exe keeps **two live copies of the same value** -
    `[edi+0x2b8]` is loaded once into `ecx`, stored to `ebp-0x118` (pushed
    as `xSrc`) and *also* pushed straight from `ecx` (as `x`); likewise
    `[edi+0x2bc]` / `ebp-0x11c` / `edx`. `eax` is free at that point, so
    this is not a spill: the allocator simply had two variables holding one
    value and put one in memory and one in a register. Our VC 2.1 coalesces
    any two locals that provably hold the same value before allocation, so
    the second copy never exists and the arm homes only the one value that
    genuinely has to spill (`cy`, because `ebx` takes the `GetSafeHdc`
    result). Measured against master ee65de0 (Accuracy 90.78, byte-weighted
    82.11, 0x407430 45.27%), on top of a per-arm rewrite of the macro:

    - per-arm block locals, `StretchBlt` arm `DWORD _x2,_y2,_cx2,_cy2,
      _cySrc;` assigned lowest-first from the descriptor, `BitBlt` arm
      `DWORD _cx,_cy,_xSrc,_ySrc;` assigned lowest-first, destination
      arguments left as the descriptor expressions - **4+1 slots per site**
      (the four doubled homes land adjacent and in hover.exe's relative
      order; `_cySrc` is coalesced into the general spill area at
      `ebp-0x54` instead of taking the slot below `_cy2`). Frame 0x1b0 ->
      0x1e0. 42.71%, Accuracy 90.74, byte-weighted 82.02. **This is the
      best structural approximation known** - see rule 419 for what else it
      fixes - but it does not pay for itself.
    - the same with the outer `x,y,cx,cy` locals kept and the arm copying
      them (rule 202's literal recipe): the `StretchBlt` arm gets only 4
      pool slots and spills `cy` to the *named* block at `ebp-0x2c`; the
      `BitBlt` arm gets 0. 39.52%, byte-weighted 82.03. Rule 290's "5 slots
      for the `StretchBlt`, laid out exactly like hover.exe's" is only
      reproducible when the arm block reads the descriptor itself.
    - six locals in the `BitBlt` arm (`_cx,_cy,_xSrc,_ySrc,_x,_y`, all six
      initialised from the descriptor, `_x,_y` passed as the destination):
      copy propagation collapses them to four values again, still 1 slot.
      45.20%, byte-weighted 82.07.
    - `volatile DWORD _cySrc` to force the fifth `StretchBlt` slot: the
      volatile gets its slot **and** the argument gets a separate parameter
      home above it, plus a load and a store (`mov [-0xf0],eax / ... /
      mov eax,[-0xf0] / mov [-0xec],eax`). So `volatile` can never add
      exactly one pool slot to an argument list. 42.13%, bw 82.01.
    - all four `BitBlt` locals `volatile` and passed for the destination
      arguments as well: six parameter homes, `pZone` spilled to the named
      block and reloaded per site. 28.19%, bw 81.82.
    - an extra level of inlining (`inline void Hover_MarkerBlt(CDC&, CDC*,
      int cx, int cy, int x, int y)` wrapping the `BitBlt`, in the hope
      that the outer inline's parameters would get homes): VC substitutes
      inline parameters completely, so still 1 slot - and defining the
      helper rotated the whole function's register assignment (`pZone` into
      `esi`, `this` into `edi`, the reverse of hover.exe). 30.03%, bw
      81.82.
    - `(void)&_cx;` does **not** mark a local address-taken for VC 2.x
      (probe: the whole frame is optimised away), so there is no zero-cost
      way to pin a scalar in memory short of `volatile`.

419. **The `GetSafeHdc` temp census of hover.exe:0x407430, corrected, and
    the one pool slot of ours that is already right.** Rule 288 says we
    home fourteen against hover.exe's ten; the true count at master ee65de0
    is **twelve**, and the two pool tops coincide, so the *last-allocated*
    `GetSafeHdc` temp sits at `ebp-0x44` in both builds. Twenty blit
    expansions in source order (St/Bi arms of `m_rcUnk0220`, `m_rcUnk0230`,
    `m_obj0430`, `m_obj0480`, waypoints 1-3, `m_obj02B8`, the final overlay
    and the `WM_COPYDATA` blit), "slot" meaning the
    `mov [ebp-N],0 / jmp / mov [ebp-N],eax` form and "reg" the
    `mov ebx,0 / je / mov ebx,[esi+0xb8]` form:

        site   1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17-20
        orig   S  S  S  S  S  r  S  r  S  r  S  r  S  r  r  S  r r r r
        ours   S  S  S  S  r  r  r  r  S  S  S  S  S  S  S  S  r r r r

    Only site 16 (`m_obj02B8`'s `BitBlt`, `ebp-0x44`) matches. Because the
    pool is anchored at its top (rule 286), getting these ten sites right
    would land ten displacements *regardless of every group below them*,
    which makes it the highest-leverage sub-target in the function - but
    the three disagreeing pairs pull in opposite directions (sites 5 and 7
    need a slot where we use a register, site 15 the reverse), so no single
    source knob moves them. Rule 418's per-arm spelling does fix sites 10,
    12 and 14 (the waypoint `BitBlt`s stop homing the handle, as hover.exe
    does), leaving only 5, 7 and 15 wrong.

420. **hover.exe:0x407430's behavioural audit is closed.** An operand
    census of the whole function - every `[reg+disp]` member offset and
    every immediate outside the image range, counted in both builds -
    yields **no** difference of kind: the same set of field offsets, the
    same constants, the same call targets (`tools/callcheck.py`,
    `opcheck.py --presence` and `flowcheck.py` all report nothing for this
    address). The only entries that differ are *counts* of re-reads
    (`[edi+0x20]` 7 vs 4, `[edi+0x24]` 6 vs 3, `[edi+0x2c]` 4 vs 6 and so
    on), which is exactly what a build that keeps a value in a register
    where hover.exe re-loads it produces, plus the frame-size immediates
    (0x224 vs 0x1b0). Rule 291's inverted gauge guard was the last
    semantic divergence in this function; do not spend another wave looking
    for one.

421. **When a function's whole residual is one frame count, do not trust
    its percentage at all.** Six spellings measured on 0x407430 in wave 15
    scored 28.19, 30.03, 39.52, 42.13, 42.71 and 45.20 against the 45.27
    baseline while moving the pool from 61 slots to between 61 and 76 of
    hover.exe's 92 - i.e. the two spellings that added the most *correct*
    structure (rule 418's first and third) scored below and level with the
    one that added none. Byte-weighted total moved 81.82-82.11 over the
    same set. The reason is rule 286: every pool displacement is a function
    of the total slot count, so a partially-repaired pool is exactly as
    wrong as an unrepaired one and only pays the cost of the extra stores
    and loads. Corollary for anyone taking 0x407430 again: the only change
    worth keeping is one that lands *all* of the deficit in rule 288
    (+27 waypoint homes, +3 raw `StretchBlt` source operands, +3 and +1 for
    the fixed-point macros' dead third slots, +1 for the final overlay's
    `width*2`, -2 `GetSafeHdc` homes), and two of those - the macro pads
    (rule 289) and the waypoint homes (rule 418) - have no spelling in this
    compiler. Treat the function as byte-blocked and spend the wave
    elsewhere.

422. **BEHAVIOURAL BUG FOUND AND FIXED: `Hover_sub_40B6E0`'s two output
      pointers each address *eight* bytes, and `CPlayer::Sub_42B8E0` was
      handing it four.** The helper writes `pdwAlong[0..1]` and
      `pdwSide[0..1]` (hover.cpp:5447), so every call site must pass a
      two-dword buffer. `CPlayer::Sub_42B8E0` (0x42b8e0) declared eight
      separate scalars `dwAX/dwAY/dwBX/dwBY/dwCX/dwCY/dwDX/dwDY` and passed
      `&dwAX` / `&dwBX`; VC placed the `Y` halves in unrelated slots (0x54
      and 0x2c where the callee wrote 0x40 and 0x24), so the callee's second
      dword landed in dead stack and every read of `dwAY`/`dwBY`/`dwCY`/
      `dwDY` returned the zero the prologue had written. In game terms the
      *whole tangential/normal Y component of a player-player collision was
      zero*: `m_awUnk01AC[2]` (the Y velocity) was overwritten with 0 on
      every hit, the tangential exchange handed `Hover_sub_41B7A0` a zero Y,
      and the final `m_awUnk01AC[2] += dwBY` added nothing. hover.exe reads
      the pair as `[esp+0xd8]`/`[esp+0xdc]` and `[esp+0xac]`/`[esp+0xb0]` -
      adjacent, four apart - and zeroes the four buffers a WORD at a time in
      four runs of four (0xc4, 0xac, 0xbc, 0xb4). Declaring them
      `DWORD adwA[2] .. adwD[2]` and zeroing `((WORD*)adwA)[0..3]` fixes the
      behaviour and took the function 38.31% -> 59.30%. **Whenever an out
      pointer is passed, check how many dwords the callee writes through
      it**: a scalar local is only safe for a one-dword output
      (`Hover_sub_41B7A0` and `Hover_sub_4386D0` really do write one).

423. **Rule 101 is solved for `CPlayer::Sub_42B8E0`: the never-written
      frame dwords belong to the *16-bit* expansions, and they come from
      the pointer side, not the factor side.** hover.exe:0x42b8e0's frame
      has four dead dwords at 0x88..0x94 sitting between the
      `HOVER_FIXMULP3` blocks (0x58..0x84, three dwords each, rule 163) and
      the four `HOVER_FIXMULWP` pointer homes (0x98..0xa4) - exactly one per
      16-bit expansion. Rule 163 had recorded that `HOVER_FIXMULWP` has no
      third slot; that is true of its *factor* (which stays in the WORD pool
      at 0x10..0x16) but not of its destination. Reproduce it by handing the
      destination address over as the return value of an inline returning a
      class by value, i.e. rule 163's mechanism applied to the pointer:

          inline void Hover_FixMulWP3(CFixArg fxA, WORD wB)
          {
              _asm mov ecx, fxA.m_dwValue
              _asm movsx eax, wB
              _asm imul dword ptr [ecx]
              _asm mov [ecx], eax
          }
          #define HOVER_FIXMULWP3(dwA, wB) \
              Hover_FixMulWP3(Hover_FixArg((DWORD)&(dwA)), (WORD)(wB))

      Each expansion then owns the pointer home the asm reads back plus the
      dead return temporary, and the factor still lands in the WORD pool.
      That restored `Sub_42B8E0`'s frame to hover.exe's exact 0xcc and took
      it 66.27% -> 76.51%. Note it is *not* universal: the same substitution
      in `CPlayer::Sub_41A6B0` allocates the right number of dead dwords but
      puts them adjacent to each pointer instead of pooling them above, and
      costs 63.76% -> 60.06%. Negative controls: an unreferenced
      `DWORD _dwPad;` in the macro block (bumpview.cpp's spelling) is simply
      deleted by /O2 here and changes nothing at all; probing it first saves
      a build cycle.

424. **A pair of scalars that hover.exe homes at the very top of the frame,
      one of them never stored, is an eight-byte aggregate whose second
      store was dead-eliminated.** `CPlayer::Sub_42B8E0` reads two proximity
      endpoints through pointers. hover.exe stores `[esp+0xcc]`/`[esp+0xd0]`
      for the second and only `[esp+0xd4]` for the first, keeping its Y in
      `ebp` - but the frame still ends at 0xd8, i.e. both pairs own eight
      bytes at the top, above the four output buffers. Reading them into
      four `DWORD` scalars (`dwPX = pMine[0]; dwPY = pMine[1];`) puts them
      in the low scalar pool and shifts every displacement above them;
      `ptMine = Hover_PtOf(pMine)` / `ptProx = Hover_PtOf(pProx)` with the
      16-bit subtractions written straight off the members
      (`(WORD)ptProx.y - (WORD)ptMine.y`) reproduces hover.exe's
      `mov ebp,ecx` / `sub cx,bp` exactly and is worth 59.30% -> 66.27%.
      VC deletes the dead store of the member it keeps in a register but
      keeps the slot, which is why one half looks unused.

425. **Statement order inside a fixed-point expansion's operand list is
      source-controlled.** In `Sub_42B8E0`'s overlap block hover.exe loads
      the sine table entry *before* computing the half-overlap
      (`mov edx,[eax+table]` then `mov eax,0x28 / sub eax,ecx / cdq / ...`).
      Assigning `dwT1 = g_adwSinTable[...]` before `wHalf = (0x28-d)/2`
      instead of after is worth 76.51% -> 79.52% on its own. Cheapest thing
      to try when a diff is nothing but two statements' instructions
      interleaved the other way round.

426. **Rule 222 refined, and it is now a net win: only the *inner* pair of
      `CPlayer::Sub_40ADE0`'s four thrust arms shares the destination.**
      Rule 222 said to delete every per-branch local; rule 254 then reverted
      the whole thing because it cost `CMerlinStatic::Serialize` more than
      it gained. Measured again on the current tree, the arms are not
      symmetric. hover.exe:0x40AE3C falls through to the "no multiply" arm
      and jumps forward over the two `0x20` arms to the single store at
      0x40AEEC, and its frame keeps *three* distinct multiply destinations
      (0x40/0x44/0x48). The spelling that reproduces both is: the innermost
      `m_dwUnk012C == 0` / `else` pair both assign the shared `dwThrust`
      (giving the fall-through and the far jump), while the
      `m_dwUnk0128 != 0` arm and the `lCount != 1` arm each declare their
      own block-scope local and copy it into `dwThrust` (restoring the 0x7c
      frame). 58.36% -> 68.78%, byte-weighted 82.2638 -> 82.2736. Giving the
      innermost multiply arm its own local as well - at block scope or at
      function scope - flips the layout straight back to 58.36%: it is that
      one arm's address-taken shared destination that decides the branch
      order.

427. **`CPlayer::Sub_41A6B0`: name the wall, not the segment.** Our source
      held `CSub40D320* pSeg = &pWall->m_obj04;` and reached everything
      through it. hover.exe:0x41A6B8 loads the `pWall` parameter into `ebx`
      and keeps it live to the `Sub_421B50` tail, recomputing
      `lea ebp,[ebx+4]` for the two `Sub_40C9B0` calls and reading the
      segment angle as `[ebx+0x20]` (rule 375 read this from the other
      side and called the field difference noise - it is, but the *pointer*
      difference is not). Deleting the local and spelling the uses
      `pWall->m_obj04.Sub_40C9B0(...)` / `pWall->m_obj04.m_dwUnk1C` is worth
      61.64% -> 62.84%; the endpoint tail then keeps the wall in a register
      instead of reloading the parameter. A companion fix: the angle helper
      `Hover_sub_4386D0` writes exactly one dword, so its output buffer is a
      plain `DWORD`, not the `DWORD[2]` we had (62.84% -> 63.76%).

428. **Negative results on `CPlayer::Sub_40B0B0` (0x40b0b0), and a clean
      behavioural audit.** All twenty-one steps of the function were checked
      instruction by instruction against hover.exe:0x40b0b0 - guards,
      operands, field offsets, argument orders, the `/-3` arms, the `0x28F`
      and `0x40000` thresholds, the `Hover_sub_408A00` word arguments at
      +0x1ae/+0x1b2, the `Hover_sub_4386D0(&out, [0x1b0], [0x1ac])` order,
      the `0x1FFFFFF` mask and both tail calls - and **no behavioural
      divergence exists**. What is left is a 0xcc-vs-0x98 frame, and these
      were tried and reverted:
      - block-scoped `DWORD* pBrush` locals for the two brush components
        (hover.exe really does `lea ecx,[esi+0x1b4]` before the test and
        addresses the member directly afterwards): 48.64% -> 47.06%. Rule
        224's function-scope verdict holds at block scope too.
      - `dwDx`/`dwDy` as a `DWORD[2]` so the pair goes to the top of the
        frame the way the eight-byte buffers do in `Sub_42B8E0`: 45.53%.
      - declaring `dwDx`/`dwDy` in the innermost block: no change.
      - rule 164's in-memory subtraction for `dwDx`/`dwDy`
        (`dwDx = dwCos; *(volatile DWORD*)&dwDx -= ...`) reproduces
        hover.exe:0x40B3E6-0x40B406's `mov/mov/sub/sub` shape exactly but
        does not move the function (the displacements still differ) and
        costs 0.01 byte-weighted elsewhere; correct but not yet payable.
      - a four-slot `Hover_FixMulP4` (both arguments through `Hover_FixArg`,
        rule 423's mechanism on the pointer *and* the factor) grows the
        frame 0x98 -> 0xbc, and with a four-slot `HOVER_FIXDIVLP` and a
        three-dword angle buffer it reaches hover.exe's exact 0xcc - but the
        score *falls* to 42.6%, so the size is a coincidence and the
        distribution is wrong. /O2 reuses stack slots between expansions
        whose lifetimes do not overlap (the original's blocks sit 8, 12, 16
        and 20 bytes apart in source order), so this function's frame cannot
        be reconstructed by counting macro locals.

429. **Header change still needed and still not made: `Hover_sub_40B6E0`'s
      first two parameters are one eight-byte by-value point.** Both call
      sites in `CPlayer::Sub_42B8E0` show rule 345's five-instruction
      subobject form (`lea eax,[esi+0x1ac] / mov ecx,[eax] / mov edx,[eax+4]
      / push edx / push ecx`) where ours emits four. Introducing a local
      `DWORD* pVel = (DWORD*)&m_awUnk01AC[0];` and calling
      `Hover_sub_40B6E0(pVel[0], pVel[1], ...)` does **not** reproduce it -
      VC copy-propagates the address back into the addressing mode and emits
      the same four instructions (measured: no change to the function,
      -0.02 byte-weighted). The only spelling that works is the real
      by-value parameter, which needs a new type in `hover.h` and is
      rejected by rules 344/346/347. Ten instructions in 0x42b8e0 are
      unreachable until that toll is paid by something else.

430. **A `#pragma optimize("g", off)` function's whole frame can be pinned
     by rule 195's identifier hash, and for `dw`+letter names the bucket is
     exactly `(letter - 'A' + 1) % 16`.** `CSub422F50::Sub_4055B0`
     (0x4055b0) had the right code shape but a scrambled frame: its twelve
     rotation locals landed on ebp-0x1c..ebp-0x48 in bucket order
     (`dwA`=1, `dwB`=2, `dwT2`=2, `dwC`=3, `dwCos`=3, `dwD`=4, `dwT`=4,
     `dwSin`=6, `dwDX`=8, `dwDY`=9, `dwSin2`=13, `dwCos2`=15), not in the
     original's declaration order. Renaming them `dwA`..`dwL` (buckets
     1..12, strictly ascending in declaration order) puts each one on the
     original's slot; the same probe measured `dwP`=0, `dwQ`=1 ... `dwZ`=10,
     and for four-character names `dwXA`=1, `dwXB`=2, `dwXC`=3, `dwXD`=4,
     `dwFX`/`dwNX`/`dwRX`=0, `dwPX`/`dwTX`/`dwDX`/`dwDx`/`dwHX`/`dwTmp`=8,
     `dwSin`=6, `dwCos`=3, `dwSum`=5, `dwDif`=14, `dwSn`=10, `dwCo`=11,
     `dwRes`=11, `dwProd`=3, `dwSinDy`/`dwCosDx`=5, `dwCosDy`=6,
     `dwSinDx`=4, `lQX`=0, `llQ`=5, `lSX`=8, `lPX`=12, `lPY`=13, `lQY`=1,
     `fm1`=9, `fm2`=10, `fm3`=11, `fm4`=12. The nested `{ long lSX;
     __int64 llQ; }` block needed `lQX` renamed to `lSX` so the 8-byte
     `llQ` takes the higher slot (ebp-0x14) and the long the lower
     (ebp-0x18). Worth 45.78% -> 68.16% on its own, with **zero** movement
     anywhere else in the table: renaming locals changes no other
     function (contrast rule 302, which is about their *number*).

431. **The projection tail of 0x4055b0 is three separate statements through
     the three-argument `Hover_FixSubTo(DWORD&, CFixed, CFixed)`, not a
     nested pair of `Hover_FixSub` calls.** Probed the four spellings side
     by side under the pragma: only the `DWORD&`-destination form gives
     `CFixed::SubEq`'s argument a slot of its own (the *highest* of the
     three, above the two by-value parameter copies) and stores the two
     copies right to left - `fmB`, `fmA`, `dwT`, exactly the original's
     ebp-0x7c / ebp-0x80 / ebp-0x78. The value-returning forms
     (`CFixed FixSub(...)`, `DWORD FixSubV(...)`, and the same assigned to
     a named `CFixed`) all fold `SubEq`'s parameter into the `fmB` slot,
     store `fmA` first, and - when nested - add one extra copy per level
     because a struct-returning call has to be materialised before its
     sibling argument is evaluated. Writing it as
     `Hover_FixSubTo(fm1.m_dwValue, m_awUnk006C[8], lSX);
     Hover_FixSubTo(fm2.m_dwValue, fm1, lV1); lSX = Hover_FixValue(fm2);`
     with four `CFixed` locals in a nested block after the `if` puts the
     four destinations on ebp-0x4c..ebp-0x58. `Hover_FixValue` also needs
     rule 49's "copy the result into a local" (`DWORD dwR = fm.m_dwValue;
     return dwR;`): with a plain `return fm.m_dwValue;` the by-value
     parameter folds away, the expansion is two instructions with no slot,
     and the frame comes out 8 bytes short of 0xcc. 68.16% -> 72.98%.

432. **Open: the /Od temporary pool runs bottom-up in hover.exe and
     top-down in ours, and no source shape reaches it.** With rules
     405/406 applied, 0x4055b0's frame is byte-for-byte the original's
     (0xcc, named locals ebp-4..ebp-0x58, 28 argument temporaries
     ebp-0x5c..ebp-0xc8, `this` at ebp-0xcc) and every instruction outside
     the temporaries matches - but the four temporary groups are in
     exactly the reverse order. Ours assigns them in source order from
     ebp-0x5c downwards (X rotation, Y rotation, X projection, Y
     projection); the original assigns Y projection at ebp-0x5c, X
     projection at ebp-0x78, Y rotation at ebp-0x94, X rotation at
     ebp-0xb0, i.e. the *first* statement group gets the *lowest* slots,
     while the order *inside* each group is forward in both. Tried and
     rejected: braces round each group (no effect - a block with no
     declarations is invisible to the allocator), comma-joining the three
     statements of a group into one statement (no effect), giving each
     group its own block *with* locals (no effect), nesting one group
     inside the other in either direction (no effect), `#pragma
     optimize("g"/"gs"/"gt"/"gle"/"", off)` (all identical), and the C2
     from VC++ 2.0, 2.1, 2.2 and 4.2 (all identical, always forward).
     `CRobotPlayer::Sub_409350` (0x409350), the other unoptimized
     function, shows the same symptom from the other side: its first two
     `MulTo` statements descend in source order but its third statement's
     three temporaries sit *above* both. Worth ~40 of 0x4055b0's 293
     instructions (~14 points) and a large share of 0x409350's 4405 bytes,
     so it is the single most valuable open codegen question left.

433. **0x404bb0's frame is exactly 12 bytes short, and the two pieces are
     identifiable but neither pays.** Depth-normalising every `[esp+N]` in
     both images (frame 0x100 + 0x10 of saved registers in the original,
     0xf4 + 0x10 in ours) shows the missing 12 bytes are (a) the eighth
     8-byte `Hover_PtOf` temporary of rule 294 - writing the wall branch's
     last y test as `Hover_PtOf(&m_obj04.m_dwUnk08).y` instead of the plain
     member `m_dwUnk0C` restores it and takes the frame to 0xfc, measured -
     and (b) one dword in the dense scalar region. But (a) alone is
     41.1% -> 40.9% and -0.03 byte-weighted, because the eight bytes move
     every displacement above it. With (a) applied the two frames line up
     slot for slot from E-0x4c to E-0xfc, which shows the rest is a
     *permutation*, not a shift: hover.exe puts the four half-written 16.16
     locals (`dwCosHalf`/`dwSinHalf`/`dwSinCap`/`dwNegCosCap`) at
     [esp+0x90]..[esp+0x9c], just above `adwPt`, and the `lSinCap`/
     `lNegCos` copies at [esp+0x1c]..[esp+0x28]; ours has them the other
     way round. Measured negative: moving those four to the front of the
     declaration list does not move them at all (/O2 slot assignment for
     address-taken scalars does not follow declaration order), and giving
     `lSinCap`/`lNegCos` the original's *two* locals each (it stores the
     `movsx` result into two slots and reads one for `lT` and the other for
     the `lHiCos + lSinCap` sum) costs 41.1% -> 40.5% and -0.40
     byte-weighted.

434. **More negatives on 0x404bb0's reversed clip tests (extends rule
     301b), and a probe that clears casts of suspicion.** The wall branch
     has the same inversion as the pod branch: hover.exe emits
     `mov ecx,[bound] / cmp [temp],ecx / jge`, ours
     `mov eax,[temp] / cmp ecx,[temp] / jle` - same semantics, opposite
     `cmp` encoding. Tried: dropping the `volatile long*`/`lClip` pair from
     the pod branch for plain `(long)adwPt[0] < (long)g_dwUnk_4A47BC`
     (41.1% -> 40.4%, -0.39 byte-weighted); retyping the four viewport
     globals `g_dwUnk_4A4798/479C/47BC/47D8` from `DWORD` to `long` so no
     cast is needed at all (41.1% -> 40.0%, byte-weighted flat) - the
     signed `jl`/`jge` do prove they are signed, but the retype buys
     nothing and was reverted. Probed directly: an explicit `(long)` cast
     on either or both operands has **no** effect on which operand ends up
     in the `cmp`'s memory slot (`(long)a < (long)b`, `a < b`,
     `a < (long)g`, `a < g` and `(long)a < g` all generate the same code),
     so rule 391-style "the cast reordered it" reasoning does not apply
     here. What does decide it is unknown; a local compared against a
     global comes out in the original's order in isolation, the same
     comparison inside 0x404bb0 does not.

435. **Behavioural audit of `CSub422F50::Sub_404BB0` (0x404bb0): clean.**
     Verified instruction by instruction against the original at 41.1%
     match: the four branch guards and their order (`CWall` /
     `CTempWallPod && m_dwUnk28` / `CPodObj` / `CPlayer`); the wall
     branch's `m_dwUnk38 != 0 -> return` versus the pod branch's
     `m_dwUnk38 != 0 -> HidePod` (they differ, and both are right); the
     `bUnk == 0 -> m_dwUnk78 null or (+0x24 & 8) == 0` sub-test in both;
     every clip constant and its direction (left/right on x with
     `jl`/`jg`, top/bottom on y); the temp-wall rect field offsets
     (+0x14/+0x18 and +0x1c/+0x20 off `m_dwUnkD4`); the pod centre and the
     `CFlag` colour selection (`m_dwUnk00AC < 4` clears the visible flag,
     `m_dwUnkCC == 0` picks `m_dwUnk008C` else `m_dwUnk0090`, non-flag
     picks `m_dwUnk0098`); the player branch's `dwT = 0xC80000`,
     `dwCap = m_wUnk0080 * 2`, `max(dwCap, 0x320000)` written as the
     original's `jl`-then-`je` pointer select, `dwCap2 = m_wUnk0080 * 8`,
     `dwT = max(dwT, dwCap2)`, `dwT = (long)dwT / 2`, all four sine-table
     indices (`((a+0x80)&0x1ff)<<4 | 1` and `(a&0x1ff)<<4 | 1`), both
     `g_dwUnk_4A47B8 = lAngle` writes and their positions, and the whole
     triangle: `*pPt1 = *pPt2 = *pPt3 = ptOrg`, `pPt1 += (lHiCos,lHiSin)`,
     `pPt2->x += lSinCap - lHiCos`, `pPt2->y += lNegCos - lHiSin`,
     `pPt3->x -= lHiCos + lSinCap`, `pPt3->y -= lHiSin + lNegCos`. No
     divergence found; everything that is left is /O2 slot assignment and
     register naming.

436. **Sharpening rule 302 with a measured price list for bumpview.cpp.**
     Adding four block-scoped locals to 0x4055b0 re-rolls the register
     allocation of the rest of the translation unit by a fixed pattern:
     `CBumperView::OnUpdateGamePause` 97.78 -> 71.11,
     `CBumperView::Sub_407430` (4345 bytes) 45.27 -> 31.64,
     `Hover_sub_411770` 51.96 -> 38.46, `CSub422BD0::Sub_40FCE0`
     69.95 -> 60.44 against `CBumperView::Sub_4282F0` 84.06 -> 95.65,
     `CSub422C30::Sub_40E9A0` 44.33 -> 49.48, `CSub423030::Sub_401E80`
     40.28 -> 45.14 - net -0.24 byte-weighted, dominated by Sub_407430's
     4345 bytes. The *same four locals declared as `CFixed` instead of
     `DWORD`* cost only -2.06 on Sub_407430 and nothing else (net -0.01),
     so the re-roll is sensitive to the local's type as well as the count.
     Renaming locals (rule 430) costs nothing at all. Practical rule for
     this file: a change that adds locals must be worth more than
     0.3 byte-weighted on its own function before it is worth taking.

437. **`jmp $+5` is an inline *function*'s return, and counting them is how
      you find the original's inline structure.** hover.exe's level loader
      (`CBumperDoc::OnOpenDocument`, 0x415450) contains 73 five-byte
      `jmp $+5` instructions; our reconstruction contained 36. Every one of
      them marks the end of an expanded inline whose return value was
      *materialised* into a frame temporary - VC 2.1 emits the store, then
      the jump to the expansion's end, then the caller's reload. An inline
      whose value VC can propagate instead (see rule 438) emits no jump at
      all, which is why our count was half. Two consecutive `jmp $+5` mean
      two nested expansions (`CString::operator LPCTSTR` inside a wrapper).
      A `#pragma optimize("", off)` around the function changes *nothing*
      here (measured, and it does work: a small probe compiled with it is
      /Od code), and neither do braces around a one-statement `if`: these
      jumps are not statement markers. Closing the gap took the loader from
      86.13% to 96.22% - all of it from making four expressions go through
      inline functions the original had and we did not.

438. **How to make VC 2.1 materialise an inline's result: give the inline a
      local and return the local.** `inline T f(...) { return <expr>; }` is
      folded into the use site - no temporary, no `jmp $+5`, and in a loop
      *condition* VC will even duplicate the whole use into both arms of an
      inner `?:`. `inline T f(...) { T n = <expr>; return n; }` pins the
      value in a frame temporary at the original's slot. Measured on the
      loader's fifteen `LVLPARAM` sites (rule 439): bare `return <?:>;`
      87.5%, two-`return` `if`/`return` form 87.3% (both duplicate at the
      eleven loop-condition sites), local-then-`return` **92.7%**. The same
      trick supplies the loader's other two missing temporaries:
      `inline LPCTSTR CStr(const CString& s) { LPCTSTR p = s; return p; }`
      for hover.exe:0x416B10/0x416BA2/0x416CEE, and
      `inline BOOL MbNe(int n) { BOOL b = (n != 0); return b; }` for the
      1/0 ladder at hover.exe:0x416B3C. Corollary for frame budgeting:
      `CStr` buys **two** slots (the `CString*` the expression already
      homed, plus the result), `MbNe` buys **three** (its own argument
      home, the flag, and the pointer) - which is exactly hover.exe's
      asymmetric 3+2+2 across the three maze-face name tests.

439. **hover.exe's `LVLPARAM` is an inline function, not a macro.** All
      fifteen sites in the level loader (0x41560C, 0x415A0E, 0x415BD8,
      0x415C4B, 0x415DFF, 0x415F42 and the nine pod loops at
      0x416DFA..0x41748D) emit `<?: into a temp> / jmp $+5 / mov eax,<temp>`
      and then do the `lea eax,[eax+eax*4] / lea eax,[ecx+eax*2]` address
      arithmetic outside, so the inline returns the *index*, not a
      reference. Written as `inline int LvlIdx(int i) { int n = (i < 16) ?
      i : (i % 3 + 15); return n; }` it also drags the whole surrounding
      temporary pool onto hover.exe's slots: the pad-loop compares
      (ebp-0x3b4..-0x3c8), the nine `new` pod objects (-0x3cc..-0x3ec) and
      the fifteen index temporaries (-0x40c..-0x444) all land exactly.
      **Do not move it into hover.h**: an inline function added to that
      header costs 0.51 byte-weighted *even when nothing calls it* (82.11%
      -> 81.60%, an inert control), and routing every translation unit's
      `LVLPARAM` through it costs 0.91 (81.20%). This extends rule 346 from
      types to functions: the toll is on the shared header, not on the kind
      of declaration. Defined file-locally in `src/texture.cpp`, with the
      hover.h spelling `#undef`/`#define`d back immediately after the
      loader, it costs nothing outside the file.

440. **A by-value *pointer* argument of an expanded inline is folded like
      `this`; every other argument kind is homed.** This is the missing
      half of rule 330. hover.exe builds the loader's three spawn points
      (0x415D1D, 0x415E7A, 0x415FBD) with two `movsx` stores straight into
      the argument temporary at ebp-0x2bc/-0x2c8/-0x2d4 and nothing homed
      in between. The ladder, measured on this function:
      - `POINT GetPoint() const { POINT pt; pt.x=..; pt.y=..; return pt; }`
        - **2 dwords/site** (the inline's `pt`, then a copy into the
        argument temporary). Returning a `CPoint` instead changes nothing.
      - `CPoint((short)p->m_wUnk14, (short)p->m_wUnk16)` - **2 dwords/site**
        (both ctor arguments homed; rule 330's measurement re-confirmed).
      - `CPoint(*(const DWORD*)&p->m_wUnk14)` - **1 dword/site**, and the
        wrong instructions: the homed dword is picked apart with
        `shr 16 / and 0xffff / movsx` instead of a second `movsx` from
        memory.
      - a file-local `struct LOCPT : public tagPOINT` whose constructor
        takes the `CMerlinLocation*` and does the two stores - **0 dwords**,
        instruction for instruction the original. The pointer parameter is
        folded into both uses exactly as `this` is, so the object is
        constructed in place in the caller's middle-pool temporary.
      MSVC 2.1 has no named-return-value extension (`T f() return r {}` is
      a syntax error), so a struct-returning inline can only avoid the copy
      by constructing its result in the `return` expression.

441. **The level loader's frame, closed.** Ours was 0x454 against
      hover.exe's 0x44c with `this` at ebp-0x460 instead of -0x458. The
      arithmetic of rule 330 was exactly right and both halves had to move
      together: **-6** dwords of doubled `GetPoint` homes (rule 440) and
      **+4** dwords of materialised temporaries (rule 438) in the pool that
      runs ebp-0x3f0..-0x408, which hover.exe fills as
      `[LPCTSTR1, flag1, ptr1, LPCTSTR2, ptr2, LPCTSTR3, ptr3]`. With
      `LOCPT` at all three spawn points, `MbNe` on the first name test and
      `CStr` on the other two, every slot from ebp-0x3a0 down to ebp-0x45c
      matches hover.exe's except two that are swapped (the ring-closing
      test's flag and the wall-list `GetHeadPosition` pointer, ebp-0x448 /
      -0x44c: hover.exe gives the *later* one the shallower slot, we give
      it to the earlier one). Intermediate measurement, for anyone who
      needs a smaller step: two `CPoint(DWORD)` sites plus one accessor
      site is the *only* mixed spelling that reaches 0x44c on its own, and
      it is worth 86.13% -> 89.79% by itself.

442. **Negatives on 0x415450 from this pass, so nobody repeats them.**
      (a) `#pragma optimize("", off)` around the loader: zero instructions
      change. (b) Braces around a one-statement `if`: zero change. (c)
      `(_mbscmp(...) != 0 ? 1 : 0)` and `pFace->m_dwUnkBC != 0` as a way to
      buy a frame slot: VC folds both, no slot. (d) Five more spellings of
      the `operator LPCTSTR` temporary - `x.Left(n).operator LPCTSTR()`,
      `(const char*)(const CString&)x.Left(n)`, `*&x.Left(n)`,
      `LPCTSTR(x.Left(n))`, `(const char*)x.Left(n) + 0` - all fold; and
      `(const unsigned char*)x.Left(n)` without the intermediate cast is
      C2440 (two user conversions). (e) `inline LPCTSTR CStr(const CString&
      s) { return s; }` (no local) folds - see rule 438. (f)
      `inline int LvlIdx(const int& i)` (reference parameter): 73.0%, frame
      0x4a4. (g) Deleting the now-unreferenced `CMerlinLocation::GetPoint`
      from `src/gameobj.h` gains the loader 0.3 points and costs 0.33
      byte-weighted overall - another instance of rule 338, so it is kept
      with a comment saying why.

443. **`CBumperDoc::OnOpenDocument` (0x415450) is behaviourally clean at
      96.22%.** `tools/callcheck.py` reports nothing, `tools/flowcheck.py`
      reports nothing, and `tools/opcheck.py` reports one spurious
      `cmp <imm 0>`. Every remaining `-`/`+` line in the instruction diff
      is a frame displacement, a jump displacement or a `jmp $+5`; the
      rule-337 multiset audit over member offsets, immediates and resolved
      symbols is empty apart from those. The three behavioural faults this
      function ever had (rules 331/332/333: `Left` vs `Right`, `Detach` vs
      `DeleteObject`, the bare member test) were fixed in wave 8 and are
      still right. What is left, in size order: the pad ring-closing block
      (~58 lines - rule 329's fill-order problem, hover.exe evaluates
      `Sub_422BF0`'s second argument first while the address-of-rvalue form
      evaluates the first, and the `CPoint::operator!=` object/argument
      slots are swapped the other way round), the switch jump table
      disassembled as code (16 lines, an artefact), the two swapped deep
      slots of rule 441 (11 lines), and about twenty statement-boundary
      `jmp $+5` whose inline functions have not been identified.

444. **`CMerlinTexture::Serialize` (0x412980) re-measured on top of the new
      texture.cpp: rules 334-336 all still hold.** Splitting
      `g_dwUnk_4C4DA8++` out of the progress callback: 39.3% -> 30.3%.
      Writing the trailing-comment skip as hover.exe's
      `if (n > 0) { i = n; do ReadByte(); while (--i); }`: 38.5%. Deleting
      the `volatile DWORD adwPad[1]` stand-in: 28.0% (the loader gains 0.3
      for it, the file loses 0.06 byte-weighted overall - keep the pad).
      The one structural difference still visible at the top of the
      function is that hover.exe keeps `m_dwUnk2C` in a register across the
      heap-cursor multiply and tests it with `test eax,eax`, where we
      re-read `[ecx+0x2c]`; it re-reads the member at 0x412B91, 0x412C02
      and 0x412E7F, so this is a CSE, not a local. This function is a
      rule-328 lottery: it moved 41.5% -> 42.1% -> 39.3% across three edits
      to *other* functions in the same file. Judge it only by the summary.

445. **The /Od temporary pool is not one source-order run: VC 2.x sorts the
      expansion groups into four tiers, and that is what decides which
      group sits deepest.** Smallest reproduction (a `#pragma
      optimize("g", off)` function, `/O2`, one class `CFx` with an
      out-of-line `MulEq`, several `static inline` helpers taking `CFx` by
      value, each statement a separate expansion): the temporaries of the
      *second* statement can land **above** those of the first, so
      "top-down in source order" (rule 432's description of our side) is
      only true when every group is of the same kind. Measured tiers, top
      of the pool first, each tier internally in source order with
      descending offsets:
      - **S** - multi-dword struct/class temporaries: a by-value struct
        returned from an inline, and the copy made into a by-value struct
        argument. In a six-statement probe these two 8-byte objects took
        the top four dwords regardless of where their statement sat.
      - **H** - groups in which no temporary is live across a call: either
        the expansion contains no call at all (`o = a.m_v;`,
        `DWORD r = a.m_v; return r;`, `o = a.m_v + a.m_v;`) or it contains
        a call but every temporary is used exactly once
        (`a.MulEq(b.m_v);`, `a.MulEq(b.m_v); o = 7;`). Also the two homed
        arguments of an inline that only builds a struct.
      - **N** - groups in which a temporary is read *again* after the
        expansion's call or nested expansion (`a.MulEq(b.m_v); o = a.m_v;`
        - the classic `Hover_FixMulTo`/`Hover_FixSubTo` shape - and also
        `o = a.m_v; a.MulEq(b.m_v);`, which reads the `this` copy before
        the call and lea's it at the call).
      - **L** - groups in which a temporary is *written* after the call
        (`a.MulEq(b.m_v); DWORD t = a.m_v; a.m_v = t;`). These sit below
        every N group even when their statement is first.
      Sizes do not order the tiers (a three-dword N group stays in source
      order between two two-dword N groups), and neither does block
      nesting, statement position or `for`/`goto` layout. This is the
      mechanism behind rule 432's stray pair: `CSub422F50::Sub_4055B0`'s
      two `Hover_FixValue` expansions have no call, so they are tier H and
      take ebp-0x5c/-0x60 at the *top* of the pool, while hover.exe keeps
      them at the bottom of their own group (ebp-0x90 and ebp-0x74).
      Reaching hover.exe's placement would need those two expansions to
      hold a temporary live across a call, which their four instructions
      do not.

446. **Rule 432 is closed as unreachable, and here is the proof rather
      than another list of failed spellings.** Two expansion groups of the
      *identical* shape always come out in source order, in every tier:
      measured for four `MulTo`s, four call-free `Two6`s, three `SubTo`s,
      three value-returning `Mul3`s, and two identical tier-L groups
      interleaved with other kinds - never once reversed, in VC++ 2.0,
      2.1, 2.2 and 4.2. hover.exe:0x4055B0 reverses *two pairs of
      identically shaped groups*: the X and Y rotations are both
      `FixMul, FixMul, FixAdd/FixSub` (7 dwords each, X at ebp-0xb0, Y at
      ebp-0x94) and the X and Y projections are literally the same three
      statements (`FixSubTo, FixSubTo, FixValue`, 7 dwords each, X at
      ebp-0x78, Y at ebp-0x5c). No per-group classification can separate
      two groups that are the same, so no source spelling, tier trick,
      block structure or helper shape can produce hover.exe's order. Its
      C2 laid the whole sub-named region out **bottom-up** - the `this`
      home first at the very bottom, then each statement's group stacked
      upward - which is also exactly what rule 160 measured on the other
      unoptimized function, 0x409350. Our C2 lays it out top-down and
      puts `this` last; the frames coincide at 0xcc only because the slot
      counts match. Exhausted this pass, all on the minimal probe and all
      identical to the `/O2` baseline: base flags `/O1 /O2 /Ox /Oxs /Ob1`
      (the ones that inline at all); `/Od /Ob1` (which produces
      byte-identical code to `/O2` plus `optimize("g", off)`, so the two
      are the same compilation mode); `/Oy- /Og- /Oi /Gy /Gs /Gf /Gm /Gh
      /Gd /Gz /Gr /Gt /GA /GD /GE /GR /GF /Gi /G3 /G4 /G5 /Zi /Z7 /Zd /Zl
      /Zp1 /Za /Ze /J /W4 /FR /Fr /Yd /vd1 /MD`, with and without `/GX`;
      `#pragma optimize` off for `a c e g l p s t w y b gl gs gt gle
      acegl ""` and on over `/Od` for `a g l p s t w y b gl gle`; sibling
      and nested blocks with and without their own locals (a nested
      block's *named* locals do go lower, rule 189, but its temporaries do
      not); `for`-increment and `goto` layouts (emission order is always
      source order); renaming an expanded inline's by-value parameters
      (no effect - an expansion's parameter homes are temporaries, not
      rule-195 hashed locals); and writing the by-value copies as explicit
      named locals *inside* the inline body (they still land in the temp
      pool, still forward). Do not spend another wave on this without a
      different C2 binary.

447. **`CSub422F50::Sub_4055B0`'s `jmp $+5` census, what closes it, and why
      it is not worth closing in bumpview.cpp.** hover.exe has 36 of them
      and we have 7 fewer in six places. Identified and reproduced in
      probes: (a) hover.exe:0x4055CE - `lPX = lX; lPY = lY;` is *one*
      inline holding both assignments (both stores, then one jump);
      `static inline void Hover_Copy2(long& rlA, long& rlB, const long&
      rlX, const long& rlY)` reproduces it instruction for instruction
      with no homed dword, because all four parameters fold (rule 440).
      (b) hover.exe:0x4056FF/0x405704 and 0x4057E9/0x4057EE - the
      rotation's final `FixAdd`/`FixSub` carries *two* consecutive jumps,
      i.e. one more expansion level than the projection's (which has only
      `SubEq` + `FixSubTo`); an outer `static inline void
      Hover_FixAddTo2(DWORD&, const CFixed&, const CFixed&)` that just
      calls the inner one produces the doubled jump. (c) hover.exe:0x4059E8
      - the `pdwOut[0] = m_dwUnk5C + hi(lSX)` store is inside an inline
      too. Measured: the three jumps in (a)+(b) do land exactly, and the
      wrapper even drags `dwI`/`dwK` onto hover.exe's ebp-0x94..-0xa0, but
      `const CFixed&` binding costs two extra dwords per wrapper, the
      wrapped group hoists into tier H (rule 445), and the whole edit is
      90.85% -> 90.72%, byte-weighted 82.82 -> 82.50, 0x4055b0 73.0 ->
      72.4. Keep the recipe, spend it in a file that is not this one.

448. **Price list, measured with an inert control: a *file-local* static
      inline added to `src/bumpview.cpp` costs about 0.05 byte-weighted
      even when nothing calls it.** One unused `static inline` (four
      reference parameters, two statements, no locals) declared just above
      0x4055b0: Accuracy 90.85 -> 90.83, byte-weighted 82.82 -> 82.77,
      0x4055b0 unchanged at 73.0, and fourteen unrelated functions in the
      file re-rolled by more than a point in both directions
      (`OnUpdateGamePause` 97.8 -> 71.1, `OnUpdateCmd34032` 100 -> 80,
      `Sub_4192F0` 98.4 -> 88.9, `Sub_4053A0` 75.5 -> 68.4, `OnDraw`
      67.0 -> 60.4, `Sub_407430` 44.2 -> 40.6 against `OnChar`
      88.3 -> 99.5, `Sub_4282F0` 84.1 -> 95.7, `Sub_40E9A0` 44.3 -> 49.5).
      Three unused inlines: 82.82 -> 82.62, and *using* all three moved
      nothing further (82.62 in all three call combinations), so the whole
      toll is on the declaration, not the call. This extends rule 439 -
      whose 0.51 was for a shared header - to file scope, and refines rule
      436: in bumpview.cpp a new file-scope inline must be worth more than
      ten points on 0x4055b0 (1118 of 227703 bytes) before it pays.

449. **Behavioural audit of `CSub422F50::Sub_4055B0` (0x4055b0, 73.0%):
      clean.** `callcheck`, `opcheck --presence` and `flowcheck` all
      report nothing. Verified by hand against the original: the
      `theApp+0x468` guard; both `FixSetInt`/`FixSubInt` pairs and their
      `lCX`/`lCY` operands; both sine and both cosine table indices
      (`(a<<4)&0x1fff` and `((a<<4)+0x800)&0x1fff`, base
      `g_adwSinTable+4`); the four multiplies' object/argument
      assignments (`dwC*dwB`, `dwE*dwA`, `dwH*dwA`, `dwJ*dwB`), the
      `dwF+dwD` add and the `dwK-dwI` subtract in that order; the two
      `movsx` high-word extractions; the `+= lCX` / `+= lCY` restore; the
      `cdq`/`__allshl 0x20`/`__alldiv` by the *dword* at `this+0x80`
      (signed, so `m_wUnk0080` is read as a 32-bit value with
      `m_wUnk0082`), the repack through a second `__alldiv` by 0x10000;
      the four projection subtractions against `m_awUnk006C[8]` (the dword
      at `this+0x7c`) with `lV1`/`lV2`; and the two output stores against
      `m_dwUnk5C`/`m_dwUnk60`. `ret 0x20` for eight arguments. Everything
      left is the rule-446 pool order plus the operand order of rule 450.

450. **`DWORD + <short lvalue>` at /Od: only a *named* `short` variable
      makes VC evaluate the left operand first.** hover.exe:0x4059D4 and
      0x4059ED emit `mov eax,[this] / mov eax,[eax+0x5c] / movsx ecx,
      word ptr [ebp-0x16] / add eax,ecx`; ours emits `movsx eax,[ebp-0x16]
      / mov ecx,[this] / add eax,[ecx+0x5c]`. Probed in a member function
      with the member at +0x5c: `m + (long)((short*)&s)[1]`,
      `m + (DWORD)((short*)&s)[1]`, `(long)m + (long)(...)`,
      `(long)(...) + m`, `m + *(short*)((char*)&s+2)`, `m + asx[1]` with
      `short asx[2]`, and `m + *(short*)&((short*)&s)[1]` all give *our*
      order - a cast or subscript on the short makes VC compute it first
      and fold the member into the `add`'s memory operand. `short h =
      ((short*)&s)[1]; m + h;` gives hover.exe's order but costs a named
      local, and `short& h = ((short*)&s)[1];` gives hover.exe's order
      with an extra homed pointer. `m + (short)(s >> 16)` also gives
      hover.exe's order but spends `sar`+`movsx` instead of one `movsx`.
      The clean reproduction is an inline taking `const short&`, which
      folds to `[ebp-0x16]` and supplies rule 447(c)'s missing jump at the
      same time - unaffordable here for the reason in rule 448.

451. **Rule 416 is wrong: `CHoverApp::CHoverApp` (0x41c100) is NOT `/Od`
     code, and once you stop treating it as such it goes 73.8% -> 89.6% on
     source shape alone.** All three of rule 416's "tells" are reproduced
     by our ordinary `/O2` build and therefore prove nothing: (1) `this` is
     homed at `ebp-0x14` in *our* build too, because the CString member
     forces an EH frame and the unwind funclet at hover.exe:0x41c7bc reads
     `this` from memory; (2) the full SEH prologue is likewise the CString
     member's, not `/Od`'s; (3) the `LVLPARAM(i)` index really is
     recomputed with a fresh `cmp`/`cdq`/`idiv` at every field access in
     *both* images - VC 2.1 at `/O2` does not CSE it, because the `?:` sits
     inside an array subscript whose base is a global. Rule 416's measured
     collapse to 15.8% under `#pragma optimize("",off)` was not "the frame
     was not rebuilt", it was the pragma being wrong in the first place.
     Do not re-attempt the pragma, and do not spend a rule-195 hash pass on
     this function.

452. **What 0x41c100 actually needed: hover.exe evaluates the destination
     row address of every level-table store *before* the right-hand side,
     and keeps the row base - not the row index - in the register.** At
     each of the nine assignments in the 0-15 loop hover.exe emits
     `lea edi,[ecx*8 + g_lvlParams]` and then stores through
     `[edi+0x44]` / `[ecx+0x40]` / ..., where our `LVLPARAM(i).field =
     expr;` kept only the *index* live and folded `base+field` into the
     store's absolute displacement. Spelling it `pLvl = &LVLPARAM(i);
     pLvl->field = expr;` (one `LVL_PARAMS* pLvl;` local, reassigned before
     every single store - hover.exe recomputes the index each time, so the
     pointer is recomputed each time too) reproduces it exactly: 73.8% ->
     80.9% for the two fixed-point stores alone and 80.9% -> 85.1% for the
     other seven. The two fixed-point stores are the clearest evidence:
     hover.exe emits the row `lea` *between* `_wB = 1000;` and
     `_pA = &dwProd1;`, i.e. **before** the `HOVER_FIXMULWP` `__asm` block,
     and VC cannot schedule across `__asm` (rule 150), so the address
     computation must precede the macro in the source. The same trick pays
     in the 16-19 copy loop (89.4% -> 89.6%): there VC folds the row `lea`
     back into the store's displacement, so no instruction is added, but
     the left-hand side is now evaluated first and hover.exe's
     `lea esi,[edx+edx*4]` before the `mov eax,[g_lvlParams+15*0x58+n]`
     falls out. Generalisation: when a store's diff is
     `mov [idx*8+G+off],r` against `lea r2,[idx*8+G]` / `mov [r2+off],r`,
     the original's left-hand side was a pointer/reference value, not a
     subscript.

453. **hover.exe:0x41c100 keeps the `and eax,0x80000000` that our
     `(dw & 0x80000000) >> 31` folded away: the NT flag is a `?:`.**
     `m_dwUnk03F8 = (dwVersion & 0x80000000) ? 1 : 0;` emits
     `and eax,0x80000000` / `shr eax,0x1f` (VC lowers "one known bit to
     0/1" as a shift but keeps the mask that produced the condition),
     while `(dwVersion & 0x80000000) >> 31` is peepholed to a bare
     `shr eax,0x1f`. Worth 85.1% -> 85.4% here; look for the same tell
     anywhere a sign/bit test is stored as 0/1.

454. **A `BOOL` local for a flag that is both stored to a member and
     tested is worth four points in 0x41c100, and one extra *named* local
     is what put its running-total block on hover.exe's registers.**
     hover.exe computes the Win95 test (`cmp cl,4` / `sbb ecx,ecx` /
     `inc ecx`) *before* the `m_pfnChangeDisplaySettings = NULL` store,
     carries it in a register across that store and across the
     `add edi,0x3dc` base rebase, and then tests the register
     (`test ecx,ecx`) rather than the member it has just written - i.e.
     `BOOL bWin95 = ...; m_pfn... = NULL; m_dwUnk03FC = bWin95; if
     (bWin95)`. Taking it 85.4% -> 89.4%: the GetVersion tail itself is
     only worth about ten instructions; the rest is that the extra local
     flips the `LVLPARAM(i).m_dwUnk38` running-total block onto hover.exe's
     register assignment (its second index in `ebx` through the *jump*
     form, its third in `ecx` through the fall-through form, and
     `lea ecx,[eax+ebx*2]` scheduled after the `imul`), 19 instructions.
     The perturbation is a pure symbol-count effect and it is not
     monotonic: an unreferenced `DWORD dwPad;` in the same place gives the
     same 89.1%, **two** pads give 85.4% (no effect), and **three or four**
     pads give the register flip but re-roll `Hover_sub_41F540` in the same
     file from 87.7% to 61.4% (byte-weighted 82.94% -> 82.23%). Prefer a
     local the source can justify; measure the whole table, and never
     assume "more padding is more of the same effect".

455. **What is left in 0x41c100 at 89.6%, so nobody re-derives it.** 57
     diff lines each side. (a) 16 lines: the frame slot permutation. Both
     images use exactly the same ten slots -
     `_wB1 -0x10, _wB2 -0xe, this -0x14, {three dwords}, divisor temp
     -0x24, _pA2 -0x28, _pA1 -0x2c, dwStep -0x30` - and differ only in
     which of `dwScale`/`dwProd1`/`dwProd2` occupies which of
     `-0x18/-0x1c/-0x20`: hover.exe has `dwProd2, dwProd1, dwScale` from
     the top, ours has `dwScale, dwProd2, dwProd1`. At `/O2` declaration
     order has **no** effect on this (measured: swapping the three
     produces a bit-identical function, as rule 414(d) predicts), and
     moving the two products into the loop's own block has none either.
     (b) 20 lines: in the 16-19 copy loop hover.exe emits
     `lea esi,[edx+edx*4]` one slot before the source load and we emit it
     one slot after - a single-position scheduling swap repeated at all
     nine sites. (c) 14+8 lines: `m_dwUnk54` and `m_dwUnk3C` are pure
     rule-77 register renames (hover.exe `ecx`=row base/`edi`=divisor
     scratch, ours the other way round). (d) 8 lines: hover.exe emits the
     constructor's fifteen member initialisers in *exact source order*
     with only the high half of the 10.0 double rescheduled, while our
     scheduler also pushes `0x15c`, `0x16c` and the 0.3 double down the
     block. (e) 4 lines: `sub esp,0x54` against our `0x48` - hover.exe has
     twelve bytes at `ebp-0x34..-0x3c` that **nothing references**, so it
     declared locals its optimizer never homed; unreferenced locals do not
     reach our frame either (three pads left `sub esp,0x48` untouched), so
     the `SYSTEM_INFO` offset and the `sub esp` cannot be matched without
     finding what those three dwords were. (f) 4 lines of tooling noise:
     hover.exe's `CWinApp::CWinApp` (0x45c92c) and `__CxxFrameHandler`
     (0x43e028) are unnamed in `orig_library.csv`, so `callcheck.py` and
     `flowcheck.py` both report the `<OFFSET2>` pair for this function
     forever. **Behaviourally the function is clean**: every member
     initialiser, the `PROCESSOR_INTEL_486` test, the GetVersion
     decomposition, all nine per-level computations and the 16-19 copy
     match value for value; `opcheck.py --presence`'s single finding
     (`disp+0x20` against our `disp+0x3fc`) is the `m_dwUnk03FC` store's
     addressing base, not a different member.

456. **`Hover_sub_41F540` (0x41f540) is structurally finished at 87.7% -
     the census proves it - and three more spellings of the tail loops
     were measured and lost.** The loader-method census (rule 437) is
     *empty* here: neither image contains a single `jmp`, so no inline in
     this function materialises a result. The `CPoint` temporary census
     matches exactly - 57 stores to `[esp+0x14]`, 49 to `[esp+0x18]`, 49
     loads from `[esp+0x18]`, 97 indirect calls on both sides, 72 loads
     from `[esp+0x14]` against our 73 - and the whole function is 1284
     instructions against hover.exe's 1281. `callcheck.py` and
     `flowcheck.py` are silent; `opcheck.py --presence`'s findings
     (`disp+0xa0` vs `disp+0x90`, `disp-0x188/-0x190` vs
     `disp+0x1a8/-0x1a0`, `disp-0x50/-0x58`) are all the known
     rolling-pointer and pointer-advance-timing shapes of rules 307/308,
     plus the loop-bound `cmp` immediates that reccmp names on our side
     only. New negatives, all measured on top of the 87.7% state:
     (a) rule 343's "hover.exe evaluates all five SetRect arguments into
     registers and pushes them in one run" is the signature of an *inline*
     wrapper (arguments evaluated left to right at the call site, pushed
     by the body), and it is only true of loop 2 - loops 1 and 3 push
     right-to-left as they compute, so they are direct `::SetRect` calls.
     Spelling loop 2 as `((CRect*)pRc)->SetRect(...)` (MFC's inline
     `CRect::SetRect`, no new type in any shared header) gives **63.6%**
     and a file-local `inline void Hover_SetRc(RECT*,int,int,int,int)`
     wrapper gives **61.0%**: in both cases the *whole* function's
     `esi`/`edi` allocation rotates, which costs far more than the loop
     gains. (b) Loop 3 genuinely does not use the `[esp+0x10]` spill
     (hover.exe keeps `i+1` in `ebx` there and never stores it), but
     spelling the increment out as `(i + 1)` instead of reading the
     `volatile int j` stand-in back costs 87.7% -> 87.1%. The residual is
     rule 343's phase-shifted register rotation in zones 4 and 0 (equal
     77- and 72-instruction hunks, identical instruction *sequences*, only
     the `CPoint` temporary's `x`/`y` registers swapped) plus the loop
     scheduling; no source-level lever exists for either at this file
     state.

457. **The mechanism behind rule 432: VC 2.x lays out `/Od` expression
     temporaries in *buckets keyed on how many temporaries the statement
     needs*, smallest bucket nearest `ebp`, each bucket filled in source
     order going deeper.** Probed with `tools/probe` under
     `#pragma optimize("", off)` (the disassembly is in the probe output,
     the files were deleted before committing). With
     `inline DWORD G1(DWORD a) { DWORD r; r = a+1; return r; }` (one
     temporary), `G2` (two) and `G3` (three), the statement sequence
     `G1 / G3 / G2 / G1 / G2 / G3` after one named local at `ebp-4` gives
     ```
     G1#1 ebp-0x08   G1#2 ebp-0x0c                     <- 1-slot bucket
     G2#1 ebp-0x10..0x14   G2#2 ebp-0x18..0x1c         <- 2-slot bucket
     G3#1 ebp-0x20..0x28   G3#2 ebp-0x2c..0x34         <- 3-slot bucket
     ```
     i.e. **not** source order overall, but source order *within* each
     bucket. Two corollaries, both measured:
     - the block is the *statement*, not the inline: an inline that calls
       another inline (`N1(a) { DWORD r; r = K1(a)+2; return r; }`)
       contributes **one** two-slot block, not two one-slot blocks, and it
       lands in the two-slot bucket;
     - inside a block the statement's *outermost* result is the shallowest
       slot and the rest follow in creation order going deeper (`G3`'s
       `t,u,r` land at `-0x24,-0x28,-0x20`).
     Nested `{}` blocks and `if` bodies change nothing (probed).
     This re-frames rule 432. The question is not "how do I reverse the
     order"; it is **"does each statement create exactly as many
     temporaries as hover.exe's does"** - a statement with one temporary
     too many or too few moves into a different bucket and takes every
     later block of both buckets with it. Two buckets of
     `CRobotPlayer::Sub_409350` are already exactly right and visible in
     the frame: the six two-temporary statements at ebp-0x154..-0x180 and
     the eight three-temporary comparison statements at ebp-0x1ac..-0x208
     are both in perfect source order.
     **Caveat, measured and unresolved.** The bucketing is exact in the
     probes and it accounts for those two runs of ours, but it does not
     yet account for hover.exe:0x409350 as a whole: clustering the
     original's 97 temporaries by "first used together and contiguous in
     the frame" gives one-slot blocks at ebp-0xa4, -0xb0, -0xcc, -0x13c,
     -0x1a4, -0x1b4, -0x1c0, -0x1f4 and -0x1f8, which are *not* one
     bucket, while its three three-slot blocks (-0x118..-0x120,
     -0x124..-0x12c, -0x130..-0x138) are contiguous and in source order,
     exactly as the rule predicts. Either the clustering is wrong for the
     big nested statements (statement 1's fourteen temporaries land in two
     runs on both sides) or a second rule splits large blocks. The next
     person should start by writing the block partition of both sides
     mechanically and diffing the *size histogram*, not the slot numbers.

458. **Inline-expansion temporaries are anonymous: rule 195's identifier
     hash and rule 430's renaming lever do not reach them.** Renaming
     `HoverFixMulR`'s `dwA`/`dwB`/`dwR` to `dwZZ`/`dwYY`/`dwXX` in
     `src/texload.cpp` leaves `CRobotPlayer::Sub_409350` byte-identical
     (checked instruction by instruction, same `ebp-0x184/-0x188/-0x18c`).
     So the frame of an `/Od` function has two independent halves: the
     *named* locals, ordered by rule 195's hash and movable by renaming
     (rule 430), and the *expansion temporaries*, ordered by rule 457's
     size buckets and movable only by changing how many temporaries a
     statement makes.

459. **No source spelling separates an inline's result local from its
     parameter homes; the block is always contiguous with the result at
     its shallow end.** Probed on `Mul(dwA,dwB)` five ways under
     `#pragma optimize("", off)`: a plain `DWORD dwR` local (control),
     the result inside a one-member `struct`, a by-value class return
     (`CFixed` with the value in a member - rule 428's spelling; it adds
     one *more* slot, the returned object, and still keeps the group
     contiguous), the result declared in a nested block inside the inline,
     and a two-level form whose outer inline takes `const DWORD&`
     parameters (VC 2.1 materialises a copy for every reference parameter
     of an expanded inline, so this *adds* two homes rather than folding
     them). All five keep `{result, second home, first home}` adjacent.
     Anyone trying to reproduce hover.exe:0x409350's `ebp-0x1f4` result
     slot sitting 0x24 bytes away from its `ebp-0x218/-0x21c` parameter
     homes should therefore look for a *different statement decomposition*
     (rule 457), not a different spelling of the same one.

460. **VC 2.1 expands the arguments of an inline call right to left, and
     that is why hover.exe:0x409415 looked swapped.** In
     `Sum(Mul(x,x), Mul(y,y))` the *second* argument's expansion is
     emitted first, and the enclosing inline then homes its second
     parameter first. hover.exe's speed sum at 0x409415 homes the
     +0x1b0 square into the addend slot and the +0x1ac square (the one
     emitted first) into the accumulator, which the plain
     `Sum(a,b) { return Add(a,b); }` forwarder cannot produce. Spelling
     the forwarder `return HoverFixAddR(dwB, dwA);` gives exactly
     hover.exe's data flow (same sum). No score change on its own - both
     instructions still differ in their frame displacement - but it is
     one less thing in the way once rule 457 is solved.

461. **Negatives on hover.exe:0x409350 from wave 16 (do not repeat).**
     Baseline 72.20%, byte-weighted 82.82%.
     - The four clamp pairs (`m_wUnk01F2 = N; m_wUnk01F0 = 0;` and the
       three like it) really are an inline in hover.exe: there is an
       expansion end after every one of them at 0x40a0d9, 0x40a17c,
       0x40a33a and 0x40a3dd, and *two* after the giving-up arm's pair at
       0x40a476/0x40a47b. Writing them as
       `HoverFixSetWhole(&m_wUnk01F0, N)` (whole part first, fraction
       after, pointer parameter folded) buys the three expansion ends we
       lack - and *costs* the function: 70.04% with only the giving-up
       arm converted, 71.44% with the two "1" clamps as well, 71.63% with
       all four plus dropping `HoverFixOfInt(1)` (which is folded into
       `cmp [x],0x10000` and contributes only its expansion end, so the
       jump count at those two sites is a coincidence). Every variant
       adds a statement to a bucket and moves rule 457's whole layout.
     - `HoverFixSqrt`'s argument home at ebp-0x224 is the function's only
       *extra* pair of instructions (hover.exe folds the square root's
       parameter onto the sum's result slot at ebp-0x200); rule 317's
       `const DWORD&` attempt is confirmed by rule 459's probe - VC 2.1
       copies every reference parameter of an expansion.

462. **`CRobotPlayer::Sub_409350` is behaviourally clean at 72.20%, and
     the residue is now itemised.** `tools/callcheck.py` reports only the
     two CRT helpers hover.exe leaves unnamed (`__CIsqrt`,
     `__adj_fdiv_m64`); `tools/opcheck.py --presence` and
     `tools/flowcheck.py` report nothing. Two multiset audits over the
     whole 860-instruction body are *empty*: every `[reg + 0xNN]` member
     offset matches one for one, and so does every immediate keyed by
     mnemonic. The conditional-jump census matches except for rule 316's
     three `?:` leaves (`jl` where hover.exe has `jge`/`jle`). Of the 238
     mismatched original instructions, **165 are the same instruction with
     a different frame displacement** (rule 457), and the rest are: the
     3 x 4 instructions of rule 316's leaves, rule 319's four instructions
     at 0x4097bd (the by-value point parameter of `Hover_sub_40B6E0`,
     still not made), the 2 extra `mov`s of rule 461's square-root
     argument home, one instruction of rule 463, six expansion ends
     hover.exe has and we do not (0x4094ce, 0x40a17c, 0x40a19e, 0x40a250,
     0x40a3dd, 0x40a476) against seven we have and it does not, and the
     jump displacements those shift. There is no behavioural divergence
     left in this function.

463. **Rule 160's locals-in-a-structure trick costs one comparison
     spelling: VC canonicalises `a > b` back into `b < a` when both
     operands are members of the same structure.** With plain `int`
     locals `if (nA > nB)` gives `mov eax,[nB] / cmp [nA],eax / jle` and
     `if (nB < nA)` gives `mov eax,[nA] / cmp [nB],eax / jge` - two
     different codegens. With the same two values as members of one local
     structure, `s.nA > s.nB`, `s.nB < s.nA`, `!(s.nA <= s.nB)` and
     `!(s.nB >= s.nA)` all give the *second* form. hover.exe:0x4098f1 has
     the first (`mov eax,[nDistSide] / cmp [nRange],eax / jle`, i.e.
     `nRange > nDistSide`), so those three instructions cannot be
     reproduced while the steering pass keeps its rule-79/125 structure.
     The source says `f.nRange > f.nDistSide` anyway, with a comment.

464. **Under `/O2` the DECLARATION ORDER of function-scope locals decides
      the order in which the register allocator considers them, and so
      which register each one gets.** This refines rule 284, which found
      that declaration order does not move /O2 *stack slots* and concluded
      the lever was dead; it moves *registers*, and that is worth far more.
      In `CSub402E00::Sub_4030F0` (0x4030f0) hover.exe loads the merge
      key into `eax` and the merge target into `edx` at all eight
      span-merge expansions; our build had them the other way round
      because `pTarget` was declared second and `pKey` twentieth. Moving
      the single line `RENDEROBJ* pTarget;` to sit *after* `void* pKey;`
      swapped the pair and took the function 59.42% -> 70.49% with no
      collateral anywhere in the table. Moving `pKey` up in front of
      `pTarget` instead scores 58.5%: it fixes the same pair but shifts
      every other candidate by one and pushes `pObj` out of `ecx` into
      `esi`, so keep the list *length* the same and move one declaration
      at a time. The lever is real but not monotone - "earlier
      declaration = earlier register" holds only when nothing else in the
      live range changes rank, so measure each permutation.

465. **`Sub_4030F0` (0x4030f0) 59.4% -> 84.3%, the wave-16 rebuild.** Four
      independent findings, each measured on its own (see rules 445,
      447-450). The residual 81 mismatched instructions are: the prologue
      instruction order (7), the sprite branch's `nW56`/`nW54` register
      pair and its scheduling (15), four `test eax, eax` that our
      scheduler hoists two slots in the edge appends, two `lea` that our
      build hoists out of the merge stores, and the jump displacements
      that follow from all of the above. The frame is now byte-exact:
      `sub esp, 0x18`, `this` at `-8`, the shared scan/first-product temp
      at `-4`, `dwDelta` at `-0xc`, the second product temp at `-0x10`,
      `nW56` at `-0x14`, `nW54` at `-0x18`.

466. **The edge-list pass of `Sub_4030F0` reads `pScan->m_pUnk04` into a
      local *before* the `-1` store, and that is what puts `pTarget` in
      `ebp`.** hover.exe emits `mov eax,[ecx+4]` and only then
      `mov [ebp+0x40],edi`; writing
      `pTarget->m_dwUnk48 = (DWORD)pScan->m_pUnk04;` inline made our build
      allocate `pTarget` to `eax` and the member load to `ebp` - the whole
      35-instruction loop mismatched on register names alone. Hoisting the
      load into `pKey` first (`pKey = pScan->m_pUnk04;
      pTarget->m_pUnk40 = (RENDEROBJ*)-1; pTarget->m_dwUnk48 = (DWORD)pKey;`)
      took 74.0% -> 78.5%. Declaring `HEAPNODE* pScan;` first (rule 464)
      had already put `pScan` in `ecx` there: 70.5% -> 74.0%. hover.exe
      duplicates the `mov eax,[ecx+4]` into *both* arms of the
      `m_dwUnk10` test; our build hoists the one copy above the test, which
      is the only instruction left wrong in that loop.

467. **A memory-resident `int` local subtracted from a `DWORD` global makes
      VC 2.1 build the difference in a stack temporary; make the local the
      same fundamental type and it emits the two-instruction form.**
      `Sub_4030F0`'s static branch does
      `pObj->m_dwUnk6C = g_dwUnk_4A4544 - nW56;` twice. With
      `int nW56, nW54` (however the expression is cast:
      `(DWORD)((int)g - nW56)`, `g - (DWORD)nW56` and `g - nW56` are all
      identical) our build emitted
      `mov edx,[g] / mov [t],edx / mov edx,[nW56] / sub [t],edx /
      mov edx,[t]`, five instructions per site against hover.exe's
      `mov edx,[g] / sub edx,[nW56]`. Retyping the two locals to `DWORD`
      (with `(DWORD)(short)` on the `movsx` assignments, which keeps the
      sign extension) collapses both sites to hover.exe's form: 78.5% ->
      79.8%, and it also lifted `Sub_403980` 67.5% -> 77.9%,
      `Sub_406740` 36.8% -> 40.4% and `Hover_sub_406A80` 33.1% -> 36.7%
      through rule-14 churn. The cast experiment is not a substitute for
      the retype (rule 403 again): only the declaration matters.

468. **Compound-assignment chains and full expressions produce DIFFERENT
      NUMBERS OF TEMPORARY SLOTS, and that is the lever that fixes an /O2
      frame that is one dword too big.** `Sub_4030F0`'s front-face test was
      written as six statements
      (`dwT1 = a; dwT1 -= b; dwT1 *= c; dwT2 = d; dwT2 -= e; dwT2 *= f;`)
      and compiled to `sub esp, 0x1c` with a dead dword at `-0xc`: the two
      named locals each got a home *and* the products got temps. Writing
      the same arithmetic as two full expressions
      (`dwT1 = (a - b) * c; dwT2 = (d - e) * f;`) gives `sub esp, 0x18` -
      hover.exe's exact frame - because both products now land in the
      *same* temporaries the named locals own, and the first one coalesces
      with the scan-value local and the divisor temp in slot `-4` exactly
      as hover.exe does. 79.8% -> 81.9%, no collateral. Removing the two
      named locals altogether (one big expression) is worse (77.2%): it
      makes three temps instead of two. The listing (`cl /FAcs`, see the
      note under rule 477) prints the frame map and is the fastest way to
      see this: named locals appear as `_name$ = -n`, temps as `-n+[esp+k]`.

469. **A `pNew = pNode; ... pNode++;` alias makes VC hoist the increment
      into the middle of the block; writing through `pNode` itself keeps
      it at the end.** All four edge appends in `Sub_4030F0` were spelled
      with a `pNew` alias, and our build emitted `add esi, 0x18` before the
      last two stores and addressed them as `[esi-8]`/`[esi-4]`;
      hover.exe stores `[esi+0x10]`/`[esi+0x14]` and then advances.
      Deleting `pNew` and writing every field through `pNode` fixes two of
      the four blocks completely: 81.9% -> 84.3%. Its rule-14 churn is
      almost exactly self-cancelling (`Sub_406740` +3.25,
      `Hover_sub_406A80` +2.07, `Sub_403A60` -5.32, `Sub_4065F0` -1.32,
      net +0.002 byte-weighted), so it was kept for the structure, not the
      score.

470. **Rule 283 is now a NEGATIVE: with the frame of rule 468 in place,
      coalescing `dwVal` into `dwT1` costs `Sub_4030F0` 84.3% -> 81.0%.**
      The slot sharing rule 283 was chasing (hover.exe's `dwT1` and
      `dwVal` in one slot at `-4`) is what the full-expression form
      produces by itself; forcing it by hand now removes a slot the
      function needs and re-rolls the file (`Sub_403A60` 27.0% -> 16.2%,
      `Sub_404030` 40.9% -> 37.5%, `Sub_4066B0` 31.4% -> 28.6%, total
      -0.115 byte-weighted). Do not re-attempt it.

471. **`CSub403980::Sub_403A60` (0x403a60) is a THREE-STATE LOTTERY on the
      operand order of its span compares, and any edit to `Sub_4030F0`
      re-rolls it.** Its ~30 clip tests come out either as
      `cmp ebp,[esp+0x20] / jle` (38.3%) or as
      `cmp [esp+0x20],ebp / jge` (27.0%, and 16.2% / 32.3% for two other
      mixtures); the original itself uses both forms, so the score is
      decided by which way VC canonicalises this .obj. A source change to
      `Sub_4030F0` that leaves its object code byte-identical (rule 284's
      declaration swap) leaves 0x403a60 alone, so the flip is real codegen
      churn and not line-number drift. Budget it: one flip is 11.3 points
      of a 1477-byte function, i.e. 0.073 byte-weighted, and wave 16 paid
      it once. Rule 280's "operand order is not source-controllable" still
      holds *within* a function; this rule is about the whole .obj.

472. **`CSub402E00::Sub_406740` (0x406740) 36.8% -> 46.7%, and its
      behavioural audit is CLEAN.** Three source fixes:
      (a) the mip shift count is `1 << g_dwUnk_461CFC` on the whole
      DWORD, not `1 << (BYTE)g_dwUnk_461CFC`, and the separate
      `cl = (BYTE)g_dwUnk_461CFC;` line was dead;
      (b) the vertical scale is read into `dwS` first
      (`dwS = g_dwUnk_4A1D70; dwV = (dwV << 0x10) / dwS; dwS -= dwB;`),
      matching hover.exe's `mov esi,[0x4a1d70] / div esi /
      sub esi,[esp+0x20]`;
      (c) the texel index is ONE expression
      `dwV = (dwV & pEntry->m_dwUnk00) * pEntry->m_dwUnk0C +
      pEntry->m_dwUnk18;` - as three compound assignments it went through
      a stack temp (rule 468 again) - and `dwCount = dwW - dwA;` belongs
      *after* `g_dwUnk_4A4A1C = dwV;`. Audited instruction by instruction
      against hover.exe: every branch polarity (all the clip tests are
      signed), the two `pfnUnk04` dispatches (`pView->` in the middle,
      `g_pUnk_4A1D68->` in the tail - hover.exe really does re-read the
      global for the second), the `(dwV4 - dwW)` / `(dwW - dwV)`
      asymmetry between the mirror and non-mirror tails, the `jns` that
      reuses the flags of `add eax,ecx`, and the flat fill. NOTE on rule
      371: hover.exe's own flat fill is CORRECT
      (`mov ch,bl / mov cl,bl / mov eax,ecx / shl eax,0x10 / mov ax,cx`
      discards the stale upper half of `ecx` in the shift); the miscompile
      rule 371 records was in OUR hand-written loop, and `memset` is both
      correct and the same expansion hover.exe uses. The residual 128
      mismatches are one consistent register permutation: hover.exe keeps
      `pView` in `edi`, the row argument `dwB` in its *parameter slot*
      `[esp+0x20]` (5 reads, never enregistered) and the pitch in `ebp`,
      where we enregister `dwB` and home the pitch. No declaration order
      reproduces that; it needs the parameter to stay in memory.

473. **`CSub402E00::Sub_402630` (0x402630) is BEHAVIOURALLY EXACT and
      register-bound at 34.7%; the shape of hover.exe's four clip blocks
      says they are an INLINE FUNCTION and that is the only lever left.**
      Audited all four blocks plus the tail: every comparison direction,
      every `Sub_403980` argument order (`m_dwUnk60/m_dwUnk48` for the
      `m_pUnk84` edge, `m_dwUnk64/m_dwUnk54` for `m_pUnk88`), the
      `+g_dwUnk_4A453C` / `-g_dwUnk_4A453C` asymmetry between the two
      edges, `Sub_4065F0(pObj, pSpan, dwB4, dwB8, m_dwUnk64)` and
      `Sub_402990(pObj, pSpan, g_dwUnk_4A4524, g_dwUnk_4A4564)`. Frame:
      hover.exe uses `sub esp, 8` with `dwB4` at `-8` and `pSpan` at `-4`
      and keeps the covered flag in `eax`; we use `sub esp, 0xc` and home
      the flag and the edge value instead. THE LEAD: each block ends
      `mov eax,1 / jmp L` or falls into `xor eax,eax / test eax,eax /
      jne <ret>`, i.e. a function returning 0/1 whose result is tested at
      the call site - rule 437's signature. Blocks 1 and 4 are the same
      helper (low edge, `(dwA,dwB)` vs the two globals) and blocks 2 and 3
      are the same helper (high edge), so hover.exe has two file-local
      inlines called twice each. Rewriting them that way needs a
      file-scope addition, which wave 16 forbade, so it is untried.
      Measured negatives: hoisting `pObj->m_dwUnk60`/`m_dwUnk64` into a
      new local as hover.exe does (`mov edx,[esi+0x60]` once, pushed at
      four call sites) is right but the extra local re-rolls the whole
      .obj for -0.14 byte-weighted (0x402630 itself falls to 26.0%);
      dropping the `dwB4` local and reading `m_acUnkAC.m_dwUnk08` at every
      use gains the function 2.9 points but costs -0.11 overall; five
      declaration permutations move it by at most -4.4 and never up.

474. **`CSub402E00::Sub_402E00` (0x402e00) 64.3% -> 69.7%, audit CLEAN.**
      Two fixes: (a) the point count is read into a local once
      (`dwMax = (DWORD)(short)pPts->m_wUnk00;` then `m_dwUnk90 = dwMax`,
      `g_pUnk_4A1D60 += dwMax * 0x14`, `if (dwMax == 0)`,
      `if ((int)dwMax <= 0)`, `i = dwMax`) - reading `pObj->m_dwUnk90`
      back four times cost two reloads and turned hover.exe's
      `lea eax,[ecx*4]` into `shl ebx,2` (+4.65); (b) the empty-point
      clamp writes through a `pSpan` pointer, and the `m_dwUnk64` clamp
      must NOT cache into `dwB` (hover.exe reads `[edi+0x64]` twice)
      (+0.79). Audited: the 16-bit `sar ax,cl` in `nPts` is arithmetically
      identical to our 32-bit shift of a sign-extended short; the
      `(BYTE)(m_dwUnk9C + 4)` load really is a dword load in hover.exe
      while `(BYTE)(m_dwUnk9C + dwLevel)` is a byte load (spelling the
      second as `(BYTE)((BYTE)a + (BYTE)b)` gets the byte load but costs
      -0.18 byte-weighted elsewhere - rejected); the "Trashing Heap"
      compare is unsigned in both; the two `Sub_404030`/`Sub_403A60`
      argument lists match. Residual: `nCount` and `dwX1` are swapped
      between `esi` and `ebx` (12 sites) and the point loop reloads
      `m_dwUnk60` in hover.exe where we CSE it; two declaration
      permutations that swap `nCount`/`dwX1` cost 16 points each.

475. **`CSub402E00::Sub_404030` (0x404030) stays at 40.8%: rule 281's
      tail-duplication ceiling is the whole gap and none of wave 16's new
      levers touch it.** hover.exe is 417 instructions to our 293 and
      reccmp compares 417 against 293, so 70% is the arithmetic maximum
      without the eleven duplicated advances. Three declaration
      permutations of `{pSpan, dwX0, dwX1, i, dwStep}` were measured
      (40.3%, 40.3%, 38.0%): hover.exe keeps the one-pixel step in `edx`
      and re-reads `dwXMax` from its parameter slot 27 times, where we
      home the step and enregister `dwXMax`, and no order flips that.
      Rules 278-281, 285 all still hold.

476. **Addendum 7's `jmp $+5` census is EMPTY for the whole texheap
      renderer cluster.** `0x4030f0`, `0x402630`, `0x406740`, `0x402e00`
      and `0x404030` contain no `eb 00` and no `e9 00000000` at all:
      these are ordinary `/O2` functions with no inline-function returns
      materialised into frame slots, so the wave-15 loader method does not
      apply to them. What *does* apply, and is the same phenomenon one
      level down, is rule 468: the number of expression temporaries the
      frame reserves is decided by whether the arithmetic is written as
      compound assignments or as whole expressions.

477. **Frame maps come out of the compiler for free.** Re-running the
      build's own `cl` line with `/FAcs /Falst\ /Folst\ /Fdlst\` on one
      `.cpp` writes `lst/<file>.cod`, whose per-function header lists
      every homed local by name and offset (`_dwDelta$ = -12`) and whose
      body spells the temporaries as `-16+[esp+40]`. Diffing that against
      `sub esp, N` tells you immediately whether a frame is too big and
      which slot is dead - three minutes per question instead of
      reconstructing displacements from `compare.py --verbose`. Delete the
      `lst/` directory before committing.

478. **`CWall::Sub_408820`'s frame, closed: the two missing dwords are a
      *per-branch* `fxT`, `pFx` and `sDelta`, and nothing else.**
      hover.exe's frame is 0x1B8, ours was 0x1B0, and rule 351's diagnosis
      ("two slots too deep at the top") was wrong: the named-local pool is
      26 dwords in hover.exe and was 24 in ours, and the two missing ones
      are the *second copies* of the grid-crossing temporaries. hover.exe
      keeps a separate `fxT` (ebp-0x28 in the row branch, -0x2c in the
      column branch), a separate `pFx` (-0x38/-0x3c), a separate
      `fxSlope` (-0x40/-0x44) and a separate `short sDelta` (-2/-4, the
      two shorts sharing one dword at the very top of the frame), always
      with the *else* branch's copy 4 bytes shallower - and our
      function-scope singletons collapsed each pair into one slot.
      Declaring `FIXEDPT* pFx; FIXEDPT fxT; short sDelta;` inside each of
      the two walk branches is +2 dwords (the two shorts still pack into
      one dword) and takes the frame to exactly 0x1B8: **35.5% -> 50.2%**,
      byte-weighted 82.82% -> 82.91%. The pay-off is not the frame number
      but the deep pool below it: all twenty accessor temporaries
      (ebp-0xB0, -0xB8, -0xC0, -0xC8 | -0xD8, -0xE8, -0xF8, -0x108 |
      -0x130, -0x138, -0x140, -0x148 | -0x158, -0x168, -0x178, -0x188 |
      -0x198, -0x19C, -0x1B0, -0x1B8) were already in the right *shape*
      and exactly 8 bytes too shallow; correcting the frame size drops
      them onto hover.exe's displacements one for one.

479. **The arithmetic of rule 478, so the next frame can be budgeted
      instead of guessed.** Measured on 0x40EF30, each block-scoped
      duplicate costs one dword only if hover.exe also spends one:
      `fxT` +1, `pFx` +1, `fxSlope` +1, `sDelta` +0 (short pool),
      `nXBound` +1, `nYBound` +1, `nColA` +1, `nRowA` +1, `nCross` +0
      (register). hover.exe spends it on `fxT`, `pFx`, `fxSlope`,
      `sDelta` and `nYBound` and keeps the row branch's `nColA`, `nRowA`
      and `nXBound` in ecx/edi/ebx, which our register allocator will not
      do - so scoping those four is pure frame growth. Full ladder from
      35.5%, byte-weighted in brackets: `fxSlope` alone 42.9% [82.86],
      `pFx` 40.9% [82.84], `fxT` 40.9% [82.84], `sDelta` 36.3% [82.81],
      `pFx+fxT` 49.8% [82.92], `fxSlope+pFx` 49.4%, `fxSlope+fxT` 49.4%,
      `pFx+fxT+sDelta` **50.2%** [82.91], `pFx+fxT+sDelta+nCross` 49.4%
      [82.93], `fxSlope+pFx+fxT` 32.4% (frame 4 too deep), and
      `pFx+fxT+nXBound` 32.8% / `+nYBound` 32.0% (each +1 dword past
      0x1B8). One dword past the target is worth about -17 points,
      because the entire deep pool moves with it.

480. **The complete frame map of hover.exe:0x40EF30, for whoever finishes
      it.** Shallow to deep: `-2` sDelta(row branch), `-4` sDelta(col
      branch), `-8` nCell, `-0xc` this, `-0x10` nDY, `-0x14` nRowB,
      `-0x18` nStartX, `-0x1c` nStartY, `-0x20` nColB, `-0x24` nYBound(col),
      `-0x28` fxT(row), `-0x2c` fxT(col), `-0x30` nEndY/nYBound(row, one
      slot, disjoint live ranges), `-0x34` nXBound(col), `-0x38` pFx(row),
      `-0x3c` pFx(col), `-0x40` fxSlope(row), `-0x44` fxSlope(col),
      `-0x48` nColA(col), `-0x4c` nColStep(row), `-0x50` nRowStep(col),
      `-0x54` nRowA(col), `-0x58` nColDir(row), `-0x5c` nRowDir(col),
      `-0x60` the `(short)nDY` divisor temp of the row branch, `-0x64`
      nRow, `-0x68` nCol; then the four coalesced 8-byte accessor
      structs `-0x70/-0x6c` SegPt0, `-0x78/-0x74` SegPt1, `-0x80/-0x7c`
      Cells(row branch), `-0x88/-0x84` Cells(col branch); then the deep
      pool of rule 478. nDX lives in esi for the whole function and is
      never homed; nEndX is edi in the column branch and ebx in the row
      branch. Note that the *struct* temporaries coalesce (one slot for
      all six SegPt0 expansions, one for all eight `Cells` expansions per
      branch) even though rule 321's `__asm` switches off coalescing of
      the *extracted* values - so the asm rule applies to the scalar
      sub-temporaries only.

481. **What still separates us from 0x40EF30, measured: one slot in the
      wrong half of the frame.** With rule 478 applied our named pool is
      the right 26 dwords but split 18/8 around `nColA` where hover.exe
      splits 17/9, so everything from `nColA` down (`nColA`, the two
      steps, `nRowA`, the two directions, the divisor - nine slots that
      carry most of the two loop bodies) sits exactly 4 bytes too deep.
      The extra slot in the top half is rule 324's hoisted
      `&((CCollider*)dwA)->m_dwUnk44` CSE temp, and the missing slot in
      the bottom half is hover.exe's `nRow`/`nCol` pair at -0x64/-0x68,
      which we enregister. Removing the CSE temp alone moves the frame to
      0x1B4 and takes the whole deep pool off its slots (measured with a
      `CCollider* pMgr` local: 51.0% -> 42.7%); removing it *and* adding
      one dword back with a per-branch `fxSlope` restores 0x1B8 but leaves
      the nine slots where they were (49.7%). The clean fix is still rule
      350's `CCollider*` retype of the vtable slot, which deletes the CSE
      temp without spending a dword.

482. **Negatives on 0x40EF30 from this pass.** (a) Spelling the collider
      as an lvalue - `(*(CCollider**)&dwA)->m_dwUnk44` (aliasing the
      parameter slot itself) or `(*(CCollider*)dwA).m_dwUnk44` - does not
      remove the CSE temp; VC folds both back to the converted rvalue.
      (b) Giving the two visits in a loop body different member offsets
      to break the CSE produces *two* temps (frame 0x1C0), which confirms
      the temp is keyed on `&obj->member`, not on the cast. (c) Block-
      scoping `nCol`/`nRow` into the degenerate case, both branches and a
      braced tail (rule 351 tried this before the frame was right): 49.0%
      for both, 50.2% for `nRow` alone - it never moves them to
      -0x64/-0x68. (d) Declaration order: of sixteen single hoists to the
      head of the list only `nColB` pays (+0.8, kept), `nRowB`/`nXBound`/
      `nYBound` give +0.4 and the rest are byte-identical or worse;
      reversing the whole list costs 2.4. A 250-candidate greedy
      permutation search from `nColB`-first found no second improvement,
      so rule 325's "spill order is not addressable from the source"
      stands for the slot *order* even though the register choice is
      addressable (rule 483).

483. **`CFloorObj::Sub_408820` (0x41A960): the four bounding-box registers
      are chosen by declaration order, and it is a rule-328 lottery.**
      hover.exe puts lMaxX in edi and lMaxY in the memory slot [esp+0x14],
      lMinX in eax and lMinY in ebp, seeding the zero pair first
      (`xor edi,edi / mov [esp+0x14],edi`) and the 0x5A00 pair second
      (`mov eax,0x5a00 / mov ebp,eax`); ours seeded the 0x5A00 pair first
      into (edi, ebx). Of the 24 orderings of `lMinX/lMinY/lMaxX/lMaxY`,
      every one with **lMaxX in the first two declarations** gives 43.9%,
      lMaxY first gives 39.0% and the two mins first give 38.2% - a 5.7
      point spread from nothing but declaration position, with the
      initialisation order and the comparison orientation (rule 326)
      making no difference at all (`if (a > b)` and `if (b < a)` compile
      identically at all eight sites). The catch: the gain evaporated the
      moment `CWall::Sub_408820` changed in the same translation unit -
      after that all 24 orderings give 38.2%. Re-roll it last, and do not
      quote it as evidence.

484. **Behavioural audit of the two slot-12 overrides, both clean.**
      0x41A960: the list walk reads `[this+0x98]` inline (no
      `GetHeadPosition` call), the eight bounds updates take
      `SegPt0(&pObj->m_obj04)` for the first four and `SegPt1` for the
      last four in the order minX/minY/maxX/maxY, the seeds are 0x5A00 and
      0, all four divides are `idiv` by `m_dwUnk0C` in the order
      nX1/nX2/nY1/nY2, both loops are inclusive (`jle`) with the guard
      hoisted above the body, and the argument order is
      `Sub_40F8A0(x,y)` / `m_obj10.Sub_4086A0(x,y,this)`. `callcheck`,
      `opcheck --presence` and `flowcheck` are all silent on it.
      0x40EF30: rule 327's audit re-confirmed; the only mechanical-check
      hit left is `imm:add:0x44` (rule 481's CSE temp) and a missing
      `disp+0x10`, which is hover.exe reading `[this+0xc]`/`[this+0x10]`
      *before* computing `this+0xc` for the second accessor while we
      compute the address first and read `[ecx]`/`[ecx+4]` - a scheduling
      difference inside one inline expansion, not a wrong operand.

485. **Toolchain hazard: an interrupted `build.sh` poisons the next
      builds.** A build killed part-way leaves `build.pdb` in the
      *worktree root* (not `build/`) and a live `wineserver32` holding
      `build/hover.sbr`, after which every later build fails with
      `error C2470: cannot open program database` or
      `fatal error C1083: Cannot open compiler generated file:
      'build\hover.sbr': Permission denied`. `build.sh` only deletes
      `build/*.obj` and `build/hover.pdb`, so it cannot recover on its
      own: `rm -f build.pdb build/*.sb*` and rebuild. Related: each wine
      build leaves about five zombie processes reparented to pid 1, which
      nothing in the container reaps - a scripted sweep of a few hundred
      builds is enough to matter, so bound your search loops.

486. **Functional equivalence is checked by differential emulation
      (`tools/equiv/`, see its README).** Each non-matching function and
      its original run side by side under Unicorn on 64 seeded inputs,
      every callee stubbed; the calls, arguments, non-stack writes per call
      and the return value must agree. Its first full run (2026-10-12)
      found real behavioural bugs at 85-99% byte similarity, the kind no
      score or invariant checker sees: a missing `return 0` path
      (0x40F7F0), `== 1` for `!= 0` (0x4126C0), two inverted booleans
      (0x43D100), swapped arguments (0x43C3C0), the wrong render target
      (0x402250), a store on the wrong side of a callback (0x412980),
      members initialised that hover.exe leaves alone (0x41B870,
      0x422BD0). Run it on anything you touch:
      `tools/equiv/run.sh 0xADDR`.

487. **`cmp x,1 / sbb eax,eax` materialises a boolean.** Followed by
      `inc eax` it is `x != 0`; followed by `neg eax` it is `x == 0`;
      followed by `add eax,2` it is `x == 0 ? 1 : 2`. VC 2.x emits it for a
      comparison *stored or passed* as a value (`b = x != 0;`,
      `wsprintf(.., x == f)`), and for an `if` on a flag that the source
      tested before clearing it (0x4126C0: `b = (m_dwUnk0500 != 0);
      m_dwUnk0500 = 0; if (b) ...`). Read the trailing instruction before
      writing the comparison: 0x43D100 had two of them inverted.

488. **Never store a struct over `&param` to update two parameters.**
      `*(SIZE*)&nWidth = *(SIZE*)p;` writes nWidth and nHeight, but VC 2.x
      keeps nHeight cached in a register across the aliased store and uses
      the stale value afterwards (0x419530 lost its reset height). Assign
      the members one by one.

489. **A destructor with no vtable store of its own is compiler-generated.**
      A user-written destructor - even an empty one - starts by storing the
      class's vptr; the implicit one the compiler generates (no destructor
      declared) does not, and goes straight to the member/base
      destructors. hover.exe's 0x41E1E0 (CSub41E1E0), 0x41D540, 0x434100
      and the ??_E at 0x426660 are such. Deleting our declaration matches
      0x41E1E0 exactly, but the header change shifted register allocation
      in mazeobj.cpp (CFloorObj::Sub_42AD60 92.9% -> 74.3%, rule 20), so
      it is left as is; the only behavioural difference is that vptr store,
      which the base destructor overwrites at once.

490. **Static data cannot be matched.** `static` (and file-scope `const`,
      internal linkage in C++) globals never reach the linker map, so
      reccmp cannot name references to them and the emulator cannot
      translate pointers to them. Declare tables external with a
      `// GLOBAL:` marker; a `const` needs `extern const` to get a symbol.
      Ten such tables were exported on 2026-10-12 (accuracy +0.04).

491. **Stores of constants with no visible caller are a missing global
      constructor.** A function reached only from the `.CRT$XC` table (a
      pointer to it in `.data`, no `call` sites) that stores constants into
      globals is a translation unit's dynamic initializer, and the globals
      are a class object with an inline constructor: a plain `DWORD g = -1;`
      goes into `.data` with no code (rule 58). hover.exe:0x42FE10
      (`mov eax,-1` + two stores) is `CPoint g_ptSplashAnchor(-1, -1)`;
      without it the pair started at 0 and the Please Wait dialog took a
      branch the original never reaches. Our initializers are not marked
      (the compiler names them), so check by disassembly that the stores
      exist. Header toll (rule 346): removing the two dead `DWORD` externs
      from hover.h and declaring the point locally in its two users was
      the best of three placements (byte-weighted +0.58, 492 exact);
      declaring it in hover.h gave +0.53.

492. **The renderer had four classes declared twice; they are merged
      (2026-10-14).** Older rules use the retired names: `CSub402E00` is
      `CSub406090` (the rasterizer), `CSub403980` is `CSub406C90` (its span
      heap), `CSub403850` is `CSub410F20` (the render context at
      g_pUnk_4A1D68) and `CSub419520` is `RENDEROBJ` (0x419520 is its empty
      constructor). All four now live in texheap.h. Measured: merging is
      score-neutral except for the program-wide register re-roll of rule
      346, which here took back `Hover_sub_41F540`'s lucky roll from rule
      491 (87.6% -> 60.7%; byte-weighted excluding it 82.97% -> 82.98%,
      exact 492 -> 494); call and operand invariants unchanged. One function
      needed a respelling to stay exact: `Sub_403850` loads `m_pUnk60` into
      a local before the clamp (the original schedules the load early).
      The shade table at +0x60 is a `DWORD*` (the original indexes it with
      `lea [eax+edx*4]`), and the +0x6C table is `DWORD*` with the flat
      filler reading only the low byte - indexing it `[dwB * 4]` after the
      retype would read four entries too far (compiler warning C4244 caught
      it).
493. **Check the data, not only the code: `tools/equiv/datadiff.py`.**
      A global the original initialises lives in `.data`; ours in `.bss`
      starts at zero. Nothing in the code shows it, and reccmp scores the
      functions that read it 100%. The data comparer pairs every address
      the two builds' code references and diffs the bytes; it found
      g_dwUnk_4C57D0 (= 100 in hover.exe) and the -1 start of
      g_ptSplashAnchor (rule 491). Run it after adding or retyping a global.

494. **A vtable slot can name a forwarding override.** CSub424720's
      DestroyWindow slot points at 0x42FE00, a bare `jmp CWnd::DestroyWindow`
      (a ClassWizard override that only calls the base). Behaviourally it
      is the inherited function; declaring it shifts register allocation in
      every file including optionsdlg.h, so it is noted, not declared.

495. **How strong is an `ok`? `tools/equiv/mutate.py`.** It plants
      single-instruction bugs in our function and checks each: a sample of
      113 functions killed 79.6% before the fixes below and 86.1% after
      (516/599). Off-by-one comparisons are the weak kind (21/56): random
      inputs rarely sit on the boundary. Other survivors are mostly
      equivalent mutants or loops whose counts come from random inputs
      (few conclusive runs reach the body). Each harness gap it exposed was
      closed: frame buffers passed to callees are compared by contents,
      callee-saved registers must survive, a null base reached through
      `lea reg,[ptr+off]` or an older stub return is learned as a pointer,
      inconclusive runs no longer count as coverage, every comparison is
      also tried at its boundary, sized API outputs get realistic contents,
      and forced inputs apply in a fixed order (results no longer depend on
      Python's string hashing).

496. **Unreached code gets a static review: `tools/equiv/semdiff.py`.**
      After a full equiv.py run (82% of the original's basic blocks covered)
      the differing instructions inside unreached blocks came to ~2,900 lines
      in 60 functions; each was reviewed against the original. Two real
      (latent) differences were found and fixed: `CBumperView::Sub_4184B0`
      passed `dwSpan` to GetPixel where hover.exe passes the fill loops'
      counter (0 for an empty span), and `CHumanPlayer::Sub_42AD30` compared
      `(long)dwC < 1` where hover.exe's `cmp eax,1; sbb eax,eax` is an
      unsigned `dwC == 0`. Neither can trigger with the game's data. A third,
      `CMerlinTexture::Serialize` storing `dwAccum` before the `wX` read
      (across the archive refill) instead of after, was found by the
      emulator once the harness fixes below let it run.
497. **Harness false alarms closed in the same pass:** an import called
      through a callee-saved register (`mov esi,[SendMessageA]` ...
      `call esi`) is resolved by reaching definitions instead of guessing
      its pop count from the preceding pushes; prologue register saves in
      the entry block are no longer counted as arguments; a 1-based index
      that folds its -1 into a table displacement is checked against the
      table, not the global before it; a forced comparand that is an
      address of the original is translated for our image. Five "weak"
      functions became fully covered and one false divergence (0x43bcf0)
      went away.

498. **The zone table and the skill parameter pointers are typed
      (2026-10-14).** Older rules use the retired names:
      `g_adwUnk_4609A0` is `ZONE_DESCRIPTOR g_aZones[6]` (now in hover.h;
      its +0x220 "aliasing union" was three POINTs and two RECTs, and
      `m_dwPad34`/`m_dwPad3C` are the live `m_yTopDashEdge`/
      `m_yBottomDashEdge`), theApp's `m_pUnk014C/0150/0154` are
      `m_pHumanParams`/`m_pRobotParams`/`m_pRobotAIParams`, and CPlayer's
      `m_dwUnk0084` is `CPlayerParams* m_pParams`. Plain struct code
      reproduces the original's address arithmetic: `ZONE_DESCRIPTOR*`
      loops in Hover_sub_41F540 get VC's biased-pointer strength reduction
      by themselves, and Sub_4184B0's search keeps rule 205's register
      pair with `pSearch++` and `pSearch <= &g_aZones[5]` in place of the
      `nZoneInts` stride. The SIZE fields are signed (LONG), as the
      snap-threshold `/ 2` and CMainFrame's `>=` against a window width
      need. Measured: the edited functions keep their scores and pass
      equiv.py on every run; datadiff 0 differ. The header re-roll (rule
      346) moved unrelated code, including two compare encodings in the
      /Od CRobotPlayer::Sub_409350 (`mov eax,C; cmp eax,x` vs
      `cmp x,C`), which no source edit there reverts.

499. **`Sub_411D10` is CSub422C80's (the gauge), not CSub423030's
      (2026-10-14).** It stores the gauge rectangle (now `RECT m_rc` at
      +0x00), sets the orientation at +0x20 and the segment size at
      +0x2C from the segment count at +0x1C; the old declaration on
      CSub423030 read those as `m_rcDirty.bottom` and a
      `ScratchQuotient()` that "overwrote" CBitmap's vptr. It stays at
      its old place in bumpview.cpp (after CSub423030::Sub_405A10). That
      placement scored 81% against 63% next to the other CSub422C80
      methods, but rule 500's header change took it back to 63% with the
      function untouched: the difference was the rule 346 re-roll, not
      the position. Also: the beacon walks
      Sub_407160/Sub_4053A0 read `g_obArrUnk_4A4840` as `CBeacon*`, whose
      `m_arr84` (a CObArray at +0x84) is what the old `CPodObj` view saw
      as `m_dwUnk88` (m_pData) and `m_pUnk8C` (m_nSize); `m_arr84[i]` and
      `GetSize()` expand inline to the same loads.

500. **The 16.16 fixed-point WORD arrays are unions (2026-10-14).**
      CPlayer's `m_awUnk0090/0194/01AC/01D0[]` and `m_wUnk01F0..F6`,
      CGameObj's and CSub423030's `m_wUnk44..4A` are each declared as
      `union { DWORD m_fxName; struct { WORD m_wNameFrac, m_wName; }; }`.
      Whole-value accesses (`*(DWORD*)&m_awUnk01AC[2]`) became `m_fxVelY`,
      16-bit stores keep their width through the halves
      (`m_awUnk0194[5]` is `m_wHeading`, the 9-bit angle), and
      `*(long*)&`/`*(volatile long*)&` pins stay as casts on the new
      member. Old to new: 0090[0..3] m_fxUnk0090/0094 (never read),
      [4] m_fxHeightStep, [6] m_fxHeightRate, [8] m_fxTargetHeight,
      [10] m_fxHeight; 0194[0] m_fxTurnRate, [2] m_fxTurnImpulse,
      [4] m_fxHeading; 01AC[0] m_fxVelX, [2] m_fxVelY, [4] m_fxAccelX,
      [6] m_fxAccelY, [8..11] m_fxUnk01BC/01C0 (never read); 01D0[0]
      m_fxThrustAccel, [2] m_fxMaxSpeed, [4] m_fxBaseMaxSpeed; 01F0
      m_fxTurnInput, 01F4 m_fxThrustInput; CGameObj 44/48 m_fxX/m_fxY;
      CSub423030 44/48 m_fxScaleX/m_fxScaleY. CMerlinBSP's m_wUnk44..4A
      are real WORD node indices and stay. Measured: all 50 functions
      the rewrite touched keep or raise their scores; the header change
      re-rolled the rest of the program (byte-weighted excluding
      Hover_sub_41F540 83.13% -> 83.03%).

501. **LVL_PARAMS score triples and CBumperDoc's shade tables are typed
      (2026-10-14).** Older rules use the retired names: LVL_PARAMS
      `m_dwUnk34/38/3C` are `m_anLevelScore[0..2]`, `m_dwUnk40/44/48`
      `m_anCaptureScore[0..2]` and `m_dwUnk4C/50/54`
      `m_anRemainingScore[0..2]`, indexed by skill (theApp.m_dwUnk01C8);
      `(&LVLPARAM(n).m_dwUnk40)[skill]` is `LVLPARAM(n).m_anCaptureScore
      [skill]`. `CSub414550` is `CShadeTable` (now in bumperdoc.h) and
      `CBumperDoc::m_pad018C` is `CShadeTable m_aShade[2]` ([1] is the
      blue-tinted table used while a smoke bomb is out). Measured: every
      function touched keeps its score (Sub_4184B0 rose 60.25 -> 69.05 on
      the re-roll), byte-weighted 82.41% -> 82.47%, datadiff 0 differ.

502. **Renaming is almost always codegen-neutral; check it, and use
      tools/rename.py.**
      Probed 2026-10-14: renaming a class (CSub422C80), a global
      (g_dwUnk_460998), a method (Sub_411D10) or a member (theApp's
      m_dwUnk01C8) leaves every function's reccmp score unchanged (zero
      movers). Unlike rule 346's type additions, the names never re-roll
      register allocation; only /Od *locals* are name-sensitive (rule
      195), so locals are not renamed in bulk. tools/rename.py takes
      `token OLD NEW` (whole-word, src/ and res/) and `member CLASS OLD
      NEW` lines: a member's declaration is renamed and every use is then
      found by building and fixing the lines the compiler reports
      (C2039/C2065), so same-named members of other classes are left
      alone. It refuses a NEW that already occurs in src/ (`member!`
      overrides that for a deliberate shared name such as OnHelpInfo) or
      in the MFC/SDK headers (an MFC virtual of that name would be
      overridden). Two things outside src/ carry names: tools/
      orig_data.csv names the original's vtables and globals by decorated
      name (`??_7CSub430780@@6B@`); a stale entry there costs a few
      percent in every constructor that stores the vtable, so the tool
      renames those too. notes/renames.md maps every old name.
      One indirect effect: the linker orders our uninitialised globals by
      name, so renaming globals reorders .bss, and reccmp's inferred size
      of a global can then swallow a neighbouring string in its labels
      (0x423140 lost 0.9% when the original's string at 0x4C5704 started
      printing as `g_dwUnk_4C5700+4`). Compare every function's score
      exactly, not only the movers above a threshold. Where one line
      touches the same placeholder (`m_dwUnk04`) of two different structs,
      the compiler-driven pass can give one of them the other's new name;
      that either fails to compile or, if the wrong name exists in the
      true class too, moves an offset - which the per-function score
      comparison catches (texheap, 2026-10-14: two such lines, fixed).
      Exception found on the WaveMix group: renaming 34 globals declared
      in hover.h swapped two registers in Hover_LoadProgress (optionsdlg.cpp,
      94.29% -> 93.81%). Either half of the 34 alone changes nothing, so it
      is not one name but the set - the compiler's symbol hash, the same
      kind of chaos as rule 346, only much rarer. Measure every rename
      PR; do not pick names to dodge it.

503. **Retyping a DWORD that holds a pointer (2026-10-14, step 4).** Change
      the declaration and let the compiler find the uses: `(DWORD)p`
      assignments and `(T*)field` reads become plain, `(DWORD)this`
      comparisons drop their cast, and functions that took the pointer as a
      DWORD get the real parameter type (CMerlinWorld::AddSprite/
      RemoveSprite/RemoveObject, CSpanRasterizer's constructor,
      AllocBuffers and RenderWorld, CGameObj::SetMerlin, CWall::SetFace,
      CRobotState::FindVisibleFlag). Two things the compiler does *not*
      catch: arithmetic on the field (`pDoc->m_pWorld + 0x1C` is now scaled
      by sizeof(CMerlinWorld) - it was the static-face count,
      `m_arrStatics.GetSize()`), and a `< 1` test, which has to keep its
      `(DWORD)` cast to stay an unsigned compare. Grep every retyped name
      for `+`/`-` before building. The header change re-rolls registers
      program-wide (rule 346); judge the edited functions by their diffs,
      not their scores: ~CGameObj fell 75.0 -> 67.9 after its region-node
      loop was rewritten with REGIONNODE members, but the old DWORD*
      spelling scores 62.5 under the same roll.

504. **VC 2.2 keeps `long` and `DWORD` loads apart when it decides what
      may alias (2026-10-14, step 4).** CPodObj::Touches reads the player's
      collision probe through a pointer and the pod's cell position from a
      stack `DWORD adwPos[2]` that GetCellPos filled in. With the probe as
      `DWORD*` the original re-reads `[esp+0x10]` for every subtraction;
      once the probe became a struct of `long` (POINT) members, VC treated
      the two as unable to overlap and cached adwPos in registers
      (0.94 -> 0.72, 30 changed lines). Declaring the position as a
      `POINT` too (`GetCellPos((DWORD*)&ptPos)`) restores the original
      exactly. So when a retype turns one side of a mixed read into `long`,
      expect neighbouring loads of the other type to start being hoisted,
      and match the types up instead of casting back. Other functions in
      the same PR with drops either scored the same under the old spelling
      (CSegment::DistanceTo) or changed only under the full revert
      (CPlayer::CollidePlayer, 3 lines) - rule 346 noise.

505. **Arguments read as one struct were struct arguments (2026-10-14,
      step 5).** Five functions took the address of their first coordinate
      argument and read the following stack slots as a POINT or RECT
      (`(POINT*)&dwA` in the IsNear overrides, `*(RECT*)&dwLeft` in
      CMapRenderer::SetClipRect/SetViewport and CVelocityIndicator::
      CreateTile). The callers show what the original was: ZoomMap loads
      the rectangle's last three fields into registers, steps its pointer
      to the first (`add edi, 0x258`) and pushes them from the end, which
      is VC 2.2's copy of a 16-byte struct argument, not four separate
      pushes. Declaring the parameter as `POINT pt` / `RECT rc` by value
      keeps every callee's code (the IsNear bodies stay exact) and moves
      the callers closer (ZoomMap 0.53 -> 0.63). It also removes a 64-bit
      break, since x64 passes the first four arguments in registers. Pass
      the struct itself at the call site (`pZone->m_rcMap`): copying
      through a `RECT*` local is what produced field-by-field pushes in
      the old spelling.

506. **Free-list pool carving is the plain indexed loop (2026-10-14,
      step 5).** `for (i = 0; i < nCount - 1; i++) pBlock[i].m_pNextFree =
      &pBlock[i + 1];` with an `int` count compiles to the original's
      strength-reduced walk (link address in edx, `lea esi, [edx +
      sizeof - offsetof]`, `add edx, sizeof`) for CRobotPlayer and
      CMerlinDynamic, exactly as CBeacon's already did. A `DWORD` count
      makes VC recompute `count - 1` for the trip counter (-4%), and an
      explicit pointer walk is worse (-23%). The literal `+0x260`/`+0x688`
      byte arithmetic that stood in for it is gone.

507. **A cosmetic edit can re-roll a whole file; keep the literal when it
      does (2026-10-14, audit leftovers).** Two readability-only changes
      cost far more than they were worth, with the edited functions
      themselves unchanged: replacing Hover_LoadGameSettings/
      SaveGameSettings' `BYTE buf[5]` with a 5-byte GAMESETTINGS struct
      moved 30 other hover.cpp functions (-0.38% byte-weighted; the same
      with the struct declared in hover.h), and spelling the flat-run
      arena's `m_dwRows * 2` as `m_dwRows * sizeof(WORD)` in
      AddFlatRun/ResetHeap moved FlushRuns 0.35 -> 0.17 (-0.23%). Neither
      changes any instruction of the function edited. Both were reverted
      and the layout documented in a comment instead. Measure every
      cosmetic change on its own build, and prefer a comment over a
      spelling change when the build moves.
