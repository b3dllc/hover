// bumpview.cpp : implementation of the CBumperView class
//
// Reconstructed from hover.exe; see bumpview.h for the RTTI/vtable data.

#include "hover.h"
#include <math.h>				// sqrt()
#include <string.h>				// strlen()
#include <stdlib.h>				// _fcvt(), _itoa(), _ltoa()
#include "bumpview.h"
#include "mazeobj.h"
#include "texload.h"
#include "texture.h"
#include "texheap.h"			// renderer globals + CSub402E00/CSub403980
#include "texmisc.h"			// Hover_sub_4262B0 (exception reporter)
#include "mainfrm.h"			// CMainFrame::Sub_408A70 (OnSize status bar)

// MSVC 2.2 does not supply a placement-new declaration in <new.h>, so
// provide the standard placement-new operator locally.
inline void* operator new(size_t, void* p)
{
	return p;
}

IMPLEMENT_DYNCREATE(CBumperView, CView)

// The original's message map: AFX_MSGMAP at hover.exe:0x4BB0C0 with its
// AFX_MSGMAP_ENTRY table at 0x4BB0C8 (base class CView). The entries are
// listed here in exactly the original table order.
BEGIN_MESSAGE_MAP(CBumperView, CView)
	//{{AFX_MSG_MAP(CBumperView)
	ON_WM_DESTROY()
	ON_WM_SIZE()
	ON_UPDATE_COMMAND_UI(ID_DBG_WALL, OnUpdateDbgFlag)
	ON_COMMAND(ID_CMD_34000, OnCmd34000)
	ON_UPDATE_COMMAND_UI(ID_CMD_34000, OnUpdateCmd34000)
	ON_COMMAND(ID_CMD_34011, OnCmd34011)
	ON_UPDATE_COMMAND_UI(ID_CMD_34011, OnUpdateCmd34011)
	ON_WM_ERASEBKGND()
	ON_COMMAND(ID_CMD_34014, OnCmd34014)
	ON_COMMAND(ID_CMD_34015, OnCmd34015)
	ON_COMMAND(ID_CMD_34013, OnCmd34013)
	ON_COMMAND(ID_CMD_34018, OnCmd34018)
	ON_COMMAND(ID_CMD_34019, OnCmd34019)
	ON_COMMAND(ID_CMD_34017, OnCmd34017)
	ON_COMMAND(ID_CMD_34016, OnCmd34016)
	ON_COMMAND(ID_GAME_PAUSE, OnGamePause)
	ON_UPDATE_COMMAND_UI(ID_GAME_PAUSE, OnUpdateGamePause)
	ON_COMMAND(ID_CMD_34005, OnCmd34005)
	ON_UPDATE_COMMAND_UI(ID_CMD_34005, OnUpdateCmd34005)
	ON_WM_KEYDOWN()
	ON_COMMAND(ID_CMD_34029, OnCmd34029)
	ON_UPDATE_COMMAND_UI(ID_CMD_34029, OnUpdateCmd34029)
	ON_COMMAND(ID_CMD_34030, OnCmd34030)
	ON_UPDATE_COMMAND_UI(ID_CMD_34030, OnUpdateCmd34030)
	ON_UPDATE_COMMAND_UI(ID_CMD_34014, OnUpdateCmd34014)
	ON_UPDATE_COMMAND_UI(ID_CMD_34015, OnUpdateCmd34015)
	ON_UPDATE_COMMAND_UI(ID_CMD_34013, OnUpdateCmd34013)
	ON_UPDATE_COMMAND_UI(ID_CMD_34018, OnUpdateCmd34018)
	ON_UPDATE_COMMAND_UI(ID_CMD_34019, OnUpdateCmd34019)
	ON_UPDATE_COMMAND_UI(ID_CMD_34017, OnUpdateCmd34017)
	ON_UPDATE_COMMAND_UI(ID_CMD_34016, OnUpdateCmd34016)
	ON_COMMAND(ID_CMD_34032, OnCmd34032)
	ON_UPDATE_COMMAND_UI(ID_CMD_34032, OnUpdateCmd34032)
	ON_WM_CREATE()
	ON_COMMAND(ID_CMD_34033, OnCmd34033)
	ON_UPDATE_COMMAND_UI(ID_CMD_34033, OnUpdateCmd34033)
	ON_UPDATE_COMMAND_UI(ID_DBG_POD, OnUpdateDbgFlag)
	ON_UPDATE_COMMAND_UI(ID_DBG_FLOOR, OnUpdateDbgFlag)
	ON_UPDATE_COMMAND_UI(ID_DBG_CAR, OnUpdateDbgFlag)
	ON_WM_CHAR()
	ON_MESSAGE(WM_USER + 0, OnUserInput)
	ON_MESSAGE(WM_USER + 1, OnUserGameOver)
	ON_MESSAGE(WM_USER + 2, OnUserNextLevel)
	ON_COMMAND(ID_ESCAPE, OnEscape)
	ON_MESSAGE(MM_MCINOTIFY, OnMciNotify)
	ON_MESSAGE(WM_HELP, OnHelpMsg)
	ON_MESSAGE(WM_CONTEXTMENU, OnContextMenuMsg)
	ON_COMMAND(ID_CMD_34034, OnCmd34034)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Globals

// hover.exe:0x4C4CC8
// GLOBAL: HOVER 0x4c4cc8
DWORD g_adwUnk_4C4CC8[6] = { 1, 2, 4, 3, 5, 6 };

// GLOBAL: HOVER 0x460998
DWORD g_dwUnk_460998;		// hover.exe:0x460998 zone descriptor index
// GLOBAL: HOVER 0x4c4ca8
DWORD g_dwUnk_4C4CA8;		// hover.exe:0x4C4CA8

// hover.exe:0x4C4CEC
// GLOBAL: HOVER 0x4c4cec
int g_nUnk_4C4CEC = 0x39;	// down-counter that paces Sub_408DF0
// GLOBAL: HOVER 0x4c4cf0
DWORD g_dwUnk_4C4CF0;		// hover.exe:0x4C4CF0 previous joystick button mask

// GLOBAL: HOVER 0x4a4790
DWORD g_dwUnk_4A4790;		// hover.exe:0x4A4790
// GLOBAL: HOVER 0x4a4794
DWORD g_dwUnk_4A4794;		// hover.exe:0x4A4794
// GLOBAL: HOVER 0x4a4798
DWORD g_dwUnk_4A4798;		// hover.exe:0x4A4798
// GLOBAL: HOVER 0x4a479c
DWORD g_dwUnk_4A479C;		// hover.exe:0x4A479C
// GLOBAL: HOVER 0x4a47b0
DWORD g_dwUnk_4A47B0;		// hover.exe:0x4A47B0
// GLOBAL: HOVER 0x4a47b8
DWORD g_dwUnk_4A47B8;		// hover.exe:0x4A47B8
// GLOBAL: HOVER 0x4a47bc
DWORD g_dwUnk_4A47BC;		// hover.exe:0x4A47BC
// GLOBAL: HOVER 0x4a47d8
DWORD g_dwUnk_4A47D8;		// hover.exe:0x4A47D8
// GLOBAL: HOVER 0x4a47e0
DWORD g_dwUnk_4A47E0;		// hover.exe:0x4A47E0
// GLOBAL: HOVER 0x4a47e4
DWORD g_dwUnk_4A47E4;		// hover.exe:0x4A47E4
// 0x4A4844/0x4A4848 are the m_pData/m_nSize members of the beacon CObArray
// at 0x4A4840 (g_obArrUnk_4A4840 in mazeobj.cpp), not globals of their own.

// hover.exe:0x4A1D50
// GLOBAL: HOVER 0x4a1d50
CPalette g_paletteUnk_4A1D50;	// palette selected into the paint DC

/////////////////////////////////////////////////////////////////////////////
// Free helper functions

// Looks up the texture named pszName in the global name -> texture map and
// returns it, or NULL when there is no such texture.
// hover.exe:0x418A30
// FUNCTION: HOVER 0x418a30
CObject* Hover_sub_418A30(const char* pszName)
{
	CObject* pObj;

	return g_mapUnk_4A1D30.Lookup(pszName, pObj) ? pObj : NULL;
}

/////////////////////////////////////////////////////////////////////////////
// CSub423030

// hover.exe:0x423030
// FUNCTION: HOVER 0x423030
CSub423030::CSub423030()
{
	m_pUnk28 = NULL;
	m_wUnk44 = 0;
	m_wUnk46 = 0;
	m_dwUnk58 = 0;
	m_wUnk48 = 0;
	m_wUnk4A = 0;
	m_pUnk4C = NULL;
	m_dwUnk50 = 0;
	m_dwUnk54 = 0;
}

// hover.exe:0x420E60
// FUNCTION: HOVER 0x420e60
CSub423030::~CSub423030()
{
	// trivial destructor in the original
}

// Recomputes the camera basis offsets from the view yaw: with
// a = -yaw (quarter-circle = 0x80 table units) the offsets are
//   m_dwUnk5C = (sin(a)*m_dwUnk8338 - sin(a-0x80)*m_dwUnk833C) * factor44
//   m_dwUnk60 = (sin(a)*m_dwUnk833C + sin(a-0x80)*m_dwUnk8338) * factor48
// all 16.16 fixed-point, keeping the integer parts.

// hover.exe:0x405A10
// FUNCTION: HOVER 0x405a10
void CSub423030::Sub_405A10(CBumperView* pView, CBumperDoc* pDoc)
{
	DWORD dwA1;
	DWORD dwT1;
	DWORD dwCos;
	DWORD dwSin;
	DWORD dwA2;
	DWORD dwT2;
	DWORD dwB1;
	DWORD dwB2;
	long lAngle;
	const DWORD* pSinA;
	const DWORD* pSinB;

	// The two sin-table entries are kept as pointers (the original derefs
	// ebx/edi twice each) and every product is a HOVER_FIXMULP expansion.
	lAngle = (long)(0xFFFFFF80 - pView->m_dwUnk00DC);
	dwCos = *(DWORD*)&pDoc->m_humanPlayer.m_awUnk01AC[0];
	dwSin = *(DWORD*)&pDoc->m_humanPlayer.m_awUnk01AC[2];
	pSinA = &g_adwSinTable[(((lAngle + 0x80) & 0x1FF) << 4) + 1];
	pSinB = &g_adwSinTable[((lAngle & 0x1FF) << 4) + 1];

	dwA1 = *pSinB;
	HOVER_FIXMULP(dwA1, dwSin);
	dwB1 = *pSinA;
	HOVER_FIXMULP(dwB1, dwCos);
	dwT1 = dwB1 - dwA1;
	HOVER_FIXMULP(dwT1, *(DWORD*)&m_wUnk44);
	m_dwUnk5C = (long)((short*)&dwT1)[1];

	dwA2 = *pSinB;
	HOVER_FIXMULP(dwA2, dwCos);
	dwB2 = *pSinA;
	HOVER_FIXMULP(dwB2, dwSin);
	dwT2 = dwB2 + dwA2;
	HOVER_FIXMULP(dwT2, *(DWORD*)&m_wUnk48);
	m_dwUnk60 = (long)((short*)&dwT2)[1];
}


// hover.exe:0x411D10
// FUNCTION: HOVER 0x411d10
int CSub423030::Sub_411D10(const RECT* pRect)
{
	int nHeight;
	int nWidth;
	int nResult;

	m_rcSource = *pRect;

	// The extents are re-read out of the stored rectangle, not out of the
	// argument.
	nHeight = m_rcSource.bottom - m_rcSource.top;
	nWidth = m_rcSource.right - m_rcSource.left;

	// +0x20 orientation flag: 1 if width is the limiting dimension, 0 if
	// height is.
	m_dwUnk20 = (nHeight <= nWidth) ? 1 : 0;

	if (m_dwUnk20 == 0)
		nResult = nHeight / (int)m_rcDirty.bottom;
	else
		nResult = nWidth / (int)m_rcDirty.bottom;

	ScratchQuotient() = nResult;
	return nResult;
}

// Computes the fixed-point x/y scaling factors stored as the dword
// pair at +0x44 (m_wUnk44/m_wUnk46) and +0x48 (m_wUnk48/m_wUnk4A).
// The factors are ((0xFAE1 * half_extent) << 16) / ((doc+0x8364) *
// (global_object+0x20) >> 16), computed independently for width and height.

// hover.exe:0x4140E0
// HOVER_FIXDIVPL(dwA, dwB): dwA = (dwA << 16) / dwB reached through a
// pointer temporary, the divide counterpart of HOVER_FIXMULP (hover.h's
// HOVER_FIXDIVPP takes the address, HOVER_FIXDIVP names the destination).
#define HOVER_FIXDIVPL(dwA, dwB) \
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

// HOVER_FIXMULWPR(dwR, dwA, wB): dwR = dwA * (short)wB with the source
// value, the destination pointer and the 16-bit factor each homed in a
// temporary of their own - the shape CSub423030::Sub_4140E0's original has
// (hover.exe:0x414122: the 0xFAE1 store is hoisted into the entry block,
// the destination is written from that temp and only then addressed).
#define HOVER_FIXMULWPR(dwR, dwA, wB) \
	{ \
		volatile DWORD _dwA; \
		DWORD* _pA; \
		WORD _wB; \
		_dwA = (dwA); \
		_pA = &(dwR); \
		*_pA = _dwA; \
		_wB = (WORD)(wB); \
		_asm mov ecx, _pA \
		_asm movsx eax, _wB \
		_asm imul dword ptr [ecx] \
		_asm mov [ecx], eax \
	}

// FUNCTION: HOVER 0x4140e0
void CSub423030::Sub_4140E0()
{
	DWORD dwNum1;
	DWORD dwScaled1;
	DWORD dwQ1;
	DWORD dwNum2;
	DWORD dwScaled2;
	DWORD dwQ2;
	DWORD dwGlobalFactor;
	DWORD dwPlayerFactor;

	dwGlobalFactor = *(DWORD*)((BYTE*)theApp.m_pUnk014C + 0x20);
	dwPlayerFactor = *(DWORD*)((BYTE*)theApp.m_pBumperDoc + 0x8364);
	dwScaled1 = dwPlayerFactor;
	HOVER_FIXMULP(dwScaled1, dwGlobalFactor);
	HOVER_FIXMULWPR(dwNum1, 0xFAE1, (short)((long)m_dwUnk20 / 2));
	// Volatile read: HOVER_FIXDIVPL writes its destination only through the
	// pointer temporary, which VC 2.1 does not model, so a plain
	// `dwQ1 = dwNum1` has its store deleted and +0x44/+0x48 receive the
	// undivided numerator (rule 95).
	dwQ1 = *(volatile DWORD*)&dwNum1;
	HOVER_FIXDIVPL(dwQ1, dwScaled1);
	*(DWORD*)&m_wUnk44 = dwQ1;

	dwGlobalFactor = *(DWORD*)((BYTE*)theApp.m_pUnk014C + 0x20);
	dwPlayerFactor = *(DWORD*)((BYTE*)theApp.m_pBumperDoc + 0x8364);
	dwScaled2 = dwPlayerFactor;
	HOVER_FIXMULP(dwScaled2, dwGlobalFactor);
	HOVER_FIXMULWPR(dwNum2, 0xFAE1, (short)((long)m_dwUnk24 / 2));
	dwQ2 = *(volatile DWORD*)&dwNum2;
	HOVER_FIXDIVPL(dwQ2, dwScaled2);
	*(DWORD*)&m_wUnk48 = dwQ2;
}


// Creates an 8-bpp DIB section sized from the source rectangle, attaches it
// to m_bitmap, selects it into m_dc and copies the zone's dashboard tile
// into the DIB bits through Hover_sub_40E730.
//
// The source rectangle is the RECT the caller passes by value in the last
// four arguments (the original reads the whole 16-byte block through one
// pointer, `lea ecx,[esp+0x430]` at hover.exe:0x411D8B).  pDib is the
// dashboard bitmap resource with its BITMAPINFOHEADER still attached: the
// original reaches the pixels with `lea ebx,[eax+ecx+0x400]` (eax = biSize)
// at hover.exe:0x411F32 and takes the source pitch and height from biWidth
// and biHeight at 0x411F40/0x411F43.

// hover.exe:0x411D80
// FUNCTION: HOVER 0x411d80
void CSub423030::Sub_411D80(CDC* pInternalDC, BYTE* pDib, int nLeft,
	int nTop, int nRight, DWORD nBottom)
{
	RECT* pSrc;
	BITMAPINFO* pBmi;
	PALETTEENTRY apePalette[256];
	PALETTEENTRY* pPe;
	BYTE* pColorRed;
	HDC hScreen;
	HBITMAP hBitmap;
	DWORD nZone;
	BYTE* pSrcBits;
	CBumperView* pView;
	long lSrcWidth;
	long lSrcHeight;
	RECT rcDest;

	pSrc = &m_rcSource;
	*pSrc = *(RECT*)&nLeft;

	m_dwUnk20 = pSrc->right - pSrc->left;
	m_dwUnk24 = pSrc->bottom - pSrc->top;
	m_dwUnk50 = (long)m_dwUnk20 / 2 + pSrc->left;
	m_dwUnk54 = (long)m_dwUnk24 / 2 + pSrc->top;
	m_dwUnk5C = 0;
	m_dwUnk60 = 0;
	m_pUnk4C = pInternalDC;

	Sub_4140E0();

	hScreen = ::GetDC(NULL);

	pBmi = (BITMAPINFO*)::operator new(0x428);
	pBmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	pBmi->bmiHeader.biWidth = m_dwUnk20;
	pBmi->bmiHeader.biHeight = -(long)m_dwUnk24;
	pBmi->bmiHeader.biPlanes = 1;
	pBmi->bmiHeader.biBitCount = 8;
	pBmi->bmiHeader.biCompression = BI_RGB;
	pBmi->bmiHeader.biSizeImage = 0;
	pBmi->bmiHeader.biXPelsPerMeter = 0;
	pBmi->bmiHeader.biYPelsPerMeter = 0;
	pBmi->bmiHeader.biClrUsed = 0;
	pBmi->bmiHeader.biClrImportant = 0;

	::GetPaletteEntries(
		(HPALETTE)g_paletteUnk_4A1D50.GetSafeHandle(),
		0,
		0x100,
		apePalette);

	// Copy the PALETTEENTRY array into the BITMAPINFO color table in the
	// shifted order used by the original (writing the current entry's red,
	// then the previous entry's blue/green/reserved at offsets -2/-1/+1
	// relative to the current red pointer).
	pPe = apePalette;
	pColorRed = (BYTE*)pBmi + 0x2A;
	while (pPe < apePalette + 0x100)
	{
		pColorRed[0] = pPe->peRed;
		pColorRed[-2] = pPe->peBlue;
		pColorRed[-1] = pPe->peGreen;
		pColorRed[1] = pPe->peFlags;
		pPe++;
		pColorRed += 4;
	}

	if (m_bitmap.m_hObject != NULL)
	{
		m_dc.SelectObject(m_pUnk28);
		m_bitmap.DeleteObject();
		m_bitmap.Detach();
	}

	hBitmap = (HBITMAP)::CreateDIBSection(
		hScreen,
		pBmi,
		DIB_RGB_COLORS,
		(void**)&m_dwUnk58,
		NULL,
		0);
	m_bitmap.Attach(hBitmap);

	::ReleaseDC(NULL, hScreen);
	::operator delete(pBmi);

	if (m_dc.m_hDC == NULL)
	{
		HDC hMem = ::CreateCompatibleDC(NULL);
		m_dc.Attach(hMem);
	}

	m_pUnk28 = m_dc.SelectObject(&m_bitmap);

	// The blit's source size and destination rectangle are built in six
	// stack slots before the view lookup (hover.exe:0x411F24..0x411F4A,
	// which is why the frame is 0x418 and not 0x400 bytes).
	rcDest.left = 0;
	rcDest.top = 0;
	pSrcBits = pDib + ((BITMAPINFOHEADER*)pDib)->biSize + 0x400;
	rcDest.bottom = m_dwUnk24;
	rcDest.right = m_dwUnk20;
	lSrcWidth = ((BITMAPINFOHEADER*)pDib)->biWidth;
	lSrcHeight = ((BITMAPINFOHEADER*)pDib)->biHeight;

	pView = Hover_sub_401140();
	if (pView->m_dwUnk0544 != 0)
		nZone = 0;
	else
		nZone = g_dwUnk_460998;

	Hover_sub_40E730(
		(long)g_adwUnk_4609A0[nZone * (0x338 / 4) + 0x268 / 4 + 0],
		(long)g_adwUnk_4609A0[nZone * (0x338 / 4) + 0x268 / 4 + 1],
		(long)g_adwUnk_4609A0[nZone * (0x338 / 4) + 0x268 / 4 + 2],
		(long)g_adwUnk_4609A0[nZone * (0x338 / 4) + 0x268 / 4 + 3],
		lSrcWidth,
		lSrcHeight,
		(long)rcDest.left,
		(long)rcDest.top,
		(long)rcDest.right,
		(long)rcDest.bottom,
		(long)m_dwUnk20,
		(long)m_dwUnk24,
		pSrcBits,
		(BYTE*)m_dwUnk58,
		0);
}

// hover.exe:0x401E80
// FUNCTION: HOVER 0x401e80
void CSub423030::Sub_401E80(CDC* pDestDC, int bFullUpdate)
{
	CBumperView* pView;
	CRect rcRaw;
	CRect rcUpdate;
	CRect rcDirty;
	CPen* pOldPen;
	RECT* prcSrc;
	int xSrc;
	int ySrc;

	pView = Hover_sub_401140();
	if (pView == NULL)
		return;

	prcSrc = &m_rcSource;

	// Build the raw update rectangle from the base point and the stored
	// extents; make a normalized/inflated copy and union it with the dirty
	// rectangle kept at +0x10.  The original re-reads left/top out of the
	// rectangle for the right/bottom sums instead of reusing the values it
	// just stored, so those two reads go through a volatile lvalue.
	rcRaw.left = (long)m_dwUnk50;
	rcRaw.top = (long)m_dwUnk54;
	rcRaw.right = (long)m_dwUnk5C + *(volatile long*)&rcRaw.left;
	rcRaw.bottom = (long)m_dwUnk60 + *(volatile long*)&rcRaw.top;
	rcUpdate = rcRaw;
	rcUpdate.NormalizeRect();
	rcUpdate.InflateRect(2, 2);
	rcDirty.UnionRect(&rcUpdate, &m_rcDirty);

	// The source DC is reached through GetSafeHdc (the original tests
	// &m_dc for NULL, i.e. `cmp esi, -0x34`) at every use, the internal
	// one straight through m_hDC; neither is cached in a local.
	xSrc = rcDirty.left - prcSrc->left;
	ySrc = rcDirty.top - prcSrc->top;

	if (bFullUpdate != 0)
	{
		// Copy the whole source bitmap from its origin to the source
		// rectangle's position in the internal DC (the original blits
		// from (0,0) to (left,top), not the other way round).
		::BitBlt(m_pUnk4C->m_hDC, prcSrc->left, prcSrc->top,
			prcSrc->right - prcSrc->left, prcSrc->bottom - prcSrc->top,
			m_dc.GetSafeHdc(), 0, 0, SRCCOPY);
	}
	else
	{
		// Copy only the dirty sub-rectangle to its corresponding position.
		::BitBlt(m_pUnk4C->m_hDC, rcDirty.left, rcDirty.top,
			rcDirty.Width(), rcDirty.Height(),
			m_dc.GetSafeHdc(), xSrc, ySrc, SRCCOPY);
	}

	// Draw the update-area indicator using the stored pen (the original
	// does not test m_pUnk64 for NULL).  Every coordinate is re-read out
	// of rcRaw's stack slots.
	pOldPen = m_pUnk4C->SelectObject(m_pUnk64);
	m_pUnk4C->MoveTo(rcRaw.left, rcRaw.top);
	m_pUnk4C->LineTo(rcRaw.right, rcRaw.bottom);
	m_pUnk4C->MoveTo(rcRaw.left + 1, rcRaw.top);
	m_pUnk4C->LineTo(rcRaw.right + 1, rcRaw.bottom);
	m_pUnk4C->SelectObject(pOldPen);

	// Present the internal bitmap to the destination DC.  pView is only
	// tested here, and the original keeps it in its stack slot across the
	// blit and the four GDI calls above rather than in a register.
	if ((*(CBumperView* volatile*)&pView)->m_dwUnk0544 != 0)
	{
		// The 2x present doubles the four rectangle values into locals of
		// their own before deriving the source DC (cf. Sub_42D3E0), which
		// is what puts the doubled values in stack slots in the original.
		if (bFullUpdate != 0)
		{
			long lLeft, lTop, lWidth, lHeight;
			long l2Left, l2Top, l2Width, l2Height;

			lTop = prcSrc->top;
			lHeight = prcSrc->bottom - lTop;
			lLeft = prcSrc->left;
			lWidth = prcSrc->right - lLeft;
			l2Height = lHeight * 2;
			l2Width = lWidth * 2;
			l2Top = lTop * 2;
			l2Left = lLeft * 2;
			::StretchBlt(pDestDC->m_hDC, l2Left, l2Top, l2Width, l2Height,
				m_pUnk4C->GetSafeHdc(), lLeft, lTop, lWidth, lHeight,
				SRCCOPY);
		}
		else
		{
			long lLeft, lTop, lWidth, lHeight;
			long l2Left, l2Top, l2Width, l2Height;

			lHeight = rcDirty.Height();
			lWidth = rcDirty.Width();
			lTop = rcDirty.top;
			lLeft = rcDirty.left;
			l2Width = lWidth * 2;
			l2Height = lHeight * 2;
			l2Top = lTop * 2;
			l2Left = lLeft * 2;
			::StretchBlt(pDestDC->m_hDC, l2Left, l2Top, l2Width, l2Height,
				m_pUnk4C->GetSafeHdc(), lLeft, lTop, lWidth, lHeight,
				SRCCOPY);
		}
	}
	else
	{
		if (bFullUpdate != 0)
		{
			long lLeft, lTop, lWidth, lHeight;

			lTop = prcSrc->top;
			lLeft = prcSrc->left;
			lHeight = prcSrc->bottom - lTop;
			lWidth = prcSrc->right - lLeft;
			::BitBlt(pDestDC->m_hDC, lLeft, lTop, lWidth, lHeight,
				m_pUnk4C->GetSafeHdc(), lLeft, lTop, SRCCOPY);
		}
		else
		{
			long lLeft, lTop, lWidth, lHeight;

			lTop = rcDirty.top;
			lLeft = rcDirty.left;
			lHeight = rcDirty.bottom - lTop;
			lWidth = rcDirty.right - lLeft;
			::BitBlt(pDestDC->m_hDC, lLeft, lTop, lWidth, lHeight,
				m_pUnk4C->GetSafeHdc(), lLeft, lTop, SRCCOPY);
		}
	}

	// Store the inflated update rectangle back as the new dirty region.
	m_rcDirty = rcUpdate;
}

/////////////////////////////////////////////////////////////////////////////
// CSub422F50

// Fixed-point multiply helper used by Sub_4055B0 (CFixed is declared in
// hover.h): computes m_dwValue = (m_dwValue * dwB) >> 16.

// hover.exe:0x4038E0
// FUNCTION: HOVER 0x4038e0
void CFixed::Sub_4038E0(DWORD dwB)
{
	DWORD dwA = m_dwValue;
	HOVER_FIXMUL(dwA, dwB);
	m_dwValue = dwA;
}

// The 16.16 fixed-point steps of the unoptimized projection helper
// CSub422F50::Sub_4055B0 (see the comment there). Each of these is expanded
// inline even inside the `#pragma optimize("g", off)` region, which is what
// gives that function its repeated three-slot temporary blocks: a reference
// or scalar argument folds into the expansion with no slot of its own, while
// a CFixed passed by value is copied into a slot (the last parameter
// highest, stored right to left) - see rule 117 in notes/perfect-match.md.

// Hover_PtOf: the renderer reads the projected/world point pairs of an
// object as an 8-byte value (`lea eax,[esi+4] / mov ecx,[eax] /
// mov edx,[eax+4]` into a fresh stack temporary at every use, with the
// unused half dead-stored away). An inline function returning POINT by
// value is the only spelling that reproduces that: VC 2.1 gives every
// expansion its own return temporary and compares straight out of it.
static inline POINT Hover_PtOf(const DWORD* pdw)
{
	return *(const POINT*)pdw;
}

// Hover_Project: the object pass hands Sub_4055B0 a whole point, not two
// coordinates - the original copies both halves into a fresh adjacent
// 8-byte slot at every call site and pushes out of it, which is what an
// inline forwarder with a by-value POINT parameter produces (a POINT local
// or a plain pair of arguments does not; see rule 139).
static inline void Hover_Project(CSub422F50* pThis, DWORD* pdwOut, POINT pt)
{
	pThis->Sub_4055B0(pdwOut, pt.x, pt.y, g_dwUnk_4A4790, g_dwUnk_4A4794,
		g_dwUnk_4A47E0, g_dwUnk_4A47E4, g_dwUnk_4A47B8);
}

static inline void Hover_FixSetInt(DWORD& dwFix, long lValue)
{
	((WORD*)&dwFix)[0] = 0;
	((WORD*)&dwFix)[1] = (WORD)lValue;
}

static inline void Hover_FixSubInt(DWORD& dwFix, long lValue)
{
	((short*)&dwFix)[1] -= (short)lValue;
}

static inline void Hover_FixSinTo(DWORD& dwOut, long lAngle)
{
	dwOut = g_adwSinTable[((lAngle << 4) & 0x1FFF) + 1];
}

static inline void Hover_FixCosTo(DWORD& dwOut, long lAngle)
{
	dwOut = g_adwSinTable[(((lAngle << 4) + 0x800) & 0x1FFF) + 1];
}

static inline void Hover_FixMulTo(DWORD& dwOut, CFixed fmA, CFixed fmB)
{
	fmA.Sub_4038E0(fmB.m_dwValue);
	dwOut = fmA.m_dwValue;
}

static inline void Hover_FixAddTo(DWORD& dwOut, CFixed fmA, CFixed fmB)
{
	fmA.AddEq(fmB.m_dwValue);
	dwOut = fmA.m_dwValue;
}

static inline CFixed Hover_FixSub(CFixed fmA, CFixed fmB)
{
	fmA.SubEq(fmB.m_dwValue);
	return fmA;
}

static inline void Hover_FixSubTo(DWORD& dwOut, CFixed fmA, CFixed fmB)
{
	fmA.SubEq(fmB.m_dwValue);
	dwOut = fmA.m_dwValue;
}

static inline DWORD Hover_FixValue(CFixed fm)
{
	return fm.m_dwValue;
}

// The 48.16 halves of the projection: the shift and the divide are done on
// an __int64 and the quotient is repacked into a 16.16 dword.
static inline void Hover_Shl32(__int64& llValue)
{
	llValue = llValue << 32;
}

static inline void Hover_FixPack(long& lOut, const __int64& llValue)
{
	((WORD*)&lOut)[0] = (WORD)llValue;
	((WORD*)&lOut)[1] = (WORD)(llValue / 0x10000);
}

// hover.exe:0x422F50
// FUNCTION: HOVER 0x422f50
CSub422F50::CSub422F50()
{
	m_awUnk006C[0] = 0;
	m_awUnk006C[1] = 0;
	m_awUnk006C[2] = 0;
	m_awUnk006C[3] = 0;
	m_awUnk006C[4] = 0;
	m_awUnk006C[5] = 0;
	m_awUnk006C[6] = 0;
	m_awUnk006C[7] = 0;
	m_awUnk006C[8] = 0;
	m_awUnk006C[9] = 0;
	m_dwUnk00AC = 0;
	m_wUnk0080 = 0;
	m_wUnk0082 = 0;
	m_dwUnk00C8 = 0x40000;
}

// Sets the viewport/projection globals from the human player's camera and
// projects every object in the doc's two object lists; when
// theApp.m_dwUnk01E0 is set, also visibility-marks and projects the tracked
// object array (g_obArrUnk_4A4840).

// hover.exe:0x407160
// FUNCTION: HOVER 0x407160
void CSub422F50::Sub_407160(CBumperDoc* pDoc)
{
	CBumperView* pView;
	DWORD dwUnk540;
	DWORD adwPos[2];
	DWORD adwPt[2];
	DWORD adwPt2[2];
	DWORD adwA[2];
	DWORD adwB[2];
	DWORD adwC[2];
	DWORD adwD[2];
	DWORD adwE[2];
	DWORD dwHalf;
	DWORD dwScale;
	DWORD dwProj;
	DWORD dwBase;
	POSITION pos;
	DWORD* pdwPos;
	CPodObj* pPod;
	CPlayer* pPlayer;
	int i;

	m_dwUnk00AC++;
	if ((long)m_dwUnk00AC >= 8)
		m_dwUnk00AC = 0;
	dwUnk540 = 0;
	pView = Hover_sub_401140();
	if (pView != NULL)
		dwUnk540 = pView->m_dwUnk0540;
	// The camera position is read back through Sub_403880's return value
	// (rule 39).
	pPlayer = &pDoc->m_humanPlayer;
	pdwPos = pPlayer->Sub_403880(adwPos);
	g_dwUnk_4A4790 = pdwPos[0];
	g_dwUnk_4A4794 = pdwPos[1];
	g_dwUnk_4A47B8 = (DWORD)((long)(short)pDoc->m_humanPlayer.m_awUnk0194[5]
		- 0x80);
	g_dwUnk_4A47B0 = *(DWORD*)(pDoc->m_dwUnk0188 + 0x1C);
	dwHalf = (DWORD)(long)(short)m_wUnk0088;
	g_dwUnk_4A47BC = g_dwUnk_4A4790 - dwHalf;
	g_dwUnk_4A47D8 = g_dwUnk_4A4794 - dwHalf;
	g_dwUnk_4A4798 = g_dwUnk_4A4790 + dwHalf;
	g_dwUnk_4A479C = g_dwUnk_4A4794 + dwHalf;
	dwScale = *(DWORD*)&m_wUnk0080;
	pdwPos = pPlayer->Sub_4038A0(adwPt);
	dwProj = pdwPos[0];
	HOVER_FIXDIVPL(dwProj, dwScale);
	dwBase = *(DWORD*)&m_awUnk006C[8];
	dwBase -= dwProj;
	g_dwUnk_4A47E0 = dwBase - *(DWORD*)&m_awUnk006C[6];
	dwScale = *(DWORD*)&m_wUnk0080;
	pdwPos = pPlayer->Sub_4038A0(adwPt2);
	dwProj = pdwPos[1];
	HOVER_FIXDIVPL(dwProj, dwScale);
	dwBase = *(DWORD*)&m_awUnk006C[8];
	dwBase -= dwProj;
	g_dwUnk_4A47E4 = dwBase - *(DWORD*)&m_awUnk006C[4];
	pos = theApp.m_pBumperDoc->m_list838C.GetHeadPosition();
	while (pos != NULL)
	{
		CGameObj* pObj;

		pObj = (CGameObj*)theApp.m_pBumperDoc->m_list838C.GetNext(pos);
		pObj->m_dwUnk3C = 0;
		Sub_404BB0(pDoc, pObj, dwUnk540);
	}
	pos = theApp.m_pBumperDoc->m_list83A8.GetHeadPosition();
	while (pos != NULL)
	{
		CGameObj* pObj;

		pObj = (CGameObj*)theApp.m_pBumperDoc->m_list83A8.GetNext(pos);
		Sub_404BB0(pDoc, pObj, dwUnk540);
	}
	if (theApp.m_dwUnk01E0 == 0)
		return;
	for (i = 0; i < g_obArrUnk_4A4840.GetSize(); i++)
	{
		pPod = (CPodObj*)g_obArrUnk_4A4840[i];
		if ((long)pPod->Sub_403880(adwA)[0] < (long)g_dwUnk_4A47BC)
			pPod->m_dwUnk3C = 0;
		else if ((long)pPod->Sub_403880(adwB)[0] > (long)g_dwUnk_4A4798)
			pPod->m_dwUnk3C = 0;
		else if ((long)pPod->Sub_403880(adwC)[1] < (long)g_dwUnk_4A47D8)
			pPod->m_dwUnk3C = 0;
		else
		{
			pPod->m_dwUnk3C = 1;
			if ((long)pPod->Sub_403880(adwD)[1] > (long)g_dwUnk_4A479C)
				pPod->m_dwUnk3C = 0;
		}
		pPod->Sub_403880(adwE);
		Sub_4055B0(&pPod->m_dwUnk9C, adwE[0], adwE[1], g_dwUnk_4A4790,
			g_dwUnk_4A4794, g_dwUnk_4A47E0, g_dwUnk_4A47E4, g_dwUnk_4A47B8);
		pPod->m_dwUnk40 = m_dwUnk0098;
	}
}

// Visibility-tests one object against the viewport rectangle and projects
// its points with Sub_4055B0: wall endpoints (+0x84/+0x8C), temp-wall-pod
// rect corners (+0xF0/+0xF8), pod center (+0xC0) or the player triangle
// (+0x164..+0x178). Sets the object's m_dwUnk3C visible flag and the
// m_dwUnk40 draw color.

// hover.exe:0x404BB0
// FUNCTION: HOVER 0x404bb0
void CSub422F50::Sub_404BB0(CBumperDoc* pDoc, CGameObj* pObj, int bUnk)
{
	DWORD* pSub;
	DWORD* pRect;
	DWORD adwPt[2];
	volatile long* plPt;
	long lClip;
	DWORD adwPl[2];
	DWORD dwT;
	DWORD dwCap;
	volatile DWORD dwCap2;
	DWORD dwCosHalf;
	DWORD dwSinHalf;
	DWORD dwSinCap;
	DWORD dwNegCosCap;
	long lAngle;
	CWall* pWall;
	CPodObj* pPod;
	CPlayer* pPlayer;
	POINT ptHidden;
	POINT ptOrg;
	DWORD* pdwMax;
	POINT* pPt1;
	POINT* pPt2;
	POINT* pPt3;
	DWORD* pdwPt;
	DWORD dwWide;
	DWORD dwCapA;
	DWORD dwCapB;
	volatile DWORD dwCapMax;
	DWORD dwP1;
	DWORD dwP2;
	DWORD dwP3;
	DWORD dwP4;
	long lHiCos;
	long lHiSin;
	long lSinCap;
	long lNegCos;
	long lT;
	long lU;

	// The "not visible" marker point, stored into the object's projected
	// point pairs; the original builds it once in the entry block.
	ptHidden.x = -10;
	ptHidden.y = -10;

	if (pObj->IsKindOf(RUNTIME_CLASS(CWall)))
	{
		pWall = (CWall*)pObj;
		if (pObj->m_dwUnk38 != 0)
			return;
		if (bUnk == 0)
		{
			pSub = (DWORD*)pObj->m_dwUnk78;
			if (pSub == NULL || (*(BYTE*)((DWORD)pSub + 0x24) & 8) == 0)
			{
				*(POINT*)&pWall->m_dwUnk84 = ptHidden;
				*(POINT*)&pWall->m_dwUnk8C = ptHidden;
				return;
			}
		}
		if (Hover_PtOf(&pObj->m_obj04.m_dwUnk00).x < (long)g_dwUnk_4A47BC &&
			Hover_PtOf(&pObj->m_obj04.m_dwUnk08).x < (long)g_dwUnk_4A47BC)
			return;
		if (Hover_PtOf(&pObj->m_obj04.m_dwUnk00).x > (long)g_dwUnk_4A4798 &&
			Hover_PtOf(&pObj->m_obj04.m_dwUnk08).x > (long)g_dwUnk_4A4798)
			return;
		if (Hover_PtOf(&pObj->m_obj04.m_dwUnk00).y < (long)g_dwUnk_4A47D8 &&
			Hover_PtOf(&pObj->m_obj04.m_dwUnk08).y < (long)g_dwUnk_4A47D8)
			return;
		if (Hover_PtOf(&pObj->m_obj04.m_dwUnk00).y > (long)g_dwUnk_4A479C &&
			(long)pObj->m_obj04.m_dwUnk0C > (long)g_dwUnk_4A479C)
			return;
		Hover_Project(this, &pWall->m_dwUnk84,
			Hover_PtOf(&pObj->m_obj04.m_dwUnk00));
		Hover_Project(this, &pWall->m_dwUnk8C,
			Hover_PtOf(&pObj->m_obj04.m_dwUnk08));
		pObj->m_dwUnk3C = 1;
		return;
	}
	if (pObj->IsKindOf(RUNTIME_CLASS(CTempWallPod)) && pObj->m_dwUnk28 != 0)
	{
		pPod = (CPodObj*)pObj;
		pRect = (DWORD*)pPod->m_dwUnkD4;
		if ((long)pRect[5] < (long)g_dwUnk_4A47BC &&
			(long)pRect[7] < (long)g_dwUnk_4A47BC)
			return;
		if ((long)pRect[5] > (long)g_dwUnk_4A4798 &&
			(long)pRect[7] > (long)g_dwUnk_4A4798)
			return;
		if ((long)pRect[6] < (long)g_dwUnk_4A47D8 &&
			(long)pRect[8] < (long)g_dwUnk_4A47D8)
			return;
		if ((long)pRect[6] > (long)g_dwUnk_4A479C &&
			(long)pRect[8] > (long)g_dwUnk_4A479C)
			return;
		// Both corners go through the by-value POINT forwarder: the
		// original fills two fresh adjacent 8-byte slot pairs
		// (hover.exe:0x404e1e/0x404e2e and 0x404e3c/0x404e48) before the
		// first call and reloads them for the pushes.
		Hover_Project(this, &pPod->m_dwUnkF0, *(POINT*)&pRect[5]);
		Hover_Project(this, &pPod->m_dwUnkF8, *(POINT*)&pRect[7]);
		pObj->m_dwUnk3C = 1;
		return;
	}
	if (pObj->IsKindOf(RUNTIME_CLASS(CPodObj)))
	{
		pPod = (CPodObj*)pObj;
		if (pPod->m_dwUnk88 != 0)
			return;
		if (bUnk == 0)
		{
			pSub = (DWORD*)pObj->m_dwUnk78;
			if (pSub == NULL || (*(BYTE*)((DWORD)pSub + 0x24) & 8) == 0)
				goto HidePod;
		}
		if (pObj->m_dwUnk38 != 0)
		{
			// The three "not drawn" exits share one store block in the
			// original, placed after the projection code.
HidePod:
			pPod->m_dwUnkC0 = 0xFFFFFFF6;
			pPod->m_dwUnkC4 = 0xFFFFFFF6;
			return;
		}
		pPod->Sub_403880(adwPt);
		// The four clip tests re-read the projected centre from memory and
		// keep the viewport bound in the register (rule 121).
		plPt = (volatile long*)adwPt;
		lClip = (long)g_dwUnk_4A47BC;
		if (plPt[0] < lClip)
			return;
		lClip = (long)g_dwUnk_4A4798;
		if (plPt[0] > lClip)
			return;
		lClip = (long)g_dwUnk_4A47D8;
		if (plPt[1] < lClip)
			return;
		lClip = (long)g_dwUnk_4A479C;
		if (plPt[1] > lClip)
			return;
		Hover_Project(this, &pPod->m_dwUnkC0, *(POINT*)adwPt);
		pObj->m_dwUnk3C = 1;
		if (pObj->IsKindOf(RUNTIME_CLASS(CFlag)))
		{
			if ((long)m_dwUnk00AC < 4)
			{
				pObj->m_dwUnk3C = 0;
				return;
			}
			if (pPod->m_dwUnkCC == 0)
				pObj->m_dwUnk40 = m_dwUnk008C;
			else
				pObj->m_dwUnk40 = m_dwUnk0090;
		}
		else
		{
			pObj->m_dwUnk40 = m_dwUnk0098;
		}
		return;
	}
	if (pObj->IsKindOf(RUNTIME_CLASS(CPlayer)))
	{
		pPlayer = (CPlayer*)pObj;
		// The two half-extents are 16.16 values built as word pairs, the
		// cap is max()'d through a reference-returning helper and every
		// product goes through the pointer-flavour macros (rule 50).
		((WORD*)&dwT)[0] = 0;
		((WORD*)&dwT)[1] = 0xC8;
		dwWide = *(DWORD*)&m_wUnk0080;
		dwCap = *(DWORD*)&m_wUnk0080;
		HOVER_FIXMULWP(dwCap, 2);
		// The cap is max()'d through a pointer select over two copies -
		// hover.exe:0x405071 compares once and branches twice (`jl` then
		// `je`, i.e. `a < b || a == b`), takes the address of the winner
		// and dereferences it.
		dwCapA = dwCap;
		if ((long)dwCapA < 0x320000 || dwCapA == 0x320000)
		{
			((WORD*)&dwCapB)[0] = 0;
			((WORD*)&dwCapB)[1] = 0x32;
			pdwMax = &dwCapB;
		}
		else
		{
			pdwMax = &dwCapA;
		}
		dwCapMax = *pdwMax;
		dwCap2 = dwWide;
		HOVER_FIXMULWP(*(DWORD*)&dwCap2, 8);
		// dwCap2 is volatile because the macro writes it through a pointer
		// that VC 2.2 does not follow: without it the compiler forwards the
		// pre-multiply copy of dwWide into both the comparison and the
		// store, i.e. the cap ends up eight times too small (the original
		// re-reads the product out of dwCap2's own slot).
		dwWide = dwCap2;
		if ((long)dwT < (long)dwWide)
			dwT = dwWide;
		((short*)&dwCosHalf)[1] = 0;
		((short*)&dwSinHalf)[1] = 0;
		((short*)&dwSinCap)[1] = 0;
		((short*)&dwNegCosCap)[1] = 0;
		dwT = (long)dwT / 2;
		lAngle = (short)pPlayer->m_awUnk0194[5];
		dwP1 = g_adwSinTable[(((lAngle + 0x80) & 0x1FF) << 4) + 1];
		HOVER_FIXMULP(dwP1, dwT);
		g_dwUnk_4A47B8 = lAngle;
		dwCosHalf = dwP1;
		dwP2 = g_adwSinTable[((lAngle & 0x1FF) << 4) + 1];
		HOVER_FIXMULP(dwP2, dwT);
		lAngle = (short)pPlayer->m_awUnk0194[5] - 0x80;
		dwSinHalf = dwP2;
		dwP3 = g_adwSinTable[(((lAngle + 0x80) & 0x1FF) << 4) + 1];
		HOVER_FIXMULP(dwP3, dwCapMax);
		g_dwUnk_4A47B8 = lAngle;
		dwSinCap = dwP3;
		dwP4 = g_adwSinTable[((lAngle & 0x1FF) << 4) + 1];
		HOVER_FIXMULP(dwP4, dwCapMax);
		dwNegCosCap = dwP4;
		pdwPt = pPlayer->Sub_4038A0(adwPl);
		// The triangle origin is one POINT copied into all three corners;
		// hover.exe:0x405235 stores it into its own slot pair and reloads
		// it for the second and third store.
		ptOrg.x = ((short*)pdwPt)[1];
		ptOrg.y = ((short*)pdwPt)[3];
		pPt1 = (POINT*)&pPlayer->m_dwUnk0164;
		pPt2 = (POINT*)&pPlayer->m_dwUnk016C;
		pPt3 = (POINT*)&pPlayer->m_dwUnk0174;
		*pPt1 = ptOrg;
		*pPt2 = ptOrg;
		*pPt3 = ptOrg;
		lHiCos = ((short*)&dwCosHalf)[1];
		lHiSin = ((short*)&dwSinHalf)[1];
		pPlayer->m_dwUnk0164 += lHiCos;
		pPlayer->m_dwUnk0168 += lHiSin;
		lSinCap = ((short*)&dwSinCap)[1];
		lT = lSinCap;
		lT -= lHiCos;
		pPt2->x += lT;
		lNegCos = ((short*)&dwNegCosCap)[1];
		lU = lNegCos;
		lU -= lHiSin;
		pPt3->x -= lHiCos + lSinCap;
		pPlayer->m_dwUnk0170 += lU;
		pPlayer->m_dwUnk0178 -= lHiSin + lNegCos;
		lAngle = (short)pDoc->m_humanPlayer.m_awUnk0194[5] - 0x80;
		g_dwUnk_4A47B8 = lAngle;
		// The three triangle projections pass the two coordinates as
		// scalars read back through the POINT* locals, not as a POINT by
		// value: hover.exe:0x40530e..0x405381 pushes straight out of
		// [esi+0x164]/[ebp]/[ebx] with no 8-byte argument temp.
		Sub_4055B0((DWORD*)pPt1, pPt1->x, pPlayer->m_dwUnk0168,
			g_dwUnk_4A4790, g_dwUnk_4A4794, g_dwUnk_4A47E0,
			g_dwUnk_4A47E4, g_dwUnk_4A47B8);
		Sub_4055B0((DWORD*)pPt2, pPt2->x, pPt2->y, g_dwUnk_4A4790,
			g_dwUnk_4A4794, g_dwUnk_4A47E0, g_dwUnk_4A47E4,
			g_dwUnk_4A47B8);
		Sub_4055B0((DWORD*)pPt3, pPt3->x, pPt3->y, g_dwUnk_4A4790,
			g_dwUnk_4A4794, g_dwUnk_4A47E0, g_dwUnk_4A47E4,
			g_dwUnk_4A47B8);
		pPlayer->m_dwUnk017C = 1;
		return;
	}
}

// Projects the world point (lX,lY) into pdwOut[0..1]: when
// theApp.m_dwUnk01D4 is set the point is first rotated by -lAngle around
// (lCX,lCY); the projection divides by the m_wUnk0080 factor and offsets
// into the screen center (m_dwUnk5C/m_dwUnk60).
//
// The rotation is done with 16.16 fixed-point values and the sin/cos table:
// lRX = (cos*dx + sin*dy) >> 16, lRY = (cos*dy - sin*dx) >> 16.
//
// This function is one of the two in hover.exe that are NOT optimized (the
// other is the robot steering pass, see rule 49 in notes/perfect-match.md):
// it has an ebp frame, every value homed on the stack and a five-byte
// `jmp $+5` at the end of every inline expansion, while still expanding
// inline functions and still calling CFixed::Sub_4038E0 out of line. That
// combination is `#pragma optimize("g", off)` on top of our /O2 build (see
// rule 117). Every fixed-point step is therefore visible as an expansion of
// one of the Hover_Fix* helpers above.

#pragma optimize("g", off)

// hover.exe:0x4055B0
// FUNCTION: HOVER 0x4055b0
void CSub422F50::Sub_4055B0(DWORD* pdwOut, long lX, long lY, long lCX,
	long lCY, long lV1, long lV2, long lAngle)
{
	long lQY;
	long lPX;
	long lPY;

	{
		long lQX;
		__int64 llQ;

		lPX = lX;
		lPY = lY;
		if (theApp.m_dwUnk01D4 != 0)
		{
			DWORD dwDX;
			DWORD dwDY;
			DWORD dwSin;
			DWORD dwA;
			DWORD dwCos;
			DWORD dwB;
			DWORD dwT;
			DWORD dwSin2;
			DWORD dwC;
			DWORD dwCos2;
			DWORD dwD;
			DWORD dwT2;

			Hover_FixSetInt(dwDX, lPX);
			Hover_FixSetInt(dwDY, lPY);
			Hover_FixSubInt(dwDX, lCX);
			Hover_FixSubInt(dwDY, lCY);
			Hover_FixSinTo(dwSin, lAngle);
			Hover_FixCosTo(dwCos, lAngle);
			Hover_FixMulTo(dwA, *(CFixed*)&dwSin, *(CFixed*)&dwDY);
			Hover_FixMulTo(dwB, *(CFixed*)&dwCos, *(CFixed*)&dwDX);
			Hover_FixAddTo(dwT, *(CFixed*)&dwB, *(CFixed*)&dwA);
			lPX = ((short*)&dwT)[1];
			Hover_FixSinTo(dwSin2, lAngle);
			Hover_FixCosTo(dwCos2, lAngle);
			Hover_FixMulTo(dwC, *(CFixed*)&dwSin2, *(CFixed*)&dwDX);
			Hover_FixMulTo(dwD, *(CFixed*)&dwCos2, *(CFixed*)&dwDY);
			Hover_FixSubTo(dwT2, *(CFixed*)&dwD, *(CFixed*)&dwC);
			lPY = ((short*)&dwT2)[1];
			lPX += lCX;
			lPY += lCY;
		}
		llQ = lPX;
		Hover_Shl32(llQ);
		llQ = llQ / (long)*(DWORD*)&m_wUnk0080;
		Hover_FixPack(lQX, llQ);
		llQ = lPY;
		Hover_Shl32(llQ);
		llQ = llQ / (long)*(DWORD*)&m_wUnk0080;
		Hover_FixPack(lQY, llQ);
		lQX = (long)Hover_FixValue(Hover_FixSub(
			Hover_FixSub(*(CFixed*)&m_awUnk006C[8], *(CFixed*)&lQX),
			*(CFixed*)&lV1));
		lQY = (long)Hover_FixValue(Hover_FixSub(
			Hover_FixSub(*(CFixed*)&m_awUnk006C[8], *(CFixed*)&lQY),
			*(CFixed*)&lV2));
		pdwOut[0] = m_dwUnk5C + (long)((short*)&lQX)[1];
		pdwOut[1] = m_dwUnk60 + (long)((short*)&lQY)[1];
	}
}
#pragma optimize("", on)

// Draws every projected object into the pixel bits (walls/temp-walls as
// lines, pods as cross markers, players as triangles), then the tracked
// object markers with their selection diamonds and link lines.

// hover.exe:0x4053A0
// FUNCTION: HOVER 0x4053a0
void CSub422F50::Sub_4053A0(CBumperView* pView, CBumperDoc* pDoc)
{
	POSITION pos;
	CGameObj* pObj;
	CPodObj* pPod;
	DWORD* pSub;
	DWORD* pPt;
	int i;
	int nLeft;
	long j;
	int nSub;
	POINT ptA;
	POINT ptB;
	POINT* pPtA;
	POINT* pPtB;
	long lX;
	long lY;
	BYTE bColor;
	WORD wColor;

	pos = theApp.m_pBumperDoc->m_list838C.GetHeadPosition();
	while (pos != NULL)
	{
		pObj = (CGameObj*)theApp.m_pBumperDoc->m_list838C.GetNext(pos);
		Sub_401C00(pObj, pView->m_dwUnk0540);
	}
	pos = theApp.m_pBumperDoc->m_list83A8.GetHeadPosition();
	while (pos != NULL)
	{
		pObj = (CGameObj*)theApp.m_pBumperDoc->m_list83A8.GetNext(pos);
		Sub_401C00(pObj, pView->m_dwUnk0540);
	}
	if (theApp.m_dwUnk01E0 == 0)
		return;
	if (g_obArrUnk_4A4840.GetSize() <= 0)
		return;
	// The beacon walk keeps its byte offset and its remaining count in two
	// stack slots and counts the latter down (dec [esp+0x18] / jne).
	i = 0;
	nLeft = g_obArrUnk_4A4840.GetSize();
	do
	{
		pPod = (CPodObj*)g_obArrUnk_4A4840[i];
		if (pPod->m_dwUnk3C != 0)
		{
			bColor = (BYTE)pPod->m_dwUnk40;
			m_bUnk1C = bColor;
			wColor = (WORD)(bColor | ((WORD)bColor << 8));
			m_dwUnk20 = ((DWORD)wColor << 16) | wColor;
			// The projected marker point is reached through a pointer to the
			// +0x9C pair, as in the original (lea edi,[esi+0x9c] / [edi],
			// [edi+4]).
			pPt = &pPod->m_dwUnk9C;
			nSub = 0;
			Sub_403910(pPt[0], pPt[1], 5);
			if (pPod->m_dwUnkA8 != 0)
			{
				// The selection diamond walks one mutable corner pair around
				// the marker: the original keeps the four coordinates in stack
				// slots and adjusts them by +-0x14 between the four calls.
				pPtA = &ptA;
				pPtB = &ptB;
				lX = pPt[0];
				lY = pPt[1];
				pPtA->x = lX;
				pPtB->x = lX;
				pPtA->y = lY;
				pPtB->y = lY;
				pPtA->y -= 0xA;
				pPtB->x += 0xA;
				Sub_401980(pPtA->x, pPtA->y, pPtB->x, pPtB->y);
				pPtA->y += 0x14;
				Sub_401980(pPtB->x, pPtB->y, pPtA->x, pPtA->y);
				pPtB->x -= 0x14;
				Sub_401980(pPtA->x, pPtA->y, pPtB->x, pPtB->y);
				pPtA->y -= 0x14;
				Sub_401980(pPtB->x, pPtB->y, pPtA->x, pPtA->y);
			}
			for (j = (long)pPod->m_pUnk8C; j > 0; j--)
			{
				pSub = ((DWORD**)pPod->m_dwUnk88)[nSub];
				nSub++;
				Sub_401980(pPt[0], pPt[1], pSub[0x27], pSub[0x28]);
			}
		}
		i++;
	} while (--nLeft != 0);
}

// Draws one projected object: walls as a line, temp-wall pods as a line,
// pods as a cross marker, players as a triangle, each in its class color.

// hover.exe:0x401C00
// FUNCTION: HOVER 0x401c00
void CSub422F50::Sub_401C00(CGameObj* pObj, int bUnk)
{
	DWORD* pSub;
	BYTE bColor;
	WORD wColor;
	CWall* pWall;
	CPodObj* pPod;
	CPlayer* pPlayer;
	POINT* pPt1;
	POINT* pPt2;
	POINT* pPt3;

	if (pObj->IsKindOf(RUNTIME_CLASS(CWall)))
	{
		pWall = (CWall*)pObj;
		if (pObj->m_dwUnk38 != 0)
			return;
		if (bUnk == 0)
		{
			pSub = (DWORD*)pObj->m_dwUnk78;
			if (pSub == NULL || (*(BYTE*)((DWORD)pSub + 0x24) & 8) == 0)
				return;
		}
		if (pObj->m_dwUnk3C == 0)
			return;
		pSub = (DWORD*)pObj->m_dwUnk78;
		if (pSub[0x2F] == 0)
			bColor = (BYTE)m_dwUnk0098;
		else if (((short*)pSub)[0x52] > 0)
			bColor = (BYTE)m_dwUnk00A8;
		else
			bColor = (BYTE)m_dwUnk009C;
		m_bUnk1C = bColor;
		wColor = (WORD)(bColor | ((WORD)bColor << 8));
		m_dwUnk20 = ((DWORD)wColor << 16) | wColor;
		pPt2 = (POINT*)&pWall->m_dwUnk8C;
		pPt1 = (POINT*)&pWall->m_dwUnk84;
		Sub_401980(pPt1->x, pPt1->y, pPt2->x, pPt2->y);
		return;
	}
	if (pObj->IsKindOf(RUNTIME_CLASS(CTempWallPod)) && pObj->m_dwUnk28 != 0)
	{
		pPod = (CPodObj*)pObj;
		bColor = (BYTE)m_dwUnk009C;
		m_bUnk1C = bColor;
		wColor = (WORD)(bColor | ((WORD)bColor << 8));
		m_dwUnk20 = ((DWORD)wColor << 16) | wColor;
		pPt2 = (POINT*)&pPod->m_dwUnkF8;
		pPt1 = (POINT*)&pPod->m_dwUnkF0;
		Sub_401980(pPt1->x, pPt1->y, pPt2->x, pPt2->y);
		return;
	}
	if (pObj->IsKindOf(RUNTIME_CLASS(CPodObj)))
	{
		pPod = (CPodObj*)pObj;
		if (bUnk == 0)
		{
			pSub = (DWORD*)pObj->m_dwUnk78;
			if (pSub == NULL || (*(BYTE*)((DWORD)pSub + 0x24) & 8) == 0)
				return;
		}
		if (pObj->m_dwUnk3C == 0)
			return;
		bColor = (BYTE)pObj->m_dwUnk40;
		m_bUnk1C = bColor;
		wColor = (WORD)(bColor | ((WORD)bColor << 8));
		m_dwUnk20 = ((DWORD)wColor << 16) | wColor;
		pPt1 = (POINT*)&pPod->m_dwUnkC0;
		Sub_403910(pPt1->x, pPt1->y, 5);
		return;
	}
	if (pObj->IsKindOf(RUNTIME_CLASS(CPlayer)))
	{
		pPlayer = (CPlayer*)pObj;
		if (pObj->IsKindOf(RUNTIME_CLASS(CHumanPlayer)))
		{
			bColor = (BYTE)m_dwUnk008C;
			m_bUnk1C = bColor;
			wColor = (WORD)(bColor | ((WORD)bColor << 8));
			m_dwUnk20 = ((DWORD)wColor << 16) | wColor;
		}
		else if (pObj->IsKindOf(RUNTIME_CLASS(CRobotPlayer)))
		{
			if (*(DWORD*)((BYTE*)pPlayer + 0xC0) != 0)
				bColor = (BYTE)m_dwUnk0098;
			else if (((CRobotPlayer*)pPlayer)->m_dwUnk021C != 0)
				bColor = (BYTE)m_dwUnk0094;
			else
				bColor = (BYTE)m_dwUnk0090;
			m_bUnk1C = bColor;
			wColor = (WORD)(bColor | ((WORD)bColor << 8));
			m_dwUnk20 = ((DWORD)wColor << 16) | wColor;
		}
		pPt2 = (POINT*)&pPlayer->m_dwUnk016C;
		pPt1 = (POINT*)&pPlayer->m_dwUnk0164;
		pPt3 = (POINT*)&pPlayer->m_dwUnk0174;
		Sub_401980(pPt1->x, pPt1->y, pPt2->x, pPt2->y);
		Sub_401980(pPt2->x, pPt2->y, pPt3->x, pPt3->y);
		Sub_401980(pPt3->x, pPt3->y, pPt1->x, pPt1->y);
		return;
	}
}

// Bresenham line into the pixel bits, clipped by Sub_401520 and drawn in
// the current color (m_bUnk1C); y is flipped against m_dwUnk18.

// hover.exe:0x401980
// FUNCTION: HOVER 0x401980
void CSub422F50::Sub_401980(long lX1, long lY1, long lX2, long lY2)
{
	// The two clip calls hand the argument block over as two coordinate
	// pairs and write the clipped endpoints straight back into it, so
	// every read of the four coordinates goes through these pointers: only
	// lX1/lX2 have had their address taken, which leaves VC free to keep
	// lY1/lY2 in registers across the calls -- the line is then drawn with
	// its *unclipped* y and writes past the end of the bitmap. The
	// original reloads all four out of the argument slots after the calls
	// and stores the flipped y values back into them
	// (hover.exe:0x4019D8..0x401A0C).
	long* plA = &lX1;
	long* plB = &lX2;
	long lDX;
	long lDY;
	long lPitch;
	long lErr;
	long lStep;
	long lSlope;
	long i;
	BYTE* pDst;
	BYTE bColor;

	m_dwUnk2C = abs(plA[0] - plB[0]);
	m_dwUnk30 = abs(plA[1] - plB[1]);
	if (Sub_401520(plA, plB) == 0)
		return;
	if (Sub_401520(plB, plA) == 0)
		return;
	plA[1] = m_dwUnk18 - plA[1] - 1;
	plB[1] = m_dwUnk18 - plB[1] - 1;
	lDX = abs(plA[0] - plB[0]);
	m_dwUnk2C = lDX;
	lDY = abs(plA[1] - plB[1]);
	m_dwUnk30 = lDY;
	if (lDX == 0 && lDY == 0)
		return;
	lPitch = m_dwUnk24;
	pDst = m_pUnk10 + lPitch * plA[1] + plA[0];
	if (lDX > lDY)
	{
		lSlope = (lDY << 16) / lDX;
		lErr = 0x8000;
		if (plB[1] < plA[1])
			lPitch = -lPitch;
		lStep = (plB[0] < plA[0]) ? -1 : 1;
		bColor = m_bUnk1C;
		for (i = lDX; i > 0; i--)
		{
			lErr += lSlope;
			*pDst = bColor;
			if ((lErr & 0xFFFF0000) != 0)
			{
				pDst += lPitch;
				lErr &= 0xFFFF;
			}
			pDst += lStep;
		}
	}
	else
	{
		lSlope = (lDX << 16) / lDY;
		lErr = 0x8000;
		if (plB[0] < plA[0])
			lSlope = -lSlope;
		if (plB[1] < plA[1])
			lPitch = -lPitch;
		bColor = m_bUnk1C;
		for (i = lDY; i > 0; i--)
		{
			lStep = lErr >> 16;
			lErr += lSlope;
			pDst[lStep] = bColor;
			pDst += lPitch;
		}
	}
}

// Draws a small cross marker at (lX,lY) in the current color, clipped
// against the +0x00..+0x0C rect.

// hover.exe:0x403910
// FUNCTION: HOVER 0x403910
void CSub422F50::Sub_403910(long lX, long lY, long /*lUnk*/)
{
	register long lYFlip;
	register BYTE* pBits;
	register long lPitch;
	BYTE bColor;
	BYTE* pUp;
	BYTE* pDown;
	BYTE* pCenter;

	if ((long)m_dwUnk04 + 1 > lY)
		return;
	if ((long)m_dwUnk00 + 1 > lX)
		return;
	if ((long)m_dwUnk0C - 1 < lY)
		return;
	if ((long)m_dwUnk08 - 1 < lX)
		return;

	lYFlip = (long)m_dwUnk18 - lY;
	pBits = m_pUnk10;
	lPitch = (long)m_dwUnk24;

	pUp = pBits + (lYFlip - 2) * lPitch + lX;
	lYFlip--;
	bColor = m_bUnk1C;
	*pUp = bColor;
	pDown = pBits + (lYFlip + 1) * lPitch + lX;
	*pDown = bColor;
	pCenter = pBits + lYFlip * lPitch + lX;
	*(pCenter - 1) = bColor;
	*pCenter = bColor;
	*(pCenter + 1) = bColor;
}

// Clips the line endpoint A (plA[0]=x, plA[1]=y) against the +0x00..+0x0C
// clip rectangle, using the other endpoint B (plB) as the reference.  Returns
// 1 if A is now inside (or on) the rectangle, 0 if the line is completely
// outside.  The |dx|/|dy| of the line must already be stored in m_dwUnk2C/
// m_dwUnk30 by the caller (Sub_401980).

// hover.exe:0x401520
// FUNCTION: HOVER 0x401520
int CSub422F50::Sub_401520(long* plA, long* plB)
{
	// The clip bounds are re-read from the object at every comparison in
	// the original (`cmp eax, [ecx]`); reading them through a volatile
	// pointer keeps VC from CSE-ing them into two stack slots, which is
	// what grew our frame from 4 bytes to 0x18 and spilled plB.
	volatile DWORD* pdwClip = &m_dwUnk00;
	volatile long* pB = plB;
	long lX1 = plA[0];
	long lTemp;

	if (lX1 < (long)pdwClip[0])
	{
		// The original re-reads plA[1] fresh inside each of the three
		// outer x branches instead of caching it once at the top.
		long lY1 = plA[1];

		if (lY1 < (long)pdwClip[1])
		{
			if (m_dwUnk2C == 0 || m_dwUnk30 == 0)
				return 0;

			// Clip A against the top edge first.
			lTemp = pB[0] + ((((lX1 - pB[0]) << 16) / (lY1 - pB[1]))
				* ((long)pdwClip[1] - pB[1]) >> 16);
			if (lTemp >= (long)pdwClip[0] && lTemp <= (long)pdwClip[2])
			{
				plA[0] = lTemp;
				plA[1] = (long)pdwClip[1];
				return 1;
			}

			// Then try the left edge.
			lTemp = pB[1] + ((((lY1 - pB[1]) << 16) / (lX1 - pB[0]))
				* ((long)pdwClip[0] - pB[0]) >> 16);
			if (lTemp >= (long)pdwClip[1] && lTemp <= (long)pdwClip[3])
			{
				plA[0] = (long)pdwClip[0];
				plA[1] = lTemp;
				return 1;
			}
			return 0;
		}
		else if (lY1 > (long)pdwClip[3])
		{
			if (m_dwUnk2C == 0 || m_dwUnk30 == 0)
				return 0;

			// Clip A against the bottom edge first.
			lTemp = pB[0] + ((((lX1 - pB[0]) << 16) / (lY1 - pB[1]))
				* ((long)pdwClip[3] - pB[1]) >> 16);
			if (lTemp >= (long)pdwClip[0] && lTemp <= (long)pdwClip[2])
			{
				plA[0] = lTemp;
				plA[1] = (long)pdwClip[3];
				return 1;
			}

			// Then try the left edge.
			lTemp = pB[1] + ((((lY1 - pB[1]) << 16) / (lX1 - pB[0]))
				* ((long)pdwClip[0] - pB[0]) >> 16);
			if (lTemp >= (long)pdwClip[1] && lTemp <= (long)pdwClip[3])
			{
				plA[0] = (long)pdwClip[0];
				plA[1] = lTemp;
				return 1;
			}
			return 0;
		}
		else
		{
			if (m_dwUnk2C == 0)
				return 0;

			lTemp = pB[1] + ((((lY1 - pB[1]) << 16) / (lX1 - pB[0]))
				* ((long)pdwClip[0] - pB[0]) >> 16);
			if (lTemp < (long)pdwClip[1] || lTemp > (long)pdwClip[3])
				return 0;

			plA[0] = (long)pdwClip[0];
			plA[1] = lTemp;
			return 1;
		}
	}
	else if (lX1 > (long)pdwClip[2])
	{
		long lY1 = plA[1];

		if (lY1 < (long)pdwClip[1])
		{
			if (m_dwUnk2C == 0 || m_dwUnk30 == 0)
				return 0;

			// Clip A against the top edge first.
			lTemp = pB[0] + ((((lX1 - pB[0]) << 16) / (lY1 - pB[1]))
				* ((long)pdwClip[1] - pB[1]) >> 16);
			if (lTemp >= (long)pdwClip[0] && lTemp <= (long)pdwClip[2])
			{
				plA[0] = lTemp;
				plA[1] = (long)pdwClip[1];
				return 1;
			}

			// Then try the right edge.
			lTemp = pB[1] + ((((lY1 - pB[1]) << 16) / (lX1 - pB[0]))
				* ((long)pdwClip[2] - pB[0]) >> 16);
			if (lTemp >= (long)pdwClip[1] && lTemp <= (long)pdwClip[3])
			{
				plA[0] = (long)pdwClip[2];
				plA[1] = lTemp;
				return 1;
			}
			return 0;
		}
		else if (lY1 > (long)pdwClip[3])
		{
			if (m_dwUnk2C == 0 || m_dwUnk30 == 0)
				return 0;

			// Clip A against the bottom edge first.
			lTemp = pB[0] + ((((lX1 - pB[0]) << 16) / (lY1 - pB[1]))
				* ((long)pdwClip[3] - pB[1]) >> 16);
			if (lTemp >= (long)pdwClip[0] && lTemp <= (long)pdwClip[2])
			{
				plA[0] = lTemp;
				plA[1] = (long)pdwClip[3];
				return 1;
			}

			// Then try the right edge.
			lTemp = pB[1] + ((((lY1 - pB[1]) << 16) / (lX1 - pB[0]))
				* ((long)pdwClip[2] - pB[0]) >> 16);
			if (lTemp >= (long)pdwClip[1] && lTemp <= (long)pdwClip[3])
			{
				plA[0] = (long)pdwClip[2];
				plA[1] = lTemp;
				return 1;
			}
			return 0;
		}
		else
		{
			if (m_dwUnk2C == 0)
				return 0;

			lTemp = pB[1] + ((((lY1 - pB[1]) << 16) / (lX1 - pB[0]))
				* ((long)pdwClip[2] - pB[0]) >> 16);
			if (lTemp < (long)pdwClip[1] || lTemp > (long)pdwClip[3])
				return 0;

			plA[0] = (long)pdwClip[2];
			plA[1] = lTemp;
			return 1;
		}
	}
	else
	{
		long lY1 = plA[1];

		if (lY1 < (long)pdwClip[1])
		{
			if (m_dwUnk30 == 0)
				return 0;

			lTemp = pB[0] + ((((lX1 - pB[0]) << 16) / (lY1 - pB[1]))
				* ((long)pdwClip[1] - pB[1]) >> 16);
			if (lTemp < (long)pdwClip[0] || lTemp > (long)pdwClip[2])
				return 0;

			plA[0] = lTemp;
			plA[1] = (long)pdwClip[1];
			return 1;
		}
		else if (lY1 > (long)pdwClip[3])
		{
			if (m_dwUnk30 == 0)
				return 0;

			lTemp = pB[0] + ((((lX1 - pB[0]) << 16) / (lY1 - pB[1]))
				* ((long)pdwClip[3] - pB[1]) >> 16);
			if (lTemp < (long)pdwClip[0] || lTemp > (long)pdwClip[2])
				return 0;

			plA[0] = lTemp;
			plA[1] = (long)pdwClip[3];
			return 1;
		}
	}

	return 1;
}

// hover.exe:0x42D3E0
// FUNCTION: HOVER 0x42d3e0
void CSub422F50::Sub_42D3E0(CDC* pDC)
{
	CBumperView* pView;
	long lLeft;
	long lTop;
	long lWidth;
	long lHeight;
	long l2Left;
	long l2Top;
	long l2Width;
	long l2Height;

	pView = Hover_sub_401140();
	if (pView == NULL)
		return;
	if (pView->m_dwUnk0544 != 0)
	{
		// The 2x present doubles the four rectangle values into locals of
		// their own before deriving the source DC, which is what puts the
		// doubled values in stack slots in the original.
		lTop = (long)m_dwUnk60;
		lHeight = (long)m_dwUnk68 - lTop;
		lLeft = (long)m_dwUnk5C;
		lWidth = (long)m_dwUnk64 - lLeft;
		l2Height = lHeight * 2;
		l2Width = lWidth * 2;
		l2Top = lTop * 2;
		l2Left = lLeft * 2;
		StretchBlt(pDC->m_hDC, l2Left, l2Top, l2Width, l2Height,
			(m_pUnk0084 == NULL) ? NULL : m_pUnk0084->m_hDC,
			lLeft, lTop, lWidth, lHeight, SRCCOPY);
	}
	else
	{
		BitBlt(pDC->m_hDC, m_dwUnk5C, m_dwUnk60, m_dwUnk64 - m_dwUnk5C,
			m_dwUnk68 - m_dwUnk60,
			(m_pUnk0084 == NULL) ? NULL : m_pUnk0084->m_hDC,
			m_dwUnk5C, m_dwUnk60, SRCCOPY);
	}
}



/////////////////////////////////////////////////////////////////////////////
// CSub422BD0

// hover.exe:0x422BD0
// FUNCTION: HOVER 0x422bd0
CSub422BD0::CSub422BD0()
{
	m_dwUnk00 = 0;
	m_dwUnk04 = 0;
	m_dwUnk08 = 0;
	m_pUnk1C = NULL;
	m_dwUnk0C = 0;
	m_dwUnk10 = 0;
}

// hover.exe:0x422C20
// FUNCTION: HOVER 0x422c20
CSub422BD0::~CSub422BD0()
{
	::operator delete(m_pUnk1C);
}

/////////////////////////////////////////////////////////////////////////////
// CSub422C80

// hover.exe:0x422C80
// FUNCTION: HOVER 0x422c80
CSub422C80::CSub422C80()
{
	m_dwUnk00 = 0;
	m_dwUnk04 = 0;
	m_dwUnk08 = 10;
	m_dwUnk0C = 10;
	m_dwUnk10 = 0;
	m_dwUnk18 = 0;
	m_dwUnk14 = 0;
	m_dwUnk2C = 0;
	m_dwUnk1C = 1;
	m_dwUnk24 = 0;
}

/////////////////////////////////////////////////////////////////////////////
// CSub422C30

// hover.exe:0x422C30
// FUNCTION: HOVER 0x422c30
CSub422C30::CSub422C30()
{
	// The original only clears these eight members; m_dwUnk04/08/40/48 and
	// both source rectangles are left uninitialized here and filled in by
	// Sub_411BB0/Sub_411CF0 before the element is drawn.  The +0xC/+0x10
	// pair is written through a pointer local (rule 94: lea esi,[this+0xc]).
	m_dwUnk00 = 1;
	m_dwUnk4C = 0;
	m_dwUnk44 = 0;

	{
		POINT ptZero;

		ptZero.x = 0;
		ptZero.y = 0;

		m_pUnk14 = NULL;
		m_dwUnk3C = 1;
		m_pUnk18 = NULL;
		*(POINT*)&m_pUnk0C = ptZero;
	}
}

// hover.exe:0x422C60
// FUNCTION: HOVER 0x422c60
CSub422C30::~CSub422C30()
{
	::operator delete(m_pUnk14);
	::operator delete(m_pUnk18);
}

// Draws the dashboard sprite element into the render target.  Sub_411BB0/
// Sub_411CF0 precompute the destination base pointer (dwDestBits) and the
// sprite's y-coordinate/extents; this routine then draws m_dwUnk3C iterations,
// tiling the sprite across the destination: the left edge advances by one
// sprite width per iteration and the right edge stays one width ahead of it.
// The first m_dwUnk44 iterations draw from m_pUnk14/m_rcSource1,
// the rest from m_pUnk18/m_rcSource2.
//
// hover.exe:0x40E9A0
// FUNCTION: HOVER 0x40e9a0
void CSub422C30::Sub_40E9A0(DWORD dwDestBits, DWORD dwX, DWORD dwY)
{
	struct {
		long lWidth;
		long lOffsetX;
		long lDestY;
		long lDestBottom;
		RECT rcDest;
	} s;
	long lW;
	int i;
	RECT* pRect1;
	CSub422C30* pThis;

	pRect1 = &m_rcSource1;
	s.lWidth = m_rcSource1.right - pRect1->left;
	pThis = this;
	s.lDestY = pThis->m_dwUnk08;
	s.lDestBottom = pThis->m_rcSource1.bottom - pThis->m_rcSource1.top
		+ s.lDestY;

	if ((long)pThis->m_dwUnk3C <= 0)
		goto done;

	// The running left offset starts at zero and advances by one sprite
	// width per tile, in step with the right edge in lW (hover.exe:0x40E9D3
	// loads both from lWidth and the zeroed loop counter inside the count
	// guard, and bumps them by the width at 0x40EA92/0x40EA94).
	lW = s.lWidth;
	s.lOffsetX = 0;
	i = 0;

	do
	{
		long lX;
		BYTE* pSrc;
		RECT* pSrcRect;

		lX = pThis->m_dwUnk04 + s.lOffsetX;
		::SetRect(&s.rcDest, lX, s.lDestY, pThis->m_dwUnk04 + lW,
			s.lDestBottom);

		if ((int)pThis->m_dwUnk44 >= i + 1)
		{
			pSrc = (BYTE*)pThis->m_pUnk14;
			pSrcRect = pRect1;
		}
		else
		{
			pSrc = (BYTE*)pThis->m_pUnk18;
			pSrcRect = &pThis->m_rcSource2;
		}

		Hover_sub_40E730(
			pSrcRect->left, pSrcRect->top, pSrcRect->right, pSrcRect->bottom,
			(long)pThis->m_pUnk0C, (long)pThis->m_pUnk10,
			s.rcDest.left, s.lDestY, s.rcDest.right, s.rcDest.bottom,
			dwX, dwY,
			pSrc, (BYTE*)dwDestBits, 1);

		i++;
		lW += s.lWidth;
		s.lOffsetX += s.lWidth;
	}
	while (i < (int)pThis->m_dwUnk3C);

done:
	pThis->m_dwUnk00 = 0;
}

// Draws the decimal value stored in m_dwUnk08 as a right-aligned string of
// m_dwUnk04 digits into the render target.  Each digit is taken from the
// source tile bitmap at m_pUnk1C (ten tiles of size m_dwUnk14 x m_dwUnk18
// laid out horizontally) and blitted through Hover_sub_40E730.

// hover.exe:0x40FCE0
// FUNCTION: HOVER 0x40fce0
void CSub422BD0::Sub_40FCE0(DWORD dwUnkHdc, DWORD dwX, DWORD dwY)
{
	long lValue;
	int nTop;
	int nBottom;
	RECT rcSrc;
	RECT rcDst;
	int nIndex;
	int nDigit;
	DWORD* pdwOrigin;

	nTop = (int)m_dwUnk24;
	nBottom = (int)m_dwUnk18 + nTop;
	lValue = (long)m_dwUnk08;
	nIndex = (int)m_dwUnk04 - 1;

	if (nIndex < 0)
		goto Done;

	// The origin pair is read through a pointer local (rule 94).
	pdwOrigin = &m_dwUnk0C;

	do
	{
		int nSrcLeft;

		nDigit = (int)(lValue % 10);
		nSrcLeft = nDigit * (int)m_dwUnk14;

		::SetRect(&rcSrc,
			nSrcLeft,
			0,
			nSrcLeft + (int)m_dwUnk14,
			(int)m_dwUnk18);

		::SetRect(&rcDst,
			(int)m_dwUnk20 + nIndex * (int)m_dwUnk14,
			nTop,
			(int)m_dwUnk20 + (nIndex + 1) * (int)m_dwUnk14,
			nBottom);

		Hover_sub_40E730(
			(long)rcSrc.left, (long)rcSrc.top,
			(long)rcSrc.right, (long)rcSrc.bottom,
			(long)pdwOrigin[0], (long)pdwOrigin[1],
			(long)rcDst.left, (long)rcDst.top,
			(long)rcDst.right, (long)rcDst.bottom,
			(long)dwX, (long)dwY,
			(BYTE*)m_pUnk1C,
			(BYTE*)dwUnkHdc,
			1);

		lValue /= 10;
		nIndex--;
	} while (nIndex >= 0);

Done:
	m_dwUnk00 = 0;
}

// Draws the gauge into pDC as a series of rectangles. The gauge is laid out
// from m_dwUnk00/m_dwUnk04 with m_dwUnk08/m_dwUnk0C extents; m_dwUnk20 selects
// horizontal (non-zero) or vertical (zero) orientation. Each segment is
// m_dwUnk2C units wide/tall. Segments m_dwUnk24..m_dwUnk1C-1 are filled with
// the active brush m_dwUnk14, the earlier ones with the inactive brush
// m_dwUnk10, using the outline pen m_dwUnk18 for all segments. When bUnk is
// zero the loop stops at max(m_dwUnk24, m_dwUnk28) to redraw only the changed
// range. Coordinates are doubled when the view is in stretch mode and pDC is
// not the offscreen DC.
//
// hover.exe:0x401AD0
// FUNCTION: HOVER 0x401ad0
void CSub422C80::Sub_401AD0(CDC* pDC, int bUnk)
{
	int nMax;
	int nLeft;
	int nBottom;
	int nTop;
	int nRight;
	int i;

	nMax = ((int)m_dwUnk28 > (int)m_dwUnk24) ? (int)m_dwUnk28 : (int)m_dwUnk24;
	m_dwUnk28 = m_dwUnk24;

	nLeft = m_dwUnk00;
	nBottom = m_dwUnk0C;
	nTop = m_dwUnk04;
	nRight = m_dwUnk08;

	pDC->SaveDC();

	pDC->SelectObject((CPen*)m_dwUnk18);
	pDC->SelectObject((CBrush*)m_dwUnk10);

	for (i = 0; i < (int)m_dwUnk1C; i++)
	{
		if (bUnk == 0 && i > nMax)
			break;

		if (m_dwUnk20 != 0)
		{
			nRight = nLeft + (int)m_dwUnk2C;
		}
		else
		{
			nTop = nBottom - (int)m_dwUnk2C;
		}

		pDC->SelectObject(((int)m_dwUnk24 <= i)
			? (CBrush*)m_dwUnk14
			: (CBrush*)m_dwUnk10);

		if (Hover_sub_401140()->m_dwUnk0544 != 0
			&& pDC != &Hover_sub_401140()->m_dcUnk00B4)
		{
			::Rectangle(pDC->m_hDC,
				nLeft * 2, nTop * 2, nRight * 2, nBottom * 2);
		}
		else
		{
			::Rectangle(pDC->m_hDC,
				nLeft, nTop, nRight, nBottom);
		}

		if (m_dwUnk20 != 0)
			nLeft = nRight;
		else
			nBottom = nTop;
	}

	pDC->RestoreDC(-1);
}

// Stores the gauge's rendering parameters: segment count, inactive brush,
// outline pen and active brush. The original keeps these as raw DWORDs that
// are later cast back to CGdiObject-derived pointers by Sub_401AD0.
// hover.exe:0x411D60
// FUNCTION: HOVER 0x411d60
void CSub422C80::Sub_411D60(DWORD dwA, DWORD dwB, DWORD dwC, DWORD dwD)
{
	m_dwUnk10 = dwB;
	m_dwUnk18 = dwC;
	m_dwUnk14 = dwD;
	m_dwUnk1C = dwA;
}

// Sets up a dashboard number element. The source bitmap pBits is dwA x dwB.
// pRect points to the first of ten contiguous RECTs that describe the digit
// tiles 0..9 in pBits; the first rect's left/top gives the strip origin and
// the tenth rect's right/bottom gives the far end. A buffer large enough for
// ten tiles laid out horizontally is allocated at m_pUnk1C; m_dwUnk14/18 hold
// the tile size and m_dwUnk0C/10 hold the buffer dimensions.

// hover.exe:0x411A00
// FUNCTION: HOVER 0x411a00
void CSub422BD0::Sub_411A00(int nFlag, const RECT* pRect, BYTE* pBits,
	DWORD dwA, DWORD dwB)
{
	int nWidth;
	int nHeight;
	int nPitch;
	DWORD* pUnk0C;
	volatile DWORD dwLeft;
	volatile DWORD dwTop;
	volatile DWORD dwRight9;
	volatile DWORD dwBottom9;
	volatile DWORD dwZero1;
	volatile DWORD dwZero2;
	volatile DWORD dwPitch;
	volatile DWORD dwHeight;

	::operator delete(m_pUnk1C);

	m_dwUnk00 = 1;
	m_dwUnk04 = nFlag;

	nWidth = pRect->right - pRect->left;
	nHeight = pRect->bottom - pRect->top;

	pUnk0C = &m_dwUnk0C;

	m_dwUnk14 = nWidth;
	m_dwUnk18 = nHeight;

	nPitch = nWidth * 10;
	*pUnk0C = nPitch;
	if (nPitch % 2 != 0)
		nPitch++;
	*pUnk0C = nPitch;
	m_dwUnk10 = nHeight;

	m_pUnk1C = ::operator new((*pUnk0C) * nHeight);

	dwLeft = pRect->left;
	dwTop = pRect->top;
	dwRight9 = pRect[9].right;
	dwBottom9 = pRect[9].bottom;
	dwZero1 = 0;
	dwZero2 = 0;
	dwPitch = pUnk0C[0];
	dwHeight = m_dwUnk10;

	Hover_sub_40E730(
		(long)dwLeft,
		(long)dwTop,
		(long)dwRight9,
		(long)dwBottom9,
		(long)dwA,
		(long)dwB,
		(long)dwZero1,
		(long)dwZero2,
		(long)dwPitch,
		(long)dwHeight,
		(long)pUnk0C[0],
		(long)pUnk0C[1],
		pBits,
		(BYTE*)m_pUnk1C,
		1);

	m_dwUnk20 = 0;
	m_dwUnk24 = 0;
}

// Copies the first two fields of pRect (left/top) into the destination
// point stored at +0x20.  The right/bottom fields are not used.
// hover.exe:0x411920
// FUNCTION: HOVER 0x411920
void CSub422BD0::Sub_411920(const RECT* pRect)
{
	m_dwUnk20 = pRect->left;
	m_dwUnk24 = pRect->top;
}

// hover.exe:0x411BB0
// FUNCTION: HOVER 0x411bb0
void CSub422C30::Sub_411BB0(DWORD dwUnk, DWORD dwLeft1, DWORD dwTop1,
	DWORD dwRight1, DWORD dwBottom1, DWORD dwLeft2, DWORD dwTop2,
	DWORD dwRight2, DWORD dwBottom2, BYTE* pBits, DWORD dwWidth, DWORD dwHeight)
{
	RECT* pSource1 = &m_rcSource1;
	DWORD* pUnk0C = (DWORD*)&m_pUnk0C;
	long nRounded;

	m_dwUnk3C = dwUnk;
	*pUnk0C = dwRight1 - dwLeft1;
	m_pUnk10 = (void*)(dwBottom1 - dwTop1);

	::SetRect(pSource1, 0, 0, (int)*pUnk0C, (int)m_pUnk10);
	::SetRect(&m_rcSource2, 0, 0, (int)*pUnk0C, (int)m_pUnk10);

	::operator delete(m_pUnk14);
	::operator delete(m_pUnk18);

	// The tile pitch is rounded up to an even number with the signed
	// remainder idiom (cdq / xor / sub / and 1 / xor / sub).
	nRounded = (long)*pUnk0C;
	if (nRounded % 2 != 0)
		nRounded++;

	m_pUnk14 = ::operator new((DWORD)m_pUnk10 * nRounded);
	m_pUnk18 = ::operator new((DWORD)m_pUnk10 * nRounded);

	Hover_sub_40E730(
		(long)dwLeft1, (long)dwTop1, (long)dwRight1, (long)dwBottom1,
		(long)dwWidth, (long)dwHeight,
		(long)pSource1->left, (long)pSource1->top,
		(long)pSource1->right, (long)pSource1->bottom,
		(long)pUnk0C[0], (long)pUnk0C[1],
		pBits, (BYTE*)m_pUnk14, 1);

	{
		RECT* pSource2 = &m_rcSource2;

		Hover_sub_40E730(
			(long)dwLeft2, (long)dwTop2, (long)dwRight2, (long)dwBottom2,
			(long)dwWidth, (long)dwHeight,
			(long)pSource2->left, (long)pSource2->top,
			(long)pSource2->right, (long)pSource2->bottom,
			(long)pUnk0C[0], (long)pUnk0C[1],
			pBits, (BYTE*)m_pUnk18, 1);
	}

	{
		DWORD* pUnk04 = &m_dwUnk04;

		pUnk04[0] = 0;
		pUnk04[1] = 0;
	}
}

// Stores the top-left corner of pRect in the destination position fields
// (+0x04 and +0x08) used by Sub_40E9A0.
//
// hover.exe:0x411CF0
// FUNCTION: HOVER 0x411cf0
void CSub422C30::Sub_411CF0(const RECT* pRect)
{
	m_dwUnk04 = pRect->left;
	m_dwUnk08 = pRect->top;
}

/////////////////////////////////////////////////////////////////////////////
// CSub406C90

// FUNCTION: CSub406C90::~CSub406C90
// hover.exe:0x41B8C0
// FUNCTION: HOVER 0x41b8c0
CSub406C90::~CSub406C90()
{
	::operator delete(m_pUnk18);
}

/////////////////////////////////////////////////////////////////////////////
// CSub406090

// FUNCTION: CSub406090::CSub406090
// hover.exe:0x4191B0
// FUNCTION: HOVER 0x4191b0
CSub406090::CSub406090(int nWidth, int nHeight, DWORD dwPalArg)
{
	m_dwWidth = (DWORD)nWidth & ~3;
	m_dwHeight = (DWORD)nHeight;
	Sub_419440(&g_paletteUnk_4A1D50);
	m_dwNegWidth = (DWORD)-(long)m_dwWidth;
	m_dwUnk1C = (DWORD)-1;
	m_dwUnk24 = 1;
	m_pUnk00 = new CSub406C90(this);
	m_pUnk3C = NULL;
	m_pUnk40 = NULL;
	m_pUnk44 = NULL;
	Sub_4192F0(dwPalArg);
}

// FUNCTION: CSub406090::~CSub406090
// hover.exe:0x4193A0
// FUNCTION: HOVER 0x4193a0
CSub406090::~CSub406090()
{
	delete m_pUnk00;
	::operator delete(m_pUnk3C);
	::operator delete(m_pUnk40);
	::operator delete(m_pUnk44);
}

// Creates an 8-bpp bottom-up DIB section sized to m_dwWidth/m_dwHeight,
// attaches it to m_bitmap and stores the pixel bits pointer in m_dwUnk04.
// The palette is copied into the BITMAPINFO color table in the same shifted
// RGBA order used by CSub423030::Sub_411D80.

// hover.exe:0x419440
// FUNCTION: HOVER 0x419440
void CSub406090::Sub_419440(CPalette* pPalette)
{
	BITMAPINFO* pBmi;
	PALETTEENTRY apePalette[256];
	BYTE* pColorRed;
	PALETTEENTRY* pPe;
	HDC hScreenDC;
	HBITMAP hBitmap;

	hScreenDC = ::GetDC(NULL);

	pBmi = (BITMAPINFO*)::operator new(0x428);
	pBmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	pBmi->bmiHeader.biPlanes = 1;
	pBmi->bmiHeader.biWidth = m_dwWidth;
	pBmi->bmiHeader.biHeight = m_dwHeight;
	pBmi->bmiHeader.biCompression = BI_RGB;
	pBmi->bmiHeader.biSizeImage = 0;
	pBmi->bmiHeader.biXPelsPerMeter = 0;
	pBmi->bmiHeader.biYPelsPerMeter = 0;
	pBmi->bmiHeader.biClrUsed = 0;
	pBmi->bmiHeader.biClrImportant = 0;
	pBmi->bmiHeader.biBitCount = 8;

	::GetPaletteEntries(
		(HPALETTE)pPalette->m_hObject,
		0,
		0x100,
		apePalette);

	// The colour table is written relative to the *current* RGBQUAD's red
	// byte: blue at -2, green at -1, reserved at +1 (the increments come
	// last).  Writing them after the increment - as this used to - put
	// three quarters of every entry into the next one.
	pColorRed = (BYTE*)pBmi + 0x2A;
	pPe = apePalette;
	do
	{
		pColorRed[0] = pPe->peRed;
		pColorRed[-2] = pPe->peBlue;
		pColorRed[-1] = pPe->peGreen;
		pColorRed[1] = pPe->peFlags;
		pPe++;
		pColorRed += 4;
	} while (pPe < apePalette + 0x100);

	hBitmap = (HBITMAP)::CreateDIBSection(
		hScreenDC,
		pBmi,
		DIB_RGB_COLORS,
		(void**)&m_dwUnk04,
		NULL,
		0);
	m_bitmap.Attach(hBitmap);

	::ReleaseDC(NULL, hScreenDC);
	::operator delete(pBmi);
}

// Trivial constructor for the 0xC0-byte span entries allocated by
// CSub406090::Sub_4192F0. The original calls this once per entry in a loop.

// hover.exe:0x419520
// FUNCTION: HOVER 0x419520
CSub419520::CSub419520()
{
}

// Allocate (or reallocate) the three span buffers owned by the render target.
// dwPalArg is the doc's CMerlinWorld world manager; the count of 0xC0-byte
// entries is taken from the DWORD at offset 0x58 of that object.

// hover.exe:0x4192F0
// FUNCTION: HOVER 0x4192f0
void CSub406090::Sub_4192F0(DWORD dwPalArg)
{
	DWORD dwCount;
	DWORD dwSize;
	CSub419520* pEntry;
	CSub419520* pAllocated;

	::operator delete(m_pUnk3C);
	::operator delete(m_pUnk40);
	::operator delete(m_pUnk44);

	dwSize = ((((DWORD*)dwPalArg)[0x58 / 4] + 0x64) << 4) * 3;
	m_pUnk3C = (DWORD*)::operator new(dwSize);

	m_pUnk40 = (DWORD*)::operator new(m_dwWidth * 4);

	dwCount = ((DWORD*)dwPalArg)[0x58 / 4] + 0x64;
	dwSize = (dwCount << 6) * 3;
	pAllocated = (CSub419520*)::operator new(dwSize);

	if (pAllocated != NULL)
	{
		long lCount;

		pEntry = pAllocated;
		lCount = (long)dwCount - 1;
		if (lCount >= 0)
		{
			do
			{
				// An explicit constructor call, not placement new: the
				// original has no null test around it.
				pEntry->CSub419520::CSub419520();
				pEntry = (CSub419520*)((BYTE*)pEntry + 0xC0);
				lCount--;
			} while (lCount >= 0);
		}
		m_pUnk44 = (DWORD*)pAllocated;
	}
	else
	{
		m_pUnk44 = NULL;
	}
}

// FUNCTION: CSub406C90::CSub406C90
// hover.exe:0x41B870
// FUNCTION: HOVER 0x41B870
CSub406C90::CSub406C90(CSub406090* pParent)
{
	m_pUnk18 = (DWORD*)operator new((pParent->m_dwHeight * 4 + 0x280) * 3 * 5);
	m_dwUnk00 = pParent->m_dwHeight;
	m_dwUnk04 = 0;
	m_dwUnk08 = 0;
	m_dwUnk0C = 0;
	m_dwUnk10 = 0;
	m_dwUnk14 = 0;
	m_dwUnk1C = (DWORD)m_pUnk18;
}

// Initialize the span-heap header chain in the m_pUnk18 arena: the header
// node at +0xA0 links the three sub-arenas (+0xCC, +0x16C, +0x20C) and the
// chain cursor m_dwUnk1C ends past the last one plus m_dwUnk00 word pairs.
// hover.exe:0x406C90
// FUNCTION: HOVER 0x406c90
void CSub406C90::Sub_406C90()
{
	DWORD* pBase = (DWORD*)m_pUnk18;
	DWORD* pHdr;
	DWORD* pCursor;

	m_dwUnk1C = (DWORD)pBase;
	m_dwUnk04 = (DWORD)pBase;
	m_dwUnk10 = 0;

	pCursor = pBase + 0xCC / 4;
	pHdr = pBase + 0xA0 / 4;

	m_dwUnk1C = (DWORD)pHdr;
	m_dwUnk0C = 0;
	m_dwUnk14 = 0;
	m_dwUnk08 = (DWORD)pHdr;

	m_dwUnk1C = (DWORD)pCursor;
	pHdr[0] = (DWORD)pCursor;

	pCursor += 0xA0 / 4;
	m_dwUnk1C = (DWORD)pCursor;
	pHdr[1] = (DWORD)pCursor;
	pHdr[0x28 / 4] = 0;

	pCursor += 0xA0 / 4;
	pHdr[0x14 / 4] = 0;
	m_dwUnk1C = (DWORD)pCursor;
	pHdr[0x10 / 4] = (DWORD)pCursor;

	pCursor = (DWORD*)((BYTE*)pCursor + m_dwUnk00 * 2);
	pHdr[0x18 / 4] = 0;
	m_dwUnk1C = (DWORD)pCursor;
	pHdr[2] = (DWORD)pHdr;
	pHdr[3] = (DWORD)(pHdr + 1);
}

/////////////////////////////////////////////////////////////////////////////
// Free helpers used by the renderer setup cluster.

// A 16.16 fixed-point dword whose integer part is read and written as a
// signed short straight out of memory (notes/perfect-match.md rule 46);
// hover.exe's dashboard gauges build their values this way.

typedef union tagBUMPFIXED
{
	DWORD dw;
	struct
	{
		WORD wFrac;		// +0 fraction (1/65536 units)
		short sInt;		// +2 signed integer part
	} s;
} BUMPFIXED;

// Remaps an 8-bpp bitmap's indices to the nearest entries in pPalette and
// overwrites the bitmap's color table with the palette entries.
// hover.exe:0x410CE0
// FUNCTION: HOVER 0x410ce0
BOOL Hover_sub_410CE0(BYTE* pCopy, BYTE* pBits, CPalette* pPalette)
{
	CPalette palUnused;
	LOGPALETTE* pLogPal;
	PALETTEENTRY* pEntries;
	RGBQUAD* pColors;
	BYTE aRemap[256];
	int nColors;
	int i;
	DWORD dwColor;

	if (((BITMAPINFOHEADER*)pCopy)->biBitCount != 8)
		return FALSE;
	if (((BITMAPINFOHEADER*)pCopy)->biCompression != 0)
		return FALSE;

	pLogPal = (LOGPALETTE*)::operator new(0x408);
	pLogPal->palVersion = 0x300;
	pLogPal->palNumEntries = 0x100;

	pEntries = pLogPal->palPalEntry;
	pPalette->GetPaletteEntries(0, 0x100, pEntries);

	nColors = (((BITMAPINFOHEADER*)pCopy)->biClrUsed != 0)
		? (int)((BITMAPINFOHEADER*)pCopy)->biClrUsed : 0x100;
	pColors = (RGBQUAD*)((BYTE*)pCopy + sizeof(BITMAPINFOHEADER));

	memset(aRemap, 0, sizeof(aRemap));

	for (i = 0; i < nColors; i++)
	{
		dwColor = RGB(pColors[i].rgbRed, pColors[i].rgbGreen,
			pColors[i].rgbBlue);
		aRemap[i] = (BYTE)Hover_sub_413730(dwColor, (BYTE*)pEntries);
	}

	{
		int nWidth = (int)((BITMAPINFOHEADER*)pCopy)->biWidth;
		int nHeight = (int)((BITMAPINFOHEADER*)pCopy)->biHeight;
		BYTE* p = pBits;
		int x;
		int y;

		for (x = 0; x < nWidth; x++)
		{
			for (y = 0; y < nHeight; y++)
			{
				*p = aRemap[*p];
				p++;
			}
		}
	}

	pPalette->GetPaletteEntries(0, 0x100, pEntries); // hover.exe:0x410E3F re-reads, not SetPaletteEntries

	for (i = 0; i < nColors; i++)
	{
		pColors[i].rgbReserved = 0;
		pColors[i].rgbRed = pEntries[i].peRed;
		pColors[i].rgbGreen = pEntries[i].peGreen;
		pColors[i].rgbBlue = pEntries[i].peBlue;
	}

	::operator delete(pLogPal);
	return TRUE;
}

// Transparent 8-bit blit between two bottom-up DIBs: copies the source
// rectangle out of the dashboard bitmap into the offscreen render target,
// skipping pixels that carry the transparent color index.  Both bitmaps are
// addressed by a linear index built from a stride of the width rounded up to
// an even number of bytes; every index is range-checked against the whole
// bitmap rather than against the rectangle, so a rectangle that falls outside
// silently drops pixels instead of overrunning the buffer.  Quirks of the
// original reproduced here: the row count comes from the *source* rectangle
// while the column count comes from the *destination* rectangle, which leaves
// nSrcRight and nDstBottom unused.
//
// Known codegen difference: the original keeps nDstRow in a register but
// spills nSrcRow to the stack, which costs it an eighth local slot and makes
// it evaluate both index expressions strictly left to right.  VC 2.2 gives
// both row bases a register (so it folds "row + x" into a LEA), and whenever
// it is forced to put a row base in memory it also reassociates and hoists
// "row + left" out of the inner loop, which the original never does.  It also
// re-tests the loop counter that the "nRows <= 0" guard above already proved
// positive.  See notes/perfect-match.md rule 13.
// hover.exe:0x411770
// FUNCTION: HOVER 0x411770
void Hover_sub_411770(int nSrcLeft, int nSrcTop, int nSrcRight,
	int nSrcBottom, int nSrcWidth, int nSrcHeight, int nDstLeft, int nDstTop,
	int nDstRight, int nDstBottom, int nDstWidth, int nDstHeight,
	BYTE* pSrcBits, BYTE* pDstBits, int nTransparent)
{
	int nSrcPitch;
	int nDstPitch;
	int nSrcSize;
	int nDstSize;
	int nRows;
	int nCols;
	int nSrcRow;
	int nDstRow;
	int nSrcIndex;
	int nDstIndex;
	int x;
	int y;
	BYTE bTransparent;
	BYTE bPixel;

	// Scan lines are padded to an even byte count, not to the usual DWORD.
	if (nSrcWidth % 2 != 0)
		nSrcPitch = nSrcWidth + 1;
	else
		nSrcPitch = nSrcWidth;

	if (nDstWidth % 2 != 0)
		nDstPitch = nDstWidth + 1;
	else
		nDstPitch = nDstWidth;

	nSrcSize = nSrcPitch * nSrcHeight;
	nDstSize = nDstPitch * nDstHeight;
	nRows = nSrcBottom - nSrcTop;
	nCols = nDstRight - nDstLeft;

	// -1 means "take the transparent index from the first source pixel".
	if (nTransparent == -1)
		bTransparent = pSrcBits[0];
	else
		bTransparent = (BYTE)nTransparent;

	if (nRows <= 0)
		return;

	// Both DIBs are bottom-up, so the top row of the rectangle sits at the
	// highest linear offset and the row bases walk backwards from there.
	nDstRow = (nDstHeight - nDstTop - 1) * nDstPitch;
	nSrcRow = (nSrcHeight - nSrcTop - 1) * nSrcPitch;

	for (y = nRows; y > 0; y--)
	{
		for (x = 0; x < nCols; x++)
		{
			nSrcIndex = nSrcLeft + x + nSrcRow;
			nDstIndex = nDstLeft + x + nDstRow;

			if (nSrcIndex < nSrcSize && nSrcIndex >= 0 &&
				nDstIndex < nDstSize && nDstIndex >= 0)
			{
				bPixel = pSrcBits[nSrcIndex];
				if (bPixel != bTransparent)
					pDstBits[nDstIndex] = bPixel;
			}
		}

		nDstRow -= nDstPitch;
		nSrcRow -= nSrcPitch;
	}
}

// MFC-style FindResource wrapper used by the dashboard bitmap loader.
// hover.exe:0x453352
// FUNCTION: HOVER 0x453352
HRSRC __stdcall Hover_sub_453352(HINSTANCE hInst, LPCSTR lpName,
	LPCSTR lpType)
{
	return ::FindResourceA(hInst, lpName, lpType);
}

/////////////////////////////////////////////////////////////////////////////
// CBumperView

// hover.exe:0x41E730
// FUNCTION: HOVER 0x41e730
CBumperView::CBumperView()
{
	// m_dwUnk00A4/m_dwUnk00A8 are zeroed by the original before the
	// embedded object constructors run (member initializer list)
	m_dwUnk00A4 = 0;
	m_dwUnk00A8 = 0;
	m_dwUnk003C = 0;
	m_dwUnk0048 = 0;
	m_pUnk00C8 = 0;
	m_pUnk00CC = 0;
	m_pUnk00D0 = 0;
	m_dwUnk0040 = 0;
	m_dwUnk0044 = 0;
	m_dwUnk0034 = 1;
	m_dwUnk00E4 = g_adwUnk_4C4CC8[theApp.m_dwUnk01C4];
	m_dwUnk0518 = 0;
	m_dwUnk051C = 0;
	m_dwUnk0520 = 0;
	m_dwUnk0524 = 0;
	m_dwUnk021C = 0;
	m_dwUnk0080 = 0;
	Hover_sub_41F540();
	m_dwUnk0540 = 0;
	m_dwUnk0544 = 0;
	m_dwUnk0084 = 0;
	m_dwUnk0064 = 0;
	m_dwUnk0068 = 0;
	m_dwUnk006C = 0;
	m_dwUnk0070 = 0;
	m_dwUnk0074 = 0;
	m_dwUnk0078 = 0;
	m_dwUnk0504 = 0;
	m_dwUnk0508 = 0;
	m_dwUnk050C = 0;
	m_dwUnk0510 = 0;
	m_dwUnk0514 = 0;
	m_dwUnk0500 = 2;
	m_dwUnk0060 = 0;
	m_dwUnk00AC = 4;
}

// Only the renderer's frame-ready event is closed by hand; the render
// contexts, the offscreen target and the worker thread are released by
// Sub_41F350 when the window is destroyed. Everything else CBumperView
// owns (the CDC, the offscreen target, the four gauges, the two dials, the
// two pens, the four brushes and the CView base) is destroyed by the
// compiler generated member/base destructor calls that follow this body.

// hover.exe:0x41E3F0
// FUNCTION: HOVER 0x41e3f0
CBumperView::~CBumperView()
{
	if (m_dwUnk0048 != 0)
		::CloseHandle((HANDLE)m_dwUnk0048);
}

// Creates the pens and brushes the dashboard, the message boxes and the
// map overlay draw with. Any MFC exception raised on the way is reported
// through Hover_sub_4262B0 (which aborts), so the function still returns
// TRUE on that path.

// hover.exe:0x41F1C0
// FUNCTION: HOVER 0x41f1c0
BOOL CBumperView::Sub_41F1C0()
{
	try
	{
		if (!m_pen04D0.CreatePen(PS_SOLID, 1, RGB(0, 0, 0)))
			return FALSE;
		if (!m_pen04D8.CreatePen(PS_SOLID, 1, RGB(255, 255, 0)))
			return FALSE;
		if (!m_brush04E8.CreateSolidBrush(RGB(255, 255, 0)))
			return FALSE;
		if (!m_brush04E0.CreateSolidBrush(RGB(0, 0, 0)))
			return FALSE;
		if (!m_brush04F0.CreateSolidBrush(RGB(0, 255, 114)))
			return FALSE;
		if (!m_brush04F8.CreateSolidBrush(RGB(255, 0, 0)))
			return FALSE;
	}
	catch (CException* e)
	{
		Hover_sub_4262B0(e);
	}
	return TRUE;
}

// The view's GDI objects have to exist before the window does, because the
// first WM_PAINT can arrive from inside ::CreateWindowEx.

// hover.exe:0x41F180
// FUNCTION: HOVER 0x41f180
BOOL CBumperView::Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName,
	DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID,
	CCreateContext* pContext)
{
	if (!Sub_41F1C0())
		return FALSE;
	return CWnd::Create(lpszClassName, lpszWindowName, dwStyle, rect,
		pParentWnd, nID, pContext);
}

// Realizes the game palette on the main frame again whenever the view
// becomes the active view, so that a palette-stealing window that had the
// focus does not leave the game with the wrong colours.

// hover.exe:0x40E310
// FUNCTION: HOVER 0x40e310
void CBumperView::OnActivateView(BOOL bActivate, CView* pActivateView,
	CView* pDeactiveView)
{
	CView::OnActivateView(bActivate, pActivateView, pDeactiveView);
	if (bActivate)
		((CMainFrame*)AfxGetApp()->m_pMainWnd)->Sub_426CF0();
}

// Per-frame render step: update the camera state from the human player,
// run the scene projection, set up the renderer context, clear the render
// target, draw the projected objects, dispatch the frame-state handler and
// present (or invalidate) the result.

// hover.exe:0x402250
// FUNCTION: HOVER 0x402250
void CBumperView::Sub_402250()
{
	CBumperDoc* pDoc;
	DWORD* pdwCam;
	DWORD* pdwDest;
	CSub422F50* pScene;
	DWORD adwCam[2];
	RECT rc;

	if (m_pUnk00D0 == NULL)
		return;

	pDoc = (CBumperDoc*)m_pDocument;

	EnterCriticalSection(&theApp.m_csUnk00D4);
	Hover_sub_406C60();

	m_dwUnk00DC = *(DWORD*)((BYTE*)&pDoc->m_humanPlayer + 0x88);
	pdwCam = pDoc->m_humanPlayer.Sub_406C40(adwCam);

	// The camera pair is written through a pointer local (rule 94).
	pdwDest = &m_dwUnk00D4;
	pdwDest[0] = pdwCam[0];
	pdwDest[1] = pdwCam[1];

	m_dwUnk00E0 = *(DWORD*)((BYTE*)&pDoc->m_humanPlayer + 0x8C) + 0x28;

	pScene = &m_obj0150;
	pScene->Sub_407160(pDoc);

	// Gauge/dashboard scaling derived from player +0x1A4 and the gauge
	// object's m_dwUnk1C. The original forms a signed integer quotient
	// from the word difference, adds 0x288, multiplies by the gauge extent,
	// and clamps the high word of the product to [0, m_dwUnk1C].
	{
		BUMPFIXED fx;
		BUMPFIXED fxOffset;
		BUMPFIXED fxArg;
		long nMax;
		long nValue;

		fxOffset.dw = 0x288;

		fx.s.wFrac = 0;
		fx.s.sInt = (short)((WORD)(*(DWORD*)((BYTE*)&pDoc->m_humanPlayer
			+ 0x1A4)) - (WORD)pDoc->m_dwUnk83C4);
		fx.dw = (DWORD)((long)fx.dw / (long)(short)pDoc->m_dwUnk83CC);

		nMax = (long)m_obj03A0.m_dwUnk1C;

		fxArg.dw = fx.dw;
		fxArg.dw += fxOffset.dw;
		HOVER_FIXMULWP(fxArg.dw, nMax);

		nValue = fxArg.s.sInt;
		if (nMax < nValue)
			nValue = nMax;
		if (nMax > nValue)
		{
			m_obj03A0.m_dwUnk24 = 0;
			if (nValue > 0)
				m_obj03A0.m_dwUnk24 = (DWORD)nValue;
		}
		else
			m_obj03A0.m_dwUnk24 = (DWORD)nMax;
	}

	LeaveCriticalSection(&theApp.m_csUnk00D4);
	EnterCriticalSection(&theApp.m_csUnk00EC);

	if (m_dwUnk021C != 0)
	{
		if (m_pUnk00CC != NULL)
			m_pUnk00CC->m_dwUnk60 = (DWORD)((BYTE*)pDoc + 0x418C);
		m_pUnk00C8->m_dwUnk60 = (DWORD)((BYTE*)pDoc + 0x18C);
	}
	else
	{
		if (m_pUnk00CC != NULL)
			m_pUnk00CC->m_dwUnk60 = (DWORD)((BYTE*)pDoc + 0x18C);
		m_pUnk00C8->m_dwUnk60 = (DWORD)((BYTE*)pDoc + 0x18C);
	}

	if (m_dwUnk0540 == 0)
	{
		m_pUnk00C8->Sub_406C00(m_dwUnk00D4, m_dwUnk00D8, m_dwUnk00E0,
			m_dwUnk00DC);
		m_pUnk00D0->Sub_406090(pDoc->m_dwUnk0188, m_pUnk00C8);

		if (theApp.m_dwUnk01D0 != 0 && m_pUnk00CC != NULL)
		{
			m_pUnk00CC->Sub_406C00(m_dwUnk00D4, m_dwUnk00D8,
				m_dwUnk00E0, (m_dwUnk00DC + 0x100) & 0x1FF);
			m_pUnk00D0->Sub_406090(pDoc->m_dwUnk0188, m_pUnk00CC);
		}

		m_obj00E8.Sub_405A10(this, pDoc);
	}

	rc.left = m_obj0150.m_dwUnk5C;
	rc.top = m_obj0150.m_dwUnk60;
	rc.right = m_obj0150.m_dwUnk64;
	rc.bottom = m_obj0150.m_dwUnk68;
	::PatBlt(m_dcUnk00B4.m_hDC, rc.left, rc.top, rc.right - rc.left,
		rc.bottom - rc.top, BLACKNESS);
	// hover.exe:0x4024e3 flushes the GDI batch here: everything after this
	// writes to the DIB bits directly, so the fill has to have landed.
	::GdiFlush();

	m_obj0150.Sub_4053A0(this, pDoc);

	switch (m_dwUnk0500)
	{
	case 1:
		Sub_40D340();
		break;
	case 2:
		{
			CString strA;
			CString strB;
			strA.LoadString(0xEF3F);
			strB.LoadString(0xEF53);
			Sub_40CCE0(strA, strB, 0, 0);
		}
		break;
	case 3:
		Sub_4283F0();
		break;
	}

	if (m_dwUnk0080 != 0 && m_dwUnk0084 != 0)
	{
		::InvalidateRect(m_hWnd, NULL, TRUE);
		m_dwUnk0080 = 0;
	}
	else
		Sub_407430();

	LeaveCriticalSection(&theApp.m_csUnk00EC);
	m_dwUnk052C = GetTickCount();
	m_dwUnk0530++;
}

/////////////////////////////////////////////////////////////////////////////
// CSub406090::Sub_406090 and its helper functions (stubs for now).

// Visibility test for one sprite: side-classify both endpoints of its
// camera-facing segment against the left/right view-edge lines
// (g_lineUnk_4A4498 / g_lineUnk_4A44E0).  The sprite is submitted when an
// endpoint sits past the left edge but inside the right one, or when the
// segment straddles the whole view wedge and the camera origin lies on the
// matching side of the segment.
// hover.exe:0x405B90
// FUNCTION: HOVER 0x405b90
int CSub406090::Sub_405B90(CMerlinLine* pSprite)
{
	DWORD dwY1 = pSprite->m_dwY1;
	int bMinA;
	int bMinB;
	int bMaxA;
	int bMaxB;
	int nSide;

	bMinA = ((int)g_lineUnk_4A4498.m_dwY1 - (int)dwY1) *
			(int)g_lineUnk_4A4498.m_dwDeltaX +
		((int)pSprite->m_dwX1 - (int)g_lineUnk_4A4498.m_dwX1) *
			(int)g_lineUnk_4A4498.m_dwDeltaY >= 0;
	bMinB = ((int)pSprite->m_dwX1 - (int)g_lineUnk_4A44E0.m_dwX1) *
			(int)g_lineUnk_4A44E0.m_dwDeltaY +
		((int)g_lineUnk_4A44E0.m_dwY1 - (int)dwY1) *
			(int)g_lineUnk_4A44E0.m_dwDeltaX >= 0;

	if (bMinA == 0 && bMinB != 0)
		return 1;

	bMaxA = ((int)g_lineUnk_4A4498.m_dwY1 - (int)pSprite->m_dwY2) *
			(int)g_lineUnk_4A4498.m_dwDeltaX +
		((int)pSprite->m_dwX2 - (int)g_lineUnk_4A4498.m_dwX1) *
			(int)g_lineUnk_4A4498.m_dwDeltaY >= 0;
	bMaxB = ((int)pSprite->m_dwX2 - (int)g_lineUnk_4A44E0.m_dwX1) *
			(int)g_lineUnk_4A44E0.m_dwDeltaY +
		((int)g_lineUnk_4A44E0.m_dwY1 - (int)pSprite->m_dwY2) *
			(int)g_lineUnk_4A44E0.m_dwDeltaX >= 0;

	if (bMaxA == 0 && bMaxB != 0)
		return 1;

	nSide = ((int)dwY1 - (int)g_lineUnk_4A4498.m_dwY1) *
		(int)pSprite->m_dwDeltaX;
	nSide += ((int)g_lineUnk_4A4498.m_dwX1 - (int)pSprite->m_dwX1) *
		(int)pSprite->m_dwDeltaY;

	if (nSide >= 0)
	{
		if (bMinA == 0 && bMinB == 0 && bMaxA != 0 && bMaxB != 0)
			return 1;
	}
	else
	{
		if (bMinA != 0 && bMinB != 0 && bMaxA == 0 && bMaxB == 0)
			return 1;
	}
	return 0;
}



// Submits one sprite's span pair to the column lists: compute the
// camera-relative angles of the facing segment's two endpoints with
// Hover_sub_4385F0, swap them when the sweep would take the long arc,
// scale the wrapped angles to screen columns by m_dwUnk30 and clamp the
// resulting column window to the screen. One SPANLINK node is linked into
// each edge column's list (m_pUnk40) from the g_pdwUnk_4A4550 buffer, then
// the sprite's span links are advanced past the consumed entries. Returns
// 1 when the nodes were submitted, 0 when the sprite is off-screen.

// hover.exe:0x405CE0
// FUNCTION: HOVER 0x405ce0
int CSub406090::Sub_405CE0(CMerlinLine* pA, CMerlinLine* pB, int nC,
	WORD wOrd)
{
	long lAng1;
	long lAng2;
	long lAng1M;
	long lTmp;
	long lStart;
	long lEnd;
	long lScaledStart;
	long lScaledEnd;
	int nStart;
	int nEnd;
	SPANLINK* pNode;
	CMerlinDynamic* pObj;
	CMerlinStatic* pFace;
	DWORD* pLink;
	CSub410F20* pCam;

	pCam = (CSub410F20*)g_pUnk_4A1D68;
	lAng1 = (long)Hover_sub_4385F0(
		(long)(pB->m_dwY1 - pCam->m_dwUnk4C),
		(long)(pB->m_dwX1 - pCam->m_dwUnk48));
	pCam = (CSub410F20*)g_pUnk_4A1D68;
	lAng2 = (long)Hover_sub_4385F0(
		(long)(pB->m_dwY2 - pCam->m_dwUnk4C),
		(long)(pB->m_dwX2 - pCam->m_dwUnk48));

	// Order the endpoints so the sweep from lAng1 to lAng2 takes the short
	// arc: skip the swap only when lAng2 already lies within the half
	// circle ahead of lAng1 or more than a half circle behind it.
	lAng1M = lAng1 & 0x1FFFFFF;
	if ((lAng2 <= lAng1M || lAng2 - 0x1000000 >= lAng1M) &&
		(lAng2 + 0x2000000 <= lAng1M || lAng2 + 0x1000000 >= lAng1M))
	{
		lTmp = lAng1;
		lAng1 = lAng2;
		lAng2 = lTmp;
	}

	// Wrap both angles to [-half circle, half circle] around the camera
	// yaw, then force the sweep forward across the circle boundary.
	lStart = (lAng1 - (long)m_dwUnk34) & 0x1FFFFFF;
	if (lStart > 0x1000000)
		lStart -= 0x2000000;
	lEnd = (lAng2 - (long)m_dwUnk34) & 0x1FFFFFF;
	if (lEnd > 0x1000000)
		lEnd -= 0x2000000;
	if (lEnd < lStart)
		lEnd += 0x2000000;

	// Scale the angles to screen columns (24.8 result) and derive the
	// inclusive column window with a three-column margin on each side.
	HOVER_FIXMULR(lScaledStart, lStart, m_dwUnk30);
	HOVER_FIXMULR(lScaledEnd, lEnd, m_dwUnk30);
	nStart = (int)(lScaledStart / 0x10000) - 3;
	nEnd = (int)(lScaledEnd / 0x10000) + 3;

	// Clamp the window to the screen; bail out when it falls off an edge
	// or collapses to a single column.
	if (nStart < 0)
		nStart = 0;
	if (nEnd < 0)
		return 0;
	if (nStart > (int)g_dwUnk_4A454C - 1)
	{
		nStart = (int)g_dwUnk_4A454C - 1;
		if (nEnd > (int)g_dwUnk_4A454C - 1)
			return 0;
	}
	if (nEnd < 0)
		nEnd = 0;
	if (nEnd > (int)g_dwUnk_4A454C - 1)
		nEnd = (int)g_dwUnk_4A454C - 1;
	if (nEnd == nStart)
		return 0;

	// Emit the opening node (m_dwUnk04 = 1: textured drawer) at the start
	// column and the closing node (m_dwUnk04 = 0: flat drawer) at the end
	// column.
	pNode = (SPANLINK*)g_pdwUnk_4A4550;
	pNode->m_wUnk00 = wOrd;
	pNode->m_dwUnk08 = nC;
	pNode->m_dwUnk04 = 1;
	if (nC != 0)
	{
		pNode->m_dwUnk10 = 0;
		pObj = (CMerlinDynamic*)pA;
	}
	else
		pNode->m_dwUnk10 = (DWORD)pB;
	pNode->m_dwUnk0C = (DWORD)pA;
	pLink = &m_pUnk40[nStart];
	pNode->m_pUnk14 = (SPANLINK*)*pLink;
	*pLink = (DWORD)pNode;

	pNode++;
	pNode->m_wUnk00 = wOrd;
	pNode->m_dwUnk04 = 0;
	pNode->m_dwUnk08 = nC;
	pNode->m_dwUnk0C = (DWORD)pA;
	if (nC != 0)
		pNode->m_dwUnk10 = 0;
	else
		pNode->m_dwUnk10 = (DWORD)pB;
	pLink = &m_pUnk40[nEnd];
	pNode->m_pUnk14 = (SPANLINK*)*pLink;
	*pLink = (DWORD)pNode;
	pNode++;
	g_pdwUnk_4A4550 = (DWORD*)pNode;

	// Advance the consumed animation frames to their next entries: the
	// sprite's single texture for nC != 0, the face's six texture slots
	// otherwise (CMerlinTexture::m_pUnk38 is the next frame).
	if (nC != 0)
	{
		if (pObj->m_pUnk50 != NULL && pObj->m_pUnk50->m_pUnk38 != NULL)
			pObj->m_pUnk50 = pObj->m_pUnk50->m_pUnk38;
	}
	else
	{
		pFace = (CMerlinStatic*)pA;
		if (pFace->m_pUnk8C != NULL && pFace->m_pUnk8C->m_pUnk38 != NULL)
			pFace->m_pUnk8C = pFace->m_pUnk8C->m_pUnk38;
		if (pFace->m_pUnk90 != NULL && pFace->m_pUnk90->m_pUnk38 != NULL)
			pFace->m_pUnk90 = pFace->m_pUnk90->m_pUnk38;
		if (pFace->m_pUnk94 != NULL && pFace->m_pUnk94->m_pUnk38 != NULL)
			pFace->m_pUnk94 = pFace->m_pUnk94->m_pUnk38;
		if (pFace->m_pUnk98 != NULL && pFace->m_pUnk98->m_pUnk38 != NULL)
			pFace->m_pUnk98 = pFace->m_pUnk98->m_pUnk38;
		if (pFace->m_pUnk9C != NULL && pFace->m_pUnk9C->m_pUnk38 != NULL)
			pFace->m_pUnk9C = pFace->m_pUnk9C->m_pUnk38;
		if (pFace->m_pUnkA0 != NULL && pFace->m_pUnkA0->m_pUnk38 != NULL)
			pFace->m_pUnkA0 = pFace->m_pUnkA0->m_pUnk38;
	}
	return 1;
}

// Walk the maze BSP tree (m_pUnk20->m_arrUnk50, root index 0) back-to-front
// from the camera (g_lineUnk_4A4498.m_dwX1 / g_lineUnk_4A44E0.m_dwY1): test
// which side of the node's facing segment the camera sits on, recurse into
// the far child, submit the node sprite if it survives the view test, then
// continue down the near child (tail recursion as a loop). g_wUnk_4A4528 is
// passed to Sub_405CE0 as the span key and counts accepted submissions.
// hover.exe:0x405FA0
// FUNCTION: HOVER 0x405fa0
void CSub406090::Sub_405FA0(short nNode)
{
	CMerlinBSP* pSprite;
	CMerlinStatic* pPartner;
	int nVisible;
	WORD wOrd;

	if (nNode == -1)
		return;

	do
	{
		pSprite = m_pUnk20->GetNode(nNode);
		if (((long)g_lineUnk_4A4498.m_dwX1 - (long)pSprite->m_dwX1) *
				(long)pSprite->m_dwDeltaY +
			((long)pSprite->m_dwY1 - (long)g_lineUnk_4A44E0.m_dwY1) *
				(long)pSprite->m_dwDeltaX < 0)
		{
			Sub_405FA0(pSprite->m_wUnk48);
			nVisible = Sub_405B90(pSprite);
			wOrd = g_wUnk_4A4528;
			if (nVisible != 0)
			{
				pPartner = m_pUnk20->GetStatic((short)pSprite->m_wUnk44);
				if (Sub_405CE0(pPartner, pSprite, 0, wOrd) != 0)
				{
					g_wUnk_4A4528++;
				}
			}
			nNode = pSprite->m_wUnk4A;
		}
		else
		{
			Sub_405FA0(pSprite->m_wUnk4A);
			nVisible = Sub_405B90(pSprite);
			wOrd = g_wUnk_4A4528;
			if (nVisible != 0)
			{
				pPartner = m_pUnk20->GetStatic((short)pSprite->m_wUnk44);
				if (Sub_405CE0(pPartner, pSprite, 0, wOrd) != 0)
				{
					g_wUnk_4A4528++;
				}
			}
			nNode = pSprite->m_wUnk48;
		}
	} while (nNode != -1);
}

// Allocate one render object from the span-buffer arena at g_pdwUnk_4A4718
// (bumped by sizeof(RENDEROBJ)) and link it into the g_pUnk_4A4714 list.
// Wall segments (dwA != 0) are linked in behind the first wall object of the
// list (appended at the end when the list holds only floor/ceiling objects);
// floor/ceiling segments (dwA == 0) are inserted into the leading
// floor/ceiling run sorted by descending m_wUnk1C, ahead of any wall.
// hover.exe:0x4044A0
// FUNCTION: HOVER 0x4044a0
void Hover_sub_4044A0(DWORD dwA, DWORD dwB, WORD wC, DWORD dwD)
{
	RENDEROBJ* pNew;
	RENDEROBJ* pCur;
	RENDEROBJ* pNext;

	pNew = (RENDEROBJ*)g_pdwUnk_4A4718;
	pCur = g_pUnk_4A4714;
	pNew->m_wUnk1C = wC;
	pNew->m_dwUnk0C = dwA;
	pNew->m_dwUnk24 = dwD;
	pNew->m_dwUnk18 = 1;
	pNew->m_pUnk10 = (CMerlinLine*)dwB;
	pNext = NULL;

	if (dwA == 0)
	{
		if (pCur != NULL)
		{
			if ((short)pCur->m_wUnk1C >= (short)wC)
			{
				while (pCur->m_dwUnk0C == 0)
				{
					pNext = pCur->m_pUnk2C;
					if (pNext == NULL)
					{
						pCur->m_pUnk2C = pNew;
						pNew->m_pUnk28 = pCur;
						pNew->m_pUnk2C = NULL;
						goto L_done;
					}
					pCur = pNext;
					if ((short)pNext->m_wUnk1C < (short)wC)
						break;
				}
			}
			pNext = pCur->m_pUnk28;
			pCur->m_pUnk28 = pNew;
			if (pNext != NULL)
			{
				pNext->m_pUnk2C = pNew;
				pNew->m_pUnk28 = pNext;
				pNew->m_pUnk2C = pCur;
			}
			else
			{
				g_pUnk_4A4714 = pNew;
				pNew->m_pUnk2C = pCur;
				pNew->m_pUnk28 = pNext;
			}
			goto L_done;
		}
	}
	else
	{
		while (pCur != NULL && pCur->m_dwUnk0C == 0 &&
			pCur->m_pUnk2C != NULL)
			pCur = pCur->m_pUnk2C;
		if (pCur != NULL)
		{
			pNext = pCur->m_pUnk2C;
			pCur->m_pUnk2C = pNew;
			if (pNext != NULL)
				pNext->m_pUnk28 = pNew;
			pNew->m_pUnk2C = pNext;
			pNew->m_pUnk28 = pCur;
			goto L_done;
		}
	}
	pNew->m_pUnk2C = pNext;
	pNew->m_pUnk28 = pNext;
	g_pUnk_4A4714 = pNew;

L_done:
	g_pdwUnk_4A4718 = (DWORD*)(pNew + 1);
}

// Find the render object keyed by (dwA, dwB) and unlink it from the
// g_pUnk_4A4714 list; returns 1 if it was not yet visible.
// hover.exe:0x404580
// FUNCTION: HOVER 0x404580
DWORD Hover_sub_404580(DWORD dwA, DWORD dwB)
{
	RENDEROBJ* pScan;
	RENDEROBJ* pHead;
	RENDEROBJ* pPrev;
	RENDEROBJ* pNext;
	DWORD dwRet;

	dwRet = 0;
	pHead = g_pUnk_4A4714;
	pScan = pHead;
	while (pScan != NULL)
	{
		if ((DWORD)pScan->m_pUnk10 == dwA && pScan->m_dwUnk24 == dwB)
			break;
		pScan = pScan->m_pUnk2C;
	}
	if (pScan != NULL)
	{
		if (pScan->m_dwUnk18 == 0)
			dwRet = 1;
		pPrev = pScan->m_pUnk28;
		pNext = pScan->m_pUnk2C;
		if (pPrev != NULL)
			pPrev->m_pUnk2C = pNext;
		if (pNext != NULL)
			pNext->m_pUnk28 = pPrev;
		if (pScan == pHead)
			g_pUnk_4A4714 = pNext;
	}
	return dwRet;
}

// Turn the render objects collected for the current column bundle into
// spans against one phase camera line. The g_pUnk_4A4714 list is walked
// twice: first over its leading floor/ceiling run (m_dwUnk0C == 0), then
// over the wall objects behind it.
//
// Floor/ceiling pass: objects whose height range [m_wUnkA4, m_wUnkA6] is
// already inside the covered range are marked done (m_dwUnk18/m_dwUnk20)
// and skipped; the rest are intersected with pLine. A crossing yields the
// camera distance (m_wUnk08) and the texture u coordinate (m_wUnk0A,
// biased by the face's u origin at m_wUnkB8/m_wUnkBA), and, for a face
// that is not flagged in m_dwUnkB0/m_dwUnkB4, grows the covered height
// range [sLo, sHi] (seeded from dwSeed). The objects are appended to the
// g_pUnk_4A471C list in list order.
//
// Wall pass: the same intersection, but the face select can be flipped by
// the object's m_dwUnk68, and each object is inserted into the
// g_pUnk_4A471C list sorted by ascending m_wUnk08 (invisible objects go
// to the end of the run).
//
// The return value is always 1 (the caller ORs it into m_dwUnk28 to force
// a span rebuild).
// hover.exe:0x4045E0
// FUNCTION: HOVER 0x4045e0
DWORD Hover_sub_4045E0(DWORD dwSeed, CMerlinLine* pLine)
{
	DWORD adwHit[2];
	DWORD bSel;
	DWORD bFace;
	RENDEROBJ* pPrev;
	CMerlinDynamic* pSrcB;
	CMerlinStatic* pSrcA;
	short sLo;
	short sHi;

	RENDEROBJ* pObj;
	RENDEROBJ* pScan;
	RENDEROBJ* pBefore;
	DWORD* pdwVis;
	WORD wRet;
	WORD wOrg;
	WORD wU;
	int bGrow;

	sHi = (short)dwSeed;
	sLo = (short)dwSeed;
	pPrev = NULL;
	pObj = g_pUnk_4A4714;
	g_pUnk_4A471C = NULL;

	while (pObj != NULL && pObj->m_dwUnk0C == 0)
	{
		pSrcA = (CMerlinStatic*)pObj->m_pUnk10;
		pObj->m_dwUnk20 = 0;
		if ((short)pSrcA->m_wUnkA6 <= sHi &&
			(short)pSrcA->m_wUnkA4 >= sLo)
		{
			pObj->m_dwUnk18 = 1;
			pObj->m_dwUnk20 = 1;
		}
		else
		{
			wRet = pSrcA->Sub_401260(pLine, adwHit);
			pObj->m_dwUnk14 = (wRet == 1);
			if (g_pUnk_4A471C == NULL)
				g_pUnk_4A471C = pObj;
			if (wRet != 0)
			{
				pObj->m_wUnk08 = Hover_Distance(pLine->m_dwX1,
					pLine->m_dwY1, adwHit[0], adwHit[1]);
				if (pObj->m_dwUnk14 != 0)
				{
					wU = Hover_Distance(pSrcA->m_dwX2,
						pSrcA->m_dwY2, adwHit[0], adwHit[1]);
					wOrg = pSrcA->m_wUnkB8;
				}
				else
				{
					wU = Hover_Distance(pSrcA->m_dwX1,
						pSrcA->m_dwY1, adwHit[0], adwHit[1]);
					wOrg = pSrcA->m_wUnkBA;
				}
				// The raw distance is stored first and immediately
				// overwritten by the biased one; the original does
				// the same (dead store).
				pObj->m_wUnk0A = wU;
				pObj->m_wUnk0A = wOrg + wU;

				if (pObj->m_dwUnk14 != 0)
					bGrow = (pSrcA->m_dwUnkB0 == 0);
				else
					bGrow = (pSrcA->m_dwUnkB4 == 0);
				if (bGrow != 0)
				{
					if (sHi >= (short)pSrcA->m_wUnkA4 &&
						sHi < (short)pSrcA->m_wUnkA6)
						sHi = (short)pSrcA->m_wUnkA6;
					if (sLo <= (short)pSrcA->m_wUnkA6 &&
						sLo > (short)pSrcA->m_wUnkA4)
						sLo = (short)pSrcA->m_wUnkA4;
				}
				pObj->m_dwUnk18 = 0;
			}
			else
			{
				pObj->m_dwUnk18 = 1;
			}

			if (pPrev != NULL)
				pPrev->m_pUnk34 = pObj;
			pObj->m_pUnk34 = NULL;
			pObj->m_pUnk30 = pPrev;
			pPrev = pObj;
		}
		pObj = pObj->m_pUnk2C;
	}

	while (pObj != NULL)
	{
		pSrcB = (CMerlinDynamic*)pObj->m_pUnk10;
		wRet = pSrcB->Sub_401260(pLine, adwHit);
		pObj->m_dwUnk20 = 0;
		bFace = (wRet == 1);
		pObj->m_dwUnk14 = bFace;
		bSel = bFace;
		if (wRet != 0)
		{
			pObj->m_wUnk08 = Hover_Distance(pLine->m_dwX1,
				pLine->m_dwY1, adwHit[0], adwHit[1]);
			if (pSrcB->m_dwUnk68 != 0)
				bSel = (bFace == 0);
			if (bSel != 0)
			{
				pObj->m_wUnk0A = Hover_Distance(pSrcB->m_dwX2,
					pSrcB->m_dwY2, adwHit[0], adwHit[1]);
			}
			else
			{
				pObj->m_wUnk0A = Hover_Distance(pSrcB->m_dwX1,
					pSrcB->m_dwY1, adwHit[0], adwHit[1]);
			}
			pdwVis = &pObj->m_dwUnk18;
			*pdwVis = 0;
		}
		else
		{
			pdwVis = &pObj->m_dwUnk18;
			*pdwVis = 1;
		}

		pScan = g_pUnk_4A471C;
		if (pScan == NULL)
		{
			pObj->m_pUnk34 = NULL;
			pObj->m_pUnk30 = NULL;
			g_pUnk_4A471C = pObj;
		}
		else
		{
			for (;;)
			{
				if (pScan->m_dwUnk18 == 0 &&
					(short)pScan->m_wUnk08 > (short)pObj->m_wUnk08)
					break;
				if (*pdwVis != 0)
					break;
				if (pScan->m_pUnk34 == NULL)
				{
					pScan->m_pUnk34 = pObj;
					pObj->m_pUnk30 = pScan;
					pObj->m_pUnk34 = NULL;
					goto L_next;
				}
				pScan = pScan->m_pUnk34;
			}
			pBefore = pScan->m_pUnk30;
			pScan->m_pUnk30 = pObj;
			if (pBefore != NULL)
				pBefore->m_pUnk34 = pObj;
			else
				g_pUnk_4A471C = pObj;
			pObj->m_pUnk34 = pScan;
			pObj->m_pUnk30 = pBefore;
		}

L_next:
		pObj = pObj->m_pUnk2C;
	}
	return 1;
}

// Second and later passes of the phase-line walk (the first pass is
// Hover_sub_4045E0): re-intersect every render object still on the
// g_pUnk_4A471C list with the new phase line, refresh its two depth keys
// and move it back into depth order. m_wUnk08 is the distance from the
// camera to the crossing point, m_wUnk0A the distance from the crossing
// point to the wall segment's own endpoint (which endpoint depends on the
// side the line crosses from, inverted again for m_dwUnk68 sources).
// Wall objects (m_dwUnk0C != 0) are re-sorted by m_wUnk08: an object that
// became visible is unlinked and re-inserted from the head, an already
// linked one is only walked backwards to its new place. Floor/ceiling
// objects instead bias m_wUnk0A by the source's per-face bias and then
// bubble any wall predecessor that now sorts behind them past themselves.
// Returns 1 when the list order changed (the caller rebuilds the span
// lists); an object that no longer meets the phase line at all restarts
// the whole phase through Hover_sub_4045E0.
// hover.exe:0x406D30
// FUNCTION: HOVER 0x406d30
DWORD Hover_sub_406D30(DWORD dwZ, CMerlinLine* pLine)
{
	DWORD adwPt[2];
	CMerlinLine* pSrc;
	DWORD dwRet;
	DWORD dwFace;
	RENDEROBJ* pNext;
	DWORD dwFaceB;
	RENDEROBJ* pObj;
	RENDEROBJ* pScan;
	RENDEROBJ* pPrev;
	RENDEROBJ* pLink;
	DWORD dwMoved;
	WORD wRes;

	dwRet = 0;
	pObj = g_pUnk_4A471C;
	while (pObj != NULL)
	{
		pNext = pObj->m_pUnk34;
		pSrc = pObj->m_pUnk10;
		wRes = pSrc->Sub_401260(pLine, adwPt);
		dwFace = (wRes == 1);
		pObj->m_dwUnk14 = dwFace;
		dwFaceB = dwFace;

		if (wRes != 0)
		{
			pObj->m_wUnk08 = Hover_Distance(pLine->m_dwX1, pLine->m_dwY1,
				adwPt[0], adwPt[1]);

			// A sprite flagged at +0x68 keys off the opposite endpoint.
			if (pObj->m_dwUnk0C != 0 &&
				((CMerlinDynamic*)pSrc)->m_dwUnk68 != 0)
				dwFaceB = (dwFace == 0);

			if (dwFaceB != 0)
				pObj->m_wUnk0A = Hover_Distance(pSrc->m_dwX2,
					pSrc->m_dwY2, adwPt[0], adwPt[1]);
			else
				pObj->m_wUnk0A = Hover_Distance(pSrc->m_dwX1,
					pSrc->m_dwY1, adwPt[0], adwPt[1]);

			if (pObj->m_dwUnk0C != 0)
			{
				if (pObj->m_dwUnk18 != 0)
				{
					// Newly visible: unlink and re-insert from the head.
					dwRet = 1;
					pObj->m_dwUnk18 = 0;
					pPrev = pObj->m_pUnk30;
					if (pPrev != NULL)
						pPrev->m_pUnk34 = pObj->m_pUnk34;
					pLink = pObj->m_pUnk34;
					if (pLink != NULL)
						pLink->m_pUnk30 = pPrev;
					if (g_pUnk_4A471C == pObj)
						g_pUnk_4A471C = pLink;

					pScan = g_pUnk_4A471C;
					if (pScan == NULL)
					{
						pObj->m_pUnk34 = NULL;
						pObj->m_pUnk30 = NULL;
						g_pUnk_4A471C = pObj;
					}
					else
					{
						while (pScan->m_dwUnk18 != 0 ||
							(short)pScan->m_wUnk08 <=
								(short)pObj->m_wUnk08)
						{
							pLink = pScan->m_pUnk34;
							if (pLink == NULL)
							{
								pScan->m_pUnk34 = pObj;
								pObj->m_pUnk30 = pScan;
								pObj->m_pUnk34 = NULL;
								goto L_next;
							}
							pScan = pLink;
						}
						pPrev = pScan->m_pUnk30;
						pScan->m_pUnk30 = pObj;
						if (pPrev != NULL)
						{
							pPrev->m_pUnk34 = pObj;
							pObj->m_pUnk30 = pPrev;
							pObj->m_pUnk34 = pScan;
						}
						else
						{
							g_pUnk_4A471C = pObj;
							pObj->m_pUnk34 = pScan;
							pObj->m_pUnk30 = pPrev;
						}
					}
				}
				else
				{
					// Already linked: walk back to the new position.
					pPrev = pObj->m_pUnk30;
					if (pPrev == NULL)
						goto L_next;
					// Reloaded rather than copied from pPrev: the
					// original's walk keeps its own null test.
					pScan = pObj->m_pUnk30;
					while (pScan != NULL && (pScan->m_dwUnk18 != 0 ||
						(short)pScan->m_wUnk08 >= (short)pObj->m_wUnk08))
						pScan = pScan->m_pUnk30;
					if (pScan == pPrev)
						goto L_next;

					dwRet = 1;
					if (pPrev != NULL)
						pPrev->m_pUnk34 = pObj->m_pUnk34;
					pLink = pObj->m_pUnk34;
					if (pLink != NULL)
						pLink->m_pUnk30 = pPrev;
					if (g_pUnk_4A471C == pObj)
						g_pUnk_4A471C = pLink;

					if (pScan != NULL)
					{
						pLink = pScan->m_pUnk34;
						pScan->m_pUnk34 = pObj;
						if (pLink != NULL)
							pLink->m_pUnk30 = pObj;
						pObj->m_pUnk34 = pLink;
						pObj->m_pUnk30 = pScan;
					}
					else
					{
						pObj->m_pUnk30 = NULL;
						pLink = g_pUnk_4A471C;
						pObj->m_pUnk34 = pLink;
						if (pLink != NULL)
							pLink->m_pUnk30 = pObj;
						g_pUnk_4A471C = pObj;
					}
				}
			}
			else
			{
				if (pObj->m_dwUnk18 != 0)
				{
					dwRet = 1;
					pObj->m_dwUnk18 = 0;
				}

				if (pObj->m_dwUnk14 != 0)
					pObj->m_wUnk0A += ((CMerlinStatic*)pSrc)->m_wUnkB8;
				else
					pObj->m_wUnk0A += ((CMerlinStatic*)pSrc)->m_wUnkBA;

				// Pull every wall predecessor that now sorts behind this
				// floor/ceiling object past it, one at a time.
				do
				{
					dwMoved = 0;
					pPrev = pObj->m_pUnk30;
					if (pPrev != NULL && pPrev->m_dwUnk0C != 0 &&
						pPrev->m_dwUnk18 == 0 &&
						(short)pPrev->m_wUnk08 > (short)pObj->m_wUnk08)
					{
						dwRet = 1;
						pLink = pPrev->m_pUnk30;
						if (pLink != NULL)
							pLink->m_pUnk34 = pPrev->m_pUnk34;
						pScan = pPrev->m_pUnk34;
						if (pScan != NULL)
							pScan->m_pUnk30 = pLink;
						if (g_pUnk_4A471C == pPrev)
							g_pUnk_4A471C = pScan;

						pScan = pObj->m_pUnk34;
						pObj->m_pUnk34 = pPrev;
						if (pScan != NULL)
							pScan->m_pUnk30 = pPrev;
						pPrev->m_pUnk34 = pScan;
						pPrev->m_pUnk30 = pObj;
						dwMoved = 1;
					}
				} while (dwMoved != 0);
			}
		}
		else if (pObj->m_dwUnk18 == 0)
		{
			// The object dropped off the phase line: redo the whole phase.
			Hover_sub_4045E0(dwZ, pLine);
			return 1;
		}

L_next:
		pObj = pNext;
	}

	return dwRet;
}

// Rasterize one frame into the offscreen target: publish the render state
// to the renderer globals, rebuild every sprite's camera-facing segment,
// then walk the span columns phase by phase.
// hover.exe:0x406090
// FUNCTION: HOVER 0x406090
void CSub406090::Sub_406090(DWORD dwUnk, CSub410F20* pCtx)
{
	CMerlinLine line;	// phase line: camera -> current column-bundle edge
	CSub410F20* pCam;
	CMerlinDynamic* pSprite;
	SPANLINK* pNode;
	DWORD* pCursor;
	long lWide;
	long lTall;
	DWORD dwMask;
	DWORD dwNegStep;
	DWORD dwTop;
	DWORD dwBottom;
	int nLogWide;
	int nLogTall;
	int bFirst;
	int i;

	m_pUnk20 = (CMerlinWorld*)dwUnk;
	g_pUnk_4A1D68 = (CSub403850*)pCtx;

	lWide = (long)pCtx->m_dwUnk40;
	lTall = (long)pCtx->m_dwUnk3C;
	g_pbUnk_4A4548 = pCtx->m_pUnk14;
	g_dwUnk_4A453C = lWide;
	g_dwUnk_4A455C = lTall;
	g_dwUnk_4A1D70 = pCtx->m_dwUnk2C;
	g_pdwUnk_4A452C = pCtx->m_pUnk5C;
	g_dwUnk_4A454C = pCtx->m_dwUnk20;
	g_dwUnk_4A4544 = pCtx->m_dwUnk50;

	lWide >>= 1;
	nLogWide = 0;
	while (lWide != 0)
	{
		lWide >>= 1;
		nLogWide++;
	}

	g_dwUnk_4A455C = lTall;

	lTall >>= 1;
	nLogTall = 0;
	while (lTall != 0)
	{
		lTall >>= 1;
		nLogTall++;
	}

	g_dwUnk_4A4538 = nLogWide;
	g_dwUnk_4A4554 = nLogTall;

	g_dwUnk_4A1D78 = m_dwUnk24;
	g_dwUnk_4A4A34 = m_dwNegWidth;
	g_dwUnk_4A4A30 = pCtx->m_dwUnk7C;

	m_pUnk00->Sub_406C90();

	// The renderer's camera context is cached in a register across the
	// whole setup block and re-read after every call, exactly as the
	// original does (ebx at hover.exe:0x406161, reloaded at 0x40620D).
	pCam = (CSub410F20*)g_pUnk_4A1D68;

	m_dwUnk30 = ((int)g_dwUnk_4A454C << 16) / 0x55;
	m_dwUnk34 = (pCam->m_dwUnk54 << 16) - 0x2AAAAB;
	m_dwUnk34 &= 0x1FFFFFF;
	memset(m_pUnk40, 0, g_dwUnk_4A454C * 4);

	line.m_dwX1 = pCam->m_dwUnk48;
	line.m_dwY1 = pCam->m_dwUnk4C;
	pCursor = pCam->m_pUnk64 +
		(((int)pCam->m_dwUnk88 *
			((int)pCam->m_dwUnk54 & 0x1FF) * 6) /
			512) * 2;

	g_lineUnk_4A4498.m_dwX1 = pCam->m_dwUnk48;
	g_lineUnk_4A4498.m_dwY1 = pCam->m_dwUnk4C;
	g_lineUnk_4A4498.m_dwX2 = pCam->m_dwUnk48 +
		pCursor[0];
	g_lineUnk_4A4498.m_dwY2 = pCam->m_dwUnk4C +
		pCursor[1];
	g_lineUnk_4A4498.Sub_4010C0();

	pCursor += g_dwUnk_4A454C * 2;
	pCam = (CSub410F20*)g_pUnk_4A1D68;
	g_lineUnk_4A44E0.m_dwX1 = pCam->m_dwUnk48;
	g_lineUnk_4A44E0.m_dwY1 = pCam->m_dwUnk4C;
	g_lineUnk_4A44E0.m_dwX2 = pCam->m_dwUnk48 +
		pCursor[0];
	g_lineUnk_4A44E0.m_dwY2 = pCursor[1] +
		pCam->m_dwUnk4C;
	g_lineUnk_4A44E0.Sub_4010C0();
	pCursor -= g_dwUnk_4A454C * 2;

	g_pdwUnk_4A4550 = m_pUnk3C;

	// Rebuild every sprite's camera-facing segment (the perpendicular of
	// the view direction at the sprite's position, scaled by its size) and
	// submit the visible ones.
	for (i = m_pUnk20->m_arrUnk28.GetSize() - 1; i >= 0; i--)
	{
		pSprite = (CMerlinDynamic*)m_pUnk20->m_arrUnk28.GetAt(i);
		if (pSprite->m_dwUnk64 != 0)
		{
			int nDX;
			int nDY;
			int dx;
			int dy;
			int nScale;
			double fDist;

			dx = (short)pSprite->m_wUnk5A;
			dy = (short)pSprite->m_wUnk5C;
			pCam = (CSub410F20*)g_pUnk_4A1D68;
			nScale = (short)pSprite->m_wUnk58;
			dx -= (int)pCam->m_dwUnk48;
			dy -= (int)pCam->m_dwUnk4C;
			fDist = sqrt(fabs((double)(dy * dy + dx * dx)));

			nDX = (int)(dx * nScale / fDist);
			nDY = (int)(dy * nScale / fDist);
			pCam = (CSub410F20*)g_pUnk_4A1D68;
			dx += (int)pCam->m_dwUnk48;
			dy += (int)pCam->m_dwUnk4C;
			pSprite->m_dwX1 = dx + nDY;
			pSprite->m_dwY1 = dy - nDX;
			pSprite->m_dwX2 = dx - nDY;
			pSprite->m_dwY2 = nDX + dy;
			pSprite->Sub_4010C0();
		}
		if (Sub_405B90(pSprite) != 0)
			Sub_405CE0(pSprite, pSprite, 1, 0);
	}

	g_wUnk_4A4528 = 0;
	Sub_405FA0(0);

	g_pdwUnk_4A4718 = m_pUnk44;
	g_pUnk_4A4714 = NULL;
	g_pUnk_4A471C = NULL;
	bFirst = 1;
	m_dwUnk28 = 0;
	dwMask = g_dwUnk_4A455C - 1;

	for (g_dwUnk_4A4534 = 0; (int)g_dwUnk_4A4534 < (int)g_dwUnk_4A454C;
		g_dwUnk_4A4534++)
	{
		if ((g_dwUnk_4A4534 & dwMask) == 0)
		{
			line.m_dwX2 = ((CSub410F20*)g_pUnk_4A1D68)->m_dwUnk48 +
				pCursor[0];
			line.m_dwY2 = ((CSub410F20*)g_pUnk_4A1D68)->m_dwUnk4C +
				pCursor[1];
			pCursor += g_dwUnk_4A455C * 2;
			line.Sub_4010C0();
		}

		pNode = (SPANLINK*)m_pUnk40[g_dwUnk_4A4534];
		if (pNode == NULL)
		{
			if ((g_dwUnk_4A4534 & dwMask) == 0)
			{
				if (bFirst != 0)
				{
					m_dwUnk28 |= Hover_sub_4045E0(g_dwUnk_4A4544, &line);
					bFirst = 0;
				}
				else
				{
					m_dwUnk28 |= Hover_sub_406D30(g_dwUnk_4A4544, &line);
				}
			}
		}
		else
		{
			while (pNode != NULL)
			{
				if (pNode->m_dwUnk04 != 0)
					Hover_sub_4044A0(pNode->m_dwUnk08, pNode->m_dwUnk0C,
						pNode->m_wUnk00, pNode->m_dwUnk10);
				else
					m_dwUnk28 |= Hover_sub_404580(pNode->m_dwUnk0C,
						pNode->m_dwUnk10);
				pNode = pNode->m_pUnk14;
			}
			if ((g_dwUnk_4A4534 & dwMask) == 0)
			{
				bFirst = 0;
				m_dwUnk28 |= Hover_sub_4045E0(g_dwUnk_4A4544, &line);
			}
			else
			{
				bFirst = 1;
			}
		}

		if ((g_dwUnk_4A4534 & dwMask) == 0)
		{
			dwNegStep = 0 - g_dwUnk_4A453C;
			dwTop = ((CSub410F20*)g_pUnk_4A1D68)->m_pUnk34[
				g_dwUnk_4A4534] & dwNegStep;
			g_dwUnk_4A1D64 = dwTop;
			dwBottom = ((CSub410F20*)g_pUnk_4A1D68)->m_pUnk38[
				g_dwUnk_4A4534] & dwNegStep;
			g_dwUnk_4A4568 = g_dwUnk_4A1D64 - g_dwUnk_4A453C - 1;
			g_dwUnk_4A4530 = g_dwUnk_4A453C + dwBottom + 1;
			g_dwUnk_4A1D6C = ((CSub410F20*)g_pUnk_4A1D68)->m_pUnk30[
				g_dwUnk_4A4534] << 10;
			if (((CSub410F20*)g_pUnk_4A1D68)->m_dwUnk68 != 0)
				g_dwUnk_4A4540 = ((CSub410F20*)g_pUnk_4A1D68)->m_dwUnk28 -
					g_dwUnk_4A455C - g_dwUnk_4A4534 + g_dwUnk_4A454C - 1;
			else
				g_dwUnk_4A4540 = ((CSub410F20*)g_pUnk_4A1D68)->m_dwUnk28 +
					g_dwUnk_4A4534;
			g_dwUnk_4A456C = dwBottom;

			if (g_pUnk_4A471C != NULL)
			{
				((CSub402E00*)this)->Sub_402E00((DWORD)g_pUnk_4A471C);
			}
			else
			{
				g_dwUnk_4A456C = dwBottom;
				m_dwUnk28 |= 1;
				if ((int)(g_dwUnk_4A454C - 1) > (int)g_dwUnk_4A4534)
					((CSub403980*)m_pUnk00)->Sub_406D00(g_dwUnk_4A1D64,
						dwBottom);
				((CSub403980*)m_pUnk00)->Sub_403A60((CSub402E00*)this);
			}
		}
	}

	((CSub403980*)m_pUnk00)->Sub_403A60((CSub402E00*)this);
}

// Three-slot flavours of the pointer-destination fixed-point macros
// (hover.h's HOVER_FIXMULP / HOVER_FIXMULWP).  Every expansion of these two
// in hover.exe reserves *three* dwords, not two - the pointer, the second
// operand and one slot below them that the function never writes
// (notes/perfect-match.md rule 101).  In the present pass at
// hover.exe:0x407430 all three live in the function's 4-byte temporary pool,
// the second operand highest (ebp-0x9c), the pointer below it (ebp-0xa0) and
// the never-written slot lowest (ebp-0xa4), with the operand stored first -
// so the two macro locals are declared operand-then-pointer here.  The
// never-written slot has no source spelling that reproduces it exactly (an
// 8-byte aggregate lands in the aligned region instead of the pool), but the
// unreferenced `_dwPad` below is not free: deleting it costs this function
// 45.04% -> 44.01% at equal Accuracy, so it stays.  It is the sole source of
// the C4101 "unreferenced local variable" warning this file emits.

#define HOVER_FIXMULP3(dwA, dwB) \
	{ \
		DWORD _dwB; \
		DWORD* _pA; \
		_dwB = (dwB); \
		_pA = &(dwA); \
		_asm mov ecx, _pA \
		_asm mov eax, [ecx] \
		_asm imul _dwB \
		_asm shrd eax, edx, 0x10 \
		_asm mov [ecx], eax \
	}

#define HOVER_FIXMULWP3(dwA, wB) \
	{ \
		WORD _wB; \
		DWORD* _pA; \
		DWORD _dwPad; \
		_wB = (WORD)(wB); \
		_pA = &(dwA); \
		_asm mov ecx, _pA \
		_asm movsx eax, _wB \
		_asm imul dword ptr [ecx] \
		_asm mov [ecx], eax \
	}

// HOVER_ZONESIZE: the width/height pair the dashboard elements are drawn
// against.  hover.exe reads it out of the zone descriptor (and out of the
// render target) through an inline accessor that returns the pair *by
// value* and hands it to the element draw calls as one 8-byte argument:
// VC treats the return slot as address-taken memory, so every call site
// materialises the pair in an 8-byte stack temporary of its own
// (notes/perfect-match.md rule 118).

struct HOVER_ZONESIZE
{
	DWORD cx;
	DWORD cy;
};

inline HOVER_ZONESIZE Hover_ZoneSize(const DWORD* pdw)
{
	return *(const HOVER_ZONESIZE*)pdw;
}

// Present one rectangle of the offscreen render target into the client DC,
// doubling the destination geometry when the view runs at 2x. CDC::BitBlt
// and CDC::StretchBlt are inline in MFC 3.1 (AFXWIN1.INL), so each use
// expands to the GDI import plus an inlined CDC::GetSafeHdc on the source
// DC, and each expansion homes its computed arguments in stack temporaries
// of its own (notes/perfect-match.md rule 91).
#define HOVER_PRESENT_BLIT(dcDest, xDst, yDst, cxDst, cyDst)		\
	if (m_dwUnk0544 != 0)											\
		(dcDest).StretchBlt((xDst) * 2, (yDst) * 2, (cxDst) * 2,	\
			(cyDst) * 2, &m_dcUnk00B4, xDst, yDst, cxDst, cyDst,	\
			SRCCOPY);												\
	else															\
		(dcDest).BitBlt(xDst, yDst, cxDst, cyDst, &m_dcUnk00B4,		\
			xDst, yDst, SRCCOPY);

// Composite/present the offscreen render target into the view's client
// area. Faithful reconstruction of the full present pass at 0x407430:
// CClientDC, palette, optional background blits, dashboard element draw
// calls, gauge calls, the final overlay and the WM_COPYDATA path.
//
// Two shapes are load-bearing for the 0x224-byte frame: every blit is an
// inlined MFC CDC::StretchBlt/BitBlt (rule 91) whose computed arguments are
// homed in temporaries of its own, and the six dashboard element calls read
// the zone size through Hover_ZoneSize (rule 118).

// hover.exe:0x407430
// FUNCTION: HOVER 0x407430
void CBumperView::Sub_407430()
{
	DWORD dwFlag;
	CBumperDoc* pDoc;
	DWORD dwY;
	DWORD dwScale;
	BUMPFIXED fx;
	DWORD dwX;
	BUMPFIXED fxArg;
	DWORD dwFix;
	CPalette* pOldPal;
	ZONE_DESCRIPTOR* pZone;
	long nMax;
	long nValue;

	// Determine the present-path flag; theApp.m_dwUnk03E8 forces it to 1.
	dwFlag = g_dwUnk_4C4CA8;
	if (theApp.m_dwUnk03E8 != 0)
		dwFlag = 1;

	pDoc = (CBumperDoc*)m_pDocument;

	// Choose the zone descriptor. When the view is in 2x mode the base
	// descriptor (zone 0) is used; otherwise the current zone index selects
	// one of the six 0x338-byte blocks.
	pZone = (ZONE_DESCRIPTOR*)g_adwUnk_4609A0;
	if (m_dwUnk0544 == 0)
		pZone += g_dwUnk_460998;

	CClientDC dc(this);

	// Select the global palette into the client DC.
	pOldPal = dc.SelectPalette(&g_paletteUnk_4A1D50, TRUE);

	if (m_dwUnk0540 == 0)
	{
		// Early-out: the view is inactive.
		if (m_dwUnk0080 != 0)
		{
			dc.SelectPalette(pOldPal, TRUE);
			return;
		}

		// Background rect-1 from m_rcUnk0220.
		if (dwFlag == 0)
		{
			HOVER_PRESENT_BLIT(dc, m_rcUnk0220.left, m_rcUnk0220.top,
				m_rcUnk0220.Width(), m_rcUnk0220.Height());

			// Background rect-2 from m_rcUnk0230 when the alternate
			// renderer is active.
			if (theApp.m_dwUnk01D0 != 0)
			{
				HOVER_PRESENT_BLIT(dc, m_rcUnk0230.left, m_rcUnk0230.top,
					m_rcUnk0230.Width(), m_rcUnk0230.Height());
			}
		}

		// m_obj00E8 present helper: draw into the offscreen DC when dwFlag
		// is set, otherwise draw into the client DC.
		m_obj00E8.Sub_401E80((dwFlag != 0) ? &m_dcUnk00B4 : &dc, 0);

		// Dashboard CSub422C30 elements.
		if (m_obj0430.m_dwUnk00 != 0)
		{
			{
				HOVER_ZONESIZE size = Hover_ZoneSize(&pZone->m_dwWidth);

				m_obj0430.Sub_40E9A0(m_pUnk00D0->m_dwUnk04, size.cx, size.cy);
			}

			if (dwFlag == 0)
			{
				HOVER_PRESENT_BLIT(dc, pZone->m_rcUnk228.left,
					pZone->m_rcUnk228.top, 6 * pZone->m_dwUnk28,
					pZone->m_dwUnk2C);
			}
		}

		if (m_obj0480.m_dwUnk00 != 0)
		{
			{
				HOVER_ZONESIZE size = Hover_ZoneSize(&pZone->m_dwWidth);

				m_obj0480.Sub_40E9A0(m_pUnk00D0->m_dwUnk04, size.cx, size.cy);
			}

			if (dwFlag == 0)
			{
				HOVER_PRESENT_BLIT(dc, pZone->m_rcUnk230.left,
					pZone->m_rcUnk230.top, 6 * pZone->m_dwUnk28,
					pZone->m_dwUnk2C);
			}
		}

		// Dashboard CSub422BD0 elements. Each of the four waypoint markers
		// reads its origin and size into locals first: the original holds
		// all four in registers across the blit expansion instead of
		// re-reading the descriptor for the source rectangle.
		if (m_obj0240.m_dwUnk00 != 0)
		{
			{
				HOVER_ZONESIZE size = Hover_ZoneSize(&pZone->m_dwWidth);

				m_obj0240.Sub_40FCE0(m_pUnk00D0->m_dwUnk04, size.cx, size.cy);
			}

			if (dwFlag == 0)
			{
				DWORD x = pZone->m_adwWaypointMarkers[0];
				DWORD y = pZone->m_adwWaypointMarkers[1];
				DWORD cx = pZone->m_dwUnk20;
				DWORD cy = pZone->m_dwUnk24;

				HOVER_PRESENT_BLIT(dc, x, y, cx, cy);
			}
		}

		if (m_obj0268.m_dwUnk00 != 0)
		{
			{
				HOVER_ZONESIZE size = Hover_ZoneSize(&pZone->m_dwWidth);

				m_obj0268.Sub_40FCE0(m_pUnk00D0->m_dwUnk04, size.cx, size.cy);
			}

			if (dwFlag == 0)
			{
				DWORD x = pZone->m_adwWaypointMarkers[4];
				DWORD y = pZone->m_adwWaypointMarkers[5];
				DWORD cx = pZone->m_dwUnk20;
				DWORD cy = pZone->m_dwUnk24;

				HOVER_PRESENT_BLIT(dc, x, y, cx, cy);
			}
		}

		if (m_obj0290.m_dwUnk00 != 0)
		{
			{
				HOVER_ZONESIZE size = Hover_ZoneSize(&pZone->m_dwWidth);

				m_obj0290.Sub_40FCE0(m_pUnk00D0->m_dwUnk04, size.cx, size.cy);
			}

			if (dwFlag == 0)
			{
				DWORD x = pZone->m_adwWaypointMarkers[2];
				DWORD y = pZone->m_adwWaypointMarkers[3];
				DWORD cx = pZone->m_dwUnk20;
				DWORD cy = pZone->m_dwUnk24;

				HOVER_PRESENT_BLIT(dc, x, y, cx, cy);
			}
		}

		if (m_obj02B8.m_dwUnk00 != 0)
		{
			{
				HOVER_ZONESIZE size = Hover_ZoneSize(&pZone->m_dwWidth);

				m_obj02B8.Sub_40FCE0(m_pUnk00D0->m_dwUnk04, size.cx, size.cy);
			}

			if (dwFlag == 0)
			{
				HOVER_PRESENT_BLIT(dc, pZone->m_rcUnk220.left,
					pZone->m_rcUnk220.top, 7 * pZone->m_dwUnk18,
					pZone->m_dwUnk1C);
			}
		}

		// Speed gauge. |velocity| is built in 16.16 from the document's two
		// velocity components, divided by the ship's scaled top speed and
		// fed to the first gauge object; CSub422C80::Sub_42A9F0 (0x42A9F0)
		// and the CSub42AE90(double) constructor (0x42A9D0) are expanded
		// inline here.
		//
		// theApp.m_pUnk014C points to the active ship params block, whose
		// +0x20 field scales the document's top-speed reading.
		dwScale = *(DWORD*)((BYTE*)pDoc + 0x8364);
		HOVER_FIXMULP3(dwScale, *(DWORD*)((BYTE*)theApp.m_pUnk014C + 0x20));

		dwX = *(DWORD*)((BYTE*)pDoc + 0x8338);
		HOVER_FIXMULP3(dwX, dwX);
		dwY = *(DWORD*)((BYTE*)pDoc + 0x833C);
		HOVER_FIXMULP3(dwY, dwY);

		fx.dw = dwX;
		fx.dw = dwY + dwX;

		dwFix = (DWORD)(long)(sqrt((double)(fx.dw & 0xFFFF) * (1.0 / 65536.0)
			+ fx.s.sInt) * 65536.0);
		HOVER_FIXDIVLP(dwFix, dwScale);

		nMax = (long)m_obj02E0.m_dwUnk1C;
		fxArg.dw = dwFix;
		fxArg.dw += 0x288;
		HOVER_FIXMULWP3(fxArg.dw, nMax);

		nValue = fxArg.s.sInt;
		if (nValue > nMax)
			nValue = nMax;
		if (nValue >= nMax)
			m_obj02E0.m_dwUnk24 = (DWORD)nMax;
		else
		{
			m_obj02E0.m_dwUnk24 = 0;
			if (nValue > 0)
				m_obj02E0.m_dwUnk24 = (DWORD)nValue;
		}

		// Gauge calls. Each gauge is drawn into the offscreen DC (bUnk=1).
		// When dwFlag is clear and the gauge's value has changed it is also
		// drawn into the client DC (bUnk=0); the first gauge uses bUnk=1
		// even for the client draw in the original.
		CDC* pOffscreenDC = &m_dcUnk00B4;

		if (dwFlag == 0)
			m_obj02E0.Sub_401AD0(&dc, 1);
		m_obj02E0.Sub_401AD0(pOffscreenDC, 1);

		if (*(DWORD*)((BYTE*)pDoc + 0x82B0) != 0)
		{
			if (dwFlag == 0 && m_obj0310.m_dwUnk24 != m_obj0310.m_dwUnk28)
				m_obj0310.Sub_401AD0(&dc, 0);
			m_obj0310.Sub_401AD0(pOffscreenDC, 1);
		}

		if (*(DWORD*)((BYTE*)pDoc + 0x82B4) != 0)
		{
			if (dwFlag == 0 && m_obj0370.m_dwUnk24 != m_obj0370.m_dwUnk28)
				m_obj0370.Sub_401AD0(&dc, 0);
			m_obj0370.Sub_401AD0(pOffscreenDC, 1);
		}

		// hover.exe:0x408217 falls into this block when +0x82B8 is set and
		// skips it when +0x82B8 is clear *and* +0x82B4 is set, i.e. the
		// second test is inverted with respect to the first.
		if (*(DWORD*)((BYTE*)pDoc + 0x82B8) != 0 ||
			*(DWORD*)((BYTE*)pDoc + 0x82B4) == 0)
		{
			if (dwFlag == 0 && m_obj0340.m_dwUnk24 != m_obj0340.m_dwUnk28)
				m_obj0340.Sub_401AD0(&dc, 0);
			m_obj0340.Sub_401AD0(pOffscreenDC, 1);
		}

		if (dwFlag == 0 && m_obj03A0.m_dwUnk24 != m_obj03A0.m_dwUnk28)
			m_obj03A0.Sub_401AD0(&dc, 0);
		m_obj03A0.Sub_401AD0(pOffscreenDC, 1);

		if (*(DWORD*)((BYTE*)pDoc + 0x82C0) != 0)
		{
			if (dwFlag == 0 && m_obj03D0.m_dwUnk24 != m_obj03D0.m_dwUnk28)
				m_obj03D0.Sub_401AD0(&dc, 0);
			m_obj03D0.Sub_401AD0(pOffscreenDC, 1);
		}

		if (*(DWORD*)((BYTE*)pDoc + 0x82BC) != 0)
		{
			if (dwFlag == 0 && m_obj0400.m_dwUnk24 != m_obj0400.m_dwUnk28)
				m_obj0400.Sub_401AD0(&dc, 0);
			m_obj0400.Sub_401AD0(pOffscreenDC, 1);
		}

		// Flush the batched GDI calls that filled the offscreen DC before
		// any of it is presented.
		::GdiFlush();

		// Blit the projected scene into the client area unless the
		// WM_COPYDATA path will handle it.
		if (dwFlag == 0)
			m_obj0150.Sub_42D3E0(&dc);

		// Optional final overlay using the zone descriptor's base size.
		if (g_dwUnk_4C4CA8 != 0)
		{
			HOVER_PRESENT_BLIT(dc, 0, 0, pZone->m_dwWidth,
				pZone->m_dwHeight);
		}

		// WM_COPYDATA path (only when dwFlag != 0). The original calls
		// USER32's SendMessageA through a register (the import address is
		// loaded once into ebx), and it returns straight out of the blit
		// below without restoring the palette.
		if (dwFlag != 0)
		{
			COPYDATASTRUCT cds;
			LRESULT (WINAPI* pfnSendMessage)(HWND, UINT, WPARAM, LPARAM);
			HWND hwndTarget;
			void* lpBits;

			lpBits = (void*)m_pUnk00D0->m_dwUnk04;
			hwndTarget = (HWND)theApp.m_dwUnk03F0;
			cds.dwData = 0x401;
			cds.cbData = 0x12C00;
			if (lpBits == NULL)
				cds.cbData = 0;
			pfnSendMessage = ::SendMessageA;
			cds.lpData = lpBits;

			if (pfnSendMessage(hwndTarget, WM_COPYDATA, 0,
					(LPARAM)&cds) != 0)
			{
				if (theApp.m_dwUnk03F0 != 0)
				{
					if (theApp.m_dwUnk03E8 != 0)
					{
						pfnSendMessage((HWND)theApp.m_dwUnk03F0, 0x404, 0,
							0);
						theApp.m_dwUnk03E8 = 0;
					}
					pfnSendMessage((HWND)theApp.m_dwUnk03F0, 0x402, 0, 0);
					theApp.m_dwUnk03F0 = 0;
				}

				theApp.m_dwUnk03E4 = 0;
				theApp.m_dwUnk03E0 = 0;

				HOVER_PRESENT_BLIT(dc, 0, 0, pZone->m_dwWidth,
					pZone->m_dwHeight);
				return;
			}
		}
	}
	else
	{
		m_obj0150.Sub_42D3E0(&dc);
	}

	dc.SelectPalette(pOldPal, TRUE);
}

/////////////////////////////////////////////////////////////////////////////
// Worker-thread entry point handed to AfxBeginThread by OnInitialUpdate:
// raises the GDI batch limit for this thread and runs the view's render
// state machine until it is told to exit.

// hover.exe:0x41ECC0
// FUNCTION: HOVER 0x41ecc0
UINT AFX_CDECL Hover_sub_41ECC0(LPVOID pParam)
{
	GdiSetBatchLimit(1);
	((CBumperView*)pParam)->Sub_41ECE0();
	return 0;
}

// Initializes the renderer: GDI batch limit, sync events, worker thread,
// offscreen DC and render contexts, dashboard defaults and initial
// game-state transition. This is the view's CView::OnInitialUpdate
// override (slot 36 of hover.exe:0x4BB6D0), run by the framework right
// after the frame, the view and the document have been created; the
// original never calls it by hand.

// hover.exe:0x417E30
// FUNCTION: HOVER 0x417e30
void CBumperView::OnInitialUpdate()
{
	RECT rcClient;
	CBumperDoc* pDoc;
	LPPROGRESSFN pfnProgress;

	g_dwUnk_4C57CC = 0x50;
	g_dwUnk_4C57D0 = 0x64;

	m_dwUnk021C = 0;
	m_dwUnk0084 = 0;

	if (m_dwUnk0040 == 0)
	{
		GdiSetBatchLimit(1);

		m_dwUnk0048 = (DWORD)CreateEventA(NULL, TRUE, FALSE, NULL);
		m_hUnk004C = CreateEventA(NULL, TRUE, FALSE, NULL);
		m_hUnk0050 = CreateEventA(NULL, TRUE, FALSE, NULL);
		m_hUnk0054 = CreateEventA(NULL, TRUE, TRUE, NULL);
		m_hUnk0058 = CreateEventA(NULL, TRUE, FALSE, NULL);
		m_hUnk005C = CreateEventA(NULL, TRUE, FALSE, NULL);

		// The worker is an MFC CWinThread kept alive by the view (the
		// teardown in Sub_41F350 waits on its m_hThread and deletes it).
		m_dwUnk0040 = (DWORD)AfxBeginThread(Hover_sub_41ECC0, this);
		((CWinThread*)m_dwUnk0040)->m_bAutoDelete = FALSE;

		m_dcUnk00B4.Attach(::CreateCompatibleDC(NULL));
		m_dwUnk00C4 = 0;

		if (m_pUnk00C8 != NULL)
			delete m_pUnk00C8;
		if (m_pUnk00CC != NULL)
			delete m_pUnk00CC;

		m_pUnk00C8 = new CSub410F20();

		if (theApp.m_dwUnk01D0 != 0)
			m_pUnk00CC = new CSub410F20();
	}

	// Initialize the CSub422C30 dashboard elements: enabled, and the
	// "changed" flag clamped like the gauges below (rule 22).
	m_obj0430.m_dwUnk00 = 1;
	m_obj0430.m_dwUnk44 = m_obj0430.m_dwUnk3C;
	if ((long)m_obj0430.m_dwUnk44 > 0)
		m_obj0430.m_dwUnk44 = 0;

	m_obj0480.m_dwUnk00 = 1;
	m_obj0480.m_dwUnk44 = m_obj0480.m_dwUnk3C;
	if ((long)m_obj0480.m_dwUnk44 > 0)
		m_obj0480.m_dwUnk44 = 0;

	// Initialize the gauge current values to the clamped maximums.
	m_obj02E0.m_dwUnk24 = m_obj02E0.m_dwUnk1C;
	if ((long)m_obj02E0.m_dwUnk24 > 0)
		m_obj02E0.m_dwUnk24 = 0;
	m_obj0310.m_dwUnk24 = m_obj0310.m_dwUnk1C;
	if ((long)m_obj0310.m_dwUnk24 > 0)
		m_obj0310.m_dwUnk24 = 0;
	m_obj0340.m_dwUnk24 = m_obj0340.m_dwUnk1C;
	if ((long)m_obj0340.m_dwUnk24 > 0)
		m_obj0340.m_dwUnk24 = 0;
	m_obj0370.m_dwUnk24 = m_obj0370.m_dwUnk1C;
	if ((long)m_obj0370.m_dwUnk24 > 0)
		m_obj0370.m_dwUnk24 = 0;
	m_obj03A0.m_dwUnk24 = m_obj03A0.m_dwUnk1C;
	if ((long)m_obj03A0.m_dwUnk24 > 0)
		m_obj03A0.m_dwUnk24 = 0;
	m_obj03D0.m_dwUnk24 = m_obj03D0.m_dwUnk1C;
	if ((long)m_obj03D0.m_dwUnk24 > 0)
		m_obj03D0.m_dwUnk24 = 0;
	m_obj0400.m_dwUnk24 = m_obj0400.m_dwUnk1C;
	if ((long)m_obj0400.m_dwUnk24 > 0)
		m_obj0400.m_dwUnk24 = 0;

	pfnProgress = g_pfnUnk_4C4DAC;
	if (pfnProgress != NULL)
		pfnProgress(1, 0x0A);

	::GetClientRect(m_hWnd, &rcClient);
	Sub_4274B0(rcClient.right - rcClient.left,
		rcClient.bottom - rcClient.top, 1);

	pfnProgress = g_pfnUnk_4C4DAC;
	if (pfnProgress != NULL)
		pfnProgress(4, 0x0A);

	pDoc = (CBumperDoc*)m_pDocument;
	m_dwUnk0080 = 1;

	if (pDoc->m_dwUnk0184 != 0)
	{
		EnterCriticalSection(&theApp.m_csUnk00D4);
		Hover_sub_408780();
		LeaveCriticalSection(&theApp.m_csUnk00D4);

		pfnProgress = g_pfnUnk_4C4DAC;
		if (pfnProgress != NULL)
			pfnProgress(5, 0x0A);

		if (theApp.m_dwUnk01B8 == 3)
		{
			m_dwUnk0500 = 0;
			m_dwUnk00A0 = (DWORD)-1;
		}

		Sub_40DC20(1);

		pfnProgress = g_pfnUnk_4C4DAC;
		if (pfnProgress != NULL)
			pfnProgress(8, 0x0A);

		pfnProgress = g_pfnUnk_4C4DAC;
		if (theApp.m_pBumperDoc->m_dwUnk8498 == 0)
		{
			theApp.m_dwUnk01B8 = 2;
			m_dwUnk007C = 0x28;
			Sub_4126C0();
			pfnProgress = g_pfnUnk_4C4DAC;
		}
		else
		{
			theApp.m_dwUnk01B8 = 0;
			m_dwUnk007C = 0x3C;
		}
	}

	if (pfnProgress != NULL)
		pfnProgress(9, 0x0A);

	m_dwUnk0084 = 1;

	CView::OnInitialUpdate();

	pfnProgress = g_pfnUnk_4C4DAC;
	if (pfnProgress != NULL)
		pfnProgress(0x0A, 0x0A);

	((CMainFrame*)AfxGetApp()->m_pMainWnd)->Sub_426CF0();

	::RedrawWindow(m_hWnd, NULL, NULL, 0x105);
}

// Looks up the zone descriptor whose first two ints match (nWidth,nHeight),
// records the index in g_dwUnk_460998 and then sets up the render target,
// the two renderer contexts and the background sampling arrays.

// hover.exe:0x4184B0
// FUNCTION: HOVER 0x4184b0
void CBumperView::Sub_4184B0(int nWidth, int nHeight)
{
	CObject* pBackground;
	ZONE_DESCRIPTOR* pZone;
	int nZoneInts;
	int nIndex;
	int* pSearch;
	CBumperDoc* pDoc;
	DWORD dwColor;
	int nSub;
	int dwLeft;
	int dwMidX;
	int dwSpan;
	int dwTopY;
	int dwBottomY;
	DWORD dwCount;
	DWORD cbArray;
	DWORD* adwTop;
	DWORD* adwBottom;
	int i;
	POINT ptOrigin;
	CPalette* pOldPal;

	nZoneInts = 0x338 / 4;
	nIndex = 0;
	pSearch = g_adwUnk_4609A0;

	// Bottom-tested walk over the six descriptors: the loop falls straight
	// out into the "no such mode" return, the match jumps forward.
	do
	{
		if (pSearch[0] == nWidth && pSearch[1] == nHeight)
			goto Found;
		pSearch += nZoneInts;
		nIndex++;
	}
	while (pSearch <= g_adwUnk_4609A0 + 5 * nZoneInts);

	return;

Found:
	g_dwUnk_460998 = nIndex;

	pOldPal = m_dcUnk00B4.SelectPalette(&g_paletteUnk_4A1D50, FALSE);

	if (m_dwUnk0544 != 0)
		pZone = (ZONE_DESCRIPTOR*)g_adwUnk_4609A0;
	else
		pZone = (ZONE_DESCRIPTOR*)g_adwUnk_4609A0 + g_dwUnk_460998;

	// In 2x stretch mode the render target is always 320x240, whatever mode
	// was asked for.
	if (m_dwUnk0544 != 0)
	{
		nWidth = 320;
		nHeight = 240;
	}

	m_dwUnk0540 = 0;

	pDoc = (CBumperDoc*)m_pDocument;

	// Put the previous frame's bitmap back before the render target is
	// destroyed, so the DIB section is not deleted while selected.
	if (m_dwUnk00C4 != 0)
		::SelectObject(m_dcUnk00B4.GetSafeHdc(), (HGDIOBJ)m_dwUnk00C4);

	delete m_pUnk00D0;

	m_pUnk00D0 = new CSub406090(nWidth, nHeight, pDoc->m_dwUnk0188);

	// The render target's size is copied as one 8-byte object (rule 56).
	*(SIZE*)&m_dwUnk008C = *(SIZE*)&m_pUnk00D0->m_dwWidth;

	m_rcUnk0220.left = 0;
	m_rcUnk0220.top = pZone->m_dwTopSplit;
	m_rcUnk0220.right = m_pUnk00D0->m_dwWidth;
	m_rcUnk0220.bottom = pZone->m_dwBottomSplit;

	m_pUnk00C8->Sub_410F20(3, m_dwUnk00E4);
	m_pUnk00C8->Sub_419530(m_pUnk00D0, &g_paletteUnk_4A1D50, 0, 0,
		(int)m_pUnk00D0->m_dwWidth, (int)m_pUnk00D0->m_dwHeight);
	m_pUnk00C8->Sub_410EC0();

	m_pUnk00C8->m_dwUnk60 = (DWORD)pDoc + 0x18C;

	// Select the new render target's DIB section into the view's offscreen
	// DC and remember the bitmap it displaces (CDC::SelectObject(CBitmap*)
	// inlines to SelectGdiObject(m_hDC, pBitmap->GetSafeHandle()); the
	// GetSafeHandle() on the result is the NULL test on the returned CBitmap*).
	// Everything that follows (PatBlt, Sub_410510, the GetPixel edge scans)
	// reads and writes this DC.
	m_dwUnk00C4 = (DWORD)m_dcUnk00B4.SelectObject(&m_pUnk00D0->m_bitmap)->GetSafeHandle();

	// The alternate renderer's viewport is the zone's +0x210 rectangle,
	// copied field by field (a plain RECT assignment; CRect's operator=
	// would be a ::CopyRect call).
	*(RECT*)&m_rcUnk0230 = pZone->m_rcUnk210;

	m_rcUnk0230.left &= ~3;
	m_rcUnk0230.right &= ~3;

	nSub = 1;
	if ((int)m_dwUnk00E4 > 3)
	{
		nSub = 2;
		if ((int)m_dwUnk00E4 > 5)
			nSub = 4;
	}

	dwLeft = m_rcUnk0230.left;
	dwSpan = m_rcUnk0230.right - dwLeft;
	dwMidX = ((dwSpan * 3) / 4) - nSub;
	dwTopY = (m_rcUnk0230.top + (dwMidX - m_rcUnk0230.bottom)) / 2;
	dwBottomY = dwTopY - m_rcUnk0230.top + m_rcUnk0230.bottom;

	if (m_rcUnk0230.bottom > m_rcUnk0220.top)
		m_rcUnk0230.bottom = m_rcUnk0220.top;

	ptOrigin = *(POINT*)&m_rcUnk0230;
	ptOrigin.y -= dwTopY;

	if (theApp.m_dwUnk01D0 == 0)
	{
		if (m_pUnk00CC != NULL)
		{
			delete m_pUnk00CC;
			m_pUnk00CC = NULL;
		}
	}
	else if (theApp.m_dwUnk01D0 == 1 && m_pUnk00CC == NULL)
	{
		m_pUnk00CC = new CSub410F20;
	}

	if (m_pUnk00CC != NULL)
	{
		m_pUnk00CC->m_dwUnk68 = 1;
		m_pUnk00CC->Sub_410F20(3, m_dwUnk00E4);
		m_pUnk00CC->Sub_419530(m_pUnk00D0, &g_paletteUnk_4A1D50,
			ptOrigin.x, ptOrigin.y, dwSpan, dwMidX);
		m_pUnk00CC->Sub_410EC0();
		m_pUnk00CC->m_dwUnk60 = (DWORD)pDoc + 0x18C;

		cbArray = dwSpan * sizeof(DWORD);
		adwTop = (DWORD*)::operator new(cbArray);
		adwBottom = (DWORD*)::operator new(cbArray);
		if (dwSpan > 0)
		{
			for (i = 0; i < dwSpan; i++)
				adwBottom[i] = dwTopY;
			for (i = 0; i < dwSpan; i++)
				adwTop[i] = dwBottomY;
		}
		m_pUnk00CC->Sub_414A10(adwBottom, adwTop);
		::operator delete(adwTop);
		::operator delete(adwBottom);
	}

	::PatBlt(m_dcUnk00B4.m_hDC, 0, 0, m_pUnk00D0->m_dwWidth, nHeight,
		BLACKNESS);

	Sub_410510();

	dwColor = ::GetPixel(m_dcUnk00B4.m_hDC, dwSpan, pZone->m_dwPad34);

	dwCount = m_pUnk00D0->m_dwWidth;
	cbArray = dwCount * sizeof(DWORD);
	adwTop = (DWORD*)::operator new(cbArray);
	adwBottom = (DWORD*)::operator new(cbArray);

	for (i = 0; (int)dwCount > i; i++)
	{
		adwBottom[i] = pZone->m_dwPad34;
		while ((DWORD)::GetPixel(m_dcUnk00B4.m_hDC, i, adwBottom[i]) == dwColor)
			adwBottom[i]--;

		adwTop[i] = pZone->m_dwPad3C;
		while ((DWORD)::GetPixel(m_dcUnk00B4.m_hDC, i, adwTop[i]) == dwColor)
			adwTop[i]++;
	}

	m_pUnk00C8->Sub_414A10(adwBottom, adwTop);

	::operator delete(adwTop);
	::operator delete(adwBottom);

	m_dcUnk00B4.SelectPalette(pOldPal, TRUE);

	pBackground = Hover_sub_418A30("BACKGRND");
	if (pBackground != NULL)
	{
		m_pUnk00C8->Sub_419940(0, (DWORD)pBackground);
		if (m_pUnk00CC != NULL)
			m_pUnk00CC->Sub_419940(0, (DWORD)pBackground);
	}
}

// Worker-thread body for the render state machine. The thread loops, waiting
// for the main thread to signal a state change on m_dwUnk0048, then dispatches
// one state per iteration. State 3 sets the exit flag; the others loop back
// and wait again. The setup pass optionally loads DISPDIB.DLL and creates the
// DisplayDibWindow. Decompiled from hover.exe:0x41ECE0.

#pragma pack(push, 1)
typedef struct tagLOADPARMS32
{
	DWORD lpEnvAddress;
	DWORD lpCmdLine;
	DWORD lpCmdShow;
	DWORD dwReserved;
} LOADPARMS32;
#pragma pack(pop)

// hover.exe:0x4605D0 cached module instance used by the renderer worker thread
// when creating the DisplayDibWindow; falls back to GetWindowLongA(GWL_HINSTANCE)
// if zero.

// The original compiles this function with optimization off (every value
// homed on the stack, a five-byte `jmp $+5` after most statements): see
// notes/perfect-match.md rule 49.
#pragma optimize("", off)

// Every store the worker thread makes into the app object goes through an
// inline accessor: with optimization off each call materialises the address
// of theApp into a stack temporary of its own (rule 92).
inline CHoverApp* HoverWorkerApp()
{
	return &theApp;
}

// hover.exe:0x41ECE0
// FUNCTION: HOVER 0x41ece0
void CBumperView::Sub_41ECE0()
{
	DWORD dwState;
	POINT ptArgs;
	int nStateArg;
	int nScaleArg;
	BOOL bContinue;

	bContinue = TRUE;

	m_dwUnk0528 = GetTickCount();
	m_dwUnk052C = m_dwUnk0528;
	m_dwUnk0530 = 0;

	HoverWorkerApp()->m_dwUnk03E4 = 0;

	{
		LOADPARMS32 loadParams;
		DWORD dwCmdShow;
		DWORD dwCmdLine;
		HINSTANCE hInstApp;
		HWND hWnd;

		hInstApp = (HINSTANCE)theApp.m_hInstance;
		hWnd = m_hWnd;

		dwCmdShow = 2;
		dwCmdLine = 0;
		loadParams.lpEnvAddress = 0;
		loadParams.lpCmdLine = (DWORD)&dwCmdLine;
		loadParams.lpCmdShow = (DWORD)&dwCmdShow;
		loadParams.dwReserved = 0;

		if (LoadModule("DISPDIB.DLL", &loadParams) < 0x20)
		{
			HoverWorkerApp()->m_dwUnk03F0 = 0;
		}
		else
		{
			HoverWorkerApp()->m_dwUnk03F0 = (DWORD)::CreateWindowExA(0, "DisplayDibWindow", "",
				WS_CHILD, 0, 0,
				::GetSystemMetrics(SM_CXSCREEN), ::GetSystemMetrics(SM_CYSCREEN),
				hWnd, NULL,
				(hInstApp != NULL) ? hInstApp
					: (HINSTANCE)::GetWindowLongA(hWnd, GWL_HINSTANCE),
				NULL);
		}
	}

	if (theApp.m_dwUnk03F0 != 0)
		HoverWorkerApp()->m_dwUnk03E0 = 1;
	else
		HoverWorkerApp()->m_dwUnk03E0 = 0;

	while (bContinue)
	{
		EnterCriticalSection(&theApp.m_csUnk011C);
			SetEvent(m_hUnk0050);
			ResetEvent(m_hUnk004C);
			LeaveCriticalSection(&theApp.m_csUnk011C);

			WaitForSingleObject((HANDLE)m_dwUnk0048, INFINITE);

			EnterCriticalSection(&theApp.m_csUnk011C);
			dwState = m_dwUnk00AC;
			switch (m_dwUnk00AC)
			{
			case 0:
				ptArgs = *(POINT*)&m_dwUnk00A4;
				break;
			case 1:
				nStateArg = theApp.m_dwUnk01C4;
				break;
			case 5:
				nScaleArg = m_dwUnk00B0;
				break;
			}
			ResetEvent(m_hUnk0050);
			SetEvent(m_hUnk004C);
			ResetEvent((HANDLE)m_dwUnk0048);
			SetEvent(m_hUnk0054);
			LeaveCriticalSection(&theApp.m_csUnk011C);

		try
		{
			switch (dwState)
			{
			case 0:
				Sub_4184B0(ptArgs.x, ptArgs.y);
				break;
			case 1:
				Sub_413C10(nStateArg);
				break;
			case 3:
				Sub_402250();
				break;
			case 2:
				bContinue = FALSE;
				break;
			case 4:
				Sub_410510();
				break;
			case 5:
				Sub_427A80(nScaleArg);
				break;
			case 6:
				Sub_427BB0();
				break;
			case 7:
				Sub_427CC0();
				break;
			}
		}
		catch (CException* e)
		{
			Hover_sub_4262B0(e);
		}
	}

	if (theApp.m_dwUnk03F0 != 0)
	{
		if (theApp.m_dwUnk03E8 != 0)
			::SendMessageA((HWND)theApp.m_dwUnk03F0, 0x404, 0, 0);
		::SendMessageA((HWND)theApp.m_dwUnk03F0, 0x402, 0, 0);
		HoverWorkerApp()->m_dwUnk03F0 = 0;
	}
	HoverWorkerApp()->m_dwUnk03E0 = 0;
	ResetEvent(m_hUnk004C);
	SetEvent(m_hUnk0050);
}

#pragma optimize("", on)

// Frame-step state 1 handler: load the level-start message strings and
// dispatch them to Sub_40CCE0. When theApp.m_dwUnk03B4 is set the two-line
// message is drawn; otherwise a three-line message is drawn.
// hover.exe:0x40D340
// FUNCTION: HOVER 0x40d340
void CBumperView::Sub_40D340()
{
	CString strA;
	CString strB;
	CString strC;

	strA.LoadString(0xEF15);
	strB.LoadString(0xEF16);

	if (theApp.m_dwUnk03B4 != 0)
	{
		Sub_40CCE0(strA, strB, NULL, NULL);
	}
	else
	{
		strC.LoadString(0xEF54);
		Sub_40CCE0(strA, strC, strB, NULL);
	}
}

// Draw a sunken rectangular box with a bevel `nWidth` pixels wide. The
// interior is filled with COLOR_BTNFACE; the bottom/right outer edges and
// the left/top inner edges are drawn with a COLOR_BTNSHADOW pen, and the
// left/top outer edges and the bottom/right inner edges with the white
// stock pen. The DC is left with the white stock brush selected and its
// state restored.
// hover.exe:0x427F40
// FUNCTION: HOVER 0x427f40
void CBumperView::Sub_427F40(RECT* pRect, int nWidth)
{
	CBrush brush(::GetSysColor(COLOR_BTNFACE));
	CPen pen(PS_SOLID, 1, ::GetSysColor(COLOR_BTNSHADOW));

	m_dcUnk00B4.SaveDC();
	m_dcUnk00B4.SelectObject(&brush);

	::PatBlt(m_dcUnk00B4.m_hDC, pRect->left, pRect->top,
		pRect->right - pRect->left, pRect->bottom - pRect->top, PATCOPY);

	m_dcUnk00B4.SelectObject(&pen);

	// Outer bottom/right edges.
	m_dcUnk00B4.MoveTo(pRect->left, pRect->bottom);
	m_dcUnk00B4.LineTo(pRect->right, pRect->bottom);
	m_dcUnk00B4.LineTo(pRect->right, pRect->top);

	// Inner left/top edges.
	m_dcUnk00B4.MoveTo(pRect->left + nWidth, pRect->bottom - nWidth);
	m_dcUnk00B4.LineTo(pRect->left + nWidth, pRect->top + nWidth);
	m_dcUnk00B4.LineTo(pRect->right - nWidth + 1, pRect->top + nWidth);

	m_dcUnk00B4.SelectStockObject(WHITE_PEN);

	// Outer left/top edges.
	m_dcUnk00B4.MoveTo(pRect->left, pRect->bottom);
	m_dcUnk00B4.LineTo(pRect->left, pRect->top);
	m_dcUnk00B4.LineTo(pRect->right + 1, pRect->top);

	// Inner bottom/right edges.
	m_dcUnk00B4.MoveTo(pRect->left + nWidth, pRect->bottom - nWidth);
	m_dcUnk00B4.LineTo(pRect->right - nWidth, pRect->bottom - nWidth);
	m_dcUnk00B4.LineTo(pRect->right - nWidth, pRect->top + nWidth);

	m_dcUnk00B4.SelectStockObject(WHITE_BRUSH);
	m_dcUnk00B4.RestoreDC(-1);
}

// Frame-step state 2 handler: draw up to four centered message strings in a
// raised box. The first string is always present; the remaining three are
// optional (NULL skips them). The original uses a 600-weight Arial font whose
// height is taken from the current zone descriptor.
// hover.exe:0x40CCE0
// FUNCTION: HOVER 0x40cce0
void CBumperView::Sub_40CCE0(const char* pszA, const char* pszB,
	const char* pszC, const char* pszD)
{
	CFont* pOldFont;
	int cxMax;
	int cyFont;
	int cyTotal;
	int cySpacing;
	int cyHalfSpacing;
	int nCount;
	int x;
	int y;
	RECT rc;
	ZONE_DESCRIPTOR* pZone;
	COLORREF clrShadow;
	COLORREF clrText;

	// Select the zone descriptor: zone 0 is forced in 2x stretch mode.
	if (m_dwUnk0544 != 0)
		pZone = (ZONE_DESCRIPTOR*)g_adwUnk_4609A0;
	else
		pZone = (ZONE_DESCRIPTOR*)g_adwUnk_4609A0 + g_dwUnk_460998;

	// Create the message font and select it into the offscreen DC.
	CFont font;

	font.Attach(::CreateFont(pZone->m_dwUnk334, 0, 0, 0, 0x258, 0, 0, 0,
		0, 2, 0, 0, 0x20, "Arial"));
	pOldFont = m_dcUnk00B4.SelectObject(&font);

	// Measure every supplied string and keep the widest width.  CDC::
	// GetTextExtent is an inline that returns a CSize by value through the
	// attribute DC, so every call gets an 8-byte return temporary of its own
	// (rule 118) and reads m_hAttribDC, not GetSafeHdc().
	nCount = 1;

	CSize sizeA = m_dcUnk00B4.GetTextExtent(pszA, strlen(pszA));

	cxMax = sizeA.cx;
	cyFont = sizeA.cy;
	cySpacing = (3 * cyFont) / 2;

	if (pszB != NULL)
	{
		CSize sizeB = m_dcUnk00B4.GetTextExtent(pszB, strlen(pszB));

		cxMax = (sizeA.cx > sizeB.cx) ? sizeA.cx : sizeB.cx;
		nCount = 2;
	}
	if (pszC != NULL)
	{
		CSize sizeC = m_dcUnk00B4.GetTextExtent(pszC, strlen(pszC));

		if (sizeC.cx > cxMax)
			cxMax = sizeC.cx;
		nCount++;
	}
	if (pszD != NULL)
	{
		CSize sizeD = m_dcUnk00B4.GetTextExtent(pszD, strlen(pszD));

		if (sizeD.cx > cxMax)
			cxMax = sizeD.cx;
		nCount++;
	}

	// Compute the total height of the message block.
	cyTotal = ((3 * nCount - 1) * cyFont) / 2;

	// Center the block in the view and shift it up by half the absolute zone
	// composite offset.
	x = ((int)m_dwUnk008C - cxMax) / 2;
	y = ((int)m_dwUnk0090 - cyTotal) / 2;
	y -= abs((pZone->m_rcUnk230.bottom - pZone->m_rcUnk240.top
		- pZone->m_rcComposite1.top) + pZone->m_rcComposite1.bottom) / 2;

	rc.left = x;
	rc.top = y;
	rc.right = x + cxMax;
	rc.bottom = y + cyTotal;

	// Inflate the box by one third of the line spacing (cyFont/2).
	cyHalfSpacing = cySpacing / 3;
	::InflateRect(&rc, cyHalfSpacing, cyHalfSpacing);

	// Save the inflated rectangle, doubling it in stretch mode for Sub_427F40.
	m_rcUnk0094 = rc;
	if (m_dwUnk0544 != 0)
	{
		m_rcUnk0094.left <<= 1;
		m_rcUnk0094.top <<= 1;
		m_rcUnk0094.right <<= 1;
		m_rcUnk0094.bottom <<= 1;
	}

	// Draw the raised box (from the un-doubled local rectangle; the doubled
	// copy in m_rcUnk0094 is for the presenter) and set transparent
	// background for the text.
	Sub_427F40(&rc, 3);
	m_dcUnk00B4.SetBkMode(TRANSPARENT);

	// Get the system colors used for the shadow/highlight and text.
	clrShadow = ::GetSysColor(COLOR_BTNSHADOW);
	clrText = ::GetSysColor(COLOR_BTNTEXT);

	// Draw each supplied string: the first line sits one margin (cySpacing/3)
	// below the box top, every further line one full line pitch (3*cyFont/2)
	// below the previous one (hover.exe:0x40CFC2 adds [ebp-0x18], the three
	// later sites add [ebp-0x14]).
	if (pszA != NULL)
	{
		rc.top += cyHalfSpacing;
		Sub_40D0A0(&rc, pszA, clrShadow, clrText);
	}
	if (pszB != NULL)
	{
		rc.top += cySpacing;
		Sub_40D0A0(&rc, pszB, clrShadow, clrText);
	}
	if (pszC != NULL)
	{
		rc.top += cySpacing;
		Sub_40D0A0(&rc, pszC, clrShadow, clrText);
	}
	if (pszD != NULL)
	{
		rc.top += cySpacing;
		Sub_40D0A0(&rc, pszD, clrShadow, clrText);
	}

	// Restore the previous font.
	m_dcUnk00B4.SelectObject(pOldFont);
}

// Helper for Sub_40CCE0: draw one message line. In non-stretch mode the text is
// drawn with a dark shadow at (+1,+1) and a white highlight at (-1,-1) to give
// a raised appearance; in stretch mode it is drawn once with the text color.
// hover.exe:0x40D0A0
// FUNCTION: HOVER 0x40d0a0
void CBumperView::Sub_40D0A0(RECT* pRect, const char* psz, COLORREF clrShadow,
	COLORREF clrText)
{
	if (m_dwUnk0544 == 0)
	{
		::OffsetRect(pRect, 1, 1);
		m_dcUnk00B4.SetTextColor(clrShadow);
		m_dcUnk00B4.DrawText(psz, -1, pRect, DT_CENTER);

		::OffsetRect(pRect, -2, -2);
		m_dcUnk00B4.SetTextColor(RGB(255, 255, 255));
		m_dcUnk00B4.DrawText(psz, -1, pRect, DT_CENTER);

		::OffsetRect(pRect, 1, 1);
	}

	m_dcUnk00B4.SetTextColor(clrText);
	m_dcUnk00B4.DrawText(psz, -1, pRect, DT_CENTER);
}

// Frame-step state 3 handler: display the end-of-level bonus score.
// Loads the "Bonus Score" title, formats the remaining-flags score line,
// the next-level message and an optional level-bonus line, then dispatches
// the strings to Sub_40CCE0 for centered drawing.
// hover.exe:0x4283F0
// FUNCTION: HOVER 0x4283f0
void CBumperView::Sub_4283F0()
{
	CString strA;
	CString strB;
	CString strC;
	CString strD;

	strD.LoadString(0xEF52);

	CString strFormat;

	if (m_dwUnk0504 == 1)
		strFormat.LoadString(0xEF3D);
	else
		strFormat.LoadString(0xEF62);

	strA.Format(strFormat, m_dwUnk0504, m_dwUnk0508, m_dwUnk050C);

	strFormat.LoadString(0xEF3E);
	strB.Format(strFormat, m_dwUnk0510);

	if (m_dwUnk0514 != 0)
	{
		strFormat.LoadString(0xEF50);
		strC.Format(strFormat, m_dwUnk0514);
		Sub_40CCE0(strD, strA, strB, strC);
	}
	else
	{
		Sub_40CCE0(strD, strA, strB, NULL);
	}
}

// Helper stubs for the renderer setup / worker-thread cluster.

// Set the offscreen bitmap size from the client rect and synchronize with
// the renderer worker thread.  nUnk selects the full handshake path when
// nonzero.

// hover.exe:0x4274B0
// FUNCTION: HOVER 0x4274b0
void CBumperView::Sub_4274B0(int nWidth, int nHeight, int nUnk)
{
	WaitForSingleObject((HANDLE)m_dwUnk0048, 0);
	WaitForSingleObject(m_hUnk004C, 0);
	WaitForSingleObject(m_hUnk0050, 0);
	WaitForSingleObject(m_hUnk0054, 0);
	WaitForSingleObject(m_hUnk0058, 0);
	WaitForSingleObject(m_hUnk005C, 0);

	if (WaitForSingleObject(m_hUnk0054, 0) == WAIT_TIMEOUT &&
		m_dwUnk00AC != 0)
	{
		Hover_sub_427A10(m_hUnk0054);
	}

	EnterCriticalSection(&theApp.m_csUnk011C);
	m_dwUnk00AC = 0;
	DWORD* pdwSize = &m_dwUnk00A4;
	pdwSize[0] = nWidth;
	pdwSize[1] = nHeight;
	LeaveCriticalSection(&theApp.m_csUnk011C);

	if (nUnk != 0)
	{
		Hover_sub_427A10(m_hUnk0050);
		EnterCriticalSection(&theApp.m_csUnk011C);
		ResetEvent(m_hUnk0054);
		SetEvent((HANDLE)m_dwUnk0048);
		LeaveCriticalSection(&theApp.m_csUnk011C);
		Hover_sub_427A10(m_hUnk0054);
		Hover_sub_427A10(m_hUnk0050);
	}
	else
	{
		EnterCriticalSection(&theApp.m_csUnk011C);
		ResetEvent(m_hUnk0054);
		SetEvent((HANDLE)m_dwUnk0048);
		LeaveCriticalSection(&theApp.m_csUnk011C);
	}

	m_dwUnk0088 = 1;
}

// Background/dashboard setup helper.  Loads the zone's dashboard bitmap from
// dashres.dll, configures the gauge/indicator helper objects from the zone
// descriptor, clears the client area and composites the sprite layers.

// hover.exe:0x410510
// FUNCTION: HOVER 0x410510
void CBumperView::Sub_410510()
{
	ZONE_DESCRIPTOR* pZone;
	DWORD* pdwSize;
	HINSTANCE hInst;
	DWORD dwResId;
	HRSRC hRsrc;
	HGLOBAL hResData;
	DWORD dwSize;
	BYTE* pDib;
	BYTE* pBits;
	BYTE* pDibBits;
	CPalette* pOldPal;
	int nWidth;
	int nHeight;
	RECT rcTemp;
	RECT rcA;
	RECT rcB;
	pOldPal = m_dcUnk00B4.SelectPalette(&g_paletteUnk_4A1D50, FALSE);

	// Select the zone descriptor: zone 0 is forced in 2x stretch mode.  The
	// base is assigned unconditionally and only the index is guarded, the
	// way Sub_410380 does it - the original loads `mov ebp,g_adwUnk_4609A0`
	// at hover.exe:0x410526, before the `cmp [ebx+0x544],0` that decides
	// whether to scale the index in at all.
	pZone = (ZONE_DESCRIPTOR*)g_adwUnk_4609A0;
	if (m_dwUnk0544 == 0)
		pZone += g_dwUnk_460998;

	hInst = ::LoadLibraryA("dashres.dll");

	// The resource id is the zone index plus 0x81; in 2x stretch mode the
	// index is 0, so the id is 0x81, not 0 (the original adds the 0x81
	// after the two arms join, at hover.exe:0x410578).
	if (m_dwUnk0544 != 0)
		dwResId = 0;
	else
		dwResId = g_dwUnk_460998;

	hRsrc = Hover_sub_453352(hInst, (LPCSTR)(dwResId + 0x81), RT_BITMAP);
	hResData = ::LoadResource(hInst, hRsrc);
	if (hResData == NULL)
		AfxThrowMemoryException();

	pDib = (BYTE*)::LockResource(hResData);
	if (pDib == NULL)
		AfxThrowMemoryException();

	dwSize = ::SizeofResource(hInst, hRsrc);
	pBits = (BYTE*)::operator new(dwSize);
	memcpy(pBits, pDib, dwSize);

	::FreeLibrary(hInst);

	// Everything below this point samples the *pixels* of the dashboard
	// DIB, never the file header: the original computes the pixel base once
	// into esi (`lea esi,[eax+ecx+0x400]` with eax = biSize at
	// hover.exe:0x4105FC) and pushes that same esi for the palette remap,
	// for the two Sub_411BB0 sprites, for the four Sub_411A00 digit banks
	// and for both Hover_sub_411770 composites.
	pDibBits = pBits + ((BITMAPINFOHEADER*)pBits)->biSize + 0x400;
	Hover_sub_410CE0(pBits, pDibBits, &g_paletteUnk_4A1D50);

	// The two sprite banks, the four digit banks and both composite passes
	// all read the descriptor's width/height pair through the same pointer
	// local: that is what keeps the original's `lea edi,[ebp+8]` (hover.exe
	// :0x410721) live as `mov ecx,[edi] / mov edx,[edi+4]` across ten call
	// sites, all the way down to the spill at 0x4109D2 (rule 94).  It is
	// set here rather than at its first use so that it and pDibBits hold
	// the last two callee-saved registers over the seven Sub_411D60 calls:
	// with one free VC hoists `&m_pen04D0` into it as a common
	// subexpression, and the original recomputes `lea ecx,[ebx+0x4d0]` at
	// every one of the seven sites.
	pdwSize = &pZone->m_dwUnk08;

	m_obj02E0.Sub_411D60(pZone->m_adwGaugeParams[0], (DWORD)&m_brush04E8,
		(DWORD)&m_pen04D0, (DWORD)&m_brush04E0);
	m_obj0340.Sub_411D60(pZone->m_adwGaugeParams[2], (DWORD)&m_brush04F8,
		(DWORD)&m_pen04D0, (DWORD)&m_brush04E0);
	m_obj0370.Sub_411D60(pZone->m_adwGaugeParams[2], (DWORD)&m_brush04F0,
		(DWORD)&m_pen04D0, (DWORD)&m_brush04E0);
	m_obj0310.Sub_411D60(pZone->m_adwGaugeParams[1], (DWORD)&m_brush04E8,
		(DWORD)&m_pen04D0, (DWORD)&m_brush04E0);
	m_obj03A0.Sub_411D60(pZone->m_adwGaugeParams[3], (DWORD)&m_brush04E8,
		(DWORD)&m_pen04D0, (DWORD)&m_brush04E0);
	m_obj03D0.Sub_411D60(pZone->m_adwGaugeParams[5], (DWORD)&m_brush04E8,
		(DWORD)&m_pen04D0, (DWORD)&m_brush04E0);
	m_obj0400.Sub_411D60(pZone->m_adwGaugeParams[4], (DWORD)&m_brush04E8,
		(DWORD)&m_pen04D0, (DWORD)&m_brush04E0);

	// The original reads *both* sprite rectangles through a base of their
	// own (`lea edx,[ebp+0x1e0]` then `lea ecx,[ebp+0x1b0]`, hover.exe:
	// 0x410728/0x41073E), but it creates them lazily inside the argument
	// evaluation, after the level-index division has finished with eax/edx.
	// A declared local is initialized at the head of its block instead, so
	// only one of the two fits: with both declared VC spills the second
	// across the `idiv` and reloads it for each of its four fields.

	{
		RECT* prc1 = &pZone->m_rcSpriteParams[0];

		m_obj0480.Sub_411BB0(
			LVLPARAM((int)g_dwUnk_46049C).m_dwUnk30,
			prc1->left, prc1->top, prc1->right, prc1->bottom,
			pZone->m_rcSpriteParams[3].left,
			pZone->m_rcSpriteParams[3].top,
			pZone->m_rcSpriteParams[3].right,
			pZone->m_rcSpriteParams[3].bottom,
			pDibBits,
			pdwSize[0], pZone->m_dwUnk0C);
		// The height of the *first* sprite bank is the one field the
		// original still reads off the descriptor itself (`mov edx,
		// [ebp+0xc]`, hover.exe:0x41071E): the width/height base is born
		// one reference later, at `lea edi,[ebp+8]` (0x410721).  Spelling
		// it that way is one reference fewer through the pointer and it
		// keeps the pointer ahead of the pixel base in VC's ranking.
	}

	{
		RECT* prc1 = &pZone->m_rcSpriteParams[1];

		m_obj0430.Sub_411BB0(
			LVLPARAM((int)g_dwUnk_46049C).m_dwUnk30,
			prc1->left, prc1->top, prc1->right, prc1->bottom,
			pZone->m_rcSpriteParams[2].left,
			pZone->m_rcSpriteParams[2].top,
			pZone->m_rcSpriteParams[2].right,
			pZone->m_rcSpriteParams[2].bottom,
			pDibBits,
			pdwSize[0], pdwSize[1]);
	}

	m_obj02B8.Sub_411A00(7, &pZone->m_rcWaypointRow1[0], pDibBits,
		pdwSize[0], pdwSize[1]);
	m_obj0240.Sub_411A00(1, &pZone->m_rcWaypointRow0[0], pDibBits,
		pdwSize[0], pdwSize[1]);
	m_obj0268.Sub_411A00(1, &pZone->m_rcWaypointRow0[0], pDibBits,
		pdwSize[0], pdwSize[1]);
	m_obj0290.Sub_411A00(1, &pZone->m_rcWaypointRow0[0], pDibBits,
		pdwSize[0], pdwSize[1]);

	m_obj0240.Sub_411920((RECT*)&pZone->m_adwWaypointMarkers[0]);
	m_obj0268.Sub_411920((RECT*)&pZone->m_adwWaypointMarkers[4]);
	m_obj0290.Sub_411920((RECT*)&pZone->m_adwWaypointMarkers[2]);
	m_obj02B8.Sub_411920(&pZone->m_rcUnk220);

	m_obj0430.Sub_411CF0(&pZone->m_rcUnk228);
	m_obj0480.Sub_411CF0(&pZone->m_rcUnk230);

	((CSub423030*)&m_obj02E0)->Sub_411D10(&pZone->m_rcGaugeRects[0]);
	((CSub423030*)&m_obj0340)->Sub_411D10(&pZone->m_rcGaugeRects[2]);
	((CSub423030*)&m_obj0370)->Sub_411D10(&pZone->m_rcGaugeRects[2]);
	((CSub423030*)&m_obj0310)->Sub_411D10(&pZone->m_rcGaugeRects[1]);
	((CSub423030*)&m_obj03A0)->Sub_411D10(&pZone->m_rcGaugeRects2[0]);
	((CSub423030*)&m_obj03D0)->Sub_411D10(&pZone->m_rcGaugeRects2[2]);
	((CSub423030*)&m_obj0400)->Sub_411D10(&pZone->m_rcGaugeRects2[1]);

	{
		RECT* prc = &pZone->m_rcViewport;

		m_obj0150.Sub_40FEE0(&m_dcUnk00B4, this, theApp.m_pBumperDoc,
			prc->left, prc->top, prc->right, prc->bottom);
	}

	{
		RECT* prc = &pZone->m_rcUnk278;

		m_obj00E8.Sub_411D80(&m_dcUnk00B4, pBits,
			prc->left, prc->top, prc->right, prc->bottom);
	}

	m_obj00E8.m_pUnk64 = &m_pen04D8;
	::GetClientRect(m_hWnd, &rcTemp);

	// The two extents are statements of their own: the original subtracts
	// both (`sub eax,[esp+0x44] / sub ecx,[esp+0x40]`, hover.exe:0x4109A9
	// and 0x4109AD) *before* it pushes the BLACKNESS constant, which is
	// what a right-to-left argument evaluation of the subtractions would
	// never do.  Height is the first statement: the original computes
	// bottom-top into eax and only then right-left into ecx.
	nHeight = rcTemp.bottom - rcTemp.top;
	nWidth = rcTemp.right - rcTemp.left;
	::PatBlt(m_dcUnk00B4.m_hDC, rcTemp.left, rcTemp.top, nWidth, nHeight,
		BLACKNESS);
	::GdiFlush();

	// The descriptor's size pair is copied out of the pointer local into
	// two scalars here (hover.exe:0x4109D2 `mov ecx,[edi] / mov eax,[edi+4]`
	// into the frame slots at +0x18/+0x1C): both composite passes read the
	// scalars, and edi is free again for the rectangle copies below.  These
	// two slots are what makes the frame 0x40 rather than 0x38 bytes.
	HOVER_ZONESIZE zone = Hover_ZoneSize(pdwSize);

	// First composite pass (zone descriptor rects at +0x1f0/+0x200).
	rcA = pZone->m_rcComposite1;
	rcB = pZone->m_rcComposite2;

	Hover_sub_411770(
		rcA.left, rcA.top, rcA.right, rcA.bottom,
		zone.cx, zone.cy,
		rcB.left, rcB.top, rcB.right, rcB.bottom,
		m_pUnk00D0->m_dwWidth, m_pUnk00D0->m_dwHeight,
		pDibBits, (BYTE*)m_pUnk00D0->m_dwUnk04, -1);

	// Second composite pass (zone descriptor rects at +0x238/+0x248).
	rcA = pZone->m_rcUnk238;
	rcB = pZone->m_rcUnk248;

	Hover_sub_411770(
		rcA.left, rcA.top, rcA.right, rcA.bottom,
		zone.cx, zone.cy,
		rcB.left, rcB.top, rcB.right, rcB.bottom,
		m_pUnk00D0->m_dwWidth, m_pUnk00D0->m_dwHeight,
		pDibBits, (BYTE*)m_pUnk00D0->m_dwUnk04, -1);

	m_obj02E0.Sub_401AD0(&m_dcUnk00B4, 1);
	m_obj0370.Sub_401AD0(&m_dcUnk00B4, 1);
	m_obj0340.Sub_401AD0(&m_dcUnk00B4, 1);
	m_obj0310.Sub_401AD0(&m_dcUnk00B4, 1);
	m_obj03A0.Sub_401AD0(&m_dcUnk00B4, 1);
	m_obj03D0.Sub_401AD0(&m_dcUnk00B4, 1);
	m_obj0400.Sub_401AD0(&m_dcUnk00B4, 1);
	m_obj00E8.Sub_401E80(&m_dcUnk00B4, 1);
	::GdiFlush();

	// Each of the six element uploads reads the render target's size pair
	// through the by-value accessor (rule 200): the six sites share one
	// 8-byte temporary of which only the `.cx` store survives
	// (hover.exe:0x410BC1 `mov [esp+0x34],ecx`, 0x410BCB `mov eax,[esp+0x34]`).
	{
		HOVER_ZONESIZE size;

		size = Hover_ZoneSize(&m_pUnk00D0->m_dwWidth);
		m_obj0430.Sub_40E9A0(m_pUnk00D0->m_dwUnk04, size.cx, size.cy);
		size = Hover_ZoneSize(&m_pUnk00D0->m_dwWidth);
		m_obj0480.Sub_40E9A0(m_pUnk00D0->m_dwUnk04, size.cx, size.cy);

		size = Hover_ZoneSize(&m_pUnk00D0->m_dwWidth);
		m_obj02B8.Sub_40FCE0(m_pUnk00D0->m_dwUnk04, size.cx, size.cy);
		size = Hover_ZoneSize(&m_pUnk00D0->m_dwWidth);
		m_obj0240.Sub_40FCE0(m_pUnk00D0->m_dwUnk04, size.cx, size.cy);
		size = Hover_ZoneSize(&m_pUnk00D0->m_dwWidth);
		m_obj0268.Sub_40FCE0(m_pUnk00D0->m_dwUnk04, size.cx, size.cy);
		size = Hover_ZoneSize(&m_pUnk00D0->m_dwWidth);
		m_obj0290.Sub_40FCE0(m_pUnk00D0->m_dwUnk04, size.cx, size.cy);
	}

	::operator delete(pBits);

	if (theApp.m_dwUnk03B0 != 0)
		Sub_428120();

	Sub_410380();

	m_dcUnk00B4.SelectPalette(pOldPal, TRUE);
}

// FUNCTION: CBumperView::Sub_410380
// hover.exe:0x410380
// FUNCTION: HOVER 0x410380
void CBumperView::Sub_410380()
{
	CString strLabel;
	CFont font;
	ZONE_DESCRIPTOR* pZone;

	// Select the zone descriptor: zone 0 is forced in 2x stretch mode.
	pZone = (ZONE_DESCRIPTOR*)g_adwUnk_4609A0;
	if (m_dwUnk0544 == 0)
		pZone += g_dwUnk_460998;

	// Create the dashboard-label font and select it into the offscreen DC.
	font.Attach(::CreateFont(pZone->m_dwUnk2D0, 0, 0, 0, 0x258, 0, 0, 0,
		0, 2, 0, 0, 0x20, "Arial"));

	m_dcUnk00B4.SaveDC();
	m_dcUnk00B4.SelectObject(&font);
	m_dcUnk00B4.SetBkMode(TRANSPARENT);
	m_dcUnk00B4.SetTextColor(0);

	// Draw the three dashboard labels into their zone rectangles.  The
	// labels are the configured control keys themselves: the VK codes in
	// theApp.m_avkControls equal the ASCII letter for A-Z and 0-9, and
	// CString::operator=(char) turns each into a one-character string.
	strLabel = (char)theApp.m_avkControls[2];
	m_dcUnk00B4.DrawText(strLabel, -1, &pZone->m_rcUnk2D4,
		DT_SINGLELINE | DT_CENTER | DT_VCENTER);

	strLabel = (char)theApp.m_avkControls[0];
	m_dcUnk00B4.DrawText(strLabel, -1, &pZone->m_rcUnk2F4,
		DT_SINGLELINE | DT_CENTER | DT_VCENTER);

	strLabel = (char)theApp.m_avkControls[1];
	m_dcUnk00B4.DrawText(strLabel, -1, &pZone->m_rcUnk2E4,
		DT_SINGLELINE | DT_CENTER | DT_VCENTER);

	m_dcUnk00B4.RestoreDC(-1);
}

// Draws the small "F4" dashboard overlay in the bottom-right corner of the
// offscreen DC.  In 2x stretch mode a custom 10pt "Small Font" is created
// and selected; otherwise the ANSI variable-width stock font is used.  The
// text is measured, a raised box is drawn around it, the rectangle is shrunk
// and the label is drawn shadowed with Sub_40D0A0.

// FUNCTION: CBumperView::Sub_428120
// hover.exe:0x428120
// FUNCTION: HOVER 0x428120
void CBumperView::Sub_428120()
{
	CFont font;
	CSize size;
	RECT rc;
	int nMargin;

	nMargin = 3;

	m_dcUnk00B4.SaveDC();

	if (m_dwUnk0544 != 0)
	{
		font.Attach(::CreateFontA(-10, 0, 0, 0, 400, 0, 0, 0, 0, 0, 0, 0, 0,
			"Small Font"));
		nMargin = 2;
		m_dcUnk00B4.SelectObject(&font);
	}
	else
	{
		m_dcUnk00B4.SelectStockObject(ANSI_VAR_FONT);
	}

	// The inner inset is written out at both use sites (the original
	// re-emits the cmp/adc pair for the InflateRect call).
	size = m_dcUnk00B4.GetTextExtent("F4", 2);
	size.cy += nMargin * 2;
	size.cx += (((m_dwUnk0544 < 1) ? 2 : 1) + nMargin) * 2;

	rc.right = m_dwUnk008C;
	rc.left = rc.right - size.cx;
	rc.bottom = m_dwUnk0090;
	rc.top = rc.bottom - size.cy;

	Sub_427F40(&rc, nMargin);
	::InflateRect(&rc, -(((int)(m_dwUnk0544 < 1) ? 2 : 1) + nMargin),
		-nMargin);

	Sub_40D0A0(&rc, "F4", ::GetSysColor(COLOR_BTNSHADOW),
		::GetSysColor(COLOR_BTNTEXT));

	m_dcUnk00B4.SelectStockObject(ANSI_VAR_FONT);
	m_dcUnk00B4.RestoreDC(-1);
}

// Re-selects the span drawers after the speed/detail slider moved. The
// slider position indexes g_adwUnk_4C4CC8, whose value is the *mode*
// argument of Sub_410F20 (1..6 = the texel/row subsampling family); the
// flags argument is always 3, so both option bits are set and the full
// drawer table is installed. Passing these the other way round leaves the
// context permanently in mode 3 and pairs a shaded span walker with the
// unshaded flat drawer Hover_sub_42A330, which then writes past the end of
// the render target.

// FUNCTION: CBumperView::Sub_413C10
// hover.exe:0x413C10
// FUNCTION: HOVER 0x413c10
void CBumperView::Sub_413C10(int nUnk)
{
	m_dwUnk00E4 = g_adwUnk_4C4CC8[nUnk];

	m_pUnk00C8->Sub_410F20(3, m_dwUnk00E4);
	m_pUnk00C8->Sub_410EC0();

	if (m_pUnk00CC != NULL)
	{
		m_pUnk00CC->Sub_410F20(3, m_dwUnk00E4);
		m_pUnk00CC->Sub_410EC0();
	}
}

// FUNCTION: CBumperView::Sub_4126C0
// hover.exe:0x4126C0
// FUNCTION: HOVER 0x4126c0
void CBumperView::Sub_4126C0()
{
	LPTIMECALLBACK lpfnTimer;

	g_dwUnk_4C4CE0 = 1;

	if (theApp.m_dwUnk03B0 != 0)
	{
		if (theApp.m_dwUnk03E4 != 0 && theApp.m_dwUnk03E8 == 0)
		{
			Sub_4277E0(1);
		}
		else
		{
			::SetMenu(AfxGetApp()->m_pMainWnd->m_hWnd, NULL);
		}

		if (m_dwUnk0034 != 0)
		{
			::ShowCursor(FALSE);
			m_dwUnk0034 = 0;
		}
	}

	if (theApp.m_dwUnk040C != 0 && theApp.m_dwUnk01B8 == 3)
	{
		::SetThreadPriority((HANDLE)theApp.m_hThread, THREAD_PRIORITY_NORMAL);
		theApp.m_pBumperDoc->m_obj0060.Sub_413710();
	}

	Hover_sub_40E560(1);

	if (theApp.m_dwUnk0404 != 0)
	{
		((CSub40B800*)theApp.m_pBumperDoc->m_adwUnk83EC[0x1A])->Sub_40B800(
			5, 5, 0xFFFF);
	}

	::SetThreadPriority((HANDLE)theApp.m_hThread, THREAD_PRIORITY_ABOVE_NORMAL);

	if (m_dwUnk0040 != 0)
	{
		::SetThreadPriority(((CWinThread*)m_dwUnk0040)->m_hThread,
			THREAD_PRIORITY_NORMAL);
	}

	if (g_pThreadUnk_4C56E0 != NULL)
	{
		::SetThreadPriority(g_pThreadUnk_4C56E0->m_hThread,
			THREAD_PRIORITY_ABOVE_NORMAL);
	}

	if (m_dwUnk0500 == 1)
	{
		m_dwUnk0500 = 0;
		m_dwUnk00A0 = 0xFFFFFFFF;
		m_dwUnk0080 = 1;
		Sub_40DC20(1);
		RedrawWindow();
	}
	else
	{
		m_dwUnk0500 = 0;
		m_dwUnk00A0 = 0xFFFFFFFF;
	}

	if (m_dwUnk0060 == 0)
	{
		DWORD dwTick;

		lpfnTimer = Hover_sub_408AA0;
		m_dwUnk0060 = timeSetEvent(0x32, 0x0A, lpfnTimer, 0, TIME_PERIODIC);
		if (m_dwUnk0060 == 0)
			AfxMessageBox("Timer could not be started", 0, 0);
		dwTick = GetTickCount();
		m_dwUnk053C = 0;
		m_dwUnk0534 = dwTick;
	}
	else
	{
		AfxMessageBox("Timer tried to start twice", 0, 0);
	}

	g_dwUnk_4C4CE0 = 0;
}

// FUNCTION: CBumperView::Sub_427A80
// hover.exe:0x427A80
// FUNCTION: HOVER 0x427a80
void CBumperView::Sub_427A80(int nUnk)
{
	ZONE_DESCRIPTOR* pZone;
	DWORD dwMul;
	RECT rcClient;

	pZone = (ZONE_DESCRIPTOR*)g_adwUnk_4609A0;
	if (m_dwUnk0544 == 0)
		pZone += g_dwUnk_460998;

	DWORD* pdwVal = &m_obj0150.m_dwUnk00C8;
	if (nUnk != 0)
	{
		if ((long)*pdwVal < 0xA0000)
			goto ComputeMul;
		return;
	}
	else
	{
		if ((long)*pdwVal <= 0xCCCC)
			return;
	}

ComputeMul:
	if (nUnk != 0)
		dwMul = 0x1547a;
	else
		dwMul = 0xC000;
	HOVER_FIXMULPP(pdwVal, dwMul);

	if (m_dwUnk0540 == 0)
	{
		m_obj0150.Sub_40FEE0(&m_dcUnk00B4, this, theApp.m_pBumperDoc,
			(DWORD)pZone->m_rcViewport.left, (DWORD)pZone->m_rcViewport.top,
			(DWORD)pZone->m_rcViewport.right, (DWORD)pZone->m_rcViewport.bottom);
	}
	else
	{
		::GetClientRect(m_hWnd, &rcClient);
		m_obj0150.Sub_40FEE0(&m_dcUnk00B4, this, theApp.m_pBumperDoc,
			(DWORD)rcClient.left, (DWORD)rcClient.top,
			(DWORD)rcClient.right, (DWORD)rcClient.bottom);
	}
}

// hover.exe:0x40FB00
// FUNCTION: HOVER 0x40fb00
void CSub422F50::Sub_40FB00(DWORD dwBits, DWORD dwWidth, DWORD dwHeight)
{
	long lWidth;
	long lRem;
	DWORD* pdwDim;

	// The width/height pair is written through a pointer over the two
	// members and the width read back out of it (rule 94).
	m_pUnk10 = (BYTE*)dwBits;
	pdwDim = (DWORD*)&m_dwUnk14;
	pdwDim[0] = dwWidth;
	pdwDim[1] = dwHeight;

	lWidth = (long)pdwDim[0];
	m_dwUnk24 = (DWORD)lWidth;
	lRem = lWidth % 4;
	if (lRem != 0)
		lWidth += lRem;
	m_dwUnk28 = (DWORD)(lWidth >> 2);
}

// hover.exe:0x40FBB0
// FUNCTION: HOVER 0x40fbb0
void CSub422F50::Sub_40FBB0(DWORD dwBottom, DWORD dwRight, DWORD dwTop,
	DWORD dwRight2)
{
	long lBottom;
	long lTop;
	long lHeightMinusRight;
	long lRem;
	long lBits;

	// The four arguments are copied into +0x00..+0x0C as one 16-byte
	// block through a pointer over the argument home.
	*(RECT*)&m_dwUnk00 = *(RECT*)&dwBottom;

	lHeightMinusRight = (long)m_dwUnk18 - (long)dwRight;
	m_dwUnk08--;
	lHeightMinusRight--;
	m_dwUnk0C--;

	m_dwUnk48 = (DWORD)lHeightMinusRight;
	m_dwUnk44 = (DWORD)(lHeightMinusRight + (long)dwRight - (long)dwRight2);

	// The two dword-alignment flags are "the coordinate is a multiple of
	// four", not "leaves remainder one".
	lBottom = (long)dwBottom;
	lRem = lBottom % 4;
	m_dwUnk34 = (lRem == 0) ? 1 : 0;

	// Only the flag looks at dwTop+1; the two derived pointers below use
	// dwTop itself.
	lTop = (long)dwTop;
	lRem = (lTop + 1) % 4;
	m_dwUnk38 = (lRem == 0) ? 1 : 0;

	lBits = (long)m_pUnk10;
	if (m_dwUnk34 != 0)
		m_dwUnk54 = (DWORD)((lBottom & ~3L) + lBits);
	else
		m_dwUnk54 = (DWORD)((lBottom & ~3L) + lBits + 4);

	m_dwUnk4C = (DWORD)(lBits + lBottom);
	m_dwUnk58 = (DWORD)((lTop & ~3L) + lBits);
	m_dwUnk50 = (DWORD)(lBits + lTop - 4);

	m_dwUnk40 = m_dwUnk24 * (DWORD)lHeightMinusRight;
	m_dwUnk3C = m_dwUnk28 * (DWORD)lHeightMinusRight;
}

// The renderer's camera fields are 16.16 fixed point and hover.exe reads
// their halves out of memory (a word store for the integer part, a movsx
// for the read back); a union with a short high member reproduces that,
// where shifting the dword does not (notes/perfect-match.md rule 46).

typedef union tagCAMFIXED
{
	DWORD dw;
	struct
	{
		WORD wFrac;
		short sInt;
	} s;
} CAMFIXED;

// Sets up the renderer viewport/scaling state from the supplied rectangle,
// the owning DC and the view's offscreen render target.
//
// The four dword camera fields inside m_awUnk006C are 16.16 fixed point and
// are built a word at a time: the width goes into the high half of the
// dword at +0x70 and the height into the high half of the one at +0x6C, so
// halving them gives the half-extents at +0x78 (x) and +0x74 (y) without a
// shift. The projection base at +0x7C is the width scaled by the zoom
// factor m_dwUnk00C8; the divisor at +0x80 is the world size (0x5A00.0000)
// over it, and the base is then recomputed as the world size over the
// divisor so the two are exact inverses of each other in 16.16. The
// viewport extent at +0x88 is the larger of the two half-extents after they
// have been scaled by the divisor and by 1.4199 (0x16b85), i.e. the radius
// of the circle that encloses the viewport.
//
// Every one of those updates goes through HOVER_FIXMULP/HOVER_FIXDIVPP,
// which take the destination by pointer, as the original does.

// hover.exe:0x40FEE0
// FUNCTION: HOVER 0x40fee0
void CSub422F50::Sub_40FEE0(CDC* pDC, CBumperView* pView, CBumperDoc* pDoc,
	DWORD dwLeft, DWORD dwTop, DWORD dwRight, DWORD dwBottom)
{
	CAMFIXED fxWorld;
	CAMFIXED fxY;
	DWORD dwScaledY;
	DWORD dwDiv;
	CAMFIXED fxX;
	DWORD dwScaledX;
	DWORD dwWidth;
	DWORD dwHeight;
	DWORD* pdw;
	short sExtent;
	POSITION pos;
	CGameObj* pObj;
	CSub406090* pRenderTarget;

	fxWorld.s.wFrac = 0;
	fxWorld.s.sInt = 0x5A00;

	// The four parameters are contiguous, and so are the four members: the
	// original copies them as one rectangle.
	*(RECT*)&m_dwUnk5C = *(RECT*)&dwLeft;

	// Build the 16.16 half-extents at +0x78 and +0x74. The width and the
	// height are stored as the integer part of the dwords at +0x70/+0x6C,
	// so the halves are a plain signed /2.
	m_awUnk006C[2] = 0;
	m_awUnk006C[3] = (short)(dwRight - dwLeft);
	dwWidth = *(DWORD*)&m_awUnk006C[2];
	*(DWORD*)&m_awUnk006C[6] = dwWidth;
	*(DWORD*)&m_awUnk006C[6] = (long)dwWidth / 2;

	m_awUnk006C[0] = 0;
	m_awUnk006C[1] = (short)(dwBottom - dwTop);
	dwHeight = *(DWORD*)&m_awUnk006C[0];
	*(DWORD*)&m_awUnk006C[4] = dwHeight;
	*(DWORD*)&m_awUnk006C[4] = (long)dwHeight / 2;

	// Projection base at +0x7C = width * zoom.
	pdw = (DWORD*)&m_awUnk006C[8];
	*pdw = dwWidth;
	HOVER_FIXMULPP(pdw, m_dwUnk00C8);

	// Divisor at +0x80 = worldSize / base, then base = worldSize / divisor.
	pdw = (DWORD*)&m_wUnk0080;
	*pdw = fxWorld.dw;
	HOVER_FIXDIVPP(pdw, *(DWORD*)&m_awUnk006C[8]);

	pdw = (DWORD*)&m_awUnk006C[8];
	*pdw = fxWorld.dw;
	HOVER_FIXDIVPP(pdw, *(DWORD*)&m_wUnk0080);

	// The enclosing radius: the larger scaled half-extent.
	dwScaledX = *(DWORD*)&m_awUnk006C[6];
	dwDiv = *(DWORD*)&m_wUnk0080;
	HOVER_FIXMULPP(&dwScaledX, dwDiv);
	fxX.dw = dwScaledX;
	HOVER_FIXMULPP(&fxX.dw, 0x16b85);

	dwScaledY = *(DWORD*)&m_awUnk006C[4];
	HOVER_FIXMULPP(&dwScaledY, dwDiv);
	fxY.dw = dwScaledY;
	HOVER_FIXMULPP(&fxY.dw, 0x16b85);

	sExtent = fxX.s.sInt;
	if (fxY.s.sInt > sExtent)
		sExtent = fxY.s.sInt;
	m_wUnk0088 = sExtent;

	// Set up the render target bits and the span clip state.
	pRenderTarget = pView->m_pUnk00D0;
	m_pUnk0084 = pDC;
	Sub_40FB00(pRenderTarget->m_dwUnk04, pRenderTarget->m_dwWidth,
		pRenderTarget->m_dwHeight);
	Sub_40FBB0(dwLeft, dwTop, dwRight, dwBottom);

	// Clear the per-object flag in the doc's static object list.
	for (pos = theApp.m_pBumperDoc->m_list838C.GetHeadPosition(); pos != NULL; )
	{
		pObj = (CGameObj*)theApp.m_pBumperDoc->m_list838C.GetNext(pos);
		pObj->m_dwUnk3C = 0;
	}

	// Resolve the draw colors through the game palette.
	m_dwUnk008C = g_paletteUnk_4A1D50.GetNearestPaletteIndex(0x000000FF);
	m_dwUnk0090 = g_paletteUnk_4A1D50.GetNearestPaletteIndex(0x00FF0000);
	m_dwUnk0094 = g_paletteUnk_4A1D50.GetNearestPaletteIndex(0x00009200);
	m_dwUnk0098 = g_paletteUnk_4A1D50.GetNearestPaletteIndex(0x0000FFFF);
	m_dwUnk009C = g_paletteUnk_4A1D50.GetNearestPaletteIndex(0x00FFFFFF);
	m_dwUnk00A0 = g_paletteUnk_4A1D50.GetNearestPaletteIndex(0x00000000);
	m_dwUnk00A4 = g_paletteUnk_4A1D50.GetNearestPaletteIndex(0x00A4A4A4);
	m_dwUnk00A8 = g_paletteUnk_4A1D50.GetNearestPaletteIndex(0x00808080);
}

// FUNCTION: CBumperView::Sub_427BB0
// hover.exe:0x427BB0
// FUNCTION: HOVER 0x427bb0
void CBumperView::Sub_427BB0()
{
	int i;
	PALETTEENTRY pe;
	BYTE* pColor;
	DWORD adwMsg[3];
	BITMAPINFO* pBmi;

	if (theApp.m_dwUnk03E4 == 0)
		return;
	if (theApp.m_dwUnk03E8 != 0)
		return;

	// The original caches theApp.m_pUnk03EC in a pointer local across this
	// run of field writes.
	pBmi = theApp.m_pUnk03EC;

	// The original writes the BITMAPINFOHEADER fields in this order.
	pBmi->bmiHeader.biPlanes = 1;
	pBmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	pBmi->bmiHeader.biCompression = BI_RGB;
	pBmi->bmiHeader.biWidth = 320;
	pBmi->bmiHeader.biXPelsPerMeter = 0;
	pBmi->bmiHeader.biYPelsPerMeter = 0;
	pBmi->bmiHeader.biHeight = 240;

	// The original computes the color-table pointer here, between
	// biHeight and the remaining header fields.
	pColor = (BYTE*)&pBmi->bmiColors[0];

	pBmi->bmiHeader.biClrImportant = 0;
	pBmi->bmiHeader.biSizeImage = 320 * 240;
	pBmi->bmiHeader.biClrUsed = 256;
	pBmi->bmiHeader.biBitCount = 8;

	for (i = 0; i < 256; i++)
	{
		::GetPaletteEntries((HPALETTE)g_paletteUnk_4A1D50.m_hObject, i, 1, &pe);
		pColor += 4;
		pColor[-2] = pe.peRed;
		pColor[-3] = pe.peGreen;
		pColor[-4] = pe.peBlue;
		pColor[-1] = pe.peFlags;
	}

	adwMsg[0] = 0x400;
	adwMsg[1] = 0x428;
	if (theApp.m_pUnk03EC == NULL)
		adwMsg[1] = 0;
	adwMsg[2] = (DWORD)theApp.m_pUnk03EC;

	if (::SendMessageA((HWND)theApp.m_dwUnk03F0, 0x4a, 0, (LPARAM)adwMsg) != 0)
		return;
	if (::SendMessageA((HWND)theApp.m_dwUnk03F0, 0x403, 0, 0) != 0)
		return;
	theApp.m_dwUnk03E8 = 1;
}

// FUNCTION: CBumperView::Sub_427CC0
// hover.exe:0x427CC0
// FUNCTION: HOVER 0x427cc0
void CBumperView::Sub_427CC0()
{
	if (theApp.m_dwUnk03E8 != 0)
	{
		if (::SendMessageA((HWND)theApp.m_dwUnk03F0, 0x404, 0, 0) == 0)
			theApp.m_dwUnk03E8 = 0;
	}
}

// Marks the view inactive (m_dwUnk0500 = 1) when the game state allows a
// transition (bit 0x80000000 of theApp.m_dwUnk01B8 clear, state > 1) and, on
// an actual transition with dwUnk nonzero, sets m_dwUnk0080, calls
// Sub_40DC20(1) and forces a full window redraw.

// hover.exe:0x40EE90
// FUNCTION: HOVER 0x40ee90
void CBumperView::Sub_40EE90(DWORD dwUnk)
{
	BOOL bRedraw;

	if (this == NULL || g_dwUnk_4C4CE0 != 0)
		return;
	if (m_dwUnk0060 != 0)
		Sub_410280();
	if ((theApp.m_dwUnk01B8 & 0x80000000) != 0)
		return;
	if (theApp.m_dwUnk01B8 == 0 || theApp.m_dwUnk01B8 == 1)
		return;
	theApp.m_dwUnk01B8 |= 0x80000000;
	bRedraw = (m_dwUnk0500 != 1 && dwUnk != 0);
	m_dwUnk0500 = 1;
	if (bRedraw)
	{
		m_dwUnk0080 = 1;
		Sub_40DC20(1);
		RedrawWindow();
	}
}

// hover.exe:0x410280
// FUNCTION: HOVER 0x410280
void CBumperView::Sub_410280()
{
	CMenu* pMenu;

	if (m_dwUnk0060 != 0)
		timeKillEvent(m_dwUnk0060);
	m_dwUnk0060 = 0;
	Sub_4279E0();
	if (theApp.m_dwUnk0404 != 0)
		((CSub40B800*)theApp.m_pBumperDoc->m_adwUnk83EC[0x1A])->Sub_4118A0(5, 1);
	if (theApp.m_dwUnk040C != 0)
		theApp.m_pBumperDoc->m_obj0060.Sub_412220();
	Hover_sub_40E560(0);
	timeKillEvent(m_dwUnk0060);
	if (theApp.m_dwUnk03B0 != 0)
	{
		if (theApp.m_dwUnk03E8 != 0)
		{
			Sub_4278E0(1);
		}
		else
		{
			pMenu = CMenu::FromHandle((HMENU)theApp.m_adwUnk03C0[6]);
			::SetMenu(AfxGetApp()->m_pMainWnd->m_hWnd,
				(pMenu != NULL) ? pMenu->m_hMenu : NULL);
		}
	}
	if (m_dwUnk0034 == 0)
	{
		ShowCursor(TRUE);
		m_dwUnk0034 = 1;
	}
	SetThreadPriority((HANDLE)theApp.m_hThread, THREAD_PRIORITY_NORMAL);
	if (m_dwUnk0040 != 0)
		SetThreadPriority(((CWinThread*)m_dwUnk0040)->m_hThread,
			THREAD_PRIORITY_NORMAL);
	if (g_pThreadUnk_4C56E0 != NULL)
		SetThreadPriority(g_pThreadUnk_4C56E0->m_hThread,
			THREAD_PRIORITY_NORMAL);
}

// Waits (non-blocking) on the six synchronization events, pumps
// m_hUnk0054 through Hover_sub_427A10 when it stays signaled, then
// transitions m_dwUnk00AC to state 3 under theApp.m_csUnk011C. When nUnk is
// nonzero the m_hUnk0050/m_hUnk0054 handshake pair runs around the
// ResetEvent/SetEvent pulse on m_hUnk0054/m_dwUnk0048.

// hover.exe:0x40DC20
// FUNCTION: HOVER 0x40dc20
void CBumperView::Sub_40DC20(int nUnk)
{
	WaitForSingleObject((HANDLE)m_dwUnk0048, 0);
	WaitForSingleObject(m_hUnk004C, 0);
	WaitForSingleObject(m_hUnk0050, 0);
	WaitForSingleObject(m_hUnk0054, 0);
	WaitForSingleObject(m_hUnk0058, 0);
	WaitForSingleObject(m_hUnk005C, 0);
	if (WaitForSingleObject(m_hUnk0054, 0) == WAIT_TIMEOUT &&
		m_dwUnk00AC != 3)
		Hover_sub_427A10(m_hUnk0054);
	EnterCriticalSection(&theApp.m_csUnk011C);
	m_dwUnk00AC = 3;
	LeaveCriticalSection(&theApp.m_csUnk011C);
	if (nUnk != 0)
	{
		Hover_sub_427A10(m_hUnk0050);
		EnterCriticalSection(&theApp.m_csUnk011C);
		ResetEvent(m_hUnk0054);
		SetEvent((HANDLE)m_dwUnk0048);
		LeaveCriticalSection(&theApp.m_csUnk011C);
		Hover_sub_427A10(m_hUnk0054);
		Hover_sub_427A10(m_hUnk0050);
	}
	else
	{
		EnterCriticalSection(&theApp.m_csUnk011C);
		ResetEvent(m_hUnk0054);
		SetEvent((HANDLE)m_dwUnk0048);
		LeaveCriticalSection(&theApp.m_csUnk011C);
	}
}

// Sets the render state m_dwUnk0500. State 0 (the "no game" state) also
// invalidates the cached colour at +0xA0. When the state actually changed
// and nRedraw is set, the worker thread is pushed through Sub_40DC20 and
// the window is repainted synchronously.

// hover.exe:0x41F4E0
// FUNCTION: HOVER 0x41f4e0
void CBumperView::Sub_41F4E0(DWORD dwState, int nRedraw)
{
	BOOL bRedraw;

	bRedraw = (m_dwUnk0500 != dwState && nRedraw != 0);
	m_dwUnk0500 = dwState;
	if (dwState == 0)
		m_dwUnk00A0 = 0xFFFFFFFF;
	if (bRedraw)
	{
		m_dwUnk0080 = 1;
		Sub_40DC20(1);
		RedrawWindow();
	}
}

// Same sequence as Sub_40DC20, but transitions m_dwUnk00AC to state 6.

// hover.exe:0x4277E0
// FUNCTION: HOVER 0x4277e0
void CBumperView::Sub_4277E0(int nUnk)
{
	WaitForSingleObject((HANDLE)m_dwUnk0048, 0);
	WaitForSingleObject(m_hUnk004C, 0);
	WaitForSingleObject(m_hUnk0050, 0);
	WaitForSingleObject(m_hUnk0054, 0);
	WaitForSingleObject(m_hUnk0058, 0);
	WaitForSingleObject(m_hUnk005C, 0);
	if (WaitForSingleObject(m_hUnk0054, 0) == WAIT_TIMEOUT &&
		m_dwUnk00AC != 3)
		Hover_sub_427A10(m_hUnk0054);
	EnterCriticalSection(&theApp.m_csUnk011C);
	m_dwUnk00AC = 6;
	LeaveCriticalSection(&theApp.m_csUnk011C);
	if (nUnk != 0)
	{
		Hover_sub_427A10(m_hUnk0050);
		EnterCriticalSection(&theApp.m_csUnk011C);
		ResetEvent(m_hUnk0054);
		SetEvent((HANDLE)m_dwUnk0048);
		LeaveCriticalSection(&theApp.m_csUnk011C);
		Hover_sub_427A10(m_hUnk0054);
		Hover_sub_427A10(m_hUnk0050);
	}
	else
	{
		EnterCriticalSection(&theApp.m_csUnk011C);
		ResetEvent(m_hUnk0054);
		SetEvent((HANDLE)m_dwUnk0048);
		LeaveCriticalSection(&theApp.m_csUnk011C);
	}
}

// Same sequence as Sub_40DC20, but transitions m_dwUnk00AC to state 7.

// hover.exe:0x4278E0
// FUNCTION: HOVER 0x4278e0
void CBumperView::Sub_4278E0(int nUnk)
{
	WaitForSingleObject((HANDLE)m_dwUnk0048, 0);
	WaitForSingleObject(m_hUnk004C, 0);
	WaitForSingleObject(m_hUnk0050, 0);
	WaitForSingleObject(m_hUnk0054, 0);
	WaitForSingleObject(m_hUnk0058, 0);
	WaitForSingleObject(m_hUnk005C, 0);
	if (WaitForSingleObject(m_hUnk0054, 0) == WAIT_TIMEOUT &&
		m_dwUnk00AC != 3)
		Hover_sub_427A10(m_hUnk0054);
	EnterCriticalSection(&theApp.m_csUnk011C);
	m_dwUnk00AC = 7;
	LeaveCriticalSection(&theApp.m_csUnk011C);
	if (nUnk != 0)
	{
		Hover_sub_427A10(m_hUnk0050);
		EnterCriticalSection(&theApp.m_csUnk011C);
		ResetEvent(m_hUnk0054);
		SetEvent((HANDLE)m_dwUnk0048);
		LeaveCriticalSection(&theApp.m_csUnk011C);
		Hover_sub_427A10(m_hUnk0054);
		Hover_sub_427A10(m_hUnk0050);
	}
	else
	{
		EnterCriticalSection(&theApp.m_csUnk011C);
		ResetEvent(m_hUnk0054);
		SetEvent((HANDLE)m_dwUnk0048);
		LeaveCriticalSection(&theApp.m_csUnk011C);
	}
}

// Same sequence as Sub_40DC20, but transitions m_dwUnk00AC to state 1 and
// publishes the new speed/detail level in theApp.m_dwUnk01C4 while the
// worker thread is parked inside the critical section. Called by the
// "Customize Game" dialog when the speed/detail slider moved.

// hover.exe:0x413A00
// FUNCTION: HOVER 0x413a00
void CBumperView::Sub_413A00(int nLevel, int nUnk)
{
	WaitForSingleObject((HANDLE)m_dwUnk0048, 0);
	WaitForSingleObject(m_hUnk004C, 0);
	WaitForSingleObject(m_hUnk0050, 0);
	WaitForSingleObject(m_hUnk0054, 0);
	WaitForSingleObject(m_hUnk0058, 0);
	WaitForSingleObject(m_hUnk005C, 0);
	if (WaitForSingleObject(m_hUnk0054, 0) == WAIT_TIMEOUT &&
		m_dwUnk00AC != 1)
		Hover_sub_427A10(m_hUnk0054);
	EnterCriticalSection(&theApp.m_csUnk011C);
	theApp.m_dwUnk01C4 = nLevel;
	m_dwUnk00AC = 1;
	LeaveCriticalSection(&theApp.m_csUnk011C);
	if (nUnk != 0)
	{
		Hover_sub_427A10(m_hUnk0050);
		EnterCriticalSection(&theApp.m_csUnk011C);
		ResetEvent(m_hUnk0054);
		SetEvent((HANDLE)m_dwUnk0048);
		LeaveCriticalSection(&theApp.m_csUnk011C);
		Hover_sub_427A10(m_hUnk0054);
		Hover_sub_427A10(m_hUnk0050);
	}
	else
	{
		EnterCriticalSection(&theApp.m_csUnk011C);
		ResetEvent(m_hUnk0054);
		SetEvent((HANDLE)m_dwUnk0048);
		LeaveCriticalSection(&theApp.m_csUnk011C);
	}
}

// Same sequence as Sub_40DC20, but transitions m_dwUnk00AC to state 4.
// Run after the player control bindings changed so the worker thread picks
// up the new ones.

// hover.exe:0x413B10
// FUNCTION: HOVER 0x413b10
void CBumperView::Sub_413B10(int nUnk)
{
	WaitForSingleObject((HANDLE)m_dwUnk0048, 0);
	WaitForSingleObject(m_hUnk004C, 0);
	WaitForSingleObject(m_hUnk0050, 0);
	WaitForSingleObject(m_hUnk0054, 0);
	WaitForSingleObject(m_hUnk0058, 0);
	WaitForSingleObject(m_hUnk005C, 0);
	if (WaitForSingleObject(m_hUnk0054, 0) == WAIT_TIMEOUT &&
		m_dwUnk00AC != 4)
		Hover_sub_427A10(m_hUnk0054);
	EnterCriticalSection(&theApp.m_csUnk011C);
	m_dwUnk00AC = 4;
	LeaveCriticalSection(&theApp.m_csUnk011C);
	if (nUnk != 0)
	{
		Hover_sub_427A10(m_hUnk0050);
		EnterCriticalSection(&theApp.m_csUnk011C);
		ResetEvent(m_hUnk0054);
		SetEvent((HANDLE)m_dwUnk0048);
		LeaveCriticalSection(&theApp.m_csUnk011C);
		Hover_sub_427A10(m_hUnk0054);
		Hover_sub_427A10(m_hUnk0050);
	}
	else
	{
		EnterCriticalSection(&theApp.m_csUnk011C);
		ResetEvent(m_hUnk0054);
		SetEvent((HANDLE)m_dwUnk0048);
		LeaveCriticalSection(&theApp.m_csUnk011C);
	}
}

// hover.exe:0x4279E0
// FUNCTION: HOVER 0x4279e0
void CBumperView::Sub_4279E0()
{
	ResetEvent((HANDLE)m_dwUnk0048);
	Hover_sub_427A10(m_hUnk0050);
	SetEvent(m_hUnk0054);
}

// Re-establishes the offscreen surfaces for the current client size and
// restarts the render thread. Called by CMainFrame::OnSize and
// CMainFrame::OnExitSizeMove once the frame has settled on its new size.
// hover.exe:0x41F480
// FUNCTION: HOVER 0x41f480
void CBumperView::Sub_41F480()
{
	RECT rect;
	int cx;
	int cy;

	if (m_dwUnk0084 == 0)
		return;

	::GetClientRect(m_hWnd, &rect);
	Sub_4279E0();
	cy = rect.bottom - rect.top;
	cx = rect.right - rect.left;
	Sub_4274B0(cx, cy, 1);
	Sub_40DC20(1);
}

// hover.exe:0x40C480
// FUNCTION: HOVER 0x40c480
void CBumperView::OnDraw(CDC* pDC)
{
	RECT rect;
	CPalette* pOldPal;
	int cx;
	int cy;

	if (theApp.m_dwUnk03E8 != 0)
		return;
	if (m_dwUnk0084 == 0)
		return;
	if (m_dwUnk0040 == 0)
		return;
	pOldPal = pDC->SelectPalette(&g_paletteUnk_4A1D50, TRUE);
	pDC->RealizePalette();
	::GetClientRect(m_hWnd, &rect);
	if (m_dwUnk0060 == 0 &&
		WaitForSingleObject(m_hUnk0050, 0) == WAIT_TIMEOUT)
	{
		m_dwUnk0080 = 1;
	}
	else
	{
		EnterCriticalSection(&theApp.m_csUnk00EC);
		cx = (int)m_dwUnk008C;
		cy = (int)m_dwUnk0090;
		if (m_dwUnk0544 != 0)
			StretchBlt(pDC->m_hDC, 0, 0, cx * 2, cy * 2,
				m_dcUnk00B4.GetSafeHdc(), 0, 0, cx, cy, SRCCOPY);
		else
			BitBlt(pDC->m_hDC, 0, 0, cx, cy,
				m_dcUnk00B4.GetSafeHdc(), 0, 0, SRCCOPY);
		LeaveCriticalSection(&theApp.m_csUnk00EC);
	}
	pDC->SelectPalette(pOldPal, TRUE);
}

// Same handshake sequence as Sub_40DC20, but transitions m_dwUnk00AC to
// state 5 and records nUnk in m_dwUnk00B0 (the requested view scale).

// hover.exe:0x4275D0
// FUNCTION: HOVER 0x4275d0
void CBumperView::Sub_4275D0(int nUnk, int nHandshake)
{
	WaitForSingleObject((HANDLE)m_dwUnk0048, 0);
	WaitForSingleObject(m_hUnk004C, 0);
	WaitForSingleObject(m_hUnk0050, 0);
	WaitForSingleObject(m_hUnk0054, 0);
	WaitForSingleObject(m_hUnk0058, 0);
	WaitForSingleObject(m_hUnk005C, 0);
	if (WaitForSingleObject(m_hUnk0054, 0) == WAIT_TIMEOUT &&
		m_dwUnk00AC != 5)
		Hover_sub_427A10(m_hUnk0054);
	EnterCriticalSection(&theApp.m_csUnk011C);
	m_dwUnk00B0 = nUnk;
	m_dwUnk00AC = 5;
	LeaveCriticalSection(&theApp.m_csUnk011C);
	if (nHandshake != 0)
	{
		Hover_sub_427A10(m_hUnk0050);
		EnterCriticalSection(&theApp.m_csUnk011C);
		ResetEvent(m_hUnk0054);
		SetEvent((HANDLE)m_dwUnk0048);
		LeaveCriticalSection(&theApp.m_csUnk011C);
		Hover_sub_427A10(m_hUnk0054);
		Hover_sub_427A10(m_hUnk0050);
	}
	else
	{
		EnterCriticalSection(&theApp.m_csUnk011C);
		ResetEvent(m_hUnk0054);
		SetEvent((HANDLE)m_dwUnk0048);
		LeaveCriticalSection(&theApp.m_csUnk011C);
	}
}

// Same sequence as Sub_40DC20, but transitions m_dwUnk00AC to state 2
// (the worker thread's "stop" state, used when the renderer is torn down).

// hover.exe:0x4276E0
// FUNCTION: HOVER 0x4276e0
void CBumperView::Sub_4276E0(int nUnk)
{
	WaitForSingleObject((HANDLE)m_dwUnk0048, 0);
	WaitForSingleObject(m_hUnk004C, 0);
	WaitForSingleObject(m_hUnk0050, 0);
	WaitForSingleObject(m_hUnk0054, 0);
	WaitForSingleObject(m_hUnk0058, 0);
	WaitForSingleObject(m_hUnk005C, 0);
	if (WaitForSingleObject(m_hUnk0054, 0) == WAIT_TIMEOUT &&
		m_dwUnk00AC != 2)
		Hover_sub_427A10(m_hUnk0054);
	EnterCriticalSection(&theApp.m_csUnk011C);
	m_dwUnk00AC = 2;
	LeaveCriticalSection(&theApp.m_csUnk011C);
	if (nUnk != 0)
	{
		Hover_sub_427A10(m_hUnk0050);
		EnterCriticalSection(&theApp.m_csUnk011C);
		ResetEvent(m_hUnk0054);
		SetEvent((HANDLE)m_dwUnk0048);
		LeaveCriticalSection(&theApp.m_csUnk011C);
		Hover_sub_427A10(m_hUnk0054);
		Hover_sub_427A10(m_hUnk0050);
	}
	else
	{
		EnterCriticalSection(&theApp.m_csUnk011C);
		ResetEvent(m_hUnk0054);
		SetEvent((HANDLE)m_dwUnk0048);
		LeaveCriticalSection(&theApp.m_csUnk011C);
	}
}

// Tears the renderer down: stop and delete the worker thread, close the six
// synchronization events, delete the two render contexts and the offscreen
// render target, and release the offscreen DC. Called from OnDestroy.

// hover.exe:0x41F350
// FUNCTION: HOVER 0x41f350
void CBumperView::Sub_41F350()
{
	CSub410F20* pCtx;
	CSub406090* pTarget;

	if (m_dwUnk0040 != 0)
	{
		Sub_4276E0(1);
		Hover_sub_427A10(((CWinThread*)m_dwUnk0040)->m_hThread);
		if (m_dwUnk0040 != 0)
			delete (CWinThread*)m_dwUnk0040;
		m_dwUnk0040 = 0;
	}
	if (m_dwUnk0048 != 0)
	{
		CloseHandle((HANDLE)m_dwUnk0048);
		CloseHandle(m_hUnk004C);
		CloseHandle(m_hUnk0050);
		CloseHandle(m_hUnk0054);
		CloseHandle(m_hUnk0058);
		CloseHandle(m_hUnk005C);
		m_dwUnk0048 = 0;
		m_hUnk004C = NULL;
		m_hUnk0050 = NULL;
		m_hUnk0054 = NULL;
		m_hUnk0058 = NULL;
		m_hUnk005C = NULL;
	}
	pCtx = m_pUnk00C8;
	if (pCtx != NULL)
		delete pCtx;
	pCtx = m_pUnk00CC;
	if (pCtx != NULL)
		delete pCtx;
	pTarget = m_pUnk00D0;
	if (pTarget != NULL)
		delete pTarget;
	m_pUnk00C8 = NULL;
	m_pUnk00CC = NULL;
	m_pUnk00D0 = NULL;
	::SelectObject(m_dcUnk00B4.GetSafeHdc(), (HGDIOBJ)m_dwUnk00C4);
	m_dcUnk00B4.DeleteDC();
}

// Reset the dashboard objects for a new game: the three digit displays and
// the score display are switched on with a zero value, and every gauge and
// sprite is rewound. The rewind idiom is the original's, quirk included -
// the segment index is first set to the segment count and only then zeroed
// when that count is positive.

// The dead store in the `if` arm is deliberate: it is what makes VC emit
// the store between the `test` and the `jle`, i.e. the original's exact
// instruction order (a plain `x = n; if (n > 0) x = 0;` puts the store
// first). See rule 22 in notes/perfect-match.md.
#define HOVER_RESET_GAUGE(obj)						\
	do {											\
		long _nSeg = (long)(obj).m_dwUnk1C;			\
		if (_nSeg > 0)								\
		{											\
			(obj).m_dwUnk24 = _nSeg;				\
			(obj).m_dwUnk24 = 0;					\
		}											\
		else										\
		{											\
			(obj).m_dwUnk24 = _nSeg;				\
		}											\
	} while (0)

#define HOVER_RESET_SPRITE(obj)						\
	do {											\
		long _nSpr = (long)(obj).m_dwUnk3C;			\
		if (_nSpr > 0)								\
		{											\
			(obj).m_dwUnk44 = _nSpr;				\
			(obj).m_dwUnk44 = 0;					\
		}											\
		else										\
		{											\
			(obj).m_dwUnk44 = _nSpr;				\
		}											\
	} while (0)

// hover.exe:0x41E5B0
// FUNCTION: HOVER 0x41e5b0
void CBumperView::Sub_41E5B0()
{
	m_obj0240.m_dwUnk00 = 1;
	m_obj0240.m_dwUnk08 = 0;
	m_obj0268.m_dwUnk00 = 1;
	m_obj0268.m_dwUnk08 = 0;
	m_obj0290.m_dwUnk00 = 1;
	m_obj0290.m_dwUnk08 = 0;
	HOVER_RESET_GAUGE(m_obj02E0);
	HOVER_RESET_GAUGE(m_obj0310);
	HOVER_RESET_GAUGE(m_obj0340);
	HOVER_RESET_GAUGE(m_obj0370);
	HOVER_RESET_GAUGE(m_obj03A0);
	HOVER_RESET_GAUGE(m_obj03D0);
	HOVER_RESET_GAUGE(m_obj0400);
	m_obj0430.m_dwUnk00 = 1;
	HOVER_RESET_SPRITE(m_obj0430);
	m_obj0480.m_dwUnk00 = 1;
	HOVER_RESET_SPRITE(m_obj0480);
	m_obj02B8.m_dwUnk00 = 1;
	theApp.m_pBumperDoc->m_dwUnk83E4 = 0;
	m_obj02B8.m_dwUnk08 = 0;
}

// Awards the end-of-level score. m_dwUnk0504 is the number of ticks left on
// the level clock (the level's tick allowance minus the elapsed ticks),
// m_dwUnk0508 the per-tick value for the current skill level and m_dwUnk050C
// their product; m_dwUnk0510 is the number of the level that follows and
// m_dwUnk0514 the flat level bonus, which is only awarded when bLevelBonus
// is set (the caller clears it when the level was restarted). Both go into
// the document's running total, which is then shown on the scoreboard.
//
// The original spells the LVLPARAM index expression out three times rather
// than computing it once.

// hover.exe:0x4282F0
// FUNCTION: HOVER 0x4282f0
void CBumperView::Sub_4282F0(DWORD bLevelBonus)
{
	int nLevel = (int)g_dwUnk_46049C;
	DWORD dwPerTick;

	m_dwUnk0504 = LVLPARAM(nLevel).m_dwUnk30
		- theApp.m_pBumperDoc->m_dwUnk83E8;
	dwPerTick = (&LVLPARAM(nLevel).m_dwUnk4C)[theApp.m_dwUnk01C8];
	m_dwUnk0508 = dwPerTick;
	m_dwUnk0510 = (DWORD)(nLevel + 2);
	m_dwUnk050C = dwPerTick * m_dwUnk0504;

	if (bLevelBonus == 0)
		m_dwUnk0514 = 0;
	else
		m_dwUnk0514 = (&LVLPARAM(nLevel).m_dwUnk34)[theApp.m_dwUnk01C8];

	theApp.m_pBumperDoc->m_dwUnk83E4 += m_dwUnk0514 + m_dwUnk050C;
	m_obj02B8.m_dwUnk00 = 1;
	m_obj02B8.m_dwUnk08 = theApp.m_pBumperDoc->m_dwUnk83E4;
}

// The two per-frame steps run from the WM_USER+0 handler (the multimedia
// timer posts it twenty times a second): Sub_408D00 advances the game, and
// Sub_408DF0 refreshes the two debug panes of the status bar.

// The timed game states run their countdown in m_dwUnk007C here. State 2 is
// the pre-game countdown - the "get set" sound fires at tick 40 and the
// state flips to 3 (playing) at 0, restarting the level music; states 4 and
// 5 wait out the end-of-game / end-of-level delay, stop the music and post
// WM_USER+1 / WM_USER+2 so the high-score or next-level flow runs outside
// the timer callback. State 2 falls through into the state 3 step, so the
// countdown frames are played as usual.

// hover.exe:0x408D00
// FUNCTION: HOVER 0x408d00
void CBumperView::Sub_408D00()
{
	switch (theApp.m_dwUnk01B8)
	{
	case 2:
		if (m_dwUnk007C == 0x28 && theApp.m_dwUnk0408 != 0)
			((CSub40B800*)theApp.m_pBumperDoc->m_adwUnk83EC[1])->Sub_40B800(0, 5, 0);
		if (--m_dwUnk007C == 0)
		{
			theApp.m_dwUnk01B8 = 3;
			m_dwUnk007C = 0x3C;
			if (theApp.m_dwUnk040C != 0)
				theApp.m_pBumperDoc->m_obj0060.Sub_413710();
		}
		// falls through into the state 3 step

	case 3:
		Sub_427D40();
		break;

	case 4:
		if (m_dwUnk007C == 0x3C)
			theApp.m_pBumperDoc->m_obj0060.Sub_412220();
		if (--m_dwUnk007C == 0)
		{
			Sub_410280();
			::PostMessage(m_hWnd, WM_USER + 1, 0, 0);
		}
		break;

	case 5:
		if (m_dwUnk007C == 0x3C)
			theApp.m_pBumperDoc->m_obj0060.Sub_412220();
		if (--m_dwUnk007C == 0)
		{
			Sub_410280();
			::PostMessage(m_hWnd, WM_USER + 2, 0, 0);
		}
		break;
	}
}

// Refresh the two debug panes of the status bar, at most once every 58
// ticks (~3 s) and only while a game is running - m_dwUnk01B8's top bit
// marks the suspended states, and states 0 and 1 have no game yet.
//
// Pane 1 gets "<present rate>/<step rate>", both averaged over the frames
// counted since the last refresh: m_dwUnk052C/m_dwUnk0530 are the tick and
// frame counter of CBumperView::Sub_407430 (the present), m_dwUnk0538/
// m_dwUnk053C those of Sub_427D40 (the game step). The rates are formatted
// by hand with _fcvt (one fractional digit) rather than with sprintf.
// Pane 0 gets the human player's position and heading.

// hover.exe:0x408DF0
// FUNCTION: HOVER 0x408df0
void CBumperView::Sub_408DF0()
{
	CBumperDoc* pDoc;
	DWORD dwState;
	int nTicks;
	DWORD dwFrames;
	DWORD dwTick;
	DWORD dwMsPerFrame;
	DWORD dwMsPerStep;
	double dblFrameRate;
	double dblStepRate;
	char* pszDigits;
	char* psz;
	int nDecFrame;
	int nDecStep;
	int nSignFrame;
	int nSignStep;
	DWORD adwPos[2];
	char szText[0x2C];

	pDoc = (CBumperDoc*)m_pDocument;
	dwState = theApp.m_dwUnk01B8 & 0x7FFFFFFF;
	if (dwState == 0 || dwState == 1 ||
		(theApp.m_dwUnk01B8 & 0x80000000) != 0)
		return;
	nTicks = g_nUnk_4C4CEC;
	g_nUnk_4C4CEC = nTicks - 1;
	if (nTicks > 0)
		return;

	dwFrames = m_dwUnk0530;
	if (dwFrames == 0)
		return;
	if (m_dwUnk053C == 0)
		return;

	dwTick = m_dwUnk052C;
	m_dwUnk0530 = 0;
	dwMsPerFrame = (dwTick - m_dwUnk0528) / dwFrames;
	m_dwUnk0528 = dwTick;
	dwTick = m_dwUnk0538;
	dblFrameRate = 1000.0 / dwMsPerFrame;
	dwMsPerStep = (dwTick - m_dwUnk0534) / m_dwUnk053C;
	m_dwUnk0534 = dwTick;
	g_nUnk_4C4CEC = 0x39;
	m_dwUnk053C = 0;
	dblStepRate = 1000.0 / dwMsPerStep;

	// "<frame rate>/<step rate>", one digit after each decimal point.
	pszDigits = _fcvt(dblFrameRate, 1, &nDecFrame, &nSignFrame);
	if (nDecFrame > 0)
	{
		strncpy(szText, pszDigits, nDecFrame);
		pszDigits += nDecFrame;
		psz = szText + nDecFrame;
	}
	else
	{
		szText[0] = '0';
		psz = szText + 1;
	}
	*psz++ = '.';
	while (nDecFrame++ < 0)
		*psz++ = '0';
	*psz++ = *pszDigits;
	*psz++ = '/';

	pszDigits = _fcvt(dblStepRate, 1, &nDecStep, &nSignStep);
	if (nDecStep > 0)
	{
		strncpy(psz, pszDigits, nDecStep);
		psz += nDecStep;
		pszDigits += nDecStep;
	}
	else
	{
		*psz++ = '0';
	}
	*psz++ = '.';
	while (nDecStep++ < 0)
		*psz++ = '0';
	psz[0] = *pszDigits;
	psz[1] = '\0';
	((CMainFrame*)AfxGetApp()->m_pMainWnd)->Sub_408A70(1, szText);

	// "X:<x>, Y:<y>, A:<heading>" - the position words come back from
	// CGameObj::Sub_403880 sign-extended, the heading is read directly.
	strcpy(szText, "X:");
	_ltoa((long)pDoc->m_humanPlayer.Sub_403880(adwPos)[0], &szText[2], 10);
	strcat(&szText[2], ", Y:");
	psz = &szText[2] + strlen(&szText[2]);
	_ltoa((long)pDoc->m_humanPlayer.Sub_403880(adwPos)[1], psz, 10);
	strcat(psz, ", A:");
	psz += strlen(psz);
	_ltoa((long)(short)pDoc->m_humanPlayer.m_awUnk0194[5], psz, 10);
	((CMainFrame*)AfxGetApp()->m_pMainWnd)->Sub_408A70(0, szText);
}

// The game step proper. While the renderer is in state 3 the worker thread
// is released for one frame, then every object in the document's second
// list gets its input/think virtual, the per-frame renderer counters are
// cleared and the collider runs its pass. Finally the newly pressed
// joystick buttons are translated into WM_KEYDOWNs for the keys they are
// bound to and the step is timed for the status bar.

// hover.exe:0x427D40
// FUNCTION: HOVER 0x427d40
void CBumperView::Sub_427D40()
{
	DWORD dwPressed;

	if (m_dwUnk00AC == 3)
	{
		EnterCriticalSection(&theApp.m_csUnk011C);
		ResetEvent((HANDLE)m_dwUnk0048);
		SetEvent(m_hUnk0054);
		LeaveCriticalSection(&theApp.m_csUnk011C);
	}

	Sub_427E50();
	Sub_427E90();

	EnterCriticalSection(&theApp.m_csUnk00D4);
	Hover_sub_408780();
	LeaveCriticalSection(&theApp.m_csUnk00D4);

	Sub_40DC20(0);

	dwPressed = 0;
	if (theApp.m_avkControls[11] != 0)
	{
		dwPressed = ~g_dwUnk_4C4CF0 & theApp.m_adwUnk0378[8];
		g_dwUnk_4C4CF0 = theApp.m_adwUnk0378[8];
	}
	if ((dwPressed & 8) != 0)
		::SendMessage(m_hWnd, WM_KEYDOWN, theApp.m_avkControls[1], 0);
	if ((dwPressed & 6) != 0)
		::SendMessage(m_hWnd, WM_KEYDOWN, theApp.m_avkControls[0], 0);
	if ((dwPressed & 1) != 0)
		::SendMessage(m_hWnd, WM_KEYDOWN, theApp.m_avkControls[2], 0);

	m_dwUnk0538 = GetTickCount();
	m_dwUnk053C++;
}

// Run the input/think virtual (vtable slot 7) on every object in the
// document's second object list.

// hover.exe:0x427E50
// FUNCTION: HOVER 0x427e50
void CBumperView::Sub_427E50()
{
	CBumperDoc* pDoc;
	POSITION pos;

	pDoc = (CBumperDoc*)m_pDocument;
	pos = pDoc->m_list83A8.GetHeadPosition();
	while (pos != NULL)
		((CGameObj*)pDoc->m_list83A8.GetNext(pos))->Sub_42AD30();
}

// Clear the four renderer debug counters shown by the ID_DBG_* update
// handler and hand the frame to the collider.

// hover.exe:0x427E90
// FUNCTION: HOVER 0x427e90
void CBumperView::Sub_427E90()
{
	CBumperDoc* pDoc;
	DWORD* pdwCounters;

	pDoc = (CBumperDoc*)m_pDocument;
	pdwCounters = &m_dwUnk0518;
	pdwCounters[0] = 0;
	pdwCounters[1] = 0;
	pdwCounters[2] = 0;
	pdwCounters[3] = 0;
	((CCollider*)pDoc->m_dwUnk8388)->Sub_40A650();
}

/////////////////////////////////////////////////////////////////////////////
// CBumperView message handlers
//
// The original's message map (AFX_MSGMAP_ENTRY table at hover.exe:0x4BB0C8,
// AFX_MSGMAP at 0x4BB0C0, base CView) is reproduced entry for entry, in the
// original order, by the BEGIN_MESSAGE_MAP block at the top of this file.

// The original module was compiled with automatic inlining on, so
// CBumperView::Sub_40EE90(1) - the "suspend the game loop" transition, which
// also exists as an out-of-line function at hover.exe:0x40EE90 because
// mainfrm.cpp calls it - is expanded inline into the five handlers below.
// This macro reproduces that expansion; the early exits are `break`s of the
// do/while because in the original they only skip the inlined block, not the
// rest of the handler.
#define HOVER_INLINE_SUB_40EE90_1()									\
	do																\
	{																\
		BOOL bSuspended_;											\
																	\
		if (this == NULL || g_dwUnk_4C4CE0 != 0)					\
			break;													\
		if (m_dwUnk0060 != 0)										\
			Sub_410280();											\
		if ((theApp.m_dwUnk01B8 & 0x80000000) != 0)					\
			break;													\
		if (theApp.m_dwUnk01B8 == 0 || theApp.m_dwUnk01B8 == 1)		\
			break;													\
		theApp.m_dwUnk01B8 |= 0x80000000;							\
		bSuspended_ = (m_dwUnk0500 != 1);							\
		m_dwUnk0500 = 1;											\
		if (bSuspended_)											\
		{															\
			m_dwUnk0080 = 1;										\
			Sub_40DC20(1);											\
			RedrawWindow();											\
		}															\
	} while (0)

// Entry 1: WM_DESTROY. Stops the frame timer, tears the renderer down and
// lets CView do the rest.

// hover.exe:0x41E710
// FUNCTION: HOVER 0x41e710
void CBumperView::OnDestroy()
{
	if (m_dwUnk0060 != 0)
		Sub_410280();
	Sub_41F350();
	CView::OnDestroy();
}

// Entry 2: WM_SIZE. Suspends the game loop, then shows the new client width
// in pane 2 of the main frame's status bar.

// hover.exe:0x4183E0
// FUNCTION: HOVER 0x4183e0
void CBumperView::OnSize(UINT nType, int cx, int cy)
{
	char szText[0x50];

	HOVER_INLINE_SUB_40EE90_1();

	Default();
	if (nType == SIZE_RESTORED || nType == SIZE_MAXIMIZED)
	{
		wsprintfA(szText, "%4d", cx);
		((CMainFrame*)GetParentFrame())->Sub_408A70(2, szText);
	}
}

// Entries 3, 37, 38 and 39: the four ID_DBG_* renderer debug flags share one
// update handler. The flags live in four consecutive members starting at
// m_dwUnk0518, indexed by the command ID.

// hover.exe:0x402610
// FUNCTION: HOVER 0x402610
void CBumperView::OnUpdateDbgFlag(CCmdUI* pCmdUI)
{
	pCmdUI->Enable((&m_dwUnk0518)[pCmdUI->m_nID - ID_DBG_WALL]);
}

// Entries 4/5: ID_CMD_34000 toggles the human player's +0x1DC flag.

// hover.exe:0x427EC0
// FUNCTION: HOVER 0x427ec0
void CBumperView::OnCmd34000()
{
	((CBumperDoc*)m_pDocument)->m_humanPlayer.m_dwUnk01DC =
		(((CBumperDoc*)m_pDocument)->m_humanPlayer.m_dwUnk01DC == 0);
}

// hover.exe:0x427EE0
// FUNCTION: HOVER 0x427ee0
void CBumperView::OnUpdateCmd34000(CCmdUI* pCmdUI)
{
	CBumperDoc* pDoc = (CBumperDoc*)m_pDocument;

	pCmdUI->SetCheck(pDoc->m_humanPlayer.m_dwUnk01DC);
}

// Entries 6/7: ID_CMD_34011 toggles the human player's +0x1E0 flag.

// hover.exe:0x427F00
// FUNCTION: HOVER 0x427f00
void CBumperView::OnCmd34011()
{
	((CBumperDoc*)m_pDocument)->m_humanPlayer.m_dwUnk01E0 =
		(((CBumperDoc*)m_pDocument)->m_humanPlayer.m_dwUnk01E0 == 0);
}

// hover.exe:0x427F20
// FUNCTION: HOVER 0x427f20
void CBumperView::OnUpdateCmd34011(CCmdUI* pCmdUI)
{
	CBumperDoc* pDoc = (CBumperDoc*)m_pDocument;

	pCmdUI->SetCheck(pDoc->m_humanPlayer.m_dwUnk01E0);
}

// Entry 8: WM_ERASEBKGND. The view paints every pixel itself, so the
// background is never erased. The original still asks for the client
// rectangle and then throws it away.

// hover.exe:0x40C600
// FUNCTION: HOVER 0x40c600
BOOL CBumperView::OnEraseBkgnd(CDC* /*pDC*/)
{
	RECT rect;

	::GetClientRect(m_hWnd, &rect);
	return FALSE;
}

// Entries 9, 10 and 11: debug commands that push one more pod into each of
// the human player's three inventory lists (up to nine per slot) and light
// up the matching dashboard counter.

// hover.exe:0x428590
// FUNCTION: HOVER 0x428590
void CBumperView::OnCmd34014()
{
	CSmokeBombPod* pPod;

	if (m_dwUnk0038 != 0)
		return;
	if (theApp.m_pBumperDoc->m_humanPlayer.m_obList00EC.GetCount() >= 9)
		return;
	pPod = new CSmokeBombPod;
	pPod->m_dwUnk88 = (DWORD)&theApp.m_pBumperDoc->m_humanPlayer;
	theApp.m_pBumperDoc->m_humanPlayer.m_obList00EC.AddHead(pPod);
	m_obj0268.m_dwUnk00 = 1;
	m_obj0268.m_dwUnk08 = theApp.m_pBumperDoc->m_humanPlayer.m_obList00EC.GetCount();
}

// hover.exe:0x428610
// FUNCTION: HOVER 0x428610
void CBumperView::OnCmd34015()
{
	CTempWallPod* pPod;

	if (m_dwUnk0038 != 0)
		return;
	if (theApp.m_pBumperDoc->m_humanPlayer.m_obList00D0.GetCount() >= 9)
		return;
	pPod = new CTempWallPod;
	pPod->m_dwUnk88 = (DWORD)&theApp.m_pBumperDoc->m_humanPlayer;
	theApp.m_pBumperDoc->m_humanPlayer.m_obList00D0.AddHead(pPod);
	m_obj0290.m_dwUnk00 = 1;
	m_obj0290.m_dwUnk08 = theApp.m_pBumperDoc->m_humanPlayer.m_obList00D0.GetCount();
}

// hover.exe:0x428690
// FUNCTION: HOVER 0x428690
void CBumperView::OnCmd34013()
{
	CJumpPod* pPod;

	if (m_dwUnk0038 != 0)
		return;
	if (theApp.m_pBumperDoc->m_humanPlayer.m_obList0108.GetCount() >= 9)
		return;
	pPod = new CJumpPod;
	pPod->m_dwUnk88 = (DWORD)&theApp.m_pBumperDoc->m_humanPlayer;
	theApp.m_pBumperDoc->m_humanPlayer.m_obList0108.AddHead(pPod);
	m_obj0240.m_dwUnk00 = 1;
	m_obj0240.m_dwUnk08 = theApp.m_pBumperDoc->m_humanPlayer.m_obList0108.GetCount();
}

// Entries 12..15: debug commands that apply a pod effect to the human
// player immediately (the pod is created, marked "already used" through
// m_dwUnk98 = 0 and handed straight to its own effect handler).

// hover.exe:0x428710
// FUNCTION: HOVER 0x428710
void CBumperView::OnCmd34018()
{
	CInviciblePod* pPod;

	if (m_dwUnk0038 != 0)
		return;
	pPod = new CInviciblePod;
	pPod->m_dwUnk98 = 0;
	pPod->m_dwUnk88 = (DWORD)&theApp.m_pBumperDoc->m_humanPlayer;
	pPod->Sub_42AD40(&theApp.m_pBumperDoc->m_humanPlayer, 0);
}

// hover.exe:0x428770
// FUNCTION: HOVER 0x428770
void CBumperView::OnCmd34019()
{
	CMapEraserPod* pPod;

	if (m_dwUnk0038 != 0)
		return;
	pPod = new CMapEraserPod;
	pPod->m_dwUnk98 = 0;
	pPod->m_dwUnk88 = (DWORD)&theApp.m_pBumperDoc->m_humanPlayer;
	pPod->Sub_42AD40(&theApp.m_pBumperDoc->m_humanPlayer, 0);
}

// hover.exe:0x4287D0
// FUNCTION: HOVER 0x4287d0
void CBumperView::OnCmd34017()
{
	CSpeedPod* pPod;

	if (m_dwUnk0038 != 0)
		return;
	pPod = new CSpeedPod;
	pPod->m_dwUnk98 = 0;
	pPod->m_dwUnk88 = (DWORD)&theApp.m_pBumperDoc->m_humanPlayer;
	pPod->Sub_42AD40(&theApp.m_pBumperDoc->m_humanPlayer, 0);
}

// hover.exe:0x428830
// FUNCTION: HOVER 0x428830
void CBumperView::OnCmd34016()
{
	CSlowPod* pPod;

	if (m_dwUnk0038 != 0)
		return;
	pPod = new CSlowPod;
	pPod->m_dwUnk98 = 0;
	pPod->m_dwUnk88 = (DWORD)&theApp.m_pBumperDoc->m_humanPlayer;
	pPod->Sub_42AD40(&theApp.m_pBumperDoc->m_humanPlayer, 0);
}

// Entry 16: ID_GAME_PAUSE. Bit 0x80000000 of theApp.m_dwUnk01B8 is the
// paused flag; the low bits are the game state. Resuming repaints the seven
// dashboard gauges and restarts the frame timer, pausing runs the inlined
// Sub_40EE90(1) transition.

// hover.exe:0x413C70
// FUNCTION: HOVER 0x413c70
void CBumperView::OnGamePause()
{
	DWORD dwState;

	dwState = theApp.m_dwUnk01B8 & 0x7FFFFFFF;
	if (dwState == 0 || dwState == 1)
		return;

	if ((theApp.m_dwUnk01B8 & 0x80000000) != 0)
	{
		m_obj02E0.Sub_401AD0(&m_dcUnk00B4, 1);
		m_obj0370.Sub_401AD0(&m_dcUnk00B4, 1);
		m_obj0340.Sub_401AD0(&m_dcUnk00B4, 1);
		m_obj0310.Sub_401AD0(&m_dcUnk00B4, 1);
		m_obj03A0.Sub_401AD0(&m_dcUnk00B4, 1);
		m_obj03D0.Sub_401AD0(&m_dcUnk00B4, 1);
		m_obj0400.Sub_401AD0(&m_dcUnk00B4, 1);
		if ((theApp.m_dwUnk01B8 & 0x80000000) != 0)
		{
			theApp.m_dwUnk01B8 &= 0x7FFFFFFF;
			Sub_4126C0();
		}
		return;
	}

	HOVER_INLINE_SUB_40EE90_1();
}

// Entry 17: ID_GAME_PAUSE update handler.

// hover.exe:0x412860
// FUNCTION: HOVER 0x412860
void CBumperView::OnUpdateGamePause(CCmdUI* pCmdUI)
{
	DWORD dwState;

	dwState = theApp.m_dwUnk01B8 & 0x7FFFFFFF;
	pCmdUI->Enable(dwState != 0 && dwState != 1);
	pCmdUI->SetCheck((theApp.m_dwUnk01B8 & 0x80000000) >> 31);
}

// Entries 18/19: ID_CMD_34005 toggles the "render into the full client
// rectangle" debug mode (m_dwUnk0540). Turning it on re-derives the
// renderer viewport from the client rectangle, turning it off restores the
// offscreen bitmap size.

// hover.exe:0x428970
// FUNCTION: HOVER 0x428970
void CBumperView::OnCmd34005()
{
	RECT rect;

	m_dwUnk0540 = (m_dwUnk0540 == 0);
	if (m_dwUnk0540 == 0)
	{
		Sub_4274B0(m_dwUnk00A4, m_dwUnk00A8, 0);
		return;
	}
	::GetClientRect(m_hWnd, &rect);
	m_obj0150.Sub_40FEE0(&m_dcUnk00B4, this, (CBumperDoc*)m_pDocument,
		rect.left, rect.top, rect.right, rect.bottom);
}

// hover.exe:0x4289F0
// FUNCTION: HOVER 0x4289f0
void CBumperView::OnUpdateCmd34005(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(m_dwUnk0540);
}

// Entry 20: WM_KEYDOWN. Only handled while the game is running (state 3):
// +/- change the view scale, and the three inventory keys drop the last pod
// of the matching list on the human player.

// hover.exe:0x40B540
// FUNCTION: HOVER 0x40b540
void CBumperView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if (theApp.m_dwUnk01B8 != 3)
	{
		Default();
		return;
	}

	if (nChar == VK_ADD ||
		(UINT)(short)::VkKeyScanA('+') == nChar ||
		(UINT)(short)::VkKeyScanA('=') == nChar)
	{
		Sub_4275D0(1, 0);
	}
	else if (nChar == VK_SUBTRACT ||
		(UINT)(short)::VkKeyScanA('-') == nChar ||
		(UINT)(short)::VkKeyScanA('_') == nChar)
	{
		Sub_4275D0(0, 0);
	}
	else if (nChar == theApp.m_avkControls[1])
	{
		if (theApp.m_pBumperDoc->m_humanPlayer.m_obList00D0.GetCount() != 0)
		{
			((CGameObj*)theApp.m_pBumperDoc->m_humanPlayer.m_obList00D0.GetTail())->
				Sub_42AD50((DWORD)&theApp.m_pBumperDoc->m_humanPlayer);
			m_obj0290.m_dwUnk00 = 1;
			m_obj0290.m_dwUnk08 =
				theApp.m_pBumperDoc->m_humanPlayer.m_obList00D0.GetCount();
		}
	}
	else if (nChar == theApp.m_avkControls[0])
	{
		if (theApp.m_pBumperDoc->m_humanPlayer.m_obList00EC.GetCount() != 0)
		{
			((CGameObj*)theApp.m_pBumperDoc->m_humanPlayer.m_obList00EC.GetTail())->
				Sub_42AD50((DWORD)&theApp.m_pBumperDoc->m_humanPlayer);
			m_obj0268.m_dwUnk00 = 1;
			m_obj0268.m_dwUnk08 =
				theApp.m_pBumperDoc->m_humanPlayer.m_obList00EC.GetCount();
		}
	}
	else if (nChar == theApp.m_avkControls[2])
	{
		// The jump pod additionally needs the player to be on the ground
		// and not already jumping.
		if (theApp.m_pBumperDoc->m_humanPlayer.m_obList0108.GetCount() != 0 &&
			(int)theApp.m_pBumperDoc->m_humanPlayer.m_dwUnk0148 <= 0 &&
			*(DWORD*)&theApp.m_pBumperDoc->m_humanPlayer.m_awUnk0090[8] ==
				*(DWORD*)&theApp.m_pBumperDoc->m_humanPlayer.m_awUnk0090[10] &&
			*(DWORD*)&theApp.m_pBumperDoc->m_humanPlayer.m_pad00A8[0] == 0)
		{
			((CGameObj*)theApp.m_pBumperDoc->m_humanPlayer.m_obList0108.GetTail())->
				Sub_42AD50((DWORD)&theApp.m_pBumperDoc->m_humanPlayer);
			m_obj0240.m_dwUnk00 = 1;
			m_obj0240.m_dwUnk08 =
				theApp.m_pBumperDoc->m_humanPlayer.m_obList0108.GetCount();
		}
	}
	Default();
}

// Entries 21/22: ID_CMD_34029 toggles the human player's +0x144 flag and
// mirrors it into +0x140.

// hover.exe:0x428A10
// FUNCTION: HOVER 0x428a10
void CBumperView::OnCmd34029()
{
	if (m_dwUnk0038 != 0)
		return;
	theApp.m_pBumperDoc->m_humanPlayer.m_dwUnk0144 =
		(theApp.m_pBumperDoc->m_humanPlayer.m_dwUnk0144 == 0);
	theApp.m_pBumperDoc->m_humanPlayer.m_dwUnk0140 =
		theApp.m_pBumperDoc->m_humanPlayer.m_dwUnk0144;
}

// hover.exe:0x428A40
// FUNCTION: HOVER 0x428a40
void CBumperView::OnUpdateCmd34029(CCmdUI* pCmdUI)
{
	DWORD dwState;

	dwState = theApp.m_dwUnk01B8 & 0x7FFFFFFF;
	pCmdUI->SetCheck(theApp.m_pBumperDoc->m_humanPlayer.m_dwUnk0144);
	pCmdUI->Enable(dwState != 0 && dwState != 1);
}

// Entry 23: ID_CMD_34030 restocks the board - one new flag per flag the
// human player is carrying plus m_dwUnk83E8 robot flags are dropped back on
// random pads, every player's carried-object list is emptied and the two
// dashboard flag sprites are reset.

// hover.exe:0x428A90
// FUNCTION: HOVER 0x428a90
void CBumperView::OnCmd34030()
{
	CBumperView* pView;
	POSITION pos;
	CObject* pObj;
	int nFlags;

	if (m_dwUnk0038 != 0)
		return;

	::EnterCriticalSection(&theApp.m_csUnk00EC);

	nFlags = theApp.m_pBumperDoc->m_humanPlayer.m_obList00B4.GetCount();
	while (nFlags != 0)
	{
		theApp.m_pBumperDoc->Sub_412250(new CFlag(1), 0);
		nFlags--;
	}

	nFlags = theApp.m_pBumperDoc->m_dwUnk83E8;
	while (nFlags != 0)
	{
		theApp.m_pBumperDoc->Sub_412250(new CFlag(0), 0);
		nFlags--;
	}
	theApp.m_pBumperDoc->m_dwUnk83E8 = 0;

	pos = theApp.m_pBumperDoc->m_list83A8.GetHeadPosition();
	while (pos != NULL)
	{
		pObj = theApp.m_pBumperDoc->m_list83A8.GetNext(pos);
		if (pObj->IsKindOf(RUNTIME_CLASS(CPlayer)))
			((CPlayer*)pObj)->Sub_414BD0(0);
	}

	// Hover_sub_401140() is expanded inline here in the original.
	pView = (theApp.m_pMainWnd == NULL) ? NULL :
		(CBumperView*)((CFrameWnd*)theApp.m_pMainWnd)->GetActiveView();
	pView->m_obj0430.m_dwUnk00 = 1;
	pView->m_obj0430.m_dwUnk44 = pView->m_obj0430.m_dwUnk3C;
	if ((int)pView->m_obj0430.m_dwUnk44 > 0)
		pView->m_obj0430.m_dwUnk44 = 0;

	pView = (theApp.m_pMainWnd == NULL) ? NULL :
		(CBumperView*)((CFrameWnd*)theApp.m_pMainWnd)->GetActiveView();
	pView->m_obj0480.m_dwUnk00 = 1;
	pView->m_obj0480.m_dwUnk44 = pView->m_obj0480.m_dwUnk3C;
	if ((int)pView->m_obj0480.m_dwUnk44 > 0)
		pView->m_obj0480.m_dwUnk44 = 0;

	::LeaveCriticalSection(&theApp.m_csUnk00EC);
}

// Entries 24..31: eight identical update handlers (one per debug pod
// command) that only enable the command while a game is actually running.
// The original emits each of them separately, 0x30 bytes apart.

// hover.exe:0x428C00
// FUNCTION: HOVER 0x428c00
void CBumperView::OnUpdateCmd34030(CCmdUI* pCmdUI)
{
	DWORD dwState;

	dwState = theApp.m_dwUnk01B8 & 0x7FFFFFFF;
	pCmdUI->Enable(dwState != 0 && dwState != 1);
}

// hover.exe:0x428C30
// FUNCTION: HOVER 0x428c30
void CBumperView::OnUpdateCmd34014(CCmdUI* pCmdUI)
{
	DWORD dwState;

	dwState = theApp.m_dwUnk01B8 & 0x7FFFFFFF;
	pCmdUI->Enable(dwState != 0 && dwState != 1);
}

// hover.exe:0x428C60
// FUNCTION: HOVER 0x428c60
void CBumperView::OnUpdateCmd34015(CCmdUI* pCmdUI)
{
	DWORD dwState;

	dwState = theApp.m_dwUnk01B8 & 0x7FFFFFFF;
	pCmdUI->Enable(dwState != 0 && dwState != 1);
}

// hover.exe:0x428C90
// FUNCTION: HOVER 0x428c90
void CBumperView::OnUpdateCmd34013(CCmdUI* pCmdUI)
{
	DWORD dwState;

	dwState = theApp.m_dwUnk01B8 & 0x7FFFFFFF;
	pCmdUI->Enable(dwState != 0 && dwState != 1);
}

// hover.exe:0x428CC0
// FUNCTION: HOVER 0x428cc0
void CBumperView::OnUpdateCmd34018(CCmdUI* pCmdUI)
{
	DWORD dwState;

	dwState = theApp.m_dwUnk01B8 & 0x7FFFFFFF;
	pCmdUI->Enable(dwState != 0 && dwState != 1);
}

// hover.exe:0x428CF0
// FUNCTION: HOVER 0x428cf0
void CBumperView::OnUpdateCmd34019(CCmdUI* pCmdUI)
{
	DWORD dwState;

	dwState = theApp.m_dwUnk01B8 & 0x7FFFFFFF;
	pCmdUI->Enable(dwState != 0 && dwState != 1);
}

// hover.exe:0x428D20
// FUNCTION: HOVER 0x428d20
void CBumperView::OnUpdateCmd34017(CCmdUI* pCmdUI)
{
	DWORD dwState;

	dwState = theApp.m_dwUnk01B8 & 0x7FFFFFFF;
	pCmdUI->Enable(dwState != 0 && dwState != 1);
}

// hover.exe:0x428D50
// FUNCTION: HOVER 0x428d50
void CBumperView::OnUpdateCmd34016(CCmdUI* pCmdUI)
{
	DWORD dwState;

	dwState = theApp.m_dwUnk01B8 & 0x7FFFFFFF;
	pCmdUI->Enable(dwState != 0 && dwState != 1);
}

// Entries 32/33: ID_CMD_34032 toggles the 2x pixel-doubled presentation
// (m_dwUnk0544); it is only available in a 640-pixel wide client area.

// hover.exe:0x429230
// FUNCTION: HOVER 0x429230
void CBumperView::OnCmd34032()
{
	HOVER_INLINE_SUB_40EE90_1();

	m_dwUnk0544 = (m_dwUnk0544 == 0);
	Sub_4274B0(m_dwUnk00A4, m_dwUnk00A8, 1);
	Sub_40DC20(1);
}

// hover.exe:0x4292F0
// FUNCTION: HOVER 0x4292f0
void CBumperView::OnUpdateCmd34032(CCmdUI* pCmdUI)
{
	RECT rect;

	::GetClientRect(m_hWnd, &rect);
	pCmdUI->Enable(rect.right - rect.left - 640 == 0);
	pCmdUI->SetCheck(m_dwUnk0544);
}

// Entry 34: WM_CREATE.

// hover.exe:0x420DF0
// FUNCTION: HOVER 0x420df0
int CBumperView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	return (CView::OnCreate(lpCreateStruct) == -1) ? -1 : 0;
}

// Entries 35/36: ID_CMD_34033 toggles the alternate present path used by
// CBumperView::Sub_407430.

// hover.exe:0x429340
// FUNCTION: HOVER 0x429340
void CBumperView::OnCmd34033()
{
	g_dwUnk_4C4CA8 = (g_dwUnk_4C4CA8 == 0);
}

// hover.exe:0x429360
// FUNCTION: HOVER 0x429360
void CBumperView::OnUpdateCmd34033(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(g_dwUnk_4C4CA8);
}

// Entry 40: WM_CHAR. Recognizes the five-keystroke cheat sequence Ctrl+I,
// Ctrl+B, Ctrl+M, Ctrl+A, Ctrl+B (character codes 9, 2, 13, 1, 2), which
// sets m_dwUnk0038 and forces the game into state 3. m_dwUnk003C is the
// position in the sequence; any wrong key restarts it.

// hover.exe:0x429380
// FUNCTION: HOVER 0x429380
void CBumperView::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if (theApp.m_pBumperDoc->m_dwUnk8498 == 0)
		return;
	if (theApp.m_dwUnk01B8 == 3)
		return;

	switch (m_dwUnk003C)
	{
	case 0:
		if (nChar == 9)
		{
			m_dwUnk003C = m_dwUnk003C + 1;
			break;
		}
		m_dwUnk0038 = 0;
		m_dwUnk003C = 0;
		break;

	case 1:
		if (nChar == 2)
		{
			m_dwUnk003C = m_dwUnk003C + 1;
			break;
		}
		m_dwUnk0038 = 0;
		m_dwUnk003C = 0;
		break;

	case 2:
		if (nChar == 13)
		{
			m_dwUnk003C = m_dwUnk003C + 1;
			break;
		}
		m_dwUnk0038 = 0;
		m_dwUnk003C = 0;
		break;

	case 3:
		if (nChar == 1)
		{
			m_dwUnk003C = m_dwUnk003C + 1;
			break;
		}
		m_dwUnk0038 = 0;
		m_dwUnk003C = 0;
		break;

	case 4:
		if (nChar == 2)
		{
			m_dwUnk0038 = 1;
			m_dwUnk003C = 0;
			theApp.m_dwUnk01B8 = 3;
			m_dwUnk007C = 60;
			Sub_4126C0();
			break;
		}
		m_dwUnk0038 = 0;
		m_dwUnk003C = 0;
		break;

	default:
		m_dwUnk0038 = 0;
		m_dwUnk003C = 0;
		break;
	}
	Default();
}

// The 50 ms multimedia timer callback installed by CBumperView::Sub_4126C0.
// g_dwUnk_4C4CE4 is the "a frame tick is already in flight" latch: it is
// raised here and lowered again by CBumperView::OnUserInput, so a tick that
// arrives while the previous one is still being processed is dropped. The
// active view is looked up inline (the same expression as Hover_sub_401140)
// and told to sample its input with WM_USER.

// hover.exe:0x408AA0
// FUNCTION: HOVER 0x408aa0
void CALLBACK Hover_sub_408AA0(UINT uID, UINT uMsg, DWORD dwUser,
	DWORD dw1, DWORD dw2)
{
	CBumperView* pView;

	if (g_dwUnk_4C4CE4 != 0)
		return;
	g_dwUnk_4C4CE4 = 1;
	pView = (*(CFrameWnd* volatile*)&theApp.m_pMainWnd != NULL)
		? (CBumperView*)((CFrameWnd*)theApp.m_pMainWnd)->GetActiveView()
		: NULL;
	if (pView != NULL)
		::PostMessage(pView->m_hWnd, WM_USER, 0, 0);
}

// Entry 41: WM_USER+0, posted by the multimedia timer callback
// (hover.exe:0x408AA0). Samples the joystick and the four direction keys
// into the input state at m_dwUnk006C..m_dwUnk0078 and runs the two frame
// steps.

// hover.exe:0x408AF0
// FUNCTION: HOVER 0x408af0
LRESULT CBumperView::OnUserInput(WPARAM wParam, LPARAM lParam)
{
	MMRESULT (WINAPI* pfnJoyGetPosEx)(UINT, LPJOYINFOEX);
	DWORD dwXMid;
	DWORD dwYMid;
	int bUpLeft;
	int bUpRight;
	int bDownLeft;
	int bDownRight;

	::EnterCriticalSection(&theApp.m_csUnk0134);
	m_dwUnk006C = 0;
	m_dwUnk0070 = 0;
	m_dwUnk0074 = 0;
	m_dwUnk0078 = 0;

	pfnJoyGetPosEx = (MMRESULT (WINAPI*)(UINT, LPJOYINFOEX))g_pfnJoyGetPosEx;
	if (theApp.m_avkControls[11] != 0 && pfnJoyGetPosEx != NULL)
	{
		// theApp.m_adwUnk0378 is a JOYINFOEX: [0] dwSize, [1] dwFlags,
		// [2] dwXpos, [3] dwYpos. The stick is treated as pushed once it
		// leaves the middle half of its calibrated range.
		theApp.m_adwUnk0378[0] = 0x34;
		theApp.m_adwUnk0378[1] = JOY_RETURNX | JOY_RETURNY |
			JOY_RETURNBUTTONS | JOY_RETURNCENTERED;
		pfnJoyGetPosEx(0, (LPJOYINFOEX)theApp.m_adwUnk0378);

		dwXMid = (theApp.m_joyCaps.wXmin + theApp.m_joyCaps.wXmax) >> 1;
		dwYMid = (theApp.m_joyCaps.wYmin + theApp.m_joyCaps.wYmax) >> 1;
		m_dwUnk006C = (theApp.m_adwUnk0378[2] <
			dwXMid - ((dwXMid - theApp.m_joyCaps.wXmin) >> 2));
		m_dwUnk0070 = (dwXMid + ((theApp.m_joyCaps.wXmax - dwXMid) >> 2) <
			theApp.m_adwUnk0378[2]);
		m_dwUnk0074 = (theApp.m_adwUnk0378[3] <
			dwYMid - ((dwYMid - theApp.m_joyCaps.wYmin) >> 2));
		m_dwUnk0078 = (dwYMid + ((theApp.m_joyCaps.wYmax - dwYMid) >> 2) <
			theApp.m_adwUnk0378[3]);
	}

	// The four diagonal keys contribute to two directions each.
	bUpLeft = (::GetAsyncKeyState(theApp.m_avkControls[7]) < 0);
	bUpRight = (::GetAsyncKeyState(theApp.m_avkControls[8]) < 0);
	bDownLeft = (::GetAsyncKeyState(theApp.m_avkControls[9]) < 0);
	bDownRight = (::GetAsyncKeyState(theApp.m_avkControls[10]) < 0);

	if (bUpLeft || bDownLeft || ::GetAsyncKeyState(theApp.m_avkControls[5]) < 0)
		m_dwUnk006C |= 1;
	if (bUpRight || bDownRight || ::GetAsyncKeyState(theApp.m_avkControls[6]) < 0)
		m_dwUnk0070 |= 1;
	if (bUpLeft || bUpRight || ::GetAsyncKeyState(theApp.m_avkControls[3]) < 0)
		m_dwUnk0074 |= 1;
	if (bDownLeft || bDownRight || ::GetAsyncKeyState(theApp.m_avkControls[4]) < 0)
		m_dwUnk0078 |= 1;

	::LeaveCriticalSection(&theApp.m_csUnk0134);
	Sub_408D00();
	Sub_408DF0();
	g_dwUnk_4C4CE4 = 0;
	return 0;
}

// Entry 42: WM_USER+1 - the game is over, run the high-score flow.

// hover.exe:0x427D20
// FUNCTION: HOVER 0x427d20
LRESULT CBumperView::OnUserGameOver(WPARAM wParam, LPARAM lParam)
{
	theApp.Sub_425B20();
	return 0;
}

// Entry 43: WM_USER+2 - the level was completed, advance to the next one.

// hover.exe:0x427D30
// FUNCTION: HOVER 0x427d30
LRESULT CBumperView::OnUserNextLevel(WPARAM wParam, LPARAM lParam)
{
	theApp.Sub_425AD0();
	return 0;
}

// Entry 44: ID_ESCAPE (the Esc accelerator) suspends the game and minimizes
// the main window.

// hover.exe:0x428890
// FUNCTION: HOVER 0x428890
void CBumperView::OnEscape()
{
	HOVER_INLINE_SUB_40EE90_1();

	if (theApp.m_dwUnk03B0 != 0)
		theApp.Sub_425600();
	::ShowWindow(AfxGetApp()->m_pMainWnd->m_hWnd, SW_SHOWMINIMIZED);
}

// Entry 45: MM_MCINOTIFY. Nudges the main thread's priority down and back
// up so the freshly started MCI playback gets scheduled.

// hover.exe:0x428940
// FUNCTION: HOVER 0x428940
LRESULT CBumperView::OnMciNotify(WPARAM wParam, LPARAM lParam)
{
	::SetThreadPriority((HANDLE)theApp.m_hThread,
		THREAD_PRIORITY_BELOW_NORMAL);
	::SetThreadPriority((HANDLE)theApp.m_hThread,
		THREAD_PRIORITY_ABOVE_NORMAL);
	return 0;
}

// Entry 46: WM_HELP. The view swallows F1 (the context menu below is the
// only help entry point).

// hover.exe:0x428D80
// FUNCTION: HOVER 0x428d80
LRESULT CBumperView::OnHelpMsg(WPARAM wParam, LPARAM lParam)
{
	return 1;
}

// The context-menu hit tests index the zone table directly instead of
// caching a pointer: the original addresses every field as
// [index*0x338 + g_adwUnk_4609A0 + field], which only comes out of an
// array subscript on the constant base.
#define HOVER_ZONE(i)	(((ZONE_DESCRIPTOR*)g_adwUnk_4609A0)[i])

// Entry 47: WM_CONTEXTMENU. Pops up a one-item "What's This?" menu and, if
// it is chosen, opens the help topic for whichever dashboard element the
// cursor is over. The hit rectangles are derived from the current board
// zone descriptor.

// hover.exe:0x428D90
// FUNCTION: HOVER 0x428d90
LRESULT CBumperView::OnContextMenuMsg(WPARAM wParam, LPARAM lParam)
{
	CPoint point;
	CRect rcWaypoints;
	CRect rcMap;
	CRect rcPods;
	CRect rcScore;
	CRect rcGauge1;
	CRect rcGauge2;
	CRect rcGauge3;
	int nZone;
	int nCmd;

	HOVER_INLINE_SUB_40EE90_1();

	::GetCursorPos(&point);

	CMenu menu;
	CString strItem;

	strItem.LoadString(IDS_WHATS_THIS);
	menu.Attach(::CreatePopupMenu());
	::AppendMenuA(menu.m_hMenu, MF_STRING, 10, strItem);
	nCmd = menu.TrackPopupMenu(TPM_RIGHTBUTTON | TPM_RETURNCMD,
		point.x, point.y, this, NULL);
	if (nCmd == 0 || nCmd == -1)
		return 1;

	::ScreenToClient(m_hWnd, &point);
	nZone = (int)g_dwUnk_460998;

	rcWaypoints.left = HOVER_ZONE(nZone).m_rcUnk228.left;
	rcWaypoints.top = HOVER_ZONE(nZone).m_rcUnk228.top;
	rcWaypoints.right = HOVER_ZONE(nZone).m_rcUnk228.left + 6 * HOVER_ZONE(nZone).m_dwUnk28;
	rcWaypoints.bottom = HOVER_ZONE(nZone).m_rcUnk228.top + 2 * HOVER_ZONE(nZone).m_dwUnk2C;

	rcMap.left = HOVER_ZONE(nZone).m_rcUnk210.left;
	rcMap.top = HOVER_ZONE(nZone).m_rcComposite2.top;
	rcMap.right = HOVER_ZONE(nZone).m_rcUnk210.right;
	rcMap.bottom = HOVER_ZONE(nZone).m_rcComposite2.bottom;

	rcPods.left = HOVER_ZONE(nZone).m_rcUnk220.left;
	rcPods.top = HOVER_ZONE(nZone).m_rcUnk220.top;
	{
		// The pod strip's extents go through two locals of their own: the
		// original spills the x7 multiplicand to a slot
		// (mov [t],eax / shl eax,3 / sub eax,[t]) and parks the bottom edge
		// in a second slot until after the right edge is stored.
		int cyPods = (int)HOVER_ZONE(nZone).m_dwUnk1C
			+ HOVER_ZONE(nZone).m_rcUnk220.top;
		int cxPods = (int)HOVER_ZONE(nZone).m_dwUnk18;

		rcPods.right = HOVER_ZONE(nZone).m_rcUnk220.left + 7 * cxPods;
		rcPods.bottom = cyPods;
	}

	rcScore.left = HOVER_ZONE(nZone).m_rcComposite2.left;
	rcScore.top = HOVER_ZONE(nZone).m_rcComposite2.bottom;
	rcScore.right = HOVER_ZONE(nZone).m_rcComposite2.right;
	rcScore.bottom = HOVER_ZONE(nZone).m_rcUnk248.top;

	rcGauge1.left = HOVER_ZONE(nZone).m_rcUnk248.left;
	rcGauge1.top = HOVER_ZONE(nZone).m_rcUnk248.top;
	rcGauge1.right = HOVER_ZONE(nZone).m_rcViewport.left;
	rcGauge1.bottom = HOVER_ZONE(nZone).m_rcUnk248.bottom;

	rcGauge2.left = HOVER_ZONE(nZone).m_rcViewport.left;
	rcGauge2.top = HOVER_ZONE(nZone).m_rcUnk248.top;
	rcGauge2.right = HOVER_ZONE(nZone).m_rcViewport.right;
	rcGauge2.bottom = HOVER_ZONE(nZone).m_rcUnk248.bottom;

	rcGauge3.left = HOVER_ZONE(nZone).m_rcViewport.right;
	rcGauge3.top = HOVER_ZONE(nZone).m_rcUnk248.top;
	rcGauge3.right = HOVER_ZONE(nZone).m_rcUnk248.right;
	rcGauge3.bottom = HOVER_ZONE(nZone).m_rcUnk248.bottom;

	if (rcWaypoints.PtInRect(point))
		::WinHelp(GetSafeHwnd(), "hover.hlp", HELP_CONTEXT, 0xA3C);
	else if (rcMap.PtInRect(point))
	{
		if (theApp.m_dwUnk01D0 != 0)
			::WinHelp(GetSafeHwnd(), "hover.hlp", HELP_CONTEXTPOPUP, 0xA32);
	}
	else if (rcPods.PtInRect(point))
		::WinHelp(GetSafeHwnd(), "hover.hlp", HELP_CONTEXT, 0xA28);
	else if (rcScore.PtInRect(point))
		::WinHelp(GetSafeHwnd(), "hover.hlp", HELP_CONTEXT, 0xA46);
	else if (rcGauge1.PtInRect(point))
		::WinHelp(GetSafeHwnd(), "hover.hlp", HELP_CONTEXT, 0xA50);
	else if (rcGauge2.PtInRect(point))
		::WinHelp(GetSafeHwnd(), "hover.hlp", HELP_CONTEXT, 0xA5A);
	else if (rcGauge3.PtInRect(point))
		::WinHelp(GetSafeHwnd(), "hover.hlp", HELP_CONTEXT, 0xA64);
	return 1;
}

// Entry 48: ID_CMD_34034 is mapped but does nothing in the shipped build.

// hover.exe:0x4294F0
// FUNCTION: HOVER 0x4294f0
void CBumperView::OnCmd34034()
{
}
