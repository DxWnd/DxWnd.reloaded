// TabDirectX.cpp : implementation file
//

#include "stdafx.h"
#include "TargetDlg.h"
#include "TabCompat.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTabCompat dialog

CTabCompat::CTabCompat(CWnd* pParent /*=NULL*/)
//	: CTargetDlg(pParent)
	: CDialog(CTabCompat::IDD, pParent)
{
	//{{AFX_DATA_INIT(CTabCompat)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

void CTabCompat::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	CTargetDlg *cTarget = ((CTargetDlg *)(this->GetParent()->GetParent()));
	DDX_Check(pDX, IDC_FAKEVERSION, cTarget->m_FakeVersion);
	DDX_LBIndex(pDX, IDC_LISTFAKE, cTarget->m_FakeVersionId);
	DDX_Check(pDX, IDC_HANDLEEXCEPTIONS, cTarget->m_HandleExceptions);
	DDX_Check(pDX, IDC_LIMITRESOURCES, cTarget->m_LimitResources);
	DDX_Check(pDX, IDC_SUPPRESSIME, cTarget->m_SuppressIME);
}

BEGIN_MESSAGE_MAP(CTabCompat, CDialog)
	//{{AFX_MSG_MAP(CTabCompat)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTabCompat message handlers

static struct {char bMajor; char bMinor; char *sName;} WinVersions[6]=
{
	{5, 0, "Windows 2000"},
	{5, 1, "Windows XP"},
	{5, 2, "Windows Server 2003"},
	{6, 0, "Windows Vista"},
	{6, 1, "Windows 7"},
	{6, 2, "Windows 8"}
};

BOOL CTabCompat::OnInitDialog()
{
	AfxEnableControlContainer();
	CListBox *List;
	CTargetDlg *cTarget = ((CTargetDlg *)(this->GetParent()->GetParent()));
	int i;
	List=(CListBox *)this->GetDlgItem(IDC_LISTFAKE);
	List->ResetContent();
	for(i=0; i<6; i++) List->AddString(WinVersions[i].sName);
	List->SetCurSel(cTarget->m_FakeVersion);
	CDialog::OnInitDialog();
	return TRUE;
}