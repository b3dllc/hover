// spandraw.h : the software renderer's hand-written span drawers
//
// The 167 parameterless span-drawing routines that CRenderContext::SetMode
// (texload.cpp) installs into the view object's drawer slots m_pfnDrawWall,
// m_pfnDrawSky, m_pfnDrawFlat and into the size-indexed table
// m_ppfnDrawFlatByShift[6..15]. Every one of them is hand-written assembly in
// hover.exe and is reproduced as a naked function operating on the renderer
// globals in texheap.cpp.
//
// The file is split by original address range, one .cpp (and one section
// below) per range; see the head of each .cpp for the family descriptions.
//
// Names follow one scheme across the three groups:
//   Column<W>x<H>[Lit][Reg]  a wall/object texture column (m_pfnDrawWall)
//   Sky<W>x<H>[Reg]          an unmapped horizontal run (m_pfnDrawSky)
//   Span<W>x<H>[Lit]_Var     a floor/ceiling span taking its v shift from
//                            g_dwVShift at run time (m_pfnDrawFlat)
//   Span<W>x<H>[Lit]_S<n>    the same with the shift n baked in, for a
//                            texture 2^(16-n) texels wide
//                            (m_ppfnDrawFlatByShift[n])
// W is screen pixels per texel across and H rows down (the mode's
// m_dwPixelW/m_dwPixelH), Lit means the texel goes through the shade row
// g_dwShadeTable, and Reg is the register-loop variant SetMode installs
// when its dwFlags bit 1 is set (every _S<n> drawer is one).

#ifndef __SPANDRAW_H__
#define __SPANDRAW_H__

/////////////////////////////////////////////////////////////////////////////
// group A

void Column4x2LitReg(void);	// wall column, 4-byte pixel, 2 rows per texel
void Sky4x2Reg(void);	// direct (unmapped) span, 4-byte pixel, 2 rows per texel
void Span4x2Lit_S10(void);	// floor/ceiling span, 4-byte pixel, 2 rows, v shift 0xA (row stride 64)
void Span4x2Lit_S11(void);	// floor/ceiling span, 4-byte pixel, 2 rows, v shift 0xB (row stride 32)
void Span4x2Lit_S12(void);	// floor/ceiling span, 4-byte pixel, 2 rows, v shift 0xC (row stride 16)
void Span4x2Lit_S13(void);	// floor/ceiling span, 4-byte pixel, 2 rows, v shift 0xD (row stride 8)
void Span4x2Lit_S14(void);	// floor/ceiling span, 4-byte pixel, 2 rows, v shift 0xE (row stride 4)
void Column2x2LitReg(void);	// wall column, 2-byte pixel, 2 rows per texel
void Sky2x2Reg(void);	// direct (unmapped) span, 2-byte pixel, 2 rows per texel
void Span2x2Lit_S10(void);	// floor/ceiling span, 2-byte pixel, 2 rows, v shift 0xA (row stride 64)
void Span2x2Lit_S11(void);	// floor/ceiling span, 2-byte pixel, 2 rows, v shift 0xB (row stride 32)
void Span2x2Lit_S12(void);	// floor/ceiling span, 2-byte pixel, 2 rows, v shift 0xC (row stride 16)
void Span2x2Lit_S13(void);	// floor/ceiling span, 2-byte pixel, 2 rows, v shift 0xD (row stride 8)
void Span2x2Lit_S14(void);	// floor/ceiling span, 2-byte pixel, 2 rows, v shift 0xE (row stride 4)
void Column2x1LitReg(void);	// wall column, 2-byte pixel, 1 row per texel
void Sky2x1Reg(void);	// direct (unmapped) span, 2-byte pixel, 1 row per texel
void Span2x1Lit_S11(void);	// floor/ceiling span, 2-byte pixel, 1 row, v shift 0xB (row stride 32)
void Span2x1Lit_S12(void);	// floor/ceiling span, 2-byte pixel, 1 row, v shift 0xC (row stride 16)
void Span2x1Lit_S13(void);	// floor/ceiling span, 2-byte pixel, 1 row, v shift 0xD (row stride 8)
void Span2x1Lit_S14(void);	// floor/ceiling span, 2-byte pixel, 1 row, v shift 0xE (row stride 4)
void Span2x1Lit_S9(void);	// floor/ceiling span, 2-byte pixel, 1 row, v shift 0x9 (row stride 128)
void Span2x1Lit_S10(void);	// floor/ceiling span, 2-byte pixel, 1 row, v shift 0xA (row stride 64)
void Column1x1LitReg(void);	// wall column, 1-byte pixel, 1 row per texel
void Sky1x1(void);	// direct (unmapped) span, 1-byte pixel, 1 row per texel
void Span1x1Lit_S9(void);	// floor/ceiling span, 1-byte pixel, 1 row, v shift 0x9 (row stride 128)
void Span1x1Lit_S10(void);	// floor/ceiling span, 1-byte pixel, 1 row, v shift 0xA (row stride 64)
void Span1x1Lit_S11(void);	// floor/ceiling span, 1-byte pixel, 1 row, v shift 0xB (row stride 32)
void Span1x1Lit_S12(void);	// floor/ceiling span, 1-byte pixel, 1 row, v shift 0xC (row stride 16)
void Span1x1Lit_S13(void);	// floor/ceiling span, 1-byte pixel, 1 row, v shift 0xD (row stride 8)
void Span1x1Lit_S14(void);	// floor/ceiling span, 1-byte pixel, 1 row, v shift 0xE (row stride 4)
void Column4x4LitReg(void);	// wall column, 4-byte pixel, 4 rows per texel
void Span4x4Lit_S11(void);	// floor/ceiling span, 4-byte pixel, 4 rows, v shift 0xB (row stride 32)
void Span4x4Lit_S12(void);	// floor/ceiling span, 4-byte pixel, 4 rows, v shift 0xC (row stride 16)
void Span4x4Lit_S13(void);	// floor/ceiling span, 4-byte pixel, 4 rows, v shift 0xD (row stride 8)
void Span4x4Lit_S14(void);	// floor/ceiling span, 4-byte pixel, 4 rows, v shift 0xE (row stride 4)
void Column4x1LitReg(void);	// wall column, 4-byte pixel, 1 row per texel
void Sky4x1Reg(void);	// direct (unmapped) span, 4-byte pixel, 1 row per texel
void Sky4x4Reg(void);	// direct (unmapped) span, 4-byte pixel, 4 rows per texel
void Span4x1Lit_S9(void);	// floor/ceiling span, 4-byte pixel, 1 row, v shift 0x9 (row stride 128)
void Span4x1Lit_S10(void);	// floor/ceiling span, 4-byte pixel, 1 row, v shift 0xA (row stride 64)
void Span4x1Lit_S11(void);	// floor/ceiling span, 4-byte pixel, 1 row, v shift 0xB (row stride 32)
void Span4x1Lit_S12(void);	// floor/ceiling span, 4-byte pixel, 1 row, v shift 0xC (row stride 16)
void Span4x1Lit_S13(void);	// floor/ceiling span, 4-byte pixel, 1 row, v shift 0xD (row stride 8)
void Span4x1Lit_S14(void);	// floor/ceiling span, 4-byte pixel, 1 row, v shift 0xE (row stride 4)
void Span2x2Lit_S8(void);	// floor/ceiling span, 2-byte pixel, 2 rows, v shift 0x8 (row stride 256)
void Span4x1Lit_S8(void);	// floor/ceiling span, 4-byte pixel, 1 row, v shift 0x8 (row stride 256)

// The drawers take no arguments, return nothing and are pure assembly: the
// original's register saves are interleaved with the working instructions,
// so the whole body including the prologue and the ret is written out and
// the compiler must not add one of its own.
#ifndef HOVER_NAKED
#define HOVER_NAKED void __declspec(naked)
#endif

/////////////////////////////////////////////////////////////////////////////
// group B - src/spandrawB.cpp, hover.exe:0x429960..0x435CA0
//
// Three families: the vertical wall/object texture columns (0x429960),
// the flat horizontal runs (0x42A2D0) and the two-axis floor/ceiling
// spans (0x434470).  "N x M px" is the pixel replication: N screen
// pixels across and M rows down per source texel.  "shaded" means the
// texel is looked up through the shade table g_dwShadeTable; "reg loop"
// means the drawer keeps the destination and texture positions in
// registers and never writes g_dwDst / 0x4A4A40 back.

void Column1x1Lit    (void);	// 0x429960  texture column, 1 x 1 px, shaded
void Column1x1    (void);	// 0x4299D0  texture column, 1 x 1 px, flat
void Column1x1Reg    (void);	// 0x429A30  texture column, 1 x 1 px, flat, reg loop
void Column2x1    (void);	// 0x429A80  texture column, 2 x 1 px, flat
void Column2x1Lit    (void);	// 0x429AF0  texture column, 2 x 1 px, shaded
void Column2x1Reg    (void);	// 0x429B70  texture column, 2 x 1 px, flat, reg loop
void Column4x1    (void);	// 0x429BD0  texture column, 4 x 1 px, flat
void Column4x1Lit    (void);	// 0x429C40  texture column, 4 x 1 px, shaded
void Column4x1Reg    (void);	// 0x429CC0  texture column, 4 x 1 px, flat, reg loop
void Column2x2    (void);	// 0x429D20  texture column, 2 x 2 px, flat
void Column2x2Lit    (void);	// 0x429DC0  texture column, 2 x 2 px, shaded
void Column2x2Reg    (void);	// 0x429E60  texture column, 2 x 2 px, flat, reg loop
void Column4x2    (void);	// 0x429EE0  texture column, 4 x 2 px, flat
void Column4x2Lit    (void);	// 0x429F80  texture column, 4 x 2 px, shaded
void Column4x2Reg    (void);	// 0x42A030  texture column, 4 x 2 px, flat, reg loop
void Column4x4    (void);	// 0x42A0B0  texture column, 4 x 4 px, flat
void Column4x4Lit    (void);	// 0x42A170  texture column, 4 x 4 px, shaded
void Column4x4Reg    (void);	// 0x42A240  texture column, 4 x 4 px, flat, reg loop
void Sky2x1    (void);	// 0x42A2D0  flat horizontal run, 2 x 1 px, flat
void Sky4x1    (void);	// 0x42A330  flat horizontal run, 4 x 1 px, flat
void Sky2x2    (void);	// 0x42A390  flat horizontal run, 2 x 2 px, flat
void Sky4x2    (void);	// 0x42A400  flat horizontal run, 4 x 2 px, flat
void Sky4x4    (void);	// 0x42A470  flat horizontal run, 4 x 4 px, flat
void Span1x1_Var    (void);	// 0x434470  floor/ceiling span, size from g_dwVShift, 1 px/texel, flat
void Span1x1Lit_Var    (void);	// 0x434500  floor/ceiling span, size from g_dwVShift, 1 px/texel, shaded
void Span1x1Lit_S6    (void);	// 0x434590  floor/ceiling span, 1024-pixel texture, 1 px/texel, shaded, reg loop
void Span1x1Lit_S7    (void);	// 0x434600  floor/ceiling span, 512-pixel texture, 1 px/texel, shaded, reg loop
void Span1x1Lit_S8    (void);	// 0x434670  floor/ceiling span, 256-pixel texture, 1 px/texel, shaded, reg loop
void Span1x1Lit_S15    (void);	// 0x4346E0  floor/ceiling span, 2-pixel texture, 1 px/texel, shaded, reg loop
void Span1x1_S6    (void);	// 0x434750  floor/ceiling span, 1024-pixel texture, 1 px/texel, flat, reg loop
void Span1x1_S7    (void);	// 0x4347B0  floor/ceiling span, 512-pixel texture, 1 px/texel, flat, reg loop
void Span1x1_S8    (void);	// 0x434810  floor/ceiling span, 256-pixel texture, 1 px/texel, flat, reg loop
void Span1x1_S9    (void);	// 0x434870  floor/ceiling span, 128-pixel texture, 1 px/texel, flat, reg loop
void Span1x1_S10    (void);	// 0x4348D0  floor/ceiling span, 64-pixel texture, 1 px/texel, flat, reg loop
void Span1x1_S11    (void);	// 0x434930  floor/ceiling span, 32-pixel texture, 1 px/texel, flat, reg loop
void Span1x1_S12    (void);	// 0x434990  floor/ceiling span, 16-pixel texture, 1 px/texel, flat, reg loop
void Span1x1_S13    (void);	// 0x4349F0  floor/ceiling span, 8-pixel texture, 1 px/texel, flat, reg loop
void Span1x1_S14    (void);	// 0x434A50  floor/ceiling span, 4-pixel texture, 1 px/texel, flat, reg loop
void Span1x1_S15    (void);	// 0x434AB0  floor/ceiling span, 2-pixel texture, 1 px/texel, flat, reg loop
void Span2x1_Var    (void);	// 0x434B10  floor/ceiling span, size from g_dwVShift, 2 px/texel, flat
void Span2x1Lit_Var    (void);	// 0x434BB0  floor/ceiling span, size from g_dwVShift, 2 px/texel, shaded
void Span2x1Lit_S6    (void);	// 0x434C60  floor/ceiling span, 1024-pixel texture, 2 px/texel, shaded, reg loop
void Span2x1Lit_S7    (void);	// 0x434CF0  floor/ceiling span, 512-pixel texture, 2 px/texel, shaded, reg loop
void Span2x1Lit_S8    (void);	// 0x434D80  floor/ceiling span, 256-pixel texture, 2 px/texel, shaded, reg loop
void Span2x1Lit_S15    (void);	// 0x434E10  floor/ceiling span, 2-pixel texture, 2 px/texel, shaded, reg loop
void Span2x1_S6    (void);	// 0x434EA0  floor/ceiling span, 1024-pixel texture, 2 px/texel, flat, reg loop
void Span2x1_S7    (void);	// 0x434F30  floor/ceiling span, 512-pixel texture, 2 px/texel, flat, reg loop
void Span2x1_S8    (void);	// 0x434FC0  floor/ceiling span, 256-pixel texture, 2 px/texel, flat, reg loop
void Span2x1_S9    (void);	// 0x435050  floor/ceiling span, 128-pixel texture, 2 px/texel, flat, reg loop
void Span2x1_S10    (void);	// 0x4350E0  floor/ceiling span, 64-pixel texture, 2 px/texel, flat, reg loop
void Span2x1_S11    (void);	// 0x435170  floor/ceiling span, 32-pixel texture, 2 px/texel, flat, reg loop
void Span2x1_S12    (void);	// 0x435200  floor/ceiling span, 16-pixel texture, 2 px/texel, flat, reg loop
void Span2x1_S13    (void);	// 0x435290  floor/ceiling span, 8-pixel texture, 2 px/texel, flat, reg loop
void Span2x1_S14    (void);	// 0x435320  floor/ceiling span, 4-pixel texture, 2 px/texel, flat, reg loop
void Span2x1_S15    (void);	// 0x4353B0  floor/ceiling span, 2-pixel texture, 2 px/texel, flat, reg loop
void Span4x1_Var    (void);	// 0x435440  floor/ceiling span, size from g_dwVShift, 4 px/texel, flat
void Span4x1Lit_Var    (void);	// 0x4354F0  floor/ceiling span, size from g_dwVShift, 4 px/texel, shaded
void Span4x1Lit_S6    (void);	// 0x4355B0  floor/ceiling span, 1024-pixel texture, 4 px/texel, shaded, reg loop
void Span4x1Lit_S7    (void);	// 0x435650  floor/ceiling span, 512-pixel texture, 4 px/texel, shaded, reg loop
void Span4x1Lit_S15    (void);	// 0x4356F0  floor/ceiling span, 2-pixel texture, 4 px/texel, shaded, reg loop
void Span4x1_S6    (void);	// 0x435790  floor/ceiling span, 1024-pixel texture, 4 px/texel, flat, reg loop
void Span4x1_S7    (void);	// 0x435820  floor/ceiling span, 512-pixel texture, 4 px/texel, flat, reg loop
void Span4x1_S8    (void);	// 0x4358B0  floor/ceiling span, 256-pixel texture, 4 px/texel, flat, reg loop
void Span4x1_S9    (void);	// 0x435940  floor/ceiling span, 128-pixel texture, 4 px/texel, flat, reg loop
void Span4x1_S10    (void);	// 0x4359D0  floor/ceiling span, 64-pixel texture, 4 px/texel, flat, reg loop
void Span4x1_S11    (void);	// 0x435A60  floor/ceiling span, 32-pixel texture, 4 px/texel, flat, reg loop
void Span4x1_S12    (void);	// 0x435AF0  floor/ceiling span, 16-pixel texture, 4 px/texel, flat, reg loop
void Span4x1_S13    (void);	// 0x435B80  floor/ceiling span, 8-pixel texture, 4 px/texel, flat, reg loop
void Span4x1_S14    (void);	// 0x435C10  floor/ceiling span, 4-pixel texture, 4 px/texel, flat, reg loop
void Span4x1_S15    (void);	// 0x435CA0  floor/ceiling span, 2-pixel texture, 4 px/texel, flat, reg loop

// group C: the "zoomed" drawers, hover.exe 0x435D30..0x437EF0
//
// Three blocks, each one texel replicated over a screen block:
//   Span2x2  2 pixels wide (word store) x 2 rows
//   Span4x2  4 pixels wide (dword store) x 2 rows
//   Span4x4  4 pixels wide (dword store) x 4 rows
// "Lit" variants push the texel through the shade/remap table at
// g_dwShadeTable; "_Var" variants take the v shift from g_dwVShift at
// run time, the "_Sn" variants have it baked in (texture width 2^(16-n)).
// Note that the "Lit" families are incomplete in the original: only the
// sizes listed below exist (2x2Lit has 6/7/9/15, 4x2Lit 6/7/8/9/15,
// 4x4Lit 6..10/15), while the direct families run 6..15 without a gap.

void Span2x2_Var(void);           // hover.exe:0x435D30  2 px x 2 rows, run-time v shift, direct texel
void Span2x2Lit_Var(void);        // hover.exe:0x435DF0  2 px x 2 rows, run-time v shift, shade/remap table
void Span2x2Lit_S6(void);         // hover.exe:0x435EB0  2 px x 2 rows, v shift 6, shade/remap table
void Span2x2Lit_S7(void);         // hover.exe:0x435F50  2 px x 2 rows, v shift 7, shade/remap table
void Span2x2Lit_S9(void);         // hover.exe:0x435FF0  2 px x 2 rows, v shift 9, shade/remap table
void Span2x2Lit_S15(void);        // hover.exe:0x436090  2 px x 2 rows, v shift 15, shade/remap table
void Span2x2_S6(void);            // hover.exe:0x436130  2 px x 2 rows, v shift 6, direct texel
void Span2x2_S7(void);            // hover.exe:0x4361C0  2 px x 2 rows, v shift 7, direct texel
void Span2x2_S8(void);            // hover.exe:0x436250  2 px x 2 rows, v shift 8, direct texel
void Span2x2_S9(void);            // hover.exe:0x4362E0  2 px x 2 rows, v shift 9, direct texel
void Span2x2_S10(void);           // hover.exe:0x436370  2 px x 2 rows, v shift 10, direct texel
void Span2x2_S11(void);           // hover.exe:0x436400  2 px x 2 rows, v shift 11, direct texel
void Span2x2_S12(void);           // hover.exe:0x436490  2 px x 2 rows, v shift 12, direct texel
void Span2x2_S13(void);           // hover.exe:0x436520  2 px x 2 rows, v shift 13, direct texel
void Span2x2_S14(void);           // hover.exe:0x4365B0  2 px x 2 rows, v shift 14, direct texel
void Span2x2_S15(void);           // hover.exe:0x436640  2 px x 2 rows, v shift 15, direct texel
void Span4x2_Var(void);           // hover.exe:0x4366D0  4 px x 2 rows, run-time v shift, direct texel
void Span4x2Lit_Var(void);        // hover.exe:0x436790  4 px x 2 rows, run-time v shift, shade/remap table
void Span4x2Lit_S6(void);         // hover.exe:0x436860  4 px x 2 rows, v shift 6, shade/remap table
void Span4x2Lit_S7(void);         // hover.exe:0x436900  4 px x 2 rows, v shift 7, shade/remap table
void Span4x2Lit_S8(void);         // hover.exe:0x4369A0  4 px x 2 rows, v shift 8, shade/remap table
void Span4x2Lit_S9(void);         // hover.exe:0x436A40  4 px x 2 rows, v shift 9, shade/remap table
void Span4x2Lit_S15(void);        // hover.exe:0x436AE0  4 px x 2 rows, v shift 15, shade/remap table
void Span4x2_S6(void);            // hover.exe:0x436B80  4 px x 2 rows, v shift 6, direct texel
void Span4x2_S7(void);            // hover.exe:0x436C20  4 px x 2 rows, v shift 7, direct texel
void Span4x2_S8(void);            // hover.exe:0x436CC0  4 px x 2 rows, v shift 8, direct texel
void Span4x2_S9(void);            // hover.exe:0x436D60  4 px x 2 rows, v shift 9, direct texel
void Span4x2_S10(void);           // hover.exe:0x436E00  4 px x 2 rows, v shift 10, direct texel
void Span4x2_S11(void);           // hover.exe:0x436EA0  4 px x 2 rows, v shift 11, direct texel
void Span4x2_S12(void);           // hover.exe:0x436F40  4 px x 2 rows, v shift 12, direct texel
void Span4x2_S13(void);           // hover.exe:0x436FE0  4 px x 2 rows, v shift 13, direct texel
void Span4x2_S14(void);           // hover.exe:0x437080  4 px x 2 rows, v shift 14, direct texel
void Span4x2_S15(void);           // hover.exe:0x437120  4 px x 2 rows, v shift 15, direct texel
void Span4x4_Var(void);           // hover.exe:0x4371C0  4 px x 4 rows, run-time v shift, direct texel
void Span4x4Lit_Var(void);        // hover.exe:0x4372A0  4 px x 4 rows, run-time v shift, shade/remap table
void Span4x4Lit_S6(void);         // hover.exe:0x4373A0  4 px x 4 rows, v shift 6, shade/remap table
void Span4x4Lit_S7(void);         // hover.exe:0x437460  4 px x 4 rows, v shift 7, shade/remap table
void Span4x4Lit_S8(void);         // hover.exe:0x437520  4 px x 4 rows, v shift 8, shade/remap table
void Span4x4Lit_S9(void);         // hover.exe:0x4375E0  4 px x 4 rows, v shift 9, shade/remap table
void Span4x4Lit_S10(void);        // hover.exe:0x4376A0  4 px x 4 rows, v shift 10, shade/remap table
void Span4x4Lit_S15(void);        // hover.exe:0x437760  4 px x 4 rows, v shift 15, shade/remap table
void Span4x4_S6(void);            // hover.exe:0x437820  4 px x 4 rows, v shift 6, direct texel
void Span4x4_S7(void);            // hover.exe:0x4378D0  4 px x 4 rows, v shift 7, direct texel
void Span4x4_S8(void);            // hover.exe:0x437980  4 px x 4 rows, v shift 8, direct texel
void Span4x4_S9(void);            // hover.exe:0x437A30  4 px x 4 rows, v shift 9, direct texel
void Span4x4_S10(void);           // hover.exe:0x437AE0  4 px x 4 rows, v shift 10, direct texel
void Span4x4_S11(void);           // hover.exe:0x437B90  4 px x 4 rows, v shift 11, direct texel
void Span4x4_S12(void);           // hover.exe:0x437C40  4 px x 4 rows, v shift 12, direct texel
void Span4x4_S13(void);           // hover.exe:0x437CF0  4 px x 4 rows, v shift 13, direct texel
void Span4x4_S14(void);           // hover.exe:0x437DA0  4 px x 4 rows, v shift 14, direct texel
void Span4x4_S15(void);           // hover.exe:0x437E50  4 px x 4 rows, v shift 15, direct texel

#endif // __SPANDRAW_H__
