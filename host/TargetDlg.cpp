// TargetDlg.cpp : Implementation
//

#include "stdafx.h"
#include "dxwndhost.h"
#include "TargetDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern BOOL gbDebug;

/////////////////////////////////////////////////////////////////////////////
// CTargetDlg Dialog

CTargetDlg::CTargetDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CTargetDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CTargetDlg)
	m_DXVersion = 0;
	m_Coordinates = 0;
	m_DxEmulationMode = 3; // default: EMULATESURFACE
	m_DCEmulationMode = 0; // default: no emulation
	m_HookDI = FALSE;
	m_ModifyMouse = TRUE; // default true !!
	m_OutProxyTrace = FALSE;
	m_OutDebug = FALSE;
	m_CursorTrace = FALSE;
	m_OutWinMessages = FALSE;
	m_OutDWTrace = FALSE;
	m_OutD3DTrace = FALSE;
	m_OutDDRAWTrace = FALSE;
	m_DXProxed = FALSE;
	m_HandleDC = FALSE;
	m_HandleExceptions = FALSE;
	m_SuppressIME = FALSE;
	m_SuppressD3DExt = FALSE;
	m_SetCompatibility = TRUE;
	m_DisableHAL = FALSE;
	m_ForcesHEL = FALSE;
	m_ColorFix = FALSE;
	m_NoPixelFormat = FALSE;
	m_NoAlphaChannel = FALSE;
	m_FixRefCounter = TRUE;
	m_SuppressChild = FALSE;
	m_LockSysColors = FALSE;
	m_ForceYUVtoRGB = FALSE;
	m_ForceRGBtoYUV = FALSE;
	m_LimitScreenRes = FALSE;
	m_SaveCaps = FALSE;
	m_SingleProcAffinity = FALSE;
	m_LimitResources = FALSE;
	m_CDROMDriveType = FALSE;
	m_FontBypass = FALSE;
	m_BufferedIOFix = FALSE;
	m_ZBufferClean = FALSE;
	m_ZBuffer0Clean = FALSE;
	m_ZBufferAlways = FALSE;
	m_DisableFogging = FALSE;
	m_NoPower2Fix = FALSE;
	m_NoPerfCounter = FALSE;
	m_AddProxyLibs = FALSE;
	m_UnNotify = FALSE;
	m_Windowize = TRUE; // default true !!
	m_HookDLLs = TRUE; // default true !!
	m_HookEnabled = TRUE; // default true !!
	m_EmulateRegistry = FALSE; 
	m_FullScreenOnly = FALSE; 
	m_FilterMessages = FALSE; 
	m_PeekAllMessages = FALSE; 
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
	m_HookGlide = FALSE;
	m_KeepCursorWithin = FALSE;
	m_KeepCursorFixed = FALSE;
	m_UseRGB565 = FALSE;
	m_SuppressDXErrors = FALSE;
	m_MarkBlit = FALSE;
	m_PreventMaximize = FALSE;
	m_ClientRemapping = TRUE; // default true !!
	m_LockWinPos = FALSE;
	m_LockWinStyle = FALSE;
	m_FixParentWin = FALSE;
	m_ModalStyle = FALSE;
	m_KeepAspectRatio = FALSE;
	m_ForceWinResize = FALSE;
	m_HideMultiMonitor = FALSE;
	m_WallpaperMode = FALSE;
	m_FixD3DFrame = FALSE;
	m_NoWindowMove = FALSE;
	m_Force16BPP = FALSE;
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
	m_ShowTimeStretch = FALSE;
	m_TimeStretch = FALSE;
	m_InterceptRDTSC = FALSE;
	m_HookOpenGL = FALSE;
	m_ForceHookOpenGL = FALSE;
	m_FakeVersion = FALSE;
	m_FullRectBlt = FALSE;
	m_SurfaceWarn = FALSE;
	m_CapMask = FALSE;
	m_NoDDRAWBlt = FALSE;
	m_NoDDRAWFlip = FALSE;
	m_NoGDIBlt = FALSE;
	m_NoFillRect = FALSE;
	m_AnalyticMode = FALSE;
	m_NoPaletteUpdate = FALSE;
	m_WireFrame = FALSE;
	m_BlackWhite = FALSE;
	m_InitX = 0;
	m_InitY = 0;
	m_MaxX = 0;
	m_MaxY = 0;
	m_MinX = 0;
	m_MinY = 0;
	m_PosX = 50;
	m_PosY = 50;
	m_SizX = 800;
	m_SizY = 600;
	m_MaxFPS = 0;
	m_InitTS = 8;
	//}}AFX_DATA_INIT

}

BOOL CTargetDlg::OnInitDialog()
{
	int i=0;
	AfxEnableControlContainer();
	CDialog::OnInitDialog();
	m_tabdxTabCtrl.InsertItem(i++, _T("Main"));
	m_tabdxTabCtrl.InsertItem(i++, _T("Video"));
	m_tabdxTabCtrl.InsertItem(i++, _T("Input"));
	m_tabdxTabCtrl.InsertItem(i++, _T("DirectX"));
	m_tabdxTabCtrl.InsertItem(i++, _T("Timing"));
	m_tabdxTabCtrl.InsertItem(i++, _T("Log"));
	m_tabdxTabCtrl.InsertItem(i++, _T("Libs"));
	m_tabdxTabCtrl.InsertItem(i++, _T("Compat"));
	if (gbDebug) m_tabdxTabCtrl.InsertItem(i++, _T("Debug"));
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
