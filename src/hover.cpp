// hover.cpp : Defines the class behaviors for the Hover! application.
//

#include "hover.h"
#include "mainfrm.h"
#include "bumperdoc.h"
#include "bumpview.h"
#include "splashdlg.h"
#include "highscrdlg.h"
#include "optionsdlg.h"
#include "texload.h"
#include "texture.h"		// the texture load progress callback hooks
#include "texmisc.h"		// Hover_sub_4262B0 (exception reporter)

#include <shellapi.h>

#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <ctype.h>

/////////////////////////////////////////////////////////////////////////////
// Reconstructed game globals

// hover.exe:0x4C4000
// GLOBAL: HOVER 0x4c4000
LVL_PARAMS g_lvlParams[21] =
{
	{ "Sounds\\Music\\MUSIC1.muz", "mazes\\MAZE1.maz", "mazes\\TEXT1.tex",
		{ 4, 4, 4, 4, 4, 4, 4 }, 1, 1, 3 },
	{ "Sounds\\Music\\MUSIC2.muz", "mazes\\MAZE2.maz", "mazes\\TEXT2.tex",
		{ 4, 4, 4, 4, 4, 4, 4 }, 1, 1, 3 },
	{ "Sounds\\Music\\MUSIC3.muz", "mazes\\MAZE3.maz", "mazes\\TEXT3.tex",
		{ 4, 4, 4, 4, 4, 4, 4 }, 1, 1, 3 },
	{ "Sounds\\Music\\MUSIC1.muz", "mazes\\MAZE1.maz", "mazes\\TEXT1.tex",
		{ 4, 4, 4, 4, 4, 4, 4 }, 2, 1, 4 },
	{ "Sounds\\Music\\MUSIC2.muz", "mazes\\MAZE2.maz", "mazes\\TEXT2.tex",
		{ 4, 4, 4, 4, 4, 4, 4 }, 2, 2, 4 },
	{ "Sounds\\Music\\MUSIC3.muz", "mazes\\MAZE3.maz", "mazes\\TEXT3.tex",
		{ 4, 4, 4, 4, 4, 4, 4 }, 2, 2, 5 },
	{ "Sounds\\Music\\MUSIC1.muz", "mazes\\MAZE1.maz", "mazes\\TEXT1.tex",
		{ 4, 4, 4, 4, 4, 4, 4 }, 3, 2, 5 },
	{ "Sounds\\Music\\MUSIC2.muz", "mazes\\MAZE2.maz", "mazes\\TEXT2.tex",
		{ 4, 4, 4, 4, 4, 4, 4 }, 3, 2, 6 },
	{ "Sounds\\Music\\MUSIC3.muz", "mazes\\MAZE3.maz", "mazes\\TEXT3.tex",
		{ 4, 4, 4, 4, 4, 4, 4 }, 3, 3, 6 },
	{ "Sounds\\Music\\MUSIC1.muz", "mazes\\MAZE1.maz", "mazes\\TEXT1.tex",
		{ 4, 4, 4, 4, 4, 4, 4 }, 4, 3, 6 },
	{ "Sounds\\Music\\MUSIC2.muz", "mazes\\MAZE2.maz", "mazes\\TEXT2.tex",
		{ 4, 4, 4, 4, 4, 4, 4 }, 4, 3, 6 },
	{ "Sounds\\Music\\MUSIC3.muz", "mazes\\MAZE3.maz", "mazes\\TEXT3.tex",
		{ 4, 4, 4, 4, 4, 4, 4 }, 4, 3, 6 },
	{ "Sounds\\Music\\MUSIC1.muz", "mazes\\MAZE1.maz", "mazes\\TEXT1.tex",
		{ 4, 4, 4, 4, 4, 4, 4 }, 5, 4, 6 },
	{ "Sounds\\Music\\MUSIC2.muz", "mazes\\MAZE2.maz", "mazes\\TEXT2.tex",
		{ 4, 4, 4, 4, 4, 4, 4 }, 5, 4, 6 },
	{ "Sounds\\Music\\MUSIC3.muz", "mazes\\MAZE3.maz", "mazes\\TEXT3.tex",
		{ 4, 4, 4, 4, 4, 4, 4 }, 5, 4, 6 },
	{ "Sounds\\Music\\MUSIC1.muz", "mazes\\MAZE1.maz", "mazes\\TEXT1.tex",
		{ 4, 4, 4, 4, 4, 4, 4 }, 6, 4, 6 },
	{ "Sounds\\Music\\MUSIC2.muz", "mazes\\MAZE2.maz", "mazes\\TEXT2.tex",
		{ 4, 4, 4, 4, 4, 4, 4 }, 6, 4, 6 },
	{ "Sounds\\Music\\MUSIC3.muz", "mazes\\MAZE3.maz", "mazes\\TEXT3.tex",
		{ 4, 4, 4, 4, 4, 4, 4 }, 6, 4, 6 },
	{ "Sounds\\Music\\MUSIC1.muz", "mazes\\MAZE1.maz", "mazes\\TEXT1.tex",
		{ 4, 4, 4, 4, 4, 4, 4 }, 6, 4, 6 },
	{ "Sounds\\Music\\MUSIC2.muz", "mazes\\MAZE2.maz", "mazes\\TEXT2.tex",
		{ 4, 4, 4, 4, 4, 4, 4 }, 6, 4, 6 },
	// Entry 20 is the silent attract-mode maze the document loads while
	// no game is running (CBumperDoc::OnNewDocument forces
	// g_dwUnk_46049C to 20 and the loader indexes g_lvlParams directly,
	// bypassing the LVLPARAM wrap).
	{ NULL, "mazes\\small.maz", "mazes\\small.tex",
		{ 0, 0, 0, 0, 0, 0, 0 }, 0, 0, 0 },
};

// Localized message strings loaded by CHoverApp::InitInstance
// GLOBAL: HOVER 0x4604d0
char g_szMemoryError[0x80];		// hover.exe:0x4604D0
// GLOBAL: HOVER 0x460550
CString g_strReadFileError;		// hover.exe:0x460550
// GLOBAL: HOVER 0x4604b0
CString g_strUnexpectedException;	// hover.exe:0x4604B0
// GLOBAL: HOVER 0x460560
CString g_strExcNotSupported;	// hover.exe:0x460560
// GLOBAL: HOVER 0x460020
CString g_strExcArchiveNone;	// hover.exe:0x460020
// GLOBAL: HOVER 0x460030
CString g_strExcArchiveGeneric;	// hover.exe:0x460030
// GLOBAL: HOVER 0x460040
CString g_strExcArchiveReadOnly;	// hover.exe:0x460040
// GLOBAL: HOVER 0x460050
CString g_strExcArchiveEndOfFile;	// hover.exe:0x460050
// GLOBAL: HOVER 0x460460
CString g_strExcArchiveWriteOnly;	// hover.exe:0x460460
// GLOBAL: HOVER 0x460470
CString g_strExcArchiveBadIndex;	// hover.exe:0x460470
// GLOBAL: HOVER 0x460480
CString g_strExcArchiveBadClass;	// hover.exe:0x460480
// GLOBAL: HOVER 0x460490
CString g_strExcArchiveBadSchema;	// hover.exe:0x460490
// GLOBAL: HOVER 0x4604c0
CString g_strExcResource;		// hover.exe:0x4604C0
// GLOBAL: HOVER 0x460010
CString g_strExcOle;			// hover.exe:0x460010
// GLOBAL: HOVER 0x4604a0
CString g_strExcDao;			// hover.exe:0x4604A0
// GLOBAL: HOVER 0x460000
CString g_strExcUser;			// hover.exe:0x460000

// GLOBAL: HOVER 0x46002c
DWORD g_cbUnk_46002C;			// hover.exe:0x46002C
// GLOBAL: HOVER 0x46003c
void* g_pUnk_46003C;			// hover.exe:0x46003C
// GLOBAL: HOVER 0x461cfc
DWORD g_dwUnk_461CFC;			// hover.exe:0x461CFC
// GLOBAL: HOVER 0x4c56d8
DWORD g_dwUnk_4C56D8;			// hover.exe:0x4C56D8
// GLOBAL: HOVER 0x4c4ce8
FARPROC g_pfnJoyGetPosEx;		// hover.exe:0x4C4CE8
// GLOBAL: HOVER 0x4c4738
CObject* g_pUnk_4C4738;			// hover.exe:0x4C4738
// GLOBAL: HOVER 0x46048c
DWORD g_dwUnk_46048C;			// hover.exe:0x46048C
// GLOBAL: HOVER 0x46049c
DWORD g_dwUnk_46049C;			// hover.exe:0x46049C
// GLOBAL: HOVER 0x4a4878
DWORD g_dwUnk_4A4878;			// hover.exe:0x4A4878
// GLOBAL: HOVER 0x4a487c
DWORD g_dwUnk_4A487C;			// hover.exe:0x4A487C

// Per-skill-level parameter blocks (the game's skill tuning), overwritten
// from the registry by Hover_sub_4249C0/Hover_sub_424A70/Hover_sub_424B20
// (H = human player, R = robot opponents, RobotAI = robot AI).
// CHoverApp::Sub_425870 points m_pUnk014C/m_pUnk0150/m_pUnk0154 at the set
// for the current skill level.
//
// The defaults below are the values the compiler-generated static
// initializer at hover.exe:0x433370 installs. They are written in the
// units the game tunes in and stored as 16.16 fixed point: the beginner
// human block through the inline 16.16 constructor (the compiler folds its
// eleven stores into the initializer), the other five player blocks
// through the out-of-line double constructor at hover.exe:0x425290 (five
// calls with eleven doubles pushed each), and the three robot-AI blocks
// through CRobotAIParams' inline constructor. The declaration order below
// is the initializer's construction order.

// Converts a tuning value to the 16.16 fixed point the blocks hold (the
// out-of-line constructor does the same at run time, via _ftol).
#define FIX16(dbl)	((long)((dbl) * 65536.0))

// hover.exe:0x4A48F8 HBegParams
// GLOBAL: HOVER 0x4a48f8
CPlayerParams g_HBegParams(FIX16(1.0), FIX16(-0.05), FIX16(50.0), FIX16(0.0),
	FIX16(0.5), FIX16(0.2), FIX16(0.2), FIX16(0.66), FIX16(1.33), FIX16(0.1),
	FIX16(5.0));
// hover.exe:0x4A49B8 HIntParams
// GLOBAL: HOVER 0x4a49b8
CPlayerParams g_HIntParams(3.0, -0.01, 65.0, 0.0, 0.5, 0.3, 0.3, 0.5, 1.33,
	0.025, 7.0);
// hover.exe:0x4A4958 HExpParams
// GLOBAL: HOVER 0x4a4958
CPlayerParams g_HExpParams(5.0, -0.001, 75.0, 0.0, 0.5, 0.5, 0.4, 0.4, 1.33,
	0.01, 15.0);
// hover.exe:0x4A4928 RBegParams
// GLOBAL: HOVER 0x4a4928
CPlayerParams g_RBegParams(1.0, -0.01, 40.0, 20.0, 0.8, 0.8, 0.3, 0.66, 1.33,
	0.025, 5.0);
// hover.exe:0x4A49E8 RIntParams
// GLOBAL: HOVER 0x4a49e8
CPlayerParams g_RIntParams(1.0, -0.01, 50.0, 20.0, 0.8, 0.8, 0.3, 0.66, 1.33,
	0.025, 5.0);
// hover.exe:0x4A4988 RExpParams
// GLOBAL: HOVER 0x4a4988
CPlayerParams g_RExpParams(1.0, -0.01, 60.0, 20.0, 0.8, 0.8, 0.3, 0.66, 1.33,
	0.025, 5.0);
// hover.exe:0x4A4880 RobotAIBegParams
// GLOBAL: HOVER 0x4a4880
CRobotAIParams g_RobotAIBegParams(1.5, 60, 0.4, 30, 15, 5000, 5000, 5000,
	100, 3);
// hover.exe:0x4A48D0 RobotAIIntParams
// GLOBAL: HOVER 0x4a48d0
CRobotAIParams g_RobotAIIntParams(1.5, 60, 0.4, 30, 15, 7000, 5000, 5000,
	100, 4);
// hover.exe:0x4A48A8 RobotAIExpParams
// GLOBAL: HOVER 0x4a48a8
CRobotAIParams g_RobotAIExpParams(1.5, 60, 0.4, 30, 15, 10000, 5000, 5000,
	100, 4);

// The modeless "Please Wait" dialog while a level loads (put up and torn
// down by CHoverApp::Sub_41C8E0); Hover_sub_40EB50 paints the progress bar
// into it.
// GLOBAL: HOVER 0x4c57d4
CSub424720* g_pLoadDlg_4C57D4;	// hover.exe:0x4C57D4

// Pointer lists emptied by the subsystem shutdown helpers called from
// CHoverApp::ExitInstance (element types not yet known)
// GLOBAL: HOVER 0x4a45b8
CPtrList g_listUnk_4A45B8;		// hover.exe:0x4A45B8
// GLOBAL: HOVER 0x4a45e0
CPtrList g_listUnk_4A45E0;		// hover.exe:0x4A45E0
// GLOBAL: HOVER 0x4a46a0
CPtrList g_listUnk_4A46A0;		// hover.exe:0x4A46A0
// GLOBAL: HOVER 0x4a46d0
CPtrList g_listUnk_4A46D0;		// hover.exe:0x4A46D0
// GLOBAL: HOVER 0x4a46f8
CPtrList g_listUnk_4A46F8;		// hover.exe:0x4A46F8
// GLOBAL: HOVER 0x4a4748
CPtrList g_listUnk_4A4748;		// hover.exe:0x4A4748
// GLOBAL: HOVER 0x4a4858
CPtrList g_listUnk_4A4858;		// hover.exe:0x4A4858

// GLOBAL: HOVER 0x4c5330
DWORD g_dwUnk_4C5330;			// hover.exe:0x4C5330
// GLOBAL: HOVER 0x4c53e0
DWORD g_dwUnk_4C53E0;			// hover.exe:0x4C53E0
// GLOBAL: HOVER 0x4c54d0
DWORD g_dwUnk_4C54D0;			// hover.exe:0x4C54D0
// GLOBAL: HOVER 0x4c5588
DWORD g_dwUnk_4C5588;			// hover.exe:0x4C5588
// GLOBAL: HOVER 0x4c55d0
DWORD g_dwUnk_4C55D0;			// hover.exe:0x4C55D0
// GLOBAL: HOVER 0x4c55d4
DWORD g_dwUnk_4C55D4 = 0x40;		// hover.exe:0x4C55D4 REGIONNODE chunk size
// GLOBAL: HOVER 0x4c5620
DWORD g_dwUnk_4C5620;			// hover.exe:0x4C5620
// GLOBAL: HOVER 0x4c57a8
DWORD g_dwUnk_4C57A8;			// hover.exe:0x4C57A8

// Game settings read/written as the 5-byte "GameSettings" registry value
// by Hover_sub_42D180/Hover_sub_42D290; the bytes are sign-extended into
// these DWORDs (theApp.m_dwUnk01C4 lives in bumpview.cpp)

// GLOBAL: HOVER 0x4c4ce0
DWORD g_dwUnk_4C4CE0;			// hover.exe:0x4C4CE0

// GLOBAL: HOVER 0x4c4ce4
DWORD g_dwUnk_4C4CE4;			// hover.exe:0x4C4CE4

// Worker-thread shutdown state used by Hover_sub_422E30
// (the sound slot registry at 0x4A47F0 is a CObArray; its m_pData/m_nSize
// members at 0x4A47F4/0x4A47F8 were previously modeled as loose globals
// g_apUnk_4A47F4/g_nUnk_4A47F8 — see g_arrUnk_4A47F0 in gameobj.h)
// GLOBAL: HOVER 0x4a4808
CRITICAL_SECTION g_csUnk_4A4808;	// hover.exe:0x4A4808
// GLOBAL: HOVER 0x4c56dc
SOUNDENGINE* g_pUnk_4C56DC;		// hover.exe:0x4C56DC sound driver engine object
// GLOBAL: HOVER 0x4c56e0
CWinThread* g_pThreadUnk_4C56E0;	// hover.exe:0x4C56E0 sound mixing thread
// GLOBAL: HOVER 0x4c56e4
DWORD g_dwUnk_4C56E4;			// hover.exe:0x4C56E4 g_csUnk_4A4808 initialized flag
// GLOBAL: HOVER 0x4c56e8
DWORD g_dwUnk_4C56E8;			// hover.exe:0x4C56E8
// GLOBAL: HOVER 0x4c56f4
HANDLE g_hUnk_4C56F4;			// hover.exe:0x4C56F4
// GLOBAL: HOVER 0x4c56f8
HANDLE g_hUnk_4C56F8;			// hover.exe:0x4C56F8
// GLOBAL: HOVER 0x4c56fc
DWORD g_dwUnk_4C56FC;			// hover.exe:0x4C56FC "wave devices in use"
							// message box already shown
// GLOBAL: HOVER 0x4c5ad4
DWORD g_dwUnk_4C5AD4;			// hover.exe:0x4C5AD4 WaveMixPlay reentry count

// WaveMix engine state (Hover_sub_42D6A0/Hover_sub_43D880/Hover_sub_4393A0):
// the queued sound objects have the sound data pointer at offset +4
// GLOBAL: HOVER 0x4c56ec
void* g_pvUnk_4C56EC;			// hover.exe:0x4C56EC queued one-shot sound
// GLOBAL: HOVER 0x4c56f0
void* g_pvUnk_4C56F0;			// hover.exe:0x4C56F0 queued looping sound
// GLOBAL: HOVER 0x4b6248
SOUNDENGINE* g_pUnk_4B6248;		// hover.exe:0x4B6248 current session
// GLOBAL: HOVER 0x4b6f14
DWORD g_dwUnk_4B6F14;			// hover.exe:0x4B6F14
// GLOBAL: HOVER 0x4b6f18
SOUNDENGINE* g_pUnk_4B6F18;		// hover.exe:0x4B6F18 active session (WaveMixActivate)
// GLOBAL: HOVER 0x4b6d50
void* g_pUnk_4B6D50;			// hover.exe:0x4B6D50 free channel-node list
// GLOBAL: HOVER 0x4b6e70
DWORD g_adwUnk_4B6E70[0x14];	// hover.exe:0x4B6E70 wave block pointer array
// GLOBAL: HOVER 0x4b6ec0
DWORD g_adwUnk_4B6EC0[0x14];	// hover.exe:0x4B6EC0 wave block pointer array 2
// GLOBAL: HOVER 0x4b6f10
HINSTANCE g_hUnk_4B6F10;		// hover.exe:0x4B6F10 module instance
// GLOBAL: HOVER 0x4b61c8
void* g_apUnk_4B61C8[0x10];		// hover.exe:0x4B61C8 channel node table
// GLOBAL: HOVER 0x4b6208
void* g_apUnk_4B6208[0x10];		// hover.exe:0x4B6208 mix source pointers
// GLOBAL: HOVER 0x4b6f20
WORD g_awUnk_4B6F20[0x20];		// hover.exe:0x4B6F20 mix volume pairs
// GLOBAL: HOVER 0x4b4a58
DWORD g_adwUnk_4B4A58[100 * (0x3C / 4)];	// hover.exe:0x4B4A58 pool of
						// 100 60-byte channel nodes, linked into
						// the g_pUnk_4B6D50 free list by
						// Hover_sub_438CA0
// GLOBAL: HOVER 0x4b6250
BYTE g_aUnk_4B6250[11][256];	// hover.exe:0x4B6250 sample-volume table
						// (built by Hover_sub_43DBE0, indexed
						// [volume][sample])
// GLOBAL: HOVER 0x4b6d58
char g_szUnk_4B6D58[0x100];		// hover.exe:0x4B6D58 ini filename buffer
					// (filled at runtime; size unknown)
// GLOBAL: HOVER 0x4c59b8
DWORD g_dwUnk_4C59B8;			// hover.exe:0x4C59B8 show debug dialogs
// GLOBAL: HOVER 0x4c59bc
DWORD g_dwUnk_4C59BC;			// hover.exe:0x4C59BC debug output verbosity
// GLOBAL: HOVER 0x4c59c0
void* g_pUnk_4C59C0;			// hover.exe:0x4C59C0 done-block queue head
// GLOBAL: HOVER 0x4c59c4
void* g_pUnk_4C59C4;			// hover.exe:0x4C59C4 done-block queue tail
// GLOBAL: HOVER 0x4c59c8
DWORD g_dwUnk_4C59C8;			// hover.exe:0x4C59C8
// GLOBAL: HOVER 0x4c59d0
char g_szUnk_4C59D0[0x100] =		// hover.exe:0x4C59D0 version banner,
	"WaveMix V 2.3 by Angel M. Diaz, Jr. (c) Microsoft 1993-1995";
					// reused as the wsprintfA scratch buffer

// Default WaveMix output format copied into the session by
// Hover_sub_43D880 (PCM, mono, 11025 Hz, 8-bit)
// hover.exe:0x4BFE30
static const PCMWAVEFORMAT g_wfxUnk_4BFE30 =
	{ { WAVE_FORMAT_PCM, 1, 11025, 11025, 1 }, 8 };

// Sound subsystem state (Hover_sub_4234F0/Hover_sub_423140)
// GLOBAL: HOVER 0x4c56d0
DWORD g_dwUnk_4C56D0;			// hover.exe:0x4C56D0 sound driver engine started
// GLOBAL: HOVER 0x4c56d4
DWORD g_dwUnk_4C56D4;			// hover.exe:0x4C56D4 wave output available

// In-game key bindings (virtual-key codes): defaults loaded from the
// string table by Hover_sub_4232E0, overridden by the 0x30-byte
// "PlayerControls" registry value; [11] is the joystick button binding

// Joystick state (Hover_sub_4232E0)

// Fixed-point math tables built by Hover_sub_438540 (entry 0 unused):
// g_adwSinTable[i] = sin((i-1) * pi / 4096) * 65536,
// g_awTanTable[i]  = tan((i-1) * pi / 65536) * 65536
// GLOBAL: HOVER 0x4aca54
DWORD g_adwSinTable[0x2001];		// hover.exe:0x4ACA54
// GLOBAL: HOVER 0x4a4a56
WORD g_awTanTable[0x4001];		// hover.exe:0x4A4A56

// Board zone geometry built by Hover_sub_41F540: six zone descriptor
// blocks of 0x338 bytes (0xCE ints) each, laid out back to back. The
// blocks hold zone sizes, split-line centers, sub-area rectangles,
// waypoint grids and flag zones; rectangles inside them are written with
// SetRect via LPRECT casts.
// hover.exe:0x4609A0
// GLOBAL: HOVER 0x4609a0
int g_adwUnk_4609A0[6 * (0x338 / 4)];

/////////////////////////////////////////////////////////////////////////////
// Free helpers

// Returns the main frame's active view (the game view window), or NULL
// when there is no main frame.
// hover.exe:0x401140
// FUNCTION: HOVER 0x401140
CBumperView* Hover_sub_401140()
{
	if (*(CFrameWnd* volatile*)&theApp.m_pMainWnd == NULL)
		return NULL;
	return (CBumperView*)((CFrameWnd*)theApp.m_pMainWnd)->GetActiveView();
}

// Processes the primary CGameObj list (g_dwUnk_4C52D8): calls each object's
// vtable slot 0x3c method, clears its list link and, if it was not already
// on the secondary list, moves it to g_dwUnk_4C52DC. Finally zeros the
// primary list head.
// hover.exe:0x408780
// FUNCTION: HOVER 0x408780
void Hover_sub_408780()
{
	CGameObj* pObj;
	CGameObj* pNext;

	pObj = (CGameObj*)g_dwUnk_4C52D8;
	while (pObj != NULL)
	{
		pNext = (CGameObj*)pObj->m_dwUnk70;
		pObj->m_dwUnk70 = 0;
		pObj->m_dwUnk68 = 0;
		pObj->Sub_4087D0();
		if (pObj->m_dwUnk6C == 0)
		{
			pObj->m_dwUnk74 = g_dwUnk_4C52DC;
			g_dwUnk_4C52DC = (DWORD)pObj;
			pObj->m_dwUnk6C = 1;
		}
		pObj = pNext;
	}
	g_dwUnk_4C52D8 = 0;
}

// Clears the secondary CGameObj list (g_dwUnk_4C52DC): calls each object's
// vtable slot 0x40 method and clears its secondary list link, then zeros
// the secondary list head.
// hover.exe:0x406C60
// FUNCTION: HOVER 0x406c60
void Hover_sub_406C60()
{
	CGameObj* pObj;
	CGameObj* pNext;

	pObj = (CGameObj*)g_dwUnk_4C52DC;
	while (pObj != NULL)
	{
		pNext = (CGameObj*)pObj->m_dwUnk74;
		pObj->m_dwUnk74 = 0;
		pObj->m_dwUnk6C = 0;
		pObj->Sub_4087F0();
		pObj = pNext;
	}
	g_dwUnk_4C52DC = 0;
}

// hover.exe:0x40E560
// FUNCTION: HOVER 0x40e560
int Hover_sub_40E560(int nUnk)
{
	int nRet;

	if (g_dwUnk_4C56D8 == 0)
		return 0;
	if (g_pUnk_4C56DC != NULL)
		return Hover_sub_439180(g_pUnk_4C56DC, nUnk);
	// The original returns the uninitialized local here
	return nRet;
}

// Waits on hEvent in 10 ms slices; while the wait keeps timing out it
// pumps WM_COPYDATA messages addressed to theApp.m_dwUnk03F0 so the
// synchronous wait cannot deadlock the shell's broadcast.

// hover.exe:0x427A10
// FUNCTION: HOVER 0x427a10
void Hover_sub_427A10(HANDLE hEvent)
{
	MSG msg;

	while (WaitForSingleObject(hEvent, 10) == WAIT_TIMEOUT)
	{
		if (PeekMessageA(&msg, (HWND)theApp.m_dwUnk03F0, WM_COPYDATA,
				WM_COPYDATA, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessageA(&msg);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
// CHoverApp

BEGIN_MESSAGE_MAP(CHoverApp, CWinApp)
	//{{AFX_MSG_MAP(CHoverApp)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
	ON_COMMAND(ID_CMD_34012, OnCmd34012)
	ON_COMMAND(ID_OPTIONS_CUSTOMIZE, OnOptionsCustomize)
	ON_COMMAND(ID_OPTIONS_CONTROLS, OnOptionsControls)
	ON_COMMAND(ID_OPTIONS_SOUNDS, OnOptionsSounds)
	ON_UPDATE_COMMAND_UI(ID_OPTIONS_SOUNDS, OnUpdateOptionsSounds)
	ON_COMMAND(ID_OPTIONS_FULLSCREEN, OnOptionsFullScreen)
	ON_UPDATE_COMMAND_UI(ID_OPTIONS_FULLSCREEN, OnUpdateOptionsFullScreen)
	ON_COMMAND(ID_GAME_STARTAT, OnGameStartAt)
	ON_COMMAND(ID_CMD_34006, OnCmd34006)
	ON_UPDATE_COMMAND_UI(ID_CMD_34006, OnUpdateCmd34006)
	ON_COMMAND(ID_CMD_34007, OnCmd34007)
	ON_UPDATE_COMMAND_UI(ID_CMD_34007, OnUpdateCmd34007)
	ON_COMMAND(ID_CMD_34009, OnCmd34009)
	ON_COMMAND(ID_CMD_34008, OnCmd34008)
	ON_COMMAND(ID_GAME_HIGHSCORES, OnGameHighScores)
	ON_UPDATE_COMMAND_UI(ID_GAME_HIGHSCORES, OnUpdateGameHighScores)
	ON_COMMAND(ID_HELP_QUICKHELP, OnHelpQuickHelp)
	ON_UPDATE_COMMAND_UI(ID_HELP_QUICKHELP, OnUpdateHelpQuickHelp)
	ON_COMMAND(ID_GAME_START, OnGameStart)
	ON_COMMAND(ID_CMD_34031, OnCmd34031)
	ON_COMMAND(ID_CMD_32821, OnCmd32821)
	//}}AFX_MSG_MAP
	// Standard file based document commands
	ON_COMMAND(ID_FILE_NEW, CWinApp::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, CWinApp::OnFileOpen)
END_MESSAGE_MAP()

// The 16.16 fixed-point scale factors below are written and read half by
// half, as the original does (notes/perfect-match.md rule 46).
#define FX_FRAC(dw) (*(WORD*)&(dw))
#define FX_INT(dw) (*((short*)&(dw) + 1))

// hover.exe:0x41C100
// FUNCTION: HOVER 0x41c100
CHoverApp::CHoverApp()
{
	m_dwUnk0158 = 360;
	m_dwUnk015C = 512;
	m_dwUnk016C = 60;
	m_dwUnk0160 = 360;
	m_dblUnk01A8 = 10.0;
	m_dblUnk01A0 = 0.3;
	m_dwUnk01BC = 0;
	m_dwUnk0164 = 180;
	m_dwUnk0168 = 180;
	m_pBumperDoc = NULL;
	m_dwUnk01C4 = 1;
	m_dwUnk01C8 = 1;
	m_dwUnk01CC = 0;
	m_dblUnk01B0 = 0.55;
	m_dwUnk01D4 = 1;

	SYSTEM_INFO sysInfo;
	GetSystemInfo(&sysInfo);
	if (sysInfo.dwProcessorType == PROCESSOR_INTEL_486)
		m_dwUnk01D0 = 0;
	else
		m_dwUnk01D0 = 1;

	m_dwUnk01E0 = 0;
	m_dwUnk03B0 = 0;
	m_dwUnk03B8 = 0;
	m_dwUnk03E0 = 0;
	m_dwUnk03E4 = 0;
	m_dwUnk01D8 = 1;
	m_dwUnk03E8 = 0;
	m_pUnk03EC = (BITMAPINFO*)new BYTE[0x428];
	m_dwUnk03F0 = 0;

	DWORD dwVersion = GetVersion();
	m_dwUnk03F4 = (DWORD)((LOBYTE(LOWORD(dwVersion)) << 8) +
		HIBYTE(LOWORD(dwVersion)));
	m_dwUnk03F8 = (dwVersion & 0x80000000) >> 31;
	m_pfnChangeDisplaySettings = NULL;
	m_dwUnk03FC = (LOBYTE(LOWORD(dwVersion)) >= 4);
	if (m_dwUnk03FC != 0)
	{
		// ChangeDisplaySettings is only available on Windows 95;
		// look it up dynamically so the game still runs on NT 3.51
		m_pfnChangeDisplaySettings =
			(LPCHANGEDISPLAYSETTINGS)GetProcAddress(
				GetModuleHandle("user32.dll"), "ChangeDisplaySettingsA");
	}

	g_dwUnk_46049C = 0;
	g_dwUnk_46048C = 1;
	m_dwUnk03BC = 0;
	m_dwUnk01B8 = 0;
	m_dwUnk01C0 = 0;

	// Compute the runtime fields of the level parameter table. The
	// scale factor is 16.16 fixed point, starting at 1.0 and increasing
	// by 0.5 per level; the original builds both halves of the two
	// constants by hand and scales through the by-pointer 16-bit multiply
	// (HOVER_FIXMULWP), reading the product's integer half back out of
	// memory as a signed short.
	DWORD dwScale;
	DWORD dwProd1;
	DWORD dwProd2;
	DWORD dwStep;
	int i;

	FX_FRAC(dwScale) = 0;
	FX_INT(dwScale) = 1;		// 1.0
	FX_INT(dwStep) = 0;
	FX_FRAC(dwStep) = 0x8000;	// 0.5
	for (i = 0; i < 16; i++)
	{
		dwProd1 = dwScale;
		HOVER_FIXMULWP(dwProd1, 1000);
		LVLPARAM(i).m_dwUnk44 = (DWORD)FX_INT(dwProd1);
		dwProd2 = dwScale;
		HOVER_FIXMULWP(dwProd2, 2500);
		LVLPARAM(i).m_dwUnk50 = (DWORD)FX_INT(dwProd2);
		if (i != 0)
			// Written out (not "+="): the original loads m_dwUnk38, adds
			// the product and stores, it does not read-modify-write.
			LVLPARAM(i).m_dwUnk38 = LVLPARAM(i).m_dwUnk30 *
				LVLPARAM(i).m_dwUnk44 + LVLPARAM(i).m_dwUnk38;
		LVLPARAM(i).m_dwUnk40 = LVLPARAM(i).m_dwUnk44 * 9 / 10;
		LVLPARAM(i).m_dwUnk4C = LVLPARAM(i).m_dwUnk50 * 9 / 10;
		if (i != 0)
			LVLPARAM(i).m_dwUnk34 = LVLPARAM(i).m_dwUnk38 * 9 / 10;
		LVLPARAM(i).m_dwUnk48 = LVLPARAM(i).m_dwUnk44 * 11 / 10;
		LVLPARAM(i).m_dwUnk54 = LVLPARAM(i).m_dwUnk50 * 11 / 10;
		if (i != 0)
			LVLPARAM(i).m_dwUnk3C = LVLPARAM(i).m_dwUnk38 * 11 / 10;
		dwScale += dwStep;
	}

	// Levels 16 and up cycle over the last three entries (via the
	// LVLPARAM mapping), which all share the values computed for
	// entry 15 above.
	for (i = 16; i < 20; i++)
	{
		LVLPARAM(i).m_dwUnk40 = g_lvlParams[15].m_dwUnk40;
		LVLPARAM(i).m_dwUnk44 = g_lvlParams[15].m_dwUnk44;
		LVLPARAM(i).m_dwUnk48 = g_lvlParams[15].m_dwUnk48;
		LVLPARAM(i).m_dwUnk4C = g_lvlParams[15].m_dwUnk4C;
		LVLPARAM(i).m_dwUnk50 = g_lvlParams[15].m_dwUnk50;
		LVLPARAM(i).m_dwUnk54 = g_lvlParams[15].m_dwUnk54;
		LVLPARAM(i).m_dwUnk34 = g_lvlParams[15].m_dwUnk34;
		LVLPARAM(i).m_dwUnk38 = g_lvlParams[15].m_dwUnk38;
		LVLPARAM(i).m_dwUnk3C = g_lvlParams[15].m_dwUnk3C;
	}
}

// The destructor has no body of its own. hover.exe:0x426660 is the scalar
// deleting destructor (??_G) that sits in slot 1 of the vtable; the
// compiler folded the empty ~CHoverApp into it, so all that is left there
// is the m_strUnk0414 CString member destructor, the chain to ~CWinApp and
// the "delete this" the flag argument selects.
// hover.exe:0x426660
// The destructor is defined in the class declaration, so the compiler folds
// it into the scalar deleting destructor that the vtable slot holds - which
// is what hover.exe:0x426660 is. The block below emits no code; it only gives
// the reccmp marker a declaration to name ??_GCHoverApp@@ (rules 34/184).
// hover.exe:0x426660
#if 0
// FUNCTION: HOVER 0x426660
CHoverApp::~CHoverApp()
{
}
#endif

/////////////////////////////////////////////////////////////////////////////
// The one and only CHoverApp object

// hover.exe:0x460570
// GLOBAL: HOVER 0x460570
CHoverApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CHoverApp initialization

// The original is not optimized code: `this` and every local live in their
// own frame slot and are reloaded at each use, the DC handles are compared
// against memory rather than tested in registers, and no import pointer is
// cached -- i.e. the original bracketed this function with
// #pragma optimize("", off) (see notes/perfect-match.md rule 49).
//
// The whole body is wrapped in a try/catch: the handler is the catch
// funclet at hover.exe:0x41D3C2, which reports the exception through
// Hover_sub_4262B0 (which aborts) and resumes at the "return TRUE".
// hover.exe:0x41C9E0
// hover.exe:0x4C47B0, defined in texload.cpp: InitInstance and the MIDI
// loader share one "winmm.dll" string.
extern char g_szUnk_4C47B0[];

// The startup code reaches the document through an inline accessor: the
// unoptimized code materializes its result into a frame temporary and then
// loads the vtable through a second one (rule 156), which is the shape at
// hover.exe:0x41D373.
inline CBumperDoc* HoverStartupDoc()
{
	return theApp.m_pBumperDoc;
}

#pragma optimize("", off)
// FUNCTION: HOVER 0x41c9e0
BOOL CHoverApp::InitInstance()
{
	// /Od gives every local its own frame slot and VC 2.1 orders the named
	// locals by C2's identifier hash instead of by declaration order, so the
	// only way to reproduce hover.exe's frame is to gather them into structs
	// (which are laid out in declaration order from their own base) and pick
	// struct names that hash into the right places -- notes/perfect-match.md
	// rule 160.  The comments give the original's ebp displacements.
	// strMessage cannot join a struct: the compiler would then construct it
	// through a generated constructor for the whole struct instead of calling
	// CString::CString on the slot.  VC reserves eight bytes of padding above
	// a CString local, which is the hole at [ebp-0x34]/[ebp-0x30].
	struct
	{
		BOOL bPaletteDevice;			// [ebp-0x2c]
		int nColors;				// [ebp-0x28]
		HDC hdc4;				// [ebp-0x24]
		HDC hdc3;				// [ebp-0x20]
		HDC hdc2;				// [ebp-0x1c]
		HDC hdc1;				// [ebp-0x18]
		HDC hdcScreen;				// [ebp-0x14]
	} hiHalf;

	hiHalf.hdc1 = NULL;

	try
	{
		if (LoadString(NULL, IDS_MEMORY_ERROR, g_szMemoryError,
				sizeof(g_szMemoryError)) == 0)
		{
			g_szMemoryError[0] = '\0';
		}
		if (!g_strReadFileError.LoadString(IDS_READ_FILE_ERROR))
			g_strReadFileError.Empty();
		if (!g_strUnexpectedException.LoadString(IDS_UNEXPECTED_EXCEPTION))
			g_strUnexpectedException.Empty();
		if (!g_strExcNotSupported.LoadString(IDS_EXC_NOT_SUPPORTED))
			g_strExcNotSupported.Empty();
		if (!g_strExcArchiveNone.LoadString(IDS_EXC_ARCHIVE_NONE))
			g_strExcArchiveNone.Empty();
		if (!g_strExcArchiveGeneric.LoadString(IDS_EXC_ARCHIVE_GENERIC))
			g_strExcArchiveGeneric.Empty();
		if (!g_strExcArchiveReadOnly.LoadString(IDS_EXC_ARCHIVE_READONLY))
			g_strExcArchiveReadOnly.Empty();
		if (!g_strExcArchiveEndOfFile.LoadString(IDS_EXC_ARCHIVE_ENDOFFILE))
			g_strExcArchiveEndOfFile.Empty();
		if (!g_strExcArchiveWriteOnly.LoadString(IDS_EXC_ARCHIVE_WRITEONLY))
			g_strExcArchiveWriteOnly.Empty();
		if (!g_strExcArchiveBadIndex.LoadString(IDS_EXC_ARCHIVE_BADINDEX))
			g_strExcArchiveBadIndex.Empty();
		if (!g_strExcArchiveBadClass.LoadString(IDS_EXC_ARCHIVE_BADCLASS))
			g_strExcArchiveBadClass.Empty();
		if (!g_strExcArchiveBadSchema.LoadString(IDS_EXC_ARCHIVE_BADSCHEMA))
			g_strExcArchiveBadSchema.Empty();
		if (!g_strExcResource.LoadString(IDS_EXC_RESOURCE))
			g_strExcResource.Empty();
		if (!g_strExcOle.LoadString(IDS_EXC_OLE))
			g_strExcOle.Empty();
		if (!g_strExcDao.LoadString(IDS_EXC_DAO))
			g_strExcDao.Empty();
		if (!g_strExcUser.LoadString(IDS_EXC_USER))
			g_strExcUser.Empty();

		// Verify that the system has enough free resources to run the game
		// by creating (and immediately deleting) four device contexts.
		hiHalf.hdcScreen = GetDC(NULL);
		if (hiHalf.hdcScreen != NULL)
		{
			hiHalf.hdc1 = CreateCompatibleDC(hiHalf.hdcScreen);
			hiHalf.hdc2 = CreateCompatibleDC(hiHalf.hdcScreen);
			hiHalf.hdc3 = CreateCompatibleDC(hiHalf.hdcScreen);
			hiHalf.hdc4 = CreateCompatibleDC(hiHalf.hdcScreen);
			if (hiHalf.hdc1 != NULL)
				DeleteDC(hiHalf.hdc1);
			if (hiHalf.hdc2 != NULL)
				DeleteDC(hiHalf.hdc2);
			if (hiHalf.hdc3 != NULL)
				DeleteDC(hiHalf.hdc3);
			if (hiHalf.hdc4 != NULL)
				DeleteDC(hiHalf.hdc4);
			ReleaseDC(NULL, hiHalf.hdcScreen);
		}
		if (hiHalf.hdc1 == NULL || hiHalf.hdc2 == NULL ||
			hiHalf.hdc3 == NULL || hiHalf.hdc4 == NULL)
		{
			AfxMessageBox(IDS_NO_RESOURCES, MB_OK, (UINT)-1);
			abort();	// hover.exe:0x43E32B
		}

		CString strMessage;

		{
			// One block deeper than strMessage: VC allocates the
			// locals of a nested scope below those of the enclosing
			// one, which is what leaves the CString sitting between
			// the two groups in hover.exe's frame.  The splash
			// dialog pointer has to stay a plain local -- a member
			// call through a struct member costs an extra /Od
			// temporary (rule 191) -- so the group is split around
			// it, and the two struct names are picked so that C2's
			// identifier hash orders them around it (rule 160).
			struct
			{
				CSingleDocTemplate* pDocTemplate;	// [ebp-0x6c]
				MEMORYSTATUS memStatus;			// [ebp-0x68]
				JOYINFO joyInfo;			// [ebp-0x48]
			} up;
			CDialog* pDlg;					// [ebp-0x70]
			struct
			{
				UINT i;					// [ebp-0x84]
				UINT nMidiDevices;			// [ebp-0x80]
				HMODULE hWinMM;				// [ebp-0x7c]
				DWORD dwVersion;			// [ebp-0x78]
				int nWinVer;				// [ebp-0x74]
			} dn;

			InitializeCriticalSection(&m_csUnk00D4);
			InitializeCriticalSection(&m_csUnk00EC);
			InitializeCriticalSection(&m_csUnk0104);
			InitializeCriticalSection(&m_csUnk011C);
			InitializeCriticalSection(&m_csUnk0134);

			if (!Sub_425E40())	// minimum configuration check
			{
				strMessage.LoadString(IDS_MIN_CONFIG);
				AfxMessageBox(strMessage, 0, 0);
				return FALSE;
			}
			if (!Sub_425E80())	// single instance check
				return FALSE;

			// Select a buffer size based on the amount of physical memory;
			// holding down a key at startup forces low memory mode. The
			// original passes 0x61 (ASCII 'a') to GetKeyState, which is
			// actually VK_NUMPAD1 -- apparently an original quirk/bug.
			up.memStatus.dwLength = sizeof(up.memStatus);
			GlobalMemoryStatus(&up.memStatus);
			g_dwUnk_461CFC = 0;
			if (up.memStatus.dwTotalPhys < 0x900000 || GetKeyState('a') < 0)
				g_dwUnk_461CFC = 1;
			switch (g_dwUnk_461CFC)
			{
			case 0:
				g_cbUnk_46002C = 0x54C338;
				g_pUnk_46003C = new BYTE[g_cbUnk_46002C];
				break;
			case 1:
				g_cbUnk_46002C = 0x1AF84C;
				g_pUnk_46003C = new BYTE[g_cbUnk_46002C];
				break;
			case 2:
				g_cbUnk_46002C = 0x8AB3D;
				g_pUnk_46003C = new BYTE[g_cbUnk_46002C];
				break;
			}

			// On Windows NT, require version 3.51 or later (GetVersion returns
			// a negative value on Windows 95, which skips this check). The
			// byte swap of the low word is what puts the major version in the
			// high byte; spelled with masks it compiles to the original's
			// xor eax,eax / mov ah / mov al (rule 189).
			dn.dwVersion = GetVersion();
			dn.nWinVer = ((dn.dwVersion << 8) & 0xff00) |
				((dn.dwVersion >> 8) & 0xff);
			if ((int)dn.dwVersion > 0 && dn.nWinVer < 0x0333)
			{
				AfxMessageBox(IDS_BAD_WINDOWS, MB_OK, (UINT)-1);
				return FALSE;
			}

			// The game requires a 256 color (or better) palette device
			hiHalf.hdcScreen = GetDC(NULL);
			hiHalf.bPaletteDevice =
				(GetDeviceCaps(hiHalf.hdcScreen, RASTERCAPS) & RC_PALETTE) != 0;
			if (hiHalf.bPaletteDevice)
				hiHalf.nColors = GetDeviceCaps(hiHalf.hdcScreen, SIZEPALETTE);
			else
				hiHalf.nColors = GetDeviceCaps(hiHalf.hdcScreen, NUMCOLORS);
			ReleaseDC(NULL, hiHalf.hdcScreen);
			if (!(hiHalf.bPaletteDevice && hiHalf.nColors == 256))
			{
				if (hiHalf.nColors == 16)
				{
					AfxMessageBox(IDS_16_COLOR, MB_OK, (UINT)-1);
					return FALSE;
				}
				AfxMessageBox(IDS_NOT_256_COLOR, MB_OK, (UINT)-1);
			}

			Enable3dControls();

			// The splash screen is shown modeless during startup; it destroys
			// itself after four seconds or on any key press (see splashdlg.cpp)
			pDlg = new CSplashDialog(NULL);
			pDlg->Create(IDD_SPLASH);

			LoadStdProfileSettings(0);	// no most recently used file list

			up.pDocTemplate = new CSingleDocTemplate(
				IDR_MAINFRAME,
				RUNTIME_CLASS(CBumperDoc),
				RUNTIME_CLASS(CMainFrame),
				RUNTIME_CLASS(CBumperView));
			AddDocTemplate(up.pDocTemplate);

			Hover_sub_4234F0();
			if (!Hover_sub_423140())
			{
				// retry with the flag flipped
				g_dwUnk_4C56D8 = !g_dwUnk_4C56D8;
				if (!Hover_sub_423140())
				{
					m_dwUnk0404 = 0;
					m_dwUnk0408 = 0;
					g_dwUnk_4C56D8 = !g_dwUnk_4C56D8;
				}
			}

			// joyGetPosEx is optional; look it up dynamically
			dn.hWinMM = GetModuleHandle(g_szUnk_4C47B0);
			if (dn.hWinMM != NULL)
				g_pfnJoyGetPosEx = GetProcAddress(dn.hWinMM, "joyGetPosEx");
			// One if/else, not nested ifs: all three failure paths
			// branch to the single m_bJoystick = FALSE in hover.exe.
			if (g_pfnJoyGetPosEx != NULL &&
				joyGetDevCaps(0, &m_joyCaps, sizeof(m_joyCaps)) == JOYERR_NOERROR &&
				joyGetPos(0, &up.joyInfo) == JOYERR_NOERROR)
			{
				m_bJoystick = TRUE;
			}
			else
			{
				m_bJoystick = FALSE;
			}

			Hover_sub_41F540();

			// Look for a MIDI output device that can play the game's music
			dn.nMidiDevices = midiOutGetNumDevs();
			if (dn.nMidiDevices == 0)
			{
				m_dwUnk0410 = 0;
				m_dwUnk040C = 0;
			}
			else
			{
				m_dwUnk0410 = 0;
				for (dn.i = 0; dn.nMidiDevices > dn.i; dn.i++)
				{
					// One scope deeper again, so these take the two
					// lowest named slots, below up and dn.
					struct
					{
						MMRESULT mmr;		// [ebp-0xbc]
						MIDIOUTCAPS midiCaps;	// [ebp-0xb8]
					} loop;

					loop.mmr = midiOutGetDevCaps(dn.i, &loop.midiCaps,
						sizeof(loop.midiCaps));
					if (loop.mmr == MMSYSERR_NOERROR &&
						(loop.midiCaps.wTechnology == MOD_MAPPER ||
							loop.midiCaps.wTechnology == MOD_FMSYNTH))
					{
						m_dwUnk0410 = 1;
					}
				}
				if (m_dwUnk0410 == 0)
				{
					m_dwUnk040C = 0;
				}
			}

			Hover_sub_438540();

			m_pUnk014C = &g_HBegParams;
			m_pUnk0150 = &g_RBegParams;
			m_pUnk0154 = &g_RobotAIBegParams;

			Hover_sub_4249C0();
			Hover_sub_424A70();
			Hover_sub_424B20();
			Hover_sub_42D180();
			Hover_sub_4232E0();
			Sub_425870();

			m_dwUnk03B4 = 1;

			Sub_41C7E0();		// locate the game data directory

			// create a new (empty) document
			OnFileNew();

			HoverStartupDoc()->SetTitle("Hover!");

			if (m_lpCmdLine[0] != '\0')
			{
				// TODO: add command line processing here
			}
		}
	}
	catch (CException* e)
	{
		Hover_sub_4262B0(e);
	}
	return TRUE;
}
#pragma optimize("", on)

/////////////////////////////////////////////////////////////////////////////
// Game settings persistence
//
// Hover_sub_42D180 (called by CHoverApp::InitInstance) loads the game
// settings from HKEY_CURRENT_USER\Software\Microsoft\Hover! and
// Hover_sub_42D290 (called by CHoverApp::ExitInstance) saves them back.
// "GameSettings" is a 5-byte REG_BINARY blob whose bytes are sign-extended
// into the g_dwUnk_4607xx DWORDs; "StartAtLevel" is a 1-byte REG_BINARY
// clamped to the range 1..20.

// hover.exe:0x42D180
// FUNCTION: HOVER 0x42d180
void Hover_sub_42D180()
{
	HKEY hKey;
	if (RegOpenKeyExA(HKEY_CURRENT_USER, "Software\\Microsoft\\Hover!",
			0, KEY_ALL_ACCESS, &hKey) != ERROR_SUCCESS)
		return;

	BYTE buf[5];
	DWORD cbData = 5;
	DWORD dwType = REG_BINARY;
	if (RegQueryValueExA(hKey, "GameSettings", NULL, &dwType, buf, &cbData)
			== ERROR_SUCCESS && cbData == 5)
	{
		theApp.m_dwUnk01CC = (signed char)buf[0];
		theApp.m_dwUnk01C8 = (signed char)buf[1];
		theApp.m_dwUnk01D4 = (signed char)buf[2];
		theApp.m_dwUnk01C4 = (signed char)buf[3];
		theApp.m_dwUnk01D0 = (signed char)buf[4];
	}

	cbData = 1;
	if (RegQueryValueExA(hKey, "StartAtLevel", NULL, &dwType, buf, &cbData)
			== ERROR_SUCCESS && cbData == 1)
	{
		g_dwUnk_46048C = (signed char)buf[0];
		if ((int)g_dwUnk_46048C < 1)
			g_dwUnk_46048C = 1;
		if ((int)g_dwUnk_46048C > 20)
			g_dwUnk_46048C = 20;
	}
	else
		g_dwUnk_46048C = 1;

	RegCloseKey(hKey);
}

// hover.exe:0x42D290
// FUNCTION: HOVER 0x42d290
void Hover_sub_42D290()
{
	HKEY hKey;
	DWORD dwDisposition;
	if (RegCreateKeyExA(HKEY_CURRENT_USER, "Software\\Microsoft\\Hover!",
			0, NULL, 0, KEY_ALL_ACCESS, NULL, &hKey, &dwDisposition)
			!= ERROR_SUCCESS)
		return;

	{
		BYTE buf[5];
		buf[0] = (BYTE)theApp.m_dwUnk01CC;
		buf[1] = (BYTE)theApp.m_dwUnk01C8;
		buf[2] = (BYTE)theApp.m_dwUnk01D4;
		buf[3] = (BYTE)theApp.m_dwUnk01C4;
		buf[4] = (BYTE)theApp.m_dwUnk01D0;
		RegSetValueExA(hKey, "GameSettings", 0, REG_BINARY, buf, 5);
	}

	{
		BYTE bLevel = (BYTE)g_dwUnk_46048C;
		RegSetValueExA(hKey, "StartAtLevel", 0, REG_BINARY, &bLevel, 1);
	}

	RegCloseKey(hKey);
}

/////////////////////////////////////////////////////////////////////////////
// Subsystem initialization helpers called by CHoverApp::InitInstance

// Detects the processor type and loads the 4-byte "SoundSettings"
// registry value (defaults: byte0 = processor type != 486, the rest 1).

// hover.exe:0x4234F0
// FUNCTION: HOVER 0x4234f0
void Hover_sub_4234F0()
{
	MEMORYSTATUS memStatus;
	memStatus.dwLength = sizeof(memStatus);
	GlobalMemoryStatus(&memStatus);

	SYSTEM_INFO sysInfo;
	GetSystemInfo(&sysInfo);
	theApp.m_dwUnk040C = 0;
	if (sysInfo.dwProcessorType != PROCESSOR_INTEL_486)
		theApp.m_dwUnk040C = 1;

	theApp.m_dwUnk0404 = 1;
	theApp.m_dwUnk0408 = 1;
	g_dwUnk_4C56D8 = 1;

	HKEY hKey;
	if (RegOpenKeyExA(HKEY_CURRENT_USER, "Software\\Microsoft\\Hover!",
			0, KEY_ALL_ACCESS, &hKey) == ERROR_SUCCESS)
	{
		BYTE buf[4];
		DWORD cbData = 4;
		DWORD dwType = REG_BINARY;
		if (RegQueryValueExA(hKey, "SoundSettings", NULL, &dwType,
				buf, &cbData) == ERROR_SUCCESS)
		{
			theApp.m_dwUnk040C = (signed char)buf[0];
			theApp.m_dwUnk0404 = (signed char)buf[1];
			theApp.m_dwUnk0408 = (signed char)buf[2];
			g_dwUnk_4C56D8 = (signed char)buf[3];
		}
		RegCloseKey(hKey);
	}
}

// Initializes the sound subsystem. When g_dwUnk_4C56D8 is zero the game
// mixes sound in a worker thread (requires a wave device supporting
// 22.05 kHz mono 8-bit); otherwise it creates the sound driver engine
// object. Returns FALSE on failure, which InitInstance retries once.

// hover.exe:0x423140
// FUNCTION: HOVER 0x423140
BOOL Hover_sub_423140()
{
	g_dwUnk_4C56D4 = 0;
	g_dwUnk_4C56D0 = 0;
	if (g_dwUnk_4C56D8 == 0)
	{
		if (waveOutGetNumDevs() != 0)
		{
			WAVEOUTCAPSA waveCaps;
			if (waveOutGetDevCapsA(0, &waveCaps, sizeof(waveCaps))
					== MMSYSERR_NOERROR &&
				(waveCaps.dwFormats & WAVE_FORMAT_2M08) != 0)
			{
				g_dwUnk_4C56D4 = 1;
			}
		}
		if (g_dwUnk_4C56D4 == 0)
			return FALSE;
		if (g_dwUnk_4C56E4 == 0)
		{
			InitializeCriticalSection(&g_csUnk_4A4808);
			g_dwUnk_4C56E4 = 1;
		}
		if (g_hUnk_4C56F4 == NULL)
			g_hUnk_4C56F4 = CreateEventA(NULL, TRUE, FALSE, NULL);
		if (g_hUnk_4C56F8 == NULL)
			g_hUnk_4C56F8 = CreateEventA(NULL, TRUE, FALSE, NULL);
		if (g_pThreadUnk_4C56E0 == NULL)
		{
			CWinThread* pThread =
				AfxBeginThread(Hover_sub_42D6A0, NULL);
			pThread->m_bAutoDelete = TRUE;
			g_dwUnk_4C56E8 = 0;
			g_pThreadUnk_4C56E0 = pThread;
		}
		return TRUE;
	}

	SOUND_DRIVER_CFG cfg;
	HKEY hKey = NULL;
	cfg.wUnk00 = 0x1e;
	cfg.dwUnk02 = 3;
	cfg.wUnk06 = 1;
	cfg.wUnk08 = 0xb;
	if (RegOpenKeyA(HKEY_LOCAL_MACHINE, "software\\Microsoft\\Hover!",
			&hKey) == ERROR_SUCCESS)
	{
		cfg.dwUnk02 |= 0x400;
		cfg.dwUnk1A = (DWORD)hKey;
	}
	g_pUnk_4C56DC = Hover_sub_43D880(&cfg);
	if (g_pUnk_4C56DC == NULL)
		return FALSE;
	if (Hover_sub_4393A0(g_pUnk_4C56DC, 1, 1) != 0)
		return FALSE;
	g_dwUnk_4C56D0 = 1;
	if (hKey != NULL)
		RegCloseKey(hKey);
	return TRUE;
}


/////////////////////////////////////////////////////////////////////////////
// MIDI stream playback (CSub425050)
//
// The eight midiStream* entry points are resolved from winmm.dll at run
// time by CSub425050::Sub_437F00 (texload.cpp) into g_pfnUnk_4C5908 ..
// g_pfnUnk_4C5924, all declared with the same one-argument
// LPMIDIDEVICEFN type, so the three that take more arguments are cast
// back to their real prototype at the call. Every call is guarded by a
// NULL test whose "not resolved" arm yields MMSYSERR_ERROR - dead code,
// because Sub_437F00 clears g_pfnUnk_4C5908 again unless all eight
// pointers resolved, but the original emits it all the same.

typedef MMRESULT (WINAPI* LPFNMIDISTREAMOPEN)(LPHMIDISTRM, LPUINT, DWORD,
	DWORD, DWORD, DWORD);
typedef MMRESULT (WINAPI* LPFNMIDISTREAMPROPERTY)(HMIDISTRM, LPBYTE, DWORD);
typedef MMRESULT (WINAPI* LPFNMIDISTREAMOUT)(HMIDISTRM, LPMIDIHDR, UINT);
typedef MMRESULT (WINAPI* LPFNMIDISTREAMHANDLE)(HMIDISTRM);

// The MidiOutProc that CSub425050::Sub_438300 installs on the stream.
// Only MOM_DONE is handled, and only while the object that owns the
// stream is still published in g_pUnk_4C5904. The finished buffer is
// first handed straight back to the driver, so the level music keeps
// looping for as long as midiStreamOut accepts it; when that fails (or
// when the object has already been marked stopped in m_dwUnk20) the
// buffer is appended to the m_dwUnk10/m_dwUnk14 free list instead and
// the queued-buffer count m_dwUnk1C goes down by one.
//
// Note the callback runs on the MIDI driver's thread and touches the
// list with no synchronization at all - an original quirk.

// hover.exe:0x438440
// FUNCTION: HOVER 0x438440
void CALLBACK Hover_sub_438440(HMIDIOUT hMidi, UINT uMsg, DWORD dwInstance,
	DWORD dwParam1, DWORD dwParam2)
{
	LPFNMIDISTREAMOUT pfnOut;
	CSub425050* pObj;
	MIDIHDR* pHdr;
	MMRESULT mmr;

	pObj = g_pUnk_4C5904;
	if (pObj == NULL)
		return;
	if (uMsg != MOM_DONE)
		return;
	pHdr = (MIDIHDR*)dwParam1;
	if (pObj->m_dwUnk20 == 0)
	{
		pfnOut = (LPFNMIDISTREAMOUT)g_pfnUnk_4C5918;
		if (pfnOut != NULL)
		{
			mmr = pfnOut((HMIDISTRM)pObj->m_dwUnk18, pHdr,
				sizeof(MIDIHDR));
			// re-read rather than keep the object live across the
			// call: this is what puts it in ecx instead of costing
			// the frame a second callee-saved register
			pObj = g_pUnk_4C5904;
		}
		else
			mmr = MMSYSERR_ERROR;
		if (mmr == 0)
			return;
		pObj->m_dwUnk20 = 1;
	}
	pHdr->lpNext = NULL;
	if (pObj->m_dwUnk14 == 0)
		pObj->m_dwUnk10 = (DWORD)pHdr;
	else
		((MIDIHDR*)pObj->m_dwUnk14)->lpNext = pHdr;
	pObj->m_dwUnk14 = (DWORD)pHdr;
	pObj->m_dwUnk1C--;
}

// Opens the MIDI stream device for the .muz file Sub_437F00 loaded and
// hands the driver every prepared buffer of the m_dwUnk10 list: the
// stream is opened on the MIDI mapper with Hover_sub_438440 as its
// callback function, each buffer is prepared with midiOutPrepareHeader,
// the file's time division (m_dwUnk08) is published through
// midiStreamProperty, the stream is paused so that nothing plays while
// the buffers are queued, and midiStreamRestart finally starts it.
// Queuing a buffer moves it off the m_dwUnk10 list and bumps the
// in-driver count m_dwUnk1C; Hover_sub_438440 puts it back. Any failure
// tears the whole stream down again through Sub_438260 and returns 0.

// hover.exe:0x438300
// FUNCTION: HOVER 0x438300
int CSub425050::Sub_438300()
{
	int bOk;
	MIDIPROPTIMEDIV prop;
	MIDIHDR* pHdr;
	LPFNMIDISTREAMOPEN pfnOpen;
	LPFNMIDISTREAMPROPERTY pfnProperty;
	LPFNMIDISTREAMOUT pfnOut;
	LPFNMIDISTREAMHANDLE pfn;
	UINT uDeviceID;
	MMRESULT mmr;

	bOk = 0;
	pfnOpen = (LPFNMIDISTREAMOPEN)g_pfnUnk_4C5908;
	m_dwUnk18 = 0;
	m_dwUnk1C = 0;
	m_dwUnk20 = 0;
	uDeviceID = MIDI_MAPPER;
	if (pfnOpen != NULL)
		mmr = pfnOpen((LPHMIDISTRM)&m_dwUnk18, &uDeviceID, 1,
			(DWORD)Hover_sub_438440, 0, CALLBACK_FUNCTION);
	else
		mmr = MMSYSERR_ERROR;
	if (mmr != 0)
		goto Done;
	pHdr = (MIDIHDR*)m_dwUnk10;
	while (pHdr != NULL)
	{
		mmr = midiOutPrepareHeader((HMIDIOUT)m_dwUnk18, pHdr,
			sizeof(MIDIHDR));
		if (mmr != 0)
			goto Done;
		pHdr = pHdr->lpNext;
	}
	prop.dwTimeDiv = m_dwUnk08;
	pfnProperty = (LPFNMIDISTREAMPROPERTY)g_pfnUnk_4C5910;
	prop.cbStruct = sizeof(prop);
	if (pfnProperty != NULL)
		mmr = pfnProperty((HMIDISTRM)m_dwUnk18, (LPBYTE)&prop,
			MIDIPROP_SET | MIDIPROP_TIMEDIV);
	else
		mmr = MMSYSERR_ERROR;
	if (mmr != 0)
		goto Done;
	pfn = (LPFNMIDISTREAMHANDLE)g_pfnUnk_4C591C;		// midiStreamPause
	if (pfn != NULL)
		mmr = pfn((HMIDISTRM)m_dwUnk18);
	else
		mmr = MMSYSERR_ERROR;
	if (mmr != 0)
		goto Done;
	pfnOut = (LPFNMIDISTREAMOUT)g_pfnUnk_4C5918;	// midiStreamOut
	while (m_dwUnk10 != 0)
	{
		pHdr = (MIDIHDR*)m_dwUnk10;
		m_dwUnk10 = (DWORD)pHdr->lpNext;
		if (pfnOut != NULL)
		{
			mmr = pfnOut((HMIDISTRM)m_dwUnk18, pHdr, sizeof(MIDIHDR));
			// re-read inside the arm: this is what makes VC hoist
			// the load into the loop preheader and reload only
			// after the call, as the original does
			pfnOut = (LPFNMIDISTREAMOUT)g_pfnUnk_4C5918;
		}
		else
			mmr = MMSYSERR_ERROR;
		if (mmr != 0)
			goto Done;
		m_dwUnk1C++;
	}
	m_dwUnk14 = 0;
	pfn = (LPFNMIDISTREAMHANDLE)g_pfnUnk_4C5920;		// midiStreamRestart
	if (pfn != NULL)
		mmr = pfn((HMIDISTRM)m_dwUnk18);
	else
		mmr = MMSYSERR_ERROR;
	if (mmr != 0)
		goto Done;
	bOk = 1;
Done:
	if (bOk == 0)
		Sub_438260();
	return bOk;
}
/////////////////////////////////////////////////////////////////////////////
// WaveMix sound engine (WaveMix V 2.3, statically linked into hover.exe)

// WaveMix debug output: the message goes to OutputDebugStringA when its
// level does not exceed the g_dwUnk_4C59BC verbosity threshold (a zero
// threshold silences all output).

// hover.exe:0x438820
// FUNCTION: HOVER 0x438820
void Hover_sub_438820(int nLevel, const char* pszMsg)
{
	if (g_dwUnk_4C59BC != 0 && nLevel <= (int)g_dwUnk_4C59BC)
		OutputDebugStringA(pszMsg);
}

// Sound mixing worker thread proc (AFX_THREADPROC). Waits on
// g_hUnk_4C56F4, then under g_csUnk_4A4808 dequeues the pending sound
// objects: g_pvUnk_4C56EC is played once (SND_MEMORY), g_pvUnk_4C56F0 is
// played looping (SND_MEMORY|SND_LOOP|SND_ASYNC); with neither queued the
// current sound is stopped (sndPlaySoundA(NULL, 0)). The queued objects
// have the sound data pointer at offset +4. g_dwUnk_4C56E8 signals the
// thread to exit after the current pass; g_hUnk_4C56F8 is set at the end
// of every pass.

// hover.exe:0x42D6A0
// FUNCTION: HOVER 0x42d6a0
UINT Hover_sub_42D6A0(LPVOID pParam)
{
	void* pOnce;
	void* pLoop;
	BOOL bDone = FALSE;

	do
	{
		WaitForSingleObject(g_hUnk_4C56F4, INFINITE);
		pOnce = NULL;
		EnterCriticalSection(&g_csUnk_4A4808);
		if (g_dwUnk_4C56E8 != 0)
		{
			bDone = TRUE;
			pLoop = NULL;
		}
		else
		{
			pOnce = g_pvUnk_4C56EC;
			pLoop = g_pvUnk_4C56F0;
		}
		g_pvUnk_4C56EC = NULL;
		LeaveCriticalSection(&g_csUnk_4A4808);
		ResetEvent(g_hUnk_4C56F4);
		if (pOnce != NULL && ((void**)pOnce)[1] != NULL)
			sndPlaySoundA((LPCSTR)((void**)pOnce)[1], SND_MEMORY);
		if (pLoop != NULL && ((void**)pLoop)[1] != NULL)
			sndPlaySoundA((LPCSTR)((void**)pLoop)[1],
				SND_MEMORY | SND_LOOP | SND_ASYNC);
		if (pOnce == NULL && pLoop == NULL)
			sndPlaySoundA(NULL, 0);
		SetEvent(g_hUnk_4C56F8);
	}
	while (!bDone);
	return 0;
}

// WaveMixOpenChannel: validates the session object (0x5432 magic words at
// both ends; an invalid handle is reported via MessageBeep/wsprintfA/
// MessageBoxA), then opens mixing channels. nMode 0 opens channel
// nChannel (must be free), nMode 1 opens all 16 channels, nMode 2 opens
// the first nChannel channels. Returns 0 on success, 4 = channel busy,
// 5 = invalid session, 0xa = invalid mode, 0xb = invalid channel count.

// hover.exe:0x4393A0
// FUNCTION: HOVER 0x4393a0
int __stdcall Hover_sub_4393A0(SOUNDENGINE* pEngine, int nChannel,
	int nMode)
{
	int i;
	Hover_sub_438820(2, "<WaveMixOpenChannel>");
	if (pEngine != NULL && pEngine->wMagic00 == 0x5432 &&
		pEngine->wMagic1BC == 0x5432)
	{
		g_pUnk_4B6248 = pEngine;
	}
	else
	{
		MessageBeep(0xFFFFFFFF);
		wsprintfA(g_szUnk_4C59D0,
			"Invalid session handle 0x%04X passed to WaveMix API",
			pEngine);
		Hover_sub_438820(1, g_szUnk_4C59D0);
		MessageBoxA(NULL, g_szUnk_4C59D0, "WavMix32",
			MB_ICONEXCLAMATION);
		g_pUnk_4B6248 = NULL;
	}
	if (g_pUnk_4B6248 == NULL)
		return 5;
	if ((DWORD)nMode > 2)
		return 0xa;
	if (nMode == 0 && nChannel >= 0x10)
		return 0xb;
	if (nMode == 2 && (nChannel > 0x10 || nChannel < 1))
		return 0xb;
	if (nMode == 0)
	{
		if (g_pUnk_4B6248->adwUnkF4[nChannel] != 0xFFFFFFFF)
			return 4;
		g_pUnk_4B6248->adwUnkF4[nChannel] = 0;
		g_pUnk_4B6248->awUnkB4[nChannel * 2] =
			g_pUnk_4B6248->wUnkB0;
		g_pUnk_4B6248->awUnkB4[nChannel * 2 + 1] =
			g_pUnk_4B6248->wUnkB2;
		g_pUnk_4B6248->dwUnk134++;
		return 0;
	}
	if (nMode == 1)
		nChannel = 0x10;
	else if (nMode != 2)
	{
		Hover_sub_438820(2,
			"WaveMixOpenChannel: Invalid flag was not trapped!!!");
		return 0xa;
	}
	for (i = nChannel - 1; i >= 0; i--)
	{
		if (g_pUnk_4B6248->adwUnkF4[i] == 0xFFFFFFFF)
		{
			g_pUnk_4B6248->adwUnkF4[i] = 0;
			g_pUnk_4B6248->awUnkB4[i * 2] =
				g_pUnk_4B6248->wUnkB0;
			g_pUnk_4B6248->awUnkB4[i * 2 + 1] =
				g_pUnk_4B6248->wUnkB2;
			g_pUnk_4B6248->dwUnk134++;
		}
	}
	return 0;
}

// Writes the session's current WaveMix settings back to the ini file.
// The dwFlags bits select which entries are written (same bit
// assignments as Hover_sub_43C8F0): 0x80 device, 0x01 channels, 0x02
// sampling rate, 0x04 wave blocks, 0x08 block length, 0x10 mixing
// function, 0x20 remix callback, 0x40 position source, 0x100 debug
// dialogs, 0x200 pause blocks. Called by Hover_sub_43C8F0 when its ini
// update parameter is set.

// hover.exe:0x43D100
// FUNCTION: HOVER 0x43d100
void Hover_sub_43D100(DWORD dwFlags)
{
	if ((dwFlags & 0x80) != 0)
	{
		wsprintfA(g_szUnk_4C59D0, "%d", g_pUnk_4B6248->dwUnk18);
		WritePrivateProfileStringA("general", "WaveOutDevice",
			g_szUnk_4C59D0, g_szUnk_4B6D58);
	}
	if ((dwFlags & 0x01) != 0)
	{
		wsprintfA(g_szUnk_4C59D0, "%d",
			g_pUnk_4B6248->wfxUnk17C.wf.nChannels);
		WritePrivateProfileStringA(g_pUnk_4B6248->szUnk1C, "Channels",
			g_szUnk_4C59D0, g_szUnk_4B6D58);
	}
	if ((dwFlags & 0x02) != 0)
	{
		WORD wSamplesPerSec;

		wSamplesPerSec = (WORD)(
			g_pUnk_4B6248->wfxUnk17C.wf.nSamplesPerSec / 11025);
		wSamplesPerSec = (WORD)(wSamplesPerSec * 11);
		wsprintfA(g_szUnk_4C59D0, "%d", wSamplesPerSec);
		WritePrivateProfileStringA(g_pUnk_4B6248->szUnk1C,
			"SamplesPerSec", g_szUnk_4C59D0, g_szUnk_4B6D58);
	}
	if ((dwFlags & 0x04) != 0)
	{
		wsprintfA(g_szUnk_4C59D0, "%d", g_pUnk_4B6248->dwUnk190);
		WritePrivateProfileStringA(g_pUnk_4B6248->szUnk1C,
			"WaveBlocks", g_szUnk_4C59D0, g_szUnk_4B6D58);
	}
	if ((dwFlags & 0x08) != 0)
	{
		wsprintfA(g_szUnk_4C59D0, "%d",
			(WORD)g_pUnk_4B6248->dwUnk18C);
		WritePrivateProfileStringA(g_pUnk_4B6248->szUnk1C,
			"WaveBlockLen", g_szUnk_4C59D0, g_szUnk_4B6D58);
	}
	if ((dwFlags & 0x10) != 0)
	{
		wsprintfA(g_szUnk_4C59D0, "%d",
			g_pUnk_4B6248->dwUnk1AC != (DWORD)Hover_sub_4389F0);
		WritePrivateProfileStringA(g_pUnk_4B6248->szUnk1C, "CMixit",
			g_szUnk_4C59D0, g_szUnk_4B6D58);
	}
	if ((dwFlags & 0x20) != 0)
	{
		wsprintfA(g_szUnk_4C59D0, "%d",
			g_pUnk_4B6248->dwUnk1B0 != (DWORD)Hover_sub_439C20 ?
				2 : 1);
		WritePrivateProfileStringA(g_pUnk_4B6248->szUnk1C, "Remix",
			g_szUnk_4C59D0, g_szUnk_4B6D58);
	}
	if ((dwFlags & 0x40) != 0)
	{
		wsprintfA(g_szUnk_4C59D0, "%d",
			g_pUnk_4B6248->dwUnk1A4 >= 1);
		WritePrivateProfileStringA(g_pUnk_4B6248->szUnk1C,
			"GoodWavePos", g_szUnk_4C59D0, g_szUnk_4B6D58);
	}
	if ((dwFlags & 0x100) != 0)
	{
		wsprintfA(g_szUnk_4C59D0, "%d", g_dwUnk_4C59B8 >= 1);
		WritePrivateProfileStringA("general", "ShowDebugDialogs",
			g_szUnk_4C59D0, g_szUnk_4B6D58);
	}
	if ((dwFlags & 0x200) != 0)
	{
		wsprintfA(g_szUnk_4C59D0, "%d", g_pUnk_4B6248->dwUnk194);
		WritePrivateProfileStringA(g_pUnk_4B6248->szUnk1C,
			"PauseBlocks", g_szUnk_4C59D0, g_szUnk_4B6D58);
	}
}

// WaveMix driver setup: selects the waveOut device (the config struct's
// device field when flag bit 0x80 is set, otherwise the "WaveOutDevice"
// ini entry), builds the device's ini section name, then reads the
// per-device mixing parameters from the ini file (defaults come from the
// "default" section) and configures the session's output format and wave
// blocks. Returns 1 on success, 0 on failure.

// hover.exe:0x43D3B0
// FUNCTION: HOVER 0x43d3b0
int Hover_sub_43D3B0(void* pv)
{
	UINT nDevs;
	int nRemix;
	int nGoodWavePos;
	int nWaveBlocks;
	int nWaveBlockLen;
	int nSamplesPerSec;
	DWORD pfnRemix;
	BOOL bUnknown;

	nDevs = waveOutGetNumDevs();
	if (nDevs == 0)
		return 0;
	if ((((BYTE*)pv)[3] & 4) != 0 && Hover_sub_43BB20(pv) != 0)
	{
		Hover_sub_43C8F0(g_pUnk_4B6248, 0, pv, 0, 0);
		return 1;
	}
	if ((((BYTE*)pv)[2] & 0x80) != 0)
		g_pUnk_4B6248->dwUnk18 = *(WORD*)((BYTE*)pv + 0x14);
	else
		g_pUnk_4B6248->dwUnk18 = GetPrivateProfileIntA("general",
			"WaveOutDevice", 0, g_szUnk_4B6D58);
	if (g_pUnk_4B6248->dwUnk18 >= nDevs)
		g_pUnk_4B6248->dwUnk18 = 0;
	bUnknown = (waveOutGetDevCapsA(g_pUnk_4B6248->dwUnk18,
			&g_pUnk_4B6248->capsUnk7C, sizeof(WAVEOUTCAPSA)) != 0 ||
		Hover_sub_43B880(g_pUnk_4B6248->capsUnk7C.szPname) == 0);
	if (bUnknown)
		lstrcpyA(g_pUnk_4B6248->capsUnk7C.szPname, "Unkown Device");
	if (Hover_sub_43BA10(g_pUnk_4B6248->dwUnk18, g_pUnk_4B6248->szUnk1C,
			sizeof(g_pUnk_4B6248->szUnk1C)) == 0)
	{
		lstrcpyA(g_pUnk_4B6248->szUnk1C, Hover_sub_43B9C0());
		lstrcatA(g_pUnk_4B6248->szUnk1C, g_pUnk_4B6248->capsUnk7C.szPname);
	}
	nRemix = GetPrivateProfileIntA("default", "Remix", 1,
		g_szUnk_4B6D58);
	nGoodWavePos = GetPrivateProfileIntA("default", "GoodWavePos", 0,
		g_szUnk_4B6D58);
	nWaveBlocks = GetPrivateProfileIntA("default", "WaveBlocks", 3,
		g_szUnk_4B6D58);
	nWaveBlockLen = GetPrivateProfileIntA("default", "WaveBlockLen", 0,
		g_szUnk_4B6D58) & 0xFFFC;
	nSamplesPerSec = GetPrivateProfileIntA("default", "SamplesPerSec", 0xB,
		g_szUnk_4B6D58);
	if ((g_pUnk_4B6248->capsUnk7C.dwSupport & WAVECAPS_SYNC) != 0)
	{
		if (g_dwUnk_4C59B8 != 0)
		{
			wsprintfA(g_szUnk_4C59D0,
				"%s is a syncronous (blocking) wave output device.  "
				"This will not permit audio to play while other "
				"applications are running.",
				g_pUnk_4B6248->capsUnk7C.szPname);
			MessageBoxA(NULL, g_szUnk_4C59D0, "WavMix32",
				MB_ICONINFORMATION);
		}
		return 0;
	}
	if (GetPrivateProfileIntA("not compatible", g_pUnk_4B6248->szUnk1C, 0,
			g_szUnk_4B6D58) != 0)
	{
		if (g_dwUnk_4C59B8 != 0)
		{
			wsprintfA(g_szUnk_4C59D0,
				"%s is not compatible with the realtime wavemixer.",
				g_pUnk_4B6248->szUnk1C);
			MessageBoxA(NULL, g_szUnk_4C59D0, "WavMix32",
				MB_ICONINFORMATION);
		}
		return 0;
	}
	nRemix = GetPrivateProfileIntA(g_pUnk_4B6248->szUnk1C, "Remix", nRemix,
		g_szUnk_4B6D58);
	pfnRemix = (nRemix == 2) ?
		(DWORD)Hover_sub_439BF0 : (DWORD)Hover_sub_439C20;
	g_pUnk_4B6248->dwUnk1B0 = pfnRemix;
	g_pUnk_4B6248->dwUnk1A4 =
		(GetPrivateProfileIntA(g_pUnk_4B6248->szUnk1C, "GoodWavePos",
			nGoodWavePos, g_szUnk_4B6D58) >= 1);
	g_pUnk_4B6248->dwUnk190 =
		GetPrivateProfileIntA(g_pUnk_4B6248->szUnk1C, "WaveBlocks",
			nWaveBlocks, g_szUnk_4B6D58);
	if ((int)g_pUnk_4B6248->dwUnk190 < 2)
		g_pUnk_4B6248->dwUnk190 = 2;
	else if ((int)g_pUnk_4B6248->dwUnk190 > 0x14)
		g_pUnk_4B6248->dwUnk190 = 0x14;
	g_pUnk_4B6248->dwUnk194 =
		GetPrivateProfileIntA(g_pUnk_4B6248->szUnk1C, "PauseBlocks",
			g_pUnk_4B6248->dwUnk190, g_szUnk_4B6D58);
	if ((int)g_pUnk_4B6248->dwUnk194 < 0)
		g_pUnk_4B6248->dwUnk194 = 0;
	else if ((int)g_pUnk_4B6248->dwUnk194 >
			(int)g_pUnk_4B6248->dwUnk190)
		g_pUnk_4B6248->dwUnk194 = g_pUnk_4B6248->dwUnk190;
	nSamplesPerSec = GetPrivateProfileIntA(g_pUnk_4B6248->szUnk1C,
		"SamplesPerSec", nSamplesPerSec, g_szUnk_4B6D58);
	g_pUnk_4B6248->wfxUnk17C.wf.nChannels =
		(WORD)GetPrivateProfileIntA(g_pUnk_4B6248->szUnk1C, "Channels",
			g_wfxUnk_4BFE30.wf.nChannels, g_szUnk_4B6D58);
	if (g_pUnk_4B6248->wfxUnk17C.wf.nChannels < 1)
		g_pUnk_4B6248->wfxUnk17C.wf.nChannels = 1;
	else if (g_pUnk_4B6248->wfxUnk17C.wf.nChannels > 2)
		g_pUnk_4B6248->wfxUnk17C.wf.nChannels = 2;
	switch (nSamplesPerSec)
	{
	case 22:
		g_pUnk_4B6248->wfxUnk17C.wf.nSamplesPerSec = 22050;
		g_pUnk_4B6248->wfxUnk17C.wf.nAvgBytesPerSec =
			22050 * g_pUnk_4B6248->wfxUnk17C.wf.nChannels;
		break;
	case 44:
		g_pUnk_4B6248->wfxUnk17C.wf.nSamplesPerSec = 44100;
		g_pUnk_4B6248->wfxUnk17C.wf.nAvgBytesPerSec =
			44100 * g_pUnk_4B6248->wfxUnk17C.wf.nChannels;
		break;
	default:
		g_pUnk_4B6248->wfxUnk17C.wf.nAvgBytesPerSec =
			11025 * g_pUnk_4B6248->wfxUnk17C.wf.nChannels;
		break;
	}
	g_pUnk_4B6248->dwUnk18C = Hover_sub_43B810(nWaveBlockLen,
		(LPWAVEFORMATEX)&g_pUnk_4B6248->wfxUnk17C);
	if (pv != NULL)
		Hover_sub_43C8F0(g_pUnk_4B6248, 0, pv, 0, 0);
	return 1;
}

// WaveMixConfigureInit: creates and initializes a WaveMix mixing session
// (0x1C0 bytes, LocalAlloc LPTR). Copies the caller's SOUND_DRIVER_CFG
// (clamped to its 0x1e-byte size), resolves the "ShowDebugDialogs"
// setting (from the config block when flag bit 0x100 is set, otherwise
// from the ini file), and bails out with a message when no wave output
// device exists. Returns the session object, or NULL on failure.

// hover.exe:0x43D880
// FUNCTION: HOVER 0x43d880
SOUNDENGINE* __stdcall Hover_sub_43D880(SOUND_DRIVER_CFG* pCfg)
{
	SOUND_DRIVER_CFG cfgLocal;
	int i;

	Hover_sub_438820(2, "<WaveMixConfigureInit>");
	memset(&cfgLocal, 0, sizeof(cfgLocal));
	cfgLocal.wUnk00 = sizeof(cfgLocal);
	if (pCfg != NULL)
	{
		WORD cb = pCfg->wUnk00;
		if (cb >= sizeof(cfgLocal))
			cb = sizeof(cfgLocal);
		memcpy(&cfgLocal, pCfg, cb);
	}
	if (g_dwUnk_4C59C8 == 0)
	{
		if (Hover_sub_43DC40(GetModuleHandleA(NULL)) == 0)
			return NULL;			// module init failed
	}
	if ((cfgLocal.dwUnk02 & 0x100) != 0)
	{
		g_dwUnk_4C59B8 = (cfgLocal.wUnk18 != 0);
	}
	else
	{
		g_dwUnk_4C59B8 = (GetPrivateProfileIntA("general",
			"ShowDebugDialogs", 0, g_szUnk_4B6D58) >= 1);
	}
	if (waveOutGetNumDevs() == 0)
	{
		if (g_dwUnk_4C59B8 != 0)
		{
			wsprintfA(g_szUnk_4C59D0,
				"This system does not have a valid wave output device.");
			MessageBoxA(NULL, g_szUnk_4C59D0, "WavMix32",
				MB_ICONINFORMATION);
		}
		return NULL;
	}
	if (GetPrivateProfileIntA("general", "ShowDevices", 0,
			g_szUnk_4B6D58) != 0)
		Hover_sub_43B8F0();			// device list
	g_pUnk_4B6248 = (SOUNDENGINE*)LocalAlloc(LPTR, sizeof(SOUNDENGINE));
	if (g_pUnk_4B6248 == NULL)
		return NULL;
	g_pUnk_4B6248->wMagic1BC = 0x5432;
	g_pUnk_4B6248->wMagic00 = g_pUnk_4B6248->wMagic1BC;
	g_pUnk_4B6248->dwUnk198 = 0;
	g_pUnk_4B6248->dwUnk14 = 0;
	g_pUnk_4B6248->wUnkB0 = 0xa;
	g_pUnk_4B6248->wUnkB2 = 0xa;
	g_pUnk_4B6248->dwUnk1AC = g_dwUnk_4B6F14;
	for (i = 0; i < 0x10; i++)
		g_pUnk_4B6248->adwUnkF4[i] = 0xFFFFFFFF;
	memmove(&g_pUnk_4B6248->wfxUnk17C, &g_wfxUnk_4BFE30, 0x10);
	if (Hover_sub_43D3B0(&cfgLocal) == 0)
	{
		g_pUnk_4B6248->wMagic1BC = 0;		// driver setup failed
		g_pUnk_4B6248->wMagic00 = g_pUnk_4B6248->wMagic1BC;
		LocalFree(g_pUnk_4B6248);
		g_pUnk_4B6248 = NULL;
	}
	return g_pUnk_4B6248;
}

// WaveMixCloseChannel: validates the session (same magic-word check and
// error reporting as Hover_sub_4393A0), flushes the channel(s) via
// Hover_sub_43A5A0 with flag bit 2 set, then frees them: flag bit 1
// closes all 16 channels, otherwise only channel nChannel. Returns 0 on
// success, 5 = invalid session, or Hover_sub_43A5A0's error code.

// hover.exe:0x43A760
// FUNCTION: HOVER 0x43a760
int __stdcall Hover_sub_43A760(SOUNDENGINE* pEngine, int nChannel,
	int nFlags)
{
	int nResult;
	int i;
	int nEnd;

	Hover_sub_438820(2, "<WaveMixCloseChannel>");
	if (pEngine != NULL && pEngine->wMagic00 == 0x5432 &&
		pEngine->wMagic1BC == 0x5432)
	{
		g_pUnk_4B6248 = pEngine;
	}
	else
	{
		MessageBeep(0xFFFFFFFF);
		wsprintfA(g_szUnk_4C59D0,
			"Invalid session handle 0x%04X passed to WaveMix API",
			pEngine);
		Hover_sub_438820(1, g_szUnk_4C59D0);
		MessageBoxA(NULL, g_szUnk_4C59D0, "WavMix32",
			MB_ICONEXCLAMATION);
		g_pUnk_4B6248 = NULL;
	}
	if (g_pUnk_4B6248 == NULL)
		return 5;
	nResult = Hover_sub_43A5A0(pEngine, nChannel, nFlags | 2);
	if (nResult != 0)					// channel flush
		return nResult;
	if ((nFlags & 1) != 0)
	{
		nChannel = 0;
		nEnd = 0x10;
	}
	else
	{
		nEnd = nChannel + 1;
	}
	for (i = nChannel; i < nEnd; i++)
	{
		if (g_pUnk_4B6248->adwUnkF4[i] != 0xFFFFFFFF)
		{
			g_pUnk_4B6248->adwUnkF4[i] = 0xFFFFFFFF;
			g_pUnk_4B6248->dwUnk134--;
		}
	}
	return 0;
}

// WaveMixCloseSession: validates the session (same magic-word check and
// error reporting as Hover_sub_4393A0), deactivates it and closes all of
// its channels, zeroes the 0x1C0-byte session object and LocalFree()s
// it. Returns 0 on success, 5 on failure.

// hover.exe:0x43DAB0
// FUNCTION: HOVER 0x43dab0
int __stdcall Hover_sub_43DAB0(SOUNDENGINE* pEngine)
{
	Hover_sub_438820(2, "<WaveMixCloseSession>");
	if (pEngine != NULL && pEngine->wMagic00 == 0x5432 &&
		pEngine->wMagic1BC == 0x5432)
	{
		g_pUnk_4B6248 = pEngine;
	}
	else
	{
		MessageBeep(0xFFFFFFFF);
		wsprintfA(g_szUnk_4C59D0,
			"Invalid session handle 0x%04X passed to WaveMix API",
			pEngine);
		Hover_sub_438820(1, g_szUnk_4C59D0);
		MessageBoxA(NULL, g_szUnk_4C59D0, "WavMix32",
			MB_ICONEXCLAMATION);
		g_pUnk_4B6248 = NULL;
	}
	if (g_pUnk_4B6248 == NULL)
		return 5;
	Hover_sub_439180(pEngine, 0);			// session deactivate
	Hover_sub_43A760(pEngine, 0, 1);
	memset(g_pUnk_4B6248, 0, sizeof(SOUNDENGINE));
	g_pUnk_4B6248 = NULL;
	if (pEngine != NULL && LocalFree(pEngine) == NULL)
		return 0;
	return 5;
}

// WaveMixActivate: validates the session (same magic-word check and
// error reporting as Hover_sub_4393A0), then activates or inactivates
// it. Activating claims the global active session, stops any playing
// sound, initializes the wave driver and queues empty wave blocks until
// the queue is full. Inactivating stops the mixer and releases the
// active session. Returns 0 on success, 4 when another session is
// active, 0xc when the driver is in use and 5 on an invalid session.

// hover.exe:0x439180
// FUNCTION: HOVER 0x439180
int __stdcall Hover_sub_439180(SOUNDENGINE* pEngine, int bActivate)
{
	DWORD** ppNode;
	DWORD* pNode;
	int nCount;
	int i;
	int nErr;

	Hover_sub_438820(2, "<WaveMixActivate>");
	if (pEngine != NULL && pEngine->wMagic00 == 0x5432 &&
		pEngine->wMagic1BC == 0x5432)
	{
		g_pUnk_4B6248 = pEngine;
	}
	else
	{
		MessageBeep(0xFFFFFFFF);
		wsprintfA(g_szUnk_4C59D0,
			"Invalid session handle 0x%04X passed to WaveMix API",
			pEngine);
		Hover_sub_438820(1, g_szUnk_4C59D0);
		MessageBoxA(NULL, g_szUnk_4C59D0, "WavMix32",
			MB_ICONEXCLAMATION);
		g_pUnk_4B6248 = NULL;
	}
	if (g_pUnk_4B6248 == NULL)
		return 5;
	if (bActivate)
	{
		Hover_sub_438820(2, "Activate");
		if (g_pUnk_4B6F18 != NULL)
			return (g_pUnk_4B6F18 == g_pUnk_4B6248) ? 0 : 4;
		if (g_pUnk_4B6248->dwUnk14 != 0)
			return 0xc;
		g_pUnk_4B6F18 = g_pUnk_4B6248;
		sndPlaySoundA(NULL, 0);
		nErr = Hover_sub_439010();			// driver init
		if (nErr != 0)
		{
			g_pUnk_4B6F18 = NULL;
			return nErr;
		}
		g_pUnk_4B6248->dwUnk10 = 1;
		Hover_sub_438900(g_pUnk_4B6248->dwUnk19C);
		for (;;)
		{
			// The scan walks the block pointer (add eax,4) and keeps
			// the "i < nCount" guard on the counter.
			i = 0;
			nCount = g_pUnk_4B6248->dwUnk190;
			ppNode = (DWORD**)g_pUnk_4B6248->dwUnk198;
			while (i < nCount && (*ppNode)[0x20 / 4] == 0)
			{
				ppNode++;
				i++;
			}
			if (i >= nCount)
			{
				Hover_sub_438820(2, "{gwb:out of wav blks}");
				pNode = NULL;
			}
			else
			{
				pNode = *ppNode;
				pNode[1] = g_pUnk_4B6248->dwUnk18C;
				pNode[0] = (DWORD)pNode + 0x30;
				pNode[0x28 / 4] = (DWORD)g_pUnk_4B6F18;
				pNode[0x20 / 4] = 0;
			}
			nErr = Hover_sub_439560(pNode, 1);	// queue block
			if (nErr == 0)
				break;
		}
		return 0;
	}
	Hover_sub_438820(2, "Inactivate");
	if (g_pUnk_4B6248->dwUnk10 != 0)
		g_pUnk_4B6248->dwUnk19C = Hover_sub_438840(
			g_pUnk_4B6248->dwUnk0C, g_pUnk_4B6248->dwUnk1A4);
	Hover_sub_438F50(g_pUnk_4B6248);
	g_pUnk_4B6248->dwUnk10 = 0;
	if (g_pUnk_4B6F18 == g_pUnk_4B6248)
		g_pUnk_4B6F18 = NULL;
	return 0;
}

// WaveMixFlushChannel: validates the session (same magic-word check and
// error reporting as Hover_sub_4393A0), then returns the queued channel
// nodes of one channel - or of every channel when bit 0 of nFlags is
// set - to the free list. Unless bit 1 of nFlags is set, an active
// session then reports the new position through the session callback.
// Returns 0 on success, 0xb on an invalid channel and 5 on an invalid
// session or unopened channel.

// hover.exe:0x43A5A0
// FUNCTION: HOVER 0x43a5a0
int __stdcall Hover_sub_43A5A0(SOUNDENGINE* pEngine, int nChannel,
	int nFlags)
{
	BOOL bFreed;
	DWORD* pNode;
	DWORD* pNext;
	int i;
	int nEnd;

	bFreed = FALSE;
	Hover_sub_438820(2, "F");
	if (pEngine != NULL && pEngine->wMagic00 == 0x5432 &&
		pEngine->wMagic1BC == 0x5432)
	{
		g_pUnk_4B6248 = pEngine;
	}
	else
	{
		MessageBeep(0xFFFFFFFF);
		wsprintfA(g_szUnk_4C59D0,
			"Invalid session handle 0x%04X passed to WaveMix API",
			pEngine);
		Hover_sub_438820(1, g_szUnk_4C59D0);
		MessageBoxA(NULL, g_szUnk_4C59D0, "WavMix32",
			MB_ICONEXCLAMATION);
		g_pUnk_4B6248 = NULL;
	}
	if (g_pUnk_4B6248 == NULL)
		return 5;
	if (nFlags & 1)
	{
		i = 0;
		nEnd = 0x10;
	}
	else
	{
		i = nChannel;
		if (i < 0 || i >= 0x10)
			return 0xb;
		if (g_pUnk_4B6248->adwUnkF4[i] == 0xFFFFFFFF)
			return 5;
		nEnd = i + 1;
	}
	for (; i < nEnd; i++)
	{
		if (g_pUnk_4B6248->adwUnkF4[i] == 0xFFFFFFFF)
			continue;
		pNode = (DWORD*)g_pUnk_4B6248->adwUnkF4[i];
		g_pUnk_4B6248->adwUnkF4[i] = 0;
		if (pNode == NULL)
			continue;
		bFreed = TRUE;
		for (;;)
		{
			pNext = (DWORD*)pNode[0];
			Hover_sub_438820(2, "{FreeCNode");
			if (pNode == NULL)	// never taken; pNode was
			{				// checked above
				Hover_sub_438820(1,
					" FreeCN: NULL parameter!}");
			}
			else
			{
				pNode[0] = (DWORD)g_pUnk_4B6D50;
				g_pUnk_4B6D50 = pNode;
				Hover_sub_438820(2, "}");
			}
			pNode = pNext;
			if (pNext == NULL)
				break;
		}
	}
	if (bFreed && (nFlags & 2) == 0 && g_pUnk_4B6248->dwUnk10 != 0)
	{
		// The queued position is computed inside the argument list: the
		// original pushes the 0 first, then evaluates Hover_sub_438840.
		((void (*)(int, int))g_pUnk_4B6248->dwUnk1B0)(
			Hover_sub_438840(g_pUnk_4B6248->dwUnk0C, g_pUnk_4B6248->dwUnk1A4),
			0);
	}
	return 0;
}

// WaveMixShowDevices: debug helper that pops up a message box with the
// number of waveOut devices on the system, then one message box per
// device with its name and driver version (or a failure notice).

// hover.exe:0x43B8F0
// FUNCTION: HOVER 0x43b8f0
void Hover_sub_43B8F0()
{
	WAVEOUTCAPSA caps;
	int i;
	int numDevs;
	int nErr;

	numDevs = waveOutGetNumDevs();
	if (numDevs == 0)
		return;
	wsprintfA(g_szUnk_4C59D0,
		"%d waveOut Devices have been detected on your system.",
		numDevs);
	MessageBoxA(NULL, g_szUnk_4C59D0, "WavMix32", MB_ICONINFORMATION);
	for (i = 0; i < numDevs; i++)
	{
		if (waveOutGetDevCapsA(i, &caps, sizeof(caps)) != 0 ||
			Hover_sub_43B880(caps.szPname) == 0)	// the
		{					// original reports
			nErr = 1;			// "failed (err 1)"
		}					// on this path
		else
			nErr = 0;
		if (nErr != 0)
			wsprintfA(g_szUnk_4C59D0,
				"waveOutGetDevCaps failed (err %u) for device %d",
				nErr, i);
		else
			wsprintfA(g_szUnk_4C59D0,
				"Device %i: %s\n\tVersion %u.%u",
				i, caps.szPname, HIBYTE(caps.vDriverVersion),
				LOBYTE(caps.vDriverVersion));
		MessageBoxA(NULL, g_szUnk_4C59D0, "WavMix32",
			MB_ICONINFORMATION);
	}
}

// WaveMix position query: when dwType is nonzero the position comes
// from waveOutGetPosition(), adjusted through the Add/Sub function
// selected by Hover_sub_438900; otherwise it is estimated from the
// timeGetTime() milliseconds elapsed since the origin at +0x1A0,
// converted to bytes and rounded down to a multiple of 8.

// hover.exe:0x438840
// FUNCTION: HOVER 0x438840
int Hover_sub_438840(DWORD dwSource, DWORD dwType)
{
	MMTIME mmt;

	if (dwType != 0)
	{
		mmt.wType = TIME_BYTES;
		waveOutGetPosition((HWAVEOUT)dwSource, &mmt, sizeof(mmt));
		return ((int (*)(int, int))g_pUnk_4B6248->dwUnk1B4)(mmt.u.cb,
			g_pUnk_4B6248->dwUnk1A8);
	}
	Hover_sub_438820(3, "{mywaveoutgetpos !fGoodGetPos}");
	return (timeGetTime() - g_pUnk_4B6248->dwUnk1A0) *
		g_pUnk_4B6248->wfxUnk17C.wf.nAvgBytesPerSec / 1000 & 0xFFFFFFF8;
}

// Position adjustment helpers; Hover_sub_438900 points the +0x1B4
// function pointer at whichever one matches the adjustment direction.

// hover.exe:0x4388C0
// FUNCTION: HOVER 0x4388c0
int Hover_sub_4388C0(int nA, int nB)
{
	Hover_sub_438820(2, "Add");
	return nA + nB;
}

// hover.exe:0x4388E0
// FUNCTION: HOVER 0x4388e0
int Hover_sub_4388E0(int nA, int nB)
{
	Hover_sub_438820(2, "Sub");
	return nA - nB;
}

// SetWaveOutPositon (sic): re-bases the position tracking onto dwPos.
// The delta between the device's current position and dwPos is stored
// at +0x1A8 with the matching Add/Sub adjustment function at +0x1B4,
// and the timeGetTime() origin at +0x1A0 is moved back by the dwPos
// equivalent in milliseconds.

// hover.exe:0x438900
// FUNCTION: HOVER 0x438900
void Hover_sub_438900(DWORD dwPos)
{
	MMTIME mmt;

	Hover_sub_438820(2, "SetWaveOutPositon");
	wsprintfA(g_szUnk_4C59D0, "=%lu", dwPos);
	Hover_sub_438820(3, g_szUnk_4C59D0);
	mmt.wType = TIME_BYTES;
	if (g_pUnk_4B6248->dwUnk0C != 0)
		waveOutGetPosition((HWAVEOUT)g_pUnk_4B6248->dwUnk0C, &mmt,
			sizeof(mmt));
	else
		mmt.u.cb = 0;
	if (mmt.u.cb >= dwPos)
	{
		g_pUnk_4B6248->dwUnk1A8 = mmt.u.cb - dwPos;
		g_pUnk_4B6248->dwUnk1B4 = (DWORD)Hover_sub_4388E0;
	}
	else
	{
		g_pUnk_4B6248->dwUnk1A8 = dwPos - mmt.u.cb;
		g_pUnk_4B6248->dwUnk1B4 = (DWORD)Hover_sub_4388C0;
	}
	g_pUnk_4B6248->dwUnk1A0 = timeGetTime() - dwPos * 1000 /
		g_pUnk_4B6248->wfxUnk17C.wf.nAvgBytesPerSec;
	g_pUnk_4B6248->dwUnk19C = dwPos;
}

// WaveMix "cmixit" mixing function (assigned to the session's dwUnk1AC
// by Hover_sub_43DC40 and by Hover_sub_43C8F0 when the config's Remix
// field selects it). Mixes nSrcs 8-bit sources into pBuf for nCount
// output bytes, applying the per-channel volume pairs in pwVols through
// the g_aUnk_4B6250 table. Stereo output (session format nChannels == 2)
// consumes/produces two bytes per sample frame; with a single source the
// table output is used directly, with several sources the centered
// (0x80-based) values are summed and clamped to 0..0xFF.

// hover.exe:0x4389F0
// FUNCTION: HOVER 0x4389f0
void Hover_sub_4389F0(BYTE* pBuf, DWORD** ppSrcs, WORD* pwVols, int nSrcs,
	WORD nCount)
{
	BYTE* pSrc;
	DWORD** ppSrc;
	WORD* pwVol;
	WORD wVol;
	WORD wVolL;
	WORD wVolR;
	WORD wOff;
	WORD wFrames;
	int nAccL;
	int nAccR;
	int i;

	if (g_pUnk_4B6248->wfxUnk17C.wf.nChannels == 2)
	{
		if (nSrcs == 1)
		{
			// Both channel volumes are read once, before the count
			// test, in the original.
			pSrc = (BYTE*)ppSrcs[0];
			wVolL = pwVols[0];
			wVolR = pwVols[1];
			if (nCount != 0)
			{
				wFrames = (WORD)(nCount + 1) >> 1;
				do
				{
					*pBuf++ = g_aUnk_4B6250[wVolL][*pSrc++];
					*pBuf++ = g_aUnk_4B6250[wVolR][*pSrc++];
					wFrames--;
				} while (wFrames != 0);
			}
			return;
		}
		wOff = 0;
		if (nCount == 0)
			return;
		wFrames = (WORD)(nCount + 1) >> 1;
		do
		{
			nAccL = 0x80;
			nAccR = 0x80;
			if (nSrcs > 0)
			{
				pwVol = pwVols;
				ppSrc = ppSrcs;
				i = nSrcs;
				do
				{
					pSrc = (BYTE*)*ppSrc + wOff;
					nAccL += g_aUnk_4B6250[pwVol[0]]
						[pSrc[0]] - 0x80;
					nAccR += g_aUnk_4B6250[pwVol[1]]
						[pSrc[1]] - 0x80;
					pwVol += 2;
					ppSrc++;
					i--;
				} while (i != 0);
			}
			if (nAccL < 0)
				nAccL = 0;
			else if (nAccL > 0xFF)
				nAccL = 0xFF;
			if (nAccR < 0)
				nAccR = 0;
			else if (nAccR > 0xFF)
				nAccR = 0xFF;
			wOff += 2;
			*pBuf++ = (BYTE)nAccL;
			*pBuf++ = (BYTE)nAccR;
			wFrames--;
		} while (wFrames != 0);
		return;
	}
	if (nSrcs == 1)
	{
		pSrc = (BYTE*)ppSrcs[0];
		wVol = (WORD)((pwVols[0] + pwVols[1]) / 2);
		wFrames = nCount;
		if (wFrames != 0)
		{
			do
			{
				*pBuf++ = g_aUnk_4B6250[wVol][*pSrc++];
				wFrames--;
			} while (wFrames != 0);
		}
		return;
	}
	wOff = 0;
	if (nCount == 0)
		return;
	wFrames = nCount;
	do
	{
		nAccL = 0x80;
		if (nSrcs > 0)
		{
			pwVol = pwVols + 1;
			ppSrc = ppSrcs;
			i = nSrcs;
			do
			{
				wVol = (WORD)((pwVol[-1] + pwVol[0]) / 2);
				pSrc = (BYTE*)*ppSrc;
				nAccL += g_aUnk_4B6250[wVol][pSrc[wOff]] - 0x80;
				ppSrc++;
				pwVol += 2;
				i--;
			} while (i != 0);
		}
		if (nAccL < 0)
			nAccL = 0;
		else if (nAccL > 0xFF)
			nAccL = 0xFF;
		wOff++;
		*pBuf++ = (BYTE)nAccL;
		wFrames--;
	} while (wFrames != 0);
}

// Links the g_adwUnk_4B4A58 channel-node pool into a singly-linked free
// list through each node's first dword, then points the free-list head
// at the pool. Called once by Hover_sub_43DC40.

// hover.exe:0x438CA0
// FUNCTION: HOVER 0x438ca0
void Hover_sub_438CA0()
{
	DWORD* pNext;
	DWORD* pNode;

	pNode = g_adwUnk_4B4A58;
	do
	{
		pNext = pNode + 0x3C / 4;
		pNode[0] = (DWORD)pNext;
		pNode = pNext;
	} while (pNode < g_adwUnk_4B4A58 + 98 * (0x3C / 4));
	g_adwUnk_4B4A58[99 * (0x3C / 4)] = 0;
	g_pUnk_4B6D50 = (void*)g_adwUnk_4B4A58;
}

// WaveMix output shutdown: resets the device, re-bases the position
// tracking, frees every queued done-block, unprepares both wave block
// arrays and closes the device and its callback window.

// hover.exe:0x438F50
// FUNCTION: HOVER 0x438f50
void Hover_sub_438F50(SOUNDENGINE* pEngine)
{
	DWORD dwPos;
	DWORD hWaveOut;

	if (pEngine->dwUnk10 == 0)
		return;
	hWaveOut = pEngine->dwUnk0C;
	if (hWaveOut == 0)
		return;
	dwPos = Hover_sub_438840(hWaveOut, g_pUnk_4B6248->dwUnk1A4);
	waveOutReset((HWAVEOUT)hWaveOut);
	Hover_sub_438900(dwPos);
	if (g_pUnk_4C59C0 != NULL)
	{
		do
		{
			((DWORD*)g_pUnk_4C59C0)[0x20 / 4] = 1;
			Hover_sub_438E90(g_pUnk_4C59C0);	// free block
		} while (g_pUnk_4C59C0 != NULL);
	}
	Hover_sub_438D00(pEngine->dwUnk0C, g_adwUnk_4B6E70);
	Hover_sub_438D00(pEngine->dwUnk0C, g_adwUnk_4B6EC0);
	waveOutClose((HWAVEOUT)pEngine->dwUnk0C);
	pEngine->dwUnk0C = 0;
	DestroyWindow(pEngine->hwndUnk04);
	pEngine->hwndUnk04 = NULL;
}

// WaveMix channel sweeper ("{F..._got_"): walks every open channel's
// node queue and frees the nodes whose end position has been reached,
// posting MM_WOM_DONE to the node's notification window when it has one.
// When position tracking is not device-based and the done-block queue is
// empty, it also posts the private 0x400 message to the callback window
// for every open channel. Called by the Hover_sub_439A80 pump.

// hover.exe:0x439900
// FUNCTION: HOVER 0x439900
void Hover_sub_439900()
{
	DWORD* pNode;
	int i;
	DWORD dwPos;

	Hover_sub_438820(2, "{F");
	dwPos = Hover_sub_438840(g_pUnk_4B6248->dwUnk0C,
		g_pUnk_4B6248->dwUnk1A4);
	i = 0;
	Hover_sub_438820(2, "_got_");
	for (; i < 0x10; i++)
	{
		pNode = (DWORD*)g_pUnk_4B6248->adwUnkF4[i];
		if (pNode == NULL || pNode == (DWORD*)0xFFFFFFFF)
			continue;
		Hover_sub_438820(2, ".");
		while (pNode != NULL)
		{
			if (pNode[0x2C / 4] > dwPos)
				break;
			Hover_sub_438820(2, "<!");
			g_pUnk_4B6248->adwUnkF4[i] = pNode[0];
			if (*(DWORD*)((BYTE*)pNode + 0x12) != 0)
				PostMessageA((HWND)*(DWORD*)((BYTE*)pNode + 0x12),
					MM_WOM_DONE, i, pNode[0x20 / 4]);
			Hover_sub_438820(2, "{FreeCNode");
			if (pNode == NULL)	// never taken; pNode was
			{				// checked above
				Hover_sub_438820(1,
					" FreeCN: NULL parameter!}");
			}
			else
			{
				pNode[0] = (DWORD)g_pUnk_4B6D50;
				g_pUnk_4B6D50 = pNode;
				Hover_sub_438820(2, "}");
			}
			pNode = (DWORD*)g_pUnk_4B6248->adwUnkF4[i];
			Hover_sub_438820(2, ">");
		}
	}
	if (g_pUnk_4B6248->dwUnk1A4 == 0 && g_pUnk_4C59C0 == NULL)
	{
		for (i = 0; i < 0x10; i++)
		{
			if (g_pUnk_4B6248->adwUnkF4[i] != 0 &&
				g_pUnk_4B6248->adwUnkF4[i] != 0xFFFFFFFF)
			{
				PostMessageA(g_pUnk_4B6248->hwndUnk04, 0x400, 0,
					(LPARAM)g_pUnk_4B6248);
			}
		}
	}
	Hover_sub_438820(2, "}");
}

// WaveMix output initialization: creates the hidden callback window,
// opens the waveOut device with window callbacks and prepares both
// wave block pointer arrays. Returns 0 on success, the waveOutOpen()
// error code, or 1 on window/preparation failure.

// hover.exe:0x439010
// FUNCTION: HOVER 0x439010
int Hover_sub_439010()
{
	int nErr;

	if (g_pUnk_4B6248->dwUnk0C != 0)
		goto success;
	g_pUnk_4B6248->hwndUnk04 = CreateWindowExA(0, "WavMix32", "",
		WS_DISABLED, 0, 0, 0, 0, NULL, NULL, g_hUnk_4B6F10, NULL);
	if (g_pUnk_4B6248->hwndUnk04 == NULL)
	{
		if (g_dwUnk_4C59B8 != 0)
			MessageBoxA(NULL, "Failed to create callback window.",
				"WavMix32", MB_ICONEXCLAMATION);
		return 1;
	}
	nErr = waveOutOpen((HWAVEOUT*)&g_pUnk_4B6248->dwUnk0C,
		g_pUnk_4B6248->dwUnk18,
		(LPCWAVEFORMATEX)&g_pUnk_4B6248->wfxUnk17C,
		(DWORD)g_pUnk_4B6248->hwndUnk04, 0, CALLBACK_WINDOW);
	if (nErr != 0)
	{
		DestroyWindow(g_pUnk_4B6248->hwndUnk04);
		g_pUnk_4B6248->hwndUnk04 = NULL;
		return nErr;
	}
	if (Hover_sub_438D60(g_pUnk_4B6248->dwUnk0C,	// prepare
		g_adwUnk_4B6E70) == 0)
		goto fail;
	if (Hover_sub_438D60(g_pUnk_4B6248->dwUnk0C,	// prepare
		g_adwUnk_4B6EC0) == 0)
		goto fail;
	g_pUnk_4B6248->dwUnk198 = (DWORD)g_adwUnk_4B6E70;
success:
	return 0;
fail:
	Hover_sub_438D00(g_pUnk_4B6248->dwUnk0C, g_adwUnk_4B6E70);
	Hover_sub_438D00(g_pUnk_4B6248->dwUnk0C, g_adwUnk_4B6EC0);
	waveOutClose((HWAVEOUT)g_pUnk_4B6248->dwUnk0C);
	g_pUnk_4B6248->dwUnk0C = 0;
	DestroyWindow(g_pUnk_4B6248->hwndUnk04);
	return 1;
}

// WaveMix mixer pump ("{MP}"): fills the wave block's buffer with up
// to +0x18C bytes - silence (0x80) where no channel has data, mixed
// channel data through the +0x1AC mixing function elsewhere - then
// submits the block with waveOutWrite(). When bQueue is set the block
// is also linked onto the done-block queue; when the block cannot be
// submitted its +0x20 in-use flag is set and it is freed through
// Hover_sub_438E90. Returns 1 when the block was submitted, 0 when
// there was nothing to mix.

// hover.exe:0x439560
// FUNCTION: HOVER 0x439560
int Hover_sub_439560(DWORD* pNode, int bQueue)
{
	UINT i;			// unsigned: the channel loop ends on an unsigned jb
	DWORD** ppTab;
	DWORD** ppEnd;
	DWORD** ppSrc;
	DWORD* pChNode;
	WORD* pwVol;
	BYTE* pBuf;
	DWORD dwMin;
	DWORD dwEnd;
	DWORD dwMixLen;
	DWORD dwPos;
	DWORD dwOff;
	DWORD dwLoopLen;
	int nCount;
	int nSrcs;
	WORD wRemain;

	Hover_sub_438820(2, "{MP}");
	if (pNode == NULL)
	{
		Hover_sub_438820(2, "{lpXWH=NULL:return F}");
		return 0;
	}
	dwMin = 0xFFFFFFFF;
	nCount = 0;
	ppTab = (DWORD**)g_apUnk_4B61C8;
	for (i = 0; i < 0x10; i++)
	{
		pChNode = (DWORD*)g_pUnk_4B6248->adwUnkF4[i];
		if (pChNode == (DWORD*)0xFFFFFFFF || pChNode == NULL)
			continue;
		for (;;)
		{
			if (g_pUnk_4B6248->dwUnk19C < pChNode[0x2C / 4])
				break;
			Hover_sub_438820(3, "{MixerPlay:skipping}");
			pChNode = (DWORD*)*pChNode;
			if (pChNode == NULL)
				break;
		}
		if (pChNode == NULL)
			continue;
		if (dwMin > pChNode[0x28 / 4])
			dwMin = pChNode[0x28 / 4];
		*ppTab = pChNode;
		ppTab++;
		nCount++;
	}
	if (nCount == 0)
	{
		Hover_sub_438820(2, "{MP:uMaxChannel==0,ret F}");
		if (bQueue != 0)
			pNode[0x20 / 4] = 1;
		return 0;
	}
	pBuf = (BYTE*)pNode[0];
	dwPos = g_pUnk_4B6248->dwUnk19C;
	wRemain = (WORD)g_pUnk_4B6248->dwUnk18C;
	if (wRemain != 0)
	{
		do
		{
			Hover_sub_438820(2, "+");
			if (dwMin > dwPos)
			{
				// silence up to the next node start
				dwMixLen = wRemain;
				if (dwPos + dwMixLen >= dwMin)
					dwMixLen = dwMin - dwPos;
				memset(pBuf, 0x80, dwMixLen);
				wRemain -= (WORD)dwMixLen;
				dwPos += dwMixLen;
				pBuf += dwMixLen;
			}
			else
			{
				nSrcs = 0;
				dwEnd = dwPos + wRemain;
				if (nCount != 0)
				{
					pwVol = g_awUnk_4B6F20;
					ppSrc = (DWORD**)g_apUnk_4B6208;
					ppTab = (DWORD**)g_apUnk_4B61C8;
					i = nCount;
					do
					{
						pChNode = *ppTab;
						if (dwPos < pChNode[0x28 / 4])
						{
							if (dwEnd > pChNode[0x28 / 4])
								dwEnd = pChNode[0x28 / 4];
						}
						else
						{
							if (dwEnd > pChNode[0x2C / 4])
								dwEnd = pChNode[0x2C / 4];
							dwOff = dwPos - pChNode[0x28 / 4];
							if (((WORD*)pChNode)[0x1A / 2] != 0)
							{
								dwLoopLen = pChNode[0x24 / 4];
								dwOff %= dwLoopLen;
								if (dwEnd > dwLoopLen -
									dwOff + dwPos)
									dwEnd = dwLoopLen -
										dwOff + dwPos;
							}
							nSrcs++;
							*ppSrc = (DWORD*)(pChNode[0x30 / 4] +
								dwOff);
							ppSrc++;
							pwVol[0] =
								((WORD*)pChNode)[0x38 / 2];
							pwVol[1] =
								((WORD*)pChNode)[0x3A / 2];
							pwVol += 2;
						}
						ppTab++;
						i--;
					} while (i != 0);
				}
				if (nSrcs == 0)
				{
					Hover_sub_438820(2,
						"***No wave data has been setup!!!!");
				}
				else
				{
					dwMixLen = dwEnd - dwPos;
					((void (*)(BYTE*, DWORD**, WORD*, int, int))
						g_pUnk_4B6248->dwUnk1AC)(pBuf,
						(DWORD**)g_apUnk_4B6208,
						g_awUnk_4B6F20, nSrcs, dwMixLen);
					dwPos += dwMixLen;
					wRemain -= (WORD)dwMixLen;
					pBuf += dwMixLen;
					dwMin = 0xFFFFFFFF;
					i = 0;
					if (nCount != 0)
					{
						ppTab = (DWORD**)g_apUnk_4B61C8;
						ppEnd = (DWORD**)g_apUnk_4B61C8 + nCount;
						do
						{
							for (;;)
							{
								if (*ppTab == NULL)
									break;
								pChNode = *ppTab;
								if (pChNode[0x2C / 4] > dwPos)
									break;
								pChNode = (DWORD*)*pChNode;
								*ppTab = pChNode;
								if (pChNode == NULL)
									break;
							}
							if (*ppTab == NULL)
							{
								pChNode = ppEnd[-1];
								ppEnd--;
								nCount--;
								*ppTab = pChNode;
								if (ppEnd ==
									(DWORD**)g_apUnk_4B61C8)
									break;
							}
							else
							{
								if (dwMin >
									(*ppTab)[0x28 / 4])
									dwMin =
										(*ppTab)[0x28 / 4];
								ppTab++;
								i++;
							}
						} while (i < (UINT)nCount);
					}
				}
			}
		} while (wRemain != 0);
	}
	Hover_sub_438820(2, "mixing done");
	pNode[0x24 / 4] = g_pUnk_4B6248->dwUnk19C;
	g_pUnk_4B6248->dwUnk19C += g_pUnk_4B6248->dwUnk18C;
	if (bQueue != 0)
	{
		pNode[0x2C / 4] = 0;
		if (g_pUnk_4C59C0 == NULL)
		{
			g_pUnk_4C59C4 = pNode;
			g_pUnk_4C59C0 = pNode;
		}
		else
		{
			((DWORD*)g_pUnk_4C59C4)[0x2C / 4] = (DWORD)pNode;
			g_pUnk_4C59C4 = pNode;
		}
		if (waveOutWrite((HWAVEOUT)g_pUnk_4B6248->dwUnk0C,
			(LPWAVEHDR)pNode, sizeof(WAVEHDR)) != 0)
		{
			if (g_dwUnk_4C59B8 != 0)
				MessageBoxA(NULL, "Failed to write block to device",
					"WavMix32", MB_ICONEXCLAMATION);
			pNode[0x20 / 4] = 1;
			Hover_sub_438E90(pNode);		// free block
		}
	}
	Hover_sub_438820(2, "{end MP: TRUE}");
	return 1;
}

// WaveMix device-name filter used by Hover_sub_43B8F0: truncates the
// name in place at the first character that is neither alphanumeric
// nor whitespace, trims trailing whitespace and returns the remaining
// length.

// hover.exe:0x43B880
// FUNCTION: HOVER 0x43b880
int Hover_sub_43B880(char* pszName)
{
	char* p;

	p = pszName;
	if (pszName == NULL || *pszName == '\0')
		return 0;
	do
	{
		if (!isalnum(*p) && !isspace(*p))
			break;
		p++;
	} while (*p != '\0');
	*p = '\0';
	p--;
	while (p >= pszName && isspace(*p))
	{
		*p = '\0';
		p--;
	}
	return lstrlenA(pszName);
}

// RemoveFromPlayingQueue ("{RemoveFPQ"): unlinks a done-block from the
// 0x4C59C0/0x4C59C4 queue and clears its +0x2C link. Logs "h" when the
// block is the head, "." otherwise. Returns the block, or NULL when
// the queue is empty or the block is not in it.

// hover.exe:0x438E90
// FUNCTION: HOVER 0x438e90
void* Hover_sub_438E90(void* pNode)
{
	DWORD* pPrev;
	DWORD* pCur;

	Hover_sub_438820(2, "{RemoveFPQ");
	if (g_pUnk_4C59C0 == NULL)
		return NULL;
	if (pNode == g_pUnk_4C59C0)
	{
		Hover_sub_438820(2, "h");
		g_pUnk_4C59C0 = (void*)((DWORD*)pNode)[0x2C / 4];
		if (g_pUnk_4C59C0 == NULL)
			g_pUnk_4C59C4 = NULL;
	}
	else
	{
		Hover_sub_438820(2, ".");
		pPrev = (DWORD*)g_pUnk_4C59C0;
		pCur = (DWORD*)pPrev[0x2C / 4];
		if (pCur != NULL)
		{
			do
			{
				if ((DWORD*)pNode == pCur)
					break;
				pPrev = pCur;
				pCur = (DWORD*)pCur[0x2C / 4];
			} while (pCur != NULL);
		}
		if (pCur == NULL)
		{
			Hover_sub_438820(1,
				"RemoveFromPlayingQueue: lpXWH is not in Queue!\n\r");
			return NULL;
		}
		pPrev[0x2C / 4] = pCur[0x2C / 4];
		if (g_pUnk_4C59C4 == pCur)
			g_pUnk_4C59C4 = pPrev;
	}
	Hover_sub_438820(2, "}");
	((DWORD*)pNode)[0x2C / 4] = 0;
	return pNode;
}

// Allocates and prepares the 20 wave blocks of one pointer array: each
// block is a 0x2030-byte global allocation whose WAVEHDR points at its
// own +0x30 data area (+0x18C bytes). On failure everything allocated
// so far is released. Returns 1 on success, 0 on failure.

// hover.exe:0x438D60
// FUNCTION: HOVER 0x438d60
int Hover_sub_438D60(DWORD hWaveOut, DWORD* pBlocks)
{
	DWORD* pp;
	DWORD* pBlock;
	HGLOBAL hMem;
	int i;
	int j;

	i = 0;
	pp = pBlocks;
	do
	{
		hMem = GlobalAlloc(GMEM_DDESHARE, 0x2030);
		pBlock = (DWORD*)GlobalLock(hMem);
		*pp = (DWORD)pBlock;
		if (pBlock == NULL)
			goto alloc_fail;
		pBlock[0] = (DWORD)pBlock + 0x30;
		pBlock[1] = g_pUnk_4B6248->dwUnk18C;
		pBlock[0x10 / 4] = 0;
		pBlock[0x14 / 4] = 0;
		pBlock[0x20 / 4] = 1;
		pBlock[0x24 / 4] = 0;
		pp++;
		i++;
	} while (i < 0x14);
	j = 0;
	pp = pBlocks;
	do
	{
		if (waveOutPrepareHeader((HWAVEOUT)hWaveOut, (LPWAVEHDR)*pp,
			sizeof(WAVEHDR)) != 0)
			goto prep_fail;
		(*(DWORD**)pp)[0x10 / 4] |= 1;
		pp++;
		j++;
	} while (j < 0x14);
	return 1;
alloc_fail:
	if (g_dwUnk_4C59B8 != 0)
		MessageBoxA(NULL,
			"Unable to allocate memory for waveform data.  Try "
			"making more memory available by closing other "
			"applications.",
			"WavMix32", MB_ICONEXCLAMATION);
	i--;
	while (i >= 0)
	{
		GlobalUnlock(GlobalHandle((LPCVOID)pBlocks[i]));
		GlobalFree(GlobalHandle((LPCVOID)pBlocks[i]));
		pBlocks[i] = 0;
		i--;
	}
	return 0;
prep_fail:
	if (g_dwUnk_4C59B8 != 0)
		MessageBoxA(NULL, "Unable to prepare wave header.",
			"WavMix32", MB_ICONEXCLAMATION);
	Hover_sub_438D00(hWaveOut, pBlocks);
	return 0;
}

// Unprepares and frees every allocated block of one pointer array
// (always all 20 entries).

// hover.exe:0x438D00
// FUNCTION: HOVER 0x438d00
void Hover_sub_438D00(DWORD hWaveOut, DWORD* pBlocks)
{
	int i;

	i = 0x14;
	do
	{
		if (*pBlocks != 0)
		{
			waveOutUnprepareHeader((HWAVEOUT)hWaveOut,
				(LPWAVEHDR)*pBlocks, sizeof(WAVEHDR));
			GlobalUnlock(GlobalHandle((LPCVOID)*pBlocks));
			GlobalFree(GlobalHandle((LPCVOID)*pBlocks));
			*pBlocks = 0;
		}
		pBlocks++;
		i--;
	} while (i != 0);
}

// WaveMix play-queue pump ("{pump}"): frees the done blocks at the head
// of the 0x4C59C0/0x4C59C4 queue (marked through Hover_sub_438E90),
// sweeps the channel queues with Hover_sub_439900, then fills and queues
// free wave blocks through Hover_sub_439560 until a block comes back
// empty or none are free. Runs on the active session.

// hover.exe:0x439A80
// FUNCTION: HOVER 0x439a80
void Hover_sub_439A80()
{
	DWORD** ppNode;
	DWORD* pNode;
	int nCount;
	int i;

	Hover_sub_438820(3, "{pump}");
	g_pUnk_4B6248 = g_pUnk_4B6F18;
	if (g_pUnk_4B6248 == NULL)
		return;
	pNode = (DWORD*)g_pUnk_4C59C0;
	while (pNode != NULL)
	{
		if (pNode[0x10 / 4] & 1)
		{
			Hover_sub_438E90(pNode);
			pNode[0x20 / 4] = 1;
			pNode = (DWORD*)g_pUnk_4C59C0;
		}
		else
		{
			pNode = (DWORD*)pNode[0x2C / 4];
		}
	}
	Hover_sub_439900();
	for (;;)
	{
		i = 0;
		nCount = g_pUnk_4B6248->dwUnk190;
		ppNode = (DWORD**)g_pUnk_4B6248->dwUnk198;
		while (i < nCount && ppNode[i][0x20 / 4] == 0)
			i++;
		if (i == nCount)
		{
			Hover_sub_438820(2, "{gwb:out of wav blks}");
			pNode = NULL;
		}
		else
		{
			pNode = ppNode[i];
			pNode[1] = g_pUnk_4B6248->dwUnk18C;
			pNode[0] = (DWORD)pNode + 0x30;
			pNode[0x28 / 4] = (DWORD)g_pUnk_4B6F18;
			pNode[0x20 / 4] = 0;
		}
		if (Hover_sub_439560(pNode, 1) == 0)
			break;
	}
	Hover_sub_438820(3, "{end pump}");
}

// WndProc of the hidden "WavMix32" callback window: MM_WOM_DONE (block
// played, lParam is the WAVEHDR) and the private 0x400 message (lParam is
// the session) both pump the play queue; each warns when the block or
// session does not belong to the active session. Everything else goes to
// DefWindowProcA.

// hover.exe:0x439B60
// FUNCTION: HOVER 0x439b60
LRESULT __stdcall Hover_sub_439B60(HWND hwnd, UINT uMsg, WPARAM wParam,
	LPARAM lParam)
{
	Hover_sub_438820(2, "{WndProc}");
	switch (uMsg)
	{
	case MM_WOM_DONE:
		if (((DWORD*)lParam)[0x28 / 4] != (DWORD)g_pUnk_4B6F18)
			Hover_sub_438820(1,
				"++++++ waveblock does not match current active "
				"global data pointer!!!!!!!");
		Hover_sub_439A80();
		return 0;
	case 0x400:
		if ((SOUNDENGINE*)lParam != g_pUnk_4B6F18)
			Hover_sub_438820(1,
				"++++++ waveblock does not match current active "
				"global data pointer!!!!!!!");
		Hover_sub_439A80();
		return 0;
	}
	return DefWindowProcA(hwnd, uMsg, wParam & 0xFFFF, lParam);
}

// "Remix 2" session callback (selected as the dwUnk1B0 remix function by
// Hover_sub_43D3B0 when the ini "Remix" setting is 2): pumps the wave
// block queue through Hover_sub_439A80. Always returns 1.

// hover.exe:0x439BF0
// FUNCTION: HOVER 0x439bf0
int Hover_sub_439BF0()
{
	Hover_sub_438820(2, "r");
	Hover_sub_439A80();
	Hover_sub_438820(2, "{Remix done***}");
	return 1;
}

// Default session callback (the dwUnk1B0 remix function when "Remix" is
// not 2): returns every done block to the free list, swaps the active wave
// block pointer array, refills the playing queue with freshly mixed blocks
// and writes them to the device. While the pause-block counter runs the
// output is paused and restarted every dwUnk194 blocks. Always returns 1.
// The four helpers below are the WaveMix DestroyPlayQueue / SwapWaveBlocks /
// GetWaveBlock / AddToPlayingQueue routines; the original was compiled with
// them inlined (no such functions exist in hover.exe).

static __inline void Hover_wavemixDestroyQueue()
{
	while (g_pUnk_4C59C0 != NULL)
	{
		((DWORD*)g_pUnk_4C59C0)[0x20 / 4] = 1;
		Hover_sub_438E90(g_pUnk_4C59C0);
	}
}

static __inline DWORD* Hover_wavemixSwapBlocks()
{
	DWORD* pBlocks;

	pBlocks = &g_pUnk_4B6248->dwUnk198;
	if (*pBlocks == (DWORD)g_adwUnk_4B6E70)
		*pBlocks = (DWORD)g_adwUnk_4B6EC0;
	else
		*pBlocks = (DWORD)g_adwUnk_4B6E70;
	return pBlocks;
}

static __inline DWORD* Hover_wavemixGetBlock()
{
	DWORD** ppNode;
	DWORD* pNode;
	int i;
	int nCount;

	i = 0;
	nCount = g_pUnk_4B6248->dwUnk190;
	if (i < nCount)
	{
		ppNode = (DWORD**)g_pUnk_4B6248->dwUnk198;
		do
		{
			if ((*ppNode)[0x20 / 4] != 0)
				break;
			ppNode++;
			i++;
		} while (i < nCount);
	}
	if (i >= nCount)
	{
		Hover_sub_438820(2, "{gwb:out of wav blks}");
		return NULL;
	}
	pNode = ((DWORD**)g_pUnk_4B6248->dwUnk198)[i];
	pNode[1] = g_pUnk_4B6248->dwUnk18C;
	pNode[0] = (DWORD)pNode + 0x30;
	pNode[0x28 / 4] = (DWORD)g_pUnk_4B6F18;
	pNode[0x20 / 4] = 0;
	return pNode;
}

static __inline void Hover_wavemixQueueBlock(DWORD* pNode)
{
	pNode[0x2C / 4] = 0;
	if (g_pUnk_4C59C0 == NULL)
	{
		g_pUnk_4C59C4 = pNode;
		g_pUnk_4C59C0 = pNode;
	}
	else
	{
		((DWORD*)g_pUnk_4C59C4)[0x2C / 4] = (DWORD)pNode;
		g_pUnk_4C59C4 = pNode;
	}
}

// hover.exe:0x439C20
// FUNCTION: HOVER 0x439c20
int Hover_sub_439C20(DWORD dwPos, int bFlag)
{
	DWORD* pWrite;
	DWORD* pNode;
	int bPaused = 0;
	int nPauseCount;
	DWORD dwSave;
	DWORD dwCurPos;
	DWORD hWaveOut;

	Hover_sub_438820(2, "R");
	g_pUnk_4B6248->dwUnk19C = dwPos;
	Hover_wavemixDestroyQueue();
	Hover_wavemixSwapBlocks();
	for (;;)
	{
		pNode = Hover_wavemixGetBlock();
		if (pNode == NULL)
			break;
		if (Hover_sub_439560(pNode, 0) == 0)
		{
			pNode[0x20 / 4] = 1;
			break;
		}
		Hover_wavemixQueueBlock(pNode);
	}
	dwSave = g_pUnk_4B6248->dwUnk19C;
	hWaveOut = g_pUnk_4B6248->dwUnk0C;
	dwCurPos = Hover_sub_438840(hWaveOut, g_pUnk_4B6248->dwUnk1A4);
	waveOutReset((HWAVEOUT)hWaveOut);
	Hover_sub_438900(dwCurPos);
	g_pUnk_4B6248->dwUnk19C = dwSave;
	if ((int)g_pUnk_4B6248->dwUnk194 > 0)
	{
		waveOutPause((HWAVEOUT)g_pUnk_4B6248->dwUnk0C);
		bPaused = 1;
		nPauseCount = 0;
	}
	pWrite = (DWORD*)g_pUnk_4C59C0;
	while (pWrite != NULL)
	{
		if (waveOutWrite((HWAVEOUT)g_pUnk_4B6248->dwUnk0C,
				(LPWAVEHDR)pWrite, sizeof(WAVEHDR)) != 0)
		{
			if (g_dwUnk_4C59B8 != 0)
				MessageBoxA(NULL, "Failed to write block to device",
					"WavMix32", MB_ICONEXCLAMATION);
			pWrite[0x20 / 4] = 1;
			Hover_sub_438E90(pWrite);
		}
		if (bPaused != 0)
		{
			nPauseCount++;
			if ((int)g_pUnk_4B6248->dwUnk194 <= nPauseCount)
			{
				waveOutRestart((HWAVEOUT)g_pUnk_4B6248->dwUnk0C);
				bPaused = 0;
			}
		}
		pWrite = (DWORD*)pWrite[0x2C / 4];
	}
	if (bPaused != 0)
		waveOutRestart((HWAVEOUT)g_pUnk_4B6248->dwUnk0C);
	return 1;
}

// Returns the wave block data length: the "WaveBlockLen" entry of the
// current device's ini section wins over the caller's default; if both are
// zero it is derived from the sampling rate (2048 bytes per 11025 Hz).
// A nonzero result is clamped to 0x200..0x2000.

// hover.exe:0x43B810
// FUNCTION: HOVER 0x43b810
DWORD Hover_sub_43B810(DWORD dwBlockLen, LPWAVEFORMATEX pwfx)
{
	DWORD dwLen;

	dwLen = GetPrivateProfileIntA(g_pUnk_4B6248->szUnk1C, "WaveBlockLen",
		0, g_szUnk_4B6D58);
	if (dwLen != 0)
	{
		if (dwLen < 0x200)
			return 0x200;
		if (dwLen > 0x2000)
			return 0x2000;
	}
	else
	{
		dwLen = dwBlockLen;
		if (dwLen != 0)
		{
			if (dwLen < 0x200)
				return 0x200;
			if (dwLen > 0x2000)
				return 0x2000;
		}
		else
			return (pwfx->nSamplesPerSec / 11025) * 0x800;
	}
	return dwLen;
}

// Returns the ini section prefix for the running operating system:
// "WinNT:", "Win95:", "Win31:" or "OS_X:".

// hover.exe:0x43B9C0
// FUNCTION: HOVER 0x43b9c0
char* Hover_sub_43B9C0()
{
	if (GetVersion() < 0x80000000)
		return "WinNT:";
	if (GetVersion() >= 0x80000000 && LOBYTE(GetVersion()) >= 4)
		return "Win95:";
	if (GetVersion() >= 0x80000000 && LOBYTE(GetVersion()) < 4)
		return "Win31:";
	return "OS_X:";
}

// Looks up the "Device<u>" value under HKLM\SOFTWARE\Microsoft\WaveMix and
// copies it into pszBuf behind the operating system prefix from
// Hover_sub_43B9C0, producing the device's ini section name. Returns 1
// when the value exists and fits, 0 otherwise.

// hover.exe:0x43BA10
// FUNCTION: HOVER 0x43ba10
int Hover_sub_43BA10(DWORD dwDevice, char* pszBuf, int cbBuf)
{
	HKEY hKey;
	long cbData;
	char szValue[52];
	int nLen;
	int bFound;

	bFound = 0;
	nLen = lstrlenA(Hover_sub_43B9C0());
	if (nLen + 10 > cbBuf)
		return 0;
	wsprintfA(szValue, "Device%u", dwDevice);
	if (RegOpenKeyA(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\WaveMix",
			&hKey) != 0)
		return 0;
	cbBuf -= nLen;
	lstrcpyA(pszBuf, Hover_sub_43B9C0());
	cbData = cbBuf;
	if (RegQueryValueA(hKey, szValue, pszBuf + nLen, &cbData) == 0 &&
		cbData > 0)
		bFound = 1;
	RegCloseKey(hKey);
	return bFound;
}

// Reads one integer registry value (stored as a string) with a default.

// hover.exe:0x43BAD0
// FUNCTION: HOVER 0x43bad0
int Hover_sub_43BAD0(HKEY hKey, const char* pszValue, int nDefault)
{
	long cbData;
	char szBuf[12];

	if (hKey != NULL)
	{
		cbData = 10;
		if (RegQueryValueA(hKey, pszValue, szBuf, &cbData) == 0)
			return atol(szBuf);
	}
	return nDefault;
}

// WaveMixGetDeviceConfig: fills the fields of the session configuration
// that the caller left unset (flag bits in dwUnk02 mark the fields that
// were preset) from the WaveMix\Device<u> key under the open HKLM key in
// dwUnk1A, then marks every field set. Returns 1 on success, 0 when the
// configuration carries no open registry key.

// hover.exe:0x43BB20
// FUNCTION: HOVER 0x43bb20
int Hover_sub_43BB20(void* pv)
{
	SOUND_DRIVER_CFG* pCfg;
	HKEY hKey;
	char szSubKey[52];
	DWORD dwFlags;

	pCfg = (SOUND_DRIVER_CFG*)pv;
	dwFlags = pCfg->dwUnk02;
	if ((dwFlags & 0x400) == 0)
		return 0;
	if (pCfg->dwUnk1A == 0)
		return 0;
	if ((dwFlags & 0x80) != 0)
	{
		if (waveOutGetNumDevs() <= pCfg->wUnk14)
			pCfg->wUnk14 = 0;
	}
	else
		pCfg->wUnk14 = (WORD)g_pUnk_4B6248->dwUnk18;
	wsprintfA(szSubKey, "WaveMix\\Device%u", pCfg->wUnk14);
	if (RegOpenKeyA((HKEY)pCfg->dwUnk1A, szSubKey, &hKey) != 0)
		return 0;
	if ((dwFlags & 0x01) == 0)
		pCfg->wUnk06 = (WORD)Hover_sub_43BAD0(hKey, "Channels", 1);
	if ((dwFlags & 0x02) == 0)
		pCfg->wUnk08 =
			(WORD)Hover_sub_43BAD0(hKey, "SamplesPerSec", 0xB);
	if ((dwFlags & 0x04) == 0)
		pCfg->wUnk0A = (WORD)Hover_sub_43BAD0(hKey, "WaveBlocks", 3);
	if ((dwFlags & 0x08) == 0)
		pCfg->wUnk0C =
			(WORD)Hover_sub_43BAD0(hKey, "WaveBlockLen", 0x800);
	pCfg->wUnk0E = 1;
	if ((dwFlags & 0x20) == 0)
		pCfg->wUnk10 = (Hover_sub_43BAD0(hKey, "Remix", 1) != 2);
	if ((dwFlags & 0x40) == 0)
		pCfg->wUnk12 = (Hover_sub_43BAD0(hKey, "GoodWavePos", 0) != 0);
	if ((dwFlags & 0x100) == 0)
		pCfg->wUnk18 =
			(WORD)Hover_sub_43BAD0(hKey, "ShowDebugDialogs", 0);
	if ((dwFlags & 0x200) == 0)
		pCfg->wUnk16 = (WORD)Hover_sub_43BAD0(hKey, "PauseBlocks",
			pCfg->wUnk0A);
	pCfg->dwUnk02 = 0x3FF;
	waveOutGetDevCapsA(pCfg->wUnk14, &g_pUnk_4B6248->capsUnk7C,
		sizeof(WAVEOUTCAPSA));
	RegCloseKey(hKey);
	return 1;
}

// WaveMix debug helper: pops up a message box summarizing the session's
// current device, mixing function, position tracking, block geometry and
// output format. Called by Hover_sub_43C8F0 when debug dialogs are on.

// hover.exe:0x43BCF0
// FUNCTION: HOVER 0x43bcf0
void Hover_sub_43BCF0()
{
	WAVEOUTCAPSA caps;
	const char* pszMix;
	const char* pszGoodPos;
	const char* pszRemix;
	BOOL bUnknown;

	bUnknown = (waveOutGetDevCapsA(g_pUnk_4B6248->dwUnk18, &caps,
			sizeof(caps)) != 0 ||
		Hover_sub_43B880(caps.szPname) == 0);
	if (bUnknown)
		lstrcpyA(caps.szPname, "Unknown Device");
	pszMix = "cmixit";
	if (g_pUnk_4B6248->dwUnk1AC != (DWORD)Hover_sub_4389F0)
		pszMix = "386 mixit";
	pszGoodPos = "TRUE";
	if (g_pUnk_4B6248->dwUnk1A4 == 0)
		pszGoodPos = "FALSE";
	pszRemix = "Reset";
	if (g_pUnk_4B6248->dwUnk1B0 != (DWORD)Hover_sub_439C20)
		pszRemix = "NoReset";
	wsprintfA(g_szUnk_4C59D0,
		"Using:\t%s.\n\tRemix = %s\n\tGoodGetPos=%s\n\t%d ping pong "
		"wave blocks\n\tWave block len = %lu bytes\n\tpfnMixit=%s\n\t"
		"SamplesPerSec=%lu,\n\tChannels=%d\n\tPauseBlocks=%d",
		caps.szPname, pszRemix, pszGoodPos,
		g_pUnk_4B6248->dwUnk190, g_pUnk_4B6248->dwUnk18C, pszMix,
		g_pUnk_4B6248->wfxUnk17C.wf.nSamplesPerSec,
		g_pUnk_4B6248->wfxUnk17C.wf.nChannels,
		g_pUnk_4B6248->dwUnk194);
	MessageBoxA(NULL, g_szUnk_4C59D0, "WavMix32", MB_ICONINFORMATION);
}

// WM_INITDIALOG handler for the WaveMix configure dialog: formats the
// window title, stashes the SOUND_DRIVER_CFG pointer in GWL_USERDATA,
// checks the checkboxes from the config, sets the edit-control text
// limits and initial values, and fills the playback-device combo box.
// Always returns 1.

// hover.exe:0x43BDF0
// FUNCTION: HOVER 0x43bdf0
int Hover_sub_43BDF0(HWND hwnd, WPARAM wParam, SOUND_DRIVER_CFG* pCfg)
{
	WAVEOUTCAPSA caps;
	char szBuf[0x100];
	int nDevs;
	int nErr;
	int i;

	GetWindowTextA(hwnd, szBuf, 0x100);
	wsprintfA(g_szUnk_4C59D0, szBuf, 2, 0x50);
	SetWindowTextA(hwnd, g_szUnk_4C59D0);
	SetWindowLongA(hwnd, GWL_USERDATA, (LONG)pCfg);
	SendMessageA(GetDlgItem(hwnd, 0x3e8), BM_SETCHECK, pCfg->wUnk06 > 1,
		0);
	SendMessageA(GetDlgItem(hwnd, 0x3e9), BM_SETCHECK, pCfg->wUnk10 >= 1,
		0);
	SendMessageA(GetDlgItem(hwnd, 0x3ec), BM_SETCHECK, pCfg->wUnk12 >= 1,
		0);
	SendMessageA(GetDlgItem(hwnd, 0x3ed), BM_SETCHECK, pCfg->wUnk0E >= 1,
		0);
	SendMessageA(GetDlgItem(hwnd, 0x3f2), BM_SETCHECK, pCfg->wUnk18 >= 1,
		0);
	EnableWindow(GetDlgItem(hwnd, 0x3ed), FALSE);
	SendMessageA(GetDlgItem(hwnd, 0x3eb), EM_LIMITTEXT, 2, 0);
	SendMessageA(GetDlgItem(hwnd, 0x3ef), EM_LIMITTEXT, 4, 0);
	SendMessageA(GetDlgItem(hwnd, 0x3f0), EM_LIMITTEXT, 2, 0);
	SendMessageA(GetDlgItem(hwnd, 0x3f3), EM_LIMITTEXT, 2, 0);
	GetWindowTextA(GetDlgItem(hwnd, 0x3f4), szBuf, 0x64);
	wsprintfA(g_szUnk_4C59D0, szBuf, 2, 0x14);
	SetWindowTextA(GetDlgItem(hwnd, 0x3f4), g_szUnk_4C59D0);
	GetWindowTextA(GetDlgItem(hwnd, 0x3f5), szBuf, 0x64);
	wsprintfA(g_szUnk_4C59D0, szBuf, 0x200, 0x2000);
	SetWindowTextA(GetDlgItem(hwnd, 0x3f5), g_szUnk_4C59D0);
	GetWindowTextA(GetDlgItem(hwnd, 0x3f6), szBuf, 0x64);
	wsprintfA(g_szUnk_4C59D0, szBuf, 0, 0x14);
	SetWindowTextA(GetDlgItem(hwnd, 0x3f6), g_szUnk_4C59D0);
	GetWindowTextA(GetDlgItem(hwnd, 0x3f7), szBuf, 0x64);
	wsprintfA(g_szUnk_4C59D0, szBuf, 0x10);
	SetWindowTextA(GetDlgItem(hwnd, 0x3f7), g_szUnk_4C59D0);
	wsprintfA(g_szUnk_4C59D0, "%d", pCfg->wUnk0A);
	SetWindowTextA(GetDlgItem(hwnd, 0x3eb), g_szUnk_4C59D0);
	wsprintfA(g_szUnk_4C59D0, "%d", pCfg->wUnk0C);
	SetWindowTextA(GetDlgItem(hwnd, 0x3ef), g_szUnk_4C59D0);
	wsprintfA(g_szUnk_4C59D0, "%d", pCfg->wUnk08);
	SetWindowTextA(GetDlgItem(hwnd, 0x3f0), g_szUnk_4C59D0);
	wsprintfA(g_szUnk_4C59D0, "%d", pCfg->wUnk16);
	SetWindowTextA(GetDlgItem(hwnd, 0x3f3), g_szUnk_4C59D0);
	nDevs = waveOutGetNumDevs();
	for (i = 0; i < nDevs; i++)
	{
		nErr = waveOutGetDevCapsA(i, &caps, sizeof(caps));
		if (nErr != 0)
		{
			wsprintfA(g_szUnk_4C59D0,
				"waveOutGetDevCaps failed (err %u) for device %d",
				nErr, i);
			MessageBoxA(NULL, g_szUnk_4C59D0, "WavMix32",
				MB_ICONINFORMATION);
		}
		else
		{
			wsprintfA(g_szUnk_4C59D0, "%d: %s", i, caps.szPname);
			SendMessageA(GetDlgItem(hwnd, 0x3f1), CB_ADDSTRING, 0,
				(LPARAM)g_szUnk_4C59D0);
		}
	}
	SendMessageA(GetDlgItem(hwnd, 0x3f1), CB_SETCURSEL, pCfg->wUnk14, 0);
	return 1;
}

// WM_COMMAND handler for the WaveMix configure dialog. On OK it reads
// every control back into the SOUND_DRIVER_CFG stashed in GWL_USERDATA
// and ends the dialog with 1; on Cancel it ends the dialog with 0.

// hover.exe:0x43C1C0
// FUNCTION: HOVER 0x43c1c0
int Hover_sub_43C1C0(HWND hwnd, DWORD dwId, DWORD dwNotify, LPARAM lParam)
{
	SOUND_DRIVER_CFG* pCfg;

	pCfg = (SOUND_DRIVER_CFG*)GetWindowLongA(hwnd, GWL_USERDATA);
	if (dwId == 1)
	{
		if (pCfg != NULL)
		{
			pCfg->wUnk06 = ((UINT)SendMessageA(GetDlgItem(hwnd, 0x3e8),
				BM_GETCHECK, 0, 0) >= 1) ? 2 : 1;
			pCfg->wUnk10 = ((UINT)SendMessageA(GetDlgItem(hwnd, 0x3e9),
				BM_GETCHECK, 0, 0) >= 1) ? 1 : 0;
			pCfg->wUnk12 = ((UINT)SendMessageA(GetDlgItem(hwnd, 0x3ec),
				BM_GETCHECK, 0, 0) >= 1) ? 1 : 0;
			pCfg->wUnk18 = ((UINT)SendMessageA(GetDlgItem(hwnd, 0x3f2),
				BM_GETCHECK, 0, 0) >= 1) ? 1 : 0;
			pCfg->wUnk0E = ((UINT)SendMessageA(GetDlgItem(hwnd, 0x3ed),
				BM_GETCHECK, 0, 0) >= 1) ? 1 : 0;
			GetWindowTextA(GetDlgItem(hwnd, 0x3eb), g_szUnk_4C59D0,
				10);
			pCfg->wUnk0A = (WORD)atoi(g_szUnk_4C59D0);
			GetWindowTextA(GetDlgItem(hwnd, 0x3ef), g_szUnk_4C59D0,
				10);
			pCfg->wUnk0C = (WORD)atoi(g_szUnk_4C59D0);
			GetWindowTextA(GetDlgItem(hwnd, 0x3f0), g_szUnk_4C59D0,
				10);
			pCfg->wUnk08 = (WORD)atoi(g_szUnk_4C59D0);
			GetWindowTextA(GetDlgItem(hwnd, 0x3f3), g_szUnk_4C59D0,
				10);
			pCfg->wUnk16 = (WORD)atoi(g_szUnk_4C59D0);
			GetWindowTextA(GetDlgItem(hwnd, 0x3f1), g_szUnk_4C59D0,
				10);
			if (isdigit(g_szUnk_4C59D0[0]))
				pCfg->wUnk14 = (WORD)atoi(g_szUnk_4C59D0);
			else
				pCfg->wUnk14 = 0;
			EndDialog(hwnd, 1);
		}
		return 1;
	}
	if (dwId == 2)
	{
		EndDialog(hwnd, 0);
		return 1;
	}
	return 0;
}

// Dialog proc for the WaveMix configure dialog: forwards WM_INITDIALOG
// to Hover_sub_43BDF0 (returning its result) and WM_COMMAND to
// Hover_sub_43C1C0.

// hover.exe:0x43C3C0
// FUNCTION: HOVER 0x43c3c0
BOOL __stdcall Hover_sub_43C3C0(HWND hwnd, UINT uMsg, WPARAM wParam,
	LPARAM lParam)
{
	if (uMsg == WM_INITDIALOG)
		return Hover_sub_43BDF0(hwnd, wParam, (SOUND_DRIVER_CFG*)lParam);
	if (uMsg == WM_COMMAND)
	{
		Hover_sub_43C1C0(hwnd, LOWORD(wParam), HIWORD(wParam), lParam);
		return TRUE;
	}
	return FALSE;
}

// Allocates a global-memory block and writes the DLGTEMPLATE header of
// an in-memory dialog template into it (in-memory templates use wide
// strings; the zeroed menu and class fields sit between the header and
// the title). Stores the block size, rounded up to a DWORD boundary,
// into *pcbTemplate and returns the GlobalLock'd pointer, or NULL on
// failure.

// hover.exe:0x43C420
// FUNCTION: HOVER 0x43c420
void* Hover_sub_43C420(DWORD* pcbTemplate, DWORD dwStyle, int x, int y,
	int cx, int cy, LPCWSTR pszTitle)
{
	DLGTEMPLATE* pTemplate;

	*pcbTemplate = wcslen(pszTitle) * 2 + 0x18;
	if ((*pcbTemplate & 3) != 0)
		*pcbTemplate = *pcbTemplate - (*pcbTemplate & 3) + 4;
	pTemplate = (DLGTEMPLATE*)GlobalLock(GlobalAlloc(GHND,
		*pcbTemplate));
	if (pTemplate == NULL)
		return NULL;
	pTemplate->style = dwStyle | WS_VISIBLE;
	pTemplate->dwExtendedStyle = 0;
	pTemplate->cdit = 0;
	pTemplate->x = (short)x;
	pTemplate->y = (short)y;
	pTemplate->cx = (short)cx;
	pTemplate->cy = (short)cy;
	memcpy((BYTE*)pTemplate + 0x16, pszTitle, wcslen(pszTitle) * 2 + 2);
	return pTemplate;
}

// Grows an in-memory dialog template built by Hover_sub_43C420 by one
// control (a DLGITEMTEMPLATE with its class stored as a 0xFFFF/ordinal
// pair and a wide title string), reallocating the block and updating
// *pcbTemplate and the template's control count. wClass is the
// predefined window class ordinal (0x80 button, 0x81 edit, 0x82 static,
// 0x85 combo box). Returns the re-GlobalLock'd template pointer, or
// NULL on failure.

// hover.exe:0x43C4C0
// FUNCTION: HOVER 0x43c4c0
void* Hover_sub_43C4C0(DWORD* pcbTemplate, void* hTemplate, WORD wClass,
	DWORD dwStyle, WORD wId, int x, int y, int cx, int cy,
	LPCWSTR pszText)
{
	DLGITEMTEMPLATE* pItem;
	DLGTEMPLATE* pTemplate;
	DWORD cbOld;

	if (hTemplate != NULL)
	{
		cbOld = *pcbTemplate;
		*pcbTemplate = cbOld + wcslen(pszText) * 2 + 0x19;
		if ((*pcbTemplate & 3) != 0)
			*pcbTemplate = *pcbTemplate - (*pcbTemplate & 3) + 4;
		GlobalUnlock(GlobalHandle((LPCVOID)hTemplate));
		hTemplate = GlobalLock(GlobalReAlloc(
			GlobalHandle((LPCVOID)hTemplate), *pcbTemplate, GHND));
		if (hTemplate != NULL)
		{
			pTemplate = (DLGTEMPLATE*)hTemplate;
			pItem = (DLGITEMTEMPLATE*)((BYTE*)pTemplate + cbOld);
			pItem->style = dwStyle | WS_VISIBLE | WS_CHILD;
			pItem->dwExtendedStyle = 0;
			pItem->x = (short)x;
			pItem->y = (short)y;
			pItem->cx = (short)cx;
			pItem->cy = (short)cy;
			pItem->id = wId;
			*(DWORD*)((BYTE*)pItem + 0x12) =
				((DWORD)wClass << 16) | 0xFFFF;
			wcscpy((WCHAR*)((BYTE*)pItem + 0x16), pszText);
			pTemplate->cdit++;
		}
	}
	return hTemplate;
}

// Builds the in-memory dialog template for the WaveMix configure dialog
// (device combo box, channel/remix/position checkboxes, wave block and
// sampling rate edit fields, OK/Cancel) and returns the GlobalLock'd
// block; the caller frees it with Hover_sub_43C8C0. Returns NULL on
// failure.

// hover.exe:0x43C5A0
// FUNCTION: HOVER 0x43c5a0
void* Hover_sub_43C5A0()
{
	DWORD cbTemplate;
	void* pTemplate;

	cbTemplate = 0;
	pTemplate = Hover_sub_43C420(&cbTemplate,
		DS_MODALFRAME | WS_POPUP | WS_CAPTION | WS_SYSMENU, 0, 0, 0xd4,
		0x84, L"WavMix32 Settings (Ver %X.%X Static)");
	pTemplate = Hover_sub_43C4C0(&cbTemplate, pTemplate, 0x80,
		WS_TABSTOP | BS_DEFPUSHBUTTON, 1, 0x9b, 5, 0x32, 0xe, L"&Ok");
	pTemplate = Hover_sub_43C4C0(&cbTemplate, pTemplate, 0x80,
		WS_TABSTOP | BS_PUSHBUTTON, 2, 0x9b, 0x19, 0x32, 0xe,
		L"&Cancel");
	pTemplate = Hover_sub_43C4C0(&cbTemplate, pTemplate, 0x82, 0, 0x3f4,
		5, 5, 0x73, 8, L"Number of WaveBlocks (%d-%d):");
	pTemplate = Hover_sub_43C4C0(&cbTemplate, pTemplate, 0x82, 0, 0x3f5,
		5, 0x14, 0x73, 8, L"Size of WaveBlocks (%d-%d):");
	pTemplate = Hover_sub_43C4C0(&cbTemplate, pTemplate, 0x82, 0, 0xffff,
		5, 0x23, 0x66, 8, L"Playback Frequency (11,22,44):");
	pTemplate = Hover_sub_43C4C0(&cbTemplate, pTemplate, 0x82, 0, 0xffff,
		5, 0x50, 0x39, 8, L"Playback Device:");
	pTemplate = Hover_sub_43C4C0(&cbTemplate, pTemplate, 0x82, 0, 0x3f6,
		5, 0x32, 0x73, 8, L"Number of Pause Blocks (%d-%d):");
	pTemplate = Hover_sub_43C4C0(&cbTemplate, pTemplate, 0x82, 0, 0x3f7,
		5, 0x41, 0x3f, 8, L"Max Channels = %d");
	pTemplate = Hover_sub_43C4C0(&cbTemplate, pTemplate, 0x81,
		ES_AUTOHSCROLL | WS_BORDER | WS_TABSTOP, 0x3eb, 0x7d, 5, 0x19,
		0xc, L"-1");
	pTemplate = Hover_sub_43C4C0(&cbTemplate, pTemplate, 0x81,
		ES_AUTOHSCROLL | WS_BORDER | WS_TABSTOP, 0x3ef, 0x7d, 0x14,
		0x19, 0xc, L"-1");
	pTemplate = Hover_sub_43C4C0(&cbTemplate, pTemplate, 0x81,
		ES_AUTOHSCROLL | WS_BORDER | WS_TABSTOP, 0x3f0, 0x7d, 0x23,
		0x19, 0xc, L"-1");
	pTemplate = Hover_sub_43C4C0(&cbTemplate, pTemplate, 0x81,
		ES_AUTOHSCROLL | WS_BORDER | WS_TABSTOP, 0x3f3, 0x7d, 0x32,
		0x19, 0xc, L"-1");
	pTemplate = Hover_sub_43C4C0(&cbTemplate, pTemplate, 0x85,
		WS_VSCROLL | WS_TABSTOP | CBS_SORT | CBS_DROPDOWN, 0x3f1, 0x41,
		0x50, 0x8c, 0x3c, L"No Devices");
	pTemplate = Hover_sub_43C4C0(&cbTemplate, pTemplate, 0x80,
		WS_TABSTOP | BS_AUTOCHECKBOX, 0x3e8, 5, 0x65, 0x28, 0xa,
		L"Stereo");
	pTemplate = Hover_sub_43C4C0(&cbTemplate, pTemplate, 0x80,
		WS_TABSTOP | BS_AUTOCHECKBOX, 0x3e9, 0x37, 0x65, 0x3c, 0xa,
		L"Reset Remix");
	pTemplate = Hover_sub_43C4C0(&cbTemplate, pTemplate, 0x80,
		WS_TABSTOP | BS_AUTOCHECKBOX, 0x3f2, 0x7d, 0x65, 0x4e, 0xa,
		L"Show Debug Dialogs");
	pTemplate = Hover_sub_43C4C0(&cbTemplate, pTemplate, 0x80,
		WS_TABSTOP | BS_AUTOCHECKBOX, 0x3ed, 5, 0x75, 0x28, 0xa,
		L"CMixit");
	pTemplate = Hover_sub_43C4C0(&cbTemplate, pTemplate, 0x80,
		WS_TABSTOP | BS_AUTOCHECKBOX, 0x3ec, 0x37, 0x75, 0x4b, 0xa,
		L"Good Get Position");
	return pTemplate;
}

// Frees the global memory block holding a WaveMixConfigure dialog
// template built by Hover_sub_43C5A0.

// hover.exe:0x43C8C0
// FUNCTION: HOVER 0x43c8c0
void Hover_sub_43C8C0(void* hTemplate)
{
	if (hTemplate != NULL)
	{
		GlobalUnlock(GlobalHandle((LPCVOID)hTemplate));
		GlobalFree(GlobalHandle((LPCVOID)hTemplate));
	}
}

// WaveMixConfigure: applies a SOUND_DRIVER_CFG to the session. When pv is
// NULL the current settings are read back into a local config (through
// Hover_sub_43CF50) and edited in a modal configuration dialog first.
// The dwUnk02 flag bits select the fields to apply (0x01 channels, 0x02
// sampling rate, 0x04 wave blocks, 0x08 block length, 0x10 mixing
// function, 0x20 remix callback, 0x40 position source, 0x80 device,
// 0x100 debug dialogs, 0x200 pause blocks). When dwB is nonzero it
// receives a flag telling whether the output format changed; when dwC is
// nonzero the applied settings are written back to the ini file. Returns
// 0 on success, 5 = invalid session, 4 = session active, 0xc = driver in
// use, 1 on dialog/config failure, or a waveOutGetDevCapsA error code.

// hover.exe:0x43C8F0
// FUNCTION: HOVER 0x43c8f0
int __stdcall Hover_sub_43C8F0(SOUNDENGINE* pEngine, DWORD dwA, void* pv,
	DWORD dwB, DWORD dwC)
{
	SOUND_DRIVER_CFG cfgLocal;
	SOUND_DRIVER_CFG* pCfg;
	WAVEOUTCAPSA caps;
	void* pTemplate;
	DWORD dwFlags;
	int bChanged;
	int nDlg;
	int nErr;

	bChanged = 0;
	if (pEngine != NULL && pEngine->wMagic00 == 0x5432 &&
		pEngine->wMagic1BC == 0x5432)
	{
		g_pUnk_4B6248 = pEngine;
	}
	else
	{
		MessageBeep(0xFFFFFFFF);
		wsprintfA(g_szUnk_4C59D0,
			"Invalid session handle 0x%04X passed to WaveMix API",
			pEngine);
		Hover_sub_438820(1, g_szUnk_4C59D0);
		MessageBoxA(NULL, g_szUnk_4C59D0, "WavMix32",
			MB_ICONEXCLAMATION);
		g_pUnk_4B6248 = NULL;
	}
	if (g_pUnk_4B6248 == NULL)
		return 5;
	if (g_pUnk_4B6248->dwUnk10 != 0)
		return 4;
	if (g_pUnk_4B6248->dwUnk14 != 0)
		return 0xc;
	Hover_sub_43A5A0(pEngine, 0xFFFFFFFF, 1);
	pCfg = (SOUND_DRIVER_CFG*)pv;
	if (pCfg == NULL)
	{
		cfgLocal.dwUnk02 = 0x3FF;
		cfgLocal.wUnk00 = 0x1E;
		Hover_sub_43CF50(pEngine, &cfgLocal);
		pTemplate = Hover_sub_43C5A0();	// dialog template
		if (pTemplate == NULL)
			return 1;
		g_pUnk_4B6248->dwUnk14 = 1;
		nDlg = DialogBoxIndirectParamA(g_hUnk_4B6F10,
			(LPCDLGTEMPLATE)pTemplate, (HWND)dwA,
			(DLGPROC)Hover_sub_43C3C0,	// dialog proc
			(LPARAM)&cfgLocal);
		Hover_sub_43C8C0(pTemplate);
		g_pUnk_4B6248->dwUnk14 = 0;
		if (nDlg != 1)
			return 1;
		pCfg = &cfgLocal;
	}
	if (pCfg->dwUnk02 == 0)
		return 1;
	if (pEngine != NULL && pEngine->wMagic00 == 0x5432 &&
		pEngine->wMagic1BC == 0x5432)
	{
		g_pUnk_4B6248 = pEngine;
	}
	else
	{
		MessageBeep(0xFFFFFFFF);
		wsprintfA(g_szUnk_4C59D0,
			"Invalid session handle 0x%04X passed to WaveMix API",
			pEngine);
		Hover_sub_438820(1, g_szUnk_4C59D0);
		MessageBoxA(NULL, g_szUnk_4C59D0, "WavMix32",
			MB_ICONEXCLAMATION);
		g_pUnk_4B6248 = NULL;
	}
	if (g_pUnk_4B6248 == NULL)
		return 5;
	dwFlags = pCfg->dwUnk02;
	if (dwFlags & 0x100)
		g_dwUnk_4C59B8 = (pCfg->wUnk18 >= 1);
	if (dwFlags & 0x80)
	{
		if (g_pUnk_4B6248->dwUnk18 != pCfg->wUnk14)
		{
			nErr = waveOutGetDevCapsA(pCfg->wUnk14, &caps,
				sizeof(caps));
			if (nErr != 0)
				return nErr;
			memcpy(&g_pUnk_4B6248->capsUnk7C, &caps, sizeof(caps));
			g_pUnk_4B6248->dwUnk18 = pCfg->wUnk14;
			if (g_pUnk_4B6248->capsUnk7C.wChannels == 1 &&
				g_pUnk_4B6248->wfxUnk17C.wf.nChannels == 2)
			{
				g_pUnk_4B6248->wfxUnk17C.wf.nChannels = 1;
				bChanged = 1;
				g_pUnk_4B6248->wfxUnk17C.wf.nBlockAlign = 1;
				g_pUnk_4B6248->wfxUnk17C.wf.nAvgBytesPerSec =
					g_pUnk_4B6248->wfxUnk17C.wf.nChannels *
					g_pUnk_4B6248->wfxUnk17C.wf.nSamplesPerSec;
			}
			lstrcpyA(g_pUnk_4B6248->szUnk1C, caps.szPname);
			if (Hover_sub_43B880(g_pUnk_4B6248->szUnk1C) == 0)
				lstrcpyA(g_pUnk_4B6248->szUnk1C, "Unkown Device");
			if (Hover_sub_43BA10(g_pUnk_4B6248->dwUnk18,
					g_pUnk_4B6248->szUnk1C,
					sizeof(g_pUnk_4B6248->szUnk1C)) == 0)
			{
				lstrcpyA(g_pUnk_4B6248->szUnk1C,
					Hover_sub_43B9C0());
				lstrcatA(g_pUnk_4B6248->szUnk1C,
					g_pUnk_4B6248->capsUnk7C.szPname);
			}
		}
	}
	if (dwFlags & 1)
	{
		if (pCfg->wUnk06 > 1)
		{
			if (g_pUnk_4B6248->capsUnk7C.wChannels > 1)
			{
				if (g_pUnk_4B6248->wfxUnk17C.wf.nChannels == 1)
					bChanged = 1;
				g_pUnk_4B6248->wfxUnk17C.wf.nChannels = 2;
				g_pUnk_4B6248->wfxUnk17C.wf.nBlockAlign = 2;
				g_pUnk_4B6248->wfxUnk17C.wf.nAvgBytesPerSec =
					g_pUnk_4B6248->wfxUnk17C.wf.nChannels *
					g_pUnk_4B6248->wfxUnk17C.wf.nSamplesPerSec;
			}
		}
		else
		{
			if (g_pUnk_4B6248->wfxUnk17C.wf.nChannels == 2)
				bChanged = 1;
			g_pUnk_4B6248->wfxUnk17C.wf.nChannels = 1;
			g_pUnk_4B6248->wfxUnk17C.wf.nBlockAlign = 1;
			g_pUnk_4B6248->wfxUnk17C.wf.nAvgBytesPerSec =
				g_pUnk_4B6248->wfxUnk17C.wf.nChannels *
				g_pUnk_4B6248->wfxUnk17C.wf.nSamplesPerSec;
		}
	}
	if (dwFlags & 2)
	{
		switch (pCfg->wUnk08)
		{
		case 22:
			if (g_pUnk_4B6248->wfxUnk17C.wf.nSamplesPerSec != 22050)
				bChanged = 1;
			g_pUnk_4B6248->wfxUnk17C.wf.nSamplesPerSec = 22050;
			g_pUnk_4B6248->wfxUnk17C.wf.nAvgBytesPerSec =
				22050 * g_pUnk_4B6248->wfxUnk17C.wf.nChannels;
			break;
		case 44:
			if (g_pUnk_4B6248->wfxUnk17C.wf.nSamplesPerSec != 44100)
				bChanged = 1;
			g_pUnk_4B6248->wfxUnk17C.wf.nSamplesPerSec = 44100;
			g_pUnk_4B6248->wfxUnk17C.wf.nAvgBytesPerSec =
				44100 * g_pUnk_4B6248->wfxUnk17C.wf.nChannels;
			break;
		default:
			if (g_pUnk_4B6248->wfxUnk17C.wf.nSamplesPerSec != 11025)
				bChanged = 1;
			g_pUnk_4B6248->wfxUnk17C.wf.nSamplesPerSec = 11025;
			g_pUnk_4B6248->wfxUnk17C.wf.nAvgBytesPerSec =
				11025 * g_pUnk_4B6248->wfxUnk17C.wf.nChannels;
			break;
		}
	}
	if (dwFlags & 4)
	{
		g_pUnk_4B6248->dwUnk190 = pCfg->wUnk0A;
		if ((int)g_pUnk_4B6248->dwUnk190 < 2)
			g_pUnk_4B6248->dwUnk190 = 2;
		else if ((int)g_pUnk_4B6248->dwUnk190 > 0x14)
			g_pUnk_4B6248->dwUnk190 = 0x14;
		if ((int)g_pUnk_4B6248->dwUnk194 < 0)
			g_pUnk_4B6248->dwUnk194 = 0;
		else if ((int)g_pUnk_4B6248->dwUnk194 >
				(int)g_pUnk_4B6248->dwUnk190)
			g_pUnk_4B6248->dwUnk194 = g_pUnk_4B6248->dwUnk190;
	}
	if (dwFlags & 8)
	{
		g_pUnk_4B6248->dwUnk18C = pCfg->wUnk0C;
		if (g_pUnk_4B6248->dwUnk18C < 0x200)
			g_pUnk_4B6248->dwUnk18C = 0x200;
		else if (g_pUnk_4B6248->dwUnk18C > 0x2000)
			g_pUnk_4B6248->dwUnk18C = 0x2000;
	}
	if (dwFlags & 0x10)
		g_pUnk_4B6248->dwUnk1AC = (DWORD)Hover_sub_4389F0;
	if (dwFlags & 0x20)
	{
		DWORD dwProc = (pCfg->wUnk10 != 0) ?
			(DWORD)Hover_sub_439C20 : (DWORD)Hover_sub_439BF0;
		g_pUnk_4B6248->dwUnk1B0 = dwProc;
	}
	if (dwFlags & 0x40)
		g_pUnk_4B6248->dwUnk1A4 = (pCfg->wUnk12 >= 1);
	if (dwFlags & 0x200)
	{
		g_pUnk_4B6248->dwUnk194 = pCfg->wUnk16;
		if ((int)g_pUnk_4B6248->dwUnk194 < 0)
			g_pUnk_4B6248->dwUnk194 = 0;
		else if ((int)g_pUnk_4B6248->dwUnk194 >
				(int)g_pUnk_4B6248->dwUnk190)
			g_pUnk_4B6248->dwUnk194 = g_pUnk_4B6248->dwUnk190;
	}
	Hover_sub_43CF50(pEngine, pCfg);
	if (g_dwUnk_4C59B8 != 0)
		Hover_sub_43BCF0();
	if (dwB != 0)
		*(DWORD*)dwB = bChanged;
	if (dwC != 0)
		Hover_sub_43D100(dwFlags);	// write ini settings
	return 0;
}

// WaveMixGetSessionConfig: writes the session's current settings back
// into the caller's SOUND_DRIVER_CFG, for the fields selected by the
// dwUnk02 flag bits (same bit assignments as Hover_sub_43C8F0). Returns
// 0 on success, 5 = invalid session, 0xb = NULL config.

// hover.exe:0x43CF50
// FUNCTION: HOVER 0x43cf50
int __stdcall Hover_sub_43CF50(SOUNDENGINE* pEngine, SOUND_DRIVER_CFG* pCfg)
{
	DWORD dwFlags;

	if (pEngine != NULL && pEngine->wMagic00 == 0x5432 &&
		pEngine->wMagic1BC == 0x5432)
	{
		g_pUnk_4B6248 = pEngine;
	}
	else
	{
		MessageBeep(0xFFFFFFFF);
		wsprintfA(g_szUnk_4C59D0,
			"Invalid session handle 0x%04X passed to WaveMix API",
			pEngine);
		Hover_sub_438820(1, g_szUnk_4C59D0);
		MessageBoxA(NULL, g_szUnk_4C59D0, "WavMix32",
			MB_ICONEXCLAMATION);
		g_pUnk_4B6248 = NULL;
	}
	if (g_pUnk_4B6248 == NULL)
		return 5;
	if (pCfg == NULL)
		return 0xb;
	dwFlags = pCfg->dwUnk02;
	if (dwFlags & 1)
		pCfg->wUnk06 = g_pUnk_4B6248->wfxUnk17C.wf.nChannels;
	if (dwFlags & 2)
		pCfg->wUnk08 = (WORD)(g_pUnk_4B6248->wfxUnk17C.wf.nSamplesPerSec /
			11025 * 11);
	if (dwFlags & 4)
		pCfg->wUnk0A = (WORD)g_pUnk_4B6248->dwUnk190;
	if (dwFlags & 8)
		pCfg->wUnk0C = (WORD)g_pUnk_4B6248->dwUnk18C;
	if (dwFlags & 0x10)
		pCfg->wUnk0E =
			(g_pUnk_4B6248->dwUnk1AC == (DWORD)Hover_sub_4389F0);
	if (dwFlags & 0x20)
		pCfg->wUnk10 =
			(g_pUnk_4B6248->dwUnk1B0 == (DWORD)Hover_sub_439C20);
	if (dwFlags & 0x40)
		pCfg->wUnk12 = (WORD)g_pUnk_4B6248->dwUnk1A4;
	if (dwFlags & 0x80)
		pCfg->wUnk14 = (WORD)g_pUnk_4B6248->dwUnk18;
	if (dwFlags & 0x100)
		pCfg->wUnk18 = (g_dwUnk_4C59B8 >= 1);
	if (dwFlags & 0x200)
		pCfg->wUnk16 = (WORD)g_pUnk_4B6248->dwUnk194;
	return 0;
}

// Builds the wavemix.ini path in g_szUnk_4B6D58: the module's own
// filename with everything from the last backslash on replaced by
// "\WAVEMIX.INF". Returns 1 on success, 0 on failure.

// hover.exe:0x43DB90
// FUNCTION: HOVER 0x43db90
int Hover_sub_43DB90(HMODULE hMod)
{
	char* p;
	DWORD dwLen;

	dwLen = GetModuleFileNameA(hMod, g_szUnk_4B6D58, 0x104);
	if (dwLen == 0)
		return 0;
	p = g_szUnk_4B6D58 + dwLen;
	while (*p != '\\')
		p--;
	*p = '\0';
	lstrcpyA(p, "\\WAVEMIX.INF");
	return 1;
}

// Builds the 11x256 sample-volume table at g_aUnk_4B6250: entry
// [volume][sample] = volume * (sample - 128) / 10 + 128, so volume row 0
// is silence and row 10 is the identity mapping.

// hover.exe:0x43DBE0
// FUNCTION: HOVER 0x43dbe0
void Hover_sub_43DBE0()
{
	int j;
	int i;

	for (i = 0; i <= 10; i++)
	{
		for (j = -128; j < 128; j++)
			g_aUnk_4B6250[i][j + 128] = (BYTE)(i * j / 10 + 128);
	}
}

// WaveMix one-time module initialization: builds the ini path and the
// volume table, reads the "debug" verbosity setting, selects the Remix=2
// mixing function, registers the "WavMix32" callback window class and
// initializes the channel-node pool. Returns 1 on success, 0 on failure.

// hover.exe:0x43DC40
// FUNCTION: HOVER 0x43dc40
int Hover_sub_43DC40(HMODULE hMod)
{
	WNDCLASSA wc;

	if (g_dwUnk_4C59C8 != 0)
		return 1;
	if (Hover_sub_43DB90(hMod) == 0)
		return 0;
	Hover_sub_43DBE0();
	g_dwUnk_4C59BC = GetPrivateProfileIntA("general", "debug", 0,
		g_szUnk_4B6D58);
	g_hUnk_4B6F10 = hMod;
	g_dwUnk_4B6F14 = (DWORD)Hover_sub_4389F0;
	// The fields are filled in the original's order, which is not the
	// WNDCLASS declaration order: the two API results come first, and
	// hInstance is read back out of the global *after* both calls (the
	// calls are what stop VC from forwarding hMod into it, which is how
	// the original gets its reload - cf. rule 32).
	wc.hCursor = LoadCursorA(NULL, IDC_ARROW);
	wc.hIcon = NULL;
	wc.lpszMenuName = NULL;
	wc.lpszClassName = "WavMix32";
	wc.hbrBackground = (HBRUSH)GetStockObject(LTGRAY_BRUSH);
	wc.style = 0;
	wc.hInstance = g_hUnk_4B6F10;
	wc.cbWndExtra = 0;
	wc.cbClsExtra = 0;
	wc.lpfnWndProc = Hover_sub_439B60;
	if (RegisterClassA(&wc) == 0)
		return 0;
	Hover_sub_438CA0();
	g_dwUnk_4C59C8 = 1;
	return 1;
}

// Pops a channel node off the g_pUnk_4B6D50 free list (the list is
// replenished from the g_adwUnk_4B4A58 pool by Hover_sub_438CA0).

// hover.exe:0x438CD0
// FUNCTION: HOVER 0x438cd0
void* Hover_sub_438CD0()
{
	void* pNode;

	if (g_pUnk_4B6D50 == NULL)
	{
		Hover_sub_438820(1, "GetChannelNode failed!");
		return NULL;
	}
	pNode = g_pUnk_4B6D50;
	g_pUnk_4B6D50 = *(void**)pNode;
	*(DWORD*)pNode = 0;
	return pNode;
}

// Resets the play position when nothing is queued on any channel and no
// done blocks are pending.

// hover.exe:0x439E40
// FUNCTION: HOVER 0x439e40
void Hover_sub_439E40()
{
	int i;

	if (g_pUnk_4C59C0 != NULL)
		return;
	for (i = 0; i < 0x10; i++)
	{
		if (g_pUnk_4B6248->adwUnkF4[i] != 0 &&
			g_pUnk_4B6248->adwUnkF4[i] != 0xFFFFFFFF)
			return;
	}
	Hover_sub_438900(0);
}

// WaveMixPlay: validates the session and the MIXWAVE (same magic-word
// check and error reporting as Hover_sub_4393A0), picks the channel (the
// least recently used one when flag 0x2 is set), queues a new channel node
// on the session's circular wait list (dwUnk1B8) and, unless flag 0x8
// requests queue-only, moves every queued node onto its channel and tops
// up the waveOut block queue. Returns 0 on success, 4 when the session is
// inactive, 5 on an invalid parameter/session/wave/channel and 8 when the
// wave is not in the current output format.

// hover.exe:0x439E80
// FUNCTION: HOVER 0x439e80
int __stdcall Hover_sub_439E80(MIXPLAYPARAM* volatile pParams)
{
	SOUNDENGINE* pEngine;
	MIXWAVE* pWave;
	DWORD* pNode;
	DWORD* pTail;
	DWORD* pCur;
	DWORD* pNext;
	DWORD* ppSlot;
	DWORD** ppBlocks;
	DWORD dwStart;
	// The original's frame is exactly three dwords, all memory-resident
	// (volatile): the play position doubles as the return code - its slot
	// holds MMSYSERR_HANDLEBUSY on entry, the error code on every failure
	// path, the mixer position while the wait list is drained, and 0 on
	// success, and is loaded into eax as the result.
	volatile int bPaused;
	volatile DWORD dwPos;
	volatile int bQueued;
	int nChannel;
	int nCount;
	int nRetry;
	int i;
	int bValidWave;

	// MMSYSERR_HANDLEBUSY: what the re-entrancy guard below returns.
	dwPos = 0xC;
	bQueued = 0;
	bPaused = 0;
	Hover_sub_438820(2, "<Play>");
	g_dwUnk_4C5AD4++;
	if ((int)g_dwUnk_4C5AD4 > 1)
		goto Done;
	if (pParams == NULL)
	{
		if (g_dwUnk_4C59B8 != 0)
			MessageBoxA(NULL,
				"NULL parameters pointer passed to WaveMixPlay!",
				"WavMix32", MB_ICONEXCLAMATION);
		dwPos = 5;
		goto Done;
	}
	pEngine = pParams->pSession;
	if (pEngine != NULL && pEngine->wMagic00 == 0x5432 &&
		pEngine->wMagic1BC == 0x5432)
	{
		g_pUnk_4B6248 = pEngine;
	}
	else
	{
		MessageBeep(0xFFFFFFFF);
		wsprintfA(g_szUnk_4C59D0,
			"Invalid session handle 0x%04X passed to WaveMix API",
			pEngine);
		Hover_sub_438820(1, g_szUnk_4C59D0);
		MessageBoxA(NULL, g_szUnk_4C59D0, "WavMix32",
			MB_ICONEXCLAMATION);
		g_pUnk_4B6248 = NULL;
	}
	if (g_pUnk_4B6248 == NULL)
	{
		if (g_dwUnk_4C59B8 != 0)
			MessageBoxA(NULL,
				"Invalid session handle passed to WaveMixPlay",
				"WavMix32", MB_ICONEXCLAMATION);
		dwPos = 5;
		goto Done;
	}
	// Split into two statements on purpose (rule 66): this is what gives
	// the original's branchless "mov ax / sub ax,MAGIC / cmp ax,1 / sbb /
	// neg" zero test instead of a compare chain.
	pWave = (MIXWAVE*)pParams->pMixWave;
	// Spelled with the NULL case first: hover.exe falls through into the
	// zero arm and jumps over it when the pointer is good (rule 175).
	bValidWave = (pWave == NULL) ? 0 : (pWave->wMagic == 0x5432);
	if (bValidWave == 0)
	{
		if (g_dwUnk_4C59B8 != 0)
			MessageBoxA(NULL,
				"Invalid or NULL wave pointer passed to WaveMixPlay!",
				"WavMix32", MB_ICONEXCLAMATION);
		dwPos = 5;
		goto Done;
	}
	if (memcmp(pWave, &g_pUnk_4B6248->wfxUnk17C, 0x10) != 0)
	{
		wsprintfA(g_szUnk_4C59D0,
			"The LPMIXWAVE 0x%lx is not in the current output format, "
			"close the wave and reopen it.", pWave);
		Hover_sub_438820(1, g_szUnk_4C59D0);
		if (g_dwUnk_4C59B8 != 0)
			MessageBoxA(NULL, g_szUnk_4C59D0, "WavMix32",
				MB_ICONEXCLAMATION);
		dwPos = 8;
		goto Done;
	}
	if (g_pUnk_4B6248->dwUnk10 == 0)
	{
		if (g_dwUnk_4C59B8 != 0)
			MessageBoxA(NULL,
				"Wave device not allocated, call "
				"WaveMixActivate(hMixSession,TRUE)",
				"WavMix32", MB_ICONEXCLAMATION);
		dwPos = 4;
		goto Done;
	}
	if (g_pUnk_4B6248->dwUnk134 == 0)
	{
		if (g_dwUnk_4C59B8 != 0)
			MessageBoxA(NULL,
				"You must open a channel before you can play a wave!",
				"WavMix32", MB_ICONEXCLAMATION);
		dwPos = 5;
		goto Done;
	}
	if ((pParams->dwFlags & 2) != 0)
	{
		// pick the least recently used open channel
		Hover_sub_438820(2, "<LRU>");
		nChannel = 0;
		for (i = 0; i < 0x10; i++)
		{
			if (g_pUnk_4B6248->adwUnkF4[i] == 0xFFFFFFFF)
				continue;
			if (g_pUnk_4B6248->adwUnkF4[nChannel] == 0)
				break;
			if (i == nChannel)
				continue;
			if (g_pUnk_4B6248->adwUnk138[nChannel] >
				g_pUnk_4B6248->adwUnk138[i])
				nChannel = i;
		}
		pParams->nChannel = nChannel;
	}
	else
	{
		nChannel = pParams->nChannel;
	}
	g_pUnk_4B6248->dwUnk178++;
	g_pUnk_4B6248->adwUnk138[nChannel] = g_pUnk_4B6248->dwUnk178;
	if (g_pUnk_4B6248->adwUnkF4[nChannel] == 0xFFFFFFFF)
	{
		dwPos = 5;
		goto Done;
	}
	pNode = (DWORD*)Hover_sub_438CD0();
	if (pNode == NULL)
		goto Done;
	memcpy(pNode + 1, pParams, 0x1C);
	pNode[0x20 / 4] = (DWORD)((MIXPLAYPARAM*)(pNode + 1))->pMixWave;
	pWave = (MIXWAVE*)pNode[0x20 / 4];
	pNode[0x24 / 4] = pWave->cbData;
	pNode[0x30 / 4] = (DWORD)pWave->pData;
	pNode[0x34 / 4] = pWave->cbData -
		g_pUnk_4B6248->wfxUnk17C.wf.nBlockAlign + (DWORD)pWave->pData;
	((MIXPLAYPARAM*)(pNode + 1))->nChannel = nChannel;
	// The kind character is computed in a register at the push site in the
	// original (and,cmp,sbb,and,add on the flag word), not through a local.
	wsprintfA(g_szUnk_4C59D0, "(%d:%s L%u %c)", nChannel, pWave->szName,
		((MIXPLAYPARAM*)(pNode + 1))->wLoops,
		((((MIXPLAYPARAM*)(pNode + 1))->dwFlags & 8) != 0) ? 'W' : '!');
	Hover_sub_438820(2, g_szUnk_4C59D0);
	// queue the node on the session's circular wait list
	pTail = (DWORD*)g_pUnk_4B6248->dwUnk1B8;
	if (pTail != NULL)
	{
		pNode[0] = *pTail;
		*pTail = (DWORD)pNode;
		g_pUnk_4B6248->dwUnk1B8 = (DWORD)pNode;
	}
	else
	{
		g_pUnk_4B6248->dwUnk1B8 = (DWORD)pNode;
		pNode[0] = (DWORD)pNode;
	}
	if ((((MIXPLAYPARAM*)(pNode + 1))->dwFlags & 8) != 0)
	{
		Hover_sub_438820(2, "wait list");
		goto Success;
	}
	Hover_sub_439E40();
	if (g_pUnk_4B6248->dwUnk1B0 == (DWORD)Hover_sub_439C20)
		dwPos = Hover_sub_438840(g_pUnk_4B6248->dwUnk0C,
			g_pUnk_4B6248->dwUnk1A4);
	else
		dwPos = g_pUnk_4B6248->dwUnk19C;
	while (g_pUnk_4B6248->dwUnk1B8 != 0)
	{
		// dequeue the oldest wait-list node
		pTail = (DWORD*)g_pUnk_4B6248->dwUnk1B8;
		pNode = (DWORD*)*pTail;
		if (pNode == pTail)
			g_pUnk_4B6248->dwUnk1B8 = 0;
		else
			*pTail = *pNode;
		pNode[0] = 0;
		nChannel = ((MIXPLAYPARAM*)(pNode + 1))->nChannel;
		if ((((MIXPLAYPARAM*)(pNode + 1))->dwFlags & 0x10) != 0)
		{
			((WORD*)pNode)[0x38 / 2] =
				((MIXPLAYPARAM*)(pNode + 1))->wVolume1;
			((WORD*)pNode)[0x3A / 2] =
				((MIXPLAYPARAM*)(pNode + 1))->wVolume2;
		}
		else
		{
			((WORD*)pNode)[0x38 / 2] =
				g_pUnk_4B6248->awUnkB4[nChannel * 2];
			((WORD*)pNode)[0x3A / 2] =
				g_pUnk_4B6248->awUnkB4[nChannel * 2 + 1];
		}
		if (((WORD*)pNode)[0x38 / 2] > 0xA)
			((WORD*)pNode)[0x38 / 2] = 0xA;
		if (((WORD*)pNode)[0x3A / 2] > 0xA)
			((WORD*)pNode)[0x3A / 2] = 0xA;
		if ((((MIXPLAYPARAM*)(pNode + 1))->dwFlags & 1) != 0)
		{
			// replace the channel's node list, recycling the old nodes
			pCur = (DWORD*)g_pUnk_4B6248->adwUnkF4[nChannel];
			while (pCur != NULL)
			{
				pNext = (DWORD*)*pCur;
				Hover_sub_438820(2, "{FreeCNode");
				if (pCur == NULL)
				{
					Hover_sub_438820(1, " FreeCN: NULL parameter!}");
				}
				else
				{
					*pCur = (DWORD)g_pUnk_4B6D50;
					g_pUnk_4B6D50 = pCur;
					Hover_sub_438820(2, "}");
				}
				pCur = pNext;
			}
			g_pUnk_4B6248->adwUnkF4[nChannel] = (DWORD)pNode;
			if (g_pUnk_4C59C0 != NULL)
				bQueued = 1;
			if ((((MIXPLAYPARAM*)(pNode + 1))->dwFlags & 4) != 0)
				pNode[0x28 / 4] = dwPos;
			else
				pNode[0x28 / 4] = g_pUnk_4B6248->dwUnk19C;
			goto SetStop;
		}
		// append the node after the channel's last queued node
		ppSlot = &g_pUnk_4B6248->adwUnkF4[nChannel];
		pCur = (DWORD*)*ppSlot;
		if (pCur != NULL)
		{
			while (*pCur != 0)
				pCur = (DWORD*)*pCur;
			*pCur = (DWORD)pNode;
			if (pCur[0x2C / 4] == 0xFFFFFFFF)
				Hover_sub_438820(1,
					"!!!! adding to channel at a position where will never get played");
			if ((((MIXPLAYPARAM*)(pNode + 1))->dwFlags & 4) != 0)
			{
				dwStart = pCur[0x2C / 4];
				if (dwStart < dwPos)
					dwStart = dwPos;
				pNode[0x28 / 4] = dwStart;
			}
			else
			{
				dwStart = g_pUnk_4B6248->dwUnk19C;
				if (dwStart < pCur[0x2C / 4])
					dwStart = pCur[0x2C / 4];
				pNode[0x28 / 4] = dwStart;
			}
		}
		else
		{
			*ppSlot = (DWORD)pNode;
			if ((((MIXPLAYPARAM*)(pNode + 1))->dwFlags & 4) != 0)
				pNode[0x28 / 4] = dwPos;
			else
				pNode[0x28 / 4] = g_pUnk_4B6248->dwUnk19C;
		}
		if (g_pUnk_4B6248->dwUnk19C > pNode[0x28 / 4])
			bQueued = 1;
SetStop:
		if (((MIXPLAYPARAM*)(pNode + 1))->wLoops == 0xFFFF)
			pNode[0x2C / 4] = 0xFFFFFFFF;
		else
			pNode[0x2C / 4] = pNode[0x24 / 4] *
				(((MIXPLAYPARAM*)(pNode + 1))->wLoops + 1) -
				g_pUnk_4B6248->wfxUnk17C.wf.nBlockAlign +
				pNode[0x28 / 4];
	}
	if (bQueued != 0)
	{
		if (((int (__cdecl*)(DWORD, int))g_pUnk_4B6248->dwUnk1B0)(
				dwPos, 0) != 0)
			goto Success;
	}
	nRetry = 0;
	if (g_pUnk_4C59C0 == NULL && (int)g_pUnk_4B6248->dwUnk194 > 0)
	{
		waveOutPause((HWAVEOUT)g_pUnk_4B6248->dwUnk0C);
		bPaused = 1;
	}
	for (;;)
	{
		// Same block scan as Hover_sub_439180: pointer walk, "i <
		// nCount" guard kept on the counter, "i >= nCount" for the
		// out-of-blocks arm.
		i = 0;
		nCount = g_pUnk_4B6248->dwUnk190;
		ppBlocks = (DWORD**)g_pUnk_4B6248->dwUnk198;
		while (i < nCount && (*ppBlocks)[0x20 / 4] == 0)
		{
			ppBlocks++;
			i++;
		}
		if (i >= nCount)
		{
			Hover_sub_438820(2, "{gwb:out of wav blks}");
			pNode = NULL;
		}
		else
		{
			pNode = *ppBlocks;
			pNode[1] = g_pUnk_4B6248->dwUnk18C;
			pNode[0] = (DWORD)pNode + 0x30;
			pNode[0x28 / 4] = (DWORD)g_pUnk_4B6F18;
			pNode[0x20 / 4] = 0;
		}
		if (Hover_sub_439560(pNode, 1) == 0)
			break;
		if (bPaused == 0)
			continue;
		nRetry++;
		if ((int)g_pUnk_4B6248->dwUnk194 > nRetry)
			continue;
		waveOutRestart((HWAVEOUT)g_pUnk_4B6248->dwUnk0C);
		bPaused = 0;
	}
	if (bPaused != 0)
		waveOutRestart((HWAVEOUT)g_pUnk_4B6248->dwUnk0C);
Success:
	dwPos = 0;
Done:
	g_dwUnk_4C5AD4--;
	return (int)dwPos;
}

// Converts the sample data between 8- and 16-bit samples. Returns the new
// GlobalAlloc'd buffer (the input buffer is freed), the input buffer
// itself when no conversion is needed, or NULL on failure.

// hover.exe:0x43AA70
// FUNCTION: HOVER 0x43aa70
void* Hover_sub_43AA70(void* pData, WORD wBitsFrom, WORD wBitsTo,
	DWORD* pcbData)
{
	void* pNew;
	DWORD cbNew;
	int nSamples;
	int i;

	if (wBitsTo == wBitsFrom)
		return pData;
	if ((wBitsFrom != 8 && wBitsFrom != 0x10) ||
		(wBitsTo != 8 && wBitsTo != 0x10))
	{
		GlobalUnlock(GlobalHandle(pData));
		GlobalFree(GlobalHandle(pData));
		return NULL;
	}
	wBitsFrom >>= 3;
	wBitsTo >>= 3;
	nSamples = *pcbData / wBitsFrom;
	*pcbData = cbNew = (DWORD)wBitsTo * nSamples;
	pNew = GlobalLock(GlobalAlloc(GMEM_MOVEABLE | GMEM_DDESHARE, cbNew));
	if (pNew == NULL)
	{
		if (g_dwUnk_4C59B8 != 0)
			MessageBoxA(NULL,
				"Unable to allocate memory for waveform data.  "
				"Try making more memory available by closing other applications.",
				"WavMix32", MB_ICONINFORMATION);
		GlobalUnlock(GlobalHandle(pData));
		GlobalFree(GlobalHandle(pData));
		return NULL;
	}
	if (wBitsTo < wBitsFrom)
	{
		// 16-bit to 8-bit
		short* pSrc = (short*)pData;
		BYTE* pDst = (BYTE*)pNew;
		i = nSamples;
		while (i != 0)
		{
			*pDst++ = (BYTE)(*pSrc++ / 256 - 128);
			i--;
		}
	}
	else
	{
		// 8-bit to 16-bit
		char* pSrc = (char*)pData;
		short* pDst = (short*)pNew;
		i = nSamples;
		while (i != 0)
		{
			*pDst++ = (short)(*pSrc++ + 0x80) << 8;
			i--;
		}
	}
	GlobalUnlock(GlobalHandle(pData));
	GlobalFree(GlobalHandle(pData));
	return pNew;
}

// Converts the sample data between mono and stereo. Returns the new
// GlobalAlloc'd buffer (the input buffer is freed), the input buffer
// itself when no conversion is needed, or NULL on failure.

// hover.exe:0x43ABC0
// FUNCTION: HOVER 0x43abc0
void* Hover_sub_43ABC0(void* pData, WORD wChFrom, WORD wChTo, WORD wBytes,
	DWORD* pcbData)
{
	void* pNew;
	DWORD cbNew;
	int nSamples;
	int i;

	if (wChTo == wChFrom)
		return pData;
	nSamples = *pcbData / wBytes / wChFrom;
	*pcbData = cbNew = (DWORD)wChTo * nSamples * wBytes;
	pNew = GlobalLock(GlobalAlloc(GMEM_MOVEABLE | GMEM_DDESHARE, cbNew));
	if (pNew == NULL)
	{
		if (g_dwUnk_4C59B8 != 0)
			MessageBoxA(NULL,
				"Unable to allocate memory for waveform data.  "
				"Try making more memory available by closing other applications.",
				"WavMix32", MB_ICONINFORMATION);
		GlobalUnlock(GlobalHandle(pData));
		GlobalFree(GlobalHandle(pData));
		return NULL;
	}
	if (wChTo > wChFrom)
	{
		// mono to stereo: duplicate every sample
		if (wBytes == 1)
		{
			BYTE* pSrc = (BYTE*)pData;
			BYTE* pDst = (BYTE*)pNew;
			i = nSamples;
			while (i != 0)
			{
				*pDst++ = *pSrc;
				*pDst++ = *pSrc++;
				i--;
			}
		}
		else
		{
			WORD* pSrc = (WORD*)pData;
			WORD* pDst = (WORD*)pNew;
			i = nSamples;
			while (i != 0)
			{
				*pDst++ = *pSrc;
				*pDst++ = *pSrc++;
				i--;
			}
		}
	}
	else
	{
		// stereo to mono: average the channel pair
		if (wBytes == 1)
		{
			BYTE* pSrc = (BYTE*)pData;
			BYTE* pDst = (BYTE*)pNew;
			i = nSamples;
			while (i != 0)
			{
				*pDst++ = (BYTE)(((int)pSrc[1] + pSrc[0]) / 2);
				pSrc += 2;
				i--;
			}
		}
		else
		{
			short* pSrc = (short*)pData;
			short* pDst = (short*)pNew;
			i = nSamples;
			while (i != 0)
			{
				*pDst++ = (short)(((int)pSrc[1] + pSrc[0]) / 2);
				pSrc += 2;
				i--;
			}
		}
	}
	GlobalUnlock(GlobalHandle(pData));
	GlobalFree(GlobalHandle(pData));
	return pNew;
}

// Averages nRatio consecutive frames of pSrc into one frame of pDst,
// channel by channel (downsampling half of Hover_sub_43AEC0). Note the
// 16-bit path never advances pDst between channels in the original.

// hover.exe:0x43AD40
// FUNCTION: HOVER 0x43ad40
void Hover_sub_43AD40(BYTE* pDst, BYTE* pSrc, int nRatio, int nBytes,
	int nChannels)
{
	int nSum;
	int i;
	int j;

	if (nBytes == 1)
	{
		for (i = nChannels; i > 0; i--)
		{
			nSum = 0;
			pSrc++;
			for (j = nRatio; j > 0; j--)
			{
				nSum += pSrc[-1] - 0x80;
				pSrc += nChannels;
			}
			*pDst++ = (BYTE)(nSum / nRatio - 0x80);
		}
	}
	else
	{
		for (i = nChannels; i > 0; i--)
		{
			nSum = 0;
			pSrc += 2;
			for (j = nRatio; j > 0; j--)
			{
				nSum += ((short*)pSrc)[-1];
				pSrc += nChannels * 2;
			}
			*(short*)pDst = (short)(nSum / nRatio);
		}
	}
}

// Interpolates one frame of pSrc out to nRatio frames of pDst, channel by
// channel (upsampling half of Hover_sub_43AEC0). Note the 16-bit path
// reads the "next" sample from the destination buffer in the original.

// hover.exe:0x43ADF0
// FUNCTION: HOVER 0x43adf0
void Hover_sub_43ADF0(BYTE* pDst, BYTE* pSrc, int nRatio, int nBytes,
	int nChannels)
{
	BYTE* pOut;
	BYTE bVal;
	int nAcc;
	int nStep;
	int i;
	int j;

	if (nBytes == 1)
	{
		for (i = nChannels; i > 0; i--)
		{
			pOut = pDst;
			bVal = pSrc[0];
			nStep = ((int)pSrc[nChannels] - bVal) / nRatio;
			pDst++;
			*pOut = bVal;
			pOut += nChannels;
			if (nRatio > 1)
			{
				for (j = nRatio - 1; j > 0; j--)
				{
					bVal += (BYTE)nStep;
					*pOut = bVal;
					pOut += nChannels;
				}
			}
			pSrc++;
		}
	}
	else
	{
		short* pDstW = (short*)pDst;
		short* pSrcW = (short*)pSrc;
		for (i = nChannels; i > 0; i--)
		{
			pOut = (BYTE*)pDstW;
			nAcc = pSrcW[0];
			nStep = (((short*)pOut)[nChannels] - nAcc) / nRatio;
			pDstW++;
			*(short*)pOut = (short)nAcc;
			pOut += nChannels * 2;
			if (nRatio > 1)
			{
				for (j = nRatio - 1; j > 0; j--)
				{
					nAcc += nStep;
					*(short*)pOut = (short)nAcc;
					pOut += nChannels * 2;
				}
			}
			pSrcW++;
		}
	}
}

// Converts the sampling rate of the sample data. Returns the new
// GlobalAlloc'd buffer (the input buffer is freed), the input buffer
// itself when the rates match, or NULL on failure.

// hover.exe:0x43AEC0
// FUNCTION: HOVER 0x43aec0
void* Hover_sub_43AEC0(void* pData, DWORD dwRateFrom, DWORD dwRateTo,
	WORD wBytes, WORD wChannels, DWORD* pcbData)
{
	void* pNew;
	BYTE* pSrc;
	BYTE* pDst;
	BYTE* p;
	DWORD cbNew;
	DWORD dwFrames;
	int nRatio;
	int nSkip;
	int nOut;
	int i;
	int j;

	if (dwRateTo == dwRateFrom)
		return pData;
	dwFrames = *pcbData / ((DWORD)wChannels * wBytes);
	if (dwRateTo > dwRateFrom)
	{
		nRatio = dwRateTo / dwRateFrom;
		nSkip = 0;
		nOut = nRatio * dwFrames;
	}
	else
	{
		nRatio = 0;
		nSkip = dwRateFrom / dwRateTo;
		nOut = dwFrames / nSkip;
	}
	*pcbData = cbNew = (DWORD)wChannels * wBytes * nOut;
	pNew = GlobalLock(GlobalAlloc(GMEM_MOVEABLE | GMEM_DDESHARE, cbNew));
	if (pNew == NULL)
	{
		if (g_dwUnk_4C59B8 != 0)
			MessageBoxA(NULL,
				"Unable to allocate memory for waveform data.  "
				"Try making more memory available by closing other applications.",
				"WavMix32", MB_ICONINFORMATION);
		GlobalUnlock(GlobalHandle(pData));
		GlobalFree(GlobalHandle(pData));
		return NULL;
	}
	pSrc = (BYTE*)pData;
	pDst = (BYTE*)pNew;
	if (nRatio > 0)
	{
		// upsample: interpolate between adjacent frames, then repeat
		// the last frame
		i = dwFrames - 1;
		while (i-- != 0)
		{
			Hover_sub_43ADF0(pDst, pSrc, nRatio, wBytes, wChannels);
			pDst += nRatio * wChannels * wBytes;
			pSrc += wChannels * wBytes;
		}
		for (i = nRatio; i > 0; i--)
		{
			p = pSrc;
			for (j = wChannels * wBytes; j > 0; j--)
				*pDst++ = *p++;
		}
	}
	else
	{
		// downsample: average each group of nSkip frames
		i = nOut - 1;
		while (i-- != 0)
		{
			Hover_sub_43AD40(pDst, pSrc, nSkip, wBytes, wChannels);
			pDst += wChannels * wBytes;
			pSrc += nSkip * wChannels * wBytes;
		}
		for (j = wChannels * wBytes; j > 0; j--)
			*pDst++ = *pSrc++;
	}
	GlobalUnlock(GlobalHandle(pData));
	GlobalFree(GlobalHandle(pData));
	return pNew;
}

// Converts pData from the wave file's format to the session's output
// format (sample depth, then channels, then sampling rate). Returns the
// converted buffer, or NULL on failure.

// hover.exe:0x43B0A0
// FUNCTION: HOVER 0x43b0a0
void* Hover_sub_43B0A0(LPWAVEFORMATEX pwfxSession, LPWAVEFORMATEX pwfxWave,
	void* pData, DWORD* pcbData)
{
	void* pConv;
	int nResult;

	if (pwfxWave->nChannels == pwfxSession->nChannels &&
		pwfxWave->nSamplesPerSec == pwfxSession->nSamplesPerSec &&
		((PCMWAVEFORMAT*)pwfxWave)->wBitsPerSample ==
			((PCMWAVEFORMAT*)pwfxSession)->wBitsPerSample)
		return pData;
	pConv = Hover_sub_43AA70(pData,
		((PCMWAVEFORMAT*)pwfxWave)->wBitsPerSample,
		((PCMWAVEFORMAT*)pwfxSession)->wBitsPerSample, pcbData);
	if (pConv == NULL)
		return NULL;
	pConv = Hover_sub_43ABC0(pConv, pwfxWave->nChannels,
		pwfxSession->nChannels,
		((PCMWAVEFORMAT*)pwfxSession)->wBitsPerSample >> 3, pcbData);
	if (pConv == NULL)
		return NULL;
	nResult = (int)Hover_sub_43AEC0(pConv, pwfxWave->nSamplesPerSec,
		pwfxSession->nSamplesPerSec,
		((PCMWAVEFORMAT*)pwfxSession)->wBitsPerSample >> 3,
		pwfxSession->nChannels, pcbData);
	// the original normalizes the result through a cmp/adc/and sequence
	return (nResult != 0) ? (void*)nResult : NULL;
}

// WaveMixOpenWave: loads a wave from a file (default), a "WAVE" resource
// (flag 0x2) or a caller-filled MMIOINFO (flag 0x4), converts the sample
// data to the session's output format and wraps it in a 0x42-byte MIXWAVE
// object. Returns the MIXWAVE, or NULL on failure.

// hover.exe:0x43B150
// FUNCTION: HOVER 0x43b150
void* __stdcall Hover_sub_43B150(SOUNDENGINE* pEngine, const char* pszWave,
	HMODULE hModule, DWORD dwFlags)
{
	MMIOINFO mmioInfo;
	MMCKINFO ckRiff;
	MMCKINFO ckChunk;
	MIXWAVE* pWave;
	HWAVEOUT hWaveOut;
	HRSRC hResInfo;
	HGLOBAL hResData;
	HMMIO hMMio;
	BYTE* pData;
	DWORD cbData;
	WORD uDevice;		// 16-bit in the original: "mov ax,0" / "mov ax,[..+0x18]"
	int nLen;

	hMMio = NULL;
	Hover_sub_438820(2, "<WaveMixOpenWave>");
	if (pEngine != NULL && pEngine->wMagic00 == 0x5432 &&
		pEngine->wMagic1BC == 0x5432)
	{
		g_pUnk_4B6248 = pEngine;
	}
	else
	{
		MessageBeep(0xFFFFFFFF);
		wsprintfA(g_szUnk_4C59D0,
			"Invalid session handle 0x%04X passed to WaveMix API",
			pEngine);
		Hover_sub_438820(1, g_szUnk_4C59D0);
		MessageBoxA(NULL, g_szUnk_4C59D0, "WavMix32",
			MB_ICONEXCLAMATION);
		g_pUnk_4B6248 = NULL;
	}
	if (g_pUnk_4B6248 != NULL)
		uDevice = (WORD)g_pUnk_4B6248->dwUnk18;
	else
		uDevice = 0;
	if (waveOutOpen(&hWaveOut, uDevice,
			(LPWAVEFORMATEX)&g_pUnk_4B6248->wfxUnk17C, 0, 0,
			WAVE_FORMAT_QUERY) != 0)
	{
		if (g_dwUnk_4C59B8 != 0)
			MessageBoxA(NULL,
				"The waveform device can't play this format.",
				"WavMix32", MB_ICONEXCLAMATION);
		return NULL;
	}
	pWave = (MIXWAVE*)GlobalLock(GlobalAlloc(GMEM_ZEROINIT | GMEM_DDESHARE,
		sizeof(MIXWAVE)));
	if (pWave == NULL)
	{
		if (g_dwUnk_4C59B8 != 0)
			MessageBoxA(NULL,
				"Unable to allocate memory for waveform data.  "
				"Try making more memory available by closing other applications.",
				"WavMix32", MB_ICONINFORMATION);
		return NULL;
	}
	hResData = NULL;
	pData = NULL;
	if ((dwFlags & 2) != 0)
	{
		hResInfo = FindResourceA(hModule, pszWave, "WAVE");
		hResData = LoadResource(hModule, hResInfo);
		if (hResData == NULL)
		{
			if (HIWORD(pszWave) != 0)
				wsprintfA(g_szUnk_4C59D0,
					"Failed to open 'WAVE' resource '%s'.", pszWave);
			else
				wsprintfA(g_szUnk_4C59D0,
					"Failed to open 'WAVE' resource %u.",
					(DWORD)LOWORD(pszWave));
			if (g_dwUnk_4C59B8 != 0)
				MessageBoxA(NULL, g_szUnk_4C59D0, "WavMix32",
					MB_ICONEXCLAMATION);
			goto Error;
		}
		memset(&mmioInfo, 0, sizeof(mmioInfo));
		mmioInfo.pchBuffer = (HPSTR)LockResource(hResData);
		if (mmioInfo.pchBuffer == NULL)
		{
			if (g_dwUnk_4C59B8 != 0)
				MessageBoxA(NULL, "Failed to lock 'WAVE' resource",
					"WavMix32", MB_ICONEXCLAMATION);
			FreeResource(hResData);
			hResData = NULL;
			goto Error;
		}
		mmioInfo.cchBuffer = SizeofResource(hModule, hResInfo);
		mmioInfo.fccIOProc = FOURCC_MEM;
		mmioInfo.adwInfo[0] = 0;
		hMMio = mmioOpenA(NULL, &mmioInfo, MMIO_READ);
		if (hMMio == NULL)
		{
			if (g_dwUnk_4C59B8 != 0)
			{
				wsprintfA(g_szUnk_4C59D0,
					"Failed to open resource, mmioOpen error=%u.\n"
					"May need to make sure resource is marked read-write",
					mmioInfo.wErrorRet);
				MessageBoxA(NULL, g_szUnk_4C59D0, "WavMix32",
					MB_ICONEXCLAMATION);
			}
			goto Error;
		}
	}
	else if ((dwFlags & 4) != 0)
	{
		mmioInfo = *(MMIOINFO*)pszWave;
		hMMio = mmioOpenA(NULL, &mmioInfo, MMIO_READ);
		if (hMMio == NULL)
		{
			if (g_dwUnk_4C59B8 != 0)
			{
				wsprintfA(g_szUnk_4C59D0,
					"Failed to open memory file, mmioOpen error=%u.\n"
					"May need to make sure memory is read-write",
					mmioInfo.wErrorRet);
				MessageBoxA(NULL, g_szUnk_4C59D0, "WavMix32",
					MB_ICONEXCLAMATION);
			}
			goto Error;
		}
	}
	else
	{
		hMMio = mmioOpenA((char*)pszWave, NULL,
			MMIO_READ | MMIO_ALLOCBUF);
		if (hMMio == NULL)
		{
			if (g_dwUnk_4C59B8 != 0)
			{
				wsprintfA(g_szUnk_4C59D0, "Failed to open wave file %s.",
					pszWave);
				MessageBoxA(NULL, g_szUnk_4C59D0, "WavMix32",
					MB_ICONEXCLAMATION);
			}
			goto Error;
		}
	}
	ckRiff.fccType = mmioFOURCC('W', 'A', 'V', 'E');
	if (mmioDescend(hMMio, &ckRiff, NULL, MMIO_FINDRIFF) != 0)
	{
		if (g_dwUnk_4C59B8 != 0)
			MessageBoxA(NULL, "This is not a WAVE file.", "WavMix32",
				MB_ICONEXCLAMATION);
		goto Error;
	}
	ckChunk.ckid = mmioFOURCC('f', 'm', 't', ' ');
	if (mmioDescend(hMMio, &ckChunk, &ckRiff, MMIO_FINDCHUNK) != 0)
	{
		if (g_dwUnk_4C59B8 != 0)
			MessageBoxA(NULL, "WAVE file is corrupted.", "WavMix32",
				MB_ICONEXCLAMATION);
		goto Error;
	}
	if (ckChunk.cksize != 0x10)
		Hover_sub_438820(2,
			"format size is not the size of PCMWAVEFORMAT!");
	if (mmioRead(hMMio, (HPSTR)pWave, 0x10) != 0x10)
	{
		if (g_dwUnk_4C59B8 != 0)
			MessageBoxA(NULL, "Failed to read format chunk.", "WavMix32",
				MB_ICONEXCLAMATION);
		goto Error;
	}
	if (pWave->wfx.wf.wFormatTag != WAVE_FORMAT_PCM)
	{
		if (g_dwUnk_4C59B8 != 0)
			MessageBoxA(NULL, "The file is not a PCM file.", "WavMix32",
				MB_ICONEXCLAMATION);
		goto Error;
	}
	mmioAscend(hMMio, &ckChunk, 0);
	ckChunk.ckid = mmioFOURCC('d', 'a', 't', 'a');
	if (mmioDescend(hMMio, &ckChunk, &ckRiff, MMIO_FINDCHUNK) != 0)
	{
		if (g_dwUnk_4C59B8 != 0)
			MessageBoxA(NULL, "WAVE file has no data chunk.", "WavMix32",
				MB_ICONEXCLAMATION);
		goto Error;
	}
	cbData = ckChunk.cksize;
	if (cbData == 0)
	{
		if (g_dwUnk_4C59B8 != 0)
			MessageBoxA(NULL, "The data chunk has no data.", "WavMix32",
				MB_ICONEXCLAMATION);
		goto Error;
	}
	pData = (BYTE*)GlobalLock(GlobalAlloc(GMEM_MOVEABLE | GMEM_DDESHARE,
		cbData));
	if (pData == NULL)
	{
		if (g_dwUnk_4C59B8 != 0)
			MessageBoxA(NULL,
				"Unable to allocate memory for waveform data.  "
				"Try making more memory available by closing other applications.",
				"WavMix32", MB_ICONINFORMATION);
		goto Error;
	}
	if (mmioRead(hMMio, (HPSTR)pData, cbData) != (LONG)cbData)
	{
		if (g_dwUnk_4C59B8 != 0)
			MessageBoxA(NULL, "Failed to read data chunk.", "WavMix32",
				MB_ICONEXCLAMATION);
		goto Error;
	}
	pData = (BYTE*)Hover_sub_43B0A0(
		(LPWAVEFORMATEX)&g_pUnk_4B6248->wfxUnk17C, (LPWAVEFORMATEX)pWave,
		pData, &cbData);
	if (pData == NULL)
	{
		if (g_dwUnk_4C59B8 != 0)
			MessageBoxA(NULL, "Failed to convert wave format.", "WavMix32",
				MB_ICONEXCLAMATION);
		goto Error;
	}
	mmioClose(hMMio, 0);
	if (hResData != NULL)
		FreeResource(hResData);
	pWave->pData = pData;
	pWave->cbData = cbData;
	pWave->dwUnk20 = 0;
	pWave->dwUnk24 = 0;
	pWave->dwUnk1C = 0;
	pWave->wMagic = 0x5432;
	memmove(pWave, &g_pUnk_4B6248->wfxUnk17C, 0x10);
	if (HIWORD(pszWave) == 0)
	{
		wsprintfA(pWave->szName, "res#%u", (DWORD)LOWORD(pszWave));
	}
	else
	{
		nLen = lstrlenA(pszWave);
		if (nLen > 0xF)
			nLen -= 0xF;
		else
			nLen = 0;
		lstrcpyA(pWave->szName, pszWave + nLen);
	}
	return pWave;
Error:
	if (hMMio != NULL)
		mmioClose(hMMio, 0);
	if (pWave != NULL)
	{
		GlobalUnlock(GlobalHandle(pWave));
		GlobalFree(GlobalHandle(pWave));
	}
	if (pData != NULL)
	{
		GlobalUnlock(GlobalHandle(pData));
		GlobalFree(GlobalHandle(pData));
	}
	if (hResData != NULL)
		FreeResource(hResData);
	return NULL;
}

// WaveMixFreeWave: closes a wave opened by Hover_sub_43B150. Declared here
// rather than in hover.h because only this module needs it (adding a free
// function to hover.h perturbs every other translation unit; see
// notes/perfect-match.md rule 20).
int __stdcall Hover_sub_43A860(SOUNDENGINE* pEngine, MIXWAVE* pWave);

// Validates the session (same magic-word check and error reporting as
// Hover_sub_4393A0, except that this one does *not* publish the session in
// g_pUnk_4B6248 - it keeps it in the parameter instead) and the wave, then
// unlinks from all 16 channel queues every node still pointing at this
// wave, returning those nodes to the g_pUnk_4B6D50 free list (the
// Hover_sub_438D00-style "{FreeCNode" expansion, inlined twice), and
// finally releases the sample data and the MIXWAVE itself. Returns 0 on
// success, 5 on an invalid session or wave handle.

// hover.exe:0x43A860
// FUNCTION: HOVER 0x43a860
int __stdcall Hover_sub_43A860(SOUNDENGINE* pEngine, MIXWAVE* pWave)
{
	DWORD* pdwChannel;
	DWORD* pNode;
	DWORD* pNext;
	DWORD* pLast;
	int bValidWave;
	int i;

	Hover_sub_438820(2, "<WaveMixFreeWave>");
	if (!(pEngine != NULL && pEngine->wMagic00 == 0x5432 &&
		pEngine->wMagic1BC == 0x5432))
	{
		MessageBeep(0xFFFFFFFF);
		wsprintfA(g_szUnk_4C59D0,
			"Invalid session handle 0x%04X passed to WaveMix API",
			pEngine);
		Hover_sub_438820(1, g_szUnk_4C59D0);
		MessageBoxA(NULL, g_szUnk_4C59D0, "WavMix32",
			MB_ICONEXCLAMATION);
		pEngine = NULL;
	}
	if (pEngine == NULL)
		return 5;
	// Split into two statements on purpose: written as one
	// short-circuit expression VC 2.2 emits a cmp against the memory
	// operand plus two constant loads, while the separate assignment
	// gives the original's branchless "mov ax / sub ax,MAGIC /
	// cmp ax,1 / sbb / neg" zero test (perfect-match.md rule 66).
	bValidWave = 0;
	if (pWave != NULL)
		bValidWave = (pWave->wMagic == 0x5432);
	if (bValidWave == 0)
		return 5;
	i = 0x10;
	pdwChannel = pEngine->adwUnkF4;
	do
	{
		pNode = (DWORD*)*pdwChannel;
		if (pNode != (DWORD*)0xFFFFFFFF)
		{
			pLast = NULL;
			while (pNode != NULL)
			{
				if (pNode[0x20 / 4] == (DWORD)pWave)
				{
					if (pLast == NULL)
					{
						pNext = (DWORD*)pNode[0];
						pNode = (DWORD*)*pdwChannel;
						Hover_sub_438820(2, "{FreeCNode");
						if (pNode == NULL)	// never taken; the
						{			// list head is this node
							Hover_sub_438820(1,
								" FreeCN: NULL parameter!}");
						}
						else
						{
							pNode[0] = (DWORD)g_pUnk_4B6D50;
							g_pUnk_4B6D50 = pNode;
							Hover_sub_438820(2, "}");
						}
						*pdwChannel = (DWORD)pNext;
						pNode = pNext;
					}
					else
					{
						pLast[0] = pNode[0];
						Hover_sub_438820(2, "{FreeCNode");
						if (pNode == NULL)	// never taken; the
						{			// loop condition proved it
							Hover_sub_438820(1,
								" FreeCN: NULL parameter!}");
						}
						else
						{
							pNode[0] = (DWORD)g_pUnk_4B6D50;
							g_pUnk_4B6D50 = pNode;
							Hover_sub_438820(2, "}");
						}
						pNode = (DWORD*)pLast[0];
					}
				}
				else
				{
					pLast = pNode;
					pNode = (DWORD*)pNode[0];
				}
			}
		}
		pdwChannel++;
		i--;
	}
	while (i != 0);
	if (pWave->pData != NULL)
	{
		GlobalUnlock(GlobalHandle(pWave->pData));
		GlobalFree(GlobalHandle(pWave->pData));
	}
	pWave->wMagic = 0;
	GlobalUnlock(GlobalHandle(pWave));
	GlobalFree(GlobalHandle(pWave));
	return 0;
}
// MM_WOM_DONE completion, called from CMainFrame::OnWomDone with the
// MIXWAVE the mixer has finished with. Only the slots that open their wave
// on every play (CSub40B800::m_dwUnk18 != 0) register themselves in
// g_mapUnk_4A4820, so finding one there means the wave was opened for this
// play and can be closed again. The Lookup return value is ignored: the
// original tests the (pre-zeroed) output instead.

// hover.exe:0x42D760
// FUNCTION: HOVER 0x42d760
void Hover_sub_42D760(void* pMixWave)
{
	CSub40B800* pSlot;
	void* pWave;

	pWave = pMixWave;
	pSlot = NULL;
	if (g_dwUnk_4C56D8 == 0)
		return;
	if (g_pUnk_4C56DC == NULL)
		return;

	g_mapUnk_4A4820.Lookup(pWave, (void*&)pSlot);
	if (pSlot == NULL)
		return;
	g_mapUnk_4A4820.RemoveKey(pWave);

	if (pSlot->m_dwUnk18 == 0)
		return;
	if (pSlot->m_pUnk08 != pWave)
		return;
	Hover_sub_43A860(g_pUnk_4C56DC, (MIXWAVE*)pSlot->m_pUnk08);
	pSlot->m_pUnk08 = NULL;
}

// Builds the fixed-point sine and tangent lookup tables used by the
// rendering engine. Entry 0 of each table is left zeroed.

// hover.exe:0x438540
// FUNCTION: HOVER 0x438540
void Hover_sub_438540()
{
	double dStep = atan(1.0) * 4.0;		// pi
	for (int i = 0; i < 0x2000; i++)
		g_adwSinTable[i + 1] =
			(DWORD)(sin(i * dStep * (1.0 / 4096)) * 65536.0);
	for (int j = 0; j < 0x4000; j++)
	{
		double dAngle = j * (1.0 / 256);
		dAngle = dAngle * dStep * (1.0 / 256);
		g_awTanTable[j + 1] = (WORD)(tan(dAngle) * 65536.0);
	}
}

// Computes the fixed-point angle of the vector (lX, lY): scalar variant of
// Hover_sub_4386D0, returning the 24.8 fixed-point angle (0x1000000 = full
// circle) directly. The ratio divide here is a plain (x << 16) / y, not
// the HOVER_FIXDIVP macro.

// hover.exe:0x4385F0
// FUNCTION: HOVER 0x4385f0
DWORD Hover_sub_4385F0(long lY, long lX)
{
	int nFlags = 0;
	long lRatio;
	long lAngle;
	int nLo, nHi, nMid;

	if (lX < 0)
	{
		nFlags = 1;
		lX = -lX;
	}
	if (lY < 0)
	{
		nFlags |= 2;
		lY = -lY;
	}
	// The default (lX == lY) is assigned here, right before its one use,
	// rather than at the declaration: hover.exe defers the store to this
	// point instead of materializing it at function entry (rule 240).
	lAngle = 0x4000;
	if (lX != lY)
	{
		// Ratio of the smaller component to the larger one as a 16.16
		// fixed-point number.
		if (lX > lY)
			lRatio = (lY << 16) / lX;
		else
			lRatio = (lX << 16) / lY;

		nLo = 0;
		nHi = 0x3FFF;
		while (nHi >= nLo)
		{
			nMid = (nLo + nHi) / 2;
			if (g_awTanTable[nMid + 1] < (WORD)lRatio)
				nLo = nMid + 1;
			else if (g_awTanTable[nMid + 1] <= (WORD)lRatio)
				break;
			else
				nHi = nMid - 1;
		}
		lAngle = nMid;
		if (lX < lY)
			lAngle = 0x8000 - lAngle;
	}
	// Written with one shared return through a local rather than four
	// separate "return expr;" statements (rule 240): this is what gives
	// hover.exe's downstream register-allocation cascade elsewhere in
	// the file (rule 237) its correct shape.
	DWORD dwResult;
	if (nFlags == 1)
		dwResult = (0x10000 - lAngle) << 8;
	else if (nFlags == 2)
		dwResult = (0x20000 - lAngle) << 8;
	else if (nFlags == 3)
		dwResult = (lAngle + 0x10000) << 8;
	else
		dwResult = lAngle << 8;
	return dwResult;
}

// Computes the fixed-point angle of the vector (lX, lY): binary-searches
// the tangent table for the ratio of the smaller component to the larger
// one, then adjusts for the half and quadrant the vector lies in. The
// result is stored at *pdwOut as a 24.8 fixed-point angle (0x1000000 =
// full circle) and the output pointer is returned.

// hover.exe:0x4386D0
// FUNCTION: HOVER 0x4386d0
DWORD* Hover_sub_4386D0(DWORD* pdwOut, long lY, long lX)
{
	int nFlags = 0;
	long lRatio;
	long lAngle = 0x4000;
	int nLo, nHi, nMid;

	if (lX < 0)
	{
		nFlags = 1;
		lX = -lX;
	}
	if (lY < 0)
	{
		nFlags |= 2;
		lY = -lY;
	}
	if (lX != lY)
	{
		// Ratio of the smaller component to the larger one as a 16.16
		// fixed-point number.
		if (lX > lY)
		{
			lRatio = lY;
			HOVER_FIXDIVP(lRatio, lX);
		}
		else
		{
			lRatio = lX;
			HOVER_FIXDIVP(lRatio, lY);
		}

		nLo = 0;
		nHi = 0x3FFF;
		while (nHi >= nLo)
		{
			nMid = (nLo + nHi) / 2;
			if (g_awTanTable[nMid + 1] < (WORD)lRatio)
				nLo = nMid + 1;
			else if (g_awTanTable[nMid + 1] == (WORD)lRatio)
				break;
			else
				nHi = nMid - 1;
		}
		lAngle = nMid;
		if (lX < lY)
			lAngle = 0x8000 - lAngle;
	}
	if (nFlags == 1)
		lAngle = 0x10000 - lAngle;
	else if (nFlags == 2)
		lAngle = 0x20000 - lAngle;
	else if (nFlags == 3)
		lAngle += 0x10000;
	*pdwOut = lAngle << 8;
	return pdwOut;
}

// Approximates the magnitude of the vector from (lX1, lY1) to (lX2, lY2):
// the max + min/2 metric refined by one Newton step against the sum of
// the component squares. The squares use 16-bit multiplies (the full
// 32-bit product is reassembled from dx:ax), so only the low words of
// the components contribute.

// hover.exe:0x408A00
// FUNCTION: HOVER 0x408a00
DWORD Hover_sub_408A00(long lX1, long lY1, long lX2, long lY2)
{
	long lDX;
	long lDY;
	DWORD dwMin;
	DWORD dwMetric;
	DWORD dwSqX;
	DWORD dwSqY;
	DWORD dwDist;

	lDX = labs(lX2 - lX1);
	lDY = labs(lY2 - lY1);
	dwMin = (lDY < lDX) ? lDY : lDX;
	dwMetric = lDY - (dwMin >> 1) + lDX + 1;
	_asm {
		mov eax, lDX
		mul ax
		shl edx, 0x10
		add edx, eax
		mov dwSqX, edx
	}
	_asm {
		mov eax, lDY
		mul ax
		shl edx, 0x10
		add eax, edx
		mov dwSqY, eax
	}
	dwDist = (dwMetric + (dwSqY + dwSqX) / dwMetric) >> 1;
	return dwDist;
}

// Splits the velocity (dwVX, dwVY) into the component along the direction
// (sin(dwAngle), sin(dwAngle - 0x80)) (*pdwAlong) and the remainder
// (*pdwSide); used by the player collision response. All values are 16.16
// fixed-point; angles use 0x200 per full circle.

// hover.exe:0x40B6E0
// FUNCTION: HOVER 0x40b6e0
void Hover_sub_40B6E0(DWORD dwVX, DWORD dwVY, DWORD dwAngle,
	DWORD* pdwAlong, DWORD* pdwSide)
{
	volatile DWORD dwSinA;
	volatile DWORD dwSinB;
	DWORD dwV1;
	DWORD dwV2;
	volatile DWORD dwDot;
	DWORD dwT1;
	DWORD dwT2;
	long lAngle;

	lAngle = (long)dwAngle - 0x80;
	dwSinA = g_adwSinTable[((lAngle & 0x1FF) << 4) + 1];
	dwSinB = g_adwSinTable[(((lAngle + 0x80) & 0x1FF) << 4) + 1];
	// Volatile read: HOVER_FIXMULP writes dwV1 only through its pointer
	// temporary, which VC 2.1 does not model, so a plain `dwV1 = dwVY`
	// has its store deleted and the product dropped (rule 95).
	dwV1 = *(volatile DWORD*)&dwVY;
	HOVER_FIXMULP(dwV1, dwSinA);
	dwV2 = dwVX;
	HOVER_FIXMULP(dwV2, dwSinB);
	dwDot = dwV2;
	dwDot += dwV1;
	dwT1 = dwDot;
	HOVER_FIXMULP(dwT1, dwSinB);
	pdwAlong[0] = dwT1;
	dwT2 = dwDot;
	HOVER_FIXMULP(dwT2, dwSinA);
	pdwAlong[1] = dwT2;
	pdwSide[0] = dwVX;
	pdwSide[1] = dwVY;
	pdwSide[0] = dwVX - pdwAlong[0];
	pdwSide[1] = dwVY - pdwAlong[1];
}

// Stores at *pdwOut the component of the vector (dwX, dwY) tangential to
// dwAngle: dwX*sin(dwAngle) + dwY*sin(dwAngle - 0x80), all 16.16
// fixed-point.

// hover.exe:0x41B7A0
// FUNCTION: HOVER 0x41b7a0
void Hover_sub_41B7A0(DWORD* pdwOut, DWORD dwX, DWORD dwY, DWORD dwAngle)
{
	volatile DWORD dwSin;
	volatile DWORD dwSinM;
	DWORD dwA;
	DWORD dwB;

	// Both products are volatile reads: HOVER_FIXMULP writes its
	// destination only through the pointer temporary, so plain copies have
	// their stores deleted and `*pdwOut` gets dwX + dwY unscaled (rule 95).
	dwSin = g_adwSinTable[(((long)dwAngle & 0x1FF) << 4) + 1];
	dwA = *(volatile DWORD*)&dwY;
	dwSinM = g_adwSinTable[((((long)dwAngle - 0x80) & 0x1FF) << 4) + 1];
	HOVER_FIXMULP(dwA, dwSinM);
	dwB = *(volatile DWORD*)&dwX;
	HOVER_FIXMULP(dwB, dwSin);
	*pdwOut = dwA + dwB;
}

// Arc test on two angles (0x200 per full circle): returns whether lB
// lies within the half circle ahead of lA.

// hover.exe:0x41B830
// FUNCTION: HOVER 0x41b830
int Hover_sub_41B830(long lA, long lB)
{
	lA &= 0x1FF;
	if (lB > lA && lB - 0x100 < lA)
		return 1;
	if (lB + 0x200 <= lA)
		return 0;
	if (lB + 0x100 >= lA)
		return 0;
	return 1;
}

// Loads the default key bindings from the string table, applies the
// 0x30-byte "PlayerControls" registry override, and detects the joystick
// (clearing the joystick button binding when none is present).

// hover.exe:0x4232E0
// FUNCTION: HOVER 0x4232e0
void Hover_sub_4232E0()
{
	JOYINFO joyInfo;

	theApp.m_avkControls[2] = Hover_sub_42DC50(IDS_DEFKEY_A);
	theApp.m_avkControls[1] = Hover_sub_42DC50(IDS_DEFKEY_S);
	theApp.m_avkControls[0] = Hover_sub_42DC50(IDS_DEFKEY_D);
	theApp.m_avkControls[7] = Hover_sub_42DC50(IDS_DEFKEY_HOME);
	theApp.m_avkControls[3] = Hover_sub_42DC50(IDS_DEFKEY_UP);
	theApp.m_avkControls[8] = Hover_sub_42DC50(IDS_DEFKEY_PGUP);
	theApp.m_avkControls[5] = Hover_sub_42DC50(IDS_DEFKEY_LEFT);
	theApp.m_avkControls[6] = Hover_sub_42DC50(IDS_DEFKEY_RIGHT);
	theApp.m_avkControls[9] = Hover_sub_42DC50(IDS_DEFKEY_END);
	theApp.m_avkControls[4] = Hover_sub_42DC50(IDS_DEFKEY_DOWN);
	theApp.m_avkControls[10] = Hover_sub_42DC50(IDS_DEFKEY_PGDN);
	theApp.m_avkControls[11] = 0;

	HKEY hKey;
	if (RegOpenKeyExA(HKEY_CURRENT_USER, "Software\\Microsoft\\Hover!",
			0, KEY_ALL_ACCESS, &hKey) == ERROR_SUCCESS)
	{
		DWORD adwControls[12];
		DWORD dwType = REG_BINARY;
		DWORD cbData = sizeof(adwControls);
		if (RegQueryValueExA(hKey, "PlayerControls", NULL, &dwType,
				(LPBYTE)adwControls, &cbData) == ERROR_SUCCESS &&
			cbData == sizeof(adwControls))
		{
			theApp.m_avkControls[1] = adwControls[0];
			theApp.m_avkControls[0] = adwControls[1];
			theApp.m_avkControls[2] = adwControls[2];
			theApp.m_avkControls[7] = adwControls[3];
			theApp.m_avkControls[3] = adwControls[4];
			theApp.m_avkControls[8] = adwControls[5];
			theApp.m_avkControls[5] = adwControls[6];
			theApp.m_avkControls[6] = adwControls[7];
			theApp.m_avkControls[9] = adwControls[8];
			theApp.m_avkControls[4] = adwControls[9];
			theApp.m_avkControls[10] = adwControls[10];
			theApp.m_avkControls[11] = adwControls[11];
		}
		RegCloseKey(hKey);
	}

	if (joyGetDevCapsA(0, &theApp.m_joyCaps, sizeof(theApp.m_joyCaps))
			== JOYERR_NOERROR)
	{
		UINT uJoyErr = joyGetPos(0, &joyInfo);
		theApp.m_bJoystick = 1;
		if (uJoyErr != JOYERR_NOERROR)
			theApp.m_bJoystick = 0;
	}
	if (theApp.m_bJoystick == 0)
		theApp.m_avkControls[11] = 0;
}

// Zone descriptor access for Hover_sub_41F540: the six 0x338-byte blocks
// are one flat int array; ZD is a dword at byte offset off within zone i,
// ZRC the same address as an LPRECT.
#define ZONE_INTS (0x338 / 4)
#define ZD(i, off) g_adwUnk_4609A0[(i) * ZONE_INTS + (off) / 4]
#define ZRC(i, off) ((LPRECT)&ZD(i, off))

// Initializes the geometry of the six board zone descriptor blocks: zone
// sizes and split-line centers, sub-area and waypoint rectangles, and the
// derived rectangles computed from them. Every paired coordinate write is
// one CPoint-valued assignment; VC 2.x cannot forward a constant through an
// aggregate copy, so each one materialises the pair in an 8-byte class
// temporary on the stack and reads it back for the two global stores.
// hover.exe:0x41F540
// FUNCTION: HOVER 0x41f540
void Hover_sub_41F540()
{
	int* p;
	RECT* pRc;
	int i;
	// hover.exe's frame is three dwords: [esp+0x14]/[esp+0x18] are the
	// compiler's own temporary for the CPoint values above and [esp+0x10]
	// is the register allocator's spill slot for the waypoint loop's i+1.
	// Only the temporary is a real C++ object; the spill dword is stood in
	// for here by a volatile scalar, which is what lands at the bottom of
	// the frame and keeps `sub esp,0xc` with the original's offsets.
	volatile int j;

	// Zone 5
	ZD(5, 0x00) = 0x2bc;
	ZD(5, 0x04) = 0x20d;
	ZD(5, 0x10) = 0x2710;
	ZD(5, 0x14) = 0x2710;
	ZD(5, 0x08) = 0x2bc;
	ZD(5, 0x0c) = 0x145;
	ZD(5, 0x30) = 0x39;
	ZD(5, 0x38) = 0x1a2;
	SetRect(ZRC(5, 0x1f0), 0, 0, 0x2bc, 0x73);
	SetRect(ZRC(5, 0x238), 0, 0x74, 0x2bc, 0x118);
	SetRect(ZRC(5, 0x258), 0x113, 0x18e, 0x1a6, 0x1fa);
	SetRect(ZRC(5, 0x210), 0xd8, 9, 0x1d4, 0x52);
	SetRect(ZRC(5, 0x278), 0x1e6, 0x1aa, 0x239, 0x1f6);
	SetRect(ZRC(5, 0x268), 0x1e6, 0xb6, 0x239, 0x102);
	SetRect(ZRC(5, 0x288), 0x1d5, 0x1a4, 0x1de, 0x1f8);
	SetRect(ZRC(5, 0x298), 0x259, 0x1c3, 0x299, 0x1cf);
	SetRect(ZRC(5, 0x2a8), 0x259, 0x1e3, 0x299, 0x1ef);
	SetRect(ZRC(5, 0x304), 0x5b, 0x1b8, 0x64, 0x1e0);
	SetRect(ZRC(5, 0x314), 0x9a, 0x1b8, 0xa3, 0x1e0);
	SetRect(ZRC(5, 0x324), 0xdb, 0x1b8, 0xe4, 0x1e0);
	*(CPoint*)&ZD(5, 0x2b8) = CPoint(0x56, 0x1e4);
	*(CPoint*)&ZD(5, 0x2c0) = CPoint(0x95, 0x1e4);
	*(CPoint*)&ZD(5, 0x2c8) = CPoint(0xd6, 0x1e4);
	ZD(5, 0x40) = 0x15;
	ZD(5, 0x44) = 0x13;
	ZD(5, 0x48) = 0x13;
	ZD(5, 0x4c) = 0xa;
	ZD(5, 0x50) = 0xa;
	ZD(5, 0x54) = 0xa;
	*(CPoint*)&ZD(5, 0x18) = CPoint(0x18, 0x29);
	*(CPoint*)&ZD(5, 0x20) = CPoint(0xf, 0x14);
	*(CPoint*)&ZD(5, 0x28) = CPoint(0x16, 0x16);
	*(CPoint*)&ZD(5, 0x58) = CPoint(6, 0x11d);
	*(CPoint*)&ZD(5, 0x60) = CPoint(0x1ca, 0x11c);
	*(CPoint*)&ZD(5, 0x68) = CPoint(0xad, 0x11d);
	*(CPoint*)&ZD(5, 0x228) = CPoint(0x39, 8);
	*(CPoint*)&ZD(5, 0x230) = CPoint(0x39, 0x1e);
	*(CPoint*)&ZD(5, 0x220) = CPoint(0x1f2, 8);
	ZD(5, 0x2d0) = 0xf;
	SetRect(ZRC(5, 0x2d4), 0x36, 0x1e5, 0x4e, 0x1f8);
	SetRect(ZRC(5, 0x2e4), 0x75, 0x1e5, 0x8d, 0x1f8);
	SetRect(ZRC(5, 0x2f4), 0xb6, 0x1e5, 0xce, 0x1f8);
	ZD(5, 0x334) = 0x20;

	// Zone 4
	ZD(4, 0x00) = 0x280;
	ZD(4, 0x04) = 0x1e0;
	ZD(4, 0x10) = (ZD(5, 0x00) + 0x280) / 2;
	ZD(4, 0x14) = (ZD(5, 0x04) + 0x1e0) / 2;
	ZD(4, 0x08) = 0x280;
	ZD(4, 0x0c) = 0x12a;
	ZD(4, 0x30) = 0x32;
	ZD(4, 0x38) = 0x17e;
	SetRect(ZRC(4, 0x1f0), 0, 0, 0x280, 0x68);
	SetRect(ZRC(4, 0x238), 0, 0x6a, 0x280, 0x100);
	SetRect(ZRC(4, 0x258), 0xff, 0x16d, 0x181, 0x1cb);
	SetRect(ZRC(4, 0x210), 0xc4, 9, 0x1ac, 0x4b);
	SetRect(ZRC(4, 0x268), 0x1ba, 0xa8, 0x206, 0xef);
	SetRect(ZRC(4, 0x278), 0x1ba, 0x186, 0x206, ZD(4, 0x274) - ZD(4, 0x26c) + 0x186);
	SetRect(ZRC(4, 0x288), 0x1ad, 0x180, 0x1b5, 0x1cb);
	SetRect(ZRC(4, 0x298), 0x224, 0x19c, 0x25a, 0x1a5);
	SetRect(ZRC(4, 0x2a8), 0x224, 0x1b9, 0x25a, 0x1c2);
	SetRect(ZRC(4, 0x304), 0x54, 0x191, 0x5a, 0x1b6);
	SetRect(ZRC(4, 0x314), 0x8e, 0x191, 0x94, 0x1b6);
	SetRect(ZRC(4, 0x324), 0xc9, 0x191, 0xcf, 0x1b6);
	*(CPoint*)&ZD(4, 0x2b8) = CPoint(0x4e, 0x1ba);
	*(CPoint*)&ZD(4, 0x2c0) = CPoint(0x88, 0x1ba);
	*(CPoint*)&ZD(4, 0x2c8) = CPoint(0xc3, 0x1ba);
	ZD(4, 0x40) = 0x19;
	ZD(4, 0x44) = 0x12;
	ZD(4, 0x48) = 0x12;
	ZD(4, 0x4c) = 9;
	ZD(4, 0x50) = 9;
	ZD(4, 0x54) = 9;
	*(CPoint*)&ZD(4, 0x18) = CPoint(0x16, 0x26);
	*(CPoint*)&ZD(4, 0x20) = CPoint(0xc, 0x12);
	*(CPoint*)&ZD(4, 0x28) = CPoint(0x14, 0x14);
	*(CPoint*)&ZD(4, 0x58) = CPoint(2, 0x104);
	*(CPoint*)&ZD(4, 0x60) = CPoint(0x19f, 0x102);
	*(CPoint*)&ZD(4, 0x68) = CPoint(0x9d, 0x10a);
	*(CPoint*)&ZD(4, 0x228) = CPoint(0x35, 8);
	*(CPoint*)&ZD(4, 0x230) = CPoint(0x35, 0x1b);
	*(CPoint*)&ZD(4, 0x220) = CPoint(0x1c7, 8);
	ZD(4, 0x2d0) = 0xf;
	SetRect(ZRC(4, 0x2d4), 0x32, 0x1bb, 0x47, 0x1cb);
	SetRect(ZRC(4, 0x2e4), 0x6c, 0x1bb, 0x81, 0x1cb);
	SetRect(ZRC(4, 0x2f4), 0xa7, 0x1bb, 0xbc, 0x1cb);
	ZD(4, 0x334) = 0x1e;

	// Zone 3
	ZD(3, 0x00) = 0x22c;
	ZD(3, 0x04) = 0x1a1;
	ZD(3, 0x10) = (ZD(4, 0x00) + 0x22c) / 2;
	ZD(3, 0x14) = (ZD(4, 0x04) + 0x1a1) / 2;
	ZD(3, 0x08) = 0x22c;
	ZD(3, 0x0c) = 0x105;
	ZD(3, 0x30) = 0x2a;
	ZD(3, 0x38) = 0x14a;
	SetRect(ZRC(3, 0x1f0), 0, 0, 0x22c, 0x5d);
	SetRect(ZRC(3, 0x238), 0, 0x5e, 0x22c, 0xe0);
	SetRect(ZRC(3, 0x258), 0xd8, 0x13e, 0x14d, 0x18e);
	SetRect(ZRC(3, 0x210), 0xac, 7, 0x171, 0x40);
	SetRect(ZRC(3, 0x268), 0x182, 0x93, 0x1c2, 0xce);
	SetRect(ZRC(3, 0x278), 0x182, 0x152, 0x1c2, ZD(3, 0x274) - ZD(3, 0x26c) + 0x152);
	SetRect(ZRC(3, 0x288), 0x175, 0x14c, 0x17c, 0x18f);
	SetRect(ZRC(3, 0x298), 0x1db, 0x166, 0x20b, 0x170);
	SetRect(ZRC(3, 0x2a8), 0x1db, 0x180, 0x20b, 0x18a);
	SetRect(ZRC(3, 0x304), 0x49, 0x15c, 0x4e, 0x17e);
	SetRect(ZRC(3, 0x314), 0x7b, 0x15c, 0x80, 0x17e);
	SetRect(ZRC(3, 0x324), 0xae, 0x15c, 0xb3, 0x17e);
	*(CPoint*)&ZD(3, 0x2b8) = CPoint(0x44, 0x181);
	*(CPoint*)&ZD(3, 0x2c0) = CPoint(0x76, 0x181);
	*(CPoint*)&ZD(3, 0x2c8) = CPoint(0xa9, 0x181);
	ZD(3, 0x40) = 0x16;
	ZD(3, 0x44) = 0x10;
	ZD(3, 0x48) = 0x10;
	ZD(3, 0x4c) = 0xb;
	ZD(3, 0x50) = 0xb;
	ZD(3, 0x54) = 0xb;
	*(CPoint*)&ZD(3, 0x18) = CPoint(0x13, 0x21);
	*(CPoint*)&ZD(3, 0x20) = CPoint(0xb, 0xf);
	*(CPoint*)&ZD(3, 0x28) = CPoint(0x11, 0x10);
	*(CPoint*)&ZD(3, 0x58) = CPoint(4, 0xe8);
	*(CPoint*)&ZD(3, 0x60) = CPoint(0x13f, 0xe3);
	*(CPoint*)&ZD(3, 0x68) = CPoint(0x82, 0xe7);
	*(CPoint*)&ZD(3, 0x228) = CPoint(0x2d, 7);
	*(CPoint*)&ZD(3, 0x230) = CPoint(0x2d, 0x18);
	*(CPoint*)&ZD(3, 0x220) = CPoint(0x18b, 6);
	ZD(3, 0x2d0) = 0xe;
	SetRect(ZRC(3, 0x2d4), 0x2d, 0x181, 0x3f, 0x18f);
	SetRect(ZRC(3, 0x2e4), 0x5f, 0x181, 0x71, 0x18f);
	SetRect(ZRC(3, 0x2f4), 0x92, 0x181, 0xa4, 0x18f);
	ZD(3, 0x334) = 0x1c;

	// Zone 2
	ZD(2, 0x00) = 0x200;
	ZD(2, 0x04) = 0x180;
	ZD(2, 0x10) = (ZD(3, 0x00) + 0x200) / 2;
	ZD(2, 0x14) = (ZD(3, 0x04) + 0x180) / 2;
	ZD(2, 0x08) = 0x200;
	ZD(2, 0x0c) = 0xf2;
	ZD(2, 0x30) = 0x2a;
	ZD(2, 0x38) = 0x134;
	SetRect(ZRC(2, 0x1f0), 0, 0, 0x200, 0x56);
	SetRect(ZRC(2, 0x238), 0, 0x54, 0x200, 0xcd);
	SetRect(ZRC(2, 0x258), 0xc9, 0x126, 0x135, 0x171);
	SetRect(ZRC(2, 0x210), 0x9c, 6, 0x156, 0x3c);
	SetRect(ZRC(2, 0x268), 0x162, 0x85, 0x1a1, 0xbd);
	SetRect(ZRC(2, 0x278), 0x162, 0x135, 0x1a1, ZD(2, 0x274) - ZD(2, 0x26c) + 0x135);
	SetRect(ZRC(2, 0x288), 0x157, 0x130, 0x15e, 0x16f);
	SetRect(ZRC(2, 0x298), 0x1b7, 0x149, 0x1e5, 0x151);
	SetRect(ZRC(2, 0x2a8), 0x1b7, 0x161, 0x1e5, 0x169);
	SetRect(ZRC(2, 0x304), 0x43, 0x13f, 0x48, 0x15d);
	SetRect(ZRC(2, 0x314), 0x71, 0x13f, 0x76, 0x15d);
	SetRect(ZRC(2, 0x324), 0xa0, 0x13f, 0xa5, 0x15d);
	*(CPoint*)&ZD(2, 0x2b8) = CPoint(0x3e, 0x161);
	*(CPoint*)&ZD(2, 0x2c0) = CPoint(0x6c, 0x161);
	*(CPoint*)&ZD(2, 0x2c8) = CPoint(0x9b, 0x161);
	ZD(2, 0x40) = 0x14;
	ZD(2, 0x44) = 0xe;
	ZD(2, 0x48) = 0xe;
	ZD(2, 0x4c) = 9;
	ZD(2, 0x50) = 9;
	ZD(2, 0x54) = 9;
	*(CPoint*)&ZD(2, 0x18) = CPoint(0x12, 0x1e);
	*(CPoint*)&ZD(2, 0x20) = CPoint(0xb, 0xf);
	*(CPoint*)&ZD(2, 0x28) = CPoint(0x10, 0x10);
	*(CPoint*)&ZD(2, 0x58) = CPoint(6, 0xd2);
	*(CPoint*)&ZD(2, 0x60) = CPoint(0x12e, 0xd1);
	*(CPoint*)&ZD(2, 0x68) = CPoint(0x7e, 0xd3);
	*(CPoint*)&ZD(2, 0x228) = CPoint(0x2a, 6);
	*(CPoint*)&ZD(2, 0x230) = CPoint(0x2a, 0x16);
	*(CPoint*)&ZD(2, 0x220) = CPoint(0x16b, 6);
	ZD(2, 0x2d0) = 0xf;
	SetRect(ZRC(2, 0x2d4), 0x2a, 0x162, 0x39, 0x16e);
	SetRect(ZRC(2, 0x2e4), 0x58, 0x162, 0x67, 0x16e);
	SetRect(ZRC(2, 0x2f4), 0x87, 0x162, 0x96, 0x16e);
	ZD(2, 0x334) = 0x19;

	// Zone 1
	ZD(1, 0x00) = 0x1a0;
	ZD(1, 0x04) = 0x138;
	ZD(1, 0x10) = (ZD(2, 0x00) + 0x1a0) / 2;
	ZD(1, 0x14) = (ZD(2, 0x04) + 0x138) / 2;
	ZD(1, 0x08) = 0x1a0;
	ZD(1, 0x0c) = 0xc4;
	ZD(1, 0x30) = 0x21;
	ZD(1, 0x38) = 0xf6;
	SetRect(ZRC(1, 0x1f0), 0, 0, 0x1a0, 0x44);
	SetRect(ZRC(1, 0x238), 0, 0x44, 0x1a0, 0xa6);
	SetRect(ZRC(1, 0x258), 0xa3, 0xec, 0xfc, 0x128);
	SetRect(ZRC(1, 0x210), 0x84, 5, 0x114, 0x30);
	SetRect(ZRC(1, 0x268), 0x122, 0x6c, 0x151, 0x98);
	SetRect(ZRC(1, 0x278), 0x122, 0xfd, 0x151, ZD(1, 0x274) - ZD(1, 0x26c) + 0xfd);
	SetRect(ZRC(1, 0x288), 0x117, 0xfa, 0x11c, 0x12a);
	SetRect(ZRC(1, 0x298), 0x167, 0x10c, 0x188, 0x111);
	SetRect(ZRC(1, 0x2a8), 0x167, 0x11f, 0x188, 0x124);
	SetRect(ZRC(1, 0x304), 0x37, 0x103, 0x3b, 0x11b);
	SetRect(ZRC(1, 0x314), 0x5d, 0x103, 0x61, 0x11b);
	SetRect(ZRC(1, 0x324), 0x83, 0x103, 0x87, 0x11b);
	*(CPoint*)&ZD(1, 0x2b8) = CPoint(0x33, 0x11f);
	*(CPoint*)&ZD(1, 0x2c0) = CPoint(0x59, 0x11f);
	*(CPoint*)&ZD(1, 0x2c8) = CPoint(0x7f, 0x11f);
	ZD(1, 0x40) = 0x10;
	ZD(1, 0x44) = 0xb;
	ZD(1, 0x48) = 0xb;
	ZD(1, 0x4c) = 7;
	ZD(1, 0x50) = 7;
	ZD(1, 0x54) = 7;
	*(CPoint*)&ZD(1, 0x18) = CPoint(0xe, 0x19);
	*(CPoint*)&ZD(1, 0x20) = CPoint(8, 0xc);
	*(CPoint*)&ZD(1, 0x28) = CPoint(0xd, 0xd);
	*(CPoint*)&ZD(1, 0x58) = CPoint(3, 0xa9);
	*(CPoint*)&ZD(1, 0x60) = CPoint(0x111, 0xa9);
	*(CPoint*)&ZD(1, 0x68) = CPoint(0x5e, 0xac);
	*(CPoint*)&ZD(1, 0x228) = CPoint(0x22, 5);
	*(CPoint*)&ZD(1, 0x230) = CPoint(0x22, 0x12);
	*(CPoint*)&ZD(1, 0x220) = CPoint(0x128, 5);
	ZD(1, 0x2d0) = 0xd;
	SetRect(ZRC(1, 0x2d4), 0x21, 0x11f, 0x2e, 0x129);
	SetRect(ZRC(1, 0x2e4), 0x48, 0x11f, 0x54, 0x129);
	SetRect(ZRC(1, 0x2f4), 0x6e, 0x11f, 0x7a, 0x129);
	ZD(1, 0x334) = 0x16;

	// Zone 0
	ZD(0, 0x00) = 0x140;
	ZD(0, 0x04) = 0xf0;
	ZD(0, 0x10) = (ZD(1, 0x00) + 0x140) / 2;
	ZD(0, 0x14) = (ZD(1, 0x04) + 0xf0) / 2;
	ZD(0, 0x08) = 0x140;
	ZD(0, 0x0c) = 0x95;
	ZD(0, 0x30) = 0x19;
	ZD(0, 0x38) = 0xbd;
	SetRect(ZRC(0, 0x1f0), 0, 0, 0x140, 0x35);
	SetRect(ZRC(0, 0x238), 0, 0x33, 0x140, 0x7f);
	SetRect(ZRC(0, 0x258), 0x7f, 0xb6, 0xc0, 0xe4);
	SetRect(ZRC(0, 0x210), 0x64, 4, 0xd4, 0x25);
	SetRect(ZRC(0, 0x268), 0xde, 0x52, 0x105, 0x74);
	SetRect(ZRC(0, 0x278), 0xde, 0xc2, 0x105, ZD(0, 0x274) - ZD(0, 0x26c) + 0xc2);
	SetRect(ZRC(0, 0x288), 0xd6, 0xba, 0xdb, 0xe6);
	SetRect(ZRC(0, 0x298), 0x111, 0xce, 0x12c, 0xd2);
	SetRect(ZRC(0, 0x2a8), 0x111, 0xdd, 0x12c, 0xe1);
	SetRect(ZRC(0, 0x304), 0x27, 0xc9, 0x2c, 0xe2);
	SetRect(ZRC(0, 0x314), 0x44, 0xc9, 0x49, 0xe2);
	SetRect(ZRC(0, 0x324), 0x61, 0xc9, 0x66, 0xe2);
	*(CPoint*)&ZD(0, 0x2b8) = CPoint(0x1a, 0xcc);
	*(CPoint*)&ZD(0, 0x2c0) = CPoint(0x37, 0xcc);
	*(CPoint*)&ZD(0, 0x2c8) = CPoint(0x54, 0xcc);
	ZD(0, 0x40) = 0xd;
	ZD(0, 0x44) = 9;
	ZD(0, 0x48) = 9;
	ZD(0, 0x4c) = 6;
	ZD(0, 0x50) = 6;
	ZD(0, 0x54) = 6;
	*(CPoint*)&ZD(0, 0x18) = CPoint(0xb, 0x13);
	*(CPoint*)&ZD(0, 0x20) = CPoint(8, 0xc);
	*(CPoint*)&ZD(0, 0x28) = CPoint(0xa, 0xb);
	*(CPoint*)&ZD(0, 0x58) = CPoint(1, 0x81);
	*(CPoint*)&ZD(0, 0x60) = CPoint(0xbc, 0x81);
	*(CPoint*)&ZD(0, 0x68) = CPoint(0x5a, 0x81);
	*(CPoint*)&ZD(0, 0x228) = CPoint(0x1a, 3);
	*(CPoint*)&ZD(0, 0x230) = CPoint(0x1a, 0xe);
	*(CPoint*)&ZD(0, 0x220) = CPoint(0xe4, 4);
	ZD(0, 0x2d0) = 0xd;
	SetRect(ZRC(0, 0x2d4), 0x1b, 0xda, 0x24, 0xe5);
	SetRect(ZRC(0, 0x2e4), 0x38, 0xda, 0x41, 0xe5);
	SetRect(ZRC(0, 0x2f4), 0x55, 0xda, 0x5e, 0xe5);
	ZD(0, 0x334) = 0x14;

	// Derived per-zone rectangles: copy the two anchor rectangles, clamp
	// their vertical extents, then cascade four rectangles across the
	// zone's right edge. The zone pointer advances mid-body, so the last
	// two SetRect calls use negative offsets from the bumped pointer.
	p = g_adwUnk_4609A0 + 0x28 / 4;
	do
	{
		*(LPRECT)(p + 0x220 / 4) = *(LPRECT)(p + 0x210 / 4);
		p[0x224 / 4] = p[0x214 / 4] - p[0x21c / 4] + p[-0x24 / 4] - 1;
		p[0x22c / 4] = p[-0x24 / 4] - 1;
		*(LPRECT)(p + 0x1d8 / 4) = *(LPRECT)(p + 0x1c8 / 4);
		p[0x1e4 / 4] = p[0x1d4 / 4] - p[0x1cc / 4] - 1;
		p[0x1dc / 4] = 0;
		p[0x0c / 4] = p[0x1e4 / 4] + 1;
		p[0x14 / 4] = p[0x224 / 4] - 1;
		SetRect((LPRECT)(p + 0x188 / 4), p[0x40 / 4], p[0x44 / 4],
			p[0] + p[0x40 / 4], p[0] + p[0x44 / 4]);
		SetRect((LPRECT)(p + 0x198 / 4), p[0] + p[0x188 / 4], p[0x18c / 4],
			p[0] + p[0x190 / 4], p[0x194 / 4]);
		p += 0x338 / 4;
		SetRect((LPRECT)(p - 0x190 / 4), p[-0x338 / 4] + p[-0x1a0 / 4],
			p[-0x1ac / 4], p[-0x338 / 4] + p[-0x198 / 4], p[-0x1a4 / 4]);
		SetRect((LPRECT)(p - 0x180 / 4), p[-0x338 / 4] + p[-0x190 / 4],
			p[-0x1ac / 4], p[-0x338 / 4] + p[-0x188 / 4], p[-0x1a4 / 4]);
	}
	while (p < g_adwUnk_4609A0 + 0x28 / 4 + 6 * (0x338 / 4));

	// Waypoint grids: two rows of ten rectangles per zone
	p = g_adwUnk_4609A0 + 0x70 / 4;
	do
	{
		pRc = (RECT*)p;
		for (i = 0; i < 10; i++)
		{
			j = i + 1;
			SetRect(pRc, p[-0x18 / 4] + i * p[-0x50 / 4], p[-0x14 / 4],
				p[-0x18 / 4] + j * p[-0x50 / 4],
				p[-0x4c / 4] + p[-0x14 / 4]);
			// (Not a bug, despite appearances: hover.exe:0x420d33
			// reads j as [esp+0x14], but the SetRect bottom argument
			// has already been pushed at 0x420d2a, so esp is four
			// lower there and the slot really is [esp+0x10].)
			SetRect(pRc + 10, p[-0x10 / 4] + i * p[-0x58 / 4], p[-0xc / 4],
				p[-0x10 / 4] + j * p[-0x58 / 4],
				p[-0x54 / 4] + p[-0xc / 4]);
			pRc++;
		}
		p += 0x338 / 4;
	}
	while (p < g_adwUnk_4609A0 + 0x70 / 4 + 6 * (0x338 / 4));

	// Flag drop zones: three rectangles per zone
	p = g_adwUnk_4609A0 + 0x1b0 / 4;
	do
	{
		pRc = (RECT*)p;
		for (i = 0; i < 3; i++)
		{
			j = i + 1;
			SetRect(pRc, p[-0x148 / 4] + i * p[-0x188 / 4], p[-0x144 / 4],
				p[-0x148 / 4] + j * p[-0x188 / 4],
				p[-0x184 / 4] + p[-0x144 / 4]);
			pRc++;
		}
		p += 0x338 / 4;
	}
	while (p < g_adwUnk_4609A0 + 0x1b0 / 4 + 6 * (0x338 / 4));
}

#undef ZONE_INTS
#undef ZD
#undef ZRC

/////////////////////////////////////////////////////////////////////////////
// Key binding name resolution


// Hover_sub_42DC50 and its key name table (hover.exe:0x42DC50 /
// 0x4BDAC8) live in optionsdlg.cpp next to the Player Controls dialog
// that shares them; in the original they are in the same translation
// unit (0x42DC50 sits between the dialog's Sub_42DB60 and Sub_42DD70).


/////////////////////////////////////////////////////////////////////////////
// Per-skill-level parameter blocks

// Builds a player parameter block from eleven tuning values, storing each
// one as 16.16 fixed point. Only the static initializer at
// hover.exe:0x433370 calls it (five times, for the two other human blocks
// and the three robot blocks); the beginner human block is built by the
// inline 16.16 constructor instead.

// hover.exe:0x425290
// FUNCTION: HOVER 0x425290
CPlayerParams::CPlayerParams(double dbl0, double dbl1, double dbl2,
	double dbl3, double dbl4, double dbl5, double dbl6, double dbl7,
	double dbl8, double dbl9, double dbl10)
{
	m_adw[0] = FIX16(dbl0);
	m_adw[1] = FIX16(dbl1);
	m_adw[2] = FIX16(dbl2);
	m_adw[3] = FIX16(dbl3);
	m_adw[4] = FIX16(dbl4);
	m_adw[5] = FIX16(dbl5);
	m_adw[6] = FIX16(dbl6);
	m_adw[7] = FIX16(dbl7);
	m_adw[8] = FIX16(dbl8);
	m_adw[9] = FIX16(dbl9);
	m_adw[10] = FIX16(dbl10);
}

// Called by CHoverApp::InitInstance, each of these loads a set of three
// REG_BINARY parameter blocks from the registry key
// HKEY_CURRENT_USER\Software\Microsoft\Hover!
// (H = human player, R = robot opponents, RobotAI = robot AI;
// Beg/Int/Exp = beginner/intermediate/expert). The query results are
// deliberately ignored -- if a value is missing the block stays zeroed.

// hover.exe:0x4249C0
// FUNCTION: HOVER 0x4249c0
void Hover_sub_4249C0()
{
	HKEY hKey;
	if (RegOpenKeyExA(HKEY_CURRENT_USER, "Software\\Microsoft\\Hover!",
			0, KEY_ALL_ACCESS, &hKey) != ERROR_SUCCESS)
		return;

	DWORD dwType = REG_BINARY;
	DWORD cbData = sizeof(g_HBegParams);
	RegQueryValueExA(hKey, "HBegParams", NULL, &dwType,
		(LPBYTE)&g_HBegParams, &cbData);
	RegQueryValueExA(hKey, "HIntParams", NULL, &dwType,
		(LPBYTE)&g_HIntParams, &cbData);
	RegQueryValueExA(hKey, "HExpParams", NULL, &dwType,
		(LPBYTE)&g_HExpParams, &cbData);

	RegCloseKey(hKey);
}

// hover.exe:0x424A70
// FUNCTION: HOVER 0x424a70
void Hover_sub_424A70()
{
	HKEY hKey;
	if (RegOpenKeyExA(HKEY_CURRENT_USER, "Software\\Microsoft\\Hover!",
			0, KEY_ALL_ACCESS, &hKey) != ERROR_SUCCESS)
		return;

	DWORD dwType = REG_BINARY;
	DWORD cbData = sizeof(g_RBegParams);
	RegQueryValueExA(hKey, "RBegParams", NULL, &dwType,
		(LPBYTE)&g_RBegParams, &cbData);
	RegQueryValueExA(hKey, "RIntParams", NULL, &dwType,
		(LPBYTE)&g_RIntParams, &cbData);
	RegQueryValueExA(hKey, "RExpParams", NULL, &dwType,
		(LPBYTE)&g_RExpParams, &cbData);

	RegCloseKey(hKey);
}

// hover.exe:0x424B20
// FUNCTION: HOVER 0x424b20
void Hover_sub_424B20()
{
	HKEY hKey;
	if (RegOpenKeyExA(HKEY_CURRENT_USER, "Software\\Microsoft\\Hover!",
			0, KEY_ALL_ACCESS, &hKey) != ERROR_SUCCESS)
		return;

	DWORD dwType = REG_BINARY;
	DWORD cbData = sizeof(g_RobotAIBegParams);
	RegQueryValueExA(hKey, "RobotAIBegParams", NULL, &dwType,
		(LPBYTE)&g_RobotAIBegParams, &cbData);
	RegQueryValueExA(hKey, "RobotAIIntParams", NULL, &dwType,
		(LPBYTE)&g_RobotAIIntParams, &cbData);
	RegQueryValueExA(hKey, "RobotAIExpParams", NULL, &dwType,
		(LPBYTE)&g_RobotAIExpParams, &cbData);

	RegCloseKey(hKey);
}

// Selects the active parameter blocks for the current skill level
// (m_dwUnk01C8: 0 = beginner, 1 = intermediate, 2 = expert).

// hover.exe:0x425870
// FUNCTION: HOVER 0x425870
void CHoverApp::Sub_425870()
{
	switch (m_dwUnk01C8)
	{
	case 0:
		m_pUnk014C = &g_HBegParams;
		m_pUnk0150 = &g_RBegParams;
		m_pUnk0154 = &g_RobotAIBegParams;
		break;
	case 1:
		m_pUnk014C = &g_HIntParams;
		m_pUnk0150 = &g_RIntParams;
		m_pUnk0154 = &g_RobotAIIntParams;
		break;
	case 2:
		m_pUnk014C = &g_HExpParams;
		m_pUnk0150 = &g_RExpParams;
		m_pUnk0154 = &g_RobotAIExpParams;
		break;
	}
}

/////////////////////////////////////////////////////////////////////////////
// Worker thread and engine object shutdown (called by
// CHoverApp::ExitInstance)

// hover.exe:0x422E30
// FUNCTION: HOVER 0x422e30
void Hover_sub_422E30()
{
	if (g_dwUnk_4C56D8 == 0 && g_pThreadUnk_4C56E0 != NULL)
	{
		EnterCriticalSection(&g_csUnk_4A4808);
		g_dwUnk_4C56E8 = 1;
		LeaveCriticalSection(&g_csUnk_4A4808);
		ResetEvent(g_hUnk_4C56F8);
		SetEvent(g_hUnk_4C56F4);
		WaitForSingleObject(g_hUnk_4C56F8, INFINITE);
		g_pThreadUnk_4C56E0 = NULL;
		if (g_hUnk_4C56F4 != NULL && CloseHandle(g_hUnk_4C56F4))
			g_hUnk_4C56F4 = NULL;
		if (g_hUnk_4C56F8 != NULL && CloseHandle(g_hUnk_4C56F8))
			g_hUnk_4C56F8 = NULL;
		if (g_dwUnk_4C56E4 == 1)
		{
			DeleteCriticalSection(&g_csUnk_4A4808);
			g_dwUnk_4C56E4 = 0;
		}
	}

	for (int i = 0; i < g_arrUnk_4A47F0.GetSize(); i++)
		if (g_arrUnk_4A47F0[i] != NULL)
			delete g_arrUnk_4A47F0[i];

	if (g_pUnk_4C56DC != NULL)
	{
		Hover_sub_43A760(g_pUnk_4C56DC, 1, 1);
		Hover_sub_43DAB0(g_pUnk_4C56DC);
		g_pUnk_4C56DC = NULL;
	}
}

/////////////////////////////////////////////////////////////////////////////
// Subsystem shutdown helpers called by CHoverApp::ExitInstance
//
// Each helper deletes every element of a global CPtrList, empties the
// list, and clears an associated global DWORD. The element types are not
// yet known; the original deletes them with scalar operator delete.

// hover.exe:0x42C700
// FUNCTION: HOVER 0x42c700
void Hover_sub_42C700()
{
	POSITION pos = g_listUnk_4A46F8.GetHeadPosition();
	while (pos != NULL)
		delete (BYTE*)g_listUnk_4A46F8.GetNext(pos);
	g_listUnk_4A46F8.RemoveAll();
	g_dwUnk_4C55D0 = 0;
}

// hover.exe:0x42C590
// FUNCTION: HOVER 0x42c590
void Hover_sub_42C590()
{
	POSITION pos = g_listUnk_4A46D0.GetHeadPosition();
	while (pos != NULL)
		delete (BYTE*)g_listUnk_4A46D0.GetNext(pos);
	g_listUnk_4A46D0.RemoveAll();
	g_dwUnk_4C5588 = 0;
}

// hover.exe:0x42C2D0
// FUNCTION: HOVER 0x42c2d0
void Hover_sub_42C2D0()
{
	POSITION pos = g_listUnk_4A46A0.GetHeadPosition();
	while (pos != NULL)
		delete (BYTE*)g_listUnk_4A46A0.GetNext(pos);
	g_listUnk_4A46A0.RemoveAll();
	g_dwUnk_4C54D0 = 0;
}

// hover.exe:0x42AED0
// FUNCTION: HOVER 0x42aed0
void Hover_sub_42AED0()
{
	POSITION pos = g_listUnk_4A45B8.GetHeadPosition();
	while (pos != NULL)
		delete (BYTE*)g_listUnk_4A45B8.GetNext(pos);
	g_listUnk_4A45B8.RemoveAll();
	g_dwUnk_4C5330 = 0;
}

// hover.exe:0x42B710
// FUNCTION: HOVER 0x42b710
void Hover_sub_42B710()
{
	POSITION pos = g_listUnk_4A45E0.GetHeadPosition();
	while (pos != NULL)
		delete (BYTE*)g_listUnk_4A45E0.GetNext(pos);
	g_listUnk_4A45E0.RemoveAll();
	g_dwUnk_4C53E0 = 0;
}

// hover.exe:0x42F050
// FUNCTION: HOVER 0x42f050
void Hover_sub_42F050()
{
	POSITION pos = g_listUnk_4A4858.GetHeadPosition();
	while (pos != NULL)
		delete (BYTE*)g_listUnk_4A4858.GetNext(pos);
	g_listUnk_4A4858.RemoveAll();
	g_dwUnk_4C57A8 = 0;
}

// hover.exe:0x42CB70
// FUNCTION: HOVER 0x42cb70
void Hover_sub_42CB70()
{
	POSITION pos = g_listUnk_4A4748.GetHeadPosition();
	while (pos != NULL)
		delete (BYTE*)g_listUnk_4A4748.GetNext(pos);
	g_listUnk_4A4748.RemoveAll();
	g_dwUnk_4C5620 = 0;
}

// hover.exe:0x41C060
// FUNCTION: HOVER 0x41c060
int CHoverApp::ExitInstance()
{
	DeleteCriticalSection(&m_csUnk00D4);
	DeleteCriticalSection(&m_csUnk00EC);
	DeleteCriticalSection(&m_csUnk0104);
	DeleteCriticalSection(&m_csUnk011C);
	Hover_sub_422E30();
	Hover_sub_42D290();
	Hover_sub_42C700();
	Hover_sub_42C590();
	Hover_sub_42C2D0();
	Hover_sub_42AED0();
	Hover_sub_42B710();
	Hover_sub_42F050();
	Hover_sub_42CB70();
	if (g_pUnk_4C4738 != NULL)
		delete g_pUnk_4C4738;
	delete (BYTE*)m_pUnk03EC;
	delete (BYTE*)g_pUnk_46003C;
	return CWinApp::ExitInstance();
}

// Idle-time driver. The original overrides CWinApp::OnIdle only to forward
// to the base implementation -- the override exists (the vtable slot at
// hover.exe:0x4BA370+0x3C points here, not at CWinApp::OnIdle) but adds
// nothing of its own. The game's frame loop runs on the render worker
// thread, not on idle time.
// hover.exe:0x405B80
// FUNCTION: HOVER 0x405b80
BOOL CHoverApp::OnIdle(LONG lCount)
{
	return CWinApp::OnIdle(lCount);
}

/////////////////////////////////////////////////////////////////////////////
// CHoverApp startup checks

// hover.exe:0x425E40
// FUNCTION: HOVER 0x425e40
BOOL CHoverApp::Sub_425E40()
{
	// Minimum configuration check: the CPU must be better than a 386
	// and there must be at least 6MB of physical memory
	SYSTEM_INFO sysInfo;
	GetSystemInfo(&sysInfo);
	if (sysInfo.dwProcessorType == PROCESSOR_INTEL_386)
		return FALSE;

	MEMORYSTATUS memStatus;
	memStatus.dwLength = sizeof(memStatus);
	GlobalMemoryStatus(&memStatus);
	return memStatus.dwTotalPhys >= 0x600000;
}

// hover.exe:0x425E80
// FUNCTION: HOVER 0x425e80
BOOL CHoverApp::Sub_425E80()
{
	// Only allow one instance of the game to run at a time
	if (CreateEventA(NULL, TRUE, FALSE, "Hover!InitInstance") != NULL &&
		GetLastError() == ERROR_ALREADY_EXISTS)
	{
		AfxMessageBox(IDS_ALREADY_RUNNING, MB_OK, (UINT)-1);
		return FALSE;
	}
	return TRUE;
}

// hover.exe:0x41C7E0
// FUNCTION: HOVER 0x41c7e0
void CHoverApp::Sub_41C7E0()
{
	// Locate the game data directory. Prefer the current directory if
	// it contains mazes\small.maz; otherwise fall back to the
	// directory containing the executable.
	GetCurrentDirectoryA(0x104, m_strUnk0414.GetBufferSetLength(0x104));
	m_strUnk0414.ReleaseBuffer();
	m_strUnk0414 += '\\';

	// The original compares signed here (jge in the disassembly)
	if (m_strUnk0414.GetLength() + 0x10 < 0x104)
	{
		char szPath[0x100];
		OFSTRUCT of;
		lstrcpy(szPath, m_strUnk0414);
		lstrcat(szPath, "mazes\\small.maz");
		if (OpenFile(szPath, &of, OF_EXIST) != (HFILE)-1)
			return;
	}

	GetModuleFileNameA(NULL, m_strUnk0414.GetBufferSetLength(0x104), 0x104);
	m_strUnk0414.ReleaseBuffer();

	char szDrive[_MAX_DRIVE], szDir[_MAX_DIR], szFname[_MAX_FNAME], szExt[_MAX_EXT];
	_splitpath(m_strUnk0414, szDrive, szDir, szFname, szExt);
	m_strUnk0414 = szDrive;
	m_strUnk0414 += szDir;
}

/////////////////////////////////////////////////////////////////////////////
// Full-screen mode switching and the game-start sequence

// The "Please Wait" dialog CHoverApp::Sub_41C8E0 puts up while a level
// loads. The constructor is bare: the CStatic member and the CDialog base
// do all the work.

// hover.exe:0x424720
// FUNCTION: HOVER 0x424720
CSub424720::CSub424720(CWnd* pParent /*=NULL*/)
	: CDialog(IDD_LOADING, pParent)
{
}

// Starts a level: puts up the modeless "Please Wait" dialog, installs the
// texture load progress callback, throws away the old document contents
// and lets CWinApp::OnFileNew build the new ones (which is where the level
// is actually loaded), then retitles the frame "Hover! - Level n" and
// takes the dialog down again. m_dwUnk01B8 marks a game as in progress.
//
// Note the original never checks the CDialog allocation: `new` returning
// NULL would take Create() through a NULL this. hWndDlg only exists to
// pull the m_hWnd load above the three global stores, where the original
// has it (notes/perfect-match.md rule 25).

// hover.exe:0x41C8E0
// FUNCTION: HOVER 0x41c8e0
void CHoverApp::Sub_41C8E0()
{
	CSub424720* pDlg;
	HWND hWndDlg;
	char szFormat[40];
	char szTitle[40];

	m_dwUnk01B8 = 1;
	pDlg = new CSub424720(NULL);
	pDlg->Create(IDD_LOADING, NULL);
	hWndDlg = pDlg->m_hWnd;
	g_pLoadDlg_4C57D4 = pDlg;
	g_dwUnk_4C4DA8 = 0;
	g_pfnUnk_4C4DAC = Hover_sub_40EB50;
	::UpdateWindow(hWndDlg);

	theApp.m_pBumperDoc->DeleteContents();
	theApp.m_pBumperDoc->m_dwUnk8498 = 0;
	CWinApp::OnFileNew();

	::LoadString(AfxGetInstanceHandle(), IDS_LEVEL_TITLE, szFormat,
		sizeof(szFormat));
	wsprintf(szTitle, szFormat, g_dwUnk_46049C + 1);
	AfxGetApp()->m_pMainWnd->SetWindowText(szTitle);

	g_pfnUnk_4C4DAC = NULL;
	pDlg->DestroyWindow();
	delete pDlg;
	g_pLoadDlg_4C57D4 = NULL;
}

// Switches the game into its 640x480 full-screen mode: the frame's window
// state (style, extended style, position and menu) is saved so that
// Sub_425600 can put it back, the frame becomes a plain WS_POPUP without
// the status bar and the view loses its client edge. A borderless topmost
// "Temp" window covers the screen while ChangeDisplaySettings runs so that
// the mode switch is not visible. Returns whether the mode switch worked.

// hover.exe:0x4253D0
// FUNCTION: HOVER 0x4253d0
BOOL CHoverApp::Sub_4253D0()
{
	CHoverApp* pApp = this;
	CBumperView* pView;
	int cx;
	int cy;

	HWND hWnd = pApp->m_pMainWnd->m_hWnd;
	pApp->m_dwUnk03B8 = 1;
	pApp->m_dwUnk03C0 = ::GetWindowLong(hWnd, GWL_STYLE);
	pApp->m_dwUnk03C4 = ::GetWindowLong(pApp->m_pMainWnd->m_hWnd, GWL_EXSTYLE);
	::GetWindowRect(pApp->m_pMainWnd->m_hWnd, &pApp->m_rcUnk03C8);
	pApp->m_hMenuUnk03D8 = pApp->m_pMainWnd->GetMenu()->m_hMenu;
	pApp->m_pMainWnd->ModifyStyleEx(pApp->m_dwUnk03C4, 0, 0);
	pApp->m_pMainWnd->ModifyStyle(pApp->m_dwUnk03C0, WS_POPUP | WS_VISIBLE,
		SWP_FRAMECHANGED);
	((CMainFrame*)pApp->m_pMainWnd)->m_wndStatusBar.ShowWindow(SW_HIDE);

	pView = Hover_sub_401140();
	pView->ModifyStyleEx(WS_EX_CLIENTEDGE, 0, 0);
	pView->ModifyStyle(WS_BORDER, 0, SWP_FRAMECHANGED);

	cx = 640;
	pApp->m_dwUnk03E8 = 0;
	cy = 480;

	DEVMODE dm;
	CWnd wndCover;

	wndCover.CreateEx(WS_EX_TOPMOST, AfxRegisterWndClass(0, 0, 0, 0),
		"Temp", WS_POPUP | WS_VISIBLE | WS_BORDER, 0, 0,
		::GetSystemMetrics(SM_CXSCREEN), ::GetSystemMetrics(SM_CYSCREEN),
		NULL, NULL, NULL);
	::RedrawWindow(wndCover.m_hWnd, NULL, NULL,
		RDW_INVALIDATE | RDW_ERASE | RDW_UPDATENOW);

	dm.dmSize = sizeof(dm);
	dm.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT;
	dm.dmPelsWidth = cx;
	dm.dmPelsHeight = cy;
	pApp->m_dwUnk03B0 =
		(pApp->m_pfnChangeDisplaySettings(&dm, CDS_FULLSCREEN) == 0);
	if (pApp->m_dwUnk03B0 != 0)
	{
		if (pApp->m_dwUnk03BC != 0)
			pApp->m_pMainWnd->SetWindowPos(&CWnd::wndTopMost,
				0, 0, cx, cy, 0);
		else
			pApp->m_pMainWnd->SetWindowPos(&CWnd::wndNoTopMost,
				0, 0, cx, cy, 0);
	}

	wndCover.DestroyWindow();
	pApp->m_dwUnk03B8 = 0;
	return pApp->m_dwUnk03B0;
}

// Leaves full-screen mode again: restores the display mode and everything
// Sub_4253D0 saved. Returns whether the mode switch worked.

// hover.exe:0x425600
// FUNCTION: HOVER 0x425600
BOOL CHoverApp::Sub_425600()
{
	BOOL bOk;
	CBumperView* pView;

	bOk = (m_pfnChangeDisplaySettings(NULL, CDS_FULLSCREEN) == 0);
	m_dwUnk03B0 = 0;
	m_pMainWnd->ModifyStyleEx(0, m_dwUnk03C4, 0);
	m_pMainWnd->ModifyStyle(WS_POPUP | WS_VISIBLE, m_dwUnk03C0,
		SWP_FRAMECHANGED);
	if (((CMainFrame*)m_pMainWnd)->m_dwUnk00CC == 0)
		((CMainFrame*)m_pMainWnd)->m_wndStatusBar.ShowWindow(SW_SHOW);

	pView = Hover_sub_401140();
	pView->m_dwUnk0544 = 0;
	pView->ModifyStyleEx(0, WS_EX_CLIENTEDGE, 0);
	pView->ModifyStyle(0, WS_BORDER, SWP_FRAMECHANGED);

	m_pMainWnd->SetMenu(CMenu::FromHandle(m_hMenuUnk03D8));
	m_pMainWnd->SetWindowPos(&CWnd::wndNoTopMost,
		m_rcUnk03C8.left, m_rcUnk03C8.top,
		m_rcUnk03C8.right - m_rcUnk03C8.left,
		m_rcUnk03C8.bottom - m_rcUnk03C8.top, 0);
	m_pMainWnd->UpdateWindow();
	m_dwUnk03E8 = 0;
	return bOk;
}

/////////////////////////////////////////////////////////////////////////////
// CHoverApp message handlers
//
// The application object owns the game commands: the frame window has no
// handlers of its own, so every menu and accelerator command lands here
// through the command routing chain. Handlers that open a modal dialog
// first push the game into its "paused" state with
// CBumperView::Sub_40EE90(1).

// Standard Help/About: the game does not use the About dialog template
// (100) but the shell's ShellAbout box with the application icon.

// hover.exe:0x425390
// FUNCTION: HOVER 0x425390
void CHoverApp::OnAppAbout()
{
	Hover_sub_401140()->Sub_40EE90(1);
	ShellAbout(AfxGetApp()->m_pMainWnd->m_hWnd, "Hover!", "",
		::LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDR_MAINFRAME)));
}

// ID_CMD_34012 (debug): edits the eight camera/view geometry values of the
// application object (the five field-of-view angles and the three
// floating-point tuning factors) in a dialog whose template (130) was
// stripped from the shipped resources.

// hover.exe:0x425730
// FUNCTION: HOVER 0x425730
void CHoverApp::OnCmd34012()
{
	CSub42C890 dlg(NULL);

	Hover_sub_401140()->Sub_40EE90(1);

	dlg.m_dwUnk44 = m_dwUnk015C;
	dlg.m_dwUnk48 = m_dwUnk0160;
	dlg.m_dwUnk4C = m_dwUnk0168;
	dlg.m_dwUnk50 = m_dwUnk0158;
	dlg.m_dwUnk54 = m_dwUnk0164;
	dlg.m_dblUnk58 = m_dblUnk01A0;
	dlg.m_dblUnk68 = m_dblUnk01A8;
	dlg.m_dblUnk60 = m_dblUnk01B0;

	if (dlg.DoModal() == IDOK)
	{
		m_dwUnk015C = dlg.m_dwUnk44;
		m_dwUnk0160 = dlg.m_dwUnk48;
		m_dwUnk0168 = dlg.m_dwUnk4C;
		m_dwUnk0158 = dlg.m_dwUnk50;
		m_dwUnk0164 = dlg.m_dwUnk54;
		m_dblUnk01A0 = dlg.m_dblUnk58;
		m_dblUnk01A8 = dlg.m_dblUnk68;
		m_dblUnk01B0 = dlg.m_dblUnk60;
	}
}

// Options|Customize Game: on OK the skill level chosen in the dialog is
// turned into the three active parameter blocks (the same selection
// Sub_425870 makes), the human player and every robot on the board are
// pointed at the new blocks, and the view's projection factors are
// recomputed.

// hover.exe:0x4137D0
// FUNCTION: HOVER 0x4137d0
void CHoverApp::OnOptionsCustomize()
{
	CBumperDoc* pDoc;
	POSITION pos;
	CObject* pObj;

	Hover_sub_401140()->Sub_40EE90(1);

	CSub413F10 dlg(NULL);

	if (dlg.DoModal() == IDOK)
	{
		// The skill-level switch of Sub_425870, spelled out again
		switch (m_dwUnk01C8)
		{
		case 0:
			m_pUnk014C = &g_HBegParams;
			m_pUnk0150 = &g_RBegParams;
			m_pUnk0154 = &g_RobotAIBegParams;
			break;
		case 1:
			m_pUnk014C = &g_HIntParams;
			m_pUnk0150 = &g_RIntParams;
			m_pUnk0154 = &g_RobotAIIntParams;
			break;
		case 2:
			m_pUnk014C = &g_HExpParams;
			m_pUnk0150 = &g_RExpParams;
			m_pUnk0154 = &g_RobotAIExpParams;
			break;
		}

		pDoc = theApp.m_pBumperDoc;
		pDoc->m_humanPlayer.Sub_413ED0(m_pUnk014C);
		Hover_sub_401140()->m_obj00E8.Sub_4140E0();

		for (pos = pDoc->m_list83A8.GetHeadPosition(); pos != NULL; )
		{
			pObj = pDoc->m_list83A8.GetNext(pos);
			if (pObj->IsKindOf(RUNTIME_CLASS(CRobotPlayer)))
				((CRobotPlayer*)pObj)->Sub_413ED0(m_pUnk0150);
		}
	}
}

// Options|Player Controls: on OK the view re-reads the key/joystick
// bindings and the worker thread is pushed through one frame so the new
// bindings take effect immediately.

// hover.exe:0x4258F0
// FUNCTION: HOVER 0x4258f0
void CHoverApp::OnOptionsControls()
{
	Hover_sub_401140()->Sub_40EE90(1);

	CSub42D910 dlg(NULL);

	if (dlg.DoModal() == IDOK)
	{
		Hover_sub_401140()->Sub_413B10(1);
		Hover_sub_401140()->m_dwUnk0080 = 1;
		Hover_sub_401140()->Sub_40DC20(1);
	}
}

// Options|Sound Options: the dialog applies its own settings, so the
// result is ignored.

// hover.exe:0x4259A0
// FUNCTION: HOVER 0x4259a0
void CHoverApp::OnOptionsSounds()
{
	Hover_sub_401140()->Sub_40EE90(1);

	CSub42E790 dlg(NULL);

	dlg.DoModal();
}

// hover.exe:0x413940
// FUNCTION: HOVER 0x413940
void CHoverApp::OnUpdateOptionsSounds(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(TRUE);
}

// Options|Full Screen: toggles between the windowed and the full-screen
// display mode. Holding Shift while choosing the command enters
// full screen without the 2x pixel doubling (the view's m_dwUnk0544
// flag). Sub_4274B0 resizes the offscreen bitmap unless the mode switch
// already did so (m_dwUnk0088).

// hover.exe:0x425A20
// FUNCTION: HOVER 0x425a20
void CHoverApp::OnOptionsFullScreen()
{
	CBumperView* pView;
	DWORD* p;

	pView = Hover_sub_401140();
	pView->Sub_40EE90(1);
	pView->m_dwUnk0088 = 0;

	if (m_dwUnk03B0 != 0)
	{
		pView->m_dwUnk0544 = 0;
		m_dwUnk03E4 = 0;
		Sub_425600();
	}
	else
	{
		if ((GetKeyState(VK_SHIFT) & 0x8000) != 0)
		{
			pView->m_dwUnk0544 = 0;
		}
		else
		{
			pView->m_dwUnk0544 = 1;
			if (m_dwUnk03E0 != 0)
				m_dwUnk03E4 = 1;
		}
		Sub_4253D0();
	}

	if (pView->m_dwUnk0088 == 0)
	{
		p = &pView->m_dwUnk00A4;
		pView->Sub_4274B0(p[0], pView->m_dwUnk00A8, 1);
	}
	pView->Sub_40DC20(1);
}

// hover.exe:0x413950
// FUNCTION: HOVER 0x413950
void CHoverApp::OnUpdateOptionsFullScreen(CCmdUI* pCmdUI)
{
	// Enabled when ChangeDisplaySettings was resolved or the display is
	// already known to support the mode switch
	pCmdUI->Enable(m_pfnChangeDisplaySettings != NULL || m_dwUnk03E0 != 0);
	pCmdUI->SetCheck(m_dwUnk03B0);
}

// The level was completed: the view's WM_USER+2 handler routes here.
// Parks the render thread, credits the end-of-level score (m_dwUnk01C0 is
// the "award the level bonus" flag, set when the level was played through
// rather than restarted), switches the view into the between-levels state,
// steps the level counter and loads the next level. The flag is cleared
// before the load so a restart of the new level does not pay the bonus
// twice.

// hover.exe:0x425AD0
// FUNCTION: HOVER 0x425ad0
void CHoverApp::Sub_425AD0()
{
	CBumperView* pView = Hover_sub_401140();

	pView->Sub_4279E0();
	pView->Sub_4282F0(m_dwUnk01C0);
	pView->Sub_41F4E0(3, 1);
	g_dwUnk_46049C++;
	m_dwUnk01C0 = 0;
	Sub_41C8E0();
}

// The game is over (the drones took the last flag): the view's WM_USER+1
// handler routes here. Stops the game, offers the high-score entry for the
// score just made, resets the total and asks whether to play again; "Yes"
// runs the same Start At... flow as OnGameStartAt (hover.exe:0x425CB0),
// whose body from the high-score dialog on this function duplicates.
//
// Note the high-score dialog stays alive to the end of the function in the
// original, not just for the length of the if.

// hover.exe:0x425B20
// FUNCTION: HOVER 0x425b20
void CHoverApp::Sub_425B20()
{
	CBumperView* pView;
	int nLevel;

	m_dwUnk01B8 = 0;

	pView = Hover_sub_401140();
	pView->Sub_410280();
	pView->Sub_41F4E0(2, 1);

	CSub424BD0 dlgScores(NULL);

	if (dlgScores.Sub_424DF0(theApp.m_pBumperDoc->m_dwUnk83E4))
		dlgScores.DoModal();
	theApp.m_pBumperDoc->m_dwUnk83E4 = 0;

	if (AfxMessageBox(IDS_DEFEATED, MB_YESNO | MB_ICONQUESTION,
			(UINT)-1) == IDYES)
	{
		pView = Hover_sub_401140();
		pView->Sub_410280();
		theApp.m_pBumperDoc->m_humanPlayer.Sub_414BD0(1);

		CSub42F640 dlg(NULL);

		if (dlg.DoModal() == IDOK)
		{
			nLevel = (int)g_dwUnk_46048C - 1;
			g_dwUnk_46049C = nLevel;
			if (nLevel >= 20)
				g_dwUnk_46049C = 19;
			pView->Sub_41E5B0();
			pView->Sub_41F4E0(0, 1);
			pView->RedrawWindow();
			Sub_41C8E0();
			m_dwUnk01C0 = 1;
		}
		else
		{
			m_dwUnk01C0 = 0;
		}
	}
}

// Game|Start At...: ends a game in progress (offering the high score
// entry), asks for the starting level and starts a new game there.
// g_dwUnk_46048C is the level the dialog left behind; the game's level
// counter g_dwUnk_46049C is zero based and capped at 19.

// hover.exe:0x425CB0
// FUNCTION: HOVER 0x425cb0
void CHoverApp::OnGameStartAt()
{
	CBumperView* pView;
	int nLevel;

	if (m_dwUnk01B8 != 0)
	{
		if (AfxMessageBox(IDS_QUIT_IN_PROGRESS, MB_YESNO, (UINT)-1) == IDNO)
			return;
	}

	m_dwUnk01B8 = 0;

	pView = Hover_sub_401140();
	pView->Sub_410280();
	pView->Sub_41F4E0(2, 1);

	CSub424BD0 dlgScores(NULL);

	if (dlgScores.Sub_424DF0(theApp.m_pBumperDoc->m_dwUnk83E4))
		dlgScores.DoModal();
	theApp.m_pBumperDoc->m_dwUnk83E4 = 0;

	pView = Hover_sub_401140();
	pView->Sub_410280();
	theApp.m_pBumperDoc->m_humanPlayer.Sub_414BD0(1);

	CSub42F640 dlg(NULL);

	if (dlg.DoModal() == IDOK)
	{
		nLevel = (int)g_dwUnk_46048C - 1;
		g_dwUnk_46049C = nLevel;
		if (nLevel >= 20)
			g_dwUnk_46049C = 19;
		pView->Sub_41E5B0();
		pView->Sub_41F4E0(0, 1);
		pView->RedrawWindow();
		Sub_41C8E0();
		m_dwUnk01C0 = 1;
	}
	else
	{
		m_dwUnk01C0 = 0;
	}
}

// ID_CMD_34006 (debug): toggles the flag at +0x3BC.

// hover.exe:0x425EC0
// FUNCTION: HOVER 0x425ec0
void CHoverApp::OnCmd34006()
{
	m_dwUnk03BC = !m_dwUnk03BC;
}

// hover.exe:0x425EE0
// FUNCTION: HOVER 0x425ee0
void CHoverApp::OnUpdateCmd34006(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(m_dwUnk03BC);
}

// ID_CMD_34007 (debug): toggles the flag at +0x1E0.

// hover.exe:0x425F00
// FUNCTION: HOVER 0x425f00
void CHoverApp::OnCmd34007()
{
	m_dwUnk01E0 = !m_dwUnk01E0;
}

// hover.exe:0x425F20
// FUNCTION: HOVER 0x425f20
void CHoverApp::OnUpdateCmd34007(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(m_dwUnk01E0);
}

// ID_CMD_34009 (debug): edits the robot AI parameters (template 149, also
// stripped) and re-points every robot on the board at the R parameter
// block afterwards.

// hover.exe:0x425F40
// FUNCTION: HOVER 0x425f40
void CHoverApp::OnCmd34009()
{
	POSITION pos;
	CObject* pObj;

	Hover_sub_401140()->Sub_40EE90(1);

	CSub4319A0 dlg(NULL);

	dlg.DoModal();

	for (pos = theApp.m_pBumperDoc->m_list83A8.GetHeadPosition(); pos != NULL; )
	{
		pObj = theApp.m_pBumperDoc->m_list83A8.GetNext(pos);
		if (pObj->IsKindOf(RUNTIME_CLASS(CRobotPlayer)))
			((CRobotPlayer*)pObj)->Sub_413ED0(m_pUnk0150);
	}
}

// ID_CMD_34008 (debug): edits the human player parameters (template 148,
// also stripped) and reloads them into the human player and the view's
// projection factors.

// hover.exe:0x426000
// FUNCTION: HOVER 0x426000
void CHoverApp::OnCmd34008()
{
	Hover_sub_401140()->Sub_40EE90(1);

	CSub430780 dlg(NULL);

	dlg.DoModal();

	theApp.m_pBumperDoc->m_humanPlayer.Sub_413ED0(m_pUnk014C);
	Hover_sub_401140()->m_obj00E8.Sub_4140E0();
}

// hover.exe:0x4260B0
// FUNCTION: HOVER 0x4260b0
void CHoverApp::OnGameHighScores()
{
	Hover_sub_401140()->Sub_40EE90(1);

	CSub424BD0 dlg(NULL);

	dlg.DoModal();
}

// hover.exe:0x412240
// FUNCTION: HOVER 0x412240
void CHoverApp::OnUpdateGameHighScores(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(TRUE);
}

// hover.exe:0x426130
// FUNCTION: HOVER 0x426130
void CHoverApp::OnHelpQuickHelp()
{
	Hover_sub_401140()->Sub_40EE90(1);

	CQuickHelpDialog dlg(NULL);

	dlg.DoModal();
}

// hover.exe:0x4261A0
// FUNCTION: HOVER 0x4261a0
void CHoverApp::OnUpdateHelpQuickHelp(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(TRUE);
}

// Game|Start (F2): ends a game in progress (offering the high score entry
// first), resets the board and starts a new game at level 1.

// hover.exe:0x41D410
// FUNCTION: HOVER 0x41d410
void CHoverApp::OnGameStart()
{
	CBumperView* pView;

	Hover_sub_401140()->Sub_40EE90(1);

	if (m_dwUnk01B8 != 0)
	{
		if (AfxMessageBox(IDS_QUIT_IN_PROGRESS, MB_YESNO, (UINT)-1) == IDNO)
			return;
	}

	CSub424BD0 dlgScores(NULL);

	if (dlgScores.Sub_424DF0(theApp.m_pBumperDoc->m_dwUnk83E4))
		dlgScores.DoModal();
	theApp.m_pBumperDoc->m_dwUnk83E4 = 0;

	pView = Hover_sub_401140();
	pView->Sub_410280();
	theApp.m_pBumperDoc->m_humanPlayer.Sub_414BD0(1);
	g_dwUnk_46049C = 0;
	m_dwUnk01C0 = 0;
	pView->Sub_41E5B0();
	pView->Sub_41F4E0(0, 1);
	pView->RedrawWindow();
	Sub_41C8E0();
}

// ID_CMD_34031 (debug): opens the dialog on template 152 (also stripped).

// hover.exe:0x4261B0
// FUNCTION: HOVER 0x4261b0
void CHoverApp::OnCmd34031()
{
	Hover_sub_401140()->Sub_40EE90(1);

	CSub432A50 dlg(NULL);

	dlg.DoModal();
}

// ID_CMD_32821 (debug): deactivates the WaveMix session, runs its own
// configuration dialog and reactivates it. When the output format changed
// the loaded sounds no longer match, so the user is told to reload a level.

// hover.exe:0x426230
// FUNCTION: HOVER 0x426230
void CHoverApp::OnCmd32821()
{
	SOUNDENGINE* pEngine;
	CBumperView* pView;
	HWND hWnd;
	DWORD dwChanged;

	hWnd = NULL;
	Hover_sub_401140()->Sub_40EE90(1);
	dwChanged = 0;
	Hover_sub_439180(g_pUnk_4C56DC, 0);
	pEngine = g_pUnk_4C56DC;

	pView = Hover_sub_401140();
	if (pView != NULL)
		hWnd = pView->m_hWnd;

	if (Hover_sub_43C8F0(pEngine, (DWORD)hWnd, NULL,
			(DWORD)&dwChanged, 1) == 0 && dwChanged != 0)
		AfxMessageBox("Reload a new level to restart sounds!", 0, 0);

	Hover_sub_439180(g_pUnk_4C56DC, 1);
}
