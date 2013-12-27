// dxwndhostView.cpp : CDxwndhostView Define the behavior of the class.
//

#include "stdafx.h"
#include "shlwapi.h"
#include "TlHelp32.h"

#include "dxwndhost.h"

#include "dxwndhostDoc.h"
#include "dxwndhostView.h"
#include "TargetDlg.h"
#include "SystemTray.h"
#include "StatusDialog.h"
#include "PaletteDialog.h"
#include "TimeSliderDialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define	WM_ICON_NOTIFY WM_APP+10

extern UINT m_StartToTray;
extern UINT m_InitialState;
extern char m_ConfigFileName[20+1];
extern BOOL Inject(DWORD, const char *);
extern int KillProcByName(char *);

PRIVATEMAP *pTitles; // global ptr: get rid of it!!
TARGETMAP *pTargets; // idem.

/////////////////////////////////////////////////////////////////////////////
// CDxwndhostView

IMPLEMENT_DYNCREATE(CDxwndhostView, CListView)

BEGIN_MESSAGE_MAP(CDxwndhostView, CListView)
	//{{AFX_MSG_MAP(CDxwndhostView)
	ON_NOTIFY_REFLECT(NM_DBLCLK, OnDblclk)
	ON_COMMAND(ID_EXPLORE, OnExplore)
	ON_COMMAND(ID_LOG_VIEW, OnViewLog)
	ON_COMMAND(ID_LOG_DELETE, OnDeleteLog)
	ON_COMMAND(ID_TASK_KILL, OnKill)
	ON_COMMAND(ID_TASK_PAUSE, OnPause)
	ON_COMMAND(ID_TASK_RESUME, OnResume)
	ON_COMMAND(ID_ADD, OnAdd)
	ON_COMMAND(ID_MODIFY, OnModify)
	ON_COMMAND(ID_PEXPORT, OnExport)
	ON_COMMAND(ID_PKILL, OnProcessKill)
	ON_COMMAND(ID_FILE_IMPORT, OnImport)
	ON_COMMAND(ID_DELETE, OnDelete)
	ON_COMMAND(ID_FILE_SORTPROGRAMSLIST, OnSort)
	ON_COMMAND(ID_FILE_CLEARALLLOGS, OnClearAllLogs)
	ON_COMMAND(ID_FILE_GOTOTRAYICON, OnGoToTrayIcon)
	ON_COMMAND(ID_FILE_SAVE, OnSaveFile)
	ON_COMMAND(ID_HOOK_START, OnHookStart)
	ON_COMMAND(ID_HOOK_STOP, OnHookStop)
	ON_COMMAND(ID_DXAPP_EXIT, OnExit)
	ON_WM_RBUTTONDOWN()
	ON_COMMAND(ID_RUN, OnRun)
	ON_COMMAND(ID_TRAY_RESTORE, OnTrayRestore)
	ON_COMMAND(ID_VIEW_STATUS, OnViewStatus)
	ON_COMMAND(ID_VIEW_PALETTE, OnViewPalette)
	ON_COMMAND(ID_VIEW_TIMESLIDER, OnViewTimeSlider)
	ON_COMMAND(ID_DESKTOPCOLORDEPTH_8BPP, OnDesktopcolordepth8bpp)
	ON_COMMAND(ID_DESKTOPCOLORDEPTH_16BPP, OnDesktopcolordepth16bpp)
	ON_COMMAND(ID_DESKTOPCOLORDEPTH_24BPP, OnDesktopcolordepth24bpp)
	ON_COMMAND(ID_DESKTOPCOLORDEPTH_32BPP, OnDesktopcolordepth32bpp)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// v2.1.68: dialog box for status check.
static BOOL CheckStatus()
{
	if(GetHookStatus(NULL)==DXW_RUNNING){
		MessageBoxEx(0, "A hooked task is still running.\nWait its termination.", 
			"Warning", MB_OK | MB_ICONEXCLAMATION, NULL);
		return TRUE;
	}
	return FALSE;
}

// v2.1.78: recover desktop settings.
static void RevertScreenChanges(DEVMODE *InitDevMode)
{
	DEVMODE CurDevMode;	
	EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &CurDevMode);
	if ((CurDevMode.dmPelsHeight != InitDevMode->dmPelsHeight) ||
		(CurDevMode.dmPelsWidth != InitDevMode->dmPelsWidth) ||
		(CurDevMode.dmBitsPerPel != InitDevMode->dmBitsPerPel)){
		if (MessageBoxEx(0, 
		"Desktop setting has changed.\n"
		"Do you want to restore the previous ones?", 
		"Warning", MB_YESNO | MB_ICONQUESTION, NULL)==IDYES) 
		ChangeDisplaySettings(InitDevMode, 0);
	}
}

static void SetTargetFromDlg(TARGETMAP *t, CTargetDlg *dlg)
{
	strcpy_s(t->path, sizeof(t->path), dlg->m_FilePath);
	strcpy_s(t->module, sizeof(t->module), dlg->m_Module);
	strcpy_s(t->OpenGLLib, sizeof(t->OpenGLLib), dlg->m_OpenGLLib);
	if(dlg->m_DXVersion > 1) dlg->m_DXVersion += 5;
	t->dxversion = dlg->m_DXVersion;
	t->coordinates = dlg->m_Coordinates;
	t->flags = 0;
	t->flags2 = 0;
	t->flags3 = 0;
	t->flags4 = 0;
	t->tflags = 0;
	if(dlg->m_UnNotify) t->flags |= UNNOTIFY;
	if(dlg->m_Windowize) t->flags2 |= WINDOWIZE;
	if(dlg->m_HookDLLs) t->flags3 |= HOOKDLLS;
	if(dlg->m_EmulateRegistry) t->flags3 |= EMULATEREGISTRY;
	if(dlg->m_HookEnabled) t->flags3 |= HOOKENABLED;
	if(dlg->m_NoBanner) t->flags2 |= NOBANNER;
	if(dlg->m_StartDebug) t->flags2 |= STARTDEBUG;
	if(dlg->m_FullScreenOnly) t->flags3 |= FULLSCREENONLY;
	if(dlg->m_FilterMessages) t->flags3 |= FILTERMESSAGES;
	if(dlg->m_PeekAllMessages) t->flags3 |= PEEKALLMESSAGES;

	t->flags &= ~EMULATEFLAGS;
	switch(dlg->m_DxEmulationMode){
		case 0: break;
		case 1: t->flags |= EMULATEBUFFER; break;
		case 2: t->flags |= LOCKEDSURFACE; break;
		case 3: t->flags |= EMULATESURFACE; break;
	}
	t->flags2 &= ~HOOKGDI;
	t->flags &= ~MAPGDITOPRIMARY;
	t->flags3 &= ~EMULATEDC;
	switch(dlg->m_DCEmulationMode){
		case 0: break;
		case 1: t->flags2 |= HOOKGDI; break;
		case 2: t->flags3 |= EMULATEDC; break;
		case 3: t->flags |= MAPGDITOPRIMARY; break;
	}
	if(dlg->m_HookDI) t->flags |= HOOKDI;
	if(dlg->m_ModifyMouse) t->flags |= MODIFYMOUSE;
	if(dlg->m_OutProxyTrace) t->tflags |= OUTPROXYTRACE;
	if(dlg->m_OutDebug) t->tflags |= OUTDEBUG;
	if(dlg->m_CursorTrace) t->tflags |= OUTCURSORTRACE;
	if(dlg->m_LogEnabled) t->tflags |= OUTTRACE;
	if(dlg->m_OutWinMessages) t->tflags |= OUTWINMESSAGES;
	if(dlg->m_OutDWTrace) t->tflags |= OUTDXWINTRACE;
	if(dlg->m_OutDDRAWTrace) t->tflags |= OUTDDRAWTRACE;
	if(dlg->m_OutD3DTrace) t->tflags |= OUTD3DTRACE;
	if(dlg->m_DXProxed) t->tflags |= DXPROXED;
	if(dlg->m_AssertDialog) t->tflags |= ASSERTDIALOG;
	if(dlg->m_ImportTable) t->tflags |= OUTIMPORTTABLE;
	if(dlg->m_RegistryOp) t->tflags |= OUTREGISTRY;
	if(dlg->m_TraceHooks) t->tflags |= TRACEHOOKS;
	if(dlg->m_HandleDC) t->flags |= HANDLEDC;
	if(dlg->m_HandleExceptions) t->flags |= HANDLEEXCEPTIONS;
	if(dlg->m_LimitResources) t->flags2 |= LIMITRESOURCES;
	if(dlg->m_CDROMDriveType) t->flags3 |= CDROMDRIVETYPE;
	if(dlg->m_FontBypass) t->flags3 |= FONTBYPASS;
	if(dlg->m_BufferedIOFix) t->flags3 |= BUFFEREDIOFIX;
	if(dlg->m_ZBufferClean) t->flags4 |= ZBUFFERCLEAN;
	if(dlg->m_ZBuffer0Clean) t->flags4 |= ZBUFFER0CLEAN;
	if(dlg->m_ZBufferAlways) t->flags4 |= ZBUFFERALWAYS;
	if(dlg->m_NoPower2Fix) t->flags4 |= NOPOWER2FIX;
	if(dlg->m_NoPerfCounter) t->flags4 |= NOPERFCOUNTER;
	if(dlg->m_AddProxyLibs) t->flags4 |= ADDPROXYLIBS;
	if(dlg->m_DisableFogging) t->flags4 |= DISABLEFOGGING;
	if(dlg->m_SuppressIME) t->flags2 |= SUPPRESSIME;
	if(dlg->m_SuppressD3DExt) t->flags3 |= SUPPRESSD3DEXT;
	if(dlg->m_SetCompatibility) t->flags2 |= SETCOMPATIBILITY;
	if(dlg->m_DisableHAL) t->flags3 |= DISABLEHAL;
	if(dlg->m_ForcesHEL) t->flags3 |= FORCESHEL;
	if(dlg->m_ColorFix) t->flags3 |= COLORFIX;
	if(dlg->m_NoPixelFormat) t->flags3 |= NOPIXELFORMAT;
	if(dlg->m_NoAlphaChannel) t->flags4 |= NOALPHACHANNEL;
	if(dlg->m_FixRefCounter) t->flags4 |= FIXREFCOUNTER;
	if(dlg->m_SuppressChild) t->flags4 |= SUPPRESSCHILD;
	if(dlg->m_LockSysColors) t->flags3 |= LOCKSYSCOLORS;
	if(dlg->m_ForceYUVtoRGB) t->flags3 |= YUV2RGB;
	if(dlg->m_ForceRGBtoYUV) t->flags3 |= RGB2YUV;
	if(dlg->m_SaveCaps) t->flags3 |= SAVECAPS;
	if(dlg->m_SingleProcAffinity) t->flags3 |= SINGLEPROCAFFINITY;
	if(dlg->m_SaveLoad) t->flags |= SAVELOAD;
	if(dlg->m_SlowDown) t->flags |= SLOWDOWN;
	if(dlg->m_BlitFromBackBuffer) t->flags |= BLITFROMBACKBUFFER;
	if(dlg->m_SuppressClipping) t->flags |= SUPPRESSCLIPPING;
	if(dlg->m_DisableGammaRamp) t->flags2 |= DISABLEGAMMARAMP;
	if(dlg->m_AutoRefresh) t->flags |= AUTOREFRESH;
	if(dlg->m_FixWinFrame) t->flags |= FIXWINFRAME;
	if(dlg->m_HideHwCursor) t->flags |= HIDEHWCURSOR;
	if(dlg->m_ShowHwCursor) t->flags2 |= SHOWHWCURSOR;
	if(dlg->m_EnableClipping) t->flags |= ENABLECLIPPING;
	if(dlg->m_CursorClipping) t->flags |= CLIPCURSOR;
	if(dlg->m_VideoToSystemMem) t->flags |= SWITCHVIDEOMEMORY;
	if(dlg->m_FixTextOut) t->flags |= FIXTEXTOUT;
	if(dlg->m_KeepCursorWithin) t->flags |= KEEPCURSORWITHIN;
	if(dlg->m_KeepCursorFixed) t->flags2 |= KEEPCURSORFIXED;
	if(dlg->m_UseRGB565) t->flags |= USERGB565;
	if(dlg->m_SuppressDXErrors) t->flags |= SUPPRESSDXERRORS;
	if(dlg->m_MarkBlit) t->flags3 |= MARKBLIT;
	if(dlg->m_PreventMaximize) t->flags |= PREVENTMAXIMIZE;
	if(dlg->m_ClientRemapping) t->flags |= CLIENTREMAPPING;
	if(dlg->m_LockWinPos) t->flags |= LOCKWINPOS;
	if(dlg->m_LockWinStyle) t->flags |= LOCKWINSTYLE;
	if(dlg->m_FixParentWin) t->flags |= FIXPARENTWIN;
	if(dlg->m_ModalStyle) t->flags2 |= MODALSTYLE;
	if(dlg->m_KeepAspectRatio) t->flags2 |= KEEPASPECTRATIO;
	if(dlg->m_ForceWinResize) t->flags2 |= FORCEWINRESIZE;
	if(dlg->m_HideMultiMonitor) t->flags2 |= HIDEMULTIMONITOR;
	if(dlg->m_WallpaperMode) t->flags2 |= WALLPAPERMODE;
	if(dlg->m_FixD3DFrame) t->flags3 |= FIXD3DFRAME;
	if(dlg->m_NoWindowMove) t->flags3 |= NOWINDOWMOVE;
	if(dlg->m_Force16BPP) t->flags3 |= FORCE16BPP;
	if(dlg->m_HookChildWin) t->flags |= HOOKCHILDWIN;
	if(dlg->m_MessageProc) t->flags |= MESSAGEPROC;
	if(dlg->m_FixNCHITTEST) t->flags2 |= FIXNCHITTEST;
	if(dlg->m_RecoverScreenMode) t->flags2 |= RECOVERSCREENMODE;
	if(dlg->m_RefreshOnResize) t->flags2 |= REFRESHONRESIZE;
	if(dlg->m_Init8BPP) t->flags2 |= INIT8BPP;
	if(dlg->m_Init16BPP) t->flags2 |= INIT16BPP;
	if(dlg->m_BackBufAttach) t->flags2 |= BACKBUFATTACH;
	if(dlg->m_HandleAltF4) t->flags |= HANDLEALTF4;
	if(dlg->m_LimitFPS) t->flags2 |= LIMITFPS;
	if(dlg->m_SkipFPS) t->flags2 |= SKIPFPS;
	if(dlg->m_ShowFPS) t->flags2 |= SHOWFPS;
	if(dlg->m_ShowFPSOverlay) t->flags2 |= SHOWFPSOVERLAY;
	if(dlg->m_ShowTimeStretch) t->flags4 |= SHOWTIMESTRETCH;
	if(dlg->m_TimeStretch) t->flags2 |= TIMESTRETCH;
	if(dlg->m_InterceptRDTSC) t->flags4 |= INTERCEPTRDTSC;
	if(dlg->m_HookOpenGL) t->flags2 |= HOOKOPENGL;
	if(dlg->m_ForceHookOpenGL) t->flags3 |= FORCEHOOKOPENGL;
	if(dlg->m_WireFrame) t->flags2 |= WIREFRAME;
	if(dlg->m_BlackWhite) t->flags3 |= BLACKWHITE;
	if(dlg->m_FakeVersion) t->flags2 |= FAKEVERSION;
	if(dlg->m_FullRectBlt) t->flags2 |= FULLRECTBLT;
	if(dlg->m_NoPaletteUpdate) t->flags2 |= NOPALETTEUPDATE;
	if(dlg->m_SurfaceWarn) t->flags3 |= SURFACEWARN;
	if(dlg->m_CapMask) t->flags3 |= CAPMASK;
	if(dlg->m_NoDDRAWBlt) t->flags3 |= NODDRAWBLT;
	if(dlg->m_NoDDRAWFlip) t->flags3 |= NODDRAWFLIP;
	if(dlg->m_NoGDIBlt) t->flags3 |= NOGDIBLT;
	if(dlg->m_AnalyticMode) t->flags3 |= ANALYTICMODE;
	t->initx = dlg->m_InitX;
	t->inity = dlg->m_InitY;
	t->minx = dlg->m_MinX;
	t->miny = dlg->m_MinY;
	t->maxx = dlg->m_MaxX;
	t->maxy = dlg->m_MaxY;
	t->posx = dlg->m_PosX;
	t->posy = dlg->m_PosY;
	t->sizx = dlg->m_SizX;
	t->sizy = dlg->m_SizY;
	t->MaxFPS = dlg->m_MaxFPS;
	t->InitTS = dlg->m_InitTS-8;
	t->FakeVersionId = dlg->m_FakeVersionId;
	strcpy_s(t->module, sizeof(t->module), dlg->m_Module);
	strcpy_s(t->OpenGLLib, sizeof(t->OpenGLLib), dlg->m_OpenGLLib);
}

static void SetDlgFromTarget(TARGETMAP *t, CTargetDlg *dlg)
{
	dlg->m_DXVersion = t->dxversion;
	if(dlg->m_DXVersion > 6) dlg->m_DXVersion -= 5;
	dlg->m_Coordinates = t->coordinates;
	dlg->m_FilePath = t->path;
	dlg->m_Module = t->module;
	dlg->m_OpenGLLib = t->OpenGLLib;
	dlg->m_UnNotify = t->flags & UNNOTIFY ? 1 : 0;
	dlg->m_Windowize = t->flags2 & WINDOWIZE ? 1 : 0;
	dlg->m_HookDLLs = t->flags3 & HOOKDLLS ? 1 : 0;
	dlg->m_EmulateRegistry = t->flags3 & EMULATEREGISTRY ? 1 : 0;
	dlg->m_HookEnabled = t->flags3 & HOOKENABLED ? 1 : 0;
	dlg->m_NoBanner = t->flags2 & NOBANNER ? 1 : 0;
	dlg->m_StartDebug = t->flags2 & STARTDEBUG ? 1 : 0;
	dlg->m_FullScreenOnly = t->flags3 & FULLSCREENONLY ? 1 : 0;
	dlg->m_FilterMessages = t->flags3 & FILTERMESSAGES ? 1 : 0;
	dlg->m_PeekAllMessages = t->flags3 & PEEKALLMESSAGES ? 1 : 0;

	dlg->m_DxEmulationMode = 0;
	if(t->flags & EMULATEBUFFER) dlg->m_DxEmulationMode = 1;
	if(t->flags & LOCKEDSURFACE) dlg->m_DxEmulationMode = 2;
	if(t->flags & EMULATESURFACE) dlg->m_DxEmulationMode = 3;

	dlg->m_DCEmulationMode = 0;
	if(t->flags2 & HOOKGDI) dlg->m_DCEmulationMode = 1;
	if(t->flags3 & EMULATEDC) dlg->m_DCEmulationMode = 2;
	if(t->flags & MAPGDITOPRIMARY) dlg->m_DCEmulationMode = 3;

	dlg->m_HookDI = t->flags & HOOKDI ? 1 : 0;
	dlg->m_ModifyMouse = t->flags & MODIFYMOUSE ? 1 : 0;
	dlg->m_OutProxyTrace = t->tflags & OUTPROXYTRACE ? 1 : 0;
	dlg->m_OutDebug = t->tflags & OUTDEBUG ? 1 : 0;
	dlg->m_CursorTrace = t->tflags & OUTCURSORTRACE ? 1 : 0;
	dlg->m_LogEnabled = t->tflags & OUTTRACE ? 1 : 0;
	dlg->m_OutWinMessages = t->tflags & OUTWINMESSAGES ? 1 : 0;
	dlg->m_OutDWTrace = t->tflags & OUTDXWINTRACE ? 1 : 0;
	dlg->m_OutD3DTrace = t->tflags & OUTD3DTRACE ? 1 : 0;
	dlg->m_OutDDRAWTrace = t->tflags & OUTDDRAWTRACE ? 1 : 0;
	dlg->m_DXProxed = t->tflags & DXPROXED ? 1 : 0;
	dlg->m_AssertDialog = t->tflags & ASSERTDIALOG ? 1 : 0;
	dlg->m_ImportTable = t->tflags & OUTIMPORTTABLE ? 1 : 0;
	dlg->m_RegistryOp = t->tflags & OUTREGISTRY ? 1 : 0;
	dlg->m_TraceHooks = t->tflags & TRACEHOOKS ? 1 : 0;
	dlg->m_HandleDC = t->flags & HANDLEDC ? 1 : 0;
	dlg->m_HandleExceptions = t->flags & HANDLEEXCEPTIONS ? 1 : 0;
	dlg->m_SuppressIME = t->flags2 & SUPPRESSIME ? 1 : 0;
	dlg->m_SuppressD3DExt = t->flags3 & SUPPRESSD3DEXT ? 1 : 0;
	dlg->m_SetCompatibility = t->flags2 & SETCOMPATIBILITY ? 1 : 0;
	dlg->m_DisableHAL = t->flags3 & DISABLEHAL ? 1 : 0;
	dlg->m_ForcesHEL = t->flags3 & FORCESHEL ? 1 : 0;
	dlg->m_ColorFix = t->flags3 & COLORFIX ? 1 : 0;
	dlg->m_NoPixelFormat = t->flags3 & NOPIXELFORMAT ? 1 : 0;
	dlg->m_NoAlphaChannel = t->flags4 & NOALPHACHANNEL ? 1 : 0;
	dlg->m_FixRefCounter = t->flags4 & FIXREFCOUNTER ? 1 : 0;
	dlg->m_SuppressChild = t->flags4 & SUPPRESSCHILD ? 1 : 0;
	dlg->m_LockSysColors = t->flags3 & LOCKSYSCOLORS ? 1 : 0;
	dlg->m_ForceRGBtoYUV = t->flags3 & RGB2YUV ? 1 : 0;
	dlg->m_ForceYUVtoRGB = t->flags3 & YUV2RGB ? 1 : 0;
	dlg->m_SaveCaps = t->flags3 & SAVECAPS ? 1 : 0;
	dlg->m_SingleProcAffinity = t->flags3 & SINGLEPROCAFFINITY ? 1 : 0;
	dlg->m_LimitResources = t->flags2 & LIMITRESOURCES ? 1 : 0;
	dlg->m_CDROMDriveType = t->flags3 & CDROMDRIVETYPE ? 1 : 0;
	dlg->m_FontBypass = t->flags3 & FONTBYPASS ? 1 : 0;
	dlg->m_BufferedIOFix = t->flags3 & BUFFEREDIOFIX ? 1 : 0;
	dlg->m_ZBufferClean = t->flags4 & ZBUFFERCLEAN ? 1 : 0;
	dlg->m_ZBuffer0Clean = t->flags4 & ZBUFFER0CLEAN ? 1 : 0;
	dlg->m_ZBufferAlways = t->flags4 & ZBUFFERALWAYS ? 1 : 0;
	dlg->m_NoPower2Fix = t->flags4 & NOPOWER2FIX ? 1 : 0;
	dlg->m_NoPerfCounter = t->flags4 & NOPERFCOUNTER ? 1 : 0;
	dlg->m_AddProxyLibs = t->flags4 & ADDPROXYLIBS ? 1 : 0;
	dlg->m_DisableFogging = t->flags4 & DISABLEFOGGING ? 1 : 0;
	dlg->m_SaveLoad = t->flags & SAVELOAD ? 1 : 0;
	dlg->m_SlowDown = t->flags & SLOWDOWN ? 1 : 0;
	dlg->m_BlitFromBackBuffer = t->flags & BLITFROMBACKBUFFER ? 1 : 0;
	dlg->m_SuppressClipping = t->flags & SUPPRESSCLIPPING ? 1 : 0;
	dlg->m_DisableGammaRamp = t->flags2 & DISABLEGAMMARAMP ? 1 : 0;
	dlg->m_AutoRefresh = t->flags & AUTOREFRESH ? 1 : 0;
	dlg->m_FixWinFrame = t->flags & FIXWINFRAME ? 1 : 0;
	dlg->m_HideHwCursor = t->flags & HIDEHWCURSOR ? 1 : 0;
	dlg->m_ShowHwCursor = t->flags2 & SHOWHWCURSOR ? 1 : 0;
	dlg->m_EnableClipping = t->flags & ENABLECLIPPING ? 1 : 0;
	dlg->m_CursorClipping = t->flags & CLIPCURSOR ? 1 : 0;
	dlg->m_VideoToSystemMem = t->flags & SWITCHVIDEOMEMORY ? 1 : 0;
	dlg->m_FixTextOut = t->flags & FIXTEXTOUT ? 1 : 0;
	dlg->m_KeepCursorWithin = t->flags & KEEPCURSORWITHIN ? 1 : 0;
	dlg->m_KeepCursorFixed = t->flags2 & KEEPCURSORFIXED ? 1 : 0;
	dlg->m_UseRGB565 = t->flags & USERGB565 ? 1 : 0;
	dlg->m_SuppressDXErrors = t->flags & SUPPRESSDXERRORS ? 1 : 0;
	dlg->m_MarkBlit = t->flags3 & MARKBLIT ? 1 : 0;
	dlg->m_PreventMaximize = t->flags & PREVENTMAXIMIZE ? 1 : 0;
	dlg->m_ClientRemapping = t->flags & CLIENTREMAPPING ? 1 : 0;
	dlg->m_LockWinPos = t->flags & LOCKWINPOS ? 1 : 0;
	dlg->m_LockWinStyle = t->flags & LOCKWINSTYLE ? 1 : 0;
	dlg->m_FixParentWin = t->flags & FIXPARENTWIN ? 1 : 0;
	dlg->m_ModalStyle = t->flags2 & MODALSTYLE ? 1 : 0;
	dlg->m_KeepAspectRatio = t->flags2 & KEEPASPECTRATIO ? 1 : 0;
	dlg->m_ForceWinResize = t->flags2 & FORCEWINRESIZE ? 1 : 0;
	dlg->m_HideMultiMonitor = t->flags2 & HIDEMULTIMONITOR ? 1 : 0;
	dlg->m_WallpaperMode = t->flags2 & WALLPAPERMODE ? 1 : 0;
	dlg->m_FixD3DFrame = t->flags3 & FIXD3DFRAME ? 1 : 0;
	dlg->m_NoWindowMove = t->flags3 & NOWINDOWMOVE ? 1 : 0;
	dlg->m_Force16BPP = t->flags3 & FORCE16BPP ? 1 : 0;
	dlg->m_HookChildWin = t->flags & HOOKCHILDWIN ? 1 : 0;
	dlg->m_MessageProc = t->flags & MESSAGEPROC ? 1 : 0;
	dlg->m_FixNCHITTEST = t->flags2 & FIXNCHITTEST ? 1 : 0;
	dlg->m_RecoverScreenMode = t->flags2 & RECOVERSCREENMODE ? 1 : 0;
	dlg->m_RefreshOnResize = t->flags2 & REFRESHONRESIZE ? 1 : 0;
	dlg->m_Init8BPP = t->flags2 & INIT8BPP ? 1 : 0;
	dlg->m_Init16BPP = t->flags2 & INIT16BPP ? 1 : 0;
	dlg->m_BackBufAttach = t->flags2 & BACKBUFATTACH ? 1 : 0;
	dlg->m_HandleAltF4 = t->flags & HANDLEALTF4 ? 1 : 0;
	dlg->m_LimitFPS = t->flags2 & LIMITFPS ? 1 : 0;
	dlg->m_SkipFPS = t->flags2 & SKIPFPS ? 1 : 0;
	dlg->m_ShowFPS = t->flags2 & SHOWFPS ? 1 : 0;
	dlg->m_ShowFPSOverlay = t->flags2 & SHOWFPSOVERLAY ? 1 : 0;
	dlg->m_ShowTimeStretch = t->flags4 & SHOWTIMESTRETCH ? 1 : 0;
	dlg->m_TimeStretch = t->flags2 & TIMESTRETCH ? 1 : 0;
	dlg->m_InterceptRDTSC = t->flags4 & INTERCEPTRDTSC ? 1 : 0;
	dlg->m_HookOpenGL = t->flags2 & HOOKOPENGL ? 1 : 0;
	dlg->m_ForceHookOpenGL = t->flags3 & FORCEHOOKOPENGL ? 1 : 0;
	dlg->m_WireFrame = t->flags2 & WIREFRAME ? 1 : 0;
	dlg->m_BlackWhite = t->flags3 & BLACKWHITE ? 1 : 0;
	dlg->m_FakeVersion = t->flags2 & FAKEVERSION ? 1 : 0;
	dlg->m_FullRectBlt = t->flags2 & FULLRECTBLT ? 1 : 0;
	dlg->m_NoPaletteUpdate = t->flags2 & NOPALETTEUPDATE ? 1 : 0;
	dlg->m_SurfaceWarn = t->flags3 & SURFACEWARN ? 1 : 0;
	dlg->m_CapMask = t->flags3 & CAPMASK ? 1 : 0;
	dlg->m_NoDDRAWBlt = t->flags3 & NODDRAWBLT ? 1 : 0;
	dlg->m_NoDDRAWFlip = t->flags3 & NODDRAWFLIP ? 1 : 0;
	dlg->m_NoGDIBlt = t->flags3 & NOGDIBLT ? 1 : 0;
	dlg->m_AnalyticMode = t->flags3 & ANALYTICMODE ? 1 : 0;
	dlg->m_InitX = t->initx;
	dlg->m_InitY = t->inity;
	dlg->m_MinX = t->minx;
	dlg->m_MinY = t->miny;
	dlg->m_MaxX = t->maxx;
	dlg->m_MaxY = t->maxy;
	dlg->m_PosX = t->posx;
	dlg->m_PosY = t->posy;
	dlg->m_SizX = t->sizx;
	dlg->m_SizY = t->sizy;
	dlg->m_MaxFPS = t->MaxFPS;
	dlg->m_InitTS = t->InitTS+8;
	dlg->m_FakeVersionId = t->FakeVersionId;
}

static void SaveConfigItem(TARGETMAP *TargetMap, char *Title, int i, char *InitPath)
{
	char key[32], val[32];
	sprintf_s(key, sizeof(key), "title%i", i);
	WritePrivateProfileString("target", key, Title, InitPath);
	sprintf_s(key, sizeof(key), "path%i", i);
	WritePrivateProfileString("target", key, TargetMap->path, InitPath);
	sprintf_s(key, sizeof(key), "module%i", i);
	WritePrivateProfileString("target", key, TargetMap->module, InitPath);
	sprintf_s(key, sizeof(key), "opengllib%i", i);
	WritePrivateProfileString("target", key, TargetMap->OpenGLLib, InitPath);
	sprintf_s(key, sizeof(key), "ver%i", i);
	sprintf_s(val, sizeof(val), "%i", TargetMap->dxversion);
	WritePrivateProfileString("target", key, val, InitPath);
	sprintf_s(key, sizeof(key), "coord%i", i);
	sprintf_s(val, sizeof(val), "%i", TargetMap->coordinates);
	WritePrivateProfileString("target", key, val, InitPath);
	sprintf_s(key, sizeof(key), "flag%i", i);
	sprintf_s(val, sizeof(val), "%i", TargetMap->flags);
	WritePrivateProfileString("target", key, val, InitPath);
	sprintf_s(key, sizeof(key), "flagg%i", i);
	sprintf_s(val, sizeof(val), "%i", TargetMap->flags2);
	WritePrivateProfileString("target", key, val, InitPath);
	sprintf_s(key, sizeof(key), "flagh%i", i);
	sprintf_s(val, sizeof(val), "%i", TargetMap->flags3);
	WritePrivateProfileString("target", key, val, InitPath);
	sprintf_s(key, sizeof(key), "flagi%i", i);
	sprintf_s(val, sizeof(val), "%i", TargetMap->flags4);
	WritePrivateProfileString("target", key, val, InitPath);
	sprintf_s(key, sizeof(key), "tflag%i", i);
	sprintf_s(val, sizeof(val), "%i", TargetMap->tflags);
	WritePrivateProfileString("target", key, val, InitPath);
	sprintf_s(key, sizeof(key), "initx%i", i);
	sprintf_s(val, sizeof(val), "%i", TargetMap->initx);
	WritePrivateProfileString("target", key, val, InitPath);
	sprintf_s(key, sizeof(key), "inity%i", i);
	sprintf_s(val, sizeof(val), "%i", TargetMap->inity);
	WritePrivateProfileString("target", key, val, InitPath);
	sprintf_s(key, sizeof(key), "minx%i", i);
	sprintf_s(val, sizeof(val), "%i", TargetMap->minx);
	WritePrivateProfileString("target", key, val, InitPath);
	sprintf_s(key, sizeof(key), "miny%i", i);
	sprintf_s(val, sizeof(val), "%i", TargetMap->miny);
	WritePrivateProfileString("target", key, val, InitPath);
	sprintf_s(key, sizeof(key), "maxx%i", i);
	sprintf_s(val, sizeof(val), "%i", TargetMap->maxx);
	WritePrivateProfileString("target", key, val, InitPath);
	sprintf_s(key, sizeof(key), "maxy%i", i);
	sprintf_s(val, sizeof(val), "%i", TargetMap->maxy);
	WritePrivateProfileString("target", key, val, InitPath);
	sprintf_s(key, sizeof(key), "posx%i", i);
	sprintf_s(val, sizeof(val), "%i", TargetMap->posx);
	WritePrivateProfileString("target", key, val, InitPath);
	sprintf_s(key, sizeof(key), "posy%i", i);
	sprintf_s(val, sizeof(val), "%i", TargetMap->posy);
	WritePrivateProfileString("target", key, val, InitPath);
	sprintf_s(key, sizeof(key), "sizx%i", i);
	sprintf_s(val, sizeof(val), "%i", TargetMap->sizx);
	WritePrivateProfileString("target", key, val, InitPath);
	sprintf_s(key, sizeof(key), "sizy%i", i);
	sprintf_s(val, sizeof(val), "%i", TargetMap->sizy);
	WritePrivateProfileString("target", key, val, InitPath);
	sprintf_s(key, sizeof(key), "maxfps%i", i);
	sprintf_s(val, sizeof(val), "%i", TargetMap->MaxFPS);
	WritePrivateProfileString("target", key, val, InitPath);
	sprintf_s(key, sizeof(key), "initts%i", i);
	sprintf_s(val, sizeof(val), "%i", TargetMap->InitTS);
	WritePrivateProfileString("target", key, val, InitPath);
}

static void ClearTarget(int i, char *InitPath)
{
	char key[32];
	sprintf_s(key, sizeof(key), "path%i", i);
	WritePrivateProfileString("target", key, 0, InitPath);
	sprintf_s(key, sizeof(key), "ver%i", i);
	WritePrivateProfileString("target", key, 0, InitPath);
	sprintf_s(key, sizeof(key), "coord%i", i);
	WritePrivateProfileString("target", key, 0, InitPath);
	sprintf_s(key, sizeof(key), "flag%i", i);
	WritePrivateProfileString("target", key, 0, InitPath);
	sprintf_s(key, sizeof(key), "flagg%i", i);
	WritePrivateProfileString("target", key, 0, InitPath);
	sprintf_s(key, sizeof(key), "flagh%i", i);
	WritePrivateProfileString("target", key, 0, InitPath);
	sprintf_s(key, sizeof(key), "flagi%i", i);
	WritePrivateProfileString("target", key, 0, InitPath);
	sprintf_s(key, sizeof(key), "tflag%i", i);
	WritePrivateProfileString("target", key, 0, InitPath);
	sprintf_s(key, sizeof(key), "initx%i", i);
	WritePrivateProfileString("target", key, 0, InitPath);
	sprintf_s(key, sizeof(key), "inity%i", i);
	WritePrivateProfileString("target", key, 0, InitPath);
	sprintf_s(key, sizeof(key), "minx%i", i);
	WritePrivateProfileString("target", key, 0, InitPath);
	sprintf_s(key, sizeof(key), "miny%i", i);
	WritePrivateProfileString("target", key, 0, InitPath);
	sprintf_s(key, sizeof(key), "maxx%i", i);
	WritePrivateProfileString("target", key, 0, InitPath);
	sprintf_s(key, sizeof(key), "maxy%i", i);
	WritePrivateProfileString("target", key, 0, InitPath);
	sprintf_s(key, sizeof(key), "posx%i", i);
	WritePrivateProfileString("target", key, 0, InitPath);
	sprintf_s(key, sizeof(key), "posy%i", i);
	WritePrivateProfileString("target", key, 0, InitPath);
	sprintf_s(key, sizeof(key), "sizx%i", i);
	WritePrivateProfileString("target", key, 0, InitPath);
	sprintf_s(key, sizeof(key), "sizy%i", i);
	WritePrivateProfileString("target", key, 0, InitPath);
	sprintf_s(key, sizeof(key), "maxfps%i", i);
	WritePrivateProfileString("target", key, 0, InitPath);
	sprintf_s(key, sizeof(key), "initts%i", i);
	WritePrivateProfileString("target", key, 0, InitPath);
}

static int LoadConfigItem(TARGETMAP *TargetMap, char *Title, int i, char *InitPath)
{
	char key[32];
	DWORD flags;
	extern BOOL gbDebug;
	sprintf_s(key, sizeof(key), "path%i", i);
	GetPrivateProfileString("target", key, "", TargetMap->path, MAX_PATH, InitPath);
	if(!TargetMap->path[0]) return FALSE;
	sprintf_s(key, sizeof(key), "title%i", i);
	GetPrivateProfileString("target", key, "", Title, sizeof(PRIVATEMAP)-1, InitPath);
	sprintf_s(key, sizeof(key), "module%i", i);
	GetPrivateProfileString("target", key, "", TargetMap->module, sizeof(TargetMap->module)-1, InitPath);
	sprintf_s(key, sizeof(key), "opengllib%i", i);
	GetPrivateProfileString("target", key, "", TargetMap->OpenGLLib, sizeof(TargetMap->OpenGLLib)-1, InitPath);
	sprintf_s(key, sizeof(key), "ver%i", i);
	TargetMap->dxversion = GetPrivateProfileInt("target", key, 0, InitPath);
	sprintf_s(key, sizeof(key), "coord%i", i);
	TargetMap->coordinates = GetPrivateProfileInt("target", key, 0, InitPath);

	// be sure just one of the emulation flags is set
	sprintf_s(key, sizeof(key), "flag%i", i);
	TargetMap->flags = GetPrivateProfileInt("target", key, 0, InitPath);
	flags = TargetMap->flags;
	TargetMap->flags &= ~EMULATEFLAGS;
	do{
		if(flags & EMULATESURFACE) {TargetMap->flags |= EMULATESURFACE; break;}
		if(flags & EMULATEBUFFER) {TargetMap->flags |= EMULATEBUFFER; break;}
		if(flags & LOCKEDSURFACE) {TargetMap->flags |= LOCKEDSURFACE; break;}
	} while (0);

	sprintf_s(key, sizeof(key), "flagg%i", i);
	TargetMap->flags2 = GetPrivateProfileInt("target", key, 0, InitPath);
	sprintf_s(key, sizeof(key), "flagh%i", i);
	TargetMap->flags3 = GetPrivateProfileInt("target", key, 0, InitPath);
	sprintf_s(key, sizeof(key), "flagi%i", i);
	TargetMap->flags4 = GetPrivateProfileInt("target", key, 0, InitPath);
	sprintf_s(key, sizeof(key), "tflag%i", i);
	TargetMap->tflags = GetPrivateProfileInt("target", key, 0, InitPath);
	sprintf_s(key, sizeof(key), "initx%i", i);
	TargetMap->initx = GetPrivateProfileInt("target", key, 0, InitPath);
	sprintf_s(key, sizeof(key), "inity%i", i);
	TargetMap->inity = GetPrivateProfileInt("target", key, 0, InitPath);
	sprintf_s(key, sizeof(key), "minx%i", i);
	TargetMap->minx = GetPrivateProfileInt("target", key, 0, InitPath);
	sprintf_s(key, sizeof(key), "miny%i", i);
	TargetMap->miny = GetPrivateProfileInt("target", key, 0, InitPath);
	sprintf_s(key, sizeof(key), "maxx%i", i);
	TargetMap->maxx = GetPrivateProfileInt("target", key, 0, InitPath);
	sprintf_s(key, sizeof(key), "maxy%i", i);
	TargetMap->maxy = GetPrivateProfileInt("target", key, 0, InitPath);
	sprintf_s(key, sizeof(key), "posx%i", i);
	TargetMap->posx = GetPrivateProfileInt("target", key, 0, InitPath);
	sprintf_s(key, sizeof(key), "posy%i", i);
	TargetMap->posy = GetPrivateProfileInt("target", key, 0, InitPath);
	sprintf_s(key, sizeof(key), "sizx%i", i);
	TargetMap->sizx = GetPrivateProfileInt("target", key, 0, InitPath);
	sprintf_s(key, sizeof(key), "sizy%i", i);
	TargetMap->sizy = GetPrivateProfileInt("target", key, 0, InitPath);
	sprintf_s(key, sizeof(key), "maxfps%i", i);
	TargetMap->MaxFPS = GetPrivateProfileInt("target", key, 0, InitPath);
	sprintf_s(key, sizeof(key), "initts%i", i);
	TargetMap->InitTS = GetPrivateProfileInt("target", key, 0, InitPath);

	if (!gbDebug){
		// clear debug flags
		TargetMap->flags &= ~(0);
		TargetMap->flags2 &= ~(FULLRECTBLT);
		TargetMap->flags3 &= ~(YUV2RGB|RGB2YUV|SURFACEWARN|ANALYTICMODE|NODDRAWBLT|NODDRAWFLIP|NOGDIBLT);
		TargetMap->flags4 &= ~(0);
	}
	return TRUE;
}

static int SetTargetIcon(TARGETMAP tm)
{
	FILE *target;
	target = fopen(tm.path, "r");
	if (target==NULL) return 3;
	fclose(target);
	if (tm.flags3 & HOOKENABLED) return (tm.flags2 & STARTDEBUG) ? 2 : 1;
	return 0;
}

/////////////////////////////////////////////////////////////////////////////
// CDxwndhostView class constructor / destructor

CDxwndhostView::CDxwndhostView()
{
	EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &this->InitDevMode);
}

void CDxwndhostView::SaveConfigFile()
{
	int i;

	for(i = 0; i < MAXTARGETS; i ++){
		if(!TargetMaps[i].path[0]) break;
		SaveConfigItem(&TargetMaps[i], TitleMaps[i].title, i, InitPath);
	}
	for(; i < MAXTARGETS; i ++) ClearTarget(i, InitPath);
	this->isUpdated=FALSE;
}

CDxwndhostView::~CDxwndhostView()
{
	EndHook();

	RevertScreenChanges(&this->InitDevMode);

	if (this->isUpdated)
	if (MessageBoxEx(0, 
		"Task list has changed.\n"
		"Do you want to save it?", 
		"Warning", MB_YESNO | MB_ICONQUESTION, NULL)==IDYES) 
	this->SaveConfigFile();

	if(this->SystemTray.Enabled()){
		this->SystemTray.StopAnimation();
		this->SystemTray.HideIcon();
	}
}

void CDxwndhostView::OnExit()
{
	// check for running apps ....
	if (GetHookStatus(NULL)==DXW_RUNNING){
		if (MessageBoxEx(0, 
			"A hooked task is still running.\n"
			"Exiting now may crash it.\n"
			"Do you still want to exit?", 
		"Warning", MB_OKCANCEL | MB_ICONQUESTION, NULL)==IDCANCEL) return;
	}
	delete(this->GetParent());
}

BOOL CDxwndhostView::PreCreateWindow(CREATESTRUCT& cs)
{
	cs.style |= LVS_REPORT | LVS_SINGLESEL | LVS_NOCOLUMNHEADER;
	return CListView::PreCreateWindow(cs);
}

/////////////////////////////////////////////////////////////////////////////
// CDxwndhostView Drawing Classes

void CDxwndhostView::OnDraw(CDC* pDC)
{
	CDxwndhostDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
}

void CDxwndhostView::OnInitialUpdate()
{
	CListView::OnInitialUpdate();
	CListCtrl& listctrl = GetListCtrl();
	LV_COLUMN listcol;
	LV_ITEM listitem;
	int i;

	// Create 256 color image lists
	HIMAGELIST hList = ImageList_Create(32,32, ILC_COLOR8 |ILC_MASK , 4, 1);
	m_cImageListNormal.Attach(hList);

	hList = ImageList_Create(16, 16, ILC_COLOR8 | ILC_MASK, 4, 1);
	m_cImageListSmall.Attach(hList);

	// Load the large icons
	CBitmap cBmp;
	cBmp.LoadBitmap(IDB_BIGICONS);
	m_cImageListNormal.Add(&cBmp, RGB(255,0, 255));
	cBmp.DeleteObject();

	// Load the small icons
	cBmp.LoadBitmap(IDB_SMALLICONS);
	m_cImageListSmall.Add(&cBmp, RGB(255,0, 255));

	// Attach them
	listctrl.SetImageList(&m_cImageListNormal, LVSIL_NORMAL);
	listctrl.SetImageList(&m_cImageListSmall, LVSIL_SMALL);

	listcol.mask = LVCF_WIDTH;
	listcol.cx = 100;
	
	listctrl.InsertColumn(0, &listcol);
	GetCurrentDirectory(MAX_PATH, InitPath);
	strcat_s(InitPath, sizeof(InitPath), "\\");
	strcat_s(InitPath, sizeof(InitPath), m_ConfigFileName);
	listctrl.InsertColumn(0, &listcol);

	for(i = 0; i < MAXTARGETS; i ++){
		if (!LoadConfigItem(&TargetMaps[i], TitleMaps[i].title, i, InitPath)) break;
		listitem.mask = LVIF_TEXT | LVIF_IMAGE;
		listitem.iItem = i;
		listitem.iSubItem = 0;
		listitem.pszText = TitleMaps[i].title;
		listitem.iImage = SetTargetIcon(TargetMaps[i]);
		listctrl.InsertItem(&listitem);
	}
	for(; i < MAXTARGETS; i ++) TargetMaps[i].path[0] = 0;
	Resize();
	SetTarget(TargetMaps);
	if(m_InitialState == DXW_ACTIVE)
		this->OnHookStart();
	else
		this->OnHookStop();
	if(m_StartToTray) this->OnGoToTrayIcon();
	this->isUpdated=FALSE;
	pTitles = &TitleMaps[0];
	pTargets= &TargetMaps[0];
}

/////////////////////////////////////////////////////////////////////////////
// CDxwndhostView Diagnostic Class

#ifdef _DEBUG
void CDxwndhostView::AssertValid() const
{
	CListView::AssertValid();
}

void CDxwndhostView::Dump(CDumpContext& dc) const
{
	CListView::Dump(dc);
}

CDxwndhostDoc* CDxwndhostView::GetDocument() // Non-debug version is inline.
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CDxwndhostDoc)));
	return (CDxwndhostDoc*)m_pDocument;
}
#endif //_DEBUG


void CDxwndhostView::OnDblclk(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Please add your control notification handler code here.
	OnRun();
	*pResult = 0;
}

void CDxwndhostView::OnExport()
{
	int i;
    char path[MAX_PATH];
	CListCtrl& listctrl = GetListCtrl();
	POSITION pos;
	if(!listctrl.GetSelectedCount()) return;
	pos = listctrl.GetFirstSelectedItemPosition();
	i = listctrl.GetNextSelectedItem(pos);
	//path[0]=0;
	strcpy_s(path, MAX_PATH, TitleMaps[i].title);
	CFileDialog dlg( FALSE, "*.dxw", path, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
        "dxwnd task config (*.dxw)|*.dxw|All Files (*.*)|*.*||",  this);
	if( dlg.DoModal() == IDOK) 
		SaveConfigItem(&TargetMaps[i], TitleMaps[i].title, 0, dlg.GetPathName().GetBuffer());
}

void CDxwndhostView::OnImport()
{
	int i;
    char path[MAX_PATH];
	for (i=0; strlen(TargetMaps[i].path) && i<MAXTARGETS; i++)
		;
	if (i==MAXTARGETS) return;
	path[0]=0;
	CFileDialog dlg( TRUE, "*.dxw", path, OFN_FILEMUSTEXIST,
        "dxwnd task config (*.dxw)|*.dxw|All Files (*.*)|*.*||",  this);
	if( dlg.DoModal() == IDOK){
		LV_ITEM listitem;
		CListCtrl& listctrl = GetListCtrl();
		
		LoadConfigItem(&TargetMaps[i], TitleMaps[i].title, 0, dlg.GetPathName().GetBuffer());

		listitem.mask = LVIF_TEXT | LVIF_IMAGE;
		listitem.iItem = i;
		listitem.iSubItem = 0;
		listitem.iImage = SetTargetIcon(TargetMaps[i]);
		if (strlen(TitleMaps[i].title)==0){
			int len;
			CString	FilePath;
			FilePath=TargetMaps[i].path;
			len=FilePath.ReverseFind('\\');	
			FilePath=FilePath.Right(FilePath.GetLength()-len-1);
			strncpy_s(TitleMaps[i].title, sizeof(TitleMaps[i].title), FilePath.GetString(), sizeof(TitleMaps[i].title)-1);
		}

		listitem.pszText = TitleMaps[i].title;
		listctrl.InsertItem(&listitem);
		Resize();
		SetTarget(TargetMaps);	
		this->isUpdated=TRUE;
	}
}

void CDxwndhostView::OnModify() 
{
	int i;
	CTargetDlg dlg;
	POSITION pos;
	LV_ITEM listitem;
	CListCtrl& listctrl = GetListCtrl();

	if(!listctrl.GetSelectedCount()) return;
	pos = listctrl.GetFirstSelectedItemPosition();
	i = listctrl.GetNextSelectedItem(pos);
	dlg.m_Title = TitleMaps[i].title;
	SetDlgFromTarget(&TargetMaps[i], &dlg);
	if(dlg.DoModal() == IDOK && dlg.m_FilePath.GetLength()){
		strncpy(TitleMaps[i].title, dlg.m_Title, 40);
		SetTargetFromDlg(&TargetMaps[i], &dlg);
		CListCtrl& listctrl = GetListCtrl();
		listitem.mask = LVIF_TEXT | LVIF_IMAGE;
		listitem.iItem = i;
		listitem.iSubItem = 0;
		listitem.iImage = SetTargetIcon(TargetMaps[i]);
		listitem.pszText = TitleMaps[i].title;
		listctrl.SetItem(&listitem);
		Resize();
		SetTarget(TargetMaps);	
		this->isUpdated=TRUE;
	}
}

void CDxwndhostView::OnExplore() 
{
	int i;
	CTargetDlg dlg;
	POSITION pos;
	int len;
	CString	FilePath;

	CListCtrl& listctrl = GetListCtrl();

	if(!listctrl.GetSelectedCount()) return;
	pos = listctrl.GetFirstSelectedItemPosition();
	i = listctrl.GetNextSelectedItem(pos);
	FilePath = TargetMaps[i].path;
	len=FilePath.ReverseFind('\\');	
	if (len==0) return;
	FilePath.Truncate(len);

	ShellExecute(NULL, "explore", FilePath, NULL, NULL, SW_SHOW);
}

void CDxwndhostView::OnViewLog() 
{
	int i;
	CTargetDlg dlg;
	POSITION pos;
	int len;
	CString	FilePath;

	CListCtrl& listctrl = GetListCtrl();

	if(!listctrl.GetSelectedCount()) return;
	pos = listctrl.GetFirstSelectedItemPosition();
	i = listctrl.GetNextSelectedItem(pos);
	FilePath = TargetMaps[i].path;
	len=FilePath.ReverseFind('\\');	
	if (len==0) return;
	FilePath.Truncate(len);
	FilePath.Append("\\dxwnd.log");

	ShellExecute(NULL, "open", FilePath, NULL, NULL, SW_SHOW);
}

void CDxwndhostView::OnDeleteLog() 
{
	int i;
	CTargetDlg dlg;
	POSITION pos;
	int len;
	CString	FilePath;
	HRESULT res;
	FILE *logfp;

	CListCtrl& listctrl = GetListCtrl();

	if(!listctrl.GetSelectedCount()) return;
	pos = listctrl.GetFirstSelectedItemPosition();
	i = listctrl.GetNextSelectedItem(pos);
	FilePath = TargetMaps[i].path;
	len=FilePath.ReverseFind('\\');	
	if (len==0) return;
	FilePath.Truncate(len);
	FilePath.Append("\\dxwnd.log");

	if((logfp=fopen(FilePath,"rb"))!=NULL){ // if the file exists ....
		fclose(logfp);
		// ... ask confirmation.
		res=MessageBoxEx(0, "Clear log file?", 
			"Warning", MB_YESNO | MB_ICONQUESTION, NULL);
		if(res!=IDYES) return;
	}

	_unlink(FilePath);
}

#define strcasecmp lstrcmpi

void CDxwndhostView::OnSort() 
{
	int i, itemcount;
	TARGETMAP MapEntry;
	PRIVATEMAP TitEntry;
	BOOL swapped;
	HRESULT res;
	CListCtrl& listctrl = GetListCtrl();

	// find list entries number and ignore the command when less than 2 
	for(i=0; ; i++) if(strlen(TargetMaps[i].path)==0) break;
	itemcount=i;
	if (itemcount<2) return;

	// ask for confirmation
	res=MessageBoxEx(0, "Sort the application list?", 
		"Warning", MB_YESNO | MB_ICONQUESTION, NULL);
	if(res!=IDYES) return;

	// bubble sort the list
	swapped=1;
	while(swapped){
		swapped=0;
		for(i=0; i<itemcount-1; i++){
			if(strcasecmp(TitleMaps[i].title, TitleMaps[i+1].title)>0){
				// swap entries
				MapEntry=TargetMaps[i];
				TargetMaps[i]=TargetMaps[i+1];
				TargetMaps[i+1]=MapEntry;
				TitEntry=TitleMaps[i];
				TitleMaps[i]=TitleMaps[i+1];
				TitleMaps[i+1]=TitEntry;
				swapped=1;
			}
		}
	}

	listctrl.DeleteAllItems();
	for(i=0; i<itemcount; i++) {
		LV_ITEM listitem;
		listitem.mask = LVIF_TEXT | LVIF_IMAGE;
		listitem.iItem = i;
		listitem.iSubItem = 0;
		listitem.iImage = SetTargetIcon(TargetMaps[i]);
		listitem.pszText = TitleMaps[i].title;
		listctrl.SetItem(&listitem);
		listctrl.InsertItem(&listitem);
	}

	SetTarget(TargetMaps);
	this->isUpdated=TRUE;
}

BOOL PauseResumeThreadList(DWORD dwOwnerPID, bool bResumeThread) 
{ 
    HANDLE        hThreadSnap = NULL; 
    BOOL          bRet        = FALSE; 
    THREADENTRY32 te32        = {0}; 
 
    // Take a snapshot of all threads currently in the system. 

    hThreadSnap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0); 
    if (hThreadSnap == INVALID_HANDLE_VALUE) 
        return (FALSE); 
 
    // Fill in the size of the structure before using it. 

    te32.dwSize = sizeof(THREADENTRY32); 
 
    // Walk the thread snapshot to find all threads of the process. 
    // If the thread belongs to the process, add its information 
    // to the display list.
 
    if (Thread32First(hThreadSnap, &te32)){ 
        do{ 
            if (te32.th32OwnerProcessID == dwOwnerPID){
				HANDLE hThread = OpenThread(THREAD_SUSPEND_RESUME, FALSE, te32.th32ThreadID);
				if (bResumeThread)
					ResumeThread(hThread);
				else
					SuspendThread(hThread);
				CloseHandle(hThread);
            } 
        }
        while (Thread32Next(hThreadSnap, &te32)); 
        bRet = TRUE; 
    } 
    else 
        bRet = FALSE;          // could not walk the list of threads 
 
    // Do not forget to clean up the snapshot object. 
    CloseHandle (hThreadSnap); 
 
    return (bRet); 
} 

void CDxwndhostView::OnPause() 
{
	CTargetDlg dlg;
	HRESULT res;
	char sMsg[128+1];
	DXWNDSTATUS DxWndStatus;
	if ((GetHookStatus(&DxWndStatus) != DXW_RUNNING) || (DxWndStatus.hWnd==NULL)) {
		MessageBoxEx(0, "No active task to pause.", "Info", MB_ICONEXCLAMATION, NULL);
	}
	else {
		sprintf_s(sMsg, 128, "Do you want to pause \nthe \"%s\" task?", TitleMaps[DxWndStatus.TaskIdx].title);
		res=MessageBoxEx(0, sMsg, "Warning", MB_YESNO | MB_ICONQUESTION, NULL);
		if(res!=IDYES) return;
		PauseResumeThreadList(DxWndStatus.dwPid, FALSE);
	}
}

void CDxwndhostView::OnResume() 
{
	CTargetDlg dlg;
	HRESULT res;
	char sMsg[128+1];
	DXWNDSTATUS DxWndStatus;
	if ((GetHookStatus(&DxWndStatus) != DXW_RUNNING) || (DxWndStatus.hWnd==NULL)) {
		MessageBoxEx(0, "No active task to resume.", "Info", MB_ICONEXCLAMATION, NULL);
	}
	else {
		sprintf_s(sMsg, 128, "Do you want to resume \nthe \"%s\" task?", TitleMaps[DxWndStatus.TaskIdx].title);
		res=MessageBoxEx(0, sMsg, "Warning", MB_YESNO | MB_ICONQUESTION, NULL);
		if(res!=IDYES) return;
		PauseResumeThreadList(DxWndStatus.dwPid, TRUE);
	}
}

void CDxwndhostView::OnKill() 
{
	CTargetDlg dlg;
	HRESULT res;
	HANDLE TargetHandle;
	char sMsg[128+1];
	DXWNDSTATUS DxWndStatus;
	if ((GetHookStatus(&DxWndStatus) != DXW_RUNNING) || (DxWndStatus.hWnd==NULL)) {
		MessageBoxEx(0, "No active task to kill.", "Info", MB_ICONEXCLAMATION, NULL);
	}
	else {
		sprintf_s(sMsg, 128, "Do you want to kill \nthe \"%s\" task?", TitleMaps[DxWndStatus.TaskIdx].title);
		res=MessageBoxEx(0, sMsg, "Warning", MB_YESNO | MB_ICONQUESTION, NULL);
		if(res!=IDYES) return;
		DxWndStatus.dwPid;

		TargetHandle = OpenProcess(PROCESS_TERMINATE, FALSE, DxWndStatus.dwPid);
		if(!TargetHandle){
			sprintf_s(sMsg, 128, "OpenProcess(%x) error %d, operation failed.", DxWndStatus.dwPid, GetLastError());
			MessageBoxEx(0, sMsg, "Error", MB_ICONEXCLAMATION, NULL);
			return;
		}
		if(!TerminateProcess(TargetHandle, 0)){
			sprintf_s(sMsg, 128, "TerminateProcess(%x) error %d, operation failed.", TargetHandle, GetLastError());
			MessageBoxEx(0, sMsg, "Error", MB_ICONEXCLAMATION, NULL);
			return;
		}
		CloseHandle(TargetHandle); 
		ClipCursor(NULL);
	}

	RevertScreenChanges(&this->InitDevMode);
}

void CDxwndhostView::OnProcessKill() 
{
	int i;
	POSITION pos;
	CListCtrl& listctrl = GetListCtrl();
	char FilePath[MAX_PATH+1];
	char *lpProcName, *lpNext;
	HRESULT res;
	char sMsg[128+1];

	if(!listctrl.GetSelectedCount()) return ;
	pos = listctrl.GetFirstSelectedItemPosition();
	i = listctrl.GetNextSelectedItem(pos);

	strncpy(FilePath,TargetMaps[i].path,MAX_PATH);
	sprintf_s(sMsg, 128, "Do you want to kill \nthe \"%s\" task?", TitleMaps[i].title);
	res=MessageBoxEx(0, sMsg, "Warning", MB_YESNO | MB_ICONQUESTION, NULL);
	if(res!=IDYES) return;

	lpProcName=FilePath;
	while (lpNext=strchr(lpProcName,'\\')) lpProcName=lpNext+1;

	KillProcByName(lpProcName);

	ClipCursor(NULL);
	RevertScreenChanges(&this->InitDevMode);
}

void CDxwndhostView::OnAdd() 
{
	// TODO: Please add a command handler code here.
	int i;
	CTargetDlg dlg;
	LV_ITEM listitem;

	dlg.m_DXVersion = 0;
	dlg.m_Coordinates = 0;
	dlg.m_MaxX = 0; //639;
	dlg.m_MaxY = 0; //479;
	dlg.m_DxEmulationMode = 3; // defaulting to EMULATIONMODE
	for(i = 0; i < MAXTARGETS; i ++) if(!TargetMaps[i].path[0]) break;
	if(i>=MAXTARGETS){
		MessageBoxEx(0, "Maximum entries number reached.\nDelete some entry to add a new one.", "Warning", MB_OK | MB_ICONEXCLAMATION, NULL);
		return;
	}
	if(dlg.DoModal() == IDOK && dlg.m_FilePath.GetLength()){
		strncpy(TitleMaps[i].title, dlg.m_Title, 40);
		SetTargetFromDlg(&TargetMaps[i], &dlg);
		CListCtrl& listctrl = GetListCtrl();
		listitem.mask = LVIF_TEXT | LVIF_IMAGE;
		listitem.iItem = i;
		listitem.iSubItem = 0;
		listitem.iImage = SetTargetIcon(TargetMaps[i]);
		if (strlen(TitleMaps[i].title)==0){
			int len;
			CString	FilePath;
			FilePath=TargetMaps[i].path;
			len=FilePath.ReverseFind('\\');	
			FilePath=FilePath.Right(FilePath.GetLength()-len-1);
			strncpy_s(TitleMaps[i].title, sizeof(TitleMaps[i].title), FilePath.GetString(), sizeof(TitleMaps[i].title)-1);
		}
		listitem.pszText = TitleMaps[i].title;
		listctrl.InsertItem(&listitem);
		Resize();
		SetTarget(TargetMaps);	
		this->isUpdated=TRUE;
	}
}

void CDxwndhostView::OnDelete() 
{
	int i, len;
	POSITION pos;
	CListCtrl& listctrl = GetListCtrl();
	CString	FilePath;
	HRESULT res;

	if(!listctrl.GetSelectedCount()) return ;
	pos = listctrl.GetFirstSelectedItemPosition();
	i = listctrl.GetNextSelectedItem(pos);

	FilePath=TitleMaps[i].title;
	if (FilePath.GetLength()==0){
		FilePath = TargetMaps[i].path;
		len=FilePath.ReverseFind('\\');	
		if (len==0) return;
		FilePath=FilePath.Right(FilePath.GetLength()-len-1);
	}

	res=MessageBoxEx(0, "Delete \""+FilePath+"\" ?", 
		"Warning", MB_YESNO | MB_ICONQUESTION, NULL);
	if(res!=IDYES) return;
	listctrl.DeleteItem(i);
	for(; i < MAXTARGETS  - 1; i ++) {
		TitleMaps[i] = TitleMaps[i + 1]; // V2.1.74 fix
		TargetMaps[i] = TargetMaps[i + 1];
	}
	Resize();
	SetTarget(TargetMaps);
	this->isUpdated=TRUE;
}

void CDxwndhostView::OnHookStart() 
{
	CMenu *menu;
	menu = this->GetParent()->GetMenu();
	menu->CheckMenuItem(ID_HOOK_START, MF_CHECKED);
	menu->CheckMenuItem(ID_HOOK_STOP , MF_UNCHECKED);

	StartHook();
	if(this->SystemTray.Enabled()) {
		UINT IdResource=IDI_DXWAIT;
		this->SystemTray.SetIcon(IdResource);
	}
}

void CDxwndhostView::OnHookStop() 
{
	CMenu *menu;
	// v.68: status control!!
	if (CheckStatus()) return;

	menu = this->GetParent()->GetMenu();
	menu->CheckMenuItem(ID_HOOK_START, MF_UNCHECKED);
	menu->CheckMenuItem(ID_HOOK_STOP , MF_CHECKED);
	
	EndHook();
	if(this->SystemTray.Enabled()) {
		UINT IdResource=IDI_DXIDLE;
		this->SystemTray.SetIcon(IdResource);
	}
}

void CDxwndhostView::OnClearAllLogs() 
{
	// TODO: Please add a command handler code here.
	int i, len;
	CListCtrl& listctrl = GetListCtrl();
	CString	FilePath;
	HRESULT res;

	res=MessageBoxEx(0, "Clear ALL logs?", 
		"Warning", MB_YESNO | MB_ICONQUESTION, NULL);
	if(res!=IDYES) return;

	for(i = 0; i < MAXTARGETS; i ++) {
		if(!TargetMaps[i].path[0]) break;
		TargetMaps[i].tflags &= ~OUTTRACE;
		FilePath = TargetMaps[i].path;
		len=FilePath.ReverseFind('\\');	
		if (len==0) continue;
		FilePath.Truncate(len);
		FilePath.Append("\\dxwnd.log");
		_unlink(FilePath);
	}
	// v2.1.92: clean %TEMP% folder
	FilePath = getenv("TEMP");
	FilePath.Append("\\dxwnd.log");
	_unlink(FilePath);
}


DWORD WINAPI TrayIconUpdate(CSystemTray *Tray)
{
	int DxStatus, PrevDxStatus;
	UINT IconId;
	char sMsg[1024];
	char *Status;
	char DllVersion[21];
	int TickCount;
	PrevDxStatus=-1; // a different one...
	TickCount=0;
	while (TRUE) {
		// once a second ...
		Sleep(1000);
		DxStatus=GetHookStatus(NULL);
		switch (DxStatus){
			case DXW_IDLE: IconId=IDI_DXIDLE; Status="DISABLED"; break;
			case DXW_ACTIVE: IconId=IDI_DXWAIT; Status="READY"; break;
			case DXW_RUNNING: IconId=IDI_DXRUN; Status="RUNNING"; break;
			default: IconId=IDI_DXIDLE; Status="???"; break;
		}
		if (DxStatus != DXW_RUNNING){
			TickCount=0;
			Tray->StopAnimation();
			Tray->SetIcon(IconId);
		}
		else {
			// animation state machine ....
			TickCount++;
			if (DxStatus!=PrevDxStatus) {
				Tray->SetIcon(IconId);
			}
			if(TickCount > 4){
				Tray->SetIconList(IDI_RUN1, IDI_RUN6);
				Tray->Animate(50,-1);
			}
			if(TickCount > 6){
				TickCount=0;
				Tray->StopAnimation();
				Tray->SetIcon(IDI_RUN0);
			}
		}
		DXWNDSTATUS DxWndStatus;
		GetDllVersion(DllVersion);
		DxWndStatus.Status=DxStatus;
		if(DxStatus==DXW_RUNNING){
			GetHookStatus(&DxWndStatus);
			sprintf_s(sMsg, 1024, 
				"Running \"%s\"\nScreen = (%dx%d) %dBPP\nFullScreen = %s\nDX version = %d", 
			pTitles[DxWndStatus.TaskIdx].title,
			DxWndStatus.Width, DxWndStatus.Height, DxWndStatus.ColorDepth, 
			DxWndStatus.IsFullScreen ? "Yes":"No", DxWndStatus.DXVersion);    
		}
		else
			sprintf_s(sMsg, 1024, "DxWnd %s\nHook status: %s", DllVersion, Status);
		Tray->SetTooltipText(sMsg);
		PrevDxStatus=DxStatus;
	}
}

void CDxwndhostView::OnGoToTrayIcon() 
{
	CWnd *pParent = this->GetParent();
	CMenu *menu = this->GetParent()->GetMenu();
	UINT IconId;
	if(!this->SystemTray.Enabled()){
		HANDLE StatusThread;
		DWORD dwThrdId;
		// Create the tray icon
		if(!this->SystemTray.Create(NULL, 
			WM_ICON_NOTIFY, "DxWnd", NULL, IDR_MENU_SYSTRAY,
			FALSE,
			m_StartToTray ? NULL : "DxWnd DirectDraw windowizer", // NULL inhibits the Tray banner
			"DxWnd", 
			NIIF_INFO, 10)){
			MessageBox(0, "SystemTray.Create failed", MB_OK);
			return;
		}
		IconId=(menu->GetMenuState(ID_HOOK_START, MF_BYCOMMAND)==MF_CHECKED)?IDI_DXWAIT:IDI_DXIDLE;
		if(!this->SystemTray.SetIcon(IconId)){
			MessageBox(0, "SystemTray.LoadIcon failed", MB_OK);
			return;
		}
		StatusThread= CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)TrayIconUpdate, (LPVOID)&this->SystemTray, 0, &dwThrdId);
	}
	this->SystemTray.ShowIcon();
    this->SystemTray.SetMenuDefaultItem(0, TRUE);
	this->SystemTray.SetTargetWnd(pParent);		// Send all menu messages here.
	this->SystemTray.MinimiseToTray(pParent, FALSE);
}

void CDxwndhostView::OnSaveFile() 
{
	if (this->isUpdated) 
	if (MessageBoxEx(0, 
		"Task list has changed.\n"
		"Do you want to save it?", 
		"Warning", MB_YESNO | MB_ICONQUESTION, NULL)==IDYES) 
	this->SaveConfigFile();
}

void CDxwndhostView::OnTrayRestore() 
{
	CWnd *pParent = this->GetParent();
	this->SystemTray.MaximiseFromTray(pParent, FALSE);
}

void CDxwndhostView::OnViewStatus()
{
	CStatusDialog *pDlg = new CStatusDialog();
	BOOL ret = pDlg->Create(CStatusDialog::IDD, this); 
	pDlg->ShowWindow(SW_SHOW);
}

void CDxwndhostView::OnViewPalette()
{
	CPaletteDialog *pDlg = new CPaletteDialog();
	BOOL ret = pDlg->Create(CPaletteDialog::IDD, this); 
	pDlg->ShowWindow(SW_SHOW);
}

void CDxwndhostView::OnViewTimeSlider()
{
	CTimeSliderDialog *pDlg = new CTimeSliderDialog();
	BOOL ret = pDlg->Create(CTimeSliderDialog::IDD, this); 
	pDlg->ShowWindow(SW_SHOW);
}

void CDxwndhostView::Resize()
{
	CListCtrl& listctrl = GetListCtrl();
	LV_COLUMN listcol;
	int i, tmp, size = 0;
	
	for(i = 0; i < MAXTARGETS; i ++){
		tmp = listctrl.GetStringWidth(TargetMaps[i].path);
		if(size < tmp) size = tmp;
	}
	
	listcol.mask = LVCF_WIDTH;
	listcol.cx = size + 10;
	listctrl.SetColumn(0, &listcol);
}

void CDxwndhostView::OnRButtonDown(UINT nFlags, CPoint point) 
{
	CMenu popup;
	int res;
	
	ClientToScreen(&point);
	popup.LoadMenu(IDR_MENU_POPUP);
	res = popup.GetSubMenu(0)->TrackPopupMenu(TPM_LEFTALIGN | TPM_TOPALIGN | TPM_LEFTBUTTON | TPM_RETURNCMD, point.x, point.y, this);
	switch(res){
	case ID_PRUN:
		OnRun();
		break;
	case ID_PMODIFY:
		OnModify();
		break;
	case ID_PDELETE:
		OnDelete();
		break;
	case ID_PADD:
		OnAdd();
		break;
	case ID_PEXPLORE:
		OnExplore();
		break;
	case ID_PLOG_VIEW:
		OnViewLog();
		break;
	case ID_PLOG_DELETE:
		OnDeleteLog();
		break;
	case ID_TASK_KILL:
		OnKill();
		break;
	case ID_PKILL:
		OnProcessKill();
		break;
	case ID_TASK_PAUSE:
		OnPause();
		break;
	case ID_TASK_RESUME:
		OnResume();
		break;
	case ID_PEXPORT:
		OnExport();
		break;
	case ID_FILE_IMPORT:
		OnImport();
		break;
	case ID_DESKTOPCOLORDEPTH_8BPP:
		OnDesktopcolordepth8bpp();
		break;
	case ID_DESKTOPCOLORDEPTH_16BPP:
		OnDesktopcolordepth16bpp();
		break;
	case ID_DESKTOPCOLORDEPTH_24BPP:
		OnDesktopcolordepth24bpp();
		break;
	case ID_DESKTOPCOLORDEPTH_32BPP:
		OnDesktopcolordepth32bpp();
		break;
	}
	CListView::OnRButtonDown(nFlags, point);
}

// For thread messaging
#define DEBUG_EVENT_MESSAGE		WM_APP + 0x100

HWND Ghwnd;

DWORD WINAPI StartDebug(void *p)
{
	TARGETMAP *TargetMap;
	STARTUPINFO sinfo;
	PROCESS_INFORMATION pinfo, *pi;
	CREATE_THREAD_DEBUG_INFO *ti;
	LOAD_DLL_DEBUG_INFO *li;
	char path[MAX_PATH];
	BOOL step=FALSE; // initialize to TRUE to enable
	extern char *GetFileNameFromHandle(HANDLE);

	TargetMap=(TARGETMAP *)p;
	ZeroMemory(&sinfo, sizeof(sinfo));
	sinfo.cb = sizeof(sinfo);
	strcpy_s(path, sizeof(path), TargetMap->path);
	PathRemoveFileSpec(path);
	CreateProcess(NULL, TargetMap->path, 0, 0, false, DEBUG_ONLY_THIS_PROCESS, NULL, path, &sinfo, &pinfo);
	CString strEventMessage;
	DEBUG_EVENT debug_event ={0};
	bool bContinueDebugging = true;
	DWORD dwContinueStatus = DBG_CONTINUE;
	while(bContinueDebugging)
	{ 
		int res;
		char DebugMessage[256+1];
		if (!WaitForDebugEvent(&debug_event, INFINITE)) return TRUE;
		switch(debug_event.dwDebugEventCode){
		case EXIT_PROCESS_DEBUG_EVENT:
			SetWindowText(Ghwnd, "EXIT PROCESS");
			bContinueDebugging=false;
			break;
		case CREATE_PROCESS_DEBUG_EVENT:
			if(step){
				pi=(PROCESS_INFORMATION *)&debug_event.u;
				sprintf(DebugMessage, "CREATE PROCESS hProcess=%x dwProcessId=%x path=%s", 
					pi->hProcess, pi->dwProcessId, GetFileNameFromHandle(pi->hProcess));
				res=MessageBoxEx(0, DebugMessage, "Continue stepping?", MB_YESNO | MB_ICONQUESTION, NULL);
				if(res!=IDYES) step=FALSE;
			}
			GetFullPathName("dxwnd.dll", MAX_PATH, path, NULL);
			if(!Inject(pinfo.dwProcessId, path)){
				sprintf(DebugMessage,"Injection error: pid=%x dll=%s", pinfo.dwProcessId, path);
				MessageBoxEx(0, DebugMessage, "Injection", MB_ICONEXCLAMATION, NULL);
			}
			break;
		case CREATE_THREAD_DEBUG_EVENT:
			if(step){
				ti=(CREATE_THREAD_DEBUG_INFO *)&debug_event.u;
				sprintf(DebugMessage, "CREATE THREAD hThread=%x lpThreadLocalBase=%x lpStartAddress=%x", 
					ti->hThread, ti->lpThreadLocalBase, ti->lpStartAddress);
				res=MessageBoxEx(0, DebugMessage, "Continue stepping?", MB_YESNO | MB_ICONQUESTION, NULL);
				if(res!=IDYES) step=FALSE;
			}
			break;
		case EXIT_THREAD_DEBUG_EVENT:
			SetWindowText(Ghwnd, "EXIT THREAD");
			break;
		case LOAD_DLL_DEBUG_EVENT:
			if(step){
				li=(LOAD_DLL_DEBUG_INFO *)&debug_event.u;
				sprintf(DebugMessage, "LOAD DLL hFile=%x path=%s", 
					li->hFile, GetFileNameFromHandle(li->hFile));
				res=MessageBoxEx(0, DebugMessage, "Continue stepping?", MB_YESNO | MB_ICONQUESTION, NULL);
				if(res!=IDYES) step=FALSE;

			}
			//li=(LOAD_DLL_DEBUG_INFO *)&debug_event.u;
			//if(strstr(GetFileNameFromHandle(li->hFile), "ddraw.dll")){
			//	res=MessageBoxEx(0, GetFileNameFromHandle(li->hFile), "ddraw.dll intercepted", MB_OK, NULL);
			//	GetFullPathName("dxwnd.dll", MAX_PATH, path, NULL);
			//	if(!Inject(pinfo.dwProcessId, path)){
			//		sprintf(DebugMessage,"Injection error: pid=%x dll=%s", pinfo.dwProcessId, path);
			//		MessageBoxEx(0, DebugMessage, "Injection", MB_ICONEXCLAMATION, NULL);
			//	}
			//}
			break;
		case UNLOAD_DLL_DEBUG_EVENT:
			SetWindowText(Ghwnd, "UNLOAD DLL");
			break;
		case OUTPUT_DEBUG_STRING_EVENT: 				
			SetWindowText(Ghwnd, "OUT STRING");
			break;
		case EXCEPTION_DEBUG_EVENT:
			SetWindowText(Ghwnd, "EXCEPTION");
			break;
		default:
			break;
		}
		SendMessage(Ghwnd, DEBUG_EVENT_MESSAGE, (WPARAM) &strEventMessage, debug_event.dwDebugEventCode);
		ContinueDebugEvent(debug_event.dwProcessId, 
			debug_event.dwThreadId, 
			dwContinueStatus);
		// Reset
		dwContinueStatus = DBG_CONTINUE;
	}
	return TRUE;
}

void CDxwndhostView::OnRun() 
{
	CListCtrl& listctrl = GetListCtrl();
	POSITION pos;
	int i;
	STARTUPINFO sinfo;
	PROCESS_INFORMATION pinfo;
	char path[MAX_PATH];
	//extern CString GetFileNameFromHandle(HANDLE);

	if(!listctrl.GetSelectedCount()) return;
	pos = listctrl.GetFirstSelectedItemPosition();
	i = listctrl.GetNextSelectedItem(pos);
	ZeroMemory(&sinfo, sizeof(sinfo));
	sinfo.cb = sizeof(sinfo);
	strcpy_s(path, sizeof(path), TargetMaps[i].path);
	PathRemoveFileSpec(path);
	if(TargetMaps[i].flags2 & STARTDEBUG){
		Ghwnd=this->m_hWnd;
		CreateThread( NULL, 0, StartDebug, &TargetMaps[i], 0, NULL); 
	}
	else{
		CreateProcess(NULL, TargetMaps[i].path, 0, 0, false, CREATE_DEFAULT_ERROR_MODE, NULL, path, &sinfo, &pinfo);
	}
}

void SwitchToColorDepth(int bpp)
{
	DEVMODE CurrentDevMode;
	BOOL res;
	char MsgBuf[256+1];
	EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &CurrentDevMode);
	sprintf(MsgBuf, "ChangeDisplaySettings: color depth %d -> %d BPP\n", CurrentDevMode.dmBitsPerPel, bpp);
	if(MessageBoxEx(0, MsgBuf, "Warning", MB_OKCANCEL | MB_ICONQUESTION, NULL)!=IDOK) return;
	//OutTraceDW("ChangeDisplaySettings: CURRENT wxh=(%dx%d) BitsPerPel=%d -> 16\n", 
	//	CurrentDevMode.dmPelsWidth, CurrentDevMode.dmPelsHeight, CurrentDevMode.dmBitsPerPel);
	CurrentDevMode.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;
	CurrentDevMode.dmBitsPerPel = bpp;
	res=ChangeDisplaySettings(&CurrentDevMode, CDS_UPDATEREGISTRY);
	if(res!=DISP_CHANGE_SUCCESSFUL) {
		char *err;
		switch(res){
			case DISP_CHANGE_BADDUALVIEW: err="BADDUALVIEW"; break;
			case DISP_CHANGE_BADFLAGS: err="BADFLAGS"; break;
			case DISP_CHANGE_BADMODE: err="BADMODE"; break;
			case DISP_CHANGE_BADPARAM: err="BADPARAM"; break;
			case DISP_CHANGE_FAILED: err="FAILED"; break;
			case DISP_CHANGE_NOTUPDATED: err="NOTUPDATED"; break;
			case DISP_CHANGE_RESTART: err="RESTART"; break;
			default: err="???"; break;
		}
		sprintf(MsgBuf, "ChangeDisplaySettings ERROR res=%s err=%d\n", err, GetLastError());
		MessageBoxEx(0, MsgBuf, "Error", MB_OKCANCEL | MB_ICONQUESTION, NULL);
	}
}

void CDxwndhostView::OnDesktopcolordepth8bpp()
{
	// TODO: Add your command handler code here
	SwitchToColorDepth(8);
}

void CDxwndhostView::OnDesktopcolordepth16bpp()
{
	// TODO: Add your command handler code here
	SwitchToColorDepth(16);
}

void CDxwndhostView::OnDesktopcolordepth24bpp()
{
	// TODO: Add your command handler code here
	SwitchToColorDepth(24);
}

void CDxwndhostView::OnDesktopcolordepth32bpp()
{
	// TODO: Add your command handler code here
	SwitchToColorDepth(32);
}
