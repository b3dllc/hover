# Makefile for Hover! (Windows 95) decompilation project.
# Built with Microsoft Visual C++ 2.2 under Wine (see build.sh).

CC      = cl
LINK    = link
RC      = rc

# /i: let RC find the bitmap files next to hover.rc.
# The remaining /d defines are window styles used by the extracted dialog
# templates that are not defined in the VC++ 2.2 winres.h extract that
# afxres.h pulls in (values from commctrl.h / winuser.h).
RFLAGS  = /i $(RESDIR) /d "DS_CONTEXTHELP=0x2000" /d "TBS_HORZ=0" \
          /d "TBS_AUTOTICKS=1" /d "TBS_BOTH=8" /d "UDS_SETBUDDYINT=2" \
          /d "UDS_ALIGNRIGHT=4" /d "UDS_ARROWKEYS=0x20"

CFLAGS  = /nologo /MT /W3 /GX /O2 /Zi /D "WIN32" /D "_WINDOWS" /D "_AFX" \
          /D "_MBCS" /FR$(INTDIR)\ /Fo$(INTDIR)\ /Fd$(INTDIR)\

LFLAGS  = /nologo /subsystem:windows /machine:I386 /incremental:no \
          /debug /debugtype:cv /pdb:$(INTDIR)\hover.pdb /map:$(INTDIR)\hover.map

LIBS    = NAFXCW.LIB LIBCMT.LIB KERNEL32.LIB USER32.LIB GDI32.LIB \
          SHELL32.LIB COMCTL32.LIB ADVAPI32.LIB comdlg32.LIB WINMM.LIB \
          WINSPOOL.LIB

SRCDIR  = src
RESDIR  = res
INTDIR  = build

OBJS    = $(INTDIR)\hover.obj $(INTDIR)\stubs.obj $(INTDIR)\mainfrm.obj \
          $(INTDIR)\bumperdoc.obj $(INTDIR)\bumpview.obj $(INTDIR)\splashdlg.obj \
          $(INTDIR)\highscrdlg.obj $(INTDIR)\gameobj.obj $(INTDIR)\texture.obj \
          $(INTDIR)\texheap.obj $(INTDIR)\mazeobj.obj $(INTDIR)\texload.obj \
          $(INTDIR)\texmisc.obj \
          $(INTDIR)\spandrawA.obj \
          $(INTDIR)\spandrawB.obj \
          $(INTDIR)\spandrawC.obj \
          $(INTDIR)\optionsdlg.obj
#         NOTE: optionsdlg.obj is deliberately last. nmake compiles the
#         objects in the order they appear here and VC 2.2's register
#         allocation in the *other* translation units shifts with that
#         order (see notes/perfect-match.md); building it last scores
#         0.08 points better overall than building it after highscrdlg.
RES     = $(INTDIR)\hover.res

TARGET  = $(INTDIR)\hover.exe

all: $(INTDIR) $(TARGET)

$(TARGET): $(OBJS) $(RES)
	$(LINK) $(LFLAGS) $(OBJS) $(RES) /out:$(TARGET) $(LIBS)

$(INTDIR)\hover.obj: $(SRCDIR)\hover.cpp $(SRCDIR)\texture.h $(SRCDIR)\hover.h $(SRCDIR)\mainfrm.h $(SRCDIR)\bumperdoc.h $(SRCDIR)\bumpview.h $(SRCDIR)\splashdlg.h $(SRCDIR)\highscrdlg.h $(SRCDIR)\optionsdlg.h $(SRCDIR)\texload.h $(SRCDIR)\resource.h $(SRCDIR)\gameobj.h
	$(CC) $(CFLAGS) /c $(SRCDIR)\hover.cpp

$(INTDIR)\stubs.obj: $(SRCDIR)\stubs.cpp $(SRCDIR)\hover.h $(SRCDIR)\texture.h $(SRCDIR)\gameobj.h $(SRCDIR)\texheap.h $(SRCDIR)\bumpview.h $(SRCDIR)\bumperdoc.h $(SRCDIR)\texload.h $(SRCDIR)\optionsdlg.h $(SRCDIR)\resource.h
	$(CC) $(CFLAGS) /c $(SRCDIR)\stubs.cpp

$(INTDIR)\mainfrm.obj: $(SRCDIR)\mainfrm.cpp $(SRCDIR)\hover.h $(SRCDIR)\mainfrm.h $(SRCDIR)\bumperdoc.h $(SRCDIR)\bumpview.h $(SRCDIR)\highscrdlg.h $(SRCDIR)\resource.h $(SRCDIR)\gameobj.h $(SRCDIR)\texture.h
	$(CC) $(CFLAGS) /c $(SRCDIR)\mainfrm.cpp

$(INTDIR)\bumperdoc.obj: $(SRCDIR)\bumperdoc.cpp $(SRCDIR)\hover.h $(SRCDIR)\bumperdoc.h $(SRCDIR)\resource.h $(SRCDIR)\gameobj.h $(SRCDIR)\texture.h
	$(CC) $(CFLAGS) /c $(SRCDIR)\bumperdoc.cpp

$(INTDIR)\bumpview.obj: $(SRCDIR)\bumpview.cpp $(SRCDIR)\hover.h $(SRCDIR)\bumpview.h $(SRCDIR)\mazeobj.h $(SRCDIR)\texload.h $(SRCDIR)\texture.h $(SRCDIR)\texheap.h $(SRCDIR)\resource.h $(SRCDIR)\gameobj.h $(SRCDIR)\bumperdoc.h
	$(CC) $(CFLAGS) /c $(SRCDIR)\bumpview.cpp

$(INTDIR)\splashdlg.obj: $(SRCDIR)\splashdlg.cpp $(SRCDIR)\splashdlg.h $(SRCDIR)\hover.h $(SRCDIR)\resource.h
	$(CC) $(CFLAGS) /c $(SRCDIR)\splashdlg.cpp

$(INTDIR)\highscrdlg.obj: $(SRCDIR)\highscrdlg.cpp $(SRCDIR)\hover.h $(SRCDIR)\highscrdlg.h $(SRCDIR)\resource.h
	$(CC) $(CFLAGS) /c $(SRCDIR)\highscrdlg.cpp

$(INTDIR)\optionsdlg.obj: $(SRCDIR)\optionsdlg.cpp $(SRCDIR)\optionsdlg.h $(SRCDIR)\hover.h $(SRCDIR)\bumperdoc.h $(SRCDIR)\bumpview.h $(SRCDIR)\resource.h $(SRCDIR)\gameobj.h $(SRCDIR)\texture.h
	$(CC) $(CFLAGS) /c $(SRCDIR)\optionsdlg.cpp

$(INTDIR)\gameobj.obj: $(SRCDIR)\gameobj.cpp $(SRCDIR)\hover.h $(SRCDIR)\gameobj.h $(SRCDIR)\bumperdoc.h $(SRCDIR)\bumpview.h $(SRCDIR)\texture.h $(SRCDIR)\texload.h $(SRCDIR)\resource.h
	$(CC) $(CFLAGS) /c $(SRCDIR)\gameobj.cpp

$(INTDIR)\texture.obj: $(SRCDIR)\texture.cpp $(SRCDIR)\hover.h $(SRCDIR)\texture.h $(SRCDIR)\gameobj.h $(SRCDIR)\mazeobj.h $(SRCDIR)\bumperdoc.h $(SRCDIR)\bumpview.h $(SRCDIR)\texmisc.h $(SRCDIR)\texload.h $(SRCDIR)\resource.h
	$(CC) $(CFLAGS) /c $(SRCDIR)\texture.cpp

$(INTDIR)\texheap.obj: $(SRCDIR)\texheap.cpp $(SRCDIR)\hover.h $(SRCDIR)\texture.h $(SRCDIR)\gameobj.h $(SRCDIR)\texheap.h $(SRCDIR)\resource.h
	$(CC) $(CFLAGS) /c $(SRCDIR)\texheap.cpp

$(INTDIR)\mazeobj.obj: $(SRCDIR)\mazeobj.cpp $(SRCDIR)\hover.h $(SRCDIR)\texture.h $(SRCDIR)\gameobj.h $(SRCDIR)\bumperdoc.h $(SRCDIR)\mazeobj.h $(SRCDIR)\resource.h $(SRCDIR)\texload.h
	$(CC) $(CFLAGS) /c $(SRCDIR)\mazeobj.cpp

$(INTDIR)\texload.obj: $(SRCDIR)\texload.cpp $(SRCDIR)\hover.h $(SRCDIR)\bumperdoc.h $(SRCDIR)\bumpview.h $(SRCDIR)\gameobj.h $(SRCDIR)\texture.h $(SRCDIR)\texload.h $(SRCDIR)\mazeobj.h $(SRCDIR)\resource.h
	$(CC) $(CFLAGS) /c $(SRCDIR)\texload.cpp

$(INTDIR)\texmisc.obj: $(SRCDIR)\texmisc.cpp $(SRCDIR)\hover.h $(SRCDIR)\texture.h $(SRCDIR)\gameobj.h $(SRCDIR)\bumperdoc.h $(SRCDIR)\bumpview.h $(SRCDIR)\texmisc.h $(SRCDIR)\texload.h $(SRCDIR)\resource.h
	$(CC) $(CFLAGS) /c $(SRCDIR)\texmisc.cpp

$(INTDIR)\spandrawA.obj: $(SRCDIR)\spandrawA.cpp $(SRCDIR)\hover.h $(SRCDIR)\texture.h $(SRCDIR)\gameobj.h $(SRCDIR)\texheap.h $(SRCDIR)\spandraw.h
	$(CC) $(CFLAGS) /c $(SRCDIR)\spandrawA.cpp

$(INTDIR)\spandrawB.obj: $(SRCDIR)\spandrawB.cpp $(SRCDIR)\hover.h $(SRCDIR)\texture.h $(SRCDIR)\gameobj.h $(SRCDIR)\texheap.h $(SRCDIR)\spandraw.h
	$(CC) $(CFLAGS) /c $(SRCDIR)\spandrawB.cpp

$(INTDIR)\hover.res: $(RESDIR)\hover.rc
	$(RC) $(RFLAGS) /r /fo $(INTDIR)\hover.res $(RESDIR)\hover.rc

clean:
	-del /q $(INTDIR)\*.obj $(INTDIR)\*.res $(INTDIR)\*.exe $(INTDIR)\*.sbr $(INTDIR)\*.pdb $(INTDIR)\*.map $(INTDIR)\*.ilk 2>nul

$(INTDIR):
	-md $(INTDIR)

$(INTDIR)\spandrawC.obj: $(SRCDIR)\spandrawC.cpp $(SRCDIR)\hover.h $(SRCDIR)\texture.h $(SRCDIR)\gameobj.h $(SRCDIR)\texheap.h $(SRCDIR)\spandraw.h $(SRCDIR)\resource.h
	$(CC) $(CFLAGS) /c $(SRCDIR)\spandrawC.cpp
