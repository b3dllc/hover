// bumperdoc.cpp : implementation of the CBumperDoc class
//
// Reconstructed from hover.exe; see bumperdoc.h for the RTTI/vtable data.

#include "hover.h"
#include "bumperdoc.h"
#include "texload.h"		// CCollider (RetireObject)
#include "bumpview.h"
#include "texmisc.h"

#include <string.h>
#include <malloc.h>			// _heapmin()

IMPLEMENT_DYNCREATE(CBumperDoc, CDocument)

// Message map at hover.exe:0x4BAC90 (base CDocument), entries at 0x4BAC98.
// Every entry is an ON_COMMAND (nMessage 0x111, AfxSig_vv); none of the
// nine command IDs appears in the menu or the accelerator table, so these
// are leftover debug commands driven from an external tool or a debug
// build's menu. The order below is the original table's order.
BEGIN_MESSAGE_MAP(CBumperDoc, CDocument)
	//{{AFX_MSG_MAP(CBumperDoc)
	ON_COMMAND(ID_DBG_SETPOSITION, OnDbgSetPosition)
	ON_COMMAND(ID_DBG_SHADE_GREEN_DEC, OnDbgShadeGreenDec)
	ON_COMMAND(ID_DBG_SHADE_GREEN_INC, OnDbgShadeGreenInc)
	ON_COMMAND(ID_DBG_SHADE_BASE_DEC, OnDbgShadeBaseDec)
	ON_COMMAND(ID_DBG_SHADE_BASE_INC, OnDbgShadeBaseInc)
	ON_COMMAND(ID_DBG_SHADE_BLUE_DEC, OnDbgShadeBlueDec)
	ON_COMMAND(ID_DBG_SHADE_BLUE_INC, OnDbgShadeBlueInc)
	ON_COMMAND(ID_DBG_SHADE_RED_DEC, OnDbgShadeRedDec)
	ON_COMMAND(ID_DBG_SHADE_RED_INC, OnDbgShadeRedInc)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// hover.exe:0x4C590C
// GLOBAL: HOVER 0x4c590c
LPMIDIDEVICEFN g_pfnMidiStreamClose;

// hover.exe:0x4C591C
// GLOBAL: HOVER 0x4c591c
LPMIDIDEVICEFN g_pfnMidiStreamPause;

// midiStreamRestart, resolved by the MIDI stream loader in texload.cpp
// (declared there too; repeated here so this file does not have to pull in
// the whole renderer header).
extern LPMIDIDEVICEFN g_pfnMidiStreamRestart;	// hover.exe:0x4C5920

// The three texture maps DeleteContents empties along with g_mapDynamicSprites
// (defined in texture.cpp; declared here so this file does not have to
// pull in the whole texture header).
extern CMapStringToOb g_mapUnk_4A1D30;	// hover.exe:0x4A1D30
extern CMapStringToOb g_mapUnk_4A4578;	// hover.exe:0x4A4578
extern CMapStringToOb g_mapUnk_4A4768;	// hover.exe:0x4A4768

// The beacon table Hover_DeleteBeacons empties (defined in mazeobj.cpp).
extern CObArray g_arrBeacons;		// hover.exe:0x4A4840

// hover.exe:0x4A4728
// GLOBAL: HOVER 0x4a4728
CMapStringToOb g_mapDynamicSprites;

// The sound slot names, in the order CBumperDoc::OnNewDocument creates
// them: indices 0x1A-0x1D are the engine loop slots the player uses.
// hover.exe:0x4C4950
// GLOBAL: HOVER 0x4c4950
const char* g_apszSoundNames[34] =
{
	"skid_st",
	"levlstrt",
	"hold_st",
	"hold_en",
	"flagr_st",
	"hit_pod",
	"obt_jump",
	"obt_cloa",
	"obt_wall",
	"invin_st",
	"invin_en",
	"speed_re",
	"speed_en",
	"speed_en",
	"speed_re",
	"hit_mape",
	"obt_hflg",
	"obt_rflg",
	"hit_wall",
	"hit_shld",
	"hit_robt",
	"jump_st",
	"jump_en",
	"cloak_st",
	"cloak_en",
	"shield",
	"eng_idle",
	"eng_low",
	"eng_norm",
	"eng_high",
	"attkping",
	"winlevel",
	"gameovr",
	"flagr_dr",
};

// Deletes every beacon built into the g_arrBeacons table and empties
// the array. Called by CBumperDoc::DeleteContents.
// hover.exe:0x4151E0
// FUNCTION: HOVER 0x4151e0
void Hover_DeleteBeacons()
{
	int i;

	for (i = 0; i < g_arrBeacons.GetSize(); i++)
		delete g_arrBeacons[i];
	g_arrBeacons.RemoveAll();
}

/////////////////////////////////////////////////////////////////////////////
// CMidiStream

// hover.exe:0x425050
// FUNCTION: HOVER 0x425050
CMidiStream::CMidiStream()
{
	m_hFile = INVALID_HANDLE_VALUE;
	m_pFreeHead = 0;
	m_pFreeTail = 0;
	m_hStream = 0;
	m_dwQueued = 0;
	m_bStopping = 0;
}

// hover.exe:0x4250C0
// FUNCTION: HOVER 0x4250c0
CMidiStream::~CMidiStream()
{
	CloseStream();
}

// hover.exe:0x438260
// FUNCTION: HOVER 0x438260
void CMidiStream::CloseStream()
{
	LPMIDIDEVICEFN pfnClose;
	LPMIDIHDR pNode;

	if (m_hStream != 0)
	{
		m_bStopping = 1;
		midiOutReset((HMIDIOUT)m_hStream);
		// unprepares every prepared MIDIHDR on the m_pFreeHead list
		pNode = m_pFreeHead;
		while (pNode != NULL)
		{
			midiOutUnprepareHeader((HMIDIOUT)m_hStream, pNode,
				sizeof(MIDIHDR));
			pNode = pNode->lpNext;
		}
		pfnClose = g_pfnMidiStreamClose;
		if (pfnClose != NULL)
			pfnClose(m_hStream);
	}
	m_hStream = 0;
	while (m_pFreeHead != 0)
	{
		pNode = m_pFreeHead;
		m_pFreeHead = pNode->lpNext;
		GlobalUnlock(GlobalHandle(pNode));
		GlobalFree(GlobalHandle(pNode));
	}
	m_pFreeHead = 0;
	m_pFreeTail = 0;
}

// hover.exe:0x412220
// FUNCTION: HOVER 0x412220
int CMidiStream::PauseMusic()
{
	LPMIDIDEVICEFN pfnClose;

	pfnClose = g_pfnMidiStreamPause;
	if (pfnClose != NULL)
		pfnClose(m_hStream);
	return 1;
}

// Restart playback of the stream this object has open. With no open device
// the whole open sequence is run again (StartStream); the original tail-jumps
// into it.

// hover.exe:0x413710
// FUNCTION: HOVER 0x413710
int CMidiStream::ResumeMusic()
{
	LPMIDIDEVICEFN pfnRestart;

	if (m_hStream != 0)
	{
		pfnRestart = g_pfnMidiStreamRestart;
		if (pfnRestart != NULL)
			pfnRestart(m_hStream);
		return 1;
	}
	return StartStream();
}

/////////////////////////////////////////////////////////////////////////////
// CGameObjList

// hover.exe:0x41E1E0
// FUNCTION: HOVER 0x41e1e0
CGameObjList::~CGameObjList()
{
	// trivial destructor in the original
}

/////////////////////////////////////////////////////////////////////////////
// CSetPositionDialog: the debug "set player position" dialog opened by
// CBumperDoc::OnDbgSetPosition (ID_DBG_SETPOSITION). Dialog template 133,
// three edit controls holding the player's X (0..23000), Y (0..23000) and
// heading (0..511). It has no DECLARE_DYNAMIC (vtable slot 0 is the inherited
// CDialog::GetRuntimeClass) and an empty message map (AFX_MSGMAP at
// hover.exe:0x4BD4A0 with a single end marker), so the class name is unknown.
// vtable at hover.exe:0x4BD4E8, sizeof = 0x50.
//
// The dialog template was stripped from the shipped hover.exe, so
// DoModal() returns -1 at runtime and the handler never applies anything.
//
// The class is declared here rather than in bumperdoc.h because
// CBumperDoc::OnDbgSetPosition is its only user and bumperdoc.h is included by
// half the project (adding a class to it reshuffles register allocation in
// every translation unit -- see notes/perfect-match.md rules 14 and 18).

class CSetPositionDialog : public CDialog
{
// Construction
public:
	CSetPositionDialog(CWnd* pParent = NULL);	// hover.exe:0x42CFE0

// Dialog Data
	//{{AFX_DATA(CSetPositionDialog)
	enum { IDD = IDD_DEBUG_SETPOSITION };
	int m_nX;				// +0x44 = 0
	int m_nY;				// +0x48 = 0
	int m_nAngle;			// +0x4C = 0
	//}}AFX_DATA

// Implementation
protected:
	// hover.exe:0x42D050; note the original does NOT chain to
	// CDialog::DoDataExchange
	virtual void DoDataExchange(CDataExchange* pDX);

	DECLARE_MESSAGE_MAP()	// empty map at hover.exe:0x4BD4A0
};							// sizeof = 0x50 in hover.exe

BEGIN_MESSAGE_MAP(CSetPositionDialog, CDialog)
	//{{AFX_MSG_MAP(CSetPositionDialog)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// hover.exe:0x42CFE0
// FUNCTION: HOVER 0x42cfe0
CSetPositionDialog::CSetPositionDialog(CWnd* pParent) : CDialog(IDD_DEBUG_SETPOSITION, pParent)
{
	m_nX = 0;
	m_nY = 0;
	m_nAngle = 0;
}

// hover.exe:0x42D050
// FUNCTION: HOVER 0x42d050
void CSetPositionDialog::DoDataExchange(CDataExchange* pDX)
{
	// NOTE: unlike every other dialog in hover.exe this one does not call
	// CDialog::DoDataExchange first.
	//{{AFX_DATA_MAP(CSetPositionDialog)
	DDX_Text(pDX, IDC_DEBUG_X, m_nX);
	DDV_MinMaxInt(pDX, m_nX, 0, 23000);
	DDX_Text(pDX, IDC_DEBUG_Y, m_nY);
	DDV_MinMaxInt(pDX, m_nY, 0, 23000);
	DDX_Text(pDX, IDC_DEBUG_ANGLE, m_nAngle);
	DDV_MinMaxInt(pDX, m_nAngle, 0, 511);
	//}}AFX_DATA_MAP
}

/////////////////////////////////////////////////////////////////////////////
// CBumperDoc

// hover.exe:0x41E230
// FUNCTION: HOVER 0x41e230
CBumperDoc::CBumperDoc()
{
	theApp.m_pBumperDoc = this;
	m_pWorld = 0;
	m_pCollider = 0;
	m_dwScore = 0;
	memset(m_adwSounds, 0, sizeof(m_adwSounds));
	m_dblShadeBase = 0.985;
	m_dblShadeRed = 1.2;
	m_dblShadeBlue = 1.2;
	m_dblShadeGreen = 1.2;
	m_bLevelLoaded = 0;
	m_dwHeightRange = 0x200;
	m_bAttractMode = 1;
}

// Deletes the 34 sound slots created by OnNewDocument. Everything else
// CBumperDoc owns -- the MIDI player at +0x60, the human player, the two
// object lists and the CDocument base -- is destroyed by the compiler
// generated member/base destructor calls that follow this body inside the
// scalar deleting destructor at hover.exe:0x41E0D0.

// hover.exe:0x41E0D0
// The destructor is defined in the class declaration, so the compiler folds
// it into the scalar deleting destructor that the vtable slot holds - which
// is what hover.exe:0x41E0D0 is. The block below emits no code; it only gives
// the reccmp marker a declaration to name ??_GCBumperDoc@@ (rules 34/184).
// hover.exe:0x41E0D0
#if 0
// FUNCTION: HOVER 0x41e0d0
CBumperDoc::~CBumperDoc()
{
	int i;

	for (i = 0; i < 34; i++)
	{
		delete (CObject*)m_adwSounds[i];
		m_adwSounds[i] = 0;
	}
}
#endif

// The document has no serialized state of its own: the level is loaded
// from the .maz/.tex files by OnOpenDocument, so the override is empty. It
// exists only to keep CObject::Serialize (which would throw a
// CNotSupportedException) out of the vtable slot.

// hover.exe:0x426F10
// FUNCTION: HOVER 0x426f10
void CBumperDoc::Serialize(CArchive& ar)
{
}

// Tears the current level down: the human player's object lists, then
// every object in the two document lists (the embedded human player is in
// the second list and must not be deleted), the collider and the world
// manager, the four texture name maps, the beacon table and the two
// CGameObj instance list heads. CDocument::DeleteContents then does the
// framework's own cleanup and _heapmin gives the freed level memory back
// to the operating system.

// hover.exe:0x414900
// FUNCTION: HOVER 0x414900
void CBumperDoc::DeleteContents()
{
	POSITION pos;
	CObject* pObj;

	m_humanPlayer.ClearPods(0);

	pos = m_listMazeObjs.GetHeadPosition();
	while (pos != NULL)
	{
		pObj = m_listMazeObjs.GetNext(pos);
		delete pObj;
	}
	m_listMazeObjs.RemoveAll();

	pos = m_listActiveObjs.GetHeadPosition();
	while (pos != NULL)
	{
		pObj = m_listActiveObjs.GetNext(pos);
		if (pObj != (CObject*)&m_humanPlayer)
			delete pObj;
	}
	m_listActiveObjs.RemoveAll();

	delete (CObject*)m_pCollider;
	m_pCollider = 0;
	delete (CObject*)m_pWorld;
	m_pWorld = 0;

	g_mapDynamicSprites.RemoveAll();
	g_mapUnk_4A4578.RemoveAll();
	g_mapUnk_4A1D30.RemoveAll();
	g_mapUnk_4A4768.RemoveAll();
	Hover_DeleteBeacons();
	g_pDirtyObjList = 0;
	g_pDrawObjList = 0;
	CDocument::DeleteContents();
	_heapmin();
}

// Runs on ID_FILE_NEW, i.e. on the Start Game command (CWinApp::OnFileNew
// -> CDocTemplate::OpenDocumentFile -> OnNewDocument). The 34 sound slots
// are created once, on the first new document; the level itself is then
// loaded by the OnOpenDocument virtual. The m_bAttractMode branch only changes
// the progress-dialog step budget (g_dwLevel) around the load.

// hover.exe:0x417B00
// FUNCTION: HOVER 0x417b00
BOOL CBumperDoc::OnNewDocument()
{
	int i;
	DWORD dwSaved;

	if (!CDocument::OnNewDocument())
		return FALSE;

	if (m_adwSounds[0] == 0)
	{
		for (i = 0; i < 34; i++)
			m_adwSounds[i] = (DWORD)new CSoundSlot(g_apszSoundNames[i], 0);
	}

	if (m_bAttractMode != 0)
	{
		dwSaved = g_dwLevel;
		g_dwLevel = 20;
		OnOpenDocument(NULL);
		g_dwLevel = dwSaved;
	}
	else
	{
		OnOpenDocument(NULL);
	}
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// Wave slot objects
//
// hover.exe:0x4C4950 the 34 wave file base names of m_adwSounds (indices
// 12/13 and 11/14 deliberately share "speed_en"/"speed_re").

// Creates the 34 wave slot objects; a slot whose allocation fails stays
// NULL. Called after the sound engine has been (re)started.

// hover.exe:0x426E90
// FUNCTION: HOVER 0x426e90
void CBumperDoc::CreateSounds()
{
	const char** ppszName;
	DWORD* pSlot;

	ppszName = g_apszSoundNames;
	pSlot = m_adwSounds;
	do
	{
		*pSlot = (DWORD)new CSoundSlot(*ppszName, 0);
		ppszName++;
		pSlot++;
	}
	while (ppszName < g_apszSoundNames + 34);
}

// Destroys the 34 wave slot objects and clears the slots.

// hover.exe:0x426EE0
// FUNCTION: HOVER 0x426ee0
void CBumperDoc::DestroySounds()
{
	DWORD* pSlot;
	int i;

	pSlot = m_adwSounds;
	for (i = 0x22; i != 0; i--)
	{
		if (*pSlot != 0)
			delete (CSoundSlot*)*pSlot;
		*pSlot = 0;
		pSlot++;
	}
}

/////////////////////////////////////////////////////////////////////////////
// CBumperDoc message handlers
//
// All nine are unlisted debug commands (no menu item, no accelerator).
// The eight shade commands below nudge one of the four shade-table
// parameters and immediately rebuild the primary shade table m_aShade[0]
// from the level palette, so the effect is visible on the next
// frame; the secondary m_aShade[1] (built with the fixed 0.8/1.5/0.8
// scales) is left alone.

// Opens the debug position dialog on the human player: the current
// position and heading go in, and on IDOK the player is teleported there
// with its velocity, spin and bounce state cleared, and every object in
// the two document lists has its +0x3C field reset. The game is flagged as
// suspended (theApp.m_bSuspended) while the dialog is up. The dialog
// template (133) is not in the shipped resources, so DoModal() fails and
// nothing is applied.
// hover.exe:0x426F20
// FUNCTION: HOVER 0x426f20
void CBumperDoc::OnDbgSetPosition()
{
	CSetPositionDialog dlg(NULL);
	DWORD adwPos1[2];
	DWORD adwPos2[2];
	POSITION pos;
	// Frame placeholder, not a variable of the original (rule 80): the
	// original's frame holds six untouched dword slots here that our
	// expressions do not generate (see notes/perfect-match.md).
	volatile DWORD adwPad[6];

	adwPad[0] = 0;

	// GetCellPos returns the position pair by value in the original, so
	// each component read compiles into its own call and its own temporary
	// (hover.exe uses [ebp-0x30] and [ebp-0x38] for them, exactly as we do).
	// NOTE: the original frame is 0x18 bytes larger than ours -- it leaves
	// six dword slots at [ebp-0x28]..[ebp-0x14] completely untouched. No
	// spelling of this function reproduces that (VC 2.2 drops locals it
	// never materializes), so every ebp-relative displacement here is
	// shifted by 0x18 against the original; see notes/perfect-match.md.
	dlg.m_nX = m_humanPlayer.GetCellPos(adwPos1)[0];
	dlg.m_nY = m_humanPlayer.GetCellPos(adwPos2)[1];
	dlg.m_nAngle = (short)m_humanPlayer.m_wHeading;
	theApp.m_bSuspended = 1;
	if (dlg.DoModal() == IDOK)
	{
		m_humanPlayer.SetCellPos(dlg.m_nX, dlg.m_nY);
		m_humanPlayer.m_wHeadingFrac = 0;
		m_humanPlayer.m_wHeading = (WORD)dlg.m_nAngle;
		m_humanPlayer.m_wVelX = 0;
		m_humanPlayer.m_wVelXFrac = 0;
		m_humanPlayer.m_wVelY = 0;
		m_humanPlayer.m_wVelYFrac = 0;
		pos = m_listMazeObjs.GetHeadPosition();
		while (pos != NULL)
			((CGameObj*)m_listMazeObjs.GetNext(pos))->m_bOnMap = 0;
		pos = m_listActiveObjs.GetHeadPosition();
		while (pos != NULL)
			((CGameObj*)m_listActiveObjs.GetNext(pos))->m_bOnMap = 0;
	}
	theApp.m_bSuspended = 0;
}

// Darkens the green channel of the shade table by 0.05.
// hover.exe:0x427060
// FUNCTION: HOVER 0x427060
void CBumperDoc::OnDbgShadeGreenDec()
{
	m_dblShadeGreen -= 0.05;
	m_aShade[0].BuildTable(&g_palGame, m_dblShadeBase,
		0xFF, 0xFF, 0xFF, m_dblShadeRed, m_dblShadeBlue, m_dblShadeGreen);
}

// Brightens the green channel of the shade table by 0.05.
// hover.exe:0x4270D0
// FUNCTION: HOVER 0x4270d0
void CBumperDoc::OnDbgShadeGreenInc()
{
	m_dblShadeGreen += 0.05;
	m_aShade[0].BuildTable(&g_palGame, m_dblShadeBase,
		0xFF, 0xFF, 0xFF, m_dblShadeRed, m_dblShadeBlue, m_dblShadeGreen);
}

// Lowers the per-level darkening base by 0.001.
// hover.exe:0x427140
// FUNCTION: HOVER 0x427140
void CBumperDoc::OnDbgShadeBaseDec()
{
	m_dblShadeBase -= 0.001;
	m_aShade[0].BuildTable(&g_palGame, m_dblShadeBase,
		0xFF, 0xFF, 0xFF, m_dblShadeRed, m_dblShadeBlue, m_dblShadeGreen);
}

// Raises the per-level darkening base by 0.001.
// hover.exe:0x4271B0
// FUNCTION: HOVER 0x4271b0
void CBumperDoc::OnDbgShadeBaseInc()
{
	m_dblShadeBase += 0.001;
	m_aShade[0].BuildTable(&g_palGame, m_dblShadeBase,
		0xFF, 0xFF, 0xFF, m_dblShadeRed, m_dblShadeBlue, m_dblShadeGreen);
}

// Darkens the blue channel of the shade table by 0.05.
// hover.exe:0x427220
// FUNCTION: HOVER 0x427220
void CBumperDoc::OnDbgShadeBlueDec()
{
	m_dblShadeBlue -= 0.05;
	m_aShade[0].BuildTable(&g_palGame, m_dblShadeBase,
		0xFF, 0xFF, 0xFF, m_dblShadeRed, m_dblShadeBlue, m_dblShadeGreen);
}

// Brightens the blue channel of the shade table by 0.05.
// hover.exe:0x427290
// FUNCTION: HOVER 0x427290
void CBumperDoc::OnDbgShadeBlueInc()
{
	m_dblShadeBlue += 0.05;
	m_aShade[0].BuildTable(&g_palGame, m_dblShadeBase,
		0xFF, 0xFF, 0xFF, m_dblShadeRed, m_dblShadeBlue, m_dblShadeGreen);
}

// Darkens the red channel of the shade table by 0.05.
// hover.exe:0x427300
// FUNCTION: HOVER 0x427300
void CBumperDoc::OnDbgShadeRedDec()
{
	m_dblShadeRed -= 0.05;
	m_aShade[0].BuildTable(&g_palGame, m_dblShadeBase,
		0xFF, 0xFF, 0xFF, m_dblShadeRed, m_dblShadeBlue, m_dblShadeGreen);
}

// Brightens the red channel of the shade table by 0.05.
// hover.exe:0x427370
// FUNCTION: HOVER 0x427370
void CBumperDoc::OnDbgShadeRedInc()
{
	m_dblShadeRed += 0.05;
	m_aShade[0].BuildTable(&g_palGame, m_dblShadeBase,
		0xFF, 0xFF, 0xFF, m_dblShadeRed, m_dblShadeBlue, m_dblShadeGreen);
}

// Retires pObj: unregisters it from the collider and relinks it onto the
// pending-delete list through its vtable slot 17 virtual.

// hover.exe:0x412630
// FUNCTION: HOVER 0x412630
void CBumperDoc::RetireObject(CGameObj* pObj)
{
	(m_pCollider)->RemoveObject(pObj);
	pObj->KillObject();
}

// Removes pObj from the pending list, or failing that from the active-pod
// list. Called by CGameObj::KillObject when an object retires itself.

// hover.exe:0x412650
// FUNCTION: HOVER 0x412650
BOOL CBumperDoc::RemoveObject(CObject* pObj)
{
	CGameObjList* pList;
	POSITION pos;

	pList = &m_listActiveObjs;
	pos = pList->Find(pObj, NULL);
	if (pos != NULL)
		pList->RemoveAt(pos);
	if (pos != NULL)
		return TRUE;

	pList = &m_listMazeObjs;
	pos = pList->Find(pObj, NULL);
	if (pos != NULL)
		pList->RemoveAt(pos);
	if (pos != NULL)
		return TRUE;

	return FALSE;
}
