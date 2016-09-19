// CGlobalSettings.cpp : implementation file
//

#include "stdafx.h"
#include "dxwndhost.h"
//#include "dxwndhostView.h"
#include "CGlobalSettings.h"

extern char gInitPath[];
extern BOOL gbDebug;

Key_Type FKeys[] = {
	{IDC_KEY_LABEL1,	"Time toggle",	IDC_KEY_COMBO1,		"timetoggle", 0},
	{IDC_KEY_LABEL2,	"Time fast",	IDC_KEY_COMBO2,		"timefast", 0},
	{IDC_KEY_LABEL3,	"Time slow",	IDC_KEY_COMBO3,		"timeslow", 0},
	{IDC_KEY_LABEL4,	"Alt-F4",		IDC_KEY_COMBO4,		"altf4", 0},
	{IDC_KEY_LABEL5,	"Clip toggle",	IDC_KEY_COMBO5,		"cliptoggle", 0},
	{IDC_KEY_LABEL6,	"Refresh",		IDC_KEY_COMBO6,		"refresh", 0},
	{IDC_KEY_LABEL7,	"Log toggle",	IDC_KEY_COMBO7,		"logtoggle", 0},
	{IDC_KEY_LABEL8,	"Position t.",	IDC_KEY_COMBO8,		"plogtoggle", 0},
	{IDC_KEY_LABEL9,	"FPS toggle",	IDC_KEY_COMBO9,		"fpstoggle", 0},
	{IDC_KEY_LABEL10,	"Print screen",	IDC_KEY_COMBO10,	"printscreen", 0},
	{IDC_KEY_LABEL11,	"Corner tog.",	IDC_KEY_COMBO11,	"corner", 0},
	{IDC_KEY_LABEL12,	"Time freeze",	IDC_KEY_COMBO12,	"freezetime", 0},
	{IDC_KEY_LABEL13,	"Fullscr. t.",	IDC_KEY_COMBO13,	"fullscreen", 0},
	{IDC_KEY_LABEL14,	"Work area t.",	IDC_KEY_COMBO14,	"workarea", 0},
	{IDC_KEY_LABEL15,	"Desktop t.",	IDC_KEY_COMBO15,	"desktop", 0},
	{0, "", 0, 0}
};

Key_Type HKeys[] = {
	{IDC_HKEY_LABEL1,	"Minimize",		IDC_HKEY_COMBO1,	"minimize", 0},
	{IDC_HKEY_LABEL2,	"Restore",		IDC_HKEY_COMBO2,	"restore", 0},
	{IDC_HKEY_LABEL3,	"Kill proc.",	IDC_HKEY_COMBO3,	"kill", 0},
	{0, "", 0, 0}
};

KeyCombo_Type FKeyCombo[] = {
	{-1, "--"},
	{VK_F1, "F1"},
	{VK_F2, "F2"},
	{VK_F3, "F3"},
	{VK_F5, "F5"},
	{VK_F6, "F6"},
	{VK_F7, "F7"},
	{VK_F8, "F8"},
	{VK_F9, "F9"},
	{VK_F10, "F10"},
	{VK_F11, "F11"},
	{VK_F12, "F12"},
	{0, ""}
};

KeyCombo_Type HKeyCombo[] = {
	{-1, "--"},
	{VK_END, "END"},
	{VK_HOME, "HOME"},
	{VK_DELETE, "DEL"},
	{VK_ESCAPE, "ESC"},
	{VK_F1, "F1"},
	{VK_F2, "F2"},
	{VK_F3, "F3"},
	{VK_F5, "F5"},
	{VK_F6, "F6"},
	{VK_F7, "F7"},
	{VK_F8, "F8"},
	{VK_F9, "F9"},
	{VK_F10, "F10"},
	{VK_F11, "F11"},
	{VK_F12, "F12"},
	{0, ""}
};

// CGlobalSettings dialog

IMPLEMENT_DYNAMIC(CGlobalSettings, CDialog)

CGlobalSettings::CGlobalSettings(CWnd* pParent /*=NULL*/)
	: CDialog(CGlobalSettings::IDD, pParent)
{
}

CGlobalSettings::~CGlobalSettings()
{
}

void CGlobalSettings::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Check(pDX, IDC_CONFIG_DEBUGMODE, m_DebugMode);
	DDX_Check(pDX, IDC_CONFIG_AUTOHIDE, m_AutoHideMode);
	DDX_Check(pDX, IDC_CONFIG_CHECKADMIN, m_CheckAdminRights);
	DDX_Check(pDX, IDC_CONFIG_NAMEFROMFOLDER, m_NameFromFolder);
	DDX_Check(pDX, IDC_CONFIG_MULTIHOOKS, m_MultiHooks);
	DDX_Check(pDX, IDC_CONFIG_SAVEPATHS, m_UpdatePaths);
	DDX_Text (pDX, IDC_TEX_MINX, m_TexMinX);
	DDX_Text (pDX, IDC_TEX_MINY, m_TexMinY);
	DDX_Text (pDX, IDC_TEX_MAXX, m_TexMaxX);
	DDX_Text (pDX, IDC_TEX_MAXY, m_TexMaxY);
}

BEGIN_MESSAGE_MAP(CGlobalSettings, CDialog)
END_MESSAGE_MAP()

#define IDPaletteTIMER 2

// CPaletteDialog message handlers

//static void SetKeys(HWND hDlg, Key_Type *FKeys, KeyCombo_Type *FKeyCombo)
//{
//	for(int i=0; FKeys[i].iLabelResourceId; i++){
//		int iCursor = 0;
//		DWORD dwKey;
//		dwKey = GetPrivateProfileInt("keymapping", FKeys[i].sIniLabel, -1, gInitPath);
//		SetDlgItemText(hDlg,FKeys[i].iLabelResourceId,FKeys[i].sLabel);
//		CComboBox *pCombo=(CComboBox *)GetDlgItem(FKeys[i].iComboResourceId);
//		pCombo->Clear();
//		for(int j=0; FKeyCombo[j].dwVKeyCode; j++) {
//			pCombo->AddString(FKeyCombo[j].sVKeyLabel);
//			if(dwKey == FKeyCombo[j].dwVKeyCode) iCursor=j;
//		}
//		pCombo->SetCurSel(iCursor);
//	}
//}

BOOL CGlobalSettings::OnInitDialog()
{
	for(int i=0; FKeys[i].iLabelResourceId; i++){
		int iCursor = 0;
		DWORD dwKey;
		dwKey = GetPrivateProfileInt("keymapping", FKeys[i].sIniLabel, -1, gInitPath);
		SetDlgItemText(FKeys[i].iLabelResourceId,FKeys[i].sLabel);
		CComboBox *pCombo=(CComboBox *)GetDlgItem(FKeys[i].iComboResourceId);
		pCombo->Clear();
		for(int j=0; FKeyCombo[j].dwVKeyCode; j++) {
			pCombo->AddString(FKeyCombo[j].sVKeyLabel);
			if(dwKey == FKeyCombo[j].dwVKeyCode) iCursor=j;
		}
		pCombo->SetCurSel(iCursor);
	}
	for(int i=0; HKeys[i].iLabelResourceId; i++){
		int iCursor = 0;
		DWORD dwKey;
		dwKey = GetPrivateProfileInt("keymapping", HKeys[i].sIniLabel, -1, gInitPath);
		SetDlgItemText(HKeys[i].iLabelResourceId,HKeys[i].sLabel);
		CComboBox *pCombo=(CComboBox *)GetDlgItem(HKeys[i].iComboResourceId);
		pCombo->Clear();
		for(int j=0; HKeyCombo[j].dwVKeyCode; j++) {
			pCombo->AddString(HKeyCombo[j].sVKeyLabel);
			if(dwKey == HKeyCombo[j].dwVKeyCode) iCursor=j;
		}
		pCombo->SetCurSel(iCursor);
	}
	//SetKeys(this, FKeys, FKeyCombo);
	//SetKeys(this, HKeys, HKeyCombo);

	m_DebugMode = GetPrivateProfileInt("window", "debug", 0, gInitPath);
	m_AutoHideMode = GetPrivateProfileInt("window", "autohide", 0, gInitPath); 
	m_CheckAdminRights = GetPrivateProfileInt("window", "checkadmin", 0, gInitPath);
	m_NameFromFolder = GetPrivateProfileInt("window", "namefromfolder", 0, gInitPath);
	m_MultiHooks = GetPrivateProfileInt("window", "multiprocesshook", 0, gInitPath);
	m_UpdatePaths = GetPrivateProfileInt("window", "updatepaths", 1, gInitPath);
	m_TexMinX = GetPrivateProfileInt("texture", "MinTexX", 0, gInitPath);
	m_TexMinY = GetPrivateProfileInt("texture", "MinTexY", 0, gInitPath);
	m_TexMaxX = GetPrivateProfileInt("texture", "MaxTexX", 0, gInitPath);
	m_TexMaxY = GetPrivateProfileInt("texture", "MaxTexY", 0, gInitPath);
	CDialog::OnInitDialog();
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CGlobalSettings::OnOK()
{
	char val[32];
	CDialog::OnOK();
	// boolean flags
	sprintf_s(val, sizeof(val), "%i", m_DebugMode);
	WritePrivateProfileString("window", "debug", val, gInitPath);
	sprintf_s(val, sizeof(val), "%i", m_AutoHideMode);
	WritePrivateProfileString("window", "autohide", val, gInitPath);
	sprintf_s(val, sizeof(val), "%i", m_CheckAdminRights);
	WritePrivateProfileString("window", "checkadmin", val, gInitPath);
	sprintf_s(val, sizeof(val), "%i", m_NameFromFolder);
	WritePrivateProfileString("window", "namefromfolder", val, gInitPath);
	sprintf_s(val, sizeof(val), "%i", m_MultiHooks);
	WritePrivateProfileString("window", "multiprocesshook", val, gInitPath);
	sprintf_s(val, sizeof(val), "%i", m_UpdatePaths);
	WritePrivateProfileString("window", "updatepaths", val, gInitPath);
	// texture limits
	sprintf_s(val, sizeof(val), "%i", m_TexMinX);
	WritePrivateProfileString("texture", "MinTexX", val, gInitPath);
	sprintf_s(val, sizeof(val), "%i", m_TexMinY);
	WritePrivateProfileString("texture", "MinTexY", val, gInitPath);
	sprintf_s(val, sizeof(val), "%i", m_TexMaxX);
	WritePrivateProfileString("texture", "MaxTexX", val, gInitPath);
	sprintf_s(val, sizeof(val), "%i", m_TexMaxY);
	WritePrivateProfileString("texture", "MaxTexY", val, gInitPath);
	// fkeys
	for(int i=0; FKeys[i].iLabelResourceId; i++){
		int iCursor = 0;
		DWORD dwKey;
		CComboBox *pCombo=(CComboBox *)GetDlgItem(FKeys[i].iComboResourceId);
		dwKey = FKeyCombo[pCombo->GetCurSel()].dwVKeyCode;
		if(dwKey != -1) {
			char sKNum[20];
			sprintf_s(sKNum, sizeof(sKNum), "%i", dwKey);
			WritePrivateProfileString("keymapping", FKeys[i].sIniLabel, sKNum, gInitPath);
		}
		else 
			WritePrivateProfileString("keymapping", FKeys[i].sIniLabel, "", gInitPath);
	}
	// hot keys
	for(int i=0; HKeys[i].iLabelResourceId; i++){
		int iCursor = 0;
		DWORD dwKey;
		CComboBox *pCombo=(CComboBox *)GetDlgItem(HKeys[i].iComboResourceId);
		dwKey = HKeyCombo[pCombo->GetCurSel()].dwVKeyCode;
		if(dwKey != -1) {
			char sKNum[20];
			sprintf_s(sKNum, sizeof(sKNum), "%i", dwKey);
			WritePrivateProfileString("keymapping", HKeys[i].sIniLabel, sKNum, gInitPath);
		}
		else 
			WritePrivateProfileString("keymapping", HKeys[i].sIniLabel, "", gInitPath);
	}
	//(CDxwndhostView *)(this->GetParent())->UpdateHotKeys();
	gbDebug = m_DebugMode;
}
