// gameobj.cpp : implementation of the CGameObj / CPlayer / CHumanPlayer
// classes
//
// Reconstructed from hover.exe; see gameobj.h for the RTTI/vtable data.

#include "hover.h"
#include "gameobj.h"
#include "bumperdoc.h"
#include "bumpview.h"
#include "texture.h"
#include "texload.h"

#include <math.h>				// sqrt()
#include <stdlib.h>				// abs()
#include <string.h>				// strcmp()

// The original calls strcmp out of line (hover.exe:0x43E87E) in
// CMerlinStatic::Serialize; VC 2.x would otherwise expand its ~20
// instruction intrinsic at each of the six call sites.
#pragma function(strcmp)

IMPLEMENT_DYNAMIC(CGameObj, CObject)
IMPLEMENT_DYNAMIC(CPlayer, CGameObj)
IMPLEMENT_DYNAMIC(CHumanPlayer, CPlayer)
IMPLEMENT_SERIAL(CMerlinLine, CMerlinObject, 1)
IMPLEMENT_SERIAL(CMerlinStatic, CMerlinLine, 1)
// IMPLEMENT_SERIAL hand-expanded for these two: their placement-new thunks
// Construct() are defined next to the classes further down so that they can
// carry the reccmp markers for hover.exe:0x415070 and 0x41BB80 (rule 221).
_IMPLEMENT_RUNTIMECLASS(CMerlinLocation, CMerlinObject, 1, CMerlinLocation::Construct)
CArchive& AFXAPI operator>>(CArchive& ar, CMerlinLocation* &pOb)
	{ pOb = (CMerlinLocation*) ar.ReadObject(RUNTIME_CLASS(CMerlinLocation));
		return ar; }
_IMPLEMENT_RUNTIMECLASS(CMerlinBSP, CMerlinLine, 1, CMerlinBSP::Construct)
CArchive& AFXAPI operator>>(CArchive& ar, CMerlinBSP* &pOb)
	{ pOb = (CMerlinBSP*) ar.ReadObject(RUNTIME_CLASS(CMerlinBSP));
		return ar; }

/////////////////////////////////////////////////////////////////////////////
// Globals

// CGameObj instance counter, incremented by CGameObj::CGameObj
static DWORD g_dwUnk_4A45A0;	// hover.exe:0x4A45A0

// hover.exe:0x4C52D8
// GLOBAL: HOVER 0x4c52d8
DWORD g_dwUnk_4C52D8;			// CGameObj instance list head (Sub_408530)

// hover.exe:0x4C52DC
// GLOBAL: HOVER 0x4c52dc
DWORD g_dwUnk_4C52DC;			// CGameObj secondary list head

// hover.exe:0x4C54A0
// GLOBAL: HOVER 0x4c54a0
DWORD g_dwUnk_4C54A0 = 0xFFFFFFFF;	// last player sound state
								// (CHumanPlayer::Sub_42AD30)

/////////////////////////////////////////////////////////////////////////////

// hover.exe:0x4A4820
// GLOBAL: HOVER 0x4a4820
CMapPtrToPtr g_mapUnk_4A4820;	// MIXWAVE handle -> CSub40B800 slot

// hover.exe:0x4A47F0
// GLOBAL: HOVER 0x4a47f0
CObArray g_arrUnk_4A47F0;	// sound slot registry
								// (CSub40B800::CSub40B800)

// hover.exe:0x4C5700
// GLOBAL: HOVER 0x4c5700
DWORD g_dwUnk_4C5700;			// sound slot count

/////////////////////////////////////////////////////////////////////////////
// The CPlayer physics pass's fixed-point multiply.
//
// hover.exe's player physics (0x40A9F0, 0x40ADE0, 0x40B0B0, 0x41A6B0,
// 0x41AB10, 0x42B8E0) reaches the destination through a pointer temporary
// like HOVER_FIXMULP (rule 50), but every expansion there owns *three*
// stack dwords, the highest of which is never written (rule 101). That
// third slot is the return temporary of an inline function that hands the
// factor over *by value as a class*: VC 2.1 never coalesces a by-value
// class parameter with the caller's temporary (rule 116), but it does
// forward the value into the callee's home and drop the store into the
// caller's, so the slot is allocated and stays untouched. Reproducing it
// is what restores those functions' frame sizes - and with them every
// esp displacement in them.

class CFixArg
{
public:
	DWORD m_dwValue;
};

inline CFixArg Hover_FixArg(DWORD dwB)
{
	CFixArg fxB;

	fxB.m_dwValue = dwB;
	return fxB;
}

inline void Hover_FixMulP3(DWORD* pdwA, CFixArg fxB)
{
	_asm mov ecx, pdwA
	_asm mov eax, [ecx]
	_asm imul fxB.m_dwValue
	_asm shrd eax, edx, 0x10
	_asm mov [ecx], eax
}

#define HOVER_FIXMULP3(dwA, dwB) Hover_FixMulP3(&(dwA), Hover_FixArg(dwB))

// Hover_PtOf: hover.exe reads a segment endpoint as one 8-byte value
// (rule 118) - the pair is copied into a fresh stack temporary of its own
// per expansion and then reached through a `lea` of that temporary.

static inline POINT Hover_PtOf(const DWORD* pdw)
{
	return *(const POINT*)pdw;
}

/////////////////////////////////////////////////////////////////////////////
// CSub40D320

// hover.exe:0x40D320
// FUNCTION: HOVER 0x40d320
CSub40D320::CSub40D320()
{
	m_dwUnk00 = 0;
	m_dwUnk08 = 0;
	m_dwUnk04 = 0;
	m_dwUnk0C = 0;
	Sub_42C820();
}

// hover.exe:0x42C820
// FUNCTION: HOVER 0x42c820
void CSub40D320::Sub_42C820()
{
	DWORD dwDy;
	DWORD dwDx;

	m_dwUnk10 = m_dwUnk08 - m_dwUnk00;
	m_dwUnk14 = m_dwUnk0C - m_dwUnk04;
	m_dwUnk18 = m_dwUnk14 * m_dwUnk14 + m_dwUnk10 * m_dwUnk10;
	// 16.16 fixed-point components (low word zeroed, 16-bit delta in the
	// high word); the angle comes back in the high word of dwDx
	((WORD*)&dwDy)[1] = (WORD)(m_dwUnk0C - m_dwUnk04);
	((WORD*)&dwDy)[0] = 0;
	((WORD*)&dwDx)[0] = 0;
	((WORD*)&dwDx)[1] = (WORD)(m_dwUnk08 - m_dwUnk00);
	m_dwUnk1C = ((short*)Hover_sub_4386D0(&dwDx, dwDy, dwDx))[1];
}

// Distance of the point pdwPt[0..1] from the segment, plus three flags:
// *pdwA is nonzero when the foot of the perpendicular falls between the
// endpoints, *pdwB is the side of the segment the point is on, and *pdwC
// is nonzero when the point lies beyond the first endpoint.
//
// Off the ends the distance is the octagonal estimate
// |dy| - min(|dx|,|dy|)/2 + |dx| of the distance to that endpoint; between
// them it is the exact perpendicular distance, the cross product divided
// by the (floating-point) segment length. Both are computed in 16 bits and
// sign-extended, exactly as the original did, so a point further than
// 0x7FFF away wraps.

// hover.exe:0x40C9B0
// FUNCTION: HOVER 0x40c9b0
int CSub40D320::Sub_40C9B0(const DWORD* pdwPt, DWORD* pdwA, DWORD* pdwB,
	DWORD* pdwC)
{
	long lX;
	long lY;
	long lDX;
	long lDY;
	long lDot;
	long lCross;
	long lMin;
	long lLen;
	short sDist;

	lX = (long)pdwPt[0];
	lY = (long)pdwPt[1];
	if (m_dwUnk18 == 0)
	{
		// Degenerate segment: the distance to the single endpoint.
		lX -= (long)m_dwUnk00;
		if (lX < 0)
			lX = -lX;
		lY -= (long)m_dwUnk04;
		if (lY < 0)
			lY = -lY;
		*pdwB = 0;
		*pdwA = 0;
		*pdwC = 0;
		lMin = (lY < lX) ? lY : lX;
		sDist = (short)lY;
		sDist -= (short)(lMin >> 1);
		sDist += (short)lX;
		return sDist;
	}

	lDY = lY - (long)m_dwUnk04;
	lDX = lX - (long)m_dwUnk00;
	lDot = lDY * (long)m_dwUnk14 + lDX * (long)m_dwUnk10;
	lCross = lDX * (long)m_dwUnk14 + ((long)m_dwUnk04 - lY) * (long)m_dwUnk10;
	if (lDot < 0)
	{
		// Before the first endpoint.
		if (lDX < 0)
			lDX = -lDX;
		if (lDY < 0)
			lDY = -lDY;
		*pdwB = (lCross >= 0);
		*pdwA = 0;
		*pdwC = 1;
		lMin = (lDY < lDX) ? lDY : lDX;
		sDist = (short)lDY;
		sDist -= (short)(lMin >> 1);
		sDist += (short)lDX;
		return sDist;
	}
	// The volatile read stops VC 2.2 CSE-ing the two m_dwUnk18 loads into
	// a stack slot; the original reads the member both times
	// (notes/perfect-match.md rule 26).
	if (lDot > (long)*(volatile DWORD*)&m_dwUnk18)
	{
		// Past the second endpoint. Note the original leaves *pdwC zero
		// here, so the two endpoint cases are told apart by *pdwC alone.
		lX -= (long)m_dwUnk08;
		if (lX < 0)
			lX = -lX;
		lY -= (long)m_dwUnk0C;
		if (lY < 0)
			lY = -lY;
		*pdwB = (lCross >= 0);
		*pdwA = 0;
		*pdwC = 0;
		lMin = (lY < lX) ? lY : lX;
		sDist = (short)lY;
		sDist -= (short)(lMin >> 1);
		sDist += (short)lX;
		return sDist;
	}

	// Between the endpoints: the perpendicular distance.
	*pdwA = 1;
	*pdwC = 0;
	*pdwB = (lCross >= 0);
	lLen = (long)sqrt((double)(long)m_dwUnk18);
	return (short)abs((short)(lCross / lLen));
}

/////////////////////////////////////////////////////////////////////////////
// CGameObj

// hover.exe:0x40D150
// FUNCTION: HOVER 0x40d150
CGameObj::CGameObj()
{
	m_wUnk44 = 0;
	m_dwUnk4C = 0;
	m_wUnk46 = 0;
	m_dwUnk50 = 0;
	m_wUnk48 = 0;
	m_dwUnk54 = 0;
	m_wUnk4A = 0;
	m_dwUnk58 = 0;
	m_dwUnk38 = 0;
	m_dwUnk24 = 0;
	m_dwUnk30 = 0;
	m_dwUnk5C = 0;
	m_dwUnk60 = 0;
	m_dwUnk78 = 0;
	m_dwUnk80 = 0;
	m_dwUnk7C = 0;
	m_dwUnk70 = 0;
	m_dwUnk74 = 0;
	m_dwUnk68 = 0;
	m_dwUnk6C = 0;
	m_dwUnk64 = 0;
	m_dwUnk34 = GetTickCount();
	g_dwUnk_4A45A0++;
}

// hover.exe:0x40D220
// FUNCTION: HOVER 0x40d220
CGameObj::~CGameObj()
{
	if (m_dwUnk78 != 0)
	{
		((CMerlinWorld*)theApp.m_pBumperDoc->m_dwUnk0188)->Sub_413DB0(m_dwUnk78);
		m_dwUnk78 = 0;
	}
	if (m_dwUnk30 != 0)
	{
		// Unlink every node of the m_dwUnk30 chain (nodes are linked
		// through +0x00/+0x04, chained through +0x08) and push it onto
		// the g_dwUnk_4C55D0 free list.
		DWORD* pNode = (DWORD*)m_dwUnk30;
		DWORD* pFree = (DWORD*)g_dwUnk_4C55D0;
		do
		{
			DWORD* pPrev = (DWORD*)pNode[1];
			DWORD* pNext = (DWORD*)pNode[0];
			pPrev[0] = (DWORD)pNext;
			if (pNext != NULL)
				pNext[1] = (DWORD)pPrev;
			pNode[0] = (DWORD)pFree;
			g_dwUnk_4C55D0 = (DWORD)pNode;
			pFree = pNode;
			pNode = (DWORD*)pNode[2];
		} while (pNode != NULL);
		m_dwUnk30 = 0;
	}
	g_dwUnk_4A45A0--;
}

// hover.exe:0x414A50
// FUNCTION: HOVER 0x414a50
int CGameObj::Sub_414A50(DWORD dwUnk)
{
	m_dwUnk78 = dwUnk;
	m_dwUnk80 = 0;
	m_dwUnk7C = 0;
	return 1;
}

// hover.exe:0x42AD10
// FUNCTION: HOVER 0x42ad10
int CGameObj::Sub_42AD10(DWORD dwA, DWORD dwB)
{
	m_dwUnk7C = dwA;
	m_dwUnk80 = dwB;
	m_dwUnk78 = 0;
	return 1;
}

// hover.exe:0x42AD30
// FUNCTION: HOVER 0x42ad30
void CGameObj::Sub_42AD30()
{
	// empty default implementation in the original
}

// hover.exe:0x42AD40
// FUNCTION: HOVER 0x42ad40
void CGameObj::Sub_42AD40(CPlayer* pOther, DWORD dwB)
{
	// empty default implementation in the original
}

// hover.exe:0x42AD50
// FUNCTION: HOVER 0x42ad50
void CGameObj::Sub_42AD50(DWORD dwUnk)
{
	// empty default implementation in the original
}

// hover.exe:0x42AD70
// FUNCTION: HOVER 0x42ad70
int CGameObj::Sub_42AD70(DWORD dwA, DWORD dwB, DWORD dwC)
{
	return 0;
}

// hover.exe:0x42AD60
// FUNCTION: HOVER 0x42ad60
int CGameObj::Sub_42AD60(DWORD dwUnk)
{
	return 0;
}

// Virtual functions beyond slot 11.

// Base virtual body for vtable slot 12. The original CGameObj vtable points
// to hover.exe:0x42AD80 (a one-instruction "return 1"); CPlayer overrides
// this slot with the real implementation at hover.exe:0x408820 below.
// FUNCTION: HOVER 0x42ad80
int CGameObj::Sub_408820(DWORD dwA)
{
	return 1;
}

// hover.exe:0x411570
// FUNCTION: HOVER 0x411570
int CGameObj::Sub_411570(DWORD dwA, DWORD dwB, DWORD dwC, DWORD* pdwOut)
{
	*pdwOut = 0;
	return 0;
}

// hover.exe:0x42AD90
// FUNCTION: HOVER 0x42ad90
int CGameObj::Sub_42AD90(CGameObj* pObj)
{
	return 1;
}

/////////////////////////////////////////////////////////////////////////////
// CMerlinLine

// hover.exe:0x401160
// FUNCTION: HOVER 0x401160
CMerlinLine::CMerlinLine()
{
	m_dwY2 = 0;
	m_dwX2 = 0;
	m_dwY1 = 0;
	m_dwX1 = 0;
	Sub_4010C0();
}

// hover.exe:0x40F760
// FUNCTION: HOVER 0x40f760
CMerlinLine::CMerlinLine(short sX1, short sY1, short sX2, short sY2)
{
	m_dwX1 = (long)sX1;
	m_dwY1 = (long)sY1;
	m_dwX2 = (long)sX2;
	m_dwY2 = (long)sY2;
	Sub_4010C0();
}

// hover.exe:0x401210
// FUNCTION: HOVER 0x401210
CMerlinLine::~CMerlinLine()
{
}

// CMerlinLine::Serialize (vtable slot 2): the base object part, then the
// four endpoint words and the trailing comment blob. Only the low word of
// each dword endpoint is stored, and only the low word is written back on
// load (the constructor leaves the high halves zero); the derived values
// are recomputed with Sub_4010C0 afterwards.

// hover.exe:0x414740
// FUNCTION: HOVER 0x414740
void CMerlinLine::Serialize(CArchive& ar)
{
	CMerlinArchive* pAr = (CMerlinArchive*)&ar;
	short sCount;
	int nLeft;
	BYTE by;

	CMerlinObject::Serialize(ar);
	if (ar.IsStoring())
	{
		pAr->WriteWord((WORD)m_dwX1);
		pAr->WriteWord((WORD)m_dwY1);
		pAr->WriteWord((WORD)m_dwX2);
		pAr->WriteWord((WORD)m_dwY2);
		pAr->WriteWord((WORD)0);
	}
	else
	{
		pAr->ReadWord((WORD*)&m_dwX1);
		pAr->ReadWord((WORD*)&m_dwY1);
		pAr->ReadWord((WORD*)&m_dwX2);
		pAr->ReadWord((WORD*)&m_dwY2);
		sCount = (short)pAr->ReadWord();
		if (sCount > 0)			// trailing comment bytes, skipped
		{
			nLeft = sCount;
			do
			{
				ar >> by;
				// The original reserves a frame slot for `by`
				// (its "sub esp, 4"): MFC's inline operator>>
				// stores through the reference, VC 2.1 deletes
				// the dead store *and* the slot, and this
				// volatile self-assignment keeps the slot
				// without emitting an instruction of its own.
				*(volatile BYTE*)&by = by;
			}
			while (--nLeft != 0);
		}
		Sub_4010C0();
	}
}

// Recompute the bounding box, deltas, dist-sq and flags from the current
// endpoints. Faithful reconstruction of hover.exe:0x4010C0.
// hover.exe:0x4010C0
// FUNCTION: HOVER 0x4010c0
void CMerlinLine::Sub_4010C0()
{
	m_wFlags &= 0xFFF8;

	if ((long)m_dwX1 < (long)m_dwX2)
	{
		m_dwMinX = m_dwX1;
		m_dwMaxX = m_dwX2;
	}
	else
	{
		m_dwMinX = m_dwX2;
		m_dwMaxX = m_dwX1;
	}

	if ((long)m_dwY1 < (long)m_dwY2)
	{
		m_dwMinY = m_dwY1;
		m_dwMaxY = m_dwY2;
	}
	else
	{
		m_dwMinY = m_dwY2;
		m_dwMaxY = m_dwY1;
	}

	if ((long)m_dwX1 != (long)m_dwX2)
		m_wFlags |= 0x0002;

	if ((long)m_dwY1 != (long)m_dwY2)
		m_wFlags |= 0x0004;

	if ((m_wFlags & 0x0006) == 0)
		m_wFlags |= 0x0001;

	m_dwDeltaX = (long)m_dwX2 - (long)m_dwX1;
	m_dwDeltaY = (long)m_dwY2 - (long)m_dwY1;
	m_dwDistSq = m_dwDeltaX * m_dwDeltaX + m_dwDeltaY * m_dwDeltaY;
}


// hover.exe:0x4087D0
// FUNCTION: HOVER 0x4087d0
void CGameObj::Sub_4087D0()
{
	m_dwUnk4C = (DWORD)(short)m_wUnk46;
	m_dwUnk50 = (DWORD)(short)m_wUnk4A;
	m_dwUnk60 = m_dwUnk5C;
}

// hover.exe:0x4087F0
// FUNCTION: HOVER 0x4087f0
void CGameObj::Sub_4087F0()
{
	m_dwUnk54 = m_dwUnk4C;
	m_dwUnk58 = m_dwUnk50;
	if (m_dwUnk60 != 0 && this != NULL)
	{
		// deleting destructor (vtable slot 1)
		delete this;
	}
}

// hover.exe:0x413020
// FUNCTION: HOVER 0x413020
void CGameObj::Sub_413020()
{
	m_dwUnk5C = 1;
	if (m_dwUnk68 == 0)
	{
		m_dwUnk68 = 1;
		m_dwUnk70 = g_dwUnk_4C52D8;
		g_dwUnk_4C52D8 = (DWORD)this;
	}
	theApp.m_pBumperDoc->Sub_412650(this);
}

// Links the object into the g_dwUnk_4C52D8 instance list (via m_dwUnk70).

// hover.exe:0x408530
// FUNCTION: HOVER 0x408530
void CGameObj::Sub_408530()
{
	if (m_dwUnk68 == 0)
	{
		m_dwUnk68 = 1;
		m_dwUnk70 = g_dwUnk_4C52D8;
		g_dwUnk_4C52D8 = (DWORD)this;
	}
}

// hover.exe:0x403880
// FUNCTION: HOVER 0x403880
DWORD* CGameObj::Sub_403880(DWORD* pdwOut)
{
	pdwOut[0] = (DWORD)(short)m_wUnk46;
	pdwOut[1] = (DWORD)(short)m_wUnk4A;
	return pdwOut;
}

// hover.exe:0x4038A0
// FUNCTION: HOVER 0x4038a0
DWORD* CGameObj::Sub_4038A0(DWORD* pdwOut)
{
	// The original reads the two dwords at +0x44 and +0x48, which contain
	// the m_wUnk44/m_wUnk46 and m_wUnk48/m_wUnk4A word pairs as 16.16
	// fixed-point values.
	pdwOut[0] = *(DWORD*)((BYTE*)this + 0x44);
	pdwOut[1] = *(DWORD*)((BYTE*)this + 0x48);
	return pdwOut;
}

// hover.exe:0x406C40
// FUNCTION: HOVER 0x406c40
DWORD* CGameObj::Sub_406C40(DWORD* pdwOut)
{
	// The original reads the two dwords through a pointer to +0x54.
	DWORD* pdw = (DWORD*)((BYTE*)this + 0x54);
	pdwOut[0] = pdw[0];
	pdwOut[1] = pdw[1];
	return pdwOut;
}

// Copies the current cell position into pdwOut[0..1] and returns it.
// hover.exe:0x413000
// FUNCTION: HOVER 0x413000
DWORD* CGameObj::Sub_413000(DWORD* pdwOut)
{
	pdwOut[0] = m_dwUnk4C;
	pdwOut[1] = m_dwUnk50;
	return pdwOut;
}

// hover.exe:0x40F880
// FUNCTION: HOVER 0x40f880
void CGameObj::Sub_40F880(DWORD dwX, DWORD dwY)
{
	m_wUnk46 = (WORD)dwX;
	m_wUnk44 = 0;
	m_wUnk4A = (WORD)dwY;
	m_wUnk48 = 0;
}

// hover.exe:0x40A9B0
// FUNCTION: HOVER 0x40a9b0
void CGameObj::Sub_40A9B0()
{
	if (m_dwUnk30 != 0)
	{
		// Unlink every node of the m_dwUnk30 chain (same linkage as in
		// ~CGameObj) and push it onto the g_dwUnk_4C55D0 free list.
		DWORD* pNode = (DWORD*)m_dwUnk30;
		DWORD* pFree = (DWORD*)g_dwUnk_4C55D0;
		do
		{
			DWORD* pPrev = (DWORD*)pNode[1];
			DWORD* pNext = (DWORD*)pNode[0];
			pPrev[0] = (DWORD)pNext;
			if (pNext != NULL)
				pNext[1] = (DWORD)pPrev;
			pNode[0] = (DWORD)pFree;
			g_dwUnk_4C55D0 = (DWORD)pNode;
			pFree = pNode;
			pNode = (DWORD*)pNode[2];
		} while (pNode != NULL);
		m_dwUnk30 = 0;
	}
}

// Plays the sound in CBumperDoc slot dwA, but at most once every 500 ms.

// hover.exe:0x40D6E0
// FUNCTION: HOVER 0x40d6e0
void CGameObj::Sub_40D6E0(DWORD dwA, DWORD dwB, DWORD dwC, DWORD dwD)
{
	if (m_dwUnk34 < GetTickCount() - 500)
		((CSub40B800*)theApp.m_pBumperDoc->m_adwUnk83EC[dwA])->Sub_40B800(
			dwB, dwC, dwD);
	m_dwUnk34 = GetTickCount();
}

/////////////////////////////////////////////////////////////////////////////
// CPlayer

// The word blocks are zeroed one member at a time, as the original does:
// a loop (or memset) becomes a rep stosd that VC cannot schedule ahead of
// the four CObList constructions the way the original's individual stores
// are (cf. rule 47).

// hover.exe:0x41AC30
// FUNCTION: HOVER 0x41ac30
CPlayer::CPlayer()
{
	m_awUnk0090[0] = 0;
	m_awUnk0090[1] = 0;
	m_awUnk0090[2] = 0;
	m_awUnk0090[3] = 0;
	m_awUnk0090[4] = 0;
	m_awUnk0090[5] = 0;
	m_awUnk0090[6] = 0;
	m_awUnk0090[7] = 0;
	m_awUnk0090[8] = 0;
	m_awUnk0090[9] = 0;
	m_awUnk0090[10] = 0;
	m_awUnk0090[11] = 0;
	m_awUnk0194[0] = 0;
	m_awUnk0194[1] = 0;
	m_awUnk0194[2] = 0;
	m_awUnk0194[3] = 0;
	m_awUnk0194[4] = 0;
	m_awUnk0194[5] = 0;
	m_awUnk01AC[0] = 0;
	m_awUnk01AC[1] = 0;
	m_awUnk01AC[2] = 0;
	m_awUnk01AC[3] = 0;
	m_awUnk01AC[4] = 0;
	m_awUnk01AC[5] = 0;
	m_awUnk01AC[6] = 0;
	m_awUnk01AC[7] = 0;
	m_awUnk01AC[8] = 0;
	m_awUnk01AC[9] = 0;
	m_awUnk01AC[10] = 0;
	m_awUnk01AC[11] = 0;
	m_awUnk01D0[0] = 0;
	m_awUnk01D0[1] = 0;
	m_awUnk01D0[2] = 0;
	m_awUnk01D0[3] = 0;
	m_awUnk01D0[4] = 0;
	m_awUnk01D0[5] = 0;
	m_wUnk01F0 = 0;
	m_dwUnk01F8 = 0;
	m_wUnk01F2 = 0;
	m_wUnk01F4 = 0;
	m_wUnk01F6 = 0;
}

// The class has both an out-of-line ??1 (hover.exe:0x421A90, called by
// ~CHumanPlayer for the player CBumperDoc embeds and by ~CRobotPlayer) and a
// scalar deleting ??_G (hover.exe:0x42C080); rule 69 says to mark the ??1,
// which is what this definition compiles to.
// hover.exe:0x421A90
// FUNCTION: HOVER 0x421a90
CPlayer::~CPlayer()
{
	Sub_414BD0(1);
}

// Deletes every object in m_obList00B4 and empties the list; when bAll is
// nonzero the other three object lists are emptied the same way.

// hover.exe:0x414BD0
// FUNCTION: HOVER 0x414bd0
void CPlayer::Sub_414BD0(int bAll)
{
	POSITION pos = m_obList00B4.GetHeadPosition();
	while (pos != NULL)
		delete m_obList00B4.GetNext(pos);
	m_obList00B4.RemoveAll();
	if (bAll != 0)
	{
		pos = m_obList00D0.GetHeadPosition();
		while (pos != NULL)
			delete m_obList00D0.GetNext(pos);
		m_obList00D0.RemoveAll();
		pos = m_obList00EC.GetHeadPosition();
		while (pos != NULL)
			delete m_obList00EC.GetNext(pos);
		m_obList00EC.RemoveAll();
		pos = m_obList0108.GetHeadPosition();
		while (pos != NULL)
			delete m_obList0108.GetNext(pos);
		m_obList0108.RemoveAll();
	}
	m_dwUnk0124 = 0;
	m_dwUnk0128 = 0;
	m_dwUnk012C = 0;
	m_dwUnk0130 = 0;
	m_dwUnk0134 = 0;
	m_dwUnk0138 = 0;
}

// Points the player at a parameter block (theApp.m_pUnk014C for the human
// player, theApp.m_pUnk0150 for the robots) and refreshes the three tuning
// values cached at +0x1D0. A NULL pParams keeps the current block, which is
// how the level loader re-reads the values after a skill change.

// hover.exe:0x413ED0
// FUNCTION: HOVER 0x413ed0
void CPlayer::Sub_413ED0(void* pParams)
{
	DWORD* pdwParams;
	DWORD dwP0;
	DWORD dwP8;

	if (pParams != NULL)
		m_dwUnk0084 = (DWORD)pParams;
	// 32-bit copies, as in Sub_41B0E0: params[0] -> +0x1D0 and
	// params[2] -> +0x1D4 and +0x1D8
	pdwParams = (DWORD*)m_dwUnk0084;
	dwP0 = pdwParams[0];
	dwP8 = pdwParams[2];
	*(DWORD*)&m_awUnk01D0[0] = dwP0;
	*(DWORD*)&m_awUnk01D0[2] = dwP8;
	*(DWORD*)&m_awUnk01D0[4] = dwP8;
}

// hover.exe:0x409110
// FUNCTION: HOVER 0x409110
void CPlayer::Sub_42AD30()
{
	if (m_dwUnk014C == 0xFFFFFFFF)
		Sub_40A9F0();
	if (m_dwUnk5C == 0 && theApp.m_dwUnk01B8 == 3)
	{
		Sub_40AB70();
		Sub_40ACE0();
		Sub_40ADE0();
		Sub_40B0B0();
		m_dwUnk0154 = 0;
		m_dwUnk0158 = 0;
		Sub_408530();
	}
	CGameObj::Sub_42AD30();
}

// hover.exe:0x40C220
// FUNCTION: HOVER 0x40c220
void CPlayer::Sub_42AD40(CPlayer* pOther, DWORD dwB)
{
	CBumperView* pView;

	if (dwB != 0)
	{
		m_dwUnk01CC = 0;
		pOther->Sub_42AD40(this, 0);
	}
	if (pOther->IsKindOf(RUNTIME_CLASS(CPlayer)))
	{
		pView = Hover_sub_401140();
		pView->m_dwUnk0524 = 1;
		Sub_42B8E0((DWORD)pOther, dwB);
		if (theApp.m_dwUnk0408 != 0 &&
			pOther->IsKindOf(RUNTIME_CLASS(CHumanPlayer)))
			Sub_40D6E0(0x14, 3, 5, 0);
	}
}

// Proximity check against the two points at +0x180/+0x188 using the
// max + min/2 distance metric.

// hover.exe:0x42BFC0
// FUNCTION: HOVER 0x42bfc0
int CPlayer::Sub_42AD70(DWORD dwA, DWORD dwB, DWORD dwC)
{
	// The original walks the five values through one pointer
	// (`add ecx, 0x180` at hover.exe:0x42BFC1), not off `this`.
	DWORD* pdw = &m_dwUnk0180;
	long lLimit = (long)(pdw[4] + dwC);
	long lX = labs((long)pdw[0] - (long)dwA);
	long lY = labs((long)pdw[1] - (long)dwB);
	long lMin = (lY < lX) ? lY : lX;

	if (lY - (lMin >> 1) + lX < lLimit)
		return 1;
	lX = labs((long)pdw[2] - (long)dwA);
	lY = labs((long)pdw[3] - (long)dwB);
	lMin = (lY < lX) ? lY : lX;
	return (lY - (lMin >> 1) + lX < lLimit);
}

// hover.exe:0x42BFA0
// FUNCTION: HOVER 0x42bfa0
int CPlayer::Sub_42AD60(DWORD dwUnk)
{
	DWORD dwA;
	DWORD dwB;

	return Sub_42BE50((CPlayer*)dwUnk, &dwB, &dwA);
}

// Maps the player's two proximity points (+0x180/+0x188) plus the +0x190
// range limit onto the collider's region grid, clamps to the grid bounds,
// and visits every covered cell. When the collider's +0x44 flag is set the
// cell objects are checked against a registered-object array
// (CCollider::Sub_40F8A0); otherwise the player is added to the cell
// (CRegionMatrix::Sub_4086A0). Always returns 1.

// hover.exe:0x408820
// FUNCTION: HOVER 0x408820
int CPlayer::Sub_408820(DWORD dwA)
{
	CCollider* pCollider;
	DWORD* pProx;
	long nX1, nX2, nY1, nY2;
	long nMinX, nMaxX, nMinY, nMaxY;
	long nX, nY;

	pCollider = (CCollider*)dwA;

	// Virtual call at vtable slot 0x4C returns &m_dwUnk0180.
	pProx = Sub_4089E0();

	nX2 = (long)pProx[2];
	nX1 = (long)pProx[0];
	if (nX2 > nX1)
	{
		nMinX = (nX1 - (long)pProx[4]) / 256;
		nMaxX = (nX2 + (long)pProx[4]) / 256;
	}
	else
	{
		nMinX = (nX2 - (long)pProx[4]) / 256;
		nMaxX = (nX1 + (long)pProx[4]) / 256;
	}

	nY1 = (long)pProx[1];
	nY2 = (long)pProx[3];
	if (nY2 > nY1)
	{
		nMinY = (nY1 - (long)pProx[4]) / 256;
		nMaxY = (nY2 + (long)pProx[4]) / 256;
	}
	else
	{
		nMinY = (nY2 - (long)pProx[4]) / 256;
		nMaxY = (nY1 + (long)pProx[4]) / 256;
	}

	if ((long)pCollider->m_dwUnk04 <= nMaxX)
		nMaxX = (long)pCollider->m_dwUnk04 - 1;
	if ((long)pCollider->m_dwUnk08 <= nMaxY)
		nMaxY = (long)pCollider->m_dwUnk08 - 1;

	if (nMaxX < nMinX || nMaxY < nMinY)
		return 1;

	for (nX = nMinX; nX <= nMaxX; nX++)
	{
		for (nY = nMinY; nY <= nMaxY; nY++)
		{
			if (pCollider->m_dwUnk44 != 0)
				pCollider->Sub_40F8A0(nX, nY);
			else
				pCollider->m_obj10.Sub_4086A0(nX, nY, this);
		}
	}
	return 1;
}

// Publishes the heading/frame state the renderer reads: the current
// heading word becomes the "previous frame" angle (+0x1A0/+0x1A8) and the
// sprite frame picked by the derived Sub_4087D0 (+0x1E4) is latched into
// +0x1E8. Then runs the CGameObj version (position snapshot).

// hover.exe:0x408960
// FUNCTION: HOVER 0x408960
void CPlayer::Sub_4087D0()
{
	m_dwUnk01A0 = (DWORD)(long)(short)m_awUnk0194[5];
	m_dwUnk01A8 = m_dwUnk01A4;
	m_dwUnk01E8 = m_dwUnk01E4;
	CGameObj::Sub_4087D0();
}

// Second half of the per-frame publish: while the object is alive
// (m_dwUnk60 == 0) the latched heading pair is copied into +0x88/+0x8C
// and, when the player owns a sprite, into the sprite's +0x54/+0x56 pair
// (the second word is the first plus a fixed 0x40 span). Then runs the
// CGameObj version (which is what actually deletes a retired object).

// hover.exe:0x408990
// FUNCTION: HOVER 0x408990
void CPlayer::Sub_4087F0()
{
	DWORD dwAngle;
	CMerlinDynamic* pSprite;

	if (m_dwUnk60 == 0)
	{
		pSprite = (CMerlinDynamic*)m_dwUnk78;
		m_dwUnk0088 = m_dwUnk01A0;
		dwAngle = m_dwUnk01A8;
		m_dwUnk008C = dwAngle;
		if (pSprite != NULL)
		{
			pSprite->m_wUnk54 = (WORD)dwAngle;
			pSprite->m_wUnk56 = (WORD)dwAngle + 0x40;
		}
	}
	CGameObj::Sub_4087F0();
}

// hover.exe:0x42C050
// FUNCTION: HOVER 0x42c050
void CPlayer::Sub_413020()
{
	CGameObj::Sub_413020();
}

// hover.exe:0x42BE40
// FUNCTION: HOVER 0x42be40
void CPlayer::Sub_42BE40(DWORD dwUnk)
{
	// empty in the original (a bare `ret 4`); every player class inherits
	// this slot 18 implementation
}

// hover.exe:0x4089E0
// FUNCTION: HOVER 0x4089e0
DWORD* CPlayer::Sub_4089E0()
{
	return &m_dwUnk0180;
}

// hover.exe:0x414A80
// FUNCTION: HOVER 0x414a80
void CPlayer::Sub_414A80()
{
	DWORD dwZero;
	DWORD dwA0;
	DWORD dwP0;
	DWORD dwP8;

	// The original clears the word arrays with straight-line stores in
	// this exact (paired, descending) order.
	m_awUnk01AC[5] = 0;
	m_awUnk01AC[4] = 0;
	m_awUnk01AC[7] = 0;
	m_awUnk01AC[6] = 0;
	m_awUnk01AC[9] = 0;
	m_awUnk01AC[8] = 0;
	m_awUnk01AC[11] = 0;
	m_awUnk01AC[10] = 0;
	m_awUnk01AC[1] = 0;
	m_awUnk01AC[0] = 0;
	m_awUnk01AC[3] = 0;
	m_awUnk01AC[2] = 0;
	m_awUnk0194[1] = 0;
	m_awUnk0194[0] = 0;
	m_dwUnk01A0 = 0;
	m_awUnk0194[5] = 0;
	m_dwUnk01A8 = 0;
	m_awUnk0194[4] = 0;
	m_dwUnk0088 = 0;
	m_awUnk0194[3] = 0;
	m_dwUnk008C = 0;
	m_awUnk0194[2] = 0;
	m_dwUnk01C4 = 0;
	((WORD*)&dwZero)[0] = 0;
	((WORD*)&dwZero)[1] = 0;
	m_dwUnk013C = 0;
	Sub_408550(dwZero);
	m_dwUnk0154 = 0;
	m_dwUnk01DC = 1;
	m_dwUnk01E0 = 1;
	m_dwUnk0148 = 0xFFFFFFFF;
	m_dwUnk0158 = 0;
	m_dwUnk014C = 0xFFFFFFFF;
	m_dwUnk0150 = 0xFFFFFFFF;
	m_obList00B4.RemoveAll();
	m_awUnk0090[8] = 0;
	m_dwUnk0140 = 0;
	m_dwUnk0144 = 0;
	m_awUnk0090[9] = 0x40;
	// 32-bit copies: words [8..9] -> [10..11], and the first two DWORDs
	// of the params block at m_dwUnk0084 -> m_awUnk01D0
	dwA0 = *(DWORD*)&m_awUnk0090[8];
	dwP0 = *(DWORD*)m_dwUnk0084;
	dwP8 = *(DWORD*)(m_dwUnk0084 + 8);
	*(DWORD*)&m_awUnk0090[10] = dwA0;
	*(DWORD*)&m_awUnk01D0[0] = dwP0;
	*(DWORD*)&m_awUnk01D0[2] = dwP8;
	*(DWORD*)&m_awUnk01D0[4] = dwP8;
}

// Recomputes the two proximity points at +0x180/+0x188 (50 fixed-point
// units ahead of and behind the position at +0x44/+0x48 along the movement
// angle m_awUnk0194[5], 0x200 per full circle) and the +0x190 range limit
// (dwUnk scaled by 1.5, at least 50).

// hover.exe:0x408550
// FUNCTION: HOVER 0x408550
void CPlayer::Sub_408550(DWORD dwUnk)
{
	DWORD dwCos;
	DWORD dwSin;
	DWORD dwX;
	DWORD dwY;
	DWORD dwFrontX;
	DWORD dwFrontY;
	DWORD dwBackX;
	DWORD dwBackY;
	DWORD dwRange;
	DWORD dwScale;
	long lAngle;

	lAngle = (long)(short)m_awUnk0194[5];
	dwCos = g_adwSinTable[(((lAngle + 0x80) & 0x1FF) << 4) + 1];
	dwSin = g_adwSinTable[((lAngle & 0x1FF) << 4) + 1];
	HOVER_FIXMULWP(dwSin, 0x32);
	HOVER_FIXMULWP(dwCos, 0x32);
	dwX = *(DWORD*)&m_wUnk44;
	dwY = *(DWORD*)&m_wUnk48;
	dwFrontX = dwX + dwCos;
	dwFrontY = dwY + dwSin;
	m_dwUnk0180 = ((short*)&dwFrontX)[1];
	m_dwUnk0184 = ((short*)&dwFrontY)[1];
	dwBackX = dwX - dwCos;
	dwBackY = dwY - dwSin;
	m_dwUnk0188 = ((short*)&dwBackX)[1];
	m_dwUnk018C = ((short*)&dwBackY)[1];
	dwRange = dwUnk;
	((WORD*)&dwScale)[0] = 0x8000;
	((WORD*)&dwScale)[1] = 1;
	HOVER_FIXMULP3(dwRange, dwScale);
	m_dwUnk0190 = ((short*)&dwRange)[1];
	if ((long)m_dwUnk0190 < 0x32)
		m_dwUnk0190 = 0x32;
}

// Throttle integration: accelerates the speed dword at +0x194 by the
// thrust parameter (+0x28 in the params block) scaled by the throttle
// input (+0x1F0), clamped to the parameter in the input direction, then
// converts the turn input (+0x1F4) scaled by the +0x1D0 dword into the
// fixed-point velocity pair at +0x1B4/+0x1B8 along the angle at +0x19C.

// hover.exe:0x40A9F0
// FUNCTION: HOVER 0x40a9f0
void CPlayer::Sub_40A9F0()
{
	DWORD dwAccel;
	DWORD dwLimit;
	DWORD dwTurn;
	DWORD dwAngle;
	DWORD dwVelX;
	DWORD dwVelY;
	DWORD* pdwParams;

	m_awUnk0194[3] = 0;
	m_awUnk0194[2] = 0;
	pdwParams = (DWORD*)(m_dwUnk0084 + 0x28);
	dwAccel = *pdwParams;
	HOVER_FIXMULP(dwAccel, *(DWORD*)&m_wUnk01F0);
	if (*(volatile long*)&m_wUnk01F0 < 0)
	{
		dwLimit = *pdwParams;
		if (-(long)dwLimit > *(long*)&m_awUnk0194[0] ||
			-(long)dwLimit == *(long*)&m_awUnk0194[0])
		{
			// already at or below the reverse limit
		}
		else
		{
			*(long*)&m_awUnk0194[0] += (long)dwAccel;
			if (-(long)dwLimit > *(long*)&m_awUnk0194[0])
				*(long*)&m_awUnk0194[0] = -(long)dwLimit;
		}
	}
	else
	{
		dwLimit = *pdwParams;
		if ((long)dwLimit > *(long*)&m_awUnk0194[0])
		{
			*(long*)&m_awUnk0194[0] += (long)dwAccel;
			if ((long)dwLimit < *(long*)&m_awUnk0194[0] ||
				(long)dwLimit == *(long*)&m_awUnk0194[0])
				*(long*)&m_awUnk0194[0] = (long)dwLimit;
		}
	}
	dwTurn = *(DWORD*)&m_awUnk01D0[0];
	HOVER_FIXMULP(dwTurn, *(DWORD*)&m_wUnk01F4);
	dwAngle = *(DWORD*)&m_awUnk0194[4];
	dwVelX = *(volatile DWORD*)&dwTurn;
	HOVER_FIXMULP(dwVelX,
		g_adwSinTable[((((long)dwAngle >> 10) + 0x2000) & 0x7FFC) / 4 + 1]);
	*(DWORD*)&m_awUnk01AC[4] = dwVelX;
	dwVelY = *(volatile DWORD*)&dwTurn;
	HOVER_FIXMULP(dwVelY,
		g_adwSinTable[((*(volatile DWORD*)&dwAngle & 0x1FFF000) >> 10) / 4 + 1]);
	*(DWORD*)&m_awUnk01AC[6] = dwVelY;
}

// Angular integration: steers the current angle (+0xA4) toward the target
// angle (+0xA0), snapping when within 2.0 fixed-point units.

// hover.exe:0x40AB70
// FUNCTION: HOVER 0x40ab70
void CPlayer::Sub_40AB70()
{
	DWORD dwTarget;
	DWORD dwCur;
	DWORD dwDelta;
	// dwVal is volatile because HOVER_FIXDIVPP writes it through a pointer
	// VC 2.1 does not follow: without it the initialising constant is
	// propagated over the divide and the two members get 0x240000 and
	// 0x180000 instead of the quotient (rule 95/140).
	volatile DWORD dwVal;
	DWORD dwDiff;
	DWORD dwVal2;

	dwTarget = *(DWORD*)&m_awUnk0090[8];
	dwCur = *(DWORD*)&m_awUnk0090[10];
	dwDelta = dwTarget - dwCur;
	if (dwCur == dwTarget)
	{
		*(DWORD*)&m_pad00A8[0] = 0;
	}
	else if ((long)dwDelta >= 0x20000 || (long)dwDelta < -0x20000 ||
		(long)dwDelta == -0x20000)
	{
		if ((long)dwDelta < 0)
		{
			dwVal = 0x240000;
			dwDiff = dwCur - dwTarget;
			HOVER_FIXDIVPP((DWORD*)&dwVal, dwDiff);
			*(DWORD*)&m_awUnk0090[6] = dwVal;
			dwVal2 = dwVal - 0xC0000;
			*(DWORD*)&m_awUnk0090[4] = dwVal2;
		}
		else
		{
			if (*(DWORD*)&m_pad00A8[0] != 0)
				*(DWORD*)&m_awUnk0090[6] =
					(DWORD)((long)(dwTarget - dwCur) / 3);
			else
				*(DWORD*)&m_awUnk0090[6] = dwTarget - dwCur;
			*(DWORD*)&m_awUnk0090[4] = *(DWORD*)&m_awUnk0090[6];
		}
		*(DWORD*)&m_awUnk0090[10] = *(DWORD*)&m_awUnk0090[4] + dwCur;
	}
	else
	{
		if (m_dwUnk01F8 != 0 && theApp.m_dwUnk0408 != 0 && (long)dwDelta < 0)
			Sub_40D6E0(0x16, 4, 5, 0);
		*(DWORD*)&m_awUnk0090[10] = dwTarget;
	}
	m_dwUnk013C = 0;
	m_dwUnk01A4 = (long)(short)m_awUnk0090[11];
}

// hover.exe:0x40ACE0
// FUNCTION: HOVER 0x40ace0
void CPlayer::Sub_40ACE0()
{
	if (m_dwUnk0154 == 0)
	{
		m_dwUnk0148 = 0xFFFFFFFF;
		if (m_dwUnk013C == 0 && *(DWORD*)&m_pad00A8[0] == 0)
		{
			m_awUnk0090[9] = 0x40;
			m_awUnk0090[8] = 0;
		}
	}
	else
	{
		if (m_dwUnk0148 == 0x5A)
		{
			m_awUnk0090[9] = 0x10;
			m_awUnk0090[8] = 0;
			if (m_dwUnk01F8 != 0 && theApp.m_dwUnk0408 != 0)
				Sub_40D6E0(2, 0, 5, 0);
		}
		else if (m_dwUnk0148 == 1)
		{
			if (m_dwUnk01F8 != 0 && theApp.m_dwUnk0408 != 0)
				Sub_40D6E0(3, 0, 5, 0);
			m_awUnk0090[9] = 0x40;
			m_awUnk0090[8] = 0;
		}
		if (m_dwUnk01E0 != 0 && (long)m_dwUnk0148 > 0)
		{
			m_dwUnk0148--;
			m_awUnk01AC[4] = 0;
			m_awUnk01AC[5] = 0;
			m_awUnk01AC[6] = 0;
			m_awUnk01AC[7] = 0;
		}
	}
}

// Wall-bounce handling while m_dwUnk014C counts down: scales the +0x1D8
// thrust into +0x1D4, splits it along the target angle (+0x160) into the
// +0x1B4/+0x1B8 velocity pair, and reverses or bleeds the speed (+0x194)
// depending on the current angle m_awUnk0194[5].

// hover.exe:0x40ADE0
// FUNCTION: HOVER 0x40ade0
void CPlayer::Sub_40ADE0()
{
	DWORD dwThrust;
	DWORD dwThrust1;
	DWORD dwThrust2;
	DWORD dwThrust3;
	DWORD dwCos;
	DWORD dwSin;
	DWORD dwMargin1;
	DWORD dwMargin2;
	DWORD dwSpeed1;
	DWORD dwSpeed2;
	CFixArg fxSpeed;
	DWORD* pdwParams;
	long lDelta;
	long lAngle;
	long lCount;
	WORD wAngle;

	// The <= 0 guard's body is the function's tail in the original:
	// hover.exe:0x40B092 stores the -1 after every other exit path.
	if ((long)m_dwUnk014C > 0)
	{
		if (m_dwUnk014C == 0x1E && m_dwUnk01F8 != 0 && theApp.m_dwUnk0408 != 0)
			Sub_40D6E0(0, 0, 5, 0);
		lCount = *(volatile long*)&m_dwUnk014C;
		if (lCount == 1)
		{
			if (m_dwUnk0128 == 0)
			{
				if (m_dwUnk012C == 0)
					dwThrust = *(DWORD*)&m_awUnk01D0[4];
				else
				{
					dwThrust1 = *(DWORD*)&m_awUnk01D0[4];
					HOVER_FIXMULP3(dwThrust1, *(DWORD*)(m_dwUnk0084 + 0x1C));
					dwThrust = dwThrust1;
				}
			}
			else
			{
				dwThrust2 = *(DWORD*)&m_awUnk01D0[4];
				HOVER_FIXMULP3(dwThrust2, *(DWORD*)(m_dwUnk0084 + 0x20));
				dwThrust = dwThrust2;
			}
		}
		else
		{
			dwThrust3 = *(DWORD*)&m_awUnk01D0[4];
			HOVER_FIXMULP3(dwThrust3, *(DWORD*)(m_dwUnk0084 + 0x20));
			dwThrust = dwThrust3;
		}
		*(DWORD*)&m_awUnk01D0[2] = dwThrust;
		if (lCount <= 0)
			return;
		lDelta = *(DWORD*)&m_pad0160[0];
		dwCos = g_adwSinTable[(((lDelta + 0x80) & 0x1FF) << 4) + 1];
		HOVER_FIXMULWP(dwCos, 4);
		*(DWORD*)&m_awUnk01AC[4] = dwCos;
		dwSin = g_adwSinTable[((lDelta & 0x1FF) << 4) + 1];
		HOVER_FIXMULWP(dwSin, 4);
		*(DWORD*)&m_awUnk01AC[6] = dwSin;
		wAngle = m_awUnk0194[5];
		pdwParams = (DWORD*)(m_dwUnk0084 + 0x28);
		dwMargin1 = *pdwParams;
		HOVER_FIXMULWP(dwMargin1, 3);
		lAngle = (long)(short)wAngle;
		if ((long)((short*)&dwMargin1)[1] + lDelta > lAngle)
		{
			dwMargin2 = *pdwParams;
			HOVER_FIXMULWP(dwMargin2, 3);
			if (lDelta - (long)((short*)&dwMargin2)[1] < lAngle)
			{
				m_awUnk0194[0] = 0;
				// The original keeps lDelta in a register and does the
				// 16-bit subtract in it (sub di, [wAngle]); writing it
				// back through a WORD* would home the local instead.
				m_dwUnk014C--;
				m_awUnk0194[1] = (WORD)lDelta - wAngle;
				return;
			}
		}
		if (Hover_sub_41B830(lDelta, lAngle) != 0)
		{
			dwSpeed1 = *pdwParams;
			HOVER_FIXMULWP(dwSpeed1, 3);
			fxSpeed = Hover_FixArg((DWORD)-(long)dwSpeed1);
		}
		else
		{
			dwSpeed2 = *pdwParams;
			HOVER_FIXMULWP(dwSpeed2, 3);
			fxSpeed = Hover_FixArg(dwSpeed2);
		}
		m_dwUnk014C--;
			*(DWORD*)&m_awUnk0194[0] = fxSpeed.m_dwValue;
		return;
	}
	m_dwUnk014C = 0xFFFFFFFF;
}

// Velocity integration: friction/brush forces on the +0x1B4/+0x1B8
// velocity pair, acceleration into +0x1AC/+0x1B0, clamping against the
// +0x1D4 thrust limit, angular drag from params +0x18, then the position
// (+0x44/+0x48) update and the proximity point refresh.

// hover.exe:0x40B0B0
// FUNCTION: HOVER 0x40b0b0
void CPlayer::Sub_40B0B0()
{
	DWORD dwDist;
	DWORD dwScale;
	DWORD dwThrust;
	DWORD dwDrag;
	DWORD dwDx;
	DWORD dwDy;
	DWORD dwOut;
	DWORD dwCos;
	DWORD dwSin;
	long lAngle;
	long lDiff;

	if (m_dwUnk01E0 != 0)
	{
		if (*(DWORD*)&m_awUnk01AC[4] == 0)
		{
			*(DWORD*)&m_awUnk01AC[4] = *(DWORD*)&m_awUnk01AC[0];
			if (m_dwUnk0154 != 0 && m_dwUnk0148 != 0)
				*(DWORD*)&m_awUnk01AC[4] =
					(DWORD)((long)*(DWORD*)&m_awUnk01AC[4] / -3);
			else
				HOVER_FIXMULP3(*(DWORD*)&m_awUnk01AC[4],
					*(DWORD*)(m_dwUnk0084 + 4));
		}
		if (*(DWORD*)&m_awUnk01AC[6] == 0)
		{
			*(DWORD*)&m_awUnk01AC[6] = *(DWORD*)&m_awUnk01AC[2];
			if (m_dwUnk0154 != 0 && m_dwUnk0148 != 0)
				*(DWORD*)&m_awUnk01AC[6] =
					(DWORD)((long)*(DWORD*)&m_awUnk01AC[6] / -3);
			else
				HOVER_FIXMULP3(*(DWORD*)&m_awUnk01AC[6],
					*(DWORD*)(m_dwUnk0084 + 4));
		}
		if (labs((long)*(DWORD*)&m_awUnk01AC[4]) < 0x28F &&
			labs((long)*(DWORD*)&m_awUnk01AC[6]) < 0x28F)
		{
			*(DWORD*)&m_awUnk01AC[4] = (DWORD)-(long)*(DWORD*)&m_awUnk01AC[0];
			*(DWORD*)&m_awUnk01AC[6] = (DWORD)-(long)*(DWORD*)&m_awUnk01AC[2];
		}
	}
	*(DWORD*)&m_awUnk01AC[0] += *(DWORD*)&m_awUnk01AC[4];
	*(DWORD*)&m_awUnk01AC[2] += *(DWORD*)&m_awUnk01AC[6];
	((WORD*)&dwDist)[1] = (WORD)Hover_sub_408A00((long)(short)m_awUnk01AC[1],
		(long)(short)m_awUnk01AC[3], 0, 0);
	((WORD*)&dwDist)[0] = 0;
	if ((long)dwDist < (long)*(DWORD*)&m_awUnk01D0[2] ||
		(long)dwDist == (long)*(DWORD*)&m_awUnk01D0[2])
	{
		// within the thrust limit
	}
	else
	{
		dwScale = *(DWORD*)&m_awUnk01D0[2];
		HOVER_FIXDIVLP(dwScale, dwDist);
		HOVER_FIXMULP3(*(DWORD*)&m_awUnk01AC[0], dwScale);
		HOVER_FIXMULP3(*(DWORD*)&m_awUnk01AC[2], dwScale);
		dwDist = *(DWORD*)&m_awUnk01D0[2];
	}
	dwThrust = *(DWORD*)&m_awUnk0194[0];
	HOVER_FIXMULP3(dwThrust, *(DWORD*)(m_dwUnk0084 + 0x18));
	// The drag is subtracted in memory in the original (rule 101's
	// by-pointer fixed-point operators): the volatile compound assignment
	// is the `mov [T],a / sub [T],b / mov eax,[T]` it emits.
	dwDrag = *(DWORD*)&m_awUnk0194[2];
	*(volatile DWORD*)&dwDrag -= dwThrust;
	*(DWORD*)&m_awUnk0194[0] += dwDrag;
	*(DWORD*)&m_awUnk0194[4] += *(DWORD*)&m_awUnk0194[0];
	if (m_dwUnk01DC != 0)
	{
		if ((long)dwDist < 0x40000 || (long)dwDist == 0x40000)
		{
			// too slow to stir the velocity
		}
		else
		{
			lAngle = (long)(short)m_awUnk0194[5];
			lDiff = ((long)((short*)Hover_sub_4386D0(&dwOut,
				*(DWORD*)&m_awUnk01AC[2],
				*(DWORD*)&m_awUnk01AC[0]))[1] - lAngle) & 0x1FF;
			if (lDiff > 0xC0 && lDiff < 0x140)
				lAngle = (lAngle + 0x100) & 0x1FF;
			dwCos = g_adwSinTable[(((lAngle + 0x80) & 0x1FF) << 4) + 1];
			dwSin = g_adwSinTable[((lAngle & 0x1FF) << 4) + 1];
			HOVER_FIXMULP3(dwSin, dwDist);
			HOVER_FIXMULP3(dwCos, dwDist);
			dwDx = dwCos - *(DWORD*)&m_awUnk01AC[0];
			dwDy = dwSin - *(DWORD*)&m_awUnk01AC[2];
			lDiff = labs((long)dwDx);
			if (lDiff < 0x100 || lDiff == 0x100)
			{
				// small correction
			}
			else
			{
				HOVER_FIXMULP3(dwDx, *(DWORD*)(m_dwUnk0084 + 0x24));
			}
			lDiff = labs((long)dwDy);
			if (0x100 > lDiff || 0x100 == lDiff)
			{
				// small correction
			}
			else
			{
				HOVER_FIXMULP3(dwDy, *(DWORD*)(m_dwUnk0084 + 0x24));
			}
			*(DWORD*)&m_awUnk01AC[0] += dwDx;
			*(DWORD*)&m_awUnk01AC[2] += dwDy;
		}
	}
	*(DWORD*)&m_wUnk44 += *(DWORD*)&m_awUnk01AC[0];
	*(DWORD*)&m_wUnk48 += *(DWORD*)&m_awUnk01AC[2];
	*(DWORD*)&m_awUnk0194[4] &= 0x1FFFFFF;
	Sub_408550(dwDist);
	((CCollider*)theApp.m_pBumperDoc->m_dwUnk8388)->Sub_40B510(this);
}

// Collision response against the player at dwA: pushes the pair apart
// when the proximity points are closer than 40 units, then splits both
// velocities along the collision normal and exchanges the tangential
// parts scaled by params +0x14/+0x10. Returns whether either player was
// moving along the normal.

// hover.exe:0x42B8E0
// FUNCTION: HOVER 0x42b8e0
int CPlayer::Sub_42B8E0(DWORD dwA, DWORD dwB)
{
	CPlayer* pOther;
	POINT ptMine;
	POINT ptProx;
	DWORD* pProx;
	DWORD* pMine;
	DWORD dwP1;
	DWORD dwP2;
	DWORD dwPX;
	DWORD dwPY;
	DWORD dwOX;
	DWORD dwOY;
	DWORD dwDy;
	DWORD dwDx;
	DWORD dwOut;
	DWORD dwOut2;
	DWORD dwAngle;
	DWORD dwAngle180;
	DWORD dwDist;
	DWORD dwAX;
	DWORD dwAY;
	DWORD dwBX;
	DWORD dwBY;
	DWORD dwCX;
	DWORD dwCY;
	DWORD dwDX;
	DWORD dwDY;
	DWORD dwT1;
	DWORD dwT2;
	DWORD dwT3;
	DWORD dwT4;
	DWORD dwT5;
	DWORD dwT6;
	DWORD dwT7;
	DWORD dwT8;
	DWORD dwVelAngle;
	DWORD dwVelAngleOther;
	DWORD dwAdj;
	WORD wHalf;
	int bThis;
	int bOther;

	if (dwB == 0)
		return 0;
	pOther = (CPlayer*)dwA;
	pMine = &m_dwUnk0180;
	m_dwUnk01CC = 0;
	pOther->m_dwUnk01CC = 0;
	Sub_42BE50(pOther, &dwP1, &dwP2);
	// Virtual call at vtable slot 0x4C in the original
	// (hover.exe:0x4089E0 on CPlayer): returns &pOther->m_dwUnk0180.
	pProx = pOther->Sub_4089E0();
	if (dwP1 == 0)
		pMine += 2;
	ptMine = Hover_PtOf(pMine);
	dwPX = (DWORD)ptMine.x;
	dwPY = (DWORD)ptMine.y;
	if (dwP2 == 0)
		pProx += 2;
	ptProx = Hover_PtOf(pProx);
	dwOX = (DWORD)ptProx.x;
	dwOY = (DWORD)ptProx.y;
	((WORD*)&dwDy)[0] = 0;
	((WORD*)&dwDy)[1] = (WORD)dwOY - (WORD)dwPY;
	((WORD*)&dwDx)[0] = 0;
	((WORD*)&dwDx)[1] = (WORD)dwOX - (WORD)dwPX;
	dwAngle = (long)((short*)Hover_sub_4386D0(&dwOut, dwDy, dwDx))[1];
	dwAngle180 = (dwAngle + 0x100) & 0x1FF;
	dwDist = Hover_sub_408A00(dwPX, dwPY, dwOX, dwOY);
	if ((long)dwDist < 0x28)
	{
		wHalf = (WORD)((0x28 - (long)dwDist) / 2);
		dwT1 = g_adwSinTable[(((dwAngle180 + 0x80) & 0x1FF) << 4) + 1];
		HOVER_FIXMULWP(dwT1, wHalf);
		*(DWORD*)&m_wUnk44 += dwT1;
		dwT2 = g_adwSinTable[((dwAngle180 & 0x1FF) << 4) + 1];
		HOVER_FIXMULWP(dwT2, wHalf);
		*(DWORD*)&m_wUnk48 += dwT2;
		dwT3 = g_adwSinTable[(((dwAngle + 0x80) & 0x1FF) << 4) + 1];
		HOVER_FIXMULWP(dwT3, wHalf);
		*(DWORD*)&pOther->m_wUnk44 += dwT3;
		dwT4 = g_adwSinTable[((dwAngle & 0x1FF) << 4) + 1];
		HOVER_FIXMULWP(dwT4, wHalf);
		*(DWORD*)&pOther->m_wUnk48 += dwT4;
	}
	dwVelAngle = (long)((short*)Hover_sub_4386D0(&dwOut,
		*(DWORD*)&m_awUnk01AC[2], *(DWORD*)&m_awUnk01AC[0]))[1];
	dwVelAngleOther = (long)((short*)Hover_sub_4386D0(&dwOut,
		*(DWORD*)&pOther->m_awUnk01AC[2],
		*(DWORD*)&pOther->m_awUnk01AC[0]))[1];
	bThis = Hover_sub_41B830(dwVelAngle, dwAngle + 0x80);
	bOther = Hover_sub_41B830(dwVelAngleOther, dwAngle180 + 0x80);
	((WORD*)&dwAX)[0] = 0;
	((WORD*)&dwAX)[1] = 0;
	((WORD*)&dwAY)[0] = 0;
	((WORD*)&dwAY)[1] = 0;
	((WORD*)&dwBX)[0] = 0;
	((WORD*)&dwBX)[1] = 0;
	((WORD*)&dwBY)[0] = 0;
	((WORD*)&dwBY)[1] = 0;
	((WORD*)&dwCX)[0] = 0;
	((WORD*)&dwCX)[1] = 0;
	((WORD*)&dwCY)[0] = 0;
	((WORD*)&dwCY)[1] = 0;
	((WORD*)&dwDX)[0] = 0;
	((WORD*)&dwDX)[1] = 0;
	((WORD*)&dwDY)[0] = 0;
	((WORD*)&dwDY)[1] = 0;
	if (bThis != 0)
	{
		Hover_sub_40B6E0(*(DWORD*)&m_awUnk01AC[0],
			*(DWORD*)&m_awUnk01AC[2], dwAngle, &dwAX, &dwBX);
		*(DWORD*)&m_awUnk01AC[0] = dwAX;
		*(DWORD*)&m_awUnk01AC[2] = dwAY;
		dwAdj = ((long)(short)pOther->m_awUnk0194[5] +
			((dwP1 < 1) ? 0x100 : 0)) & 0x1FF;
		Hover_sub_41B7A0(&dwOut2, dwBX, dwBY, dwAdj);
		dwT5 = *(volatile DWORD*)&dwOut2;
		HOVER_FIXMULP3(dwT5, *(DWORD*)(m_dwUnk0084 + 0x14));
		*(DWORD*)&pOther->m_awUnk0194[4] += dwT5;
		dwT6 = *(volatile DWORD*)&dwOut2;
		HOVER_FIXMULP3(dwT6, *(DWORD*)(m_dwUnk0084 + 0x10));
		*(DWORD*)&pOther->m_awUnk0194[0] += dwT6;
	}
	if (bOther != 0)
	{
		Hover_sub_40B6E0(*(DWORD*)&pOther->m_awUnk01AC[0],
			*(DWORD*)&pOther->m_awUnk01AC[2], dwAngle180, &dwCX, &dwDX);
		*(DWORD*)&pOther->m_awUnk01AC[0] = dwCX;
		*(DWORD*)&pOther->m_awUnk01AC[2] = dwCY;
		*(DWORD*)&m_awUnk01AC[0] += dwDX;
		*(DWORD*)&m_awUnk01AC[2] += dwDY;
		dwAdj = ((long)(short)m_awUnk0194[5] +
			((dwP1 < 1) ? 0x100 : 0)) & 0x1FF;
		Hover_sub_41B7A0(&dwOut2, dwDX, dwDY, dwAdj);
		dwT7 = *(volatile DWORD*)&dwOut2;
		HOVER_FIXMULP3(dwT7, *(DWORD*)(m_dwUnk0084 + 0x14));
		*(DWORD*)&m_awUnk0194[4] += dwT7;
		dwT8 = *(volatile DWORD*)&dwOut2;
		HOVER_FIXMULP3(dwT8, *(DWORD*)(m_dwUnk0084 + 0x10));
		*(DWORD*)&m_awUnk0194[0] += dwT8;
	}
	if (bThis != 0)
	{
		*(DWORD*)&pOther->m_awUnk01AC[0] += dwBX;
		*(DWORD*)&pOther->m_awUnk01AC[2] += dwBY;
	}
	m_dwUnk01CC = bThis;
	pOther->m_dwUnk01CC = bOther;
	if (bThis == 0)
	{
		if (bOther == 0)
			return 0;
	}
	return 1;
}

// Proximity test between this player's points at +0x180/+0x188 and
// pOther's (fetched through the vtable slot 0x4C virtual, which returns
// &pOther->m_dwUnk0180), using the max + min/2 metric against the sum of
// the two +0x190 range limits. *pdwA/*pdwB report which point pairs
// matched; returns 1 on the first match.

// hover.exe:0x42BE50
// FUNCTION: HOVER 0x42be50
int CPlayer::Sub_42BE50(CPlayer* pOther, DWORD* pdwA, DWORD* pdwB)
{
	DWORD* pProx;
	DWORD* pMine;
	long lLimit;
	long lX;
	long lY;
	long lMin;

	pMine = &m_dwUnk0180;
	// Virtual call at vtable slot 0x4C in the original
	// (hover.exe:0x4089E0 on CPlayer): returns &pOther->m_dwUnk0180.
	pProx = pOther->Sub_4089E0();
	lLimit = (long)(pProx[4] + m_dwUnk0190);
	{
		long lMineP40 = (long)m_dwUnk01A4 + 0x40;
		long lOtherP40 = (long)pOther->m_dwUnk01A4 + 0x40;

		if (lMineP40 < (long)pOther->m_dwUnk01A4 ||
			(long)m_dwUnk01A4 > lOtherP40)
			return 0;
	}
	*pdwB = 1;
	*pdwA = 1;
	lX = labs((long)pMine[0] - (long)pProx[0]);
	lY = labs((long)pMine[1] - (long)pProx[1]);
	lMin = (lY < lX) ? lY : lX;
	if (lY - (lMin >> 1) + lX < lLimit)
		return 1;
	*pdwA = 0;
	lX = labs((long)pMine[2] - (long)pProx[0]);
	lY = labs((long)pMine[3] - (long)pProx[1]);
	lMin = (lY < lX) ? lY : lX;
	if (lY - (lMin >> 1) + lX < lLimit)
		return 1;
	*pdwB = 0;
	lX = labs((long)pMine[2] - (long)pProx[2]);
	lY = labs((long)pMine[3] - (long)pProx[3]);
	lMin = (lY < lX) ? lY : lX;
	if (lY - (lMin >> 1) + lX < lLimit)
		return 1;
	*pdwA = 1;
	lX = labs((long)pMine[0] - (long)pProx[2]);
	lY = labs((long)pMine[1] - (long)pProx[3]);
	lMin = (lY < lX) ? lY : lX;
	return (lY - (lMin >> 1) + lX < lLimit);
}

// Wall bounce response, called from CWall::Sub_42AD40 once the collider
// has reported a hit: the two proximity points of the player
// (m_dwUnk0180/m_dwUnk0184 in front, m_dwUnk0188/m_dwUnk018C behind) are
// measured against the wall segment and the closer one decides the
// contact. When the contact falls between the wall's endpoints the player
// is reflected about the wall angle and pushed back out along the wall
// normal; when it falls off an end, the endpoint bounce Sub_421B50 takes
// over. Returns nonzero when the wall was actually hit (zero when the
// player is already moving away from it).

// hover.exe:0x41A6B0
// FUNCTION: HOVER 0x41a6b0
int CPlayer::Sub_41A6B0(CGameObj* pWall, DWORD dwB)
{
	CSub40D320* pSeg;
	DWORD* pdwFront;
	DWORD dwOnSeg1;
	DWORD dwSide1;
	DWORD dwFirst1;
	DWORD dwOnSeg2;
	DWORD dwSide2;
	DWORD dwFirst2;
	int nDist1;
	int nDist2;
	long lAngle;
	long lWallAngle;
	DWORD adwAngle[2];
	DWORD* pdwAngle;
	DWORD dwCos;
	DWORD dwSin;

	pdwFront = &m_dwUnk0180;
	pSeg = &pWall->m_obj04;
	nDist1 = pSeg->Sub_40C9B0(pdwFront, &dwOnSeg1, &dwSide1, &dwFirst1);
	nDist2 = pSeg->Sub_40C9B0(&m_dwUnk0188, &dwOnSeg2, &dwSide2, &dwFirst2);
	// Both segment cases put the endpoint bounce *after* the body in
	// hover.exe (0x41A70F jumps forward to it, 0x41A80E), so the guard is
	// spelled the positive way round with the Sub_421B50 tail last.
	if (nDist2 > nDist1)
	{
		if (dwOnSeg1 != 0)
		{
			pdwAngle = Hover_sub_4386D0(adwAngle, *(DWORD*)&m_awUnk01AC[2],
				*(DWORD*)&m_awUnk01AC[0]);
			lWallAngle = (long)pSeg->m_dwUnk1C;
			if (Hover_sub_41B830((long)((short*)pdwAngle)[1], lWallAngle) !=
				(dwSide1 == 0))
				return 0;
			m_dwUnk01CC = 1;
			Sub_41AB10(1, lWallAngle);
			if (nDist1 < 0x32)
			{
				lAngle = lWallAngle - 0x80 + (dwSide1 == 0 ? 0x100 : 0);
				dwCos = g_adwSinTable[(((lAngle + 0x80) & 0x1FF) << 4) + 1];
				HOVER_FIXMULWP(dwCos, 0x32 - (WORD)nDist1);
				*(DWORD*)&m_wUnk44 += dwCos;
				dwSin = g_adwSinTable[((lAngle & 0x1FF) << 4) + 1];
				HOVER_FIXMULWP(dwSin, 0x32 - (WORD)nDist1);
				*(DWORD*)&m_wUnk48 += dwSin;
			}
			return 1;
		}
		return Sub_421B50(1, pdwFront[0], pdwFront[1], pWall, dwFirst1,
			nDist1);
	}

	// The back point is the closer one. Note that the endpoint case below
	// still passes the *front* point to Sub_421B50: the original reuses
	// the pointer it set up for the first Sub_40C9B0 call, so the back
	// point never reaches the endpoint bounce.
	if (dwOnSeg2 != 0)
	{
		pdwAngle = Hover_sub_4386D0(adwAngle, *(DWORD*)&m_awUnk01AC[2],
			*(DWORD*)&m_awUnk01AC[0]);
		lWallAngle = (long)pSeg->m_dwUnk1C;
		if (Hover_sub_41B830((long)((short*)pdwAngle)[1], lWallAngle) !=
			(dwSide2 == 0))
			return 0;
		m_dwUnk01CC = 1;
		Sub_41AB10(0, lWallAngle);
		if (nDist2 < 0x32)
		{
			lAngle = lWallAngle - 0x80 + (dwSide2 == 0 ? 0x100 : 0);
			dwCos = g_adwSinTable[(((lAngle + 0x80) & 0x1FF) << 4) + 1];
			HOVER_FIXMULWP(dwCos, 0x32 - (WORD)nDist2);
			*(DWORD*)&m_wUnk44 += dwCos;
			dwSin = g_adwSinTable[((lAngle & 0x1FF) << 4) + 1];
			HOVER_FIXMULWP(dwSin, 0x32 - (WORD)nDist2);
			*(DWORD*)&m_wUnk48 += dwSin;
		}
		return 1;
	}
	return Sub_421B50(1, pdwFront[0], pdwFront[1], pWall, dwFirst2, nDist2);
}

// Reflects the velocity (*(DWORD*)&m_awUnk01AC[0], *(DWORD*)&m_awUnk01AC[2]) about dwAngle: the
// component along the angle is removed twice (once through the first pair
// of stores, once through the second, exactly as the original wrote it),
// and the removed part is fed back into the heading (*(DWORD*)&m_awUnk0194[4]) and the
// turn rate (m_dwUnk0194) scaled by the player parameters at +0x14/+0x10.
// dwFlip selects which side of the wall the reaction is taken from.

// hover.exe:0x41AB10
// FUNCTION: HOVER 0x41ab10
void CPlayer::Sub_41AB10(DWORD dwFlip, DWORD dwAngle)
{
	DWORD adwAlong[2];
	DWORD adwSide[2];
	DWORD dwTangent;
	DWORD* pdwParams;
	DWORD* pdwVel;
	DWORD dwSpin;
	DWORD dwTurn;

	// The two 16.16 pairs are cleared half-word by half-word, as the
	// original's fixed-point zero initializer did.
	pdwVel = (DWORD*)&m_awUnk01AC[0];
	((WORD*)adwAlong)[0] = 0;
	((WORD*)adwAlong)[1] = 0;
	((WORD*)adwAlong)[2] = 0;
	((WORD*)adwAlong)[3] = 0;
	((WORD*)adwSide)[0] = 0;
	((WORD*)adwSide)[1] = 0;
	((WORD*)adwSide)[2] = 0;
	((WORD*)adwSide)[3] = 0;
	Hover_sub_40B6E0(pdwVel[0], pdwVel[1], dwAngle, adwAlong, adwSide);
	pdwVel[0] = adwSide[0];
	*(DWORD*)&m_awUnk01AC[2] = adwSide[1];
	pdwVel[0] = adwSide[0] - adwAlong[0];
	*(DWORD*)&m_awUnk01AC[2] = adwSide[1] - adwAlong[1];
	Hover_sub_41B7A0(&dwTangent, adwAlong[0], adwAlong[1],
		(((dwFlip == 0) ? 0x100 : 0) + (long)(short)m_awUnk0194[5]) & 0x1FF);
	pdwParams = (DWORD*)m_dwUnk0084;
	dwSpin = *(volatile DWORD*)&dwTangent;
	HOVER_FIXMULP3(dwSpin, pdwParams[5]);
	*(DWORD*)&m_awUnk0194[4] += dwSpin;
	dwTurn = *(volatile DWORD*)&dwTangent;
	HOVER_FIXMULP3(dwTurn, pdwParams[4]);
	*(DWORD*)&m_awUnk0194[0] += dwTurn;
}

// The endpoint case of the wall bounce: the contact is the segment
// endpoint selected by dwFirst (the first one when it is nonzero), and
// the bounce normal is the direction from that endpoint to (dwX, dwY).
// Returns 0 when the player is already moving away from the endpoint.

// hover.exe:0x421B50
// FUNCTION: HOVER 0x421b50
int CPlayer::Sub_421B50(DWORD dwFlip, DWORD dwX, DWORD dwY, CGameObj* pWall,
	DWORD dwFirst, int nDist)
{
	DWORD* pdwEnd;
	POINT ptEnd1;
	POINT ptEnd2;
	DWORD adwAngle[2];
	DWORD* pdwAngle;
	DWORD dwDX;
	DWORD dwDY;
	long lNormal;
	DWORD dwCos;
	DWORD dwSin;

	if (dwFirst != 0)
	{
		ptEnd1 = Hover_PtOf(&pWall->m_obj04.m_dwUnk00);
		pdwEnd = (DWORD*)&ptEnd1;
	}
	else
	{
		ptEnd2 = Hover_PtOf(&pWall->m_obj04.m_dwUnk08);
		pdwEnd = (DWORD*)&ptEnd2;
	}
	// The 16.16 deltas from the contact point to (dwX, dwY): only the
	// integer halves are set, so the low words stay zero.
	((WORD*)&dwDY)[1] = (WORD)pdwEnd[1] - (WORD)dwY;
	((WORD*)&dwDY)[0] = 0;
	((WORD*)&dwDX)[0] = 0;
	((WORD*)&dwDX)[1] = (WORD)pdwEnd[0] - (WORD)dwX;
	lNormal = ((long)((short*)Hover_sub_4386D0(adwAngle, dwDY, dwDX))[1] +
		0x80) & 0x1FF;
	pdwAngle = Hover_sub_4386D0(adwAngle, *(DWORD*)&m_awUnk01AC[2],
		*(DWORD*)&m_awUnk01AC[0]);
	if (Hover_sub_41B830((long)((short*)pdwAngle)[1], lNormal) == 0)
		return 0;
	m_dwUnk01CC = 1;
	Sub_41AB10(dwFlip, lNormal);
	if (nDist < 0x32)
	{
		dwCos = g_adwSinTable[(((lNormal + 0x100) & 0x1FF) << 4) + 1];
		HOVER_FIXMULWP(dwCos, 0x32 - (WORD)nDist);
		*(DWORD*)&m_wUnk44 += dwCos;
		dwSin = g_adwSinTable[(((lNormal + 0x80) & 0x1FF) << 4) + 1];
		HOVER_FIXMULWP(dwSin, 0x32 - (WORD)nDist);
		*(DWORD*)&m_wUnk48 += dwSin;
	}
	return 1;
}

/////////////////////////////////////////////////////////////////////////////
// CHumanPlayer

// hover.exe:0x421FA0
// FUNCTION: HOVER 0x421fa0
CHumanPlayer::CHumanPlayer()
{
	m_dwUnk01F8 = 1;
	m_dwUnk0084 = (DWORD)theApp.m_pUnk014C;
	Sub_414A80();
	m_dwUnk0140 = 0;
	m_dwUnk0144 = 0;
	m_dwUnk0124 = 0;
	m_dwUnk0128 = 0;
	m_dwUnk012C = 0;
	m_dwUnk0130 = 0;
	m_dwUnk0134 = 0;
	m_dwUnk0138 = 0;
}

// hover.exe:0x422050
// FUNCTION: HOVER 0x422050
CHumanPlayer::~CHumanPlayer()
{
	// empty body in the original (just the base-class destructor call)
}

// hover.exe:0x409180
// FUNCTION: HOVER 0x409180
void CHumanPlayer::Sub_42AD30()
{
	CBumperView* pView;
	DWORD dwSound;

	m_dwUnk015C = 0;
	if (m_dwUnk0158 == 0)
		m_dwUnk015C = 0xFFFFFFFF;
	if (m_dwUnk5C == 0 && theApp.m_dwUnk01B8 == 3)
	{
		m_wUnk01F6 = 0;
		m_wUnk01F4 = 0;
		// 32-bit copy: m_wUnk01F0 = m_wUnk01F4, m_wUnk01F2 = m_wUnk01F6
		*(DWORD*)&m_wUnk01F0 = *(DWORD*)&m_wUnk01F4;
		pView = Hover_sub_401140();
		EnterCriticalSection(&theApp.m_csUnk0134);
		if (pView->m_dwUnk0070 != pView->m_dwUnk006C)
		{
			DWORD dwC = pView->m_dwUnk006C;

			m_wUnk01F0 = 0;
			m_wUnk01F2 = ((long)dwC < 1) ? 1 : -1;
		}
		if (pView->m_dwUnk0074 != pView->m_dwUnk0078)
		{
			DWORD dwC2 = pView->m_dwUnk0078;

			m_wUnk01F4 = 0;
			m_wUnk01F6 = ((long)dwC2 < 1) ? 1 : -1;
			if (m_dwUnk012C != 0)
				dwSound = 1;
			else
				dwSound = (m_dwUnk0128 < 1) ? 2 : 3;
		}
		else
			dwSound = 0;
		LeaveCriticalSection(&theApp.m_csUnk0134);
		if (dwSound != g_dwUnk_4C54A0 && theApp.m_dwUnk01B8 == 3)
		{
			if (theApp.m_dwUnk0404 != 0)
			{
				switch (dwSound)
				{
				case 0:
					((CSub40B800*)theApp.m_pBumperDoc->m_adwUnk83EC[0x1A])->
						Sub_40B800(5, 5, 0xFFFF);
					break;
				case 1:
					((CSub40B800*)theApp.m_pBumperDoc->m_adwUnk83EC[0x1B])->
						Sub_40B800(5, 5, 0xFFFF);
					break;
				case 2:
					((CSub40B800*)theApp.m_pBumperDoc->m_adwUnk83EC[0x1C])->
						Sub_40B800(5, 5, 0xFFFF);
					break;
				case 3:
					((CSub40B800*)theApp.m_pBumperDoc->m_adwUnk83EC[0x1D])->
						Sub_40B800(5, 5, 0xFFFF);
					break;
				}
			}
			g_dwUnk_4C54A0 = dwSound;
		}
	}
	CPlayer::Sub_42AD30();
}

// hover.exe:0x40C400
// FUNCTION: HOVER 0x40c400
void CHumanPlayer::Sub_42AD40(CPlayer* pOther, DWORD dwB)
{
	CPlayer::Sub_42AD40(pOther, dwB);
}

// The three overrides below are the original's inline forwarders: the
// compiler tail-jumps straight into the CPlayer version (hover.exe:
// 0x4089D0 -> 0x408960, 0x4089F0 -> 0x408990, 0x42C240 -> 0x42C050).

// hover.exe:0x4089D0
// FUNCTION: HOVER 0x4089d0
void CHumanPlayer::Sub_4087D0()
{
	CPlayer::Sub_4087D0();
}

// hover.exe:0x4089F0
// FUNCTION: HOVER 0x4089f0
void CHumanPlayer::Sub_4087F0()
{
	CPlayer::Sub_4087F0();
}

// hover.exe:0x42C240
// FUNCTION: HOVER 0x42c240
void CHumanPlayer::Sub_413020()
{
	CPlayer::Sub_413020();
}

/////////////////////////////////////////////////////////////////////////////
// CMerlinStatic

// The constructor only presets the words and flags; the six CStrings are
// built by their own default constructors and the destructor
// (hover.exe:0x419CF0, the scalar deleting destructor) is likewise
// compiler-generated.

// hover.exe:0x419B30
// FUNCTION: HOVER 0x419b30
CMerlinStatic::CMerlinStatic()
{
	m_wUnkA4 = 0;
	m_wUnkA8 = 0;
	m_dwUnkB0 = 0;
	m_wUnkAA = 0;
	m_dwUnkB4 = 0;
	m_wUnkAC = 0;
	m_dwUnkC0 = 0;
	m_wUnkB8 = 0;
	m_dwUnkBC = 1;
	m_wUnkA6 = 0x80;
	m_wUnkBA = 0;
	// hover.exe:0x419C11 calls CString::Empty on all six texture names
	// after the scalar block, although they were just default-constructed
	m_strUnk44.Empty();
	m_strUnk50.Empty();
	m_strUnk5C.Empty();
	m_strUnk68.Empty();
	m_strUnk74.Empty();
	m_strUnk80.Empty();
	m_pUnk8C = NULL;
	m_pUnk90 = NULL;
	m_pUnk94 = NULL;
	m_pUnk98 = NULL;
	m_pUnk9C = NULL;
	m_pUnkA0 = NULL;
}

// hover.exe:0x419CF0 (the original has no destructor body of its own; the
// address is the compiler-generated scalar deleting destructor, which
// destroys the six CStrings and runs ~CMerlinLine -- see rule 34 in
// notes/perfect-match.md)
// The destructor is defined in the class declaration, so the compiler folds
// it into the scalar deleting destructor that the vtable slot holds - which
// is what hover.exe:0x419CF0 is. The block below emits no code; it only gives
// the reccmp marker a declaration to name ??_GCMerlinStatic@@ (rules 34/184).
// hover.exe:0x419CF0
#if 0
// FUNCTION: HOVER 0x419cf0
CMerlinStatic::~CMerlinStatic()
{
}
#endif

// Serialize (vtable slot 2): the six texture names, the two edge heights
// and two more words, then three flag bytes. The trailing comment blob is
// used as a version marker here: a count of five or more carries the
// m_dwUnkC0 byte and the two texture u origins. On load the face
// registers itself in g_mapUnk_4A4578 under its object name and resolves
// each non-empty texture name in g_mapUnk_4A1D30; the front/back flags
// record whether the resolved texture actually has frame data.

// hover.exe:0x419E00
// FUNCTION: HOVER 0x419e00
void CMerlinStatic::Serialize(CArchive& ar)
{
	CMerlinArchive* pAr = (CMerlinArchive*)&ar;
	short sCount;
	int nLeft;
	BYTE by;

	CMerlinLine::Serialize(ar);
	if (ar.IsStoring())
	{
		*pAr << m_strUnk44;
		*pAr << m_strUnk50;
		*pAr << m_strUnk5C;
		*pAr << m_strUnk68;
		*pAr << m_strUnk74;
		*pAr << m_strUnk80;
		pAr->WriteWord(m_wUnkA4);
		pAr->WriteWord(m_wUnkA6);
		pAr->WriteWord(m_wUnkA8);
		pAr->WriteWord(m_wUnkAA);
		pAr->WriteByte((BYTE)m_dwUnkB0);
		pAr->WriteByte((BYTE)m_dwUnkB4);
		pAr->WriteByte((BYTE)m_dwUnkBC);
		pAr->WriteWord(5);		// the five extra bytes written below
		pAr->WriteByte((BYTE)m_dwUnkC0);
		pAr->WriteWord(m_wUnkB8);
		pAr->WriteWord(m_wUnkBA);
	}
	else
	{
		if (!m_strUnk04.IsEmpty())
			g_mapUnk_4A4578[m_strUnk04] = this;
		*pAr >> m_strUnk44;
		*pAr >> m_strUnk50;
		*pAr >> m_strUnk5C;
		*pAr >> m_strUnk68;
		*pAr >> m_strUnk74;
		*pAr >> m_strUnk80;
		if (strcmp(m_strUnk44, "") != 0)
		{
			if (!g_mapUnk_4A1D30.Lookup(m_strUnk44,
					(CObject*&)m_pUnk8C))
				m_pUnk8C = NULL;
		}
		if (strcmp(m_strUnk50, "") != 0)
		{
			if (!g_mapUnk_4A1D30.Lookup(m_strUnk50,
					(CObject*&)m_pUnk90))
				m_pUnk90 = NULL;
		}
		if (strcmp(m_strUnk5C, "") != 0)
		{
			if (!g_mapUnk_4A1D30.Lookup(m_strUnk5C,
					(CObject*&)m_pUnk94))
			{
				m_pUnk94 = NULL;
			}
			else
			{
				m_dwUnkB0 = 1;
				if (m_pUnk94->m_dwUnk1C == 0)
					m_dwUnkB0 = 0;
			}
		}
		if (strcmp(m_strUnk68, "") != 0)
		{
			if (!g_mapUnk_4A1D30.Lookup(m_strUnk68,
					(CObject*&)m_pUnk98))
			{
				m_pUnk98 = NULL;
			}
			else
			{
				m_dwUnkB4 = 1;
				if (m_pUnk98->m_dwUnk1C == 0)
					m_dwUnkB4 = 0;
			}
		}
		if (strcmp(m_strUnk74, "") != 0)
		{
			if (!g_mapUnk_4A1D30.Lookup(m_strUnk74,
					(CObject*&)m_pUnk9C))
				m_pUnk9C = NULL;
		}
		if (strcmp(m_strUnk80, "") != 0)
		{
			if (!g_mapUnk_4A1D30.Lookup(m_strUnk80,
					(CObject*&)m_pUnkA0))
				m_pUnkA0 = NULL;
		}
		pAr->ReadWord(&m_wUnkA4);
		pAr->ReadWord(&m_wUnkA6);
		pAr->ReadWord(&m_wUnkA8);
		pAr->ReadWord(&m_wUnkAA);
		*(BYTE*)&m_dwUnkB0 = pAr->ReadByte();
		*(BYTE*)&m_dwUnkB4 = pAr->ReadByte();
		*(BYTE*)&m_dwUnkBC = pAr->ReadByte();
		// The count lives in the frame slot the original reserves with
		// its "sub esp, 4" (at [esp+0x12], the BYTE above it unused):
		// the by-pointer read is what puts it there and what produces
		// the in-memory "sub word ptr [esp+0x12], 5" and guard below.
		pAr->ReadWord((WORD*)&sCount);
		if (sCount >= 5)
		{
			*(BYTE*)&m_dwUnkC0 = pAr->ReadByte();
			pAr->ReadWord(&m_wUnkB8);
			pAr->ReadWord(&m_wUnkBA);
			sCount -= 5;
		}
		// Trailing comment bytes, skipped. The original guards on the
		// in-memory count and then counts down in a register, which is
		// what the explicit copy reproduces.
		if (sCount > 0)
		{
			nLeft = sCount;
			do
			{
				ar >> by;
				*(volatile BYTE*)&by = by;	// see 0x414740
			}
			while (--nLeft != 0);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
// CMerlinLocation

// The IMPLEMENT_SERIAL placement-construction thunk. CMerlinLocation's
// constructor is inline (gameobj.h), so the compiler expands its body -
// the CMerlinObject base call, the vtable store and the four zeroed words -
// into this thunk, behind the null test that `new(p)` needs; that is all
// hover.exe:0x415070 is.
// hover.exe:0x415070
// FUNCTION: HOVER 0x415070
void PASCAL CMerlinLocation::Construct(void* p)
{
	new(p) CMerlinLocation;
}

// Serialize (vtable slot 2): the four point words. On load the point
// registers itself in g_mapUnk_4A4768 under its object name, which is how
// the level loader finds the start positions and the robot AI its
// "BEACON_nn" waypoints.

// hover.exe:0x41B940
// FUNCTION: HOVER 0x41b940
void CMerlinLocation::Serialize(CArchive& ar)
{
	CMerlinArchive* pAr = (CMerlinArchive*)&ar;
	short sCount;
	int nLeft;
	BYTE by;

	CMerlinObject::Serialize(ar);
	if (ar.IsStoring())
	{
		pAr->WriteWord(m_wUnk14);
		pAr->WriteWord(m_wUnk16);
		pAr->WriteWord(m_wUnk18);
		pAr->WriteWord(m_wUnk1A);
		pAr->WriteWord((WORD)0);
	}
	else
	{
		if (!m_strUnk04.IsEmpty())
			g_mapUnk_4A4768[m_strUnk04] = this;
		pAr->ReadWord(&m_wUnk14);
		pAr->ReadWord(&m_wUnk16);
		pAr->ReadWord(&m_wUnk18);
		pAr->ReadWord(&m_wUnk1A);
		sCount = (short)pAr->ReadWord();
		if (sCount > 0)			// trailing comment bytes, skipped
		{
			nLeft = sCount;
			do
			{
				ar >> by;
				*(volatile BYTE*)&by = by;	// see 0x414740
			}
			while (--nLeft != 0);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
// CMerlinBSP

// The IMPLEMENT_SERIAL placement-construction thunk. CMerlinBSP's
// constructor is inline (gameobj.h), so the compiler expands its body -
// the CMerlinLine base call, the vtable store, the four -1 words and the
// two plane doubles - into this thunk, behind the null test that `new(p)`
// needs; that is all hover.exe:0x41BB80 is.
// hover.exe:0x41BB80
// FUNCTION: HOVER 0x41bb80
void PASCAL CMerlinBSP::Construct(void* p)
{
	new(p) CMerlinBSP;
}

// Serialize (vtable slot 2): the face index, the two child indices, one
// more word and the two doubles of the splitting plane.

// hover.exe:0x41BC40
// FUNCTION: HOVER 0x41bc40
void CMerlinBSP::Serialize(CArchive& ar)
{
	short sCount;
	int nLeft;
	BYTE by;
	CMerlinArchive* pAr = (CMerlinArchive*)&ar;

	CMerlinLine::Serialize(ar);
	if (ar.IsStoring())
	{
		pAr->WriteWord(m_wUnk44);
		pAr->WriteWord(m_wUnk46);
		pAr->WriteWord(m_wUnk48);
		pAr->WriteWord(m_wUnk4A);
		pAr->WriteWord(m_wUnk60);
		pAr->WriteDouble(m_dblUnk50);
		pAr->WriteDouble(m_dblUnk58);
		pAr->WriteWord((WORD)0);
	}
	else
	{
		pAr->ReadWord(&m_wUnk44);
		pAr->ReadWord(&m_wUnk46);
		pAr->ReadWord(&m_wUnk48);
		pAr->ReadWord(&m_wUnk4A);
		pAr->ReadWord(&m_wUnk60);
		pAr->ReadDouble(&m_dblUnk50);
		pAr->ReadDouble(&m_dblUnk58);
		sCount = (short)pAr->ReadWord();
		if (sCount > 0)			// trailing comment bytes, skipped
		{
			nLeft = sCount;
			do
			{
				// hover.exe:0x41BE99 `call 0x42CF30`: this loop
				// is one of the two that read the byte through
				// the out-of-line CMerlinArchive reader rather
				// than inlining MFC's operator>>(BYTE&) (see
				// mazeobj.cpp).
				pAr->ReadByte(&by);
			}
			while (--nLeft != 0);
		}
	}
}

// The out-of-line double reader CMerlinBSP::Serialize calls twice
// (hover.exe:0x41BE57 and 0x41BE5F). Like the byte reader at 0x42CF30 it
// is MFC's own CArchive::operator>>(double&) body given a definition of
// its own in this module; the write side stayed inline.
// hover.exe:0x41BEE0
// FUNCTION: HOVER 0x41bee0
CMerlinArchive& CMerlinArchive::ReadDouble(double* pd)
{
	BYTE* p;

	if (m_lpBufCur + 8 > m_lpBufMax)
		FillBuffer(8 - (UINT)(m_lpBufMax - m_lpBufCur));
	// The cursor is advanced through a local and the two halves are then
	// read off it, low dword first: a plain `*pd = *(double*)...` comes
	// out high dword first and re-reads m_lpBufCur in between.
	p = m_lpBufCur + 8;
	m_lpBufCur = p;
	((DWORD*)pd)[0] = ((DWORD*)p)[-2];
	((DWORD*)pd)[1] = ((DWORD*)p)[-1];
	return *this;
}

/////////////////////////////////////////////////////////////////////////////
// CMerlinWorld

// Removes dwUnk from the two tracked arrays (the removed slot is filled
// with the array's last element), deletes the object, and returns whether
// it was found in either array.

// Registers dwUnk in the second tracked-object array unless it is
// already listed (the pods and players call this when they create their
// renderer sprite).

// hover.exe:0x4128A0
// FUNCTION: HOVER 0x4128a0
int CMerlinWorld::Sub_4128A0(DWORD dwUnk)
{
	int nSize = m_arrUnk28.GetSize();
	int i;

	for (i = 0; i < nSize; i++)
	{
		if (m_arrUnk28[i] == (CObject*)dwUnk)
			return 0;
	}
	m_arrUnk28.SetAtGrow(nSize, (CObject*)dwUnk);
	return 1;
}

// hover.exe:0x413DB0
// FUNCTION: HOVER 0x413db0
int CMerlinWorld::Sub_413DB0(DWORD dwUnk)
{
	int nIdx;
	int nCount;
	int bFound;
	int i;

	bFound = 0;
	nCount = m_arrUnk28.GetSize();
	for (i = 0; i < nCount; i++)
	{
		if (m_arrUnk28[i] == (CObject*)dwUnk)
			goto Found28;
	}
	i = -1;
Found28:
	nIdx = i;
	if (nIdx < 0)
	{
		nCount = m_arrUnk14.GetSize();
		for (i = 0; i < nCount; i++)
		{
			if (m_arrUnk14[i] == (CObject*)dwUnk)
				goto Found14;
		}
		i = -1;
Found14:
		nIdx = i;
		if (nIdx >= 0)
		{
			m_arrUnk14[nIdx] = m_arrUnk14[nCount - 1];
			m_arrUnk14.RemoveAt(nCount - 1, 1);
			bFound = 1;
		}
	}
	else
	{
		m_arrUnk28[nIdx] = m_arrUnk28[nCount - 1];
		m_arrUnk28.RemoveAt(nCount - 1, 1);
		bFound = 1;
	}
	if (bFound != 0 && dwUnk != 0)
		delete (CGameObj*)dwUnk;
	return (nIdx >= 0) ? 1 : 0;
}

// Removes dwUnk from the m_arrUnk28 object array (swapping the last
// element into its slot) and deletes the object either way.
// hover.exe:0x414300
// FUNCTION: HOVER 0x414300
int CMerlinWorld::Sub_414300(DWORD dwUnk)
{
	int nIdx;
	int nCount;
	int i;

	nCount = m_arrUnk28.GetSize();
	for (i = 0; i < nCount; i++)
	{
		if (m_arrUnk28[i] == (CObject*)dwUnk)
			goto Found;
	}
	i = -1;
Found:
	nIdx = i;
	if (nIdx >= 0)
	{
		m_arrUnk28[nIdx] = m_arrUnk28[nCount - 1];
		m_arrUnk28.RemoveAt(nCount - 1, 1);
	}
	if (dwUnk != 0)
		delete (CGameObj*)dwUnk;
	return (nIdx >= 0) ? 1 : 0;
}

/////////////////////////////////////////////////////////////////////////////
// CSub40B800

// hover.exe:0x415380 (the coverage table lists this function as 0x415386,
// the address after the 6-byte "mov eax,fs:0" SEH prefix)
// FUNCTION: HOVER 0x415380
CSub40B800::CSub40B800(const char* pszName, DWORD dwUnk)
{
	m_pUnk08 = NULL;
	g_arrUnk_4A47F0.SetAtGrow(g_arrUnk_4A47F0.GetSize(), this);
	m_dwUnk18 = dwUnk;
	m_pUnk04 = NULL;
	if (pszName != NULL)
		Sub_417860(pszName);
	m_strUnk0C = pszName;
	g_dwUnk_4C5700++;
}

// Loads the slot's wave file. Without the WaveMix engine (g_dwUnk_4C56D8)
// the raw file is read into m_pUnk04; with it, m_pUnk08 receives an open
// MIXWAVE handle (unless m_dwUnk18 defers the open to Sub_40B800).

// hover.exe:0x417860
// FUNCTION: HOVER 0x417860
int CSub40B800::Sub_417860(const char* pszName)
{
	char szFile[0x104];

	if (g_dwUnk_4C56D8 == 0)
	{
		DWORD cbData;

		// hover.exe spells the guarded block out rather than using MFC's
		// TRY/CATCH: there is no AFX_EXCEPTION_LINK construction in front
		// of it, the handler catches CException* and does its own
		// IsKindOf test (hover.exe:0x41795D), and the mismatch arm is a
		// bare rethrow (0x4179EC).
		wsprintf(szFile, "%sSounds\\Unmixed\\%s.WAV", (LPCTSTR)theApp.m_strUnk0414,
			pszName);
		try
		{
			CFile file(szFile, CFile::modeRead | CFile::typeBinary);
			cbData = file.GetLength();
			m_pUnk04 = ::operator new(cbData);
			file.Seek(0, CFile::begin);
			file.Read(m_pUnk04, cbData);
			file.Close();
		}
		catch (CException* e)
		{
			if (e->IsKindOf(RUNTIME_CLASS(CFileException)))
			{
				CFileException* pe = (CFileException*)e;

				if (m_pUnk04 != NULL)
					delete (BYTE*)m_pUnk04;
				m_pUnk04 = NULL;
				e->Delete();
				return 0;
			}
			throw;
		}
		return 1;
	}
	if (g_pUnk_4C56DC == NULL)
		return 0;
	m_pUnk08 = NULL;
	if (m_dwUnk18 != 0)
	{
		m_strUnk0C = pszName;
		return 1;
	}
	wsprintf(szFile, "%sSounds\\Mixed\\%s.WAV", (LPCTSTR)theApp.m_strUnk0414, pszName);
	m_pUnk08 = Hover_sub_43B150(g_pUnk_4C56DC, szFile,
		AfxGetInstanceHandle(), 1);
	if (m_pUnk08 == NULL)
		return 0;
	return 1;
}

// Plays the slot's sound on channel dwA with flags dwB and loop count
// (WORD)dwC. Without the WaveMix engine (g_dwUnk_4C56D8) the slot object is
// queued to the sound worker thread (looping when dwC == 0xFFFF); with it,
// the slot's wave file is opened on first use (registered in the
// g_mapUnk_4A4820 handle map) and played through WaveMixPlay, showing the
// "wave devices in use" message box at most once per failure streak.

// hover.exe:0x40B800
// FUNCTION: HOVER 0x40b800
int CSub40B800::Sub_40B800(DWORD dwA, DWORD dwB, DWORD dwC)
{
	MIXPLAYPARAM playParams;
	char szFile[0x104];
	int nResult;

	if (g_dwUnk_4C56D8 == 0)
	{
		if (g_dwUnk_4C56D4 == 0)
			return 1;
		if (dwC == 0xFFFF)
		{
			EnterCriticalSection(&g_csUnk_4A4808);
			g_pvUnk_4C56F0 = this;
			SetEvent(g_hUnk_4C56F4);
			LeaveCriticalSection(&g_csUnk_4A4808);
			return 0;
		}
		EnterCriticalSection(&g_csUnk_4A4808);
		g_pvUnk_4C56EC = this;
		SetEvent(g_hUnk_4C56F4);
		LeaveCriticalSection(&g_csUnk_4A4808);
		return 0;
	}
	playParams.pSession = g_pUnk_4C56DC;
	playParams.nChannel = dwA;
	playParams.wLoops = (WORD)dwC;
	playParams.dwFlags = dwB;
	playParams.hwndNotify = NULL;
	playParams.wSize = sizeof(playParams);
	if (playParams.pSession == NULL)
		return 0;
	if (m_dwUnk18 == 0)
	{
		playParams.pMixWave = m_pUnk08;
		if (playParams.pMixWave == NULL)
			return 1;
	}
	else
	{
		strcpy(szFile, m_strUnk0C);
		strcat(szFile, ".wav");
		m_pUnk08 = Hover_sub_43B150(g_pUnk_4C56DC, szFile,
			AfxGetInstanceHandle(), 1);
		playParams.pMixWave = m_pUnk08;
		if (playParams.pMixWave == NULL)
			return 1;
		if (AfxGetApp()->m_pMainWnd == NULL)
			playParams.hwndNotify = NULL;
		else
			playParams.hwndNotify = AfxGetApp()->m_pMainWnd->m_hWnd;
		g_mapUnk_4A4820[playParams.pMixWave] = this;
	}
	nResult = Hover_sub_439E80(&playParams);
	if (nResult != 0)
	{
		if (g_dwUnk_4C56FC == 0)
		{
			g_dwUnk_4C56FC = 1;
			AfxMessageBox(IDS_WAVE_BUSY, MB_OK, (UINT)-1);
		}
	}
	else
	{
		g_dwUnk_4C56FC = 0;
	}
	if (nResult != 0 && m_dwUnk18 != 0)
		g_mapUnk_4A4820.RemoveKey(playParams.pMixWave);
	return nResult;
}

// hover.exe:0x4118A0
// FUNCTION: HOVER 0x4118a0
int CSub40B800::Sub_4118A0(DWORD dwA, DWORD dwB)
{
	if (g_dwUnk_4C56D8 == 0)
	{
		if (g_pThreadUnk_4C56E0 != NULL)
		{
			EnterCriticalSection(&g_csUnk_4A4808);
			g_pvUnk_4C56EC = NULL;
			g_pvUnk_4C56F0 = NULL;
			LeaveCriticalSection(&g_csUnk_4A4808);
			SetEvent(g_hUnk_4C56F4);
		}
		return 0;
	}
	if (g_pUnk_4C56DC == NULL)
		return 0;
	return Hover_sub_43A5A0(g_pUnk_4C56DC, dwA, dwB);
}

/////////////////////////////////////////////////////////////////////////////
// CCollider

// hover.exe:0x40B510
// FUNCTION: HOVER 0x40b510
int CCollider::Sub_40B510(CPlayer* pPlayer)
{
	pPlayer->Sub_40A9B0();
	// Virtual call at vtable slot 0x30 in the original
	// (hover.exe:0x408820 on CPlayer), passing this object.
	pPlayer->Sub_408820((DWORD)this);
	return 1;
}

// Intersect this segment with pLine and write the crossing point into
// pdwOut[0..1]. With this segment as P + t*r and pLine as Q + u*s, the
// two numerators and the denominator below are (Q-P)xs, (Q-P)xr and rxs,
// so the crossing point is P + (lNumT / lDenom) * r; both parameters are
// range-checked against the denominator instead of being divided, which
// is why the sign of the denominator splits the routine in two. The
// return value doubles as the side of the crossing: 0 when the segments
// do not cross, 1 when pLine crosses from the front and 0xFFFF when it
// crosses from the back. Both branches share the sign test on lNumU, so
// the side is really decided by the denominator (r x s): the positive
// branch can only reach the tail with lNumU >= 0 and the negative one
// with lNumU <= 0, the lNumU == 0 edge (pLine passing exactly through
// this segment's start point) falling to the front. The crossing point
// is written before the range checks, so the caller sees the infinite-
// line intersection even when the result is 0. The maze objects
// (CMerlinStatic) reach this through the CMerlinLine part of their layout,
// which is how the span setup Hover_sub_4045E0 uses it.
//
// Note that the two coordinates are computed with a rounded 64-bit
// MulDiv (HOVER_MULDIVP/HOVER_MULDIVN, one per sign of the denominator)
// but the parameter tests use the plain 32-bit cross products, which
// overflow silently for long segments - reproduced here as the original
// had it. `this` is copied into a local pointer because the original
// kept it in a register: VC 2.2 spills the incoming `this` here and
// then CSEs m_dwX1 into a second stack slot, which grows the frame and
// shifts every local (see rule 17 in notes/perfect-match.md).
// hover.exe:0x401260
// FUNCTION: HOVER 0x401260
WORD CMerlinLine::Sub_401260(CMerlinLine* pLine, DWORD* pdwOut)
{
	CMerlinLine* pThis;
	long lDX;
	long lDY;
	long lNumT;
	long lNumU;
	long lDenom;
	long lHalf;
	WORD wRet;

	pThis = this;
	lDX = (long)pThis->m_dwX1 - (long)pLine->m_dwX1;
	lDY = (long)pThis->m_dwY1 - (long)pLine->m_dwY1;
	lNumT = lDY * (long)pLine->m_dwDeltaX - lDX * (long)pLine->m_dwDeltaY;
	lDenom = (long)pThis->m_dwDeltaX * (long)pLine->m_dwDeltaY -
		(long)pLine->m_dwDeltaX * (long)pThis->m_dwDeltaY;
	lNumU = lDY * (long)pThis->m_dwDeltaX - lDX * (long)pThis->m_dwDeltaY;
	lHalf = lDenom / 2;

	if (lDenom > 0)
	{
		HOVER_MULDIVP(pdwOut[0], pThis->m_dwX1, lNumT, pThis->m_dwDeltaX,
			lHalf, lDenom)
		HOVER_MULDIVP(pdwOut[1], pThis->m_dwY1, lNumT, pThis->m_dwDeltaY,
			lHalf, lDenom)

		if (lNumT < 0 || lDenom < lNumT)
			return 0;
		if (lNumU < 0 || lDenom < lNumU)
			return 0;
	}
	else
	{
		if (lDenom == 0)
			return 0;

		HOVER_MULDIVN(pdwOut[0], pThis->m_dwX1, lNumT, pThis->m_dwDeltaX,
			lHalf, lDenom)
		HOVER_MULDIVN(pdwOut[1], pThis->m_dwY1, lNumT, pThis->m_dwDeltaY,
			lHalf, lDenom)

		if (lNumT > 0 || lDenom > lNumT)
			return 0;
		if (lNumU > 0 || lDenom > lNumU)
			return 0;
	}

	wRet = 0xFFFF;
	if (lNumU >= 0)
		wRet = 1;
	return wRet;
}

// WaveMixFreeWave: closes the MIXWAVE opened by Sub_417860. Declared here
// rather than in hover.h (see notes/perfect-match.md rule 20).
int __stdcall Hover_sub_43A860(SOUNDENGINE* pEngine, MIXWAVE* pWave);

// Tears the slot down: when a raw buffer was loaded (no WaveMix engine) the
// sound worker thread is first made to drop the slot - under
// g_csUnk_4A4808 both queue slots that still point at us are cleared and,
// if either did, we wait on g_hUnk_4C56F8 for the thread to acknowledge the
// kick before the buffer goes away. With the engine up the MIXWAVE handle
// is released instead. Finally the slot is unregistered from
// g_arrUnk_4A47F0 (swap with the last entry, then RemoveAt the tail).
//
// EH layout: the frame is the usual /GX one (push -1 / push 0x422E1D /
// push fs:[0]); state 1 covers m_strUnk0C, and the two tail "call"s at
// 0x422DF1/0x422DFD are the compiler's unwind funclets for m_strUnk0C
// (0x422E12 -> CString::~CString) and the CObject base (0x422E27 ->
// 0x401000) reused as the normal destruction path.

// hover.exe:0x422CB0 (the coverage table lists this function as 0x422CB6,
// the address after the 6-byte "mov eax,fs:0" SEH prefix)
// FUNCTION: HOVER 0x422cb0
CSub40B800::~CSub40B800()
{
	SOUNDENGINE* pEngine;
	int i;
	int bKicked = 0;

	if (m_pUnk04 != NULL)
	{
		if (g_dwUnk_4C56D8 == 0 && g_pThreadUnk_4C56E0 != NULL)
		{
			EnterCriticalSection(&g_csUnk_4A4808);
			if (g_pvUnk_4C56EC == this)
			{
				g_pvUnk_4C56EC = NULL;
				bKicked = 1;
			}
			if (g_pvUnk_4C56F0 == this)
			{
				g_pvUnk_4C56F0 = NULL;
				bKicked = 1;
			}
			LeaveCriticalSection(&g_csUnk_4A4808);
			ResetEvent(g_hUnk_4C56F8);
			SetEvent(g_hUnk_4C56F4);
			if (bKicked)
				WaitForSingleObject(g_hUnk_4C56F8, INFINITE);
		}
		::operator delete(m_pUnk04);
	}
	pEngine = g_pUnk_4C56DC;
	if (pEngine != NULL && g_dwUnk_4C56D0 != 0 && m_pUnk08 != NULL)
		Hover_sub_43A860(pEngine, (MIXWAVE*)m_pUnk08);
	for (i = 0; i < g_arrUnk_4A47F0.GetSize(); i++)
	{
		if (g_arrUnk_4A47F0.GetAt(i) == this)
		{
			g_arrUnk_4A47F0[i] =
				g_arrUnk_4A47F0[g_arrUnk_4A47F0.GetSize() - 1];
			g_arrUnk_4A47F0.RemoveAt(g_arrUnk_4A47F0.GetSize() - 1);
		}
	}
	g_dwUnk_4C5700--;
}
