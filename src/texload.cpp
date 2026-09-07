// texload.cpp : loader/helper functions decompiled from hover.exe
//
// See texload.h for the class layouts and RTTI/vtable data. The MIDI
// stream loader (CSub425050::Sub_437F00) parses the RIFF "MIDS" .muz
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
CSub425050* g_pUnk_4C5904;		// object that opened the current MIDI stream

// hover.exe:0x4C5908
// GLOBAL: HOVER 0x4c5908
LPMIDIDEVICEFN g_pfnUnk_4C5908;	// midiStreamOpen

// hover.exe:0x4C5910
// GLOBAL: HOVER 0x4c5910
LPMIDIDEVICEFN g_pfnUnk_4C5910;	// midiStreamProperty

// hover.exe:0x4C5914
// GLOBAL: HOVER 0x4c5914
LPMIDIDEVICEFN g_pfnUnk_4C5914;	// midiStreamPosition

// hover.exe:0x4C5918
// GLOBAL: HOVER 0x4c5918
LPMIDIDEVICEFN g_pfnUnk_4C5918;	// midiStreamOut

// hover.exe:0x4C5920
// GLOBAL: HOVER 0x4c5920
LPMIDIDEVICEFN g_pfnUnk_4C5920;	// midiStreamRestart

// hover.exe:0x4C5924
// GLOBAL: HOVER 0x4c5924
LPMIDIDEVICEFN g_pfnUnk_4C5924;	// midiStreamStop

// hover.exe:0x4C47B0
// GLOBAL: HOVER 0x4c47b0
char g_szUnk_4C47B0[] = "winmm.dll";

// hover.exe:0x4C5928
// GLOBAL: HOVER 0x4c5928
char g_szUnk_4C5928[] = "midiStreamStop";

// hover.exe:0x4C5938
// GLOBAL: HOVER 0x4c5938
char g_szUnk_4C5938[] = "midiStreamRestart";

// hover.exe:0x4C594C
// GLOBAL: HOVER 0x4c594c
char g_szUnk_4C594C[] = "midiStreamPause";

// hover.exe:0x4C595C
// GLOBAL: HOVER 0x4c595c
char g_szUnk_4C595C[] = "midiStreamOut";

// hover.exe:0x4C596C
// GLOBAL: HOVER 0x4c596c
char g_szUnk_4C596C[] = "midiStreamPosition";

// hover.exe:0x4C5980
// GLOBAL: HOVER 0x4c5980
char g_szUnk_4C5980[] = "midiStreamProperty";

// hover.exe:0x4C5994
// GLOBAL: HOVER 0x4c5994
char g_szUnk_4C5994[] = "midiStreamClose";

// hover.exe:0x4C59A4
// GLOBAL: HOVER 0x4c59a4
char g_szUnk_4C59A4[] = "midiStreamOpen";

// hover.exe:0x4C54D4
// GLOBAL: HOVER 0x4c54d4
DWORD g_dwUnk_4C54D4 = 0xA;		// CRobotPlayer pool chunk size

// hover.exe:0x4C5624
// GLOBAL: HOVER 0x4c5624
DWORD g_dwUnk_4C5624 = 0x20;	// renderer-sprite pool chunk size

// hover.exe:0x4C4DB0
// GLOBAL: HOVER 0x4c4db0
WORD g_awUnk_4C4DB0[512] =		// g_awUnk_4C4DB0[255 + d] = d * d
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
// CSub425050

// hover.exe:0x437F00
// FUNCTION: HOVER 0x437f00
int CSub425050::Sub_437F00(const char* pszPath)
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
	Sub_438260();
	if (g_pfnUnk_4C5908 == NULL)
	{
		hMod = GetModuleHandleA(g_szUnk_4C47B0);
		if (hMod == NULL)
			return 0;
		g_pfnUnk_4C5908 =
			(LPMIDIDEVICEFN)GetProcAddress(hMod, g_szUnk_4C59A4);
		g_pfnUnk_4C590C =
			(LPMIDIDEVICEFN)GetProcAddress(hMod, g_szUnk_4C5994);
		g_pfnUnk_4C5910 =
			(LPMIDIDEVICEFN)GetProcAddress(hMod, g_szUnk_4C5980);
		g_pfnUnk_4C5914 =
			(LPMIDIDEVICEFN)GetProcAddress(hMod, g_szUnk_4C596C);
		g_pfnUnk_4C5918 =
			(LPMIDIDEVICEFN)GetProcAddress(hMod, g_szUnk_4C595C);
		g_pfnUnk_4C591C =
			(LPMIDIDEVICEFN)GetProcAddress(hMod, g_szUnk_4C594C);
		g_pfnUnk_4C5920 =
			(LPMIDIDEVICEFN)GetProcAddress(hMod, g_szUnk_4C5938);
		g_pfnUnk_4C5924 =
			(LPMIDIDEVICEFN)GetProcAddress(hMod, g_szUnk_4C5928);
		if (g_pfnUnk_4C5908 == NULL || g_pfnUnk_4C590C == NULL ||
			g_pfnUnk_4C5910 == NULL || g_pfnUnk_4C5914 == NULL ||
			g_pfnUnk_4C5918 == NULL || g_pfnUnk_4C591C == NULL ||
			g_pfnUnk_4C5920 == NULL || g_pfnUnk_4C5924 == NULL)
		{
			g_pfnUnk_4C5908 = NULL;
			return 0;
		}
	}
	m_dwUnk04 = (DWORD)CreateFileA(pszPath, GENERIC_READ, FILE_SHARE_READ,
		NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (m_dwUnk04 == 0xFFFFFFFF)
		goto lFail;
	if (!ReadFile((HANDLE)m_dwUnk04, &dwA, 4, &dwRead, NULL))
		goto lFail;
	if (dwA != 0x46464952)		// "RIFF"
		goto lFail;
	if (!ReadFile((HANDLE)m_dwUnk04, &dwB, 4, &dwRead, NULL))
		goto lFail;
	if (!ReadFile((HANDLE)m_dwUnk04, &dwA, 4, &dwRead, NULL))
		goto lFail;
	if (dwA != 0x5344494D)		// "MIDS"
		goto lFail;
	if (!ReadFile((HANDLE)m_dwUnk04, &dwA, 4, &dwRead, NULL))
		goto lFail;
	if (!ReadFile((HANDLE)m_dwUnk04, &dwB, 4, &dwRead, NULL))
		goto lFail;
	if (dwA != 0x20746D66)		// "fmt "
		goto lFail;
	if (dwB < 8)
		goto lFail;
	if (!ReadFile((HANDLE)m_dwUnk04, &m_dwUnk08, 8, &dwRead, NULL))
		goto lFail;
	if (dwB > 8)
		SetFilePointer((HANDLE)m_dwUnk04, dwB - 8, NULL, FILE_CURRENT);
	if (!ReadFile((HANDLE)m_dwUnk04, &dwA, 4, &dwRead, NULL))
		goto lFail;
	if (!ReadFile((HANDLE)m_dwUnk04, &dwB, 4, &dwRead, NULL))
		goto lFail;
	if (dwA != 0x61746164)		// "data"
		goto lFail;
	if (!ReadFile((HANDLE)m_dwUnk04, &dwBlocks, 4, &dwRead, NULL))
		goto lFail;
	m_dwUnk10 = 0;
	m_dwUnk14 = 0;
	while (dwBlocks-- != 0)
	{
		// per block: 8-byte header (only the second dword, the data
		// length, is used), then the MIDI stream data
		if (!ReadFile((HANDLE)m_dwUnk04, adwHdr, 8, &dwRead, NULL))
			goto lFail;
		// 0x2042 = GMEM_MOVEABLE | GMEM_ZEROINIT | GMEM_DDESHARE
		pNode = (MIDIHDR*)GlobalLock(GlobalAlloc(0x2042, adwHdr[1] + 0x40));
		if (pNode == NULL)
		{
			AfxThrowMemoryException();
			if (pNode == NULL)
				goto lFail;
		}
		LPSTR pData = (LPSTR)pNode + 0x40;
		pNode->lpData = pData;
		pNode->dwBufferLength = adwHdr[1];
		pNode->dwBytesRecorded = adwHdr[1];
		if (!ReadFile((HANDLE)m_dwUnk04, pData, adwHdr[1],
			&dwRead, NULL))
			goto lFail;
		if (m_dwUnk14 == 0)
		{
			m_dwUnk14 = (DWORD)pNode;
			m_dwUnk10 = (DWORD)pNode;
		}
		else
		{
			((MIDIHDR*)m_dwUnk14)->lpNext = pNode;
			m_dwUnk14 = (DWORD)pNode;
		}
	}
	bLoaded = 1;
lFail:
	if (bLoaded == 0)
		Sub_438260();
	return bLoaded;
}

// hover.exe:0x41C030
// FUNCTION: HOVER 0x41c030
int CSub425050::Sub_41C030(const char* pszPath)
{
	if (Sub_437F00(pszPath) == 0)
		return 0;
	g_pUnk_4C5904 = this;
	return 1;
}

/////////////////////////////////////////////////////////////////////////////
// CRegionMatrix

// hover.exe:0x415030
// FUNCTION: HOVER 0x415030
REGIONNODE::tagREGIONNODE()
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
	delete[] m_pUnk04;
}

// hover.exe:0x4086A0
// FUNCTION: HOVER 0x4086a0
REGIONNODE* CRegionMatrix::Sub_4086A0(int nX, int nY, CGameObj* pObj)
{
	REGIONNODE* pNode;
	REGIONNODE* pHead;
	REGIONNODE* pCur;
	REGIONNODE** ppCell;
	int nLeft;
	int nCount;

	// The head of the free list is loaded once here and returned below.
	pNode = (REGIONNODE*)g_dwUnk_4C55D0;
	if (pNode == NULL)
	{
		// Allocate a fresh chunk of REGIONNODEs and link them into the
		// g_dwUnk_4C55D0 free list. The chunk pointer is also registered in
		// g_listUnk_4A46F8 so Hover_sub_42C700 can free it at shutdown.
		pNode = new REGIONNODE[g_dwUnk_4C55D4];
		g_dwUnk_4C55D0 = (DWORD)pNode;
		g_listUnk_4A46F8.AddTail(pNode);

		nCount = (int)g_dwUnk_4C55D4;
		pNode = (REGIONNODE*)g_dwUnk_4C55D0;
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

	g_dwUnk_4C55D0 = (DWORD)pNode->m_pNext;

	pNode->m_pNext = NULL;
	pNode->m_pPrev = NULL;
	pNode->m_pObj = NULL;
	pNode->m_pNextObj = NULL;

	ppCell = (REGIONNODE**)&m_pUnk04[m_dwUnk08 * nY + nX];
	pNode->m_pPrev = (REGIONNODE*)ppCell;
	pHead = *ppCell;
	pNode->m_pNext = pHead;
	if (pHead != NULL)
		pHead->m_pPrev = pNode;
	*ppCell = pNode;

	pNode->m_pObj = pObj;
	pNode->m_pNextObj = (REGIONNODE*)pObj->m_dwUnk30;
	pObj->m_dwUnk30 = (DWORD)pNode;

	return pNode;
}

/////////////////////////////////////////////////////////////////////////////
// CCollider

// hover.exe:0x41B560
// FUNCTION: HOVER 0x41b560
CCollider::CCollider(CSize sizeExtent, DWORD dwCell)
	: m_listUnk28(10)
{
	int nCols;
	int nRows;

	// Signed divisions in the original, and the cell count and the cell
	// array are read back out of the matrix for everything that follows.
	nCols = (int)sizeExtent.cx / (int)dwCell;
	nRows = (int)sizeExtent.cy / (int)dwCell;
	m_dwUnk04 = (DWORD)nCols;
	m_dwUnk08 = (DWORD)nRows;
	m_dwUnk0C = dwCell;
	m_obj10.m_dwUnk10 = (DWORD)(nCols * nRows);
	m_obj10.m_pUnk04 = new DWORD[m_obj10.m_dwUnk10];
	m_obj10.m_dwUnk08 = (DWORD)nCols;
	m_obj10.m_dwUnk0C = (DWORD)nRows;
	m_obj10.m_pUnk14 = m_obj10.m_pUnk04 + m_obj10.m_dwUnk10;
	memset(m_obj10.m_pUnk04, 0, m_obj10.m_dwUnk10 * 4);
	m_dwUnk44 = 0;
}

CCollider::~CCollider()
{
	// member destruction only in the original as well
}

// hover.exe:0x40F900
// FUNCTION: HOVER 0x40f900
int CCollider::Sub_40F900(CGameObj* pObj)
{
	// Virtual call at vtable slot 0x30 in the original
	// (hover.exe:0x408820 on CPlayer), passing this object.
	pObj->Sub_408820((DWORD)this);
	return 1;
}

// hover.exe:0x40F8A0
// FUNCTION: HOVER 0x40f8a0
void CCollider::Sub_40F8A0(int nX, int nY)
{
	REGIONNODE* pNode;
	int nSize;
	int i;
	int bFound;

	pNode = (REGIONNODE*)m_obj10.m_pUnk04[m_obj10.m_dwUnk08 * nY + nX];
	while (pNode != NULL)
	{
		// bFound is zeroed first so its register serves as the known
		// zero the loop guard compares the size against (rule 6).
		bFound = 0;
		i = 0;
		nSize = m_arrUnk48.GetSize();
		for (; i < nSize; i++)
		{
			if ((CGameObj*)m_arrUnk48.GetAt(i) == pNode->m_pObj)
			{
				bFound = 1;
				break;
			}
		}
		if (!bFound)
			m_arrUnk48.SetAtGrow(nSize, pNode->m_pObj);
		pNode = pNode->m_pNext;
	}
}

// hover.exe:0x40F9B0
// FUNCTION: HOVER 0x40f9b0
int CCollider::Sub_40F9B0(POINT pt, long lR)
{
	REGIONNODE* pNode;
	long lH;
	long lW;
	long lCol;
	long lRow;
	int bBlocked;

	lW = (long)(m_dwUnk04 * m_dwUnk0C);
	lH = (long)(m_dwUnk08 * m_dwUnk0C);
	lCol = pt.x / (long)m_dwUnk0C;
	lRow = pt.y / (long)m_dwUnk0C;
	// One guard, one epilogue: the original branches all four range
	// tests to the same `return 1` (rule 38).
	if (pt.x - lR <= 0 || pt.x + lR >= lW || pt.y - lR <= 0 || pt.y + lR >= lH)
		return 1;
	pNode = (REGIONNODE*)m_obj10.m_pUnk04[
		(long)m_obj10.m_dwUnk08 * lRow + lCol];
	if (pNode == NULL)
		return 0;
	bBlocked = 0;
	do
	{
		// CFloorObj instances do not block the spot
		if (!pNode->m_pObj->IsKindOf(RUNTIME_CLASS(CFloorObj)) &&
			pNode->m_pObj->Sub_42AD70(pt.x, pt.y, lR))
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
// CBumperView::Sub_427E90.

// For every registered object, collect the objects that share one of its
// region-matrix cells (skipping itself, anything already visited earlier
// in the registration list, and duplicates), then ask each of them
// whether it wants the collision (vtable slot 11, Sub_42AD60) and hand
// the pair to the owner's collision handler (vtable slot 8, Sub_42AD40).
// The candidate list is a CPtrList local, emptied after each object.

// hover.exe:0x40A650
// FUNCTION: HOVER 0x40a650
void CCollider::Sub_40A650()
{
	CObList listHits(10);
	POSITION pos;
	POSITION posOther;
	REGIONNODE* pChain;
	REGIONNODE* pCell;
	CGameObj* pObj;
	CGameObj* pOther;
	int bSeen;

	pos = m_listUnk28.GetHeadPosition();
	while (pos != NULL)
	{
		pObj = (CGameObj*)m_listUnk28.GetNext(pos);
		pChain = (REGIONNODE*)pObj->m_dwUnk30;
		while (pChain != NULL)
		{
			// Walk back to the head of the cell's node list: the head is
			// the node whose m_pPrev points into the cell array itself.
			pCell = pChain;
			while (pCell->m_pPrev < (REGIONNODE*)m_obj10.m_pUnk04 ||
				(REGIONNODE*)m_obj10.m_pUnk14 < pCell->m_pPrev)
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
					posOther = m_listUnk28.GetHeadPosition();
					while (posOther != pos)
					{
						// The volatile read keeps VC from hoisting
						// pCell->m_pObj into a register across the whole
						// pair test, which is what costs pObj its own
						// register in the original (rule 26's lever).
						if (m_listUnk28.GetNext(posOther) ==
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
			if (pOther->Sub_42AD60((DWORD)pObj))
				pObj->Sub_42AD40((CPlayer*)pOther, 1);
		}
		listHits.RemoveAll();
	}
}

/////////////////////////////////////////////////////////////////////////////
// CSub40B510 (gameobj.h's skeletal declaration of CCollider)

// hover.exe:0x41B770
// FUNCTION: HOVER 0x41b770
int CCollider::Sub_41B770(CGameObj* pObj)
{
	CCollider* pThis;

	pThis = (CCollider*)this;
	pThis->m_listUnk28.AddHead(pObj);
	// Virtual call at vtable slot 0x30 in the original
	// (hover.exe:0x408820 on CPlayer), passing this object.
	pObj->Sub_408820((DWORD)this);
	return 1;
}

/////////////////////////////////////////////////////////////////////////////
// CPlayer mirror method

// hover.exe:0x41B0E0
// FUNCTION: HOVER 0x41b0e0
void CPlayer::Sub_41B0E0(POINT pt, DWORD dwC, DWORD dwD)
{
	// The zero handed to Sub_408550 is a 16.16 local the original builds
	// half by half out of the same zero register as the member stores.
	DWORD dwZero;
	long lX;
	long lY;

	lX = pt.x;
	lY = pt.y;
	Sub_40F880(lX, lY);
	m_awUnk0090[9] = 0x40;			// +0xA2
	m_awUnk0194[5] = (WORD)dwC;		// +0x19E
	m_awUnk0194[4] = 0;				// +0x19C
	m_awUnk0090[8] = 0;				// +0xA0
	((WORD*)&dwZero)[0] = 0;
	((WORD*)&dwZero)[1] = 0;
	*(DWORD*)&m_awUnk0090[10] = *(DWORD*)&m_awUnk0090[8];	// +0xA4
	m_dwUnk01A4 = dwD;
	*(DWORD*)&m_pad00A8[8] = dwD;	// +0xB0
	*(DWORD*)&m_pad00A8[4] = dwD;	// +0xAC
	Sub_408550(dwZero);
	Sub_408530();
}

// hover.exe:0x41B2D0
// FUNCTION: HOVER 0x41b2d0
void CPlayer::Sub_41B2D0(POINT pt, DWORD dwC, DWORD dwD)
{
	Sub_41A580();
	Sub_414A80();
	Sub_414A80();
	m_dwUnk0140 = 0;
	m_dwUnk0144 = 0;
	m_dwUnk0124 = 0;
	m_dwUnk0128 = 0;
	m_dwUnk012C = 0;
	m_dwUnk0130 = 0;
	m_dwUnk0134 = 0;
	m_dwUnk0138 = 0;
	Sub_41B0E0(pt, dwC, 0x40);
}

/////////////////////////////////////////////////////////////////////////////
// CSub410F20

// hover.exe:0x406C00
// FUNCTION: HOVER 0x406c00
void CSub410F20::Sub_406C00(DWORD dwA, DWORD dwB, DWORD dwC, DWORD dwD)
{
	// The camera position pair is written through a base pointer of its
	// own (rule 25).
	DWORD dwAngle = dwD & 0x1FF;
	DWORD* pdwPos = &m_dwUnk48;
	int bChanged;

	m_dwUnk50 = dwC;
	m_dwUnk54 = dwAngle;
	// The compare happens before the camera pair is stored, so the flag
	// is a separate value in the source.
	bChanged = (m_dwUnkA8 != dwC);
	pdwPos[0] = dwA;
	pdwPos[1] = dwB;
	if (bChanged)
		m_dwUnkA8 = dwC;
}

// FUNCTION: CSub410F20::Sub_410EC0
// hover.exe:0x410EC0
// FUNCTION: HOVER 0x410ec0
void CSub410F20::Sub_410EC0()
{
	int i = 0;
	int nAccum = 0;
	int nStep = m_dwUnk40 << 6;
	int nDivisor = m_dwUnk8C * 5;

	for (; i < 0x8000; i += 16)
	{
		int nValue = nAccum / nDivisor;
		int nCount = 0;
		while (nValue != 0)
		{
			nValue >>= 1;
			nCount++;
		}
		m_pUnk14[i >> 4] = (BYTE)nCount;
		nAccum += nStep;
	}
}

// Copies the caller-supplied top/bottom span arrays into the render context.
// hover.exe:0x414A10
// FUNCTION: HOVER 0x414a10
void CSub410F20::Sub_414A10(void* pA, void* pB)
{
	int nCount = (int)m_dwUnk88;

	if (nCount > 0)
	{
		DWORD* pTop = m_pUnk34;
		DWORD* pBottom = m_pUnk38;

		memcpy(pBottom, pB, nCount * sizeof(DWORD));
		memcpy(pTop, pA, nCount * sizeof(DWORD));
	}
}

// FUNCTION: CSub410F20::Sub_419940
// hover.exe:0x419940
// FUNCTION: HOVER 0x419940
void CSub410F20::Sub_419940(DWORD dwA, DWORD dwB)
{
	m_dwUnk74 = dwA;
	m_dwUnk70 = dwB;
}

// hover.exe:0x419960
// FUNCTION: HOVER 0x419960
void CSub410F20::Sub_419960(int nBase, BYTE bRed, BYTE bGreen, BYTE bBlue,
	CPalette* pPalette)
{
	int i;
	BYTE bR;
	BYTE bG;
	BYTE bB;

	m_dwUnk74 = nBase;

	// The top half of the column fades the colour out linearly with the
	// distance from the bottom of the target; note the bound is the
	// height halved on the spot while the divisor is the stored half.
	for (i = 0; i < (int)m_dwUnk8C / 2; i++)
	{
		int nDiv = (int)m_dwUnk2C;
		int nDist = (int)m_dwUnk8C - i;
		bR = (BYTE)((nDist * bRed) / nDiv);
		bG = (BYTE)((nDist * bGreen) / nDiv);
		bB = (BYTE)((nDist * bBlue) / nDiv);
		m_pUnk6C[i] = (DWORD)::GetNearestPaletteIndex(
			(HPALETTE)pPalette->m_hObject,
			(COLORREF)((DWORD)bR + ((DWORD)bG << 8) + ((DWORD)bB << 16)));
		// The palette index is replicated into all four bytes in place.
		m_pUnk6C[i] |= m_pUnk6C[i] << 8;
		m_pUnk6C[i] |= m_pUnk6C[i] << 16;
	}

	// The bottom half runs the same ramp back up from an accumulator.
	int nAccB = bBlue * ((int)m_dwUnk8C / 2);
	int nAccG = bGreen * ((int)m_dwUnk8C / 2);
	int nAccR = bRed * ((int)m_dwUnk8C / 2);
	for (i = (int)m_dwUnk8C / 2; i < (int)m_dwUnk8C; i++)
	{
		bR = (BYTE)(nAccR / (int)m_dwUnk2C);
		bG = (BYTE)(nAccG / (int)m_dwUnk2C);
		bB = (BYTE)(nAccB / (int)m_dwUnk2C);
		m_pUnk6C[i] = (DWORD)::GetNearestPaletteIndex(
			(HPALETTE)pPalette->m_hObject,
			(COLORREF)((DWORD)bR + ((DWORD)bG << 8) + ((DWORD)bB << 16)));
		m_pUnk6C[i] |= m_pUnk6C[i] << 8;
		m_pUnk6C[i] |= m_pUnk6C[i] << 16;
		nAccG += bGreen;
		nAccR += bRed;
		nAccB += bBlue;
	}
}

// The 8-byte sine/cosine table entries allocated at +0x64. They are built
// with `new SINCOSENTRY[n]`, so the original carries the compiler's vector-
// new expansion (null check, backwards constructor loop) and an empty
// __thiscall constructor of its own at hover.exe:0x419B20.

struct SINCOSENTRY
{
	SINCOSENTRY();

	int nCos;
	int nSin;
};

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
void CSub410F20::Sub_419530(CSub406090* pTarget, CPalette* pPal, int nA,
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
		// A two-dword struct copy, which is what keeps the original's
		// re-read of the pair out of the earlier width load's CSE.
		*(SIZE*)&nWidth = *(SIZE*)pnDim;
	}

	::operator delete(m_pUnk34);
	::operator delete(m_pUnk38);
	::operator delete(m_pUnk30);
	::operator delete(m_pUnk64);
	::operator delete(m_pUnk6C);
	::operator delete(m_pUnk5C);

	m_dwUnk88 = (DWORD)nWidth;
	m_dwUnk8C = (DWORD)nHeight;
	m_dwUnk20 = (DWORD)nWidth;
	m_dwUnk24 = (DWORD)nHeight;
	m_dwUnk18 = (DWORD)nA;
	m_dwUnk1C = (DWORD)nB;
	m_dwUnkA8 = (DWORD)-5000;

	int nTargetPitch = pnDim[0];
	m_dwUnk78 = (DWORD)nTargetPitch;
	m_dwUnk7C = (DWORD)(nTargetPitch * (int)pTarget->m_dwUnk1C);
	m_dwUnk84 = pTarget->m_dwHeight;
	m_dwUnk2C = (DWORD)(nHeight / 2);

	m_pUnk5C = (DWORD*)::operator new(0x1FA4);
	int nCurrent = -((int)m_dwUnk2C << 11);
	int nStep = (int)m_dwUnk2C * 2;
	double dFactor = 1.0 / cos(30.0 * 3.14159265359 / 180.0);
	DWORD* pOut = m_pUnk5C;
	int nCount = 0x7E9;
	do
	{
		int nVal = (int)((double)nCurrent * dFactor);
		*pOut++ = (DWORD)abs(nVal);
		nCurrent += nStep;
	} while (--nCount != 0);

	// The scan-line origin: the target's own origin plus the window, with
	// the rows counted from the bottom when the target is flipped.
	if (pTarget->m_dwUnk1C == (DWORD)-1)
		m_dwUnk28 = (DWORD)(((int)m_dwUnk84 - (int)m_dwUnk1C - 1)
			* (int)m_dwUnk78 + (int)pTarget->m_dwUnk04 + (int)m_dwUnk18);
	else
		m_dwUnk28 = (DWORD)((int)m_dwUnk78 * (int)m_dwUnk1C
			+ (int)pTarget->m_dwUnk04 + (int)m_dwUnk18);

	m_pUnk34 = (DWORD*)::operator new(m_dwUnk88 * sizeof(DWORD));
	m_pUnk38 = (DWORD*)::operator new(m_dwUnk88 * sizeof(DWORD));
	if ((int)m_dwUnk88 > 0)
	{
		DWORD* pTop = m_pUnk34;
		DWORD dwBottom = m_dwUnk8C - 1;
		DWORD* pBottom = m_pUnk38;
		for (i = 0; i < (int)m_dwUnk88; i++)
			pBottom[i] = dwBottom;
		for (i = 0; i < (int)m_dwUnk88; i++)
			pTop[i] = 0;
	}

	m_pUnk30 = (DWORD*)::operator new(m_dwUnk88 * sizeof(DWORD));
	int nCols = (int)m_dwUnk88;
	double dHalf = (double)(int)m_dwUnk88 * -0.5;
	double dStep = 30.0 / ((double)(int)m_dwUnk88 * 0.5);
	DWORD* pCol = m_pUnk30;
	double dHeight = (double)(int)m_dwUnk8C;
	for (i = 0; i < nCols; i++)
	{
		double d = (2.0 - cos((((double)i + dHalf) * dStep)
			* (3.14159265359 / 180.0))) * dHeight;
		pCol[i] = (DWORD)(int)d;
	}

	// The entry table is allocated whatever the count works out to; only
	// the fill below is guarded.
	m_pUnk64 = (DWORD*)new SINCOSENTRY[(int)((double)(int)m_dwUnk88 * 7.0)];

	double dWidth = (double)(int)m_dwUnk88;
	double dCosCount = dWidth * 7.0;
	int j = 0;
	if (dCosCount > 0.0)
	{
		double dDenom = dWidth * 6.0;
		double dOffset = 3.14159265359 / 6.0;
		SINCOSENTRY* pEntry = (SINCOSENTRY*)m_pUnk64;
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

	m_pUnk6C = (DWORD*)::operator new(m_dwUnk8C * sizeof(DWORD));
	Sub_419960(1, 0xFF, 0xFF, 0xFF, pPal);
}

// hover.exe:0x410F20
// FUNCTION: HOVER 0x410f20
void CSub410F20::Sub_410F20(DWORD dwFlags, DWORD dwMode)
{
	// Declaration order picks the registers here: with dwB1 first VC puts
	// the two flags in edx/esi the wrong way round (rule 8).
	DWORD dwB2;
	DWORD dwB1;
	DWORD* pTable;

	dwB1 = dwFlags & 1;
	dwB2 = (dwFlags & 2) >> 1;
	m_dwUnk90 = dwB1;
	m_dwUnk80 = dwB2;
	m_dwUnk44 = dwMode;
	// NOTE: the stored values are function addresses in hover.exe (the
	// render/span dispatchers); the renderer cluster is not yet
	// decompiled, so the original addresses are stored as constants.
	switch (dwMode)
	{
	case 1:
		m_pfnUnk00 = (DWORD)Hover_sub_4299D0;
		m_pfnUnk04 = (DWORD)Hover_sub_40F710;
		m_pfnUnk0C = (DWORD)Hover_sub_434470;
		if (dwB2 != 0)
		{
			m_pfnUnk00 = (DWORD)Hover_sub_429A30;
			pTable = m_pUnk08;
			pTable[6] = (DWORD)Hover_sub_434750;
			pTable[7] = (DWORD)Hover_sub_4347B0;
			pTable[8] = (DWORD)Hover_sub_434810;
			pTable[9] = (DWORD)Hover_sub_434870;
			pTable[10] = (DWORD)Hover_sub_4348D0;
			pTable[11] = (DWORD)Hover_sub_434930;
			pTable[12] = (DWORD)Hover_sub_434990;
			pTable[13] = (DWORD)Hover_sub_4349F0;
			pTable[14] = (DWORD)Hover_sub_434A50;
			pTable[15] = (DWORD)Hover_sub_434AB0;
		}
		if (dwB1 != 0)
			m_pfnUnk00 = (DWORD)Hover_sub_429960;
		if (dwB1 != 0)
		{
			m_pfnUnk0C = (DWORD)Hover_sub_434500;
			if (dwB2 != 0)
			{
				m_pfnUnk00 = (DWORD)Hover_sub_40F6B0;
				pTable = m_pUnk08;
				pTable[6] = (DWORD)Hover_sub_434590;
				pTable[7] = (DWORD)Hover_sub_434600;
				pTable[8] = (DWORD)Hover_sub_434670;
				pTable[9] = (DWORD)Hover_sub_40FA90;
				pTable[10] = (DWORD)Hover_sub_40FB40;
				pTable[11] = (DWORD)Hover_sub_40FC70;
				pTable[12] = (DWORD)Hover_sub_40FDD0;
				pTable[13] = (DWORD)Hover_sub_40FE70;
				pTable[14] = (DWORD)Hover_sub_4101E0;
				pTable[15] = (DWORD)Hover_sub_4346E0;
			}
		}
		m_dwUnk40 = 1;
		m_dwUnk3C = 1;
		break;
	case 2:
		m_pfnUnk00 = (DWORD)Hover_sub_429A80;
		m_pfnUnk04 = (DWORD)Hover_sub_42A2D0;
		m_pfnUnk0C = (DWORD)Hover_sub_434B10;
		if (dwB2 != 0)
		{
			m_pfnUnk00 = (DWORD)Hover_sub_429B70;
			pTable = m_pUnk08;
			pTable[6] = (DWORD)Hover_sub_434EA0;
			pTable[7] = (DWORD)Hover_sub_434F30;
			pTable[8] = (DWORD)Hover_sub_434FC0;
			pTable[9] = (DWORD)Hover_sub_435050;
			pTable[10] = (DWORD)Hover_sub_4350E0;
			pTable[11] = (DWORD)Hover_sub_435170;
			pTable[12] = (DWORD)Hover_sub_435200;
			pTable[13] = (DWORD)Hover_sub_435290;
			pTable[14] = (DWORD)Hover_sub_435320;
			pTable[15] = (DWORD)Hover_sub_4353B0;
			m_pfnUnk04 = (DWORD)Hover_sub_40E600;
		}
		if (dwB1 != 0)
			m_pfnUnk00 = (DWORD)Hover_sub_429AF0;
		if (dwB1 != 0)
		{
			m_pfnUnk0C = (DWORD)Hover_sub_434BB0;
			if (dwB2 != 0)
			{
				m_pfnUnk00 = (DWORD)Hover_sub_40E5A0;
				pTable = m_pUnk08;
				pTable[6] = (DWORD)Hover_sub_434C60;
				pTable[7] = (DWORD)Hover_sub_434CF0;
				pTable[8] = (DWORD)Hover_sub_434D80;
				pTable[9] = (DWORD)Hover_sub_40F540;
				pTable[10] = (DWORD)Hover_sub_40F5D0;
				pTable[11] = (DWORD)Hover_sub_40E850;
				pTable[12] = (DWORD)Hover_sub_40E910;
				pTable[13] = (DWORD)Hover_sub_40EAC0;
				pTable[14] = (DWORD)Hover_sub_40EDD0;
				pTable[15] = (DWORD)Hover_sub_434E10;
			}
		}
		m_dwUnk40 = 1;
		m_dwUnk3C = 2;
		break;
	case 3:
		m_pfnUnk00 = (DWORD)Hover_sub_429BD0;
		m_pfnUnk04 = (DWORD)Hover_sub_42A330;
		m_pfnUnk0C = (DWORD)Hover_sub_435440;
		if (dwB2 != 0)
		{
			m_pfnUnk00 = (DWORD)Hover_sub_429CC0;
			pTable = m_pUnk08;
			pTable[6] = (DWORD)Hover_sub_435790;
			pTable[7] = (DWORD)Hover_sub_435820;
			pTable[8] = (DWORD)Hover_sub_4358B0;
			pTable[9] = (DWORD)Hover_sub_435940;
			pTable[10] = (DWORD)Hover_sub_4359D0;
			pTable[11] = (DWORD)Hover_sub_435A60;
			pTable[12] = (DWORD)Hover_sub_435AF0;
			pTable[13] = (DWORD)Hover_sub_435B80;
			pTable[14] = (DWORD)Hover_sub_435C10;
			pTable[15] = (DWORD)Hover_sub_435CA0;
			m_pfnUnk04 = (DWORD)Hover_sub_412F50;
		}
		if (dwB1 != 0)
			m_pfnUnk00 = (DWORD)Hover_sub_429C40;
		if (dwB1 != 0)
		{
			m_pfnUnk0C = (DWORD)Hover_sub_4354F0;
			if (dwB2 != 0)
			{
				m_pfnUnk00 = (DWORD)Hover_sub_412EF0;
				pTable = m_pUnk08;
				pTable[6] = (DWORD)Hover_sub_4355B0;
				pTable[7] = (DWORD)Hover_sub_435650;
				pTable[8] = (DWORD)Hover_sub_424FB0;
				pTable[9] = (DWORD)Hover_sub_413350;
				pTable[10] = (DWORD)Hover_sub_4133F0;
				pTable[11] = (DWORD)Hover_sub_413490;
				pTable[12] = (DWORD)Hover_sub_413530;
				pTable[13] = (DWORD)Hover_sub_4135D0;
				pTable[14] = (DWORD)Hover_sub_413670;
				pTable[15] = (DWORD)Hover_sub_4356F0;
			}
		}
		m_dwUnk40 = 1;
		m_dwUnk3C = 4;
		break;
	case 4:
		m_pfnUnk00 = (DWORD)Hover_sub_429D20;
		m_pfnUnk04 = (DWORD)Hover_sub_42A390;
		m_pfnUnk0C = (DWORD)Span2x2_Var;
		if (dwB2 != 0)
		{
			m_pfnUnk00 = (DWORD)Hover_sub_429E60;
			m_pfnUnk04 = (DWORD)Hover_sub_40DE20;
			pTable = m_pUnk08;
			pTable[6] = (DWORD)Span2x2_S6;
			pTable[7] = (DWORD)Span2x2_S7;
			pTable[8] = (DWORD)Span2x2_S8;
			pTable[9] = (DWORD)Span2x2_S9;
			pTable[10] = (DWORD)Span2x2_S10;
			pTable[11] = (DWORD)Span2x2_S11;
			pTable[12] = (DWORD)Span2x2_S12;
			pTable[13] = (DWORD)Span2x2_S13;
			pTable[14] = (DWORD)Span2x2_S14;
			pTable[15] = (DWORD)Span2x2_S15;
		}
		if (dwB1 != 0)
			m_pfnUnk00 = (DWORD)Hover_sub_429DC0;
		if (dwB1 != 0)
		{
			m_pfnUnk0C = (DWORD)Span2x2Lit_Var;
			if (dwB2 != 0)
			{
				m_pfnUnk00 = (DWORD)Hover_sub_40DDA0;
				pTable = m_pUnk08;
				pTable[6] = (DWORD)Span2x2Lit_S6;
				pTable[7] = (DWORD)Span2x2Lit_S7;
				pTable[8] = (DWORD)Hover_sub_414220;
				pTable[9] = (DWORD)Span2x2Lit_S9;
				pTable[10] = (DWORD)Hover_sub_40DF80;
				pTable[11] = (DWORD)Hover_sub_40E020;
				pTable[12] = (DWORD)Hover_sub_40E0C0;
				pTable[13] = (DWORD)Hover_sub_40E160;
				pTable[14] = (DWORD)Hover_sub_40E200;
				pTable[15] = (DWORD)Span2x2Lit_S15;
			}
		}
		m_dwUnk40 = 2;
		m_dwUnk3C = 2;
		break;
	case 5:
		m_pfnUnk00 = (DWORD)Hover_sub_429EE0;
		m_pfnUnk04 = (DWORD)Hover_sub_42A400;
		m_pfnUnk0C = (DWORD)Span4x2_Var;
		if (dwB2 != 0)
		{
			m_pfnUnk00 = (DWORD)Hover_sub_42A030;
			m_pfnUnk04 = (DWORD)Hover_sub_40D680;
			pTable = m_pUnk08;
			pTable[6] = (DWORD)Span4x2_S6;
			pTable[7] = (DWORD)Span4x2_S7;
			pTable[8] = (DWORD)Span4x2_S8;
			pTable[9] = (DWORD)Span4x2_S9;
			pTable[10] = (DWORD)Span4x2_S10;
			pTable[11] = (DWORD)Span4x2_S11;
			pTable[12] = (DWORD)Span4x2_S12;
			pTable[13] = (DWORD)Span4x2_S13;
			pTable[14] = (DWORD)Span4x2_S14;
			pTable[15] = (DWORD)Span4x2_S15;
		}
		if (dwB1 != 0)
			m_pfnUnk00 = (DWORD)Hover_sub_429F80;
		if (dwB1 != 0)
		{
			m_pfnUnk0C = (DWORD)Span4x2Lit_Var;
			if (dwB2 != 0)
			{
				m_pfnUnk00 = (DWORD)Hover_sub_40D600;
				pTable = m_pUnk08;
				pTable[6] = (DWORD)Span4x2Lit_S6;
				pTable[7] = (DWORD)Span4x2Lit_S7;
				pTable[8] = (DWORD)Span4x2Lit_S8;
				pTable[9] = (DWORD)Span4x2Lit_S9;
				pTable[10] = (DWORD)Hover_sub_40D900;
				pTable[11] = (DWORD)Hover_sub_40D9A0;
				pTable[12] = (DWORD)Hover_sub_40DA40;
				pTable[13] = (DWORD)Hover_sub_40DAE0;
				pTable[14] = (DWORD)Hover_sub_40DB80;
				pTable[15] = (DWORD)Span4x2Lit_S15;
			}
		}
		m_dwUnk40 = 2;
		m_dwUnk3C = 4;
		break;
	case 6:
		m_pfnUnk00 = (DWORD)Hover_sub_42A0B0;
		m_pfnUnk04 = (DWORD)Hover_sub_42A470;
		m_pfnUnk0C = (DWORD)Span4x4_Var;
		if (dwB2 != 0)
		{
			m_pfnUnk00 = (DWORD)Hover_sub_42A240;
			m_pfnUnk04 = (DWORD)Hover_sub_412FA0;
			pTable = m_pUnk08;
			pTable[6] = (DWORD)Span4x4_S6;
			pTable[7] = (DWORD)Span4x4_S7;
			pTable[8] = (DWORD)Span4x4_S8;
			pTable[9] = (DWORD)Span4x4_S9;
			pTable[10] = (DWORD)Span4x4_S10;
			pTable[11] = (DWORD)Span4x4_S11;
			pTable[12] = (DWORD)Span4x4_S12;
			pTable[13] = (DWORD)Span4x4_S13;
			pTable[14] = (DWORD)Span4x4_S14;
			pTable[15] = (DWORD)Span4x4_S15;
		}
		if (dwB1 != 0)
			m_pfnUnk00 = (DWORD)Hover_sub_42A170;
		if (dwB1 != 0)
		{
			m_pfnUnk0C = (DWORD)Span4x4Lit_Var;
			if (dwB2 != 0)
			{
				m_pfnUnk00 = (DWORD)Hover_sub_4114D0;
				pTable = m_pUnk08;
				pTable[6] = (DWORD)Span4x4Lit_S6;
				pTable[7] = (DWORD)Span4x4Lit_S7;
				pTable[8] = (DWORD)Span4x4Lit_S8;
				pTable[9] = (DWORD)Span4x4Lit_S9;
				pTable[10] = (DWORD)Span4x4Lit_S10;
				pTable[11] = (DWORD)Hover_sub_411940;
				pTable[12] = (DWORD)Hover_sub_411FE0;
				pTable[13] = (DWORD)Hover_sub_4120A0;
				pTable[14] = (DWORD)Hover_sub_412160;
				pTable[15] = (DWORD)Span4x4Lit_S15;
			}
		}
		m_dwUnk40 = 4;
		m_dwUnk3C = 4;
		break;
	}
}

/////////////////////////////////////////////////////////////////////////////
// Free helper functions

// hover.exe:0x413730
// FUNCTION: HOVER 0x413730
int Hover_sub_413730(DWORD dwColor, const BYTE* pPal)
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
		lDist = (long)g_awUnk_4C4DB0[255 + nB - pPal[0]] +
			(long)g_awUnk_4C4DB0[255 + nG - pPal[-1]] +
			(long)g_awUnk_4C4DB0[255 + nR - pPal[-2]];
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
void Hover_sub_40E730(long lA1, long lA2, long lA3, long lA4, long lA5,
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
CSub4235E0::CSub4235E0(CRobotPlayer* pParent)
	: CSub423670(pParent)
{
	m_dwUnk8C = 0;
}

// hover.exe:0x423710
// FUNCTION: HOVER 0x423710
CSub423710::CSub423710(CRobotPlayer* pParent)
	: CSub423670(pParent)
{
}

// hover.exe:0x4237A0
// FUNCTION: HOVER 0x4237a0
CSub4237A0::CSub4237A0(CRobotPlayer* pParent)
	: CSub423670(pParent)
{
}

// hover.exe:0x4238D0
// FUNCTION: HOVER 0x4238d0
CSub4238D0::CSub4238D0(CRobotPlayer* pParent)
	: CSub423670(pParent)
{
}

// hover.exe:0x423960
// FUNCTION: HOVER 0x423960
CSub423960::CSub423960(CRobotPlayer* pParent)
	: CSub423670(pParent)
{
}

// hover.exe:0x423A90
// FUNCTION: HOVER 0x423a90
CSub423A90::CSub423A90(CRobotPlayer* pParent)
	: CSub423670(pParent)
{
}

// hover.exe:0x423B80
// FUNCTION: HOVER 0x423b80
CSub423B80::CSub423B80(CRobotPlayer* pParent)
	: CSub423670(pParent)
{
}

/////////////////////////////////////////////////////////////////////////////
// CRobotPlayer

// hover.exe:0x4220A0
// FUNCTION: HOVER 0x4220a0
CRobotPlayer::CRobotPlayer(DWORD dwRobot)
	: m_obj0264(this), m_obj02F4(this), m_obj0388(this), m_obj0418(this),
		m_obj04A8(this), m_obj0534(this), m_obj05FC(this)
{
	m_wUnk0200 = 0;
	m_wUnk0202 = 0;
	m_dwUnk0084 = (DWORD)theApp.m_pUnk0150;
	m_dwUnk021C = dwRobot;
	memset(m_adwUnk0224, 0, sizeof(m_adwUnk0224));
	m_dwUnk024C = 0;
	Sub_414A80();
	m_dwUnk01FC = 0;
	m_pUnk0220 = &m_obj0264;
	m_dwUnk0218 = 0;
}


// hover.exe:0x40E710
// FUNCTION: HOVER 0x40e710
int CCollider::Sub_40E710(CGameObj* pObj)
{
	pObj->Sub_40A9B0();
	return 1;
}

// Pool allocator for the 0x70-byte renderer sprite objects: carves them
// out of g_dwUnk_4C5624-sized blocks (each block registered in
// g_listUnk_4A4748 and freed at shutdown by Hover_sub_42CA80). The free
// list is threaded through each object's +0x6C field, and the nSize
// argument is ignored, exactly as in the original.

// hover.exe:0x42CBC0
// FUNCTION: HOVER 0x42cbc0
void* CMerlinDynamic::operator new(size_t cb)
{
	BYTE* pBlock;
	DWORD dwCount;
	int i;
	BYTE** ppLink;

	pBlock = (BYTE*)g_dwUnk_4C5620;
	if (pBlock == NULL)
	{
		g_dwUnk_4C5620 = (DWORD)::operator new(0x70 * g_dwUnk_4C5624);
		g_listUnk_4A4748.AddTail((void*)g_dwUnk_4C5620);
		dwCount = g_dwUnk_4C5624;
		pBlock = (BYTE*)g_dwUnk_4C5620;
		i = (int)dwCount - 1;
		if (i > 0)
		{
			ppLink = (BYTE**)(pBlock + 0x6C);
			do
			{
				ppLink = (BYTE**)((BYTE*)ppLink + 0x70);
				i--;
				*(BYTE**)((BYTE*)ppLink - 0x70) = (BYTE*)ppLink - 0x70 + 4;
			} while (i != 0);
		}
		*(DWORD*)(pBlock + dwCount * 0x70 - 4) = 0;
	}
	g_dwUnk_4C5620 = *(DWORD*)(pBlock + 0x6C);
	return pBlock;
}

/////////////////////////////////////////////////////////////////////////////
// CRobotPlayer AI component virtuals
//
// The seven components are the states of the robot's AI state machine.
// CRobotPlayer::m_pUnk0220 points at the active one; CRobotPlayer::
// Sub_42AD30 runs its Sub_42AD30 (vtable slot 7) once per frame, and the
// states hand over to each other through CSub423670::Sub_41BC10, which
// calls the old state's Sub_42ECF0 (slot 19, "leave") and the new one's
// Sub_42ECE0 (slot 18, "enter"). The tuning values come from the RobotAI
// parameter block theApp.m_pUnk0154 (g_RobotAI*Params).

// The class has both an out-of-line ??1 (hover.exe:0x423670, called by the
// embedded state objects' destructors in ~CRobotPlayer) and a scalar deleting
// ??_G (hover.exe:0x42F180); rule 69 says to mark the ??1, which is what this
// definition compiles to.
// hover.exe:0x423670
// FUNCTION: HOVER 0x423670
CSub423670::~CSub423670()
{
	// empty in the original (just the CGameObj destructor call)
}

// hover.exe:0x42ECD0
// FUNCTION: HOVER 0x42ecd0
void CSub423670::Sub_42AD30()
{
	// the base state does nothing per frame
}

// Answers the "does this state care about pObj" query the collision code
// asks through vtable slot 14 (pObj is the object the robot ran into): a
// CFlagRemover latches the m_dwUnk84 flag on.

// hover.exe:0x41BEB0
// FUNCTION: HOVER 0x41beb0
int CSub423670::Sub_42AD90(CGameObj* pObj)
{
	if (((CObject*)pObj)->IsKindOf(RUNTIME_CLASS(CFlagRemover)))
		m_dwUnk84 = 1;
	return (int)m_dwUnk84;
}

// hover.exe:0x42ECE0
// FUNCTION: HOVER 0x42ece0
void CSub423670::Sub_42ECE0()
{
	// empty in the original
}

// hover.exe:0x42ECF0
// FUNCTION: HOVER 0x42ecf0
void CSub423670::Sub_42ECF0()
{
	// empty in the original
}

// hover.exe:0x41BC10
// FUNCTION: HOVER 0x41bc10
void CSub423670::Sub_41BC10(CSub423670* pNext)
{
	m_pUnk88->m_pUnk0220->Sub_42ECF0();
	m_pUnk88->m_pUnk0220 = pNext;
	pNext->Sub_42ECE0();
}

// Nonzero when the collider reports a clear path between the human
// player and (dwX, dwY): the robots use it to decide whether a candidate
// beacon can see the player. dwA/dwB bracket the height band tested.

// hover.exe:0x42ED00
// FUNCTION: HOVER 0x42ed00
int CSub423670::Sub_42ED00(DWORD dwX, DWORD dwY, DWORD dwA, DWORD dwB)
{
	CHumanPlayer* pHuman;
	CCollider* pCollider;
	DWORD* pdwPos;
	DWORD adwPos[2];

	pHuman = &theApp.m_pBumperDoc->m_humanPlayer;
	pCollider = (CCollider*)theApp.m_pBumperDoc->m_dwUnk8388;
	if ((long)dwB < (long)pHuman->m_dwUnk01A4 ||
		(long)pHuman->m_dwUnk01A4 + 0x40 < (long)dwA)
		return 0;
	if (pHuman->m_dwUnk0134 != 0 &&
		((CGameObj*)pHuman->m_dwUnk0134)->m_dwUnk28 != 0)
		return 0;
	// The segment runs from the caller's point to the human's, not the
	// other way round (the original pushes dwX/dwY as arguments 1 and 2).
	pdwPos = pHuman->Sub_403880(adwPos);
	return pCollider->Sub_40C2B0(dwX, dwY, pdwPos[0], pdwPos[1], dwA, dwB,
		(int)((DWORD*)theApp.m_pUnk0154)[5]);
}

// The same "clear path" test as Sub_42ED00, but taken from (dwX, dwY) --
// the robot's own position -- against every flag still lying in the maze:
// the first flag whose height band overlaps [dwA, dwB] and to which the
// collider reports a clear line comes back in *pdwTarget. The line is
// then registered as a temporary wall (Sub_411600), which calls back into
// this state object's Sub_42AD90 and sets m_dwUnk84 when something is in
// the way after all; only when that flag stays clear is the flag
// accepted.

// hover.exe:0x40BA10
// FUNCTION: HOVER 0x40ba10
int CSub423670::Sub_40BA10(DWORD dwX, DWORD dwY, DWORD dwA, DWORD dwB,
	DWORD* pdwTarget)
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

	*pdwTarget = 0;
	dwLimA = dwA;
	dwLimB = dwB;
	pCollider = (CCollider*)theApp.m_pBumperDoc->m_dwUnk8388;
	pos = theApp.m_pBumperDoc->m_list838C.GetHeadPosition();
	while (pos != NULL)
	{
		pFlag = (CPodObj*)theApp.m_pBumperDoc->m_list838C.GetNext(pos);
		if (!pFlag->IsKindOf(RUNTIME_CLASS(CFlag)))
			continue;
		if (pFlag->m_dwUnkCC != 0)
			continue;
		if (pFlag->m_dwUnk94 == 0)
			continue;
		lBase = (long)pFlag->m_dwUnkAC + (long)pFlag->m_dwUnkA4;
		if ((long)pFlag->m_dwUnkB8 + lBase < (long)dwLimA)
			continue;
		if ((long)dwLimB < lBase)
			continue;
		pdwAI = (DWORD*)theApp.m_pUnk0154;
		pdwPos = pFlag->Sub_403880(adwPos);
		if (pCollider->Sub_40C2B0(dwX, dwY, pdwPos[0], pdwPos[1], dwLimA,
			dwLimB, (int)pdwAI[6]) == 0)
			continue;
		m_dwUnk84 = 0;
		pdwAI = (DWORD*)theApp.m_pUnk0154;
		pdwPos = pFlag->Sub_403880(adwPos2);
		pCollider->Sub_411600(dwX, dwY, pdwPos[0], pdwPos[1],
			(int)pdwAI[6], (CBeacon*)this);
		if (m_dwUnk84 != 0)
			continue;
		*pdwTarget = (DWORD)pFlag;
		return 1;
	}
	return 0;
}

// The flag robots' "can I see the human player" pass: locks onto the
// player object embedded in the document and switches to CSub423B80.

// hover.exe:0x40A790
// FUNCTION: HOVER 0x40a790
int CSub423670::Sub_40A790()
{
	CSub423670* pNext;
	CRobotPlayer* pParent;
	DWORD* pdwPos;
	DWORD adwPos[2];

	// Both failure paths share the one `return 0` the original has.
	pParent = m_pUnk88;
	if (pParent->m_dwUnk021C != 0)
	{
		pdwPos = pParent->Sub_403880(adwPos);
		if (Sub_42ED00(pdwPos[0], pdwPos[1], pParent->m_dwUnk01A4,
			pParent->m_dwUnk01A4 + 0x40) != 0)
		{
			if (m_pUnk88->m_dwUnk01FC != 0 &&
				((CObject*)m_pUnk88->m_dwUnk01FC)->IsKindOf(
					RUNTIME_CLASS(CBeacon)))
			{
				((CBeacon*)m_pUnk88->m_dwUnk01FC)->m_dwUnkA8 = 0;
			}
			pParent = m_pUnk88;
			pParent->m_dwUnk01FC =
				(DWORD)&theApp.m_pBumperDoc->m_humanPlayer;
			pNext = &pParent->m_obj05FC;
			pParent->m_pUnk0220->Sub_42ECF0();
			m_pUnk88->m_pUnk0220 = pNext;
			pNext->Sub_42ECE0();
			return 1;
		}
	}
	return 0;
}

// The same pass for the non-flag robots, using the line-of-sight test
// taken from the robot's own position; hands over to CSub4238D0.

// hover.exe:0x40BB40
// FUNCTION: HOVER 0x40bb40
int CSub423670::Sub_40BB40()
{
	CSub423670* pNext;
	CRobotPlayer* pParent;
	DWORD adwPos[2];
	DWORD dwTarget;

	// Both failure paths share the one `return 0` the original has.
	pParent = m_pUnk88;
	if (pParent->m_dwUnk021C == 0)
	{
		pParent->Sub_403880(adwPos);
		dwTarget = 0;
		if (Sub_40BA10(adwPos[0], adwPos[1], m_pUnk88->m_dwUnk01A4,
			m_pUnk88->m_dwUnk01A4 + 0x40, &dwTarget) != 0)
		{
			if (m_pUnk88->m_dwUnk01FC != 0 &&
				((CObject*)m_pUnk88->m_dwUnk01FC)->IsKindOf(
					RUNTIME_CLASS(CBeacon)))
			{
				((CBeacon*)m_pUnk88->m_dwUnk01FC)->m_dwUnkA8 = 0;
			}
			pParent = m_pUnk88;
			pParent->m_dwUnk01FC = dwTarget;
			pNext = &pParent->m_obj0418;
			pParent->m_pUnk0220->Sub_42ECF0();
			m_pUnk88->m_pUnk0220 = pNext;
			pNext->Sub_42ECE0();
			return 1;
		}
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////
// CSub4235E0 - target acquisition (the robot's initial and fallback state)

// Retires the "still moving" flag when the robot has slowed below the
// parameter block's threshold, gives the two dedicated states the first
// chance to take over, then scans the tracked-object array for the
// closest beacon that is free, within the robot's height band and has an
// unobstructed line to it. Registering that segment with the collider
// (Sub_411600, with this component as the hit callback) is what sets
// m_dwUnk84 when something is in the way.

// hover.exe:0x414D70
// FUNCTION: HOVER 0x414d70
void CSub4235E0::Sub_42AD30()
{
	CSub423670* pNext;
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

	pParent = m_pUnk88;
	if (pParent->m_dwUnk0214 != 0)
	{
		dwSpeedX = *(DWORD*)&pParent->m_awUnk01AC[0];
		HOVER_FIXMULP(dwSpeedX, dwSpeedX);
		dwSpeedY = *(DWORD*)&pParent->m_awUnk01AC[2];
		HOVER_FIXMULP(dwSpeedY, dwSpeedY);
		// 16.16 square root: the fractional part comes from the sum but
		// the integer part from the first term only, exactly as in the
		// original, which reads it as a signed short straight out of the
		// sum's stack slot (rule 46).
		fxSum.dw = dwSpeedX;
		dwLow = dwSpeedX + dwSpeedY;
		dwDist = (DWORD)(long)(sqrt((double)(DWORD)LOWORD(dwLow)
			* 1.52587890625e-05 + (long)fxSum.s.sInt) * 65536.0);
		if ((long)dwDist < (long)*(DWORD*)&pParent->m_awUnk01D0[0])
			pParent->m_dwUnk0214 = 0;
		if (pParent->m_dwUnk0214 != 0)
			return;
	}

	pCollider = (CCollider*)theApp.m_pBumperDoc->m_dwUnk8388;
	pParent->Sub_403880(adwSelf);
	if (Sub_40A790() != 0)
		return;
	if (Sub_40BB40() != 0)
		return;

	nBeacons = g_obArrUnk_4A4840.GetSize();

	// Release the beacon the robot was heading for before looking for a
	// new one.
	if (m_pUnk88->m_dwUnk01FC != 0 &&
		((CObject*)m_pUnk88->m_dwUnk01FC)->IsKindOf(RUNTIME_CLASS(CBeacon)))
	{
		((CBeacon*)m_pUnk88->m_dwUnk01FC)->m_dwUnkA8 = 0;
	}

	for (nLeft = nBeacons, i = 0; nLeft > 0; i++, nLeft--)
	{
		pBeacon = (CBeacon*)g_obArrUnk_4A4840.GetAt(i);
		if (pBeacon->m_dwUnkA8 != 0)
			continue;
		lHeight = (long)m_pUnk88->m_dwUnk01A4;
		lTexY = (long)(short)((CMerlinLocation*)pBeacon->m_pUnk98)->m_wUnk1A;
		if (lHeight - 0x4B > lTexY)
			continue;
		if (lHeight + 0x72 < lTexY)
			continue;
		pBeacon->Sub_403880(adwPos);
		dwDist = Hover_sub_408A00((long)adwSelf[0], (long)adwSelf[1],
			(long)adwPos[0], (long)adwPos[1]);
		// dwBest is not initialized by the original either: it is only
		// consulted once the robot already has a target.
		if (m_pUnk88->m_dwUnk01FC != 0 && (long)dwBest <= (long)dwDist)
			continue;
		pAI = (DWORD*)theApp.m_pUnk0154;
		if (pCollider->Sub_40C2B0(adwSelf[0], adwSelf[1], adwPos[0],
			adwPos[1], m_pUnk88->m_dwUnk01A4,
			m_pUnk88->m_dwUnk01A4 + 0x40, (int)pAI[7]) == 0)
			continue;
		m_dwUnk8C = 1;
		m_dwUnk84 = 0;
		pAI = (DWORD*)theApp.m_pUnk0154;
		pCollider->Sub_411600(adwSelf[0], adwSelf[1], adwPos[0], adwPos[1],
			(int)pAI[7], (CBeacon*)this);
		m_dwUnk8C = 0;
		if (m_dwUnk84 != 0)
			continue;
		dwBest = dwDist;
		m_pUnk88->m_dwUnk01FC = (DWORD)pBeacon;
		m_pUnk88->Sub_40BC10(pBeacon);
	}

	if (m_pUnk88->m_dwUnk01FC != 0)
	{
		((CBeacon*)m_pUnk88->m_dwUnk01FC)->m_dwUnkA8 = 1;
		pNext = &m_pUnk88->m_obj0388;
		m_pUnk88->m_pUnk0220->Sub_42ECF0();
		m_pUnk88->m_pUnk0220 = pNext;
		pNext->Sub_42ECE0();
	}
}

// Latches m_dwUnk84 when the object that hit the robot is a CHoldPad (on
// a flag robot) or a CFlagRemover (on a normal robot).

// hover.exe:0x41BF20
// FUNCTION: HOVER 0x41bf20
int CSub4235E0::Sub_42AD90(CGameObj* pObj)
{
	if ((m_pUnk88->m_dwUnk021C != 0 &&
			((CObject*)pObj)->IsKindOf(RUNTIME_CLASS(CHoldPad))) ||
		(m_pUnk88->m_dwUnk021C == 0 &&
			((CObject*)pObj)->IsKindOf(RUNTIME_CLASS(CFlagRemover))))
	{
		m_dwUnk84 = 1;
	}
	return (int)m_dwUnk84;
}

// hover.exe:0x41BF80
// FUNCTION: HOVER 0x41bf80
void CSub4235E0::Sub_42ECE0()
{
	CRobotPlayer* pParent;

	pParent = m_pUnk88;
	if (pParent->m_dwUnk01FC != 0 &&
		((CObject*)pParent->m_dwUnk01FC)->IsKindOf(RUNTIME_CLASS(CBeacon)))
	{
		((CBeacon*)m_pUnk88->m_dwUnk01FC)->m_dwUnkA8 = 0;
	}
	pParent = m_pUnk88;
	pParent->m_dwUnk01FC = 0;
	pParent->m_dwUnk0204 = 0;
	pParent->m_dwUnk0208 = 0;
	pParent->m_dwUnk020C = 0;
	pParent->m_dwUnk0210 = 0;
	*(DWORD*)&pParent->m_wUnk0200 = *(DWORD*)&pParent->m_awUnk0194[4];
	pParent->m_dwUnk0214 = 1;
	*(DWORD*)&pParent->m_awUnk01D0[2] =
		(DWORD)(*(long*)&pParent->m_awUnk01D0[4] / 3);
}

// hover.exe:0x415040
// FUNCTION: HOVER 0x415040
void CSub4235E0::Sub_42ECF0()
{
	CRobotPlayer* pParent;

	pParent = m_pUnk88;
	pParent->m_dwUnk0214 = 0;
	*(DWORD*)&pParent->m_awUnk01D0[2] = *(DWORD*)&pParent->m_awUnk01D0[4];
}

/////////////////////////////////////////////////////////////////////////////
// CSub423710 - the post-collision spin/recover state

// hover.exe:0x42ED90
// FUNCTION: HOVER 0x42ed90
void CSub423710::Sub_42AD30()
{
	CSub423670* pNext;
	CRobotPlayer* pParent;
	DWORD* pdwStep;
	DWORD dwStep;

	if ((long)m_dwUnk8C-- <= 0)
	{
		// the state transition is inlined by the original here
		pNext = &m_pUnk88->m_obj0264;
		m_pUnk88->m_pUnk0220->Sub_42ECF0();
		m_pUnk88->m_pUnk0220 = pNext;
		pNext->Sub_42ECE0();
	}
	// The turn step is the AI parameter block's first field, negated
	// through a stack temporary the original reaches by pointer.
	pParent = m_pUnk88;
	pdwStep = (DWORD*)theApp.m_pUnk0154;
	if (pParent->m_dwUnk020C == 0)
	{
		dwStep = (DWORD)(-(long)pdwStep[0]);
		pdwStep = &dwStep;
	}
	*(DWORD*)&pParent->m_wUnk0200 += *pdwStep;
}

// hover.exe:0x42EE10
// FUNCTION: HOVER 0x42ee10
void CSub423710::Sub_42ECE0()
{
	CRobotPlayer* pParent;
	DWORD dwSpeed;
	long lFrom;
	long lTo;

	pParent = m_pUnk88;
	if (pParent->m_dwUnk01FC != 0 &&
		((CObject*)pParent->m_dwUnk01FC)->IsKindOf(RUNTIME_CLASS(CBeacon)))
	{
		((CBeacon*)m_pUnk88->m_dwUnk01FC)->m_dwUnkA8 = 0;
	}
	pParent = m_pUnk88;
	// Both turn arguments are read before the two stores, as the
	// original has them.
	lTo = (long)(short)pParent->m_wUnk0202;
	lFrom = (long)(short)pParent->m_awUnk0194[5];
	pParent->m_dwUnk01FC = 0;
	pParent->m_dwUnk0208 = 1;
	pParent->m_dwUnk020C = (DWORD)(Hover_sub_41B830(lFrom, lTo) == 0);
	pParent = m_pUnk88;
	pParent->m_dwUnk0204 = 0;
	pParent->m_dwUnk0210 = 0;
	*(DWORD*)&pParent->m_wUnk0200 = *(DWORD*)&pParent->m_awUnk0194[4];
	dwSpeed = *(DWORD*)&pParent->m_awUnk01D0[4];
	HOVER_FIXMULP(dwSpeed, ((DWORD*)theApp.m_pUnk0154)[2]);
	*(DWORD*)&pParent->m_awUnk01D0[2] = dwSpeed;
	m_dwUnk8C = ((DWORD*)theApp.m_pUnk0154)[1];
}

// hover.exe:0x42EF00
// FUNCTION: HOVER 0x42ef00
void CSub423710::Sub_42ECF0()
{
	CRobotPlayer* pParent;

	pParent = m_pUnk88;
	*(DWORD*)&pParent->m_awUnk01D0[2] = *(DWORD*)&pParent->m_awUnk01D0[4];
	pParent->m_dwUnk0208 = 0;
	pParent->m_dwUnk020C = 0;
}

/////////////////////////////////////////////////////////////////////////////
// CSub4237A0 - chase the locked target

// hover.exe:0x40A860
// FUNCTION: HOVER 0x40a860
void CSub4237A0::Sub_42AD30()
{
	CSub423670* pNext;
	CRobotPlayer* pParent;
	DWORD adwSelf[2];
	DWORD adwTarget[2];
	DWORD dwSpeedX;
	DWORD dwSpeedY;
	FIXED1616 fxSum;
	DWORD dwLow;
	DWORD dwDist;

	if (Sub_40A790() != 0)
		return;
	m_pUnk88->Sub_403880(adwSelf);
	((CGameObj*)m_pUnk88->m_dwUnk01FC)->Sub_403880(adwTarget);
	dwDist = Hover_sub_408A00((long)adwSelf[0], (long)adwSelf[1],
		(long)adwTarget[0], (long)adwTarget[1]);
	pParent = m_pUnk88;
	pParent->m_dwUnk0214 = (DWORD)((long)dwDist < 0x32);
	if ((long)dwDist >= 0x32)
		return;

	// Close enough: hand over once the robot has also slowed down.
	dwSpeedX = *(DWORD*)&pParent->m_awUnk01AC[0];
	HOVER_FIXMULP(dwSpeedX, dwSpeedX);
	dwSpeedY = *(DWORD*)&pParent->m_awUnk01AC[2];
	HOVER_FIXMULP(dwSpeedY, dwSpeedY);
	fxSum.dw = dwSpeedX;
	dwLow = dwSpeedX + dwSpeedY;
	if ((long)(DWORD)(long)(sqrt((double)(DWORD)LOWORD(dwLow) *
		1.52587890625e-05 + (long)fxSum.s.sInt) * 65536.0) >= 0x10000)
		return;
	if (pParent->m_dwUnk021C != 0)
		pNext = &pParent->m_obj0534;
	else
		pNext = &pParent->m_obj04A8;
	m_pUnk88->m_pUnk0220->Sub_42ECF0();
	m_pUnk88->m_pUnk0220 = pNext;
	pNext->Sub_42ECE0();
}

// hover.exe:0x40BC50
// FUNCTION: HOVER 0x40bc50
void CSub4237A0::Sub_42ECE0()
{
	m_dwUnk8C = 0xC8;
	m_pUnk88->m_dwUnk0204 = 1;
}

// hover.exe:0x40BC70
// FUNCTION: HOVER 0x40bc70
void CSub4237A0::Sub_42ECF0()
{
	CRobotPlayer* pParent;

	pParent = m_pUnk88;
	pParent->m_dwUnk0204 = 0;
	pParent->m_dwUnk0218 = 0;
	pParent->m_dwUnk0214 = 0;
}

/////////////////////////////////////////////////////////////////////////////
// CSub4238D0 - hold the line of sight to the locked target

// hover.exe:0x4150F0
// FUNCTION: HOVER 0x4150f0
void CSub4238D0::Sub_42AD30()
{
	CSub423670* pNext;
	CRobotPlayer* pParent;
	CCollider* pCollider;
	DWORD* pAI;
	DWORD adwSelf[2];
	DWORD adwTarget[2];

	if ((long)m_dwUnk8C-- < 0)
	{
		pNext = &m_pUnk88->m_obj0264;
		m_pUnk88->m_pUnk0220->Sub_42ECF0();
		m_pUnk88->m_pUnk0220 = pNext;
		pNext->Sub_42ECE0();
		return;
	}
	m_pUnk88->Sub_403880(adwSelf);
	((CGameObj*)m_pUnk88->m_dwUnk01FC)->Sub_403880(adwTarget);
	pParent = m_pUnk88;
	pAI = (DWORD*)theApp.m_pUnk0154;
	pCollider = (CCollider*)theApp.m_pBumperDoc->m_dwUnk8388;
	if (pCollider->Sub_40C2B0(adwSelf[0], adwSelf[1], adwTarget[0],
		adwTarget[1], pParent->m_dwUnk01A4, pParent->m_dwUnk01A4 + 0x40,
		(int)pAI[6]) == 0)
	{
		pParent = m_pUnk88;
		pParent->m_dwUnk01FC = 0;
		pNext = &pParent->m_obj0264;
		pParent->m_pUnk0220->Sub_42ECF0();
		m_pUnk88->m_pUnk0220 = pNext;
		pNext->Sub_42ECE0();
	}
}

// hover.exe:0x41C010
// FUNCTION: HOVER 0x41c010
void CSub4238D0::Sub_42ECE0()
{
	m_dwUnk8C = 0xC8;
}

/////////////////////////////////////////////////////////////////////////////
// CSub423960 - keep chasing the human player, or fall back to the search

// hover.exe:0x40BC90
// FUNCTION: HOVER 0x40bc90
void CSub423960::Sub_42AD30()
{
	CSub423670* pNext;
	if (Sub_40BB40() == 0)
	{
		pNext = &m_pUnk88->m_obj0534;
		m_pUnk88->m_pUnk0220->Sub_42ECF0();
		m_pUnk88->m_pUnk0220 = pNext;
		pNext->Sub_42ECE0();
	}
}

// hover.exe:0x42EF30
// FUNCTION: HOVER 0x42ef30
void CSub423960::Sub_42ECE0()
{
	// empty in the original
}

/////////////////////////////////////////////////////////////////////////////
// CSub423A90 - beacon-graph search towards the human player

// Pops the next unvisited beacon off the search stack. Each stack frame
// is a {beacon, child index} pair; the walk descends into the beacon's
// linked-beacon array (CBeacon::m_arr84) and skips beacons that are
// already on the stack. Returns NULL when the stack runs dry.

// hover.exe:0x40BEF0
// FUNCTION: HOVER 0x40bef0
CBeacon* CSub424130::Sub_40BEF0()
{
	CBeacon* pBeacon;
	DWORD* pFrame;
	DWORD* pScan;
	int nIndex;
	int nNext;
	int nDepth;
	int bNew;
	int i;

	do
	{
		pFrame = m_pUnk0C;
		pBeacon = (CBeacon*)pFrame[0];
		nIndex = (int)pFrame[1];
		if (pBeacon->m_arr84.GetSize() > nIndex)
		{
			nNext = nIndex + 1;
			pBeacon = (CBeacon*)pBeacon->m_arr84[nIndex];
			pFrame[1] = (DWORD)nNext;
			// The scan stops as soon as the beacon is already on the
			// walk stack: bNew stays set only while the entries differ.
			bNew = 1;
			i = 0;
			pScan = m_adwUnk10;
			nDepth = (int)m_dwUnk08;
			while (i < nDepth)
			{
				bNew = (*pScan != (DWORD)pBeacon);
				pScan += 2;
				i++;
				if (bNew == 0)
					break;
			}
			if (bNew != 0)
			{
				if ((int)m_dwUnk04 == nDepth)
					return pBeacon;
				pFrame[2] = (DWORD)pBeacon;
				m_dwUnk08 = (DWORD)(nDepth + 1);
				pFrame += 2;
				m_pUnk0C = pFrame;
				pFrame[1] = 0;
			}
		}
		else
		{
			nDepth = (int)m_dwUnk08;
			m_dwUnk08 = (DWORD)(nDepth - 1);
			if (nDepth > 0)
				m_pUnk0C = pFrame - 2;
		}
	} while ((long)m_dwUnk08 >= 0);
	return NULL;
}

// Walks the beacon graph outwards from the current target until it finds
// a beacon the human player can be seen from, then chases it
// (CSub4237A0). Gives up after the parameter block's step budget.

// hover.exe:0x40BCD0
// FUNCTION: HOVER 0x40bcd0
void CSub423A90::Sub_42AD30()
{
	CSub423670* pNext;
	CRobotPlayer* pParent;
	CBeacon* pBeacon;
	CBeacon* pTarget;
	DWORD* pdwTaken;
	int nStep;

	if ((long)m_obj8C.m_dwUnk38 == -1)
	{
		Sub_40BFF0();
		return;
	}

	pBeacon = m_obj8C.Sub_40BEF0();
	while (pBeacon == NULL)
	{
		nStep = (int)m_obj8C.m_dwUnk38 + 1;
		m_obj8C.m_dwUnk38 = (DWORD)nStep;
		if ((long)((DWORD*)theApp.m_pUnk0154)[9] <= nStep)
		{
			Sub_40BFF0();
			return;
		}
		if (nStep < 5)
		{
			m_obj8C.m_adwUnk10[1] = 0;
			m_obj8C.m_dwUnk08 = 0;
			m_obj8C.m_dwUnk04 = (DWORD)nStep;
			m_obj8C.m_pUnk0C = m_obj8C.m_adwUnk10;
			m_obj8C.m_pUnk0C[0] = m_pUnk88->m_dwUnk01FC;
		}
	}

	// Note that only the flag test and the hand-over below follow the
	// hop substitution: the original keeps calling Sub_403880 on the
	// beacon the walk returned, not on the substituted one (its `this`
	// register is never reloaded before the call).
	pTarget = pBeacon;
	if (m_obj8C.m_dwUnk38 != 0)
		pTarget = (CBeacon*)m_obj8C.m_adwUnk10[2];
	// The original keeps the address of the beacon's "taken" flag in a
	// register across the whole test (rule 25).
	pdwTaken = &pTarget->m_dwUnkA8;
	if (*pdwTaken != 0)
		return;

	// The original spells the hand-over out in both arms rather than
	// joining them (the two copies are byte-identical in hover.exe).
	pParent = m_pUnk88;
	if (pParent->m_dwUnk021C != 0)
	{
		DWORD* pdwPos;
		DWORD adwPos[2];

		// Assigning the position pointer inside the first argument is
		// what puts the Sub_403880 call between the height pushes and
		// the coordinate pushes, exactly as the original has it.
		if (Sub_42ED00((pdwPos = pBeacon->Sub_403880(adwPos))[0],
			pdwPos[1], m_pUnk88->m_dwUnk01A4,
			m_pUnk88->m_dwUnk01A4 + 0x40) == 0)
			return;
		if (m_pUnk88->m_dwUnk01FC != 0 &&
			((CObject*)m_pUnk88->m_dwUnk01FC)->IsKindOf(
				RUNTIME_CLASS(CBeacon)))
		{
			((CBeacon*)m_pUnk88->m_dwUnk01FC)->m_dwUnkA8 = 0;
		}
		*pdwTaken = 1;
		m_pUnk88->m_dwUnk01FC = (DWORD)pTarget;
		pNext = &m_pUnk88->m_obj0388;
		m_pUnk88->m_pUnk0220->Sub_42ECF0();
		m_pUnk88->m_pUnk0220 = pNext;
		pNext->Sub_42ECE0();
		m_pUnk88->Sub_40BC10(pTarget);
	}
	else
	{
		DWORD* pdwPos;
		DWORD adwPos[2];
		DWORD dwTarget;

		// dwTarget is a pure out parameter; the original does not
		// initialize it either.
		if (Sub_40BA10((pdwPos = pBeacon->Sub_403880(adwPos))[0],
			pdwPos[1], m_pUnk88->m_dwUnk01A4,
			m_pUnk88->m_dwUnk01A4 + 0x40, &dwTarget) == 0)
			return;
		if (m_pUnk88->m_dwUnk01FC != 0 &&
			((CObject*)m_pUnk88->m_dwUnk01FC)->IsKindOf(
				RUNTIME_CLASS(CBeacon)))
		{
			((CBeacon*)m_pUnk88->m_dwUnk01FC)->m_dwUnkA8 = 0;
		}
		*pdwTaken = 1;
		m_pUnk88->m_dwUnk01FC = (DWORD)pTarget;
		pNext = &m_pUnk88->m_obj0388;
		m_pUnk88->m_pUnk0220->Sub_42ECF0();
		m_pUnk88->m_pUnk0220 = pNext;
		pNext->Sub_42ECE0();
		m_pUnk88->Sub_40BC10(pTarget);
	}
}

// Abandons the search: the next hop of the beacon graph becomes the new
// target, and the state machine goes back to CSub4235E0 (no target left)
// or CSub4237A0 (new target).

// hover.exe:0x40BFF0
// FUNCTION: HOVER 0x40bff0
void CSub423A90::Sub_40BFF0()
{
	CSub423670* pNext;
	CRobotPlayer* pParent;
	CBeacon* pTarget;

	if (m_pUnk88->m_dwUnk01FC != 0 &&
		((CObject*)m_pUnk88->m_dwUnk01FC)->IsKindOf(RUNTIME_CLASS(CBeacon)))
	{
		((CBeacon*)m_pUnk88->m_dwUnk01FC)->m_dwUnkA8 = 0;
	}
	pParent = m_pUnk88;
	pParent->m_dwUnk01FC =
		(DWORD)((CBeacon*)pParent->m_dwUnk01FC)->Sub_40C0E0(pParent);
	pParent = m_pUnk88;
	pTarget = (CBeacon*)pParent->m_dwUnk01FC;
	if (pTarget != NULL && pTarget->m_dwUnkA8 != 0)
		pParent->m_dwUnk01FC = 0;
	pTarget = (CBeacon*)pParent->m_dwUnk01FC;
	if (pTarget == NULL)
	{
		pNext = &pParent->m_obj0264;
		pParent->m_pUnk0220->Sub_42ECF0();
		m_pUnk88->m_pUnk0220 = pNext;
		pNext->Sub_42ECE0();
		return;
	}
	pTarget->m_dwUnkA8 = 1;
	pNext = &pParent->m_obj0388;
	pParent->m_pUnk0220->Sub_42ECF0();
	m_pUnk88->m_pUnk0220 = pNext;
	pNext->Sub_42ECE0();
	m_pUnk88->Sub_40BC10((CGameObj*)m_pUnk88->m_dwUnk01FC);
}

// hover.exe:0x40BF80
// FUNCTION: HOVER 0x40bf80
void CSub423A90::Sub_42ECE0()
{
	CRobotPlayer* pParent;
	DWORD* pdwStack;

	pParent = m_pUnk88;
	if (pParent->m_dwUnk01FC == 0 ||
		!((CObject*)pParent->m_dwUnk01FC)->IsKindOf(RUNTIME_CLASS(CBeacon)))
	{
		m_obj8C.m_dwUnk38 = 0xFFFFFFFF;
		return;
	}
	pdwStack = m_obj8C.m_adwUnk10;
	m_obj8C.m_dwUnk38 = 0;
	m_obj8C.m_dwUnk08 = 0;
	m_obj8C.m_dwUnk04 = 0;
	m_obj8C.m_pUnk0C = pdwStack;
	pdwStack[0] = m_pUnk88->m_dwUnk01FC;
	pdwStack[1] = 0;
}

/////////////////////////////////////////////////////////////////////////////
// CSub423B80 - keep the human player in sight while chasing him

// hover.exe:0x42EF40
// FUNCTION: HOVER 0x42ef40
void CSub423B80::Sub_42AD30()
{
	CSub423670* pNext;
	CRobotPlayer* pParent;
	CPlayer* pTarget;
	CCollider* pCollider;
	DWORD* pAI;
	DWORD adwSelf[2];
	DWORD adwTarget[2];

	m_pUnk88->Sub_403880(adwSelf);
	((CGameObj*)m_pUnk88->m_dwUnk01FC)->Sub_403880(adwTarget);
	pParent = m_pUnk88;
	pTarget = (CPlayer*)pParent->m_dwUnk01FC;
	if (pTarget->m_dwUnk0134 == 0 ||
		((CGameObj*)pTarget->m_dwUnk0134)->m_dwUnk28 == 0)
	{
		pAI = (DWORD*)theApp.m_pUnk0154;
		pCollider = (CCollider*)theApp.m_pBumperDoc->m_dwUnk8388;
		if (pCollider->Sub_40C2B0(adwSelf[0], adwSelf[1], adwTarget[0],
			adwTarget[1], pParent->m_dwUnk01A4,
			pParent->m_dwUnk01A4 + 0x40, (int)pAI[5]) != 0)
			return;
	}
	pParent = m_pUnk88;
	pParent->m_dwUnk01FC = 0;
	pNext = &pParent->m_obj0264;
	pParent->m_pUnk0220->Sub_42ECF0();
	m_pUnk88->m_pUnk0220 = pNext;
	pNext->Sub_42ECE0();
}

// hover.exe:0x42F000
// FUNCTION: HOVER 0x42f000
void CSub423B80::Sub_42ECE0()
{
	CRobotPlayer* pParent;

	pParent = m_pUnk88;
	pParent->m_dwUnk0204 = 0;
	pParent->m_dwUnk0208 = 0;
	pParent->m_dwUnk020C = 0;
	pParent->m_dwUnk0210 = (DWORD)((rand() >> 8) & 0xFF);
	if (theApp.m_dwUnk0408 != 0)
		Sub_40D6E0(0x1E, 6, 5, 0);
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
void CRobotPlayer::Sub_42AD30()
{
	if (m_dwUnk5C == 0)
	{
		m_dwUnk015C = 0;
		if (m_dwUnk0158 == 0)
			m_dwUnk015C = 0xFFFFFFFF;
		if (theApp.m_dwUnk01B8 == 3)
		{
			m_pUnk0220->Sub_42AD30();
			Sub_409350();
		}
	}
	CPlayer::Sub_42AD30();
}

// Collision handler: a CSledPad resets the AI to the acquisition state,
// and any collision that set m_dwUnk01CC toggles the robot between the
// acquisition state and the post-collision spin state.

// hover.exe:0x4144D0
// FUNCTION: HOVER 0x4144d0
void CRobotPlayer::Sub_42AD40(CPlayer* pOther, DWORD dwB)
{
	CPlayer::Sub_42AD40(pOther, dwB);
	if (((CObject*)pOther)->IsKindOf(RUNTIME_CLASS(CSledPad)))
		m_pUnk0220->Sub_41BC10(&m_obj0264);
	if (m_dwUnk01CC != 0)
	{
		if ((DWORD)m_pUnk0220 - (DWORD)this == 0x2F4)
			m_pUnk0220->Sub_41BC10(&m_obj0264);
		else
			m_pUnk0220->Sub_41BC10(&m_obj02F4);
	}
}

// hover.exe:0x42C3C0
// FUNCTION: HOVER 0x42c3c0
int CRobotPlayer::Sub_42AD90(CGameObj* pObj)
{
	return 1;
}

// Picks the sprite frame for the robot: the angle between the robot's
// heading and the direction to the human player selects one of the 32
// "DRONE_nn" textures, and frames past the half circle are drawn
// mirrored (m_dwUnk01EC).

// hover.exe:0x40A4C0
// FUNCTION: HOVER 0x40a4c0
void CRobotPlayer::Sub_4087D0()
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
	dwAngle = *(DWORD*)&m_wUnk48;
	lDY = (long)pHuman->Sub_4038A0(adwA)[1];
	lDY -= (long)dwAngle;
	dwAngle = *(DWORD*)&m_wUnk44;
	lDX = (long)pHuman->Sub_4038A0(adwB)[0];
	lDX -= (long)dwAngle;
	pdwAngle = Hover_sub_4386D0(&dwAngle, lDY, lDX);
	lFrame = ((((long)(short)m_awUnk0194[5] -
		(long)((short*)pdwAngle)[1]) & 0x1F8) << 6) >> 9;
	if (lFrame >= 0x20)
	{
		m_dwUnk01EC = 1;
		lFrame = 0x3F - lFrame;
	}
	else
	{
		m_dwUnk01EC = 0;
	}
	m_dwUnk64 = 1;
	m_dwUnk01E4 = (DWORD)g_apUnk_4A4620[lFrame];
	CPlayer::Sub_4087D0();
}

// Builds/updates the renderer sprite from the frame Sub_4087D0 picked:
// the sprite is created lazily out of the 0x70-byte sprite pool and
// registered with the world object, then its screen band, scale, world
// position and mirror flag are refreshed.

// hover.exe:0x40A590
// FUNCTION: HOVER 0x40a590
void CRobotPlayer::Sub_4087F0()
{
	CBumperDoc* pDoc;
	CMerlinDynamic* pSprite;
	CMerlinTexture* pTex;
	WORD wY;

	pDoc = theApp.m_pBumperDoc;
	if (m_dwUnk60 == 0)
	{
		if (m_dwUnk78 == 0 && m_dwUnk64 != 0)
		{
			// The original dereferences the sprite unconditionally
			// here, even when the pool allocation failed.
			m_dwUnk78 = (DWORD)new CMerlinDynamic;
			pSprite = (CMerlinDynamic*)m_dwUnk78;
			pSprite->m_dwUnk60 = 0;
			((CMerlinWorld*)pDoc->m_dwUnk0188)->Sub_4128A0((DWORD)pSprite);
		}
		pSprite = (CMerlinDynamic*)m_dwUnk78;
		if (pSprite != NULL)
		{
			pTex = (CMerlinTexture*)m_dwUnk01E8;
			wY = (WORD)(m_dwUnk01A8 + 0xD);
			pSprite->m_pUnk50 = pTex;
			pSprite->m_wUnk56 = wY;
			wY = (WORD)(wY - pTex->m_wUnk18);
			pSprite->m_wUnk54 = wY;
			if ((short)wY < 0)
				pSprite->m_wUnk54 = 0;
			pSprite->m_wUnk5C = (WORD)m_dwUnk50;
			pSprite->m_dwUnk68 = m_dwUnk01EC;
			pSprite->m_wUnk58 = (WORD)((long)(short)pTex->m_wUnk14 / 2);
			pSprite->m_wUnk5A = (WORD)m_dwUnk4C;
		}
	}
	CPlayer::Sub_4087F0();
}

// hover.exe:0x42C3B0
// FUNCTION: HOVER 0x42c3b0
void CRobotPlayer::Sub_413020()
{
	CPlayer::Sub_413020();
}

// Picks the next hop of the beacon graph for pParent: of the beacons this
// one is linked to, take a random one that pParent has not visited
// recently (CRobotPlayer::m_adwUnk0224). When every neighbour is on that
// ring buffer the walk is retried ignoring all of it but the most recent
// entry, so the robot at worst turns straight back. Returns NULL when the
// beacon has no links at all.

// hover.exe:0x40C0E0
// FUNCTION: HOVER 0x40c0e0
CGameObj* CBeacon::Sub_40C0E0(CRobotPlayer* pParent)
{
	CGameObj* apCandidates[20];
	CGameObj** ppOut;
	CGameObj* pLink;
	int nLinks;
	int nFound;
	int i;
	int nLast;

	nLinks = m_arr84.GetSize();
	if (nLinks == 0)
		return NULL;
	if (nLinks == 1)
		return (CGameObj*)m_arr84[0];

	nFound = 0;
	i = 0;
	while (i < nLinks)
	{
		ppOut = apCandidates + nFound;
		if (ppOut >= apCandidates + 20)
			break;
		pLink = (CGameObj*)m_arr84[i];
		if (pParent->Sub_40C1F0(pLink) == 0)
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
			pLink = (CGameObj*)m_arr84[i];
			nLast = (int)pParent->m_dwUnk024C;
			if (nLast == 0)
				nLast = 9;
			else
				nLast--;
			if (pParent->m_adwUnk0224[nLast] != (DWORD)pLink)
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
int CRobotPlayer::Sub_40C1F0(CGameObj* pObj)
{
	DWORD* pdw;
	int i;

	pdw = m_adwUnk0224;
	for (i = 0; i < 10; i++)
	{
		if (*pdw == (DWORD)pObj)
			return 1;
		pdw++;
	}
	return 0;
}


/////////////////////////////////////////////////////////////////////////////
// The robot steering pass (CRobotPlayer::Sub_409350)

// The steering pass below was compiled with the optimizer off, so its
// 16.16 fixed-point arithmetic stays real calls into CFixed (declared in
// hover.h). Sub_4038E0 is defined in bumpview.cpp, next to its other
// caller; only the two used solely from here are defined below.

// Copying `this` into a local is what makes VC 2.2 home it in the single
// stack slot the original reserves; the `_asm` body then reads the
// argument straight out of the caller's slot and the function returns the
// same pointer.

// hover.exe:0x40A490
// FUNCTION: HOVER 0x40a490
CFixed* CFixed::Sub_40A490(DWORD dwB)
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
CFixed* CFixed::Sub_40B4F0(short sB)
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
	((CFixed*)&dwA)->Sub_4038E0(((CFixed*)&dwB)->m_dwValue);
	return dwA;
}

// The same product where the result is handed to another expansion rather
// than stored: the original keeps it in a slot of its own there.
inline DWORD HoverFixMulR(DWORD dwA, DWORD dwB)
{
	DWORD dwR;

	((CFixed*)&dwA)->Sub_4038E0(((CFixed*)&dwB)->m_dwValue);
	dwR = dwA;
	return dwR;
}

inline DWORD HoverFixDiv(DWORD dwA, DWORD dwB)
{
	((CFixed*)&dwA)->Sub_40A490(((CFixed*)&dwB)->m_dwValue);
	return dwA;
}

inline DWORD HoverFixMulW(DWORD dwA, short sB)
{
	((CFixed*)&dwA)->Sub_40B4F0(sB);
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
	return HoverFixAddR(dwA, dwB);
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

	pdwR = Hover_sub_4386D0(pdwBuf, dwDY, dwDX);
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
	return (long)((DWORD*)theApp.m_pUnk0154)[3];
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
// CRobotPlayer::Sub_42AD30 while the game is in state 3. It publishes the
// two control values the physics step reads, m_wUnk01F0 (throttle) and
// m_wUnk01F4 (turn), both 16.16 and clamped to [-1, 1]:
//
//  - the heading to the current AI target (m_dwUnk01FC) goes into
//    m_wUnk0200, and, while the target is close enough that the robot
//    would overshoot it, m_dwUnk0218 latches the "drifting past it" state
//    so the robot steers by its own velocity vector instead;
//  - the heading error against m_dwUnk019C is normalized into
//    (-half turn, +half turn], reversed when it exceeds a quarter turn
//    (which makes the robot back up instead of turning round), and
//    divided by the robot's turn parameter to give the turn control;
//  - the throttle is the difference between the wanted speed (0 while
//    drifting, m_dwUnk01D4 otherwise) and the current speed, divided by
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
void CRobotPlayer::Sub_409350()
{
	struct ROBOTSTEER
	{
		DWORD dwTmp;			// ebp-0xa0 the ?: result
		DWORD dwNegSteer;		// ebp-0x9c
		DWORD dwSteerVal;		// ebp-0x98
		DWORD dwWantDelta;		// ebp-0x94
		DWORD dwAbs2;			// ebp-0x90
		DWORD dwThrottle;		// ebp-0x8c
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
	f.nLos = (int)m_dwUnk0214;
	f.dwSpeed = HoverFixSqrt(HoverFixSum(
		HoverFixMulR(*(DWORD*)&m_awUnk01AC[2], *(DWORD*)&m_awUnk01AC[2]),
		HoverFixMulR(*(DWORD*)&m_awUnk01AC[0], *(DWORD*)&m_awUnk01AC[0])));
	HoverFixZeroEq(&f.dwWanted);

	if (m_dwUnk01FC != 0 && f.nLos == 0)
	{
		((CGameObj*)m_dwUnk01FC)->Sub_4038A0(f.adwTarget);
		HoverPtSub(f.adwTarget, Sub_4038A0(f.adwSelf));
		*(DWORD*)&m_wUnk0200 = HoverFixValOf(*Hover_sub_4386D0(f.adwAngle,
			f.adwTarget[1], f.adwTarget[0]));
		if (m_dwUnk0204 != 0)
		{
			// The distance the robot needs to come to a stop:
			// v + (v*v / brake) / 2.
			f.dwSquare = HoverFixMul(f.dwSpeed, f.dwSpeed);
			f.dwBrake = HoverFixDiv(f.dwSquare, *(DWORD*)&m_awUnk01D0[0]);
			f.dwHalf = HoverFixHalf(f.dwBrake);
			f.dwStop = HoverFixAdd(f.dwHalf, f.dwSpeed);
			f.nRange = HoverFixWholeInt(f.dwStop);
			f.nDistTarget = (int)Hover_sub_408A00(
				HoverFixWholeInt(f.adwTarget[0]),
				HoverFixWholeInt(f.adwTarget[1]), 0, 0);
			if (f.nRange * 2 > f.nDistTarget)
			{
				HoverFixOfWord(&f.dwVelFix,
					((WORD*)HoverHeadingOf(f.adwAngle, *(DWORD*)&m_awUnk01AC[0],
					*(DWORD*)&m_awUnk01AC[2]))[1]);
				f.nVelAngle = HoverFixWholeInt(f.dwVelFix);
				HoverZeroPoint(f.adwAlong);
				HoverZeroPoint(f.adwSide);
				Hover_sub_40B6E0(f.adwTarget[0], f.adwTarget[1], f.nVelAngle,
					f.adwAlong, f.adwSide);
				if (m_dwUnk0218 != 0)
				{
					// Once the sideways component has flipped sign the
					// robot has passed its target.
					if (HoverFixIsNeg(f.adwSide[0]) !=
						HoverFixLt(0, *(DWORD*)&m_awUnk01AC[0]))
					{
						m_dwUnk0218 = 1;
						f.nLos = 1;
					}
				}
				if (f.nLos == 0)
				{
					f.nDistSide = (int)Hover_sub_408A00(
						HoverFixWholeInt(f.adwSide[0]),
						HoverFixWholeInt(f.adwSide[1]), 0, 0);
					if (f.nDistSide < f.nRange)
					{
						m_dwUnk0218 = 1;
						f.nLos = 1;
					}
				}
			}
			else
			{
				m_dwUnk0218 = 0;
				f.nLos = 0;
			}
		}
	}

	f.dwHeading = *(DWORD*)&m_wUnk0200;
	f.bReverse = 0;
	if (f.nLos != 0 || m_dwUnk0218 != 0)
	{
		// Drifting: steer by the velocity vector, and turn round when it
		// is more than a quarter turn away from the current heading.
		HoverFixOfWord(&f.dwVelFix2,
			((WORD*)HoverHeadingOf(f.adwAngle, *(DWORD*)&m_awUnk01AC[0],
			*(DWORD*)&m_awUnk01AC[2]))[1]);
		f.dwHeading = HoverFixVal(f.dwVelFix2);
		f.dwDelta = HoverFixSub(*(DWORD*)&m_awUnk0194[4], f.dwHeading);
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
		f.dwTmp = *(DWORD*)&m_awUnk01D0[2];
	}
	f.dwWanted = HoverFixVal(f.dwTmp);

	f.dwFacing = f.dwHeading;
	if (HoverFixLt((DWORD)HoverFixOfInt(1), f.dwSpeed))
	{
		HoverFixOfWord(&f.dwVelFix3,
			((WORD*)HoverHeadingOf(f.adwAngle, *(DWORD*)&m_awUnk01AC[0],
			*(DWORD*)&m_awUnk01AC[2]))[1]);
		f.dwFacing = HoverFixVal(f.dwVelFix3);
		if (m_dwUnk0208 != 0)
		{
			HoverFixAddWhole(&f.dwFacing, 0x100);
			HoverFixWrap(&f.dwFacing);
		}
	}
	else
	{
		m_dwUnk0218 = 0;
	}
	HoverFixSubEq(&f.dwFacing, *(CFixed*)&f.dwHeading);
	HoverFixWrap(&f.dwFacing);
	if (HoverFixLt((DWORD)HoverFixOfInt(0x100), f.dwFacing))
		HoverFixSubWhole(&f.dwFacing, 0x200);

	if (f.nLos == 0 && m_dwUnk0218 == 0)
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

	f.dwDelta2 = HoverFixSub(*(DWORD*)&m_awUnk0194[4], f.dwHeading);
	f.dwFacing = HoverFixVal(f.dwDelta2);
	HoverFixWrap(&f.dwFacing);
	if (HoverFixLt((DWORD)HoverFixOfInt(0x100), f.dwFacing))
		HoverFixSubWhole(&f.dwFacing, 0x200);
	f.dwNegFacing = HoverFixNeg(f.dwFacing);
	f.dwThrottle = HoverFixDiv(f.dwNegFacing, ((DWORD*)m_dwUnk0084)[10]);
	*(DWORD*)&m_wUnk01F0 = HoverFixVal(f.dwThrottle);
	if (HoverFixIsLt(*(DWORD*)&m_wUnk01F0, 0xFFFF0000))
	{
		m_wUnk01F2 = 0xFFFF;
		m_wUnk01F0 = 0;
	}
	if (HoverFixLt((DWORD)HoverFixOfInt(1), *(DWORD*)&m_wUnk01F0))
	{
		m_wUnk01F2 = 1;
		m_wUnk01F0 = 0;
	}

	// One `else` for the two ways of giving up, which is why the whole
	// guard is one condition in the original.
	if ((m_dwUnk0218 == 0 || f.nLos != 0)
		&& HoverFixIsGt(HoverFixOfInt(((short*)theApp.m_pUnk0154)[8]),
			(long)(f.dwAbs2 = HoverFixAbs(f.dwFacing))))
	{
		f.dwWantDelta = HoverFixSub(f.dwWanted, f.dwSpeed);
		f.dwSteerVal = HoverFixDiv(f.dwWantDelta, *(DWORD*)&m_awUnk01D0[0]);
		*(DWORD*)&m_wUnk01F4 = HoverFixVal(f.dwSteerVal);
		if (HoverFixIsLt(*(DWORD*)&m_wUnk01F4, 0xFFFF0000))
		{
			m_wUnk01F6 = 0xFFFF;
			m_wUnk01F4 = 0;
		}
		if (HoverFixLt((DWORD)HoverFixOfInt(1), *(DWORD*)&m_wUnk01F4))
		{
			m_wUnk01F6 = 1;
			m_wUnk01F4 = 0;
		}
		if (m_dwUnk0208 != 0 || f.bReverse != 0)
		{
			f.dwNegSteer = HoverFixNeg(*(DWORD*)&m_wUnk01F4);
			*(DWORD*)&m_wUnk01F4 = HoverFixVal(f.dwNegSteer);
		}
	}
	else
	{
		m_wUnk01F6 = 0;
		m_wUnk01F4 = 0;
	}
}

#pragma optimize("", on)

// hover.exe:0x40BC10
// FUNCTION: HOVER 0x40bc10
void CRobotPlayer::Sub_40BC10(CGameObj* pObj)
{
	m_adwUnk0224[m_dwUnk024C] = (DWORD)pObj;
	m_dwUnk024C++;
	if (m_dwUnk024C == 10)
		m_dwUnk024C = 0;
}

// hover.exe:0x42C320
// FUNCTION: HOVER 0x42c320
void* CRobotPlayer::operator new(size_t cb)
{
	CRobotPlayer* pBlock;
	DWORD dwCount;
	int i;
	void** ppLink;

	// The head of the free list is loaded once and reused as the result,
	// and the block base and pool size are re-read out of the globals
	// after the AddTail call; the last entry's link is reached by index
	// rather than through the walking pointer.
	pBlock = (CRobotPlayer*)g_dwUnk_4C54D0;
	if (pBlock == NULL)
	{
		g_dwUnk_4C54D0 = (DWORD)::operator new(0x688 * g_dwUnk_4C54D4);
		g_listUnk_4A46A0.AddTail((void*)g_dwUnk_4C54D0);
		dwCount = g_dwUnk_4C54D4;
		pBlock = (CRobotPlayer*)g_dwUnk_4C54D0;
		i = (int)dwCount - 1;
		if (i > 0)
		{
			ppLink = (void**)((BYTE*)pBlock + 0x260);
			do
			{
				ppLink = (void**)((BYTE*)ppLink + 0x688);
				i--;
				*(void**)((BYTE*)ppLink - 0x688) =
					(BYTE*)ppLink - 0x688 + 0x428;
			} while (i != 0);
		}
		pBlock[dwCount - 1].m_pUnk0260 = NULL;
	}
	g_dwUnk_4C54D0 = (DWORD)pBlock->m_pUnk0260;
	return pBlock;
}

/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// CSub410F20

// Builds a render context: default camera/shade constants, mode 1 with both
// option bits clear, empty tables, then the two blocks the context owns for
// its whole life - the 0x800-byte shift-count table filled by Sub_410EC0 and
// the 0x44-byte span-drawer function table whose +0x18..+0x3C slots
// Sub_410F20 rewrites for every mode.  Note that the Sub_410F20(1, 1) call
// happens *before* m_pUnk08 exists: mode 1 with dwFlags == 1 takes the
// (dwFlags & 2) == 0 path, which only writes m_pfnUnk00/m_pfnUnk0C and never
// dereferences m_pUnk08.

// hover.exe:0x420ED0
// FUNCTION: HOVER 0x420ed0
CSub410F20::CSub410F20()
{
	m_dblUnkA0 = 64.0;
	m_dblUnk98 = 64.0;
	m_dwUnk2C = 0;
	m_dwUnk58 = 0x80;
	m_dwUnkA8 = (DWORD)-1;
	m_dwUnk50 = 0x40;

	Sub_410F20(1, 1);

	m_pUnk34 = NULL;
	m_pUnk38 = NULL;
	m_pUnk30 = NULL;
	m_pUnk64 = NULL;
	m_pUnk6C = NULL;
	m_pUnk5C = NULL;
	m_dwUnk68 = 0;

	m_pUnk14 = (BYTE*)::operator new(0x800);
	m_pUnk08 = (DWORD*)::operator new(0x44);
}

// Releases the eight tables the render context owns. The original has no
// null checks: operator delete tolerates NULL.

// hover.exe:0x420F60
// FUNCTION: HOVER 0x420f60
CSub410F20::~CSub410F20()
{
	delete m_pUnk08;
	delete m_pUnk14;
	delete m_pUnk34;
	delete m_pUnk38;
	delete m_pUnk30;
	delete m_pUnk64;
	delete m_pUnk6C;
	delete m_pUnk5C;
}
