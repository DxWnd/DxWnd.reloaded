// TabProgram.cpp : implementation file
//

#include "stdafx.h"
#include "TargetDlg.h"
#include "TabProgram.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

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
	CDialog::DoDataExchange(pDX);
	CTargetDlg *cTarget = ((CTargetDlg *)(this->GetParent()->GetParent()));
	DDX_Radio(pDX, IDC_COORDINATES, cTarget->m_Coordinates);
	DDX_Control(pDX, IDC_FILE, cTarget->m_File);
	DDX_Control(pDX, IDC_LAUNCH, cTarget->m_Launch);
	DDX_Text(pDX, IDC_FILE, cTarget->m_FilePath);
	DDX_Text(pDX, IDC_LAUNCH, cTarget->m_LaunchPath);
	DDX_Text(pDX, IDC_TITLE, cTarget->m_Title);
	DDX_Text(pDX, IDC_MODULE, cTarget->m_Module);
	DDX_Check(pDX, IDC_UNNOTIFY, cTarget->m_UnNotify);
	DDX_Check(pDX, IDC_WINDOWIZE, cTarget->m_Windowize);
	DDX_Check(pDX, IDC_HOTPATCH, cTarget->m_HotPatch);
	DDX_Check(pDX, IDC_HOOKDLLS, cTarget->m_HookDLLs);
	DDX_Check(pDX, IDC_CONFIRMONCLOSE, cTarget->m_ConfirmOnClose);
	DDX_Check(pDX, IDC_TERMINATEONCLOSE, cTarget->m_TerminateOnClose);
	DDX_Check(pDX, IDC_FULLSCREENONLY, cTarget->m_FullScreenOnly);
	DDX_Check(pDX, IDC_HOOKCHILDWIN, cTarget->m_HookChildWin);
	DDX_Check(pDX, IDC_HOOKENABLED, cTarget->m_HookEnabled);
	DDX_Check(pDX, IDC_STARTDEBUG, cTarget->m_StartDebug);
	DDX_Check(pDX, IDC_SAVELOAD, cTarget->m_SaveLoad);
	DDX_Check(pDX, IDC_KEEPASPECTRATIO, cTarget->m_KeepAspectRatio);
	DDX_Check(pDX, IDC_NOBANNER, cTarget->m_NoBanner);
	DDX_Text(pDX, IDC_POSX, cTarget->m_PosX);
	DDX_Text(pDX, IDC_POSY, cTarget->m_PosY);
	DDX_Text(pDX, IDC_SIZX, cTarget->m_SizX);
	DDX_Text(pDX, IDC_SIZY, cTarget->m_SizY);
}

BEGIN_MESSAGE_MAP(CTabProgram, CDialog)
	//{{AFX_MSG_MAP(CTabProgram)
		// NOTE: the ClassWizard will add message map macros here
	ON_BN_CLICKED(IDC_OPEN, OnOpen)
	ON_BN_CLICKED(IDC_OPENLAUNCH, OnOpenLaunch)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTabProgram message handlers

extern void GetFolderFromPath(char *);

void CTabProgram::OnOpen() 
{
	// TODO: Please add your control notification handler code here
    char path[MAX_PATH];
	extern char *gInitFilePath;
	CTargetDlg *cTarget = ((CTargetDlg *)(this->GetParent()->GetParent()));
	cTarget->m_File.GetWindowText(path, MAX_PATH);
	GetPrivateProfileString("window", "exepath", NULL, path, MAX_PATH, gInitFilePath);
	CFileDialog dlg( TRUE, "*.*", path, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
        "Program (*.exe)|*.exe|All Files (*.*)|*.*||",  this);
	if( dlg.DoModal() == IDOK) {
		cTarget->m_File.SetWindowText(dlg.GetPathName());
		if(GetPrivateProfileInt("window", "updatepaths", 1, gInitFilePath)){
			strcpy(path, dlg.GetPathName());
			GetFolderFromPath(path);
			WritePrivateProfileString("window", "exepath", path, gInitFilePath);
		}
	}
}

void CTabProgram::OnOpenLaunch() 
{
	// TODO: Please add your control notification handler code here
    char path[MAX_PATH];
	extern char *gInitFilePath;
	CTargetDlg *cTarget = ((CTargetDlg *)(this->GetParent()->GetParent()));
	cTarget->m_File.GetWindowText(path, MAX_PATH);
	GetPrivateProfileString("window", "exepath", NULL, path, MAX_PATH, gInitFilePath);
	CFileDialog dlg( TRUE, "*.*", path, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
        "Program (*.exe)|*.exe|All Files (*.*)|*.*||",  this);
	if( dlg.DoModal() == IDOK) {
		cTarget->m_Launch.SetWindowText(dlg.GetPathName());
		if(GetPrivateProfileInt("window", "updatepaths", 1, gInitFilePath)){
			strcpy(path, dlg.GetPathName());
			GetFolderFromPath(path);
			WritePrivateProfileString("window", "exepath", path, gInitFilePath);
		}
	}
}

BOOL CTabProgram::OnInitDialog()
{
	HINSTANCE Hinst;
	HICON Icon, PrevIcon;
	CStatic *IconBox;

	CDialog::OnInitDialog();
	CTargetDlg *cTarget = ((CTargetDlg *)(this->GetParent()->GetParent()));
	Hinst = ::LoadLibrary(cTarget->m_FilePath);
	if(!Hinst) return TRUE;
	Icon = ::ExtractIcon(Hinst, cTarget->m_FilePath, 0);
	IconBox=(CStatic *)this->GetDlgItem(IDC_STATIC_ICON);
	PrevIcon = IconBox->SetIcon(Icon);
	if (IconBox->GetIcon() == NULL)
		IconBox->SetIcon(::LoadIcon(NULL, IDI_ERROR));  
	::FreeLibrary(Hinst);
	if(PrevIcon) ::DestroyIcon(PrevIcon);
	return TRUE;
}
