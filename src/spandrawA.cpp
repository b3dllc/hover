// spandrawA.cpp : span drawers, group A
//
// 46 of the 167 hand-written span drawers that CRenderContext::SetMode
// (texload.cpp) installs in the render context's drawer slots. Each is a C
// function whose body is one `_asm` block; the compiler contributes only the
// ebx/esi/edi saves, the `ret`, and - in the drawers that halve or quarter
// the span - the leading C statements that rescale g_dwSpanCount and the
// texture steps. `push ebp` / `pop ebp` (or `pushad` / `popad`) inside the
// block is the original's own save of the registers VC 2.1 does not save.
//
// Three families live here; all of them read the renderer globals set up by
// the span builders in texheap.cpp and write through g_dwDst.
//
//   Wall column     one 16.16 texture coordinate (g_dwColumnV, stepped by
//                   g_dwUStep and wrapped with g_dwVMask), shaded
//                   through the light table g_dwShadeTable, destination
//                   advanced by g_dwDstRowStride (one framebuffer row) per
//                   texel: a vertical texture-mapped column.
//
//   Floor/ceiling   two coordinates - u = g_dwU & g_dwUMask
//   span            stepped by g_dwUStep (the texture column, taken
//                   from bit 16 up) and v = g_dwV & g_dwVMask
//                   stepped by g_dwVStep (the texture row, shifted right
//                   by the family's shift constant, which therefore selects
//                   the texture's row stride: shift 8 = 256, 9 = 128,
//                   0xA = 64, 0xB = 32, 0xC = 16, 0xD = 8, 0xE = 4 bytes).
//                   Shaded through g_dwShadeTable, destination advanced
//                   along the scanline.
//
//   Direct span     the unshaded variant: texture position g_dwSkyU
//                   stepped by g_dwUStep, the texel taken straight from
//                   g_dwTexels with no light-table lookup. These are the
//                   ones that save everything with `pushad`.
//
// Every family comes in a 1-, 2- or 4-byte destination pixel (the texel byte
// is replicated into ch and then into the high half of the register with the
// `push cx / shl ecx,0x10 / pop cx` idiom) crossed with 1, 2 or 4 destination
// rows per texel; the multi-row variants are the ones whose C prologue
// divides the span length and scales the steps to match.

#include "hover.h"
#include "texture.h"
#include "gameobj.h"
#include "texheap.h"
#include "spandraw.h"


// hover.exe:0x40D600
// wall column, 4-byte pixel, 2 rows per texel
// FUNCTION: HOVER 0x40d600
void Column4x2LitReg(void)
{
	g_dwSpanCount = (DWORD)((int)g_dwSpanCount / 2);
	g_dwUStep <<= 1;

	if ((int)g_dwSpanCount > 0)
	{
		_asm
		{
			push   ebp
			mov    ebx, g_dwUStep
			mov    edi, g_dwDst
			mov    esi, g_dwTexels
			mov    edx, g_dwColumnV
			mov    ebp, g_dwShadeTable
			sub    edi, g_dwDstRowStride
			sub    eax, eax
		L1:
			mov    ecx, edx
			add    edx, ebx
			shr    ecx, 0x10
			add    edi, g_dwDstRowStride
			and    edx, g_dwVMask
			mov    al, byte ptr [esi + ecx]
			mov    cl, byte ptr [ebp + eax]
			mov    ch, cl
			push   cx
			shl    ecx, 0x10
			pop    cx
			mov    dword ptr [edi], ecx
			add    edi, g_dwDstRowStride
			dec    g_dwSpanCount
			mov    dword ptr [edi], ecx
			jg     L1
			pop    ebp
		}
	}
}

// hover.exe:0x40D680
// direct (unmapped) span, 4-byte pixel, 2 rows per texel
// FUNCTION: HOVER 0x40d680
void Sky4x2Reg(void)
{
	_asm
	{
		pushad
		mov    edi, g_dwDst
		mov    edx, g_dwSkyU
		mov    ebx, g_dwUStep
		mov    esi, g_dwTexels
		mov    ecx, g_dwDstRowStride
		mov    ebp, g_dwSpanCount
		test   ebp, ebp
		jle    L1
		sub    edi, 4
	L2:
		mov    eax, edx
		shr    eax, 0x10
		add    edx, ebx
		mov    al, byte ptr [esi + eax]
		add    edi, 4
		mov    ah, al
		push   ax
		rol    eax, 0x10
		pop    ax
		mov    dword ptr [edi], eax
		mov    dword ptr [edi + ecx], eax
		dec    ebp
		jne    L2
	L1:
		popad
	}
}

// hover.exe:0x40D900
// floor/ceiling span, 4-byte pixel, 2 rows, v shift 0xA (row stride 64)
// FUNCTION: HOVER 0x40d900
void Span4x2Lit_S10(void)
{
	g_dwSpanCount = (DWORD)((int)g_dwSpanCount / 4);
	g_dwUStep <<= 2;
	g_dwVStep <<= 2;

	_asm
	{
		push   ebp
		mov    eax, g_dwU
		and    eax, g_dwUMask
		mov    edx, g_dwV
		mov    edi, g_dwDst
		mov    ebp, g_dwTexels
		sub    ecx, ecx
		sub    edi, 4
	L1:
		mov    ebx, eax
		add    eax, g_dwUStep
		shr    ebx, 0x10
		and    eax, g_dwUMask
		lea    esi, [ebp + ebx]
		mov    ebx, edx
		add    edx, g_dwVStep
		and    ebx, g_dwVMask
		shr    ebx, 0xa
		add    edi, 4
		sub    ecx, ecx
		mov    cl, byte ptr [esi + ebx]
		mov    ebx, g_dwShadeTable
		mov    cl, byte ptr [ebx + ecx]
		mov    ch, cl
		push   cx
		shl    ecx, 0x10
		pop    cx
		dec    g_dwSpanCount
		mov    dword ptr [edi], ecx
		mov    ebx, g_dwDstRowStride
		mov    dword ptr [edi + ebx], ecx
		jg     L1
		pop    ebp
	}
}

// hover.exe:0x40D9A0
// floor/ceiling span, 4-byte pixel, 2 rows, v shift 0xB (row stride 32)
// FUNCTION: HOVER 0x40d9a0
void Span4x2Lit_S11(void)
{
	g_dwSpanCount = (DWORD)((int)g_dwSpanCount / 4);
	g_dwUStep <<= 2;
	g_dwVStep <<= 2;

	_asm
	{
		push   ebp
		mov    eax, g_dwU
		and    eax, g_dwUMask
		mov    edx, g_dwV
		mov    edi, g_dwDst
		mov    ebp, g_dwTexels
		sub    ecx, ecx
		sub    edi, 4
	L1:
		mov    ebx, eax
		add    eax, g_dwUStep
		shr    ebx, 0x10
		and    eax, g_dwUMask
		lea    esi, [ebp + ebx]
		mov    ebx, edx
		add    edx, g_dwVStep
		and    ebx, g_dwVMask
		shr    ebx, 0xb
		add    edi, 4
		sub    ecx, ecx
		mov    cl, byte ptr [esi + ebx]
		mov    ebx, g_dwShadeTable
		mov    cl, byte ptr [ebx + ecx]
		mov    ch, cl
		push   cx
		shl    ecx, 0x10
		pop    cx
		dec    g_dwSpanCount
		mov    dword ptr [edi], ecx
		mov    ebx, g_dwDstRowStride
		mov    dword ptr [edi + ebx], ecx
		jg     L1
		pop    ebp
	}
}

// hover.exe:0x40DA40
// floor/ceiling span, 4-byte pixel, 2 rows, v shift 0xC (row stride 16)
// FUNCTION: HOVER 0x40da40
void Span4x2Lit_S12(void)
{
	g_dwSpanCount = (DWORD)((int)g_dwSpanCount / 4);
	g_dwUStep <<= 2;
	g_dwVStep <<= 2;

	_asm
	{
		push   ebp
		mov    eax, g_dwU
		and    eax, g_dwUMask
		mov    edx, g_dwV
		mov    edi, g_dwDst
		mov    ebp, g_dwTexels
		sub    ecx, ecx
		sub    edi, 4
	L1:
		mov    ebx, eax
		add    eax, g_dwUStep
		shr    ebx, 0x10
		and    eax, g_dwUMask
		lea    esi, [ebp + ebx]
		mov    ebx, edx
		add    edx, g_dwVStep
		and    ebx, g_dwVMask
		shr    ebx, 0xc
		add    edi, 4
		sub    ecx, ecx
		mov    cl, byte ptr [esi + ebx]
		mov    ebx, g_dwShadeTable
		mov    cl, byte ptr [ebx + ecx]
		mov    ch, cl
		push   cx
		shl    ecx, 0x10
		pop    cx
		dec    g_dwSpanCount
		mov    dword ptr [edi], ecx
		mov    ebx, g_dwDstRowStride
		mov    dword ptr [edi + ebx], ecx
		jg     L1
		pop    ebp
	}
}

// hover.exe:0x40DAE0
// floor/ceiling span, 4-byte pixel, 2 rows, v shift 0xD (row stride 8)
// FUNCTION: HOVER 0x40dae0
void Span4x2Lit_S13(void)
{
	g_dwSpanCount = (DWORD)((int)g_dwSpanCount / 4);
	g_dwUStep <<= 2;
	g_dwVStep <<= 2;

	_asm
	{
		push   ebp
		mov    eax, g_dwU
		and    eax, g_dwUMask
		mov    edx, g_dwV
		mov    edi, g_dwDst
		mov    ebp, g_dwTexels
		sub    ecx, ecx
		sub    edi, 4
	L1:
		mov    ebx, eax
		add    eax, g_dwUStep
		shr    ebx, 0x10
		and    eax, g_dwUMask
		lea    esi, [ebp + ebx]
		mov    ebx, edx
		add    edx, g_dwVStep
		and    ebx, g_dwVMask
		shr    ebx, 0xd
		add    edi, 4
		sub    ecx, ecx
		mov    cl, byte ptr [esi + ebx]
		mov    ebx, g_dwShadeTable
		mov    cl, byte ptr [ebx + ecx]
		mov    ch, cl
		push   cx
		shl    ecx, 0x10
		pop    cx
		dec    g_dwSpanCount
		mov    dword ptr [edi], ecx
		mov    ebx, g_dwDstRowStride
		mov    dword ptr [edi + ebx], ecx
		jg     L1
		pop    ebp
	}
}

// hover.exe:0x40DB80
// floor/ceiling span, 4-byte pixel, 2 rows, v shift 0xE (row stride 4)
// FUNCTION: HOVER 0x40db80
void Span4x2Lit_S14(void)
{
	g_dwSpanCount = (DWORD)((int)g_dwSpanCount / 4);
	g_dwUStep <<= 2;
	g_dwVStep <<= 2;

	_asm
	{
		push   ebp
		mov    eax, g_dwU
		and    eax, g_dwUMask
		mov    edx, g_dwV
		mov    edi, g_dwDst
		mov    ebp, g_dwTexels
		sub    ecx, ecx
		sub    edi, 4
	L1:
		mov    ebx, eax
		add    eax, g_dwUStep
		shr    ebx, 0x10
		and    eax, g_dwUMask
		lea    esi, [ebp + ebx]
		mov    ebx, edx
		add    edx, g_dwVStep
		and    ebx, g_dwVMask
		shr    ebx, 0xe
		add    edi, 4
		sub    ecx, ecx
		mov    cl, byte ptr [esi + ebx]
		mov    ebx, g_dwShadeTable
		mov    cl, byte ptr [ebx + ecx]
		mov    ch, cl
		push   cx
		shl    ecx, 0x10
		pop    cx
		dec    g_dwSpanCount
		mov    dword ptr [edi], ecx
		mov    ebx, g_dwDstRowStride
		mov    dword ptr [edi + ebx], ecx
		jg     L1
		pop    ebp
	}
}

// hover.exe:0x40DDA0
// wall column, 2-byte pixel, 2 rows per texel
// FUNCTION: HOVER 0x40dda0
void Column2x2LitReg(void)
{
	g_dwSpanCount = (DWORD)((int)g_dwSpanCount / 2);
	g_dwUStep <<= 1;

	if ((int)g_dwSpanCount > 0)
	{
		_asm
		{
			push   ebp
			mov    ebx, g_dwUStep
			mov    edi, g_dwDst
			mov    esi, g_dwTexels
			mov    edx, g_dwColumnV
			mov    ebp, g_dwShadeTable
			sub    edi, g_dwDstRowStride
			sub    eax, eax
		L1:
			mov    ecx, edx
			add    edx, ebx
			shr    ecx, 0x10
			add    edi, g_dwDstRowStride
			and    edx, g_dwVMask
			mov    al, byte ptr [esi + ecx]
			mov    cl, byte ptr [ebp + eax]
			mov    ch, cl
			mov    word ptr [edi], cx
			add    edi, g_dwDstRowStride
			dec    g_dwSpanCount
			mov    word ptr [edi], cx
			jg     L1
			pop    ebp
		}
	}
}

// hover.exe:0x40DE20
// direct (unmapped) span, 2-byte pixel, 2 rows per texel
// FUNCTION: HOVER 0x40de20
void Sky2x2Reg(void)
{
	_asm
	{
		pushad
		mov    edi, g_dwDst
		mov    edx, g_dwSkyU
		mov    ebx, g_dwUStep
		mov    esi, g_dwTexels
		mov    ecx, g_dwDstRowStride
		push   g_dwUMask
		mov    ebp, g_dwSpanCount
		test   ebp, ebp
		jle    L1
		sub    edi, 2
	L2:
		mov    eax, edx
		inc    edi
		shr    eax, 0x10
		and    eax, dword ptr [esp]
		add    edx, ebx
		mov    al, byte ptr [esi + eax]
		inc    edi
		mov    ah, al
		mov    word ptr [edi], ax
		mov    word ptr [edi + ecx], ax
		dec    ebp
		jne    L2
	L1:
		pop    eax
		popad
	}
}

// hover.exe:0x40DF80
// floor/ceiling span, 2-byte pixel, 2 rows, v shift 0xA (row stride 64)
// FUNCTION: HOVER 0x40df80
void Span2x2Lit_S10(void)
{
	g_dwSpanCount = (DWORD)((int)g_dwSpanCount / 2);
	g_dwUStep <<= 1;
	g_dwVStep <<= 1;

	_asm
	{
		push   ebp
		mov    eax, g_dwU
		and    eax, g_dwUMask
		mov    edx, g_dwV
		mov    edi, g_dwDst
		mov    ebp, g_dwTexels
		sub    ecx, ecx
		sub    edi, 2
	L1:
		mov    ebx, eax
		add    eax, g_dwUStep
		shr    ebx, 0x10
		and    eax, g_dwUMask
		lea    esi, [ebp + ebx]
		mov    ebx, edx
		add    edx, g_dwVStep
		and    ebx, g_dwVMask
		shr    ebx, 0xa
		add    edi, 2
		sub    ch, ch
		mov    cl, byte ptr [esi + ebx]
		mov    ebx, g_dwShadeTable
		mov    cl, byte ptr [ebx + ecx]
		dec    g_dwSpanCount
		mov    ch, cl
		mov    word ptr [edi], cx
		mov    ebx, g_dwDstRowStride
		mov    word ptr [edi + ebx], cx
		jg     L1
		pop    ebp
	}
}

// hover.exe:0x40E020
// floor/ceiling span, 2-byte pixel, 2 rows, v shift 0xB (row stride 32)
// FUNCTION: HOVER 0x40e020
void Span2x2Lit_S11(void)
{
	g_dwSpanCount = (DWORD)((int)g_dwSpanCount / 2);
	g_dwUStep <<= 1;
	g_dwVStep <<= 1;

	_asm
	{
		push   ebp
		mov    eax, g_dwU
		and    eax, g_dwUMask
		mov    edx, g_dwV
		mov    edi, g_dwDst
		mov    ebp, g_dwTexels
		sub    ecx, ecx
		sub    edi, 2
	L1:
		mov    ebx, eax
		add    eax, g_dwUStep
		shr    ebx, 0x10
		and    eax, g_dwUMask
		lea    esi, [ebp + ebx]
		mov    ebx, edx
		add    edx, g_dwVStep
		and    ebx, g_dwVMask
		shr    ebx, 0xb
		add    edi, 2
		sub    ch, ch
		mov    cl, byte ptr [esi + ebx]
		mov    ebx, g_dwShadeTable
		mov    cl, byte ptr [ebx + ecx]
		dec    g_dwSpanCount
		mov    ch, cl
		mov    word ptr [edi], cx
		mov    ebx, g_dwDstRowStride
		mov    word ptr [edi + ebx], cx
		jg     L1
		pop    ebp
	}
}

// hover.exe:0x40E0C0
// floor/ceiling span, 2-byte pixel, 2 rows, v shift 0xC (row stride 16)
// FUNCTION: HOVER 0x40e0c0
void Span2x2Lit_S12(void)
{
	g_dwSpanCount = (DWORD)((int)g_dwSpanCount / 2);
	g_dwUStep <<= 1;
	g_dwVStep <<= 1;

	_asm
	{
		push   ebp
		mov    eax, g_dwU
		and    eax, g_dwUMask
		mov    edx, g_dwV
		mov    edi, g_dwDst
		mov    ebp, g_dwTexels
		sub    ecx, ecx
		sub    edi, 2
	L1:
		mov    ebx, eax
		add    eax, g_dwUStep
		shr    ebx, 0x10
		and    eax, g_dwUMask
		lea    esi, [ebp + ebx]
		mov    ebx, edx
		add    edx, g_dwVStep
		and    ebx, g_dwVMask
		shr    ebx, 0xc
		add    edi, 2
		sub    ch, ch
		mov    cl, byte ptr [esi + ebx]
		mov    ebx, g_dwShadeTable
		mov    cl, byte ptr [ebx + ecx]
		dec    g_dwSpanCount
		mov    ch, cl
		mov    word ptr [edi], cx
		mov    ebx, g_dwDstRowStride
		mov    word ptr [edi + ebx], cx
		jg     L1
		pop    ebp
	}
}

// hover.exe:0x40E160
// floor/ceiling span, 2-byte pixel, 2 rows, v shift 0xD (row stride 8)
// FUNCTION: HOVER 0x40e160
void Span2x2Lit_S13(void)
{
	g_dwSpanCount = (DWORD)((int)g_dwSpanCount / 2);
	g_dwUStep <<= 1;
	g_dwVStep <<= 1;

	_asm
	{
		push   ebp
		mov    eax, g_dwU
		and    eax, g_dwUMask
		mov    edx, g_dwV
		mov    edi, g_dwDst
		mov    ebp, g_dwTexels
		sub    ecx, ecx
		sub    edi, 2
	L1:
		mov    ebx, eax
		add    eax, g_dwUStep
		shr    ebx, 0x10
		and    eax, g_dwUMask
		lea    esi, [ebp + ebx]
		mov    ebx, edx
		add    edx, g_dwVStep
		and    ebx, g_dwVMask
		shr    ebx, 0xd
		add    edi, 2
		sub    ch, ch
		mov    cl, byte ptr [esi + ebx]
		mov    ebx, g_dwShadeTable
		mov    cl, byte ptr [ebx + ecx]
		dec    g_dwSpanCount
		mov    ch, cl
		mov    word ptr [edi], cx
		mov    ebx, g_dwDstRowStride
		mov    word ptr [edi + ebx], cx
		jg     L1
		pop    ebp
	}
}

// hover.exe:0x40E200
// floor/ceiling span, 2-byte pixel, 2 rows, v shift 0xE (row stride 4)
// FUNCTION: HOVER 0x40e200
void Span2x2Lit_S14(void)
{
	g_dwSpanCount = (DWORD)((int)g_dwSpanCount / 2);
	g_dwUStep <<= 1;
	g_dwVStep <<= 1;

	_asm
	{
		push   ebp
		mov    eax, g_dwU
		and    eax, g_dwUMask
		mov    edx, g_dwV
		mov    edi, g_dwDst
		mov    ebp, g_dwTexels
		sub    ecx, ecx
		sub    edi, 2
	L1:
		mov    ebx, eax
		add    eax, g_dwUStep
		shr    ebx, 0x10
		and    eax, g_dwUMask
		lea    esi, [ebp + ebx]
		mov    ebx, edx
		add    edx, g_dwVStep
		and    ebx, g_dwVMask
		shr    ebx, 0xe
		add    edi, 2
		sub    ch, ch
		mov    cl, byte ptr [esi + ebx]
		mov    ebx, g_dwShadeTable
		mov    cl, byte ptr [ebx + ecx]
		dec    g_dwSpanCount
		mov    ch, cl
		mov    word ptr [edi], cx
		mov    ebx, g_dwDstRowStride
		mov    word ptr [edi + ebx], cx
		jg     L1
		pop    ebp
	}
}

// hover.exe:0x40E5A0
// wall column, 2-byte pixel, 1 row per texel
// FUNCTION: HOVER 0x40e5a0
void Column2x1LitReg(void)
{
	_asm
	{
		push   ebp
		mov    ebx, g_dwSpanCount
		mov    edi, g_dwDst
		mov    esi, g_dwTexels
		mov    edx, g_dwColumnV
		mov    ebp, g_dwShadeTable
		sub    edi, g_dwDstRowStride
		sub    eax, eax
	L1:
		mov    ecx, edx
		add    edx, g_dwUStep
		shr    ecx, 0x10
		add    edi, g_dwDstRowStride
		and    edx, g_dwVMask
		mov    al, byte ptr [esi + ecx]
		dec    ebx
		mov    cl, byte ptr [ebp + eax]
		mov    ch, cl
		mov    word ptr [edi], cx
		jg     L1
		pop    ebp
	}
}

// hover.exe:0x40E600
// direct (unmapped) span, 2-byte pixel, 1 row per texel
// FUNCTION: HOVER 0x40e600
void Sky2x1Reg(void)
{
	_asm
	{
		pushad
		mov    edi, g_dwDst
		mov    edx, g_dwSkyU
		mov    ebx, g_dwUStep
		mov    esi, g_dwTexels
		mov    ecx, g_dwDstRowStride
		mov    ebp, g_dwSpanCount
		test   ebp, ebp
		jle    L1
		sub    edi, 2
	L2:
		mov    eax, edx
		inc    edi
		shr    eax, 0x10
		add    edx, ebx
		mov    al, byte ptr [esi + eax]
		inc    edi
		mov    ah, al
		mov    word ptr [edi], ax
		dec    ebp
		jne    L2
	L1:
		popad
	}
}

// hover.exe:0x40E850
// floor/ceiling span, 2-byte pixel, 1 row, v shift 0xB (row stride 32)
// FUNCTION: HOVER 0x40e850
void Span2x1Lit_S11(void)
{
	g_dwSpanCount = (DWORD)((int)g_dwSpanCount / 2);
	g_dwUStep <<= 1;
	g_dwVStep <<= 1;

	_asm
	{
		push   ebp
		mov    eax, g_dwU
		and    eax, g_dwUMask
		mov    edx, g_dwV
		mov    edi, g_dwDst
		mov    ebp, g_dwTexels
		sub    ecx, ecx
		sub    edi, 2
	L1:
		mov    ebx, eax
		add    eax, g_dwUStep
		shr    ebx, 0x10
		and    eax, g_dwUMask
		lea    esi, [ebp + ebx]
		mov    ebx, edx
		add    edx, g_dwVStep
		and    ebx, g_dwVMask
		shr    ebx, 0xb
		add    edi, 2
		sub    ch, ch
		mov    cl, byte ptr [esi + ebx]
		mov    ebx, g_dwShadeTable
		mov    cl, byte ptr [ebx + ecx]
		dec    g_dwSpanCount
		mov    ch, cl
		mov    word ptr [edi], cx
		jg     L1
		pop    ebp
	}
}

// hover.exe:0x40E910
// floor/ceiling span, 2-byte pixel, 1 row, v shift 0xC (row stride 16)
// FUNCTION: HOVER 0x40e910
void Span2x1Lit_S12(void)
{
	g_dwSpanCount = (DWORD)((int)g_dwSpanCount / 2);
	g_dwUStep <<= 1;
	g_dwVStep <<= 1;

	_asm
	{
		push   ebp
		mov    eax, g_dwU
		and    eax, g_dwUMask
		mov    edx, g_dwV
		mov    edi, g_dwDst
		mov    ebp, g_dwTexels
		sub    ecx, ecx
		sub    edi, 2
	L1:
		mov    ebx, eax
		add    eax, g_dwUStep
		shr    ebx, 0x10
		and    eax, g_dwUMask
		lea    esi, [ebp + ebx]
		mov    ebx, edx
		add    edx, g_dwVStep
		and    ebx, g_dwVMask
		shr    ebx, 0xc
		add    edi, 2
		sub    ch, ch
		mov    cl, byte ptr [esi + ebx]
		mov    ebx, g_dwShadeTable
		mov    cl, byte ptr [ebx + ecx]
		dec    g_dwSpanCount
		mov    ch, cl
		mov    word ptr [edi], cx
		jg     L1
		pop    ebp
	}
}

// hover.exe:0x40EAC0
// floor/ceiling span, 2-byte pixel, 1 row, v shift 0xD (row stride 8)
// FUNCTION: HOVER 0x40eac0
void Span2x1Lit_S13(void)
{
	g_dwSpanCount = (DWORD)((int)g_dwSpanCount / 2);
	g_dwUStep <<= 1;
	g_dwVStep <<= 1;

	_asm
	{
		push   ebp
		mov    eax, g_dwU
		and    eax, g_dwUMask
		mov    edx, g_dwV
		mov    edi, g_dwDst
		mov    ebp, g_dwTexels
		sub    ecx, ecx
		sub    edi, 2
	L1:
		mov    ebx, eax
		add    eax, g_dwUStep
		shr    ebx, 0x10
		and    eax, g_dwUMask
		lea    esi, [ebp + ebx]
		mov    ebx, edx
		add    edx, g_dwVStep
		and    ebx, g_dwVMask
		shr    ebx, 0xd
		add    edi, 2
		sub    ch, ch
		mov    cl, byte ptr [esi + ebx]
		mov    ebx, g_dwShadeTable
		mov    cl, byte ptr [ebx + ecx]
		dec    g_dwSpanCount
		mov    ch, cl
		mov    word ptr [edi], cx
		jg     L1
		pop    ebp
	}
}

// hover.exe:0x40EDD0
// floor/ceiling span, 2-byte pixel, 1 row, v shift 0xE (row stride 4)
// FUNCTION: HOVER 0x40edd0
void Span2x1Lit_S14(void)
{
	g_dwSpanCount = (DWORD)((int)g_dwSpanCount / 2);
	g_dwUStep <<= 1;
	g_dwVStep <<= 1;

	_asm
	{
		push   ebp
		mov    eax, g_dwU
		and    eax, g_dwUMask
		mov    edx, g_dwV
		mov    edi, g_dwDst
		mov    ebp, g_dwTexels
		sub    ecx, ecx
		sub    edi, 2
	L1:
		mov    ebx, eax
		add    eax, g_dwUStep
		shr    ebx, 0x10
		and    eax, g_dwUMask
		lea    esi, [ebp + ebx]
		mov    ebx, edx
		add    edx, g_dwVStep
		and    ebx, g_dwVMask
		shr    ebx, 0xe
		add    edi, 2
		sub    ch, ch
		mov    cl, byte ptr [esi + ebx]
		mov    ebx, g_dwShadeTable
		mov    cl, byte ptr [ebx + ecx]
		dec    g_dwSpanCount
		mov    ch, cl
		mov    word ptr [edi], cx
		jg     L1
		pop    ebp
	}
}

// hover.exe:0x40F540
// floor/ceiling span, 2-byte pixel, 1 row, v shift 0x9 (row stride 128)
// FUNCTION: HOVER 0x40f540
void Span2x1Lit_S9(void)
{
	g_dwSpanCount = (DWORD)((int)g_dwSpanCount / 2);
	g_dwUStep <<= 1;
	g_dwVStep <<= 1;

	_asm
	{
		push   ebp
		mov    eax, g_dwU
		and    eax, g_dwUMask
		mov    edx, g_dwV
		mov    edi, g_dwDst
		mov    ebp, g_dwTexels
		sub    ecx, ecx
		sub    edi, 2
	L1:
		mov    ebx, eax
		add    eax, g_dwUStep
		shr    ebx, 0x10
		and    eax, g_dwUMask
		lea    esi, [ebp + ebx]
		mov    ebx, edx
		add    edx, g_dwVStep
		and    ebx, g_dwVMask
		shr    ebx, 9
		add    edi, 2
		sub    ch, ch
		mov    cl, byte ptr [esi + ebx]
		mov    ebx, g_dwShadeTable
		mov    cl, byte ptr [ebx + ecx]
		dec    g_dwSpanCount
		mov    ch, cl
		mov    word ptr [edi], cx
		jg     L1
		pop    ebp
	}
}

// hover.exe:0x40F5D0
// floor/ceiling span, 2-byte pixel, 1 row, v shift 0xA (row stride 64)
// FUNCTION: HOVER 0x40f5d0
void Span2x1Lit_S10(void)
{
	g_dwSpanCount = (DWORD)((int)g_dwSpanCount / 2);
	g_dwUStep <<= 1;
	g_dwVStep <<= 1;

	_asm
	{
		push   ebp
		mov    eax, g_dwU
		and    eax, g_dwUMask
		mov    edx, g_dwV
		mov    edi, g_dwDst
		mov    ebp, g_dwTexels
		sub    ecx, ecx
		sub    edi, 2
	L1:
		mov    ebx, eax
		add    eax, g_dwUStep
		shr    ebx, 0x10
		and    eax, g_dwUMask
		lea    esi, [ebp + ebx]
		mov    ebx, edx
		add    edx, g_dwVStep
		and    ebx, g_dwVMask
		shr    ebx, 0xa
		add    edi, 2
		sub    ch, ch
		mov    cl, byte ptr [esi + ebx]
		mov    ebx, g_dwShadeTable
		mov    cl, byte ptr [ebx + ecx]
		dec    g_dwSpanCount
		mov    ch, cl
		mov    word ptr [edi], cx
		jg     L1
		pop    ebp
	}
}

// hover.exe:0x40F6B0
// wall column, 1-byte pixel, 1 row per texel
// FUNCTION: HOVER 0x40f6b0
void Column1x1LitReg(void)
{
	_asm
	{
		push   ebp
		mov    ebx, g_dwSpanCount
		mov    edi, g_dwDst
		mov    esi, g_dwTexels
		mov    edx, g_dwColumnV
		mov    ebp, g_dwShadeTable
		sub    edi, g_dwDstRowStride
		sub    eax, eax
	L1:
		mov    ecx, edx
		add    edx, g_dwUStep
		shr    ecx, 0x10
		add    edi, g_dwDstRowStride
		and    edx, g_dwVMask
		mov    al, byte ptr [esi + ecx]
		dec    ebx
		mov    cl, byte ptr [ebp + eax]
		mov    byte ptr [edi], cl
		jg     L1
		pop    ebp
	}
}

// hover.exe:0x40F710
// direct (unmapped) span, 1-byte pixel, 1 row per texel
// FUNCTION: HOVER 0x40f710
void Sky1x1(void)
{
	_asm
	{
		pushad
		mov    edi, g_dwDst
		mov    edx, g_dwSkyU
		mov    ebx, g_dwUStep
		mov    esi, g_dwTexels
		mov    ecx, g_dwUMask
		mov    ebp, g_dwSpanCount
		test   ebp, ebp
		jle    L1
		dec    edi
	L2:
		mov    eax, edx
		inc    edi
		shr    eax, 0x10
		add    edx, ebx
		and    eax, ecx
		mov    al, byte ptr [esi + eax]
		mov    byte ptr [edi], al
		dec    ebp
		jne    L2
	L1:
		popad
	}
}

// hover.exe:0x40FA90
// floor/ceiling span, 1-byte pixel, 1 row, v shift 0x9 (row stride 128)
// FUNCTION: HOVER 0x40fa90
void Span1x1Lit_S9(void)
{
	_asm
	{
		push   ebp
		mov    eax, g_dwU
		and    eax, g_dwUMask
		mov    edx, g_dwV
		mov    edi, g_dwDst
		mov    ebp, g_dwTexels
		sub    ecx, ecx
		dec    edi
	L1:
		mov    ebx, eax
		add    eax, g_dwUStep
		shr    ebx, 0x10
		and    eax, g_dwUMask
		lea    esi, [ebp + ebx]
		mov    ebx, edx
		add    edx, g_dwVStep
		and    ebx, g_dwVMask
		shr    ebx, 9
		inc    edi
		mov    cl, byte ptr [esi + ebx]
		mov    ebx, g_dwShadeTable
		mov    cl, byte ptr [ebx + ecx]
		dec    g_dwSpanCount
		mov    byte ptr [edi], cl
		jg     L1
		pop    ebp
	}
}

// hover.exe:0x40FB40
// floor/ceiling span, 1-byte pixel, 1 row, v shift 0xA (row stride 64)
// FUNCTION: HOVER 0x40fb40
void Span1x1Lit_S10(void)
{
	_asm
	{
		push   ebp
		mov    eax, g_dwU
		and    eax, g_dwUMask
		mov    edx, g_dwV
		mov    edi, g_dwDst
		mov    ebp, g_dwTexels
		sub    ecx, ecx
		dec    edi
	L1:
		mov    ebx, eax
		add    eax, g_dwUStep
		shr    ebx, 0x10
		and    eax, g_dwUMask
		lea    esi, [ebp + ebx]
		mov    ebx, edx
		add    edx, g_dwVStep
		and    ebx, g_dwVMask
		shr    ebx, 0xa
		inc    edi
		mov    cl, byte ptr [esi + ebx]
		mov    ebx, g_dwShadeTable
		mov    cl, byte ptr [ebx + ecx]
		dec    g_dwSpanCount
		mov    byte ptr [edi], cl
		jg     L1
		pop    ebp
	}
}

// hover.exe:0x40FC70
// floor/ceiling span, 1-byte pixel, 1 row, v shift 0xB (row stride 32)
// FUNCTION: HOVER 0x40fc70
void Span1x1Lit_S11(void)
{
	_asm
	{
		push   ebp
		mov    eax, g_dwU
		and    eax, g_dwUMask
		mov    edx, g_dwV
		mov    edi, g_dwDst
		mov    ebp, g_dwTexels
		sub    ecx, ecx
		dec    edi
	L1:
		mov    ebx, eax
		add    eax, g_dwUStep
		shr    ebx, 0x10
		and    eax, g_dwUMask
		lea    esi, [ebp + ebx]
		mov    ebx, edx
		add    edx, g_dwVStep
		and    ebx, g_dwVMask
		shr    ebx, 0xb
		inc    edi
		mov    cl, byte ptr [esi + ebx]
		mov    ebx, g_dwShadeTable
		mov    cl, byte ptr [ebx + ecx]
		dec    g_dwSpanCount
		mov    byte ptr [edi], cl
		jg     L1
		pop    ebp
	}
}

// hover.exe:0x40FDD0
// floor/ceiling span, 1-byte pixel, 1 row, v shift 0xC (row stride 16)
// FUNCTION: HOVER 0x40fdd0
void Span1x1Lit_S12(void)
{
	_asm
	{
		push   ebp
		mov    eax, g_dwU
		and    eax, g_dwUMask
		mov    edx, g_dwV
		mov    edi, g_dwDst
		mov    ebp, g_dwTexels
		sub    ecx, ecx
		dec    edi
	L1:
		mov    ebx, eax
		add    eax, g_dwUStep
		shr    ebx, 0x10
		and    eax, g_dwUMask
		lea    esi, [ebp + ebx]
		mov    ebx, edx
		add    edx, g_dwVStep
		and    ebx, g_dwVMask
		shr    ebx, 0xc
		inc    edi
		mov    cl, byte ptr [esi + ebx]
		mov    ebx, g_dwShadeTable
		mov    cl, byte ptr [ebx + ecx]
		dec    g_dwSpanCount
		mov    byte ptr [edi], cl
		jg     L1
		pop    ebp
	}
}

// hover.exe:0x40FE70
// floor/ceiling span, 1-byte pixel, 1 row, v shift 0xD (row stride 8)
// FUNCTION: HOVER 0x40fe70
void Span1x1Lit_S13(void)
{
	_asm
	{
		push   ebp
		mov    eax, g_dwU
		and    eax, g_dwUMask
		mov    edx, g_dwV
		mov    edi, g_dwDst
		mov    ebp, g_dwTexels
		sub    ecx, ecx
		dec    edi
	L1:
		mov    ebx, eax
		add    eax, g_dwUStep
		shr    ebx, 0x10
		and    eax, g_dwUMask
		lea    esi, [ebp + ebx]
		mov    ebx, edx
		add    edx, g_dwVStep
		and    ebx, g_dwVMask
		shr    ebx, 0xd
		inc    edi
		mov    cl, byte ptr [esi + ebx]
		mov    ebx, g_dwShadeTable
		mov    cl, byte ptr [ebx + ecx]
		dec    g_dwSpanCount
		mov    byte ptr [edi], cl
		jg     L1
		pop    ebp
	}
}

// hover.exe:0x4101E0
// floor/ceiling span, 1-byte pixel, 1 row, v shift 0xE (row stride 4)
// FUNCTION: HOVER 0x4101e0
void Span1x1Lit_S14(void)
{
	_asm
	{
		push   ebp
		mov    eax, g_dwU
		and    eax, g_dwUMask
		mov    edx, g_dwV
		mov    edi, g_dwDst
		mov    ebp, g_dwTexels
		sub    ecx, ecx
		dec    edi
	L1:
		mov    ebx, eax
		add    eax, g_dwUStep
		shr    ebx, 0x10
		and    eax, g_dwUMask
		lea    esi, [ebp + ebx]
		mov    ebx, edx
		add    edx, g_dwVStep
		and    ebx, g_dwVMask
		shr    ebx, 0xe
		inc    edi
		mov    cl, byte ptr [esi + ebx]
		mov    ebx, g_dwShadeTable
		mov    cl, byte ptr [ebx + ecx]
		dec    g_dwSpanCount
		mov    byte ptr [edi], cl
		jg     L1
		pop    ebp
	}
}

// hover.exe:0x4114D0
// wall column, 4-byte pixel, 4 rows per texel
// FUNCTION: HOVER 0x4114d0
void Column4x4LitReg(void)
{
	g_dwSpanCount = (DWORD)((int)g_dwSpanCount / 4);
	g_dwUStep <<= 2;

	if ((int)g_dwSpanCount > 0)
	{
		_asm
		{
			push   ebp
			mov    ebx, g_dwUStep
			mov    edi, g_dwDst
			mov    esi, g_dwTexels
			mov    edx, g_dwColumnV
			mov    ebp, g_dwShadeTable
			sub    edi, g_dwDstRowStride
			sub    eax, eax
		L1:
			mov    ecx, edx
			add    edx, ebx
			shr    ecx, 0x10
			add    edi, g_dwDstRowStride
			and    edx, g_dwVMask
			mov    al, byte ptr [esi + ecx]
			mov    cl, byte ptr [ebp + eax]
			mov    ch, cl
			push   cx
			shl    ecx, 0x10
			pop    cx
			mov    dword ptr [edi], ecx
			add    edi, g_dwDstRowStride
			mov    dword ptr [edi], ecx
			add    edi, g_dwDstRowStride
			mov    dword ptr [edi], ecx
			add    edi, g_dwDstRowStride
			dec    g_dwSpanCount
			mov    dword ptr [edi], ecx
			jg     L1
			pop    ebp
		}
	}
}

// hover.exe:0x411940
// floor/ceiling span, 4-byte pixel, 4 rows, v shift 0xB (row stride 32)
// FUNCTION: HOVER 0x411940
void Span4x4Lit_S11(void)
{
	g_dwSpanCount = (DWORD)((int)g_dwSpanCount / 4);
	g_dwUStep <<= 2;
	g_dwVStep <<= 2;

	_asm
	{
		push   ebp
		mov    eax, g_dwU
		and    eax, g_dwUMask
		mov    edx, g_dwV
		mov    edi, g_dwDst
		mov    ebp, g_dwTexels
		sub    ecx, ecx
		sub    edi, 4
	L1:
		mov    ebx, eax
		add    eax, g_dwUStep
		shr    ebx, 0x10
		and    eax, g_dwUMask
		lea    esi, [ebp + ebx]
		mov    ebx, edx
		add    edx, g_dwVStep
		and    ebx, g_dwVMask
		shr    ebx, 0xb
		add    edi, 4
		sub    ecx, ecx
		mov    cl, byte ptr [esi + ebx]
		mov    ebx, g_dwShadeTable
		mov    cl, byte ptr [ebx + ecx]
		mov    ch, cl
		push   cx
		shl    ecx, 0x10
		pop    cx
		mov    dword ptr [edi], ecx
		mov    ebx, g_dwDstRowStride
		mov    dword ptr [edi + ebx], ecx
		mov    dword ptr [edi + ebx*2], ecx
		add    ebx, g_dwDstRowStride
		add    ebx, g_dwDstRowStride
		mov    dword ptr [edi + ebx], ecx
		dec    g_dwSpanCount
		jg     L1
		pop    ebp
	}
}

// hover.exe:0x411FE0
// floor/ceiling span, 4-byte pixel, 4 rows, v shift 0xC (row stride 16)
// FUNCTION: HOVER 0x411fe0
void Span4x4Lit_S12(void)
{
	g_dwSpanCount = (DWORD)((int)g_dwSpanCount / 4);
	g_dwUStep <<= 2;
	g_dwVStep <<= 2;

	_asm
	{
		push   ebp
		mov    eax, g_dwU
		and    eax, g_dwUMask
		mov    edx, g_dwV
		mov    edi, g_dwDst
		mov    ebp, g_dwTexels
		sub    ecx, ecx
		sub    edi, 4
	L1:
		mov    ebx, eax
		add    eax, g_dwUStep
		shr    ebx, 0x10
		and    eax, g_dwUMask
		lea    esi, [ebp + ebx]
		mov    ebx, edx
		add    edx, g_dwVStep
		and    ebx, g_dwVMask
		shr    ebx, 0xc
		add    edi, 4
		sub    ecx, ecx
		mov    cl, byte ptr [esi + ebx]
		mov    ebx, g_dwShadeTable
		mov    cl, byte ptr [ebx + ecx]
		mov    ch, cl
		push   cx
		shl    ecx, 0x10
		pop    cx
		mov    dword ptr [edi], ecx
		mov    ebx, g_dwDstRowStride
		mov    dword ptr [edi + ebx], ecx
		mov    dword ptr [edi + ebx*2], ecx
		add    ebx, g_dwDstRowStride
		add    ebx, g_dwDstRowStride
		mov    dword ptr [edi + ebx], ecx
		dec    g_dwSpanCount
		jg     L1
		pop    ebp
	}
}

// hover.exe:0x4120A0
// floor/ceiling span, 4-byte pixel, 4 rows, v shift 0xD (row stride 8)
// FUNCTION: HOVER 0x4120a0
void Span4x4Lit_S13(void)
{
	g_dwSpanCount = (DWORD)((int)g_dwSpanCount / 4);
	g_dwUStep <<= 2;
	g_dwVStep <<= 2;

	_asm
	{
		push   ebp
		mov    eax, g_dwU
		and    eax, g_dwUMask
		mov    edx, g_dwV
		mov    edi, g_dwDst
		mov    ebp, g_dwTexels
		sub    ecx, ecx
		sub    edi, 4
	L1:
		mov    ebx, eax
		add    eax, g_dwUStep
		shr    ebx, 0x10
		and    eax, g_dwUMask
		lea    esi, [ebp + ebx]
		mov    ebx, edx
		add    edx, g_dwVStep
		and    ebx, g_dwVMask
		shr    ebx, 0xd
		add    edi, 4
		sub    ecx, ecx
		mov    cl, byte ptr [esi + ebx]
		mov    ebx, g_dwShadeTable
		mov    cl, byte ptr [ebx + ecx]
		mov    ch, cl
		push   cx
		shl    ecx, 0x10
		pop    cx
		mov    dword ptr [edi], ecx
		mov    ebx, g_dwDstRowStride
		mov    dword ptr [edi + ebx], ecx
		mov    dword ptr [edi + ebx*2], ecx
		add    ebx, g_dwDstRowStride
		add    ebx, g_dwDstRowStride
		mov    dword ptr [edi + ebx], ecx
		dec    g_dwSpanCount
		jg     L1
		pop    ebp
	}
}

// hover.exe:0x412160
// floor/ceiling span, 4-byte pixel, 4 rows, v shift 0xE (row stride 4)
// FUNCTION: HOVER 0x412160
void Span4x4Lit_S14(void)
{
	g_dwSpanCount = (DWORD)((int)g_dwSpanCount / 4);
	g_dwUStep <<= 2;
	g_dwVStep <<= 2;

	_asm
	{
		push   ebp
		mov    eax, g_dwU
		and    eax, g_dwUMask
		mov    edx, g_dwV
		mov    edi, g_dwDst
		mov    ebp, g_dwTexels
		sub    ecx, ecx
		sub    edi, 4
	L1:
		mov    ebx, eax
		add    eax, g_dwUStep
		shr    ebx, 0x10
		and    eax, g_dwUMask
		lea    esi, [ebp + ebx]
		mov    ebx, edx
		add    edx, g_dwVStep
		and    ebx, g_dwVMask
		shr    ebx, 0xe
		add    edi, 4
		sub    ecx, ecx
		mov    cl, byte ptr [esi + ebx]
		mov    ebx, g_dwShadeTable
		mov    cl, byte ptr [ebx + ecx]
		mov    ch, cl
		push   cx
		shl    ecx, 0x10
		pop    cx
		mov    dword ptr [edi], ecx
		mov    ebx, g_dwDstRowStride
		mov    dword ptr [edi + ebx], ecx
		mov    dword ptr [edi + ebx*2], ecx
		add    ebx, g_dwDstRowStride
		add    ebx, g_dwDstRowStride
		mov    dword ptr [edi + ebx], ecx
		dec    g_dwSpanCount
		jg     L1
		pop    ebp
	}
}

// hover.exe:0x412EF0
// wall column, 4-byte pixel, 1 row per texel
// FUNCTION: HOVER 0x412ef0
void Column4x1LitReg(void)
{
	_asm
	{
		push   ebp
		mov    ebx, g_dwSpanCount
		mov    edi, g_dwDst
		mov    esi, g_dwTexels
		mov    edx, g_dwColumnV
		mov    ebp, g_dwShadeTable
		sub    edi, g_dwDstRowStride
		sub    eax, eax
	L1:
		mov    ecx, edx
		add    edx, g_dwUStep
		shr    ecx, 0x10
		add    edi, g_dwDstRowStride
		and    edx, g_dwVMask
		mov    al, byte ptr [esi + ecx]
		mov    cl, byte ptr [ebp + eax]
		mov    ch, cl
		push   cx
		shl    ecx, 0x10
		pop    cx
		dec    ebx
		mov    dword ptr [edi], ecx
		jg     L1
		pop    ebp
	}
}

// hover.exe:0x412F50
// direct (unmapped) span, 4-byte pixel, 1 row per texel
// FUNCTION: HOVER 0x412f50
void Sky4x1Reg(void)
{
	_asm
	{
		pushad
		mov    edi, g_dwDst
		mov    edx, g_dwSkyU
		mov    ebx, g_dwUStep
		mov    esi, g_dwTexels
		mov    ecx, g_dwDstRowStride
		mov    ebp, g_dwSpanCount
		test   ebp, ebp
		jle    L1
		sub    edi, 4
	L2:
		mov    eax, edx
		shr    eax, 0x10
		add    edx, ebx
		mov    al, byte ptr [esi + eax]
		add    edi, 4
		mov    ah, al
		push   ax
		rol    eax, 0x10
		pop    ax
		mov    dword ptr [edi], eax
		dec    ebp
		jne    L2
	L1:
		popad
	}
}

// hover.exe:0x412FA0
// direct (unmapped) span, 4-byte pixel, 4 rows per texel
// FUNCTION: HOVER 0x412fa0
void Sky4x4Reg(void)
{
	_asm
	{
		pushad
		mov    edi, g_dwDst
		mov    edx, g_dwSkyU
		mov    ebx, g_dwUStep
		mov    esi, g_dwTexels
		mov    ecx, g_dwDstRowStride
		mov    ebp, g_dwSpanCount
		test   ebp, ebp
		jle    L1
		sub    edi, 4
	L2:
		mov    eax, edx
		shr    eax, 0x10
		add    edx, ebx
		mov    al, byte ptr [esi + eax]
		add    edi, 4
		mov    ah, al
		push   ax
		rol    eax, 0x10
		pop    ax
		mov    dword ptr [edi], eax
		add    edi, ecx
		mov    dword ptr [edi], eax
		mov    dword ptr [edi + ecx], eax
		mov    dword ptr [edi + ecx*2], eax
		sub    edi, ecx
		dec    ebp
		jne    L2
	L1:
		popad
	}
}

// hover.exe:0x413350
// floor/ceiling span, 4-byte pixel, 1 row, v shift 0x9 (row stride 128)
// FUNCTION: HOVER 0x413350
void Span4x1Lit_S9(void)
{
	g_dwSpanCount = (DWORD)((int)g_dwSpanCount / 4);
	g_dwUStep <<= 2;
	g_dwVStep <<= 2;

	_asm
	{
		push   ebp
		mov    eax, g_dwU
		and    eax, g_dwUMask
		mov    edx, g_dwV
		mov    edi, g_dwDst
		mov    ebp, g_dwTexels
		sub    ecx, ecx
		sub    edi, 4
	L1:
		mov    ebx, eax
		add    eax, g_dwUStep
		shr    ebx, 0x10
		and    eax, g_dwUMask
		lea    esi, [ebp + ebx]
		mov    ebx, edx
		add    edx, g_dwVStep
		and    ebx, g_dwVMask
		shr    ebx, 9
		add    edi, 4
		sub    ecx, ecx
		mov    cl, byte ptr [esi + ebx]
		mov    ebx, g_dwShadeTable
		mov    cl, byte ptr [ebx + ecx]
		mov    ch, cl
		push   cx
		shl    ecx, 0x10
		pop    cx
		dec    g_dwSpanCount
		mov    dword ptr [edi], ecx
		jg     L1
		pop    ebp
	}
}

// hover.exe:0x4133F0
// floor/ceiling span, 4-byte pixel, 1 row, v shift 0xA (row stride 64)
// FUNCTION: HOVER 0x4133f0
void Span4x1Lit_S10(void)
{
	g_dwSpanCount = (DWORD)((int)g_dwSpanCount / 4);
	g_dwUStep <<= 2;
	g_dwVStep <<= 2;

	_asm
	{
		push   ebp
		mov    eax, g_dwU
		and    eax, g_dwUMask
		mov    edx, g_dwV
		mov    edi, g_dwDst
		mov    ebp, g_dwTexels
		sub    ecx, ecx
		sub    edi, 4
	L1:
		mov    ebx, eax
		add    eax, g_dwUStep
		shr    ebx, 0x10
		and    eax, g_dwUMask
		lea    esi, [ebp + ebx]
		mov    ebx, edx
		add    edx, g_dwVStep
		and    ebx, g_dwVMask
		shr    ebx, 0xa
		add    edi, 4
		sub    ecx, ecx
		mov    cl, byte ptr [esi + ebx]
		mov    ebx, g_dwShadeTable
		mov    cl, byte ptr [ebx + ecx]
		mov    ch, cl
		push   cx
		shl    ecx, 0x10
		pop    cx
		dec    g_dwSpanCount
		mov    dword ptr [edi], ecx
		jg     L1
		pop    ebp
	}
}

// hover.exe:0x413490
// floor/ceiling span, 4-byte pixel, 1 row, v shift 0xB (row stride 32)
// FUNCTION: HOVER 0x413490
void Span4x1Lit_S11(void)
{
	g_dwSpanCount = (DWORD)((int)g_dwSpanCount / 4);
	g_dwUStep <<= 2;
	g_dwVStep <<= 2;

	_asm
	{
		push   ebp
		mov    eax, g_dwU
		and    eax, g_dwUMask
		mov    edx, g_dwV
		mov    edi, g_dwDst
		mov    ebp, g_dwTexels
		sub    ecx, ecx
		sub    edi, 4
	L1:
		mov    ebx, eax
		add    eax, g_dwUStep
		shr    ebx, 0x10
		and    eax, g_dwUMask
		lea    esi, [ebp + ebx]
		mov    ebx, edx
		add    edx, g_dwVStep
		and    ebx, g_dwVMask
		shr    ebx, 0xb
		add    edi, 4
		sub    ecx, ecx
		mov    cl, byte ptr [esi + ebx]
		mov    ebx, g_dwShadeTable
		mov    cl, byte ptr [ebx + ecx]
		mov    ch, cl
		push   cx
		shl    ecx, 0x10
		pop    cx
		dec    g_dwSpanCount
		mov    dword ptr [edi], ecx
		jg     L1
		pop    ebp
	}
}

// hover.exe:0x413530
// floor/ceiling span, 4-byte pixel, 1 row, v shift 0xC (row stride 16)
// FUNCTION: HOVER 0x413530
void Span4x1Lit_S12(void)
{
	g_dwSpanCount = (DWORD)((int)g_dwSpanCount / 4);
	g_dwUStep <<= 2;
	g_dwVStep <<= 2;

	_asm
	{
		push   ebp
		mov    eax, g_dwU
		and    eax, g_dwUMask
		mov    edx, g_dwV
		mov    edi, g_dwDst
		mov    ebp, g_dwTexels
		sub    ecx, ecx
		sub    edi, 4
	L1:
		mov    ebx, eax
		add    eax, g_dwUStep
		shr    ebx, 0x10
		and    eax, g_dwUMask
		lea    esi, [ebp + ebx]
		mov    ebx, edx
		add    edx, g_dwVStep
		and    ebx, g_dwVMask
		shr    ebx, 0xc
		add    edi, 4
		sub    ecx, ecx
		mov    cl, byte ptr [esi + ebx]
		mov    ebx, g_dwShadeTable
		mov    cl, byte ptr [ebx + ecx]
		mov    ch, cl
		push   cx
		shl    ecx, 0x10
		pop    cx
		dec    g_dwSpanCount
		mov    dword ptr [edi], ecx
		jg     L1
		pop    ebp
	}
}

// hover.exe:0x4135D0
// floor/ceiling span, 4-byte pixel, 1 row, v shift 0xD (row stride 8)
// FUNCTION: HOVER 0x4135d0
void Span4x1Lit_S13(void)
{
	g_dwSpanCount = (DWORD)((int)g_dwSpanCount / 4);
	g_dwUStep <<= 2;
	g_dwVStep <<= 2;

	_asm
	{
		push   ebp
		mov    eax, g_dwU
		and    eax, g_dwUMask
		mov    edx, g_dwV
		mov    edi, g_dwDst
		mov    ebp, g_dwTexels
		sub    ecx, ecx
		sub    edi, 4
	L1:
		mov    ebx, eax
		add    eax, g_dwUStep
		shr    ebx, 0x10
		and    eax, g_dwUMask
		lea    esi, [ebp + ebx]
		mov    ebx, edx
		add    edx, g_dwVStep
		and    ebx, g_dwVMask
		shr    ebx, 0xd
		add    edi, 4
		sub    ecx, ecx
		mov    cl, byte ptr [esi + ebx]
		mov    ebx, g_dwShadeTable
		mov    cl, byte ptr [ebx + ecx]
		mov    ch, cl
		push   cx
		shl    ecx, 0x10
		pop    cx
		dec    g_dwSpanCount
		mov    dword ptr [edi], ecx
		jg     L1
		pop    ebp
	}
}

// hover.exe:0x413670
// floor/ceiling span, 4-byte pixel, 1 row, v shift 0xE (row stride 4)
// FUNCTION: HOVER 0x413670
void Span4x1Lit_S14(void)
{
	g_dwSpanCount = (DWORD)((int)g_dwSpanCount / 4);
	g_dwUStep <<= 2;
	g_dwVStep <<= 2;

	_asm
	{
		push   ebp
		mov    eax, g_dwU
		and    eax, g_dwUMask
		mov    edx, g_dwV
		mov    edi, g_dwDst
		mov    ebp, g_dwTexels
		sub    ecx, ecx
		sub    edi, 4
	L1:
		mov    ebx, eax
		add    eax, g_dwUStep
		shr    ebx, 0x10
		and    eax, g_dwUMask
		lea    esi, [ebp + ebx]
		mov    ebx, edx
		add    edx, g_dwVStep
		and    ebx, g_dwVMask
		shr    ebx, 0xe
		add    edi, 4
		sub    ecx, ecx
		mov    cl, byte ptr [esi + ebx]
		mov    ebx, g_dwShadeTable
		mov    cl, byte ptr [ebx + ecx]
		mov    ch, cl
		push   cx
		shl    ecx, 0x10
		pop    cx
		dec    g_dwSpanCount
		mov    dword ptr [edi], ecx
		jg     L1
		pop    ebp
	}
}

// hover.exe:0x414220
// floor/ceiling span, 2-byte pixel, 2 rows, v shift 0x8 (row stride 256)
// FUNCTION: HOVER 0x414220
void Span2x2Lit_S8(void)
{
	g_dwSpanCount = (DWORD)((int)g_dwSpanCount / 2);
	g_dwUStep <<= 1;
	g_dwVStep <<= 1;

	_asm
	{
		push   ebp
		mov    eax, g_dwU
		and    eax, g_dwUMask
		mov    edx, g_dwV
		mov    edi, g_dwDst
		mov    ebp, g_dwTexels
		sub    ecx, ecx
		sub    edi, 2
	L1:
		mov    ebx, eax
		add    eax, g_dwUStep
		shr    ebx, 0x10
		and    eax, g_dwUMask
		lea    esi, [ebp + ebx]
		mov    ebx, edx
		add    edx, g_dwVStep
		and    ebx, g_dwVMask
		shr    ebx, 8
		add    edi, 2
		sub    ch, ch
		mov    cl, byte ptr [esi + ebx]
		mov    ebx, g_dwShadeTable
		mov    cl, byte ptr [ebx + ecx]
		dec    g_dwSpanCount
		mov    ch, cl
		mov    word ptr [edi], cx
		mov    ebx, g_dwDstRowStride
		mov    word ptr [edi + ebx], cx
		jg     L1
		pop    ebp
	}
}

// hover.exe:0x424FB0
// floor/ceiling span, 4-byte pixel, 1 row, v shift 0x8 (row stride 256)
// FUNCTION: HOVER 0x424fb0
void Span4x1Lit_S8(void)
{
	g_dwSpanCount = (DWORD)((int)g_dwSpanCount / 4);
	g_dwUStep <<= 2;
	g_dwVStep <<= 2;

	_asm
	{
		push   ebp
		mov    eax, g_dwU
		and    eax, g_dwUMask
		mov    edx, g_dwV
		mov    edi, g_dwDst
		mov    ebp, g_dwTexels
		sub    ecx, ecx
		sub    edi, 4
	L1:
		mov    ebx, eax
		add    eax, g_dwUStep
		shr    ebx, 0x10
		and    eax, g_dwUMask
		lea    esi, [ebp + ebx]
		mov    ebx, edx
		add    edx, g_dwVStep
		and    ebx, g_dwVMask
		shr    ebx, 8
		add    edi, 4
		sub    ecx, ecx
		mov    cl, byte ptr [esi + ebx]
		mov    ebx, g_dwShadeTable
		mov    cl, byte ptr [ebx + ecx]
		mov    ch, cl
		push   cx
		shl    ecx, 0x10
		pop    cx
		dec    g_dwSpanCount
		mov    dword ptr [edi], ecx
		jg     L1
		pop    ebp
	}
}
