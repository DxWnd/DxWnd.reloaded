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
	DDX_Check(pDX, IDC_ENUM16BITMODES, cTarget->m_Enum16bitModes);

	// Direct3D window tweaks
	DDX_Check(pDX, IDC_FIXD3DFRAME, cTarget->m_FixD3DFrame);
	DDX_Check(pDX, IDC_NOWINDOWMOVE, cTarget->m_NoWindowMove);

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

