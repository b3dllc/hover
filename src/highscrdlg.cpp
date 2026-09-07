// highscrdlg.cpp : implementation of the high score dialogs
//
// Reconstructed from hover.exe; see highscrdlg.h for the vtable/message
// map data and the notes on member offsets.

#include "hover.h"
#include "highscrdlg.h"

#include <string.h>

/////////////////////////////////////////////////////////////////////////////
// CSub424BD0 dialog

// hover.exe:0x4BF8B8
// Help context ID pairs (control ID, help context ID) passed to WinHelp
// with HELP_WM_HELP / HELP_CONTEXTMENU; terminated by a zero pair.
static DWORD s_adwHelpMap[] =
{
	IDC_CLEARSCORES, 2210,
	1071, 2200,
	1070, 2200,
	1072, 2200,
	1073, 2200,
	1074, 2200,
	1075, 2200,
	1076, 2200,
	1077, 2200,
	1078, 2200,
	1079, 2200,
	1090, 2200,
	1091, 2200,
	1092, 2200,
	1093, 2200,
	1094, 2200,
	1080, 2200,
	1081, 2200,
	1082, 2200,
	1083, 2200,
	1084, 2200,
	1085, 2200,
	1086, 2200,
	1087, 2200,
	1088, 2200,
	1089, 2200,
	1095, 2200,
	1096, 2200,
	1097, 2200,
	1098, 2200,
	1099, 2200,
	0, 0
};

BEGIN_MESSAGE_MAP(CSub424BD0, CDialog)
	//{{AFX_MSG_MAP(CSub424BD0)
	ON_MESSAGE(WM_HELP, Sub_434010)
	ON_MESSAGE(WM_CONTEXTMENU, Sub_434030)
	ON_COMMAND(IDC_CLEARSCORES, Sub_433C60)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// hover.exe:0x424BD0
// FUNCTION: HOVER 0x424bd0
CSub424BD0::CSub424BD0(CWnd* pParent) : CDialog(IDD_HIGHSCORES, pParent)
{
	m_dwUnk134 = 0;
}

// hover.exe:0x41D540
// FUNCTION: HOVER 0x41d540
CSub424BD0::~CSub424BD0()
{
}

// hover.exe:0x424C80
// FUNCTION: HOVER 0x424c80
void CSub424BD0::Sub_424C80()
{
	HKEY hKey;
	// hover.exe places the RegOpenKeyExA failure handler at the end of the
	// function, not inline where the guard clause is written; an if/else
	// with everything else nested in the "if" arm (rule 93) reproduces
	// that layout instead of duplicating the shared epilogue.
	if (RegOpenKeyExA(HKEY_CURRENT_USER, "Software\\Microsoft\\Hover!", 0,
			KEY_ALL_ACCESS, &hKey) == ERROR_SUCCESS) {
		DWORD dwType = REG_MULTI_SZ;
		DWORD cbData;
		if (RegQueryValueExA(hKey, "HighScoreNames", 0, &dwType, NULL,
				&cbData) != ERROR_SUCCESS || cbData == 0) {
			RegCloseKey(hKey);
			Sub_433CF0(FALSE);
			return;
		}

		char* pNames = new char[cbData];
		RegQueryValueExA(hKey, "HighScoreNames", 0, &dwType, (LPBYTE)pNames, &cbData);

		dwType = REG_BINARY;
		cbData = sizeof(m_adwScores);
		if (RegQueryValueExA(hKey, "HighScores", 0, &dwType, (LPBYTE)m_adwScores,
				&cbData) != ERROR_SUCCESS || cbData != sizeof(m_adwScores)) {
			RegCloseKey(hKey);
			Sub_433CF0(FALSE);
			delete[] pNames;
			return;
		}

		char* p = pNames;
		for (int i = 0; i < 15; i++) {
			m_astrNames[i].Empty();
			while (*p != '\0')
				m_astrNames[i] += *p++;
			p++;
		}

		delete[] pNames;
		RegCloseKey(hKey);
	}
	else {
		Sub_433CF0(FALSE);
		Sub_433940();
	}
}

// hover.exe:0x424DF0
// FUNCTION: HOVER 0x424df0
BOOL CSub424BD0::Sub_424DF0(DWORD dwScore)
{
	Sub_424C80();

	if ((long)m_adwScores[14] < (long)dwScore) {
		CSub434170 dlg(NULL);

		if (dlg.DoModal() == IDOK) {
			if (dlg.m_strName.IsEmpty())
				dlg.m_strName = "  ";
			m_adwScores[14] = dwScore;
			m_astrNames[14] = dlg.m_strName;
			Sub_433940();
			Sub_433F10();
			return TRUE;
		}
	}

	return FALSE;
}

// hover.exe:0x434050
// FUNCTION: HOVER 0x434050
void CSub424BD0::OnOK()
{
	Sub_433F10();
	CDialog::OnOK();
}

// hover.exe:0x433EE0
// This is OnInitDialog, not OnCancel: hover.exe:0x433ee3 calls
// CDialog::OnInitDialog (0x44a9b0, confirmed by tools/libmatch.py --show)
// and the function returns TRUE in eax, which a void OnCancel would not do.
// FUNCTION: HOVER 0x433ee0
BOOL CSub424BD0::OnInitDialog()
{
	CDialog::OnInitDialog();
	m_dwUnk134 = 1;
	Sub_424C80();
	Sub_433940();
	return TRUE;
}

// hover.exe:0x433840
// FUNCTION: HOVER 0x433840
void CSub424BD0::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

// hover.exe:0x434010
// FUNCTION: HOVER 0x434010
LRESULT CSub424BD0::Sub_434010(WPARAM wParam, LPARAM lParam)
{
	return ::WinHelp((HWND)((LPHELPINFO)lParam)->hItemHandle, "hover.hlp",
		HELP_WM_HELP, (DWORD)s_adwHelpMap);
}

// hover.exe:0x434030
// FUNCTION: HOVER 0x434030
LRESULT CSub424BD0::Sub_434030(WPARAM wParam, LPARAM lParam)
{
	::WinHelp((HWND)wParam, "hover.hlp", HELP_CONTEXTMENU, (DWORD)s_adwHelpMap);
	return 1;
}

// hover.exe:0x433C60
// FUNCTION: HOVER 0x433c60
void CSub424BD0::Sub_433C60()
{
	CString str;

	if (str.LoadString(IDS_CLEARSCORES_CONFIRM)) {
		if (AfxMessageBox(str, MB_OKCANCEL) == IDOK) {
			Sub_433CF0(TRUE);
			Sub_433940();
		}
	}
}

// Formats dwScore with thousands separators (e.g. "9,999,999,999") into
// the dialog's static control nID.

// hover.exe:0x433860
// FUNCTION: HOVER 0x433860
void CSub424BD0::Sub_433860(UINT nID, DWORD dwScore)
{
	CString strNum;
	CString strText;
	int nLast;
	int i;

	strNum.Format("%ld", dwScore);
	nLast = strNum.GetLength() - 1;
	for (i = 0; i <= nLast; i++)
	{
		strText += strNum[i];
		if (nLast > i && (nLast - i) % 3 == 0)
			strText += ',';
	}
	SetDlgItemText(nID, strText);
}

// hover.exe:0x433940
// FUNCTION: HOVER 0x433940
void CSub424BD0::Sub_433940()
{
	CString strTemp;
	DWORD dwScore;
	int i;
	int j;

	// Insertion sort, highest score first (m_adwScores[14] is the lowest
	// score on the list).
	for (i = 1; i < 15; i++)
	{
		dwScore = m_adwScores[i];
		strTemp = m_astrNames[i];
		for (j = i - 1; j >= 0; j--)
		{
			if ((long)m_adwScores[j] >= (long)dwScore)
				break;
			m_adwScores[j + 1] = m_adwScores[j];
			m_astrNames[j + 1] = m_astrNames[j];
		}
		m_adwScores[j + 1] = dwScore;
		m_astrNames[j + 1] = strTemp;
	}
	if (m_dwUnk134 != 0)
	{
		SetDlgItemText(1071, m_astrNames[0]);
		SetDlgItemText(1070, m_astrNames[1]);
		SetDlgItemText(1072, m_astrNames[2]);
		SetDlgItemText(1073, m_astrNames[3]);
		SetDlgItemText(1074, m_astrNames[4]);
		SetDlgItemText(1075, m_astrNames[5]);
		SetDlgItemText(1076, m_astrNames[6]);
		SetDlgItemText(1077, m_astrNames[7]);
		SetDlgItemText(1078, m_astrNames[8]);
		SetDlgItemText(1079, m_astrNames[9]);
		SetDlgItemText(1090, m_astrNames[10]);
		SetDlgItemText(1091, m_astrNames[11]);
		SetDlgItemText(1092, m_astrNames[12]);
		SetDlgItemText(1093, m_astrNames[13]);
		SetDlgItemText(1094, m_astrNames[14]);
		Sub_433860(1080, m_adwScores[0]);
		Sub_433860(1081, m_adwScores[1]);
		Sub_433860(1082, m_adwScores[2]);
		Sub_433860(1083, m_adwScores[3]);
		Sub_433860(1084, m_adwScores[4]);
		Sub_433860(1085, m_adwScores[5]);
		Sub_433860(1086, m_adwScores[6]);
		Sub_433860(1087, m_adwScores[7]);
		Sub_433860(1088, m_adwScores[8]);
		Sub_433860(1089, m_adwScores[9]);
		Sub_433860(1095, m_adwScores[10]);
		Sub_433860(1096, m_adwScores[11]);
		Sub_433860(1097, m_adwScores[12]);
		Sub_433860(1098, m_adwScores[13]);
		Sub_433860(1099, m_adwScores[14]);
	}
}

// hover.exe:0x433CF0
// FUNCTION: HOVER 0x433cf0
void CSub424BD0::Sub_433CF0(BOOL bReset)
{
	DWORD dwTemp;
	int i;
	int j;

	if (bReset)
	{
		for (i = 0; i < 15; i++)
		{
			m_astrNames[i].Empty();
			m_adwScores[i] = 0;
		}
		return;
	}
	m_astrNames[0] = "Greg";
	m_astrNames[1] = "John";
	m_astrNames[2] = "Josh";
	m_astrNames[3] = "Steve";
	m_astrNames[4] = "Larry";
	m_astrNames[5] = "Jo";
	m_astrNames[6] = "Jon";
	m_astrNames[7] = "Patrick";
	m_astrNames[8] = "Julie";
	m_astrNames[9] = "Tony";
	m_astrNames[10] = "Charles";
	m_astrNames[11] = "Joy";
	m_astrNames[12] = "Jennifer";
	m_astrNames[13] = "Harley";
	m_astrNames[14] = "Kevin";
	m_adwScores[0] = 0x5FFA;
	m_adwScores[1] = 0x5994;
	m_adwScores[2] = 0x532E;
	m_adwScores[3] = 0x4CC8;
	m_adwScores[4] = 0x4662;
	m_adwScores[5] = 0x3FFC;
	m_adwScores[6] = 0x3996;
	m_adwScores[7] = 0x3330;
	m_adwScores[8] = 0x2CCA;
	m_adwScores[9] = 0x2664;
	m_adwScores[10] = 0x1FFE;
	m_adwScores[11] = 0x1998;
	m_adwScores[12] = 0x1332;
	m_adwScores[13] = 0xCCC;
	m_adwScores[14] = 0x666;
	// shuffle the default scores over the fixed default names
	srand((unsigned)time(NULL));
	i = 15;
	do
	{
		j = rand() % i;
		i--;
		dwTemp = m_adwScores[j];
		m_adwScores[j] = m_adwScores[i];
		m_adwScores[i] = dwTemp;
	}
	while (i != 0);
}

// hover.exe:0x433F10
// FUNCTION: HOVER 0x433f10
void CSub424BD0::Sub_433F10()
{
	HKEY hKey;
	DWORD dwDisp;
	char* pNames;
	char* p;
	int cb;
	int nLen;
	int i;

	cb = 0;
	for (i = 0; i < 15; i++)
		cb += m_astrNames[i].GetLength();
	cb += 0x11;	// one NUL per name plus the REG_MULTI_SZ terminator
	pNames = new char[cb];
	p = pNames;
	for (i = 0; i < 15; i++)
	{
		nLen = m_astrNames[i].GetLength();
		memcpy(p, (const char*)m_astrNames[i], nLen);
		p += nLen;
		*p++ = '\0';
	}
	p[0] = '\0';
	p[1] = '\0';
	if (RegCreateKeyExA(HKEY_CURRENT_USER, "Software\\Microsoft\\Hover!", 0,
			NULL, 0, KEY_ALL_ACCESS, NULL, &hKey, &dwDisp) == ERROR_SUCCESS)
	{
		RegSetValueExA(hKey, "HighScoreNames", 0, REG_MULTI_SZ,
			(CONST BYTE*)pNames, cb);
		RegSetValueExA(hKey, "HighScores", 0, REG_BINARY,
			(CONST BYTE*)m_adwScores, 0x3C);
		RegCloseKey(hKey);
	}
	delete pNames;
}

/////////////////////////////////////////////////////////////////////////////
// CSub434170 dialog

BEGIN_MESSAGE_MAP(CSub434170, CDialog)
	//{{AFX_MSG_MAP(CSub434170)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// hover.exe:0x434170
// FUNCTION: HOVER 0x434170
CSub434170::CSub434170(CWnd* pParent) : CDialog(IDD_ENTERHIGHSCORE, pParent)
{
	m_strName = "";
}

// hover.exe:0x434100
// FUNCTION: HOVER 0x434100
CSub434170::~CSub434170()
{
}

// hover.exe:0x434200
// FUNCTION: HOVER 0x434200
void CSub434170::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSub434170)
	DDX_Text(pDX, IDC_HIGHSCORE_NAME, m_strName);
	DDV_MaxChars(pDX, m_strName, 26);
	//}}AFX_DATA_MAP
}
