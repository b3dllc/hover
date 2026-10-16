// texload.cpp : loader/helper functions decompiled from hover.exe
//
// See texload.h for the class layouts and RTTI/vtable data. The MIDI
// stream loader (CMidiStream::LoadMusicFile) parses the RIFF "MIDS" .muz
// music files; CCollider/CRobotPlayer back the level loader's placement
// code.

#include "hover.h"
#include "bumperdoc.h"
#include "bumpview.h"
#include "gameobj.h"
#include "texture.h"
#include "texload.h"
#include "mazeobj.h"
#include "texmisc.h"
#include "spandraw.h"
#include "texheap.h"

#include <string.h>
#include <stdlib.h>
#include <math.h>

// C4355: the AI component members legitimately receive the outer 'this'
// in the member initializer list of CRobotPlayer::CRobotPlayer — the
// original does the same (each component constructor takes the parent
// pointer, hover.exe:0x4220A0 passes it to hover.exe:0x4235E0 etc.).
#pragma warning(disable : 4355)

// A 16.16 fixed-point dword the way hover.exe reads one out of memory:
// the integer half as a signed short at +2, the fraction out of the dword
// copy (notes/perfect-match.md rule 46).

typedef union tagTEXFIXED1616
{
	DWORD dw;
	struct
	{
		WORD wFrac;		// +0 fraction (1/65536 units)
		short sInt;		// +2 signed integer part
	} s;
} FIXED1616;

IMPLEMENT_DYNAMIC(CRegionMatrix, CObject)
IMPLEMENT_DYNAMIC(CCollider, CObject)
IMPLEMENT_DYNAMIC(CRobotPlayer, CPlayer)

/////////////////////////////////////////////////////////////////////////////
// Globals

// hover.exe:0x4C5904
// GLOBAL: HOVER 0x4c5904
CMidiStream* g_pCurrentMidiStream;		// object that opened the current MIDI stream

// hover.exe:0x4C5908
// GLOBAL: HOVER 0x4c5908
LPMIDIDEVICEFN g_pfnMidiStreamOpen;	// midiStreamOpen

// hover.exe:0x4C5910
// GLOBAL: HOVER 0x4c5910
LPMIDIDEVICEFN g_pfnMidiStreamProperty;	// midiStreamProperty

// hover.exe:0x4C5914
// GLOBAL: HOVER 0x4c5914
LPMIDIDEVICEFN g_pfnMidiStreamPosition;	// midiStreamPosition

// hover.exe:0x4C5918
// GLOBAL: HOVER 0x4c5918
LPMIDIDEVICEFN g_pfnMidiStreamOut;	// midiStreamOut

// hover.exe:0x4C5920
// GLOBAL: HOVER 0x4c5920
LPMIDIDEVICEFN g_pfnMidiStreamRestart;	// midiStreamRestart

// hover.exe:0x4C5924
// GLOBAL: HOVER 0x4c5924
LPMIDIDEVICEFN g_pfnMidiStreamStop;	// midiStreamStop

// hover.exe:0x4C47B0
// GLOBAL: HOVER 0x4c47b0
char g_szWinMM[] = "winmm.dll";

// hover.exe:0x4C5928
// GLOBAL: HOVER 0x4c5928
char g_szMidiStreamStop[] = "midiStreamStop";

// hover.exe:0x4C5938
// GLOBAL: HOVER 0x4c5938
char g_szMidiStreamRestart[] = "midiStreamRestart";

// hover.exe:0x4C594C
// GLOBAL: HOVER 0x4c594c
char g_szMidiStreamPause[] = "midiStreamPause";

// hover.exe:0x4C595C
// GLOBAL: HOVER 0x4c595c
char g_szMidiStreamOut[] = "midiStreamOut";

// hover.exe:0x4C596C
// GLOBAL: HOVER 0x4c596c
char g_szMidiStreamPosition[] = "midiStreamPosition";

// hover.exe:0x4C5980
// GLOBAL: HOVER 0x4c5980
char g_szMidiStreamProperty[] = "midiStreamProperty";

// hover.exe:0x4C5994
// GLOBAL: HOVER 0x4c5994
char g_szMidiStreamClose[] = "midiStreamClose";

// hover.exe:0x4C59A4
// GLOBAL: HOVER 0x4c59a4
char g_szMidiStreamOpen[] = "midiStreamOpen";

// hover.exe:0x4C54D4
// GLOBAL: HOVER 0x4c54d4
DWORD g_dwRobotPoolBlock = 0xA;		// CRobotPlayer pool chunk size

// hover.exe:0x4C5624
// GLOBAL: HOVER 0x4c5624
DWORD g_dwSpritePoolBlock = 0x20;	// renderer-sprite pool chunk size

// hover.exe:0x4C4DB0
// GLOBAL: HOVER 0x4c4db0
WORD g_awSquares[512] =		// g_awSquares[255 + d] = d * d
{
	0xFE01, 0xFC04, 0xFA09, 0xF810, 0xF619, 0xF424, 0xF231, 0xF040,
	0xEE51, 0xEC64, 0xEA79, 0xE890, 0xE6A9, 0xE4C4, 0xE2E1, 0xE100,
	0xDF21, 0xDD44, 0xDB69, 0xD990, 0xD7B9, 0xD5E4, 0xD411, 0xD240,
	0xD071, 0xCEA4, 0xCCD9, 0xCB10, 0xC949, 0xC784, 0xC5C1, 0xC400,
	0xC241, 0xC084, 0xBEC9, 0xBD10, 0xBB59, 0xB9A4, 0xB7F1, 0xB640,
	0xB491, 0xB2E4, 0xB139, 0xAF90, 0xADE9, 0xAC44, 0xAAA1, 0xA900,
	0xA761, 0xA5C4, 0xA429, 0xA290, 0xA0F9, 0x9F64, 0x9DD1, 0x9C40,
	0x9AB1, 0x9924, 0x9799, 0x9610, 0x9489, 0x9304, 0x9181, 0x9000,
	0x8E81, 0x8D04, 0x8B89, 0x8A10, 0x8899, 0x8724, 0x85B1, 0x8440,
	0x82D1, 0x8164, 0x7FF9, 0x7E90, 0x7D29, 0x7BC4, 0x7A61, 0x7900,
	0x77A1, 0x7644, 0x74E9, 0x7390, 0x7239, 0x70E4, 0x6F91, 0x6E40,
	0x6CF1, 0x6BA4, 0x6A59, 0x6910, 0x67C9, 0x6684, 0x6541, 0x6400,
	0x62C1, 0x6184, 0x6049, 0x5F10, 0x5DD9, 0x5CA4, 0x5B71, 0x5A40,
	0x5911, 0x57E4, 0x56B9, 0x5590, 0x5469, 0x5344, 0x5221, 0x5100,
	0x4FE1, 0x4EC4, 0x4DA9, 0x4C90, 0x4B79, 0x4A64, 0x4951, 0x4840,
	0x4731, 0x4624, 0x4519, 0x4410, 0x4309, 0x4204, 0x4101, 0x4000,
	0x3F01, 0x3E04, 0x3D09, 0x3C10, 0x3B19, 0x3A24, 0x3931, 0x3840,
	0x3751, 0x3664, 0x3579, 0x3490, 0x33A9, 0x32C4, 0x31E1, 0x3100,
	0x3021, 0x2F44, 0x2E69, 0x2D90, 0x2CB9, 0x2BE4, 0x2B11, 0x2A40,
	0x2971, 0x28A4, 0x27D9, 0x2710, 0x2649, 0x2584, 0x24C1, 0x2400,
	0x2341, 0x2284, 0x21C9, 0x2110, 0x2059, 0x1FA4, 0x1EF1, 0x1E40,
	0x1D91, 0x1CE4, 0x1C39, 0x1B90, 0x1AE9, 0x1A44, 0x19A1, 0x1900,
	0x1861, 0x17C4, 0x1729, 0x1690, 0x15F9, 0x1564, 0x14D1, 0x1440,
	0x13B1, 0x1324, 0x1299, 0x1210, 0x1189, 0x1104, 0x1081, 0x1000,
	0x0F81, 0x0F04, 0x0E89, 0x0E10, 0x0D99, 0x0D24, 0x0CB1, 0x0C40,
	0x0BD1, 0x0B64, 0x0AF9, 0x0A90, 0x0A29, 0x09C4, 0x0961, 0x0900,
	0x08A1, 0x0844, 0x07E9, 0x0790, 0x0739, 0x06E4, 0x0691, 0x0640,
	0x05F1, 0x05A4, 0x0559, 0x0510, 0x04C9, 0x0484, 0x0441, 0x0400,
	0x03C1, 0x0384, 0x0349, 0x0310, 0x02D9, 0x02A4, 0x0271, 0x0240,
	0x0211, 0x01E4, 0x01B9, 0x0190, 0x0169, 0x0144, 0x0121, 0x0100,
	0x00E1, 0x00C4, 0x00A9, 0x0090, 0x0079, 0x0064, 0x0051, 0x0040,
	0x0031, 0x0024, 0x0019, 0x0010, 0x0009, 0x0004, 0x0001, 0x0000,
	0x0001, 0x0004, 0x0009, 0x0010, 0x0019, 0x0024, 0x0031, 0x0040,
	0x0051, 0x0064, 0x0079, 0x0090, 0x00A9, 0x00C4, 0x00E1, 0x0100,
	0x0121, 0x0144, 0x0169, 0x0190, 0x01B9, 0x01E4, 0x0211, 0x0240,
	0x0271, 0x02A4, 0x02D9, 0x0310, 0x0349, 0x0384, 0x03C1, 0x0400,
	0x0441, 0x0484, 0x04C9, 0x0510, 0x0559, 0x05A4, 0x05F1, 0x0640,
	0x0691, 0x06E4, 0x0739, 0x0790, 0x07E9, 0x0844, 0x08A1, 0x0900,
	0x0961, 0x09C4, 0x0A29, 0x0A90, 0x0AF9, 0x0B64, 0x0BD1, 0x0C40,
	0x0CB1, 0x0D24, 0x0D99, 0x0E10, 0x0E89, 0x0F04, 0x0F81, 0x1000,
	0x1081, 0x1104, 0x1189, 0x1210, 0x1299, 0x1324, 0x13B1, 0x1440,
	0x14D1, 0x1564, 0x15F9, 0x1690, 0x1729, 0x17C4, 0x1861, 0x1900,
	0x19A1, 0x1A44, 0x1AE9, 0x1B90, 0x1C39, 0x1CE4, 0x1D91, 0x1E40,
	0x1EF1, 0x1FA4, 0x2059, 0x2110, 0x21C9, 0x2284, 0x2341, 0x2400,
	0x24C1, 0x2584, 0x2649, 0x2710, 0x27D9, 0x28A4, 0x2971, 0x2A40,
	0x2B11, 0x2BE4, 0x2CB9, 0x2D90, 0x2E69, 0x2F44, 0x3021, 0x3100,
	0x31E1, 0x32C4, 0x33A9, 0x3490, 0x3579, 0x3664, 0x3751, 0x3840,
	0x3931, 0x3A24, 0x3B19, 0x3C10, 0x3D09, 0x3E04, 0x3F01, 0x4000,
	0x4101, 0x4204, 0x4309, 0x4410, 0x4519, 0x4624, 0x4731, 0x4840,
	0x4951, 0x4A64, 0x4B79, 0x4C90, 0x4DA9, 0x4EC4, 0x4FE1, 0x5100,
	0x5221, 0x5344, 0x5469, 0x5590, 0x56B9, 0x57E4, 0x5911, 0x5A40,
	0x5B71, 0x5CA4, 0x5DD9, 0x5F10, 0x6049, 0x6184, 0x62C1, 0x6400,
	0x6541, 0x6684, 0x67C9, 0x6910, 0x6A59, 0x6BA4, 0x6CF1, 0x6E40,
	0x6F91, 0x70E4, 0x7239, 0x7390, 0x74E9, 0x7644, 0x77A1, 0x7900,
	0x7A61, 0x7BC4, 0x7D29, 0x7E90, 0x7FF9, 0x8164, 0x82D1, 0x8440,
	0x85B1, 0x8724, 0x8899, 0x8A10, 0x8B89, 0x8D04, 0x8E81, 0x9000,
	0x9181, 0x9304, 0x9489, 0x9610, 0x9799, 0x9924, 0x9AB1, 0x9C40,
	0x9DD1, 0x9F64, 0xA0F9, 0xA290, 0xA429, 0xA5C4, 0xA761, 0xA900,
	0xAAA1, 0xAC44, 0xADE9, 0xAF90, 0xB139, 0xB2E4, 0xB491, 0xB640,
	0xB7F1, 0xB9A4, 0xBB59, 0xBD10, 0xBEC9, 0xC084, 0xC241, 0xC400,
	0xC5C1, 0xC784, 0xC949, 0xCB10, 0xCCD9, 0xCEA4, 0xD071, 0xD240,
	0xD411, 0xD5E4, 0xD7B9, 0xD990, 0xDB69, 0xDD44, 0xDF21, 0xE100,
	0xE2E1, 0xE4C4, 0xE6A9, 0xE890, 0xEA79, 0xEC64, 0xEE51, 0xF040,
	0xF231, 0xF424, 0xF619, 0xF810, 0xFA09, 0xFC04, 0xFE01, 0x0000
};

/////////////////////////////////////////////////////////////////////////////
// CMidiStream

// Original bug: m_hFile is never closed, on success or failure, so every
// level's music load leaks a file handle.

// hover.exe:0x437F00
// FUNCTION: HOVER 0x437f00
int CMidiStream::LoadMusicFile(const char* pszPath)
{
	int bLoaded;
	DWORD dwRead;
	DWORD dwA;
	DWORD dwB;
	DWORD dwBlocks;
	DWORD adwHdr[2];
	MIDIHDR* pNode;
	HMODULE hMod;

	bLoaded = 0;
	CloseStream();
	if (g_pfnMidiStreamOpen == NULL)
	{
		hMod = GetModuleHandleA(g_szWinMM);
		if (hMod == NULL)
			return 0;
		g_pfnMidiStreamOpen =
			(LPMIDIDEVICEFN)GetProcAddress(hMod, g_szMidiStreamOpen);
		g_pfnMidiStreamClose =
			(LPMIDIDEVICEFN)GetProcAddress(hMod, g_szMidiStreamClose);
		g_pfnMidiStreamProperty =
			(LPMIDIDEVICEFN)GetProcAddress(hMod, g_szMidiStreamProperty);
		g_pfnMidiStreamPosition =
			(LPMIDIDEVICEFN)GetProcAddress(hMod, g_szMidiStreamPosition);
		g_pfnMidiStreamOut =
			(LPMIDIDEVICEFN)GetProcAddress(hMod, g_szMidiStreamOut);
		g_pfnMidiStreamPause =
			(LPMIDIDEVICEFN)GetProcAddress(hMod, g_szMidiStreamPause);
		g_pfnMidiStreamRestart =
			(LPMIDIDEVICEFN)GetProcAddress(hMod, g_szMidiStreamRestart);
		g_pfnMidiStreamStop =
			(LPMIDIDEVICEFN)GetProcAddress(hMod, g_szMidiStreamStop);
		if (g_pfnMidiStreamOpen == NULL || g_pfnMidiStreamClose == NULL ||
			g_pfnMidiStreamProperty == NULL || g_pfnMidiStreamPosition == NULL ||
			g_pfnMidiStreamOut == NULL || g_pfnMidiStreamPause == NULL ||
			g_pfnMidiStreamRestart == NULL || g_pfnMidiStreamStop == NULL)
		{
			g_pfnMidiStreamOpen = NULL;
			return 0;
		}
	}
	m_hFile = CreateFileA(pszPath, GENERIC_READ, FILE_SHARE_READ,
		NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (m_hFile == INVALID_HANDLE_VALUE)
		goto lFail;
	if (!ReadFile(m_hFile, &dwA, sizeof(dwA), &dwRead, NULL))
		goto lFail;
	if (dwA != 0x46464952)		// "RIFF"
		goto lFail;
	if (!ReadFile(m_hFile, &dwB, sizeof(dwB), &dwRead, NULL))
		goto lFail;
	if (!ReadFile(m_hFile, &dwA, sizeof(dwA), &dwRead, NULL))
		goto lFail;
	if (dwA != 0x5344494D)		// "MIDS"
		goto lFail;
	if (!ReadFile(m_hFile, &dwA, sizeof(dwA), &dwRead, NULL))
		goto lFail;
	if (!ReadFile(m_hFile, &dwB, sizeof(dwB), &dwRead, NULL))
		goto lFail;
	if (dwA != 0x20746D66)		// "fmt "
		goto lFail;
	if (dwB < 8)
		goto lFail;
	// The first two fields of the MIDS format chunk fill m_dwTimeDiv and
	// m_cbMaxBuffer.
	if (!ReadFile(m_hFile, &m_dwTimeDiv, 8, &dwRead, NULL))
		goto lFail;
	if (dwB > 8)
		SetFilePointer(m_hFile, dwB - 8, NULL, FILE_CURRENT);
	if (!ReadFile(m_hFile, &dwA, sizeof(dwA), &dwRead, NULL))
		goto lFail;
	if (!ReadFile(m_hFile, &dwB, sizeof(dwB), &dwRead, NULL))
		goto lFail;
	if (dwA != 0x61746164)		// "data"
		goto lFail;
	if (!ReadFile(m_hFile, &dwBlocks, sizeof(dwBlocks), &dwRead, NULL))
		goto lFail;
	m_pFreeHead = 0;
	m_pFreeTail = 0;
	while (dwBlocks-- != 0)
	{
		// per block: 8-byte header (only the second dword, the data
		// length, is used), then the MIDI stream data
		if (!ReadFile(m_hFile, adwHdr, sizeof(adwHdr), &dwRead, NULL))
			goto lFail;
		// The header and the stream data share one block.
		pNode = (MIDIHDR*)GlobalLock(GlobalAlloc(
			GMEM_MOVEABLE | GMEM_ZEROINIT | GMEM_DDESHARE,
			adwHdr[1] + sizeof(MIDIHDR)));
		if (pNode == NULL)
		{
			AfxThrowMemoryException();
			if (pNode == NULL)
				goto lFail;
		}
		LPSTR pData = (LPSTR)(pNode + 1);
		pNode->lpData = pData;
		pNode->dwBufferLength = adwHdr[1];
		pNode->dwBytesRecorded = adwHdr[1];
		if (!ReadFile(m_hFile, pData, adwHdr[1],
			&dwRead, NULL))
			goto lFail;
		if (m_pFreeTail == 0)
		{
			m_pFreeTail = pNode;
			m_pFreeHead = pNode;
		}
		else
		{
			(m_pFreeTail)->lpNext = pNode;
			m_pFreeTail = pNode;
		}
	}
	bLoaded = 1;
lFail:
	if (bLoaded == 0)
		CloseStream();
	return bLoaded;
}

// hover.exe:0x41C030
// FUNCTION: HOVER 0x41c030
int CMidiStream::OpenMusic(const char* pszPath)
{
	if (LoadMusicFile(pszPath) == 0)
		return 0;
	g_pCurrentMidiStream = this;
	return 1;
}

/////////////////////////////////////////////////////////////////////////////
// CRegionMatrix

// hover.exe:0x415030
// FUNCTION: HOVER 0x415030
tagREGIONNODE::tagREGIONNODE()
{
	m_pNext = NULL;
	m_pPrev = NULL;
	m_pNextObj = NULL;
	m_pObj = NULL;
}

// hover.exe:0x41B4F0
// FUNCTION: HOVER 0x41b4f0
CRegionMatrix::~CRegionMatrix()
{
	delete[] m_pCells;
}

// hover.exe:0x4086A0
// FUNCTION: HOVER 0x4086a0
REGIONNODE* CRegionMatrix::AddNode(int nX, int nY, CGameObj* pObj)
{
	REGIONNODE* pNode;
	REGIONNODE* pHead;
	REGIONNODE* pCur;
	REGIONNODE** ppCell;
	int nLeft;
	int nCount;

	// The head of the free list is loaded once here and returned below.
	pNode = g_pRegionNodeFree;
	if (pNode == NULL)
	{
		// Allocate a fresh chunk of REGIONNODEs and link them into the
		// g_pRegionNodeFree free list. The chunk pointer is also registered
		// in g_listRegionNodeBlocks so Hover_FreeRegionNodePool can free it
		// at shutdown.
		pNode = new REGIONNODE[g_dwRegionNodeChunk];
		g_pRegionNodeFree = pNode;
		g_listRegionNodeBlocks.AddTail(pNode);

		nCount = (int)g_dwRegionNodeChunk;
		pNode = g_pRegionNodeFree;
		nLeft = nCount - 1;
		if (nLeft > 0)
		{
			pCur = pNode;
			do
			{
				pCur->m_pNext = pCur + 1;
				pCur++;
				nLeft--;
			} while (nLeft != 0);
		}
		pNode[nCount - 1].m_pNext = NULL;
	}

	g_pRegionNodeFree = pNode->m_pNext;

	pNode->m_pNext = NULL;
	pNode->m_pPrev = NULL;
	pNode->m_pObj = NULL;
	pNode->m_pNextObj = NULL;

	ppCell = &m_pCells[m_dwCols * nY + nX];
	pNode->m_pPrev = (REGIONNODE*)ppCell;
	pHead = *ppCell;
	pNode->m_pNext = pHead;
	if (pHead != NULL)
		pHead->m_pPrev = pNode;
	*ppCell = pNode;

	pNode->m_pObj = pObj;
	pNode->m_pNextObj = pObj->m_pRegionNodes;
	pObj->m_pRegionNodes = pNode;

	return pNode;
}

/////////////////////////////////////////////////////////////////////////////
// CCollider

// hover.exe:0x41B560
// FUNCTION: HOVER 0x41b560
CCollider::CCollider(CSize sizeExtent, DWORD dwCell)
	: m_listMovers(10)
{
	int nCols;
	int nRows;

	// Signed divisions in the original, and the cell count and the cell
	// array are read back out of the matrix for everything that follows.
	nCols = (int)sizeExtent.cx / (int)dwCell;
	nRows = (int)sizeExtent.cy / (int)dwCell;
	m_dwCols = (DWORD)nCols;
	m_dwRows = (DWORD)nRows;
	m_dwCellSize = dwCell;
	m_grid.m_dwCells = (DWORD)(nCols * nRows);
	m_grid.m_pCells = new REGIONNODE*[m_grid.m_dwCells];
	m_grid.m_dwCols = (DWORD)nCols;
	m_grid.m_dwRows = (DWORD)nRows;
	m_grid.m_pCellsEnd = m_grid.m_pCells + m_grid.m_dwCells;
	memset(m_grid.m_pCells, 0, m_grid.m_dwCells * sizeof(REGIONNODE*));
	m_bQuerying = 0;
}

CCollider::~CCollider()
{
	// member destruction only in the original as well
}

// hover.exe:0x40F900
// FUNCTION: HOVER 0x40f900
int CCollider::AddStatic(CGameObj* pObj)
{
	// Virtual call at vtable slot 0x30 in the original
	// (hover.exe:0x408820 on CPlayer), passing this object.
	pObj->UpdateRegions((DWORD)this);
	return 1;
}

// hover.exe:0x40F8A0
// FUNCTION: HOVER 0x40f8a0
void CCollider::CollectCell(int nX, int nY)
{
	REGIONNODE* pNode;
	int nSize;
	int i;
	int bFound;

	pNode = m_grid.m_pCells[m_grid.m_dwCols * nY + nX];
	while (pNode != NULL)
	{
		// bFound is zeroed first so its register serves as the known
		// zero the loop guard compares the size against (rule 6).
		bFound = 0;
		i = 0;
		nSize = m_arrQueryHits.GetSize();
		for (; i < nSize; i++)
		{
			if ((CGameObj*)m_arrQueryHits.GetAt(i) == pNode->m_pObj)
			{
				bFound = 1;
				break;
			}
		}
		if (!bFound)
			m_arrQueryHits.SetAtGrow(nSize, pNode->m_pObj);
		pNode = pNode->m_pNext;
	}
}

// hover.exe:0x40F9B0
// FUNCTION: HOVER 0x40f9b0
int CCollider::IsSpotBlocked(POINT pt, long lR)
{
	REGIONNODE* pNode;
	long lH;
	long lW;
	long lCol;
	long lRow;
	int bBlocked;

	lW = (long)(m_dwCols * m_dwCellSize);
	lH = (long)(m_dwRows * m_dwCellSize);
	lCol = pt.x / (long)m_dwCellSize;
	lRow = pt.y / (long)m_dwCellSize;
	// One guard, one epilogue: the original branches all four range
	// tests to the same `return 1` (rule 38).
	if (pt.x - lR <= 0 || pt.x + lR >= lW || pt.y - lR <= 0 || pt.y + lR >= lH)
		return 1;
	pNode = m_grid.m_pCells[
		(long)m_grid.m_dwCols * lRow + lCol];
	if (pNode == NULL)
		return 0;
	bBlocked = 0;
	do
	{
		// CFloorObj instances do not block the spot
		if (!pNode->m_pObj->IsKindOf(RUNTIME_CLASS(CFloorObj)) &&
			pNode->m_pObj->IsNear(pt, lR))
		{
			bBlocked = 1;
			break;
		}
		pNode = pNode->m_pNext;
	} while (pNode != NULL);
	return bBlocked;
}


/////////////////////////////////////////////////////////////////////////////
// The collider's per-frame pass, run once per game step by
// CBumperView::RunCollider.

// For every registered object, collect the objects that share one of its
// region-matrix cells (skipping itself, anything already visited earlier
// in the registration list, and duplicates), then ask each of them
// whether it wants the collision (vtable slot 11, Touches) and hand
// the pair to the owner's collision handler (vtable slot 8, OnTouch).
// The candidate list is a CPtrList local, emptied after each object.

// hover.exe:0x40A650
// FUNCTION: HOVER 0x40a650
void CCollider::DetectCollisions()
{
	CObList listHits(10);
	POSITION pos;
	POSITION posOther;
	REGIONNODE* pChain;
	REGIONNODE* pCell;
	CGameObj* pObj;
	CGameObj* pOther;
	int bSeen;

	pos = m_listMovers.GetHeadPosition();
	while (pos != NULL)
	{
		pObj = (CGameObj*)m_listMovers.GetNext(pos);
		pChain = pObj->m_pRegionNodes;
		while (pChain != NULL)
		{
			// Walk back to the head of the cell's node list: the head is
			// the node whose m_pPrev points into the cell array itself.
			pCell = pChain;
			while (pCell->m_pPrev < (REGIONNODE*)m_grid.m_pCells ||
				(REGIONNODE*)m_grid.m_pCellsEnd < pCell->m_pPrev)
			{
				pCell = pCell->m_pPrev;
			}
			while (pCell != NULL)
			{
				if (pObj != pCell->m_pObj)
				{
					// Objects earlier in the registration list have
					// already had this pair handed to them.
					bSeen = 0;
					posOther = m_listMovers.GetHeadPosition();
					while (posOther != pos)
					{
						// The volatile read keeps VC from hoisting
						// pCell->m_pObj into a register across the whole
						// pair test, which is what costs pObj its own
						// register in the original (rule 26's lever).
						if (m_listMovers.GetNext(posOther) ==
							((volatile REGIONNODE*)pCell)->m_pObj)
						{
							bSeen = 1;
							break;
						}
					}
					if (!bSeen)
					{
						if (listHits.Find(pCell->m_pObj) == NULL)
							listHits.AddTail(pCell->m_pObj);
					}
				}
				pCell = pCell->m_pNext;
			}
			pChain = pChain->m_pNextObj;
		}
		posOther = listHits.GetHeadPosition();
		while (posOther != NULL)
		{
			pOther = (CGameObj*)listHits.GetNext(posOther);
			if (pOther->Touches((DWORD)pObj))
				pObj->OnTouch((CPlayer*)pOther, 1);
		}
		listHits.RemoveAll();
	}
}

/////////////////////////////////////////////////////////////////////////////
// CCollider (forward-declared in gameobj.h)

// hover.exe:0x41B770
// FUNCTION: HOVER 0x41b770
int CCollider::AddMover(CGameObj* pObj)
{
	CCollider* pThis;

	pThis = (CCollider*)this;
	pThis->m_listMovers.AddHead(pObj);
	// Virtual call at vtable slot 0x30 in the original
	// (hover.exe:0x408820 on CPlayer), passing this object.
	pObj->UpdateRegions((DWORD)this);
	return 1;
}

/////////////////////////////////////////////////////////////////////////////
// CPlayer mirror method

// hover.exe:0x41B0E0
// FUNCTION: HOVER 0x41b0e0
void CPlayer::PlacePlayer(POINT pt, DWORD dwC, DWORD dwD)
{
	// The zero handed to UpdateProbe is a 16.16 local the original builds
	// half by half out of the same zero register as the member stores.
	DWORD dwZero;
	long lX;
	long lY;

	lX = pt.x;
	lY = pt.y;
	SetCellPos(lX, lY);
	m_wTargetHeight = 0x40;			// +0xA2
	m_wHeading = (WORD)dwC;		// +0x19E
	m_wHeadingFrac = 0;				// +0x19C
	m_wTargetHeightFrac = 0;				// +0xA0
	((WORD*)&dwZero)[0] = 0;
	((WORD*)&dwZero)[1] = 0;
	m_fxHeight = m_fxTargetHeight;	// +0xA4
	m_dwAltitude = dwD;
	m_dwSpawnHeight2 = dwD;	// +0xB0
	m_dwSpawnHeight1 = dwD;	// +0xAC
	UpdateProbe(dwZero);
	QueueBeginFrame();
}

// hover.exe:0x41B2D0
// FUNCTION: HOVER 0x41b2d0
void CPlayer::Spawn(POINT pt, DWORD dwC, DWORD dwD)
{
	ResetState();
	ResetMotion();
	ResetMotion();
	m_bInvincible = 0;
	m_bGodMode = 0;
	m_pInvinciblePod = 0;
	m_pSpeedPod = 0;
	m_pSlowPod = 0;
	m_pTempWallPod = 0;
	m_pSmokeBombPod = 0;
	m_pJumpPod = 0;
	PlacePlayer(pt, dwC, 0x40);
}

/////////////////////////////////////////////////////////////////////////////
// CRenderContext

// hover.exe:0x406C00
// FUNCTION: HOVER 0x406c00
void CRenderContext::SetCamera(DWORD dwA, DWORD dwB, DWORD dwC, DWORD dwD)
{
	// The camera position pair is written through a base pointer of its
	// own (rule 25).
	DWORD dwAngle = dwD & 0x1FF;
	DWORD* pdwPos = &m_dwCamX;
	int bChanged;

	m_dwCamZ = dwC;
	m_dwCamAngle = dwAngle;
	// The compare happens before the camera pair is stored, so the flag
	// is a separate value in the source.
	bChanged = (m_dwLastCamZ != dwC);
	pdwPos[0] = dwA;
	pdwPos[1] = dwB;
	if (bChanged)
		m_dwLastCamZ = dwC;
}

// FUNCTION: CRenderContext::BuildMipShiftTable
// hover.exe:0x410EC0
// FUNCTION: HOVER 0x410ec0
void CRenderContext::BuildMipShiftTable()
{
	int i = 0;
	int nAccum = 0;
	int nStep = m_dwPixelH << 6;
	int nDivisor = m_dwRows * 5;

	for (; i < 0x8000; i += 16)
	{
		int nValue = nAccum / nDivisor;
		int nCount = 0;
		while (nValue != 0)
		{
			nValue >>= 1;
			nCount++;
		}
		m_pMipShift[i >> 4] = (BYTE)nCount;
		nAccum += nStep;
	}
}

// Copies the caller-supplied top/bottom span arrays into the render context.
// hover.exe:0x414A10
// FUNCTION: HOVER 0x414a10
void CRenderContext::SetClipSpans(void* pA, void* pB)
{
	int nCount = (int)m_dwColumns;

	if (nCount > 0)
	{
		DWORD* pTop = m_pClipTop;
		DWORD* pBottom = m_pClipBottom;

		memcpy(pBottom, pB, nCount * sizeof(DWORD));
		memcpy(pTop, pA, nCount * sizeof(DWORD));
	}
}

// FUNCTION: CRenderContext::SetSkyTexture
// hover.exe:0x419940
// FUNCTION: HOVER 0x419940
void CRenderContext::SetSkyTexture(DWORD dwMode, CMerlinTexture* pTexture)
{
	m_dwSkyMode = dwMode;
	m_pSkyTex = pTexture;
}

// hover.exe:0x419960
// FUNCTION: HOVER 0x419960
void CRenderContext::BuildSkyGradient(int nBase, BYTE bRed, BYTE bGreen, BYTE bBlue,
	CPalette* pPalette)
{
	int i;
	BYTE bR;
	BYTE bG;
	BYTE bB;

	m_dwSkyMode = nBase;

	// The top half of the column fades the colour out linearly with the
	// distance from the bottom of the target; note the bound is the
	// height halved on the spot while the divisor is the stored half.
	for (i = 0; i < (int)m_dwRows / 2; i++)
	{
		int nDiv = (int)m_dwHorizon;
		int nDist = (int)m_dwRows - i;
		bR = (BYTE)((nDist * bRed) / nDiv);
		bG = (BYTE)((nDist * bGreen) / nDiv);
		bB = (BYTE)((nDist * bBlue) / nDiv);
		m_pSkyGradient[i] = (DWORD)::GetNearestPaletteIndex(
			(HPALETTE)pPalette->m_hObject,
			(COLORREF)((DWORD)bR + ((DWORD)bG << 8) + ((DWORD)bB << 16)));
		// The palette index is replicated into all four bytes in place.
		m_pSkyGradient[i] |= m_pSkyGradient[i] << 8;
		m_pSkyGradient[i] |= m_pSkyGradient[i] << 16;
	}

	// The bottom half runs the same ramp back up from an accumulator.
	int nAccB = bBlue * ((int)m_dwRows / 2);
	int nAccG = bGreen * ((int)m_dwRows / 2);
	int nAccR = bRed * ((int)m_dwRows / 2);
	for (i = (int)m_dwRows / 2; i < (int)m_dwRows; i++)
	{
		bR = (BYTE)(nAccR / (int)m_dwHorizon);
		bG = (BYTE)(nAccG / (int)m_dwHorizon);
		bB = (BYTE)(nAccB / (int)m_dwHorizon);
		m_pSkyGradient[i] = (DWORD)::GetNearestPaletteIndex(
			(HPALETTE)pPalette->m_hObject,
			(COLORREF)((DWORD)bR + ((DWORD)bG << 8) + ((DWORD)bB << 16)));
		m_pSkyGradient[i] |= m_pSkyGradient[i] << 8;
		m_pSkyGradient[i] |= m_pSkyGradient[i] << 16;
		nAccG += bGreen;
		nAccR += bRed;
		nAccB += bBlue;
	}
}

// hover.exe:0x419B20
SINCOSENTRY::SINCOSENTRY()
{
}

// The original's two span-limit fills below are `rep stosd`, which only
// `/Oa` ("assume no aliasing") produces from a store loop with this
// compiler -- but turning it on for this function alone
// (`#pragma optimize("a", on)`) costs more in extra spills than the two
// loops are worth (58% -> 41%), so the scalar loops stand. See
// notes/perfect-match.md rule 111.

// hover.exe:0x419530
// FUNCTION: HOVER 0x419530
void CRenderContext::SetViewport(CSpanRasterizer* pTarget, CPalette* pPal, int nA,
	int nB, int nWidth, int nHeight)
{
	int i;

	nWidth &= ~3;
	nHeight &= ~3;

	// The original reads the target width through a pointer to the
	// width/height pair and the height by name, and the reset path below
	// takes both dimensions back out through that pointer.
	int* pnDim = (int*)&pTarget->m_dwWidth;
	int nTargetH = (int)pTarget->m_dwHeight;
	int nTargetW = pnDim[0];
	if (nA + nWidth > nTargetW)
		nWidth = (nTargetW - nA) & ~3;

	if (nB + nHeight > nTargetH)
		nHeight = (nTargetH - nB) & ~3;

	// Note the "both zero" test: a window that is off the target in one
	// axis only keeps the other axis' clipped size.
	if (nTargetW < nA || nTargetH < nB || (nWidth == 0 && nHeight == 0))
	{
		nA = 0;
		nB = 0;
		// Both dimensions come back out of the target.  (A two-dword struct
		// copy onto &nWidth used to stand here: VC 2.x keeps nHeight cached
		// in a register across that aliased store, so the reset height was
		// lost - found by differential emulation.)
		nWidth = pnDim[0];
		nHeight = pnDim[1];
	}

	::operator delete(m_pClipTop);
	::operator delete(m_pClipBottom);
	::operator delete(m_pColumnScale);
	::operator delete(m_pRayDirs);
	::operator delete(m_pSkyGradient);
	::operator delete(m_pRowDistance);

	m_dwColumns = (DWORD)nWidth;
	m_dwRows = (DWORD)nHeight;
	m_dwViewW = (DWORD)nWidth;
	m_dwViewH = (DWORD)nHeight;
	m_dwViewX = (DWORD)nA;
	m_dwViewY = (DWORD)nB;
	m_dwLastCamZ = (DWORD)-5000;

	int nTargetPitch = pnDim[0];
	m_dwTargetPitch = (DWORD)nTargetPitch;
	m_dwRowStride = (DWORD)(nTargetPitch * (int)pTarget->m_dwRowDir);
	m_dwTargetHeight = pTarget->m_dwHeight;
	m_dwHorizon = (DWORD)(nHeight / 2);

	m_pRowDistance = (DWORD*)::operator new(ROWDIST_ENTRIES * sizeof(DWORD));
	int nCurrent = -((int)m_dwHorizon << 11);
	int nStep = (int)m_dwHorizon * 2;
	double dFactor = 1.0 / cos(30.0 * 3.14159265359 / 180.0);
	DWORD* pOut = m_pRowDistance;
	int nCount = ROWDIST_ENTRIES;
	do
	{
		int nVal = (int)((double)nCurrent * dFactor);
		*pOut++ = (DWORD)abs(nVal);
		nCurrent += nStep;
	} while (--nCount != 0);

	// The scan-line origin: the target's own origin plus the window, with
	// the rows counted from the bottom when the target is flipped.
	if (pTarget->m_dwRowDir == (DWORD)-1)
		m_pScanOrigin = (BYTE*)(((int)m_dwTargetHeight - (int)m_dwViewY - 1)
			* (int)m_dwTargetPitch + (int)pTarget->m_pBits + (int)m_dwViewX);
	else
		m_pScanOrigin = (BYTE*)((int)m_dwTargetPitch * (int)m_dwViewY
			+ (int)pTarget->m_pBits + (int)m_dwViewX);

	m_pClipTop = (DWORD*)::operator new(m_dwColumns * sizeof(DWORD));
	m_pClipBottom = (DWORD*)::operator new(m_dwColumns * sizeof(DWORD));
	if ((int)m_dwColumns > 0)
	{
		DWORD* pTop = m_pClipTop;
		DWORD dwBottom = m_dwRows - 1;
		DWORD* pBottom = m_pClipBottom;
		for (i = 0; i < (int)m_dwColumns; i++)
			pBottom[i] = dwBottom;
		for (i = 0; i < (int)m_dwColumns; i++)
			pTop[i] = 0;
	}

	m_pColumnScale = (DWORD*)::operator new(m_dwColumns * sizeof(DWORD));
	int nCols = (int)m_dwColumns;
	double dHalf = (double)(int)m_dwColumns * -0.5;
	double dStep = 30.0 / ((double)(int)m_dwColumns * 0.5);
	DWORD* pCol = m_pColumnScale;
	double dHeight = (double)(int)m_dwRows;
	for (i = 0; i < nCols; i++)
	{
		double d = (2.0 - cos((((double)i + dHalf) * dStep)
			* (3.14159265359 / 180.0))) * dHeight;
		pCol[i] = (DWORD)(int)d;
	}

	// The entry table is allocated whatever the count works out to; only
	// the fill below is guarded.
	m_pRayDirs = new SINCOSENTRY[(int)((double)(int)m_dwColumns * 7.0)];

	double dWidth = (double)(int)m_dwColumns;
	double dCosCount = dWidth * 7.0;
	int j = 0;
	if (dCosCount > 0.0)
	{
		double dDenom = dWidth * 6.0;
		double dOffset = 3.14159265359 / 6.0;
		SINCOSENTRY* pEntry = m_pRayDirs;
		do
		{
			double dAngle = (((double)j * (2.0 * 3.14159265359)) / dDenom)
				- dOffset;
			pEntry->nCos = (int)(cos(dAngle) * 32767.0);
			pEntry->nSin = (int)(sin(dAngle) * 32767.0);
			pEntry++;
			j++;
		} while ((double)j < dCosCount);
	}

	m_pSkyGradient = (DWORD*)::operator new(m_dwRows * sizeof(DWORD));
	BuildSkyGradient(1, 0xFF, 0xFF, 0xFF, pPal);
}

// hover.exe:0x410F20
// FUNCTION: HOVER 0x410f20
void CRenderContext::SetMode(DWORD dwFlags, DWORD dwMode)
{
	// Declaration order picks the registers here: with dwB1 first VC puts
	// the two flags in edx/esi the wrong way round (rule 8).
	DWORD dwB2;
	DWORD dwB1;
	SPANFN* pTable;

	dwB1 = dwFlags & 1;
	dwB2 = (dwFlags & 2) >> 1;
	m_bLit = dwB1;
	m_bUseShiftTable = dwB2;
	m_dwMode = dwMode;
	switch (dwMode)
	{
	case 1:
		m_pfnDrawWall = Column1x1;
		m_pfnDrawSky = Sky1x1;
		m_pfnDrawFlat = Span1x1_Var;
		if (dwB2 != 0)
		{
			m_pfnDrawWall = Column1x1Reg;
			pTable = m_ppfnDrawFlatByShift;
			pTable[6] = Span1x1_S6;
			pTable[7] = Span1x1_S7;
			pTable[8] = Span1x1_S8;
			pTable[9] = Span1x1_S9;
			pTable[10] = Span1x1_S10;
			pTable[11] = Span1x1_S11;
			pTable[12] = Span1x1_S12;
			pTable[13] = Span1x1_S13;
			pTable[14] = Span1x1_S14;
			pTable[15] = Span1x1_S15;
		}
		if (dwB1 != 0)
			m_pfnDrawWall = Column1x1Lit;
		if (dwB1 != 0)
		{
			m_pfnDrawFlat = Span1x1Lit_Var;
			if (dwB2 != 0)
			{
				m_pfnDrawWall = Column1x1LitReg;
				pTable = m_ppfnDrawFlatByShift;
				pTable[6] = Span1x1Lit_S6;
				pTable[7] = Span1x1Lit_S7;
				pTable[8] = Span1x1Lit_S8;
				pTable[9] = Span1x1Lit_S9;
				pTable[10] = Span1x1Lit_S10;
				pTable[11] = Span1x1Lit_S11;
				pTable[12] = Span1x1Lit_S12;
				pTable[13] = Span1x1Lit_S13;
				pTable[14] = Span1x1Lit_S14;
				pTable[15] = Span1x1Lit_S15;
			}
		}
		m_dwPixelH = 1;
		m_dwPixelW = 1;
		break;
	case 2:
		m_pfnDrawWall = Column2x1;
		m_pfnDrawSky = Sky2x1;
		m_pfnDrawFlat = Span2x1_Var;
		if (dwB2 != 0)
		{
			m_pfnDrawWall = Column2x1Reg;
			pTable = m_ppfnDrawFlatByShift;
			pTable[6] = Span2x1_S6;
			pTable[7] = Span2x1_S7;
			pTable[8] = Span2x1_S8;
			pTable[9] = Span2x1_S9;
			pTable[10] = Span2x1_S10;
			pTable[11] = Span2x1_S11;
			pTable[12] = Span2x1_S12;
			pTable[13] = Span2x1_S13;
			pTable[14] = Span2x1_S14;
			pTable[15] = Span2x1_S15;
			m_pfnDrawSky = Sky2x1Reg;
		}
		if (dwB1 != 0)
			m_pfnDrawWall = Column2x1Lit;
		if (dwB1 != 0)
		{
			m_pfnDrawFlat = Span2x1Lit_Var;
			if (dwB2 != 0)
			{
				m_pfnDrawWall = Column2x1LitReg;
				pTable = m_ppfnDrawFlatByShift;
				pTable[6] = Span2x1Lit_S6;
				pTable[7] = Span2x1Lit_S7;
				pTable[8] = Span2x1Lit_S8;
				pTable[9] = Span2x1Lit_S9;
				pTable[10] = Span2x1Lit_S10;
				pTable[11] = Span2x1Lit_S11;
				pTable[12] = Span2x1Lit_S12;
				pTable[13] = Span2x1Lit_S13;
				pTable[14] = Span2x1Lit_S14;
				pTable[15] = Span2x1Lit_S15;
			}
		}
		m_dwPixelH = 1;
		m_dwPixelW = 2;
		break;
	case 3:
		m_pfnDrawWall = Column4x1;
		m_pfnDrawSky = Sky4x1;
		m_pfnDrawFlat = Span4x1_Var;
		if (dwB2 != 0)
		{
			m_pfnDrawWall = Column4x1Reg;
			pTable = m_ppfnDrawFlatByShift;
			pTable[6] = Span4x1_S6;
			pTable[7] = Span4x1_S7;
			pTable[8] = Span4x1_S8;
			pTable[9] = Span4x1_S9;
			pTable[10] = Span4x1_S10;
			pTable[11] = Span4x1_S11;
			pTable[12] = Span4x1_S12;
			pTable[13] = Span4x1_S13;
			pTable[14] = Span4x1_S14;
			pTable[15] = Span4x1_S15;
			m_pfnDrawSky = Sky4x1Reg;
		}
		if (dwB1 != 0)
			m_pfnDrawWall = Column4x1Lit;
		if (dwB1 != 0)
		{
			m_pfnDrawFlat = Span4x1Lit_Var;
			if (dwB2 != 0)
			{
				m_pfnDrawWall = Column4x1LitReg;
				pTable = m_ppfnDrawFlatByShift;
				pTable[6] = Span4x1Lit_S6;
				pTable[7] = Span4x1Lit_S7;
				pTable[8] = Span4x1Lit_S8;
				pTable[9] = Span4x1Lit_S9;
				pTable[10] = Span4x1Lit_S10;
				pTable[11] = Span4x1Lit_S11;
				pTable[12] = Span4x1Lit_S12;
				pTable[13] = Span4x1Lit_S13;
				pTable[14] = Span4x1Lit_S14;
				pTable[15] = Span4x1Lit_S15;
			}
		}
		m_dwPixelH = 1;
		m_dwPixelW = 4;
		break;
	case 4:
		m_pfnDrawWall = Column2x2;
		m_pfnDrawSky = Sky2x2;
		m_pfnDrawFlat = Span2x2_Var;
		if (dwB2 != 0)
		{
			m_pfnDrawWall = Column2x2Reg;
			m_pfnDrawSky = Sky2x2Reg;
			pTable = m_ppfnDrawFlatByShift;
			pTable[6] = Span2x2_S6;
			pTable[7] = Span2x2_S7;
			pTable[8] = Span2x2_S8;
			pTable[9] = Span2x2_S9;
			pTable[10] = Span2x2_S10;
			pTable[11] = Span2x2_S11;
			pTable[12] = Span2x2_S12;
			pTable[13] = Span2x2_S13;
			pTable[14] = Span2x2_S14;
			pTable[15] = Span2x2_S15;
		}
		if (dwB1 != 0)
			m_pfnDrawWall = Column2x2Lit;
		if (dwB1 != 0)
		{
			m_pfnDrawFlat = Span2x2Lit_Var;
			if (dwB2 != 0)
			{
				m_pfnDrawWall = Column2x2LitReg;
				pTable = m_ppfnDrawFlatByShift;
				pTable[6] = Span2x2Lit_S6;
				pTable[7] = Span2x2Lit_S7;
				pTable[8] = Span2x2Lit_S8;
				pTable[9] = Span2x2Lit_S9;
				pTable[10] = Span2x2Lit_S10;
				pTable[11] = Span2x2Lit_S11;
				pTable[12] = Span2x2Lit_S12;
				pTable[13] = Span2x2Lit_S13;
				pTable[14] = Span2x2Lit_S14;
				pTable[15] = Span2x2Lit_S15;
			}
		}
		m_dwPixelH = 2;
		m_dwPixelW = 2;
		break;
	case 5:
		m_pfnDrawWall = Column4x2;
		m_pfnDrawSky = Sky4x2;
		m_pfnDrawFlat = Span4x2_Var;
		if (dwB2 != 0)
		{
			m_pfnDrawWall = Column4x2Reg;
			m_pfnDrawSky = Sky4x2Reg;
			pTable = m_ppfnDrawFlatByShift;
			pTable[6] = Span4x2_S6;
			pTable[7] = Span4x2_S7;
			pTable[8] = Span4x2_S8;
			pTable[9] = Span4x2_S9;
			pTable[10] = Span4x2_S10;
			pTable[11] = Span4x2_S11;
			pTable[12] = Span4x2_S12;
			pTable[13] = Span4x2_S13;
			pTable[14] = Span4x2_S14;
			pTable[15] = Span4x2_S15;
		}
		if (dwB1 != 0)
			m_pfnDrawWall = Column4x2Lit;
		if (dwB1 != 0)
		{
			m_pfnDrawFlat = Span4x2Lit_Var;
			if (dwB2 != 0)
			{
				m_pfnDrawWall = Column4x2LitReg;
				pTable = m_ppfnDrawFlatByShift;
				pTable[6] = Span4x2Lit_S6;
				pTable[7] = Span4x2Lit_S7;
				pTable[8] = Span4x2Lit_S8;
				pTable[9] = Span4x2Lit_S9;
				pTable[10] = Span4x2Lit_S10;
				pTable[11] = Span4x2Lit_S11;
				pTable[12] = Span4x2Lit_S12;
				pTable[13] = Span4x2Lit_S13;
				pTable[14] = Span4x2Lit_S14;
				pTable[15] = Span4x2Lit_S15;
			}
		}
		m_dwPixelH = 2;
		m_dwPixelW = 4;
		break;
	case 6:
		m_pfnDrawWall = Column4x4;
		m_pfnDrawSky = Sky4x4;
		m_pfnDrawFlat = Span4x4_Var;
		if (dwB2 != 0)
		{
			m_pfnDrawWall = Column4x4Reg;
			m_pfnDrawSky = Sky4x4Reg;
			pTable = m_ppfnDrawFlatByShift;
			pTable[6] = Span4x4_S6;
			pTable[7] = Span4x4_S7;
			pTable[8] = Span4x4_S8;
			pTable[9] = Span4x4_S9;
			pTable[10] = Span4x4_S10;
			pTable[11] = Span4x4_S11;
			pTable[12] = Span4x4_S12;
			pTable[13] = Span4x4_S13;
			pTable[14] = Span4x4_S14;
			pTable[15] = Span4x4_S15;
		}
		if (dwB1 != 0)
			m_pfnDrawWall = Column4x4Lit;
		if (dwB1 != 0)
		{
			m_pfnDrawFlat = Span4x4Lit_Var;
			if (dwB2 != 0)
			{
				m_pfnDrawWall = Column4x4LitReg;
				pTable = m_ppfnDrawFlatByShift;
				pTable[6] = Span4x4Lit_S6;
				pTable[7] = Span4x4Lit_S7;
				pTable[8] = Span4x4Lit_S8;
				pTable[9] = Span4x4Lit_S9;
				pTable[10] = Span4x4Lit_S10;
				pTable[11] = Span4x4Lit_S11;
				pTable[12] = Span4x4Lit_S12;
				pTable[13] = Span4x4Lit_S13;
				pTable[14] = Span4x4Lit_S14;
				pTable[15] = Span4x4Lit_S15;
			}
		}
		m_dwPixelH = 4;
		m_dwPixelW = 4;
		break;
	}
}

/////////////////////////////////////////////////////////////////////////////
// Free helper functions

// hover.exe:0x413730
// FUNCTION: HOVER 0x413730
int Hover_NearestPaletteIndex(DWORD dwColor, const BYTE* pPal)
{
	long lBest;
	long lDist;
	int nBest;
	int nG;
	int nB;
	int nR;
	int i;

	lBest = 0x8000001;
	nBest = 0;
	nR = (int)(BYTE)dwColor;
	nG = (int)(BYTE)(dwColor >> 8);
	nB = (int)(BYTE)(dwColor >> 16);
	// The walking pointer is anchored on the blue byte of each entry (two
	// bytes past its start), with green and red reached by negative
	// offsets from it - not on the entry's start with positive offsets.
	pPal += 0x2A;
	for (i = 10; i < 0xF6; i++)
	{
		lDist = (long)g_awSquares[255 + nB - pPal[0]] +
			(long)g_awSquares[255 + nG - pPal[-1]] +
			(long)g_awSquares[255 + nR - pPal[-2]];
		if (lDist < lBest)
		{
			lBest = lDist;
			nBest = i;
		}
		pPal += 4;
	}
	return nBest;
}

// hover.exe:0x40E730
// FUNCTION: HOVER 0x40e730
void Hover_BlitRect(long lA1, long lA2, long lA3, long lA4, long lA5,
	long lA6, long lA7, long lA8, long lA9, long lA10, long lA11, long lA12,
	BYTE* pSrc, BYTE* pDst, int bUnk)
{
	long lR5;
	long lR11;
	long lXMax;
	long lYMax;
	long lW;
	long lRows;
	long lU;
	long lV;
	long lX;
	long lS;
	long lD;
	long i;
	BYTE b;

	if (lA5 % 2 != 0)
		lR5 = lA5 + 1;
	else
		lR5 = lA5;
	if (lA11 % 2 != 0)
		lR11 = lA11 + 1;
	else
		lR11 = lA11;
	lXMax = lA6 * lR5;
	lYMax = lR11 * lA12;
	lW = lA9 - lA7;
	lRows = lA10 - lA8;
	if (lRows > 0)
	{
		lU = lA8 * lR11;
		lV = (lA12 - lA8 - 1) * lR11;
		lX = (lA6 - lA2 - 1) * lR5;
		do
		{
			if (bUnk == 0)
				lD = lU;
			else
				lD = lV;
			lD += lA7;
			lS = lA1 + lX;
			if (lW > 0)
			{
				i = lW;
				do
				{
					if (lS < lXMax && lS >= 0 && lD < lYMax && lD >= 0)
					{
						b = pSrc[lS];
						lS++;
						pDst[lD] = b;
						lD++;
					}
					i--;
				} while (i != 0);
			}
			lV -= lR11;
			lU += lR11;
			lX -= lR5;
			lRows--;
		} while (lRows != 0);
	}
}

/////////////////////////////////////////////////////////////////////////////
// CRobotPlayer AI component objects

// hover.exe:0x4235E0
// FUNCTION: HOVER 0x4235e0
CRobotSeekState::CRobotSeekState(CRobotPlayer* pParent)
	: CRobotState(pParent)
{
	m_bProbing = 0;
}

// hover.exe:0x423710
// FUNCTION: HOVER 0x423710
CRobotSpinState::CRobotSpinState(CRobotPlayer* pParent)
	: CRobotState(pParent)
{
}

// hover.exe:0x4237A0
// FUNCTION: HOVER 0x4237a0
CRobotGotoState::CRobotGotoState(CRobotPlayer* pParent)
	: CRobotState(pParent)
{
}

// hover.exe:0x4238D0
// FUNCTION: HOVER 0x4238d0
CRobotFlagChaseState::CRobotFlagChaseState(CRobotPlayer* pParent)
	: CRobotState(pParent)
{
}

// hover.exe:0x423960
// FUNCTION: HOVER 0x423960
CRobotFlagWaitState::CRobotFlagWaitState(CRobotPlayer* pParent)
	: CRobotState(pParent)
{
}

// hover.exe:0x423A90
// FUNCTION: HOVER 0x423a90
CRobotRouteState::CRobotRouteState(CRobotPlayer* pParent)
	: CRobotState(pParent)
{
}

// hover.exe:0x423B80
// FUNCTION: HOVER 0x423b80
CRobotRamState::CRobotRamState(CRobotPlayer* pParent)
	: CRobotState(pParent)
{
}

/////////////////////////////////////////////////////////////////////////////
// CRobotPlayer

// hover.exe:0x4220A0
// FUNCTION: HOVER 0x4220a0
CRobotPlayer::CRobotPlayer(DWORD dwRobot)
	: m_stSeek(this), m_stSpin(this), m_stGoto(this), m_stFlagChase(this),
		m_stFlagWait(this), m_stRoute(this), m_stRam(this)
{
	m_wTargetHeadingFrac = 0;
	m_wTargetHeadingInt = 0;
	m_pParams = theApp.m_pRobotParams;
	m_bHunter = dwRobot;
	memset(m_apRecent, 0, sizeof(m_apRecent));
	m_dwRecentNext = 0;
	ResetMotion();
	m_pTarget = 0;
	m_pState = &m_stSeek;
	m_bOvershooting = 0;
}


// hover.exe:0x40E710
// FUNCTION: HOVER 0x40e710
int CCollider::RemoveObject(CGameObj* pObj)
{
	pObj->ReleaseRegionNodes();
	return 1;
}

// Pool allocator for the 0x70-byte renderer sprite objects: carves them out
// of g_dwSpritePoolBlock-sized blocks (each block registered in
// g_listMerlinDynamicBlocks and freed at shutdown by
// Hover_FreeMerlinDynamicPool). The free list is threaded through each
// object's +0x6C field, and the nSize argument is ignored, exactly as in
// the original.

// hover.exe:0x42CBC0
// FUNCTION: HOVER 0x42cbc0
void* CMerlinDynamic::operator new(size_t cb)
{
	CMerlinDynamic* pBlock;
	int nCount;
	int i;

	pBlock = g_pMerlinDynamicFree;
	if (pBlock == NULL)
	{
		g_pMerlinDynamicFree = (CMerlinDynamic*)::operator new(
			g_dwSpritePoolBlock * sizeof(CMerlinDynamic));
		g_listMerlinDynamicBlocks.AddTail((void*)g_pMerlinDynamicFree);
		nCount = (int)g_dwSpritePoolBlock;
		pBlock = g_pMerlinDynamicFree;
		for (i = 0; i < nCount - 1; i++)
			pBlock[i].m_pNextFree = &pBlock[i + 1];
		pBlock[nCount - 1].m_pNextFree = NULL;
	}
	g_pMerlinDynamicFree = pBlock->m_pNextFree;
	return pBlock;
}

/////////////////////////////////////////////////////////////////////////////
// CRobotPlayer AI component virtuals
//
// The seven components are the states of the robot's AI state machine.
// CRobotPlayer::m_pState points at the active one; CRobotPlayer::
// Think runs its Think (vtable slot 7) once per frame, and the
// states hand over to each other through CRobotState::ChangeState, which
// calls the old state's OnLeave (slot 19, "leave") and the new one's
// OnEnter (slot 18, "enter"). The tuning values come from the RobotAI
// parameter block theApp.m_pRobotAIParams (g_RobotAI*Params).

// The class has both an out-of-line ??1 (hover.exe:0x423670, called by the
// embedded state objects' destructors in ~CRobotPlayer) and a scalar deleting
// ??_G (hover.exe:0x42F180); rule 69 says to mark the ??1, which is what this
// definition compiles to.
// hover.exe:0x423670
// FUNCTION: HOVER 0x423670
CRobotState::~CRobotState()
{
	// empty in the original (just the CGameObj destructor call)
}

// hover.exe:0x42ECD0
// FUNCTION: HOVER 0x42ecd0
void CRobotState::Think()
{
	// the base state does nothing per frame
}

// Answers the "does this state care about pObj" query the collision code
// asks through vtable slot 14 (pObj is the object the robot ran into): a
// CFlagRemover latches the m_bBlocked flag on.

// hover.exe:0x41BEB0
// FUNCTION: HOVER 0x41beb0
int CRobotState::OnProbeHit(CGameObj* pObj)
{
	if (((CObject*)pObj)->IsKindOf(RUNTIME_CLASS(CFlagRemover)))
		m_bBlocked = 1;
	return (int)m_bBlocked;
}

// hover.exe:0x42ECE0
// FUNCTION: HOVER 0x42ece0
void CRobotState::OnEnter()
{
	// empty in the original
}

// hover.exe:0x42ECF0
// FUNCTION: HOVER 0x42ecf0
void CRobotState::OnLeave()
{
	// empty in the original
}

// hover.exe:0x41BC10
// FUNCTION: HOVER 0x41bc10
void CRobotState::ChangeState(CRobotState* pNext)
{
	m_pRobot->m_pState->OnLeave();
	m_pRobot->m_pState = pNext;
	pNext->OnEnter();
}

// Nonzero when the collider reports a clear path between the human
// player and (dwX, dwY): the robots use it to decide whether a candidate
// beacon can see the player. dwA/dwB bracket the height band tested.

// hover.exe:0x42ED00
// FUNCTION: HOVER 0x42ed00
int CRobotState::CanSeeHuman(DWORD dwX, DWORD dwY, DWORD dwA, DWORD dwB)
{
	CHumanPlayer* pHuman;
	CCollider* pCollider;
	DWORD* pdwPos;
	DWORD adwPos[2];

	pHuman = &theApp.m_pBumperDoc->m_humanPlayer;
	pCollider = theApp.m_pBumperDoc->m_pCollider;
	if ((long)dwB < (long)pHuman->m_dwAltitude ||
		(long)pHuman->m_dwAltitude + 0x40 < (long)dwA)
		return 0;
	if (pHuman->m_pSmokeBombPod != 0 &&
		((CGameObj*)pHuman->m_pSmokeBombPod)->m_bInUse != 0)
		return 0;
	// The segment runs from the caller's point to the human's, not the
	// other way round (the original pushes dwX/dwY as arguments 1 and 2).
	pdwPos = pHuman->GetCellPos(adwPos);
	return pCollider->IsPathClear(dwX, dwY, pdwPos[0], pdwPos[1], dwA, dwB,
		(int)theApp.m_pRobotAIParams->m_adw[5]);
}

// The same "clear path" test as CanSeeHuman, but taken from (dwX, dwY) --
// the robot's own position -- against every flag still lying in the maze:
// the first flag whose height band overlaps [dwA, dwB] and to which the
// collider reports a clear line comes back in *ppTarget. The line is
// then registered as a temporary wall (ProbeSegment), which calls back into
// this state object's OnProbeHit and sets m_bBlocked when something is in
// the way after all; only when that flag stays clear is the flag
// accepted.

// hover.exe:0x40BA10
// FUNCTION: HOVER 0x40ba10
int CRobotState::FindVisibleFlag(DWORD dwX, DWORD dwY, DWORD dwA, DWORD dwB,
	CGameObj** ppTarget)
{
	CCollider* pCollider;
	CPodObj* pFlag;
	POSITION pos;
	DWORD* pdwPos;
	DWORD* pdwAI;
	long lBase;
	DWORD adwPos[2];
	DWORD adwPos2[2];
	DWORD dwLimA;
	DWORD dwLimB;

	*ppTarget = 0;
	dwLimA = dwA;
	dwLimB = dwB;
	pCollider = theApp.m_pBumperDoc->m_pCollider;
	pos = theApp.m_pBumperDoc->m_listMazeObjs.GetHeadPosition();
	while (pos != NULL)
	{
		pFlag = (CPodObj*)theApp.m_pBumperDoc->m_listMazeObjs.GetNext(pos);
		if (!pFlag->IsKindOf(RUNTIME_CLASS(CFlag)))
			continue;
		if (pFlag->m_bRobotFlag != 0)
			continue;
		if (pFlag->m_bOnBoard == 0)
			continue;
		lBase = (long)pFlag->m_dwZOffset + (long)pFlag->m_dwBaseZ;
		if ((long)pFlag->m_dwHeight + lBase < (long)dwLimA)
			continue;
		if ((long)dwLimB < lBase)
			continue;
		pdwAI = theApp.m_pRobotAIParams->m_adw;
		pdwPos = pFlag->GetCellPos(adwPos);
		if (pCollider->IsPathClear(dwX, dwY, pdwPos[0], pdwPos[1], dwLimA,
			dwLimB, (int)pdwAI[6]) == 0)
			continue;
		m_bBlocked = 0;
		pdwAI = theApp.m_pRobotAIParams->m_adw;
		pdwPos = pFlag->GetCellPos(adwPos2);
		pCollider->ProbeSegment(dwX, dwY, pdwPos[0], pdwPos[1],
			(int)pdwAI[6], (CBeacon*)this);
		if (m_bBlocked != 0)
			continue;
		*ppTarget = pFlag;
		return 1;
	}
	return 0;
}

// The flag robots' "can I see the human player" pass: locks onto the
// player object embedded in the document and switches to CRobotRamState.

// hover.exe:0x40A790
// FUNCTION: HOVER 0x40a790
int CRobotState::TryLockOnHuman()
{
	CRobotState* pNext;
	CRobotPlayer* pParent;
	DWORD* pdwPos;
	DWORD adwPos[2];

	// Both failure paths share the one `return 0` the original has.
	pParent = m_pRobot;
	if (pParent->m_bHunter != 0)
	{
		pdwPos = pParent->GetCellPos(adwPos);
		if (CanSeeHuman(pdwPos[0], pdwPos[1], pParent->m_dwAltitude,
			pParent->m_dwAltitude + 0x40) != 0)
		{
			if (m_pRobot->m_pTarget != 0 &&
				m_pRobot->m_pTarget->IsKindOf(
					RUNTIME_CLASS(CBeacon)))
			{
				((CBeacon*)m_pRobot->m_pTarget)->m_bTargeted = 0;
			}
			pParent = m_pRobot;
			pParent->m_pTarget =
				&theApp.m_pBumperDoc->m_humanPlayer;
			pNext = &pParent->m_stRam;
			pParent->m_pState->OnLeave();
			m_pRobot->m_pState = pNext;
			pNext->OnEnter();
			return 1;
		}
	}
	return 0;
}

// The same pass for the non-flag robots, using the line-of-sight test
// taken from the robot's own position; hands over to CRobotFlagChaseState.

// hover.exe:0x40BB40
// FUNCTION: HOVER 0x40bb40
int CRobotState::TryLockOnFlag()
{
	CRobotState* pNext;
	CRobotPlayer* pParent;
	DWORD adwPos[2];
	CGameObj* pFlag;

	// Both failure paths share the one `return 0` the original has.
	pParent = m_pRobot;
	if (pParent->m_bHunter == 0)
	{
		pParent->GetCellPos(adwPos);
		pFlag = 0;
		if (FindVisibleFlag(adwPos[0], adwPos[1], m_pRobot->m_dwAltitude,
			m_pRobot->m_dwAltitude + 0x40, &pFlag) != 0)
		{
			if (m_pRobot->m_pTarget != 0 &&
				m_pRobot->m_pTarget->IsKindOf(
					RUNTIME_CLASS(CBeacon)))
			{
				((CBeacon*)m_pRobot->m_pTarget)->m_bTargeted = 0;
			}
			pParent = m_pRobot;
			pParent->m_pTarget = pFlag;
			pNext = &pParent->m_stFlagChase;
			pParent->m_pState->OnLeave();
			m_pRobot->m_pState = pNext;
			pNext->OnEnter();
			return 1;
		}
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////
// CRobotSeekState - target acquisition (the robot's initial and fallback state)

// Retires the "still moving" flag when the robot has slowed below the
// parameter block's threshold, gives the two dedicated states the first
// chance to take over, then scans the tracked-object array for the
// closest beacon that is free, within the robot's height band and has an
// unobstructed line to it. Registering that segment with the collider
// (ProbeSegment, with this component as the hit callback) is what sets
// m_bBlocked when something is in the way.

// hover.exe:0x414D70
// FUNCTION: HOVER 0x414d70
void CRobotSeekState::Think()
{
	CRobotState* pNext;
	CRobotPlayer* pParent;
	CCollider* pCollider;
	CBeacon* pBeacon;
	DWORD* pAI;
	DWORD adwSelf[2];
	DWORD adwPos[2];
	DWORD dwSpeedX;
	DWORD dwSpeedY;
	FIXED1616 fxSum;
	DWORD dwLow;
	DWORD dwBest;
	DWORD dwDist;
	long lTexY;
	long lHeight;
	int nBeacons;
	int nLeft;
	int i;

	pParent = m_pRobot;
	if (pParent->m_bStopping != 0)
	{
		dwSpeedX = pParent->m_fxVelX;
		HOVER_FIXMULP(dwSpeedX, dwSpeedX);
		dwSpeedY = pParent->m_fxVelY;
		HOVER_FIXMULP(dwSpeedY, dwSpeedY);
		// 16.16 square root: the fractional part comes from the sum but
		// the integer part from the first term only, exactly as in the
		// original, which reads it as a signed short straight out of the
		// sum's stack slot (rule 46).
		fxSum.dw = dwSpeedX;
		dwLow = dwSpeedX + dwSpeedY;
		dwDist = (DWORD)(long)(sqrt((double)(DWORD)LOWORD(dwLow)
			* 1.52587890625e-05 + (long)fxSum.s.sInt) * 65536.0);
		if ((long)dwDist < (long)pParent->m_fxThrustAccel)
			pParent->m_bStopping = 0;
		if (pParent->m_bStopping != 0)
			return;
	}

	pCollider = theApp.m_pBumperDoc->m_pCollider;
	pParent->GetCellPos(adwSelf);
	if (TryLockOnHuman() != 0)
		return;
	if (TryLockOnFlag() != 0)
		return;

	nBeacons = g_arrBeacons.GetSize();

	// Release the beacon the robot was heading for before looking for a
	// new one.
	if (m_pRobot->m_pTarget != 0 &&
		m_pRobot->m_pTarget->IsKindOf(RUNTIME_CLASS(CBeacon)))
	{
		((CBeacon*)m_pRobot->m_pTarget)->m_bTargeted = 0;
	}

	for (nLeft = nBeacons, i = 0; nLeft > 0; i++, nLeft--)
	{
		pBeacon = (CBeacon*)g_arrBeacons.GetAt(i);
		if (pBeacon->m_bTargeted != 0)
			continue;
		lHeight = (long)m_pRobot->m_dwAltitude;
		lTexY = (long)(short)((CMerlinLocation*)pBeacon->m_pLocation)->m_wHeight;
		if (lHeight - 0x4B > lTexY)
			continue;
		if (lHeight + 0x72 < lTexY)
			continue;
		pBeacon->GetCellPos(adwPos);
		dwDist = Hover_ApproxDistance((long)adwSelf[0], (long)adwSelf[1],
			(long)adwPos[0], (long)adwPos[1]);
		// dwBest is not initialized by the original either: it is only
		// consulted once the robot already has a target.
		if (m_pRobot->m_pTarget != 0 && (long)dwBest <= (long)dwDist)
			continue;
		pAI = theApp.m_pRobotAIParams->m_adw;
		if (pCollider->IsPathClear(adwSelf[0], adwSelf[1], adwPos[0],
			adwPos[1], m_pRobot->m_dwAltitude,
			m_pRobot->m_dwAltitude + 0x40, (int)pAI[7]) == 0)
			continue;
		m_bProbing = 1;
		m_bBlocked = 0;
		pAI = theApp.m_pRobotAIParams->m_adw;
		pCollider->ProbeSegment(adwSelf[0], adwSelf[1], adwPos[0], adwPos[1],
			(int)pAI[7], (CBeacon*)this);
		m_bProbing = 0;
		if (m_bBlocked != 0)
			continue;
		dwBest = dwDist;
		m_pRobot->m_pTarget = pBeacon;
		m_pRobot->RememberVisited(pBeacon);
	}

	if (m_pRobot->m_pTarget != 0)
	{
		((CBeacon*)m_pRobot->m_pTarget)->m_bTargeted = 1;
		pNext = &m_pRobot->m_stGoto;
		m_pRobot->m_pState->OnLeave();
		m_pRobot->m_pState = pNext;
		pNext->OnEnter();
	}
}

// Latches m_bBlocked when the object that hit the robot is a CHoldPad (on
// a flag robot) or a CFlagRemover (on a normal robot).

// hover.exe:0x41BF20
// FUNCTION: HOVER 0x41bf20
int CRobotSeekState::OnProbeHit(CGameObj* pObj)
{
	if ((m_pRobot->m_bHunter != 0 &&
			((CObject*)pObj)->IsKindOf(RUNTIME_CLASS(CHoldPad))) ||
		(m_pRobot->m_bHunter == 0 &&
			((CObject*)pObj)->IsKindOf(RUNTIME_CLASS(CFlagRemover))))
	{
		m_bBlocked = 1;
	}
	return (int)m_bBlocked;
}

// hover.exe:0x41BF80
// FUNCTION: HOVER 0x41bf80
void CRobotSeekState::OnEnter()
{
	CRobotPlayer* pParent;

	pParent = m_pRobot;
	if (pParent->m_pTarget != 0 &&
		pParent->m_pTarget->IsKindOf(RUNTIME_CLASS(CBeacon)))
	{
		((CBeacon*)m_pRobot->m_pTarget)->m_bTargeted = 0;
	}
	pParent = m_pRobot;
	pParent->m_pTarget = 0;
	pParent->m_bBrakeAtTarget = 0;
	pParent->m_bSpinning = 0;
	pParent->m_bSpinLeft = 0;
	pParent->m_dwUnk0210 = 0;
	pParent->m_fxTargetHeading = pParent->m_fxHeading;
	pParent->m_bStopping = 1;
	pParent->m_fxMaxSpeed =
		(DWORD)(*(long*)&pParent->m_fxBaseMaxSpeed / 3);
}

// hover.exe:0x415040
// FUNCTION: HOVER 0x415040
void CRobotSeekState::OnLeave()
{
	CRobotPlayer* pParent;

	pParent = m_pRobot;
	pParent->m_bStopping = 0;
	pParent->m_fxMaxSpeed = pParent->m_fxBaseMaxSpeed;
}

/////////////////////////////////////////////////////////////////////////////
// CRobotSpinState - the post-collision spin/recover state

// hover.exe:0x42ED90
// FUNCTION: HOVER 0x42ed90
void CRobotSpinState::Think()
{
	CRobotState* pNext;
	CRobotPlayer* pParent;
	DWORD* pdwStep;
	DWORD dwStep;

	if ((long)m_dwSpinFrames-- <= 0)
	{
		// the state transition is inlined by the original here
		pNext = &m_pRobot->m_stSeek;
		m_pRobot->m_pState->OnLeave();
		m_pRobot->m_pState = pNext;
		pNext->OnEnter();
	}
	// The turn step is the AI parameter block's first field, negated
	// through a stack temporary the original reaches by pointer.
	pParent = m_pRobot;
	pdwStep = theApp.m_pRobotAIParams->m_adw;
	if (pParent->m_bSpinLeft == 0)
	{
		dwStep = (DWORD)(-(long)pdwStep[0]);
		pdwStep = &dwStep;
	}
	pParent->m_fxTargetHeading += *pdwStep;
}

// hover.exe:0x42EE10
// FUNCTION: HOVER 0x42ee10
void CRobotSpinState::OnEnter()
{
	CRobotPlayer* pParent;
	DWORD dwSpeed;
	long lFrom;
	long lTo;

	pParent = m_pRobot;
	if (pParent->m_pTarget != 0 &&
		pParent->m_pTarget->IsKindOf(RUNTIME_CLASS(CBeacon)))
	{
		((CBeacon*)m_pRobot->m_pTarget)->m_bTargeted = 0;
	}
	pParent = m_pRobot;
	// Both turn arguments are read before the two stores, as the
	// original has them.
	lTo = (long)(short)pParent->m_wTargetHeadingInt;
	lFrom = (long)(short)pParent->m_wHeading;
	pParent->m_pTarget = 0;
	pParent->m_bSpinning = 1;
	pParent->m_bSpinLeft = (DWORD)(Hover_AngleInFrontArc(lFrom, lTo) == 0);
	pParent = m_pRobot;
	pParent->m_bBrakeAtTarget = 0;
	pParent->m_dwUnk0210 = 0;
	pParent->m_fxTargetHeading = pParent->m_fxHeading;
	dwSpeed = pParent->m_fxBaseMaxSpeed;
	HOVER_FIXMULP(dwSpeed, theApp.m_pRobotAIParams->m_adw[2]);
	pParent->m_fxMaxSpeed = dwSpeed;
	m_dwSpinFrames = theApp.m_pRobotAIParams->m_adw[1];
}

// hover.exe:0x42EF00
// FUNCTION: HOVER 0x42ef00
void CRobotSpinState::OnLeave()
{
	CRobotPlayer* pParent;

	pParent = m_pRobot;
	pParent->m_fxMaxSpeed = pParent->m_fxBaseMaxSpeed;
	pParent->m_bSpinning = 0;
	pParent->m_bSpinLeft = 0;
}

/////////////////////////////////////////////////////////////////////////////
// CRobotGotoState - chase the locked target

// hover.exe:0x40A860
// FUNCTION: HOVER 0x40a860
void CRobotGotoState::Think()
{
	CRobotState* pNext;
	CRobotPlayer* pParent;
	DWORD adwTarget[2];
	DWORD adwSelf[2];
	DWORD dwSpeedX;
	DWORD dwSpeedY;
	FIXED1616 fxSum;
	DWORD dwLow;
	DWORD dwDist;

	if (TryLockOnHuman() != 0)
		return;
	m_pRobot->GetCellPos(adwSelf);
	(m_pRobot->m_pTarget)->GetCellPos(adwTarget);
	// target first, then self, as hover.exe pushes them
	dwDist = Hover_ApproxDistance((long)adwTarget[0], (long)adwTarget[1],
		(long)adwSelf[0], (long)adwSelf[1]);
	pParent = m_pRobot;
	pParent->m_bStopping = (DWORD)((long)dwDist < 0x32);
	if ((long)dwDist >= 0x32)
		return;

	// Close enough: hand over once the robot has also slowed down.
	dwSpeedX = pParent->m_fxVelX;
	HOVER_FIXMULP(dwSpeedX, dwSpeedX);
	dwSpeedY = pParent->m_fxVelY;
	HOVER_FIXMULP(dwSpeedY, dwSpeedY);
	fxSum.dw = dwSpeedX;
	dwLow = dwSpeedX + dwSpeedY;
	if ((long)(DWORD)(long)(sqrt((double)(DWORD)LOWORD(dwLow) *
		1.52587890625e-05 + (long)fxSum.s.sInt) * 65536.0) >= 0x10000)
		return;
	if (pParent->m_bHunter != 0)
		pNext = &pParent->m_stRoute;
	else
		pNext = &pParent->m_stFlagWait;
	m_pRobot->m_pState->OnLeave();
	m_pRobot->m_pState = pNext;
	pNext->OnEnter();
}

// hover.exe:0x40BC50
// FUNCTION: HOVER 0x40bc50
void CRobotGotoState::OnEnter()
{
	m_dwTimeout = 0xC8;
	m_pRobot->m_bBrakeAtTarget = 1;
}

// hover.exe:0x40BC70
// FUNCTION: HOVER 0x40bc70
void CRobotGotoState::OnLeave()
{
	CRobotPlayer* pParent;

	pParent = m_pRobot;
	pParent->m_bBrakeAtTarget = 0;
	pParent->m_bOvershooting = 0;
	pParent->m_bStopping = 0;
}

/////////////////////////////////////////////////////////////////////////////
// CRobotFlagChaseState - hold the line of sight to the locked target

// hover.exe:0x4150F0
// FUNCTION: HOVER 0x4150f0
void CRobotFlagChaseState::Think()
{
	CRobotState* pNext;
	CRobotPlayer* pParent;
	CCollider* pCollider;
	DWORD* pAI;
	DWORD adwSelf[2];
	DWORD adwTarget[2];

	if ((long)m_dwTimeout-- < 0)
	{
		pNext = &m_pRobot->m_stSeek;
		m_pRobot->m_pState->OnLeave();
		m_pRobot->m_pState = pNext;
		pNext->OnEnter();
		return;
	}
	m_pRobot->GetCellPos(adwSelf);
	(m_pRobot->m_pTarget)->GetCellPos(adwTarget);
	pParent = m_pRobot;
	pAI = theApp.m_pRobotAIParams->m_adw;
	pCollider = theApp.m_pBumperDoc->m_pCollider;
	if (pCollider->IsPathClear(adwSelf[0], adwSelf[1], adwTarget[0],
		adwTarget[1], pParent->m_dwAltitude, pParent->m_dwAltitude + 0x40,
		(int)pAI[6]) == 0)
	{
		pParent = m_pRobot;
		pParent->m_pTarget = 0;
		pNext = &pParent->m_stSeek;
		pParent->m_pState->OnLeave();
		m_pRobot->m_pState = pNext;
		pNext->OnEnter();
	}
}

// hover.exe:0x41C010
// FUNCTION: HOVER 0x41c010
void CRobotFlagChaseState::OnEnter()
{
	m_dwTimeout = 0xC8;
}

/////////////////////////////////////////////////////////////////////////////
// CRobotFlagWaitState - keep chasing the human player, or fall back to the
// search

// hover.exe:0x40BC90
// FUNCTION: HOVER 0x40bc90
void CRobotFlagWaitState::Think()
{
	CRobotState* pNext;
	if (TryLockOnFlag() == 0)
	{
		pNext = &m_pRobot->m_stRoute;
		m_pRobot->m_pState->OnLeave();
		m_pRobot->m_pState = pNext;
		pNext->OnEnter();
	}
}

// hover.exe:0x42EF30
// FUNCTION: HOVER 0x42ef30
void CRobotFlagWaitState::OnEnter()
{
	// empty in the original
}

/////////////////////////////////////////////////////////////////////////////
// CRobotRouteState - beacon-graph search towards the human player

// Pops the next unvisited beacon off the search stack. Each stack frame
// is a {beacon, child index} pair; the walk descends into the beacon's
// linked-beacon array (CBeacon::m_arrLinks) and skips beacons that are
// already on the stack. Returns NULL when the stack runs dry.

// hover.exe:0x40BEF0
// FUNCTION: HOVER 0x40bef0
CBeacon* CBeaconSearch::NextBeacon()
{
	CBeacon* pBeacon;
	BEACONFRAME* pFrame;
	BEACONFRAME* pScan;
	int nIndex;
	int nNext;
	int nDepth;
	int bNew;
	int i;

	do
	{
		pFrame = m_pTop;
		pBeacon = pFrame->pBeacon;
		nIndex = pFrame->nChild;
		if (pBeacon->m_arrLinks.GetSize() > nIndex)
		{
			nNext = nIndex + 1;
			pBeacon = (CBeacon*)pBeacon->m_arrLinks[nIndex];
			pFrame->nChild = nNext;
			// The scan stops as soon as the beacon is already on the
			// walk stack: bNew stays set only while the entries differ.
			bNew = 1;
			i = 0;
			pScan = m_aFrames;
			nDepth = (int)m_dwDepth;
			while (i < nDepth)
			{
				bNew = (pScan->pBeacon != pBeacon);
				pScan++;
				i++;
				if (bNew == 0)
					break;
			}
			if (bNew != 0)
			{
				if ((int)m_dwDepthLimit == nDepth)
					return pBeacon;
				pFrame[1].pBeacon = pBeacon;
				m_dwDepth = (DWORD)(nDepth + 1);
				pFrame++;
				m_pTop = pFrame;
				pFrame->nChild = 0;
			}
		}
		else
		{
			nDepth = (int)m_dwDepth;
			m_dwDepth = (DWORD)(nDepth - 1);
			if (nDepth > 0)
				m_pTop = pFrame - 1;
		}
	} while ((long)m_dwDepth >= 0);
	return NULL;
}

// Walks the beacon graph outwards from the current target until it finds
// a beacon the human player can be seen from, then chases it
// (CRobotGotoState). Gives up after the parameter block's step budget.

// hover.exe:0x40BCD0
// FUNCTION: HOVER 0x40bcd0
void CRobotRouteState::Think()
{
	CRobotState* pNext;
	CRobotPlayer* pParent;
	CBeacon* pBeacon;
	CBeacon* pTarget;
	DWORD* pdwTaken;
	int nStep;

	if ((long)m_search.m_dwIteration == -1)
	{
		GiveUp();
		return;
	}

	pBeacon = m_search.NextBeacon();
	while (pBeacon == NULL)
	{
		nStep = (int)m_search.m_dwIteration + 1;
		m_search.m_dwIteration = (DWORD)nStep;
		if ((long)theApp.m_pRobotAIParams->m_adw[9] <= nStep)
		{
			GiveUp();
			return;
		}
		if (nStep < 5)
		{
			m_search.m_aFrames[0].nChild = 0;
			m_search.m_dwDepth = 0;
			m_search.m_dwDepthLimit = (DWORD)nStep;
			m_search.m_pTop = m_search.m_aFrames;
			m_search.m_pTop->pBeacon = (CBeacon*)m_pRobot->m_pTarget;
		}
	}

	// Note that only the flag test and the hand-over below follow the
	// hop substitution: the original keeps calling GetCellPos on the
	// beacon the walk returned, not on the substituted one (its `this`
	// register is never reloaded before the call).
	pTarget = pBeacon;
	if (m_search.m_dwIteration != 0)
		pTarget = m_search.m_aFrames[1].pBeacon;
	// The original keeps the address of the beacon's "taken" flag in a
	// register across the whole test (rule 25).
	pdwTaken = &pTarget->m_bTargeted;
	if (*pdwTaken != 0)
		return;

	// The original spells the hand-over out in both arms rather than
	// joining them (the two copies are byte-identical in hover.exe).
	pParent = m_pRobot;
	if (pParent->m_bHunter != 0)
	{
		DWORD* pdwPos;
		DWORD adwPos[2];

		// Assigning the position pointer inside the first argument is
		// what puts the GetCellPos call between the height pushes and
		// the coordinate pushes, exactly as the original has it.
		if (CanSeeHuman((pdwPos = pBeacon->GetCellPos(adwPos))[0],
			pdwPos[1], m_pRobot->m_dwAltitude,
			m_pRobot->m_dwAltitude + 0x40) == 0)
			return;
		if (m_pRobot->m_pTarget != 0 &&
			m_pRobot->m_pTarget->IsKindOf(
				RUNTIME_CLASS(CBeacon)))
		{
			((CBeacon*)m_pRobot->m_pTarget)->m_bTargeted = 0;
		}
		*pdwTaken = 1;
		m_pRobot->m_pTarget = pTarget;
		pNext = &m_pRobot->m_stGoto;
		m_pRobot->m_pState->OnLeave();
		m_pRobot->m_pState = pNext;
		pNext->OnEnter();
		m_pRobot->RememberVisited(pTarget);
	}
	else
	{
		DWORD* pdwPos;
		DWORD adwPos[2];
		CGameObj* pFlag;

		// pFlag is a pure out parameter; the original does not
		// initialize it either.
		if (FindVisibleFlag((pdwPos = pBeacon->GetCellPos(adwPos))[0],
			pdwPos[1], m_pRobot->m_dwAltitude,
			m_pRobot->m_dwAltitude + 0x40, &pFlag) == 0)
			return;
		if (m_pRobot->m_pTarget != 0 &&
			m_pRobot->m_pTarget->IsKindOf(
				RUNTIME_CLASS(CBeacon)))
		{
			((CBeacon*)m_pRobot->m_pTarget)->m_bTargeted = 0;
		}
		*pdwTaken = 1;
		m_pRobot->m_pTarget = pTarget;
		pNext = &m_pRobot->m_stGoto;
		m_pRobot->m_pState->OnLeave();
		m_pRobot->m_pState = pNext;
		pNext->OnEnter();
		m_pRobot->RememberVisited(pTarget);
	}
}

// Abandons the search: the next hop of the beacon graph becomes the new
// target, and the state machine goes back to CRobotSeekState (no target left)
// or CRobotGotoState (new target).

// hover.exe:0x40BFF0
// FUNCTION: HOVER 0x40bff0
void CRobotRouteState::GiveUp()
{
	CRobotState* pNext;
	CRobotPlayer* pParent;
	CBeacon* pTarget;

	if (m_pRobot->m_pTarget != 0 &&
		m_pRobot->m_pTarget->IsKindOf(RUNTIME_CLASS(CBeacon)))
	{
		((CBeacon*)m_pRobot->m_pTarget)->m_bTargeted = 0;
	}
	pParent = m_pRobot;
	pParent->m_pTarget =
		((CBeacon*)pParent->m_pTarget)->PickNextHop(pParent);
	pParent = m_pRobot;
	pTarget = (CBeacon*)pParent->m_pTarget;
	if (pTarget != NULL && pTarget->m_bTargeted != 0)
		pParent->m_pTarget = 0;
	pTarget = (CBeacon*)pParent->m_pTarget;
	if (pTarget == NULL)
	{
		pNext = &pParent->m_stSeek;
		pParent->m_pState->OnLeave();
		m_pRobot->m_pState = pNext;
		pNext->OnEnter();
		return;
	}
	pTarget->m_bTargeted = 1;
	pNext = &pParent->m_stGoto;
	pParent->m_pState->OnLeave();
	m_pRobot->m_pState = pNext;
	pNext->OnEnter();
	m_pRobot->RememberVisited(m_pRobot->m_pTarget);
}

// hover.exe:0x40BF80
// FUNCTION: HOVER 0x40bf80
void CRobotRouteState::OnEnter()
{
	CRobotPlayer* pParent;
	BEACONFRAME* pStack;

	pParent = m_pRobot;
	if (pParent->m_pTarget == 0 ||
		!pParent->m_pTarget->IsKindOf(RUNTIME_CLASS(CBeacon)))
	{
		m_search.m_dwIteration = 0xFFFFFFFF;
		return;
	}
	pStack = m_search.m_aFrames;
	m_search.m_dwIteration = 0;
	m_search.m_dwDepth = 0;
	m_search.m_dwDepthLimit = 0;
	m_search.m_pTop = pStack;
	pStack->pBeacon = (CBeacon*)m_pRobot->m_pTarget;
	pStack->nChild = 0;
}

/////////////////////////////////////////////////////////////////////////////
// CRobotRamState - keep the human player in sight while chasing him

// hover.exe:0x42EF40
// FUNCTION: HOVER 0x42ef40
void CRobotRamState::Think()
{
	CRobotState* pNext;
	CRobotPlayer* pParent;
	CPlayer* pTarget;
	CCollider* pCollider;
	DWORD* pAI;
	DWORD adwSelf[2];
	DWORD adwTarget[2];

	m_pRobot->GetCellPos(adwSelf);
	(m_pRobot->m_pTarget)->GetCellPos(adwTarget);
	pParent = m_pRobot;
	pTarget = (CPlayer*)pParent->m_pTarget;
	if (pTarget->m_pSmokeBombPod == 0 ||
		((CGameObj*)pTarget->m_pSmokeBombPod)->m_bInUse == 0)
	{
		pAI = theApp.m_pRobotAIParams->m_adw;
		pCollider = theApp.m_pBumperDoc->m_pCollider;
		if (pCollider->IsPathClear(adwSelf[0], adwSelf[1], adwTarget[0],
			adwTarget[1], pParent->m_dwAltitude,
			pParent->m_dwAltitude + 0x40, (int)pAI[5]) != 0)
			return;
	}
	pParent = m_pRobot;
	pParent->m_pTarget = 0;
	pNext = &pParent->m_stSeek;
	pParent->m_pState->OnLeave();
	m_pRobot->m_pState = pNext;
	pNext->OnEnter();
}

// hover.exe:0x42F000
// FUNCTION: HOVER 0x42f000
void CRobotRamState::OnEnter()
{
	CRobotPlayer* pParent;

	pParent = m_pRobot;
	pParent->m_bBrakeAtTarget = 0;
	pParent->m_bSpinning = 0;
	pParent->m_bSpinLeft = 0;
	pParent->m_dwUnk0210 = (DWORD)((rand() >> 8) & 0xFF);
	if (theApp.m_bGameSounds != 0)
		PlayObjSound(0x1E, 6, 5, 0);
}

/////////////////////////////////////////////////////////////////////////////
// CRobotPlayer virtual overrides

// hover.exe:0x422250
// The destructor is defined in the class declaration, so the compiler folds
// it into the scalar deleting destructor that the vtable slot holds - which
// is what hover.exe:0x422250 is. The block below emits no code; it only gives
// the reccmp marker a declaration to name ??_GCRobotPlayer@@ (rules 34/184).
// hover.exe:0x422250
#if 0
// FUNCTION: HOVER 0x422250
CRobotPlayer::~CRobotPlayer()
{
	// Empty body: the original's deleting destructor is exactly the
	// compiler-generated chain (the seven AI components in reverse
	// declaration order, then ~CPlayer) followed by the inlined
	// CRobotPlayer::operator delete.
}
#endif

// Per-frame think: refresh the "brake released" state, run the active AI
// component (vtable slot 7) and the steering pass, then the CPlayer
// physics update.

// hover.exe:0x409300
// FUNCTION: HOVER 0x409300
void CRobotPlayer::Think()
{
	if (m_bKilled == 0)
	{
		m_dwFlagRemoverState = 0;
		if (m_bOnFlagRemover == 0)
			m_dwFlagRemoverState = 0xFFFFFFFF;
		if (theApp.m_dwGameState == 3)
		{
			m_pState->Think();
			Steer();
		}
	}
	CPlayer::Think();
}

// Collision handler: a CSledPad resets the AI to the acquisition state,
// and any collision that set m_bBounced toggles the robot between the
// acquisition state and the post-collision spin state.

// hover.exe:0x4144D0
// FUNCTION: HOVER 0x4144d0
void CRobotPlayer::OnTouch(CPlayer* pOther, DWORD dwB)
{
	CPlayer::OnTouch(pOther, dwB);
	if (((CObject*)pOther)->IsKindOf(RUNTIME_CLASS(CSledPad)))
		m_pState->ChangeState(&m_stSeek);
	if (m_bBounced != 0)
	{
		if ((DWORD)m_pState - (DWORD)this == 0x2F4)
			m_pState->ChangeState(&m_stSeek);
		else
			m_pState->ChangeState(&m_stSpin);
	}
}

// hover.exe:0x42C3C0
// FUNCTION: HOVER 0x42c3c0
int CRobotPlayer::OnProbeHit(CGameObj* pObj)
{
	return 1;
}

// Picks the sprite frame for the robot: the angle between the robot's
// heading and the direction to the human player selects one of the 32
// "DRONE_nn" textures, and frames past the half circle are drawn
// mirrored (m_bMirror).

// hover.exe:0x40A4C0
// FUNCTION: HOVER 0x40a4c0
void CRobotPlayer::BeginFrame()
{
	CHumanPlayer* pHuman;
	DWORD adwA[2];
	DWORD adwB[2];
	// The heading comes back through the returned pointer.
	long lDX;
	long lDY;
	DWORD dwAngle;
	DWORD* pdwAngle;
	long lFrame;

	pHuman = &theApp.m_pBumperDoc->m_humanPlayer;
	dwAngle = m_fxY;
	lDY = (long)pHuman->GetFixedPos(adwA)[1];
	lDY -= (long)dwAngle;
	dwAngle = m_fxX;
	lDX = (long)pHuman->GetFixedPos(adwB)[0];
	lDX -= (long)dwAngle;
	pdwAngle = Hover_FixAngleP(&dwAngle, lDY, lDX);
	lFrame = ((((long)(short)m_wHeading -
		(long)((short*)pdwAngle)[1]) & 0x1F8) << 6) >> 9;
	if (lFrame >= 0x20)
	{
		m_bMirror = 1;
		lFrame = 0x3F - lFrame;
	}
	else
	{
		m_bMirror = 0;
	}
	m_dwUnk64 = 1;
	m_pHeadingFrame = (CMerlinTexture*)g_apDroneFrames[lFrame];
	CPlayer::BeginFrame();
}

// Builds/updates the renderer sprite from the frame BeginFrame picked:
// the sprite is created lazily out of the 0x70-byte sprite pool and
// registered with the world object, then its screen band, scale, world
// position and mirror flag are refreshed.

// hover.exe:0x40A590
// FUNCTION: HOVER 0x40a590
void CRobotPlayer::EndFrame()
{
	CBumperDoc* pDoc;
	CMerlinDynamic* pSprite;
	CMerlinTexture* pTex;
	WORD wY;

	pDoc = theApp.m_pBumperDoc;
	if (m_bDead == 0)
	{
		if (m_pMerlin == 0 && m_dwUnk64 != 0)
		{
			// The original dereferences the sprite unconditionally
			// here, even when the pool allocation failed.
			m_pMerlin = new CMerlinDynamic;
			pSprite = (CMerlinDynamic*)m_pMerlin;
			pSprite->m_dwUnk60 = 0;
			(pDoc->m_pWorld)->AddSprite(pSprite);
		}
		pSprite = (CMerlinDynamic*)m_pMerlin;
		if (pSprite != NULL)
		{
			pTex = m_pFrame;
			wY = (WORD)(m_dwFrameAltitude + 0xD);
			pSprite->m_pTexture = pTex;
			pSprite->m_wTop = wY;
			wY = (WORD)(wY - pTex->m_wHeight);
			pSprite->m_wBottom = wY;
			if ((short)wY < 0)
				pSprite->m_wBottom = 0;
			pSprite->m_wY = (WORD)m_dwFrameY;
			pSprite->m_bMirror = m_bMirror;
			pSprite->m_wHalfWidth = (WORD)((long)(short)pTex->m_wWidth / 2);
			pSprite->m_wX = (WORD)m_dwFrameX;
		}
	}
	CPlayer::EndFrame();
}

// hover.exe:0x42C3B0
// FUNCTION: HOVER 0x42c3b0
void CRobotPlayer::KillObject()
{
	CPlayer::KillObject();
}

// Picks the next hop of the beacon graph for pParent: of the beacons this
// one is linked to, take a random one that pParent has not visited
// recently (CRobotPlayer::m_apRecent). When every neighbour is on that
// ring buffer the walk is retried ignoring all of it but the most recent
// entry, so the robot at worst turns straight back. Returns NULL when the
// beacon has no links at all.

// hover.exe:0x40C0E0
// FUNCTION: HOVER 0x40c0e0
CGameObj* CBeacon::PickNextHop(CRobotPlayer* pParent)
{
	CGameObj* apCandidates[20];
	CGameObj** ppOut;
	CGameObj* pLink;
	int nLinks;
	int nFound;
	int i;
	int nLast;

	nLinks = m_arrLinks.GetSize();
	if (nLinks == 0)
		return NULL;
	if (nLinks == 1)
		return (CGameObj*)m_arrLinks[0];

	nFound = 0;
	i = 0;
	while (i < nLinks)
	{
		ppOut = apCandidates + nFound;
		if (ppOut >= apCandidates + 20)
			break;
		pLink = (CGameObj*)m_arrLinks[i];
		if (pParent->WasRecentlyVisited(pLink) == 0)
		{
			*ppOut = pLink;
			nFound++;
		}
		i++;
	}
	if (nFound == 0)
	{
		// Second pass: only the beacon the robot came from is excluded.
		i = 0;
		while (i < nLinks)
		{
			ppOut = apCandidates + nFound;
			if (ppOut >= apCandidates + 20)
				break;
			pLink = (CGameObj*)m_arrLinks[i];
			nLast = (int)pParent->m_dwRecentNext;
			if (nLast == 0)
				nLast = 9;
			else
				nLast--;
			if (pParent->m_apRecent[nLast] != pLink)
			{
				*ppOut = pLink;
				nFound++;
			}
			i++;
		}
	}
	return apCandidates[rand() * nFound / 0x8000];
}

// hover.exe:0x40C1F0
// FUNCTION: HOVER 0x40c1f0
int CRobotPlayer::WasRecentlyVisited(CGameObj* pObj)
{
	CGameObj** ppRecent;
	int i;

	ppRecent = m_apRecent;
	for (i = 0; i < 10; i++)
	{
		if (*ppRecent == pObj)
			return 1;
		ppRecent++;
	}
	return 0;
}


/////////////////////////////////////////////////////////////////////////////
// The robot steering pass (CRobotPlayer::Steer)

// The steering pass below was compiled with the optimizer off, so its
// 16.16 fixed-point arithmetic stays real calls into CFixed (declared in
// hover.h). MulEq is defined in bumpview.cpp, next to its other
// caller; only the two used solely from here are defined below.

// Copying `this` into a local is what makes VC 2.2 home it in the single
// stack slot the original reserves; the `_asm` body then reads the
// argument straight out of the caller's slot and the function returns the
// same pointer.

// hover.exe:0x40A490
// FUNCTION: HOVER 0x40a490
CFixed* CFixed::DivEq(DWORD dwB)
{
	CFixed* pThis = this;

	_asm mov ecx, pThis
	_asm mov eax, [ecx]
	_asm mov edx, eax
	_asm sar edx, 0x10
	_asm shl eax, 0x10
	_asm idiv dwB
	_asm mov [ecx], eax

	return pThis;
}

// hover.exe:0x40B4F0
// FUNCTION: HOVER 0x40b4f0
CFixed* CFixed::MulShortEq(short sB)
{
	CFixed* pThis = this;

	_asm mov ecx, pThis
	_asm movsx eax, sB
	_asm imul dword ptr [ecx]
	_asm mov [ecx], eax

	return pThis;
}

// The inline wrappers the steering pass calls them through. Each one has
// its arguments and its result in stack temporaries of its own, which is
// what produces the long runs of `mov` pairs in the original. The value
// being worked on is always the *first* parameter: the original homes the
// second argument first and reaches the first one with `lea ecx`
// (notes/perfect-match.md rules 117 and 123).

// The in-place steps the steering pass expands inline. They are file-local
// free functions rather than CFixed members on purpose: adding members to
// the shared CFixed reshuffles the register allocation of every other
// function in this translation unit (rules 45 and 87) and cost 0x410F20
// eleven points. A pointer parameter folds away exactly like `this`.

inline void HoverFixAddEq(DWORD* pdwA, CFixed fB)
{
	*pdwA += fB.m_dwValue;
}

inline void HoverFixSubEq(DWORD* pdwA, CFixed fB)
{
	*pdwA -= (DWORD)(-(-(long)fB.m_dwValue));
}

inline void HoverFixHalfEq(DWORD* pdwA)
{
	*pdwA = (DWORD)((long)*pdwA / 2);
}

inline void HoverFixZeroEq(DWORD* pdwA)
{
	((WORD*)pdwA)[0] = 0;
	((WORD*)pdwA)[1] = 0;
}

inline DWORD HoverFixMul(DWORD dwA, DWORD dwB)
{
	((CFixed*)&dwA)->MulEq(((CFixed*)&dwB)->m_dwValue);
	return dwA;
}

// The same product where the result is handed to another expansion rather
// than stored: the original keeps it in a slot of its own there.
inline DWORD HoverFixMulR(DWORD dwA, DWORD dwB)
{
	DWORD dwR;

	((CFixed*)&dwA)->MulEq(((CFixed*)&dwB)->m_dwValue);
	dwR = dwA;
	return dwR;
}

inline DWORD HoverFixDiv(DWORD dwA, DWORD dwB)
{
	((CFixed*)&dwA)->DivEq(((CFixed*)&dwB)->m_dwValue);
	return dwA;
}

inline DWORD HoverFixMulW(DWORD dwA, short sB)
{
	((CFixed*)&dwA)->MulShortEq(sB);
	return dwA;
}

inline DWORD HoverFixAdd(DWORD dwA, DWORD dwB)
{
	HoverFixAddEq(&dwA, *(CFixed*)&dwB);
	return dwA;
}

// The sum of two squares is reached through one more level than the sums
// that are stored straight into a local, and keeps its result in a slot.
inline DWORD HoverFixAddR(DWORD dwA, DWORD dwB)
{
	DWORD dwR;

	HoverFixAddEq(&dwA, *(CFixed*)&dwB);
	dwR = dwA;
	return dwR;
}

inline DWORD HoverFixSum(DWORD dwA, DWORD dwB)
{
	return HoverFixAddR(dwB, dwA);
}

inline DWORD HoverFixSub(DWORD dwA, DWORD dwB)
{
	HoverFixSubEq(&dwA, *(CFixed*)&dwB);
	return dwA;
}

// Halving is a two-level expansion in the original (an in-place member
// inside the by-value wrapper); negation and absolute value are one.

inline DWORD HoverFixHalf(DWORD dwA)
{
	HoverFixHalfEq(&dwA);
	return dwA;
}

inline DWORD HoverFixNeg(DWORD dwA)
{
	dwA = (DWORD)(-(long)dwA);
	return dwA;
}

inline DWORD HoverFixAbs(DWORD dwA)
{
	dwA = (DWORD)labs((long)dwA);
	return dwA;
}

// A plain copy through a temporary: the original pushes every value it
// publishes through one of these, which is what the lone
// `mov eax, [t] / mov [dst], eax` pairs after a `jmp $+5` are.
inline DWORD HoverFixVal(DWORD dwA)
{
	DWORD dwR;

	dwR = dwA;
	return dwR;
}

inline DWORD HoverFixValOf(DWORD dwA)
{
	return dwA;
}

// 16.16 from a whole number. Spelling the quarter/half-turn constants
// this way is what keeps them in a register at the comparison
// (`mov eax, 0x1000000 / cmp eax, [x]`); a plain literal is canonicalized
// into the memory operand instead.
inline long HoverFixOfInt(short sN)
{
	return (long)sN << 16;
}

// 16.16 whose whole part is a heading word and whose fraction is zero.
inline void HoverFixSetWord(DWORD* pdwR, WORD wN)
{
	((WORD*)pdwR)[0] = 0;
	((WORD*)pdwR)[1] = wN;
}

inline void HoverFixOfWord(DWORD* pdwR, WORD wN)
{
	HoverFixSetWord(pdwR, wN);
}

// The whole part of a 16.16 value. Returning a reference makes the read
// fold into an lvalue: the expansion emits no code of its own, so its
// `jmp $+5` lands in *front* of the `mov ax, [x+2]` that the assignment
// then makes (hover.exe:0x409e3e and 0x409e6a).
inline short& HoverFixWhole(DWORD& dwV)
{
	return ((short*)&dwV)[1];
}

inline int HoverFixWholeInt(const DWORD& dwV)
{
	return (int)(short)((short*)&dwV)[1];
}

// The three in-place steps the steering pass does on a heading: add to the
// whole part, subtract from it, and wrap back into (-half turn, half turn].
inline void HoverFixAddWhole(DWORD* pdwA, short sN)
{
	((short*)pdwA)[1] += sN;
}

inline void HoverFixAddWholeN(DWORD* pdwA, const short& sN)
{
	((short*)pdwA)[1] += sN;
}

inline void HoverFixSubWhole(DWORD* pdwA, short sN)
{
	((short*)pdwA)[1] -= sN;
}

inline void HoverFixWrap(DWORD* pdwA)
{
	*pdwA &= 0x1FFFFFF;
}

// The heading helper is reached through an inline wrapper, which is what
// homes its two arguments in slots of their own before the push.
inline DWORD* HoverHeadingCall(DWORD* pdwBuf, DWORD dwDX, DWORD dwDY)
{
	DWORD* pdwR;

	pdwR = Hover_FixAngleP(pdwBuf, dwDY, dwDX);
	return pdwR;
}

inline DWORD* HoverHeadingOf(DWORD* pdwBuf, DWORD dwDX, DWORD dwDY)
{
	return HoverHeadingCall(pdwBuf, dwDX, dwDY);
}

// The robot's turn limit, read out of the options block through an inline
// whose body is a plain lvalue: it folds into the address arithmetic at the
// point of use and leaves only its expansion end in front of it.
inline long HoverAppTurn()
{
	return (long)theApp.m_pRobotAIParams->m_adw[3];
}

// The two components of a point are subtracted through one wrapper, which
// is what homes the source pointer in a slot of its own.
inline void HoverPtSub(DWORD* pdwA, const DWORD* pdwB)
{
	HoverFixSubEq(&pdwA[0], *(CFixed*)&pdwB[0]);
	HoverFixSubEq(&pdwA[1], *(CFixed*)&pdwB[1]);
}

inline DWORD HoverFixZero()
{
	DWORD dwR;

	HoverFixZeroEq(&dwR);
	return dwR;
}

inline void HoverZeroPoint(DWORD* pdwPt)
{
	HoverFixZeroEq(&pdwPt[0]);
	HoverFixZeroEq(&pdwPt[1]);
}

// The comparisons are split into an ">" and an "==" test the same way the
// original's were, so each leaf gets its own expansion (and its own dead
// "materialize the result" tail) in the unoptimized code below.

inline int HoverFixGt(DWORD dwA, DWORD dwB)
{
	return (long)dwA > (long)dwB;
}

inline int HoverFixEq(DWORD dwA, DWORD dwB)
{
	return dwA == dwB;
}

inline int HoverFixLt(DWORD dwA, DWORD dwB)
{
	return !HoverFixGt(dwA, dwB) && !HoverFixEq(dwA, dwB);
}

// Where the result of a comparison is not immediately branched on, the
// original's leaf materializes it: two `return`s and a third, unreachable
// `return` of an uninitialized local, which is what leaves the dead
// "copy the local into the result slot" tail behind every one of them
// (rule 159).

inline int HoverFixIsLt(DWORD dwA, DWORD dwB)
{
	int nR;

	return (long)dwA < (long)dwB ? 1 : 0;
	return nR;
}

inline int HoverFixIsGt(long lA, long lB)
{
	int nR;

	return lA > lB ? 1 : 0;
	return nR;
}

inline int HoverFixIsNeg(DWORD dwA)
{
	int nR;

	return (long)dwA < 0 ? 1 : 0;
	return nR;
}

// The 16.16 square root: the fractional half is divided out in double
// precision, the integer half added back, and the result scaled up again.

inline DWORD HoverFixSqrt(DWORD dwA)
{
	double dblRoot;
	DWORD dwR;

	dblRoot = sqrt((double)(DWORD)(dwA & 0xFFFF) / 65536.0
		+ (double)(short)((WORD*)&dwA)[1]);
	dwR = (DWORD)(long)(dblRoot * 65536.0);
	return dwR;
}

// The robot's steering/throttle pass, run once per frame from
// CRobotPlayer::Think while the game is in state 3. It publishes the
// two control values the physics step reads, m_fxTurnInput (turn) and
// m_fxThrustInput (throttle), both 16.16 and clamped to [-1, 1]:
//
//  - the heading to the current AI target (m_pTarget) goes into
//    m_fxTargetHeading, and, while the target is close enough that the robot
//    would overshoot it, m_bOvershooting latches the "drifting past it" state
//    so the robot steers by its own velocity vector instead;
//  - the heading error against m_fxHeading is normalized into
//    (-half turn, +half turn], reversed when it exceeds a quarter turn
//    (which makes the robot back up instead of turning round), and
//    divided by the robot's turn parameter to give the turn control;
//  - the throttle is the difference between the wanted speed (0 while
//    drifting, m_fxMaxSpeed otherwise) and the current speed, divided by
//    the robot's acceleration parameter, and is zeroed whenever the robot
//    is not pointing close enough at its target.
//
// The original compiled this one function with the optimizer off but
// inline expansion on (`#pragma optimize("", off)` under /O2): every
// temporary is homed on the stack, every inline expansion ends in a jump
// to the next statement, and each inlined comparison leaves an
// unreachable "test the result" tail behind it. Reproducing the pragma is
// what keeps the shape of the reconstruction comparable at all -- see
// notes/perfect-match.md rule 49. Every intermediate value below is a
// named local of its own because the original's frame has one slot per
// expansion result.

// VC 2.1 lays a /Od frame's *named* locals out in the order its identifier
// hash table walks them, not in declaration order (notes/perfect-match.md
// rules 79 and 125), so the only way to reproduce hover.exe's frame slot
// for slot is to put every local of the steering pass into one structure
// and spell its members out in reverse frame order: a struct is laid out
// in declaration order from its own base, and a function whose only local
// is the struct puts that base at the bottom of the named region. The
// member comments give the original's `ebp` displacement.


#pragma optimize("", off)

// hover.exe:0x409350
// FUNCTION: HOVER 0x409350
void CRobotPlayer::Steer()
{
	struct ROBOTSTEER
	{
		DWORD dwTmp;			// ebp-0xa0 the ?: result
		DWORD dwNegThrottle;		// ebp-0x9c
		DWORD dwThrottle;		// ebp-0x98
		DWORD dwWantDelta;		// ebp-0x94
		DWORD dwAbs2;			// ebp-0x90
		DWORD dwTurnCtl;		// ebp-0x8c
		DWORD dwNegFacing;		// ebp-0x88
		DWORD dwDelta2;			// ebp-0x84
		DWORD dwTurn;			// ebp-0x80
		DWORD dwNeg;			// ebp-0x7c
		DWORD dwVelFix3;		// ebp-0x78
		DWORD dwZero;			// ebp-0x74
		DWORD dwAbs;			// ebp-0x70
		DWORD dwVelFix2;		// ebp-0x6c
		DWORD dwVelFix;			// ebp-0x68
		DWORD dwStop;			// ebp-0x64
		DWORD dwHalf;			// ebp-0x60
		DWORD dwBrake;			// ebp-0x5c
		DWORD dwSquare;			// ebp-0x58
		DWORD adwAngle[1];		// ebp-0x54 heading helper's output
		DWORD adwSelf[2];		// ebp-0x50 our own position
		short sSteer;			// ebp-0x48
		WORD wPad;				// ebp-0x46 (alignment, as in the original)
		DWORD dwDelta;			// ebp-0x44
		int nDistSide;			// ebp-0x40
		DWORD adwAlong[2];		// ebp-0x3c
		DWORD adwSide[2];		// ebp-0x34
		int nVelAngle;			// ebp-0x2c
		int nRange;				// ebp-0x28
		int nDistTarget;		// ebp-0x24
		DWORD adwTarget[2];		// ebp-0x20
		DWORD dwWanted;			// ebp-0x18
		DWORD dwSpeed;			// ebp-0x14
		int nLos;				// ebp-0x10
		DWORD dwFacing;			// ebp-0xc
		DWORD dwHeading;		// ebp-8
		int bReverse;			// ebp-4
	};
	ROBOTSTEER f;

	m_dwUnk01C8 = 0;
	f.nLos = (int)m_bStopping;
	f.dwSpeed = HoverFixSqrt(HoverFixSum(
		HoverFixMulR(m_fxVelY, m_fxVelY),
		HoverFixMulR(m_fxVelX, m_fxVelX)));
	HoverFixZeroEq(&f.dwWanted);

	if (m_pTarget != 0 && f.nLos == 0)
	{
		(m_pTarget)->GetFixedPos(f.adwTarget);
		HoverPtSub(f.adwTarget, GetFixedPos(f.adwSelf));
		m_fxTargetHeading = HoverFixValOf(*Hover_FixAngleP(f.adwAngle,
			f.adwTarget[1], f.adwTarget[0]));
		if (m_bBrakeAtTarget != 0)
		{
			// The distance the robot needs to come to a stop:
			// v + (v*v / brake) / 2.
			f.dwSquare = HoverFixMul(f.dwSpeed, f.dwSpeed);
			f.dwBrake = HoverFixDiv(f.dwSquare, m_fxThrustAccel);
			f.dwHalf = HoverFixHalf(f.dwBrake);
			f.dwStop = HoverFixAdd(f.dwHalf, f.dwSpeed);
			f.nRange = HoverFixWholeInt(f.dwStop);
			f.nDistTarget = (int)Hover_ApproxDistance(
				HoverFixWholeInt(f.adwTarget[0]),
				HoverFixWholeInt(f.adwTarget[1]), 0, 0);
			if (f.nRange * 2 > f.nDistTarget)
			{
				HoverFixOfWord(&f.dwVelFix,
					((WORD*)HoverHeadingOf(f.adwAngle, m_fxVelX,
					m_fxVelY))[1]);
				f.nVelAngle = HoverFixWholeInt(f.dwVelFix);
				HoverZeroPoint(f.adwAlong);
				HoverZeroPoint(f.adwSide);
				Hover_SplitVelocity(f.adwTarget[0], f.adwTarget[1], f.nVelAngle,
					f.adwAlong, f.adwSide);
				if (m_bOvershooting != 0)
				{
					// Once the sideways component has flipped sign the
					// robot has passed its target.
					if (HoverFixIsNeg(f.adwSide[0]) !=
						HoverFixLt(0, m_fxVelX))
					{
						m_bOvershooting = 1;
						f.nLos = 1;
					}
				}
				if (f.nLos == 0)
				{
					f.nDistSide = (int)Hover_ApproxDistance(
						HoverFixWholeInt(f.adwSide[0]),
						HoverFixWholeInt(f.adwSide[1]), 0, 0);
					// hover.exe:0x4098f1 spells this `nRange > nDistSide`
					// (`mov eax,[nDistSide] / cmp [nRange],eax / jle`);
					// with the locals in a structure VC canonicalizes `>`
					// back into `<` and both spellings give the `<` form.
					if (f.nRange > f.nDistSide)
					{
						m_bOvershooting = 1;
						f.nLos = 1;
					}
				}
			}
			else
			{
				m_bOvershooting = 0;
				f.nLos = 0;
			}
		}
	}

	f.dwHeading = m_fxTargetHeading;
	f.bReverse = 0;
	if (f.nLos != 0 || m_bOvershooting != 0)
	{
		// Drifting: steer by the velocity vector, and turn round when it
		// is more than a quarter turn away from the current heading.
		HoverFixOfWord(&f.dwVelFix2,
			((WORD*)HoverHeadingOf(f.adwAngle, m_fxVelX,
			m_fxVelY))[1]);
		f.dwHeading = HoverFixVal(f.dwVelFix2);
		f.dwDelta = HoverFixSub(m_fxHeading, f.dwHeading);
		HoverFixWrap(&f.dwDelta);
		if (HoverFixLt((DWORD)HoverFixOfInt(0x100), f.dwDelta))
			HoverFixSubWhole(&f.dwDelta, 0x200);
		f.dwAbs = HoverFixAbs(f.dwDelta);
		if (HoverFixLt((DWORD)HoverFixOfInt(0x80), f.dwAbs))
		{
			HoverFixAddWhole(&f.dwHeading, 0x100);
			HoverFixWrap(&f.dwHeading);
			f.bReverse = 1;
		}
	}

	if (f.nLos != 0)
	{
		HoverFixZeroEq(&f.dwZero);
		f.dwTmp = f.dwZero;
	}
	else
	{
		f.dwTmp = m_fxMaxSpeed;
	}
	f.dwWanted = HoverFixVal(f.dwTmp);

	f.dwFacing = f.dwHeading;
	if (HoverFixLt((DWORD)HoverFixOfInt(1), f.dwSpeed))
	{
		HoverFixOfWord(&f.dwVelFix3,
			((WORD*)HoverHeadingOf(f.adwAngle, m_fxVelX,
			m_fxVelY))[1]);
		f.dwFacing = HoverFixVal(f.dwVelFix3);
		if (m_bSpinning != 0)
		{
			HoverFixAddWhole(&f.dwFacing, 0x100);
			HoverFixWrap(&f.dwFacing);
		}
	}
	else
	{
		m_bOvershooting = 0;
	}
	HoverFixSubEq(&f.dwFacing, *(CFixed*)&f.dwHeading);
	HoverFixWrap(&f.dwFacing);
	if (HoverFixLt((DWORD)HoverFixOfInt(0x100), f.dwFacing))
		HoverFixSubWhole(&f.dwFacing, 0x200);

	if (f.nLos == 0 && m_bOvershooting == 0)
	{
		if (labs((long)HoverFixWhole(f.dwFacing)) < HoverAppTurn())
		{
			// The first assignment is overwritten immediately; the
			// original has both.
			f.dwNeg = HoverFixNeg(f.dwFacing);
			f.sSteer = HoverFixWhole(f.dwNeg);
			f.dwTurn = HoverFixMulW(f.dwFacing, -5);
			f.sSteer = HoverFixWhole(f.dwTurn);
			if (HoverAppTurn() < (long)f.sSteer)
				f.sSteer = (short)HoverAppTurn();
			if (-HoverAppTurn() > (long)f.sSteer)
				f.sSteer = (short)-HoverAppTurn();
			HoverFixAddWholeN(&f.dwHeading, f.sSteer);
		}
	}

	f.dwDelta2 = HoverFixSub(m_fxHeading, f.dwHeading);
	f.dwFacing = HoverFixVal(f.dwDelta2);
	HoverFixWrap(&f.dwFacing);
	if (HoverFixLt((DWORD)HoverFixOfInt(0x100), f.dwFacing))
		HoverFixSubWhole(&f.dwFacing, 0x200);
	f.dwNegFacing = HoverFixNeg(f.dwFacing);
	f.dwTurnCtl = HoverFixDiv(f.dwNegFacing, m_pParams->m_adw[10]);
	m_fxTurnInput = HoverFixVal(f.dwTurnCtl);
	if (HoverFixIsLt(m_fxTurnInput, 0xFFFF0000))
	{
		m_wTurnInput = 0xFFFF;
		m_wTurnInputFrac = 0;
	}
	if (HoverFixLt((DWORD)HoverFixOfInt(1), m_fxTurnInput))
	{
		m_wTurnInput = 1;
		m_wTurnInputFrac = 0;
	}

	// One `else` for the two ways of giving up, which is why the whole
	// guard is one condition in the original.
	if ((m_bOvershooting == 0 || f.nLos != 0)
		&& HoverFixIsGt(HoverFixOfInt((short)theApp.m_pRobotAIParams->m_adw[4]),
			(long)(f.dwAbs2 = HoverFixAbs(f.dwFacing))))
	{
		f.dwWantDelta = HoverFixSub(f.dwWanted, f.dwSpeed);
		f.dwThrottle = HoverFixDiv(f.dwWantDelta, m_fxThrustAccel);
		m_fxThrustInput = HoverFixVal(f.dwThrottle);
		if (HoverFixIsLt(m_fxThrustInput, 0xFFFF0000))
		{
			m_wThrustInput = 0xFFFF;
			m_wThrustInputFrac = 0;
		}
		if (HoverFixLt((DWORD)HoverFixOfInt(1), m_fxThrustInput))
		{
			m_wThrustInput = 1;
			m_wThrustInputFrac = 0;
		}
		if (m_bSpinning != 0 || f.bReverse != 0)
		{
			f.dwNegThrottle = HoverFixNeg(m_fxThrustInput);
			m_fxThrustInput = HoverFixVal(f.dwNegThrottle);
		}
	}
	else
	{
		m_wThrustInput = 0;
		m_wThrustInputFrac = 0;
	}
}

#pragma optimize("", on)

// hover.exe:0x40BC10
// FUNCTION: HOVER 0x40bc10
void CRobotPlayer::RememberVisited(CGameObj* pObj)
{
	m_apRecent[m_dwRecentNext] = pObj;
	m_dwRecentNext++;
	if (m_dwRecentNext == 10)
		m_dwRecentNext = 0;
}

// hover.exe:0x42C320
// FUNCTION: HOVER 0x42c320
void* CRobotPlayer::operator new(size_t cb)
{
	CRobotPlayer* pBlock;
	int nCount;
	int i;

	pBlock = g_pRobotFree;
	if (pBlock == NULL)
	{
		g_pRobotFree = (CRobotPlayer*)::operator new(
			g_dwRobotPoolBlock * sizeof(CRobotPlayer));
		g_listRobotBlocks.AddTail((void*)g_pRobotFree);
		nCount = (int)g_dwRobotPoolBlock;
		pBlock = g_pRobotFree;
		for (i = 0; i < nCount - 1; i++)
			pBlock[i].m_pNextFree = &pBlock[i + 1];
		pBlock[nCount - 1].m_pNextFree = NULL;
	}
	g_pRobotFree = pBlock->m_pNextFree;
	return pBlock;
}

/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// CRenderContext

// Builds a render context: default camera/shade constants, mode 1 with both
// option bits clear, empty tables, then the two blocks the context owns for
// its whole life - the 0x800-byte shift-count table filled by
// BuildMipShiftTable and the 0x44-byte span-drawer function table whose
// +0x18..+0x3C slots SetMode rewrites for every mode. Note that the
// SetMode(1, 1) call happens *before* m_ppfnDrawFlatByShift exists: mode 1
// with dwFlags == 1 takes the (dwFlags & 2) == 0 path, which only writes
// m_pfnDrawWall/m_pfnDrawFlat and never dereferences
// m_ppfnDrawFlatByShift.

// hover.exe:0x420ED0
// FUNCTION: HOVER 0x420ed0
CRenderContext::CRenderContext()
{
	m_dblUnkA0 = 64.0;
	m_dblUnk98 = 64.0;
	m_dwHorizon = 0;
	m_dwUnk58 = 0x80;
	m_dwLastCamZ = (DWORD)-1;
	m_dwCamZ = 0x40;

	SetMode(1, 1);

	m_pClipTop = NULL;
	m_pClipBottom = NULL;
	m_pColumnScale = NULL;
	m_pRayDirs = NULL;
	m_pSkyGradient = NULL;
	m_pRowDistance = NULL;
	m_bMirror = 0;

	m_pMipShift = (BYTE*)::operator new(0x800);
	m_ppfnDrawFlatByShift = (SPANFN*)::operator new(17 * sizeof(SPANFN));
}

// Releases the eight tables the render context owns. The original has no
// null checks: operator delete tolerates NULL.

// hover.exe:0x420F60
// FUNCTION: HOVER 0x420f60
CRenderContext::~CRenderContext()
{
	delete m_ppfnDrawFlatByShift;
	delete m_pMipShift;
	delete m_pClipTop;
	delete m_pClipBottom;
	delete m_pColumnScale;
	delete m_pRayDirs;
	delete m_pSkyGradient;
	delete m_pRowDistance;
}
