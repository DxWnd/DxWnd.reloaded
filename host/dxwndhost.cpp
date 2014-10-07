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

/////////////////////////////////////////////////////////////////////////////
// CDxwndhostApp

BEGIN_MESSAGE_MAP(CDxwndhostApp, CWinApp)
	//{{AFX_MSG_MAP(CDxwndhostApp)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
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
	AfxEnableControlContainer();

	// standard initialization

//#ifdef _AFXDLL
//	Enable3dControls();		// if MFC is in shared DLL use this call.
//#else
//	Enable3dControlsStatic();	// if MFC is statically linked use this call.
//#endif

	// Register the document template.

	CSingleDocTemplate* pDocTemplate;
	pDocTemplate = new CSingleDocTemplate(
		IDR_MAINFRAME,
		RUNTIME_CLASS(CDxwndhostDoc),
		RUNTIME_CLASS(CMainFrame),       // SDI main frame window
		RUNTIME_CLASS(CDxwndhostView));
	AddDocTemplate(pDocTemplate);

	
	// DDE file open: Parse command line for standard shell commands and so on.
	// Parse command line for standard shell commands, DDE, file open
	// and user-defined flags. The CCommandLine class has been replaced
	CNewCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);

	if(!LangSelected){
		LANGID LangId;
		char LangString[20+1];
		char InitPath[MAX_PATH];
		GetCurrentDirectory(MAX_PATH, InitPath);
		strcat_s(InitPath, sizeof(InitPath), "\\dxwnd.ini");
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

	extern BOOL IsProcessElevated();
	extern BOOL IsUserInAdminGroup();
    OSVERSIONINFO osver = { sizeof(osver) };
    if (GetVersionEx(&osver) && osver.dwMajorVersion >= 6)
    {
		BOOL const fInAdminGroup = IsUserInAdminGroup();
		if(!fInAdminGroup) return TRUE;

        // Get and display the process elevation information.
        BOOL const fIsElevated = IsProcessElevated();
		BOOL MustRestart;
		if(fIsElevated) return TRUE;
		MustRestart=MessageBoxLang(DXW_STRING_ADMINCAP, DXW_STRING_WARNING, MB_OKCANCEL | MB_ICONQUESTION);
		if(MustRestart==IDOK){
			extern HANDLE GlobalLocker;
			CloseHandle(GlobalLocker);
			char szPath[MAX_PATH];
			if (GetModuleFileName(NULL, szPath, ARRAYSIZE(szPath)))
			{
				// Launch itself as administrator.
				SHELLEXECUTEINFO sei = { sizeof(sei) };
				CString args;
				sei.lpVerb = "runas";
				sei.lpFile = szPath;
				//sei.hwnd = (HWND)this->GetMainWnd();
				sei.hwnd = (HWND)NULL; // set to NULL to force the confirmation dialog on top of everything...
				sei.nShow = SW_NORMAL;
				args = "";
				for(int i=1; i<=__argc; i++) {
					args += (LPCSTR)(__argv[i]);
					args += " ";
				}
				sei.lpParameters = args;
				if (!ShellExecuteEx(&sei)){
					DWORD dwError = GetLastError();
					if (dwError == ERROR_CANCELLED){
						// The user refused the elevation.
						// Do nothing ...
					}
				}
				else{
					exit(0); // Quit itself
				}
			}
		}
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
