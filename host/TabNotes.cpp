// TabNotes.cpp : implementation file
//

#include "stdafx.h"
#include "TargetDlg.h"
#include "TabNotes.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTabNotes dialog

CTabNotes::CTabNotes(CWnd* pParent /*=NULL*/)
//	: CTargetDlg(pParent)
	: CDialog(CTabNotes::IDD, pParent)
{
	//{{AFX_DATA_INIT(CTabNotes)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

void CTabNotes::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	CTargetDlg *cTarget = ((CTargetDlg *)(this->GetParent()->GetParent()));
	DDX_Text(pDX, IDC_NOTES, cTarget->m_Notes);
}

BEGIN_MESSAGE_MAP(CTabNotes, CDialog)
	//{{AFX_MSG_MAP(CTabNotes)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTabNotes message handlers


//BOOL CTabNotes::OnInitDialog()
//{
//	AfxEnableControlContainer();
//	CListBox *List;
//	CTargetDlg *cTarget = ((CTargetDlg *)(this->GetParent()->GetParent()));
//	int i;
//	List=(CListBox *)this->GetDlgItem(IDC_LISTFAKE);
//	List->ResetContent();
//	for(i=0; i<9; i++) List->AddString(WinVersions[i].sName);
//	List->SetCurSel(cTarget->m_FakeVersion);
//	CDialog::OnInitDialog();
//	return TRUE;
//}