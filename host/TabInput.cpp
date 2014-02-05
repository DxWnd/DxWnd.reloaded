// TabInput.cpp : implementation file
//

#include "stdafx.h"
#include "TargetDlg.h"
#include "TabInput.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTabInput dialog

CTabInput::CTabInput(CWnd* pParent /*=NULL*/)
//	: CTargetDlg(pParent)
	: CDialog(CTabInput::IDD, pParent)
{
	//{{AFX_DATA_INIT(CTabInput)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

void CTabInput::DoDataExchange(CDataExchange* pDX)
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
	DDX_Check(pDX, IDC_RELEASEMOUSE, cTarget->m_ReleaseMouse);
	DDX_Check(pDX, IDC_FRAMECOMPENSATION, cTarget->m_FrameCompensation);
	// DirectInput
	DDX_Check(pDX, IDC_HOOKDI, cTarget->m_HookDI);
	DDX_Text(pDX, IDC_INITX, cTarget->m_InitX);
	DDX_Text(pDX, IDC_INITY, cTarget->m_InitY);
	DDX_Text(pDX, IDC_MAXX, cTarget->m_MaxX);
	DDX_Text(pDX, IDC_MAXY, cTarget->m_MaxY);
	DDX_Text(pDX, IDC_MINX, cTarget->m_MinX);
	DDX_Text(pDX, IDC_MINY, cTarget->m_MinY);
	// Message processing
	DDX_Check(pDX, IDC_FILTERMESSAGES, cTarget->m_FilterMessages);
	DDX_Check(pDX, IDC_PEEKALLMESSAGES, cTarget->m_PeekAllMessages);
}

BEGIN_MESSAGE_MAP(CTabInput, CDialog)
	//{{AFX_MSG_MAP(CTabInput)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTabInput message handlers
