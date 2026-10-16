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
#include "bumpview.h"		// CSegmentGauge (the CSlowPod dashboard gauge)
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
DWORD g_dwPodPoolBlock = 0xA;	// CPodObj pool block size (object count)

// hover.exe:0x4C53E4
// GLOBAL: HOVER 0x4c53e4
DWORD g_dwFloorObjPoolBlock = 0xA;	// CFloorObj pool block size (object count)

// hover.exe:0x4C558C
// GLOBAL: HOVER 0x4c558c
DWORD g_dwWallPoolBlock = 0x20;	// CWall pool block size (object count)

// hover.exe:0x4C57AC
// GLOBAL: HOVER 0x4c57ac
DWORD g_dwBeaconPoolBlock = 5;	// CBeacon pool block size (object count)

// hover.exe:0x4C5590
// GLOBAL: HOVER 0x4c5590
DWORD g_dwWallRegisterCount;	// incremented by CWall::UpdateRegions, never read

// hover.exe:0x4A4840
// GLOBAL: HOVER 0x4a4840
CObArray g_arrBeacons;	// beacon table built by Hover_BuildBeacons

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
inline CGaugeFraction::CGaugeFraction(WORD wInt, WORD wFrac)
{
	m_wFrac = wFrac;
	m_wInt = wInt;
}



// hover.exe expands CSegmentGauge::SetFraction (0x42A9F0) inline in the five
// per-frame pod handlers below while still calling it from the two
// UsePod handlers, i.e. the gauge update was written twice: once as the
// out-of-line method and once here. `pGauge` must be a local, because the
// original computes the gauge address once.
#define HOVER_INLINE_SUB_42A9F0(pGauge, fix) \
	do { \
		long _lSegs; \
		long _lIndex; \
		*(long*)&(fix) += 0x288; \
		HOVER_FIXMULWP(*(DWORD*)&(fix), (WORD)(pGauge)->m_dwSegments); \
		_lIndex = (short)(fix).m_wInt; \
		_lSegs = (long)(pGauge)->m_dwSegments; \
		if (_lIndex > _lSegs) \
			_lIndex = _lSegs; \
		if (_lIndex >= _lSegs) \
		{ \
			(pGauge)->m_dwLevel = _lSegs; \
		} \
		else if (_lIndex <= 0) \
		{ \
			(pGauge)->m_dwLevel = 0; \
		} \
		else \
		{ \
			(pGauge)->m_dwLevel = _lIndex; \
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
	m_pOwner = 0;
	m_awUnkE0[5] = 0;
	m_awUnkE0[6] = 0;
	m_awUnkE0[7] = 0;
	m_dwZOffset = 0x64;	// stored 0x64 here, then overwritten with 0 below
	m_pTexture = NULL;
	wsprintfA(szName, "%s%02d", "POD06_", 0);
	g_mapUnk_4A1D30.Lookup(szName, (CObject*&)m_pTexture);
	pTexture = m_pTexture;
	m_dwHeight = (short)pTexture->m_wHeight;
	nHalf = (short)pTexture->m_wWidth / 2;
	m_bOnBoard = 0;
	m_dwUnk64 = 0;
	m_dwZOffset = 0;
	m_dwRadius = 0xA;
	m_dwUnkB4 = 0x80;
	m_bRespawn = 1;
	m_dwHalfWidth = (short)nHalf;
}

// hover.exe:0x42AF20
// FUNCTION: HOVER 0x42af20
void* CPodObj::operator new(size_t nSize)
{
	CPodObj* pObj;
	int nCount;
	int i;

	pObj = g_pPodObjFree;
	if (pObj == NULL)
	{
		g_pPodObjFree = (CPodObj*)::operator new(g_dwPodPoolBlock * sizeof(CPodObj));
		g_listPodObjBlocks.AddTail((void*)g_pPodObjFree);
		nCount = g_dwPodPoolBlock;
		pObj = g_pPodObjFree;
		for (i = 0; i < nCount - 1; i++)
			pObj[i].m_pNextFree = &pObj[i + 1];
		pObj[nCount - 1].m_pNextFree = 0;
	}
	g_pPodObjFree = pObj->m_pNextFree;
	return pObj;
}

// hover.exe:0x42AF90
// FUNCTION: HOVER 0x42af90
void CPodObj::operator delete(void* p)
{
	CPodObj* pHead = g_pPodObjFree;
	g_pPodObjFree = (CPodObj*)p;
	((CPodObj*)p)->m_pNextFree = pHead;
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
void CPodObj::Think()
{
	CGameObj::Think();
}

// Player touched the pod (vtable slot 8): flag the view for a dashboard
// repaint, drop the pod from the document's active list and put it back
// on the draw list.
// hover.exe:0x4115D0
// FUNCTION: HOVER 0x4115d0
void CPodObj::OnTouch(CPlayer* pOther, DWORD dwB)
{
	Hover_GetView()->m_dwUnk051C = 1;
	theApp.m_pBumperDoc->RemoveFromMazeList(this);
	QueueBeginFrame();
	m_bHideOnMap = 1;
}

// Proximity test (vtable slot 10): is the point (dwA,dwB) within
// m_dwRadius + lRadius of the pod, measured with the max + min/2 metric?
// hover.exe:0x413E70
// FUNCTION: HOVER 0x413e70
int CPodObj::IsNear(POINT pt, long lRadius)
{
	long lRange;
	DWORD adwPos[2];
	long lDX;
	long lDY;
	long lMin;

	lRange = (long)m_dwRadius + lRadius;
	GetCellPos(adwPos);
	lDX = abs(pt.x - (long)adwPos[0]);
	lDY = abs(pt.y - (long)adwPos[1]);
	lMin = (lDY < lDX) ? lDY : lDX;
	return (lDY - (lMin >> 1) + lDX) < lRange;
}

// Pickup test (vtable slot 11): only the human player picks pods up
// (a CFlag is picked up by anyone). Tests the height bands for overlap
// and then both of the player's proximity points against the pod radius.
// hover.exe:0x40CB40
// FUNCTION: HOVER 0x40cb40
int CPodObj::Touches(DWORD dwUnk)
{
	PLAYERPROBE* pProbe;
	long lRange;
	long lZ;
	long lPodZ;
	long lPodTop;
	long lTop;
	POINT ptPos;
	long lDX;
	long lDY;
	long lMin;
	CPlayer* pPlayer = (CPlayer*)dwUnk;

	if (!pPlayer->IsKindOf(RUNTIME_CLASS(CHumanPlayer)) &&
		!IsKindOf(RUNTIME_CLASS(CFlag)))
		return 0;

	pProbe = pPlayer->GetProbe();
	lRange = (long)m_dwRadius + pProbe->lRadius;
	lZ = (long)pPlayer->m_dwAltitude;
	lPodZ = (long)m_dwZ;
	lPodTop = lPodZ + 0x40;
	lTop = lZ + 0x40;
	// one guard, so the two height tests share a single return-0 tail
	if (lTop < lPodZ || lPodTop < lZ)
		return 0;

	GetCellPos((DWORD*)&ptPos);
	lDX = abs(pProbe->ptFront.x - ptPos.x);
	lDY = abs(pProbe->ptFront.y - ptPos.y);
	lMin = (lDY < lDX) ? lDY : lDX;
	if (lDY - (lMin >> 1) + lDX < lRange)
		return 1;

	lDX = abs(pProbe->ptBack.x - ptPos.x);
	lDY = abs(pProbe->ptBack.y - ptPos.y);
	lMin = (lDY < lDX) ? lDY : lDX;
	return (lDY - (lMin >> 1) + lDX) < lRange;
}

// Register the pod with the collider (vtable slot 12): insert it into
// every region-matrix cell its radius touches.
// hover.exe:0x413180
// FUNCTION: HOVER 0x413180
int CPodObj::UpdateRegions(DWORD dwA)
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

	nCell = (int)pMgr->m_dwCellSize;
	GetCellPos(adwPos);
	nX2 = ((long)adwPos[0] + (long)m_dwRadius) / nCell;
	nY1 = ((long)adwPos[1] - (long)m_dwRadius) / nCell;
	nY2 = ((long)adwPos[1] + (long)m_dwRadius) / nCell;
	nX1 = ((long)adwPos[0] - (long)m_dwRadius) / nCell;
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
					if (pMgr->m_bQuerying != 0)
						pMgr->CollectCell(x, y);
					else
						pMgr->m_grid.AddNode(x, y, this);
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
void CPodObj::BeginFrame()
{
	CGameObj::BeginFrame();
	m_pSpriteTexture = m_pTexture;
	m_dwUnkA8 = m_dwUnkB4;
	m_dwUnk64 = 1;
	m_dwZ = m_dwZOffset + m_dwBaseZ;
}

// End of frame (vtable slot 16): create, update or drop the CMerlinDynamic
// sprite that represents the pod inside the level world.
// hover.exe:0x40E380
// FUNCTION: HOVER 0x40e380
void CPodObj::EndFrame()
{
	CBumperDoc* pDoc;
	CMerlinDynamic* pSprite;
	long lBase;
	DWORD adwPos[2];

	pDoc = theApp.m_pBumperDoc;
	if (m_bDead == 0)
	{
		if (m_bOnBoard != 0 && m_pMerlin == 0 && m_dwUnk64 != 0)
		{
			m_pMerlin = new CMerlinDynamic;
			pSprite = (CMerlinDynamic*)m_pMerlin;
			pSprite->m_pTexture = m_pSpriteTexture;
			pSprite->m_dwUnk60 = 0;
			(pDoc->m_pWorld)->AddSprite(pSprite);
			if (m_pMerlin != 0)
				((CMerlinDynamic*)m_pMerlin)->m_wFlags |= 8;
		}
		else if (m_bOnBoard == 0 && m_pMerlin != 0)
		{
			(pDoc->m_pWorld)->RemoveSprite((CMerlinDynamic*)m_pMerlin);
			m_pMerlin = 0;
		}
		pSprite = (CMerlinDynamic*)m_pMerlin;
		if (pSprite != NULL)
		{
			lBase = (long)m_dwZ;
			pSprite->m_wTop = (WORD)((long)m_dwHeight + lBase);
			if (lBase <= 0)
				lBase = 0;
			pSprite->m_wBottom = (WORD)lBase;
			pSprite->m_wHalfWidth = (WORD)m_dwHalfWidth;
			pSprite->m_wX = (WORD)GetFramePos(adwPos)[0];
			pSprite->m_wY = (WORD)GetFramePos(adwPos)[1];
		}
	}
	CGameObj::EndFrame();
}

// Link into the draw list (vtable slot 17); forwards to CGameObj.
// hover.exe:0x42AFB0
// FUNCTION: HOVER 0x42afb0
void CPodObj::KillObject()
{
	CGameObj::KillObject();
}

/////////////////////////////////////////////////////////////////////////////
// The power-up pods

// hover.exe:0x420FD0
// FUNCTION: HOVER 0x420fd0
CSmokeBombPod::CSmokeBombPod()
{
	char szName[0x34];

	m_bInUse = 0;
	m_bOnBoard = 0;
	m_pTexture = NULL;
	m_dwRadius = 0x40;
	wsprintfA(szName, "%s%02d", "POD00_", 0);
	g_mapUnk_4A1D30.Lookup(szName, (CObject*&)m_pTexture);
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
void CSmokeBombPod::Think()
{
	CGaugeFraction fix;
	CGaugeFraction fixOut;
	CSegmentGauge* pGauge;
	CBumperView* pView;

	if (m_bKilled == 0 && m_bInUse != 0)
	{
		m_dwTimeLeft--;
		fix = CGaugeFraction((WORD)m_dwTimeLeft, 0);
		*(long*)&fixOut = *(long*)&fix / (short)theApp.m_dwSmokePodTime;
		pGauge = &Hover_GetView()->m_gaugeSmoke;
		HOVER_INLINE_SUB_42A9F0(pGauge, fixOut);
		if (m_dwTimeLeft == 0)
		{
			pView = Hover_GetView();
			pView->m_bSmokeBlind = 0;
			(m_pOwner)->m_pSmokeBombPod = 0;
			theApp.m_pBumperDoc->RetireObject(this);
			if (theApp.m_bGameSounds != 0)
				PlayObjSound(0x18, 4, 5, 0);
			return;
		}
	}
	CPodObj::Think();
}

// Player touched the pod (slot 8): add it to the player's smoke-bomb
// stock (up to nine), take it out of the maze and drop a fresh smoke
// bomb pod somewhere else on the board.
// hover.exe:0x42A760
// FUNCTION: HOVER 0x42a760
void CSmokeBombPod::OnTouch(CPlayer* pOther, DWORD dwB)
{
	CPlayer* pPlayer;
	CSmokeBombPod* pNew;
	CBumperView* pView;

	// the "no more pods" message is the cold arm at the end of the
	// function in hover.exe, so it has to be the `else`
	if (pOther->m_listSmokeBombPods.GetCount() < 9)
	{
		CPodObj::OnTouch(pOther, dwB);
		m_pOwner = pOther;
		m_bOnBoard = 0;
		(theApp.m_pBumperDoc->m_pCollider)->RemoveObject(this);
		pPlayer = m_pOwner;
		if (pPlayer->m_bHuman != 0 && theApp.m_bGameSounds != 0)
			PlayObjSound(7, 1, 5, 0);
		(m_pOwner)->m_listSmokeBombPods.AddHead(this);
		pView = Hover_GetView();
		pView->m_ctrSmokeBombs.m_bDigitsDirty = 1;
		pView->m_ctrSmokeBombs.m_dwDisplayValue = (m_pOwner)->m_listSmokeBombPods.GetCount();
		pNew = new CSmokeBombPod;
		theApp.m_pBumperDoc->PlaceObject(pNew, (rand() % 4) == 0);
	}
	else if (pOther->m_bHuman != 0 && theApp.m_bGameSounds != 0)
	{
		PlayObjSound(5, 1, 5, 0);
	}
}

// Deploy (slot 9): the player drops the smoke bomb; it goes back into
// the maze, blanks the view's shade table and starts the timer.
// hover.exe:0x42A900
// FUNCTION: HOVER 0x42a900
void CSmokeBombPod::UsePod(DWORD dwUnk)
{
	CPlayer* pPlayer;
	POSITION pos;

	m_dwTimeLeft = theApp.m_dwSmokePodTime;
	m_bInUse = 1;
	Hover_GetView()->m_bSmokeBlind = 1;
	theApp.m_pBumperDoc->m_listActiveObjs.AddTail(this);
	pPlayer = m_pOwner;
	if (pPlayer->m_pSmokeBombPod != 0)
		theApp.m_pBumperDoc->RetireObject((CGameObj*)pPlayer->m_pSmokeBombPod);
	pos = (m_pOwner)->m_listSmokeBombPods.Find(this);
	(m_pOwner)->m_listSmokeBombPods.RemoveAt(pos);
	(m_pOwner)->m_pSmokeBombPod = this;
	Hover_GetView()->m_gaugeSmoke.SetFraction(CGaugeFraction(1.0));
	if (theApp.m_bGameSounds != 0)
		PlayObjSound(0x17, 4, 5, 0);
}

// Start/end of frame and draw-list linkage (slots 15/16/17): forwarded.
// hover.exe:0x421270
// FUNCTION: HOVER 0x421270
void CSmokeBombPod::BeginFrame()
{
	CPodObj::BeginFrame();
}

// hover.exe:0x421280
// FUNCTION: HOVER 0x421280
void CSmokeBombPod::EndFrame()
{
	CPodObj::EndFrame();
}

// hover.exe:0x42AA60
// FUNCTION: HOVER 0x42aa60
void CSmokeBombPod::KillObject()
{
	CGameObj::KillObject();
}

// hover.exe:0x421310
// FUNCTION: HOVER 0x421310
CJumpPod::CJumpPod()
{
	char szName[0x34];

	m_bInUse = 0;
	m_bOnBoard = 0;
	m_pTexture = NULL;
	m_dwRadius = 0x40;
	wsprintfA(szName, "%s%02d", "POD01_", 0);
	g_mapUnk_4A1D30.Lookup(szName, (CObject*&)m_pTexture);
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
void CJumpPod::Think()
{
	CPlayer* pPlayer;

	if (m_bKilled == 0 && m_bInUse != 0)
	{
		pPlayer = m_pOwner;
		if (pPlayer->m_bAirborne == 0)
		{
			pPlayer->m_pJumpPod = 0;
			theApp.m_pBumperDoc->RetireObject(this);
			return;
		}
	}
	CPodObj::Think();
}

// Player touched the pod (slot 8): add it to the player's jump-pad stock
// (up to nine) and drop a fresh jump pod somewhere else on the board.
// hover.exe:0x42AAC0
// FUNCTION: HOVER 0x42aac0
void CJumpPod::OnTouch(CPlayer* pOther, DWORD dwB)
{
	CPlayer* pPlayer;
	CJumpPod* pNew;
	CBumperView* pView;

	// the "no more pods" message is the cold arm at the end of the
	// function in hover.exe, so it has to be the `else`
	if (pOther->m_listJumpPods.GetCount() < 9)
	{
		CPodObj::OnTouch(pOther, dwB);
		m_pOwner = pOther;
		m_bOnBoard = 0;
		(theApp.m_pBumperDoc->m_pCollider)->RemoveObject(this);
		pPlayer = m_pOwner;
		if (pPlayer->m_bHuman != 0 && theApp.m_bGameSounds != 0)
			PlayObjSound(6, 1, 5, 0);
		(m_pOwner)->m_listJumpPods.AddHead(this);
		pView = Hover_GetView();
		pView->m_ctrJumpPods.m_bDigitsDirty = 1;
		pView->m_ctrJumpPods.m_dwDisplayValue = (m_pOwner)->m_listJumpPods.GetCount();
		pNew = new CJumpPod;
		theApp.m_pBumperDoc->PlaceObject(pNew, (rand() % 4) == 0);
	}
	else if (pOther->m_bHuman != 0 && theApp.m_bGameSounds != 0)
	{
		PlayObjSound(5, 1, 5, 0);
	}
}

// Deploy (slot 9): the player drops the jump pad under himself.
// hover.exe:0x42AC60
// FUNCTION: HOVER 0x42ac60
void CJumpPod::UsePod(DWORD dwUnk)
{
	CPlayer* pPlayer;
	POSITION pos;

	theApp.m_pBumperDoc->m_listActiveObjs.AddTail(this);
	m_bInUse = 1;
	pPlayer = m_pOwner;
	pPlayer->m_pJumpPod = this;
	pPlayer->m_bAirborne = 1;
	pPlayer->m_wTargetHeight += 0x100;
	pos = (m_pOwner)->m_listJumpPods.Find(this);
	(m_pOwner)->m_listJumpPods.RemoveAt(pos);
	if (theApp.m_bGameSounds != 0)
		PlayObjSound(0x15, 4, 5, 0);
}

// hover.exe:0x421300
// FUNCTION: HOVER 0x421300
void CJumpPod::BeginFrame()
{
	CPodObj::BeginFrame();
}

// hover.exe:0x4213B0
// FUNCTION: HOVER 0x4213b0
void CJumpPod::EndFrame()
{
	CPodObj::EndFrame();
}

// hover.exe:0x42ACE0
// FUNCTION: HOVER 0x42ace0
void CJumpPod::KillObject()
{
	CGameObj::KillObject();
}

// hover.exe:0x421440
// FUNCTION: HOVER 0x421440
CInviciblePod::CInviciblePod()
{
	char szName[0x34];

	m_bInUse = 0;
	m_bOnBoard = 0;
	m_pTexture = NULL;
	m_dwRadius = 0x40;
	wsprintfA(szName, "%s%02d", "POD04_", 0);
	g_mapUnk_4A1D30.Lookup(szName, (CObject*&)m_pTexture);
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
void CInviciblePod::Think()
{
	CGaugeFraction fix;
	CGaugeFraction fixOut;
	CSegmentGauge* pGauge;
	CPlayer* pPlayer;

	if (m_bKilled == 0 && m_bInUse != 0)
	{
		m_dwTimeLeft--;
		fix = CGaugeFraction((WORD)m_dwTimeLeft, 0);
		*(long*)&fixOut = *(long*)&fix / (short)theApp.m_dwInvinciblePodTime;
		pGauge = &Hover_GetView()->m_gaugeInvincible;
		HOVER_INLINE_SUB_42A9F0(pGauge, fixOut);
		if (m_dwTimeLeft == 0)
		{
			if (theApp.m_bGameSounds != 0)
				PlayObjSound(0xA, 1, 5, 0);
			pPlayer = m_pOwner;
			if (pPlayer->m_bGodMode == 0)
				pPlayer->m_bInvincible = 0;
			pPlayer->m_pInvinciblePod = 0;
			theApp.m_pBumperDoc->RetireObject(this);
			return;
		}
	}
	CPodObj::Think();
}

// Player touched the pod (slot 8): make the player invincible for
// theApp.m_dwInvinciblePodTime frames, cancelling any slow pod that was
// running.
// hover.exe:0x4181E0
// FUNCTION: HOVER 0x4181e0
void CInviciblePod::OnTouch(CPlayer* pOther, DWORD dwB)
{
	CPlayer* pPlayer;
	CInviciblePod* pNew;

	CPodObj::OnTouch(pOther, dwB);
	theApp.m_pBumperDoc->m_listActiveObjs.AddTail(this);
	m_pOwner = pOther;
	m_bOnBoard = 0;
	(theApp.m_pBumperDoc->m_pCollider)->RemoveObject(this);
	pPlayer = m_pOwner;
	if (pPlayer->m_bHuman != 0 && theApp.m_bGameSounds != 0)
		PlayObjSound(9, 1, 5, 0);
	if (m_bRespawn != 0)
	{
		pNew = new CInviciblePod;
		theApp.m_pBumperDoc->PlaceObject(pNew, (rand() % 4) == 0);
	}
	m_dwTimeLeft = theApp.m_dwInvinciblePodTime;
	m_bInUse = 1;
	(m_pOwner)->m_bInvincible = 1;
	if ((m_pOwner)->m_pInvinciblePod != 0)
		theApp.m_pBumperDoc->RetireObject(
			(CGameObj*)(m_pOwner)->m_pInvinciblePod);
	if ((m_pOwner)->m_pSlowPod != 0)
	{
		theApp.m_pBumperDoc->RetireObject(
			(CGameObj*)(m_pOwner)->m_pSlowPod);
		pPlayer = m_pOwner;
		pPlayer->m_pSlowPod = 0;
		pPlayer->m_fxMaxSpeed = pPlayer->m_fxBaseMaxSpeed;
		Hover_GetView()->m_gaugeSlow.SetFraction(CGaugeFraction(0, 0));
	}
	(m_pOwner)->m_pInvinciblePod = this;
}

// hover.exe:0x413E50
// FUNCTION: HOVER 0x413e50
void CInviciblePod::BeginFrame()
{
	CPodObj::BeginFrame();
}

// hover.exe:0x413E60
// FUNCTION: HOVER 0x413e60
void CInviciblePod::EndFrame()
{
	CPodObj::EndFrame();
}

// hover.exe:0x419DF0
// FUNCTION: HOVER 0x419df0
void CInviciblePod::KillObject()
{
	CGameObj::KillObject();
}

// hover.exe:0x421520
// FUNCTION: HOVER 0x421520
CTempWallPod::CTempWallPod()
{
	char szName[0x34];

	m_pForceField = 0;
	m_bInUse = 0;
	m_bOnBoard = 0;
	m_dwRadius = 0x40;
	m_pTexture = NULL;
	wsprintfA(szName, "%s%02d", "POD07_", 0);
	g_mapUnk_4A1D30.Lookup(szName, (CObject*&)m_pTexture);
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
void CTempWallPod::Think()
{
	CPlayer* pPlayer;
	CGaugeFraction fix;
	CGaugeFraction fixOut;
	CSegmentGauge* pGauge;

	if (m_bKilled == 0 && m_bInUse != 0)
	{
		pPlayer = m_pOwner;
		m_dwTimeLeft--;
		if (pPlayer->m_pTempWallPod == this)
		{
			fix = CGaugeFraction((WORD)m_dwTimeLeft, 0);
			*(long*)&fixOut = *(long*)&fix / (short)theApp.m_dwWallPodTime;
			pGauge = &Hover_GetView()->m_gaugeTempWall;
		HOVER_INLINE_SUB_42A9F0(pGauge, fixOut);
		}
		if (m_dwTimeLeft == 0)
		{
			if (m_pOwner->m_pTempWallPod == this)
				(m_pOwner)->m_pTempWallPod = 0;
			theApp.m_pBumperDoc->RetireObject(this);
			return;
		}
	}
	CPodObj::Think();
}

// Player touched the pod (slot 8). While the wall is up this is the wall
// itself being hit; otherwise the pod is added to the player's stock (up
// to nine) and a fresh one is dropped elsewhere.
// hover.exe:0x42B160
// FUNCTION: HOVER 0x42b160
void CTempWallPod::OnTouch(CPlayer* pOther, DWORD dwB)
{
	int nHit;
	WORD wTop;
	CBumperView* pView;
	CTempWallPod* pNew;

	if (m_bInUse != 0)
	{
		if (m_bSolidHit != 0)
		{
			nHit = pOther->BounceOffWall(this, dwB);
			if (pOther->m_bHuman != 0)
			{
				Hover_GetView()->m_dwUnk0518 = 1;
				if (nHit != 0 && theApp.m_bGameSounds != 0)
					PlayObjSound(0x12, 3, 5, 0);
			}
		}
		else
		{
			wTop = (WORD)((m_pForceField)->m_wTop + 0x40);
			if (((long)(short)wTop << 16) > (long)pOther->m_fxTargetHeight)
			{
				pOther->m_bOnTop = 1;
				pOther->m_wTargetHeightFrac = 0;
				pOther->m_wTargetHeight = wTop;
			}
		}
		return;
	}

	// the "no more pods" message is the cold arm at the end of the
	// function in hover.exe, so it has to be the `else`
	if (pOther->m_listTempWallPods.GetCount() < 9)
	{
		CPodObj::OnTouch(pOther, dwB);
		m_pOwner = pOther;
		m_bOnBoard = 0;
		(theApp.m_pBumperDoc->m_pCollider)->RemoveObject(this);
		if ((m_pOwner)->m_bHuman != 0 && theApp.m_bGameSounds != 0)
			PlayObjSound(8, 1, 5, 0);
		(m_pOwner)->m_listTempWallPods.AddHead(this);
		pView = Hover_GetView();
		pView->m_ctrTempWalls.m_bDigitsDirty = 1;
		pView->m_ctrTempWalls.m_dwDisplayValue = (m_pOwner)->m_listTempWallPods.GetCount();
		pNew = new CTempWallPod;
		theApp.m_pBumperDoc->PlaceObject(pNew, (rand() % 4) == 0);
	}
	else if (pOther->m_bHuman != 0 && theApp.m_bGameSounds != 0)
	{
		PlayObjSound(5, 1, 5, 0);
	}
}

// Deploy (slot 9): raise the "FORCE_FIELD" wall across the player's
// path. The wall is a CMerlinDynamic sprite built from the player's position
// and heading, registered with both the collider and the level world.
// hover.exe:0x42B3B0
// FUNCTION: HOVER 0x42b3b0
void CTempWallPod::UsePod(DWORD dwUnk)
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

	if (theApp.m_bGameSounds != 0)
		PlayObjSound(0x19, 4, 5, 0);
	theApp.m_pBumperDoc->m_listActiveObjs.AddTail(this);
	m_bInUse = 1;
	m_dwTimeLeft = theApp.m_dwWallPodTime;
	pPlayer = m_pOwner;
	{
		CTempWallPod** ppActiveWall = &pPlayer->m_pTempWallPod;
		if (*ppActiveWall != 0)
			*ppActiveWall = 0;
	}
	Hover_GetView()->m_gaugeTempWall.SetFraction(CGaugeFraction(1.0));

	pSprite = new CMerlinDynamic;
	m_pForceField = pSprite;
	g_mapUnk_4A1D30.Lookup("FORCE_FIELD", (CObject*&)pSprite->m_pTexture);
	pSprite->m_wHalfWidth = 0x190;
	pSprite->m_bBillboard = 0;
	pPlayer = m_pOwner;
	wBase = (WORD)pPlayer->m_dwAltitude - 0x40;
	pSprite->m_wBottom = wBase;
	pSprite->m_wTop = wBase + 0x100;
	pSprite->m_wX = (WORD)(m_pOwner)->GetCellPos(adwPt1)[0];
	pSprite->m_wY = (WORD)(m_pOwner)->GetCellPos(adwPt2)[1];

	pPlayer = m_pOwner;
	lAngle = (short)pPlayer->m_wHeading;
	dwDirX = g_adwSinTable[(((lAngle + 0x80) & 0x1FF) << 4) + 1];
	HOVER_FIXMULWP(dwDirX, 0x96);
	dwDirY = g_adwSinTable[((lAngle & 0x1FF) << 4) + 1];
	HOVER_FIXMULWP(dwDirY, 0x96);
	lAngle = (short)theApp.m_pBumperDoc->m_humanPlayer.m_wHeading;
	dwSideX = g_adwSinTable[((lAngle & 0x1FF) << 4) + 1];
	HOVER_FIXMULWP(dwSideX, 0xC8);
	dwSideY = g_adwSinTable[(((lAngle - 0x80) & 0x1FF) << 4) + 1];
	HOVER_FIXMULWP(dwSideY, 0xC8);

	pPlayer->GetFixedPos(adwPos);
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
	pSprite->Recalc();
	m_seg.SetEndpoints(adwPt1, adwPt3);
	pSprite->m_dwUnk60 = 0;
	(theApp.m_pBumperDoc->m_pCollider)->AddStatic(this);
	(theApp.m_pBumperDoc->m_pWorld)->AddSprite(pSprite);
	(m_pOwner)->m_pTempWallPod = this;
	pos = (m_pOwner)->m_listTempWallPods.Find(this);
	(m_pOwner)->m_listTempWallPods.RemoveAt(pos);
	QueueBeginFrame();
}

// Register with the collider (slot 12): while the wall is up, the cells
// covered by its sprite are claimed instead of the pod's own radius.
// hover.exe:0x421640
// FUNCTION: HOVER 0x421640
int CTempWallPod::UpdateRegions(DWORD dwA)
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
	if (m_bInUse != 0)
	{
		nCell = (int)pMgr->m_dwCellSize;
		pSprite = m_pForceField;
		nHalf = (short)pSprite->m_wHalfWidth;
		nX = (short)pSprite->m_wX;
		nY = (short)pSprite->m_wY;
		nX2 = (nX + nHalf) / nCell;
		nY1 = (nY - nHalf) / nCell;
		nY2 = (nY + nHalf) / nCell;
		nX1 = (nX - nHalf) / nCell;
		for (x = nX1; x <= nX2; x++)
		{
			for (y = nY1; y <= nY2; y++)
			{
				if (pMgr->m_bQuerying != 0)
					pMgr->CollectCell(x, y);
				else
					pMgr->m_grid.AddNode(x, y, this);
			}
		}
		return 1;
	}
	return CPodObj::UpdateRegions((DWORD)pMgr);
}

// Wall collision test (slot 11): while the wall is up, the player is
// tested against the sprite's height band and the pod's own segment;
// otherwise the CPodObj pickup test applies.
// hover.exe:0x41A3A0
// FUNCTION: HOVER 0x41a3a0
int CTempWallPod::Touches(DWORD dwUnk)
{
	CPlayer* pPlayer;
	CMerlinDynamic* pSprite;
	PLAYERPROBE* pProbe;
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
	if (m_bInUse != 0)
	{
		pProbe = pPlayer->GetProbe();
		pSprite = m_pForceField;
		lBottom = (short)pSprite->m_wBottom;
		lTopEdge = (short)pSprite->m_wTop;
		lZ = (long)pPlayer->m_dwAltitude;
		lTop = lZ + 0x40;
		if (lTopEdge >= lZ)
		{
			if (lBottom > lTop)
				return 0;
			// one shared body: the original's first test jumps
			// straight into the success tail (rule 38)
			if (m_seg.DistanceTo(&pProbe->ptFront, &dwOutC, &dwOutB, &dwOutA) <=
					pProbe->lRadius ||
				m_seg.DistanceTo(&pProbe->ptBack, &dwOutC, &dwOutB,
					&dwOutA) <= pProbe->lRadius)
			{
				m_bSolidHit = 1;
				return 1;
			}
		}
		// the field top is below the player: only a brush is possible
		else if (m_seg.DistanceTo(&pProbe->ptBack, &dwOutC, &dwOutB, &dwOutA) <=
					pProbe->lRadius ||
				m_seg.DistanceTo((POINT*)pPlayer->GetCellPos(adwPos), &dwOutC,
					&dwOutB, &dwOutA) <= pProbe->lRadius ||
				m_seg.DistanceTo(&pProbe->ptFront, &dwOutC, &dwOutB, &dwOutA) <=
					pProbe->lRadius)
		{
			m_bSolidHit = 0;
			return 1;
		}
		return 0;
	}
	return CPodObj::Touches((DWORD)pPlayer);
}

// hover.exe:0x421430
// FUNCTION: HOVER 0x421430
void CTempWallPod::BeginFrame()
{
	CPodObj::BeginFrame();
}

// End of frame (slot 16): drop the wall sprite from the level world
// once the pod has been released.
// hover.exe:0x4214E0
// FUNCTION: HOVER 0x4214e0
void CTempWallPod::EndFrame()
{
	if (m_bDead != 0 && m_pForceField != 0)
	{
		(theApp.m_pBumperDoc->m_pWorld)->RemoveSprite(m_pForceField);
		m_pForceField = 0;
	}
	CPodObj::EndFrame();
}

// hover.exe:0x42B690
// FUNCTION: HOVER 0x42b690
void CTempWallPod::KillObject()
{
	CGameObj::KillObject();
}

// hover.exe:0x421710
// FUNCTION: HOVER 0x421710
CSpeedPod::CSpeedPod()
{
	char szName[0x34];

	m_bInUse = 0;
	m_bOnBoard = 0;
	m_pTexture = NULL;
	m_dwRadius = 0x40;
	wsprintfA(szName, "%s%02d", "POD02_", 0);
	g_mapUnk_4A1D30.Lookup(szName, (CObject*&)m_pTexture);
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
void CSpeedPod::Think()
{
	CGaugeFraction fix;
	CGaugeFraction fixOut;
	CSegmentGauge* pGauge;
	CPlayer* pPlayer;

	if (m_bKilled == 0 && m_bInUse != 0)
	{
		m_dwTimeLeft--;
		fix = CGaugeFraction((WORD)m_dwTimeLeft, 0);
		*(long*)&fixOut = *(long*)&fix / (short)theApp.m_dwSpeedPodTime;
		pGauge = &Hover_GetView()->m_gaugeSpeedPod;
		HOVER_INLINE_SUB_42A9F0(pGauge, fixOut);
		if ((long)m_dwTimeLeft <= 0)
		{
			if (theApp.m_bGameSounds != 0)
				PlayObjSound(0xE, 1, 5, 0);
			pPlayer = m_pOwner;
			pPlayer->m_pSpeedPod = 0;
			pPlayer->m_fxMaxSpeed = pPlayer->m_fxBaseMaxSpeed;
			theApp.m_pBumperDoc->RetireObject(this);
			return;
		}
	}
	CPodObj::Think();
}

// Player touched the pod (slot 8): raise the player's speed limit for
// theApp.m_dwSpeedPodTime frames, cancelling any slow pod that was running.
// hover.exe:0x421070
// FUNCTION: HOVER 0x421070
void CSpeedPod::OnTouch(CPlayer* pOther, DWORD dwB)
{
	CPlayer* pPlayer;
	CSpeedPod* pNew;
	DWORD dwLimit;
	DWORD dwScale;

	CPodObj::OnTouch(pOther, dwB);
	m_pOwner = pOther;
	m_bOnBoard = 0;
	(theApp.m_pBumperDoc->m_pCollider)->RemoveObject(this);
	pPlayer = m_pOwner;
	if (pPlayer->m_bHuman != 0 && theApp.m_bGameSounds != 0)
		PlayObjSound(0xD, 1, 5, 0);
	if (m_bRespawn != 0)
	{
		pNew = new CSpeedPod;
		theApp.m_pBumperDoc->PlaceObject(pNew, (rand() % 4) == 0);
	}
	theApp.m_pBumperDoc->m_listActiveObjs.AddTail(this);
	m_bInUse = 1;
	m_dwTimeLeft = theApp.m_dwSpeedPodTime;
	pPlayer = m_pOwner;
	if (pPlayer->m_pSpeedPod != 0)
	{
		theApp.m_pBumperDoc->RetireObject((CGameObj*)pPlayer->m_pSpeedPod);
		(m_pOwner)->m_pSpeedPod = 0;
	}
	if ((m_pOwner)->m_pSlowPod != 0)
	{
		theApp.m_pBumperDoc->RetireObject(
			(CGameObj*)(m_pOwner)->m_pSlowPod);
		(m_pOwner)->m_pSlowPod = 0;
	}
	pPlayer = m_pOwner;
	dwLimit = pPlayer->m_fxBaseMaxSpeed;
	dwScale = pPlayer->m_pParams->m_adw[8];
	pPlayer->m_pSpeedPod = this;
	HOVER_FIXMULP(dwLimit, dwScale);
	pPlayer->m_fxMaxSpeed = dwLimit;
}

// hover.exe:0x414A40
// FUNCTION: HOVER 0x414a40
void CSpeedPod::BeginFrame()
{
	CPodObj::BeginFrame();
}

// hover.exe:0x414A70
// FUNCTION: HOVER 0x414a70
void CSpeedPod::EndFrame()
{
	CPodObj::EndFrame();
}

// hover.exe:0x421700
// FUNCTION: HOVER 0x421700
void CSpeedPod::KillObject()
{
	CGameObj::KillObject();
}

// hover.exe:0x4223C0
// FUNCTION: HOVER 0x4223c0
CMapEraserPod::CMapEraserPod()
{
	char szName[0x34];

	m_pOwner = 0;
	m_bInUse = 0;
	m_bOnBoard = 0;
	m_dwRadius = 0x40;
	m_pTexture = NULL;
	wsprintfA(szName, "%s%02d", "POD05_", 0);
	g_mapUnk_4A1D30.Lookup(szName, (CObject*&)m_pTexture);
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
void CMapEraserPod::Think()
{
	CPodObj::Think();
}

// Player touched the pod (slot 8): clear the "seen on the map" bit of
// every non-pod object the document still tracks, then release the pod.
// hover.exe:0x422470
// FUNCTION: HOVER 0x422470
void CMapEraserPod::OnTouch(CPlayer* pOther, DWORD dwB)
{
	CMapEraserPod* pNew;
	POSITION pos;
	CGameObj* pObj;

	CPodObj::OnTouch(pOther, dwB);
	m_bOnBoard = 0;
	m_pOwner = pOther;
	(theApp.m_pBumperDoc->m_pCollider)->RemoveObject(this);
	if (m_bRespawn != 0)
	{
		pNew = new CMapEraserPod;
		theApp.m_pBumperDoc->PlaceObject(pNew, (rand() % 4) == 0);
	}
	if ((m_pOwner)->m_bInvincible == 0)
	{
		if (theApp.m_bGameSounds != 0)
			PlayObjSound(0xF, 1, 5, 0);
		pos = theApp.m_pBumperDoc->m_listMazeObjs.GetHeadPosition();
		while (pos != NULL)
		{
			pObj = (CGameObj*)theApp.m_pBumperDoc->m_listMazeObjs.GetNext(pos);
			if (!pObj->IsKindOf(RUNTIME_CLASS(CPodObj)) && pObj->m_pMerlin != 0)
				// hover.exe clears the flag with a byte-wide AND
				*(BYTE*)&(pObj->m_pMerlin)->m_wFlags &= ~8;
			if (pObj->IsKindOf(RUNTIME_CLASS(CWall)) && pObj->m_pMerlin != 0)
				// hover.exe clears the flag with a byte-wide AND
				*(BYTE*)&(pObj->m_pMerlin)->m_wFlags &= ~8;
		}
	}
	theApp.m_pBumperDoc->RetireObject(this);
}

// hover.exe:0x41B2C0
// FUNCTION: HOVER 0x41b2c0
void CMapEraserPod::BeginFrame()
{
	CPodObj::BeginFrame();
}

// hover.exe:0x41B340
// FUNCTION: HOVER 0x41b340
void CMapEraserPod::EndFrame()
{
	CPodObj::EndFrame();
}

// hover.exe:0x422620
// FUNCTION: HOVER 0x422620
void CMapEraserPod::KillObject()
{
	CGameObj::KillObject();
}

// hover.exe:0x422630
// FUNCTION: HOVER 0x422630
CSlowPod::CSlowPod()
{
	char szName[0x34];

	m_bInUse = 0;
	m_bOnBoard = 0;
	m_pTexture = NULL;
	m_dwRadius = 0x40;
	wsprintfA(szName, "%s%02d", "POD03_", 0);
	g_mapUnk_4A1D30.Lookup(szName, (CObject*&)m_pTexture);
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
void CSlowPod::Think()
{
	CGaugeFraction fix;
	CGaugeFraction fixOut;
	CSegmentGauge* pGauge;
	CPlayer* pPlayer;

	if (m_bKilled == 0 && m_bInUse != 0)
	{
		m_dwTimeLeft--;
		fix = CGaugeFraction((WORD)m_dwTimeLeft, 0);
		*(long*)&fixOut = *(long*)&fix / (short)theApp.m_dwSlowPodTime;
		pGauge = &Hover_GetView()->m_gaugeSlow;
		HOVER_INLINE_SUB_42A9F0(pGauge, fixOut);
		if ((long)m_dwTimeLeft <= 0)
		{
			if (theApp.m_bGameSounds != 0)
				PlayObjSound(0xC, 1, 5, 0);
			pPlayer = m_pOwner;
			pPlayer->m_pSlowPod = 0;
			pPlayer->m_fxMaxSpeed = pPlayer->m_fxBaseMaxSpeed;
			theApp.m_pBumperDoc->RetireObject(this);
			return;
		}
	}
	CPodObj::Think();
}

// Player touched the pod (slot 8): slow the player down for
// theApp.m_dwSlowPodTime frames, cancelling any speed pod that was running.
// hover.exe:0x4226D0
// FUNCTION: HOVER 0x4226d0
void CSlowPod::OnTouch(CPlayer* pOther, DWORD dwB)
{
	CSlowPod* pNew;
	CPlayer* pPlayer;
	DWORD dwLimit;
	DWORD dwScale;

	CPodObj::OnTouch(pOther, dwB);
	m_bOnBoard = 0;
	m_pOwner = pOther;
	(theApp.m_pBumperDoc->m_pCollider)->RemoveObject(this);
	if (m_bRespawn != 0)
	{
		pNew = new CSlowPod;
		theApp.m_pBumperDoc->PlaceObject(pNew, (rand() % 4) == 0);
	}
	pPlayer = m_pOwner;
	if (pPlayer->m_bInvincible != 0)
	{
		theApp.m_pBumperDoc->RetireObject(this);
		return;
	}
	if (pPlayer->m_bHuman != 0 && theApp.m_bGameSounds != 0)
		PlayObjSound(0xB, 1, 5, 0);
	theApp.m_pBumperDoc->m_listActiveObjs.AddTail(this);
	m_bInUse = 1;
	m_dwTimeLeft = theApp.m_dwSlowPodTime;
	pPlayer = m_pOwner;
	if (pPlayer->m_pSlowPod != 0)
	{
		theApp.m_pBumperDoc->RetireObject((CGameObj*)pPlayer->m_pSlowPod);
		(m_pOwner)->m_pSlowPod = 0;
	}
	if ((m_pOwner)->m_pSpeedPod != 0)
	{
		theApp.m_pBumperDoc->RetireObject(
			(CGameObj*)(m_pOwner)->m_pSpeedPod);
		(m_pOwner)->m_pSpeedPod = 0;
		Hover_GetView()->m_gaugeSpeedPod.SetFraction(CGaugeFraction(0, 0));
	}
	pPlayer = m_pOwner;
	dwLimit = pPlayer->m_fxBaseMaxSpeed;
	dwScale = pPlayer->m_pParams->m_adw[7];
	pPlayer->m_pSlowPod = this;
	HOVER_FIXMULP(dwLimit, dwScale);
	pPlayer->m_fxMaxSpeed = dwLimit;
}

// hover.exe:0x41B3C0
// FUNCTION: HOVER 0x41b3c0
void CSlowPod::BeginFrame()
{
	CPodObj::BeginFrame();
}

// hover.exe:0x41B3D0
// FUNCTION: HOVER 0x41b3d0
void CSlowPod::EndFrame()
{
	CPodObj::EndFrame();
}

// hover.exe:0x422910
// FUNCTION: HOVER 0x422910
void CSlowPod::KillObject()
{
	CGameObj::KillObject();
}

/////////////////////////////////////////////////////////////////////////////
// CFlag

// hover.exe:0x421CD0
// FUNCTION: HOVER 0x421cd0
CFlag::CFlag(DWORD dwUnk)
{
	char szName[0x34];

	m_pOwner = 0;
	m_dwRadius = 0x80;
	m_pTexture = NULL;
	m_bRobotFlag = dwUnk;
	if (dwUnk == 0)
		wsprintfA(szName, "%s%02d", "FLG01_", 0);
	else
		wsprintfA(szName, "%s%02d", "FLG00_", 0);
	g_mapUnk_4A1D30.Lookup(szName, (CObject*&)m_pTexture);
	m_dwHeight = (short)m_pTexture->m_wHeight * 2;
	m_dwHalfWidth = (short)m_pTexture->m_wWidth;
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
void CFlag::Think()
{
	CPodObj::Think();
}

// The flag is captured (slot 8): it joins the player's flag list, the
// score/flag gauges are refreshed and the level-complete state is entered
// when the last flag of the level has been taken.
// hover.exe:0x41AE40
// FUNCTION: HOVER 0x41ae40
void CFlag::OnTouch(CPlayer* pOther, DWORD dwB)
{
	CRobotPlayer* pRobot;
	CBumperDoc* pDoc;
	CBumperView* pView;
	long lLimit;
	long lCount;

	CPodObj::OnTouch(pOther, dwB);
	m_pOwner = pOther;
	m_bOnBoard = 0;
	(theApp.m_pBumperDoc->m_pCollider)->RemoveObject(this);
	(m_pOwner)->m_listFlags.AddHead(this);

	if (m_bRobotFlag == 0)
	{
		// a robot took the human flag
		pRobot = (CRobotPlayer*)m_pOwner;
		if (pRobot->m_pTarget == this)
		{
			pRobot->m_pTarget = 0;
			pRobot->m_pState->ChangeState((CRobotState*)&pRobot->m_stSeek);
		}
		pDoc = theApp.m_pBumperDoc;
		pDoc->m_dwRobotFlags++;
		pView = Hover_GetView();
		{
			CFlagStrip* pGauge = &pView->m_flagsRobot;
			lLimit = (long)pGauge->m_dwSlots;
			pGauge->m_bFlagsDirty = 1;
			lCount = (long)pDoc->m_dwRobotFlags;
			if (lLimit <= lCount)
			{
				pGauge->m_dwFilled = lLimit;
			}
			else
			{
				pGauge->m_dwFilled = 0;
				if (lCount > 0)
					pGauge->m_dwFilled = lCount;
			}
		}
		if (LVLPARAM((int)g_dwLevel).m_dwUnk30 ==
			(int)theApp.m_pBumperDoc->m_dwRobotFlags)
		{
			Hover_GetView()->SetGameState(4);
			if (theApp.m_bEngineSound != 0)
				((CSoundSlot*)theApp.m_pBumperDoc->m_adwSounds[0x1A])->
					StopWave(5, 1);
			if (theApp.m_bGameSounds != 0)
				((CSoundSlot*)theApp.m_pBumperDoc->m_adwSounds[0x20])->
					PlayWave(8, 5, 0);
			return;
		}
		if (theApp.m_bGameSounds != 0)
			PlayObjSound(0x10, 2, 5, 0);
		return;
	}

	// the human took a robot flag
	pDoc = theApp.m_pBumperDoc;
	pDoc->m_dwScore +=
		LVLPARAM((int)g_dwLevel).m_anCaptureScore[theApp.m_dwSkill];
	pView = Hover_GetView();
	{
		CFlagStrip* pGauge = &pView->m_flagsPlayer;
		pGauge->m_bFlagsDirty = 1;
		lCount = (long)(m_pOwner)->m_listFlags.GetCount();
		lLimit = (long)pGauge->m_dwSlots;
		if (lCount >= lLimit)
		{
			pGauge->m_dwFilled = lLimit;
		}
		else
		{
			pGauge->m_dwFilled = 0;
			if (lCount > 0)
				pGauge->m_dwFilled = lCount;
		}
	}
	lCount = (long)theApp.m_pBumperDoc->m_dwScore;
	pView = Hover_GetView();
	{
		CDigitDisplay* pGauge2 = &pView->m_score;
		pGauge2->m_bDigitsDirty = 1;
		pGauge2->m_dwDisplayValue = lCount;
	}
	if (LVLPARAM((int)g_dwLevel).m_dwUnk30 ==
		(m_pOwner)->m_listFlags.GetCount())
	{
		Hover_GetView()->SetGameState(5);
		if (theApp.m_bEngineSound != 0)
			((CSoundSlot*)theApp.m_pBumperDoc->m_adwSounds[0x1A])->
				StopWave(5, 1);
		if (theApp.m_bGameSounds != 0)
			((CSoundSlot*)theApp.m_pBumperDoc->m_adwSounds[0x1F])->
				PlayWave(8, 5, 0);
		return;
	}
	if (theApp.m_bGameSounds != 0)
		PlayObjSound(0x11, 2, 5, 0);
}

// Pickup test (slot 11): a flag can only be picked up by the other team
// (m_bRobotFlag 0 = human flag, 1 = robot flag).
// hover.exe:0x414470
// FUNCTION: HOVER 0x414470
int CFlag::Touches(DWORD dwUnk)
{
	// two independent tests, not an if/else chain: the original re-reads
	// m_bRobotFlag for the second one (there is a call in between, rule 65)
	if (m_bRobotFlag == 0 &&
		((CPlayer*)dwUnk)->IsKindOf(RUNTIME_CLASS(CHumanPlayer)))
		return 0;
	if (m_bRobotFlag == 1 &&
		((CPlayer*)dwUnk)->IsKindOf(RUNTIME_CLASS(CRobotPlayer)))
		return 0;
	return CPodObj::Touches(dwUnk);
}

// hover.exe:0x414D00
// FUNCTION: HOVER 0x414d00
void CFlag::BeginFrame()
{
	CPodObj::BeginFrame();
}

// hover.exe:0x414D10
// FUNCTION: HOVER 0x414d10
void CFlag::EndFrame()
{
	CPodObj::EndFrame();
}

// hover.exe:0x42C170
// FUNCTION: HOVER 0x42c170
void CFlag::KillObject()
{
	CGameObj::KillObject();
}

/////////////////////////////////////////////////////////////////////////////
// CContainer

// hover.exe:0x41B160
// FUNCTION: HOVER 0x41b160
CContainer::CContainer(CPodObj* pObj, DWORD dwX, DWORD dwY, DWORD dwPt)
{
	char szName[0x34];

	m_pInner = pObj;
	SetCellPos(dwX, dwY);
	m_dwRadius = dwPt;
	// re-read through the member, not through the parameter: the original
	// keeps a pointer to m_pInner live across SetCellPos (rule 65)
	m_dwBaseZ = (m_pInner)->m_dwBaseZ;
	m_pTexture = NULL;
	wsprintfA(szName, "%s%02d", "POD06_", 0);
	g_mapUnk_4A1D30.Lookup(szName, (CObject*&)m_pTexture);
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
	if (m_pInner != 0)
		delete (CGameObj*)m_pInner;
}
#endif

// Player touched the container (slot 8): hand the touch on to the pod
// nested inside it and release the container, unless the player's stock
// of that pod type is already full.
// hover.exe:0x421E20
// FUNCTION: HOVER 0x421e20
void CContainer::OnTouch(CPlayer* pOther, DWORD dwB)
{
	CPodObj* pInner;
	int bRelease;

	pInner = m_pInner;
	if ((pInner->IsKindOf(RUNTIME_CLASS(CTempWallPod)) &&
			pOther->m_listTempWallPods.GetCount() == 9) ||
		((m_pInner)->IsKindOf(RUNTIME_CLASS(CSmokeBombPod)) &&
			pOther->m_listSmokeBombPods.GetCount() == 9) ||
		((m_pInner)->IsKindOf(RUNTIME_CLASS(CJumpPod)) &&
			pOther->m_listJumpPods.GetCount() == 9))
		bRelease = 0;
	else
		bRelease = 1;

	(m_pInner)->OnTouch(pOther, dwB);
	if (bRelease != 0)
	{
		m_pInner = 0;
		theApp.m_pBumperDoc->RetireObject(this);
	}
}

// hover.exe:0x414D20
// FUNCTION: HOVER 0x414d20
void CContainer::BeginFrame()
{
	CPodObj::BeginFrame();
}

// hover.exe:0x414D30
// FUNCTION: HOVER 0x414d30
void CContainer::EndFrame()
{
	CPodObj::EndFrame();
}

// hover.exe:0x421EC0
// FUNCTION: HOVER 0x421ec0
void CContainer::KillObject()
{
	CGameObj::KillObject();
}

/////////////////////////////////////////////////////////////////////////////
// CWall

// hover.exe:0x40E6A0
// FUNCTION: HOVER 0x40e6a0
CWall::CWall()
{
	m_pNextFree = 0;
}

// hover.exe:0x414D40
// FUNCTION: HOVER 0x414d40
int CWall::SetFace(CMerlinLine* pFace)
{
	int nRet = CGameObj::SetMerlin(pFace);
	m_seg.SetFromLine(pFace);
	return nRet;
}

// The class destructor has an empty body; only CGameObj::~CGameObj runs.
// Unlike the pod classes, CWall keeps a separate out-of-line ??1 (the
// collider builds CWall objects on the stack, see CCollider::IsPathClear
// and ProbeSegment, so the plain destructor is needed); the scalar deleting
// destructor at hover.exe:0x417BA0 is a second copy of this body followed
// by an inlined CWall::operator delete.
// hover.exe:0x40E650
// FUNCTION: HOVER 0x40e650
CWall::~CWall()
{
}

// hover.exe:0x42C660
// FUNCTION: HOVER 0x42c660
void CWall::Think()
{
	CGameObj::Think();
}

// The player hit the wall (slot 8): bounce him off it, or - when the
// wall is low enough to stand on - lift him onto its top edge.
// hover.exe:0x41B450
// FUNCTION: HOVER 0x41b450
void CWall::OnTouch(CPlayer* pOther, DWORD dwB)
{
	int nHit;
	WORD wTop;

	if (m_bSolidHit != 0)
	{
		nHit = pOther->BounceOffWall(this, dwB);
		if (pOther->m_bHuman != 0)
		{
			Hover_GetView()->m_dwUnk0518 = 1;
			if (nHit != 0 && theApp.m_bGameSounds != 0)
				PlayObjSound(0x12, 3, 5, 0);
		}
		return;
	}
	wTop = (WORD)(((CMerlinStatic*)m_pMerlin)->m_wTop + 0x40);
	if (((long)(short)wTop << 16) > (long)pOther->m_fxTargetHeight)
	{
		pOther->m_bOnTop = 1;
		pOther->m_wTargetHeightFrac = 0;
		pOther->m_wTargetHeight = wTop;
	}
}

// Proximity test (slot 10): is the point (dwA,dwB) closer than dwC to
// the wall segment?
// hover.exe:0x422B90
// FUNCTION: HOVER 0x422b90
int CWall::IsNear(POINT pt, long lRadius)
{
	DWORD dwOutA;
	DWORD dwOutB;
	DWORD dwOutC;

	return m_seg.DistanceTo(&pt, &dwOutC, &dwOutB, &dwOutA) <= lRadius;
}

// Player collision test (slot 11). A wall whose top is below the player
// only registers a brush (m_dwUnk2C = 0); otherwise a hit sets
// m_dwUnk2C so that OnTouch bounces him off.
// hover.exe:0x40D4B0
// FUNCTION: HOVER 0x40d4b0
int CWall::Touches(DWORD dwUnk)
{
	CPlayer* pPlayer;
	CMerlinStatic* pLine;
	PLAYERPROBE* pProbe;
	long lZ;
	long lTop;
	long lBottom;
	long lTopEdge;
	DWORD adwPos[2];
	DWORD dwOutA;
	DWORD dwOutB;
	DWORD dwOutC;

	pPlayer = (CPlayer*)dwUnk;
	pProbe = pPlayer->GetProbe();
	lZ = (long)pPlayer->m_dwAltitude;
	lTop = lZ + 0x40;
	pLine = (CMerlinStatic*)m_pMerlin;
	if (pLine == NULL)
		return 0;
	lBottom = (short)pLine->m_wBottom;
	lTopEdge = (short)pLine->m_wTop;
	if (lTopEdge >= lZ)
	{
		if (lBottom > lTop)
			return 0;
		// the two proximity points are tested with their own success
		// return, which is what makes hover.exe emit the
		// `m_dwUnk2C = 1; return 1` tail twice
		if (m_seg.DistanceTo(&pProbe->ptFront, &dwOutC, &dwOutB, &dwOutA) <=
				pProbe->lRadius)
		{
			m_bSolidHit = 1;
			return 1;
		}
		if (m_seg.DistanceTo(&pProbe->ptBack, &dwOutC, &dwOutB, &dwOutA) <=
				pProbe->lRadius)
		{
			m_bSolidHit = 1;
			return 1;
		}
	}
	// the wall top is below the player: only a brush is possible
	else if (m_seg.DistanceTo(&pProbe->ptBack, &dwOutC, &dwOutB, &dwOutA) <=
				pProbe->lRadius ||
			m_seg.DistanceTo((POINT*)pPlayer->GetCellPos(adwPos), &dwOutC,
				&dwOutB, &dwOutA) <= pProbe->lRadius ||
			m_seg.DistanceTo(&pProbe->ptFront, &dwOutC, &dwOutB, &dwOutA) <=
				pProbe->lRadius)
	{
		m_bSolidHit = 0;
		return 1;
	}
	return 0;
}

// Segment test (slot 13): does the segment dwA cross this wall, and does
// it pass through the wall's height band?
// hover.exe:0x40F7F0
// FUNCTION: HOVER 0x40f7f0
int CWall::IntersectLine(DWORD dwA, DWORD dwB, DWORD dwC, DWORD* pdwOut)
{
	CMerlinStatic* pLine;
	long lBottom;
	long lTop;
	DWORD adwPt[2];

	pLine = (CMerlinStatic*)m_pMerlin;
	if (pLine == NULL)
	{
		*pdwOut = 0;
		return 0;
	}
	if (pLine->IntersectSegment((CMerlinLine*)dwA, adwPt) != 0)
	{
		// the original re-reads m_pMerlin after the intersection call
		lTop = (short)((CMerlinStatic*)m_pMerlin)->m_wTop;
		lBottom = (short)((CMerlinStatic*)m_pMerlin)->m_wBottom;
		if (lBottom < (long)dwC && (long)dwB < lTop)
		{
			*pdwOut = 1;
			return 1;
		}
		*pdwOut = 0;
		return 1;
	}
	// no intersection: hover.exe reports 0 here, not 1
	*pdwOut = 0;
	return 0;
}

// hover.exe:0x42C670
// FUNCTION: HOVER 0x42c670
void CWall::BeginFrame()
{
	CGameObj::BeginFrame();
}

// hover.exe:0x42C680
// FUNCTION: HOVER 0x42c680
void CWall::EndFrame()
{
	CGameObj::EndFrame();
}

// hover.exe:0x42C690
// FUNCTION: HOVER 0x42c690
void CWall::KillObject()
{
	CGameObj::KillObject();
}

// The original disables frame-pointer omission for this function (full ebp
// frame without SEH, unlike its neighbors).
#pragma optimize("y", off)

// The wall registration below reads the segment endpoints and the collider's
// cell counts through inline accessors that return the pair by value: every
// use site therefore materializes an 8-byte temporary (edx:eax) and, when the
// first member is the one selected, copies it into a further stack slot.
// That is what gives hover.exe:0x40EF30 its 0x1B8-byte frame.
inline POINT SegPt0(const CSegment* pSeg)
{
	return *(POINT*)&pSeg->m_dwX1;
}

inline POINT SegPt1(const CSegment* pSeg)
{
	return *(POINT*)&pSeg->m_dwX2;
}

inline POINT ColliderCells(const CCollider* pMgr)
{
	return *(POINT*)&pMgr->m_dwCols;
}

// hover.exe:0x40EF30
// FUNCTION: HOVER 0x40ef30
int CWall::UpdateRegions(DWORD dwA)
{
	// 16.16 fixed-point value built word-wise by the original (signed:
	// the slope divides and multiplies are idiv/imul)
	union FIXEDPT
	{
		long dw;
		short s[2];
	};

	int nColB;
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
	int nRowA;
	int nRowB;
	int nXBound;
	int nYBound;
	int nCross;
	FIXEDPT fxSlope;

	nCell = ((CCollider*)dwA)->m_dwCellSize;
	nDX = SegPt0(&m_seg).x - SegPt1(&m_seg).x;
	g_dwWallRegisterCount++;
	nDY = SegPt0(&m_seg).y - SegPt1(&m_seg).y;

	if (nDY == 0 && nDX == 0)
	{
		nCol = SegPt0(&m_seg).x / nCell;
		nRow = SegPt0(&m_seg).y / nCell;
		if (((CCollider*)dwA)->m_bQuerying)
			((CCollider*)dwA)->CollectCell(nCol, nRow);
		else
			((CCollider*)dwA)->m_grid.AddNode(nCol, nRow, this);
		return 1;
	}

	if (abs(nDY) < abs(nDX))
	{
		FIXEDPT* pFx;
		FIXEDPT fxT;
		short sDelta;
		int nRowStep;
		int nRowDir;

		// walk the cell columns from the leftmost endpoint
		if (nDX < 0)
		{
			nStartX = SegPt0(&m_seg).x;
			nStartY = SegPt0(&m_seg).y;
			nEndX = SegPt1(&m_seg).x;
			nEndY = SegPt1(&m_seg).y;
		}
		else
		{
			nEndX = SegPt0(&m_seg).x;
			nEndY = SegPt0(&m_seg).y;
			nStartX = SegPt1(&m_seg).x;
			nStartY = SegPt1(&m_seg).y;
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
			if (((CCollider*)dwA)->m_bQuerying)
				((CCollider*)dwA)->CollectCell(nCol, nRow);
			else
				((CCollider*)dwA)->m_grid.AddNode(nCol, nRow, this);

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
				if (((CCollider*)dwA)->m_bQuerying)
					((CCollider*)dwA)->CollectCell(nCol, nRow);
				else
					((CCollider*)dwA)->m_grid.AddNode(nCol, nRow, this);
			}

			nXBound += nCell;
		}
	}
	else
	{
		FIXEDPT* pFx;
		FIXEDPT fxT;
		short sDelta;
		int nColStep;
		int nColDir;

		// walk the cell rows from the topmost endpoint
		if (nDY < 0)
		{
			nStartX = SegPt0(&m_seg).x;
			nStartY = SegPt0(&m_seg).y;
			nEndX = SegPt1(&m_seg).x;
			nEndY = SegPt1(&m_seg).y;
		}
		else
		{
			nEndX = SegPt0(&m_seg).x;
			nEndY = SegPt0(&m_seg).y;
			nStartX = SegPt1(&m_seg).x;
			nStartY = SegPt1(&m_seg).y;
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
			if (((CCollider*)dwA)->m_bQuerying)
				((CCollider*)dwA)->CollectCell(nCol, nRow);
			else
				((CCollider*)dwA)->m_grid.AddNode(nCol, nRow, this);

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
				if (((CCollider*)dwA)->m_bQuerying)
					((CCollider*)dwA)->CollectCell(nCol, nRow);
				else
					((CCollider*)dwA)->m_grid.AddNode(nCol, nRow, this);
			}

			nYBound += nCell;
		}
	}

	nCol = nColB;
	nRow = nRowB;
	if (((CCollider*)dwA)->m_bQuerying)
		((CCollider*)dwA)->CollectCell(nCol, nRow);
	else
		((CCollider*)dwA)->m_grid.AddNode(nCol, nRow, this);
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

	pWall = g_pWallFree;
	if (pWall == NULL)
	{
		g_pWallFree = (CWall*)::operator new(g_dwWallPoolBlock * sizeof(CWall));
		g_listWallBlocks.AddTail((void*)g_pWallFree);
		nCount = g_dwWallPoolBlock;
		pWall = g_pWallFree;
		for (i = 0; i < nCount - 1; i++)
			pWall[i].m_pNextFree = &pWall[i + 1];
		pWall[nCount - 1].m_pNextFree = 0;
	}
	g_pWallFree = pWall->m_pNextFree;
	return pWall;
}

// The original folds CWall::operator delete into the deleting destructor
// (hover.exe:0x417BA0); there is no standalone function in hover.exe.
void CWall::operator delete(void* p)
{
	CWall* pHead = g_pWallFree;
	g_pWallFree = (CWall*)p;
	((CWall*)p)->m_pNextFree = pHead;
}

/////////////////////////////////////////////////////////////////////////////
// CFloorObj

// hover.exe:0x421930
// FUNCTION: HOVER 0x421930
CFloorObj::CFloorObj()
	: m_listEdges(10)
{
	m_dwLowHeight = 0;
	m_dwHighHeight = 0;
	m_bSolidHit = 0;
	m_dwKind = 0xFFFFFFFF;
}

// hover.exe:0x41A610
// FUNCTION: HOVER 0x41a610
CFloorObj::CFloorObj(DWORD dwUnk)
	: m_listEdges(10)
{
	m_dwLowHeight = 0;
	m_dwHighHeight = 0;
	m_bSolidHit = 0;
	m_dwKind = dwUnk;
}

// Deletes every object still standing on the pad. The pool release
// happens in the compiler-generated deleting destructor (0x4219D0).
// hover.exe:0x421890
// FUNCTION: HOVER 0x421890
CFloorObj::~CFloorObj()
{
	POSITION pos;
	CGameObj* pObj;

	pos = m_listEdges.GetHeadPosition();
	while (pos != NULL)
	{
		pObj = (CGameObj*)m_listEdges.GetNext(pos);
		if (pObj != NULL)
			delete pObj;
	}
}

// Per-frame think (slot 7): pads do nothing.
// hover.exe:0x42B800
// FUNCTION: HOVER 0x42b800
void CFloorObj::Think()
{
}

// The player is standing on / bumping into the pad (slot 8). Type 6 is a
// two-level pad (the nearer of m_dwLowHeight and m_dwHighHeight wins),
// type 7 is a solid step, everything else drops the player back to the
// floor.
// hover.exe:0x40D730
// FUNCTION: HOVER 0x40d730
void CFloorObj::OnTouch(CPlayer* pOther, DWORD dwB)
{
	long lZ;
	long lLow;
	long lHigh;
	WORD wTop;

	if (pOther->m_bHuman != 0)
		Hover_GetView()->m_dwUnk0520 = 1;

	if (m_bSolidHit != 0)
	{
		if (pOther->BounceOffWall((CGameObj*)m_pHitEdge, dwB) != 0 &&
			pOther->m_bHuman != 0 && theApp.m_bGameSounds != 0)
			PlayObjSound(0x12, 3, 5, 0);
		if (m_dwKind == 7 && pOther->m_bAirborne == 0)
		{
			wTop = (WORD)(m_dwLowHeight + 0x40);
			pOther->m_bOnTop = 1;
			pOther->m_wTargetHeightFrac = 0;
			pOther->m_wTargetHeight = wTop;
		}
		return;
	}

	// Not a switch: hover.exe tests "!= 6" first so the case-7 test (and
	// its short body) falls straight through with no jump, while case 6's
	// longer body sits off in the else arm reached by a jump (mirrors the
	// inverted-if shape rule 148 uses for the sibling Touches; a plain
	// switch or an ascending if-chain both place case 6 first instead,
	// notes/perfect-match.md rule 243).
	if (m_dwKind != 6)
	{
		if (m_dwKind == 7)
		{
			if (pOther->m_bAirborne != 0)
				return;
			wTop = (WORD)(m_dwLowHeight + 0x40);
			pOther->m_bOnTop = 1;
			pOther->m_wTargetHeightFrac = 0;
			pOther->m_wTargetHeight = wTop;
			return;
		}
	}
	else
	{
		lZ = (long)pOther->m_dwAltitude;
		if ((long)m_dwHighHeight < lZ)
		{
			if (pOther->m_bAirborne != 0)
				return;
			wTop = (WORD)(m_dwHighHeight + 0x40);
			pOther->m_bOnTop = 1;
			pOther->m_wTargetHeightFrac = 0;
			pOther->m_wTargetHeight = wTop;
			return;
		}
		lLow = abs(lZ - (long)m_dwLowHeight + 0x40);
		lHigh = abs(lZ - (long)m_dwHighHeight);
		if (lLow < lHigh && m_dwLowHeight != 0)
		{
			// closer to the lower level: drop back to the floor
			pOther->m_wTargetHeightFrac = 0;
			pOther->m_bOnTop = 0;
			pOther->m_wTargetHeight = 0x40;
			pOther->m_bAirborne = 0;
			return;
		}
		pOther->m_wTargetHeight = (WORD)(m_dwHighHeight + 0x40);
		pOther->m_wTargetHeightFrac = 0;
		pOther->m_bOnTop = 1;
		pOther->m_bAirborne = 0;
		return;
	}

	if (pOther->m_bAirborne != 0)
		return;
	pOther->m_wTargetHeight = 0x40;
	pOther->m_wTargetHeightFrac = 0;
}

// Containment test (slot 10): the point (dwA,dwB) is on the pad when it
// is on the inner side of every one of the pad's edge objects.
// hover.exe:0x42B810
// FUNCTION: HOVER 0x42b810
int CFloorObj::IsNear(POINT pt, long lRadius)
{
	POSITION pos;
	CGameObj* pObj;

	pos = m_listEdges.GetHeadPosition();
	while (pos != NULL)
	{
		pObj = (CGameObj*)m_listEdges.GetNext(pos);
		if (pObj->m_seg.SideOf(&pt) == 0)
			return 0;
	}
	return 1;
}

// Player/pad interaction test (slot 11). m_dwKind selects the pad kind:
// -1 is a plain floor patch, 6 a two-level pad and 7 a step; the edge
// object that was hit is left in m_pHitEdge for OnTouch.
// hover.exe:0x40C620
// FUNCTION: HOVER 0x40c620
int CFloorObj::Touches(DWORD dwUnk)
{
	PLAYERPROBE* pProbe;
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
	pProbe = pPlayer->GetProbe();
	m_pHitEdge = 0;

	// hover.exe tests -1, then 7, then 6 and falls through into the
	// case 6 body, which is a nested chain of inverted equality tests
	// (a `switch` sorts its comparison chain ascending and would put
	// the -1 test last, notes/perfect-match.md rule 102).
	if (m_dwKind != (DWORD)-1)
	{
		if (m_dwKind != 7)
		{
			if (m_dwKind == 6)
			{
				if ((long)m_dwLowHeight > (long)pPlayer->m_dwAltitude + 0x40)
					return 0;

				bIn1 = 1;
				bIn2 = 1;
				bIn3 = 1;
				m_bSolidHit = 0;
				pos = m_listEdges.GetHeadPosition();
				while (pos != NULL)
				{
					pObj = (CGameObj*)m_listEdges.GetNext(pos);
					if (pObj->m_seg.SideOf(&pProbe->ptFront) == 0)
					{
						bIn1 = 0;
						break;
					}
				}
				if (bIn1 != 0)
					return 1;

				pos = m_listEdges.GetHeadPosition();
				while (pos != NULL)
				{
					pObj = (CGameObj*)m_listEdges.GetNext(pos);
					if (pObj->m_seg.SideOf(&pProbe->ptBack) == 0)
					{
						bIn2 = 0;
						break;
					}
				}
				if (bIn2 != 0)
					return 1;

				pos = m_listEdges.GetHeadPosition();
				while (pos != NULL)
				{
					pObj = (CGameObj*)m_listEdges.GetNext(pos);
					if (pObj->m_seg.SideOf((POINT*)pPlayer->GetCellPos(adwPos)) == 0)
					{
						bIn3 = 0;
						break;
					}
				}
				if (bIn3 != 0)
					return 1;

				if ((long)m_dwHighHeight < (long)pPlayer->m_dwAltitude)
					return 0;

				pos = m_listEdges.GetHeadPosition();
				while (pos != NULL)
				{
					pObj = (CGameObj*)m_listEdges.GetNext(pos);
					if (pObj->m_seg.DistanceTo(&pProbe->ptFront, &dwOutC, &dwOutB, &dwOutA) <=
							pProbe->lRadius ||
						pObj->m_seg.DistanceTo(&pProbe->ptBack, &dwOutC, &dwOutB, &dwOutA) <=
							pProbe->lRadius)
					{
						m_pHitEdge = (CWall*)pObj;
						m_bSolidHit = 1;
						return 1;
					}
				}
			}

			return 0;
		}
		else
		{
			if ((long)m_dwHighHeight >= (long)pPlayer->m_dwAltitude)
			{
				pos = m_listEdges.GetHeadPosition();
				while (pos != NULL)
				{
					pObj = (CGameObj*)m_listEdges.GetNext(pos);
					// The pad's edge objects carry their top height at
					// +0x98; the concrete class of the list elements is not
					// identified yet, so the field is reached by offset.
					if (*(long*)((BYTE*)pObj + 0x98) <=
						(long)pPlayer->m_dwAltitude)
						continue;
					if (pObj->m_seg.DistanceTo(&pProbe->ptFront, &dwOutC, &dwOutB,
							&dwOutA) <= pProbe->lRadius ||
						pObj->m_seg.DistanceTo(&pProbe->ptBack, &dwOutC, &dwOutB,
							&dwOutA) <= pProbe->lRadius)
					{
						m_pHitEdge = (CWall*)pObj;
						m_bSolidHit = 1;
						return 1;
					}
				}
			}
			m_bSolidHit = 0;
			pos = m_listEdges.GetHeadPosition();
			while (pos != NULL)
			{
				pObj = (CGameObj*)m_listEdges.GetNext(pos);
				if (pObj->m_seg.SideOf((POINT*)pPlayer->GetCellPos(adwPos)) == 0)
					return 0;
			}
			return 1;
		}
	}
	else
	{
		if ((long)m_dwLowHeight + 0x60 < (long)pPlayer->m_dwAltitude)
			return 0;
		bIn1 = 1;
		bIn2 = 1;
		m_bSolidHit = 0;
		pos = m_listEdges.GetHeadPosition();
		while (pos != NULL)
		{
			pObj = (CGameObj*)m_listEdges.GetNext(pos);
			if (pObj->m_seg.SideOf(&pProbe->ptFront) == 0)
				bIn1 = 0;
		}
		// Original quirk: `pos` is NULL by now and is never reset to the
		// head, so the second proximity point is in fact never tested.
		while (pos != NULL)
		{
			pObj = (CGameObj*)m_listEdges.GetNext(pos);
			if (pObj->m_seg.SideOf(&pProbe->ptBack) == 0)
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
int CFloorObj::UpdateRegions(DWORD dwA)
{
	CCollider* pMgr;
	POSITION pos;
	CGameObj* pObj;
	int nCell;
	long lMaxX;
	long lMaxY;
	long lMinX;
	long lMinY;
	int nX1;
	int nX2;
	int nY1;
	int nY2;
	int x;
	int y;

	pMgr = (CCollider*)dwA;
	nCell = (int)pMgr->m_dwCellSize;
	lMaxX = 0;
	lMaxY = 0;
	lMinX = 0x5A00;
	lMinY = 0x5A00;
	pos = m_listEdges.GetHeadPosition();
	while (pos != NULL)
	{
		pObj = (CGameObj*)m_listEdges.GetNext(pos);
		if (lMinX > SegPt0(&pObj->m_seg).x)
			lMinX = SegPt0(&pObj->m_seg).x;
		if (lMinY > SegPt0(&pObj->m_seg).y)
			lMinY = SegPt0(&pObj->m_seg).y;
		if (SegPt0(&pObj->m_seg).x > lMaxX)
			lMaxX = SegPt0(&pObj->m_seg).x;
		if (SegPt0(&pObj->m_seg).y > lMaxY)
			lMaxY = SegPt0(&pObj->m_seg).y;
		if (SegPt1(&pObj->m_seg).x < lMinX)
			lMinX = SegPt1(&pObj->m_seg).x;
		if (SegPt1(&pObj->m_seg).y < lMinY)
			lMinY = SegPt1(&pObj->m_seg).y;
		if (SegPt1(&pObj->m_seg).x > lMaxX)
			lMaxX = SegPt1(&pObj->m_seg).x;
		if (lMaxY < SegPt1(&pObj->m_seg).y)
			lMaxY = SegPt1(&pObj->m_seg).y;
	}
	nX1 = lMinX / nCell;
	nX2 = lMaxX / nCell;
	nY1 = lMinY / nCell;
	nY2 = lMaxY / nCell;
	for (x = nX1; x <= nX2; x++)
	{
		for (y = nY1; y <= nY2; y++)
		{
			if (pMgr->m_bQuerying != 0)
				pMgr->CollectCell(x, y);
			else
				pMgr->m_grid.AddNode(x, y, this);
		}
	}
	return 1;
}

// Segment test (slot 13): forwards the query to every object on the pad
// and ORs the results together.
// hover.exe:0x413220
// FUNCTION: HOVER 0x413220
int CFloorObj::IntersectLine(DWORD dwA, DWORD dwB, DWORD dwC, DWORD* pdwOut)
{
	POSITION pos;
	CGameObj* pObj;
	int bAny;
	DWORD dwFlag;

	bAny = 0;
	*pdwOut = 0;
	pos = m_listEdges.GetHeadPosition();
	while (pos != NULL)
	{
		pObj = (CGameObj*)m_listEdges.GetNext(pos);
		if (pObj != NULL)
		{
			bAny |= pObj->IntersectLine(dwA, dwB, dwC, &dwFlag);
			*pdwOut |= dwFlag;
		}
		if (bAny != 0 && *pdwOut != 0)
			return 1;
	}
	return bAny;
}

// hover.exe:0x42B860
// FUNCTION: HOVER 0x42b860
void CFloorObj::BeginFrame()
{
	CGameObj::BeginFrame();
}

// hover.exe:0x42B870
// FUNCTION: HOVER 0x42b870
void CFloorObj::EndFrame()
{
	CGameObj::EndFrame();
}

// hover.exe:0x42B880
// FUNCTION: HOVER 0x42b880
void CFloorObj::KillObject()
{
	CGameObj::KillObject();
}

// hover.exe:0x42B760
// FUNCTION: HOVER 0x42b760
void* CFloorObj::operator new(size_t nSize)
{
	CFloorObj* pObj;
	int nCount;
	int i;

	pObj = g_pFloorObjFree;
	if (pObj == NULL)
	{
		g_pFloorObjFree =
			(CFloorObj*)::operator new(g_dwFloorObjPoolBlock * sizeof(CFloorObj));
		g_listFloorObjBlocks.AddTail((void*)g_pFloorObjFree);
		nCount = g_dwFloorObjPoolBlock;
		pObj = g_pFloorObjFree;
		for (i = 0; i < nCount - 1; i++)
			pObj[i].m_pNextFree = &pObj[i + 1];
		pObj[nCount - 1].m_pNextFree = 0;
	}
	g_pFloorObjFree = pObj->m_pNextFree;
	return pObj;
}

// hover.exe:0x42B7E0
// FUNCTION: HOVER 0x42b7e0
void CFloorObj::operator delete(void* p)
{
	CFloorObj* pHead = g_pFloorObjFree;
	g_pFloorObjFree = (CFloorObj*)p;
	((CFloorObj*)p)->m_pNextFree = pHead;
}

/////////////////////////////////////////////////////////////////////////////
// The pad subclasses

// hover.exe:0x421820
// FUNCTION: HOVER 0x421820
CSledPad::CSledPad(DWORD dwUnk)
{
	m_dwSledDir = dwUnk;
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
void CSledPad::Think()
{
	CFloorObj::Think();
}

// Player entered the sled pad (slot 8): latch the pad direction into the
// player and start the sled timer.
// hover.exe:0x41A5C0
// FUNCTION: HOVER 0x41a5c0
void CSledPad::OnTouch(CPlayer* pOther, DWORD dwB)
{
	if (pOther->m_bInvincible != 0)
		return;
	pOther->m_dwSledAngle = m_dwSledDir;
	if (pOther->m_bHuman != 0)
		Hover_GetView()->m_dwUnk0520 = 1;
	if (pOther->m_dwSledTimer == (DWORD)-1)
		pOther->m_dwSledTimer = 0x1E;
}

// hover.exe:0x42B6D0
// FUNCTION: HOVER 0x42b6d0
void CSledPad::BeginFrame()
{
	CFloorObj::BeginFrame();
}

// hover.exe:0x42B6E0
// FUNCTION: HOVER 0x42b6e0
void CSledPad::EndFrame()
{
	CFloorObj::EndFrame();
}

// hover.exe:0x42B6F0
// FUNCTION: HOVER 0x42b6f0
void CSledPad::KillObject()
{
	CFloorObj::KillObject();
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
void CHoldPad::Think()
{
}

// Player entered the hold pad (slot 8): freeze him for 90 frames.
// hover.exe:0x42C1B0
// FUNCTION: HOVER 0x42c1b0
void CHoldPad::OnTouch(CPlayer* pOther, DWORD dwB)
{
	if (pOther->m_bInvincible != 0)
		return;
	if (pOther->m_bHuman != 0)
		Hover_GetView()->m_dwUnk0520 = 1;
	if (pOther->m_dwHoldTimer == (DWORD)-1)
		pOther->m_dwHoldTimer = 0x5A;
	pOther->m_bOnHoldPad = 1;
}

// hover.exe:0x42C200
// FUNCTION: HOVER 0x42c200
void CHoldPad::BeginFrame()
{
	CFloorObj::BeginFrame();
}

// hover.exe:0x42C210
// FUNCTION: HOVER 0x42c210
void CHoldPad::EndFrame()
{
	CFloorObj::EndFrame();
}

// hover.exe:0x42C220
// FUNCTION: HOVER 0x42c220
void CHoldPad::KillObject()
{
	CFloorObj::KillObject();
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
void CFlagRemover::Think()
{
}

// Player entered the flag-removal pad (slot 8): the flag he is carrying
// is scored and a fresh one is dropped back onto the board.
// hover.exe:0x422920
// FUNCTION: HOVER 0x422920
void CFlagRemover::OnTouch(CPlayer* pOther, DWORD dwB)
{
	CBumperDoc* pDoc;
	CBumperView* pView;
	CFlag* pFlag;
	long lCount;
	long lLimit;

	if (pOther->m_bHuman != 0)
		Hover_GetView()->m_dwUnk0520 = 1;
	if (pOther->m_bInvincible != 0)
		return;
	if (pOther->m_dwFlagRemoverState == (DWORD)-1)
	{
		pOther->m_dwFlagRemoverState = 1;
		if (pOther->m_listFlags.GetCount() != 0)
		{
			theApp.m_pBumperDoc->RetireObject(
				(CGameObj*)pOther->m_listFlags.GetHead());
			pOther->m_listFlags.RemoveHead();
			if (pOther->m_bHuman == 0)
			{
				pDoc = theApp.m_pBumperDoc;
				pDoc->m_dwRobotFlags--;
				pView = Hover_GetView();
				{
					CFlagStrip* pGauge = &pView->m_flagsRobot;
					lLimit = (long)pGauge->m_dwSlots;
					pGauge->m_bFlagsDirty = 1;
					lCount = (long)pDoc->m_dwRobotFlags;
					if (lLimit <= lCount)
					{
						pGauge->m_dwFilled = lLimit;
					}
					else
					{
						pGauge->m_dwFilled = 0;
						if (lCount > 0)
							pGauge->m_dwFilled = lCount;
					}
				}
				pFlag = new CFlag(0);
				theApp.m_pBumperDoc->PlaceObject(pFlag, 0);
				if (theApp.m_bGameSounds != 0)
					PlayObjSound(0x21, 0, 5, 0);
			}
			else
			{
				pView = Hover_GetView();
				{
					CFlagStrip* pGauge = &pView->m_flagsPlayer;
					lLimit = (long)pGauge->m_dwSlots;
					pGauge->m_bFlagsDirty = 1;
					lCount = (long)pOther->m_listFlags.GetCount();
					if (lLimit <= lCount)
					{
						pGauge->m_dwFilled = lLimit;
					}
					else
					{
						pGauge->m_dwFilled = 0;
						if (lCount > 0)
							pGauge->m_dwFilled = lCount;
					}
				}
				pFlag = new CFlag(1);
				theApp.m_pBumperDoc->PlaceObject(pFlag, 0);
				if (theApp.m_bGameSounds != 0)
					PlayObjSound(4, 0, 5, 0);
			}
		}
	}
	pOther->m_bOnFlagRemover = 1;
}

// hover.exe:0x42C550
// FUNCTION: HOVER 0x42c550
void CFlagRemover::BeginFrame()
{
	CFloorObj::BeginFrame();
}

// hover.exe:0x42C560
// FUNCTION: HOVER 0x42c560
void CFlagRemover::EndFrame()
{
	CFloorObj::EndFrame();
}

// hover.exe:0x42C570
// FUNCTION: HOVER 0x42c570
void CFlagRemover::KillObject()
{
	CFloorObj::KillObject();
}

/////////////////////////////////////////////////////////////////////////////
// CBeaconArray

// hover.exe:0x424210
// FUNCTION: HOVER 0x424210
CBeaconArray::~CBeaconArray()
{
}

/////////////////////////////////////////////////////////////////////////////
// CBeacon

// The original CBeacon constructor is folded into Hover_BuildBeacons
// (hover.exe:0x423EA0), which placement-constructs the object straight
// out of the class pool; there is no standalone constructor in hover.exe.
inline CBeacon::CBeacon(CObject* pTexture)
{
	WORD wUnk;

	m_wX = ((WORD*)pTexture)[0xA];	// texture +0x14
	wUnk = ((WORD*)pTexture)[0xB];		// texture +0x16
	m_pLocation = pTexture;
	m_bTargeted = 0;
	m_wXFrac = 0;
	m_wYFrac = 0;
	m_wY = wUnk;
}

// The class destructor is empty: the m_arrLinks member and the CGameObj base
// are destroyed by the compiler-generated calls that follow it inside the
// scalar deleting destructor, which also inlines CBeacon::operator delete
// (the pool push onto g_pBeaconFree). Rule 34: the marker pairs our ??1
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
int CBeacon::OnProbeHit(CGameObj* pObj)
{
	if (pObj->IsKindOf(RUNTIME_CLASS(CSledPad)) ||
		pObj->IsKindOf(RUNTIME_CLASS(CHoldPad)) ||
		pObj->IsKindOf(RUNTIME_CLASS(CFlagRemover)))
		m_bBlocked = 1;
	return m_bBlocked;
}

// The original inlines this pool allocator into Hover_BuildBeacons
// (hover.exe:0x423FD0-0x424036) and the matching operator delete into
// the deleting destructor (hover.exe:0x424180).
inline void* CBeacon::operator new(size_t nSize)
{
	CBeacon* pBeacon;
	int nCount;
	int i;

	nCount = g_dwBeaconPoolBlock;
	pBeacon = g_pBeaconFree;
	if (pBeacon == NULL)
	{
		g_pBeaconFree = (CBeacon*)::operator new(nCount * sizeof(CBeacon));
		g_listBeaconBlocks.AddTail((void*)g_pBeaconFree);
		nCount = g_dwBeaconPoolBlock;
		pBeacon = g_pBeaconFree;
		for (i = 0; i < nCount - 1; i++)
			pBeacon[i].m_pNextFree = &pBeacon[i + 1];
		pBeacon[nCount - 1].m_pNextFree = 0;
	}
	g_pBeaconFree = pBeacon->m_pNextFree;
	return pBeacon;
}

/////////////////////////////////////////////////////////////////////////////
// CSegment methods

// hover.exe:0x422BF0
// FUNCTION: HOVER 0x422bf0
void CSegment::SetEndpoints(DWORD* pdwPt1, DWORD* pdwPt2)
{
	*(POINT*)&m_dwX1 = *(POINT*)pdwPt1;
	*(POINT*)&m_dwX2 = *(POINT*)pdwPt2;
	Recalc();
}

// hover.exe:0x40E8E0
// FUNCTION: HOVER 0x40e8e0
CSegment* CSegment::SetFromLine(CObject* pObj)
{
	m_dwX1 = ((DWORD*)pObj)[5];	// pObj +0x14
	m_dwX2 = ((DWORD*)pObj)[7];	// pObj +0x1C
	m_dwY1 = ((DWORD*)pObj)[6];	// pObj +0x18
	m_dwY2 = ((DWORD*)pObj)[8];	// pObj +0x20
	Recalc();
	return this;
}

/////////////////////////////////////////////////////////////////////////////
// Beacon setup

// hover.exe:0x423EA0
// FUNCTION: HOVER 0x423ea0
void Hover_BuildBeacons()
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
	g_arrBeacons.SetSize(nBeacons, -1);
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
			g_arrBeacons[i++] = new CBeacon(pValue);
		}
	}
	Hover_LinkBeacons();
}

// hover.exe:0x423CD0
// FUNCTION: HOVER 0x423cd0
void Hover_LinkBeacons()
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

	nSize = g_arrBeacons.GetSize();
	pMgr = theApp.m_pBumperDoc->m_pCollider;
	for (i = 0; i < nSize - 1; i++)
	{
		pBeaconA = (CBeacon*)g_arrBeacons[i];
		pBeaconA->GetCellPos(adwA);
		for (j = i + 1; j < nSize; j++)
		{
			pBeaconB = (CBeacon*)g_arrBeacons[j];
			pBeaconB->GetCellPos(adwB);
			sA = (short)((WORD*)pBeaconA->m_pLocation)[0xD];	// texture +0x1A
			sB = (short)((WORD*)pBeaconB->m_pLocation)[0xD];
			if (abs(sA - sB) >= (int)theApp.m_pRobotAIParams->m_adw[8])	// +0x20
				continue;
			nHi = (sA > sB) ? sA : sB;
			nLo = ((sA < sB) ? sA : sB) + 0x64;
			if (nLo < nHi)
			{
				nHi = (nHi + nLo) / 2;
				nLo = nHi;
			}
			if (pMgr->IsPathClear(adwA[0], adwA[1], adwB[0], adwB[1],
					(DWORD)nHi, (DWORD)nLo,
					theApp.m_pRobotAIParams->m_adw[7]))	// +0x1C
			{
				pBeaconA->m_bBlocked = 0;
				pMgr->ProbeSegment(adwA[0], adwA[1], adwB[0], adwB[1],
					theApp.m_pRobotAIParams->m_adw[7], pBeaconA);
				if (pBeaconA->m_bBlocked == 0)
				{
					pBeaconA->m_arrLinks.Add(pBeaconB);
					pBeaconB->m_arrLinks.Add(pBeaconA);
				}
			}
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
// CCollider manager methods used by Hover_LinkBeacons.

// hover.exe:0x40C2B0
// FUNCTION: HOVER 0x40c2b0
int CCollider::IsPathClear(DWORD dwX1, DWORD dwY1, DWORD dwX2, DWORD dwY2,
	DWORD dwHi, DWORD dwLo, int nLimit)
{
	int nDist = (int)Hover_ApproxDistance((long)dwX1, (long)dwY1, (long)dwX2,
		(long)dwY2);

	if (nDist > nLimit)
		return 0;

	CMerlinLine line((short)dwX1, (short)dwY1, (short)dwX2, (short)dwY2);

	{
		CWall wall;

		wall.m_seg.SetFromLine(&line);

		m_arrQueryHits.SetSize(0, -1);
		m_bQuerying = 1;
		wall.UpdateRegions((DWORD)this);
		m_bQuerying = 0;
	}

	int i = 0;
	if (i < m_arrQueryHits.GetSize())
	{
		do
		{
			CGameObj* pObj = (CGameObj*)m_arrQueryHits.GetAt(i);
			DWORD dwResult;

			pObj->IntersectLine((DWORD)&line, dwHi, dwLo, &dwResult);
			if (dwResult != 0)
				return 0;
			i++;
		} while (i < m_arrQueryHits.GetSize());
	}

	return 1;
}

// hover.exe:0x411600
// FUNCTION: HOVER 0x411600
void CCollider::ProbeSegment(DWORD dwX1, DWORD dwY1, DWORD dwX2, DWORD dwY2,
	int nLimit, CBeacon* pBeacon)
{
	// Shadow copies of the three parameters that stay live across the
	// Hover_ApproxDistance call (dwY1 does not and is left alone): declaring
	// them as locals rather than reading the stack parameters directly
	// is what gets dwX2 into the original's ebx (notes/perfect-match.md
	// rule 207's lever, extended from one shadowed value to three).
	DWORD dwLX1 = dwX1;
	DWORD dwLX2 = dwX2;
	DWORD dwLY2 = dwY2;
	int nDist = (int)Hover_ApproxDistance((long)dwLX1, (long)dwY1, (long)dwLX2,
		(long)dwLY2);

	if (nDist > nLimit)
	{
		dwLX2 = dwLX1 + ((int)dwLX2 - (int)dwLX1) * nLimit / nDist;
		dwLY2 = dwY1 + ((int)dwLY2 - (int)dwY1) * nLimit / nDist;
	}

	CMerlinLine line((short)dwLX1, (short)dwY1, (short)dwLX2, (short)dwLY2);

	{
		CWall wall;

		wall.m_seg.SetFromLine(&line);

		m_arrQueryHits.SetSize(0, -1);
		m_bQuerying = 1;
		wall.UpdateRegions((DWORD)this);
		m_bQuerying = 0;
	}

	for (int i = 0; i < m_arrQueryHits.GetSize(); i++)
	{
		CGameObj* pObj = (CGameObj*)m_arrQueryHits.GetAt(i);
		DWORD dwResult;

		if (pObj->IntersectLine((DWORD)&line, 0, 0, &dwResult))
		{
			if (pBeacon->OnProbeHit(pObj))
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
	m_wBottom = 0;
	m_wTop = 0x80;
	m_dwUnk60 = 1;
	m_wHalfWidth = 0x80;
	m_bBillboard = 1;
	m_wY = 0;
	m_wX = 0;
	m_strTexName.Empty();
	m_pTexture = NULL;
	m_bMirror = 0;
}

// Serialize (vtable slot 2): the texture name, the height/scale/position
// words and the m_dwUnk60 flag byte. On load the sprite registers itself
// in g_mapDynamicSprites under its object name and resolves its texture name
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
		*pAr << m_strTexName;
		pAr->WriteWord(m_wBottom);
		pAr->WriteWord(m_wTop);
		pAr->WriteWord(m_wHalfWidth);
		pAr->WriteWord(m_wX);
		pAr->WriteWord(m_wY);
		pAr->WriteByte((BYTE)m_dwUnk60);
		pAr->WriteWord((WORD)0);
	}
	else
	{
		if (!m_strName.IsEmpty())
			g_mapDynamicSprites[m_strName] = this;
		*pAr >> m_strTexName;
		if (strcmp(m_strTexName, "") != 0)
		{
			if (!g_mapUnk_4A1D30.Lookup(m_strTexName,
					(CObject*&)m_pTexture))
				m_pTexture = NULL;
		}
		pAr->ReadWord(&m_wBottom);
		pAr->ReadWord(&m_wTop);
		pAr->ReadWord(&m_wHalfWidth);
		pAr->ReadWord(&m_wX);
		pAr->ReadWord(&m_wY);
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
// CSegment geometry helpers used by the maze objects

// Side-of-line test: nonzero when pPt is on (or in front of) the
// segment's positive side.
// hover.exe:0x40D2F0
// FUNCTION: HOVER 0x40d2f0
int CSegment::SideOf(const POINT* pPt)
{
	long lCross;

	// The members are DWORD in this reconstruction; going through long
	// locals is what makes VC 2.2 emit the original's xor/setns instead
	// of a branch (notes/perfect-match.md rule 10).
	long lX0 = (long)m_dwX1;
	long lY0 = (long)m_dwY1;
	long lDX = (long)m_dwDeltaX;
	long lDY = (long)m_dwDeltaY;

	lCross = (lY0 - pPt->y) * lDX + (pPt->x - lX0) * lDY;
	return lCross >= 0;
}

/////////////////////////////////////////////////////////////////////////////
// CBumperDoc helpers reached from the maze objects (declared in
// bumperdoc.h; defined here because they only serve the pod classes)

// Drops pObj from the document's active-pod list.
// hover.exe:0x410250
// FUNCTION: HOVER 0x410250
int CBumperDoc::RemoveFromMazeList(CObject* pObj)
{
	POSITION pos;

	pos = m_listMazeObjs.Find(pObj);
	if (pos != NULL)
		m_listMazeObjs.RemoveAt(pos);
	return pos != NULL;
}

/////////////////////////////////////////////////////////////////////////////
// The 16.16 fixed-point gauge values (declared in bumpview.h)

// hover.exe:0x42A9D0
// FUNCTION: HOVER 0x42a9d0
CGaugeFraction::CGaugeFraction(double dbl)
{
	*(long*)this = (long)(dbl * 65536.0);
}


// Lights the first clamp(fix * m_dwSegments, 0, m_dwSegments) gauge
// segments.
// hover.exe:0x42A9F0
// FUNCTION: HOVER 0x42a9f0
void CSegmentGauge::SetFraction(CGaugeFraction fix)
{
	long lSegs;
	long lIndex;

	*(long*)&fix += 0x288;
	HOVER_FIXMULWP(*(DWORD*)&fix, (WORD)m_dwSegments);
	lIndex = (short)fix.m_wInt;
	lSegs = (long)m_dwSegments;
	if (lIndex > lSegs)
		lIndex = lSegs;
	if (lIndex >= lSegs)
	{
		m_dwLevel = lSegs;
	}
	else if (lIndex <= 0)
	{
		m_dwLevel = 0;
	}
	else
	{
		m_dwLevel = lIndex;
	}
}

/////////////////////////////////////////////////////////////////////////////
// CBumperView game-state entry (declared in bumpview.h; defined here
// because it is only reached from the flag capture above)

// hover.exe:0x427CF0
// FUNCTION: HOVER 0x427cf0
void CBumperView::SetGameState(DWORD dwState)
{
	theApp.m_dwGameState = dwState;
	if (dwState == 2)
		m_dwStateCountdown = 0x28;
	else
		m_dwStateCountdown = 0x3C;
}
