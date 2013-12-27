// TabDirectX.cpp : implementation file
//

#include "stdafx.h"
#include "TargetDlg.h"
#include "TabTiming.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTabTiming dialog


CTabTiming::CTabTiming(CWnd* pParent /*=NULL*/)
	: CDialog(CTabTiming::IDD, pParent)
{
	//{{AFX_DATA_INIT(CTabTiming)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CTabTiming::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	CTargetDlg *cTarget = ((CTargetDlg *)(this->GetParent()->GetParent()));
	DDX_Check(pDX, IDC_LIMITFPS, cTarget->m_LimitFPS);
	DDX_Check(pDX, IDC_SKIPFPS, cTarget->m_SkipFPS);
	DDX_Check(pDX, IDC_SHOWFPS, cTarget->m_ShowFPS);
	DDX_Check(pDX, IDC_SHOWFPSOVERLAY, cTarget->m_ShowFPSOverlay);
	DDX_Check(pDX, IDC_SHOWTIMESTRETCH, cTarget->m_ShowTimeStretch);
	DDX_Check(pDX, IDC_TIMESTRETCH, cTarget->m_TimeStretch);
	DDX_Check(pDX, IDC_INTERCEPTRDTSC, cTarget->m_InterceptRDTSC);
	DDX_Text(pDX, IDC_MAXFPS, cTarget->m_MaxFPS);
	DDX_LBIndex(pDX, IDC_LISTTS, cTarget->m_InitTS);
	//{{AFX_DATA_MAP(CTabTiming)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}

BOOL CTabTiming::OnInitDialog()
{

	AfxEnableControlContainer();

	CListBox *List;
	CDialog::OnInitDialog();
	int i;
	extern char *GetTSCaption(int);
	List=(CListBox *)this->GetDlgItem(IDC_LISTTS);
	List->ResetContent();
	for(i=-8; i<=8; i++) List->AddString(GetTSCaption(i));
	CTargetDlg *cTarget = ((CTargetDlg *)(this->GetParent()->GetParent()));
	List->SetCurSel(cTarget->m_InitTS);

	return TRUE;
}


BEGIN_MESSAGE_MAP(CTabTiming, CDialog)
	//{{AFX_MSG_MAP(CTabTiming)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTabTiming message handlers
