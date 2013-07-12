// TabOpenGL.cpp : implementation file
//

#include "stdafx.h"
#include "TargetDlg.h"
#include "TabWindow.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTabWindow dialog


CTabWindow::CTabWindow(CWnd* pParent /*=NULL*/)
	: CDialog(CTabWindow::IDD, pParent)
{
	//{{AFX_DATA_INIT(CTabWindow)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CTabWindow::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	CTargetDlg *cTarget = ((CTargetDlg *)(this->GetParent()->GetParent()));

	// window management
	DDX_Check(pDX, IDC_FIXWINFRAME, cTarget->m_FixWinFrame);
	DDX_Check(pDX, IDC_PREVENTMAXIMIZE, cTarget->m_PreventMaximize);
	DDX_Check(pDX, IDC_LOCKWINPOS, cTarget->m_LockWinPos);
	DDX_Check(pDX, IDC_LOCKWINSTYLE, cTarget->m_LockWinStyle);
	DDX_Check(pDX, IDC_FIXPARENTWIN, cTarget->m_FixParentWin);
	DDX_Check(pDX, IDC_MODALSTYLE, cTarget->m_ModalStyle);
	DDX_Check(pDX, IDC_FORCEWINRESIZE, cTarget->m_ForceWinResize);
	DDX_Check(pDX, IDC_HIDEMULTIMONITOR, cTarget->m_HideMultiMonitor);
	DDX_Check(pDX, IDC_WALLPAPERMODE, cTarget->m_WallpaperMode);
	DDX_Check(pDX, IDC_RECOVERSCREENMODE, cTarget->m_RecoverScreenMode);
	DDX_Check(pDX, IDC_REFRESHONRESIZE, cTarget->m_RefreshOnResize);
	DDX_Check(pDX, IDC_FIXD3DFRAME, cTarget->m_FixD3DFrame);
	DDX_Check(pDX, IDC_NOWINDOWMOVE, cTarget->m_NoWindowMove);
	DDX_Check(pDX, IDC_SUPPRESSCHILD, cTarget->m_SuppressChild);

	// color management
	DDX_Check(pDX, IDC_INIT8BPP, cTarget->m_Init8BPP);
	DDX_Check(pDX, IDC_INIT16BPP, cTarget->m_Init16BPP);
	DDX_Check(pDX, IDC_DISABLEGAMMARAMP, cTarget->m_DisableGammaRamp);
	DDX_Check(pDX, IDC_FORCE16BPP, cTarget->m_Force16BPP);
	DDX_Check(pDX, IDC_WIREFRAME, cTarget->m_WireFrame);
	DDX_Check(pDX, IDC_BLACKWHITE, cTarget->m_BlackWhite);
	DDX_Check(pDX, IDC_USERGB565, cTarget->m_UseRGB565);
	DDX_Check(pDX, IDC_LOCKSYSCOLORS, cTarget->m_LockSysColors);
}

BOOL CTabWindow::OnInitDialog()
{

	AfxEnableControlContainer();
	CDialog::OnInitDialog();
	return TRUE;
}


BEGIN_MESSAGE_MAP(CTabWindow, CDialog)
	//{{AFX_MSG_MAP(CTabWindow)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTabWindow message handlers
