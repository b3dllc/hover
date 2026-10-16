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
BYTE* g_pArenaCursor;			// span arena cursor

// hover.exe:0x4A1D64
// GLOBAL: HOVER 0x4a1d64
DWORD g_dwColTopY;			// span window start

// hover.exe:0x4A1D68
// GLOBAL: HOVER 0x4a1d68
CRenderContext* g_pRenderCtx;		// render context in use

// hover.exe:0x4A1D6C
// GLOBAL: HOVER 0x4a1d6c
DWORD g_dwColProjScale;			// span scale numerator

// hover.exe:0x4A1D70
// GLOBAL: HOVER 0x4a1d70
DWORD g_dwHorizonY;			// span origin offset

// hover.exe:0x4A1D74
// GLOBAL: HOVER 0x4a1d74
DWORD g_dwFlatMipLevel;			// texture mip level

// hover.exe:0x4A1D78
// GLOBAL: HOVER 0x4a1d78
DWORD g_bWallMipMapping;			// angle->mip LUT enable

// hover.exe:0x4A1D80
// GLOBAL: HOVER 0x4a1d80
BYTE g_abSpanArena[SPAN_ARENA_SIZE];	// span arena

// hover.exe:0x4A4490
// GLOBAL: HOVER 0x4a4490
EDGENODE* g_pEdgeList;		// edge list head

// hover.exe:0x4A4498
// GLOBAL: HOVER 0x4a4498
CMerlinLine g_lineFrustumLeft;	// phase camera line (CSpanRasterizer::RenderWorld)

// hover.exe:0x4A44E0
// GLOBAL: HOVER 0x4a44e0
CMerlinLine g_lineFrustumRight;	// right-edge phase camera line

// hover.exe:0x4A4524
// GLOBAL: HOVER 0x4a4524
DWORD g_dwClipTopY;			// clip window start

// hover.exe:0x4A4528
// GLOBAL: HOVER 0x4a4528
WORD g_wBspOrder;				// reset to 0 by CSpanRasterizer::RenderWorld

// hover.exe:0x4A452C
// GLOBAL: HOVER 0x4a452c
DWORD* g_pdwDistTable;			// scanline scale table

// hover.exe:0x4A4530
// GLOBAL: HOVER 0x4a4530
DWORD g_dwBelowClipY;			// low edge default

// hover.exe:0x4A4534
// GLOBAL: HOVER 0x4a4534
DWORD g_dwCurColumn;			// fill color

// hover.exe:0x4A4538
// GLOBAL: HOVER 0x4a4538
DWORD g_dwLogRowStep;			// log2 of g_dwRowStep

// hover.exe:0x4A453C
// GLOBAL: HOVER 0x4a453c
DWORD g_dwRowStep;			// rows per texel

// hover.exe:0x4A4540
// GLOBAL: HOVER 0x4a4540
DWORD g_dwColumnOrigin;			// frame buffer origin

// hover.exe:0x4A4544
// GLOBAL: HOVER 0x4a4544
DWORD g_dwEyeZ;			// camera (eye) height

// hover.exe:0x4A4548
// GLOBAL: HOVER 0x4a4548
BYTE* g_pbMipLut;			// angle->mip LUT

// hover.exe:0x4A454C
// GLOBAL: HOVER 0x4a454c
DWORD g_dwViewWidth;			// screen width

// hover.exe:0x4A4550
// GLOBAL: HOVER 0x4a4550
SPANLINK* g_pSpanLinkCursor;			// next free node in m_pLinkPool

// hover.exe:0x4A4554
// GLOBAL: HOVER 0x4a4554
DWORD g_dwLogColStep;			// texture shift count (read as a byte by
								// the span drawers; stored as a dword by
								// CSpanRasterizer::RenderWorld)

// hover.exe:0x4A455C
// GLOBAL: HOVER 0x4a455c
DWORD g_dwColStep;			// pixels per texel across

// hover.exe:0x4A4564
// GLOBAL: HOVER 0x4a4564
DWORD g_dwClipBottomY;			// clip window end

// hover.exe:0x4A4568
// GLOBAL: HOVER 0x4a4568
DWORD g_dwAboveClipY;			// high edge default

// hover.exe:0x4A456C
// GLOBAL: HOVER 0x4a456c
DWORD g_dwColBottomY;			// span window end

// hover.exe:0x4A4714
// GLOBAL: HOVER 0x4a4714
RENDEROBJ* g_pObjPool;		// render-object pool list

// hover.exe:0x4A4718
// GLOBAL: HOVER 0x4a4718
RENDEROBJ* g_pObjArenaCursor;		// render-object pool cursor (m_pRenderObjPool)

// hover.exe:0x4A471C
// GLOBAL: HOVER 0x4a471c
RENDEROBJ* g_pColumnObjs;		// static list head

// hover.exe:0x4A4720
// GLOBAL: HOVER 0x4a4720
RENDEROBJ* g_pActiveObjs;		// active list head

// hover.exe:0x4A4A14
// GLOBAL: HOVER 0x4a4a14
DWORD g_dwSpanCount;			// span length

// hover.exe:0x4A4A18
// GLOBAL: HOVER 0x4a4a18
DWORD g_dwSkyU;

// hover.exe:0x4A4A1C
// GLOBAL: HOVER 0x4a4a1c
DWORD g_dwTexels;			// texture pixel pointer

// hover.exe:0x4A4A20
// GLOBAL: HOVER 0x4a4a20
DWORD g_dwUMask;

// hover.exe:0x4A4A24
// GLOBAL: HOVER 0x4a4a24
DWORD g_dwVStep;			// texture v step

// hover.exe:0x4A4A28
// GLOBAL: HOVER 0x4a4a28
DWORD g_dwU;			// texture u origin

// hover.exe:0x4A4A2C
// GLOBAL: HOVER 0x4a4a2c
DWORD g_dwV;			// texture v origin

// hover.exe:0x4A4A30
// GLOBAL: HOVER 0x4a4a30
DWORD g_dwDstRowStride;			// render target width * m_dwRowDir

// hover.exe:0x4A4A34
// GLOBAL: HOVER 0x4a4a34
DWORD g_dwRowPitch;			// frame buffer pitch

// hover.exe:0x4A4A38
// GLOBAL: HOVER 0x4a4a38
DWORD g_dwVMask;

// hover.exe:0x4A4A3C
// GLOBAL: HOVER 0x4a4a3c
DWORD g_dwShadeTable;			// current 256-byte shade row

// hover.exe:0x4A4A40
// GLOBAL: HOVER 0x4a4a40
DWORD g_dwColumnV;

// hover.exe:0x4A4A44
// GLOBAL: HOVER 0x4a4a44
DWORD g_dwSpanDepth;			// span scale

// hover.exe:0x4A4A48
// GLOBAL: HOVER 0x4a4a48
DWORD g_dwVShift;			// drawer selector

// hover.exe:0x4A4A4C
// GLOBAL: HOVER 0x4a4a4c
DWORD g_dwDst;			// destination pointer

// hover.exe:0x4A4A50
// GLOBAL: HOVER 0x4a4a50
DWORD g_dwUStep;			// texture u step (the wall v step)

// hover.exe:0x4C5208
// GLOBAL: HOVER 0x4c5208
DWORD g_dwNoEdgeY = 0xFFF0000;	// edge sentinel (dereferenced through
									// RENDEROBJ::m_pTopEdgeY/m_pBottomEdgeY)

/////////////////////////////////////////////////////////////////////////////
// Helper macros reconstructed from the inline code sequences in the
// original (the binary carries several compiled copies of each)

// Table lookup used by DrawFlatRow: g_adwSinTable is addressed by byte
// offset ((expr) & 0x7FFC) starting one dword into the table
#define HOVER_SINLUT(expr) g_adwSinTable[(((expr) & 0x7FFC) >> 2) + 1]

// LinkFlatEdges: unlink pScan from the g_pEdgeList list and advance to the
// next node (ends the scan loop when the list runs out)
#define HOVER_SPANUNLINK(pScan, pPrev)							\
	pPrev = pScan->m_pPrev;									\
	pScan = pScan->m_pNext;									\
	if (pPrev != 0)												\
		pPrev->m_pNext = pScan;								\
	else														\
		g_pEdgeList = pScan;									\
	if (pScan == 0)												\
		break;													\
	pScan->m_pPrev = pPrev

// LinkFlatEdges: merge the render object behind pScan into pTarget's top
// edge and unlink the node; falls through to the next comparison when the
// two objects are not both merge candidates
#define HOVER_SPANMERGE(pObj, pScan, pPrev, pKey, pEdge)		\
	if (pObj->m_bCovered != 0)									\
	{															\
		HOVER_SPANUNLINK(pScan, pPrev);							\
		continue;												\
	}															\
	if (pTarget->m_bHidden == 0 && pObj->m_bHidden == 0)		\
	{															\
		if (pScan->m_bTopEdge != 0)								\
		{														\
			pTarget->m_pTopNeighbour = pObj;							\
			pTarget->m_pTopFlatTex = (pKey);					\
			pTarget->m_bTopAboveEye = 1;								\
			pTarget->m_bHasFlats = 1;								\
			pTarget->m_pTopEdgeY = (pEdge);						\
		}														\
		else													\
		{														\
			pTarget->m_pBottomNeighbour = pObj;							\
			pTarget->m_pBottomFlatTex = (pKey);					\
			pTarget->m_bBottomAboveEye = 1;								\
			pTarget->m_bHasFlats = 1;								\
			pTarget->m_pBottomEdgeY = (pEdge);						\
		}														\
		HOVER_SPANUNLINK(pScan, pPrev);							\
		continue;												\
	}

// LinkFlatEdges: the same merge for the bottom edge.  The original spells
// this copy out separately: the edge flag is cleared instead of set and
// the five stores go out in a different order.
#define HOVER_SPANMERGE_LO(pObj, pScan, pPrev, pKey, pEdge)		\
	if (pObj->m_bCovered != 0)									\
	{															\
		HOVER_SPANUNLINK(pScan, pPrev);							\
		continue;												\
	}															\
	if (pTarget->m_bHidden == 0 && pObj->m_bHidden == 0)		\
	{															\
		if (pScan->m_bTopEdge != 0)								\
		{														\
			pTarget->m_bTopAboveEye = 0;								\
			pTarget->m_pTopNeighbour = pObj;							\
			pTarget->m_bHasFlats = 1;								\
			pTarget->m_pTopFlatTex = (pKey);					\
			pTarget->m_pTopEdgeY = (pEdge);						\
		}														\
		else													\
		{														\
			pTarget->m_bBottomAboveEye = 0;								\
			pTarget->m_pBottomNeighbour = pObj;							\
			pTarget->m_bHasFlats = 1;								\
			pTarget->m_pBottomFlatTex = (pKey);					\
			pTarget->m_pBottomEdgeY = (pEdge);						\
		}														\
		HOVER_SPANUNLINK(pScan, pPrev);							\
		continue;												\
	}

/////////////////////////////////////////////////////////////////////////////
// CSpanRasterizer (span pass)

// hover.exe:0x402630
// FUNCTION: HOVER 0x402630
void CSpanRasterizer::DrawWallWithFlats(RENDEROBJ* pObj, DWORD dwA, DWORD dwB)
{
	SPANENTRY* pSpan;
	DWORD dwB4;
	DWORD dwB8;
	DWORD dwV;
	int bCovered;

	dwB4 = pObj->m_run.m_dwTopY;
	dwB8 = pObj->m_run.m_dwBottomY;
	pSpan = &pObj->m_run;
	g_dwClipBottomY = dwB;
	// the original carries a dead cmp of m_bSprite here
	g_dwClipTopY = dwA;
	pObj->m_pSource->m_wFlags |= 8;
	if (pObj->m_pTopNeighbour != 0)
	{
		if ((int)pObj->m_dwTopDZ > 0)
		{
			bCovered = 0;
			dwV = *pObj->m_pTopEdgeY;
			if ((int)dwV > (int)dwB4)
			{
				if ((int)dwV > (int)dwA)
				{
					if ((int)dwV >= (int)dwB)
					{
						m_pRunHeap->AddFlatRun(pObj->m_dwTopZ,
							pObj->m_pTopFlatTex, dwA, dwB);
						bCovered = 1;
					}
					else
					{
						m_pRunHeap->AddFlatRun(pObj->m_dwTopZ,
							pObj->m_pTopFlatTex, dwA, dwV);
						g_dwClipTopY = dwV + g_dwRowStep;
					}
				}
			}
			else if ((int)dwA < (int)dwB4)
			{
				if ((int)dwB <= (int)dwB4)
				{
					m_pRunHeap->AddFlatRun(pObj->m_dwTopZ, pObj->m_pTopFlatTex,
						dwA, dwB);
					bCovered = 1;
				}
				else
				{
					m_pRunHeap->AddFlatRun(pObj->m_dwTopZ, pObj->m_pTopFlatTex,
						dwA, dwB4);
					g_dwClipTopY = dwB4 + g_dwRowStep;
				}
			}
			if (bCovered)
				return;
			if (pObj->m_pBottomNeighbour == 0)
				goto L_final;
			dwV = *pObj->m_pBottomEdgeY;
			if ((int)dwV > (int)dwB8)
			{
				if ((int)g_dwClipBottomY > (int)dwB8)
				{
					if ((int)g_dwClipTopY >= (int)dwB8)
					{
						m_pRunHeap->AddFlatRun(pObj->m_dwBottomZ,
							pObj->m_pBottomFlatTex, g_dwClipTopY, g_dwClipBottomY);
						bCovered = 1;
					}
					else
					{
						m_pRunHeap->AddFlatRun(pObj->m_dwBottomZ,
							pObj->m_pBottomFlatTex, dwB8, g_dwClipBottomY);
						g_dwClipBottomY = dwB8 - g_dwRowStep;
					}
				}
			}
			else if ((int)g_dwClipBottomY > (int)dwV)
			{
				if ((int)g_dwClipTopY >= (int)dwV)
				{
					m_pRunHeap->AddFlatRun(pObj->m_dwBottomZ, pObj->m_pBottomFlatTex,
						g_dwClipTopY, g_dwClipBottomY);
					bCovered = 1;
				}
				else
				{
					m_pRunHeap->AddFlatRun(pObj->m_dwBottomZ, pObj->m_pBottomFlatTex,
						dwV, g_dwClipBottomY);
					g_dwClipBottomY = dwV - g_dwRowStep;
				}
			}
			if (bCovered)
				return;
			goto L_final;
		}
		if (pObj->m_pBottomNeighbour != 0)
		{
			bCovered = 0;
			dwV = *pObj->m_pBottomEdgeY;
			if ((int)dwV > (int)dwB8)
			{
				if ((int)dwB > (int)dwB8)
				{
					if ((int)dwA >= (int)dwB8)
					{
						m_pRunHeap->AddFlatRun(pObj->m_dwBottomZ,
							pObj->m_pBottomFlatTex, dwA, dwB);
						bCovered = 1;
					}
					else
					{
						m_pRunHeap->AddFlatRun(pObj->m_dwBottomZ,
							pObj->m_pBottomFlatTex, dwB8, dwB);
						g_dwClipBottomY = dwB8 - g_dwRowStep;
					}
				}
			}
			else if ((int)dwB > (int)dwV)
			{
				if ((int)dwA >= (int)dwV)
				{
					m_pRunHeap->AddFlatRun(pObj->m_dwBottomZ, pObj->m_pBottomFlatTex,
						dwA, dwB);
					bCovered = 1;
				}
				else
				{
					m_pRunHeap->AddFlatRun(pObj->m_dwBottomZ, pObj->m_pBottomFlatTex,
						dwV, dwB);
					g_dwClipBottomY = dwV - g_dwRowStep;
				}
			}
			if (bCovered)
				return;
		}
		bCovered = 0;
		dwV = *pObj->m_pTopEdgeY;
		if ((int)dwB4 < (int)dwV)
		{
			if ((int)g_dwClipTopY < (int)dwV)
			{
				if ((int)g_dwClipBottomY <= (int)dwV)
				{
					m_pRunHeap->AddFlatRun(pObj->m_dwTopZ, pObj->m_pTopFlatTex,
						g_dwClipTopY, g_dwClipBottomY);
					bCovered = 1;
				}
				else
				{
					m_pRunHeap->AddFlatRun(pObj->m_dwTopZ, pObj->m_pTopFlatTex,
						g_dwClipTopY, dwV);
					g_dwClipTopY = dwV + g_dwRowStep;
				}
			}
		}
		else if ((int)g_dwClipTopY < (int)dwB4)
		{
			if ((int)g_dwClipBottomY <= (int)dwB4)
			{
				m_pRunHeap->AddFlatRun(pObj->m_dwTopZ, pObj->m_pTopFlatTex,
					g_dwClipTopY, g_dwClipBottomY);
				bCovered = 1;
			}
			else
			{
				m_pRunHeap->AddFlatRun(pObj->m_dwTopZ, pObj->m_pTopFlatTex,
					g_dwClipTopY, dwB4);
				g_dwClipTopY = dwB4 + g_dwRowStep;
			}
		}
		if (bCovered)
			return;
		goto L_final;
	}
	if (ClipBottomFlat(pObj, pSpan, dwB4, dwB8, pObj->m_dwBottomZ) != 0)
		return;
L_final:
	if ((int)g_dwClipBottomY < (int)g_dwClipTopY)
		return;
	DrawWallRun(pObj, pSpan, g_dwClipTopY, g_dwClipBottomY);
}

// hover.exe:0x402990
// FUNCTION: HOVER 0x402990
void CSpanRasterizer::DrawWallRun(RENDEROBJ* pObj, SPANENTRY* pSpan, DWORD dwA,
	DWORD dwB)
{
	CMerlinTexture* pTex;
	MERLIN_TEXFRAME* pEntry;
	TEXCOLUMN* pPts;
	DWORD dwLevel;
	DWORD dwMax;
	DWORD dwVal;
	DWORD dwX;
	DWORD dwP;
	DWORD dwT;
	BYTE cl;

	g_dwSpanCount = dwB + (g_dwRowStep - dwA);
	if ((int)g_dwSpanCount <= 0)
		return;
	g_dwDst = g_dwColumnOrigin + dwA * g_dwRowPitch;
	g_dwSpanDepth = (DWORD)(short)pObj->m_wDistance;
	if (pObj->m_bMappingCached == 0)
	{
		pTex = pObj->m_pTex;
		// the original carries a dead cmp of m_bSprite here
		pObj->m_pSource->m_wFlags |= 8;
		if (g_bWallMipMapping != 0)
		{
			cl = (BYTE)(pObj->m_dwTexScaleLog2 + 4);
			dwVal = (DWORD)(((int)g_dwSpanDepth >> cl) & 0x7FF);
			dwLevel = g_pbMipLut[dwVal];
			dwMax = pTex->m_dwFrameCount - 1;
			if ((int)dwMax < (int)dwLevel)
				dwLevel = dwMax;
			if ((int)g_dwMipBias > (int)dwLevel)
				dwLevel = g_dwMipBias;
		}
		else
		{
			dwLevel = g_dwMipBias;
		}
		pEntry = &pTex->m_pFrames[dwLevel];
		cl = (BYTE)(pObj->m_dwTexScaleLog2 + dwLevel);
		dwVal = (DWORD)(((short)pObj->m_wTexU >> cl) &
			(int)pEntry->m_dwColMask);
		pPts = &pEntry->m_pColumns[dwVal];
		dwP = (DWORD)pPts->m_pRuns->m_pPixels;
		dwX = pSpan->m_dwBottomY - pSpan->m_dwTopY;
		pSpan->m_pPixels = (BYTE*)dwP;
		g_dwTexels = dwP;
		if (dwX != 0)
		{
			dwP = (DWORD)((int)pObj->m_dwHeight16 / (int)dwX);
			dwP = (DWORD)((int)dwP >> cl);
		}
		else
		{
			dwP = 0;
		}
		dwT = pEntry->m_dwUnk04;
		pObj->m_dwVStep = dwP;
		pObj->m_bMappingCached = 1;
		dwT = (dwT << 0x10) + 0xFFFF;
		pObj->m_dwVMask = dwT;
		g_dwVMask = dwT;
	}
	else
	{
		g_dwVMask = pObj->m_dwVMask;
		dwP = pObj->m_dwVStep;
		g_dwTexels = (DWORD)pSpan->m_pPixels;
	}
	g_dwUStep = dwP;
	g_dwColumnV = (dwA - pSpan->m_dwTopY) * dwP;
	g_dwShadeTable = (DWORD)g_pRenderCtx->GetShadeRow(g_dwSpanDepth);
	g_pRenderCtx->m_pfnDrawWall();
}

// hover.exe:0x402B10
// FUNCTION: HOVER 0x402b10
void CSpanRasterizer::DrawFlatRow(DWORD dwA, DWORD dwB, CMerlinTexture* pTex,
	DWORD dwY)
{
	CRenderContext* pView;
	MERLIN_TEXFRAME* pEntry;
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
	dwRow = g_dwEyeZ - dwY;
	nT = abs((int)((dwX0 & (0 - g_dwRowStep)) - g_dwHorizonY));
	dwScale = (DWORD)((int)g_pdwDistTable[dwRow + 0x400] / (nT + 1));
	g_dwSpanDepth = dwScale;
	if (m_bMipMapping != 0)
	{
		dwLevel = (DWORD)g_pbMipLut[(dwScale & 0x7FF0) >> 4] + 1;
		g_dwFlatMipLevel = dwLevel;
		if ((int)g_dwSpanDepth < 0xFA)
			dwLevel = g_dwMipBias;
		g_dwFlatMipLevel = dwLevel;
		if ((int)dwLevel >= (int)pTex->m_dwFrameCount)
			dwLevel = pTex->m_dwFrameCount - 1;
		g_dwFlatMipLevel = dwLevel;
		if ((int)g_dwMipBias > (int)dwLevel)
			dwLevel = g_dwMipBias;
	}
	else
	{
		dwLevel = g_dwMipBias;
	}
	cl = (BYTE)dwLevel;
	pEntry = &pTex->m_pFrames[dwLevel];
	pView = g_pRenderCtx;
	g_dwTexels = (DWORD)pEntry->m_pPixels;
	if (pView->m_bMirror != 0)
	{
		dwT14 = g_dwViewWidth - g_dwCurColumn - 1;
		dwAng = pView->m_dwCamAngle << 6;
		dwBX = pView->m_dwCamX << 0x10;
		dwT24 = g_dwViewWidth - dwA - 1;
		dwBY = pView->m_dwCamY << 0x10;
		dwF0 = HOVER_SINLUT(dwAng + 0x1554) * g_dwSpanDepth + dwBX;
		dwF1 = dwBX + HOVER_SINLUT(dwAng + 0x2AA8) * g_dwSpanDepth;
		dwF3 = HOVER_SINLUT(dwAng - 0xAAC) * g_dwSpanDepth + dwBY;
		dwF2 = dwBY + HOVER_SINLUT(dwAng + 0xAA8) * g_dwSpanDepth;
	}
	else
	{
		dwT14 = dwA;
		dwBX = pView->m_dwCamX << 0x10;
		dwAng = pView->m_dwCamAngle << 6;
		dwT24 = g_dwCurColumn;
		dwBY = pView->m_dwCamY << 0x10;
		dwF1 = HOVER_SINLUT(dwAng + 0x1554) * g_dwSpanDepth + dwBX;
		dwF0 = HOVER_SINLUT(dwAng + 0x2AA8) * g_dwSpanDepth + dwBX;
		dwAng += 0xAA8;
		dwF2 = HOVER_SINLUT(dwAng - 0x1554) * g_dwSpanDepth + dwBY;
		dwF3 = HOVER_SINLUT(dwAng) * g_dwSpanDepth + dwBY;
	}
	dwCount = dwT24 - dwT14;
	g_dwFlatMipLevel = dwLevel;
	dwFb = (DWORD)(dwX0 * g_dwRowPitch + pView->m_pScanOrigin);
	g_dwSpanCount = dwCount;
	g_dwDst = dwT14 + dwFb;
	dwDiv = g_dwViewWidth << cl;
	g_dwUStep = (DWORD)((int)(dwF3 - dwF2) / (int)dwDiv);
	dwF2 = (DWORD)((int)dwF2 >> cl);
	g_dwVStep = (DWORD)((int)(dwF0 - dwF1) / (int)dwDiv);
	dwF1 = (DWORD)((int)dwF1 >> cl);
	g_dwU = dwT14 * g_dwUStep + dwF2;
	g_dwV = g_dwVStep * dwT14 + dwF1;
	g_dwShadeTable = (DWORD)pView->GetShadeRow(g_dwSpanDepth);
	g_dwUMask = (pEntry->m_dwUnk04 << 0x10) + 0xFFFF;
	g_dwVMask = pEntry->m_dwColMask << 0x10;
	g_dwVShift = 0x10 - pEntry->m_dwUnk10;
	if (pView->m_bUseShiftTable != 0)
		pView->m_ppfnDrawFlatByShift[g_dwVShift]();
	else
		pView->m_pfnDrawFlat();
}

// hover.exe:0x402E00
// FUNCTION: HOVER 0x402e00
void CSpanRasterizer::RenderColumn(DWORD dwUnk)
{
	RENDEROBJ* pObj;
	SPANENTRY* pSpan;
	SPANENTRY* pArena;
	TEXCOLUMN* pPts;
	TEXRUN* pPt;
	MERLIN_TEXFRAME* pEntry;
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

	g_pArenaCursor = g_abSpanArena;
	if (m_bRelink != 0)
		LinkFlatEdges();
	for (pObj = g_pColumnObjs; pObj != 0; pObj = pObj->m_pColNext)
	{
		nCount = (short)pObj->m_wDistance;
		if (pObj->m_bHidden != 0 || nCount <= 0)
		{
			if (nCount == 0)
			{
				// no points: clamp the embedded span to the edge defaults
				pSpan = &pObj->m_run;
				if ((int)pObj->m_dwTopDZ > 0)
					pSpan->m_dwTopY = g_dwBelowClipY;
				else
					pSpan->m_dwTopY = g_dwAboveClipY;
				if ((int)pObj->m_dwBottomDZ > 0)
					pSpan->m_dwBottomY = g_dwBelowClipY;
				else
					pSpan->m_dwBottomY = g_dwAboveClipY;
			}
			pObj->m_bVisible = 0;
			continue;
		}
		pSpan = &pObj->m_run;
		dwScale = (DWORD)((int)g_dwColProjScale / nCount);
		pObj->m_bVisible = 1;
		dwX0 = (DWORD)(((int)(pObj->m_dwTopDZ * dwScale) >> 0xA) +
			(int)g_dwHorizonY);
		dwMask = 0 - g_dwRowStep;
		dwX0 &= dwMask;
		dwX1 = (DWORD)(((int)(pObj->m_dwBottomDZ * dwScale) >> 0xA) +
			(int)g_dwHorizonY);
		dwX1 &= dwMask;
		pSpan->m_dwTopY = dwX0;
		pSpan->m_dwBottomY = dwX1;
		if (pObj->m_bMasked == 0)
		{
			DWORD dwA, dwB, dwMin;

			pObj->m_bMappingCached = 0;
			if (pObj->m_bHasFlats == 0)
				continue;
			dwA = *pObj->m_pTopEdgeY;
			dwB = *pObj->m_pBottomEdgeY;
			dwMin = ((int)dwB >= (int)dwA) ? dwA : dwB;
			pObj->m_dwMinY = ((int)dwMin > (int)dwX0) ? dwX0 : dwMin;
			if (dwA != 0xFFF0000 && (int)dwA > (int)dwX1)
				dwX1 = dwA;
			if (dwB != 0xFFF0000 && (int)dwX1 < (int)dwB)
				pObj->m_dwMaxY = dwB;
			else
				pObj->m_dwMaxY = dwX1;
			continue;
		}
		pObj->m_dwMinY = dwX0;
		pObj->m_dwMaxY = dwX1;
		if (g_bWallMipMapping != 0)
		{
			cl = (BYTE)(pObj->m_dwTexScaleLog2 + 4);
			dwT = (DWORD)((nCount >> cl) & 0x7FF);
			dwLevel = g_pbMipLut[dwT];
			dwMax = pObj->m_pTex->m_dwFrameCount - 1;
			if ((int)dwLevel > (int)dwMax)
				dwLevel = dwMax;
			if ((int)g_dwMipBias > (int)dwLevel)
				dwLevel = g_dwMipBias;
		}
		else
		{
			dwLevel = g_dwMipBias;
		}
		// the original carries a dead cmp of m_bSprite here
		pObj->m_pSource->m_wFlags |= 8;
		pEntry = &pObj->m_pTex->m_pFrames[dwLevel];
		cl = (BYTE)(pObj->m_dwTexScaleLog2 + dwLevel);
		nPts = ((short)pObj->m_wTexU >> cl) & (int)pEntry->m_dwColMask;
		pPts = &pEntry->m_pColumns[nPts];
		dwX1 -= dwX0;
		if (dwX1 != 0)
		{
			dwX1 <<= cl;
			pObj->m_dwVStep = (DWORD)((int)pObj->m_dwHeight16 / (int)dwX1);
		}
		else
		{
			pObj->m_dwVStep = 0;
		}
		dwT = pEntry->m_dwUnk04 << 0x10;
		pArena = (SPANENTRY*)g_pArenaCursor;
		pObj->m_pRuns = pArena;
		dwT += 0xFFFF;
		dwMax = (DWORD)(short)pPts->m_wRunCount;
		pObj->m_dwRunCount = dwMax;
		pObj->m_dwVMask = dwT;
		g_pArenaCursor += dwMax * sizeof(SPANENTRY);
		if (dwMax == 0)
		{
			pObj->m_bVisible = 0;
			continue;
		}
		pPt = pPts->m_pRuns;
		if ((int)dwMax <= 0)
			continue;
		i = dwMax;
		do
		{
			cl = (BYTE)pObj->m_dwTexScaleLog2;
			dwA = pObj->m_dwTopZ - (DWORD)(((short)pPt->m_wTop) << cl);
			dwB = pObj->m_dwTopZ - (DWORD)(((short)pPt->m_wBottom) << cl);
			pArena->m_dwTopZ = dwA;
			pArena->m_dwBottomZ = dwB;
			if ((int)pObj->m_dwBottomZ > (int)dwB)
				pArena->m_dwBottomZ = pObj->m_dwBottomZ;
			pArena->m_pPixels = pPt->m_pPixels;
			pPt++;
			pArena++;
			dwT = (DWORD)(((int)((g_dwEyeZ - dwA) * dwScale) >> 0xA) +
				(int)g_dwHorizonY);
			dwT &= dwMask;
			pArena[-1].m_dwTopY = dwT;
			dwT = (DWORD)(((int)((g_dwEyeZ - pArena[-1].m_dwBottomZ) *
				dwScale) >> 0xA) + (int)g_dwHorizonY);
			dwT &= dwMask;
			pArena[-1].m_dwBottomY = dwT;
			i--;
		} while (i != 0);
	}
	CoverRows(g_pActiveObjs, g_dwColTopY, g_dwColBottomY);
	m_pRunHeap->FlushRuns(this);
	if (g_pArenaCursor > g_abSpanArena + sizeof(g_abSpanArena))
		OutputDebugStringA("Trashing Heap");	// hover.exe:0x4C520C
}

// hover.exe:0x4030F0
// FUNCTION: HOVER 0x4030f0
void CSpanRasterizer::LinkFlatEdges()
{
	EDGENODE* pScan;
	RENDEROBJ* pObj;
	RENDEROBJ* pObj2;
	RENDEROBJ* pNext2;
	RENDEROBJ* pPrev2;
	CMerlinStatic* pSrc;
	CMerlinTexture* pTex;
	EDGENODE* pNode;
	EDGENODE* pPrev;
	EDGENODE* pOldHead;
	DWORD dwArea;
	DWORD dwDelta;
	DWORD dwShift;
	DWORD dwT1;
	DWORD dwT2;
	DWORD dwVal;
	CMerlinTexture* pSrcB;
	CMerlinTexture* pKey;
	RENDEROBJ* pTarget;
	DWORD nW56;
	DWORD nW54;

	g_pEdgeList = 0;
	pObj = g_pObjPool;
	pNode = (EDGENODE*)g_abSpanArena;
	if (pObj->m_pPoolNext != 0)
	{
		do
		{
			pObj = pObj->m_pPoolNext;
		} while (pObj->m_pPoolNext != 0);
	}
	g_pArenaCursor = (BYTE*)pNode;
	if (pObj != 0)
	{
		for (;;)
		{
			pObj->m_pTopNeighbour = 0;
			pObj->m_pBottomNeighbour = 0;
			pObj->m_bHasFlats = 0;
			pObj->m_pTopEdgeY = &g_dwNoEdgeY;
			pObj->m_pBottomEdgeY = &g_dwNoEdgeY;
			if (pObj->m_bSprite != 0)
			{
				// sprite object: derive the span from the sprite's
				// height words
				nW56 = (DWORD)(short)((CMerlinDynamic*)pObj->m_pSource)->m_wTop;
				nW54 = (DWORD)(short)((CMerlinDynamic*)pObj->m_pSource)->m_wBottom;
				dwArea = (nW56 - nW54) << 0x10;
				pObj->m_dwTopZ = nW56;
				pObj->m_dwBottomZ = nW54;
				pObj->m_dwHeight16 = dwArea;
				pObj->m_dwTopDZ = g_dwEyeZ - nW56;
				pObj->m_bMasked = 1;
				pObj->m_dwBottomDZ = g_dwEyeZ - nW54;
				pTex = ((CMerlinDynamic*)pObj->m_pSource)->m_pTexture;
				pObj->m_pTex = pTex;
				if (pTex != 0)
				{
					pObj->m_dwTexScaleLog2 = (DWORD)(((int)dwArea >> 0x10) /
						(short)pTex->m_wHeight);
				}
				else
				{
					pObj->m_bHidden = 1;
				}
				dwShift = 0;
				while ((int)pObj->m_dwTexScaleLog2 > 1)
				{
					dwShift++;
					pObj->m_dwTexScaleLog2 = (DWORD)((int)pObj->m_dwTexScaleLog2 >> 1);
				}
				pObj->m_dwTexScaleLog2 = dwShift;
			}
			else
			{
				pSrc = (CMerlinStatic*)pObj->m_pSource;
				pScan = g_pEdgeList;
				nW56 = (DWORD)(short)pSrc->m_wTop;
				nW54 = (DWORD)(short)pSrc->m_wBottom;
				pObj->m_dwTopZ = nW56;
				dwDelta = nW56 - nW54;
				pObj->m_dwBottomZ = nW54;
				dwArea = dwDelta << 0x10;
				pObj->m_dwHeight16 = dwArea;
				pObj->m_dwTopDZ = g_dwEyeZ - nW56;
				pObj->m_dwBottomDZ = g_dwEyeZ - nW54;
				if (pObj->m_bHidden != 0)
				{
					dwT1 = (pSrc->m_dwY1 -
						g_pRenderCtx->m_dwCamY) *
						pSrc->m_dwDeltaX;
					dwT2 = (g_pRenderCtx->m_dwCamX -
						pSrc->m_dwX1) * pSrc->m_dwDeltaY;
					pObj->m_bFrontFace = 0;
					if ((int)(dwT1 + dwT2) >= 0)
						pObj->m_bFrontFace = 1;
				}
				if (pObj->m_bFrontFace != 0)
				{
					pObj->m_pTex = pSrc->m_pFrontTex;
					pObj->m_bMasked = pSrc->m_bFrontTransparent;
				}
				else
				{
					pObj->m_pTex = pSrc->m_pBackTex;
					pObj->m_bMasked = pSrc->m_bBackTransparent;
				}
				if (pObj->m_pTex != 0)
				{
					dwT1 = (DWORD)(short)pObj->m_pTex->m_wHeight;
					pObj->m_dwTexScaleLog2 = (DWORD)(((int)dwArea >> 0x10) /
						(int)dwT1);
				}
				else
				{
					pObj->m_bHidden = 1;
				}
				dwShift = 0;
				while ((int)pObj->m_dwTexScaleLog2 > 1)
				{
					dwShift++;
					pObj->m_dwTexScaleLog2 = (DWORD)((int)pObj->m_dwTexScaleLog2 >> 1);
				}
				pObj->m_dwTexScaleLog2 = dwShift;
				if (pObj->m_bFrontFace != 0)
				{
					// merge matching front-face edges out of the list
					if (g_pEdgeList != 0)
					{
						do
						{
							dwVal = pScan->m_dwEdgeZ;
							if (dwVal == nW56)
							{
								pKey = pScan->m_pFlatTex;
								if (pSrc->m_pBackTopTex == pKey)
								{
									pTarget = pScan->m_pObj;
									HOVER_SPANMERGE(pObj, pScan,
										pPrev, pKey,
										&pObj->m_run.m_dwTopY);
								}
							}
							if (dwVal == nW54)
							{
								pKey = pScan->m_pFlatTex;
								if (pSrc->m_pBackBottomTex == pKey)
								{
									pTarget = pScan->m_pObj;
									HOVER_SPANMERGE_LO(pObj, pScan,
										pPrev, pKey,
										&pObj->m_run.m_dwBottomY);
								}
							}
							pScan = pScan->m_pNext;
						} while (pScan != 0);
					}
					if (pObj->m_bCovered == 0)
					{
						pSrcB = pSrc->m_pFrontTopTex;
						if (pSrcB != 0)
						{
							pOldHead = g_pEdgeList;
							pNode->m_pPrev = 0;
							g_pEdgeList = pNode;
							pNode->m_pNext = pOldHead;
							if (pOldHead != 0)
								pOldHead->m_pPrev = pNode;
							pNode->m_pFlatTex = pSrcB;
							pNode->m_pObj = pObj;
							pNode->m_bTopEdge = 1;
							pNode->m_dwEdgeZ = nW56;
							pNode++;
						}
						pSrcB = pSrc->m_pFrontBottomTex;
						if (pSrcB != 0 && (int)dwDelta > 0)
						{
							pOldHead = g_pEdgeList;
							pNode->m_pPrev = 0;
							g_pEdgeList = pNode;
							pNode->m_pNext = pOldHead;
							if (pOldHead != 0)
								pOldHead->m_pPrev = pNode;
							pNode->m_pFlatTex = pSrcB;
							pNode->m_dwEdgeZ = nW54;
							pNode->m_pObj = pObj;
							pNode->m_bTopEdge = 0;
							pNode++;
						}
					}
				}
				else
				{
					// merge matching back-face edges out of the list
					while (pScan != 0)
					{
						dwVal = pScan->m_dwEdgeZ;
						if (dwVal == nW56)
						{
							pKey = pScan->m_pFlatTex;
							if (pSrc->m_pFrontTopTex == pKey)
							{
								pTarget = pScan->m_pObj;
								HOVER_SPANMERGE(pObj, pScan, pPrev,
									pKey, &pObj->m_run.m_dwTopY);
							}
						}
						if (dwVal == nW54)
						{
							pKey = pScan->m_pFlatTex;
							if (pSrc->m_pFrontBottomTex == pKey)
							{
								pTarget = pScan->m_pObj;
								HOVER_SPANMERGE_LO(pObj, pScan, pPrev,
									pKey, &pObj->m_run.m_dwBottomY);
							}
						}
						pScan = pScan->m_pNext;
					}
					if (pObj->m_bCovered == 0)
					{
						pSrcB = pSrc->m_pBackTopTex;
						if (pSrcB != 0)
						{
							pOldHead = g_pEdgeList;
							pNode->m_pPrev = 0;
							g_pEdgeList = pNode;
							pNode->m_pNext = pOldHead;
							if (pOldHead != 0)
								pOldHead->m_pPrev = pNode;
							pNode->m_pFlatTex = pSrcB;
							pNode->m_pObj = pObj;
							pNode->m_bTopEdge = 1;
							pNode->m_dwEdgeZ = nW56;
							pNode++;
						}
						pSrcB = pSrc->m_pBackBottomTex;
						if (pSrcB != 0 && (int)dwDelta > 0)
						{
							pOldHead = g_pEdgeList;
							pNode->m_pPrev = 0;
							g_pEdgeList = pNode;
							pNode->m_pNext = pOldHead;
							if (pOldHead != 0)
								pOldHead->m_pPrev = pNode;
							pNode->m_pFlatTex = pSrcB;
							pNode->m_dwEdgeZ = nW54;
							pNode->m_pObj = pObj;
							pNode->m_bTopEdge = 0;
							pNode++;
						}
					}
				}
			}
			g_pArenaCursor = (BYTE*)pNode;
			pObj = pObj->m_pPoolPrev;
			if (pObj == 0)
				break;
		}
	}
	// turn the edge list into the objects' edge pointers
	pScan = g_pEdgeList;
	while (pScan != 0)
	{
		pTarget = pScan->m_pObj;
		if (pTarget->m_bHidden != 0)
		{
			pPrev = pScan->m_pPrev;
			pScan = pScan->m_pNext;
			if (pPrev != 0)
				pPrev->m_pNext = pScan;
			else
				g_pEdgeList = pScan;
			if (pScan == 0)
				break;
			pScan->m_pPrev = pPrev;
			continue;
		}
		if (pScan->m_bTopEdge != 0)
		{
			pKey = pScan->m_pFlatTex;
			pTarget->m_pTopNeighbour = (RENDEROBJ*)-1;
			pTarget->m_pTopFlatTex = pKey;
			pTarget->m_bHasFlats = 1;
			if ((int)pScan->m_dwEdgeZ > (int)g_dwEyeZ)
			{
				pTarget->m_bTopAboveEye = 1;
				pTarget->m_pTopEdgeY = &g_dwAboveClipY;
			}
			else
			{
				pTarget->m_pTopEdgeY = &g_dwBelowClipY;
				pTarget->m_bTopAboveEye = 0;
			}
		}
		else
		{
			pKey = pScan->m_pFlatTex;
			pTarget->m_pBottomNeighbour = (RENDEROBJ*)-1;
			pTarget->m_pBottomFlatTex = pKey;
			pTarget->m_bHasFlats = 1;
			if ((int)pScan->m_dwEdgeZ > (int)g_dwEyeZ)
			{
				pTarget->m_bBottomAboveEye = 1;
				pTarget->m_pBottomEdgeY = &g_dwAboveClipY;
			}
			else
			{
				pTarget->m_pBottomEdgeY = &g_dwBelowClipY;
				pTarget->m_bBottomAboveEye = 0;
			}
		}
		pScan = pScan->m_pNext;
	}
	// copy the static list into the active list links and re-sort it
	pObj2 = g_pColumnObjs;
	if (pObj2->m_pColNext != 0)
	{
		do
		{
			pNext2 = pObj2->m_pColNext;
			pPrev2 = pObj2->m_pColPrev;
			pObj2->m_pActiveNext = pNext2;
			pObj2->m_pActivePrev = pPrev2;
			pObj2 = pNext2;
		} while (pNext2->m_pColNext != 0);
	}
	pPrev2 = pObj2->m_pColPrev;
	pObj2->m_pActivePrev = pPrev2;
	pObj2->m_pActiveNext = 0;
	if (pPrev2 != 0)
	{
		do
		{
			if (pObj2->m_bHasFlats != 0)
				Hover_SortBehindNeighbours(pObj2);
			pPrev2 = pObj2->m_pActivePrev;
			if (pPrev2 != 0)
				pObj2 = pPrev2;
		} while (pObj2->m_pActivePrev != 0);
	}
	pObj2->m_pActivePrev = 0;
	g_pActiveObjs = pObj2;
	m_bRelink = 0;
}

/////////////////////////////////////////////////////////////////////////////
// CRenderContext

// hover.exe:0x403850
// FUNCTION: HOVER 0x403850
DWORD* CRenderContext::GetShadeRow(DWORD dwY)
{
	DWORD dwOff;
	DWORD* pTable;

	dwOff = dwY - 0x15E;
	if ((int)dwOff < 1)
		dwOff = 1;
	dwOff &= 0xFFFFFFC0;
	pTable = m_pShade;
	if ((int)dwOff > 0xFC0)
		return pTable + 0xFC0;
	return pTable + dwOff;
}

/////////////////////////////////////////////////////////////////////////////
// CFlatRunHeap

// Find-or-create the span-run node for one (scanline, texture) pair and
// append the x span [dwC, dwD] to it. The node is keyed on
// (dwA << 0x12) + dwB, so every run in one node shares the scanline dwA and
// the texture dwB; FlushRuns later passes that texture to DrawFlatRow.

// hover.exe:0x403980
// FUNCTION: HOVER 0x403980
void CFlatRunHeap::AddFlatRun(DWORD dwA, CMerlinTexture* pTex, DWORD dwC, DWORD dwD)
{
	FLATNODE* pNode;
	BYTE* pEnd;
	BYTE* pCur;
	DWORD dwKey;
	DWORD dwCount;
	WORD* pw;
	WORD** ppw;

	pNode = m_pFlatHead;
	// The key folds the texture's address into 32 bits (a 64-bit build
	// would need a real hash here).
	dwKey = (dwA << 0x12) + (DWORD)pTex;
	while (pNode != 0)
	{
		if (pNode->m_dwKey == dwKey)
			break;
		pNode = pNode->m_pNext;
	}
	if (pNode == 0)
	{
		// allocate a new node (header + buffers) from the heap
		if (m_pFlatTail != 0)
			m_pFlatTail->m_pNext = (FLATNODE*)m_pHeapCursor;
		else
			m_pFlatHead = (FLATNODE*)m_pHeapCursor;
		m_pFlatTail = pNode = (FLATNODE*)m_pHeapCursor;
		pCur = (BYTE*)(pNode + 1);
		m_pHeapCursor = pCur;
		pNode->m_pRunsA = (WORD*)pCur;
		pCur += 0xA0;
		m_pHeapCursor = pCur;
		pNode->m_pRunsB = (WORD*)pCur;
		pEnd = pCur + m_dwRows * 2 + 0xA0;
		pCur += 0xA0;
		m_pHeapCursor = pCur;
		pNode->m_pRowStartCol = (WORD*)pCur;
		m_pHeapCursor = pEnd;
		pNode->m_pNext = 0;
		pNode->m_dwCurRuns = 0;
		pNode->m_dwPrevRuns = 0;
		pNode->m_ppPrevRuns = &pNode->m_pRunsA;
		pNode->m_dwHeight = dwA;
		pNode->m_ppCurRuns = &pNode->m_pRunsB;
		pNode->m_pTex = pTex;
		pNode->m_dwKey = dwKey;
		m_dwFlatNodes++;
		if (m_pHeapBase + 0x16B48 < pEnd)
			OutputDebugStringA("Blowing floor heap");	// hover.exe:0x4C55F4
	}
	dwCount = pNode->m_dwCurRuns;
	if ((int)dwCount < 0x28)
	{
		ppw = pNode->m_ppCurRuns;
		pNode->m_dwCurRuns = ++dwCount;
		pw = *ppw + (dwCount - 1) * 2;
		pw[0] = (WORD)dwC;
		pw[1] = (WORD)dwD;
	}
}

// One node's merge pass, written once because the original carries two
// compiled copies of it: the first for the current node (which emits its
// runs through DrawSkyRow) and the second for every node of the span-run
// list (DrawFlatRow).  EMIT names the drawer call.
//
// The new spans of the node are copied into the merge scratch buffer, then
// walked in lockstep with the spans accumulated by the previous pass: where
// an old span is uncovered its index words are handed to the drawer, where a
// new span is uncovered the index words are stamped with the current column.
#define HOVER_SPANMERGE_NODE(EMIT)													\
	pNew = m_pScratchRuns;																\
	pSrc = *pNode->m_ppCurRuns;														\
	pDst = pNew;																	\
	dwCount = pNode->m_dwCurRuns;														\
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
	dwStep = g_dwRowStep;														\
	nNew = 0;																		\
	nOld = 0;																		\
	pOld = *pNode->m_ppPrevRuns;														\
	if ((int)pNode->m_dwPrevRuns > 0)													\
	{																				\
		do																			\
		{																			\
			int nOldX0;																\
			volatile int nOldX1;													\
			int nNewX0;																\
			volatile int nNewX1;													\
																					\
			if ((int)pNode->m_dwCurRuns <= (int)nNew)									\
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
							dwStep = g_dwRowStep;								\
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
								dwStep = g_dwRowStep;							\
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
								dwStep = g_dwRowStep;							\
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
						pIdx = pNode->m_pRowStartCol + nNewX0;							\
						do															\
						{															\
							*pIdx = *(WORD*)&g_dwCurColumn;						\
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
							pIdx = pNode->m_pRowStartCol + nNewX0;						\
							do														\
							{														\
								*pIdx = *(WORD*)&g_dwCurColumn;					\
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
							pIdx = pNode->m_pRowStartCol + nNewX0;						\
							do														\
							{														\
								*pIdx = *(WORD*)&g_dwCurColumn;					\
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
		} while ((int)pNode->m_dwPrevRuns > (int)nOld);								\
	}																				\
	if ((int)nOld < (int)pNode->m_dwPrevRuns)											\
	{																				\
		i = nOld;																	\
		do																			\
		{																			\
			pOld = *pNode->m_ppPrevRuns + i * 2;										\
			nX0 = (short)pOld[0];													\
			nX1 = (short)pOld[1];													\
			if (nX1 >= nX0)															\
			{																		\
				do																	\
				{																	\
					EMIT(nX0);														\
					dwStep = g_dwRowStep;										\
					nX0 += (int)dwStep;												\
					pOld = *pNode->m_ppPrevRuns + i * 2;								\
					nX1 = (short)pOld[1];											\
				} while (nX1 >= nX0);												\
			}																		\
			i++;																	\
		} while ((int)pNode->m_dwPrevRuns > (int)i);									\
	}																				\
	if ((int)nNew < (int)pNode->m_dwCurRuns)											\
	{																				\
		pDst = m_pScratchRuns + nNew * 2 + 1;												\
		dwCount = pNode->m_dwCurRuns - nNew;											\
		do																			\
		{																			\
			nX0 = (short)pDst[-1];													\
			nX1 = (short)pDst[0];													\
			if (nX0 <= nX1)															\
			{																		\
				pIdx = pNode->m_pRowStartCol + nX0;										\
				do																	\
				{																	\
					*pIdx = *(WORD*)&g_dwCurColumn;								\
					nX0 += (int)dwStep;												\
					pIdx += dwStep;													\
				} while (nX0 <= nX1);												\
			}																		\
			pDst += 2;																\
			dwCount--;																\
		} while (dwCount != 0);														\
	}																				\
	ppA = pNode->m_ppCurRuns;															\
	ppB = pNode->m_ppPrevRuns;															\
	dwCount = pNode->m_dwCurRuns;														\
	pNode->m_ppCurRuns = ppB;															\
	pNode->m_ppPrevRuns = ppA;															\
	pNode->m_dwPrevRuns = dwCount;														\
	pNode->m_dwCurRuns = 0

// The flat (untextured) drawer of the current node's runs
#define HOVER_SPANEMIT_FLAT(nX)										\
	pRender->DrawSkyRow((DWORD)(short)pNode->m_pRowStartCol[nX], (DWORD)(nX))

// The textured drawer of a span-run node: the run carries its own scanline
// (m_dwHeight) and CMerlinTexture (m_pTex)
#define HOVER_SPANEMIT_TEX(nX)										\
	pRender->DrawFlatRow((DWORD)(short)pNode->m_pRowStartCol[nX], (DWORD)(nX),	\
		pNode->m_pTex, pNode->m_dwHeight)

// hover.exe:0x403A60
// FUNCTION: HOVER 0x403a60
void CFlatRunHeap::FlushRuns(CSpanRasterizer* pRender)
{
	FLATNODE* pNode;
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

	pNode = m_pSkyNode;
	HOVER_SPANMERGE_NODE(HOVER_SPANEMIT_FLAT);
	pNode = m_pFlatHead;
	if (pNode != 0)
	{
		do
		{
			HOVER_SPANMERGE_NODE(HOVER_SPANEMIT_TEX);
			pNode = pNode->m_pNext;
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
void CSpanRasterizer::CoverRows(RENDEROBJ* pObj, DWORD dwX, DWORD dwXMax)
{
	SPANENTRY* pSpan;
	DWORD dwX0;
	DWORD dwX1;
	DWORD i;
	DWORD dwStep;			// g_dwRowStep, the one-pixel step

	dwStep = g_dwRowStep;
	for (;;)
	{
		if (pObj->m_bVisible == 0)
			goto L_advance;
		if (pObj->m_bHasFlats != 0)
		{
			dwX0 = pObj->m_dwMinY;
			dwX1 = pObj->m_dwMaxY;
			if ((int)dwX0 > (int)dwXMax)
				goto L_advance;
			if ((int)dwX1 < (int)dwX)
				goto L_advance;
			if ((int)dwX0 > (int)dwX)
			{
				if (pObj->m_pActiveNext != 0)
					CoverRows(pObj->m_pActiveNext, dwX, dwX0 - dwStep);
				else
					m_pRunHeap->AddSkyRun(dwX, dwX0 - dwStep);
				if ((int)dwX1 >= (int)dwXMax)
				{
					DrawWallWithFlats(pObj, dwX0, dwXMax);
					return;
				}
				DrawWallWithFlats(pObj, dwX0, dwX1);
				dwX = dwX1 + dwStep;
				if ((int)dwX1 >= (int)dwX)
					return;
				goto L_advance;
			}
			if ((int)dwX1 < (int)dwX)
				goto L_advance;
			if ((int)dwX1 > (int)dwXMax)
			{
				DrawWallWithFlats(pObj, dwX, dwXMax);
				return;
			}
			DrawWallWithFlats(pObj, dwX, dwX1);
			dwX = dwX1 + dwStep;
			if ((int)dwX1 >= (int)dwX)
				return;
			goto L_advance;
		}
		if (pObj->m_bMasked == 0)
		{
			dwX0 = pObj->m_run.m_dwTopY;
			dwX1 = pObj->m_run.m_dwBottomY;
			pSpan = &pObj->m_run;
			if ((int)dwX0 > (int)dwXMax)
				goto L_advance;
			if ((int)dwX1 < (int)dwX)
				goto L_advance;
			if ((int)dwX0 > (int)dwX)
			{
				if (pObj->m_pActiveNext != 0)
					CoverRows(pObj->m_pActiveNext, dwX, dwX0 - dwStep);
				else
					m_pRunHeap->AddSkyRun(dwX, dwX0 - dwStep);
				if ((int)dwX1 >= (int)dwXMax)
				{
					DrawWallRun(pObj, pSpan, dwX0, dwXMax);
					return;
				}
				DrawWallRun(pObj, pSpan, dwX0, dwX1);
				dwX = dwX1 + dwStep;
				if ((int)dwX1 >= (int)dwX)
					return;
				goto L_advance;
			}
			if ((int)dwX1 < (int)dwX)
				goto L_advance;
			if ((int)dwX1 > (int)dwXMax)
			{
				DrawWallRun(pObj, pSpan, dwX, dwXMax);
				return;
			}
			DrawWallRun(pObj, pSpan, dwX, dwX1);
			dwX = dwX1 + dwStep;
			if ((int)dwX1 >= (int)dwX)
				return;
			goto L_advance;
		}
		if ((int)pObj->m_dwMinY > (int)dwXMax)
			goto L_advance;
		if ((int)pObj->m_dwMaxY < (int)dwX)
			goto L_advance;
		i = 0;
		pSpan = pObj->m_pRuns;
		if ((int)pObj->m_dwRunCount > 0)
		{
			do
			{
				dwX0 = pSpan->m_dwTopY;
				dwX1 = pSpan->m_dwBottomY;
				if ((int)dwX0 > (int)dwX)
				{
					if ((int)dwX0 > (int)dwXMax)
						goto L_advance;
					if (pObj->m_pActiveNext != 0)
						CoverRows(pObj->m_pActiveNext, dwX,
							dwX0 - dwStep);
					else
						m_pRunHeap->AddSkyRun(dwX, dwX0 - dwStep);
					if ((int)dwX1 >= (int)dwXMax)
					{
						DrawMaskedRun(pObj, pSpan, dwX0, dwXMax);
						return;
					}
					DrawMaskedRun(pObj, pSpan, dwX0, dwX1);
					dwX = dwX1 + dwStep;
				}
				else if ((int)dwX1 >= (int)dwX)
				{
					if ((int)dwX1 > (int)dwXMax)
					{
						DrawMaskedRun(pObj, pSpan, dwX, dwXMax);
						return;
					}
					DrawMaskedRun(pObj, pSpan, dwX, dwX1);
					dwX = dwX1 + dwStep;
				}
				pSpan++;
				i++;
			} while ((int)pObj->m_dwRunCount > (int)i);
		}
		if ((int)dwX1 >= (int)dwX)
			return;
	L_advance:
		pObj = pObj->m_pActiveNext;
		if (pObj == 0)
		{
			m_pRunHeap->AddSkyRun(dwX, dwXMax);
			return;
		}
	}
}

// hover.exe:0x4065F0
// FUNCTION: HOVER 0x4065f0
int CSpanRasterizer::ClipBottomFlat(RENDEROBJ* pObj, SPANENTRY* pSpan, DWORD dwC,
	DWORD dwD, DWORD dwE)
{
	DWORD dwV;

	dwV = *pObj->m_pBottomEdgeY;
	if ((int)dwD < (int)dwV)
	{
		if ((int)dwD >= (int)g_dwClipBottomY)
			return 0;
		if ((int)dwD <= (int)g_dwClipTopY)
		{
			m_pRunHeap->AddFlatRun(dwE, pObj->m_pBottomFlatTex, g_dwClipTopY,
				g_dwClipBottomY);
			return 1;
		}
		m_pRunHeap->AddFlatRun(dwE, pObj->m_pBottomFlatTex, dwD, g_dwClipBottomY);
		g_dwClipBottomY = dwD - g_dwRowStep;
		return 0;
	}
	if ((int)g_dwClipBottomY <= (int)dwV)
		return 0;
	if ((int)g_dwClipTopY >= (int)dwV)
	{
		m_pRunHeap->AddFlatRun(dwE, pObj->m_pBottomFlatTex, g_dwClipTopY,
			g_dwClipBottomY);
		return 1;
	}
	m_pRunHeap->AddFlatRun(dwE, pObj->m_pBottomFlatTex, dwV, g_dwClipBottomY);
	g_dwClipBottomY = dwV - g_dwRowStep;
	return 0;
}

// hover.exe:0x4066B0
// FUNCTION: HOVER 0x4066b0
void CSpanRasterizer::DrawMaskedRun(RENDEROBJ* pObj, SPANENTRY* pSpan, DWORD dwA,
	DWORD dwB)
{
	DWORD dwY;

	g_dwSpanCount = dwB + (g_dwRowStep - dwA);
	if ((int)g_dwSpanCount < 1)
		return;
	g_dwDst = g_dwColumnOrigin + dwA * g_dwRowPitch;
	dwY = (DWORD)(short)pObj->m_wDistance;
	g_dwSpanDepth = dwY;
	g_dwVMask = pObj->m_dwVMask;
	g_dwUStep = pObj->m_dwVStep;
	g_dwColumnV = (dwA - pSpan->m_dwTopY) * pObj->m_dwVStep;
	g_dwTexels = (DWORD)pSpan->m_pPixels;
	g_dwShadeTable = (DWORD)g_pRenderCtx->GetShadeRow(dwY);
	g_pRenderCtx->m_pfnDrawWall();
}

// hover.exe:0x406740
// FUNCTION: HOVER 0x406740
void CSpanRasterizer::DrawSkyRow(DWORD dwA, DWORD dwB)
{
	CRenderContext* pView;
	MERLIN_TEXFRAME* pEntry;
	BYTE* pDst;
	DWORD dwStep;
	DWORD dwFill;
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
	BYTE bPix;
	BYTE cl;

	dwStep = g_dwRowStep;
	pView = g_pRenderCtx;
	dwB &= 0 - g_dwRowStep;
	if (pView->m_dwSkyMode == 1)
	{
		// flat fill: dwStep rows of the same color byte
		pDst = (BYTE*)(dwB * g_dwRowPitch + pView->m_pScanOrigin) + dwA;
		g_dwDst = (DWORD)pDst;
		if ((int)dwStep <= 0)
			return;
		bPix = *(BYTE*)&pView->m_pSkyGradient[dwB];	// low byte of the entry
		dwFill = g_dwCurColumn - dwA;
		do
		{
			memset(pDst, bPix, dwFill);
			pDst += g_dwRowPitch;
			dwStep--;
		} while (dwStep != 0);
		return;
	}
	dwS = (DWORD)(1 << g_dwMipBias);
	dwV = (DWORD)(0x280 / (int)dwS);
	dwBX = dwV * 6;
	dwV = (dwV << 0x10) / g_dwViewWidth;
	g_dwUStep = dwV;
	pEntry = &pView->m_pSkyTex->m_pFrames[g_dwMipBias];
	g_dwUMask = pEntry->m_dwUnk04;
	dwV = (DWORD)(0xF0 / (int)dwS);
	dwS = g_dwHorizonY;
	dwV = (dwV << 0x10) / dwS;
	dwS -= dwB;
	dwW = g_dwCurColumn;
	dwV = (DWORD)((int)(dwV * dwS) >> 0x10);
	dwV = (dwV & pEntry->m_dwColMask) * pEntry->m_dwHeight +
		(DWORD)pEntry->m_pPixels;
	g_dwTexels = dwV;
	dwCount = dwW - dwA;
	if (pView->m_bMirror != 0)
	{
		dwV2 = dwB * g_dwRowPitch + (DWORD)pView->m_pScanOrigin - dwW + g_dwViewWidth -
			1;
		dwW = dwW - g_dwViewWidth + 1;
		g_dwDst = dwV2;
		dwV3 = pView->m_dwCamAngle;
		// The mirrored start column: the run's own left edge dwA folded
		// through the viewport, NOT the texture offset computed above
		// (hover.exe:0x406878 re-reads the dwA argument here).
		dwV = dwA - g_dwViewWidth + 1;
		dwW *= g_dwUStep;
		dwV3 += 0x55;
		dwV *= g_dwUStep;
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
			cl = (BYTE)g_dwLogColStep;
			dwT10 = (DWORD)((int)dwV2 / (int)g_dwUStep);
			dwCount -= dwT10;
			dwCount = (DWORD)((int)dwCount >> cl);
			dwT14 = g_dwUStep;
			g_dwUStep <<= cl;
			g_dwUStep = 0 - g_dwUStep;
			g_dwSkyU = dwV2;
			g_dwSpanCount = (DWORD)(((int)dwT10 >> cl) + 1);
			pView->m_pfnDrawSky();
			dwW = g_dwColStep;
			g_dwSpanCount = dwCount;
			dwV4 = (dwW - 1) & dwT10;
			if (dwV4 != 0)
			{
				g_dwDst += dwT10 - dwV4 + dwW;
				g_dwSkyU = (dwV4 - dwW) * dwT14 + dwBX;
			}
			else
			{
				g_dwSkyU = dwBX;
				g_dwDst += dwT10;
			}
			g_pRenderCtx->m_pfnDrawSky();
			return;
		}
		cl = (BYTE)g_dwLogColStep;
		g_dwSkyU = dwV2;
		dwCount = (DWORD)((int)dwCount >> cl);
		g_dwUStep <<= cl;
		g_dwSpanCount = dwCount;
		g_dwUStep = 0 - g_dwUStep;
		pView->m_pfnDrawSky();
		return;
	}
	g_dwDst = (DWORD)(dwB * g_dwRowPitch + pView->m_pScanOrigin + dwA);
	dwV3 = pView->m_dwCamAngle & 0x1FF;
	dwW *= g_dwUStep;
	dwV3 = (dwV3 * dwBX) << 7;
	dwV = dwA * g_dwUStep;
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
		cl = (BYTE)g_dwLogColStep;
		dwT10 = (DWORD)((int)dwV / (int)g_dwUStep);
		dwCount -= dwT10;
		dwV4 = dwT10;
		dwCount = (DWORD)((int)dwCount >> cl);
		dwT14 = g_dwUStep;
		g_dwSkyU = dwV2;
		g_dwUStep <<= cl;
		g_dwSpanCount = (DWORD)(((int)dwV4 >> cl) + 1);
		pView->m_pfnDrawSky();
		dwW = g_dwColStep;
		g_dwSpanCount = dwCount;
		dwV = (dwW - 1) & dwV4;
		if (dwV != 0)
		{
			dwV4 = dwV4 - dwV + dwW;
			// Not the mirror branch's (dwV - dwW): here g_dwUStep is
			// still positive, and hover.exe:0x406a40 subtracts the other
			// way round.
			dwV = (dwW - dwV) * dwT14;
			g_dwDst += dwV4;
			g_dwSkyU = dwV;
		}
		else
		{
			g_dwSkyU = 0;
			g_dwDst += dwV4;
		}
		g_pRenderCtx->m_pfnDrawSky();
		return;
	}
	cl = (BYTE)g_dwLogColStep;
	g_dwSkyU = dwV2;
	dwCount = (DWORD)((int)dwCount >> cl);
	g_dwUStep <<= cl;
	g_dwSpanCount = dwCount;
	pView->m_pfnDrawSky();
}

// hover.exe:0x406A80
// FUNCTION: HOVER 0x406a80
void Hover_SortBehindNeighbours(RENDEROBJ* pObj)
{
	RENDEROBJ* pStop;
	RENDEROBJ* pCur;
	RENDEROBJ* pNext;
	RENDEROBJ* pPrev;
	RENDEROBJ* pOld;

	pStop = pObj->m_pTopNeighbour;
	if (pStop != 0)
	{
		if (pStop == (RENDEROBJ*)-1)
			pStop = 0;
		if (pObj->m_pActivePrev != pStop)
		{
			pCur = pObj;
			do
			{
				pCur = pCur->m_pActivePrev;
				if (pCur == 0)
					break;
				if ((int)pObj->m_dwTopDZ > 0)
				{
					if (pCur->m_bHasFlats != 0)
						Hover_SortBehindNeighbours(pCur);
					if ((int)pObj->m_dwTopZ >= (int)pCur->m_dwTopZ)
					{
						pNext = pCur->m_pActivePrev;
						pPrev = pCur->m_pActiveNext;
						pOld = pObj->m_pActiveNext;
						if (pNext != 0)
							pNext->m_pActiveNext = pPrev;
						if (pPrev != 0)
							pPrev->m_pActivePrev = pNext;
						pObj->m_pActiveNext = pCur;
						if (pOld != 0)
							pOld->m_pActivePrev = pCur;
						pCur->m_pActivePrev = pObj;
						pCur->m_pActiveNext = pOld;
						pCur = pPrev;
					}
				}
				else
				{
					if (pCur->m_bHasFlats != 0)
						Hover_SortBehindNeighbours(pCur);
					if ((int)pObj->m_dwTopZ <= (int)pCur->m_dwBottomZ)
					{
						pNext = pCur->m_pActivePrev;
						pPrev = pCur->m_pActiveNext;
						pOld = pObj->m_pActiveNext;
						if (pNext != 0)
							pNext->m_pActiveNext = pPrev;
						if (pPrev != 0)
							pPrev->m_pActivePrev = pNext;
						pObj->m_pActiveNext = pCur;
						if (pOld != 0)
							pOld->m_pActivePrev = pCur;
						pCur->m_pActivePrev = pObj;
						pCur->m_pActiveNext = pOld;
						pCur = pPrev;
					}
				}
			} while (pCur->m_pActivePrev != pStop);
		}
	}
	pStop = pObj->m_pBottomNeighbour;
	if (pStop != 0)
	{
		if (pStop == (RENDEROBJ*)-1)
			pStop = 0;
		if (pObj->m_pActivePrev != pStop)
		{
			pCur = pObj;
			do
			{
				pCur = pCur->m_pActivePrev;
				if (pCur == 0)
					break;
				if ((int)pObj->m_dwBottomDZ > 0)
				{
					if (pCur->m_bHasFlats != 0)
						Hover_SortBehindNeighbours(pCur);
					if ((int)pCur->m_dwTopZ <= (int)pObj->m_dwBottomZ)
					{
						pNext = pCur->m_pActivePrev;
						pPrev = pCur->m_pActiveNext;
						pOld = pObj->m_pActiveNext;
						if (pNext != 0)
							pNext->m_pActiveNext = pPrev;
						if (pPrev != 0)
							pPrev->m_pActivePrev = pNext;
						pObj->m_pActiveNext = pCur;
						if (pOld != 0)
							pOld->m_pActivePrev = pCur;
						pCur->m_pActivePrev = pObj;
						pCur->m_pActiveNext = pOld;
						pCur = pPrev;
					}
				}
				else
				{
					if (pCur->m_bHasFlats != 0)
						Hover_SortBehindNeighbours(pCur);
					if ((int)pCur->m_dwBottomZ >= (int)pObj->m_dwBottomZ)
					{
						pNext = pCur->m_pActivePrev;
						pPrev = pCur->m_pActiveNext;
						pOld = pObj->m_pActiveNext;
						if (pNext != 0)
							pNext->m_pActiveNext = pPrev;
						if (pPrev != 0)
							pPrev->m_pActivePrev = pNext;
						pObj->m_pActiveNext = pCur;
						if (pOld != 0)
							pOld->m_pActivePrev = pCur;
						pCur->m_pActivePrev = pObj;
						pCur->m_pActiveNext = pOld;
						pCur = pPrev;
					}
				}
			} while (pCur->m_pActivePrev != pStop);
		}
	}
}

// hover.exe:0x406D00
// FUNCTION: HOVER 0x406d00
void CFlatRunHeap::AddSkyRun(DWORD dwA, DWORD dwB)
{
	FLATNODE* pNode;
	WORD* pw;
	WORD** ppw;
	DWORD dwCount;

	pNode = m_pSkyNode;
	dwCount = pNode->m_dwCurRuns;
	if ((int)dwCount < 0x28)
	{
		ppw = pNode->m_ppCurRuns;
		pNode->m_dwCurRuns = ++dwCount;
		pw = *ppw + (dwCount - 1) * 2;
		pw[1] = (WORD)dwB;
		pw[0] = (WORD)dwA;
	}
}
