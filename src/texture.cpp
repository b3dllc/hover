// texture.cpp : implementation of the CMerlinObject / CMerlinTexture
// classes and the fixed-heap texture cache
//
// Reconstructed from hover.exe; see texture.h for the RTTI/vtable data.

#include "hover.h"
#include "texture.h"
#include "gameobj.h"
#include "mazeobj.h"
#include "bumperdoc.h"
#include "bumpview.h"
#include "texmisc.h"
#include "texload.h"

#include <mbstring.h>

// The shipped build has the pad-highlight debug flag off; the loader still
// carries both arms of the test (hover.exe:0x417583 and 0x4175f4).
#define HOVER_SHOWPADS 0

IMPLEMENT_SERIAL(CMerlinObject, CObject, 1)
// IMPLEMENT_SERIAL hand-expanded: the placement-new thunk it generates,
// CMerlinTexture::Construct, is defined further down so that it can carry
// the reccmp marker for hover.exe:0x4128F0 (rule 221).
_IMPLEMENT_RUNTIMECLASS(CMerlinTexture, CMerlinObject, 1, CMerlinTexture::Construct)
CArchive& AFXAPI operator>>(CArchive& ar, CMerlinTexture* &pOb)
	{ pOb = (CMerlinTexture*) ar.ReadObject(RUNTIME_CLASS(CMerlinTexture));
		return ar; }

/////////////////////////////////////////////////////////////////////////////
// Globals

// hover.exe:0x4A1D08
// GLOBAL: HOVER 0x4a1d08
CObArray g_obArrUnk_4A1D08;		// loaded texture objects (serialized from
								// the level's .tex file)

// hover.exe:0x4A1D30
// GLOBAL: HOVER 0x4a1d30
CMapStringToOb g_mapUnk_4A1D30;	// texture name -> CMerlinTexture (filled by
								// CMerlinTexture::Serialize)

// hover.exe:0x4A4578
// GLOBAL: HOVER 0x4a4578
CMapStringToOb g_mapUnk_4A4578;	// object name -> CMerlinStatic face

// hover.exe:0x4A4768
// GLOBAL: HOVER 0x4a4768
CMapStringToOb g_mapUnk_4A4768;	// object name -> CMerlinLocation point

// hover.exe:0x461D00
// GLOBAL: HOVER 0x461d00
void* g_pUnk_461D00;			// fixed texture heap cursor (reset to
								// g_pUnk_46003C by CBumperDoc::OnOpenDocument)

DWORD g_dwUnk_4C4DA0;			// hover.exe:0x4C4DA0 texture count
DWORD g_dwUnk_4C4DA8;			// hover.exe:0x4C4DA8 textures loaded so far

// hover.exe:0x4C4DAC
// GLOBAL: HOVER 0x4c4dac
LPPROGRESSFN g_pfnUnk_4C4DAC;	// load progress callback

DWORD g_dwUnk_4A1D10;			// hover.exe:0x4A1D10 texture total

// GLOBAL: HOVER 0x4c57cc
DWORD g_dwUnk_4C57CC;			// hover.exe:0x4C57CC load progress base
// GLOBAL: HOVER 0x4c57d0
DWORD g_dwUnk_4C57D0;			// hover.exe:0x4C57D0 load progress range

// hover.exe:0x4C48F0
// GLOBAL: HOVER 0x4c48f0
const char* g_apszUnk_4C48F0[8] = {	// pad texture base names
	"PAD_HOLD_", "PAD_FLAG_REMOVER_", "PAD_SLED_L_", "PAD_SLED_R_",
	"PAD_SLED_T_", "PAD_SLED_B_", "PAD_RAZED_", "PAD_LOWERED_"
};

// hover.exe:0x4C4928
// GLOBAL: HOVER 0x4c4928
const char* g_apszUnk_4C4928[10] = {	// flag/pod texture base names
	"FLG00_", "FLG01_", "POD06_", "POD02_", "POD03_", "POD04_", "POD01_",
	"POD00_", "POD07_", "POD05_"
};

/////////////////////////////////////////////////////////////////////////////
// CMerlinObject

// hover.exe:0x401030
// FUNCTION: HOVER 0x401030
CMerlinObject::CMerlinObject()
{
	m_dwUnk10 = 0;
	m_strUnk04.Empty();
}

// The body is empty -- only the m_strUnk04 CString is destroyed, by the
// compiler-generated member call. The original emits this out of line
// because the maze loaders build Merlin objects on the stack, and inlines
// a second copy into the scalar deleting destructor (hover.exe:0x42D620).
// hover.exe:0x4021E0
// FUNCTION: HOVER 0x4021e0
CMerlinObject::~CMerlinObject()
{
}

/////////////////////////////////////////////////////////////////////////////
// CMerlinTexture

// The IMPLEMENT_SERIAL placement-construction thunk. CMerlinTexture's
// constructor is inline (texture.h), so the compiler expands its whole
// body - the CMerlinObject base call, the vtable store, the zeroed members
// and the texture-count increment - into this thunk, behind the null test
// that `new(p)` needs; that is all hover.exe:0x4128F0 is.
// hover.exe:0x4128F0
// FUNCTION: HOVER 0x4128f0
void PASCAL CMerlinTexture::Construct(void* p)
{
	new(p) CMerlinTexture;
}

// Give the texture count back. The body is folded into the scalar deleting
// destructor the vtable slot holds, so the marker pairs our ??1 against
// the original's ??_G (notes/perfect-match.md rule 34).
// hover.exe:0x418A60
// The destructor is defined in the class declaration, so the compiler folds
// it into the scalar deleting destructor that the vtable slot holds - which
// is what hover.exe:0x418A60 is. The block below emits no code; it only gives
// the reccmp marker a declaration to name ??_GCMerlinTexture@@ (rules 34/184).
// hover.exe:0x418A60
#if 0
// FUNCTION: HOVER 0x418a60
CMerlinTexture::~CMerlinTexture()
{
	g_dwUnk_4C4DA0--;
}
#endif

// CMerlinObject::Serialize (vtable slot 2) -- the shared part of every
// serializable Merlin object: the object name, followed by a WORD count
// and that many comment bytes, which the loader skips. Every derived
// Serialize calls this first and ends with the same trailing blob.

// hover.exe:0x411B10
// FUNCTION: HOVER 0x411b10
void CMerlinObject::Serialize(CArchive& ar)
{
	CMerlinArchive* pAr = (CMerlinArchive*)&ar;
	short sCount;
	int i;
	BYTE by;
	volatile BYTE byPad;

	byPad = 0;
	if (ar.IsStoring())
	{
		*pAr << m_strUnk04;
		pAr->WriteWord((WORD)0);
	}
	else
	{
		*pAr >> m_strUnk04;
		sCount = (short)pAr->ReadWord();
		if (sCount > 0)			// trailing comment bytes, skipped
		{
			i = sCount;
			do
				ar >> by;
			while (--i);
		}
	}
}

// hover.exe:0x412980
// FUNCTION: HOVER 0x412980
void CMerlinTexture::Serialize(CArchive& ar)
{
	CMerlinArchive* pAr = (CMerlinArchive*)&ar;
	int nLevel;
	int i;
	DWORD dwSize;
	int nSpans;
	BYTE* pPts;
	DWORD dwAccum;
	WORD wY;
	WORD wX;
	// Frame placeholder, not a variable of the original (rule 80): the
	// original's frame is 0x20 and holds one spill slot more than our
	// expressions generate.
	volatile DWORD adwPad[1];

	adwPad[0] = 0;
	CMerlinObject::Serialize(ar);
	if (ar.IsStoring())
		return;

	// The two counts are stored as WORDs into the low half of their
	// dword members, exactly as in the original.
	// Store-into-destination form: the original writes the member before
	// it advances the archive cursor (hover.exe:0x4129C6/0x4129CA).
	pAr->ReadWord((WORD*)&m_dwUnk1C);
	pAr->ReadWord((WORD*)&m_dwUnk2C);
	g_mapUnk_4A1D30[m_strUnk04] = this;
	m_pUnk28 = (MERLIN_TEXENTRY*)g_pUnk_461D00;
	g_pUnk_461D00 = (BYTE*)g_pUnk_461D00 + m_dwUnk2C * 0x20;
	for (nLevel = 0; nLevel < (int)m_dwUnk2C; nLevel++)
	{
		m_pUnk28[nLevel].m_dwUnk08 = (short)pAr->ReadWord();
		m_pUnk28[nLevel].m_dwUnk00 = (short)pAr->ReadWord();
		m_pUnk28[nLevel].m_dwUnk0C = (short)pAr->ReadWord();
		m_pUnk28[nLevel].m_dwUnk04 = (short)pAr->ReadWord();
		m_pUnk28[nLevel].m_dwUnk10 = (short)pAr->ReadWord();
		m_pUnk28[nLevel].m_dwUnk14 = pAr->ReadDWord();
		if (m_pUnk28[nLevel].m_dwUnk14 != 0)
		{
			if (nLevel < (int)g_dwUnk_461CFC && (int)m_dwUnk2C > 1)
			{
				// mip levels below the memory configuration mode are
				// skipped, not loaded
				for (i = 0; i < (int)m_pUnk28[nLevel].m_dwUnk14; i++)
					pAr->ReadByte();
			}
			else
			{
				m_pUnk28[nLevel].m_dwUnk18 = (DWORD)g_pUnk_461D00;
				g_pUnk_461D00 = (BYTE*)g_pUnk_461D00 + m_pUnk28[nLevel].m_dwUnk14;
				ar.Read((void*)m_pUnk28[nLevel].m_dwUnk18, m_pUnk28[nLevel].m_dwUnk14);
			}
		}
		else
			m_pUnk28[nLevel].m_dwUnk18 = 0;
		if (nLevel < (int)g_dwUnk_461CFC && (int)m_dwUnk2C > 1)
		{
			pAr->ReadDWord();	// skipped
			for (i = 0; i < (int)m_pUnk28[nLevel].m_dwUnk08; i++)
			{
				nSpans = (short)pAr->ReadWord();
				while (nSpans > 0)
				{
					pAr->ReadWord();	// skipped
					pAr->ReadWord();	// skipped
					nSpans--;
				}
			}
		}
		else
		{
			dwSize = pAr->ReadDWord();
			m_pUnk28[nLevel].m_dwUnk1C = (DWORD)g_pUnk_461D00;
			// pPts comes off the heap cursor before it moves on:
			// hover.exe:0x412d27 reuses the value loaded at 0x412cfb.
			pPts = (BYTE*)m_pUnk28[nLevel].m_dwUnk1C + m_pUnk28[nLevel].m_dwUnk08 * 8;
			g_pUnk_461D00 = (BYTE*)g_pUnk_461D00 +
				(m_pUnk28[nLevel].m_dwUnk08 + dwSize) * 8;
			dwAccum = m_pUnk28[nLevel].m_dwUnk18;
			for (i = 0; i < (int)m_pUnk28[nLevel].m_dwUnk08; i++)
			{
				nSpans = (short)pAr->ReadWord();
				*(WORD*)((BYTE*)m_pUnk28[nLevel].m_dwUnk1C + i * 8) = (WORD)nSpans;
				*(DWORD*)((BYTE*)m_pUnk28[nLevel].m_dwUnk1C + i * 8 + 4) =
					(DWORD)pPts;
				while (nSpans > 0)
				{
					wY = pAr->ReadWord();
					*(WORD*)pPts = wY;
					*(DWORD*)(pPts + 4) = dwAccum;
					wX = pAr->ReadWord();
					*(WORD*)(pPts + 2) = wX;
					pPts += 8;
					dwAccum += (short)((short)wX >> nLevel) -
						(short)((short)wY >> nLevel) + 1;
					nSpans--;
				}
			}
		}
		nSpans = (short)pAr->ReadWord();
		while (nSpans > 0)			// trailing comment bytes, skipped
		{
			pAr->ReadByte();
			nSpans--;
		}
	}
	m_wUnk14 = (WORD)m_pUnk28[0].m_dwUnk08;
	m_wUnk16 = (WORD)m_pUnk28[0].m_dwUnk00;
	m_wUnk18 = (WORD)m_pUnk28[0].m_dwUnk0C;
	m_wUnk1A = (WORD)m_pUnk28[0].m_dwUnk04;
	m_dwUnk34 = 0;
	m_dwUnk24 = 0;
	m_dwUnk20 = m_pUnk28[0].m_dwUnk18;
	if (g_pfnUnk_4C4DAC != NULL)
		g_pfnUnk_4C4DAC(g_dwUnk_4C4DA8++, g_obArrUnk_4A1D08.GetSize() - 1);
}

/////////////////////////////////////////////////////////////////////////////
// Texture-by-name lookups

// hover.exe:0x41A4E0
// FUNCTION: HOVER 0x41a4e0
CObject* Hover_sub_41A4E0(const CString& strName)
{
	CObject* pOb;

	if (g_mapUnk_4A4578.Lookup(strName, pOb))
		return pOb;
	return NULL;
}

// hover.exe:0x40FE40
// FUNCTION: HOVER 0x40fe40
CObject* Hover_sub_40FE40(const CString& strName)
{
	CObject* pOb;

	if (g_mapUnk_4A4768.Lookup(strName, pOb))
		return pOb;
	return NULL;
}

/////////////////////////////////////////////////////////////////////////////
// CBumperDoc::Sub_412250 — place a flag/pod object on a random free pad
// ("FLAG_HUMAN_"/"FLAG_ROBOT_" pads for flags, "POD_RANDOM_" pods
// otherwise); bUnk spawns a nested "POD06_" pod at the same point.

// hover.exe:0x412250 (the coverage table lists this function as 0x412256,
// the address after the 6-byte "mov eax,fs:0" SEH prefix)
// FUNCTION: HOVER 0x412250
void CBumperDoc::Sub_412250(CGameObj* pObj, int bUnk)
{
	CMerlinLocation* pTex;
	// The point handed to CCollider::Sub_40F9B0 is built by hand rather
	// than through CMerlinLocation::GetPoint(): VC 2.1 does not elide the
	// copy out of a POINT-returning inline (rule 174), and the extra
	// 8-byte temporary per search loop wrecks the frame.  Spelt this way
	// the four calls match the original instruction for instruction,
	// including the x spill at ebp-0x50/-0x40/-0x38 and the radius pushed
	// from memory.
	POINT pt;
	CPodObj* pNew;
	DWORD adwPt[2];
	DWORD* pPt;
	// Frame placeholder, not a variable of the original (rule 80): four
	// bytes, the difference between our one shared POINT and the
	// original's four per-block temporaries.
	volatile DWORD adwPad[1];

	if (bUnk == 0)
	{
		adwPad[0] = 0;
		if (pObj->IsKindOf(RUNTIME_CLASS(CFlag)))
		{
			if (((CPodObj*)pObj)->m_dwUnkCC != 0)
			{
				char szName[0x80];
				CMerlinLocation* pT;
				int nDist;

				nDist = 0x50;
				do
				{
					sprintf(szName, "%s%02d", g_apszUnk_4C4910[3],
						rand() % (int)m_adwUnk83D0[3]);
					pT = (CMerlinLocation*)Hover_sub_40FE40(
						CString(szName));
				}
				while (pT != NULL &&
					(pt.x = (short)pT->m_wUnk14,
					 pt.y = (short)pT->m_wUnk16,
					 ((CCollider*)m_dwUnk8388)->Sub_40F9B0(pt, nDist)));
				pTex = pT;
			}
			else
			{
				char szName[0x80];
				CMerlinLocation* pT;
				int nDist;

				nDist = 0x50;
				do
				{
					sprintf(szName, "%s%02d", g_apszUnk_4C4910[2],
						rand() % (int)m_adwUnk83D0[2]);
					pT = (CMerlinLocation*)Hover_sub_40FE40(
						CString(szName));
				}
				while (pT != NULL &&
					(pt.x = (short)pT->m_wUnk14,
					 pt.y = (short)pT->m_wUnk16,
					 ((CCollider*)m_dwUnk8388)->Sub_40F9B0(pt, nDist)));
				pTex = pT;
			}
		}
		else
		{
			char szName[0x80];
			CMerlinLocation* pT;
			int nDist;

			nDist = 0x50;
			do
			{
				sprintf(szName, "%s%02d", g_apszUnk_4C4910[4],
					rand() % (int)m_adwUnk83D0[4]);
				pT = (CMerlinLocation*)Hover_sub_40FE40(
					CString(szName));
			}
			while (pT != NULL &&
				(pt.x = (short)pT->m_wUnk14,
				 pt.y = (short)pT->m_wUnk16,
				 ((CCollider*)m_dwUnk8388)->Sub_40F9B0(pt, nDist)));
			pTex = pT;
		}
		((CPodObj*)pObj)->Sub_40F880((short)pTex->m_wUnk14,
			(short)pTex->m_wUnk16);
		((CPodObj*)pObj)->m_dwUnk94 = 1;
		((CPodObj*)pObj)->m_dwUnkA4 = (short)pTex->m_wUnk1A;
		((CPodObj*)pObj)->Sub_408530();
		((CCollider*)m_dwUnk8388)->Sub_40F900((CPodObj*)pObj);
		m_list838C.AddTail((CPodObj*)pObj);
		return;
	}
	{
	char szName[0x80];
	CMerlinLocation* pT;
	int nDist;

	nDist = 0x50;
	do
	{
		sprintf(szName, "%s%02d", g_apszUnk_4C4910[4],
			rand() % (int)m_adwUnk83D0[4]);
		pT = (CMerlinLocation*)Hover_sub_40FE40(CString(szName));
	}
	while (pT != NULL &&
		(pt.x = (short)pT->m_wUnk14,
		 pt.y = (short)pT->m_wUnk16,
		 ((CCollider*)m_dwUnk8388)->Sub_40F9B0(pt, nDist)));
	pTex = pT;
	}
	((CPodObj*)pObj)->Sub_40F880((short)pTex->m_wUnk14,
		(short)pTex->m_wUnk16);
	((CPodObj*)pObj)->m_dwUnkA4 = (short)pTex->m_wUnk1A;
	pPt = ((CPodObj*)pObj)->Sub_403880(adwPt);
	pNew = new CContainer((CPodObj*)pObj, pPt[0], pPt[1],
		((CPodObj*)pObj)->m_dwUnk9C);
	pNew->m_dwUnk94 = 1;
	pNew->Sub_408530();
	((CCollider*)m_dwUnk8388)->Sub_40F900(pNew);
	m_list838C.AddTail(pNew);
}
/////////////////////////////////////////////////////////////////////////////
// Endpoint helpers for the loader's wall-chain fix-up.
//
// hover.exe reads each endpoint of a pad's wall chain as one 8-byte value
// into an *expression* temporary, not into a named local
// (hover.exe:0x41670A, 0x416732, 0x4167E4 and 0x41680C all store the pair
// into a slot below the loader's named region).  A by-value POINT
// parameter of an expanded inline is exactly such a temporary, so the two
// places that need a pair go through these file-local inlines.

// hover.exe:0x41674B -- the ring-closed test, `<CPoint temporary> !=
// <POINT temporary>`: the object is filled first, the operator's by-value
// argument second.
inline CPoint PtAt(const DWORD* pdw)
{
	return *(CPoint*)pdw;
}

// hover.exe:0x416825 -- both endpoints are copied into temporaries and
// their addresses are handed to Sub_422BF0.
inline void SetSegment(CSub40D320* pSeg, POINT ptA, POINT ptB)
{
	pSeg->Sub_422BF0((DWORD*)&ptA, (DWORD*)&ptB);
}

/////////////////////////////////////////////////////////////////////////////
// CBumperDoc::OnOpenDocument — the level loader (slot 0x13 of the
// original's CBumperDoc vtable, i.e. CDocument::OnOpenDocument): palette
// and textures from mazes\TEXn.tex into the fixed heap, maze data through
// CMerlinWorld::Serialize, palette shade tables, music, players, and (for
// a fresh game) the default-maze objects. lpszPathName is ignored -- the
// file names are built from the level number -- and OnNewDocument passes
// NULL.

// Note: the original of this function is unoptimized code (`this` and every
// temporary homed on the stack, a `jmp $+5` after most statements). So is
// ours -- VC 2.1's optimizer silently gives up on a function this large, so
// no `#pragma optimize("", off)` (rule 49) is needed or has any effect here.

// hover.exe:0x415450
// FUNCTION: HOVER 0x415450
BOOL CBumperDoc::OnOpenDocument(LPCTSTR lpszPathName)
{
	// The original uses the C++ keywords, not MFC's TRY/CATCH_ALL macros:
	// its frame holds no AFX_EXCEPTION_LINK and its EH state numbering
	// starts at 0 for the try block itself.  The state store
	// (`mov dword ptr [ebp-4],0`, hover.exe:0x41547D) is the loader's
	// very first instruction, so the try opens ahead of the progress
	// counters, not at the first archive -- and *every* local of the
	// loader lives inside it: the function block itself declares none,
	// which is why the original's first named slot is ebp-0x14 and why
	// the two files and the two archives share one frame pool with the
	// scalars instead of being pushed below them (a nested block gets a
	// pool of its own, allocated after the enclosing one).
	try
	{
		// The loader's nineteen locals, pinned to the original's frame
		// slots.  VC 2.1 orders /Od named locals by C2's identifier
		// hash, not by declaration order (rules 160/195): locals that are
		// adjacent in the original's frame are therefore grouped into
		// structs -- a struct is laid out in declaration order from
		// its own base, so listing the members bottom-up pins every
		// slot -- and the twelve resulting names are chosen so that
		// their hash buckets sort into the original's frame order:
		//   sDoc(0) pLoc(0) sHead(2) fileMaze(6) sDir(7) sK(7)
		//   arTexIn(8) nLev(10) fileTexture(11) pEntry(11) sTail(12)
		//   arMaze(15).
		// Where two share a bucket the *later* declaration takes the
		// higher slot, so pLoc is declared ahead of sDoc and sK ahead
		// of sDir.
		// sK, pEntry and pLoc have to stay out of the structs: the
		// archive extractor and the POINT accessor fold a plain
		// local straight into the expansion, but a struct member is
		// materialised into a frame temporary first (five extra
		// slots, 20 bytes).
		CMerlinLocation* pLoc;			// ebp-0x1c
		struct
		{
			int nRet;					// ebp-0x18
			int i;						// ebp-0x14
		} sDoc;
		struct
		{
			int nWalls;					// ebp-0x28
			POSITION pos;				// ebp-0x24
			LOGPALETTE* pPal;			// ebp-0x20
		} sHead;
		short sK;						// ebp-0x144
		struct
		{
			CWall* pWall;				// ebp-0x140
			char szPath[_MAX_PATH];		// ebp-0x13c (0x104 bytes)
			BYTE by;					// ebp-0x38
		} sDir;
		int nLev;						// ebp-0x17c
		PALETTEENTRY* pEntry;			// ebp-0x18c
		struct
		{
			WORD wCount;				// ebp-0x214
			time_t timeUnk;				// ebp-0x210
			char szName[0x80];			// ebp-0x20c
		} sTail;

		g_dwUnk_4C57CC = 0;
		g_dwUnk_4C57D0 = 0x3C;
		SetCursor(LoadCursor(NULL, (LPCTSTR)0x7F02));	// IDC_WAIT
		m_dwUnk83E8 = 0;
		// Two statements: the original re-reads the variable rather
		// than using time()'s return value (hover.exe:0x4154D5).
		time(&sTail.timeUnk);
		srand((unsigned)sTail.timeUnk);
		m_dwUnk0188 = (DWORD)new CMerlinWorld;
		if (theApp.m_dwUnk01CC == 0)
			nLev = g_dwUnk_46049C;
		else
			nLev = rand() % 0x14;
		if (g_dwUnk_46049C == 0x14)
			nLev = g_dwUnk_46049C;

		// palette + texture frames from mazes\TEXn.tex
		lstrcpy(sDir.szPath, theApp.m_strUnk0414);
		if (m_dwUnk8498 != 0)
			lstrcat(sDir.szPath, g_lvlParams[nLev].m_pszTextures);
		else
			lstrcat(sDir.szPath, LVLPARAM(nLev).m_pszTextures);
		CFile fileTexture(sDir.szPath, CFile::modeRead | CFile::typeBinary);
		CArchive arTexIn(&fileTexture, CArchive::load, 0x200, NULL);
		sHead.pPal = (LOGPALETTE*)::operator new(0x408);
		sHead.pPal->palVersion = 0x300;
		sHead.pPal->palNumEntries = 0x100;
		pEntry = sHead.pPal->palPalEntry;
		for (sK = 0; sK < 0x100; sK++)
		{
			arTexIn >> pEntry[sK].peRed;
			arTexIn >> pEntry[sK].peGreen;
			arTexIn >> pEntry[sK].peBlue;
			arTexIn >> pEntry[sK].peFlags;
		}
		// Detach, not DeleteObject: hover.exe:0x415823 calls 0x451349,
		// which clears m_hObject and returns it without deleting the
		// palette (0x451378 is DeleteObject and is not called here).
		g_paletteUnk_4A1D50.Detach();
		g_paletteUnk_4A1D50.Attach(CreatePalette(sHead.pPal));
		delete sHead.pPal;
		g_pUnk_461D00 = g_pUnk_46003C;
		g_obArrUnk_4A1D08.Serialize(arTexIn);
		if (g_cbUnk_46002C + (BYTE*)g_pUnk_46003C < (BYTE*)g_pUnk_461D00)
			OutputDebugString("Blowing fixed memory heap.  "
				"Increase MAXTEXTURESIZE in CTexture.h and "
				"re-compile.");
		arTexIn >> sTail.wCount;
		for (sK = 0; (short)sTail.wCount > sK; sK++)
			arTexIn >> sDir.by;	// comment, skipped
		arTexIn.Close();
		fileTexture.Close();

		// maze data
		lstrcpy(sDir.szPath, theApp.m_strUnk0414);
		if (m_dwUnk8498 != 0)
			lstrcat(sDir.szPath, g_lvlParams[nLev].m_pszMaze);
		else
			lstrcat(sDir.szPath, LVLPARAM(nLev).m_pszMaze);
		CFile fileMaze(sDir.szPath, CFile::modeRead | CFile::typeBinary);
		CArchive arMaze(&fileMaze, CArchive::load, 0x200, NULL);
		((CMerlinWorld*)m_dwUnk0188)->Serialize(arMaze);
		arMaze.Close();
		fileMaze.Close();
		((CMerlinWorld*)m_dwUnk0188)->Sub_418FA0();
		g_dwUnk_4C57CC = 0x3C;
		g_dwUnk_4C57D0 = 0x46;
		((CSub414550*)m_pad018C)->Sub_414550(&g_paletteUnk_4A1D50,
			m_dblUnk8478, 0xFF, 0xFF, 0xFF, m_dblUnk8480, m_dblUnk8488,
			m_dblUnk8490);
		g_dwUnk_4C57CC = 0x46;
		g_dwUnk_4C57D0 = 0x50;
		((CSub414550*)(m_pad018C + 0x4000))->Sub_414550(
			&g_paletteUnk_4A1D50, m_dblUnk8478, 0xFF, 0xFF, 0xFF, 0.8,
			1.5, 0.8);
		if (LVLPARAM(nLev).m_pszMusic != NULL)
		{
			lstrcpy(sDir.szPath, theApp.m_strUnk0414);
			lstrcat(sDir.szPath, LVLPARAM(nLev).m_pszMusic);
			m_obj0060.Sub_41C030(sDir.szPath);
		}
		m_dwUnk8388 = (DWORD)new CCollider(CSize(0x5A00, 0x5A00),
			0x100);
		this->Sub_417C10();
		pLoc = (CMerlinLocation*)this->Sub_417D40(0);
		m_humanPlayer.Sub_41B2D0(pLoc->GetPoint(),
			(short)pLoc->m_wUnk18, (short)pLoc->m_wUnk1A);
		m_list83A8.AddHead(&m_humanPlayer);
		((CCollider*)m_dwUnk8388)->Sub_41B770(&m_humanPlayer);

		if (m_dwUnk8498 == 0)
		{
			// The robot loops, the animation chaining and the pad
			// loop all draw on one frame pool in the original
			// (ebp-0x24c..ebp-0x290, eighteen dwords), so they are
			// one block here, and the names are again picked so
			// that their hash buckets sort into the original's
			// order -- declared bottom-up, because within a bucket
			// the *later* declaration takes the higher slot:
			//   pTexEnt(1) nK(3) pFloorObj(6) nSeq(6) nGroup(7)
			//   pLoc1(7) pDrone(7) pLoc2(8) pDrone2(9) pFrst(9)
			//   pLast(9) j1(9) pAfter(9) pSide1(9) pSide2(10)
			//   posIt(10) pSide3(11) pSide4(12).
			CWall* pSide4;				// ebp-0x290
			CWall* pSide3;				// ebp-0x28c
			POSITION posIt;				// ebp-0x288
			CWall* pSide2;				// ebp-0x284
			CWall* pSide1;				// ebp-0x280
			CObject* pAfter;			// ebp-0x27c
			int j1;						// ebp-0x278
			CMerlinTexture* pLast;		// ebp-0x274
			CObject* pFrst;				// ebp-0x270
			CRobotPlayer* pDrone2;		// ebp-0x26c
			CMerlinLocation* pLoc2;		// ebp-0x268
			CRobotPlayer* pDrone;		// ebp-0x264
			CMerlinLocation* pLoc1;		// ebp-0x260
			int nGroup;					// ebp-0x25c
			int nSeq;					// ebp-0x258
			CFloorObj* pFloorObj;		// ebp-0x254
			int nK;						// ebp-0x250
			CMerlinStatic* pTexEnt;		// ebp-0x24c

			Hover_sub_422370();
			// robot players
			for (sDoc.i = 0; sDoc.i < (int)LVLPARAM((int)g_dwUnk_46049C).m_dwUnk28;
				sDoc.i++)
			{
				pLoc1 = (CMerlinLocation*)this->Sub_417D40(1);
				pDrone = new CRobotPlayer(1);
				pDrone->Sub_41B0E0(pLoc1->GetPoint(),
					(short)pLoc1->m_wUnk18,
					(short)pLoc1->m_wUnk1A);
				m_list83A8.AddTail(pDrone);
				((CCollider*)m_dwUnk8388)->Sub_41B770(pDrone);
			}
			for (sDoc.i = 0; sDoc.i < (int)LVLPARAM((int)g_dwUnk_46049C).m_dwUnk2C;
				sDoc.i++)
			{
				pLoc2 = (CMerlinLocation*)this->Sub_417D40(1);
				pDrone2 = new CRobotPlayer(0);
				pDrone2->Sub_41B0E0(pLoc2->GetPoint(),
					(short)pLoc2->m_wUnk18,
					(short)pLoc2->m_wUnk1A);
				m_list83A8.AddTail(pDrone2);
				((CCollider*)m_dwUnk8388)->Sub_41B770(pDrone2);
			}
			// chain the numbered animation frames of each family
			for (nGroup = 0; nGroup < 10; nGroup++)
			{
				pFrst = NULL;
				wsprintf(sTail.szName, "%s%02d", g_apszUnk_4C4928[nGroup], 0);
				// Both lookups are written as if/else with the
				// failure arm breaking: the original's `je` lands
				// on a `jmp` past the *whole* loop, so a missing
				// frame 00 stops the chaining of every later
				// family, it does not skip just this one.
				if (g_mapUnk_4A1D30.Lookup(sTail.szName, pFrst))
				{
					pLast = (CMerlinTexture*)pFrst;
					for (j1 = 1; ; j1++)
					{
						wsprintf(sTail.szName,
							"%s%02d",
							g_apszUnk_4C4928[nGroup], j1);
						pAfter = NULL;
						if (g_mapUnk_4A1D30.Lookup(sTail.szName,
							pAfter))
						{
							pLast->m_pUnk38 =
								(CMerlinTexture*)pAfter;
							pLast =
								(CMerlinTexture*)pAfter;
						}
						else
							break;
					}
					pLast->m_pUnk38 = (CMerlinTexture*)pFrst;
				}
				else
					break;
			}
			// floor pads and their walls
			for (sDoc.i = 0; sDoc.i < 8; sDoc.i++)
			{
				nSeq = 0;
				do
				{
					nK = 0;
					sprintf(sTail.szName, "%s%02d_%02d",
						g_apszUnk_4C48F0[sDoc.i], nSeq, nK);
					// One statement, not two: the original
					// materialises the `== NULL` result into a
					// frame temporary *before* the CString
					// temporary is destroyed and branches on it
					// afterwards (hover.exe:0x4161FD).
					if ((pTexEnt = (CMerlinStatic*)
						Hover_sub_41A4E0(CString(sTail.szName)))
						== NULL)
						break;
					switch (sDoc.i)
					{
					case 0:
						pFloorObj = new CHoldPad;
						break;
					case 1:
						pFloorObj = new CFlagRemover;
						break;
					case 2:
						pFloorObj = new CSledPad(0x100);
						break;
					case 3:
						pFloorObj = new CSledPad(0);
						break;
					case 4:
						pFloorObj = new CSledPad(0x180);
						break;
					case 5:
						pFloorObj = new CSledPad(0x80);
						break;
					case 6:
						pFloorObj = new CFloorObj(6);
						break;
					case 7:
						pFloorObj = new CFloorObj(7);
						break;
					}
					pSide1 = new CWall;
					pSide1->Sub_414D40((CObject*)pTexEnt);
					pFloorObj->m_list94.AddTail(pSide1);
					pFloorObj->m_dwUnk8C = (short)pTexEnt->m_wUnkA4;
					pSide1->m_dwUnk94 = pFloorObj->m_dwUnk8C;
					pFloorObj->m_dwUnk90 = (short)pTexEnt->m_wUnkA6;
					pSide1->m_dwUnk98 = pFloorObj->m_dwUnk90;
					while (++nK)
					{
						sprintf(sTail.szName, "%s%02d_%02d",
							g_apszUnk_4C48F0[sDoc.i], nSeq, nK);
						if ((pTexEnt = (CMerlinStatic*)
							Hover_sub_41A4E0(
							CString(sTail.szName))) == NULL)
							break;
						pSide2 = new CWall;
						pSide2->m_dwUnk94 =
							(short)pTexEnt->m_wUnkA4;
						pSide2->m_dwUnk98 =
							(short)pTexEnt->m_wUnkA6;
						pSide2->Sub_414D40((CObject*)pTexEnt);
						pFloorObj->m_list94.AddTail(pSide2);
					}
					nK--;
					// Link the wall chain's endpoints with a new
					// wall if they are not already the same point.
					if (PtAt(&((CWall*)pFloorObj->m_list94.GetTail())->
							m_obj04.m_dwUnk08) !=
						PtAt(&((CWall*)pFloorObj->m_list94.GetHead())->
							m_obj04.m_dwUnk00))
					{
						pSide3 = new CWall;
						pSide3->m_obj04.Sub_422BF0(
							(DWORD*)&PtAt(&((CWall*)pFloorObj->
								m_list94.GetTail())->m_obj04.
								m_dwUnk08),
							(DWORD*)&PtAt(&((CWall*)pFloorObj->
								m_list94.GetHead())->m_obj04.
								m_dwUnk00));
						posIt = pFloorObj->m_list94.GetHeadPosition();
						pSide4 = (CWall*)pFloorObj->m_list94.GetNext(
							posIt);
						if (sDoc.i == 7)
						{
							pSide3->m_dwUnk94 = pFloorObj->m_dwUnk8C;
							pSide3->m_dwUnk98 = pFloorObj->m_dwUnk8C;
						}
						else
						{
							pFloorObj->m_dwUnk8C = (short)(
								(CMerlinStatic*)pSide4->
								m_dwUnk78)->m_wUnkA4;
							pSide4 = (CWall*)pFloorObj->m_list94.
								GetNext(posIt);
							pFloorObj->m_dwUnk90 = (short)(
								(CMerlinStatic*)pSide4->
								m_dwUnk78)->m_wUnkA6;
						}
						pFloorObj->m_list94.AddTail(pSide3);
					}
					((CCollider*)m_dwUnk8388)->Sub_40F900(pFloorObj);
					m_list838C.AddTail(pFloorObj);
				}
				while (++nSeq);
			}
		}

		// maze wall records
		sHead.nWalls = ((CMerlinWorld*)m_dwUnk0188)->m_arrUnk14.GetSize();
		m_dwUnk83C8 = 0;
		for (sDoc.i = 0; sDoc.i < sHead.nWalls; sDoc.i++)
		{
			CMerlinStatic* pFace;

			pFace = (CMerlinStatic*)((CMerlinWorld*)m_dwUnk0188)->
				m_arrUnk14[sDoc.i];
			sDir.pWall = new CWall;
			sDir.pWall->Sub_414D40((CObject*)pFace);
			m_list838C.AddTail(sDir.pWall);
			if ((short)pFace->m_wUnkA4 > (int)m_dwUnk83C8)
				m_dwUnk83C8 = (short)pFace->m_wUnkA4;
			if ((short)pFace->m_wUnkA6 > (int)m_dwUnk83C8)
				m_dwUnk83C8 = (short)pFace->m_wUnkA6;
			// All three name tests are "!= 0", i.e. the name does
			// *not* start with the given prefix, and they are one
			// flat if/else-if: the
			// original materialises each _mbscmp result with
			// `je <zero arm>` (hover.exe:0x416B36, 0x416BC8,
			// 0x416D14), so a face whose name ends in "PAD_" takes
			// neither arm and everything else is either registered
			// with the collider or flagged.
			if (_mbscmp((const unsigned char*)(const char*)pFace->
					m_strUnk04.Left(strlen("PAD_")),
					(const unsigned char*)"PAD_") != 0 &&
				_mbscmp((const unsigned char*)(const char*)
					pFace->m_strUnk5C.Left(strlen("DECAL_")),
					(const unsigned char*)"DECAL_") != 0 &&
				// no `!= 0`: hover.exe:0x416c14 copies the member
				// raw into its temporary and tests that
				pFace->m_dwUnkBC)
			{
				((CCollider*)m_dwUnk8388)->Sub_40F900(sDir.pWall);
				if ((short)pFace->m_wUnkA4 > 0)
					sDir.pWall->m_dwUnk38 = 1;
			}
			else if (_mbscmp((const unsigned char*)(const char*)
					pFace->m_strUnk04.Left(strlen("PAD_")),
					(const unsigned char*)"PAD_") != 0)
				sDir.pWall->m_dwUnk38 = 1;
		}
		m_dwUnk83C4 = 0x40;
		m_dwUnk83CC = m_dwUnk83C8 - m_dwUnk83C4;

		if (m_dwUnk8498 == 0)
		{
			Hover_sub_423EA0();
			for (sDoc.i = 0; sDoc.i < (int)LVLPARAM((int)g_dwUnk_46049C).
				m_adwUnk0C[0]; sDoc.i++)
				this->Sub_412250(new CSpeedPod, rand() % 4 == 0);
			for (sDoc.i = 0; sDoc.i < (int)LVLPARAM((int)g_dwUnk_46049C).
				m_adwUnk0C[1]; sDoc.i++)
				this->Sub_412250(new CSlowPod, rand() % 4 == 0);
			for (sDoc.i = 0; sDoc.i < (int)LVLPARAM((int)g_dwUnk_46049C).
				m_adwUnk0C[2]; sDoc.i++)
				this->Sub_412250(new CMapEraserPod, rand() % 4 == 0);
			for (sDoc.i = 0; sDoc.i < (int)LVLPARAM((int)g_dwUnk_46049C).
				m_adwUnk0C[3]; sDoc.i++)
				this->Sub_412250(new CTempWallPod, rand() % 4 == 0);
			for (sDoc.i = 0; sDoc.i < (int)LVLPARAM((int)g_dwUnk_46049C).
				m_adwUnk0C[4]; sDoc.i++)
				this->Sub_412250(new CSmokeBombPod, rand() % 4 == 0);
			for (sDoc.i = 0; sDoc.i < (int)LVLPARAM((int)g_dwUnk_46049C).
				m_adwUnk0C[5]; sDoc.i++)
				this->Sub_412250(new CJumpPod, rand() % 4 == 0);
			for (sDoc.i = 0; sDoc.i < (int)LVLPARAM((int)g_dwUnk_46049C).
				m_adwUnk0C[6]; sDoc.i++)
				this->Sub_412250(new CInviciblePod, rand() % 4 == 0);
			for (sDoc.i = 0; sDoc.i < (int)LVLPARAM((int)g_dwUnk_46049C).m_dwUnk30;
				sDoc.i++)
				this->Sub_412250(new CFlag(0), 0);
			for (sDoc.i = 0; sDoc.i < (int)LVLPARAM((int)g_dwUnk_46049C).m_dwUnk30;
				sDoc.i++)
				this->Sub_412250(new CFlag(1), 0);
		}

		// fix up the wall flags (the original also carries dead "|= 8"
		// copies of these blocks; the reachable stores clear the bit)
		for (sHead.pos = m_list838C.GetHeadPosition(); sHead.pos != NULL;)
		{
			// pObj is a local of the loop body, not of the try block:
			// the original puts it at ebp-0x298, in the pool that
			// follows the try block's own (hover.exe:0x417550).
			CObject* pObj;

			pObj = (CObject*)m_list838C.GetNext(sHead.pos);
			if (!pObj->IsKindOf(RUNTIME_CLASS(CPodObj)))
			{
				if (((CWall*)pObj)->m_dwUnk78 != 0)
				{
					// HOVER_SHOWPADS is 0 in the shipped build, so the
					// original emits the |= 8 arm as dead code behind an
					// unconditional jump and only the &= ~8 arm runs.
					if (HOVER_SHOWPADS)
						((CMerlinStatic*)((CWall*)pObj)->m_dwUnk78)->
							m_wFlags |= 8;
					else
						((CMerlinStatic*)((CWall*)pObj)->m_dwUnk78)->
							m_wFlags &= ~8;
				}
			}
			if (pObj->IsKindOf(RUNTIME_CLASS(CWall)))
			{
				if (((CWall*)pObj)->m_dwUnk78 != 0)
				{
					if (HOVER_SHOWPADS)
						((CMerlinStatic*)((CWall*)pObj)->m_dwUnk78)->
							m_wFlags |= 8;
					else
						((CMerlinStatic*)((CWall*)pObj)->m_dwUnk78)->
							m_wFlags &= ~8;
				}
			}
		}
		sDoc.nRet = ((CMerlinWorld*)m_dwUnk0188)->Sub_418EA0();
		m_dwUnk0184 = 1;
		SetCursor(LoadCursor(NULL, (LPCTSTR)0x7F00));	// IDC_ARROW
	}
	catch (CException* e)
	{
		CBumperView* pView;

		SetCursor(LoadCursor(NULL, (LPCTSTR)0x7F00));	// IDC_ARROW
		pView = Hover_sub_401140();
		if (pView != NULL && pView->m_dwUnk0040 != 0)
		{
			TerminateThread(((CWinThread*)pView->m_dwUnk0040)->
				m_hThread, 0x5F);
			delete (CWinThread*)pView->m_dwUnk0040;
			pView->m_dwUnk0040 = 0;
		}
		if (g_pThreadUnk_4C56E0 != NULL)
		{
			TerminateThread(g_pThreadUnk_4C56E0->m_hThread, 0x5F);
			delete g_pThreadUnk_4C56E0;
			g_pThreadUnk_4C56E0 = NULL;
		}
		Hover_sub_4262B0(e);
	}
	return 1;
}
