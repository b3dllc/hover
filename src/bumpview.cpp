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
#include "texheap.h"			// renderer classes and globals
#include "texmisc.h"			// Hover_FatalException (exception reporter)
#include "mainfrm.h"			// CMainFrame::SetPaneMessage (OnSize status bar)

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
	ON_COMMAND(ID_DBG_ADD_SMOKEPOD, OnDbgAddSmokePod)
	ON_COMMAND(ID_DBG_ADD_WALLPOD, OnDbgAddWallPod)
	ON_COMMAND(ID_DBG_ADD_JUMPPOD, OnDbgAddJumpPod)
	ON_COMMAND(ID_DBG_APPLY_INVINCIBLE, OnDbgApplyInvincible)
	ON_COMMAND(ID_DBG_APPLY_MAPERASER, OnDbgApplyMapEraser)
	ON_COMMAND(ID_DBG_APPLY_SPEED, OnDbgApplySpeed)
	ON_COMMAND(ID_DBG_APPLY_SLOW, OnDbgApplySlow)
	ON_COMMAND(ID_GAME_PAUSE, OnGamePause)
	ON_UPDATE_COMMAND_UI(ID_GAME_PAUSE, OnUpdateGamePause)
	ON_COMMAND(ID_DBG_FULLCLIENT_RENDER, OnDbgFullClientRender)
	ON_UPDATE_COMMAND_UI(ID_DBG_FULLCLIENT_RENDER, OnUpdateDbgFullClientRender)
	ON_WM_KEYDOWN()
	ON_COMMAND(ID_DBG_INVINCIBLE, OnDbgInvincible)
	ON_UPDATE_COMMAND_UI(ID_DBG_INVINCIBLE, OnUpdateDbgInvincible)
	ON_COMMAND(ID_DBG_RESTOCK_FLAGS, OnDbgRestockFlags)
	ON_UPDATE_COMMAND_UI(ID_DBG_RESTOCK_FLAGS, OnUpdateDbgRestockFlags)
	ON_UPDATE_COMMAND_UI(ID_DBG_ADD_SMOKEPOD, OnUpdateDbgAddSmokePod)
	ON_UPDATE_COMMAND_UI(ID_DBG_ADD_WALLPOD, OnUpdateDbgAddWallPod)
	ON_UPDATE_COMMAND_UI(ID_DBG_ADD_JUMPPOD, OnUpdateDbgAddJumpPod)
	ON_UPDATE_COMMAND_UI(ID_DBG_APPLY_INVINCIBLE, OnUpdateDbgApplyInvincible)
	ON_UPDATE_COMMAND_UI(ID_DBG_APPLY_MAPERASER, OnUpdateDbgApplyMapEraser)
	ON_UPDATE_COMMAND_UI(ID_DBG_APPLY_SPEED, OnUpdateDbgApplySpeed)
	ON_UPDATE_COMMAND_UI(ID_DBG_APPLY_SLOW, OnUpdateDbgApplySlow)
	ON_COMMAND(ID_DBG_PIXEL_DOUBLE, OnDbgPixelDouble)
	ON_UPDATE_COMMAND_UI(ID_DBG_PIXEL_DOUBLE, OnUpdateDbgPixelDouble)
	ON_WM_CREATE()
	ON_COMMAND(ID_DBG_ALT_PRESENT, OnDbgAltPresent)
	ON_UPDATE_COMMAND_UI(ID_DBG_ALT_PRESENT, OnUpdateDbgAltPresent)
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
DWORD g_adwDrawerModeForDetail[6] = { 1, 2, 4, 3, 5, 6 };

// GLOBAL: HOVER 0x460998
DWORD g_dwCurZone;		// hover.exe:0x460998 zone descriptor index
// GLOBAL: HOVER 0x4c4ca8
DWORD g_bFullFramePresent;		// hover.exe:0x4C4CA8

// hover.exe:0x4C4CEC
// GLOBAL: HOVER 0x4c4cec
int g_nStatusRefreshCountdown = 0x39;	// down-counter that paces UpdateStatusBar
// GLOBAL: HOVER 0x4c4cf0
DWORD g_dwPrevJoyButtons;		// hover.exe:0x4C4CF0 previous joystick button mask

// GLOBAL: HOVER 0x4a4790
DWORD g_dwMapCamX;		// hover.exe:0x4A4790
// GLOBAL: HOVER 0x4a4794
DWORD g_dwMapCamY;		// hover.exe:0x4A4794
// GLOBAL: HOVER 0x4a4798
DWORD g_dwMapCullRight;		// hover.exe:0x4A4798
// GLOBAL: HOVER 0x4a479c
DWORD g_dwMapCullBottom;		// hover.exe:0x4A479C
// GLOBAL: HOVER 0x4a47b0
DWORD g_dwMapFaceCount;		// hover.exe:0x4A47B0
// GLOBAL: HOVER 0x4a47b8
DWORD g_dwMapRotAngle;		// hover.exe:0x4A47B8
// GLOBAL: HOVER 0x4a47bc
DWORD g_dwMapCullLeft;		// hover.exe:0x4A47BC
// GLOBAL: HOVER 0x4a47d8
DWORD g_dwMapCullTop;		// hover.exe:0x4A47D8
// GLOBAL: HOVER 0x4a47e0
DWORD g_dwMapOriginX;		// hover.exe:0x4A47E0
// GLOBAL: HOVER 0x4a47e4
DWORD g_dwMapOriginY;		// hover.exe:0x4A47E4
// 0x4A4844/0x4A4848 are the m_pData/m_nSize members of the beacon CObArray
// at 0x4A4840 (g_arrBeacons in mazeobj.cpp), not globals of their own.

// hover.exe:0x4A1D50
// GLOBAL: HOVER 0x4a1d50
CPalette g_palGame;	// palette selected into the paint DC

/////////////////////////////////////////////////////////////////////////////
// Free helper functions

// Looks up the texture named pszName in the global name -> texture map and
// returns it, or NULL when there is no such texture.
// hover.exe:0x418A30
// FUNCTION: HOVER 0x418a30
CObject* Hover_LookupTexture(const char* pszName)
{
	CObject* pObj;

	return g_mapUnk_4A1D30.Lookup(pszName, pObj) ? pObj : NULL;
}

/////////////////////////////////////////////////////////////////////////////
// CVelocityIndicator

// hover.exe:0x423030
// FUNCTION: HOVER 0x423030
CVelocityIndicator::CVelocityIndicator()
{
	// hover.exe never initializes m_pTargetDC/m_dwCenterX/m_dwCenterY
	// here: they are always written by CreateTile before DrawNeedle (the only
	// reader) runs, so leaving them uninitialized cannot be observed.
	m_pOldBitmap = NULL;
	m_wScaleXFrac = 0;
	m_wScaleX = 0;
	m_pBits = 0;
	m_wScaleYFrac = 0;
	m_wScaleY = 0;
}

// hover.exe:0x420E60
// FUNCTION: HOVER 0x420e60
CVelocityIndicator::~CVelocityIndicator()
{
	// trivial destructor in the original
}

// Recomputes the camera basis offsets from the view yaw and the human
// player's velocity (vx, vy = m_humanPlayer.m_fxVelX/m_fxVelY): with
// a = -yaw (quarter-circle = 0x80 table units) the offsets are
//   m_dwNeedleDX = (sin(a)*vx - sin(a-0x80)*vy) * m_fxScaleX
//   m_dwNeedleDY = (sin(a)*vy + sin(a-0x80)*vx) * m_fxScaleY
// all 16.16 fixed-point, keeping the integer parts.

// hover.exe:0x405A10
// FUNCTION: HOVER 0x405a10
void CVelocityIndicator::UpdateNeedle(CBumperView* pView, CBumperDoc* pDoc)
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
	lAngle = (long)(0xFFFFFF80 - pView->m_dwCamAngle);
	dwCos = pDoc->m_humanPlayer.m_fxVelX;
	dwSin = pDoc->m_humanPlayer.m_fxVelY;
	pSinA = &g_adwSinTable[(((lAngle + 0x80) & 0x1FF) << 4) + 1];
	pSinB = &g_adwSinTable[((lAngle & 0x1FF) << 4) + 1];

	dwA1 = *pSinB;
	HOVER_FIXMULP(dwA1, dwSin);
	dwB1 = *pSinA;
	HOVER_FIXMULP(dwB1, dwCos);
	dwT1 = dwB1 - dwA1;
	HOVER_FIXMULP(dwT1, m_fxScaleX);
	m_dwNeedleDX = (long)((short*)&dwT1)[1];

	dwA2 = *pSinB;
	HOVER_FIXMULP(dwA2, dwCos);
	dwB2 = *pSinA;
	HOVER_FIXMULP(dwB2, dwSin);
	dwT2 = dwB2 + dwA2;
	HOVER_FIXMULP(dwT2, m_fxScaleY);
	m_dwNeedleDY = (long)((short*)&dwT2)[1];
}

// Places the gauge: segments run along the rectangle's longer side, each
// a m_dwSegments-th of it.
// hover.exe:0x411D10
// FUNCTION: HOVER 0x411d10
int CSegmentGauge::SetLayout(const RECT* pRect)
{
	int nHeight;
	int nWidth;
	int nResult;

	m_rc = *pRect;

	// The extents are re-read out of the stored rectangle, not out of the
	// argument.
	nHeight = m_rc.bottom - m_rc.top;
	nWidth = m_rc.right - m_rc.left;

	// Horizontal when the rectangle is at least as wide as it is high.
	m_bHorizontal = (nHeight <= nWidth) ? 1 : 0;

	if (m_bHorizontal == 0)
		nResult = nHeight / (int)m_dwSegments;
	else
		nResult = nWidth / (int)m_dwSegments;

	m_dwSegmentSize = nResult;
	return nResult;
}

// Computes the fixed-point x/y scaling factors stored as the dword
// pair at +0x44 (m_wScaleXFrac/m_wScaleX) and +0x48 (m_wScaleYFrac/m_wScaleY).
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

// HOVER_FIXMULWPR(dwR, dwA, wB): dwR = dwA * (short)wB with the source value,
// the destination pointer and the 16-bit factor each homed in a temporary of
// their own - the shape CVelocityIndicator::ComputeScale's original has
// (hover.exe:0x414122: the 0xFAE1 store is hoisted into the entry block, the
// destination is written from that temp and only then addressed).
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
void CVelocityIndicator::ComputeScale()
{
	DWORD dwNum1;
	DWORD dwScaled1;
	DWORD dwQ1;
	DWORD dwNum2;
	DWORD dwScaled2;
	DWORD dwQ2;
	DWORD dwGlobalFactor;
	DWORD dwPlayerFactor;

	dwGlobalFactor = theApp.m_pHumanParams->m_adw[8];
	dwPlayerFactor = *(DWORD*)((BYTE*)theApp.m_pBumperDoc + 0x8364);
	dwScaled1 = dwPlayerFactor;
	HOVER_FIXMULP(dwScaled1, dwGlobalFactor);
	HOVER_FIXMULWPR(dwNum1, 0xFAE1, (short)((long)m_dwTileWidth / 2));
	// Volatile read: HOVER_FIXDIVPL writes its destination only through the
	// pointer temporary, which VC 2.1 does not model, so a plain
	// `dwQ1 = dwNum1` has its store deleted and +0x44/+0x48 receive the
	// undivided numerator (rule 95).
	dwQ1 = *(volatile DWORD*)&dwNum1;
	HOVER_FIXDIVPL(dwQ1, dwScaled1);
	m_fxScaleX = dwQ1;

	dwGlobalFactor = theApp.m_pHumanParams->m_adw[8];
	dwPlayerFactor = *(DWORD*)((BYTE*)theApp.m_pBumperDoc + 0x8364);
	dwScaled2 = dwPlayerFactor;
	HOVER_FIXMULP(dwScaled2, dwGlobalFactor);
	HOVER_FIXMULWPR(dwNum2, 0xFAE1, (short)((long)m_dwTileHeight / 2));
	dwQ2 = *(volatile DWORD*)&dwNum2;
	HOVER_FIXDIVPL(dwQ2, dwScaled2);
	m_fxScaleY = dwQ2;
}


// Creates an 8-bpp DIB section sized from the source rectangle, attaches it
// to m_bitmap, selects it into m_dc and copies the zone's dashboard tile
// into the DIB bits through Hover_BlitRect.
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
void CVelocityIndicator::CreateTile(CDC* pInternalDC, BYTE* pDib, RECT rc)
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
	*pSrc = rc;

	m_dwTileWidth = pSrc->right - pSrc->left;
	m_dwTileHeight = pSrc->bottom - pSrc->top;
	m_dwCenterX = (long)m_dwTileWidth / 2 + pSrc->left;
	m_dwCenterY = (long)m_dwTileHeight / 2 + pSrc->top;
	m_dwNeedleDX = 0;
	m_dwNeedleDY = 0;
	m_pTargetDC = pInternalDC;

	ComputeScale();

	hScreen = ::GetDC(NULL);

	pBmi = (BITMAPINFO*)::operator new(BMI8_SIZE);
	pBmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	pBmi->bmiHeader.biWidth = m_dwTileWidth;
	pBmi->bmiHeader.biHeight = -(long)m_dwTileHeight;
	pBmi->bmiHeader.biPlanes = 1;
	pBmi->bmiHeader.biBitCount = 8;
	pBmi->bmiHeader.biCompression = BI_RGB;
	pBmi->bmiHeader.biSizeImage = 0;
	pBmi->bmiHeader.biXPelsPerMeter = 0;
	pBmi->bmiHeader.biYPelsPerMeter = 0;
	pBmi->bmiHeader.biClrUsed = 0;
	pBmi->bmiHeader.biClrImportant = 0;

	::GetPaletteEntries(
		(HPALETTE)g_palGame.GetSafeHandle(),
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
		m_dc.SelectObject(m_pOldBitmap);
		m_bitmap.DeleteObject();
		m_bitmap.Detach();
	}

	hBitmap = (HBITMAP)::CreateDIBSection(
		hScreen,
		pBmi,
		DIB_RGB_COLORS,
		(void**)&m_pBits,
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

	m_pOldBitmap = m_dc.SelectObject(&m_bitmap);

	// The blit's source size and destination rectangle are built in six
	// stack slots before the view lookup (hover.exe:0x411F24..0x411F4A,
	// which is why the frame is 0x418 and not 0x400 bytes).
	rcDest.left = 0;
	rcDest.top = 0;
	pSrcBits = pDib + ((BITMAPINFOHEADER*)pDib)->biSize + 0x400;
	rcDest.bottom = m_dwTileHeight;
	rcDest.right = m_dwTileWidth;
	lSrcWidth = ((BITMAPINFOHEADER*)pDib)->biWidth;
	lSrcHeight = ((BITMAPINFOHEADER*)pDib)->biHeight;

	pView = Hover_GetView();
	if (pView->m_bDoubleSize != 0)
		nZone = 0;
	else
		nZone = g_dwCurZone;

	Hover_BlitRect(
		g_aZones[nZone].m_rcNeedleSrc.left,
		g_aZones[nZone].m_rcNeedleSrc.top,
		g_aZones[nZone].m_rcNeedleSrc.right,
		g_aZones[nZone].m_rcNeedleSrc.bottom,
		lSrcWidth,
		lSrcHeight,
		(long)rcDest.left,
		(long)rcDest.top,
		(long)rcDest.right,
		(long)rcDest.bottom,
		(long)m_dwTileWidth,
		(long)m_dwTileHeight,
		pSrcBits,
		m_pBits,
		0);
}

// hover.exe:0x401E80
// FUNCTION: HOVER 0x401e80
void CVelocityIndicator::DrawNeedle(CDC* pDestDC, int bFullUpdate)
{
	CBumperView* pView;
	CRect rcRaw;
	CRect rcUpdate;
	CRect rcDirty;
	CPen* pOldPen;
	RECT* prcSrc;
	int xSrc;
	int ySrc;

	pView = Hover_GetView();
	if (pView == NULL)
		return;

	prcSrc = &m_rcSource;

	// Build the raw update rectangle from the base point and the stored
	// extents; make a normalized/inflated copy and union it with the dirty
	// rectangle kept at +0x10.  The original re-reads left/top out of the
	// rectangle for the right/bottom sums instead of reusing the values it
	// just stored, so those two reads go through a volatile lvalue.
	rcRaw.left = (long)m_dwCenterX;
	rcRaw.top = (long)m_dwCenterY;
	rcRaw.right = (long)m_dwNeedleDX + *(volatile long*)&rcRaw.left;
	rcRaw.bottom = (long)m_dwNeedleDY + *(volatile long*)&rcRaw.top;
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
		::BitBlt(m_pTargetDC->m_hDC, prcSrc->left, prcSrc->top,
			prcSrc->right - prcSrc->left, prcSrc->bottom - prcSrc->top,
			m_dc.GetSafeHdc(), 0, 0, SRCCOPY);
	}
	else
	{
		// Copy only the dirty sub-rectangle to its corresponding position.
		::BitBlt(m_pTargetDC->m_hDC, rcDirty.left, rcDirty.top,
			rcDirty.Width(), rcDirty.Height(),
			m_dc.GetSafeHdc(), xSrc, ySrc, SRCCOPY);
	}

	// Draw the update-area indicator using the stored pen (the original
	// does not test m_pPen for NULL).  Every coordinate is re-read out
	// of rcRaw's stack slots.
	pOldPen = m_pTargetDC->SelectObject(m_pPen);
	m_pTargetDC->MoveTo(rcRaw.left, rcRaw.top);
	m_pTargetDC->LineTo(rcRaw.right, rcRaw.bottom);
	m_pTargetDC->MoveTo(rcRaw.left + 1, rcRaw.top);
	m_pTargetDC->LineTo(rcRaw.right + 1, rcRaw.bottom);
	m_pTargetDC->SelectObject(pOldPen);

	// Present the internal bitmap to the destination DC.  pView is only
	// tested here, and the original keeps it in its stack slot across the
	// blit and the four GDI calls above rather than in a register.
	if ((*(CBumperView* volatile*)&pView)->m_bDoubleSize != 0)
	{
		// The 2x present doubles the four rectangle values into locals of
		// their own before deriving the source DC (cf. PresentMap), which
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
				m_pTargetDC->GetSafeHdc(), lLeft, lTop, lWidth, lHeight,
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
				m_pTargetDC->GetSafeHdc(), lLeft, lTop, lWidth, lHeight,
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
				m_pTargetDC->GetSafeHdc(), lLeft, lTop, SRCCOPY);
		}
		else
		{
			long lLeft, lTop, lWidth, lHeight;

			lTop = rcDirty.top;
			lLeft = rcDirty.left;
			lHeight = rcDirty.bottom - lTop;
			lWidth = rcDirty.right - lLeft;
			::BitBlt(pDestDC->m_hDC, lLeft, lTop, lWidth, lHeight,
				m_pTargetDC->GetSafeHdc(), lLeft, lTop, SRCCOPY);
		}
	}

	// Store the inflated update rectangle back as the new dirty region.
	m_rcDirty = rcUpdate;
}

/////////////////////////////////////////////////////////////////////////////
// CMapRenderer

// Fixed-point multiply helper used by ProjectPoint (CFixed is declared in
// hover.h): computes m_dwValue = (m_dwValue * dwB) >> 16.

// hover.exe:0x4038E0
// FUNCTION: HOVER 0x4038e0
void CFixed::MulEq(DWORD dwB)
{
	DWORD dwA = m_dwValue;
	HOVER_FIXMUL(dwA, dwB);
	m_dwValue = dwA;
}

// The 16.16 fixed-point steps of the unoptimized projection helper
// CMapRenderer::ProjectPoint (see the comment there). Each of these is expanded
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

// Hover_Project: the object pass hands ProjectPoint a whole point, not two
// coordinates - the original copies both halves into a fresh adjacent
// 8-byte slot at every call site and pushes out of it, which is what an
// inline forwarder with a by-value POINT parameter produces (a POINT local
// or a plain pair of arguments does not; see rule 139).
static inline void Hover_Project(CMapRenderer* pThis, POINT* pptOut, POINT pt)
{
	pThis->ProjectPoint(pptOut, pt.x, pt.y, g_dwMapCamX, g_dwMapCamY,
		g_dwMapOriginX, g_dwMapOriginY, g_dwMapRotAngle);
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
	fmA.MulEq(fmB.m_dwValue);
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

// Rule 49's "an inline wrapper must copy its result into a local": with a
// plain `return fm.m_dwValue;` the by-value parameter folds away and the
// expansion is two instructions with no slot; the original's projection tail
// reads the value out of a slot of its own before storing it.
static inline DWORD Hover_FixValue(CFixed fm)
{
	DWORD dwR = fm.m_dwValue;
	return dwR;
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
CMapRenderer::CMapRenderer()
{
	m_wHeightFrac = 0;
	m_wHeight = 0;
	m_wWidthFrac = 0;
	m_wWidth = 0;
	m_wHalfHeightFrac = 0;
	m_wHalfHeight = 0;
	m_wHalfWidthFrac = 0;
	m_wHalfWidth = 0;
	m_wProjBaseFrac = 0;
	m_wProjBase = 0;
	m_dwBlinkPhase = 0;
	m_wScaleFrac = 0;
	m_wScale = 0;
	m_dwZoom = 0x40000;
}

// Sets the viewport/projection globals from the human player's camera and
// projects every object in the doc's two object lists; when
// theApp.m_bShowBeacons is set, also visibility-marks and projects the tracked
// object array (g_arrBeacons).

// hover.exe:0x407160
// FUNCTION: HOVER 0x407160
void CMapRenderer::ProjectScene(CBumperDoc* pDoc)
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
	CBeacon* pBeacon;
	CPlayer* pPlayer;
	int i;

	m_dwBlinkPhase++;
	if ((long)m_dwBlinkPhase >= 8)
		m_dwBlinkPhase = 0;
	dwUnk540 = 0;
	pView = Hover_GetView();
	if (pView != NULL)
		dwUnk540 = pView->m_bFullMap;
	// The camera position is read back through GetCellPos's return value
	// (rule 39).
	pPlayer = &pDoc->m_humanPlayer;
	pdwPos = pPlayer->GetCellPos(adwPos);
	g_dwMapCamX = pdwPos[0];
	g_dwMapCamY = pdwPos[1];
	g_dwMapRotAngle = (DWORD)((long)(short)pDoc->m_humanPlayer.m_wHeading
		- 0x80);
	g_dwMapFaceCount = pDoc->m_pWorld->m_arrStatics.GetSize();
	dwHalf = (DWORD)(long)(short)m_wViewRadius;
	g_dwMapCullLeft = g_dwMapCamX - dwHalf;
	g_dwMapCullTop = g_dwMapCamY - dwHalf;
	g_dwMapCullRight = g_dwMapCamX + dwHalf;
	g_dwMapCullBottom = g_dwMapCamY + dwHalf;
	dwScale = m_fxScale;
	pdwPos = pPlayer->GetFixedPos(adwPt);
	dwProj = pdwPos[0];
	HOVER_FIXDIVPL(dwProj, dwScale);
	dwBase = m_fxProjBase;
	dwBase -= dwProj;
	g_dwMapOriginX = dwBase - m_fxHalfWidth;
	dwScale = m_fxScale;
	pdwPos = pPlayer->GetFixedPos(adwPt2);
	dwProj = pdwPos[1];
	HOVER_FIXDIVPL(dwProj, dwScale);
	dwBase = m_fxProjBase;
	dwBase -= dwProj;
	g_dwMapOriginY = dwBase - m_fxHalfHeight;
	pos = theApp.m_pBumperDoc->m_listMazeObjs.GetHeadPosition();
	while (pos != NULL)
	{
		CGameObj* pObj;

		pObj = (CGameObj*)theApp.m_pBumperDoc->m_listMazeObjs.GetNext(pos);
		pObj->m_bOnMap = 0;
		ProjectObject(pDoc, pObj, dwUnk540);
	}
	pos = theApp.m_pBumperDoc->m_listActiveObjs.GetHeadPosition();
	while (pos != NULL)
	{
		CGameObj* pObj;

		pObj = (CGameObj*)theApp.m_pBumperDoc->m_listActiveObjs.GetNext(pos);
		ProjectObject(pDoc, pObj, dwUnk540);
	}
	if (theApp.m_bShowBeacons == 0)
		return;
	for (i = 0; i < g_arrBeacons.GetSize(); i++)
	{
		pBeacon = (CBeacon*)g_arrBeacons[i];
		if ((long)pBeacon->GetCellPos(adwA)[0] < (long)g_dwMapCullLeft)
			pBeacon->m_bOnMap = 0;
		else if ((long)pBeacon->GetCellPos(adwB)[0] > (long)g_dwMapCullRight)
			pBeacon->m_bOnMap = 0;
		else if ((long)pBeacon->GetCellPos(adwC)[1] < (long)g_dwMapCullTop)
			pBeacon->m_bOnMap = 0;
		else
		{
			pBeacon->m_bOnMap = 1;
			if ((long)pBeacon->GetCellPos(adwD)[1] > (long)g_dwMapCullBottom)
				pBeacon->m_bOnMap = 0;
		}
		pBeacon->GetCellPos(adwE);
		ProjectPoint(&pBeacon->m_ptMap, adwE[0], adwE[1], g_dwMapCamX,
			g_dwMapCamY, g_dwMapOriginX, g_dwMapOriginY, g_dwMapRotAngle);
		pBeacon->m_dwMapColor = m_dwColorYellow;
	}
}

// Visibility-tests one object against the viewport rectangle and projects
// its points with ProjectPoint: wall endpoints (+0x84/+0x8C), temp-wall-pod
// rect corners (+0xF0/+0xF8), pod center (+0xC0) or the player triangle
// (+0x164..+0x178). Sets the object's m_bOnMap visible flag and the
// m_dwMapColor draw color.

// hover.exe:0x404BB0
// FUNCTION: HOVER 0x404bb0
void CMapRenderer::ProjectObject(CBumperDoc* pDoc, CGameObj* pObj, int bUnk)
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
		if (pObj->m_bHideOnMap != 0)
			return;
		if (bUnk == 0)
		{
			pSub = (DWORD*)pObj->m_pMerlin;
			if (pSub == NULL || (*(BYTE*)((DWORD)pSub + 0x24) & 8) == 0)
			{
				pWall->m_ptMap1 = ptHidden;
				pWall->m_ptMap2 = ptHidden;
				return;
			}
		}
		if (Hover_PtOf(&pObj->m_seg.m_dwX1).x < (long)g_dwMapCullLeft &&
			Hover_PtOf(&pObj->m_seg.m_dwX2).x < (long)g_dwMapCullLeft)
			return;
		if (Hover_PtOf(&pObj->m_seg.m_dwX1).x > (long)g_dwMapCullRight &&
			Hover_PtOf(&pObj->m_seg.m_dwX2).x > (long)g_dwMapCullRight)
			return;
		if (Hover_PtOf(&pObj->m_seg.m_dwX1).y < (long)g_dwMapCullTop &&
			Hover_PtOf(&pObj->m_seg.m_dwX2).y < (long)g_dwMapCullTop)
			return;
		if (Hover_PtOf(&pObj->m_seg.m_dwX1).y > (long)g_dwMapCullBottom &&
			(long)pObj->m_seg.m_dwY2 > (long)g_dwMapCullBottom)
			return;
		Hover_Project(this, &pWall->m_ptMap1,
			Hover_PtOf(&pObj->m_seg.m_dwX1));
		Hover_Project(this, &pWall->m_ptMap2,
			Hover_PtOf(&pObj->m_seg.m_dwX2));
		pObj->m_bOnMap = 1;
		return;
	}
	if (pObj->IsKindOf(RUNTIME_CLASS(CTempWallPod)) && pObj->m_bInUse != 0)
	{
		pPod = (CPodObj*)pObj;
		pRect = (DWORD*)pPod->m_pForceField;
		if ((long)pRect[5] < (long)g_dwMapCullLeft &&
			(long)pRect[7] < (long)g_dwMapCullLeft)
			return;
		if ((long)pRect[5] > (long)g_dwMapCullRight &&
			(long)pRect[7] > (long)g_dwMapCullRight)
			return;
		if ((long)pRect[6] < (long)g_dwMapCullTop &&
			(long)pRect[8] < (long)g_dwMapCullTop)
			return;
		if ((long)pRect[6] > (long)g_dwMapCullBottom &&
			(long)pRect[8] > (long)g_dwMapCullBottom)
			return;
		// Both corners go through the by-value POINT forwarder: the
		// original fills two fresh adjacent 8-byte slot pairs
		// (hover.exe:0x404e1e/0x404e2e and 0x404e3c/0x404e48) before the
		// first call and reloads them for the pushes.
		Hover_Project(this, &pPod->m_ptMapWall1, *(POINT*)&pRect[5]);
		Hover_Project(this, &pPod->m_ptMapWall2, *(POINT*)&pRect[7]);
		pObj->m_bOnMap = 1;
		return;
	}
	if (pObj->IsKindOf(RUNTIME_CLASS(CPodObj)))
	{
		pPod = (CPodObj*)pObj;
		if (pPod->m_pOwner != 0)
			return;
		if (bUnk == 0)
		{
			pSub = (DWORD*)pObj->m_pMerlin;
			if (pSub == NULL || (*(BYTE*)((DWORD)pSub + 0x24) & 8) == 0)
				goto HidePod;
		}
		if (pObj->m_bHideOnMap != 0)
		{
			// The three "not drawn" exits share one store block in the
			// original, placed after the projection code.
HidePod:
			pPod->m_ptMap.x = -10;
			pPod->m_ptMap.y = -10;
			return;
		}
		pPod->GetCellPos(adwPt);
		// The four clip tests re-read the projected centre from memory and
		// keep the viewport bound in the register (rule 121).
		plPt = (volatile long*)adwPt;
		lClip = (long)g_dwMapCullLeft;
		if (plPt[0] < lClip)
			return;
		lClip = (long)g_dwMapCullRight;
		if (plPt[0] > lClip)
			return;
		lClip = (long)g_dwMapCullTop;
		if (plPt[1] < lClip)
			return;
		lClip = (long)g_dwMapCullBottom;
		if (plPt[1] > lClip)
			return;
		Hover_Project(this, &pPod->m_ptMap, *(POINT*)adwPt);
		pObj->m_bOnMap = 1;
		if (pObj->IsKindOf(RUNTIME_CLASS(CFlag)))
		{
			if ((long)m_dwBlinkPhase < 4)
			{
				pObj->m_bOnMap = 0;
				return;
			}
			if (pPod->m_bRobotFlag == 0)
				pObj->m_dwMapColor = m_dwColorHuman;
			else
				pObj->m_dwMapColor = m_dwColorRobot;
		}
		else
		{
			pObj->m_dwMapColor = m_dwColorYellow;
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
		dwWide = m_fxScale;
		dwCap = m_fxScale;
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
		lAngle = (short)pPlayer->m_wHeading;
		dwP1 = g_adwSinTable[(((lAngle + 0x80) & 0x1FF) << 4) + 1];
		HOVER_FIXMULP(dwP1, dwT);
		g_dwMapRotAngle = lAngle;
		dwCosHalf = dwP1;
		dwP2 = g_adwSinTable[((lAngle & 0x1FF) << 4) + 1];
		HOVER_FIXMULP(dwP2, dwT);
		lAngle = (short)pPlayer->m_wHeading - 0x80;
		dwSinHalf = dwP2;
		dwP3 = g_adwSinTable[(((lAngle + 0x80) & 0x1FF) << 4) + 1];
		HOVER_FIXMULP(dwP3, dwCapMax);
		g_dwMapRotAngle = lAngle;
		dwSinCap = dwP3;
		dwP4 = g_adwSinTable[((lAngle & 0x1FF) << 4) + 1];
		HOVER_FIXMULP(dwP4, dwCapMax);
		dwNegCosCap = dwP4;
		pdwPt = pPlayer->GetFixedPos(adwPl);
		// The triangle origin is one POINT copied into all three corners;
		// hover.exe:0x405235 stores it into its own slot pair and reloads
		// it for the second and third store.
		ptOrg.x = ((short*)pdwPt)[1];
		ptOrg.y = ((short*)pdwPt)[3];
		pPt1 = &pPlayer->m_aptMapTri[0];
		pPt2 = &pPlayer->m_aptMapTri[1];
		pPt3 = &pPlayer->m_aptMapTri[2];
		*pPt1 = ptOrg;
		*pPt2 = ptOrg;
		*pPt3 = ptOrg;
		lHiCos = ((short*)&dwCosHalf)[1];
		lHiSin = ((short*)&dwSinHalf)[1];
		pPlayer->m_aptMapTri[0].x += lHiCos;
		pPlayer->m_aptMapTri[0].y += lHiSin;
		lSinCap = ((short*)&dwSinCap)[1];
		lT = lSinCap;
		lT -= lHiCos;
		pPt2->x += lT;
		lNegCos = ((short*)&dwNegCosCap)[1];
		lU = lNegCos;
		lU -= lHiSin;
		pPt3->x -= lHiCos + lSinCap;
		pPlayer->m_aptMapTri[1].y += lU;
		pPlayer->m_aptMapTri[2].y -= lHiSin + lNegCos;
		lAngle = (short)pDoc->m_humanPlayer.m_wHeading - 0x80;
		g_dwMapRotAngle = lAngle;
		// The three triangle projections pass the two coordinates as
		// scalars read back through the POINT* locals, not as a POINT by
		// value: hover.exe:0x40530e..0x405381 pushes straight out of
		// [esi+0x164]/[ebp]/[ebx] with no 8-byte argument temp.
		ProjectPoint(pPt1, pPt1->x, pPlayer->m_aptMapTri[0].y,
			g_dwMapCamX, g_dwMapCamY, g_dwMapOriginX,
			g_dwMapOriginY, g_dwMapRotAngle);
		ProjectPoint(pPt2, pPt2->x, pPt2->y, g_dwMapCamX,
			g_dwMapCamY, g_dwMapOriginX, g_dwMapOriginY,
			g_dwMapRotAngle);
		ProjectPoint(pPt3, pPt3->x, pPt3->y, g_dwMapCamX,
			g_dwMapCamY, g_dwMapOriginX, g_dwMapOriginY,
			g_dwMapRotAngle);
		pPlayer->m_dwUnk017C = 1;
		return;
	}
}

// Projects the world point (lX,lY) into pdwOut[0..1]: when
// theApp.m_bRotateMap is set the point is first rotated by -lAngle around
// (lCX,lCY); the projection divides by the m_wScaleFrac factor and offsets
// into the view origin (m_rcView.left/m_rcView.top).
//
// The rotation is done with 16.16 fixed-point values and the sin/cos table:
// lRX = (cos*dx + sin*dy) >> 16, lRY = (cos*dy - sin*dx) >> 16.
//
// This function is one of the two in hover.exe that are NOT optimized (the
// other is the robot steering pass, see rule 49 in notes/perfect-match.md):
// it has an ebp frame, every value homed on the stack and a five-byte
// `jmp $+5` at the end of every inline expansion, while still expanding
// inline functions and still calling CFixed::MulEq out of line. That
// combination is `#pragma optimize("g", off)` on top of our /O2 build (see
// rule 117). Every fixed-point step is therefore visible as an expansion of
// one of the Hover_Fix* helpers above.

#pragma optimize("g", off)

// hover.exe:0x4055B0
// FUNCTION: HOVER 0x4055b0
void CMapRenderer::ProjectPoint(POINT* pptOut, long lX, long lY, long lCX,
	long lCY, long lV1, long lV2, long lAngle)
{
	long lQY;
	long lPX;
	long lPY;

	{
		long lSX;
		__int64 llQ;

		lPX = lX;
		lPY = lY;
		if (theApp.m_bRotateMap != 0)
		{
			// dwA/dwB are the 16.16 deltas from (lCX,lCY), dwC/dwE the
			// sine and cosine of lAngle, dwD/dwF their products with the
			// deltas and dwG their sum (the rotated x); dwH..dwL are the
			// same five values again for the rotated y. The one-letter
			// names are the /Od frame (rule 195): their buckets have to
			// ascend in declaration order to land on ebp-0x1c..ebp-0x48.
			DWORD dwA;
			DWORD dwB;
			DWORD dwC;
			DWORD dwD;
			DWORD dwE;
			DWORD dwF;
			DWORD dwG;
			DWORD dwH;
			DWORD dwI;
			DWORD dwJ;
			DWORD dwK;
			DWORD dwL;

			Hover_FixSetInt(dwA, lPX);
			Hover_FixSetInt(dwB, lPY);
			Hover_FixSubInt(dwA, lCX);
			Hover_FixSubInt(dwB, lCY);
			Hover_FixSinTo(dwC, lAngle);
			Hover_FixCosTo(dwE, lAngle);
			Hover_FixMulTo(dwD, *(CFixed*)&dwC, *(CFixed*)&dwB);
			Hover_FixMulTo(dwF, *(CFixed*)&dwE, *(CFixed*)&dwA);
			Hover_FixAddTo(dwG, *(CFixed*)&dwF, *(CFixed*)&dwD);
			lPX = ((short*)&dwG)[1];
			Hover_FixSinTo(dwH, lAngle);
			Hover_FixCosTo(dwJ, lAngle);
			Hover_FixMulTo(dwI, *(CFixed*)&dwH, *(CFixed*)&dwA);
			Hover_FixMulTo(dwK, *(CFixed*)&dwJ, *(CFixed*)&dwB);
			Hover_FixSubTo(dwL, *(CFixed*)&dwK, *(CFixed*)&dwI);
			lPY = ((short*)&dwL)[1];
			lPX += lCX;
			lPY += lCY;
		}
		llQ = lPX;
		Hover_Shl32(llQ);
		llQ = llQ / (long)m_fxScale;
		Hover_FixPack(lSX, llQ);
		llQ = lPY;
		Hover_Shl32(llQ);
		llQ = llQ / (long)m_fxScale;
		Hover_FixPack(lQY, llQ);
		{
			// Both subtractions go through the three-argument
			// Hover_FixSubTo, not a nested pair of Hover_FixSub calls:
			// only the `DWORD&` destination form gives CFixed::SubEq's
			// argument a slot of its own (the highest of the three) and
			// stores the two by-value copies right to left.  dwP..dwS
			// are those destinations, at ebp-0x4c..ebp-0x58.
			CFixed fm1;
			CFixed fm2;
			CFixed fm3;
			CFixed fm4;

			Hover_FixSubTo(fm1.m_dwValue,
				*(CFixed*)&m_fxProjBase, *(CFixed*)&lSX);
			Hover_FixSubTo(fm2.m_dwValue, fm1, *(CFixed*)&lV1);
			lSX = (long)Hover_FixValue(fm2);
			Hover_FixSubTo(fm3.m_dwValue,
				*(CFixed*)&m_fxProjBase, *(CFixed*)&lQY);
			Hover_FixSubTo(fm4.m_dwValue, fm3, *(CFixed*)&lV2);
			lQY = (long)Hover_FixValue(fm4);
		}
		pptOut->x = m_rcView.left + (long)((short*)&lSX)[1];
		pptOut->y = m_rcView.top + (long)((short*)&lQY)[1];
	}
}
#pragma optimize("", on)

// Draws every projected object into the pixel bits (walls/temp-walls as
// lines, pods as cross markers, players as triangles), then the tracked
// object markers with their selection diamonds and link lines.

// hover.exe:0x4053A0
// FUNCTION: HOVER 0x4053a0
void CMapRenderer::DrawScene(CBumperView* pView, CBumperDoc* pDoc)
{
	POSITION pos;
	CGameObj* pObj;
	CBeacon* pBeacon;
	CBeacon* pLinked;
	POINT* pPt;
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

	pos = theApp.m_pBumperDoc->m_listMazeObjs.GetHeadPosition();
	while (pos != NULL)
	{
		pObj = (CGameObj*)theApp.m_pBumperDoc->m_listMazeObjs.GetNext(pos);
		DrawObject(pObj, pView->m_bFullMap);
	}
	pos = theApp.m_pBumperDoc->m_listActiveObjs.GetHeadPosition();
	while (pos != NULL)
	{
		pObj = (CGameObj*)theApp.m_pBumperDoc->m_listActiveObjs.GetNext(pos);
		DrawObject(pObj, pView->m_bFullMap);
	}
	if (theApp.m_bShowBeacons == 0)
		return;
	if (g_arrBeacons.GetSize() <= 0)
		return;
	// The beacon walk keeps its byte offset and its remaining count in two
	// stack slots and counts the latter down (dec [esp+0x18] / jne).
	i = 0;
	nLeft = g_arrBeacons.GetSize();
	do
	{
		pBeacon = (CBeacon*)g_arrBeacons[i];
		if (pBeacon->m_bOnMap != 0)
		{
			bColor = (BYTE)pBeacon->m_dwMapColor;
			m_bColor = bColor;
			wColor = (WORD)(bColor | ((WORD)bColor << 8));
			m_dwColor4 = ((DWORD)wColor << 16) | wColor;
			// The projected marker point is reached through a pointer to the
			// +0x9C pair, as in the original (lea edi,[esi+0x9c] / [edi],
			// [edi+4]).
			pPt = &pBeacon->m_ptMap;
			nSub = 0;
			DrawMarker(pPt->x, pPt->y, 5);
			if (pBeacon->m_bTargeted != 0)
			{
				// The selection diamond walks one mutable corner pair around
				// the marker: the original keeps the four coordinates in stack
				// slots and adjusts them by +-0x14 between the four calls.
				pPtA = &ptA;
				pPtB = &ptB;
				lX = pPt->x;
				lY = pPt->y;
				pPtA->x = lX;
				pPtB->x = lX;
				pPtA->y = lY;
				pPtB->y = lY;
				pPtA->y -= 0xA;
				pPtB->x += 0xA;
				DrawLine(pPtA->x, pPtA->y, pPtB->x, pPtB->y);
				pPtA->y += 0x14;
				DrawLine(pPtB->x, pPtB->y, pPtA->x, pPtA->y);
				pPtB->x -= 0x14;
				DrawLine(pPtA->x, pPtA->y, pPtB->x, pPtB->y);
				pPtA->y -= 0x14;
				DrawLine(pPtB->x, pPtB->y, pPtA->x, pPtA->y);
			}
			for (j = pBeacon->m_arrLinks.GetSize(); j > 0; j--)
			{
				POINT* pField;

				pLinked = (CBeacon*)pBeacon->m_arrLinks[nSub];
				nSub++;
				pField = &pLinked->m_ptMap;
				DrawLine(pPt->x, pPt->y, pField->x, pField->y);
			}
		}
		i++;
	} while (--nLeft != 0);
}

// Draws one projected object: walls as a line, temp-wall pods as a line,
// pods as a cross marker, players as a triangle, each in its class color.

// hover.exe:0x401C00
// FUNCTION: HOVER 0x401c00
void CMapRenderer::DrawObject(CGameObj* pObj, int bUnk)
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
		if (pObj->m_bHideOnMap != 0)
			return;
		if (bUnk == 0)
		{
			pSub = (DWORD*)pObj->m_pMerlin;
			if (pSub == NULL || (*(BYTE*)((DWORD)pSub + 0x24) & 8) == 0)
				return;
		}
		if (pObj->m_bOnMap == 0)
			return;
		pSub = (DWORD*)pObj->m_pMerlin;
		if (pSub[0x2F] == 0)
			bColor = (BYTE)m_dwColorYellow;
		else if (((short*)pSub)[0x52] > 0)
			bColor = (BYTE)m_dwColorGray;
		else
			bColor = (BYTE)m_dwColorWhite;
		m_bColor = bColor;
		wColor = (WORD)(bColor | ((WORD)bColor << 8));
		m_dwColor4 = ((DWORD)wColor << 16) | wColor;
		pPt2 = &pWall->m_ptMap2;
		pPt1 = &pWall->m_ptMap1;
		DrawLine(pPt1->x, pPt1->y, pPt2->x, pPt2->y);
		return;
	}
	if (pObj->IsKindOf(RUNTIME_CLASS(CTempWallPod)) && pObj->m_bInUse != 0)
	{
		pPod = (CPodObj*)pObj;
		bColor = (BYTE)m_dwColorWhite;
		m_bColor = bColor;
		wColor = (WORD)(bColor | ((WORD)bColor << 8));
		m_dwColor4 = ((DWORD)wColor << 16) | wColor;
		pPt2 = &pPod->m_ptMapWall2;
		pPt1 = &pPod->m_ptMapWall1;
		DrawLine(pPt1->x, pPt1->y, pPt2->x, pPt2->y);
		return;
	}
	if (pObj->IsKindOf(RUNTIME_CLASS(CPodObj)))
	{
		pPod = (CPodObj*)pObj;
		if (bUnk == 0)
		{
			pSub = (DWORD*)pObj->m_pMerlin;
			if (pSub == NULL || (*(BYTE*)((DWORD)pSub + 0x24) & 8) == 0)
				return;
		}
		if (pObj->m_bOnMap == 0)
			return;
		bColor = (BYTE)pObj->m_dwMapColor;
		m_bColor = bColor;
		wColor = (WORD)(bColor | ((WORD)bColor << 8));
		m_dwColor4 = ((DWORD)wColor << 16) | wColor;
		pPt1 = &pPod->m_ptMap;
		DrawMarker(pPt1->x, pPt1->y, 5);
		return;
	}
	if (pObj->IsKindOf(RUNTIME_CLASS(CPlayer)))
	{
		pPlayer = (CPlayer*)pObj;
		if (pObj->IsKindOf(RUNTIME_CLASS(CHumanPlayer)))
		{
			bColor = (BYTE)m_dwColorHuman;
			m_bColor = bColor;
			wColor = (WORD)(bColor | ((WORD)bColor << 8));
			m_dwColor4 = ((DWORD)wColor << 16) | wColor;
		}
		else if (pObj->IsKindOf(RUNTIME_CLASS(CRobotPlayer)))
		{
			if (*(DWORD*)((BYTE*)pPlayer + 0xC0) != 0)
				bColor = (BYTE)m_dwColorYellow;
			else if (((CRobotPlayer*)pPlayer)->m_bHunter != 0)
				bColor = (BYTE)m_dwColorRobotAlt;
			else
				bColor = (BYTE)m_dwColorRobot;
			m_bColor = bColor;
			wColor = (WORD)(bColor | ((WORD)bColor << 8));
			m_dwColor4 = ((DWORD)wColor << 16) | wColor;
		}
		pPt2 = &pPlayer->m_aptMapTri[1];
		pPt1 = &pPlayer->m_aptMapTri[0];
		pPt3 = &pPlayer->m_aptMapTri[2];
		DrawLine(pPt1->x, pPt1->y, pPt2->x, pPt2->y);
		DrawLine(pPt2->x, pPt2->y, pPt3->x, pPt3->y);
		DrawLine(pPt3->x, pPt3->y, pPt1->x, pPt1->y);
		return;
	}
}

// Bresenham line into the pixel bits, clipped by ClipEndpoint and drawn in
// the current color (m_bColor); y is flipped against m_dwBitsHeight.

// hover.exe:0x401980
// FUNCTION: HOVER 0x401980
void CMapRenderer::DrawLine(long lX1, long lY1, long lX2, long lY2)
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

	m_dwLineDX = abs(plA[0] - plB[0]);
	m_dwLineDY = abs(plA[1] - plB[1]);
	if (ClipEndpoint(plA, plB) == 0)
		return;
	if (ClipEndpoint(plB, plA) == 0)
		return;
	plA[1] = m_dwBitsHeight - plA[1] - 1;
	plB[1] = m_dwBitsHeight - plB[1] - 1;
	lDX = abs(plA[0] - plB[0]);
	m_dwLineDX = lDX;
	lDY = abs(plA[1] - plB[1]);
	m_dwLineDY = lDY;
	if (lDX == 0 && lDY == 0)
		return;
	lPitch = m_dwPitch;
	pDst = m_pBits + lPitch * plA[1] + plA[0];
	if (lDX > lDY)
	{
		lSlope = (lDY << 16) / lDX;
		lErr = 0x8000;
		if (plB[1] < plA[1])
			lPitch = -lPitch;
		lStep = (plB[0] < plA[0]) ? -1 : 1;
		bColor = m_bColor;
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
		bColor = m_bColor;
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
void CMapRenderer::DrawMarker(long lX, long lY, long /*lUnk*/)
{
	register long lYFlip;
	register BYTE* pBits;
	register long lPitch;
	BYTE bColor;
	BYTE* pUp;
	BYTE* pDown;
	BYTE* pCenter;

	if (m_rcClip.top + 1 > lY)
		return;
	if (m_rcClip.left + 1 > lX)
		return;
	if (m_rcClip.bottom - 1 < lY)
		return;
	if (m_rcClip.right - 1 < lX)
		return;

	lYFlip = (long)m_dwBitsHeight - lY;
	pBits = m_pBits;
	lPitch = (long)m_dwPitch;

	pUp = pBits + (lYFlip - 2) * lPitch + lX;
	lYFlip--;
	bColor = m_bColor;
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
// m_dwUnk30 by the caller (DrawLine).

// hover.exe:0x401520
// FUNCTION: HOVER 0x401520
int CMapRenderer::ClipEndpoint(long* plA, long* plB)
{
	// The clip bounds are re-read from the object at every comparison in
	// the original (`cmp eax, [ecx]`); reading them through a volatile
	// pointer keeps VC from CSE-ing them into two stack slots, which is
	// what grew our frame from 4 bytes to 0x18 and spilled plB.
	volatile DWORD* pdwClip = (volatile DWORD*)&m_rcClip.left;
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
			if (m_dwLineDX == 0 || m_dwLineDY == 0)
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
			if (m_dwLineDX == 0 || m_dwLineDY == 0)
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
			if (m_dwLineDX == 0)
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
			if (m_dwLineDX == 0 || m_dwLineDY == 0)
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
			if (m_dwLineDX == 0 || m_dwLineDY == 0)
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
			if (m_dwLineDX == 0)
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
			if (m_dwLineDY == 0)
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
			if (m_dwLineDY == 0)
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
void CMapRenderer::PresentMap(CDC* pDC)
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

	pView = Hover_GetView();
	if (pView == NULL)
		return;
	if (pView->m_bDoubleSize != 0)
	{
		// The 2x present doubles the four rectangle values into locals of
		// their own before deriving the source DC, which is what puts the
		// doubled values in stack slots in the original.
		lTop = m_rcView.top;
		lHeight = m_rcView.bottom - lTop;
		lLeft = m_rcView.left;
		lWidth = m_rcView.right - lLeft;
		l2Height = lHeight * 2;
		l2Width = lWidth * 2;
		l2Top = lTop * 2;
		l2Left = lLeft * 2;
		StretchBlt(pDC->m_hDC, l2Left, l2Top, l2Width, l2Height,
			(m_pMapDC == NULL) ? NULL : m_pMapDC->m_hDC,
			lLeft, lTop, lWidth, lHeight, SRCCOPY);
	}
	else
	{
		BitBlt(pDC->m_hDC, m_rcView.left, m_rcView.top,
			m_rcView.right - m_rcView.left, m_rcView.bottom - m_rcView.top,
			(m_pMapDC == NULL) ? NULL : m_pMapDC->m_hDC,
			m_rcView.left, m_rcView.top, SRCCOPY);
	}
}



/////////////////////////////////////////////////////////////////////////////
// CDigitDisplay

// hover.exe:0x422BD0
// FUNCTION: HOVER 0x422bd0
CDigitDisplay::CDigitDisplay()
{
	// m_dwUnk00 is left uninitialised, as in hover.exe
	m_dwDigits = 0;
	m_dwDisplayValue = 0;
	m_pStripBits = NULL;
	m_dwStripWidth = 0;
	m_dwStripHeight = 0;
}

// hover.exe:0x422C20
// FUNCTION: HOVER 0x422c20
CDigitDisplay::~CDigitDisplay()
{
	::operator delete(m_pStripBits);
}

/////////////////////////////////////////////////////////////////////////////
// CSegmentGauge

// hover.exe:0x422C80
// FUNCTION: HOVER 0x422c80
CSegmentGauge::CSegmentGauge()
{
	m_rc.left = 0;
	m_rc.top = 0;
	m_rc.right = 10;
	m_rc.bottom = 10;
	m_pBrushFilled = 0;
	m_pPen = 0;
	m_pBrushEmpty = 0;
	m_dwSegmentSize = 0;
	m_dwSegments = 1;
	m_dwLevel = 0;
}

/////////////////////////////////////////////////////////////////////////////
// CFlagStrip

// hover.exe:0x422C30
// FUNCTION: HOVER 0x422c30
CFlagStrip::CFlagStrip()
{
	// The original only clears these eight members; m_dwUnk04/08/40/48 and
	// both source rectangles are left uninitialized here and filled in by
	// LoadIcons/SetPosition before the element is drawn.  The +0xC/+0x10
	// pair is written through a pointer local (rule 94: lea esi,[this+0xc]).
	m_bFlagsDirty = 1;
	m_dwUnk4C = 0;
	m_dwFilled = 0;

	{
		POINT ptZero;

		ptZero.x = 0;
		ptZero.y = 0;

		m_pFullBits = NULL;
		m_dwSlots = 1;
		m_pEmptyBits = NULL;
		*(POINT*)&m_pUnk0C = ptZero;
	}
}

// hover.exe:0x422C60
// FUNCTION: HOVER 0x422c60
CFlagStrip::~CFlagStrip()
{
	::operator delete(m_pFullBits);
	::operator delete(m_pEmptyBits);
}

// Draws the dashboard sprite element into the render target.  LoadIcons/
// SetPosition precompute the destination base pointer (dwDestBits) and the
// sprite's y-coordinate/extents; this routine then draws m_dwSlots iterations,
// tiling the sprite across the destination: the left edge advances by one
// sprite width per iteration and the right edge stays one width ahead of it.
// The first m_dwFilled iterations draw from m_pFullBits/m_rcFull,
// the rest from m_pEmptyBits/m_rcEmpty.
//
// hover.exe:0x40E9A0
// FUNCTION: HOVER 0x40e9a0
void CFlagStrip::DrawFlags(BYTE* pDestBits, DWORD dwX, DWORD dwY)
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
	CFlagStrip* pThis;

	pRect1 = &m_rcFull;
	s.lWidth = m_rcFull.right - pRect1->left;
	pThis = this;
	s.lDestY = pThis->m_dwDestY;
	s.lDestBottom = pThis->m_rcFull.bottom - pThis->m_rcFull.top
		+ s.lDestY;

	if ((long)pThis->m_dwSlots <= 0)
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

		lX = pThis->m_dwDestX + s.lOffsetX;
		::SetRect(&s.rcDest, lX, s.lDestY, pThis->m_dwDestX + lW,
			s.lDestBottom);

		if ((int)pThis->m_dwFilled >= i + 1)
		{
			pSrc = (BYTE*)pThis->m_pFullBits;
			pSrcRect = pRect1;
		}
		else
		{
			pSrc = (BYTE*)pThis->m_pEmptyBits;
			pSrcRect = &pThis->m_rcEmpty;
		}

		Hover_BlitRect(
			pSrcRect->left, pSrcRect->top, pSrcRect->right, pSrcRect->bottom,
			(long)pThis->m_pUnk0C, (long)pThis->m_pUnk10,
			s.rcDest.left, s.lDestY, s.rcDest.right, s.rcDest.bottom,
			dwX, dwY,
			pSrc, pDestBits, 1);

		i++;
		lW += s.lWidth;
		s.lOffsetX += s.lWidth;
	}
	while (i < (int)pThis->m_dwSlots);

done:
	pThis->m_bFlagsDirty = 0;
}

// Draws the decimal value stored in m_dwDisplayValue as a right-aligned
// string of m_dwDigits digits into the render target.  Each digit is taken
// from the source tile bitmap at m_pStripBits (ten tiles of size
// m_dwDigitWidth x m_dwDigitHeight laid out horizontally) and blitted
// through Hover_BlitRect.

// hover.exe:0x40FCE0
// FUNCTION: HOVER 0x40fce0
void CDigitDisplay::DrawDigits(BYTE* pDestBits, DWORD dwX, DWORD dwY)
{
	long lValue;
	int nTop;
	int nBottom;
	RECT rcSrc;
	RECT rcDst;
	int nIndex;
	int nDigit;
	DWORD* pdwOrigin;

	nTop = (int)m_dwDestY;
	nBottom = (int)m_dwDigitHeight + nTop;
	lValue = (long)m_dwDisplayValue;
	nIndex = (int)m_dwDigits - 1;

	if (nIndex < 0)
		goto Done;

	// The origin pair is read through a pointer local (rule 94).
	pdwOrigin = &m_dwStripWidth;

	do
	{
		int nSrcLeft;

		nDigit = (int)(lValue % 10);
		nSrcLeft = nDigit * (int)m_dwDigitWidth;

		::SetRect(&rcSrc,
			nSrcLeft,
			0,
			nSrcLeft + (int)m_dwDigitWidth,
			(int)m_dwDigitHeight);

		::SetRect(&rcDst,
			(int)m_dwDestX + nIndex * (int)m_dwDigitWidth,
			nTop,
			(int)m_dwDestX + (nIndex + 1) * (int)m_dwDigitWidth,
			nBottom);

		Hover_BlitRect(
			(long)rcSrc.left, (long)rcSrc.top,
			(long)rcSrc.right, (long)rcSrc.bottom,
			(long)pdwOrigin[0], (long)pdwOrigin[1],
			(long)rcDst.left, (long)rcDst.top,
			(long)rcDst.right, (long)rcDst.bottom,
			(long)dwX, (long)dwY,
			m_pStripBits,
			pDestBits,
			1);

		lValue /= 10;
		nIndex--;
	} while (nIndex >= 0);

Done:
	m_bDigitsDirty = 0;
}

// Draws the gauge into pDC as a series of rectangles. The gauge is laid out
// in m_rc; m_bHorizontal selects horizontal (non-zero) or vertical (zero)
// orientation. Each segment is m_dwSegmentSize units wide/tall. Segments
// m_dwLevel..m_dwSegments-1 are filled with m_pBrushEmpty, the earlier
// ones with m_pBrushFilled, using the outline pen m_pPen for all segments.
// When bUnk is zero the loop stops at max(m_dwLevel, m_dwLastLevel) to
// redraw only the changed range. Coordinates are doubled when the view is
// in stretch mode and pDC is not the offscreen DC.
//
// hover.exe:0x401AD0
// FUNCTION: HOVER 0x401ad0
void CSegmentGauge::DrawGauge(CDC* pDC, int bUnk)
{
	int nMax;
	int nLeft;
	int nBottom;
	int nTop;
	int nRight;
	int i;

	nMax = ((int)m_dwLastLevel > (int)m_dwLevel) ? (int)m_dwLastLevel : (int)m_dwLevel;
	m_dwLastLevel = m_dwLevel;

	nLeft = m_rc.left;
	nBottom = m_rc.bottom;
	nTop = m_rc.top;
	nRight = m_rc.right;

	pDC->SaveDC();

	pDC->SelectObject(m_pPen);
	pDC->SelectObject(m_pBrushFilled);

	for (i = 0; i < (int)m_dwSegments; i++)
	{
		if (bUnk == 0 && i > nMax)
			break;

		if (m_bHorizontal != 0)
		{
			nRight = nLeft + (int)m_dwSegmentSize;
		}
		else
		{
			nTop = nBottom - (int)m_dwSegmentSize;
		}

		pDC->SelectObject(((int)m_dwLevel <= i)
			? m_pBrushEmpty
			: m_pBrushFilled);

		if (Hover_GetView()->m_bDoubleSize != 0
			&& pDC != &Hover_GetView()->m_dcOffscreen)
		{
			::Rectangle(pDC->m_hDC,
				nLeft * 2, nTop * 2, nRight * 2, nBottom * 2);
		}
		else
		{
			::Rectangle(pDC->m_hDC,
				nLeft, nTop, nRight, nBottom);
		}

		if (m_bHorizontal != 0)
			nLeft = nRight;
		else
			nBottom = nTop;
	}

	pDC->RestoreDC(-1);
}

// Stores the gauge's rendering parameters: segment count, inactive brush,
// outline pen and active brush. The original keeps these as raw DWORDs that
// are later cast back to CGdiObject-derived pointers by DrawGauge.
// hover.exe:0x411D60
// FUNCTION: HOVER 0x411d60
void CSegmentGauge::SetStyle(DWORD dwSegments, CBrush* pBrushFilled, CPen* pPen,
	CBrush* pBrushEmpty)
{
	m_pBrushFilled = pBrushFilled;
	m_pPen = pPen;
	m_pBrushEmpty = pBrushEmpty;
	m_dwSegments = dwSegments;
}

// Sets up a dashboard number element. The source bitmap pBits is dwA x dwB.
// pRect points to the first of ten contiguous RECTs that describe the digit
// tiles 0..9 in pBits; the first rect's left/top gives the strip origin and
// the tenth rect's right/bottom gives the far end. A buffer large enough for
// ten tiles laid out horizontally is allocated at m_pStripBits;
// m_dwDigitWidth/m_dwDigitHeight hold the tile size and
// m_dwStripWidth/m_dwStripHeight hold the buffer dimensions.

// hover.exe:0x411A00
// FUNCTION: HOVER 0x411a00
void CDigitDisplay::LoadDigits(int nFlag, const RECT* pRect, BYTE* pBits,
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

	::operator delete(m_pStripBits);

	m_bDigitsDirty = 1;
	m_dwDigits = nFlag;

	nWidth = pRect->right - pRect->left;
	nHeight = pRect->bottom - pRect->top;

	pUnk0C = &m_dwStripWidth;

	m_dwDigitWidth = nWidth;
	m_dwDigitHeight = nHeight;

	nPitch = nWidth * 10;
	*pUnk0C = nPitch;
	if (nPitch % 2 != 0)
		nPitch++;
	*pUnk0C = nPitch;
	m_dwStripHeight = nHeight;

	m_pStripBits = (BYTE*)::operator new((*pUnk0C) * nHeight);

	dwLeft = pRect->left;
	dwTop = pRect->top;
	dwRight9 = pRect[9].right;
	dwBottom9 = pRect[9].bottom;
	dwZero1 = 0;
	dwZero2 = 0;
	dwPitch = pUnk0C[0];
	dwHeight = m_dwStripHeight;

	Hover_BlitRect(
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
		m_pStripBits,
		1);

	m_dwDestX = 0;
	m_dwDestY = 0;
}

// Copies pPoint into the destination point stored at +0x20.
// hover.exe:0x411920
// FUNCTION: HOVER 0x411920
void CDigitDisplay::SetPosition(const POINT* pPoint)
{
	m_dwDestX = pPoint->x;
	m_dwDestY = pPoint->y;
}

// hover.exe:0x411BB0
// FUNCTION: HOVER 0x411bb0
void CFlagStrip::LoadIcons(DWORD dwUnk, DWORD dwLeft1, DWORD dwTop1,
	DWORD dwRight1, DWORD dwBottom1, DWORD dwLeft2, DWORD dwTop2,
	DWORD dwRight2, DWORD dwBottom2, BYTE* pBits, DWORD dwWidth, DWORD dwHeight)
{
	RECT* pSource1 = &m_rcFull;
	DWORD* pUnk0C = (DWORD*)&m_pUnk0C;
	long nRounded;

	m_dwSlots = dwUnk;
	*pUnk0C = dwRight1 - dwLeft1;
	m_pUnk10 = (void*)(dwBottom1 - dwTop1);

	::SetRect(pSource1, 0, 0, (int)*pUnk0C, (int)m_pUnk10);
	::SetRect(&m_rcEmpty, 0, 0, (int)*pUnk0C, (int)m_pUnk10);

	::operator delete(m_pFullBits);
	::operator delete(m_pEmptyBits);

	// The tile pitch is rounded up to an even number with the signed
	// remainder idiom (cdq / xor / sub / and 1 / xor / sub).
	nRounded = (long)*pUnk0C;
	if (nRounded % 2 != 0)
		nRounded++;

	m_pFullBits = ::operator new((DWORD)m_pUnk10 * nRounded);
	m_pEmptyBits = ::operator new((DWORD)m_pUnk10 * nRounded);

	Hover_BlitRect(
		(long)dwLeft1, (long)dwTop1, (long)dwRight1, (long)dwBottom1,
		(long)dwWidth, (long)dwHeight,
		(long)pSource1->left, (long)pSource1->top,
		(long)pSource1->right, (long)pSource1->bottom,
		(long)pUnk0C[0], (long)pUnk0C[1],
		pBits, (BYTE*)m_pFullBits, 1);

	{
		RECT* pSource2 = &m_rcEmpty;

		Hover_BlitRect(
			(long)dwLeft2, (long)dwTop2, (long)dwRight2, (long)dwBottom2,
			(long)dwWidth, (long)dwHeight,
			(long)pSource2->left, (long)pSource2->top,
			(long)pSource2->right, (long)pSource2->bottom,
			(long)pUnk0C[0], (long)pUnk0C[1],
			pBits, (BYTE*)m_pEmptyBits, 1);
	}

	{
		DWORD* pUnk04 = &m_dwDestX;

		pUnk04[0] = 0;
		pUnk04[1] = 0;
	}
}

// Stores pPoint in the destination position fields (+0x04 and +0x08)
// used by DrawFlags.
//
// hover.exe:0x411CF0
// FUNCTION: HOVER 0x411cf0
void CFlagStrip::SetPosition(const POINT* pPoint)
{
	m_dwDestX = pPoint->x;
	m_dwDestY = pPoint->y;
}

/////////////////////////////////////////////////////////////////////////////
// CFlatRunHeap

// FUNCTION: CFlatRunHeap::~CFlatRunHeap
// hover.exe:0x41B8C0
// FUNCTION: HOVER 0x41b8c0
CFlatRunHeap::~CFlatRunHeap()
{
	::operator delete(m_pHeapBase);
}

/////////////////////////////////////////////////////////////////////////////
// CSpanRasterizer

// FUNCTION: CSpanRasterizer::CSpanRasterizer
// hover.exe:0x4191B0
// FUNCTION: HOVER 0x4191b0
CSpanRasterizer::CSpanRasterizer(int nWidth, int nHeight, CMerlinWorld* pWorld)
{
	m_dwWidth = (DWORD)nWidth & ~3;
	m_dwHeight = (DWORD)nHeight;
	CreateTargetDIB(&g_palGame);
	m_dwNegWidth = (DWORD)-(long)m_dwWidth;
	m_dwRowDir = (DWORD)-1;
	m_bMipMapping = 1;
	m_pRunHeap = new CFlatRunHeap(this);
	m_pLinkPool = NULL;
	m_pColumnLists = NULL;
	m_pRenderObjPool = NULL;
	AllocBuffers(pWorld);
}

// FUNCTION: CSpanRasterizer::~CSpanRasterizer
// hover.exe:0x4193A0
// FUNCTION: HOVER 0x4193a0
CSpanRasterizer::~CSpanRasterizer()
{
	delete m_pRunHeap;
	::operator delete(m_pLinkPool);
	::operator delete(m_pColumnLists);
	::operator delete(m_pRenderObjPool);
}

// Creates an 8-bpp bottom-up DIB section sized to m_dwWidth/m_dwHeight,
// attaches it to m_bitmap and stores the pixel bits pointer in m_dwUnk04.
// The palette is copied into the BITMAPINFO color table in the same shifted
// RGBA order used by CVelocityIndicator::CreateTile.

// hover.exe:0x419440
// FUNCTION: HOVER 0x419440
void CSpanRasterizer::CreateTargetDIB(CPalette* pPalette)
{
	BITMAPINFO* pBmi;
	PALETTEENTRY apePalette[256];
	BYTE* pColorRed;
	PALETTEENTRY* pPe;
	HDC hScreenDC;
	HBITMAP hBitmap;

	hScreenDC = ::GetDC(NULL);

	pBmi = (BITMAPINFO*)::operator new(BMI8_SIZE);
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
		(void**)&m_pBits,
		NULL,
		0);
	m_bitmap.Attach(hBitmap);

	::ReleaseDC(NULL, hScreenDC);
	::operator delete(pBmi);
}

// Empty constructor for the render objects allocated by
// CSpanRasterizer::AllocBuffers. The original calls it once per object in a
// loop.

// hover.exe:0x419520
// FUNCTION: HOVER 0x419520
tagRENDEROBJ::tagRENDEROBJ()
{
}

// Allocate (or reallocate) the three span buffers owned by the render target.
// pWorld is the doc's CMerlinWorld; the count of 0xC0-byte
// entries is taken from the DWORD at offset 0x58 of that object.

// hover.exe:0x4192F0
// FUNCTION: HOVER 0x4192f0
void CSpanRasterizer::AllocBuffers(CMerlinWorld* pWorld)
{
	DWORD dwCount;
	DWORD dwSize;
	RENDEROBJ* pEntry;
	RENDEROBJ* pAllocated;

	::operator delete(m_pLinkPool);
	::operator delete(m_pColumnLists);
	::operator delete(m_pRenderObjPool);

	dwSize = (((DWORD)pWorld->m_arrNodes.GetSize() + 0x64) << 4) * 3;
	m_pLinkPool = (SPANLINK*)::operator new(dwSize);

	m_pColumnLists = (SPANLINK**)::operator new(m_dwWidth * sizeof(SPANLINK*));

	dwCount = (DWORD)pWorld->m_arrNodes.GetSize() + 0x64;
	dwSize = (dwCount << 6) * 3;
	pAllocated = (RENDEROBJ*)::operator new(dwSize);

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
				pEntry->tagRENDEROBJ::tagRENDEROBJ();
				pEntry++;
				lCount--;
			} while (lCount >= 0);
		}
		m_pRenderObjPool = pAllocated;
	}
	else
	{
		m_pRenderObjPool = NULL;
	}
}

// FUNCTION: CFlatRunHeap::CFlatRunHeap
// hover.exe:0x41B870
// FUNCTION: HOVER 0x41B870
CFlatRunHeap::CFlatRunHeap(CSpanRasterizer* pParent)
{
	m_pHeapBase = (BYTE*)operator new((pParent->m_dwHeight * 4 + 0x280) * 3 * 5);
	m_dwRows = pParent->m_dwHeight;
	// m_pScratchRuns is left uninitialised, as in hover.exe (ResetHeap sets it)
	m_pSkyNode = NULL;
	m_pFlatHead = NULL;
	m_pFlatTail = NULL;
	m_dwFlatNodes = 0;
	m_pHeapCursor = m_pHeapBase;
}

// Reset the arena: the merge scratch buffer sits at its start, the first
// FLATNODE after it, followed by the node's two word-pair buffers and its
// m_dwRows-word index array; the cursor ends past them. Every word-pair
// buffer is 0xA0 bytes (40 pairs).
// hover.exe:0x406C90
// FUNCTION: HOVER 0x406c90
void CFlatRunHeap::ResetHeap()
{
	BYTE* pBase = m_pHeapBase;
	FLATNODE* pNode;
	BYTE* pCursor;

	m_pHeapCursor = pBase;
	m_pScratchRuns = (WORD*)pBase;
	m_pFlatTail = NULL;

	pCursor = pBase + 0xA0 + sizeof(FLATNODE);
	pNode = (FLATNODE*)(pBase + 0xA0);

	m_pHeapCursor = (BYTE*)pNode;
	m_pFlatHead = NULL;
	m_dwFlatNodes = 0;
	m_pSkyNode = pNode;

	m_pHeapCursor = pCursor;
	pNode->m_pRunsA = (WORD*)pCursor;

	pCursor += 0xA0;
	m_pHeapCursor = pCursor;
	pNode->m_pRunsB = (WORD*)pCursor;
	pNode->m_pNext = NULL;

	pCursor += 0xA0;
	pNode->m_dwCurRuns = 0;
	m_pHeapCursor = pCursor;
	pNode->m_pRowStartCol = (WORD*)pCursor;

	pCursor += m_dwRows * 2;
	pNode->m_dwPrevRuns = 0;
	m_pHeapCursor = pCursor;
	pNode->m_ppPrevRuns = &pNode->m_pRunsA;
	pNode->m_ppCurRuns = &pNode->m_pRunsB;
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
// Original bug: biClrUsed is not clamped, so a header claiming more than
// 256 colors overruns aRemap (and pColors/pEntries) on the stack.
// hover.exe:0x410CE0
// FUNCTION: HOVER 0x410ce0
BOOL Hover_RemapDibToPalette(BYTE* pCopy, BYTE* pBits, CPalette* pPalette)
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

	pLogPal = (LOGPALETTE*)::operator new(LOGPAL256_SIZE);
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
		aRemap[i] = (BYTE)Hover_NearestPaletteIndex(dwColor, (BYTE*)pEntries);
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
void Hover_TransparentBlt8(int nSrcLeft, int nSrcTop, int nSrcRight,
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
// MFC's internal afxData (AFXIMPL.H AUX_DATA); only nWinVer (+0x74) and
// bWin32s (+0x78) are read here, so the type stays incomplete.
struct AUX_DATA;
extern AUX_DATA afxData;

// This is MFC 3.1's AfxFindResource (WINUTIL.CPP): Windows NT 3.1 could
// return a handle with the high bit set, which MFC treats as "not found".
// FUNCTION: HOVER 0x453352
HRSRC __stdcall Hover_FindResource(HINSTANCE hInst, LPCSTR lpName,
	LPCSTR lpType)
{
	HRSRC hResource = ::FindResourceA(hInst, lpName, lpType);
	if (((BOOL*)&afxData)[0x78 / 4] == 0 && ((UINT*)&afxData)[0x74 / 4] < 0x332 &&
		((DWORD)hResource & 0x80000000) != 0)
		hResource = NULL;
	return hResource;
}

/////////////////////////////////////////////////////////////////////////////
// CBumperView

// hover.exe:0x41E730
// FUNCTION: HOVER 0x41e730
CBumperView::CBumperView()
{
	// m_dwRequestedWidth/m_dwRequestedHeight are zeroed by the original
	// before the embedded object constructors run (member initializer list)
	m_dwRequestedWidth = 0;
	m_dwRequestedHeight = 0;
	m_dwKeySeqPos = 0;
	m_hEvtCommand = 0;
	m_pViewCtx = 0;
	m_pMirrorCtx = 0;
	m_pRasterizer = 0;
	m_pRenderThread = 0;
	m_dwUnk0044 = 0;
	m_bCursorShown = 1;
	m_dwDrawerMode = g_adwDrawerModeForDetail[theApp.m_dwSpeedDetail];
	m_dwUnk0518 = 0;
	m_dwUnk051C = 0;
	m_dwUnk0520 = 0;
	m_dwUnk0524 = 0;
	m_bSmokeBlind = 0;
	m_bNeedRepaint = 0;
	Hover_InitZones();
	m_bFullMap = 0;
	m_bDoubleSize = 0;
	m_bInitialized = 0;
	m_dwUnk0064 = 0;
	m_dwUnk0068 = 0;
	m_bInputLeft = 0;
	m_bInputRight = 0;
	m_bInputUp = 0;
	m_bInputDown = 0;
	m_dwFlagsLeft = 0;
	m_dwPointsPerFlag = 0;
	m_dwFlagBonus = 0;
	m_dwNextLevel = 0;
	m_dwLevelBonus = 0;
	m_dwMessageState = 2;
	m_dwTimerID = 0;
	m_dwWorkerCmd = 4;
}

// Only the renderer's frame-ready event is closed by hand; the render
// contexts, the offscreen target and the worker thread are released by
// DestroyRenderer when the window is destroyed. Everything else CBumperView
// owns (the CDC, the offscreen target, the four gauges, the two dials, the
// two pens, the four brushes and the CView base) is destroyed by the
// compiler generated member/base destructor calls that follow this body.

// hover.exe:0x41E3F0
// FUNCTION: HOVER 0x41e3f0
CBumperView::~CBumperView()
{
	if (m_hEvtCommand != 0)
		::CloseHandle(m_hEvtCommand);
}

// Creates the pens and brushes the dashboard, the message boxes and the
// map overlay draw with. Any MFC exception raised on the way is reported
// through Hover_FatalException (which aborts), so the function still returns
// TRUE on that path.

// hover.exe:0x41F1C0
// FUNCTION: HOVER 0x41f1c0
BOOL CBumperView::CreateGdiObjects()
{
	try
	{
		if (!m_penBlack.CreatePen(PS_SOLID, 1, RGB(0, 0, 0)))
			return FALSE;
		if (!m_penYellow.CreatePen(PS_SOLID, 1, RGB(255, 255, 0)))
			return FALSE;
		if (!m_brushYellow.CreateSolidBrush(RGB(255, 255, 0)))
			return FALSE;
		if (!m_brushBlack.CreateSolidBrush(RGB(0, 0, 0)))
			return FALSE;
		if (!m_brushGreen.CreateSolidBrush(RGB(0, 255, 114)))
			return FALSE;
		if (!m_brushRed.CreateSolidBrush(RGB(255, 0, 0)))
			return FALSE;
	}
	catch (CException* e)
	{
		Hover_FatalException(e);
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
	if (!CreateGdiObjects())
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
		((CMainFrame*)AfxGetApp()->m_pMainWnd)->RealizeGamePalette();
}

// Per-frame render step: update the camera state from the human player,
// run the scene projection, set up the renderer context, clear the render
// target, draw the projected objects, dispatch the frame-state handler and
// present (or invalidate) the result.

// hover.exe:0x402250
// FUNCTION: HOVER 0x402250
void CBumperView::RenderFrame()
{
	CBumperDoc* pDoc;
	DWORD* pdwCam;
	DWORD* pdwDest;
	CMapRenderer* pScene;
	DWORD adwCam[2];
	RECT rc;

	if (m_pRasterizer == NULL)
		return;

	pDoc = (CBumperDoc*)m_pDocument;

	EnterCriticalSection(&theApp.m_csScene);
	Hover_ReleaseDrawList();

	m_dwCamAngle = *(DWORD*)((BYTE*)&pDoc->m_humanPlayer + 0x88);
	pdwCam = pDoc->m_humanPlayer.GetPrevPos(adwCam);

	// The camera pair is written through a pointer local (rule 94).
	pdwDest = &m_dwCamX;
	pdwDest[0] = pdwCam[0];
	pdwDest[1] = pdwCam[1];

	m_dwCamZ = *(DWORD*)((BYTE*)&pDoc->m_humanPlayer + 0x8C) + 0x28;

	pScene = &m_map;
	pScene->ProjectScene(pDoc);

	// Gauge/dashboard scaling derived from player +0x1A4 and the gauge
	// object's m_dwSegments. The original forms a signed integer quotient
	// from the word difference, adds 0x288, multiplies by the gauge extent,
	// and clamps the high word of the product to [0, m_dwSegments].
	{
		BUMPFIXED fx;
		BUMPFIXED fxOffset;
		BUMPFIXED fxArg;
		long nMax;
		long nValue;

		fxOffset.dw = 0x288;

		fx.s.wFrac = 0;
		fx.s.sInt = (short)((WORD)(*(DWORD*)((BYTE*)&pDoc->m_humanPlayer
			+ 0x1A4)) - (WORD)pDoc->m_dwFloorHeight);
		fx.dw = (DWORD)((long)fx.dw / (long)(short)pDoc->m_dwHeightRange);

		nMax = (long)m_gaugeAltitude.m_dwSegments;

		fxArg.dw = fx.dw;
		fxArg.dw += fxOffset.dw;
		HOVER_FIXMULWP(fxArg.dw, nMax);

		nValue = fxArg.s.sInt;
		if (nMax < nValue)
			nValue = nMax;
		if (nMax > nValue)
		{
			m_gaugeAltitude.m_dwLevel = 0;
			if (nValue > 0)
				m_gaugeAltitude.m_dwLevel = (DWORD)nValue;
		}
		else
			m_gaugeAltitude.m_dwLevel = (DWORD)nMax;
	}

	LeaveCriticalSection(&theApp.m_csScene);
	EnterCriticalSection(&theApp.m_csOffscreen);

	if (m_bSmokeBlind != 0)
	{
		if (m_pMirrorCtx != NULL)
			m_pMirrorCtx->m_pShade = (DWORD*)pDoc->m_aShade[1].m_abTable;
		m_pViewCtx->m_pShade = (DWORD*)pDoc->m_aShade[1].m_abTable;
	}
	else
	{
		if (m_pMirrorCtx != NULL)
			m_pMirrorCtx->m_pShade = (DWORD*)pDoc->m_aShade[0].m_abTable;
		m_pViewCtx->m_pShade = (DWORD*)pDoc->m_aShade[0].m_abTable;
	}

	if (m_bFullMap == 0)
	{
		m_pViewCtx->SetCamera(m_dwCamX, m_dwCamY, m_dwCamZ,
			m_dwCamAngle);
		m_pRasterizer->RenderWorld(pDoc->m_pWorld, m_pViewCtx);

		if (theApp.m_bRearView != 0 && m_pMirrorCtx != NULL)
		{
			m_pMirrorCtx->SetCamera(m_dwCamX, m_dwCamY,
				m_dwCamZ, (m_dwCamAngle + 0x100) & 0x1FF);
			m_pRasterizer->RenderWorld(pDoc->m_pWorld, m_pMirrorCtx);
		}

		m_needle.UpdateNeedle(this, pDoc);
	}

	rc.left = m_map.m_rcView.left;
	rc.top = m_map.m_rcView.top;
	rc.right = m_map.m_rcView.right;
	rc.bottom = m_map.m_rcView.bottom;
	::PatBlt(m_dcOffscreen.m_hDC, rc.left, rc.top, rc.right - rc.left,
		rc.bottom - rc.top, BLACKNESS);
	// hover.exe:0x4024e3 flushes the GDI batch here: everything after this
	// writes to the DIB bits directly, so the fill has to have landed.
	::GdiFlush();

	m_map.DrawScene(this, pDoc);

	switch (m_dwMessageState)
	{
	case 1:
		DrawPausedMessage();
		break;
	case 2:
		{
			CString strA;
			CString strB;
			strA.LoadString(0xEF3F);
			strB.LoadString(0xEF53);
			DrawMessageBox(strA, strB, 0, 0);
		}
		break;
	case 3:
		DrawBonusMessage();
		break;
	}

	if (m_bNeedRepaint != 0 && m_bInitialized != 0)
	{
		::InvalidateRect(m_hWnd, NULL, TRUE);
		m_bNeedRepaint = 0;
	}
	else
		PresentFrame();

	LeaveCriticalSection(&theApp.m_csOffscreen);
	m_dwLastPresentTick = GetTickCount();
	m_dwPresentCount++;
}

/////////////////////////////////////////////////////////////////////////////
// CSpanRasterizer::RenderWorld and its helper functions (stubs for now).

// Visibility test for one sprite: side-classify both endpoints of its
// camera-facing segment against the left/right view-edge lines
// (g_lineFrustumLeft / g_lineFrustumRight).  The sprite is submitted when an
// endpoint sits past the left edge but inside the right one, or when the
// segment straddles the whole view wedge and the camera origin lies on the
// matching side of the segment.
// hover.exe:0x405B90
// FUNCTION: HOVER 0x405b90
int CSpanRasterizer::IsSegmentVisible(CMerlinLine* pSprite)
{
	DWORD dwY1 = pSprite->m_dwY1;
	int bMinA;
	int bMinB;
	int bMaxA;
	int bMaxB;
	int nSide;

	bMinA = ((int)g_lineFrustumLeft.m_dwY1 - (int)dwY1) *
			(int)g_lineFrustumLeft.m_dwDeltaX +
		((int)pSprite->m_dwX1 - (int)g_lineFrustumLeft.m_dwX1) *
			(int)g_lineFrustumLeft.m_dwDeltaY >= 0;
	bMinB = ((int)pSprite->m_dwX1 - (int)g_lineFrustumRight.m_dwX1) *
			(int)g_lineFrustumRight.m_dwDeltaY +
		((int)g_lineFrustumRight.m_dwY1 - (int)dwY1) *
			(int)g_lineFrustumRight.m_dwDeltaX >= 0;

	if (bMinA == 0 && bMinB != 0)
		return 1;

	bMaxA = ((int)g_lineFrustumLeft.m_dwY1 - (int)pSprite->m_dwY2) *
			(int)g_lineFrustumLeft.m_dwDeltaX +
		((int)pSprite->m_dwX2 - (int)g_lineFrustumLeft.m_dwX1) *
			(int)g_lineFrustumLeft.m_dwDeltaY >= 0;
	bMaxB = ((int)pSprite->m_dwX2 - (int)g_lineFrustumRight.m_dwX1) *
			(int)g_lineFrustumRight.m_dwDeltaY +
		((int)g_lineFrustumRight.m_dwY1 - (int)pSprite->m_dwY2) *
			(int)g_lineFrustumRight.m_dwDeltaX >= 0;

	if (bMaxA == 0 && bMaxB != 0)
		return 1;

	nSide = ((int)dwY1 - (int)g_lineFrustumLeft.m_dwY1) *
		(int)pSprite->m_dwDeltaX;
	nSide += ((int)g_lineFrustumLeft.m_dwX1 - (int)pSprite->m_dwX1) *
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
// Hover_FixAngle, swap them when the sweep would take the long arc,
// scale the wrapped angles to screen columns by m_dwColumnsPerAngle and
// clamp the resulting column window to the screen. One SPANLINK node is
// linked into each edge column's list (m_pColumnLists) from the
// g_pSpanLinkCursor buffer, then the sprite's span links are advanced past
// the consumed entries. Returns 1 when the nodes were submitted, 0 when the
// sprite is off-screen.

// hover.exe:0x405CE0
// FUNCTION: HOVER 0x405ce0
int CSpanRasterizer::SubmitSegment(CMerlinLine* pA, CMerlinLine* pB, int nC,
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
	SPANLINK** pLink;
	CRenderContext* pCam;

	pCam = g_pRenderCtx;
	lAng1 = (long)Hover_FixAngle(
		(long)(pB->m_dwY1 - pCam->m_dwCamY),
		(long)(pB->m_dwX1 - pCam->m_dwCamX));
	pCam = g_pRenderCtx;
	lAng2 = (long)Hover_FixAngle(
		(long)(pB->m_dwY2 - pCam->m_dwCamY),
		(long)(pB->m_dwX2 - pCam->m_dwCamX));

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
	lStart = (lAng1 - (long)m_dwLeftEdgeAngle) & 0x1FFFFFF;
	if (lStart > 0x1000000)
		lStart -= 0x2000000;
	lEnd = (lAng2 - (long)m_dwLeftEdgeAngle) & 0x1FFFFFF;
	if (lEnd > 0x1000000)
		lEnd -= 0x2000000;
	if (lEnd < lStart)
		lEnd += 0x2000000;

	// Scale the angles to screen columns (24.8 result) and derive the
	// inclusive column window with a three-column margin on each side.
	HOVER_FIXMULR(lScaledStart, lStart, m_dwColumnsPerAngle);
	HOVER_FIXMULR(lScaledEnd, lEnd, m_dwColumnsPerAngle);
	nStart = (int)(lScaledStart / 0x10000) - 3;
	nEnd = (int)(lScaledEnd / 0x10000) + 3;

	// Clamp the window to the screen; bail out when it falls off an edge
	// or collapses to a single column.
	if (nStart < 0)
	{
		nStart = 0;
		if (nEnd < 0)
			return 0;
	}
	if (nStart > (int)g_dwViewWidth - 1)
	{
		nStart = (int)g_dwViewWidth - 1;
		if (nEnd > (int)g_dwViewWidth - 1)
			return 0;
	}
	if (nEnd < 0)
		nEnd = 0;
	if (nEnd > (int)g_dwViewWidth - 1)
		nEnd = (int)g_dwViewWidth - 1;
	if (nEnd == nStart)
		return 0;

	// Emit the opening node (m_dwUnk04 = 1: textured drawer) at the start
	// column and the closing node (m_dwUnk04 = 0: flat drawer) at the end
	// column.
	pNode = g_pSpanLinkCursor;
	pNode->m_wOrder = wOrd;
	pNode->m_bSprite = nC;
	pNode->m_bOpen = 1;
	if (nC != 0)
	{
		pNode->m_pSegment = 0;
		pObj = (CMerlinDynamic*)pA;
	}
	else
		pNode->m_pSegment = pB;
	pNode->m_pObj = pA;
	pLink = &m_pColumnLists[nStart];
	pNode->m_pNext = *pLink;
	*pLink = pNode;

	pNode++;
	pNode->m_wOrder = wOrd;
	pNode->m_bOpen = 0;
	pNode->m_bSprite = nC;
	pNode->m_pObj = pA;
	if (nC != 0)
		pNode->m_pSegment = 0;
	else
		pNode->m_pSegment = pB;
	pLink = &m_pColumnLists[nEnd];
	pNode->m_pNext = *pLink;
	*pLink = pNode;
	pNode++;
	g_pSpanLinkCursor = pNode;

	// Advance the consumed animation frames to their next entries: the
	// sprite's single texture for nC != 0, the face's six texture slots
	// otherwise (CMerlinTexture::m_pNextFrame is the next frame).
	if (nC != 0)
	{
		if (pObj->m_pTexture != NULL && pObj->m_pTexture->m_pNextFrame != NULL)
			pObj->m_pTexture = pObj->m_pTexture->m_pNextFrame;
	}
	else
	{
		pFace = (CMerlinStatic*)pA;
		if (pFace->m_pFrontTopTex != NULL && pFace->m_pFrontTopTex->m_pNextFrame != NULL)
			pFace->m_pFrontTopTex = pFace->m_pFrontTopTex->m_pNextFrame;
		if (pFace->m_pBackTopTex != NULL && pFace->m_pBackTopTex->m_pNextFrame != NULL)
			pFace->m_pBackTopTex = pFace->m_pBackTopTex->m_pNextFrame;
		if (pFace->m_pFrontTex != NULL && pFace->m_pFrontTex->m_pNextFrame != NULL)
			pFace->m_pFrontTex = pFace->m_pFrontTex->m_pNextFrame;
		if (pFace->m_pBackTex != NULL && pFace->m_pBackTex->m_pNextFrame != NULL)
			pFace->m_pBackTex = pFace->m_pBackTex->m_pNextFrame;
		if (pFace->m_pFrontBottomTex != NULL && pFace->m_pFrontBottomTex->m_pNextFrame != NULL)
			pFace->m_pFrontBottomTex = pFace->m_pFrontBottomTex->m_pNextFrame;
		if (pFace->m_pBackBottomTex != NULL && pFace->m_pBackBottomTex->m_pNextFrame != NULL)
			pFace->m_pBackBottomTex = pFace->m_pBackBottomTex->m_pNextFrame;
	}
	return 1;
}

// Walk the maze BSP tree (m_pWorld->m_arrNodes, root index 0) back-to-front
// from the camera (g_lineFrustumLeft.m_dwX1 / g_lineFrustumRight.m_dwY1): test
// which side of the node's facing segment the camera sits on, recurse into
// the far child, submit the node sprite if it survives the view test, then
// continue down the near child (tail recursion as a loop). g_wBspOrder is
// passed to SubmitSegment as the span key and counts accepted submissions.
// hover.exe:0x405FA0
// FUNCTION: HOVER 0x405fa0
void CSpanRasterizer::WalkBSP(short nNode)
{
	CMerlinBSP* pSprite;
	CMerlinStatic* pPartner;
	int nVisible;
	WORD wOrd;

	if (nNode == -1)
		return;

	do
	{
		pSprite = m_pWorld->GetNode(nNode);
		if (((long)g_lineFrustumLeft.m_dwX1 - (long)pSprite->m_dwX1) *
				(long)pSprite->m_dwDeltaY +
			((long)pSprite->m_dwY1 - (long)g_lineFrustumRight.m_dwY1) *
				(long)pSprite->m_dwDeltaX < 0)
		{
			WalkBSP(pSprite->m_wFront);
			nVisible = IsSegmentVisible(pSprite);
			wOrd = g_wBspOrder;
			if (nVisible != 0)
			{
				pPartner = m_pWorld->GetStatic((short)pSprite->m_wFace);
				if (SubmitSegment(pPartner, pSprite, 0, wOrd) != 0)
				{
					g_wBspOrder++;
				}
			}
			nNode = pSprite->m_wBack;
		}
		else
		{
			WalkBSP(pSprite->m_wBack);
			nVisible = IsSegmentVisible(pSprite);
			wOrd = g_wBspOrder;
			if (nVisible != 0)
			{
				pPartner = m_pWorld->GetStatic((short)pSprite->m_wFace);
				if (SubmitSegment(pPartner, pSprite, 0, wOrd) != 0)
				{
					g_wBspOrder++;
				}
			}
			nNode = pSprite->m_wFront;
		}
	} while (nNode != -1);
}

// Allocate one render object from the span-buffer arena at g_pObjArenaCursor
// (bumped by sizeof(RENDEROBJ)) and link it into the g_pObjPool list.
// Wall segments (dwA != 0) are linked in behind the first wall object of the
// list (appended at the end when the list holds only floor/ceiling objects);
// floor/ceiling segments (dwA == 0) are inserted into the leading
// floor/ceiling run sorted by descending m_wBspOrder, ahead of any wall.
// hover.exe:0x4044A0
// FUNCTION: HOVER 0x4044a0
void Hover_InsertRenderObj(DWORD dwA, CMerlinLine* pSource, WORD wC,
	CMerlinLine* pSegment)
{
	RENDEROBJ* pNew;
	RENDEROBJ* pCur;
	RENDEROBJ* pNext;

	pNew = g_pObjArenaCursor;
	pCur = g_pObjPool;
	pNew->m_wBspOrder = wC;
	pNew->m_bSprite = dwA;
	pNew->m_pSegment = pSegment;
	pNew->m_bHidden = 1;
	pNew->m_pSource = pSource;
	pNext = NULL;

	if (dwA == 0)
	{
		if (pCur != NULL)
		{
			if ((short)pCur->m_wBspOrder >= (short)wC)
			{
				while (pCur->m_bSprite == 0)
				{
					pNext = pCur->m_pPoolNext;
					if (pNext == NULL)
					{
						pCur->m_pPoolNext = pNew;
						pNew->m_pPoolPrev = pCur;
						pNew->m_pPoolNext = NULL;
						goto L_done;
					}
					pCur = pNext;
					if ((short)pNext->m_wBspOrder < (short)wC)
						break;
				}
			}
			pNext = pCur->m_pPoolPrev;
			pCur->m_pPoolPrev = pNew;
			if (pNext != NULL)
			{
				pNext->m_pPoolNext = pNew;
				pNew->m_pPoolPrev = pNext;
				pNew->m_pPoolNext = pCur;
			}
			else
			{
				g_pObjPool = pNew;
				pNew->m_pPoolNext = pCur;
				pNew->m_pPoolPrev = pNext;
			}
			goto L_done;
		}
	}
	else
	{
		while (pCur != NULL && pCur->m_bSprite == 0 &&
			pCur->m_pPoolNext != NULL)
			pCur = pCur->m_pPoolNext;
		if (pCur != NULL)
		{
			pNext = pCur->m_pPoolNext;
			pCur->m_pPoolNext = pNew;
			if (pNext != NULL)
				pNext->m_pPoolPrev = pNew;
			pNew->m_pPoolNext = pNext;
			pNew->m_pPoolPrev = pCur;
			goto L_done;
		}
	}
	pNew->m_pPoolNext = pNext;
	pNew->m_pPoolPrev = pNext;
	g_pObjPool = pNew;

L_done:
	g_pObjArenaCursor = pNew + 1;
}

// Find the render object keyed by (dwA, dwB) and unlink it from the
// g_pObjPool list; returns 1 if it was not yet visible.
// hover.exe:0x404580
// FUNCTION: HOVER 0x404580
DWORD Hover_RemoveRenderObj(CMerlinLine* pSource, CMerlinLine* pSegment)
{
	RENDEROBJ* pScan;
	RENDEROBJ* pHead;
	RENDEROBJ* pPrev;
	RENDEROBJ* pNext;
	DWORD dwRet;

	dwRet = 0;
	pHead = g_pObjPool;
	pScan = pHead;
	while (pScan != NULL)
	{
		if (pScan->m_pSource == pSource && pScan->m_pSegment == pSegment)
			break;
		pScan = pScan->m_pPoolNext;
	}
	if (pScan != NULL)
	{
		if (pScan->m_bHidden == 0)
			dwRet = 1;
		pPrev = pScan->m_pPoolPrev;
		pNext = pScan->m_pPoolNext;
		if (pPrev != NULL)
			pPrev->m_pPoolNext = pNext;
		if (pNext != NULL)
			pNext->m_pPoolPrev = pPrev;
		if (pScan == pHead)
			g_pObjPool = pNext;
	}
	return dwRet;
}

// Turn the render objects collected for the current column bundle into
// spans against one phase camera line. The g_pObjPool list is walked
// twice: first over its leading floor/ceiling run (m_bSprite == 0), then
// over the wall objects behind it.
//
// Floor/ceiling pass: objects whose height range [m_wBottom, m_wTop] is
// already inside the covered range are marked done (m_bHidden/m_bCovered)
// and skipped; the rest are intersected with pLine. A crossing yields the
// camera distance (m_wDistance) and the texture u coordinate (m_wTexU,
// biased by the face's u origin at m_wFrontUOrg/m_wBackUOrg), and, for a face
// that is not flagged in m_bFrontTransparent/m_bBackTransparent, grows
// the covered height range [sLo, sHi] (seeded from dwSeed). The objects
// are appended to the g_pColumnObjs list in list order.
//
// Wall pass: the same intersection, but the face select can be flipped by
// the source's m_bMirror, and each object is inserted into the
// g_pColumnObjs list sorted by ascending m_wDistance (invisible objects go
// to the end of the run).
//
// The return value is always 1 (the caller ORs it into m_bRelink to force
// a span rebuild).
// hover.exe:0x4045E0
// FUNCTION: HOVER 0x4045e0
DWORD Hover_BuildDepthList(DWORD dwSeed, CMerlinLine* pLine)
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
	pObj = g_pObjPool;
	g_pColumnObjs = NULL;

	while (pObj != NULL && pObj->m_bSprite == 0)
	{
		pSrcA = (CMerlinStatic*)pObj->m_pSource;
		pObj->m_bCovered = 0;
		if ((short)pSrcA->m_wTop <= sHi &&
			(short)pSrcA->m_wBottom >= sLo)
		{
			pObj->m_bHidden = 1;
			pObj->m_bCovered = 1;
		}
		else
		{
			wRet = pSrcA->IntersectSegment(pLine, adwHit);
			pObj->m_bFrontFace = (wRet == 1);
			if (g_pColumnObjs == NULL)
				g_pColumnObjs = pObj;
			if (wRet != 0)
			{
				pObj->m_wDistance = Hover_Distance(pLine->m_dwX1,
					pLine->m_dwY1, adwHit[0], adwHit[1]);
				if (pObj->m_bFrontFace != 0)
				{
					wU = Hover_Distance(pSrcA->m_dwX2,
						pSrcA->m_dwY2, adwHit[0], adwHit[1]);
					wOrg = pSrcA->m_wFrontUOrg;
				}
				else
				{
					wU = Hover_Distance(pSrcA->m_dwX1,
						pSrcA->m_dwY1, adwHit[0], adwHit[1]);
					wOrg = pSrcA->m_wBackUOrg;
				}
				// The raw distance is stored first and immediately
				// overwritten by the biased one; the original does
				// the same (dead store).
				pObj->m_wTexU = wU;
				pObj->m_wTexU = wOrg + wU;

				if (pObj->m_bFrontFace != 0)
					bGrow = (pSrcA->m_bFrontTransparent == 0);
				else
					bGrow = (pSrcA->m_bBackTransparent == 0);
				if (bGrow != 0)
				{
					if (sHi >= (short)pSrcA->m_wBottom &&
						sHi < (short)pSrcA->m_wTop)
						sHi = (short)pSrcA->m_wTop;
					if (sLo <= (short)pSrcA->m_wTop &&
						sLo > (short)pSrcA->m_wBottom)
						sLo = (short)pSrcA->m_wBottom;
				}
				pObj->m_bHidden = 0;
			}
			else
			{
				pObj->m_bHidden = 1;
			}

			if (pPrev != NULL)
				pPrev->m_pColNext = pObj;
			pObj->m_pColNext = NULL;
			pObj->m_pColPrev = pPrev;
			pPrev = pObj;
		}
		pObj = pObj->m_pPoolNext;
	}

	while (pObj != NULL)
	{
		pSrcB = (CMerlinDynamic*)pObj->m_pSource;
		wRet = pSrcB->IntersectSegment(pLine, adwHit);
		pObj->m_bCovered = 0;
		bFace = (wRet == 1);
		pObj->m_bFrontFace = bFace;
		bSel = bFace;
		if (wRet != 0)
		{
			pObj->m_wDistance = Hover_Distance(pLine->m_dwX1,
				pLine->m_dwY1, adwHit[0], adwHit[1]);
			if (pSrcB->m_bMirror != 0)
				bSel = (bFace == 0);
			if (bSel != 0)
			{
				pObj->m_wTexU = Hover_Distance(pSrcB->m_dwX2,
					pSrcB->m_dwY2, adwHit[0], adwHit[1]);
			}
			else
			{
				pObj->m_wTexU = Hover_Distance(pSrcB->m_dwX1,
					pSrcB->m_dwY1, adwHit[0], adwHit[1]);
			}
			pdwVis = &pObj->m_bHidden;
			*pdwVis = 0;
		}
		else
		{
			pdwVis = &pObj->m_bHidden;
			*pdwVis = 1;
		}

		pScan = g_pColumnObjs;
		if (pScan == NULL)
		{
			pObj->m_pColNext = NULL;
			pObj->m_pColPrev = NULL;
			g_pColumnObjs = pObj;
		}
		else
		{
			for (;;)
			{
				if (pScan->m_bHidden == 0 &&
					(short)pScan->m_wDistance > (short)pObj->m_wDistance)
					break;
				if (*pdwVis != 0)
					break;
				if (pScan->m_pColNext == NULL)
				{
					pScan->m_pColNext = pObj;
					pObj->m_pColPrev = pScan;
					pObj->m_pColNext = NULL;
					goto L_next;
				}
				pScan = pScan->m_pColNext;
			}
			pBefore = pScan->m_pColPrev;
			pScan->m_pColPrev = pObj;
			if (pBefore != NULL)
				pBefore->m_pColNext = pObj;
			else
				g_pColumnObjs = pObj;
			pObj->m_pColNext = pScan;
			pObj->m_pColPrev = pBefore;
		}

L_next:
		pObj = pObj->m_pPoolNext;
	}
	return 1;
}

// Second and later passes of the phase-line walk (the first pass is
// Hover_BuildDepthList): re-intersect every render object still on the
// g_pColumnObjs list with the new phase line, refresh its two depth keys
// and move it back into depth order. m_wDistance is the distance from the
// camera to the crossing point, m_wTexU the distance from the crossing
// point to the wall segment's own endpoint (which endpoint depends on the
// side the line crosses from, inverted again for m_bMirror sources).
// Wall objects (m_bSprite != 0) are re-sorted by m_wDistance: an object
// that became visible is unlinked and re-inserted from the head, an
// already linked one is only walked backwards to its new place.
// Floor/ceiling objects instead bias m_wTexU by the source's per-face bias
// and then bubble any wall predecessor that now sorts behind them past
// themselves.
// Returns 1 when the list order changed (the caller rebuilds the span
// lists); an object that no longer meets the phase line at all restarts
// the whole phase through Hover_BuildDepthList.
// hover.exe:0x406D30
// FUNCTION: HOVER 0x406d30
DWORD Hover_UpdateDepthList(DWORD dwZ, CMerlinLine* pLine)
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
	pObj = g_pColumnObjs;
	while (pObj != NULL)
	{
		pNext = pObj->m_pColNext;
		pSrc = pObj->m_pSource;
		wRes = pSrc->IntersectSegment(pLine, adwPt);
		dwFace = (wRes == 1);
		pObj->m_bFrontFace = dwFace;
		dwFaceB = dwFace;

		if (wRes != 0)
		{
			pObj->m_wDistance = Hover_Distance(pLine->m_dwX1, pLine->m_dwY1,
				adwPt[0], adwPt[1]);

			// A sprite flagged at +0x68 keys off the opposite endpoint.
			if (pObj->m_bSprite != 0 &&
				((CMerlinDynamic*)pSrc)->m_bMirror != 0)
				dwFaceB = (dwFace == 0);

			if (dwFaceB != 0)
				pObj->m_wTexU = Hover_Distance(pSrc->m_dwX2,
					pSrc->m_dwY2, adwPt[0], adwPt[1]);
			else
				pObj->m_wTexU = Hover_Distance(pSrc->m_dwX1,
					pSrc->m_dwY1, adwPt[0], adwPt[1]);

			if (pObj->m_bSprite != 0)
			{
				if (pObj->m_bHidden != 0)
				{
					// Newly visible: unlink and re-insert from the head.
					dwRet = 1;
					pObj->m_bHidden = 0;
					pPrev = pObj->m_pColPrev;
					if (pPrev != NULL)
						pPrev->m_pColNext = pObj->m_pColNext;
					pLink = pObj->m_pColNext;
					if (pLink != NULL)
						pLink->m_pColPrev = pPrev;
					if (g_pColumnObjs == pObj)
						g_pColumnObjs = pLink;

					pScan = g_pColumnObjs;
					if (pScan == NULL)
					{
						pObj->m_pColNext = NULL;
						pObj->m_pColPrev = NULL;
						g_pColumnObjs = pObj;
					}
					else
					{
						while (pScan->m_bHidden != 0 ||
							(short)pScan->m_wDistance <=
								(short)pObj->m_wDistance)
						{
							pLink = pScan->m_pColNext;
							if (pLink == NULL)
							{
								pScan->m_pColNext = pObj;
								pObj->m_pColPrev = pScan;
								pObj->m_pColNext = NULL;
								goto L_next;
							}
							pScan = pLink;
						}
						pPrev = pScan->m_pColPrev;
						pScan->m_pColPrev = pObj;
						if (pPrev != NULL)
						{
							pPrev->m_pColNext = pObj;
							pObj->m_pColPrev = pPrev;
							pObj->m_pColNext = pScan;
						}
						else
						{
							g_pColumnObjs = pObj;
							pObj->m_pColNext = pScan;
							pObj->m_pColPrev = pPrev;
						}
					}
				}
				else
				{
					// Already linked: walk back to the new position.
					pPrev = pObj->m_pColPrev;
					if (pPrev == NULL)
						goto L_next;
					// Reloaded rather than copied from pPrev: the
					// original's walk keeps its own null test.
					pScan = pObj->m_pColPrev;
					while (pScan != NULL && (pScan->m_bHidden != 0 ||
						(short)pScan->m_wDistance >= (short)pObj->m_wDistance))
						pScan = pScan->m_pColPrev;
					if (pScan == pPrev)
						goto L_next;

					dwRet = 1;
					if (pPrev != NULL)
						pPrev->m_pColNext = pObj->m_pColNext;
					pLink = pObj->m_pColNext;
					if (pLink != NULL)
						pLink->m_pColPrev = pPrev;
					if (g_pColumnObjs == pObj)
						g_pColumnObjs = pLink;

					if (pScan != NULL)
					{
						pLink = pScan->m_pColNext;
						pScan->m_pColNext = pObj;
						if (pLink != NULL)
							pLink->m_pColPrev = pObj;
						pObj->m_pColNext = pLink;
						pObj->m_pColPrev = pScan;
					}
					else
					{
						pObj->m_pColPrev = NULL;
						pLink = g_pColumnObjs;
						pObj->m_pColNext = pLink;
						if (pLink != NULL)
							pLink->m_pColPrev = pObj;
						g_pColumnObjs = pObj;
					}
				}
			}
			else
			{
				if (pObj->m_bHidden != 0)
				{
					dwRet = 1;
					pObj->m_bHidden = 0;
				}

				if (pObj->m_bFrontFace != 0)
					pObj->m_wTexU += ((CMerlinStatic*)pSrc)->m_wFrontUOrg;
				else
					pObj->m_wTexU += ((CMerlinStatic*)pSrc)->m_wBackUOrg;

				// Pull every wall predecessor that now sorts behind this
				// floor/ceiling object past it, one at a time.
				do
				{
					dwMoved = 0;
					pPrev = pObj->m_pColPrev;
					if (pPrev != NULL && pPrev->m_bSprite != 0 &&
						pPrev->m_bHidden == 0 &&
						(short)pPrev->m_wDistance > (short)pObj->m_wDistance)
					{
						dwRet = 1;
						pLink = pPrev->m_pColPrev;
						if (pLink != NULL)
							pLink->m_pColNext = pPrev->m_pColNext;
						pScan = pPrev->m_pColNext;
						if (pScan != NULL)
							pScan->m_pColPrev = pLink;
						if (g_pColumnObjs == pPrev)
							g_pColumnObjs = pScan;

						pScan = pObj->m_pColNext;
						pObj->m_pColNext = pPrev;
						if (pScan != NULL)
							pScan->m_pColPrev = pPrev;
						pPrev->m_pColNext = pScan;
						pPrev->m_pColPrev = pObj;
						dwMoved = 1;
					}
				} while (dwMoved != 0);
			}
		}
		else if (pObj->m_bHidden == 0)
		{
			// The object dropped off the phase line: redo the whole phase.
			Hover_BuildDepthList(dwZ, pLine);
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
void CSpanRasterizer::RenderWorld(CMerlinWorld* pWorld, CRenderContext* pCtx)
{
	CMerlinLine line;	// phase line: camera -> current column-bundle edge
	CRenderContext* pCam;
	CMerlinDynamic* pSprite;
	SPANLINK* pNode;
	SINCOSENTRY* pCursor;
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

	m_pWorld = pWorld;
	g_pRenderCtx = pCtx;

	lWide = (long)pCtx->m_dwPixelH;
	lTall = (long)pCtx->m_dwPixelW;
	g_pbMipLut = pCtx->m_pMipShift;
	g_dwRowStep = lWide;
	g_dwColStep = lTall;
	g_dwHorizonY = pCtx->m_dwHorizon;
	g_pdwDistTable = pCtx->m_pRowDistance;
	g_dwViewWidth = pCtx->m_dwViewW;
	g_dwEyeZ = pCtx->m_dwCamZ;

	lWide >>= 1;
	nLogWide = 0;
	while (lWide != 0)
	{
		lWide >>= 1;
		nLogWide++;
	}

	g_dwColStep = lTall;

	lTall >>= 1;
	nLogTall = 0;
	while (lTall != 0)
	{
		lTall >>= 1;
		nLogTall++;
	}

	g_dwLogRowStep = nLogWide;
	g_dwLogColStep = nLogTall;

	g_bWallMipMapping = m_bMipMapping;
	g_dwRowPitch = m_dwNegWidth;
	g_dwDstRowStride = pCtx->m_dwRowStride;

	m_pRunHeap->ResetHeap();

	// The renderer's camera context is cached in a register across the
	// whole setup block and re-read after every call, exactly as the
	// original does (ebx at hover.exe:0x406161, reloaded at 0x40620D).
	pCam = g_pRenderCtx;

	m_dwColumnsPerAngle = ((int)g_dwViewWidth << 16) / 0x55;
	m_dwLeftEdgeAngle = (pCam->m_dwCamAngle << 16) - 0x2AAAAB;
	m_dwLeftEdgeAngle &= 0x1FFFFFF;
	memset(m_pColumnLists, 0, g_dwViewWidth * sizeof(SPANLINK*));

	line.m_dwX1 = pCam->m_dwCamX;
	line.m_dwY1 = pCam->m_dwCamY;
	pCursor = pCam->m_pRayDirs +
		((int)pCam->m_dwColumns *
			((int)pCam->m_dwCamAngle & 0x1FF) * 6) /
			512;

	g_lineFrustumLeft.m_dwX1 = pCam->m_dwCamX;
	g_lineFrustumLeft.m_dwY1 = pCam->m_dwCamY;
	g_lineFrustumLeft.m_dwX2 = pCam->m_dwCamX + pCursor->nCos;
	g_lineFrustumLeft.m_dwY2 = pCam->m_dwCamY + pCursor->nSin;
	g_lineFrustumLeft.Recalc();

	pCursor += g_dwViewWidth;
	pCam = g_pRenderCtx;
	g_lineFrustumRight.m_dwX1 = pCam->m_dwCamX;
	g_lineFrustumRight.m_dwY1 = pCam->m_dwCamY;
	g_lineFrustumRight.m_dwX2 = pCam->m_dwCamX + pCursor->nCos;
	g_lineFrustumRight.m_dwY2 = pCursor->nSin + pCam->m_dwCamY;
	g_lineFrustumRight.Recalc();
	pCursor -= g_dwViewWidth;

	g_pSpanLinkCursor = m_pLinkPool;

	// Rebuild every sprite's camera-facing segment (the perpendicular of
	// the view direction at the sprite's position, scaled by its size) and
	// submit the visible ones.
	for (i = m_pWorld->m_arrSprites.GetSize() - 1; i >= 0; i--)
	{
		pSprite = (CMerlinDynamic*)m_pWorld->m_arrSprites.GetAt(i);
		if (pSprite->m_bBillboard != 0)
		{
			int nDX;
			int nDY;
			int dx;
			int dy;
			int nScale;
			double fDist;

			dx = (short)pSprite->m_wX;
			dy = (short)pSprite->m_wY;
			pCam = g_pRenderCtx;
			nScale = (short)pSprite->m_wHalfWidth;
			dx -= (int)pCam->m_dwCamX;
			dy -= (int)pCam->m_dwCamY;
			fDist = sqrt(fabs((double)(dy * dy + dx * dx)));

			nDX = (int)(dx * nScale / fDist);
			nDY = (int)(dy * nScale / fDist);
			pCam = g_pRenderCtx;
			dx += (int)pCam->m_dwCamX;
			dy += (int)pCam->m_dwCamY;
			pSprite->m_dwX1 = dx + nDY;
			pSprite->m_dwY1 = dy - nDX;
			pSprite->m_dwX2 = dx - nDY;
			pSprite->m_dwY2 = nDX + dy;
			pSprite->Recalc();
		}
		if (IsSegmentVisible(pSprite) != 0)
			SubmitSegment(pSprite, pSprite, 1, 0);
	}

	g_wBspOrder = 0;
	WalkBSP(0);

	g_pObjArenaCursor = m_pRenderObjPool;
	g_pObjPool = NULL;
	g_pColumnObjs = NULL;
	bFirst = 1;
	m_bRelink = 0;
	dwMask = g_dwColStep - 1;

	for (g_dwCurColumn = 0; (int)g_dwCurColumn < (int)g_dwViewWidth;
		g_dwCurColumn++)
	{
		if ((g_dwCurColumn & dwMask) == 0)
		{
			line.m_dwX2 = g_pRenderCtx->m_dwCamX + pCursor->nCos;
			line.m_dwY2 = g_pRenderCtx->m_dwCamY + pCursor->nSin;
			pCursor += g_dwColStep;
			line.Recalc();
		}

		pNode = m_pColumnLists[g_dwCurColumn];
		if (pNode == NULL)
		{
			if ((g_dwCurColumn & dwMask) == 0)
			{
				if (bFirst != 0)
				{
					m_bRelink |= Hover_BuildDepthList(g_dwEyeZ, &line);
					bFirst = 0;
				}
				else
				{
					m_bRelink |= Hover_UpdateDepthList(g_dwEyeZ, &line);
				}
			}
		}
		else
		{
			while (pNode != NULL)
			{
				if (pNode->m_bOpen != 0)
					Hover_InsertRenderObj(pNode->m_bSprite, pNode->m_pObj,
						pNode->m_wOrder, pNode->m_pSegment);
				else
					m_bRelink |= Hover_RemoveRenderObj(pNode->m_pObj,
						pNode->m_pSegment);
				pNode = pNode->m_pNext;
			}
			if ((g_dwCurColumn & dwMask) == 0)
			{
				bFirst = 0;
				m_bRelink |= Hover_BuildDepthList(g_dwEyeZ, &line);
			}
			else
			{
				bFirst = 1;
			}
		}

		if ((g_dwCurColumn & dwMask) == 0)
		{
			dwNegStep = 0 - g_dwRowStep;
			dwTop = g_pRenderCtx->m_pClipTop[
				g_dwCurColumn] & dwNegStep;
			g_dwColTopY = dwTop;
			dwBottom = g_pRenderCtx->m_pClipBottom[
				g_dwCurColumn] & dwNegStep;
			g_dwAboveClipY = g_dwColTopY - g_dwRowStep - 1;
			g_dwBelowClipY = g_dwRowStep + dwBottom + 1;
			g_dwColProjScale = g_pRenderCtx->m_pColumnScale[
				g_dwCurColumn] << 10;
			if (g_pRenderCtx->m_bMirror != 0)
				g_dwColumnOrigin = (DWORD)g_pRenderCtx->m_pScanOrigin -
					g_dwColStep - g_dwCurColumn + g_dwViewWidth - 1;
			else
				g_dwColumnOrigin = (DWORD)g_pRenderCtx->m_pScanOrigin +
					g_dwCurColumn;
			g_dwColBottomY = dwBottom;

			if (g_pColumnObjs != NULL)
			{
				RenderColumn((DWORD)g_pColumnObjs);
			}
			else
			{
				g_dwColBottomY = dwBottom;
				m_bRelink |= 1;
				if ((int)(g_dwViewWidth - 1) > (int)g_dwCurColumn)
					m_pRunHeap->AddSkyRun(g_dwColTopY,
						dwBottom);
				m_pRunHeap->FlushRuns(this);
			}
		}
	}

	m_pRunHeap->FlushRuns(this);
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

inline HOVER_ZONESIZE Hover_ZoneSize(const SIZE* pSize)
{
	return *(const HOVER_ZONESIZE*)pSize;
}

// Present one rectangle of the offscreen render target into the client DC,
// doubling the destination geometry when the view runs at 2x. CDC::BitBlt
// and CDC::StretchBlt are inline in MFC 3.1 (AFXWIN1.INL), so each use
// expands to the GDI import plus an inlined CDC::GetSafeHdc on the source
// DC, and each expansion homes its computed arguments in stack temporaries
// of its own (notes/perfect-match.md rule 91).
#define HOVER_PRESENT_BLIT(dcDest, xDst, yDst, cxDst, cyDst)		\
	if (m_bDoubleSize != 0)											\
		(dcDest).StretchBlt((xDst) * 2, (yDst) * 2, (cxDst) * 2,	\
			(cyDst) * 2, &m_dcOffscreen, xDst, yDst, cxDst, cyDst,	\
			SRCCOPY);												\
	else															\
		(dcDest).BitBlt(xDst, yDst, cxDst, cyDst, &m_dcOffscreen,		\
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
void CBumperView::PresentFrame()
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

	// Determine the present-path flag; theApp.m_bDisplayDibActive forces it
	// to 1.
	dwFlag = g_bFullFramePresent;
	if (theApp.m_bDisplayDibActive != 0)
		dwFlag = 1;

	pDoc = (CBumperDoc*)m_pDocument;

	// Choose the zone descriptor. When the view is in 2x mode the base
	// descriptor (zone 0) is used; otherwise the current zone index selects
	// one of the six 0x338-byte blocks.
	pZone = g_aZones;
	if (m_bDoubleSize == 0)
		pZone += g_dwCurZone;

	CClientDC dc(this);

	// Select the global palette into the client DC.
	pOldPal = dc.SelectPalette(&g_palGame, TRUE);

	if (m_bFullMap == 0)
	{
		// Early-out: the view is inactive.
		if (m_bNeedRepaint != 0)
		{
			dc.SelectPalette(pOldPal, TRUE);
			return;
		}

		// Background rect-1 from m_rc3DView.
		if (dwFlag == 0)
		{
			HOVER_PRESENT_BLIT(dc, m_rc3DView.left, m_rc3DView.top,
				m_rc3DView.Width(), m_rc3DView.Height());

			// Background rect-2 from m_rcMirrorView when the alternate
			// renderer is active.
			if (theApp.m_bRearView != 0)
			{
				HOVER_PRESENT_BLIT(dc, m_rcMirrorView.left, m_rcMirrorView.top,
					m_rcMirrorView.Width(), m_rcMirrorView.Height());
			}
		}

		// m_needle present helper: draw into the offscreen DC when dwFlag
		// is set, otherwise draw into the client DC.
		m_needle.DrawNeedle((dwFlag != 0) ? &m_dcOffscreen : &dc, 0);

		// Dashboard CFlagStrip elements.
		if (m_flagsPlayer.m_bFlagsDirty != 0)
		{
			{
				HOVER_ZONESIZE size = Hover_ZoneSize(&pZone->m_size);

				m_flagsPlayer.DrawFlags(m_pRasterizer->m_pBits, size.cx, size.cy);
			}

			if (dwFlag == 0)
			{
				HOVER_PRESENT_BLIT(dc, pZone->m_ptPlayerFlags.x,
					pZone->m_ptPlayerFlags.y, 6 * pZone->m_sizeFlagIcon.cx,
					pZone->m_sizeFlagIcon.cy);
			}
		}

		if (m_flagsRobot.m_bFlagsDirty != 0)
		{
			{
				HOVER_ZONESIZE size = Hover_ZoneSize(&pZone->m_size);

				m_flagsRobot.DrawFlags(m_pRasterizer->m_pBits, size.cx, size.cy);
			}

			if (dwFlag == 0)
			{
				HOVER_PRESENT_BLIT(dc, pZone->m_ptRobotFlags.x,
					pZone->m_ptRobotFlags.y, 6 * pZone->m_sizeFlagIcon.cx,
					pZone->m_sizeFlagIcon.cy);
			}
		}

		// Dashboard CDigitDisplay elements. Each of the four waypoint markers
		// reads its origin and size into locals first: the original holds
		// all four in registers across the blit expansion instead of
		// re-reading the descriptor for the source rectangle.
		if (m_ctrJumpPods.m_bDigitsDirty != 0)
		{
			{
				HOVER_ZONESIZE size = Hover_ZoneSize(&pZone->m_size);

				m_ctrJumpPods.DrawDigits(m_pRasterizer->m_pBits, size.cx, size.cy);
			}

			if (dwFlag == 0)
			{
				DWORD x = pZone->m_aptPodCounters[0].x;
				DWORD y = pZone->m_aptPodCounters[0].y;
				DWORD cx = pZone->m_sizePodDigit.cx;
				DWORD cy = pZone->m_sizePodDigit.cy;

				HOVER_PRESENT_BLIT(dc, x, y, cx, cy);
			}
		}

		if (m_ctrSmokeBombs.m_bDigitsDirty != 0)
		{
			{
				HOVER_ZONESIZE size = Hover_ZoneSize(&pZone->m_size);

				m_ctrSmokeBombs.DrawDigits(m_pRasterizer->m_pBits, size.cx, size.cy);
			}

			if (dwFlag == 0)
			{
				DWORD x = pZone->m_aptPodCounters[2].x;
				DWORD y = pZone->m_aptPodCounters[2].y;
				DWORD cx = pZone->m_sizePodDigit.cx;
				DWORD cy = pZone->m_sizePodDigit.cy;

				HOVER_PRESENT_BLIT(dc, x, y, cx, cy);
			}
		}

		if (m_ctrTempWalls.m_bDigitsDirty != 0)
		{
			{
				HOVER_ZONESIZE size = Hover_ZoneSize(&pZone->m_size);

				m_ctrTempWalls.DrawDigits(m_pRasterizer->m_pBits, size.cx, size.cy);
			}

			if (dwFlag == 0)
			{
				DWORD x = pZone->m_aptPodCounters[1].x;
				DWORD y = pZone->m_aptPodCounters[1].y;
				DWORD cx = pZone->m_sizePodDigit.cx;
				DWORD cy = pZone->m_sizePodDigit.cy;

				HOVER_PRESENT_BLIT(dc, x, y, cx, cy);
			}
		}

		if (m_score.m_bDigitsDirty != 0)
		{
			{
				HOVER_ZONESIZE size = Hover_ZoneSize(&pZone->m_size);

				m_score.DrawDigits(m_pRasterizer->m_pBits, size.cx, size.cy);
			}

			if (dwFlag == 0)
			{
				HOVER_PRESENT_BLIT(dc, pZone->m_ptScore.x,
					pZone->m_ptScore.y, 7 * pZone->m_sizeScoreDigit.cx,
					pZone->m_sizeScoreDigit.cy);
			}
		}

		// Speed gauge. |velocity| is built in 16.16 from the document's two
		// velocity components, divided by the ship's scaled top speed and
		// fed to the first gauge object; CSegmentGauge::SetFraction (0x42A9F0)
		// and the CGaugeFraction(double) constructor (0x42A9D0) are expanded
		// inline here.
		//
		// theApp.m_pHumanParams points to the active ship params block,
		// whose [8] scales the document's top-speed reading.
		dwScale = *(DWORD*)((BYTE*)pDoc + 0x8364);
		HOVER_FIXMULP3(dwScale, theApp.m_pHumanParams->m_adw[8]);

		dwX = *(DWORD*)((BYTE*)pDoc + 0x8338);
		HOVER_FIXMULP3(dwX, dwX);
		dwY = *(DWORD*)((BYTE*)pDoc + 0x833C);
		HOVER_FIXMULP3(dwY, dwY);

		fx.dw = dwX;
		fx.dw = dwY + dwX;

		dwFix = (DWORD)(long)(sqrt((double)(fx.dw & 0xFFFF) * (1.0 / 65536.0)
			+ fx.s.sInt) * 65536.0);
		HOVER_FIXDIVLP(dwFix, dwScale);

		nMax = (long)m_gaugeSpeed.m_dwSegments;
		fxArg.dw = dwFix;
		fxArg.dw += 0x288;
		HOVER_FIXMULWP3(fxArg.dw, nMax);

		nValue = fxArg.s.sInt;
		if (nValue > nMax)
			nValue = nMax;
		if (nValue >= nMax)
			m_gaugeSpeed.m_dwLevel = (DWORD)nMax;
		else
		{
			m_gaugeSpeed.m_dwLevel = 0;
			if (nValue > 0)
				m_gaugeSpeed.m_dwLevel = (DWORD)nValue;
		}

		// Gauge calls. Each gauge is drawn into the offscreen DC (bUnk=1).
		// When dwFlag is clear and the gauge's value has changed it is also
		// drawn into the client DC (bUnk=0); the first gauge uses bUnk=1
		// even for the client draw in the original.
		CDC* pOffscreenDC = &m_dcOffscreen;

		if (dwFlag == 0)
			m_gaugeSpeed.DrawGauge(&dc, 1);
		m_gaugeSpeed.DrawGauge(pOffscreenDC, 1);

		if (*(DWORD*)((BYTE*)pDoc + 0x82B0) != 0)
		{
			if (dwFlag == 0 && m_gaugeInvincible.m_dwLevel != m_gaugeInvincible.m_dwLastLevel)
				m_gaugeInvincible.DrawGauge(&dc, 0);
			m_gaugeInvincible.DrawGauge(pOffscreenDC, 1);
		}

		if (*(DWORD*)((BYTE*)pDoc + 0x82B4) != 0)
		{
			if (dwFlag == 0 && m_gaugeSpeedPod.m_dwLevel != m_gaugeSpeedPod.m_dwLastLevel)
				m_gaugeSpeedPod.DrawGauge(&dc, 0);
			m_gaugeSpeedPod.DrawGauge(pOffscreenDC, 1);
		}

		// hover.exe:0x408217 falls into this block when +0x82B8 is set and
		// skips it when +0x82B8 is clear *and* +0x82B4 is set, i.e. the
		// second test is inverted with respect to the first.
		if (*(DWORD*)((BYTE*)pDoc + 0x82B8) != 0 ||
			*(DWORD*)((BYTE*)pDoc + 0x82B4) == 0)
		{
			if (dwFlag == 0 && m_gaugeSlow.m_dwLevel != m_gaugeSlow.m_dwLastLevel)
				m_gaugeSlow.DrawGauge(&dc, 0);
			m_gaugeSlow.DrawGauge(pOffscreenDC, 1);
		}

		if (dwFlag == 0 && m_gaugeAltitude.m_dwLevel != m_gaugeAltitude.m_dwLastLevel)
			m_gaugeAltitude.DrawGauge(&dc, 0);
		m_gaugeAltitude.DrawGauge(pOffscreenDC, 1);

		if (*(DWORD*)((BYTE*)pDoc + 0x82C0) != 0)
		{
			if (dwFlag == 0 && m_gaugeSmoke.m_dwLevel != m_gaugeSmoke.m_dwLastLevel)
				m_gaugeSmoke.DrawGauge(&dc, 0);
			m_gaugeSmoke.DrawGauge(pOffscreenDC, 1);
		}

		if (*(DWORD*)((BYTE*)pDoc + 0x82BC) != 0)
		{
			if (dwFlag == 0 && m_gaugeTempWall.m_dwLevel != m_gaugeTempWall.m_dwLastLevel)
				m_gaugeTempWall.DrawGauge(&dc, 0);
			m_gaugeTempWall.DrawGauge(pOffscreenDC, 1);
		}

		// Flush the batched GDI calls that filled the offscreen DC before
		// any of it is presented.
		::GdiFlush();

		// Blit the projected scene into the client area unless the
		// WM_COPYDATA path will handle it.
		if (dwFlag == 0)
			m_map.PresentMap(&dc);

		// Optional final overlay using the zone descriptor's base size.
		if (g_bFullFramePresent != 0)
		{
			HOVER_PRESENT_BLIT(dc, 0, 0, pZone->m_size.cx,
				pZone->m_size.cy);
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

			lpBits = (void*)m_pRasterizer->m_pBits;
			hwndTarget = theApp.m_hwndDisplayDib;
			cds.dwData = 0x401;
			cds.cbData = 0x12C00;
			if (lpBits == NULL)
				cds.cbData = 0;
			pfnSendMessage = ::SendMessageA;
			cds.lpData = lpBits;

			if (pfnSendMessage(hwndTarget, WM_COPYDATA, 0,
					(LPARAM)&cds) != 0)
			{
				if (theApp.m_hwndDisplayDib != 0)
				{
					if (theApp.m_bDisplayDibActive != 0)
					{
						pfnSendMessage(theApp.m_hwndDisplayDib, 0x404, 0,
							0);
						theApp.m_bDisplayDibActive = 0;
					}
					pfnSendMessage(theApp.m_hwndDisplayDib, 0x402, 0, 0);
					theApp.m_hwndDisplayDib = 0;
				}

				theApp.m_bUseDisplayDib = 0;
				theApp.m_bDisplayDibAvailable = 0;

				HOVER_PRESENT_BLIT(dc, 0, 0, pZone->m_size.cx,
					pZone->m_size.cy);
				return;
			}
		}
	}
	else
	{
		m_map.PresentMap(&dc);
	}

	dc.SelectPalette(pOldPal, TRUE);
}

/////////////////////////////////////////////////////////////////////////////
// Worker-thread entry point handed to AfxBeginThread by OnInitialUpdate:
// raises the GDI batch limit for this thread and runs the view's render
// state machine until it is told to exit.

// hover.exe:0x41ECC0
// FUNCTION: HOVER 0x41ecc0
UINT AFX_CDECL Hover_RenderThreadProc(LPVOID pParam)
{
	GdiSetBatchLimit(1);
	((CBumperView*)pParam)->WorkerThreadMain();
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

	m_bSmokeBlind = 0;
	m_bInitialized = 0;

	if (m_pRenderThread == 0)
	{
		GdiSetBatchLimit(1);

		m_hEvtCommand = CreateEventA(NULL, TRUE, FALSE, NULL);
		m_hEvtWorkerBusy = CreateEventA(NULL, TRUE, FALSE, NULL);
		m_hEvtWorkerIdle = CreateEventA(NULL, TRUE, FALSE, NULL);
		m_hEvtCommandAck = CreateEventA(NULL, TRUE, TRUE, NULL);
		m_hEvtUnused1 = CreateEventA(NULL, TRUE, FALSE, NULL);
		m_hEvtUnused2 = CreateEventA(NULL, TRUE, FALSE, NULL);

		// The worker is an MFC CWinThread kept alive by the view (the
		// teardown in DestroyRenderer waits on its m_hThread and deletes it).
		m_pRenderThread = AfxBeginThread(Hover_RenderThreadProc, this);
		(m_pRenderThread)->m_bAutoDelete = FALSE;

		m_dcOffscreen.Attach(::CreateCompatibleDC(NULL));
		m_hOldBitmap = 0;

		if (m_pViewCtx != NULL)
			delete m_pViewCtx;
		if (m_pMirrorCtx != NULL)
			delete m_pMirrorCtx;

		m_pViewCtx = new CRenderContext();

		if (theApp.m_bRearView != 0)
			m_pMirrorCtx = new CRenderContext();
	}

	// Initialize the CFlagStrip dashboard elements: enabled, and the
	// "changed" flag clamped like the gauges below (rule 22).
	m_flagsPlayer.m_bFlagsDirty = 1;
	m_flagsPlayer.m_dwFilled = m_flagsPlayer.m_dwSlots;
	if ((long)m_flagsPlayer.m_dwFilled > 0)
		m_flagsPlayer.m_dwFilled = 0;

	m_flagsRobot.m_bFlagsDirty = 1;
	m_flagsRobot.m_dwFilled = m_flagsRobot.m_dwSlots;
	if ((long)m_flagsRobot.m_dwFilled > 0)
		m_flagsRobot.m_dwFilled = 0;

	// Initialize the gauge current values to the clamped maximums.
	m_gaugeSpeed.m_dwLevel = m_gaugeSpeed.m_dwSegments;
	if ((long)m_gaugeSpeed.m_dwLevel > 0)
		m_gaugeSpeed.m_dwLevel = 0;
	m_gaugeInvincible.m_dwLevel = m_gaugeInvincible.m_dwSegments;
	if ((long)m_gaugeInvincible.m_dwLevel > 0)
		m_gaugeInvincible.m_dwLevel = 0;
	m_gaugeSlow.m_dwLevel = m_gaugeSlow.m_dwSegments;
	if ((long)m_gaugeSlow.m_dwLevel > 0)
		m_gaugeSlow.m_dwLevel = 0;
	m_gaugeSpeedPod.m_dwLevel = m_gaugeSpeedPod.m_dwSegments;
	if ((long)m_gaugeSpeedPod.m_dwLevel > 0)
		m_gaugeSpeedPod.m_dwLevel = 0;
	m_gaugeAltitude.m_dwLevel = m_gaugeAltitude.m_dwSegments;
	if ((long)m_gaugeAltitude.m_dwLevel > 0)
		m_gaugeAltitude.m_dwLevel = 0;
	m_gaugeSmoke.m_dwLevel = m_gaugeSmoke.m_dwSegments;
	if ((long)m_gaugeSmoke.m_dwLevel > 0)
		m_gaugeSmoke.m_dwLevel = 0;
	m_gaugeTempWall.m_dwLevel = m_gaugeTempWall.m_dwSegments;
	if ((long)m_gaugeTempWall.m_dwLevel > 0)
		m_gaugeTempWall.m_dwLevel = 0;

	pfnProgress = g_pfnUnk_4C4DAC;
	if (pfnProgress != NULL)
		pfnProgress(1, 0x0A);

	::GetClientRect(m_hWnd, &rcClient);
	RequestResize(rcClient.right - rcClient.left,
		rcClient.bottom - rcClient.top, 1);

	pfnProgress = g_pfnUnk_4C4DAC;
	if (pfnProgress != NULL)
		pfnProgress(4, 0x0A);

	pDoc = (CBumperDoc*)m_pDocument;
	m_bNeedRepaint = 1;

	if (pDoc->m_bLevelLoaded != 0)
	{
		EnterCriticalSection(&theApp.m_csScene);
		Hover_CommitDirtyObjects();
		LeaveCriticalSection(&theApp.m_csScene);

		pfnProgress = g_pfnUnk_4C4DAC;
		if (pfnProgress != NULL)
			pfnProgress(5, 0x0A);

		if (theApp.m_dwGameState == 3)
		{
			m_dwMessageState = 0;
			m_rcMessage.bottom = -1;
		}

		RequestRenderFrame(1);

		pfnProgress = g_pfnUnk_4C4DAC;
		if (pfnProgress != NULL)
			pfnProgress(8, 0x0A);

		pfnProgress = g_pfnUnk_4C4DAC;
		if (theApp.m_pBumperDoc->m_bAttractMode == 0)
		{
			theApp.m_dwGameState = 2;
			m_dwStateCountdown = 0x28;
			StartGameTimer();
			pfnProgress = g_pfnUnk_4C4DAC;
		}
		else
		{
			theApp.m_dwGameState = 0;
			m_dwStateCountdown = 0x3C;
		}
	}

	if (pfnProgress != NULL)
		pfnProgress(9, 0x0A);

	m_bInitialized = 1;

	CView::OnInitialUpdate();

	pfnProgress = g_pfnUnk_4C4DAC;
	if (pfnProgress != NULL)
		pfnProgress(0x0A, 0x0A);

	((CMainFrame*)AfxGetApp()->m_pMainWnd)->RealizeGamePalette();

	::RedrawWindow(m_hWnd, NULL, NULL, 0x105);
}

// Looks up the zone descriptor whose first two ints match (nWidth,nHeight),
// records the index in g_dwCurZone and then sets up the render target,
// the two renderer contexts and the background sampling arrays.

// hover.exe:0x4184B0
// FUNCTION: HOVER 0x4184b0
void CBumperView::SetupZone(int nWidth, int nHeight)
{
	CObject* pBackground;
	ZONE_DESCRIPTOR* pZone;
	int nIndex;
	ZONE_DESCRIPTOR* pSearch;
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

	nIndex = 0;
	pSearch = g_aZones;

	// Bottom-tested walk over the six descriptors: the loop falls straight
	// out into the "no such mode" return, the match jumps forward.
	do
	{
		if (pSearch->m_size.cx == nWidth && pSearch->m_size.cy == nHeight)
			goto Found;
		pSearch++;
		nIndex++;
	}
	while (pSearch <= &g_aZones[5]);

	return;

Found:
	g_dwCurZone = nIndex;

	pOldPal = m_dcOffscreen.SelectPalette(&g_palGame, FALSE);

	if (m_bDoubleSize != 0)
		pZone = g_aZones;
	else
		pZone = &g_aZones[g_dwCurZone];

	// In 2x stretch mode the render target is always 320x240, whatever mode
	// was asked for.
	if (m_bDoubleSize != 0)
	{
		nWidth = 320;
		nHeight = 240;
	}

	m_bFullMap = 0;

	pDoc = (CBumperDoc*)m_pDocument;

	// Put the previous frame's bitmap back before the render target is
	// destroyed, so the DIB section is not deleted while selected.
	if (m_hOldBitmap != 0)
		::SelectObject(m_dcOffscreen.GetSafeHdc(), (HGDIOBJ)m_hOldBitmap);

	delete m_pRasterizer;

	m_pRasterizer = new CSpanRasterizer(nWidth, nHeight, pDoc->m_pWorld);

	// The render target's size is copied as one 8-byte object (rule 56).
	*(SIZE*)&m_dwOffscreenWidth = *(SIZE*)&m_pRasterizer->m_dwWidth;

	m_rc3DView.left = 0;
	m_rc3DView.top = pZone->m_y3DTop;
	m_rc3DView.right = m_pRasterizer->m_dwWidth;
	m_rc3DView.bottom = pZone->m_y3DBottom;

	m_pViewCtx->SetMode(3, m_dwDrawerMode);
	m_pViewCtx->SetViewport(m_pRasterizer, &g_palGame, 0, 0,
		(int)m_pRasterizer->m_dwWidth, (int)m_pRasterizer->m_dwHeight);
	m_pViewCtx->BuildMipShiftTable();

	m_pViewCtx->m_pShade = (DWORD*)pDoc->m_aShade[0].m_abTable;

	// Select the new render target's DIB section into the view's offscreen
	// DC and remember the bitmap it displaces (CDC::SelectObject(CBitmap*)
	// inlines to SelectGdiObject(m_hDC, pBitmap->GetSafeHandle()); the
	// GetSafeHandle() on the result is the NULL test on the returned CBitmap*).
	// Everything that follows (PatBlt, BuildDashboard, the GetPixel edge scans)
	// reads and writes this DC.
	m_hOldBitmap = (HBITMAP)m_dcOffscreen.SelectObject(&m_pRasterizer->m_bitmap)->GetSafeHandle();

	// The alternate renderer's viewport is the zone's +0x210 rectangle,
	// copied field by field (a plain RECT assignment; CRect's operator=
	// would be a ::CopyRect call).
	*(RECT*)&m_rcMirrorView = pZone->m_rcMirror;

	m_rcMirrorView.left &= ~3;
	m_rcMirrorView.right &= ~3;

	nSub = 1;
	if ((int)m_dwDrawerMode > 3)
	{
		nSub = 2;
		if ((int)m_dwDrawerMode > 5)
			nSub = 4;
	}

	dwLeft = m_rcMirrorView.left;
	dwSpan = m_rcMirrorView.right - dwLeft;
	dwMidX = ((dwSpan * 3) / 4) - nSub;
	dwTopY = (m_rcMirrorView.top + (dwMidX - m_rcMirrorView.bottom)) / 2;
	dwBottomY = dwTopY - m_rcMirrorView.top + m_rcMirrorView.bottom;

	if (m_rcMirrorView.bottom > m_rc3DView.top)
		m_rcMirrorView.bottom = m_rc3DView.top;

	ptOrigin = *(POINT*)&m_rcMirrorView;
	ptOrigin.y -= dwTopY;

	if (theApp.m_bRearView == 0)
	{
		if (m_pMirrorCtx != NULL)
		{
			delete m_pMirrorCtx;
			m_pMirrorCtx = NULL;
		}
	}
	else if (theApp.m_bRearView == 1 && m_pMirrorCtx == NULL)
	{
		m_pMirrorCtx = new CRenderContext;
	}

	// i doubles as GetPixel's x below: dwSpan, or where the fill loops
	// stopped (0 for an empty span), as in hover.exe:0x418866/0x4188B5
	i = dwSpan;
	if (m_pMirrorCtx != NULL)
	{
		m_pMirrorCtx->m_bMirror = 1;
		m_pMirrorCtx->SetMode(3, m_dwDrawerMode);
		m_pMirrorCtx->SetViewport(m_pRasterizer, &g_palGame,
			ptOrigin.x, ptOrigin.y, dwSpan, dwMidX);
		m_pMirrorCtx->BuildMipShiftTable();
		m_pMirrorCtx->m_pShade = (DWORD*)pDoc->m_aShade[0].m_abTable;

		cbArray = dwSpan * sizeof(DWORD);
		adwTop = (DWORD*)::operator new(cbArray);
		adwBottom = (DWORD*)::operator new(cbArray);
		for (i = 0; i < dwSpan; i++)
			adwBottom[i] = dwTopY;
		for (i = 0; i < dwSpan; i++)
			adwTop[i] = dwBottomY;
		m_pMirrorCtx->SetClipSpans(adwBottom, adwTop);
		::operator delete(adwTop);
		::operator delete(adwBottom);
	}

	::PatBlt(m_dcOffscreen.m_hDC, 0, 0, m_pRasterizer->m_dwWidth, nHeight,
		BLACKNESS);

	BuildDashboard();

	dwColor = ::GetPixel(m_dcOffscreen.m_hDC, i, pZone->m_yTopDashEdge);

	dwCount = m_pRasterizer->m_dwWidth;
	cbArray = dwCount * sizeof(DWORD);
	adwTop = (DWORD*)::operator new(cbArray);
	adwBottom = (DWORD*)::operator new(cbArray);

	for (i = 0; (int)dwCount > i; i++)
	{
		adwBottom[i] = pZone->m_yTopDashEdge;
		while ((DWORD)::GetPixel(m_dcOffscreen.m_hDC, i, adwBottom[i]) == dwColor)
			adwBottom[i]--;

		adwTop[i] = pZone->m_yBottomDashEdge;
		while ((DWORD)::GetPixel(m_dcOffscreen.m_hDC, i, adwTop[i]) == dwColor)
			adwTop[i]++;
	}

	m_pViewCtx->SetClipSpans(adwBottom, adwTop);

	::operator delete(adwTop);
	::operator delete(adwBottom);

	m_dcOffscreen.SelectPalette(pOldPal, TRUE);

	pBackground = Hover_LookupTexture("BACKGRND");
	if (pBackground != NULL)
	{
		m_pViewCtx->SetSkyTexture(0, (CMerlinTexture*)pBackground);
		if (m_pMirrorCtx != NULL)
			m_pMirrorCtx->SetSkyTexture(0, (CMerlinTexture*)pBackground);
	}
}

// Worker-thread body for the render state machine. The thread loops, waiting
// for the main thread to signal a state change on m_hEvtCommand, then
// dispatches one state per iteration. State 3 sets the exit flag; the others
// loop back and wait again. The setup pass optionally loads DISPDIB.DLL and
// creates the DisplayDibWindow. Decompiled from hover.exe:0x41ECE0.

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
void CBumperView::WorkerThreadMain()
{
	DWORD dwState;
	POINT ptArgs;
	int nStateArg;
	int nScaleArg;
	BOOL bContinue;

	bContinue = TRUE;

	m_dwFpsBaseTick = GetTickCount();
	m_dwLastPresentTick = m_dwFpsBaseTick;
	m_dwPresentCount = 0;

	HoverWorkerApp()->m_bUseDisplayDib = 0;

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
			HoverWorkerApp()->m_hwndDisplayDib = 0;
		}
		else
		{
			HoverWorkerApp()->m_hwndDisplayDib = ::CreateWindowExA(0, "DisplayDibWindow", "",
				WS_CHILD, 0, 0,
				::GetSystemMetrics(SM_CXSCREEN), ::GetSystemMetrics(SM_CYSCREEN),
				hWnd, NULL,
				(hInstApp != NULL) ? hInstApp
					: (HINSTANCE)::GetWindowLongA(hWnd, GWL_HINSTANCE),
				NULL);
		}
	}

	if (theApp.m_hwndDisplayDib != 0)
		HoverWorkerApp()->m_bDisplayDibAvailable = 1;
	else
		HoverWorkerApp()->m_bDisplayDibAvailable = 0;

	while (bContinue)
	{
		EnterCriticalSection(&theApp.m_csWorkerCmd);
			SetEvent(m_hEvtWorkerIdle);
			ResetEvent(m_hEvtWorkerBusy);
			LeaveCriticalSection(&theApp.m_csWorkerCmd);

			WaitForSingleObject(m_hEvtCommand, INFINITE);

			EnterCriticalSection(&theApp.m_csWorkerCmd);
			dwState = m_dwWorkerCmd;
			switch (m_dwWorkerCmd)
			{
			case 0:
				ptArgs = *(POINT*)&m_dwRequestedWidth;
				break;
			case 1:
				nStateArg = theApp.m_dwSpeedDetail;
				break;
			case 5:
				nScaleArg = m_bZoomIn;
				break;
			}
			ResetEvent(m_hEvtWorkerIdle);
			SetEvent(m_hEvtWorkerBusy);
			ResetEvent(m_hEvtCommand);
			SetEvent(m_hEvtCommandAck);
			LeaveCriticalSection(&theApp.m_csWorkerCmd);

		try
		{
			switch (dwState)
			{
			case 0:
				SetupZone(ptArgs.x, ptArgs.y);
				break;
			case 1:
				ApplyDetailLevel(nStateArg);
				break;
			case 3:
				RenderFrame();
				break;
			case 2:
				bContinue = FALSE;
				break;
			case 4:
				BuildDashboard();
				break;
			case 5:
				ZoomMap(nScaleArg);
				break;
			case 6:
				BeginDispDib();
				break;
			case 7:
				EndDispDib();
				break;
			}
		}
		catch (CException* e)
		{
			Hover_FatalException(e);
		}
	}

	if (theApp.m_hwndDisplayDib != 0)
	{
		if (theApp.m_bDisplayDibActive != 0)
			::SendMessageA(theApp.m_hwndDisplayDib, 0x404, 0, 0);
		::SendMessageA(theApp.m_hwndDisplayDib, 0x402, 0, 0);
		HoverWorkerApp()->m_hwndDisplayDib = 0;
	}
	HoverWorkerApp()->m_bDisplayDibAvailable = 0;
	ResetEvent(m_hEvtWorkerBusy);
	SetEvent(m_hEvtWorkerIdle);
}

#pragma optimize("", on)

// Frame-step state 1 handler: load the level-start message strings and
// dispatch them to DrawMessageBox. When theApp.m_bAppActive is set the two-line
// message is drawn; otherwise a three-line message is drawn.
// hover.exe:0x40D340
// FUNCTION: HOVER 0x40d340
void CBumperView::DrawPausedMessage()
{
	CString strA;
	CString strB;
	CString strC;

	strA.LoadString(0xEF15);
	strB.LoadString(0xEF16);

	if (theApp.m_bAppActive != 0)
	{
		DrawMessageBox(strA, strB, NULL, NULL);
	}
	else
	{
		strC.LoadString(0xEF54);
		DrawMessageBox(strA, strC, strB, NULL);
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
void CBumperView::DrawBevelBox(RECT* pRect, int nWidth)
{
	CBrush brush(::GetSysColor(COLOR_BTNFACE));
	CPen pen(PS_SOLID, 1, ::GetSysColor(COLOR_BTNSHADOW));

	m_dcOffscreen.SaveDC();
	m_dcOffscreen.SelectObject(&brush);

	::PatBlt(m_dcOffscreen.m_hDC, pRect->left, pRect->top,
		pRect->right - pRect->left, pRect->bottom - pRect->top, PATCOPY);

	m_dcOffscreen.SelectObject(&pen);

	// Outer bottom/right edges.
	m_dcOffscreen.MoveTo(pRect->left, pRect->bottom);
	m_dcOffscreen.LineTo(pRect->right, pRect->bottom);
	m_dcOffscreen.LineTo(pRect->right, pRect->top);

	// Inner left/top edges.
	m_dcOffscreen.MoveTo(pRect->left + nWidth, pRect->bottom - nWidth);
	m_dcOffscreen.LineTo(pRect->left + nWidth, pRect->top + nWidth);
	m_dcOffscreen.LineTo(pRect->right - nWidth + 1, pRect->top + nWidth);

	m_dcOffscreen.SelectStockObject(WHITE_PEN);

	// Outer left/top edges.
	m_dcOffscreen.MoveTo(pRect->left, pRect->bottom);
	m_dcOffscreen.LineTo(pRect->left, pRect->top);
	m_dcOffscreen.LineTo(pRect->right + 1, pRect->top);

	// Inner bottom/right edges.
	m_dcOffscreen.MoveTo(pRect->left + nWidth, pRect->bottom - nWidth);
	m_dcOffscreen.LineTo(pRect->right - nWidth, pRect->bottom - nWidth);
	m_dcOffscreen.LineTo(pRect->right - nWidth, pRect->top + nWidth);

	m_dcOffscreen.SelectStockObject(WHITE_BRUSH);
	m_dcOffscreen.RestoreDC(-1);
}

// Frame-step state 2 handler: draw up to four centered message strings in a
// raised box. The first string is always present; the remaining three are
// optional (NULL skips them). The original uses a 600-weight Arial font whose
// height is taken from the current zone descriptor.
// hover.exe:0x40CCE0
// FUNCTION: HOVER 0x40cce0
void CBumperView::DrawMessageBox(const char* pszA, const char* pszB,
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
	if (m_bDoubleSize != 0)
		pZone = g_aZones;
	else
		pZone = &g_aZones[g_dwCurZone];

	// Create the message font and select it into the offscreen DC.
	CFont font;

	font.Attach(::CreateFont(pZone->m_cyMessageFont, 0, 0, 0, 0x258, 0, 0, 0,
		0, 2, 0, 0, 0x20, "Arial"));
	pOldFont = m_dcOffscreen.SelectObject(&font);

	// Measure every supplied string and keep the widest width.  CDC::
	// GetTextExtent is an inline that returns a CSize by value through the
	// attribute DC, so every call gets an 8-byte return temporary of its own
	// (rule 118) and reads m_hAttribDC, not GetSafeHdc().
	nCount = 1;

	CSize sizeA = m_dcOffscreen.GetTextExtent(pszA, strlen(pszA));

	cxMax = sizeA.cx;
	cyFont = sizeA.cy;
	cySpacing = (3 * cyFont) / 2;

	if (pszB != NULL)
	{
		CSize sizeB = m_dcOffscreen.GetTextExtent(pszB, strlen(pszB));

		cxMax = (sizeA.cx > sizeB.cx) ? sizeA.cx : sizeB.cx;
		nCount = 2;
	}
	if (pszC != NULL)
	{
		CSize sizeC = m_dcOffscreen.GetTextExtent(pszC, strlen(pszC));

		if (sizeC.cx > cxMax)
			cxMax = sizeC.cx;
		nCount++;
	}
	if (pszD != NULL)
	{
		CSize sizeD = m_dcOffscreen.GetTextExtent(pszD, strlen(pszD));

		if (sizeD.cx > cxMax)
			cxMax = sizeD.cx;
		nCount++;
	}

	// Compute the total height of the message block.
	cyTotal = ((3 * nCount - 1) * cyFont) / 2;

	// Get the system colors used for the shadow/highlight and text
	// (hover.exe fetches both here, before laying out the box).
	clrShadow = ::GetSysColor(COLOR_BTNSHADOW);
	clrText = ::GetSysColor(COLOR_BTNTEXT);

	// Center the block in the view and shift it up by half the absolute zone
	// composite offset.
	x = ((int)m_dwOffscreenWidth - cxMax) / 2;
	y = ((int)m_dwOffscreenHeight - cyTotal) / 2;
	y -= abs((pZone->m_rcBottomDashSrc.top - pZone->m_rcBottomDashSrc.bottom
		- pZone->m_rcTopDashSrc.top) + pZone->m_rcTopDashSrc.bottom) / 2;

	rc.left = x;
	rc.top = y;
	rc.right = x + cxMax;
	rc.bottom = y + cyTotal;

	// Inflate the box by one third of the line spacing (cyFont/2).
	cyHalfSpacing = cySpacing / 3;
	::InflateRect(&rc, cyHalfSpacing, cyHalfSpacing);

	// Save the inflated rectangle, doubling it in stretch mode for
	// DrawBevelBox.
	m_rcMessage = rc;
	if (m_bDoubleSize != 0)
	{
		m_rcMessage.left <<= 1;
		m_rcMessage.top <<= 1;
		m_rcMessage.right <<= 1;
		m_rcMessage.bottom <<= 1;
	}

	// Draw the raised box (from the un-doubled local rectangle; the doubled
	// copy in m_rcMessage is for the presenter) and set transparent
	// background for the text.
	DrawBevelBox(&rc, 3);
	m_dcOffscreen.SetBkMode(TRANSPARENT);

	// Draw each supplied string: the first line sits one margin (cySpacing/3)
	// below the box top, every further line one full line pitch (3*cyFont/2)
	// below the previous one (hover.exe:0x40CFC2 adds [ebp-0x18], the three
	// later sites add [ebp-0x14]). pszA is drawn unconditionally: the
	// original never null-checks it (only pszB/pszC/pszD get a guard).
	rc.top += cyHalfSpacing;
	DrawMessageLine(&rc, pszA, clrShadow, clrText);
	if (pszB != NULL)
	{
		rc.top += cySpacing;
		DrawMessageLine(&rc, pszB, clrShadow, clrText);
	}
	if (pszC != NULL)
	{
		rc.top += cySpacing;
		DrawMessageLine(&rc, pszC, clrShadow, clrText);
	}
	if (pszD != NULL)
	{
		rc.top += cySpacing;
		DrawMessageLine(&rc, pszD, clrShadow, clrText);
	}

	// Restore the previous font.
	m_dcOffscreen.SelectObject(pOldFont);
}

// Helper for DrawMessageBox: draw one message line. In non-stretch mode the
// text is drawn with a dark shadow at (+1,+1) and a white highlight at
// (-1,-1) to give a raised appearance; in stretch mode it is drawn once with
// the text color.
// hover.exe:0x40D0A0
// FUNCTION: HOVER 0x40d0a0
void CBumperView::DrawMessageLine(RECT* pRect, const char* psz, COLORREF clrShadow,
	COLORREF clrText)
{
	if (m_bDoubleSize == 0)
	{
		::OffsetRect(pRect, 1, 1);
		m_dcOffscreen.SetTextColor(clrShadow);
		m_dcOffscreen.DrawText(psz, -1, pRect, DT_CENTER);

		::OffsetRect(pRect, -2, -2);
		m_dcOffscreen.SetTextColor(RGB(255, 255, 255));
		m_dcOffscreen.DrawText(psz, -1, pRect, DT_CENTER);

		::OffsetRect(pRect, 1, 1);
	}

	m_dcOffscreen.SetTextColor(clrText);
	m_dcOffscreen.DrawText(psz, -1, pRect, DT_CENTER);
}

// Frame-step state 3 handler: display the end-of-level bonus score.
// Loads the "Bonus Score" title, formats the remaining-flags score line,
// the next-level message and an optional level-bonus line, then dispatches
// the strings to DrawMessageBox for centered drawing.
// hover.exe:0x4283F0
// FUNCTION: HOVER 0x4283f0
void CBumperView::DrawBonusMessage()
{
	CString strA;
	CString strB;
	CString strC;
	CString strD;

	strD.LoadString(0xEF52);

	CString strFormat;

	if (m_dwFlagsLeft == 1)
		strFormat.LoadString(0xEF3D);
	else
		strFormat.LoadString(0xEF62);

	strA.Format(strFormat, m_dwFlagsLeft, m_dwPointsPerFlag, m_dwFlagBonus);

	strFormat.LoadString(0xEF3E);
	strB.Format(strFormat, m_dwNextLevel);

	if (m_dwLevelBonus != 0)
	{
		strFormat.LoadString(0xEF50);
		strC.Format(strFormat, m_dwLevelBonus);
		DrawMessageBox(strD, strA, strB, strC);
	}
	else
	{
		DrawMessageBox(strD, strA, strB, NULL);
	}
}

// Helper stubs for the renderer setup / worker-thread cluster.

// Set the offscreen bitmap size from the client rect and synchronize with
// the renderer worker thread.  nUnk selects the full handshake path when
// nonzero.

// hover.exe:0x4274B0
// FUNCTION: HOVER 0x4274b0
void CBumperView::RequestResize(int nWidth, int nHeight, int nUnk)
{
	WaitForSingleObject(m_hEvtCommand, 0);
	WaitForSingleObject(m_hEvtWorkerBusy, 0);
	WaitForSingleObject(m_hEvtWorkerIdle, 0);
	WaitForSingleObject(m_hEvtCommandAck, 0);
	WaitForSingleObject(m_hEvtUnused1, 0);
	WaitForSingleObject(m_hEvtUnused2, 0);

	if (WaitForSingleObject(m_hEvtCommandAck, 0) == WAIT_TIMEOUT &&
		m_dwWorkerCmd != 0)
	{
		Hover_WaitPumpCopyData(m_hEvtCommandAck);
	}

	EnterCriticalSection(&theApp.m_csWorkerCmd);
	m_dwWorkerCmd = 0;
	DWORD* pdwSize = &m_dwRequestedWidth;
	pdwSize[0] = nWidth;
	pdwSize[1] = nHeight;
	LeaveCriticalSection(&theApp.m_csWorkerCmd);

	if (nUnk != 0)
	{
		Hover_WaitPumpCopyData(m_hEvtWorkerIdle);
		EnterCriticalSection(&theApp.m_csWorkerCmd);
		ResetEvent(m_hEvtCommandAck);
		SetEvent(m_hEvtCommand);
		LeaveCriticalSection(&theApp.m_csWorkerCmd);
		Hover_WaitPumpCopyData(m_hEvtCommandAck);
		Hover_WaitPumpCopyData(m_hEvtWorkerIdle);
	}
	else
	{
		EnterCriticalSection(&theApp.m_csWorkerCmd);
		ResetEvent(m_hEvtCommandAck);
		SetEvent(m_hEvtCommand);
		LeaveCriticalSection(&theApp.m_csWorkerCmd);
	}

	m_dwUnk0088 = 1;
}

// Background/dashboard setup helper.  Loads the zone's dashboard bitmap from
// dashres.dll, configures the gauge/indicator helper objects from the zone
// descriptor, clears the client area and composites the sprite layers.

// hover.exe:0x410510
// FUNCTION: HOVER 0x410510
void CBumperView::BuildDashboard()
{
	ZONE_DESCRIPTOR* pZone;
	SIZE* psizeDash;
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
	pOldPal = m_dcOffscreen.SelectPalette(&g_palGame, FALSE);

	// Select the zone descriptor: zone 0 is forced in 2x stretch mode.  The
	// base is assigned unconditionally and only the index is guarded, the
	// way DrawKeyLabels does it - the original loads `mov ebp,g_aZones`
	// at hover.exe:0x410526, before the `cmp [ebx+0x544],0` that decides
	// whether to scale the index in at all.
	pZone = g_aZones;
	if (m_bDoubleSize == 0)
		pZone += g_dwCurZone;

	hInst = ::LoadLibraryA("dashres.dll");

	// The resource id is the zone index plus 0x81; in 2x stretch mode the
	// index is 0, so the id is 0x81, not 0 (the original adds the 0x81
	// after the two arms join, at hover.exe:0x410578).
	if (m_bDoubleSize != 0)
		dwResId = 0;
	else
		dwResId = g_dwCurZone;

	hRsrc = Hover_FindResource(hInst, (LPCSTR)(dwResId + 0x81), RT_BITMAP);
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
	// for the two LoadIcons sprites, for the four LoadDigits digit banks
	// and for both Hover_TransparentBlt8 composites.
	pDibBits = pBits + ((BITMAPINFOHEADER*)pBits)->biSize + 0x400;
	Hover_RemapDibToPalette(pBits, pDibBits, &g_palGame);

	// The two sprite banks, the four digit banks and both composite passes
	// all read the descriptor's width/height pair through the same pointer
	// local: that is what keeps the original's `lea edi,[ebp+8]` (hover.exe
	// :0x410721) live as `mov ecx,[edi] / mov edx,[edi+4]` across ten call
	// sites, all the way down to the spill at 0x4109D2 (rule 94).  It is
	// set here rather than at its first use so that it and pDibBits hold
	// the last two callee-saved registers over the seven SetStyle calls:
	// with one free VC hoists `&m_penBlack` into it as a common
	// subexpression, and the original recomputes `lea ecx,[ebx+0x4d0]` at
	// every one of the seven sites.
	psizeDash = &pZone->m_sizeDash;

	m_gaugeSpeed.SetStyle(pZone->m_anGaugeSegments[0], &m_brushYellow,
		&m_penBlack, &m_brushBlack);
	m_gaugeSlow.SetStyle(pZone->m_anGaugeSegments[2], &m_brushRed,
		&m_penBlack, &m_brushBlack);
	m_gaugeSpeedPod.SetStyle(pZone->m_anGaugeSegments[2], &m_brushGreen,
		&m_penBlack, &m_brushBlack);
	m_gaugeInvincible.SetStyle(pZone->m_anGaugeSegments[1], &m_brushYellow,
		&m_penBlack, &m_brushBlack);
	m_gaugeAltitude.SetStyle(pZone->m_anGaugeSegments[3], &m_brushYellow,
		&m_penBlack, &m_brushBlack);
	m_gaugeSmoke.SetStyle(pZone->m_anGaugeSegments[5], &m_brushYellow,
		&m_penBlack, &m_brushBlack);
	m_gaugeTempWall.SetStyle(pZone->m_anGaugeSegments[4], &m_brushYellow,
		&m_penBlack, &m_brushBlack);

	// The original reads *both* sprite rectangles through a base of their
	// own (`lea edx,[ebp+0x1e0]` then `lea ecx,[ebp+0x1b0]`, hover.exe:
	// 0x410728/0x41073E), but it creates them lazily inside the argument
	// evaluation, after the level-index division has finished with eax/edx.
	// A declared local is initialized at the head of its block instead, so
	// only one of the two fits: with both declared VC spills the second
	// across the `idiv` and reloads it for each of its four fields.

	{
		RECT* prc1 = &pZone->m_arcFlagIcons[0];

		m_flagsRobot.LoadIcons(
			LVLPARAM((int)g_dwLevel).m_dwUnk30,
			prc1->left, prc1->top, prc1->right, prc1->bottom,
			pZone->m_arcFlagIcons[3].left,
			pZone->m_arcFlagIcons[3].top,
			pZone->m_arcFlagIcons[3].right,
			pZone->m_arcFlagIcons[3].bottom,
			pDibBits,
			psizeDash->cx, pZone->m_sizeDash.cy);
		// The height of the *first* sprite bank is the one field the
		// original still reads off the descriptor itself (`mov edx,
		// [ebp+0xc]`, hover.exe:0x41071E): the width/height base is born
		// one reference later, at `lea edi,[ebp+8]` (0x410721).  Spelling
		// it that way is one reference fewer through the pointer and it
		// keeps the pointer ahead of the pixel base in VC's ranking.
	}

	{
		RECT* prc1 = &pZone->m_arcFlagIcons[1];

		m_flagsPlayer.LoadIcons(
			LVLPARAM((int)g_dwLevel).m_dwUnk30,
			prc1->left, prc1->top, prc1->right, prc1->bottom,
			pZone->m_arcFlagIcons[2].left,
			pZone->m_arcFlagIcons[2].top,
			pZone->m_arcFlagIcons[2].right,
			pZone->m_arcFlagIcons[2].bottom,
			pDibBits,
			psizeDash->cx, psizeDash->cy);
	}

	m_score.LoadDigits(7, &pZone->m_arcScoreDigits[0], pDibBits,
		psizeDash->cx, psizeDash->cy);
	m_ctrJumpPods.LoadDigits(1, &pZone->m_arcPodDigits[0], pDibBits,
		psizeDash->cx, psizeDash->cy);
	m_ctrSmokeBombs.LoadDigits(1, &pZone->m_arcPodDigits[0], pDibBits,
		psizeDash->cx, psizeDash->cy);
	m_ctrTempWalls.LoadDigits(1, &pZone->m_arcPodDigits[0], pDibBits,
		psizeDash->cx, psizeDash->cy);

	m_ctrJumpPods.SetPosition(&pZone->m_aptPodCounters[0]);
	m_ctrSmokeBombs.SetPosition(&pZone->m_aptPodCounters[2]);
	m_ctrTempWalls.SetPosition(&pZone->m_aptPodCounters[1]);
	m_score.SetPosition(&pZone->m_ptScore);

	m_flagsPlayer.SetPosition(&pZone->m_ptPlayerFlags);
	m_flagsRobot.SetPosition(&pZone->m_ptRobotFlags);

	m_gaugeSpeed.SetLayout(&pZone->m_arcGauges[0]);
	m_gaugeSlow.SetLayout(&pZone->m_arcGauges[2]);
	m_gaugeSpeedPod.SetLayout(&pZone->m_arcGauges[2]);
	m_gaugeInvincible.SetLayout(&pZone->m_arcGauges[1]);
	m_gaugeAltitude.SetLayout(&pZone->m_arcPodGauges[0]);
	m_gaugeSmoke.SetLayout(&pZone->m_arcPodGauges[2]);
	m_gaugeTempWall.SetLayout(&pZone->m_arcPodGauges[1]);

	m_map.SetViewport(&m_dcOffscreen, this, theApp.m_pBumperDoc,
		pZone->m_rcMap);
	m_needle.CreateTile(&m_dcOffscreen, pBits, pZone->m_rcNeedle);

	m_needle.m_pPen = &m_penYellow;
	::GetClientRect(m_hWnd, &rcTemp);

	// The two extents are statements of their own: the original subtracts
	// both (`sub eax,[esp+0x44] / sub ecx,[esp+0x40]`, hover.exe:0x4109A9
	// and 0x4109AD) *before* it pushes the BLACKNESS constant, which is
	// what a right-to-left argument evaluation of the subtractions would
	// never do.  Height is the first statement: the original computes
	// bottom-top into eax and only then right-left into ecx.
	nHeight = rcTemp.bottom - rcTemp.top;
	nWidth = rcTemp.right - rcTemp.left;
	::PatBlt(m_dcOffscreen.m_hDC, rcTemp.left, rcTemp.top, nWidth, nHeight,
		BLACKNESS);
	::GdiFlush();

	// The descriptor's size pair is copied out of the pointer local into
	// two scalars here (hover.exe:0x4109D2 `mov ecx,[edi] / mov eax,[edi+4]`
	// into the frame slots at +0x18/+0x1C): both composite passes read the
	// scalars, and edi is free again for the rectangle copies below.  These
	// two slots are what makes the frame 0x40 rather than 0x38 bytes.
	HOVER_ZONESIZE zone = Hover_ZoneSize(psizeDash);

	// First composite pass (zone descriptor rects at +0x1f0/+0x200).
	rcA = pZone->m_rcTopDashSrc;
	rcB = pZone->m_rcTopDashDst;

	Hover_TransparentBlt8(
		rcA.left, rcA.top, rcA.right, rcA.bottom,
		zone.cx, zone.cy,
		rcB.left, rcB.top, rcB.right, rcB.bottom,
		m_pRasterizer->m_dwWidth, m_pRasterizer->m_dwHeight,
		pDibBits, m_pRasterizer->m_pBits, -1);

	// Second composite pass (zone descriptor rects at +0x238/+0x248).
	rcA = pZone->m_rcBottomDashSrc;
	rcB = pZone->m_rcBottomDashDst;

	Hover_TransparentBlt8(
		rcA.left, rcA.top, rcA.right, rcA.bottom,
		zone.cx, zone.cy,
		rcB.left, rcB.top, rcB.right, rcB.bottom,
		m_pRasterizer->m_dwWidth, m_pRasterizer->m_dwHeight,
		pDibBits, m_pRasterizer->m_pBits, -1);

	m_gaugeSpeed.DrawGauge(&m_dcOffscreen, 1);
	m_gaugeSpeedPod.DrawGauge(&m_dcOffscreen, 1);
	m_gaugeSlow.DrawGauge(&m_dcOffscreen, 1);
	m_gaugeInvincible.DrawGauge(&m_dcOffscreen, 1);
	m_gaugeAltitude.DrawGauge(&m_dcOffscreen, 1);
	m_gaugeSmoke.DrawGauge(&m_dcOffscreen, 1);
	m_gaugeTempWall.DrawGauge(&m_dcOffscreen, 1);
	m_needle.DrawNeedle(&m_dcOffscreen, 1);
	::GdiFlush();

	// Each of the six element uploads reads the render target's size pair
	// through the by-value accessor (rule 200): the six sites share one
	// 8-byte temporary of which only the `.cx` store survives
	// (hover.exe:0x410BC1 `mov [esp+0x34],ecx`, 0x410BCB `mov eax,[esp+0x34]`).
	{
		HOVER_ZONESIZE size;

		size = Hover_ZoneSize(&m_pRasterizer->m_dwWidth);
		m_flagsPlayer.DrawFlags(m_pRasterizer->m_pBits, size.cx, size.cy);
		size = Hover_ZoneSize(&m_pRasterizer->m_dwWidth);
		m_flagsRobot.DrawFlags(m_pRasterizer->m_pBits, size.cx, size.cy);

		size = Hover_ZoneSize(&m_pRasterizer->m_dwWidth);
		m_score.DrawDigits(m_pRasterizer->m_pBits, size.cx, size.cy);
		size = Hover_ZoneSize(&m_pRasterizer->m_dwWidth);
		m_ctrJumpPods.DrawDigits(m_pRasterizer->m_pBits, size.cx, size.cy);
		size = Hover_ZoneSize(&m_pRasterizer->m_dwWidth);
		m_ctrSmokeBombs.DrawDigits(m_pRasterizer->m_pBits, size.cx, size.cy);
		size = Hover_ZoneSize(&m_pRasterizer->m_dwWidth);
		m_ctrTempWalls.DrawDigits(m_pRasterizer->m_pBits, size.cx, size.cy);
	}

	::operator delete(pBits);

	if (theApp.m_bFullScreen != 0)
		DrawF4Hint();

	DrawKeyLabels();

	m_dcOffscreen.SelectPalette(pOldPal, TRUE);
}

// FUNCTION: CBumperView::DrawKeyLabels
// hover.exe:0x410380
// FUNCTION: HOVER 0x410380
void CBumperView::DrawKeyLabels()
{
	CString strLabel;
	CFont font;
	ZONE_DESCRIPTOR* pZone;

	// Select the zone descriptor: zone 0 is forced in 2x stretch mode.
	pZone = g_aZones;
	if (m_bDoubleSize == 0)
		pZone += g_dwCurZone;

	// Create the dashboard-label font and select it into the offscreen DC.
	font.Attach(::CreateFont(pZone->m_cyKeyFont, 0, 0, 0, 0x258, 0, 0, 0,
		0, 2, 0, 0, 0x20, "Arial"));

	m_dcOffscreen.SaveDC();
	m_dcOffscreen.SelectObject(&font);
	m_dcOffscreen.SetBkMode(TRANSPARENT);
	m_dcOffscreen.SetTextColor(0);

	// Draw the three dashboard labels into their zone rectangles.  The
	// labels are the configured control keys themselves: the VK codes in
	// theApp.m_avkControls equal the ASCII letter for A-Z and 0-9, and
	// CString::operator=(char) turns each into a one-character string.
	strLabel = (char)theApp.m_avkControls[2];
	m_dcOffscreen.DrawText(strLabel, -1, &pZone->m_rcKeyJump,
		DT_SINGLELINE | DT_CENTER | DT_VCENTER);

	strLabel = (char)theApp.m_avkControls[0];
	m_dcOffscreen.DrawText(strLabel, -1, &pZone->m_rcKeyCloak,
		DT_SINGLELINE | DT_CENTER | DT_VCENTER);

	strLabel = (char)theApp.m_avkControls[1];
	m_dcOffscreen.DrawText(strLabel, -1, &pZone->m_rcKeyWall,
		DT_SINGLELINE | DT_CENTER | DT_VCENTER);

	m_dcOffscreen.RestoreDC(-1);
}

// Draws the small "F4" dashboard overlay in the bottom-right corner of the
// offscreen DC.  In 2x stretch mode a custom 10pt "Small Font" is created
// and selected; otherwise the ANSI variable-width stock font is used.  The
// text is measured, a raised box is drawn around it, the rectangle is shrunk
// and the label is drawn shadowed with DrawMessageLine.

// FUNCTION: CBumperView::DrawF4Hint
// hover.exe:0x428120
// FUNCTION: HOVER 0x428120
void CBumperView::DrawF4Hint()
{
	CFont font;
	CSize size;
	RECT rc;
	int nMargin;

	nMargin = 3;

	m_dcOffscreen.SaveDC();

	if (m_bDoubleSize != 0)
	{
		font.Attach(::CreateFontA(-10, 0, 0, 0, 400, 0, 0, 0, 0, 0, 0, 0, 0,
			"Small Font"));
		nMargin = 2;
		m_dcOffscreen.SelectObject(&font);
	}
	else
	{
		m_dcOffscreen.SelectStockObject(ANSI_VAR_FONT);
	}

	// The inner inset is written out at both use sites (the original
	// re-emits the cmp/adc pair for the InflateRect call).
	size = m_dcOffscreen.GetTextExtent("F4", 2);
	size.cy += nMargin * 2;
	size.cx += (((m_bDoubleSize < 1) ? 2 : 1) + nMargin) * 2;

	rc.right = m_dwOffscreenWidth;
	rc.left = rc.right - size.cx;
	rc.bottom = m_dwOffscreenHeight;
	rc.top = rc.bottom - size.cy;

	DrawBevelBox(&rc, nMargin);
	::InflateRect(&rc, -(((int)(m_bDoubleSize < 1) ? 2 : 1) + nMargin),
		-nMargin);

	DrawMessageLine(&rc, "F4", ::GetSysColor(COLOR_BTNSHADOW),
		::GetSysColor(COLOR_BTNTEXT));

	m_dcOffscreen.SelectStockObject(ANSI_VAR_FONT);
	m_dcOffscreen.RestoreDC(-1);
}

// Re-selects the span drawers after the speed/detail slider moved. The
// slider position indexes g_adwDrawerModeForDetail, whose value is the *mode*
// argument of SetMode (1..6 = the texel/row subsampling family); the
// flags argument is always 3, so both option bits are set and the full
// drawer table is installed. Passing these the other way round leaves the
// context permanently in mode 3 and pairs a shaded span walker with the
// unshaded flat drawer Sky4x1, which then writes past the end of
// the render target.

// FUNCTION: CBumperView::ApplyDetailLevel
// hover.exe:0x413C10
// FUNCTION: HOVER 0x413c10
void CBumperView::ApplyDetailLevel(int nUnk)
{
	m_dwDrawerMode = g_adwDrawerModeForDetail[nUnk];

	m_pViewCtx->SetMode(3, m_dwDrawerMode);
	m_pViewCtx->BuildMipShiftTable();

	if (m_pMirrorCtx != NULL)
	{
		m_pMirrorCtx->SetMode(3, m_dwDrawerMode);
		m_pMirrorCtx->BuildMipShiftTable();
	}
}

// FUNCTION: CBumperView::StartGameTimer
// hover.exe:0x4126C0
// FUNCTION: HOVER 0x4126c0
void CBumperView::StartGameTimer()
{
	BOOL bWasSet;
	LPTIMECALLBACK lpfnTimer;

	g_bStartingTimer = 1;

	if (theApp.m_bFullScreen != 0)
	{
		if (theApp.m_bUseDisplayDib != 0 && theApp.m_bDisplayDibActive == 0)
		{
			RequestDispDibBegin(1);
		}
		else
		{
			::SetMenu(AfxGetApp()->m_pMainWnd->m_hWnd, NULL);
		}

		if (m_bCursorShown != 0)
		{
			::ShowCursor(FALSE);
			m_bCursorShown = 0;
		}
	}

	if (theApp.m_bMusic != 0 && theApp.m_dwGameState == 3)
	{
		::SetThreadPriority((HANDLE)theApp.m_hThread, THREAD_PRIORITY_NORMAL);
		theApp.m_pBumperDoc->m_music.ResumeMusic();
	}

	Hover_WaveMixActivate(1);

	if (theApp.m_bEngineSound != 0)
	{
		((CSoundSlot*)theApp.m_pBumperDoc->m_adwSounds[0x1A])->PlayWave(
			5, 5, 0xFFFF);
	}

	::SetThreadPriority((HANDLE)theApp.m_hThread, THREAD_PRIORITY_ABOVE_NORMAL);

	if (m_pRenderThread != 0)
	{
		::SetThreadPriority((m_pRenderThread)->m_hThread,
			THREAD_PRIORITY_NORMAL);
	}

	if (g_pSoundThread != NULL)
	{
		::SetThreadPriority(g_pSoundThread->m_hThread,
			THREAD_PRIORITY_ABOVE_NORMAL);
	}

	// hover.exe tests the flag for nonzero (cmp/sbb/inc into a boolean)
	// before clearing it, not for == 1
	bWasSet = (m_dwMessageState != 0);
	m_dwMessageState = 0;
	m_rcMessage.bottom = -1;
	if (bWasSet)
	{
		m_bNeedRepaint = 1;
		RequestRenderFrame(1);
		RedrawWindow();
	}

	if (m_dwTimerID == 0)
	{
		DWORD dwTick;

		lpfnTimer = Hover_GameTimerProc;
		m_dwTimerID = timeSetEvent(0x32, 0x0A, lpfnTimer, 0, TIME_PERIODIC);
		if (m_dwTimerID == 0)
			AfxMessageBox("Timer could not be started", 0, 0);
		dwTick = GetTickCount();
		m_dwStepCount = 0;
		m_dwStepRateBaseTick = dwTick;
	}
	else
	{
		AfxMessageBox("Timer tried to start twice", 0, 0);
	}

	g_bStartingTimer = 0;
}

// FUNCTION: CBumperView::ZoomMap
// hover.exe:0x427A80
// FUNCTION: HOVER 0x427a80
void CBumperView::ZoomMap(int nUnk)
{
	ZONE_DESCRIPTOR* pZone;
	DWORD dwMul;
	RECT rcClient;

	pZone = g_aZones;
	if (m_bDoubleSize == 0)
		pZone += g_dwCurZone;

	DWORD* pdwVal = &m_map.m_dwZoom;
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

	if (m_bFullMap == 0)
	{
		m_map.SetViewport(&m_dcOffscreen, this, theApp.m_pBumperDoc,
			pZone->m_rcMap);
	}
	else
	{
		::GetClientRect(m_hWnd, &rcClient);
		m_map.SetViewport(&m_dcOffscreen, this, theApp.m_pBumperDoc,
			rcClient);
	}
}

// hover.exe:0x40FB00
// FUNCTION: HOVER 0x40fb00
void CMapRenderer::SetBits(BYTE* pBits, DWORD dwWidth, DWORD dwHeight)
{
	long lWidth;
	long lRem;
	DWORD* pdwDim;

	// The width/height pair is written through a pointer over the two
	// members and the width read back out of it (rule 94).
	m_pBits = pBits;
	pdwDim = (DWORD*)&m_dwBitsWidth;
	pdwDim[0] = dwWidth;
	pdwDim[1] = dwHeight;

	lWidth = (long)pdwDim[0];
	m_dwPitch = (DWORD)lWidth;
	lRem = lWidth % 4;
	if (lRem != 0)
		lWidth += lRem;
	m_dwPitchDwords = (DWORD)(lWidth >> 2);
}

// hover.exe:0x40FBB0
// FUNCTION: HOVER 0x40fbb0
void CMapRenderer::SetClipRect(RECT rc)
{
	long lLeft;
	long lRight;
	long lFlipTop;
	long lRem;
	long lBits;

	// The rectangle arrives by value and is copied whole.
	m_rcClip = rc;

	lFlipTop = (long)m_dwBitsHeight - rc.top;
	m_rcClip.right--;
	lFlipTop--;
	m_rcClip.bottom--;

	m_dwFlipTop = (DWORD)lFlipTop;
	m_dwFlipBottom = (DWORD)(lFlipTop + rc.top - rc.bottom);

	// The two dword-alignment flags are "the coordinate is a multiple of
	// four", not "leaves remainder one".
	lLeft = rc.left;
	lRem = lLeft % 4;
	m_bLeftAligned = (lRem == 0) ? 1 : 0;

	// Only the flag looks at right+1; the two derived addresses below use
	// right itself.
	lRight = rc.right;
	lRem = (lRight + 1) % 4;
	m_bRightAligned = (lRem == 0) ? 1 : 0;

	lBits = (long)m_pBits;
	if (m_bLeftAligned != 0)
		m_dwLeftAligned = (DWORD)((lLeft & ~3L) + lBits);
	else
		m_dwLeftAligned = (DWORD)((lLeft & ~3L) + lBits + 4);

	m_dwLeftEdge = (DWORD)(lBits + lLeft);
	m_dwRightAligned = (DWORD)((lRight & ~3L) + lBits);
	m_dwRightEdge4 = (DWORD)(lBits + lRight - 4);

	m_dwTopOffset = m_dwPitch * (DWORD)lFlipTop;
	m_dwTopOffsetDw = m_dwPitchDwords * (DWORD)lFlipTop;
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
// The four dword camera fields (m_fxHalfHeight .. m_fxProjBase) are 16.16
// fixed point and are built a word at a time: the width goes into the high
// half of the dword at +0x70 and the height into the high half of the one at
// +0x6C, so halving them gives the half-extents at +0x78 (x) and +0x74 (y)
// without a shift. The projection base at +0x7C is the width scaled by the
// zoom factor m_dwZoom; the divisor at +0x80 is the world size (0x5A00.0000)
// over it, and the base is then recomputed as the world size over the divisor
// so the two are exact inverses of each other in 16.16. The viewport extent
// at +0x88 is the larger of the two half-extents after they have been scaled
// by the divisor and by 1.4199 (0x16b85), i.e. the radius of the circle that
// encloses the viewport.
//
// Every one of those updates goes through HOVER_FIXMULP/HOVER_FIXDIVPP,
// which take the destination by pointer, as the original does.

// hover.exe:0x40FEE0
// FUNCTION: HOVER 0x40fee0
void CMapRenderer::SetViewport(CDC* pDC, CBumperView* pView, CBumperDoc* pDoc,
	RECT rc)
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
	CSpanRasterizer* pRenderTarget;

	fxWorld.s.wFrac = 0;
	fxWorld.s.sInt = 0x5A00;

	m_rcView = rc;

	// Build the 16.16 half-extents at +0x78 and +0x74. The width and the
	// height are stored as the integer part of the dwords at +0x70/+0x6C,
	// so the halves are a plain signed /2.
	m_wWidthFrac = 0;
	m_wWidth = (short)(rc.right - rc.left);
	dwWidth = m_fxWidth;
	m_fxHalfWidth = dwWidth;
	m_fxHalfWidth = (long)dwWidth / 2;

	m_wHeightFrac = 0;
	m_wHeight = (short)(rc.bottom - rc.top);
	dwHeight = m_fxHeight;
	m_fxHalfHeight = dwHeight;
	m_fxHalfHeight = (long)dwHeight / 2;

	// Projection base at +0x7C = width * zoom.
	pdw = &m_fxProjBase;
	*pdw = dwWidth;
	HOVER_FIXMULPP(pdw, m_dwZoom);

	// Divisor at +0x80 = worldSize / base, then base = worldSize / divisor.
	pdw = &m_fxScale;
	*pdw = fxWorld.dw;
	HOVER_FIXDIVPP(pdw, m_fxProjBase);

	pdw = &m_fxProjBase;
	*pdw = fxWorld.dw;
	HOVER_FIXDIVPP(pdw, m_fxScale);

	// The enclosing radius: the larger scaled half-extent.
	dwScaledX = m_fxHalfWidth;
	dwDiv = m_fxScale;
	HOVER_FIXMULPP(&dwScaledX, dwDiv);
	fxX.dw = dwScaledX;
	HOVER_FIXMULPP(&fxX.dw, 0x16b85);

	dwScaledY = m_fxHalfHeight;
	HOVER_FIXMULPP(&dwScaledY, dwDiv);
	fxY.dw = dwScaledY;
	HOVER_FIXMULPP(&fxY.dw, 0x16b85);

	sExtent = fxX.s.sInt;
	if (fxY.s.sInt > sExtent)
		sExtent = fxY.s.sInt;
	m_wViewRadius = sExtent;

	// Set up the render target bits and the span clip state.
	pRenderTarget = pView->m_pRasterizer;
	m_pMapDC = pDC;
	SetBits(pRenderTarget->m_pBits, pRenderTarget->m_dwWidth,
		pRenderTarget->m_dwHeight);
	SetClipRect(rc);

	// Clear the per-object flag in the doc's static object list.
	for (pos = theApp.m_pBumperDoc->m_listMazeObjs.GetHeadPosition(); pos != NULL; )
	{
		pObj = (CGameObj*)theApp.m_pBumperDoc->m_listMazeObjs.GetNext(pos);
		pObj->m_bOnMap = 0;
	}

	// Resolve the draw colors through the game palette.
	m_dwColorHuman = g_palGame.GetNearestPaletteIndex(0x000000FF);
	m_dwColorRobot = g_palGame.GetNearestPaletteIndex(0x00FF0000);
	m_dwColorRobotAlt = g_palGame.GetNearestPaletteIndex(0x00009200);
	m_dwColorYellow = g_palGame.GetNearestPaletteIndex(0x0000FFFF);
	m_dwColorWhite = g_palGame.GetNearestPaletteIndex(0x00FFFFFF);
	m_dwColorBlack = g_palGame.GetNearestPaletteIndex(0x00000000);
	m_dwColorLightGray = g_palGame.GetNearestPaletteIndex(0x00A4A4A4);
	m_dwColorGray = g_palGame.GetNearestPaletteIndex(0x00808080);
}

// FUNCTION: CBumperView::BeginDispDib
// hover.exe:0x427BB0
// FUNCTION: HOVER 0x427bb0
void CBumperView::BeginDispDib()
{
	int i;
	PALETTEENTRY pe;
	BYTE* pColor;
	COPYDATASTRUCT cds;
	BITMAPINFO* pBmi;

	if (theApp.m_bUseDisplayDib == 0)
		return;
	if (theApp.m_bDisplayDibActive != 0)
		return;

	// The original caches theApp.m_pbmiDisplayDib in a pointer local across
	// this run of field writes.
	pBmi = theApp.m_pbmiDisplayDib;

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
		::GetPaletteEntries((HPALETTE)g_palGame.m_hObject, i, 1, &pe);
		pColor += 4;
		pColor[-2] = pe.peRed;
		pColor[-3] = pe.peGreen;
		pColor[-4] = pe.peBlue;
		pColor[-1] = pe.peFlags;
	}

	cds.dwData = 0x400;
	cds.cbData = BMI8_SIZE;
	if (theApp.m_pbmiDisplayDib == NULL)
		cds.cbData = 0;
	cds.lpData = theApp.m_pbmiDisplayDib;

	if (::SendMessageA(theApp.m_hwndDisplayDib, 0x4a, 0, (LPARAM)&cds) != 0)
		return;
	if (::SendMessageA(theApp.m_hwndDisplayDib, 0x403, 0, 0) != 0)
		return;
	theApp.m_bDisplayDibActive = 1;
}

// FUNCTION: CBumperView::EndDispDib
// hover.exe:0x427CC0
// FUNCTION: HOVER 0x427cc0
void CBumperView::EndDispDib()
{
	if (theApp.m_bDisplayDibActive != 0)
	{
		if (::SendMessageA(theApp.m_hwndDisplayDib, 0x404, 0, 0) == 0)
			theApp.m_bDisplayDibActive = 0;
	}
}

// Marks the view inactive (m_dwMessageState = 1) when the game state allows a
// transition (bit 0x80000000 of theApp.m_dwGameState clear, state > 1) and, on
// an actual transition with dwUnk nonzero, sets m_bNeedRepaint, calls
// RequestRenderFrame(1) and forces a full window redraw.

// hover.exe:0x40EE90
// FUNCTION: HOVER 0x40ee90
void CBumperView::SuspendGame(DWORD dwUnk)
{
	BOOL bRedraw;

	if (this == NULL || g_bStartingTimer != 0)
		return;
	if (m_dwTimerID != 0)
		StopGameTimer();
	if ((theApp.m_dwGameState & 0x80000000) != 0)
		return;
	if (theApp.m_dwGameState == 0 || theApp.m_dwGameState == 1)
		return;
	theApp.m_dwGameState |= 0x80000000;
	bRedraw = (m_dwMessageState != 1 && dwUnk != 0);
	m_dwMessageState = 1;
	if (bRedraw)
	{
		m_bNeedRepaint = 1;
		RequestRenderFrame(1);
		RedrawWindow();
	}
}

// hover.exe:0x410280
// FUNCTION: HOVER 0x410280
void CBumperView::StopGameTimer()
{
	CMenu* pMenu;

	if (m_dwTimerID != 0)
		timeKillEvent(m_dwTimerID);
	m_dwTimerID = 0;
	WaitWorkerIdle();
	if (theApp.m_bEngineSound != 0)
		((CSoundSlot*)theApp.m_pBumperDoc->m_adwSounds[0x1A])->StopWave(5, 1);
	if (theApp.m_bMusic != 0)
		theApp.m_pBumperDoc->m_music.PauseMusic();
	Hover_WaveMixActivate(0);
	timeKillEvent(m_dwTimerID);
	if (theApp.m_bFullScreen != 0)
	{
		if (theApp.m_bDisplayDibActive != 0)
		{
			RequestDispDibEnd(1);
		}
		else
		{
			pMenu = CMenu::FromHandle((HMENU)theApp.m_adwSavedWindowState[6]);
			::SetMenu(AfxGetApp()->m_pMainWnd->m_hWnd,
				(pMenu != NULL) ? pMenu->m_hMenu : NULL);
		}
	}
	if (m_bCursorShown == 0)
	{
		ShowCursor(TRUE);
		m_bCursorShown = 1;
	}
	SetThreadPriority((HANDLE)theApp.m_hThread, THREAD_PRIORITY_NORMAL);
	if (m_pRenderThread != 0)
		SetThreadPriority((m_pRenderThread)->m_hThread,
			THREAD_PRIORITY_NORMAL);
	if (g_pSoundThread != NULL)
		SetThreadPriority(g_pSoundThread->m_hThread,
			THREAD_PRIORITY_NORMAL);
}

// Waits (non-blocking) on the six synchronization events, pumps
// m_hEvtCommandAck through Hover_WaitPumpCopyData when it stays signaled, then
// transitions m_dwWorkerCmd to state 3 under theApp.m_csWorkerCmd. When nUnk is
// nonzero the m_hEvtWorkerIdle/m_hEvtCommandAck handshake pair runs around the
// ResetEvent/SetEvent pulse on m_hEvtCommandAck/m_hEvtCommand.

// hover.exe:0x40DC20
// FUNCTION: HOVER 0x40dc20
void CBumperView::RequestRenderFrame(int nUnk)
{
	WaitForSingleObject(m_hEvtCommand, 0);
	WaitForSingleObject(m_hEvtWorkerBusy, 0);
	WaitForSingleObject(m_hEvtWorkerIdle, 0);
	WaitForSingleObject(m_hEvtCommandAck, 0);
	WaitForSingleObject(m_hEvtUnused1, 0);
	WaitForSingleObject(m_hEvtUnused2, 0);
	if (WaitForSingleObject(m_hEvtCommandAck, 0) == WAIT_TIMEOUT &&
		m_dwWorkerCmd != 3)
		Hover_WaitPumpCopyData(m_hEvtCommandAck);
	EnterCriticalSection(&theApp.m_csWorkerCmd);
	m_dwWorkerCmd = 3;
	LeaveCriticalSection(&theApp.m_csWorkerCmd);
	if (nUnk != 0)
	{
		Hover_WaitPumpCopyData(m_hEvtWorkerIdle);
		EnterCriticalSection(&theApp.m_csWorkerCmd);
		ResetEvent(m_hEvtCommandAck);
		SetEvent(m_hEvtCommand);
		LeaveCriticalSection(&theApp.m_csWorkerCmd);
		Hover_WaitPumpCopyData(m_hEvtCommandAck);
		Hover_WaitPumpCopyData(m_hEvtWorkerIdle);
	}
	else
	{
		EnterCriticalSection(&theApp.m_csWorkerCmd);
		ResetEvent(m_hEvtCommandAck);
		SetEvent(m_hEvtCommand);
		LeaveCriticalSection(&theApp.m_csWorkerCmd);
	}
}

// Sets the render state m_dwMessageState. State 0 (the "no game" state) also
// invalidates the cached colour at +0xA0. When the state actually changed and
// nRedraw is set, the worker thread is pushed through RequestRenderFrame and
// the window is repainted synchronously.

// hover.exe:0x41F4E0
// FUNCTION: HOVER 0x41f4e0
void CBumperView::SetMessageState(DWORD dwState, int nRedraw)
{
	BOOL bRedraw;

	bRedraw = (m_dwMessageState != dwState && nRedraw != 0);
	m_dwMessageState = dwState;
	if (dwState == 0)
		m_rcMessage.bottom = -1;
	if (bRedraw)
	{
		m_bNeedRepaint = 1;
		RequestRenderFrame(1);
		RedrawWindow();
	}
}

// Same sequence as RequestRenderFrame, but transitions m_dwWorkerCmd to state
// 6.

// hover.exe:0x4277E0
// FUNCTION: HOVER 0x4277e0
void CBumperView::RequestDispDibBegin(int nUnk)
{
	WaitForSingleObject(m_hEvtCommand, 0);
	WaitForSingleObject(m_hEvtWorkerBusy, 0);
	WaitForSingleObject(m_hEvtWorkerIdle, 0);
	WaitForSingleObject(m_hEvtCommandAck, 0);
	WaitForSingleObject(m_hEvtUnused1, 0);
	WaitForSingleObject(m_hEvtUnused2, 0);
	if (WaitForSingleObject(m_hEvtCommandAck, 0) == WAIT_TIMEOUT &&
		m_dwWorkerCmd != 3)
		Hover_WaitPumpCopyData(m_hEvtCommandAck);
	EnterCriticalSection(&theApp.m_csWorkerCmd);
	m_dwWorkerCmd = 6;
	LeaveCriticalSection(&theApp.m_csWorkerCmd);
	if (nUnk != 0)
	{
		Hover_WaitPumpCopyData(m_hEvtWorkerIdle);
		EnterCriticalSection(&theApp.m_csWorkerCmd);
		ResetEvent(m_hEvtCommandAck);
		SetEvent(m_hEvtCommand);
		LeaveCriticalSection(&theApp.m_csWorkerCmd);
		Hover_WaitPumpCopyData(m_hEvtCommandAck);
		Hover_WaitPumpCopyData(m_hEvtWorkerIdle);
	}
	else
	{
		EnterCriticalSection(&theApp.m_csWorkerCmd);
		ResetEvent(m_hEvtCommandAck);
		SetEvent(m_hEvtCommand);
		LeaveCriticalSection(&theApp.m_csWorkerCmd);
	}
}

// Same sequence as RequestRenderFrame, but transitions m_dwWorkerCmd to state
// 7.

// hover.exe:0x4278E0
// FUNCTION: HOVER 0x4278e0
void CBumperView::RequestDispDibEnd(int nUnk)
{
	WaitForSingleObject(m_hEvtCommand, 0);
	WaitForSingleObject(m_hEvtWorkerBusy, 0);
	WaitForSingleObject(m_hEvtWorkerIdle, 0);
	WaitForSingleObject(m_hEvtCommandAck, 0);
	WaitForSingleObject(m_hEvtUnused1, 0);
	WaitForSingleObject(m_hEvtUnused2, 0);
	if (WaitForSingleObject(m_hEvtCommandAck, 0) == WAIT_TIMEOUT &&
		m_dwWorkerCmd != 3)
		Hover_WaitPumpCopyData(m_hEvtCommandAck);
	EnterCriticalSection(&theApp.m_csWorkerCmd);
	m_dwWorkerCmd = 7;
	LeaveCriticalSection(&theApp.m_csWorkerCmd);
	if (nUnk != 0)
	{
		Hover_WaitPumpCopyData(m_hEvtWorkerIdle);
		EnterCriticalSection(&theApp.m_csWorkerCmd);
		ResetEvent(m_hEvtCommandAck);
		SetEvent(m_hEvtCommand);
		LeaveCriticalSection(&theApp.m_csWorkerCmd);
		Hover_WaitPumpCopyData(m_hEvtCommandAck);
		Hover_WaitPumpCopyData(m_hEvtWorkerIdle);
	}
	else
	{
		EnterCriticalSection(&theApp.m_csWorkerCmd);
		ResetEvent(m_hEvtCommandAck);
		SetEvent(m_hEvtCommand);
		LeaveCriticalSection(&theApp.m_csWorkerCmd);
	}
}

// Same sequence as RequestRenderFrame, but transitions m_dwWorkerCmd to state
// 1 and publishes the new speed/detail level in theApp.m_dwSpeedDetail while
// the worker thread is parked inside the critical section. Called by the
// "Customize Game" dialog when the speed/detail slider moved.

// hover.exe:0x413A00
// FUNCTION: HOVER 0x413a00
void CBumperView::RequestDetailLevel(int nLevel, int nUnk)
{
	WaitForSingleObject(m_hEvtCommand, 0);
	WaitForSingleObject(m_hEvtWorkerBusy, 0);
	WaitForSingleObject(m_hEvtWorkerIdle, 0);
	WaitForSingleObject(m_hEvtCommandAck, 0);
	WaitForSingleObject(m_hEvtUnused1, 0);
	WaitForSingleObject(m_hEvtUnused2, 0);
	if (WaitForSingleObject(m_hEvtCommandAck, 0) == WAIT_TIMEOUT &&
		m_dwWorkerCmd != 1)
		Hover_WaitPumpCopyData(m_hEvtCommandAck);
	EnterCriticalSection(&theApp.m_csWorkerCmd);
	theApp.m_dwSpeedDetail = nLevel;
	m_dwWorkerCmd = 1;
	LeaveCriticalSection(&theApp.m_csWorkerCmd);
	if (nUnk != 0)
	{
		Hover_WaitPumpCopyData(m_hEvtWorkerIdle);
		EnterCriticalSection(&theApp.m_csWorkerCmd);
		ResetEvent(m_hEvtCommandAck);
		SetEvent(m_hEvtCommand);
		LeaveCriticalSection(&theApp.m_csWorkerCmd);
		Hover_WaitPumpCopyData(m_hEvtCommandAck);
		Hover_WaitPumpCopyData(m_hEvtWorkerIdle);
	}
	else
	{
		EnterCriticalSection(&theApp.m_csWorkerCmd);
		ResetEvent(m_hEvtCommandAck);
		SetEvent(m_hEvtCommand);
		LeaveCriticalSection(&theApp.m_csWorkerCmd);
	}
}

// Same sequence as RequestRenderFrame, but transitions m_dwWorkerCmd to state
// 4. Run after the player control bindings changed so the worker thread picks
// up the new ones.

// hover.exe:0x413B10
// FUNCTION: HOVER 0x413b10
void CBumperView::RequestDashboardRebuild(int nUnk)
{
	WaitForSingleObject(m_hEvtCommand, 0);
	WaitForSingleObject(m_hEvtWorkerBusy, 0);
	WaitForSingleObject(m_hEvtWorkerIdle, 0);
	WaitForSingleObject(m_hEvtCommandAck, 0);
	WaitForSingleObject(m_hEvtUnused1, 0);
	WaitForSingleObject(m_hEvtUnused2, 0);
	if (WaitForSingleObject(m_hEvtCommandAck, 0) == WAIT_TIMEOUT &&
		m_dwWorkerCmd != 4)
		Hover_WaitPumpCopyData(m_hEvtCommandAck);
	EnterCriticalSection(&theApp.m_csWorkerCmd);
	m_dwWorkerCmd = 4;
	LeaveCriticalSection(&theApp.m_csWorkerCmd);
	if (nUnk != 0)
	{
		Hover_WaitPumpCopyData(m_hEvtWorkerIdle);
		EnterCriticalSection(&theApp.m_csWorkerCmd);
		ResetEvent(m_hEvtCommandAck);
		SetEvent(m_hEvtCommand);
		LeaveCriticalSection(&theApp.m_csWorkerCmd);
		Hover_WaitPumpCopyData(m_hEvtCommandAck);
		Hover_WaitPumpCopyData(m_hEvtWorkerIdle);
	}
	else
	{
		EnterCriticalSection(&theApp.m_csWorkerCmd);
		ResetEvent(m_hEvtCommandAck);
		SetEvent(m_hEvtCommand);
		LeaveCriticalSection(&theApp.m_csWorkerCmd);
	}
}

// hover.exe:0x4279E0
// FUNCTION: HOVER 0x4279e0
void CBumperView::WaitWorkerIdle()
{
	ResetEvent(m_hEvtCommand);
	Hover_WaitPumpCopyData(m_hEvtWorkerIdle);
	SetEvent(m_hEvtCommandAck);
}

// Re-establishes the offscreen surfaces for the current client size and
// restarts the render thread. Called by CMainFrame::OnSize and
// CMainFrame::OnExitSizeMove once the frame has settled on its new size.
// hover.exe:0x41F480
// FUNCTION: HOVER 0x41f480
void CBumperView::OnFrameResized()
{
	RECT rect;
	int cx;
	int cy;

	if (m_bInitialized == 0)
		return;

	::GetClientRect(m_hWnd, &rect);
	WaitWorkerIdle();
	cy = rect.bottom - rect.top;
	cx = rect.right - rect.left;
	RequestResize(cx, cy, 1);
	RequestRenderFrame(1);
}

// hover.exe:0x40C480
// FUNCTION: HOVER 0x40c480
void CBumperView::OnDraw(CDC* pDC)
{
	RECT rect;
	CPalette* pOldPal;
	int cx;
	int cy;

	if (theApp.m_bDisplayDibActive != 0)
		return;
	if (m_bInitialized == 0)
		return;
	if (m_pRenderThread == 0)
		return;
	pOldPal = pDC->SelectPalette(&g_palGame, TRUE);
	pDC->RealizePalette();
	::GetClientRect(m_hWnd, &rect);
	if (m_dwTimerID == 0 &&
		WaitForSingleObject(m_hEvtWorkerIdle, 0) == WAIT_TIMEOUT)
	{
		m_bNeedRepaint = 1;
	}
	else
	{
		EnterCriticalSection(&theApp.m_csOffscreen);
		cx = (int)m_dwOffscreenWidth;
		cy = (int)m_dwOffscreenHeight;
		if (m_bDoubleSize != 0)
			StretchBlt(pDC->m_hDC, 0, 0, cx * 2, cy * 2,
				m_dcOffscreen.GetSafeHdc(), 0, 0, cx, cy, SRCCOPY);
		else
			BitBlt(pDC->m_hDC, 0, 0, cx, cy,
				m_dcOffscreen.GetSafeHdc(), 0, 0, SRCCOPY);
		LeaveCriticalSection(&theApp.m_csOffscreen);
	}
	pDC->SelectPalette(pOldPal, TRUE);
}

// Same handshake sequence as RequestRenderFrame, but transitions
// m_dwWorkerCmd to state 5 and records nUnk in m_bZoomIn (the requested view
// scale).

// hover.exe:0x4275D0
// FUNCTION: HOVER 0x4275d0
void CBumperView::RequestMapZoom(int nUnk, int nHandshake)
{
	WaitForSingleObject(m_hEvtCommand, 0);
	WaitForSingleObject(m_hEvtWorkerBusy, 0);
	WaitForSingleObject(m_hEvtWorkerIdle, 0);
	WaitForSingleObject(m_hEvtCommandAck, 0);
	WaitForSingleObject(m_hEvtUnused1, 0);
	WaitForSingleObject(m_hEvtUnused2, 0);
	if (WaitForSingleObject(m_hEvtCommandAck, 0) == WAIT_TIMEOUT &&
		m_dwWorkerCmd != 5)
		Hover_WaitPumpCopyData(m_hEvtCommandAck);
	EnterCriticalSection(&theApp.m_csWorkerCmd);
	m_bZoomIn = nUnk;
	m_dwWorkerCmd = 5;
	LeaveCriticalSection(&theApp.m_csWorkerCmd);
	if (nHandshake != 0)
	{
		Hover_WaitPumpCopyData(m_hEvtWorkerIdle);
		EnterCriticalSection(&theApp.m_csWorkerCmd);
		ResetEvent(m_hEvtCommandAck);
		SetEvent(m_hEvtCommand);
		LeaveCriticalSection(&theApp.m_csWorkerCmd);
		Hover_WaitPumpCopyData(m_hEvtCommandAck);
		Hover_WaitPumpCopyData(m_hEvtWorkerIdle);
	}
	else
	{
		EnterCriticalSection(&theApp.m_csWorkerCmd);
		ResetEvent(m_hEvtCommandAck);
		SetEvent(m_hEvtCommand);
		LeaveCriticalSection(&theApp.m_csWorkerCmd);
	}
}

// Same sequence as RequestRenderFrame, but transitions m_dwWorkerCmd to state 2
// (the worker thread's "stop" state, used when the renderer is torn down).

// hover.exe:0x4276E0
// FUNCTION: HOVER 0x4276e0
void CBumperView::RequestWorkerExit(int nUnk)
{
	WaitForSingleObject(m_hEvtCommand, 0);
	WaitForSingleObject(m_hEvtWorkerBusy, 0);
	WaitForSingleObject(m_hEvtWorkerIdle, 0);
	WaitForSingleObject(m_hEvtCommandAck, 0);
	WaitForSingleObject(m_hEvtUnused1, 0);
	WaitForSingleObject(m_hEvtUnused2, 0);
	if (WaitForSingleObject(m_hEvtCommandAck, 0) == WAIT_TIMEOUT &&
		m_dwWorkerCmd != 2)
		Hover_WaitPumpCopyData(m_hEvtCommandAck);
	EnterCriticalSection(&theApp.m_csWorkerCmd);
	m_dwWorkerCmd = 2;
	LeaveCriticalSection(&theApp.m_csWorkerCmd);
	if (nUnk != 0)
	{
		Hover_WaitPumpCopyData(m_hEvtWorkerIdle);
		EnterCriticalSection(&theApp.m_csWorkerCmd);
		ResetEvent(m_hEvtCommandAck);
		SetEvent(m_hEvtCommand);
		LeaveCriticalSection(&theApp.m_csWorkerCmd);
		Hover_WaitPumpCopyData(m_hEvtCommandAck);
		Hover_WaitPumpCopyData(m_hEvtWorkerIdle);
	}
	else
	{
		EnterCriticalSection(&theApp.m_csWorkerCmd);
		ResetEvent(m_hEvtCommandAck);
		SetEvent(m_hEvtCommand);
		LeaveCriticalSection(&theApp.m_csWorkerCmd);
	}
}

// Tears the renderer down: stop and delete the worker thread, close the six
// synchronization events, delete the two render contexts and the offscreen
// render target, and release the offscreen DC. Called from OnDestroy.

// hover.exe:0x41F350
// FUNCTION: HOVER 0x41f350
void CBumperView::DestroyRenderer()
{
	CRenderContext* pCtx;
	CSpanRasterizer* pTarget;

	if (m_pRenderThread != 0)
	{
		RequestWorkerExit(1);
		Hover_WaitPumpCopyData((m_pRenderThread)->m_hThread);
		if (m_pRenderThread != 0)
			delete m_pRenderThread;
		m_pRenderThread = 0;
	}
	if (m_hEvtCommand != 0)
	{
		CloseHandle(m_hEvtCommand);
		CloseHandle(m_hEvtWorkerBusy);
		CloseHandle(m_hEvtWorkerIdle);
		CloseHandle(m_hEvtCommandAck);
		CloseHandle(m_hEvtUnused1);
		CloseHandle(m_hEvtUnused2);
		m_hEvtCommand = 0;
		m_hEvtWorkerBusy = NULL;
		m_hEvtWorkerIdle = NULL;
		m_hEvtCommandAck = NULL;
		m_hEvtUnused1 = NULL;
		m_hEvtUnused2 = NULL;
	}
	pCtx = m_pViewCtx;
	if (pCtx != NULL)
		delete pCtx;
	pCtx = m_pMirrorCtx;
	if (pCtx != NULL)
		delete pCtx;
	pTarget = m_pRasterizer;
	if (pTarget != NULL)
		delete pTarget;
	m_pViewCtx = NULL;
	m_pMirrorCtx = NULL;
	m_pRasterizer = NULL;
	::SelectObject(m_dcOffscreen.GetSafeHdc(), (HGDIOBJ)m_hOldBitmap);
	m_dcOffscreen.DeleteDC();
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
		long _nSeg = (long)(obj).m_dwSegments;			\
		if (_nSeg > 0)								\
		{											\
			(obj).m_dwLevel = _nSeg;				\
			(obj).m_dwLevel = 0;					\
		}											\
		else										\
		{											\
			(obj).m_dwLevel = _nSeg;				\
		}											\
	} while (0)

#define HOVER_RESET_SPRITE(obj)						\
	do {											\
		long _nSpr = (long)(obj).m_dwSlots;			\
		if (_nSpr > 0)								\
		{											\
			(obj).m_dwFilled = _nSpr;				\
			(obj).m_dwFilled = 0;					\
		}											\
		else										\
		{											\
			(obj).m_dwFilled = _nSpr;				\
		}											\
	} while (0)

// hover.exe:0x41E5B0
// FUNCTION: HOVER 0x41e5b0
void CBumperView::ResetDashboard()
{
	m_ctrJumpPods.m_bDigitsDirty = 1;
	m_ctrJumpPods.m_dwDisplayValue = 0;
	m_ctrSmokeBombs.m_bDigitsDirty = 1;
	m_ctrSmokeBombs.m_dwDisplayValue = 0;
	m_ctrTempWalls.m_bDigitsDirty = 1;
	m_ctrTempWalls.m_dwDisplayValue = 0;
	HOVER_RESET_GAUGE(m_gaugeSpeed);
	HOVER_RESET_GAUGE(m_gaugeInvincible);
	HOVER_RESET_GAUGE(m_gaugeSlow);
	HOVER_RESET_GAUGE(m_gaugeSpeedPod);
	HOVER_RESET_GAUGE(m_gaugeAltitude);
	HOVER_RESET_GAUGE(m_gaugeSmoke);
	HOVER_RESET_GAUGE(m_gaugeTempWall);
	m_flagsPlayer.m_bFlagsDirty = 1;
	HOVER_RESET_SPRITE(m_flagsPlayer);
	m_flagsRobot.m_bFlagsDirty = 1;
	HOVER_RESET_SPRITE(m_flagsRobot);
	m_score.m_bDigitsDirty = 1;
	theApp.m_pBumperDoc->m_dwScore = 0;
	m_score.m_dwDisplayValue = 0;
}

// Awards the end-of-level score. m_dwFlagsLeft is the number of ticks left on
// the level clock (the level's tick allowance minus the elapsed ticks),
// m_dwPointsPerFlag the per-tick value for the current skill level and
// m_dwFlagBonus their product; m_dwNextLevel is the number of the level that
// follows and m_dwLevelBonus the flat level bonus, which is only awarded when
// bLevelBonus is set (the caller clears it when the level was restarted).
// Both go into the document's running total, which is then shown on the
// scoreboard.
//
// The original spells the LVLPARAM index expression out three times rather
// than computing it once.

// hover.exe:0x4282F0
// FUNCTION: HOVER 0x4282f0
void CBumperView::ComputeLevelBonus(DWORD bLevelBonus)
{
	int nLevel = (int)g_dwLevel;
	DWORD dwPerTick;

	m_dwFlagsLeft = LVLPARAM(nLevel).m_dwUnk30
		- theApp.m_pBumperDoc->m_dwRobotFlags;
	dwPerTick = LVLPARAM(nLevel).m_anRemainingScore[theApp.m_dwSkill];
	m_dwPointsPerFlag = dwPerTick;
	m_dwNextLevel = (DWORD)(nLevel + 2);
	m_dwFlagBonus = dwPerTick * m_dwFlagsLeft;

	if (bLevelBonus == 0)
		m_dwLevelBonus = 0;
	else
		m_dwLevelBonus = LVLPARAM(nLevel).m_anLevelScore[theApp.m_dwSkill];

	theApp.m_pBumperDoc->m_dwScore += m_dwLevelBonus + m_dwFlagBonus;
	m_score.m_bDigitsDirty = 1;
	m_score.m_dwDisplayValue = theApp.m_pBumperDoc->m_dwScore;
}

// The two per-frame steps run from the WM_USER+0 handler (the multimedia
// timer posts it twenty times a second): GameStep advances the game, and
// UpdateStatusBar refreshes the two debug panes of the status bar.

// The timed game states run their countdown in m_dwStateCountdown here. State
// 2 is the pre-game countdown - the "get set" sound fires at tick 40 and the
// state flips to 3 (playing) at 0, restarting the level music; states 4 and 5
// wait out the end-of-game / end-of-level delay, stop the music and post
// WM_USER+1 / WM_USER+2 so the high-score or next-level flow runs outside the
// timer callback. State 2 falls through into the state 3 step, so the
// countdown frames are played as usual.

// hover.exe:0x408D00
// FUNCTION: HOVER 0x408d00
void CBumperView::GameStep()
{
	switch (theApp.m_dwGameState)
	{
	case 2:
		if (m_dwStateCountdown == 0x28 && theApp.m_bGameSounds != 0)
			((CSoundSlot*)theApp.m_pBumperDoc->m_adwSounds[1])->PlayWave(0, 5, 0);
		if (--m_dwStateCountdown == 0)
		{
			theApp.m_dwGameState = 3;
			m_dwStateCountdown = 0x3C;
			if (theApp.m_bMusic != 0)
				theApp.m_pBumperDoc->m_music.ResumeMusic();
		}
		// falls through into the state 3 step

	case 3:
		StepObjects();
		break;

	case 4:
		if (m_dwStateCountdown == 0x3C)
			theApp.m_pBumperDoc->m_music.PauseMusic();
		if (--m_dwStateCountdown == 0)
		{
			StopGameTimer();
			::PostMessage(m_hWnd, WM_USER + 1, 0, 0);
		}
		break;

	case 5:
		if (m_dwStateCountdown == 0x3C)
			theApp.m_pBumperDoc->m_music.PauseMusic();
		if (--m_dwStateCountdown == 0)
		{
			StopGameTimer();
			::PostMessage(m_hWnd, WM_USER + 2, 0, 0);
		}
		break;
	}
}

// Refresh the two debug panes of the status bar, at most once every 58
// ticks (~3 s) and only while a game is running - m_dwGameState's top bit
// marks the suspended states, and states 0 and 1 have no game yet.
//
// Pane 1 gets "<present rate>/<step rate>", both averaged over the frames
// counted since the last refresh: m_dwLastPresentTick/m_dwPresentCount are
// the tick and frame counter of CBumperView::PresentFrame (the present),
// m_dwLastStepTick/ m_dwStepCount those of StepObjects (the game step). The
// rates are formatted by hand with _fcvt (one fractional digit) rather than
// with sprintf. Pane 0 gets the human player's position and heading.

// hover.exe:0x408DF0
// FUNCTION: HOVER 0x408df0
void CBumperView::UpdateStatusBar()
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
	dwState = theApp.m_dwGameState & 0x7FFFFFFF;
	if (dwState == 0 || dwState == 1 ||
		(theApp.m_dwGameState & 0x80000000) != 0)
		return;
	nTicks = g_nStatusRefreshCountdown;
	g_nStatusRefreshCountdown = nTicks - 1;
	if (nTicks > 0)
		return;

	dwFrames = m_dwPresentCount;
	if (dwFrames == 0)
		return;
	if (m_dwStepCount == 0)
		return;

	dwTick = m_dwLastPresentTick;
	m_dwPresentCount = 0;
	dwMsPerFrame = (dwTick - m_dwFpsBaseTick) / dwFrames;
	m_dwFpsBaseTick = dwTick;
	dwTick = m_dwLastStepTick;
	dblFrameRate = 1000.0 / dwMsPerFrame;
	dwMsPerStep = (dwTick - m_dwStepRateBaseTick) / m_dwStepCount;
	m_dwStepRateBaseTick = dwTick;
	g_nStatusRefreshCountdown = 0x39;
	m_dwStepCount = 0;
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
	((CMainFrame*)AfxGetApp()->m_pMainWnd)->SetPaneMessage(1, szText);

	// "X:<x>, Y:<y>, A:<heading>" - the position words come back from
	// CGameObj::GetCellPos sign-extended, the heading is read directly.
	strcpy(szText, "X:");
	_ltoa((long)pDoc->m_humanPlayer.GetCellPos(adwPos)[0], &szText[2], 10);
	strcat(&szText[2], ", Y:");
	psz = &szText[2] + strlen(&szText[2]);
	_ltoa((long)pDoc->m_humanPlayer.GetCellPos(adwPos)[1], psz, 10);
	strcat(psz, ", A:");
	psz += strlen(psz);
	_ltoa((long)(short)pDoc->m_humanPlayer.m_wHeading, psz, 10);
	((CMainFrame*)AfxGetApp()->m_pMainWnd)->SetPaneMessage(0, szText);
}

// The game step proper. While the renderer is in state 3 the worker thread
// is released for one frame, then every object in the document's second
// list gets its input/think virtual, the per-frame renderer counters are
// cleared and the collider runs its pass. Finally the newly pressed
// joystick buttons are translated into WM_KEYDOWNs for the keys they are
// bound to and the step is timed for the status bar.

// hover.exe:0x427D40
// FUNCTION: HOVER 0x427d40
void CBumperView::StepObjects()
{
	DWORD dwPressed;

	if (m_dwWorkerCmd == 3)
	{
		EnterCriticalSection(&theApp.m_csWorkerCmd);
		ResetEvent(m_hEvtCommand);
		SetEvent(m_hEvtCommandAck);
		LeaveCriticalSection(&theApp.m_csWorkerCmd);
	}

	ThinkAll();
	RunCollider();

	EnterCriticalSection(&theApp.m_csScene);
	Hover_CommitDirtyObjects();
	LeaveCriticalSection(&theApp.m_csScene);

	RequestRenderFrame(0);

	dwPressed = 0;
	if (theApp.m_avkControls[11] != 0)
	{
		dwPressed = ~g_dwPrevJoyButtons & theApp.m_joyInfo.dwButtons;
		g_dwPrevJoyButtons = theApp.m_joyInfo.dwButtons;
	}
	if ((dwPressed & 8) != 0)
		::SendMessage(m_hWnd, WM_KEYDOWN, theApp.m_avkControls[1], 0);
	if ((dwPressed & 6) != 0)
		::SendMessage(m_hWnd, WM_KEYDOWN, theApp.m_avkControls[0], 0);
	if ((dwPressed & 1) != 0)
		::SendMessage(m_hWnd, WM_KEYDOWN, theApp.m_avkControls[2], 0);

	m_dwLastStepTick = GetTickCount();
	m_dwStepCount++;
}

// Run the input/think virtual (vtable slot 7) on every object in the
// document's second object list.

// hover.exe:0x427E50
// FUNCTION: HOVER 0x427e50
void CBumperView::ThinkAll()
{
	CBumperDoc* pDoc;
	POSITION pos;

	pDoc = (CBumperDoc*)m_pDocument;
	pos = pDoc->m_listActiveObjs.GetHeadPosition();
	while (pos != NULL)
		((CGameObj*)pDoc->m_listActiveObjs.GetNext(pos))->Think();
}

// Clear the four renderer debug counters shown by the ID_DBG_* update
// handler and hand the frame to the collider.

// hover.exe:0x427E90
// FUNCTION: HOVER 0x427e90
void CBumperView::RunCollider()
{
	CBumperDoc* pDoc;
	DWORD* pdwCounters;

	pDoc = (CBumperDoc*)m_pDocument;
	pdwCounters = &m_dwUnk0518;
	pdwCounters[0] = 0;
	pdwCounters[1] = 0;
	pdwCounters[2] = 0;
	pdwCounters[3] = 0;
	(pDoc->m_pCollider)->DetectCollisions();
}

/////////////////////////////////////////////////////////////////////////////
// CBumperView message handlers
//
// The original's message map (AFX_MSGMAP_ENTRY table at hover.exe:0x4BB0C8,
// AFX_MSGMAP at 0x4BB0C0, base CView) is reproduced entry for entry, in the
// original order, by the BEGIN_MESSAGE_MAP block at the top of this file.

// The original module was compiled with automatic inlining on, so
// CBumperView::SuspendGame(1) - the "suspend the game loop" transition, which
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
		if (this == NULL || g_bStartingTimer != 0)					\
			break;													\
		if (m_dwTimerID != 0)										\
			StopGameTimer();											\
		if ((theApp.m_dwGameState & 0x80000000) != 0)					\
			break;													\
		if (theApp.m_dwGameState == 0 || theApp.m_dwGameState == 1)		\
			break;													\
		theApp.m_dwGameState |= 0x80000000;							\
		bSuspended_ = (m_dwMessageState != 1);							\
		m_dwMessageState = 1;											\
		if (bSuspended_)											\
		{															\
			m_bNeedRepaint = 1;										\
			RequestRenderFrame(1);											\
			RedrawWindow();											\
		}															\
	} while (0)

// Entry 1: WM_DESTROY. Stops the frame timer, tears the renderer down and
// lets CView do the rest.

// hover.exe:0x41E710
// FUNCTION: HOVER 0x41e710
void CBumperView::OnDestroy()
{
	if (m_dwTimerID != 0)
		StopGameTimer();
	DestroyRenderer();
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
		((CMainFrame*)GetParentFrame())->SetPaneMessage(2, szText);
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
	((CBumperDoc*)m_pDocument)->m_humanPlayer.m_bSteerAssist =
		(((CBumperDoc*)m_pDocument)->m_humanPlayer.m_bSteerAssist == 0);
}

// hover.exe:0x427EE0
// FUNCTION: HOVER 0x427ee0
void CBumperView::OnUpdateCmd34000(CCmdUI* pCmdUI)
{
	CBumperDoc* pDoc = (CBumperDoc*)m_pDocument;

	pCmdUI->SetCheck(pDoc->m_humanPlayer.m_bSteerAssist);
}

// Entries 6/7: ID_CMD_34011 toggles the human player's +0x1E0 flag.

// hover.exe:0x427F00
// FUNCTION: HOVER 0x427f00
void CBumperView::OnCmd34011()
{
	((CBumperDoc*)m_pDocument)->m_humanPlayer.m_bFriction =
		(((CBumperDoc*)m_pDocument)->m_humanPlayer.m_bFriction == 0);
}

// hover.exe:0x427F20
// FUNCTION: HOVER 0x427f20
void CBumperView::OnUpdateCmd34011(CCmdUI* pCmdUI)
{
	CBumperDoc* pDoc = (CBumperDoc*)m_pDocument;

	pCmdUI->SetCheck(pDoc->m_humanPlayer.m_bFriction);
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
void CBumperView::OnDbgAddSmokePod()
{
	CSmokeBombPod* pPod;

	if (m_dwUnk0038 != 0)
		return;
	if (theApp.m_pBumperDoc->m_humanPlayer.m_listSmokeBombPods.GetCount() >= 9)
		return;
	pPod = new CSmokeBombPod;
	pPod->m_pOwner = &theApp.m_pBumperDoc->m_humanPlayer;
	theApp.m_pBumperDoc->m_humanPlayer.m_listSmokeBombPods.AddHead(pPod);
	m_ctrSmokeBombs.m_bDigitsDirty = 1;
	m_ctrSmokeBombs.m_dwDisplayValue = theApp.m_pBumperDoc->m_humanPlayer.m_listSmokeBombPods.GetCount();
}

// hover.exe:0x428610
// FUNCTION: HOVER 0x428610
void CBumperView::OnDbgAddWallPod()
{
	CTempWallPod* pPod;

	if (m_dwUnk0038 != 0)
		return;
	if (theApp.m_pBumperDoc->m_humanPlayer.m_listTempWallPods.GetCount() >= 9)
		return;
	pPod = new CTempWallPod;
	pPod->m_pOwner = &theApp.m_pBumperDoc->m_humanPlayer;
	theApp.m_pBumperDoc->m_humanPlayer.m_listTempWallPods.AddHead(pPod);
	m_ctrTempWalls.m_bDigitsDirty = 1;
	m_ctrTempWalls.m_dwDisplayValue = theApp.m_pBumperDoc->m_humanPlayer.m_listTempWallPods.GetCount();
}

// hover.exe:0x428690
// FUNCTION: HOVER 0x428690
void CBumperView::OnDbgAddJumpPod()
{
	CJumpPod* pPod;

	if (m_dwUnk0038 != 0)
		return;
	if (theApp.m_pBumperDoc->m_humanPlayer.m_listJumpPods.GetCount() >= 9)
		return;
	pPod = new CJumpPod;
	pPod->m_pOwner = &theApp.m_pBumperDoc->m_humanPlayer;
	theApp.m_pBumperDoc->m_humanPlayer.m_listJumpPods.AddHead(pPod);
	m_ctrJumpPods.m_bDigitsDirty = 1;
	m_ctrJumpPods.m_dwDisplayValue = theApp.m_pBumperDoc->m_humanPlayer.m_listJumpPods.GetCount();
}

// Entries 12..15: debug commands that apply a pod effect to the human
// player immediately (the pod is created, marked "already used" through
// m_dwVStep = 0 and handed straight to its own effect handler).

// hover.exe:0x428710
// FUNCTION: HOVER 0x428710
void CBumperView::OnDbgApplyInvincible()
{
	CInviciblePod* pPod;

	if (m_dwUnk0038 != 0)
		return;
	pPod = new CInviciblePod;
	pPod->m_bRespawn = 0;
	pPod->m_pOwner = &theApp.m_pBumperDoc->m_humanPlayer;
	pPod->OnTouch(&theApp.m_pBumperDoc->m_humanPlayer, 0);
}

// hover.exe:0x428770
// FUNCTION: HOVER 0x428770
void CBumperView::OnDbgApplyMapEraser()
{
	CMapEraserPod* pPod;

	if (m_dwUnk0038 != 0)
		return;
	pPod = new CMapEraserPod;
	pPod->m_bRespawn = 0;
	pPod->m_pOwner = &theApp.m_pBumperDoc->m_humanPlayer;
	pPod->OnTouch(&theApp.m_pBumperDoc->m_humanPlayer, 0);
}

// hover.exe:0x4287D0
// FUNCTION: HOVER 0x4287d0
void CBumperView::OnDbgApplySpeed()
{
	CSpeedPod* pPod;

	if (m_dwUnk0038 != 0)
		return;
	pPod = new CSpeedPod;
	pPod->m_bRespawn = 0;
	pPod->m_pOwner = &theApp.m_pBumperDoc->m_humanPlayer;
	pPod->OnTouch(&theApp.m_pBumperDoc->m_humanPlayer, 0);
}

// hover.exe:0x428830
// FUNCTION: HOVER 0x428830
void CBumperView::OnDbgApplySlow()
{
	CSlowPod* pPod;

	if (m_dwUnk0038 != 0)
		return;
	pPod = new CSlowPod;
	pPod->m_bRespawn = 0;
	pPod->m_pOwner = &theApp.m_pBumperDoc->m_humanPlayer;
	pPod->OnTouch(&theApp.m_pBumperDoc->m_humanPlayer, 0);
}

// Entry 16: ID_GAME_PAUSE. Bit 0x80000000 of theApp.m_dwGameState is the
// paused flag; the low bits are the game state. Resuming repaints the seven
// dashboard gauges and restarts the frame timer, pausing runs the inlined
// SuspendGame(1) transition.

// hover.exe:0x413C70
// FUNCTION: HOVER 0x413c70
void CBumperView::OnGamePause()
{
	DWORD dwState;

	dwState = theApp.m_dwGameState & 0x7FFFFFFF;
	if (dwState == 0 || dwState == 1)
		return;

	if ((theApp.m_dwGameState & 0x80000000) != 0)
	{
		m_gaugeSpeed.DrawGauge(&m_dcOffscreen, 1);
		m_gaugeSpeedPod.DrawGauge(&m_dcOffscreen, 1);
		m_gaugeSlow.DrawGauge(&m_dcOffscreen, 1);
		m_gaugeInvincible.DrawGauge(&m_dcOffscreen, 1);
		m_gaugeAltitude.DrawGauge(&m_dcOffscreen, 1);
		m_gaugeSmoke.DrawGauge(&m_dcOffscreen, 1);
		m_gaugeTempWall.DrawGauge(&m_dcOffscreen, 1);
		if ((theApp.m_dwGameState & 0x80000000) != 0)
		{
			theApp.m_dwGameState &= 0x7FFFFFFF;
			StartGameTimer();
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

	dwState = theApp.m_dwGameState & 0x7FFFFFFF;
	pCmdUI->Enable(dwState != 0 && dwState != 1);
	pCmdUI->SetCheck((theApp.m_dwGameState & 0x80000000) >> 31);
}

// Entries 18/19: ID_DBG_FULLCLIENT_RENDER toggles the "render into the full
// client rectangle" debug mode (m_bFullMap). Turning it on re-derives the
// renderer viewport from the client rectangle, turning it off restores the
// offscreen bitmap size.

// hover.exe:0x428970
// FUNCTION: HOVER 0x428970
void CBumperView::OnDbgFullClientRender()
{
	RECT rect;

	m_bFullMap = (m_bFullMap == 0);
	if (m_bFullMap == 0)
	{
		RequestResize(m_dwRequestedWidth, m_dwRequestedHeight, 0);
		return;
	}
	::GetClientRect(m_hWnd, &rect);
	m_map.SetViewport(&m_dcOffscreen, this, (CBumperDoc*)m_pDocument, rect);
}

// hover.exe:0x4289F0
// FUNCTION: HOVER 0x4289f0
void CBumperView::OnUpdateDbgFullClientRender(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(m_bFullMap);
}

// Entry 20: WM_KEYDOWN. Only handled while the game is running (state 3):
// +/- change the view scale, and the three inventory keys drop the last pod
// of the matching list on the human player.

// hover.exe:0x40B540
// FUNCTION: HOVER 0x40b540
void CBumperView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if (theApp.m_dwGameState != 3)
	{
		Default();
		return;
	}

	if (nChar == VK_ADD ||
		(UINT)(short)::VkKeyScanA('+') == nChar ||
		(UINT)(short)::VkKeyScanA('=') == nChar)
	{
		RequestMapZoom(1, 0);
	}
	else if (nChar == VK_SUBTRACT ||
		(UINT)(short)::VkKeyScanA('-') == nChar ||
		(UINT)(short)::VkKeyScanA('_') == nChar)
	{
		RequestMapZoom(0, 0);
	}
	else if (nChar == theApp.m_avkControls[1])
	{
		if (theApp.m_pBumperDoc->m_humanPlayer.m_listTempWallPods.GetCount() != 0)
		{
			((CGameObj*)theApp.m_pBumperDoc->m_humanPlayer.m_listTempWallPods.GetTail())->
				UsePod((DWORD)&theApp.m_pBumperDoc->m_humanPlayer);
			m_ctrTempWalls.m_bDigitsDirty = 1;
			m_ctrTempWalls.m_dwDisplayValue =
				theApp.m_pBumperDoc->m_humanPlayer.m_listTempWallPods.GetCount();
		}
	}
	else if (nChar == theApp.m_avkControls[0])
	{
		if (theApp.m_pBumperDoc->m_humanPlayer.m_listSmokeBombPods.GetCount() != 0)
		{
			((CGameObj*)theApp.m_pBumperDoc->m_humanPlayer.m_listSmokeBombPods.GetTail())->
				UsePod((DWORD)&theApp.m_pBumperDoc->m_humanPlayer);
			m_ctrSmokeBombs.m_bDigitsDirty = 1;
			m_ctrSmokeBombs.m_dwDisplayValue =
				theApp.m_pBumperDoc->m_humanPlayer.m_listSmokeBombPods.GetCount();
		}
	}
	else if (nChar == theApp.m_avkControls[2])
	{
		// The jump pod additionally needs the player to be on the ground
		// and not already jumping.
		if (theApp.m_pBumperDoc->m_humanPlayer.m_listJumpPods.GetCount() != 0 &&
			(int)theApp.m_pBumperDoc->m_humanPlayer.m_dwHoldTimer <= 0 &&
			theApp.m_pBumperDoc->m_humanPlayer.m_fxTargetHeight ==
				theApp.m_pBumperDoc->m_humanPlayer.m_fxHeight &&
			theApp.m_pBumperDoc->m_humanPlayer.m_bAirborne == 0)
		{
			((CGameObj*)theApp.m_pBumperDoc->m_humanPlayer.m_listJumpPods.GetTail())->
				UsePod((DWORD)&theApp.m_pBumperDoc->m_humanPlayer);
			m_ctrJumpPods.m_bDigitsDirty = 1;
			m_ctrJumpPods.m_dwDisplayValue =
				theApp.m_pBumperDoc->m_humanPlayer.m_listJumpPods.GetCount();
		}
	}
	Default();
}

// Entries 21/22: ID_DBG_INVINCIBLE toggles the human player's +0x144 flag and
// mirrors it into +0x140.

// hover.exe:0x428A10
// FUNCTION: HOVER 0x428a10
void CBumperView::OnDbgInvincible()
{
	if (m_dwUnk0038 != 0)
		return;
	theApp.m_pBumperDoc->m_humanPlayer.m_bGodMode =
		(theApp.m_pBumperDoc->m_humanPlayer.m_bGodMode == 0);
	theApp.m_pBumperDoc->m_humanPlayer.m_bInvincible =
		theApp.m_pBumperDoc->m_humanPlayer.m_bGodMode;
}

// hover.exe:0x428A40
// FUNCTION: HOVER 0x428a40
void CBumperView::OnUpdateDbgInvincible(CCmdUI* pCmdUI)
{
	DWORD dwState;

	dwState = theApp.m_dwGameState & 0x7FFFFFFF;
	pCmdUI->SetCheck(theApp.m_pBumperDoc->m_humanPlayer.m_bGodMode);
	pCmdUI->Enable(dwState != 0 && dwState != 1);
}

// Entry 23: ID_DBG_RESTOCK_FLAGS restocks the board - one new flag per flag the
// human player is carrying plus m_dwRobotFlags robot flags are dropped back on
// random pads, every player's carried-object list is emptied and the two
// dashboard flag sprites are reset.

// hover.exe:0x428A90
// FUNCTION: HOVER 0x428a90
void CBumperView::OnDbgRestockFlags()
{
	CBumperView* pView;
	POSITION pos;
	CObject* pObj;
	int nFlags;

	if (m_dwUnk0038 != 0)
		return;

	::EnterCriticalSection(&theApp.m_csOffscreen);

	nFlags = theApp.m_pBumperDoc->m_humanPlayer.m_listFlags.GetCount();
	while (nFlags != 0)
	{
		theApp.m_pBumperDoc->PlaceObject(new CFlag(1), 0);
		nFlags--;
	}

	nFlags = theApp.m_pBumperDoc->m_dwRobotFlags;
	while (nFlags != 0)
	{
		theApp.m_pBumperDoc->PlaceObject(new CFlag(0), 0);
		nFlags--;
	}
	theApp.m_pBumperDoc->m_dwRobotFlags = 0;

	pos = theApp.m_pBumperDoc->m_listActiveObjs.GetHeadPosition();
	while (pos != NULL)
	{
		pObj = theApp.m_pBumperDoc->m_listActiveObjs.GetNext(pos);
		if (pObj->IsKindOf(RUNTIME_CLASS(CPlayer)))
			((CPlayer*)pObj)->ClearPods(0);
	}

	// Hover_GetView() is expanded inline here in the original.
	pView = (theApp.m_pMainWnd == NULL) ? NULL :
		(CBumperView*)((CFrameWnd*)theApp.m_pMainWnd)->GetActiveView();
	pView->m_flagsPlayer.m_bFlagsDirty = 1;
	pView->m_flagsPlayer.m_dwFilled = pView->m_flagsPlayer.m_dwSlots;
	if ((int)pView->m_flagsPlayer.m_dwFilled > 0)
		pView->m_flagsPlayer.m_dwFilled = 0;

	pView = (theApp.m_pMainWnd == NULL) ? NULL :
		(CBumperView*)((CFrameWnd*)theApp.m_pMainWnd)->GetActiveView();
	pView->m_flagsRobot.m_bFlagsDirty = 1;
	pView->m_flagsRobot.m_dwFilled = pView->m_flagsRobot.m_dwSlots;
	if ((int)pView->m_flagsRobot.m_dwFilled > 0)
		pView->m_flagsRobot.m_dwFilled = 0;

	::LeaveCriticalSection(&theApp.m_csOffscreen);
}

// Entries 24..31: eight identical update handlers (one per debug pod
// command) that only enable the command while a game is actually running.
// The original emits each of them separately, 0x30 bytes apart.

// hover.exe:0x428C00
// FUNCTION: HOVER 0x428c00
void CBumperView::OnUpdateDbgRestockFlags(CCmdUI* pCmdUI)
{
	DWORD dwState;

	dwState = theApp.m_dwGameState & 0x7FFFFFFF;
	pCmdUI->Enable(dwState != 0 && dwState != 1);
}

// hover.exe:0x428C30
// FUNCTION: HOVER 0x428c30
void CBumperView::OnUpdateDbgAddSmokePod(CCmdUI* pCmdUI)
{
	DWORD dwState;

	dwState = theApp.m_dwGameState & 0x7FFFFFFF;
	pCmdUI->Enable(dwState != 0 && dwState != 1);
}

// hover.exe:0x428C60
// FUNCTION: HOVER 0x428c60
void CBumperView::OnUpdateDbgAddWallPod(CCmdUI* pCmdUI)
{
	DWORD dwState;

	dwState = theApp.m_dwGameState & 0x7FFFFFFF;
	pCmdUI->Enable(dwState != 0 && dwState != 1);
}

// hover.exe:0x428C90
// FUNCTION: HOVER 0x428c90
void CBumperView::OnUpdateDbgAddJumpPod(CCmdUI* pCmdUI)
{
	DWORD dwState;

	dwState = theApp.m_dwGameState & 0x7FFFFFFF;
	pCmdUI->Enable(dwState != 0 && dwState != 1);
}

// hover.exe:0x428CC0
// FUNCTION: HOVER 0x428cc0
void CBumperView::OnUpdateDbgApplyInvincible(CCmdUI* pCmdUI)
{
	DWORD dwState;

	dwState = theApp.m_dwGameState & 0x7FFFFFFF;
	pCmdUI->Enable(dwState != 0 && dwState != 1);
}

// hover.exe:0x428CF0
// FUNCTION: HOVER 0x428cf0
void CBumperView::OnUpdateDbgApplyMapEraser(CCmdUI* pCmdUI)
{
	DWORD dwState;

	dwState = theApp.m_dwGameState & 0x7FFFFFFF;
	pCmdUI->Enable(dwState != 0 && dwState != 1);
}

// hover.exe:0x428D20
// FUNCTION: HOVER 0x428d20
void CBumperView::OnUpdateDbgApplySpeed(CCmdUI* pCmdUI)
{
	DWORD dwState;

	dwState = theApp.m_dwGameState & 0x7FFFFFFF;
	pCmdUI->Enable(dwState != 0 && dwState != 1);
}

// hover.exe:0x428D50
// FUNCTION: HOVER 0x428d50
void CBumperView::OnUpdateDbgApplySlow(CCmdUI* pCmdUI)
{
	DWORD dwState;

	dwState = theApp.m_dwGameState & 0x7FFFFFFF;
	pCmdUI->Enable(dwState != 0 && dwState != 1);
}

// Entries 32/33: ID_DBG_PIXEL_DOUBLE toggles the 2x pixel-doubled presentation
// (m_bDoubleSize); it is only available in a 640-pixel wide client area.

// hover.exe:0x429230
// FUNCTION: HOVER 0x429230
void CBumperView::OnDbgPixelDouble()
{
	HOVER_INLINE_SUB_40EE90_1();

	m_bDoubleSize = (m_bDoubleSize == 0);
	RequestResize(m_dwRequestedWidth, m_dwRequestedHeight, 1);
	RequestRenderFrame(1);
}

// hover.exe:0x4292F0
// FUNCTION: HOVER 0x4292f0
void CBumperView::OnUpdateDbgPixelDouble(CCmdUI* pCmdUI)
{
	RECT rect;

	::GetClientRect(m_hWnd, &rect);
	pCmdUI->Enable(rect.right - rect.left - 640 == 0);
	pCmdUI->SetCheck(m_bDoubleSize);
}

// Entry 34: WM_CREATE.

// hover.exe:0x420DF0
// FUNCTION: HOVER 0x420df0
int CBumperView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	return (CView::OnCreate(lpCreateStruct) == -1) ? -1 : 0;
}

// Entries 35/36: ID_DBG_ALT_PRESENT toggles the alternate present path used by
// CBumperView::PresentFrame.

// hover.exe:0x429340
// FUNCTION: HOVER 0x429340
void CBumperView::OnDbgAltPresent()
{
	g_bFullFramePresent = (g_bFullFramePresent == 0);
}

// hover.exe:0x429360
// FUNCTION: HOVER 0x429360
void CBumperView::OnUpdateDbgAltPresent(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(g_bFullFramePresent);
}

// Entry 40: WM_CHAR. Recognizes the five-keystroke cheat sequence Ctrl+I,
// Ctrl+B, Ctrl+M, Ctrl+A, Ctrl+B (character codes 9, 2, 13, 1, 2), which
// sets m_dwUnk0038 and forces the game into state 3. m_dwKeySeqPos is the
// position in the sequence; any wrong key restarts it.

// hover.exe:0x429380
// FUNCTION: HOVER 0x429380
void CBumperView::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if (theApp.m_pBumperDoc->m_bAttractMode == 0)
		return;
	if (theApp.m_dwGameState == 3)
		return;

	switch (m_dwKeySeqPos)
	{
	case 0:
		if (nChar == 9)
		{
			m_dwKeySeqPos = m_dwKeySeqPos + 1;
			break;
		}
		m_dwUnk0038 = 0;
		m_dwKeySeqPos = 0;
		break;

	case 1:
		if (nChar == 2)
		{
			m_dwKeySeqPos = m_dwKeySeqPos + 1;
			break;
		}
		m_dwUnk0038 = 0;
		m_dwKeySeqPos = 0;
		break;

	case 2:
		if (nChar == 13)
		{
			m_dwKeySeqPos = m_dwKeySeqPos + 1;
			break;
		}
		m_dwUnk0038 = 0;
		m_dwKeySeqPos = 0;
		break;

	case 3:
		if (nChar == 1)
		{
			m_dwKeySeqPos = m_dwKeySeqPos + 1;
			break;
		}
		m_dwUnk0038 = 0;
		m_dwKeySeqPos = 0;
		break;

	case 4:
		if (nChar == 2)
		{
			m_dwUnk0038 = 1;
			m_dwKeySeqPos = 0;
			theApp.m_dwGameState = 3;
			m_dwStateCountdown = 60;
			StartGameTimer();
			break;
		}
		m_dwUnk0038 = 0;
		m_dwKeySeqPos = 0;
		break;

	default:
		m_dwUnk0038 = 0;
		m_dwKeySeqPos = 0;
		break;
	}
	Default();
}

// The 50 ms multimedia timer callback installed by CBumperView::StartGameTimer.
// g_bTickPending is the "a frame tick is already in flight" latch: it is
// raised here and lowered again by CBumperView::OnUserInput, so a tick that
// arrives while the previous one is still being processed is dropped. The
// active view is looked up inline (the same expression as Hover_GetView)
// and told to sample its input with WM_USER.

// hover.exe:0x408AA0
// FUNCTION: HOVER 0x408aa0
void CALLBACK Hover_GameTimerProc(UINT uID, UINT uMsg, DWORD dwUser,
	DWORD dw1, DWORD dw2)
{
	CBumperView* pView;

	if (g_bTickPending != 0)
		return;
	g_bTickPending = 1;
	pView = (*(CFrameWnd* volatile*)&theApp.m_pMainWnd != NULL)
		? (CBumperView*)((CFrameWnd*)theApp.m_pMainWnd)->GetActiveView()
		: NULL;
	if (pView != NULL)
		::PostMessage(pView->m_hWnd, WM_USER, 0, 0);
}

// Entry 41: WM_USER+0, posted by the multimedia timer callback
// (hover.exe:0x408AA0). Samples the joystick and the four direction keys
// into the input state at m_bInputLeft..m_bInputDown and runs the two frame
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

	::EnterCriticalSection(&theApp.m_csInput);
	m_bInputLeft = 0;
	m_bInputRight = 0;
	m_bInputUp = 0;
	m_bInputDown = 0;

	pfnJoyGetPosEx = (MMRESULT (WINAPI*)(UINT, LPJOYINFOEX))g_pfnJoyGetPosEx;
	if (theApp.m_avkControls[11] != 0 && pfnJoyGetPosEx != NULL)
	{
		// The stick is treated as pushed once it leaves the middle half of
		// its calibrated range.
		theApp.m_joyInfo.dwSize = sizeof(JOYINFOEX);
		theApp.m_joyInfo.dwFlags = JOY_RETURNX | JOY_RETURNY |
			JOY_RETURNBUTTONS | JOY_RETURNCENTERED;
		pfnJoyGetPosEx(0, &theApp.m_joyInfo);

		dwXMid = (theApp.m_joyCaps.wXmin + theApp.m_joyCaps.wXmax) >> 1;
		dwYMid = (theApp.m_joyCaps.wYmin + theApp.m_joyCaps.wYmax) >> 1;
		m_bInputLeft = (theApp.m_joyInfo.dwXpos <
			dwXMid - ((dwXMid - theApp.m_joyCaps.wXmin) >> 2));
		m_bInputRight = (dwXMid + ((theApp.m_joyCaps.wXmax - dwXMid) >> 2) <
			theApp.m_joyInfo.dwXpos);
		m_bInputUp = (theApp.m_joyInfo.dwYpos <
			dwYMid - ((dwYMid - theApp.m_joyCaps.wYmin) >> 2));
		m_bInputDown = (dwYMid + ((theApp.m_joyCaps.wYmax - dwYMid) >> 2) <
			theApp.m_joyInfo.dwYpos);
	}

	// The four diagonal keys contribute to two directions each.
	bUpLeft = (::GetAsyncKeyState(theApp.m_avkControls[7]) < 0);
	bUpRight = (::GetAsyncKeyState(theApp.m_avkControls[8]) < 0);
	bDownLeft = (::GetAsyncKeyState(theApp.m_avkControls[9]) < 0);
	bDownRight = (::GetAsyncKeyState(theApp.m_avkControls[10]) < 0);

	if (bUpLeft || bDownLeft || ::GetAsyncKeyState(theApp.m_avkControls[5]) < 0)
		m_bInputLeft |= 1;
	if (bUpRight || bDownRight || ::GetAsyncKeyState(theApp.m_avkControls[6]) < 0)
		m_bInputRight |= 1;
	if (bUpLeft || bUpRight || ::GetAsyncKeyState(theApp.m_avkControls[3]) < 0)
		m_bInputUp |= 1;
	if (bDownLeft || bDownRight || ::GetAsyncKeyState(theApp.m_avkControls[4]) < 0)
		m_bInputDown |= 1;

	::LeaveCriticalSection(&theApp.m_csInput);
	GameStep();
	UpdateStatusBar();
	g_bTickPending = 0;
	return 0;
}

// Entry 42: WM_USER+1 - the game is over, run the high-score flow.

// hover.exe:0x427D20
// FUNCTION: HOVER 0x427d20
LRESULT CBumperView::OnUserGameOver(WPARAM wParam, LPARAM lParam)
{
	theApp.OnGameOver();
	return 0;
}

// Entry 43: WM_USER+2 - the level was completed, advance to the next one.

// hover.exe:0x427D30
// FUNCTION: HOVER 0x427d30
LRESULT CBumperView::OnUserNextLevel(WPARAM wParam, LPARAM lParam)
{
	theApp.OnLevelComplete();
	return 0;
}

// Entry 44: ID_ESCAPE (the Esc accelerator) suspends the game and minimizes
// the main window.

// hover.exe:0x428890
// FUNCTION: HOVER 0x428890
void CBumperView::OnEscape()
{
	HOVER_INLINE_SUB_40EE90_1();

	if (theApp.m_bFullScreen != 0)
		theApp.LeaveFullScreen();
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
// [index*0x338 + g_aZones + field], which only comes out of an array
// subscript on the constant base.

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
	nZone = (int)g_dwCurZone;

	rcWaypoints.left = g_aZones[nZone].m_ptPlayerFlags.x;
	rcWaypoints.top = g_aZones[nZone].m_ptPlayerFlags.y;
	rcWaypoints.right = g_aZones[nZone].m_ptPlayerFlags.x + 6 * g_aZones[nZone].m_sizeFlagIcon.cx;
	rcWaypoints.bottom = g_aZones[nZone].m_ptPlayerFlags.y + 2 * g_aZones[nZone].m_sizeFlagIcon.cy;

	rcMap.left = g_aZones[nZone].m_rcMirror.left;
	rcMap.top = g_aZones[nZone].m_rcTopDashDst.top;
	rcMap.right = g_aZones[nZone].m_rcMirror.right;
	rcMap.bottom = g_aZones[nZone].m_rcTopDashDst.bottom;

	rcPods.left = g_aZones[nZone].m_ptScore.x;
	rcPods.top = g_aZones[nZone].m_ptScore.y;
	{
		// The pod strip's extents go through two locals of their own: the
		// original spills the x7 multiplicand to a slot
		// (mov [t],eax / shl eax,3 / sub eax,[t]) and parks the bottom edge
		// in a second slot until after the right edge is stored.
		int cyPods = (int)g_aZones[nZone].m_sizeScoreDigit.cy
			+ g_aZones[nZone].m_ptScore.y;
		int cxPods = (int)g_aZones[nZone].m_sizeScoreDigit.cx;

		rcPods.right = g_aZones[nZone].m_ptScore.x + 7 * cxPods;
		rcPods.bottom = cyPods;
	}

	rcScore.left = g_aZones[nZone].m_rcTopDashDst.left;
	rcScore.top = g_aZones[nZone].m_rcTopDashDst.bottom;
	rcScore.right = g_aZones[nZone].m_rcTopDashDst.right;
	rcScore.bottom = g_aZones[nZone].m_rcBottomDashDst.top;

	rcGauge1.left = g_aZones[nZone].m_rcBottomDashDst.left;
	rcGauge1.top = g_aZones[nZone].m_rcBottomDashDst.top;
	rcGauge1.right = g_aZones[nZone].m_rcMap.left;
	rcGauge1.bottom = g_aZones[nZone].m_rcBottomDashDst.bottom;

	rcGauge2.left = g_aZones[nZone].m_rcMap.left;
	rcGauge2.top = g_aZones[nZone].m_rcBottomDashDst.top;
	rcGauge2.right = g_aZones[nZone].m_rcMap.right;
	rcGauge2.bottom = g_aZones[nZone].m_rcBottomDashDst.bottom;

	rcGauge3.left = g_aZones[nZone].m_rcMap.right;
	rcGauge3.top = g_aZones[nZone].m_rcBottomDashDst.top;
	rcGauge3.right = g_aZones[nZone].m_rcBottomDashDst.right;
	rcGauge3.bottom = g_aZones[nZone].m_rcBottomDashDst.bottom;

	if (rcWaypoints.PtInRect(point))
		::WinHelp(GetSafeHwnd(), "hover.hlp", HELP_CONTEXTPOPUP, 0xA3C);
	else if (rcMap.PtInRect(point))
	{
		if (theApp.m_bRearView != 0)
			::WinHelp(GetSafeHwnd(), "hover.hlp", HELP_CONTEXTPOPUP, 0xA32);
	}
	else if (rcPods.PtInRect(point))
		::WinHelp(GetSafeHwnd(), "hover.hlp", HELP_CONTEXTPOPUP, 0xA28);
	else if (rcScore.PtInRect(point))
		::WinHelp(GetSafeHwnd(), "hover.hlp", HELP_CONTEXTPOPUP, 0xA46);
	else if (rcGauge1.PtInRect(point))
		::WinHelp(GetSafeHwnd(), "hover.hlp", HELP_CONTEXTPOPUP, 0xA50);
	else if (rcGauge2.PtInRect(point))
		::WinHelp(GetSafeHwnd(), "hover.hlp", HELP_CONTEXTPOPUP, 0xA5A);
	else if (rcGauge3.PtInRect(point))
		::WinHelp(GetSafeHwnd(), "hover.hlp", HELP_CONTEXTPOPUP, 0xA64);
	return 1;
}

// Entry 48: ID_CMD_34034 is mapped but does nothing in the shipped build.

// hover.exe:0x4294F0
// FUNCTION: HOVER 0x4294f0
void CBumperView::OnCmd34034()
{
}
