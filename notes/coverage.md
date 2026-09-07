# Coverage gap analysis — original hover.exe .text

> **Status note (2026-09-04):** this is a snapshot from 2026-08-11 (183 annotated
> functions). It is still the best map of *which* original functions are game
> code and what they reference, but the totals are stale: as of 2026-09-04,
> 363 game functions / 90.0 KB of the 241.7 KB of game code below the library
> frontier are decompiled (37.2% by bytes) and 3 stubs remain (0x401260,
> 0x406D30, 0x411770). 0x401216 in the table below is really two functions:
> the CMerlinLine destructor helper at 0x401216 and the segment-intersection
> routine at 0x401260 that 0x4045E0/0x406D30 call. Several catalog addresses
> are 6 bytes past the true function start (e.g. 0x407436 is 0x407430). The
> generating scripts lived in /tmp and are gone; the method below is
> reproducible from the description.


Generated 2026-08-11 by /tmp/hover-recon/analyze.py + classify.py + report.py + gen_md.py
(capstone fingerprint matching against build/hover.map symbols and the COFF objects inside
msvc22 LIBCMT.LIB / NAFXCW.LIB / LIBC.LIB; analysis state in /tmp/hover-recon/*.pkl).

Original .text: 0x401000..0x45F56E (386,560 bytes = 377.5 KB). Annotated/decompiled: 183 functions (FUNCTION/STUB markers), 48.2 KB.

## Method

1. Candidate function starts in the original come from an int3-padding scan (coverage.pkl); 183 are already annotated. The other 855 regions (329 KB raw, including inter-function padding) were classified as follows.
2. **Exact library twins**: normalized instruction fingerprints (mnemonic + operand kinds, all immediates/addresses stripped) of every library function in our build (map modules NAFXCW/LIBCMT/import libs = 1,193 functions) plus every function parsed from the COFF objects in the MSVC 2.2 LIBCMT.LIB, NAFXCW.LIB and LIBC.LIB archives (6,192 functions) were searched in the missing regions. Matches required >=10-token full-function equality or >=20-token prefix ending in `ret`.
3. **Fuzzy twins** (difflib ratio >= 0.90 over the token stream, >=15 tokens): catches MFC 3.1-vs-3.2 and CRT drift — the original was linked by linker 2.55 (VC++ 2.1), we build with 2.2, so some library functions differ by a few instructions.
4. **Library frontier**: scanning from the top of .text down, sustained library coverage (exact+fuzzy) begins at **0x43E400** — immediately above the highest annotated game function (0x43DC40, WaveMix). This matches linker layout: game .text first, then MFC .text$AFX_* groups, then CRT. Everything unmatched above the frontier is library; everything below is missing game code.
5. Evidence per function was collected by a full-.text decode: direct E8/E9 callers, import calls via the IAT, referenced .rdata/.data strings, references to known game globals, and code pointers from .rdata/.data (vtables / MFC message maps).

Caveats:
- 65 small functions below the frontier (5.7 KB) fuzzy-match MFC scalar-deleting-destructor/constructor shapes (every C++ class generates lookalike `??_G` deleting destructors); they are counted as GAME because the linker never places library code below the frontier.
- Functions without `55 8B EC` prologues (naked/FPO asm) merge with a neighbour in the split; sizes of individual entries can be off, totals are not.
- Accounting delta vs. section size: ~7.3 KB (1.9%) of padding/overlap between ranges.

## Summary

| class | functions | bytes | KB |
|---|---|---|---|
| .text total | 1,009 candidate starts | 386,560 | 377.5 |
| decompiled (annotated) | 183 | 49408 | 48.2 |
| library — exact twin | 722 | 72426 | 70.7 |
| library — fuzzy twin (MFC 3.1/CRT drift) | 336 | 49890 | 48.7 |
| library — unmatched, above frontier | 164 | 18140 | 17.7 |
| **missing GAME code** | **863** | **189410** | **185.0** |

Library code intentionally not decompiled: **137.2 KB** (722 exact + 336 fuzzy + 164 unmatched-band ranges).
Missing game code: **185.0 KB in 863 functions** (vs. 48.2 KB / 183 done → ~21% of game code by bytes).

Size profile of the missing game functions: >=512 B: 73 funcs / 91.0 KB · 256-511 B: 89 / 31.1 KB · 128-255 B: 227 / 38.0 KB · 64-127 B: 207 / 19.3 KB · <64 B: 267 / 5.6 KB.

## Renderer located

The software renderer writes into the offscreen DIB that `CBumperView::OnDraw` blits from `m_dcUnk00B4`. The frame pipeline is a cluster of mutually-calling missing functions low in .text:

| VA | size | role | evidence |
|---|---|---|---|
| 0x402256 | 946 | **per-frame render step** (member of the view/render object) | wraps body in `Enter/LeaveCriticalSection(&g_csUnk_46065C)` — the exact CS that guards the OnDraw blit; calls scene update 0x407160, then present 0x407436; PatBlt, GdiFlush, GetTickCount |
| 0x407436 | 4,338 | **composite + present** | member fn (ecx=this) testing `[this+0x544]` = `m_dwUnk0544`, the same StretchBlt-vs-BitBlt flag OnDraw checks; BitBlt/StretchBlt/GdiFlush; checks `g_dwUnk_460958` like OnDraw |
| 0x407160 | 706 | scene/object iteration | called only from 0x402256; walks `g_listUnk_4A4748` (CPtrList) |
| 0x404BB0 | 2,550 | object transform/animation | called from 0x402256/0x407160; uses `g_adwSinTable` fixed-point trig; writes position fields |
| 0x411D10 | 705 | **offscreen buffer creation** | CreateDIBSection + CreateCompatibleDC + GetDC; called from dashboard/view init 0x410386 |
| 0x417E30 | 939 | DC/palette setup | CreateCompatibleDC, GdiSetBatchLimit, EnterCriticalSection; refs g_dwUnk_460728/460740 |
| 0x4184C0 | 1,383 | background layer | "BACKGRND" resource string, GetPixel, PatBlt, SelectObject, refs `g_paletteUnk_4A1D50` |
| 0x401E80 | 859 | blit helper | BitBlt/StretchBlt/InflateRect |
| 0x41ECE0 | 1,154 | DisplayDib fallback | "DISPDIB.DLL", "DisplayDibWindow", GetTickCount, critical sections |

No direct E8 caller of 0x402256 exists and no .rdata/.data pointer targets it, so it is reached by virtual dispatch (vtable slot) or as the render-thread callback — consistent with the worker-thread machinery already decompiled in bumpview.cpp (m_hUnk0048..005C events, g_dwUnk_460598 thread handle).

The FPU-heavy cluster 0x430B80 / 0x431D10 / 0x4315E0 / 0x432660 / 0x432F50 / 0x433370 is **not** the renderer: those functions interpolate the per-skill-level parameter blocks `g_adwHBegParams`/`g_adwRBegParams`/`g_adwRobotAI*` (hover.cpp comments: H = human, R = robot, RobotAI = robot AI) → gameplay physics/AI.

## Missing game code by subsystem

| subsystem | funcs | KB | notes |
|---|---|---|---|
| game (other) | 539 | 74.7 | small helpers, thunks, ctors/dtors, msg handlers |
| physics/AI params | 166 | 35.7 | skill-level param interpolation, movement integration, robot AI |
| game logic/doc | 41 | 10.4 | CBumperDoc-centric gameplay logic |
| input | 33 | 10.6 | joystick (joyGetPosEx), keyboard (GetAsyncKeyState), control mapping |
| renderer/display | 22 | 10.9 | frame pipeline, DIB/DC setup, palette, blits |
| maze objects | 18 | 6.2 | FORCE_FIELD / BEACON_ / POD02_/POD03_ object types |
| UI | 10 | 8.0 | menus, layout rects, dialogs |
| config/registry | 10 | 2.2 | Software\Microsoft\Hover! settings |
| threads/sync | 8 | 2.5 | render/timer worker synchronization |
| 3D math | 6 | 5.5 | sin/tan-table fixed-point users |
| dashboard | 3 | 3.7 | Arial/dashres.dll instrument panel |
| sound/WaveMix | 3 | 2.0 | MIDI stream, sound file paths, WaveMix debug |
| texture mgr | 1 | 9.0 | fixed-heap texture cache (0x415450) |
| renderer/background | 1 | 1.4 | BACKGRND bitmap layer |
| physics/renderer (fpu) | 1 | 1.0 | unresolved FPU-heavy |
| display/DIB | 1 | 1.2 | DISPDIB.DLL fallback |

## Catalog: missing game functions >= 256 bytes

Callers are direct E8/E9 callers (`lib:`/`lib~:` = library twin, names from markers when decompiled). Strings/imports/globals are evidence collected from the instruction stream.

| VA | size | subsystem | callers | strings | imports | globals |
|---|---|---|---|---|---|---|
| 0x415450 | 9229 | texture mgr | - | %s%02d_%02d; Blowing fixed memory heap.  Increase MAXTEXTURESIZE in CTexture.h and re-compile.; DECAL_ | CreatePalette, OutputDebugStringA, TerminateThread | g_cbUnk_46002C, g_dwUnk_46049C, g_dwUnk_46073C |
| 0x41F4E0 | 6372 | UI | CHoverApp::InitInstance, unk_41E736, unk_425AD0 | - | RedrawWindow, SetRect | - |
| 0x409350 | 4405 | - | unk_409300 | - | - | - |
| 0x407436 | 4338 | renderer/display | - | - | BitBlt, GdiFlush, StretchBlt | g_dwUnk_4606BC, g_dwUnk_460740 |
| 0x430B80 | 2643 | physics/AI params | - | - | - | g_adwHBegParams |
| 0x404BB0 | 2550 | 3D math | unk_402256, unk_407160 | - | - | - |
| 0x410386 | 2385 | dashboard | unk_4184B0, unk_41ECE0 | Arial; dashres.dll | CreateFontA, GdiFlush, PatBlt | g_avkControls |
| 0x431D10 | 2379 | physics/AI params | - | QPRh | - | g_adwRBegParams |
| 0x4030F0 | 1881 | input | unk_402E00 | - | - | - |
| 0x40EF30 | 1544 | - | unk_40C2B6, unk_411622 | - | - | - |
| 0x403A60 | 1476 | - | unk_402E00, unk_406096 | - | - | - |
| 0x4045E0 | 1473 | - | unk_406096, unk_406D30 | - | - | - |
| 0x410F20 | 1451 | - | unk_413C10, unk_4184B0, unk_420ED0 | - | - | - |
| 0x404030 | 1449 | - | unk_402E00, unk_406096 | - | - | - |
| 0x401520 | 1448 | - | unk_401AD0, unk_404BB0 | - | - | - |
| 0x419E00 | 1426 | - | - | - | - | - |
| 0x41E736 | 1406 | - | - | - | - | g_adwUnk_4C4CC8, g_dwUnk_460734 |
| 0x4184B0 | 1399 | renderer/background | unk_41ECE0 | BACKGRND | GetPixel, PatBlt, SelectObject | g_dwUnk_460740 |
| 0x412980 | 1380 | input | - | - | - | g_dwUnk_461CFC |
| 0x406096 | 1363 | physics/AI params | - | - | - | - |
| 0x40C620 | 1301 | - | unk_40D4B0, unk_41A3A0, unk_41A6B0 | - | - | - |
| 0x41ECE0 | 1181 | display/DIB | unk_41ECC0 | DISPDIB.DLL; DisplayDibWindow | EnterCriticalSection, GetTickCount, LeaveCriticalSection | g_dwUnk_460734, g_dwUnk_460958 |
| 0x4065F0 | 1158 | physics/AI params | unk_402630, unk_403A60, unk_404030 | - | - | - |
| 0x4055B0 | 1117 | 3D math | unk_404BB0, unk_407160 | - | - | g_dwUnk_460744 |
| 0x428D96 | 1065 | - | - | hover.hlp | AppendMenuA, CreatePopupMenu, GetCursorPos | g_dwUnk_460728, g_dwUnk_460740 |
| 0x406D30 | 1062 | - | unk_406096 | - | - | - |
| 0x40D900 | 1053 | physics/AI params | CBumperView::Sub_40EE90, unk_4126C0, unk_413C70 | - | ResetEvent, SetEvent | - |
| 0x419530 | 1028 | physics/renderer (fpu) | unk_4184B0 | - | - | - |
| 0x412256 | 981 | - | - | - | - | - |
| 0x436860 | 948 | physics/AI params | - | - | - | - |
| 0x402256 | 946 | renderer/display | - | - | GdiFlush, PatBlt, EnterCriticalSection | g_csUnk_46065C, g_dwUnk_460740 |
| 0x40CCE6 | 943 | dashboard | - | Arial | CreateFontA, GetTextExtentPointA, InflateRect | - |
| 0x417E30 | 939 | input | - | - | CreateCompatibleDC, GdiSetBatchLimit, EnterCriticalSection | g_dwUnk_460728 |
| 0x401AD0 | 935 | - | unk_404BB0, unk_407436, unk_410386 | - | Rectangle | - |
| 0x433370 | 883 | physics/AI params | - | - | - | g_adwHBegParams |
| 0x40FEE0 | 870 | physics/AI params | unk_410386, unk_427AD8, unk_428970 | - | - | - |
| 0x401E80 | 859 | renderer/display | unk_407436, unk_410386 | - | BitBlt, StretchBlt, InflateRect | - |
| 0x4319A6 | 855 | - | - | - | - | - |
| 0x437F00 | 853 | sound/WaveMix | unk_41C030 | midiStreamClose; midiStreamOpen; midiStreamOut | CreateFileA, GetModuleHandleA, GlobalAlloc | g_pfnUnk_4C590C |
| 0x402630 | 852 | - | unk_404030 | - | - | - |
| 0x408DF0 | 797 | input | unk_408AF0 | - | - | g_dwUnk_460728 |
| 0x432F50 | 794 | physics/AI params | - | - | - | g_adwRobotAIBegParams |
| 0x430196 | 792 | input | - | - | - | - |
| 0x402E00 | 749 | input | unk_406096 | Trashing Heap | OutputDebugStringA | g_dwUnk_461CFC |
| 0x402B10 | 745 | physics/AI params | unk_403A60 | - | - | - |
| 0x42B3B0 | 741 | maze objects | - | FORCE_FIELD | - | g_dwUnk_460978 |
| 0x432660 | 710 | physics/AI params | unk_432930, unk_4329B0 | QPRh | - | g_adwRBegParams |
| 0x407160 | 706 | game logic/doc | unk_402256 | - | - | - |
| 0x411D10 | 705 | renderer/display | unk_410386 | - | CreateCompatibleDC, CreateDIBSection, GetDC | - |
| 0x41D850 | 705 | - | unk_41DCE0, unk_41DDF0, unk_426A60 | - | GetMenu, GetSystemMetrics, GetWindowLongA | g_dwUnk_460920 |
| 0x414D70 | 698 | game logic/doc | - | - | - | - |
| 0x405CE0 | 695 | renderer/display | unk_405FA0, unk_406096 | - | - | - |
| 0x42FE26 | 684 | - | - | - | - | - |
| 0x41A6B0 | 675 | 3D math | unk_40D730, unk_41B450, unk_42B166 | - | - | - |
| 0x422CB6 | 663 | threads/sync | CHoverApp::ExitInstance, unk_42EAC0 | - | DeleteCriticalSection, EnterCriticalSection, LeaveCriticalSection | g_apUnk_4A47F4, g_csUnk_4A4808, g_dwUnk_4C56D0 |
| 0x41AE40 | 662 | game logic/doc | - | - | - | g_dwUnk_46049C, g_dwUnk_460738, g_dwUnk_460974 |
| 0x4315E0 | 660 | physics/AI params | unk_431880, unk_431900 | - | - | g_adwHBegParams |
| 0x417860 | 653 | sound/WaveMix | unk_415386 | %sSounds\Mixed\%s.WAV; %sSounds\Unmixed\%s.WAV | wsprintfA | g_dwUnk_4C56D8 |
| 0x423EA6 | 648 | maze objects | - | BEACON_ | CharUpperA | g_dwUnk_4C57A8 |
| 0x42CC40 | 642 | - | - | - | - | - |
| 0x41BC40 | 617 | - | - | - | - | - |
| 0x401216 | 611 | - | unk_4045E0, unk_406D30, unk_40F7F0 | - | - | - |
| 0x42B166 | 574 | maze objects | - | POD07_ | wsprintfA | g_dwUnk_460978, g_pBumperDoc |
| 0x430930 | 569 | - | - | - | - | - |
| 0x4226D6 | 558 | maze objects | - | POD03_ | wsprintfA | g_dwUnk_460978, g_pBumperDoc |
| 0x432B70 | 557 | - | - | - | - | - |
| 0x4385F0 | 556 | 3D math | unk_405CE0, unk_409350, unk_40A4C0 | - | - | - |
| 0x4253D6 | 549 | - | - | Temp | GetMenu, GetSystemMetrics, GetWindowRect | - |
| 0x423C86 | 534 | game logic/doc | unk_423EA6 | - | - | - |
| 0x40BCD0 | 533 | - | - | - | - | - |
| 0x408AF0 | 523 | input | - | - | EnterCriticalSection, LeaveCriticalSection, GetAsyncKeyState | - |
| 0x425CB6 | 515 | game logic/doc | CHoverApp::InitInstance | Hover!InitInstance | CreateEventA, GetLastError, GetSystemInfo | g_dwUnk_46048C, g_dwUnk_46049C, g_pBumperDoc |
| 0x43A860 | 514 | sound/WaveMix | unk_422CB6, unk_42D760 |  FreeCN: NULL parameter!}; <WaveMixFreeWave>; Invalid session handle 0x%04X passed to WaveMix API | MessageBeep, MessageBoxA, wsprintfA | - |
| 0x40A656 | 508 | game logic/doc | unk_40A860, unk_414D70 | - | - | g_pBumperDoc |
| 0x4262B0 | 506 | UI | unk_415450, CHoverApp::InitInstance, unk_41ECE0 | - | GetLastError, LocalFree, MessageBoxA | - |
| 0x421076 | 503 | maze objects | - | POD02_ | wsprintfA | g_dwUnk_460978, g_pBumperDoc |
| 0x418FA0 | 494 | renderer/display | unk_415450 | - | GetDeviceCaps, GetPaletteEntries, GetSystemPaletteEntries | - |
| 0x4181E6 | 490 | maze objects | - | POD04_ | wsprintfA | g_dwUnk_460978, g_pBumperDoc |
| 0x427AD8 | 480 | renderer/display | unk_41ECE0 | - | GetClientRect | g_dwUnk_460958 |
| 0x42D160 | 477 | config/registry | CHoverApp::ExitInstance, CHoverApp::InitInstance | GameSettings; Software\Microsoft\Hover!; StartAtLevel | RegCloseKey, RegCreateKeyExA, RegOpenKeyExA | g_dwUnk_46048C, g_dwUnk_460734, g_dwUnk_460738 |
| 0x41DDF0 | 473 | config/registry | - | ScreenPosition; Software\Microsoft\Hover! | RegCloseKey, RegOpenKeyExA, RegQueryValueExA | - |
| 0x4140E0 | 471 | physics/AI params | unk_411D10, unk_426006, unk_4315E0 | - | - | - |
| 0x411B10 | 470 | - | unk_410386, unk_412980, unk_414740 | - | - | - |
| 0x42E0F6 | 470 | input | - | - | IsDlgButtonChecked | g_avkControls |
| 0x410CE6 | 461 | - | - | - | GetPaletteEntries | - |
| 0x41B940 | 455 | - | - | - | - | - |
| 0x40D730 | 453 | - | - | - | - | g_dwUnk_460978 |
| 0x428126 | 453 | dashboard | - | Small Font | CreateFontA, GetTextExtentPointA, InflateRect | - |
| 0x42D916 | 449 | - | - | - | - | - |
| 0x419960 | 443 | - | unk_419530 | - | GetNearestPaletteIndex | - |
| 0x414740 | 440 | - | unk_419E00, unk_41BC40, unk_42CC40 | - | - | - |
| 0x418C06 | 440 | renderer/display | - | - | - | g_paletteUnk_4A1D50 |
| 0x419B36 | 433 | - | - | - | - | - |
| 0x422476 | 420 | maze objects | - | POD05_ | wsprintfA | g_dwUnk_460978 |
| 0x41A960 | 418 | - | - | - | - | - |
| 0x430786 | 413 | - | - | - | - | - |
| 0x40B540 | 412 | input | - | - | - | g_avkControls |
| 0x42A766 | 405 | maze objects | - | POD00_ | wsprintfA | g_dwUnk_460978, g_pBumperDoc |
| 0x42AAC6 | 405 | maze objects | - | POD01_ | wsprintfA | g_dwUnk_460978, g_pBumperDoc |
| 0x4126C0 | 402 | input | unk_413C70, unk_417E30, unk_429380 | Timer could not be started; Timer tried to start twice | GetTickCount, RedrawWindow, SetMenu | g_dwUnk_460598, g_dwUnk_460728 |
| 0x422920 | 402 | game logic/doc | - | - | - | g_dwUnk_460978, g_pBumperDoc |
| 0x4283F6 | 397 | - | - | - | - | - |
| 0x41F1C0 | 389 | - | unk_41F180 | - | CreatePen, CreateSolidBrush | - |
| 0x42F646 | 389 | - | - | - | - | g_dwUnk_46048C |
| 0x425B26 | 387 | game logic/doc | - | - | RedrawWindow | g_dwUnk_46048C, g_dwUnk_46049C, g_pBumperDoc |
| 0x42E796 | 384 | - | - | - | - | - |
| 0x406A80 | 382 | - | unk_4030F0 | - | - | - |
| 0x432DB0 | 381 | physics/AI params | unk_432F30, unk_4332D0 | - | - | g_adwRobotAIBegParams |
| 0x402990 | 370 | physics/AI params | unk_402630, unk_404030 | - | - | - |
| 0x421B50 | 370 | 3D math | unk_41A6B0 | - | - | - |
| 0x42DD76 | 369 | - | - | - | - | - |
| 0x428A90 | 365 | renderer/display | - | - | EnterCriticalSection, LeaveCriticalSection | g_csUnk_46065C, g_pBumperDoc |
| 0x429380 | 363 | game logic/doc | - | - | - | g_dwUnk_460728, g_pBumperDoc |
| 0x405A10 | 360 | 3D math | unk_402256 | - | - | - |
| 0x414550 | 352 | input | unk_415450, unk_427060, unk_4270D0 | - | GetPaletteEntries | - |
| 0x41E5A0 | 344 | game logic/doc | unk_425B26, unk_425CB6 | - | - | g_pBumperDoc |
| 0x42B006 | 339 | game logic/doc | - | - | - | g_dwUnk_4C5330, g_pBumperDoc |
| 0x42FB66 | 338 | - | - | - | SendMessageA | g_dwUnk_460738 |
| 0x41B566 | 337 | - | - | - | - | - |
| 0x405B90 | 332 | - | unk_405FA0, unk_406096 | - | - | - |
| 0x40D4B0 | 325 | - | - | - | - | - |
| 0x40A860 | 324 | - | - | - | - | - |
| 0x411622 | 322 | - | - | - | - | - |
| 0x4247F0 | 322 | - | - | - | MoveWindow, SetWindowPos | g_dwUnk_4A4878, g_dwUnk_4A487C |
| 0x42EAC0 | 318 | config/registry | - | Software\Microsoft\Hover!; SoundSettings; hover.hlp | RegCloseKey, RegCreateKeyExA, RegSetValueExA | g_dwUnk_460974, g_dwUnk_460978, g_dwUnk_46097C |
| 0x40C2B6 | 316 | - | - | - | - | - |
| 0x41DB96 | 315 | - | - | - | GetTextExtentPointA, SendMessageA | - |
| 0x408820 | 312 | - | - | - | - | - |
| 0x414900 | 312 | - | unk_4184B0 | - | - | g_dwUnk_4C52D8 |
| 0x41A3A0 | 312 | - | - | - | - | - |
| 0x413C70 | 311 | - | - | - | RedrawWindow | g_dwUnk_460728, g_dwUnk_4C4CE0 |
| 0x425736 | 308 | - | - | - | - | - |
| 0x438300 | 307 | - | unk_413710 | - | - | - |
| 0x4191B6 | 304 | renderer/display | - | - | - | g_paletteUnk_4A1D50 |
| 0x410250 | 301 | game logic/doc | unk_408D00, CBumperView::Sub_40EE90, unk_411586 | - | SetMenu, ShowCursor | g_dwUnk_460598, g_dwUnk_460920 |
| 0x425600 | 301 | - | unk_415300, unk_41DB20, unk_425A20 | - | SetMenu, SetWindowPos, ShowWindow | - |
| 0x42D626 | 301 | threads/sync | - | - | EnterCriticalSection, LeaveCriticalSection, ResetEvent | g_csUnk_4A4808, g_dwUnk_4C56E8, g_hUnk_4C56F4 |
| 0x40BA10 | 299 | game logic/doc | unk_40BB40, unk_40BCD0 | - | - | g_pBumperDoc |
| 0x426F26 | 299 | - | - | - | - | - |
| 0x41F350 | 298 | - | unk_41E710 | - | SelectObject | - |
| 0x40FBB0 | 294 | physics/AI params | unk_40FEE0 | - | - | - |
| 0x411770 | 292 | - | unk_410386 | - | - | - |
| 0x413056 | 290 | maze objects | - | POD06_ | wsprintfA | - |
| 0x427D20 | 290 | input | unk_408D00 | - | GetTickCount, ResetEvent, SetEvent | g_avkControls |
| 0x41AB10 | 284 | - | unk_41A6B0, unk_421B50 | - | - | - |
| 0x430656 | 282 | - | - | - | - | - |
| 0x4274B0 | 278 | threads/sync | unk_413FE0, unk_417E30, unk_41F480 | - | ResetEvent, SetEvent | g_csUnk_46068C |
| 0x40E730 | 277 | - | unk_40E9A0, unk_40FCE0, unk_411A00 | - | - | - |
| 0x422256 | 277 | - | - | - | - | g_dwUnk_4C54D0 |
| 0x42E2D0 | 275 | - | - | - | - | - |
| 0x418AE6 | 273 | - | - | - | - | - |
| 0x40E9A0 | 272 | UI | unk_407436, unk_410386 | - | SetRect | - |
| 0x42C920 | 268 | - | - | - | - | - |
| 0x42EF40 | 267 | game logic/doc | - | - | - | g_dwUnk_460978, g_pBumperDoc |
| 0x411A00 | 266 | - | unk_410386 | - | - | - |
| 0x432A56 | 266 | - | - | - | - | - |
| 0x4275D0 | 263 | threads/sync | unk_40B540 | - | ResetEvent, SetEvent | g_csUnk_46068C |
| 0x42C420 | 263 | game logic/doc | - | - | - | g_dwUnk_460978 |
| 0x413A00 | 262 | threads/sync | unk_413FE0 | - | ResetEvent, SetEvent | g_csUnk_46068C, g_dwUnk_460734 |
| 0x41DCE0 | 262 | UI | - | - | GetClientRect, GetWindowRect, SetWindowPos | - |
| 0x40C0E0 | 260 | - | unk_40BFF0 | - | - | - |
| 0x42E686 | 258 | - | - | - | - | - |

## Catalog: missing game functions 128-255 bytes

| VA | size | subsystem | callers | strings/imports |
|---|---|---|---|---|
| 0x401036 | 129 | - | - | - |
| 0x403980 | 215 | - | unk_402630, unk_4065F0 | Blowing floor heap |
| 0x405FA0 | 232 | - | unk_406096 | - |
| 0x406C60 | 145 | - | unk_402256, unk_406096 | - |
| 0x4086A0 | 214 | - | unk_408820, unk_40EF30, unk_413180 | - |
| 0x408D00 | 235 | game logic/doc | unk_408AF0 | PostMessageA |
| 0x40A4C0 | 203 | game logic/doc | - | - |
| 0x40A590 | 180 | game logic/doc | - | - |
| 0x40BB40 | 203 | - | unk_40BC90, unk_414D70 | - |
| 0x40BEF0 | 139 | - | unk_40BCD0 | - |
| 0x40BFF0 | 231 | - | unk_40BCD0 | - |
| 0x40CB40 | 252 | - | unk_414470, unk_41A3A0 | - |
| 0x40D0A0 | 160 | - | unk_40CCE6, unk_428126 | OffsetRect |
| 0x40D346 | 223 | - | - | - |
| 0x40D600 | 210 | physics/AI params | - | - |
| 0x40DE80 | 254 | game logic/doc | - | - |
| 0x40DF80 | 151 | physics/AI params | - | - |
| 0x40E020 | 151 | physics/AI params | - | - |
| 0x40E0C0 | 151 | physics/AI params | - | - |
| 0x40E160 | 151 | physics/AI params | - | - |
| 0x40E200 | 151 | physics/AI params | - | - |
| 0x40E380 | 248 | game logic/doc | unk_413E60, unk_414A70, unk_414D10 | - |
| 0x40E480 | 194 | - | - | SendMessageA |
| 0x40E850 | 141 | physics/AI params | - | - |
| 0x40E910 | 141 | physics/AI params | - | - |
| 0x40EAC0 | 141 | physics/AI params | - | - |
| 0x40EDD0 | 141 | physics/AI params | - | - |
| 0x40F540 | 141 | physics/AI params | - | - |
| 0x40F5D0 | 141 | physics/AI params | - | - |
| 0x40F666 | 156 | physics/AI params | - | - |
| 0x40F7F0 | 172 | - | unk_412256, unk_41B0E0, unk_41B166 | - |
| 0x40F9B0 | 213 | - | unk_412256, unk_417D46 | - |
| 0x40FCE0 | 235 | UI | unk_407436, unk_410386 | SetRect |
| 0x4114D0 | 146 | physics/AI params | - | - |
| 0x411940 | 177 | physics/AI params | - | - |
| 0x411FE0 | 177 | physics/AI params | - | - |
| 0x4120A0 | 177 | physics/AI params | - | - |
| 0x412160 | 177 | physics/AI params | - | - |
| 0x412F50 | 171 | physics/AI params | - | - |
| 0x413180 | 154 | - | unk_421640 | - |
| 0x4132A6 | 165 | - | - | - |
| 0x413350 | 150 | physics/AI params | - | - |
| 0x4133F0 | 150 | physics/AI params | - | - |
| 0x413490 | 150 | physics/AI params | - | - |
| 0x413530 | 150 | physics/AI params | - | - |
| 0x4135D0 | 150 | physics/AI params | - | - |
| 0x413670 | 150 | physics/AI params | - | - |
| 0x413730 | 151 | input | unk_410CE6, unk_414550 | - |
| 0x413B10 | 253 | threads/sync | unk_413FE0, unk_4258F6 | ResetEvent, SetEvent |
| 0x413F16 | 197 | - | - | - |
| 0x413FE0 | 246 | - | - | SendMessageA |
| 0x414370 | 247 | game logic/doc | - | - |
| 0x4150F0 | 237 | game logic/doc | - | - |
| 0x415386 | 190 | - | - | - |
| 0x417B00 | 153 | - | - | flagr_dr |
| 0x417C16 | 212 | - | - | - |
| 0x417D46 | 221 | - | - | - |
| 0x4183E0 | 203 | input | - | RedrawWindow, wsprintfA |
| 0x418DC0 | 217 | - | - | - |
| 0x418F00 | 149 | renderer/display | unk_418FA0 | CreatePalette, DeleteObject, RealizePalette |
| 0x4192F0 | 161 | - | unk_4191B6 | - |
| 0x4193A6 | 149 | - | - | - |
| 0x419440 | 221 | - | unk_4191B6 | GetDC, ReleaseDC |
| 0x419CF6 | 231 | - | - | - |
| 0x41A616 | 147 | - | - | - |
| 0x41B0E0 | 130 | - | unk_412256, unk_415450, unk_41B2D0 | - |
| 0x41B166 | 196 | maze objects | - | POD06_ |
| 0x41B450 | 151 | - | - | - |
| 0x41B6C6 | 168 | - | - | - |
| 0x41BB86 | 129 | - | - | - |
| 0x41BF80 | 155 | - | - | - |
| 0x41C8E0 | 243 | input | unk_425AD0, unk_425B26, unk_425CB6 | LoadStringA, SetWindowTextA, UpdateWindow |
| 0x41DFD6 | 146 | - | - | FreeLibrary |
| 0x41E0D6 | 235 | - | - | - |
| 0x420ED0 | 131 | - | unk_417E30, unk_4184B0, unk_420E10 | - |
| 0x420FD6 | 156 | maze objects | - | POD00_ |
| 0x421316 | 159 | maze objects | - | POD01_ |
| 0x421446 | 209 | maze objects | - | POD04_ |
| 0x421526 | 159 | maze objects | - | POD07_ |
| 0x421640 | 188 | - | - | - |
| 0x421716 | 156 | maze objects | - | POD02_ |
| 0x421896 | 150 | - | - | - |
| 0x421936 | 145 | - | - | - |
| 0x4219D6 | 182 | - | - | - |
| 0x421A96 | 182 | - | - | - |
| 0x421CD6 | 203 | - | - | FLG00_; FLG01_ |
| 0x421E20 | 153 | input | - | - |
| 0x4223C6 | 159 | maze objects | - | POD05_ |
| 0x422636 | 156 | maze objects | - | POD03_ |
| 0x423AAF | 206 | - | - | - |
| 0x424186 | 132 | - | - | - |
| 0x424676 | 164 | - | - | - |
| 0x424726 | 158 | - | - | - |
| 0x425290 | 205 | - | unk_433370 | - |
| 0x4258F6 | 161 | - | - | - |
| 0x425A20 | 161 | - | - | GetKeyState |
| 0x425F46 | 182 | game logic/doc | - | - |
| 0x426006 | 157 | game logic/doc | - | - |
| 0x4264B0 | 174 | - | - | - |
| 0x426566 | 235 | - | - | - |
| 0x426736 | 186 | - | - | - |
| 0x426806 | 163 | - | - | - |
| 0x4268B6 | 163 | - | - | - |
| 0x426A60 | 216 | UI | - | GetClientRect, SetWindowPos |
| 0x426C00 | 154 | - | - | hover.hlp |
| 0x426CF6 | 252 | renderer/display | - | PatBlt, RealizePalette, GetClientRect |
| 0x4276E0 | 253 | threads/sync | unk_41F350 | ResetEvent, SetEvent |
| 0x4277E0 | 253 | threads/sync | unk_4126C0 | ResetEvent, SetEvent |
| 0x4282F0 | 243 | game logic/doc | unk_425AD0 | - |
| 0x428890 | 170 | - | - | RedrawWindow, ShowWindow |
| 0x429230 | 182 | - | - | RedrawWindow |
| 0x429746 | 176 | input | - | - |
| 0x429D20 | 146 | physics/AI params | - | - |
| 0x429DC0 | 156 | physics/AI params | - | - |
| 0x429EE0 | 151 | physics/AI params | - | - |
| 0x429F80 | 161 | physics/AI params | - | - |
| 0x42A0B0 | 183 | physics/AI params | - | - |
| 0x42A170 | 198 | physics/AI params | - | - |
| 0x42A240 | 142 | physics/AI params | - | - |
| 0x42A470 | 135 | physics/AI params | - | - |
| 0x42A660 | 241 | game logic/doc | - | - |
| 0x42A900 | 204 | game logic/doc | - | - |
| 0x42ADB6 | 206 | game logic/doc | - | - |
| 0x42AF20 | 134 | - | unk_412256, lib~:??_GAFX_TOOLTIP@@UAEPAXI@Z, unk_415450 | - |
| 0x42C320 | 137 | - | unk_415450 | - |
| 0x42CAB6 | 176 | - | - | - |
| 0x42CFE6 | 213 | - | - | - |
| 0x42D3E0 | 226 | renderer/display | unk_407436 | BitBlt, StretchBlt |
| 0x42D4D0 | 162 | - | - | - |
| 0x42D576 | 136 | - | - | - |
| 0x42DEF0 | 247 | input | - | joyGetDevCapsA, joyGetPos |
| 0x42DFF6 | 234 | - | - | - |
| 0x42E420 | 236 | config/registry | unk_42E0F6 | PlayerControls; Software\Microsoft\Hover!; hover.hlp |
| 0x42E5A6 | 210 | - | - | - |
| 0x42E920 | 214 | - | - | - |
| 0x42EA00 | 153 | - | - | - |
| 0x42EC06 | 187 | - | - | - |
| 0x42ED00 | 135 | game logic/doc | unk_40A656, unk_40BCD0 | - |
| 0x42EE10 | 237 | - | - | - |
| 0x42F896 | 240 | - | - | - |
| 0x42F996 | 222 | - | - | - |
| 0x42FA86 | 210 | - | - | SetDlgItemTextA |
| 0x42FD36 | 187 | - | - | - |
| 0x4300E0 | 153 | - | - | - |
| 0x4304B0 | 203 | input | - | - |
| 0x430580 | 138 | - | - | - |
| 0x431900 | 154 | config/registry | - | HBegParams; HExpParams; HIntParams |
| 0x4329B0 | 154 | config/registry | - | RBegParams; RExpParams; RIntParams |
| 0x4332D0 | 154 | config/registry | - | RobotAIBegParams; RobotAIExpParams; RobotAIIntParams |
| 0x434470 | 129 | physics/AI params | - | - |
| 0x434500 | 139 | physics/AI params | - | - |
| 0x434B10 | 157 | physics/AI params | - | - |
| 0x434BB0 | 168 | physics/AI params | - | - |
| 0x434C60 | 141 | physics/AI params | - | - |
| 0x434CF0 | 141 | physics/AI params | - | - |
| 0x434D80 | 141 | physics/AI params | - | - |
| 0x434E10 | 141 | physics/AI params | - | - |
| 0x434EA0 | 130 | physics/AI params | - | - |
| 0x434F30 | 130 | physics/AI params | - | - |
| 0x434FC0 | 130 | physics/AI params | - | - |
| 0x435050 | 130 | physics/AI params | - | - |
| 0x4350E0 | 130 | physics/AI params | - | - |
| 0x435170 | 130 | physics/AI params | - | - |
| 0x435200 | 130 | physics/AI params | - | - |
| 0x435290 | 130 | physics/AI params | - | - |
| 0x435320 | 130 | physics/AI params | - | - |
| 0x4353B0 | 130 | physics/AI params | - | - |
| 0x435440 | 166 | physics/AI params | - | - |
| 0x4354F0 | 181 | physics/AI params | - | - |
| 0x4355B0 | 150 | physics/AI params | - | - |
| 0x435650 | 150 | physics/AI params | - | - |
| 0x4356F0 | 150 | physics/AI params | - | - |
| 0x435790 | 139 | physics/AI params | - | - |
| 0x435820 | 139 | physics/AI params | - | - |
| 0x4358B0 | 139 | physics/AI params | - | - |
| 0x435940 | 139 | physics/AI params | - | - |
| 0x4359D0 | 139 | physics/AI params | - | - |
| 0x435A60 | 139 | physics/AI params | - | - |
| 0x435AF0 | 139 | physics/AI params | - | - |
| 0x435B80 | 139 | physics/AI params | - | - |
| 0x435C10 | 139 | physics/AI params | - | - |
| 0x435CA0 | 139 | physics/AI params | - | - |
| 0x435D30 | 180 | physics/AI params | - | - |
| 0x435DF0 | 190 | physics/AI params | - | - |
| 0x435EB0 | 151 | physics/AI params | - | - |
| 0x435F50 | 151 | physics/AI params | - | - |
| 0x435FF0 | 151 | physics/AI params | - | - |
| 0x436090 | 151 | physics/AI params | - | - |
| 0x436130 | 140 | physics/AI params | - | - |
| 0x4361C0 | 140 | physics/AI params | - | - |
| 0x436250 | 140 | physics/AI params | - | - |
| 0x4362E0 | 140 | physics/AI params | - | - |
| 0x436370 | 140 | physics/AI params | - | - |
| 0x436400 | 140 | physics/AI params | - | - |
| 0x436490 | 140 | physics/AI params | - | - |
| 0x436520 | 140 | physics/AI params | - | - |
| 0x4365B0 | 140 | physics/AI params | - | - |
| 0x436640 | 140 | physics/AI params | - | - |
| 0x4366D0 | 188 | physics/AI params | - | - |
| 0x436790 | 198 | physics/AI params | - | - |
| 0x436C20 | 148 | physics/AI params | - | - |
| 0x436CC0 | 148 | physics/AI params | - | - |
| 0x436D60 | 148 | physics/AI params | - | - |
| 0x436E00 | 148 | physics/AI params | - | - |
| 0x436EA0 | 148 | physics/AI params | - | - |
| 0x436F40 | 148 | physics/AI params | - | - |
| 0x436FE0 | 148 | physics/AI params | - | - |
| 0x437080 | 148 | physics/AI params | - | - |
| 0x437120 | 148 | physics/AI params | - | - |
| 0x4371C0 | 220 | physics/AI params | - | - |
| 0x4372A0 | 246 | physics/AI params | - | - |
| 0x4373A0 | 177 | physics/AI params | - | - |
| 0x437460 | 177 | physics/AI params | - | - |
| 0x437520 | 177 | physics/AI params | - | - |
| 0x4375E0 | 177 | physics/AI params | - | - |
| 0x4376A0 | 177 | physics/AI params | - | - |
| 0x437760 | 177 | physics/AI params | - | - |
| 0x437820 | 166 | physics/AI params | - | - |
| 0x4378D0 | 166 | physics/AI params | - | - |
| 0x437980 | 166 | physics/AI params | - | - |
| 0x437A30 | 166 | physics/AI params | - | - |
| 0x437AE0 | 166 | physics/AI params | - | - |
| 0x437B90 | 166 | physics/AI params | - | - |
| 0x437C40 | 166 | physics/AI params | - | - |
| 0x437CF0 | 166 | physics/AI params | - | - |
| 0x437DA0 | 166 | physics/AI params | - | - |
| 0x437E50 | 166 | physics/AI params | - | - |

Functions < 128 bytes: 474 total (24.9 KB) — mostly ctors/dtors, scalar-deleting destructors, thunks and small handlers; decompile together with their subsystem batch. Full per-function data: /tmp/hover-recon/final.pkl.


## Proposed batching plan

Coherent clusters for sequential decompilation PRs, ordered so that foundational
pieces (heap, renderer, globals naming) land before their consumers. Sizes are the
missing bytes in the cluster's >=256 B functions; each batch also owns the small
(<256 B) functions in its address neighbourhood.

### Batch 1 — fixed-heap allocator + texture manager (~15 KB)
- 0x415450 (9,229 B) — texture manager init/load: "Blowing fixed memory heap.
  Increase MAXTEXTURESIZE in CTexture.h", "DECAL_", "PAD_", "%s%02d_%02d" naming,
  CreatePalette, TerminateThread (texture preload thread).
- Fixed-heap allocator cluster around `g_dwUnk_461CFC`: 0x412980 (1,380),
  0x402E00 (749, "Trashing Heap"), 0x402B10 (745), 0x4065F0 (1,158), 0x4030F0 (1,881).
- Texture-name/loader helpers: 0x412256 (981, "%s%02d"), 0x410F20 (1,451),
  0x415386 (190).
Why first: nearly every later subsystem (renderer, dashboard, maze) references the
texture/heap globals; decompiling it pins down `MAXTEXTURESIZE` data layout.

### Batch 2 — renderer frame pipeline (~14 KB)
- 0x402256 (946) frame step · 0x407436 (4,338) composite/present ·
  0x407160 (706) scene iteration · 0x404BB0 (2,550) object transform (sin-table) ·
  0x4055B0 (1,117) · 0x405A10 (360) · helpers 0x406C00/0x406C40/0x406C60.
- Buffer/DC setup: 0x411D10 (705, CreateDIBSection), 0x417E30 (939),
  0x401E80 (859, blit helper), 0x4184B0 (1,399, "BACKGRND" layer),
  0x41ECE0 (1,181, DISPDIB fallback).
Deliverable: names for the offscreen-DIB globals used by `CBumperView::OnDraw`;
validates the render-thread model already in bumpview.cpp.

### Batch 3 — dashboard (~6 KB)
- 0x410386 (2,385, "Arial"/"dashres.dll" per landmark) panel init/draw ·
  0x40CCFF (918, "Arial" text metrics) · 0x428126 (453, "Small Font") ·
  layout helpers 0x4205F9 / 0x42093D (SetRect) · menu/UI 0x41D850 (705),
  0x428D96 (1,065, "hover.hlp", AppendMenu).

### Batch 4 — gameplay physics & AI (~12 KB core)
- Skill-param interpolators: 0x430B80 (2,643, H params, 270 FPU ops),
  0x431D10 (2,379, R params, 243 FPU), 0x4315E0 (660), 0x432660 (710),
  0x433370 (883), 0x432F50 (794, RobotAI), 0x436860 (948).
- Movement/collision users of `g_adwRIntParams`/event sync: 0x40D900 (1,053),
  0x40FEE0 (870), 0x406096 (1,363), 0x409350 (4,405).
Note: register how `Sub_425870`'s m_pUnk014C/0150/0154 pointers are consumed.

### Batch 5 — maze objects & game logic (~8 KB)
- Object-type handlers: 0x42B3B0 (741, "FORCE_FIELD"), 0x423EA6 (579, "BEACON_"),
  0x4226EE (534, "POD03_"), 0x42108E (479, "POD02_"), 0x423C86 (534),
  0x422920 (402), 0x40D730 (453), 0x414D70 (698), 0x427AD8 (480).
- Board/zone setup: 0x41F4E0 (6,372, called by CHoverApp::InitInstance, sits right
  before the already-known zone-geometry builder Hover_sub_41F540),
  0x41E736 (1,406, `g_adwUnk_4C4CC8`).

### Batch 6 — sound remainder (~3 KB)
- 0x437F00 (853) MIDI stream load ("midiStreamOpen"/"midiStreamClose",
  `g_pfnUnk_4C590C`/`g_pfnUnk_4C591C`) · 0x417860 (627, "%sSounds\\Mixed\\%s.WAV"
  path building) · WaveMix tail 0x43A860 (514, WaveMixFreeWave debug),
  0x438300 / 0x438440 / 0x4385F0.

### Batch 7 — config, input, timer, misc glue (~6 KB)
- Registry: 0x42D160 (477) & 0x41DDF0 (473) ("Software\\Microsoft\\Hover!").
- Input: 0x408AF0 (523, joyGetPosEx/GetAsyncKeyState, `g_csUnk_4606A4`),
  0x408DF0 (797, debug coords) — extends Hover_sub_4232E0 joystick work.
- Timer: 0x4126C0 (402, "Timer could not be started/tried to start twice").
- Resource-error UI: 0x4262B0 (506, archive-exception strings + MessageBox,
  called from InitInstance/0x41ECE0).

### Batch 8 — long tail of small functions (~25 KB)
Remaining <256 B functions: vtable/message-map handlers (use the data-pointer
evidence in final.pkl to bind them to their classes), ctors/dtors, and glue.
Sweep per address neighbourhood once the named batches provide context.
