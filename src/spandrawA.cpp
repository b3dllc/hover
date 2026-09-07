// spandrawA.cpp : span drawers, group A
//
// 46 of the 167 hand-written span drawers that CSub410F20::Sub_410F20
// (texload.cpp) installs in the render context's drawer slots. Each is a C
// function whose body is one `_asm` block; the compiler contributes only the
// ebx/esi/edi saves, the `ret`, and - in the drawers that halve or quarter
// the span - the leading C statements that rescale g_dwUnk_4A4A14 and the
// texture steps. `push ebp` / `pop ebp` (or `pushad` / `popad`) inside the
// block is the original's own save of the registers VC 2.1 does not save.
//
// Three families live here; all of them read the renderer globals set up by
// the span builders in texheap.cpp and write through g_dwUnk_4A4A4C.
//
//   Wall column     one 16.16 texture coordinate (g_dwUnk_4A4A40, stepped by
//                   g_dwUnk_4A4A50 and wrapped with g_dwUnk_4A4A38), shaded
//                   through the light table g_dwUnk_4A4A3C, destination
//                   advanced by g_dwUnk_4A4A30 (one framebuffer row) per
//                   texel: a vertical texture-mapped column.
//
//   Floor/ceiling   two coordinates - u = g_dwUnk_4A4A28 & g_dwUnk_4A4A20
//   span            stepped by g_dwUnk_4A4A50 (the texture column, taken
//                   from bit 16 up) and v = g_dwUnk_4A4A2C & g_dwUnk_4A4A38
//                   stepped by g_dwUnk_4A4A24 (the texture row, shifted right
//                   by the family's shift constant, which therefore selects
//                   the texture's row stride: shift 8 = 256, 9 = 128,
//                   0xA = 64, 0xB = 32, 0xC = 16, 0xD = 8, 0xE = 4 bytes).
//                   Shaded through g_dwUnk_4A4A3C, destination advanced
//                   along the scanline.
//
//   Direct span     the unshaded variant: texture position g_dwUnk_4A4A18
//                   stepped by g_dwUnk_4A4A50, the texel taken straight from
//                   g_dwUnk_4A4A1C with no light-table lookup. These are the
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
void Hover_sub_40D600(void)
{
	g_dwUnk_4A4A14 = (DWORD)((int)g_dwUnk_4A4A14 / 2);
	g_dwUnk_4A4A50 <<= 1;

	if ((int)g_dwUnk_4A4A14 > 0)
	{
		_asm
		{
			push   ebp
			mov    ebx, g_dwUnk_4A4A50
			mov    edi, g_dwUnk_4A4A4C
			mov    esi, g_dwUnk_4A4A1C
			mov    edx, g_dwUnk_4A4A40
			mov    ebp, g_dwUnk_4A4A3C
			sub    edi, g_dwUnk_4A4A30
			sub    eax, eax
		L1:
			mov    ecx, edx
			add    edx, ebx
			shr    ecx, 0x10
			add    edi, g_dwUnk_4A4A30
			and    edx, g_dwUnk_4A4A38
			mov    al, byte ptr [esi + ecx]
			mov    cl, byte ptr [ebp + eax]
			mov    ch, cl
			push   cx
			shl    ecx, 0x10
			pop    cx
			mov    dword ptr [edi], ecx
			add    edi, g_dwUnk_4A4A30
			dec    g_dwUnk_4A4A14
			mov    dword ptr [edi], ecx
			jg     L1
			pop    ebp
		}
	}
}

// hover.exe:0x40D680
// direct (unmapped) span, 4-byte pixel, 2 rows per texel
// FUNCTION: HOVER 0x40d680
void Hover_sub_40D680(void)
{
	_asm
	{
		pushad
		mov    edi, g_dwUnk_4A4A4C
		mov    edx, g_dwUnk_4A4A18
		mov    ebx, g_dwUnk_4A4A50
		mov    esi, g_dwUnk_4A4A1C
		mov    ecx, g_dwUnk_4A4A30
		mov    ebp, g_dwUnk_4A4A14
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
void Hover_sub_40D900(void)
{
	g_dwUnk_4A4A14 = (DWORD)((int)g_dwUnk_4A4A14 / 4);
	g_dwUnk_4A4A50 <<= 2;
	g_dwUnk_4A4A24 <<= 2;

	_asm
	{
		push   ebp
		mov    eax, g_dwUnk_4A4A28
		and    eax, g_dwUnk_4A4A20
		mov    edx, g_dwUnk_4A4A2C
		mov    edi, g_dwUnk_4A4A4C
		mov    ebp, g_dwUnk_4A4A1C
		sub    ecx, ecx
		sub    edi, 4
	L1:
		mov    ebx, eax
		add    eax, g_dwUnk_4A4A50
		shr    ebx, 0x10
		and    eax, g_dwUnk_4A4A20
		lea    esi, [ebp + ebx]
		mov    ebx, edx
		add    edx, g_dwUnk_4A4A24
		and    ebx, g_dwUnk_4A4A38
		shr    ebx, 0xa
		add    edi, 4
		sub    ecx, ecx
		mov    cl, byte ptr [esi + ebx]
		mov    ebx, g_dwUnk_4A4A3C
		mov    cl, byte ptr [ebx + ecx]
		mov    ch, cl
		push   cx
		shl    ecx, 0x10
		pop    cx
		dec    g_dwUnk_4A4A14
		mov    dword ptr [edi], ecx
		mov    ebx, g_dwUnk_4A4A30
		mov    dword ptr [edi + ebx], ecx
		jg     L1
		pop    ebp
	}
}

// hover.exe:0x40D9A0
// floor/ceiling span, 4-byte pixel, 2 rows, v shift 0xB (row stride 32)
// FUNCTION: HOVER 0x40d9a0
void Hover_sub_40D9A0(void)
{
	g_dwUnk_4A4A14 = (DWORD)((int)g_dwUnk_4A4A14 / 4);
	g_dwUnk_4A4A50 <<= 2;
	g_dwUnk_4A4A24 <<= 2;

	_asm
	{
		push   ebp
		mov    eax, g_dwUnk_4A4A28
		and    eax, g_dwUnk_4A4A20
		mov    edx, g_dwUnk_4A4A2C
		mov    edi, g_dwUnk_4A4A4C
		mov    ebp, g_dwUnk_4A4A1C
		sub    ecx, ecx
		sub    edi, 4
	L1:
		mov    ebx, eax
		add    eax, g_dwUnk_4A4A50
		shr    ebx, 0x10
		and    eax, g_dwUnk_4A4A20
		lea    esi, [ebp + ebx]
		mov    ebx, edx
		add    edx, g_dwUnk_4A4A24
		and    ebx, g_dwUnk_4A4A38
		shr    ebx, 0xb
		add    edi, 4
		sub    ecx, ecx
		mov    cl, byte ptr [esi + ebx]
		mov    ebx, g_dwUnk_4A4A3C
		mov    cl, byte ptr [ebx + ecx]
		mov    ch, cl
		push   cx
		shl    ecx, 0x10
		pop    cx
		dec    g_dwUnk_4A4A14
		mov    dword ptr [edi], ecx
		mov    ebx, g_dwUnk_4A4A30
		mov    dword ptr [edi + ebx], ecx
		jg     L1
		pop    ebp
	}
}

// hover.exe:0x40DA40
// floor/ceiling span, 4-byte pixel, 2 rows, v shift 0xC (row stride 16)
// FUNCTION: HOVER 0x40da40
void Hover_sub_40DA40(void)
{
	g_dwUnk_4A4A14 = (DWORD)((int)g_dwUnk_4A4A14 / 4);
	g_dwUnk_4A4A50 <<= 2;
	g_dwUnk_4A4A24 <<= 2;

	_asm
	{
		push   ebp
		mov    eax, g_dwUnk_4A4A28
		and    eax, g_dwUnk_4A4A20
		mov    edx, g_dwUnk_4A4A2C
		mov    edi, g_dwUnk_4A4A4C
		mov    ebp, g_dwUnk_4A4A1C
		sub    ecx, ecx
		sub    edi, 4
	L1:
		mov    ebx, eax
		add    eax, g_dwUnk_4A4A50
		shr    ebx, 0x10
		and    eax, g_dwUnk_4A4A20
		lea    esi, [ebp + ebx]
		mov    ebx, edx
		add    edx, g_dwUnk_4A4A24
		and    ebx, g_dwUnk_4A4A38
		shr    ebx, 0xc
		add    edi, 4
		sub    ecx, ecx
		mov    cl, byte ptr [esi + ebx]
		mov    ebx, g_dwUnk_4A4A3C
		mov    cl, byte ptr [ebx + ecx]
		mov    ch, cl
		push   cx
		shl    ecx, 0x10
		pop    cx
		dec    g_dwUnk_4A4A14
		mov    dword ptr [edi], ecx
		mov    ebx, g_dwUnk_4A4A30
		mov    dword ptr [edi + ebx], ecx
		jg     L1
		pop    ebp
	}
}

// hover.exe:0x40DAE0
// floor/ceiling span, 4-byte pixel, 2 rows, v shift 0xD (row stride 8)
// FUNCTION: HOVER 0x40dae0
void Hover_sub_40DAE0(void)
{
	g_dwUnk_4A4A14 = (DWORD)((int)g_dwUnk_4A4A14 / 4);
	g_dwUnk_4A4A50 <<= 2;
	g_dwUnk_4A4A24 <<= 2;

	_asm
	{
		push   ebp
		mov    eax, g_dwUnk_4A4A28
		and    eax, g_dwUnk_4A4A20
		mov    edx, g_dwUnk_4A4A2C
		mov    edi, g_dwUnk_4A4A4C
		mov    ebp, g_dwUnk_4A4A1C
		sub    ecx, ecx
		sub    edi, 4
	L1:
		mov    ebx, eax
		add    eax, g_dwUnk_4A4A50
		shr    ebx, 0x10
		and    eax, g_dwUnk_4A4A20
		lea    esi, [ebp + ebx]
		mov    ebx, edx
		add    edx, g_dwUnk_4A4A24
		and    ebx, g_dwUnk_4A4A38
		shr    ebx, 0xd
		add    edi, 4
		sub    ecx, ecx
		mov    cl, byte ptr [esi + ebx]
		mov    ebx, g_dwUnk_4A4A3C
		mov    cl, byte ptr [ebx + ecx]
		mov    ch, cl
		push   cx
		shl    ecx, 0x10
		pop    cx
		dec    g_dwUnk_4A4A14
		mov    dword ptr [edi], ecx
		mov    ebx, g_dwUnk_4A4A30
		mov    dword ptr [edi + ebx], ecx
		jg     L1
		pop    ebp
	}
}

// hover.exe:0x40DB80
// floor/ceiling span, 4-byte pixel, 2 rows, v shift 0xE (row stride 4)
// FUNCTION: HOVER 0x40db80
void Hover_sub_40DB80(void)
{
	g_dwUnk_4A4A14 = (DWORD)((int)g_dwUnk_4A4A14 / 4);
	g_dwUnk_4A4A50 <<= 2;
	g_dwUnk_4A4A24 <<= 2;

	_asm
	{
		push   ebp
		mov    eax, g_dwUnk_4A4A28
		and    eax, g_dwUnk_4A4A20
		mov    edx, g_dwUnk_4A4A2C
		mov    edi, g_dwUnk_4A4A4C
		mov    ebp, g_dwUnk_4A4A1C
		sub    ecx, ecx
		sub    edi, 4
	L1:
		mov    ebx, eax
		add    eax, g_dwUnk_4A4A50
		shr    ebx, 0x10
		and    eax, g_dwUnk_4A4A20
		lea    esi, [ebp + ebx]
		mov    ebx, edx
		add    edx, g_dwUnk_4A4A24
		and    ebx, g_dwUnk_4A4A38
		shr    ebx, 0xe
		add    edi, 4
		sub    ecx, ecx
		mov    cl, byte ptr [esi + ebx]
		mov    ebx, g_dwUnk_4A4A3C
		mov    cl, byte ptr [ebx + ecx]
		mov    ch, cl
		push   cx
		shl    ecx, 0x10
		pop    cx
		dec    g_dwUnk_4A4A14
		mov    dword ptr [edi], ecx
		mov    ebx, g_dwUnk_4A4A30
		mov    dword ptr [edi + ebx], ecx
		jg     L1
		pop    ebp
	}
}

// hover.exe:0x40DDA0
// wall column, 2-byte pixel, 2 rows per texel
// FUNCTION: HOVER 0x40dda0
void Hover_sub_40DDA0(void)
{
	g_dwUnk_4A4A14 = (DWORD)((int)g_dwUnk_4A4A14 / 2);
	g_dwUnk_4A4A50 <<= 1;

	if ((int)g_dwUnk_4A4A14 > 0)
	{
		_asm
		{
			push   ebp
			mov    ebx, g_dwUnk_4A4A50
			mov    edi, g_dwUnk_4A4A4C
			mov    esi, g_dwUnk_4A4A1C
			mov    edx, g_dwUnk_4A4A40
			mov    ebp, g_dwUnk_4A4A3C
			sub    edi, g_dwUnk_4A4A30
			sub    eax, eax
		L1:
			mov    ecx, edx
			add    edx, ebx
			shr    ecx, 0x10
			add    edi, g_dwUnk_4A4A30
			and    edx, g_dwUnk_4A4A38
			mov    al, byte ptr [esi + ecx]
			mov    cl, byte ptr [ebp + eax]
			mov    ch, cl
			mov    word ptr [edi], cx
			add    edi, g_dwUnk_4A4A30
			dec    g_dwUnk_4A4A14
			mov    word ptr [edi], cx
			jg     L1
			pop    ebp
		}
	}
}

// hover.exe:0x40DE20
// direct (unmapped) span, 2-byte pixel, 2 rows per texel
// FUNCTION: HOVER 0x40de20
void Hover_sub_40DE20(void)
{
	_asm
	{
		pushad
		mov    edi, g_dwUnk_4A4A4C
		mov    edx, g_dwUnk_4A4A18
		mov    ebx, g_dwUnk_4A4A50
		mov    esi, g_dwUnk_4A4A1C
		mov    ecx, g_dwUnk_4A4A30
		push   g_dwUnk_4A4A20
		mov    ebp, g_dwUnk_4A4A14
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
void Hover_sub_40DF80(void)
{
	g_dwUnk_4A4A14 = (DWORD)((int)g_dwUnk_4A4A14 / 2);
	g_dwUnk_4A4A50 <<= 1;
	g_dwUnk_4A4A24 <<= 1;

	_asm
	{
		push   ebp
		mov    eax, g_dwUnk_4A4A28
		and    eax, g_dwUnk_4A4A20
		mov    edx, g_dwUnk_4A4A2C
		mov    edi, g_dwUnk_4A4A4C
		mov    ebp, g_dwUnk_4A4A1C
		sub    ecx, ecx
		sub    edi, 2
	L1:
		mov    ebx, eax
		add    eax, g_dwUnk_4A4A50
		shr    ebx, 0x10
		and    eax, g_dwUnk_4A4A20
		lea    esi, [ebp + ebx]
		mov    ebx, edx
		add    edx, g_dwUnk_4A4A24
		and    ebx, g_dwUnk_4A4A38
		shr    ebx, 0xa
		add    edi, 2
		sub    ch, ch
		mov    cl, byte ptr [esi + ebx]
		mov    ebx, g_dwUnk_4A4A3C
		mov    cl, byte ptr [ebx + ecx]
		dec    g_dwUnk_4A4A14
		mov    ch, cl
		mov    word ptr [edi], cx
		mov    ebx, g_dwUnk_4A4A30
		mov    word ptr [edi + ebx], cx
		jg     L1
		pop    ebp
	}
}

// hover.exe:0x40E020
// floor/ceiling span, 2-byte pixel, 2 rows, v shift 0xB (row stride 32)
// FUNCTION: HOVER 0x40e020
void Hover_sub_40E020(void)
{
	g_dwUnk_4A4A14 = (DWORD)((int)g_dwUnk_4A4A14 / 2);
	g_dwUnk_4A4A50 <<= 1;
	g_dwUnk_4A4A24 <<= 1;

	_asm
	{
		push   ebp
		mov    eax, g_dwUnk_4A4A28
		and    eax, g_dwUnk_4A4A20
		mov    edx, g_dwUnk_4A4A2C
		mov    edi, g_dwUnk_4A4A4C
		mov    ebp, g_dwUnk_4A4A1C
		sub    ecx, ecx
		sub    edi, 2
	L1:
		mov    ebx, eax
		add    eax, g_dwUnk_4A4A50
		shr    ebx, 0x10
		and    eax, g_dwUnk_4A4A20
		lea    esi, [ebp + ebx]
		mov    ebx, edx
		add    edx, g_dwUnk_4A4A24
		and    ebx, g_dwUnk_4A4A38
		shr    ebx, 0xb
		add    edi, 2
		sub    ch, ch
		mov    cl, byte ptr [esi + ebx]
		mov    ebx, g_dwUnk_4A4A3C
		mov    cl, byte ptr [ebx + ecx]
		dec    g_dwUnk_4A4A14
		mov    ch, cl
		mov    word ptr [edi], cx
		mov    ebx, g_dwUnk_4A4A30
		mov    word ptr [edi + ebx], cx
		jg     L1
		pop    ebp
	}
}

// hover.exe:0x40E0C0
// floor/ceiling span, 2-byte pixel, 2 rows, v shift 0xC (row stride 16)
// FUNCTION: HOVER 0x40e0c0
void Hover_sub_40E0C0(void)
{
	g_dwUnk_4A4A14 = (DWORD)((int)g_dwUnk_4A4A14 / 2);
	g_dwUnk_4A4A50 <<= 1;
	g_dwUnk_4A4A24 <<= 1;

	_asm
	{
		push   ebp
		mov    eax, g_dwUnk_4A4A28
		and    eax, g_dwUnk_4A4A20
		mov    edx, g_dwUnk_4A4A2C
		mov    edi, g_dwUnk_4A4A4C
		mov    ebp, g_dwUnk_4A4A1C
		sub    ecx, ecx
		sub    edi, 2
	L1:
		mov    ebx, eax
		add    eax, g_dwUnk_4A4A50
		shr    ebx, 0x10
		and    eax, g_dwUnk_4A4A20
		lea    esi, [ebp + ebx]
		mov    ebx, edx
		add    edx, g_dwUnk_4A4A24
		and    ebx, g_dwUnk_4A4A38
		shr    ebx, 0xc
		add    edi, 2
		sub    ch, ch
		mov    cl, byte ptr [esi + ebx]
		mov    ebx, g_dwUnk_4A4A3C
		mov    cl, byte ptr [ebx + ecx]
		dec    g_dwUnk_4A4A14
		mov    ch, cl
		mov    word ptr [edi], cx
		mov    ebx, g_dwUnk_4A4A30
		mov    word ptr [edi + ebx], cx
		jg     L1
		pop    ebp
	}
}

// hover.exe:0x40E160
// floor/ceiling span, 2-byte pixel, 2 rows, v shift 0xD (row stride 8)
// FUNCTION: HOVER 0x40e160
void Hover_sub_40E160(void)
{
	g_dwUnk_4A4A14 = (DWORD)((int)g_dwUnk_4A4A14 / 2);
	g_dwUnk_4A4A50 <<= 1;
	g_dwUnk_4A4A24 <<= 1;

	_asm
	{
		push   ebp
		mov    eax, g_dwUnk_4A4A28
		and    eax, g_dwUnk_4A4A20
		mov    edx, g_dwUnk_4A4A2C
		mov    edi, g_dwUnk_4A4A4C
		mov    ebp, g_dwUnk_4A4A1C
		sub    ecx, ecx
		sub    edi, 2
	L1:
		mov    ebx, eax
		add    eax, g_dwUnk_4A4A50
		shr    ebx, 0x10
		and    eax, g_dwUnk_4A4A20
		lea    esi, [ebp + ebx]
		mov    ebx, edx
		add    edx, g_dwUnk_4A4A24
		and    ebx, g_dwUnk_4A4A38
		shr    ebx, 0xd
		add    edi, 2
		sub    ch, ch
		mov    cl, byte ptr [esi + ebx]
		mov    ebx, g_dwUnk_4A4A3C
		mov    cl, byte ptr [ebx + ecx]
		dec    g_dwUnk_4A4A14
		mov    ch, cl
		mov    word ptr [edi], cx
		mov    ebx, g_dwUnk_4A4A30
		mov    word ptr [edi + ebx], cx
		jg     L1
		pop    ebp
	}
}

// hover.exe:0x40E200
// floor/ceiling span, 2-byte pixel, 2 rows, v shift 0xE (row stride 4)
// FUNCTION: HOVER 0x40e200
void Hover_sub_40E200(void)
{
	g_dwUnk_4A4A14 = (DWORD)((int)g_dwUnk_4A4A14 / 2);
	g_dwUnk_4A4A50 <<= 1;
	g_dwUnk_4A4A24 <<= 1;

	_asm
	{
		push   ebp
		mov    eax, g_dwUnk_4A4A28
		and    eax, g_dwUnk_4A4A20
		mov    edx, g_dwUnk_4A4A2C
		mov    edi, g_dwUnk_4A4A4C
		mov    ebp, g_dwUnk_4A4A1C
		sub    ecx, ecx
		sub    edi, 2
	L1:
		mov    ebx, eax
		add    eax, g_dwUnk_4A4A50
		shr    ebx, 0x10
		and    eax, g_dwUnk_4A4A20
		lea    esi, [ebp + ebx]
		mov    ebx, edx
		add    edx, g_dwUnk_4A4A24
		and    ebx, g_dwUnk_4A4A38
		shr    ebx, 0xe
		add    edi, 2
		sub    ch, ch
		mov    cl, byte ptr [esi + ebx]
		mov    ebx, g_dwUnk_4A4A3C
		mov    cl, byte ptr [ebx + ecx]
		dec    g_dwUnk_4A4A14
		mov    ch, cl
		mov    word ptr [edi], cx
		mov    ebx, g_dwUnk_4A4A30
		mov    word ptr [edi + ebx], cx
		jg     L1
		pop    ebp
	}
}

// hover.exe:0x40E5A0
// wall column, 2-byte pixel, 1 row per texel
// FUNCTION: HOVER 0x40e5a0
void Hover_sub_40E5A0(void)
{
	_asm
	{
		push   ebp
		mov    ebx, g_dwUnk_4A4A14
		mov    edi, g_dwUnk_4A4A4C
		mov    esi, g_dwUnk_4A4A1C
		mov    edx, g_dwUnk_4A4A40
		mov    ebp, g_dwUnk_4A4A3C
		sub    edi, g_dwUnk_4A4A30
		sub    eax, eax
	L1:
		mov    ecx, edx
		add    edx, g_dwUnk_4A4A50
		shr    ecx, 0x10
		add    edi, g_dwUnk_4A4A30
		and    edx, g_dwUnk_4A4A38
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
void Hover_sub_40E600(void)
{
	_asm
	{
		pushad
		mov    edi, g_dwUnk_4A4A4C
		mov    edx, g_dwUnk_4A4A18
		mov    ebx, g_dwUnk_4A4A50
		mov    esi, g_dwUnk_4A4A1C
		mov    ecx, g_dwUnk_4A4A30
		mov    ebp, g_dwUnk_4A4A14
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
void Hover_sub_40E850(void)
{
	g_dwUnk_4A4A14 = (DWORD)((int)g_dwUnk_4A4A14 / 2);
	g_dwUnk_4A4A50 <<= 1;
	g_dwUnk_4A4A24 <<= 1;

	_asm
	{
		push   ebp
		mov    eax, g_dwUnk_4A4A28
		and    eax, g_dwUnk_4A4A20
		mov    edx, g_dwUnk_4A4A2C
		mov    edi, g_dwUnk_4A4A4C
		mov    ebp, g_dwUnk_4A4A1C
		sub    ecx, ecx
		sub    edi, 2
	L1:
		mov    ebx, eax
		add    eax, g_dwUnk_4A4A50
		shr    ebx, 0x10
		and    eax, g_dwUnk_4A4A20
		lea    esi, [ebp + ebx]
		mov    ebx, edx
		add    edx, g_dwUnk_4A4A24
		and    ebx, g_dwUnk_4A4A38
		shr    ebx, 0xb
		add    edi, 2
		sub    ch, ch
		mov    cl, byte ptr [esi + ebx]
		mov    ebx, g_dwUnk_4A4A3C
		mov    cl, byte ptr [ebx + ecx]
		dec    g_dwUnk_4A4A14
		mov    ch, cl
		mov    word ptr [edi], cx
		jg     L1
		pop    ebp
	}
}

// hover.exe:0x40E910
// floor/ceiling span, 2-byte pixel, 1 row, v shift 0xC (row stride 16)
// FUNCTION: HOVER 0x40e910
void Hover_sub_40E910(void)
{
	g_dwUnk_4A4A14 = (DWORD)((int)g_dwUnk_4A4A14 / 2);
	g_dwUnk_4A4A50 <<= 1;
	g_dwUnk_4A4A24 <<= 1;

	_asm
	{
		push   ebp
		mov    eax, g_dwUnk_4A4A28
		and    eax, g_dwUnk_4A4A20
		mov    edx, g_dwUnk_4A4A2C
		mov    edi, g_dwUnk_4A4A4C
		mov    ebp, g_dwUnk_4A4A1C
		sub    ecx, ecx
		sub    edi, 2
	L1:
		mov    ebx, eax
		add    eax, g_dwUnk_4A4A50
		shr    ebx, 0x10
		and    eax, g_dwUnk_4A4A20
		lea    esi, [ebp + ebx]
		mov    ebx, edx
		add    edx, g_dwUnk_4A4A24
		and    ebx, g_dwUnk_4A4A38
		shr    ebx, 0xc
		add    edi, 2
		sub    ch, ch
		mov    cl, byte ptr [esi + ebx]
		mov    ebx, g_dwUnk_4A4A3C
		mov    cl, byte ptr [ebx + ecx]
		dec    g_dwUnk_4A4A14
		mov    ch, cl
		mov    word ptr [edi], cx
		jg     L1
		pop    ebp
	}
}

// hover.exe:0x40EAC0
// floor/ceiling span, 2-byte pixel, 1 row, v shift 0xD (row stride 8)
// FUNCTION: HOVER 0x40eac0
void Hover_sub_40EAC0(void)
{
	g_dwUnk_4A4A14 = (DWORD)((int)g_dwUnk_4A4A14 / 2);
	g_dwUnk_4A4A50 <<= 1;
	g_dwUnk_4A4A24 <<= 1;

	_asm
	{
		push   ebp
		mov    eax, g_dwUnk_4A4A28
		and    eax, g_dwUnk_4A4A20
		mov    edx, g_dwUnk_4A4A2C
		mov    edi, g_dwUnk_4A4A4C
		mov    ebp, g_dwUnk_4A4A1C
		sub    ecx, ecx
		sub    edi, 2
	L1:
		mov    ebx, eax
		add    eax, g_dwUnk_4A4A50
		shr    ebx, 0x10
		and    eax, g_dwUnk_4A4A20
		lea    esi, [ebp + ebx]
		mov    ebx, edx
		add    edx, g_dwUnk_4A4A24
		and    ebx, g_dwUnk_4A4A38
		shr    ebx, 0xd
		add    edi, 2
		sub    ch, ch
		mov    cl, byte ptr [esi + ebx]
		mov    ebx, g_dwUnk_4A4A3C
		mov    cl, byte ptr [ebx + ecx]
		dec    g_dwUnk_4A4A14
		mov    ch, cl
		mov    word ptr [edi], cx
		jg     L1
		pop    ebp
	}
}

// hover.exe:0x40EDD0
// floor/ceiling span, 2-byte pixel, 1 row, v shift 0xE (row stride 4)
// FUNCTION: HOVER 0x40edd0
void Hover_sub_40EDD0(void)
{
	g_dwUnk_4A4A14 = (DWORD)((int)g_dwUnk_4A4A14 / 2);
	g_dwUnk_4A4A50 <<= 1;
	g_dwUnk_4A4A24 <<= 1;

	_asm
	{
		push   ebp
		mov    eax, g_dwUnk_4A4A28
		and    eax, g_dwUnk_4A4A20
		mov    edx, g_dwUnk_4A4A2C
		mov    edi, g_dwUnk_4A4A4C
		mov    ebp, g_dwUnk_4A4A1C
		sub    ecx, ecx
		sub    edi, 2
	L1:
		mov    ebx, eax
		add    eax, g_dwUnk_4A4A50
		shr    ebx, 0x10
		and    eax, g_dwUnk_4A4A20
		lea    esi, [ebp + ebx]
		mov    ebx, edx
		add    edx, g_dwUnk_4A4A24
		and    ebx, g_dwUnk_4A4A38
		shr    ebx, 0xe
		add    edi, 2
		sub    ch, ch
		mov    cl, byte ptr [esi + ebx]
		mov    ebx, g_dwUnk_4A4A3C
		mov    cl, byte ptr [ebx + ecx]
		dec    g_dwUnk_4A4A14
		mov    ch, cl
		mov    word ptr [edi], cx
		jg     L1
		pop    ebp
	}
}

// hover.exe:0x40F540
// floor/ceiling span, 2-byte pixel, 1 row, v shift 0x9 (row stride 128)
// FUNCTION: HOVER 0x40f540
void Hover_sub_40F540(void)
{
	g_dwUnk_4A4A14 = (DWORD)((int)g_dwUnk_4A4A14 / 2);
	g_dwUnk_4A4A50 <<= 1;
	g_dwUnk_4A4A24 <<= 1;

	_asm
	{
		push   ebp
		mov    eax, g_dwUnk_4A4A28
		and    eax, g_dwUnk_4A4A20
		mov    edx, g_dwUnk_4A4A2C
		mov    edi, g_dwUnk_4A4A4C
		mov    ebp, g_dwUnk_4A4A1C
		sub    ecx, ecx
		sub    edi, 2
	L1:
		mov    ebx, eax
		add    eax, g_dwUnk_4A4A50
		shr    ebx, 0x10
		and    eax, g_dwUnk_4A4A20
		lea    esi, [ebp + ebx]
		mov    ebx, edx
		add    edx, g_dwUnk_4A4A24
		and    ebx, g_dwUnk_4A4A38
		shr    ebx, 9
		add    edi, 2
		sub    ch, ch
		mov    cl, byte ptr [esi + ebx]
		mov    ebx, g_dwUnk_4A4A3C
		mov    cl, byte ptr [ebx + ecx]
		dec    g_dwUnk_4A4A14
		mov    ch, cl
		mov    word ptr [edi], cx
		jg     L1
		pop    ebp
	}
}

// hover.exe:0x40F5D0
// floor/ceiling span, 2-byte pixel, 1 row, v shift 0xA (row stride 64)
// FUNCTION: HOVER 0x40f5d0
void Hover_sub_40F5D0(void)
{
	g_dwUnk_4A4A14 = (DWORD)((int)g_dwUnk_4A4A14 / 2);
	g_dwUnk_4A4A50 <<= 1;
	g_dwUnk_4A4A24 <<= 1;

	_asm
	{
		push   ebp
		mov    eax, g_dwUnk_4A4A28
		and    eax, g_dwUnk_4A4A20
		mov    edx, g_dwUnk_4A4A2C
		mov    edi, g_dwUnk_4A4A4C
		mov    ebp, g_dwUnk_4A4A1C
		sub    ecx, ecx
		sub    edi, 2
	L1:
		mov    ebx, eax
		add    eax, g_dwUnk_4A4A50
		shr    ebx, 0x10
		and    eax, g_dwUnk_4A4A20
		lea    esi, [ebp + ebx]
		mov    ebx, edx
		add    edx, g_dwUnk_4A4A24
		and    ebx, g_dwUnk_4A4A38
		shr    ebx, 0xa
		add    edi, 2
		sub    ch, ch
		mov    cl, byte ptr [esi + ebx]
		mov    ebx, g_dwUnk_4A4A3C
		mov    cl, byte ptr [ebx + ecx]
		dec    g_dwUnk_4A4A14
		mov    ch, cl
		mov    word ptr [edi], cx
		jg     L1
		pop    ebp
	}
}

// hover.exe:0x40F6B0
// wall column, 1-byte pixel, 1 row per texel
// FUNCTION: HOVER 0x40f6b0
void Hover_sub_40F6B0(void)
{
	_asm
	{
		push   ebp
		mov    ebx, g_dwUnk_4A4A14
		mov    edi, g_dwUnk_4A4A4C
		mov    esi, g_dwUnk_4A4A1C
		mov    edx, g_dwUnk_4A4A40
		mov    ebp, g_dwUnk_4A4A3C
		sub    edi, g_dwUnk_4A4A30
		sub    eax, eax
	L1:
		mov    ecx, edx
		add    edx, g_dwUnk_4A4A50
		shr    ecx, 0x10
		add    edi, g_dwUnk_4A4A30
		and    edx, g_dwUnk_4A4A38
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
void Hover_sub_40F710(void)
{
	_asm
	{
		pushad
		mov    edi, g_dwUnk_4A4A4C
		mov    edx, g_dwUnk_4A4A18
		mov    ebx, g_dwUnk_4A4A50
		mov    esi, g_dwUnk_4A4A1C
		mov    ecx, g_dwUnk_4A4A20
		mov    ebp, g_dwUnk_4A4A14
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
void Hover_sub_40FA90(void)
{
	_asm
	{
		push   ebp
		mov    eax, g_dwUnk_4A4A28
		and    eax, g_dwUnk_4A4A20
		mov    edx, g_dwUnk_4A4A2C
		mov    edi, g_dwUnk_4A4A4C
		mov    ebp, g_dwUnk_4A4A1C
		sub    ecx, ecx
		dec    edi
	L1:
		mov    ebx, eax
		add    eax, g_dwUnk_4A4A50
		shr    ebx, 0x10
		and    eax, g_dwUnk_4A4A20
		lea    esi, [ebp + ebx]
		mov    ebx, edx
		add    edx, g_dwUnk_4A4A24
		and    ebx, g_dwUnk_4A4A38
		shr    ebx, 9
		inc    edi
		mov    cl, byte ptr [esi + ebx]
		mov    ebx, g_dwUnk_4A4A3C
		mov    cl, byte ptr [ebx + ecx]
		dec    g_dwUnk_4A4A14
		mov    byte ptr [edi], cl
		jg     L1
		pop    ebp
	}
}

// hover.exe:0x40FB40
// floor/ceiling span, 1-byte pixel, 1 row, v shift 0xA (row stride 64)
// FUNCTION: HOVER 0x40fb40
void Hover_sub_40FB40(void)
{
	_asm
	{
		push   ebp
		mov    eax, g_dwUnk_4A4A28
		and    eax, g_dwUnk_4A4A20
		mov    edx, g_dwUnk_4A4A2C
		mov    edi, g_dwUnk_4A4A4C
		mov    ebp, g_dwUnk_4A4A1C
		sub    ecx, ecx
		dec    edi
	L1:
		mov    ebx, eax
		add    eax, g_dwUnk_4A4A50
		shr    ebx, 0x10
		and    eax, g_dwUnk_4A4A20
		lea    esi, [ebp + ebx]
		mov    ebx, edx
		add    edx, g_dwUnk_4A4A24
		and    ebx, g_dwUnk_4A4A38
		shr    ebx, 0xa
		inc    edi
		mov    cl, byte ptr [esi + ebx]
		mov    ebx, g_dwUnk_4A4A3C
		mov    cl, byte ptr [ebx + ecx]
		dec    g_dwUnk_4A4A14
		mov    byte ptr [edi], cl
		jg     L1
		pop    ebp
	}
}

// hover.exe:0x40FC70
// floor/ceiling span, 1-byte pixel, 1 row, v shift 0xB (row stride 32)
// FUNCTION: HOVER 0x40fc70
void Hover_sub_40FC70(void)
{
	_asm
	{
		push   ebp
		mov    eax, g_dwUnk_4A4A28
		and    eax, g_dwUnk_4A4A20
		mov    edx, g_dwUnk_4A4A2C
		mov    edi, g_dwUnk_4A4A4C
		mov    ebp, g_dwUnk_4A4A1C
		sub    ecx, ecx
		dec    edi
	L1:
		mov    ebx, eax
		add    eax, g_dwUnk_4A4A50
		shr    ebx, 0x10
		and    eax, g_dwUnk_4A4A20
		lea    esi, [ebp + ebx]
		mov    ebx, edx
		add    edx, g_dwUnk_4A4A24
		and    ebx, g_dwUnk_4A4A38
		shr    ebx, 0xb
		inc    edi
		mov    cl, byte ptr [esi + ebx]
		mov    ebx, g_dwUnk_4A4A3C
		mov    cl, byte ptr [ebx + ecx]
		dec    g_dwUnk_4A4A14
		mov    byte ptr [edi], cl
		jg     L1
		pop    ebp
	}
}

// hover.exe:0x40FDD0
// floor/ceiling span, 1-byte pixel, 1 row, v shift 0xC (row stride 16)
// FUNCTION: HOVER 0x40fdd0
void Hover_sub_40FDD0(void)
{
	_asm
	{
		push   ebp
		mov    eax, g_dwUnk_4A4A28
		and    eax, g_dwUnk_4A4A20
		mov    edx, g_dwUnk_4A4A2C
		mov    edi, g_dwUnk_4A4A4C
		mov    ebp, g_dwUnk_4A4A1C
		sub    ecx, ecx
		dec    edi
	L1:
		mov    ebx, eax
		add    eax, g_dwUnk_4A4A50
		shr    ebx, 0x10
		and    eax, g_dwUnk_4A4A20
		lea    esi, [ebp + ebx]
		mov    ebx, edx
		add    edx, g_dwUnk_4A4A24
		and    ebx, g_dwUnk_4A4A38
		shr    ebx, 0xc
		inc    edi
		mov    cl, byte ptr [esi + ebx]
		mov    ebx, g_dwUnk_4A4A3C
		mov    cl, byte ptr [ebx + ecx]
		dec    g_dwUnk_4A4A14
		mov    byte ptr [edi], cl
		jg     L1
		pop    ebp
	}
}

// hover.exe:0x40FE70
// floor/ceiling span, 1-byte pixel, 1 row, v shift 0xD (row stride 8)
// FUNCTION: HOVER 0x40fe70
void Hover_sub_40FE70(void)
{
	_asm
	{
		push   ebp
		mov    eax, g_dwUnk_4A4A28
		and    eax, g_dwUnk_4A4A20
		mov    edx, g_dwUnk_4A4A2C
		mov    edi, g_dwUnk_4A4A4C
		mov    ebp, g_dwUnk_4A4A1C
		sub    ecx, ecx
		dec    edi
	L1:
		mov    ebx, eax
		add    eax, g_dwUnk_4A4A50
		shr    ebx, 0x10
		and    eax, g_dwUnk_4A4A20
		lea    esi, [ebp + ebx]
		mov    ebx, edx
		add    edx, g_dwUnk_4A4A24
		and    ebx, g_dwUnk_4A4A38
		shr    ebx, 0xd
		inc    edi
		mov    cl, byte ptr [esi + ebx]
		mov    ebx, g_dwUnk_4A4A3C
		mov    cl, byte ptr [ebx + ecx]
		dec    g_dwUnk_4A4A14
		mov    byte ptr [edi], cl
		jg     L1
		pop    ebp
	}
}

// hover.exe:0x4101E0
// floor/ceiling span, 1-byte pixel, 1 row, v shift 0xE (row stride 4)
// FUNCTION: HOVER 0x4101e0
void Hover_sub_4101E0(void)
{
	_asm
	{
		push   ebp
		mov    eax, g_dwUnk_4A4A28
		and    eax, g_dwUnk_4A4A20
		mov    edx, g_dwUnk_4A4A2C
		mov    edi, g_dwUnk_4A4A4C
		mov    ebp, g_dwUnk_4A4A1C
		sub    ecx, ecx
		dec    edi
	L1:
		mov    ebx, eax
		add    eax, g_dwUnk_4A4A50
		shr    ebx, 0x10
		and    eax, g_dwUnk_4A4A20
		lea    esi, [ebp + ebx]
		mov    ebx, edx
		add    edx, g_dwUnk_4A4A24
		and    ebx, g_dwUnk_4A4A38
		shr    ebx, 0xe
		inc    edi
		mov    cl, byte ptr [esi + ebx]
		mov    ebx, g_dwUnk_4A4A3C
		mov    cl, byte ptr [ebx + ecx]
		dec    g_dwUnk_4A4A14
		mov    byte ptr [edi], cl
		jg     L1
		pop    ebp
	}
}

// hover.exe:0x4114D0
// wall column, 4-byte pixel, 4 rows per texel
// FUNCTION: HOVER 0x4114d0
void Hover_sub_4114D0(void)
{
	g_dwUnk_4A4A14 = (DWORD)((int)g_dwUnk_4A4A14 / 4);
	g_dwUnk_4A4A50 <<= 2;

	if ((int)g_dwUnk_4A4A14 > 0)
	{
		_asm
		{
			push   ebp
			mov    ebx, g_dwUnk_4A4A50
			mov    edi, g_dwUnk_4A4A4C
			mov    esi, g_dwUnk_4A4A1C
			mov    edx, g_dwUnk_4A4A40
			mov    ebp, g_dwUnk_4A4A3C
			sub    edi, g_dwUnk_4A4A30
			sub    eax, eax
		L1:
			mov    ecx, edx
			add    edx, ebx
			shr    ecx, 0x10
			add    edi, g_dwUnk_4A4A30
			and    edx, g_dwUnk_4A4A38
			mov    al, byte ptr [esi + ecx]
			mov    cl, byte ptr [ebp + eax]
			mov    ch, cl
			push   cx
			shl    ecx, 0x10
			pop    cx
			mov    dword ptr [edi], ecx
			add    edi, g_dwUnk_4A4A30
			mov    dword ptr [edi], ecx
			add    edi, g_dwUnk_4A4A30
			mov    dword ptr [edi], ecx
			add    edi, g_dwUnk_4A4A30
			dec    g_dwUnk_4A4A14
			mov    dword ptr [edi], ecx
			jg     L1
			pop    ebp
		}
	}
}

// hover.exe:0x411940
// floor/ceiling span, 4-byte pixel, 4 rows, v shift 0xB (row stride 32)
// FUNCTION: HOVER 0x411940
void Hover_sub_411940(void)
{
	g_dwUnk_4A4A14 = (DWORD)((int)g_dwUnk_4A4A14 / 4);
	g_dwUnk_4A4A50 <<= 2;
	g_dwUnk_4A4A24 <<= 2;

	_asm
	{
		push   ebp
		mov    eax, g_dwUnk_4A4A28
		and    eax, g_dwUnk_4A4A20
		mov    edx, g_dwUnk_4A4A2C
		mov    edi, g_dwUnk_4A4A4C
		mov    ebp, g_dwUnk_4A4A1C
		sub    ecx, ecx
		sub    edi, 4
	L1:
		mov    ebx, eax
		add    eax, g_dwUnk_4A4A50
		shr    ebx, 0x10
		and    eax, g_dwUnk_4A4A20
		lea    esi, [ebp + ebx]
		mov    ebx, edx
		add    edx, g_dwUnk_4A4A24
		and    ebx, g_dwUnk_4A4A38
		shr    ebx, 0xb
		add    edi, 4
		sub    ecx, ecx
		mov    cl, byte ptr [esi + ebx]
		mov    ebx, g_dwUnk_4A4A3C
		mov    cl, byte ptr [ebx + ecx]
		mov    ch, cl
		push   cx
		shl    ecx, 0x10
		pop    cx
		mov    dword ptr [edi], ecx
		mov    ebx, g_dwUnk_4A4A30
		mov    dword ptr [edi + ebx], ecx
		mov    dword ptr [edi + ebx*2], ecx
		add    ebx, g_dwUnk_4A4A30
		add    ebx, g_dwUnk_4A4A30
		mov    dword ptr [edi + ebx], ecx
		dec    g_dwUnk_4A4A14
		jg     L1
		pop    ebp
	}
}

// hover.exe:0x411FE0
// floor/ceiling span, 4-byte pixel, 4 rows, v shift 0xC (row stride 16)
// FUNCTION: HOVER 0x411fe0
void Hover_sub_411FE0(void)
{
	g_dwUnk_4A4A14 = (DWORD)((int)g_dwUnk_4A4A14 / 4);
	g_dwUnk_4A4A50 <<= 2;
	g_dwUnk_4A4A24 <<= 2;

	_asm
	{
		push   ebp
		mov    eax, g_dwUnk_4A4A28
		and    eax, g_dwUnk_4A4A20
		mov    edx, g_dwUnk_4A4A2C
		mov    edi, g_dwUnk_4A4A4C
		mov    ebp, g_dwUnk_4A4A1C
		sub    ecx, ecx
		sub    edi, 4
	L1:
		mov    ebx, eax
		add    eax, g_dwUnk_4A4A50
		shr    ebx, 0x10
		and    eax, g_dwUnk_4A4A20
		lea    esi, [ebp + ebx]
		mov    ebx, edx
		add    edx, g_dwUnk_4A4A24
		and    ebx, g_dwUnk_4A4A38
		shr    ebx, 0xc
		add    edi, 4
		sub    ecx, ecx
		mov    cl, byte ptr [esi + ebx]
		mov    ebx, g_dwUnk_4A4A3C
		mov    cl, byte ptr [ebx + ecx]
		mov    ch, cl
		push   cx
		shl    ecx, 0x10
		pop    cx
		mov    dword ptr [edi], ecx
		mov    ebx, g_dwUnk_4A4A30
		mov    dword ptr [edi + ebx], ecx
		mov    dword ptr [edi + ebx*2], ecx
		add    ebx, g_dwUnk_4A4A30
		add    ebx, g_dwUnk_4A4A30
		mov    dword ptr [edi + ebx], ecx
		dec    g_dwUnk_4A4A14
		jg     L1
		pop    ebp
	}
}

// hover.exe:0x4120A0
// floor/ceiling span, 4-byte pixel, 4 rows, v shift 0xD (row stride 8)
// FUNCTION: HOVER 0x4120a0
void Hover_sub_4120A0(void)
{
	g_dwUnk_4A4A14 = (DWORD)((int)g_dwUnk_4A4A14 / 4);
	g_dwUnk_4A4A50 <<= 2;
	g_dwUnk_4A4A24 <<= 2;

	_asm
	{
		push   ebp
		mov    eax, g_dwUnk_4A4A28
		and    eax, g_dwUnk_4A4A20
		mov    edx, g_dwUnk_4A4A2C
		mov    edi, g_dwUnk_4A4A4C
		mov    ebp, g_dwUnk_4A4A1C
		sub    ecx, ecx
		sub    edi, 4
	L1:
		mov    ebx, eax
		add    eax, g_dwUnk_4A4A50
		shr    ebx, 0x10
		and    eax, g_dwUnk_4A4A20
		lea    esi, [ebp + ebx]
		mov    ebx, edx
		add    edx, g_dwUnk_4A4A24
		and    ebx, g_dwUnk_4A4A38
		shr    ebx, 0xd
		add    edi, 4
		sub    ecx, ecx
		mov    cl, byte ptr [esi + ebx]
		mov    ebx, g_dwUnk_4A4A3C
		mov    cl, byte ptr [ebx + ecx]
		mov    ch, cl
		push   cx
		shl    ecx, 0x10
		pop    cx
		mov    dword ptr [edi], ecx
		mov    ebx, g_dwUnk_4A4A30
		mov    dword ptr [edi + ebx], ecx
		mov    dword ptr [edi + ebx*2], ecx
		add    ebx, g_dwUnk_4A4A30
		add    ebx, g_dwUnk_4A4A30
		mov    dword ptr [edi + ebx], ecx
		dec    g_dwUnk_4A4A14
		jg     L1
		pop    ebp
	}
}

// hover.exe:0x412160
// floor/ceiling span, 4-byte pixel, 4 rows, v shift 0xE (row stride 4)
// FUNCTION: HOVER 0x412160
void Hover_sub_412160(void)
{
	g_dwUnk_4A4A14 = (DWORD)((int)g_dwUnk_4A4A14 / 4);
	g_dwUnk_4A4A50 <<= 2;
	g_dwUnk_4A4A24 <<= 2;

	_asm
	{
		push   ebp
		mov    eax, g_dwUnk_4A4A28
		and    eax, g_dwUnk_4A4A20
		mov    edx, g_dwUnk_4A4A2C
		mov    edi, g_dwUnk_4A4A4C
		mov    ebp, g_dwUnk_4A4A1C
		sub    ecx, ecx
		sub    edi, 4
	L1:
		mov    ebx, eax
		add    eax, g_dwUnk_4A4A50
		shr    ebx, 0x10
		and    eax, g_dwUnk_4A4A20
		lea    esi, [ebp + ebx]
		mov    ebx, edx
		add    edx, g_dwUnk_4A4A24
		and    ebx, g_dwUnk_4A4A38
		shr    ebx, 0xe
		add    edi, 4
		sub    ecx, ecx
		mov    cl, byte ptr [esi + ebx]
		mov    ebx, g_dwUnk_4A4A3C
		mov    cl, byte ptr [ebx + ecx]
		mov    ch, cl
		push   cx
		shl    ecx, 0x10
		pop    cx
		mov    dword ptr [edi], ecx
		mov    ebx, g_dwUnk_4A4A30
		mov    dword ptr [edi + ebx], ecx
		mov    dword ptr [edi + ebx*2], ecx
		add    ebx, g_dwUnk_4A4A30
		add    ebx, g_dwUnk_4A4A30
		mov    dword ptr [edi + ebx], ecx
		dec    g_dwUnk_4A4A14
		jg     L1
		pop    ebp
	}
}

// hover.exe:0x412EF0
// wall column, 4-byte pixel, 1 row per texel
// FUNCTION: HOVER 0x412ef0
void Hover_sub_412EF0(void)
{
	_asm
	{
		push   ebp
		mov    ebx, g_dwUnk_4A4A14
		mov    edi, g_dwUnk_4A4A4C
		mov    esi, g_dwUnk_4A4A1C
		mov    edx, g_dwUnk_4A4A40
		mov    ebp, g_dwUnk_4A4A3C
		sub    edi, g_dwUnk_4A4A30
		sub    eax, eax
	L1:
		mov    ecx, edx
		add    edx, g_dwUnk_4A4A50
		shr    ecx, 0x10
		add    edi, g_dwUnk_4A4A30
		and    edx, g_dwUnk_4A4A38
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
void Hover_sub_412F50(void)
{
	_asm
	{
		pushad
		mov    edi, g_dwUnk_4A4A4C
		mov    edx, g_dwUnk_4A4A18
		mov    ebx, g_dwUnk_4A4A50
		mov    esi, g_dwUnk_4A4A1C
		mov    ecx, g_dwUnk_4A4A30
		mov    ebp, g_dwUnk_4A4A14
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
void Hover_sub_412FA0(void)
{
	_asm
	{
		pushad
		mov    edi, g_dwUnk_4A4A4C
		mov    edx, g_dwUnk_4A4A18
		mov    ebx, g_dwUnk_4A4A50
		mov    esi, g_dwUnk_4A4A1C
		mov    ecx, g_dwUnk_4A4A30
		mov    ebp, g_dwUnk_4A4A14
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
void Hover_sub_413350(void)
{
	g_dwUnk_4A4A14 = (DWORD)((int)g_dwUnk_4A4A14 / 4);
	g_dwUnk_4A4A50 <<= 2;
	g_dwUnk_4A4A24 <<= 2;

	_asm
	{
		push   ebp
		mov    eax, g_dwUnk_4A4A28
		and    eax, g_dwUnk_4A4A20
		mov    edx, g_dwUnk_4A4A2C
		mov    edi, g_dwUnk_4A4A4C
		mov    ebp, g_dwUnk_4A4A1C
		sub    ecx, ecx
		sub    edi, 4
	L1:
		mov    ebx, eax
		add    eax, g_dwUnk_4A4A50
		shr    ebx, 0x10
		and    eax, g_dwUnk_4A4A20
		lea    esi, [ebp + ebx]
		mov    ebx, edx
		add    edx, g_dwUnk_4A4A24
		and    ebx, g_dwUnk_4A4A38
		shr    ebx, 9
		add    edi, 4
		sub    ecx, ecx
		mov    cl, byte ptr [esi + ebx]
		mov    ebx, g_dwUnk_4A4A3C
		mov    cl, byte ptr [ebx + ecx]
		mov    ch, cl
		push   cx
		shl    ecx, 0x10
		pop    cx
		dec    g_dwUnk_4A4A14
		mov    dword ptr [edi], ecx
		jg     L1
		pop    ebp
	}
}

// hover.exe:0x4133F0
// floor/ceiling span, 4-byte pixel, 1 row, v shift 0xA (row stride 64)
// FUNCTION: HOVER 0x4133f0
void Hover_sub_4133F0(void)
{
	g_dwUnk_4A4A14 = (DWORD)((int)g_dwUnk_4A4A14 / 4);
	g_dwUnk_4A4A50 <<= 2;
	g_dwUnk_4A4A24 <<= 2;

	_asm
	{
		push   ebp
		mov    eax, g_dwUnk_4A4A28
		and    eax, g_dwUnk_4A4A20
		mov    edx, g_dwUnk_4A4A2C
		mov    edi, g_dwUnk_4A4A4C
		mov    ebp, g_dwUnk_4A4A1C
		sub    ecx, ecx
		sub    edi, 4
	L1:
		mov    ebx, eax
		add    eax, g_dwUnk_4A4A50
		shr    ebx, 0x10
		and    eax, g_dwUnk_4A4A20
		lea    esi, [ebp + ebx]
		mov    ebx, edx
		add    edx, g_dwUnk_4A4A24
		and    ebx, g_dwUnk_4A4A38
		shr    ebx, 0xa
		add    edi, 4
		sub    ecx, ecx
		mov    cl, byte ptr [esi + ebx]
		mov    ebx, g_dwUnk_4A4A3C
		mov    cl, byte ptr [ebx + ecx]
		mov    ch, cl
		push   cx
		shl    ecx, 0x10
		pop    cx
		dec    g_dwUnk_4A4A14
		mov    dword ptr [edi], ecx
		jg     L1
		pop    ebp
	}
}

// hover.exe:0x413490
// floor/ceiling span, 4-byte pixel, 1 row, v shift 0xB (row stride 32)
// FUNCTION: HOVER 0x413490
void Hover_sub_413490(void)
{
	g_dwUnk_4A4A14 = (DWORD)((int)g_dwUnk_4A4A14 / 4);
	g_dwUnk_4A4A50 <<= 2;
	g_dwUnk_4A4A24 <<= 2;

	_asm
	{
		push   ebp
		mov    eax, g_dwUnk_4A4A28
		and    eax, g_dwUnk_4A4A20
		mov    edx, g_dwUnk_4A4A2C
		mov    edi, g_dwUnk_4A4A4C
		mov    ebp, g_dwUnk_4A4A1C
		sub    ecx, ecx
		sub    edi, 4
	L1:
		mov    ebx, eax
		add    eax, g_dwUnk_4A4A50
		shr    ebx, 0x10
		and    eax, g_dwUnk_4A4A20
		lea    esi, [ebp + ebx]
		mov    ebx, edx
		add    edx, g_dwUnk_4A4A24
		and    ebx, g_dwUnk_4A4A38
		shr    ebx, 0xb
		add    edi, 4
		sub    ecx, ecx
		mov    cl, byte ptr [esi + ebx]
		mov    ebx, g_dwUnk_4A4A3C
		mov    cl, byte ptr [ebx + ecx]
		mov    ch, cl
		push   cx
		shl    ecx, 0x10
		pop    cx
		dec    g_dwUnk_4A4A14
		mov    dword ptr [edi], ecx
		jg     L1
		pop    ebp
	}
}

// hover.exe:0x413530
// floor/ceiling span, 4-byte pixel, 1 row, v shift 0xC (row stride 16)
// FUNCTION: HOVER 0x413530
void Hover_sub_413530(void)
{
	g_dwUnk_4A4A14 = (DWORD)((int)g_dwUnk_4A4A14 / 4);
	g_dwUnk_4A4A50 <<= 2;
	g_dwUnk_4A4A24 <<= 2;

	_asm
	{
		push   ebp
		mov    eax, g_dwUnk_4A4A28
		and    eax, g_dwUnk_4A4A20
		mov    edx, g_dwUnk_4A4A2C
		mov    edi, g_dwUnk_4A4A4C
		mov    ebp, g_dwUnk_4A4A1C
		sub    ecx, ecx
		sub    edi, 4
	L1:
		mov    ebx, eax
		add    eax, g_dwUnk_4A4A50
		shr    ebx, 0x10
		and    eax, g_dwUnk_4A4A20
		lea    esi, [ebp + ebx]
		mov    ebx, edx
		add    edx, g_dwUnk_4A4A24
		and    ebx, g_dwUnk_4A4A38
		shr    ebx, 0xc
		add    edi, 4
		sub    ecx, ecx
		mov    cl, byte ptr [esi + ebx]
		mov    ebx, g_dwUnk_4A4A3C
		mov    cl, byte ptr [ebx + ecx]
		mov    ch, cl
		push   cx
		shl    ecx, 0x10
		pop    cx
		dec    g_dwUnk_4A4A14
		mov    dword ptr [edi], ecx
		jg     L1
		pop    ebp
	}
}

// hover.exe:0x4135D0
// floor/ceiling span, 4-byte pixel, 1 row, v shift 0xD (row stride 8)
// FUNCTION: HOVER 0x4135d0
void Hover_sub_4135D0(void)
{
	g_dwUnk_4A4A14 = (DWORD)((int)g_dwUnk_4A4A14 / 4);
	g_dwUnk_4A4A50 <<= 2;
	g_dwUnk_4A4A24 <<= 2;

	_asm
	{
		push   ebp
		mov    eax, g_dwUnk_4A4A28
		and    eax, g_dwUnk_4A4A20
		mov    edx, g_dwUnk_4A4A2C
		mov    edi, g_dwUnk_4A4A4C
		mov    ebp, g_dwUnk_4A4A1C
		sub    ecx, ecx
		sub    edi, 4
	L1:
		mov    ebx, eax
		add    eax, g_dwUnk_4A4A50
		shr    ebx, 0x10
		and    eax, g_dwUnk_4A4A20
		lea    esi, [ebp + ebx]
		mov    ebx, edx
		add    edx, g_dwUnk_4A4A24
		and    ebx, g_dwUnk_4A4A38
		shr    ebx, 0xd
		add    edi, 4
		sub    ecx, ecx
		mov    cl, byte ptr [esi + ebx]
		mov    ebx, g_dwUnk_4A4A3C
		mov    cl, byte ptr [ebx + ecx]
		mov    ch, cl
		push   cx
		shl    ecx, 0x10
		pop    cx
		dec    g_dwUnk_4A4A14
		mov    dword ptr [edi], ecx
		jg     L1
		pop    ebp
	}
}

// hover.exe:0x413670
// floor/ceiling span, 4-byte pixel, 1 row, v shift 0xE (row stride 4)
// FUNCTION: HOVER 0x413670
void Hover_sub_413670(void)
{
	g_dwUnk_4A4A14 = (DWORD)((int)g_dwUnk_4A4A14 / 4);
	g_dwUnk_4A4A50 <<= 2;
	g_dwUnk_4A4A24 <<= 2;

	_asm
	{
		push   ebp
		mov    eax, g_dwUnk_4A4A28
		and    eax, g_dwUnk_4A4A20
		mov    edx, g_dwUnk_4A4A2C
		mov    edi, g_dwUnk_4A4A4C
		mov    ebp, g_dwUnk_4A4A1C
		sub    ecx, ecx
		sub    edi, 4
	L1:
		mov    ebx, eax
		add    eax, g_dwUnk_4A4A50
		shr    ebx, 0x10
		and    eax, g_dwUnk_4A4A20
		lea    esi, [ebp + ebx]
		mov    ebx, edx
		add    edx, g_dwUnk_4A4A24
		and    ebx, g_dwUnk_4A4A38
		shr    ebx, 0xe
		add    edi, 4
		sub    ecx, ecx
		mov    cl, byte ptr [esi + ebx]
		mov    ebx, g_dwUnk_4A4A3C
		mov    cl, byte ptr [ebx + ecx]
		mov    ch, cl
		push   cx
		shl    ecx, 0x10
		pop    cx
		dec    g_dwUnk_4A4A14
		mov    dword ptr [edi], ecx
		jg     L1
		pop    ebp
	}
}

// hover.exe:0x414220
// floor/ceiling span, 2-byte pixel, 2 rows, v shift 0x8 (row stride 256)
// FUNCTION: HOVER 0x414220
void Hover_sub_414220(void)
{
	g_dwUnk_4A4A14 = (DWORD)((int)g_dwUnk_4A4A14 / 2);
	g_dwUnk_4A4A50 <<= 1;
	g_dwUnk_4A4A24 <<= 1;

	_asm
	{
		push   ebp
		mov    eax, g_dwUnk_4A4A28
		and    eax, g_dwUnk_4A4A20
		mov    edx, g_dwUnk_4A4A2C
		mov    edi, g_dwUnk_4A4A4C
		mov    ebp, g_dwUnk_4A4A1C
		sub    ecx, ecx
		sub    edi, 2
	L1:
		mov    ebx, eax
		add    eax, g_dwUnk_4A4A50
		shr    ebx, 0x10
		and    eax, g_dwUnk_4A4A20
		lea    esi, [ebp + ebx]
		mov    ebx, edx
		add    edx, g_dwUnk_4A4A24
		and    ebx, g_dwUnk_4A4A38
		shr    ebx, 8
		add    edi, 2
		sub    ch, ch
		mov    cl, byte ptr [esi + ebx]
		mov    ebx, g_dwUnk_4A4A3C
		mov    cl, byte ptr [ebx + ecx]
		dec    g_dwUnk_4A4A14
		mov    ch, cl
		mov    word ptr [edi], cx
		mov    ebx, g_dwUnk_4A4A30
		mov    word ptr [edi + ebx], cx
		jg     L1
		pop    ebp
	}
}

// hover.exe:0x424FB0
// floor/ceiling span, 4-byte pixel, 1 row, v shift 0x8 (row stride 256)
// FUNCTION: HOVER 0x424fb0
void Hover_sub_424FB0(void)
{
	g_dwUnk_4A4A14 = (DWORD)((int)g_dwUnk_4A4A14 / 4);
	g_dwUnk_4A4A50 <<= 2;
	g_dwUnk_4A4A24 <<= 2;

	_asm
	{
		push   ebp
		mov    eax, g_dwUnk_4A4A28
		and    eax, g_dwUnk_4A4A20
		mov    edx, g_dwUnk_4A4A2C
		mov    edi, g_dwUnk_4A4A4C
		mov    ebp, g_dwUnk_4A4A1C
		sub    ecx, ecx
		sub    edi, 4
	L1:
		mov    ebx, eax
		add    eax, g_dwUnk_4A4A50
		shr    ebx, 0x10
		and    eax, g_dwUnk_4A4A20
		lea    esi, [ebp + ebx]
		mov    ebx, edx
		add    edx, g_dwUnk_4A4A24
		and    ebx, g_dwUnk_4A4A38
		shr    ebx, 8
		add    edi, 4
		sub    ecx, ecx
		mov    cl, byte ptr [esi + ebx]
		mov    ebx, g_dwUnk_4A4A3C
		mov    cl, byte ptr [ebx + ecx]
		mov    ch, cl
		push   cx
		shl    ecx, 0x10
		pop    cx
		dec    g_dwUnk_4A4A14
		mov    dword ptr [edi], ecx
		jg     L1
		pop    ebp
	}
}
