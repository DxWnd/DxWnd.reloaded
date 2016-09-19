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
	//DDX_Check(pDX, IDC_WALLPAPERMODE, cTarget->m_WallpaperMode);
	DDX_Check(pDX, IDC_RECOVERSCREENMODE, cTarget->m_RecoverScreenMode);
	DDX_Check(pDX, IDC_REFRESHONRESIZE, cTarget->m_RefreshOnResize);
	DDX_Check(pDX, IDC_FIXD3DFRAME, cTarget->m_FixD3DFrame);
	DDX_Check(pDX, IDC_NOWINDOWMOVE, cTarget->m_NoWindowMove);
	DDX_Check(pDX, IDC_HIDETASKBAR, cTarget->m_HideTaskbar);
	DDX_Check(pDX, IDC_UNLOCKZORDER, cTarget->m_UnlockZOrder);
	DDX_Check(pDX, IDC_NODESTROYWINDOW, cTarget->m_NoDestroyWindow);
	DDX_Check(pDX, IDC_ACTIVATEAPP, cTarget->m_ActivateApp);

	// color management
	DDX_Radio(pDX, IDC_COLORCURRENT, cTarget->m_InitColorDepth);
	DDX_Check(pDX, IDC_DISABLEGAMMARAMP, cTarget->m_DisableGammaRamp);
	DDX_Check(pDX, IDC_FORCE16BPP, cTarget->m_Force16BPP);
	DDX_Check(pDX, IDC_BLACKWHITE, cTarget->m_BlackWhite);
	DDX_Check(pDX, IDC_USERGB565, cTarget->m_UseRGB565);
	DDX_Check(pDX, IDC_LOCKSYSCOLORS, cTarget->m_LockSysColors);
	DDX_Check(pDX, IDC_LOCKRESERVEDPALETTE, cTarget->m_LockReservedPalette);

	// screen resolution management
	DDX_Check(pDX, IDC_LIMITSCREENRES, cTarget->m_LimitScreenRes);
	DDX_LBIndex(pDX, IDC_LISTRES, cTarget->m_MaxScreenRes);
	DDX_Radio(pDX, IDC_SUPPORTSVGA, cTarget->m_ResTypes);
}

static char *Resolutions[]={
	"unlimited",
	"320x200",
	"400x300",
	"640x480",
	"800x600",
	"1024x768",
	"1280x960",
	"1280x1024",
	"" // terminator
};

char UnlimitedString[20+1];

BOOL CTabWindow::OnInitDialog()
{

	AfxEnableControlContainer();

	CListBox *List;
	CTargetDlg *cTarget = ((CTargetDlg *)(this->GetParent()->GetParent()));
	int i;
	List=(CListBox *)this->GetDlgItem(IDC_LISTRES);
	List->ResetContent();
	if(LoadString(AfxGetResourceHandle(), DXW_STRING_UNLIMITED, UnlimitedString, sizeof(UnlimitedString))){
		Resolutions[0]=UnlimitedString;
	}
	for(i=0; strlen(Resolutions[i]); i++) List->AddString(Resolutions[i]);
	List->SetCurSel(cTarget->m_MaxScreenRes);

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


