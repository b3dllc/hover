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
// (not static: a static never reaches the map file, so no tool could
// match it)
// hover.exe:0x4A45A0
// GLOBAL: HOVER 0x4a45a0
DWORD g_dwGameObjects;

// hover.exe:0x4C52D8
// GLOBAL: HOVER 0x4c52d8
CGameObj* g_pDirtyObjList;			// CGameObj instance list head (QueueBeginFrame)

// hover.exe:0x4C52DC
// GLOBAL: HOVER 0x4c52dc
CGameObj* g_pDrawObjList;			// CGameObj secondary list head

// hover.exe:0x4C54A0
// GLOBAL: HOVER 0x4c54a0
DWORD g_dwUnk_4C54A0 = 0xFFFFFFFF;	// last player sound state
								// (CHumanPlayer::Think)

/////////////////////////////////////////////////////////////////////////////

// hover.exe:0x4A4820
// GLOBAL: HOVER 0x4a4820
CMapPtrToPtr g_mapOpenWaveToSlot;	// MIXWAVE handle -> CSoundSlot slot

// hover.exe:0x4A47F0
// GLOBAL: HOVER 0x4a47f0
CObArray g_arrUnk_4A47F0;	// sound slot registry
								// (CSoundSlot::CSoundSlot)

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
// CSegment

// hover.exe:0x40D320
// FUNCTION: HOVER 0x40d320
CSegment::CSegment()
{
	m_dwX1 = 0;
	m_dwX2 = 0;
	m_dwY1 = 0;
	m_dwY2 = 0;
	Recalc();
}

// hover.exe:0x42C820
// FUNCTION: HOVER 0x42c820
void CSegment::Recalc()
{
	DWORD dwDy;
	DWORD dwDx;

	m_dwDeltaX = m_dwX2 - m_dwX1;
	m_dwDeltaY = m_dwY2 - m_dwY1;
	m_dwDistSq = m_dwDeltaY * m_dwDeltaY + m_dwDeltaX * m_dwDeltaX;
	// 16.16 fixed-point components (low word zeroed, 16-bit delta in the
	// high word); the angle comes back in the high word of dwDx
	((WORD*)&dwDy)[1] = (WORD)(m_dwY2 - m_dwY1);
	((WORD*)&dwDy)[0] = 0;
	((WORD*)&dwDx)[0] = 0;
	((WORD*)&dwDx)[1] = (WORD)(m_dwX2 - m_dwX1);
	m_dwAngle = ((short*)Hover_FixAngleP(&dwDx, dwDy, dwDx))[1];
}

// Distance of the point pPt from the segment, plus three flags:
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
int CSegment::DistanceTo(const POINT* pPt, DWORD* pdwA, DWORD* pdwB,
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

	lX = pPt->x;
	lY = pPt->y;
	if (m_dwDistSq == 0)
	{
		// Degenerate segment: the distance to the single endpoint.
		lX -= (long)m_dwX1;
		if (lX < 0)
			lX = -lX;
		lY -= (long)m_dwY1;
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

	lDY = lY - (long)m_dwY1;
	lDX = lX - (long)m_dwX1;
	lDot = lDY * (long)m_dwDeltaY + lDX * (long)m_dwDeltaX;
	lCross = lDX * (long)m_dwDeltaY + ((long)m_dwY1 - lY) * (long)m_dwDeltaX;
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
	// The volatile read stops VC 2.2 CSE-ing the two m_dwDistSq loads into
	// a stack slot; the original reads the member both times
	// (notes/perfect-match.md rule 26).
	if (lDot > (long)*(volatile DWORD*)&m_dwDistSq)
	{
		// Past the second endpoint. Note the original leaves *pdwC zero
		// here, so the two endpoint cases are told apart by *pdwC alone.
		lX -= (long)m_dwX2;
		if (lX < 0)
			lX = -lX;
		lY -= (long)m_dwY2;
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
	lLen = (long)sqrt((double)(long)m_dwDistSq);
	return (short)abs((short)(lCross / lLen));
}

/////////////////////////////////////////////////////////////////////////////
// CGameObj

// hover.exe:0x40D150
// FUNCTION: HOVER 0x40d150
CGameObj::CGameObj()
{
	m_wXFrac = 0;
	m_dwFrameX = 0;
	m_wX = 0;
	m_dwFrameY = 0;
	m_wYFrac = 0;
	m_dwPrevX = 0;
	m_wY = 0;
	m_dwPrevY = 0;
	m_bHideOnMap = 0;
	m_dwUnk24 = 0;
	m_pRegionNodes = 0;
	m_bKilled = 0;
	m_bDead = 0;
	m_pMerlin = 0;
	m_dwUnk80 = 0;
	m_dwUnk7C = 0;
	m_pNextStart = 0;
	m_pNextEnd = 0;
	m_bOnStartList = 0;
	m_bOnEndList = 0;
	m_dwUnk64 = 0;
	m_dwLastSoundTick = GetTickCount();
	g_dwGameObjects++;
}

// hover.exe:0x40D220
// FUNCTION: HOVER 0x40d220
CGameObj::~CGameObj()
{
	if (m_pMerlin != 0)
	{
		(theApp.m_pBumperDoc->m_pWorld)->RemoveObject(m_pMerlin);
		m_pMerlin = 0;
	}
	if (m_pRegionNodes != 0)
	{
		// Unlink every node of the m_pRegionNodes chain (linked in their
		// cells through m_pNext/m_pPrev, chained through m_pNextObj) and push
		// it onto the g_pRegionNodeFree free list.
		REGIONNODE* pNode = m_pRegionNodes;
		REGIONNODE* pFree = g_pRegionNodeFree;
		do
		{
			REGIONNODE* pPrev = pNode->m_pPrev;
			REGIONNODE* pNext = pNode->m_pNext;
			pPrev->m_pNext = pNext;
			if (pNext != NULL)
				pNext->m_pPrev = pPrev;
			pNode->m_pNext = pFree;
			g_pRegionNodeFree = pNode;
			pFree = pNode;
			pNode = pNode->m_pNextObj;
		} while (pNode != NULL);
		m_pRegionNodes = 0;
	}
	g_dwGameObjects--;
}

// hover.exe:0x414A50
// FUNCTION: HOVER 0x414a50
int CGameObj::SetMerlin(CMerlinLine* pMerlin)
{
	m_pMerlin = pMerlin;
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
	m_pMerlin = 0;
	return 1;
}

// hover.exe:0x42AD30
// FUNCTION: HOVER 0x42ad30
void CGameObj::Think()
{
	// empty default implementation in the original
}

// hover.exe:0x42AD40
// FUNCTION: HOVER 0x42ad40
void CGameObj::OnTouch(CPlayer* pOther, DWORD dwB)
{
	// empty default implementation in the original
}

// hover.exe:0x42AD50
// FUNCTION: HOVER 0x42ad50
void CGameObj::UsePod(DWORD dwUnk)
{
	// empty default implementation in the original
}

// hover.exe:0x42AD70
// FUNCTION: HOVER 0x42ad70
int CGameObj::IsNear(POINT pt, long lRadius)
{
	return 0;
}

// hover.exe:0x42AD60
// FUNCTION: HOVER 0x42ad60
int CGameObj::Touches(DWORD dwUnk)
{
	return 0;
}

// Virtual functions beyond slot 11.

// Base virtual body for vtable slot 12. The original CGameObj vtable points
// to hover.exe:0x42AD80 (a one-instruction "return 1"); CPlayer overrides
// this slot with the real implementation at hover.exe:0x408820 below.
// FUNCTION: HOVER 0x42ad80
int CGameObj::UpdateRegions(DWORD dwA)
{
	return 1;
}

// hover.exe:0x411570
// FUNCTION: HOVER 0x411570
int CGameObj::IntersectLine(DWORD dwA, DWORD dwB, DWORD dwC, DWORD* pdwOut)
{
	*pdwOut = 0;
	return 0;
}

// hover.exe:0x42AD90
// FUNCTION: HOVER 0x42ad90
int CGameObj::OnProbeHit(CGameObj* pObj)
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
	Recalc();
}

// hover.exe:0x40F760
// FUNCTION: HOVER 0x40f760
CMerlinLine::CMerlinLine(short sX1, short sY1, short sX2, short sY2)
{
	m_dwX1 = (long)sX1;
	m_dwY1 = (long)sY1;
	m_dwX2 = (long)sX2;
	m_dwY2 = (long)sY2;
	Recalc();
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
// are recomputed with Recalc afterwards.

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
		Recalc();
	}
}

// Recompute the bounding box, deltas, dist-sq and flags from the current
// endpoints. Faithful reconstruction of hover.exe:0x4010C0.
// hover.exe:0x4010C0
// FUNCTION: HOVER 0x4010c0
void CMerlinLine::Recalc()
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
void CGameObj::BeginFrame()
{
	m_dwFrameX = (DWORD)(short)m_wX;
	m_dwFrameY = (DWORD)(short)m_wY;
	m_bDead = m_bKilled;
}

// hover.exe:0x4087F0
// FUNCTION: HOVER 0x4087f0
void CGameObj::EndFrame()
{
	m_dwPrevX = m_dwFrameX;
	m_dwPrevY = m_dwFrameY;
	if (m_bDead != 0 && this != NULL)
	{
		// deleting destructor (vtable slot 1)
		delete this;
	}
}

// hover.exe:0x413020
// FUNCTION: HOVER 0x413020
void CGameObj::KillObject()
{
	m_bKilled = 1;
	if (m_bOnStartList == 0)
	{
		m_bOnStartList = 1;
		m_pNextStart = g_pDirtyObjList;
		g_pDirtyObjList = this;
	}
	theApp.m_pBumperDoc->RemoveObject(this);
}

// Links the object into the g_pDirtyObjList instance list (via m_dwBottomDZ).

// hover.exe:0x408530
// FUNCTION: HOVER 0x408530
void CGameObj::QueueBeginFrame()
{
	if (m_bOnStartList == 0)
	{
		m_bOnStartList = 1;
		m_pNextStart = g_pDirtyObjList;
		g_pDirtyObjList = this;
	}
}

// hover.exe:0x403880
// FUNCTION: HOVER 0x403880
DWORD* CGameObj::GetCellPos(DWORD* pdwOut)
{
	pdwOut[0] = (DWORD)(short)m_wX;
	pdwOut[1] = (DWORD)(short)m_wY;
	return pdwOut;
}

// hover.exe:0x4038A0
// FUNCTION: HOVER 0x4038a0
DWORD* CGameObj::GetFixedPos(DWORD* pdwOut)
{
	pdwOut[0] = m_fxX;
	pdwOut[1] = m_fxY;
	return pdwOut;
}

// hover.exe:0x406C40
// FUNCTION: HOVER 0x406c40
DWORD* CGameObj::GetPrevPos(DWORD* pdwOut)
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
DWORD* CGameObj::GetFramePos(DWORD* pdwOut)
{
	pdwOut[0] = m_dwFrameX;
	pdwOut[1] = m_dwFrameY;
	return pdwOut;
}

// hover.exe:0x40F880
// FUNCTION: HOVER 0x40f880
void CGameObj::SetCellPos(DWORD dwX, DWORD dwY)
{
	m_wX = (WORD)dwX;
	m_wXFrac = 0;
	m_wY = (WORD)dwY;
	m_wYFrac = 0;
}

// hover.exe:0x40A9B0
// FUNCTION: HOVER 0x40a9b0
void CGameObj::ReleaseRegionNodes()
{
	if (m_pRegionNodes != 0)
	{
		// Unlink every node of the m_pRegionNodes chain (same linkage as in
		// ~CGameObj) and push it onto the g_pRegionNodeFree free list.
		REGIONNODE* pNode = m_pRegionNodes;
		REGIONNODE* pFree = g_pRegionNodeFree;
		do
		{
			REGIONNODE* pPrev = pNode->m_pPrev;
			REGIONNODE* pNext = pNode->m_pNext;
			pPrev->m_pNext = pNext;
			if (pNext != NULL)
				pNext->m_pPrev = pPrev;
			pNode->m_pNext = pFree;
			g_pRegionNodeFree = pNode;
			pFree = pNode;
			pNode = pNode->m_pNextObj;
		} while (pNode != NULL);
		m_pRegionNodes = 0;
	}
}

// Plays the sound in CBumperDoc slot dwA, but at most once every 500 ms.

// hover.exe:0x40D6E0
// FUNCTION: HOVER 0x40d6e0
void CGameObj::PlayObjSound(DWORD dwA, DWORD dwB, DWORD dwC, DWORD dwD)
{
	if (m_dwLastSoundTick < GetTickCount() - 500)
		((CSoundSlot*)theApp.m_pBumperDoc->m_adwSounds[dwA])->PlayWave(
			dwB, dwC, dwD);
	m_dwLastSoundTick = GetTickCount();
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
	m_wUnk0090 = 0;
	m_wUnk0092 = 0;
	m_wUnk0094 = 0;
	m_wUnk0096 = 0;
	m_wHeightStepFrac = 0;
	m_wHeightStep = 0;
	m_wHeightRateFrac = 0;
	m_wHeightRate = 0;
	m_wTargetHeightFrac = 0;
	m_wTargetHeight = 0;
	m_wHeightFrac = 0;
	m_wHeight = 0;
	m_wTurnRateFrac = 0;
	m_wTurnRate = 0;
	m_wTurnImpulseFrac = 0;
	m_wTurnImpulse = 0;
	m_wHeadingFrac = 0;
	m_wHeading = 0;
	m_wVelXFrac = 0;
	m_wVelX = 0;
	m_wVelYFrac = 0;
	m_wVelY = 0;
	m_wAccelXFrac = 0;
	m_wAccelX = 0;
	m_wAccelYFrac = 0;
	m_wAccelY = 0;
	m_wUnk01BC = 0;
	m_wUnk01BE = 0;
	m_wUnk01C0 = 0;
	m_wUnk01C2 = 0;
	m_wThrustAccelFrac = 0;
	m_wThrustAccel = 0;
	m_wMaxSpeedFrac = 0;
	m_wMaxSpeed = 0;
	m_wBaseMaxSpeedFrac = 0;
	m_wBaseMaxSpeed = 0;
	m_wTurnInputFrac = 0;
	m_bHuman = 0;
	m_wTurnInput = 0;
	m_wThrustInputFrac = 0;
	m_wThrustInput = 0;
}

// The class has both an out-of-line ??1 (hover.exe:0x421A90, called by
// ~CHumanPlayer for the player CBumperDoc embeds and by ~CRobotPlayer) and a
// scalar deleting ??_G (hover.exe:0x42C080); rule 69 says to mark the ??1,
// which is what this definition compiles to.
// hover.exe:0x421A90
// FUNCTION: HOVER 0x421a90
CPlayer::~CPlayer()
{
	ClearPods(1);
}

// Deletes every object in m_listFlags and empties the list; when bAll is
// nonzero the other three object lists are emptied the same way.

// hover.exe:0x414BD0
// FUNCTION: HOVER 0x414bd0
void CPlayer::ClearPods(int bAll)
{
	POSITION pos = m_listFlags.GetHeadPosition();
	while (pos != NULL)
		delete m_listFlags.GetNext(pos);
	m_listFlags.RemoveAll();
	if (bAll != 0)
	{
		pos = m_listTempWallPods.GetHeadPosition();
		while (pos != NULL)
			delete m_listTempWallPods.GetNext(pos);
		m_listTempWallPods.RemoveAll();
		pos = m_listSmokeBombPods.GetHeadPosition();
		while (pos != NULL)
			delete m_listSmokeBombPods.GetNext(pos);
		m_listSmokeBombPods.RemoveAll();
		pos = m_listJumpPods.GetHeadPosition();
		while (pos != NULL)
			delete m_listJumpPods.GetNext(pos);
		m_listJumpPods.RemoveAll();
	}
	m_pInvinciblePod = 0;
	m_pSpeedPod = 0;
	m_pSlowPod = 0;
	m_pTempWallPod = 0;
	m_pSmokeBombPod = 0;
	m_pJumpPod = 0;
}

// Points the player at a parameter block (theApp.m_pHumanParams for the
// human player, theApp.m_pRobotParams for the robots) and refreshes the
// three tuning values cached at +0x1D0. A NULL pParams keeps the current block, which is
// how the level loader re-reads the values after a skill change.

// hover.exe:0x413ED0
// FUNCTION: HOVER 0x413ed0
void CPlayer::SetParams(CPlayerParams* pParams)
{
	DWORD* pdwParams;
	DWORD dwP0;
	DWORD dwP8;

	if (pParams != NULL)
		m_pParams = pParams;
	// 32-bit copies, as in PlacePlayer: params[0] -> +0x1D0 and
	// params[2] -> +0x1D4 and +0x1D8
	pdwParams = m_pParams->m_adw;
	dwP0 = pdwParams[0];
	dwP8 = pdwParams[2];
	m_fxThrustAccel = dwP0;
	m_fxMaxSpeed = dwP8;
	m_fxBaseMaxSpeed = dwP8;
}

// hover.exe:0x409110
// FUNCTION: HOVER 0x409110
void CPlayer::Think()
{
	if (m_dwSledTimer == 0xFFFFFFFF)
		ApplyControls();
	if (m_bKilled == 0 && theApp.m_dwGameState == 3)
	{
		UpdateHeight();
		UpdateHoldPad();
		UpdateSled();
		MovePlayer();
		m_bOnHoldPad = 0;
		m_bOnFlagRemover = 0;
		QueueBeginFrame();
	}
	CGameObj::Think();
}

// hover.exe:0x40C220
// FUNCTION: HOVER 0x40c220
void CPlayer::OnTouch(CPlayer* pOther, DWORD dwB)
{
	CBumperView* pView;

	if (dwB != 0)
	{
		m_bBounced = 0;
		pOther->OnTouch(this, 0);
	}
	if (pOther->IsKindOf(RUNTIME_CLASS(CPlayer)))
	{
		pView = Hover_GetView();
		pView->m_dwUnk0524 = 1;
		CollidePlayer((DWORD)pOther, dwB);
		if (theApp.m_bGameSounds != 0 &&
			pOther->IsKindOf(RUNTIME_CLASS(CHumanPlayer)))
			PlayObjSound(0x14, 3, 5, 0);
	}
}

// Proximity check against the two points at +0x180/+0x188 using the
// max + min/2 distance metric.

// hover.exe:0x42BFC0
// FUNCTION: HOVER 0x42bfc0
int CPlayer::IsNear(POINT pt, long lRadius)
{
	// The original walks the five values through one pointer
	// (`add ecx, 0x180` at hover.exe:0x42BFC1), not off `this`.
	PLAYERPROBE* pProbe = &m_probe;
	long lLimit = pProbe->lRadius + lRadius;
	long lX = labs(pProbe->ptFront.x - pt.x);
	long lY = labs(pProbe->ptFront.y - pt.y);
	long lMin = (lY < lX) ? lY : lX;

	if (lY - (lMin >> 1) + lX < lLimit)
		return 1;
	lX = labs(pProbe->ptBack.x - pt.x);
	lY = labs(pProbe->ptBack.y - pt.y);
	lMin = (lY < lX) ? lY : lX;
	return (lY - (lMin >> 1) + lX < lLimit);
}

// hover.exe:0x42BFA0
// FUNCTION: HOVER 0x42bfa0
int CPlayer::Touches(DWORD dwUnk)
{
	DWORD dwA;
	DWORD dwB;

	return TouchesPlayer((CPlayer*)dwUnk, &dwB, &dwA);
}

// Maps the player's two proximity points (+0x180/+0x188) plus the +0x190
// range limit onto the collider's region grid, clamps to the grid bounds,
// and visits every covered cell. When the collider's +0x44 flag is set the
// cell objects are checked against a registered-object array
// (CCollider::CollectCell); otherwise the player is added to the cell
// (CRegionMatrix::AddNode). Always returns 1.

// hover.exe:0x408820
// FUNCTION: HOVER 0x408820
int CPlayer::UpdateRegions(DWORD dwA)
{
	CCollider* pCollider;
	PLAYERPROBE* pProx;
	long nX1, nX2, nY1, nY2;
	long nMinX, nMaxX, nMinY, nMaxY;
	long nX, nY;

	pCollider = (CCollider*)dwA;

	// Virtual call at vtable slot 0x4C returns &m_probe.
	pProx = GetProbe();

	nX2 = pProx->ptBack.x;
	nX1 = pProx->ptFront.x;
	if (nX2 > nX1)
	{
		nMinX = (nX1 - pProx->lRadius) / 256;
		nMaxX = (nX2 + pProx->lRadius) / 256;
	}
	else
	{
		nMinX = (nX2 - pProx->lRadius) / 256;
		nMaxX = (nX1 + pProx->lRadius) / 256;
	}

	nY1 = pProx->ptFront.y;
	nY2 = pProx->ptBack.y;
	if (nY2 > nY1)
	{
		nMinY = (nY1 - pProx->lRadius) / 256;
		nMaxY = (nY2 + pProx->lRadius) / 256;
	}
	else
	{
		nMinY = (nY2 - pProx->lRadius) / 256;
		nMaxY = (nY1 + pProx->lRadius) / 256;
	}

	if ((long)pCollider->m_dwCols <= nMaxX)
		nMaxX = (long)pCollider->m_dwCols - 1;
	if ((long)pCollider->m_dwRows <= nMaxY)
		nMaxY = (long)pCollider->m_dwRows - 1;

	if (nMaxX < nMinX || nMaxY < nMinY)
		return 1;

	nX = nMinX;
	do
	{
		nY = nMinY;
		do
		{
			if (pCollider->m_bQuerying != 0)
				pCollider->CollectCell(nX, nY);
			else
				pCollider->m_grid.AddNode(nX, nY, this);
		} while (++nY <= nMaxY);
	} while (++nX <= nMaxX);
	return 1;
}

// Publishes the heading/frame state the renderer reads: the current
// heading word becomes the "previous frame" angle (+0x1A0/+0x1A8) and the
// sprite frame picked by the derived BeginFrame (+0x1E4) is latched into
// +0x1E8. Then runs the CGameObj version (position snapshot).

// hover.exe:0x408960
// FUNCTION: HOVER 0x408960
void CPlayer::BeginFrame()
{
	m_dwFrameHeading = (DWORD)(long)(short)m_wHeading;
	m_dwFrameAltitude = m_dwAltitude;
	m_pFrame = m_pHeadingFrame;
	CGameObj::BeginFrame();
}

// Second half of the per-frame publish: while the object is alive
// (m_dwUnk60 == 0) the latched heading pair is copied into +0x88/+0x8C
// and, when the player owns a sprite, into the sprite's +0x54/+0x56 pair
// (the second word is the first plus a fixed 0x40 span). Then runs the
// CGameObj version (which is what actually deletes a retired object).

// hover.exe:0x408990
// FUNCTION: HOVER 0x408990
void CPlayer::EndFrame()
{
	DWORD dwAngle;
	CMerlinDynamic* pSprite;

	if (m_bDead == 0)
	{
		pSprite = (CMerlinDynamic*)m_pMerlin;
		m_dwUnk0088 = m_dwFrameHeading;
		dwAngle = m_dwFrameAltitude;
		m_dwUnk008C = dwAngle;
		if (pSprite != NULL)
		{
			pSprite->m_wBottom = (WORD)dwAngle;
			pSprite->m_wTop = (WORD)dwAngle + 0x40;
		}
	}
	CGameObj::EndFrame();
}

// hover.exe:0x42C050
// FUNCTION: HOVER 0x42c050
void CPlayer::KillObject()
{
	CGameObj::KillObject();
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
PLAYERPROBE* CPlayer::GetProbe()
{
	return &m_probe;
}

// hover.exe:0x414A80
// FUNCTION: HOVER 0x414a80
void CPlayer::ResetMotion()
{
	DWORD dwZero;
	DWORD dwA0;
	DWORD dwP0;
	DWORD dwP8;

	// The original clears the word arrays with straight-line stores in
	// this exact (paired, descending) order.
	m_wAccelX = 0;
	m_wAccelXFrac = 0;
	m_wAccelY = 0;
	m_wAccelYFrac = 0;
	m_wUnk01BE = 0;
	m_wUnk01BC = 0;
	m_wUnk01C2 = 0;
	m_wUnk01C0 = 0;
	m_wVelX = 0;
	m_wVelXFrac = 0;
	m_wVelY = 0;
	m_wVelYFrac = 0;
	m_wTurnRate = 0;
	m_wTurnRateFrac = 0;
	m_dwFrameHeading = 0;
	m_wHeading = 0;
	m_dwFrameAltitude = 0;
	m_wHeadingFrac = 0;
	m_dwUnk0088 = 0;
	m_wTurnImpulse = 0;
	m_dwUnk008C = 0;
	m_wTurnImpulseFrac = 0;
	m_dwUnk01C4 = 0;
	((WORD*)&dwZero)[0] = 0;
	((WORD*)&dwZero)[1] = 0;
	m_bOnTop = 0;
	UpdateProbe(dwZero);
	m_bOnHoldPad = 0;
	m_bSteerAssist = 1;
	m_bFriction = 1;
	m_dwHoldTimer = 0xFFFFFFFF;
	m_bOnFlagRemover = 0;
	m_dwSledTimer = 0xFFFFFFFF;
	m_dwUnk0150 = 0xFFFFFFFF;
	m_listFlags.RemoveAll();
	m_wTargetHeightFrac = 0;
	m_bInvincible = 0;
	m_bGodMode = 0;
	m_wTargetHeight = 0x40;
	// The height snaps to the target, and the thrust and speed limits
	// reload from the params block
	dwA0 = m_fxTargetHeight;
	dwP0 = m_pParams->m_adw[0];
	dwP8 = m_pParams->m_adw[2];
	m_fxHeight = dwA0;
	m_fxThrustAccel = dwP0;
	m_fxMaxSpeed = dwP8;
	m_fxBaseMaxSpeed = dwP8;
}

// Recomputes the two proximity points at +0x180/+0x188 (50 fixed-point
// units ahead of and behind the position at +0x44/+0x48 along the movement
// angle m_wHeading, 0x200 per full circle) and the +0x190 range limit
// (dwUnk scaled by 1.5, at least 50).

// hover.exe:0x408550
// FUNCTION: HOVER 0x408550
void CPlayer::UpdateProbe(DWORD dwUnk)
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

	lAngle = (long)(short)m_wHeading;
	dwCos = g_adwSinTable[(((lAngle + 0x80) & 0x1FF) << 4) + 1];
	dwSin = g_adwSinTable[((lAngle & 0x1FF) << 4) + 1];
	HOVER_FIXMULWP(dwSin, 0x32);
	HOVER_FIXMULWP(dwCos, 0x32);
	dwX = m_fxX;
	dwY = m_fxY;
	dwFrontX = dwX + dwCos;
	dwFrontY = dwY + dwSin;
	m_probe.ptFront.x = ((short*)&dwFrontX)[1];
	m_probe.ptFront.y = ((short*)&dwFrontY)[1];
	dwBackX = dwX - dwCos;
	dwBackY = dwY - dwSin;
	m_probe.ptBack.x = ((short*)&dwBackX)[1];
	m_probe.ptBack.y = ((short*)&dwBackY)[1];
	dwRange = dwUnk;
	((WORD*)&dwScale)[0] = 0x8000;
	((WORD*)&dwScale)[1] = 1;
	HOVER_FIXMULP3(dwRange, dwScale);
	m_probe.lRadius = ((short*)&dwRange)[1];
	if ((long)m_probe.lRadius < 0x32)
		m_probe.lRadius = 0x32;
}

// Control integration: accelerates the turn rate (m_fxTurnRate) by the
// turn parameter (m_pParams->m_adw[10]) scaled by the turn input
// (m_fxTurnInput), clamped to the parameter in the input direction, then
// converts the thrust input (m_fxThrustInput) scaled by m_fxThrustAccel
// into the acceleration pair m_fxAccelX/m_fxAccelY along m_fxHeading.

// hover.exe:0x40A9F0
// FUNCTION: HOVER 0x40a9f0
void CPlayer::ApplyControls()
{
	DWORD dwAccel;
	DWORD dwLimit;
	DWORD dwTurn;
	DWORD dwAngle;
	DWORD dwVelX;
	DWORD dwVelY;
	DWORD* pdwParams;

	m_wTurnImpulse = 0;
	m_wTurnImpulseFrac = 0;
	pdwParams = &m_pParams->m_adw[10];
	dwAccel = *pdwParams;
	HOVER_FIXMULP(dwAccel, m_fxTurnInput);
	if (*(volatile long*)&m_fxTurnInput < 0)
	{
		dwLimit = *pdwParams;
		if (-(long)dwLimit > *(long*)&m_fxTurnRate ||
			-(long)dwLimit == *(long*)&m_fxTurnRate)
		{
			// already at or below the reverse limit
		}
		else
		{
			*(long*)&m_fxTurnRate += (long)dwAccel;
			if (-(long)dwLimit > *(long*)&m_fxTurnRate)
				*(long*)&m_fxTurnRate = -(long)dwLimit;
		}
	}
	else
	{
		dwLimit = *pdwParams;
		if ((long)dwLimit > *(long*)&m_fxTurnRate)
		{
			*(long*)&m_fxTurnRate += (long)dwAccel;
			if ((long)dwLimit < *(long*)&m_fxTurnRate ||
				(long)dwLimit == *(long*)&m_fxTurnRate)
				*(long*)&m_fxTurnRate = (long)dwLimit;
		}
	}
	dwTurn = m_fxThrustAccel;
	HOVER_FIXMULP(dwTurn, m_fxThrustInput);
	dwAngle = m_fxHeading;
	dwVelX = *(volatile DWORD*)&dwTurn;
	HOVER_FIXMULP(dwVelX,
		g_adwSinTable[((((long)dwAngle >> 10) + 0x2000) & 0x7FFC) / 4 + 1]);
	m_fxAccelX = dwVelX;
	dwVelY = *(volatile DWORD*)&dwTurn;
	HOVER_FIXMULP(dwVelY,
		g_adwSinTable[((*(volatile DWORD*)&dwAngle & 0x1FFF000) >> 10) / 4 + 1]);
	m_fxAccelY = dwVelY;
}

// Height integration: moves the hover height (m_fxHeight) toward the
// target height (m_fxTargetHeight), snapping when within 2.0 units.

// hover.exe:0x40AB70
// FUNCTION: HOVER 0x40ab70
void CPlayer::UpdateHeight()
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

	dwTarget = m_fxTargetHeight;
	dwCur = m_fxHeight;
	dwDelta = dwTarget - dwCur;
	if (dwCur == dwTarget)
	{
		m_bAirborne = 0;
	}
	else if ((long)dwDelta >= 0x20000 || (long)dwDelta < -0x20000 ||
		(long)dwDelta == -0x20000)
	{
		if ((long)dwDelta < 0)
		{
			dwVal = 0x240000;
			dwDiff = dwCur - dwTarget;
			HOVER_FIXDIVPP((DWORD*)&dwVal, dwDiff);
			m_fxHeightRate = dwVal;
			dwVal2 = dwVal - 0xC0000;
			m_fxHeightStep = dwVal2;
		}
		else
		{
			if (m_bAirborne != 0)
				m_fxHeightRate =
					(DWORD)((long)(dwTarget - dwCur) / 3);
			else
				m_fxHeightRate = dwTarget - dwCur;
			m_fxHeightStep = m_fxHeightRate;
		}
		m_fxHeight = m_fxHeightStep + dwCur;
	}
	else
	{
		if (m_bHuman != 0 && theApp.m_bGameSounds != 0 && (long)dwDelta < 0)
			PlayObjSound(0x16, 4, 5, 0);
		m_fxHeight = dwTarget;
	}
	m_bOnTop = 0;
	m_dwAltitude = (long)(short)m_wHeight;
}

// hover.exe:0x40ACE0
// FUNCTION: HOVER 0x40ace0
void CPlayer::UpdateHoldPad()
{
	if (m_bOnHoldPad == 0)
	{
		m_dwHoldTimer = 0xFFFFFFFF;
		if (m_bOnTop == 0 && m_bAirborne == 0)
		{
			m_wTargetHeight = 0x40;
			m_wTargetHeightFrac = 0;
		}
	}
	else
	{
		if (m_dwHoldTimer == 0x5A)
		{
			m_wTargetHeight = 0x10;
			m_wTargetHeightFrac = 0;
			if (m_bHuman != 0 && theApp.m_bGameSounds != 0)
				PlayObjSound(2, 0, 5, 0);
		}
		else if (m_dwHoldTimer == 1)
		{
			if (m_bHuman != 0 && theApp.m_bGameSounds != 0)
				PlayObjSound(3, 0, 5, 0);
			m_wTargetHeight = 0x40;
			m_wTargetHeightFrac = 0;
		}
		if (m_bFriction != 0 && (long)m_dwHoldTimer > 0)
		{
			m_dwHoldTimer--;
			m_wAccelXFrac = 0;
			m_wAccelX = 0;
			m_wAccelYFrac = 0;
			m_wAccelY = 0;
		}
	}
}

// Sled pad push while m_dwSledTimer counts down: scales m_fxBaseMaxSpeed
// into m_fxMaxSpeed, splits it along the sled's angle (+0x160) into the
// acceleration pair, and reverses or bleeds the turn rate depending on
// the current angle m_wHeading.

// hover.exe:0x40ADE0
// FUNCTION: HOVER 0x40ade0
void CPlayer::UpdateSled()
{
	DWORD dwThrust;
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
	if ((long)m_dwSledTimer > 0)
	{
		if (m_dwSledTimer == 0x1E && m_bHuman != 0 && theApp.m_bGameSounds != 0)
			PlayObjSound(0, 0, 5, 0);
		lCount = *(volatile long*)&m_dwSledTimer;
		// Rule 222: every arm computes into the *shared* dwThrust, which is
		// what lets hover.exe:0x40AE3C jump clear over the sibling arms to
		// the one store at 0x40AEEC.
		if (lCount == 1)
		{
			if (m_pSpeedPod == 0)
			{
				if (m_pSlowPod == 0)
					dwThrust = m_fxBaseMaxSpeed;
				else
				{
					dwThrust = m_fxBaseMaxSpeed;
					HOVER_FIXMULP3(dwThrust, m_pParams->m_adw[7]);
				}
			}
			else
			{
				DWORD dwThrustB;

				dwThrustB = m_fxBaseMaxSpeed;
				HOVER_FIXMULP3(dwThrustB, m_pParams->m_adw[8]);
				dwThrust = dwThrustB;
			}
		}
		else
		{
			DWORD dwThrustC;

			dwThrustC = m_fxBaseMaxSpeed;
			HOVER_FIXMULP3(dwThrustC, m_pParams->m_adw[8]);
			dwThrust = dwThrustC;
		}
		m_fxMaxSpeed = dwThrust;
		if (lCount <= 0)
			return;
		lDelta = m_dwSledAngle;
		dwCos = g_adwSinTable[(((lDelta + 0x80) & 0x1FF) << 4) + 1];
		HOVER_FIXMULWP(dwCos, 4);
		m_fxAccelX = dwCos;
		dwSin = g_adwSinTable[((lDelta & 0x1FF) << 4) + 1];
		HOVER_FIXMULWP(dwSin, 4);
		m_fxAccelY = dwSin;
		wAngle = m_wHeading;
		pdwParams = &m_pParams->m_adw[10];
		dwMargin1 = *pdwParams;
		HOVER_FIXMULWP(dwMargin1, 3);
		lAngle = (long)(short)wAngle;
		if ((long)((short*)&dwMargin1)[1] + lDelta > lAngle)
		{
			dwMargin2 = *pdwParams;
			HOVER_FIXMULWP(dwMargin2, 3);
			if (lDelta - (long)((short*)&dwMargin2)[1] < lAngle)
			{
				m_wTurnRateFrac = 0;
				// The original keeps lDelta in a register and does the
				// 16-bit subtract in it (sub di, [wAngle]); writing it
				// back through a WORD* would home the local instead.
				m_dwSledTimer--;
				m_wTurnRate = (WORD)lDelta - wAngle;
				return;
			}
		}
		if (Hover_AngleInFrontArc(lDelta, lAngle) != 0)
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
		m_dwSledTimer--;
			m_fxTurnRate = fxSpeed.m_dwValue;
		return;
	}
	m_dwSledTimer = 0xFFFFFFFF;
}

// Velocity integration: friction/brush forces on the +0x1B4/+0x1B8
// velocity pair, acceleration into +0x1AC/+0x1B0, clamping against the
// +0x1D4 thrust limit, angular drag from params +0x18, then the position
// (+0x44/+0x48) update and the proximity point refresh.

// hover.exe:0x40B0B0
// FUNCTION: HOVER 0x40b0b0
void CPlayer::MovePlayer()
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

	if (m_bFriction != 0)
	{
		if (m_fxAccelX == 0)
		{
			m_fxAccelX = m_fxVelX;
			if (m_bOnHoldPad != 0 && m_dwHoldTimer != 0)
				m_fxAccelX =
					(DWORD)((long)m_fxAccelX / -3);
			else
				HOVER_FIXMULP3(m_fxAccelX,
					m_pParams->m_adw[1]);
		}
		if (m_fxAccelY == 0)
		{
			m_fxAccelY = m_fxVelY;
			if (m_bOnHoldPad != 0 && m_dwHoldTimer != 0)
				m_fxAccelY =
					(DWORD)((long)m_fxAccelY / -3);
			else
				HOVER_FIXMULP3(m_fxAccelY,
					m_pParams->m_adw[1]);
		}
		if (labs((long)m_fxAccelX) < 0x28F &&
			labs((long)m_fxAccelY) < 0x28F)
		{
			m_fxAccelX = (DWORD)-(long)m_fxVelX;
			m_fxAccelY = (DWORD)-(long)m_fxVelY;
		}
	}
	m_fxVelX += m_fxAccelX;
	m_fxVelY += m_fxAccelY;
	((WORD*)&dwDist)[1] = (WORD)Hover_ApproxDistance((long)(short)m_wVelX,
		(long)(short)m_wVelY, 0, 0);
	((WORD*)&dwDist)[0] = 0;
	if ((long)dwDist < (long)m_fxMaxSpeed ||
		(long)dwDist == (long)m_fxMaxSpeed)
	{
		// within the thrust limit
	}
	else
	{
		dwScale = m_fxMaxSpeed;
		HOVER_FIXDIVLP(dwScale, dwDist);
		HOVER_FIXMULP3(m_fxVelX, dwScale);
		HOVER_FIXMULP3(m_fxVelY, dwScale);
		dwDist = m_fxMaxSpeed;
	}
	dwThrust = m_fxTurnRate;
	HOVER_FIXMULP3(dwThrust, m_pParams->m_adw[6]);
	// The drag is subtracted in memory in the original (rule 101's
	// by-pointer fixed-point operators): the volatile compound assignment
	// is the `mov [T],a / sub [T],b / mov eax,[T]` it emits.
	dwDrag = m_fxTurnImpulse;
	*(volatile DWORD*)&dwDrag -= dwThrust;
	m_fxTurnRate += dwDrag;
	m_fxHeading += m_fxTurnRate;
	if (m_bSteerAssist != 0)
	{
		if ((long)dwDist < 0x40000 || (long)dwDist == 0x40000)
		{
			// too slow to stir the velocity
		}
		else
		{
			lAngle = (long)(short)m_wHeading;
			lDiff = ((long)((short*)Hover_FixAngleP(&dwOut,
				m_fxVelY,
				m_fxVelX))[1] - lAngle) & 0x1FF;
			if (lDiff > 0xC0 && lDiff < 0x140)
				lAngle = (lAngle + 0x100) & 0x1FF;
			dwCos = g_adwSinTable[(((lAngle + 0x80) & 0x1FF) << 4) + 1];
			dwSin = g_adwSinTable[((lAngle & 0x1FF) << 4) + 1];
			HOVER_FIXMULP3(dwSin, dwDist);
			HOVER_FIXMULP3(dwCos, dwDist);
			dwDx = dwCos - m_fxVelX;
			dwDy = dwSin - m_fxVelY;
			lDiff = labs((long)dwDx);
			if (lDiff < 0x100 || lDiff == 0x100)
			{
				// small correction
			}
			else
			{
				HOVER_FIXMULP3(dwDx, m_pParams->m_adw[9]);
			}
			lDiff = labs((long)dwDy);
			if (0x100 > lDiff || 0x100 == lDiff)
			{
				// small correction
			}
			else
			{
				HOVER_FIXMULP3(dwDy, m_pParams->m_adw[9]);
			}
			m_fxVelX += dwDx;
			m_fxVelY += dwDy;
		}
	}
	m_fxX += m_fxVelX;
	m_fxY += m_fxVelY;
	m_fxHeading &= 0x1FFFFFF;
	UpdateProbe(dwDist);
	(theApp.m_pBumperDoc->m_pCollider)->ReinsertObject(this);
}

// HOVER_FIXMULWP3: the 16-bit expansion in the shape hover.exe uses inside
// CPlayer::CollidePlayer.  Rule 163's third slot appears here too, but on the
// *pointer* side: the destination address is handed over as the return value
// of Hover_FixArg, so each expansion owns one dead dword (the return
// temporary, rule 101's 0x88..0x94) plus the pointer home the asm reads back,
// while the factor stays down in the WORD pool with the other three.

inline void Hover_FixMulWP3(CFixArg fxA, WORD wB)
{
	_asm mov ecx, fxA.m_dwValue
	_asm movsx eax, wB
	_asm imul dword ptr [ecx]
	_asm mov [ecx], eax
}

#define HOVER_FIXMULWP3(dwA, wB) \
	Hover_FixMulWP3(Hover_FixArg((DWORD)&(dwA)), (WORD)(wB))

// Collision response against the player at dwA: pushes the pair apart
// when the proximity points are closer than 40 units, then splits both
// velocities along the collision normal and exchanges the tangential
// parts scaled by params +0x14/+0x10. Returns whether either player was
// moving along the normal.

// hover.exe:0x42B8E0
// FUNCTION: HOVER 0x42b8e0
int CPlayer::CollidePlayer(DWORD dwA, DWORD dwB)
{
	CPlayer* pOther;
	POINT ptMine;
	POINT ptProx;
	POINT* pProx;
	POINT* pMine;
	DWORD dwP1;
	DWORD dwP2;
	DWORD dwDy;
	DWORD dwDx;
	DWORD dwOut;
	DWORD dwOutA;
	DWORD dwOutB;
	DWORD dwAngle;
	DWORD dwAngle180;
	DWORD dwDist;
	// Each of the four is one eight-byte output buffer: Hover_SplitVelocity
	// stores two dwords through every pointer it is handed.
	DWORD adwA[2];
	DWORD adwB[2];
	DWORD adwC[2];
	DWORD adwD[2];
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
	pMine = &m_probe.ptFront;
	m_bBounced = 0;
	pOther->m_bBounced = 0;
	TouchesPlayer(pOther, &dwP1, &dwP2);
	// Virtual call at vtable slot 0x4C in the original
	// (hover.exe:0x4089E0 on CPlayer): returns &pOther->m_probe.
	pProx = &pOther->GetProbe()->ptFront;
	if (dwP1 == 0)
		pMine++;
	ptMine = *pMine;
	if (dwP2 == 0)
		pProx++;
	ptProx = *pProx;
	((WORD*)&dwDy)[0] = 0;
	((WORD*)&dwDy)[1] = (WORD)ptProx.y - (WORD)ptMine.y;
	((WORD*)&dwDx)[0] = 0;
	((WORD*)&dwDx)[1] = (WORD)ptProx.x - (WORD)ptMine.x;
	dwAngle = (long)((short*)Hover_FixAngleP(&dwOut, dwDy, dwDx))[1];
	dwAngle180 = (dwAngle + 0x100) & 0x1FF;
	dwDist = Hover_ApproxDistance(ptMine.x, ptMine.y, ptProx.x, ptProx.y);
	if ((long)dwDist < 0x28)
	{
		dwT1 = g_adwSinTable[(((dwAngle180 + 0x80) & 0x1FF) << 4) + 1];
		wHalf = (WORD)((0x28 - (long)dwDist) / 2);
		HOVER_FIXMULWP3(dwT1, wHalf);
		m_fxX += dwT1;
		dwT2 = g_adwSinTable[((dwAngle180 & 0x1FF) << 4) + 1];
		HOVER_FIXMULWP3(dwT2, wHalf);
		m_fxY += dwT2;
		dwT3 = g_adwSinTable[(((dwAngle + 0x80) & 0x1FF) << 4) + 1];
		HOVER_FIXMULWP3(dwT3, wHalf);
		pOther->m_fxX += dwT3;
		dwT4 = g_adwSinTable[((dwAngle & 0x1FF) << 4) + 1];
		HOVER_FIXMULWP3(dwT4, wHalf);
		pOther->m_fxY += dwT4;
	}
	dwVelAngle = (long)((short*)Hover_FixAngleP(&dwOut,
		m_fxVelY, m_fxVelX))[1];
	dwVelAngleOther = (long)((short*)Hover_FixAngleP(&dwOut,
		pOther->m_fxVelY,
		pOther->m_fxVelX))[1];
	bThis = Hover_AngleInFrontArc(dwVelAngle, dwAngle + 0x80);
	bOther = Hover_AngleInFrontArc(dwVelAngleOther, dwAngle180 + 0x80);
	((WORD*)adwA)[0] = 0;
	((WORD*)adwA)[1] = 0;
	((WORD*)adwA)[2] = 0;
	((WORD*)adwA)[3] = 0;
	((WORD*)adwB)[0] = 0;
	((WORD*)adwB)[1] = 0;
	((WORD*)adwB)[2] = 0;
	((WORD*)adwB)[3] = 0;
	((WORD*)adwC)[0] = 0;
	((WORD*)adwC)[1] = 0;
	((WORD*)adwC)[2] = 0;
	((WORD*)adwC)[3] = 0;
	((WORD*)adwD)[0] = 0;
	((WORD*)adwD)[1] = 0;
	((WORD*)adwD)[2] = 0;
	((WORD*)adwD)[3] = 0;
	if (bThis != 0)
	{
		Hover_SplitVelocity(m_fxVelX,
			m_fxVelY, dwAngle, adwA, adwB);
		m_fxVelX = adwA[0];
		m_fxVelY = adwA[1];
		dwAdj = ((long)(short)pOther->m_wHeading +
			((dwP1 < 1) ? 0x100 : 0)) & 0x1FF;
		Hover_TangentialComponent(&dwOutA, adwB[0], adwB[1], dwAdj);
		dwT5 = *(volatile DWORD*)&dwOutA;
		HOVER_FIXMULP3(dwT5, m_pParams->m_adw[5]);
		pOther->m_fxHeading += dwT5;
		dwT6 = *(volatile DWORD*)&dwOutA;
		HOVER_FIXMULP3(dwT6, m_pParams->m_adw[4]);
		pOther->m_fxTurnRate += dwT6;
	}
	if (bOther != 0)
	{
		Hover_SplitVelocity(pOther->m_fxVelX,
			pOther->m_fxVelY, dwAngle180, adwC, adwD);
		pOther->m_fxVelX = adwC[0];
		pOther->m_fxVelY = adwC[1];
		m_fxVelX += adwD[0];
		m_fxVelY += adwD[1];
		dwAdj = ((long)(short)m_wHeading +
			((dwP1 < 1) ? 0x100 : 0)) & 0x1FF;
		Hover_TangentialComponent(&dwOutB, adwD[0], adwD[1], dwAdj);
		dwT7 = *(volatile DWORD*)&dwOutB;
		HOVER_FIXMULP3(dwT7, m_pParams->m_adw[5]);
		m_fxHeading += dwT7;
		dwT8 = *(volatile DWORD*)&dwOutB;
		HOVER_FIXMULP3(dwT8, m_pParams->m_adw[4]);
		m_fxTurnRate += dwT8;
	}
	if (bThis != 0)
	{
		pOther->m_fxVelX += adwB[0];
		pOther->m_fxVelY += adwB[1];
	}
	m_bBounced = bThis;
	pOther->m_bBounced = bOther;
	if (bThis == 0)
	{
		if (bOther == 0)
			return 0;
	}
	return 1;
}

#undef HOVER_FIXMULWP3

// Proximity test between this player's points at +0x180/+0x188 and
// pOther's (fetched through the vtable slot 0x4C virtual, which returns
// &pOther->m_probe), using the max + min/2 metric against the sum of
// the two +0x190 range limits. *pdwA/*pdwB report which point pairs
// matched; returns 1 on the first match.

// hover.exe:0x42BE50
// FUNCTION: HOVER 0x42be50
int CPlayer::TouchesPlayer(CPlayer* pOther, DWORD* pdwA, DWORD* pdwB)
{
	PLAYERPROBE* pProx;
	PLAYERPROBE* pMine;
	long lLimit;
	long lX;
	long lY;
	long lMin;

	pMine = &m_probe;
	// Virtual call at vtable slot 0x4C in the original
	// (hover.exe:0x4089E0 on CPlayer): returns &pOther->m_probe.
	pProx = pOther->GetProbe();
	lLimit = (long)(pProx->lRadius + m_probe.lRadius);
	{
		long lMineP40 = (long)m_dwAltitude + 0x40;
		long lOtherP40 = (long)pOther->m_dwAltitude + 0x40;

		if (lMineP40 < (long)pOther->m_dwAltitude ||
			(long)m_dwAltitude > lOtherP40)
			return 0;
	}
	*pdwB = 1;
	*pdwA = 1;
	lX = labs(pMine->ptFront.x - pProx->ptFront.x);
	lY = labs(pMine->ptFront.y - pProx->ptFront.y);
	lMin = (lY < lX) ? lY : lX;
	if (lY - (lMin >> 1) + lX < lLimit)
		return 1;
	*pdwA = 0;
	lX = labs(pMine->ptBack.x - pProx->ptFront.x);
	lY = labs(pMine->ptBack.y - pProx->ptFront.y);
	lMin = (lY < lX) ? lY : lX;
	if (lY - (lMin >> 1) + lX < lLimit)
		return 1;
	*pdwB = 0;
	lX = labs(pMine->ptBack.x - pProx->ptBack.x);
	lY = labs(pMine->ptBack.y - pProx->ptBack.y);
	lMin = (lY < lX) ? lY : lX;
	if (lY - (lMin >> 1) + lX < lLimit)
		return 1;
	*pdwA = 1;
	lX = labs(pMine->ptFront.x - pProx->ptBack.x);
	lY = labs(pMine->ptFront.y - pProx->ptBack.y);
	lMin = (lY < lX) ? lY : lX;
	return (lY - (lMin >> 1) + lX < lLimit);
}

// Wall bounce response, called from CWall::OnTouch once the collider
// has reported a hit: the two proximity points of the player
// (m_probe.ptFront and m_probe.ptBack) are
// measured against the wall segment and the closer one decides the
// contact. When the contact falls between the wall's endpoints the player
// is reflected about the wall angle and pushed back out along the wall
// normal; when it falls off an end, the endpoint bounce BounceOffEndpoint takes
// over. Returns nonzero when the wall was actually hit (zero when the
// player is already moving away from it).

// hover.exe:0x41A6B0
// FUNCTION: HOVER 0x41a6b0
int CPlayer::BounceOffWall(CGameObj* pWall, DWORD dwB)
{
	POINT* pptFront;
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
	DWORD dwOut;
	DWORD* pdwAngle;
	DWORD dwCos;
	DWORD dwSin;

	pptFront = &m_probe.ptFront;
	// hover.exe keeps the wall pointer itself live (ebx) and reads the
	// segment angle off it as [pWall+0x20]; naming the sub-object in a
	// local instead makes VC hold that pointer and lose pWall (rule 375).
	nDist1 = pWall->m_seg.DistanceTo(pptFront, &dwOnSeg1, &dwSide1,
		&dwFirst1);
	nDist2 = pWall->m_seg.DistanceTo(&m_probe.ptBack, &dwOnSeg2, &dwSide2,
		&dwFirst2);
	// Both segment cases put the endpoint bounce *after* the body in
	// hover.exe (0x41A70F jumps forward to it, 0x41A80E), so the guard is
	// spelled the positive way round with the BounceOffEndpoint tail last.
	if (nDist2 > nDist1)
	{
		if (dwOnSeg1 != 0)
		{
			pdwAngle = Hover_FixAngleP(&dwOut, m_fxVelY,
				m_fxVelX);
			lWallAngle = (long)pWall->m_seg.m_dwAngle;
			if (Hover_AngleInFrontArc((long)((short*)pdwAngle)[1], lWallAngle) !=
				(dwSide1 == 0))
				return 0;
			m_bBounced = 1;
			Reflect(1, lWallAngle);
			if (nDist1 < 0x32)
			{
				lAngle = lWallAngle - 0x80 + (dwSide1 == 0 ? 0x100 : 0);
				dwCos = g_adwSinTable[(((lAngle + 0x80) & 0x1FF) << 4) + 1];
				HOVER_FIXMULWP(dwCos, 0x32 - (WORD)nDist1);
				m_fxX += dwCos;
				dwSin = g_adwSinTable[((lAngle & 0x1FF) << 4) + 1];
				HOVER_FIXMULWP(dwSin, 0x32 - (WORD)nDist1);
				m_fxY += dwSin;
			}
			return 1;
		}
		return BounceOffEndpoint(1, pptFront->x, pptFront->y, pWall, dwFirst1,
			nDist1);
	}

	// The back point is the closer one. Note that the endpoint case below
	// still passes the *front* point to BounceOffEndpoint: the original reuses
	// the pointer it set up for the first DistanceTo call, so the back
	// point never reaches the endpoint bounce.
	if (dwOnSeg2 != 0)
	{
		pdwAngle = Hover_FixAngleP(&dwOut, m_fxVelY,
			m_fxVelX);
		lWallAngle = (long)pWall->m_seg.m_dwAngle;
		if (Hover_AngleInFrontArc((long)((short*)pdwAngle)[1], lWallAngle) !=
			(dwSide2 == 0))
			return 0;
		m_bBounced = 1;
		Reflect(0, lWallAngle);
		if (nDist2 < 0x32)
		{
			lAngle = lWallAngle - 0x80 + (dwSide2 == 0 ? 0x100 : 0);
			dwCos = g_adwSinTable[(((lAngle + 0x80) & 0x1FF) << 4) + 1];
			HOVER_FIXMULWP(dwCos, 0x32 - (WORD)nDist2);
			m_fxX += dwCos;
			dwSin = g_adwSinTable[((lAngle & 0x1FF) << 4) + 1];
			HOVER_FIXMULWP(dwSin, 0x32 - (WORD)nDist2);
			m_fxY += dwSin;
		}
		return 1;
	}
	return BounceOffEndpoint(1, pptFront->x, pptFront->y, pWall, dwFirst2, nDist2);
}

// Reflects the velocity (m_fxVelX, m_fxVelY) about dwAngle: the
// component along the angle is removed twice (once through the first pair
// of stores, once through the second, exactly as the original wrote it),
// and the removed part is fed back into the heading (m_fxHeading) and the
// turn rate (m_fxTurnRate) scaled by the player parameters at +0x14/+0x10.
// dwFlip selects which side of the wall the reaction is taken from.

// hover.exe:0x41AB10
// FUNCTION: HOVER 0x41ab10
void CPlayer::Reflect(DWORD dwFlip, DWORD dwAngle)
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
	pdwVel = &m_fxVelX;
	((WORD*)adwAlong)[0] = 0;
	((WORD*)adwAlong)[1] = 0;
	((WORD*)adwAlong)[2] = 0;
	((WORD*)adwAlong)[3] = 0;
	((WORD*)adwSide)[0] = 0;
	((WORD*)adwSide)[1] = 0;
	((WORD*)adwSide)[2] = 0;
	((WORD*)adwSide)[3] = 0;
	Hover_SplitVelocity(pdwVel[0], pdwVel[1], dwAngle, adwAlong, adwSide);
	pdwVel[0] = adwSide[0];
	m_fxVelY = adwSide[1];
	pdwVel[0] = adwSide[0] - adwAlong[0];
	m_fxVelY = adwSide[1] - adwAlong[1];
	Hover_TangentialComponent(&dwTangent, adwAlong[0], adwAlong[1],
		(((dwFlip == 0) ? 0x100 : 0) + (long)(short)m_wHeading) & 0x1FF);
	pdwParams = m_pParams->m_adw;
	dwSpin = *(volatile DWORD*)&dwTangent;
	HOVER_FIXMULP3(dwSpin, pdwParams[5]);
	m_fxHeading += dwSpin;
	dwTurn = *(volatile DWORD*)&dwTangent;
	HOVER_FIXMULP3(dwTurn, pdwParams[4]);
	m_fxTurnRate += dwTurn;
}

// The endpoint case of the wall bounce: the contact is the segment
// endpoint selected by dwFirst (the first one when it is nonzero), and
// the bounce normal is the direction from that endpoint to (dwX, dwY).
// Returns 0 when the player is already moving away from the endpoint.

// hover.exe:0x421B50
// FUNCTION: HOVER 0x421b50
int CPlayer::BounceOffEndpoint(DWORD dwFlip, DWORD dwX, DWORD dwY, CGameObj* pWall,
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
		ptEnd1 = Hover_PtOf(&pWall->m_seg.m_dwX1);
		pdwEnd = (DWORD*)&ptEnd1;
	}
	else
	{
		ptEnd2 = Hover_PtOf(&pWall->m_seg.m_dwX2);
		pdwEnd = (DWORD*)&ptEnd2;
	}
	// The 16.16 deltas from the contact point to (dwX, dwY): only the
	// integer halves are set, so the low words stay zero.
	((WORD*)&dwDY)[1] = (WORD)pdwEnd[1] - (WORD)dwY;
	((WORD*)&dwDY)[0] = 0;
	((WORD*)&dwDX)[0] = 0;
	((WORD*)&dwDX)[1] = (WORD)pdwEnd[0] - (WORD)dwX;
	lNormal = ((long)((short*)Hover_FixAngleP(adwAngle, dwDY, dwDX))[1] +
		0x80) & 0x1FF;
	pdwAngle = Hover_FixAngleP(adwAngle, m_fxVelY,
		m_fxVelX);
	if (Hover_AngleInFrontArc((long)((short*)pdwAngle)[1], lNormal) == 0)
		return 0;
	m_bBounced = 1;
	Reflect(dwFlip, lNormal);
	if (nDist < 0x32)
	{
		dwCos = g_adwSinTable[(((lNormal + 0x100) & 0x1FF) << 4) + 1];
		HOVER_FIXMULWP(dwCos, 0x32 - (WORD)nDist);
		m_fxX += dwCos;
		dwSin = g_adwSinTable[(((lNormal + 0x80) & 0x1FF) << 4) + 1];
		HOVER_FIXMULWP(dwSin, 0x32 - (WORD)nDist);
		m_fxY += dwSin;
	}
	return 1;
}

/////////////////////////////////////////////////////////////////////////////
// CHumanPlayer

// hover.exe:0x421FA0
// FUNCTION: HOVER 0x421fa0
CHumanPlayer::CHumanPlayer()
{
	m_bHuman = 1;
	m_pParams = theApp.m_pHumanParams;
	ResetMotion();
	m_bInvincible = 0;
	m_bGodMode = 0;
	m_pInvinciblePod = 0;
	m_pSpeedPod = 0;
	m_pSlowPod = 0;
	m_pTempWallPod = 0;
	m_pSmokeBombPod = 0;
	m_pJumpPod = 0;
}

// hover.exe:0x422050
// FUNCTION: HOVER 0x422050
CHumanPlayer::~CHumanPlayer()
{
	// empty body in the original (just the base-class destructor call)
}

// hover.exe:0x409180
// FUNCTION: HOVER 0x409180
void CHumanPlayer::Think()
{
	CBumperView* pView;
	DWORD dwSound;

	m_dwFlagRemoverState = 0;
	if (m_bOnFlagRemover == 0)
		m_dwFlagRemoverState = 0xFFFFFFFF;
	if (m_bKilled == 0 && theApp.m_dwGameState == 3)
	{
		m_wThrustInput = 0;
		m_wThrustInputFrac = 0;
		// 32-bit copy: m_wTurnInputFrac = m_wThrustInputFrac, m_wTurnInput = m_wThrustInput
		m_fxTurnInput = m_fxThrustInput;
		pView = Hover_GetView();
		EnterCriticalSection(&theApp.m_csInput);
		if (pView->m_bInputRight != pView->m_bInputLeft)
		{
			DWORD dwC = pView->m_bInputLeft;

			m_wTurnInputFrac = 0;
			m_wTurnInput = (dwC < 1) ? 1 : -1;	// unsigned, as in hover.exe:0x4091FC
		}
		if (pView->m_bInputUp != pView->m_bInputDown)
		{
			DWORD dwC2 = pView->m_bInputDown;

			m_wThrustInputFrac = 0;
			m_wThrustInput = (dwC2 < 1) ? 1 : -1;	// unsigned (0x40921D)
			if (m_pSlowPod != 0)
				dwSound = 1;
			else
				dwSound = ((DWORD)m_pSpeedPod < 1) ? 2 : 3;
		}
		else
			dwSound = 0;
		LeaveCriticalSection(&theApp.m_csInput);
		if (dwSound != g_dwUnk_4C54A0 && theApp.m_dwGameState == 3)
		{
			if (theApp.m_bEngineSound != 0)
			{
				switch (dwSound)
				{
				case 0:
					((CSoundSlot*)theApp.m_pBumperDoc->m_adwSounds[0x1A])->
						PlayWave(5, 5, 0xFFFF);
					break;
				case 1:
					((CSoundSlot*)theApp.m_pBumperDoc->m_adwSounds[0x1B])->
						PlayWave(5, 5, 0xFFFF);
					break;
				case 2:
					((CSoundSlot*)theApp.m_pBumperDoc->m_adwSounds[0x1C])->
						PlayWave(5, 5, 0xFFFF);
					break;
				case 3:
					((CSoundSlot*)theApp.m_pBumperDoc->m_adwSounds[0x1D])->
						PlayWave(5, 5, 0xFFFF);
					break;
				}
			}
			g_dwUnk_4C54A0 = dwSound;
		}
	}
	CPlayer::Think();
}

// hover.exe:0x40C400
// FUNCTION: HOVER 0x40c400
void CHumanPlayer::OnTouch(CPlayer* pOther, DWORD dwB)
{
	CPlayer::OnTouch(pOther, dwB);
}

// The three overrides below are the original's inline forwarders: the
// compiler tail-jumps straight into the CPlayer version (hover.exe:
// 0x4089D0 -> 0x408960, 0x4089F0 -> 0x408990, 0x42C240 -> 0x42C050).

// hover.exe:0x4089D0
// FUNCTION: HOVER 0x4089d0
void CHumanPlayer::BeginFrame()
{
	CPlayer::BeginFrame();
}

// hover.exe:0x4089F0
// FUNCTION: HOVER 0x4089f0
void CHumanPlayer::EndFrame()
{
	CPlayer::EndFrame();
}

// hover.exe:0x42C240
// FUNCTION: HOVER 0x42c240
void CHumanPlayer::KillObject()
{
	CPlayer::KillObject();
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
	m_wBottom = 0;
	m_wUnkA8 = 0;
	m_bFrontTransparent = 0;
	m_wUnkAA = 0;
	m_bBackTransparent = 0;
	m_wUnkAC = 0;
	m_dwUnkC0 = 0;
	m_wFrontUOrg = 0;
	m_bSolid = 1;
	m_wTop = 0x80;
	m_wBackUOrg = 0;
	// hover.exe:0x419C11 calls CString::Empty on all six texture names
	// after the scalar block, although they were just default-constructed
	m_strFrontTopTex.Empty();
	m_strBackTopTex.Empty();
	m_strFrontTex.Empty();
	m_strBackTex.Empty();
	m_strFrontBottomTex.Empty();
	m_strBackBottomTex.Empty();
	m_pFrontTopTex = NULL;
	m_pBackTopTex = NULL;
	m_pFrontTex = NULL;
	m_pBackTex = NULL;
	m_pFrontBottomTex = NULL;
	m_pBackBottomTex = NULL;
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
		*pAr << m_strFrontTopTex;
		*pAr << m_strBackTopTex;
		*pAr << m_strFrontTex;
		*pAr << m_strBackTex;
		*pAr << m_strFrontBottomTex;
		*pAr << m_strBackBottomTex;
		pAr->WriteWord(m_wBottom);
		pAr->WriteWord(m_wTop);
		pAr->WriteWord(m_wUnkA8);
		pAr->WriteWord(m_wUnkAA);
		pAr->WriteByte((BYTE)m_bFrontTransparent);
		pAr->WriteByte((BYTE)m_bBackTransparent);
		pAr->WriteByte((BYTE)m_bSolid);
		pAr->WriteWord(5);		// the five extra bytes written below
		pAr->WriteByte((BYTE)m_dwUnkC0);
		pAr->WriteWord(m_wFrontUOrg);
		pAr->WriteWord(m_wBackUOrg);
	}
	else
	{
		if (!m_strName.IsEmpty())
			g_mapUnk_4A4578[m_strName] = this;
		*pAr >> m_strFrontTopTex;
		*pAr >> m_strBackTopTex;
		*pAr >> m_strFrontTex;
		*pAr >> m_strBackTex;
		*pAr >> m_strFrontBottomTex;
		*pAr >> m_strBackBottomTex;
		if (strcmp(m_strFrontTopTex, "") != 0)
		{
			if (!g_mapUnk_4A1D30.Lookup(m_strFrontTopTex,
					(CObject*&)m_pFrontTopTex))
				m_pFrontTopTex = NULL;
		}
		if (strcmp(m_strBackTopTex, "") != 0)
		{
			if (!g_mapUnk_4A1D30.Lookup(m_strBackTopTex,
					(CObject*&)m_pBackTopTex))
				m_pBackTopTex = NULL;
		}
		if (strcmp(m_strFrontTex, "") != 0)
		{
			if (!g_mapUnk_4A1D30.Lookup(m_strFrontTex,
					(CObject*&)m_pFrontTex))
			{
				m_pFrontTex = NULL;
			}
			else
			{
				m_bFrontTransparent = 1;
				if (m_pFrontTex->m_bTransparent == 0)
					m_bFrontTransparent = 0;
			}
		}
		if (strcmp(m_strBackTex, "") != 0)
		{
			if (!g_mapUnk_4A1D30.Lookup(m_strBackTex,
					(CObject*&)m_pBackTex))
			{
				m_pBackTex = NULL;
			}
			else
			{
				m_bBackTransparent = 1;
				if (m_pBackTex->m_bTransparent == 0)
					m_bBackTransparent = 0;
			}
		}
		if (strcmp(m_strFrontBottomTex, "") != 0)
		{
			if (!g_mapUnk_4A1D30.Lookup(m_strFrontBottomTex,
					(CObject*&)m_pFrontBottomTex))
				m_pFrontBottomTex = NULL;
		}
		if (strcmp(m_strBackBottomTex, "") != 0)
		{
			if (!g_mapUnk_4A1D30.Lookup(m_strBackBottomTex,
					(CObject*&)m_pBackBottomTex))
				m_pBackBottomTex = NULL;
		}
		pAr->ReadWord(&m_wBottom);
		pAr->ReadWord(&m_wTop);
		pAr->ReadWord(&m_wUnkA8);
		pAr->ReadWord(&m_wUnkAA);
		*(BYTE*)&m_bFrontTransparent = pAr->ReadByte();
		*(BYTE*)&m_bBackTransparent = pAr->ReadByte();
		*(BYTE*)&m_bSolid = pAr->ReadByte();
		// The count lives in the frame slot the original reserves with
		// its "sub esp, 4" (at [esp+0x12], the BYTE above it unused):
		// the by-pointer read is what puts it there and what produces
		// the in-memory "sub word ptr [esp+0x12], 5" and guard below.
		pAr->ReadWord((WORD*)&sCount);
		if (sCount >= 5)
		{
			*(BYTE*)&m_dwUnkC0 = pAr->ReadByte();
			pAr->ReadWord(&m_wFrontUOrg);
			pAr->ReadWord(&m_wBackUOrg);
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
		pAr->WriteWord(m_wX);
		pAr->WriteWord(m_wY);
		pAr->WriteWord(m_wAngle);
		pAr->WriteWord(m_wHeight);
		pAr->WriteWord((WORD)0);
	}
	else
	{
		if (!m_strName.IsEmpty())
			g_mapUnk_4A4768[m_strName] = this;
		pAr->ReadWord(&m_wX);
		pAr->ReadWord(&m_wY);
		pAr->ReadWord(&m_wAngle);
		pAr->ReadWord(&m_wHeight);
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
		pAr->WriteWord(m_wFace);
		pAr->WriteWord(m_wUnk46);
		pAr->WriteWord(m_wFront);
		pAr->WriteWord(m_wBack);
		pAr->WriteWord(m_wUnk60);
		pAr->WriteDouble(m_dblUnk50);
		pAr->WriteDouble(m_dblUnk58);
		pAr->WriteWord((WORD)0);
	}
	else
	{
		pAr->ReadWord(&m_wFace);
		pAr->ReadWord(&m_wUnk46);
		pAr->ReadWord(&m_wFront);
		pAr->ReadWord(&m_wBack);
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
int CMerlinWorld::AddSprite(CMerlinDynamic* pSprite)
{
	int nSize = m_arrSprites.GetSize();
	int i;

	for (i = 0; i < nSize; i++)
	{
		if (m_arrSprites[i] == pSprite)
		{
			if (i != -1)
				return 0;
			goto DoInsert;
		}
	}
DoInsert:
	m_arrSprites.SetAtGrow(nSize, pSprite);
	return 1;
}

// hover.exe:0x413DB0
// FUNCTION: HOVER 0x413db0
int CMerlinWorld::RemoveObject(CMerlinLine* pObj)
{
	int nIdx;
	int nCount;
	int bFound;
	int i;

	bFound = 0;
	nCount = m_arrSprites.GetSize();
	for (i = 0; i < nCount; i++)
	{
		if (m_arrSprites[i] == pObj)
			goto Found28;
	}
	i = -1;
Found28:
	nIdx = i;
	if (nIdx < 0)
	{
		nCount = m_arrStatics.GetSize();
		for (i = 0; i < nCount; i++)
		{
			if (m_arrStatics[i] == pObj)
				goto Found14;
		}
		i = -1;
Found14:
		nIdx = i;
		if (nIdx >= 0)
		{
			m_arrStatics[nIdx] = m_arrStatics[nCount - 1];
			m_arrStatics.RemoveAt(nCount - 1, 1);
			bFound = 1;
		}
	}
	else
	{
		m_arrSprites[nIdx] = m_arrSprites[nCount - 1];
		m_arrSprites.RemoveAt(nCount - 1, 1);
		bFound = 1;
	}
	if (bFound != 0 && pObj != 0)
		delete (CGameObj*)pObj;
	return (nIdx >= 0) ? 1 : 0;
}

// Removes pSprite from the m_arrSprites object array (swapping the last
// element into its slot) and deletes the object either way.
// hover.exe:0x414300
// FUNCTION: HOVER 0x414300
int CMerlinWorld::RemoveSprite(CMerlinDynamic* pSprite)
{
	int nIdx;
	int nCount;
	int i;

	nCount = m_arrSprites.GetSize();
	for (i = 0; i < nCount; i++)
	{
		if (m_arrSprites[i] == pSprite)
			goto Found;
	}
	i = -1;
Found:
	nIdx = i;
	if (nIdx >= 0)
	{
		m_arrSprites[nIdx] = m_arrSprites[nCount - 1];
		m_arrSprites.RemoveAt(nCount - 1, 1);
	}
	if (pSprite != 0)
		delete (CGameObj*)pSprite;
	return (nIdx >= 0) ? 1 : 0;
}

/////////////////////////////////////////////////////////////////////////////
// CSoundSlot

// hover.exe:0x415380 (the coverage table lists this function as 0x415386,
// the address after the 6-byte "mov eax,fs:0" SEH prefix)
// FUNCTION: HOVER 0x415380
CSoundSlot::CSoundSlot(const char* pszName, DWORD dwUnk)
{
	m_pMixWave = NULL;
	g_arrUnk_4A47F0.SetAtGrow(g_arrUnk_4A47F0.GetSize(), this);
	m_bOpenOnPlay = dwUnk;
	m_pWaveData = NULL;
	if (pszName != NULL)
		LoadWave(pszName);
	m_strWaveName = pszName;
	g_dwUnk_4C5700++;
}

// Loads the slot's wave file. Without the WaveMix engine (g_bUseWaveMix)
// the raw file is read into m_pWaveData; with it, m_pMixWave receives an
// open MIXWAVE handle (unless m_bOpenOnPlay defers the open to PlayWave).

// hover.exe:0x417860
// FUNCTION: HOVER 0x417860
int CSoundSlot::LoadWave(const char* pszName)
{
	char szFile[0x104];

	if (g_bUseWaveMix == 0)
	{
		DWORD cbData;

		// hover.exe spells the guarded block out rather than using MFC's
		// TRY/CATCH: there is no AFX_EXCEPTION_LINK construction in front
		// of it, the handler catches CException* and does its own
		// IsKindOf test (hover.exe:0x41795D), and the mismatch arm is a
		// bare rethrow (0x4179EC).
		wsprintf(szFile, "%sSounds\\Unmixed\\%s.WAV", (LPCTSTR)theApp.m_strDataDir,
			pszName);
		try
		{
			CFile file(szFile, CFile::modeRead | CFile::typeBinary);
			cbData = file.GetLength();
			m_pWaveData = ::operator new(cbData);
			file.Seek(0, CFile::begin);
			file.Read(m_pWaveData, cbData);
			file.Close();
		}
		catch (CException* e)
		{
			if (e->IsKindOf(RUNTIME_CLASS(CFileException)))
			{
				CFileException* pe = (CFileException*)e;

				if (m_pWaveData != NULL)
					delete (BYTE*)m_pWaveData;
				m_pWaveData = NULL;
				e->Delete();
				return 0;
			}
			throw;
		}
		return 1;
	}
	if (g_pMixSession == NULL)
		return 0;
	m_pMixWave = NULL;
	if (m_bOpenOnPlay != 0)
	{
		m_strWaveName = pszName;
		return 1;
	}
	wsprintf(szFile, "%sSounds\\Mixed\\%s.WAV", (LPCTSTR)theApp.m_strDataDir, pszName);
	m_pMixWave = WaveMixOpenWave(g_pMixSession, szFile,
		AfxGetInstanceHandle(), 1);
	if (m_pMixWave == NULL)
		return 0;
	return 1;
}

// Plays the slot's sound on channel dwA with flags dwB and loop count
// (WORD)dwC. Without the WaveMix engine (g_bUseWaveMix) the slot object is
// queued to the sound worker thread (looping when dwC == 0xFFFF); with it,
// the slot's wave file is opened on first use (registered in the
// g_mapOpenWaveToSlot handle map) and played through WaveMixPlay, showing the
// "wave devices in use" message box at most once per failure streak.

// hover.exe:0x40B800
// FUNCTION: HOVER 0x40b800
int CSoundSlot::PlayWave(DWORD dwA, DWORD dwB, DWORD dwC)
{
	MIXPLAYPARAMS playParams;
	char szFile[0x104];
	int nResult;

	if (g_bUseWaveMix == 0)
	{
		if (g_bWaveOutAvailable == 0)
			return 1;
		if (dwC == 0xFFFF)
		{
			EnterCriticalSection(&g_csSoundQueue);
			g_pSoundLoop = this;
			SetEvent(g_hSoundWakeEvent);
			LeaveCriticalSection(&g_csSoundQueue);
			return 0;
		}
		EnterCriticalSection(&g_csSoundQueue);
		g_pSoundOnce = this;
		SetEvent(g_hSoundWakeEvent);
		LeaveCriticalSection(&g_csSoundQueue);
		return 0;
	}
	playParams.pSession = g_pMixSession;
	playParams.nChannel = dwA;
	playParams.wLoops = (WORD)dwC;
	playParams.dwFlags = dwB;
	playParams.hwndNotify = NULL;
	playParams.wSize = sizeof(playParams);
	if (playParams.pSession == NULL)
		return 0;
	if (m_bOpenOnPlay == 0)
	{
		playParams.pMixWave = m_pMixWave;
		if (playParams.pMixWave == NULL)
			return 1;
	}
	else
	{
		strcpy(szFile, m_strWaveName);
		strcat(szFile, ".wav");
		m_pMixWave = WaveMixOpenWave(playParams.pSession, szFile,
			AfxGetInstanceHandle(), 1);
		playParams.pMixWave = m_pMixWave;
		if (playParams.pMixWave == NULL)
			return 1;
		if (AfxGetApp()->m_pMainWnd == NULL)
			playParams.hwndNotify = NULL;
		else
			playParams.hwndNotify = AfxGetApp()->m_pMainWnd->m_hWnd;
		g_mapOpenWaveToSlot[playParams.pMixWave] = this;
	}
	nResult = WaveMixPlay(&playParams);
	if (nResult != 0)
	{
		if (g_bWaveBusyMsgShown == 0)
		{
			g_bWaveBusyMsgShown = 1;
			AfxMessageBox(IDS_WAVE_BUSY, MB_OK, (UINT)-1);
		}
	}
	else
	{
		g_bWaveBusyMsgShown = 0;
	}
	if (nResult != 0 && m_bOpenOnPlay != 0)
		g_mapOpenWaveToSlot.RemoveKey(playParams.pMixWave);
	return nResult;
}

// hover.exe:0x4118A0
// FUNCTION: HOVER 0x4118a0
int CSoundSlot::StopWave(DWORD dwA, DWORD dwB)
{
	if (g_bUseWaveMix == 0)
	{
		if (g_pSoundThread != NULL)
		{
			EnterCriticalSection(&g_csSoundQueue);
			g_pSoundOnce = NULL;
			g_pSoundLoop = NULL;
			LeaveCriticalSection(&g_csSoundQueue);
			SetEvent(g_hSoundWakeEvent);
		}
		return 0;
	}
	if (g_pMixSession == NULL)
		return 0;
	return WaveMixFlushChannel(g_pMixSession, dwA, dwB);
}

/////////////////////////////////////////////////////////////////////////////
// CCollider

// hover.exe:0x40B510
// FUNCTION: HOVER 0x40b510
int CCollider::ReinsertObject(CPlayer* pPlayer)
{
	pPlayer->ReleaseRegionNodes();
	// Virtual call at vtable slot 0x30 in the original
	// (hover.exe:0x408820 on CPlayer), passing this object.
	pPlayer->UpdateRegions((DWORD)this);
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
// which is how the span setup Hover_BuildDepthList uses it.
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
WORD CMerlinLine::IntersectSegment(CMerlinLine* pLine, DWORD* pdwOut)
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

// WaveMixFreeWave: closes the MIXWAVE opened by LoadWave. Declared here
// rather than in hover.h (see notes/perfect-match.md rule 20).
int __stdcall WaveMixFreeWave(MIXSESSION* pEngine, MIXWAVE* pWave);

// Tears the slot down: when a raw buffer was loaded (no WaveMix engine) the
// sound worker thread is first made to drop the slot - under
// g_csSoundQueue both queue slots that still point at us are cleared and,
// if either did, we wait on g_hSoundAckEvent for the thread to acknowledge the
// kick before the buffer goes away. With the engine up the MIXWAVE handle
// is released instead. Finally the slot is unregistered from
// g_arrUnk_4A47F0 (swap with the last entry, then RemoveAt the tail).
//
// EH layout: the frame is the usual /GX one (push -1 / push 0x422E1D /
// push fs:[0]); state 1 covers m_strWaveName, and the two tail "call"s at
// 0x422DF1/0x422DFD are the compiler's unwind funclets for m_strWaveName
// (0x422E12 -> CString::~CString) and the CObject base (0x422E27 ->
// 0x401000) reused as the normal destruction path.

// hover.exe:0x422CB0 (the coverage table lists this function as 0x422CB6,
// the address after the 6-byte "mov eax,fs:0" SEH prefix)
// FUNCTION: HOVER 0x422cb0
CSoundSlot::~CSoundSlot()
{
	MIXSESSION* pEngine;
	int i;
	int bKicked = 0;

	if (m_pWaveData != NULL)
	{
		if (g_bUseWaveMix == 0 && g_pSoundThread != NULL)
		{
			EnterCriticalSection(&g_csSoundQueue);
			if (g_pSoundOnce == this)
			{
				g_pSoundOnce = NULL;
				bKicked = 1;
			}
			if (g_pSoundLoop == this)
			{
				g_pSoundLoop = NULL;
				bKicked = 1;
			}
			LeaveCriticalSection(&g_csSoundQueue);
			ResetEvent(g_hSoundAckEvent);
			SetEvent(g_hSoundWakeEvent);
			if (bKicked)
				WaitForSingleObject(g_hSoundAckEvent, INFINITE);
		}
		::operator delete(m_pWaveData);
	}
	pEngine = g_pMixSession;
	if (pEngine != NULL && g_bMixSessionOpen != 0 && m_pMixWave != NULL)
		WaveMixFreeWave(pEngine, m_pMixWave);
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
