// gameobj.h : interface of the CGameObj / CPlayer / CHumanPlayer classes
//
// Skeleton reconstructed from hover.exe:
//   CRuntimeClass "CGameObj" at 0x4C52C0 (size 0x84, DYNAMIC, base CObject)
//   CRuntimeClass "CPlayer" at 0x4C53F8 (size 0x1FC, DYNAMIC, base CGameObj)
//   CRuntimeClass "CHumanPlayer" at 0x4C5488 (size 0x1FC, DYNAMIC, base CPlayer)
//   vtables at 0x4BC380 / 0x4BC910 / 0x4BCBE0 (slots 0-4 are the CObject
//   set, slots 5-11 are the game virtuals declared below; the original
//   vtables run further — CGameObj to slot 0x11, CPlayer/CHumanPlayer add
//   slots 0x12-0x13, where slot 0x13 (0x4089E0) returns &m_dwUnk0180 —
//   reached through explicit vtable-slot calls where needed)
//
// Note on member offsets: the comments give each member's offset within the
// class in hover.exe.

#ifndef __GAMEOBJ_H__
#define __GAMEOBJ_H__

#include "texture.h"	// CMerlinObject (the base of CMerlinLine) and
						// CMerlinTexture

/////////////////////////////////////////////////////////////////////////////
// CSub40D320: small value struct embedded in CGameObj at offset 0x04 (and
// used elsewhere). Stores two endpoints (+0x00..+0x0C) plus values
// recomputed from them by Sub_42C820 (+0x10..+0x1C).
// ctor hover.exe:0x40D320, sizeof = 0x20.

class CSub40D320
{
public:
	CSub40D320();			// hover.exe:0x40D320

	// hover.exe:0x42C820 recompute m_dwUnk10..m_dwUnk1C from endpoints
	void Sub_42C820();

	// hover.exe:0x422BF0 set the endpoints from two point-pair pointers
	// and recompute the derived values
	void Sub_422BF0(DWORD* pdwPt1, DWORD* pdwPt2);

	// hover.exe:0x40E8E0 set the endpoints from the +0x14..+0x20 dwords
	// of pObj (a CMerlinLine in the collider paths) and recompute;
	// returns this
	CSub40D320* Sub_40E8E0(CObject* pObj);

	// hover.exe:0x40D2F0 side-of-line test: nonzero when the point
	// plPt[0..1] is on the positive side of the segment
	int Sub_40D2F0(const long* plPt);

	// hover.exe:0x40C9B0 distance of the point pdwPt[0..1] from the
	// segment. *pdwA is nonzero when the foot of the perpendicular falls
	// inside the segment, *pdwB is the side the point is on and *pdwC is
	// nonzero when the point lies beyond the first endpoint.
	int Sub_40C9B0(const DWORD* pdwPt, DWORD* pdwA, DWORD* pdwB,
		DWORD* pdwC);

	DWORD m_dwUnk00;		// +0x00 = 0
	DWORD m_dwUnk04;		// +0x04 = 0
	DWORD m_dwUnk08;		// +0x08 = 0
	DWORD m_dwUnk0C;		// +0x0C = 0
	DWORD m_dwUnk10;		// +0x10 = m_dwUnk08 - m_dwUnk00 (computed)
	DWORD m_dwUnk14;		// +0x14 = m_dwUnk0C - m_dwUnk04 (computed)
	DWORD m_dwUnk18;		// +0x18 = dx*dx + dy*dy (computed)
	DWORD m_dwUnk1C;		// +0x1C = angle (computed, via hover.exe:0x4386D0)
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
	void Sub_4010C0();

	// hover.exe:0x401260 intersect this segment with pLine, writing the
	// crossing point into pdwOut[0..1]; returns 0 when the segments do
	// not cross, 1 when pLine crosses this segment from the front and
	// 0xFFFF when it crosses it from the back. Also reached through the
	// maze faces (CMerlinStatic) and sprites (CMerlinDynamic), which
	// derive from this class.
	WORD Sub_401260(CMerlinLine* pLine, DWORD* pdwOut);

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
// The objects live in CMerlinWorld::m_arrUnk14 and, by name, in
// g_mapUnk_4A4578; the renderer reaches them through RENDEROBJ::m_pUnk10
// and CWall::m_dwUnk78.

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
	CString m_strUnk44;			// +0x44 texture name of m_pUnk8C
	CString m_strUnk50;			// +0x50 texture name of m_pUnk90
	CString m_strUnk5C;			// +0x5C front texture name ("..DECAL_"
								//   on the pad faces)
	CString m_strUnk68;			// +0x68 back texture name
	CString m_strUnk74;			// +0x74 texture name of m_pUnk9C
	CString m_strUnk80;			// +0x80 texture name of m_pUnkA0
	CMerlinTexture* m_pUnk8C;	// +0x8C = 0 resolved texture
	CMerlinTexture* m_pUnk90;	// +0x90 = 0 resolved texture
	CMerlinTexture* m_pUnk94;	// +0x94 = 0 front face texture
	CMerlinTexture* m_pUnk98;	// +0x98 = 0 back face texture
	CMerlinTexture* m_pUnk9C;	// +0x9C = 0 resolved texture
	CMerlinTexture* m_pUnkA0;	// +0xA0 = 0 resolved texture
	WORD m_wUnkA4;				// +0xA4 = 0 low edge height (signed)
	WORD m_wUnkA6;				// +0xA6 = 0x80 high edge height (signed)
	WORD m_wUnkA8;				// +0xA8 = 0
	WORD m_wUnkAA;				// +0xAA = 0
	WORD m_wUnkAC;				// +0xAC = 0
	WORD m_wUnkAE;				// +0xAE (never touched)
	DWORD m_dwUnkB0;			// +0xB0 = 0 front face has pixel data
	DWORD m_dwUnkB4;			// +0xB4 = 0 back face has pixel data
	WORD m_wUnkB8;				// +0xB8 = 0 front texture u origin
	WORD m_wUnkBA;				// +0xBA = 0 back texture u origin
	DWORD m_dwUnkBC;			// +0xBC = 1 nonzero enables the pad
								//   registration in the level loader
	DWORD m_dwUnkC0;			// +0xC0 = 0
};								// sizeof = 0xC4

/////////////////////////////////////////////////////////////////////////////
// CMerlinDynamic: the billboard sprite object -- the CMerlinLine base is
// the camera-facing segment, rebuilt each frame by CSub406090::Sub_406090
// from the world position (m_wUnk5A/m_wUnk5C) and the m_wUnk58 scale so
// the sprite always faces the viewer. CRuntimeClass "CMerlinDynamic" at
// 0x4C5608 (size 0x70, schema 1, base CMerlinLine); vtable at
// hover.exe:0x4BD348. The objects live in CMerlinWorld::m_arrUnk28 and,
// by name, in g_mapUnk_4A4728; the pods, players and the temporary wall
// create theirs out of a pool of g_dwUnk_4C5624 objects whose blocks are
// kept in g_listUnk_4A4748 and whose free list is threaded through
// m_dwUnk6C.

class CMerlinDynamic : public CMerlinLine
{
	DECLARE_SERIAL(CMerlinDynamic)

public:
	CMerlinDynamic();			// hover.exe:0x4132A0

	// hover.exe:0x42CC40 (vtable slot 2) serialize the texture name, the
	// height/scale/position words and the m_dwUnk60 flag byte; on load,
	// register the sprite in g_mapUnk_4A4728 and resolve the texture name
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
		((CMerlinDynamic*)p)->m_dwUnk6C = g_dwUnk_4C5620;
		g_dwUnk_4C5620 = (DWORD)p;
	}

// Member variables (hover.exe offsets in comments)
	CString m_strUnk44;			// +0x44 texture name
	CMerlinTexture* m_pUnk50;	// +0x50 = 0 current animation frame
								//   (advanced by Sub_405CE0 through
								//   CMerlinTexture::m_pUnk38)
	WORD m_wUnk54;				// +0x54 = 0 base height
	WORD m_wUnk56;				// +0x56 = 0x80 top height
	WORD m_wUnk58;				// +0x58 = 0x80 half width (sprite scale)
	WORD m_wUnk5A;				// +0x5A = 0 world x
	WORD m_wUnk5C;				// +0x5C = 0 world y
	WORD m_wUnk5E;				// +0x5E
	DWORD m_dwUnk60;			// +0x60 = 1
	DWORD m_dwUnk64;			// +0x64 = 1 nonzero: rebuild the facing
								//   segment this frame
	DWORD m_dwUnk68;			// +0x68 = 0 nonzero: draw mirrored
	DWORD m_dwUnk6C;			// +0x6C pool free-list link
};								// sizeof = 0x70

/////////////////////////////////////////////////////////////////////////////
// CMerlinLocation: a named point of the maze (spawn points, pad centers,
// the "BEACON_nn" waypoints the robot AI steers by). CRuntimeClass
// "CMerlinLocation" at 0x4C5638 (size 0x1C, schema 1, base CMerlinObject);
// vtable at hover.exe:0x4BD3C0. The objects live in
// CMerlinWorld::m_arrUnk3C and, by name, in g_mapUnk_4A4768.

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
		m_wUnk14 = 0;
		m_wUnk16 = 0;
		m_wUnk18 = 0;
		m_wUnk1A = 0;
	}

	// hover.exe:0x41B940 (vtable slot 2) serialize the four words; on
	// load, register the point in g_mapUnk_4A4768
	virtual void Serialize(CArchive& ar);

	// The level loader spawns players and robots from these points and
	// hands the first two words over as one 8-byte value: hover.exe
	// 0x415D20, 0x415E7D and 0x415FC0 each widen +0x14 and +0x16 into
	// the two halves of a single frame temporary and close with the
	// inline's return `jmp`, then push that temporary as a unit.  So the
	// pair is reached through a POINT-returning inline accessor.
	POINT GetPoint() const
	{
		POINT pt;

		pt.x = (short)m_wUnk14;
		pt.y = (short)m_wUnk16;
		return pt;
	}

// Member variables (hover.exe offsets in comments)
	WORD m_wUnk14;			// +0x14 = 0 x
	WORD m_wUnk16;			// +0x16 = 0 y
	WORD m_wUnk18;			// +0x18 = 0 z / height
	WORD m_wUnk1A;			// +0x1A = 0 angle
};							// sizeof = 0x1C

/////////////////////////////////////////////////////////////////////////////
// CMerlinBSP: one node of the maze's BSP tree. The CMerlinLine base is the
// splitting segment, m_wUnk44 indexes the CMerlinStatic face the node
// carries and m_wUnk48/m_wUnk4A are the two child node indices (-1 = leaf).
// CRuntimeClass "CMerlinBSP" at 0x4C5660 (size 0x68, schema 1, base
// CMerlinLine); vtable at hover.exe:0x4BD438. The nodes live in
// CMerlinWorld::m_arrUnk50, node 0 being the root.

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
		m_wUnk44 = 0xFFFF;
		m_wUnk46 = 0xFFFF;
		m_wUnk48 = 0xFFFF;
		m_wUnk4A = 0xFFFF;
		m_dblUnk50 = 0.0;
		m_dblUnk58 = 1.0;
		m_wUnk60 = 0;
	}

	// hover.exe:0x41BC40 (vtable slot 2) serialize the four node words,
	// the +0x60 word and the two doubles
	virtual void Serialize(CArchive& ar);

// Member variables (hover.exe offsets in comments)
	WORD m_wUnk44;			// +0x44 = -1 index into CMerlinWorld::m_arrUnk14
	WORD m_wUnk46;			// +0x46 = -1
	WORD m_wUnk48;			// +0x48 = -1 child node index (front)
	WORD m_wUnk4A;			// +0x4A = -1 child node index (back)
							// +0x4C..0x4F: alignment padding for the
							// doubles below
	double m_dblUnk50;		// +0x50 = 0.0
	double m_dblUnk58;		// +0x58 = 1.0
	WORD m_wUnk60;			// +0x60 = 0
};							// sizeof = 0x68

/////////////////////////////////////////////////////////////////////////////
// CGameObj

class CPlayer;

class CGameObj : public CObject
{
	DECLARE_DYNAMIC(CGameObj)

public:
	CGameObj();				// hover.exe:0x40D150

	// hover.exe:0x40D220 (vtable slot 1 deleting dtor at 0x42ADB0);
	// releases resources via theApp.m_pBumperDoc
	virtual ~CGameObj();

// Virtual functions. Slots 5-11 are the game virtuals decompiled in
// gameobj.cpp; slot 12 is Sub_408820 (base CGameObj returns 1; CPlayer
// overrides it at hover.exe:0x408820). Slots 13-17 hold the remaining
// CGameObj methods, keeping the renderer helpers' offsets at 0x3c and 0x40.
	virtual int Sub_414A50(DWORD dwUnk);	// slot 5, hover.exe:0x414A50
	virtual int Sub_42AD10(DWORD dwA, DWORD dwB); // slot 6, hover.exe:0x42AD10
	virtual void Sub_42AD30();				// slot 7, hover.exe:0x42AD30
	virtual void Sub_42AD40(CPlayer* pOther, DWORD dwB); // slot 8, hover.exe:0x42AD40
	virtual void Sub_42AD50(DWORD dwUnk);	// slot 9, hover.exe:0x42AD50
	virtual int Sub_42AD70(DWORD dwA, DWORD dwB, DWORD dwC); // slot 10, hover.exe:0x42AD70
	virtual int Sub_42AD60(DWORD dwUnk);	// slot 11, hover.exe:0x42AD60
	virtual int Sub_408820(DWORD dwA);		// slot 12, hover.exe:0x408820
	virtual int Sub_411570(DWORD dwA, DWORD dwB, DWORD dwC, DWORD* pdwOut); // slot 13, hover.exe:0x411570
	virtual int Sub_42AD90(CGameObj* pObj);	// slot 14, hover.exe:0x42AD90
	virtual void Sub_4087D0();				// slot 15, hover.exe:0x4087D0
	virtual void Sub_4087F0();				// slot 16, hover.exe:0x4087F0
	virtual void Sub_413020();				// slot 17, hover.exe:0x413020

// Non-virtual helpers shared by the game object classes.
	void Sub_408530();	// hover.exe:0x408530 link into the g_dwUnk_4C52D8 list
	// hover.exe:0x403880 copy the m_wUnk46/m_wUnk4A position words
	// sign-extended into pdwOut[0..1]
	DWORD* Sub_403880(DWORD* pdwOut);
	// hover.exe:0x4038A0 copy the m_wUnk44/m_wUnk48 words into pdwOut[0..1]
	DWORD* Sub_4038A0(DWORD* pdwOut);
	// hover.exe:0x406C40 copy m_dwUnk54/m_dwUnk58 into pdwOut[0..1]
	DWORD* Sub_406C40(DWORD* pdwOut);
	// hover.exe:0x413000 copy m_dwUnk4C/m_dwUnk50 into pdwOut[0..1]
	DWORD* Sub_413000(DWORD* pdwOut);
	// hover.exe:0x40F880 set the m_wUnk46/m_wUnk4A position words and
	// clear m_wUnk44/m_wUnk48
	void Sub_40F880(DWORD dwX, DWORD dwY);
	// hover.exe:0x40A9B0 push the m_dwUnk30 node chain onto the
	// g_dwUnk_4C55D0 free list
	void Sub_40A9B0();
	// hover.exe:0x40D6E0 throttle-limited sound trigger via CBumperDoc slots
	void Sub_40D6E0(DWORD dwA, DWORD dwB, DWORD dwC, DWORD dwD);
	// hover.exe:0x41A580 clear the dynamic state fields (+0x24..+0x80) and
	// stamp m_dwUnk34 with GetTickCount (called by CPlayer::Sub_41B2D0)
	void Sub_41A580();

// Member variables (hover.exe offsets in comments)
	CSub40D320 m_obj04;		// +0x04
	DWORD m_dwUnk24;		// +0x24 = 0
	DWORD m_dwUnk28;		// +0x28
	DWORD m_dwUnk2C;		// +0x2C
	DWORD m_dwUnk30;		// +0x30 = 0
	DWORD m_dwUnk34;		// +0x34 = GetTickCount()
	DWORD m_dwUnk38;		// +0x38 = 0
	DWORD m_dwUnk3C;		// +0x3C
	DWORD m_dwUnk40;		// +0x40
	WORD m_wUnk44;			// +0x44 = 0
	WORD m_wUnk46;			// +0x46 = 0
	WORD m_wUnk48;			// +0x48 = 0
	WORD m_wUnk4A;			// +0x4A = 0
	DWORD m_dwUnk4C;		// +0x4C = 0
	DWORD m_dwUnk50;		// +0x50 = 0
	DWORD m_dwUnk54;		// +0x54 = 0
	DWORD m_dwUnk58;		// +0x58 = 0
	DWORD m_dwUnk5C;		// +0x5C = 0
	DWORD m_dwUnk60;		// +0x60 = 0
	DWORD m_dwUnk64;		// +0x64 = 0
	DWORD m_dwUnk68;		// +0x68 = 0
	DWORD m_dwUnk6C;		// +0x6C = 0
	DWORD m_dwUnk70;		// +0x70 = 0
	DWORD m_dwUnk74;		// +0x74 = 0
	DWORD m_dwUnk78;		// +0x78 = 0
	DWORD m_dwUnk7C;		// +0x7C = 0
	DWORD m_dwUnk80;		// +0x80 = 0
};							// sizeof = 0x84

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
	virtual void Sub_42AD30();				// hover.exe:0x409110
	virtual void Sub_42AD40(CPlayer* pOther, DWORD dwB); // hover.exe:0x40C220
	virtual int Sub_42AD70(DWORD dwA, DWORD dwB, DWORD dwC); // hover.exe:0x42BFC0
	virtual int Sub_42AD60(DWORD dwUnk);	// hover.exe:0x42BFA0
	virtual int Sub_408820(DWORD dwA);		// hover.exe:0x408820
	virtual void Sub_4087D0();				// hover.exe:0x408960
	virtual void Sub_4087F0();				// hover.exe:0x408990
	virtual void Sub_413020();				// hover.exe:0x42C050
	// New CPlayer virtual, vtable slot 18 (offset 0x48): empty in every
	// player class of the original (hover.exe:0x42BE40 is a bare `ret 4`).
	// Declared before Sub_4089E0 so it lands in slot 18 and Sub_4089E0
	// keeps slot 19 (offset 0x4C), as in the original vtables.
	virtual void Sub_42BE40(DWORD dwUnk);	// hover.exe:0x42BE40
	// hover.exe:0x4089E0 returns &m_dwUnk0180 (vtable slot 0x4C)
	virtual DWORD* Sub_4089E0();

	// hover.exe:0x414A80 reinitializes the word arrays below
	// (called by CHumanPlayer::CHumanPlayer)
	void Sub_414A80();

	// hover.exe:0x414BD0 empties the object lists (deleting the elements);
	// called with 1 by the destructor
	void Sub_414BD0(int bAll);

	// hover.exe:0x413ED0 point the player at a parameter block (H params
	// for the human player, R params for the robots) and reload the three
	// tuning values cached at +0x1D0
	void Sub_413ED0(void* pParams);

	// hover.exe:0x408550 recompute the +0x180/+0x188 proximity points and
	// the +0x190 range limit from the current position and angle
	void Sub_408550(DWORD dwUnk);
	// hover.exe:0x40A9F0 throttle/turn input integration
	void Sub_40A9F0();
	// hover.exe:0x40AB70 angular velocity integration
	void Sub_40AB70();
	// hover.exe:0x40ACE0 bounce/brush state countdown
	void Sub_40ACE0();
	// hover.exe:0x40ADE0 wall-bounce handling
	void Sub_40ADE0();
	// hover.exe:0x40B0B0 velocity integration and position update
	void Sub_40B0B0();
	// hover.exe:0x42B8E0 collision response against another player
	int Sub_42B8E0(DWORD dwA, DWORD dwB);
	// hover.exe:0x42BE50 proximity test against pOther's points (vtable
	// slot 0x4C in the original returns &pOther->m_dwUnk0180)
	int Sub_42BE50(CPlayer* pOther, DWORD* pdwA, DWORD* pdwB);
	// hover.exe:0x41B2D0 set the start position/angle (called by
	// CBumperDoc::Sub_415450 with the start object's +0x14..+0x1A words)
	void Sub_41B2D0(POINT pt, DWORD dwC, DWORD dwD);
	// hover.exe:0x41B0E0 set the start position/velocity state (called by
	// CPlayer::Sub_41B2D0 and by the level loader on robot objects)
	void Sub_41B0E0(POINT pt, DWORD dwC, DWORD dwD);
	// hover.exe:0x41A6B0 bounce the player off pWall; returns nonzero
	// when the wall was actually hit
	int Sub_41A6B0(CGameObj* pWall, DWORD dwB);
	// hover.exe:0x41AB10 reflect the velocity about dwAngle and feed the
	// tangential part back into the heading and the turn rate
	void Sub_41AB10(DWORD dwFlip, DWORD dwAngle);
	// hover.exe:0x421B50 the endpoint case of the wall bounce: bounce off
	// the segment endpoint selected by dwFirst, returns nonzero when the
	// player was actually moving into it
	int Sub_421B50(DWORD dwFlip, DWORD dwX, DWORD dwY, CGameObj* pWall,
		DWORD dwFirst, int nDist);

// Member variables (hover.exe offsets in comments)
	DWORD m_dwUnk0084;		// +0x84 (set by CHumanPlayer)
	DWORD m_dwUnk0088;		// +0x88
	DWORD m_dwUnk008C;		// +0x8C
	WORD m_awUnk0090[12];	// +0x90 = {0}
	char m_pad00A8[0xC];	// +0xA8
	// All four are CObList(10): CPlayer::CPlayer calls 0x4491E4 at
	// 0x41ACB8/0x41ACCC/0x41ACE0/0x41ACF4, and 0x4491E4 stores the
	// CObList vtable 0x4C2118 (rule 251 - libmatch misnames it
	// CPtrList::CPtrList).
	CObList m_obList00B4;	// +0xB4
	CObList m_obList00D0;	// +0xD0
	CObList m_obList00EC;	// +0xEC
	CObList m_obList0108;	// +0x108
	DWORD m_dwUnk0124;		// +0x124 (zeroed by CHumanPlayer)
	DWORD m_dwUnk0128;		// +0x128 (zeroed by CHumanPlayer)
	DWORD m_dwUnk012C;		// +0x12C (zeroed by CHumanPlayer)
	DWORD m_dwUnk0130;		// +0x130 (zeroed by CHumanPlayer)
	DWORD m_dwUnk0134;		// +0x134 (zeroed by CHumanPlayer)
	DWORD m_dwUnk0138;		// +0x138 (zeroed by CHumanPlayer)
	DWORD m_dwUnk013C;		// +0x13C
	DWORD m_dwUnk0140;		// +0x140 (zeroed by CHumanPlayer)
	DWORD m_dwUnk0144;		// +0x144 (zeroed by CHumanPlayer)
	DWORD m_dwUnk0148;		// +0x148
	DWORD m_dwUnk014C;		// +0x14C
	DWORD m_dwUnk0150;		// +0x150
	DWORD m_dwUnk0154;		// +0x154
	DWORD m_dwUnk0158;		// +0x158
	DWORD m_dwUnk015C;		// +0x15C
	char m_pad0160[0x4];	// +0x160
	DWORD m_dwUnk0164;		// +0x164 projected triangle x1
	DWORD m_dwUnk0168;		// +0x168 projected triangle y1
	DWORD m_dwUnk016C;		// +0x16C projected triangle x2
	DWORD m_dwUnk0170;		// +0x170 projected triangle y2
	DWORD m_dwUnk0174;		// +0x174 projected triangle x3
	DWORD m_dwUnk0178;		// +0x178 projected triangle y3
	DWORD m_dwUnk017C;		// +0x17C visible flag
	DWORD m_dwUnk0180;		// +0x180 proximity point 1 x (Sub_42AD70)
	DWORD m_dwUnk0184;		// +0x184 proximity point 1 y
	DWORD m_dwUnk0188;		// +0x188 proximity point 2 x
	DWORD m_dwUnk018C;		// +0x18C proximity point 2 y
	DWORD m_dwUnk0190;		// +0x190 proximity range limit
	WORD m_awUnk0194[6];	// +0x194 = {0}
	DWORD m_dwUnk01A0;		// +0x1A0 (zeroed by Sub_414A80, not by the ctor)
	DWORD m_dwUnk01A4;		// +0x1A4
	DWORD m_dwUnk01A8;		// +0x1A8 (zeroed by Sub_414A80, not by the ctor)
	WORD m_awUnk01AC[12];	// +0x1AC = {0}
	DWORD m_dwUnk01C4;		// +0x1C4 (zeroed by Sub_414A80)
	DWORD m_dwUnk01C8;		// +0x1C8
	DWORD m_dwUnk01CC;		// +0x1CC (zeroed by Sub_42AD40/Sub_42B8E0)
	WORD m_awUnk01D0[6];	// +0x1D0 = {0}
	DWORD m_dwUnk01DC;		// +0x1DC
	DWORD m_dwUnk01E0;		// +0x1E0
	DWORD m_dwUnk01E4;		// +0x1E4 sprite frame for the current heading
	DWORD m_dwUnk01E8;		// +0x1E8 sprite frame published to the renderer
	DWORD m_dwUnk01EC;		// +0x1EC nonzero: draw the frame mirrored
	WORD m_wUnk01F0;		// +0x1F0 = 0
	WORD m_wUnk01F2;		// +0x1F2 = 0
	WORD m_wUnk01F4;		// +0x1F4 = 0
	WORD m_wUnk01F6;		// +0x1F6 = 0
	DWORD m_dwUnk01F8;		// +0x1F8 = 0 (= 1 in CHumanPlayer)
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
	virtual void Sub_42AD30();				// hover.exe:0x409180
	virtual void Sub_42AD40(CPlayer* pOther, DWORD dwB); // hover.exe:0x40C400
	virtual void Sub_4087D0();				// hover.exe:0x4089D0
	virtual void Sub_4087F0();				// hover.exe:0x4089F0
	virtual void Sub_413020();				// hover.exe:0x42C240
};							// sizeof = 0x1FC

/////////////////////////////////////////////////////////////////////////////
// CMerlinWorld: the level world manager pointed to by
// CBumperDoc::m_dwUnk0188 (CRuntimeClass at hover.exe:0x4C51E0, size 0x64,
// schema 1, base CObject; vtable at hover.exe:0x4BBEC8). Its vtable slot 2
// Serialize override (hover.exe:0x418DC0) loads the maze: a four-WORD
// header followed by the four CObArrays of maze objects embedded at
// +0x14/+0x28/+0x3C/+0x50 -- the static faces, the sprites, the named
// points and the BSP nodes. The renderer walks the same three arrays
// through this pointer (CSub406090::Sub_406090/Sub_405FA0).

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
	// or above the g_dwUnk_461CFC mip level
	DWORD Sub_418EA0();
	// hover.exe:0x418F00 realize an all-black PC_NOCOLLAPSE palette on the
	// display to force a system palette rebuild
	void Sub_418F00();
	// hover.exe:0x418FA0 rebuild g_paletteUnk_4A1D50 from the system
	// palette: static entries keep their colors, the dynamic middle
	// entries take the game palette marked PC_NOCOLLAPSE and are made
	// unique
	void Sub_418FA0();

	// hover.exe:0x413DB0 remove dwUnk from the arrays and delete it
	int Sub_413DB0(DWORD dwUnk);
	// hover.exe:0x4128A0 add dwUnk to m_arrUnk28 unless it is already
	// there; returns nonzero when it was added
	int Sub_4128A0(DWORD dwUnk);
	// hover.exe:0x414300 remove dwUnk from m_arrUnk28 (if present) and
	// delete it; returns nonzero when it was on the array
	int Sub_414300(DWORD dwUnk);

	// Typed accessors for the renderer's walks over the three arrays.
	CMerlinStatic* GetStatic(int i)
		{ return (CMerlinStatic*)m_arrUnk14[i]; }
	CMerlinDynamic* GetDynamic(int i)
		{ return (CMerlinDynamic*)m_arrUnk28[i]; }
	CMerlinBSP* GetNode(int i)
		{ return (CMerlinBSP*)m_arrUnk50[i]; }

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
	CObArray m_arrUnk14;	// +0x14 CMerlinStatic faces (m_pData at +0x18)
	CObArray m_arrUnk28;	// +0x28 CMerlinDynamic sprites (m_pData at
							//   +0x2C, m_nSize at +0x30)
	CObArray m_arrUnk3C;	// +0x3C CMerlinLocation points
	CObArray m_arrUnk50;	// +0x50 CMerlinBSP nodes (m_pData at +0x54,
							//   node 0 is the root)
};							// sizeof = 0x64 in hover.exe

/////////////////////////////////////////////////////////////////////////////
// CSub40B800: sound-slot objects referenced from CBumperDoc::m_adwUnk83EC
// (indices 0x1A-0x1D are the player sound slots). CObject-derived (plain
// CObject vtable at hover.exe:0x4BD8E0; deleting dtor 0x423120); the
// constructor registers the slot in g_arrUnk_4A47F0 and preloads the
// wave file.

class CSub40B800 : public CObject
{
public:
	CSub40B800(const char* pszName, DWORD dwUnk);	// hover.exe:0x415380
	// hover.exe:0x422CB0 (virtual through CObject; the deleting dtor the
	// compiler puts in vtable slot 1 is 0x423120)
	~CSub40B800();

	// hover.exe:0x417860 load the wave file (raw buffer when WaveMix is
	// unavailable, MIXWAVE handle otherwise); returns 0 on failure
	int Sub_417860(const char* pszName);
	// hover.exe:0x40B800 play the slot's sound (WaveMix when the engine is
	// up, otherwise queued to the sound worker thread)
	int Sub_40B800(DWORD dwA, DWORD dwB, DWORD dwC);
	// hover.exe:0x4118A0 stop the sound (WaveMixFlushChannel / worker queue)
	int Sub_4118A0(DWORD dwA, DWORD dwB);

// Member variables (hover.exe offsets in comments)
	void* m_pUnk04;			// +0x04 sound data (played via SND_MEMORY by the
							// sound worker thread Hover_sub_42D6A0)
	void* m_pUnk08;			// +0x08 open MIXWAVE handle (WaveMix path)
	CString m_strUnk0C;		// +0x0C wave file name (".wav" is appended)
	DWORD m_dwUnk18;		// +0x18 nonzero: open/play through WaveMix
};

/////////////////////////////////////////////////////////////////////////////
// CSub40B510 is the skeletal name this reconstruction previously used for
// the class at CBumperDoc::m_dwUnk8388; the original's CRuntimeClass names
// it CCollider (0x4C55A0) — see texload.h. Sub_40B510 is implemented in
// gameobj.cpp (below) as CCollider::Sub_40B510.

/////////////////////////////////////////////////////////////////////////////
// Globals defined in gameobj.cpp

extern DWORD g_dwUnk_4C52D8;	// hover.exe:0x4C52D8 CGameObj instance list head
extern DWORD g_dwUnk_4C52DC;	// hover.exe:0x4C52DC CGameObj secondary list head
extern DWORD g_dwUnk_4C54A0;	// hover.exe:0x4C54A0 = -1 (last sound state)
extern CMapPtrToPtr g_mapUnk_4A4820;	// hover.exe:0x4A4820 MIXWAVE handle ->
									// CSub40B800 slot map (Sub_40B800)
extern CObArray g_arrUnk_4A47F0;	// hover.exe:0x4A47F0 sound slot
									// registry (CSub40B800::CSub40B800)
extern DWORD g_dwUnk_4C5700;		// hover.exe:0x4C5700 sound slot count

#endif // __GAMEOBJ_H__
