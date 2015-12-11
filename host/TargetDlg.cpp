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
	m_DxEmulationMode = 3; // default: emulated
	m_DxFilterMode = 0; // default: ddraw filtering
	m_DCEmulationMode = 0; // default: no emulation
	m_MouseVisibility = 0;
	m_TextureHandling = 0;
	m_HookDI = FALSE;
	m_ModifyMouse = TRUE; // default true !!
	m_VirtualJoystick = FALSE; 
	m_Unacquire = FALSE; 
	m_LogEnabled = FALSE;
	m_OutProxyTrace = FALSE;
	m_OutDebug = FALSE;
	m_RegistryOp = FALSE;
	m_CursorTrace = FALSE;
	m_OutWinMessages = FALSE;
	m_OutDWTrace = FALSE;
	m_OutD3DTrace = FALSE;
	m_OutDDRAWTrace = FALSE;
	m_OutDebugString = FALSE;
	m_EraseLogFile = FALSE;
	m_AddTimeStamp = FALSE;
	m_ImportTable = FALSE;
	m_TraceHooks = FALSE;
	m_DXProxed = FALSE;
	//m_HandleDC = FALSE;
	m_HandleExceptions = FALSE;
	m_SuppressIME = FALSE;
	m_SuppressD3DExt = FALSE;
	m_SetCompatibility = TRUE; // default true !!
	m_AEROBoost = TRUE; // default true !!
	m_DiabloTweak = FALSE;
	m_EASportsHack = FALSE;
	m_LegacyAlloc = FALSE;
	m_DisableMaxWinMode = FALSE;
	m_NoImagehlp = FALSE;
	m_ReplacePrivOps = FALSE;
	m_ForcesHEL = FALSE;
	m_SetZBufferBitDepths = FALSE;
	m_ForcesSwapEffect = FALSE;
	m_ColorFix = FALSE;
	m_NoPixelFormat = FALSE;
	m_NoAlphaChannel = FALSE;
	m_FixRefCounter = TRUE; // default true !!
	m_ReturnNullRef = FALSE;
	m_NoD3DReset = FALSE;
	m_HideDesktop = FALSE;
	m_HideTaskbar = FALSE;
	m_ActivateApp = FALSE;
	m_UnlockZOrder = FALSE;
	m_NoDestroyWindow = FALSE;
	m_LockSysColors = FALSE;
	m_LockReservedPalette = FALSE;
	m_ForceYUVtoRGB = FALSE;
	m_ForceRGBtoYUV = FALSE;
	m_LimitScreenRes = FALSE;
	m_SingleProcAffinity = FALSE;
	m_LimitResources = FALSE;
	m_CDROMDriveType = FALSE;
	m_HideCDROMEmpty = FALSE;
	m_FontBypass = FALSE;
	m_BufferedIOFix = FALSE;
	m_ZBufferClean = FALSE;
	m_ZBuffer0Clean = FALSE;
	m_ZBufferAlways = FALSE;
	m_HotPatchAlways = FALSE;
	m_FreezeInjectedSon = FALSE;
	m_StressResources = FALSE;
	m_DisableFogging = FALSE;
	m_NoPower2Fix = FALSE;
	m_NoPerfCounter = FALSE;
	m_UnNotify = FALSE;
	m_Windowize = TRUE; // default true !!
	m_HotPatch = FALSE; 
	m_HookDLLs = TRUE; // default true !!
	m_TerminateOnClose = FALSE; // default true !!
	m_ConfirmOnClose = FALSE; // default true !!
	m_HookEnabled = TRUE; // default true !!
	m_EmulateRegistry = FALSE; 
	m_OverrideRegistry = FALSE; 
	m_Wow64Registry = FALSE; 
	m_Wow32Registry = FALSE; 
	m_FullScreenOnly = FALSE; 
	m_FilterMessages = FALSE; 
	m_PeekAllMessages = FALSE; 
	m_NoWinPosChanges = FALSE; 
	m_MessagePump = FALSE; 
	m_NoBanner = FALSE;
	m_StartDebug = FALSE;
	m_FilePath = _T("");
	m_Module = _T("");
	m_SaveLoad = FALSE;
	m_SlowDown = FALSE;
	m_BlitFromBackBuffer = FALSE;
	m_NoFlipEmulation = FALSE;
	m_SuppressClipping = FALSE;
	m_DisableGammaRamp = FALSE;
	m_AutoRefresh = FALSE;
	m_TextureFormat = FALSE;
	m_FixWinFrame = FALSE;
	m_EnableClipping = FALSE;
	m_CursorClipping = FALSE;
	m_VideoToSystemMem = FALSE;
	m_FixTextOut = FALSE;
	m_SharedDC = TRUE; // seems better .....
	m_HookGlide = FALSE;
	m_RemapMCI = TRUE;
	m_NoMovies = FALSE;
	m_FixMoviesColor = FALSE;
	m_StretchMovies = FALSE;
	m_BypassMCI = FALSE;
	m_SuppressRelease = FALSE;
	m_KeepCursorWithin = FALSE;
	m_KeepCursorFixed = FALSE;
	m_UseRGB565 = TRUE; // seems the default for 16bit video mode
	m_SuppressDXErrors = FALSE;
	m_FlipEmulation = TRUE; // better set true default, since this mode is more compatible
	m_MarkBlit = FALSE;
	m_MarkLock = FALSE;
	m_NoSysMemPrimary = FALSE;
	m_NoSysMemBackBuf = FALSE;
	m_NoBlt = FALSE;
	m_BilinearBlt = FALSE;
	m_FastBlt = FALSE;
	m_GDIColorConv = FALSE;
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
	//m_NoMouseProc = FALSE;
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
	m_StretchTimers = FALSE;
	m_NormalizePerfCount = FALSE;
	m_QuarterBlt = FALSE;
	m_FineTiming = FALSE;
	m_ReleaseMouse = FALSE;
	//m_FrameCompensation = FALSE;
	m_EnableHotKeys = TRUE; // default true !!
	m_InterceptRDTSC = FALSE;
	m_HookOpenGL = FALSE;
	m_ForceHookOpenGL = FALSE;
	m_FixPixelZoom = FALSE;
	m_FakeVersion = FALSE;
	m_FullRectBlt = FALSE;
	m_CenterToWin = FALSE;
	m_Deinterlace = FALSE;
	m_SurfaceWarn = FALSE;
	m_CapMask = FALSE;
	m_NoWindowHooks = FALSE;
	m_NoDDRAWBlt = FALSE;
	m_NoDDRAWFlip = FALSE;
	m_NoGDIBlt = FALSE;
	m_NoFillRect = FALSE;
	m_ReuseEmulatedDC = FALSE; // ??
	m_CreateDesktop = FALSE;
	m_SyncPalette = FALSE;
	m_AnalyticMode = FALSE;
	m_NoPaletteUpdate = FALSE;
	m_WireFrame = FALSE;
	m_NoTextures = FALSE;
	m_BlackWhite = FALSE;
	m_AssertDialog = FALSE;
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
	m_SwapEffect = 0;
	//}}AFX_DATA_INIT

}

BOOL CTargetDlg::OnInitDialog()
{
	int i=0;
	AfxEnableControlContainer();
	CDialog::OnInitDialog();
#if 0
	m_tabdxTabCtrl.InsertItem(i++, _T("Main"));
	m_tabdxTabCtrl.InsertItem(i++, _T("Video"));
	m_tabdxTabCtrl.InsertItem(i++, _T("Input"));
	m_tabdxTabCtrl.InsertItem(i++, _T("DirectX"));
	m_tabdxTabCtrl.InsertItem(i++, _T("Timing"));
	m_tabdxTabCtrl.InsertItem(i++, _T("Log"));
	m_tabdxTabCtrl.InsertItem(i++, _T("Libs"));
	m_tabdxTabCtrl.InsertItem(i++, _T("Compat"));
	m_tabdxTabCtrl.InsertItem(i++, _T("Registry"));
	m_tabdxTabCtrl.InsertItem(i++, _T("Notes"));
	if (gbDebug) m_tabdxTabCtrl.InsertItem(i++, _T("Debug"));
#else
	char sCaption[48+1];
	LoadString(AfxGetResourceHandle(), DXW_TAB_MAIN, sCaption, sizeof(sCaption));
	m_tabdxTabCtrl.InsertItem(i++, _T(sCaption));
	LoadString(AfxGetResourceHandle(), DXW_TAB_VIDEO, sCaption, sizeof(sCaption));
	m_tabdxTabCtrl.InsertItem(i++, _T(sCaption));
	LoadString(AfxGetResourceHandle(), DXW_TAB_INPUT, sCaption, sizeof(sCaption));
	m_tabdxTabCtrl.InsertItem(i++, _T(sCaption));
	LoadString(AfxGetResourceHandle(), DXW_TAB_DIRECTX, sCaption, sizeof(sCaption));
	m_tabdxTabCtrl.InsertItem(i++, _T(sCaption));
	LoadString(AfxGetResourceHandle(), DXW_TAB_D3D, sCaption, sizeof(sCaption));
	m_tabdxTabCtrl.InsertItem(i++, _T(sCaption));
	LoadString(AfxGetResourceHandle(), DXW_TAB_TIMING, sCaption, sizeof(sCaption));
	m_tabdxTabCtrl.InsertItem(i++, _T(sCaption));
	LoadString(AfxGetResourceHandle(), DXW_TAB_LOGS, sCaption, sizeof(sCaption));
	m_tabdxTabCtrl.InsertItem(i++, _T(sCaption));
	LoadString(AfxGetResourceHandle(), DXW_TAB_LIBS, sCaption, sizeof(sCaption));
	m_tabdxTabCtrl.InsertItem(i++, _T(sCaption));
	LoadString(AfxGetResourceHandle(), DXW_TAB_COMPAT, sCaption, sizeof(sCaption));
	m_tabdxTabCtrl.InsertItem(i++, _T(sCaption));
	LoadString(AfxGetResourceHandle(), DXW_TAB_REGISTRY, sCaption, sizeof(sCaption));
	m_tabdxTabCtrl.InsertItem(i++, _T(sCaption));
	LoadString(AfxGetResourceHandle(), DXW_TAB_NOTES, sCaption, sizeof(sCaption));
	m_tabdxTabCtrl.InsertItem(i++, _T(sCaption));
	LoadString(AfxGetResourceHandle(), DXW_TAB_DEBUG, sCaption, sizeof(sCaption));
	if (gbDebug) m_tabdxTabCtrl.InsertItem(i++, _T(sCaption));
#endif
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
