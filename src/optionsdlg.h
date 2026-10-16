// optionsdlg.h : the option and debug dialogs opened by the CHoverApp
// command handlers.
//
// This is a reconstructed version of the original Hover! for Windows 95
// source code, produced by manual decompilation of hover.exe.
//
// None of these dialogs has a CRuntimeClass entry in hover.exe, so their
// original names are unknown and the classes are named after their
// constructors.
//
// hover.exe's CDialog is 0x44 bytes and so is ours, and its CWnd (the
// base of every embedded control object) is 0x30, so the member offsets
// in the comments below are directly comparable.
//
// The four debug dialogs (CPodTimesDialog, CHumanParamsDialog,
// CRobotParamsDialog and CRobotAIParamsDialog) run on templates 130, 148, 149
// and 152, which were stripped from the shipped hover.exe: DoModal() returns
// -1 on them at runtime and nothing they do is ever visible. They are
// decompiled anyway.

#ifndef __OPTIONSDLG_H__
#define __OPTIONSDLG_H__

#include <afxwin.h>
#include <afxcmn.h>		// CSliderCtrl, CSpinButtonCtrl

#include "resource.h"

class CPlayerControlsDialog;

/////////////////////////////////////////////////////////////////////////////
// CCustomizeDialog: the "Customize Game" dialog (ID_OPTIONS_CUSTOMIZE).
// vtable at hover.exe:0x4BD6E0; the embedded CSliderCtrl at +0x44 has its
// own vtable at hover.exe:0x4BD670. The three int members are seeded from
// the settings in theApp; only m_nRearView is read back (through DDX), the
// radio groups are exchanged by hand.

class CCustomizeDialog : public CDialog
{
// Construction
public:
	CCustomizeDialog(CWnd* pParent = NULL);	// hover.exe:0x413F10

// Dialog Data
	//{{AFX_DATA(CCustomizeDialog)
	enum { IDD = IDD_CUSTOMIZE };
	CSliderCtrl m_sliderSpeed;	// +0x44 IDC_CUST_SPEEDDETAIL
	int m_nRearView;			// +0x74 IDC_CUST_REARVIEW
	//}}AFX_DATA

// Implementation
protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// hover.exe:0x40E480
	virtual BOOL OnInitDialog();	// hover.exe:0x413FD0
	virtual void OnOK();			// hover.exe:0x413FE0

	//{{AFX_MSG(CCustomizeDialog)
	afx_msg LRESULT OnHelpInfo(WPARAM wParam, LPARAM lParam);	// hover.exe:0x42D140
	afx_msg LRESULT OnHelpContextMenu(WPARAM wParam, LPARAM lParam); // hover.exe:0x42D160
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()	// map at hover.exe:0x4BD598

public:
	int m_nSkill;			// +0x78 copy of theApp.m_dwSkill (write-only)
	int m_nMazeType;		// +0x7C copy of theApp.m_dwMazeOrder (write-only)
	DWORD m_dwUnk80;		// +0x80 never touched by any of the class's code
};							// sizeof = 0x84 in hover.exe

/////////////////////////////////////////////////////////////////////////////
// CPodTimesDialog: the debug dialog opened by ID_DBG_PODTIMES (template
// 130). CHoverApp::OnDbgPodTimes exchanges the five pod effect durations
// of the application object (in frames: speed 360, invincible 512, slow
// 360, temporary wall 180, smoke bomb 180) and three floating-point
// tuning values that nothing else reads. The five integer members are int
// rather than DWORD because the original calls the signed
// DDX_Text/DDV_MinMaxInt overloads.

class CPodTimesDialog : public CDialog
{
public:
	CPodTimesDialog(CWnd* pParent = NULL);	// hover.exe:0x42C890

	enum { IDD = IDD_DBG_PODTIMES };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// hover.exe:0x42C920

	DECLARE_MESSAGE_MAP()	// empty map at hover.exe:0x4BD208

public:
	int m_nInvincibleTime;		// +0x44 <-> theApp.m_dwInvinciblePodTime (IDC 1008)
	int m_nSlowTime;		// +0x48 <-> theApp.m_dwSlowPodTime (IDC 1007)
	int m_nSmokeTime;		// +0x4C <-> theApp.m_dwSmokePodTime (IDC 1010)
	int m_nSpeedTime;		// +0x50 <-> theApp.m_dwSpeedPodTime (IDC 1006)
	int m_nWallTime;		// +0x54 <-> theApp.m_dwWallPodTime (IDC 1011)
	double m_dblUnk58;	// +0x58 <-> theApp.m_dblUnk01A0 (IDC 1017)
	double m_dblUnk60;	// +0x60 <-> theApp.m_dblUnk01B0 (IDC 1019)
	double m_dblUnk68;	// +0x68 <-> theApp.m_dblUnk01A8 (IDC 1018)
};						// sizeof = 0x70 in hover.exe

/////////////////////////////////////////////////////////////////////////////
// CPlayerControlsDialog: the "Player Controls" dialog (ID_OPTIONS_CONTROLS).
// The five embedded controls have the CButton (hover.exe:0x4BDDA0) and
// CComboBox (hover.exe:0x4BDC98) vtables.

class CPlayerControlsDialog : public CDialog
{
// Construction
public:
	CPlayerControlsDialog(CWnd* pParent = NULL);	// hover.exe:0x42D910

// Dialog Data
	//{{AFX_DATA(CPlayerControlsDialog)
	enum { IDD = IDD_PLAYERCONTROLS };
	CButton m_btnJoystick;		// +0x44  IDC_CTRL_JOYSTICK
	CButton m_btnCalibrate;		// +0x74  IDC_CTRL_CALIBRATE
	CComboBox m_comboWall;		// +0xA4  IDC_CTRL_WALL
	CComboBox m_comboJump;		// +0xD4  IDC_CTRL_JUMP
	CComboBox m_comboCloak;		// +0x104 IDC_CTRL_CLOAK
	//}}AFX_DATA

// Implementation
public:
	// hover.exe:0x42DB60 sorts a copy of the 11 chosen virtual key codes
	// and returns whether they are all distinct
	BOOL AreKeysUnique(DWORD* avk, int nCount);
	// hover.exe:0x42DBE0 selects the combo box item whose item data is dwVK
	void SelectKey(CWnd* pCombo, DWORD dwVK);
	// hover.exe:0x42DD70 fills a key combo box with "A".."Z", "0".."9" and
	// (unless bLettersOnly) the named keys of g_aKeyNames, each carrying
	// its virtual key code as item data, then selects dwVK
	void FillKeyCombo(CWnd* pCombo, DWORD dwVK, BOOL bLettersOnly);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// hover.exe:0x42DAE0
	virtual BOOL OnInitDialog();	// hover.exe:0x42DEF0
	virtual void OnOK();			// hover.exe:0x42E0F0

	//{{AFX_MSG(CPlayerControlsDialog)
	afx_msg void OnSetKeys();		// hover.exe:0x42DFF0 IDC_CTRL_SETKEYS
	afx_msg void OnDefault();		// hover.exe:0x42E2D0 IDC_CTRL_DEFAULT
	afx_msg void OnCalibrate();		// hover.exe:0x42E3F0 IDC_CTRL_CALIBRATE
	afx_msg LRESULT OnHelpInfo(WPARAM wParam, LPARAM lParam);	// hover.exe:0x42E4F0
	afx_msg LRESULT OnHelpContextMenu(WPARAM wParam, LPARAM lParam); // hover.exe:0x42E510
	afx_msg void OnJoystick();		// hover.exe:0x42E530 IDC_CTRL_JOYSTICK
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()	// map at hover.exe:0x4BDA18
};							// sizeof = 0x134 in hover.exe

/////////////////////////////////////////////////////////////////////////////
// CKeyboardDialog: the "Keyboard Settings" dialog (template 103) put up by
// CPlayerControlsDialog::OnSetKeys. vtable at hover.exe:0x4BF2E8.
//
// NOTE: m_pParentDlg (+0x44) is never assigned by anything - neither the
// constructor nor CPlayerControlsDialog::OnSetKeys touch it - yet every
// helper call below goes through it. That is harmless only because
// AreKeysUnique, SelectKey and FillKeyCombo never dereference `this`.

class CKeyboardDialog : public CDialog
{
// Construction
public:
	CKeyboardDialog(CWnd* pParent = NULL);	// hover.exe:0x42FE20

// Dialog Data
	//{{AFX_DATA(CKeyboardDialog)
	enum { IDD = IDD_KEYBOARD };
	CPlayerControlsDialog* m_pParentDlg;	// +0x44 (never initialized, see above)
	DWORD m_dwJumpKey;			// +0x48 the three object keys of the parent
	DWORD m_dwCloakKey;			// +0x4C dialog, so that the duplicate check
	DWORD m_dwWallKey;			// +0x50 in OnOK covers all eleven bindings
	CComboBox m_comboRevRight;	// +0x54  IDC_KEY_REVRIGHT
	CComboBox m_comboRevLeft;	// +0x84  IDC_KEY_REVLEFT
	CComboBox m_comboLeft;		// +0xB4  IDC_KEY_LEFT
	CComboBox m_comboRight;		// +0xE4  IDC_KEY_RIGHT
	CComboBox m_comboReverse;	// +0x114 IDC_KEY_REVERSE
	CComboBox m_comboFwdRight;	// +0x144 IDC_KEY_FWDRIGHT
	CComboBox m_comboFwdLeft;	// +0x174 IDC_KEY_FWDLEFT
	CComboBox m_comboForward;	// +0x1A4 IDC_KEY_FORWARD
	//}}AFX_DATA

// Implementation
protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// hover.exe:0x4300E0
	virtual BOOL OnInitDialog();	// hover.exe:0x4304B0
	virtual void OnOK();			// hover.exe:0x430190

	//{{AFX_MSG(CKeyboardDialog)
	afx_msg void OnDefault();		// hover.exe:0x430580 IDC_KEY_DEFAULT
	afx_msg LRESULT OnHelpInfo(WPARAM wParam, LPARAM lParam);	// hover.exe:0x430610
	afx_msg LRESULT OnHelpContextMenu(WPARAM wParam, LPARAM lParam); // hover.exe:0x430630
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()	// map at hover.exe:0x4BF178
};							// sizeof = 0x1D4 in hover.exe

/////////////////////////////////////////////////////////////////////////////
// CSoundOptionsDialog: the "Sound Options" dialog (ID_OPTIONS_SOUNDS).

class CSoundOptionsDialog : public CDialog
{
// Construction
public:
	CSoundOptionsDialog(CWnd* pParent = NULL);	// hover.exe:0x42E790

// Dialog Data
	//{{AFX_DATA(CSoundOptionsDialog)
	enum { IDD = IDD_SOUNDOPTIONS };
	CButton m_btnEngine;		// +0x44  IDC_SOUND_ENGINE
	CButton m_btnGameSounds;	// +0x74  IDC_SOUND_GAMESOUNDS
	CButton m_btnMixing;		// +0xA4  IDC_SOUND_MIXING
	CButton m_btnMusic;			// +0xD4  IDC_SOUND_MUSIC
	int m_nEngine;				// +0x104 <-> theApp.m_bEngineSound
	int m_nMusic;				// +0x108 <-> theApp.m_bMusic
	int m_nMixing;				// +0x10C <-> g_bUseWaveMix
	int m_nGameSounds;			// +0x110 <-> theApp.m_bGameSounds
	//}}AFX_DATA

// Implementation
protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// hover.exe:0x42E920
	virtual BOOL OnInitDialog();	// hover.exe:0x42EA00
	virtual void OnOK();			// hover.exe:0x42EB40

	//{{AFX_MSG(CSoundOptionsDialog)
	afx_msg LRESULT OnHelpInfo(WPARAM wParam, LPARAM lParam);	// hover.exe:0x42EAA0
	afx_msg LRESULT OnHelpContextMenu(WPARAM wParam, LPARAM lParam); // hover.exe:0x42EAC0
	afx_msg void OnMixing();		// hover.exe:0x42EAE0 IDC_SOUND_MIXING
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()	// map at hover.exe:0x4BDF78
};							// sizeof = 0x114 in hover.exe

/////////////////////////////////////////////////////////////////////////////
// CStartAtDialog: the "Start At" dialog (ID_GAME_STARTAT). On OK it leaves the
// chosen level in g_dwStartAtLevel. The vtables of the two embedded controls
// are at hover.exe:0x4BEE40 (CEdit) and 0x4BEDD0 (CSpinButtonCtrl).

class CStartAtDialog : public CDialog
{
// Construction
public:
	CStartAtDialog(CWnd* pParent = NULL);	// hover.exe:0x42F640

// Dialog Data
	//{{AFX_DATA(CStartAtDialog)
	enum { IDD = IDD_STARTAT };
	CButton m_btnCustomize;		// +0x44  IDC_STARTAT_CUSTOMIZE
	CButton m_btnOK;			// +0x74  IDOK
	CEdit m_editLevel;			// +0xA4  IDC_STARTAT_LEVEL
	CSpinButtonCtrl m_spinLevel;// +0xD4  IDC_STARTAT_SPIN
	UINT m_nLevel;				// +0x104 <-> g_dwStartAtLevel (1..16)
	//}}AFX_DATA

// Implementation
public:
	// hover.exe:0x42FA80 formats dwScore with thousands separators into
	// the static control nID
	void SetScoreText(UINT nID, DWORD dwScore);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// hover.exe:0x42F7D0
	virtual BOOL OnInitDialog();	// hover.exe:0x42F890
	virtual void OnOK();			// hover.exe:0x42FCC0

	//{{AFX_MSG(CStartAtDialog)
	afx_msg LRESULT OnHelpInfo(WPARAM wParam, LPARAM lParam);	// hover.exe:0x42F850
	afx_msg LRESULT OnHelpContextMenu(WPARAM wParam, LPARAM lParam); // hover.exe:0x42F870
	afx_msg void OnCustomize();		// hover.exe:0x42F990 IDC_STARTAT_CUSTOMIZE
	afx_msg void OnChangeLevel();	// hover.exe:0x42FB60 EN_CHANGE on the level edit
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()	// map at hover.exe:0x4BEC50
};							// sizeof = 0x108 in hover.exe

/////////////////////////////////////////////////////////////////////////////
// CHumanParamsDialog: the human-player parameter debug dialog opened by
// ID_DBG_HUMANPARAMS (template 148). The 30 doubles are the ten used entries of
// each of g_HBegParams, g_HExpParams and g_HIntParams, converted
// from 16.16 fixed point and rounded to four decimals.

class CHumanParamsDialog : public CDialog
{
public:
	CHumanParamsDialog(CWnd* pParent = NULL);	// hover.exe:0x430780

	enum { IDD = IDD_DBG_HUMANPARAMS };

	// hover.exe:0x4315E0 writes the 30 doubles back into the three
	// parameter blocks as 16.16 fixed point
	void ApplyParams();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// hover.exe:0x430930
	virtual BOOL OnInitDialog();	// hover.exe:0x430B80
	virtual void OnOK();			// hover.exe:0x431880

	//{{AFX_MSG(CHumanParamsDialog)
	afx_msg void OnDeleteSettings();	// hover.exe:0x4318A0 (button 4)
	afx_msg void OnSaveSettings();		// hover.exe:0x431900 (button 3)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()	// map at hover.exe:0x4BF400

public:
	double m_adblParams[30];	// +0x48 (+0x44 is alignment padding)
};								// sizeof = 0x138 in hover.exe

/////////////////////////////////////////////////////////////////////////////
// CRobotParamsDialog: the robot-player parameter debug dialog opened by
// ID_DBG_ROBOTPARAMS (template 149). 33 doubles = 11 entries of each of
// g_RBegParams, g_RExpParams and g_RIntParams; entries 7 and 8 of
// every block are neither exchanged nor initialized here, so they keep
// their constructor defaults (0.66 and 1.33).

class CRobotParamsDialog : public CDialog
{
public:
	CRobotParamsDialog(CWnd* pParent = NULL);	// hover.exe:0x4319A0

	enum { IDD = IDD_DBG_ROBOTPARAMS };

	void ApplyParams();		// hover.exe:0x432660 (see ApplyParams)

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// hover.exe:0x431B20
	virtual BOOL OnInitDialog();	// hover.exe:0x431D10
	virtual void OnOK();			// hover.exe:0x432930

	//{{AFX_MSG(CRobotParamsDialog)
	afx_msg void OnDeleteSettings();	// hover.exe:0x432950 (button 4)
	afx_msg void OnSaveSettings();		// hover.exe:0x4329B0 (button 3)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()	// map at hover.exe:0x4BF450

public:
	double m_adblParams[33];	// +0x48
};								// sizeof = 0x150 in hover.exe

/////////////////////////////////////////////////////////////////////////////
// CRobotAIParamsDialog: the robot-AI parameter debug dialog opened by
// ID_DBG_ROBOTAIPARAMS (template 152). Each of the three blocks (Beg, Int,
// Exp) is one integer, two 16.16 fixed-point values and seven more integers.

class CRobotAIParamsDialog : public CDialog
{
public:
	CRobotAIParamsDialog(CWnd* pParent = NULL);	// hover.exe:0x432A50

	enum { IDD = IDD_DBG_ROBOTAIPARAMS };

	void ApplyParams();		// hover.exe:0x432DB0 (see ApplyParams)

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// hover.exe:0x432B70
	virtual BOOL OnInitDialog();	// hover.exe:0x432F50
	virtual void OnOK();			// hover.exe:0x432F30

	//{{AFX_MSG(CRobotAIParamsDialog)
	afx_msg void OnDeleteSettings();	// hover.exe:0x433270 (button 4)
	afx_msg void OnSaveSettings();		// hover.exe:0x4332D0 (button 3)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()	// map at hover.exe:0x4BF4A0

public:
	// Beginner block (g_RobotAIBegParams)
	int m_nBeg1;			// +0x44 <- [1]
	double m_dblBeg2;		// +0x48 <- [2]
	double m_dblBeg0;		// +0x50 <- [0]
	int m_nBeg9;			// +0x58 <- [9]
	int m_nBeg4;			// +0x5C <- [4]
	int m_nBeg3;			// +0x60 <- [3]
	int m_nBeg7;			// +0x64 <- [7]
	int m_nBeg6;			// +0x68 <- [6]
	int m_nBeg5;			// +0x6C <- [5]
	int m_nBeg8;			// +0x70 <- [8]
	// Intermediate block (g_RobotAIIntParams)
	int m_nInt1;			// +0x74
	double m_dblInt2;		// +0x78
	double m_dblInt0;		// +0x80
	int m_nInt9;			// +0x88
	int m_nInt4;			// +0x8C
	int m_nInt3;			// +0x90
	int m_nInt7;			// +0x94
	int m_nInt6;			// +0x98
	int m_nInt5;			// +0x9C
	int m_nInt8;			// +0xA0
	// Expert block (g_RobotAIExpParams)
	int m_nExp1;			// +0xA4
	double m_dblExp2;		// +0xA8
	double m_dblExp0;		// +0xB0
	int m_nExp9;			// +0xB8
	int m_nExp4;			// +0xBC
	int m_nExp3;			// +0xC0
	int m_nExp7;			// +0xC4
	int m_nExp6;			// +0xC8
	int m_nExp5;			// +0xCC
	int m_nExp8;			// +0xD0
};							// sizeof = 0xD8 in hover.exe

/////////////////////////////////////////////////////////////////////////////
// CLoadingDialog: the modeless "Please Wait" dialog (template 143) that
// CHoverApp::StartLevel puts up while a level loads. vtable at
// hover.exe:0x4BF090; the single CStatic at +0x44 (vtable 0x4BEB28) wraps
// the SS_BLACKFRAME control the progress bar is painted into by the load
// progress callback Hover_LoadProgress.

class CLoadingDialog : public CDialog
{
public:
	CLoadingDialog(CWnd* pParent = NULL);	// hover.exe:0x424720

	enum { IDD = IDD_LOADING };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// hover.exe:0x4247D0
	virtual BOOL OnInitDialog();	// hover.exe:0x4247F0
	// hover.exe's vtable also has a DestroyWindow override at 0x42FE00
	// (a bare jmp to CWnd::DestroyWindow, so behaviourally inherited);
	// declaring it here costs register allocation elsewhere (rule 20)

	DECLARE_MESSAGE_MAP()	// empty map at hover.exe:0x4BF038

public:
	CStatic m_staticFrame;	// +0x44 IDC_LOADING_FRAME
};							// sizeof = 0x74 in hover.exe

// hover.exe:0x4C57D4 the loading dialog while it is up (Hover_LoadProgress
// paints the progress bar into it and does nothing when it is NULL)
extern CLoadingDialog* g_pLoadingDlg;

// The two halves of the load progress counter, latched by every call of
// Hover_LoadProgress (the "n of m" it was passed). g_dwLoadDone is clamped
// to g_dwLoadTotal, which is forced to at least 1 so the percentage
// division cannot divide by zero.
extern DWORD g_dwLoadDone;	// hover.exe:0x4C57C4 items done
extern DWORD g_dwLoadTotal;	// hover.exe:0x4C57C8 items total

// hover.exe:0x40EB50 texture load progress callback (LPPROGRESSFN): draws
// the "n of m" progress bar into g_pLoadingDlg's frame control
void __cdecl Hover_LoadProgress(DWORD dwDone, DWORD dwTotal);

/////////////////////////////////////////////////////////////////////////////

#endif // __OPTIONSDLG_H__
