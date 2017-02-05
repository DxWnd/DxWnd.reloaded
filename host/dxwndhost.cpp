// dxwndhost.cpp : Application and class definitions
//

#include "stdafx.h"
#include "dxwndhost.h"

#include "MainFrm.h"
#include "dxwndhostDoc.h"
#include "dxwndhostView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern int KillProcByName(char *, BOOL, BOOL);

/////////////////////////////////////////////////////////////////////////////
// CDxwndhostApp

BEGIN_MESSAGE_MAP(CDxwndhostApp, CWinApp)
	//{{AFX_MSG_MAP(CDxwndhostApp)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
	ON_COMMAND(ID_HELP_VIEWHELP, OnViewHelp)
	//}}AFX_MSG_MAP
	// Basic file command 
	ON_COMMAND(ID_FILE_NEW, CWinApp::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, CWinApp::OnFileOpen)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// custom command line parsing:
// CNewCommandLineInfo Class constructor

// too do: eliminate nasty global variables....
UINT m_StartToTray = FALSE;
UINT m_InitialState = DXW_ACTIVE;
BOOL gbDebug = FALSE;
BOOL gWarnOnExit = FALSE;
BOOL gTransientMode = FALSE;
BOOL gAutoHideMode = FALSE;
BOOL gQuietMode = FALSE;
BOOL gMustDie = FALSE;
int iProgIndex;
extern char m_ConfigFileName[20+1] = "dxwnd.ini";

class CNewCommandLineInfo : public CCommandLineInfo
{
//public:
//	UINT m_StartToTray;
//	UINT m_InitialState;

	void ParseParam(LPCTSTR lpszParam, BOOL bFlag, BOOL bLast);
};

BOOL LangSelected=FALSE;
char LangString[20+1] = {0};

void CNewCommandLineInfo::ParseParam(LPCTSTR lpszParam, BOOL bFlag, BOOL bLast)
{
	// syntax:
	// /t -- start minimized in tray icon
	// /i -- start in idle state
	// /d -- start in debug mode
	// /lang=<XX> -- loads the language resources in Resources_<XX>.dll extension
	// /c:<path> -- loads <path> config file instead of default dxwnd.ini
	// /e -- terminates (Ends) the active dxwnd session
	// /r:<n> -- run the n-th game in configuration and terminate together with it
	// /a -- auto-hide mode while a game is running
	// /q -- quiet mode, no error/message dialog boxes

	if(bFlag) {
		CString sParam(lpszParam);
		if (sParam.MakeLower() == "t"){
			m_StartToTray=TRUE;
			return;
		}
		if (sParam.MakeLower() == "i"){
			m_InitialState=DXW_IDLE;
			return;
		}
		if (sParam.MakeLower() == "a"){
			gAutoHideMode=TRUE;
			return;
		}
		if (sParam.MakeLower() == "debug"){
			gbDebug = TRUE;
			return;
		}
		if (sParam.MakeLower().Left(5) == "lang="){
			HMODULE ResLib;
			CString Lang;
			Lang = sParam.MakeLower().Right(2);
			ResLib=LoadLibrary("Resources_"+Lang+".dll");
			if(ResLib) {
				AfxSetResourceHandle(ResLib);
				LangSelected=TRUE;
			}
			else MessageBoxEx(NULL, "Missing language \""+Lang+"\"\nUsing default language \"en\"", "Warning", MB_OK, NULL);
			//strcpy(LangString, sParam.MakeLower().Mid(5));
			return;
		}
		if (sParam.Left(2).MakeLower() == "c:"){
			strcpy_s(m_ConfigFileName, sizeof(m_ConfigFileName)-1, sParam.Mid(2,sizeof(m_ConfigFileName)-1));
			return;
		}
		if (sParam.MakeLower() == "e"){
			// Exit (kill) existing DxWnd session
			KillProcByName("DxWnd.exe", TRUE, FALSE);
			exit(0);
		}
		if (sParam.Left(2).MakeLower() == "r:"){
			gTransientMode = TRUE;
			m_StartToTray=TRUE;
			char *p = (char *)sParam.GetString();
			iProgIndex = atoi(&p[2]);
			return;
		}
		if (sParam.MakeLower() == "q"){
			// (Q)uiet mode: no message dialogs on screen
			gQuietMode = TRUE;
		}
	}

	// Call the base class to ensure proper command line processing
	CCommandLineInfo::ParseParam(lpszParam, bFlag, bLast);
}

/////////////////////////////////////////////////////////////////////////////
// CDxwndhostApp Class constructor

CDxwndhostApp::CDxwndhostApp()
{
}

/////////////////////////////////////////////////////////////////////////////
// The only CDxwndhostApp object

CDxwndhostApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CDxwndhostApp Class initialization

BOOL CDxwndhostApp::InitInstance()
{
	char InitPath[MAX_PATH];
	AfxEnableControlContainer();

	// standard initialization

	// DDE file open: Parse command line for standard shell commands and so on.
	// Parse command line for standard shell commands, DDE, file open
	// and user-defined flags. The CCommandLine class has been replaced
	CNewCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);
	
	GetCurrentDirectory(MAX_PATH, InitPath);
	strcat_s(InitPath, sizeof(InitPath), "\\dxwnd.ini");
	
	//CompatibilityMinLevel = GetPrivateProfileInt("window", "compatminlevel", 0, InitPath);

	if(!gbDebug) gbDebug = GetPrivateProfileInt("window", "debug", 0, InitPath); // debug flag set from config file
	if(!gAutoHideMode) gAutoHideMode = GetPrivateProfileInt("window", "autohide", 0, InitPath); // debug flag set from config file
	gWarnOnExit = GetPrivateProfileInt("window", "warnonexit", 0, InitPath); // WarnOnExit flag set from config file

	// Register the document template.

	CSingleDocTemplate* pDocTemplate;
	pDocTemplate = new CSingleDocTemplate(
		gbDebug ? IDR_MAINFRAME_EX : IDR_MAINFRAME,
		RUNTIME_CLASS(CDxwndhostDoc),
		RUNTIME_CLASS(CMainFrame),       // SDI main frame window
		RUNTIME_CLASS(CDxwndhostView));
	AddDocTemplate(pDocTemplate);

	if(!LangSelected){
		LANGID LangId;
		char LangString[20+1];
		GetPrivateProfileString("window", "lang", "", LangString, 20+1, InitPath);
		if(!strcmp(LangString, "default") || !strlen(LangString)){ // if no specification, or lang=default
			// do nothing
		}
		if(!strcmp(LangString, "automatic")){ // lang=automatic
			HMODULE ResLib;
			LangId=GetUserDefaultUILanguage();
			// other codes to be implemented:
			// 409 (2-9): english (default)
			// 411 (2-11): japanese
			switch(LangId & 0x1FF){
				case 0x04: // chinese family
					ResLib=LoadLibrary("Resources_CN.dll");
					if(ResLib) AfxSetResourceHandle(ResLib);
					else MessageBoxEx(NULL, "Missing language \"CN\"\nUsing default language \"en\"", "Warning", MB_OK, NULL);
					break;
				case 0x10: // 410 - italian, 810 - switzerland italian
					ResLib=LoadLibrary("Resources_IT.dll");
					if(ResLib) AfxSetResourceHandle(ResLib);
					else MessageBoxEx(NULL, "Missing language \"IT\"\nUsing default language \"en\"", "Warning", MB_OK, NULL);
					break;
				case 0x19: // 419 - russian
					ResLib=LoadLibrary("Resources_RU.dll");
					if(ResLib) AfxSetResourceHandle(ResLib);
					else MessageBoxEx(NULL, "Missing language \"RU\"\nUsing default language \"en\"", "Warning", MB_OK, NULL);
					break;
				default: 
					//char sBuf[81];
					//sprintf(sBuf, "Got Lang=%x(%x-%x)", LangId, LangId>>9, (LangId & 0x1FF));
					//MessageBox(NULL, sBuf, "LangId", MB_OK);
					break;
			}
		}
		if(strcmp(LangString, "automatic") && strcmp(LangString, "default") && strlen(LangString)) { // lang=something different from both automatic and default
			HMODULE ResLib;
			CString Lang;
			Lang.SetString(LangString);
			ResLib=LoadLibrary("Resources_"+Lang+".dll");
			if(ResLib) AfxSetResourceHandle(ResLib);
			else MessageBoxEx(NULL, "Missing language \""+Lang+"\"\nUsing default language \"en\"", "Warning", MB_OK, NULL);
		}
	}

	// Dispatch commands specified on the command line.
	if (!ProcessShellCommand(cmdInfo))
		return FALSE;
	m_pMainWnd->SetWindowText("DXWnd");
	RECT rect;
	m_pMainWnd->GetWindowRect(&rect);
	rect.right = rect.left + 320;
	rect.bottom = rect.top + 200;
	//m_pMainWnd->MoveWindow(&rect, TRUE);
	m_pMainWnd->ShowWindow(SW_SHOW);
	m_pMainWnd->UpdateWindow();

	BOOL bCheckAdminRights = GetPrivateProfileInt("window", "checkadmin", 0, InitPath); 
	extern BOOL DxSelfElevate(CDxwndhostView *);
    OSVERSIONINFO osver = { sizeof(osver) };
	// self elevation at start if configured for the whole DxWnd session
    if (bCheckAdminRights && (GetVersionEx(&osver)) && (osver.dwMajorVersion >= 6)){
		DxSelfElevate((CDxwndhostView *)NULL);
	}
	return TRUE;
}


/////////////////////////////////////////////////////////////////////////////
// Used in application version information dialog CAboutDlg

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Data Dialog 
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	CString	m_Version;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides.
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV Support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
		// There is no message handler.
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	m_Version = _T("");
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	DDX_Text(pDX, IDC_VERSION, m_Version);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// There is no message handler.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// The application command to run the dialog
void CDxwndhostApp::OnAppAbout()
{
	char tmp[32], ver[32];
	CAboutDlg aboutDlg;
	GetDllVersion(tmp);
	sprintf(ver, "DLL version %s", tmp);
	aboutDlg.m_Version = ver;
	aboutDlg.DoModal();
}

void CDxwndhostApp::OnViewHelp()
{
	HINSTANCE ret;
	char sHelpPath[MAX_PATH];
	GetPrivateProfileString("window", "help", ".\\help\\DxWnd.html", sHelpPath, MAX_PATH, gInitPath);
	ret=ShellExecute(NULL, "open", sHelpPath, NULL, NULL, SW_SHOWNORMAL);
	if((DWORD)ret<=32){
		char *m;
		switch((DWORD)ret){
			case 0:							m="Out of memory resources"; break;
			case ERROR_BAD_FORMAT:			m="Invalid .exe file"; break;
			case SE_ERR_ACCESSDENIED:		m="Access denied"; break;
			case SE_ERR_ASSOCINCOMPLETE:	m="File name association incomplete or invalid"; break;
			case SE_ERR_DDEBUSY:			m="DDE busy"; break;
			case SE_ERR_DDEFAIL:			m="DDE failed"; break;
			case SE_ERR_DDETIMEOUT:			m="DDE timed out"; break;
			case SE_ERR_DLLNOTFOUND:		m="DLL not found"; break;
			case SE_ERR_FNF:				m="File not found"; break;
			case SE_ERR_NOASSOC:			m="No association with file extension"; break;
			case SE_ERR_OOM:				m="Not enough memory"; break;
			case SE_ERR_PNF:				m="Path not found"; break;
			case SE_ERR_SHARE:				m="Sharing violation"; break;
			default:						m="Unknown error"; break;
		}
		MessageBox(0, m, "DxWnd error", MB_ICONEXCLAMATION|MB_OK);
	}
}

/////////////////////////////////////////////////////////////////////////////
// CDxwndhostApp Message Handler

char *GetTSCaption(int shift)
{
	static char *sTSCaption[17]={
		"x16","x12","x8","x6",
		"x4","x3","x2","x1.5",
		"x1",
		":1.5",":2",":3",":4",
		":6",":8",":12",":16"};
	if (shift<-8 || shift>8) return "???";
	return sTSCaption[shift+8];
}
