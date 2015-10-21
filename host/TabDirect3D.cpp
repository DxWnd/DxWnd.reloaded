// TabDirect3D.cpp : implementation file
//

#include "stdafx.h"
#include "TargetDlg.h"
#include "TabDirect3D.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTabDirect3D dialog

CTabDirect3D::CTabDirect3D(CWnd* pParent /*=NULL*/)
	: CDialog(CTabDirect3D::IDD, pParent)
{
	//{{AFX_DATA_INIT(CTabDirect3D)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

void CTabDirect3D::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	CTargetDlg *cTarget = ((CTargetDlg *)(this->GetParent()->GetParent()));

	// Direct3D tweaks
	DDX_Check(pDX, IDC_ZBUFFERCLEAN, cTarget->m_ZBufferClean);
	DDX_Check(pDX, IDC_ZBUFFER0CLEAN, cTarget->m_ZBuffer0Clean);
	DDX_Check(pDX, IDC_NOPOWER2FIX, cTarget->m_NoPower2Fix);
	DDX_Check(pDX, IDC_NOD3DRESET, cTarget->m_NoD3DReset);
	DDX_Check(pDX, IDC_SUPPRESSD3DEXT, cTarget->m_SuppressD3DExt);
	DDX_Check(pDX, IDC_FORCESHEL, cTarget->m_ForcesHEL);
	DDX_Check(pDX, IDC_SETZBUFFERBITDEPTHS, cTarget->m_SetZBufferBitDepths);

	// Ddraw tweaks
	DDX_Check(pDX, IDC_NOSYSMEMPRIMARY, cTarget->m_NoSysMemPrimary);
	DDX_Check(pDX, IDC_NOSYSMEMBACKBUF, cTarget->m_NoSysMemBackBuf);
	DDX_Check(pDX, IDC_FIXPITCH, cTarget->m_FixPitch);
	DDX_Check(pDX, IDC_POWER2WIDTH, cTarget->m_Power2Width);
	DDX_Check(pDX, IDC_FIXREFCOUNTER, cTarget->m_FixRefCounter);
	DDX_Check(pDX, IDC_RETURNNULLREF, cTarget->m_ReturnNullRef);
	DDX_Check(pDX, IDC_FULLRECTBLT, cTarget->m_FullRectBlt);
	DDX_Check(pDX, IDC_CENTERTOWIN, cTarget->m_CenterToWin);
	DDX_Check(pDX, IDC_DEINTERLACE, cTarget->m_Deinterlace);

	// Texture management
	DDX_Radio(pDX, IDC_TEXTURENONE, cTarget->m_TextureHandling);

	// 3D Effects
	DDX_Check(pDX, IDC_NOTEXTURES, cTarget->m_NoTextures);
	DDX_Check(pDX, IDC_WIREFRAME, cTarget->m_WireFrame);
	DDX_Check(pDX, IDC_DISABLEFOGGING, cTarget->m_DisableFogging);
	DDX_Check(pDX, IDC_CLEARTARGET, cTarget->m_ClearTarget);

	// Swap Effect
	DDX_Check(pDX, IDC_FORCESWAPEFFECT, cTarget->m_ForcesSwapEffect);
	DDX_Radio(pDX, IDC_SWAP_DISCARD, cTarget->m_SwapEffect);
}

BEGIN_MESSAGE_MAP(CTabDirect3D, CDialog)
	//{{AFX_MSG_MAP(CTabDirect3D)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTabDirect3D message handlers

