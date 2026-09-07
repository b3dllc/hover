// mazeobj.cpp : implementation of the maze object classes (the CPodObj
// and CFloorObj hierarchies, CWall, CBeacon) and their pool allocators
//
// Reconstructed from hover.exe; see mazeobj.h for the RTTI/vtable data.

#include "hover.h"
#include <stdlib.h>			// abs()
#include "texture.h"
#include "gameobj.h"
#include "bumperdoc.h"
#include "mazeobj.h"
#include "bumpview.h"		// CSub422C80 (the CSlowPod dashboard gauge)
#include "texload.h"		// CCollider
#include "bumpview.h"

IMPLEMENT_DYNAMIC(CPodObj, CGameObj)
IMPLEMENT_DYNAMIC(CSmokeBombPod, CPodObj)
IMPLEMENT_DYNAMIC(CJumpPod, CPodObj)
IMPLEMENT_DYNAMIC(CInviciblePod, CPodObj)
IMPLEMENT_DYNAMIC(CTempWallPod, CPodObj)
IMPLEMENT_DYNAMIC(CSpeedPod, CPodObj)
IMPLEMENT_DYNAMIC(CMapEraserPod, CPodObj)
IMPLEMENT_DYNAMIC(CSlowPod, CPodObj)
IMPLEMENT_DYNAMIC(CFlag, CPodObj)
IMPLEMENT_DYNAMIC(CWall, CGameObj)
IMPLEMENT_DYNAMIC(CFloorObj, CGameObj)
IMPLEMENT_DYNAMIC(CSledPad, CFloorObj)
IMPLEMENT_DYNAMIC(CHoldPad, CFloorObj)
IMPLEMENT_DYNAMIC(CFlagRemover, CFloorObj)
IMPLEMENT_DYNAMIC(CBeacon, CGameObj)
IMPLEMENT_SERIAL(CMerlinDynamic, CMerlinLine, 1)
IMPLEMENT_DYNAMIC(CContainer, CPodObj)

/////////////////////////////////////////////////////////////////////////////
// Globals

// hover.exe:0x4C5334
// GLOBAL: HOVER 0x4c5334
DWORD g_dwUnk_4C5334 = 0xA;	// CPodObj pool block size (object count)

// hover.exe:0x4C53E4
// GLOBAL: HOVER 0x4c53e4
DWORD g_dwUnk_4C53E4 = 0xA;	// CFloorObj pool block size (object count)

// hover.exe:0x4C558C
// GLOBAL: HOVER 0x4c558c
DWORD g_dwUnk_4C558C = 0x20;	// CWall pool block size (object count)

// hover.exe:0x4C57AC
// GLOBAL: HOVER 0x4c57ac
DWORD g_dwUnk_4C57AC = 5;	// CBeacon pool block size (object count)

// hover.exe:0x4C5590
// GLOBAL: HOVER 0x4c5590
DWORD g_dwUnk_4C5590;	// incremented by CWall::Sub_408820, never read

// hover.exe:0x4A4840
// GLOBAL: HOVER 0x4a4840
CObArray g_obArrUnk_4A4840;	// beacon table built by Hover_sub_423EA0

// Sanity checks on the reconstructed class layouts (the pool allocators
// carve blocks in units of the class size, so these must match hover.exe)
typedef char _check_cpodobj_size[(sizeof(CPodObj) == 0x100) ? 1 : -1];
typedef char _check_cwall_size[(sizeof(CWall) == 0xA0) ? 1 : -1];
typedef char _check_cfloorobj_size[(sizeof(CFloorObj) == 0xB8) ? 1 : -1];
typedef char _check_cbeacon_size[(sizeof(CBeacon) == 0xB0) ? 1 : -1];

// The (WORD, WORD) gauge-value constructor is expanded inline at every use
// in this module (hover.exe stores the two words straight into the
// temporary), so it has to be defined before its first caller; the compiler
// still emits the out-of-line copy at hover.exe:0x42AE90.
// hover.exe:0x42AE90
// FUNCTION: HOVER 0x42ae90
inline CSub42AE90::CSub42AE90(WORD wInt, WORD wFrac)
{
	m_wFrac = wFrac;
	m_wInt = wInt;
}



// hover.exe expands CSub422C80::Sub_42A9F0 (0x42A9F0) inline in the five
// per-frame pod handlers below while still calling it from the two
// Sub_42AD50 handlers, i.e. the gauge update was written twice: once as the
// out-of-line method and once here. `pGauge` must be a local, because the
// original computes the gauge address once.
#define HOVER_INLINE_SUB_42A9F0(pGauge, fix) \
	do { \
		long _lSegs; \
		long _lIndex; \
		*(long*)&(fix) += 0x288; \
		HOVER_FIXMULWP(*(DWORD*)&(fix), (WORD)(pGauge)->m_dwUnk1C); \
		_lIndex = (short)(fix).m_wInt; \
		_lSegs = (long)(pGauge)->m_dwUnk1C; \
		if (_lIndex > _lSegs) \
			_lIndex = _lSegs; \
		if (_lIndex >= _lSegs) \
		{ \
			(pGauge)->m_dwUnk24 = _lSegs; \
		} \
		else if (_lIndex <= 0) \
		{ \
			(pGauge)->m_dwUnk24 = 0; \
		} \
		else \
		{ \
			(pGauge)->m_dwUnk24 = _lIndex; \
		} \
	} while (0)

/////////////////////////////////////////////////////////////////////////////
// CPodObj

// hover.exe:0x413050
// FUNCTION: HOVER 0x413050
CPodObj::CPodObj()
{
	char szName[0x34];
	CMerlinTexture* pTexture;
	int nHalf;

	m_awUnkE0[0] = 0;
	m_awUnkE0[1] = 0;
	m_awUnkE0[2] = 0;
	m_awUnkE0[3] = 0;
	m_awUnkE0[4] = 0;
	m_dwUnk88 = 0;
	m_awUnkE0[5] = 0;
	m_awUnkE0[6] = 0;
	m_awUnkE0[7] = 0;
	m_dwUnkAC = 0x64;	// stored 0x64 here, then overwritten with 0 below
	m_pUnk8C = NULL;
	wsprintfA(szName, "%s%02d", "POD06_", 0);
	g_mapUnk_4A1D30.Lookup(szName, (CObject*&)m_pUnk8C);
	pTexture = m_pUnk8C;
	m_dwUnkB8 = (short)pTexture->m_wUnk18;
	nHalf = (short)pTexture->m_wUnk14 / 2;
	m_dwUnk94 = 0;
	m_dwUnk64 = 0;
	m_dwUnkAC = 0;
	m_dwUnk9C = 0xA;
	m_dwUnkB4 = 0x80;
	m_dwUnk98 = 1;
	m_dwUnkBC = (short)nHalf;
}

// hover.exe:0x42AF20
// FUNCTION: HOVER 0x42af20
void* CPodObj::operator new(size_t nSize)
{
	CPodObj* pObj;
	int nCount;
	int i;

	pObj = (CPodObj*)g_dwUnk_4C5330;
	if (pObj == NULL)
	{
		g_dwUnk_4C5330 = (DWORD)::operator new(g_dwUnk_4C5334 * sizeof(CPodObj));
		g_listUnk_4A45B8.AddTail((void*)g_dwUnk_4C5330);
		nCount = g_dwUnk_4C5334;
		pObj = (CPodObj*)g_dwUnk_4C5330;
		for (i = 0; i < nCount - 1; i++)
			pObj[i].m_dwUnk84 = (DWORD)&pObj[i + 1];
		pObj[nCount - 1].m_dwUnk84 = 0;
	}
	g_dwUnk_4C5330 = pObj->m_dwUnk84;
	return pObj;
}

// hover.exe:0x42AF90
// FUNCTION: HOVER 0x42af90
void CPodObj::operator delete(void* p)
{
	DWORD dwHead = g_dwUnk_4C5330;
	g_dwUnk_4C5330 = (DWORD)p;
	((CPodObj*)p)->m_dwUnk84 = dwHead;
}

// The class destructor has an empty body; the pool release happens in the
// compiler-generated deleting destructor (hover.exe:0x42B000).
// hover.exe:0x411580
// FUNCTION: HOVER 0x411580
CPodObj::~CPodObj()
{
}

// Per-frame think (vtable slot 7). CPodObj has nothing of its own to do
// and forwards to CGameObj; the original still emits the override as a
// one-instruction tail jump.
// hover.exe:0x40D2E0
// FUNCTION: HOVER 0x40d2e0
void CPodObj::Sub_42AD30()
{
	CGameObj::Sub_42AD30();
}

// Player touched the pod (vtable slot 8): flag the view for a dashboard
// repaint, drop the pod from the document's active list and put it back
// on the draw list.
// hover.exe:0x4115D0
// FUNCTION: HOVER 0x4115d0
void CPodObj::Sub_42AD40(CPlayer* pOther, DWORD dwB)
{
	Hover_sub_401140()->m_dwUnk051C = 1;
	theApp.m_pBumperDoc->Sub_410250(this);
	Sub_408530();
	m_dwUnk38 = 1;
}

// Proximity test (vtable slot 10): is the point (dwA,dwB) within
// m_dwUnk9C + dwC of the pod, measured with the max + min/2 metric?
// hover.exe:0x413E70
// FUNCTION: HOVER 0x413e70
int CPodObj::Sub_42AD70(DWORD dwA, DWORD dwB, DWORD dwC)
{
	long lRange;
	DWORD adwPos[2];
	long lDX;
	long lDY;
	long lMin;

	lRange = (long)m_dwUnk9C + (long)dwC;
	Sub_403880(adwPos);
	lDX = abs((long)dwA - (long)adwPos[0]);
	lDY = abs((long)dwB - (long)adwPos[1]);
	lMin = (lDY < lDX) ? lDY : lDX;
	return (lDY - (lMin >> 1) + lDX) < lRange;
}

// Pickup test (vtable slot 11): only the human player picks pods up
// (a CFlag is picked up by anyone). Tests the height bands for overlap
// and then both of the player's proximity points against the pod radius.
// hover.exe:0x40CB40
// FUNCTION: HOVER 0x40cb40
int CPodObj::Sub_42AD60(DWORD dwUnk)
{
	DWORD* pdwPts;
	long lRange;
	long lZ;
	long lPodZ;
	long lPodTop;
	long lTop;
	DWORD adwPos[2];
	long lDX;
	long lDY;
	long lMin;
	CPlayer* pPlayer = (CPlayer*)dwUnk;

	if (!pPlayer->IsKindOf(RUNTIME_CLASS(CHumanPlayer)) &&
		!IsKindOf(RUNTIME_CLASS(CFlag)))
		return 0;

	pdwPts = pPlayer->Sub_4089E0();
	lRange = (long)m_dwUnk9C + (long)pdwPts[4];
	lZ = (long)pPlayer->m_dwUnk01A4;
	lPodZ = (long)m_dwUnkB0;
	lPodTop = lPodZ + 0x40;
	lTop = lZ + 0x40;
	// one guard, so the two height tests share a single return-0 tail
	if (lTop < lPodZ || lPodTop < lZ)
		return 0;

	Sub_403880(adwPos);
	lDX = abs((long)pdwPts[0] - (long)adwPos[0]);
	lDY = abs((long)pdwPts[1] - (long)adwPos[1]);
	lMin = (lDY < lDX) ? lDY : lDX;
	if (lDY - (lMin >> 1) + lDX < lRange)
		return 1;

	lDX = abs((long)pdwPts[2] - (long)adwPos[0]);
	lDY = abs((long)pdwPts[3] - (long)adwPos[1]);
	lMin = (lDY < lDX) ? lDY : lDX;
	return (lDY - (lMin >> 1) + lDX) < lRange;
}

// Register the pod with the collider (vtable slot 12): insert it into
// every region-matrix cell its radius touches.
// hover.exe:0x413180
// FUNCTION: HOVER 0x413180
int CPodObj::Sub_408820(DWORD dwA)
{
	int nCell;
	DWORD adwPos[2];
	int nX1;
	int nX2;
	int nY1;
	int nY2;
	int y;
	int x;
	CCollider* pMgr = (CCollider*)dwA;

	nCell = (int)pMgr->m_dwUnk0C;
	Sub_403880(adwPos);
	nX2 = ((long)adwPos[0] + (long)m_dwUnk9C) / nCell;
	nY1 = ((long)adwPos[1] - (long)m_dwUnk9C) / nCell;
	nY2 = ((long)adwPos[1] + (long)m_dwUnk9C) / nCell;
	nX1 = ((long)adwPos[0] - (long)m_dwUnk9C) / nCell;
	x = nX1;
	if (nX2 >= x)
	{
		do
		{
			y = nY1;
			if (nY2 >= y)
			{
				do
				{
					if (pMgr->m_dwUnk44 != 0)
						pMgr->Sub_40F8A0(x, y);
					else
						pMgr->m_obj10.Sub_4086A0(x, y, this);
					y++;
				} while (nY2 >= y);
			}
			x++;
		} while (nX2 >= x);
	}
	return 1;
}

// Start of frame (vtable slot 15): snapshot the position, latch the
// texture and the height, and mark the pod dirty.
// hover.exe:0x40E340
// FUNCTION: HOVER 0x40e340
void CPodObj::Sub_4087D0()
{
	CGameObj::Sub_4087D0();
	m_dwUnk90 = (DWORD)m_pUnk8C;
	m_dwUnkA8 = m_dwUnkB4;
	m_dwUnk64 = 1;
	m_dwUnkB0 = m_dwUnkAC + m_dwUnkA4;
}

// End of frame (vtable slot 16): create, update or drop the CMerlinDynamic
// sprite that represents the pod inside the level world.
// hover.exe:0x40E380
// FUNCTION: HOVER 0x40e380
void CPodObj::Sub_4087F0()
{
	CBumperDoc* pDoc;
	CMerlinDynamic* pSprite;
	long lBase;
	DWORD adwPos[2];

	pDoc = theApp.m_pBumperDoc;
	if (m_dwUnk60 == 0)
	{
		if (m_dwUnk94 != 0 && m_dwUnk78 == 0 && m_dwUnk64 != 0)
		{
			m_dwUnk78 = (DWORD)new CMerlinDynamic;
			pSprite = (CMerlinDynamic*)m_dwUnk78;
			pSprite->m_pUnk50 = (CMerlinTexture*)m_dwUnk90;
			pSprite->m_dwUnk60 = 0;
			((CMerlinWorld*)pDoc->m_dwUnk0188)->Sub_4128A0((DWORD)pSprite);
			if (m_dwUnk78 != 0)
				((CMerlinDynamic*)m_dwUnk78)->m_wFlags |= 8;
		}
		else if (m_dwUnk94 == 0 && m_dwUnk78 != 0)
		{
			((CMerlinWorld*)pDoc->m_dwUnk0188)->Sub_414300(m_dwUnk78);
			m_dwUnk78 = 0;
		}
		pSprite = (CMerlinDynamic*)m_dwUnk78;
		if (pSprite != NULL)
		{
			lBase = (long)m_dwUnkB0;
			pSprite->m_wUnk56 = (WORD)((long)m_dwUnkB8 + lBase);
			if (lBase <= 0)
				lBase = 0;
			pSprite->m_wUnk54 = (WORD)lBase;
			pSprite->m_wUnk58 = (WORD)m_dwUnkBC;
			pSprite->m_wUnk5A = (WORD)Sub_413000(adwPos)[0];
			pSprite->m_wUnk5C = (WORD)Sub_413000(adwPos)[1];
		}
	}
	CGameObj::Sub_4087F0();
}

// Link into the draw list (vtable slot 17); forwards to CGameObj.
// hover.exe:0x42AFB0
// FUNCTION: HOVER 0x42afb0
void CPodObj::Sub_413020()
{
	CGameObj::Sub_413020();
}

/////////////////////////////////////////////////////////////////////////////
// The power-up pods

// hover.exe:0x420FD0
// FUNCTION: HOVER 0x420fd0
CSmokeBombPod::CSmokeBombPod()
{
	char szName[0x34];

	m_dwUnk28 = 0;
	m_dwUnk94 = 0;
	m_pUnk8C = NULL;
	m_dwUnk9C = 0x40;
	wsprintfA(szName, "%s%02d", "POD00_", 0);
	g_mapUnk_4A1D30.Lookup(szName, (CObject*&)m_pUnk8C);
}

// The destructor is defined in the class declaration, so the compiler folds
// it into the scalar deleting destructor that the vtable slot holds - which
// is what hover.exe:0x421290 is. The block below emits no code; it only gives
// the reccmp marker a declaration to name ??_GCSmokeBombPod@@ (rules 34/184).
// hover.exe:0x421290
#if 0
// FUNCTION: HOVER 0x421290
CSmokeBombPod::~CSmokeBombPod()
{
}
#endif

// Per-frame think (slot 7): count the smoke-bomb timer down, drive the
// dashboard gauge from it, and release the pod when it runs out.
// hover.exe:0x42A660
// FUNCTION: HOVER 0x42a660
void CSmokeBombPod::Sub_42AD30()
{
	CSub42AE90 fix;
	CSub42AE90 fixOut;
	CSub422C80* pGauge;
	CBumperView* pView;

	if (m_dwUnk5C == 0 && m_dwUnk28 != 0)
	{
		m_dwUnkA0--;
		fix = CSub42AE90((WORD)m_dwUnkA0, 0);
		*(long*)&fixOut = *(long*)&fix / (short)theApp.m_dwUnk0168;
		pGauge = &Hover_sub_401140()->m_obj03D0;
		HOVER_INLINE_SUB_42A9F0(pGauge, fixOut);
		if (m_dwUnkA0 == 0)
		{
			pView = Hover_sub_401140();
			pView->m_dwUnk021C = 0;
			((CPlayer*)m_dwUnk88)->m_dwUnk0134 = 0;
			theApp.m_pBumperDoc->Sub_412630(this);
			if (theApp.m_dwUnk0408 != 0)
				Sub_40D6E0(0x18, 4, 5, 0);
			return;
		}
	}
	CPodObj::Sub_42AD30();
}

// Player touched the pod (slot 8): add it to the player's smoke-bomb
// stock (up to nine), take it out of the maze and drop a fresh smoke
// bomb pod somewhere else on the board.
// hover.exe:0x42A760
// FUNCTION: HOVER 0x42a760
void CSmokeBombPod::Sub_42AD40(CPlayer* pOther, DWORD dwB)
{
	CPlayer* pPlayer;
	CSmokeBombPod* pNew;
	CBumperView* pView;

	// the "no more pods" message is the cold arm at the end of the
	// function in hover.exe, so it has to be the `else`
	if (pOther->m_obList00EC.GetCount() < 9)
	{
		CPodObj::Sub_42AD40(pOther, dwB);
		m_dwUnk88 = (DWORD)pOther;
		m_dwUnk94 = 0;
		((CCollider*)theApp.m_pBumperDoc->m_dwUnk8388)->Sub_40E710(this);
		pPlayer = (CPlayer*)m_dwUnk88;
		if (pPlayer->m_dwUnk01F8 != 0 && theApp.m_dwUnk0408 != 0)
			Sub_40D6E0(7, 1, 5, 0);
		((CPlayer*)m_dwUnk88)->m_obList00EC.AddHead(this);
		pView = Hover_sub_401140();
		pView->m_obj0268.m_dwUnk00 = 1;
		pView->m_obj0268.m_dwUnk08 = ((CPlayer*)m_dwUnk88)->m_obList00EC.GetCount();
		pNew = new CSmokeBombPod;
		theApp.m_pBumperDoc->Sub_412250(pNew, (rand() % 4) == 0);
	}
	else if (pOther->m_dwUnk01F8 != 0 && theApp.m_dwUnk0408 != 0)
	{
		Sub_40D6E0(5, 1, 5, 0);
	}
}

// Deploy (slot 9): the player drops the smoke bomb; it goes back into
// the maze, blanks the view's shade table and starts the timer.
// hover.exe:0x42A900
// FUNCTION: HOVER 0x42a900
void CSmokeBombPod::Sub_42AD50(DWORD dwUnk)
{
	CPlayer* pPlayer;
	POSITION pos;

	m_dwUnkA0 = theApp.m_dwUnk0168;
	m_dwUnk28 = 1;
	Hover_sub_401140()->m_dwUnk021C = 1;
	theApp.m_pBumperDoc->m_list83A8.AddTail(this);
	pPlayer = (CPlayer*)m_dwUnk88;
	if (pPlayer->m_dwUnk0134 != 0)
		theApp.m_pBumperDoc->Sub_412630((CGameObj*)pPlayer->m_dwUnk0134);
	pos = ((CPlayer*)m_dwUnk88)->m_obList00EC.Find(this);
	((CPlayer*)m_dwUnk88)->m_obList00EC.RemoveAt(pos);
	((CPlayer*)m_dwUnk88)->m_dwUnk0134 = (DWORD)this;
	Hover_sub_401140()->m_obj03D0.Sub_42A9F0(CSub42AE90(1.0));
	if (theApp.m_dwUnk0408 != 0)
		Sub_40D6E0(0x17, 4, 5, 0);
}

// Start/end of frame and draw-list linkage (slots 15/16/17): forwarded.
// hover.exe:0x421270
// FUNCTION: HOVER 0x421270
void CSmokeBombPod::Sub_4087D0()
{
	CPodObj::Sub_4087D0();
}

// hover.exe:0x421280
// FUNCTION: HOVER 0x421280
void CSmokeBombPod::Sub_4087F0()
{
	CPodObj::Sub_4087F0();
}

// hover.exe:0x42AA60
// FUNCTION: HOVER 0x42aa60
void CSmokeBombPod::Sub_413020()
{
	CGameObj::Sub_413020();
}

// hover.exe:0x421310
// FUNCTION: HOVER 0x421310
CJumpPod::CJumpPod()
{
	char szName[0x34];

	m_dwUnk28 = 0;
	m_dwUnk94 = 0;
	m_pUnk8C = NULL;
	m_dwUnk9C = 0x40;
	wsprintfA(szName, "%s%02d", "POD01_", 0);
	g_mapUnk_4A1D30.Lookup(szName, (CObject*&)m_pUnk8C);
}

// The destructor is defined in the class declaration, so the compiler folds
// it into the scalar deleting destructor that the vtable slot holds - which
// is what hover.exe:0x4213C0 is. The block below emits no code; it only gives
// the reccmp marker a declaration to name ??_GCJumpPod@@ (rules 34/184).
// hover.exe:0x4213C0
#if 0
// FUNCTION: HOVER 0x4213c0
CJumpPod::~CJumpPod()
{
}
#endif

// Per-frame think (slot 7): once the player is back on the ground the
// deployed jump pad is put back into the maze.
// hover.exe:0x42AA80
// FUNCTION: HOVER 0x42aa80
void CJumpPod::Sub_42AD30()
{
	CPlayer* pPlayer;

	if (m_dwUnk5C == 0 && m_dwUnk28 != 0)
	{
		pPlayer = (CPlayer*)m_dwUnk88;
		if (*(DWORD*)&pPlayer->m_pad00A8[0] == 0)
		{
			pPlayer->m_dwUnk0138 = 0;
			theApp.m_pBumperDoc->Sub_412630(this);
			return;
		}
	}
	CPodObj::Sub_42AD30();
}

// Player touched the pod (slot 8): add it to the player's jump-pad stock
// (up to nine) and drop a fresh jump pod somewhere else on the board.
// hover.exe:0x42AAC0
// FUNCTION: HOVER 0x42aac0
void CJumpPod::Sub_42AD40(CPlayer* pOther, DWORD dwB)
{
	CPlayer* pPlayer;
	CJumpPod* pNew;
	CBumperView* pView;

	// the "no more pods" message is the cold arm at the end of the
	// function in hover.exe, so it has to be the `else`
	if (pOther->m_obList0108.GetCount() < 9)
	{
		CPodObj::Sub_42AD40(pOther, dwB);
		m_dwUnk88 = (DWORD)pOther;
		m_dwUnk94 = 0;
		((CCollider*)theApp.m_pBumperDoc->m_dwUnk8388)->Sub_40E710(this);
		pPlayer = (CPlayer*)m_dwUnk88;
		if (pPlayer->m_dwUnk01F8 != 0 && theApp.m_dwUnk0408 != 0)
			Sub_40D6E0(6, 1, 5, 0);
		((CPlayer*)m_dwUnk88)->m_obList0108.AddHead(this);
		pView = Hover_sub_401140();
		pView->m_obj0240.m_dwUnk00 = 1;
		pView->m_obj0240.m_dwUnk08 = ((CPlayer*)m_dwUnk88)->m_obList0108.GetCount();
		pNew = new CJumpPod;
		theApp.m_pBumperDoc->Sub_412250(pNew, (rand() % 4) == 0);
	}
	else if (pOther->m_dwUnk01F8 != 0 && theApp.m_dwUnk0408 != 0)
	{
		Sub_40D6E0(5, 1, 5, 0);
	}
}

// Deploy (slot 9): the player drops the jump pad under himself.
// hover.exe:0x42AC60
// FUNCTION: HOVER 0x42ac60
void CJumpPod::Sub_42AD50(DWORD dwUnk)
{
	CPlayer* pPlayer;
	POSITION pos;

	theApp.m_pBumperDoc->m_list83A8.AddTail(this);
	m_dwUnk28 = 1;
	pPlayer = (CPlayer*)m_dwUnk88;
	pPlayer->m_dwUnk0138 = (DWORD)this;
	*(DWORD*)&pPlayer->m_pad00A8[0] = 1;
	pPlayer->m_awUnk0090[9] += 0x100;
	pos = ((CPlayer*)m_dwUnk88)->m_obList0108.Find(this);
	((CPlayer*)m_dwUnk88)->m_obList0108.RemoveAt(pos);
	if (theApp.m_dwUnk0408 != 0)
		Sub_40D6E0(0x15, 4, 5, 0);
}

// hover.exe:0x421300
// FUNCTION: HOVER 0x421300
void CJumpPod::Sub_4087D0()
{
	CPodObj::Sub_4087D0();
}

// hover.exe:0x4213B0
// FUNCTION: HOVER 0x4213b0
void CJumpPod::Sub_4087F0()
{
	CPodObj::Sub_4087F0();
}

// hover.exe:0x42ACE0
// FUNCTION: HOVER 0x42ace0
void CJumpPod::Sub_413020()
{
	CGameObj::Sub_413020();
}

// hover.exe:0x421440
// FUNCTION: HOVER 0x421440
CInviciblePod::CInviciblePod()
{
	char szName[0x34];

	m_dwUnk28 = 0;
	m_dwUnk94 = 0;
	m_pUnk8C = NULL;
	m_dwUnk9C = 0x40;
	wsprintfA(szName, "%s%02d", "POD04_", 0);
	g_mapUnk_4A1D30.Lookup(szName, (CObject*&)m_pUnk8C);
}

// The class destructor is empty and lives in the class declaration, so the
// compiler folds it into the scalar deleting destructor that the vtable slot
// holds - which is exactly what hover.exe:0x4146C0 is. The marker below has
// no code of its own; it names ??_GCInviciblePod@@ for reccmp (rule 34/184).
// hover.exe:0x4146C0
#if 0
// FUNCTION: HOVER 0x4146c0
CInviciblePod::~CInviciblePod()
{
}
#endif

// Per-frame think (slot 7): count the invincibility timer down, drive the
// dashboard gauge, and clear the player's invincibility when it expires.
// hover.exe:0x40DE80
// FUNCTION: HOVER 0x40de80
void CInviciblePod::Sub_42AD30()
{
	CSub42AE90 fix;
	CSub42AE90 fixOut;
	CSub422C80* pGauge;
	CPlayer* pPlayer;

	if (m_dwUnk5C == 0 && m_dwUnk28 != 0)
	{
		m_dwUnkA0--;
		fix = CSub42AE90((WORD)m_dwUnkA0, 0);
		*(long*)&fixOut = *(long*)&fix / (short)theApp.m_dwUnk015C;
		pGauge = &Hover_sub_401140()->m_obj0310;
		HOVER_INLINE_SUB_42A9F0(pGauge, fixOut);
		if (m_dwUnkA0 == 0)
		{
			if (theApp.m_dwUnk0408 != 0)
				Sub_40D6E0(0xA, 1, 5, 0);
			pPlayer = (CPlayer*)m_dwUnk88;
			if (pPlayer->m_dwUnk0144 == 0)
				pPlayer->m_dwUnk0140 = 0;
			pPlayer->m_dwUnk0124 = 0;
			theApp.m_pBumperDoc->Sub_412630(this);
			return;
		}
	}
	CPodObj::Sub_42AD30();
}

// Player touched the pod (slot 8): make the player invincible for
// theApp.m_dwUnk015C frames, cancelling any slow pod that was running.
// hover.exe:0x4181E0
// FUNCTION: HOVER 0x4181e0
void CInviciblePod::Sub_42AD40(CPlayer* pOther, DWORD dwB)
{
	CPlayer* pPlayer;
	CInviciblePod* pNew;

	CPodObj::Sub_42AD40(pOther, dwB);
	theApp.m_pBumperDoc->m_list83A8.AddTail(this);
	m_dwUnk88 = (DWORD)pOther;
	m_dwUnk94 = 0;
	((CCollider*)theApp.m_pBumperDoc->m_dwUnk8388)->Sub_40E710(this);
	pPlayer = (CPlayer*)m_dwUnk88;
	if (pPlayer->m_dwUnk01F8 != 0 && theApp.m_dwUnk0408 != 0)
		Sub_40D6E0(9, 1, 5, 0);
	if (m_dwUnk98 != 0)
	{
		pNew = new CInviciblePod;
		theApp.m_pBumperDoc->Sub_412250(pNew, (rand() % 4) == 0);
	}
	m_dwUnkA0 = theApp.m_dwUnk015C;
	m_dwUnk28 = 1;
	((CPlayer*)m_dwUnk88)->m_dwUnk0140 = 1;
	if (((CPlayer*)m_dwUnk88)->m_dwUnk0124 != 0)
		theApp.m_pBumperDoc->Sub_412630(
			(CGameObj*)((CPlayer*)m_dwUnk88)->m_dwUnk0124);
	if (((CPlayer*)m_dwUnk88)->m_dwUnk012C != 0)
	{
		theApp.m_pBumperDoc->Sub_412630(
			(CGameObj*)((CPlayer*)m_dwUnk88)->m_dwUnk012C);
		pPlayer = (CPlayer*)m_dwUnk88;
		pPlayer->m_dwUnk012C = 0;
		*(DWORD*)&pPlayer->m_awUnk01D0[2] = *(DWORD*)&pPlayer->m_awUnk01D0[4];
		Hover_sub_401140()->m_obj0340.Sub_42A9F0(CSub42AE90(0, 0));
	}
	((CPlayer*)m_dwUnk88)->m_dwUnk0124 = (DWORD)this;
}

// hover.exe:0x413E50
// FUNCTION: HOVER 0x413e50
void CInviciblePod::Sub_4087D0()
{
	CPodObj::Sub_4087D0();
}

// hover.exe:0x413E60
// FUNCTION: HOVER 0x413e60
void CInviciblePod::Sub_4087F0()
{
	CPodObj::Sub_4087F0();
}

// hover.exe:0x419DF0
// FUNCTION: HOVER 0x419df0
void CInviciblePod::Sub_413020()
{
	CGameObj::Sub_413020();
}

// hover.exe:0x421520
// FUNCTION: HOVER 0x421520
CTempWallPod::CTempWallPod()
{
	char szName[0x34];

	m_dwUnkD4 = 0;
	m_dwUnk28 = 0;
	m_dwUnk94 = 0;
	m_dwUnk9C = 0x40;
	m_pUnk8C = NULL;
	wsprintfA(szName, "%s%02d", "POD07_", 0);
	g_mapUnk_4A1D30.Lookup(szName, (CObject*&)m_pUnk8C);
}

// The destructor is defined in the class declaration, so the compiler folds
// it into the scalar deleting destructor that the vtable slot holds - which
// is what hover.exe:0x4215D0 is. The block below emits no code; it only gives
// the reccmp marker a declaration to name ??_GCTempWallPod@@ (rules 34/184).
// hover.exe:0x4215D0
#if 0
// FUNCTION: HOVER 0x4215d0
CTempWallPod::~CTempWallPod()
{
}
#endif

// Per-frame think (slot 7): count the temporary wall's lifetime down,
// drive the dashboard gauge while this pod is the player's active wall,
// and put the pod back into the maze when it expires.
// hover.exe:0x42B070
// FUNCTION: HOVER 0x42b070
void CTempWallPod::Sub_42AD30()
{
	CPlayer* pPlayer;
	CSub42AE90 fix;
	CSub42AE90 fixOut;
	CSub422C80* pGauge;

	if (m_dwUnk5C == 0 && m_dwUnk28 != 0)
	{
		pPlayer = (CPlayer*)m_dwUnk88;
		m_dwUnkA0--;
		if (pPlayer->m_dwUnk0130 == (DWORD)this)
		{
			fix = CSub42AE90((WORD)m_dwUnkA0, 0);
			*(long*)&fixOut = *(long*)&fix / (short)theApp.m_dwUnk0164;
			pGauge = &Hover_sub_401140()->m_obj0400;
		HOVER_INLINE_SUB_42A9F0(pGauge, fixOut);
		}
		if (m_dwUnkA0 == 0)
		{
			if (((CPlayer*)m_dwUnk88)->m_dwUnk0130 == (DWORD)this)
				((CPlayer*)m_dwUnk88)->m_dwUnk0130 = 0;
			theApp.m_pBumperDoc->Sub_412630(this);
			return;
		}
	}
	CPodObj::Sub_42AD30();
}

// Player touched the pod (slot 8). While the wall is up this is the wall
// itself being hit; otherwise the pod is added to the player's stock (up
// to nine) and a fresh one is dropped elsewhere.
// hover.exe:0x42B160
// FUNCTION: HOVER 0x42b160
void CTempWallPod::Sub_42AD40(CPlayer* pOther, DWORD dwB)
{
	int nHit;
	WORD wTop;
	CBumperView* pView;
	CTempWallPod* pNew;

	if (m_dwUnk28 != 0)
	{
		if (m_dwUnk2C != 0)
		{
			nHit = pOther->Sub_41A6B0(this, dwB);
			if (pOther->m_dwUnk01F8 != 0)
			{
				Hover_sub_401140()->m_dwUnk0518 = 1;
				if (nHit != 0 && theApp.m_dwUnk0408 != 0)
					Sub_40D6E0(0x12, 3, 5, 0);
			}
		}
		else
		{
			wTop = (WORD)(((CMerlinDynamic*)m_dwUnkD4)->m_wUnk56 + 0x40);
			if (((long)(short)wTop << 16) > (long)*(DWORD*)&pOther->m_awUnk0090[8])
			{
				pOther->m_dwUnk013C = 1;
				pOther->m_awUnk0090[8] = 0;
				pOther->m_awUnk0090[9] = wTop;
			}
		}
		return;
	}

	// the "no more pods" message is the cold arm at the end of the
	// function in hover.exe, so it has to be the `else`
	if (pOther->m_obList00D0.GetCount() < 9)
	{
		CPodObj::Sub_42AD40(pOther, dwB);
		m_dwUnk88 = (DWORD)pOther;
		m_dwUnk94 = 0;
		((CCollider*)theApp.m_pBumperDoc->m_dwUnk8388)->Sub_40E710(this);
		if (((CPlayer*)m_dwUnk88)->m_dwUnk01F8 != 0 && theApp.m_dwUnk0408 != 0)
			Sub_40D6E0(8, 1, 5, 0);
		((CPlayer*)m_dwUnk88)->m_obList00D0.AddHead(this);
		pView = Hover_sub_401140();
		pView->m_obj0290.m_dwUnk00 = 1;
		pView->m_obj0290.m_dwUnk08 = ((CPlayer*)m_dwUnk88)->m_obList00D0.GetCount();
		pNew = new CTempWallPod;
		theApp.m_pBumperDoc->Sub_412250(pNew, (rand() % 4) == 0);
	}
	else if (pOther->m_dwUnk01F8 != 0 && theApp.m_dwUnk0408 != 0)
	{
		Sub_40D6E0(5, 1, 5, 0);
	}
}

// Deploy (slot 9): raise the "FORCE_FIELD" wall across the player's
// path. The wall is a CMerlinDynamic sprite built from the player's position
// and heading, registered with both the collider and the level world.
// hover.exe:0x42B3B0
// FUNCTION: HOVER 0x42b3b0
void CTempWallPod::Sub_42AD50(DWORD dwUnk)
{
	CPlayer* pPlayer;
	CMerlinDynamic* pSprite;
	POSITION pos;
	WORD wBase;
	long lAngle;
	DWORD adwPos[2];
	DWORD adwPt1[2];
	DWORD adwPt2[2];
	DWORD adwPt3[2];
	DWORD dwDirX;
	DWORD dwDirY;
	DWORD dwSideX;
	DWORD dwSideY;

	if (theApp.m_dwUnk0408 != 0)
		Sub_40D6E0(0x19, 4, 5, 0);
	theApp.m_pBumperDoc->m_list83A8.AddTail(this);
	m_dwUnk28 = 1;
	m_dwUnkA0 = theApp.m_dwUnk0164;
	pPlayer = (CPlayer*)m_dwUnk88;
	{
		DWORD* pdwUnk0130 = &pPlayer->m_dwUnk0130;
		if (*pdwUnk0130 != 0)
			*pdwUnk0130 = 0;
	}
	Hover_sub_401140()->m_obj0400.Sub_42A9F0(CSub42AE90(1.0));

	pSprite = new CMerlinDynamic;
	g_mapUnk_4A1D30.Lookup("FORCE_FIELD", (CObject*&)pSprite->m_pUnk50);
	m_dwUnkD4 = (DWORD)pSprite;
	pSprite->m_wUnk58 = 0x190;
	pSprite->m_dwUnk64 = 0;
	pPlayer = (CPlayer*)m_dwUnk88;
	wBase = (WORD)pPlayer->m_dwUnk01A4 - 0x40;
	pSprite->m_wUnk54 = wBase;
	pSprite->m_wUnk56 = wBase + 0x100;
	pSprite->m_wUnk5A = (WORD)((CPlayer*)m_dwUnk88)->Sub_403880(adwPt1)[0];
	pSprite->m_wUnk5C = (WORD)((CPlayer*)m_dwUnk88)->Sub_403880(adwPt2)[1];

	pPlayer = (CPlayer*)m_dwUnk88;
	lAngle = (short)pPlayer->m_awUnk0194[5];
	dwDirX = g_adwSinTable[(((lAngle + 0x80) & 0x1FF) << 4) + 1];
	HOVER_FIXMULWP(dwDirX, 0x96);
	dwDirY = g_adwSinTable[((lAngle & 0x1FF) << 4) + 1];
	HOVER_FIXMULWP(dwDirY, 0x96);
	lAngle = (short)theApp.m_pBumperDoc->m_humanPlayer.m_awUnk0194[5];
	dwSideX = g_adwSinTable[((lAngle & 0x1FF) << 4) + 1];
	HOVER_FIXMULWP(dwSideX, 0xC8);
	dwSideY = g_adwSinTable[(((lAngle - 0x80) & 0x1FF) << 4) + 1];
	HOVER_FIXMULWP(dwSideY, 0xC8);

	pPlayer->Sub_4038A0(adwPos);
	adwPos[0] -= dwDirX;
	adwPos[1] -= dwDirY;
	adwPt2[0] = adwPos[0];
	adwPt2[1] = adwPos[1];
	adwPos[0] += dwSideX;
	adwPos[1] += dwSideY;
	adwPt2[0] -= dwSideX;
	adwPt2[1] -= dwSideY;
	pSprite->m_dwX1 = ((short*)&adwPos[0])[1];
	adwPt1[0] = pSprite->m_dwX1;
	pSprite->m_dwY1 = ((short*)&adwPos[1])[1];
	adwPt1[1] = pSprite->m_dwY1;
	pSprite->m_dwX2 = ((short*)&adwPt2[0])[1];
	adwPt3[0] = pSprite->m_dwX2;
	pSprite->m_dwY2 = ((short*)&adwPt2[1])[1];
	adwPt3[1] = pSprite->m_dwY2;
	pSprite->Sub_4010C0();
	m_obj04.Sub_422BF0(adwPt1, adwPt3);
	pSprite->m_dwUnk60 = 0;
	((CCollider*)theApp.m_pBumperDoc->m_dwUnk8388)->Sub_40F900(this);
	((CMerlinWorld*)theApp.m_pBumperDoc->m_dwUnk0188)->Sub_4128A0((DWORD)pSprite);
	((CPlayer*)m_dwUnk88)->m_dwUnk0130 = (DWORD)this;
	pos = ((CPlayer*)m_dwUnk88)->m_obList00D0.Find(this);
	((CPlayer*)m_dwUnk88)->m_obList00D0.RemoveAt(pos);
	Sub_408530();
}

// Register with the collider (slot 12): while the wall is up, the cells
// covered by its sprite are claimed instead of the pod's own radius.
// hover.exe:0x421640
// FUNCTION: HOVER 0x421640
int CTempWallPod::Sub_408820(DWORD dwA)
{
	CMerlinDynamic* pSprite;
	int nCell;
	int nHalf;
	int nX;
	int nY;
	int nX1;
	int nX2;
	int nY1;
	int nY2;
	int x;
	int y;
	CCollider* pMgr;

	// while the force field is up the pod claims the field's cells; the
	// plain pod registration is the tail of the function (rule 25)
	pMgr = (CCollider*)dwA;
	if (m_dwUnk28 != 0)
	{
		nCell = (int)pMgr->m_dwUnk0C;
		pSprite = (CMerlinDynamic*)m_dwUnkD4;
		nHalf = (short)pSprite->m_wUnk58;
		nX = (short)pSprite->m_wUnk5A;
		nY = (short)pSprite->m_wUnk5C;
		nX2 = (nX + nHalf) / nCell;
		nY1 = (nY - nHalf) / nCell;
		nY2 = (nY + nHalf) / nCell;
		nX1 = (nX - nHalf) / nCell;
		for (x = nX1; x <= nX2; x++)
		{
			for (y = nY1; y <= nY2; y++)
			{
				if (pMgr->m_dwUnk44 != 0)
					pMgr->Sub_40F8A0(x, y);
				else
					pMgr->m_obj10.Sub_4086A0(x, y, this);
			}
		}
		return 1;
	}
	return CPodObj::Sub_408820((DWORD)pMgr);
}

// Wall collision test (slot 11): while the wall is up, the player is
// tested against the sprite's height band and the pod's own segment;
// otherwise the CPodObj pickup test applies.
// hover.exe:0x41A3A0
// FUNCTION: HOVER 0x41a3a0
int CTempWallPod::Sub_42AD60(DWORD dwUnk)
{
	CPlayer* pPlayer;
	CMerlinDynamic* pSprite;
	DWORD* pdwPts;
	long lZ;
	long lTop;
	long lBottom;
	long lTopEdge;
	DWORD adwPos[2];
	DWORD dwOutA;
	DWORD dwOutB;
	DWORD dwOutC;

	// the raised force field is tested here; without it the pod is an
	// ordinary pickup, and the base test is the tail of the function
	// (notes/perfect-match.md rule 25)
	pPlayer = (CPlayer*)dwUnk;
	if (m_dwUnk28 != 0)
	{
		pdwPts = pPlayer->Sub_4089E0();
		pSprite = (CMerlinDynamic*)m_dwUnkD4;
		lBottom = (short)pSprite->m_wUnk54;
		lTopEdge = (short)pSprite->m_wUnk56;
		lZ = (long)pPlayer->m_dwUnk01A4;
		lTop = lZ + 0x40;
		if (lTopEdge >= lZ)
		{
			if (lBottom > lTop)
				return 0;
			// one shared body: the original's first test jumps
			// straight into the success tail (rule 38)
			if (m_obj04.Sub_40C9B0(pdwPts, &dwOutC, &dwOutB, &dwOutA) <=
					(long)pdwPts[4] ||
				m_obj04.Sub_40C9B0(pdwPts + 2, &dwOutC, &dwOutB,
					&dwOutA) <= (long)pdwPts[4])
			{
				m_dwUnk2C = 1;
				return 1;
			}
		}
		// the field top is below the player: only a brush is possible
		else if (m_obj04.Sub_40C9B0(pdwPts + 2, &dwOutC, &dwOutB, &dwOutA) <=
					(long)pdwPts[4] ||
				m_obj04.Sub_40C9B0(pPlayer->Sub_403880(adwPos), &dwOutC,
					&dwOutB, &dwOutA) <= (long)pdwPts[4] ||
				m_obj04.Sub_40C9B0(pdwPts, &dwOutC, &dwOutB, &dwOutA) <=
					(long)pdwPts[4])
		{
			m_dwUnk2C = 0;
			return 1;
		}
		return 0;
	}
	return CPodObj::Sub_42AD60((DWORD)pPlayer);
}

// hover.exe:0x421430
// FUNCTION: HOVER 0x421430
void CTempWallPod::Sub_4087D0()
{
	CPodObj::Sub_4087D0();
}

// End of frame (slot 16): drop the wall sprite from the level world
// once the pod has been released.
// hover.exe:0x4214E0
// FUNCTION: HOVER 0x4214e0
void CTempWallPod::Sub_4087F0()
{
	if (m_dwUnk60 != 0 && m_dwUnkD4 != 0)
	{
		((CMerlinWorld*)theApp.m_pBumperDoc->m_dwUnk0188)->Sub_414300(m_dwUnkD4);
		m_dwUnkD4 = 0;
	}
	CPodObj::Sub_4087F0();
}

// hover.exe:0x42B690
// FUNCTION: HOVER 0x42b690
void CTempWallPod::Sub_413020()
{
	CGameObj::Sub_413020();
}

// hover.exe:0x421710
// FUNCTION: HOVER 0x421710
CSpeedPod::CSpeedPod()
{
	char szName[0x34];

	m_dwUnk28 = 0;
	m_dwUnk94 = 0;
	m_pUnk8C = NULL;
	m_dwUnk9C = 0x40;
	wsprintfA(szName, "%s%02d", "POD02_", 0);
	g_mapUnk_4A1D30.Lookup(szName, (CObject*&)m_pUnk8C);
}

// The destructor is defined in the class declaration, so the compiler folds
// it into the scalar deleting destructor that the vtable slot holds - which
// is what hover.exe:0x41A510 is. The block below emits no code; it only gives
// the reccmp marker a declaration to name ??_GCSpeedPod@@ (rules 34/184).
// hover.exe:0x41A510
#if 0
// FUNCTION: HOVER 0x41a510
CSpeedPod::~CSpeedPod()
{
}
#endif

// Per-frame think (slot 7): count the speed boost down, drive the
// dashboard gauge, and restore the player's normal top speed when it
// expires.
// hover.exe:0x414370
// FUNCTION: HOVER 0x414370
void CSpeedPod::Sub_42AD30()
{
	CSub42AE90 fix;
	CSub42AE90 fixOut;
	CSub422C80* pGauge;
	CPlayer* pPlayer;

	if (m_dwUnk5C == 0 && m_dwUnk28 != 0)
	{
		m_dwUnkA0--;
		fix = CSub42AE90((WORD)m_dwUnkA0, 0);
		*(long*)&fixOut = *(long*)&fix / (short)theApp.m_dwUnk0158;
		pGauge = &Hover_sub_401140()->m_obj0370;
		HOVER_INLINE_SUB_42A9F0(pGauge, fixOut);
		if ((long)m_dwUnkA0 <= 0)
		{
			if (theApp.m_dwUnk0408 != 0)
				Sub_40D6E0(0xE, 1, 5, 0);
			pPlayer = (CPlayer*)m_dwUnk88;
			pPlayer->m_dwUnk0128 = 0;
			*(DWORD*)&pPlayer->m_awUnk01D0[2] = *(DWORD*)&pPlayer->m_awUnk01D0[4];
			theApp.m_pBumperDoc->Sub_412630(this);
			return;
		}
	}
	CPodObj::Sub_42AD30();
}

// Player touched the pod (slot 8): raise the player's speed limit for
// theApp.m_dwUnk0158 frames, cancelling any slow pod that was running.
// hover.exe:0x421070
// FUNCTION: HOVER 0x421070
void CSpeedPod::Sub_42AD40(CPlayer* pOther, DWORD dwB)
{
	CPlayer* pPlayer;
	CSpeedPod* pNew;
	DWORD dwLimit;
	DWORD dwScale;

	CPodObj::Sub_42AD40(pOther, dwB);
	m_dwUnk88 = (DWORD)pOther;
	m_dwUnk94 = 0;
	((CCollider*)theApp.m_pBumperDoc->m_dwUnk8388)->Sub_40E710(this);
	pPlayer = (CPlayer*)m_dwUnk88;
	if (pPlayer->m_dwUnk01F8 != 0 && theApp.m_dwUnk0408 != 0)
		Sub_40D6E0(0xD, 1, 5, 0);
	if (m_dwUnk98 != 0)
	{
		pNew = new CSpeedPod;
		theApp.m_pBumperDoc->Sub_412250(pNew, (rand() % 4) == 0);
	}
	theApp.m_pBumperDoc->m_list83A8.AddTail(this);
	m_dwUnk28 = 1;
	m_dwUnkA0 = theApp.m_dwUnk0158;
	pPlayer = (CPlayer*)m_dwUnk88;
	if (pPlayer->m_dwUnk0128 != 0)
	{
		theApp.m_pBumperDoc->Sub_412630((CGameObj*)pPlayer->m_dwUnk0128);
		((CPlayer*)m_dwUnk88)->m_dwUnk0128 = 0;
	}
	if (((CPlayer*)m_dwUnk88)->m_dwUnk012C != 0)
	{
		theApp.m_pBumperDoc->Sub_412630(
			(CGameObj*)((CPlayer*)m_dwUnk88)->m_dwUnk012C);
		((CPlayer*)m_dwUnk88)->m_dwUnk012C = 0;
	}
	pPlayer = (CPlayer*)m_dwUnk88;
	dwLimit = *(DWORD*)&pPlayer->m_awUnk01D0[4];
	dwScale = ((DWORD*)pPlayer->m_dwUnk0084)[8];
	pPlayer->m_dwUnk0128 = (DWORD)this;
	HOVER_FIXMULP(dwLimit, dwScale);
	*(DWORD*)&pPlayer->m_awUnk01D0[2] = dwLimit;
}

// hover.exe:0x414A40
// FUNCTION: HOVER 0x414a40
void CSpeedPod::Sub_4087D0()
{
	CPodObj::Sub_4087D0();
}

// hover.exe:0x414A70
// FUNCTION: HOVER 0x414a70
void CSpeedPod::Sub_4087F0()
{
	CPodObj::Sub_4087F0();
}

// hover.exe:0x421700
// FUNCTION: HOVER 0x421700
void CSpeedPod::Sub_413020()
{
	CGameObj::Sub_413020();
}

// hover.exe:0x4223C0
// FUNCTION: HOVER 0x4223c0
CMapEraserPod::CMapEraserPod()
{
	char szName[0x34];

	m_dwUnk88 = 0;
	m_dwUnk28 = 0;
	m_dwUnk94 = 0;
	m_dwUnk9C = 0x40;
	m_pUnk8C = NULL;
	wsprintfA(szName, "%s%02d", "POD05_", 0);
	g_mapUnk_4A1D30.Lookup(szName, (CObject*&)m_pUnk8C);
}

// The destructor is defined in the class declaration, so the compiler folds
// it into the scalar deleting destructor that the vtable slot holds - which
// is what hover.exe:0x41B350 is. The block below emits no code; it only gives
// the reccmp marker a declaration to name ??_GCMapEraserPod@@ (rules 34/184).
// hover.exe:0x41B350
#if 0
// FUNCTION: HOVER 0x41b350
CMapEraserPod::~CMapEraserPod()
{
}
#endif

// hover.exe:0x42C410
// FUNCTION: HOVER 0x42c410
void CMapEraserPod::Sub_42AD30()
{
	CPodObj::Sub_42AD30();
}

// Player touched the pod (slot 8): clear the "seen on the map" bit of
// every non-pod object the document still tracks, then release the pod.
// hover.exe:0x422470
// FUNCTION: HOVER 0x422470
void CMapEraserPod::Sub_42AD40(CPlayer* pOther, DWORD dwB)
{
	CMapEraserPod* pNew;
	POSITION pos;
	CGameObj* pObj;

	CPodObj::Sub_42AD40(pOther, dwB);
	m_dwUnk94 = 0;
	m_dwUnk88 = (DWORD)pOther;
	((CCollider*)theApp.m_pBumperDoc->m_dwUnk8388)->Sub_40E710(this);
	if (m_dwUnk98 != 0)
	{
		pNew = new CMapEraserPod;
		theApp.m_pBumperDoc->Sub_412250(pNew, (rand() % 4) == 0);
	}
	if (((CPlayer*)m_dwUnk88)->m_dwUnk0140 == 0)
	{
		if (theApp.m_dwUnk0408 != 0)
			Sub_40D6E0(0xF, 1, 5, 0);
		pos = theApp.m_pBumperDoc->m_list838C.GetHeadPosition();
		while (pos != NULL)
		{
			pObj = (CGameObj*)theApp.m_pBumperDoc->m_list838C.GetNext(pos);
			if (!pObj->IsKindOf(RUNTIME_CLASS(CPodObj)) && pObj->m_dwUnk78 != 0)
				// hover.exe clears the flag with a byte-wide AND
				*(BYTE*)&((CMerlinLine*)pObj->m_dwUnk78)->m_wFlags &= ~8;
			if (pObj->IsKindOf(RUNTIME_CLASS(CWall)) && pObj->m_dwUnk78 != 0)
				// hover.exe clears the flag with a byte-wide AND
				*(BYTE*)&((CMerlinLine*)pObj->m_dwUnk78)->m_wFlags &= ~8;
		}
	}
	theApp.m_pBumperDoc->Sub_412630(this);
}

// hover.exe:0x41B2C0
// FUNCTION: HOVER 0x41b2c0
void CMapEraserPod::Sub_4087D0()
{
	CPodObj::Sub_4087D0();
}

// hover.exe:0x41B340
// FUNCTION: HOVER 0x41b340
void CMapEraserPod::Sub_4087F0()
{
	CPodObj::Sub_4087F0();
}

// hover.exe:0x422620
// FUNCTION: HOVER 0x422620
void CMapEraserPod::Sub_413020()
{
	CGameObj::Sub_413020();
}

// hover.exe:0x422630
// FUNCTION: HOVER 0x422630
CSlowPod::CSlowPod()
{
	char szName[0x34];

	m_dwUnk28 = 0;
	m_dwUnk94 = 0;
	m_pUnk8C = NULL;
	m_dwUnk9C = 0x40;
	wsprintfA(szName, "%s%02d", "POD03_", 0);
	g_mapUnk_4A1D30.Lookup(szName, (CObject*&)m_pUnk8C);
}

// The destructor is defined in the class declaration, so the compiler folds
// it into the scalar deleting destructor that the vtable slot holds - which
// is what hover.exe:0x41B3E0 is. The block below emits no code; it only gives
// the reccmp marker a declaration to name ??_GCSlowPod@@ (rules 34/184).
// hover.exe:0x41B3E0
#if 0
// FUNCTION: HOVER 0x41b3e0
CSlowPod::~CSlowPod()
{
}
#endif

// Per-frame think (slot 7): count the slow-down timer down, drive the
// dashboard gauge and restore the player's speed limit when it expires.
// hover.exe:0x42C430
// FUNCTION: HOVER 0x42c430
void CSlowPod::Sub_42AD30()
{
	CSub42AE90 fix;
	CSub42AE90 fixOut;
	CSub422C80* pGauge;
	CPlayer* pPlayer;

	if (m_dwUnk5C == 0 && m_dwUnk28 != 0)
	{
		m_dwUnkA0--;
		fix = CSub42AE90((WORD)m_dwUnkA0, 0);
		*(long*)&fixOut = *(long*)&fix / (short)theApp.m_dwUnk0160;
		pGauge = &Hover_sub_401140()->m_obj0340;
		HOVER_INLINE_SUB_42A9F0(pGauge, fixOut);
		if ((long)m_dwUnkA0 <= 0)
		{
			if (theApp.m_dwUnk0408 != 0)
				Sub_40D6E0(0xC, 1, 5, 0);
			pPlayer = (CPlayer*)m_dwUnk88;
			pPlayer->m_dwUnk012C = 0;
			*(DWORD*)&pPlayer->m_awUnk01D0[2] = *(DWORD*)&pPlayer->m_awUnk01D0[4];
			theApp.m_pBumperDoc->Sub_412630(this);
			return;
		}
	}
	CPodObj::Sub_42AD30();
}

// Player touched the pod (slot 8): slow the player down for
// theApp.m_dwUnk0160 frames, cancelling any speed pod that was running.
// hover.exe:0x4226D0
// FUNCTION: HOVER 0x4226d0
void CSlowPod::Sub_42AD40(CPlayer* pOther, DWORD dwB)
{
	CSlowPod* pNew;
	CPlayer* pPlayer;
	DWORD dwLimit;
	DWORD dwScale;

	CPodObj::Sub_42AD40(pOther, dwB);
	m_dwUnk94 = 0;
	m_dwUnk88 = (DWORD)pOther;
	((CCollider*)theApp.m_pBumperDoc->m_dwUnk8388)->Sub_40E710(this);
	if (m_dwUnk98 != 0)
	{
		pNew = new CSlowPod;
		theApp.m_pBumperDoc->Sub_412250(pNew, (rand() % 4) == 0);
	}
	pPlayer = (CPlayer*)m_dwUnk88;
	if (pPlayer->m_dwUnk0140 != 0)
	{
		theApp.m_pBumperDoc->Sub_412630(this);
		return;
	}
	if (pPlayer->m_dwUnk01F8 != 0 && theApp.m_dwUnk0408 != 0)
		Sub_40D6E0(0xB, 1, 5, 0);
	theApp.m_pBumperDoc->m_list83A8.AddTail(this);
	m_dwUnk28 = 1;
	m_dwUnkA0 = theApp.m_dwUnk0160;
	pPlayer = (CPlayer*)m_dwUnk88;
	if (pPlayer->m_dwUnk012C != 0)
	{
		theApp.m_pBumperDoc->Sub_412630((CGameObj*)pPlayer->m_dwUnk012C);
		((CPlayer*)m_dwUnk88)->m_dwUnk012C = 0;
	}
	if (((CPlayer*)m_dwUnk88)->m_dwUnk0128 != 0)
	{
		theApp.m_pBumperDoc->Sub_412630(
			(CGameObj*)((CPlayer*)m_dwUnk88)->m_dwUnk0128);
		((CPlayer*)m_dwUnk88)->m_dwUnk0128 = 0;
		Hover_sub_401140()->m_obj0370.Sub_42A9F0(CSub42AE90(0, 0));
	}
	pPlayer = (CPlayer*)m_dwUnk88;
	dwLimit = *(DWORD*)&pPlayer->m_awUnk01D0[4];
	dwScale = ((DWORD*)pPlayer->m_dwUnk0084)[7];
	pPlayer->m_dwUnk012C = (DWORD)this;
	HOVER_FIXMULP(dwLimit, dwScale);
	*(DWORD*)&pPlayer->m_awUnk01D0[2] = dwLimit;
}

// hover.exe:0x41B3C0
// FUNCTION: HOVER 0x41b3c0
void CSlowPod::Sub_4087D0()
{
	CPodObj::Sub_4087D0();
}

// hover.exe:0x41B3D0
// FUNCTION: HOVER 0x41b3d0
void CSlowPod::Sub_4087F0()
{
	CPodObj::Sub_4087F0();
}

// hover.exe:0x422910
// FUNCTION: HOVER 0x422910
void CSlowPod::Sub_413020()
{
	CGameObj::Sub_413020();
}

/////////////////////////////////////////////////////////////////////////////
// CFlag

// hover.exe:0x421CD0
// FUNCTION: HOVER 0x421cd0
CFlag::CFlag(DWORD dwUnk)
{
	char szName[0x34];

	m_dwUnk88 = 0;
	m_dwUnk9C = 0x80;
	m_pUnk8C = NULL;
	m_dwUnkCC = dwUnk;
	if (dwUnk == 0)
		wsprintfA(szName, "%s%02d", "FLG01_", 0);
	else
		wsprintfA(szName, "%s%02d", "FLG00_", 0);
	g_mapUnk_4A1D30.Lookup(szName, (CObject*&)m_pUnk8C);
	m_dwUnkB8 = (short)m_pUnk8C->m_wUnk18 * 2;
	m_dwUnkBC = (short)m_pUnk8C->m_wUnk14;
}

// The destructor is defined in the class declaration, so the compiler folds
// it into the scalar deleting destructor that the vtable slot holds - which
// is what hover.exe:0x421DB0 is. The block below emits no code; it only gives
// the reccmp marker a declaration to name ??_GCFlag@@ (rules 34/184).
// hover.exe:0x421DB0
#if 0
// FUNCTION: HOVER 0x421db0
CFlag::~CFlag()
{
}
#endif

// hover.exe:0x42C160
// FUNCTION: HOVER 0x42c160
void CFlag::Sub_42AD30()
{
	CPodObj::Sub_42AD30();
}

// The flag is captured (slot 8): it joins the player's flag list, the
// score/flag gauges are refreshed and the level-complete state is entered
// when the last flag of the level has been taken.
// hover.exe:0x41AE40
// FUNCTION: HOVER 0x41ae40
void CFlag::Sub_42AD40(CPlayer* pOther, DWORD dwB)
{
	CRobotPlayer* pRobot;
	CBumperDoc* pDoc;
	CBumperView* pView;
	long lLimit;
	long lCount;

	CPodObj::Sub_42AD40(pOther, dwB);
	m_dwUnk88 = (DWORD)pOther;
	m_dwUnk94 = 0;
	((CCollider*)theApp.m_pBumperDoc->m_dwUnk8388)->Sub_40E710(this);
	((CPlayer*)m_dwUnk88)->m_obList00B4.AddHead(this);

	if (m_dwUnkCC == 0)
	{
		// a robot took the human flag
		pRobot = (CRobotPlayer*)m_dwUnk88;
		if (pRobot->m_dwUnk01FC == (DWORD)this)
		{
			pRobot->m_dwUnk01FC = 0;
			pRobot->m_pUnk0220->Sub_41BC10((CSub423670*)&pRobot->m_obj0264);
		}
		pDoc = theApp.m_pBumperDoc;
		pDoc->m_dwUnk83E8++;
		pView = Hover_sub_401140();
		{
			CSub422C30* pGauge = &pView->m_obj0480;
			lLimit = (long)pGauge->m_dwUnk3C;
			pGauge->m_dwUnk00 = 1;
			lCount = (long)pDoc->m_dwUnk83E8;
			if (lLimit <= lCount)
			{
				pGauge->m_dwUnk44 = lLimit;
			}
			else
			{
				pGauge->m_dwUnk44 = 0;
				if (lCount > 0)
					pGauge->m_dwUnk44 = lCount;
			}
		}
		if (LVLPARAM((int)g_dwUnk_46049C).m_dwUnk30 ==
			theApp.m_pBumperDoc->m_dwUnk83E8)
		{
			Hover_sub_401140()->Sub_427CF0(4);
			if (theApp.m_dwUnk0404 != 0)
				((CSub40B800*)theApp.m_pBumperDoc->m_adwUnk83EC[0x1A])->
					Sub_4118A0(5, 1);
			if (theApp.m_dwUnk0408 != 0)
				((CSub40B800*)theApp.m_pBumperDoc->m_adwUnk83EC[0x20])->
					Sub_40B800(8, 5, 0);
			return;
		}
		if (theApp.m_dwUnk0408 != 0)
			Sub_40D6E0(0x10, 2, 5, 0);
		return;
	}

	// the human took a robot flag
	pDoc = theApp.m_pBumperDoc;
	pDoc->m_dwUnk83E4 +=
		(&LVLPARAM((int)g_dwUnk_46049C).m_dwUnk40)[theApp.m_dwUnk01C8];
	pView = Hover_sub_401140();
	{
		CSub422C30* pGauge = &pView->m_obj0430;
		pGauge->m_dwUnk00 = 1;
		lCount = (long)((CPlayer*)m_dwUnk88)->m_obList00B4.GetCount();
		lLimit = (long)pGauge->m_dwUnk3C;
		if (lCount >= lLimit)
		{
			pGauge->m_dwUnk44 = lLimit;
		}
		else
		{
			pGauge->m_dwUnk44 = 0;
			if (lCount > 0)
				pGauge->m_dwUnk44 = lCount;
		}
	}
	lCount = (long)theApp.m_pBumperDoc->m_dwUnk83E4;
	pView = Hover_sub_401140();
	pView->m_obj02B8.m_dwUnk00 = 1;
	pView->m_obj02B8.m_dwUnk08 = lCount;
	if (LVLPARAM((int)g_dwUnk_46049C).m_dwUnk30 ==
		(DWORD)((CPlayer*)m_dwUnk88)->m_obList00B4.GetCount())
	{
		Hover_sub_401140()->Sub_427CF0(5);
		if (theApp.m_dwUnk0404 != 0)
			((CSub40B800*)theApp.m_pBumperDoc->m_adwUnk83EC[0x1A])->
				Sub_4118A0(5, 1);
		if (theApp.m_dwUnk0408 != 0)
			((CSub40B800*)theApp.m_pBumperDoc->m_adwUnk83EC[0x1F])->
				Sub_40B800(8, 5, 0);
		return;
	}
	if (theApp.m_dwUnk0408 != 0)
		Sub_40D6E0(0x11, 2, 5, 0);
}

// Pickup test (slot 11): a flag can only be picked up by the other team
// (m_dwUnkCC 0 = human flag, 1 = robot flag).
// hover.exe:0x414470
// FUNCTION: HOVER 0x414470
int CFlag::Sub_42AD60(DWORD dwUnk)
{
	// two independent tests, not an if/else chain: the original re-reads
	// m_dwUnkCC for the second one (there is a call in between, rule 65)
	if (m_dwUnkCC == 0 &&
		((CPlayer*)dwUnk)->IsKindOf(RUNTIME_CLASS(CHumanPlayer)))
		return 0;
	if (m_dwUnkCC == 1 &&
		((CPlayer*)dwUnk)->IsKindOf(RUNTIME_CLASS(CRobotPlayer)))
		return 0;
	return CPodObj::Sub_42AD60(dwUnk);
}

// hover.exe:0x414D00
// FUNCTION: HOVER 0x414d00
void CFlag::Sub_4087D0()
{
	CPodObj::Sub_4087D0();
}

// hover.exe:0x414D10
// FUNCTION: HOVER 0x414d10
void CFlag::Sub_4087F0()
{
	CPodObj::Sub_4087F0();
}

// hover.exe:0x42C170
// FUNCTION: HOVER 0x42c170
void CFlag::Sub_413020()
{
	CGameObj::Sub_413020();
}

/////////////////////////////////////////////////////////////////////////////
// CContainer

// hover.exe:0x41B160
// FUNCTION: HOVER 0x41b160
CContainer::CContainer(CPodObj* pObj, DWORD dwX, DWORD dwY, DWORD dwPt)
{
	char szName[0x34];

	m_dwUnkD0 = (DWORD)pObj;
	Sub_40F880(dwX, dwY);
	m_dwUnk9C = dwPt;
	// re-read through the member, not through the parameter: the original
	// keeps a pointer to m_dwUnkD0 live across Sub_40F880 (rule 65)
	m_dwUnkA4 = ((CPodObj*)m_dwUnkD0)->m_dwUnkA4;
	m_pUnk8C = NULL;
	wsprintfA(szName, "%s%02d", "POD06_", 0);
	g_mapUnk_4A1D30.Lookup(szName, (CObject*&)m_pUnk8C);
}

// Destroys the pod nested inside the container. The body is folded into
// the scalar deleting destructor the vtable slot holds, so the marker
// pairs our ??1 against the original's ??_G (rule 34).
// hover.exe:0x41B230
// The destructor is defined in the class declaration, so the compiler folds
// it into the scalar deleting destructor that the vtable slot holds - which
// is what hover.exe:0x41B230 is. The block below emits no code; it only gives
// the reccmp marker a declaration to name ??_GCContainer@@ (rules 34/184).
// hover.exe:0x41B230
#if 0
// FUNCTION: HOVER 0x41b230
CContainer::~CContainer()
{
	if (m_dwUnkD0 != 0)
		delete (CGameObj*)m_dwUnkD0;
}
#endif

// Player touched the container (slot 8): hand the touch on to the pod
// nested inside it and release the container, unless the player's stock
// of that pod type is already full.
// hover.exe:0x421E20
// FUNCTION: HOVER 0x421e20
void CContainer::Sub_42AD40(CPlayer* pOther, DWORD dwB)
{
	CPodObj* pInner;
	int bRelease;

	pInner = (CPodObj*)m_dwUnkD0;
	if ((pInner->IsKindOf(RUNTIME_CLASS(CTempWallPod)) &&
			pOther->m_obList00D0.GetCount() == 9) ||
		(((CPodObj*)m_dwUnkD0)->IsKindOf(RUNTIME_CLASS(CSmokeBombPod)) &&
			pOther->m_obList00EC.GetCount() == 9) ||
		(((CPodObj*)m_dwUnkD0)->IsKindOf(RUNTIME_CLASS(CJumpPod)) &&
			pOther->m_obList0108.GetCount() == 9))
		bRelease = 0;
	else
		bRelease = 1;

	((CPodObj*)m_dwUnkD0)->Sub_42AD40(pOther, dwB);
	if (bRelease != 0)
	{
		m_dwUnkD0 = 0;
		theApp.m_pBumperDoc->Sub_412630(this);
	}
}

// hover.exe:0x414D20
// FUNCTION: HOVER 0x414d20
void CContainer::Sub_4087D0()
{
	CPodObj::Sub_4087D0();
}

// hover.exe:0x414D30
// FUNCTION: HOVER 0x414d30
void CContainer::Sub_4087F0()
{
	CPodObj::Sub_4087F0();
}

// hover.exe:0x421EC0
// FUNCTION: HOVER 0x421ec0
void CContainer::Sub_413020()
{
	CGameObj::Sub_413020();
}

/////////////////////////////////////////////////////////////////////////////
// CWall

// hover.exe:0x40E6A0
// FUNCTION: HOVER 0x40e6a0
CWall::CWall()
{
	m_dwUnk9C = 0;
}

// hover.exe:0x414D40
// FUNCTION: HOVER 0x414d40
int CWall::Sub_414D40(CObject* pTexture)
{
	int nRet = CGameObj::Sub_414A50((DWORD)pTexture);
	m_obj04.Sub_40E8E0(pTexture);
	return nRet;
}

// The class destructor has an empty body; only CGameObj::~CGameObj runs.
// Unlike the pod classes, CWall keeps a separate out-of-line ??1 (the
// collider builds CWall objects on the stack, see CCollider::Sub_40C2B0
// and Sub_411600, so the plain destructor is needed); the scalar deleting
// destructor at hover.exe:0x417BA0 is a second copy of this body followed
// by an inlined CWall::operator delete.
// hover.exe:0x40E650
// FUNCTION: HOVER 0x40e650
CWall::~CWall()
{
}

// hover.exe:0x42C660
// FUNCTION: HOVER 0x42c660
void CWall::Sub_42AD30()
{
	CGameObj::Sub_42AD30();
}

// The player hit the wall (slot 8): bounce him off it, or - when the
// wall is low enough to stand on - lift him onto its top edge.
// hover.exe:0x41B450
// FUNCTION: HOVER 0x41b450
void CWall::Sub_42AD40(CPlayer* pOther, DWORD dwB)
{
	int nHit;
	WORD wTop;

	if (m_dwUnk2C != 0)
	{
		nHit = pOther->Sub_41A6B0(this, dwB);
		if (pOther->m_dwUnk01F8 != 0)
		{
			Hover_sub_401140()->m_dwUnk0518 = 1;
			if (nHit != 0 && theApp.m_dwUnk0408 != 0)
				Sub_40D6E0(0x12, 3, 5, 0);
		}
		return;
	}
	wTop = (WORD)(((CMerlinStatic*)m_dwUnk78)->m_wUnkA6 + 0x40);
	if (((long)(short)wTop << 16) > (long)*(DWORD*)&pOther->m_awUnk0090[8])
	{
		pOther->m_dwUnk013C = 1;
		pOther->m_awUnk0090[8] = 0;
		pOther->m_awUnk0090[9] = wTop;
	}
}

// Proximity test (slot 10): is the point (dwA,dwB) closer than dwC to
// the wall segment?
// hover.exe:0x422B90
// FUNCTION: HOVER 0x422b90
int CWall::Sub_42AD70(DWORD dwA, DWORD dwB, DWORD dwC)
{
	DWORD dwOutA;
	DWORD dwOutB;
	DWORD dwOutC;

	return m_obj04.Sub_40C9B0(&dwA, &dwOutC, &dwOutB, &dwOutA) <= (long)dwC;
}

// Player collision test (slot 11). A wall whose top is below the player
// only registers a brush (m_dwUnk2C = 0); otherwise a hit sets
// m_dwUnk2C so that Sub_42AD40 bounces him off.
// hover.exe:0x40D4B0
// FUNCTION: HOVER 0x40d4b0
int CWall::Sub_42AD60(DWORD dwUnk)
{
	CPlayer* pPlayer;
	CMerlinStatic* pLine;
	DWORD* pdwPts;
	long lZ;
	long lTop;
	long lBottom;
	long lTopEdge;
	DWORD adwPos[2];
	DWORD dwOutA;
	DWORD dwOutB;
	DWORD dwOutC;

	pPlayer = (CPlayer*)dwUnk;
	pdwPts = pPlayer->Sub_4089E0();
	lZ = (long)pPlayer->m_dwUnk01A4;
	lTop = lZ + 0x40;
	pLine = (CMerlinStatic*)m_dwUnk78;
	if (pLine == NULL)
		return 0;
	lBottom = (short)pLine->m_wUnkA4;
	lTopEdge = (short)pLine->m_wUnkA6;
	if (lTopEdge >= lZ)
	{
		if (lBottom > lTop)
			return 0;
		// the two proximity points are tested with their own success
		// return, which is what makes hover.exe emit the
		// `m_dwUnk2C = 1; return 1` tail twice
		if (m_obj04.Sub_40C9B0(pdwPts, &dwOutC, &dwOutB, &dwOutA) <=
				(long)pdwPts[4])
		{
			m_dwUnk2C = 1;
			return 1;
		}
		if (m_obj04.Sub_40C9B0(pdwPts + 2, &dwOutC, &dwOutB, &dwOutA) <=
				(long)pdwPts[4])
		{
			m_dwUnk2C = 1;
			return 1;
		}
	}
	// the wall top is below the player: only a brush is possible
	else if (m_obj04.Sub_40C9B0(pdwPts + 2, &dwOutC, &dwOutB, &dwOutA) <=
				(long)pdwPts[4] ||
			m_obj04.Sub_40C9B0(pPlayer->Sub_403880(adwPos), &dwOutC,
				&dwOutB, &dwOutA) <= (long)pdwPts[4] ||
			m_obj04.Sub_40C9B0(pdwPts, &dwOutC, &dwOutB, &dwOutA) <=
				(long)pdwPts[4])
	{
		m_dwUnk2C = 0;
		return 1;
	}
	return 0;
}

// Segment test (slot 13): does the segment dwA cross this wall, and does
// it pass through the wall's height band?
// hover.exe:0x40F7F0
// FUNCTION: HOVER 0x40f7f0
int CWall::Sub_411570(DWORD dwA, DWORD dwB, DWORD dwC, DWORD* pdwOut)
{
	CMerlinStatic* pLine;
	long lBottom;
	long lTop;
	DWORD adwPt[2];

	pLine = (CMerlinStatic*)m_dwUnk78;
	if (pLine == NULL)
	{
		*pdwOut = 0;
		return 0;
	}
	if (pLine->Sub_401260((CMerlinLine*)dwA, adwPt) != 0)
	{
		// the original re-reads m_dwUnk78 after the intersection call
		lTop = (short)((CMerlinStatic*)m_dwUnk78)->m_wUnkA6;
		lBottom = (short)((CMerlinStatic*)m_dwUnk78)->m_wUnkA4;
		if (lBottom < (long)dwC && (long)dwB < lTop)
		{
			*pdwOut = 1;
			return 1;
		}
	}
	*pdwOut = 0;
	return 1;
}

// hover.exe:0x42C670
// FUNCTION: HOVER 0x42c670
void CWall::Sub_4087D0()
{
	CGameObj::Sub_4087D0();
}

// hover.exe:0x42C680
// FUNCTION: HOVER 0x42c680
void CWall::Sub_4087F0()
{
	CGameObj::Sub_4087F0();
}

// hover.exe:0x42C690
// FUNCTION: HOVER 0x42c690
void CWall::Sub_413020()
{
	CGameObj::Sub_413020();
}

// The original disables frame-pointer omission for this function (full ebp
// frame without SEH, unlike its neighbors).
#pragma optimize("y", off)

// The wall registration below reads the segment endpoints and the collider's
// cell counts through inline accessors that return the pair by value: every
// use site therefore materializes an 8-byte temporary (edx:eax) and, when the
// first member is the one selected, copies it into a further stack slot.
// That is what gives hover.exe:0x40EF30 its 0x1B8-byte frame.
inline POINT SegPt0(const CSub40D320* pSeg)
{
	return *(POINT*)&pSeg->m_dwUnk00;
}

inline POINT SegPt1(const CSub40D320* pSeg)
{
	return *(POINT*)&pSeg->m_dwUnk08;
}

inline POINT ColliderCells(const CCollider* pMgr)
{
	return *(POINT*)&pMgr->m_dwUnk04;
}

// hover.exe:0x40EF30
// FUNCTION: HOVER 0x40ef30
int CWall::Sub_408820(DWORD dwA)
{
	// 16.16 fixed-point value built word-wise by the original (signed:
	// the slope divides and multiplies are idiv/imul)
	union FIXEDPT
	{
		long dw;
		short s[2];
	};

	int nCell;
	int nDX;
	int nDY;
	int nCol;
	int nRow;
	int nStartX;
	int nStartY;
	int nEndX;
	int nEndY;
	int nColA;
	int nColB;
	int nRowA;
	int nRowB;
	int nXBound;
	int nYBound;
	int nCross;
	FIXEDPT fxSlope;
	FIXEDPT fxT;
	FIXEDPT* pFx;
	short sDelta;

	nCell = ((CCollider*)dwA)->m_dwUnk0C;
	nDX = SegPt0(&m_obj04).x - SegPt1(&m_obj04).x;
	g_dwUnk_4C5590++;
	nDY = SegPt0(&m_obj04).y - SegPt1(&m_obj04).y;

	if (nDY == 0 && nDX == 0)
	{
		nCol = SegPt0(&m_obj04).x / nCell;
		nRow = SegPt0(&m_obj04).y / nCell;
		if (((CCollider*)dwA)->m_dwUnk44)
			((CCollider*)dwA)->Sub_40F8A0(nCol, nRow);
		else
			((CCollider*)dwA)->m_obj10.Sub_4086A0(nCol, nRow, this);
		return 1;
	}

	if (abs(nDY) < abs(nDX))
	{
		int nRowStep;
		int nRowDir;

		// walk the cell columns from the leftmost endpoint
		if (nDX < 0)
		{
			nStartX = SegPt0(&m_obj04).x;
			nStartY = SegPt0(&m_obj04).y;
			nEndX = SegPt1(&m_obj04).x;
			nEndY = SegPt1(&m_obj04).y;
		}
		else
		{
			nEndX = SegPt0(&m_obj04).x;
			nEndY = SegPt0(&m_obj04).y;
			nStartX = SegPt1(&m_obj04).x;
			nStartY = SegPt1(&m_obj04).y;
			nDX = -nDX;
			nDY = -nDY;
		}

		nColA = nStartX / nCell;
		nColB = nEndX / nCell;
		nRowA = nStartY / nCell;
		nRowB = nEndY / nCell;

		if (nColA < 0)
			nColA = 0;
		if (nColA >= ColliderCells((CCollider*)dwA).x)
			nColA = ColliderCells((CCollider*)dwA).x - 1;
		if (nColB < 0)
			nColA = 0;
		if (nColB >= ColliderCells((CCollider*)dwA).x)
			nColB = ColliderCells((CCollider*)dwA).x - 1;
		if (nRowA < 0)
			nRowA = 0;
		if (nRowA >= ColliderCells((CCollider*)dwA).y)
			nRowA = ColliderCells((CCollider*)dwA).y - 1;
		if (nRowB < 0)
			nRowB = 0;
		if (nRowB >= ColliderCells((CCollider*)dwA).y)
			nRowB = ColliderCells((CCollider*)dwA).y - 1;

		// 16.16 fixed-point rows-per-column slope
		fxSlope.s[0] = 0;
		fxSlope.s[1] = (short)nDY;
		fxSlope.dw /= (short)nDX;

		nRowStep = (nEndY > nStartY) ? nCell : -nCell;
		nRowDir = (nEndY > nStartY) ? 1 : -1;
		nYBound = nRowA * nCell + max(nRowStep, 0);
		nRow = nRowA;
		nXBound = (nColA + 1) * nCell;

		for (nCol = nColA; nCol < nColB; nCol++)
		{
			if (((CCollider*)dwA)->m_dwUnk44)
				((CCollider*)dwA)->Sub_40F8A0(nCol, nRow);
			else
				((CCollider*)dwA)->m_obj10.Sub_4086A0(nCol, nRow, this);

			// y at the next vertical grid line
			fxT.dw = fxSlope.dw;
			pFx = &fxT;
			sDelta = nXBound - nStartX;
			_asm mov ecx, pFx
			_asm movsx eax, sDelta
			_asm imul dword ptr [ecx]
			_asm mov [ecx], eax
			nCross = fxT.s[1] + nStartY;

			if ((nDY < 0 && nYBound <= nCross) ||
				(nDY > 0 && nYBound > nCross))
			{
				// crossed into the next row before leaving this column
				nRow += nRowDir;
				// a debug trap left in the shipping build
				if (nRow < 0)
				{
					_asm int 3
				}
				nYBound += nRowStep;
				if (((CCollider*)dwA)->m_dwUnk44)
					((CCollider*)dwA)->Sub_40F8A0(nCol, nRow);
				else
					((CCollider*)dwA)->m_obj10.Sub_4086A0(nCol, nRow, this);
			}

			nXBound += nCell;
		}
	}
	else
	{
		int nColStep;
		int nColDir;

		// walk the cell rows from the topmost endpoint
		if (nDY < 0)
		{
			nStartX = SegPt0(&m_obj04).x;
			nStartY = SegPt0(&m_obj04).y;
			nEndX = SegPt1(&m_obj04).x;
			nEndY = SegPt1(&m_obj04).y;
		}
		else
		{
			nEndX = SegPt0(&m_obj04).x;
			nEndY = SegPt0(&m_obj04).y;
			nStartX = SegPt1(&m_obj04).x;
			nStartY = SegPt1(&m_obj04).y;
			nDX = -nDX;
			nDY = -nDY;
		}

		nColA = nStartX / nCell;
		nColB = nEndX / nCell;
		nRowA = nStartY / nCell;
		nRowB = nEndY / nCell;

		if (nColA < 0)
			nColA = 0;
		if (nColA >= ColliderCells((CCollider*)dwA).x)
			nColA = ColliderCells((CCollider*)dwA).x - 1;
		if (nColB < 0)
			nColA = 0;
		if (nColB >= ColliderCells((CCollider*)dwA).x)
			nColB = ColliderCells((CCollider*)dwA).x - 1;
		if (nRowA < 0)
			nRowA = 0;
		if (nRowA >= ColliderCells((CCollider*)dwA).y)
			nRowA = ColliderCells((CCollider*)dwA).y - 1;
		if (nRowB < 0)
			nRowB = 0;
		if (nRowB >= ColliderCells((CCollider*)dwA).y)
			nRowB = ColliderCells((CCollider*)dwA).y - 1;

		// 16.16 fixed-point columns-per-row slope
		fxSlope.s[0] = 0;
		fxSlope.s[1] = (short)nDX;
		fxSlope.dw /= (short)nDY;

		nColStep = (nEndX > nStartX) ? nCell : -nCell;
		nColDir = (nEndX > nStartX) ? 1 : -1;
		nXBound = nColA * nCell + max(nColStep, 0);
		nCol = nColA;
		nYBound = (nRowA + 1) * nCell;

		for (nRow = nRowA; nRow < nRowB; nRow++)
		{
			if (((CCollider*)dwA)->m_dwUnk44)
				((CCollider*)dwA)->Sub_40F8A0(nCol, nRow);
			else
				((CCollider*)dwA)->m_obj10.Sub_4086A0(nCol, nRow, this);

			// x at the next horizontal grid line
			fxT.dw = fxSlope.dw;
			pFx = &fxT;
			sDelta = nYBound - nStartY;
			_asm mov ecx, pFx
			_asm movsx eax, sDelta
			_asm imul dword ptr [ecx]
			_asm mov [ecx], eax
			nCross = fxT.s[1] + nStartX;

			if ((nDX < 0 && nCross >= nXBound) ||
				(nDX > 0 && nCross < nXBound))
			{
				// crossed into the next column before leaving this row
				nXBound += nColStep;
				nCol += nColDir;
				if (((CCollider*)dwA)->m_dwUnk44)
					((CCollider*)dwA)->Sub_40F8A0(nCol, nRow);
				else
					((CCollider*)dwA)->m_obj10.Sub_4086A0(nCol, nRow, this);
			}

			nYBound += nCell;
		}
	}

	nCol = nColB;
	nRow = nRowB;
	if (((CCollider*)dwA)->m_dwUnk44)
		((CCollider*)dwA)->Sub_40F8A0(nCol, nRow);
	else
		((CCollider*)dwA)->m_obj10.Sub_4086A0(nCol, nRow, this);
	return 1;
}

#pragma optimize("", on)

// hover.exe:0x42C5E0
// FUNCTION: HOVER 0x42c5e0
void* CWall::operator new(size_t nSize)
{
	CWall* pWall;
	int nCount;
	int i;

	pWall = (CWall*)g_dwUnk_4C5588;
	if (pWall == NULL)
	{
		g_dwUnk_4C5588 = (DWORD)::operator new(g_dwUnk_4C558C * sizeof(CWall));
		g_listUnk_4A46D0.AddTail((void*)g_dwUnk_4C5588);
		nCount = g_dwUnk_4C558C;
		pWall = (CWall*)g_dwUnk_4C5588;
		for (i = 0; i < nCount - 1; i++)
			pWall[i].m_dwUnk9C = (DWORD)&pWall[i + 1];
		pWall[nCount - 1].m_dwUnk9C = 0;
	}
	g_dwUnk_4C5588 = pWall->m_dwUnk9C;
	return pWall;
}

// The original folds CWall::operator delete into the deleting destructor
// (hover.exe:0x417BA0); there is no standalone function in hover.exe.
void CWall::operator delete(void* p)
{
	DWORD dwHead = g_dwUnk_4C5588;
	g_dwUnk_4C5588 = (DWORD)p;
	((CWall*)p)->m_dwUnk9C = dwHead;
}

/////////////////////////////////////////////////////////////////////////////
// CFloorObj

// hover.exe:0x421930
// FUNCTION: HOVER 0x421930
CFloorObj::CFloorObj()
	: m_list94(10)
{
	m_dwUnk8C = 0;
	m_dwUnk90 = 0;
	m_dwUnk2C = 0;
	m_dwUnkB0 = 0xFFFFFFFF;
}

// hover.exe:0x41A610
// FUNCTION: HOVER 0x41a610
CFloorObj::CFloorObj(DWORD dwUnk)
	: m_list94(10)
{
	m_dwUnk8C = 0;
	m_dwUnk90 = 0;
	m_dwUnk2C = 0;
	m_dwUnkB0 = dwUnk;
}

// Deletes every object still standing on the pad. The pool release
// happens in the compiler-generated deleting destructor (0x4219D0).
// hover.exe:0x421890
// FUNCTION: HOVER 0x421890
CFloorObj::~CFloorObj()
{
	POSITION pos;
	CGameObj* pObj;

	pos = m_list94.GetHeadPosition();
	while (pos != NULL)
	{
		pObj = (CGameObj*)m_list94.GetNext(pos);
		if (pObj != NULL)
			delete pObj;
	}
}

// Per-frame think (slot 7): pads do nothing.
// hover.exe:0x42B800
// FUNCTION: HOVER 0x42b800
void CFloorObj::Sub_42AD30()
{
}

// The player is standing on / bumping into the pad (slot 8). Type 6 is a
// two-level pad (the nearer of m_dwUnk8C and m_dwUnk90 wins), type 7 is a
// solid step, everything else drops the player back to the floor.
// hover.exe:0x40D730
// FUNCTION: HOVER 0x40d730
void CFloorObj::Sub_42AD40(CPlayer* pOther, DWORD dwB)
{
	long lZ;
	long lLow;
	long lHigh;
	WORD wTop;

	if (pOther->m_dwUnk01F8 != 0)
		Hover_sub_401140()->m_dwUnk0520 = 1;

	if (m_dwUnk2C != 0)
	{
		if (pOther->Sub_41A6B0((CGameObj*)m_dwUnk88, dwB) != 0 &&
			pOther->m_dwUnk01F8 != 0 && theApp.m_dwUnk0408 != 0)
			Sub_40D6E0(0x12, 3, 5, 0);
		if (m_dwUnkB0 == 7 && *(DWORD*)&pOther->m_pad00A8[0] == 0)
		{
			wTop = (WORD)(m_dwUnk8C + 0x40);
			pOther->m_dwUnk013C = 1;
			pOther->m_awUnk0090[8] = 0;
			pOther->m_awUnk0090[9] = wTop;
		}
		return;
	}

	// Not a switch: hover.exe tests "!= 6" first so the case-7 test (and
	// its short body) falls straight through with no jump, while case 6's
	// longer body sits off in the else arm reached by a jump (mirrors the
	// inverted-if shape rule 148 uses for the sibling Sub_42AD60; a plain
	// switch or an ascending if-chain both place case 6 first instead,
	// notes/perfect-match.md rule 243).
	if (m_dwUnkB0 != 6)
	{
		if (m_dwUnkB0 == 7)
		{
			if (*(DWORD*)&pOther->m_pad00A8[0] != 0)
				return;
			wTop = (WORD)(m_dwUnk8C + 0x40);
			pOther->m_dwUnk013C = 1;
			pOther->m_awUnk0090[8] = 0;
			pOther->m_awUnk0090[9] = wTop;
			return;
		}
	}
	else
	{
		lZ = (long)pOther->m_dwUnk01A4;
		if ((long)m_dwUnk90 < lZ)
		{
			if (*(DWORD*)&pOther->m_pad00A8[0] != 0)
				return;
			wTop = (WORD)(m_dwUnk90 + 0x40);
			pOther->m_dwUnk013C = 1;
			pOther->m_awUnk0090[8] = 0;
			pOther->m_awUnk0090[9] = wTop;
			return;
		}
		lLow = abs(lZ - (long)m_dwUnk8C + 0x40);
		lHigh = abs(lZ - (long)m_dwUnk90);
		if (lLow < lHigh && m_dwUnk8C != 0)
		{
			// closer to the lower level: drop back to the floor
			pOther->m_awUnk0090[8] = 0;
			pOther->m_dwUnk013C = 0;
			pOther->m_awUnk0090[9] = 0x40;
			*(DWORD*)&pOther->m_pad00A8[0] = 0;
			return;
		}
		pOther->m_awUnk0090[9] = (WORD)(m_dwUnk90 + 0x40);
		pOther->m_awUnk0090[8] = 0;
		pOther->m_dwUnk013C = 1;
		*(DWORD*)&pOther->m_pad00A8[0] = 0;
		return;
	}

	if (*(DWORD*)&pOther->m_pad00A8[0] != 0)
		return;
	pOther->m_awUnk0090[9] = 0x40;
	pOther->m_awUnk0090[8] = 0;
}

// Containment test (slot 10): the point (dwA,dwB) is on the pad when it
// is on the inner side of every one of the pad's edge objects.
// hover.exe:0x42B810
// FUNCTION: HOVER 0x42b810
int CFloorObj::Sub_42AD70(DWORD dwA, DWORD dwB, DWORD dwC)
{
	POSITION pos;
	CGameObj* pObj;

	pos = m_list94.GetHeadPosition();
	while (pos != NULL)
	{
		pObj = (CGameObj*)m_list94.GetNext(pos);
		if (pObj->m_obj04.Sub_40D2F0((long*)&dwA) == 0)
			return 0;
	}
	return 1;
}

// Player/pad interaction test (slot 11). m_dwUnkB0 selects the pad kind:
// -1 is a plain floor patch, 6 a two-level pad and 7 a step; the edge
// object that was hit is left in m_dwUnk88 for Sub_42AD40.
// hover.exe:0x40C620
// FUNCTION: HOVER 0x40c620
int CFloorObj::Sub_42AD60(DWORD dwUnk)
{
	DWORD* pdwPts;
	POSITION pos;
	CGameObj* pObj;
	int bIn1;
	int bIn2;
	int bIn3;
	DWORD adwPos[2];
	DWORD dwOutA;
	DWORD dwOutB;
	DWORD dwOutC;
	CPlayer* pPlayer;

	pPlayer = (CPlayer*)dwUnk;
	pdwPts = pPlayer->Sub_4089E0();
	m_dwUnk88 = 0;

	// hover.exe tests -1, then 7, then 6 and falls through into the
	// case 6 body, which is a nested chain of inverted equality tests
	// (a `switch` sorts its comparison chain ascending and would put
	// the -1 test last, notes/perfect-match.md rule 102).
	if (m_dwUnkB0 != (DWORD)-1)
	{
		if (m_dwUnkB0 != 7)
		{
			if (m_dwUnkB0 == 6)
			{
				if ((long)m_dwUnk8C > (long)pPlayer->m_dwUnk01A4 + 0x40)
					return 0;

				bIn1 = 1;
				bIn2 = 1;
				bIn3 = 1;
				m_dwUnk2C = 0;
				pos = m_list94.GetHeadPosition();
				while (pos != NULL)
				{
					pObj = (CGameObj*)m_list94.GetNext(pos);
					if (pObj->m_obj04.Sub_40D2F0((long*)pdwPts) == 0)
					{
						bIn1 = 0;
						break;
					}
				}
				if (bIn1 != 0)
					return 1;

				pos = m_list94.GetHeadPosition();
				while (pos != NULL)
				{
					pObj = (CGameObj*)m_list94.GetNext(pos);
					if (pObj->m_obj04.Sub_40D2F0((long*)(pdwPts + 2)) == 0)
					{
						bIn2 = 0;
						break;
					}
				}
				if (bIn2 != 0)
					return 1;

				pos = m_list94.GetHeadPosition();
				while (pos != NULL)
				{
					pObj = (CGameObj*)m_list94.GetNext(pos);
					if (pObj->m_obj04.Sub_40D2F0((long*)pPlayer->Sub_403880(adwPos)) == 0)
					{
						bIn3 = 0;
						break;
					}
				}
				if (bIn3 != 0)
					return 1;

				if ((long)m_dwUnk90 < (long)pPlayer->m_dwUnk01A4)
					return 0;

				pos = m_list94.GetHeadPosition();
				while (pos != NULL)
				{
					pObj = (CGameObj*)m_list94.GetNext(pos);
					if (pObj->m_obj04.Sub_40C9B0(pdwPts, &dwOutC, &dwOutB, &dwOutA) <=
							(long)pdwPts[4] ||
						pObj->m_obj04.Sub_40C9B0(pdwPts + 2, &dwOutC, &dwOutB, &dwOutA) <=
							(long)pdwPts[4])
					{
						m_dwUnk88 = (DWORD)pObj;
						m_dwUnk2C = 1;
						return 1;
					}
				}
			}

			return 0;
		}
		else
		{
			if ((long)m_dwUnk90 >= (long)pPlayer->m_dwUnk01A4)
			{
				pos = m_list94.GetHeadPosition();
				while (pos != NULL)
				{
					pObj = (CGameObj*)m_list94.GetNext(pos);
					// The pad's edge objects carry their top height at
					// +0x98; the concrete class of the list elements is not
					// identified yet, so the field is reached by offset.
					if (*(long*)((BYTE*)pObj + 0x98) <=
						(long)pPlayer->m_dwUnk01A4)
						continue;
					if (pObj->m_obj04.Sub_40C9B0(pdwPts, &dwOutC, &dwOutB,
							&dwOutA) <= (long)pdwPts[4] ||
						pObj->m_obj04.Sub_40C9B0(pdwPts + 2, &dwOutC, &dwOutB,
							&dwOutA) <= (long)pdwPts[4])
					{
						m_dwUnk88 = (DWORD)pObj;
						m_dwUnk2C = 1;
						return 1;
					}
				}
			}
			m_dwUnk2C = 0;
			pos = m_list94.GetHeadPosition();
			while (pos != NULL)
			{
				pObj = (CGameObj*)m_list94.GetNext(pos);
				if (pObj->m_obj04.Sub_40D2F0((long*)pPlayer->Sub_403880(adwPos)) == 0)
					return 0;
			}
			return 1;
		}
	}
	else
	{
		if ((long)m_dwUnk8C + 0x60 < (long)pPlayer->m_dwUnk01A4)
			return 0;
		bIn1 = 1;
		bIn2 = 1;
		m_dwUnk2C = 0;
		pos = m_list94.GetHeadPosition();
		while (pos != NULL)
		{
			pObj = (CGameObj*)m_list94.GetNext(pos);
			if (pObj->m_obj04.Sub_40D2F0((long*)pdwPts) == 0)
				bIn1 = 0;
		}
		// Original quirk: `pos` is NULL by now and is never reset to the
		// head, so the second proximity point is in fact never tested.
		while (pos != NULL)
		{
			pObj = (CGameObj*)m_list94.GetNext(pos);
			if (pObj->m_obj04.Sub_40D2F0((long*)(pdwPts + 2)) == 0)
				bIn2 = 0;
		}
		return (bIn1 != 0 && bIn2 != 0);
	}

	return 0;
}

// Register the pad with the collider (slot 12): claim every cell of the
// bounding box of the pad's edge points.
// hover.exe:0x41A960
// FUNCTION: HOVER 0x41a960
int CFloorObj::Sub_408820(DWORD dwA)
{
	CCollider* pMgr;
	POSITION pos;
	CGameObj* pObj;
	int nCell;
	long lMinX;
	long lMinY;
	long lMaxX;
	long lMaxY;
	int nX1;
	int nX2;
	int nY1;
	int nY2;
	int x;
	int y;

	pMgr = (CCollider*)dwA;
	nCell = (int)pMgr->m_dwUnk0C;
	lMaxX = 0;
	lMaxY = 0;
	lMinX = 0x5A00;
	lMinY = 0x5A00;
	pos = m_list94.GetHeadPosition();
	while (pos != NULL)
	{
		pObj = (CGameObj*)m_list94.GetNext(pos);
		if (lMinX > SegPt0(&pObj->m_obj04).x)
			lMinX = SegPt0(&pObj->m_obj04).x;
		if (lMinY > SegPt0(&pObj->m_obj04).y)
			lMinY = SegPt0(&pObj->m_obj04).y;
		if (SegPt0(&pObj->m_obj04).x > lMaxX)
			lMaxX = SegPt0(&pObj->m_obj04).x;
		if (SegPt0(&pObj->m_obj04).y > lMaxY)
			lMaxY = SegPt0(&pObj->m_obj04).y;
		if (SegPt1(&pObj->m_obj04).x < lMinX)
			lMinX = SegPt1(&pObj->m_obj04).x;
		if (SegPt1(&pObj->m_obj04).y < lMinY)
			lMinY = SegPt1(&pObj->m_obj04).y;
		if (SegPt1(&pObj->m_obj04).x > lMaxX)
			lMaxX = SegPt1(&pObj->m_obj04).x;
		if (lMaxY < SegPt1(&pObj->m_obj04).y)
			lMaxY = SegPt1(&pObj->m_obj04).y;
	}
	nX1 = lMinX / nCell;
	nX2 = lMaxX / nCell;
	nY1 = lMinY / nCell;
	nY2 = lMaxY / nCell;
	for (x = nX1; x <= nX2; x++)
	{
		for (y = nY1; y <= nY2; y++)
		{
			if (pMgr->m_dwUnk44 != 0)
				pMgr->Sub_40F8A0(x, y);
			else
				pMgr->m_obj10.Sub_4086A0(x, y, this);
		}
	}
	return 1;
}

// Segment test (slot 13): forwards the query to every object on the pad
// and ORs the results together.
// hover.exe:0x413220
// FUNCTION: HOVER 0x413220
int CFloorObj::Sub_411570(DWORD dwA, DWORD dwB, DWORD dwC, DWORD* pdwOut)
{
	POSITION pos;
	CGameObj* pObj;
	int bAny;
	DWORD dwFlag;

	bAny = 0;
	*pdwOut = 0;
	pos = m_list94.GetHeadPosition();
	while (pos != NULL)
	{
		pObj = (CGameObj*)m_list94.GetNext(pos);
		if (pObj != NULL)
		{
			bAny |= pObj->Sub_411570(dwA, dwB, dwC, &dwFlag);
			*pdwOut |= dwFlag;
			if (bAny != 0 && *pdwOut != 0)
				return 1;
		}
	}
	return bAny;
}

// hover.exe:0x42B860
// FUNCTION: HOVER 0x42b860
void CFloorObj::Sub_4087D0()
{
	CGameObj::Sub_4087D0();
}

// hover.exe:0x42B870
// FUNCTION: HOVER 0x42b870
void CFloorObj::Sub_4087F0()
{
	CGameObj::Sub_4087F0();
}

// hover.exe:0x42B880
// FUNCTION: HOVER 0x42b880
void CFloorObj::Sub_413020()
{
	CGameObj::Sub_413020();
}

// hover.exe:0x42B760
// FUNCTION: HOVER 0x42b760
void* CFloorObj::operator new(size_t nSize)
{
	CFloorObj* pObj;
	int nCount;
	int i;

	pObj = (CFloorObj*)g_dwUnk_4C53E0;
	if (pObj == NULL)
	{
		g_dwUnk_4C53E0 =
			(DWORD)::operator new(g_dwUnk_4C53E4 * sizeof(CFloorObj));
		g_listUnk_4A45E0.AddTail((void*)g_dwUnk_4C53E0);
		nCount = g_dwUnk_4C53E4;
		pObj = (CFloorObj*)g_dwUnk_4C53E0;
		for (i = 0; i < nCount - 1; i++)
			pObj[i].m_dwUnk84 = (DWORD)&pObj[i + 1];
		pObj[nCount - 1].m_dwUnk84 = 0;
	}
	g_dwUnk_4C53E0 = pObj->m_dwUnk84;
	return pObj;
}

// hover.exe:0x42B7E0
// FUNCTION: HOVER 0x42b7e0
void CFloorObj::operator delete(void* p)
{
	DWORD dwHead = g_dwUnk_4C53E0;
	g_dwUnk_4C53E0 = (DWORD)p;
	((CFloorObj*)p)->m_dwUnk84 = dwHead;
}

/////////////////////////////////////////////////////////////////////////////
// The pad subclasses

// hover.exe:0x421820
// FUNCTION: HOVER 0x421820
CSledPad::CSledPad(DWORD dwUnk)
{
	m_dwUnkB4 = dwUnk;
}

// The destructor is defined in the class declaration, so the compiler folds
// it into the scalar deleting destructor that the vtable slot holds - which
// is what hover.exe:0x4217B0 is. The block below emits no code; it only gives
// the reccmp marker a declaration to name ??_GCSledPad@@ (rules 34/184).
// hover.exe:0x4217B0
#if 0
// FUNCTION: HOVER 0x4217b0
CSledPad::~CSledPad()
{
}
#endif

// hover.exe:0x42B6C0
// FUNCTION: HOVER 0x42b6c0
void CSledPad::Sub_42AD30()
{
	CFloorObj::Sub_42AD30();
}

// Player entered the sled pad (slot 8): latch the pad direction into the
// player and start the sled timer.
// hover.exe:0x41A5C0
// FUNCTION: HOVER 0x41a5c0
void CSledPad::Sub_42AD40(CPlayer* pOther, DWORD dwB)
{
	if (pOther->m_dwUnk0140 != 0)
		return;
	*(DWORD*)&pOther->m_pad0160[0] = m_dwUnkB4;
	if (pOther->m_dwUnk01F8 != 0)
		Hover_sub_401140()->m_dwUnk0520 = 1;
	if (pOther->m_dwUnk014C == (DWORD)-1)
		pOther->m_dwUnk014C = 0x1E;
}

// hover.exe:0x42B6D0
// FUNCTION: HOVER 0x42b6d0
void CSledPad::Sub_4087D0()
{
	CFloorObj::Sub_4087D0();
}

// hover.exe:0x42B6E0
// FUNCTION: HOVER 0x42b6e0
void CSledPad::Sub_4087F0()
{
	CFloorObj::Sub_4087F0();
}

// hover.exe:0x42B6F0
// FUNCTION: HOVER 0x42b6f0
void CSledPad::Sub_413020()
{
	CFloorObj::Sub_413020();
}

// hover.exe:0x421ED0
// FUNCTION: HOVER 0x421ed0
CHoldPad::CHoldPad()
{
}

// The destructor is defined in the class declaration, so the compiler folds
// it into the scalar deleting destructor that the vtable slot holds - which
// is what hover.exe:0x421F30 is. The block below emits no code; it only gives
// the reccmp marker a declaration to name ??_GCHoldPad@@ (rules 34/184).
// hover.exe:0x421F30
#if 0
// FUNCTION: HOVER 0x421f30
CHoldPad::~CHoldPad()
{
}
#endif

// hover.exe:0x42C1A0
// FUNCTION: HOVER 0x42c1a0
void CHoldPad::Sub_42AD30()
{
}

// Player entered the hold pad (slot 8): freeze him for 90 frames.
// hover.exe:0x42C1B0
// FUNCTION: HOVER 0x42c1b0
void CHoldPad::Sub_42AD40(CPlayer* pOther, DWORD dwB)
{
	if (pOther->m_dwUnk0140 != 0)
		return;
	if (pOther->m_dwUnk01F8 != 0)
		Hover_sub_401140()->m_dwUnk0520 = 1;
	if (pOther->m_dwUnk0148 == (DWORD)-1)
		pOther->m_dwUnk0148 = 0x5A;
	pOther->m_dwUnk0154 = 1;
}

// hover.exe:0x42C200
// FUNCTION: HOVER 0x42c200
void CHoldPad::Sub_4087D0()
{
	CFloorObj::Sub_4087D0();
}

// hover.exe:0x42C210
// FUNCTION: HOVER 0x42c210
void CHoldPad::Sub_4087F0()
{
	CFloorObj::Sub_4087F0();
}

// hover.exe:0x42C220
// FUNCTION: HOVER 0x42c220
void CHoldPad::Sub_413020()
{
	CFloorObj::Sub_413020();
}

// hover.exe:0x422AC0
// FUNCTION: HOVER 0x422ac0
CFlagRemover::CFlagRemover()
{
}

// The destructor is defined in the class declaration, so the compiler folds
// it into the scalar deleting destructor that the vtable slot holds - which
// is what hover.exe:0x422B20 is. The block below emits no code; it only gives
// the reccmp marker a declaration to name ??_GCFlagRemover@@ (rules 34/184).
// hover.exe:0x422B20
#if 0
// FUNCTION: HOVER 0x422b20
CFlagRemover::~CFlagRemover()
{
}
#endif

// hover.exe:0x42C540
// FUNCTION: HOVER 0x42c540
void CFlagRemover::Sub_42AD30()
{
}

// Player entered the flag-removal pad (slot 8): the flag he is carrying
// is scored and a fresh one is dropped back onto the board.
// hover.exe:0x422920
// FUNCTION: HOVER 0x422920
void CFlagRemover::Sub_42AD40(CPlayer* pOther, DWORD dwB)
{
	CBumperDoc* pDoc;
	CBumperView* pView;
	CFlag* pFlag;
	long lCount;
	long lLimit;

	if (pOther->m_dwUnk01F8 != 0)
		Hover_sub_401140()->m_dwUnk0520 = 1;
	if (pOther->m_dwUnk0140 != 0)
		return;
	if (pOther->m_dwUnk015C == (DWORD)-1)
	{
		pOther->m_dwUnk015C = 1;
		if (pOther->m_obList00B4.GetCount() != 0)
		{
			theApp.m_pBumperDoc->Sub_412630(
				(CGameObj*)pOther->m_obList00B4.GetHead());
			pOther->m_obList00B4.RemoveHead();
			if (pOther->m_dwUnk01F8 == 0)
			{
				pDoc = theApp.m_pBumperDoc;
				pDoc->m_dwUnk83E8--;
				pView = Hover_sub_401140();
				{
					CSub422C30* pGauge = &pView->m_obj0480;
					lLimit = (long)pGauge->m_dwUnk3C;
					pGauge->m_dwUnk00 = 1;
					lCount = (long)pDoc->m_dwUnk83E8;
					if (lLimit <= lCount)
					{
						pGauge->m_dwUnk44 = lLimit;
					}
					else
					{
						pGauge->m_dwUnk44 = 0;
						if (lCount > 0)
							pGauge->m_dwUnk44 = lCount;
					}
				}
				pFlag = new CFlag(0);
				theApp.m_pBumperDoc->Sub_412250(pFlag, 0);
				if (theApp.m_dwUnk0408 != 0)
					Sub_40D6E0(0x21, 0, 5, 0);
			}
			else
			{
				pView = Hover_sub_401140();
				{
					CSub422C30* pGauge = &pView->m_obj0430;
					lLimit = (long)pGauge->m_dwUnk3C;
					pGauge->m_dwUnk00 = 1;
					lCount = (long)pOther->m_obList00B4.GetCount();
					if (lLimit <= lCount)
					{
						pGauge->m_dwUnk44 = lLimit;
					}
					else
					{
						pGauge->m_dwUnk44 = 0;
						if (lCount > 0)
							pGauge->m_dwUnk44 = lCount;
					}
				}
				pFlag = new CFlag(1);
				theApp.m_pBumperDoc->Sub_412250(pFlag, 0);
				if (theApp.m_dwUnk0408 != 0)
					Sub_40D6E0(4, 0, 5, 0);
			}
		}
	}
	pOther->m_dwUnk0158 = 1;
}

// hover.exe:0x42C550
// FUNCTION: HOVER 0x42c550
void CFlagRemover::Sub_4087D0()
{
	CFloorObj::Sub_4087D0();
}

// hover.exe:0x42C560
// FUNCTION: HOVER 0x42c560
void CFlagRemover::Sub_4087F0()
{
	CFloorObj::Sub_4087F0();
}

// hover.exe:0x42C570
// FUNCTION: HOVER 0x42c570
void CFlagRemover::Sub_413020()
{
	CFloorObj::Sub_413020();
}

/////////////////////////////////////////////////////////////////////////////
// CSub424210

// hover.exe:0x424210
// FUNCTION: HOVER 0x424210
CSub424210::~CSub424210()
{
}

/////////////////////////////////////////////////////////////////////////////
// CBeacon

// The original CBeacon constructor is folded into Hover_sub_423EA0
// (hover.exe:0x423EA0), which placement-constructs the object straight
// out of the class pool; there is no standalone constructor in hover.exe.
inline CBeacon::CBeacon(CObject* pTexture)
{
	WORD wUnk;

	m_wUnk46 = ((WORD*)pTexture)[0xA];	// texture +0x14
	wUnk = ((WORD*)pTexture)[0xB];		// texture +0x16
	m_pUnk98 = pTexture;
	m_dwUnkA8 = 0;
	m_wUnk44 = 0;
	m_wUnk48 = 0;
	m_wUnk4A = wUnk;
}

// The class destructor is empty: the m_arr84 member and the CGameObj base
// are destroyed by the compiler-generated calls that follow it inside the
// scalar deleting destructor, which also inlines CBeacon::operator delete
// (the pool push onto g_dwUnk_4C57A8). Rule 34: the marker pairs our ??1
// against the original's ??_G, which is what the vtable slot holds.
// hover.exe:0x424180
// The destructor is defined in the class declaration, so the compiler folds
// it into the scalar deleting destructor that the vtable slot holds - which
// is what hover.exe:0x424180 is. The block below emits no code; it only gives
// the reccmp marker a declaration to name ??_GCBeacon@@ (rules 34/184).
// hover.exe:0x424180
#if 0
// FUNCTION: HOVER 0x424180
CBeacon::~CBeacon()
{
}
#endif

// hover.exe:0x423E50
// FUNCTION: HOVER 0x423e50
int CBeacon::Sub_42AD90(CGameObj* pObj)
{
	if (pObj->IsKindOf(RUNTIME_CLASS(CSledPad)) ||
		pObj->IsKindOf(RUNTIME_CLASS(CHoldPad)) ||
		pObj->IsKindOf(RUNTIME_CLASS(CFlagRemover)))
		m_dwUnkA4 = 1;
	return m_dwUnkA4;
}

// The original inlines this pool allocator into Hover_sub_423EA0
// (hover.exe:0x423FD0-0x424036) and the matching operator delete into
// the deleting destructor (hover.exe:0x424180).
inline void* CBeacon::operator new(size_t nSize)
{
	CBeacon* pBeacon;
	int nCount;
	int i;

	nCount = g_dwUnk_4C57AC;
	pBeacon = (CBeacon*)g_dwUnk_4C57A8;
	if (pBeacon == NULL)
	{
		g_dwUnk_4C57A8 = (DWORD)::operator new(nCount * sizeof(CBeacon));
		g_listUnk_4A4858.AddTail((void*)g_dwUnk_4C57A8);
		nCount = g_dwUnk_4C57AC;
		pBeacon = (CBeacon*)g_dwUnk_4C57A8;
		for (i = 0; i < nCount - 1; i++)
			pBeacon[i].m_dwUnkAC = (DWORD)&pBeacon[i + 1];
		pBeacon[nCount - 1].m_dwUnkAC = 0;
	}
	g_dwUnk_4C57A8 = pBeacon->m_dwUnkAC;
	return pBeacon;
}

/////////////////////////////////////////////////////////////////////////////
// CSub40D320 methods

// hover.exe:0x422BF0
// FUNCTION: HOVER 0x422bf0
void CSub40D320::Sub_422BF0(DWORD* pdwPt1, DWORD* pdwPt2)
{
	*(POINT*)&m_dwUnk00 = *(POINT*)pdwPt1;
	*(POINT*)&m_dwUnk08 = *(POINT*)pdwPt2;
	Sub_42C820();
}

// hover.exe:0x40E8E0
// FUNCTION: HOVER 0x40e8e0
CSub40D320* CSub40D320::Sub_40E8E0(CObject* pObj)
{
	m_dwUnk00 = ((DWORD*)pObj)[5];	// pObj +0x14
	m_dwUnk08 = ((DWORD*)pObj)[7];	// pObj +0x1C
	m_dwUnk04 = ((DWORD*)pObj)[6];	// pObj +0x18
	m_dwUnk0C = ((DWORD*)pObj)[8];	// pObj +0x20
	Sub_42C820();
	return this;
}

/////////////////////////////////////////////////////////////////////////////
// Beacon setup

// hover.exe:0x423EA0
// FUNCTION: HOVER 0x423ea0
void Hover_sub_423EA0()
{
	POSITION pos;
	int nBeacons;
	int i;

	nBeacons = 0;
	pos = g_mapUnk_4A4768.GetStartPosition();
	while (pos != NULL)
	{
		CString strName;
		CObject* pValue;

		g_mapUnk_4A4768.GetNextAssoc(pos, strName, pValue);
		CharUpperA((char*)(const char*)strName);
		if (strncmp((const char*)strName, "BEACON_", 7) == 0)
			nBeacons++;
	}
	g_obArrUnk_4A4840.SetSize(nBeacons, -1);
	pos = g_mapUnk_4A4768.GetStartPosition();
	i = 0;
	while (pos != NULL)
	{
		CString strName;
		CObject* pValue;

		g_mapUnk_4A4768.GetNextAssoc(pos, strName, pValue);
		CharUpperA((char*)(const char*)strName);
		if (strncmp((const char*)strName, "BEACON_", 7) == 0)
		{
			// CBeacon::operator new returns NULL when the pool is
			// exhausted; the original stores that NULL in the table
			g_obArrUnk_4A4840[i++] = new CBeacon(pValue);
		}
	}
	Hover_sub_423CD0();
}

// hover.exe:0x423CD0
// FUNCTION: HOVER 0x423cd0
void Hover_sub_423CD0()
{
	DWORD adwA[2];
	DWORD adwB[2];
	CCollider* pMgr;
	CBeacon* pBeaconA;
	CBeacon* pBeaconB;
	short sA;
	short sB;
	int nHi;
	int nLo;
	int nSize;
	int i;
	int j;

	nSize = g_obArrUnk_4A4840.GetSize();
	pMgr = (CCollider*)theApp.m_pBumperDoc->m_dwUnk8388;
	for (i = 0; i < nSize - 1; i++)
	{
		pBeaconA = (CBeacon*)g_obArrUnk_4A4840[i];
		pBeaconA->Sub_403880(adwA);
		for (j = i + 1; j < nSize; j++)
		{
			pBeaconB = (CBeacon*)g_obArrUnk_4A4840[j];
			pBeaconB->Sub_403880(adwB);
			sA = (short)((WORD*)pBeaconA->m_pUnk98)[0xD];	// texture +0x1A
			sB = (short)((WORD*)pBeaconB->m_pUnk98)[0xD];
			if (abs(sA - sB) >= (int)((DWORD*)theApp.m_pUnk0154)[8])	// +0x20
				continue;
			nHi = (sA > sB) ? sA : sB;
			nLo = ((sA < sB) ? sA : sB) + 0x64;
			if (nLo < nHi)
			{
				nHi = (nHi + nLo) / 2;
				nLo = nHi;
			}
			if (pMgr->Sub_40C2B0(adwA[0], adwA[1], adwB[0], adwB[1],
					(DWORD)nHi, (DWORD)nLo,
					((DWORD*)theApp.m_pUnk0154)[7]))	// +0x1C
			{
				pBeaconA->m_dwUnkA4 = 0;
				pMgr->Sub_411600(adwA[0], adwA[1], adwB[0], adwB[1],
					((DWORD*)theApp.m_pUnk0154)[7], pBeaconA);
				if (pBeaconA->m_dwUnkA4 == 0)
				{
					pBeaconA->m_arr84.Add(pBeaconB);
					pBeaconB->m_arr84.Add(pBeaconA);
				}
			}
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
// CCollider manager methods used by Hover_sub_423CD0.

// hover.exe:0x40C2B0
// FUNCTION: HOVER 0x40c2b0
int CCollider::Sub_40C2B0(DWORD dwX1, DWORD dwY1, DWORD dwX2, DWORD dwY2,
	DWORD dwHi, DWORD dwLo, int nLimit)
{
	int nDist = (int)Hover_sub_408A00((long)dwX1, (long)dwY1, (long)dwX2,
		(long)dwY2);

	if (nDist > nLimit)
		return 0;

	CMerlinLine line((short)dwX1, (short)dwY1, (short)dwX2, (short)dwY2);

	{
		CWall wall;

		wall.m_obj04.Sub_40E8E0(&line);

		m_arrUnk48.SetSize(0, -1);
		m_dwUnk44 = 1;
		wall.Sub_408820((DWORD)this);
		m_dwUnk44 = 0;
	}

	int i = 0;
	if (i < m_arrUnk48.GetSize())
	{
		do
		{
			CGameObj* pObj = (CGameObj*)m_arrUnk48.GetAt(i);
			DWORD dwResult;

			pObj->Sub_411570((DWORD)&line, dwHi, dwLo, &dwResult);
			if (dwResult != 0)
				return 0;
			i++;
		} while (i < m_arrUnk48.GetSize());
	}

	return 1;
}

// hover.exe:0x411600
// FUNCTION: HOVER 0x411600
void CCollider::Sub_411600(DWORD dwX1, DWORD dwY1, DWORD dwX2, DWORD dwY2,
	int nLimit, CBeacon* pBeacon)
{
	// Shadow copies of the three parameters that stay live across the
	// Hover_sub_408A00 call (dwY1 does not and is left alone): declaring
	// them as locals rather than reading the stack parameters directly
	// is what gets dwX2 into the original's ebx (notes/perfect-match.md
	// rule 207's lever, extended from one shadowed value to three).
	DWORD dwLX1 = dwX1;
	DWORD dwLX2 = dwX2;
	DWORD dwLY2 = dwY2;
	int nDist = (int)Hover_sub_408A00((long)dwLX1, (long)dwY1, (long)dwLX2,
		(long)dwLY2);

	if (nDist > nLimit)
	{
		dwLX2 = dwLX1 + ((int)dwLX2 - (int)dwLX1) * nLimit / nDist;
		dwLY2 = dwY1 + ((int)dwLY2 - (int)dwY1) * nLimit / nDist;
	}

	CMerlinLine line((short)dwLX1, (short)dwY1, (short)dwLX2, (short)dwLY2);

	{
		CWall wall;

		wall.m_obj04.Sub_40E8E0(&line);

		m_arrUnk48.SetSize(0, -1);
		m_dwUnk44 = 1;
		wall.Sub_408820((DWORD)this);
		m_dwUnk44 = 0;
	}

	for (int i = 0; i < m_arrUnk48.GetSize(); i++)
	{
		CGameObj* pObj = (CGameObj*)m_arrUnk48.GetAt(i);
		DWORD dwResult;

		if (pObj->Sub_411570((DWORD)&line, 0, 0, &dwResult))
		{
			if (pBeacon->Sub_42AD90(pObj))
				break;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// CMerlinDynamic: the maze sprite the pod classes hand to the level world

// The constructor runs the CMerlinLine base constructor, builds the +0x44
// CString and presets the height/scale words. The original also inlines
// this whole body into the IMPLEMENT_SERIAL placement construction thunk
// at hover.exe:0x42CAB0 (see README).

// hover.exe:0x4132A0
// FUNCTION: HOVER 0x4132a0
CMerlinDynamic::CMerlinDynamic()
{
	m_wUnk54 = 0;
	m_wUnk56 = 0x80;
	m_dwUnk60 = 1;
	m_wUnk58 = 0x80;
	m_dwUnk64 = 1;
	m_wUnk5C = 0;
	m_wUnk5A = 0;
	m_strUnk44.Empty();
	m_pUnk50 = NULL;
	m_dwUnk68 = 0;
}

// Serialize (vtable slot 2): the texture name, the height/scale/position
// words and the m_dwUnk60 flag byte. On load the sprite registers itself
// in g_mapUnk_4A4728 under its object name and resolves its texture name
// in g_mapUnk_4A1D30.

// hover.exe:0x42CC40
// The original calls strcmp out of line here (two pushes and an add esp,8),
// so the intrinsic expansion has to be switched off for this function.
#pragma function(strcmp)

// FUNCTION: HOVER 0x42cc40
void CMerlinDynamic::Serialize(CArchive& ar)
{
	CMerlinArchive* pAr = (CMerlinArchive*)&ar;
	int nCount;
	BYTE by;

	CMerlinLine::Serialize(ar);
	if (ar.IsStoring())
	{
		*pAr << m_strUnk44;
		pAr->WriteWord(m_wUnk54);
		pAr->WriteWord(m_wUnk56);
		pAr->WriteWord(m_wUnk58);
		pAr->WriteWord(m_wUnk5A);
		pAr->WriteWord(m_wUnk5C);
		pAr->WriteByte((BYTE)m_dwUnk60);
		pAr->WriteWord((WORD)0);
	}
	else
	{
		if (!m_strUnk04.IsEmpty())
			g_mapUnk_4A4728[m_strUnk04] = this;
		*pAr >> m_strUnk44;
		if (strcmp(m_strUnk44, "") != 0)
		{
			if (!g_mapUnk_4A1D30.Lookup(m_strUnk44,
					(CObject*&)m_pUnk50))
				m_pUnk50 = NULL;
		}
		pAr->ReadWord(&m_wUnk54);
		pAr->ReadWord(&m_wUnk56);
		pAr->ReadWord(&m_wUnk58);
		pAr->ReadWord(&m_wUnk5A);
		pAr->ReadWord(&m_wUnk5C);
		*(BYTE*)&m_dwUnk60 = pAr->ReadByte();
		nCount = (short)pAr->ReadCount();
		while (nCount > 0)		// trailing comment bytes, skipped
		{
			pAr->ReadByte(&by);
			nCount--;
		}
	}
}

// The out-of-line byte reader the trailing-comment loop above calls (and
// CMerlinBSP::Serialize with it, hover.exe:0x41BE99). It is MFC's own
// CArchive::operator>>(BYTE&) body with the cursor advanced before the
// read; the original is almost certainly a CMerlinArchive overload of
// operator>>, which is why it returns the archive, but reccmp's marker
// parser cannot name an operator, so it carries an ordinary name here.
// Note that CMerlinObject::Serialize (texture.cpp) keeps `ar >> by`, i.e.
// the inline CArchive operator, exactly as the original does.
// hover.exe:0x42CF30
// FUNCTION: HOVER 0x42cf30
CMerlinArchive& CMerlinArchive::ReadByte(BYTE* pby)
{
	if (m_lpBufCur + 1 > m_lpBufMax)
		FillBuffer(1 - (UINT)(m_lpBufMax - m_lpBufCur));
	m_lpBufCur++;
	*pby = m_lpBufCur[-1];
	return *this;
}

/////////////////////////////////////////////////////////////////////////////
// CSub40D320 geometry helpers used by the maze objects

// Side-of-line test: nonzero when pdwPt is on (or in front of) the
// segment's positive side.
// hover.exe:0x40D2F0
// FUNCTION: HOVER 0x40d2f0
int CSub40D320::Sub_40D2F0(const long* plPt)
{
	long lCross;

	// The members are DWORD in this reconstruction; going through long
	// locals is what makes VC 2.2 emit the original's xor/setns instead
	// of a branch (notes/perfect-match.md rule 10).
	long lX0 = (long)m_dwUnk00;
	long lY0 = (long)m_dwUnk04;
	long lDX = (long)m_dwUnk10;
	long lDY = (long)m_dwUnk14;

	lCross = (lY0 - plPt[1]) * lDX + (plPt[0] - lX0) * lDY;
	return lCross >= 0;
}

/////////////////////////////////////////////////////////////////////////////
// CBumperDoc helpers reached from the maze objects (declared in
// bumperdoc.h; defined here because they only serve the pod classes)

// Drops pObj from the document's active-pod list.
// hover.exe:0x410250
// FUNCTION: HOVER 0x410250
int CBumperDoc::Sub_410250(CObject* pObj)
{
	POSITION pos;

	pos = m_list838C.Find(pObj);
	if (pos != NULL)
		m_list838C.RemoveAt(pos);
	return pos != NULL;
}

/////////////////////////////////////////////////////////////////////////////
// The 16.16 fixed-point gauge values (declared in bumpview.h)

// hover.exe:0x42A9D0
// FUNCTION: HOVER 0x42a9d0
CSub42AE90::CSub42AE90(double dbl)
{
	*(long*)this = (long)(dbl * 65536.0);
}


// Lights the first clamp(fix * m_dwUnk1C, 0, m_dwUnk1C) gauge segments.
// hover.exe:0x42A9F0
// FUNCTION: HOVER 0x42a9f0
void CSub422C80::Sub_42A9F0(CSub42AE90 fix)
{
	long lSegs;
	long lIndex;

	*(long*)&fix += 0x288;
	HOVER_FIXMULWP(*(DWORD*)&fix, (WORD)m_dwUnk1C);
	lIndex = (short)fix.m_wInt;
	lSegs = (long)m_dwUnk1C;
	if (lIndex > lSegs)
		lIndex = lSegs;
	if (lIndex >= lSegs)
	{
		m_dwUnk24 = lSegs;
	}
	else if (lIndex <= 0)
	{
		m_dwUnk24 = 0;
	}
	else
	{
		m_dwUnk24 = lIndex;
	}
}

/////////////////////////////////////////////////////////////////////////////
// CBumperView game-state entry (declared in bumpview.h; defined here
// because it is only reached from the flag capture above)

// hover.exe:0x427CF0
// FUNCTION: HOVER 0x427cf0
void CBumperView::Sub_427CF0(DWORD dwState)
{
	theApp.m_dwUnk01B8 = dwState;
	if (dwState == 2)
		m_dwUnk007C = 0x28;
	else
		m_dwUnk007C = 0x3C;
}
