// splashdlg.cpp : startup dialogs for the Hover! application.
//

#include "splashdlg.h"
#include "hover.h"
#include "bumpview.h"

extern CPoint g_ptSplashAnchor;		// hover.exe:0x4A4878 (hover.cpp)

#include <stdlib.h>

/////////////////////////////////////////////////////////////////////////////
// CSplashDialog dialog

// hover.exe:0x425120
// FUNCTION: HOVER 0x425120
CSplashDialog::CSplashDialog(CWnd* pParent /*=NULL*/)
	: CDialog(IDD_SPLASH, pParent)
{
	//{{AFX_DATA_INIT(CSplashDialog)
	//}}AFX_DATA_INIT
	m_nTimer = 0;
}

BEGIN_MESSAGE_MAP(CSplashDialog, CDialog)
	//{{AFX_MSG_MAP(CSplashDialog)
	ON_WM_PAINT()
	ON_WM_DESTROY()
	ON_WM_TIMER()
	ON_WM_KEYDOWN()
	ON_WM_PALETTECHANGED()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSplashDialog message handlers

// hover.exe:0x424260
// FUNCTION: HOVER 0x424260
void CSplashDialog::DoDataExchange(CDataExchange* pDX)
{
	//{{AFX_DATA_MAP(CSplashDialog)
	DDX_Control(pDX, IDC_SPLASH_DIVIDER, m_staticDivider);
	DDX_Control(pDX, IDC_SPLASH_FRAME, m_staticFrame);
	//}}AFX_DATA_MAP
}

// hover.exe:0x424290
// FUNCTION: HOVER 0x424290
BOOL CSplashDialog::OnInitDialog()
{
	// The original constructs an empty CString here that is never used
	CString strUnused;

	CDialog::OnInitDialog();

	// close the splash screen automatically after four seconds
	m_nTimer = ::SetTimer(m_hWnd, 1, 4000, NULL);

	// Load the splash bitmap as a raw DIB. The original re-reads theApp's
	// own m_hInstance at each call rather than caching it in a local, and
	// goes through the MFC-style FindResource wrapper at hover.exe:0x453352,
	// not AfxGetInstanceHandle()/FindResource.
	HRSRC hRsrc = Hover_FindResource((HINSTANCE)theApp.m_hInstance,
		MAKEINTRESOURCE(IDB_SPLASH), RT_BITMAP);
	HGLOBAL hDib = LoadResource((HINSTANCE)theApp.m_hInstance, hRsrc);
	m_pDib = (LPBYTE)LockResource(hDib);
	BITMAPINFOHEADER* pHeader = (BITMAPINFOHEADER*)m_pDib;
	RGBQUAD* pColors = (RGBQUAD*)(m_pDib + sizeof(BITMAPINFOHEADER));

	// build a 256 entry logical palette from the DIB's color table; the
	// size includes the one PALETTEENTRY already counted in sizeof(LOGPALETTE)
	// (hover.exe:0x424306 pushes 0x408, one entry more than the strict
	// 4 + 256*sizeof(PALETTEENTRY) == 0x404 would need)
	LOGPALETTE* pLogPal = (LOGPALETTE*)::operator new(sizeof(LOGPALETTE) + 256 * sizeof(PALETTEENTRY));
	pLogPal->palVersion = 0x300;
	pLogPal->palNumEntries = 256;
	int nColors = pHeader->biClrUsed;
	if (nColors == 0)
		nColors = 256;
	memset(pLogPal->palPalEntry, 0, 256 * sizeof(PALETTEENTRY));
	for (int i = 0; i < nColors; i++)
	{
		pLogPal->palPalEntry[i].peRed = pColors[i].rgbRed;
		pLogPal->palPalEntry[i].peGreen = pColors[i].rgbGreen;
		pLogPal->palPalEntry[i].peBlue = pColors[i].rgbBlue;
		pLogPal->palPalEntry[i].peFlags = 0;
	}
	m_palette.CreatePalette(pLogPal);
	::operator delete(pLogPal);

	// copy the DIB's pixel bits into a DIB section
	HDC hScreen = ::GetDC(NULL);
	LPVOID pBits;
	m_hBitmap = CreateDIBSection(hScreen, (BITMAPINFO*)m_pDib,
		DIB_RGB_COLORS, &pBits, NULL, 0);
	DWORD cbBits = pHeader->biWidth * pHeader->biHeight;
	memcpy(pBits, (LPBYTE)pColors + nColors * sizeof(RGBQUAD), cbBits);

	// resize the dialog to fit the bitmap and center it on the screen
	RECT rcDivider;
	m_staticDivider.GetClientRect(&rcDivider);
	int cx = pHeader->biWidth + 10;
	int cy = pHeader->biHeight + (rcDivider.bottom - rcDivider.top) + 20;
	int y = GetDeviceCaps(hScreen, VERTRES) / 2 - cy / 2;
	int x = GetDeviceCaps(hScreen, HORZRES) / 2 - cx / 2;
	MoveWindow(x, y, cx, cy, TRUE);

	// center the etched divider line near the bottom of the dialog
	RECT rcDlg;
	GetClientRect(&rcDlg);
	int xDiv = ((rcDivider.left - rcDivider.right) -
		rcDlg.left + rcDlg.right) / 2;
	int yDiv = (rcDivider.top - rcDivider.bottom) -
		rcDlg.top + rcDlg.bottom - 5;
	m_staticDivider.MoveWindow(xDiv, yDiv,
		rcDivider.right - rcDivider.left,
		rcDivider.bottom - rcDivider.top, TRUE);

	// center the black frame (which the image is drawn over) at the top
	int xFrame = (rcDlg.right - pHeader->biWidth - rcDlg.left) / 2;
	m_staticFrame.MoveWindow(xFrame, 5, pHeader->biWidth,
		pHeader->biHeight, TRUE);

	// record where the image is drawn within the dialog
	RECT rcImage;
	m_staticFrame.GetClientRect(&rcImage);
	m_dwImageCX = rcImage.right;
	m_dwImageCY = rcImage.bottom;
	::ClientToScreen(m_staticFrame.m_hWnd, (POINT*)&rcImage.left);
	::ClientToScreen(m_staticFrame.m_hWnd, (POINT*)&rcImage.right);
	::ScreenToClient(m_hWnd, (POINT*)&rcImage.left);
	::ScreenToClient(m_hWnd, (POINT*)&rcImage.right);
	m_dwImageX = rcImage.left;
	m_dwImageY = rcImage.top;

	// prepare the memory DC the image is painted from
	m_hMemDC = CreateCompatibleDC(hScreen);
	m_hOldPalette = SelectPalette(m_hMemDC, (HPALETTE)m_palette.m_hObject, FALSE);
	m_hOldBitmap = SelectObject(m_hMemDC, m_hBitmap);
	::ReleaseDC(NULL, hScreen);

	return TRUE;
}

// hover.exe:0x415220
// FUNCTION: HOVER 0x415220
void CSplashDialog::OnPaint()
{
	CPaintDC dc(this);
	CPalette* pOldPal = dc.SelectPalette(&m_palette, FALSE);
	RealizePalette(dc.m_hDC);
	BitBlt(dc.m_hDC, m_dwImageX, m_dwImageY, m_dwImageCX, m_dwImageCY,
		m_hMemDC, 0, 0, SRCCOPY);
	dc.SelectPalette(pOldPal, TRUE);
}

// hover.exe:0x422FB0
// FUNCTION: HOVER 0x422fb0
void CSplashDialog::OnDestroy()
{
	CDialog::OnDestroy();

	if (m_nTimer != 0)
		::KillTimer(m_hWnd, m_nTimer);
	SelectObject(m_hMemDC, m_hOldBitmap);
	SelectPalette(m_hMemDC, m_hOldPalette, TRUE);
	DeleteObject(m_hBitmap);
	DeleteDC(m_hMemDC);

	g_ptSplashAnchor.x = -1;
	g_ptSplashAnchor.y = -1;
}

// hover.exe:0x4245D0
// FUNCTION: HOVER 0x4245d0
void CSplashDialog::OnTimer(UINT nIDEvent)
{
	DestroyWindow();
}

// hover.exe:0x42F600
// FUNCTION: HOVER 0x42f600
void CSplashDialog::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	// any key press dismisses the splash screen
	DestroyWindow();

	CDialog::OnKeyDown(nChar, nRepCnt, nFlags);
}

// hover.exe:0x4152E0
// FUNCTION: HOVER 0x4152e0
void CSplashDialog::OnPaletteChanged(CWnd* pFocusWnd)
{
	RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_ERASE | RDW_UPDATENOW);
}

// hover.exe:0x4245E0
// FUNCTION: HOVER 0x4245e0
void CSplashDialog::PostNcDestroy()
{
	CDialog::PostNcDestroy();

	// show the Quick Help dialog unless the user asked to skip it
	CQuickHelpDialog dlg(NULL);
	if (dlg.ReadQuickHelpFlag() == 0)
		dlg.DoModal();

	delete this;
}

/////////////////////////////////////////////////////////////////////////////
// CQuickHelpDialog dialog

// hover.exe:0x424EC0
// FUNCTION: HOVER 0x424ec0
CQuickHelpDialog::CQuickHelpDialog(CWnd* pParent /*=NULL*/)
	: CDialog(IDD_QUICKHELP, pParent)
{
	//{{AFX_DATA_INIT(CQuickHelpDialog)
	//}}AFX_DATA_INIT
}

BEGIN_MESSAGE_MAP(CQuickHelpDialog, CDialog)
	//{{AFX_MSG_MAP(CQuickHelpDialog)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CQuickHelpDialog message handlers

// hover.exe:0x4342C0
// FUNCTION: HOVER 0x4342c0
void CQuickHelpDialog::DoDataExchange(CDataExchange* pDX)
{
	// this override is intentionally empty in the original
	//{{AFX_DATA_MAP(CQuickHelpDialog)
	//}}AFX_DATA_MAP
}

// hover.exe:0x424F30
// FUNCTION: HOVER 0x424f30
int CQuickHelpDialog::ReadQuickHelpFlag()
{
	// Read the QuickHelpFlag value from the registry. Returns 0 when the
	// value cannot be read, in which case the dialog is shown.
	HKEY hKey;
	// Registry value buffer bigger than the value (rule 137): only
	// abFlag[0] is ever used, but the original's frame reserves eight
	// bytes here, not a single BYTE.
	BYTE abFlag[8];
	if (RegOpenKeyExA(HKEY_CURRENT_USER, "Software\\Microsoft\\Hover!",
			0, KEY_ALL_ACCESS, &hKey) == ERROR_SUCCESS)
	{
		DWORD dwType = REG_BINARY;
		// Original bug: the size passed is 0x3C, not sizeof(abFlag), so
		// a QuickHelpFlag value of 9-60 bytes would overrun the frame.
		DWORD cbData = 0x3C;
		if (RegQueryValueExA(hKey, "QuickHelpFlag", NULL, &dwType,
				abFlag, &cbData) != ERROR_SUCCESS)
		{
			abFlag[0] = 0;
		}
		RegCloseKey(hKey);
	}
	else
	{
		abFlag[0] = 0;
	}
	return (int)(char)abFlag[0];
}

// hover.exe:0x434360
// FUNCTION: HOVER 0x434360
BOOL CQuickHelpDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

	// check the "Skip Quick Help at Startup" box from the registry
	HKEY hKey;
	// Registry value buffer bigger than the value (rule 137): only
	// abSkip[0] is ever used, but the original's frame reserves eight
	// bytes here, not a single BYTE.
	BYTE abSkip[8];
	if (RegOpenKeyExA(HKEY_CURRENT_USER, "Software\\Microsoft\\Hover!",
			0, KEY_ALL_ACCESS, &hKey) == ERROR_SUCCESS)
	{
		DWORD dwType = REG_BINARY;
		// Original bug: the size passed is 0x3C, not sizeof(abSkip), so
		// a QuickHelpFlag value of 9-60 bytes would overrun the frame.
		DWORD cbData = 0x3C;
		if (RegQueryValueExA(hKey, "QuickHelpFlag", NULL, &dwType,
				abSkip, &cbData) != ERROR_SUCCESS)
		{
			abSkip[0] = 0;
		}
		RegCloseKey(hKey);
	}
	else
	{
		abSkip[0] = 0;
	}
	::CheckDlgButton(m_hWnd, IDC_QUICKHELP_SKIP, (char)abSkip[0]);

	return TRUE;
}

// hover.exe:0x4342E0
// FUNCTION: HOVER 0x4342e0
void CQuickHelpDialog::OnOK()
{
	// save the "Skip Quick Help at Startup" checkbox to the registry
	BOOL bChecked = (::IsDlgButtonChecked(m_hWnd, IDC_QUICKHELP_SKIP) != 0);
	BYTE bSkip;
	// Frame placeholder, not a variable of the original (rule 80): four
	// bytes our expressions do not generate.
	volatile DWORD dwPad;

	HKEY hKey;
	DWORD dwDisposition;

	dwPad = 0;
	if (RegCreateKeyExA(HKEY_CURRENT_USER, "Software\\Microsoft\\Hover!",
			0, NULL, 0, KEY_ALL_ACCESS, NULL, &hKey,
			&dwDisposition) == ERROR_SUCCESS)
	{
		bSkip = (BYTE)bChecked;
		RegSetValueExA(hKey, "QuickHelpFlag", 0, REG_BINARY, &bSkip, 1);
		RegCloseKey(hKey);
	}

	CDialog::OnOK();
}
