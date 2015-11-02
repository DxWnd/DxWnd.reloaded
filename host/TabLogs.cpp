// TabLogs.cpp : implementation file
//

#include "stdafx.h"
#include "TargetDlg.h"
#include "TabLogs.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTabLogs dialog

CTabLogs::CTabLogs(CWnd* pParent /*=NULL*/)
	: CDialog(CTabLogs::IDD, pParent)
{
	//{{AFX_DATA_INIT(CTabLogs)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

//BOOL CTabLogs::OnInitDialog()
//{
//	extern BOOL gbDebug;
//	CDialog::OnInitDialog();
//	(CButton *)(this->GetDlgItem(IDC_DXPROXED))->EnableWindow(gbDebug ? TRUE : FALSE);
//	(CButton *)(this->GetDlgItem(IDC_ASSERT))->EnableWindow(gbDebug ? TRUE : FALSE);
//	return TRUE;
//}

void CTabLogs::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	CTargetDlg *cTarget = ((CTargetDlg *)(this->GetParent()->GetParent()));
	//extern BOOL gbDebug;
	DDX_Check(pDX, IDC_OUTPROXYTRACE, cTarget->m_OutProxyTrace);
	DDX_Check(pDX, IDC_OUTD3DTRACE, cTarget->m_OutD3DTrace);
	DDX_Check(pDX, IDC_OUTDDRAWTRACE, cTarget->m_OutDDRAWTrace);
	DDX_Check(pDX, IDC_OUTDEBUG, cTarget->m_OutDebug);
	DDX_Check(pDX, IDC_CURSORTRACE, cTarget->m_CursorTrace);
	DDX_Check(pDX, IDC_LOGENABLED, cTarget->m_LogEnabled);
	DDX_Check(pDX, IDC_OUTDEBUGSTRING, cTarget->m_OutDebugString);
	DDX_Check(pDX, IDC_ERASELOGFILE, cTarget->m_EraseLogFile);
	DDX_Check(pDX, IDC_ADDTIMESTAMP, cTarget->m_AddTimeStamp);
	DDX_Check(pDX, IDC_OUTWINMESSAGES, cTarget->m_OutWinMessages);
	DDX_Check(pDX, IDC_OUTDWTRACE, cTarget->m_OutDWTrace);
	DDX_Check(pDX, IDC_IMPORTTABLE, cTarget->m_ImportTable);
	DDX_Check(pDX, IDC_OUTREGISTRY, cTarget->m_RegistryOp);
	DDX_Check(pDX, IDC_TRACEHOOKS, cTarget->m_TraceHooks);
}

BEGIN_MESSAGE_MAP(CTabLogs, CDialog)
	//{{AFX_MSG_MAP(CTabLogs)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTabLogs message handlers
