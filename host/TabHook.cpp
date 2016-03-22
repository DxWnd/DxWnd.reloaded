// TabHook.cpp : implementation file
//

#include "stdafx.h"
#include "TargetDlg.h"
#include "TabHook.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTabHook dialog

CTabHook::CTabHook(CWnd* pParent /*=NULL*/)
//	: CTargetDlg(pParent)
	: CDialog(CTabHook::IDD, pParent)
{
	//{{AFX_DATA_INIT(CTabHook)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

void CTabHook::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	CTargetDlg *cTarget = ((CTargetDlg *)(this->GetParent()->GetParent()));
	DDX_Text(pDX, IDC_MODULE, cTarget->m_Module);
	DDX_Check(pDX, IDC_HOOKENABLED, cTarget->m_HookEnabled);
	DDX_Check(pDX, IDC_STARTDEBUG, cTarget->m_StartDebug);
	DDX_Check(pDX, IDC_HOTPATCH, cTarget->m_HotPatch);
	DDX_Check(pDX, IDC_HOOKDLLS, cTarget->m_HookDLLs);
	DDX_Check(pDX, IDC_HOOKCHILDWIN, cTarget->m_HookChildWin);

	// Kernel32
	DDX_Radio(pDX, IDC_SONDEFAULT, cTarget->m_SonProcessMode);
}

BEGIN_MESSAGE_MAP(CTabHook, CDialog)
	//{{AFX_MSG_MAP(CTabHook)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTabHook message handlers
