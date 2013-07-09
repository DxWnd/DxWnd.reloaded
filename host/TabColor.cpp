// TabLogs.cpp : implementation file
//

#include "stdafx.h"
#include "TargetDlg.h"
#include "TabColor.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTabColor dialog

CTabColor::CTabColor(CWnd* pParent /*=NULL*/)
	: CDialog(CTabColor::IDD, pParent)
{
	//{{AFX_DATA_INIT(CTabColor)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

void CTabColor::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	CTargetDlg *cTarget = ((CTargetDlg *)(this->GetParent()->GetParent()));
	DDX_Check(pDX, IDC_INIT8BPP, cTarget->m_Init8BPP);
	DDX_Check(pDX, IDC_INIT16BPP, cTarget->m_Init16BPP);
	DDX_Check(pDX, IDC_DISABLEGAMMARAMP, cTarget->m_DisableGammaRamp);
	DDX_Check(pDX, IDC_FORCE16BPP, cTarget->m_Force16BPP);
	DDX_Check(pDX, IDC_WIREFRAME, cTarget->m_WireFrame);
	DDX_Check(pDX, IDC_BLACKWHITE, cTarget->m_BlackWhite);
	DDX_Check(pDX, IDC_USERGB565, cTarget->m_UseRGB565);
	DDX_Check(pDX, IDC_LOCKSYSCOLORS, cTarget->m_LockSysColors);
	DDX_Check(pDX, IDC_RGB2YUV, cTarget->m_ForceRGBtoYUV);
	DDX_Check(pDX, IDC_YUV2RGB, cTarget->m_ForceYUVtoRGB);
}

BEGIN_MESSAGE_MAP(CTabColor, CDialog)
	//{{AFX_MSG_MAP(CTabColor)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTabColor message handlers
