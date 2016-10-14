// TabOpenGL.cpp : implementation file
//

#include "stdafx.h"
#include "TargetDlg.h"
#include "TabOpenGL.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTabOpenGL dialog

CTabOpenGL::CTabOpenGL(CWnd* pParent /*=NULL*/)
	: CDialog(CTabOpenGL::IDD, pParent)
{
	//{{AFX_DATA_INIT(CTabOpenGL)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

void CTabOpenGL::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	CTargetDlg *cTarget = ((CTargetDlg *)(this->GetParent()->GetParent()));
	DDX_Check(pDX, IDC_HOOKOPENGL, cTarget->m_HookOpenGL);
	DDX_Check(pDX, IDC_FORCEHOOKOPENGL, cTarget->m_ForceHookOpenGL);
	DDX_Text(pDX, IDC_OPENGLLIB, cTarget->m_OpenGLLib);
}

BOOL CTabOpenGL::OnInitDialog()
{
	AfxEnableControlContainer();
	CDialog::OnInitDialog();
	return TRUE;
}


BEGIN_MESSAGE_MAP(CTabOpenGL, CDialog)
	//{{AFX_MSG_MAP(CTabOpenGL)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTabOpenGL message handlers
