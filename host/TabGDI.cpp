// TabGDI.cpp : implementation file
//

#include "stdafx.h"
#include "TargetDlg.h"
#include "TabGDI.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTabLogs dialog

CTabGDI::CTabGDI(CWnd* pParent /*=NULL*/)
	: CDialog(CTabGDI::IDD, pParent)
{
	//{{AFX_DATA_INIT(CTabGDI)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

void CTabGDI::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	CTargetDlg *cTarget = ((CTargetDlg *)(this->GetParent()->GetParent()));
	DDX_Radio(pDX, IDC_GDINONE, cTarget->m_DCEmulationMode);
	DDX_Check(pDX, IDC_FIXTEXTOUT, cTarget->m_FixTextOut);
}

BEGIN_MESSAGE_MAP(CTabGDI, CDialog)
	//{{AFX_MSG_MAP(CTabLogs)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTabLogs message handlers
