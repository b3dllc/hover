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
#include "texmisc.h"		// Hover_FatalException (exception reporter)

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
	// g_dwLevel to 20 and the loader indexes g_lvlParams directly,
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
DWORD g_cbTextureHeap;			// hover.exe:0x46002C
// GLOBAL: HOVER 0x46003c
void* g_pTextureHeap;			// hover.exe:0x46003C
// GLOBAL: HOVER 0x461cfc
DWORD g_dwMipBias;			// hover.exe:0x461CFC
// GLOBAL: HOVER 0x4c56d8
DWORD g_bUseWaveMix;			// hover.exe:0x4C56D8
// GLOBAL: HOVER 0x4c4ce8
FARPROC g_pfnJoyGetPosEx;		// hover.exe:0x4C4CE8
// GLOBAL: HOVER 0x4c4738
CObject* g_pUnk_4C4738;			// hover.exe:0x4C4738
// GLOBAL: HOVER 0x46048c
DWORD g_dwStartAtLevel;			// hover.exe:0x46048C
// GLOBAL: HOVER 0x46049c
DWORD g_dwLevel;			// hover.exe:0x46049C
// A global object, not a plain pair of initialised DWORDs: hover.exe sets
// it to (-1, -1) from a compiler-generated static initializer
// (hover.exe:0x42FE10, two stores of -1), which only a constructor
// produces; a constant initializer would have gone into .data instead.
// GLOBAL: HOVER 0x4a4878
CPoint g_ptSplashAnchor(-1, -1);	// hover.exe:0x4A4878

// Per-skill-level parameter blocks (the game's skill tuning), overwritten
// from the registry by
// Hover_LoadHumanParams/Hover_LoadRobotParams/Hover_LoadRobotAIParams (H =
// human player, R = robot opponents, RobotAI = robot AI).
// CHoverApp::SelectSkillParams points m_pHumanParams/m_pRobotParams/
// m_pRobotAIParams at the set for the current skill level.
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
// down by CHoverApp::StartLevel); Hover_LoadProgress paints the progress bar
// into it.
// GLOBAL: HOVER 0x4c57d4
CLoadingDialog* g_pLoadingDlg;	// hover.exe:0x4C57D4

// Pointer lists emptied by the subsystem shutdown helpers called from
// CHoverApp::ExitInstance (element types not yet known)
// GLOBAL: HOVER 0x4a45b8
CPtrList g_listPodObjBlocks;		// hover.exe:0x4A45B8
// GLOBAL: HOVER 0x4a45e0
CPtrList g_listFloorObjBlocks;		// hover.exe:0x4A45E0
// GLOBAL: HOVER 0x4a46a0
CPtrList g_listRobotBlocks;		// hover.exe:0x4A46A0
// GLOBAL: HOVER 0x4a46d0
CPtrList g_listWallBlocks;		// hover.exe:0x4A46D0
// GLOBAL: HOVER 0x4a46f8
CPtrList g_listRegionNodeBlocks;		// hover.exe:0x4A46F8
// GLOBAL: HOVER 0x4a4748
CPtrList g_listMerlinDynamicBlocks;		// hover.exe:0x4A4748
// GLOBAL: HOVER 0x4a4858
CPtrList g_listBeaconBlocks;		// hover.exe:0x4A4858

// GLOBAL: HOVER 0x4c5330
CPodObj* g_pPodObjFree;			// hover.exe:0x4C5330
// GLOBAL: HOVER 0x4c53e0
CFloorObj* g_pFloorObjFree;			// hover.exe:0x4C53E0
// GLOBAL: HOVER 0x4c54d0
CRobotPlayer* g_pRobotFree;			// hover.exe:0x4C54D0
// GLOBAL: HOVER 0x4c5588
CWall* g_pWallFree;			// hover.exe:0x4C5588
// GLOBAL: HOVER 0x4c55d0
struct tagREGIONNODE* g_pRegionNodeFree;			// hover.exe:0x4C55D0
// GLOBAL: HOVER 0x4c55d4
DWORD g_dwRegionNodeChunk = 0x40;		// hover.exe:0x4C55D4 REGIONNODE chunk size
// GLOBAL: HOVER 0x4c5620
CMerlinDynamic* g_pMerlinDynamicFree;			// hover.exe:0x4C5620
// GLOBAL: HOVER 0x4c57a8
CBeacon* g_pBeaconFree;			// hover.exe:0x4C57A8

// Game settings read/written as the 5-byte "GameSettings" registry value by
// Hover_LoadGameSettings/Hover_SaveGameSettings; the bytes are sign-extended
// into these DWORDs (theApp.m_dwSpeedDetail lives in bumpview.cpp)

// GLOBAL: HOVER 0x4c4ce0
DWORD g_bStartingTimer;			// hover.exe:0x4C4CE0

// GLOBAL: HOVER 0x4c4ce4
DWORD g_bTickPending;			// hover.exe:0x4C4CE4

// Worker-thread shutdown state used by Hover_ShutdownSound
// (the sound slot registry at 0x4A47F0 is a CObArray; its m_pData/m_nSize
// members at 0x4A47F4/0x4A47F8 were previously modeled as loose globals
// g_apUnk_4A47F4/g_nUnk_4A47F8 — see g_arrUnk_4A47F0 in gameobj.h)
// GLOBAL: HOVER 0x4a4808
CRITICAL_SECTION g_csSoundQueue;	// hover.exe:0x4A4808
// GLOBAL: HOVER 0x4c56dc
MIXSESSION* g_pMixSession;		// hover.exe:0x4C56DC sound driver engine object
// GLOBAL: HOVER 0x4c56e0
CWinThread* g_pSoundThread;	// hover.exe:0x4C56E0 sound mixing thread
// GLOBAL: HOVER 0x4c56e4
DWORD g_bSoundQueueCsInit;			// hover.exe:0x4C56E4 g_csSoundQueue initialized flag
// GLOBAL: HOVER 0x4c56e8
DWORD g_bSoundThreadQuit;			// hover.exe:0x4C56E8
// GLOBAL: HOVER 0x4c56f4
HANDLE g_hSoundWakeEvent;			// hover.exe:0x4C56F4
// GLOBAL: HOVER 0x4c56f8
HANDLE g_hSoundAckEvent;			// hover.exe:0x4C56F8
// GLOBAL: HOVER 0x4c56fc
DWORD g_bWaveBusyMsgShown;			// hover.exe:0x4C56FC "wave devices in use"
							// message box already shown
// GLOBAL: HOVER 0x4c5ad4
DWORD g_dwPlayNesting;			// hover.exe:0x4C5AD4 WaveMixPlay reentry count

// WaveMix engine state
// (Hover_SoundThreadProc/WaveMixConfigureInit/WaveMixOpenChannel): the queued
// sound objects have the sound data pointer at offset +4
// GLOBAL: HOVER 0x4c56ec
CSoundSlot* g_pSoundOnce;			// hover.exe:0x4C56EC queued one-shot sound
// GLOBAL: HOVER 0x4c56f0
CSoundSlot* g_pSoundLoop;			// hover.exe:0x4C56F0 queued looping sound
// GLOBAL: HOVER 0x4b6248
MIXSESSION* g_pCurSession;		// hover.exe:0x4B6248 current session
// GLOBAL: HOVER 0x4b6f14
MIXITFN g_pfnDefaultMixit;			// hover.exe:0x4B6F14
// GLOBAL: HOVER 0x4b6f18
MIXSESSION* g_pActiveSession;		// hover.exe:0x4B6F18 active session (WaveMixActivate)
// GLOBAL: HOVER 0x4b6d50
CHANNELNODE* g_pFreeChannelNodes;	// hover.exe:0x4B6D50 free channel-node list
// GLOBAL: HOVER 0x4b6e70
XWAVEHDR* g_apWaveBlocksA[0x14];	// hover.exe:0x4B6E70 wave block array
// GLOBAL: HOVER 0x4b6ec0
XWAVEHDR* g_apWaveBlocksB[0x14];	// hover.exe:0x4B6EC0 wave block array 2
// GLOBAL: HOVER 0x4b6f10
HINSTANCE g_hWaveMixInst;		// hover.exe:0x4B6F10 module instance
// GLOBAL: HOVER 0x4b61c8
CHANNELNODE* g_apMixChannelNodes[0x10];	// hover.exe:0x4B61C8 channel node table
// GLOBAL: HOVER 0x4b6208
void* g_apMixSources[0x10];		// hover.exe:0x4B6208 mix source pointers
// GLOBAL: HOVER 0x4b6f20
WORD g_awMixVolumes[0x20];		// hover.exe:0x4B6F20 mix volume pairs
// GLOBAL: HOVER 0x4b4a58
CHANNELNODE g_aChannelNodePool[100];	// hover.exe:0x4B4A58 pool of
						// 100 channel nodes, linked into
						// the g_pFreeChannelNodes free list by
						// WaveMix_InitChannelNodes
// GLOBAL: HOVER 0x4b6250
BYTE g_abVolumeTable[11][256];	// hover.exe:0x4B6250 sample-volume table
						// (built by WaveMix_InitVolumeTable, indexed
						// [volume][sample])
// The original leaves 0x118 bytes here (up to g_apWaveBlocksA), so the
// MAX_PATH limit below fits; a 0x100 buffer would be overrun.
// GLOBAL: HOVER 0x4b6d58
char g_szIniFileName[MAX_PATH];		// hover.exe:0x4B6D58 ini filename buffer
					// (filled at runtime; size unknown)
// GLOBAL: HOVER 0x4c59b8
DWORD g_bShowDebugDialogs;			// hover.exe:0x4C59B8 show debug dialogs
// GLOBAL: HOVER 0x4c59bc
DWORD g_dwDebugLevel;			// hover.exe:0x4C59BC debug output verbosity
// GLOBAL: HOVER 0x4c59c0
XWAVEHDR* g_pPlayQueueHead;		// hover.exe:0x4C59C0 playing queue head
// GLOBAL: HOVER 0x4c59c4
XWAVEHDR* g_pPlayQueueTail;		// hover.exe:0x4C59C4 playing queue tail
// GLOBAL: HOVER 0x4c59c8
DWORD g_bWaveMixInitialized;			// hover.exe:0x4C59C8
// GLOBAL: HOVER 0x4c59d0
char g_szStringBuffer[0x100] =		// hover.exe:0x4C59D0 version banner,
	"WaveMix V 2.3 by Angel M. Diaz, Jr. (c) Microsoft 1993-1995";
					// reused as the wsprintfA scratch buffer

// Default WaveMix output format copied into the session by
// WaveMixConfigureInit (PCM, mono, 11025 Hz, 8-bit)
// hover.exe:0x4BFE30
// GLOBAL: HOVER 0x4bfe30
extern const PCMWAVEFORMAT g_wfxDefaultMix =
	{ { WAVE_FORMAT_PCM, 1, 11025, 11025, 1 }, 8 };

// Sound subsystem state (Hover_LoadSoundSettings/Hover_InitSound)
// GLOBAL: HOVER 0x4c56d0
DWORD g_bMixSessionOpen;			// hover.exe:0x4C56D0 sound driver engine started
// GLOBAL: HOVER 0x4c56d4
DWORD g_bWaveOutAvailable;			// hover.exe:0x4C56D4 wave output available

// In-game key bindings (virtual-key codes): defaults loaded from the
// string table by Hover_LoadControls, overridden by the 0x30-byte
// "PlayerControls" registry value; [11] is the joystick button binding

// Joystick state (Hover_LoadControls)

// Fixed-point math tables built by Hover_BuildTrigTables (entry 0 unused):
// g_adwSinTable[i] = sin((i-1) * pi / 4096) * 65536,
// g_awTanTable[i]  = tan((i-1) * pi / 65536) * 65536
// GLOBAL: HOVER 0x4aca54
DWORD g_adwSinTable[0x2001];		// hover.exe:0x4ACA54
// GLOBAL: HOVER 0x4a4a56
WORD g_awTanTable[0x4001];		// hover.exe:0x4A4A56

// Board zone geometry, built by Hover_InitZones.
// hover.exe:0x4609A0
// GLOBAL: HOVER 0x4609a0
ZONE_DESCRIPTOR g_aZones[6];

/////////////////////////////////////////////////////////////////////////////
// Free helpers

// Returns the main frame's active view (the game view window), or NULL
// when there is no main frame.
// hover.exe:0x401140
// FUNCTION: HOVER 0x401140
CBumperView* Hover_GetView()
{
	if (*(CFrameWnd* volatile*)&theApp.m_pMainWnd == NULL)
		return NULL;
	return (CBumperView*)((CFrameWnd*)theApp.m_pMainWnd)->GetActiveView();
}

// Processes the primary CGameObj list (g_pDirtyObjList): calls each object's
// vtable slot 0x3c method, clears its list link and, if it was not already
// on the secondary list, moves it to g_pDrawObjList. Finally zeros the
// primary list head.
// hover.exe:0x408780
// FUNCTION: HOVER 0x408780
void Hover_CommitDirtyObjects()
{
	CGameObj* pObj;
	CGameObj* pNext;

	pObj = g_pDirtyObjList;
	while (pObj != NULL)
	{
		pNext = pObj->m_pNextStart;
		pObj->m_pNextStart = 0;
		pObj->m_bOnStartList = 0;
		pObj->BeginFrame();
		if (pObj->m_bOnEndList == 0)
		{
			pObj->m_pNextEnd = g_pDrawObjList;
			g_pDrawObjList = pObj;
			pObj->m_bOnEndList = 1;
		}
		pObj = pNext;
	}
	g_pDirtyObjList = 0;
}

// Clears the secondary CGameObj list (g_pDrawObjList): calls each object's
// vtable slot 0x40 method and clears its secondary list link, then zeros
// the secondary list head.
// hover.exe:0x406C60
// FUNCTION: HOVER 0x406c60
void Hover_ReleaseDrawList()
{
	CGameObj* pObj;
	CGameObj* pNext;

	pObj = g_pDrawObjList;
	while (pObj != NULL)
	{
		pNext = pObj->m_pNextEnd;
		pObj->m_pNextEnd = 0;
		pObj->m_bOnEndList = 0;
		pObj->EndFrame();
		pObj = pNext;
	}
	g_pDrawObjList = 0;
}

// hover.exe:0x40E560
// FUNCTION: HOVER 0x40e560
int Hover_WaveMixActivate(int nUnk)
{
	int nRet;

	if (g_bUseWaveMix == 0)
		return 0;
	if (g_pMixSession != NULL)
		return WaveMixActivate(g_pMixSession, nUnk);
	// The original returns the uninitialized local here
	return nRet;
}

// Waits on hEvent in 10 ms slices; while the wait keeps timing out it
// pumps WM_COPYDATA messages addressed to theApp.m_hwndDisplayDib so the
// synchronous wait cannot deadlock the shell's broadcast.

// hover.exe:0x427A10
// FUNCTION: HOVER 0x427a10
void Hover_WaitPumpCopyData(HANDLE hEvent)
{
	MSG msg;

	while (WaitForSingleObject(hEvent, 10) == WAIT_TIMEOUT)
	{
		if (PeekMessageA(&msg, theApp.m_hwndDisplayDib, WM_COPYDATA,
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
	ON_COMMAND(ID_DBG_PODTIMES, OnDbgPodTimes)
	ON_COMMAND(ID_OPTIONS_CUSTOMIZE, OnOptionsCustomize)
	ON_COMMAND(ID_OPTIONS_CONTROLS, OnOptionsControls)
	ON_COMMAND(ID_OPTIONS_SOUNDS, OnOptionsSounds)
	ON_UPDATE_COMMAND_UI(ID_OPTIONS_SOUNDS, OnUpdateOptionsSounds)
	ON_COMMAND(ID_OPTIONS_FULLSCREEN, OnOptionsFullScreen)
	ON_UPDATE_COMMAND_UI(ID_OPTIONS_FULLSCREEN, OnUpdateOptionsFullScreen)
	ON_COMMAND(ID_GAME_STARTAT, OnGameStartAt)
	ON_COMMAND(ID_DBG_FULLSCREEN_TOPMOST, OnDbgFullScreenTopmost)
	ON_UPDATE_COMMAND_UI(ID_DBG_FULLSCREEN_TOPMOST, OnUpdateDbgFullScreenTopmost)
	ON_COMMAND(ID_DBG_SHOW_BEACONS, OnDbgShowBeacons)
	ON_UPDATE_COMMAND_UI(ID_DBG_SHOW_BEACONS, OnUpdateDbgShowBeacons)
	ON_COMMAND(ID_DBG_ROBOTPARAMS, OnDbgRobotParams)
	ON_COMMAND(ID_DBG_HUMANPARAMS, OnDbgHumanParams)
	ON_COMMAND(ID_GAME_HIGHSCORES, OnGameHighScores)
	ON_UPDATE_COMMAND_UI(ID_GAME_HIGHSCORES, OnUpdateGameHighScores)
	ON_COMMAND(ID_HELP_QUICKHELP, OnHelpQuickHelp)
	ON_UPDATE_COMMAND_UI(ID_HELP_QUICKHELP, OnUpdateHelpQuickHelp)
	ON_COMMAND(ID_GAME_START, OnGameStart)
	ON_COMMAND(ID_DBG_ROBOTAIPARAMS, OnDbgRobotAIParams)
	ON_COMMAND(ID_DBG_WAVEMIX_CONFIG, OnDbgWaveMixConfig)
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
	m_dwSpeedPodTime = 360;
	m_dwInvinciblePodTime = 512;
	m_dwUnk016C = 60;
	m_dwSlowPodTime = 360;
	m_dblUnk01A8 = 10.0;
	m_dblUnk01A0 = 0.3;
	m_bSuspended = 0;
	m_dwWallPodTime = 180;
	m_dwSmokePodTime = 180;
	m_pBumperDoc = NULL;
	m_dwSpeedDetail = 1;
	m_dwSkill = 1;
	m_dwMazeOrder = 0;
	m_dblUnk01B0 = 0.55;
	m_bRotateMap = 1;

	SYSTEM_INFO sysInfo;
	GetSystemInfo(&sysInfo);
	if (sysInfo.dwProcessorType == PROCESSOR_INTEL_486)
		m_bRearView = 0;
	else
		m_bRearView = 1;

	m_bShowBeacons = 0;
	m_bFullScreen = 0;
	m_bSwitchingMode = 0;
	m_bDisplayDibAvailable = 0;
	m_bUseDisplayDib = 0;
	m_dwUnk01D8 = 1;
	m_bDisplayDibActive = 0;
	m_pbmiDisplayDib = (BITMAPINFO*)new BYTE[BMI8_SIZE];
	m_hwndDisplayDib = 0;

	DWORD dwVersion = GetVersion();
	m_dwWinVersion = (DWORD)((LOBYTE(LOWORD(dwVersion)) << 8) +
		HIBYTE(LOWORD(dwVersion)));
	m_bNotNT = (dwVersion & 0x80000000) ? 1 : 0;
	// The "is this Windows 95" test goes through a local: hover.exe
	// computes it (cmp cl,4 / sbb ecx,ecx / inc ecx) before the NULL store,
	// keeps it in a register across that store and the `add edi,0x3dc`
	// rebase, and then tests the register (`test ecx,ecx`) instead of the
	// member it has just written.  The local is also the one extra name
	// this function's frame needs: with it the running-total block below
	// falls onto hover.exe's register choices (idxB in ebx through the
	// jump form, idxC in ecx through the fall-through form, and the
	// `lea ecx,[eax+ebx*2]` after the imul), which is worth more than the
	// GetVersion tail itself - 85.4% -> 89.4% for the two together.
	BOOL bWin95 = (LOBYTE(LOWORD(dwVersion)) >= 4);
	m_pfnChangeDisplaySettings = NULL;
	m_bNewShell = bWin95;
	if (bWin95)
	{
		// ChangeDisplaySettings is only available on Windows 95;
		// look it up dynamically so the game still runs on NT 3.51
		m_pfnChangeDisplaySettings =
			(LPCHANGEDISPLAYSETTINGS)GetProcAddress(
				GetModuleHandle("user32.dll"), "ChangeDisplaySettingsA");
	}

	g_dwLevel = 0;
	g_dwStartAtLevel = 1;
	m_bFullScreenTopmost = 0;
	m_dwGameState = 0;
	m_bLevelBonus = 0;

	// Compute the runtime fields of the level parameter table. The
	// scale factor is 16.16 fixed point, starting at 1.0 and increasing
	// by 0.5 per level; the original builds both halves of the two
	// constants by hand and scales through the by-pointer 16-bit multiply
	// (HOVER_FIXMULWP), reading the product's integer half back out of
	// memory as a signed short.
	LVL_PARAMS* pLvl;
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
		pLvl = &LVLPARAM(i);
		dwProd1 = dwScale;
		HOVER_FIXMULWP(dwProd1, 1000);
		pLvl->m_anCaptureScore[1] = FX_INT(dwProd1);
		pLvl = &LVLPARAM(i);
		dwProd2 = dwScale;
		HOVER_FIXMULWP(dwProd2, 2500);
		pLvl->m_anRemainingScore[1] = FX_INT(dwProd2);
		if (i != 0)
		{
			// Written out (not "+="): the original loads the score, adds
			// the product and stores, it does not read-modify-write. The
			// third term is the PREVIOUS level's score (LVLPARAM(i-1),
			// confirmed by the original's `lea eax,[esi-1]` index calc at
			// hover.exe:0x41c3d0) - this is a running total across levels,
			// not a self-reference.
			pLvl = &LVLPARAM(i);
			pLvl->m_anLevelScore[1] = LVLPARAM(i).m_dwUnk30 *
				LVLPARAM(i).m_anCaptureScore[1] +
				LVLPARAM(i - 1).m_anLevelScore[1];
		}
		pLvl = &LVLPARAM(i);
		pLvl->m_anCaptureScore[0] = LVLPARAM(i).m_anCaptureScore[1] * 9 / 10;
		pLvl = &LVLPARAM(i);
		pLvl->m_anRemainingScore[0] = LVLPARAM(i).m_anRemainingScore[1] * 9 / 10;
		if (i != 0)
		{
			pLvl = &LVLPARAM(i);
			pLvl->m_anLevelScore[0] = LVLPARAM(i).m_anLevelScore[1] * 9 / 10;
		}
		pLvl = &LVLPARAM(i);
		pLvl->m_anCaptureScore[2] = LVLPARAM(i).m_anCaptureScore[1] * 11 / 10;
		pLvl = &LVLPARAM(i);
		pLvl->m_anRemainingScore[2] = LVLPARAM(i).m_anRemainingScore[1] * 11 / 10;
		if (i != 0)
		{
			pLvl = &LVLPARAM(i);
			pLvl->m_anLevelScore[2] = LVLPARAM(i).m_anLevelScore[1] * 11 / 10;
		}
		dwScale += dwStep;
	}

	// Levels 16 and up cycle over the last three entries (via the
	// LVLPARAM mapping), which all share the values computed for
	// entry 15 above.  The destination goes through the pointer here as
	// well - hover.exe evaluates the row address before it reads the
	// source field (`lea esi,[edx+edx*4]` comes before the
	// `mov eax,[g_lvlParams+15*0x58+n]`), which only happens when the
	// left-hand side is a value of its own; VC still folds the row
	// address into the store's displacement, so no instruction is added.
	for (i = 16; i < 20; i++)
	{
		pLvl = &LVLPARAM(i);
		pLvl->m_anCaptureScore[0] = g_lvlParams[15].m_anCaptureScore[0];
		pLvl = &LVLPARAM(i);
		pLvl->m_anCaptureScore[1] = g_lvlParams[15].m_anCaptureScore[1];
		pLvl = &LVLPARAM(i);
		pLvl->m_anCaptureScore[2] = g_lvlParams[15].m_anCaptureScore[2];
		pLvl = &LVLPARAM(i);
		pLvl->m_anRemainingScore[0] = g_lvlParams[15].m_anRemainingScore[0];
		pLvl = &LVLPARAM(i);
		pLvl->m_anRemainingScore[1] = g_lvlParams[15].m_anRemainingScore[1];
		pLvl = &LVLPARAM(i);
		pLvl->m_anRemainingScore[2] = g_lvlParams[15].m_anRemainingScore[2];
		pLvl = &LVLPARAM(i);
		pLvl->m_anLevelScore[0] = g_lvlParams[15].m_anLevelScore[0];
		pLvl = &LVLPARAM(i);
		pLvl->m_anLevelScore[1] = g_lvlParams[15].m_anLevelScore[1];
		pLvl = &LVLPARAM(i);
		pLvl->m_anLevelScore[2] = g_lvlParams[15].m_anLevelScore[2];
	}
}

// The destructor has no body of its own. hover.exe:0x426660 is the scalar
// deleting destructor (??_G) that sits in slot 1 of the vtable; the
// compiler folded the empty ~CHoverApp into it, so all that is left there
// is the m_strDataDir CString member destructor, the chain to ~CWinApp and
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
// Hover_FatalException (which aborts) and resumes at the "return TRUE".
// hover.exe:0x41C9E0
// hover.exe:0x4C47B0, defined in texload.cpp: InitInstance and the MIDI
// loader share one "winmm.dll" string.
extern char g_szWinMM[];

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

			InitializeCriticalSection(&m_csScene);
			InitializeCriticalSection(&m_csOffscreen);
			InitializeCriticalSection(&m_csUnused);
			InitializeCriticalSection(&m_csWorkerCmd);
			InitializeCriticalSection(&m_csInput);

			if (!CheckMinimumConfig())	// minimum configuration check
			{
				strMessage.LoadString(IDS_MIN_CONFIG);
				AfxMessageBox(strMessage, 0, 0);
				return FALSE;
			}
			if (!CheckSingleInstance())	// single instance check
				return FALSE;

			// Select a buffer size based on the amount of physical memory;
			// holding down a key at startup forces low memory mode. The
			// original passes 0x61 (ASCII 'a') to GetKeyState, which is
			// actually VK_NUMPAD1 -- apparently an original quirk/bug.
			up.memStatus.dwLength = sizeof(up.memStatus);
			GlobalMemoryStatus(&up.memStatus);
			g_dwMipBias = 0;
			if (up.memStatus.dwTotalPhys < 0x900000 || GetKeyState('a') < 0)
				g_dwMipBias = 1;
			switch (g_dwMipBias)
			{
			case 0:
				g_cbTextureHeap = 0x54C338;
				g_pTextureHeap = new BYTE[g_cbTextureHeap];
				break;
			case 1:
				g_cbTextureHeap = 0x1AF84C;
				g_pTextureHeap = new BYTE[g_cbTextureHeap];
				break;
			case 2:
				g_cbTextureHeap = 0x8AB3D;
				g_pTextureHeap = new BYTE[g_cbTextureHeap];
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

			Hover_LoadSoundSettings();
			if (!Hover_InitSound())
			{
				// retry with the flag flipped
				g_bUseWaveMix = !g_bUseWaveMix;
				if (!Hover_InitSound())
				{
					m_bEngineSound = 0;
					m_bGameSounds = 0;
					g_bUseWaveMix = !g_bUseWaveMix;
				}
			}

			// joyGetPosEx is optional; look it up dynamically
			dn.hWinMM = GetModuleHandle(g_szWinMM);
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

			Hover_InitZones();

			// Look for a MIDI output device that can play the game's music
			dn.nMidiDevices = midiOutGetNumDevs();
			if (dn.nMidiDevices == 0)
			{
				m_bMidiAvailable = 0;
				m_bMusic = 0;
			}
			else
			{
				m_bMidiAvailable = 0;
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
						m_bMidiAvailable = 1;
					}
				}
				if (m_bMidiAvailable == 0)
				{
					m_bMusic = 0;
				}
			}

			Hover_BuildTrigTables();

			m_pHumanParams = &g_HBegParams;
			m_pRobotParams = &g_RBegParams;
			m_pRobotAIParams = &g_RobotAIBegParams;

			Hover_LoadHumanParams();
			Hover_LoadRobotParams();
			Hover_LoadRobotAIParams();
			Hover_LoadGameSettings();
			Hover_LoadControls();
			SelectSkillParams();

			m_bAppActive = 1;

			FindDataDirectory();		// locate the game data directory

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
		Hover_FatalException(e);
	}
	return TRUE;
}
#pragma optimize("", on)

/////////////////////////////////////////////////////////////////////////////
// Game settings persistence
//
// Hover_LoadGameSettings (called by CHoverApp::InitInstance) loads the game
// settings from HKEY_CURRENT_USER\Software\Microsoft\Hover! and
// Hover_SaveGameSettings (called by CHoverApp::ExitInstance) saves them back.
// "GameSettings" is a 5-byte REG_BINARY blob of signed bytes: maze order,
// skill, rotate map, speed/detail and rear view, sign-extended into the
// matching theApp members. "StartAtLevel" is a 1-byte REG_BINARY read into
// the same buffer and clamped to the range 1..20. (A GAMESETTINGS struct
// in place of buf[5] compiles the same here but re-rolls register
// allocation across hover.cpp, -0.4% byte-weighted.)

// hover.exe:0x42D180
// FUNCTION: HOVER 0x42d180
void Hover_LoadGameSettings()
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
		theApp.m_dwMazeOrder = (signed char)buf[0];
		theApp.m_dwSkill = (signed char)buf[1];
		theApp.m_bRotateMap = (signed char)buf[2];
		theApp.m_dwSpeedDetail = (signed char)buf[3];
		theApp.m_bRearView = (signed char)buf[4];
	}

	cbData = 1;
	if (RegQueryValueExA(hKey, "StartAtLevel", NULL, &dwType, buf, &cbData)
			== ERROR_SUCCESS && cbData == 1)
	{
		g_dwStartAtLevel = (signed char)buf[0];
		if ((int)g_dwStartAtLevel < 1)
			g_dwStartAtLevel = 1;
		if ((int)g_dwStartAtLevel > 20)
			g_dwStartAtLevel = 20;
	}
	else
		g_dwStartAtLevel = 1;

	RegCloseKey(hKey);
}

// hover.exe:0x42D290
// FUNCTION: HOVER 0x42d290
void Hover_SaveGameSettings()
{
	HKEY hKey;
	DWORD dwDisposition;
	if (RegCreateKeyExA(HKEY_CURRENT_USER, "Software\\Microsoft\\Hover!",
			0, NULL, 0, KEY_ALL_ACCESS, NULL, &hKey, &dwDisposition)
			!= ERROR_SUCCESS)
		return;

	{
		BYTE buf[5];
		buf[0] = (BYTE)theApp.m_dwMazeOrder;
		buf[1] = (BYTE)theApp.m_dwSkill;
		buf[2] = (BYTE)theApp.m_bRotateMap;
		buf[3] = (BYTE)theApp.m_dwSpeedDetail;
		buf[4] = (BYTE)theApp.m_bRearView;
		RegSetValueExA(hKey, "GameSettings", 0, REG_BINARY, buf, 5);
	}

	{
		BYTE bLevel = (BYTE)g_dwStartAtLevel;
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
void Hover_LoadSoundSettings()
{
	MEMORYSTATUS memStatus;
	memStatus.dwLength = sizeof(memStatus);
	GlobalMemoryStatus(&memStatus);

	SYSTEM_INFO sysInfo;
	GetSystemInfo(&sysInfo);
	theApp.m_bMusic = 0;
	if (sysInfo.dwProcessorType != PROCESSOR_INTEL_486)
		theApp.m_bMusic = 1;

	theApp.m_bEngineSound = 1;
	theApp.m_bGameSounds = 1;
	g_bUseWaveMix = 1;

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
			theApp.m_bMusic = (signed char)buf[0];
			theApp.m_bEngineSound = (signed char)buf[1];
			theApp.m_bGameSounds = (signed char)buf[2];
			g_bUseWaveMix = (signed char)buf[3];
		}
		RegCloseKey(hKey);
	}
}

// Initializes the sound subsystem. When g_bUseWaveMix is zero the game
// mixes sound in a worker thread (requires a wave device supporting
// 22.05 kHz mono 8-bit); otherwise it creates the sound driver engine
// object. Returns FALSE on failure, which InitInstance retries once.

// hover.exe:0x423140
// FUNCTION: HOVER 0x423140
BOOL Hover_InitSound()
{
	g_bWaveOutAvailable = 0;
	g_bMixSessionOpen = 0;
	if (g_bUseWaveMix == 0)
	{
		if (waveOutGetNumDevs() != 0)
		{
			WAVEOUTCAPSA waveCaps;
			if (waveOutGetDevCapsA(0, &waveCaps, sizeof(waveCaps))
					== MMSYSERR_NOERROR &&
				(waveCaps.dwFormats & WAVE_FORMAT_2M08) != 0)
			{
				g_bWaveOutAvailable = 1;
			}
		}
		if (g_bWaveOutAvailable == 0)
			return FALSE;
		if (g_bSoundQueueCsInit == 0)
		{
			InitializeCriticalSection(&g_csSoundQueue);
			g_bSoundQueueCsInit = 1;
		}
		if (g_hSoundWakeEvent == NULL)
			g_hSoundWakeEvent = CreateEventA(NULL, TRUE, FALSE, NULL);
		if (g_hSoundAckEvent == NULL)
			g_hSoundAckEvent = CreateEventA(NULL, TRUE, FALSE, NULL);
		if (g_pSoundThread == NULL)
		{
			CWinThread* pThread =
				AfxBeginThread(Hover_SoundThreadProc, NULL);
			pThread->m_bAutoDelete = TRUE;
			g_bSoundThreadQuit = 0;
			g_pSoundThread = pThread;
		}
		return TRUE;
	}

	MIXCONFIG cfg;
	HKEY hKey = NULL;
	cfg.wSize = 0x1e;
	cfg.dwFlags = 3;
	cfg.wChannels = 1;
	cfg.wSamplingRate = 0xb;
	if (RegOpenKeyA(HKEY_LOCAL_MACHINE, "software\\Microsoft\\Hover!",
			&hKey) == ERROR_SUCCESS)
	{
		cfg.dwFlags |= 0x400;
		cfg.hRegistryKey = hKey;
	}
	g_pMixSession = WaveMixConfigureInit(&cfg);
	if (g_pMixSession == NULL)
		return FALSE;
	if (WaveMixOpenChannel(g_pMixSession, 1, 1) != 0)
		return FALSE;
	g_bMixSessionOpen = 1;
	if (hKey != NULL)
		RegCloseKey(hKey);
	return TRUE;
}


/////////////////////////////////////////////////////////////////////////////
// MIDI stream playback (CMidiStream)
//
// The eight midiStream* entry points are resolved from winmm.dll at run
// time by CMidiStream::LoadMusicFile (texload.cpp) into g_pfnMidiStreamOpen ..
// g_pfnMidiStreamStop, all declared with the same one-argument
// LPMIDIDEVICEFN type, so the three that take more arguments are cast
// back to their real prototype at the call. Every call is guarded by a
// NULL test whose "not resolved" arm yields MMSYSERR_ERROR - dead code,
// because LoadMusicFile clears g_pfnMidiStreamOpen again unless all eight
// pointers resolved, but the original emits it all the same.

typedef MMRESULT (WINAPI* LPFNMIDISTREAMOPEN)(LPHMIDISTRM, LPUINT, DWORD,
	DWORD, DWORD, DWORD);
typedef MMRESULT (WINAPI* LPFNMIDISTREAMPROPERTY)(HMIDISTRM, LPBYTE, DWORD);
typedef MMRESULT (WINAPI* LPFNMIDISTREAMOUT)(HMIDISTRM, LPMIDIHDR, UINT);
typedef MMRESULT (WINAPI* LPFNMIDISTREAMHANDLE)(HMIDISTRM);

// The MidiOutProc that CMidiStream::StartStream installs on the stream.
// Only MOM_DONE is handled, and only while the object that owns the
// stream is still published in g_pCurrentMidiStream. The finished buffer is
// first handed straight back to the driver, so the level music keeps
// looping for as long as midiStreamOut accepts it; when that fails (or
// when the object has already been marked stopped in m_bStopping) the
// buffer is appended to the m_pFreeHead/m_pFreeTail free list instead
// and the queued-buffer count m_dwQueued goes down by one.
//
// Note the callback runs on the MIDI driver's thread and touches the
// list with no synchronization at all - an original quirk.

// hover.exe:0x438440
// FUNCTION: HOVER 0x438440
void CALLBACK Hover_MidiStreamProc(HMIDIOUT hMidi, UINT uMsg, DWORD dwInstance,
	DWORD dwParam1, DWORD dwParam2)
{
	LPFNMIDISTREAMOUT pfnOut;
	CMidiStream* pObj;
	MIDIHDR* pHdr;
	MMRESULT mmr;

	pObj = g_pCurrentMidiStream;
	if (pObj == NULL)
		return;
	if (uMsg != MOM_DONE)
		return;
	pHdr = (MIDIHDR*)dwParam1;
	if (pObj->m_bStopping == 0)
	{
		pfnOut = (LPFNMIDISTREAMOUT)g_pfnMidiStreamOut;
		if (pfnOut != NULL)
		{
			mmr = pfnOut(pObj->m_hStream, pHdr,
				sizeof(MIDIHDR));
			// re-read rather than keep the object live across the
			// call: this is what puts it in ecx instead of costing
			// the frame a second callee-saved register
			pObj = g_pCurrentMidiStream;
		}
		else
			mmr = MMSYSERR_ERROR;
		if (mmr == 0)
			return;
		pObj->m_bStopping = 1;
	}
	pHdr->lpNext = NULL;
	if (pObj->m_pFreeTail == 0)
		pObj->m_pFreeHead = pHdr;
	else
		(pObj->m_pFreeTail)->lpNext = pHdr;
	pObj->m_pFreeTail = pHdr;
	pObj->m_dwQueued--;
}

// Opens the MIDI stream device for the .muz file LoadMusicFile loaded and
// hands the driver every prepared buffer of the m_pFreeHead list: the
// stream is opened on the MIDI mapper with Hover_MidiStreamProc as its
// callback function, each buffer is prepared with midiOutPrepareHeader,
// the file's time division (m_dwTimeDiv) is published through
// midiStreamProperty, the stream is paused so that nothing plays while
// the buffers are queued, and midiStreamRestart finally starts it.
// Queuing a buffer moves it off the m_pFreeHead list and bumps the
// in-driver count m_dwQueued; Hover_MidiStreamProc puts it back. Any failure
// tears the whole stream down again through CloseStream and returns 0.

// hover.exe:0x438300
// FUNCTION: HOVER 0x438300
int CMidiStream::StartStream()
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
	pfnOpen = (LPFNMIDISTREAMOPEN)g_pfnMidiStreamOpen;
	m_hStream = 0;
	m_dwQueued = 0;
	m_bStopping = 0;
	uDeviceID = MIDI_MAPPER;
	if (pfnOpen != NULL)
		mmr = pfnOpen(&m_hStream, &uDeviceID, 1,
			(DWORD)Hover_MidiStreamProc, 0, CALLBACK_FUNCTION);
	else
		mmr = MMSYSERR_ERROR;
	if (mmr != 0)
		goto Done;
	pHdr = m_pFreeHead;
	while (pHdr != NULL)
	{
		mmr = midiOutPrepareHeader((HMIDIOUT)m_hStream, pHdr,
			sizeof(MIDIHDR));
		if (mmr != 0)
			goto Done;
		pHdr = pHdr->lpNext;
	}
	prop.dwTimeDiv = m_dwTimeDiv;
	pfnProperty = (LPFNMIDISTREAMPROPERTY)g_pfnMidiStreamProperty;
	prop.cbStruct = sizeof(prop);
	if (pfnProperty != NULL)
		mmr = pfnProperty(m_hStream, (LPBYTE)&prop,
			MIDIPROP_SET | MIDIPROP_TIMEDIV);
	else
		mmr = MMSYSERR_ERROR;
	if (mmr != 0)
		goto Done;
	pfn = (LPFNMIDISTREAMHANDLE)g_pfnMidiStreamPause;		// midiStreamPause
	if (pfn != NULL)
		mmr = pfn(m_hStream);
	else
		mmr = MMSYSERR_ERROR;
	if (mmr != 0)
		goto Done;
	pfnOut = (LPFNMIDISTREAMOUT)g_pfnMidiStreamOut;	// midiStreamOut
	while (m_pFreeHead != 0)
	{
		pHdr = m_pFreeHead;
		m_pFreeHead = pHdr->lpNext;
		if (pfnOut != NULL)
		{
			mmr = pfnOut(m_hStream, pHdr, sizeof(MIDIHDR));
			// re-read inside the arm: this is what makes VC hoist
			// the load into the loop preheader and reload only
			// after the call, as the original does
			pfnOut = (LPFNMIDISTREAMOUT)g_pfnMidiStreamOut;
		}
		else
			mmr = MMSYSERR_ERROR;
		if (mmr != 0)
			goto Done;
		m_dwQueued++;
	}
	m_pFreeTail = 0;
	pfn = (LPFNMIDISTREAMHANDLE)g_pfnMidiStreamRestart;		// midiStreamRestart
	if (pfn != NULL)
		mmr = pfn(m_hStream);
	else
		mmr = MMSYSERR_ERROR;
	if (mmr != 0)
		goto Done;
	bOk = 1;
Done:
	if (bOk == 0)
		CloseStream();
	return bOk;
}
/////////////////////////////////////////////////////////////////////////////
// WaveMix sound engine (WaveMix V 2.3, statically linked into hover.exe)

// WaveMix debug output: the message goes to OutputDebugStringA when its
// level does not exceed the g_dwDebugLevel verbosity threshold (a zero
// threshold silences all output).

// hover.exe:0x438820
// FUNCTION: HOVER 0x438820
void WaveMix_DebugOut(int nLevel, const char* pszMsg)
{
	if (g_dwDebugLevel != 0 && nLevel <= (int)g_dwDebugLevel)
		OutputDebugStringA(pszMsg);
}

// Sound mixing worker thread proc (AFX_THREADPROC). Waits on
// g_hSoundWakeEvent, then under g_csSoundQueue dequeues the pending sound
// objects: g_pSoundOnce is played once (SND_MEMORY), g_pSoundLoop is
// played looping (SND_MEMORY|SND_LOOP|SND_ASYNC); with neither queued the
// current sound is stopped (sndPlaySoundA(NULL, 0)). The queued objects
// have the sound data pointer at offset +4. g_bSoundThreadQuit signals the
// thread to exit after the current pass; g_hSoundAckEvent is set at the end
// of every pass.

// hover.exe:0x42D6A0
// FUNCTION: HOVER 0x42d6a0
UINT Hover_SoundThreadProc(LPVOID pParam)
{
	void* pOnce;
	void* pLoop;
	BOOL bDone = FALSE;

	do
	{
		WaitForSingleObject(g_hSoundWakeEvent, INFINITE);
		pOnce = NULL;
		EnterCriticalSection(&g_csSoundQueue);
		if (g_bSoundThreadQuit != 0)
		{
			bDone = TRUE;
			pLoop = NULL;
		}
		else
		{
			pOnce = g_pSoundOnce;
			pLoop = g_pSoundLoop;
		}
		g_pSoundOnce = NULL;
		LeaveCriticalSection(&g_csSoundQueue);
		ResetEvent(g_hSoundWakeEvent);
		if (pOnce != NULL && ((void**)pOnce)[1] != NULL)
			sndPlaySoundA((LPCSTR)((void**)pOnce)[1], SND_MEMORY);
		if (pLoop != NULL && ((void**)pLoop)[1] != NULL)
			sndPlaySoundA((LPCSTR)((void**)pLoop)[1],
				SND_MEMORY | SND_LOOP | SND_ASYNC);
		if (pOnce == NULL && pLoop == NULL)
			sndPlaySoundA(NULL, 0);
		SetEvent(g_hSoundAckEvent);
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
int __stdcall WaveMixOpenChannel(MIXSESSION* pEngine, int nChannel,
	int nMode)
{
	int i;
	WaveMix_DebugOut(2, "<WaveMixOpenChannel>");
	if (pEngine != NULL && pEngine->wMagic1 == 0x5432 &&
		pEngine->wMagic2 == 0x5432)
	{
		g_pCurSession = pEngine;
	}
	else
	{
		MessageBeep(0xFFFFFFFF);
		wsprintfA(g_szStringBuffer,
			"Invalid session handle 0x%04X passed to WaveMix API",
			pEngine);
		WaveMix_DebugOut(1, g_szStringBuffer);
		MessageBoxA(NULL, g_szStringBuffer, "WavMix32",
			MB_ICONEXCLAMATION);
		g_pCurSession = NULL;
	}
	if (g_pCurSession == NULL)
		return 5;
	if ((DWORD)nMode > 2)
		return 0xa;
	if (nMode == 0 && nChannel >= 0x10)
		return 0xb;
	if (nMode == 2 && (nChannel > 0x10 || nChannel < 1))
		return 0xb;
	if (nMode == 0)
	{
		if (g_pCurSession->apChannel[nChannel] != WMIX_CHANNEL_CLOSED)
			return 4;
		g_pCurSession->apChannel[nChannel] = NULL;
		g_pCurSession->awChannelVolume[nChannel * 2] =
			g_pCurSession->wDefaultVolumeL;
		g_pCurSession->awChannelVolume[nChannel * 2 + 1] =
			g_pCurSession->wDefaultVolumeR;
		g_pCurSession->dwOpenChannels++;
		return 0;
	}
	if (nMode == 1)
		nChannel = 0x10;
	else if (nMode != 2)
	{
		WaveMix_DebugOut(2,
			"WaveMixOpenChannel: Invalid flag was not trapped!!!");
		return 0xa;
	}
	for (i = nChannel - 1; i >= 0; i--)
	{
		if (g_pCurSession->apChannel[i] == WMIX_CHANNEL_CLOSED)
		{
			g_pCurSession->apChannel[i] = NULL;
			g_pCurSession->awChannelVolume[i * 2] =
				g_pCurSession->wDefaultVolumeL;
			g_pCurSession->awChannelVolume[i * 2 + 1] =
				g_pCurSession->wDefaultVolumeR;
			g_pCurSession->dwOpenChannels++;
		}
	}
	return 0;
}

// Writes the session's current WaveMix settings back to the ini file.
// The dwFlags bits select which entries are written (same bit
// assignments as WaveMixConfigure): 0x80 device, 0x01 channels, 0x02
// sampling rate, 0x04 wave blocks, 0x08 block length, 0x10 mixing
// function, 0x20 remix callback, 0x40 position source, 0x100 debug
// dialogs, 0x200 pause blocks. Called by WaveMixConfigure when its ini
// update parameter is set.

// hover.exe:0x43D100
// FUNCTION: HOVER 0x43d100
void WaveMix_SaveConfigSettings(DWORD dwFlags)
{
	if ((dwFlags & 0x80) != 0)
	{
		wsprintfA(g_szStringBuffer, "%d", g_pCurSession->dwDeviceID);
		WritePrivateProfileStringA("general", "WaveOutDevice",
			g_szStringBuffer, g_szIniFileName);
	}
	if ((dwFlags & 0x01) != 0)
	{
		wsprintfA(g_szStringBuffer, "%d",
			g_pCurSession->pcm.wf.nChannels);
		WritePrivateProfileStringA(g_pCurSession->szDevicePName, "Channels",
			g_szStringBuffer, g_szIniFileName);
	}
	if ((dwFlags & 0x02) != 0)
	{
		WORD wSamplesPerSec;

		wSamplesPerSec = (WORD)(
			g_pCurSession->pcm.wf.nSamplesPerSec / 11025);
		wSamplesPerSec = (WORD)(wSamplesPerSec * 11);
		wsprintfA(g_szStringBuffer, "%d", wSamplesPerSec);
		WritePrivateProfileStringA(g_pCurSession->szDevicePName,
			"SamplesPerSec", g_szStringBuffer, g_szIniFileName);
	}
	if ((dwFlags & 0x04) != 0)
	{
		wsprintfA(g_szStringBuffer, "%d", g_pCurSession->dwWaveBlockCount);
		WritePrivateProfileStringA(g_pCurSession->szDevicePName,
			"WaveBlocks", g_szStringBuffer, g_szIniFileName);
	}
	if ((dwFlags & 0x08) != 0)
	{
		wsprintfA(g_szStringBuffer, "%d",
			(WORD)g_pCurSession->dwWaveBlockLen);
		WritePrivateProfileStringA(g_pCurSession->szDevicePName,
			"WaveBlockLen", g_szStringBuffer, g_szIniFileName);
	}
	if ((dwFlags & 0x10) != 0)
	{
		wsprintfA(g_szStringBuffer, "%d",
			g_pCurSession->pfnMixit == (MIXITFN)WaveMix_CMixit);
		WritePrivateProfileStringA(g_pCurSession->szDevicePName, "CMixit",
			g_szStringBuffer, g_szIniFileName);
	}
	if ((dwFlags & 0x20) != 0)
	{
		wsprintfA(g_szStringBuffer, "%d",
			g_pCurSession->pfnRemix != WaveMix_ResetRemix ?
				2 : 1);
		WritePrivateProfileStringA(g_pCurSession->szDevicePName, "Remix",
			g_szStringBuffer, g_szIniFileName);
	}
	if ((dwFlags & 0x40) != 0)
	{
		wsprintfA(g_szStringBuffer, "%d",
			g_pCurSession->fGoodGetPos == 0);
		WritePrivateProfileStringA(g_pCurSession->szDevicePName,
			"GoodWavePos", g_szStringBuffer, g_szIniFileName);
	}
	if ((dwFlags & 0x100) != 0)
	{
		wsprintfA(g_szStringBuffer, "%d", g_bShowDebugDialogs >= 1);
		WritePrivateProfileStringA("general", "ShowDebugDialogs",
			g_szStringBuffer, g_szIniFileName);
	}
	if ((dwFlags & 0x200) != 0)
	{
		wsprintfA(g_szStringBuffer, "%d", g_pCurSession->dwPauseBlocks);
		WritePrivateProfileStringA(g_pCurSession->szDevicePName,
			"PauseBlocks", g_szStringBuffer, g_szIniFileName);
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
int WaveMix_ReadConfigSettings(void* pv)
{
	UINT nDevs;
	int nRemix;
	int nGoodWavePos;
	int nWaveBlocks;
	int nWaveBlockLen;
	int nSamplesPerSec;
	REMIXFN pfnRemix;
	BOOL bUnknown;

	nDevs = waveOutGetNumDevs();
	if (nDevs == 0)
		return 0;
	if ((((BYTE*)pv)[3] & 4) != 0 && WaveMix_ReadRegistryForAppSpecificConfigs(pv) != 0)
	{
		WaveMixConfigure(g_pCurSession, 0, pv, 0, 0);
		return 1;
	}
	if ((((BYTE*)pv)[2] & 0x80) != 0)
		g_pCurSession->dwDeviceID = *(WORD*)((BYTE*)pv + 0x14);
	else
		g_pCurSession->dwDeviceID = GetPrivateProfileIntA("general",
			"WaveOutDevice", 0, g_szIniFileName);
	if (g_pCurSession->dwDeviceID >= nDevs)
		g_pCurSession->dwDeviceID = 0;
	bUnknown = (waveOutGetDevCapsA(g_pCurSession->dwDeviceID,
			&g_pCurSession->woCaps, sizeof(WAVEOUTCAPSA)) != 0 ||
		WaveMix_RemoveInvalidIniNameCharacters(g_pCurSession->woCaps.szPname) == 0);
	if (bUnknown)
		lstrcpyA(g_pCurSession->woCaps.szPname, "Unkown Device");
	if (WaveMix_ReadRegistryToGetMachineSpecificInfSection(g_pCurSession->dwDeviceID, g_pCurSession->szDevicePName,
			sizeof(g_pCurSession->szDevicePName)) == 0)
	{
		lstrcpyA(g_pCurSession->szDevicePName, WaveMix_GetOperatingSystemPrefix());
		lstrcatA(g_pCurSession->szDevicePName, g_pCurSession->woCaps.szPname);
	}
	nRemix = GetPrivateProfileIntA("default", "Remix", 1,
		g_szIniFileName);
	nGoodWavePos = GetPrivateProfileIntA("default", "GoodWavePos", 0,
		g_szIniFileName);
	nWaveBlocks = GetPrivateProfileIntA("default", "WaveBlocks", 3,
		g_szIniFileName);
	nWaveBlockLen = GetPrivateProfileIntA("default", "WaveBlockLen", 0,
		g_szIniFileName) & 0xFFFC;
	nSamplesPerSec = GetPrivateProfileIntA("default", "SamplesPerSec", 0xB,
		g_szIniFileName);
	if ((g_pCurSession->woCaps.dwSupport & WAVECAPS_SYNC) != 0)
	{
		if (g_bShowDebugDialogs != 0)
		{
			wsprintfA(g_szStringBuffer,
				"%s is a syncronous (blocking) wave output device.  "
				"This will not permit audio to play while other "
				"applications are running.",
				g_pCurSession->woCaps.szPname);
			MessageBoxA(NULL, g_szStringBuffer, "WavMix32",
				MB_ICONINFORMATION);
		}
		return 0;
	}
	if (GetPrivateProfileIntA("not compatible", g_pCurSession->szDevicePName, 0,
			g_szIniFileName) != 0)
	{
		if (g_bShowDebugDialogs != 0)
		{
			wsprintfA(g_szStringBuffer,
				"%s is not compatible with the realtime wavemixer.",
				g_pCurSession->szDevicePName);
			MessageBoxA(NULL, g_szStringBuffer, "WavMix32",
				MB_ICONINFORMATION);
		}
		return 0;
	}
	nRemix = GetPrivateProfileIntA(g_pCurSession->szDevicePName, "Remix", nRemix,
		g_szIniFileName);
	pfnRemix = (nRemix == 2) ?
		(REMIXFN)WaveMix_NoResetRemix : WaveMix_ResetRemix;
	g_pCurSession->pfnRemix = pfnRemix;
	g_pCurSession->fGoodGetPos =
		(GetPrivateProfileIntA(g_pCurSession->szDevicePName, "GoodWavePos",
			nGoodWavePos, g_szIniFileName) >= 1);
	g_pCurSession->dwWaveBlockCount =
		GetPrivateProfileIntA(g_pCurSession->szDevicePName, "WaveBlocks",
			nWaveBlocks, g_szIniFileName);
	if ((int)g_pCurSession->dwWaveBlockCount < 2)
		g_pCurSession->dwWaveBlockCount = 2;
	else if ((int)g_pCurSession->dwWaveBlockCount > 0x14)
		g_pCurSession->dwWaveBlockCount = 0x14;
	g_pCurSession->dwPauseBlocks =
		GetPrivateProfileIntA(g_pCurSession->szDevicePName, "PauseBlocks",
			g_pCurSession->dwWaveBlockCount, g_szIniFileName);
	if ((int)g_pCurSession->dwPauseBlocks < 0)
		g_pCurSession->dwPauseBlocks = 0;
	else if ((int)g_pCurSession->dwPauseBlocks >
			(int)g_pCurSession->dwWaveBlockCount)
		g_pCurSession->dwPauseBlocks = g_pCurSession->dwWaveBlockCount;
	nSamplesPerSec = GetPrivateProfileIntA(g_pCurSession->szDevicePName,
		"SamplesPerSec", nSamplesPerSec, g_szIniFileName);
	g_pCurSession->pcm.wf.nChannels =
		(WORD)GetPrivateProfileIntA(g_pCurSession->szDevicePName, "Channels",
			g_wfxDefaultMix.wf.nChannels, g_szIniFileName);
	if (g_pCurSession->pcm.wf.nChannels < 1)
		g_pCurSession->pcm.wf.nChannels = 1;
	else if (g_pCurSession->pcm.wf.nChannels > 2)
		g_pCurSession->pcm.wf.nChannels = 2;
	switch (nSamplesPerSec)
	{
	case 22:
		g_pCurSession->pcm.wf.nSamplesPerSec = 22050;
		g_pCurSession->pcm.wf.nAvgBytesPerSec =
			22050 * g_pCurSession->pcm.wf.nChannels;
		break;
	case 44:
		g_pCurSession->pcm.wf.nSamplesPerSec = 44100;
		g_pCurSession->pcm.wf.nAvgBytesPerSec =
			44100 * g_pCurSession->pcm.wf.nChannels;
		break;
	default:
		g_pCurSession->pcm.wf.nAvgBytesPerSec =
			11025 * g_pCurSession->pcm.wf.nChannels;
		break;
	}
	g_pCurSession->dwWaveBlockLen = WaveMix_GetWaveBlockLen(nWaveBlockLen,
		(LPWAVEFORMATEX)&g_pCurSession->pcm);
	if (pv != NULL)
		WaveMixConfigure(g_pCurSession, 0, pv, 0, 0);
	return 1;
}

// WaveMixConfigureInit: creates and initializes a WaveMix mixing session
// (0x1C0 bytes, LocalAlloc LPTR). Copies the caller's MIXCONFIG
// (clamped to its 0x1e-byte size), resolves the "ShowDebugDialogs"
// setting (from the config block when flag bit 0x100 is set, otherwise
// from the ini file), and bails out with a message when no wave output
// device exists. Returns the session object, or NULL on failure.

// hover.exe:0x43D880
// FUNCTION: HOVER 0x43d880
MIXSESSION* __stdcall WaveMixConfigureInit(MIXCONFIG* pCfg)
{
	MIXCONFIG cfgLocal;
	int i;

	WaveMix_DebugOut(2, "<WaveMixConfigureInit>");
	memset(&cfgLocal, 0, sizeof(cfgLocal));
	cfgLocal.wSize = sizeof(cfgLocal);
	if (pCfg != NULL)
	{
		WORD cb = pCfg->wSize;
		if (cb >= sizeof(cfgLocal))
			cb = sizeof(cfgLocal);
		memcpy(&cfgLocal, pCfg, cb);
	}
	if (g_bWaveMixInitialized == 0)
	{
		if (WaveMix_Startup(GetModuleHandleA(NULL)) == 0)
			return NULL;			// module init failed
	}
	if ((cfgLocal.dwFlags & 0x100) != 0)
	{
		g_bShowDebugDialogs = (cfgLocal.wShowDebugDialogs != 0);
	}
	else
	{
		g_bShowDebugDialogs = (GetPrivateProfileIntA("general",
			"ShowDebugDialogs", 0, g_szIniFileName) >= 1);
	}
	if (waveOutGetNumDevs() == 0)
	{
		if (g_bShowDebugDialogs != 0)
		{
			wsprintfA(g_szStringBuffer,
				"This system does not have a valid wave output device.");
			MessageBoxA(NULL, g_szStringBuffer, "WavMix32",
				MB_ICONINFORMATION);
		}
		return NULL;
	}
	if (GetPrivateProfileIntA("general", "ShowDevices", 0,
			g_szIniFileName) != 0)
		WaveMix_ShowWaveOutDevices();			// device list
	g_pCurSession = (MIXSESSION*)LocalAlloc(LPTR, sizeof(MIXSESSION));
	if (g_pCurSession == NULL)
		return NULL;
	g_pCurSession->wMagic2 = 0x5432;
	g_pCurSession->wMagic1 = g_pCurSession->wMagic2;
	g_pCurSession->ppWaveBlocks = NULL;
	g_pCurSession->fSettingsDialogActive = 0;
	g_pCurSession->wDefaultVolumeL = 0xa;
	g_pCurSession->wDefaultVolumeR = 0xa;
	g_pCurSession->pfnMixit = g_pfnDefaultMixit;
	for (i = 0; i < 0x10; i++)
		g_pCurSession->apChannel[i] = WMIX_CHANNEL_CLOSED;
	memmove(&g_pCurSession->pcm, &g_wfxDefaultMix, sizeof(PCMWAVEFORMAT));
	if (WaveMix_ReadConfigSettings(&cfgLocal) == 0)
	{
		g_pCurSession->wMagic2 = 0;		// driver setup failed
		g_pCurSession->wMagic1 = g_pCurSession->wMagic2;
		LocalFree(g_pCurSession);
		g_pCurSession = NULL;
	}
	return g_pCurSession;
}

// WaveMixCloseChannel: validates the session (same magic-word check and
// error reporting as WaveMixOpenChannel), flushes the channel(s) via
// WaveMixFlushChannel with flag bit 2 set, then frees them: flag bit 1
// closes all 16 channels, otherwise only channel nChannel. Returns 0 on
// success, 5 = invalid session, or WaveMixFlushChannel's error code.

// hover.exe:0x43A760
// FUNCTION: HOVER 0x43a760
int __stdcall WaveMixCloseChannel(MIXSESSION* pEngine, int nChannel,
	int nFlags)
{
	int nResult;
	int i;
	int nEnd;

	WaveMix_DebugOut(2, "<WaveMixCloseChannel>");
	if (pEngine != NULL && pEngine->wMagic1 == 0x5432 &&
		pEngine->wMagic2 == 0x5432)
	{
		g_pCurSession = pEngine;
	}
	else
	{
		MessageBeep(0xFFFFFFFF);
		wsprintfA(g_szStringBuffer,
			"Invalid session handle 0x%04X passed to WaveMix API",
			pEngine);
		WaveMix_DebugOut(1, g_szStringBuffer);
		MessageBoxA(NULL, g_szStringBuffer, "WavMix32",
			MB_ICONEXCLAMATION);
		g_pCurSession = NULL;
	}
	if (g_pCurSession == NULL)
		return 5;
	nResult = WaveMixFlushChannel(pEngine, nChannel, nFlags | 2);
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
		if (g_pCurSession->apChannel[i] != WMIX_CHANNEL_CLOSED)
		{
			g_pCurSession->apChannel[i] = WMIX_CHANNEL_CLOSED;
			g_pCurSession->dwOpenChannels--;
		}
	}
	return 0;
}

// WaveMixCloseSession: validates the session (same magic-word check and
// error reporting as WaveMixOpenChannel), deactivates it and closes all of
// its channels, zeroes the 0x1C0-byte session object and LocalFree()s
// it. Returns 0 on success, 5 on failure.

// hover.exe:0x43DAB0
// FUNCTION: HOVER 0x43dab0
int __stdcall WaveMixCloseSession(MIXSESSION* pEngine)
{
	WaveMix_DebugOut(2, "<WaveMixCloseSession>");
	if (pEngine != NULL && pEngine->wMagic1 == 0x5432 &&
		pEngine->wMagic2 == 0x5432)
	{
		g_pCurSession = pEngine;
	}
	else
	{
		MessageBeep(0xFFFFFFFF);
		wsprintfA(g_szStringBuffer,
			"Invalid session handle 0x%04X passed to WaveMix API",
			pEngine);
		WaveMix_DebugOut(1, g_szStringBuffer);
		MessageBoxA(NULL, g_szStringBuffer, "WavMix32",
			MB_ICONEXCLAMATION);
		g_pCurSession = NULL;
	}
	if (g_pCurSession == NULL)
		return 5;
	WaveMixActivate(pEngine, 0);			// session deactivate
	WaveMixCloseChannel(pEngine, 0, 1);
	memset(g_pCurSession, 0, sizeof(MIXSESSION));
	g_pCurSession = NULL;
	if (pEngine != NULL && LocalFree(pEngine) == NULL)
		return 0;
	return 5;
}

// WaveMixActivate: validates the session (same magic-word check and
// error reporting as WaveMixOpenChannel), then activates or inactivates
// it. Activating claims the global active session, stops any playing
// sound, initializes the wave driver and queues empty wave blocks until
// the queue is full. Inactivating stops the mixer and releases the
// active session. Returns 0 on success, 4 when another session is
// active, 0xc when the driver is in use and 5 on an invalid session.

// hover.exe:0x439180
// FUNCTION: HOVER 0x439180
int __stdcall WaveMixActivate(MIXSESSION* pEngine, int bActivate)
{
	XWAVEHDR** ppBlocks;
	XWAVEHDR* pXWH;
	int nCount;
	int i;
	int nErr;

	WaveMix_DebugOut(2, "<WaveMixActivate>");
	if (pEngine != NULL && pEngine->wMagic1 == 0x5432 &&
		pEngine->wMagic2 == 0x5432)
	{
		g_pCurSession = pEngine;
	}
	else
	{
		MessageBeep(0xFFFFFFFF);
		wsprintfA(g_szStringBuffer,
			"Invalid session handle 0x%04X passed to WaveMix API",
			pEngine);
		WaveMix_DebugOut(1, g_szStringBuffer);
		MessageBoxA(NULL, g_szStringBuffer, "WavMix32",
			MB_ICONEXCLAMATION);
		g_pCurSession = NULL;
	}
	if (g_pCurSession == NULL)
		return 5;
	if (bActivate)
	{
		WaveMix_DebugOut(2, "Activate");
		if (g_pActiveSession != NULL)
			return (g_pActiveSession == g_pCurSession) ? 0 : 4;
		if (g_pCurSession->fSettingsDialogActive != 0)
			return 0xc;
		g_pActiveSession = g_pCurSession;
		sndPlaySoundA(NULL, 0);
		nErr = WaveMix_OpenWaveOut();			// driver init
		if (nErr != 0)
		{
			g_pActiveSession = NULL;
			return nErr;
		}
		g_pCurSession->fActive = 1;
		WaveMix_SetWaveOutPosition(g_pCurSession->dwCurrentSample);
		for (;;)
		{
			// The scan walks the block pointer (add eax,4) and keeps
			// the "i < nCount" guard on the counter.
			i = 0;
			nCount = g_pCurSession->dwWaveBlockCount;
			ppBlocks = g_pCurSession->ppWaveBlocks;
			while (i < nCount && (*ppBlocks)->fAvailable == 0)
			{
				ppBlocks++;
				i++;
			}
			if (i >= nCount)
			{
				WaveMix_DebugOut(2, "{gwb:out of wav blks}");
				pXWH = NULL;
			}
			else
			{
				pXWH = *ppBlocks;
				pXWH->wh.dwBufferLength = g_pCurSession->dwWaveBlockLen;
				pXWH->wh.lpData = (LPSTR)pXWH->abData;
				pXWH->pSession = g_pActiveSession;
				pXWH->fAvailable = 0;
			}
			nErr = WaveMix_MixerPlay(pXWH, 1);	// queue block
			if (nErr == 0)
				break;
		}
		return 0;
	}
	WaveMix_DebugOut(2, "Inactivate");
	if (g_pCurSession->fActive != 0)
		g_pCurSession->dwCurrentSample = WaveMix_GetWaveOutPosition(
			g_pCurSession->hWaveOut, g_pCurSession->fGoodGetPos);
	WaveMix_FreeWaveOut(g_pCurSession);
	g_pCurSession->fActive = 0;
	if (g_pActiveSession == g_pCurSession)
		g_pActiveSession = NULL;
	return 0;
}

// WaveMixFlushChannel: validates the session (same magic-word check and
// error reporting as WaveMixOpenChannel), then returns the queued channel
// nodes of one channel - or of every channel when bit 0 of nFlags is
// set - to the free list. Unless bit 1 of nFlags is set, an active
// session then reports the new position through the session callback.
// Returns 0 on success, 0xb on an invalid channel and 5 on an invalid
// session or unopened channel.

// hover.exe:0x43A5A0
// FUNCTION: HOVER 0x43a5a0
int __stdcall WaveMixFlushChannel(MIXSESSION* pEngine, int nChannel,
	int nFlags)
{
	BOOL bFreed;
	CHANNELNODE* pNode;
	CHANNELNODE* pNext;
	int i;
	int nEnd;

	bFreed = FALSE;
	WaveMix_DebugOut(2, "F");
	if (pEngine != NULL && pEngine->wMagic1 == 0x5432 &&
		pEngine->wMagic2 == 0x5432)
	{
		g_pCurSession = pEngine;
	}
	else
	{
		MessageBeep(0xFFFFFFFF);
		wsprintfA(g_szStringBuffer,
			"Invalid session handle 0x%04X passed to WaveMix API",
			pEngine);
		WaveMix_DebugOut(1, g_szStringBuffer);
		MessageBoxA(NULL, g_szStringBuffer, "WavMix32",
			MB_ICONEXCLAMATION);
		g_pCurSession = NULL;
	}
	if (g_pCurSession == NULL)
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
		if (g_pCurSession->apChannel[i] == WMIX_CHANNEL_CLOSED)
			return 5;
		nEnd = i + 1;
	}
	for (; i < nEnd; i++)
	{
		if (g_pCurSession->apChannel[i] == WMIX_CHANNEL_CLOSED)
			continue;
		pNode = g_pCurSession->apChannel[i];
		g_pCurSession->apChannel[i] = NULL;
		if (pNode == NULL)
			continue;
		bFreed = TRUE;
		for (;;)
		{
			pNext = pNode->pNext;
			WaveMix_DebugOut(2, "{FreeCNode");
			if (pNode == NULL)	// never taken; pNode was
			{				// checked above
				WaveMix_DebugOut(1,
					" FreeCN: NULL parameter!}");
			}
			else
			{
				pNode->pNext = g_pFreeChannelNodes;
				g_pFreeChannelNodes = pNode;
				WaveMix_DebugOut(2, "}");
			}
			pNode = pNext;
			if (pNext == NULL)
				break;
		}
	}
	if (bFreed && (nFlags & 2) == 0 && g_pCurSession->fActive != 0)
	{
		// The queued position is computed inside the argument list: the
		// original pushes the 0 first, then evaluates
		// WaveMix_GetWaveOutPosition.
		g_pCurSession->pfnRemix(
			WaveMix_GetWaveOutPosition(g_pCurSession->hWaveOut, g_pCurSession->fGoodGetPos),
			0);
	}
	return 0;
}

// WaveMixShowDevices: debug helper that pops up a message box with the
// number of waveOut devices on the system, then one message box per
// device with its name and driver version (or a failure notice).

// hover.exe:0x43B8F0
// FUNCTION: HOVER 0x43b8f0
void WaveMix_ShowWaveOutDevices()
{
	WAVEOUTCAPSA caps;
	int i;
	int numDevs;
	int nErr;

	numDevs = waveOutGetNumDevs();
	if (numDevs == 0)
		return;
	wsprintfA(g_szStringBuffer,
		"%d waveOut Devices have been detected on your system.",
		numDevs);
	MessageBoxA(NULL, g_szStringBuffer, "WavMix32", MB_ICONINFORMATION);
	for (i = 0; i < numDevs; i++)
	{
		if (waveOutGetDevCapsA(i, &caps, sizeof(caps)) != 0 ||
			WaveMix_RemoveInvalidIniNameCharacters(caps.szPname) == 0)	// the
		{					// original reports
			nErr = 1;			// "failed (err 1)"
		}					// on this path
		else
			nErr = 0;
		if (nErr != 0)
			wsprintfA(g_szStringBuffer,
				"waveOutGetDevCaps failed (err %u) for device %d",
				nErr, i);
		else
			wsprintfA(g_szStringBuffer,
				"Device %i: %s\n\tVersion %u.%u",
				i, caps.szPname, HIBYTE(caps.vDriverVersion),
				LOBYTE(caps.vDriverVersion));
		MessageBoxA(NULL, g_szStringBuffer, "WavMix32",
			MB_ICONINFORMATION);
	}
}

// WaveMix position query: when dwType is nonzero the position comes
// from waveOutGetPosition(), adjusted through the Add/Sub function
// selected by WaveMix_SetWaveOutPosition; otherwise it is estimated from the
// timeGetTime() milliseconds elapsed since the origin at +0x1A0,
// converted to bytes and rounded down to a multiple of 8.

// hover.exe:0x438840
// FUNCTION: HOVER 0x438840
int WaveMix_GetWaveOutPosition(HWAVEOUT hWaveOut, DWORD fGoodGetPos)
{
	MMTIME mmt;

	if (fGoodGetPos != 0)
	{
		mmt.wType = TIME_BYTES;
		waveOutGetPosition(hWaveOut, &mmt, sizeof(mmt));
		return g_pCurSession->pfnPosAdjust(mmt.u.cb,
			g_pCurSession->dwWaveOutPosDelta);
	}
	WaveMix_DebugOut(3, "{mywaveoutgetpos !fGoodGetPos}");
	return (timeGetTime() - g_pCurSession->dwBaseTime) *
		g_pCurSession->pcm.wf.nAvgBytesPerSec / 1000 & 0xFFFFFFF8;
}

// Position adjustment helpers; WaveMix_SetWaveOutPosition points the +0x1B4
// function pointer at whichever one matches the adjustment direction.

// hover.exe:0x4388C0
// FUNCTION: HOVER 0x4388c0
int WaveMix_AddFactor(int nA, int nB)
{
	WaveMix_DebugOut(2, "Add");
	return nA + nB;
}

// hover.exe:0x4388E0
// FUNCTION: HOVER 0x4388e0
int WaveMix_SubFactor(int nA, int nB)
{
	WaveMix_DebugOut(2, "Sub");
	return nA - nB;
}

// SetWaveOutPositon (sic): re-bases the position tracking onto dwPos.
// The delta between the device's current position and dwPos is stored
// at +0x1A8 with the matching Add/Sub adjustment function at +0x1B4,
// and the timeGetTime() origin at +0x1A0 is moved back by the dwPos
// equivalent in milliseconds.

// hover.exe:0x438900
// FUNCTION: HOVER 0x438900
void WaveMix_SetWaveOutPosition(DWORD dwPos)
{
	MMTIME mmt;

	WaveMix_DebugOut(2, "SetWaveOutPositon");
	wsprintfA(g_szStringBuffer, "=%lu", dwPos);
	WaveMix_DebugOut(3, g_szStringBuffer);
	mmt.wType = TIME_BYTES;
	if (g_pCurSession->hWaveOut != 0)
		waveOutGetPosition(g_pCurSession->hWaveOut, &mmt,
			sizeof(mmt));
	else
		mmt.u.cb = 0;
	if (mmt.u.cb >= dwPos)
	{
		g_pCurSession->dwWaveOutPosDelta = mmt.u.cb - dwPos;
		g_pCurSession->pfnPosAdjust = WaveMix_SubFactor;
	}
	else
	{
		g_pCurSession->dwWaveOutPosDelta = dwPos - mmt.u.cb;
		g_pCurSession->pfnPosAdjust = WaveMix_AddFactor;
	}
	g_pCurSession->dwBaseTime = timeGetTime() - dwPos * 1000 /
		g_pCurSession->pcm.wf.nAvgBytesPerSec;
	g_pCurSession->dwCurrentSample = dwPos;
}

// WaveMix "cmixit" mixing function (assigned to the session's pfnMixit
// by WaveMix_Startup and by WaveMixConfigure when the config's Remix
// field selects it). Mixes nSrcs 8-bit sources into pBuf for nCount
// output bytes, applying the per-channel volume pairs in pwVols through
// the g_abVolumeTable table. Stereo output (session format nChannels == 2)
// consumes/produces two bytes per sample frame; with a single source the
// table output is used directly, with several sources the centered
// (0x80-based) values are summed and clamped to 0..0xFF.

// hover.exe:0x4389F0
// FUNCTION: HOVER 0x4389f0
void WaveMix_CMixit(BYTE* pBuf, DWORD** ppSrcs, WORD* pwVols, int nSrcs,
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

	if (g_pCurSession->pcm.wf.nChannels == 2)
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
					*pBuf++ = g_abVolumeTable[wVolL][*pSrc++];
					*pBuf++ = g_abVolumeTable[wVolR][*pSrc++];
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
					nAccL += g_abVolumeTable[pwVol[0]]
						[pSrc[0]] - 0x80;
					nAccR += g_abVolumeTable[pwVol[1]]
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
				*pBuf++ = g_abVolumeTable[wVol][*pSrc++];
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
				nAccL += g_abVolumeTable[wVol][pSrc[wOff]] - 0x80;
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

// Links the g_aChannelNodePool channel-node pool into a singly-linked free
// list through each node's pNext, then points the free-list head at the
// pool. Note that it stops one node early: node 98's link is left at its
// zero initial value and node 99 is never reached, so only 99 nodes are
// ever handed out. Called once by WaveMix_Startup.

// hover.exe:0x438CA0
// FUNCTION: HOVER 0x438ca0
void WaveMix_InitChannelNodes()
{
	CHANNELNODE* pNext;
	CHANNELNODE* pNode;

	pNode = g_aChannelNodePool;
	do
	{
		pNext = pNode + 1;
		pNode->pNext = pNext;
		pNode = pNext;
	} while (pNode < &g_aChannelNodePool[98]);
	g_aChannelNodePool[99].pNext = NULL;
	g_pFreeChannelNodes = g_aChannelNodePool;
}

// WaveMix output shutdown: resets the device, re-bases the position
// tracking, frees every queued done-block, unprepares both wave block
// arrays and closes the device and its callback window.

// hover.exe:0x438F50
// FUNCTION: HOVER 0x438f50
void WaveMix_FreeWaveOut(MIXSESSION* pEngine)
{
	DWORD dwPos;
	HWAVEOUT hWaveOut;

	if (pEngine->fActive == 0)
		return;
	hWaveOut = pEngine->hWaveOut;
	if (hWaveOut == 0)
		return;
	dwPos = WaveMix_GetWaveOutPosition(hWaveOut, g_pCurSession->fGoodGetPos);
	waveOutReset(hWaveOut);
	WaveMix_SetWaveOutPosition(dwPos);
	if (g_pPlayQueueHead != NULL)
	{
		do
		{
			g_pPlayQueueHead->fAvailable = 1;
			WaveMix_RemoveFromPlayingQueue(g_pPlayQueueHead);	// free block
		} while (g_pPlayQueueHead != NULL);
	}
	WaveMix_FreeWaveBlocks(pEngine->hWaveOut, g_apWaveBlocksA);
	WaveMix_FreeWaveBlocks(pEngine->hWaveOut, g_apWaveBlocksB);
	waveOutClose(pEngine->hWaveOut);
	pEngine->hWaveOut = 0;
	DestroyWindow(pEngine->hWndCallback);
	pEngine->hWndCallback = NULL;
}

// WaveMix channel sweeper ("{F..._got_"): walks every open channel's
// node queue and frees the nodes whose end position has been reached,
// posting MM_WOM_DONE to the node's notification window when it has one.
// When position tracking is not device-based and the done-block queue is
// empty, it also posts the private 0x400 message to the callback window
// for every open channel. Called by the WaveMixPump pump.

// hover.exe:0x439900
// FUNCTION: HOVER 0x439900
void WaveMix_FreePlayedChannelNodes()
{
	CHANNELNODE* pNode;
	int i;
	DWORD dwPos;

	WaveMix_DebugOut(2, "{F");
	dwPos = WaveMix_GetWaveOutPosition(g_pCurSession->hWaveOut,
		g_pCurSession->fGoodGetPos);
	i = 0;
	WaveMix_DebugOut(2, "_got_");
	for (; i < 0x10; i++)
	{
		pNode = g_pCurSession->apChannel[i];
		if (pNode == NULL || pNode == WMIX_CHANNEL_CLOSED)
			continue;
		WaveMix_DebugOut(2, ".");
		while (pNode != NULL)
		{
			if (pNode->dwEndPos > dwPos)
				break;
			WaveMix_DebugOut(2, "<!");
			g_pCurSession->apChannel[i] = pNode->pNext;
			if (pNode->PlayParams.hwndNotify != NULL)
				PostMessageA(pNode->PlayParams.hwndNotify,
					MM_WOM_DONE, i, (LPARAM)pNode->lpMixWave);
			WaveMix_DebugOut(2, "{FreeCNode");
			if (pNode == NULL)	// never taken; pNode was
			{				// checked above
				WaveMix_DebugOut(1,
					" FreeCN: NULL parameter!}");
			}
			else
			{
				pNode->pNext = g_pFreeChannelNodes;
				g_pFreeChannelNodes = pNode;
				WaveMix_DebugOut(2, "}");
			}
			pNode = g_pCurSession->apChannel[i];
			WaveMix_DebugOut(2, ">");
		}
	}
	if (g_pCurSession->fGoodGetPos == 0 && g_pPlayQueueHead == NULL)
	{
		for (i = 0; i < 0x10; i++)
		{
			if (g_pCurSession->apChannel[i] != NULL &&
				g_pCurSession->apChannel[i] != WMIX_CHANNEL_CLOSED)
			{
				PostMessageA(g_pCurSession->hWndCallback, 0x400, 0,
					(LPARAM)g_pCurSession);
			}
		}
	}
	WaveMix_DebugOut(2, "}");
}

// WaveMix output initialization: creates the hidden callback window,
// opens the waveOut device with window callbacks and prepares both
// wave block pointer arrays. Returns 0 on success, the waveOutOpen()
// error code, or 1 on window/preparation failure.

// hover.exe:0x439010
// FUNCTION: HOVER 0x439010
int WaveMix_OpenWaveOut()
{
	int nErr;

	if (g_pCurSession->hWaveOut != 0)
		goto success;
	g_pCurSession->hWndCallback = CreateWindowExA(0, "WavMix32", "",
		WS_DISABLED, 0, 0, 0, 0, NULL, NULL, g_hWaveMixInst, NULL);
	if (g_pCurSession->hWndCallback == NULL)
	{
		if (g_bShowDebugDialogs != 0)
			MessageBoxA(NULL, "Failed to create callback window.",
				"WavMix32", MB_ICONEXCLAMATION);
		return 1;
	}
	nErr = waveOutOpen(&g_pCurSession->hWaveOut,
		g_pCurSession->dwDeviceID,
		(LPCWAVEFORMATEX)&g_pCurSession->pcm,
		(DWORD)g_pCurSession->hWndCallback, 0, CALLBACK_WINDOW);
	if (nErr != 0)
	{
		DestroyWindow(g_pCurSession->hWndCallback);
		g_pCurSession->hWndCallback = NULL;
		return nErr;
	}
	if (WaveMix_AllocWaveBlocks(g_pCurSession->hWaveOut,	// prepare
		g_apWaveBlocksA) == 0)
		goto fail;
	if (WaveMix_AllocWaveBlocks(g_pCurSession->hWaveOut,	// prepare
		g_apWaveBlocksB) == 0)
		goto fail;
	g_pCurSession->ppWaveBlocks = g_apWaveBlocksA;
success:
	return 0;
fail:
	WaveMix_FreeWaveBlocks(g_pCurSession->hWaveOut, g_apWaveBlocksA);
	WaveMix_FreeWaveBlocks(g_pCurSession->hWaveOut, g_apWaveBlocksB);
	waveOutClose(g_pCurSession->hWaveOut);
	g_pCurSession->hWaveOut = 0;
	DestroyWindow(g_pCurSession->hWndCallback);
	return 1;
}

// WaveMix mixer pump ("{MP}"): fills the wave block's buffer with up to
// +0x18C bytes - silence (0x80) where no channel has data, mixed channel data
// through the +0x1AC mixing function elsewhere - then submits the block with
// waveOutWrite(). When bQueue is set the block is also linked onto the
// done-block queue; when the block cannot be submitted its +0x20 in-use flag
// is set and it is freed through WaveMix_RemoveFromPlayingQueue. Returns 1
// when the block was submitted, 0 when there was nothing to mix.

// hover.exe:0x439560
// FUNCTION: HOVER 0x439560
int WaveMix_MixerPlay(XWAVEHDR* pXWH, int bQueue)
{
	UINT i;			// unsigned: the channel loop ends on an unsigned jb
	CHANNELNODE** ppTab;
	CHANNELNODE** ppEnd;
	DWORD** ppSrc;
	CHANNELNODE* pChNode;
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

	WaveMix_DebugOut(2, "{MP}");
	if (pXWH == NULL)
	{
		WaveMix_DebugOut(2, "{lpXWH=NULL:return F}");
		return 0;
	}
	dwMin = 0xFFFFFFFF;
	nCount = 0;
	ppTab = g_apMixChannelNodes;
	for (i = 0; i < 0x10; i++)
	{
		pChNode = g_pCurSession->apChannel[i];
		if (pChNode == WMIX_CHANNEL_CLOSED || pChNode == NULL)
			continue;
		for (;;)
		{
			if (g_pCurSession->dwCurrentSample < pChNode->dwEndPos)
				break;
			WaveMix_DebugOut(3, "{MixerPlay:skipping}");
			pChNode = pChNode->pNext;
			if (pChNode == NULL)
				break;
		}
		if (pChNode == NULL)
			continue;
		if (dwMin > pChNode->dwStartPos)
			dwMin = pChNode->dwStartPos;
		*ppTab = pChNode;
		ppTab++;
		nCount++;
	}
	if (nCount == 0)
	{
		WaveMix_DebugOut(2, "{MP:uMaxChannel==0,ret F}");
		if (bQueue != 0)
			pXWH->fAvailable = 1;
		return 0;
	}
	pBuf = (BYTE*)pXWH->wh.lpData;
	dwPos = g_pCurSession->dwCurrentSample;
	wRemain = (WORD)g_pCurSession->dwWaveBlockLen;
	if (wRemain != 0)
	{
		do
		{
			WaveMix_DebugOut(2, "+");
			if (dwMin > dwPos)
			{
				// silence up to the next node start
				dwMixLen = wRemain;
				if (dwPos + dwMixLen >= dwMin)
					dwMixLen = dwMin - dwPos;
				// hover.exe truncates the length to WORD here (movzx
				// edx,bx) even though dwMixLen is a DWORD everywhere
				// else in this function - explicit narrow-then-widen
				// cast reproduces it; the value never exceeds 0xffff
				// (dwMixLen starts as wRemain, a WORD, and is only ever
				// reduced from there), so this is not a behaviour change.
				memset(pBuf, 0x80, (WORD)dwMixLen);
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
					pwVol = g_awMixVolumes;
					ppSrc = (DWORD**)g_apMixSources;
					ppTab = g_apMixChannelNodes;
					i = nCount;
					do
					{
						pChNode = *ppTab;
						if (dwPos < pChNode->dwStartPos)
						{
							if (dwEnd > pChNode->dwStartPos)
								dwEnd = pChNode->dwStartPos;
						}
						else
						{
							if (dwEnd > pChNode->dwEndPos)
								dwEnd = pChNode->dwEndPos;
							dwOff = dwPos - pChNode->dwStartPos;
							if (pChNode->PlayParams.wLoops != 0)
							{
								dwLoopLen = pChNode->dwNumSamples;
								dwOff %= dwLoopLen;
								if (dwEnd > dwLoopLen -
									dwOff + dwPos)
									dwEnd = dwLoopLen -
										dwOff + dwPos;
							}
							nSrcs++;
							*ppSrc = (DWORD*)(pChNode->lpPos +
								dwOff);
							ppSrc++;
							pwVol[0] =
								pChNode->wVolumeL;
							pwVol[1] =
								pChNode->wVolumeR;
							pwVol += 2;
						}
						ppTab++;
						i--;
					} while (i != 0);
				}
				if (nSrcs == 0)
				{
					WaveMix_DebugOut(2,
						"***No wave data has been setup!!!!");
				}
				else
				{
					dwMixLen = dwEnd - dwPos;
					g_pCurSession->pfnMixit(pBuf,
						(DWORD**)g_apMixSources,
						g_awMixVolumes, nSrcs, dwMixLen);
					dwPos += dwMixLen;
					wRemain -= (WORD)dwMixLen;
					pBuf += dwMixLen;
					dwMin = 0xFFFFFFFF;
					i = 0;
					if (nCount != 0)
					{
						ppTab = g_apMixChannelNodes;
						ppEnd = g_apMixChannelNodes + nCount;
						do
						{
							for (;;)
							{
								if (*ppTab == NULL)
									break;
								pChNode = *ppTab;
								if (pChNode->dwEndPos > dwPos)
									break;
								pChNode = pChNode->pNext;
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
									g_apMixChannelNodes)
									break;
							}
							else
							{
								if (dwMin >
									(*ppTab)->dwStartPos)
									dwMin =
										(*ppTab)->dwStartPos;
								ppTab++;
								i++;
							}
						} while (i < (UINT)nCount);
					}
				}
			}
		} while (wRemain != 0);
	}
	WaveMix_DebugOut(2, "mixing done");
	pXWH->dwWavePos = g_pCurSession->dwCurrentSample;
	g_pCurSession->dwCurrentSample += g_pCurSession->dwWaveBlockLen;
	if (bQueue != 0)
	{
		pXWH->pQNext = NULL;
		if (g_pPlayQueueHead == NULL)
		{
			g_pPlayQueueTail = pXWH;
			g_pPlayQueueHead = pXWH;
		}
		else
		{
			g_pPlayQueueTail->pQNext = pXWH;
			g_pPlayQueueTail = pXWH;
		}
		if (waveOutWrite(g_pCurSession->hWaveOut,
			&pXWH->wh, sizeof(WAVEHDR)) != 0)
		{
			if (g_bShowDebugDialogs != 0)
				MessageBoxA(NULL, "Failed to write block to device",
					"WavMix32", MB_ICONEXCLAMATION);
			pXWH->fAvailable = 1;
			WaveMix_RemoveFromPlayingQueue(pXWH);		// free block
		}
	}
	WaveMix_DebugOut(2, "{end MP: TRUE}");
	return 1;
}

// WaveMix device-name filter used by WaveMix_ShowWaveOutDevices: truncates the
// name in place at the first character that is neither alphanumeric
// nor whitespace, trims trailing whitespace and returns the remaining
// length.

// hover.exe:0x43B880
// FUNCTION: HOVER 0x43b880
int WaveMix_RemoveInvalidIniNameCharacters(char* pszName)
{
	char* p;

	p = pszName;
	if (pszName == NULL || *pszName == '\0')
		return 0;
	do
	{
		// Parenthesized to call the real CRT isalnum/isspace functions
		// (hover.exe:0x43fe99/0x43fe6e) instead of the _MBCS ctype.h
		// macros, which inline an extra ___mb_cur_max/__pctype/__isctype
		// runtime check the original does not have.
		if (!(isalnum)(*p) && !(isspace)(*p))
			break;
		p++;
	} while (*p != '\0');
	*p = '\0';
	p--;
	while (p >= pszName && (isspace)(*p))
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
XWAVEHDR* WaveMix_RemoveFromPlayingQueue(XWAVEHDR* pXWH)
{
	XWAVEHDR* pPrev;
	XWAVEHDR* pCur;

	WaveMix_DebugOut(2, "{RemoveFPQ");
	if (g_pPlayQueueHead == NULL)
		return NULL;
	if (pXWH == g_pPlayQueueHead)
	{
		WaveMix_DebugOut(2, "h");
		g_pPlayQueueHead = pXWH->pQNext;
		if (g_pPlayQueueHead == NULL)
			g_pPlayQueueTail = NULL;
	}
	else
	{
		WaveMix_DebugOut(2, ".");
		pPrev = g_pPlayQueueHead;
		pCur = pPrev->pQNext;
		if (pCur != NULL)
		{
			do
			{
				if (pXWH == pCur)
					break;
				pPrev = pCur;
				pCur = pCur->pQNext;
			} while (pCur != NULL);
		}
		if (pCur == NULL)
		{
			WaveMix_DebugOut(1,
				"RemoveFromPlayingQueue: lpXWH is not in Queue!\n\r");
			return NULL;
		}
		pPrev->pQNext = pCur->pQNext;
		if (g_pPlayQueueTail == pCur)
			g_pPlayQueueTail = pPrev;
	}
	WaveMix_DebugOut(2, "}");
	pXWH->pQNext = NULL;
	return pXWH;
}

// Allocates and prepares the 20 wave blocks of one pointer array: each
// block is a 0x2030-byte global allocation whose WAVEHDR points at its
// own +0x30 data area (+0x18C bytes). On failure everything allocated
// so far is released. Returns 1 on success, 0 on failure.

// hover.exe:0x438D60
// FUNCTION: HOVER 0x438d60
int WaveMix_AllocWaveBlocks(HWAVEOUT hWaveOut, XWAVEHDR** ppBlocks)
{
	XWAVEHDR** pp;
	XWAVEHDR* pBlock;
	HGLOBAL hMem;
	int i;
	int j;

	i = 0;
	pp = ppBlocks;
	do
	{
		hMem = GlobalAlloc(GMEM_DDESHARE, sizeof(XWAVEHDR));
		pBlock = (XWAVEHDR*)GlobalLock(hMem);
		*pp = pBlock;
		if (pBlock == NULL)
			goto alloc_fail;
		pBlock->wh.lpData = (LPSTR)pBlock->abData;
		pBlock->wh.dwBufferLength = g_pCurSession->dwWaveBlockLen;
		pBlock->wh.dwFlags = 0;
		pBlock->wh.dwLoops = 0;
		pBlock->fAvailable = 1;
		pBlock->dwWavePos = 0;
		pp++;
		i++;
	} while (i < 0x14);
	j = 0;
	pp = ppBlocks;
	do
	{
		if (waveOutPrepareHeader(hWaveOut, &(*pp)->wh,
			sizeof(WAVEHDR)) != 0)
			goto prep_fail;
		(*pp)->wh.dwFlags |= WHDR_DONE;
		pp++;
		j++;
	} while (j < 0x14);
	return 1;
alloc_fail:
	if (g_bShowDebugDialogs != 0)
		MessageBoxA(NULL,
			"Unable to allocate memory for waveform data.  Try "
			"making more memory available by closing other "
			"applications.",
			"WavMix32", MB_ICONEXCLAMATION);
	i--;
	while (i >= 0)
	{
		GlobalUnlock(GlobalHandle(ppBlocks[i]));
		GlobalFree(GlobalHandle(ppBlocks[i]));
		ppBlocks[i] = NULL;
		i--;
	}
	return 0;
prep_fail:
	if (g_bShowDebugDialogs != 0)
		MessageBoxA(NULL, "Unable to prepare wave header.",
			"WavMix32", MB_ICONEXCLAMATION);
	WaveMix_FreeWaveBlocks(hWaveOut, ppBlocks);
	return 0;
}

// Unprepares and frees every allocated block of one pointer array
// (always all 20 entries).

// hover.exe:0x438D00
// FUNCTION: HOVER 0x438d00
void WaveMix_FreeWaveBlocks(HWAVEOUT hWaveOut, XWAVEHDR** ppBlocks)
{
	int i;

	i = 0x14;
	do
	{
		if (*ppBlocks != NULL)
		{
			waveOutUnprepareHeader(hWaveOut,
				&(*ppBlocks)->wh, sizeof(WAVEHDR));
			GlobalUnlock(GlobalHandle(*ppBlocks));
			GlobalFree(GlobalHandle(*ppBlocks));
			*ppBlocks = NULL;
		}
		ppBlocks++;
		i--;
	} while (i != 0);
}

// WaveMix play-queue pump ("{pump}"): frees the done blocks at the head of
// the 0x4C59C0/0x4C59C4 queue (marked through
// WaveMix_RemoveFromPlayingQueue), sweeps the channel queues with
// WaveMix_FreePlayedChannelNodes, then fills and queues free wave blocks
// through WaveMix_MixerPlay until a block comes back empty or none are free.
// Runs on the active session.

// hover.exe:0x439A80
// FUNCTION: HOVER 0x439a80
void WaveMixPump()
{
	XWAVEHDR** ppBlocks;
	XWAVEHDR* pXWH;
	int nCount;
	int i;

	WaveMix_DebugOut(3, "{pump}");
	g_pCurSession = g_pActiveSession;
	if (g_pCurSession == NULL)
		return;
	pXWH = g_pPlayQueueHead;
	while (pXWH != NULL)
	{
		if (pXWH->wh.dwFlags & WHDR_DONE)
		{
			WaveMix_RemoveFromPlayingQueue(pXWH);
			pXWH->fAvailable = 1;
			pXWH = g_pPlayQueueHead;
		}
		else
		{
			pXWH = pXWH->pQNext;
		}
	}
	WaveMix_FreePlayedChannelNodes();
	for (;;)
	{
		i = 0;
		nCount = g_pCurSession->dwWaveBlockCount;
		ppBlocks = g_pCurSession->ppWaveBlocks;
		while (i < nCount && ppBlocks[i]->fAvailable == 0)
			i++;
		// "i >= nCount", not "i == nCount" (same block-scan idiom as
		// WaveMixPlay/WaveMixActivate's out-of-blocks arm): a
		// signed jge, not an equality jump.
		if (i >= nCount)
		{
			WaveMix_DebugOut(2, "{gwb:out of wav blks}");
			pXWH = NULL;
		}
		else
		{
			pXWH = ppBlocks[i];
			pXWH->wh.dwBufferLength = g_pCurSession->dwWaveBlockLen;
			pXWH->wh.lpData = (LPSTR)pXWH->abData;
			pXWH->pSession = g_pActiveSession;
			pXWH->fAvailable = 0;
		}
		if (WaveMix_MixerPlay(pXWH, 1) == 0)
			break;
	}
	WaveMix_DebugOut(3, "{end pump}");
}

// WndProc of the hidden "WavMix32" callback window: MM_WOM_DONE (block
// played, lParam is the WAVEHDR) and the private 0x400 message (lParam is
// the session) both pump the play queue; each warns when the block or
// session does not belong to the active session. Everything else goes to
// DefWindowProcA.

// hover.exe:0x439B60
// FUNCTION: HOVER 0x439b60
LRESULT __stdcall WaveMix_WndProc(HWND hwnd, UINT uMsg, WPARAM wParam,
	LPARAM lParam)
{
	WaveMix_DebugOut(2, "{WndProc}");
	switch (uMsg)
	{
	case MM_WOM_DONE:
		if (((XWAVEHDR*)lParam)->pSession != g_pActiveSession)
			WaveMix_DebugOut(1,
				"++++++ waveblock does not match current active "
				"global data pointer!!!!!!!");
		WaveMixPump();
		return 0;
	case 0x400:
		if ((MIXSESSION*)lParam != g_pActiveSession)
			WaveMix_DebugOut(1,
				"++++++ waveblock does not match current active "
				"global data pointer!!!!!!!");
		WaveMixPump();
		return 0;
	}
	return DefWindowProcA(hwnd, uMsg, wParam & 0xFFFF, lParam);
}

// "Remix 2" session callback (selected as the pfnRemix remix function by
// WaveMix_ReadConfigSettings when the ini "Remix" setting is 2): pumps the wave
// block queue through WaveMixPump. Always returns 1.

// hover.exe:0x439BF0
// FUNCTION: HOVER 0x439bf0
int WaveMix_NoResetRemix()
{
	WaveMix_DebugOut(2, "r");
	WaveMixPump();
	WaveMix_DebugOut(2, "{Remix done***}");
	return 1;
}

// Default session callback (the pfnRemix remix function when "Remix" is
// not 2): returns every done block to the free list, swaps the active wave
// block pointer array, refills the playing queue with freshly mixed blocks
// and writes them to the device. While the pause-block counter runs the
// output is paused and restarted every dwPauseBlocks blocks. Always returns 1.
// The four helpers below are the WaveMix DestroyPlayQueue / SwapWaveBlocks /
// GetWaveBlock / AddToPlayingQueue routines; the original was compiled with
// them inlined (no such functions exist in hover.exe).

static __inline void WaveMix_DestroyPlayQueue()
{
	while (g_pPlayQueueHead != NULL)
	{
		g_pPlayQueueHead->fAvailable = 1;
		WaveMix_RemoveFromPlayingQueue(g_pPlayQueueHead);
	}
}

static __inline XWAVEHDR*** WaveMix_SwapWaveBlocks()
{
	XWAVEHDR*** pppBlocks;

	pppBlocks = &g_pCurSession->ppWaveBlocks;
	if (*pppBlocks == g_apWaveBlocksA)
		*pppBlocks = g_apWaveBlocksB;
	else
		*pppBlocks = g_apWaveBlocksA;
	return pppBlocks;
}

static __inline XWAVEHDR* WaveMix_GetWaveBlock()
{
	XWAVEHDR** ppBlocks;
	XWAVEHDR* pXWH;
	int i;
	int nCount;

	i = 0;
	nCount = g_pCurSession->dwWaveBlockCount;
	if (i < nCount)
	{
		ppBlocks = g_pCurSession->ppWaveBlocks;
		do
		{
			if ((*ppBlocks)->fAvailable != 0)
				break;
			ppBlocks++;
			i++;
		} while (i < nCount);
	}
	if (i >= nCount)
	{
		WaveMix_DebugOut(2, "{gwb:out of wav blks}");
		return NULL;
	}
	pXWH = g_pCurSession->ppWaveBlocks[i];
	pXWH->wh.dwBufferLength = g_pCurSession->dwWaveBlockLen;
	pXWH->wh.lpData = (LPSTR)pXWH->abData;
	pXWH->pSession = g_pActiveSession;
	pXWH->fAvailable = 0;
	return pXWH;
}

static __inline void WaveMix_AddToPlayingQueue(XWAVEHDR* pXWH)
{
	pXWH->pQNext = NULL;
	if (g_pPlayQueueHead == NULL)
	{
		g_pPlayQueueTail = pXWH;
		g_pPlayQueueHead = pXWH;
	}
	else
	{
		g_pPlayQueueTail->pQNext = pXWH;
		g_pPlayQueueTail = pXWH;
	}
}

// hover.exe:0x439C20
// FUNCTION: HOVER 0x439c20
int WaveMix_ResetRemix(DWORD dwPos, int bFlag)
{
	XWAVEHDR* pWrite;
	XWAVEHDR* pXWH;
	int bPaused = 0;
	int nPauseCount;
	DWORD dwSave;
	DWORD dwCurPos;
	HWAVEOUT hWaveOut;

	WaveMix_DebugOut(2, "R");
	g_pCurSession->dwCurrentSample = dwPos;
	WaveMix_DestroyPlayQueue();
	WaveMix_SwapWaveBlocks();
	for (;;)
	{
		pXWH = WaveMix_GetWaveBlock();
		if (pXWH == NULL)
			break;
		if (WaveMix_MixerPlay(pXWH, 0) == 0)
		{
			pXWH->fAvailable = 1;
			break;
		}
		WaveMix_AddToPlayingQueue(pXWH);
	}
	dwSave = g_pCurSession->dwCurrentSample;
	hWaveOut = g_pCurSession->hWaveOut;
	dwCurPos = WaveMix_GetWaveOutPosition(hWaveOut, g_pCurSession->fGoodGetPos);
	waveOutReset(hWaveOut);
	WaveMix_SetWaveOutPosition(dwCurPos);
	g_pCurSession->dwCurrentSample = dwSave;
	if ((int)g_pCurSession->dwPauseBlocks > 0)
	{
		waveOutPause(g_pCurSession->hWaveOut);
		bPaused = 1;
		nPauseCount = 0;
	}
	pWrite = g_pPlayQueueHead;
	while (pWrite != NULL)
	{
		if (waveOutWrite(g_pCurSession->hWaveOut,
				&pWrite->wh, sizeof(WAVEHDR)) != 0)
		{
			if (g_bShowDebugDialogs != 0)
				MessageBoxA(NULL, "Failed to write block to device",
					"WavMix32", MB_ICONEXCLAMATION);
			pWrite->fAvailable = 1;
			WaveMix_RemoveFromPlayingQueue(pWrite);
		}
		if (bPaused != 0)
		{
			nPauseCount++;
			if ((int)g_pCurSession->dwPauseBlocks <= nPauseCount)
			{
				waveOutRestart(g_pCurSession->hWaveOut);
				bPaused = 0;
			}
		}
		pWrite = pWrite->pQNext;
	}
	if (bPaused != 0)
		waveOutRestart(g_pCurSession->hWaveOut);
	return 1;
}

// Returns the wave block data length: the "WaveBlockLen" entry of the
// current device's ini section wins over the caller's default; if both are
// zero it is derived from the sampling rate (2048 bytes per 11025 Hz).
// A nonzero result is clamped to 0x200..0x2000.

// hover.exe:0x43B810
// FUNCTION: HOVER 0x43b810
DWORD WaveMix_GetWaveBlockLen(DWORD dwBlockLen, LPWAVEFORMATEX pwfx)
{
	DWORD dwLen;

	dwLen = GetPrivateProfileIntA(g_pCurSession->szDevicePName, "WaveBlockLen",
		0, g_szIniFileName);
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
char* WaveMix_GetOperatingSystemPrefix()
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
// WaveMix_GetOperatingSystemPrefix, producing the device's ini section name.
// Returns 1 when the value exists and fits, 0 otherwise.

// hover.exe:0x43BA10
// FUNCTION: HOVER 0x43ba10
int WaveMix_ReadRegistryToGetMachineSpecificInfSection(DWORD dwDevice, char* pszBuf, int cbBuf)
{
	HKEY hKey;
	long cbData;
	char szValue[52];
	int nLen;
	int bFound;

	bFound = 0;
	nLen = lstrlenA(WaveMix_GetOperatingSystemPrefix());
	if (nLen + 10 > cbBuf)
		return 0;
	wsprintfA(szValue, "Device%u", dwDevice);
	if (RegOpenKeyA(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\WaveMix",
			&hKey) != 0)
		return 0;
	cbBuf -= nLen;
	lstrcpyA(pszBuf, WaveMix_GetOperatingSystemPrefix());
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
int WaveMix_ReadRegistryInt(HKEY hKey, const char* pszValue, int nDefault)
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
// that the caller left unset (flag bits in dwFlags mark the fields that
// were preset) from the WaveMix\Device<u> key under the open HKLM key in
// hRegistryKey, then marks every field set. Returns 1 on success, 0 when the
// configuration carries no open registry key.

// hover.exe:0x43BB20
// FUNCTION: HOVER 0x43bb20
int WaveMix_ReadRegistryForAppSpecificConfigs(void* pv)
{
	MIXCONFIG* pCfg;
	HKEY hKey;
	char szSubKey[52];
	DWORD dwFlags;

	pCfg = (MIXCONFIG*)pv;
	dwFlags = pCfg->dwFlags;
	if ((dwFlags & 0x400) == 0)
		return 0;
	if (pCfg->hRegistryKey == 0)
		return 0;
	if ((dwFlags & 0x80) != 0)
	{
		if (waveOutGetNumDevs() <= pCfg->wDeviceID)
			pCfg->wDeviceID = 0;
	}
	else
		pCfg->wDeviceID = (WORD)g_pCurSession->dwDeviceID;
	wsprintfA(szSubKey, "WaveMix\\Device%u", pCfg->wDeviceID);
	if (RegOpenKeyA(pCfg->hRegistryKey, szSubKey, &hKey) != 0)
		return 0;
	if ((dwFlags & 0x01) == 0)
		pCfg->wChannels = (WORD)WaveMix_ReadRegistryInt(hKey, "Channels", 1);
	if ((dwFlags & 0x02) == 0)
		pCfg->wSamplingRate =
			(WORD)WaveMix_ReadRegistryInt(hKey, "SamplesPerSec", 0xB);
	if ((dwFlags & 0x04) == 0)
		pCfg->wWaveBlockCount = (WORD)WaveMix_ReadRegistryInt(hKey, "WaveBlocks", 3);
	if ((dwFlags & 0x08) == 0)
		pCfg->wWaveBlockLen =
			(WORD)WaveMix_ReadRegistryInt(hKey, "WaveBlockLen", 0x800);
	pCfg->wCmixPtrDefaultFlag = 1;
	if ((dwFlags & 0x20) == 0)
		pCfg->wResetMixDefaultFlag = (WaveMix_ReadRegistryInt(hKey, "Remix", 1) != 2);
	if ((dwFlags & 0x40) == 0)
		pCfg->wGoodWavePos = (WaveMix_ReadRegistryInt(hKey, "GoodWavePos", 0) != 0);
	if ((dwFlags & 0x100) == 0)
		pCfg->wShowDebugDialogs =
			(WORD)WaveMix_ReadRegistryInt(hKey, "ShowDebugDialogs", 0);
	if ((dwFlags & 0x200) == 0)
		pCfg->wPauseBlocks = (WORD)WaveMix_ReadRegistryInt(hKey, "PauseBlocks",
			pCfg->wWaveBlockCount);
	pCfg->dwFlags = 0x3FF;
	waveOutGetDevCapsA(pCfg->wDeviceID, &g_pCurSession->woCaps,
		sizeof(WAVEOUTCAPSA));
	RegCloseKey(hKey);
	return 1;
}

// WaveMix debug helper: pops up a message box summarizing the session's
// current device, mixing function, position tracking, block geometry and
// output format. Called by WaveMixConfigure when debug dialogs are on.

// hover.exe:0x43BCF0
// FUNCTION: HOVER 0x43bcf0
void WaveMix_ShowCurrentSettings()
{
	WAVEOUTCAPSA caps;
	const char* pszMix;
	const char* pszGoodPos;
	const char* pszRemix;
	BOOL bUnknown;

	bUnknown = (waveOutGetDevCapsA(g_pCurSession->dwDeviceID, &caps,
			sizeof(caps)) != 0 ||
		WaveMix_RemoveInvalidIniNameCharacters(caps.szPname) == 0);
	if (bUnknown)
		lstrcpyA(caps.szPname, "Unknown Device");
	pszMix = "cmixit";
	if (g_pCurSession->pfnMixit != (MIXITFN)WaveMix_CMixit)
		pszMix = "386 mixit";
	pszGoodPos = "TRUE";
	if (g_pCurSession->fGoodGetPos == 0)
		pszGoodPos = "FALSE";
	pszRemix = "Reset";
	if (g_pCurSession->pfnRemix != WaveMix_ResetRemix)
		pszRemix = "NoReset";
	wsprintfA(g_szStringBuffer,
		"Using:\t%s.\n\tRemix = %s\n\tGoodGetPos=%s\n\t%d ping pong "
		"wave blocks\n\tWave block len = %lu bytes\n\tpfnMixit=%s\n\t"
		"SamplesPerSec=%lu,\n\tChannels=%d\n\tPauseBlocks=%d",
		caps.szPname, pszRemix, pszGoodPos,
		g_pCurSession->dwWaveBlockCount, g_pCurSession->dwWaveBlockLen, pszMix,
		g_pCurSession->pcm.wf.nSamplesPerSec,
		g_pCurSession->pcm.wf.nChannels,
		g_pCurSession->dwPauseBlocks);
	MessageBoxA(NULL, g_szStringBuffer, "WavMix32", MB_ICONINFORMATION);
}

// WM_INITDIALOG handler for the WaveMix configure dialog: formats the
// window title, stashes the MIXCONFIG pointer in GWL_USERDATA,
// checks the checkboxes from the config, sets the edit-control text
// limits and initial values, and fills the playback-device combo box.
// Always returns 1.

// hover.exe:0x43BDF0
// FUNCTION: HOVER 0x43bdf0
int WaveMix_SettingsOnInitDialog(HWND hwnd, WPARAM wParam, MIXCONFIG* pCfg)
{
	WAVEOUTCAPSA caps;
	char szBuf[0x100];
	int nDevs;
	int nErr;
	int i;

	GetWindowTextA(hwnd, szBuf, sizeof(szBuf));
	wsprintfA(g_szStringBuffer, szBuf, 2, 0x50);
	SetWindowTextA(hwnd, g_szStringBuffer);
	SetWindowLongA(hwnd, GWL_USERDATA, (LONG)pCfg);
	SendMessageA(GetDlgItem(hwnd, 0x3e8), BM_SETCHECK, pCfg->wChannels > 1,
		0);
	SendMessageA(GetDlgItem(hwnd, 0x3e9), BM_SETCHECK, pCfg->wResetMixDefaultFlag >= 1,
		0);
	SendMessageA(GetDlgItem(hwnd, 0x3ec), BM_SETCHECK, pCfg->wGoodWavePos >= 1,
		0);
	SendMessageA(GetDlgItem(hwnd, 0x3ed), BM_SETCHECK, pCfg->wCmixPtrDefaultFlag >= 1,
		0);
	SendMessageA(GetDlgItem(hwnd, 0x3f2), BM_SETCHECK, pCfg->wShowDebugDialogs >= 1,
		0);
	EnableWindow(GetDlgItem(hwnd, 0x3ed), FALSE);
	SendMessageA(GetDlgItem(hwnd, 0x3eb), EM_LIMITTEXT, 2, 0);
	SendMessageA(GetDlgItem(hwnd, 0x3ef), EM_LIMITTEXT, 4, 0);
	SendMessageA(GetDlgItem(hwnd, 0x3f0), EM_LIMITTEXT, 2, 0);
	SendMessageA(GetDlgItem(hwnd, 0x3f3), EM_LIMITTEXT, 2, 0);
	GetWindowTextA(GetDlgItem(hwnd, 0x3f4), szBuf, 0x64);
	wsprintfA(g_szStringBuffer, szBuf, 2, 0x14);
	SetWindowTextA(GetDlgItem(hwnd, 0x3f4), g_szStringBuffer);
	GetWindowTextA(GetDlgItem(hwnd, 0x3f5), szBuf, 0x64);
	wsprintfA(g_szStringBuffer, szBuf, 0x200, 0x2000);
	SetWindowTextA(GetDlgItem(hwnd, 0x3f5), g_szStringBuffer);
	GetWindowTextA(GetDlgItem(hwnd, 0x3f6), szBuf, 0x64);
	wsprintfA(g_szStringBuffer, szBuf, 0, 0x14);
	SetWindowTextA(GetDlgItem(hwnd, 0x3f6), g_szStringBuffer);
	GetWindowTextA(GetDlgItem(hwnd, 0x3f7), szBuf, 0x64);
	wsprintfA(g_szStringBuffer, szBuf, 0x10);
	SetWindowTextA(GetDlgItem(hwnd, 0x3f7), g_szStringBuffer);
	wsprintfA(g_szStringBuffer, "%d", pCfg->wWaveBlockCount);
	SetWindowTextA(GetDlgItem(hwnd, 0x3eb), g_szStringBuffer);
	wsprintfA(g_szStringBuffer, "%d", pCfg->wWaveBlockLen);
	SetWindowTextA(GetDlgItem(hwnd, 0x3ef), g_szStringBuffer);
	wsprintfA(g_szStringBuffer, "%d", pCfg->wSamplingRate);
	SetWindowTextA(GetDlgItem(hwnd, 0x3f0), g_szStringBuffer);
	wsprintfA(g_szStringBuffer, "%d", pCfg->wPauseBlocks);
	SetWindowTextA(GetDlgItem(hwnd, 0x3f3), g_szStringBuffer);
	nDevs = waveOutGetNumDevs();
	for (i = 0; i < nDevs; i++)
	{
		nErr = waveOutGetDevCapsA(i, &caps, sizeof(caps));
		if (nErr != 0)
		{
			wsprintfA(g_szStringBuffer,
				"waveOutGetDevCaps failed (err %u) for device %d",
				nErr, i);
			MessageBoxA(NULL, g_szStringBuffer, "WavMix32",
				MB_ICONINFORMATION);
		}
		else
		{
			wsprintfA(g_szStringBuffer, "%d: %s", i, caps.szPname);
			SendMessageA(GetDlgItem(hwnd, 0x3f1), CB_ADDSTRING, 0,
				(LPARAM)g_szStringBuffer);
		}
	}
	SendMessageA(GetDlgItem(hwnd, 0x3f1), CB_SETCURSEL, pCfg->wDeviceID, 0);
	return 1;
}

// WM_COMMAND handler for the WaveMix configure dialog. On OK it reads
// every control back into the MIXCONFIG stashed in GWL_USERDATA
// and ends the dialog with 1; on Cancel it ends the dialog with 0.

// hover.exe:0x43C1C0
// FUNCTION: HOVER 0x43c1c0
int WaveMix_SettingsOnCommand(HWND hwnd, DWORD dwId, LPARAM lParam, DWORD dwNotify)
{
	MIXCONFIG* pCfg;

	pCfg = (MIXCONFIG*)GetWindowLongA(hwnd, GWL_USERDATA);
	if (dwId == 1)
	{
		if (pCfg != NULL)
		{
			pCfg->wChannels = ((UINT)SendMessageA(GetDlgItem(hwnd, 0x3e8),
				BM_GETCHECK, 0, 0) >= 1) ? 2 : 1;
			pCfg->wResetMixDefaultFlag = ((UINT)SendMessageA(GetDlgItem(hwnd, 0x3e9),
				BM_GETCHECK, 0, 0) >= 1) ? 1 : 0;
			pCfg->wGoodWavePos = ((UINT)SendMessageA(GetDlgItem(hwnd, 0x3ec),
				BM_GETCHECK, 0, 0) >= 1) ? 1 : 0;
			pCfg->wShowDebugDialogs = ((UINT)SendMessageA(GetDlgItem(hwnd, 0x3f2),
				BM_GETCHECK, 0, 0) >= 1) ? 1 : 0;
			pCfg->wCmixPtrDefaultFlag = ((UINT)SendMessageA(GetDlgItem(hwnd, 0x3ed),
				BM_GETCHECK, 0, 0) >= 1) ? 1 : 0;
			GetWindowTextA(GetDlgItem(hwnd, 0x3eb), g_szStringBuffer,
				10);
			pCfg->wWaveBlockCount = (WORD)atoi(g_szStringBuffer);
			GetWindowTextA(GetDlgItem(hwnd, 0x3ef), g_szStringBuffer,
				10);
			pCfg->wWaveBlockLen = (WORD)atoi(g_szStringBuffer);
			GetWindowTextA(GetDlgItem(hwnd, 0x3f0), g_szStringBuffer,
				10);
			pCfg->wSamplingRate = (WORD)atoi(g_szStringBuffer);
			GetWindowTextA(GetDlgItem(hwnd, 0x3f3), g_szStringBuffer,
				10);
			pCfg->wPauseBlocks = (WORD)atoi(g_szStringBuffer);
			GetWindowTextA(GetDlgItem(hwnd, 0x3f1), g_szStringBuffer,
				10);
			if (isdigit(g_szStringBuffer[0]))
				pCfg->wDeviceID = (WORD)atoi(g_szStringBuffer);
			else
				pCfg->wDeviceID = 0;
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
// to WaveMix_SettingsOnInitDialog (returning its result) and WM_COMMAND to
// WaveMix_SettingsOnCommand.

// hover.exe:0x43C3C0
// FUNCTION: HOVER 0x43c3c0
BOOL __stdcall WaveMix_SettingsDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam,
	LPARAM lParam)
{
	if (uMsg == WM_INITDIALOG)
		return WaveMix_SettingsOnInitDialog(hwnd, wParam, (MIXCONFIG*)lParam);
	if (uMsg == WM_COMMAND)
	{
		// lParam before the notification code, as hover.exe pushes them
		WaveMix_SettingsOnCommand(hwnd, LOWORD(wParam), lParam, HIWORD(wParam));
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
void* WaveMix_MakeDlgTemplate(DWORD* pcbTemplate, DWORD dwStyle, int x, int y,
	int cx, int cy, LPCWSTR pszTitle)
{
	DLGTEMPLATE* pTemplate;

	// The header, then the empty menu and class words and the title with
	// its terminator.
	*pcbTemplate = wcslen(pszTitle) * sizeof(WCHAR) + sizeof(DLGTEMPLATE) +
		3 * sizeof(WORD);
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
	memcpy((WORD*)(pTemplate + 1) + 2, pszTitle,
		(wcslen(pszTitle) + 1) * sizeof(WCHAR));
	return pTemplate;
}

// Grows an in-memory dialog template built by WaveMix_MakeDlgTemplate by one
// control (a DLGITEMTEMPLATE with its class stored as a 0xFFFF/ordinal
// pair and a wide title string), reallocating the block and updating
// *pcbTemplate and the template's control count. wClass is the
// predefined window class ordinal (0x80 button, 0x81 edit, 0x82 static,
// 0x85 combo box). Returns the re-GlobalLock'd template pointer, or
// NULL on failure.

// hover.exe:0x43C4C0
// FUNCTION: HOVER 0x43c4c0
void* WaveMix_AddDlgControl(DWORD* pcbTemplate, void* hTemplate, WORD wClass,
	DWORD dwStyle, WORD wId, int x, int y, int cx, int cy,
	LPCWSTR pszText)
{
	DLGITEMTEMPLATE* pItem;
	DLGTEMPLATE* pTemplate;
	DWORD cbOld;

	if (hTemplate != NULL)
	{
		cbOld = *pcbTemplate;
		// The item header, the 0xFFFF/ordinal class pair, the title with
		// its terminator, and a creation-data count (the original
		// reserves one byte for it; the DWORD rounding below covers the
		// second).
		*pcbTemplate = cbOld + wcslen(pszText) * sizeof(WCHAR) +
			sizeof(DLGITEMTEMPLATE) + sizeof(DWORD) + sizeof(WCHAR) + 1;
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
			*(DWORD*)(pItem + 1) = ((DWORD)wClass << 16) | 0xFFFF;
			wcscpy((WCHAR*)(pItem + 1) + 2, pszText);
			pTemplate->cdit++;
		}
	}
	return hTemplate;
}

// Builds the in-memory dialog template for the WaveMix configure dialog
// (device combo box, channel/remix/position checkboxes, wave block and
// sampling rate edit fields, OK/Cancel) and returns the GlobalLock'd block;
// the caller frees it with WaveMix_DestroySettingsDlgTemplate. Returns NULL
// on failure.

// hover.exe:0x43C5A0
// FUNCTION: HOVER 0x43c5a0
void* WaveMix_MakeSettingsDlgTemplate()
{
	DWORD cbTemplate;
	void* pTemplate;

	cbTemplate = 0;
	pTemplate = WaveMix_MakeDlgTemplate(&cbTemplate,
		DS_MODALFRAME | WS_POPUP | WS_CAPTION | WS_SYSMENU, 0, 0, 0xd4,
		0x84, L"WavMix32 Settings (Ver %X.%X Static)");
	pTemplate = WaveMix_AddDlgControl(&cbTemplate, pTemplate, 0x80,
		WS_TABSTOP | BS_DEFPUSHBUTTON, 1, 0x9b, 5, 0x32, 0xe, L"&Ok");
	pTemplate = WaveMix_AddDlgControl(&cbTemplate, pTemplate, 0x80,
		WS_TABSTOP | BS_PUSHBUTTON, 2, 0x9b, 0x19, 0x32, 0xe,
		L"&Cancel");
	pTemplate = WaveMix_AddDlgControl(&cbTemplate, pTemplate, 0x82, 0, 0x3f4,
		5, 5, 0x73, 8, L"Number of WaveBlocks (%d-%d):");
	pTemplate = WaveMix_AddDlgControl(&cbTemplate, pTemplate, 0x82, 0, 0x3f5,
		5, 0x14, 0x73, 8, L"Size of WaveBlocks (%d-%d):");
	pTemplate = WaveMix_AddDlgControl(&cbTemplate, pTemplate, 0x82, 0, 0xffff,
		5, 0x23, 0x66, 8, L"Playback Frequency (11,22,44):");
	pTemplate = WaveMix_AddDlgControl(&cbTemplate, pTemplate, 0x82, 0, 0xffff,
		5, 0x50, 0x39, 8, L"Playback Device:");
	pTemplate = WaveMix_AddDlgControl(&cbTemplate, pTemplate, 0x82, 0, 0x3f6,
		5, 0x32, 0x73, 8, L"Number of Pause Blocks (%d-%d):");
	pTemplate = WaveMix_AddDlgControl(&cbTemplate, pTemplate, 0x82, 0, 0x3f7,
		5, 0x41, 0x3f, 8, L"Max Channels = %d");
	pTemplate = WaveMix_AddDlgControl(&cbTemplate, pTemplate, 0x81,
		ES_AUTOHSCROLL | WS_BORDER | WS_TABSTOP, 0x3eb, 0x7d, 5, 0x19,
		0xc, L"-1");
	pTemplate = WaveMix_AddDlgControl(&cbTemplate, pTemplate, 0x81,
		ES_AUTOHSCROLL | WS_BORDER | WS_TABSTOP, 0x3ef, 0x7d, 0x14,
		0x19, 0xc, L"-1");
	pTemplate = WaveMix_AddDlgControl(&cbTemplate, pTemplate, 0x81,
		ES_AUTOHSCROLL | WS_BORDER | WS_TABSTOP, 0x3f0, 0x7d, 0x23,
		0x19, 0xc, L"-1");
	pTemplate = WaveMix_AddDlgControl(&cbTemplate, pTemplate, 0x81,
		ES_AUTOHSCROLL | WS_BORDER | WS_TABSTOP, 0x3f3, 0x7d, 0x32,
		0x19, 0xc, L"-1");
	pTemplate = WaveMix_AddDlgControl(&cbTemplate, pTemplate, 0x85,
		WS_VSCROLL | WS_TABSTOP | CBS_SORT | CBS_DROPDOWN, 0x3f1, 0x41,
		0x50, 0x8c, 0x3c, L"No Devices");
	pTemplate = WaveMix_AddDlgControl(&cbTemplate, pTemplate, 0x80,
		WS_TABSTOP | BS_AUTOCHECKBOX, 0x3e8, 5, 0x65, 0x28, 0xa,
		L"Stereo");
	pTemplate = WaveMix_AddDlgControl(&cbTemplate, pTemplate, 0x80,
		WS_TABSTOP | BS_AUTOCHECKBOX, 0x3e9, 0x37, 0x65, 0x3c, 0xa,
		L"Reset Remix");
	pTemplate = WaveMix_AddDlgControl(&cbTemplate, pTemplate, 0x80,
		WS_TABSTOP | BS_AUTOCHECKBOX, 0x3f2, 0x7d, 0x65, 0x4e, 0xa,
		L"Show Debug Dialogs");
	pTemplate = WaveMix_AddDlgControl(&cbTemplate, pTemplate, 0x80,
		WS_TABSTOP | BS_AUTOCHECKBOX, 0x3ed, 5, 0x75, 0x28, 0xa,
		L"CMixit");
	pTemplate = WaveMix_AddDlgControl(&cbTemplate, pTemplate, 0x80,
		WS_TABSTOP | BS_AUTOCHECKBOX, 0x3ec, 0x37, 0x75, 0x4b, 0xa,
		L"Good Get Position");
	return pTemplate;
}

// Frees the global memory block holding a WaveMixConfigure dialog
// template built by WaveMix_MakeSettingsDlgTemplate.

// hover.exe:0x43C8C0
// FUNCTION: HOVER 0x43c8c0
void WaveMix_DestroySettingsDlgTemplate(void* hTemplate)
{
	if (hTemplate != NULL)
	{
		GlobalUnlock(GlobalHandle((LPCVOID)hTemplate));
		GlobalFree(GlobalHandle((LPCVOID)hTemplate));
	}
}

// WaveMixConfigure: applies a MIXCONFIG to the session. When pv is
// NULL the current settings are read back into a local config (through
// WaveMixGetConfig) and edited in a modal configuration dialog first.
// The dwFlags flag bits select the fields to apply (0x01 channels, 0x02
// sampling rate, 0x04 wave blocks, 0x08 block length, 0x10 mixing
// function, 0x20 remix callback, 0x40 position source, 0x80 device,
// 0x100 debug dialogs, 0x200 pause blocks). When dwB is nonzero it
// receives a flag telling whether the output format changed; when dwC is
// nonzero the applied settings are written back to the ini file. Returns
// 0 on success, 5 = invalid session, 4 = session active, 0xc = driver in
// use, 1 on dialog/config failure, or a waveOutGetDevCapsA error code.

// hover.exe:0x43C8F0
// FUNCTION: HOVER 0x43c8f0
int __stdcall WaveMixConfigure(MIXSESSION* pEngine, DWORD dwA, void* pv,
	DWORD dwB, DWORD dwC)
{
	MIXCONFIG cfgLocal;
	MIXCONFIG* pCfg;
	WAVEOUTCAPSA caps;
	void* pTemplate;
	DWORD dwFlags;
	int bChanged;
	int nDlg;
	int nErr;

	bChanged = 0;
	if (pEngine != NULL && pEngine->wMagic1 == 0x5432 &&
		pEngine->wMagic2 == 0x5432)
	{
		g_pCurSession = pEngine;
	}
	else
	{
		MessageBeep(0xFFFFFFFF);
		wsprintfA(g_szStringBuffer,
			"Invalid session handle 0x%04X passed to WaveMix API",
			pEngine);
		WaveMix_DebugOut(1, g_szStringBuffer);
		MessageBoxA(NULL, g_szStringBuffer, "WavMix32",
			MB_ICONEXCLAMATION);
		g_pCurSession = NULL;
	}
	if (g_pCurSession == NULL)
		return 5;
	if (g_pCurSession->fActive != 0)
		return 4;
	if (g_pCurSession->fSettingsDialogActive != 0)
		return 0xc;
	WaveMixFlushChannel(pEngine, 0xFFFFFFFF, 1);
	pCfg = (MIXCONFIG*)pv;
	if (pCfg == NULL)
	{
		cfgLocal.dwFlags = 0x3FF;
		cfgLocal.wSize = 0x1E;
		WaveMixGetConfig(pEngine, &cfgLocal);
		pTemplate = WaveMix_MakeSettingsDlgTemplate();	// dialog template
		if (pTemplate == NULL)
			return 1;
		g_pCurSession->fSettingsDialogActive = 1;
		nDlg = DialogBoxIndirectParamA(g_hWaveMixInst,
			(LPCDLGTEMPLATE)pTemplate, (HWND)dwA,
			(DLGPROC)WaveMix_SettingsDlgProc,	// dialog proc
			(LPARAM)&cfgLocal);
		WaveMix_DestroySettingsDlgTemplate(pTemplate);
		g_pCurSession->fSettingsDialogActive = 0;
		if (nDlg != 1)
			return 1;
		pCfg = &cfgLocal;
	}
	if (pCfg->dwFlags == 0)
		return 1;
	if (pEngine != NULL && pEngine->wMagic1 == 0x5432 &&
		pEngine->wMagic2 == 0x5432)
	{
		g_pCurSession = pEngine;
	}
	else
	{
		MessageBeep(0xFFFFFFFF);
		wsprintfA(g_szStringBuffer,
			"Invalid session handle 0x%04X passed to WaveMix API",
			pEngine);
		WaveMix_DebugOut(1, g_szStringBuffer);
		MessageBoxA(NULL, g_szStringBuffer, "WavMix32",
			MB_ICONEXCLAMATION);
		g_pCurSession = NULL;
	}
	if (g_pCurSession == NULL)
		return 5;
	dwFlags = pCfg->dwFlags;
	if (dwFlags & 0x100)
		g_bShowDebugDialogs = (pCfg->wShowDebugDialogs >= 1);
	if (dwFlags & 0x80)
	{
		if (g_pCurSession->dwDeviceID != pCfg->wDeviceID)
		{
			nErr = waveOutGetDevCapsA(pCfg->wDeviceID, &caps,
				sizeof(caps));
			if (nErr != 0)
				return nErr;
			memcpy(&g_pCurSession->woCaps, &caps, sizeof(caps));
			g_pCurSession->dwDeviceID = pCfg->wDeviceID;
			if (g_pCurSession->woCaps.wChannels == 1 &&
				g_pCurSession->pcm.wf.nChannels == 2)
			{
				g_pCurSession->pcm.wf.nChannels = 1;
				bChanged = 1;
				g_pCurSession->pcm.wf.nBlockAlign = 1;
				g_pCurSession->pcm.wf.nAvgBytesPerSec =
					g_pCurSession->pcm.wf.nChannels *
					g_pCurSession->pcm.wf.nSamplesPerSec;
			}
			lstrcpyA(g_pCurSession->szDevicePName, caps.szPname);
			if (WaveMix_RemoveInvalidIniNameCharacters(g_pCurSession->szDevicePName) == 0)
				lstrcpyA(g_pCurSession->szDevicePName, "Unkown Device");
			if (WaveMix_ReadRegistryToGetMachineSpecificInfSection(g_pCurSession->dwDeviceID,
					g_pCurSession->szDevicePName,
					sizeof(g_pCurSession->szDevicePName)) == 0)
			{
				lstrcpyA(g_pCurSession->szDevicePName,
					WaveMix_GetOperatingSystemPrefix());
				lstrcatA(g_pCurSession->szDevicePName,
					g_pCurSession->woCaps.szPname);
			}
		}
	}
	if (dwFlags & 1)
	{
		if (pCfg->wChannels > 1)
		{
			if (g_pCurSession->woCaps.wChannels > 1)
			{
				if (g_pCurSession->pcm.wf.nChannels == 1)
					bChanged = 1;
				g_pCurSession->pcm.wf.nChannels = 2;
				g_pCurSession->pcm.wf.nBlockAlign = 2;
				g_pCurSession->pcm.wf.nAvgBytesPerSec =
					g_pCurSession->pcm.wf.nChannels *
					g_pCurSession->pcm.wf.nSamplesPerSec;
			}
		}
		else
		{
			if (g_pCurSession->pcm.wf.nChannels == 2)
				bChanged = 1;
			g_pCurSession->pcm.wf.nChannels = 1;
			g_pCurSession->pcm.wf.nBlockAlign = 1;
			g_pCurSession->pcm.wf.nAvgBytesPerSec =
				g_pCurSession->pcm.wf.nChannels *
				g_pCurSession->pcm.wf.nSamplesPerSec;
		}
	}
	if (dwFlags & 2)
	{
		switch (pCfg->wSamplingRate)
		{
		case 22:
			if (g_pCurSession->pcm.wf.nSamplesPerSec != 22050)
				bChanged = 1;
			g_pCurSession->pcm.wf.nSamplesPerSec = 22050;
			g_pCurSession->pcm.wf.nAvgBytesPerSec =
				22050 * g_pCurSession->pcm.wf.nChannels;
			break;
		case 44:
			if (g_pCurSession->pcm.wf.nSamplesPerSec != 44100)
				bChanged = 1;
			g_pCurSession->pcm.wf.nSamplesPerSec = 44100;
			g_pCurSession->pcm.wf.nAvgBytesPerSec =
				44100 * g_pCurSession->pcm.wf.nChannels;
			break;
		default:
			if (g_pCurSession->pcm.wf.nSamplesPerSec != 11025)
				bChanged = 1;
			g_pCurSession->pcm.wf.nSamplesPerSec = 11025;
			g_pCurSession->pcm.wf.nAvgBytesPerSec =
				11025 * g_pCurSession->pcm.wf.nChannels;
			break;
		}
	}
	if (dwFlags & 4)
	{
		g_pCurSession->dwWaveBlockCount = pCfg->wWaveBlockCount;
		if ((int)g_pCurSession->dwWaveBlockCount < 2)
			g_pCurSession->dwWaveBlockCount = 2;
		else if ((int)g_pCurSession->dwWaveBlockCount > 0x14)
			g_pCurSession->dwWaveBlockCount = 0x14;
		if ((int)g_pCurSession->dwPauseBlocks < 0)
			g_pCurSession->dwPauseBlocks = 0;
		else if ((int)g_pCurSession->dwPauseBlocks >
				(int)g_pCurSession->dwWaveBlockCount)
			g_pCurSession->dwPauseBlocks = g_pCurSession->dwWaveBlockCount;
	}
	if (dwFlags & 8)
	{
		g_pCurSession->dwWaveBlockLen = pCfg->wWaveBlockLen;
		if (g_pCurSession->dwWaveBlockLen < 0x200)
			g_pCurSession->dwWaveBlockLen = 0x200;
		else if (g_pCurSession->dwWaveBlockLen > 0x2000)
			g_pCurSession->dwWaveBlockLen = 0x2000;
	}
	if (dwFlags & 0x10)
		g_pCurSession->pfnMixit = (MIXITFN)WaveMix_CMixit;
	if (dwFlags & 0x20)
	{
		REMIXFN pfnProc = (pCfg->wResetMixDefaultFlag != 0) ?
			WaveMix_ResetRemix : (REMIXFN)WaveMix_NoResetRemix;
		g_pCurSession->pfnRemix = pfnProc;
	}
	if (dwFlags & 0x40)
		g_pCurSession->fGoodGetPos = (pCfg->wGoodWavePos >= 1);
	if (dwFlags & 0x200)
	{
		g_pCurSession->dwPauseBlocks = pCfg->wPauseBlocks;
		if ((int)g_pCurSession->dwPauseBlocks < 0)
			g_pCurSession->dwPauseBlocks = 0;
		else if ((int)g_pCurSession->dwPauseBlocks >
				(int)g_pCurSession->dwWaveBlockCount)
			g_pCurSession->dwPauseBlocks = g_pCurSession->dwWaveBlockCount;
	}
	WaveMixGetConfig(pEngine, pCfg);
	if (g_bShowDebugDialogs != 0)
		WaveMix_ShowCurrentSettings();
	if (dwB != 0)
		*(DWORD*)dwB = bChanged;
	if (dwC != 0)
		WaveMix_SaveConfigSettings(dwFlags);	// write ini settings
	return 0;
}

// WaveMixGetSessionConfig: writes the session's current settings back
// into the caller's MIXCONFIG, for the fields selected by the
// dwFlags flag bits (same bit assignments as WaveMixConfigure). Returns
// 0 on success, 5 = invalid session, 0xb = NULL config.

// hover.exe:0x43CF50
// FUNCTION: HOVER 0x43cf50
int __stdcall WaveMixGetConfig(MIXSESSION* pEngine, MIXCONFIG* pCfg)
{
	DWORD dwFlags;

	if (pEngine != NULL && pEngine->wMagic1 == 0x5432 &&
		pEngine->wMagic2 == 0x5432)
	{
		g_pCurSession = pEngine;
	}
	else
	{
		MessageBeep(0xFFFFFFFF);
		wsprintfA(g_szStringBuffer,
			"Invalid session handle 0x%04X passed to WaveMix API",
			pEngine);
		WaveMix_DebugOut(1, g_szStringBuffer);
		MessageBoxA(NULL, g_szStringBuffer, "WavMix32",
			MB_ICONEXCLAMATION);
		g_pCurSession = NULL;
	}
	if (g_pCurSession == NULL)
		return 5;
	if (pCfg == NULL)
		return 0xb;
	dwFlags = pCfg->dwFlags;
	if (dwFlags & 1)
		pCfg->wChannels = g_pCurSession->pcm.wf.nChannels;
	if (dwFlags & 2)
		pCfg->wSamplingRate = (WORD)(g_pCurSession->pcm.wf.nSamplesPerSec /
			11025 * 11);
	if (dwFlags & 4)
		pCfg->wWaveBlockCount = (WORD)g_pCurSession->dwWaveBlockCount;
	if (dwFlags & 8)
		pCfg->wWaveBlockLen = (WORD)g_pCurSession->dwWaveBlockLen;
	if (dwFlags & 0x10)
		pCfg->wCmixPtrDefaultFlag =
			(g_pCurSession->pfnMixit == (MIXITFN)WaveMix_CMixit);
	if (dwFlags & 0x20)
		pCfg->wResetMixDefaultFlag =
			(g_pCurSession->pfnRemix == WaveMix_ResetRemix);
	if (dwFlags & 0x40)
		pCfg->wGoodWavePos = (WORD)g_pCurSession->fGoodGetPos;
	if (dwFlags & 0x80)
		pCfg->wDeviceID = (WORD)g_pCurSession->dwDeviceID;
	if (dwFlags & 0x100)
		pCfg->wShowDebugDialogs = (g_bShowDebugDialogs >= 1);
	if (dwFlags & 0x200)
		pCfg->wPauseBlocks = (WORD)g_pCurSession->dwPauseBlocks;
	return 0;
}

// Builds the wavemix.ini path in g_szIniFileName: the module's own
// filename with everything from the last backslash on replaced by
// "\WAVEMIX.INF". Returns 1 on success, 0 on failure.

// hover.exe:0x43DB90
// FUNCTION: HOVER 0x43db90
int WaveMix_SetIniFileName(HMODULE hMod)
{
	char* p;
	DWORD dwLen;

	dwLen = GetModuleFileNameA(hMod, g_szIniFileName, MAX_PATH);
	if (dwLen == 0)
		return 0;
	p = g_szIniFileName + dwLen;
	while (*p != '\\')
		p--;
	*p = '\0';
	lstrcpyA(p, "\\WAVEMIX.INF");
	return 1;
}

// Builds the 11x256 sample-volume table at g_abVolumeTable: entry
// [volume][sample] = volume * (sample - 128) / 10 + 128, so volume row 0
// is silence and row 10 is the identity mapping.

// hover.exe:0x43DBE0
// FUNCTION: HOVER 0x43dbe0
void WaveMix_InitVolumeTable()
{
	int j;
	int i;

	for (i = 0; i <= 10; i++)
	{
		for (j = -128; j < 128; j++)
			g_abVolumeTable[i][j + 128] = (BYTE)(i * j / 10 + 128);
	}
}

// WaveMix one-time module initialization: builds the ini path and the
// volume table, reads the "debug" verbosity setting, selects the Remix=2
// mixing function, registers the "WavMix32" callback window class and
// initializes the channel-node pool. Returns 1 on success, 0 on failure.

// hover.exe:0x43DC40
// FUNCTION: HOVER 0x43dc40
int WaveMix_Startup(HMODULE hMod)
{
	WNDCLASSA wc;

	if (g_bWaveMixInitialized != 0)
		return 1;
	if (WaveMix_SetIniFileName(hMod) == 0)
		return 0;
	WaveMix_InitVolumeTable();
	g_dwDebugLevel = GetPrivateProfileIntA("general", "debug", 0,
		g_szIniFileName);
	g_hWaveMixInst = hMod;
	g_pfnDefaultMixit = (MIXITFN)WaveMix_CMixit;
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
	wc.hInstance = g_hWaveMixInst;
	wc.cbWndExtra = 0;
	wc.cbClsExtra = 0;
	wc.lpfnWndProc = WaveMix_WndProc;
	if (RegisterClassA(&wc) == 0)
		return 0;
	WaveMix_InitChannelNodes();
	g_bWaveMixInitialized = 1;
	return 1;
}

// Pops a channel node off the g_pFreeChannelNodes free list (the list is
// built from the g_aChannelNodePool pool by WaveMix_InitChannelNodes).

// hover.exe:0x438CD0
// FUNCTION: HOVER 0x438cd0
CHANNELNODE* WaveMix_GetChannelNode()
{
	CHANNELNODE* pNode;

	if (g_pFreeChannelNodes == NULL)
	{
		WaveMix_DebugOut(1, "GetChannelNode failed!");
		return NULL;
	}
	pNode = g_pFreeChannelNodes;
	g_pFreeChannelNodes = pNode->pNext;
	pNode->pNext = NULL;
	return pNode;
}

// Resets the play position when nothing is queued on any channel and no
// done blocks are pending.

// hover.exe:0x439E40
// FUNCTION: HOVER 0x439e40
void WaveMix_ResetWavePosIfNoChannelData()
{
	int i;

	if (g_pPlayQueueHead != NULL)
		return;
	for (i = 0; i < 0x10; i++)
	{
		if (g_pCurSession->apChannel[i] != NULL &&
			g_pCurSession->apChannel[i] != WMIX_CHANNEL_CLOSED)
			return;
	}
	WaveMix_SetWaveOutPosition(0);
}

// WaveMixPlay: validates the session and the MIXWAVE (same magic-word
// check and error reporting as WaveMixOpenChannel), picks the channel (the
// least recently used one when flag 0x2 is set), queues a new channel node
// on the session's circular wait list (pWaitList) and, unless flag 0x8
// requests queue-only, moves every queued node onto its channel and tops
// up the waveOut block queue. Returns 0 on success, 4 when the session is
// inactive, 5 on an invalid parameter/session/wave/channel and 8 when the
// wave is not in the current output format.

// hover.exe:0x439E80
// FUNCTION: HOVER 0x439e80
int __stdcall WaveMixPlay(MIXPLAYPARAMS* volatile pParams)
{
	MIXSESSION* pEngine;
	MIXWAVE* pWave;
	CHANNELNODE* pNode;
	CHANNELNODE* pTail;
	CHANNELNODE* pCur;
	CHANNELNODE* pNext;
	CHANNELNODE** ppSlot;
	XWAVEHDR** ppBlocks;
	XWAVEHDR* pXWH;
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
	WaveMix_DebugOut(2, "<Play>");
	g_dwPlayNesting++;
	if ((int)g_dwPlayNesting > 1)
		goto Done;
	if (pParams == NULL)
	{
		if (g_bShowDebugDialogs != 0)
			MessageBoxA(NULL,
				"NULL parameters pointer passed to WaveMixPlay!",
				"WavMix32", MB_ICONEXCLAMATION);
		dwPos = 5;
		goto Done;
	}
	pEngine = pParams->pSession;
	if (pEngine != NULL && pEngine->wMagic1 == 0x5432 &&
		pEngine->wMagic2 == 0x5432)
	{
		g_pCurSession = pEngine;
	}
	else
	{
		MessageBeep(0xFFFFFFFF);
		wsprintfA(g_szStringBuffer,
			"Invalid session handle 0x%04X passed to WaveMix API",
			pEngine);
		WaveMix_DebugOut(1, g_szStringBuffer);
		MessageBoxA(NULL, g_szStringBuffer, "WavMix32",
			MB_ICONEXCLAMATION);
		g_pCurSession = NULL;
	}
	if (g_pCurSession == NULL)
	{
		if (g_bShowDebugDialogs != 0)
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
		if (g_bShowDebugDialogs != 0)
			MessageBoxA(NULL,
				"Invalid or NULL wave pointer passed to WaveMixPlay!",
				"WavMix32", MB_ICONEXCLAMATION);
		dwPos = 5;
		goto Done;
	}
	if (memcmp(pWave, &g_pCurSession->pcm, sizeof(PCMWAVEFORMAT)) != 0)
	{
		wsprintfA(g_szStringBuffer,
			"The LPMIXWAVE 0x%lx is not in the current output format, "
			"close the wave and reopen it.", pWave);
		WaveMix_DebugOut(1, g_szStringBuffer);
		if (g_bShowDebugDialogs != 0)
			MessageBoxA(NULL, g_szStringBuffer, "WavMix32",
				MB_ICONEXCLAMATION);
		dwPos = 8;
		goto Done;
	}
	if (g_pCurSession->fActive == 0)
	{
		if (g_bShowDebugDialogs != 0)
			MessageBoxA(NULL,
				"Wave device not allocated, call "
				"WaveMixActivate(hMixSession,TRUE)",
				"WavMix32", MB_ICONEXCLAMATION);
		dwPos = 4;
		goto Done;
	}
	if (g_pCurSession->dwOpenChannels == 0)
	{
		if (g_bShowDebugDialogs != 0)
			MessageBoxA(NULL,
				"You must open a channel before you can play a wave!",
				"WavMix32", MB_ICONEXCLAMATION);
		dwPos = 5;
		goto Done;
	}
	if ((pParams->dwFlags & 2) != 0)
	{
		// pick the least recently used open channel
		WaveMix_DebugOut(2, "<LRU>");
		nChannel = 0;
		for (i = 0; i < 0x10; i++)
		{
			if (g_pCurSession->apChannel[i] == WMIX_CHANNEL_CLOSED)
				continue;
			if (g_pCurSession->apChannel[nChannel] == NULL)
				break;
			if (i == nChannel)
				continue;
			if (g_pCurSession->adwChannelMRU[nChannel] >
				g_pCurSession->adwChannelMRU[i])
				nChannel = i;
		}
		pParams->nChannel = nChannel;
	}
	else
	{
		nChannel = pParams->nChannel;
	}
	g_pCurSession->dwMRU++;
	g_pCurSession->adwChannelMRU[nChannel] = g_pCurSession->dwMRU;
	if (g_pCurSession->apChannel[nChannel] == WMIX_CHANNEL_CLOSED)
	{
		dwPos = 5;
		goto Done;
	}
	pNode = WaveMix_GetChannelNode();
	if (pNode == NULL)
		goto Done;
	memcpy(&pNode->PlayParams, pParams, sizeof(MIXPLAYPARAMS));
	pNode->lpMixWave = pNode->PlayParams.pMixWave;
	pWave = pNode->lpMixWave;
	pNode->dwNumSamples = pWave->wh.dwBufferLength;
	pNode->lpPos = (BYTE*)pWave->wh.lpData;
	pNode->lpEnd = (BYTE*)pWave->wh.lpData + (pWave->wh.dwBufferLength -
		g_pCurSession->pcm.wf.nBlockAlign);
	pNode->PlayParams.nChannel = nChannel;
	// The kind character is computed in a register at the push site in the
	// original (and,cmp,sbb,and,add on the flag word), not through a local.
	wsprintfA(g_szStringBuffer, "(%d:%s L%u %c)", nChannel, pWave->szName,
		pNode->PlayParams.wLoops,
		((pNode->PlayParams.dwFlags & 8) != 0) ? 'W' : '!');
	WaveMix_DebugOut(2, g_szStringBuffer);
	// queue the node on the session's circular wait list
	pTail = g_pCurSession->pWaitList;
	if (pTail != NULL)
	{
		pNode->pNext = pTail->pNext;
		pTail->pNext = pNode;
		g_pCurSession->pWaitList = pNode;
	}
	else
	{
		g_pCurSession->pWaitList = pNode;
		pNode->pNext = pNode;
	}
	if ((pNode->PlayParams.dwFlags & 8) != 0)
	{
		WaveMix_DebugOut(2, "wait list");
		goto Success;
	}
	WaveMix_ResetWavePosIfNoChannelData();
	if (g_pCurSession->pfnRemix == WaveMix_ResetRemix)
		dwPos = WaveMix_GetWaveOutPosition(g_pCurSession->hWaveOut,
			g_pCurSession->fGoodGetPos);
	else
		dwPos = g_pCurSession->dwCurrentSample;
	while (g_pCurSession->pWaitList != NULL)
	{
		// dequeue the oldest wait-list node
		pTail = g_pCurSession->pWaitList;
		pNode = pTail->pNext;
		if (pNode == pTail)
			g_pCurSession->pWaitList = NULL;
		else
			pTail->pNext = pNode->pNext;
		pNode->pNext = NULL;
		nChannel = pNode->PlayParams.nChannel;
		if ((pNode->PlayParams.dwFlags & 0x10) != 0)
		{
			pNode->wVolumeL =
				pNode->PlayParams.wVolumeL;
			pNode->wVolumeR =
				pNode->PlayParams.wVolumeR;
		}
		else
		{
			pNode->wVolumeL =
				g_pCurSession->awChannelVolume[nChannel * 2];
			pNode->wVolumeR =
				g_pCurSession->awChannelVolume[nChannel * 2 + 1];
		}
		if (pNode->wVolumeL > 0xA)
			pNode->wVolumeL = 0xA;
		if (pNode->wVolumeR > 0xA)
			pNode->wVolumeR = 0xA;
		if ((pNode->PlayParams.dwFlags & 1) != 0)
		{
			// replace the channel's node list, recycling the old nodes
			pCur = g_pCurSession->apChannel[nChannel];
			while (pCur != NULL)
			{
				pNext = pCur->pNext;
				WaveMix_DebugOut(2, "{FreeCNode");
				if (pCur == NULL)
				{
					WaveMix_DebugOut(1, " FreeCN: NULL parameter!}");
				}
				else
				{
					pCur->pNext = g_pFreeChannelNodes;
					g_pFreeChannelNodes = pCur;
					WaveMix_DebugOut(2, "}");
				}
				pCur = pNext;
			}
			g_pCurSession->apChannel[nChannel] = pNode;
			if (g_pPlayQueueHead != NULL)
				bQueued = 1;
			if ((pNode->PlayParams.dwFlags & 4) != 0)
				pNode->dwStartPos = dwPos;
			else
				pNode->dwStartPos = g_pCurSession->dwCurrentSample;
			goto SetStop;
		}
		// append the node after the channel's last queued node
		ppSlot = &g_pCurSession->apChannel[nChannel];
		pCur = *ppSlot;
		if (pCur != NULL)
		{
			while (pCur->pNext != NULL)
				pCur = pCur->pNext;
			pCur->pNext = pNode;
			if (pCur->dwEndPos == 0xFFFFFFFF)
				WaveMix_DebugOut(1,
					"!!!! adding to channel at a position where will never get played!!!!\n\r");
			if ((pNode->PlayParams.dwFlags & 4) != 0)
			{
				dwStart = pCur->dwEndPos;
				if (dwStart < dwPos)
					dwStart = dwPos;
				pNode->dwStartPos = dwStart;
			}
			else
			{
				dwStart = g_pCurSession->dwCurrentSample;
				if (dwStart < pCur->dwEndPos)
					dwStart = pCur->dwEndPos;
				pNode->dwStartPos = dwStart;
			}
		}
		else
		{
			*ppSlot = pNode;
			if ((pNode->PlayParams.dwFlags & 4) != 0)
				pNode->dwStartPos = dwPos;
			else
				pNode->dwStartPos = g_pCurSession->dwCurrentSample;
		}
		if (g_pCurSession->dwCurrentSample > pNode->dwStartPos)
			bQueued = 1;
SetStop:
		if (pNode->PlayParams.wLoops == 0xFFFF)
			pNode->dwEndPos = 0xFFFFFFFF;
		else
			pNode->dwEndPos = pNode->dwNumSamples *
				(pNode->PlayParams.wLoops + 1) -
				g_pCurSession->pcm.wf.nBlockAlign +
				pNode->dwStartPos;
	}
	if (bQueued != 0)
	{
		if (g_pCurSession->pfnRemix(
				dwPos, 0) != 0)
			goto Success;
	}
	nRetry = 0;
	if (g_pPlayQueueHead == NULL && (int)g_pCurSession->dwPauseBlocks > 0)
	{
		waveOutPause(g_pCurSession->hWaveOut);
		bPaused = 1;
	}
	for (;;)
	{
		// Same block scan as WaveMixActivate: pointer walk, "i <
		// nCount" guard kept on the counter, "i >= nCount" for the
		// out-of-blocks arm.
		i = 0;
		nCount = g_pCurSession->dwWaveBlockCount;
		ppBlocks = g_pCurSession->ppWaveBlocks;
		while (i < nCount && (*ppBlocks)->fAvailable == 0)
		{
			ppBlocks++;
			i++;
		}
		if (i >= nCount)
		{
			WaveMix_DebugOut(2, "{gwb:out of wav blks}");
			pXWH = NULL;
		}
		else
		{
			pXWH = *ppBlocks;
			pXWH->wh.dwBufferLength = g_pCurSession->dwWaveBlockLen;
			pXWH->wh.lpData = (LPSTR)pXWH->abData;
			pXWH->pSession = g_pActiveSession;
			pXWH->fAvailable = 0;
		}
		if (WaveMix_MixerPlay(pXWH, 1) == 0)
			break;
		if (bPaused == 0)
			continue;
		nRetry++;
		if ((int)g_pCurSession->dwPauseBlocks > nRetry)
			continue;
		waveOutRestart(g_pCurSession->hWaveOut);
		bPaused = 0;
	}
	if (bPaused != 0)
		waveOutRestart(g_pCurSession->hWaveOut);
Success:
	dwPos = 0;
Done:
	g_dwPlayNesting--;
	return (int)dwPos;
}

// Converts the sample data between 8- and 16-bit samples. Returns the new
// GlobalAlloc'd buffer (the input buffer is freed), the input buffer
// itself when no conversion is needed, or NULL on failure.

// hover.exe:0x43AA70
// FUNCTION: HOVER 0x43aa70
void* WaveMix_BitsPerSampleAlign(void* pData, WORD wBitsFrom, WORD wBitsTo,
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
		if (g_bShowDebugDialogs != 0)
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
void* WaveMix_ChannelAlign(void* pData, WORD wChFrom, WORD wChTo, WORD wBytes,
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
		if (g_bShowDebugDialogs != 0)
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

// Averages nRatio consecutive frames of pSrc into one frame of pDst, channel
// by channel (downsampling half of WaveMix_SamplesPerSecAlign). Note the
// 16-bit path never advances pDst between channels in the original.

// hover.exe:0x43AD40
// FUNCTION: HOVER 0x43ad40
void WaveMix_AvgSample(BYTE* pDst, BYTE* pSrc, int nRatio, int nBytes,
	int nChannels)
{
	int nSum;
	int i;
	int j;
	BYTE* pb;
	short* ps;

	// Each channel is averaged with its own walker starting at the
	// channel's first sample (hover.exe keeps pSrc as the channel base and
	// steps it by one sample per channel; it never advances it by the
	// walk).  The 16-bit path writes every channel to the same pDst, as in
	// hover.exe.
	if (nBytes == 1)
	{
		for (i = nChannels; i > 0; i--)
		{
			nSum = 0;
			for (pb = pSrc++, j = nRatio; j > 0; j--, pb += nChannels)
				nSum += *pb - 0x80;
			*pDst++ = (BYTE)(nSum / nRatio - 0x80);
		}
	}
	else
	{
		for (i = nChannels; i > 0; i--)
		{
			nSum = 0;
			for (ps = (short*)pSrc, pSrc += 2, j = nRatio; j > 0; j--, ps += nChannels)
				nSum += *ps;
			*(short*)pDst = (short)(nSum / nRatio);
		}
	}
}

// Interpolates one frame of pSrc out to nRatio frames of pDst, channel by
// channel (upsampling half of WaveMix_SamplesPerSecAlign). Note the 16-bit path
// reads the "next" sample from the destination buffer in the original.

// hover.exe:0x43ADF0
// FUNCTION: HOVER 0x43adf0
void WaveMix_RepSample(BYTE* pDst, BYTE* pSrc, int nRatio, int nBytes,
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
void* WaveMix_SamplesPerSecAlign(void* pData, DWORD dwRateFrom, DWORD dwRateTo,
	WORD wBytes, WORD wChannels, DWORD* pcbData)
{
	void* pNew;
	BYTE* pSrc;
	BYTE* pDst;
	BYTE* p;
	DWORD cbNew;
	DWORD dwFrames;
	// hover.exe computes wChannels*wBytes exactly once (into a register
	// it keeps live for the rest of the function, across both the
	// upsample and downsample branches and their calls) rather than
	// re-multiplying at each use - confirmed with tools/disasm.py
	// 0x43aec0 (ebx set at 0x43af0b, reused unchanged through 0x43b064).
	DWORD dwFrameBytes;
	int nRatio;
	int nSkip;
	int nOut;
	int i;
	int j;

	if (dwRateTo == dwRateFrom)
		return pData;
	dwFrameBytes = (DWORD)wChannels * wBytes;
	dwFrames = *pcbData / dwFrameBytes;
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
	*pcbData = cbNew = dwFrameBytes * nOut;
	pNew = GlobalLock(GlobalAlloc(GMEM_MOVEABLE | GMEM_DDESHARE, cbNew));
	if (pNew == NULL)
	{
		if (g_bShowDebugDialogs != 0)
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
			WaveMix_RepSample(pDst, pSrc, nRatio, wBytes, wChannels);
			pDst += nRatio * dwFrameBytes;
			pSrc += dwFrameBytes;
		}
		for (i = nRatio; i > 0; i--)
		{
			p = pSrc;
			for (j = dwFrameBytes; j > 0; j--)
				*pDst++ = *p++;
		}
	}
	else
	{
		// downsample: average each group of nSkip frames
		i = nOut - 1;
		while (i-- != 0)
		{
			WaveMix_AvgSample(pDst, pSrc, nSkip, wBytes, wChannels);
			pDst += dwFrameBytes;
			pSrc += nSkip * dwFrameBytes;
		}
		for (j = dwFrameBytes; j > 0; j--)
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
void* WaveMix_ConvertWaveFormat(LPWAVEFORMATEX pwfxSession, LPWAVEFORMATEX pwfxWave,
	void* pData, DWORD* pcbData)
{
	void* pConv;
	int nResult;

	if (pwfxWave->nChannels == pwfxSession->nChannels &&
		pwfxWave->nSamplesPerSec == pwfxSession->nSamplesPerSec &&
		((PCMWAVEFORMAT*)pwfxWave)->wBitsPerSample ==
			((PCMWAVEFORMAT*)pwfxSession)->wBitsPerSample)
		return pData;
	pConv = WaveMix_BitsPerSampleAlign(pData,
		((PCMWAVEFORMAT*)pwfxWave)->wBitsPerSample,
		((PCMWAVEFORMAT*)pwfxSession)->wBitsPerSample, pcbData);
	if (pConv == NULL)
		return NULL;
	pConv = WaveMix_ChannelAlign(pConv, pwfxWave->nChannels,
		pwfxSession->nChannels,
		((PCMWAVEFORMAT*)pwfxSession)->wBitsPerSample >> 3, pcbData);
	if (pConv == NULL)
		return NULL;
	nResult = (int)WaveMix_SamplesPerSecAlign(pConv, pwfxWave->nSamplesPerSec,
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
MIXWAVE* __stdcall WaveMixOpenWave(MIXSESSION* pEngine, const char* pszWave,
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
	WaveMix_DebugOut(2, "<WaveMixOpenWave>");
	if (pEngine != NULL && pEngine->wMagic1 == 0x5432 &&
		pEngine->wMagic2 == 0x5432)
	{
		g_pCurSession = pEngine;
	}
	else
	{
		MessageBeep(0xFFFFFFFF);
		wsprintfA(g_szStringBuffer,
			"Invalid session handle 0x%04X passed to WaveMix API",
			pEngine);
		WaveMix_DebugOut(1, g_szStringBuffer);
		MessageBoxA(NULL, g_szStringBuffer, "WavMix32",
			MB_ICONEXCLAMATION);
		g_pCurSession = NULL;
	}
	if (g_pCurSession != NULL)
		uDevice = (WORD)g_pCurSession->dwDeviceID;
	else
		uDevice = 0;
	if (waveOutOpen(&hWaveOut, uDevice,
			(LPWAVEFORMATEX)&g_pCurSession->pcm, 0, 0,
			WAVE_FORMAT_QUERY) != 0)
	{
		if (g_bShowDebugDialogs != 0)
			MessageBoxA(NULL,
				"The waveform device can't play this format.",
				"WavMix32", MB_ICONEXCLAMATION);
		return NULL;
	}
	pWave = (MIXWAVE*)GlobalLock(GlobalAlloc(GMEM_ZEROINIT | GMEM_DDESHARE,
		sizeof(MIXWAVE)));
	if (pWave == NULL)
	{
		if (g_bShowDebugDialogs != 0)
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
				wsprintfA(g_szStringBuffer,
					"Failed to open 'WAVE' resource '%s'.", pszWave);
			else
				wsprintfA(g_szStringBuffer,
					"Failed to open 'WAVE' resource %u.",
					(DWORD)LOWORD(pszWave));
			if (g_bShowDebugDialogs != 0)
				MessageBoxA(NULL, g_szStringBuffer, "WavMix32",
					MB_ICONEXCLAMATION);
			goto Error;
		}
		memset(&mmioInfo, 0, sizeof(mmioInfo));
		mmioInfo.pchBuffer = (HPSTR)LockResource(hResData);
		if (mmioInfo.pchBuffer == NULL)
		{
			if (g_bShowDebugDialogs != 0)
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
			if (g_bShowDebugDialogs != 0)
			{
				wsprintfA(g_szStringBuffer,
					"Failed to open resource, mmioOpen error=%u.\n"
					"May need to make sure resource is marked read-write",
					mmioInfo.wErrorRet);
				MessageBoxA(NULL, g_szStringBuffer, "WavMix32",
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
			if (g_bShowDebugDialogs != 0)
			{
				wsprintfA(g_szStringBuffer,
					"Failed to open memory file, mmioOpen error=%u.\n"
					"May need to make sure memory is read-write",
					mmioInfo.wErrorRet);
				MessageBoxA(NULL, g_szStringBuffer, "WavMix32",
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
			if (g_bShowDebugDialogs != 0)
			{
				wsprintfA(g_szStringBuffer, "Failed to open wave file %s.",
					pszWave);
				MessageBoxA(NULL, g_szStringBuffer, "WavMix32",
					MB_ICONEXCLAMATION);
			}
			goto Error;
		}
	}
	ckRiff.fccType = mmioFOURCC('W', 'A', 'V', 'E');
	if (mmioDescend(hMMio, &ckRiff, NULL, MMIO_FINDRIFF) != 0)
	{
		if (g_bShowDebugDialogs != 0)
			MessageBoxA(NULL, "This is not a WAVE file.", "WavMix32",
				MB_ICONEXCLAMATION);
		goto Error;
	}
	ckChunk.ckid = mmioFOURCC('f', 'm', 't', ' ');
	if (mmioDescend(hMMio, &ckChunk, &ckRiff, MMIO_FINDCHUNK) != 0)
	{
		if (g_bShowDebugDialogs != 0)
			MessageBoxA(NULL, "WAVE file is corrupted.", "WavMix32",
				MB_ICONEXCLAMATION);
		goto Error;
	}
	if (ckChunk.cksize != sizeof(PCMWAVEFORMAT))
		WaveMix_DebugOut(2,
			"format size is not the size of PCMWAVEFORMAT!");
	if (mmioRead(hMMio, (HPSTR)pWave, sizeof(PCMWAVEFORMAT)) !=
		sizeof(PCMWAVEFORMAT))
	{
		if (g_bShowDebugDialogs != 0)
			MessageBoxA(NULL, "Failed to read format chunk.", "WavMix32",
				MB_ICONEXCLAMATION);
		goto Error;
	}
	if (pWave->wfx.wf.wFormatTag != WAVE_FORMAT_PCM)
	{
		if (g_bShowDebugDialogs != 0)
			MessageBoxA(NULL, "The file is not a PCM file.", "WavMix32",
				MB_ICONEXCLAMATION);
		goto Error;
	}
	mmioAscend(hMMio, &ckChunk, 0);
	ckChunk.ckid = mmioFOURCC('d', 'a', 't', 'a');
	if (mmioDescend(hMMio, &ckChunk, &ckRiff, MMIO_FINDCHUNK) != 0)
	{
		if (g_bShowDebugDialogs != 0)
			MessageBoxA(NULL, "WAVE file has no data chunk.", "WavMix32",
				MB_ICONEXCLAMATION);
		goto Error;
	}
	cbData = ckChunk.cksize;
	if (cbData == 0)
	{
		if (g_bShowDebugDialogs != 0)
			MessageBoxA(NULL, "The data chunk has no data.", "WavMix32",
				MB_ICONEXCLAMATION);
		goto Error;
	}
	pData = (BYTE*)GlobalLock(GlobalAlloc(GMEM_MOVEABLE | GMEM_DDESHARE,
		cbData));
	if (pData == NULL)
	{
		if (g_bShowDebugDialogs != 0)
			MessageBoxA(NULL,
				"Unable to allocate memory for waveform data.  "
				"Try making more memory available by closing other applications.",
				"WavMix32", MB_ICONINFORMATION);
		goto Error;
	}
	if (mmioRead(hMMio, (HPSTR)pData, cbData) != (LONG)cbData)
	{
		if (g_bShowDebugDialogs != 0)
			MessageBoxA(NULL, "Failed to read data chunk.", "WavMix32",
				MB_ICONEXCLAMATION);
		goto Error;
	}
	pData = (BYTE*)WaveMix_ConvertWaveFormat(
		(LPWAVEFORMATEX)&g_pCurSession->pcm, (LPWAVEFORMATEX)pWave,
		pData, &cbData);
	if (pData == NULL)
	{
		if (g_bShowDebugDialogs != 0)
			MessageBoxA(NULL, "Failed to convert wave format.", "WavMix32",
				MB_ICONEXCLAMATION);
		goto Error;
	}
	mmioClose(hMMio, 0);
	if (hResData != NULL)
		FreeResource(hResData);
	pWave->wh.lpData = (LPSTR)pData;
	pWave->wh.dwBufferLength = cbData;
	pWave->wh.dwFlags = 0;
	pWave->wh.dwLoops = 0;
	pWave->wh.dwUser = 0;
	pWave->wMagic = 0x5432;
	memmove(pWave, &g_pCurSession->pcm, sizeof(PCMWAVEFORMAT));
	if (HIWORD(pszWave) == 0)
	{
		wsprintfA(pWave->szName, "res#%u", (DWORD)LOWORD(pszWave));
	}
	else
	{
		nLen = lstrlenA(pszWave);
		// keep only the last 15 characters
		if (nLen > (int)sizeof(pWave->szName) - 1)
			nLen -= (int)sizeof(pWave->szName) - 1;
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

// WaveMixFreeWave: closes a wave opened by WaveMixOpenWave. Declared here
// rather than in hover.h because only this module needs it (adding a free
// function to hover.h perturbs every other translation unit; see
// notes/perfect-match.md rule 20).
int __stdcall WaveMixFreeWave(MIXSESSION* pEngine, MIXWAVE* pWave);

// Validates the session (same magic-word check and error reporting as
// WaveMixOpenChannel, except that this one does *not* publish the session in
// g_pCurSession - it keeps it in the parameter instead) and the wave, then
// unlinks from all 16 channel queues every node still pointing at this
// wave, returning those nodes to the g_pFreeChannelNodes free list (the
// WaveMix_FreeWaveBlocks-style "{FreeCNode" expansion, inlined twice), and
// finally releases the sample data and the MIXWAVE itself. Returns 0 on
// success, 5 on an invalid session or wave handle.

// hover.exe:0x43A860
// FUNCTION: HOVER 0x43a860
int __stdcall WaveMixFreeWave(MIXSESSION* pEngine, MIXWAVE* pWave)
{
	CHANNELNODE** ppChannel;
	CHANNELNODE* pNode;
	CHANNELNODE* pNext;
	CHANNELNODE* pLast;
	int bValidWave;
	int i;

	WaveMix_DebugOut(2, "<WaveMixFreeWave>");
	if (!(pEngine != NULL && pEngine->wMagic1 == 0x5432 &&
		pEngine->wMagic2 == 0x5432))
	{
		MessageBeep(0xFFFFFFFF);
		wsprintfA(g_szStringBuffer,
			"Invalid session handle 0x%04X passed to WaveMix API",
			pEngine);
		WaveMix_DebugOut(1, g_szStringBuffer);
		MessageBoxA(NULL, g_szStringBuffer, "WavMix32",
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
	ppChannel = pEngine->apChannel;
	do
	{
		pNode = *ppChannel;
		if (pNode != WMIX_CHANNEL_CLOSED)
		{
			pLast = NULL;
			while (pNode != NULL)
			{
				// compared as integers: as a pointer compare VC 2.2
				// re-rolls the function's register assignment (the
				// three kernel32 imports at the end swap registers)
				if ((DWORD)pNode->lpMixWave == (DWORD)pWave)
				{
					if (pLast == NULL)
					{
						pNext = pNode->pNext;
						pNode = *ppChannel;
						WaveMix_DebugOut(2, "{FreeCNode");
						if (pNode == NULL)	// never taken; the
						{			// list head is this node
							WaveMix_DebugOut(1,
								" FreeCN: NULL parameter!}");
						}
						else
						{
							pNode->pNext = g_pFreeChannelNodes;
							g_pFreeChannelNodes = pNode;
							WaveMix_DebugOut(2, "}");
						}
						*ppChannel = pNext;
						pNode = pNext;
					}
					else
					{
						pLast->pNext = pNode->pNext;
						WaveMix_DebugOut(2, "{FreeCNode");
						if (pNode == NULL)	// never taken; the
						{			// loop condition proved it
							WaveMix_DebugOut(1,
								" FreeCN: NULL parameter!}");
						}
						else
						{
							pNode->pNext = g_pFreeChannelNodes;
							g_pFreeChannelNodes = pNode;
							WaveMix_DebugOut(2, "}");
						}
						pNode = pLast->pNext;
					}
				}
				else
				{
					pLast = pNode;
					pNode = pNode->pNext;
				}
			}
		}
		ppChannel++;
		i--;
	}
	while (i != 0);
	if (pWave->wh.lpData != NULL)
	{
		GlobalUnlock(GlobalHandle(pWave->wh.lpData));
		GlobalFree(GlobalHandle(pWave->wh.lpData));
	}
	pWave->wMagic = 0;
	GlobalUnlock(GlobalHandle(pWave));
	GlobalFree(GlobalHandle(pWave));
	return 0;
}
// MM_WOM_DONE completion, called from CMainFrame::OnWomDone with the
// MIXWAVE the mixer has finished with. Only the slots that open their wave
// on every play (CSoundSlot::m_bOpenOnPlay != 0) register themselves in
// g_mapOpenWaveToSlot, so finding one there means the wave was opened for this
// play and can be closed again. The Lookup return value is ignored: the
// original tests the (pre-zeroed) output instead.

// hover.exe:0x42D760
// FUNCTION: HOVER 0x42d760
void Hover_OnWaveMixDone(void* pMixWave)
{
	CSoundSlot* pSlot;
	void* pWave;

	pWave = pMixWave;
	pSlot = NULL;
	if (g_bUseWaveMix == 0)
		return;
	if (g_pMixSession == NULL)
		return;

	g_mapOpenWaveToSlot.Lookup(pWave, (void*&)pSlot);
	if (pSlot == NULL)
		return;
	g_mapOpenWaveToSlot.RemoveKey(pWave);

	if (pSlot->m_bOpenOnPlay == 0)
		return;
	if (pSlot->m_pMixWave != pWave)
		return;
	WaveMixFreeWave(g_pMixSession, pSlot->m_pMixWave);
	pSlot->m_pMixWave = NULL;
}

// Builds the fixed-point sine and tangent lookup tables used by the
// rendering engine. Entry 0 of each table is left zeroed.

// hover.exe:0x438540
// FUNCTION: HOVER 0x438540
void Hover_BuildTrigTables()
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
// Hover_FixAngleP, returning the 24.8 fixed-point angle (0x1000000 = full
// circle) directly. The ratio divide here is a plain (x << 16) / y, not
// the HOVER_FIXDIVP macro.

// hover.exe:0x4385F0
// FUNCTION: HOVER 0x4385f0
DWORD Hover_FixAngle(long lY, long lX)
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
DWORD* Hover_FixAngleP(DWORD* pdwOut, long lY, long lX)
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
			// "<=", not "==" (same effect here, since the first arm
			// already excludes "<"): this is what lets the compiler
			// reuse the first branch's cmp flags for this one (a
			// single unsigned jbe) instead of emitting a second
			// compare, matching hover.exe and the sibling binary
			// search above (which already spells it this way).
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
DWORD Hover_ApproxDistance(long lX1, long lY1, long lX2, long lY2)
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
void Hover_SplitVelocity(DWORD dwVX, DWORD dwVY, DWORD dwAngle,
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
void Hover_TangentialComponent(DWORD* pdwOut, DWORD dwX, DWORD dwY, DWORD dwAngle)
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
int Hover_AngleInFrontArc(long lA, long lB)
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
void Hover_LoadControls()
{
	UINT uJoyErr;
	JOYINFO joyInfo;

	theApp.m_avkControls[2] = Hover_KeyNameToVK(IDS_DEFKEY_A);
	theApp.m_avkControls[1] = Hover_KeyNameToVK(IDS_DEFKEY_S);
	theApp.m_avkControls[0] = Hover_KeyNameToVK(IDS_DEFKEY_D);
	theApp.m_avkControls[7] = Hover_KeyNameToVK(IDS_DEFKEY_HOME);
	theApp.m_avkControls[3] = Hover_KeyNameToVK(IDS_DEFKEY_UP);
	theApp.m_avkControls[8] = Hover_KeyNameToVK(IDS_DEFKEY_PGUP);
	theApp.m_avkControls[5] = Hover_KeyNameToVK(IDS_DEFKEY_LEFT);
	theApp.m_avkControls[6] = Hover_KeyNameToVK(IDS_DEFKEY_RIGHT);
	theApp.m_avkControls[9] = Hover_KeyNameToVK(IDS_DEFKEY_END);
	theApp.m_avkControls[4] = Hover_KeyNameToVK(IDS_DEFKEY_DOWN);
	theApp.m_avkControls[10] = Hover_KeyNameToVK(IDS_DEFKEY_PGDN);
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

	// No joystick unless both the caps and a position read succeed: a
	// failed joyGetDevCaps clears the flag too (ours used to leave it).
	if (joyGetDevCapsA(0, &theApp.m_joyCaps, sizeof(theApp.m_joyCaps))
			!= JOYERR_NOERROR
		|| (uJoyErr = joyGetPos(0, &joyInfo), theApp.m_bJoystick = 1,
			uJoyErr != JOYERR_NOERROR))
		theApp.m_bJoystick = 0;
	if (theApp.m_bJoystick == 0)
		theApp.m_avkControls[11] = 0;
}

// Initializes the geometry of the six board zone descriptor blocks: zone
// sizes and split-line centers, sub-area and waypoint rectangles, and the
// derived rectangles computed from them. Every paired coordinate write is
// one CPoint- or CSize-valued assignment; VC 2.x cannot forward a constant through an
// aggregate copy, so each one materialises the pair in an 8-byte class
// temporary on the stack and reads it back for the two global stores.
// hover.exe:0x41F540
// FUNCTION: HOVER 0x41f540
void Hover_InitZones()
{
	ZONE_DESCRIPTOR* pZone;
	int i;
	// hover.exe's frame is three dwords: [esp+0x14]/[esp+0x18] are the
	// compiler's own temporary for the CPoint values above and [esp+0x10]
	// is the register allocator's spill slot for the waypoint loop's i+1.
	// Only the temporary is a real C++ object; the spill dword is stood in
	// for here by a volatile scalar, which is what lands at the bottom of
	// the frame and keeps `sub esp,0xc` with the original's offsets.
	volatile int j;

	// Zone 5
	g_aZones[5].m_size.cx = 0x2bc;
	g_aZones[5].m_size.cy = 0x20d;
	g_aZones[5].m_sizeSnap.cx = 0x2710;
	g_aZones[5].m_sizeSnap.cy = 0x2710;
	g_aZones[5].m_sizeDash.cx = 0x2bc;
	g_aZones[5].m_sizeDash.cy = 0x145;
	g_aZones[5].m_y3DTop = 0x39;
	g_aZones[5].m_y3DBottom = 0x1a2;
	SetRect(&g_aZones[5].m_rcTopDashSrc, 0, 0, 0x2bc, 0x73);
	SetRect(&g_aZones[5].m_rcBottomDashSrc, 0, 0x74, 0x2bc, 0x118);
	SetRect(&g_aZones[5].m_rcMap, 0x113, 0x18e, 0x1a6, 0x1fa);
	SetRect(&g_aZones[5].m_rcMirror, 0xd8, 9, 0x1d4, 0x52);
	SetRect(&g_aZones[5].m_rcNeedle, 0x1e6, 0x1aa, 0x239, 0x1f6);
	SetRect(&g_aZones[5].m_rcNeedleSrc, 0x1e6, 0xb6, 0x239, 0x102);
	SetRect(&g_aZones[5].m_arcGauges[0], 0x1d5, 0x1a4, 0x1de, 0x1f8);
	SetRect(&g_aZones[5].m_arcGauges[1], 0x259, 0x1c3, 0x299, 0x1cf);
	SetRect(&g_aZones[5].m_arcGauges[2], 0x259, 0x1e3, 0x299, 0x1ef);
	SetRect(&g_aZones[5].m_arcPodGauges[0], 0x5b, 0x1b8, 0x64, 0x1e0);
	SetRect(&g_aZones[5].m_arcPodGauges[1], 0x9a, 0x1b8, 0xa3, 0x1e0);
	SetRect(&g_aZones[5].m_arcPodGauges[2], 0xdb, 0x1b8, 0xe4, 0x1e0);
	g_aZones[5].m_aptPodCounters[0] = CPoint(0x56, 0x1e4);
	g_aZones[5].m_aptPodCounters[1] = CPoint(0x95, 0x1e4);
	g_aZones[5].m_aptPodCounters[2] = CPoint(0xd6, 0x1e4);
	g_aZones[5].m_anGaugeSegments[0] = 0x15;
	g_aZones[5].m_anGaugeSegments[1] = 0x13;
	g_aZones[5].m_anGaugeSegments[2] = 0x13;
	g_aZones[5].m_anGaugeSegments[3] = 0xa;
	g_aZones[5].m_anGaugeSegments[4] = 0xa;
	g_aZones[5].m_anGaugeSegments[5] = 0xa;
	g_aZones[5].m_sizeScoreDigit = CSize(0x18, 0x29);
	g_aZones[5].m_sizePodDigit = CSize(0xf, 0x14);
	g_aZones[5].m_sizeFlagIcon = CSize(0x16, 0x16);
	g_aZones[5].m_ptPodDigits = CPoint(6, 0x11d);
	g_aZones[5].m_ptScoreDigits = CPoint(0x1ca, 0x11c);
	g_aZones[5].m_ptFlagIcons = CPoint(0xad, 0x11d);
	g_aZones[5].m_ptPlayerFlags = CPoint(0x39, 8);
	g_aZones[5].m_ptRobotFlags = CPoint(0x39, 0x1e);
	g_aZones[5].m_ptScore = CPoint(0x1f2, 8);
	g_aZones[5].m_cyKeyFont = 0xf;
	SetRect(&g_aZones[5].m_rcKeyJump, 0x36, 0x1e5, 0x4e, 0x1f8);
	SetRect(&g_aZones[5].m_rcKeyWall, 0x75, 0x1e5, 0x8d, 0x1f8);
	SetRect(&g_aZones[5].m_rcKeyCloak, 0xb6, 0x1e5, 0xce, 0x1f8);
	g_aZones[5].m_cyMessageFont = 0x20;

	// Zone 4
	g_aZones[4].m_size.cx = 0x280;
	g_aZones[4].m_size.cy = 0x1e0;
	g_aZones[4].m_sizeSnap.cx = (g_aZones[5].m_size.cx + 0x280) / 2;
	g_aZones[4].m_sizeSnap.cy = (g_aZones[5].m_size.cy + 0x1e0) / 2;
	g_aZones[4].m_sizeDash.cx = 0x280;
	g_aZones[4].m_sizeDash.cy = 0x12a;
	g_aZones[4].m_y3DTop = 0x32;
	g_aZones[4].m_y3DBottom = 0x17e;
	SetRect(&g_aZones[4].m_rcTopDashSrc, 0, 0, 0x280, 0x68);
	SetRect(&g_aZones[4].m_rcBottomDashSrc, 0, 0x6a, 0x280, 0x100);
	SetRect(&g_aZones[4].m_rcMap, 0xff, 0x16d, 0x181, 0x1cb);
	SetRect(&g_aZones[4].m_rcMirror, 0xc4, 9, 0x1ac, 0x4b);
	SetRect(&g_aZones[4].m_rcNeedleSrc, 0x1ba, 0xa8, 0x206, 0xef);
	SetRect(&g_aZones[4].m_rcNeedle, 0x1ba, 0x186, 0x206, g_aZones[4].m_rcNeedleSrc.bottom - g_aZones[4].m_rcNeedleSrc.top + 0x186);
	SetRect(&g_aZones[4].m_arcGauges[0], 0x1ad, 0x180, 0x1b5, 0x1cb);
	SetRect(&g_aZones[4].m_arcGauges[1], 0x224, 0x19c, 0x25a, 0x1a5);
	SetRect(&g_aZones[4].m_arcGauges[2], 0x224, 0x1b9, 0x25a, 0x1c2);
	SetRect(&g_aZones[4].m_arcPodGauges[0], 0x54, 0x191, 0x5a, 0x1b6);
	SetRect(&g_aZones[4].m_arcPodGauges[1], 0x8e, 0x191, 0x94, 0x1b6);
	SetRect(&g_aZones[4].m_arcPodGauges[2], 0xc9, 0x191, 0xcf, 0x1b6);
	g_aZones[4].m_aptPodCounters[0] = CPoint(0x4e, 0x1ba);
	g_aZones[4].m_aptPodCounters[1] = CPoint(0x88, 0x1ba);
	g_aZones[4].m_aptPodCounters[2] = CPoint(0xc3, 0x1ba);
	g_aZones[4].m_anGaugeSegments[0] = 0x19;
	g_aZones[4].m_anGaugeSegments[1] = 0x12;
	g_aZones[4].m_anGaugeSegments[2] = 0x12;
	g_aZones[4].m_anGaugeSegments[3] = 9;
	g_aZones[4].m_anGaugeSegments[4] = 9;
	g_aZones[4].m_anGaugeSegments[5] = 9;
	g_aZones[4].m_sizeScoreDigit = CSize(0x16, 0x26);
	g_aZones[4].m_sizePodDigit = CSize(0xc, 0x12);
	g_aZones[4].m_sizeFlagIcon = CSize(0x14, 0x14);
	g_aZones[4].m_ptPodDigits = CPoint(2, 0x104);
	g_aZones[4].m_ptScoreDigits = CPoint(0x19f, 0x102);
	g_aZones[4].m_ptFlagIcons = CPoint(0x9d, 0x10a);
	g_aZones[4].m_ptPlayerFlags = CPoint(0x35, 8);
	g_aZones[4].m_ptRobotFlags = CPoint(0x35, 0x1b);
	g_aZones[4].m_ptScore = CPoint(0x1c7, 8);
	g_aZones[4].m_cyKeyFont = 0xf;
	SetRect(&g_aZones[4].m_rcKeyJump, 0x32, 0x1bb, 0x47, 0x1cb);
	SetRect(&g_aZones[4].m_rcKeyWall, 0x6c, 0x1bb, 0x81, 0x1cb);
	SetRect(&g_aZones[4].m_rcKeyCloak, 0xa7, 0x1bb, 0xbc, 0x1cb);
	g_aZones[4].m_cyMessageFont = 0x1e;

	// Zone 3
	g_aZones[3].m_size.cx = 0x22c;
	g_aZones[3].m_size.cy = 0x1a1;
	g_aZones[3].m_sizeSnap.cx = (g_aZones[4].m_size.cx + 0x22c) / 2;
	g_aZones[3].m_sizeSnap.cy = (g_aZones[4].m_size.cy + 0x1a1) / 2;
	g_aZones[3].m_sizeDash.cx = 0x22c;
	g_aZones[3].m_sizeDash.cy = 0x105;
	g_aZones[3].m_y3DTop = 0x2a;
	g_aZones[3].m_y3DBottom = 0x14a;
	SetRect(&g_aZones[3].m_rcTopDashSrc, 0, 0, 0x22c, 0x5d);
	SetRect(&g_aZones[3].m_rcBottomDashSrc, 0, 0x5e, 0x22c, 0xe0);
	SetRect(&g_aZones[3].m_rcMap, 0xd8, 0x13e, 0x14d, 0x18e);
	SetRect(&g_aZones[3].m_rcMirror, 0xac, 7, 0x171, 0x40);
	SetRect(&g_aZones[3].m_rcNeedleSrc, 0x182, 0x93, 0x1c2, 0xce);
	SetRect(&g_aZones[3].m_rcNeedle, 0x182, 0x152, 0x1c2, g_aZones[3].m_rcNeedleSrc.bottom - g_aZones[3].m_rcNeedleSrc.top + 0x152);
	SetRect(&g_aZones[3].m_arcGauges[0], 0x175, 0x14c, 0x17c, 0x18f);
	SetRect(&g_aZones[3].m_arcGauges[1], 0x1db, 0x166, 0x20b, 0x170);
	SetRect(&g_aZones[3].m_arcGauges[2], 0x1db, 0x180, 0x20b, 0x18a);
	SetRect(&g_aZones[3].m_arcPodGauges[0], 0x49, 0x15c, 0x4e, 0x17e);
	SetRect(&g_aZones[3].m_arcPodGauges[1], 0x7b, 0x15c, 0x80, 0x17e);
	SetRect(&g_aZones[3].m_arcPodGauges[2], 0xae, 0x15c, 0xb3, 0x17e);
	g_aZones[3].m_aptPodCounters[0] = CPoint(0x44, 0x181);
	g_aZones[3].m_aptPodCounters[1] = CPoint(0x76, 0x181);
	g_aZones[3].m_aptPodCounters[2] = CPoint(0xa9, 0x181);
	g_aZones[3].m_anGaugeSegments[0] = 0x16;
	g_aZones[3].m_anGaugeSegments[1] = 0x10;
	g_aZones[3].m_anGaugeSegments[2] = 0x10;
	g_aZones[3].m_anGaugeSegments[3] = 0xb;
	g_aZones[3].m_anGaugeSegments[4] = 0xb;
	g_aZones[3].m_anGaugeSegments[5] = 0xb;
	g_aZones[3].m_sizeScoreDigit = CSize(0x13, 0x21);
	g_aZones[3].m_sizePodDigit = CSize(0xb, 0xf);
	g_aZones[3].m_sizeFlagIcon = CSize(0x11, 0x10);
	g_aZones[3].m_ptPodDigits = CPoint(4, 0xe8);
	g_aZones[3].m_ptScoreDigits = CPoint(0x13f, 0xe3);
	g_aZones[3].m_ptFlagIcons = CPoint(0x82, 0xe7);
	g_aZones[3].m_ptPlayerFlags = CPoint(0x2d, 7);
	g_aZones[3].m_ptRobotFlags = CPoint(0x2d, 0x18);
	g_aZones[3].m_ptScore = CPoint(0x18b, 6);
	g_aZones[3].m_cyKeyFont = 0xe;
	SetRect(&g_aZones[3].m_rcKeyJump, 0x2d, 0x181, 0x3f, 0x18f);
	SetRect(&g_aZones[3].m_rcKeyWall, 0x5f, 0x181, 0x71, 0x18f);
	SetRect(&g_aZones[3].m_rcKeyCloak, 0x92, 0x181, 0xa4, 0x18f);
	g_aZones[3].m_cyMessageFont = 0x1c;

	// Zone 2
	g_aZones[2].m_size.cx = 0x200;
	g_aZones[2].m_size.cy = 0x180;
	g_aZones[2].m_sizeSnap.cx = (g_aZones[3].m_size.cx + 0x200) / 2;
	g_aZones[2].m_sizeSnap.cy = (g_aZones[3].m_size.cy + 0x180) / 2;
	g_aZones[2].m_sizeDash.cx = 0x200;
	g_aZones[2].m_sizeDash.cy = 0xf2;
	g_aZones[2].m_y3DTop = 0x2a;
	g_aZones[2].m_y3DBottom = 0x134;
	SetRect(&g_aZones[2].m_rcTopDashSrc, 0, 0, 0x200, 0x56);
	SetRect(&g_aZones[2].m_rcBottomDashSrc, 0, 0x54, 0x200, 0xcd);
	SetRect(&g_aZones[2].m_rcMap, 0xc9, 0x126, 0x135, 0x171);
	SetRect(&g_aZones[2].m_rcMirror, 0x9c, 6, 0x156, 0x3c);
	SetRect(&g_aZones[2].m_rcNeedleSrc, 0x162, 0x85, 0x1a1, 0xbd);
	SetRect(&g_aZones[2].m_rcNeedle, 0x162, 0x135, 0x1a1, g_aZones[2].m_rcNeedleSrc.bottom - g_aZones[2].m_rcNeedleSrc.top + 0x135);
	SetRect(&g_aZones[2].m_arcGauges[0], 0x157, 0x130, 0x15e, 0x16f);
	SetRect(&g_aZones[2].m_arcGauges[1], 0x1b7, 0x149, 0x1e5, 0x151);
	SetRect(&g_aZones[2].m_arcGauges[2], 0x1b7, 0x161, 0x1e5, 0x169);
	SetRect(&g_aZones[2].m_arcPodGauges[0], 0x43, 0x13f, 0x48, 0x15d);
	SetRect(&g_aZones[2].m_arcPodGauges[1], 0x71, 0x13f, 0x76, 0x15d);
	SetRect(&g_aZones[2].m_arcPodGauges[2], 0xa0, 0x13f, 0xa5, 0x15d);
	g_aZones[2].m_aptPodCounters[0] = CPoint(0x3e, 0x161);
	g_aZones[2].m_aptPodCounters[1] = CPoint(0x6c, 0x161);
	g_aZones[2].m_aptPodCounters[2] = CPoint(0x9b, 0x161);
	g_aZones[2].m_anGaugeSegments[0] = 0x14;
	g_aZones[2].m_anGaugeSegments[1] = 0xe;
	g_aZones[2].m_anGaugeSegments[2] = 0xe;
	g_aZones[2].m_anGaugeSegments[3] = 9;
	g_aZones[2].m_anGaugeSegments[4] = 9;
	g_aZones[2].m_anGaugeSegments[5] = 9;
	g_aZones[2].m_sizeScoreDigit = CSize(0x12, 0x1e);
	g_aZones[2].m_sizePodDigit = CSize(0xb, 0xf);
	g_aZones[2].m_sizeFlagIcon = CSize(0x10, 0x10);
	g_aZones[2].m_ptPodDigits = CPoint(6, 0xd2);
	g_aZones[2].m_ptScoreDigits = CPoint(0x12e, 0xd1);
	g_aZones[2].m_ptFlagIcons = CPoint(0x7e, 0xd3);
	g_aZones[2].m_ptPlayerFlags = CPoint(0x2a, 6);
	g_aZones[2].m_ptRobotFlags = CPoint(0x2a, 0x16);
	g_aZones[2].m_ptScore = CPoint(0x16b, 6);
	g_aZones[2].m_cyKeyFont = 0xf;
	SetRect(&g_aZones[2].m_rcKeyJump, 0x2a, 0x162, 0x39, 0x16e);
	SetRect(&g_aZones[2].m_rcKeyWall, 0x58, 0x162, 0x67, 0x16e);
	SetRect(&g_aZones[2].m_rcKeyCloak, 0x87, 0x162, 0x96, 0x16e);
	g_aZones[2].m_cyMessageFont = 0x19;

	// Zone 1
	g_aZones[1].m_size.cx = 0x1a0;
	g_aZones[1].m_size.cy = 0x138;
	g_aZones[1].m_sizeSnap.cx = (g_aZones[2].m_size.cx + 0x1a0) / 2;
	g_aZones[1].m_sizeSnap.cy = (g_aZones[2].m_size.cy + 0x138) / 2;
	g_aZones[1].m_sizeDash.cx = 0x1a0;
	g_aZones[1].m_sizeDash.cy = 0xc4;
	g_aZones[1].m_y3DTop = 0x21;
	g_aZones[1].m_y3DBottom = 0xf6;
	SetRect(&g_aZones[1].m_rcTopDashSrc, 0, 0, 0x1a0, 0x44);
	SetRect(&g_aZones[1].m_rcBottomDashSrc, 0, 0x44, 0x1a0, 0xa6);
	SetRect(&g_aZones[1].m_rcMap, 0xa3, 0xec, 0xfc, 0x128);
	SetRect(&g_aZones[1].m_rcMirror, 0x84, 5, 0x114, 0x30);
	SetRect(&g_aZones[1].m_rcNeedleSrc, 0x122, 0x6c, 0x151, 0x98);
	SetRect(&g_aZones[1].m_rcNeedle, 0x122, 0xfd, 0x151, g_aZones[1].m_rcNeedleSrc.bottom - g_aZones[1].m_rcNeedleSrc.top + 0xfd);
	SetRect(&g_aZones[1].m_arcGauges[0], 0x117, 0xfa, 0x11c, 0x12a);
	SetRect(&g_aZones[1].m_arcGauges[1], 0x167, 0x10c, 0x188, 0x111);
	SetRect(&g_aZones[1].m_arcGauges[2], 0x167, 0x11f, 0x188, 0x124);
	SetRect(&g_aZones[1].m_arcPodGauges[0], 0x37, 0x103, 0x3b, 0x11b);
	SetRect(&g_aZones[1].m_arcPodGauges[1], 0x5d, 0x103, 0x61, 0x11b);
	SetRect(&g_aZones[1].m_arcPodGauges[2], 0x83, 0x103, 0x87, 0x11b);
	g_aZones[1].m_aptPodCounters[0] = CPoint(0x33, 0x11f);
	g_aZones[1].m_aptPodCounters[1] = CPoint(0x59, 0x11f);
	g_aZones[1].m_aptPodCounters[2] = CPoint(0x7f, 0x11f);
	g_aZones[1].m_anGaugeSegments[0] = 0x10;
	g_aZones[1].m_anGaugeSegments[1] = 0xb;
	g_aZones[1].m_anGaugeSegments[2] = 0xb;
	g_aZones[1].m_anGaugeSegments[3] = 7;
	g_aZones[1].m_anGaugeSegments[4] = 7;
	g_aZones[1].m_anGaugeSegments[5] = 7;
	g_aZones[1].m_sizeScoreDigit = CSize(0xe, 0x19);
	g_aZones[1].m_sizePodDigit = CSize(8, 0xc);
	g_aZones[1].m_sizeFlagIcon = CSize(0xd, 0xd);
	g_aZones[1].m_ptPodDigits = CPoint(3, 0xa9);
	g_aZones[1].m_ptScoreDigits = CPoint(0x111, 0xa9);
	g_aZones[1].m_ptFlagIcons = CPoint(0x5e, 0xac);
	g_aZones[1].m_ptPlayerFlags = CPoint(0x22, 5);
	g_aZones[1].m_ptRobotFlags = CPoint(0x22, 0x12);
	g_aZones[1].m_ptScore = CPoint(0x128, 5);
	g_aZones[1].m_cyKeyFont = 0xd;
	SetRect(&g_aZones[1].m_rcKeyJump, 0x21, 0x11f, 0x2e, 0x129);
	SetRect(&g_aZones[1].m_rcKeyWall, 0x48, 0x11f, 0x54, 0x129);
	SetRect(&g_aZones[1].m_rcKeyCloak, 0x6e, 0x11f, 0x7a, 0x129);
	g_aZones[1].m_cyMessageFont = 0x16;

	// Zone 0
	g_aZones[0].m_size.cx = 0x140;
	g_aZones[0].m_size.cy = 0xf0;
	g_aZones[0].m_sizeSnap.cx = (g_aZones[1].m_size.cx + 0x140) / 2;
	g_aZones[0].m_sizeSnap.cy = (g_aZones[1].m_size.cy + 0xf0) / 2;
	g_aZones[0].m_sizeDash.cx = 0x140;
	g_aZones[0].m_sizeDash.cy = 0x95;
	g_aZones[0].m_y3DTop = 0x19;
	g_aZones[0].m_y3DBottom = 0xbd;
	SetRect(&g_aZones[0].m_rcTopDashSrc, 0, 0, 0x140, 0x35);
	SetRect(&g_aZones[0].m_rcBottomDashSrc, 0, 0x33, 0x140, 0x7f);
	SetRect(&g_aZones[0].m_rcMap, 0x7f, 0xb6, 0xc0, 0xe4);
	SetRect(&g_aZones[0].m_rcMirror, 0x64, 4, 0xd4, 0x25);
	SetRect(&g_aZones[0].m_rcNeedleSrc, 0xde, 0x52, 0x105, 0x74);
	SetRect(&g_aZones[0].m_rcNeedle, 0xde, 0xc2, 0x105, g_aZones[0].m_rcNeedleSrc.bottom - g_aZones[0].m_rcNeedleSrc.top + 0xc2);
	SetRect(&g_aZones[0].m_arcGauges[0], 0xd6, 0xba, 0xdb, 0xe6);
	SetRect(&g_aZones[0].m_arcGauges[1], 0x111, 0xce, 0x12c, 0xd2);
	SetRect(&g_aZones[0].m_arcGauges[2], 0x111, 0xdd, 0x12c, 0xe1);
	SetRect(&g_aZones[0].m_arcPodGauges[0], 0x27, 0xc9, 0x2c, 0xe2);
	SetRect(&g_aZones[0].m_arcPodGauges[1], 0x44, 0xc9, 0x49, 0xe2);
	SetRect(&g_aZones[0].m_arcPodGauges[2], 0x61, 0xc9, 0x66, 0xe2);
	g_aZones[0].m_aptPodCounters[0] = CPoint(0x1a, 0xcc);
	g_aZones[0].m_aptPodCounters[1] = CPoint(0x37, 0xcc);
	g_aZones[0].m_aptPodCounters[2] = CPoint(0x54, 0xcc);
	g_aZones[0].m_anGaugeSegments[0] = 0xd;
	g_aZones[0].m_anGaugeSegments[1] = 9;
	g_aZones[0].m_anGaugeSegments[2] = 9;
	g_aZones[0].m_anGaugeSegments[3] = 6;
	g_aZones[0].m_anGaugeSegments[4] = 6;
	g_aZones[0].m_anGaugeSegments[5] = 6;
	g_aZones[0].m_sizeScoreDigit = CSize(0xb, 0x13);
	g_aZones[0].m_sizePodDigit = CSize(8, 0xc);
	g_aZones[0].m_sizeFlagIcon = CSize(0xa, 0xb);
	g_aZones[0].m_ptPodDigits = CPoint(1, 0x81);
	g_aZones[0].m_ptScoreDigits = CPoint(0xbc, 0x81);
	g_aZones[0].m_ptFlagIcons = CPoint(0x5a, 0x81);
	g_aZones[0].m_ptPlayerFlags = CPoint(0x1a, 3);
	g_aZones[0].m_ptRobotFlags = CPoint(0x1a, 0xe);
	g_aZones[0].m_ptScore = CPoint(0xe4, 4);
	g_aZones[0].m_cyKeyFont = 0xd;
	SetRect(&g_aZones[0].m_rcKeyJump, 0x1b, 0xda, 0x24, 0xe5);
	SetRect(&g_aZones[0].m_rcKeyWall, 0x38, 0xda, 0x41, 0xe5);
	SetRect(&g_aZones[0].m_rcKeyCloak, 0x55, 0xda, 0x5e, 0xe5);
	g_aZones[0].m_cyMessageFont = 0x14;

	// Derived per-zone rectangles: the two dashboard strips' on-screen
	// rectangles (the top one at the top of the window, the bottom one
	// against its bottom edge), the rows sampled for their edges, and the
	// four flag icon tiles side by side.
	for (pZone = g_aZones; pZone < g_aZones + 6; pZone++)
	{
		pZone->m_rcBottomDashDst = pZone->m_rcBottomDashSrc;
		pZone->m_rcBottomDashDst.top = pZone->m_rcBottomDashSrc.top
			- pZone->m_rcBottomDashSrc.bottom + pZone->m_size.cy - 1;
		pZone->m_rcBottomDashDst.bottom = pZone->m_size.cy - 1;
		pZone->m_rcTopDashDst = pZone->m_rcTopDashSrc;
		pZone->m_rcTopDashDst.bottom = pZone->m_rcTopDashSrc.bottom
			- pZone->m_rcTopDashSrc.top - 1;
		pZone->m_rcTopDashDst.top = 0;
		pZone->m_yTopDashEdge = pZone->m_rcTopDashDst.bottom + 1;
		pZone->m_yBottomDashEdge = pZone->m_rcBottomDashDst.top - 1;
		SetRect(&pZone->m_arcFlagIcons[0], pZone->m_ptFlagIcons.x,
			pZone->m_ptFlagIcons.y,
			pZone->m_sizeFlagIcon.cx + pZone->m_ptFlagIcons.x,
			pZone->m_sizeFlagIcon.cx + pZone->m_ptFlagIcons.y);
		SetRect(&pZone->m_arcFlagIcons[1],
			pZone->m_sizeFlagIcon.cx + pZone->m_arcFlagIcons[0].left,
			pZone->m_arcFlagIcons[0].top,
			pZone->m_sizeFlagIcon.cx + pZone->m_arcFlagIcons[0].right,
			pZone->m_arcFlagIcons[0].bottom);
		SetRect(&pZone->m_arcFlagIcons[2],
			pZone->m_sizeFlagIcon.cx + pZone->m_arcFlagIcons[1].left,
			pZone->m_arcFlagIcons[0].top,
			pZone->m_sizeFlagIcon.cx + pZone->m_arcFlagIcons[1].right,
			pZone->m_arcFlagIcons[0].bottom);
		SetRect(&pZone->m_arcFlagIcons[3],
			pZone->m_sizeFlagIcon.cx + pZone->m_arcFlagIcons[2].left,
			pZone->m_arcFlagIcons[0].top,
			pZone->m_sizeFlagIcon.cx + pZone->m_arcFlagIcons[2].right,
			pZone->m_arcFlagIcons[0].bottom);
	}

	// The digit tiles: a row of ten for the pod counters and a row of ten
	// for the score
	for (pZone = g_aZones; pZone < g_aZones + 6; pZone++)
	{
		for (i = 0; i < 10; i++)
		{
			j = i + 1;
			SetRect(&pZone->m_arcPodDigits[i],
				pZone->m_ptPodDigits.x + i * pZone->m_sizePodDigit.cx,
				pZone->m_ptPodDigits.y,
				pZone->m_ptPodDigits.x + j * pZone->m_sizePodDigit.cx,
				pZone->m_sizePodDigit.cy + pZone->m_ptPodDigits.y);
			// (Not a bug, despite appearances: hover.exe:0x420d33
			// reads j as [esp+0x14], but the SetRect bottom argument
			// has already been pushed at 0x420d2a, so esp is four
			// lower there and the slot really is [esp+0x10].)
			SetRect(&pZone->m_arcScoreDigits[i],
				pZone->m_ptScoreDigits.x + i * pZone->m_sizeScoreDigit.cx,
				pZone->m_ptScoreDigits.y,
				pZone->m_ptScoreDigits.x + j * pZone->m_sizeScoreDigit.cx,
				pZone->m_sizeScoreDigit.cy + pZone->m_ptScoreDigits.y);
		}
	}

	// The flag icons again, the first three now with their real height
	for (pZone = g_aZones; pZone < g_aZones + 6; pZone++)
	{
		for (i = 0; i < 3; i++)
		{
			j = i + 1;
			SetRect(&pZone->m_arcFlagIcons[i],
				pZone->m_ptFlagIcons.x + i * pZone->m_sizeFlagIcon.cx,
				pZone->m_ptFlagIcons.y,
				pZone->m_ptFlagIcons.x + j * pZone->m_sizeFlagIcon.cx,
				pZone->m_sizeFlagIcon.cy + pZone->m_ptFlagIcons.y);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
// Key binding name resolution


// Hover_KeyNameToVK and its key name table (hover.exe:0x42DC50 /
// 0x4BDAC8) live in optionsdlg.cpp next to the Player Controls dialog
// that shares them; in the original they are in the same translation
// unit (0x42DC50 sits between the dialog's AreKeysUnique and FillKeyCombo).


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
void Hover_LoadHumanParams()
{
	HKEY hKey;
	if (RegOpenKeyExA(HKEY_CURRENT_USER, "Software\\Microsoft\\Hover!",
			0, KEY_ALL_ACCESS, &hKey) != ERROR_SUCCESS)
		return;

	DWORD dwType = REG_BINARY;
	// Original quirk: cbData is not reset between the three reads, so a
	// short first value also shortens the next two.
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
void Hover_LoadRobotParams()
{
	HKEY hKey;
	if (RegOpenKeyExA(HKEY_CURRENT_USER, "Software\\Microsoft\\Hover!",
			0, KEY_ALL_ACCESS, &hKey) != ERROR_SUCCESS)
		return;

	DWORD dwType = REG_BINARY;
	// cbData is not reset between reads (see Hover_LoadHumanParams).
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
void Hover_LoadRobotAIParams()
{
	HKEY hKey;
	if (RegOpenKeyExA(HKEY_CURRENT_USER, "Software\\Microsoft\\Hover!",
			0, KEY_ALL_ACCESS, &hKey) != ERROR_SUCCESS)
		return;

	DWORD dwType = REG_BINARY;
	// cbData is not reset between reads (see Hover_LoadHumanParams).
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
// (m_dwSkill: 0 = beginner, 1 = intermediate, 2 = expert).

// hover.exe:0x425870
// FUNCTION: HOVER 0x425870
void CHoverApp::SelectSkillParams()
{
	switch (m_dwSkill)
	{
	case 0:
		m_pHumanParams = &g_HBegParams;
		m_pRobotParams = &g_RBegParams;
		m_pRobotAIParams = &g_RobotAIBegParams;
		break;
	case 1:
		m_pHumanParams = &g_HIntParams;
		m_pRobotParams = &g_RIntParams;
		m_pRobotAIParams = &g_RobotAIIntParams;
		break;
	case 2:
		m_pHumanParams = &g_HExpParams;
		m_pRobotParams = &g_RExpParams;
		m_pRobotAIParams = &g_RobotAIExpParams;
		break;
	}
}

/////////////////////////////////////////////////////////////////////////////
// Worker thread and engine object shutdown (called by
// CHoverApp::ExitInstance)

// hover.exe:0x422E30
// FUNCTION: HOVER 0x422e30
void Hover_ShutdownSound()
{
	if (g_bUseWaveMix == 0 && g_pSoundThread != NULL)
	{
		EnterCriticalSection(&g_csSoundQueue);
		g_bSoundThreadQuit = 1;
		LeaveCriticalSection(&g_csSoundQueue);
		ResetEvent(g_hSoundAckEvent);
		SetEvent(g_hSoundWakeEvent);
		WaitForSingleObject(g_hSoundAckEvent, INFINITE);
		g_pSoundThread = NULL;
		if (g_hSoundWakeEvent != NULL && CloseHandle(g_hSoundWakeEvent))
			g_hSoundWakeEvent = NULL;
		if (g_hSoundAckEvent != NULL && CloseHandle(g_hSoundAckEvent))
			g_hSoundAckEvent = NULL;
		if (g_bSoundQueueCsInit == 1)
		{
			DeleteCriticalSection(&g_csSoundQueue);
			g_bSoundQueueCsInit = 0;
		}
	}

	for (int i = 0; i < g_arrUnk_4A47F0.GetSize(); i++)
		if (g_arrUnk_4A47F0[i] != NULL)
			delete g_arrUnk_4A47F0[i];

	if (g_pMixSession != NULL)
	{
		WaveMixCloseChannel(g_pMixSession, 1, 1);
		WaveMixCloseSession(g_pMixSession);
		g_pMixSession = NULL;
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
void Hover_FreeRegionNodePool()
{
	POSITION pos = g_listRegionNodeBlocks.GetHeadPosition();
	while (pos != NULL)
		delete (BYTE*)g_listRegionNodeBlocks.GetNext(pos);
	g_listRegionNodeBlocks.RemoveAll();
	g_pRegionNodeFree = 0;
}

// hover.exe:0x42C590
// FUNCTION: HOVER 0x42c590
void Hover_FreeWallPool()
{
	POSITION pos = g_listWallBlocks.GetHeadPosition();
	while (pos != NULL)
		delete (BYTE*)g_listWallBlocks.GetNext(pos);
	g_listWallBlocks.RemoveAll();
	g_pWallFree = 0;
}

// hover.exe:0x42C2D0
// FUNCTION: HOVER 0x42c2d0
void Hover_FreeRobotPool()
{
	POSITION pos = g_listRobotBlocks.GetHeadPosition();
	while (pos != NULL)
		delete (BYTE*)g_listRobotBlocks.GetNext(pos);
	g_listRobotBlocks.RemoveAll();
	g_pRobotFree = 0;
}

// hover.exe:0x42AED0
// FUNCTION: HOVER 0x42aed0
void Hover_FreePodObjPool()
{
	POSITION pos = g_listPodObjBlocks.GetHeadPosition();
	while (pos != NULL)
		delete (BYTE*)g_listPodObjBlocks.GetNext(pos);
	g_listPodObjBlocks.RemoveAll();
	g_pPodObjFree = 0;
}

// hover.exe:0x42B710
// FUNCTION: HOVER 0x42b710
void Hover_FreeFloorObjPool()
{
	POSITION pos = g_listFloorObjBlocks.GetHeadPosition();
	while (pos != NULL)
		delete (BYTE*)g_listFloorObjBlocks.GetNext(pos);
	g_listFloorObjBlocks.RemoveAll();
	g_pFloorObjFree = 0;
}

// hover.exe:0x42F050
// FUNCTION: HOVER 0x42f050
void Hover_FreeBeaconPool()
{
	POSITION pos = g_listBeaconBlocks.GetHeadPosition();
	while (pos != NULL)
		delete (BYTE*)g_listBeaconBlocks.GetNext(pos);
	g_listBeaconBlocks.RemoveAll();
	g_pBeaconFree = 0;
}

// hover.exe:0x42CB70
// FUNCTION: HOVER 0x42cb70
void Hover_FreeMerlinDynamicPool()
{
	POSITION pos = g_listMerlinDynamicBlocks.GetHeadPosition();
	while (pos != NULL)
		delete (BYTE*)g_listMerlinDynamicBlocks.GetNext(pos);
	g_listMerlinDynamicBlocks.RemoveAll();
	g_pMerlinDynamicFree = 0;
}

// hover.exe:0x41C060
// FUNCTION: HOVER 0x41c060
int CHoverApp::ExitInstance()
{
	DeleteCriticalSection(&m_csScene);
	DeleteCriticalSection(&m_csOffscreen);
	DeleteCriticalSection(&m_csUnused);
	DeleteCriticalSection(&m_csWorkerCmd);
	Hover_ShutdownSound();
	Hover_SaveGameSettings();
	Hover_FreeRegionNodePool();
	Hover_FreeWallPool();
	Hover_FreeRobotPool();
	Hover_FreePodObjPool();
	Hover_FreeFloorObjPool();
	Hover_FreeBeaconPool();
	Hover_FreeMerlinDynamicPool();
	if (g_pUnk_4C4738 != NULL)
		delete g_pUnk_4C4738;
	delete (BYTE*)m_pbmiDisplayDib;
	delete (BYTE*)g_pTextureHeap;
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
BOOL CHoverApp::CheckMinimumConfig()
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
BOOL CHoverApp::CheckSingleInstance()
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
void CHoverApp::FindDataDirectory()
{
	// Locate the game data directory. Prefer the current directory if
	// it contains mazes\small.maz; otherwise fall back to the
	// directory containing the executable.
	GetCurrentDirectoryA(MAX_PATH, m_strDataDir.GetBufferSetLength(MAX_PATH));
	m_strDataDir.ReleaseBuffer();
	m_strDataDir += '\\';

	// The original compares signed here (jge in the disassembly).
	// Original bug: the limit allows a 0x103-byte path into the 0x100
	// szPath below (0x10 is the length of "mazes\small.maz" with its
	// terminator).
	if (m_strDataDir.GetLength() + 0x10 < MAX_PATH)
	{
		char szPath[0x100];
		OFSTRUCT of;
		lstrcpy(szPath, m_strDataDir);
		lstrcat(szPath, "mazes\\small.maz");
		if (OpenFile(szPath, &of, OF_EXIST) != (HFILE)-1)
			return;
	}

	GetModuleFileNameA(NULL, m_strDataDir.GetBufferSetLength(MAX_PATH), MAX_PATH);
	m_strDataDir.ReleaseBuffer();

	char szDrive[_MAX_DRIVE], szDir[_MAX_DIR], szFname[_MAX_FNAME], szExt[_MAX_EXT];
	_splitpath(m_strDataDir, szDrive, szDir, szFname, szExt);
	m_strDataDir = szDrive;
	m_strDataDir += szDir;
}

/////////////////////////////////////////////////////////////////////////////
// Full-screen mode switching and the game-start sequence

// The "Please Wait" dialog CHoverApp::StartLevel puts up while a level
// loads. The constructor is bare: the CStatic member and the CDialog base
// do all the work.

// hover.exe:0x424720
// FUNCTION: HOVER 0x424720
CLoadingDialog::CLoadingDialog(CWnd* pParent /*=NULL*/)
	: CDialog(IDD_LOADING, pParent)
{
}

// Starts a level: puts up the modeless "Please Wait" dialog, installs the
// texture load progress callback, throws away the old document contents
// and lets CWinApp::OnFileNew build the new ones (which is where the level
// is actually loaded), then retitles the frame "Hover! - Level n" and
// takes the dialog down again. m_dwGameState marks a game as in progress.
//
// Note the original never checks the CDialog allocation: `new` returning
// NULL would take Create() through a NULL this. hWndDlg only exists to
// pull the m_hWnd load above the three global stores, where the original
// has it (notes/perfect-match.md rule 25).

// hover.exe:0x41C8E0
// FUNCTION: HOVER 0x41c8e0
void CHoverApp::StartLevel()
{
	CLoadingDialog* pDlg;
	HWND hWndDlg;
	char szFormat[40];
	char szTitle[40];

	m_dwGameState = 1;
	pDlg = new CLoadingDialog(NULL);
	pDlg->Create(IDD_LOADING, NULL);
	hWndDlg = pDlg->m_hWnd;
	g_pLoadingDlg = pDlg;
	g_dwUnk_4C4DA8 = 0;
	g_pfnUnk_4C4DAC = Hover_LoadProgress;
	::UpdateWindow(hWndDlg);

	theApp.m_pBumperDoc->DeleteContents();
	theApp.m_pBumperDoc->m_bAttractMode = 0;
	CWinApp::OnFileNew();

	::LoadString(AfxGetInstanceHandle(), IDS_LEVEL_TITLE, szFormat,
		sizeof(szFormat));
	wsprintf(szTitle, szFormat, g_dwLevel + 1);
	AfxGetApp()->m_pMainWnd->SetWindowText(szTitle);

	g_pfnUnk_4C4DAC = NULL;
	pDlg->DestroyWindow();
	delete pDlg;
	g_pLoadingDlg = NULL;
}

// Switches the game into its 640x480 full-screen mode: the frame's window
// state (style, extended style, position and menu) is saved so that
// LeaveFullScreen can put it back, the frame becomes a plain WS_POPUP without
// the status bar and the view loses its client edge. A borderless topmost
// "Temp" window covers the screen while ChangeDisplaySettings runs so that
// the mode switch is not visible. Returns whether the mode switch worked.

// hover.exe:0x4253D0
// FUNCTION: HOVER 0x4253d0
BOOL CHoverApp::EnterFullScreen()
{
	CHoverApp* pApp = this;
	CBumperView* pView;
	int cx;
	int cy;

	HWND hWnd = pApp->m_pMainWnd->m_hWnd;
	pApp->m_bSwitchingMode = 1;
	pApp->m_dwSavedStyle = ::GetWindowLong(hWnd, GWL_STYLE);
	pApp->m_dwSavedExStyle = ::GetWindowLong(pApp->m_pMainWnd->m_hWnd, GWL_EXSTYLE);
	::GetWindowRect(pApp->m_pMainWnd->m_hWnd, &pApp->m_rcSavedWindow);
	pApp->m_hSavedMenu = pApp->m_pMainWnd->GetMenu()->m_hMenu;
	pApp->m_pMainWnd->ModifyStyleEx(pApp->m_dwSavedExStyle, 0, 0);
	pApp->m_pMainWnd->ModifyStyle(pApp->m_dwSavedStyle, WS_POPUP | WS_VISIBLE,
		SWP_FRAMECHANGED);
	((CMainFrame*)pApp->m_pMainWnd)->m_wndStatusBar.ShowWindow(SW_HIDE);

	pView = Hover_GetView();
	pView->ModifyStyleEx(WS_EX_CLIENTEDGE, 0, 0);
	pView->ModifyStyle(WS_BORDER, 0, SWP_FRAMECHANGED);

	cx = 640;
	pApp->m_bDisplayDibActive = 0;
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
	pApp->m_bFullScreen =
		(pApp->m_pfnChangeDisplaySettings(&dm, CDS_FULLSCREEN) == 0);
	if (pApp->m_bFullScreen != 0)
	{
		if (pApp->m_bFullScreenTopmost != 0)
			pApp->m_pMainWnd->SetWindowPos(&CWnd::wndTopMost,
				0, 0, cx, cy, 0);
		else
			pApp->m_pMainWnd->SetWindowPos(&CWnd::wndNoTopMost,
				0, 0, cx, cy, 0);
	}

	wndCover.DestroyWindow();
	pApp->m_bSwitchingMode = 0;
	return pApp->m_bFullScreen;
}

// Leaves full-screen mode again: restores the display mode and everything
// EnterFullScreen saved. Returns whether the mode switch worked.

// hover.exe:0x425600
// FUNCTION: HOVER 0x425600
BOOL CHoverApp::LeaveFullScreen()
{
	BOOL bOk;
	CBumperView* pView;

	bOk = (m_pfnChangeDisplaySettings(NULL, CDS_FULLSCREEN) == 0);
	m_bFullScreen = 0;
	m_pMainWnd->ModifyStyleEx(0, m_dwSavedExStyle, 0);
	m_pMainWnd->ModifyStyle(WS_POPUP | WS_VISIBLE, m_dwSavedStyle,
		SWP_FRAMECHANGED);
	if (((CMainFrame*)m_pMainWnd)->m_bAutoStatusBar == 0)
		((CMainFrame*)m_pMainWnd)->m_wndStatusBar.ShowWindow(SW_SHOW);

	pView = Hover_GetView();
	pView->m_bDoubleSize = 0;
	pView->ModifyStyleEx(0, WS_EX_CLIENTEDGE, 0);
	pView->ModifyStyle(0, WS_BORDER, SWP_FRAMECHANGED);

	m_pMainWnd->SetMenu(CMenu::FromHandle(m_hSavedMenu));
	m_pMainWnd->SetWindowPos(&CWnd::wndNoTopMost,
		m_rcSavedWindow.left, m_rcSavedWindow.top,
		m_rcSavedWindow.right - m_rcSavedWindow.left,
		m_rcSavedWindow.bottom - m_rcSavedWindow.top, 0);
	m_pMainWnd->UpdateWindow();
	m_bDisplayDibActive = 0;
	return bOk;
}

/////////////////////////////////////////////////////////////////////////////
// CHoverApp message handlers
//
// The application object owns the game commands: the frame window has no
// handlers of its own, so every menu and accelerator command lands here
// through the command routing chain. Handlers that open a modal dialog
// first push the game into its "paused" state with
// CBumperView::SuspendGame(1).

// Standard Help/About: the game does not use the About dialog template
// (100) but the shell's ShellAbout box with the application icon.

// hover.exe:0x425390
// FUNCTION: HOVER 0x425390
void CHoverApp::OnAppAbout()
{
	Hover_GetView()->SuspendGame(1);
	ShellAbout(AfxGetApp()->m_pMainWnd->m_hWnd, "Hover!", "",
		::LoadIcon(AfxGetResourceHandle(), MAKEINTRESOURCE(IDR_MAINFRAME)));
}

// ID_DBG_PODTIMES (debug): edits the five pod effect durations and the
// three floating-point tuning values of the application object in a
// dialog whose template (130) was stripped from the shipped resources.

// hover.exe:0x425730
// FUNCTION: HOVER 0x425730
void CHoverApp::OnDbgPodTimes()
{
	CPodTimesDialog dlg(NULL);

	Hover_GetView()->SuspendGame(1);

	dlg.m_nInvincibleTime = m_dwInvinciblePodTime;
	dlg.m_nSlowTime = m_dwSlowPodTime;
	dlg.m_nSmokeTime = m_dwSmokePodTime;
	dlg.m_nSpeedTime = m_dwSpeedPodTime;
	dlg.m_nWallTime = m_dwWallPodTime;
	dlg.m_dblUnk58 = m_dblUnk01A0;
	dlg.m_dblUnk68 = m_dblUnk01A8;
	dlg.m_dblUnk60 = m_dblUnk01B0;

	if (dlg.DoModal() == IDOK)
	{
		m_dwInvinciblePodTime = dlg.m_nInvincibleTime;
		m_dwSlowPodTime = dlg.m_nSlowTime;
		m_dwSmokePodTime = dlg.m_nSmokeTime;
		m_dwSpeedPodTime = dlg.m_nSpeedTime;
		m_dwWallPodTime = dlg.m_nWallTime;
		m_dblUnk01A0 = dlg.m_dblUnk58;
		m_dblUnk01A8 = dlg.m_dblUnk68;
		m_dblUnk01B0 = dlg.m_dblUnk60;
	}
}

// Options|Customize Game: on OK the skill level chosen in the dialog is
// turned into the three active parameter blocks (the same selection
// SelectSkillParams makes), the human player and every robot on the board are
// pointed at the new blocks, and the view's projection factors are
// recomputed.

// hover.exe:0x4137D0
// FUNCTION: HOVER 0x4137d0
void CHoverApp::OnOptionsCustomize()
{
	CBumperDoc* pDoc;
	POSITION pos;
	CObject* pObj;

	Hover_GetView()->SuspendGame(1);

	CCustomizeDialog dlg(NULL);

	if (dlg.DoModal() == IDOK)
	{
		// The skill-level switch of SelectSkillParams, spelled out again
		switch (m_dwSkill)
		{
		case 0:
			m_pHumanParams = &g_HBegParams;
			m_pRobotParams = &g_RBegParams;
			m_pRobotAIParams = &g_RobotAIBegParams;
			break;
		case 1:
			m_pHumanParams = &g_HIntParams;
			m_pRobotParams = &g_RIntParams;
			m_pRobotAIParams = &g_RobotAIIntParams;
			break;
		case 2:
			m_pHumanParams = &g_HExpParams;
			m_pRobotParams = &g_RExpParams;
			m_pRobotAIParams = &g_RobotAIExpParams;
			break;
		}

		pDoc = theApp.m_pBumperDoc;
		pDoc->m_humanPlayer.SetParams(m_pHumanParams);
		Hover_GetView()->m_needle.ComputeScale();

		for (pos = pDoc->m_listActiveObjs.GetHeadPosition(); pos != NULL; )
		{
			pObj = pDoc->m_listActiveObjs.GetNext(pos);
			if (pObj->IsKindOf(RUNTIME_CLASS(CRobotPlayer)))
				((CRobotPlayer*)pObj)->SetParams(m_pRobotParams);
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
	Hover_GetView()->SuspendGame(1);

	CPlayerControlsDialog dlg(NULL);

	if (dlg.DoModal() == IDOK)
	{
		Hover_GetView()->RequestDashboardRebuild(1);
		Hover_GetView()->m_bNeedRepaint = 1;
		Hover_GetView()->RequestRenderFrame(1);
	}
}

// Options|Sound Options: the dialog applies its own settings, so the
// result is ignored.

// hover.exe:0x4259A0
// FUNCTION: HOVER 0x4259a0
void CHoverApp::OnOptionsSounds()
{
	Hover_GetView()->SuspendGame(1);

	CSoundOptionsDialog dlg(NULL);

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
// full screen without the 2x pixel doubling (the view's m_bDoubleSize
// flag). RequestResize resizes the offscreen bitmap unless the mode switch
// already did so (m_dwUnk0088).

// hover.exe:0x425A20
// FUNCTION: HOVER 0x425a20
void CHoverApp::OnOptionsFullScreen()
{
	CBumperView* pView;
	DWORD* p;

	pView = Hover_GetView();
	pView->SuspendGame(1);
	pView->m_dwUnk0088 = 0;

	if (m_bFullScreen != 0)
	{
		pView->m_bDoubleSize = 0;
		m_bUseDisplayDib = 0;
		LeaveFullScreen();
	}
	else
	{
		if ((GetKeyState(VK_SHIFT) & 0x8000) != 0)
		{
			pView->m_bDoubleSize = 0;
		}
		else
		{
			pView->m_bDoubleSize = 1;
			if (m_bDisplayDibAvailable != 0)
				m_bUseDisplayDib = 1;
		}
		EnterFullScreen();
	}

	if (pView->m_dwUnk0088 == 0)
	{
		p = &pView->m_dwRequestedWidth;
		pView->RequestResize(p[0], pView->m_dwRequestedHeight, 1);
	}
	pView->RequestRenderFrame(1);
}

// hover.exe:0x413950
// FUNCTION: HOVER 0x413950
void CHoverApp::OnUpdateOptionsFullScreen(CCmdUI* pCmdUI)
{
	// Enabled when ChangeDisplaySettings was resolved or the display is
	// already known to support the mode switch
	pCmdUI->Enable(m_pfnChangeDisplaySettings != NULL || m_bDisplayDibAvailable != 0);
	pCmdUI->SetCheck(m_bFullScreen);
}

// The level was completed: the view's WM_USER+2 handler routes here.
// Parks the render thread, credits the end-of-level score (m_bLevelBonus is
// the "award the level bonus" flag, set when the level was played through
// rather than restarted), switches the view into the between-levels state,
// steps the level counter and loads the next level. The flag is cleared
// before the load so a restart of the new level does not pay the bonus
// twice.

// hover.exe:0x425AD0
// FUNCTION: HOVER 0x425ad0
void CHoverApp::OnLevelComplete()
{
	CBumperView* pView = Hover_GetView();

	pView->WaitWorkerIdle();
	pView->ComputeLevelBonus(m_bLevelBonus);
	pView->SetMessageState(3, 1);
	g_dwLevel++;
	m_bLevelBonus = 0;
	StartLevel();
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
void CHoverApp::OnGameOver()
{
	CBumperView* pView;
	int nLevel;

	m_dwGameState = 0;

	pView = Hover_GetView();
	pView->StopGameTimer();
	pView->SetMessageState(2, 1);

	CHighScoresDialog dlgScores(NULL);

	if (dlgScores.CheckNewHighScore(theApp.m_pBumperDoc->m_dwScore))
		dlgScores.DoModal();
	theApp.m_pBumperDoc->m_dwScore = 0;

	if (AfxMessageBox(IDS_DEFEATED, MB_YESNO | MB_ICONQUESTION,
			(UINT)-1) == IDYES)
	{
		pView = Hover_GetView();
		pView->StopGameTimer();
		theApp.m_pBumperDoc->m_humanPlayer.ClearPods(1);

		CStartAtDialog dlg(NULL);

		if (dlg.DoModal() == IDOK)
		{
			nLevel = (int)g_dwStartAtLevel - 1;
			g_dwLevel = nLevel;
			if (nLevel >= 20)
				g_dwLevel = 19;
			pView->ResetDashboard();
			pView->SetMessageState(0, 1);
			pView->RedrawWindow();
			StartLevel();
			m_bLevelBonus = 1;
		}
		else
		{
			m_bLevelBonus = 0;
		}
	}
}

// Game|Start At...: ends a game in progress (offering the high score
// entry), asks for the starting level and starts a new game there.
// g_dwStartAtLevel is the level the dialog left behind; the game's level
// counter g_dwLevel is zero based and capped at 19.

// hover.exe:0x425CB0
// FUNCTION: HOVER 0x425cb0
void CHoverApp::OnGameStartAt()
{
	CBumperView* pView;
	int nLevel;

	if (m_dwGameState != 0)
	{
		if (AfxMessageBox(IDS_QUIT_IN_PROGRESS, MB_YESNO, (UINT)-1) == IDNO)
			return;
	}

	m_dwGameState = 0;

	pView = Hover_GetView();
	pView->StopGameTimer();
	pView->SetMessageState(2, 1);

	CHighScoresDialog dlgScores(NULL);

	if (dlgScores.CheckNewHighScore(theApp.m_pBumperDoc->m_dwScore))
		dlgScores.DoModal();
	theApp.m_pBumperDoc->m_dwScore = 0;

	pView = Hover_GetView();
	pView->StopGameTimer();
	theApp.m_pBumperDoc->m_humanPlayer.ClearPods(1);

	CStartAtDialog dlg(NULL);

	if (dlg.DoModal() == IDOK)
	{
		nLevel = (int)g_dwStartAtLevel - 1;
		g_dwLevel = nLevel;
		if (nLevel >= 20)
			g_dwLevel = 19;
		pView->ResetDashboard();
		pView->SetMessageState(0, 1);
		pView->RedrawWindow();
		StartLevel();
		m_bLevelBonus = 1;
	}
	else
	{
		m_bLevelBonus = 0;
	}
}

// ID_DBG_FULLSCREEN_TOPMOST (debug): toggles the flag at +0x3BC.

// hover.exe:0x425EC0
// FUNCTION: HOVER 0x425ec0
void CHoverApp::OnDbgFullScreenTopmost()
{
	m_bFullScreenTopmost = !m_bFullScreenTopmost;
}

// hover.exe:0x425EE0
// FUNCTION: HOVER 0x425ee0
void CHoverApp::OnUpdateDbgFullScreenTopmost(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(m_bFullScreenTopmost);
}

// ID_DBG_SHOW_BEACONS (debug): toggles the flag at +0x1E0.

// hover.exe:0x425F00
// FUNCTION: HOVER 0x425f00
void CHoverApp::OnDbgShowBeacons()
{
	m_bShowBeacons = !m_bShowBeacons;
}

// hover.exe:0x425F20
// FUNCTION: HOVER 0x425f20
void CHoverApp::OnUpdateDbgShowBeacons(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(m_bShowBeacons);
}

// ID_DBG_ROBOTPARAMS (debug): edits the robot player parameters (template
// 149, also stripped) and re-points every robot on the board at the R
// parameter block afterwards.

// hover.exe:0x425F40
// FUNCTION: HOVER 0x425f40
void CHoverApp::OnDbgRobotParams()
{
	POSITION pos;
	CObject* pObj;

	Hover_GetView()->SuspendGame(1);

	CRobotParamsDialog dlg(NULL);

	dlg.DoModal();

	for (pos = theApp.m_pBumperDoc->m_listActiveObjs.GetHeadPosition(); pos != NULL; )
	{
		pObj = theApp.m_pBumperDoc->m_listActiveObjs.GetNext(pos);
		if (pObj->IsKindOf(RUNTIME_CLASS(CRobotPlayer)))
			((CRobotPlayer*)pObj)->SetParams(m_pRobotParams);
	}
}

// ID_DBG_HUMANPARAMS (debug): edits the human player parameters (template 148,
// also stripped) and reloads them into the human player and the view's
// projection factors.

// hover.exe:0x426000
// FUNCTION: HOVER 0x426000
void CHoverApp::OnDbgHumanParams()
{
	Hover_GetView()->SuspendGame(1);

	CHumanParamsDialog dlg(NULL);

	dlg.DoModal();

	theApp.m_pBumperDoc->m_humanPlayer.SetParams(m_pHumanParams);
	Hover_GetView()->m_needle.ComputeScale();
}

// hover.exe:0x4260B0
// FUNCTION: HOVER 0x4260b0
void CHoverApp::OnGameHighScores()
{
	Hover_GetView()->SuspendGame(1);

	CHighScoresDialog dlg(NULL);

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
	Hover_GetView()->SuspendGame(1);

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

	Hover_GetView()->SuspendGame(1);

	if (m_dwGameState != 0)
	{
		if (AfxMessageBox(IDS_QUIT_IN_PROGRESS, MB_YESNO, (UINT)-1) == IDNO)
			return;
	}

	CHighScoresDialog dlgScores(NULL);

	if (dlgScores.CheckNewHighScore(theApp.m_pBumperDoc->m_dwScore))
		dlgScores.DoModal();
	theApp.m_pBumperDoc->m_dwScore = 0;

	pView = Hover_GetView();
	pView->StopGameTimer();
	theApp.m_pBumperDoc->m_humanPlayer.ClearPods(1);
	g_dwLevel = 0;
	m_bLevelBonus = 0;
	pView->ResetDashboard();
	pView->SetMessageState(0, 1);
	pView->RedrawWindow();
	StartLevel();
}

// ID_DBG_ROBOTAIPARAMS (debug): opens the dialog on template 152 (also
// stripped).

// hover.exe:0x4261B0
// FUNCTION: HOVER 0x4261b0
void CHoverApp::OnDbgRobotAIParams()
{
	Hover_GetView()->SuspendGame(1);

	CRobotAIParamsDialog dlg(NULL);

	dlg.DoModal();
}

// ID_DBG_WAVEMIX_CONFIG (debug): deactivates the WaveMix session, runs its own
// configuration dialog and reactivates it. When the output format changed
// the loaded sounds no longer match, so the user is told to reload a level.

// hover.exe:0x426230
// FUNCTION: HOVER 0x426230
void CHoverApp::OnDbgWaveMixConfig()
{
	MIXSESSION* pEngine;
	CBumperView* pView;
	HWND hWnd;
	DWORD dwChanged;

	hWnd = NULL;
	Hover_GetView()->SuspendGame(1);
	dwChanged = 0;
	WaveMixActivate(g_pMixSession, 0);
	pEngine = g_pMixSession;

	pView = Hover_GetView();
	if (pView != NULL)
		hWnd = pView->m_hWnd;

	if (WaveMixConfigure(pEngine, (DWORD)hWnd, NULL,
			(DWORD)&dwChanged, 1) == 0 && dwChanged != 0)
		AfxMessageBox("Reload a new level to restart sounds!", 0, 0);

	WaveMixActivate(g_pMixSession, 1);
}
