// TabDirectX.cpp : implementation file
//

#include "stdafx.h"
#include "TargetDlg.h"
#include "TabMouse.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTabMouse dialog

CTabMouse::CTabMouse(CWnd* pParent /*=NULL*/)
//	: CTargetDlg(pParent)
	: CDialog(CTabMouse::IDD, pParent)
{
	//{{AFX_DATA_INIT(CTabMouse)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

void CTabMouse::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	CTargetDlg *cTarget = ((CTargetDlg *)(this->GetParent()->GetParent()));
	DDX_Check(pDX, IDC_MODIFYMOUSE, cTarget->m_ModifyMouse);
	DDX_Check(pDX, IDC_HIDEHWCURSOR, cTarget->m_HideHwCursor);
	DDX_Check(pDX, IDC_SHOWHWCURSOR, cTarget->m_ShowHwCursor);
	DDX_Check(pDX, IDC_ENABLECLIPPING, cTarget->m_EnableClipping);
	DDX_Check(pDX, IDC_CLIPCURSOR, cTarget->m_CursorClipping);
	DDX_Check(pDX, IDC_KEEPCURSORWITHIN, cTarget->m_KeepCursorWithin);
	DDX_Check(pDX, IDC_KEEPCURSORFIXED, cTarget->m_KeepCursorFixed);
	DDX_Check(pDX, IDC_MESSAGEPROC, cTarget->m_MessageProc);
	DDX_Check(pDX, IDC_FIXNCHITTEST, cTarget->m_FixNCHITTEST);
	DDX_Check(pDX, IDC_SLOW, cTarget->m_SlowDown);
}

BEGIN_MESSAGE_MAP(CTabMouse, CDialog)
	//{{AFX_MSG_MAP(CTabMouse)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTabMouse message handlers
