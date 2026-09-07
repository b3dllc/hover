// mainfrm.h : interface of the CMainFrame class
//
// Skeleton reconstructed from hover.exe:
//   CRuntimeClass "CMainFrame" at 0x4C4808 (size 0x158, DYNCREATE)
//   vtable at 0x4BAB30
//   message map (AFX_MSGMAP) at 0x4BA8D0, entries at 0x4BA8D8
//
// The original compiler folded the constructor body into the
// IMPLEMENT_DYNCREATE Construct(void*) thunk at 0x41D600; the member
// offsets below are relative to that reconstruction and match our build
// exactly (same MFC 3.2 CFrameWnd base, size 0xC0).
//
// WM_ENTERSIZEMOVE (0x231), WM_EXITSIZEMOVE (0x232) and MM_WOM_DONE
// (0x3BD) are all declared by the VC++ 2.2 headers (WINUSER.H and
// MMSYSTEM.H), so the ON_MESSAGE entries below use the SDK names.

#ifndef __MAINFRM_H__
#define __MAINFRM_H__

class CBumperView;

// hover.exe:0x42D760 MM_WOM_DONE completion: look the finished MIXWAVE up
// in the wave map (hover.exe:0x4A4820) and close it again
void Hover_sub_42D760(void* pMixWave);

class CMainFrame : public CFrameWnd
{
	DECLARE_DYNCREATE(CMainFrame)

public:
	// Defined here, not in mainfrm.cpp: the original's constructor is
	// inline, so the only copy of its body in hover.exe is the one the
	// compiler expanded into the IMPLEMENT_DYNCREATE Construct(void*)
	// thunk at 0x41D600 (notes/perfect-match.md rule 221).
	CMainFrame()			// hover.exe:0x41D600
	{
		m_dwUnk00C0 = 0;
		m_dwUnk00C4 = 0;
		m_dwUnk00C8 = 0;
		m_dwUnk00D0 = 0;
	}
	// The compiler folded the destructor into the scalar deleting
	// destructor at hover.exe:0x41DFD0 (slot 1 of the vtable).
	virtual ~CMainFrame()	// hover.exe:0x41DFD0
	{
		if (m_dwUnk00C4 != 0)
			::FreeLibrary((HINSTANCE)m_dwUnk00C4);
	}

	// hover.exe:0x408A70 show pszText in pane nPane of the status bar
	// (called by CBumperView::OnSize through GetParentFrame())
	void Sub_408A70(int nPane, const char* pszText);

// Overrides
	// hover.exe:0x41DDF0 drop FWS_ADDTOTITLE and place the frame at the
	// size the CPU can drive, or at the rectangle saved in the registry
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	// hover.exe:0x41E070 reparent the already-created view into the frame
	virtual BOOL OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext);

// Implementation
public:
	// hover.exe:0x426CF0 realize the game palette on the frame, black out
	// the view's client area, then put the old palette back
	void Sub_426CF0();

	// hover.exe:0x41D850 turn the client rectangle in *lpRect into the
	// window rectangle the current display mode wants: it picks the mode
	// table entry (hover.exe:0x4609A0) closest to the requested client
	// size, adds the non-client metrics with AdjustWindowRectEx and the
	// status bar height, and either offsets the rect (bCenter == 0) or
	// centres it on the screen.
	void Sub_41D850(LPRECT lpRect, BOOL bCenter, DWORD dwStyle,
		DWORD dwExStyle);

protected:
	//{{AFX_MSG(CMainFrame)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);	// hover.exe:0x41DB90
	afx_msg void OnSize(UINT nType, int cx, int cy);		// hover.exe:0x41DCE0
	afx_msg void OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI);	// hover.exe:0x417AF0
	afx_msg void OnHelp();	// hover.exe:0x426BA0 (ID_HELP and
									// ID_DEFAULT_HELP, i.e. F1 and the fallback)
	afx_msg void OnHelpIndex();		// hover.exe:0x426BC0
	afx_msg void OnHelpHowToPlay();	// hover.exe:0x426C00
	afx_msg void OnActivateApp(BOOL bActive, HTASK hTask);	// hover.exe:0x415300
	afx_msg void OnCmd34010();		// hover.exe:0x426C20
	afx_msg void OnUpdateCmd34010(CCmdUI* pCmdUI);	// hover.exe:0x426CA0
	afx_msg BOOL OnQueryNewPalette();		// hover.exe:0x40EE60
	afx_msg void OnPaletteChanged(CWnd* pFocusWnd);	// hover.exe:0x4142C0
	afx_msg void OnClose();			// hover.exe:0x41D6B0
	afx_msg void OnHelpUsing();		// hover.exe:0x426BE0
	afx_msg void OnInitMenu(CMenu* pMenu);	// hover.exe:0x40E2B0
	afx_msg void OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex,
		BOOL bSysMenu);				// hover.exe:0x40E2C0
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);	// hover.exe:0x41DB20
	afx_msg void OnMove(int x, int y);					// hover.exe:0x426E00
	afx_msg void OnMenuSelect(UINT nItemID, UINT nFlags, HMENU hSysMenu);
										// hover.exe:0x426B60
	//}}AFX_MSG
	afx_msg LRESULT OnWomDone(WPARAM wParam, LPARAM lParam);	// hover.exe:0x426B80
	afx_msg LRESULT OnExitSizeMove(WPARAM wParam, LPARAM lParam); // hover.exe:0x426A60
	afx_msg LRESULT OnEnterSizeMove(WPARAM wParam, LPARAM lParam); // hover.exe:0x426B40
	DECLARE_MESSAGE_MAP()

public:
// Member variables (hover.exe offsets in comments)
	DWORD m_dwUnk00C0;		// +0xC0 = 0
	DWORD m_dwUnk00C4;		// +0xC4 = 0
	DWORD m_dwUnk00C8;		// +0xC8 = 0 set between WM_ENTERSIZEMOVE and
							// WM_EXITSIZEMOVE; suppresses the OnSize snap
	DWORD m_dwUnk00CC;		// +0xCC = 1 while the status bar is managed by
							// the frame (cleared by the ID_CMD_34010 toggle)
	DWORD m_dwUnk00D0;		// +0xD0 = 0 set once m_rcUnk00D4 is valid
	RECT m_rcUnk00D4;		// +0xD4 saved window rect, restored by OnClose
							// when the window is iconic
	CStatusBar m_wndStatusBar;	// +0xE4 (hover.exe CRuntimeClass "CStatusBar"
							// at 0x4C7118, size 0x74: 0xE4+0x74 = 0x158)
};							// sizeof = 0x158 in hover.exe

#endif // __MAINFRM_H__
