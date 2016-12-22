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
	DDX_Text(pDX, IDC_STARTFOLDER, cTarget->m_StartFolder);
	DDX_Check(pDX, IDC_HOOKENABLED, cTarget->m_HookEnabled);
	DDX_Radio(pDX, IDC_INJECT_WINDOWSHOOK, cTarget->m_InjectionMode);
	DDX_Check(pDX, IDC_HOTPATCH, cTarget->m_HotPatch);
	DDX_Check(pDX, IDC_HOOKDLLS, cTarget->m_HookDLLs);
	DDX_Check(pDX, IDC_ANSIWIDE, cTarget->m_AnsiWide);
	DDX_Check(pDX, IDC_HOOKNORUN, cTarget->m_HookNoRun);
	DDX_Check(pDX, IDC_COPYNOSHIMS, cTarget->m_CopyNoShims);
	DDX_Check(pDX, IDC_HOOKNOUPDATE, cTarget->m_HookNoUpdate);
	DDX_Check(pDX, IDC_SEQUENCEDIAT, cTarget->m_SequencedIAT);
	DDX_Check(pDX, IDC_HOOKCHILDWIN, cTarget->m_HookChildWin);

	// Kernel32
	DDX_Radio(pDX, IDC_SONDEFAULT, cTarget->m_SonProcessMode);

	// additional hooks
	DDX_Check(pDX, IDC_HOOKDIRECTSOUND, cTarget->m_HookDirectSound);
	DDX_Check(pDX, IDC_HOOKWING32, cTarget->m_HookWinG32);
	DDX_Check(pDX, IDC_HOOKGLIDE, cTarget->m_HookGlide);
}

BEGIN_MESSAGE_MAP(CTabHook, CDialog)
	//{{AFX_MSG_MAP(CTabHook)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTabHook message handlers
