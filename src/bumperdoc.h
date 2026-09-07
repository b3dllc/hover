// bumperdoc.h : interface of the CBumperDoc class
//
// Skeleton reconstructed from hover.exe:
//   CRuntimeClass "CBumperDoc" at 0x4C48D8 (size 0x84A0, DYNCREATE)
//   vtable at 0x4BAE10
//
// Note on member offsets: the comments give each member's offset within
// CBumperDoc in hover.exe. The original was built with an earlier MFC 4.x
// whose CDocument base class is a different size than the MFC 4.2 used for
// this build, so the offsets in our build will not match; the comments
// record the ground truth from the executable.

#ifndef __BUMPERDOC_H__
#define __BUMPERDOC_H__

#include "gameobj.h"

/////////////////////////////////////////////////////////////////////////////
// CSub425050: game class embedded in CBumperDoc at offset 0x60.
// Appears to manage MIDI music playback: its cleanup helper Sub_438260
// (called by the destructor) passes m_dwUnk18 to midiOutReset /
// midiOutUnprepareHeader and frees a linked list of buffers at m_dwUnk10.
// No DECLARE_DYNAMIC in the original (vtable slot 0 is the inherited
// CObject::GetRuntimeClass); vtable at hover.exe:0x4BFDA0.
// ctor hover.exe:0x425050, sizeof = 0x24.

class CSub425050 : public CObject
{
public:
	CSub425050();			// hover.exe:0x425050
	virtual ~CSub425050();	// hover.exe:0x4250C0

	// hover.exe:0x438260 cleanup: midiOutReset/midiOutUnprepareHeader
	// on m_dwUnk18 and frees the m_dwUnk10 buffer list
	void Sub_438260();
	// hover.exe:0x412220 closes m_dwUnk18 through g_pfnUnk_4C591C
	int Sub_412220();
	// hover.exe:0x413710 restart playback of the open stream through
	// g_pfnUnk_4C5920 (midiStreamRestart), or reopen it when there is none
	int Sub_413710();
	// hover.exe:0x438300 (re)open the MIDI stream device and queue the
	// prepared buffer list (hover.cpp)
	int Sub_438300();
	// hover.exe:0x437F00 load the MIDI stream driver and open pszPath
	int Sub_437F00(const char* pszPath);
	// hover.exe:0x41C030 open the level music; on success this object
	// becomes g_pUnk_4C5904
	int Sub_41C030(const char* pszPath);

	DWORD m_dwUnk04;		// +0x04 = -1
	DWORD m_dwUnk08;		// +0x08
	DWORD m_dwUnk0C;		// +0x0C
	DWORD m_dwUnk10;		// +0x10 = 0 (buffer list head)
	DWORD m_dwUnk14;		// +0x14 = 0
	DWORD m_dwUnk18;		// +0x18 = 0 (HMIDIOUT handle)
	DWORD m_dwUnk1C;		// +0x1C = 0
	DWORD m_dwUnk20;		// +0x20 = 0
};							// sizeof = 0x24

/////////////////////////////////////////////////////////////////////////////
// CSub41E1E0: CObList-derived class of the two CBumperDoc members at
// +0x838C and +0x83A8. It really is a class of its own: its vtable
// (hover.exe:0x4BAE90) is distinct from CObList's (0x4C2118), inherits
// CObList::GetRuntimeClass (0x45E232) and CObList::Serialize (0x449425)
// and overrides only the destructor, which is trivial. The constructor
// is inlined into CBumperDoc::CBumperDoc in the original (0x41E28B and
// 0x41E2BC call CObList::CObList(10) at 0x4491E4 - libmatch misnames
// that CPtrList, see rule 251 - and each is followed by the 0x4BAE90
// vtable store), so it is defined inline here as well.

class CSub41E1E0 : public CObList
{
public:
	CSub41E1E0() : CObList(10) { }
	virtual ~CSub41E1E0();	// hover.exe:0x41E1E0 (trivial)
};

/////////////////////////////////////////////////////////////////////////////
// CBumperDoc

// MIDI device close/release entry points resolved at runtime by the MIDI
// subsystem (both zero in hover.exe's data image)
typedef void (__stdcall* LPMIDIDEVICEFN)(DWORD dwDevice);
extern LPMIDIDEVICEFN g_pfnUnk_4C590C;	// hover.exe:0x4C590C
extern LPMIDIDEVICEFN g_pfnUnk_4C591C;	// hover.exe:0x4C591C
extern CSub425050* g_pUnk_4C5904;	// hover.exe:0x4C5904 object that opened
									// the current MIDI stream

// The 34 sound-slot names CBumperDoc::OnNewDocument turns into the
// CSub40B800 objects of m_adwUnk83EC, in the same order.
extern const char* g_apszUnk_4C4950[34];	// hover.exe:0x4C4950

// Fourth name -> object map cleared by CBumperDoc::DeleteContents; the
// other three (g_mapUnk_4A1D30, g_mapUnk_4A4578 and g_mapUnk_4A4768) live
// in texture.cpp, but nothing decompiled so far fills this one, so it is
// kept next to its only user.
extern CMapStringToOb g_mapUnk_4A4728;	// hover.exe:0x4A4728

// hover.exe:0x4151E0 delete every beacon in g_obArrUnk_4A4840 and empty
// the array (called by CBumperDoc::DeleteContents)
void Hover_sub_4151E0();

class CBumperDoc : public CDocument
{
	DECLARE_DYNCREATE(CBumperDoc)

public:
	CBumperDoc();			// hover.exe:0x41E230
	// The compiler folded the destructor into the scalar deleting
	// destructor at hover.exe:0x41E0D0 (slot 1 of the vtable).
	virtual ~CBumperDoc()	// hover.exe:0x41E0D0
	{
		int i;

		for (i = 0; i < 34; i++)
		{
			delete (CObject*)m_adwUnk83EC[i];
			m_adwUnk83EC[i] = 0;
		}
	}

// Overrides
	// hover.exe:0x426F10 empty override: the game keeps no serialized
	// document state (the level files are loaded by OnOpenDocument)
	virtual void Serialize(CArchive& ar);
	// hover.exe:0x414900 tear the loaded level down
	virtual void DeleteContents();
	// hover.exe:0x417B00 create the sound slots and load the first level
	virtual BOOL OnNewDocument();

	// hover.exe:0x415450 load the level: palette, textures, maze, and
	// game objects. This is slot 19 of the original's CBumperDoc vtable,
	// which is CDocument::OnOpenDocument in MFC 3.2 -- the original's
	// vtable has exactly the 32 slots of CDocument and overrides this
	// one, and OnNewDocument calls it with a NULL path. It was previously
	// reconstructed as a new virtual "Sub_415450(DWORD)", which put it in
	// a 33rd slot and left MFC's own OnOpenDocument in slot 19. The path
	// argument is ignored: the file names come from the level number.
	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
	// hover.exe:0x412250 place a flag/pod object on a random free pad
	void Sub_412250(CGameObj* pObj, int bUnk);
	// hover.exe:0x412630 retire pObj: unregister it from the collider and
	// relink it onto the pending-delete list (CGameObj vtable slot 17)
	void Sub_412630(CGameObj* pObj);
	// hover.exe:0x412650 unlink pObj from whichever of the two object lists
	// holds it (m_list83A8 first, then m_list838C); TRUE if it was found
	BOOL Sub_412650(CObject* pObj);
	// hover.exe:0x417C10 count the texture variants of the five
	// g_apszUnk_4C4910 families into m_adwUnk83D0
	void Sub_417C10();
	// hover.exe:0x417D40 returns a random start-position texture object of
	// the nUnk family (its +0x14..+0x1A words feed CPlayer::Sub_41B2D0),
	// or NULL when the family has no loaded textures
	CObject* Sub_417D40(int nUnk);
	// hover.exe:0x410250 remove pObj from the m_list838C active-pod list;
	// returns nonzero when it was on the list (defined in mazeobj.cpp)
	int Sub_410250(CObject* pObj);
	// hover.exe:0x426E90 (re)create the 34 wave slot objects of
	// m_adwUnk83EC from the g_apszWaveNames table
	void Sub_426E90();
	// hover.exe:0x426EE0 destroy the 34 wave slot objects
	void Sub_426EE0();

// Message map handlers (message map at hover.exe:0x4BAC90, entries at
// 0x4BAC98). All nine are unlisted debug commands; the names follow the
// reconstructed ID_CMD_34xxx identifiers because the original command
// names are unknown.
	//{{AFX_MSG(CBumperDoc)
	afx_msg void OnCmd34002();	// hover.exe:0x426F20 set player position
	afx_msg void OnCmd34027();	// hover.exe:0x427060 green scale -= 0.05
	afx_msg void OnCmd34026();	// hover.exe:0x4270D0 green scale += 0.05
	afx_msg void OnCmd34021();	// hover.exe:0x427140 shade base -= 0.001
	afx_msg void OnCmd34028();	// hover.exe:0x4271B0 shade base += 0.001
	afx_msg void OnCmd34025();	// hover.exe:0x427220 blue scale -= 0.05
	afx_msg void OnCmd34024();	// hover.exe:0x427290 blue scale += 0.05
	afx_msg void OnCmd34023();	// hover.exe:0x427300 red scale -= 0.05
	afx_msg void OnCmd34022();	// hover.exe:0x427370 red scale += 0.05
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:
// Member variables (hover.exe offsets in comments)
	CSub425050 m_obj0060;	// +0x60
	char m_pad0084[0x100];	// +0x84
	DWORD m_dwUnk0184;		// +0x184 = 0
	DWORD m_dwUnk0188;		// +0x188 = 0
	char m_pad018C[0x8000];	// +0x18C two CSub414550 shade tables at +0x18C
							// and +0x418C (texmisc.h), built at load time
	CHumanPlayer m_humanPlayer; // +0x818C
	DWORD m_dwUnk8388;		// +0x8388 = 0
	CSub41E1E0 m_list838C;	// +0x838C
	CSub41E1E0 m_list83A8;	// +0x83A8
	DWORD m_dwUnk83C4;		// +0x83C4 = 0x40 (texture dimension margin)
	DWORD m_dwUnk83C8;		// +0x83C8 largest texture dimension found
	DWORD m_dwUnk83CC;		// +0x83CC = 0x200
	DWORD m_adwUnk83D0[5];	// +0x83D0 = {0} texture variant counts for the
							// g_apszUnk_4C4910 families ("HUMAN_", "ROBOT_",
							// "FLAG_HUMAN_", "FLAG_ROBOT_", "POD_RANDOM_";
							// counted by Sub_417C10)
	DWORD m_dwUnk83E4;		// +0x83E4 = 0
	DWORD m_dwUnk83E8;		// +0x83E8
	DWORD m_adwUnk83EC[0x22]; // +0x83EC = {0}
	DWORD m_dwUnk8474;		// +0x8474
	// Shade-table parameters passed to the +0x18C CSub414550 table: the
	// per-level darkening base and the red/blue/green channel scales (the
	// original's argument order puts blue before green). The ID_CMD_34021
	// -> ID_CMD_34028 debug commands nudge them and rebuild the table.
	double m_dblUnk8478;	// +0x8478 = 0.985 shade base
	double m_dblUnk8480;	// +0x8480 = 1.2 red scale
	double m_dblUnk8488;	// +0x8488 = 1.2 blue scale
	double m_dblUnk8490;	// +0x8490 = 1.2 green scale
	DWORD m_dwUnk8498;		// +0x8498 = 1
	DWORD m_dwUnk849C;		// +0x849C
};							// sizeof = 0x84A0 in hover.exe

#endif // __BUMPERDOC_H__
