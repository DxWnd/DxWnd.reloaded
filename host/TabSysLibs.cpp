// TabGDI.cpp : implementation file
//

#include "stdafx.h"
#include "TargetDlg.h"
#include "TabSysLibs.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTabLogs dialog

CTabSysLibs::CTabSysLibs(CWnd* pParent /*=NULL*/)
	: CDialog(CTabSysLibs::IDD, pParent)
{
	//{{AFX_DATA_INIT(CTabSysLibs)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

void CTabSysLibs::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	CTargetDlg *cTarget = ((CTargetDlg *)(this->GetParent()->GetParent()));

	// GDI
	DDX_Check(pDX, IDC_CLIENTREMAPPING, cTarget->m_ClientRemapping);
	DDX_Radio(pDX, IDC_GDINONE, cTarget->m_DCEmulationMode);
	DDX_Check(pDX, IDC_FIXTEXTOUT, cTarget->m_FixTextOut);
	DDX_Check(pDX, IDC_QUALITYFONTS, cTarget->m_QualityFonts);
	DDX_Check(pDX, IDC_NOFILLRECT, cTarget->m_NoFillRect);
	DDX_Check(pDX, IDC_FIXCLIPPERAREA, cTarget->m_FixClipperArea);
	DDX_Check(pDX, IDC_SHAREDDCHYBRID, cTarget->m_SharedDCHybrid);
	DDX_Check(pDX, IDC_SYNCPALETTE, cTarget->m_SyncPalette);
	DDX_Check(pDX, IDC_NOWINERRORS, cTarget->m_NoWinErrors);

	// OpenGL
	DDX_Check(pDX, IDC_HOOKOPENGL, cTarget->m_HookOpenGL); // duplicated
	DDX_Check(pDX, IDC_FORCEHOOKOPENGL, cTarget->m_ForceHookOpenGL);
	DDX_Check(pDX, IDC_FIXPIXELZOOM, cTarget->m_FixPixelZoom);
	DDX_Check(pDX, IDC_FIXBINDTEXTURE, cTarget->m_FixBindTexture);
	DDX_Check(pDX, IDC_HOOKGLUT32, cTarget->m_HookGlut32);
	DDX_Text(pDX, IDC_OPENGLLIB, cTarget->m_OpenGLLib);

	// MCI
	DDX_Check(pDX, IDC_REMAPMCI, cTarget->m_RemapMCI);
	DDX_Check(pDX, IDC_NOMOVIES, cTarget->m_NoMovies);
	DDX_Check(pDX, IDC_STRETCHMOVIES, cTarget->m_StretchMovies);
	DDX_Check(pDX, IDC_FIXMOVIESCOLOR, cTarget->m_FixMoviesColor);
	DDX_Check(pDX, IDC_BYPASSMCI, cTarget->m_BypassMCI);

	// SmackW32
	DDX_Check(pDX, IDC_HOOKSMACKW32, cTarget->m_HookSmackW32);
	DDX_Check(pDX, IDC_FIXSMACKLOOP, cTarget->m_FixSmackLoop);
}

BEGIN_MESSAGE_MAP(CTabSysLibs, CDialog)
	//{{AFX_MSG_MAP(CTabLogs)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTabLogs message handlers
