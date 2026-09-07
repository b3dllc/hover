// stubs.cpp : stub infrastructure for game-specific functions.
//
// Every function that has not yet been decompiled should be clearly
// marked with a "// STUB:" comment and call Hover_StubAlert() so that
// unimplemented code is visible both in the source and at runtime (via
// a message box).
//
// Note: the "// STUB: HOVER 0x..." reccmp annotation must be the line
// immediately above the function signature; reccmp's parser treats any
// comment between a marker and the signature as the function's name.
//
// Remaining stubs live next to the classes they belong to. Stubs are
// parked here rather than beside their class when the collateral
// register-allocation churn in a file full of decompiled code is not
// wanted (see notes/perfect-match.md rule 14).

#include "hover.h"
#include "texture.h"
#include "gameobj.h"
#include "texheap.h"
#include "bumpview.h"
#include "bumperdoc.h"		// CSub425050
#include "texload.h"		// CCollider
#include "mazeobj.h"		// CPodObj::Sub_42AD40
#include "optionsdlg.h"
#include "mazeobj.h"

#include <string.h>

void Hover_StubAlert(const char* funcName, DWORD addr)
{
	char buf[256];
	wsprintfA(buf, "STUB: %s\n\nhover.exe:0x%08lX", funcName, addr);
	MessageBoxA(NULL, buf, "Hover! Decompilation - Unimplemented Function",
		MB_ICONWARNING | MB_OK);
}

/////////////////////////////////////////////////////////////////////////////
// Subsystem helpers reached from the per-frame game step
// (CBumperView::Sub_408D00 / Sub_427D40).

// The MIDI stream (re)open path taken by CSub425050::Sub_413710 when the
// object has no open device: it calls the resolved midiStreamOpen /
// midiOutPrepareHeader / midiStreamProperty / midiStreamOut entry points on
// the buffer list at m_dwUnk10.



