// spandrawB.cpp : software renderer span drawers, group B
//
// The 70 hand-written span drawers at hover.exe:0x429960..0x435CA0. They are
// the functions CRenderContext::SetMode (texload.cpp) installs into the view
// object's drawer slots m_pfnDrawWall / m_pfnDrawSky / m_pfnDrawFlat and into
// the size-indexed table m_ppfnDrawFlatByShift[6..15]; every one of them is a
// parameterless routine that consumes the renderer globals set up by
// CSpanRasterizer / CRenderContext (see texheap.cpp):
//
//   g_dwSpanCount  span length (pixels left to draw; decremented in place)
//   g_dwSkyU  u position for the flat "run" drawers (16.16)
//   g_dwTexels  texture pixel base
//   g_dwUMask  u mask, ((width - 1) << 16) | 0xFFFF
//   g_dwVStep  v step (16.16)
//   g_dwU  u position (16.16)
//   g_dwV  v position (16.16)
//   g_dwDstRowStride  destination row stride (frame buffer pitch * zoom)
//   g_dwVMask  v mask, (height - 1) << 16
//   g_dwShadeTable  shade/colour-map table for the lit variants
//   g_dwColumnV  column position (16.16) for the column drawers
//   g_dwVShift  log2 selector: 0x10 - log2(texture width)
//   g_dwDst  destination pointer
//   g_dwUStep  u step (16.16)
//
// All 70 are naked (__declspec(naked)) hand-written assembly in the
// original: the callee-saved pushes are interleaved with the first working
// instructions (0x429960 pushes ebp *after* loading esi, 0x42A240 pushes
// ebp only on the taken side of its guard), a scheduling the compiler's
// own prologue never produces, and they use 16-bit push/pop pairs
// (push ax / pop ax at 0x429CC0) to splice a byte into all four lanes of a
// dword.  They are therefore reproduced here as naked functions whose body
// is the original instruction stream, with the renderer globals named.
//
// Texture-size parameter: the two-axis (floor/ceiling) drawers index the
// texture as base + (u >> 16) + ((v & vmask) >> s), so the row stride is
// 2^(16 - s), i.e. s = 6 selects a 1024-pixel-wide texture and s = 15 a
// 2-pixel one - the same value CRenderContext stores in g_dwVShift
// (0x10 - log2(width)), which is why the table is indexed 6..15.

#include "hover.h"
#include "texture.h"
#include "gameobj.h"
#include "texheap.h"
#include "spandraw.h"

/////////////////////////////////////////////////////////////////////////////
// Vertical texture columns (hover.exe:0x429960..0x42A2CD)
//
// One texture axis only: the position g_dwColumnV is stepped by
// g_dwUStep, masked with g_dwVMask and shifted right 16 to give a
// texel offset into g_dwTexels; the destination walks down the frame
// buffer by g_dwDstRowStride per output row.  This is the wall / scaled-object
// column drawer.  The variants differ in three parameters: the horizontal
// pixel replication (byte = 1, word = 2, dword = 4 screen pixels per texel,
// built with shl 8 / shl 0x10 / a 16-bit push-pop), the vertical zoom (1, 2
// or 4 rows per texel, which also pre-scales the span length and the step:
// g_dwSpanCount /= zoom, g_dwUStep <<= log2(zoom)) and whether the
// texel goes through the shade table g_dwShadeTable.

// hover.exe:0x429960 (97 bytes)
// vertical texture column, sar 0x10;
// 1 pixel wide (byte write), shade table g_dwShadeTable.
// FUNCTION: HOVER 0x429960
HOVER_NAKED Column1x1Lit(void)
{
	_asm
	{
		push ebx
		push esi
		push edi
		mov  esi, g_dwSpanCount
		push ebp
		test esi, esi
		jle  L4299BC
		mov  eax, g_dwTexels
		mov  ecx, g_dwDst
		mov  edx, g_dwShadeTable
		mov  edi, g_dwColumnV
	L429985:
		mov  ebp, g_dwVMask
		xor  ebx, ebx
		and  ebp, edi
		add  edi, g_dwUStep
		sar  ebp, 0x10
		mov  g_dwColumnV, edi
		mov  bl, byte ptr [ebp + eax]
		mov  bl, byte ptr [ebx + edx]
		mov  byte ptr [ecx], bl
		add  ecx, g_dwDstRowStride
		dec  esi
		mov  g_dwDst, ecx
		mov  g_dwSpanCount, esi
		jne  L429985
	L4299BC:
		pop  ebp
		pop  edi
		pop  esi
		pop  ebx
		ret
	}
}

// hover.exe:0x4299D0 (89 bytes)
// vertical texture column, sar 0x10;
// 1 pixel wide (byte write), no shade table.
// FUNCTION: HOVER 0x4299d0
HOVER_NAKED Column1x1(void)
{
	_asm
	{
		push ebx
		mov  edx, g_dwSpanCount
		push esi
		test edx, edx
		push edi
		push ebp
		jle  L429A24
		mov  eax, g_dwTexels
		mov  ecx, g_dwDst
		mov  esi, g_dwColumnV
		mov  edi, g_dwDstRowStride
		mov  ebp, g_dwUStep
	L4299FB:
		mov  ebx, g_dwVMask
		and  ebx, esi
		add  esi, ebp
		sar  ebx, 0x10
		mov  g_dwColumnV, esi
		mov  bl, byte ptr [ebx + eax]
		mov  byte ptr [ecx], bl
		add  ecx, edi
		dec  edx
		mov  g_dwDst, ecx
		mov  g_dwSpanCount, edx
		jne  L4299FB
	L429A24:
		pop  ebp
		pop  edi
		pop  esi
		pop  ebx
		ret
	}
}

// hover.exe:0x429A30 (78 bytes) vertical texture column, sar 0x10; 1 pixel
// wide (byte write), no shade table (g_dwShadeTable loaded but unused),
// register-resident loop (leaves g_dwDst/40/14 stale).
// FUNCTION: HOVER 0x429a30
HOVER_NAKED Column1x1Reg(void)
{
	_asm
	{
		push ebx
		push esi
		push edi
		push ebp
		mov  ebx, g_dwSpanCount
		mov  edi, g_dwDst
		mov  esi, g_dwTexels
		mov  edx, g_dwColumnV
		mov  ebp, g_dwShadeTable
		sub  edi, g_dwDstRowStride
		sub  eax, eax
	L429A5A:
		mov  ecx, edx
		add  edx, g_dwUStep
		shr  ecx, 0x10
		add  edi, g_dwDstRowStride
		and  edx, g_dwVMask
		mov  al, byte ptr [esi + ecx]
		dec  ebx
		mov  byte ptr [edi], al
		jg   L429A5A
		pop  ebp
		pop  edi
		pop  esi
		pop  ebx
		ret
	}
}

// hover.exe:0x429A80 (103 bytes)
// vertical texture column, sar 0x10;
// 2 pixels wide (word write), no shade table.
// FUNCTION: HOVER 0x429a80
HOVER_NAKED Column2x1(void)
{
	_asm
	{
		push esi
		mov  eax, g_dwDstRowStride
		sub  g_dwDst, eax
		cmp  g_dwSpanCount, 0
		jle  L429AE5
		xor  edx, edx
	L429A97:
		mov  eax, g_dwVMask
		mov  esi, g_dwTexels
		and  eax, g_dwColumnV
		xor  ecx, ecx
		sar  eax, 0x10
		mov  cl, byte ptr [eax + esi]
		mov  esi, g_dwDstRowStride
		mov  eax, ecx
		add  g_dwDst, esi
		shl  eax, 8
		or   ecx, eax
		mov  eax, g_dwDst
		mov  word ptr [eax], cx
		mov  ecx, g_dwUStep
		add  g_dwColumnV, ecx
		dec  g_dwSpanCount
		cmp  g_dwSpanCount, edx
		jg   L429A97
	L429AE5:
		pop  esi
		ret
	}
}

// hover.exe:0x429AF0 (113 bytes)
// vertical texture column, sar 0x10;
// 2 pixels wide (word write), shade table g_dwShadeTable.
// FUNCTION: HOVER 0x429af0
HOVER_NAKED Column2x1Lit(void)
{
	_asm
	{
		push ebx
		mov  eax, g_dwDstRowStride
		sub  g_dwDst, eax
		cmp  g_dwSpanCount, 0
		jle  L429B5F
		xor  edx, edx
	L429B07:
		mov  eax, g_dwVMask
		mov  ecx, g_dwTexels
		and  eax, g_dwColumnV
		xor  ebx, ebx
		sar  eax, 0x10
		mov  bl, byte ptr [eax + ecx]
		mov  eax, g_dwShadeTable
		xor  ecx, ecx
		mov  cl, byte ptr [ebx + eax]
		mov  ebx, g_dwDstRowStride
		mov  eax, ecx
		add  g_dwDst, ebx
		shl  eax, 8
		or   ecx, eax
		mov  eax, g_dwDst
		mov  word ptr [eax], cx
		mov  ecx, g_dwUStep
		add  g_dwColumnV, ecx
		dec  g_dwSpanCount
		cmp  g_dwSpanCount, edx
		jg   L429B07
	L429B5F:
		pop  ebx
		ret
	}
}

// hover.exe:0x429B70 (81 bytes) vertical texture column, sar 0x10; 2 pixels
// wide (word write), no shade table (g_dwShadeTable loaded but unused),
// register-resident loop (leaves g_dwDst/40/14 stale).
// FUNCTION: HOVER 0x429b70
HOVER_NAKED Column2x1Reg(void)
{
	_asm
	{
		push ebx
		push esi
		push edi
		push ebp
		mov  ebx, g_dwSpanCount
		mov  edi, g_dwDst
		mov  esi, g_dwTexels
		mov  edx, g_dwColumnV
		mov  ebp, g_dwShadeTable
		sub  edi, g_dwDstRowStride
		sub  eax, eax
	L429B9A:
		mov  ecx, edx
		add  edx, g_dwUStep
		shr  ecx, 0x10
		add  edi, g_dwDstRowStride
		and  edx, g_dwVMask
		mov  al, byte ptr [esi + ecx]
		dec  ebx
		mov  ah, al
		mov  word ptr [edi], ax
		jg   L429B9A
		pop  ebp
		pop  edi
		pop  esi
		pop  ebx
		ret
	}
}

// hover.exe:0x429BD0 (109 bytes)
// vertical texture column, sar 0x10;
// 4 pixels wide (dword write), no shade table.
// FUNCTION: HOVER 0x429bd0
HOVER_NAKED Column4x1(void)
{
	_asm
	{
		push esi
		mov  eax, g_dwDstRowStride
		sub  g_dwDst, eax
		cmp  g_dwSpanCount, 0
		jle  L429C3B
		xor  edx, edx
	L429BE7:
		mov  eax, g_dwVMask
		mov  esi, g_dwTexels
		and  eax, g_dwColumnV
		xor  ecx, ecx
		sar  eax, 0x10
		mov  cl, byte ptr [eax + esi]
		mov  esi, g_dwDstRowStride
		mov  eax, ecx
		add  g_dwDst, esi
		shl  eax, 8
		or   ecx, eax
		mov  eax, ecx
		shl  eax, 0x10
		or   ecx, eax
		mov  eax, g_dwDst
		mov  dword ptr [eax], ecx
		mov  ecx, g_dwUStep
		add  g_dwColumnV, ecx
		dec  g_dwSpanCount
		cmp  g_dwSpanCount, edx
		jg   L429BE7
	L429C3B:
		pop  esi
		ret
	}
}

// hover.exe:0x429C40 (119 bytes)
// vertical texture column, sar 0x10;
// 4 pixels wide (dword write), shade table g_dwShadeTable.
// FUNCTION: HOVER 0x429c40
HOVER_NAKED Column4x1Lit(void)
{
	_asm
	{
		push ebx
		mov  eax, g_dwDstRowStride
		sub  g_dwDst, eax
		cmp  g_dwSpanCount, 0
		jle  L429CB5
		xor  edx, edx
	L429C57:
		mov  eax, g_dwVMask
		mov  ecx, g_dwTexels
		and  eax, g_dwColumnV
		xor  ebx, ebx
		sar  eax, 0x10
		mov  bl, byte ptr [eax + ecx]
		mov  eax, g_dwShadeTable
		xor  ecx, ecx
		mov  cl, byte ptr [ebx + eax]
		mov  ebx, g_dwDstRowStride
		mov  eax, ecx
		add  g_dwDst, ebx
		shl  eax, 8
		or   ecx, eax
		mov  eax, ecx
		shl  eax, 0x10
		or   ecx, eax
		mov  eax, g_dwDst
		mov  dword ptr [eax], ecx
		mov  ecx, g_dwUStep
		add  g_dwColumnV, ecx
		dec  g_dwSpanCount
		cmp  g_dwSpanCount, edx
		jg   L429C57
	L429CB5:
		pop  ebx
		ret
	}
}

// hover.exe:0x429CC0 (87 bytes) vertical texture column, sar 0x10; 4 pixels
// wide (dword write), no shade table (g_dwShadeTable loaded but unused),
// register-resident loop (leaves g_dwDst/40/14 stale).
// FUNCTION: HOVER 0x429cc0
HOVER_NAKED Column4x1Reg(void)
{
	_asm
	{
		push ebx
		push esi
		push edi
		push ebp
		mov  ebx, g_dwSpanCount
		mov  edi, g_dwDst
		mov  esi, g_dwTexels
		mov  edx, g_dwColumnV
		mov  ebp, g_dwShadeTable
		sub  edi, g_dwDstRowStride
		sub  eax, eax
	L429CEA:
		mov  ecx, edx
		add  edx, g_dwUStep
		shr  ecx, 0x10
		add  edi, g_dwDstRowStride
		and  edx, g_dwVMask
		mov  al, byte ptr [esi + ecx]
		mov  ah, al
		push ax
		shl  eax, 0x10
		pop  ax
		dec  ebx
		mov  dword ptr [edi], eax
		jg   L429CEA
		pop  ebp
		pop  edi
		pop  esi
		pop  ebx
		ret
	}
}

// hover.exe:0x429D20 (146 bytes)
// vertical texture column, sar 0x10;
// 2 pixels wide (word write), 2 rows per texel (vertical zoom 2), no shade table.
// FUNCTION: HOVER 0x429d20
HOVER_NAKED Column2x2(void)
{
	_asm
	{
		push esi
		mov  eax, g_dwSpanCount
		cdq
		shl  g_dwUStep, 1
		sub  eax, edx
		sar  eax, 1
		mov  g_dwSpanCount, eax
		mov  eax, g_dwDstRowStride
		sub  g_dwDst, eax
		cmp  g_dwSpanCount, 0
		jle  L429DB0
		xor  edx, edx
	L429D4E:
		mov  eax, g_dwVMask
		mov  esi, g_dwTexels
		and  eax, g_dwColumnV
		xor  ecx, ecx
		sar  eax, 0x10
		mov  cl, byte ptr [eax + esi]
		mov  esi, g_dwDstRowStride
		mov  eax, ecx
		add  g_dwDst, esi
		shl  eax, 8
		or   ecx, eax
		mov  eax, g_dwDst
		mov  word ptr [eax], cx
		mov  esi, g_dwDstRowStride
		add  g_dwDst, esi
		mov  eax, g_dwDst
		mov  word ptr [eax], cx
		mov  ecx, g_dwUStep
		add  g_dwColumnV, ecx
		dec  g_dwSpanCount
		cmp  g_dwSpanCount, edx
		jg   L429D4E
	L429DB0:
		pop  esi
		ret
	}
}

// hover.exe:0x429DC0 (156 bytes) vertical texture column, sar 0x10; 2 pixels
// wide (word write), 2 rows per texel (vertical zoom 2), shade table
// g_dwShadeTable.
// FUNCTION: HOVER 0x429dc0
HOVER_NAKED Column2x2Lit(void)
{
	_asm
	{
		push ebx
		mov  eax, g_dwSpanCount
		cdq
		shl  g_dwUStep, 1
		sub  eax, edx
		sar  eax, 1
		mov  g_dwSpanCount, eax
		mov  eax, g_dwDstRowStride
		sub  g_dwDst, eax
		cmp  g_dwSpanCount, 0
		jle  L429E5A
		xor  edx, edx
	L429DEE:
		mov  eax, g_dwVMask
		mov  ecx, g_dwTexels
		and  eax, g_dwColumnV
		xor  ebx, ebx
		sar  eax, 0x10
		mov  bl, byte ptr [eax + ecx]
		mov  eax, g_dwShadeTable
		xor  ecx, ecx
		mov  cl, byte ptr [ebx + eax]
		mov  ebx, g_dwDstRowStride
		mov  eax, ecx
		add  g_dwDst, ebx
		shl  eax, 8
		or   ecx, eax
		mov  eax, g_dwDst
		mov  word ptr [eax], cx
		mov  ebx, g_dwDstRowStride
		add  g_dwDst, ebx
		mov  eax, g_dwDst
		mov  word ptr [eax], cx
		mov  ecx, g_dwUStep
		add  g_dwColumnV, ecx
		dec  g_dwSpanCount
		cmp  g_dwSpanCount, edx
		jg   L429DEE
	L429E5A:
		pop  ebx
		ret
	}
}

// hover.exe:0x429E60 (118 bytes) vertical texture column, sar 0x10; 2 pixels
// wide (word write), 2 rows per texel (vertical zoom 2), no shade table
// (g_dwShadeTable loaded but unused), register-resident loop (leaves
// g_dwDst/40/14 stale).
// FUNCTION: HOVER 0x429e60
HOVER_NAKED Column2x2Reg(void)
{
	_asm
	{
		push ebx
		mov  eax, g_dwSpanCount
		cdq
		push esi
		sub  eax, edx
		sar  eax, 1
		push edi
		shl  g_dwUStep, 1
		mov  g_dwSpanCount, eax
		test eax, eax
		jle  L429ED2
		push ebp
		mov  ebx, g_dwUStep
		mov  edi, g_dwDst
		mov  esi, g_dwTexels
		mov  edx, g_dwColumnV
		mov  ebp, g_dwShadeTable
		sub  edi, g_dwDstRowStride
		sub  eax, eax
	L429EA5:
		mov  ecx, edx
		add  edx, ebx
		shr  ecx, 0x10
		add  edi, g_dwDstRowStride
		and  edx, g_dwVMask
		mov  al, byte ptr [esi + ecx]
		mov  ah, al
		mov  word ptr [edi], ax
		add  edi, g_dwDstRowStride
		dec  g_dwSpanCount
		mov  word ptr [edi], ax
		jg   L429EA5
		pop  ebp
	L429ED2:
		pop  edi
		pop  esi
		pop  ebx
		ret
	}
}

// hover.exe:0x429EE0 (151 bytes)
// vertical texture column, sar 0x10;
// 4 pixels wide (dword write), 2 rows per texel (vertical zoom 2), no shade table.
// FUNCTION: HOVER 0x429ee0
HOVER_NAKED Column4x2(void)
{
	_asm
	{
		push esi
		mov  eax, g_dwSpanCount
		cdq
		shl  g_dwUStep, 1
		sub  eax, edx
		sar  eax, 1
		mov  g_dwSpanCount, eax
		mov  eax, g_dwDstRowStride
		sub  g_dwDst, eax
		cmp  g_dwSpanCount, 0
		jle  L429F75
		xor  edx, edx
	L429F0E:
		mov  eax, g_dwVMask
		mov  esi, g_dwTexels
		and  eax, g_dwColumnV
		xor  ecx, ecx
		sar  eax, 0x10
		mov  cl, byte ptr [eax + esi]
		mov  esi, g_dwDstRowStride
		mov  eax, ecx
		add  g_dwDst, esi
		shl  eax, 8
		or   ecx, eax
		mov  eax, ecx
		shl  eax, 0x10
		or   ecx, eax
		mov  eax, g_dwDst
		mov  dword ptr [eax], ecx
		mov  esi, g_dwDstRowStride
		add  g_dwDst, esi
		mov  eax, g_dwDst
		mov  dword ptr [eax], ecx
		mov  ecx, g_dwUStep
		add  g_dwColumnV, ecx
		dec  g_dwSpanCount
		cmp  g_dwSpanCount, edx
		jg   L429F0E
	L429F75:
		pop  esi
		ret
	}
}

// hover.exe:0x429F80 (161 bytes) vertical texture column, sar 0x10; 4 pixels
// wide (dword write), 2 rows per texel (vertical zoom 2), shade table
// g_dwShadeTable.
// FUNCTION: HOVER 0x429f80
HOVER_NAKED Column4x2Lit(void)
{
	_asm
	{
		push ebx
		mov  eax, g_dwSpanCount
		cdq
		shl  g_dwUStep, 1
		sub  eax, edx
		sar  eax, 1
		mov  g_dwSpanCount, eax
		mov  eax, g_dwDstRowStride
		sub  g_dwDst, eax
		cmp  g_dwSpanCount, 0
		jle  L42A01F
		xor  edx, edx
	L429FAE:
		mov  eax, g_dwVMask
		mov  ecx, g_dwTexels
		and  eax, g_dwColumnV
		xor  ebx, ebx
		sar  eax, 0x10
		mov  bl, byte ptr [eax + ecx]
		mov  eax, g_dwShadeTable
		xor  ecx, ecx
		mov  cl, byte ptr [ebx + eax]
		mov  ebx, g_dwDstRowStride
		mov  eax, ecx
		add  g_dwDst, ebx
		shl  eax, 8
		or   ecx, eax
		mov  eax, ecx
		shl  eax, 0x10
		or   ecx, eax
		mov  eax, g_dwDst
		mov  dword ptr [eax], ecx
		mov  ebx, g_dwDstRowStride
		add  g_dwDst, ebx
		mov  eax, g_dwDst
		mov  dword ptr [eax], ecx
		mov  ecx, g_dwUStep
		add  g_dwColumnV, ecx
		dec  g_dwSpanCount
		cmp  g_dwSpanCount, edx
		jg   L429FAE
	L42A01F:
		pop  ebx
		ret
	}
}

// hover.exe:0x42A030 (123 bytes) vertical texture column, sar 0x10; 4 pixels
// wide (dword write), 2 rows per texel (vertical zoom 2), no shade table
// (g_dwShadeTable loaded but unused), register-resident loop (leaves
// g_dwDst/40/14 stale).
// FUNCTION: HOVER 0x42a030
HOVER_NAKED Column4x2Reg(void)
{
	_asm
	{
		push ebx
		mov  eax, g_dwSpanCount
		cdq
		push esi
		sub  eax, edx
		sar  eax, 1
		push edi
		shl  g_dwUStep, 1
		mov  g_dwSpanCount, eax
		test eax, eax
		jle  L42A0A7
		push ebp
		mov  ebx, g_dwUStep
		mov  edi, g_dwDst
		mov  esi, g_dwTexels
		mov  edx, g_dwColumnV
		mov  ebp, g_dwShadeTable
		sub  edi, g_dwDstRowStride
		sub  eax, eax
	L42A075:
		mov  ecx, edx
		add  edx, ebx
		shr  ecx, 0x10
		add  edi, g_dwDstRowStride
		and  edx, g_dwVMask
		mov  al, byte ptr [esi + ecx]
		mov  ah, al
		push ax
		shl  eax, 0x10
		pop  ax
		mov  dword ptr [edi], eax
		add  edi, g_dwDstRowStride
		dec  g_dwSpanCount
		mov  dword ptr [edi], eax
		jg   L42A075
		pop  ebp
	L42A0A7:
		pop  edi
		pop  esi
		pop  ebx
		ret
	}
}

// hover.exe:0x42A0B0 (183 bytes)
// vertical texture column, sar 0x10;
// 4 pixels wide (dword write), 4 rows per texel (vertical zoom 4), no shade table.
// FUNCTION: HOVER 0x42a0b0
HOVER_NAKED Column4x4(void)
{
	_asm
	{
		push ebx
		mov  eax, g_dwSpanCount
		cdq
		push esi
		and  edx, 3
		push edi
		add  eax, edx
		sar  eax, 2
		push ebp
		shl  g_dwUStep, 2
		mov  g_dwSpanCount, eax
		mov  eax, g_dwDstRowStride
		lea  edx, [eax*4]
		lea  esi, [eax*2]
		cmp  g_dwSpanCount, 0
		lea  ecx, [esi + eax]
		jle  L42A162
		xor  edi, edi
	L42A0EF:
		mov  ebp, g_dwVMask
		mov  eax, g_dwTexels
		and  ebp, g_dwColumnV
		xor  ebx, ebx
		sar  ebp, 0x10
		mov  bl, byte ptr [ebp + eax]
		mov  ebp, g_dwDst
		mov  eax, ebx
		shl  eax, 8
		or   ebx, eax
		mov  eax, ebx
		shl  eax, 0x10
		or   ebx, eax
		mov  dword ptr [ebp], ebx
		mov  ebp, g_dwDst
		mov  eax, g_dwDstRowStride
		mov  dword ptr [ebp + eax], ebx
		mov  ebp, g_dwDst
		mov  dword ptr [ebp + esi], ebx
		mov  ebp, g_dwDst
		mov  dword ptr [ebp + ecx], ebx
		mov  eax, g_dwUStep
		add  g_dwDst, edx
		add  g_dwColumnV, eax
		dec  g_dwSpanCount
		cmp  g_dwSpanCount, edi
		jg   L42A0EF
	L42A162:
		pop  ebp
		pop  edi
		pop  esi
		pop  ebx
		ret
	}
}

// hover.exe:0x42A170 (198 bytes) vertical texture column, sar 0x10; 4 pixels
// wide (dword write), 4 rows per texel (vertical zoom 4), shade table
// g_dwShadeTable.
// FUNCTION: HOVER 0x42a170
HOVER_NAKED Column4x4Lit(void)
{
	_asm
	{
		push ebx
		mov  eax, g_dwSpanCount
		cdq
		push esi
		and  edx, 3
		push edi
		add  eax, edx
		sar  eax, 2
		push ebp
		mov  g_dwSpanCount, eax
		mov  eax, g_dwDstRowStride
		shl  g_dwUStep, 2
		lea  edx, [eax*4]
		lea  esi, [eax*2]
		add  eax, esi
		cmp  g_dwSpanCount, 0
		jle  L42A231
		xor  edi, edi
	L42A1B2:
		mov  ebp, g_dwVMask
		mov  ecx, g_dwTexels
		and  ebp, g_dwColumnV
		xor  ebx, ebx
		sar  ebp, 0x10
		mov  bl, byte ptr [ebp + ecx]
		mov  ebp, g_dwShadeTable
		xor  ecx, ecx
		mov  cl, byte ptr [ebx + ebp]
		mov  ebp, g_dwDst
		mov  ebx, ecx
		shl  ebx, 8
		or   ecx, ebx
		mov  ebx, ecx
		shl  ebx, 0x10
		or   ecx, ebx
		mov  dword ptr [ebp], ecx
		mov  ebp, g_dwDst
		mov  ebx, g_dwDstRowStride
		mov  dword ptr [ebp + ebx], ecx
		mov  ebp, g_dwDst
		mov  dword ptr [esi + ebp], ecx
		mov  ebp, g_dwDst
		mov  dword ptr [eax + ebp], ecx
		mov  ecx, g_dwUStep
		add  g_dwDst, edx
		add  g_dwColumnV, ecx
		dec  g_dwSpanCount
		cmp  g_dwSpanCount, edi
		jg   L42A1B2
	L42A231:
		pop  ebp
		pop  edi
		pop  esi
		pop  ebx
		ret
	}
}

// hover.exe:0x42A240 (142 bytes) vertical texture column, sar 0x10; 4 pixels
// wide (dword write), 4 rows per texel (vertical zoom 4), no shade table
// (g_dwShadeTable loaded but unused), register-resident loop (leaves
// g_dwDst/40/14 stale).
// FUNCTION: HOVER 0x42a240
HOVER_NAKED Column4x4Reg(void)
{
	_asm
	{
		push ebx
		mov  eax, g_dwSpanCount
		cdq
		push esi
		and  edx, 3
		push edi
		add  eax, edx
		shl  g_dwUStep, 2
		sar  eax, 2
		mov  g_dwSpanCount, eax
		test eax, eax
		jle  L42A2CA
		push ebp
		mov  ebx, g_dwUStep
		mov  edi, g_dwDst
		mov  esi, g_dwTexels
		mov  edx, g_dwColumnV
		mov  ebp, g_dwShadeTable
		sub  edi, g_dwDstRowStride
		sub  eax, eax
	L42A288:
		mov  ecx, edx
		add  edx, ebx
		shr  ecx, 0x10
		add  edi, g_dwDstRowStride
		and  edx, g_dwVMask
		mov  al, byte ptr [esi + ecx]
		mov  ah, al
		push ax
		shl  eax, 0x10
		pop  ax
		mov  dword ptr [edi], eax
		add  edi, g_dwDstRowStride
		mov  dword ptr [edi], eax
		add  edi, g_dwDstRowStride
		mov  dword ptr [edi], eax
		add  edi, g_dwDstRowStride
		dec  g_dwSpanCount
		mov  dword ptr [edi], eax
		jg   L42A288
		pop  ebp
	L42A2CA:
		pop  edi
		pop  esi
		pop  ebx
		ret
	}
}

/////////////////////////////////////////////////////////////////////////////
// Flat horizontal runs (hover.exe:0x42A2D0..0x42A4F6)
//
// The "sky"/flat-run drawers: a single u position g_dwSkyU stepped by
// g_dwUStep and masked with g_dwUMask, one texel replicated across
// 2 or 4 horizontal pixels, the destination advancing by that many bytes
// (esi/edx hold the constant 2 or 4).  The 2-row variants write the same
// word/dword again one row down (+ g_dwDstRowStride).

// hover.exe:0x42A2D0 (83 bytes)
// horizontal flat-shaded run (u only, g_dwSkyU), no shift;
// 2 pixels wide (word write), no shade table.
// FUNCTION: HOVER 0x42a2d0
HOVER_NAKED Sky2x1(void)
{
	_asm
	{
		cmp  g_dwSpanCount, 0
		push esi
		je   L42A321
		mov  esi, 2
	L42A2DF:
		mov  eax, g_dwSkyU
		mov  edx, g_dwTexels
		sar  eax, 0x10
		xor  ecx, ecx
		and  eax, g_dwUMask
		mov  cl, byte ptr [eax + edx]
		mov  edx, g_dwDst
		mov  eax, ecx
		shl  eax, 8
		or   ecx, eax
		mov  word ptr [edx], cx
		mov  eax, g_dwUStep
		add  g_dwDst, esi
		add  g_dwSkyU, eax
		dec  g_dwSpanCount
		jne  L42A2DF
	L42A321:
		pop  esi
		ret
	}
}

// hover.exe:0x42A330 (89 bytes)
// horizontal flat-shaded run (u only, g_dwSkyU), no shift;
// 4 pixels wide (dword write), no shade table.
// FUNCTION: HOVER 0x42a330
HOVER_NAKED Sky4x1(void)
{
	_asm
	{
		cmp  g_dwSpanCount, 0
		push esi
		je   L42A387
		mov  edx, 4
	L42A33F:
		mov  eax, g_dwSkyU
		mov  esi, g_dwTexels
		sar  eax, 0x10
		xor  ecx, ecx
		and  eax, g_dwUMask
		mov  cl, byte ptr [eax + esi]
		mov  esi, g_dwDst
		mov  eax, ecx
		shl  eax, 8
		or   ecx, eax
		mov  eax, ecx
		shl  eax, 0x10
		or   ecx, eax
		mov  dword ptr [esi], ecx
		mov  eax, g_dwUStep
		add  g_dwDst, edx
		add  g_dwSkyU, eax
		dec  g_dwSpanCount
		jne  L42A33F
	L42A387:
		pop  esi
		ret
	}
}

// hover.exe:0x42A390 (98 bytes)
// horizontal flat-shaded run (u only, g_dwSkyU), no shift;
// 2 pixels wide (word write), 2 rows, no shade table.
// FUNCTION: HOVER 0x42a390
HOVER_NAKED Sky2x2(void)
{
	_asm
	{
		cmp  g_dwSpanCount, 0
		push esi
		je   L42A3F0
		mov  edx, 2
	L42A39F:
		mov  eax, g_dwSkyU
		mov  esi, g_dwTexels
		sar  eax, 0x10
		xor  ecx, ecx
		and  eax, g_dwUMask
		mov  cl, byte ptr [eax + esi]
		mov  eax, ecx
		shl  eax, 8
		or   ecx, eax
		mov  eax, g_dwDst
		mov  word ptr [eax], cx
		mov  eax, g_dwDst
		mov  esi, g_dwDstRowStride
		mov  word ptr [eax + esi], cx
		mov  ecx, g_dwUStep
		add  g_dwSkyU, ecx
		add  g_dwDst, edx
		dec  g_dwSpanCount
		jne  L42A39F
	L42A3F0:
		pop  esi
		ret
	}
}

// hover.exe:0x42A400 (104 bytes)
// horizontal flat-shaded run (u only, g_dwSkyU), no shift;
// 4 pixels wide (dword write), 2 rows, no shade table.
// FUNCTION: HOVER 0x42a400
HOVER_NAKED Sky4x2(void)
{
	_asm
	{
		cmp  g_dwSpanCount, 0
		push esi
		je   L42A466
		mov  edx, 4
	L42A40F:
		mov  eax, g_dwSkyU
		mov  esi, g_dwTexels
		sar  eax, 0x10
		xor  ecx, ecx
		and  eax, g_dwUMask
		mov  cl, byte ptr [eax + esi]
		mov  esi, g_dwDst
		mov  eax, ecx
		shl  eax, 8
		or   ecx, eax
		mov  eax, ecx
		shl  eax, 0x10
		or   ecx, eax
		mov  dword ptr [esi], ecx
		mov  esi, g_dwDstRowStride
		mov  eax, g_dwDst
		mov  dword ptr [eax + esi], ecx
		mov  ecx, g_dwUStep
		add  g_dwSkyU, ecx
		add  g_dwDst, edx
		dec  g_dwSpanCount
		jne  L42A40F
	L42A466:
		pop  esi
		ret
	}
}

// hover.exe:0x42A470 (135 bytes)
// horizontal flat-shaded run (u only, g_dwSkyU), no shift;
// 4 pixels wide (dword write), 4 rows, no shade table.
// FUNCTION: HOVER 0x42a470
HOVER_NAKED Sky4x4(void)
{
	_asm
	{
		cmp  g_dwSpanCount, 0
		push esi
		je   L42A4F5
		mov  edx, 4
	L42A47F:
		mov  eax, g_dwSkyU
		mov  esi, g_dwTexels
		sar  eax, 0x10
		xor  ecx, ecx
		and  eax, g_dwUMask
		mov  cl, byte ptr [eax + esi]
		mov  esi, g_dwDst
		mov  eax, ecx
		shl  eax, 8
		or   ecx, eax
		mov  eax, ecx
		shl  eax, 0x10
		or   ecx, eax
		mov  dword ptr [esi], ecx
		mov  esi, g_dwDst
		mov  eax, g_dwDstRowStride
		mov  dword ptr [esi + eax], ecx
		mov  esi, g_dwDstRowStride
		mov  eax, g_dwDst
		mov  dword ptr [eax + esi*2], ecx
		mov  esi, g_dwDstRowStride
		mov  eax, g_dwDst
		lea  esi, [esi + esi*2]
		mov  dword ptr [esi + eax], ecx
		mov  ecx, g_dwUStep
		add  g_dwSkyU, ecx
		add  g_dwDst, edx
		dec  g_dwSpanCount
		jne  L42A47F
	L42A4F5:
		pop  esi
		ret
	}
}

/////////////////////////////////////////////////////////////////////////////
// Two-axis floor / ceiling spans (hover.exe:0x434470..0x435D2A)
//
// The perspective-correct floor and ceiling drawers: u = g_dwU
// stepped by g_dwUStep and wrapped with g_dwUMask, v =
// g_dwV stepped by g_dwVStep and wrapped with g_dwVMask;
// the texel address is g_dwTexels + (u >> 16) + ((v & vmask) >> s).
// Two families of variants:
//   - the generic ones (0x434470, 0x434500, 0x434B10, 0x434BB0, 0x435440,
//     0x4354F0) take the shift from g_dwVShift at run time (sar reg, cl);
//   - the specialised ones hard-code s = 6..15, i.e. one function per
//     texture width from 1024 down to 2 pixels, which is exactly the range
//     of m_ppfnDrawFlatByShift[6..15].
// Orthogonal to that: 1 / 2 / 4 screen pixels per texel (byte / word / dword
// store, the 2x and 4x forms pre-scaling g_dwUStep by the same factor)
// and with or without the shade table g_dwShadeTable.

// hover.exe:0x434470 (129 bytes)
// floor/ceiling span, texture size from g_dwVShift (shr cl);
// 1 pixel wide (byte write), no shade table.
// FUNCTION: HOVER 0x434470
HOVER_NAKED Span1x1_Var(void)
{
	_asm
	{
		push ebx
		mov  eax, g_dwSpanCount
		push esi
		dec  g_dwSpanCount
		push edi
		test eax, eax
		push ebp
		je   L4344EC
		mov  cl, byte ptr g_dwVShift
		mov  eax, g_dwTexels
		mov  edx, g_dwDst
		mov  edi, g_dwU
		mov  ebp, g_dwV
	L4344A0:
		mov  esi, g_dwVMask
		mov  ebx, g_dwUMask
		and  esi, ebp
		and  ebx, edi
		sar  ebx, 0x10
		inc  edx
		add  edi, g_dwUStep
		add  ebp, g_dwVStep
		sar  esi, cl
		add  esi, ebx
		mov  g_dwV, ebp
		mov  g_dwDst, edx
		mov  g_dwU, edi
		mov  bl, byte ptr [esi + eax]
		mov  esi, g_dwSpanCount
		mov  byte ptr [edx - 1], bl
		dec  g_dwSpanCount
		test esi, esi
		jne  L4344A0
	L4344EC:
		pop  ebp
		pop  edi
		pop  esi
		pop  ebx
		ret
	}
}

// hover.exe:0x434500 (139 bytes)
// floor/ceiling span, texture size from g_dwVShift (shr cl);
// 1 pixel wide (byte write), shade table g_dwShadeTable.
// FUNCTION: HOVER 0x434500
HOVER_NAKED Span1x1Lit_Var(void)
{
	_asm
	{
		push ebx
		mov  eax, g_dwSpanCount
		push esi
		dec  g_dwSpanCount
		push edi
		test eax, eax
		push ebp
		je   L434586
		mov  cl, byte ptr g_dwVShift
		mov  eax, g_dwTexels
		mov  edx, g_dwDst
		mov  esi, g_dwShadeTable
		mov  edi, g_dwU
	L434530:
		mov  ebp, g_dwVMask
		mov  ebx, g_dwUMask
		and  ebp, g_dwV
		and  ebx, edi
		sar  ebx, 0x10
		inc  edx
		add  edi, g_dwUStep
		mov  g_dwDst, edx
		sar  ebp, cl
		add  ebp, ebx
		mov  g_dwU, edi
		xor  ebx, ebx
		mov  bl, byte ptr [ebp + eax]
		mov  ebp, g_dwSpanCount
		dec  g_dwSpanCount
		mov  bl, byte ptr [ebx + esi]
		mov  byte ptr [edx - 1], bl
		mov  ebx, g_dwVStep
		add  g_dwV, ebx
		test ebp, ebp
		jne  L434530
	L434586:
		pop  ebp
		pop  edi
		pop  esi
		pop  ebx
		ret
	}
}

// hover.exe:0x434590 (102 bytes) floor/ceiling span, shr 6 = 1024-pixel
// texture; 1 pixel wide (byte write), shade table g_dwShadeTable,
// register-resident loop (leaves g_dwDst/40/14 stale).
// FUNCTION: HOVER 0x434590
HOVER_NAKED Span1x1Lit_S6(void)
{
	_asm
	{
		push ebx
		push esi
		push edi
		push ebp
		mov  eax, g_dwU
		and  eax, g_dwUMask
		mov  edx, g_dwV
		mov  edi, g_dwDst
		mov  ebp, g_dwTexels
		sub  ecx, ecx
		dec  edi
	L4345B4:
		mov  ebx, eax
		add  eax, g_dwUStep
		shr  ebx, 0x10
		and  eax, g_dwUMask
		lea  esi, [ebp + ebx]
		mov  ebx, edx
		add  edx, g_dwVStep
		and  ebx, g_dwVMask
		shr  ebx, 6
		inc  edi
		mov  cl, byte ptr [esi + ebx]
		mov  ebx, g_dwShadeTable
		mov  cl, byte ptr [ebx + ecx]
		dec  g_dwSpanCount
		mov  byte ptr [edi], cl
		jg   L4345B4
		pop  ebp
		pop  edi
		pop  esi
		pop  ebx
		ret
	}
}

// hover.exe:0x434600 (102 bytes) floor/ceiling span, shr 7 = 512-pixel
// texture; 1 pixel wide (byte write), shade table g_dwShadeTable,
// register-resident loop (leaves g_dwDst/40/14 stale).
// FUNCTION: HOVER 0x434600
HOVER_NAKED Span1x1Lit_S7(void)
{
	_asm
	{
		push ebx
		push esi
		push edi
		push ebp
		mov  eax, g_dwU
		and  eax, g_dwUMask
		mov  edx, g_dwV
		mov  edi, g_dwDst
		mov  ebp, g_dwTexels
		sub  ecx, ecx
		dec  edi
	L434624:
		mov  ebx, eax
		add  eax, g_dwUStep
		shr  ebx, 0x10
		and  eax, g_dwUMask
		lea  esi, [ebp + ebx]
		mov  ebx, edx
		add  edx, g_dwVStep
		and  ebx, g_dwVMask
		shr  ebx, 7
		inc  edi
		mov  cl, byte ptr [esi + ebx]
		mov  ebx, g_dwShadeTable
		mov  cl, byte ptr [ebx + ecx]
		dec  g_dwSpanCount
		mov  byte ptr [edi], cl
		jg   L434624
		pop  ebp
		pop  edi
		pop  esi
		pop  ebx
		ret
	}
}

// hover.exe:0x434670 (102 bytes) floor/ceiling span, shr 8 = 256-pixel
// texture; 1 pixel wide (byte write), shade table g_dwShadeTable,
// register-resident loop (leaves g_dwDst/40/14 stale).
// FUNCTION: HOVER 0x434670
HOVER_NAKED Span1x1Lit_S8(void)
{
	_asm
	{
		push ebx
		push esi
		push edi
		push ebp
		mov  eax, g_dwU
		and  eax, g_dwUMask
		mov  edx, g_dwV
		mov  edi, g_dwDst
		mov  ebp, g_dwTexels
		sub  ecx, ecx
		dec  edi
	L434694:
		mov  ebx, eax
		add  eax, g_dwUStep
		shr  ebx, 0x10
		and  eax, g_dwUMask
		lea  esi, [ebp + ebx]
		mov  ebx, edx
		add  edx, g_dwVStep
		and  ebx, g_dwVMask
		shr  ebx, 8
		inc  edi
		mov  cl, byte ptr [esi + ebx]
		mov  ebx, g_dwShadeTable
		mov  cl, byte ptr [ebx + ecx]
		dec  g_dwSpanCount
		mov  byte ptr [edi], cl
		jg   L434694
		pop  ebp
		pop  edi
		pop  esi
		pop  ebx
		ret
	}
}

// hover.exe:0x4346E0 (102 bytes) floor/ceiling span, shr 15 = 2-pixel
// texture; 1 pixel wide (byte write), shade table g_dwShadeTable,
// register-resident loop (leaves g_dwDst/40/14 stale).
// FUNCTION: HOVER 0x4346e0
HOVER_NAKED Span1x1Lit_S15(void)
{
	_asm
	{
		push ebx
		push esi
		push edi
		push ebp
		mov  eax, g_dwU
		and  eax, g_dwUMask
		mov  edx, g_dwV
		mov  edi, g_dwDst
		mov  ebp, g_dwTexels
		sub  ecx, ecx
		dec  edi
	L434704:
		mov  ebx, eax
		add  eax, g_dwUStep
		shr  ebx, 0x10
		and  eax, g_dwUMask
		lea  esi, [ebp + ebx]
		mov  ebx, edx
		add  edx, g_dwVStep
		and  ebx, g_dwVMask
		shr  ebx, 0xf
		inc  edi
		mov  cl, byte ptr [esi + ebx]
		mov  ebx, g_dwShadeTable
		mov  cl, byte ptr [ebx + ecx]
		dec  g_dwSpanCount
		mov  byte ptr [edi], cl
		jg   L434704
		pop  ebp
		pop  edi
		pop  esi
		pop  ebx
		ret
	}
}

// hover.exe:0x434750 (93 bytes) floor/ceiling span, shr 6 = 1024-pixel
// texture; 1 pixel wide (byte write), no shade table, register-resident loop
// (leaves g_dwDst/40/14 stale).
// FUNCTION: HOVER 0x434750
HOVER_NAKED Span1x1_S6(void)
{
	_asm
	{
		push ebx
		push esi
		push edi
		push ebp
		mov  eax, g_dwU
		and  eax, g_dwUMask
		mov  edx, g_dwV
		mov  edi, g_dwDst
		mov  ebp, g_dwTexels
		sub  ecx, ecx
		dec  edi
	L434774:
		mov  ebx, eax
		add  eax, g_dwUStep
		shr  ebx, 0x10
		and  eax, g_dwUMask
		lea  esi, [ebp + ebx]
		mov  ebx, edx
		add  edx, g_dwVStep
		and  ebx, g_dwVMask
		shr  ebx, 6
		inc  edi
		mov  cl, byte ptr [esi + ebx]
		dec  g_dwSpanCount
		mov  byte ptr [edi], cl
		jg   L434774
		pop  ebp
		pop  edi
		pop  esi
		pop  ebx
		ret
	}
}

// hover.exe:0x4347B0 (93 bytes) floor/ceiling span, shr 7 = 512-pixel
// texture; 1 pixel wide (byte write), no shade table, register-resident loop
// (leaves g_dwDst/40/14 stale).
// FUNCTION: HOVER 0x4347b0
HOVER_NAKED Span1x1_S7(void)
{
	_asm
	{
		push ebx
		push esi
		push edi
		push ebp
		mov  eax, g_dwU
		and  eax, g_dwUMask
		mov  edx, g_dwV
		mov  edi, g_dwDst
		mov  ebp, g_dwTexels
		sub  ecx, ecx
		dec  edi
	L4347D4:
		mov  ebx, eax
		add  eax, g_dwUStep
		shr  ebx, 0x10
		and  eax, g_dwUMask
		lea  esi, [ebp + ebx]
		mov  ebx, edx
		add  edx, g_dwVStep
		and  ebx, g_dwVMask
		shr  ebx, 7
		inc  edi
		mov  cl, byte ptr [esi + ebx]
		dec  g_dwSpanCount
		mov  byte ptr [edi], cl
		jg   L4347D4
		pop  ebp
		pop  edi
		pop  esi
		pop  ebx
		ret
	}
}

// hover.exe:0x434810 (93 bytes) floor/ceiling span, shr 8 = 256-pixel
// texture; 1 pixel wide (byte write), no shade table, register-resident loop
// (leaves g_dwDst/40/14 stale).
// FUNCTION: HOVER 0x434810
HOVER_NAKED Span1x1_S8(void)
{
	_asm
	{
		push ebx
		push esi
		push edi
		push ebp
		mov  eax, g_dwU
		and  eax, g_dwUMask
		mov  edx, g_dwV
		mov  edi, g_dwDst
		mov  ebp, g_dwTexels
		sub  ecx, ecx
		dec  edi
	L434834:
		mov  ebx, eax
		add  eax, g_dwUStep
		shr  ebx, 0x10
		and  eax, g_dwUMask
		lea  esi, [ebp + ebx]
		mov  ebx, edx
		add  edx, g_dwVStep
		and  ebx, g_dwVMask
		shr  ebx, 8
		inc  edi
		mov  cl, byte ptr [esi + ebx]
		dec  g_dwSpanCount
		mov  byte ptr [edi], cl
		jg   L434834
		pop  ebp
		pop  edi
		pop  esi
		pop  ebx
		ret
	}
}

// hover.exe:0x434870 (93 bytes) floor/ceiling span, shr 9 = 128-pixel
// texture; 1 pixel wide (byte write), no shade table, register-resident loop
// (leaves g_dwDst/40/14 stale).
// FUNCTION: HOVER 0x434870
HOVER_NAKED Span1x1_S9(void)
{
	_asm
	{
		push ebx
		push esi
		push edi
		push ebp
		mov  eax, g_dwU
		and  eax, g_dwUMask
		mov  edx, g_dwV
		mov  edi, g_dwDst
		mov  ebp, g_dwTexels
		sub  ecx, ecx
		dec  edi
	L434894:
		mov  ebx, eax
		add  eax, g_dwUStep
		shr  ebx, 0x10
		and  eax, g_dwUMask
		lea  esi, [ebp + ebx]
		mov  ebx, edx
		add  edx, g_dwVStep
		and  ebx, g_dwVMask
		shr  ebx, 9
		inc  edi
		mov  cl, byte ptr [esi + ebx]
		dec  g_dwSpanCount
		mov  byte ptr [edi], cl
		jg   L434894
		pop  ebp
		pop  edi
		pop  esi
		pop  ebx
		ret
	}
}

// hover.exe:0x4348D0 (93 bytes) floor/ceiling span, shr 10 = 64-pixel
// texture; 1 pixel wide (byte write), no shade table, register-resident loop
// (leaves g_dwDst/40/14 stale).
// FUNCTION: HOVER 0x4348d0
HOVER_NAKED Span1x1_S10(void)
{
	_asm
	{
		push ebx
		push esi
		push edi
		push ebp
		mov  eax, g_dwU
		and  eax, g_dwUMask
		mov  edx, g_dwV
		mov  edi, g_dwDst
		mov  ebp, g_dwTexels
		sub  ecx, ecx
		dec  edi
	L4348F4:
		mov  ebx, eax
		add  eax, g_dwUStep
		shr  ebx, 0x10
		and  eax, g_dwUMask
		lea  esi, [ebp + ebx]
		mov  ebx, edx
		add  edx, g_dwVStep
		and  ebx, g_dwVMask
		shr  ebx, 0xa
		inc  edi
		mov  cl, byte ptr [esi + ebx]
		dec  g_dwSpanCount
		mov  byte ptr [edi], cl
		jg   L4348F4
		pop  ebp
		pop  edi
		pop  esi
		pop  ebx
		ret
	}
}

// hover.exe:0x434930 (93 bytes) floor/ceiling span, shr 11 = 32-pixel
// texture; 1 pixel wide (byte write), no shade table, register-resident loop
// (leaves g_dwDst/40/14 stale).
// FUNCTION: HOVER 0x434930
HOVER_NAKED Span1x1_S11(void)
{
	_asm
	{
		push ebx
		push esi
		push edi
		push ebp
		mov  eax, g_dwU
		and  eax, g_dwUMask
		mov  edx, g_dwV
		mov  edi, g_dwDst
		mov  ebp, g_dwTexels
		sub  ecx, ecx
		dec  edi
	L434954:
		mov  ebx, eax
		add  eax, g_dwUStep
		shr  ebx, 0x10
		and  eax, g_dwUMask
		lea  esi, [ebp + ebx]
		mov  ebx, edx
		add  edx, g_dwVStep
		and  ebx, g_dwVMask
		shr  ebx, 0xb
		inc  edi
		mov  cl, byte ptr [esi + ebx]
		dec  g_dwSpanCount
		mov  byte ptr [edi], cl
		jg   L434954
		pop  ebp
		pop  edi
		pop  esi
		pop  ebx
		ret
	}
}

// hover.exe:0x434990 (93 bytes) floor/ceiling span, shr 12 = 16-pixel
// texture; 1 pixel wide (byte write), no shade table, register-resident loop
// (leaves g_dwDst/40/14 stale).
// FUNCTION: HOVER 0x434990
HOVER_NAKED Span1x1_S12(void)
{
	_asm
	{
		push ebx
		push esi
		push edi
		push ebp
		mov  eax, g_dwU
		and  eax, g_dwUMask
		mov  edx, g_dwV
		mov  edi, g_dwDst
		mov  ebp, g_dwTexels
		sub  ecx, ecx
		dec  edi
	L4349B4:
		mov  ebx, eax
		add  eax, g_dwUStep
		shr  ebx, 0x10
		and  eax, g_dwUMask
		lea  esi, [ebp + ebx]
		mov  ebx, edx
		add  edx, g_dwVStep
		and  ebx, g_dwVMask
		shr  ebx, 0xc
		inc  edi
		mov  cl, byte ptr [esi + ebx]
		dec  g_dwSpanCount
		mov  byte ptr [edi], cl
		jg   L4349B4
		pop  ebp
		pop  edi
		pop  esi
		pop  ebx
		ret
	}
}

// hover.exe:0x4349F0 (93 bytes) floor/ceiling span, shr 13 = 8-pixel texture;
// 1 pixel wide (byte write), no shade table, register-resident loop (leaves
// g_dwDst/40/14 stale).
// FUNCTION: HOVER 0x4349f0
HOVER_NAKED Span1x1_S13(void)
{
	_asm
	{
		push ebx
		push esi
		push edi
		push ebp
		mov  eax, g_dwU
		and  eax, g_dwUMask
		mov  edx, g_dwV
		mov  edi, g_dwDst
		mov  ebp, g_dwTexels
		sub  ecx, ecx
		dec  edi
	L434A14:
		mov  ebx, eax
		add  eax, g_dwUStep
		shr  ebx, 0x10
		and  eax, g_dwUMask
		lea  esi, [ebp + ebx]
		mov  ebx, edx
		add  edx, g_dwVStep
		and  ebx, g_dwVMask
		shr  ebx, 0xd
		inc  edi
		mov  cl, byte ptr [esi + ebx]
		dec  g_dwSpanCount
		mov  byte ptr [edi], cl
		jg   L434A14
		pop  ebp
		pop  edi
		pop  esi
		pop  ebx
		ret
	}
}

// hover.exe:0x434A50 (93 bytes) floor/ceiling span, shr 14 = 4-pixel texture;
// 1 pixel wide (byte write), no shade table, register-resident loop (leaves
// g_dwDst/40/14 stale).
// FUNCTION: HOVER 0x434a50
HOVER_NAKED Span1x1_S14(void)
{
	_asm
	{
		push ebx
		push esi
		push edi
		push ebp
		mov  eax, g_dwU
		and  eax, g_dwUMask
		mov  edx, g_dwV
		mov  edi, g_dwDst
		mov  ebp, g_dwTexels
		sub  ecx, ecx
		dec  edi
	L434A74:
		mov  ebx, eax
		add  eax, g_dwUStep
		shr  ebx, 0x10
		and  eax, g_dwUMask
		lea  esi, [ebp + ebx]
		mov  ebx, edx
		add  edx, g_dwVStep
		and  ebx, g_dwVMask
		shr  ebx, 0xe
		inc  edi
		mov  cl, byte ptr [esi + ebx]
		dec  g_dwSpanCount
		mov  byte ptr [edi], cl
		jg   L434A74
		pop  ebp
		pop  edi
		pop  esi
		pop  ebx
		ret
	}
}

// hover.exe:0x434AB0 (93 bytes) floor/ceiling span, shr 15 = 2-pixel texture;
// 1 pixel wide (byte write), no shade table, register-resident loop (leaves
// g_dwDst/40/14 stale).
// FUNCTION: HOVER 0x434ab0
HOVER_NAKED Span1x1_S15(void)
{
	_asm
	{
		push ebx
		push esi
		push edi
		push ebp
		mov  eax, g_dwU
		and  eax, g_dwUMask
		mov  edx, g_dwV
		mov  edi, g_dwDst
		mov  ebp, g_dwTexels
		sub  ecx, ecx
		dec  edi
	L434AD4:
		mov  ebx, eax
		add  eax, g_dwUStep
		shr  ebx, 0x10
		and  eax, g_dwUMask
		lea  esi, [ebp + ebx]
		mov  ebx, edx
		add  edx, g_dwVStep
		and  ebx, g_dwVMask
		shr  ebx, 0xf
		inc  edi
		mov  cl, byte ptr [esi + ebx]
		dec  g_dwSpanCount
		mov  byte ptr [edi], cl
		jg   L434AD4
		pop  ebp
		pop  edi
		pop  esi
		pop  ebx
		ret
	}
}

// hover.exe:0x434B10 (157 bytes)
// floor/ceiling span, texture size from g_dwVShift (shr cl);
// 2 pixels wide (word write), no shade table.
// FUNCTION: HOVER 0x434b10
HOVER_NAKED Span2x1_Var(void)
{
	_asm
	{
		push ebx
		mov  eax, g_dwSpanCount
		cdq
		push esi
		sub  eax, edx
		shl  g_dwUStep, 1
		shl  g_dwVStep, 1
		sar  eax, 1
		mov  g_dwSpanCount, eax
		dec  g_dwSpanCount
		test eax, eax
		je   L434BAA
		mov  cl, byte ptr g_dwVShift
		mov  edx, 2
	L434B45:
		mov  esi, g_dwVMask
		mov  eax, g_dwUMask
		and  esi, g_dwV
		and  eax, g_dwU
		sar  eax, 0x10
		xor  ebx, ebx
		sar  esi, cl
		add  esi, eax
		mov  eax, g_dwTexels
		mov  bl, byte ptr [esi + eax]
		mov  esi, g_dwDst
		mov  eax, ebx
		shl  eax, 8
		or   ebx, eax
		mov  word ptr [esi], bx
		mov  ebx, g_dwVStep
		mov  eax, g_dwUStep
		mov  esi, g_dwSpanCount
		add  g_dwDst, edx
		add  g_dwU, eax
		add  g_dwV, ebx
		dec  g_dwSpanCount
		test esi, esi
		jne  L434B45
	L434BAA:
		pop  esi
		pop  ebx
		ret
	}
}

// hover.exe:0x434BB0 (168 bytes)
// floor/ceiling span, texture size from g_dwVShift (shr cl);
// 2 pixels wide (word write), shade table g_dwShadeTable.
// FUNCTION: HOVER 0x434bb0
HOVER_NAKED Span2x1Lit_Var(void)
{
	_asm
	{
		push ebx
		mov  eax, g_dwSpanCount
		cdq
		push esi
		sub  eax, edx
		shl  g_dwUStep, 1
		shl  g_dwVStep, 1
		sar  eax, 1
		mov  g_dwSpanCount, eax
		dec  g_dwSpanCount
		test eax, eax
		je   L434C55
		mov  cl, byte ptr g_dwVShift
		mov  edx, 2
	L434BE5:
		mov  esi, g_dwVMask
		mov  eax, g_dwUMask
		and  esi, g_dwV
		and  eax, g_dwU
		sar  eax, 0x10
		xor  ebx, ebx
		sar  esi, cl
		add  esi, eax
		mov  eax, g_dwTexels
		mov  bl, byte ptr [esi + eax]
		mov  esi, g_dwShadeTable
		xor  eax, eax
		mov  al, byte ptr [ebx + esi]
		mov  ebx, g_dwDst
		mov  esi, eax
		shl  esi, 8
		or   eax, esi
		mov  word ptr [ebx], ax
		mov  eax, g_dwVStep
		mov  esi, g_dwUStep
		mov  ebx, g_dwSpanCount
		add  g_dwDst, edx
		add  g_dwU, esi
		add  g_dwV, eax
		dec  g_dwSpanCount
		test ebx, ebx
		jne  L434BE5
	L434C55:
		pop  esi
		pop  ebx
		ret
	}
}

// hover.exe:0x434C60 (141 bytes) floor/ceiling span, shr 6 = 1024-pixel
// texture; 2 pixels wide (word write), shade table g_dwShadeTable,
// register-resident loop (leaves g_dwDst/40/14 stale).
// FUNCTION: HOVER 0x434c60
HOVER_NAKED Span2x1Lit_S6(void)
{
	_asm
	{
		push ebx
		mov  eax, g_dwSpanCount
		cdq
		push esi
		sub  eax, edx
		sar  eax, 1
		push edi
		shl  g_dwUStep, 1
		shl  g_dwVStep, 1
		mov  g_dwSpanCount, eax
		push ebp
		mov  eax, g_dwU
		and  eax, g_dwUMask
		mov  edx, g_dwV
		mov  edi, g_dwDst
		mov  ebp, g_dwTexels
		sub  ecx, ecx
		sub  edi, 2
	L434CA4:
		mov  ebx, eax
		add  eax, g_dwUStep
		shr  ebx, 0x10
		and  eax, g_dwUMask
		lea  esi, [ebp + ebx]
		mov  ebx, edx
		add  edx, g_dwVStep
		and  ebx, g_dwVMask
		shr  ebx, 6
		add  edi, 2
		sub  ch, ch
		mov  cl, byte ptr [esi + ebx]
		mov  ebx, g_dwShadeTable
		mov  cl, byte ptr [ebx + ecx]
		dec  g_dwSpanCount
		mov  ch, cl
		mov  word ptr [edi], cx
		jg   L434CA4
		pop  ebp
		pop  edi
		pop  esi
		pop  ebx
		ret
	}
}

// hover.exe:0x434CF0 (141 bytes) floor/ceiling span, shr 7 = 512-pixel
// texture; 2 pixels wide (word write), shade table g_dwShadeTable,
// register-resident loop (leaves g_dwDst/40/14 stale).
// FUNCTION: HOVER 0x434cf0
HOVER_NAKED Span2x1Lit_S7(void)
{
	_asm
	{
		push ebx
		mov  eax, g_dwSpanCount
		cdq
		push esi
		sub  eax, edx
		sar  eax, 1
		push edi
		shl  g_dwUStep, 1
		shl  g_dwVStep, 1
		mov  g_dwSpanCount, eax
		push ebp
		mov  eax, g_dwU
		and  eax, g_dwUMask
		mov  edx, g_dwV
		mov  edi, g_dwDst
		mov  ebp, g_dwTexels
		sub  ecx, ecx
		sub  edi, 2
	L434D34:
		mov  ebx, eax
		add  eax, g_dwUStep
		shr  ebx, 0x10
		and  eax, g_dwUMask
		lea  esi, [ebp + ebx]
		mov  ebx, edx
		add  edx, g_dwVStep
		and  ebx, g_dwVMask
		shr  ebx, 7
		add  edi, 2
		sub  ch, ch
		mov  cl, byte ptr [esi + ebx]
		mov  ebx, g_dwShadeTable
		mov  cl, byte ptr [ebx + ecx]
		dec  g_dwSpanCount
		mov  ch, cl
		mov  word ptr [edi], cx
		jg   L434D34
		pop  ebp
		pop  edi
		pop  esi
		pop  ebx
		ret
	}
}

// hover.exe:0x434D80 (141 bytes) floor/ceiling span, shr 8 = 256-pixel
// texture; 2 pixels wide (word write), shade table g_dwShadeTable,
// register-resident loop (leaves g_dwDst/40/14 stale).
// FUNCTION: HOVER 0x434d80
HOVER_NAKED Span2x1Lit_S8(void)
{
	_asm
	{
		push ebx
		mov  eax, g_dwSpanCount
		cdq
		push esi
		sub  eax, edx
		sar  eax, 1
		push edi
		shl  g_dwUStep, 1
		shl  g_dwVStep, 1
		mov  g_dwSpanCount, eax
		push ebp
		mov  eax, g_dwU
		and  eax, g_dwUMask
		mov  edx, g_dwV
		mov  edi, g_dwDst
		mov  ebp, g_dwTexels
		sub  ecx, ecx
		sub  edi, 2
	L434DC4:
		mov  ebx, eax
		add  eax, g_dwUStep
		shr  ebx, 0x10
		and  eax, g_dwUMask
		lea  esi, [ebp + ebx]
		mov  ebx, edx
		add  edx, g_dwVStep
		and  ebx, g_dwVMask
		shr  ebx, 8
		add  edi, 2
		sub  ch, ch
		mov  cl, byte ptr [esi + ebx]
		mov  ebx, g_dwShadeTable
		mov  cl, byte ptr [ebx + ecx]
		dec  g_dwSpanCount
		mov  ch, cl
		mov  word ptr [edi], cx
		jg   L434DC4
		pop  ebp
		pop  edi
		pop  esi
		pop  ebx
		ret
	}
}

// hover.exe:0x434E10 (141 bytes) floor/ceiling span, shr 15 = 2-pixel
// texture; 2 pixels wide (word write), shade table g_dwShadeTable,
// register-resident loop (leaves g_dwDst/40/14 stale).
// FUNCTION: HOVER 0x434e10
HOVER_NAKED Span2x1Lit_S15(void)
{
	_asm
	{
		push ebx
		mov  eax, g_dwSpanCount
		cdq
		push esi
		sub  eax, edx
		sar  eax, 1
		push edi
		shl  g_dwUStep, 1
		shl  g_dwVStep, 1
		mov  g_dwSpanCount, eax
		push ebp
		mov  eax, g_dwU
		and  eax, g_dwUMask
		mov  edx, g_dwV
		mov  edi, g_dwDst
		mov  ebp, g_dwTexels
		sub  ecx, ecx
		sub  edi, 2
	L434E54:
		mov  ebx, eax
		add  eax, g_dwUStep
		shr  ebx, 0x10
		and  eax, g_dwUMask
		lea  esi, [ebp + ebx]
		mov  ebx, edx
		add  edx, g_dwVStep
		and  ebx, g_dwVMask
		shr  ebx, 0xf
		add  edi, 2
		sub  ch, ch
		mov  cl, byte ptr [esi + ebx]
		mov  ebx, g_dwShadeTable
		mov  cl, byte ptr [ebx + ecx]
		dec  g_dwSpanCount
		mov  ch, cl
		mov  word ptr [edi], cx
		jg   L434E54
		pop  ebp
		pop  edi
		pop  esi
		pop  ebx
		ret
	}
}

// hover.exe:0x434EA0 (130 bytes) floor/ceiling span, shr 6 = 1024-pixel
// texture; 2 pixels wide (word write), no shade table, register-resident loop
// (leaves g_dwDst/40/14 stale).
// FUNCTION: HOVER 0x434ea0
HOVER_NAKED Span2x1_S6(void)
{
	_asm
	{
		push ebx
		mov  eax, g_dwSpanCount
		cdq
		push esi
		sub  eax, edx
		sar  eax, 1
		push edi
		shl  g_dwUStep, 1
		shl  g_dwVStep, 1
		mov  g_dwSpanCount, eax
		push ebp
		mov  eax, g_dwU
		and  eax, g_dwUMask
		mov  edx, g_dwV
		mov  edi, g_dwDst
		mov  ebp, g_dwTexels
		sub  ecx, ecx
		sub  edi, 2
	L434EE4:
		mov  ebx, eax
		add  eax, g_dwUStep
		shr  ebx, 0x10
		and  eax, g_dwUMask
		lea  esi, [ebp + ebx]
		mov  ebx, edx
		add  edx, g_dwVStep
		and  ebx, g_dwVMask
		shr  ebx, 6
		add  edi, 2
		mov  cl, byte ptr [esi + ebx]
		dec  g_dwSpanCount
		mov  ch, cl
		mov  word ptr [edi], cx
		jg   L434EE4
		pop  ebp
		pop  edi
		pop  esi
		pop  ebx
		ret
	}
}

// hover.exe:0x434F30 (130 bytes) floor/ceiling span, shr 7 = 512-pixel
// texture; 2 pixels wide (word write), no shade table, register-resident loop
// (leaves g_dwDst/40/14 stale).
// FUNCTION: HOVER 0x434f30
HOVER_NAKED Span2x1_S7(void)
{
	_asm
	{
		push ebx
		mov  eax, g_dwSpanCount
		cdq
		push esi
		sub  eax, edx
		sar  eax, 1
		push edi
		shl  g_dwUStep, 1
		shl  g_dwVStep, 1
		mov  g_dwSpanCount, eax
		push ebp
		mov  eax, g_dwU
		and  eax, g_dwUMask
		mov  edx, g_dwV
		mov  edi, g_dwDst
		mov  ebp, g_dwTexels
		sub  ecx, ecx
		sub  edi, 2
	L434F74:
		mov  ebx, eax
		add  eax, g_dwUStep
		shr  ebx, 0x10
		and  eax, g_dwUMask
		lea  esi, [ebp + ebx]
		mov  ebx, edx
		add  edx, g_dwVStep
		and  ebx, g_dwVMask
		shr  ebx, 7
		add  edi, 2
		mov  cl, byte ptr [esi + ebx]
		dec  g_dwSpanCount
		mov  ch, cl
		mov  word ptr [edi], cx
		jg   L434F74
		pop  ebp
		pop  edi
		pop  esi
		pop  ebx
		ret
	}
}

// hover.exe:0x434FC0 (130 bytes) floor/ceiling span, shr 8 = 256-pixel
// texture; 2 pixels wide (word write), no shade table, register-resident loop
// (leaves g_dwDst/40/14 stale).
// FUNCTION: HOVER 0x434fc0
HOVER_NAKED Span2x1_S8(void)
{
	_asm
	{
		push ebx
		mov  eax, g_dwSpanCount
		cdq
		push esi
		sub  eax, edx
		sar  eax, 1
		push edi
		shl  g_dwUStep, 1
		shl  g_dwVStep, 1
		mov  g_dwSpanCount, eax
		push ebp
		mov  eax, g_dwU
		and  eax, g_dwUMask
		mov  edx, g_dwV
		mov  edi, g_dwDst
		mov  ebp, g_dwTexels
		sub  ecx, ecx
		sub  edi, 2
	L435004:
		mov  ebx, eax
		add  eax, g_dwUStep
		shr  ebx, 0x10
		and  eax, g_dwUMask
		lea  esi, [ebp + ebx]
		mov  ebx, edx
		add  edx, g_dwVStep
		and  ebx, g_dwVMask
		shr  ebx, 8
		add  edi, 2
		mov  cl, byte ptr [esi + ebx]
		dec  g_dwSpanCount
		mov  ch, cl
		mov  word ptr [edi], cx
		jg   L435004
		pop  ebp
		pop  edi
		pop  esi
		pop  ebx
		ret
	}
}

// hover.exe:0x435050 (130 bytes) floor/ceiling span, shr 9 = 128-pixel
// texture; 2 pixels wide (word write), no shade table, register-resident loop
// (leaves g_dwDst/40/14 stale).
// FUNCTION: HOVER 0x435050
HOVER_NAKED Span2x1_S9(void)
{
	_asm
	{
		push ebx
		mov  eax, g_dwSpanCount
		cdq
		push esi
		sub  eax, edx
		sar  eax, 1
		push edi
		shl  g_dwUStep, 1
		shl  g_dwVStep, 1
		mov  g_dwSpanCount, eax
		push ebp
		mov  eax, g_dwU
		and  eax, g_dwUMask
		mov  edx, g_dwV
		mov  edi, g_dwDst
		mov  ebp, g_dwTexels
		sub  ecx, ecx
		sub  edi, 2
	L435094:
		mov  ebx, eax
		add  eax, g_dwUStep
		shr  ebx, 0x10
		and  eax, g_dwUMask
		lea  esi, [ebp + ebx]
		mov  ebx, edx
		add  edx, g_dwVStep
		and  ebx, g_dwVMask
		shr  ebx, 9
		add  edi, 2
		mov  cl, byte ptr [esi + ebx]
		dec  g_dwSpanCount
		mov  ch, cl
		mov  word ptr [edi], cx
		jg   L435094
		pop  ebp
		pop  edi
		pop  esi
		pop  ebx
		ret
	}
}

// hover.exe:0x4350E0 (130 bytes) floor/ceiling span, shr 10 = 64-pixel
// texture; 2 pixels wide (word write), no shade table, register-resident loop
// (leaves g_dwDst/40/14 stale).
// FUNCTION: HOVER 0x4350e0
HOVER_NAKED Span2x1_S10(void)
{
	_asm
	{
		push ebx
		mov  eax, g_dwSpanCount
		cdq
		push esi
		sub  eax, edx
		sar  eax, 1
		push edi
		shl  g_dwUStep, 1
		shl  g_dwVStep, 1
		mov  g_dwSpanCount, eax
		push ebp
		mov  eax, g_dwU
		and  eax, g_dwUMask
		mov  edx, g_dwV
		mov  edi, g_dwDst
		mov  ebp, g_dwTexels
		sub  ecx, ecx
		sub  edi, 2
	L435124:
		mov  ebx, eax
		add  eax, g_dwUStep
		shr  ebx, 0x10
		and  eax, g_dwUMask
		lea  esi, [ebp + ebx]
		mov  ebx, edx
		add  edx, g_dwVStep
		and  ebx, g_dwVMask
		shr  ebx, 0xa
		add  edi, 2
		mov  cl, byte ptr [esi + ebx]
		dec  g_dwSpanCount
		mov  ch, cl
		mov  word ptr [edi], cx
		jg   L435124
		pop  ebp
		pop  edi
		pop  esi
		pop  ebx
		ret
	}
}

// hover.exe:0x435170 (130 bytes) floor/ceiling span, shr 11 = 32-pixel
// texture; 2 pixels wide (word write), no shade table, register-resident loop
// (leaves g_dwDst/40/14 stale).
// FUNCTION: HOVER 0x435170
HOVER_NAKED Span2x1_S11(void)
{
	_asm
	{
		push ebx
		mov  eax, g_dwSpanCount
		cdq
		push esi
		sub  eax, edx
		sar  eax, 1
		push edi
		shl  g_dwUStep, 1
		shl  g_dwVStep, 1
		mov  g_dwSpanCount, eax
		push ebp
		mov  eax, g_dwU
		and  eax, g_dwUMask
		mov  edx, g_dwV
		mov  edi, g_dwDst
		mov  ebp, g_dwTexels
		sub  ecx, ecx
		sub  edi, 2
	L4351B4:
		mov  ebx, eax
		add  eax, g_dwUStep
		shr  ebx, 0x10
		and  eax, g_dwUMask
		lea  esi, [ebp + ebx]
		mov  ebx, edx
		add  edx, g_dwVStep
		and  ebx, g_dwVMask
		shr  ebx, 0xb
		add  edi, 2
		mov  cl, byte ptr [esi + ebx]
		dec  g_dwSpanCount
		mov  ch, cl
		mov  word ptr [edi], cx
		jg   L4351B4
		pop  ebp
		pop  edi
		pop  esi
		pop  ebx
		ret
	}
}

// hover.exe:0x435200 (130 bytes) floor/ceiling span, shr 12 = 16-pixel
// texture; 2 pixels wide (word write), no shade table, register-resident loop
// (leaves g_dwDst/40/14 stale).
// FUNCTION: HOVER 0x435200
HOVER_NAKED Span2x1_S12(void)
{
	_asm
	{
		push ebx
		mov  eax, g_dwSpanCount
		cdq
		push esi
		sub  eax, edx
		sar  eax, 1
		push edi
		shl  g_dwUStep, 1
		shl  g_dwVStep, 1
		mov  g_dwSpanCount, eax
		push ebp
		mov  eax, g_dwU
		and  eax, g_dwUMask
		mov  edx, g_dwV
		mov  edi, g_dwDst
		mov  ebp, g_dwTexels
		sub  ecx, ecx
		sub  edi, 2
	L435244:
		mov  ebx, eax
		add  eax, g_dwUStep
		shr  ebx, 0x10
		and  eax, g_dwUMask
		lea  esi, [ebp + ebx]
		mov  ebx, edx
		add  edx, g_dwVStep
		and  ebx, g_dwVMask
		shr  ebx, 0xc
		add  edi, 2
		mov  cl, byte ptr [esi + ebx]
		dec  g_dwSpanCount
		mov  ch, cl
		mov  word ptr [edi], cx
		jg   L435244
		pop  ebp
		pop  edi
		pop  esi
		pop  ebx
		ret
	}
}

// hover.exe:0x435290 (130 bytes) floor/ceiling span, shr 13 = 8-pixel
// texture; 2 pixels wide (word write), no shade table, register-resident loop
// (leaves g_dwDst/40/14 stale).
// FUNCTION: HOVER 0x435290
HOVER_NAKED Span2x1_S13(void)
{
	_asm
	{
		push ebx
		mov  eax, g_dwSpanCount
		cdq
		push esi
		sub  eax, edx
		sar  eax, 1
		push edi
		shl  g_dwUStep, 1
		shl  g_dwVStep, 1
		mov  g_dwSpanCount, eax
		push ebp
		mov  eax, g_dwU
		and  eax, g_dwUMask
		mov  edx, g_dwV
		mov  edi, g_dwDst
		mov  ebp, g_dwTexels
		sub  ecx, ecx
		sub  edi, 2
	L4352D4:
		mov  ebx, eax
		add  eax, g_dwUStep
		shr  ebx, 0x10
		and  eax, g_dwUMask
		lea  esi, [ebp + ebx]
		mov  ebx, edx
		add  edx, g_dwVStep
		and  ebx, g_dwVMask
		shr  ebx, 0xd
		add  edi, 2
		mov  cl, byte ptr [esi + ebx]
		dec  g_dwSpanCount
		mov  ch, cl
		mov  word ptr [edi], cx
		jg   L4352D4
		pop  ebp
		pop  edi
		pop  esi
		pop  ebx
		ret
	}
}

// hover.exe:0x435320 (130 bytes) floor/ceiling span, shr 14 = 4-pixel
// texture; 2 pixels wide (word write), no shade table, register-resident loop
// (leaves g_dwDst/40/14 stale).
// FUNCTION: HOVER 0x435320
HOVER_NAKED Span2x1_S14(void)
{
	_asm
	{
		push ebx
		mov  eax, g_dwSpanCount
		cdq
		push esi
		sub  eax, edx
		sar  eax, 1
		push edi
		shl  g_dwUStep, 1
		shl  g_dwVStep, 1
		mov  g_dwSpanCount, eax
		push ebp
		mov  eax, g_dwU
		and  eax, g_dwUMask
		mov  edx, g_dwV
		mov  edi, g_dwDst
		mov  ebp, g_dwTexels
		sub  ecx, ecx
		sub  edi, 2
	L435364:
		mov  ebx, eax
		add  eax, g_dwUStep
		shr  ebx, 0x10
		and  eax, g_dwUMask
		lea  esi, [ebp + ebx]
		mov  ebx, edx
		add  edx, g_dwVStep
		and  ebx, g_dwVMask
		shr  ebx, 0xe
		add  edi, 2
		mov  cl, byte ptr [esi + ebx]
		dec  g_dwSpanCount
		mov  ch, cl
		mov  word ptr [edi], cx
		jg   L435364
		pop  ebp
		pop  edi
		pop  esi
		pop  ebx
		ret
	}
}

// hover.exe:0x4353B0 (130 bytes) floor/ceiling span, shr 15 = 2-pixel
// texture; 2 pixels wide (word write), no shade table, register-resident loop
// (leaves g_dwDst/40/14 stale).
// FUNCTION: HOVER 0x4353b0
HOVER_NAKED Span2x1_S15(void)
{
	_asm
	{
		push ebx
		mov  eax, g_dwSpanCount
		cdq
		push esi
		sub  eax, edx
		sar  eax, 1
		push edi
		shl  g_dwUStep, 1
		shl  g_dwVStep, 1
		mov  g_dwSpanCount, eax
		push ebp
		mov  eax, g_dwU
		and  eax, g_dwUMask
		mov  edx, g_dwV
		mov  edi, g_dwDst
		mov  ebp, g_dwTexels
		sub  ecx, ecx
		sub  edi, 2
	L4353F4:
		mov  ebx, eax
		add  eax, g_dwUStep
		shr  ebx, 0x10
		and  eax, g_dwUMask
		lea  esi, [ebp + ebx]
		mov  ebx, edx
		add  edx, g_dwVStep
		and  ebx, g_dwVMask
		shr  ebx, 0xf
		add  edi, 2
		mov  cl, byte ptr [esi + ebx]
		dec  g_dwSpanCount
		mov  ch, cl
		mov  word ptr [edi], cx
		jg   L4353F4
		pop  ebp
		pop  edi
		pop  esi
		pop  ebx
		ret
	}
}

// hover.exe:0x435440 (166 bytes)
// floor/ceiling span, texture size from g_dwVShift (shr cl);
// 4 pixels wide (dword write), no shade table.
// FUNCTION: HOVER 0x435440
HOVER_NAKED Span4x1_Var(void)
{
	_asm
	{
		push ebx
		mov  eax, g_dwSpanCount
		cdq
		push esi
		and  edx, 3
		shl  g_dwUStep, 2
		shl  g_dwVStep, 2
		add  eax, edx
		sar  eax, 2
		mov  g_dwSpanCount, eax
		dec  g_dwSpanCount
		test eax, eax
		je   L4354E3
		mov  cl, byte ptr g_dwVShift
		mov  edx, 4
	L435478:
		mov  esi, g_dwVMask
		mov  eax, g_dwUMask
		and  esi, g_dwV
		and  eax, g_dwU
		sar  eax, 0x10
		xor  ebx, ebx
		sar  esi, cl
		add  esi, eax
		mov  eax, g_dwTexels
		mov  bl, byte ptr [esi + eax]
		mov  esi, g_dwDst
		mov  eax, ebx
		shl  eax, 8
		or   ebx, eax
		mov  eax, ebx
		shl  eax, 0x10
		or   ebx, eax
		mov  dword ptr [esi], ebx
		mov  ebx, g_dwVStep
		mov  eax, g_dwUStep
		mov  esi, g_dwSpanCount
		add  g_dwDst, edx
		add  g_dwU, eax
		add  g_dwV, ebx
		dec  g_dwSpanCount
		test esi, esi
		jne  L435478
	L4354E3:
		pop  esi
		pop  ebx
		ret
	}
}

// hover.exe:0x4354F0 (181 bytes)
// floor/ceiling span, texture size from g_dwVShift (shr cl);
// 4 pixels wide (dword write), shade table g_dwShadeTable.
// FUNCTION: HOVER 0x4354f0
HOVER_NAKED Span4x1Lit_Var(void)
{
	_asm
	{
		push ebx
		mov  eax, g_dwSpanCount
		cdq
		push esi
		and  edx, 3
		shl  g_dwUStep, 2
		shl  g_dwVStep, 2
		add  eax, edx
		sar  eax, 2
		mov  g_dwSpanCount, eax
		dec  g_dwSpanCount
		test eax, eax
		je   L4355A2
		mov  cl, byte ptr g_dwVShift
		mov  edx, 4
	L43552C:
		mov  esi, g_dwVMask
		mov  eax, g_dwUMask
		and  esi, g_dwV
		and  eax, g_dwU
		sar  eax, 0x10
		xor  ebx, ebx
		sar  esi, cl
		add  esi, eax
		mov  eax, g_dwTexels
		mov  bl, byte ptr [esi + eax]
		mov  esi, g_dwShadeTable
		xor  eax, eax
		mov  al, byte ptr [ebx + esi]
		mov  ebx, g_dwDst
		mov  esi, eax
		shl  esi, 8
		or   eax, esi
		mov  esi, eax
		shl  esi, 0x10
		or   eax, esi
		mov  dword ptr [ebx], eax
		mov  eax, g_dwVStep
		mov  esi, g_dwUStep
		mov  ebx, g_dwSpanCount
		add  g_dwDst, edx
		add  g_dwU, esi
		add  g_dwV, eax
		dec  g_dwSpanCount
		test ebx, ebx
		jne  L43552C
	L4355A2:
		pop  esi
		pop  ebx
		ret
	}
}

// hover.exe:0x4355B0 (150 bytes) floor/ceiling span, shr 6 = 1024-pixel
// texture; 4 pixels wide (dword write), shade table g_dwShadeTable,
// register-resident loop (leaves g_dwDst/40/14 stale).
// FUNCTION: HOVER 0x4355b0
HOVER_NAKED Span4x1Lit_S6(void)
{
	_asm
	{
		push ebx
		mov  eax, g_dwSpanCount
		cdq
		push esi
		and  edx, 3
		push edi
		add  eax, edx
		shl  g_dwUStep, 2
		shl  g_dwVStep, 2
		sar  eax, 2
		mov  g_dwSpanCount, eax
		push ebp
		mov  eax, g_dwU
		and  eax, g_dwUMask
		mov  edx, g_dwV
		mov  edi, g_dwDst
		mov  ebp, g_dwTexels
		sub  ecx, ecx
		sub  edi, 4
	L4355F7:
		mov  ebx, eax
		add  eax, g_dwUStep
		shr  ebx, 0x10
		and  eax, g_dwUMask
		lea  esi, [ebp + ebx]
		mov  ebx, edx
		add  edx, g_dwVStep
		and  ebx, g_dwVMask
		shr  ebx, 6
		add  edi, 4
		sub  ecx, ecx
		mov  cl, byte ptr [esi + ebx]
		mov  ebx, g_dwShadeTable
		mov  cl, byte ptr [ebx + ecx]
		mov  ch, cl
		push cx
		shl  ecx, 0x10
		pop  cx
		dec  g_dwSpanCount
		mov  dword ptr [edi], ecx
		jg   L4355F7
		pop  ebp
		pop  edi
		pop  esi
		pop  ebx
		ret
	}
}

// hover.exe:0x435650 (150 bytes) floor/ceiling span, shr 7 = 512-pixel
// texture; 4 pixels wide (dword write), shade table g_dwShadeTable,
// register-resident loop (leaves g_dwDst/40/14 stale).
// FUNCTION: HOVER 0x435650
HOVER_NAKED Span4x1Lit_S7(void)
{
	_asm
	{
		push ebx
		mov  eax, g_dwSpanCount
		cdq
		push esi
		and  edx, 3
		push edi
		add  eax, edx
		shl  g_dwUStep, 2
		shl  g_dwVStep, 2
		sar  eax, 2
		mov  g_dwSpanCount, eax
		push ebp
		mov  eax, g_dwU
		and  eax, g_dwUMask
		mov  edx, g_dwV
		mov  edi, g_dwDst
		mov  ebp, g_dwTexels
		sub  ecx, ecx
		sub  edi, 4
	L435697:
		mov  ebx, eax
		add  eax, g_dwUStep
		shr  ebx, 0x10
		and  eax, g_dwUMask
		lea  esi, [ebp + ebx]
		mov  ebx, edx
		add  edx, g_dwVStep
		and  ebx, g_dwVMask
		shr  ebx, 7
		add  edi, 4
		sub  ecx, ecx
		mov  cl, byte ptr [esi + ebx]
		mov  ebx, g_dwShadeTable
		mov  cl, byte ptr [ebx + ecx]
		mov  ch, cl
		push cx
		shl  ecx, 0x10
		pop  cx
		dec  g_dwSpanCount
		mov  dword ptr [edi], ecx
		jg   L435697
		pop  ebp
		pop  edi
		pop  esi
		pop  ebx
		ret
	}
}

// hover.exe:0x4356F0 (150 bytes) floor/ceiling span, shr 15 = 2-pixel
// texture; 4 pixels wide (dword write), shade table g_dwShadeTable,
// register-resident loop (leaves g_dwDst/40/14 stale).
// FUNCTION: HOVER 0x4356f0
HOVER_NAKED Span4x1Lit_S15(void)
{
	_asm
	{
		push ebx
		mov  eax, g_dwSpanCount
		cdq
		push esi
		and  edx, 3
		push edi
		add  eax, edx
		shl  g_dwUStep, 2
		shl  g_dwVStep, 2
		sar  eax, 2
		mov  g_dwSpanCount, eax
		push ebp
		mov  eax, g_dwU
		and  eax, g_dwUMask
		mov  edx, g_dwV
		mov  edi, g_dwDst
		mov  ebp, g_dwTexels
		sub  ecx, ecx
		sub  edi, 4
	L435737:
		mov  ebx, eax
		add  eax, g_dwUStep
		shr  ebx, 0x10
		and  eax, g_dwUMask
		lea  esi, [ebp + ebx]
		mov  ebx, edx
		add  edx, g_dwVStep
		and  ebx, g_dwVMask
		shr  ebx, 0xf
		add  edi, 4
		sub  ecx, ecx
		mov  cl, byte ptr [esi + ebx]
		mov  ebx, g_dwShadeTable
		mov  cl, byte ptr [ebx + ecx]
		mov  ch, cl
		push cx
		shl  ecx, 0x10
		pop  cx
		dec  g_dwSpanCount
		mov  dword ptr [edi], ecx
		jg   L435737
		pop  ebp
		pop  edi
		pop  esi
		pop  ebx
		ret
	}
}

// hover.exe:0x435790 (139 bytes) floor/ceiling span, shr 6 = 1024-pixel
// texture; 4 pixels wide (dword write), no shade table, register-resident
// loop (leaves g_dwDst/40/14 stale).
// FUNCTION: HOVER 0x435790
HOVER_NAKED Span4x1_S6(void)
{
	_asm
	{
		push ebx
		mov  eax, g_dwSpanCount
		cdq
		push esi
		and  edx, 3
		push edi
		add  eax, edx
		shl  g_dwUStep, 2
		shl  g_dwVStep, 2
		sar  eax, 2
		mov  g_dwSpanCount, eax
		push ebp
		mov  eax, g_dwU
		and  eax, g_dwUMask
		mov  edx, g_dwV
		mov  edi, g_dwDst
		mov  ebp, g_dwTexels
		sub  ecx, ecx
		sub  edi, 4
	L4357D7:
		mov  ebx, eax
		add  eax, g_dwUStep
		shr  ebx, 0x10
		and  eax, g_dwUMask
		lea  esi, [ebp + ebx]
		mov  ebx, edx
		add  edx, g_dwVStep
		and  ebx, g_dwVMask
		shr  ebx, 6
		add  edi, 4
		mov  cl, byte ptr [esi + ebx]
		mov  ch, cl
		push cx
		shl  ecx, 0x10
		pop  cx
		dec  g_dwSpanCount
		mov  dword ptr [edi], ecx
		jg   L4357D7
		pop  ebp
		pop  edi
		pop  esi
		pop  ebx
		ret
	}
}

// hover.exe:0x435820 (139 bytes) floor/ceiling span, shr 7 = 512-pixel
// texture; 4 pixels wide (dword write), no shade table, register-resident
// loop (leaves g_dwDst/40/14 stale).
// FUNCTION: HOVER 0x435820
HOVER_NAKED Span4x1_S7(void)
{
	_asm
	{
		push ebx
		mov  eax, g_dwSpanCount
		cdq
		push esi
		and  edx, 3
		push edi
		add  eax, edx
		shl  g_dwUStep, 2
		shl  g_dwVStep, 2
		sar  eax, 2
		mov  g_dwSpanCount, eax
		push ebp
		mov  eax, g_dwU
		and  eax, g_dwUMask
		mov  edx, g_dwV
		mov  edi, g_dwDst
		mov  ebp, g_dwTexels
		sub  ecx, ecx
		sub  edi, 4
	L435867:
		mov  ebx, eax
		add  eax, g_dwUStep
		shr  ebx, 0x10
		and  eax, g_dwUMask
		lea  esi, [ebp + ebx]
		mov  ebx, edx
		add  edx, g_dwVStep
		and  ebx, g_dwVMask
		shr  ebx, 7
		add  edi, 4
		mov  cl, byte ptr [esi + ebx]
		mov  ch, cl
		push cx
		shl  ecx, 0x10
		pop  cx
		dec  g_dwSpanCount
		mov  dword ptr [edi], ecx
		jg   L435867
		pop  ebp
		pop  edi
		pop  esi
		pop  ebx
		ret
	}
}

// hover.exe:0x4358B0 (139 bytes) floor/ceiling span, shr 8 = 256-pixel
// texture; 4 pixels wide (dword write), no shade table, register-resident
// loop (leaves g_dwDst/40/14 stale).
// FUNCTION: HOVER 0x4358b0
HOVER_NAKED Span4x1_S8(void)
{
	_asm
	{
		push ebx
		mov  eax, g_dwSpanCount
		cdq
		push esi
		and  edx, 3
		push edi
		add  eax, edx
		shl  g_dwUStep, 2
		shl  g_dwVStep, 2
		sar  eax, 2
		mov  g_dwSpanCount, eax
		push ebp
		mov  eax, g_dwU
		and  eax, g_dwUMask
		mov  edx, g_dwV
		mov  edi, g_dwDst
		mov  ebp, g_dwTexels
		sub  ecx, ecx
		sub  edi, 4
	L4358F7:
		mov  ebx, eax
		add  eax, g_dwUStep
		shr  ebx, 0x10
		and  eax, g_dwUMask
		lea  esi, [ebp + ebx]
		mov  ebx, edx
		add  edx, g_dwVStep
		and  ebx, g_dwVMask
		shr  ebx, 8
		add  edi, 4
		mov  cl, byte ptr [esi + ebx]
		mov  ch, cl
		push cx
		shl  ecx, 0x10
		pop  cx
		dec  g_dwSpanCount
		mov  dword ptr [edi], ecx
		jg   L4358F7
		pop  ebp
		pop  edi
		pop  esi
		pop  ebx
		ret
	}
}

// hover.exe:0x435940 (139 bytes) floor/ceiling span, shr 9 = 128-pixel
// texture; 4 pixels wide (dword write), no shade table, register-resident
// loop (leaves g_dwDst/40/14 stale).
// FUNCTION: HOVER 0x435940
HOVER_NAKED Span4x1_S9(void)
{
	_asm
	{
		push ebx
		mov  eax, g_dwSpanCount
		cdq
		push esi
		and  edx, 3
		push edi
		add  eax, edx
		shl  g_dwUStep, 2
		shl  g_dwVStep, 2
		sar  eax, 2
		mov  g_dwSpanCount, eax
		push ebp
		mov  eax, g_dwU
		and  eax, g_dwUMask
		mov  edx, g_dwV
		mov  edi, g_dwDst
		mov  ebp, g_dwTexels
		sub  ecx, ecx
		sub  edi, 4
	L435987:
		mov  ebx, eax
		add  eax, g_dwUStep
		shr  ebx, 0x10
		and  eax, g_dwUMask
		lea  esi, [ebp + ebx]
		mov  ebx, edx
		add  edx, g_dwVStep
		and  ebx, g_dwVMask
		shr  ebx, 9
		add  edi, 4
		mov  cl, byte ptr [esi + ebx]
		mov  ch, cl
		push cx
		shl  ecx, 0x10
		pop  cx
		dec  g_dwSpanCount
		mov  dword ptr [edi], ecx
		jg   L435987
		pop  ebp
		pop  edi
		pop  esi
		pop  ebx
		ret
	}
}

// hover.exe:0x4359D0 (139 bytes) floor/ceiling span, shr 10 = 64-pixel
// texture; 4 pixels wide (dword write), no shade table, register-resident
// loop (leaves g_dwDst/40/14 stale).
// FUNCTION: HOVER 0x4359d0
HOVER_NAKED Span4x1_S10(void)
{
	_asm
	{
		push ebx
		mov  eax, g_dwSpanCount
		cdq
		push esi
		and  edx, 3
		push edi
		add  eax, edx
		shl  g_dwUStep, 2
		shl  g_dwVStep, 2
		sar  eax, 2
		mov  g_dwSpanCount, eax
		push ebp
		mov  eax, g_dwU
		and  eax, g_dwUMask
		mov  edx, g_dwV
		mov  edi, g_dwDst
		mov  ebp, g_dwTexels
		sub  ecx, ecx
		sub  edi, 4
	L435A17:
		mov  ebx, eax
		add  eax, g_dwUStep
		shr  ebx, 0x10
		and  eax, g_dwUMask
		lea  esi, [ebp + ebx]
		mov  ebx, edx
		add  edx, g_dwVStep
		and  ebx, g_dwVMask
		shr  ebx, 0xa
		add  edi, 4
		mov  cl, byte ptr [esi + ebx]
		mov  ch, cl
		push cx
		shl  ecx, 0x10
		pop  cx
		dec  g_dwSpanCount
		mov  dword ptr [edi], ecx
		jg   L435A17
		pop  ebp
		pop  edi
		pop  esi
		pop  ebx
		ret
	}
}

// hover.exe:0x435A60 (139 bytes) floor/ceiling span, shr 11 = 32-pixel
// texture; 4 pixels wide (dword write), no shade table, register-resident
// loop (leaves g_dwDst/40/14 stale).
// FUNCTION: HOVER 0x435a60
HOVER_NAKED Span4x1_S11(void)
{
	_asm
	{
		push ebx
		mov  eax, g_dwSpanCount
		cdq
		push esi
		and  edx, 3
		push edi
		add  eax, edx
		shl  g_dwUStep, 2
		shl  g_dwVStep, 2
		sar  eax, 2
		mov  g_dwSpanCount, eax
		push ebp
		mov  eax, g_dwU
		and  eax, g_dwUMask
		mov  edx, g_dwV
		mov  edi, g_dwDst
		mov  ebp, g_dwTexels
		sub  ecx, ecx
		sub  edi, 4
	L435AA7:
		mov  ebx, eax
		add  eax, g_dwUStep
		shr  ebx, 0x10
		and  eax, g_dwUMask
		lea  esi, [ebp + ebx]
		mov  ebx, edx
		add  edx, g_dwVStep
		and  ebx, g_dwVMask
		shr  ebx, 0xb
		add  edi, 4
		mov  cl, byte ptr [esi + ebx]
		mov  ch, cl
		push cx
		shl  ecx, 0x10
		pop  cx
		dec  g_dwSpanCount
		mov  dword ptr [edi], ecx
		jg   L435AA7
		pop  ebp
		pop  edi
		pop  esi
		pop  ebx
		ret
	}
}

// hover.exe:0x435AF0 (139 bytes) floor/ceiling span, shr 12 = 16-pixel
// texture; 4 pixels wide (dword write), no shade table, register-resident
// loop (leaves g_dwDst/40/14 stale).
// FUNCTION: HOVER 0x435af0
HOVER_NAKED Span4x1_S12(void)
{
	_asm
	{
		push ebx
		mov  eax, g_dwSpanCount
		cdq
		push esi
		and  edx, 3
		push edi
		add  eax, edx
		shl  g_dwUStep, 2
		shl  g_dwVStep, 2
		sar  eax, 2
		mov  g_dwSpanCount, eax
		push ebp
		mov  eax, g_dwU
		and  eax, g_dwUMask
		mov  edx, g_dwV
		mov  edi, g_dwDst
		mov  ebp, g_dwTexels
		sub  ecx, ecx
		sub  edi, 4
	L435B37:
		mov  ebx, eax
		add  eax, g_dwUStep
		shr  ebx, 0x10
		and  eax, g_dwUMask
		lea  esi, [ebp + ebx]
		mov  ebx, edx
		add  edx, g_dwVStep
		and  ebx, g_dwVMask
		shr  ebx, 0xc
		add  edi, 4
		mov  cl, byte ptr [esi + ebx]
		mov  ch, cl
		push cx
		shl  ecx, 0x10
		pop  cx
		dec  g_dwSpanCount
		mov  dword ptr [edi], ecx
		jg   L435B37
		pop  ebp
		pop  edi
		pop  esi
		pop  ebx
		ret
	}
}

// hover.exe:0x435B80 (139 bytes) floor/ceiling span, shr 13 = 8-pixel
// texture; 4 pixels wide (dword write), no shade table, register-resident
// loop (leaves g_dwDst/40/14 stale).
// FUNCTION: HOVER 0x435b80
HOVER_NAKED Span4x1_S13(void)
{
	_asm
	{
		push ebx
		mov  eax, g_dwSpanCount
		cdq
		push esi
		and  edx, 3
		push edi
		add  eax, edx
		shl  g_dwUStep, 2
		shl  g_dwVStep, 2
		sar  eax, 2
		mov  g_dwSpanCount, eax
		push ebp
		mov  eax, g_dwU
		and  eax, g_dwUMask
		mov  edx, g_dwV
		mov  edi, g_dwDst
		mov  ebp, g_dwTexels
		sub  ecx, ecx
		sub  edi, 4
	L435BC7:
		mov  ebx, eax
		add  eax, g_dwUStep
		shr  ebx, 0x10
		and  eax, g_dwUMask
		lea  esi, [ebp + ebx]
		mov  ebx, edx
		add  edx, g_dwVStep
		and  ebx, g_dwVMask
		shr  ebx, 0xd
		add  edi, 4
		mov  cl, byte ptr [esi + ebx]
		mov  ch, cl
		push cx
		shl  ecx, 0x10
		pop  cx
		dec  g_dwSpanCount
		mov  dword ptr [edi], ecx
		jg   L435BC7
		pop  ebp
		pop  edi
		pop  esi
		pop  ebx
		ret
	}
}

// hover.exe:0x435C10 (139 bytes) floor/ceiling span, shr 14 = 4-pixel
// texture; 4 pixels wide (dword write), no shade table, register-resident
// loop (leaves g_dwDst/40/14 stale).
// FUNCTION: HOVER 0x435c10
HOVER_NAKED Span4x1_S14(void)
{
	_asm
	{
		push ebx
		mov  eax, g_dwSpanCount
		cdq
		push esi
		and  edx, 3
		push edi
		add  eax, edx
		shl  g_dwUStep, 2
		shl  g_dwVStep, 2
		sar  eax, 2
		mov  g_dwSpanCount, eax
		push ebp
		mov  eax, g_dwU
		and  eax, g_dwUMask
		mov  edx, g_dwV
		mov  edi, g_dwDst
		mov  ebp, g_dwTexels
		sub  ecx, ecx
		sub  edi, 4
	L435C57:
		mov  ebx, eax
		add  eax, g_dwUStep
		shr  ebx, 0x10
		and  eax, g_dwUMask
		lea  esi, [ebp + ebx]
		mov  ebx, edx
		add  edx, g_dwVStep
		and  ebx, g_dwVMask
		shr  ebx, 0xe
		add  edi, 4
		mov  cl, byte ptr [esi + ebx]
		mov  ch, cl
		push cx
		shl  ecx, 0x10
		pop  cx
		dec  g_dwSpanCount
		mov  dword ptr [edi], ecx
		jg   L435C57
		pop  ebp
		pop  edi
		pop  esi
		pop  ebx
		ret
	}
}

// hover.exe:0x435CA0 (139 bytes) floor/ceiling span, shr 15 = 2-pixel
// texture; 4 pixels wide (dword write), no shade table, register-resident
// loop (leaves g_dwDst/40/14 stale).
// FUNCTION: HOVER 0x435ca0
HOVER_NAKED Span4x1_S15(void)
{
	_asm
	{
		push ebx
		mov  eax, g_dwSpanCount
		cdq
		push esi
		and  edx, 3
		push edi
		add  eax, edx
		shl  g_dwUStep, 2
		shl  g_dwVStep, 2
		sar  eax, 2
		mov  g_dwSpanCount, eax
		push ebp
		mov  eax, g_dwU
		and  eax, g_dwUMask
		mov  edx, g_dwV
		mov  edi, g_dwDst
		mov  ebp, g_dwTexels
		sub  ecx, ecx
		sub  edi, 4
	L435CE7:
		mov  ebx, eax
		add  eax, g_dwUStep
		shr  ebx, 0x10
		and  eax, g_dwUMask
		lea  esi, [ebp + ebx]
		mov  ebx, edx
		add  edx, g_dwVStep
		and  ebx, g_dwVMask
		shr  ebx, 0xf
		add  edi, 4
		mov  cl, byte ptr [esi + ebx]
		mov  ch, cl
		push cx
		shl  ecx, 0x10
		pop  cx
		dec  g_dwSpanCount
		mov  dword ptr [edi], ecx
		jg   L435CE7
		pop  ebp
		pop  edi
		pop  esi
		pop  ebx
		ret
	}
}
