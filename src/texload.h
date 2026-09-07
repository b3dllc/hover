// texload.h : loader/helper functions decompiled from hover.exe
//
// Contents:
//   - CCollider (the class gameobj.h's skeletal CSub40B510 refers to) and
//     its embedded CRegionMatrix
//   - CRobotPlayer (the 0x688-byte robot/flag player object) with its
//     seven embedded AI component objects and its pool operator new
//   - CSub410F20 renderer context object
//   - A mirror class used to decompile the CPlayer method hover.exe:0x41B0E0,
//     which is not declared in the shared gameobj.h
//   - Free helper functions Hover_sub_413730 / Hover_sub_40E730
//
// Note on member offsets: the comments give each member's offset within
// the class in hover.exe.

#ifndef __TEXLOAD_H__
#define __TEXLOAD_H__

/////////////////////////////////////////////////////////////////////////////
// REGIONNODE: one element of the per-cell object lists held in
// CCollider's region matrix cell array (CRegionMatrix::m_pUnk04). The nodes
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
// CRegionMatrix, CSub424130 and the whole CMerlin* family store into the
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
	CRegionMatrix() { m_pUnk04 = NULL; m_dwUnk10 = 0; m_pUnk14 = NULL; }
	virtual ~CRegionMatrix();	// hover.exe:0x41B4F0 (operator delete of
								// m_pUnk04)

	// hover.exe:0x4086A0 allocate a REGIONNODE and insert pObj into cell
	// (nX,nY), linking the node onto pObj->m_dwUnk30
	REGIONNODE* Sub_4086A0(int nX, int nY, CGameObj* pObj);

	DWORD* m_pUnk04;			// +0x04 = 0 cell array (allocated by
								// CCollider::CCollider)
	DWORD m_dwUnk08;			// +0x08 cell columns
	DWORD m_dwUnk0C;			// +0x0C cell rows
	DWORD m_dwUnk10;			// +0x10 = 0 cell count
	DWORD* m_pUnk14;			// +0x14 = 0 end of the cell array
};								// sizeof = 0x18

/////////////////////////////////////////////////////////////////////////////
// CCollider: the class gameobj.h's skeletal CSub40B510 declaration refers
// to (the 0x5C-byte object at CBumperDoc::m_dwUnk8388, constructed with
// (0x5A00, 0x5A00, 0x100) by the level loader).
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
	int Sub_40F900(CGameObj* pObj);
	// hover.exe:0x40E710 release pObj's region-matrix node chain; always
	// returns 1 (the collider itself is not touched)
	int Sub_40E710(CGameObj* pObj);
	// hover.exe:0x40A650 per-frame collision pass over the registered
	// objects
	void Sub_40A650();
	// hover.exe:0x40F8A0 check the objects in cell (nX,nY) against the
	// registered-object array
	void Sub_40F8A0(int nX, int nY);
	// hover.exe:0x40F9B0 returns nonzero when the (lX,lY,lR) spot is out
	// of range or occupied by a non-CFloorObj object
	// The x/y pair arrives as one 8-byte value: both call sites
	// (hover.exe:0x412306 and 0x417DD3) widen +0x14 and +0x16 into it
	// left to right before the radius is even loaded.
	int Sub_40F9B0(POINT pt, long lR);
	// hover.exe:0x40B510 release the player's m_dwUnk30 chain and notify
	// it via its vtable slot 0x30 virtual, passing this object
	// (implemented in gameobj.cpp)
	int Sub_40B510(CPlayer* pPlayer);
	// hover.exe:0x41B770 register an object (m_listUnk28.AddHead + notify
	// via the vtable slot 0x30 virtual)
	int Sub_41B770(CGameObj* pObj);
	// hover.exe:0x411600 register a wall between the two points with
	// pBeacon (implemented in mazeobj.cpp)
	void Sub_411600(DWORD dwX1, DWORD dwY1, DWORD dwX2, DWORD dwY2,
		int nLimit, CBeacon* pBeacon);
	// hover.exe:0x40C2B0 test the segment (dwX1,dwY1)-(dwX2,dwY2) against
	// the registered walls (implemented in mazeobj.cpp)
	int Sub_40C2B0(DWORD dwX1, DWORD dwY1, DWORD dwX2, DWORD dwY2,
		DWORD dwHi, DWORD dwLo, int nLimit);

	DWORD m_dwUnk04;			// +0x04 = dwA / dwC (cell columns)
	DWORD m_dwUnk08;			// +0x08 = dwB / dwC (cell rows)
	DWORD m_dwUnk0C;			// +0x0C = dwC (cell size)
	CRegionMatrix m_obj10;		// +0x10
	// CObList and CObArray, not CPtrList/CDWordArray: the constructor
	// calls 0x4491E4 (stores the CObList vtable 0x4C2118) for +0x28 and
	// 0x4494CC (stores ??_7CObArray@@6B@, 0x4C2340) for +0x48 (rule 251).
	CObList m_listUnk28;		// +0x28 registered objects (Sub_41B770)
	DWORD m_dwUnk44;			// +0x44 = 0
	CObArray m_arrUnk48;		// +0x48
};								// sizeof = 0x5C

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// CSub423670: shared base of the seven CGameObj-derived AI component
// objects embedded in CRobotPlayer. vtable at hover.exe:0x4BE148 (slot 0
// is CGameObj::GetRuntimeClass - no CRuntimeClass of its own); destructor
// at hover.exe:0x423670 (trivial chain). The constructor is inlined into
// the component constructors in the original.

class CRobotPlayer;

class CSub423670 : public CGameObj
{
public:
	CSub423670(CRobotPlayer* pParent) { m_pUnk88 = pParent; }
	virtual ~CSub423670();				// hover.exe:0x423670 (??_G 0x42F180)

// The AI components are a state machine: the owning CRobotPlayer keeps the
// active state in m_pUnk0220 and calls its Sub_42AD30 (slot 7) once per
// frame. Slots 18/19 - new virtuals introduced by this class - are the
// state entry/exit hooks (vtable offsets 0x48 and 0x4C); slot 14
// (Sub_42AD90) answers "does this state handle the object class passed
// in?" and is used by CPlayer::Sub_42AD40's collision routing.
	virtual void Sub_42AD30();			// slot 7, hover.exe:0x42ECD0 (think)
	virtual int Sub_42AD90(CGameObj* pObj);	// slot 14, hover.exe:0x41BEB0
	virtual void Sub_42ECE0();			// slot 18, hover.exe:0x42ECE0 (enter)
	virtual void Sub_42ECF0();			// slot 19, hover.exe:0x42ECF0 (leave)

	// hover.exe:0x41BC10 leave this state and make pNext the parent's
	// active state
	void Sub_41BC10(CSub423670* pNext);
	// hover.exe:0x40A790 when the human player is in sight, lock onto him
	// and switch to CSub423B80; returns nonzero when the state changed
	int Sub_40A790();
	// hover.exe:0x40BB40 same for a flag robot chasing the flag carrier
	// (switches to CSub4238D0); returns nonzero when the state changed
	int Sub_40BB40();
	// hover.exe:0x42ED00 nonzero when the collider reports a clear path
	// between the human player and (dwX, dwY)
	int Sub_42ED00(DWORD dwX, DWORD dwY, DWORD dwA, DWORD dwB);
	// hover.exe:0x40BA10 the same test taken from (dwX, dwY) against every
	// flag still in play; the flag it finds comes back in *pdwTarget
	int Sub_40BA10(DWORD dwX, DWORD dwY, DWORD dwA, DWORD dwB,
		DWORD* pdwTarget);

	DWORD m_dwUnk84;			// +0x84 (not initialized by the ctors)
	CRobotPlayer* m_pUnk88;		// +0x88 owning CRobotPlayer
};								// sizeof = 0x8C

/////////////////////////////////////////////////////////////////////////////
// The seven AI component classes. None has a CRuntimeClass; the names are
// the constructor addresses. The original vtables (0x4BE1F0, 0x4BE2A8,
// 0x4BE350, 0x4BE3F8, 0x4BE4A0, 0x4BE570, 0x4BE620) override CGameObj
// virtuals that are not yet decompiled, so these declarations currently
// inherit them.

// CSub4235E0 - "roam" / target-acquisition state (the robot's initial and
// fallback state; the parent's m_pUnk0220 starts here).

class CSub4235E0 : public CSub423670
{
public:
	CSub4235E0(CRobotPlayer* pParent);	// hover.exe:0x4235E0

	virtual void Sub_42AD30();			// hover.exe:0x414D70 (think)
	virtual int Sub_42AD90(CGameObj* pObj);	// hover.exe:0x41BF20
	virtual void Sub_42ECE0();			// hover.exe:0x41BF80 (enter)
	virtual void Sub_42ECF0();			// hover.exe:0x415040 (leave)

	DWORD m_dwUnk8C;			// +0x8C = 0 countdown/timer
};								// sizeof = 0x90

// CSub423710 - "stunned / recover" state entered after a collision: it
// counts m_dwUnk8C down and falls back to CSub4235E0.

class CSub423710 : public CSub423670
{
public:
	CSub423710(CRobotPlayer* pParent);	// hover.exe:0x423710

	virtual void Sub_42AD30();			// hover.exe:0x42ED90 (think)
	virtual void Sub_42ECE0();			// hover.exe:0x42EE10 (enter)
	virtual void Sub_42ECF0();			// hover.exe:0x42EF00 (leave)

	DWORD m_dwUnk8C;			// +0x8C countdown
	DWORD m_dwUnk90;			// +0x90
};								// sizeof = 0x94

// CSub4237A0 - "chase" state: steers towards m_pUnk88->m_pUnk01FC and
// hands over to CSub423960/CSub423A90 when the target is close enough.

class CSub4237A0 : public CSub423670
{
public:
	CSub4237A0(CRobotPlayer* pParent);	// hover.exe:0x4237A0

	virtual void Sub_42AD30();			// hover.exe:0x40A860 (think)
	virtual void Sub_42ECE0();			// hover.exe:0x40BC50 (enter)
	virtual void Sub_42ECF0();			// hover.exe:0x40BC70 (leave)

	DWORD m_dwUnk8C;			// +0x8C countdown
};								// sizeof = 0x90

// CSub4238D0 - "line of sight" state: keeps going while the collider
// still reports a clear path to the target, otherwise falls back.

class CSub4238D0 : public CSub423670
{
public:
	CSub4238D0(CRobotPlayer* pParent);	// hover.exe:0x4238D0

	virtual void Sub_42AD30();			// hover.exe:0x4150F0 (think)
	virtual void Sub_42ECE0();			// hover.exe:0x41C010 (enter)

	DWORD m_dwUnk8C;			// +0x8C countdown
};								// sizeof = 0x90

// CSub423960 - "approach beacon" state (uses CSub423A90's search result).

class CSub423960 : public CSub423670
{
public:
	CSub423960(CRobotPlayer* pParent);	// hover.exe:0x423960

	virtual void Sub_42AD30();			// hover.exe:0x40BC90 (think)
	virtual void Sub_42ECE0();			// hover.exe:0x42EF30 (enter)
};								// sizeof = 0x8C

/////////////////////////////////////////////////////////////////////////////
// CSub424130: object embedded at +0x8C in CSub423A90. vtable at
// hover.exe:0x4BE558 (slot 0 is CObject::GetRuntimeClass); destructor at
// hover.exe:0x424130 (trivial chain). The constructor is inlined into
// CSub423A90's constructor in the original.

class CSub424130 : public CObject
{
public:
	CSub424130()
	{
		memset(m_adwUnk10, 0, sizeof(m_adwUnk10));
		m_dwUnk04 = 0;
		m_dwUnk08 = 0;
	}

	// hover.exe:0x40BEF0 pop the next unvisited beacon off the search
	// stack (breadth-first walk of the beacon graph); returns NULL when
	// the stack runs dry
	CBeacon* Sub_40BEF0();

	DWORD m_dwUnk04;			// +0x04 = 0 visited-entry limit
	DWORD m_dwUnk08;			// +0x08 = 0 stack depth
	DWORD* m_pUnk0C;			// +0x0C current stack frame (pairs of
								//   {beacon, child index})
	DWORD m_adwUnk10[10];		// +0x10 = {0} the stack frames themselves
	DWORD m_dwUnk38;			// +0x38 search step counter (-1 = idle);
								//   +0xC4 seen from the owning CSub423A90
};								// sizeof = 0x3C

// CSub423A90 - "route to the human player" state: walks the beacon graph
// with the embedded CSub424130 search stack until it finds a beacon with
// a clear line of sight, then hands over to CSub4237A0.

class CSub423A90 : public CSub423670
{
public:
	CSub423A90(CRobotPlayer* pParent);	// hover.exe:0x423A90

	virtual void Sub_42AD30();			// hover.exe:0x40BCD0 (think)
	virtual void Sub_42ECE0();			// hover.exe:0x40BF80 (enter)

	// hover.exe:0x40BFF0 give up on the current beacon and back out of
	// the search
	void Sub_40BFF0();

	CSub424130 m_obj8C;			// +0x8C
};								// sizeof = 0xC8

// CSub423B80 - "flee / avoid" state: turns away when a wall blocks the
// straight line to the target.

class CSub423B80 : public CSub423670
{
public:
	CSub423B80(CRobotPlayer* pParent);	// hover.exe:0x423B80

	virtual void Sub_42AD30();			// hover.exe:0x42EF40 (think)
	virtual void Sub_42ECE0();			// hover.exe:0x42F000 (enter)
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
	// AI components and pushes the object back onto the g_dwUnk_4C54D0
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
	virtual void Sub_42AD30();				// hover.exe:0x409300
	virtual void Sub_42AD40(CPlayer* pOther, DWORD dwB); // hover.exe:0x4144D0
	virtual int Sub_42AD90(CGameObj* pObj);	// hover.exe:0x42C3C0
	virtual void Sub_4087D0();				// hover.exe:0x40A4C0
	virtual void Sub_4087F0();				// hover.exe:0x40A590
	virtual void Sub_413020();				// hover.exe:0x42C3B0

	// hover.exe:0x409350 the robot's steering/throttle pass (STUB)
	void Sub_409350();
	// hover.exe:0x40BC10 push pObj onto the recently-visited ring buffer
	void Sub_40BC10(CGameObj* pObj);
	// hover.exe:0x40C1F0 nonzero when pObj is on that ring buffer
	int Sub_40C1F0(CGameObj* pObj);

	// hover.exe:0x42C320 pool allocator: carves the 0x688-byte objects out
	// of g_dwUnk_4C54D4-sized blocks (each block registered in
	// g_listUnk_4A46A0 and freed at shutdown). The matching deallocator
	// only pushes the object back onto the free list; the original has it
	// inlined in the deleting destructor, so it is inline here too.
	void* operator new(size_t cb);
	void operator delete(void* p)
	{
		((CRobotPlayer*)p)->m_pUnk0260 = (void*)g_dwUnk_4C54D0;
		g_dwUnk_4C54D0 = (DWORD)p;
	}

	DWORD m_dwUnk01FC;			// +0x1FC = 0 current AI target object
	WORD m_wUnk0200;			// +0x200 = 0 target heading (also read as a
								//   dword by the AI states)
	WORD m_wUnk0202;			// +0x202 = 0
	DWORD m_dwUnk0204;			// +0x204 turn-in-place request
	DWORD m_dwUnk0208;			// +0x208
	DWORD m_dwUnk020C;			// +0x20C reverse flag
	DWORD m_dwUnk0210;			// +0x210
	DWORD m_dwUnk0214;			// +0x214 "line of sight valid" flag
	DWORD m_dwUnk0218;			// +0x218 = 0
	DWORD m_dwUnk021C;			// +0x21C = dwRobot
	CSub423670* m_pUnk0220;		// +0x220 = &m_obj0264
	DWORD m_adwUnk0224[10];		// +0x224 = {0}
	DWORD m_dwUnk024C;			// +0x24C = 0
	char m_pad0250[0x10];		// +0x250
	void* m_pUnk0260;			// +0x260 free-list link (operator new)
	CSub4235E0 m_obj0264;		// +0x264
	CSub423710 m_obj02F4;		// +0x2F4
	CSub4237A0 m_obj0388;		// +0x388
	CSub4238D0 m_obj0418;		// +0x418
	CSub423960 m_obj04A8;		// +0x4A8
	CSub423A90 m_obj0534;		// +0x534
	CSub423B80 m_obj05FC;		// +0x5FC
};								// sizeof = 0x688

/////////////////////////////////////////////////////////////////////////////
// Forward declarations

class CSub406090;

/////////////////////////////////////////////////////////////////////////////
// Forward declarations

class CSub406090;

/////////////////////////////////////////////////////////////////////////////
// CSub410F20: renderer context object (its address is kept in the
// CBumperDoc/manager objects at +0xC8/+0xCC and passed as 'this' to
// hover.exe:0x410F20). Selects the render/span function set for a mode
// (dwMode 1..6) and two option bits in dwFlags. Only the members touched
// by Sub_410F20 are known.

class CSub410F20
{
public:
	// hover.exe:0x420ED0 allocates the span-drawer function table
	// (m_pUnk08) and the shift-count table (m_pUnk14)
	CSub410F20();
	// hover.exe:0x420F60 frees the eight tables the context owns
	~CSub410F20();
	// hover.exe:0x410F20
	void Sub_410F20(DWORD dwFlags, DWORD dwMode);
	// hover.exe:0x406C00 set the camera/projection state
	void Sub_406C00(DWORD dwA, DWORD dwB, DWORD dwC, DWORD dwD);
	// hover.exe:0x410EC0
	void Sub_410EC0();
	// hover.exe:0x414A10
	void Sub_414A10(void* pA, void* pB);
	// hover.exe:0x419940
	void Sub_419940(DWORD dwA, DWORD dwB);
	// hover.exe:0x419530 render-target palette/shade setup
	void Sub_419530(CSub406090* pTarget, CPalette* pPal, int nA, int nB,
		int nWidth, int nHeight);
	// hover.exe:0x419960 helper used by Sub_419530 to fill m_pUnk6C
	void Sub_419960(int nBase, BYTE bRed, BYTE bGreen, BYTE bBlue,
		CPalette* pPalette);

	DWORD m_pfnUnk00;			// +0x00 render dispatch function
	DWORD m_pfnUnk04;			// +0x04 render function
	DWORD* m_pUnk08;			// +0x08 -> object whose +0x18..+0x3C dword
								// slots receive the span function table
	DWORD m_pfnUnk0C;			// +0x0C render function
	char m_pad10[0x4];			// +0x10
	BYTE* m_pUnk14;				// +0x14 -> table filled by Sub_410EC0
	DWORD m_dwUnk18;			// +0x18 viewport X origin
	DWORD m_dwUnk1C;			// +0x1C viewport Y origin
	DWORD m_dwUnk20;			// +0x20 viewport width
	DWORD m_dwUnk24;			// +0x24 viewport height
	DWORD m_dwUnk28;			// +0x28 render target byte offset
	DWORD m_dwUnk2C;			// +0x2C viewport height / 2
	DWORD* m_pUnk30;			// +0x30 -> per-column lookup table
	DWORD* m_pUnk34;			// +0x34 span array (top)
	DWORD* m_pUnk38;			// +0x38 span array (bottom)
	DWORD m_dwUnk3C;			// +0x3C mode parameter
	DWORD m_dwUnk40;			// +0x40 mode parameter
	DWORD m_dwUnk44;			// +0x44 = dwMode
	DWORD m_dwUnk48;			// +0x48 camera x
	DWORD m_dwUnk4C;			// +0x4C camera y
	DWORD m_dwUnk50;			// +0x50 camera z
	DWORD m_dwUnk54;			// +0x54 camera angle
	DWORD m_dwUnk58;			// +0x58 = 0x80 (constructor default)
	DWORD* m_pUnk5C;			// +0x5C distance lookup table
	DWORD m_dwUnk60;			// +0x60 -> shade/palette table
	DWORD* m_pUnk64;			// +0x64 sin/cos lookup table
	DWORD m_dwUnk68;			// +0x68 nonzero: reverse the span-column
							//   order (CSub406090::Sub_406090)
	DWORD* m_pUnk6C;			// +0x6C vertical shade table
	DWORD m_dwUnk70;			// +0x70
	DWORD m_dwUnk74;			// +0x74
	DWORD m_dwUnk78;			// +0x78 render target width
	DWORD m_dwUnk7C;			// +0x7C render target width * target->m_dwUnk1C
	DWORD m_dwUnk80;			// +0x80 = (dwFlags & 2) >> 1
	DWORD m_dwUnk84;			// +0x84 render target height
	DWORD m_dwUnk88;			// +0x88 span array count
	DWORD m_dwUnk8C;			// +0x8C shade table parameter
	DWORD m_dwUnk90;			// +0x90 = dwFlags & 1
								// +0x94 padding before the doubles
	double m_dblUnk98;			// +0x98 = 64.0 (constructor default)
	double m_dblUnkA0;			// +0xA0 = 64.0 (constructor default)
	DWORD m_dwUnkA8;			// +0xA8 last camera z
};								// sizeof == 0xB0 (the size the original
								// passes to operator new)

/////////////////////////////////////////////////////////////////////////////
// Free helper functions



// hover.exe:0x413730 nearest-palette-match: returns the index in
// [10, 0xF6) of the palette entry (RGB triples, dword stride, entries at
// pPal + 0x28) closest to dwColor by summed squared component distance
int Hover_sub_413730(DWORD dwColor, const BYTE* pPal);

// hover.exe:0x40E730 clipped span blitter used by the span renderers
void Hover_sub_40E730(long lA1, long lA2, long lA3, long lA4, long lA5,
	long lA6, long lA7, long lA8, long lA9, long lA10, long lA11, long lA12,
	BYTE* pSrc, BYTE* pDst, int bUnk);

/////////////////////////////////////////////////////////////////////////////
// Globals defined in texload.cpp

extern DWORD g_dwUnk_4C54D4;	// hover.exe:0x4C54D4 = 0xA CRobotPlayer pool
								// chunk size (objects per allocation)
extern DWORD g_dwUnk_4C5624;	// hover.exe:0x4C5624 = 0x20 renderer-sprite
								// pool chunk size (objects per allocation)

// MIDI stream driver entry points resolved from winmm.dll by
// CSub425050::Sub_437F00 (g_pfnUnk_4C590C/g_pfnUnk_4C591C are in
// bumperdoc.cpp/bumperdoc.h)
extern LPMIDIDEVICEFN g_pfnUnk_4C5908;	// hover.exe:0x4C5908 midiStreamOpen
extern LPMIDIDEVICEFN g_pfnUnk_4C5910;	// hover.exe:0x4C5910 midiStreamProperty
extern LPMIDIDEVICEFN g_pfnUnk_4C5914;	// hover.exe:0x4C5914 midiStreamPosition
extern LPMIDIDEVICEFN g_pfnUnk_4C5918;	// hover.exe:0x4C5918 midiStreamOut
extern LPMIDIDEVICEFN g_pfnUnk_4C5920;	// hover.exe:0x4C5920 midiStreamRestart
extern LPMIDIDEVICEFN g_pfnUnk_4C5924;	// hover.exe:0x4C5924 midiStreamStop

// Distance-square table: g_awUnk_4C4DB0[255 + d] = d*d for d in
// [-255, 255] (used by Hover_sub_413730 through its center at
// hover.exe:0x4C4FAE)
extern WORD g_awUnk_4C4DB0[512];	// hover.exe:0x4C4DB0

/////////////////////////////////////////////////////////////////////////////

#endif // __TEXLOAD_H__
