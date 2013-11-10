// TabDebug.cpp : implementation file
//

#include "stdafx.h"
#include "TargetDlg.h"
#include "TabDebug.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTabCompat dialog

CTabDebug::CTabDebug(CWnd* pParent /*=NULL*/)
//	: CTargetDlg(pParent)
	: CDialog(CTabDebug::IDD, pParent)
{
	//{{AFX_DATA_INIT(CTabDebug)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

void CTabDebug::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	CTargetDlg *cTarget = ((CTargetDlg *)(this->GetParent()->GetParent()));
	DDX_Check(pDX, IDC_RGB2YUV, cTarget->m_ForceRGBtoYUV);
	DDX_Check(pDX, IDC_YUV2RGB, cTarget->m_ForceYUVtoRGB);
	DDX_Check(pDX, IDC_DXPROXED, cTarget->m_DXProxed);
	DDX_Check(pDX, IDC_ASSERT, cTarget->m_AssertDialog);
	DDX_Check(pDX, IDC_FULLRECTBLT, cTarget->m_FullRectBlt);
	DDX_Check(pDX, IDC_MARKBLIT, cTarget->m_MarkBlit);
	DDX_Check(pDX, IDC_SURFACEWARN, cTarget->m_SurfaceWarn);
}

BEGIN_MESSAGE_MAP(CTabDebug, CDialog)
	//{{AFX_MSG_MAP(CTabCompat)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTabCompat message handlers


//BOOL CTabCompat::OnInitDialog()
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