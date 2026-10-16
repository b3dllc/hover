// hover.h : main header file for the Hover! application.
//
// This is a reconstructed version of the original Hover! for Windows 95
// source code, produced by manual decompilation of hover.exe.

#ifndef __HOVER_H__
#define __HOVER_H__

#include <afxwin.h>
#include <afxext.h>			// CStatusBar (CMainFrame member)
#include <afxcoll.h>
#include <mmsystem.h>

#include "resource.h"

// MFC 3.1's afxcmn.inl still refers to two tab-control messages that the
// Windows 95 SDK headers we build with (from VC++ 2.2) no longer define.
// The inline functions using them (CTabCtrl::Get/SetBkColor) are never
// called; the values are the pre-release ones.
#ifndef TCM_GETBKCOLOR
#define TCM_GETBKCOLOR (TCM_FIRST + 0)
#define TCM_SETBKCOLOR (TCM_FIRST + 1)
#endif

/////////////////////////////////////////////////////////////////////////////
// Per-level game parameters.
//
// Global table of 18 entries (0x58 bytes each) at hover.exe:0x4C4000.
// The first three fields cycle through three music/maze/texture file
// sets; the m_anLevelScore..m_anRemainingScore fields are filled in at
// runtime by the CHoverApp constructor.

typedef struct tagLVL_PARAMS
{
	const char* m_pszMusic;		// +0x00 "Sounds\Music\MUSICn.muz"
	const char* m_pszMaze;		// +0x04 "mazes\MAZEn.maz"
	const char* m_pszTextures;	// +0x08 "mazes\TEXn.tex"
	DWORD m_adwUnk0C[7];		// +0x0C (all 4 in hover.exe)
	DWORD m_dwUnk28;			// +0x28
	DWORD m_dwUnk2C;			// +0x2C
	// m_dwUnk30 and the computed fields below are read and divided with
	// signed `cdq`/`idiv` throughout hover.exe (the CHoverApp constructor's
	// six `* 9 / 10` / `* 11 / 10` divisions and every reader that casts
	// them to (int)/(long)); see notes/perfect-match.md rule 391/401.
	int m_dwUnk30;				// +0x30 flags per side
	// The three scores, each per skill level (theApp.m_dwSkill: 0 =
	// beginner, 1 = intermediate, 2 = expert), all computed by the
	// CHoverApp constructor: [1] from the level, [0] = [1] * 9 / 10 and
	// [2] = [1] * 11 / 10. The Start At dialog shows the three.
	int m_anLevelScore[3];		// +0x34 bonus for finishing the level: the
								//   running total of m_dwUnk30 *
								//   m_anCaptureScore[1] over the levels
	int m_anCaptureScore[3];	// +0x40 per flag captured (1000, growing
								//   by half each level)
	int m_anRemainingScore[3];	// +0x4C per flag kept at the end of the
								//   level (2500, growing likewise)
} LVL_PARAMS;					// sizeof = 0x58

// Levels 16 and up cycle over the last three parameter entries. This
// indexing expression appears throughout the CHoverApp constructor
// (hover.exe:0x41C100) and was most likely a macro in the original source.
// It only ever reaches entries 0..17; entries 18 and 19 are dead and entry
// 20 (the silent "small" attract maze) is reached by indexing
// g_lvlParams directly, which is what the level loader does when the
// document is in attract mode.
#define LVLPARAM(i) (g_lvlParams[((i) < 16) ? (i) : ((i) % 3 + 15)])

extern LVL_PARAMS g_lvlParams[21];	// hover.exe:0x4C4000 (0x4C4000..0x4C4738)

/////////////////////////////////////////////////////////////////////////////
// Localized message strings loaded by CHoverApp::InitInstance.
// g_szMemoryError is loaded with a raw LoadString; the rest are global
// CString objects (constructed before WinMain like any global object).

extern char g_szMemoryError[0x80];	// hover.exe:0x4604D0 IDS_MEMORY_ERROR
extern CString g_strReadFileError;	// hover.exe:0x460550 IDS_READ_FILE_ERROR
extern CString g_strUnexpectedException; // hover.exe:0x4604B0 IDS_UNEXPECTED_EXCEPTION
extern CString g_strExcNotSupported;	// hover.exe:0x460560 IDS_EXC_NOT_SUPPORTED
extern CString g_strExcArchiveNone;		// hover.exe:0x460020 IDS_EXC_ARCHIVE_NONE
extern CString g_strExcArchiveGeneric;	// hover.exe:0x460030 IDS_EXC_ARCHIVE_GENERIC
extern CString g_strExcArchiveReadOnly;	// hover.exe:0x460040 IDS_EXC_ARCHIVE_READONLY
extern CString g_strExcArchiveEndOfFile; // hover.exe:0x460050 IDS_EXC_ARCHIVE_ENDOFFILE
extern CString g_strExcArchiveWriteOnly; // hover.exe:0x460460 IDS_EXC_ARCHIVE_WRITEONLY
extern CString g_strExcArchiveBadIndex;	// hover.exe:0x460470 IDS_EXC_ARCHIVE_BADINDEX
extern CString g_strExcArchiveBadClass;	// hover.exe:0x460480 IDS_EXC_ARCHIVE_BADCLASS
extern CString g_strExcArchiveBadSchema; // hover.exe:0x460490 IDS_EXC_ARCHIVE_BADSCHEMA
extern CString g_strExcResource;		// hover.exe:0x4604C0 IDS_EXC_RESOURCE
extern CString g_strExcOle;				// hover.exe:0x460010 IDS_EXC_OLE
extern CString g_strExcDao;				// hover.exe:0x4604A0 IDS_EXC_DAO
extern CString g_strExcUser;			// hover.exe:0x460000 IDS_EXC_USER

/////////////////////////////////////////////////////////////////////////////
// Other reconstructed globals (defined in hover.cpp)

extern DWORD g_cbTextureHeap;		// hover.exe:0x46002C large buffer size
extern void* g_pTextureHeap;			// hover.exe:0x46003C large buffer (malloc'd)
extern DWORD g_dwMipBias;		// hover.exe:0x461CFC memory configuration mode
extern DWORD g_bUseWaveMix;		// hover.exe:0x4C56D8
extern FARPROC g_pfnJoyGetPosEx;	// hover.exe:0x4C4CE8 joyGetPosEx (optional)
extern CObject* g_pUnk_4C4738;		// hover.exe:0x4C4738 deleted by ExitInstance
extern DWORD g_dwStartAtLevel;		// hover.exe:0x46048C
extern DWORD g_dwLevel;		// hover.exe:0x46049C
extern DWORD g_bStartingTimer;		// hover.exe:0x4C4CE0 blocks CBumperView::SuspendGame
									// when nonzero
extern DWORD g_bTickPending;		// hover.exe:0x4C4CE4 cleared by
									// CBumperView::OnUserInput once the
									// frame's input sample has been taken

/////////////////////////////////////////////////////////////////////////////
// Per-skill-level parameter blocks (the game's skill tuning)
//
// Nine global objects, three per skill level: the human player's block (H),
// the robot players' block (R) and the robot AI's block (RobotAI);
// Beg/Int/Exp = beginner/intermediate/expert. CHoverApp::SelectSkillParams
// points m_pHumanParams/m_pRobotParams/m_pRobotAIParams at the set for the
// current skill level,
// Hover_LoadHumanParams/Hover_LoadRobotParams/Hover_LoadRobotAIParams
// overwrite them from the registry, and the debug dialogs
// CHumanParamsDialog/CRobotParamsDialog/CRobotAIParamsDialog edit them. Every
// entry is a 16.16 fixed-point value except where marked as a plain integer,
// so the constructors take the tuning in floating point and store it
// multiplied by 65536.
//
// The blocks are indexed as DWORD arrays all over the game (through
// theApp.m_pHumanParams etc.), hence the public m_adw member.

// CPlayerParams: the eleven-entry human/robot player block (sizeof 0x2C, the
// byte count the registry loaders ask for; the original's blocks sit 0x30
// apart, i.e. the linker padded them to an 8-byte boundary). Known entries:
// [0] and [2] are the tuning values CPlayer::SetParams caches at
// +0x1D0/+0x1D4/+0x1D8; [3] is 0 in every human block (the debug dialog does
// not even exchange it) and 20.0 in every robot block; [8] is the view scale
// factor CVelocityIndicator::ComputeScale divides by, which is why a zeroed
// block used to make the first drawn frame divide by zero. The parameter
// debug dialog CHumanParamsDialog exchanges [0..2] and [4..10].

class CPlayerParams
{
public:
	// The 16.16 constructor: used for the beginner human block, whose
	// eleven stores the compiler folds into the static initializer.
	CPlayerParams(long l0, long l1, long l2, long l3, long l4, long l5,
		long l6, long l7, long l8, long l9, long l10)
	{
		m_adw[0] = l0;
		m_adw[1] = l1;
		m_adw[2] = l2;
		m_adw[3] = l3;
		m_adw[4] = l4;
		m_adw[5] = l5;
		m_adw[6] = l6;
		m_adw[7] = l7;
		m_adw[8] = l8;
		m_adw[9] = l9;
		m_adw[10] = l10;
	}

	// The floating-point constructor, out of line, so the other five
	// blocks are built by a call with the eleven doubles on the stack.
	CPlayerParams(double dbl0, double dbl1, double dbl2, double dbl3,
		double dbl4, double dbl5, double dbl6, double dbl7, double dbl8,
		double dbl9, double dbl10);		// hover.exe:0x425290

	DWORD m_adw[11];		// +0x00..+0x28
};							// sizeof = 0x2C

// CRobotAIParams: the ten-entry robot-AI block (sizeof 0x28). [0] and [2] are
// 16.16 fixed point (CRobotAIParamsDialog edits them as doubles, the rest as
// integers); [1] is copied to a robot's +0x8C, [2] scales its speed, [7] and
// [8] are the beacon-choice thresholds in CMazeObj, [9] is a step count
// limit. The constructor is inline, so all three blocks are filled with
// folded constants in the static initializer.

class CRobotAIParams
{
public:
	CRobotAIParams(double dbl0, int n1, double dbl2, int n3, int n4,
		int n5, int n6, int n7, int n8, int n9)
	{
		m_adw[0] = (long)(dbl0 * 65536.0);
		m_adw[1] = n1;
		m_adw[2] = (long)(dbl2 * 65536.0);
		m_adw[3] = n3;
		m_adw[4] = n4;
		m_adw[5] = n5;
		m_adw[6] = n6;
		m_adw[7] = n7;
		m_adw[8] = n8;
		m_adw[9] = n9;
	}

	DWORD m_adw[10];		// +0x00..+0x24
};							// sizeof = 0x28

extern CPlayerParams g_HBegParams;		// hover.exe:0x4A48F8 HBegParams
extern CPlayerParams g_RBegParams;		// hover.exe:0x4A4928 RBegParams
extern CPlayerParams g_HExpParams;		// hover.exe:0x4A4958 HExpParams
extern CPlayerParams g_RExpParams;		// hover.exe:0x4A4988 RExpParams
extern CPlayerParams g_HIntParams;		// hover.exe:0x4A49B8 HIntParams
extern CPlayerParams g_RIntParams;		// hover.exe:0x4A49E8 RIntParams
extern CRobotAIParams g_RobotAIBegParams;	// hover.exe:0x4A4880 RobotAIBegParams
extern CRobotAIParams g_RobotAIExpParams;	// hover.exe:0x4A48A8 RobotAIExpParams
extern CRobotAIParams g_RobotAIIntParams;	// hover.exe:0x4A48D0 RobotAIIntParams

class CPodObj;
class CFloorObj;
class CRobotPlayer;
class CWall;
class CMerlinDynamic;
class CBeacon;
struct tagREGIONNODE;

// Pointer lists emptied by the subsystem shutdown helpers called from
// CHoverApp::ExitInstance (element types not yet known)
extern CPtrList g_listPodObjBlocks;	// hover.exe:0x4A45B8
extern CPtrList g_listFloorObjBlocks;	// hover.exe:0x4A45E0
extern CPtrList g_listRobotBlocks;	// hover.exe:0x4A46A0
extern CPtrList g_listWallBlocks;	// hover.exe:0x4A46D0
extern CPtrList g_listRegionNodeBlocks;	// hover.exe:0x4A46F8
extern CPtrList g_listMerlinDynamicBlocks;	// hover.exe:0x4A4748
extern CPtrList g_listBeaconBlocks;	// hover.exe:0x4A4858

extern CPodObj* g_pPodObjFree;		// hover.exe:0x4C5330
extern CFloorObj* g_pFloorObjFree;		// hover.exe:0x4C53E0
extern CRobotPlayer* g_pRobotFree;		// hover.exe:0x4C54D0
extern CWall* g_pWallFree;		// hover.exe:0x4C5588
extern struct tagREGIONNODE* g_pRegionNodeFree;		// hover.exe:0x4C55D0
extern DWORD g_dwRegionNodeChunk;		// hover.exe:0x4C55D4
extern CMerlinDynamic* g_pMerlinDynamicFree;		// hover.exe:0x4C5620
extern CBeacon* g_pBeaconFree;		// hover.exe:0x4C57A8

// Game settings ("GameSettings" registry value; theApp.m_dwSpeedDetail is in
// bumpview.cpp, extern in bumpview.h)

// WaveMix session configuration passed to WaveMixConfigureInit
// (WaveMixConfigureInit). WaveMix V 2.3 by Angel M. Diaz, Jr. (c)
// Microsoft 1993-1995 is statically linked into hover.exe.
#pragma pack(push, 1)
typedef struct tagMIXCONFIG
{
	WORD wSize;			// +0x00 = 0x1e structure size
	DWORD dwFlags;			// +0x02 = 3 flags (bit 0x400 set when the
					//	HKLM registry key opens, bit 0x100
					//	selects wShowDebugDialogs over the ini setting)
	WORD wChannels;			// +0x06 = 1 channels
	WORD wSamplingRate;			// +0x08 = 0xb sampling rate (11 kHz)
	WORD wWaveBlockCount;			// +0x0A wave block count ("WaveBlocks")
	WORD wWaveBlockLen;			// +0x0C wave block length ("WaveBlockLen")
	WORD wCmixPtrDefaultFlag;			// +0x0E = 1 (set by WaveMix_ReadRegistryForAppSpecificConfigs)
	WORD wResetMixDefaultFlag;			// +0x10 remix mode ("Remix")
	WORD wGoodWavePos;			// +0x12 good position flag ("GoodWavePos")
	WORD wDeviceID;			// +0x14 waveOut device ID
	WORD wPauseBlocks;			// +0x16 pause block count ("PauseBlocks")
	WORD wShowDebugDialogs;			// +0x18 show debug dialogs (when bit 0x100)
	HKEY hRegistryKey;			// +0x1A = hKey (if the key opens)
} MIXCONFIG;
#pragma pack(pop)

// WaveMix mixing session object (0x1C0 bytes, LocalAlloc'd with LPTR by
// WaveMixConfigureInit; validated by the 0x5432 magic words at both ends)
// The session's three callbacks, typed as hover.exe calls them. The
// functions installed in them do not all match: WaveMix_CMixit takes its
// count as a WORD and WaveMix_NoResetRemix declares no parameters, so
// those two are cast where they are stored.
typedef void (*MIXITFN)(BYTE* pBuf, DWORD** ppSrcs, WORD* pwVols, int nSrcs,
	int nCount);
typedef int (*REMIXFN)(DWORD dwPos, int bFlag);
typedef int (*POSADJUSTFN)(int nA, int nB);

typedef struct tagMIXSESSION
{
	WORD wMagic1;			// +0x00 = 0x5432
	BYTE abUnk02[2];		// +0x02
	HWND hWndCallback;			// +0x04 wave callback window
	BYTE abUnk08[4];		// +0x08
	HWAVEOUT hWaveOut;			// +0x0C waveOut handle
	DWORD fActive;			// +0x10 session active (WaveMixActivate)
	DWORD fSettingsDialogActive;			// +0x14
	DWORD dwDeviceID;			// +0x18 waveOut device ID
	char szDevicePName[0x60];		// +0x1C wavemix.inf section name for the
						//	device (built by WaveMix_ReadConfigSettings)
	WAVEOUTCAPSA woCaps;	// +0x7C waveOut device caps (szPname at +0x84)
	WORD wDefaultVolumeL;			// +0xB0 = 10 default channel parameter 1
	WORD wDefaultVolumeR;			// +0xB2 = 10 default channel parameter 2
	WORD awChannelVolume[0x20];		// +0xB4 per-channel parameter pairs
					//	(accessed with a dword stride)
	struct tagCHANNELNODE* apChannel[0x10];	// +0xF4 per-channel node queues
					//	(WMIX_CHANNEL_CLOSED = not open)
	DWORD dwOpenChannels;			// +0x134 open channel count
	DWORD adwChannelMRU[0x10];		// +0x138 per-channel play order stamps
	DWORD dwMRU;			// +0x178 play order counter (WaveMixPlay)
	PCMWAVEFORMAT pcm;	// +0x17C output format (PCM mono 11kHz 8-bit)
	DWORD dwWaveBlockLen;			// +0x18C wave block data length
	DWORD dwWaveBlockCount;			// +0x190 wave block count
	DWORD dwPauseBlocks;			// +0x194
	struct tagXWAVEHDR** ppWaveBlocks;	// +0x198 active wave block array
						//	(g_apWaveBlocksA or B)
	DWORD dwCurrentSample;			// +0x19C current play position (bytes)
	DWORD dwBaseTime;			// +0x1A0 timeGetTime() origin
	DWORD fGoodGetPos;			// +0x1A4 position source flag
	DWORD dwWaveOutPosDelta;			// +0x1A8 position adjustment delta
	MIXITFN pfnMixit;			// +0x1AC mixing function (cdecl, 5 args)
	REMIXFN pfnRemix;			// +0x1B0 session callback (cdecl, 2 args)
	POSADJUSTFN pfnPosAdjust;			// +0x1B4 position adjust function (Add/Sub)
	struct tagCHANNELNODE* pWaitList;	// +0x1B8 newest node of the circular
						//	WaveMixPlay "wait list" (its pNext
						//	is the oldest)
	WORD wMagic2;			// +0x1BC = 0x5432
	WORD wUnk1BE;			// +0x1BE (pads the object to 0x1C0)
} MIXSESSION;

// apChannel value of a channel that is not open (an open channel holds its
// node queue, NULL when empty)
#define WMIX_CHANNEL_CLOSED ((struct tagCHANNELNODE*)0xFFFFFFFF)

// WaveMix output block ("lpXWH"): a WAVEHDR plus WaveMix's bookkeeping,
// followed by the sample data. Each block is a 0x2030-byte GlobalAlloc
// (WaveMix_AllocWaveBlocks); 20 of them make up each of the two block
// arrays.
typedef struct tagXWAVEHDR
{
	WAVEHDR wh;				// +0x00 lpData = abData
	BOOL fAvailable;		// +0x20 free for the next mix
	DWORD dwWavePos;		// +0x24 mix position of the block's first sample
	MIXSESSION* pSession;	// +0x28 owning session
	struct tagXWAVEHDR* pQNext;	// +0x2C next block of the playing queue
	BYTE abData[0x2000];	// +0x30 sample data (dwWaveBlockLen bytes used)
} XWAVEHDR;					// sizeof = 0x2030

// WaveMix play parameters passed to WaveMixPlay (WaveMixPlay)
#pragma pack(push, 2)
typedef struct tagMIXPLAYPARAMS
{
	WORD wSize;				// +0x00 = 0x1C
	MIXSESSION* pSession;	// +0x02 mixing session
	int nChannel;			// +0x06 channel (flag 0x2: pick least recently used)
	struct tagMIXWAVE* pMixWave;			// +0x0A the wave, from WaveMixOpenWave
	HWND hwndNotify;		// +0x0E notification window
	DWORD dwFlags;			// +0x12 (0x1 prepend, 0x2 LRU channel, 0x4 use
							//	current position, 0x8 queue only, 0x10 volumes)
	WORD wLoops;			// +0x16 loop count (0xFFFF = forever)
	WORD wVolumeL;			// +0x18 per-play volumes when flag 0x10
	WORD wVolumeR;			// +0x1A
} MIXPLAYPARAMS;				// sizeof = 0x1C
#pragma pack(pop)

// WaveMix channel node: one WaveMixPlay request queued on a channel (or on
// the session's wait list). The 100 nodes live in g_aChannelNodePool and
// are handed out from the g_pFreeChannelNodes free list.
#pragma pack(push, 2)
typedef struct tagCHANNELNODE
{
	struct tagCHANNELNODE* pNext;	// +0x00 next node (channel queue, free
								//	list or circular wait list)
	MIXPLAYPARAMS PlayParams;	// +0x04 copy of the WaveMixPlay parameters
	struct tagMIXWAVE* lpMixWave;	// +0x20 the wave
	DWORD dwNumSamples;			// +0x24 wave length (bytes; one loop)
	DWORD dwStartPos;			// +0x28 mix position the wave starts at
	DWORD dwEndPos;				// +0x2C mix position it ends at (-1 = loops
								//	forever)
	BYTE* lpPos;				// +0x30 sample data
	BYTE* lpEnd;				// +0x34 last sample frame
	WORD wVolumeL;				// +0x38 volumes (0..10)
	WORD wVolumeR;				// +0x3A
} CHANNELNODE;					// sizeof = 0x3C
#pragma pack(pop)

// WaveMix loaded-wave object built by WaveMixOpenWave (WaveMixOpenWave);
// the format header always holds the session's output format because the
// sample data is converted at load time
#pragma pack(push, 2)
typedef struct tagMIXWAVE
{
	PCMWAVEFORMAT wfx;		// +0x00 output format of pData
	WAVEHDR wh;				// +0x10 lpData/dwBufferLength: the sample
							//	data (GlobalAlloc'd); dwUser, dwFlags
							//	and dwLoops are cleared, the rest unused
	char szName[0x10];		// +0x30 resource id or file name tail
	WORD wMagic;			// +0x40 = 0x5432
} MIXWAVE;					// sizeof = 0x42
#pragma pack(pop)

// Worker-thread shutdown state used by Hover_ShutdownSound
// (the sound slot registry is the CObArray g_arrUnk_4A47F0 in
// gameobj.h; its m_pData/m_nSize members sit at 0x4A47F4/0x4A47F8)
extern CRITICAL_SECTION g_csSoundQueue;	// hover.exe:0x4A4808
extern MIXSESSION* g_pMixSession;	// hover.exe:0x4C56DC sound driver engine object
extern CWinThread* g_pSoundThread;	// hover.exe:0x4C56E0 sound mixing thread
extern DWORD g_bSoundQueueCsInit;		// hover.exe:0x4C56E4
extern DWORD g_bSoundThreadQuit;		// hover.exe:0x4C56E8
extern HANDLE g_hSoundWakeEvent;		// hover.exe:0x4C56F4
extern HANDLE g_hSoundAckEvent;		// hover.exe:0x4C56F8
extern DWORD g_bWaveBusyMsgShown;		// hover.exe:0x4C56FC "wave devices in use"
									// message box already shown
extern DWORD g_dwPlayNesting;		// hover.exe:0x4C5AD4 WaveMixPlay reentry count

// WaveMix engine state
// (Hover_SoundThreadProc/WaveMixConfigureInit/WaveMixOpenChannel)
class CSoundSlot;	// gameobj.h
extern CSoundSlot* g_pSoundOnce;		// hover.exe:0x4C56EC queued one-shot sound
extern CSoundSlot* g_pSoundLoop;		// hover.exe:0x4C56F0 queued looping sound
extern MIXSESSION* g_pCurSession;	// hover.exe:0x4B6248 current session
extern MIXITFN g_pfnDefaultMixit;		// hover.exe:0x4B6F14
extern MIXSESSION* g_pActiveSession;	// hover.exe:0x4B6F18 active session (WaveMixActivate)
extern CHANNELNODE* g_pFreeChannelNodes;	// hover.exe:0x4B6D50 free channel-node list
extern XWAVEHDR* g_apWaveBlocksA[0x14];	// hover.exe:0x4B6E70 wave block array
extern XWAVEHDR* g_apWaveBlocksB[0x14];	// hover.exe:0x4B6EC0 wave block array 2
extern HINSTANCE g_hWaveMixInst;		// hover.exe:0x4B6F10 module instance
extern CHANNELNODE* g_apMixChannelNodes[0x10];	// hover.exe:0x4B61C8 channel node table
extern void* g_apMixSources[0x10];	// hover.exe:0x4B6208 mix source pointers
extern WORD g_awMixVolumes[0x20];	// hover.exe:0x4B6F20 mix volume pairs
extern CHANNELNODE g_aChannelNodePool[100];	// hover.exe:0x4B4A58 channel node pool
extern BYTE g_abVolumeTable[11][256];	// hover.exe:0x4B6250 sample-volume table
extern char g_szIniFileName[];		// hover.exe:0x4B6D58 ini filename buffer
extern DWORD g_bShowDebugDialogs;		// hover.exe:0x4C59B8 show debug dialogs
extern DWORD g_dwDebugLevel;		// hover.exe:0x4C59BC debug output verbosity
extern XWAVEHDR* g_pPlayQueueHead;	// hover.exe:0x4C59C0 playing queue head
extern XWAVEHDR* g_pPlayQueueTail;	// hover.exe:0x4C59C4 playing queue tail
extern DWORD g_bWaveMixInitialized;		// hover.exe:0x4C59C8
extern char g_szStringBuffer[0x100];	// hover.exe:0x4C59D0 version banner/scratch

// Sound subsystem state (Hover_LoadSoundSettings/Hover_InitSound)
extern DWORD g_bMixSessionOpen;		// hover.exe:0x4C56D0 sound driver engine started
extern DWORD g_bWaveOutAvailable;		// hover.exe:0x4C56D4 wave output available

// In-game key bindings (virtual-key codes); [11] is the joystick button

// Joystick state (Hover_LoadControls)

// Fixed-point math tables built by Hover_BuildTrigTables (entry 0 unused)
extern DWORD g_adwSinTable[0x2001];	// hover.exe:0x4ACA54
extern WORD g_awTanTable[0x4001];	// hover.exe:0x4A4A56

/////////////////////////////////////////////////////////////////////////////
// 16.16 fixed-point arithmetic. The original implemented these as _asm
// macros that keep the 64-bit intermediate of the 32x32-bit operation.
// The operand must be a simple variable: it is referenced directly by the
// _asm code (going through a pointer local instead, as the original
// appears to have done, breaks under this compiler's optimizer, which
// cannot see the indirect read/write).
//
// HOVER_FIXMUL(dwA, dwB):  dwA = (dwA * dwB) >> 16
// HOVER_FIXMULW(dwA, wB):  dwA = dwA * wB (signed 16-bit factor)
// HOVER_FIXDIVP(dwA, dwB): dwA = (dwA << 16) / dwB

#define HOVER_FIXMUL(dwA, dwB) \
	{ \
		DWORD _dwB = (dwB); \
		_asm mov eax, dwA \
		_asm imul _dwB \
		_asm shrd eax, edx, 0x10 \
		_asm mov dwA, eax \
	}

#define HOVER_FIXMULW(dwA, wB) \
	{ \
		WORD _wB = (WORD)(wB); \
		_asm movsx eax, _wB \
		_asm imul dwA \
		_asm mov dwA, eax \
	}

#define HOVER_FIXDIVP(dwA, dwB) \
	{ \
		DWORD _dwB = (dwB); \
		_asm mov eax, dwA \
		_asm mov edx, eax \
		_asm sar edx, 0x10 \
		_asm shl eax, 0x10 \
		_asm idiv _dwB \
		_asm mov dwA, eax \
	}

// Pointer-destination variants of the two multiplies above. The original's
// macros took the *address* of the destination in a stack temporary and
// worked through it (`mov ecx, _pA` / `imul dword ptr [ecx]`), which is
// what the physics code at hover.exe:0x41A6B0, 0x41AB10 and 0x421B50 has;
// the plain variants above name the destination directly and produce a
// different (shorter) instruction stream. The two temporaries are declared
// pointer-first and assigned value-first so both the slot order and the
// store order match the original (notes/perfect-match.md rule 15).
//
// HOVER_FIXMULP(dwA, dwB):  dwA = (dwA * dwB) >> 16
// HOVER_FIXMULWP(dwA, wB):  dwA = dwA * (short)wB

#define HOVER_FIXMULP(dwA, dwB) \
	{ \
		DWORD* _pA; \
		DWORD _dwB; \
		_dwB = (dwB); \
		_pA = &(dwA); \
		_asm mov ecx, _pA \
		_asm mov eax, [ecx] \
		_asm imul _dwB \
		_asm shrd eax, edx, 0x10 \
		_asm mov [ecx], eax \
	}

#define HOVER_FIXMULWP(dwA, wB) \
	{ \
		DWORD* _pA; \
		WORD _wB; \
		_wB = (WORD)(wB); \
		_pA = &(dwA); \
		_asm mov ecx, _pA \
		_asm movsx eax, _wB \
		_asm imul dword ptr [ecx] \
		_asm mov [ecx], eax \
	}

// HOVER_FIXDIVLP(dwA, dwB): dwA = (dwA << 16) / dwB through the same
// pointer temporary as HOVER_FIXMULP (the speed reading at
// hover.exe:0x4080e8 uses the divide in that shape).

#define HOVER_FIXDIVLP(dwA, dwB) \
	{ \
		DWORD* _pA; \
		DWORD _dwB; \
		_dwB = (dwB); \
		_pA = &(dwA); \
		_asm mov ecx, _pA \
		_asm mov eax, [ecx] \
		_asm mov edx, eax \
		_asm sar edx, 0x10 \
		_asm shl eax, 0x10 \
		_asm idiv _dwB \
		_asm mov [ecx], eax \
	}

// Pointer-argument forms of the two multiplies/divides above
// (HOVER_FIXMULPP / HOVER_FIXDIVPP): the original's renderer setup passes
// the *address* of the destination (a member of the object being set up),
// and VC 2.2's inline assembler cannot see through a C++ reference -
// `_asm mov eax, refname` loads the reference variable itself, i.e. the
// pointer, and `_asm mov refname, eax` overwrites it - so a destination that
// is not a plain local has to be passed as a DWORD*. hover.exe stores the
// pointer and the second operand in that order, in two stack slots of their
// own. (Distinct from HOVER_FIXMULP, which takes the destination lvalue.)
//
// HOVER_FIXMULPP(pdwA, dwB):  *pdwA = (*pdwA * dwB) >> 16
// HOVER_FIXDIVPP(pdwA, dwB): *pdwA = (*pdwA << 16) / dwB

#define HOVER_FIXMULPP(pdwA, dwB) \
	{ \
		DWORD* _pdwA = (pdwA); \
		DWORD _dwB = (dwB); \
		_asm mov ecx, _pdwA \
		_asm mov eax, [ecx] \
		_asm imul _dwB \
		_asm shrd eax, edx, 0x10 \
		_asm mov [ecx], eax \
	}

#define HOVER_FIXDIVPP(pdwA, dwB) \
	{ \
		DWORD* _pdwA = (pdwA); \
		DWORD _dwB = (dwB); \
		_asm mov ecx, _pdwA \
		_asm mov eax, [ecx] \
		_asm mov edx, eax \
		_asm sar edx, 0x10 \
		_asm shl eax, 0x10 \
		_asm idiv _dwB \
		_asm mov [ecx], eax \
	}

// HOVER_FIXMULR(dwR, dwA, dwB): dwR = (dwA * dwB) >> 16 into a separate
// destination; recombines the 64-bit product with shl/shr/add instead of shrd
// (the variant used by CSpanRasterizer::SubmitSegment's sprite span scaling)
#define HOVER_FIXMULR(dwR, dwA, dwB) \
	{ \
		DWORD _dwA = (dwA); \
		DWORD _dwB = (dwB); \
		_asm mov eax, _dwB \
		_asm imul _dwA \
		_asm shl edx, 0x10 \
		_asm shr eax, 0x10 \
		_asm add eax, edx \
		_asm mov dwR, eax \
	}

/////////////////////////////////////////////////////////////////////////////
// CFixed: the original's 16.16 fixed-point value type. Its multiply,
// divide and 16-bit multiply are out-of-line `_asm` methods that take the
// value by pointer in ecx (hover.exe:0x4038E0, 0x40A490, 0x40B4F0) and
// return `this`. Everywhere else in hover.exe the optimizer expands them
// inline - that is what the HOVER_FIXMUL* macros above reproduce - but the
// two unoptimized callers (CMapRenderer::ProjectPoint in bumpview.cpp and the
// robot steering pass CRobotPlayer::Steer in texload.cpp, see rule 49
// in notes/perfect-match.md) still `call` them, so the class needs a real
// declaration shared by both translation units. The bodies live next to
// their callers: MulEq in bumpview.cpp, the other two in texload.cpp.

class CFixed
{
public:
	void MulEq(DWORD dwB);		// hover.exe:0x4038E0 (bumpview.cpp)
	CFixed* DivEq(DWORD dwB);	// hover.exe:0x40A490 (texload.cpp)
	CFixed* MulShortEq(short sB);	// hover.exe:0x40B4F0 (texload.cpp)

	// The in-place operators are inline members: expanded into the
	// steering pass they need no `this` slot of their own, which is what
	// makes the original's `+=` and `-=` show up as a single argument
	// temporary plus an `add`/`sub` straight into the caller's slot. The
	// double negation in SubEq is the original's, not a transcription
	// slip: it emits `xor eax,eax / sub eax,arg / neg eax / sub [dst],eax`.
	void AddEq(DWORD dwA) { DWORD dwT = dwA; m_dwValue += dwT; }
	void SubEq(DWORD dwA)
		{ DWORD dwT = dwA; m_dwValue -= (DWORD)(-(-(long)dwT)); }

	DWORD m_dwValue;
};

// _asm label helper: pastes __LINE__ onto a label name so a macro whose
// body contains a branch can be expanded more than once in the same
// function (used by HOVER_MULDIVP/HOVER_MULDIVN below).
#define HOVER_DIST_CAT2(a, b) a##b
#define HOVER_DIST_CAT(a, b) HOVER_DIST_CAT2(a, b)

/////////////////////////////////////////////////////////////////////////////
// Hover_Distance: integer point-to-point distance used by the renderer's
// depth keys. The original is an _asm helper that the compiler expands at
// every call site (six copies in hover.exe:0x4045E0, three in
// hover.exe:0x406D30, one more at hover.exe:0x408A30); the four arguments
// and the result get one stack home per expansion because the _asm body
// references them.
//
// It starts from the octagonal estimate
//   dGuess = |dy| - min(|dx|, |dy|) / 2 + |dx| + 1
// and refines it with a single Newton-Raphson step of the integer square
// root of |dx|^2 + |dy|^2:
//   d = (dGuess + (dx * dx + dy * dy) / dGuess) / 2
// The squares are 16x16-bit (mul ax, recombined with shl/add), so the
// helper is only exact while both deltas fit in a WORD - the high half of
// a delta larger than 0xFFFF leaks into the sum instead of being squared.

// It is the game's only distance helper: the HOVER_DIST macro that used to
// sit above (a `{ long _lX1; ... _asm ... }` block, kept because an earlier
// measurement had it scoring better in Hover_BuildDepthList) is gone. Measured
// 2026-09-04 with the argument order below - the four values are stored in
// the order y2, x2, y1, x1, i.e. the *second* point first, which is what
// the original does at 0x4046a6 and 0x406d89 - the inline function emits
// the identical instruction stream at all six call sites in
// Hover_BuildDepthList: its reccmp diff is line-for-line the macro's (95.60%
// either way), so the two forms were the same code all along. Note that
// the block clobbers ebx, esi and edi behind the compiler's back, exactly
// as the original did, and that the `_asm` label is renamed per expansion
// by the compiler (a macro would have needed HOVER_DIST_CAT for that).
inline WORD Hover_Distance(DWORD dwX1, DWORD dwY1, DWORD dwX2, DWORD dwY2)
{
	DWORD dwD;

	_asm mov eax, dwX2
	_asm sub eax, dwX1
	_asm cdq
	_asm xor eax, edx
	_asm sub eax, edx
	_asm mov ebx, eax
	_asm mov eax, dwY2
	_asm sub eax, dwY1
	_asm cdq
	_asm xor eax, edx
	_asm sub eax, edx
	_asm mov esi, eax
	_asm mov ecx, eax
	_asm cmp eax, ebx
	_asm jl HoverDistanceMin
	_asm mov ecx, ebx
	_asm HoverDistanceMin:
	_asm shr ecx, 1
	_asm sub eax, ecx
	_asm lea ecx, [eax+ebx]
	_asm inc ecx
	_asm mov eax, ebx
	_asm mul ax
	_asm shl edx, 0x10
	_asm add edx, eax
	_asm mov edi, edx
	_asm mov eax, esi
	_asm mul ax
	_asm shl edx, 0x10
	_asm add eax, edx
	_asm add eax, edi
	_asm xor edx, edx
	_asm div ecx
	_asm add ecx, eax
	_asm shr ecx, 1
	_asm mov dwD, ecx

	return (WORD)dwD;
}

// HOVER_MULDIVP(dwR, dwBase, dwA, dwB, dwHalf, dwDen):
//   dwR = dwBase + (dwA * dwB + dwHalf) / dwDen, keeping the whole 64-bit
//   product; the rounding term is added with the sign of the product, so
//   the quotient is rounded away from zero when dwHalf is half of dwDen.
// HOVER_MULDIVN(...) is the same operation for a negative dwHalf (i.e. a
//   negative denominator): the original builds the 64-bit sign extension
//   of the rounding term with the sign of dwHalf baked into the code, so
//   the segment intersection at hover.exe:0x401260 carries one copy of the
//   block for each sign of its denominator. Both clobber esi behind the
//   compiler's back, exactly as the original did.
// The five temporaries are declared in the order that fixes their stack
// slots and assigned in the order that fixes the emitted store order (see
// rule 15 in notes/perfect-match.md); the branch label is made unique per
// expansion with the same helper HOVER_DIST uses.

#define HOVER_MULDIVP(dwR, dwBase, dwA, dwB, dwHalf, dwDen) \
	{ \
		long _lA; \
		long _lHalf; \
		long _lB; \
		long _lDen; \
		long _lT; \
		_lB = (long)(dwB); \
		_lA = (long)(dwA); \
		_lHalf = (long)(dwHalf); \
		_lDen = (long)(dwDen); \
		{ \
		_asm mov eax, _lA \
		_asm mov ecx, _lHalf \
		_asm sub esi, esi \
		_asm imul _lB \
		_asm test edx, 0x80000000 \
		_asm je HOVER_DIST_CAT(_hm, __LINE__) \
		_asm neg ecx \
		_asm not esi \
		_asm HOVER_DIST_CAT(_hm, __LINE__): \
		_asm add eax, ecx \
		_asm adc edx, esi \
		_asm idiv _lDen \
		_asm mov _lT, eax \
		} \
		(dwR) = (DWORD)((long)(dwBase) + _lT); \
	}

#define HOVER_MULDIVN(dwR, dwBase, dwA, dwB, dwHalf, dwDen) \
	{ \
		long _lA; \
		long _lHalf; \
		long _lB; \
		long _lDen; \
		long _lT; \
		_lB = (long)(dwB); \
		_lA = (long)(dwA); \
		_lHalf = (long)(dwHalf); \
		_lDen = (long)(dwDen); \
		{ \
		_asm mov eax, _lA \
		_asm mov ecx, _lHalf \
		_asm mov esi, 0xffffffff \
		_asm imul _lB \
		_asm test edx, 0x80000000 \
		_asm jne HOVER_DIST_CAT(_hm, __LINE__) \
		_asm neg ecx \
		_asm sub esi, esi \
		_asm HOVER_DIST_CAT(_hm, __LINE__): \
		_asm add eax, ecx \
		_asm adc edx, esi \
		_asm idiv _lDen \
		_asm mov _lT, eax \
		} \
		(dwR) = (DWORD)((long)(dwBase) + _lT); \
	}

/////////////////////////////////////////////////////////////////////////////
// ZONE_DESCRIPTOR: the screen layout for one of the six window sizes the
// game runs at (320x240 up to 700x525), built by Hover_InitZones. The
// dashboard bitmap for the zone is cut into the top and bottom strips
// around the 3D view; everything else here is where its counters, gauges
// and icons sit on those strips. CMainFrame snaps the window to a zone;
// CBumperView::SetupZone picks the descriptor whose size matches.

struct ZONE_DESCRIPTOR
{
	SIZE m_size;				// +0x00 window client size
	SIZE m_sizeDash;			// +0x08 dashboard bitmap size
	SIZE m_sizeSnap;			// +0x10 snap threshold: midway to the next
								//   zone's size (zone 5: 10000)
	SIZE m_sizeScoreDigit;		// +0x18 score digit tile
	SIZE m_sizePodDigit;		// +0x20 pod counter digit tile
	SIZE m_sizeFlagIcon;		// +0x28 flag icon tile
	int m_y3DTop;				// +0x30 first row of the 3D view
	int m_yTopDashEdge;			// +0x34 row sampled for the top strip's
								//   edge (m_rcTopDashDst.bottom + 1)
	int m_y3DBottom;			// +0x38 last row of the 3D view
	int m_yBottomDashEdge;		// +0x3C row sampled for the bottom strip's
								//   edge (m_rcBottomDashDst.top - 1)
	int m_anGaugeSegments[6];	// +0x40 segment counts of the seven gauges
								//   (CSegmentGauge::SetStyle)
	POINT m_ptPodDigits;		// +0x58 origin of the pod digit tiles in
								//   the dashboard bitmap
	POINT m_ptScoreDigits;		// +0x60 origin of the score digit tiles
	POINT m_ptFlagIcons;		// +0x68 origin of the flag icon tiles
	RECT m_arcPodDigits[10];	// +0x70 pod digit tiles 0-9
	RECT m_arcScoreDigits[10];	// +0x110 score digit tiles 0-9
	RECT m_arcFlagIcons[4];		// +0x1B0 robot full, player full, player
								//   empty, robot empty
	RECT m_rcTopDashSrc;		// +0x1F0 top strip in the dashboard bitmap
	RECT m_rcTopDashDst;		// +0x200 top strip on screen
	RECT m_rcMirror;			// +0x210 rear-view mirror
	POINT m_ptScore;			// +0x220 the seven score digits
	POINT m_ptPlayerFlags;		// +0x228 the player's captured flags
	POINT m_ptRobotFlags;		// +0x230 the robots' captured flags
	RECT m_rcBottomDashSrc;		// +0x238 bottom strip in the dashboard bitmap
	RECT m_rcBottomDashDst;		// +0x248 bottom strip on screen
	RECT m_rcMap;				// +0x258 overview map (CMapRenderer)
	RECT m_rcNeedleSrc;			// +0x268 velocity needle tile in the bitmap
	RECT m_rcNeedle;			// +0x278 velocity needle on screen
	RECT m_arcGauges[3];		// +0x288 gauges m_gaugeSpeed, m_gaugeInvincible and
								//   m_gaugeSlow/m_gaugeSpeedPod
	POINT m_aptPodCounters[3];	// +0x2B8 jump, wall and cloak pod counters
	int m_cyKeyFont;			// +0x2D0 height of the key label font
	RECT m_rcKeyJump;			// +0x2D4 key labels under the pod counters
	RECT m_rcKeyWall;			// +0x2E4
	RECT m_rcKeyCloak;			// +0x2F4
	RECT m_arcPodGauges[3];		// +0x304 jump, wall and cloak time left
	int m_cyMessageFont;		// +0x334 height of the message font
};								// sizeof = 0x338

extern ZONE_DESCRIPTOR g_aZones[6];	// hover.exe:0x4609A0

/////////////////////////////////////////////////////////////////////////////
// CHoverApp:
// See hover.cpp for the implementation of this class.
//
// Note on member offsets: the comments below give each member's offset
// within CHoverApp in hover.exe. The original was built with an earlier
// MFC 4.x whose CWinApp base class is a different size than the MFC 4.2
// used for this build, so the offsets in our build will not match; the
// comments record the ground truth from the executable.

typedef BOOL (WINAPI* LPCHANGEDISPLAYSETTINGS)(LPDEVMODE lpDevMode, DWORD dwFlags);

class CBumperDoc;
class CBumperView;

// An 8-bpp BITMAPINFO: the header plus a full 256-entry color table.
#define BMI8_SIZE (sizeof(BITMAPINFOHEADER) + 256 * sizeof(RGBQUAD))
// A 256-entry LOGPALETTE. The original allocates a whole extra entry
// (0x408, where 0x404 would do): LOGPALETTE already contains one.
#define LOGPAL256_SIZE (sizeof(LOGPALETTE) + 256 * sizeof(PALETTEENTRY))


// The one and only application object (theApp, hover.exe:0x460570) lives in
// .bss, so members the original addresses through the object in CHoverApp
// methods are addressed absolutely (e.g. [0x460728] == theApp.m_dwGameState)
// everywhere else. Both spellings produce the same code; always use the
// member, never a separate global, or the two copies drift apart.
class CHoverApp : public CWinApp
{
public:
	CHoverApp();

	// The destructor is empty in the original: the compiler folded it
	// into the scalar deleting destructor at hover.exe:0x426660, whose
	// body is nothing but the CString member's destructor and the
	// CWinApp base destructor.
	virtual ~CHoverApp() {}			// hover.exe:0x426660

// Overrides
	virtual BOOL InitInstance();		// hover.exe:0x41C9E0
	virtual int ExitInstance();		// hover.exe:0x41C060
	virtual BOOL OnIdle(LONG lCount);	// hover.exe:0x405B80

	// NOTE: CHoverApp does not override Run(). The vtable in hover.exe
	// (0x4BA370) points its Run slot at hover.exe:0x44AAAB, which is the
	// stock MFC CWinApp::Run compiled into the executable, so the original
	// class simply inherited CWinApp::Run.

// Implementation
	void SelectSkillParams();	// hover.exe:0x425870 select params for current skill level
	BOOL CheckMinimumConfig();	// hover.exe:0x425E40 minimum configuration check
	BOOL CheckSingleInstance();	// hover.exe:0x425E80 single instance check
	void FindDataDirectory();	// hover.exe:0x41C7E0 locate the game data directory
	BOOL EnterFullScreen();	// hover.exe:0x4253D0 switch the game into full-screen
						// mode
	BOOL LeaveFullScreen();	// hover.exe:0x425600 switch the game back into a
						// window
	void StartLevel();	// hover.exe:0x41C8E0 run the game: put up the
						// "Loading Level" dialog, load the level and set the
						// frame title
	// hover.exe:0x425AD0 advance to the next level (stub; called by the
	// view's WM_USER+2 handler)
	void OnLevelComplete();
	// hover.exe:0x425B20 end the game and run the high-score flow (stub;
	// called by the view's WM_USER+1 handler)
	void OnGameOver();

// Message handlers (message map at hover.exe:0x4BA0B0). The ID_CMD_34xxx
// commands are leftover debug commands with no menu or accelerator entry;
// the names follow their command ids because their original names are
// unknown.
	//{{AFX_MSG(CHoverApp)
	afx_msg void OnAppAbout();			// hover.exe:0x425390
	afx_msg void OnDbgPodTimes();			// hover.exe:0x425730
	afx_msg void OnOptionsCustomize();	// hover.exe:0x4137D0
	afx_msg void OnOptionsControls();	// hover.exe:0x4258F0
	afx_msg void OnOptionsSounds();		// hover.exe:0x4259A0
	afx_msg void OnUpdateOptionsSounds(CCmdUI* pCmdUI);	// hover.exe:0x413940
	afx_msg void OnOptionsFullScreen();	// hover.exe:0x425A20
	afx_msg void OnUpdateOptionsFullScreen(CCmdUI* pCmdUI);	// hover.exe:0x413950
	afx_msg void OnGameStartAt();		// hover.exe:0x425CB0
	afx_msg void OnDbgFullScreenTopmost();			// hover.exe:0x425EC0
	afx_msg void OnUpdateDbgFullScreenTopmost(CCmdUI* pCmdUI);	// hover.exe:0x425EE0
	afx_msg void OnDbgShowBeacons();			// hover.exe:0x425F00
	afx_msg void OnUpdateDbgShowBeacons(CCmdUI* pCmdUI);	// hover.exe:0x425F20
	afx_msg void OnDbgRobotParams();			// hover.exe:0x425F40
	afx_msg void OnDbgHumanParams();			// hover.exe:0x426000
	afx_msg void OnGameHighScores();	// hover.exe:0x4260B0
	afx_msg void OnUpdateGameHighScores(CCmdUI* pCmdUI);	// hover.exe:0x412240
	afx_msg void OnHelpQuickHelp();		// hover.exe:0x426130
	afx_msg void OnUpdateHelpQuickHelp(CCmdUI* pCmdUI);	// hover.exe:0x4261A0
	afx_msg void OnGameStart();			// hover.exe:0x41D410
	afx_msg void OnDbgRobotAIParams();			// hover.exe:0x4261B0
	afx_msg void OnDbgWaveMixConfig();			// hover.exe:0x426230
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

// Member variables (hover.exe offsets in comments)
public:
	CRITICAL_SECTION m_csScene;	// +0xD4  deleted by ExitInstance
	CRITICAL_SECTION m_csOffscreen;	// +0xEC  deleted by ExitInstance
	CRITICAL_SECTION m_csUnused;	// +0x104 deleted by ExitInstance
	CRITICAL_SECTION m_csWorkerCmd;	// +0x11C deleted by ExitInstance
	CRITICAL_SECTION m_csInput;	// +0x134 (never deleted)
	CPlayerParams* m_pHumanParams;	// +0x14C active human block (g_H*Params)
	CPlayerParams* m_pRobotParams;	// +0x150 active robot block (g_R*Params)
	CRobotAIParams* m_pRobotAIParams;	// +0x154 active robot AI block
									//   (g_RobotAI*Params)
	DWORD m_dwSpeedPodTime;				// +0x158 = 360
	DWORD m_dwInvinciblePodTime;				// +0x15C = 512
	DWORD m_dwSlowPodTime;				// +0x160 = 360
	DWORD m_dwWallPodTime;				// +0x164 = 180
	DWORD m_dwSmokePodTime;				// +0x168 = 180
	DWORD m_dwUnk016C;				// +0x16C = 60
	DWORD m_avkControls[12];		// +0x170 control key bindings (virtual-key
									//   codes, set by Hover_LoadControls; not
									//   initialized by the ctor)
	double m_dblUnk01A0;			// +0x1A0 = 0.3
	double m_dblUnk01A8;			// +0x1A8 = 10.0
	double m_dblUnk01B0;			// +0x1B0 = 0.55
	DWORD m_dwGameState;				// +0x1B8
	DWORD m_bSuspended;				// +0x1BC
	DWORD m_bLevelBonus;				// +0x1C0
	DWORD m_dwSpeedDetail;				// +0x1C4 = 1
	DWORD m_dwSkill;				// +0x1C8 = 1 skill level (0=Beg, 1=Int, 2=Exp)
	DWORD m_dwMazeOrder;				// +0x1CC
	DWORD m_bRearView;				// +0x1D0 = (processor type != 486)
	DWORD m_bRotateMap;				// +0x1D4 = 1
	DWORD m_dwUnk01D8;				// +0x1D8 = 1
	DWORD m_dwUnk01DC;				// +0x1DC
	DWORD m_bShowBeacons;				// +0x1E0
	JOYCAPS m_joyCaps;				// +0x1E4 (hover.exe passes 0x194 bytes)
	JOYINFOEX m_joyInfo;				// +0x378 last joyGetPosEx result
	BOOL m_bJoystick;				// +0x3AC
	DWORD m_bFullScreen;				// +0x3B0
	DWORD m_bAppActive;				// +0x3B4 = 1 after InitInstance
	DWORD m_bSwitchingMode;				// +0x3B8 set while EnterFullScreen runs
	DWORD m_bFullScreenTopmost;				// +0x3BC ID_DBG_FULLSCREEN_TOPMOST: keep the full-screen
									//   window topmost
	// Windowed state saved by EnterFullScreen and restored by
	// LeaveFullScreen. The seven dwords are also still reachable as
	// m_adwSavedWindowState[0..6], which is how CBumperView::StopGameTimer gets
	// at the saved menu.
	union
	{
		DWORD m_adwSavedWindowState[7];		// +0x3C0
		struct
		{
			DWORD m_dwSavedStyle;		// +0x3C0 saved GWL_STYLE of the frame
			DWORD m_dwSavedExStyle;		// +0x3C4 saved GWL_EXSTYLE of the frame
			RECT m_rcSavedWindow;		// +0x3C8 saved frame window rectangle
			HMENU m_hSavedMenu;	// +0x3D8 saved frame menu
		};
	};
	LPCHANGEDISPLAYSETTINGS m_pfnChangeDisplaySettings; // +0x3DC
	DWORD m_bDisplayDibAvailable;				// +0x3E0
	DWORD m_bUseDisplayDib;				// +0x3E4
	DWORD m_bDisplayDibActive;				// +0x3E8
	BITMAPINFO* m_pbmiDisplayDib;			// +0x3EC DisplayDib BITMAPINFO buffer (BMI8_SIZE), freed by ExitInstance
	HWND m_hwndDisplayDib;				// +0x3F0
	DWORD m_dwWinVersion;				// +0x3F4 = GetVersion() (major << 8) | minor
	DWORD m_bNotNT;				// +0x3F8 = GetVersion() sign bit (Win95)
	DWORD m_bNewShell;				// +0x3FC = major version >= 4
	CBumperDoc* m_pBumperDoc;		// +0x400 the one document (set by its ctor;
									//   addressed absolutely as 0x460970 outside
									//   CHoverApp)
	DWORD m_bEngineSound;				// +0x404
	DWORD m_bGameSounds;				// +0x408
	DWORD m_bMusic;				// +0x40C MIDI music enable
	DWORD m_bMidiAvailable;				// +0x410 MIDI FM synth/mapper found
	CString m_strDataDir;			// +0x414 game data directory (set by FindDataDirectory;
									//   12 bytes in MFC 3.x, so +0x418 is its length)
};

extern CHoverApp theApp;			// hover.exe:0x460570 (.bss: constructed at startup)

/////////////////////////////////////////////////////////////////////////////
// Stub infrastructure
//
// Every function that has not yet been decompiled should call
// Hover_StubAlert() as its first statement. This makes stubs easy to find
// in the source and impossible to miss at runtime.

void Hover_StubAlert(const char* funcName, DWORD addr);

/////////////////////////////////////////////////////////////////////////////
// Free functions (defined in hover.cpp)

// Game settings persistence (registry)
// hover.exe:0x42D180
void Hover_LoadGameSettings();
// hover.exe:0x42D290
void Hover_SaveGameSettings();

// Worker thread and engine object shutdown (called by CHoverApp::ExitInstance)
// hover.exe:0x422E30
void Hover_ShutdownSound();

// Subsystem shutdown helpers called by CHoverApp::ExitInstance
// hover.exe:0x42C700
void Hover_FreeRegionNodePool();
// hover.exe:0x42C590
void Hover_FreeWallPool();
// hover.exe:0x42C2D0
void Hover_FreeRobotPool();
// hover.exe:0x42AED0
void Hover_FreePodObjPool();
// hover.exe:0x42B710
void Hover_FreeFloorObjPool();
// hover.exe:0x42F050
void Hover_FreeBeaconPool();
// hover.exe:0x42CB70
void Hover_FreeMerlinDynamicPool();

// Per-skill-level parameter block loaders called by CHoverApp::InitInstance
// hover.exe:0x4249C0
void Hover_LoadHumanParams();
// hover.exe:0x424A70
void Hover_LoadRobotParams();
// hover.exe:0x424B20
void Hover_LoadRobotAIParams();

// Subsystem initialization helpers called by CHoverApp::InitInstance
// hover.exe:0x4234F0
void Hover_LoadSoundSettings();
// hover.exe:0x423140
BOOL Hover_InitSound();
// hover.exe:0x438540
void Hover_BuildTrigTables();
// hover.exe:0x4385F0 fixed-point vector angle via the tangent table
// (scalar variant of Hover_FixAngleP, returns the 24.8 angle)
DWORD Hover_FixAngle(long lY, long lX);
// hover.exe:0x4386D0 fixed-point vector angle via the tangent table
DWORD* Hover_FixAngleP(DWORD* pdwOut, long lY, long lX);
// hover.exe:0x408A00 approximate vector magnitude: the max + min/2 metric
// refined by one Newton step against the sum of the 16-bit squares
DWORD Hover_ApproxDistance(long lX1, long lY1, long lX2, long lY2);
// hover.exe:0x40B6E0 split the velocity (dwVX, dwVY) into the components
// along (*pdwAlong) and across (*pdwSide) the given angle
void Hover_SplitVelocity(DWORD dwVX, DWORD dwVY, DWORD dwAngle,
	DWORD* pdwAlong, DWORD* pdwSide);
// hover.exe:0x41B7A0 *pdwOut = dwX*sin(dwAngle) + dwY*sin(dwAngle-0x80)
// (the velocity component tangential to dwAngle), all 16.16 fixed-point
void Hover_TangentialComponent(DWORD* pdwOut, DWORD dwX, DWORD dwY, DWORD dwAngle);
// hover.exe:0x41B830 arc test on two angles (full circle = 0x200)
int Hover_AngleInFrontArc(long lA, long lB);
// hover.exe:0x4232E0
void Hover_LoadControls();
// hover.exe:0x41F540
void Hover_InitZones();
// hover.exe:0x42DC50
int Hover_KeyNameToVK(UINT nID);
// hover.exe:0x40E560 activate/inactivate the WaveMix session
// (WaveMixActivate wrapper)
int Hover_WaveMixActivate(int nUnk);
// hover.exe:0x427A10 wait on an event, pumping WM_COPYDATA messages
// addressed to theApp.m_hwndDisplayDib while the wait times out
void Hover_WaitPumpCopyData(HANDLE hEvent);

// WaveMix sound engine (Hover_InitSound creates and starts the session)
// hover.exe:0x438820
void WaveMix_DebugOut(int nLevel, const char* pszMsg);
// hover.exe:0x42D6A0
UINT Hover_SoundThreadProc(LPVOID pParam);
// hover.exe:0x43D880
MIXSESSION* __stdcall WaveMixConfigureInit(MIXCONFIG* pCfg);
// hover.exe:0x43D3B0
int WaveMix_ReadConfigSettings(void* pv);
// hover.exe:0x4393A0
int __stdcall WaveMixOpenChannel(MIXSESSION* pEngine, int nChannel, int nMode);
// hover.exe:0x43A760
int __stdcall WaveMixCloseChannel(MIXSESSION* pEngine, int nChannel, int nFlags);
// hover.exe:0x43DAB0
int __stdcall WaveMixCloseSession(MIXSESSION* pEngine);
// hover.exe:0x439180
int __stdcall WaveMixActivate(MIXSESSION* pEngine, int bActivate);
// hover.exe:0x43A5A0
int __stdcall WaveMixFlushChannel(MIXSESSION* pEngine, int nChannel, int nFlags);
// hover.exe:0x43B8F0
void WaveMix_ShowWaveOutDevices();

// WaveMix mixer internals (called by WaveMixActivate/WaveMixFlushChannel)
// hover.exe:0x438840
int WaveMix_GetWaveOutPosition(HWAVEOUT hWaveOut, DWORD fGoodGetPos);
// hover.exe:0x4388C0
int WaveMix_AddFactor(int nA, int nB);
// hover.exe:0x4388E0
int WaveMix_SubFactor(int nA, int nB);
// hover.exe:0x438900
void WaveMix_SetWaveOutPosition(DWORD dwPos);
// hover.exe:0x4389F0
void WaveMix_CMixit(BYTE* pBuf, DWORD** ppSrcs, WORD* pwVols, int nSrcs,
	WORD nCount);
// hover.exe:0x438CA0
void WaveMix_InitChannelNodes();
// hover.exe:0x438F50
void WaveMix_FreeWaveOut(MIXSESSION* pEngine);
// hover.exe:0x439900
void WaveMix_FreePlayedChannelNodes();
// hover.exe:0x439010
int WaveMix_OpenWaveOut();
// hover.exe:0x439560
int WaveMix_MixerPlay(XWAVEHDR* pXWH, int bQueue);
// hover.exe:0x43B880
int WaveMix_RemoveInvalidIniNameCharacters(char* pszName);
// hover.exe:0x438D60
int WaveMix_AllocWaveBlocks(HWAVEOUT hWaveOut, XWAVEHDR** ppBlocks);
// hover.exe:0x438D00
void WaveMix_FreeWaveBlocks(HWAVEOUT hWaveOut, XWAVEHDR** ppBlocks);
// hover.exe:0x438E90
XWAVEHDR* WaveMix_RemoveFromPlayingQueue(XWAVEHDR* pXWH);

// WaveMix driver setup helpers called by WaveMix_ReadConfigSettings
// hover.exe:0x43B810
DWORD WaveMix_GetWaveBlockLen(DWORD dwBlockLen, LPWAVEFORMATEX pwfx);
// hover.exe:0x43B9C0
char* WaveMix_GetOperatingSystemPrefix();
// hover.exe:0x43BA10
int WaveMix_ReadRegistryToGetMachineSpecificInfSection(DWORD dwDevice, char* pszBuf, int cbBuf);
// hover.exe:0x43BAD0
int WaveMix_ReadRegistryInt(HKEY hKey, const char* pszValue, int nDefault);
// hover.exe:0x43BB20
int WaveMix_ReadRegistryForAppSpecificConfigs(void* pv);
// hover.exe:0x43BCF0
void WaveMix_ShowCurrentSettings();
// hover.exe:0x43C8C0
void WaveMix_DestroySettingsDlgTemplate(void* hTemplate);
// hover.exe:0x43C8F0
int __stdcall WaveMixConfigure(MIXSESSION* pEngine, DWORD dwA, void* pv,
	DWORD dwB, DWORD dwC);
// hover.exe:0x43CF50
int __stdcall WaveMixGetConfig(MIXSESSION* pEngine, MIXCONFIG* pCfg);
// hover.exe:0x43DB90
int WaveMix_SetIniFileName(HMODULE hMod);
// hover.exe:0x43DBE0
void WaveMix_InitVolumeTable();
// hover.exe:0x43DC40
int WaveMix_Startup(HMODULE hMod);
// hover.exe:0x439BF0
int WaveMix_NoResetRemix();
// hover.exe:0x439C20
int WaveMix_ResetRemix(DWORD dwPos, int bFlag);
// hover.exe:0x439A80
void WaveMixPump();
// hover.exe:0x439B60
LRESULT __stdcall WaveMix_WndProc(HWND hwnd, UINT uMsg, WPARAM wParam,
	LPARAM lParam);

// WaveMix channel node allocator / play entry points
// hover.exe:0x438CD0 pop a channel node off the g_pFreeChannelNodes free list
CHANNELNODE* WaveMix_GetChannelNode();
// hover.exe:0x439E40 reset the play position when nothing is queued
void WaveMix_ResetWavePosIfNoChannelData();
// hover.exe:0x439E80 WaveMixPlay
int __stdcall WaveMixPlay(MIXPLAYPARAMS* volatile pParams);

// WaveMix wave loading and format conversion (WaveMixOpenWave and its
// helpers); each conversion step returns the (possibly new) buffer and
// frees the old one, or NULL on failure
// hover.exe:0x43AA70 convert between 8- and 16-bit samples
void* WaveMix_BitsPerSampleAlign(void* pData, WORD wBitsFrom, WORD wBitsTo,
	DWORD* pcbData);
// hover.exe:0x43ABC0 convert between mono and stereo
void* WaveMix_ChannelAlign(void* pData, WORD wChFrom, WORD wChTo, WORD wBytes,
	DWORD* pcbData);
// hover.exe:0x43AD40 average nRatio frames into one (downsampling)
void WaveMix_AvgSample(BYTE* pDst, BYTE* pSrc, int nRatio, int nBytes,
	int nChannels);
// hover.exe:0x43ADF0 interpolate one frame out to nRatio (upsampling)
void WaveMix_RepSample(BYTE* pDst, BYTE* pSrc, int nRatio, int nBytes,
	int nChannels);
// hover.exe:0x43AEC0 convert the sampling rate
void* WaveMix_SamplesPerSecAlign(void* pData, DWORD dwRateFrom, DWORD dwRateTo,
	WORD wBytes, WORD wChannels, DWORD* pcbData);
// hover.exe:0x43B0A0 convert pData from the wave format to the session
// format (no-op when they already match)
void* WaveMix_ConvertWaveFormat(LPWAVEFORMATEX pwfxSession, LPWAVEFORMATEX pwfxWave,
	void* pData, DWORD* pcbData);
// hover.exe:0x43B150 WaveMixOpenWave
MIXWAVE* __stdcall WaveMixOpenWave(MIXSESSION* pEngine, const char* pszWave,
	HMODULE hModule, DWORD dwFlags);

/////////////////////////////////////////////////////////////////////////////
// WaveMixConfigure dialog cluster (defined in hover.cpp, called by
// WaveMixConfigure)

// hover.exe:0x43C5A0
void* WaveMix_MakeSettingsDlgTemplate();
// hover.exe:0x43C420
void* WaveMix_MakeDlgTemplate(DWORD* pcbTemplate, DWORD dwStyle, int x, int y,
	int cx, int cy, LPCWSTR pszTitle);
// hover.exe:0x43C4C0
void* WaveMix_AddDlgControl(DWORD* pcbTemplate, void* hTemplate, WORD wClass,
	DWORD dwStyle, WORD wId, int x, int y, int cx, int cy,
	LPCWSTR pszText);
// hover.exe:0x43C3C0
BOOL __stdcall WaveMix_SettingsDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam,
	LPARAM lParam);
// hover.exe:0x43BDF0
int WaveMix_SettingsOnInitDialog(HWND hwnd, WPARAM wParam, MIXCONFIG* pCfg);
// hover.exe:0x43C1C0
int WaveMix_SettingsOnCommand(HWND hwnd, DWORD dwId, LPARAM lParam, DWORD dwNotify);
// hover.exe:0x43D100
void WaveMix_SaveConfigSettings(DWORD dwFlags);

/////////////////////////////////////////////////////////////////////////////
// Renderer helper functions (defined in hover.cpp / bumpview.cpp)

// hover.exe:0x406C60 clear the secondary CGameObj list at g_pDrawObjList
void Hover_ReleaseDrawList();
// hover.exe:0x408780 process the primary CGameObj list and move the objects
// to the secondary list
void Hover_CommitDirtyObjects();

// hover.exe:0x401140
// Returns NULL when theApp.m_pMainWnd (hover.exe:0x46058C) is NULL, otherwise
// the frame's active view via CFrameWnd::GetActiveView (hover.exe:0x459D94).
CBumperView* Hover_GetView();

/////////////////////////////////////////////////////////////////////////////

#endif // __HOVER_H__
