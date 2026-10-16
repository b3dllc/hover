// gameobj.h : interface of the CGameObj / CPlayer / CHumanPlayer classes
//
// Skeleton reconstructed from hover.exe:
//   CRuntimeClass "CGameObj" at 0x4C52C0 (size 0x84, DYNAMIC, base CObject)
//   CRuntimeClass "CPlayer" at 0x4C53F8 (size 0x1FC, DYNAMIC, base CGameObj)
//   CRuntimeClass "CHumanPlayer" at 0x4C5488 (size 0x1FC, DYNAMIC, base CPlayer)
//   vtables at 0x4BC380 / 0x4BC910 / 0x4BCBE0 (slots 0-4 are the CObject
//   set, slots 5-11 are the game virtuals declared below; the original
//   vtables run further — CGameObj to slot 0x11, CPlayer/CHumanPlayer add
//   slots 0x12-0x13, where slot 0x13 (0x4089E0) returns &m_probe —
//   reached through explicit vtable-slot calls where needed)
//
// Note on member offsets: the comments give each member's offset within the
// class in hover.exe.

#ifndef __GAMEOBJ_H__
#define __GAMEOBJ_H__

#include "texture.h"	// CMerlinObject (the base of CMerlinLine) and
						// CMerlinTexture

/////////////////////////////////////////////////////////////////////////////
// CSegment: small value struct embedded in CGameObj at offset 0x04 (and
// used elsewhere). Stores two endpoints (+0x00..+0x0C) plus values
// recomputed from them by Recalc (+0x10..+0x1C).
// ctor hover.exe:0x40D320, sizeof = 0x20.

class CSegment
{
public:
	CSegment();			// hover.exe:0x40D320

	// hover.exe:0x42C820 recompute m_dwDeltaX..m_dwAngle from endpoints
	void Recalc();

	// hover.exe:0x422BF0 set the endpoints from two point-pair pointers
	// and recompute the derived values
	void SetEndpoints(DWORD* pdwPt1, DWORD* pdwPt2);

	// hover.exe:0x40E8E0 set the endpoints from the +0x14..+0x20 dwords
	// of pObj (a CMerlinLine in the collider paths) and recompute;
	// returns this
	CSegment* SetFromLine(CObject* pObj);

	// hover.exe:0x40D2F0 side-of-line test: nonzero when the point
	// pPt is on the positive side of the segment
	int SideOf(const POINT* pPt);

	// hover.exe:0x40C9B0 distance of the point pPt from the
	// segment. *pdwA is nonzero when the foot of the perpendicular falls
	// inside the segment, *pdwB is the side the point is on and *pdwC is
	// nonzero when the point lies beyond the first endpoint.
	int DistanceTo(const POINT* pPt, DWORD* pdwA, DWORD* pdwB,
		DWORD* pdwC);

	DWORD m_dwX1;		// +0x00 = 0
	DWORD m_dwY1;		// +0x04 = 0
	DWORD m_dwX2;		// +0x08 = 0
	DWORD m_dwY2;		// +0x0C = 0
	DWORD m_dwDeltaX;		// +0x10 = m_dwX2 - m_dwX1 (computed)
	DWORD m_dwDeltaY;		// +0x14 = m_dwY2 - m_dwY1 (computed)
	DWORD m_dwDistSq;		// +0x18 = dx*dx + dy*dy (computed)
	DWORD m_dwAngle;		// +0x1C = angle (computed, via hover.exe:0x4386D0)
};

/////////////////////////////////////////////////////////////////////////////
// CMerlinLine: the base of every geometric Merlin object -- a line segment
// plus its bounding box and deltas (CRuntimeClass "CMerlinLine" at
// 0x4C5248, size 0x44, schema 1, base CMerlinObject; vtable at
// hover.exe:0x4BC110). Used directly by the collider wall-registration
// paths and by the renderer's phase lines.

class CMerlinLine : public CMerlinObject
{
	DECLARE_SERIAL(CMerlinLine)

public:
	CMerlinLine();											// hover.exe:0x401160
	CMerlinLine(short sX1, short sY1, short sX2, short sY2);	// hover.exe:0x40F760
	virtual ~CMerlinLine();									// hover.exe:0x401210

	// hover.exe:0x414740 (vtable slot 2) serialize the four endpoint
	// words and the trailing comment blob; recomputes the derived values
	// after loading
	virtual void Serialize(CArchive& ar);

	// hover.exe:0x4010C0 recompute min/max, deltas and dist-sq from
	// the current endpoints and update m_wFlags.
	void Recalc();

	// hover.exe:0x401260 intersect this segment with pLine, writing the
	// crossing point into pdwOut[0..1]; returns 0 when the segments do
	// not cross, 1 when pLine crosses this segment from the front and
	// 0xFFFF when it crosses it from the back. Also reached through the
	// maze faces (CMerlinStatic) and sprites (CMerlinDynamic), which
	// derive from this class.
	WORD IntersectSegment(CMerlinLine* pLine, DWORD* pdwOut);

// Member variables (hover.exe offsets in comments). The endpoints are
// stored as dwords but only their low words are serialized (see
// CMerlinLine::Serialize).
	DWORD m_dwX1;			// +0x14
	DWORD m_dwY1;			// +0x18
	DWORD m_dwX2;			// +0x1C
	DWORD m_dwY2;			// +0x20
	// Signed: the level loader's pad-flag fixups read it with
	// `movsx eax, word ptr [eax+0x24]` (hover.exe:0x41758C etc.).
	short m_wFlags;			// +0x24
	char m_pad26[0x2];		// +0x26
	DWORD m_dwMinX;			// +0x28
	DWORD m_dwMaxX;			// +0x2C
	DWORD m_dwMinY;			// +0x30
	DWORD m_dwMaxY;			// +0x34
	DWORD m_dwDeltaX;		// +0x38
	DWORD m_dwDeltaY;		// +0x3C
	DWORD m_dwDistSq;		// +0x40
};							// sizeof = 0x44

/////////////////////////////////////////////////////////////////////////////
// CMerlinStatic: one static face of the maze -- the CMerlinLine base is
// the wall segment in plan view, the six CStrings name the textures of its
// six texture slots and the +0xA4/+0xA6 words are the signed heights of
// its low and high edge. CRuntimeClass "CMerlinStatic" at 0x4C5220 (size
// 0xC4, schema 1, base CMerlinLine); vtable at hover.exe:0x4BC078.
// The objects live in CMerlinWorld::m_arrStatics and, by name, in
// g_mapUnk_4A4578; the renderer reaches them through RENDEROBJ::m_pSource
// and CWall::m_pMerlin.

class CMerlinStatic : public CMerlinLine
{
	DECLARE_SERIAL(CMerlinStatic)

public:
	CMerlinStatic();		// hover.exe:0x419B30
	// hover.exe:0x419CF0 is the compiler-generated scalar deleting
	// destructor (six CString destructions plus the base); the class has
	// no destructor body of its own.
	virtual ~CMerlinStatic() {}

	// hover.exe:0x419E00 (vtable slot 2) serialize the six texture names,
	// the edge words and the flag bytes; on load, register the face in
	// g_mapUnk_4A4578 and resolve the six texture names
	virtual void Serialize(CArchive& ar);

// Member variables (hover.exe offsets in comments)
	CString m_strFrontTopTex;			// +0x44 texture name of m_pFrontTopTex
	CString m_strBackTopTex;			// +0x50 texture name of m_pBackTopTex
	CString m_strFrontTex;			// +0x5C front texture name ("..DECAL_"
								//   on the pad faces)
	CString m_strBackTex;			// +0x68 back texture name
	CString m_strFrontBottomTex;			// +0x74 texture name of m_pFrontBottomTex
	CString m_strBackBottomTex;			// +0x80 texture name of m_pBackBottomTex
	CMerlinTexture* m_pFrontTopTex;	// +0x8C = 0 resolved texture
	CMerlinTexture* m_pBackTopTex;	// +0x90 = 0 resolved texture
	CMerlinTexture* m_pFrontTex;	// +0x94 = 0 front face texture
	CMerlinTexture* m_pBackTex;	// +0x98 = 0 back face texture
	CMerlinTexture* m_pFrontBottomTex;	// +0x9C = 0 resolved texture
	CMerlinTexture* m_pBackBottomTex;	// +0xA0 = 0 resolved texture
	WORD m_wBottom;				// +0xA4 = 0 low edge height (signed)
	WORD m_wTop;				// +0xA6 = 0x80 high edge height (signed)
	WORD m_wUnkA8;				// +0xA8 = 0
	WORD m_wUnkAA;				// +0xAA = 0
	WORD m_wUnkAC;				// +0xAC = 0
	WORD m_wUnkAE;				// +0xAE (never touched)
	DWORD m_bFrontTransparent;			// +0xB0 = 0 front face has pixel data
	DWORD m_bBackTransparent;			// +0xB4 = 0 back face has pixel data
	WORD m_wFrontUOrg;				// +0xB8 = 0 front texture u origin
	WORD m_wBackUOrg;				// +0xBA = 0 back texture u origin
	DWORD m_bSolid;			// +0xBC = 1 nonzero enables the pad
								//   registration in the level loader
	DWORD m_dwUnkC0;			// +0xC0 = 0
};								// sizeof = 0xC4

/////////////////////////////////////////////////////////////////////////////
// CMerlinDynamic: the billboard sprite object -- the CMerlinLine base is
// the camera-facing segment, rebuilt each frame by CSpanRasterizer::RenderWorld
// from the world position (m_wX/m_wY) and the m_wHalfWidth scale so
// the sprite always faces the viewer. CRuntimeClass "CMerlinDynamic" at
// 0x4C5608 (size 0x70, schema 1, base CMerlinLine); vtable at
// hover.exe:0x4BD348. The objects live in CMerlinWorld::m_arrSprites and,
// by name, in g_mapDynamicSprites; the pods, players and the temporary wall
// create theirs out of a pool of g_dwSpritePoolBlock objects whose blocks are
// kept in g_listMerlinDynamicBlocks and whose free list is threaded through
// m_dwTopDZ.

class CMerlinDynamic : public CMerlinLine
{
	DECLARE_SERIAL(CMerlinDynamic)

public:
	CMerlinDynamic();			// hover.exe:0x4132A0

	// hover.exe:0x42CC40 (vtable slot 2) serialize the texture name, the
	// height/scale/position words and the m_dwUnk60 flag byte; on load,
	// register the sprite in g_mapDynamicSprites and resolve the texture name
	virtual void Serialize(CArchive& ar);

	static void* operator new(size_t nSize);	// hover.exe:0x42CBC0
	// Placement form, needed by the IMPLEMENT_SERIAL construction thunk
	// (which is `new(p) CMerlinDynamic`): the class-scope operator new
	// above would otherwise hide MFC's global one.
	static void* operator new(size_t, void* p) { return p; }
	// The matching deallocator pushes the object back onto the pool free
	// list; the original inlines it into the scalar deleting destructor
	// at hover.exe:0x40F920.
	static void operator delete(void* p)
	{
		((CMerlinDynamic*)p)->m_pNextFree = g_pMerlinDynamicFree;
		g_pMerlinDynamicFree = (CMerlinDynamic*)p;
	}

// Member variables (hover.exe offsets in comments)
	CString m_strTexName;			// +0x44 texture name
	CMerlinTexture* m_pTexture;	// +0x50 = 0 current animation frame
								//   (advanced by SubmitSegment through
								//   CMerlinTexture::m_pNextFrame)
	WORD m_wBottom;				// +0x54 = 0 base height
	WORD m_wTop;				// +0x56 = 0x80 top height
	WORD m_wHalfWidth;				// +0x58 = 0x80 half width (sprite scale)
	WORD m_wX;				// +0x5A = 0 world x
	WORD m_wY;				// +0x5C = 0 world y
	WORD m_wUnk5E;				// +0x5E
	DWORD m_dwUnk60;			// +0x60 = 1
	DWORD m_bBillboard;			// +0x64 = 1 nonzero: rebuild the facing
								//   segment this frame
	DWORD m_bMirror;			// +0x68 = 0 nonzero: draw mirrored
	CMerlinDynamic* m_pNextFree;			// +0x6C pool free-list link
};								// sizeof = 0x70

/////////////////////////////////////////////////////////////////////////////
// CMerlinLocation: a named point of the maze (spawn points, pad centers,
// the "BEACON_nn" waypoints the robot AI steers by). CRuntimeClass
// "CMerlinLocation" at 0x4C5638 (size 0x1C, schema 1, base CMerlinObject);
// vtable at hover.exe:0x4BD3C0. The objects live in
// CMerlinWorld::m_arrLocations and, by name, in g_mapUnk_4A4768.

class CMerlinLocation : public CMerlinObject
{
	DECLARE_SERIAL(CMerlinLocation)

public:
	// Defined here, not in gameobj.cpp: the original's constructor is
	// inline, so the only copy of its body in hover.exe is the one the
	// compiler expanded into the IMPLEMENT_SERIAL Construct(void*) thunk
	// at 0x415070 (notes/perfect-match.md rule 221).
	CMerlinLocation()		// hover.exe:0x415070
	{
		m_wX = 0;
		m_wY = 0;
		m_wAngle = 0;
		m_wHeight = 0;
	}

	// hover.exe:0x41B940 (vtable slot 2) serialize the four words; on
	// load, register the point in g_mapUnk_4A4768
	virtual void Serialize(CArchive& ar);

	// Kept although nothing calls it any more (rule 338: deleting a dead
	// inline from a shared header costs 0.33 byte-weighted here).  The
	// loader builds its spawn points with texture.cpp's LOCPT instead,
	// which homes no temporary; this accessor homes its own `pt`.
	// The level loader spawns players and robots from these points and
	// hands the first two words over as one 8-byte value: hover.exe
	// 0x415D20, 0x415E7D and 0x415FC0 each widen +0x14 and +0x16 into
	// the two halves of a single frame temporary and close with the
	// inline's return `jmp`, then push that temporary as a unit.  So the
	// pair is reached through a POINT-returning inline accessor.
	POINT GetPoint() const
	{
		POINT pt;

		pt.x = (short)m_wX;
		pt.y = (short)m_wY;
		return pt;
	}

// Member variables (hover.exe offsets in comments)
	WORD m_wX;			// +0x14 = 0 x
	WORD m_wY;			// +0x16 = 0 y
	WORD m_wAngle;			// +0x18 = 0 heading (passed to CPlayer::PlacePlayer)
	WORD m_wHeight;			// +0x1A = 0 height (base height of pods)
};							// sizeof = 0x1C

/////////////////////////////////////////////////////////////////////////////
// CMerlinBSP: one node of the maze's BSP tree. The CMerlinLine base is the
// splitting segment, m_wFace indexes the CMerlinStatic face the node
// carries and m_wFront/m_wBack are the two child node indices (-1 = leaf).
// CRuntimeClass "CMerlinBSP" at 0x4C5660 (size 0x68, schema 1, base
// CMerlinLine); vtable at hover.exe:0x4BD438. The nodes live in
// CMerlinWorld::m_arrNodes, node 0 being the root.

class CMerlinBSP : public CMerlinLine
{
	DECLARE_SERIAL(CMerlinBSP)

public:
	// Defined here, not in gameobj.cpp: the original's constructor is
	// inline, so the only copy of its body in hover.exe is the one the
	// compiler expanded into the IMPLEMENT_SERIAL Construct(void*) thunk
	// at 0x41BB80 (notes/perfect-match.md rule 221).
	CMerlinBSP()			// hover.exe:0x41BB80
	{
		m_wFace = 0xFFFF;
		m_wUnk46 = 0xFFFF;
		m_wFront = 0xFFFF;
		m_wBack = 0xFFFF;
		m_dblUnk50 = 0.0;
		m_dblUnk58 = 1.0;
		m_wUnk60 = 0;
	}

	// hover.exe:0x41BC40 (vtable slot 2) serialize the four node words,
	// the +0x60 word and the two doubles
	virtual void Serialize(CArchive& ar);

// Member variables (hover.exe offsets in comments)
	WORD m_wFace;			// +0x44 = -1 index into CMerlinWorld::m_arrStatics
	WORD m_wUnk46;			// +0x46 = -1
	WORD m_wFront;			// +0x48 = -1 child node index (front)
	WORD m_wBack;			// +0x4A = -1 child node index (back)
							// +0x4C..0x4F: alignment padding for the
							// doubles below
	double m_dblUnk50;		// +0x50 = 0.0
	double m_dblUnk58;		// +0x58 = 1.0
	WORD m_wUnk60;			// +0x60 = 0
};							// sizeof = 0x68

/////////////////////////////////////////////////////////////////////////////
// CGameObj

class CPlayer;
class CInviciblePod;
class CSpeedPod;
class CSlowPod;
class CTempWallPod;
class CSmokeBombPod;
class CJumpPod;
class CMerlinTexture;
struct tagREGIONNODE;

class CGameObj : public CObject
{
	DECLARE_DYNAMIC(CGameObj)

public:
	CGameObj();				// hover.exe:0x40D150

	// hover.exe:0x40D220 (vtable slot 1 deleting dtor at 0x42ADB0);
	// releases resources via theApp.m_pBumperDoc
	virtual ~CGameObj();

// Virtual functions. Slots 5-11 are the game virtuals decompiled in
// gameobj.cpp; slot 12 is UpdateRegions (base CGameObj returns 1; CPlayer
// overrides it at hover.exe:0x408820). Slots 13-17 hold the remaining
// CGameObj methods, keeping the renderer helpers' offsets at 0x3c and 0x40.
	virtual int SetMerlin(CMerlinLine* pMerlin);	// slot 5, hover.exe:0x414A50
	virtual int Sub_42AD10(DWORD dwA, DWORD dwB); // slot 6, hover.exe:0x42AD10
	virtual void Think();				// slot 7, hover.exe:0x42AD30
	virtual void OnTouch(CPlayer* pOther, DWORD dwB); // slot 8, hover.exe:0x42AD40
	virtual void UsePod(DWORD dwUnk);	// slot 9, hover.exe:0x42AD50
	virtual int IsNear(POINT pt, long lRadius); // slot 10, hover.exe:0x42AD70
	virtual int Touches(DWORD dwUnk);	// slot 11, hover.exe:0x42AD60
	virtual int UpdateRegions(DWORD dwA);		// slot 12, hover.exe:0x408820
	virtual int IntersectLine(DWORD dwA, DWORD dwB, DWORD dwC, DWORD* pdwOut); // slot 13, hover.exe:0x411570
	virtual int OnProbeHit(CGameObj* pObj);	// slot 14, hover.exe:0x42AD90
	virtual void BeginFrame();				// slot 15, hover.exe:0x4087D0
	virtual void EndFrame();				// slot 16, hover.exe:0x4087F0
	virtual void KillObject();				// slot 17, hover.exe:0x413020

// Non-virtual helpers shared by the game object classes.
	void QueueBeginFrame();	// hover.exe:0x408530 link into the g_pDirtyObjList list
	// hover.exe:0x403880 copy the m_wX/m_wY position words
	// sign-extended into pdwOut[0..1]
	DWORD* GetCellPos(DWORD* pdwOut);
	// hover.exe:0x4038A0 copy m_fxX/m_fxY into pdwOut[0..1]
	DWORD* GetFixedPos(DWORD* pdwOut);
	// hover.exe:0x406C40 copy m_dwPrevX/m_dwPrevY into pdwOut[0..1]
	DWORD* GetPrevPos(DWORD* pdwOut);
	// hover.exe:0x413000 copy m_dwFrameX/m_dwFrameY into pdwOut[0..1]
	DWORD* GetFramePos(DWORD* pdwOut);
	// hover.exe:0x40F880 set the m_wX/m_wY position words and clear
	// their fractions
	void SetCellPos(DWORD dwX, DWORD dwY);
	// hover.exe:0x40A9B0 push the m_pRegionNodes node chain onto the
	// g_pRegionNodeFree free list
	void ReleaseRegionNodes();
	// hover.exe:0x40D6E0 throttle-limited sound trigger via CBumperDoc slots
	void PlayObjSound(DWORD dwA, DWORD dwB, DWORD dwC, DWORD dwD);
	// hover.exe:0x41A580 clear the dynamic state fields (+0x24..+0x80) and
	// stamp m_dwLastSoundTick with GetTickCount (called by CPlayer::Spawn)
	void ResetState();

// Member variables (hover.exe offsets in comments)
	CSegment m_seg;		// +0x04
	DWORD m_dwUnk24;		// +0x24 = 0
	DWORD m_bInUse;		// +0x28
	DWORD m_bSolidHit;		// +0x2C
	struct tagREGIONNODE* m_pRegionNodes;		// +0x30 = 0
	DWORD m_dwLastSoundTick;		// +0x34 = GetTickCount()
	DWORD m_bHideOnMap;		// +0x38 = 0
	DWORD m_bOnMap;		// +0x3C
	DWORD m_dwMapColor;		// +0x40
	// Position, 16.16 fixed point: whole as m_fxX/m_fxY, or by halves
	// (m_wX/m_wY are the integer cell coordinates).
	union					// +0x44 = 0 x
	{
		DWORD m_fxX;
		struct { WORD m_wXFrac, m_wX; };
	};
	union					// +0x48 = 0 y
	{
		DWORD m_fxY;
		struct { WORD m_wYFrac, m_wY; };
	};
	DWORD m_dwFrameX;		// +0x4C = 0
	DWORD m_dwFrameY;		// +0x50 = 0
	DWORD m_dwPrevX;		// +0x54 = 0
	DWORD m_dwPrevY;		// +0x58 = 0
	DWORD m_bKilled;		// +0x5C = 0
	DWORD m_bDead;		// +0x60 = 0
	DWORD m_dwUnk64;		// +0x64 = 0
	DWORD m_bOnStartList;		// +0x68 = 0
	DWORD m_bOnEndList;		// +0x6C = 0
	CGameObj* m_pNextStart;		// +0x70 = 0
	CGameObj* m_pNextEnd;		// +0x74 = 0
	CMerlinLine* m_pMerlin;		// +0x78 = 0
	DWORD m_dwUnk7C;		// +0x7C = 0
	DWORD m_dwUnk80;		// +0x80 = 0
};							// sizeof = 0x84

/////////////////////////////////////////////////////////////////////////////
// PLAYERPROBE: a player's collision probe (CPlayer +0x180), rebuilt every
// frame by CPlayer::UpdateProbe: two points 0x32 units ahead of and behind
// the craft along its heading, and the radius around each of them that
// counts as contact. Handed out by the GetProbe virtual.

typedef struct tagPLAYERPROBE
{
	POINT ptFront;				// +0x00 point ahead of the craft
	POINT ptBack;				// +0x08 point behind the craft
	long lRadius;				// +0x10 contact radius (at least 0x32)
} PLAYERPROBE;					// sizeof = 0x14

/////////////////////////////////////////////////////////////////////////////
// CPlayer

class CPlayer : public CGameObj
{
	DECLARE_DYNAMIC(CPlayer)

public:
	CPlayer();				// hover.exe:0x41AC30

	// hover.exe:0x42C080 (vtable slot 1 deleting dtor)
	virtual ~CPlayer();		// hover.exe:0x421A90 (??_G 0x42C080)

// Overrides of CGameObj virtuals. The override must keep the base name;
// each comment records the address of the original CPlayer implementation.
	virtual void Think();				// hover.exe:0x409110
	virtual void OnTouch(CPlayer* pOther, DWORD dwB); // hover.exe:0x40C220
	virtual int IsNear(POINT pt, long lRadius); // hover.exe:0x42BFC0
	virtual int Touches(DWORD dwUnk);	// hover.exe:0x42BFA0
	virtual int UpdateRegions(DWORD dwA);		// hover.exe:0x408820
	virtual void BeginFrame();				// hover.exe:0x408960
	virtual void EndFrame();				// hover.exe:0x408990
	virtual void KillObject();				// hover.exe:0x42C050
	// New CPlayer virtual, vtable slot 18 (offset 0x48): empty in every
	// player class of the original (hover.exe:0x42BE40 is a bare `ret 4`).
	// Declared before GetProbe so it lands in slot 18 and GetProbe
	// keeps slot 19 (offset 0x4C), as in the original vtables.
	virtual void Sub_42BE40(DWORD dwUnk);	// hover.exe:0x42BE40
	// hover.exe:0x4089E0 returns &m_probe (vtable slot 0x4C)
	virtual PLAYERPROBE* GetProbe();

	// hover.exe:0x414A80 reinitializes the word arrays below
	// (called by CHumanPlayer::CHumanPlayer)
	void ResetMotion();

	// hover.exe:0x414BD0 empties the object lists (deleting the elements);
	// called with 1 by the destructor
	void ClearPods(int bAll);

	// hover.exe:0x413ED0 point the player at a parameter block (H params
	// for the human player, R params for the robots) and reload the three
	// tuning values cached at +0x1D0
	void SetParams(CPlayerParams* pParams);

	// hover.exe:0x408550 recompute the +0x180/+0x188 proximity points and
	// the +0x190 range limit from the current position and angle
	void UpdateProbe(DWORD dwUnk);
	// hover.exe:0x40A9F0 turn and thrust input integration
	void ApplyControls();
	// hover.exe:0x40AB70 move the hover height toward its target
	void UpdateHeight();
	// hover.exe:0x40ACE0 the hold pad's countdown (m_dwHoldTimer)
	void UpdateHoldPad();
	// hover.exe:0x40ADE0 the sled pad's push (m_dwSledTimer, the angle
	// at +0x160)
	void UpdateSled();
	// hover.exe:0x40B0B0 velocity integration and position update
	void MovePlayer();
	// hover.exe:0x42B8E0 collision response against another player
	int CollidePlayer(DWORD dwA, DWORD dwB);
	// hover.exe:0x42BE50 proximity test against pOther's points (vtable
	// slot 0x4C in the original returns &pOther->m_probe)
	int TouchesPlayer(CPlayer* pOther, DWORD* pdwA, DWORD* pdwB);
	// hover.exe:0x41B2D0 set the start position/angle (called by
	// CBumperDoc::OnOpenDocument with the start object's +0x14..+0x1A words)
	void Spawn(POINT pt, DWORD dwC, DWORD dwD);
	// hover.exe:0x41B0E0 set the start position/velocity state (called by
	// CPlayer::Spawn and by the level loader on robot objects)
	void PlacePlayer(POINT pt, DWORD dwC, DWORD dwD);
	// hover.exe:0x41A6B0 bounce the player off pWall; returns nonzero
	// when the wall was actually hit
	int BounceOffWall(CGameObj* pWall, DWORD dwB);
	// hover.exe:0x41AB10 reflect the velocity about dwAngle and feed the
	// tangential part back into the heading and the turn rate
	void Reflect(DWORD dwFlip, DWORD dwAngle);
	// hover.exe:0x421B50 the endpoint case of the wall bounce: bounce off
	// the segment endpoint selected by dwFirst, returns nonzero when the
	// player was actually moving into it
	int BounceOffEndpoint(DWORD dwFlip, DWORD dwX, DWORD dwY, CGameObj* pWall,
		DWORD dwFirst, int nDist);

// Member variables (hover.exe offsets in comments)
	CPlayerParams* m_pParams;	// +0x84 tuning block (theApp.m_pHumanParams
							//   or m_pRobotParams, set by SetParams)
	DWORD m_dwUnk0088;		// +0x88
	DWORD m_dwUnk008C;		// +0x8C
	// The 16.16 fixed-point state below is read and written whole
	// (m_fx...) or, where the original stores 16 bits at a time, by halves
	// (m_w...Frac is the low word, m_w... the integer word).
	union					// +0x90 = 0 (never read)
	{
		DWORD m_fxUnk0090;
		struct { WORD m_wUnk0090, m_wUnk0092; };
	};
	union					// +0x94 = 0 (never read)
	{
		DWORD m_fxUnk0094;
		struct { WORD m_wUnk0094, m_wUnk0096; };
	};
	union					// +0x98 = 0 height change applied this frame
	{
		DWORD m_fxHeightStep;
		struct { WORD m_wHeightStepFrac, m_wHeightStep; };
	};
	union					// +0x9C = 0 height change per frame toward the target
	{
		DWORD m_fxHeightRate;
		struct { WORD m_wHeightRateFrac, m_wHeightRate; };
	};
	union					// +0xA0 = 0 height the craft settles to (0x40;
							//   more on a jump or on top of a wall)
	{
		DWORD m_fxTargetHeight;
		struct { WORD m_wTargetHeightFrac, m_wTargetHeight; };
	};
	union					// +0xA4 = 0 current height (moves toward the target)
	{
		DWORD m_fxHeight;
		struct { WORD m_wHeightFrac, m_wHeight; };
	};
	DWORD m_bAirborne;		// +0xA8 jumping (set by a jump pod, cleared on
							//   landing)
	DWORD m_dwSpawnHeight1;	// +0xAC spawn height (written, never read)
	DWORD m_dwSpawnHeight2;	// +0xB0
	// All four are CObList(10): CPlayer::CPlayer calls 0x4491E4 at
	// 0x41ACB8/0x41ACCC/0x41ACE0/0x41ACF4, and 0x4491E4 stores the
	// CObList vtable 0x4C2118 (rule 251 - libmatch misnames it
	// CPtrList::CPtrList).
	CObList m_listFlags;	// +0xB4
	CObList m_listTempWallPods;	// +0xD0
	CObList m_listSmokeBombPods;	// +0xEC
	CObList m_listJumpPods;	// +0x108
	CInviciblePod* m_pInvinciblePod;		// +0x124 (zeroed by CHumanPlayer)
	CSpeedPod* m_pSpeedPod;		// +0x128 (zeroed by CHumanPlayer)
	CSlowPod* m_pSlowPod;		// +0x12C (zeroed by CHumanPlayer)
	CTempWallPod* m_pTempWallPod;		// +0x130 (zeroed by CHumanPlayer)
	CSmokeBombPod* m_pSmokeBombPod;		// +0x134 (zeroed by CHumanPlayer)
	CJumpPod* m_pJumpPod;		// +0x138 (zeroed by CHumanPlayer)
	DWORD m_bOnTop;		// +0x13C
	DWORD m_bInvincible;		// +0x140 (zeroed by CHumanPlayer)
	DWORD m_bGodMode;		// +0x144 (zeroed by CHumanPlayer)
	DWORD m_dwHoldTimer;		// +0x148
	DWORD m_dwSledTimer;		// +0x14C
	DWORD m_dwUnk0150;		// +0x150
	DWORD m_bOnHoldPad;		// +0x154
	DWORD m_bOnFlagRemover;		// +0x158
	DWORD m_dwFlagRemoverState;		// +0x15C
	DWORD m_dwSledAngle;	// +0x160 push direction of the sled pad (UpdateSled)
	POINT m_aptMapTri[3];	// +0x164 the craft's triangle on the map
	DWORD m_dwUnk017C;		// +0x17C visible flag
	PLAYERPROBE m_probe;	// +0x180 collision probe (set by UpdateProbe)
	union					// +0x194 = 0 heading change per frame
	{
		DWORD m_fxTurnRate;
		struct { WORD m_wTurnRateFrac, m_wTurnRate; };
	};
	union					// +0x198 = 0 one-frame turn (collisions), cleared each frame
	{
		DWORD m_fxTurnImpulse;
		struct { WORD m_wTurnImpulseFrac, m_wTurnImpulse; };
	};
	union					// +0x19C = 0 heading; m_wHeading is the 9-bit angle
	{
		DWORD m_fxHeading;
		struct { WORD m_wHeadingFrac, m_wHeading; };
	};
	DWORD m_dwFrameHeading;		// +0x1A0 (zeroed by ResetMotion, not by the ctor)
	DWORD m_dwAltitude;		// +0x1A4
	DWORD m_dwFrameAltitude;		// +0x1A8 (zeroed by ResetMotion, not by the ctor)
	union					// +0x1AC = 0 velocity
	{
		DWORD m_fxVelX;
		struct { WORD m_wVelXFrac, m_wVelX; };
	};
	union					// +0x1B0 = 0
	{
		DWORD m_fxVelY;
		struct { WORD m_wVelYFrac, m_wVelY; };
	};
	union					// +0x1B4 = 0 acceleration (thrust along the heading)
	{
		DWORD m_fxAccelX;
		struct { WORD m_wAccelXFrac, m_wAccelX; };
	};
	union					// +0x1B8 = 0
	{
		DWORD m_fxAccelY;
		struct { WORD m_wAccelYFrac, m_wAccelY; };
	};
	union					// +0x1BC = 0 (never read)
	{
		DWORD m_fxUnk01BC;
		struct { WORD m_wUnk01BC, m_wUnk01BE; };
	};
	union					// +0x1C0 = 0 (never read)
	{
		DWORD m_fxUnk01C0;
		struct { WORD m_wUnk01C0, m_wUnk01C2; };
	};
	DWORD m_dwUnk01C4;		// +0x1C4 (zeroed by ResetMotion)
	DWORD m_dwUnk01C8;		// +0x1C8
	DWORD m_bBounced;		// +0x1CC (zeroed by OnTouch/CollidePlayer)
	union					// +0x1D0 = 0 acceleration per unit of thrust
							//   (m_pParams->m_adw[0])
	{
		DWORD m_fxThrustAccel;
		struct { WORD m_wThrustAccelFrac, m_wThrustAccel; };
	};
	union					// +0x1D4 = 0 current speed limit (speed/slow pods
							//   change it)
	{
		DWORD m_fxMaxSpeed;
		struct { WORD m_wMaxSpeedFrac, m_wMaxSpeed; };
	};
	union					// +0x1D8 = 0 normal speed limit (m_pParams->m_adw[2])
	{
		DWORD m_fxBaseMaxSpeed;
		struct { WORD m_wBaseMaxSpeedFrac, m_wBaseMaxSpeed; };
	};
	DWORD m_bSteerAssist;		// +0x1DC
	DWORD m_bFriction;		// +0x1E0
	CMerlinTexture* m_pHeadingFrame;		// +0x1E4 sprite frame for the current heading
	CMerlinTexture* m_pFrame;		// +0x1E8 sprite frame published to the renderer
	DWORD m_bMirror;		// +0x1EC nonzero: draw the frame mirrored
	union					// +0x1F0 = 0 steering input (left/right keys, or the
							//   robot's facing error)
	{
		DWORD m_fxTurnInput;
		struct { WORD m_wTurnInputFrac, m_wTurnInput; };
	};
	union					// +0x1F4 = 0 thrust input (up/down keys, or the
							//   robot's speed error)
	{
		DWORD m_fxThrustInput;
		struct { WORD m_wThrustInputFrac, m_wThrustInput; };
	};
	DWORD m_bHuman;		// +0x1F8 = 0 (= 1 in CHumanPlayer)
};							// sizeof = 0x1FC

/////////////////////////////////////////////////////////////////////////////
// CHumanPlayer

class CHumanPlayer : public CPlayer
{
	DECLARE_DYNAMIC(CHumanPlayer)

public:
	CHumanPlayer();			// hover.exe:0x421FA0

	// hover.exe:0x422050 (empty body; just the base-class dtor call)
	virtual ~CHumanPlayer();

// Overrides of CGameObj virtuals (see CPlayer note above).
	virtual void Think();				// hover.exe:0x409180
	virtual void OnTouch(CPlayer* pOther, DWORD dwB); // hover.exe:0x40C400
	virtual void BeginFrame();				// hover.exe:0x4089D0
	virtual void EndFrame();				// hover.exe:0x4089F0
	virtual void KillObject();				// hover.exe:0x42C240
};							// sizeof = 0x1FC

/////////////////////////////////////////////////////////////////////////////
// CMerlinWorld: the level world manager pointed to by
// CBumperDoc::m_pWorld (CRuntimeClass at hover.exe:0x4C51E0, size 0x64,
// schema 1, base CObject; vtable at hover.exe:0x4BBEC8). Its vtable slot 2
// Serialize override (hover.exe:0x418DC0) loads the maze: a four-WORD
// header followed by the four CObArrays of maze objects embedded at
// +0x14/+0x28/+0x3C/+0x50 -- the static faces, the sprites, the named
// points and the BSP nodes. The renderer walks the same three arrays
// through this pointer (CSpanRasterizer::RenderWorld/WalkBSP).

class CMerlinWorld : public CObject
{
	DECLARE_SERIAL(CMerlinWorld)

public:
	// hover.exe:0x418AE0; also resets the level state: deletes every
	// loaded texture and empties g_obArrUnk_4A1D08
	CMerlinWorld();
	// hover.exe:0x418C00 (vtable slot 1 deleting dtor at 0x419190);
	// deletes the tracked objects, the loaded textures, and the level
	// palette
	virtual ~CMerlinWorld();

	// hover.exe:0x418DC0 (vtable slot 2) load the maze from the archive
	virtual void Serialize(CArchive& ar);

	// hover.exe:0x418EA0 total pixel-data bytes of all texture frames at
	// or above the g_dwMipBias mip level
	DWORD GetTextureBytes();
	// hover.exe:0x418F00 realize an all-black PC_NOCOLLAPSE palette on the
	// display to force a system palette rebuild
	void FlushSystemPalette();
	// hover.exe:0x418FA0 rebuild g_palGame from the system
	// palette: static entries keep their colors, the dynamic middle
	// entries take the game palette marked PC_NOCOLLAPSE and are made
	// unique
	void BuildIdentityPalette();

	// hover.exe:0x413DB0 remove dwUnk from the arrays and delete it
	int RemoveObject(CMerlinLine* pObj);
	// hover.exe:0x4128A0 add dwUnk to m_arrSprites unless it is already
	// there; returns nonzero when it was added
	int AddSprite(CMerlinDynamic* pSprite);
	// hover.exe:0x414300 remove dwUnk from m_arrSprites (if present) and
	// delete it; returns nonzero when it was on the array
	int RemoveSprite(CMerlinDynamic* pSprite);

	// Typed accessors for the renderer's walks over the three arrays.
	CMerlinStatic* GetStatic(int i)
		{ return (CMerlinStatic*)m_arrStatics[i]; }
	CMerlinDynamic* GetDynamic(int i)
		{ return (CMerlinDynamic*)m_arrSprites[i]; }
	CMerlinBSP* GetNode(int i)
		{ return (CMerlinBSP*)m_arrNodes[i]; }

// Member variables (hover.exe offsets in comments)
	// The maze header loaded by Serialize stores four WORDs at +0x04,
	// +0x08, +0x0C and +0x10; the intervening WORDs are padding in the
	// original layout.
	WORD m_wUnk04;			// +0x04
	WORD m_wPad06;			// +0x06
	WORD m_wUnk08;			// +0x08
	WORD m_wPad0A;			// +0x0A
	WORD m_wUnk0C;			// +0x0C
	WORD m_wPad0E;			// +0x0E
	WORD m_wUnk10;			// +0x10
	WORD m_wPad12;			// +0x12
	CObArray m_arrStatics;	// +0x14 CMerlinStatic faces (m_pData at +0x18)
	CObArray m_arrSprites;	// +0x28 CMerlinDynamic sprites (m_pData at
							//   +0x2C, m_nSize at +0x30)
	CObArray m_arrLocations;	// +0x3C CMerlinLocation points
	CObArray m_arrNodes;	// +0x50 CMerlinBSP nodes (m_pData at +0x54,
							//   node 0 is the root)
};							// sizeof = 0x64 in hover.exe

/////////////////////////////////////////////////////////////////////////////
// CSoundSlot: sound-slot objects referenced from CBumperDoc::m_adwSounds
// (indices 0x1A-0x1D are the player sound slots). CObject-derived (plain
// CObject vtable at hover.exe:0x4BD8E0; deleting dtor 0x423120); the
// constructor registers the slot in g_arrUnk_4A47F0 and preloads the
// wave file.

class CSoundSlot : public CObject
{
public:
	CSoundSlot(const char* pszName, DWORD dwUnk);	// hover.exe:0x415380
	// hover.exe:0x422CB0 (virtual through CObject; the deleting dtor the
	// compiler puts in vtable slot 1 is 0x423120)
	~CSoundSlot();

	// hover.exe:0x417860 load the wave file (raw buffer when WaveMix is
	// unavailable, MIXWAVE handle otherwise); returns 0 on failure
	int LoadWave(const char* pszName);
	// hover.exe:0x40B800 play the slot's sound (WaveMix when the engine is
	// up, otherwise queued to the sound worker thread)
	int PlayWave(DWORD dwA, DWORD dwB, DWORD dwC);
	// hover.exe:0x4118A0 stop the sound (WaveMixFlushChannel / worker queue)
	int StopWave(DWORD dwA, DWORD dwB);

// Member variables (hover.exe offsets in comments)
	void* m_pWaveData;			// +0x04 sound data (played via SND_MEMORY by the
							// sound worker thread Hover_SoundThreadProc)
	MIXWAVE* m_pMixWave;			// +0x08 open MIXWAVE handle (WaveMix path)
	CString m_strWaveName;		// +0x0C wave file name (".wav" is appended)
	DWORD m_bOpenOnPlay;		// +0x18 nonzero: open/play through WaveMix
};

/////////////////////////////////////////////////////////////////////////////
// CSub40B510 is the skeletal name this reconstruction previously used for
// the class at CBumperDoc::m_pCollider; the original's CRuntimeClass names
// it CCollider (0x4C55A0) — see texload.h. ReinsertObject is implemented in
// gameobj.cpp (below) as CCollider::ReinsertObject.

/////////////////////////////////////////////////////////////////////////////
// Globals defined in gameobj.cpp

extern CGameObj* g_pDirtyObjList;	// hover.exe:0x4C52D8 CGameObj instance list head
extern CGameObj* g_pDrawObjList;	// hover.exe:0x4C52DC CGameObj secondary list head
extern DWORD g_dwUnk_4C54A0;	// hover.exe:0x4C54A0 = -1 (last sound state)
extern CMapPtrToPtr g_mapOpenWaveToSlot;	// hover.exe:0x4A4820 MIXWAVE handle ->
									// CSoundSlot slot map (PlayWave)
extern CObArray g_arrUnk_4A47F0;	// hover.exe:0x4A47F0 sound slot
									// registry (CSoundSlot::CSoundSlot)
extern DWORD g_dwUnk_4C5700;		// hover.exe:0x4C5700 sound slot count

#endif // __GAMEOBJ_H__
