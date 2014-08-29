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
	DDX_Check(pDX, IDC_MARKBLIT, cTarget->m_MarkBlit);
	DDX_Check(pDX, IDC_ANALYTICMODE, cTarget->m_AnalyticMode);
	DDX_Check(pDX, IDC_SURFACEWARN, cTarget->m_SurfaceWarn);
	DDX_Check(pDX, IDC_CAPMASK, cTarget->m_CapMask);
	DDX_Check(pDX, IDC_NODDRAWBLT, cTarget->m_NoDDRAWBlt);
	DDX_Check(pDX, IDC_NODDRAWFLIP, cTarget->m_NoDDRAWFlip);
	DDX_Check(pDX, IDC_NOGDIBLT, cTarget->m_NoGDIBlt);
	DDX_Check(pDX, IDC_NOFILLRECT, cTarget->m_NoFillRect);
	DDX_Check(pDX, IDC_ZBUFFERALWAYS, cTarget->m_ZBufferAlways);
	DDX_Check(pDX, IDC_HOTPATCHALWAYS, cTarget->m_HotPatchAlways);
}

BEGIN_MESSAGE_MAP(CTabDebug, CDialog)
	//{{AFX_MSG_MAP(CTabCompat)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTabCompat message handlers


