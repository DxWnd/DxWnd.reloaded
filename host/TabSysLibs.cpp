// TabGDI.cpp : implementation file
//

#include "stdafx.h"
#include "TargetDlg.h"
#include "TabSysLibs.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTabLogs dialog

CTabSysLibs::CTabSysLibs(CWnd* pParent /*=NULL*/)
	: CDialog(CTabSysLibs::IDD, pParent)
{
	//{{AFX_DATA_INIT(CTabSysLibs)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

void CTabSysLibs::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	CTargetDlg *cTarget = ((CTargetDlg *)(this->GetParent()->GetParent()));

	// GDI
	DDX_Radio(pDX, IDC_GDINONE, cTarget->m_DCEmulationMode);
	DDX_Check(pDX, IDC_FIXTEXTOUT, cTarget->m_FixTextOut);

	// OpenGL
	DDX_Check(pDX, IDC_FORCEHOOKOPENGL, cTarget->m_ForceHookOpenGL);
	DDX_Text(pDX, IDC_OPENGLLIB, cTarget->m_OpenGLLib);

	// Glide
	DDX_Check(pDX, IDC_HOOKGLIDE, cTarget->m_HookGlide);

	// MCI
	DDX_Check(pDX, IDC_REMAPMCI, cTarget->m_RemapMCI);

	// Kernel32
	DDX_Radio(pDX, IDC_SONDEFAULT, cTarget->m_SonProcessMode);
}

BEGIN_MESSAGE_MAP(CTabSysLibs, CDialog)
	//{{AFX_MSG_MAP(CTabLogs)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTabLogs message handlers
