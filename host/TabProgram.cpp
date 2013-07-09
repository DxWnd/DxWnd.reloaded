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
	DDX_Text(pDX, IDC_FILE, cTarget->m_FilePath);
	DDX_Text(pDX, IDC_MODULE, cTarget->m_Module);
	DDX_Text(pDX, IDC_TITLE, cTarget->m_Title);
	DDX_Check(pDX, IDC_UNNOTIFY, cTarget->m_UnNotify);
	DDX_Check(pDX, IDC_WINDOWIZE, cTarget->m_Windowize);
	DDX_Check(pDX, IDC_HOOKDLLS, cTarget->m_HookDLLs);
	DDX_Check(pDX, IDC_EMULATEREGISTRY, cTarget->m_EmulateRegistry);
	DDX_Check(pDX, IDC_FULLSCREENONLY, cTarget->m_FullScreenOnly);
	DDX_Check(pDX, IDC_HOOKCHILDWIN, cTarget->m_HookChildWin);
	DDX_Check(pDX, IDC_HOOKENABLED, cTarget->m_HookEnabled);
	DDX_Check(pDX, IDC_NOBANNER, cTarget->m_NoBanner);
	DDX_Check(pDX, IDC_STARTDEBUG, cTarget->m_StartDebug);
	DDX_Check(pDX, IDC_CLIENTREMAPPING, cTarget->m_ClientRemapping);
	DDX_Check(pDX, IDC_SAVELOAD, cTarget->m_SaveLoad);
	DDX_Check(pDX, IDC_HANDLEALTF4, cTarget->m_HandleAltF4);
	DDX_Text(pDX, IDC_POSX, cTarget->m_PosX);
	DDX_Text(pDX, IDC_POSY, cTarget->m_PosY);
	DDX_Text(pDX, IDC_SIZX, cTarget->m_SizX);
	DDX_Text(pDX, IDC_SIZY, cTarget->m_SizY);
}

BEGIN_MESSAGE_MAP(CTabProgram, CDialog)
	//{{AFX_MSG_MAP(CTabProgram)
		// NOTE: the ClassWizard will add message map macros here
	ON_BN_CLICKED(IDC_OPEN, OnOpen)
	//}}AFX_MSG_MAP
	//ON_BN_CLICKED(IDC_COORDINATES, &CTabProgram::OnBnClickedCoordinates)
	//ON_BN_CLICKED(IDC_DESKTOPWORKAREA, &CTabProgram::OnBnClickedDesktopworkarea)
	//ON_BN_CLICKED(IDC_DESKTOPCENTER, &CTabProgram::OnBnClickedDesktopcenter)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTabProgram message handlers

void CTabProgram::OnOpen() 
{
	// TODO: Please add your control notification handler code here
    char path[MAX_PATH];
	CTargetDlg *cTarget = ((CTargetDlg *)(this->GetParent()->GetParent()));
	cTarget->m_File.GetWindowText(path, MAX_PATH);
	CFileDialog dlg( TRUE, "*.*", path, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
        "Program (*.exe)|*.exe|All Files (*.*)|*.*||",  this);
    if( dlg.DoModal() == IDOK) cTarget->m_File.SetWindowText(dlg.GetPathName());
}
void CTabProgram::OnBnClickedCoordinates()
{
	// TODO: Add your control notification handler code here
	//CWnd *cTarget = ((CTargetDlg *)(this->GetParent());
	//(CButton*)(cTarget->GetDlgItem(IDC_POSX))
}

void CTabProgram::OnBnClickedDesktopworkarea()
{
	// TODO: Add your control notification handler code here
}

void CTabProgram::OnBnClickedDesktopcenter()
{
	// TODO: Add your control notification handler code here
}
