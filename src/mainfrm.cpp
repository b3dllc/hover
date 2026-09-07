// mainfrm.cpp : implementation of the CMainFrame class
//
// Reconstructed from hover.exe; see mainfrm.h for the RTTI/vtable data.
// The functions are ordered by their address in the original.

#include "hover.h"
#include "mainfrm.h"
#include "bumperdoc.h"
#include "bumpview.h"
#include "highscrdlg.h"

#include <stdlib.h>

// IMPLEMENT_DYNCREATE hand-expanded: its placement-new thunk
// CMainFrame::Construct is defined further down so that it can carry the
// reccmp marker for hover.exe:0x41D600 (rule 221).
_IMPLEMENT_RUNTIMECLASS(CMainFrame, CFrameWnd, 0xFFFF, CMainFrame::Construct)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
	//{{AFX_MSG_MAP(CMainFrame)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_GETMINMAXINFO()
	ON_COMMAND(ID_HELP, OnHelp)
	ON_COMMAND(ID_HELP_INDEX, OnHelpIndex)
	ON_COMMAND(ID_HELP_HOWTOPLAY, OnHelpHowToPlay)
	ON_WM_ACTIVATEAPP()
	ON_COMMAND(ID_CMD_34010, OnCmd34010)
	ON_UPDATE_COMMAND_UI(ID_CMD_34010, OnUpdateCmd34010)
	ON_WM_QUERYNEWPALETTE()
	ON_WM_PALETTECHANGED()
	ON_WM_CLOSE()
	ON_COMMAND(ID_HELP_USING, OnHelpUsing)
	ON_WM_INITMENU()
	ON_WM_INITMENUPOPUP()
	ON_WM_SYSCOMMAND()
	ON_WM_MOVE()
	ON_COMMAND(ID_DEFAULT_HELP, OnHelp)
	ON_WM_MENUSELECT()
	//}}AFX_MSG_MAP
	ON_MESSAGE(MM_WOM_DONE, OnWomDone)
	ON_MESSAGE(WM_EXITSIZEMOVE, OnExitSizeMove)
	ON_MESSAGE(WM_ENTERSIZEMOVE, OnEnterSizeMove)
END_MESSAGE_MAP()

// Status bar panes: a stretched separator followed by two fixed-width
// separators and the four debug indicators (hover.exe:0x4C4820).
// hover.exe:0x4C4820
static UINT indicators[] =
{
	ID_SEPARATOR,
	ID_SEPARATOR,
	ID_SEPARATOR,
	ID_DBG_WALL,
	ID_DBG_POD,
	ID_DBG_FLOOR,
	ID_DBG_CAR,
};

// Set to 1 by CMainFrame::Sub_41D850 the first time it runs (the stubbed
// helper is the only user); kept here because it lives between the
// indicator array and the resize guard in the original's .data.
// hover.exe:0x4C483C
static DWORD g_dwUnk_4C483C = 1;

// Nonzero while OnSize's own SetWindowPos call is in flight, so that the
// WM_SIZE it generates does not recurse.
// hover.exe:0x4C4840
static DWORD g_dwUnk_4C4840 = 0;

// The six board zone descriptors at hover.exe:0x4609A0 are one flat int
// array; ZONE_INTS is the size of one block in ints (cf. hover.cpp).
#define ZONE_INTS (0x338 / 4)

/////////////////////////////////////////////////////////////////////////////
// CMainFrame message handlers

// Nothing is left of this handler but the default processing; the game is
// paused from OnInitMenuPopup/OnMenuSelect instead.
// hover.exe:0x40E2B0
// FUNCTION: HOVER 0x40e2b0
void CMainFrame::OnInitMenu(CMenu* pMenu)
{
	Default();
}

// hover.exe:0x40E2C0
// FUNCTION: HOVER 0x40e2c0
void CMainFrame::OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex,
	BOOL bSysMenu)
{
	CBumperView* pView;

	pView = Hover_sub_401140();
	if (bSysMenu && pView != NULL)
		pView->Sub_40EE90(1);
	CFrameWnd::OnInitMenuPopup(pPopupMenu, nIndex, bSysMenu);
}

// Realize the game palette on the frame and force the view to repaint with
// it. The return value tells Windows the palette actually changed.
//
// hover.exe:0x40EE60
// FUNCTION: HOVER 0x40ee60
BOOL CMainFrame::OnQueryNewPalette()
{
	CBumperView* pView;

	Sub_426CF0();
	pView = Hover_sub_401140();
	if (pView != NULL)
		::RedrawWindow(pView->m_hWnd, NULL, NULL,
			RDW_INVALIDATE | RDW_ERASE | RDW_UPDATENOW);
	return TRUE;
}

// hover.exe:0x4142C0
// FUNCTION: HOVER 0x4142c0
void CMainFrame::OnPaletteChanged(CWnd* pFocusWnd)
{
	CBumperView* pView;

	Default();
	if (this == pFocusWnd)
		return;

	Sub_426CF0();
	pView = Hover_sub_401140();
	if (pView != NULL)
		::RedrawWindow(pView->m_hWnd, NULL, NULL,
			RDW_INVALIDATE | RDW_ERASE | RDW_UPDATENOW);
}

// Pauses the game whenever the application loses or gains the foreground,
// drops out of full screen on deactivation and resynchronizes the render
// thread when the activation state really changed.
// hover.exe:0x415300
// FUNCTION: HOVER 0x415300
void CMainFrame::OnActivateApp(BOOL bActive, HTASK hTask)
{
	BOOL bChanged;
	CBumperView* pView;

	bChanged = ((bActive == 0) != (theApp.m_dwUnk03B4 == 0));
	theApp.m_dwUnk03B4 = bActive;

	pView = Hover_sub_401140();
	if (pView != NULL && pView->m_dwUnk0040 != 0)
	{
		pView->Sub_40EE90(1);
		if (bActive == 0 && theApp.m_dwUnk03B0 != 0)
			theApp.Sub_425600();
		if (bChanged)
			pView->Sub_40DC20(1);
	}
	Default();
}

// Like OnInitMenu, this handler does nothing but the default processing
// (the frame's size limits come out of Sub_41D850 instead).
// hover.exe:0x417AF0
// FUNCTION: HOVER 0x417af0
void CMainFrame::OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI)
{
	Default();
}

// The IMPLEMENT_DYNCREATE placement-construction thunk. CMainFrame's
// constructor is inline (mainfrm.h), so the compiler expands its body -
// the CFrameWnd base call, the CStatusBar member at +0xE4, the vtable
// store and the four zeroed members - straight into this thunk, guarded
// by the null test that `new(p)` needs; that is all hover.exe:0x41D600 is.
// hover.exe:0x41D600
// FUNCTION: HOVER 0x41d600
void PASCAL CMainFrame::Construct(void* p)
{
	new(p) CMainFrame;
}

// hover.exe:0x41D6B0
// FUNCTION: HOVER 0x41d6b0
void CMainFrame::OnClose()
{
	Hover_sub_401140()->Sub_40EE90(1);

	if (theApp.m_dwUnk01B8 != 0) {
		CSub424BD0 dlg(NULL);

		if (AfxMessageBox(IDS_PRESS_F2, MB_YESNO, (UINT)-1) == IDNO)
			return;

		if (dlg.Sub_424DF0(theApp.m_pBumperDoc->m_dwUnk83E4))
			dlg.DoModal();

		theApp.m_pBumperDoc->m_dwUnk83E4 = 0;
	}

	if (theApp.m_dwUnk03B0 == 0) {
		RECT rect;
		BOOL bSave = TRUE;

		if (::IsIconic(m_hWnd)) {
			if (m_dwUnk00D0)
				rect = m_rcUnk00D4;
			else
				bSave = FALSE;
		} else {
			::GetWindowRect(m_hWnd, &rect);
		}

		if (bSave) {
			HKEY hKey;
			DWORD dwDisposition;
			if (RegCreateKeyExA(HKEY_CURRENT_USER,
					"Software\\Microsoft\\Hover!", 0, NULL, 0, KEY_ALL_ACCESS,
					NULL, &hKey, &dwDisposition) == ERROR_SUCCESS) {
				RegSetValueExA(hKey, "ScreenPosition", 0, REG_BINARY,
					(LPBYTE)&rect, sizeof(rect));
				RegCloseKey(hKey);
			}
		}
	}

	CFrameWnd::OnClose();
}

// Turns the client rectangle in *lpRect into the window rectangle the
// display mode wants. The six board zone descriptors (hover.exe:0x4609A0)
// double as the list of supported client sizes: +0x00/+0x04 are the zone's
// client width and height (320x240, 416x312, 512x384, 556x417, 640x480 and
// 700x525) and +0x10/+0x14 the midpoint between this zone and the next,
// i.e. the size at which the game snaps to the next zone up.
//
// The requested width and height each pick a zone; when both already agree
// with the zone in use (g_dwUnk_460998) the request is a resize by the
// user, so the mode moves one step in the direction it was dragged,
// otherwise the zone that actually changed wins. Full-screen mode
// (theApp.m_dwUnk03B0) is nailed to zone 4 (640x480), and a 640-pixel wide
// desktop caps the choice at zone 3, which is the largest window that
// still fits with its border. g_dwUnk_4C483C makes the very first call
// take the width's zone unchanged.
//
// The zone's client size is then grown by AdjustWindowRectEx (using the
// active view's own styles when there is one) and by the status bar's
// height, and the result is either offset onto the incoming rectangle's
// origin (bCenter == 0) or centred on the screen.

// hover.exe:0x41D850
// FUNCTION: HOVER 0x41d850
void CMainFrame::Sub_41D850(LPRECT lpRect, BOOL bCenter, DWORD dwStyle,
	DWORD dwExStyle)
{
	int nZone;
	int nZoneCY;
	int nZoneCX;
	int nZoneY;
	int nMaxZone;
	int nDX;
	int nDY;
	RECT rect;
	CView* pView;
	BOOL bMenu;
	DWORD dwViewStyle;
	DWORD dwViewExStyle;

	if (theApp.m_dwUnk03B0 != 0)
	{
		nZone = 4;
	}
	else
	{
		nMaxZone = 5;
		if (::GetSystemMetrics(SM_CXSCREEN) == 640)
			nMaxZone = 3;

		for (nZone = 0; nZone < nMaxZone; nZone++)
		{
			if (g_adwUnk_4609A0[nZone * ZONE_INTS + 0x10 / 4] >=
					lpRect->right - lpRect->left)
				break;
		}
		if (nZone > nMaxZone)
			nZone = nMaxZone;

		for (nZoneY = 0; nZoneY < nMaxZone; nZoneY++)
		{
			if (g_adwUnk_4609A0[nZoneY * ZONE_INTS + 0x14 / 4] >=
					lpRect->bottom - lpRect->top)
				break;
		}
		if (nZoneY > nMaxZone)
			nZoneY = nMaxZone;

		if (g_dwUnk_4C483C != 0)
		{
			g_dwUnk_4C483C = 0;
		}
		else
		{
			nZoneCX = g_adwUnk_4609A0[nZone * ZONE_INTS];
			nDX = abs(lpRect->right - lpRect->left - nZoneCX);
			nZoneCY = g_adwUnk_4609A0[nZoneY * ZONE_INTS + 1];
			nDY = abs(lpRect->bottom - lpRect->top - nZoneCY);

			if (nZone == (int)g_dwUnk_460998 && nZoneY == (int)g_dwUnk_460998 &&
				(nDX != 0 || nDY != 0))
			{
				// A plain resize: step one zone in the dragged direction.
				// The two size differences below are read through a
				// volatile RECT so that VC 2.2 recomputes them from
				// *lpRect, as the original does. Without it the compiler
				// reassociates the abs() expressions above into
				// (right - left) - nZoneCX, finds the difference common to
				// all three uses and spills it to two extra stack slots,
				// which shifts every frame offset in the function. No
				// non-volatile spelling of the abs() expression (statement
				// splitting, compound assignment, reparenthesizing) avoids
				// the reassociation.
				if (nDY > nDX)
				{
					if (((volatile RECT*)lpRect)->bottom -
							((volatile RECT*)lpRect)->top < nZoneCY)
						nZone = (nZoneY > 0) ? nZoneY - 1 : 0;
					else
						nZone = (nZoneY < nMaxZone) ? nZoneY + 1 : nMaxZone;
				}
				else
				{
					if (((volatile RECT*)lpRect)->right -
							((volatile RECT*)lpRect)->left < nZoneCX)
						nZone = (nZone > 0) ? nZone - 1 : 0;
					else
						nZone = (nZone < nMaxZone) ? nZone + 1 : nMaxZone;
				}
			}
			else if (nZone == (int)g_dwUnk_460998)
			{
				nZone = nZoneY;
			}
			else if (nZoneY != (int)g_dwUnk_460998)
			{
				if (nZoneY <= nZone)
					nZoneY = nZone;
				nZone = nZoneY;
			}
		}
	}

	pView = NULL;
	rect.left = 0;
	rect.top = 0;
	rect.right = g_adwUnk_4609A0[nZone * ZONE_INTS];
	rect.bottom = g_adwUnk_4609A0[nZone * ZONE_INTS + 1];
	if (m_hWnd != NULL)
		pView = GetActiveView();

	if (pView != NULL)
	{
		dwViewStyle = ::GetWindowLong(pView->m_hWnd, GWL_STYLE);
		dwViewExStyle = ::GetWindowLong(pView->m_hWnd, GWL_EXSTYLE);
	}
	else
	{
		dwViewStyle = WS_CHILD | WS_BORDER;
		dwViewExStyle = theApp.m_dwUnk03FC ? WS_EX_CLIENTEDGE : 0;
	}
	::AdjustWindowRectEx(&rect, dwViewStyle & ~WS_BORDER, FALSE,
		dwViewExStyle);

	if (m_wndStatusBar.m_hWnd != NULL &&
		::IsWindowVisible(m_wndStatusBar.m_hWnd))
	{
		RECT rectBar;

		::GetWindowRect(m_wndStatusBar.m_hWnd, &rectBar);
		rect.bottom += rectBar.bottom - rectBar.top;
	}

	if (m_hWnd != NULL)
		bMenu = (GetMenu() != NULL);
	else
		bMenu = FALSE;
	::AdjustWindowRectEx(&rect, dwStyle, bMenu, dwExStyle);

	if (bCenter == 0)
	{
		lpRect->left += rect.left;
		lpRect->top += rect.top;
	}
	else
	{
		lpRect->left =
			(::GetSystemMetrics(SM_CXSCREEN) - rect.right + rect.left) / 2;
		lpRect->top =
			(::GetSystemMetrics(SM_CYSCREEN) - rect.bottom + rect.top) / 2;
	}
	lpRect->right = lpRect->left - rect.left + rect.right;
	lpRect->bottom = lpRect->top - rect.top + rect.bottom;
}

// Sizes and places the frame window before it is created. The frame drops
// FWS_ADDTOTITLE (the game maintains its own "Hover! - Level n" caption),
// then picks a default client size from the processor type -- 416x312 on a
// 486, 512x384 on anything faster -- and hands the resulting rectangle to
// Sub_41D850, which snaps it to a board zone and centres it.
//
// If HKCU\Software\Microsoft\Hover!\ScreenPosition holds a 16-byte RECT
// from a previous run, that rectangle is used instead (after being nudged
// back onto the screen), and Sub_41D850 is skipped altogether, so the
// saved window rectangle survives verbatim.

// hover.exe:0x41DDF0
// FUNCTION: HOVER 0x41ddf0
BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	HKEY hKey;
	DWORD cbData;
	DWORD dwType;
	RECT rcSaved;
	RECT rect;
	SYSTEM_INFO sysInfo;
	int bDefaultRect = TRUE;
	int cxScreen;
	int cyScreen;
	int nAdjust;
	HDC hDC;

	cs.style &= ~FWS_ADDTOTITLE;
	if (!CFrameWnd::PreCreateWindow(cs))
		return FALSE;

	rect.left = cs.x;
	rect.top = cs.y;
	::GetSystemInfo(&sysInfo);
	if (sysInfo.dwProcessorType == PROCESSOR_INTEL_486)
	{
		rect.right = cs.x + 416;
		rect.bottom = cs.y + 312;
	}
	else
	{
		rect.right = cs.x + 512;
		rect.bottom = cs.y + 384;
	}

	if (::RegOpenKeyExA(HKEY_CURRENT_USER, "Software\\Microsoft\\Hover!", 0,
			KEY_ALL_ACCESS, &hKey) == ERROR_SUCCESS)
	{
		dwType = REG_BINARY;
		cbData = sizeof(rcSaved);
		if (::RegQueryValueExA(hKey, "ScreenPosition", NULL, &dwType,
				(LPBYTE)&rcSaved, &cbData) == ERROR_SUCCESS &&
			cbData == sizeof(rcSaved))
		{
			hDC = ::GetDC(NULL);
			cxScreen = ::GetDeviceCaps(hDC, HORZRES);
			cyScreen = ::GetDeviceCaps(hDC, VERTRES);
			::ReleaseDC(NULL, hDC);

			// slide the saved rectangle back onto the screen; note the
			// original really does write "x -= (x - limit)" rather than
			// assigning the limit directly
			if (cxScreen < rcSaved.right)
			{
				nAdjust = rcSaved.right - cxScreen;
				rcSaved.right -= nAdjust;
				rcSaved.left -= nAdjust;
			}
			if (cyScreen < rcSaved.bottom)
			{
				nAdjust = rcSaved.bottom - cyScreen;
				rcSaved.bottom -= nAdjust;
				rcSaved.top -= nAdjust;
			}
			if (rcSaved.left < 0)
			{
				nAdjust = -rcSaved.left;
				rcSaved.left += nAdjust;
				rcSaved.right += nAdjust;
			}
			if (rcSaved.top < 0)
			{
				nAdjust = -rcSaved.top;
				rcSaved.top += nAdjust;
				rcSaved.bottom += nAdjust;
			}
			rect = rcSaved;
			bDefaultRect = FALSE;
		}
		::RegCloseKey(hKey);
	}

	if (bDefaultRect)
		Sub_41D850(&rect, TRUE, cs.style, cs.dwExStyle);

	cs.x = rect.left;
	cs.y = rect.top;
	cs.cx = rect.right - rect.left;
	cs.cy = rect.bottom - rect.top;
	return TRUE;
}

// Creates the frame's client area. m_dwUnk00C0 is never set in the code we
// have decompiled so far, so in practice this only forwards to the base
// class; when it is set, the view has already been created as a top-level
// window and is merely reparented into the frame (and loses its client
// edge) instead of being created by CFrameWnd::OnCreateClient.

// hover.exe:0x41E070
// FUNCTION: HOVER 0x41e070
BOOL CMainFrame::OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext)
{
	CBumperView* pView;

	if (m_dwUnk00C0 != 0)
	{
		pView = Hover_sub_401140();
		pView->SetParent(this);
		pView->ModifyStyleEx(WS_EX_CLIENTEDGE, 0);
	}
	return CFrameWnd::OnCreateClient(lpcs, pContext);
}

// Releases the help/extension module m_dwUnk00C4 refers to. Like
// m_dwUnk00C0 above, nothing in the decompiled code ever loads it, so the
// destructor body is dead in practice; the member destructors for the
// status bar and the CFrameWnd base that follow it are generated.

// hover.exe:0x41DFD0
// The destructor is defined in the class declaration, so the compiler folds
// it into the scalar deleting destructor that the vtable slot holds - which
// is what hover.exe:0x41DFD0 is. The block below emits no code; it only gives
// the reccmp marker a declaration to name ??_GCMainFrame@@ (rules 34/184).
// hover.exe:0x41DFD0
#if 0
// FUNCTION: HOVER 0x41dfd0
CMainFrame::~CMainFrame()
{
	if (m_dwUnk00C4 != 0)
		::FreeLibrary((HINSTANCE)m_dwUnk00C4);
}
#endif

// Windows asks before starting the screen saver: refuse while a game is
// running, otherwise leave full screen first and let the saver through.
// hover.exe:0x41DB20
// FUNCTION: HOVER 0x41db20
void CMainFrame::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == SC_SCREENSAVE)
	{
		::OutputDebugString("ScreenSaver Starting...\n");
		if (theApp.m_dwUnk01B8 != 0 &&
			(theApp.m_dwUnk01B8 & 0x80000000) == 0)
		{
			::OutputDebugString("Disabling Screen Saver\n");
			return;
		}
		::OutputDebugString("Allowing Screen Saver\n");
		if (theApp.m_dwUnk03B0 != 0)
			theApp.Sub_425600();
	}
	CFrameWnd::OnSysCommand(nID, lParam);
}

// Creates the status bar (hidden: the ID_CMD_34010 debug command toggles
// it) and sizes its three separator panes from the width of a digit.
// hover.exe:0x41DB90
// FUNCTION: HOVER 0x41db90
int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	CFont* pOldFont;
	int cxDigit;

	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	if (!m_wndStatusBar.Create(this, WS_CHILD | CBRS_BOTTOM) ||
		!m_wndStatusBar.SetIndicators(indicators,
			sizeof(indicators) / sizeof(UINT)))
	{
		return -1;		// fail to create
	}

	m_dwUnk00CC = 1;
	m_wndStatusBar.SetPaneInfo(0, 0, SBPS_NOBORDERS | SBPS_STRETCH, 10);

	CClientDC dc(&m_wndStatusBar);
	pOldFont = dc.SelectObject(CFont::FromHandle(
		(HFONT)m_wndStatusBar.SendMessage(WM_GETFONT)));
	cxDigit = dc.GetTextExtent("0", 1).cx;
	m_wndStatusBar.SetPaneInfo(1, 0, 0, cxDigit * 9);
	m_wndStatusBar.SetPaneInfo(2, 0, 0, cxDigit * 4);
	dc.SelectObject(pOldFont);

	return 0;
}

// Snaps the frame back to the window size the current display mode wants
// (g_dwUnk_4C4840 guards against the recursion through SetWindowPos) and
// hands the new client size to the renderer.
// hover.exe:0x41DCE0
// FUNCTION: HOVER 0x41dce0
void CMainFrame::OnSize(UINT nType, int cx, int cy)
{
	CBumperView* pView;
	RECT rect;
	int cxWindow;

	CFrameWnd::OnSize(nType, cx, cy);

	if (nType == SIZE_MINIMIZED || nType == SIZE_MAXHIDE)
		return;
	if (g_dwUnk_4C4840 != 0)
		return;
	if (m_dwUnk00C8 != 0)
		return;

	pView = Hover_sub_401140();
	::GetClientRect((pView == NULL) ? m_hWnd : pView->m_hWnd, &rect);
	Sub_41D850(&rect, 0, ::GetWindowLong(m_hWnd, GWL_STYLE),
		::GetWindowLong(m_hWnd, GWL_EXSTYLE));

	cxWindow = rect.right - rect.left;
	if (cxWindow != cx || cy != rect.bottom - rect.top)
	{
		g_dwUnk_4C4840 = 1;
		::SetWindowPos(m_hWnd, NULL, 0, 0, cxWindow,
			rect.bottom - rect.top,
			SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
		g_dwUnk_4C4840 = 0;
	}

	if (pView != NULL)
	{
		m_dwUnk00D0 = 1;
		::GetWindowRect(m_hWnd, &m_rcUnk00D4);
		pView->Sub_41F480();
	}
}

// WM_EXITSIZEMOVE: the user has let go of the frame, so re-apply the
// display mode's window size (the OnSize snap is disabled while the modal
// resize loop runs) and restart the renderer at the new size.
// hover.exe:0x426A60
// FUNCTION: HOVER 0x426a60
LRESULT CMainFrame::OnExitSizeMove(WPARAM wParam, LPARAM lParam)
{
	CBumperView* pView;
	RECT rectClient;
	RECT rectWindow;

	pView = Hover_sub_401140();
	::GetClientRect((pView == NULL) ? m_hWnd : pView->m_hWnd, &rectClient);
	rectWindow = rectClient;
	Sub_41D850(&rectWindow, 0, ::GetWindowLong(m_hWnd, GWL_STYLE),
		::GetWindowLong(m_hWnd, GWL_EXSTYLE));
	m_dwUnk00C8 = 0;

	if (rectClient.right - rectClient.left ==
			rectWindow.right - rectWindow.left &&
		rectClient.bottom - rectClient.top ==
			rectWindow.bottom - rectWindow.top)
	{
		if (pView != NULL)
			pView->Sub_41F480();
	}
	else
	{
		::SetWindowPos(m_hWnd, NULL, 0, 0,
			rectWindow.right - rectWindow.left,
			rectWindow.bottom - rectWindow.top,
			SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
	}

	return Default();
}

// WM_ENTERSIZEMOVE: suppress the OnSize snap for the duration of the modal
// move/size loop.
// hover.exe:0x426B40
// FUNCTION: HOVER 0x426b40
LRESULT CMainFrame::OnEnterSizeMove(WPARAM wParam, LPARAM lParam)
{
	m_dwUnk00C8 = 1;
	return Default();
}

// hover.exe:0x426B60
// FUNCTION: HOVER 0x426b60
void CMainFrame::OnMenuSelect(UINT nItemID, UINT nFlags, HMENU hSysMenu)
{
	CBumperView* pView;

	pView = Hover_sub_401140();
	if (pView != NULL)
		pView->Sub_40EE90(1);
}

// MM_WOM_DONE: lParam is the MIXWAVE the mixer has finished with; hand it
// to the sound engine's completion routine.
// hover.exe:0x426B80
// FUNCTION: HOVER 0x426b80
LRESULT CMainFrame::OnWomDone(WPARAM wParam, LPARAM lParam)
{
	Hover_sub_42D760((void*)lParam);
	return 0;
}

// hover.exe:0x426BA0
// FUNCTION: HOVER 0x426ba0
void CMainFrame::OnHelp()
{
	::WinHelp(GetSafeHwnd(), "hover.hlp", HELP_FINDER, 0);
}

// The original opens the help finder here too, not the (obsolete) index.
// hover.exe:0x426BC0
// FUNCTION: HOVER 0x426bc0
void CMainFrame::OnHelpIndex()
{
	::WinHelp(GetSafeHwnd(), "hover.hlp", HELP_FINDER, 0);
}

// hover.exe:0x426BE0
// FUNCTION: HOVER 0x426be0
void CMainFrame::OnHelpUsing()
{
	::WinHelp(GetSafeHwnd(), "hover.hlp", HELP_HELPONHELP, 0);
}

// hover.exe:0x426C00
// FUNCTION: HOVER 0x426c00
void CMainFrame::OnHelpHowToPlay()
{
	::WinHelp(GetSafeHwnd(), "hover.hlp", HELP_CONTEXT, 1000);
}

// Debug command: toggle the status bar. The frame is then resized by one
// pixel in height so that OnSize runs and snaps the window back to the
// size the display mode wants with the new status bar height.
// hover.exe:0x426C20
// FUNCTION: HOVER 0x426c20
void CMainFrame::OnCmd34010()
{
	RECT rect;
	int cx;
	int cy;
	int nShow;

	if (m_wndStatusBar.m_hWnd == NULL)
		return;

	m_dwUnk00CC = 0;
	nShow = ((::GetWindowLong(m_wndStatusBar.m_hWnd, GWL_STYLE) & WS_VISIBLE)
		>> 28) ? SW_HIDE : SW_SHOW;
	::ShowWindow(m_wndStatusBar.m_hWnd, nShow);
	::GetWindowRect(m_hWnd, &rect);
	cy = rect.bottom - rect.top + 1;
	cx = rect.right - rect.left;
	::SetWindowPos(m_hWnd, NULL, 0, 0, cx, cy,
		SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
}

// hover.exe:0x426CA0
// FUNCTION: HOVER 0x426ca0
void CMainFrame::OnUpdateCmd34010(CCmdUI* pCmdUI)
{
	if (m_wndStatusBar.m_hWnd == NULL)
	{
		pCmdUI->SetCheck(0);
		pCmdUI->Enable(FALSE);
	}
	else
	{
		pCmdUI->SetCheck(
			(::GetWindowLong(m_wndStatusBar.m_hWnd, GWL_STYLE) & WS_VISIBLE)
				>> 28);
		pCmdUI->Enable(TRUE);
	}
}

// Display-mode/palette helper: realize the game palette on the frame,
// black out the view's client area, then put the old palette back. The
// original takes `this` in ecx and hands it straight to the CClientDC
// constructor, which is what makes this a CMainFrame member and not the
// free function it looks like at the call sites in bumpview.cpp.
// hover.exe:0x426CF0
// FUNCTION: HOVER 0x426cf0
void CMainFrame::Sub_426CF0()
{
	CClientDC dc(this);
	// Frame placeholder, not a variable of the original (rule 80): the
	// original keeps the second RealizePalette result below in a dead
	// frame slot it never reads back.
	volatile BOOL bUnused;
	CPalette* pOldPal = dc.SelectPalette(&g_paletteUnk_4A1D50, TRUE);
	if (RealizePalette(dc.m_hDC))
	{
		CBumperView* pView = Hover_sub_401140();
		if (pView != NULL)
		{
			CClientDC viewDC(pView);
			CRect rc;
			pView->GetClientRect(&rc);
			viewDC.PatBlt(rc.left, rc.top, rc.Width(), rc.Height(), BLACKNESS);
		}
	}
	dc.SelectPalette(&g_paletteUnk_4A1D50, FALSE);
	bUnused = RealizePalette(dc.m_hDC);
	dc.SelectPalette(pOldPal, TRUE);
}

// Remembers the window position for the registry. Note the quirk: when a
// view exists the *view's* window rectangle is saved, not the frame's
// (OnSize saves the frame's).
// hover.exe:0x426E00
// FUNCTION: HOVER 0x426e00
void CMainFrame::OnMove(int x, int y)
{
	CBumperView* pView;

	Default();
	if (::IsIconic(m_hWnd))
		return;

	m_dwUnk00D0 = 1;
	pView = Hover_sub_401140();
	if (pView == NULL)
		::GetWindowRect(m_hWnd, &m_rcUnk00D4);
	else
		::GetWindowRect(pView->m_hWnd, &m_rcUnk00D4);
}

// Displays pszText in pane nPane of the status bar, if the status bar has
// already been created.

// hover.exe:0x408A70
// FUNCTION: HOVER 0x408a70
void CMainFrame::Sub_408A70(int nPane, const char* pszText)
{
	if (m_wndStatusBar.m_hWnd != NULL)
		m_wndStatusBar.SetPaneText(nPane, pszText, TRUE);
}
