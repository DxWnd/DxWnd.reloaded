// TabDirectX.cpp : implementation file
//

#include "stdafx.h"
#include "TargetDlg.h"
#include "TabDirectX.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTabDirectX dialog

CTabDirectX::CTabDirectX(CWnd* pParent /*=NULL*/)
	: CDialog(CTabDirectX::IDD, pParent)
{
	//{{AFX_DATA_INIT(CTabDirectX)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

void CTabDirectX::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	CTargetDlg *cTarget = ((CTargetDlg *)(this->GetParent()->GetParent()));
	DDX_Radio(pDX, IDC_AUTO, cTarget->m_DXVersion);
	DDX_Radio(pDX, IDC_NOEMULATESURFACE, cTarget->m_DxEmulationMode);
	//DDX_Check(pDX, IDC_HANDLEDC, cTarget->m_HandleDC);
	DDX_Check(pDX, IDC_SUPPRESSCLIPPING, cTarget->m_SuppressClipping);
	DDX_Check(pDX, IDC_BLITFROMBACKBUFFER, cTarget->m_BlitFromBackBuffer);
	DDX_Check(pDX, IDC_AUTOREFRESH, cTarget->m_AutoRefresh);
	DDX_Check(pDX, IDC_VIDEOTOSYSTEMMEM, cTarget->m_VideoToSystemMem);
	DDX_Check(pDX, IDC_SUPPRESSDXERRORS, cTarget->m_SuppressDXErrors);
	DDX_Check(pDX, IDC_BACKBUFATTACH, cTarget->m_BackBufAttach);
	DDX_Check(pDX, IDC_NOPALETTEUPDATE, cTarget->m_NoPaletteUpdate);
	DDX_Check(pDX, IDC_SETCOMPATIBILITY, cTarget->m_SetCompatibility);
	DDX_Check(pDX, IDC_DISABLEHAL, cTarget->m_DisableHAL);
	DDX_Check(pDX, IDC_FORCESHEL, cTarget->m_ForcesHEL);
	DDX_Check(pDX, IDC_COLORFIX, cTarget->m_ColorFix);
	DDX_Check(pDX, IDC_NOPIXELFORMAT, cTarget->m_NoPixelFormat);
	DDX_Check(pDX, IDC_NOALPHACHANNEL, cTarget->m_NoAlphaChannel);
	DDX_Check(pDX, IDC_FIXREFCOUNTER, cTarget->m_FixRefCounter);
	DDX_Check(pDX, IDC_RETURNNULLREF, cTarget->m_ReturnNullRef);
	DDX_Check(pDX, IDC_NOD3DRESET, cTarget->m_NoD3DReset);
	DDX_Check(pDX, IDC_NOFLIPEMULATION, cTarget->m_NoFlipEmulation);
	DDX_Check(pDX, IDC_FULLRECTBLT, cTarget->m_FullRectBlt);
}

BEGIN_MESSAGE_MAP(CTabDirectX, CDialog)
	//{{AFX_MSG_MAP(CTabDirectX)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTabDirectX message handlers

