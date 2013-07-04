// TargetDlg.cpp : Implementation
//

#define _CRT_SECURE_NO_DEPRECATE 1

#include "stdafx.h"
#include "dxwndhost.h"
#include "TargetDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTargetDlg Dialog

CTargetDlg::CTargetDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CTargetDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CTargetDlg)
	m_DXVersion = -1;
	m_EmulateSurface = FALSE;
	m_NoEmulateSurface = TRUE; // current default
	m_EmulateBuffer = FALSE;
	m_HookDI = FALSE;
	m_ModifyMouse = FALSE;
	m_OutTrace = FALSE;
	m_OutDebug = FALSE;
	m_CursorTrace = FALSE;
	m_OutWinMessages = FALSE;
	m_OutDXTrace = FALSE;
	m_DXProxed = FALSE;
	m_HandleDC = FALSE;
	m_HandleExceptions = FALSE;
	m_SuppressIME = FALSE;
	m_SetCompatibility = FALSE;
	m_LimitResources = FALSE;
	m_UnNotify = FALSE;
	m_Windowize = TRUE;
	m_NoBanner = FALSE;
	m_StartDebug = FALSE;
	m_FilePath = _T("");
	m_Module = _T("");
	m_SaveLoad = FALSE;
	m_SlowDown = FALSE;
	m_BlitFromBackBuffer = FALSE;
	m_SuppressClipping = FALSE;
	m_DisableGammaRamp = FALSE;
	m_AutoRefresh = FALSE;
	m_FixWinFrame = FALSE;
	m_HideHwCursor = FALSE;
	m_ShowHwCursor = FALSE;
	m_EnableClipping = FALSE;
	m_CursorClipping = FALSE;
	m_VideoToSystemMem = FALSE;
	m_FixTextOut = FALSE;
	m_KeepCursorWithin = FALSE;
	m_KeepCursorFixed = FALSE;
	m_UseRGB565 = FALSE;
	m_SuppressDXErrors = FALSE;
	m_PreventMaximize = FALSE;
	m_ClientRemapping = FALSE;
	m_MapGDIToPrimary = FALSE;
	m_LockWinPos = FALSE;
	m_LockWinStyle = FALSE;
	m_FixParentWin = FALSE;
	m_ModalStyle = FALSE;
	m_KeepAspectRatio = FALSE;
	m_ForceWinResize = FALSE;
	m_HookGDI = FALSE;
	m_HideMultiMonitor = FALSE;
	m_WallpaperMode = FALSE;
	m_HookChildWin = FALSE;
	m_MessageProc = FALSE;
	m_FixNCHITTEST = FALSE;
	m_RecoverScreenMode = FALSE;
	m_RefreshOnResize = FALSE;
	m_Init8BPP = FALSE;
	m_Init16BPP = FALSE;
	m_BackBufAttach = FALSE;
	m_HandleAltF4 = FALSE;
	m_LimitFPS = FALSE;
	m_SkipFPS = FALSE;
	m_ShowFPS = FALSE;
	m_ShowFPSOverlay = FALSE;
	m_TimeStretch = FALSE;
	m_HookOpenGL = FALSE;
	m_FakeVersion = FALSE;
	m_FullRectBlt = FALSE;
	m_NoPaletteUpdate = FALSE;
	m_InitX = 0;
	m_InitY = 0;
	m_MaxX = 0;
	m_MaxY = 0;
	m_MinX = 0;
	m_MinY = 0;
	m_PosX = 0;
	m_PosY = 0;
	m_SizX = 800;
	m_SizY = 600;
	m_MaxFPS = 0;
	m_InitTS = 8;
	//}}AFX_DATA_INIT

}

BOOL CTargetDlg::OnInitDialog()
{
	AfxEnableControlContainer();
	CDialog::OnInitDialog();
	m_tabdxTabCtrl.InsertItem(0, _T("Main"));
	m_tabdxTabCtrl.InsertItem(1, _T("Window"));
	m_tabdxTabCtrl.InsertItem(2, _T("Mouse"));
	m_tabdxTabCtrl.InsertItem(3, _T("Timing"));
	m_tabdxTabCtrl.InsertItem(4, _T("Log"));
	m_tabdxTabCtrl.InsertItem(5, _T("DirectX"));
	m_tabdxTabCtrl.InsertItem(6, _T("OpenGL"));
	m_tabdxTabCtrl.InsertItem(7, _T("GDI"));
	m_tabdxTabCtrl.InsertItem(8, _T("Compat"));
	m_tabdxTabCtrl.Init();
	return TRUE;
}

void CTargetDlg::OnOK()
{
	m_tabdxTabCtrl.OnOK();
	CDialog::OnOK();
}

void CTargetDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);

	//{{AFX_DATA_MAP(CTargetDlg)
	DDX_Control(pDX, IDC_TABPANEL, m_tabdxTabCtrl);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CTargetDlg, CDialog)
	//{{AFX_MSG_MAP(CTargetDlg)
	//}}AFX_MSG_MAP

END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTargetDlg Message Handler
