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
								// g_pTextureHeap by CBumperDoc::OnOpenDocument)

DWORD g_dwUnk_4C4DA0;			// hover.exe:0x4C4DA0 texture count
DWORD g_dwUnk_4C4DA8;			// hover.exe:0x4C4DA8 textures loaded so far

// hover.exe:0x4C4DAC
// GLOBAL: HOVER 0x4c4dac
LPPROGRESSFN g_pfnUnk_4C4DAC;	// load progress callback

DWORD g_dwUnk_4A1D10;			// hover.exe:0x4A1D10 texture total

// GLOBAL: HOVER 0x4c57cc
DWORD g_dwUnk_4C57CC;			// hover.exe:0x4C57CC load progress base
// GLOBAL: HOVER 0x4c57d0
DWORD g_dwUnk_4C57D0 = 100;		// hover.exe:0x4C57D0 load progress range (in .data)

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
	m_strName.Empty();
}

// The body is empty -- only the m_strName CString is destroyed, by the
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
		*pAr << m_strName;
		pAr->WriteWord((WORD)0);
	}
	else
	{
		*pAr >> m_strName;
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
	LPPROGRESSFN pfnProgress;
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
	pAr->ReadWord((WORD*)&m_bTransparent);
	pAr->ReadWord((WORD*)&m_dwFrameCount);
	g_mapUnk_4A1D30[m_strName] = this;
	m_pFrames = (MERLIN_TEXFRAME*)g_pUnk_461D00;
	g_pUnk_461D00 = (BYTE*)g_pUnk_461D00 + m_dwFrameCount * 0x20;
	for (nLevel = 0; nLevel < (int)m_dwFrameCount; nLevel++)
	{
		m_pFrames[nLevel].m_dwWidth = (short)pAr->ReadWord();
		m_pFrames[nLevel].m_dwColMask = (short)pAr->ReadWord();
		m_pFrames[nLevel].m_dwHeight = (short)pAr->ReadWord();
		m_pFrames[nLevel].m_dwUnk04 = (short)pAr->ReadWord();
		m_pFrames[nLevel].m_dwUnk10 = (short)pAr->ReadWord();
		m_pFrames[nLevel].m_cbPixels = pAr->ReadDWord();
		if (m_pFrames[nLevel].m_cbPixels != 0)
		{
			if (nLevel < (int)g_dwMipBias && (int)m_dwFrameCount > 1)
			{
				// mip levels below the memory configuration mode are
				// skipped, not loaded
				for (i = 0; i < (int)m_pFrames[nLevel].m_cbPixels; i++)
					pAr->ReadByte();
			}
			else
			{
				m_pFrames[nLevel].m_pPixels = (BYTE*)g_pUnk_461D00;
				g_pUnk_461D00 = (BYTE*)g_pUnk_461D00 + m_pFrames[nLevel].m_cbPixels;
				ar.Read((void*)m_pFrames[nLevel].m_pPixels, m_pFrames[nLevel].m_cbPixels);
			}
		}
		else
			m_pFrames[nLevel].m_pPixels = 0;
		if (nLevel < (int)g_dwMipBias && (int)m_dwFrameCount > 1)
		{
			pAr->ReadDWord();	// skipped
			for (i = 0; i < (int)m_pFrames[nLevel].m_dwWidth; i++)
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
			m_pFrames[nLevel].m_pColumns = (tagTEXCOLUMN*)g_pUnk_461D00;
			// pPts comes off the heap cursor before it moves on:
			// hover.exe:0x412d27 reuses the value loaded at 0x412cfb.
			pPts = (BYTE*)m_pFrames[nLevel].m_pColumns + m_pFrames[nLevel].m_dwWidth * 8;
			g_pUnk_461D00 = (BYTE*)g_pUnk_461D00 +
				(m_pFrames[nLevel].m_dwWidth + dwSize) * 8;
			dwAccum = (DWORD)m_pFrames[nLevel].m_pPixels;
			for (i = 0; i < (int)m_pFrames[nLevel].m_dwWidth; i++)
			{
				nSpans = (short)pAr->ReadWord();
				*(WORD*)((BYTE*)m_pFrames[nLevel].m_pColumns + i * 8) = (WORD)nSpans;
				*(DWORD*)((BYTE*)m_pFrames[nLevel].m_pColumns + i * 8 + 4) =
					(DWORD)pPts;
				while (nSpans > 0)
				{
					wY = pAr->ReadWord();
					*(WORD*)pPts = wY;
					wX = pAr->ReadWord();
					*(DWORD*)(pPts + 4) = dwAccum;
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
	m_wWidth = (WORD)m_pFrames[0].m_dwWidth;
	m_wColMask = (WORD)m_pFrames[0].m_dwColMask;
	m_wHeight = (WORD)m_pFrames[0].m_dwHeight;
	m_wUnk1A = (WORD)m_pFrames[0].m_dwUnk04;
	m_dwUnk34 = 0;
	m_dwUnk24 = 0;
	m_dwUnk20 = (DWORD)m_pFrames[0].m_pPixels;
	// the count advances after the callback returns, as in hover.exe,
	// which calls through a register copy of the pointer
	pfnProgress = g_pfnUnk_4C4DAC;
	if (pfnProgress != NULL)
	{
		pfnProgress(g_dwUnk_4C4DA8, g_obArrUnk_4A1D08.GetSize() - 1);
		g_dwUnk_4C4DA8++;
	}
}

/////////////////////////////////////////////////////////////////////////////
// Texture-by-name lookups

// hover.exe:0x41A4E0
// FUNCTION: HOVER 0x41a4e0
CObject* Hover_FindStatic(const CString& strName)
{
	CObject* pOb;

	if (g_mapUnk_4A4578.Lookup(strName, pOb))
		return pOb;
	return NULL;
}

// hover.exe:0x40FE40
// FUNCTION: HOVER 0x40fe40
CObject* Hover_FindLocation(const CString& strName)
{
	CObject* pOb;

	if (g_mapUnk_4A4768.Lookup(strName, pOb))
		return pOb;
	return NULL;
}

/////////////////////////////////////////////////////////////////////////////
// CBumperDoc::PlaceObject — place a flag/pod object on a random free pad
// ("FLAG_HUMAN_"/"FLAG_ROBOT_" pads for flags, "POD_RANDOM_" pods
// otherwise); bUnk spawns a nested "POD06_" pod at the same point.

// hover.exe:0x412250 (the coverage table lists this function as 0x412256,
// the address after the 6-byte "mov eax,fs:0" SEH prefix)
// FUNCTION: HOVER 0x412250
void CBumperDoc::PlaceObject(CGameObj* pObj, int bUnk)
{
	CMerlinLocation* pTex;
	// The point handed to CCollider::IsSpotBlocked is built by hand rather
	// than through a POINT-returning accessor: VC 2.1 does not elide the
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
			if (((CPodObj*)pObj)->m_bRobotFlag != 0)
			{
				char szName[0x80];
				CMerlinLocation* pT;
				int nDist;

				nDist = 0x50;
				do
				{
					sprintf(szName, "%s%02d", g_apszSpawnFamilies[3],
						rand() % (int)m_adwSpawnPoints[3]);
					pT = (CMerlinLocation*)Hover_FindLocation(
						CString(szName));
				}
				while (pT != NULL &&
					(pt.x = (short)pT->m_wX,
					 pt.y = (short)pT->m_wY,
					 (m_pCollider)->IsSpotBlocked(pt, nDist)));
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
					sprintf(szName, "%s%02d", g_apszSpawnFamilies[2],
						rand() % (int)m_adwSpawnPoints[2]);
					pT = (CMerlinLocation*)Hover_FindLocation(
						CString(szName));
				}
				while (pT != NULL &&
					(pt.x = (short)pT->m_wX,
					 pt.y = (short)pT->m_wY,
					 (m_pCollider)->IsSpotBlocked(pt, nDist)));
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
				sprintf(szName, "%s%02d", g_apszSpawnFamilies[4],
					rand() % (int)m_adwSpawnPoints[4]);
				pT = (CMerlinLocation*)Hover_FindLocation(
					CString(szName));
			}
			while (pT != NULL &&
				(pt.x = (short)pT->m_wX,
				 pt.y = (short)pT->m_wY,
				 (m_pCollider)->IsSpotBlocked(pt, nDist)));
			pTex = pT;
		}
		((CPodObj*)pObj)->SetCellPos((short)pTex->m_wX,
			(short)pTex->m_wY);
		((CPodObj*)pObj)->m_bOnBoard = 1;
		((CPodObj*)pObj)->m_dwBaseZ = (short)pTex->m_wHeight;
		((CPodObj*)pObj)->QueueBeginFrame();
		(m_pCollider)->AddStatic((CPodObj*)pObj);
		m_listMazeObjs.AddTail((CPodObj*)pObj);
		return;
	}
	{
	char szName[0x80];
	CMerlinLocation* pT;
	int nDist;

	nDist = 0x50;
	do
	{
		sprintf(szName, "%s%02d", g_apszSpawnFamilies[4],
			rand() % (int)m_adwSpawnPoints[4]);
		pT = (CMerlinLocation*)Hover_FindLocation(CString(szName));
	}
	while (pT != NULL &&
		(pt.x = (short)pT->m_wX,
		 pt.y = (short)pT->m_wY,
		 (m_pCollider)->IsSpotBlocked(pt, nDist)));
	pTex = pT;
	}
	((CPodObj*)pObj)->SetCellPos((short)pTex->m_wX,
		(short)pTex->m_wY);
	((CPodObj*)pObj)->m_dwBaseZ = (short)pTex->m_wHeight;
	pPt = ((CPodObj*)pObj)->GetCellPos(adwPt);
	pNew = new CContainer((CPodObj*)pObj, pPt[0], pPt[1],
		((CPodObj*)pObj)->m_dwRadius);
	pNew->m_bOnBoard = 1;
	pNew->QueueBeginFrame();
	(m_pCollider)->AddStatic(pNew);
	m_listMazeObjs.AddTail(pNew);
}
/////////////////////////////////////////////////////////////////////////////
// Endpoint helpers for the loader's wall-chain fix-up.
//
// hover.exe reads each endpoint of a pad's wall chain as one 8-byte value
// into an *expression* temporary, not into a named local
// (hover.exe:0x41670A, 0x416732, 0x4167E4 and 0x41680C all store the pair
// into a slot below the loader's named region).  A by-value POINT
// returned value of an expanded inline is exactly such a temporary, so
// both places that need a pair read it through PtAt; the second
// (hover.exe:0x416825) hands the temporaries' addresses to SetEndpoints.
// Taking the address of a returned temporary (`&PtAt(...)`) is not
// standard C++; MSVC accepts it (warning C4238), and it is kept because
// that is the codegen being reproduced.

// hover.exe:0x41674B -- the ring-closed test, `<CPoint temporary> !=
// <POINT temporary>`: the object is filled first, the operator's by-value
// argument second.
inline CPoint PtAt(const DWORD* pdw)
{
	return *(CPoint*)pdw;
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

// Four more file-local inlines that hover.exe's loader is built out of.
// Each of them exists because the original materialises a temporary that
// the obvious spelling folds away, and because the loader's whole frame
// (0x44c, `this` at ebp-0x458, 65 references) only comes out right when
// the count of those temporaries is exact -- rules 405-408.

// hover.exe:0x416B3C.  The first of the three maze-face name tests puts
// its `_mbscmp(...) != 0` through a 1/0 ladder into ebp-0x3f4 and closes
// with an inline return `jmp`, so the comparison is a call, not a term
// written out in the `&&` chain.  Assigning to a local is what pins the
// flag: `return nCmp != 0;` is folded into the branch.
inline BOOL MbNe(int nCmp)
{
	BOOL b = (nCmp != 0);

	return b;
}

// hover.exe:0x416B10/0x416BA2/0x416CEE.  `CString::operator LPCTSTR` on
// the `Left()` temporary lands in a frame temporary of its own (two
// inline-return jumps back to back), where casting the temporary
// directly folds the `mov eax,[eax]` into the argument push.
inline LPCTSTR CStr(const CString& str)
{
	LPCTSTR psz = str;

	return psz;
}

// hover.exe:0x415D1D/0x415E7A/0x415FBD.  The spawn points hand
// CPlayer::Spawn/PlacePlayer an 8-byte point built by two `movsx`
// stores straight into the argument temporary, with nothing homed in
// between.  Only a constructor taking the *location pointer* does that:
// a POINT-returning accessor homes its own `pt` and copies (two dwords
// per site), `CPoint((short)p->m_wX, (short)p->m_wY)` homes the
// two ctor arguments (two dwords, rule 330) and `CPoint(DWORD)` homes
// the packed word pair and picks it apart with shr/and (one dword).  A
// pointer argument is folded exactly like `this`, so this form homes
// nothing.
struct LOCPT : public tagPOINT
{
	LOCPT(const CMerlinLocation* p)
	{
		x = (short)p->m_wX;
		y = (short)p->m_wY;
	}
};

// hover.exe's level-parameter wrap is an inline *function* whose result
// goes through a local, not hover.h's bare `?:` macro.  Every one of the
// loader's fifteen LVLPARAM sites closes with the `jmp $+5` of an inline
// return (hover.exe:0x41560C, 0x415A0E, 0x415BD8, 0x415C4B, 0x415DFF,
// 0x415F42 and the nine pod loops at 0x416DFA..0x41748D) and parks the
// index in a temporary that the address arithmetic then reloads.  The
// macro gets the two arms right but not the return jump, and an inline
// whose body is a bare `return <?:>;` is worse still: in a loop condition
// VC duplicates the whole compare into both arms and materialises no
// temporary at all.  Assigning the `?:` to a local first pins it.
// Only the loader uses LVLPARAM in this file; the hover.h spelling is
// restored below so nothing else changes.
inline int LvlIdx(int i)
{
	int n = (i < 16) ? i : (i % 3 + 15);

	return n;
}

#undef LVLPARAM
#define LVLPARAM(i) (g_lvlParams[LvlIdx(i)])

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
		m_dwRobotFlags = 0;
		// Two statements: the original re-reads the variable rather
		// than using time()'s return value (hover.exe:0x4154D5).
		time(&sTail.timeUnk);
		srand((unsigned)sTail.timeUnk);
		m_pWorld = new CMerlinWorld;
		if (theApp.m_dwMazeOrder == 0)
			nLev = g_dwLevel;
		else
			nLev = rand() % 0x14;
		if (g_dwLevel == 0x14)
			nLev = g_dwLevel;

		// palette + texture frames from mazes\TEXn.tex
		lstrcpy(sDir.szPath, theApp.m_strDataDir);
		if (m_bAttractMode != 0)
			lstrcat(sDir.szPath, g_lvlParams[nLev].m_pszTextures);
		else
			lstrcat(sDir.szPath, LVLPARAM(nLev).m_pszTextures);
		CFile fileTexture(sDir.szPath, CFile::modeRead | CFile::typeBinary);
		CArchive arTexIn(&fileTexture, CArchive::load, 0x200, NULL);
		sHead.pPal = (LOGPALETTE*)::operator new(LOGPAL256_SIZE);
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
		g_palGame.Detach();
		g_palGame.Attach(CreatePalette(sHead.pPal));
		delete sHead.pPal;
		g_pUnk_461D00 = g_pTextureHeap;
		g_obArrUnk_4A1D08.Serialize(arTexIn);
		if (g_cbTextureHeap + (BYTE*)g_pTextureHeap < (BYTE*)g_pUnk_461D00)
			OutputDebugString("Blowing fixed memory heap.  "
				"Increase MAXTEXTURESIZE in CTexture.h and "
				"re-compile.");
		arTexIn >> sTail.wCount;
		for (sK = 0; (short)sTail.wCount > sK; sK++)
			arTexIn >> sDir.by;	// comment, skipped
		arTexIn.Close();
		fileTexture.Close();

		// maze data
		lstrcpy(sDir.szPath, theApp.m_strDataDir);
		if (m_bAttractMode != 0)
			lstrcat(sDir.szPath, g_lvlParams[nLev].m_pszMaze);
		else
			lstrcat(sDir.szPath, LVLPARAM(nLev).m_pszMaze);
		CFile fileMaze(sDir.szPath, CFile::modeRead | CFile::typeBinary);
		CArchive arMaze(&fileMaze, CArchive::load, 0x200, NULL);
		(m_pWorld)->Serialize(arMaze);
		arMaze.Close();
		fileMaze.Close();
		(m_pWorld)->BuildIdentityPalette();
		g_dwUnk_4C57CC = 0x3C;
		g_dwUnk_4C57D0 = 0x46;
		m_aShade[0].BuildTable(&g_palGame,
			m_dblShadeBase, 0xFF, 0xFF, 0xFF, m_dblShadeRed, m_dblShadeBlue,
			m_dblShadeGreen);
		g_dwUnk_4C57CC = 0x46;
		g_dwUnk_4C57D0 = 0x50;
		m_aShade[1].BuildTable(
			&g_palGame, m_dblShadeBase, 0xFF, 0xFF, 0xFF, 0.8,
			1.5, 0.8);
		if (LVLPARAM(nLev).m_pszMusic != NULL)
		{
			lstrcpy(sDir.szPath, theApp.m_strDataDir);
			lstrcat(sDir.szPath, LVLPARAM(nLev).m_pszMusic);
			m_music.OpenMusic(sDir.szPath);
		}
		m_pCollider = new CCollider(CSize(0x5A00, 0x5A00),
			0x100);
		this->CountSpawnPoints();
		pLoc = (CMerlinLocation*)this->GetRandomSpawnPoint(0);
		// The by-value point is built by LOCPT's constructor, which
		// is expanded straight into the argument temporary at
		// ebp-0x2bc (hover.exe:0x415D1D) and homes nothing.
		m_humanPlayer.Spawn(LOCPT(pLoc),
			(short)pLoc->m_wAngle, (short)pLoc->m_wHeight);
		m_listActiveObjs.AddHead(&m_humanPlayer);
		(m_pCollider)->AddMover(&m_humanPlayer);

		if (m_bAttractMode == 0)
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

			Hover_CacheDroneFrames();
			// robot players
			for (sDoc.i = 0; sDoc.i < (int)LVLPARAM((int)g_dwLevel).m_dwUnk28;
				sDoc.i++)
			{
				pLoc1 = (CMerlinLocation*)this->GetRandomSpawnPoint(1);
				pDrone = new CRobotPlayer(1);
				pDrone->PlacePlayer(LOCPT(pLoc1),
					(short)pLoc1->m_wAngle,
					(short)pLoc1->m_wHeight);
				m_listActiveObjs.AddTail(pDrone);
				(m_pCollider)->AddMover(pDrone);
			}
			for (sDoc.i = 0; sDoc.i < (int)LVLPARAM((int)g_dwLevel).m_dwUnk2C;
				sDoc.i++)
			{
				pLoc2 = (CMerlinLocation*)this->GetRandomSpawnPoint(1);
				pDrone2 = new CRobotPlayer(0);
				pDrone2->PlacePlayer(LOCPT(pLoc2),
					(short)pLoc2->m_wAngle,
					(short)pLoc2->m_wHeight);
				m_listActiveObjs.AddTail(pDrone2);
				(m_pCollider)->AddMover(pDrone2);
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
							pLast->m_pNextFrame =
								(CMerlinTexture*)pAfter;
							pLast =
								(CMerlinTexture*)pAfter;
						}
						else
							break;
					}
					pLast->m_pNextFrame = (CMerlinTexture*)pFrst;
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
						Hover_FindStatic(CString(sTail.szName)))
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
					pSide1->SetFace(pTexEnt);
					pFloorObj->m_listEdges.AddTail(pSide1);
					pFloorObj->m_dwLowHeight = (short)pTexEnt->m_wBottom;
					pSide1->m_dwBottom = pFloorObj->m_dwLowHeight;
					pFloorObj->m_dwHighHeight = (short)pTexEnt->m_wTop;
					pSide1->m_dwTop = pFloorObj->m_dwHighHeight;
					while (++nK)
					{
						sprintf(sTail.szName, "%s%02d_%02d",
							g_apszUnk_4C48F0[sDoc.i], nSeq, nK);
						if ((pTexEnt = (CMerlinStatic*)
							Hover_FindStatic(
							CString(sTail.szName))) == NULL)
							break;
						pSide2 = new CWall;
						pSide2->m_dwBottom =
							(short)pTexEnt->m_wBottom;
						pSide2->m_dwTop =
							(short)pTexEnt->m_wTop;
						pSide2->SetFace(pTexEnt);
						pFloorObj->m_listEdges.AddTail(pSide2);
					}
					nK--;
					// Link the wall chain's endpoints with a new
					// wall if they are not already the same point.
					if (PtAt(&((CWall*)pFloorObj->m_listEdges.GetTail())->
							m_seg.m_dwX2) !=
						PtAt(&((CWall*)pFloorObj->m_listEdges.GetHead())->
							m_seg.m_dwX1))
					{
						pSide3 = new CWall;
						pSide3->m_seg.SetEndpoints(
							(DWORD*)&PtAt(&((CWall*)pFloorObj->
								m_listEdges.GetTail())->m_seg.
								m_dwX2),
							(DWORD*)&PtAt(&((CWall*)pFloorObj->
								m_listEdges.GetHead())->m_seg.
								m_dwX1));
						posIt = pFloorObj->m_listEdges.GetHeadPosition();
						pSide4 = (CWall*)pFloorObj->m_listEdges.GetNext(
							posIt);
						if (sDoc.i == 7)
						{
							pSide3->m_dwBottom = pFloorObj->m_dwLowHeight;
							pSide3->m_dwTop = pFloorObj->m_dwLowHeight;
						}
						else
						{
							pFloorObj->m_dwLowHeight = (short)(
								(CMerlinStatic*)pSide4->
								m_pMerlin)->m_wBottom;
							pSide4 = (CWall*)pFloorObj->m_listEdges.
								GetNext(posIt);
							pFloorObj->m_dwHighHeight = (short)(
								(CMerlinStatic*)pSide4->
								m_pMerlin)->m_wTop;
						}
						pFloorObj->m_listEdges.AddTail(pSide3);
					}
					(m_pCollider)->AddStatic(pFloorObj);
					m_listMazeObjs.AddTail(pFloorObj);
				}
				while (++nSeq);
			}
		}

		// maze wall records
		sHead.nWalls = (m_pWorld)->m_arrStatics.GetSize();
		m_dwMaxWallHeight = 0;
		for (sDoc.i = 0; sDoc.i < sHead.nWalls; sDoc.i++)
		{
			CMerlinStatic* pFace;

			pFace = (CMerlinStatic*)(m_pWorld)->
				m_arrStatics[sDoc.i];
			sDir.pWall = new CWall;
			sDir.pWall->SetFace(pFace);
			m_listMazeObjs.AddTail(sDir.pWall);
			if ((short)pFace->m_wBottom > (int)m_dwMaxWallHeight)
				m_dwMaxWallHeight = (short)pFace->m_wBottom;
			if ((short)pFace->m_wTop > (int)m_dwMaxWallHeight)
				m_dwMaxWallHeight = (short)pFace->m_wTop;
			// All three name tests are "!= 0", i.e. the name does
			// *not* start with the given prefix, and they are one
			// flat if/else-if: the
			// original materialises each _mbscmp result with
			// `je <zero arm>` (hover.exe:0x416B36, 0x416BC8,
			// 0x416D14), so a face whose name ends in "PAD_" takes
			// neither arm and everything else is either registered
			// with the collider or flagged.
			// The three sites are not spelled alike because the
			// original's frame is not symmetric either: site one
			// keeps three temporaries (the `!= 0` flag at
			// ebp-0x3f4 between the string and the CString
			// pointer) and sites two and three keep two.  MbNe
			// buys three slots, CStr two.
			if (MbNe(_mbscmp((const unsigned char*)(const char*)pFace->
					m_strName.Left(strlen("PAD_")),
					(const unsigned char*)"PAD_")) &&
				_mbscmp((const unsigned char*)CStr(
					pFace->m_strFrontTex.Left(strlen("DECAL_"))),
					(const unsigned char*)"DECAL_") != 0 &&
				// no `!= 0`: hover.exe:0x416c14 copies the member
				// raw into its temporary and tests that
				pFace->m_bSolid)
			{
				(m_pCollider)->AddStatic(sDir.pWall);
				if ((short)pFace->m_wBottom > 0)
					sDir.pWall->m_bHideOnMap = 1;
			}
			else if (_mbscmp((const unsigned char*)CStr(
					pFace->m_strName.Left(strlen("PAD_"))),
					(const unsigned char*)"PAD_") != 0)
				sDir.pWall->m_bHideOnMap = 1;
		}
		m_dwFloorHeight = 0x40;
		m_dwHeightRange = m_dwMaxWallHeight - m_dwFloorHeight;

		if (m_bAttractMode == 0)
		{
			Hover_BuildBeacons();
			for (sDoc.i = 0; sDoc.i < (int)LVLPARAM((int)g_dwLevel).
				m_adwUnk0C[0]; sDoc.i++)
				this->PlaceObject(new CSpeedPod, rand() % 4 == 0);
			for (sDoc.i = 0; sDoc.i < (int)LVLPARAM((int)g_dwLevel).
				m_adwUnk0C[1]; sDoc.i++)
				this->PlaceObject(new CSlowPod, rand() % 4 == 0);
			for (sDoc.i = 0; sDoc.i < (int)LVLPARAM((int)g_dwLevel).
				m_adwUnk0C[2]; sDoc.i++)
				this->PlaceObject(new CMapEraserPod, rand() % 4 == 0);
			for (sDoc.i = 0; sDoc.i < (int)LVLPARAM((int)g_dwLevel).
				m_adwUnk0C[3]; sDoc.i++)
				this->PlaceObject(new CTempWallPod, rand() % 4 == 0);
			for (sDoc.i = 0; sDoc.i < (int)LVLPARAM((int)g_dwLevel).
				m_adwUnk0C[4]; sDoc.i++)
				this->PlaceObject(new CSmokeBombPod, rand() % 4 == 0);
			for (sDoc.i = 0; sDoc.i < (int)LVLPARAM((int)g_dwLevel).
				m_adwUnk0C[5]; sDoc.i++)
				this->PlaceObject(new CJumpPod, rand() % 4 == 0);
			for (sDoc.i = 0; sDoc.i < (int)LVLPARAM((int)g_dwLevel).
				m_adwUnk0C[6]; sDoc.i++)
				this->PlaceObject(new CInviciblePod, rand() % 4 == 0);
			for (sDoc.i = 0; sDoc.i < LVLPARAM((int)g_dwLevel).m_dwUnk30;
				sDoc.i++)
				this->PlaceObject(new CFlag(0), 0);
			for (sDoc.i = 0; sDoc.i < LVLPARAM((int)g_dwLevel).m_dwUnk30;
				sDoc.i++)
				this->PlaceObject(new CFlag(1), 0);
		}

		// fix up the wall flags (the original also carries dead "|= 8"
		// copies of these blocks; the reachable stores clear the bit)
		for (sHead.pos = m_listMazeObjs.GetHeadPosition(); sHead.pos != NULL;)
		{
			// pObj is a local of the loop body, not of the try block:
			// the original puts it at ebp-0x298, in the pool that
			// follows the try block's own (hover.exe:0x417550).
			CObject* pObj;

			pObj = (CObject*)m_listMazeObjs.GetNext(sHead.pos);
			if (!pObj->IsKindOf(RUNTIME_CLASS(CPodObj)))
			{
				if (((CWall*)pObj)->m_pMerlin != 0)
				{
					// HOVER_SHOWPADS is 0 in the shipped build, so the
					// original emits the |= 8 arm as dead code behind an
					// unconditional jump and only the &= ~8 arm runs.
					if (HOVER_SHOWPADS)
						((CMerlinStatic*)((CWall*)pObj)->m_pMerlin)->
							m_wFlags |= 8;
					else
						((CMerlinStatic*)((CWall*)pObj)->m_pMerlin)->
							m_wFlags &= ~8;
				}
			}
			if (pObj->IsKindOf(RUNTIME_CLASS(CWall)))
			{
				if (((CWall*)pObj)->m_pMerlin != 0)
				{
					if (HOVER_SHOWPADS)
						((CMerlinStatic*)((CWall*)pObj)->m_pMerlin)->
							m_wFlags |= 8;
					else
						((CMerlinStatic*)((CWall*)pObj)->m_pMerlin)->
							m_wFlags &= ~8;
				}
			}
		}
		sDoc.nRet = (m_pWorld)->GetTextureBytes();
		m_bLevelLoaded = 1;
		SetCursor(LoadCursor(NULL, (LPCTSTR)0x7F00));	// IDC_ARROW
	}
	catch (CException* e)
	{
		CBumperView* pView;

		SetCursor(LoadCursor(NULL, (LPCTSTR)0x7F00));	// IDC_ARROW
		pView = Hover_GetView();
		if (pView != NULL && pView->m_pRenderThread != 0)
		{
			TerminateThread((pView->m_pRenderThread)->
				m_hThread, 0x5F);
			delete pView->m_pRenderThread;
			pView->m_pRenderThread = 0;
		}
		if (g_pSoundThread != NULL)
		{
			TerminateThread(g_pSoundThread->m_hThread, 0x5F);
			delete g_pSoundThread;
			g_pSoundThread = NULL;
		}
		Hover_FatalException(e);
	}
	return 1;
}

#undef LVLPARAM
#define LVLPARAM(i) (g_lvlParams[((i) < 16) ? (i) : ((i) % 3 + 15)])
