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
// Globals defined in texmisc.cpp

extern CObject* g_apDroneFrames[0x20];	// hover.exe:0x4A4620 "DRONE_%02d"
									// texture cache (Hover_CacheDroneFrames)
extern char g_szFatalError[0x400];	// hover.exe:0x460060 error message
									// scratch buffer (Hover_FatalException);
									// the size is not verified against the
									// binary
extern const char* g_apszSpawnFamilies[5];	// hover.exe:0x4C4910 texture family
									// base names ("HUMAN_", "ROBOT_",
									// "FLAG_HUMAN_", "FLAG_ROBOT_",
									// "POD_RANDOM_")

/////////////////////////////////////////////////////////////////////////////
// Free functions defined in texmisc.cpp

// hover.exe:0x422370 cache the "DRONE_00".."DRONE_30" textures in
// g_apDroneFrames (called at the start of the default-maze builder)
void Hover_CacheDroneFrames();

// hover.exe:0x4262B0 report a load/resource exception: format the OS error
// text and the MFC exception message into g_szFatalError, show it in a
// system-modal message box, and abort
void Hover_FatalException(CException* pException);

/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////

#endif // __TEXMISC_H__
