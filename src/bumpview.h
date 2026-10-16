// bumpview.h : interface of the CBumperView class
//
// Skeleton reconstructed from hover.exe:
//   CRuntimeClass "CBumperView" at 0x4C4CB0 (size 0x55C, DYNCREATE)
//   vtable at 0x4BB6D0
//
// Note on member offsets: the comments give each member's offset within
// CBumperView in hover.exe. The original was built with an earlier MFC 4.x
// whose CView base class is a different size than the MFC 4.2 used for
// this build, so the offsets in our build will not match; the comments
// record the ground truth from the executable.

#ifndef __BUMPVIEW_H__
#define __BUMPVIEW_H__

class CBumperView;
class CBumperDoc;
class CRenderContext;
class CGameObj;
class CSpanRasterizer;
class CMerlinLine;
class CMerlinStatic;
class CMerlinDynamic;
class CMerlinBSP;
class CMerlinWorld;

/////////////////////////////////////////////////////////////////////////////
// Helper classes embedded in CBumperView. None of these has a vtable in
// the original; their constructors are decompiled in bumpview.cpp.

// CVelocityIndicator: offscreen render target (holds a CDC and a CBitmap).
// ctor hover.exe:0x423030, dtor hover.exe:0x420E60 (trivial), sizeof = 0x68.
class CVelocityIndicator
{
public:
	CVelocityIndicator();			// hover.exe:0x423030
	~CVelocityIndicator();			// hover.exe:0x420E60

	// hover.exe:0x405A10 recompute the m_dwNeedleDX/m_dwNeedleDY camera basis
	// offsets from the view yaw (pView->m_dwCamAngle) and the human
	// player's m_fxVelX/m_fxVelY
	void UpdateNeedle(CBumperView* pView, CBumperDoc* pDoc);
	// hover.exe:0x401E80 blit the held bitmap into pDC, framing the dirty
	// rect
	void DrawNeedle(CDC* pDC, int bUnk);

	// hover.exe:0x4140E0 compute the fixed-point x/y scale factors
	// (m_fxScaleX/m_fxScaleY) from the bitmap dimensions and player/global
	// constants; helper called by CreateTile.
	void ComputeScale();
	// hover.exe:0x411D80 create the 8-bpp DIB section render target and
	// select it into the internal DC. Takes the destination CDC, the
	// dashboard DIB (header first, so the pixels start biSize + 0x400
	// bytes in) and the source rectangle (by value).
	void CreateTile(CDC* pInternalDC, BYTE* pDib, RECT rc);

	// The first 0x28 bytes are two RECTs (+0x00 source, +0x10 dirty), a
	// width scratch at +0x20 and a height scratch at +0x24, used by
	// CreateTile/ComputeScale.
	RECT m_rcSource;		// +0x00
	RECT m_rcDirty;			// +0x10
	DWORD m_dwTileWidth;		// +0x20 width
	DWORD m_dwTileHeight;		// +0x24 height
	CGdiObject* m_pOldBitmap;	// +0x28 = NULL (old GDI object selected in m_dc)
	CBitmap m_bitmap;		// +0x2C
	CDC m_dc;				// +0x34
	// 16.16 x/y scale factors (ComputeScale), whole or by halves
	union					// +0x44 = 0
	{
		DWORD m_fxScaleX;
		struct { WORD m_wScaleXFrac, m_wScaleX; };
	};
	union					// +0x48 = 0
	{
		DWORD m_fxScaleY;
		struct { WORD m_wScaleYFrac, m_wScaleY; };
	};
	CDC* m_pTargetDC;			// +0x4C internal destination CDC used by DrawNeedle
	DWORD m_dwCenterX;		// +0x50 = 0
	DWORD m_dwCenterY;		// +0x54 = 0
	BYTE* m_pBits;		// +0x58 = 0
	DWORD m_dwNeedleDX;		// +0x5C camera basis offset x (UpdateNeedle)
	DWORD m_dwNeedleDY;		// +0x60 render height / camera basis offset y
	CPen* m_pPen;			// +0x64 pen selected in to draw the update-area
							//   indicator in DrawNeedle
};							// sizeof = 0x68

// CMapRenderer: the software renderer's frame buffer / span-draw context
// embedded in CBumperView at +0x150. Holds the pixel bits, clip rect, row
// pitch, current color, the camera/projection state and the per-class draw
// colors; the frame step projects every visible game object into it.
// ctor hover.exe:0x422F50, sizeof = 0xCC (no destructor).
class CMapRenderer
{
public:
	CMapRenderer();			// hover.exe:0x422F50

	// hover.exe:0x407160 per-frame scene pass: set the viewport globals
	// from the human player's camera, then project every object in the
	// doc's two object lists (and the tracked array when theApp.m_bShowBeacons)
	void ProjectScene(CBumperDoc* pDoc);
	// hover.exe:0x404BB0 visibility-test one object against the viewport
	// globals and project its points with ProjectPoint
	void ProjectObject(CBumperDoc* pDoc, CGameObj* pObj, int bUnk);
	// hover.exe:0x4055B0 project the world point (lX,lY) to screen,
	// rotating by lAngle when theApp.m_bRotateMap is set
	void ProjectPoint(POINT* pptOut, long lX, long lY, long lCX, long lCY,
		long lV1, long lV2, long lAngle);
	// hover.exe:0x4053A0 draw every projected object (walls, pods, player
	// triangles) and the tracked-object markers into the pixel bits
	void DrawScene(CBumperView* pView, CBumperDoc* pDoc);
	// hover.exe:0x401C00 draw one projected object (dispatch on class)
	void DrawObject(CGameObj* pObj, int bUnk);
	// hover.exe:0x401980 draw a clipped line into the pixel bits
	void DrawLine(long lX1, long lY1, long lX2, long lY2);
	// hover.exe:0x403910 draw a small cross marker into the pixel bits
	void DrawMarker(long lX, long lY, long lUnk);
	// hover.exe:0x401520 clip a line endpoint pair to the bitmap (stub)
	int ClipEndpoint(long* plA, long* plB);
	// hover.exe:0x42D3E0 blit the rendered rect into pDC (StretchBlt 2x
	// when the view's m_bDoubleSize, BitBlt otherwise)
	void PresentMap(CDC* pDC);
	// hover.exe:0x40FEE0 set up the renderer viewport/scaling state from
	// the supplied rectangle and doc pointer.
	void SetViewport(CDC* pDC, CBumperView* pView, CBumperDoc* pDoc,
		RECT rc);

	RECT m_rcClip;			// +0x00 clip rectangle (right/bottom inclusive)
	BYTE* m_pBits;			// +0x10 pixel bits / render-target bits handle
	DWORD m_dwBitsWidth;		// +0x14 render target width (bytes per row basis)
	DWORD m_dwBitsHeight;		// +0x18 bitmap height (y-flip base)
	BYTE m_bColor;			// +0x1C current draw color
	char m_pad001D[0x3];	// +0x1D
	DWORD m_dwColor4;		// +0x20 current color, dword-expanded
	DWORD m_dwPitch;		// +0x24 row pitch
	DWORD m_dwPitchDwords;		// +0x28 row pitch in dwords (rounded)
	DWORD m_dwLineDX;		// +0x2C line-draw |dx| (DrawLine)
	DWORD m_dwLineDY;		// +0x30 line-draw |dy|
	// +0x34..+0x58: derived by SetClipRect and not read by any recovered
	// code (the left/right edge addresses are pointers kept in DWORDs).
	DWORD m_bLeftAligned;	// +0x34 left edge on a dword boundary
	DWORD m_bRightAligned;	// +0x38 right edge + 1 on a dword boundary
	DWORD m_dwTopOffsetDw;	// +0x3C flipped top row * pitch in dwords
	DWORD m_dwTopOffset;	// +0x40 flipped top row * pitch
	DWORD m_dwFlipBottom;	// +0x44 bottom row counted from the bottom
	DWORD m_dwFlipTop;		// +0x48 top row counted from the bottom
	DWORD m_dwLeftEdge;		// +0x4C bits + left
	DWORD m_dwRightEdge4;	// +0x50 bits + right - 4
	DWORD m_dwLeftAligned;	// +0x54 left edge rounded to a dword
	DWORD m_dwRightAligned;	// +0x58 right edge rounded down to a dword
	RECT m_rcView;			// +0x5C the map's rectangle in the view; its
							//   left/top is the projection origin
	// 16.16 view geometry (SetViewport), whole or by halves
	union					// +0x6C = 0 view height
	{
		DWORD m_fxHeight;
		struct { WORD m_wHeightFrac, m_wHeight; };
	};
	union					// +0x70 = 0 view width
	{
		DWORD m_fxWidth;
		struct { WORD m_wWidthFrac, m_wWidth; };
	};
	union					// +0x74 = 0 view height / 2
	{
		DWORD m_fxHalfHeight;
		struct { WORD m_wHalfHeightFrac, m_wHalfHeight; };
	};
	union					// +0x78 = 0 view width / 2
	{
		DWORD m_fxHalfWidth;
		struct { WORD m_wHalfWidthFrac, m_wHalfWidth; };
	};
	union					// +0x7C = 0 projection base
	{
		DWORD m_fxProjBase;
		struct { WORD m_wProjBaseFrac, m_wProjBase; };
	};
	union					// +0x80 = 0 world units per pixel
	{
		DWORD m_fxScale;
		struct { WORD m_wScaleFrac, m_wScale; };
	};
	CDC* m_pMapDC;		// +0x84 owning DC (PresentMap blit source)
	WORD m_wViewRadius;		// +0x88 half the viewport extent (ProjectScene)
	char m_pad008A[0x2];	// +0x8A
	DWORD m_dwColorHuman;		// +0x8C color: human player
	DWORD m_dwColorRobot;		// +0x90 color: robot
	DWORD m_dwColorRobotAlt;		// +0x94 color: robot, alternate
	DWORD m_dwColorYellow;		// +0x98 color: default (walls, pods, markers)
	DWORD m_dwColorWhite;		// +0x9C color: wall/pod alternate
	DWORD m_dwColorBlack;		// +0xA0 color: wall, flagged (alternate)
	DWORD m_dwColorLightGray;		// +0xA4 color: waypoint/track marker
	DWORD m_dwColorGray;		// +0xA8 color: wall, flagged
	DWORD m_dwBlinkPhase;		// +0xAC frame counter (mod 8)
	char m_pad00B0[0x18];	// +0xB0
	DWORD m_dwZoom;		// +0xC8 = 0x40000

private:
	// hover.exe:0x40FB00 helper: store the render-target bits, width and
	// height and compute the dword row pitch.
	void SetBits(BYTE* pBits, DWORD dwWidth, DWORD dwHeight);
	// hover.exe:0x40FBB0 helper: set the clip rectangle and derived span
	// rasterizer pointers from the rectangle (by value).
	void SetClipRect(RECT rc);
};							// sizeof = 0xCC

// CDigitDisplay: ctor hover.exe:0x422BD0, dtor hover.exe:0x422C20
// (operator delete on m_pStripBits), sizeof = 0x28.
class CDigitDisplay
{
public:
	CDigitDisplay();			// hover.exe:0x422BD0
	~CDigitDisplay();			// hover.exe:0x422C20

	// hover.exe:0x40FCE0 draw the number into the render target
	void DrawDigits(BYTE* pDestBits, DWORD dwX, DWORD dwY);
	// hover.exe:0x411A00 dashboard element setup
	void LoadDigits(int nFlag, const RECT* pRect, BYTE* pBits, DWORD dwA,
		DWORD dwB);
	// hover.exe:0x411920 set the destination point at +0x20
	void SetPosition(const POINT* pPoint);

	DWORD m_bDigitsDirty;		// +0x00 = 0; used as an active flag by
							// CBumperView::PresentFrame
	DWORD m_dwDigits;		// +0x04 = 0; number of digits to draw
	DWORD m_dwDisplayValue;		// +0x08 = 0; value to display
	DWORD m_dwStripWidth;		// +0x0C = 0; source bitmap width
	DWORD m_dwStripHeight;		// +0x10 = 0; source bitmap height
	DWORD m_dwDigitWidth;		// +0x14 = 0; digit tile width
	DWORD m_dwDigitHeight;		// +0x18 = 0; digit tile height
	BYTE* m_pStripBits;			// +0x1C = 0 (freed by the destructor)
	DWORD m_dwDestX;		// +0x20 = 0; destination x
	DWORD m_dwDestY;		// +0x24 = 0; destination y
};							// sizeof = 0x28

// CSegmentGauge: ctor hover.exe:0x422C80, sizeof = 0x30 (no destructor).
// CGaugeFraction: the 16.16 fixed-point value the dashboard gauges are fed
// with (integer part in the high word, fraction in the low word; the
// double constructor multiplies by 65536.0 and truncates). Passed to
// CSegmentGauge::SetFraction by value; both classes' gauge-fill methods are
// defined in mazeobj.cpp, next to the pod classes that build the values.

class CGaugeFraction
{
public:
	CGaugeFraction() { }
	inline CGaugeFraction(WORD wInt, WORD wFrac);	// hover.exe:0x42AE90
	CGaugeFraction(double dbl);				// hover.exe:0x42A9D0

	WORD m_wFrac;			// +0x00 fractional part
	WORD m_wInt;			// +0x02 integer part
};							// sizeof = 4

class CSegmentGauge
{
public:
	CSegmentGauge();			// hover.exe:0x422C80

	// hover.exe:0x42A9F0 light the segments corresponding to the 0..1
	// fixed-point fraction (m_dwLevel = clamp(fix * m_dwSegments, 0,
	// m_dwSegments)); defined in mazeobj.cpp
	void SetFraction(CGaugeFraction fix);

	// hover.exe:0x401AD0 draw the gauge into pDC
	void DrawGauge(CDC* pDC, int bUnk);
	// hover.exe:0x411D60 gauge setup helper (stub; part of the dashboard
	// subsystem, not the DIB buffer setup).
	void SetStyle(DWORD dwSegments, CBrush* pBrushFilled, CPen* pPen,
		CBrush* pBrushEmpty);
	// hover.exe:0x411D10 place the gauge in pRect: lay the segments along
	// its longer side and return the size of one segment
	int SetLayout(const RECT* pRect);

	RECT m_rc;				// +0x00 = {0, 0, 10, 10}; the gauge's rectangle
	CBrush* m_pBrushFilled;	// +0x10 = 0; segments below the level
	CBrush* m_pBrushEmpty;	// +0x14 = 0; segments from the level up
	CPen* m_pPen;		// +0x18 = 0; outline CPen* (cast from DWORD)
	DWORD m_dwSegments;		// +0x1C = 1; number of segments
	DWORD m_bHorizontal;		// +0x20 orientation (SetLayout): nonzero
							//   horizontal, zero vertical
	DWORD m_dwLevel;		// +0x24 = 0; first active segment index
	DWORD m_dwLastLevel;		// +0x28; previous m_dwLevel, updated each draw
	DWORD m_dwSegmentSize;		// +0x2C = 0; segment size (width or height)
};							// sizeof = 0x30

// CFlagStrip: ctor hover.exe:0x422C30, dtor hover.exe:0x422C60
// (operator delete on m_pFullBits and m_pEmptyBits), sizeof = 0x50.
class CFlagStrip
{
public:
	CFlagStrip();			// hover.exe:0x422C30
	~CFlagStrip();			// hover.exe:0x422C60

	// hover.exe:0x40E9A0 draw the element into the render target (stub)
	void DrawFlags(BYTE* pDestBits, DWORD dwX, DWORD dwY);
	// hover.exe:0x411BB0 dashboard sprite setup
	void LoadIcons(DWORD dwUnk, DWORD dwLeft1, DWORD dwTop1,
		DWORD dwRight1, DWORD dwBottom1, DWORD dwLeft2, DWORD dwTop2,
		DWORD dwRight2, DWORD dwBottom2, BYTE* pBits, DWORD dwWidth,
		DWORD dwHeight);
	// hover.exe:0x411CF0 set the destination position at +0x04
	void SetPosition(const POINT* pPoint);

	DWORD m_bFlagsDirty;		// +0x00 = 1
	DWORD m_dwDestX;		// +0x04 = 0 (destination x base / pointer)
	DWORD m_dwDestY;		// +0x08 = 0 (destination y)
	void* m_pUnk0C;			// +0x0C = 0
	void* m_pUnk10;			// +0x10 = 0
	void* m_pFullBits;			// +0x14 = 0 (source bits A, freed by destructor)
	void* m_pEmptyBits;			// +0x18 = 0 (source bits B, freed by destructor)
	RECT m_rcFull;		// +0x1C
	RECT m_rcEmpty;		// +0x2C
	DWORD m_dwSlots;		// +0x3C = 1 (sprite count)
	DWORD m_dwUnk40;		// +0x40 = 0
	DWORD m_dwFilled;		// +0x44 = 0 (source selection threshold)
	DWORD m_dwUnk48;		// +0x48 = 0
	DWORD m_dwUnk4C;		// +0x4C = 0
};							// sizeof = 0x50

/////////////////////////////////////////////////////////////////////////////
// CBumperView

class CBumperView : public CView
{
	DECLARE_DYNCREATE(CBumperView)

public:
	// hover.exe:0x427CF0 enter game state dwState (defined in mazeobj.cpp,
	// next to the flag capture that triggers the win state)
	void SetGameState(DWORD dwState);
	CBumperView();			// hover.exe:0x41E730
	virtual ~CBumperView();	// hover.exe:0x41E3F0

// Overrides
	// hover.exe:0x41F180 create the view's window after its GDI objects
	virtual BOOL Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName,
		DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID,
		CCreateContext* pContext = NULL);
	// hover.exe:0x40E310 re-realize the game palette when the view is
	// activated
	virtual void OnActivateView(BOOL bActivate, CView* pActivateView,
		CView* pDeactiveView);

	// hover.exe:0x41F1C0 create the six pens/brushes the dashboard and
	// the message boxes draw with (helper called by Create)
	BOOL CreateGdiObjects();

	void SuspendGame(DWORD dwUnk);	// hover.exe:0x40EE90
	void StopGameTimer();			// hover.exe:0x410280
	void RequestRenderFrame(int nUnk);		// hover.exe:0x40DC20
	void RequestDispDibBegin(int nUnk);		// hover.exe:0x4277E0 (same body as
									// RequestRenderFrame, state 6 instead of 3)
	void RequestDispDibEnd(int nUnk);		// hover.exe:0x4278E0 (same body as
									// RequestRenderFrame, state 7 instead of 3)
	void WaitWorkerIdle();				// hover.exe:0x4279E0
	// hover.exe:0x41F4E0 set the render state (m_dwMessageState) and, when it
	// actually changed and nRedraw is set, force the window to repaint
	void SetMessageState(DWORD dwState, int nRedraw);
	// hover.exe:0x413A00 stop the worker thread, apply the new speed/detail
	// level (theApp.m_dwSpeedDetail) and restart it; called by the Customize
	// Game dialog when the speed/detail slider moved
	void RequestDetailLevel(int nLevel, int nUnk);
	// hover.exe:0x413B10 stop the worker thread and enter state 4 (same body
	// as RequestRenderFrame); called after the player controls were changed
	void RequestDashboardRebuild(int nUnk);
	// hover.exe:0x41E5B0 reset the per-game HUD/scoreboard objects for a
	// new game
	void ResetDashboard();
	// hover.exe:0x4282F0 award the end-of-level score: time bonus, points
	// per remaining tick and (when bLevelBonus is set) the level bonus,
	// added into the document's running total and shown on the scoreboard
	void ComputeLevelBonus(DWORD bLevelBonus);
	// hover.exe:0x41F480 re-establish the offscreen buffers for the current
	// client size and restart the render thread (called by CMainFrame when
	// the frame has finished resizing)
	void OnFrameResized();

	// hover.exe:0x402250 per-frame render step: update the camera state,
	// run the scene pass, rasterize and present; called from the render
	// state dispatcher at hover.exe:0x41F012
	void RenderFrame();
	// hover.exe:0x407430 composite the offscreen buffers and present them
	// (BitBlt/StretchBlt into a CClientDC)
	void PresentFrame();
	// hover.exe:0x417E30 initialize the renderer DC, events and worker
	// thread. This is slot 36 of the original's CBumperView vtable, which
	// is CView::OnInitialUpdate in MFC 3.2, so the framework calls it once
	// the document exists -- nothing in the game calls it directly. It was
	// previously reconstructed as the non-virtual "Sub_417E30", which left
	// CView::OnInitialUpdate in the slot and meant the renderer was never
	// started, i.e. the view stayed blank.
	virtual void OnInitialUpdate();
	// hover.exe:0x4184B0 set up the background/render target for the current
	// zone dimensions (intended to be called with the zone width/height)
	void SetupZone(int nWidth, int nHeight);
	// hover.exe:0x41ECE0 worker thread body for the render state machine
	void WorkerThreadMain();

	// Helper stubs called by the renderer setup / worker thread cluster.
	// hover.exe:0x4274B0 set the offscreen bitmap size from the client rect
	void RequestResize(int nWidth, int nHeight, int nUnk);
	// hover.exe:0x410510 background-layer setup helper
	void BuildDashboard();
	// hover.exe:0x410380 renderer setup continuation (stub)
	void DrawKeyLabels();
	// hover.exe:0x428120 alternate renderer setup path (stub)
	void DrawF4Hint();
	// hover.exe:0x413C10 state 1 setup handler
	void ApplyDetailLevel(int nUnk);
	// hover.exe:0x4126C0 timer start helper
	void StartGameTimer();
	// hover.exe:0x427A80 state 5 setup handler
	void ZoomMap(int nUnk);
	// hover.exe:0x427BB0 state 6 setup handler
	void BeginDispDib();
	// hover.exe:0x427CC0 state 7 setup handler
	void EndDispDib();

	// hover.exe:0x40D340 frame-step state 1 handler (stub)
	void DrawPausedMessage();
	// hover.exe:0x40CCE0 frame-step state 2 handler: draw up to four
	// message strings (stub)
	void DrawMessageBox(const char* pszA, const char* pszB, const char* pszC,
		const char* pszD);
	// hover.exe:0x40D0A0 helper used by DrawMessageBox to draw one shadowed
	// (non-stretch) or plain (stretch) text line.
	void DrawMessageLine(RECT* pRect, const char* psz, COLORREF clrShadow,
		COLORREF clrText);
	// hover.exe:0x427F40 helper used by DrawMessageBox to draw a raised box
	// around the message rectangle.
	void DrawBevelBox(RECT* pRect, int nWidth);
	// hover.exe:0x4283F0 frame-step state 3 handler (stub)
	void DrawBonusMessage();

	// Worker-thread handshake variants of RequestRenderFrame (same body,
	// different target state for m_dwWorkerCmd). hover.exe:0x4275D0 stop the
	// worker and enter state 5, recording nUnk in m_bZoomIn
	void RequestMapZoom(int nUnk, int nHandshake);
	// hover.exe:0x4276E0 stop the worker and enter state 2
	void RequestWorkerExit(int nUnk);
	// hover.exe:0x41F350 tear the renderer down: stop the worker thread,
	// close the six events, delete the render contexts and the render
	// target, and release the offscreen DC
	void DestroyRenderer();
	// hover.exe:0x408D00 per-frame game step: run the countdown of the
	// timed game states and drive the per-frame object step
	void GameStep();
	// hover.exe:0x408DF0 per-frame status-bar step: refresh the two frame
	// rates and the player position readout every 58 ticks
	void UpdateStatusBar();
	// hover.exe:0x427D40 the per-frame object step itself: hand the frame
	// to the worker thread, step every object in the doc's second list,
	// reset the per-frame counters, translate the joystick buttons into
	// WM_KEYDOWNs and time the step
	void StepObjects();
	// hover.exe:0x427E50 call the input/think virtual (slot 7) on every
	// object in the document's second object list
	void ThinkAll();
	// hover.exe:0x427E90 clear the four renderer debug counters and run
	// the collider's per-frame pass
	void RunCollider();

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBumperView)
	protected:
	virtual void OnDraw(CDC* pDC);	// overridden to draw this view
	//}}AFX_VIRTUAL

// Message map handlers. The original's AFX_MSGMAP_ENTRY table lives at
// hover.exe:0x4BB0C8 (AFX_MSGMAP at 0x4BB0C0, base CView); the entries
// below are declared and mapped in the original table order.
public:
	afx_msg void OnDestroy();				// hover.exe:0x41E710
	afx_msg void OnSize(UINT nType, int cx, int cy);	// hover.exe:0x4183E0
	// one handler for the four ID_DBG_* flags (32768..32771)
	afx_msg void OnUpdateDbgFlag(CCmdUI* pCmdUI);	// hover.exe:0x402610
	afx_msg void OnCmd34000();				// hover.exe:0x427EC0
	afx_msg void OnUpdateCmd34000(CCmdUI* pCmdUI);	// hover.exe:0x427EE0
	afx_msg void OnCmd34011();				// hover.exe:0x427F00
	afx_msg void OnUpdateCmd34011(CCmdUI* pCmdUI);	// hover.exe:0x427F20
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);	// hover.exe:0x40C600
	afx_msg void OnDbgAddSmokePod();				// hover.exe:0x428590
	afx_msg void OnDbgAddWallPod();				// hover.exe:0x428610
	afx_msg void OnDbgAddJumpPod();				// hover.exe:0x428690
	afx_msg void OnDbgApplyInvincible();				// hover.exe:0x428710
	afx_msg void OnDbgApplyMapEraser();				// hover.exe:0x428770
	afx_msg void OnDbgApplySpeed();				// hover.exe:0x4287D0
	afx_msg void OnDbgApplySlow();				// hover.exe:0x428830
	afx_msg void OnGamePause();				// hover.exe:0x413C70
	afx_msg void OnUpdateGamePause(CCmdUI* pCmdUI);	// hover.exe:0x412860
	afx_msg void OnDbgFullClientRender();				// hover.exe:0x428970
	afx_msg void OnUpdateDbgFullClientRender(CCmdUI* pCmdUI);	// hover.exe:0x4289F0
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
											// hover.exe:0x40B540
	afx_msg void OnDbgInvincible();				// hover.exe:0x428A10
	afx_msg void OnUpdateDbgInvincible(CCmdUI* pCmdUI);	// hover.exe:0x428A40
	afx_msg void OnDbgRestockFlags();				// hover.exe:0x428A90
	afx_msg void OnUpdateDbgRestockFlags(CCmdUI* pCmdUI);	// hover.exe:0x428C00
	afx_msg void OnUpdateDbgAddSmokePod(CCmdUI* pCmdUI);	// hover.exe:0x428C30
	afx_msg void OnUpdateDbgAddWallPod(CCmdUI* pCmdUI);	// hover.exe:0x428C60
	afx_msg void OnUpdateDbgAddJumpPod(CCmdUI* pCmdUI);	// hover.exe:0x428C90
	afx_msg void OnUpdateDbgApplyInvincible(CCmdUI* pCmdUI);	// hover.exe:0x428CC0
	afx_msg void OnUpdateDbgApplyMapEraser(CCmdUI* pCmdUI);	// hover.exe:0x428CF0
	afx_msg void OnUpdateDbgApplySpeed(CCmdUI* pCmdUI);	// hover.exe:0x428D20
	afx_msg void OnUpdateDbgApplySlow(CCmdUI* pCmdUI);	// hover.exe:0x428D50
	afx_msg void OnDbgPixelDouble();				// hover.exe:0x429230
	afx_msg void OnUpdateDbgPixelDouble(CCmdUI* pCmdUI);	// hover.exe:0x4292F0
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);	// hover.exe:0x420DF0
	afx_msg void OnDbgAltPresent();				// hover.exe:0x429340
	afx_msg void OnUpdateDbgAltPresent(CCmdUI* pCmdUI);	// hover.exe:0x429360
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
											// hover.exe:0x429380
	afx_msg LRESULT OnUserInput(WPARAM wParam, LPARAM lParam);
											// hover.exe:0x408AF0 (WM_USER+0)
	afx_msg LRESULT OnUserGameOver(WPARAM wParam, LPARAM lParam);
											// hover.exe:0x427D20 (WM_USER+1)
	afx_msg LRESULT OnUserNextLevel(WPARAM wParam, LPARAM lParam);
											// hover.exe:0x427D30 (WM_USER+2)
	afx_msg void OnEscape();				// hover.exe:0x428890
	afx_msg LRESULT OnMciNotify(WPARAM wParam, LPARAM lParam);
											// hover.exe:0x428940 (MM_MCINOTIFY)
	afx_msg LRESULT OnHelpMsg(WPARAM wParam, LPARAM lParam);
											// hover.exe:0x428D80 (WM_HELP)
	afx_msg LRESULT OnContextMenuMsg(WPARAM wParam, LPARAM lParam);
											// hover.exe:0x428D90 (WM_CONTEXTMENU)
	afx_msg void OnCmd34034();				// hover.exe:0x4294F0

	DECLARE_MESSAGE_MAP()

public:
// Member variables (hover.exe offsets in comments). Public in the
// original: CHumanPlayer::Think reads the joystick state fields
// (+0x6C..+0x78) directly through Hover_GetView's return value.
	DWORD m_bCursorShown;		// +0x34 = 1
	DWORD m_dwUnk0038;		// +0x38
	DWORD m_dwKeySeqPos;		// +0x3C = 0
	CWinThread* m_pRenderThread;		// +0x40 = 0
	DWORD m_dwUnk0044;		// +0x44 = 0
	HANDLE m_hEvtCommand;		// +0x48 = 0
	HANDLE m_hEvtWorkerBusy;		// +0x4C synchronization event handle
	HANDLE m_hEvtWorkerIdle;		// +0x50 synchronization event handle
	HANDLE m_hEvtCommandAck;		// +0x54 synchronization event handle
	HANDLE m_hEvtUnused1;		// +0x58 synchronization event handle
	HANDLE m_hEvtUnused2;		// +0x5C synchronization event handle
							// (+0x48/+0x4C are event handles too;
							// m_hEvtCommand is waited on/signaled with them)
	DWORD m_dwTimerID;		// +0x60 = 0
	DWORD m_dwUnk0064;		// +0x64 = 0
	DWORD m_dwUnk0068;		// +0x68 = 0
	DWORD m_bInputLeft;		// +0x6C = 0
	DWORD m_bInputRight;		// +0x70 = 0
	DWORD m_bInputUp;		// +0x74 = 0
	DWORD m_bInputDown;		// +0x78 = 0
	DWORD m_dwStateCountdown;		// +0x7C
	DWORD m_bNeedRepaint;		// +0x80 = 0
	DWORD m_bInitialized;		// +0x84 = 0
	DWORD m_dwUnk0088;		// +0x88 set to 1 by CBumperView::RequestResize
	DWORD m_dwOffscreenWidth;		// +0x8C offscreen width (OnDraw blit size)
	DWORD m_dwOffscreenHeight;		// +0x90 offscreen height
	RECT m_rcMessage;		// +0x94 the message box (DrawMessageBox); a
							//   bottom of -1 marks it as not shown
	DWORD m_dwRequestedWidth;		// +0xA4 = 0
	DWORD m_dwRequestedHeight;		// +0xA8 = 0
	DWORD m_dwWorkerCmd;		// +0xAC = 4
	DWORD m_bZoomIn;		// +0xB0
	CDC m_dcOffscreen;		// +0xB4
	HBITMAP m_hOldBitmap;		// +0xC4
	CRenderContext* m_pViewCtx;	// +0xC8 render context (camera/shade state)
	CRenderContext* m_pMirrorCtx;	// +0xCC second render context (used when
							// m_bSmokeBlind selects the alternate shade table)
	CSpanRasterizer* m_pRasterizer;	// +0xD0 offscreen render target (span rasterizer)
	DWORD m_dwCamX;		// +0xD4 camera x (from the human player)
	DWORD m_dwCamY;		// +0xD8 camera y
	DWORD m_dwCamAngle;		// +0xDC camera angle
	DWORD m_dwCamZ;		// +0xE0 camera z (player +0x8C + 0x28)
	DWORD m_dwDrawerMode;		// +0xE4 = g_adwDrawerModeForDetail[theApp.m_dwSpeedDetail]
	CVelocityIndicator m_needle;	// +0xE8
	CMapRenderer m_map;	// +0x150
	DWORD m_bSmokeBlind;		// +0x21C = 0
	CRect m_rc3DView;		// +0x220 present blit rect (PresentFrame)
	CRect m_rcMirrorView;		// +0x230 second present blit rect (used when
							// theApp.m_bRearView is set)
	// The original constructor calls each of these member ctors
	// individually (no ??_L vector-constructor loop), so they are
	// declared as separate members rather than arrays.
	CDigitDisplay m_ctrJumpPods;		// +0x240
	CDigitDisplay m_ctrSmokeBombs;		// +0x268
	CDigitDisplay m_ctrTempWalls;		// +0x290
	CDigitDisplay m_score;		// +0x2B8
	CSegmentGauge m_gaugeSpeed;		// +0x2E0
	CSegmentGauge m_gaugeInvincible;		// +0x310
	CSegmentGauge m_gaugeSlow;		// +0x340
	CSegmentGauge m_gaugeSpeedPod;		// +0x370
	CSegmentGauge m_gaugeAltitude;		// +0x3A0
	CSegmentGauge m_gaugeSmoke;		// +0x3D0
	CSegmentGauge m_gaugeTempWall;		// +0x400
	CFlagStrip m_flagsPlayer;		// +0x430
	CFlagStrip m_flagsRobot;		// +0x480
	CPen m_penBlack;			// +0x4D0
	CPen m_penYellow;			// +0x4D8
	CBrush m_brushBlack;		// +0x4E0
	CBrush m_brushYellow;		// +0x4E8
	CBrush m_brushGreen;		// +0x4F0
	CBrush m_brushRed;		// +0x4F8
	DWORD m_dwMessageState;		// +0x500 = 2
	DWORD m_dwFlagsLeft;		// +0x504 = 0
	DWORD m_dwPointsPerFlag;		// +0x508 = 0
	DWORD m_dwFlagBonus;		// +0x50C = 0
	DWORD m_dwNextLevel;		// +0x510 = 0
	DWORD m_dwLevelBonus;		// +0x514 = 0
	DWORD m_dwUnk0518;		// +0x518 = 0
	DWORD m_dwUnk051C;		// +0x51C = 0
	DWORD m_dwUnk0520;		// +0x520 = 0
	DWORD m_dwUnk0524;		// +0x524 = 0
	DWORD m_dwFpsBaseTick;		// +0x528
	DWORD m_dwLastPresentTick;		// +0x52C last presented frame's GetTickCount
	DWORD m_dwPresentCount;		// +0x530 presented frame counter
	DWORD m_dwStepRateBaseTick;		// +0x534 tick of the last status-bar refresh
							//   (game-step rate base, UpdateStatusBar)
	DWORD m_dwLastStepTick;		// +0x538 last game step's GetTickCount
							//   (StepObjects)
	DWORD m_dwStepCount;		// +0x53C game step counter (StepObjects)
	DWORD m_bFullMap;		// +0x540 = 0
	DWORD m_bDoubleSize;		// +0x544 = 0
	char m_pad0548[0x14];	// +0x548
};							// sizeof = 0x55C in hover.exe

/////////////////////////////////////////////////////////////////////////////
// Globals defined in bumpview.cpp

extern DWORD g_adwDrawerModeForDetail[6];	// hover.exe:0x4C4CC8
extern CPalette g_palGame;	// hover.exe:0x4A1D50 palette selected
									// into the paint DC by OnDraw

extern DWORD g_dwCurZone;		// hover.exe:0x460998 zone descriptor index
									// used by the PresentFrame present
extern DWORD g_bFullFramePresent;		// hover.exe:0x4C4CA8 selects the present
									// path in PresentFrame

extern int g_nStatusRefreshCountdown;			// hover.exe:0x4C4CEC = 0x39 down-counter
									// that paces the UpdateStatusBar status-bar
									// refresh
extern DWORD g_dwPrevJoyButtons;		// hover.exe:0x4C4CF0 previous joystick
									// button mask (StepObjects)

// Viewport/projection state computed per frame by CMapRenderer::ProjectScene
// and consumed by ProjectObject/ProjectPoint
extern DWORD g_dwMapCamX;		// hover.exe:0x4A4790 camera x
extern DWORD g_dwMapCamY;		// hover.exe:0x4A4794 camera y
extern DWORD g_dwMapCullRight;		// hover.exe:0x4A4798 viewport right
extern DWORD g_dwMapCullBottom;		// hover.exe:0x4A479C viewport bottom
extern DWORD g_dwMapFaceCount;		// hover.exe:0x4A47B0 world object +0x1C
extern DWORD g_dwMapRotAngle;		// hover.exe:0x4A47B8 camera angle - 0x80
extern DWORD g_dwMapCullLeft;		// hover.exe:0x4A47BC viewport left
extern DWORD g_dwMapCullTop;		// hover.exe:0x4A47D8 viewport top
extern DWORD g_dwMapOriginX;		// hover.exe:0x4A47E0 projection term x
extern DWORD g_dwMapOriginY;		// hover.exe:0x4A47E4 projection term y

// Free helpers called by the renderer setup cluster.
// hover.exe:0x453352 MFC-style FindResource wrapper, also used by the
// splash dialog's DIB loader (splashdlg.cpp)
HRSRC __stdcall Hover_FindResource(HINSTANCE hInst, LPCSTR lpName,
	LPCSTR lpType);
// hover.exe:0x418A30 look up a texture by name in g_mapUnk_4A1D30 (NULL if absent)
CObject* Hover_LookupTexture(const char* pszName);
// hover.exe:0x408AA0 multimedia timer callback used by
// CBumperView::StartGameTimer
void CALLBACK Hover_GameTimerProc(UINT uID, UINT uMsg, DWORD dwUser,
	DWORD dw1, DWORD dw2);
#endif // __BUMPVIEW_H__
