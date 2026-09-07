// texmisc.h : palette shade tables, the DRONE_ texture cache, and the
// resource/archive error reporter
//
// Reconstructed from hover.exe; see texmisc.cpp for the implementations.
// The CMerlinWorld level manager itself is declared in gameobj.h;
// its constructor, destructor, and palette methods are implemented in
// texmisc.cpp.

#ifndef __TEXMISC_H__
#define __TEXMISC_H__

/////////////////////////////////////////////////////////////////////////////
// CSub414550: palette shade table (0x4000 bytes: 64 shade levels of 256
// palette indices each). Two instances are embedded in CBumperDoc at
// +0x18C and +0x418C (inside m_pad018C) and are rebuilt from the level
// palette by CBumperDoc::Sub_415450.

class CSub414550
{
public:
	// hover.exe:0x414550 build the shade table from pPalette: for each of
	// the 64 levels the three scale factors are multiplied by dblBase
	// (darkening), and every palette entry's scaled/clamped RGB is mapped
	// to its nearest in-game color index by Hover_sub_413730. NOTE: the
	// original passes the blue limit/scale before the green ones.
	void Sub_414550(CPalette* pPalette, double dblBase, int nMaxR, int nMaxB,
		int nMaxG, double dblScaleR, double dblScaleB, double dblScaleG);

	BYTE m_abTable[0x4000];	// +0x00 [level * 0x100 + color] = shaded index
};							// sizeof = 0x4000

/////////////////////////////////////////////////////////////////////////////
// Globals defined in texmisc.cpp

extern CObject* g_apUnk_4A4620[0x20];	// hover.exe:0x4A4620 "DRONE_%02d"
									// texture cache (Hover_sub_422370)
extern char g_szUnk_460060[0x400];	// hover.exe:0x460060 error message
									// scratch buffer (Hover_sub_4262B0);
									// the size is not verified against the
									// binary
extern const char* g_apszUnk_4C4910[5];	// hover.exe:0x4C4910 texture family
									// base names ("HUMAN_", "ROBOT_",
									// "FLAG_HUMAN_", "FLAG_ROBOT_",
									// "POD_RANDOM_")

/////////////////////////////////////////////////////////////////////////////
// Free functions defined in texmisc.cpp

// hover.exe:0x422370 cache the "DRONE_00".."DRONE_30" textures in
// g_apUnk_4A4620 (called at the start of the default-maze builder)
void Hover_sub_422370();

// hover.exe:0x4262B0 report a load/resource exception: format the OS error
// text and the MFC exception message into g_szUnk_460060, show it in a
// system-modal message box, and abort
void Hover_sub_4262B0(CException* pException);

/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////

#endif // __TEXMISC_H__
