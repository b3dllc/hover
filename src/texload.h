// texload.h : loader/helper functions decompiled from hover.exe
//
// Contents:
//   - CCollider (the class gameobj.h forward-declares) and
//     its embedded CRegionMatrix
//   - CRobotPlayer (the 0x688-byte robot/flag player object) with its
//     seven embedded AI component objects and its pool operator new
//   - CRenderContext renderer context object
//   - A mirror class used to decompile the CPlayer method hover.exe:0x41B0E0,
//     which is not declared in the shared gameobj.h
//   - Free helper functions Hover_NearestPaletteIndex / Hover_BlitRect
//
// Note on member offsets: the comments give each member's offset within
// the class in hover.exe.

#ifndef __TEXLOAD_H__
#define __TEXLOAD_H__

/////////////////////////////////////////////////////////////////////////////
// REGIONNODE: one element of the per-cell object lists held in
// CCollider's region matrix cell array (CRegionMatrix::m_pCells). The nodes
// form a doubly-linked list per cell (+0x00/+0x04) and are also chained onto
// each occupied object's m_dwUnk30 field through +0x08.

typedef struct tagREGIONNODE
{
	struct tagREGIONNODE* m_pNext;	// +0x00 next node in the cell
	struct tagREGIONNODE* m_pPrev;	// +0x04 previous node in the cell
	struct tagREGIONNODE* m_pNextObj;	// +0x08 next node on the object's chain
	CGameObj* m_pObj;				// +0x0C object occupying the cell

	tagREGIONNODE();				// hover.exe:0x415030
} REGIONNODE;

/////////////////////////////////////////////////////////////////////////////
// Note on hover.exe:0x4BB670 (the vtable the constructors of CCollider,
// CRegionMatrix, CBeaconSearch and the whole CMerlin* family store into the
// vptr before storing their own). It used to be read here as an
// undocumented empty intermediate base class, declared as "CSub429530".
// It is not: 0x4BB670 is CObject's own vtable, and the store is just
// CObject's inline constructor being inlined into the derived one.
// Evidence:
//   - slot 0 (0x44E70A) returns the CRuntimeClass at 0x4C6A20, whose name
//     string is "CObject" (size 4, schema 0xFFFF);
//   - slots 2/3/4 are the empty CObject::Serialize/AssertValid/Dump
//     (0x425360/0x425370/0x425380), and slot 1 (0x429530) is CObject's
//     scalar deleting destructor -- a COMDAT the linker happened to place
//     in the game's address range, like the out-of-line copy of
//     CObject::~CObject at 0x401000;
//   - the same store appears in MFC's own library code (CObArray's
//     constructor at 0x4494EA, and 0x448894/0x449202/0x44B467/...), which
//     cannot possibly derive from a Hover-specific class.
// So every class below derives from CObject directly.

/////////////////////////////////////////////////////////////////////////////
// CRegionMatrix: grid of REGIONNODE list heads covering the game board.
// CRuntimeClass "CRegionMatrix" at 0x4C55B8 (size 0x18, DYNAMIC, base link
// CObject); vtable at hover.exe:0x4BD078. The constructor is inlined into
// CCollider::CCollider in the original.

class CRegionMatrix : public CObject
{
	DECLARE_DYNAMIC(CRegionMatrix)

public:
	CRegionMatrix() { m_pCells = NULL; m_dwCells = 0; m_pCellsEnd = NULL; }
	virtual ~CRegionMatrix();	// hover.exe:0x41B4F0 (operator delete of
								// m_pCells)

	// hover.exe:0x4086A0 allocate a REGIONNODE and insert pObj into cell
	// (nX,nY), linking the node onto pObj->m_dwUnk30
	REGIONNODE* AddNode(int nX, int nY, CGameObj* pObj);

	REGIONNODE** m_pCells;		// +0x04 = 0 cell array (allocated by
								// CCollider::CCollider)
	DWORD m_dwCols;			// +0x08 cell columns
	DWORD m_dwRows;			// +0x0C cell rows
	DWORD m_dwCells;			// +0x10 = 0 cell count
	REGIONNODE** m_pCellsEnd;		// +0x14 = 0 end of the cell array
};								// sizeof = 0x18

/////////////////////////////////////////////////////////////////////////////
// CCollider: the class gameobj.h forward-declares (the 0x5C-byte object
// at CBumperDoc::m_pCollider, constructed with (0x5A00, 0x5A00, 0x100) by
// the level loader).
//   CRuntimeClass "CCollider" at 0x4C55A0 (size 0x5C, DYNAMIC, base link
//   CObject); vtable at hover.exe:0x4BD100.

class CBeacon;	// mazeobj.h

class CCollider : public CObject
{
	DECLARE_DYNAMIC(CCollider)

public:
	// hover.exe:0x41B560. The extent arrives as one 8-byte value: the
	// only call site (hover.exe:0x415CAA) stores 0x5A00 into two
	// adjacent temporaries, emits an inline's `jmp $+5` return jump and
	// only then pushes both halves, and the constructor is `ret 0xC`.
	CCollider(CSize sizeExtent, DWORD dwCell);
	virtual ~CCollider();		// (member destruction only; the deleting
								// destructor is hover.exe:0x41B6C0)

	// hover.exe:0x40F900 notify pObj through its vtable slot 0x30
	// virtual, passing this object; always returns 1
	int AddStatic(CGameObj* pObj);
	// hover.exe:0x40E710 release pObj's region-matrix node chain; always
	// returns 1 (the collider itself is not touched)
	int RemoveObject(CGameObj* pObj);
	// hover.exe:0x40A650 per-frame collision pass over the registered
	// objects
	void DetectCollisions();
	// hover.exe:0x40F8A0 check the objects in cell (nX,nY) against the
	// registered-object array
	void CollectCell(int nX, int nY);
	// hover.exe:0x40F9B0 returns nonzero when the (lX,lY,lR) spot is out
	// of range or occupied by a non-CFloorObj object
	// The x/y pair arrives as one 8-byte value: both call sites
	// (hover.exe:0x412306 and 0x417DD3) widen +0x14 and +0x16 into it
	// left to right before the radius is even loaded.
	int IsSpotBlocked(POINT pt, long lR);
	// hover.exe:0x40B510 release the player's m_dwUnk30 chain and notify
	// it via its vtable slot 0x30 virtual, passing this object
	// (implemented in gameobj.cpp)
	int ReinsertObject(CPlayer* pPlayer);
	// hover.exe:0x41B770 register an object (m_listMovers.AddHead + notify
	// via the vtable slot 0x30 virtual)
	int AddMover(CGameObj* pObj);
	// hover.exe:0x411600 register a wall between the two points with
	// pBeacon (implemented in mazeobj.cpp)
	void ProbeSegment(DWORD dwX1, DWORD dwY1, DWORD dwX2, DWORD dwY2,
		int nLimit, CBeacon* pBeacon);
	// hover.exe:0x40C2B0 test the segment (dwX1,dwY1)-(dwX2,dwY2) against
	// the registered walls (implemented in mazeobj.cpp)
	int IsPathClear(DWORD dwX1, DWORD dwY1, DWORD dwX2, DWORD dwY2,
		DWORD dwHi, DWORD dwLo, int nLimit);

	DWORD m_dwCols;			// +0x04 = dwA / dwC (cell columns)
	DWORD m_dwRows;			// +0x08 = dwB / dwC (cell rows)
	DWORD m_dwCellSize;			// +0x0C = dwC (cell size)
	CRegionMatrix m_grid;		// +0x10
	// CObList and CObArray, not CPtrList/CDWordArray: the constructor
	// calls 0x4491E4 (stores the CObList vtable 0x4C2118) for +0x28 and
	// 0x4494CC (stores ??_7CObArray@@6B@, 0x4C2340) for +0x48 (rule 251).
	CObList m_listMovers;		// +0x28 registered objects (AddMover)
	DWORD m_bQuerying;			// +0x44 = 0
	CObArray m_arrQueryHits;		// +0x48
};								// sizeof = 0x5C

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// CRobotState: shared base of the seven CGameObj-derived AI component
// objects embedded in CRobotPlayer. vtable at hover.exe:0x4BE148 (slot 0
// is CGameObj::GetRuntimeClass - no CRuntimeClass of its own); destructor
// at hover.exe:0x423670 (trivial chain). The constructor is inlined into
// the component constructors in the original.

class CRobotPlayer;

class CRobotState : public CGameObj
{
public:
	CRobotState(CRobotPlayer* pParent) { m_pRobot = pParent; }
	virtual ~CRobotState();				// hover.exe:0x423670 (??_G 0x42F180)

// The AI components are a state machine: the owning CRobotPlayer keeps the
// active state in m_pState and calls its Think (slot 7) once per
// frame. Slots 18/19 - new virtuals introduced by this class - are the
// state entry/exit hooks (vtable offsets 0x48 and 0x4C); slot 14
// (OnProbeHit) answers "does this state handle the object class passed
// in?" and is used by CPlayer::OnTouch's collision routing.
	virtual void Think();			// slot 7, hover.exe:0x42ECD0 (think)
	virtual int OnProbeHit(CGameObj* pObj);	// slot 14, hover.exe:0x41BEB0
	virtual void OnEnter();			// slot 18, hover.exe:0x42ECE0 (enter)
	virtual void OnLeave();			// slot 19, hover.exe:0x42ECF0 (leave)

	// hover.exe:0x41BC10 leave this state and make pNext the parent's
	// active state
	void ChangeState(CRobotState* pNext);
	// hover.exe:0x40A790 when the human player is in sight, lock onto him
	// and switch to CRobotRamState; returns nonzero when the state changed
	int TryLockOnHuman();
	// hover.exe:0x40BB40 same for a flag robot chasing the flag carrier
	// (switches to CRobotFlagChaseState); returns nonzero when the state
	// changed
	int TryLockOnFlag();
	// hover.exe:0x42ED00 nonzero when the collider reports a clear path
	// between the human player and (dwX, dwY)
	int CanSeeHuman(DWORD dwX, DWORD dwY, DWORD dwA, DWORD dwB);
	// hover.exe:0x40BA10 the same test taken from (dwX, dwY) against every
	// flag still in play; the flag it finds comes back in *ppTarget
	int FindVisibleFlag(DWORD dwX, DWORD dwY, DWORD dwA, DWORD dwB,
		CGameObj** ppTarget);

	DWORD m_bBlocked;			// +0x84 (not initialized by the ctors)
	CRobotPlayer* m_pRobot;		// +0x88 owning CRobotPlayer
};								// sizeof = 0x8C

/////////////////////////////////////////////////////////////////////////////
// The seven AI component classes. None has a CRuntimeClass; the names are
// the constructor addresses. The original vtables (0x4BE1F0, 0x4BE2A8,
// 0x4BE350, 0x4BE3F8, 0x4BE4A0, 0x4BE570, 0x4BE620) override CGameObj
// virtuals that are not yet decompiled, so these declarations currently
// inherit them.

// CRobotSeekState - "roam" / target-acquisition state (the robot's initial and
// fallback state; the parent's m_pState starts here).

class CRobotSeekState : public CRobotState
{
public:
	CRobotSeekState(CRobotPlayer* pParent);	// hover.exe:0x4235E0

	virtual void Think();			// hover.exe:0x414D70 (think)
	virtual int OnProbeHit(CGameObj* pObj);	// hover.exe:0x41BF20
	virtual void OnEnter();			// hover.exe:0x41BF80 (enter)
	virtual void OnLeave();			// hover.exe:0x415040 (leave)

	DWORD m_bProbing;			// +0x8C = 0; 1 only around the ProbeSegment call
};								// sizeof = 0x90

// CRobotSpinState - "stunned / recover" state entered after a collision: it
// counts m_dwSpinFrames down and falls back to CRobotSeekState.

class CRobotSpinState : public CRobotState
{
public:
	CRobotSpinState(CRobotPlayer* pParent);	// hover.exe:0x423710

	virtual void Think();			// hover.exe:0x42ED90 (think)
	virtual void OnEnter();			// hover.exe:0x42EE10 (enter)
	virtual void OnLeave();			// hover.exe:0x42EF00 (leave)

	DWORD m_dwSpinFrames;			// +0x8C countdown
	DWORD m_dwUnk90;			// +0x90
};								// sizeof = 0x94

// CRobotGotoState - "chase" state: steers towards m_pRobot->m_pTarget
// and hands over to CRobotFlagWaitState/CRobotRouteState when the target is
// close enough.

class CRobotGotoState : public CRobotState
{
public:
	CRobotGotoState(CRobotPlayer* pParent);	// hover.exe:0x4237A0

	virtual void Think();			// hover.exe:0x40A860 (think)
	virtual void OnEnter();			// hover.exe:0x40BC50 (enter)
	virtual void OnLeave();			// hover.exe:0x40BC70 (leave)

	DWORD m_dwTimeout;			// +0x8C countdown
};								// sizeof = 0x90

// CRobotFlagChaseState - "line of sight" state: keeps going while the collider
// still reports a clear path to the target, otherwise falls back.

class CRobotFlagChaseState : public CRobotState
{
public:
	CRobotFlagChaseState(CRobotPlayer* pParent);	// hover.exe:0x4238D0

	virtual void Think();			// hover.exe:0x4150F0 (think)
	virtual void OnEnter();			// hover.exe:0x41C010 (enter)

	DWORD m_dwTimeout;			// +0x8C countdown
};								// sizeof = 0x90

// CRobotFlagWaitState - "approach beacon" state (uses CRobotRouteState's
// search result).

class CRobotFlagWaitState : public CRobotState
{
public:
	CRobotFlagWaitState(CRobotPlayer* pParent);	// hover.exe:0x423960

	virtual void Think();			// hover.exe:0x40BC90 (think)
	virtual void OnEnter();			// hover.exe:0x42EF30 (enter)
};								// sizeof = 0x8C

// One frame of CBeaconSearch's walk stack: a beacon and the index of the
// next link of it to follow.
typedef struct tagBEACONFRAME
{
	CBeacon* pBeacon;			// +0x00 beacon on the walk path
	int nChild;					// +0x04 next link of it to visit
} BEACONFRAME;					// sizeof = 0x08

/////////////////////////////////////////////////////////////////////////////
// CBeaconSearch: object embedded at +0x8C in CRobotRouteState. vtable at
// hover.exe:0x4BE558 (slot 0 is CObject::GetRuntimeClass); destructor at
// hover.exe:0x424130 (trivial chain). The constructor is inlined into
// CRobotRouteState's constructor in the original.

class CBeaconSearch : public CObject
{
public:
	CBeaconSearch()
	{
		memset(m_aFrames, 0, sizeof(m_aFrames));
		m_dwDepthLimit = 0;
		m_dwDepth = 0;
	}

	// hover.exe:0x40BEF0 pop the next unvisited beacon off the search
	// stack (breadth-first walk of the beacon graph); returns NULL when
	// the stack runs dry
	CBeacon* NextBeacon();

	DWORD m_dwDepthLimit;			// +0x04 = 0 visited-entry limit
	DWORD m_dwDepth;			// +0x08 = 0 stack depth
	BEACONFRAME* m_pTop;		// +0x0C current stack frame
	BEACONFRAME m_aFrames[5];	// +0x10 = {0} the stack frames themselves
	DWORD m_dwIteration;			// +0x38 search step counter (-1 = idle);
								//   +0xC4 seen from the owning CRobotRouteState
};								// sizeof = 0x3C

// CRobotRouteState - "route to the human player" state: walks the beacon graph
// with the embedded CBeaconSearch search stack until it finds a beacon with
// a clear line of sight, then hands over to CRobotGotoState.

class CRobotRouteState : public CRobotState
{
public:
	CRobotRouteState(CRobotPlayer* pParent);	// hover.exe:0x423A90

	virtual void Think();			// hover.exe:0x40BCD0 (think)
	virtual void OnEnter();			// hover.exe:0x40BF80 (enter)

	// hover.exe:0x40BFF0 give up on the current beacon and back out of
	// the search
	void GiveUp();

	CBeaconSearch m_search;			// +0x8C
};								// sizeof = 0xC8

// CRobotRamState - "flee / avoid" state: turns away when a wall blocks the
// straight line to the target.

class CRobotRamState : public CRobotState
{
public:
	CRobotRamState(CRobotPlayer* pParent);	// hover.exe:0x423B80

	virtual void Think();			// hover.exe:0x42EF40 (think)
	virtual void OnEnter();			// hover.exe:0x42F000 (enter)
};								// sizeof = 0x8C

/////////////////////////////////////////////////////////////////////////////
// CRobotPlayer: the robot/flag player object created by the level loader
// with argument 1 (robot) or 0. Allocated from the 0x42C320 pool.
//   CRuntimeClass "CRobotPlayer" at 0x4C54B8 (size 0x688, DYNAMIC, base
//   CPlayer); vtable at hover.exe:0x4BCCE0.

class CRobotPlayer : public CPlayer
{
	DECLARE_DYNAMIC(CRobotPlayer)

public:
	CRobotPlayer(DWORD dwRobot);	// hover.exe:0x4220A0
	// hover.exe:0x422250 (vtable slot 1 deleting dtor); destroys the seven
	// AI components and pushes the object back onto the g_pRobotFree
	// free list
	virtual ~CRobotPlayer()
	{
		// Empty body: the original's deleting destructor is exactly the
		// compiler-generated chain (the seven AI components in reverse
		// declaration order, then ~CPlayer) followed by the inlined
		// CRobotPlayer::operator delete.
	}

// Overrides of the CGameObj/CPlayer virtuals (see gameobj.h; the comment
// records the address of the original CRobotPlayer implementation).
	virtual void Think();				// hover.exe:0x409300
	virtual void OnTouch(CPlayer* pOther, DWORD dwB); // hover.exe:0x4144D0
	virtual int OnProbeHit(CGameObj* pObj);	// hover.exe:0x42C3C0
	virtual void BeginFrame();				// hover.exe:0x40A4C0
	virtual void EndFrame();				// hover.exe:0x40A590
	virtual void KillObject();				// hover.exe:0x42C3B0

	// hover.exe:0x409350 the robot's steering/throttle pass (STUB)
	void Steer();
	// hover.exe:0x40BC10 push pObj onto the recently-visited ring buffer
	void RememberVisited(CGameObj* pObj);
	// hover.exe:0x40C1F0 nonzero when pObj is on that ring buffer
	int WasRecentlyVisited(CGameObj* pObj);

	// hover.exe:0x42C320 pool allocator: carves the 0x688-byte objects out
	// of g_dwRobotPoolBlock-sized blocks (each block registered in
	// g_listRobotBlocks and freed at shutdown). The matching deallocator
	// only pushes the object back onto the free list; the original has it
	// inlined in the deleting destructor, so it is inline here too.
	void* operator new(size_t cb);
	void operator delete(void* p)
	{
		((CRobotPlayer*)p)->m_pNextFree = g_pRobotFree;
		g_pRobotFree = (CRobotPlayer*)p;
	}

	CGameObj* m_pTarget;			// +0x1FC = 0 current AI target object
	union						// +0x200 target heading (16.16 angle; the
	{							//   constructor clears the halves separately)
		DWORD m_fxTargetHeading;
		struct
		{
			WORD m_wTargetHeadingFrac;	// +0x200 = 0
			WORD m_wTargetHeadingInt;	// +0x202 = 0
		};
	};
	DWORD m_bBrakeAtTarget;	// +0x204 stop at the target (set by the goto state)
	DWORD m_bSpinning;			// +0x208 in the post-collision spin state
	DWORD m_bSpinLeft;			// +0x20C spin direction
	DWORD m_dwUnk0210;			// +0x210
	DWORD m_bStopping;			// +0x214 forces the wanted speed to 0 near the target
	DWORD m_bOvershooting;		// +0x218 = 0; latched when it would drift past the target
	DWORD m_bHunter;			// +0x21C = dwRobot: 1 hunts the human, 0 steals flags
	CRobotState* m_pState;		// +0x220 = &m_stSeek
	CGameObj* m_apRecent[10];		// +0x224 = {0}
	DWORD m_dwRecentNext;			// +0x24C = 0
	char m_pad0250[0x10];		// +0x250
	CRobotPlayer* m_pNextFree;			// +0x260 free-list link (operator new)
	CRobotSeekState m_stSeek;		// +0x264
	CRobotSpinState m_stSpin;		// +0x2F4
	CRobotGotoState m_stGoto;		// +0x388
	CRobotFlagChaseState m_stFlagChase;		// +0x418
	CRobotFlagWaitState m_stFlagWait;		// +0x4A8
	CRobotRouteState m_stRoute;		// +0x534
	CRobotRamState m_stRam;		// +0x5FC
};								// sizeof = 0x688

/////////////////////////////////////////////////////////////////////////////
// Free helper functions



// hover.exe:0x413730 nearest-palette-match: returns the index in
// [10, 0xF6) of the palette entry (RGB triples, dword stride, entries at
// pPal + 0x28) closest to dwColor by summed squared component distance
int Hover_NearestPaletteIndex(DWORD dwColor, const BYTE* pPal);

// hover.exe:0x40E730 clipped span blitter used by the span renderers
void Hover_BlitRect(long lA1, long lA2, long lA3, long lA4, long lA5,
	long lA6, long lA7, long lA8, long lA9, long lA10, long lA11, long lA12,
	BYTE* pSrc, BYTE* pDst, int bUnk);

/////////////////////////////////////////////////////////////////////////////
// Globals defined in texload.cpp

extern DWORD g_dwRobotPoolBlock;	// hover.exe:0x4C54D4 = 0xA CRobotPlayer pool
								// chunk size (objects per allocation)
extern DWORD g_dwSpritePoolBlock;	// hover.exe:0x4C5624 = 0x20 renderer-sprite
								// pool chunk size (objects per allocation)

// MIDI stream driver entry points resolved from winmm.dll by
// CMidiStream::LoadMusicFile (g_pfnMidiStreamClose/g_pfnMidiStreamPause are in
// bumperdoc.cpp/bumperdoc.h)
extern LPMIDIDEVICEFN g_pfnMidiStreamOpen;	// hover.exe:0x4C5908 midiStreamOpen
extern LPMIDIDEVICEFN g_pfnMidiStreamProperty;	// hover.exe:0x4C5910 midiStreamProperty
extern LPMIDIDEVICEFN g_pfnMidiStreamPosition;	// hover.exe:0x4C5914 midiStreamPosition
extern LPMIDIDEVICEFN g_pfnMidiStreamOut;	// hover.exe:0x4C5918 midiStreamOut
extern LPMIDIDEVICEFN g_pfnMidiStreamRestart;	// hover.exe:0x4C5920 midiStreamRestart
extern LPMIDIDEVICEFN g_pfnMidiStreamStop;	// hover.exe:0x4C5924 midiStreamStop

// Distance-square table: g_awSquares[255 + d] = d*d for d in
// [-255, 255] (used by Hover_NearestPaletteIndex through its center at
// hover.exe:0x4C4FAE)
extern WORD g_awSquares[512];	// hover.exe:0x4C4DB0

/////////////////////////////////////////////////////////////////////////////

#endif // __TEXLOAD_H__
