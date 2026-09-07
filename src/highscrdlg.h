// highscrdlg.h : high score dialogs for the Hover! application.
//
// This is a reconstructed version of the original Hover! for Windows 95
// source code, produced by manual decompilation of hover.exe.
//
// Neither class has a CRuntimeClass entry in hover.exe (vtable slot 0
// points at CDialog::GetRuntimeClass), so they were not declared with
// DECLARE_DYNAMIC and their original names are unknown.
//
// CSub424BD0 (vtable at hover.exe:0x4BF9E8, message map at
// hover.exe:0x4BF850, sizeof = 0x138) is the "High Scores" dialog
// (template IDD_HIGHSCORES = 108). It keeps the 15 names and scores in
// memory, loads/saves them under HKCU\Software\Microsoft\Hover!
// ("HighScoreNames" REG_MULTI_SZ / "HighScores" REG_BINARY), and is also
// used by CMainFrame::OnClose to offer entering a new score at exit.
//
// CSub434170 (vtable at hover.exe:0x4BFBC8, message map at
// hover.exe:0x4BFB78, sizeof = 0x50) is the "Enter High Score" name
// entry dialog (template IDD_ENTERHIGHSCORE = 112).
//
// Member offsets in hover.exe are given in the comments. Our MFC 3.2
// build has the same 12-byte CString layout (m_pchData, length, alloc
// length) as the original, so the offsets after m_astrNames match as
// well; only the CDialog base size differs slightly.

#ifndef __HIGHSCRDLG_H__
#define __HIGHSCRDLG_H__

#include <afxwin.h>

#include "resource.h"

/////////////////////////////////////////////////////////////////////////////
// CSub424BD0 dialog ("High Scores")

class CSub424BD0 : public CDialog
{
// Construction
public:
	CSub424BD0(CWnd* pParent = NULL);	// hover.exe:0x424BD0
	virtual ~CSub424BD0();				// hover.exe:0x41D540

// Dialog Data
	//{{AFX_DATA(CSub424BD0)
	enum { IDD = IDD_HIGHSCORES };
	//}}AFX_DATA

// Implementation
	void Sub_424C80();				// hover.exe:0x424C80 load the names and
									// scores from the registry (defaults on failure)
	BOOL Sub_424DF0(DWORD dwScore);	// hover.exe:0x424DF0 offer the name entry
									// dialog when dwScore makes the list

	// hover.exe:0x433860 format dwScore with thousands separators into
	// the static control nID
	void Sub_433860(UINT nID, DWORD dwScore);
	// hover.exe:0x433940 sort the entries after an insert/reset (highest
	// score first) and refresh the dialog controls when m_dwUnk134 is set
	void Sub_433940();
	// hover.exe:0x433CF0 clear (bReset) or load the built-in default
	// names/scores (scores are shuffled with rand())
	void Sub_433CF0(BOOL bReset);
	// hover.exe:0x433F10 save the names and scores to the registry
	void Sub_433F10();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// hover.exe:0x433840 (empty)
	virtual void OnOK();			// hover.exe:0x434050
	virtual BOOL OnInitDialog();		// hover.exe:0x433EE0

	//{{AFX_MSG(CSub424BD0)
	afx_msg LRESULT Sub_434010(WPARAM wParam, LPARAM lParam);	// hover.exe:0x434010 WM_HELP
	afx_msg LRESULT Sub_434030(WPARAM wParam, LPARAM lParam);	// hover.exe:0x434030 WM_CONTEXTMENU
	afx_msg void Sub_433C60();		// hover.exe:0x433C60 IDC_CLEARSCORES
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

// Member variables (hover.exe offsets in comments)
	CString m_astrNames[15];	// +0x44 (element size 0xC in hover.exe)
	DWORD m_adwScores[15];		// +0xF8 (the last entry, +0x130, is the
								// lowest score on the list: a new score must
								// beat it to qualify)
	DWORD m_dwUnk134;			// +0x134 = 0; set to 1 by OnInitDialog
};								// sizeof = 0x138 in hover.exe

/////////////////////////////////////////////////////////////////////////////
// CSub434170 dialog ("Enter High Score")

class CSub434170 : public CDialog
{
// Construction
public:
	CSub434170(CWnd* pParent = NULL);	// hover.exe:0x434170
	virtual ~CSub434170();				// hover.exe:0x434100

// Dialog Data
	//{{AFX_DATA(CSub434170)
	enum { IDD = IDD_ENTERHIGHSCORE };
	CString m_strName;			// +0x44 entered name, max 26 characters
	//}}AFX_DATA

// Implementation
protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// hover.exe:0x434200

	DECLARE_MESSAGE_MAP()
};								// sizeof = 0x50 in hover.exe

/////////////////////////////////////////////////////////////////////////////

#endif // __HIGHSCRDLG_H__
