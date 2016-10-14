// TabRegistry.cpp : implementation file
//

#include "stdafx.h"
#include "TargetDlg.h"
#include "TabRegistry.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTabRegistry dialog

CTabRegistry::CTabRegistry(CWnd* pParent /*=NULL*/)
//	: CTargetDlg(pParent)
	: CDialog(CTabRegistry::IDD, pParent)
{
	//{{AFX_DATA_INIT(CTabRegistry)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

void CTabRegistry::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	CTargetDlg *cTarget = ((CTargetDlg *)(this->GetParent()->GetParent()));
	DDX_Text(pDX, IDC_REGISTRY, cTarget->m_Registry);
}

BEGIN_MESSAGE_MAP(CTabRegistry, CDialog)
	//{{AFX_MSG_MAP(CTabRegistry)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTabRegistry message handlers
