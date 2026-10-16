// texheap.h : interface of the fixed-heap span renderer
//
// Skeleton reconstructed from hover.exe: the renderer. Three objects are
// involved:
//   CSpanRasterizer  the span rasterizer and offscreen render target (drives
//               the whole pipeline; methods in bumpview.cpp and
//               texheap.cpp)
//   CFlatRunHeap  its span heap ("floor heap") allocating span-run nodes
//               from its own fixed arena
//   CRenderContext  the render context (camera, frame buffer, shade tables and
//               the span drawer function pointers); the one in use is
//               g_pRenderCtx (methods in texload.cpp)
// The span arena at 0x4A1D80 (size 0x2328, end 0x4A40A8) is shared by the
// 0x18-byte HEAPNODEs built by CSpanRasterizer::LinkFlatEdges and the 0x14-byte
// SPANENTRY point spans built by CSpanRasterizer::RenderColumn; the "Trashing
// Heap" debug warning fires when the arena cursor passes the end.
//
// Note on member offsets: the comments give each member's offset within
// the class/struct in hover.exe.
//
// This header requires hover.h, texture.h and gameobj.h to be included first.

#ifndef __TEXHEAP_H__
#define __TEXHEAP_H__

/////////////////////////////////////////////////////////////////////////////
// tagTEXRUN: one 8-byte texture point (consumed by the span builder)

typedef struct tagTEXRUN
{
	WORD m_wTop;		// +0x00
	WORD m_wBottom;		// +0x02
	BYTE* m_pPixels;	// +0x04 data (copied into the span entry)
} TEXRUN;				// sizeof = 0x8

/////////////////////////////////////////////////////////////////////////////
// tagTEXCOLUMN: point array header inside a MERLIN_TEXFRAME's point data

typedef struct tagTEXCOLUMN
{
	WORD m_wRunCount;		// +0x00 point count
	WORD m_wUnk02;		// +0x02
	TEXRUN* m_pRuns;	// +0x04 point array
} TEXCOLUMN;				// sizeof = 0x8

/////////////////////////////////////////////////////////////////////////////
// tagSPANENTRY: one 0x14-byte span endpoint, built in the fixed arena by
// CSpanRasterizer::RenderColumn; also embedded in RENDEROBJ at +0xAC

typedef struct tagSPANENTRY
{
	DWORD m_dwTopZ;	// +0x00 span start
	DWORD m_dwBottomZ;	// +0x04 span end
	DWORD m_dwTopY;	// +0x08 scaled span start
	DWORD m_dwBottomY;	// +0x0C scaled span end
	BYTE* m_pPixels;	// +0x10 point data
} SPANENTRY;			// sizeof = 0x14

/////////////////////////////////////////////////////////////////////////////
// tagRENDEROBJ: one 0xC0-byte render object. Lives in three doubly-linked
// lists (m_pPoolPrev/m_pPoolNext at g_pObjPool, m_pColPrev/m_pColNext at
// g_pColumnObjs and m_pActiveNext/m_pActivePrev at g_pActiveObjs).

typedef struct tagRENDEROBJ
{
	// hover.exe:0x419520 (empty; CSpanRasterizer::AllocBuffers calls it once
	// per object of the pool it allocates)
	tagRENDEROBJ();

	DWORD m_dwUnk00;		// +0x00
	DWORD m_dwUnk04;		// +0x04
	WORD m_wDistance;			// +0x08 distance (sort key, projection divisor)
	WORD m_wTexU;			// +0x0A
	DWORD m_bSprite;		// +0x0C nonzero: sprite object (m_pSource is a
							//   CMerlinDynamic); zero: maze face (m_pSource
							//   is a CMerlinStatic)
	CMerlinLine* m_pSource;	// +0x10 the source object
	DWORD m_bFrontFace;		// +0x14 face select (computed)
	DWORD m_bHidden;		// +0x18 visibility flag
	WORD m_wBspOrder;			// +0x1C sort key (signed; span y)
	WORD m_wUnk1E;			// +0x1E
	DWORD m_bCovered;		// +0x20
	CMerlinLine* m_pSegment;		// +0x24 the sprite's segment (second lookup key)
	tagRENDEROBJ* m_pPoolPrev;	// +0x28 prev (g_pObjPool list)
	tagRENDEROBJ* m_pPoolNext;	// +0x2C next (g_pObjPool list)
	tagRENDEROBJ* m_pColPrev;	// +0x30 prev (g_pColumnObjs list)
	tagRENDEROBJ* m_pColNext;	// +0x34 next (g_pColumnObjs list)
	tagRENDEROBJ* m_pActiveNext;	// +0x38 next (active list)
	tagRENDEROBJ* m_pActivePrev;	// +0x3C prev (active list)
	tagRENDEROBJ* m_pTopNeighbour;	// +0x40 merged object (-1 = list end)
	DWORD m_bTopAboveEye;		// +0x44
	CMerlinTexture* m_pTopFlatTex;		// +0x48 the CMerlinTexture of the upper span run
							// (AddFlatRun's dwB, keyed with m_dwTopZ)
	tagRENDEROBJ* m_pBottomNeighbour;	// +0x4C merged object (-1 = list end)
	DWORD m_bBottomAboveEye;		// +0x50
	CMerlinTexture* m_pBottomFlatTex;		// +0x54 the CMerlinTexture of the lower span run
							// (AddFlatRun's dwB, keyed with m_dwBottomZ)
	DWORD m_bVisible;		// +0x58 visible this frame
	CMerlinTexture* m_pTex;	// +0x5C
	DWORD m_dwTopZ;		// +0x60 span y
	DWORD m_dwBottomZ;		// +0x64
	DWORD m_dwHeight16;		// +0x68 (y delta) << 16
	DWORD m_dwTopDZ;		// +0x6C eye z - top z
	DWORD m_dwBottomDZ;		// +0x70 eye z - bottom z
	DWORD m_dwMinY;		// +0x74 span start (computed by RenderColumn)
	DWORD m_dwMaxY;		// +0x78 span end
	DWORD m_dwUnk7C;		// +0x7C
	DWORD m_dwUnk80;		// +0x80
	DWORD* m_pTopEdgeY;		// +0x84 left edge value pointer
	DWORD* m_pBottomEdgeY;		// +0x88 right edge value pointer
	DWORD m_bHasFlats;		// +0x8C edges valid
	DWORD m_dwRunCount;		// +0x90 point count
	DWORD m_dwVMask;		// +0x94 texture v wrap mask (16.16)
	DWORD m_dwVStep;		// +0x98 texture v step
	DWORD m_dwTexScaleLog2;		// +0x9C fixed-point shift
	DWORD m_bMasked;		// +0xA0 span mode
	DWORD m_bMappingCached;		// +0xA4 mapping values valid
	SPANENTRY* m_pRuns;	// +0xA8 point spans (fixed arena)
	SPANENTRY m_run;	// +0xAC embedded span (m_dwTopY/m_dwBottomY are
							// the +0xB4/+0xB8 limits used by CoverRows)
} RENDEROBJ;				// sizeof = 0xC0

/////////////////////////////////////////////////////////////////////////////
// tagEDGENODE: one 0x18-byte node of the g_pEdgeList list, allocated in
// the span arena by CSpanRasterizer::LinkFlatEdges

typedef struct tagEDGENODE
{
	RENDEROBJ* m_pObj;	// +0x00 owning render object
	CMerlinTexture* m_pFlatTex;	// +0x04 the texture of the flat at this edge
	tagEDGENODE* m_pNext;	// +0x08 next
	tagEDGENODE* m_pPrev;	// +0x0C prev
	DWORD m_bTopEdge;		// +0x10 edge selector
	DWORD m_dwEdgeZ;		// +0x14 edge y
} EDGENODE;					// sizeof = 0x18

/////////////////////////////////////////////////////////////////////////////
// tagFLATNODE: span-run node allocated inside the CFlatRunHeap heap (the
// 0x2C header is followed in place by two 0xA0-byte word-pair buffers and
// the m_dwRows-word index array)

typedef struct tagFLATNODE
{
	WORD* m_pRunsA;			// +0x00 word-pair buffer A
	WORD* m_pRunsB;			// +0x04 word-pair buffer B
	WORD** m_ppPrevRuns;		// +0x08 current read buffer slot
	WORD** m_ppCurRuns;		// +0x0C current write buffer slot
	WORD* m_pRowStartCol;			// +0x10 word index array
	DWORD m_dwCurRuns;		// +0x14 written span count
	DWORD m_dwPrevRuns;		// +0x18 accumulated span count
	DWORD m_dwHeight;		// +0x1C span y
	DWORD m_dwKey;		// +0x20 key ((y << 0x12) + texture)
	CMerlinTexture* m_pTex;		// +0x24 the run's CMerlinTexture (AddFlatRun's
							// second argument; FlushRuns hands it to
							// DrawFlatRow as pTex)
	tagFLATNODE* m_pNext;	// +0x28 next
} FLATNODE;					// sizeof = 0x2C (header only)

/////////////////////////////////////////////////////////////////////////////
// Span drawers: the hand-written __asm routines in spandrawA/B/C.cpp that
// CRenderContext::SetMode installs for the current render mode. They take
// no arguments; everything they need is in the g_*_4A4A** globals.

typedef void (*SPANFN)();

class CSpanRasterizer;

/////////////////////////////////////////////////////////////////////////////
// CRenderContext: the render context (camera, viewport, shade tables and the
// span drawers for the current mode). CBumperView owns two (m_pViewCtx and
// m_pMirrorCtx); the one in use for a frame is g_pRenderCtx.

// The 8-byte sine/cosine table entries of CRenderContext::m_pRayDirs, one
// per screen column over 7/6 of a turn. They are built with
// `new SINCOSENTRY[n]`, so the original carries the compiler's vector-new
// expansion (null check, backwards constructor loop) and an empty
// __thiscall constructor of its own at hover.exe:0x419B20.

struct SINCOSENTRY
{
	SINCOSENTRY();

	int nCos;
	int nSin;
};

#define ROWDIST_ENTRIES 0x7E9		// m_pRowDistance entries

class CRenderContext
{
public:
	// hover.exe:0x420ED0 allocates the span-drawer function table
	// (m_ppfnDrawFlatByShift) and the shift-count table (m_pMipShift)
	CRenderContext();
	// hover.exe:0x420F60 frees the eight tables the context owns
	~CRenderContext();
	// hover.exe:0x410F20 install the span drawers for dwMode (1..6) and
	// the two option bits in dwFlags
	void SetMode(DWORD dwFlags, DWORD dwMode);
	// hover.exe:0x406C00 set the camera/projection state
	void SetCamera(DWORD dwA, DWORD dwB, DWORD dwC, DWORD dwD);
	// hover.exe:0x410EC0
	void BuildMipShiftTable();
	// hover.exe:0x414A10
	void SetClipSpans(void* pA, void* pB);
	// hover.exe:0x419940 set the render mode and the backdrop texture
	void SetSkyTexture(DWORD dwMode, CMerlinTexture* pTexture);
	// hover.exe:0x419530 render-target palette/shade setup
	void SetViewport(CSpanRasterizer* pTarget, CPalette* pPal, int nA, int nB,
		int nWidth, int nHeight);
	// hover.exe:0x419960 helper used by SetViewport to fill m_pSkyGradient
	void BuildSkyGradient(int nBase, BYTE bRed, BYTE bGreen, BYTE bBlue,
		CPalette* pPalette);
	// hover.exe:0x403850 the shade table row for scanline dwY
	DWORD* GetShadeRow(DWORD dwY);

	SPANFN m_pfnDrawWall;			// +0x00 wall column drawer (DrawWallRun)
	SPANFN m_pfnDrawSky;			// +0x04 sky/direct run drawer (DrawSkyRow)
	SPANFN* m_ppfnDrawFlatByShift;		// +0x08 17-slot drawer table indexed by
								// texture shift (g_dwVShift; slots
								// 6..15 used), used when m_bUseShiftTable
	SPANFN m_pfnDrawFlat;			// +0x0C variable-shift floor/ceiling drawer
	char m_pad10[0x4];			// +0x10
	BYTE* m_pMipShift;				// +0x14 -> table filled by BuildMipShiftTable
	DWORD m_dwViewX;			// +0x18 viewport X origin
	DWORD m_dwViewY;			// +0x1C viewport Y origin
	DWORD m_dwViewW;			// +0x20 viewport width
	DWORD m_dwViewH;			// +0x24 viewport height
	BYTE* m_pScanOrigin;			// +0x28 frame buffer base (render target
								// byte offset)
	DWORD m_dwHorizon;			// +0x2C viewport height / 2
	DWORD* m_pColumnScale;			// +0x30 -> per-column lookup table
	DWORD* m_pClipTop;			// +0x34 span array (top)
	DWORD* m_pClipBottom;			// +0x38 span array (bottom)
	DWORD m_dwPixelW;			// +0x3C screen pixels per texel across
	DWORD m_dwPixelH;			// +0x40 screen rows per texel
	DWORD m_dwMode;			// +0x44 = dwMode
	DWORD m_dwCamX;			// +0x48 camera x (16.16)
	DWORD m_dwCamY;			// +0x4C camera y
	DWORD m_dwCamZ;			// +0x50 camera z
	DWORD m_dwCamAngle;			// +0x54 camera angle
	DWORD m_dwUnk58;			// +0x58 = 0x80 (constructor default)
	DWORD* m_pRowDistance;			// +0x5C distance lookup table
								// (ROWDIST_ENTRIES)
	DWORD* m_pShade;			// +0x60 shade tables (one of CBumperDoc's
								// two; 64-DWORD rows picked by GetShadeRow)
	SINCOSENTRY* m_pRayDirs;		// +0x64 sin/cos lookup table
	DWORD m_bMirror;			// +0x68 nonzero: reverse the span-column
								//   order (CSpanRasterizer::RenderWorld)
	DWORD* m_pSkyGradient;			// +0x6C vertical shade table (the span
								// drawers read the low byte)
	CMerlinTexture* m_pSkyTex;	// +0x70 backdrop texture
	DWORD m_dwSkyMode;			// +0x74 render mode (1 = flat fill)
	DWORD m_dwTargetPitch;			// +0x78 render target width
	DWORD m_dwRowStride;			// +0x7C render target width * target->m_dwRowDir
	DWORD m_bUseShiftTable;			// +0x80 = (dwFlags & 2) >> 1: use the
								// m_ppfnDrawFlatByShift table
	DWORD m_dwTargetHeight;			// +0x84 render target height
	DWORD m_dwColumns;			// +0x88 span array count
	DWORD m_dwRows;			// +0x8C shade table parameter
	DWORD m_bLit;			// +0x90 = dwFlags & 1
								// +0x94 padding before the doubles
	double m_dblUnk98;			// +0x98 = 64.0 (constructor default)
	double m_dblUnkA0;			// +0xA0 = 64.0 (constructor default)
	DWORD m_dwLastCamZ;			// +0xA8 last camera z
};								// sizeof == 0xB0 (the size the original
								// passes to operator new)

/////////////////////////////////////////////////////////////////////////////
// CFlatRunHeap: the span heap ("floor heap"). Allocates SPANNODEs from its
// own fixed arena (m_pHeapBase base, m_pHeapCursor cursor; the "Blowing floor
// heap" debug warning fires when a new node would pass the end). Owned by
// the rasterizer (CSpanRasterizer::m_pRunHeap).

class CFlatRunHeap
{
public:
	// hover.exe:0x41B870 construct from the parent render target
	CFlatRunHeap(CSpanRasterizer* pParent);
	// hover.exe:0x41B8C0 destroy the arena
	~CFlatRunHeap();

	// hover.exe:0x406C90 reset the arena: build the first FLATNODE chain
	// in the m_pHeapBase buffer
	void ResetHeap();
	// hover.exe:0x403980 find-or-create the span-run node for the
	// (scanline dwA, texture dwB) key and append the x pair dwC/dwD to it
	void AddFlatRun(DWORD dwA, CMerlinTexture* pTex, DWORD dwC, DWORD dwD);
	// hover.exe:0x403A60 merge the accumulated span runs and emit them
	// through the rasterizer
	void FlushRuns(CSpanRasterizer* pRender);
	// hover.exe:0x406D00 append one word pair to the current node
	void AddSkyRun(DWORD dwA, DWORD dwB);

	DWORD m_dwRows;		// +0x00 index words per node (= target height)
	WORD* m_pScratchRuns;			// +0x04 merge scratch buffer (word pairs)
	FLATNODE* m_pSkyNode;		// +0x08 current span node
	FLATNODE* m_pFlatHead;		// +0x0C span-run list head
	FLATNODE* m_pFlatTail;		// +0x10 span-run list tail
	DWORD m_dwFlatNodes;		// +0x14 span-run node count
	BYTE* m_pHeapBase;			// +0x18 heap base
	BYTE* m_pHeapCursor;			// +0x1C allocation cursor
};							// sizeof = 0x20

/////////////////////////////////////////////////////////////////////////////
// CSpanRasterizer: the span rasterizer and its offscreen render target, pointed
// to by CBumperView::m_pRasterizer.
//
// The object RenderWorld takes as its first argument (CBumperDoc::
// m_pWorld) is the level's CMerlinWorld: the sprite array it walks is
// m_arrSprites (m_pData at +0x2C, m_nSize at +0x30), the BSP node array is
// m_arrNodes (m_pData at +0x54, node 0 is the root) and the face table
// WalkBSP indexes with the node's m_wFace is m_arrStatics (m_pData at
// +0x18).

// tagSPANLINK: one node of a per-column span list (the m_pColumnLists array
// elements consumed by RenderWorld's column loop).

typedef struct tagSPANLINK
{
	WORD m_wOrder;				// +0x00
	WORD m_wUnk02;				// +0x02
	DWORD m_bOpen;			// +0x04 nonzero: 4-arg textured drawer
	DWORD m_bSprite;			// +0x08
	CMerlinLine* m_pObj;			// +0x0C
	CMerlinLine* m_pSegment;			// +0x10
	tagSPANLINK* m_pNext;		// +0x14 next
} SPANLINK;						// sizeof = 0x18

class CMerlinWorld;

class CSpanRasterizer
{
public:
	// hover.exe:0x4191B0 construct an offscreen render target of the given
	// dimensions for the level world pWorld.
	CSpanRasterizer(int nWidth, int nHeight, CMerlinWorld* pWorld);
	// hover.exe:0x4193A0 destroy the offscreen render target and its buffers.
	~CSpanRasterizer();

	// hover.exe:0x406090 rasterize the world through the render context
	// pCtx: rebuild the sprite facing segments, then walk the span columns
	void RenderWorld(CMerlinWorld* pWorld, CRenderContext* pCtx);

	// hover.exe:0x419440 create the DIB section for the render target using
	// the supplied palette and store the pixel bits pointer at +0x04.
	void CreateTargetDIB(CPalette* pPalette);
	// hover.exe:0x4192F0 allocate the span buffers for pWorld's BSP.
	void AllocBuffers(CMerlinWorld* pWorld);

	// Sprite-pass helpers called by RenderWorld:
	// hover.exe:0x405B90 test a segment against the view (called with
	// both CMerlinDynamic sprites and CMerlinBSP nodes)
	int IsSegmentVisible(CMerlinLine* pLine);
	// hover.exe:0x405CE0 submit a span pair (returns 0 on rejection).
	// pA is the object whose texture links are advanced -- a
	// CMerlinDynamic when nC != 0, a CMerlinStatic when nC == 0 -- and pB
	// supplies the segment.
	int SubmitSegment(CMerlinLine* pA, CMerlinLine* pB, int nC, WORD wOrd);
	// hover.exe:0x405FA0 walk the maze BSP tree back-to-front, submitting
	// the visible faces with SubmitSegment
	void WalkBSP(short nNode);

	// Span pass (texheap.cpp):
	// hover.exe:0x402E00 rebuild the span lists (when m_bRelink), compute
	// the per-object span mapping values and render the frame
	void RenderColumn(DWORD dwUnk);
	// hover.exe:0x4030F0 rebuild the g_pEdgeList edge list and the
	// active render-object list from g_pObjPool/g_pColumnObjs
	void LinkFlatEdges();
	// hover.exe:0x402630 clip an object's span window against the current
	// clip range, queuing the off-window parts into the span heap
	void DrawWallWithFlats(RENDEROBJ* pObj, DWORD dwA, DWORD dwB);
	// hover.exe:0x402990 emit one flat-mapped span pair
	void DrawWallRun(RENDEROBJ* pObj, SPANENTRY* pSpan, DWORD dwA, DWORD dwB);
	// hover.exe:0x402B10 emit one floor/ceiling textured span run
	void DrawFlatRow(DWORD dwA, DWORD dwB, CMerlinTexture* pTex, DWORD dwY);
	// hover.exe:0x404030 walk the active list covering [dwA, dwB] with
	// spans (recursive; gaps go to the span heap's AddSkyRun)
	void CoverRows(RENDEROBJ* pObj, DWORD dwA, DWORD dwB);
	// hover.exe:0x4065F0 right-edge variant of the DrawWallWithFlats clip test
	int ClipBottomFlat(RENDEROBJ* pObj, SPANENTRY* pSpan, DWORD dwC, DWORD dwD,
		DWORD dwE);
	// hover.exe:0x4066B0 emit one span from a precomputed point span
	void DrawMaskedRun(RENDEROBJ* pObj, SPANENTRY* pSpan, DWORD dwA, DWORD dwB);
	// hover.exe:0x406740 emit one vertical span run (flat fill when the
	// view is in mode 1)
	void DrawSkyRow(DWORD dwA, DWORD dwB);

	CFlatRunHeap* m_pRunHeap;	// +0x00 span buffer object
	BYTE* m_pBits;		// +0x04 DIB section pixel bits pointer (used as the
						//   rasterizer destination base)
	char m_pad08[0x4];		// +0x08
	CBitmap m_bitmap;		// +0x0C DIB section bitmap (m_hObject at +0x10)
	DWORD m_dwWidth;		// +0x14 render target width (aligned to 4)
	DWORD m_dwHeight;		// +0x18 render target height
	DWORD m_dwRowDir;		// +0x1C
	CMerlinWorld* m_pWorld;	// +0x20 the level world for this frame
	DWORD m_bMipMapping;		// +0x24
	DWORD m_bRelink;		// +0x28 render flags accumulated over the frame
	DWORD m_dwNegWidth;		// +0x2C negated width
	DWORD m_dwColumnsPerAngle;		// +0x30 (screen width << 16) / 0x55
	DWORD m_dwLeftEdgeAngle;		// +0x34 camera angle << 16, biased/wrapped
	DWORD m_dwUnk38;		// +0x38
	SPANLINK* m_pLinkPool;		// +0x3C allocated span buffer
	SPANLINK** m_pColumnLists;	// +0x40 allocated span buffer (column lists)
	RENDEROBJ* m_pRenderObjPool;	// +0x44 render-object pool (AllocBuffers)
};							// sizeof = 0x48


/////////////////////////////////////////////////////////////////////////////
// hover.exe:0x406A80 insert pObj's chain into the active list at its
// sorted position (recursive)
void Hover_SortBehindNeighbours(RENDEROBJ* pObj);

/////////////////////////////////////////////////////////////////////////////
// Globals defined in texheap.cpp

extern BYTE* g_pArenaCursor;		// hover.exe:0x4A1D60 span arena cursor
extern DWORD g_dwColTopY;	// hover.exe:0x4A1D64 span window start
extern CRenderContext* g_pRenderCtx;	// hover.exe:0x4A1D68 render context in use
extern DWORD g_dwColProjScale;	// hover.exe:0x4A1D6C span scale numerator
extern DWORD g_dwHorizonY;	// hover.exe:0x4A1D70 span origin offset
extern DWORD g_dwFlatMipLevel;	// hover.exe:0x4A1D74 texture mip level
extern DWORD g_bWallMipMapping;	// hover.exe:0x4A1D78 angle->mip LUT enable
#define SPAN_ARENA_SIZE 0x2328	// 0x4A1D80..0x4A40A8
extern BYTE g_abSpanArena[SPAN_ARENA_SIZE];	// hover.exe:0x4A1D80 span arena
extern EDGENODE* g_pEdgeList;	// hover.exe:0x4A4490 edge list head
extern CMerlinLine g_lineFrustumLeft;	// hover.exe:0x4A4498 phase camera
								// line (CSpanRasterizer::RenderWorld)
extern CMerlinLine g_lineFrustumRight;	// hover.exe:0x4A44E0 right-edge
								// phase camera line
extern DWORD g_dwClipTopY;	// hover.exe:0x4A4524 clip window start
extern WORD g_wBspOrder;		// hover.exe:0x4A4528
extern DWORD* g_pdwDistTable;	// hover.exe:0x4A452C scanline scale table
extern DWORD g_dwBelowClipY;	// hover.exe:0x4A4530 low edge default
extern DWORD g_dwCurColumn;	// hover.exe:0x4A4534 fill color
extern DWORD g_dwLogRowStep;	// hover.exe:0x4A4538 log2 of g_dwRowStep
extern DWORD g_dwRowStep;	// hover.exe:0x4A453C rows per texel
extern DWORD g_dwColumnOrigin;	// hover.exe:0x4A4540 frame buffer origin
extern DWORD g_dwEyeZ;	// hover.exe:0x4A4544 camera (eye) height
extern BYTE* g_pbMipLut;	// hover.exe:0x4A4548 angle->mip LUT
extern DWORD g_dwViewWidth;	// hover.exe:0x4A454C screen width
extern SPANLINK* g_pSpanLinkCursor;	// hover.exe:0x4A4550 next free node in
								// m_pLinkPool
extern DWORD g_dwLogColStep;	// hover.exe:0x4A4554 texture shift count
								// (read as a byte by the span drawers)
extern DWORD g_dwColStep;	// hover.exe:0x4A455C pixels per texel across
extern DWORD g_dwClipBottomY;	// hover.exe:0x4A4564 clip window end
extern DWORD g_dwAboveClipY;	// hover.exe:0x4A4568 high edge default
extern DWORD g_dwColBottomY;	// hover.exe:0x4A456C span window end
extern RENDEROBJ* g_pObjPool;	// hover.exe:0x4A4714 render-object pool list
extern RENDEROBJ* g_pObjArenaCursor;	// hover.exe:0x4A4718 render-object pool
								// cursor (CSpanRasterizer::m_pRenderObjPool)
extern RENDEROBJ* g_pColumnObjs;	// hover.exe:0x4A471C static list head
extern RENDEROBJ* g_pActiveObjs;	// hover.exe:0x4A4720 active list head
extern DWORD g_dwSpanCount;	// hover.exe:0x4A4A14 span length
extern DWORD g_dwSkyU;	// hover.exe:0x4A4A18
extern DWORD g_dwTexels;	// hover.exe:0x4A4A1C texture pixel pointer
extern DWORD g_dwUMask;	// hover.exe:0x4A4A20
extern DWORD g_dwVStep;	// hover.exe:0x4A4A24 texture v step
extern DWORD g_dwU;	// hover.exe:0x4A4A28 texture u origin
extern DWORD g_dwV;	// hover.exe:0x4A4A2C texture v origin
extern DWORD g_dwDstRowStride;	// hover.exe:0x4A4A30 render target
								// width * m_dwRowDir
extern DWORD g_dwRowPitch;	// hover.exe:0x4A4A34 frame buffer pitch
extern DWORD g_dwVMask;	// hover.exe:0x4A4A38
extern DWORD g_dwShadeTable;	// hover.exe:0x4A4A3C current 256-byte shade row
extern DWORD g_dwColumnV;	// hover.exe:0x4A4A40
extern DWORD g_dwSpanDepth;	// hover.exe:0x4A4A44 span scale
extern DWORD g_dwVShift;	// hover.exe:0x4A4A48 drawer selector
extern DWORD g_dwDst;	// hover.exe:0x4A4A4C destination pointer
extern DWORD g_dwUStep;	// hover.exe:0x4A4A50 texture u step (the wall v step)
extern DWORD g_dwNoEdgeY;	// hover.exe:0x4C5208 = 0xFFF0000 (edge
								// sentinel dereferenced through
								// RENDEROBJ::m_pTopEdgeY/m_pBottomEdgeY)

/////////////////////////////////////////////////////////////////////////////

#endif // __TEXHEAP_H__
