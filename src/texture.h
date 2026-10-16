// texture.h : interface of the CMerlinObject / CMerlinTexture classes and
// the fixed-heap texture cache
//
// Skeleton reconstructed from hover.exe:
//   CRuntimeClass "CMerlinObject" at 0x4C56A8 (size 0x14, schema 1, base
//   CObject) and "CMerlinTexture" at 0x4C51B0 (size 0x3C, schema 1, base
//   CMerlinObject) -- both DECLARE_SERIAL / IMPLEMENT_SERIAL(.., .., 1)
//   CMerlinObject vtable at 0x4BD808, CMerlinTexture vtable at 0x4BBD20
// The original header was called CTexture.h (per the "Blowing fixed memory
// heap.  Increase MAXTEXTURESIZE in CTexture.h and re-compile." debug
// string emitted by CBumperDoc::OnOpenDocument).
//
// Note on member offsets: the comments give each member's offset within
// the class in hover.exe.

#ifndef __TEXTURE_H__
#define __TEXTURE_H__

/////////////////////////////////////////////////////////////////////////////
// CMerlinArchive: CArchive-derived helper class used by the texture/maze
// loaders. The loaders read WORDs/BYTEs through the archive's buffer
// pointers directly (protected CArchive members), so the original must
// have declared a derived class with inline readers in CTexture.h.

class CMerlinArchive : public CArchive
{
public:
	CMerlinArchive(CFile* pFile, UINT nMode, int nBufSize, BYTE* lpBuf)
		: CArchive(pFile, nMode, nBufSize, lpBuf) { }

	// Inline buffered readers and writers. They are MFC's own
	// CArchive::operator>>/<< bodies with the buffer cursor copied into a
	// local first: without that copy VC 2.2 reloads m_lpBufCur after
	// every store (it cannot prove the store does not alias the member),
	// while the original keeps the cursor in one register across a whole
	// run of reads or writes. The FillBuffer argument is the number of
	// additional bytes needed, as in MFC's own CArchive::ReadCount.
	// The trailing-comment count reader. Deliberately one statement
	// shorter than ReadWord() below: with the extra `BYTE* p` local the
	// body is over VC 2.1's /Ob1 body-size limit and the compiler
	// silently emits a call instead of expanding it (measured in
	// CMerlinDynamic::Serialize, mazeobj.cpp), while the original
	// expands it there.
	WORD ReadCount()
	{
		WORD w;

		if (m_lpBufCur + 2 > m_lpBufMax)
			FillBuffer(2 - (m_lpBufMax - m_lpBufCur));
		w = *(WORD*)m_lpBufCur;
		m_lpBufCur += 2;
		return w;
	}
	WORD ReadWord()
	{
		BYTE* p;
		WORD w;

		if (m_lpBufCur + 2 > m_lpBufMax)
			FillBuffer(2 - (m_lpBufMax - m_lpBufCur));
		p = m_lpBufCur;
		w = *(WORD*)p;
		m_lpBufCur = p + 2;
		return w;
	}
	// Store-into-destination form: the destination is written before the
	// cursor is advanced, which is what keeps the cursor live in a
	// register across a run of reads in the original (the value-returning
	// form above stores the cursor first, so the caller's store into the
	// member forces a reload).
	void ReadWord(WORD* pw)
	{
		BYTE* p;

		if (m_lpBufCur + 2 > m_lpBufMax)
			FillBuffer(2 - (m_lpBufMax - m_lpBufCur));
		p = m_lpBufCur;
		*pw = *(WORD*)p;
		m_lpBufCur = p + 2;
	}
	// Out-of-line byte reader, hover.exe:0x42CF30 (defined in mazeobj.cpp,
	// next to CMerlinDynamic::Serialize, where the original placed it):
	// MFC's own CArchive::operator>>(BYTE&) body with the cursor advanced
	// before the read. It is what the trailing-comment skip loop of the
	// Merlin Serialize overrides calls.
	CMerlinArchive& ReadByte(BYTE* pby);
	BYTE ReadByte()
	{
		if (m_lpBufCur + 1 > m_lpBufMax)
			FillBuffer(1 - (m_lpBufMax - m_lpBufCur));
		m_lpBufCur++;
		return m_lpBufCur[-1];
	}
	DWORD ReadDWord()
	{
		BYTE* p;
		DWORD dw;

		if (m_lpBufCur + 4 > m_lpBufMax)
			FillBuffer(4 - (m_lpBufMax - m_lpBufCur));
		p = m_lpBufCur;
		dw = *(DWORD*)p;
		m_lpBufCur = p + 4;
		return dw;
	}
	// Out-of-line double reader, hover.exe:0x41BEE0 (defined in
	// gameobj.cpp, right behind CMerlinBSP::Serialize, its only caller):
	// like ReadByte above it is really a CMerlinArchive overload of
	// operator>>, so it returns the archive.
	CMerlinArchive& ReadDouble(double* pd);

	void WriteWord(WORD w)
	{
		BYTE* p;

		if (m_lpBufCur + 2 > m_lpBufMax)
			Flush();
		p = m_lpBufCur;
		*(WORD*)p = w;
		m_lpBufCur = p + 2;
	}
	void WriteByte(BYTE b)
	{
		if (m_lpBufCur + 1 > m_lpBufMax)
			Flush();
		m_lpBufCur++;
		m_lpBufCur[-1] = b;
	}
	// Mirror of ReadDouble (gameobj.cpp): the cursor is advanced through a
	// local and the two halves are stored off it, low dword first
	// (hover.exe:0x41BD22..0x41BD2B inlined in CMerlinBSP::Serialize).
	void WriteDouble(double d)
	{
		BYTE* p;

		if (m_lpBufCur + 8 > m_lpBufMax)
			Flush();
		p = m_lpBufCur + 8;
		m_lpBufCur = p;
		((DWORD*)p)[-2] = ((DWORD*)&d)[0];
		((DWORD*)p)[-1] = ((DWORD*)&d)[1];
	}
};

/////////////////////////////////////////////////////////////////////////////
// CMerlinObject: base class of the Merlin engine objects. Carries the
// object name (used as the key into the texture maps) plus one dword.
// vtable at hover.exe:0x4BD808.

class CMerlinObject : public CObject
{
	DECLARE_SERIAL(CMerlinObject)

public:
	CMerlinObject();			// hover.exe:0x401030

	// hover.exe:0x4021E0 (the scalar deleting destructor the vtable slot
	// holds is a second, inlined copy at hover.exe:0x42D620)
	virtual ~CMerlinObject();

	// hover.exe:0x411B10 (vtable slot 2) serialize the object name and
	// the trailing per-object comment blob (a WORD count followed by that
	// many bytes, skipped on load)
	virtual void Serialize(CArchive& ar);

	CString m_strName;			// +0x04 object/texture name
	DWORD m_dwUnk10;			// +0x10 = 0
};

/////////////////////////////////////////////////////////////////////////////
// MERLIN_TEXFRAME: one 0x20-byte texture frame entry, allocated inside the
// fixed texture heap (the entry array pointer is CMerlinTexture::m_pFrames).

struct tagTEXCOLUMN;	// texheap.h

typedef struct tagMERLIN_TEXFRAME
{
	DWORD m_dwColMask;	// +0x00
	DWORD m_dwUnk04;	// +0x04
	DWORD m_dwWidth;	// +0x08 point/span count
	DWORD m_dwHeight;	// +0x0C
	DWORD m_dwUnk10;	// +0x10
	DWORD m_cbPixels;	// +0x14 pixel data byte count (0 = none)
	BYTE* m_pPixels;	// +0x18 pixel data (fixed-heap pointer)
	tagTEXCOLUMN* m_pColumns;	// +0x1C point data (fixed-heap pointer)
} MERLIN_TEXFRAME;		// sizeof = 0x20

/////////////////////////////////////////////////////////////////////////////
// CMerlinTexture

// Declared with the rest of the globals at the bottom of this header; the
// inline destructor below needs it here.
extern DWORD g_dwUnk_4C4DA0;		// hover.exe:0x4C4DA0 texture count

class CMerlinTexture : public CMerlinObject
{
	DECLARE_SERIAL(CMerlinTexture)

public:
	// Defined here, not in texture.cpp: the original's constructor is
	// inline, so the only copy of its body in hover.exe is the one the
	// compiler expanded into the IMPLEMENT_SERIAL Construct(void*) thunk
	// at 0x4128F0 (notes/perfect-match.md rule 221).
	CMerlinTexture()		// hover.exe:0x4128F0
	{
		m_wWidth = 0;
		m_wHeight = 0;
		m_dwUnk34 = 0;
		m_dwUnk20 = 0;
		m_pFrames = NULL;
		m_dwFrameCount = 0;
		m_bTransparent = 0;
		m_pNextFrame = NULL;
		g_dwUnk_4C4DA0++;
	}

	// hover.exe:0x418A60 is the scalar deleting destructor (vtable slot 1);
	// the class destructor itself is empty apart from the texture-count
	// decrement and is folded into it
	virtual ~CMerlinTexture()
	{
		g_dwUnk_4C4DA0--;
	}

	// hover.exe:0x412980 load the texture frames from the .tex archive
	// into the fixed heap (vtable slot 2)
	virtual void Serialize(CArchive& ar);

// Member variables (hover.exe offsets in comments)
	WORD m_wWidth;			// +0x14 = 0
	WORD m_wColMask;			// +0x16
	WORD m_wHeight;			// +0x18 = 0
	WORD m_wUnk1A;			// +0x1A
	DWORD m_bTransparent;		// +0x1C = 0
	DWORD m_dwUnk20;		// +0x20 = 0
	DWORD m_dwUnk24;		// +0x24
	MERLIN_TEXFRAME* m_pFrames;	// +0x28 = 0 frame entries (fixed heap)
	DWORD m_dwFrameCount;		// +0x2C = 0 frame count
	DWORD m_dwUnk30;		// +0x30 (never touched by the constructor or
							// Serialize in hover.exe; keeps the following
							// members at their original offsets)
	DWORD m_dwUnk34;		// +0x34 = 0
	CMerlinTexture* m_pNextFrame;	// +0x38 = 0 next animation frame (linked by
							// CBumperDoc::OnOpenDocument)
};							// sizeof = 0x3C

/////////////////////////////////////////////////////////////////////////////
// Globals defined in texture.cpp

extern CObArray g_obArrUnk_4A1D08;		// hover.exe:0x4A1D08 loaded textures
extern CMapStringToOb g_mapUnk_4A1D30;	// hover.exe:0x4A1D30 name -> texture
extern CMapStringToOb g_mapUnk_4A4578;	// hover.exe:0x4A4578 name ->
										// CMerlinStatic (maze face)
extern CMapStringToOb g_mapUnk_4A4768;	// hover.exe:0x4A4768 name ->
										// CMerlinLocation (named point)
extern void* g_pUnk_461D00;			// hover.exe:0x461D00 fixed texture heap
									// cursor (base is g_pTextureHeap)
extern DWORD g_dwUnk_4C4DA0;		// hover.exe:0x4C4DA0 texture count
extern DWORD g_dwUnk_4C4DA8;		// hover.exe:0x4C4DA8 textures loaded so far
typedef void (__cdecl* LPPROGRESSFN)(DWORD dwDone, DWORD dwTotal);
extern LPPROGRESSFN g_pfnUnk_4C4DAC;	// hover.exe:0x4C4DAC load progress
									// callback
extern DWORD g_dwUnk_4A1D10;		// hover.exe:0x4A1D10 texture total
extern DWORD g_dwUnk_4C57CC;		// hover.exe:0x4C57CC load progress base
extern DWORD g_dwUnk_4C57D0;		// hover.exe:0x4C57D0 load progress range

// Texture base-name tables used to build "%s%02d"/"%s%02d_%02d" names
extern const char* g_apszUnk_4C48F0[8];	// hover.exe:0x4C48F0 "PAD_*"
extern const char* g_apszUnk_4C4928[10];	// hover.exe:0x4C4928 "FLG*"/"POD*"

/////////////////////////////////////////////////////////////////////////////
// Name lookups in the two maze object maps (each searches its own map and
// returns NULL when the name is absent). g_mapUnk_4A4578 holds the
// CMerlinStatic faces (their +0xA4/+0xA6 words are the edge heights),
// g_mapUnk_4A4768 the CMerlinLocation points.

// hover.exe:0x41A4E0 search g_mapUnk_4A4578
CObject* Hover_FindStatic(const CString& strName);
// hover.exe:0x40FE40 search g_mapUnk_4A4768
CObject* Hover_FindLocation(const CString& strName);

/////////////////////////////////////////////////////////////////////////////

#endif // __TEXTURE_H__
