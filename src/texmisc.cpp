// texmisc.cpp : palette shade tables, the CMerlinWorld manager methods,
// the DRONE_ texture cache, and the resource/archive error reporter
//
// Reconstructed from hover.exe; see texmisc.h and gameobj.h for the class
// declarations.

#include "hover.h"
#include "texture.h"
#include "gameobj.h"
#include "bumperdoc.h"
#include "bumpview.h"
#include "texmisc.h"
#include "texload.h"

IMPLEMENT_SERIAL(CMerlinWorld, CObject, 1)

/////////////////////////////////////////////////////////////////////////////
// Globals

// hover.exe:0x4A4620
// GLOBAL: HOVER 0x4a4620
CObject* g_apUnk_4A4620[0x20];	// "DRONE_%02d" texture cache
								// (Hover_sub_422370)

// hover.exe:0x460060
// GLOBAL: HOVER 0x460060
char g_szUnk_460060[0x400];		// error message scratch buffer
								// (Hover_sub_4262B0); the size is not
								// verified against the binary

// hover.exe:0x4C4910
// GLOBAL: HOVER 0x4c4910
const char* g_apszUnk_4C4910[5] =	// texture family base names counted by
{									// CBumperDoc::Sub_417C10
	"HUMAN_",
	"ROBOT_",
	"FLAG_HUMAN_",
	"FLAG_ROBOT_",
	"POD_RANDOM_"
};

/////////////////////////////////////////////////////////////////////////////
// CMerlinWorld

// The constructor doubles as the level-load reset: after initializing the
// four tracked-object arrays it deletes every loaded texture and empties
// the global texture array.

// hover.exe:0x418AE0
// FUNCTION: HOVER 0x418ae0
CMerlinWorld::CMerlinWorld()
{
	int i;
	CObject* pObj;

	m_arrUnk14.SetSize(0);
	for (i = 0; i < g_obArrUnk_4A1D08.GetSize(); i++)
	{
		pObj = g_obArrUnk_4A1D08[i];
		if (pObj != NULL)
			delete pObj;
	}
	g_obArrUnk_4A1D08.SetSize(0);
	m_arrUnk28.SetSize(0);
}

// hover.exe:0x418C00 (the vtable slot 1 deleting destructor at 0x419190
// wraps this body)
// FUNCTION: HOVER 0x418c00
CMerlinWorld::~CMerlinWorld()
{
	int i;
	int n;
	CObject* pObj;

	n = m_arrUnk14.GetSize();
	for (i = 0; i < n; i++)
	{
		pObj = m_arrUnk14[i];
		if (pObj != NULL)
			delete pObj;
	}
	m_arrUnk14.SetSize(0);
	n = m_arrUnk28.GetSize();
	for (i = 0; i < n; i++)
	{
		pObj = m_arrUnk28[i];
		if (pObj != NULL)
			delete pObj;
	}
	m_arrUnk28.SetSize(0);
	n = m_arrUnk3C.GetSize();
	for (i = 0; i < n; i++)
	{
		pObj = m_arrUnk3C[i];
		if (pObj != NULL)
			delete pObj;
	}
	m_arrUnk3C.SetSize(0);
	n = g_obArrUnk_4A1D08.GetSize();
	for (i = 0; i < n; i++)
	{
		pObj = g_obArrUnk_4A1D08[i];
		if (pObj != NULL)
			delete pObj;
	}
	g_obArrUnk_4A1D08.SetSize(0);
	// Detach, not DeleteObject: the destructor only lets go of the palette
	// handle here, it does not destroy the GDI object (hover.exe:0x418D07).
	g_paletteUnk_4A1D50.Detach();
	n = m_arrUnk50.GetSize();
	for (i = 0; i < n; i++)
	{
		pObj = m_arrUnk50[i];
		if (pObj != NULL)
			delete pObj;
	}
	m_arrUnk50.SetSize(0);
}

// hover.exe:0x418DC0 (vtable slot 2) load the maze from the archive.
// Reads four WORD header fields (in the order +0x08, +0x04, +0x0C, +0x10)
// and then serializes the four object arrays: the CMerlinStatic faces,
// the CMerlinDynamic sprites, the CMerlinLocation points and the
// CMerlinBSP nodes. CObArray::Serialize reads each element back through
// CArchive::ReadObject, which is why all four element classes carry
// IMPLEMENT_SERIAL(.., .., 1).
// FUNCTION: HOVER 0x418dc0
void CMerlinWorld::Serialize(CArchive& ar)
{
	CMerlinArchive* pAr = (CMerlinArchive*)&ar;

	if (!ar.IsLoading())
		return;

	pAr->ReadWord(&m_wUnk08);
	pAr->ReadWord(&m_wUnk04);
	pAr->ReadWord(&m_wUnk0C);
	pAr->ReadWord(&m_wUnk10);

	m_arrUnk14.Serialize(ar);
	m_arrUnk28.Serialize(ar);
	m_arrUnk3C.Serialize(ar);
	m_arrUnk50.Serialize(ar);
}

// Sums the pixel-data byte counts of every texture frame at or above the
// g_dwUnk_461CFC mip level over all loaded textures. The result is
// computed entirely from globals; the original is a method only because
// the level loader calls it on the manager.

// hover.exe:0x418EA0
// FUNCTION: HOVER 0x418ea0
DWORD CMerlinWorld::Sub_418EA0()
{
	DWORD dwTotal;
	int i;
	int j;
	CMerlinTexture* pTexture;

	dwTotal = 0;
	for (i = 0; i < g_obArrUnk_4A1D08.GetSize(); i++)
	{
		pTexture = (CMerlinTexture*)g_obArrUnk_4A1D08[i];
		for (j = 0; j < (int)pTexture->m_dwUnk2C; j++)
		{
			if (j >= (int)g_dwUnk_461CFC)
				dwTotal += pTexture->m_pUnk28[j].m_dwUnk14;
		}
	}
	return dwTotal;
}

// Realizes an all-black PC_NOCOLLAPSE palette on the display and deletes
// it again, forcing Windows to rebuild the system palette.

// hover.exe:0x418F00
// FUNCTION: HOVER 0x418f00
void CMerlinWorld::Sub_418F00()
{
	// the original frame is exactly 0x404 bytes: a LOGPALETTE header plus
	// 256 entries
	struct tagFULLPALETTE
	{
		WORD palVersion;
		WORD palNumEntries;
		PALETTEENTRY palPalEntry[0x100];
	} logpal;
	HDC hDC;
	HPALETTE hPal;
	HPALETTE hOldPal;
	int i;

	logpal.palVersion = 0x300;
	logpal.palNumEntries = 0x100;
	memset(logpal.palPalEntry, 0, sizeof(logpal.palPalEntry));
	for (i = 0; i < 0x100; i++)
	{
		logpal.palPalEntry[i].peRed = 0;
		logpal.palPalEntry[i].peGreen = 0;
		logpal.palPalEntry[i].peBlue = 0;
		logpal.palPalEntry[i].peFlags = PC_NOCOLLAPSE;
	}
	hDC = GetDC(NULL);
	hPal = CreatePalette((LOGPALETTE*)&logpal);
	if (hPal != NULL)
	{
		hOldPal = SelectPalette(hDC, hPal, FALSE);
		RealizePalette(hDC);
		DeleteObject(SelectPalette(hDC, hOldPal, FALSE));
	}
	ReleaseDC(NULL, hDC);
}

// Rebuilds g_paletteUnk_4A1D50 around the system palette: the static
// entries at both ends keep their colors (flags cleared), the dynamic
// middle entries take the game palette colors marked PC_NOCOLLAPSE, and
// any duplicate colors in entries 10..0xF6 are nudged apart so each gets
// its own realized slot. The red/blue swap done on the way in is undone
// on the way out.

// hover.exe:0x418FA0
// FUNCTION: HOVER 0x418fa0
void CMerlinWorld::Sub_418FA0()
{
	PALETTEENTRY aEntries[0x100];
	PALETTEENTRY aGame[0x100];
	PALETTEENTRY aSystem[0x100];
	HDC hDC;
	int nHalf;
	int nEnd;
	int i;
	int j;
	BYTE bRed;
	BYTE bGreen;
	BYTE bBlue;

	Sub_418F00();
	g_paletteUnk_4A1D50.GetPaletteEntries(0, 0x100, aGame);
	hDC = GetDC(NULL);
	nHalf = GetDeviceCaps(hDC, NUMCOLORS);
	GetSystemPaletteEntries(hDC, 0, 0x100, aSystem);
	for (i = 0; i < 0x100; i++)
	{
		aEntries[i].peBlue = aSystem[i].peRed;
		aEntries[i].peRed = aSystem[i].peBlue;
		aEntries[i].peGreen = aSystem[i].peGreen;
		aEntries[i].peFlags = aSystem[i].peFlags;
	}
	nHalf = nHalf / 2;
	i = 0;
	if (nHalf > 0)
	{
		for (i = 0; i < nHalf; i++)
			aEntries[i].peFlags = 0;
	}
	nEnd = 0x100 - nHalf;
	for (; i < nEnd; i++)
	{
		aEntries[i].peRed = aGame[i].peBlue;
		aEntries[i].peGreen = aGame[i].peGreen;
		aEntries[i].peBlue = aGame[i].peRed;
		aEntries[i].peFlags = PC_NOCOLLAPSE;
	}
	for (; i < 0x100; i++)
		aEntries[i].peFlags = 0;
	for (i = 0xA; i < 0xF7; i++)
	{
		bBlue = aEntries[i].peBlue;
		bGreen = aEntries[i].peGreen;
		bRed = aEntries[i].peRed;
		for (j = i + 1; j < 0x100; j++)
		{
			if (aEntries[j].peBlue == bBlue &&
				aEntries[j].peGreen == bGreen &&
				aEntries[j].peRed == bRed)
			{
				if (bRed == 0xFF)
					bRed -= 4;
				else
					bRed++;
				aEntries[i].peRed = bRed;
				i = 9;		// restart the scan from entry 10
				break;
			}
		}
	}
	for (i = 0; i < 0x100; i++)
	{
		aGame[i].peRed = aEntries[i].peBlue;
		aGame[i].peGreen = aEntries[i].peGreen;
		aGame[i].peBlue = aEntries[i].peRed;
		aGame[i].peFlags = aEntries[i].peFlags;
	}
	g_paletteUnk_4A1D50.SetPaletteEntries(0, 0x100, aGame);
	ReleaseDC(NULL, hDC);
}

/////////////////////////////////////////////////////////////////////////////
// CSub414550

// hover.exe:0x414550
// FUNCTION: HOVER 0x414550
void CSub414550::Sub_414550(CPalette* pPalette, double dblBase, int nMaxR,
	int nMaxB, int nMaxG, double dblScaleR, double dblScaleB, double dblScaleG)
{
	PALETTEENTRY aEntries[0x100];
	int nLevel;
	int i;
	int nRed;
	int nGreen;
	int nBlue;

	pPalette->GetPaletteEntries(0, 0x100, aEntries);
	for (nLevel = 0; nLevel < 0x40; nLevel++)
	{
		dblScaleR *= dblBase;
		dblScaleG *= dblBase;
		dblScaleB *= dblBase;
		for (i = 0; i < 0x100; i++)
		{
			nRed = (int)(aEntries[i].peRed * dblScaleR);
			if (nRed > nMaxR)
				nRed = nMaxR;
			nGreen = (int)(aEntries[i].peGreen * dblScaleG);
			if (nGreen > nMaxG)
				nGreen = nMaxG;
			nBlue = (int)(aEntries[i].peBlue * dblScaleB);
			if (nBlue > nMaxB)
				nBlue = nMaxB;
			m_abTable[nLevel * 0x100 + i] =
				(BYTE)Hover_sub_413730(RGB(nRed, nGreen, nBlue),
				(const BYTE*)aEntries);
		}
		if (g_pfnUnk_4C4DAC != NULL)
			g_pfnUnk_4C4DAC(nLevel, 0x40);
	}
}

/////////////////////////////////////////////////////////////////////////////
// CBumperDoc

// Counts the consecutive "%s%02d" texture variants of each of the five
// g_apszUnk_4C4910 families ("HUMAN_00", "HUMAN_01", ... until the first
// missing name) into m_adwUnk83D0.

// hover.exe:0x417C10
// FUNCTION: HOVER 0x417c10
void CBumperDoc::Sub_417C10()
{
	char szName[0x80];
	int i;

	for (i = 0; i < 5; i++)
	{
		m_adwUnk83D0[i] = 0;
		for (;;)
		{
			sprintf(szName, "%s%02d", g_apszUnk_4C4910[i],
				m_adwUnk83D0[i]);
			// A temporary, not a named CString: the original passes the
			// constructor's return value straight on (`push eax`), as
			// CBumperDoc::Sub_417D40 already does for the same call.
			if (Hover_sub_40FE40(CString(szName)) == NULL)
				break;
			m_adwUnk83D0[i]++;
		}
	}
}

// Picks a random texture of the nUnk family ("HUMAN_%02d" for 0,
// "ROBOT_%02d" for 1) whose position cell is free according to the
// m_dwUnk8388 grid; retries until Sub_40F9B0 reports a free spot. Returns
// NULL when the family has no loaded textures.

// hover.exe:0x417D40
// FUNCTION: HOVER 0x417d40
CObject* CBumperDoc::Sub_417D40(int nUnk)
{
	char szName[0x80];
	int nRange;
	CObject* pObj;
	CMerlinLocation* pLoc;
	POINT pt;

	nRange = 0x50;
	for (;;)
	{
		sprintf(szName, "%s%02d", g_apszUnk_4C4910[nUnk],
			rand() % (int)m_adwUnk83D0[nUnk]);
		// A temporary, not a named CString: the original passes the
		// constructor's return value straight on (`push eax`) and
		// destroys it before the null test (hover.exe:0x417DAF).
		pObj = Hover_sub_40FE40(CString(szName));
		if (pObj == NULL)
			return NULL;
		// the point's x/y words (g_mapUnk_4A4768 holds CMerlinLocations)
		pLoc = (CMerlinLocation*)pObj;
		pt.x = (short)pLoc->m_wUnk14;
		pt.y = (short)pLoc->m_wUnk16;
		if (((CCollider*)m_dwUnk8388)->Sub_40F9B0(pt, nRange) == 0)
			return pObj;
	}
}

/////////////////////////////////////////////////////////////////////////////
// CGameObj

// Clears the dynamic state fields and restamps m_dwUnk34; called by
// CPlayer::Sub_41B2D0 when the player is placed at a start position.

// hover.exe:0x41A580
// FUNCTION: HOVER 0x41a580
void CGameObj::Sub_41A580()
{
	m_dwUnk38 = 0;
	m_dwUnk24 = 0;
	m_dwUnk30 = 0;
	m_dwUnk5C = 0;
	m_dwUnk60 = 0;
	m_dwUnk78 = 0;
	m_dwUnk80 = 0;
	m_dwUnk7C = 0;
	m_dwUnk70 = 0;
	m_dwUnk74 = 0;
	m_dwUnk68 = 0;
	m_dwUnk6C = 0;
	m_dwUnk64 = 0;
	m_dwUnk34 = GetTickCount();
}

/////////////////////////////////////////////////////////////////////////////
// Free functions

// Caches the "DRONE_00".."DRONE_31" textures in g_apUnk_4A4620; called at
// the start of the default-maze builder.

// hover.exe:0x422370
// FUNCTION: HOVER 0x422370
void Hover_sub_422370()
{
	char szName[0xC];
	int i;

	// 0x20 frames, not 0x1F: the original increments the counter before
	// it forms the array address, so the last pass caches "DRONE_31" into
	// g_apUnk_4A4620[0x1F] (hover.exe:0x422387/0x422392/0x4223AB).
	for (i = 0; i < 0x20; i++)
	{
		wsprintf(szName, "DRONE_%02d", i);
		g_apUnk_4A4620[i] = NULL;
		g_mapUnk_4A1D30.Lookup(szName, g_apUnk_4A4620[i]);
	}
}

// Reports a load/resource exception: selects the message for the MFC
// exception class, prepends the OS error text when one is available, shows
// the result in a system-modal message box, and aborts.

// hover.exe:0x4262B0
// FUNCTION: HOVER 0x4262b0
void Hover_sub_4262B0(CException* pException)
{
	LPSTR pszOsError = NULL;
	DWORD dwError;
	DWORD dwLength;
	const char* pszMessage;

	dwError = GetLastError();
	pszMessage = g_strUnexpectedException;
	if (pException->IsKindOf(RUNTIME_CLASS(CFileException)))
	{
		pszMessage = g_strReadFileError;
		dwError = ((CFileException*)pException)->m_lOsError;
	}
	if (pException->IsKindOf(RUNTIME_CLASS(CMemoryException)))
		pszMessage = g_szMemoryError;
	if (pException->IsKindOf(RUNTIME_CLASS(CNotSupportedException)))
		pszMessage = g_strExcNotSupported;
	if (pException->IsKindOf(RUNTIME_CLASS(CArchiveException)))
	{
		dwError = 0;
		pszMessage = g_strExcArchiveGeneric;
		// hover.exe tests the cause against 0, 2, 3, 4, 5, 6, 7 as a
		// sequence of independent, non-exclusive ifs (no jump table, no
		// jump to a shared end - it just keeps testing after a match,
		// which is harmless since only one test can be true); there is
		// no case for CArchiveException::generic == 1, which leaves the
		// message set above. A switch or an else-if chain compiles to a
		// different shape and does not match.
		if (((CArchiveException*)pException)->m_cause == CArchiveException::none)
			pszMessage = g_strExcArchiveNone;
		if (((CArchiveException*)pException)->m_cause == CArchiveException::readOnly)
			pszMessage = g_strExcArchiveReadOnly;
		if (((CArchiveException*)pException)->m_cause == CArchiveException::endOfFile)
			pszMessage = g_strExcArchiveEndOfFile;
		if (((CArchiveException*)pException)->m_cause == CArchiveException::writeOnly)
			pszMessage = g_strExcArchiveWriteOnly;
		if (((CArchiveException*)pException)->m_cause == CArchiveException::badIndex)
			pszMessage = g_strExcArchiveBadIndex;
		if (((CArchiveException*)pException)->m_cause == CArchiveException::badClass)
			pszMessage = g_strExcArchiveBadClass;
		if (((CArchiveException*)pException)->m_cause == CArchiveException::badSchema)
			pszMessage = g_strExcArchiveBadSchema;
	}
	if (pException->IsKindOf(RUNTIME_CLASS(CResourceException)))
		pszMessage = g_strExcResource;
	if (pException->IsKindOf(RUNTIME_CLASS(CUserException)))
		pszMessage = g_strExcUser;
	g_szUnk_460060[0] = '\0';
	dwLength = 0;
	if (dwError != 0)
		dwLength = FormatMessage(
			FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
			NULL, dwError, 0, (LPSTR)&pszOsError, 0, NULL);
	if (dwLength != 0 && dwLength < 0x100)
		strcat(g_szUnk_460060, pszOsError);
	if (pszMessage != NULL)
	{
		if (dwLength != 0)
			strcat(g_szUnk_460060, "\n");
		strcat(g_szUnk_460060, pszMessage);
	}
	if (pszOsError != NULL)
		LocalFree(pszOsError);
	MessageBox(NULL, g_szUnk_460060, NULL, MB_ICONSTOP | MB_SYSTEMMODAL);
	abort();
}

/////////////////////////////////////////////////////////////////////////////
