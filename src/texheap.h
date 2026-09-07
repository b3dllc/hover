// texheap.h : interface of the fixed-heap span renderer
//
// Skeleton reconstructed from hover.exe: the "span heap" cluster that
// turns the render-object lists into textured spans. Three objects are
// involved:
//   CSub402E00  the span renderer (drives the whole pipeline)
//   CSub403980  the span heap ("floor heap") allocating span-run nodes
//               from its own fixed arena
//   CSub403850  the view/raster state object at g_pUnk_4A1D68 (camera,
//               frame buffer and the span drawer function pointers)
// The span arena at 0x4A1D80 (size 0x2328, end 0x4A40A8) is shared by the
// 0x18-byte HEAPNODEs built by CSub402E00::Sub_4030F0 and the 0x14-byte
// SPANENTRY point spans built by CSub402E00::Sub_402E00; the "Trashing
// Heap" debug warning fires when the arena cursor passes the end.
//
// Note on member offsets: the comments give each member's offset within
// the class/struct in hover.exe.
//
// This header requires hover.h, texture.h and gameobj.h to be included first.

#ifndef __TEXHEAP_H__
#define __TEXHEAP_H__

/////////////////////////////////////////////////////////////////////////////
// tagSPANPT: one 8-byte texture point (consumed by the span builder)

typedef struct tagSPANPT
{
	WORD m_wUnk00;		// +0x00
	WORD m_wUnk02;		// +0x02
	DWORD m_dwUnk04;	// +0x04 data (copied into the span entry)
} SPANPT;				// sizeof = 0x8

/////////////////////////////////////////////////////////////////////////////
// tagSPANPTS: point array header inside a MERLIN_TEXENTRY's point data

typedef struct tagSPANPTS
{
	WORD m_wUnk00;		// +0x00 point count
	WORD m_wUnk02;		// +0x02
	SPANPT* m_pUnk04;	// +0x04 point array
} SPANPTS;				// sizeof = 0x8

/////////////////////////////////////////////////////////////////////////////
// tagSPANENTRY: one 0x14-byte span endpoint, built in the fixed arena by
// CSub402E00::Sub_402E00; also embedded in RENDEROBJ at +0xAC

typedef struct tagSPANENTRY
{
	DWORD m_dwUnk00;	// +0x00 span start
	DWORD m_dwUnk04;	// +0x04 span end
	DWORD m_dwUnk08;	// +0x08 scaled span start
	DWORD m_dwUnk0C;	// +0x0C scaled span end
	DWORD m_dwUnk10;	// +0x10 point data
} SPANENTRY;			// sizeof = 0x14

/////////////////////////////////////////////////////////////////////////////
// tagRENDEROBJ: one 0xC0-byte render object. Lives in three doubly-linked
// lists (m_pUnk28/m_pUnk2C at g_pUnk_4A4714, m_pUnk30/m_pUnk34 at
// g_pUnk_4A471C and m_pUnk38/m_pUnk3C at g_pUnk_4A4720).

typedef struct tagRENDEROBJ
{
	DWORD m_dwUnk00;		// +0x00
	DWORD m_dwUnk04;		// +0x04
	WORD m_wUnk08;			// +0x08 point count / span y
	WORD m_wUnk0A;			// +0x0A
	DWORD m_dwUnk0C;		// +0x0C nonzero: sprite object (m_pUnk10 is a
							//   CMerlinDynamic); zero: maze face (m_pUnk10
							//   is a CMerlinStatic)
	CMerlinLine* m_pUnk10;	// +0x10 the source object
	DWORD m_dwUnk14;		// +0x14 face select (computed)
	DWORD m_dwUnk18;		// +0x18 visibility flag
	WORD m_wUnk1C;			// +0x1C sort key (signed; span y)
	WORD m_wUnk1E;			// +0x1E
	DWORD m_dwUnk20;		// +0x20
	DWORD m_dwUnk24;		// +0x24
	tagRENDEROBJ* m_pUnk28;	// +0x28 prev (g_pUnk_4A4714 list)
	tagRENDEROBJ* m_pUnk2C;	// +0x2C next (g_pUnk_4A4714 list)
	tagRENDEROBJ* m_pUnk30;	// +0x30 prev (g_pUnk_4A471C list)
	tagRENDEROBJ* m_pUnk34;	// +0x34 next (g_pUnk_4A471C list)
	tagRENDEROBJ* m_pUnk38;	// +0x38 next (active list)
	tagRENDEROBJ* m_pUnk3C;	// +0x3C prev (active list)
	tagRENDEROBJ* m_pUnk40;	// +0x40 merged object (-1 = list end)
	DWORD m_dwUnk44;		// +0x44
	DWORD m_dwUnk48;		// +0x48 the CMerlinTexture of the upper span run
							// (Sub_403980's dwB, keyed with m_dwUnk60)
	tagRENDEROBJ* m_pUnk4C;	// +0x4C merged object (-1 = list end)
	DWORD m_dwUnk50;		// +0x50
	DWORD m_dwUnk54;		// +0x54 the CMerlinTexture of the lower span run
							// (Sub_403980's dwB, keyed with m_dwUnk64)
	DWORD m_dwUnk58;		// +0x58 visible this frame
	CMerlinTexture* m_pUnk5C;	// +0x5C
	DWORD m_dwUnk60;		// +0x60 span y
	DWORD m_dwUnk64;		// +0x64
	DWORD m_dwUnk68;		// +0x68 (y delta) << 16
	DWORD m_dwUnk6C;		// +0x6C screen y of top edge
	DWORD m_dwUnk70;		// +0x70 screen y of bottom edge
	DWORD m_dwUnk74;		// +0x74 span start (computed by Sub_402E00)
	DWORD m_dwUnk78;		// +0x78 span end
	DWORD m_dwUnk7C;		// +0x7C
	DWORD m_dwUnk80;		// +0x80
	DWORD* m_pUnk84;		// +0x84 left edge value pointer
	DWORD* m_pUnk88;		// +0x88 right edge value pointer
	DWORD m_dwUnk8C;		// +0x8C edges valid
	DWORD m_dwUnk90;		// +0x90 point count
	DWORD m_dwUnk94;		// +0x94 texture origin (16.16)
	DWORD m_dwUnk98;		// +0x98 texture step
	DWORD m_dwUnk9C;		// +0x9C fixed-point shift
	DWORD m_dwUnkA0;		// +0xA0 span mode
	DWORD m_dwUnkA4;		// +0xA4 mapping values valid
	SPANENTRY* m_pUnkA8;	// +0xA8 point spans (fixed arena)
	SPANENTRY m_acUnkAC;	// +0xAC embedded span (m_dwUnk08/m_dwUnk0C are
							// the +0xB4/+0xB8 limits used by Sub_404030)
} RENDEROBJ;				// sizeof = 0xC0

/////////////////////////////////////////////////////////////////////////////
// tagHEAPNODE: one 0x18-byte node of the g_pUnk_4A4490 list, allocated in
// the span arena by CSub402E00::Sub_4030F0

typedef struct tagHEAPNODE
{
	RENDEROBJ* m_pUnk00;	// +0x00 owning render object
	void* m_pUnk04;			// +0x04 the CMerlinTexture the face draws with
	tagHEAPNODE* m_pUnk08;	// +0x08 next
	tagHEAPNODE* m_pUnk0C;	// +0x0C prev
	DWORD m_dwUnk10;		// +0x10 edge selector
	DWORD m_dwUnk14;		// +0x14 edge y
} HEAPNODE;					// sizeof = 0x18

/////////////////////////////////////////////////////////////////////////////
// tagSPANNODE: span-run node allocated inside the CSub403980 heap (the
// 0x2C header is followed in place by two 0xA0-byte word-pair buffers and
// the m_dwUnk00-word index array)

typedef struct tagSPANNODE
{
	WORD* m_pUnk00;			// +0x00 word-pair buffer A
	WORD* m_pUnk04;			// +0x04 word-pair buffer B
	WORD** m_pUnk08;		// +0x08 current read buffer slot
	WORD** m_pUnk0C;		// +0x0C current write buffer slot
	WORD* m_pUnk10;			// +0x10 word index array
	DWORD m_dwUnk14;		// +0x14 written span count
	DWORD m_dwUnk18;		// +0x18 accumulated span count
	DWORD m_dwUnk1C;		// +0x1C span y
	DWORD m_dwUnk20;		// +0x20 key ((y << 0x12) + texture)
	DWORD m_dwUnk24;		// +0x24 the run's CMerlinTexture (Sub_403980's
							// second argument; Sub_403A60 hands it to
							// Sub_402B10 as pTex)
	tagSPANNODE* m_pUnk28;	// +0x28 next
} SPANNODE;					// sizeof = 0x2C (header only)

/////////////////////////////////////////////////////////////////////////////
// CSub403850: view/raster state object at g_pUnk_4A1D68

class CSub403850
{
public:
	// hover.exe:0x403850 scanline -> span limit table entry
	DWORD* Sub_403850(DWORD dwY);

// Member variables (hover.exe offsets in comments)
	void (*m_pfnUnk00)();	// +0x00 span drawer
	void (*m_pfnUnk04)();	// +0x04 span drawer
	void (**m_ppfnUnk08)();	// +0x08 span drawer table (g_dwUnk_4A4A48 index)
	void (*m_pfnUnk0C)();	// +0x0C span drawer
	char m_pad10[0x18];		// +0x10
	DWORD m_dwUnk28;		// +0x28 frame buffer base
	char m_pad2C[0x1A];		// +0x2C
	WORD m_wUnk46;			// +0x46 camera position x (word)
	DWORD m_dwUnk48;		// +0x48 camera position x (16.16; the +0x4A
							// word read by Sub_403880 is its high half)
	DWORD m_dwUnk4C;		// +0x4C camera position y
	DWORD m_dwUnk50;		// +0x50
	DWORD m_dwUnk54;		// +0x54 camera angle
	DWORD m_dwUnk58;		// +0x58
	DWORD m_dwUnk5C;		// +0x5C
	DWORD* m_pUnk60;		// +0x60 span limit table (Sub_403850)
	DWORD m_dwUnk64;		// +0x64
	DWORD m_dwUnk68;		// +0x68
	BYTE* m_pUnk6C;			// +0x6C flat fill color table
	CMerlinTexture* m_pUnk70;	// +0x70 floor/ceiling texture
	DWORD m_dwUnk74;		// +0x74 render mode (1 = flat fill)
	DWORD m_dwUnk78;		// +0x78
	DWORD m_dwUnk7C;		// +0x7C
	DWORD m_dwUnk80;		// +0x80 use the m_ppfnUnk08 table
};

/////////////////////////////////////////////////////////////////////////////
// CSub403980: the span heap ("floor heap"). Allocates SPANNODEs from its
// own fixed arena (m_pUnk18 base, m_pUnk1C cursor, 0x16B48 bytes; the
// "Blowing floor heap" debug warning fires when a new node would pass the
// end)

class CSub402E00;

class CSub403980
{
public:
	// hover.exe:0x403980 find-or-create the span-run node for the
	// (scanline dwA, texture dwB) key and append the x pair dwC/dwD to it
	void Sub_403980(DWORD dwA, DWORD dwB, DWORD dwC, DWORD dwD);
	// hover.exe:0x403A60 merge the accumulated span runs and emit them
	// through the renderer
	void Sub_403A60(CSub402E00* pRender);
	// hover.exe:0x406D00 append one word pair to the current node
	void Sub_406D00(DWORD dwA, DWORD dwB);

// Member variables (hover.exe offsets in comments)
	DWORD m_dwUnk00;		// +0x00 index words per node
	WORD* m_pUnk04;			// +0x04 merge scratch buffer (word pairs)
	SPANNODE* m_pUnk08;		// +0x08 current span node
	SPANNODE* m_pUnk0C;		// +0x0C span-run list head
	SPANNODE* m_pUnk10;		// +0x10 span-run list tail
	DWORD m_dwUnk14;		// +0x14 span-run node count
	BYTE* m_pUnk18;			// +0x18 heap base
	BYTE* m_pUnk1C;			// +0x1C allocation cursor
};

/////////////////////////////////////////////////////////////////////////////
// CSub402E00: the span renderer

class CSub402E00
{
public:
	// hover.exe:0x402E00 rebuild the span lists (when m_dwUnk28), compute
	// the per-object span mapping values and render the frame
	void Sub_402E00(DWORD dwUnk);
	// hover.exe:0x4030F0 rebuild the g_pUnk_4A4490 edge list and the
	// active render-object list from g_pUnk_4A4714/g_pUnk_4A471C
	void Sub_4030F0();
	// hover.exe:0x402630 clip an object's span window against the current
	// clip range, queuing the off-window parts into the span heap
	void Sub_402630(RENDEROBJ* pObj, DWORD dwA, DWORD dwB);
	// hover.exe:0x402990 emit one flat-mapped span pair
	void Sub_402990(RENDEROBJ* pObj, SPANENTRY* pSpan, DWORD dwA, DWORD dwB);
	// hover.exe:0x402B10 emit one floor/ceiling textured span run
	void Sub_402B10(DWORD dwA, DWORD dwB, CMerlinTexture* pTex, DWORD dwY);
	// hover.exe:0x404030 walk the active list covering [dwA, dwB] with
	// spans (recursive; gaps go to the span heap's Sub_406D00)
	void Sub_404030(RENDEROBJ* pObj, DWORD dwA, DWORD dwB);
	// hover.exe:0x4065F0 right-edge variant of the Sub_402630 clip test
	int Sub_4065F0(RENDEROBJ* pObj, SPANENTRY* pSpan, DWORD dwC, DWORD dwD,
		DWORD dwE);
	// hover.exe:0x4066B0 emit one span from a precomputed point span
	void Sub_4066B0(RENDEROBJ* pObj, SPANENTRY* pSpan, DWORD dwA, DWORD dwB);
	// hover.exe:0x406740 emit one vertical span run (flat fill when the
	// view is in mode 1)
	void Sub_406740(DWORD dwA, DWORD dwB);

// Member variables (hover.exe offsets in comments)
	CSub403980* m_pUnk00;	// +0x00 span heap
	char m_pad04[0x20];		// +0x04
	DWORD m_dwUnk24;		// +0x24
	DWORD m_dwUnk28;		// +0x28 nonzero: rebuild the span lists
};

/////////////////////////////////////////////////////////////////////////////
// hover.exe:0x406A80 insert pObj's chain into the active list at its
// sorted position (recursive)
void Hover_sub_406A80(RENDEROBJ* pObj);

/////////////////////////////////////////////////////////////////////////////
// Globals defined in texheap.cpp

extern BYTE* g_pUnk_4A1D60;		// hover.exe:0x4A1D60 span arena cursor
extern DWORD g_dwUnk_4A1D64;	// hover.exe:0x4A1D64 span window start
extern CSub403850* g_pUnk_4A1D68;	// hover.exe:0x4A1D68 view object
extern DWORD g_dwUnk_4A1D6C;	// hover.exe:0x4A1D6C span scale numerator
extern DWORD g_dwUnk_4A1D70;	// hover.exe:0x4A1D70 span origin offset
extern DWORD g_dwUnk_4A1D74;	// hover.exe:0x4A1D74 texture mip level
extern DWORD g_dwUnk_4A1D78;	// hover.exe:0x4A1D78 angle->mip LUT enable
extern BYTE g_abUnk_4A1D80[0x4A40A8 - 0x4A1D80];	// hover.exe:0x4A1D80
								// span arena (end marker 0x4A40A8)
extern HEAPNODE* g_pUnk_4A4490;	// hover.exe:0x4A4490 edge list head
extern CMerlinLine g_lineUnk_4A4498;	// hover.exe:0x4A4498 phase camera
								// line (CSub406090::Sub_406090)
extern CMerlinLine g_lineUnk_4A44E0;	// hover.exe:0x4A44E0 right-edge
								// phase camera line
extern DWORD g_dwUnk_4A4524;	// hover.exe:0x4A4524 clip window start
extern WORD g_wUnk_4A4528;		// hover.exe:0x4A4528
extern DWORD* g_pdwUnk_4A452C;	// hover.exe:0x4A452C scanline scale table
extern DWORD g_dwUnk_4A4530;	// hover.exe:0x4A4530 low edge default
extern DWORD g_dwUnk_4A4534;	// hover.exe:0x4A4534 fill color
extern DWORD g_dwUnk_4A4538;	// hover.exe:0x4A4538 log2 screen width
extern DWORD g_dwUnk_4A453C;	// hover.exe:0x4A453C span step
extern DWORD g_dwUnk_4A4540;	// hover.exe:0x4A4540 frame buffer origin
extern DWORD g_dwUnk_4A4544;	// hover.exe:0x4A4544 screen height
extern BYTE* g_pbUnk_4A4548;	// hover.exe:0x4A4548 angle->mip LUT
extern DWORD g_dwUnk_4A454C;	// hover.exe:0x4A454C screen width
extern DWORD* g_pdwUnk_4A4550;	// hover.exe:0x4A4550 render target span
								// buffer (m_pUnk3C)
extern DWORD g_dwUnk_4A4554;	// hover.exe:0x4A4554 texture shift count
								// (read as a byte by the span drawers)
extern DWORD g_dwUnk_4A455C;	// hover.exe:0x4A455C span length mask
extern DWORD g_dwUnk_4A4564;	// hover.exe:0x4A4564 clip window end
extern DWORD g_dwUnk_4A4568;	// hover.exe:0x4A4568 high edge default
extern DWORD g_dwUnk_4A456C;	// hover.exe:0x4A456C span window end
extern RENDEROBJ* g_pUnk_4A4714;	// hover.exe:0x4A4714 render-object pool list
extern DWORD* g_pdwUnk_4A4718;	// hover.exe:0x4A4718 span buffer tail
								// (m_pUnk44)
extern RENDEROBJ* g_pUnk_4A471C;	// hover.exe:0x4A471C static list head
extern RENDEROBJ* g_pUnk_4A4720;	// hover.exe:0x4A4720 active list head
extern DWORD g_dwUnk_4A4A14;	// hover.exe:0x4A4A14 span length
extern DWORD g_dwUnk_4A4A18;	// hover.exe:0x4A4A18
extern DWORD g_dwUnk_4A4A1C;	// hover.exe:0x4A4A1C texture pixel pointer
extern DWORD g_dwUnk_4A4A20;	// hover.exe:0x4A4A20
extern DWORD g_dwUnk_4A4A24;	// hover.exe:0x4A4A24 texture u step
extern DWORD g_dwUnk_4A4A28;	// hover.exe:0x4A4A28 texture u origin
extern DWORD g_dwUnk_4A4A2C;	// hover.exe:0x4A4A2C texture v origin
extern DWORD g_dwUnk_4A4A30;	// hover.exe:0x4A4A30 render target
								// width * m_dwUnk1C
extern DWORD g_dwUnk_4A4A34;	// hover.exe:0x4A4A34 frame buffer pitch
extern DWORD g_dwUnk_4A4A38;	// hover.exe:0x4A4A38
extern DWORD g_dwUnk_4A4A3C;	// hover.exe:0x4A4A3C span limit table entry
extern DWORD g_dwUnk_4A4A40;	// hover.exe:0x4A4A40
extern DWORD g_dwUnk_4A4A44;	// hover.exe:0x4A4A44 span scale
extern DWORD g_dwUnk_4A4A48;	// hover.exe:0x4A4A48 drawer selector
extern DWORD g_dwUnk_4A4A4C;	// hover.exe:0x4A4A4C destination pointer
extern DWORD g_dwUnk_4A4A50;	// hover.exe:0x4A4A50 texture step
extern DWORD g_dwUnk_4C5208;	// hover.exe:0x4C5208 = 0xFFF0000 (edge
								// sentinel dereferenced through
								// RENDEROBJ::m_pUnk84/m_pUnk88)

/////////////////////////////////////////////////////////////////////////////

#endif // __TEXHEAP_H__
