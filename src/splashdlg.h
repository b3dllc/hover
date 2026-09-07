// splashdlg.h : startup dialogs for the Hover! application.
//
// This is a reconstructed version of the original Hover! for Windows 95
// source code, produced by manual decompilation of hover.exe.
//
// CSplashDialog (vtable at hover.exe:0x4BEAA0, message map at
// hover.exe:0x4BE9A8, sizeof = 0xD4) is the modeless splash screen shown
// while the game starts up; it destroys itself after four seconds or on
// any key press.
//
// CQuickHelpDialog (vtable at hover.exe:0x4BFCC8, message map at
// hover.exe:0x4BFC80) is the "Quick Help" instructions dialog shown when
// the splash screen closes, unless the user checked "Skip Quick Help at
// Startup" (persisted in HKCU\Software\Microsoft\Hover!\QuickHelpFlag).

#ifndef __SPLASHDLG_H__
#define __SPLASHDLG_H__

#include <afxwin.h>

#include "resource.h"

/////////////////////////////////////////////////////////////////////////////
// CSplashDialog dialog
//
// Member offsets in hover.exe are given in the comments. The dialog is
// 0xD4 bytes in hover.exe; our build's MFC 4.2 CDialog base class is a
// different size than the original's, so our offsets will not match.

class CSplashDialog : public CDialog
{
// Construction
public:
	CSplashDialog(CWnd* pParent = NULL);	// hover.exe:0x425120

// Dialog Data
	//{{AFX_DATA(CSplashDialog)
	enum { IDD = IDD_SPLASH };
	CStatic m_staticDivider;	// +0x44 IDC_SPLASH_DIVIDER (etched line)
	CStatic m_staticFrame;		// +0x74 IDC_SPLASH_FRAME (black frame)
	//}}AFX_DATA

// Implementation
protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// hover.exe:0x424260
	virtual BOOL OnInitDialog();	// hover.exe:0x424290
	virtual void PostNcDestroy();	// hover.exe:0x4245E0

	//{{AFX_MSG(CSplashDialog)
	afx_msg void OnPaint();			// hover.exe:0x415220
	afx_msg void OnDestroy();		// hover.exe:0x422FB0
	afx_msg void OnTimer(UINT nIDEvent);	// hover.exe:0x4245D0
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags); // hover.exe:0x42F600
	afx_msg void OnPaletteChanged(CWnd* pFocusWnd);	// hover.exe:0x4152E0
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

// Member variables (hover.exe offsets in comments)
	HDC m_hMemDC;			// +0xA4 memory DC the splash image is drawn from
	HBITMAP m_hBitmap;		// +0xA8 DIB section holding the splash image
	HGDIOBJ m_hOldBitmap;	// +0xAC previous bitmap in m_hMemDC
	HPALETTE m_hOldPalette;	// +0xB0 previous palette in m_hMemDC
	LPBYTE m_pDib;			// +0xB4 locked IDB_SPLASH resource
	DWORD m_dwImageX;		// +0xB8 splash image position (dialog client coords)
	DWORD m_dwImageY;		// +0xBC
	DWORD m_dwImageCX;		// +0xC0 splash image size
	DWORD m_dwImageCY;		// +0xC4
	CPalette m_palette;		// +0xC8 palette from the splash bitmap's color table
	UINT m_nTimer;			// +0xD0 auto-close timer id
};

/////////////////////////////////////////////////////////////////////////////
// CQuickHelpDialog dialog

class CQuickHelpDialog : public CDialog
{
// Construction
public:
	CQuickHelpDialog(CWnd* pParent = NULL);	// hover.exe:0x424EC0

// Dialog Data
	//{{AFX_DATA(CQuickHelpDialog)
	enum { IDD = IDD_QUICKHELP };
	//}}AFX_DATA

// Implementation
	int Sub_424F30();		// hover.exe:0x424F30 read the QuickHelpFlag registry value

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// hover.exe:0x4342C0
	virtual BOOL OnInitDialog();	// hover.exe:0x434360
	virtual void OnOK();			// hover.exe:0x4342E0

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

#endif // __SPLASHDLG_H__
