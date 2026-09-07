// texheap.cpp : implementation of the fixed-heap span renderer
//
// Reconstructed from hover.exe; see texheap.h for the class/struct notes.

#include "hover.h"
#include "texture.h"
#include "gameobj.h"	// CMerlinLine (global line objects)
#include "texheap.h"

/////////////////////////////////////////////////////////////////////////////
// Globals

// hover.exe:0x4A1D60
// GLOBAL: HOVER 0x4a1d60
BYTE* g_pUnk_4A1D60;			// span arena cursor

// hover.exe:0x4A1D64
// GLOBAL: HOVER 0x4a1d64
DWORD g_dwUnk_4A1D64;			// span window start

// hover.exe:0x4A1D68
// GLOBAL: HOVER 0x4a1d68
CSub403850* g_pUnk_4A1D68;		// view object

// hover.exe:0x4A1D6C
// GLOBAL: HOVER 0x4a1d6c
DWORD g_dwUnk_4A1D6C;			// span scale numerator

// hover.exe:0x4A1D70
// GLOBAL: HOVER 0x4a1d70
DWORD g_dwUnk_4A1D70;			// span origin offset

// hover.exe:0x4A1D74
// GLOBAL: HOVER 0x4a1d74
DWORD g_dwUnk_4A1D74;			// texture mip level

// hover.exe:0x4A1D78
// GLOBAL: HOVER 0x4a1d78
DWORD g_dwUnk_4A1D78;			// angle->mip LUT enable

// hover.exe:0x4A1D80
// GLOBAL: HOVER 0x4a1d80
BYTE g_abUnk_4A1D80[0x4A40A8 - 0x4A1D80];	// span arena (end 0x4A40A8)

// hover.exe:0x4A4490
// GLOBAL: HOVER 0x4a4490
HEAPNODE* g_pUnk_4A4490;		// edge list head

// hover.exe:0x4A4498
// GLOBAL: HOVER 0x4a4498
CMerlinLine g_lineUnk_4A4498;	// phase camera line (CSub406090::Sub_406090)

// hover.exe:0x4A44E0
// GLOBAL: HOVER 0x4a44e0
CMerlinLine g_lineUnk_4A44E0;	// right-edge phase camera line

// hover.exe:0x4A4524
// GLOBAL: HOVER 0x4a4524
DWORD g_dwUnk_4A4524;			// clip window start

// hover.exe:0x4A4528
// GLOBAL: HOVER 0x4a4528
WORD g_wUnk_4A4528;				// reset to 0 by CSub406090::Sub_406090

// hover.exe:0x4A452C
// GLOBAL: HOVER 0x4a452c
DWORD* g_pdwUnk_4A452C;			// scanline scale table

// hover.exe:0x4A4530
// GLOBAL: HOVER 0x4a4530
DWORD g_dwUnk_4A4530;			// low edge default

// hover.exe:0x4A4534
// GLOBAL: HOVER 0x4a4534
DWORD g_dwUnk_4A4534;			// fill color

// hover.exe:0x4A4538
// GLOBAL: HOVER 0x4a4538
DWORD g_dwUnk_4A4538;			// log2 screen width

// hover.exe:0x4A453C
// GLOBAL: HOVER 0x4a453c
DWORD g_dwUnk_4A453C;			// span step

// hover.exe:0x4A4540
// GLOBAL: HOVER 0x4a4540
DWORD g_dwUnk_4A4540;			// frame buffer origin

// hover.exe:0x4A4544
// GLOBAL: HOVER 0x4a4544
DWORD g_dwUnk_4A4544;			// screen height

// hover.exe:0x4A4548
// GLOBAL: HOVER 0x4a4548
BYTE* g_pbUnk_4A4548;			// angle->mip LUT

// hover.exe:0x4A454C
// GLOBAL: HOVER 0x4a454c
DWORD g_dwUnk_4A454C;			// screen width

// hover.exe:0x4A4550
// GLOBAL: HOVER 0x4a4550
DWORD* g_pdwUnk_4A4550;			// render target span buffer (m_pUnk3C)

// hover.exe:0x4A4554
// GLOBAL: HOVER 0x4a4554
DWORD g_dwUnk_4A4554;			// texture shift count (read as a byte by
								// the span drawers; stored as a dword by
								// CSub406090::Sub_406090)

// hover.exe:0x4A455C
// GLOBAL: HOVER 0x4a455c
DWORD g_dwUnk_4A455C;			// span length mask

// hover.exe:0x4A4564
// GLOBAL: HOVER 0x4a4564
DWORD g_dwUnk_4A4564;			// clip window end

// hover.exe:0x4A4568
// GLOBAL: HOVER 0x4a4568
DWORD g_dwUnk_4A4568;			// high edge default

// hover.exe:0x4A456C
// GLOBAL: HOVER 0x4a456c
DWORD g_dwUnk_4A456C;			// span window end

// hover.exe:0x4A4714
// GLOBAL: HOVER 0x4a4714
RENDEROBJ* g_pUnk_4A4714;		// render-object pool list

// hover.exe:0x4A4718
// GLOBAL: HOVER 0x4a4718
DWORD* g_pdwUnk_4A4718;			// span buffer tail (m_pUnk44)

// hover.exe:0x4A471C
// GLOBAL: HOVER 0x4a471c
RENDEROBJ* g_pUnk_4A471C;		// static list head

// hover.exe:0x4A4720
// GLOBAL: HOVER 0x4a4720
RENDEROBJ* g_pUnk_4A4720;		// active list head

// hover.exe:0x4A4A14
// GLOBAL: HOVER 0x4a4a14
DWORD g_dwUnk_4A4A14;			// span length

// hover.exe:0x4A4A18
// GLOBAL: HOVER 0x4a4a18
DWORD g_dwUnk_4A4A18;

// hover.exe:0x4A4A1C
// GLOBAL: HOVER 0x4a4a1c
DWORD g_dwUnk_4A4A1C;			// texture pixel pointer

// hover.exe:0x4A4A20
// GLOBAL: HOVER 0x4a4a20
DWORD g_dwUnk_4A4A20;

// hover.exe:0x4A4A24
// GLOBAL: HOVER 0x4a4a24
DWORD g_dwUnk_4A4A24;			// texture u step

// hover.exe:0x4A4A28
// GLOBAL: HOVER 0x4a4a28
DWORD g_dwUnk_4A4A28;			// texture u origin

// hover.exe:0x4A4A2C
// GLOBAL: HOVER 0x4a4a2c
DWORD g_dwUnk_4A4A2C;			// texture v origin

// hover.exe:0x4A4A30
// GLOBAL: HOVER 0x4a4a30
DWORD g_dwUnk_4A4A30;			// render target width * m_dwUnk1C

// hover.exe:0x4A4A34
// GLOBAL: HOVER 0x4a4a34
DWORD g_dwUnk_4A4A34;			// frame buffer pitch

// hover.exe:0x4A4A38
// GLOBAL: HOVER 0x4a4a38
DWORD g_dwUnk_4A4A38;

// hover.exe:0x4A4A3C
// GLOBAL: HOVER 0x4a4a3c
DWORD g_dwUnk_4A4A3C;			// span limit table entry

// hover.exe:0x4A4A40
// GLOBAL: HOVER 0x4a4a40
DWORD g_dwUnk_4A4A40;

// hover.exe:0x4A4A44
// GLOBAL: HOVER 0x4a4a44
DWORD g_dwUnk_4A4A44;			// span scale

// hover.exe:0x4A4A48
// GLOBAL: HOVER 0x4a4a48
DWORD g_dwUnk_4A4A48;			// drawer selector

// hover.exe:0x4A4A4C
// GLOBAL: HOVER 0x4a4a4c
DWORD g_dwUnk_4A4A4C;			// destination pointer

// hover.exe:0x4A4A50
// GLOBAL: HOVER 0x4a4a50
DWORD g_dwUnk_4A4A50;			// texture step

// hover.exe:0x4C5208
// GLOBAL: HOVER 0x4c5208
DWORD g_dwUnk_4C5208 = 0xFFF0000;	// edge sentinel (dereferenced through
									// RENDEROBJ::m_pUnk84/m_pUnk88)

/////////////////////////////////////////////////////////////////////////////
// Helper macros reconstructed from the inline code sequences in the
// original (the binary carries several compiled copies of each)

// Table lookup used by Sub_402B10: g_adwSinTable is addressed by byte
// offset ((expr) & 0x7FFC) starting one dword into the table
#define HOVER_SINLUT(expr) g_adwSinTable[(((expr) & 0x7FFC) >> 2) + 1]

// Sub_4030F0: unlink pScan from the g_pUnk_4A4490 list and advance to the
// next node (ends the scan loop when the list runs out)
#define HOVER_SPANUNLINK(pScan, pPrev)							\
	pPrev = pScan->m_pUnk0C;									\
	pScan = pScan->m_pUnk08;									\
	if (pPrev != 0)												\
		pPrev->m_pUnk08 = pScan;								\
	else														\
		g_pUnk_4A4490 = pScan;									\
	if (pScan == 0)												\
		break;													\
	pScan->m_pUnk0C = pPrev

// Sub_4030F0: merge the render object behind pScan into pTarget's top
// edge and unlink the node; falls through to the next comparison when the
// two objects are not both merge candidates
#define HOVER_SPANMERGE(pObj, pScan, pPrev, pKey, pEdge)		\
	if (pObj->m_dwUnk20 != 0)									\
	{															\
		HOVER_SPANUNLINK(pScan, pPrev);							\
		continue;												\
	}															\
	if (pTarget->m_dwUnk18 == 0 && pObj->m_dwUnk18 == 0)		\
	{															\
		if (pScan->m_dwUnk10 != 0)								\
		{														\
			pTarget->m_pUnk40 = pObj;							\
			pTarget->m_dwUnk48 = (DWORD)(pKey);					\
			pTarget->m_dwUnk44 = 1;								\
			pTarget->m_dwUnk8C = 1;								\
			pTarget->m_pUnk84 = (pEdge);						\
		}														\
		else													\
		{														\
			pTarget->m_pUnk4C = pObj;							\
			pTarget->m_dwUnk54 = (DWORD)(pKey);					\
			pTarget->m_dwUnk50 = 1;								\
			pTarget->m_dwUnk8C = 1;								\
			pTarget->m_pUnk88 = (pEdge);						\
		}														\
		HOVER_SPANUNLINK(pScan, pPrev);							\
		continue;												\
	}

// Sub_4030F0: the same merge for the bottom edge.  The original spells
// this copy out separately: the edge flag is cleared instead of set and
// the five stores go out in a different order.
#define HOVER_SPANMERGE_LO(pObj, pScan, pPrev, pKey, pEdge)		\
	if (pObj->m_dwUnk20 != 0)									\
	{															\
		HOVER_SPANUNLINK(pScan, pPrev);							\
		continue;												\
	}															\
	if (pTarget->m_dwUnk18 == 0 && pObj->m_dwUnk18 == 0)		\
	{															\
		if (pScan->m_dwUnk10 != 0)								\
		{														\
			pTarget->m_dwUnk44 = 0;								\
			pTarget->m_pUnk40 = pObj;							\
			pTarget->m_dwUnk8C = 1;								\
			pTarget->m_dwUnk48 = (DWORD)(pKey);					\
			pTarget->m_pUnk84 = (pEdge);						\
		}														\
		else													\
		{														\
			pTarget->m_dwUnk50 = 0;								\
			pTarget->m_pUnk4C = pObj;							\
			pTarget->m_dwUnk8C = 1;								\
			pTarget->m_dwUnk54 = (DWORD)(pKey);					\
			pTarget->m_pUnk88 = (pEdge);						\
		}														\
		HOVER_SPANUNLINK(pScan, pPrev);							\
		continue;												\
	}

/////////////////////////////////////////////////////////////////////////////
// CSub402E00

// hover.exe:0x402630
// FUNCTION: HOVER 0x402630
void CSub402E00::Sub_402630(RENDEROBJ* pObj, DWORD dwA, DWORD dwB)
{
	SPANENTRY* pSpan;
	DWORD dwB4;
	DWORD dwB8;
	DWORD dwV;
	int bCovered;

	dwB4 = pObj->m_acUnkAC.m_dwUnk08;
	dwB8 = pObj->m_acUnkAC.m_dwUnk0C;
	pSpan = &pObj->m_acUnkAC;
	g_dwUnk_4A4564 = dwB;
	// the original carries a dead cmp of m_dwUnk0C here
	g_dwUnk_4A4524 = dwA;
	pObj->m_pUnk10->m_wFlags |= 8;
	if (pObj->m_pUnk40 != 0)
	{
		if ((int)pObj->m_dwUnk6C > 0)
		{
			bCovered = 0;
			dwV = *pObj->m_pUnk84;
			if ((int)dwV > (int)dwB4)
			{
				if ((int)dwV > (int)dwA)
				{
					if ((int)dwV >= (int)dwB)
					{
						m_pUnk00->Sub_403980(pObj->m_dwUnk60,
							pObj->m_dwUnk48, dwA, dwB);
						bCovered = 1;
					}
					else
					{
						m_pUnk00->Sub_403980(pObj->m_dwUnk60,
							pObj->m_dwUnk48, dwA, dwV);
						g_dwUnk_4A4524 = dwV + g_dwUnk_4A453C;
					}
				}
			}
			else if ((int)dwA < (int)dwB4)
			{
				if ((int)dwB <= (int)dwB4)
				{
					m_pUnk00->Sub_403980(pObj->m_dwUnk60, pObj->m_dwUnk48,
						dwA, dwB);
					bCovered = 1;
				}
				else
				{
					m_pUnk00->Sub_403980(pObj->m_dwUnk60, pObj->m_dwUnk48,
						dwA, dwB4);
					g_dwUnk_4A4524 = dwB4 + g_dwUnk_4A453C;
				}
			}
			if (bCovered)
				return;
			if (pObj->m_pUnk4C == 0)
				goto L_final;
			dwV = *pObj->m_pUnk88;
			if ((int)dwV > (int)dwB8)
			{
				if ((int)g_dwUnk_4A4564 > (int)dwB8)
				{
					if ((int)g_dwUnk_4A4524 >= (int)dwB8)
					{
						m_pUnk00->Sub_403980(pObj->m_dwUnk64,
							pObj->m_dwUnk54, g_dwUnk_4A4524, g_dwUnk_4A4564);
						bCovered = 1;
					}
					else
					{
						m_pUnk00->Sub_403980(pObj->m_dwUnk64,
							pObj->m_dwUnk54, dwB8, g_dwUnk_4A4564);
						g_dwUnk_4A4564 = dwB8 - g_dwUnk_4A453C;
					}
				}
			}
			else if ((int)g_dwUnk_4A4564 > (int)dwV)
			{
				if ((int)g_dwUnk_4A4524 >= (int)dwV)
				{
					m_pUnk00->Sub_403980(pObj->m_dwUnk64, pObj->m_dwUnk54,
						g_dwUnk_4A4524, g_dwUnk_4A4564);
					bCovered = 1;
				}
				else
				{
					m_pUnk00->Sub_403980(pObj->m_dwUnk64, pObj->m_dwUnk54,
						dwV, g_dwUnk_4A4564);
					g_dwUnk_4A4564 = dwV - g_dwUnk_4A453C;
				}
			}
			if (bCovered)
				return;
			goto L_final;
		}
		if (pObj->m_pUnk4C != 0)
		{
			bCovered = 0;
			dwV = *pObj->m_pUnk88;
			if ((int)dwV > (int)dwB8)
			{
				if ((int)dwB > (int)dwB8)
				{
					if ((int)dwA >= (int)dwB8)
					{
						m_pUnk00->Sub_403980(pObj->m_dwUnk64,
							pObj->m_dwUnk54, dwA, dwB);
						bCovered = 1;
					}
					else
					{
						m_pUnk00->Sub_403980(pObj->m_dwUnk64,
							pObj->m_dwUnk54, dwB8, dwB);
						g_dwUnk_4A4564 = dwB8 - g_dwUnk_4A453C;
					}
				}
			}
			else if ((int)dwB > (int)dwV)
			{
				if ((int)dwA >= (int)dwV)
				{
					m_pUnk00->Sub_403980(pObj->m_dwUnk64, pObj->m_dwUnk54,
						dwA, dwB);
					bCovered = 1;
				}
				else
				{
					m_pUnk00->Sub_403980(pObj->m_dwUnk64, pObj->m_dwUnk54,
						dwV, dwB);
					g_dwUnk_4A4564 = dwV - g_dwUnk_4A453C;
				}
			}
			if (bCovered)
				return;
		}
		bCovered = 0;
		dwV = *pObj->m_pUnk84;
		if ((int)dwB4 < (int)dwV)
		{
			if ((int)g_dwUnk_4A4524 < (int)dwV)
			{
				if ((int)g_dwUnk_4A4564 <= (int)dwV)
				{
					m_pUnk00->Sub_403980(pObj->m_dwUnk60, pObj->m_dwUnk48,
						g_dwUnk_4A4524, g_dwUnk_4A4564);
					bCovered = 1;
				}
				else
				{
					m_pUnk00->Sub_403980(pObj->m_dwUnk60, pObj->m_dwUnk48,
						g_dwUnk_4A4524, dwV);
					g_dwUnk_4A4524 = dwV + g_dwUnk_4A453C;
				}
			}
		}
		else if ((int)g_dwUnk_4A4524 < (int)dwB4)
		{
			if ((int)g_dwUnk_4A4564 <= (int)dwB4)
			{
				m_pUnk00->Sub_403980(pObj->m_dwUnk60, pObj->m_dwUnk48,
					g_dwUnk_4A4524, g_dwUnk_4A4564);
				bCovered = 1;
			}
			else
			{
				m_pUnk00->Sub_403980(pObj->m_dwUnk60, pObj->m_dwUnk48,
					g_dwUnk_4A4524, dwB4);
				g_dwUnk_4A4524 = dwB4 + g_dwUnk_4A453C;
			}
		}
		if (bCovered)
			return;
		goto L_final;
	}
	if (Sub_4065F0(pObj, pSpan, dwB4, dwB8, pObj->m_dwUnk64) != 0)
		return;
L_final:
	if ((int)g_dwUnk_4A4564 < (int)g_dwUnk_4A4524)
		return;
	Sub_402990(pObj, pSpan, g_dwUnk_4A4524, g_dwUnk_4A4564);
}

// hover.exe:0x402990
// FUNCTION: HOVER 0x402990
void CSub402E00::Sub_402990(RENDEROBJ* pObj, SPANENTRY* pSpan, DWORD dwA,
	DWORD dwB)
{
	CMerlinTexture* pTex;
	MERLIN_TEXENTRY* pEntry;
	SPANPTS* pPts;
	DWORD dwLevel;
	DWORD dwMax;
	DWORD dwVal;
	DWORD dwX;
	DWORD dwP;
	DWORD dwT;
	BYTE cl;

	g_dwUnk_4A4A14 = dwB + (g_dwUnk_4A453C - dwA);
	if ((int)g_dwUnk_4A4A14 <= 0)
		return;
	g_dwUnk_4A4A4C = g_dwUnk_4A4540 + dwA * g_dwUnk_4A4A34;
	g_dwUnk_4A4A44 = (DWORD)(short)pObj->m_wUnk08;
	if (pObj->m_dwUnkA4 == 0)
	{
		pTex = pObj->m_pUnk5C;
		// the original carries a dead cmp of m_dwUnk0C here
		pObj->m_pUnk10->m_wFlags |= 8;
		if (g_dwUnk_4A1D78 != 0)
		{
			cl = (BYTE)(pObj->m_dwUnk9C + 4);
			dwVal = (DWORD)(((int)g_dwUnk_4A4A44 >> cl) & 0x7FF);
			dwLevel = g_pbUnk_4A4548[dwVal];
			dwMax = pTex->m_dwUnk2C - 1;
			if ((int)dwMax < (int)dwLevel)
				dwLevel = dwMax;
			if ((int)g_dwUnk_461CFC > (int)dwLevel)
				dwLevel = g_dwUnk_461CFC;
		}
		else
		{
			dwLevel = g_dwUnk_461CFC;
		}
		pEntry = &pTex->m_pUnk28[dwLevel];
		cl = (BYTE)(pObj->m_dwUnk9C + dwLevel);
		dwVal = (DWORD)(((short)pObj->m_wUnk0A >> cl) &
			(int)pEntry->m_dwUnk00);
		pPts = (SPANPTS*)(pEntry->m_dwUnk1C + dwVal * 8);
		dwP = pPts->m_pUnk04->m_dwUnk04;
		dwX = pSpan->m_dwUnk0C - pSpan->m_dwUnk08;
		pSpan->m_dwUnk10 = dwP;
		g_dwUnk_4A4A1C = dwP;
		if (dwX != 0)
		{
			dwP = (DWORD)((int)pObj->m_dwUnk68 / (int)dwX);
			dwP = (DWORD)((int)dwP >> cl);
		}
		else
		{
			dwP = 0;
		}
		dwT = pEntry->m_dwUnk04;
		pObj->m_dwUnk98 = dwP;
		pObj->m_dwUnkA4 = 1;
		dwT = (dwT << 0x10) + 0xFFFF;
		pObj->m_dwUnk94 = dwT;
		g_dwUnk_4A4A38 = dwT;
	}
	else
	{
		g_dwUnk_4A4A38 = pObj->m_dwUnk94;
		dwP = pObj->m_dwUnk98;
		g_dwUnk_4A4A1C = pSpan->m_dwUnk10;
	}
	g_dwUnk_4A4A50 = dwP;
	g_dwUnk_4A4A40 = (dwA - pSpan->m_dwUnk08) * dwP;
	g_dwUnk_4A4A3C = (DWORD)g_pUnk_4A1D68->Sub_403850(g_dwUnk_4A4A44);
	g_pUnk_4A1D68->m_pfnUnk00();
}

// hover.exe:0x402B10
// FUNCTION: HOVER 0x402b10
void CSub402E00::Sub_402B10(DWORD dwA, DWORD dwB, CMerlinTexture* pTex,
	DWORD dwY)
{
	CSub403850* pView;
	MERLIN_TEXENTRY* pEntry;
	DWORD dwX0;
	DWORD dwRow;
	DWORD dwScale;
	DWORD dwLevel;
	DWORD dwCount;
	DWORD dwFb;
	DWORD dwDiv;
	DWORD dwAng;
	DWORD dwBX;
	DWORD dwBY;
	DWORD dwT14;
	DWORD dwT24;
	DWORD dwF0;
	DWORD dwF1;
	DWORD dwF2;
	DWORD dwF3;
	int nT;
	BYTE cl;

	dwX0 = dwB;
	dwRow = g_dwUnk_4A4544 - dwY;
	nT = abs((int)((dwX0 & (0 - g_dwUnk_4A453C)) - g_dwUnk_4A1D70));
	dwScale = (DWORD)((int)g_pdwUnk_4A452C[dwRow + 0x400] / (nT + 1));
	g_dwUnk_4A4A44 = dwScale;
	if (m_dwUnk24 != 0)
	{
		dwLevel = (DWORD)g_pbUnk_4A4548[(dwScale & 0x7FF0) >> 4] + 1;
		g_dwUnk_4A1D74 = dwLevel;
		if ((int)g_dwUnk_4A4A44 < 0xFA)
			dwLevel = g_dwUnk_461CFC;
		g_dwUnk_4A1D74 = dwLevel;
		if ((int)dwLevel >= (int)pTex->m_dwUnk2C)
			dwLevel = pTex->m_dwUnk2C - 1;
		g_dwUnk_4A1D74 = dwLevel;
		if ((int)g_dwUnk_461CFC > (int)dwLevel)
			dwLevel = g_dwUnk_461CFC;
	}
	else
	{
		dwLevel = g_dwUnk_461CFC;
	}
	cl = (BYTE)dwLevel;
	pEntry = &pTex->m_pUnk28[dwLevel];
	pView = g_pUnk_4A1D68;
	g_dwUnk_4A4A1C = pEntry->m_dwUnk18;
	if (pView->m_dwUnk68 != 0)
	{
		dwT14 = g_dwUnk_4A454C - g_dwUnk_4A4534 - 1;
		dwAng = pView->m_dwUnk54 << 6;
		dwBX = pView->m_dwUnk48 << 0x10;
		dwT24 = g_dwUnk_4A454C - dwA - 1;
		dwBY = pView->m_dwUnk4C << 0x10;
		dwF0 = HOVER_SINLUT(dwAng + 0x1554) * g_dwUnk_4A4A44 + dwBX;
		dwF1 = dwBX + HOVER_SINLUT(dwAng + 0x2AA8) * g_dwUnk_4A4A44;
		dwF3 = HOVER_SINLUT(dwAng - 0xAAC) * g_dwUnk_4A4A44 + dwBY;
		dwF2 = dwBY + HOVER_SINLUT(dwAng + 0xAA8) * g_dwUnk_4A4A44;
	}
	else
	{
		dwT14 = dwA;
		dwBX = pView->m_dwUnk48 << 0x10;
		dwAng = pView->m_dwUnk54 << 6;
		dwT24 = g_dwUnk_4A4534;
		dwBY = pView->m_dwUnk4C << 0x10;
		dwF1 = HOVER_SINLUT(dwAng + 0x1554) * g_dwUnk_4A4A44 + dwBX;
		dwF0 = HOVER_SINLUT(dwAng + 0x2AA8) * g_dwUnk_4A4A44 + dwBX;
		dwAng += 0xAA8;
		dwF2 = HOVER_SINLUT(dwAng - 0x1554) * g_dwUnk_4A4A44 + dwBY;
		dwF3 = HOVER_SINLUT(dwAng) * g_dwUnk_4A4A44 + dwBY;
	}
	dwCount = dwT24 - dwT14;
	g_dwUnk_4A1D74 = dwLevel;
	dwFb = dwX0 * g_dwUnk_4A4A34 + pView->m_dwUnk28;
	g_dwUnk_4A4A14 = dwCount;
	g_dwUnk_4A4A4C = dwT14 + dwFb;
	dwDiv = g_dwUnk_4A454C << cl;
	g_dwUnk_4A4A50 = (DWORD)((int)(dwF3 - dwF2) / (int)dwDiv);
	dwF2 = (DWORD)((int)dwF2 >> cl);
	g_dwUnk_4A4A24 = (DWORD)((int)(dwF0 - dwF1) / (int)dwDiv);
	dwF1 = (DWORD)((int)dwF1 >> cl);
	g_dwUnk_4A4A28 = dwT14 * g_dwUnk_4A4A50 + dwF2;
	g_dwUnk_4A4A2C = g_dwUnk_4A4A24 * dwT14 + dwF1;
	g_dwUnk_4A4A3C = (DWORD)pView->Sub_403850(g_dwUnk_4A4A44);
	g_dwUnk_4A4A20 = (pEntry->m_dwUnk04 << 0x10) + 0xFFFF;
	g_dwUnk_4A4A38 = pEntry->m_dwUnk00 << 0x10;
	g_dwUnk_4A4A48 = 0x10 - pEntry->m_dwUnk10;
	if (pView->m_dwUnk80 != 0)
		pView->m_ppfnUnk08[g_dwUnk_4A4A48]();
	else
		pView->m_pfnUnk0C();
}

// hover.exe:0x402E00
// FUNCTION: HOVER 0x402e00
void CSub402E00::Sub_402E00(DWORD dwUnk)
{
	RENDEROBJ* pObj;
	SPANENTRY* pSpan;
	SPANENTRY* pArena;
	SPANPTS* pPts;
	SPANPT* pPt;
	MERLIN_TEXENTRY* pEntry;
	DWORD dwScale;
	DWORD dwMask;
	DWORD dwX0;
	DWORD dwX1;
	DWORD dwLevel;
	DWORD dwMax;
	DWORD dwT;
	DWORD dwA;
	DWORD dwB;
	DWORD i;
	int nCount;
	int nPts;
	BYTE cl;

	g_pUnk_4A1D60 = g_abUnk_4A1D80;
	if (m_dwUnk28 != 0)
		Sub_4030F0();
	for (pObj = g_pUnk_4A471C; pObj != 0; pObj = pObj->m_pUnk34)
	{
		nCount = (short)pObj->m_wUnk08;
		if (pObj->m_dwUnk18 != 0 || nCount <= 0)
		{
			if (nCount == 0)
			{
				// no points: clamp the embedded span to the edge defaults
				if ((int)pObj->m_dwUnk6C > 0)
					pObj->m_acUnkAC.m_dwUnk08 = g_dwUnk_4A4530;
				else
					pObj->m_acUnkAC.m_dwUnk08 = g_dwUnk_4A4568;
				if ((int)pObj->m_dwUnk70 > 0)
					pObj->m_acUnkAC.m_dwUnk0C = g_dwUnk_4A4530;
				else
					pObj->m_acUnkAC.m_dwUnk0C = g_dwUnk_4A4568;
			}
			pObj->m_dwUnk58 = 0;
			continue;
		}
		pSpan = &pObj->m_acUnkAC;
		dwScale = (DWORD)((int)g_dwUnk_4A1D6C / nCount);
		pObj->m_dwUnk58 = 1;
		dwX0 = (DWORD)(((int)(pObj->m_dwUnk6C * dwScale) >> 0xA) +
			(int)g_dwUnk_4A1D70);
		dwMask = 0 - g_dwUnk_4A453C;
		dwX0 &= dwMask;
		dwX1 = (DWORD)(((int)(pObj->m_dwUnk70 * dwScale) >> 0xA) +
			(int)g_dwUnk_4A1D70);
		dwX1 &= dwMask;
		pSpan->m_dwUnk08 = dwX0;
		pSpan->m_dwUnk0C = dwX1;
		if (pObj->m_dwUnkA0 == 0)
		{
			DWORD dwA, dwB, dwMin;

			pObj->m_dwUnkA4 = 0;
			if (pObj->m_dwUnk8C == 0)
				continue;
			dwA = *pObj->m_pUnk84;
			dwB = *pObj->m_pUnk88;
			dwMin = ((int)dwB >= (int)dwA) ? dwA : dwB;
			pObj->m_dwUnk74 = ((int)dwMin > (int)dwX0) ? dwX0 : dwMin;
			if (dwA != 0xFFF0000 && (int)dwA > (int)dwX1)
				dwX1 = dwA;
			if (dwB != 0xFFF0000 && (int)dwX1 < (int)dwB)
				pObj->m_dwUnk78 = dwB;
			else
				pObj->m_dwUnk78 = dwX1;
			continue;
		}
		pObj->m_dwUnk74 = dwX0;
		pObj->m_dwUnk78 = dwX1;
		if (g_dwUnk_4A1D78 != 0)
		{
			cl = (BYTE)(pObj->m_dwUnk9C + 4);
			dwT = (DWORD)((nCount >> cl) & 0x7FF);
			dwLevel = g_pbUnk_4A4548[dwT];
			dwMax = pObj->m_pUnk5C->m_dwUnk2C - 1;
			if ((int)dwLevel > (int)dwMax)
				dwLevel = dwMax;
			if ((int)g_dwUnk_461CFC > (int)dwLevel)
				dwLevel = g_dwUnk_461CFC;
		}
		else
		{
			dwLevel = g_dwUnk_461CFC;
		}
		// the original carries a dead cmp of m_dwUnk0C here
		pObj->m_pUnk10->m_wFlags |= 8;
		pEntry = &pObj->m_pUnk5C->m_pUnk28[dwLevel];
		cl = (BYTE)(pObj->m_dwUnk9C + dwLevel);
		nPts = ((short)pObj->m_wUnk0A >> cl) & (int)pEntry->m_dwUnk00;
		pPts = (SPANPTS*)(pEntry->m_dwUnk1C + ((DWORD)nPts << 3));
		dwX1 -= dwX0;
		if (dwX1 != 0)
		{
			dwX1 <<= cl;
			pObj->m_dwUnk98 = (DWORD)((int)pObj->m_dwUnk68 / (int)dwX1);
		}
		else
		{
			pObj->m_dwUnk98 = 0;
		}
		dwT = pEntry->m_dwUnk04 << 0x10;
		pArena = (SPANENTRY*)g_pUnk_4A1D60;
		pObj->m_pUnkA8 = pArena;
		dwT += 0xFFFF;
		pObj->m_dwUnk90 = (DWORD)(short)pPts->m_wUnk00;
		pObj->m_dwUnk94 = dwT;
		g_pUnk_4A1D60 += pObj->m_dwUnk90 * 0x14;
		if (pObj->m_dwUnk90 == 0)
		{
			pObj->m_dwUnk58 = 0;
			continue;
		}
		pPt = pPts->m_pUnk04;
		if ((int)pObj->m_dwUnk90 <= 0)
			continue;
		i = pObj->m_dwUnk90;
		do
		{
			cl = (BYTE)pObj->m_dwUnk9C;
			dwA = pObj->m_dwUnk60 - (DWORD)(((short)pPt->m_wUnk00) << cl);
			dwB = pObj->m_dwUnk60 - (DWORD)(((short)pPt->m_wUnk02) << cl);
			pArena->m_dwUnk00 = dwA;
			pArena->m_dwUnk04 = dwB;
			if ((int)pObj->m_dwUnk64 > (int)dwB)
			{
				dwB = pObj->m_dwUnk64;
				pArena->m_dwUnk04 = dwB;
			}
			pArena->m_dwUnk10 = pPt->m_dwUnk04;
			pPt++;
			pArena++;
			dwT = (DWORD)(((int)((g_dwUnk_4A4544 - dwA) * dwScale) >> 0xA) +
				(int)g_dwUnk_4A1D70);
			dwT &= dwMask;
			pArena[-1].m_dwUnk08 = dwT;
			dwT = (DWORD)(((int)((g_dwUnk_4A4544 - pArena[-1].m_dwUnk04) *
				dwScale) >> 0xA) + (int)g_dwUnk_4A1D70);
			dwT &= dwMask;
			pArena[-1].m_dwUnk0C = dwT;
			i--;
		} while (i != 0);
	}
	Sub_404030(g_pUnk_4A4720, g_dwUnk_4A1D64, g_dwUnk_4A456C);
	m_pUnk00->Sub_403A60(this);
	if (g_pUnk_4A1D60 > g_abUnk_4A1D80 + (0x4A40A8 - 0x4A1D80))
		OutputDebugStringA("Trashing Heap");	// hover.exe:0x4C520C
}

// hover.exe:0x4030F0
// FUNCTION: HOVER 0x4030f0
void CSub402E00::Sub_4030F0()
{
	RENDEROBJ* pObj;
	RENDEROBJ* pTarget;
	RENDEROBJ* pObj2;
	RENDEROBJ* pNext2;
	RENDEROBJ* pPrev2;
	CMerlinStatic* pSrc;
	CMerlinTexture* pTex;
	HEAPNODE* pNode;
	HEAPNODE* pScan;
	HEAPNODE* pNew;
	HEAPNODE* pPrev;
	HEAPNODE* pOldHead;
	DWORD dwArea;
	DWORD dwDelta;
	DWORD dwShift;
	DWORD dwT1;
	DWORD dwT2;
	DWORD dwVal;
	void* pSrcB;
	void* pKey;
	int nW56;
	int nW54;

	g_pUnk_4A4490 = 0;
	pObj = g_pUnk_4A4714;
	pNode = (HEAPNODE*)g_abUnk_4A1D80;
	if (pObj->m_pUnk2C != 0)
	{
		do
		{
			pObj = pObj->m_pUnk2C;
		} while (pObj->m_pUnk2C != 0);
	}
	g_pUnk_4A1D60 = (BYTE*)pNode;
	if (pObj != 0)
	{
		for (;;)
		{
			pObj->m_pUnk40 = 0;
			pObj->m_pUnk4C = 0;
			pObj->m_dwUnk8C = 0;
			pObj->m_pUnk84 = &g_dwUnk_4C5208;
			pObj->m_pUnk88 = &g_dwUnk_4C5208;
			if (pObj->m_dwUnk0C != 0)
			{
				// sprite object: derive the span from the sprite's
				// height words
				nW56 = (short)((CMerlinDynamic*)pObj->m_pUnk10)->m_wUnk56;
				nW54 = (short)((CMerlinDynamic*)pObj->m_pUnk10)->m_wUnk54;
				dwArea = (DWORD)(nW56 - nW54) << 0x10;
				pObj->m_dwUnk60 = (DWORD)nW56;
				pObj->m_dwUnk64 = (DWORD)nW54;
				pObj->m_dwUnk68 = dwArea;
				pObj->m_dwUnk6C = g_dwUnk_4A4544 - (DWORD)nW56;
				pObj->m_dwUnkA0 = 1;
				pObj->m_dwUnk70 = g_dwUnk_4A4544 - (DWORD)nW54;
				pTex = ((CMerlinDynamic*)pObj->m_pUnk10)->m_pUnk50;
				pObj->m_pUnk5C = pTex;
				if (pTex != 0)
				{
					pObj->m_dwUnk9C = (DWORD)(((int)dwArea >> 0x10) /
						(short)pTex->m_wUnk18);
				}
				else
				{
					pObj->m_dwUnk18 = 1;
				}
				dwShift = 0;
				while ((int)pObj->m_dwUnk9C > 1)
				{
					dwShift++;
					pObj->m_dwUnk9C = (DWORD)((int)pObj->m_dwUnk9C >> 1);
				}
				pObj->m_dwUnk9C = dwShift;
			}
			else
			{
				pSrc = (CMerlinStatic*)pObj->m_pUnk10;
				pScan = g_pUnk_4A4490;
				nW56 = (short)pSrc->m_wUnkA6;
				nW54 = (short)pSrc->m_wUnkA4;
				pObj->m_dwUnk60 = (DWORD)nW56;
				dwDelta = (DWORD)(nW56 - nW54);
				pObj->m_dwUnk64 = (DWORD)nW54;
				dwArea = dwDelta << 0x10;
				pObj->m_dwUnk68 = dwArea;
				pObj->m_dwUnk6C = (DWORD)((int)g_dwUnk_4A4544 - nW56);
				pObj->m_dwUnk70 = (DWORD)((int)g_dwUnk_4A4544 - nW54);
				if (pObj->m_dwUnk18 != 0)
				{
					dwT1 = pSrc->m_dwY1;
					dwT1 -= g_pUnk_4A1D68->m_dwUnk4C;
					dwT1 *= pSrc->m_dwDeltaX;
					dwT2 = g_pUnk_4A1D68->m_dwUnk48;
					dwT2 -= pSrc->m_dwX1;
					pObj->m_dwUnk14 = 0;
					dwT2 *= pSrc->m_dwDeltaY;
					if ((int)(dwT1 + dwT2) >= 0)
						pObj->m_dwUnk14 = 1;
				}
				if (pObj->m_dwUnk14 != 0)
				{
					pObj->m_pUnk5C = pSrc->m_pUnk94;
					pObj->m_dwUnkA0 = pSrc->m_dwUnkB0;
				}
				else
				{
					pObj->m_pUnk5C = pSrc->m_pUnk98;
					pObj->m_dwUnkA0 = pSrc->m_dwUnkB4;
				}
				if (pObj->m_pUnk5C != 0)
				{
					dwT1 = (DWORD)(short)pObj->m_pUnk5C->m_wUnk18;
					pObj->m_dwUnk9C = (DWORD)(((int)dwArea >> 0x10) /
						(int)dwT1);
				}
				else
				{
					pObj->m_dwUnk18 = 1;
				}
				dwShift = 0;
				while ((int)pObj->m_dwUnk9C > 1)
				{
					dwShift++;
					pObj->m_dwUnk9C = (DWORD)((int)pObj->m_dwUnk9C >> 1);
				}
				pObj->m_dwUnk9C = dwShift;
				if (pObj->m_dwUnk14 != 0)
				{
					// merge matching front-face edges out of the list
					if (g_pUnk_4A4490 != 0)
					{
						do
						{
							dwVal = pScan->m_dwUnk14;
							if (dwVal == (DWORD)nW56)
							{
								pKey = pScan->m_pUnk04;
								if (pSrc->m_pUnk90 == pKey)
								{
									pTarget = pScan->m_pUnk00;
									HOVER_SPANMERGE(pObj, pScan,
										pPrev, pKey,
										&pObj->m_acUnkAC.m_dwUnk08);
								}
							}
							if (dwVal == (DWORD)nW54)
							{
								pKey = pScan->m_pUnk04;
								if (pSrc->m_pUnkA0 == pKey)
								{
									pTarget = pScan->m_pUnk00;
									HOVER_SPANMERGE_LO(pObj, pScan,
										pPrev, pKey,
										&pObj->m_acUnkAC.m_dwUnk0C);
								}
							}
							pScan = pScan->m_pUnk08;
						} while (pScan != 0);
					}
					if (pObj->m_dwUnk20 == 0)
					{
						pSrcB = pSrc->m_pUnk8C;
						if (pSrcB != 0)
						{
							pOldHead = g_pUnk_4A4490;
							pNew = pNode;
							pNew->m_pUnk0C = 0;
							g_pUnk_4A4490 = pNew;
							pNew->m_pUnk08 = pOldHead;
							if (pOldHead != 0)
								pOldHead->m_pUnk0C = pNew;
							pNew->m_pUnk04 = pSrcB;
							pNew->m_pUnk00 = pObj;
							pNew->m_dwUnk10 = 1;
							pNew->m_dwUnk14 = (DWORD)nW56;
							pNode++;
						}
						pSrcB = pSrc->m_pUnk9C;
						if (pSrcB != 0 && (int)dwDelta > 0)
						{
							pOldHead = g_pUnk_4A4490;
							pNew = pNode;
							pNew->m_pUnk0C = 0;
							g_pUnk_4A4490 = pNew;
							pNew->m_pUnk08 = pOldHead;
							if (pOldHead != 0)
								pOldHead->m_pUnk0C = pNew;
							pNew->m_pUnk04 = pSrcB;
							pNew->m_dwUnk14 = (DWORD)nW54;
							pNew->m_pUnk00 = pObj;
							pNew->m_dwUnk10 = 0;
							pNode++;
						}
					}
				}
				else
				{
					// merge matching back-face edges out of the list
					while (pScan != 0)
					{
						dwVal = pScan->m_dwUnk14;
						if (dwVal == (DWORD)nW56)
						{
							pKey = pScan->m_pUnk04;
							if (pSrc->m_pUnk8C == pKey)
							{
								pTarget = pScan->m_pUnk00;
								HOVER_SPANMERGE(pObj, pScan, pPrev,
									pKey, &pObj->m_acUnkAC.m_dwUnk08);
							}
						}
						if (dwVal == (DWORD)nW54)
						{
							pKey = pScan->m_pUnk04;
							if (pSrc->m_pUnk9C == pKey)
							{
								pTarget = pScan->m_pUnk00;
								HOVER_SPANMERGE_LO(pObj, pScan, pPrev,
									pKey, &pObj->m_acUnkAC.m_dwUnk0C);
							}
						}
						pScan = pScan->m_pUnk08;
					}
					if (pObj->m_dwUnk20 == 0)
					{
						pSrcB = pSrc->m_pUnk90;
						if (pSrcB != 0)
						{
							pOldHead = g_pUnk_4A4490;
							pNew = pNode;
							pNew->m_pUnk0C = 0;
							g_pUnk_4A4490 = pNew;
							pNew->m_pUnk08 = pOldHead;
							if (pOldHead != 0)
								pOldHead->m_pUnk0C = pNew;
							pNew->m_pUnk04 = pSrcB;
							pNew->m_pUnk00 = pObj;
							pNew->m_dwUnk10 = 1;
							pNew->m_dwUnk14 = (DWORD)nW56;
							pNode++;
						}
						pSrcB = pSrc->m_pUnkA0;
						if (pSrcB != 0 && (int)dwDelta > 0)
						{
							pOldHead = g_pUnk_4A4490;
							pNew = pNode;
							pNew->m_pUnk0C = 0;
							g_pUnk_4A4490 = pNew;
							pNew->m_pUnk08 = pOldHead;
							if (pOldHead != 0)
								pOldHead->m_pUnk0C = pNew;
							pNew->m_pUnk04 = pSrcB;
							pNew->m_dwUnk14 = (DWORD)nW54;
							pNew->m_pUnk00 = pObj;
							pNew->m_dwUnk10 = 0;
							pNode++;
						}
					}
				}
			}
			g_pUnk_4A1D60 = (BYTE*)pNode;
			pObj = pObj->m_pUnk28;
			if (pObj == 0)
				break;
		}
	}
	// turn the edge list into the objects' edge pointers
	pScan = g_pUnk_4A4490;
	while (pScan != 0)
	{
		pTarget = pScan->m_pUnk00;
		if (pTarget->m_dwUnk18 != 0)
		{
			pPrev = pScan->m_pUnk0C;
			pScan = pScan->m_pUnk08;
			if (pPrev != 0)
				pPrev->m_pUnk08 = pScan;
			else
				g_pUnk_4A4490 = pScan;
			if (pScan == 0)
				break;
			pScan->m_pUnk0C = pPrev;
			continue;
		}
		if (pScan->m_dwUnk10 != 0)
		{
			pTarget->m_pUnk40 = (RENDEROBJ*)-1;
			pTarget->m_dwUnk48 = (DWORD)pScan->m_pUnk04;
			pTarget->m_dwUnk8C = 1;
			if ((int)pScan->m_dwUnk14 > (int)g_dwUnk_4A4544)
			{
				pTarget->m_dwUnk44 = 1;
				pTarget->m_pUnk84 = &g_dwUnk_4A4568;
			}
			else
			{
				pTarget->m_pUnk84 = &g_dwUnk_4A4530;
				pTarget->m_dwUnk44 = 0;
			}
		}
		else
		{
			pTarget->m_pUnk4C = (RENDEROBJ*)-1;
			pTarget->m_dwUnk54 = (DWORD)pScan->m_pUnk04;
			pTarget->m_dwUnk8C = 1;
			if ((int)pScan->m_dwUnk14 > (int)g_dwUnk_4A4544)
			{
				pTarget->m_dwUnk50 = 1;
				pTarget->m_pUnk88 = &g_dwUnk_4A4568;
			}
			else
			{
				pTarget->m_pUnk88 = &g_dwUnk_4A4530;
				pTarget->m_dwUnk50 = 0;
			}
		}
		pScan = pScan->m_pUnk08;
	}
	// copy the static list into the active list links and re-sort it
	pObj2 = g_pUnk_4A471C;
	if (pObj2->m_pUnk34 != 0)
	{
		do
		{
			pNext2 = pObj2->m_pUnk34;
			pPrev2 = pObj2->m_pUnk30;
			pObj2->m_pUnk38 = pNext2;
			pObj2->m_pUnk3C = pPrev2;
			pObj2 = pNext2;
		} while (pNext2->m_pUnk34 != 0);
	}
	pPrev2 = pObj2->m_pUnk30;
	pObj2->m_pUnk3C = pPrev2;
	pObj2->m_pUnk38 = 0;
	if (pPrev2 != 0)
	{
		do
		{
			if (pObj2->m_dwUnk8C != 0)
				Hover_sub_406A80(pObj2);
			pPrev2 = pObj2->m_pUnk3C;
			if (pPrev2 != 0)
				pObj2 = pPrev2;
		} while (pObj2->m_pUnk3C != 0);
	}
	pObj2->m_pUnk3C = 0;
	g_pUnk_4A4720 = pObj2;
	m_dwUnk28 = 0;
}

/////////////////////////////////////////////////////////////////////////////
// CSub403850

// hover.exe:0x403850
// FUNCTION: HOVER 0x403850
DWORD* CSub403850::Sub_403850(DWORD dwY)
{
	DWORD dwOff;

	dwOff = dwY - 0x15E;
	if ((int)dwOff < 1)
		dwOff = 1;
	dwOff &= 0xFFFFFFC0;
	if ((int)dwOff > 0xFC0)
		return (DWORD*)((BYTE*)m_pUnk60 + 0x3F00);
	return (DWORD*)((BYTE*)m_pUnk60 + dwOff * 4);
}

/////////////////////////////////////////////////////////////////////////////
// CSub403980

// Find-or-create the span-run node for one (scanline, texture) pair and
// append the x span [dwC, dwD] to it. The node is keyed on
// (dwA << 0x12) + dwB, so every run in one node shares the scanline dwA and
// the texture dwB; Sub_403A60 later passes that texture to Sub_402B10.

// hover.exe:0x403980
// FUNCTION: HOVER 0x403980
void CSub403980::Sub_403980(DWORD dwA, DWORD dwB, DWORD dwC, DWORD dwD)
{
	SPANNODE* pNode;
	BYTE* pEnd;
	BYTE* pCur;
	DWORD dwKey;
	DWORD dwCount;
	WORD* pw;
	WORD** ppw;

	pNode = m_pUnk0C;
	dwKey = (dwA << 0x12) + dwB;
	while (pNode != 0)
	{
		if (pNode->m_dwUnk20 == dwKey)
			break;
		pNode = pNode->m_pUnk28;
	}
	if (pNode == 0)
	{
		// allocate a new node (header + buffers) from the heap
		if (m_pUnk10 != 0)
			m_pUnk10->m_pUnk28 = (SPANNODE*)m_pUnk1C;
		else
			m_pUnk0C = (SPANNODE*)m_pUnk1C;
		m_pUnk10 = pNode = (SPANNODE*)m_pUnk1C;
		pCur = (BYTE*)pNode + 0x2C;
		m_pUnk1C = pCur;
		pNode->m_pUnk00 = (WORD*)pCur;
		pCur += 0xA0;
		m_pUnk1C = pCur;
		pNode->m_pUnk04 = (WORD*)pCur;
		pEnd = pCur + m_dwUnk00 * 2 + 0xA0;
		pCur += 0xA0;
		m_pUnk1C = pCur;
		pNode->m_pUnk10 = (WORD*)pCur;
		m_pUnk1C = pEnd;
		pNode->m_pUnk28 = 0;
		pNode->m_dwUnk14 = 0;
		pNode->m_dwUnk18 = 0;
		pNode->m_pUnk08 = &pNode->m_pUnk00;
		pNode->m_dwUnk1C = dwA;
		pNode->m_pUnk0C = &pNode->m_pUnk04;
		pNode->m_dwUnk24 = dwB;
		pNode->m_dwUnk20 = dwKey;
		m_dwUnk14++;
		if (m_pUnk18 + 0x16B48 < pEnd)
			OutputDebugStringA("Blowing floor heap");	// hover.exe:0x4C55F4
	}
	dwCount = pNode->m_dwUnk14;
	if ((int)dwCount < 0x28)
	{
		ppw = pNode->m_pUnk0C;
		pNode->m_dwUnk14 = ++dwCount;
		pw = *ppw + (dwCount - 1) * 2;
		pw[0] = (WORD)dwC;
		pw[1] = (WORD)dwD;
	}
}

// One node's merge pass, written once because the original carries two
// compiled copies of it: the first for the current node (which emits its
// runs through Sub_406740) and the second for every node of the span-run
// list (Sub_402B10).  EMIT names the drawer call.
//
// The new spans of the node are copied into the merge scratch buffer, then
// walked in lockstep with the spans accumulated by the previous pass: where
// an old span is uncovered its index words are handed to the drawer, where a
// new span is uncovered the index words are stamped with the current column.
#define HOVER_SPANMERGE_NODE(EMIT)													\
	pNew = m_pUnk04;																\
	pSrc = *pNode->m_pUnk0C;														\
	pDst = pNew;																	\
	dwCount = pNode->m_dwUnk14;														\
	if ((int)dwCount > 0)															\
	{																				\
		do																			\
		{																			\
			pDst[0] = pSrc[0];														\
			pDst[1] = pSrc[1];														\
			pDst += 2;																\
			pSrc += 2;																\
			dwCount--;																\
		} while (dwCount != 0);														\
	}																				\
	dwStep = g_dwUnk_4A453C;														\
	nNew = 0;																		\
	nOld = 0;																		\
	pOld = *pNode->m_pUnk08;														\
	if ((int)pNode->m_dwUnk18 > 0)													\
	{																				\
		do																			\
		{																			\
			int nOldX0;																\
			volatile int nOldX1;													\
			int nNewX0;																\
			volatile int nNewX1;													\
																					\
			if ((int)pNode->m_dwUnk14 <= (int)nNew)									\
				break;																\
			nNewX0 = (short)pNew[0];												\
			nOldX0 = (short)pOld[0];												\
			if (nNewX0 > nOldX0)													\
			{																		\
				nOldX1 = (short)pOld[1];											\
				if (nNewX0 > nOldX1)												\
				{																	\
					if (nOldX1 >= nOldX0)											\
					{																\
						do															\
						{															\
							EMIT(nOldX0);											\
							dwStep = g_dwUnk_4A453C;								\
							nOldX0 += (int)dwStep;									\
						} while (nOldX1 >= nOldX0);									\
					}																\
					pOld += 2;														\
					nOld++;															\
				}																	\
				else																\
				{																	\
					nNewX1 = (short)pNew[1];										\
					if (nNewX1 >= nOldX1)											\
					{																\
						if (nNewX0 > nOldX0)										\
						{															\
							do														\
							{														\
								EMIT(nOldX0);										\
								dwStep = g_dwUnk_4A453C;							\
								nOldX0 += (int)dwStep;								\
							} while (nNewX0 > nOldX0);								\
						}															\
						pNew[0] = (WORD)(nOldX1 +									\
							(int)dwStep);											\
						pOld += 2;													\
						nOld++;														\
					}																\
					else															\
					{																\
						if (nNewX0 > nOldX0)										\
						{															\
							do														\
							{														\
								EMIT(nOldX0);										\
								dwStep = g_dwUnk_4A453C;							\
								nOldX0 += (int)dwStep;								\
							} while (nNewX0 > nOldX0);								\
						}															\
						pOld[0] = (WORD)(nNewX1 +									\
							(int)dwStep);											\
						pNew += 2;													\
						nNew++;														\
					}																\
				}																	\
			}																		\
			else																	\
			{																		\
				nNewX1 = (short)pNew[1];											\
				if (nNewX1 < nOldX0)												\
				{																	\
					if (nNewX1 >= nNewX0)											\
					{																\
						pIdx = pNode->m_pUnk10 + nNewX0;							\
						do															\
						{															\
							*pIdx = *(WORD*)&g_dwUnk_4A4534;						\
							nNewX0 += (int)dwStep;									\
							pIdx += dwStep;											\
						} while (nNewX1 >= nNewX0);									\
					}																\
					pNew += 2;														\
					nNew++;															\
				}																	\
				else																\
				{																	\
					nOldX1 = (short)pOld[1];										\
					if (nNewX1 <= nOldX1)											\
					{																\
						if (nNewX0 < nOldX0)										\
						{															\
							pIdx = pNode->m_pUnk10 + nNewX0;						\
							do														\
							{														\
								*pIdx = *(WORD*)&g_dwUnk_4A4534;					\
								nNewX0 += (int)dwStep;								\
								pIdx += dwStep;										\
							} while (nOldX0 > nNewX0);								\
						}															\
						pOld[0] = (WORD)(nNewX1 +									\
							(int)dwStep);											\
						pNew += 2;													\
						nNew++;														\
					}																\
					else															\
					{																\
						if (nNewX0 < nOldX0)										\
						{															\
							pIdx = pNode->m_pUnk10 + nNewX0;						\
							do														\
							{														\
								*pIdx = *(WORD*)&g_dwUnk_4A4534;					\
								nNewX0 += (int)dwStep;								\
								pIdx += dwStep;										\
							} while (nOldX0 > nNewX0);								\
						}															\
						pNew[0] = (WORD)(nOldX1 +									\
							(int)dwStep);											\
						pOld += 2;													\
						nOld++;														\
					}																\
				}																	\
			}																		\
		} while ((int)pNode->m_dwUnk18 > (int)nOld);								\
	}																				\
	if ((int)nOld < (int)pNode->m_dwUnk18)											\
	{																				\
		i = nOld;																	\
		do																			\
		{																			\
			pOld = *pNode->m_pUnk08 + i * 2;										\
			nX0 = (short)pOld[0];													\
			nX1 = (short)pOld[1];													\
			if (nX1 >= nX0)															\
			{																		\
				do																	\
				{																	\
					EMIT(nX0);														\
					dwStep = g_dwUnk_4A453C;										\
					nX0 += (int)dwStep;												\
					pOld = *pNode->m_pUnk08 + i * 2;								\
					nX1 = (short)pOld[1];											\
				} while (nX1 >= nX0);												\
			}																		\
			i++;																	\
		} while ((int)pNode->m_dwUnk18 > (int)i);									\
	}																				\
	if ((int)nNew < (int)pNode->m_dwUnk14)											\
	{																				\
		pDst = m_pUnk04 + nNew * 2 + 1;												\
		dwCount = pNode->m_dwUnk14 - nNew;											\
		do																			\
		{																			\
			nX0 = (short)pDst[-1];													\
			nX1 = (short)pDst[0];													\
			if (nX0 <= nX1)															\
			{																		\
				pIdx = pNode->m_pUnk10 + nX0;										\
				do																	\
				{																	\
					*pIdx = *(WORD*)&g_dwUnk_4A4534;								\
					nX0 += (int)dwStep;												\
					pIdx += dwStep;													\
				} while (nX0 <= nX1);												\
			}																		\
			pDst += 2;																\
			dwCount--;																\
		} while (dwCount != 0);														\
	}																				\
	ppA = pNode->m_pUnk0C;															\
	ppB = pNode->m_pUnk08;															\
	dwCount = pNode->m_dwUnk14;														\
	pNode->m_pUnk0C = ppB;															\
	pNode->m_pUnk08 = ppA;															\
	pNode->m_dwUnk18 = dwCount;														\
	pNode->m_dwUnk14 = 0

// The flat (untextured) drawer of the current node's runs
#define HOVER_SPANEMIT_FLAT(nX)										\
	pRender->Sub_406740((DWORD)(short)pNode->m_pUnk10[nX], (DWORD)(nX))

// The textured drawer of a span-run node: the run carries its own scanline
// (m_dwUnk1C) and CMerlinTexture (m_dwUnk24)
#define HOVER_SPANEMIT_TEX(nX)										\
	pRender->Sub_402B10((DWORD)(short)pNode->m_pUnk10[nX], (DWORD)(nX),	\
		(CMerlinTexture*)pNode->m_dwUnk24, pNode->m_dwUnk1C)

// hover.exe:0x403A60
// FUNCTION: HOVER 0x403a60
void CSub403980::Sub_403A60(CSub402E00* pRender)
{
	SPANNODE* pNode;
	WORD* pNew;
	WORD* pDst;
	WORD* pSrc;
	WORD* pOld;
	WORD* pIdx;
	WORD** ppA;
	WORD** ppB;
	DWORD nNew;
	DWORD nOld;
	DWORD dwStep;
	DWORD dwCount;
	DWORD i;
	int nX0;
	int nX1;

	pNode = m_pUnk08;
	HOVER_SPANMERGE_NODE(HOVER_SPANEMIT_FLAT);
	pNode = m_pUnk0C;
	if (pNode != 0)
	{
		do
		{
			HOVER_SPANMERGE_NODE(HOVER_SPANEMIT_TEX);
			pNode = pNode->m_pUnk28;
		} while (pNode != 0);
	}
}

// hover.exe:0x404030
// Walk the render-object list from pObj and paint the horizontal run
// [dwX, dwXMax]: for each object the front face, the single-span case and the
// multi-span case each clip against the run, fill the gap ahead of the span by
// recursing into the next object (or the span heap when there is none) and
// then hand the span itself to the matching emitter.  dwX1 is deliberately
// left uninitialised: the original reads its frame slot on the path where an
// object has no spans at all.  The step is held in a local so that the pixel
// pitch stays in one place across the three clip arms.
// FUNCTION: HOVER 0x404030
void CSub402E00::Sub_404030(RENDEROBJ* pObj, DWORD dwX, DWORD dwXMax)
{
	SPANENTRY* pSpan;
	DWORD dwX0;
	DWORD dwX1;
	DWORD i;
	DWORD dwStep;			// g_dwUnk_4A453C, the one-pixel step

	dwStep = g_dwUnk_4A453C;
	for (;;)
	{
		if (pObj->m_dwUnk58 == 0)
			goto L_advance;
		if (pObj->m_dwUnk8C != 0)
		{
			dwX0 = pObj->m_dwUnk74;
			dwX1 = pObj->m_dwUnk78;
			if ((int)dwX0 > (int)dwXMax)
				goto L_advance;
			if ((int)dwX1 < (int)dwX)
				goto L_advance;
			if ((int)dwX0 > (int)dwX)
			{
				if (pObj->m_pUnk38 != 0)
					Sub_404030(pObj->m_pUnk38, dwX, dwX0 - dwStep);
				else
					m_pUnk00->Sub_406D00(dwX, dwX0 - dwStep);
				if ((int)dwX1 >= (int)dwXMax)
				{
					Sub_402630(pObj, dwX0, dwXMax);
					return;
				}
				Sub_402630(pObj, dwX0, dwX1);
				dwX = dwX1 + dwStep;
				if ((int)dwX1 >= (int)dwX)
					return;
				goto L_advance;
			}
			if ((int)dwX1 < (int)dwX)
				goto L_advance;
			if ((int)dwX1 > (int)dwXMax)
			{
				Sub_402630(pObj, dwX, dwXMax);
				return;
			}
			Sub_402630(pObj, dwX, dwX1);
			dwX = dwX1 + dwStep;
			if ((int)dwX1 >= (int)dwX)
				return;
			goto L_advance;
		}
		if (pObj->m_dwUnkA0 == 0)
		{
			dwX0 = pObj->m_acUnkAC.m_dwUnk08;
			dwX1 = pObj->m_acUnkAC.m_dwUnk0C;
			pSpan = &pObj->m_acUnkAC;
			if ((int)dwX0 > (int)dwXMax)
				goto L_advance;
			if ((int)dwX1 < (int)dwX)
				goto L_advance;
			if ((int)dwX0 > (int)dwX)
			{
				if (pObj->m_pUnk38 != 0)
					Sub_404030(pObj->m_pUnk38, dwX, dwX0 - dwStep);
				else
					m_pUnk00->Sub_406D00(dwX, dwX0 - dwStep);
				if ((int)dwX1 >= (int)dwXMax)
				{
					Sub_402990(pObj, pSpan, dwX0, dwXMax);
					return;
				}
				Sub_402990(pObj, pSpan, dwX0, dwX1);
				dwX = dwX1 + dwStep;
				if ((int)dwX1 >= (int)dwX)
					return;
				goto L_advance;
			}
			if ((int)dwX1 < (int)dwX)
				goto L_advance;
			if ((int)dwX1 > (int)dwXMax)
			{
				Sub_402990(pObj, pSpan, dwX, dwXMax);
				return;
			}
			Sub_402990(pObj, pSpan, dwX, dwX1);
			dwX = dwX1 + dwStep;
			if ((int)dwX1 >= (int)dwX)
				return;
			goto L_advance;
		}
		if ((int)pObj->m_dwUnk74 > (int)dwXMax)
			goto L_advance;
		if ((int)pObj->m_dwUnk78 < (int)dwX)
			goto L_advance;
		i = 0;
		pSpan = pObj->m_pUnkA8;
		if ((int)pObj->m_dwUnk90 > 0)
		{
			do
			{
				dwX0 = pSpan->m_dwUnk08;
				dwX1 = pSpan->m_dwUnk0C;
				if ((int)dwX0 > (int)dwX)
				{
					if ((int)dwX0 > (int)dwXMax)
						goto L_advance;
					if (pObj->m_pUnk38 != 0)
						Sub_404030(pObj->m_pUnk38, dwX,
							dwX0 - dwStep);
					else
						m_pUnk00->Sub_406D00(dwX, dwX0 - dwStep);
					if ((int)dwX1 >= (int)dwXMax)
					{
						Sub_4066B0(pObj, pSpan, dwX0, dwXMax);
						return;
					}
					Sub_4066B0(pObj, pSpan, dwX0, dwX1);
					dwX = dwX1 + dwStep;
				}
				else if ((int)dwX1 >= (int)dwX)
				{
					if ((int)dwX1 > (int)dwXMax)
					{
						Sub_4066B0(pObj, pSpan, dwX, dwXMax);
						return;
					}
					Sub_4066B0(pObj, pSpan, dwX, dwX1);
					dwX = dwX1 + dwStep;
				}
				pSpan++;
				i++;
			} while ((int)pObj->m_dwUnk90 > (int)i);
		}
		if ((int)dwX1 >= (int)dwX)
			return;
	L_advance:
		pObj = pObj->m_pUnk38;
		if (pObj == 0)
		{
			m_pUnk00->Sub_406D00(dwX, dwXMax);
			return;
		}
	}
}

// hover.exe:0x4065F0
// FUNCTION: HOVER 0x4065f0
int CSub402E00::Sub_4065F0(RENDEROBJ* pObj, SPANENTRY* pSpan, DWORD dwC,
	DWORD dwD, DWORD dwE)
{
	DWORD dwV;

	dwV = *pObj->m_pUnk88;
	if ((int)dwD < (int)dwV)
	{
		if ((int)dwD >= (int)g_dwUnk_4A4564)
			return 0;
		if ((int)dwD <= (int)g_dwUnk_4A4524)
		{
			m_pUnk00->Sub_403980(dwE, pObj->m_dwUnk54, g_dwUnk_4A4524,
				g_dwUnk_4A4564);
			return 1;
		}
		m_pUnk00->Sub_403980(dwE, pObj->m_dwUnk54, dwD, g_dwUnk_4A4564);
		g_dwUnk_4A4564 = dwD - g_dwUnk_4A453C;
		return 0;
	}
	if ((int)g_dwUnk_4A4564 <= (int)dwV)
		return 0;
	if ((int)g_dwUnk_4A4524 >= (int)dwV)
	{
		m_pUnk00->Sub_403980(dwE, pObj->m_dwUnk54, g_dwUnk_4A4524,
			g_dwUnk_4A4564);
		return 1;
	}
	m_pUnk00->Sub_403980(dwE, pObj->m_dwUnk54, dwV, g_dwUnk_4A4564);
	g_dwUnk_4A4564 = dwV - g_dwUnk_4A453C;
	return 0;
}

// hover.exe:0x4066B0
// FUNCTION: HOVER 0x4066b0
void CSub402E00::Sub_4066B0(RENDEROBJ* pObj, SPANENTRY* pSpan, DWORD dwA,
	DWORD dwB)
{
	DWORD dwY;

	g_dwUnk_4A4A14 = dwB + (g_dwUnk_4A453C - dwA);
	if ((int)g_dwUnk_4A4A14 < 1)
		return;
	g_dwUnk_4A4A4C = g_dwUnk_4A4540 + dwA * g_dwUnk_4A4A34;
	dwY = (DWORD)(short)pObj->m_wUnk08;
	g_dwUnk_4A4A44 = dwY;
	g_dwUnk_4A4A38 = pObj->m_dwUnk94;
	g_dwUnk_4A4A50 = pObj->m_dwUnk98;
	g_dwUnk_4A4A40 = (dwA - pSpan->m_dwUnk08) * pObj->m_dwUnk98;
	g_dwUnk_4A4A1C = pSpan->m_dwUnk10;
	g_dwUnk_4A4A3C = (DWORD)g_pUnk_4A1D68->Sub_403850(dwY);
	g_pUnk_4A1D68->m_pfnUnk00();
}

// hover.exe:0x406740
// FUNCTION: HOVER 0x406740
void CSub402E00::Sub_406740(DWORD dwA, DWORD dwB)
{
	CSub403850* pView;
	MERLIN_TEXENTRY* pEntry;
	BYTE* pDst;
	BYTE* p;
	DWORD dwStep;
	DWORD dwFill;
	DWORD dwPat;
	DWORD dwV;
	DWORD dwS;
	DWORD dwBX;
	DWORD dwW;
	DWORD dwCount;
	DWORD dwT10;
	DWORD dwT14;
	DWORD dwV2;
	DWORD dwV3;
	DWORD dwV4;
	DWORD n;
	BYTE bPix;
	BYTE cl;

	dwStep = g_dwUnk_4A453C;
	pView = g_pUnk_4A1D68;
	dwB &= 0 - g_dwUnk_4A453C;
	if (pView->m_dwUnk74 == 1)
	{
		// flat fill: dwStep rows of the same color byte
		pDst = (BYTE*)(dwB * g_dwUnk_4A4A34 + pView->m_dwUnk28) + dwA;
		g_dwUnk_4A4A4C = (DWORD)pDst;
		if ((int)dwStep <= 0)
			return;
		bPix = pView->m_pUnk6C[dwB * 4];
		dwFill = g_dwUnk_4A4534 - dwA;
		do
		{
			dwPat = (DWORD)bPix * 0x01010101;
			p = pDst;
			for (n = dwFill >> 2; n != 0; n--)
			{
				*(DWORD*)p = dwPat;
				p += 4;
			}
			for (n = dwFill & 3; n != 0; n--)
			{
				*p = (BYTE)dwPat;
				p++;
			}
			pDst += g_dwUnk_4A4A34;
			dwStep--;
		} while (dwStep != 0);
		return;
	}
	dwS = (DWORD)(1 << (BYTE)g_dwUnk_461CFC);
	dwV = (DWORD)(0x280 / (int)dwS);
	dwBX = dwV * 6;
	dwV = (dwV << 0x10) / g_dwUnk_4A454C;
	cl = (BYTE)g_dwUnk_461CFC;
	g_dwUnk_4A4A50 = dwV;
	pEntry = &pView->m_pUnk70->m_pUnk28[g_dwUnk_461CFC];
	g_dwUnk_4A4A20 = pEntry->m_dwUnk04;
	dwV = (DWORD)(0xF0 / (int)dwS);
	dwV = (dwV << 0x10) / g_dwUnk_4A1D70;
	dwS = g_dwUnk_4A1D70 - dwB;
	dwW = g_dwUnk_4A4534;
	dwV = (DWORD)((int)(dwV * dwS) >> 0x10);
	dwCount = dwW - dwA;
	dwV &= pEntry->m_dwUnk00;
	dwV = dwV * pEntry->m_dwUnk0C + pEntry->m_dwUnk18;
	g_dwUnk_4A4A1C = dwV;
	if (pView->m_dwUnk68 != 0)
	{
		dwV2 = dwB * g_dwUnk_4A4A34 + pView->m_dwUnk28 - dwW + g_dwUnk_4A454C -
			1;
		dwW = dwW - g_dwUnk_4A454C + 1;
		g_dwUnk_4A4A4C = dwV2;
		dwV3 = pView->m_dwUnk54;
		// The mirrored start column: the run's own left edge dwA folded
		// through the viewport, NOT the texture offset computed above
		// (hover.exe:0x406878 re-reads the dwA argument here).
		dwV = dwA - g_dwUnk_4A454C + 1;
		dwW *= g_dwUnk_4A4A50;
		dwV3 += 0x55;
		dwV *= g_dwUnk_4A4A50;
		dwV3 &= 0x1FF;
		dwV3 = (dwV3 * dwBX) << 7;
		dwV += dwV3;
		dwV2 = dwW + dwV3;
		if ((int)dwV < 0)
			dwV2 += dwBX << 0x10;
		dwBX <<= 0x10;
		if ((int)dwBX <= (int)dwV2)
		{
			dwV2 -= dwBX;
			cl = (BYTE)g_dwUnk_4A4554;
			dwT10 = (DWORD)((int)dwV2 / (int)g_dwUnk_4A4A50);
			dwCount -= dwT10;
			dwCount = (DWORD)((int)dwCount >> cl);
			dwT14 = g_dwUnk_4A4A50;
			g_dwUnk_4A4A50 <<= cl;
			g_dwUnk_4A4A50 = 0 - g_dwUnk_4A4A50;
			g_dwUnk_4A4A18 = dwV2;
			g_dwUnk_4A4A14 = (DWORD)(((int)dwT10 >> cl) + 1);
			pView->m_pfnUnk04();
			dwW = g_dwUnk_4A455C;
			g_dwUnk_4A4A14 = dwCount;
			dwV4 = (dwW - 1) & dwT10;
			if (dwV4 != 0)
			{
				g_dwUnk_4A4A4C += dwT10 - dwV4 + dwW;
				g_dwUnk_4A4A18 = (dwV4 - dwW) * dwT14 + dwBX;
			}
			else
			{
				g_dwUnk_4A4A18 = dwBX;
				g_dwUnk_4A4A4C += dwT10;
			}
			g_pUnk_4A1D68->m_pfnUnk04();
			return;
		}
		cl = (BYTE)g_dwUnk_4A4554;
		g_dwUnk_4A4A18 = dwV2;
		dwCount = (DWORD)((int)dwCount >> cl);
		g_dwUnk_4A4A50 <<= cl;
		g_dwUnk_4A4A14 = dwCount;
		g_dwUnk_4A4A50 = 0 - g_dwUnk_4A4A50;
		pView->m_pfnUnk04();
		return;
	}
	g_dwUnk_4A4A4C = dwB * g_dwUnk_4A4A34 + pView->m_dwUnk28 + dwA;
	dwV3 = pView->m_dwUnk54 & 0x1FF;
	dwW *= g_dwUnk_4A4A50;
	dwV3 = (dwV3 * dwBX) << 7;
	dwV = dwA * g_dwUnk_4A4A50;
	dwV2 = dwV + dwV3;
	dwV3 += dwW;
	dwBX <<= 0x10;
	if ((int)dwBX < (int)dwV2)
	{
		dwV2 -= dwBX;
		dwV3 -= dwBX;
	}
	if ((int)dwBX <= (int)dwV3)
	{
		dwV = dwBX - dwV2;
		cl = (BYTE)g_dwUnk_4A4554;
		dwT10 = (DWORD)((int)dwV / (int)g_dwUnk_4A4A50);
		dwCount -= dwT10;
		dwV4 = dwT10;
		dwCount = (DWORD)((int)dwCount >> cl);
		dwT14 = g_dwUnk_4A4A50;
		g_dwUnk_4A4A18 = dwV2;
		g_dwUnk_4A4A50 <<= cl;
		g_dwUnk_4A4A14 = (DWORD)(((int)dwV4 >> cl) + 1);
		pView->m_pfnUnk04();
		dwW = g_dwUnk_4A455C;
		g_dwUnk_4A4A14 = dwCount;
		dwV = (dwW - 1) & dwV4;
		if (dwV != 0)
		{
			dwV4 = dwV4 - dwV + dwW;
			// Not the mirror branch's (dwV - dwW): here g_dwUnk_4A4A50 is
			// still positive, and hover.exe:0x406a40 subtracts the other
			// way round.
			dwV = (dwW - dwV) * dwT14;
			g_dwUnk_4A4A4C += dwV4;
			g_dwUnk_4A4A18 = dwV;
		}
		else
		{
			g_dwUnk_4A4A18 = 0;
			g_dwUnk_4A4A4C += dwV4;
		}
		g_pUnk_4A1D68->m_pfnUnk04();
		return;
	}
	cl = (BYTE)g_dwUnk_4A4554;
	g_dwUnk_4A4A18 = dwV2;
	dwCount = (DWORD)((int)dwCount >> cl);
	g_dwUnk_4A4A50 <<= cl;
	g_dwUnk_4A4A14 = dwCount;
	pView->m_pfnUnk04();
}

// hover.exe:0x406A80
// FUNCTION: HOVER 0x406a80
void Hover_sub_406A80(RENDEROBJ* pObj)
{
	RENDEROBJ* pStop;
	RENDEROBJ* pCur;
	RENDEROBJ* pNext;
	RENDEROBJ* pPrev;
	RENDEROBJ* pOld;

	pStop = pObj->m_pUnk40;
	if (pStop != 0)
	{
		if (pStop == (RENDEROBJ*)-1)
			pStop = 0;
		if (pObj->m_pUnk3C != pStop)
		{
			pCur = pObj;
			do
			{
				pCur = pCur->m_pUnk3C;
				if (pCur == 0)
					break;
				if ((int)pObj->m_dwUnk6C > 0)
				{
					if (pCur->m_dwUnk8C != 0)
						Hover_sub_406A80(pCur);
					if ((int)pObj->m_dwUnk60 >= (int)pCur->m_dwUnk60)
					{
						pNext = pCur->m_pUnk3C;
						pPrev = pCur->m_pUnk38;
						pOld = pObj->m_pUnk38;
						if (pNext != 0)
							pNext->m_pUnk38 = pPrev;
						if (pPrev != 0)
							pPrev->m_pUnk3C = pNext;
						pObj->m_pUnk38 = pCur;
						if (pOld != 0)
							pOld->m_pUnk3C = pCur;
						pCur->m_pUnk3C = pObj;
						pCur->m_pUnk38 = pOld;
						pCur = pPrev;
					}
				}
				else
				{
					if (pCur->m_dwUnk8C != 0)
						Hover_sub_406A80(pCur);
					if ((int)pObj->m_dwUnk60 <= (int)pCur->m_dwUnk64)
					{
						pNext = pCur->m_pUnk3C;
						pPrev = pCur->m_pUnk38;
						pOld = pObj->m_pUnk38;
						if (pNext != 0)
							pNext->m_pUnk38 = pPrev;
						if (pPrev != 0)
							pPrev->m_pUnk3C = pNext;
						pObj->m_pUnk38 = pCur;
						if (pOld != 0)
							pOld->m_pUnk3C = pCur;
						pCur->m_pUnk3C = pObj;
						pCur->m_pUnk38 = pOld;
						pCur = pPrev;
					}
				}
			} while (pCur->m_pUnk3C != pStop);
		}
	}
	pStop = pObj->m_pUnk4C;
	if (pStop != 0)
	{
		if (pStop == (RENDEROBJ*)-1)
			pStop = 0;
		if (pObj->m_pUnk3C != pStop)
		{
			pCur = pObj;
			do
			{
				pCur = pCur->m_pUnk3C;
				if (pCur == 0)
					break;
				if ((int)pObj->m_dwUnk70 > 0)
				{
					if (pCur->m_dwUnk8C != 0)
						Hover_sub_406A80(pCur);
					if ((int)pCur->m_dwUnk60 <= (int)pObj->m_dwUnk64)
					{
						pNext = pCur->m_pUnk3C;
						pPrev = pCur->m_pUnk38;
						pOld = pObj->m_pUnk38;
						if (pNext != 0)
							pNext->m_pUnk38 = pPrev;
						if (pPrev != 0)
							pPrev->m_pUnk3C = pNext;
						pObj->m_pUnk38 = pCur;
						if (pOld != 0)
							pOld->m_pUnk3C = pCur;
						pCur->m_pUnk3C = pObj;
						pCur->m_pUnk38 = pOld;
						pCur = pPrev;
					}
				}
				else
				{
					if (pCur->m_dwUnk8C != 0)
						Hover_sub_406A80(pCur);
					if ((int)pCur->m_dwUnk64 >= (int)pObj->m_dwUnk64)
					{
						pNext = pCur->m_pUnk3C;
						pPrev = pCur->m_pUnk38;
						pOld = pObj->m_pUnk38;
						if (pNext != 0)
							pNext->m_pUnk38 = pPrev;
						if (pPrev != 0)
							pPrev->m_pUnk3C = pNext;
						pObj->m_pUnk38 = pCur;
						if (pOld != 0)
							pOld->m_pUnk3C = pCur;
						pCur->m_pUnk3C = pObj;
						pCur->m_pUnk38 = pOld;
						pCur = pPrev;
					}
				}
			} while (pCur->m_pUnk3C != pStop);
		}
	}
}

// hover.exe:0x406D00
// FUNCTION: HOVER 0x406d00
void CSub403980::Sub_406D00(DWORD dwA, DWORD dwB)
{
	SPANNODE* pNode;
	WORD* pw;
	WORD** ppw;
	DWORD dwCount;

	pNode = m_pUnk08;
	dwCount = pNode->m_dwUnk14;
	if ((int)dwCount < 0x28)
	{
		ppw = pNode->m_pUnk0C;
		pNode->m_dwUnk14 = ++dwCount;
		pw = *ppw + (dwCount - 1) * 2;
		pw[1] = (WORD)dwB;
		pw[0] = (WORD)dwA;
	}
}
