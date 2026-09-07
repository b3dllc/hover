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
class CSub410F20;
class CGameObj;
class CSub406090;
class CMerlinLine;
class CMerlinStatic;
class CMerlinDynamic;
class CMerlinBSP;
class CMerlinWorld;

/////////////////////////////////////////////////////////////////////////////
// Helper classes embedded in CBumperView. None of these has a vtable in
// the original; their constructors are decompiled in bumpview.cpp.

// CSub423030: offscreen render target (holds a CDC and a CBitmap).
// ctor hover.exe:0x423030, dtor hover.exe:0x420E60 (trivial), sizeof = 0x68.
class CSub423030
{
public:
	CSub423030();			// hover.exe:0x423030
	~CSub423030();			// hover.exe:0x420E60

	// hover.exe:0x405A10 recompute the m_dwUnk5C/m_dwUnk60 camera basis
	// offsets from the view yaw (pView->m_dwUnk00DC) and the doc's
	// m_dwUnk8338/m_dwUnk833C factors
	void Sub_405A10(CBumperView* pView, CBumperDoc* pDoc);
	// hover.exe:0x411D10 store the source rectangle and compute a scaling
	// quotient from the smaller dimension and the divisor at +0x1C
	int Sub_411D10(const RECT* pRect);
	// hover.exe:0x401E80 blit the held bitmap into pDC, framing the dirty
	// rect
	void Sub_401E80(CDC* pDC, int bUnk);

	// hover.exe:0x4140E0 compute the fixed-point x/y scale factors
	// (m_wUnk44/m_wUnk48) from the bitmap dimensions and player/global
	// constants; helper called by Sub_411D80.
	void Sub_4140E0();
	// hover.exe:0x411D80 create the 8-bpp DIB section render target and
	// select it into the internal DC. Takes the destination CDC, the
	// dashboard DIB (header first, so the pixels start biSize + 0x400
	// bytes in) and the source rectangle components.
	void Sub_411D80(CDC* pInternalDC, BYTE* pDib, int nLeft, int nTop,
		int nRight, DWORD nBottom);

	// The first 0x28 bytes are used as two RECTs (+0x00 source, +0x10
	// dirty), a width/orientation scratch at +0x20 and a height scratch at
	// +0x24 by Sub_411D10/Sub_411D80/Sub_4140E0.  The dirty RECT's bottom
	// field (+0x1C) is also read as the divisor by Sub_411D10.  The quotient
	// written by Sub_411D10 aliases the first DWORD of m_bitmap at +0x2C,
	// so it is accessed through ScratchQuotient().
	RECT m_rcSource;		// +0x00
	RECT m_rcDirty;			// +0x10; bottom (+0x1C) used as divisor
	DWORD m_dwUnk20;		// +0x20 orientation (Sub_411D10), then width
	DWORD m_dwUnk24;		// +0x24 height
	CGdiObject* m_pUnk28;	// +0x28 = NULL (old GDI object selected in m_dc)
	CBitmap m_bitmap;		// +0x2C

	DWORD& ScratchQuotient() { return *(DWORD*)&m_bitmap; }
	CDC m_dc;				// +0x34
	WORD m_wUnk44;			// +0x44 = 0 (read as a dword fixed-point factor
							// together with m_wUnk46 by Sub_405A10)
	WORD m_wUnk46;			// +0x46 = 0
	WORD m_wUnk48;			// +0x48 = 0 (same for m_wUnk4A)
	WORD m_wUnk4A;			// +0x4A = 0
	CDC* m_pUnk4C;			// +0x4C internal destination CDC used by Sub_401E80
	DWORD m_dwUnk50;		// +0x50 = 0
	DWORD m_dwUnk54;		// +0x54 = 0
	DWORD m_dwUnk58;		// +0x58 = 0
	DWORD m_dwUnk5C;		// +0x5C camera basis offset x (Sub_405A10)
	DWORD m_dwUnk60;		// +0x60 render height / camera basis offset y
	CPen* m_pUnk64;			// +0x64 pen selected in to draw the update-area
							//   indicator in Sub_401E80
};							// sizeof = 0x68

// CSub422F50: the software renderer's frame buffer / span-draw context
// embedded in CBumperView at +0x150. Holds the pixel bits, clip rect, row
// pitch, current color, the camera/projection state and the per-class draw
// colors; the frame step projects every visible game object into it.
// ctor hover.exe:0x422F50, sizeof = 0xCC (no destructor).
class CSub422F50
{
public:
	CSub422F50();			// hover.exe:0x422F50

	// hover.exe:0x407160 per-frame scene pass: set the viewport globals
	// from the human player's camera, then project every object in the
	// doc's two object lists (and the tracked array when theApp.m_dwUnk01E0)
	void Sub_407160(CBumperDoc* pDoc);
	// hover.exe:0x404BB0 visibility-test one object against the viewport
	// globals and project its points with Sub_4055B0
	void Sub_404BB0(CBumperDoc* pDoc, CGameObj* pObj, int bUnk);
	// hover.exe:0x4055B0 project the world point (lX,lY) to screen,
	// rotating by lAngle when theApp.m_dwUnk01D4 is set
	void Sub_4055B0(DWORD* pdwOut, long lX, long lY, long lCX, long lCY,
		long lV1, long lV2, long lAngle);
	// hover.exe:0x4053A0 draw every projected object (walls, pods, player
	// triangles) and the tracked-object markers into the pixel bits
	void Sub_4053A0(CBumperView* pView, CBumperDoc* pDoc);
	// hover.exe:0x401C00 draw one projected object (dispatch on class)
	void Sub_401C00(CGameObj* pObj, int bUnk);
	// hover.exe:0x401980 draw a clipped line into the pixel bits
	void Sub_401980(long lX1, long lY1, long lX2, long lY2);
	// hover.exe:0x403910 draw a small cross marker into the pixel bits
	void Sub_403910(long lX, long lY, long lUnk);
	// hover.exe:0x401520 clip a line endpoint pair to the bitmap (stub)
	int Sub_401520(long* plA, long* plB);
	// hover.exe:0x42D3E0 blit the rendered rect into pDC (StretchBlt 2x
	// when the view's m_dwUnk0544, BitBlt otherwise)
	void Sub_42D3E0(CDC* pDC);
	// hover.exe:0x40FEE0 set up the renderer viewport/scaling state from
	// the supplied rectangle and doc pointer.
	void Sub_40FEE0(CDC* pDC, CBumperView* pView, CBumperDoc* pDoc,
		DWORD dwLeft, DWORD dwTop, DWORD dwRight, DWORD dwBottom);

	DWORD m_dwUnk00;		// +0x00 clip rect left (Sub_403910)
	DWORD m_dwUnk04;		// +0x04 clip top
	DWORD m_dwUnk08;		// +0x08 clip right
	DWORD m_dwUnk0C;		// +0x0C clip bottom
	BYTE* m_pUnk10;			// +0x10 pixel bits / render-target bits handle
	DWORD m_dwUnk14;		// +0x14 render target width (bytes per row basis)
	DWORD m_dwUnk18;		// +0x18 bitmap height (y-flip base)
	BYTE m_bUnk1C;			// +0x1C current draw color
	char m_pad001D[0x3];	// +0x1D
	DWORD m_dwUnk20;		// +0x20 current color, dword-expanded
	DWORD m_dwUnk24;		// +0x24 row pitch
	DWORD m_dwUnk28;		// +0x28 row pitch in dwords (rounded)
	DWORD m_dwUnk2C;		// +0x2C line-draw |dx| (Sub_401980)
	DWORD m_dwUnk30;		// +0x30 line-draw |dy|
	// +0x34..+0x58: clip/scan setup used by the rasterizer helpers.
	DWORD m_dwUnk34;		// +0x34
	DWORD m_dwUnk38;		// +0x38
	DWORD m_dwUnk3C;		// +0x3C
	DWORD m_dwUnk40;		// +0x40
	DWORD m_dwUnk44;		// +0x44
	DWORD m_dwUnk48;		// +0x48
	DWORD m_dwUnk4C;		// +0x4C
	DWORD m_dwUnk50;		// +0x50
	DWORD m_dwUnk54;		// +0x54
	DWORD m_dwUnk58;		// +0x58
	DWORD m_dwUnk5C;		// +0x5C rendered rect left / screen center x
	DWORD m_dwUnk60;		// +0x60 rendered rect top / screen center y
	DWORD m_dwUnk64;		// +0x64 rendered rect right
	DWORD m_dwUnk68;		// +0x68 rendered rect bottom
	WORD m_awUnk006C[10];	// +0x6C = {0}; the last four pairs are read as
							// dword camera fields (+0x74/+0x78 position,
							// +0x7C projection base) by the frame pass
	WORD m_wUnk0080;		// +0x80 = 0 (with m_wUnk0082: projection divisor)
	WORD m_wUnk0082;		// +0x82 = 0
	CDC* m_pUnk0084;		// +0x84 owning DC (Sub_42D3E0 blit source)
	WORD m_wUnk0088;		// +0x88 half the viewport extent (Sub_407160)
	char m_pad008A[0x2];	// +0x8A
	DWORD m_dwUnk008C;		// +0x8C color: human player
	DWORD m_dwUnk0090;		// +0x90 color: robot
	DWORD m_dwUnk0094;		// +0x94 color: robot, alternate
	DWORD m_dwUnk0098;		// +0x98 color: default (walls, pods, markers)
	DWORD m_dwUnk009C;		// +0x9C color: wall/pod alternate
	DWORD m_dwUnk00A0;		// +0xA0 color: wall, flagged (alternate)
	DWORD m_dwUnk00A4;		// +0xA4 color: waypoint/track marker
	DWORD m_dwUnk00A8;		// +0xA8 color: wall, flagged
	DWORD m_dwUnk00AC;		// +0xAC frame counter (mod 8)
	char m_pad00B0[0x18];	// +0xB0
	DWORD m_dwUnk00C8;		// +0xC8 = 0x40000

private:
	// hover.exe:0x40FB00 helper: store the render-target bits, width and
	// height and compute the dword row pitch.
	void Sub_40FB00(DWORD dwBits, DWORD dwWidth, DWORD dwHeight);
	// hover.exe:0x40FBB0 helper: set the clip rectangle and derived span
	// rasterizer pointers from (bottom, right, top, right).
	void Sub_40FBB0(DWORD dwBottom, DWORD dwRight, DWORD dwTop,
		DWORD dwRight2);
};							// sizeof = 0xCC

// CSub422BD0: ctor hover.exe:0x422BD0, dtor hover.exe:0x422C20
// (operator delete on m_pUnk1C), sizeof = 0x28.
class CSub422BD0
{
public:
	CSub422BD0();			// hover.exe:0x422BD0
	~CSub422BD0();			// hover.exe:0x422C20

	// hover.exe:0x40FCE0 draw the number into the render target
	void Sub_40FCE0(DWORD dwUnkHdc, DWORD dwX, DWORD dwY);
	// hover.exe:0x411A00 dashboard element setup
	void Sub_411A00(int nFlag, const RECT* pRect, BYTE* pBits, DWORD dwA,
		DWORD dwB);
	// hover.exe:0x411920 copy a rectangle into the destination point at +0x20
	void Sub_411920(const RECT* pRect);

	DWORD m_dwUnk00;		// +0x00 = 0; used as an active flag by
							// CBumperView::Sub_407430
	DWORD m_dwUnk04;		// +0x04 = 0; number of digits to draw
	DWORD m_dwUnk08;		// +0x08 = 0; value to display
	DWORD m_dwUnk0C;		// +0x0C = 0; source bitmap width
	DWORD m_dwUnk10;		// +0x10 = 0; source bitmap height
	DWORD m_dwUnk14;		// +0x14 = 0; digit tile width
	DWORD m_dwUnk18;		// +0x18 = 0; digit tile height
	void* m_pUnk1C;			// +0x1C = 0 (freed by the destructor)
	DWORD m_dwUnk20;		// +0x20 = 0; destination x
	DWORD m_dwUnk24;		// +0x24 = 0; destination y
};							// sizeof = 0x28

// hover.exe:0x42A9F0 reset a dashboard gauge to the {nA, nB} tick pair
// (STUB; the original inlines the pair's constructor, hover.exe:0x42AE90)
class CSub422C80;
void Hover_sub_42A9F0(CSub422C80* pGauge, int nA, int nB);

// CSub422C80: ctor hover.exe:0x422C80, sizeof = 0x30 (no destructor).
// CSub42AE90: the 16.16 fixed-point value the dashboard gauges are fed
// with (integer part in the high word, fraction in the low word; the
// double constructor multiplies by 65536.0 and truncates). Passed to
// CSub422C80::Sub_42A9F0 by value; both classes' gauge-fill methods are
// defined in mazeobj.cpp, next to the pod classes that build the values.

class CSub42AE90
{
public:
	CSub42AE90() { }
	inline CSub42AE90(WORD wInt, WORD wFrac);	// hover.exe:0x42AE90
	CSub42AE90(double dbl);				// hover.exe:0x42A9D0

	WORD m_wFrac;			// +0x00 fractional part
	WORD m_wInt;			// +0x02 integer part
};							// sizeof = 4

class CSub422C80
{
public:
	CSub422C80();			// hover.exe:0x422C80

	// hover.exe:0x42A9F0 light the segments corresponding to the 0..1
	// fixed-point fraction (m_dwUnk24 = clamp(fix * m_dwUnk1C, 0,
	// m_dwUnk1C)); defined in mazeobj.cpp
	void Sub_42A9F0(CSub42AE90 fix);

	// hover.exe:0x401AD0 draw the gauge into pDC
	void Sub_401AD0(CDC* pDC, int bUnk);
	// hover.exe:0x411D60 gauge setup helper (stub; part of the dashboard
	// subsystem, not the DIB buffer setup).
	void Sub_411D60(DWORD dwA, DWORD dwB, DWORD dwC, DWORD dwD);

	DWORD m_dwUnk00;		// +0x00 = 0; gauge left (horizontal) or left (vertical)
	DWORD m_dwUnk04;		// +0x04 = 0; gauge top (horizontal) or bottom (vertical)
	DWORD m_dwUnk08;		// +0x08 = 10; gauge right edge / segment width
	DWORD m_dwUnk0C;		// +0x0C = 10; gauge bottom edge / segment height
	DWORD m_dwUnk10;		// +0x10 = 0; inactive segment CBrush* (cast from DWORD)
	DWORD m_dwUnk14;		// +0x14 = 0; active segment CBrush* (cast from DWORD)
	DWORD m_dwUnk18;		// +0x18 = 0; outline CPen* (cast from DWORD)
	DWORD m_dwUnk1C;		// +0x1C = 1; number of segments
	DWORD m_dwUnk20;		// +0x20; orientation: nonzero horizontal, zero vertical
	DWORD m_dwUnk24;		// +0x24 = 0; first active segment index
	DWORD m_dwUnk28;		// +0x28; previous m_dwUnk24, updated each draw
	DWORD m_dwUnk2C;		// +0x2C = 0; segment size (width or height)
};							// sizeof = 0x30

// CSub422C30: ctor hover.exe:0x422C30, dtor hover.exe:0x422C60
// (operator delete on m_pUnk14 and m_pUnk18), sizeof = 0x50.
class CSub422C30
{
public:
	CSub422C30();			// hover.exe:0x422C30
	~CSub422C30();			// hover.exe:0x422C60

	// hover.exe:0x40E9A0 draw the element into the render target (stub)
	void Sub_40E9A0(DWORD dwUnkHdc, DWORD dwX, DWORD dwY);
	// hover.exe:0x411BB0 dashboard sprite setup
	void Sub_411BB0(DWORD dwUnk, DWORD dwLeft1, DWORD dwTop1,
		DWORD dwRight1, DWORD dwBottom1, DWORD dwLeft2, DWORD dwTop2,
		DWORD dwRight2, DWORD dwBottom2, BYTE* pBits, DWORD dwWidth,
		DWORD dwHeight);
	// hover.exe:0x411CF0 copy a rectangle into the source rect at +0x04
	void Sub_411CF0(const RECT* pRect);

	DWORD m_dwUnk00;		// +0x00 = 1
	DWORD m_dwUnk04;		// +0x04 = 0 (destination x base / pointer)
	DWORD m_dwUnk08;		// +0x08 = 0 (destination y)
	void* m_pUnk0C;			// +0x0C = 0
	void* m_pUnk10;			// +0x10 = 0
	void* m_pUnk14;			// +0x14 = 0 (source bits A, freed by destructor)
	void* m_pUnk18;			// +0x18 = 0 (source bits B, freed by destructor)
	RECT m_rcSource1;		// +0x1C
	RECT m_rcSource2;		// +0x2C
	DWORD m_dwUnk3C;		// +0x3C = 1 (sprite count)
	DWORD m_dwUnk40;		// +0x40 = 0
	DWORD m_dwUnk44;		// +0x44 = 0 (source selection threshold)
	DWORD m_dwUnk48;		// +0x48 = 0
	DWORD m_dwUnk4C;		// +0x4C = 0
};							// sizeof = 0x50

/////////////////////////////////////////////////////////////////////////////
// CSub419520: trivial 0xC0-byte entry constructed in the span buffer
// allocated by CSub406090::Sub_4192F0. ctor hover.exe:0x419520.

class CSub419520
{
public:
	CSub419520();			// hover.exe:0x419520

	char m_pad[0xC0];		// +0x00
};

/////////////////////////////////////////////////////////////////////////////
// CSub406C90: span-buffer header object reached through the render target
// object (CSub406090::m_pUnk00). Only Sub_406C90 (called by the stubbed
// rasterizer core hover.exe:0x406090) is known.

class CSub406C90
{
public:
	// hover.exe:0x41B870 construct from the parent render target
	CSub406C90(CSub406090* pParent);
	// hover.exe:0x41B8C0 destroy the base span-buffer
	~CSub406C90();

	// hover.exe:0x406C90 initialize the span-buffer header chain in the
	// m_pUnk18 buffer
	void Sub_406C90();

	DWORD m_dwUnk00;		// +0x00
	DWORD m_dwUnk04;		// +0x04
	DWORD m_dwUnk08;		// +0x08
	DWORD m_dwUnk0C;		// +0x0C
	DWORD m_dwUnk10;		// +0x10
	DWORD m_dwUnk14;		// +0x14
	DWORD* m_pUnk18;		// +0x18 base buffer
	DWORD m_dwUnk1C;		// +0x1C chain cursor
};							// sizeof = 0x20

/////////////////////////////////////////////////////////////////////////////
// CSub406090: the offscreen render target object (the span rasterizer)
// pointed to by CBumperView::m_pUnk00D0.
//
// This is the same class texheap.h calls CSub402E00 (and its +0x00 span
// heap is what texheap.h calls CSub403980); the two views of the class are
// not yet unified, so renderer methods declared there are called through
// casts from this side.

// The object Sub_406090 takes as its first argument (CBumperDoc::
// m_dwUnk0188) is the level's CMerlinWorld: the sprite array it walks is
// m_arrUnk28 (m_pData at +0x2C, m_nSize at +0x30), the BSP node array is
// m_arrUnk50 (m_pData at +0x54, node 0 is the root) and the face table
// Sub_405FA0 indexes with the node's m_wUnk44 is m_arrUnk14 (m_pData at
// +0x18).

// tagSPANLINK: one node of a per-column span list (the m_pUnk40 array
// elements consumed by Sub_406090's column loop).

typedef struct tagSPANLINK
{
	WORD m_wUnk00;				// +0x00
	WORD m_wUnk02;				// +0x02
	DWORD m_dwUnk04;			// +0x04 nonzero: 4-arg textured drawer
	DWORD m_dwUnk08;			// +0x08
	DWORD m_dwUnk0C;			// +0x0C
	DWORD m_dwUnk10;			// +0x10
	tagSPANLINK* m_pUnk14;		// +0x14 next
} SPANLINK;						// sizeof = 0x18

class CSub406090
{
public:
	// hover.exe:0x4191B0 construct an offscreen render target of the given
	// dimensions; dwPalArg is the doc's world-manager palette argument.
	CSub406090(int nWidth, int nHeight, DWORD dwPalArg);
	// hover.exe:0x4193A0 destroy the offscreen render target and its buffers.
	~CSub406090();

	// hover.exe:0x406090 rasterize the world through the render context
	// pCtx: rebuild the sprite facing segments, then walk the span columns
	void Sub_406090(DWORD dwUnk, CSub410F20* pCtx);

	// hover.exe:0x419440 create the DIB section for the render target using
	// the supplied palette and store the pixel bits pointer at +0x04.
	void Sub_419440(CPalette* pPalette);
	// hover.exe:0x4192F0 allocate the span buffers using dwPalArg.
	void Sub_4192F0(DWORD dwPalArg);

	// Sprite-pass helpers called by Sub_406090:
	// hover.exe:0x405B90 test a segment against the view (called with
	// both CMerlinDynamic sprites and CMerlinBSP nodes)
	int Sub_405B90(CMerlinLine* pLine);
	// hover.exe:0x405CE0 submit a span pair (returns 0 on rejection).
	// pA is the object whose texture links are advanced -- a
	// CMerlinDynamic when nC != 0, a CMerlinStatic when nC == 0 -- and pB
	// supplies the segment.
	int Sub_405CE0(CMerlinLine* pA, CMerlinLine* pB, int nC, WORD wOrd);
	// hover.exe:0x405FA0 walk the maze BSP tree back-to-front, submitting
	// the visible faces with Sub_405CE0
	void Sub_405FA0(short nNode);

	CSub406C90* m_pUnk00;	// +0x00 span buffer object
	DWORD m_dwUnk04;		// +0x04 DIB section pixel bits pointer (used as the
						//   rasterizer destination base)
	char m_pad08[0x4];		// +0x08
	CBitmap m_bitmap;		// +0x0C DIB section bitmap (m_hObject at +0x10)
	DWORD m_dwWidth;		// +0x14 render target width (aligned to 4)
	DWORD m_dwHeight;		// +0x18 render target height
	DWORD m_dwUnk1C;		// +0x1C
	CMerlinWorld* m_pUnk20;	// +0x20 the level world for this frame
	DWORD m_dwUnk24;		// +0x24
	DWORD m_dwUnk28;		// +0x28 render flags accumulated over the frame
	DWORD m_dwNegWidth;		// +0x2C negated width
	DWORD m_dwUnk30;		// +0x30 (screen width << 16) / 0x55
	DWORD m_dwUnk34;		// +0x34 camera angle << 16, biased/wrapped
	DWORD m_dwUnk38;		// +0x38
	DWORD* m_pUnk3C;		// +0x3C allocated span buffer
	DWORD* m_pUnk40;		// +0x40 allocated span buffer (column lists)
	DWORD* m_pUnk44;		// +0x44 allocated span buffer
};							// sizeof = 0x48

/////////////////////////////////////////////////////////////////////////////
// ZONE_DESCRIPTOR: one of the six board-zone geometry blocks at
// hover.exe:0x4609A0.  Each block is 0x338 bytes and is initialized by
// Hover_sub_41F540 in hover.cpp.  The field names reflect the offsets and
// the callers in CBumperView; many semantic meanings are still unknown.

struct ZONE_DESCRIPTOR
{
	DWORD m_dwWidth;			// +0x00
	DWORD m_dwHeight;			// +0x04
	DWORD m_dwUnk08;			// +0x08
	DWORD m_dwUnk0C;			// +0x0C
	DWORD m_dwUnk10;			// +0x10
	DWORD m_dwUnk14;			// +0x14
	DWORD m_dwUnk18;			// +0x18
	DWORD m_dwUnk1C;			// +0x1C
	DWORD m_dwUnk20;			// +0x20
	DWORD m_dwUnk24;			// +0x24
	DWORD m_dwUnk28;			// +0x28
	DWORD m_dwUnk2C;			// +0x2C
	DWORD m_dwTopSplit;			// +0x30 used by Sub_4184B0
	DWORD m_dwPad34;			// +0x34
	DWORD m_dwBottomSplit;		// +0x38 used by Sub_4184B0
	DWORD m_dwPad3C;			// +0x3C
	DWORD m_adwGaugeParams[6];	// +0x40..0x54 passed to CSub422C80::Sub_411D60
	DWORD m_adwUnk58[6];		// +0x58..0x6C spacing values for waypoint grids
	RECT m_rcWaypointRow0[10];	// +0x70 first row of waypoint rectangles
	RECT m_rcWaypointRow1[10];	// +0x110 second row of waypoint rectangles
	RECT m_rcSpriteParams[4];	// +0x1B0..0x1EF sprite/flag-drop rectangles
	RECT m_rcComposite1;		// +0x1F0
	RECT m_rcComposite2;		// +0x200
	RECT m_rcUnk210;			// +0x210

	// The dashboard rectangles at +0x220..+0x257 intentionally alias each
	// other in the original binary, so model them with an anonymous union.
	union
	{
		struct
		{
			RECT m_rcUnk220;	// +0x220
			RECT m_rcUnk230;	// +0x230
			RECT m_rcUnk240;	// +0x240 (spill)
		};
		struct
		{
			DWORD m_dwPad220[2];	// +0x220
			RECT m_rcUnk228;		// +0x228
			RECT m_rcUnk238;		// +0x238
			RECT m_rcUnk248;		// +0x248
		};
	};

	RECT m_rcViewport;			// +0x258 passed to CSub422F50::Sub_40FEE0
	RECT m_rcUnk268;			// +0x268
	RECT m_rcUnk278;			// +0x278 passed to CSub423030::Sub_411D80
	RECT m_rcGaugeRects[3];		// +0x288, 0x298, 0x2A8
	DWORD m_adwWaypointMarkers[6];// +0x2B8..0x2CC dashboard marker positions
	DWORD m_dwUnk2D0;			// +0x2D0
	RECT m_rcUnk2D4;			// +0x2D4
	RECT m_rcUnk2E4;			// +0x2E4
	RECT m_rcUnk2F4;			// +0x2F4
	RECT m_rcGaugeRects2[3];	// +0x304, 0x314, 0x324
	DWORD m_dwUnk334;			// +0x334
};							// sizeof = 0x338

/////////////////////////////////////////////////////////////////////////////
// CBumperView

class CBumperView : public CView
{
	DECLARE_DYNCREATE(CBumperView)

public:
	// hover.exe:0x427CF0 enter game state dwState (defined in mazeobj.cpp,
	// next to the flag capture that triggers the win state)
	void Sub_427CF0(DWORD dwState);
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
	BOOL Sub_41F1C0();

	void Sub_40EE90(DWORD dwUnk);	// hover.exe:0x40EE90
	void Sub_410280();			// hover.exe:0x410280
	void Sub_40DC20(int nUnk);		// hover.exe:0x40DC20
	void Sub_4277E0(int nUnk);		// hover.exe:0x4277E0 (same body as
									// Sub_40DC20, state 6 instead of 3)
	void Sub_4278E0(int nUnk);		// hover.exe:0x4278E0 (same body as
									// Sub_40DC20, state 7 instead of 3)
	void Sub_4279E0();				// hover.exe:0x4279E0
	// hover.exe:0x41F4E0 set the render state (m_dwUnk0500) and, when it
	// actually changed and nRedraw is set, force the window to repaint
	void Sub_41F4E0(DWORD dwState, int nRedraw);
	// hover.exe:0x413A00 stop the worker thread, apply the new speed/detail
	// level (theApp.m_dwUnk01C4) and restart it; called by the Customize
	// Game dialog when the speed/detail slider moved
	void Sub_413A00(int nLevel, int nUnk);
	// hover.exe:0x413B10 stop the worker thread and enter state 4 (same
	// body as Sub_40DC20); called after the player controls were changed
	void Sub_413B10(int nUnk);
	// hover.exe:0x41E5B0 reset the per-game HUD/scoreboard objects for a
	// new game
	void Sub_41E5B0();
	// hover.exe:0x4282F0 award the end-of-level score: time bonus, points
	// per remaining tick and (when bLevelBonus is set) the level bonus,
	// added into the document's running total and shown on the scoreboard
	void Sub_4282F0(DWORD bLevelBonus);
	// hover.exe:0x41F480 re-establish the offscreen buffers for the current
	// client size and restart the render thread (called by CMainFrame when
	// the frame has finished resizing)
	void Sub_41F480();

	// hover.exe:0x402250 per-frame render step: update the camera state,
	// run the scene pass, rasterize and present; called from the render
	// state dispatcher at hover.exe:0x41F012
	void Sub_402250();
	// hover.exe:0x407430 composite the offscreen buffers and present them
	// (BitBlt/StretchBlt into a CClientDC)
	void Sub_407430();
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
	void Sub_4184B0(int nWidth, int nHeight);
	// hover.exe:0x41ECE0 worker thread body for the render state machine
	void Sub_41ECE0();

	// Helper stubs called by the renderer setup / worker thread cluster.
	// hover.exe:0x4274B0 set the offscreen bitmap size from the client rect
	void Sub_4274B0(int nWidth, int nHeight, int nUnk);
	// hover.exe:0x410510 background-layer setup helper
	void Sub_410510();
	// hover.exe:0x410380 renderer setup continuation (stub)
	void Sub_410380();
	// hover.exe:0x428120 alternate renderer setup path (stub)
	void Sub_428120();
	// hover.exe:0x413C10 state 1 setup handler
	void Sub_413C10(int nUnk);
	// hover.exe:0x4126C0 timer start helper
	void Sub_4126C0();
	// hover.exe:0x427A80 state 5 setup handler
	void Sub_427A80(int nUnk);
	// hover.exe:0x427BB0 state 6 setup handler
	void Sub_427BB0();
	// hover.exe:0x427CC0 state 7 setup handler
	void Sub_427CC0();

	// hover.exe:0x40D340 frame-step state 1 handler (stub)
	void Sub_40D340();
	// hover.exe:0x40CCE0 frame-step state 2 handler: draw up to four
	// message strings (stub)
	void Sub_40CCE0(const char* pszA, const char* pszB, const char* pszC,
		const char* pszD);
	// hover.exe:0x40D0A0 helper used by Sub_40CCE0 to draw one shadowed
	// (non-stretch) or plain (stretch) text line.
	void Sub_40D0A0(RECT* pRect, const char* psz, COLORREF clrShadow,
		COLORREF clrText);
	// hover.exe:0x427F40 helper used by Sub_40CCE0 to draw a raised box
	// around the message rectangle.
	void Sub_427F40(RECT* pRect, int nWidth);
	// hover.exe:0x4283F0 frame-step state 3 handler (stub)
	void Sub_4283F0();

	// Worker-thread handshake variants of Sub_40DC20 (same body, different
	// target state for m_dwUnk00AC).
	// hover.exe:0x4275D0 stop the worker and enter state 5, recording nUnk
	// in m_dwUnk00B0
	void Sub_4275D0(int nUnk, int nHandshake);
	// hover.exe:0x4276E0 stop the worker and enter state 2
	void Sub_4276E0(int nUnk);
	// hover.exe:0x41F350 tear the renderer down: stop the worker thread,
	// close the six events, delete the render contexts and the render
	// target, and release the offscreen DC
	void Sub_41F350();
	// hover.exe:0x408D00 per-frame game step: run the countdown of the
	// timed game states and drive the per-frame object step
	void Sub_408D00();
	// hover.exe:0x408DF0 per-frame status-bar step: refresh the two frame
	// rates and the player position readout every 58 ticks
	void Sub_408DF0();
	// hover.exe:0x427D40 the per-frame object step itself: hand the frame
	// to the worker thread, step every object in the doc's second list,
	// reset the per-frame counters, translate the joystick buttons into
	// WM_KEYDOWNs and time the step
	void Sub_427D40();
	// hover.exe:0x427E50 call the input/think virtual (slot 7) on every
	// object in the document's second object list
	void Sub_427E50();
	// hover.exe:0x427E90 clear the four renderer debug counters and run
	// the collider's per-frame pass
	void Sub_427E90();

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
	afx_msg void OnCmd34014();				// hover.exe:0x428590
	afx_msg void OnCmd34015();				// hover.exe:0x428610
	afx_msg void OnCmd34013();				// hover.exe:0x428690
	afx_msg void OnCmd34018();				// hover.exe:0x428710
	afx_msg void OnCmd34019();				// hover.exe:0x428770
	afx_msg void OnCmd34017();				// hover.exe:0x4287D0
	afx_msg void OnCmd34016();				// hover.exe:0x428830
	afx_msg void OnGamePause();				// hover.exe:0x413C70
	afx_msg void OnUpdateGamePause(CCmdUI* pCmdUI);	// hover.exe:0x412860
	afx_msg void OnCmd34005();				// hover.exe:0x428970
	afx_msg void OnUpdateCmd34005(CCmdUI* pCmdUI);	// hover.exe:0x4289F0
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
											// hover.exe:0x40B540
	afx_msg void OnCmd34029();				// hover.exe:0x428A10
	afx_msg void OnUpdateCmd34029(CCmdUI* pCmdUI);	// hover.exe:0x428A40
	afx_msg void OnCmd34030();				// hover.exe:0x428A90
	afx_msg void OnUpdateCmd34030(CCmdUI* pCmdUI);	// hover.exe:0x428C00
	afx_msg void OnUpdateCmd34014(CCmdUI* pCmdUI);	// hover.exe:0x428C30
	afx_msg void OnUpdateCmd34015(CCmdUI* pCmdUI);	// hover.exe:0x428C60
	afx_msg void OnUpdateCmd34013(CCmdUI* pCmdUI);	// hover.exe:0x428C90
	afx_msg void OnUpdateCmd34018(CCmdUI* pCmdUI);	// hover.exe:0x428CC0
	afx_msg void OnUpdateCmd34019(CCmdUI* pCmdUI);	// hover.exe:0x428CF0
	afx_msg void OnUpdateCmd34017(CCmdUI* pCmdUI);	// hover.exe:0x428D20
	afx_msg void OnUpdateCmd34016(CCmdUI* pCmdUI);	// hover.exe:0x428D50
	afx_msg void OnCmd34032();				// hover.exe:0x429230
	afx_msg void OnUpdateCmd34032(CCmdUI* pCmdUI);	// hover.exe:0x4292F0
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);	// hover.exe:0x420DF0
	afx_msg void OnCmd34033();				// hover.exe:0x429340
	afx_msg void OnUpdateCmd34033(CCmdUI* pCmdUI);	// hover.exe:0x429360
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
// original: CHumanPlayer::Sub_42AD30 reads the joystick state fields
// (+0x6C..+0x78) directly through Hover_sub_401140's return value.
	DWORD m_dwUnk0034;		// +0x34 = 1
	DWORD m_dwUnk0038;		// +0x38
	DWORD m_dwUnk003C;		// +0x3C = 0
	DWORD m_dwUnk0040;		// +0x40 = 0
	DWORD m_dwUnk0044;		// +0x44 = 0
	DWORD m_dwUnk0048;		// +0x48 = 0
	HANDLE m_hUnk004C;		// +0x4C synchronization event handle
	HANDLE m_hUnk0050;		// +0x50 synchronization event handle
	HANDLE m_hUnk0054;		// +0x54 synchronization event handle
	HANDLE m_hUnk0058;		// +0x58 synchronization event handle
	HANDLE m_hUnk005C;		// +0x5C synchronization event handle
							// (+0x48/+0x4C are event handles too;
							// m_dwUnk0048 is waited on/signaled with them)
	DWORD m_dwUnk0060;		// +0x60 = 0
	DWORD m_dwUnk0064;		// +0x64 = 0
	DWORD m_dwUnk0068;		// +0x68 = 0
	DWORD m_dwUnk006C;		// +0x6C = 0
	DWORD m_dwUnk0070;		// +0x70 = 0
	DWORD m_dwUnk0074;		// +0x74 = 0
	DWORD m_dwUnk0078;		// +0x78 = 0
	DWORD m_dwUnk007C;		// +0x7C
	DWORD m_dwUnk0080;		// +0x80 = 0
	DWORD m_dwUnk0084;		// +0x84 = 0
	DWORD m_dwUnk0088;		// +0x88 set to 1 by CBumperView::Sub_4274B0
	DWORD m_dwUnk008C;		// +0x8C offscreen width (OnDraw blit size)
	DWORD m_dwUnk0090;		// +0x90 offscreen height
	// The four dwords at +0x94..+0xA0 are accessed both as individual
	// flags/values and as a RECT by Sub_40CCE0.
	union
	{
		struct
		{
			DWORD m_dwUnk0094;		// +0x94
			DWORD m_dwUnk0098;		// +0x98
			DWORD m_dwUnk009C;		// +0x9C
			DWORD m_dwUnk00A0;		// +0xA0
		};
		RECT m_rcUnk0094;		// +0x94 aliased rect used by Sub_40CCE0
	};
	DWORD m_dwUnk00A4;		// +0xA4 = 0
	DWORD m_dwUnk00A8;		// +0xA8 = 0
	DWORD m_dwUnk00AC;		// +0xAC = 4
	DWORD m_dwUnk00B0;		// +0xB0
	CDC m_dcUnk00B4;		// +0xB4
	DWORD m_dwUnk00C4;		// +0xC4
	CSub410F20* m_pUnk00C8;	// +0xC8 render context (camera/shade state)
	CSub410F20* m_pUnk00CC;	// +0xCC second render context (used when
							// m_dwUnk021C selects the alternate shade table)
	CSub406090* m_pUnk00D0;	// +0xD0 offscreen render target (span rasterizer)
	DWORD m_dwUnk00D4;		// +0xD4 camera x (from the human player)
	DWORD m_dwUnk00D8;		// +0xD8 camera y
	DWORD m_dwUnk00DC;		// +0xDC camera angle
	DWORD m_dwUnk00E0;		// +0xE0 camera z (player +0x8C + 0x28)
	DWORD m_dwUnk00E4;		// +0xE4 = g_adwUnk_4C4CC8[theApp.m_dwUnk01C4]
	CSub423030 m_obj00E8;	// +0xE8
	CSub422F50 m_obj0150;	// +0x150
	DWORD m_dwUnk021C;		// +0x21C = 0
	CRect m_rcUnk0220;		// +0x220 present blit rect (Sub_407430)
	CRect m_rcUnk0230;		// +0x230 second present blit rect (used when
							// theApp.m_dwUnk01D0 is set)
	// The original constructor calls each of these member ctors
	// individually (no ??_L vector-constructor loop), so they are
	// declared as separate members rather than arrays.
	CSub422BD0 m_obj0240;		// +0x240
	CSub422BD0 m_obj0268;		// +0x268
	CSub422BD0 m_obj0290;		// +0x290
	CSub422BD0 m_obj02B8;		// +0x2B8
	CSub422C80 m_obj02E0;		// +0x2E0
	CSub422C80 m_obj0310;		// +0x310
	CSub422C80 m_obj0340;		// +0x340
	CSub422C80 m_obj0370;		// +0x370
	CSub422C80 m_obj03A0;		// +0x3A0
	CSub422C80 m_obj03D0;		// +0x3D0
	CSub422C80 m_obj0400;		// +0x400
	CSub422C30 m_obj0430;		// +0x430
	CSub422C30 m_obj0480;		// +0x480
	CPen m_pen04D0;			// +0x4D0
	CPen m_pen04D8;			// +0x4D8
	CBrush m_brush04E0;		// +0x4E0
	CBrush m_brush04E8;		// +0x4E8
	CBrush m_brush04F0;		// +0x4F0
	CBrush m_brush04F8;		// +0x4F8
	DWORD m_dwUnk0500;		// +0x500 = 2
	DWORD m_dwUnk0504;		// +0x504 = 0
	DWORD m_dwUnk0508;		// +0x508 = 0
	DWORD m_dwUnk050C;		// +0x50C = 0
	DWORD m_dwUnk0510;		// +0x510 = 0
	DWORD m_dwUnk0514;		// +0x514 = 0
	DWORD m_dwUnk0518;		// +0x518 = 0
	DWORD m_dwUnk051C;		// +0x51C = 0
	DWORD m_dwUnk0520;		// +0x520 = 0
	DWORD m_dwUnk0524;		// +0x524 = 0
	DWORD m_dwUnk0528;		// +0x528
	DWORD m_dwUnk052C;		// +0x52C last presented frame's GetTickCount
	DWORD m_dwUnk0530;		// +0x530 presented frame counter
	DWORD m_dwUnk0534;		// +0x534 tick of the last status-bar refresh
							//   (game-step rate base, Sub_408DF0)
	DWORD m_dwUnk0538;		// +0x538 last game step's GetTickCount
							//   (Sub_427D40)
	DWORD m_dwUnk053C;		// +0x53C game step counter (Sub_427D40)
	DWORD m_dwUnk0540;		// +0x540 = 0
	DWORD m_dwUnk0544;		// +0x544 = 0
	char m_pad0548[0x14];	// +0x548
};							// sizeof = 0x55C in hover.exe

/////////////////////////////////////////////////////////////////////////////
// Globals defined in bumpview.cpp

extern DWORD g_adwUnk_4C4CC8[6];	// hover.exe:0x4C4CC8
extern CPalette g_paletteUnk_4A1D50;	// hover.exe:0x4A1D50 palette selected
									// into the paint DC by OnDraw

extern DWORD g_dwUnk_460998;		// hover.exe:0x460998 zone descriptor index
									// used by the Sub_407430 present
extern DWORD g_dwUnk_4C4CA8;		// hover.exe:0x4C4CA8 selects the present
									// path in Sub_407430

extern int g_nUnk_4C4CEC;			// hover.exe:0x4C4CEC = 0x39 down-counter
									// that paces the Sub_408DF0 status-bar
									// refresh
extern DWORD g_dwUnk_4C4CF0;		// hover.exe:0x4C4CF0 previous joystick
									// button mask (Sub_427D40)

// Viewport/projection state computed per frame by CSub422F50::Sub_407160
// and consumed by Sub_404BB0/Sub_4055B0
extern DWORD g_dwUnk_4A4790;		// hover.exe:0x4A4790 camera x
extern DWORD g_dwUnk_4A4794;		// hover.exe:0x4A4794 camera y
extern DWORD g_dwUnk_4A4798;		// hover.exe:0x4A4798 viewport right
extern DWORD g_dwUnk_4A479C;		// hover.exe:0x4A479C viewport bottom
extern DWORD g_dwUnk_4A47B0;		// hover.exe:0x4A47B0 world object +0x1C
extern DWORD g_dwUnk_4A47B8;		// hover.exe:0x4A47B8 camera angle - 0x80
extern DWORD g_dwUnk_4A47BC;		// hover.exe:0x4A47BC viewport left
extern DWORD g_dwUnk_4A47D8;		// hover.exe:0x4A47D8 viewport top
extern DWORD g_dwUnk_4A47E0;		// hover.exe:0x4A47E0 projection term x
extern DWORD g_dwUnk_4A47E4;		// hover.exe:0x4A47E4 projection term y

// Free helpers called by the renderer setup cluster.
// hover.exe:0x453352 MFC-style FindResource wrapper, also used by the
// splash dialog's DIB loader (splashdlg.cpp)
HRSRC __stdcall Hover_sub_453352(HINSTANCE hInst, LPCSTR lpName,
	LPCSTR lpType);
// hover.exe:0x408780 camera/shade table update helper
void Hover_sub_408780();
// hover.exe:0x418A30 look up a texture by name in g_mapUnk_4A1D30 (NULL if absent)
CObject* Hover_sub_418A30(const char* pszName);
// hover.exe:0x408AA0 multimedia timer callback used by CBumperView::Sub_4126C0
void CALLBACK Hover_sub_408AA0(UINT uID, UINT uMsg, DWORD dwUser,
	DWORD dw1, DWORD dw2);
#endif // __BUMPVIEW_H__
