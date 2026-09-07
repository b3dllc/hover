// spandrawC.cpp : the "zoomed" span drawers of the software renderer
//
// Reconstructed from hover.exe 0x435D30..0x437EF0, the last third of the
// hand-written span-drawer cluster whose addresses CSub410F20::Sub_410F20
// stores in the render context's drawer table.  Every function here is a
// leaf with no arguments, called through a `void (*)()` slot, and every
// body is one hand-written `_asm` block preceded by three C statements
// that halve/quarter the span length and scale the texture steps to match.
//
// Three blocks, each replicating one texel over a rectangular screen block
// (which is why the span length is divided and the u/v steps multiplied):
//
//   0x435D30..0x4366C0  Span2x2  2 pixels wide (word store)  x 2 rows
//   0x4366D0..0x4371B0  Span4x2  4 pixels wide (dword store) x 2 rows
//   0x4371C0..0x437EF0  Span4x4  4 pixels wide (dword store) x 4 rows
//
// and inside each block four variants:
//
//   ..._Var     v shift taken from g_dwUnk_4A4A48 at run time (`sar ebx,cl`)
//   ...Lit_Var  the same, but the texel is passed through the shade/remap
//               table at g_dwUnk_4A4A3C ("lit" span)
//   ..._Sn      v shift baked in as an immediate (texture width 2^(16-n),
//               n = 6..15), direct texel
//   ...Lit_Sn   v shift baked in, texel through the shade/remap table
//
// The inner loop is the usual affine texture walk: u = g_dwUnk_4A4A28
// (masked with g_dwUnk_4A4A20, stepped by g_dwUnk_4A4A50), v =
// g_dwUnk_4A4A2C (masked with g_dwUnk_4A4A38, stepped by g_dwUnk_4A4A24),
// texel = g_dwUnk_4A4A1C[(u >> 16) + (v >> n)], destination g_dwUnk_4A4A4C
// with g_dwUnk_4A4A30 (fixed-shift variants) or g_dwUnk_4A4A34 (run-time
// shift variants) as the row stride, and g_dwUnk_4A4A14 as the counter.
//
// The `push ebp` / `pop ebp` around the blocks that use ebp as the texture
// base is part of the assembly, not the compiler's prologue: VC++ 2.x does
// not add ebp to the saved-register list for registers only an `_asm` block
// touches, and the original saves it in exactly that position.
//
// Two more spellings worth recording:
//   - the run-time-shift drawers guard the loop with `if (g_dwUnk_4A4A14--)`,
//     which is exactly the original's `dec [0x4A4A14] / test reg,reg / je`
//     over the value the division left in a register; the fixed-shift
//     drawers have no guard at all (their loop is bottom-tested with
//     `dec` / `jg`).
//   - the fixed-shift drawers are one macro per family, expanded once per
//     texture size.  A macro rather than an inline function because the
//     loop label has to be function-scoped and every expansion has to stay
//     a separate function (cf. rule 17 in notes/perfect-match.md).

#include "hover.h"
#include "texture.h"
#include "gameobj.h"
#include "texheap.h"
#include "spandraw.h"

/////////////////////////////////////////////////////////////////////////////
// The four families that differ only in the v shift.  One macro expansion
// per function: the loop label is function-scoped, so a macro (not an
// inline function) is what keeps every instantiation a separate function.


// 2 px x 2 rows, texel through the shade/remap table (0x435EB0 family)
#define SPAN2X2_LIT(shift) \
	g_dwUnk_4A4A14 = (DWORD)((long)g_dwUnk_4A4A14 / 2); \
	g_dwUnk_4A4A50 <<= 1; \
	g_dwUnk_4A4A24 <<= 1; \
 \
	_asm push ebp \
	_asm mov eax, g_dwUnk_4A4A28 \
	_asm and eax, g_dwUnk_4A4A20 \
	_asm mov edx, g_dwUnk_4A4A2C \
	_asm mov edi, g_dwUnk_4A4A4C \
	_asm mov ebp, g_dwUnk_4A4A1C \
	_asm sub ecx, ecx \
	_asm sub edi, 2 \
	_asm _sp_top: \
	_asm mov ebx, eax \
	_asm add eax, g_dwUnk_4A4A50 \
	_asm shr ebx, 0x10 \
	_asm and eax, g_dwUnk_4A4A20 \
	_asm lea esi, [ebp+ebx] \
	_asm mov ebx, edx \
	_asm add edx, g_dwUnk_4A4A24 \
	_asm and ebx, g_dwUnk_4A4A38 \
	_asm shr ebx, shift \
	_asm add edi, 2 \
	_asm sub ch, ch \
	_asm mov cl, byte ptr [esi+ebx] \
	_asm mov ebx, g_dwUnk_4A4A3C \
	_asm mov cl, byte ptr [ebx+ecx] \
	_asm dec g_dwUnk_4A4A14 \
	_asm mov ch, cl \
	_asm mov word ptr [edi], cx \
	_asm mov ebx, g_dwUnk_4A4A30 \
	_asm mov word ptr [edi+ebx], cx \
	_asm jg _sp_top \
	_asm pop ebp

// 2 px x 2 rows, direct texel (0x436130 family)
#define SPAN2X2(shift) \
	g_dwUnk_4A4A14 = (DWORD)((long)g_dwUnk_4A4A14 / 2); \
	g_dwUnk_4A4A50 <<= 1; \
	g_dwUnk_4A4A24 <<= 1; \
 \
	_asm push ebp \
	_asm mov eax, g_dwUnk_4A4A28 \
	_asm and eax, g_dwUnk_4A4A20 \
	_asm mov edx, g_dwUnk_4A4A2C \
	_asm mov edi, g_dwUnk_4A4A4C \
	_asm mov ebp, g_dwUnk_4A4A1C \
	_asm sub ecx, ecx \
	_asm sub edi, 2 \
	_asm _sp_top: \
	_asm mov ebx, eax \
	_asm add eax, g_dwUnk_4A4A50 \
	_asm shr ebx, 0x10 \
	_asm and eax, g_dwUnk_4A4A20 \
	_asm lea esi, [ebp+ebx] \
	_asm mov ebx, edx \
	_asm add edx, g_dwUnk_4A4A24 \
	_asm and ebx, g_dwUnk_4A4A38 \
	_asm shr ebx, shift \
	_asm add edi, 2 \
	_asm mov cl, byte ptr [esi+ebx] \
	_asm dec g_dwUnk_4A4A14 \
	_asm mov ch, cl \
	_asm mov word ptr [edi], cx \
	_asm mov ebx, g_dwUnk_4A4A30 \
	_asm mov word ptr [edi+ebx], cx \
	_asm jg _sp_top \
	_asm pop ebp

// 4 px x 2 rows, texel through the shade/remap table (0x436860 family)
#define SPAN4X2_LIT(shift) \
	g_dwUnk_4A4A14 = (DWORD)((long)g_dwUnk_4A4A14 / 4); \
	g_dwUnk_4A4A50 <<= 2; \
	g_dwUnk_4A4A24 <<= 2; \
 \
	_asm push ebp \
	_asm mov eax, g_dwUnk_4A4A28 \
	_asm and eax, g_dwUnk_4A4A20 \
	_asm mov edx, g_dwUnk_4A4A2C \
	_asm mov edi, g_dwUnk_4A4A4C \
	_asm mov ebp, g_dwUnk_4A4A1C \
	_asm sub ecx, ecx \
	_asm sub edi, 4 \
	_asm _sp_top: \
	_asm mov ebx, eax \
	_asm add eax, g_dwUnk_4A4A50 \
	_asm shr ebx, 0x10 \
	_asm and eax, g_dwUnk_4A4A20 \
	_asm lea esi, [ebp+ebx] \
	_asm mov ebx, edx \
	_asm add edx, g_dwUnk_4A4A24 \
	_asm and ebx, g_dwUnk_4A4A38 \
	_asm shr ebx, shift \
	_asm add edi, 4 \
	_asm sub ecx, ecx \
	_asm mov cl, byte ptr [esi+ebx] \
	_asm mov ebx, g_dwUnk_4A4A3C \
	_asm mov cl, byte ptr [ebx+ecx] \
	_asm mov ch, cl \
	_asm push cx \
	_asm shl ecx, 0x10 \
	_asm pop cx \
	_asm dec g_dwUnk_4A4A14 \
	_asm mov dword ptr [edi], ecx \
	_asm mov ebx, g_dwUnk_4A4A30 \
	_asm mov dword ptr [edi+ebx], ecx \
	_asm jg _sp_top \
	_asm pop ebp

// 4 px x 2 rows, direct texel (0x436B80 family)
#define SPAN4X2(shift) \
	g_dwUnk_4A4A14 = (DWORD)((long)g_dwUnk_4A4A14 / 4); \
	g_dwUnk_4A4A50 <<= 2; \
	g_dwUnk_4A4A24 <<= 2; \
 \
	_asm push ebp \
	_asm mov eax, g_dwUnk_4A4A28 \
	_asm and eax, g_dwUnk_4A4A20 \
	_asm mov edx, g_dwUnk_4A4A2C \
	_asm mov edi, g_dwUnk_4A4A4C \
	_asm mov ebp, g_dwUnk_4A4A1C \
	_asm sub ecx, ecx \
	_asm sub edi, 4 \
	_asm _sp_top: \
	_asm mov ebx, eax \
	_asm add eax, g_dwUnk_4A4A50 \
	_asm shr ebx, 0x10 \
	_asm and eax, g_dwUnk_4A4A20 \
	_asm lea esi, [ebp+ebx] \
	_asm mov ebx, edx \
	_asm add edx, g_dwUnk_4A4A24 \
	_asm and ebx, g_dwUnk_4A4A38 \
	_asm shr ebx, shift \
	_asm add edi, 4 \
	_asm mov cl, byte ptr [esi+ebx] \
	_asm mov ch, cl \
	_asm push cx \
	_asm shl ecx, 0x10 \
	_asm pop cx \
	_asm dec g_dwUnk_4A4A14 \
	_asm mov dword ptr [edi], ecx \
	_asm mov ebx, g_dwUnk_4A4A30 \
	_asm mov dword ptr [edi+ebx], ecx \
	_asm jg _sp_top \
	_asm pop ebp

// 4 px x 4 rows, texel through the shade/remap table (0x4373A0 family)
#define SPAN4X4_LIT(shift) \
	g_dwUnk_4A4A14 = (DWORD)((long)g_dwUnk_4A4A14 / 4); \
	g_dwUnk_4A4A50 <<= 2; \
	g_dwUnk_4A4A24 <<= 2; \
 \
	_asm push ebp \
	_asm mov eax, g_dwUnk_4A4A28 \
	_asm and eax, g_dwUnk_4A4A20 \
	_asm mov edx, g_dwUnk_4A4A2C \
	_asm mov edi, g_dwUnk_4A4A4C \
	_asm mov ebp, g_dwUnk_4A4A1C \
	_asm sub ecx, ecx \
	_asm sub edi, 4 \
	_asm _sp_top: \
	_asm mov ebx, eax \
	_asm add eax, g_dwUnk_4A4A50 \
	_asm shr ebx, 0x10 \
	_asm and eax, g_dwUnk_4A4A20 \
	_asm lea esi, [ebp+ebx] \
	_asm mov ebx, edx \
	_asm add edx, g_dwUnk_4A4A24 \
	_asm and ebx, g_dwUnk_4A4A38 \
	_asm shr ebx, shift \
	_asm add edi, 4 \
	_asm sub ecx, ecx \
	_asm mov cl, byte ptr [esi+ebx] \
	_asm mov ebx, g_dwUnk_4A4A3C \
	_asm mov cl, byte ptr [ebx+ecx] \
	_asm mov ch, cl \
	_asm push cx \
	_asm shl ecx, 0x10 \
	_asm pop cx \
	_asm mov dword ptr [edi], ecx \
	_asm mov ebx, g_dwUnk_4A4A30 \
	_asm mov dword ptr [edi+ebx], ecx \
	_asm mov dword ptr [edi+ebx*2], ecx \
	_asm add ebx, g_dwUnk_4A4A30 \
	_asm add ebx, g_dwUnk_4A4A30 \
	_asm mov dword ptr [edi+ebx], ecx \
	_asm dec g_dwUnk_4A4A14 \
	_asm jg _sp_top \
	_asm pop ebp

// 4 px x 4 rows, direct texel (0x437820 family)
#define SPAN4X4(shift) \
	g_dwUnk_4A4A14 = (DWORD)((long)g_dwUnk_4A4A14 / 4); \
	g_dwUnk_4A4A50 <<= 2; \
	g_dwUnk_4A4A24 <<= 2; \
 \
	_asm push ebp \
	_asm mov eax, g_dwUnk_4A4A28 \
	_asm and eax, g_dwUnk_4A4A20 \
	_asm mov edx, g_dwUnk_4A4A2C \
	_asm mov edi, g_dwUnk_4A4A4C \
	_asm mov ebp, g_dwUnk_4A4A1C \
	_asm sub ecx, ecx \
	_asm sub edi, 4 \
	_asm _sp_top: \
	_asm mov ebx, eax \
	_asm add eax, g_dwUnk_4A4A50 \
	_asm shr ebx, 0x10 \
	_asm and eax, g_dwUnk_4A4A20 \
	_asm lea esi, [ebp+ebx] \
	_asm mov ebx, edx \
	_asm add edx, g_dwUnk_4A4A24 \
	_asm and ebx, g_dwUnk_4A4A38 \
	_asm shr ebx, shift \
	_asm add edi, 4 \
	_asm mov cl, byte ptr [esi+ebx] \
	_asm mov ch, cl \
	_asm push cx \
	_asm shl ecx, 0x10 \
	_asm pop cx \
	_asm mov dword ptr [edi], ecx \
	_asm mov ebx, g_dwUnk_4A4A30 \
	_asm mov dword ptr [edi+ebx], ecx \
	_asm mov dword ptr [edi+ebx*2], ecx \
	_asm add ebx, g_dwUnk_4A4A30 \
	_asm add ebx, g_dwUnk_4A4A30 \
	_asm mov dword ptr [edi+ebx], ecx \
	_asm dec g_dwUnk_4A4A14 \
	_asm jg _sp_top \
	_asm pop ebp

/////////////////////////////////////////////////////////////////////////////
// Span2x2: one texel over a 2 x 2 screen block (word store, two rows)


// hover.exe:0x435D30  2 px x 2 rows, run-time v shift, direct texel
// FUNCTION: HOVER 0x435d30
void Span2x2_Var(void)
{
	g_dwUnk_4A4A14 = (DWORD)((long)g_dwUnk_4A4A14 / 2);
	g_dwUnk_4A4A50 <<= 1;
	g_dwUnk_4A4A24 <<= 1;

	if (g_dwUnk_4A4A14--)
	{
		_asm mov cl, byte ptr g_dwUnk_4A4A48
		_asm mov edx, g_dwUnk_4A4A34
		_asm mov esi, 2
		_asm _sp_top:
		_asm mov ebx, g_dwUnk_4A4A38
		_asm mov eax, g_dwUnk_4A4A20
		_asm and ebx, g_dwUnk_4A4A2C
		_asm and eax, g_dwUnk_4A4A28
		_asm sar eax, 0x10
		_asm mov edi, g_dwUnk_4A4A1C
		_asm sar ebx, cl
		_asm add ebx, eax
		_asm xor eax, eax
		_asm mov al, byte ptr [ebx+edi]
		_asm mov edi, eax
		_asm shl edi, 8
		_asm or eax, edi
		_asm mov edi, g_dwUnk_4A4A4C
		_asm mov word ptr [edi], ax
		_asm mov edi, g_dwUnk_4A4A4C
		_asm mov word ptr [edi+edx], ax
		_asm mov eax, g_dwUnk_4A4A50
		_asm mov ebx, g_dwUnk_4A4A24
		_asm mov edi, g_dwUnk_4A4A14
		_asm add g_dwUnk_4A4A4C, esi
		_asm add g_dwUnk_4A4A28, eax
		_asm add g_dwUnk_4A4A2C, ebx
		_asm dec g_dwUnk_4A4A14
		_asm test edi, edi
		_asm jne _sp_top
	}
}

// hover.exe:0x435DF0  2 px x 2 rows, run-time v shift, shade/remap table
// FUNCTION: HOVER 0x435df0
void Span2x2Lit_Var(void)
{
	g_dwUnk_4A4A14 = (DWORD)((long)g_dwUnk_4A4A14 / 2);
	g_dwUnk_4A4A50 <<= 1;
	g_dwUnk_4A4A24 <<= 1;

	if (g_dwUnk_4A4A14--)
	{
		_asm mov cl, byte ptr g_dwUnk_4A4A48
		_asm mov edx, g_dwUnk_4A4A34
		_asm mov esi, 2
		_asm _sp_top:
		_asm mov edi, g_dwUnk_4A4A38
		_asm mov eax, g_dwUnk_4A4A20
		_asm and edi, g_dwUnk_4A4A2C
		_asm and eax, g_dwUnk_4A4A28
		_asm sar eax, 0x10
		_asm xor ebx, ebx
		_asm sar edi, cl
		_asm add edi, eax
		_asm mov eax, g_dwUnk_4A4A1C
		_asm mov bl, byte ptr [edi+eax]
		_asm mov edi, g_dwUnk_4A4A3C
		_asm xor eax, eax
		_asm mov al, byte ptr [ebx+edi]
		_asm mov edi, eax
		_asm shl edi, 8
		_asm or eax, edi
		_asm mov edi, g_dwUnk_4A4A4C
		_asm mov word ptr [edi], ax
		_asm mov edi, g_dwUnk_4A4A4C
		_asm mov word ptr [edi+edx], ax
		_asm mov eax, g_dwUnk_4A4A50
		_asm mov ebx, g_dwUnk_4A4A24
		_asm mov edi, g_dwUnk_4A4A14
		_asm add g_dwUnk_4A4A4C, esi
		_asm add g_dwUnk_4A4A28, eax
		_asm add g_dwUnk_4A4A2C, ebx
		_asm dec g_dwUnk_4A4A14
		_asm test edi, edi
		_asm jne _sp_top
	}
}

// hover.exe:0x435EB0  2 px x 2 rows, v shift 6, shade/remap table
// FUNCTION: HOVER 0x435eb0
void Span2x2Lit_S6(void)
{
	SPAN2X2_LIT(6)
}

// hover.exe:0x435F50  2 px x 2 rows, v shift 7, shade/remap table
// FUNCTION: HOVER 0x435f50
void Span2x2Lit_S7(void)
{
	SPAN2X2_LIT(7)
}

// hover.exe:0x435FF0  2 px x 2 rows, v shift 9, shade/remap table
// FUNCTION: HOVER 0x435ff0
void Span2x2Lit_S9(void)
{
	SPAN2X2_LIT(9)
}

// hover.exe:0x436090  2 px x 2 rows, v shift 15, shade/remap table
// FUNCTION: HOVER 0x436090
void Span2x2Lit_S15(void)
{
	SPAN2X2_LIT(15)
}

// hover.exe:0x436130  2 px x 2 rows, v shift 6, direct texel
// FUNCTION: HOVER 0x436130
void Span2x2_S6(void)
{
	SPAN2X2(6)
}

// hover.exe:0x4361C0  2 px x 2 rows, v shift 7, direct texel
// FUNCTION: HOVER 0x4361c0
void Span2x2_S7(void)
{
	SPAN2X2(7)
}

// hover.exe:0x436250  2 px x 2 rows, v shift 8, direct texel
// FUNCTION: HOVER 0x436250
void Span2x2_S8(void)
{
	SPAN2X2(8)
}

// hover.exe:0x4362E0  2 px x 2 rows, v shift 9, direct texel
// FUNCTION: HOVER 0x4362e0
void Span2x2_S9(void)
{
	SPAN2X2(9)
}

// hover.exe:0x436370  2 px x 2 rows, v shift 10, direct texel
// FUNCTION: HOVER 0x436370
void Span2x2_S10(void)
{
	SPAN2X2(10)
}

// hover.exe:0x436400  2 px x 2 rows, v shift 11, direct texel
// FUNCTION: HOVER 0x436400
void Span2x2_S11(void)
{
	SPAN2X2(11)
}

// hover.exe:0x436490  2 px x 2 rows, v shift 12, direct texel
// FUNCTION: HOVER 0x436490
void Span2x2_S12(void)
{
	SPAN2X2(12)
}

// hover.exe:0x436520  2 px x 2 rows, v shift 13, direct texel
// FUNCTION: HOVER 0x436520
void Span2x2_S13(void)
{
	SPAN2X2(13)
}

// hover.exe:0x4365B0  2 px x 2 rows, v shift 14, direct texel
// FUNCTION: HOVER 0x4365b0
void Span2x2_S14(void)
{
	SPAN2X2(14)
}

// hover.exe:0x436640  2 px x 2 rows, v shift 15, direct texel
// FUNCTION: HOVER 0x436640
void Span2x2_S15(void)
{
	SPAN2X2(15)
}

/////////////////////////////////////////////////////////////////////////////
// Span4x2: one texel over a 4 x 2 screen block (dword store, two rows)


// hover.exe:0x4366D0  4 px x 2 rows, run-time v shift, direct texel
// FUNCTION: HOVER 0x4366d0
void Span4x2_Var(void)
{
	g_dwUnk_4A4A14 = (DWORD)((long)g_dwUnk_4A4A14 / 4);
	g_dwUnk_4A4A50 <<= 2;
	g_dwUnk_4A4A24 <<= 2;

	if (g_dwUnk_4A4A14--)
	{
		_asm mov cl, byte ptr g_dwUnk_4A4A48
		_asm mov edx, g_dwUnk_4A4A34
		_asm mov esi, 4
		_asm _sp_top:
		_asm mov ebx, g_dwUnk_4A4A38
		_asm mov eax, g_dwUnk_4A4A20
		_asm and ebx, g_dwUnk_4A4A2C
		_asm and eax, g_dwUnk_4A4A28
		_asm sar eax, 0x10
		_asm mov edi, g_dwUnk_4A4A1C
		_asm sar ebx, cl
		_asm add ebx, eax
		_asm xor eax, eax
		_asm mov al, byte ptr [ebx+edi]
		_asm mov ebx, g_dwUnk_4A4A4C
		_asm mov edi, eax
		_asm shl edi, 8
		_asm or eax, edi
		_asm mov edi, eax
		_asm shl edi, 0x10
		_asm or eax, edi
		_asm mov dword ptr [ebx], eax
		_asm mov ebx, g_dwUnk_4A4A4C
		_asm mov dword ptr [ebx+edx], eax
		_asm mov eax, g_dwUnk_4A4A50
		_asm mov edi, g_dwUnk_4A4A24
		_asm mov ebx, g_dwUnk_4A4A14
		_asm add g_dwUnk_4A4A4C, esi
		_asm add g_dwUnk_4A4A28, eax
		_asm add g_dwUnk_4A4A2C, edi
		_asm dec g_dwUnk_4A4A14
		_asm test ebx, ebx
		_asm jne _sp_top
	}
}

// hover.exe:0x436790  4 px x 2 rows, run-time v shift, shade/remap table
// FUNCTION: HOVER 0x436790
void Span4x2Lit_Var(void)
{
	g_dwUnk_4A4A14 = (DWORD)((long)g_dwUnk_4A4A14 / 4);
	g_dwUnk_4A4A50 <<= 2;
	g_dwUnk_4A4A24 <<= 2;

	if (g_dwUnk_4A4A14--)
	{
		_asm mov cl, byte ptr g_dwUnk_4A4A48
		_asm mov edx, g_dwUnk_4A4A34
		_asm mov esi, 4
		_asm _sp_top:
		_asm mov edi, g_dwUnk_4A4A38
		_asm mov eax, g_dwUnk_4A4A20
		_asm and edi, g_dwUnk_4A4A2C
		_asm and eax, g_dwUnk_4A4A28
		_asm sar eax, 0x10
		_asm xor ebx, ebx
		_asm sar edi, cl
		_asm add edi, eax
		_asm mov eax, g_dwUnk_4A4A1C
		_asm mov bl, byte ptr [edi+eax]
		_asm mov edi, g_dwUnk_4A4A3C
		_asm xor eax, eax
		_asm mov al, byte ptr [ebx+edi]
		_asm mov ebx, g_dwUnk_4A4A4C
		_asm mov edi, eax
		_asm shl edi, 8
		_asm or eax, edi
		_asm mov edi, eax
		_asm shl edi, 0x10
		_asm or eax, edi
		_asm mov dword ptr [ebx], eax
		_asm mov ebx, g_dwUnk_4A4A4C
		_asm mov dword ptr [ebx+edx], eax
		_asm mov eax, g_dwUnk_4A4A50
		_asm mov edi, g_dwUnk_4A4A24
		_asm mov ebx, g_dwUnk_4A4A14
		_asm add g_dwUnk_4A4A4C, esi
		_asm add g_dwUnk_4A4A28, eax
		_asm add g_dwUnk_4A4A2C, edi
		_asm dec g_dwUnk_4A4A14
		_asm test ebx, ebx
		_asm jne _sp_top
	}
}

// hover.exe:0x436860  4 px x 2 rows, v shift 6, shade/remap table
// FUNCTION: HOVER 0x436860
void Span4x2Lit_S6(void)
{
	SPAN4X2_LIT(6)
}

// hover.exe:0x436900  4 px x 2 rows, v shift 7, shade/remap table
// FUNCTION: HOVER 0x436900
void Span4x2Lit_S7(void)
{
	SPAN4X2_LIT(7)
}

// hover.exe:0x4369A0  4 px x 2 rows, v shift 8, shade/remap table
// FUNCTION: HOVER 0x4369a0
void Span4x2Lit_S8(void)
{
	SPAN4X2_LIT(8)
}

// hover.exe:0x436A40  4 px x 2 rows, v shift 9, shade/remap table
// FUNCTION: HOVER 0x436a40
void Span4x2Lit_S9(void)
{
	SPAN4X2_LIT(9)
}

// hover.exe:0x436AE0  4 px x 2 rows, v shift 15, shade/remap table
// FUNCTION: HOVER 0x436ae0
void Span4x2Lit_S15(void)
{
	SPAN4X2_LIT(15)
}

// hover.exe:0x436B80  4 px x 2 rows, v shift 6, direct texel
// FUNCTION: HOVER 0x436b80
void Span4x2_S6(void)
{
	SPAN4X2(6)
}

// hover.exe:0x436C20  4 px x 2 rows, v shift 7, direct texel
// FUNCTION: HOVER 0x436c20
void Span4x2_S7(void)
{
	SPAN4X2(7)
}

// hover.exe:0x436CC0  4 px x 2 rows, v shift 8, direct texel
// FUNCTION: HOVER 0x436cc0
void Span4x2_S8(void)
{
	SPAN4X2(8)
}

// hover.exe:0x436D60  4 px x 2 rows, v shift 9, direct texel
// FUNCTION: HOVER 0x436d60
void Span4x2_S9(void)
{
	SPAN4X2(9)
}

// hover.exe:0x436E00  4 px x 2 rows, v shift 10, direct texel
// FUNCTION: HOVER 0x436e00
void Span4x2_S10(void)
{
	SPAN4X2(10)
}

// hover.exe:0x436EA0  4 px x 2 rows, v shift 11, direct texel
// FUNCTION: HOVER 0x436ea0
void Span4x2_S11(void)
{
	SPAN4X2(11)
}

// hover.exe:0x436F40  4 px x 2 rows, v shift 12, direct texel
// FUNCTION: HOVER 0x436f40
void Span4x2_S12(void)
{
	SPAN4X2(12)
}

// hover.exe:0x436FE0  4 px x 2 rows, v shift 13, direct texel
// FUNCTION: HOVER 0x436fe0
void Span4x2_S13(void)
{
	SPAN4X2(13)
}

// hover.exe:0x437080  4 px x 2 rows, v shift 14, direct texel
// FUNCTION: HOVER 0x437080
void Span4x2_S14(void)
{
	SPAN4X2(14)
}

// hover.exe:0x437120  4 px x 2 rows, v shift 15, direct texel
// FUNCTION: HOVER 0x437120
void Span4x2_S15(void)
{
	SPAN4X2(15)
}

/////////////////////////////////////////////////////////////////////////////
// Span4x4: one texel over a 4 x 4 screen block (dword store, four rows)


// hover.exe:0x4371C0  4 px x 4 rows, run-time v shift, direct texel
// FUNCTION: HOVER 0x4371c0
void Span4x4_Var(void)
{
	g_dwUnk_4A4A14 = (DWORD)((long)g_dwUnk_4A4A14 / 4);
	g_dwUnk_4A4A50 <<= 2;
	g_dwUnk_4A4A24 <<= 2;

	if (g_dwUnk_4A4A14--)
	{
		_asm mov cl, byte ptr g_dwUnk_4A4A48
		_asm mov edx, 4
		_asm _sp_top:
		_asm mov esi, g_dwUnk_4A4A38
		_asm mov eax, g_dwUnk_4A4A20
		_asm and esi, g_dwUnk_4A4A2C
		_asm and eax, g_dwUnk_4A4A28
		_asm sar eax, 0x10
		_asm xor ebx, ebx
		_asm sar esi, cl
		_asm add esi, eax
		_asm mov eax, g_dwUnk_4A4A1C
		_asm mov bl, byte ptr [esi+eax]
		_asm mov esi, g_dwUnk_4A4A4C
		_asm mov eax, ebx
		_asm shl eax, 8
		_asm or ebx, eax
		_asm mov eax, ebx
		_asm shl eax, 0x10
		_asm or ebx, eax
		_asm mov dword ptr [esi], ebx
		_asm mov esi, g_dwUnk_4A4A4C
		_asm mov eax, g_dwUnk_4A4A30
		_asm mov dword ptr [esi+eax], ebx
		_asm mov esi, g_dwUnk_4A4A30
		_asm mov eax, g_dwUnk_4A4A4C
		_asm mov dword ptr [eax+esi*2], ebx
		_asm mov esi, g_dwUnk_4A4A30
		_asm lea eax, [esi+esi*2]
		_asm mov esi, g_dwUnk_4A4A4C
		_asm mov dword ptr [eax+esi], ebx
		_asm mov ebx, g_dwUnk_4A4A50
		_asm mov eax, g_dwUnk_4A4A24
		_asm mov esi, g_dwUnk_4A4A14
		_asm add g_dwUnk_4A4A4C, edx
		_asm add g_dwUnk_4A4A28, ebx
		_asm add g_dwUnk_4A4A2C, eax
		_asm dec g_dwUnk_4A4A14
		_asm test esi, esi
		_asm jne _sp_top
	}
}

// hover.exe:0x4372A0  4 px x 4 rows, run-time v shift, shade/remap table
//
// The odd one out: this is the only drawer whose C statements are spelled
// *between* the assembly instructions rather than in front of the block.
// VC 2.x schedules the two streams together (which is why the prologue
// pushes come out interleaved with the length division everywhere else),
// and here the original's interleaving is only reproduced by writing the
// statements in that order: the division into a local that is stored one
// instruction later, then one `<<= 2` after each of the two `[0x4A4A30]`
// loads that precompute the 2x and 3x row strides the loop keeps in edx
// and esi.  The entry guard is assembly too (`mov ecx` / `dec` / `test`,
// with `pop ebp` after the skipped-over loop), not the `if (n--)` the
// other run-time-shift drawers use, because the compiler would have
// reloaded the counter into eax.
// FUNCTION: HOVER 0x4372a0
void Span4x4Lit_Var(void)
{
	long lLen = (long)g_dwUnk_4A4A14 / 4;

	_asm push ebp
	_asm mov edx, g_dwUnk_4A4A30

	g_dwUnk_4A4A14 = (DWORD)lLen;

	_asm add edx, edx
	_asm mov eax, g_dwUnk_4A4A30

	g_dwUnk_4A4A50 <<= 2;

	_asm lea esi, [eax+eax*2]

	_asm mov ecx, g_dwUnk_4A4A14

	g_dwUnk_4A4A24 <<= 2;

	_asm dec g_dwUnk_4A4A14
	_asm test ecx, ecx
	_asm je _sp_done
	{
		_asm mov cl, byte ptr g_dwUnk_4A4A48
		_asm mov edi, 4
		_asm _sp_top:
		_asm mov ebp, g_dwUnk_4A4A38
		_asm mov eax, g_dwUnk_4A4A20
		_asm and ebp, g_dwUnk_4A4A2C
		_asm and eax, g_dwUnk_4A4A28
		_asm sar eax, 0x10
		_asm xor ebx, ebx
		_asm sar ebp, cl
		_asm add ebp, eax
		_asm mov eax, g_dwUnk_4A4A1C
		_asm mov bl, byte ptr [ebp+eax]
		_asm mov ebp, g_dwUnk_4A4A3C
		_asm xor eax, eax
		_asm mov al, byte ptr [ebx+ebp]
		_asm mov ebp, g_dwUnk_4A4A4C
		_asm mov ebx, eax
		_asm shl ebx, 8
		_asm or eax, ebx
		_asm mov ebx, eax
		_asm shl ebx, 0x10
		_asm or eax, ebx
		_asm mov dword ptr [ebp], eax
		_asm mov ebp, g_dwUnk_4A4A30
		_asm mov ebx, g_dwUnk_4A4A4C
		_asm mov dword ptr [ebx+ebp], eax
		_asm mov ebx, g_dwUnk_4A4A4C
		_asm mov dword ptr [ebx+edx], eax
		_asm mov ebx, g_dwUnk_4A4A4C
		_asm mov dword ptr [ebx+esi], eax
		_asm mov eax, g_dwUnk_4A4A50
		_asm mov ebp, g_dwUnk_4A4A24
		_asm mov ebx, g_dwUnk_4A4A14
		_asm add g_dwUnk_4A4A4C, edi
		_asm add g_dwUnk_4A4A28, eax
		_asm add g_dwUnk_4A4A2C, ebp
		_asm dec g_dwUnk_4A4A14
		_asm test ebx, ebx
		_asm jne _sp_top
	}

	_asm _sp_done:
	_asm pop ebp
}

// hover.exe:0x4373A0  4 px x 4 rows, v shift 6, shade/remap table
// FUNCTION: HOVER 0x4373a0
void Span4x4Lit_S6(void)
{
	SPAN4X4_LIT(6)
}

// hover.exe:0x437460  4 px x 4 rows, v shift 7, shade/remap table
// FUNCTION: HOVER 0x437460
void Span4x4Lit_S7(void)
{
	SPAN4X4_LIT(7)
}

// hover.exe:0x437520  4 px x 4 rows, v shift 8, shade/remap table
// FUNCTION: HOVER 0x437520
void Span4x4Lit_S8(void)
{
	SPAN4X4_LIT(8)
}

// hover.exe:0x4375E0  4 px x 4 rows, v shift 9, shade/remap table
// FUNCTION: HOVER 0x4375e0
void Span4x4Lit_S9(void)
{
	SPAN4X4_LIT(9)
}

// hover.exe:0x4376A0  4 px x 4 rows, v shift 10, shade/remap table
// FUNCTION: HOVER 0x4376a0
void Span4x4Lit_S10(void)
{
	SPAN4X4_LIT(10)
}

// hover.exe:0x437760  4 px x 4 rows, v shift 15, shade/remap table
// FUNCTION: HOVER 0x437760
void Span4x4Lit_S15(void)
{
	SPAN4X4_LIT(15)
}

// hover.exe:0x437820  4 px x 4 rows, v shift 6, direct texel
// FUNCTION: HOVER 0x437820
void Span4x4_S6(void)
{
	SPAN4X4(6)
}

// hover.exe:0x4378D0  4 px x 4 rows, v shift 7, direct texel
// FUNCTION: HOVER 0x4378d0
void Span4x4_S7(void)
{
	SPAN4X4(7)
}

// hover.exe:0x437980  4 px x 4 rows, v shift 8, direct texel
// FUNCTION: HOVER 0x437980
void Span4x4_S8(void)
{
	SPAN4X4(8)
}

// hover.exe:0x437A30  4 px x 4 rows, v shift 9, direct texel
// FUNCTION: HOVER 0x437a30
void Span4x4_S9(void)
{
	SPAN4X4(9)
}

// hover.exe:0x437AE0  4 px x 4 rows, v shift 10, direct texel
// FUNCTION: HOVER 0x437ae0
void Span4x4_S10(void)
{
	SPAN4X4(10)
}

// hover.exe:0x437B90  4 px x 4 rows, v shift 11, direct texel
// FUNCTION: HOVER 0x437b90
void Span4x4_S11(void)
{
	SPAN4X4(11)
}

// hover.exe:0x437C40  4 px x 4 rows, v shift 12, direct texel
// FUNCTION: HOVER 0x437c40
void Span4x4_S12(void)
{
	SPAN4X4(12)
}

// hover.exe:0x437CF0  4 px x 4 rows, v shift 13, direct texel
// FUNCTION: HOVER 0x437cf0
void Span4x4_S13(void)
{
	SPAN4X4(13)
}

// hover.exe:0x437DA0  4 px x 4 rows, v shift 14, direct texel
// FUNCTION: HOVER 0x437da0
void Span4x4_S14(void)
{
	SPAN4X4(14)
}

// hover.exe:0x437E50  4 px x 4 rows, v shift 15, direct texel
// FUNCTION: HOVER 0x437e50
void Span4x4_S15(void)
{
	SPAN4X4(15)
}
