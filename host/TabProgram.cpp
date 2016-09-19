// TabProgram.cpp : implementation file
//

#include "stdafx.h"
#include "TargetDlg.h"
#include "TabProgram.h"
#include "dxwndhost.h"
#include "specialedit.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static BOOL dirExists(char *path)
{
	DWORD ftyp = GetFileAttributesA(path);
	if (ftyp == INVALID_FILE_ATTRIBUTES) return FALSE;  //something is wrong with your path!
	if (ftyp & FILE_ATTRIBUTE_DIRECTORY) return TRUE;   // this is a directory!
	return false;    // this is not a directory!
}

/////////////////////////////////////////////////////////////////////////////
// CTabProgram dialog

CTabProgram::CTabProgram(CWnd* pParent /*=NULL*/)
	: CDialog(CTabProgram::IDD, pParent)
{
	//{{AFX_DATA_INIT(CTabProgram)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

void CTabProgram::DoDataExchange(CDataExchange* pDX)
{
	CString sPosX, sPosY;
	CDialog::DoDataExchange(pDX);
	CTargetDlg *cTarget = ((CTargetDlg *)(this->GetParent()->GetParent()));
	sPosX.Format("%d", cTarget->m_PosX);
	sPosY.Format("%d", cTarget->m_PosY);
	DDX_Radio(pDX, IDC_COORDINATES, cTarget->m_Coordinates);
	DDX_Control(pDX, IDC_FILE, cTarget->m_File);
	DDX_Control(pDX, IDC_LAUNCH, cTarget->m_Launch);
	DDX_Text(pDX, IDC_FILE, cTarget->m_FilePath);
	DDX_Text(pDX, IDC_LAUNCH, cTarget->m_LaunchPath);
	DDX_Text(pDX, IDC_TITLE, cTarget->m_Title);
	DDX_Check(pDX, IDC_NEEDADMINCAPS, cTarget->m_NeedAdminCaps);
	DDX_Check(pDX, IDC_UNNOTIFY, cTarget->m_UnNotify);
	DDX_Check(pDX, IDC_WINDOWIZE, cTarget->m_Windowize);
	DDX_Check(pDX, IDC_CONFIRMONCLOSE, cTarget->m_ConfirmOnClose);
	DDX_Check(pDX, IDC_TERMINATEONCLOSE, cTarget->m_TerminateOnClose);
	DDX_Check(pDX, IDC_FULLSCREENONLY, cTarget->m_FullScreenOnly);
	DDX_Check(pDX, IDC_SHOWHINTS, cTarget->m_ShowHints);
	DDX_Check(pDX, IDC_SAVELOAD, cTarget->m_SaveLoad);
	DDX_Check(pDX, IDC_KEEPASPECTRATIO, cTarget->m_KeepAspectRatio);
	DDX_Check(pDX, IDC_NOBANNER, cTarget->m_NoBanner);
	DDX_Check(pDX, IDC_HIDEDESKTOP, cTarget->m_HideDesktop);
	//DDX_Text(pDX, IDC_POSX, cTarget->m_PosX);
	//DDX_Text(pDX, IDC_POSY, cTarget->m_PosY);
	DDX_CBIndex(pDX, IDC_MONITOR_ID, cTarget->m_MonitorId);
	DDX_Text(pDX, IDC_POSX, sPosX);
	DDX_Text(pDX, IDC_POSY, sPosY);
	DDX_Text(pDX, IDC_SIZX, cTarget->m_SizX);
	DDX_Text(pDX, IDC_SIZY, cTarget->m_SizY);
	cTarget->m_PosX = atoi(sPosX);
	cTarget->m_PosY = atoi(sPosY);
}

BEGIN_MESSAGE_MAP(CTabProgram, CDialog)
	//{{AFX_MSG_MAP(CTabProgram)
	// NOTE: the ClassWizard will add message map macros here
	ON_BN_CLICKED(IDC_OPEN, OnOpen)
	ON_BN_CLICKED(IDC_OPENLAUNCH, OnOpenLaunch)
	ON_WM_DROPFILES()
	//}}AFX_MSG_MAP
	ON_STN_CLICKED(IDC_REGISTRY, &CTabProgram::OnStnClickedRegistry)
	ON_STN_CLICKED(IDC_NOTES, &CTabProgram::OnStnClickedNotes)
	ON_STN_CLICKED(IDC_XYPICK, &CTabProgram::OnStnClickedXYPick)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTabProgram message handlers

extern void GetFolderFromPath(char *);

static BOOL IsWinXP()
{
	OSVERSIONINFO osvi;
	ZeroMemory(&osvi, sizeof(OSVERSIONINFO));
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	GetVersionEx(&osvi);
	return osvi.dwMajorVersion == 5;
}

void CTabProgram::OnOpen() 
{
    char path[MAX_PATH];
	CTargetDlg *cTarget = ((CTargetDlg *)(this->GetParent()->GetParent()));
	cTarget->m_File.GetWindowText(path, MAX_PATH);
	GetPrivateProfileString("window", "exepath", NULL, path, MAX_PATH, gInitPath);
	if(!dirExists(path)) strcpy(path, "");
	// XP fix: path must end with '\.' 
	if(IsWinXP()) if((strlen(path) > 2) && (path[strlen(path)-1] == '\\')) strcat(path, ".");
	while(TRUE){
		int ret;
		CFileDialog dlg( TRUE, "*.*", path, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
			"Program (*.exe)|*.exe|All Files (*.*)|*.*||",  this);
		ret = dlg.DoModal();
		//char debug[512];
		//sprintf(debug, "ret=%x path=%s", ret, path);
		//MessageBox(debug, "debug", MB_OK);
		if(ret==IDOK){ 
			cTarget->m_File.SetWindowText(dlg.GetPathName());
			if(GetPrivateProfileInt("window", "updatepaths", 1, gInitPath)){
				strcpy(path, dlg.GetPathName());
				GetFolderFromPath(path);
				WritePrivateProfileString("window", "exepath", path, gInitPath);
			}	
			break;
		}
		if(ret==IDCANCEL) break;
		//else
		strcpy(path,"");
	}
}

void CTabProgram::OnOpenLaunch() 
{
    char path[MAX_PATH];
	CTargetDlg *cTarget = ((CTargetDlg *)(this->GetParent()->GetParent()));
	cTarget->m_File.GetWindowText(path, MAX_PATH);
	GetPrivateProfileString("window", "exepath", NULL, path, MAX_PATH, gInitPath);
	if(!dirExists(path)) strcpy(path, "");
	// XP fix: path must end with '\.' 
	if(IsWinXP()) if((strlen(path) > 2) && (path[strlen(path)-1] == '\\')) strcat(path, ".");
	while(TRUE){
		int ret;
		CFileDialog dlg( TRUE, "*.*", path, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
			"Program (*.exe)|*.exe|All Files (*.*)|*.*||",  this);
		ret = dlg.DoModal();
		//char debug[512];
		//sprintf(debug, "ret=%x path=%s", ret, path);
		//MessageBox(debug, "debug", MB_OK);
		if(ret==IDOK){ 
			cTarget->m_Launch.SetWindowText(dlg.GetPathName());
			if(GetPrivateProfileInt("window", "updatepaths", 1, gInitPath)){
				strcpy(path, dlg.GetPathName());
				GetFolderFromPath(path);
				WritePrivateProfileString("window", "exepath", path, gInitPath);
			}
			break;
		}
		if(ret==IDCANCEL) break;
		//else
		strcpy(path,"");
	}
}

void CTabProgram::OnDropFiles(HDROP dropInfo)
{
	CString sFile;
	DWORD nBuffer = 0;
	// Get number of files
	UINT nFilesDropped = DragQueryFile(dropInfo, 0xFFFFFFFF, NULL, 0);
	if(nFilesDropped > 0){
		nBuffer = DragQueryFile(dropInfo, 0, NULL, 0);
		DragQueryFile(dropInfo, 0, sFile.GetBuffer(nBuffer+1), nBuffer+1);
		//CTargetDlg *pDlg = (CTargetDlg *)this->GetParent()->GetParent();
		//pDlg->m_FilePath = sFile;
		CDragEdit *pEditFile = (CDragEdit *)this->GetDlgItem(IDC_FILE);
		pEditFile->SetWindowTextA(sFile.GetBuffer());
		//MessageBox(sFile.GetBuffer(), "debug", 0);
		sFile.ReleaseBuffer();
	}
	DragFinish(dropInfo);
}

BOOL CTabProgram::OnInitDialog()
{
	HINSTANCE Hinst;
	HICON Icon, PrevIcon;
	CStatic *IconBox;
	IFormat *m_pRelIntegerFormat = new(RelIntegerFormat);

	DragAcceptFiles();
	CDragEdit *pEditFile;
	pEditFile = (CDragEdit *)this->GetDlgItem(IDC_FILE);
	pEditFile->DragAcceptFiles();
	pEditFile = (CDragEdit *)this->GetDlgItem(IDC_LAUNCH);
	pEditFile->DragAcceptFiles();
	//m_File.DragAcceptFiles();
	//m_Launch.DragAcceptFiles();
	CDialog::OnInitDialog();
	CTargetDlg *cTarget = ((CTargetDlg *)(this->GetParent()->GetParent()));
	Hinst = ::LoadLibrary(cTarget->m_FilePath);
	if(Hinst){
		Icon = ::ExtractIcon(Hinst, cTarget->m_FilePath, 0);
		IconBox=(CStatic *)this->GetDlgItem(IDC_STATIC_ICON);
		PrevIcon = IconBox->SetIcon(Icon);
		if (IconBox->GetIcon() == NULL)
			IconBox->SetIcon(::LoadIcon(NULL, IDI_ERROR));  
		::FreeLibrary(Hinst);
		if(PrevIcon) ::DestroyIcon(PrevIcon);
	}

	IconBox=(CStatic *)this->GetDlgItem(IDC_NOTES);
	if(cTarget->m_Notes.IsEmpty()) IconBox->SetBitmap(NULL);

	IconBox=(CStatic *)this->GetDlgItem(IDC_REGISTRY);
	if(cTarget->m_Registry.IsEmpty()) IconBox->SetBitmap(NULL);

	CComboBox *cScreens = (CComboBox *)this->GetDlgItem(IDC_MONITOR_ID);
	cScreens->ResetContent();
	int iMonitorCount = ::GetSystemMetrics(SM_CMONITORS);
	cScreens->AddString("def.");
	for (int i=1; i<=iMonitorCount; i++){
		char sMonitorId[16];
		sprintf(sMonitorId, "%d", i);
		cScreens->AddString(sMonitorId);
	}
	cScreens->SetCurSel(cTarget->m_MonitorId);

	m_EditPosX.SubclassDlgItem(IDC_POSX, this);
	m_EditPosY.SubclassDlgItem(IDC_POSY, this);
	m_EditPosX.SetFormatter(m_pRelIntegerFormat);
	m_EditPosY.SetFormatter(m_pRelIntegerFormat);
	return TRUE;
}


void CTabProgram::OnStnClickedRegistry()
{
	// TODO: Add your control notification handler code here
	CDXTabCtrl *cTab = (CDXTabCtrl *)this->GetParent();
	cTab->SwitchToTab(11);
}

void CTabProgram::OnStnClickedNotes()
{
	// TODO: Add your control notification handler code here
	CDXTabCtrl *cTab = (CDXTabCtrl *)this->GetParent();
	cTab->SwitchToTab(12);
}

void CTabProgram::OnStnClickedXYPick()
{
	//return;
	HCURSOR hPrevCursor;
	HCURSOR hViewFinder;
	BOOL Looping = TRUE;
	BOOL Picked = FALSE;
	BOOL bMoved = FALSE;
	HWND TargethWnd;
	RECT TargetRect = {0, 0, 0, 0};
	POINT UpLeft = {0, 0};
	char sMessage[81];
	hViewFinder=LoadCursor(NULL, IDC_CROSS);
	//hViewFinder=LoadCursor(NULL, IDC_SIZE);
	hPrevCursor=SetCursor(hViewFinder);
	ShowCursor(TRUE);
	this->SetCapture();
	//MessageBox("Pick window coordinates", "DxWnd", 0);
	while(TRUE){
		POINT pt;
		MSG Msg;
		GetMessage(&Msg, NULL, 0, 0);
		//if(GetAsyncKeyState(VK_LBUTTON) & 0x8000){
		if(Msg.message == WM_LBUTTONUP){
			if(bMoved){
				GetCursorPos(&pt);
				TargethWnd=::WindowFromPoint(pt);
				::GetClientRect(TargethWnd, &TargetRect);
				::ClientToScreen(TargethWnd, &UpLeft);
				OffsetRect(&TargetRect, UpLeft.x, UpLeft.y);
				Picked = TRUE;
			}
			ReleaseCapture();
			break;
		}
		if(Msg.message == WM_LBUTTONDOWN) break;
		if(Msg.message == WM_MOUSEMOVE) bMoved = TRUE;
	}

	if(!Picked) return;
	sprintf(sMessage, "Pick rect=(%d,%d)-(%d,%d) ?", 
		TargetRect.left, TargetRect.top, TargetRect.right, TargetRect.bottom);
	if(MessageBox(sMessage, "DxWnd", MB_OKCANCEL)){
		this->SetDlgItemInt(IDC_POSX, TargetRect.left, TRUE);
		this->SetDlgItemInt(IDC_POSY, TargetRect.top, TRUE);
		this->SetDlgItemInt(IDC_SIZX, TargetRect.right - TargetRect.left, TRUE);
		this->SetDlgItemInt(IDC_SIZY, TargetRect.bottom - TargetRect.top, TRUE);
	}
}
