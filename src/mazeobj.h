// mazeobj.h : interface of the maze object classes (the CPodObj and
// CFloorObj hierarchies, CWall, CBeacon) and their pool allocators
//
// Skeleton reconstructed from hover.exe:
//   CRuntimeClass "CPodObj" at 0x4C5318 (size 0x100, DYNAMIC, base CGameObj)
//   CRuntimeClass "CWall" at 0x4C5570 (size 0xA0, DYNAMIC, base CGameObj)
//   CRuntimeClass "CFloorObj" at 0x4C53C8 (size 0xB8, DYNAMIC, base CGameObj)
//   CRuntimeClass "CBeacon" at 0x4C5790 (size 0xB0, DYNAMIC, base CGameObj)
//   plus the pod/pad subclasses listed below (all DYNAMIC)
//
// The pod, pad and wall classes each override operator new/operator delete
// with a fixed-size pool: blocks of objects are carved out of one CRT
// allocation (kept in a global CPtrList so they can be released at
// shutdown) and threaded through a free list whose link lives in the
// object itself.
//
// Note on member offsets: the comments give each member's offset within
// the class in hover.exe.

#ifndef __MAZEOBJ_H__
#define __MAZEOBJ_H__

#include "gameobj.h"
#include "bumperdoc.h"
#include "texload.h"

class CMerlinTexture;

/////////////////////////////////////////////////////////////////////////////
// CPodObj: base class of the flag and the power-up pods. The constructor
// looks up the "POD06_00" texture; each subclass re-looks-up its own
// "POD0n_00" name and resets the shared members.
// vtable at hover.exe:0x4BC500.

class CPodObj : public CGameObj
{
	DECLARE_DYNAMIC(CPodObj)

public:
	CPodObj();				// hover.exe:0x413050

	// hover.exe:0x411580 (vtable slot 1 deleting dtor at 0x42B000); the
	// body is empty, only the CGameObj base destructor runs
	virtual ~CPodObj();

// Overrides of the CGameObj virtuals. The overriding function must keep
// the base name; the comment records the address of CPodObj's own
// implementation. Slots 7/17 only forward to CGameObj.
	virtual void Sub_42AD30();				// hover.exe:0x40D2E0 (slot 7)
	virtual void Sub_42AD40(CPlayer* pOther, DWORD dwB); // 0x4115D0 (slot 8)
	virtual int Sub_42AD70(DWORD dwA, DWORD dwB, DWORD dwC); // 0x413E70 (slot 10)
	virtual int Sub_42AD60(DWORD dwUnk);	// hover.exe:0x40CB40 (slot 11)
	virtual int Sub_408820(DWORD dwA);		// hover.exe:0x413180 (slot 12)
	virtual void Sub_4087D0();				// hover.exe:0x40E340 (slot 15)
	virtual void Sub_4087F0();				// hover.exe:0x40E380 (slot 16)
	virtual void Sub_413020();				// hover.exe:0x42AFB0 (slot 17)

// Pool allocator (blocks of g_dwUnk_4C5334 objects, blocks kept in
// g_listUnk_4A45B8, free list threaded through m_dwUnk84)
	static void* operator new(size_t nSize);	// hover.exe:0x42AF20
	static void operator delete(void* p);		// hover.exe:0x42AF90

// Member variables (hover.exe offsets in comments)
	DWORD m_dwUnk84;		// +0x84 pool free-list link
	DWORD m_dwUnk88;		// +0x88 = 0
	CMerlinTexture* m_pUnk8C;	// +0x8C pod texture ("POD06_00" in CPodObj)
	DWORD m_dwUnk90;		// +0x90
	DWORD m_dwUnk94;		// +0x94 = 0
	DWORD m_dwUnk98;		// +0x98 = 1
	DWORD m_dwUnk9C;		// +0x9C = 0xA (0x40 in the pod subclasses,
							// 0x80 in CFlag)
	DWORD m_dwUnkA0;		// +0xA0
	DWORD m_dwUnkA4;		// +0xA4 (pod position/variant; written as a
							// dword by CContainer/CBumperDoc::Sub_412250)
	DWORD m_dwUnkA8;		// +0xA8
	DWORD m_dwUnkAC;		// +0xAC = 0x64, then overwritten with 0
	DWORD m_dwUnkB0;		// +0xB0
	DWORD m_dwUnkB4;		// +0xB4 = 0x80
	DWORD m_dwUnkB8;		// +0xB8 = texture +0x18 word
	DWORD m_dwUnkBC;		// +0xBC = texture +0x14 word / 2
	DWORD m_dwUnkC0;		// +0xC0
	DWORD m_dwUnkC4;		// +0xC4
	DWORD m_dwUnkC8;		// +0xC8
	DWORD m_dwUnkCC;		// +0xCC (CFlag ctor argument)
	DWORD m_dwUnkD0;		// +0xD0
	DWORD m_dwUnkD4;		// +0xD4 = 0 in CTempWallPod
	DWORD m_dwUnkD8;		// +0xD8
	DWORD m_dwUnkDC;		// +0xDC
	WORD m_awUnkE0[8];		// +0xE0 = {0}
	DWORD m_dwUnkF0;		// +0xF0 projected temp-wall corner x1
	DWORD m_dwUnkF4;		// +0xF4 projected temp-wall corner y1
	DWORD m_dwUnkF8;		// +0xF8 projected temp-wall corner x2
	DWORD m_dwUnkFC;		// +0xFC projected temp-wall corner y2
};							// sizeof = 0x100

/////////////////////////////////////////////////////////////////////////////
// The power-up pods (no extra members; the constructors only set the
// vtable, re-initialize the CPodObj members and look up their own
// "POD0n_00" texture)

class CSmokeBombPod : public CPodObj	// CRuntimeClass 0x4C5270
{
	DECLARE_DYNAMIC(CSmokeBombPod)
public:
	CSmokeBombPod();		// hover.exe:0x420FD0 ("POD00_")
	virtual ~CSmokeBombPod() {}	// folded into deleting dtor hover.exe:0x421290

	virtual void Sub_42AD30();				// hover.exe:0x42A660 (slot 7)
	virtual void Sub_42AD40(CPlayer* pOther, DWORD dwB); // 0x42A760 (slot 8)
	virtual void Sub_42AD50(DWORD dwUnk);	// hover.exe:0x42A900 (slot 9)
	virtual void Sub_4087D0();				// hover.exe:0x421270 (slot 15)
	virtual void Sub_4087F0();				// hover.exe:0x421280 (slot 16)
	virtual void Sub_413020();				// hover.exe:0x42AA60 (slot 17)
};

class CJumpPod : public CPodObj	// CRuntimeClass 0x4C5298
{
	DECLARE_DYNAMIC(CJumpPod)
public:
	CJumpPod();				// hover.exe:0x421310 ("POD01_")
	virtual ~CJumpPod() {}	// folded into deleting dtor hover.exe:0x4213C0

	virtual void Sub_42AD30();				// hover.exe:0x42AA80 (slot 7)
	virtual void Sub_42AD40(CPlayer* pOther, DWORD dwB); // 0x42AAC0 (slot 8)
	virtual void Sub_42AD50(DWORD dwUnk);	// hover.exe:0x42AC60 (slot 9)
	virtual void Sub_4087D0();				// hover.exe:0x421300 (slot 15)
	virtual void Sub_4087F0();				// hover.exe:0x4213B0 (slot 16)
	virtual void Sub_413020();				// hover.exe:0x42ACE0 (slot 17)
};

class CInviciblePod : public CPodObj	// CRuntimeClass 0x4C52F0
{
	DECLARE_DYNAMIC(CInviciblePod)
public:
	CInviciblePod();		// hover.exe:0x421440 ("POD04_")
	virtual ~CInviciblePod() {}	// folded into deleting dtor hover.exe:0x4146C0

	virtual void Sub_42AD30();				// hover.exe:0x40DE80 (slot 7)
	virtual void Sub_42AD40(CPlayer* pOther, DWORD dwB); // 0x4181E0 (slot 8)
	virtual void Sub_4087D0();				// hover.exe:0x413E50 (slot 15)
	virtual void Sub_4087F0();				// hover.exe:0x413E60 (slot 16)
	virtual void Sub_413020();				// hover.exe:0x419DF0 (slot 17)
};

class CTempWallPod : public CPodObj	// CRuntimeClass 0x4C5340
{
	DECLARE_DYNAMIC(CTempWallPod)
public:
	CTempWallPod();			// hover.exe:0x421520 ("POD07_")
	virtual ~CTempWallPod() {}	// folded into deleting dtor hover.exe:0x4215D0

	virtual void Sub_42AD30();				// hover.exe:0x42B070 (slot 7)
	virtual void Sub_42AD40(CPlayer* pOther, DWORD dwB); // 0x42B160 (slot 8)
	virtual void Sub_42AD50(DWORD dwUnk);	// hover.exe:0x42B3B0 (slot 9)
	virtual int Sub_42AD60(DWORD dwUnk);	// hover.exe:0x41A3A0 (slot 11)
	virtual int Sub_408820(DWORD dwA);		// hover.exe:0x421640 (slot 12)
	virtual void Sub_4087D0();				// hover.exe:0x421430 (slot 15)
	virtual void Sub_4087F0();				// hover.exe:0x4214E0 (slot 16)
	virtual void Sub_413020();				// hover.exe:0x42B690 (slot 17)
};

class CSpeedPod : public CPodObj	// CRuntimeClass 0x4C5378
{
	DECLARE_DYNAMIC(CSpeedPod)
public:
	CSpeedPod();			// hover.exe:0x421710 ("POD02_")
	virtual ~CSpeedPod() {}	// folded into deleting dtor hover.exe:0x41A510

	virtual void Sub_42AD30();				// hover.exe:0x414370 (slot 7)
	virtual void Sub_42AD40(CPlayer* pOther, DWORD dwB); // 0x421070 (slot 8)
	virtual void Sub_4087D0();				// hover.exe:0x414A40 (slot 15)
	virtual void Sub_4087F0();				// hover.exe:0x414A70 (slot 16)
	virtual void Sub_413020();				// hover.exe:0x421700 (slot 17)
};

// CMapEraserPod hides every non-pod object from the overhead map when it
// is picked up; vtable at hover.exe:0x4BCDC0.

class CMapEraserPod : public CPodObj	// CRuntimeClass 0x4C54F8
{
	DECLARE_DYNAMIC(CMapEraserPod)
public:
	CMapEraserPod();		// hover.exe:0x4223C0 ("POD05_")
	virtual ~CMapEraserPod() {}	// folded into deleting dtor hover.exe:0x41B350

	virtual void Sub_42AD30();				// hover.exe:0x42C410 (slot 7)
	virtual void Sub_42AD40(CPlayer* pOther, DWORD dwB); // 0x422470 (slot 8)
	virtual void Sub_4087D0();				// hover.exe:0x41B2C0 (slot 15)
	virtual void Sub_4087F0();				// hover.exe:0x41B340 (slot 16)
	virtual void Sub_413020();				// hover.exe:0x422620 (slot 17)
};

// CSlowPod slows the player down for theApp.m_dwUnk0160 ticks and drives
// the dashboard gauge at CBumperView::m_obj0340 while it runs; vtable at
// hover.exe:0x4BCE80.

class CSlowPod : public CPodObj	// CRuntimeClass 0x4C5520
{
	DECLARE_DYNAMIC(CSlowPod)
public:
	CSlowPod();				// hover.exe:0x422630 ("POD03_")
	virtual ~CSlowPod() {}	// folded into deleting dtor hover.exe:0x41B3E0

	virtual void Sub_42AD30();				// hover.exe:0x42C430 (slot 7)
	virtual void Sub_42AD40(CPlayer* pOther, DWORD dwB); // 0x4226D0 (slot 8)
	virtual void Sub_4087D0();				// hover.exe:0x41B3C0 (slot 15)
	virtual void Sub_4087F0();				// hover.exe:0x41B3D0 (slot 16)
	virtual void Sub_413020();				// hover.exe:0x422910 (slot 17)
};

/////////////////////////////////////////////////////////////////////////////
// CFlag (CRuntimeClass at 0x4C5418, vtable at hover.exe:0x4BC9F0)

class CFlag : public CPodObj
{
	DECLARE_DYNAMIC(CFlag)

public:
	// hover.exe:0x421CD0; dwUnk selects the texture set (nonzero:
	// "FLG00_00", zero: "FLG01_00") and is kept in m_dwUnkCC
	CFlag(DWORD dwUnk);
	virtual ~CFlag() {}		// folded into deleting dtor hover.exe:0x421DB0

	virtual void Sub_42AD30();				// hover.exe:0x42C160 (slot 7)
	virtual void Sub_42AD40(CPlayer* pOther, DWORD dwB); // 0x41AE40 (slot 8)
	virtual int Sub_42AD60(DWORD dwUnk);	// hover.exe:0x414470 (slot 11)
	virtual void Sub_4087D0();				// hover.exe:0x414D00 (slot 15)
	virtual void Sub_4087F0();				// hover.exe:0x414D10 (slot 16)
	virtual void Sub_413020();				// hover.exe:0x42C170 (slot 17)
};

/////////////////////////////////////////////////////////////////////////////
// CContainer (CRuntimeClass "CContainer" at 0x4C5438, size 0x100, DYNAMIC,
// base CPodObj; vtable at hover.exe:0x4BCA88): the "POD06_" pod nested
// inside a random pod by CBumperDoc::Sub_412250. Named CSub41B160 in
// earlier revisions of this reconstruction; the class name comes from the
// CRuntimeClass its GetRuntimeClass (hover.exe:0x4038C0) returns.

class CContainer : public CPodObj
{
	DECLARE_DYNAMIC(CContainer)

public:
	// hover.exe:0x41B160 (pObj = owning pod; dwX/dwY = position;
	// dwPt = the owner's point pointer)
	CContainer(CPodObj* pObj, DWORD dwX, DWORD dwY, DWORD dwPt);
	virtual ~CContainer()	// deleting dtor hover.exe:0x41B230 (empty)
	{
		if (m_dwUnkD0 != 0)
			delete (CGameObj*)m_dwUnkD0;
	}

	virtual void Sub_42AD40(CPlayer* pOther, DWORD dwB); // 0x421E20 (slot 8)
	virtual void Sub_4087D0();				// hover.exe:0x414D20 (slot 15)
	virtual void Sub_4087F0();				// hover.exe:0x414D30 (slot 16)
	virtual void Sub_413020();				// hover.exe:0x421EC0 (slot 17)
};

/////////////////////////////////////////////////////////////////////////////
// CWall (CRuntimeClass at 0x4C5570, vtable at hover.exe:0x4BCFD8)

class CWall : public CGameObj
{
	DECLARE_DYNAMIC(CWall)

public:
	CWall();				// hover.exe:0x40E6A0

	// deleting dtor hover.exe:0x417BA0; the class destructor is empty and
	// inlined into it, only the CGameObj base destructor runs
	virtual ~CWall();

// Overrides of the CGameObj virtuals (slots 7/15/16/17 only forward).
	virtual void Sub_42AD30();				// hover.exe:0x42C660 (slot 7)
	virtual void Sub_42AD40(CPlayer* pOther, DWORD dwB); // 0x41B450 (slot 8)
	virtual int Sub_42AD70(DWORD dwA, DWORD dwB, DWORD dwC); // 0x422B90 (slot 10)
	virtual int Sub_42AD60(DWORD dwUnk);	// hover.exe:0x40D4B0 (slot 11)
	// hover.exe:0x40EF30 (slot 12) register the wall with the collider's
	// region matrix and collect any objects whose cells the wall passes
	// through into the collider's m_arrUnk48
	virtual int Sub_408820(DWORD dwA);
	virtual int Sub_411570(DWORD dwA, DWORD dwB, DWORD dwC, DWORD* pdwOut); // 0x40F7F0 (slot 13)
	virtual void Sub_4087D0();				// hover.exe:0x42C670 (slot 15)
	virtual void Sub_4087F0();				// hover.exe:0x42C680 (slot 16)
	virtual void Sub_413020();				// hover.exe:0x42C690 (slot 17)

	// hover.exe:0x414D40 (vtable slot 0x12 in the original) set the wall
	// texture: forwards to Sub_414A50 and initializes m_obj04 from the
	// texture's +0x14..+0x20 dwords
	virtual int Sub_414D40(CObject* pTexture);



// Pool allocator (blocks of g_dwUnk_4C558C objects, blocks kept in
// g_listUnk_4A46D0, free list threaded through m_dwUnk9C)
	static void* operator new(size_t nSize);	// hover.exe:0x42C5E0
	static void operator delete(void* p);	// folded into the original
										// deleting destructor (0x417BA0)

// Member variables (hover.exe offsets in comments)
	DWORD m_dwUnk84;		// +0x84
	DWORD m_dwUnk88;		// +0x88
	DWORD m_dwUnk8C;		// +0x8C texture ref
	DWORD m_dwUnk90;		// +0x90 texture ref
	DWORD m_dwUnk94;		// +0x94 texture ref
	DWORD m_dwUnk98;		// +0x98 texture ref
	DWORD m_dwUnk9C;		// +0x9C = 0 (pool free-list link)
};							// sizeof = 0xA0

/////////////////////////////////////////////////////////////////////////////
// CFloorObj (CRuntimeClass at 0x4C53C8, vtable at hover.exe:0x4BC7F0):
// floor pads; keeps a list of objects standing on the pad in m_list94.

class CFloorObj : public CGameObj
{
	DECLARE_DYNAMIC(CFloorObj)

public:
	CFloorObj();			// hover.exe:0x421930
	CFloorObj(DWORD dwUnk);	// hover.exe:0x41A610

	// hover.exe:0x421890 (vtable slot 1 deleting dtor at 0x4219D0);
	// deletes every object still standing on the pad
	virtual ~CFloorObj();

// Overrides of the CGameObj virtuals.
	virtual void Sub_42AD30();				// hover.exe:0x42B800 (slot 7)
	virtual void Sub_42AD40(CPlayer* pOther, DWORD dwB); // 0x40D730 (slot 8)
	virtual int Sub_42AD70(DWORD dwA, DWORD dwB, DWORD dwC); // 0x42B810 (slot 10)
	virtual int Sub_42AD60(DWORD dwUnk);	// hover.exe:0x40C620 (slot 11)
	virtual int Sub_408820(DWORD dwA);		// hover.exe:0x41A960 (slot 12)
	virtual int Sub_411570(DWORD dwA, DWORD dwB, DWORD dwC, DWORD* pdwOut); // 0x413220 (slot 13)
	virtual void Sub_4087D0();				// hover.exe:0x42B860 (slot 15)
	virtual void Sub_4087F0();				// hover.exe:0x42B870 (slot 16)
	virtual void Sub_413020();				// hover.exe:0x42B880 (slot 17)

// Pool allocator (blocks of g_dwUnk_4C53E4 objects, blocks kept in
// g_listUnk_4A45E0, free list threaded through m_dwUnk84)
	static void* operator new(size_t nSize);	// hover.exe:0x42B760
	static void operator delete(void* p);		// hover.exe:0x42B7E0

// Member variables (hover.exe offsets in comments)
	DWORD m_dwUnk84;		// +0x84 pool free-list link
	DWORD m_dwUnk88;		// +0x88
	DWORD m_dwUnk8C;		// +0x8C = 0
	DWORD m_dwUnk90;		// +0x90 = 0
	CObList m_list94;		// +0x94 CObList(10): both CFloorObj
							// constructors call 0x4491E4 (rule 251)
	DWORD m_dwUnkB0;		// +0xB0 = -1 (CFloorObj()) or the ctor argument
	DWORD m_dwUnkB4;		// +0xB4 (CSledPad ctor argument)
};							// sizeof = 0xB8

/////////////////////////////////////////////////////////////////////////////
// The pad subclasses (no extra members)

class CSledPad : public CFloorObj	// CRuntimeClass 0x4C53A0
{
	DECLARE_DYNAMIC(CSledPad)
public:
	// hover.exe:0x421820; dwUnk is the direction (0x100/0/0x180/0x80)
	CSledPad(DWORD dwUnk);
	virtual ~CSledPad() {}	// folded into deleting dtor hover.exe:0x4217B0

	virtual void Sub_42AD30();				// hover.exe:0x42B6C0 (slot 7)
	virtual void Sub_42AD40(CPlayer* pOther, DWORD dwB); // 0x41A5C0 (slot 8)
	virtual void Sub_4087D0();				// hover.exe:0x42B6D0 (slot 15)
	virtual void Sub_4087F0();				// hover.exe:0x42B6E0 (slot 16)
	virtual void Sub_413020();				// hover.exe:0x42B6F0 (slot 17)
};

class CHoldPad : public CFloorObj	// CRuntimeClass 0x4C5460
{
	DECLARE_DYNAMIC(CHoldPad)
public:
	CHoldPad();				// hover.exe:0x421ED0
	virtual ~CHoldPad() {}	// folded into deleting dtor hover.exe:0x421F30

	virtual void Sub_42AD30();				// hover.exe:0x42C1A0 (slot 7)
	virtual void Sub_42AD40(CPlayer* pOther, DWORD dwB); // 0x42C1B0 (slot 8)
	virtual void Sub_4087D0();				// hover.exe:0x42C200 (slot 15)
	virtual void Sub_4087F0();				// hover.exe:0x42C210 (slot 16)
	virtual void Sub_413020();				// hover.exe:0x42C220 (slot 17)
};

class CFlagRemover : public CFloorObj	// CRuntimeClass 0x4C5548
{
	DECLARE_DYNAMIC(CFlagRemover)
public:
	CFlagRemover();			// hover.exe:0x422AC0
	virtual ~CFlagRemover() {}	// folded into deleting dtor hover.exe:0x422B20

	virtual void Sub_42AD30();				// hover.exe:0x42C540 (slot 7)
	virtual void Sub_42AD40(CPlayer* pOther, DWORD dwB); // 0x422920 (slot 8)
	virtual void Sub_4087D0();				// hover.exe:0x42C550 (slot 15)
	virtual void Sub_4087F0();				// hover.exe:0x42C560 (slot 16)
	virtual void Sub_413020();				// hover.exe:0x42C570 (slot 17)
};

/////////////////////////////////////////////////////////////////////////////
// CSub424210: CObArray-derived class embedded in CBeacon at offset 0x84
// (the beacon's list of linked beacons). Like the original, it overrides
// only the (trivial) destructor; vtable at hover.exe:0x4BE6E0.

class CSub424210 : public CObArray
{
public:
	virtual ~CSub424210();	// hover.exe:0x424210 (trivial)
};

/////////////////////////////////////////////////////////////////////////////
// CBeacon (CRuntimeClass at 0x4C5790, vtable at hover.exe:0x4BE698).
// Beacons are built once per level by Hover_sub_423EA0, which also
// placement-constructs them straight out of the class pool (the original
// has no standalone constructor; CBeacon::operator new/delete are inlined
// there and in the deleting destructor at hover.exe:0x424180).

class CRobotPlayer;

class CBeacon : public CGameObj
{
	DECLARE_DYNAMIC(CBeacon)

public:
	inline CBeacon(CObject* pTexture);	// folded into hover.exe:0x423EA0

	// hover.exe:0x424180 is the scalar deleting destructor (vtable slot 1);
	// the class destructor itself is empty and folded into it
	virtual ~CBeacon() {}

	// hover.exe:0x423E50 (vtable slot 0x0E in the original) sets
	// m_dwUnkA4 when pObj is one of the pad classes the beacon ignores;
	// returns m_dwUnkA4
	virtual int Sub_42AD90(CGameObj* pObj);

	// hover.exe:0x40C0E0 pick the next hop of the beacon graph for
	// pParent (defined in texload.cpp, next to its only caller)
	CGameObj* Sub_40C0E0(CRobotPlayer* pParent);

// Pool allocator (blocks of g_dwUnk_4C57AC objects, blocks kept in
// g_listUnk_4A4858, free list threaded through m_dwUnkAC)
	inline static void* operator new(size_t nSize);
	// Inlined by the original into the deleting destructor at
	// hover.exe:0x424180: the pool push onto g_dwUnk_4C57A8.
	static void operator delete(void* p)
	{
		DWORD dwHead = g_dwUnk_4C57A8;

		g_dwUnk_4C57A8 = (DWORD)p;
		((CBeacon*)p)->m_dwUnkAC = dwHead;
	}

// Member variables (hover.exe offsets in comments)
	CSub424210 m_arr84;		// +0x84 linked beacons
	CObject* m_pUnk98;		// +0x98 the CMerlinLocation the beacon marks
	DWORD m_dwUnk9C;		// +0x9C
	DWORD m_dwUnkA0;		// +0xA0
	DWORD m_dwUnkA4;		// +0xA4 (set by Sub_42AD90, tested by
							// Hover_sub_423CD0)
	DWORD m_dwUnkA8;		// +0xA8 = 0
	DWORD m_dwUnkAC;		// +0xAC pool free-list link
};							// sizeof = 0xB0

/////////////////////////////////////////////////////////////////////////////
// Globals defined in mazeobj.cpp

// Pool block sizes (number of objects carved out of each allocated block;
// the pool heads and block lists are declared in hover.h)
extern DWORD g_dwUnk_4C5334;	// hover.exe:0x4C5334 = 10 (CPodObj pool)
extern DWORD g_dwUnk_4C53E4;	// hover.exe:0x4C53E4 = 10 (CFloorObj pool)
extern DWORD g_dwUnk_4C558C;	// hover.exe:0x4C558C = 0x20 (CWall pool)
extern DWORD g_dwUnk_4C57AC;	// hover.exe:0x4C57AC = 5 (CBeacon pool)
extern DWORD g_dwUnk_4C5590;	// hover.exe:0x4C5590 incremented by
								// CWall::Sub_408820 (0x40EF30), never read
extern DWORD g_dwUnk_4C5624;	// hover.exe:0x4C5624 = 0x20 (CMerlinDynamic pool)

extern CObArray g_obArrUnk_4A4840;	// hover.exe:0x4A4840 beacon table built
									// by Hover_sub_423EA0

/////////////////////////////////////////////////////////////////////////////
// Free functions

// hover.exe:0x423EA0 build the beacon table from the g_mapUnk_4A4768 maze
// texture objects whose uppercased names start with "BEACON_" (called
// once by CBumperDoc::Sub_415450)
void Hover_sub_423EA0();

// hover.exe:0x423CD0 link every pair of beacons whose connecting wall
// does not hit another registered object
void Hover_sub_423CD0();

// hover.exe:0x422BF0 and hover.exe:0x40E8E0 are CSub40D320 methods
// (declared in gameobj.h); hover.exe:0x40C2B0 and hover.exe:0x411600 are
// CCollider methods (declared in texload.h).

/////////////////////////////////////////////////////////////////////////////

#endif // __MAZEOBJ_H__
