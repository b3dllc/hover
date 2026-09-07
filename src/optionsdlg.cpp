// optionsdlg.cpp : implementation of the option and debug dialogs
//
// Reconstructed from hover.exe; see optionsdlg.h for the vtable/message
// map data and the notes on member offsets.
//
// The four parameter dialogs at the end run on templates that are not in
// the shipped hover.exe (130, 148, 149 and 152), so their DoModal() always
// fails; they are decompiled for completeness.

#include "hover.h"
#include "bumperdoc.h"
#include "bumpview.h"
#include "optionsdlg.h"

#include <math.h>
#include <string.h>	// _stricmp

/////////////////////////////////////////////////////////////////////////////
// The three parameter blocks the debug dialogs edit hold 16.16 fixed-point
// numbers. hover.exe rounds them to four decimals on the way in (the high
// word contributes 10000 per unit, the low word 10000/65536 per 1/65536)
// and truncates them straight back on the way out.

// The value is read through a stack copy, its two halves separately: the
// integer part as a signed short straight out of memory, the fraction out
// of the register the copy came from (that is what hover.exe's frame and
// instruction selection show).

typedef union tagFIXED1616
{
	DWORD dw;
	struct
	{
		WORD wFrac;		// +0 fraction (1/65536 units)
		short sInt;		// +2 signed integer part
	} s;
} FIXED1616;

// The integer term has to be named before the sum: written as one
// expression the compiler schedules the (unsigned, and so 64-bit) fraction
// conversion first, while hover.exe converts the integer part first
// everywhere.  Naming it also makes the schedule independent of where the
// caller sits in the file (cf. the position dependence of rule 76).

static __inline double FixToDbl(FIXED1616 fx)
{
	double dInt = (double)fx.s.sInt * 10000.0;

	return floor(dInt + (double)(fx.dw & 0xFFFF) * 0.152587890625 + 0.5)
		* 0.0001;
}

#define FIX_TO_DBL(fx)	FixToDbl(fx)
#define DBL_TO_FIX(d)	((long)((d) * 65536.0))

// hover.exe:0x42E420 (defined below, next to its only caller)
void Hover_sub_42E420();

// joy.cpl's exported joystick calibration entry point
typedef void (WINAPI* LPSHOWJOYCPL)(HWND hWnd);

/////////////////////////////////////////////////////////////////////////////
// Key display name lookup table used by Hover_sub_42DC50 and by the key
// combo boxes of the Player Controls / Keyboard Settings dialogs: string
// resource ID of the key's display name -> virtual key code,
// zero-terminated (hover.exe:0x4BDAC8).

typedef struct tagKEYNAME
{
	UINT nID;		// string resource ID (IDS_KEY_*)
	DWORD dwVK;		// virtual key code
} KEYNAME;

static const KEYNAME g_aKeyNames[] =
{
	{ IDS_KEY_BACKSPACE, VK_BACK },
	{ IDS_KEY_TAB, VK_TAB },
	{ IDS_KEY_ENTER, VK_RETURN },
	{ IDS_KEY_SHIFT, VK_SHIFT },
	{ IDS_KEY_CONTROL, VK_CONTROL },
	{ IDS_KEY_SPACE, VK_SPACE },
	{ IDS_KEY_PGUP, VK_PRIOR },
	{ IDS_KEY_PGDN, VK_NEXT },
	{ IDS_KEY_END, VK_END },
	{ IDS_KEY_HOME, VK_HOME },
	{ IDS_KEY_LEFT, VK_LEFT },
	{ IDS_KEY_UP, VK_UP },
	{ IDS_KEY_RIGHT, VK_RIGHT },
	{ IDS_KEY_DOWN, VK_DOWN },
	{ IDS_KEY_INSERT, VK_INSERT },
	{ IDS_KEY_DELETE, VK_DELETE },
	{ IDS_KEY_NUMPAD0, VK_NUMPAD0 },
	{ IDS_KEY_NUMPAD1, VK_NUMPAD1 },
	{ IDS_KEY_NUMPAD2, VK_NUMPAD2 },
	{ IDS_KEY_NUMPAD3, VK_NUMPAD3 },
	{ IDS_KEY_NUMPAD4, VK_NUMPAD4 },
	{ IDS_KEY_NUMPAD5, VK_NUMPAD5 },
	{ IDS_KEY_NUMPAD6, VK_NUMPAD6 },
	{ IDS_KEY_NUMPAD7, VK_NUMPAD7 },
	{ IDS_KEY_NUMPAD8, VK_NUMPAD8 },
	{ IDS_KEY_NUMPAD9, VK_NUMPAD9 },
	{ IDS_KEY_MULTIPLY, VK_MULTIPLY },
	{ IDS_KEY_DECIMAL, VK_DECIMAL },
	{ IDS_KEY_DIVIDE, VK_DIVIDE },
	{ 0, 0 },
};

// Resolves a key display name string resource to a virtual-key code.
// Single letters/digits map to their own character code; named keys are
// looked up in g_aKeyNames with an MBCS-aware, case-sensitive compare
// (hover.exe:0x42DD12 calls __mbscmp, not a case-insensitive compare).
// Defaults to 'A' when the resource is missing or the name is not
// recognized.

// hover.exe:0x42DC50
// FUNCTION: HOVER 0x42dc50
int Hover_sub_42DC50(UINT nID)
{
	CString strName;
	CString strKey;
	int i;
	int nKey = 'A';
	if (strKey.LoadString(nID))
	{
		if (strKey.GetLength() == 1)
		{
			char c = strKey[0];
			if ((c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9'))
				return c;
		}
		for (i = 0; g_aKeyNames[i].nID != 0; i++)
		{
			if (strName.LoadString(g_aKeyNames[i].nID) &&
				_mbscmp((const unsigned char*)(const char*)strKey,
					(const unsigned char*)(const char*)strName) == 0)
			{
				nKey = g_aKeyNames[i].dwVK;
				break;
			}
		}
	}
	return nKey;
}

/////////////////////////////////////////////////////////////////////////////
// CSub413F10 dialog ("Customize Game")

// hover.exe:0x4BD5E8
static DWORD s_adwCustomizeHelp[] =
{
	IDC_CUST_SPEEDDETAIL, 2100,
	IDC_CUST_EASY, 2120,
	IDC_CUST_INTERMEDIATE, 2120,
	IDC_CUST_DIFFICULT, 2120,
	IDC_CUST_SEQUENTIAL, 2110,
	IDC_CUST_RANDOM, 2110,
	IDC_CUST_ROTATEMAP, 2130,
	IDC_CUST_ROTATECRAFT, 2130,
	IDC_CUST_REARVIEW, 2140,
	0, 0
};

BEGIN_MESSAGE_MAP(CSub413F10, CDialog)
	//{{AFX_MSG_MAP(CSub413F10)
	ON_MESSAGE(WM_HELP, OnHelpInfo)
	ON_MESSAGE(WM_CONTEXTMENU, OnHelpContextMenu)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// hover.exe:0x413F10
// FUNCTION: HOVER 0x413f10
CSub413F10::CSub413F10(CWnd* pParent) : CDialog(IDD_CUSTOMIZE, pParent)
{
	m_nRearView = theApp.m_dwUnk01D0;
	m_nMazeType = theApp.m_dwUnk01CC;
	m_nSkill = theApp.m_dwUnk01C8;
}

// The radio groups are not exchanged with DDX_Radio: on the way in the
// dialog checks the buttons itself (straight from the settings in theApp,
// not from the members seeded by the constructor) and on the way out OnOK
// reads them back with GetCheckedRadioButton.

// hover.exe:0x40E480
// FUNCTION: HOVER 0x40e480
void CSub413F10::DoDataExchange(CDataExchange* pDX)
{
	//{{AFX_DATA_MAP(CSub413F10)
	DDX_Control(pDX, IDC_CUST_SPEEDDETAIL, m_sliderSpeed);
	DDX_Check(pDX, IDC_CUST_REARVIEW, m_nRearView);
	//}}AFX_DATA_MAP

	if (!pDX->m_bSaveAndValidate)
	{
		::CheckRadioButton(pDX->m_pDlgWnd->m_hWnd, IDC_CUST_EASY,
			IDC_CUST_DIFFICULT, IDC_CUST_EASY + theApp.m_dwUnk01C8);
		::CheckRadioButton(pDX->m_pDlgWnd->m_hWnd, IDC_CUST_SEQUENTIAL,
			IDC_CUST_RANDOM, IDC_CUST_SEQUENTIAL + theApp.m_dwUnk01CC);
		::CheckRadioButton(pDX->m_pDlgWnd->m_hWnd, IDC_CUST_ROTATEMAP,
			IDC_CUST_ROTATECRAFT,
			IDC_CUST_ROTATEMAP + (theApp.m_dwUnk01D4 == 0));
		m_sliderSpeed.SetRange(0, 5);
		::SendMessage(m_sliderSpeed.m_hWnd, TBM_SETPOS, TRUE,
			theApp.m_dwUnk01C4);
	}
}

// hover.exe:0x413FD0
// FUNCTION: HOVER 0x413fd0
BOOL CSub413F10::OnInitDialog()
{
	CDialog::OnInitDialog();
	return TRUE;
}

// Applies the settings. Only a changed "Show Rear View" needs the
// offscreen bitmap resized (Sub_4274B0) and only a changed speed/detail
// slider needs the view's detail tables rebuilt; either one pushes the
// worker thread through a frame.

// hover.exe:0x413FE0
// FUNCTION: HOVER 0x413fe0
void CSub413F10::OnOK()
{
	int nSpeed;
	CBumperView* pView;
	int bChanged;

	UpdateData(TRUE);

	theApp.m_dwUnk01CC = GetCheckedRadioButton(IDC_CUST_SEQUENTIAL,
		IDC_CUST_RANDOM) - IDC_CUST_SEQUENTIAL;
	theApp.m_dwUnk01C8 = GetCheckedRadioButton(IDC_CUST_EASY,
		IDC_CUST_DIFFICULT) - IDC_CUST_EASY;
	theApp.m_dwUnk01D4 = (GetCheckedRadioButton(IDC_CUST_ROTATEMAP,
		IDC_CUST_ROTATECRAFT) - IDC_CUST_ROTATEMAP) == 0;

	nSpeed = (int)::SendMessage(m_sliderSpeed.m_hWnd, TBM_GETPOS, 0, 0);
	bChanged = 0;
	pView = Hover_sub_401140();

	if ((DWORD)m_nRearView != theApp.m_dwUnk01D0)
	{
		DWORD* pdw;
		theApp.m_dwUnk01D0 = m_nRearView;
		pdw = &Hover_sub_401140()->m_dwUnk00A4;
		pView->Sub_4274B0(pdw[0], pdw[1], 1);
		bChanged = 1;
	}

	if ((DWORD)nSpeed != theApp.m_dwUnk01C4)
	{
		bChanged = 1;
		pView->Sub_413A00(nSpeed, bChanged);
		pView->Sub_413B10(bChanged);
		pView->m_dwUnk0080 = bChanged;
	}

	if (bChanged)
		pView->Sub_40DC20(1);

	CDialog::OnOK();
}

// hover.exe:0x42D140
// FUNCTION: HOVER 0x42d140
LRESULT CSub413F10::OnHelpInfo(WPARAM wParam, LPARAM lParam)
{
	return ::WinHelp((HWND)((LPHELPINFO)lParam)->hItemHandle, "hover.hlp",
		HELP_WM_HELP, (DWORD)s_adwCustomizeHelp);
}

// hover.exe:0x42D160
// FUNCTION: HOVER 0x42d160
LRESULT CSub413F10::OnHelpContextMenu(WPARAM wParam, LPARAM lParam)
{
	::WinHelp((HWND)wParam, "hover.hlp", HELP_CONTEXTMENU,
		(DWORD)s_adwCustomizeHelp);
	return 1;
}

/////////////////////////////////////////////////////////////////////////////
// CSub42C890 dialog (debug template 130: camera/view geometry)

BEGIN_MESSAGE_MAP(CSub42C890, CDialog)
	//{{AFX_MSG_MAP(CSub42C890)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// hover.exe:0x42C890
// FUNCTION: HOVER 0x42c890
CSub42C890::CSub42C890(CWnd* pParent) : CDialog(IDD_DBG_130, pParent)
{
	m_dwUnk44 = 0;
	m_dwUnk48 = 0;
	m_dwUnk4C = 0;
	m_dwUnk50 = 0;
	m_dwUnk54 = 0;
	m_dblUnk58 = 0.0;
	m_dblUnk60 = 0.0;
	m_dblUnk68 = 0.0;
}

// hover.exe:0x42C920
// FUNCTION: HOVER 0x42c920
void CSub42C890::DoDataExchange(CDataExchange* pDX)
{
	//{{AFX_DATA_MAP(CSub42C890)
	DDX_Text(pDX, 1008, m_dwUnk44);
	DDV_MinMaxInt(pDX, m_dwUnk44, 0, 5000);
	DDX_Text(pDX, 1007, m_dwUnk48);
	DDV_MinMaxInt(pDX, m_dwUnk48, 0, 5000);
	DDX_Text(pDX, 1010, m_dwUnk4C);
	DDV_MinMaxInt(pDX, m_dwUnk4C, 0, 5000);
	DDX_Text(pDX, 1006, m_dwUnk50);
	DDV_MinMaxInt(pDX, m_dwUnk50, 0, 5000);
	DDX_Text(pDX, 1011, m_dwUnk54);
	DDV_MinMaxInt(pDX, m_dwUnk54, 0, 5000);
	DDX_Text(pDX, 1017, m_dblUnk58);
	DDV_MinMaxDouble(pDX, m_dblUnk58, 0.0, 1.0);
	DDX_Text(pDX, 1019, m_dblUnk60);
	DDV_MinMaxDouble(pDX, m_dblUnk60, 0.0, 1.0);
	DDX_Text(pDX, 1018, m_dblUnk68);
	DDV_MinMaxDouble(pDX, m_dblUnk68, 0.0, 100.0);
	//}}AFX_DATA_MAP
}

/////////////////////////////////////////////////////////////////////////////
// CSub42D910 dialog ("Player Controls")

// hover.exe:0x4BDBB8
static DWORD s_adwControlsHelp[] =
{
	IDC_CTRL_JOYSTICK, 2400,
	IDC_CTRL_KEYBOARD, 2400,
	IDC_CTRL_OBJECTSBOX, 2410,
	IDC_CTRL_LBL_CLOAK, 2410,
	IDC_CTRL_LBL_WALL, 2410,
	IDC_CTRL_LBL_JUMP, 2410,
	IDC_CTRL_WALL, 2410,
	IDC_CTRL_CLOAK, 2410,
	IDC_CTRL_JUMP, 2410,
	IDC_CTRL_SETKEYS, 2420,
	IDC_CTRL_CALIBRATE, 2430,
	IDC_CTRL_DEFAULT, 2440,
	0, 0
};

BEGIN_MESSAGE_MAP(CSub42D910, CDialog)
	//{{AFX_MSG_MAP(CSub42D910)
	ON_COMMAND(IDC_CTRL_SETKEYS, OnSetKeys)
	ON_COMMAND(IDC_CTRL_DEFAULT, OnDefault)
	ON_COMMAND(IDC_CTRL_CALIBRATE, OnCalibrate)
	ON_MESSAGE(WM_HELP, OnHelpInfo)
	ON_MESSAGE(WM_CONTEXTMENU, OnHelpContextMenu)
	ON_COMMAND(IDC_CTRL_JOYSTICK, OnJoystick)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// hover.exe:0x42D910
// FUNCTION: HOVER 0x42d910
CSub42D910::CSub42D910(CWnd* pParent) : CDialog(IDD_PLAYERCONTROLS, pParent)
{
}

// hover.exe:0x42DAE0
// FUNCTION: HOVER 0x42dae0
void CSub42D910::DoDataExchange(CDataExchange* pDX)
{
	//{{AFX_DATA_MAP(CSub42D910)
	DDX_Control(pDX, IDC_CTRL_JOYSTICK, m_btnJoystick);
	DDX_Control(pDX, IDC_CTRL_CALIBRATE, m_btnCalibrate);
	DDX_Control(pDX, IDC_CTRL_WALL, m_comboWall);
	DDX_Control(pDX, IDC_CTRL_JUMP, m_comboJump);
	DDX_Control(pDX, IDC_CTRL_CLOAK, m_comboCloak);
	//}}AFX_DATA_MAP
}

// Sorts a copy of the chosen key codes (insertion sort) and removes the
// duplicates in place; returns whether nothing was removed, i.e. whether
// all nCount bindings are distinct. The caller passes a scratch array.

// hover.exe:0x42DB60
// FUNCTION: HOVER 0x42db60
BOOL CSub42D910::Sub_42DB60(DWORD* avk, int nCount)
{
	int i, j;
	DWORD* p;
	DWORD* pOut;
	DWORD* pEnd;

	if (nCount == 0)
		return FALSE;

	for (i = 1; i < nCount; i++)
	{
		DWORD dwKey = avk[i];
		for (j = i - 1; j >= 0 && dwKey < avk[j]; j--)
			avk[j + 1] = avk[j];
		avk[j + 1] = dwKey;
	}

	pEnd = avk + nCount;
	pOut = avk;
	p = avk + 1;
	if (p < pEnd)
	{
		do
		{
			if (*pOut != *p)
				*++pOut = *p;
			p++;
		} while (p < pEnd);
	}

	return (pOut - avk + 1 - nCount) == 0;
}

// Selects the item of pCombo whose item data is dwVK.

// hover.exe:0x42DBE0
// FUNCTION: HOVER 0x42dbe0
void CSub42D910::Sub_42DBE0(CWnd* pCombo, DWORD dwVK)
{
	int i;

	::SendMessage(pCombo->m_hWnd, CB_SETCURSEL, 0, 0);
	for (i = (int)::SendMessage(pCombo->m_hWnd, CB_GETCOUNT, 0, 0);
		i >= 0; i--)
	{
		if (::SendMessage(pCombo->m_hWnd, CB_GETITEMDATA, i, 0) == (LRESULT)dwVK)
		{
			::SendMessage(pCombo->m_hWnd, CB_SETCURSEL, i, 0);
			break;
		}
	}
}

// Fills a key combo box: "A".."Z" and "0".."9" always, the named keys of
// g_aKeyNames only when bLettersOnly is clear (the three object keys of
// the Player Controls dialog are letters and digits only). Each item
// carries its virtual key code as item data. Finally selects dwVK.

// hover.exe:0x42DD70
// FUNCTION: HOVER 0x42dd70
void CSub42D910::Sub_42DD70(CWnd* pCombo, DWORD dwVK, BOOL bLettersOnly)
{
	CString str;
	const KEYNAME* pKey;
	int i;
	int n;
	volatile DWORD dwPad;	// rule 80 placeholder: hover.exe's frame is one
				// dword bigger than ours and never touches it

	dwPad = 0;
	for (i = 0; i <= 25; i++)
	{
		str.Empty();
		str += (char)('A' + i);
		n = (int)::SendMessage(pCombo->m_hWnd, CB_ADDSTRING, 0, (LPARAM)(LPCSTR)str);
		::SendMessage(pCombo->m_hWnd, CB_SETITEMDATA, n, 'A' + i);
	}

	for (i = 0; i <= 9; i++)
	{
		str.Empty();
		str += (char)('0' + i);
		n = (int)::SendMessage(pCombo->m_hWnd, CB_ADDSTRING, 0, (LPARAM)(LPCSTR)str);
		::SendMessage(pCombo->m_hWnd, CB_SETITEMDATA, n, '0' + i);
	}

	if (!bLettersOnly && g_aKeyNames[0].nID != 0)
	{
		for (pKey = g_aKeyNames; pKey->nID != 0; pKey++)
		{
			if (str.LoadString(pKey->nID))
			{
				DWORD dwData = pKey->dwVK;
				n = (int)::SendMessage(pCombo->m_hWnd, CB_ADDSTRING, 0,
					(LPARAM)(LPCSTR)str);
				::SendMessage(pCombo->m_hWnd, CB_SETITEMDATA, n, dwData);
			}
		}
	}

	::SendMessage(pCombo->m_hWnd, CB_SETCURSEL, 0, 0);
	for (i = (int)::SendMessage(pCombo->m_hWnd, CB_GETCOUNT, 0, 0);
		i >= 0; i--)
	{
		if (::SendMessage(pCombo->m_hWnd, CB_GETITEMDATA, i, 0) == (LRESULT)dwVK)
		{
			::SendMessage(pCombo->m_hWnd, CB_SETCURSEL, i, 0);
			break;
		}
	}
}

// Re-detects the joystick (the "Joystick" checkbox and the "Calibrate"
// button are disabled without one) and fills the three object key combo
// boxes from the current bindings.

// hover.exe:0x42DEF0
// FUNCTION: HOVER 0x42def0
BOOL CSub42D910::OnInitDialog()
{
	CDialog::OnInitDialog();

	if (joyGetDevCapsA(0, &theApp.m_joyCaps, sizeof(theApp.m_joyCaps))
			== JOYERR_NOERROR)
	{
		JOYINFO joyInfo;
		UINT uJoyErr = joyGetPos(0, &joyInfo);
		theApp.m_bJoystick = 1;
		if (uJoyErr != JOYERR_NOERROR)
			theApp.m_bJoystick = 0;
	}
	else
	{
		theApp.m_bJoystick = 0;
	}

	if (theApp.m_bJoystick == 0)
	{
		::EnableWindow(m_btnJoystick.m_hWnd, FALSE);
		theApp.m_avkControls[11] = 0;
	}

	::CheckDlgButton(m_hWnd, IDC_CTRL_KEYBOARD, 1);
	::CheckDlgButton(m_hWnd, IDC_CTRL_JOYSTICK, theApp.m_avkControls[11]);
	::EnableWindow(m_btnCalibrate.m_hWnd, theApp.m_avkControls[11]);

	Sub_42DD70(&m_comboWall, theApp.m_avkControls[1], TRUE);
	Sub_42DD70(&m_comboCloak, theApp.m_avkControls[0], TRUE);
	Sub_42DD70(&m_comboJump, theApp.m_avkControls[2], TRUE);
	return TRUE;
}

// IDC_CTRL_SETKEYS: runs the "Keyboard Settings" dialog on the eight
// movement keys, handing it the three object keys currently selected here
// so that its duplicate check sees all eleven bindings.

// hover.exe:0x42DFF0
// FUNCTION: HOVER 0x42dff0
void CSub42D910::OnSetKeys()
{
	CSub42FE20 dlg(NULL);

	dlg.m_dwWallKey = m_comboWall.GetItemData(m_comboWall.GetCurSel());
	dlg.m_dwCloakKey = m_comboCloak.GetItemData(m_comboCloak.GetCurSel());
	dlg.m_dwJumpKey = m_comboJump.GetItemData(m_comboJump.GetCurSel());
	dlg.DoModal();
}

// Applies the three object keys plus the eight movement keys, rejecting
// the set when two actions share a key, and saves them to the registry.

// hover.exe:0x42E0F0
// FUNCTION: HOVER 0x42e0f0
void CSub42D910::OnOK()
{
	DWORD avk[11];

	avk[0] = m_comboWall.GetItemData(m_comboWall.GetCurSel());
	avk[1] = m_comboCloak.GetItemData(m_comboCloak.GetCurSel());
	avk[2] = m_comboJump.GetItemData(m_comboJump.GetCurSel());
	avk[3] = theApp.m_avkControls[7];
	avk[4] = theApp.m_avkControls[3];
	avk[5] = theApp.m_avkControls[8];
	avk[6] = theApp.m_avkControls[5];
	avk[7] = theApp.m_avkControls[6];
	avk[8] = theApp.m_avkControls[9];
	avk[9] = theApp.m_avkControls[4];
	avk[10] = theApp.m_avkControls[10];

	if (!Sub_42DB60(avk, 11))
	{
		CString str;
		str.LoadString(IDS_DUPLICATE_KEYS);
		AfxMessageBox(str);
		return;
	}

	theApp.m_avkControls[1] = m_comboWall.GetItemData(m_comboWall.GetCurSel());
	theApp.m_avkControls[0] = m_comboCloak.GetItemData(m_comboCloak.GetCurSel());
	theApp.m_avkControls[2] = m_comboJump.GetItemData(m_comboJump.GetCurSel());
	theApp.m_avkControls[11] = ::IsDlgButtonChecked(m_hWnd, IDC_CTRL_JOYSTICK);
	Hover_sub_42E420();
	CDialog::OnOK();
}

// IDC_CTRL_DEFAULT: back to A (jump), S (wall) and D (cloak).

// hover.exe:0x42E2D0
// FUNCTION: HOVER 0x42e2d0
void CSub42D910::OnDefault()
{
	int i;

	m_comboJump.SetCurSel(0);
	for (i = m_comboJump.GetCount(); i >= 0; i--)
	{
		if (m_comboJump.GetItemData(i) == 'A')
		{
			m_comboJump.SetCurSel(i);
			break;
		}
	}

	m_comboWall.SetCurSel(0);
	for (i = m_comboWall.GetCount(); i >= 0; i--)
	{
		if (m_comboWall.GetItemData(i) == 'S')
		{
			m_comboWall.SetCurSel(i);
			break;
		}
	}

	m_comboCloak.SetCurSel(0);
	for (i = m_comboCloak.GetCount(); i >= 0; i--)
	{
		if (m_comboCloak.GetItemData(i) == 'D')
		{
			m_comboCloak.SetCurSel(i);
			break;
		}
	}
}

// IDC_CTRL_CALIBRATE: hands the joystick over to the control panel applet.
// The library is deliberately never freed.

// hover.exe:0x42E3F0
// FUNCTION: HOVER 0x42e3f0
void CSub42D910::OnCalibrate()
{
	HINSTANCE hLib;
	LPSHOWJOYCPL pfnShowJoyCPL;

	hLib = ::LoadLibraryA("joy.cpl");
	if (hLib != NULL)
	{
		pfnShowJoyCPL = (LPSHOWJOYCPL)::GetProcAddress(hLib, "ShowJoyCPL");
		if (pfnShowJoyCPL != NULL)
			pfnShowJoyCPL(NULL);
	}
}

// hover.exe:0x42E4F0
// FUNCTION: HOVER 0x42e4f0
LRESULT CSub42D910::OnHelpInfo(WPARAM wParam, LPARAM lParam)
{
	return ::WinHelp((HWND)((LPHELPINFO)lParam)->hItemHandle, "hover.hlp",
		HELP_WM_HELP, (DWORD)s_adwControlsHelp);
}

// hover.exe:0x42E510
// FUNCTION: HOVER 0x42e510
LRESULT CSub42D910::OnHelpContextMenu(WPARAM wParam, LPARAM lParam)
{
	::WinHelp((HWND)wParam, "hover.hlp", HELP_CONTEXTMENU,
		(DWORD)s_adwControlsHelp);
	return 1;
}

// IDC_CTRL_JOYSTICK: "Calibrate..." follows the checkbox.

// hover.exe:0x42E530
// FUNCTION: HOVER 0x42e530
void CSub42D910::OnJoystick()
{
	::EnableWindow(m_btnCalibrate.m_hWnd,
		::IsDlgButtonChecked(m_hWnd, IDC_CTRL_JOYSTICK));
}

// Saves the eleven key bindings plus the joystick flag as the 0x30-byte
// "PlayerControls" REG_BINARY value, in the order Hover_sub_4232E0 reads
// them back in.

// hover.exe:0x42E420
// FUNCTION: HOVER 0x42e420
void Hover_sub_42E420()
{
	HKEY hKey;
	DWORD dwDisposition;

	if (RegCreateKeyExA(HKEY_CURRENT_USER, "Software\\Microsoft\\Hover!", 0,
			NULL, 0, KEY_ALL_ACCESS, NULL, &hKey, &dwDisposition)
			== ERROR_SUCCESS)
	{
		DWORD adwControls[12];
		const DWORD* pvk = theApp.m_avkControls;
		adwControls[0] = pvk[1];
		adwControls[1] = pvk[0];
		adwControls[2] = pvk[2];
		adwControls[3] = pvk[7];
		adwControls[4] = pvk[3];
		adwControls[5] = pvk[8];
		adwControls[6] = pvk[5];
		adwControls[7] = pvk[6];
		adwControls[8] = pvk[9];
		adwControls[9] = pvk[4];
		adwControls[10] = pvk[10];
		adwControls[11] = pvk[11];
		RegSetValueExA(hKey, "PlayerControls", 0, REG_BINARY,
			(LPBYTE)adwControls, sizeof(adwControls));
		RegCloseKey(hKey);
	}
}

/////////////////////////////////////////////////////////////////////////////
// CSub42FE20 dialog ("Keyboard Settings")

// hover.exe:0x4BF1E0. The first entry is the dialog's own template id and
// IDC_KEY_FORWARD appears twice; both are as in hover.exe.
static DWORD s_adwKeyboardHelp[] =
{
	IDD_KEYBOARD, 2500,
	IDC_KEY_FWDLEFT, 2500,
	IDC_KEY_LEFT, 2500,
	IDC_KEY_REVLEFT, 2500,
	IDC_KEY_REVERSE, 2500,
	IDC_KEY_FORWARD, 2500,
	IDC_KEY_REVRIGHT, 2500,
	IDC_KEY_RIGHT, 2500,
	IDC_KEY_FWDRIGHT, 2500,
	IDC_KEY_FORWARD, 2500,
	IDC_KEY_DEFAULT, 2510,
	0, 0
};

BEGIN_MESSAGE_MAP(CSub42FE20, CDialog)
	//{{AFX_MSG_MAP(CSub42FE20)
	ON_COMMAND(IDC_KEY_DEFAULT, OnDefault)
	ON_MESSAGE(WM_HELP, OnHelpInfo)
	ON_MESSAGE(WM_CONTEXTMENU, OnHelpContextMenu)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// hover.exe:0x42FE20
// FUNCTION: HOVER 0x42fe20
CSub42FE20::CSub42FE20(CWnd* pParent) : CDialog(IDD_KEYBOARD, pParent)
{
}

// hover.exe:0x4300E0
// FUNCTION: HOVER 0x4300e0
void CSub42FE20::DoDataExchange(CDataExchange* pDX)
{
	//{{AFX_DATA_MAP(CSub42FE20)
	DDX_Control(pDX, IDC_KEY_REVRIGHT, m_comboRevRight);
	DDX_Control(pDX, IDC_KEY_REVLEFT, m_comboRevLeft);
	DDX_Control(pDX, IDC_KEY_LEFT, m_comboLeft);
	DDX_Control(pDX, IDC_KEY_RIGHT, m_comboRight);
	DDX_Control(pDX, IDC_KEY_REVERSE, m_comboReverse);
	DDX_Control(pDX, IDC_KEY_FWDRIGHT, m_comboFwdRight);
	DDX_Control(pDX, IDC_KEY_FWDLEFT, m_comboFwdLeft);
	DDX_Control(pDX, IDC_KEY_FORWARD, m_comboForward);
	//}}AFX_DATA_MAP
}

// hover.exe:0x4304B0
// FUNCTION: HOVER 0x4304b0
BOOL CSub42FE20::OnInitDialog()
{
	CDialog::OnInitDialog();
	m_pParentDlg->Sub_42DD70(&m_comboFwdLeft, theApp.m_avkControls[7], FALSE);
	m_pParentDlg->Sub_42DD70(&m_comboForward, theApp.m_avkControls[3], FALSE);
	m_pParentDlg->Sub_42DD70(&m_comboFwdRight, theApp.m_avkControls[8], FALSE);
	m_pParentDlg->Sub_42DD70(&m_comboLeft, theApp.m_avkControls[5], FALSE);
	m_pParentDlg->Sub_42DD70(&m_comboRight, theApp.m_avkControls[6], FALSE);
	m_pParentDlg->Sub_42DD70(&m_comboRevLeft, theApp.m_avkControls[9], FALSE);
	m_pParentDlg->Sub_42DD70(&m_comboReverse, theApp.m_avkControls[4], FALSE);
	m_pParentDlg->Sub_42DD70(&m_comboRevRight, theApp.m_avkControls[10], FALSE);
	return TRUE;
}

// hover.exe:0x430190
// FUNCTION: HOVER 0x430190
void CSub42FE20::OnOK()
{
	DWORD avk[11];

	avk[0] = m_dwWallKey;
	avk[1] = m_dwCloakKey;
	avk[2] = m_dwJumpKey;
	avk[3] = m_comboFwdLeft.GetItemData(m_comboFwdLeft.GetCurSel());
	avk[4] = m_comboForward.GetItemData(m_comboForward.GetCurSel());
	avk[5] = m_comboFwdRight.GetItemData(m_comboFwdRight.GetCurSel());
	avk[6] = m_comboLeft.GetItemData(m_comboLeft.GetCurSel());
	avk[7] = m_comboRight.GetItemData(m_comboRight.GetCurSel());
	avk[8] = m_comboRevLeft.GetItemData(m_comboRevLeft.GetCurSel());
	avk[9] = m_comboReverse.GetItemData(m_comboReverse.GetCurSel());
	avk[10] = m_comboRevRight.GetItemData(m_comboRevRight.GetCurSel());

	if (!m_pParentDlg->Sub_42DB60(avk, 11))
	{
		CString str;
		str.LoadString(IDS_DUPLICATE_KEYS);
		AfxMessageBox(str);
		return;
	}

	theApp.m_avkControls[7] = m_comboFwdLeft.GetItemData(m_comboFwdLeft.GetCurSel());
	theApp.m_avkControls[3] = m_comboForward.GetItemData(m_comboForward.GetCurSel());
	theApp.m_avkControls[8] = m_comboFwdRight.GetItemData(m_comboFwdRight.GetCurSel());
	theApp.m_avkControls[5] = m_comboLeft.GetItemData(m_comboLeft.GetCurSel());
	theApp.m_avkControls[6] = m_comboRight.GetItemData(m_comboRight.GetCurSel());
	theApp.m_avkControls[9] = m_comboRevLeft.GetItemData(m_comboRevLeft.GetCurSel());
	theApp.m_avkControls[4] = m_comboReverse.GetItemData(m_comboReverse.GetCurSel());
	theApp.m_avkControls[10] = m_comboRevRight.GetItemData(m_comboRevRight.GetCurSel());
	CDialog::OnOK();
}

// IDC_KEY_DEFAULT: the arrow key cluster (Home/Up/PgUp, Left/Right,
// End/Down/PgDn).

// hover.exe:0x430580
// FUNCTION: HOVER 0x430580
void CSub42FE20::OnDefault()
{
	m_pParentDlg->Sub_42DBE0(&m_comboFwdLeft, VK_HOME);
	m_pParentDlg->Sub_42DBE0(&m_comboForward, VK_UP);
	m_pParentDlg->Sub_42DBE0(&m_comboFwdRight, VK_PRIOR);
	m_pParentDlg->Sub_42DBE0(&m_comboLeft, VK_LEFT);
	m_pParentDlg->Sub_42DBE0(&m_comboRight, VK_RIGHT);
	m_pParentDlg->Sub_42DBE0(&m_comboRevLeft, VK_END);
	m_pParentDlg->Sub_42DBE0(&m_comboReverse, VK_DOWN);
	m_pParentDlg->Sub_42DBE0(&m_comboRevRight, VK_NEXT);
}

// hover.exe:0x430610
// FUNCTION: HOVER 0x430610
LRESULT CSub42FE20::OnHelpInfo(WPARAM wParam, LPARAM lParam)
{
	return ::WinHelp((HWND)((LPHELPINFO)lParam)->hItemHandle, "hover.hlp",
		HELP_WM_HELP, (DWORD)s_adwKeyboardHelp);
}

// hover.exe:0x430630
// FUNCTION: HOVER 0x430630
LRESULT CSub42FE20::OnHelpContextMenu(WPARAM wParam, LPARAM lParam)
{
	::WinHelp((HWND)wParam, "hover.hlp", HELP_CONTEXTMENU,
		(DWORD)s_adwKeyboardHelp);
	return 1;
}

/////////////////////////////////////////////////////////////////////////////
// CSub42E790 dialog ("Sound Options")

// hover.exe:0x4BDFE0
static DWORD s_adwSoundHelp[] =
{
	IDC_SOUND_EFFECTSBOX, 2300,
	IDC_SOUND_MUSIC, 2300,
	IDC_SOUND_GAMESOUNDS, 2300,
	IDC_SOUND_ENGINE, 2300,
	IDC_SOUND_MIXING, 2310,
	0, 0
};

BEGIN_MESSAGE_MAP(CSub42E790, CDialog)
	//{{AFX_MSG_MAP(CSub42E790)
	ON_MESSAGE(WM_HELP, OnHelpInfo)
	ON_MESSAGE(WM_CONTEXTMENU, OnHelpContextMenu)
	ON_COMMAND(IDC_SOUND_MIXING, OnMixing)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// hover.exe:0x42E790
// FUNCTION: HOVER 0x42e790
CSub42E790::CSub42E790(CWnd* pParent) : CDialog(IDD_SOUNDOPTIONS, pParent)
{
	m_nEngine = 0;
	m_nMusic = 0;
	m_nMixing = 0;
	m_nGameSounds = 0;
}

// hover.exe:0x42E920
// FUNCTION: HOVER 0x42e920
void CSub42E790::DoDataExchange(CDataExchange* pDX)
{
	if (!pDX->m_bSaveAndValidate)
	{
		Hover_sub_4234F0();
		m_nEngine = theApp.m_dwUnk0404;
		m_nGameSounds = theApp.m_dwUnk0408;
		m_nMusic = theApp.m_dwUnk040C;
		m_nMixing = g_dwUnk_4C56D8;
	}

	//{{AFX_DATA_MAP(CSub42E790)
	DDX_Control(pDX, IDC_SOUND_ENGINE, m_btnEngine);
	DDX_Control(pDX, IDC_SOUND_GAMESOUNDS, m_btnGameSounds);
	DDX_Control(pDX, IDC_SOUND_MIXING, m_btnMixing);
	DDX_Control(pDX, IDC_SOUND_MUSIC, m_btnMusic);
	DDX_Check(pDX, IDC_SOUND_ENGINE, m_nEngine);
	DDX_Check(pDX, IDC_SOUND_MUSIC, m_nMusic);
	DDX_Check(pDX, IDC_SOUND_MIXING, m_nMixing);
	DDX_Check(pDX, IDC_SOUND_GAMESOUNDS, m_nGameSounds);
	//}}AFX_DATA_MAP
}

// Without a usable wave device the three effect checkboxes are disabled
// and cleared, and without a MIDI device the music checkbox goes too.

// hover.exe:0x42EA00
// FUNCTION: HOVER 0x42ea00
BOOL CSub42E790::OnInitDialog()
{
	CDialog::OnInitDialog();
	Hover_sub_4234F0();

	if (g_dwUnk_4C56D4 == 0 && g_dwUnk_4C56D0 == 0)
	{
		::EnableWindow(m_btnEngine.m_hWnd, FALSE);
		::EnableWindow(m_btnGameSounds.m_hWnd, FALSE);
		::EnableWindow(m_btnMixing.m_hWnd, FALSE);
		m_nMixing = 0;
		m_nGameSounds = 0;
		m_nEngine = 0;
	}

	if (theApp.m_dwUnk0410 == 0)
	{
		::EnableWindow(m_btnMusic.m_hWnd, FALSE);
		m_nMusic = 0;
	}

	UpdateData(FALSE);
	return TRUE;
}

// hover.exe:0x42EB40
// FUNCTION: HOVER 0x42eb40
void CSub42E790::OnOK()
{
	HKEY hKey;
	DWORD dwDisposition;
	BYTE buf[8];		// the original's frame holds eight bytes here,
				// only the first four of which it ever writes

	UpdateData(TRUE);
	theApp.m_dwUnk0404 = m_nEngine;
	theApp.m_dwUnk0408 = m_nGameSounds;
	theApp.m_dwUnk040C = m_nMusic;
	g_dwUnk_4C56D8 = m_nMixing;

	if (RegCreateKeyExA(HKEY_CURRENT_USER, "Software\\Microsoft\\Hover!", 0,
			NULL, 0, KEY_ALL_ACCESS, NULL, &hKey, &dwDisposition)
			== ERROR_SUCCESS)
	{
		buf[0] = (BYTE)theApp.m_dwUnk040C;
		buf[1] = (BYTE)theApp.m_dwUnk0404;
		buf[2] = (BYTE)theApp.m_dwUnk0408;
		buf[3] = (BYTE)g_dwUnk_4C56D8;
		RegSetValueExA(hKey, "SoundSettings", 0, REG_BINARY, buf, 4);
		RegCloseKey(hKey);
	}

	CDialog::OnOK();
}

// IDC_SOUND_MIXING: switching the mixer on or off changes the format the
// waves have to be loaded in, so the whole sound engine is torn down,
// restarted and every wave slot of the document reloaded.

// hover.exe:0x42EAE0
// FUNCTION: HOVER 0x42eae0
void CSub42E790::OnMixing()
{
	UpdateData(TRUE);
	::SetCursor(::LoadCursor(NULL, IDC_WAIT));
	theApp.m_pBumperDoc->Sub_426EE0();
	Hover_sub_422E30();
	g_dwUnk_4C56D8 = m_nMixing;
	Hover_sub_423140();
	theApp.m_pBumperDoc->Sub_426E90();
	::SetCursor(::LoadCursor(NULL, IDC_ARROW));
}

// hover.exe:0x42EAA0
// FUNCTION: HOVER 0x42eaa0
LRESULT CSub42E790::OnHelpInfo(WPARAM wParam, LPARAM lParam)
{
	return ::WinHelp((HWND)((LPHELPINFO)lParam)->hItemHandle, "hover.hlp",
		HELP_WM_HELP, (DWORD)s_adwSoundHelp);
}

// hover.exe:0x42EAC0
// FUNCTION: HOVER 0x42eac0
LRESULT CSub42E790::OnHelpContextMenu(WPARAM wParam, LPARAM lParam)
{
	::WinHelp((HWND)wParam, "hover.hlp", HELP_CONTEXTMENU,
		(DWORD)s_adwSoundHelp);
	return 1;
}

/////////////////////////////////////////////////////////////////////////////
// CSub42F640 dialog ("Start At")

// hover.exe:0x4BED50 / 0x4BED60: the settings summary strings.
static UINT s_anSkillNames[] =
	{ IDS_SKILL_EASY, IDS_SKILL_INTERMEDIATE, IDS_SKILL_DIFFICULT, 0 };
static UINT s_anMazeNames[] = { IDS_MAZE_SEQUENTIAL, IDS_MAZE_RANDOM };

// hover.exe:0x4BECD0
static DWORD s_adwStartAtHelp[] =
{
	IDC_STARTAT_LEVEL, 2000,
	IDC_STARTAT_SPIN, 2000,
	IDC_STARTAT_CUSTOMIZE, 2020,
	IDC_STARTAT_CONTROLS, 2010,
	IDC_STARTAT_MAZETYPE, 2010,
	IDC_STARTAT_LBL_CONTROLS, 2010,
	IDC_STARTAT_LBL_MAZETYPE, 2010,
	IDC_STARTAT_SETTINGSBOX, 2010,
	IDC_STARTAT_SCOREBOX, (DWORD)-1,
	IDC_STARTAT_LBL_LEVEL, (DWORD)-1,
	IDC_STARTAT_LBL_CAPTURED, (DWORD)-1,
	IDC_STARTAT_LBL_REMAINING, (DWORD)-1,
	IDC_STARTAT_LEVELSCORE, (DWORD)-1,
	IDC_STARTAT_CAPTURED, (DWORD)-1,
	IDC_STARTAT_REMAINING, (DWORD)-1,
	0, 0
};

BEGIN_MESSAGE_MAP(CSub42F640, CDialog)
	//{{AFX_MSG_MAP(CSub42F640)
	ON_MESSAGE(WM_HELP, OnHelpInfo)
	ON_MESSAGE(WM_CONTEXTMENU, OnHelpContextMenu)
	ON_COMMAND(IDC_STARTAT_CUSTOMIZE, OnCustomize)
	ON_EN_CHANGE(IDC_STARTAT_LEVEL, OnChangeLevel)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// hover.exe:0x42F640
// FUNCTION: HOVER 0x42f640
CSub42F640::CSub42F640(CWnd* pParent) : CDialog(IDD_STARTAT, pParent)
{
	m_nLevel = g_dwUnk_46048C;
}

// hover.exe:0x42F890
// FUNCTION: HOVER 0x42f890
BOOL CSub42F640::OnInitDialog()
{
	CString str;

	CDialog::OnInitDialog();
	m_spinLevel.SetRange(1, 16);
	m_spinLevel.SetBuddy(&m_editLevel);
	OnChangeLevel();

	str.LoadString(s_anMazeNames[theApp.m_dwUnk01CC]);
	::SetDlgItemText(m_hWnd, IDC_STARTAT_MAZETYPE, str);
	str.LoadString(s_anSkillNames[theApp.m_dwUnk01C8]);
	::SetDlgItemText(m_hWnd, IDC_STARTAT_CONTROLS, str);
	return TRUE;
}

// hover.exe:0x42FCC0
// FUNCTION: HOVER 0x42fcc0
void CSub42F640::OnOK()
{
	if (UpdateData(TRUE))
	{
		g_dwUnk_46048C = m_nLevel;
		CDialog::OnOK();
	}
}

// Formats dwScore with thousands separators into the static control nID
// (the same code as CSub424BD0::Sub_433860).

// hover.exe:0x42FA80
// FUNCTION: HOVER 0x42fa80
void CSub42F640::Sub_42FA80(UINT nID, DWORD dwScore)
{
	CString strDigits;
	CString strOut;
	int nLast;
	int i;

	strDigits.Format("%ld", dwScore);
	nLast = strDigits.GetLength() - 1;
	for (i = 0; i <= nLast; i++)
	{
		strOut += strDigits[i];
		if (i < nLast && (nLast - i) % 3 == 0)
			strOut += ',';
	}
	::SetDlgItemText(m_hWnd, nID, strOut);
}

// IDC_STARTAT_CUSTOMIZE: runs the Customize Game dialog and refreshes the
// settings summary and the scoring table afterwards.

// hover.exe:0x42F990
// FUNCTION: HOVER 0x42f990
void CSub42F640::OnCustomize()
{
	CSub413F10 dlg(NULL);
	CString str;

	dlg.DoModal();
	str.LoadString(s_anMazeNames[theApp.m_dwUnk01CC]);
	::SetDlgItemText(m_hWnd, IDC_STARTAT_MAZETYPE, str);
	str.LoadString(s_anSkillNames[theApp.m_dwUnk01C8]);
	::SetDlgItemText(m_hWnd, IDC_STARTAT_CONTROLS, str);
	OnChangeLevel();
}

// EN_CHANGE on the level edit box: shows what the three score values are
// worth on the level now entered. The level is clamped to the spin
// button's range and mapped through the LVLPARAM cycle.

// hover.exe:0x42FB60
// FUNCTION: HOVER 0x42fb60
void CSub42F640::OnChangeLevel()
{
	CString str1;
	CString str2;
	int nLower, nUpper;
	int nLevel;

	m_spinLevel.GetRange(nLower, nUpper);
	nLevel = LOWORD(::SendMessage(m_spinLevel.m_hWnd, UDM_GETPOS, 0, 0));
	if (nLevel < nLower)
		nLevel = nLower;
	if (nLevel > nUpper)
		nLevel = nUpper;
	nLevel--;

	Sub_42FA80(IDC_STARTAT_CAPTURED,
		(&LVLPARAM(nLevel).m_dwUnk40)[theApp.m_dwUnk01C8]);
	Sub_42FA80(IDC_STARTAT_REMAINING,
		(&LVLPARAM(nLevel).m_dwUnk4C)[theApp.m_dwUnk01C8]);
	Sub_42FA80(IDC_STARTAT_LEVELSCORE,
		(&LVLPARAM(nLevel).m_dwUnk34)[theApp.m_dwUnk01C8]);
}

// hover.exe:0x42F7D0
// FUNCTION: HOVER 0x42f7d0
void CSub42F640::DoDataExchange(CDataExchange* pDX)
{
	//{{AFX_DATA_MAP(CSub42F640)
	DDX_Control(pDX, IDC_STARTAT_CUSTOMIZE, m_btnCustomize);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDC_STARTAT_LEVEL, m_editLevel);
	DDX_Control(pDX, IDC_STARTAT_SPIN, m_spinLevel);
	DDX_Text(pDX, IDC_STARTAT_LEVEL, m_nLevel);
	DDV_MinMaxUInt(pDX, m_nLevel, 1, 16);
	//}}AFX_DATA_MAP
}

// hover.exe:0x42F850
// FUNCTION: HOVER 0x42f850
LRESULT CSub42F640::OnHelpInfo(WPARAM wParam, LPARAM lParam)
{
	return ::WinHelp((HWND)((LPHELPINFO)lParam)->hItemHandle, "hover.hlp",
		HELP_WM_HELP, (DWORD)s_adwStartAtHelp);
}

// hover.exe:0x42F870
// FUNCTION: HOVER 0x42f870
LRESULT CSub42F640::OnHelpContextMenu(WPARAM wParam, LPARAM lParam)
{
	::WinHelp((HWND)wParam, "hover.hlp", HELP_CONTEXTMENU,
		(DWORD)s_adwStartAtHelp);
	return 1;
}

/////////////////////////////////////////////////////////////////////////////
// CSub430780 dialog (debug template 148: human player parameters)

BEGIN_MESSAGE_MAP(CSub430780, CDialog)
	//{{AFX_MSG_MAP(CSub430780)
	ON_COMMAND(IDRETRY, OnDeleteSettings)
	ON_COMMAND(IDABORT, OnSaveSettings)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// hover.exe:0x430780
// FUNCTION: HOVER 0x430780
CSub430780::CSub430780(CWnd* pParent) : CDialog(IDD_DBG_148, pParent)
{
	m_adblParams[0] = 0.0;
	m_adblParams[1] = 0.0;
	m_adblParams[2] = 0.0;
	m_adblParams[3] = 0.0;
	m_adblParams[4] = 0.0;
	m_adblParams[5] = 0.0;
	m_adblParams[6] = 0.0;
	m_adblParams[7] = 0.0;
	m_adblParams[8] = 0.0;
	m_adblParams[9] = 0.0;
	m_adblParams[10] = 0.0;
	m_adblParams[11] = 0.0;
	m_adblParams[12] = 0.0;
	m_adblParams[13] = 0.0;
	m_adblParams[14] = 0.0;
	m_adblParams[15] = 0.0;
	m_adblParams[16] = 0.0;
	m_adblParams[17] = 0.0;
	m_adblParams[18] = 0.0;
	m_adblParams[19] = 0.0;
	m_adblParams[20] = 0.0;
	m_adblParams[21] = 0.0;
	m_adblParams[22] = 0.0;
	m_adblParams[23] = 0.0;
	m_adblParams[24] = 0.0;
	m_adblParams[25] = 0.0;
	m_adblParams[26] = 0.0;
	m_adblParams[27] = 0.0;
	m_adblParams[28] = 0.0;
	m_adblParams[29] = 0.0;
}

// hover.exe:0x430930
// FUNCTION: HOVER 0x430930
void CSub430780::DoDataExchange(CDataExchange* pDX)
{
	//{{AFX_DATA_MAP(CSub430780)
	DDX_Text(pDX, 1046, m_adblParams[0]);
	DDX_Text(pDX, 1055, m_adblParams[1]);
	DDV_MinMaxDouble(pDX, m_adblParams[1], -1.0, 0.0);
	DDX_Text(pDX, 1043, m_adblParams[2]);
	DDX_Text(pDX, 1067, m_adblParams[3]);
	DDX_Text(pDX, 1064, m_adblParams[4]);
	DDX_Text(pDX, 1061, m_adblParams[5]);
	DDX_Text(pDX, 1019, m_adblParams[6]);
	DDX_Text(pDX, 1018, m_adblParams[7]);
	DDX_Text(pDX, 1058, m_adblParams[8]);
	DDV_MinMaxDouble(pDX, m_adblParams[8], 0.0, 1.0);
	DDX_Text(pDX, 1052, m_adblParams[9]);
	DDX_Text(pDX, 1048, m_adblParams[10]);
	DDX_Text(pDX, 1057, m_adblParams[11]);
	DDX_Text(pDX, 1045, m_adblParams[12]);
	DDX_Text(pDX, 1069, m_adblParams[13]);
	DDX_Text(pDX, 1066, m_adblParams[14]);
	DDX_Text(pDX, 1063, m_adblParams[15]);
	DDX_Text(pDX, 1032, m_adblParams[16]);
	DDX_Text(pDX, 1031, m_adblParams[17]);
	DDX_Text(pDX, 1060, m_adblParams[18]);
	DDX_Text(pDX, 1054, m_adblParams[19]);
	DDX_Text(pDX, 1047, m_adblParams[20]);
	DDX_Text(pDX, 1056, m_adblParams[21]);
	DDX_Text(pDX, 1044, m_adblParams[22]);
	DDX_Text(pDX, 1068, m_adblParams[23]);
	DDX_Text(pDX, 1065, m_adblParams[24]);
	DDX_Text(pDX, 1062, m_adblParams[25]);
	DDX_Text(pDX, 1030, m_adblParams[26]);
	DDX_Text(pDX, 1029, m_adblParams[27]);
	DDX_Text(pDX, 1059, m_adblParams[28]);
	DDX_Text(pDX, 1053, m_adblParams[29]);
	//}}AFX_DATA_MAP
}

// hover.exe:0x430B80
// FUNCTION: HOVER 0x430b80
BOOL CSub430780::OnInitDialog()
{
	FIXED1616 fx;

	fx.dw = g_HBegParams.m_adw[0];
	m_adblParams[0] = FIX_TO_DBL(fx);
	fx.dw = g_HBegParams.m_adw[1];
	m_adblParams[1] = FIX_TO_DBL(fx);
	fx.dw = g_HBegParams.m_adw[2];
	m_adblParams[2] = FIX_TO_DBL(fx);
	fx.dw = g_HBegParams.m_adw[4];
	m_adblParams[3] = FIX_TO_DBL(fx);
	fx.dw = g_HBegParams.m_adw[5];
	m_adblParams[4] = FIX_TO_DBL(fx);
	fx.dw = g_HBegParams.m_adw[6];
	m_adblParams[5] = FIX_TO_DBL(fx);
	fx.dw = g_HBegParams.m_adw[7];
	m_adblParams[6] = FIX_TO_DBL(fx);
	fx.dw = g_HBegParams.m_adw[8];
	m_adblParams[7] = FIX_TO_DBL(fx);
	fx.dw = g_HBegParams.m_adw[9];
	m_adblParams[8] = FIX_TO_DBL(fx);
	fx.dw = g_HBegParams.m_adw[10];
	m_adblParams[9] = FIX_TO_DBL(fx);
	fx.dw = g_HExpParams.m_adw[0];
	m_adblParams[10] = FIX_TO_DBL(fx);
	fx.dw = g_HExpParams.m_adw[1];
	m_adblParams[11] = FIX_TO_DBL(fx);
	fx.dw = g_HExpParams.m_adw[2];
	m_adblParams[12] = FIX_TO_DBL(fx);
	fx.dw = g_HExpParams.m_adw[4];
	m_adblParams[13] = FIX_TO_DBL(fx);
	fx.dw = g_HExpParams.m_adw[5];
	m_adblParams[14] = FIX_TO_DBL(fx);
	fx.dw = g_HExpParams.m_adw[6];
	m_adblParams[15] = FIX_TO_DBL(fx);
	fx.dw = g_HExpParams.m_adw[7];
	m_adblParams[16] = FIX_TO_DBL(fx);
	fx.dw = g_HExpParams.m_adw[8];
	m_adblParams[17] = FIX_TO_DBL(fx);
	fx.dw = g_HExpParams.m_adw[9];
	m_adblParams[18] = FIX_TO_DBL(fx);
	fx.dw = g_HExpParams.m_adw[10];
	m_adblParams[19] = FIX_TO_DBL(fx);
	fx.dw = g_HIntParams.m_adw[0];
	m_adblParams[20] = FIX_TO_DBL(fx);
	fx.dw = g_HIntParams.m_adw[1];
	m_adblParams[21] = FIX_TO_DBL(fx);
	fx.dw = g_HIntParams.m_adw[2];
	m_adblParams[22] = FIX_TO_DBL(fx);
	fx.dw = g_HIntParams.m_adw[4];
	m_adblParams[23] = FIX_TO_DBL(fx);
	fx.dw = g_HIntParams.m_adw[5];
	m_adblParams[24] = FIX_TO_DBL(fx);
	fx.dw = g_HIntParams.m_adw[6];
	m_adblParams[25] = FIX_TO_DBL(fx);
	fx.dw = g_HIntParams.m_adw[7];
	m_adblParams[26] = FIX_TO_DBL(fx);
	fx.dw = g_HIntParams.m_adw[8];
	m_adblParams[27] = FIX_TO_DBL(fx);
	fx.dw = g_HIntParams.m_adw[9];
	m_adblParams[28] = FIX_TO_DBL(fx);
	fx.dw = g_HIntParams.m_adw[10];
	m_adblParams[29] = FIX_TO_DBL(fx);
	CDialog::OnInitDialog();
	return TRUE;
}

// hover.exe:0x4315E0
// FUNCTION: HOVER 0x4315e0
void CSub430780::Sub_4315E0()
{
	g_HBegParams.m_adw[0] = DBL_TO_FIX(m_adblParams[0]);
	g_HBegParams.m_adw[1] = DBL_TO_FIX(m_adblParams[1]);
	g_HBegParams.m_adw[2] = DBL_TO_FIX(m_adblParams[2]);
	g_HBegParams.m_adw[4] = DBL_TO_FIX(m_adblParams[3]);
	g_HBegParams.m_adw[5] = DBL_TO_FIX(m_adblParams[4]);
	g_HBegParams.m_adw[6] = DBL_TO_FIX(m_adblParams[5]);
	g_HBegParams.m_adw[7] = DBL_TO_FIX(m_adblParams[6]);
	g_HBegParams.m_adw[8] = DBL_TO_FIX(m_adblParams[7]);
	g_HBegParams.m_adw[9] = DBL_TO_FIX(m_adblParams[8]);
	g_HBegParams.m_adw[10] = DBL_TO_FIX(m_adblParams[9]);
	g_HExpParams.m_adw[0] = DBL_TO_FIX(m_adblParams[10]);
	g_HExpParams.m_adw[1] = DBL_TO_FIX(m_adblParams[11]);
	g_HExpParams.m_adw[2] = DBL_TO_FIX(m_adblParams[12]);
	g_HExpParams.m_adw[4] = DBL_TO_FIX(m_adblParams[13]);
	g_HExpParams.m_adw[5] = DBL_TO_FIX(m_adblParams[14]);
	g_HExpParams.m_adw[6] = DBL_TO_FIX(m_adblParams[15]);
	g_HExpParams.m_adw[7] = DBL_TO_FIX(m_adblParams[16]);
	g_HExpParams.m_adw[8] = DBL_TO_FIX(m_adblParams[17]);
	g_HExpParams.m_adw[9] = DBL_TO_FIX(m_adblParams[18]);
	g_HExpParams.m_adw[10] = DBL_TO_FIX(m_adblParams[19]);
	g_HIntParams.m_adw[0] = DBL_TO_FIX(m_adblParams[20]);
	g_HIntParams.m_adw[1] = DBL_TO_FIX(m_adblParams[21]);
	g_HIntParams.m_adw[2] = DBL_TO_FIX(m_adblParams[22]);
	g_HIntParams.m_adw[4] = DBL_TO_FIX(m_adblParams[23]);
	g_HIntParams.m_adw[5] = DBL_TO_FIX(m_adblParams[24]);
	g_HIntParams.m_adw[6] = DBL_TO_FIX(m_adblParams[25]);
	g_HIntParams.m_adw[7] = DBL_TO_FIX(m_adblParams[26]);
	g_HIntParams.m_adw[8] = DBL_TO_FIX(m_adblParams[27]);
	g_HIntParams.m_adw[9] = DBL_TO_FIX(m_adblParams[28]);
	g_HIntParams.m_adw[10] = DBL_TO_FIX(m_adblParams[29]);
	Hover_sub_401140()->m_obj00E8.Sub_4140E0();
}

// hover.exe:0x431880
// FUNCTION: HOVER 0x431880
void CSub430780::OnOK()
{
	CDialog::OnOK();
	Sub_4315E0();
}

// Button 4: drops the three saved parameter blocks from the registry.

// hover.exe:0x4318A0
// FUNCTION: HOVER 0x4318a0
void CSub430780::OnDeleteSettings()
{
	HKEY hKey;

	if (RegOpenKeyExA(HKEY_CURRENT_USER, "Software\\Microsoft\\Hover!", 0,
			KEY_ALL_ACCESS, &hKey) == ERROR_SUCCESS)
	{
		RegDeleteValueA(hKey, "HBegParams");
		RegDeleteValueA(hKey, "HIntParams");
		RegDeleteValueA(hKey, "HExpParams");
		RegCloseKey(hKey);
	}
}

// Button 3: applies the dialog and writes the blocks to the registry.

// hover.exe:0x431900
// FUNCTION: HOVER 0x431900
void CSub430780::OnSaveSettings()
{
	HKEY hKey;
	DWORD dwDisposition;

	UpdateData(TRUE);
	Sub_4315E0();

	if (RegCreateKeyExA(HKEY_CURRENT_USER, "Software\\Microsoft\\Hover!", 0,
			NULL, 0, KEY_ALL_ACCESS, NULL, &hKey, &dwDisposition)
			== ERROR_SUCCESS)
	{
		RegSetValueExA(hKey, "HBegParams", 0, REG_BINARY,
			(LPBYTE)&g_HBegParams, sizeof(g_HBegParams));
		RegSetValueExA(hKey, "HIntParams", 0, REG_BINARY,
			(LPBYTE)&g_HIntParams, sizeof(g_HIntParams));
		RegSetValueExA(hKey, "HExpParams", 0, REG_BINARY,
			(LPBYTE)&g_HExpParams, sizeof(g_HExpParams));
		RegCloseKey(hKey);
	}
}

/////////////////////////////////////////////////////////////////////////////
// CSub4319A0 dialog (debug template 149: robot player parameters)
//
// Entries 7 and 8 of each block (m_adblParams 7/8, 18/19, 29/30) have no
// control and are never touched.

BEGIN_MESSAGE_MAP(CSub4319A0, CDialog)
	//{{AFX_MSG_MAP(CSub4319A0)
	ON_COMMAND(IDRETRY, OnDeleteSettings)
	ON_COMMAND(IDABORT, OnSaveSettings)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// hover.exe:0x4319A0
// FUNCTION: HOVER 0x4319a0
CSub4319A0::CSub4319A0(CWnd* pParent) : CDialog(IDD_DBG_149, pParent)
{
	m_adblParams[0] = 0.0;
	m_adblParams[1] = 0.0;
	m_adblParams[2] = 0.0;
	m_adblParams[3] = 0.0;
	m_adblParams[4] = 0.0;
	m_adblParams[5] = 0.0;
	m_adblParams[6] = 0.0;
	m_adblParams[9] = 0.0;
	m_adblParams[10] = 0.0;
	m_adblParams[11] = 0.0;
	m_adblParams[12] = 0.0;
	m_adblParams[13] = 0.0;
	m_adblParams[14] = 0.0;
	m_adblParams[15] = 0.0;
	m_adblParams[16] = 0.0;
	m_adblParams[17] = 0.0;
	m_adblParams[20] = 0.0;
	m_adblParams[21] = 0.0;
	m_adblParams[22] = 0.0;
	m_adblParams[23] = 0.0;
	m_adblParams[24] = 0.0;
	m_adblParams[25] = 0.0;
	m_adblParams[26] = 0.0;
	m_adblParams[27] = 0.0;
	m_adblParams[28] = 0.0;
	m_adblParams[31] = 0.0;
	m_adblParams[32] = 0.0;
}

// hover.exe:0x431B20
// FUNCTION: HOVER 0x431b20
void CSub4319A0::DoDataExchange(CDataExchange* pDX)
{
	//{{AFX_DATA_MAP(CSub4319A0)
	DDX_Text(pDX, 1046, m_adblParams[0]);
	DDX_Text(pDX, 1055, m_adblParams[1]);
	DDX_Text(pDX, 1043, m_adblParams[2]);
	DDX_Text(pDX, 1049, m_adblParams[3]);
	DDX_Text(pDX, 1067, m_adblParams[4]);
	DDX_Text(pDX, 1064, m_adblParams[5]);
	DDX_Text(pDX, 1061, m_adblParams[6]);
	DDX_Text(pDX, 1058, m_adblParams[9]);
	DDX_Text(pDX, 1052, m_adblParams[10]);
	DDX_Text(pDX, 1048, m_adblParams[11]);
	DDX_Text(pDX, 1057, m_adblParams[12]);
	DDX_Text(pDX, 1045, m_adblParams[13]);
	DDX_Text(pDX, 1051, m_adblParams[14]);
	DDX_Text(pDX, 1069, m_adblParams[15]);
	DDX_Text(pDX, 1066, m_adblParams[16]);
	DDX_Text(pDX, 1063, m_adblParams[17]);
	DDX_Text(pDX, 1060, m_adblParams[20]);
	DDX_Text(pDX, 1054, m_adblParams[21]);
	DDX_Text(pDX, 1047, m_adblParams[22]);
	DDX_Text(pDX, 1056, m_adblParams[23]);
	DDX_Text(pDX, 1044, m_adblParams[24]);
	DDX_Text(pDX, 1050, m_adblParams[25]);
	DDX_Text(pDX, 1068, m_adblParams[26]);
	DDX_Text(pDX, 1065, m_adblParams[27]);
	DDX_Text(pDX, 1062, m_adblParams[28]);
	DDX_Text(pDX, 1059, m_adblParams[31]);
	DDX_Text(pDX, 1053, m_adblParams[32]);
	//}}AFX_DATA_MAP
}

// Writes all 33 doubles back, including the six (indices 7/8 of each
// block) that have no control and are never initialized - a bug in the
// original: entries 7 and 8 of the three R parameter blocks are
// overwritten with whatever was on the stack.

// hover.exe:0x432660
// FUNCTION: HOVER 0x432660
void CSub4319A0::Sub_432660()
{
	g_RBegParams.m_adw[0] = DBL_TO_FIX(m_adblParams[0]);
	g_RBegParams.m_adw[1] = DBL_TO_FIX(m_adblParams[1]);
	g_RBegParams.m_adw[2] = DBL_TO_FIX(m_adblParams[2]);
	g_RBegParams.m_adw[3] = DBL_TO_FIX(m_adblParams[3]);
	g_RBegParams.m_adw[4] = DBL_TO_FIX(m_adblParams[4]);
	g_RBegParams.m_adw[5] = DBL_TO_FIX(m_adblParams[5]);
	g_RBegParams.m_adw[6] = DBL_TO_FIX(m_adblParams[6]);
	g_RBegParams.m_adw[7] = DBL_TO_FIX(m_adblParams[7]);
	g_RBegParams.m_adw[8] = DBL_TO_FIX(m_adblParams[8]);
	g_RBegParams.m_adw[9] = DBL_TO_FIX(m_adblParams[9]);
	g_RBegParams.m_adw[10] = DBL_TO_FIX(m_adblParams[10]);
	g_RExpParams.m_adw[0] = DBL_TO_FIX(m_adblParams[11]);
	g_RExpParams.m_adw[1] = DBL_TO_FIX(m_adblParams[12]);
	g_RExpParams.m_adw[2] = DBL_TO_FIX(m_adblParams[13]);
	g_RExpParams.m_adw[3] = DBL_TO_FIX(m_adblParams[14]);
	g_RExpParams.m_adw[4] = DBL_TO_FIX(m_adblParams[15]);
	g_RExpParams.m_adw[5] = DBL_TO_FIX(m_adblParams[16]);
	g_RExpParams.m_adw[6] = DBL_TO_FIX(m_adblParams[17]);
	g_RExpParams.m_adw[7] = DBL_TO_FIX(m_adblParams[18]);
	g_RExpParams.m_adw[8] = DBL_TO_FIX(m_adblParams[19]);
	g_RExpParams.m_adw[9] = DBL_TO_FIX(m_adblParams[20]);
	g_RExpParams.m_adw[10] = DBL_TO_FIX(m_adblParams[21]);
	g_RIntParams.m_adw[0] = DBL_TO_FIX(m_adblParams[22]);
	g_RIntParams.m_adw[1] = DBL_TO_FIX(m_adblParams[23]);
	g_RIntParams.m_adw[2] = DBL_TO_FIX(m_adblParams[24]);
	g_RIntParams.m_adw[3] = DBL_TO_FIX(m_adblParams[25]);
	g_RIntParams.m_adw[4] = DBL_TO_FIX(m_adblParams[26]);
	g_RIntParams.m_adw[5] = DBL_TO_FIX(m_adblParams[27]);
	g_RIntParams.m_adw[6] = DBL_TO_FIX(m_adblParams[28]);
	g_RIntParams.m_adw[7] = DBL_TO_FIX(m_adblParams[29]);
	g_RIntParams.m_adw[8] = DBL_TO_FIX(m_adblParams[30]);
	g_RIntParams.m_adw[9] = DBL_TO_FIX(m_adblParams[31]);
	g_RIntParams.m_adw[10] = DBL_TO_FIX(m_adblParams[32]);
}

// hover.exe:0x431D10
// FUNCTION: HOVER 0x431d10
BOOL CSub4319A0::OnInitDialog()
{
	FIXED1616 fx;

	fx.dw = g_RBegParams.m_adw[0];
	m_adblParams[0] = FIX_TO_DBL(fx);
	fx.dw = g_RBegParams.m_adw[1];
	m_adblParams[1] = FIX_TO_DBL(fx);
	fx.dw = g_RBegParams.m_adw[2];
	m_adblParams[2] = FIX_TO_DBL(fx);
	fx.dw = g_RBegParams.m_adw[3];
	m_adblParams[3] = FIX_TO_DBL(fx);
	fx.dw = g_RBegParams.m_adw[4];
	m_adblParams[4] = FIX_TO_DBL(fx);
	fx.dw = g_RBegParams.m_adw[5];
	m_adblParams[5] = FIX_TO_DBL(fx);
	fx.dw = g_RBegParams.m_adw[6];
	m_adblParams[6] = FIX_TO_DBL(fx);
	fx.dw = g_RBegParams.m_adw[9];
	m_adblParams[9] = FIX_TO_DBL(fx);
	fx.dw = g_RBegParams.m_adw[10];
	m_adblParams[10] = FIX_TO_DBL(fx);
	fx.dw = g_RExpParams.m_adw[0];
	m_adblParams[11] = FIX_TO_DBL(fx);
	fx.dw = g_RExpParams.m_adw[1];
	m_adblParams[12] = FIX_TO_DBL(fx);
	fx.dw = g_RExpParams.m_adw[2];
	m_adblParams[13] = FIX_TO_DBL(fx);
	fx.dw = g_RExpParams.m_adw[3];
	m_adblParams[14] = FIX_TO_DBL(fx);
	fx.dw = g_RExpParams.m_adw[4];
	m_adblParams[15] = FIX_TO_DBL(fx);
	fx.dw = g_RExpParams.m_adw[5];
	m_adblParams[16] = FIX_TO_DBL(fx);
	fx.dw = g_RExpParams.m_adw[6];
	m_adblParams[17] = FIX_TO_DBL(fx);
	fx.dw = g_RExpParams.m_adw[9];
	m_adblParams[20] = FIX_TO_DBL(fx);
	fx.dw = g_RExpParams.m_adw[10];
	m_adblParams[21] = FIX_TO_DBL(fx);
	fx.dw = g_RIntParams.m_adw[0];
	m_adblParams[22] = FIX_TO_DBL(fx);
	fx.dw = g_RIntParams.m_adw[1];
	m_adblParams[23] = FIX_TO_DBL(fx);
	fx.dw = g_RIntParams.m_adw[2];
	m_adblParams[24] = FIX_TO_DBL(fx);
	fx.dw = g_RIntParams.m_adw[3];
	m_adblParams[25] = FIX_TO_DBL(fx);
	fx.dw = g_RIntParams.m_adw[4];
	m_adblParams[26] = FIX_TO_DBL(fx);
	fx.dw = g_RIntParams.m_adw[5];
	m_adblParams[27] = FIX_TO_DBL(fx);
	fx.dw = g_RIntParams.m_adw[6];
	m_adblParams[28] = FIX_TO_DBL(fx);
	fx.dw = g_RIntParams.m_adw[9];
	m_adblParams[31] = FIX_TO_DBL(fx);
	fx.dw = g_RIntParams.m_adw[10];
	m_adblParams[32] = FIX_TO_DBL(fx);
	CDialog::OnInitDialog();
	return TRUE;
}

// hover.exe:0x432930
// FUNCTION: HOVER 0x432930
void CSub4319A0::OnOK()
{
	CDialog::OnOK();
	Sub_432660();
}

// hover.exe:0x432950
// FUNCTION: HOVER 0x432950
void CSub4319A0::OnDeleteSettings()
{
	HKEY hKey;

	if (RegOpenKeyExA(HKEY_CURRENT_USER, "Software\\Microsoft\\Hover!", 0,
			KEY_ALL_ACCESS, &hKey) == ERROR_SUCCESS)
	{
		RegDeleteValueA(hKey, "RBegParams");
		RegDeleteValueA(hKey, "RIntParams");
		RegDeleteValueA(hKey, "RExpParams");
		RegCloseKey(hKey);
	}
}

// hover.exe:0x4329B0
// FUNCTION: HOVER 0x4329b0
void CSub4319A0::OnSaveSettings()
{
	HKEY hKey;
	DWORD dwDisposition;

	UpdateData(TRUE);
	Sub_432660();

	if (RegCreateKeyExA(HKEY_CURRENT_USER, "Software\\Microsoft\\Hover!", 0,
			NULL, 0, KEY_ALL_ACCESS, NULL, &hKey, &dwDisposition)
			== ERROR_SUCCESS)
	{
		RegSetValueExA(hKey, "RBegParams", 0, REG_BINARY,
			(LPBYTE)&g_RBegParams, sizeof(g_RBegParams));
		RegSetValueExA(hKey, "RIntParams", 0, REG_BINARY,
			(LPBYTE)&g_RIntParams, sizeof(g_RIntParams));
		RegSetValueExA(hKey, "RExpParams", 0, REG_BINARY,
			(LPBYTE)&g_RExpParams, sizeof(g_RExpParams));
		RegCloseKey(hKey);
	}
}

/////////////////////////////////////////////////////////////////////////////
// CSub432A50 dialog (debug template 152: robot AI parameters)

BEGIN_MESSAGE_MAP(CSub432A50, CDialog)
	//{{AFX_MSG_MAP(CSub432A50)
	ON_COMMAND(IDRETRY, OnDeleteSettings)
	ON_COMMAND(IDABORT, OnSaveSettings)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// hover.exe:0x432A50
// FUNCTION: HOVER 0x432a50
CSub432A50::CSub432A50(CWnd* pParent) : CDialog(IDD_DBG_152, pParent)
{
	m_nBeg1 = 0;
	m_dblBeg2 = 0.0;
	m_dblBeg0 = 0.0;
	m_nBeg9 = 0;
	m_nBeg4 = 0;
	m_nBeg3 = 0;
	m_nBeg7 = 0;
	m_nBeg6 = 0;
	m_nBeg5 = 0;
	m_nBeg8 = 0;
	m_nInt1 = 0;
	m_dblInt2 = 0.0;
	m_dblInt0 = 0.0;
	m_nInt9 = 0;
	m_nInt4 = 0;
	m_nInt3 = 0;
	m_nInt7 = 0;
	m_nInt6 = 0;
	m_nInt5 = 0;
	m_nInt8 = 0;
	m_nExp1 = 0;
	m_dblExp2 = 0.0;
	m_dblExp0 = 0.0;
	m_nExp9 = 0;
	m_nExp4 = 0;
	m_nExp3 = 0;
	m_nExp7 = 0;
	m_nExp6 = 0;
	m_nExp5 = 0;
	m_nExp8 = 0;
}

// hover.exe:0x432B70
// FUNCTION: HOVER 0x432b70
void CSub432A50::DoDataExchange(CDataExchange* pDX)
{
	//{{AFX_DATA_MAP(CSub432A50)
	DDX_Text(pDX, 1107, m_nBeg1);
	DDX_Text(pDX, 1108, m_dblBeg2);
	DDX_Text(pDX, 1106, m_dblBeg0);
	DDX_Text(pDX, 1009, m_nBeg9);
	DDV_MinMaxInt(pDX, m_nBeg9, 0, 4);
	DDX_Text(pDX, 1004, m_nBeg4);
	DDX_Text(pDX, 1003, m_nBeg3);
	DDX_Text(pDX, 1007, m_nBeg7);
	DDX_Text(pDX, 1006, m_nBeg6);
	DDX_Text(pDX, 1005, m_nBeg5);
	DDX_Text(pDX, 1008, m_nBeg8);
	DDX_Text(pDX, 1110, m_nInt1);
	DDX_Text(pDX, 1111, m_dblInt2);
	DDX_Text(pDX, 1109, m_dblInt0);
	DDX_Text(pDX, 1016, m_nInt9);
	DDV_MinMaxInt(pDX, m_nInt9, 0, 4);
	DDX_Text(pDX, 1011, m_nInt4);
	DDX_Text(pDX, 1010, m_nInt3);
	DDX_Text(pDX, 1014, m_nInt7);
	DDX_Text(pDX, 1013, m_nInt6);
	DDX_Text(pDX, 1012, m_nInt5);
	DDX_Text(pDX, 1015, m_nInt8);
	DDX_Text(pDX, 1113, m_nExp1);
	DDX_Text(pDX, 1114, m_dblExp2);
	DDX_Text(pDX, 1112, m_dblExp0);
	DDX_Text(pDX, 1023, m_nExp9);
	DDV_MinMaxInt(pDX, m_nExp9, 0, 4);
	DDX_Text(pDX, 1018, m_nExp4);
	DDX_Text(pDX, 1017, m_nExp3);
	DDX_Text(pDX, 1021, m_nExp7);
	DDX_Text(pDX, 1020, m_nExp6);
	DDX_Text(pDX, 1019, m_nExp5);
	DDX_Text(pDX, 1022, m_nExp8);
	//}}AFX_DATA_MAP
}

// hover.exe:0x432DB0
// FUNCTION: HOVER 0x432db0
void CSub432A50::Sub_432DB0()
{
	g_RobotAIBegParams.m_adw[1] = m_nBeg1;
	g_RobotAIBegParams.m_adw[2] = DBL_TO_FIX(m_dblBeg2);
	g_RobotAIBegParams.m_adw[0] = DBL_TO_FIX(m_dblBeg0);
	g_RobotAIBegParams.m_adw[9] = m_nBeg9;
	g_RobotAIBegParams.m_adw[4] = m_nBeg4;
	g_RobotAIBegParams.m_adw[3] = m_nBeg3;
	g_RobotAIBegParams.m_adw[7] = m_nBeg7;
	g_RobotAIBegParams.m_adw[6] = m_nBeg6;
	g_RobotAIBegParams.m_adw[5] = m_nBeg5;
	g_RobotAIBegParams.m_adw[8] = m_nBeg8;
	g_RobotAIIntParams.m_adw[1] = m_nInt1;
	g_RobotAIIntParams.m_adw[2] = DBL_TO_FIX(m_dblInt2);
	g_RobotAIIntParams.m_adw[0] = DBL_TO_FIX(m_dblInt0);
	g_RobotAIIntParams.m_adw[9] = m_nInt9;
	g_RobotAIIntParams.m_adw[4] = m_nInt4;
	g_RobotAIIntParams.m_adw[3] = m_nInt3;
	g_RobotAIIntParams.m_adw[7] = m_nInt7;
	g_RobotAIIntParams.m_adw[6] = m_nInt6;
	g_RobotAIIntParams.m_adw[5] = m_nInt5;
	g_RobotAIIntParams.m_adw[8] = m_nInt8;
	g_RobotAIExpParams.m_adw[1] = m_nExp1;
	g_RobotAIExpParams.m_adw[2] = DBL_TO_FIX(m_dblExp2);
	g_RobotAIExpParams.m_adw[0] = DBL_TO_FIX(m_dblExp0);
	g_RobotAIExpParams.m_adw[9] = m_nExp9;
	g_RobotAIExpParams.m_adw[4] = m_nExp4;
	g_RobotAIExpParams.m_adw[3] = m_nExp3;
	g_RobotAIExpParams.m_adw[7] = m_nExp7;
	g_RobotAIExpParams.m_adw[6] = m_nExp6;
	g_RobotAIExpParams.m_adw[5] = m_nExp5;
	g_RobotAIExpParams.m_adw[8] = m_nExp8;
}

// hover.exe:0x432F50
// FUNCTION: HOVER 0x432f50
BOOL CSub432A50::OnInitDialog()
{
	FIXED1616 fx;

	m_nBeg1 = g_RobotAIBegParams.m_adw[1];
	fx.dw = g_RobotAIBegParams.m_adw[2];
	m_dblBeg2 = FIX_TO_DBL(fx);
	fx.dw = g_RobotAIBegParams.m_adw[0];
	m_dblBeg0 = FIX_TO_DBL(fx);
	m_nBeg9 = g_RobotAIBegParams.m_adw[9];
	m_nBeg4 = g_RobotAIBegParams.m_adw[4];
	m_nBeg3 = g_RobotAIBegParams.m_adw[3];
	m_nBeg7 = g_RobotAIBegParams.m_adw[7];
	m_nBeg6 = g_RobotAIBegParams.m_adw[6];
	m_nBeg5 = g_RobotAIBegParams.m_adw[5];
	m_nBeg8 = g_RobotAIBegParams.m_adw[8];
	m_nInt1 = g_RobotAIIntParams.m_adw[1];
	fx.dw = g_RobotAIIntParams.m_adw[2];
	m_dblInt2 = FIX_TO_DBL(fx);
	fx.dw = g_RobotAIIntParams.m_adw[0];
	m_dblInt0 = FIX_TO_DBL(fx);
	m_nInt9 = g_RobotAIIntParams.m_adw[9];
	m_nInt4 = g_RobotAIIntParams.m_adw[4];
	m_nInt3 = g_RobotAIIntParams.m_adw[3];
	m_nInt7 = g_RobotAIIntParams.m_adw[7];
	m_nInt6 = g_RobotAIIntParams.m_adw[6];
	m_nInt5 = g_RobotAIIntParams.m_adw[5];
	m_nInt8 = g_RobotAIIntParams.m_adw[8];
	m_nExp1 = g_RobotAIExpParams.m_adw[1];
	fx.dw = g_RobotAIExpParams.m_adw[2];
	m_dblExp2 = FIX_TO_DBL(fx);
	fx.dw = g_RobotAIExpParams.m_adw[0];
	m_dblExp0 = FIX_TO_DBL(fx);
	m_nExp9 = g_RobotAIExpParams.m_adw[9];
	m_nExp4 = g_RobotAIExpParams.m_adw[4];
	m_nExp3 = g_RobotAIExpParams.m_adw[3];
	m_nExp7 = g_RobotAIExpParams.m_adw[7];
	m_nExp6 = g_RobotAIExpParams.m_adw[6];
	m_nExp5 = g_RobotAIExpParams.m_adw[5];
	m_nExp8 = g_RobotAIExpParams.m_adw[8];
	CDialog::OnInitDialog();
	return TRUE;
}

// hover.exe:0x432F30
// FUNCTION: HOVER 0x432f30
void CSub432A50::OnOK()
{
	CDialog::OnOK();
	Sub_432DB0();
}

// hover.exe:0x433270
// FUNCTION: HOVER 0x433270
void CSub432A50::OnDeleteSettings()
{
	HKEY hKey;

	if (RegOpenKeyExA(HKEY_CURRENT_USER, "Software\\Microsoft\\Hover!", 0,
			KEY_ALL_ACCESS, &hKey) == ERROR_SUCCESS)
	{
		RegDeleteValueA(hKey, "RobotAIBegParams");
		RegDeleteValueA(hKey, "RobotAIIntParams");
		RegDeleteValueA(hKey, "RobotAIExpParams");
		RegCloseKey(hKey);
	}
}

// hover.exe:0x4332D0
// FUNCTION: HOVER 0x4332d0
void CSub432A50::OnSaveSettings()
{
	HKEY hKey;
	DWORD dwDisposition;

	UpdateData(TRUE);
	Sub_432DB0();

	if (RegCreateKeyExA(HKEY_CURRENT_USER, "Software\\Microsoft\\Hover!", 0,
			NULL, 0, KEY_ALL_ACCESS, NULL, &hKey, &dwDisposition)
			== ERROR_SUCCESS)
	{
		RegSetValueExA(hKey, "RobotAIBegParams", 0, REG_BINARY,
			(LPBYTE)&g_RobotAIBegParams, sizeof(g_RobotAIBegParams));
		RegSetValueExA(hKey, "RobotAIIntParams", 0, REG_BINARY,
			(LPBYTE)&g_RobotAIIntParams, sizeof(g_RobotAIIntParams));
		RegSetValueExA(hKey, "RobotAIExpParams", 0, REG_BINARY,
			(LPBYTE)&g_RobotAIExpParams, sizeof(g_RobotAIExpParams));
		RegCloseKey(hKey);
	}
}

/////////////////////////////////////////////////////////////////////////////
// CSub424720 dialog ("Please Wait")

// GLOBAL: HOVER 0x4c57c4
DWORD g_dwUnk_4C57C4;			// hover.exe:0x4C57C4 items done
// GLOBAL: HOVER 0x4c57c8
DWORD g_dwUnk_4C57C8;			// hover.exe:0x4C57C8 items total

BEGIN_MESSAGE_MAP(CSub424720, CDialog)
	//{{AFX_MSG_MAP(CSub424720)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// hover.exe:0x4247D0
// FUNCTION: HOVER 0x4247d0
void CSub424720::DoDataExchange(CDataExchange* pDX)
{
	//{{AFX_DATA_MAP(CSub424720)
	DDX_Control(pDX, IDC_LOADING_FRAME, m_staticFrame);
	//}}AFX_DATA_MAP
}

// Positions the dialog. While the splash screen is up it hands down its
// own centre in g_dwUnk_4A4878/4A487C and the dialog is hung under it;
// otherwise the dialog is centred on the view (or, once the view has a
// board rectangle, put just below it).

// hover.exe:0x4247F0
// FUNCTION: HOVER 0x4247f0
BOOL CSub424720::OnInitDialog()
{
	RECT rc;

	CDialog::OnInitDialog();
	::GetWindowRect(m_hWnd, &rc);

	if (g_dwUnk_4A4878 != (DWORD)-1)
	{
		int x;
		int cx, cy;

		cy = rc.bottom - rc.top;
		cx = rc.right - rc.left;
		x = g_dwUnk_4A4878 + (rc.left - rc.right) / 2;
		::MoveWindow(m_hWnd, x, g_dwUnk_4A487C, cx, cy, FALSE);
	}
	else
	{
		RECT rcView;
		CBumperView* pView;
		int x, y;

		pView = Hover_sub_401140();
		rcView = pView->m_rcUnk0094;
		if (rcView.bottom != -1)
		{
			::ClientToScreen(pView->m_hWnd, (LPPOINT)&rcView);
			::ClientToScreen(pView->m_hWnd, ((LPPOINT)&rcView) + 1);
			x = (rc.left - rc.right) / 2 + (rcView.right + rcView.left) / 2;
			y = rcView.bottom + 10;
		}
		else
		{
			::GetWindowRect(pView->m_hWnd, &rcView);
			x = (rc.left - rc.right) / 2 + (rcView.right + rcView.left) / 2;
			y = (rc.top - rc.bottom) / 2 + (rcView.top + rcView.bottom) / 2;
		}

		::SetWindowPos(m_hWnd, NULL, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
	}
	return TRUE;
}

// The texture loader's progress callback, installed in g_pfnUnk_4C4DAC by
// CHoverApp::Sub_41C8E0 while a level loads. It pumps the wave mixer (the
// load blocks the message loop, so the music would otherwise stop) and
// paints a percentage bar over the IDD_LOADING frame control: the same
// "nnn%" string is drawn twice with the two colour pairs swapped, clipped
// to the filled and the unfilled part of the frame rectangle, so the text
// reads white-on-blue where the bar has reached it and blue-on-white
// beyond it. Both draws place the text at the same spot, centred over the
// whole frame; the original recomputes the position for the second call.
//
// g_dwUnk_4C57CC/g_dwUnk_4C57D0 scale the percentage into a sub-range (the
// view sets them to 50..100 for the second half of a level load), so a
// caller can drive the bar through several passes.

// hover.exe:0x40EB50
// FUNCTION: HOVER 0x40eb50
void __cdecl Hover_sub_40EB50(DWORD dwDone, DWORD dwTotal)
{
	CString str;
	CSub424720* pDlg;

	Hover_sub_439A80();

	g_dwUnk_4C57C4 = dwDone;
	g_dwUnk_4C57C8 = dwTotal;

	pDlg = g_pLoadDlg_4C57D4;
	if (pDlg != NULL)
	{
		CClientDC dc(pDlg);
		RECT rc;
		RECT rcFilled;
		CSize size;
		int nWidth;
		int nHeight;
		int nTotal;
		int nPercent;

		dc.SelectObject(CFont::FromHandle(
			(HFONT)::GetStockObject(SYSTEM_FONT)));

		// The frame control's rectangle in the dialog's client coordinates.
		g_pLoadDlg_4C57D4->m_staticFrame.GetClientRect(&rc);
		nWidth = rc.right;
		nHeight = rc.bottom;
		g_pLoadDlg_4C57D4->m_staticFrame.ClientToScreen(&rc);
		g_pLoadDlg_4C57D4->ScreenToClient(&rc);
		rc.right = rc.left + nWidth - 1;
		rc.bottom = rc.top + nHeight - 1;

		nTotal = (int)g_dwUnk_4C57C8;
		if (nTotal <= 0)
			nTotal = 1;
		if ((int)g_dwUnk_4C57C4 > nTotal)
			g_dwUnk_4C57C4 = nTotal;
		g_dwUnk_4C57C8 = nTotal;

		rcFilled = rc;

		nPercent = (int)g_dwUnk_4C57CC;
		nPercent += ((int)g_dwUnk_4C57D0 - nPercent)
			* (int)g_dwUnk_4C57C4 / nTotal;

		rc.left += ((rc.right - rc.left) * nPercent) / 100;
		rcFilled.right = rc.left;

		str.Format("%3d%%", nPercent);
		size = dc.GetTextExtent(str, str.GetLength());

		// The filled part: white on blue, clipped to rcFilled.
		dc.SetTextColor(RGB(255, 255, 255));
		dc.SetBkColor(RGB(0, 0, 127));
		dc.ExtTextOut(
			rcFilled.left + (rc.right - size.cx - rcFilled.left) / 2,
			rcFilled.top + (rcFilled.bottom - size.cy - rcFilled.top) / 2,
			ETO_OPAQUE | ETO_CLIPPED, &rcFilled, str, str.GetLength(),
			NULL);

		// The rest: blue on white, clipped to what is left of rc. The
		// original recomputes the (identical) text position here.
		dc.SetBkColor(RGB(255, 255, 255));
		dc.SetTextColor(RGB(0, 0, 127));
		dc.ExtTextOut(
			rcFilled.left + (rc.right - size.cx - rcFilled.left) / 2,
			rcFilled.top + (rcFilled.bottom - size.cy - rcFilled.top) / 2,
			ETO_OPAQUE | ETO_CLIPPED, &rc, str, str.GetLength(), NULL);
	}
}

/////////////////////////////////////////////////////////////////////////////
