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
// CMidiStream: game class embedded in CBumperDoc at offset 0x60.
// Appears to manage MIDI music playback: its cleanup helper CloseStream
// (called by the destructor) passes m_hStream to midiOutReset /
// midiOutUnprepareHeader and frees a linked list of buffers at
// m_pFreeHead.
// No DECLARE_DYNAMIC in the original (vtable slot 0 is the inherited
// CObject::GetRuntimeClass); vtable at hover.exe:0x4BFDA0.
// ctor hover.exe:0x425050, sizeof = 0x24.

class CCollider;
class CMerlinWorld;

class CMidiStream : public CObject
{
public:
	CMidiStream();			// hover.exe:0x425050
	virtual ~CMidiStream();	// hover.exe:0x4250C0

	// hover.exe:0x438260 cleanup: midiOutReset/midiOutUnprepareHeader
	// on m_hStream and frees the m_pFreeHead buffer list
	void CloseStream();
	// hover.exe:0x412220 closes m_hStream through g_pfnMidiStreamPause
	int PauseMusic();
	// hover.exe:0x413710 restart playback of the open stream through
	// g_pfnMidiStreamRestart (midiStreamRestart), or reopen it when there is
	// none
	int ResumeMusic();
	// hover.exe:0x438300 (re)open the MIDI stream device and queue the
	// prepared buffer list (hover.cpp)
	int StartStream();
	// hover.exe:0x437F00 load the MIDI stream driver and open pszPath
	int LoadMusicFile(const char* pszPath);
	// hover.exe:0x41C030 open the level music; on success this object
	// becomes g_pCurrentMidiStream
	int OpenMusic(const char* pszPath);

	HANDLE m_hFile;		// +0x04 = -1
	DWORD m_dwTimeDiv;		// +0x08
	DWORD m_cbMaxBuffer;		// +0x0C
	MIDIHDR* m_pFreeHead;		// +0x10 = 0 (buffer list head)
	MIDIHDR* m_pFreeTail;		// +0x14 = 0
	HMIDISTRM m_hStream;		// +0x18 = 0 (HMIDIOUT handle)
	DWORD m_dwQueued;		// +0x1C = 0
	DWORD m_bStopping;		// +0x20 = 0
};							// sizeof = 0x24

/////////////////////////////////////////////////////////////////////////////
// CGameObjList: CObList-derived class of the two CBumperDoc members at
// +0x838C and +0x83A8. It really is a class of its own: its vtable
// (hover.exe:0x4BAE90) is distinct from CObList's (0x4C2118), inherits
// CObList::GetRuntimeClass (0x45E232) and CObList::Serialize (0x449425)
// and overrides only the destructor, which is trivial. The constructor
// is inlined into CBumperDoc::CBumperDoc in the original (0x41E28B and
// 0x41E2BC call CObList::CObList(10) at 0x4491E4 - libmatch misnames
// that CPtrList, see rule 251 - and each is followed by the 0x4BAE90
// vtable store), so it is defined inline here as well.

class CGameObjList : public CObList
{
public:
	CGameObjList() : CObList(10) { }
	virtual ~CGameObjList();	// hover.exe:0x41E1E0 (trivial)
};

/////////////////////////////////////////////////////////////////////////////
// CShadeTable: palette shade table (0x4000 bytes: 64 shade levels of 256
// palette indices each), CBumperDoc::m_aShade[2]. Built from the level
// palette when the level loads (BuildTable is defined in texmisc.cpp).

class CShadeTable
{
public:
	// hover.exe:0x414550 build the shade table from pPalette: for each of the
	// 64 levels the three scale factors are multiplied by dblBase
	// (darkening), and every palette entry's scaled/clamped RGB is mapped to
	// its nearest in-game color index by Hover_NearestPaletteIndex. NOTE: the
	// original passes the blue limit/scale before the green ones.
	void BuildTable(CPalette* pPalette, double dblBase, int nMaxR, int nMaxB,
		int nMaxG, double dblScaleR, double dblScaleB, double dblScaleG);

	BYTE m_abTable[0x4000];	// +0x00 [level * 0x100 + color] = shaded index
};							// sizeof = 0x4000

/////////////////////////////////////////////////////////////////////////////
// CBumperDoc

// midiStreamClose and midiStreamPause, resolved at runtime with
// GetProcAddress by CMidiStream::LoadMusicFile (both zero in hover.exe's
// data image)
typedef void (__stdcall* LPMIDIDEVICEFN)(HMIDISTRM hStream);
extern LPMIDIDEVICEFN g_pfnMidiStreamClose;	// hover.exe:0x4C590C
extern LPMIDIDEVICEFN g_pfnMidiStreamPause;	// hover.exe:0x4C591C
extern CMidiStream* g_pCurrentMidiStream;	// hover.exe:0x4C5904 object that opened
									// the current MIDI stream

// The 34 sound-slot names CBumperDoc::OnNewDocument turns into the
// CSoundSlot objects of m_adwSounds, in the same order.
extern const char* g_apszSoundNames[34];	// hover.exe:0x4C4950

// The loaded sprites by name: CMerlinDynamic::Serialize registers every
// one here, CBumperDoc::DeleteContents clears it. The other three name
// maps (g_mapUnk_4A1D30, g_mapUnk_4A4578 and g_mapUnk_4A4768) live in
// texture.cpp.
extern CMapStringToOb g_mapDynamicSprites;	// hover.exe:0x4A4728

// hover.exe:0x4151E0 delete every beacon in g_arrBeacons and empty
// the array (called by CBumperDoc::DeleteContents)
void Hover_DeleteBeacons();

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
			delete (CObject*)m_adwSounds[i];
			m_adwSounds[i] = 0;
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
	void PlaceObject(CGameObj* pObj, int bUnk);
	// hover.exe:0x412630 retire pObj: unregister it from the collider and
	// relink it onto the pending-delete list (CGameObj vtable slot 17)
	void RetireObject(CGameObj* pObj);
	// hover.exe:0x412650 unlink pObj from whichever of the two object lists
	// holds it (m_listActiveObjs first, then m_listMazeObjs); TRUE if it was
	// found
	BOOL RemoveObject(CObject* pObj);
	// hover.exe:0x417C10 count the texture variants of the five
	// g_apszSpawnFamilies families into m_adwSpawnPoints
	void CountSpawnPoints();
	// hover.exe:0x417D40 returns a random start-position texture object of
	// the nUnk family (its +0x14..+0x1A words feed CPlayer::Spawn),
	// or NULL when the family has no loaded textures
	CObject* GetRandomSpawnPoint(int nUnk);
	// hover.exe:0x410250 remove pObj from the m_listMazeObjs active-pod list;
	// returns nonzero when it was on the list (defined in mazeobj.cpp)
	int RemoveFromMazeList(CObject* pObj);
	// hover.exe:0x426E90 (re)create the 34 wave slot objects of
	// m_adwSounds from the g_apszWaveNames table
	void CreateSounds();
	// hover.exe:0x426EE0 destroy the 34 wave slot objects
	void DestroySounds();

// Message map handlers (message map at hover.exe:0x4BAC90, entries at
// 0x4BAC98). All nine are unlisted debug commands; the names follow the
// reconstructed ID_CMD_34xxx identifiers because the original command
// names are unknown.
	//{{AFX_MSG(CBumperDoc)
	afx_msg void OnDbgSetPosition();	// hover.exe:0x426F20 set player position
	afx_msg void OnDbgShadeGreenDec();	// hover.exe:0x427060 green scale -= 0.05
	afx_msg void OnDbgShadeGreenInc();	// hover.exe:0x4270D0 green scale += 0.05
	afx_msg void OnDbgShadeBaseDec();	// hover.exe:0x427140 shade base -= 0.001
	afx_msg void OnDbgShadeBaseInc();	// hover.exe:0x4271B0 shade base += 0.001
	afx_msg void OnDbgShadeBlueDec();	// hover.exe:0x427220 blue scale -= 0.05
	afx_msg void OnDbgShadeBlueInc();	// hover.exe:0x427290 blue scale += 0.05
	afx_msg void OnDbgShadeRedDec();	// hover.exe:0x427300 red scale -= 0.05
	afx_msg void OnDbgShadeRedInc();	// hover.exe:0x427370 red scale += 0.05
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:
// Member variables (hover.exe offsets in comments)
	CMidiStream m_music;	// +0x60
	char m_pad0084[0x100];	// +0x84
	DWORD m_bLevelLoaded;		// +0x184 = 0
	CMerlinWorld* m_pWorld;		// +0x188 = 0
	CShadeTable m_aShade[2];	// +0x18C [0] the level's shade table,
							// [1] the blue-tinted one the view uses while
							// a smoke bomb is out (CBumperView::m_bSmokeBlind)
	CHumanPlayer m_humanPlayer; // +0x818C
	CCollider* m_pCollider;		// +0x8388 = 0
	CGameObjList m_listMazeObjs;	// +0x838C
	CGameObjList m_listActiveObjs;	// +0x83A8
	DWORD m_dwFloorHeight;		// +0x83C4 = 0x40 hover height over the floor
	DWORD m_dwMaxWallHeight;	// +0x83C8 highest edge of any maze face
	DWORD m_dwHeightRange;		// +0x83CC = 0x200, then max - floor: the
								//   altimeter gauge's range
	DWORD m_adwSpawnPoints[5];	// +0x83D0 = {0} texture variant counts for the
							// g_apszSpawnFamilies families ("HUMAN_", "ROBOT_",
							// "FLAG_HUMAN_", "FLAG_ROBOT_", "POD_RANDOM_";
							// counted by CountSpawnPoints)
	DWORD m_dwScore;		// +0x83E4 = 0
	DWORD m_dwRobotFlags;		// +0x83E8
	DWORD m_adwSounds[0x22]; // +0x83EC = {0}
	DWORD m_dwUnk8474;		// +0x8474
	// Shade-table parameters passed to m_aShade[0]: the per-level darkening
	// base and the red/blue/green channel scales (the original's argument
	// order puts blue before green). The ID_DBG_SHADE_BASE_DEC
	// -> ID_DBG_SHADE_BASE_INC debug commands nudge them and rebuild the table.
	double m_dblShadeBase;	// +0x8478 = 0.985 shade base
	double m_dblShadeRed;	// +0x8480 = 1.2 red scale
	double m_dblShadeBlue;	// +0x8488 = 1.2 blue scale
	double m_dblShadeGreen;	// +0x8490 = 1.2 green scale
	DWORD m_bAttractMode;		// +0x8498 = 1
	DWORD m_dwUnk849C;		// +0x849C
};							// sizeof = 0x84A0 in hover.exe

#endif // __BUMPERDOC_H__
