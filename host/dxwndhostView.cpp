// dxwndhostView.cpp : CDxwndhostView Define the behavior of the class.
//

#include "stdafx.h"
#include "shlwapi.h"
#include "TlHelp32.h"
#include "CoolUtils.h"

#include "dxwndhost.h"

#include "dxwndhostDoc.h"
#include "dxwndhostView.h"
#include "TargetDlg.h"
#include "SystemTray.h"
#include "StatusDialog.h"
#include "DesktopDialog.h"
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
extern int KillProcByName(char *, BOOL);

PRIVATEMAP *pTitles; // global ptr: get rid of it!!
TARGETMAP *pTargets; // idem.

#define LOCKINJECTIONTHREADS

char gInitPath[MAX_PATH]; // don't put it into the class because it must be used after destructor

// beware: it must operate upon count+1 sized arrays
char *strnncpy(char *dest, char *src, size_t count)
{
	dest[count]=0;
	return strncpy(dest, src, count);
}

static char *Escape(char *s, char **dest)
{
	if(!*dest)	*dest=(char *)malloc(strlen(s)+100);
	else		*dest=(char *)realloc(*dest, strlen(s)+100); 
	char *t = *dest;
	for(; *s; s++){
		switch(*s){
			case '\n':
				*t++ = '\\';
				*t++ = 'n';
				break;
			case '\r':
				break;
			default:
				*t++ = *s;
				break;
		}
	}
	*t=0;
	return *dest;
}

static char *Unescape(char *s, char **dest)
{
	if(!*dest)	*dest=(char *)malloc(strlen(s)+100);
	else		*dest=(char *)realloc(*dest, strlen(s)+100); 
	char *t = *dest;
	for(; *s; s++){
		if((*s=='\\') && (*(s+1)=='n')){
			*t++ = '\r';
			*t++ = '\n';
			s++;
		}
		else{
			*t++ = *s;
		}
	}
	*t=0;
	return *dest;
}


void GetFolderFromPath(char *path)
{
	for(char *c=&path[strlen(path)-1]; (c>path) && (*c!='\\'); c--) *c=0;
}

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
	ON_COMMAND(ID_WINDOW_MINIMIZE, OnWindowMinimize)
	ON_COMMAND(ID_WINDOW_RESTORE, OnWindowRestore)
	ON_COMMAND(ID_WINDOW_CLOSE, OnWindowClose)
	ON_COMMAND(ID_WINDOW_MINIMIZE, OnWindowMinimize)
	ON_COMMAND(ID_TASKBAR_HIDE, OnTaskbarHide)
	ON_COMMAND(ID_TASKBAR_SHOW, OnTaskbarShow)
	ON_COMMAND(ID_MODIFY, OnModify)
	ON_COMMAND(ID_ADD, OnAdd)
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
	ON_COMMAND(ID_VIEW_DESKTOP, OnViewDesktop)
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
		MessageBoxLang(DXW_STRING_WAITTASK, DXW_STRING_WARNING, MB_OK | MB_ICONEXCLAMATION);
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
		if (MessageBoxLang(DXW_STRING_RESTORE, DXW_STRING_WARNING, MB_YESNO | MB_ICONQUESTION)==IDYES) 
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
	t->flags5 = 0;
	t->flags6 = 0;
	t->tflags = 0;
	if(dlg->m_UnNotify) t->flags |= UNNOTIFY;
	if(dlg->m_Windowize) t->flags2 |= WINDOWIZE;
	if(dlg->m_HookDLLs) t->flags3 |= HOOKDLLS;
	if(dlg->m_TerminateOnClose) t->flags6 |= TERMINATEONCLOSE;
	if(dlg->m_ConfirmOnClose) t->flags6 |= CONFIRMONCLOSE;
	if(dlg->m_EmulateRegistry) t->flags3 |= EMULATEREGISTRY;
	if(dlg->m_OverrideRegistry) t->flags4 |= OVERRIDEREGISTRY;
	if(dlg->m_Wow64Registry) t->flags6 |= WOW64REGISTRY;
	if(dlg->m_Wow32Registry) t->flags6 |= WOW32REGISTRY;
	if(dlg->m_HookEnabled) t->flags3 |= HOOKENABLED;
	if(dlg->m_NoBanner) t->flags2 |= NOBANNER;
	if(dlg->m_StartDebug) t->flags2 |= STARTDEBUG;
	if(dlg->m_HotPatch) t->flags4 |= HOTPATCH;
	if(dlg->m_FullScreenOnly) t->flags3 |= FULLSCREENONLY;
	if(dlg->m_FilterMessages) t->flags3 |= FILTERMESSAGES;
	if(dlg->m_PeekAllMessages) t->flags3 |= PEEKALLMESSAGES;
	if(dlg->m_NoWinPosChanges) t->flags5 |= NOWINPOSCHANGES;
	if(dlg->m_MessagePump) t->flags5 |= MESSAGEPUMP;

	switch(dlg->m_DxEmulationMode){
		//case 0: t->flags |= AUTOMATIC; break;
		case 0: break;
		case 1: t->flags |= EMULATEBUFFER; break;
		case 2: t->flags |= LOCKEDSURFACE; break;
		case 3: t->flags |= EMULATESURFACE; break;
		case 4: t->flags5 |= HYBRIDMODE; break;
		case 5: t->flags5 |= GDIMODE; break;
			break;
	}

	switch(dlg->m_DxFilterMode){
		case 0: break;
		case 1: t->flags4 |= BILINEAR2XFILTER; break;
		case 2: t->flags5 |= BILINEARFILTER; break;
	}

	switch(dlg->m_DCEmulationMode){
		case 0: break;
		case 1: t->flags2 |= GDISTRETCHED; break;
		case 2: t->flags3 |= GDIEMULATEDC; break;
		//case 3: t->flags |= MAPGDITOPRIMARY; break;
	}

	switch(dlg->m_ResTypes){
		case 0: t->flags4 |= SUPPORTSVGA; break;
		case 1: t->flags4 |= SUPPORTHDTV; break;
		case 2: t->flags4 |= NATIVERES; break;
	}	

	switch(dlg->m_MouseVisibility){
		case 0: break;
		case 1: t->flags |= HIDEHWCURSOR; break;
		case 2: t->flags2 |= SHOWHWCURSOR; break;
	}	

	switch(dlg->m_TextureHandling){
		case 0: break;
		case 1: t->flags5 |= TEXTUREHIGHLIGHT; break;
		case 2: t->flags5 |= TEXTUREDUMP; break;
		case 3: t->flags5 |= TEXTUREHACK; break;
	}

	switch(dlg->m_SonProcessMode){
		case 0: break;
		case 1: t->flags4 |= SUPPRESSCHILD; break;
		case 2: t->flags5 |= ENABLESONHOOK; break;
		case 3: t->flags5 |= INJECTSON; break;
	}	

	if(dlg->m_HookDI) t->flags |= HOOKDI;
	if(dlg->m_HookDI8) t->flags |= HOOKDI8;
	if(dlg->m_ModifyMouse) t->flags |= MODIFYMOUSE;
	if(dlg->m_VirtualJoystick) t->flags6 |= VIRTUALJOYSTICK;
	if(dlg->m_Unacquire) t->flags6 |= UNACQUIRE;
	if(dlg->m_OutProxyTrace) t->tflags |= OUTPROXYTRACE;
	if(dlg->m_OutDebug) t->tflags |= OUTDEBUG;
	if(dlg->m_CursorTrace) t->tflags |= OUTCURSORTRACE;
	if(dlg->m_LogEnabled) t->tflags |= OUTTRACE;
	if(dlg->m_OutDebugString) t->tflags |= OUTDEBUGSTRING;
	if(dlg->m_EraseLogFile) t->tflags |= ERASELOGFILE;
	if(dlg->m_AddTimeStamp) t->tflags |= ADDTIMESTAMP;
	if(dlg->m_OutWinMessages) t->tflags |= OUTWINMESSAGES;
	if(dlg->m_OutDWTrace) t->tflags |= OUTDXWINTRACE;
	if(dlg->m_OutDDRAWTrace) t->tflags |= OUTDDRAWTRACE;
	if(dlg->m_OutD3DTrace) t->tflags |= OUTD3DTRACE;
	if(dlg->m_DXProxed) t->tflags |= DXPROXED;
	if(dlg->m_AssertDialog) t->tflags |= ASSERTDIALOG;
	if(dlg->m_ImportTable) t->tflags |= OUTIMPORTTABLE;
	if(dlg->m_RegistryOp) t->tflags |= OUTREGISTRY;
	if(dlg->m_TraceHooks) t->tflags |= TRACEHOOKS;
	if(dlg->m_HandleExceptions) t->flags |= HANDLEEXCEPTIONS;
	if(dlg->m_LimitResources) t->flags2 |= LIMITRESOURCES;
	if(dlg->m_CDROMDriveType) t->flags3 |= CDROMDRIVETYPE;
	if(dlg->m_HideCDROMEmpty) t->flags4 |= HIDECDROMEMPTY;
	if(dlg->m_HookGOGLibs) t->flags6 |= HOOKGOGLIBS;
	if(dlg->m_BypassGOGLibs) t->flags6 |= BYPASSGOGLIBS;
	if(dlg->m_FontBypass) t->flags3 |= FONTBYPASS;
	if(dlg->m_BufferedIOFix) t->flags3 |= BUFFEREDIOFIX;
	if(dlg->m_ZBufferClean) t->flags4 |= ZBUFFERCLEAN;
	if(dlg->m_ZBuffer0Clean) t->flags4 |= ZBUFFER0CLEAN;
	if(dlg->m_ZBufferAlways) t->flags4 |= ZBUFFERALWAYS;
	if(dlg->m_HotPatchAlways) t->flags4 |= HOTPATCHALWAYS;
	if(dlg->m_FreezeInjectedSon) t->flags5 |= FREEZEINJECTEDSON;
	if(dlg->m_StressResources) t->flags5 |= STRESSRESOURCES;
	if(dlg->m_NoPower2Fix) t->flags4 |= NOPOWER2FIX;
	if(dlg->m_NoPerfCounter) t->flags4 |= NOPERFCOUNTER;
	if(dlg->m_DisableFogging) t->flags4 |= DISABLEFOGGING;
	if(dlg->m_ClearTarget) t->flags5 |= CLEARTARGET;
	if(dlg->m_FixPitch) t->flags6 |= FIXPITCH;
	if(dlg->m_Power2Width) t->flags6 |= POWER2WIDTH;
	if(dlg->m_SuppressIME) t->flags2 |= SUPPRESSIME;
	if(dlg->m_SuppressD3DExt) t->flags3 |= SUPPRESSD3DEXT;
	if(dlg->m_SetCompatibility) t->flags2 |= SETCOMPATIBILITY;
	if(dlg->m_AEROBoost) t->flags5 |= AEROBOOST;
	if(dlg->m_DiabloTweak) t->flags5 |= DIABLOTWEAK;
	if(dlg->m_EASportsHack) t->flags5 |= EASPORTSHACK;
	if(dlg->m_LegacyAlloc) t->flags6 |= LEGACYALLOC;
	if(dlg->m_DisableMaxWinMode) t->flags6 |= DISABLEMAXWINMODE;
	if(dlg->m_NoImagehlp) t->flags5 |= NOIMAGEHLP;
	if(dlg->m_ForcesHEL) t->flags3 |= FORCESHEL;
	if(dlg->m_SetZBufferBitDepths) t->flags6 |= SETZBUFFERBITDEPTHS;
	if(dlg->m_ForcesSwapEffect) t->flags6 |= FORCESWAPEFFECT;
	if(dlg->m_ColorFix) t->flags3 |= COLORFIX;
	if(dlg->m_NoPixelFormat) t->flags3 |= NOPIXELFORMAT;
	if(dlg->m_NoAlphaChannel) t->flags4 |= NOALPHACHANNEL;
	if(dlg->m_FixRefCounter) t->flags4 |= FIXREFCOUNTER;
	if(dlg->m_ReturnNullRef) t->flags4 |= RETURNNULLREF;
	if(dlg->m_NoD3DReset) t->flags4 |= NOD3DRESET;
	if(dlg->m_HideDesktop) t->flags4 |= HIDEDESKTOP;
	if(dlg->m_HideTaskbar) t->flags6 |= HIDETASKBAR;
	if(dlg->m_ActivateApp) t->flags6 |= ACTIVATEAPP;
	if(dlg->m_UnlockZOrder) t->flags5 |= UNLOCKZORDER;
	if(dlg->m_NoDestroyWindow) t->flags6 |= NODESTROYWINDOW;
	if(dlg->m_LockSysColors) t->flags3 |= LOCKSYSCOLORS;
	if(dlg->m_LockReservedPalette) t->flags5 |= LOCKRESERVEDPALETTE;
	if(dlg->m_ForceYUVtoRGB) t->flags3 |= YUV2RGB;
	if(dlg->m_ForceRGBtoYUV) t->flags3 |= RGB2YUV;
	if(dlg->m_LimitScreenRes) t->flags4 |= LIMITSCREENRES;
	if(dlg->m_SingleProcAffinity) t->flags3 |= SINGLEPROCAFFINITY;
	if(dlg->m_SaveLoad) t->flags |= SAVELOAD;
	if(dlg->m_SlowDown) t->flags |= SLOWDOWN;
	if(dlg->m_BlitFromBackBuffer) t->flags |= BLITFROMBACKBUFFER;
	if(dlg->m_NoFlipEmulation) t->flags4 |= NOFLIPEMULATION;
	if(dlg->m_SuppressClipping) t->flags |= SUPPRESSCLIPPING;
	if(dlg->m_DisableGammaRamp) t->flags2 |= DISABLEGAMMARAMP;
	if(dlg->m_AutoRefresh) t->flags |= AUTOREFRESH;
	if(dlg->m_TextureFormat) t->flags5 |= TEXTUREFORMAT;
	if(dlg->m_FixWinFrame) t->flags |= FIXWINFRAME;
	if(dlg->m_EnableClipping) t->flags |= ENABLECLIPPING;
	if(dlg->m_CursorClipping) t->flags |= CLIPCURSOR;
	if(dlg->m_VideoToSystemMem) t->flags |= SWITCHVIDEOMEMORY;
	if(dlg->m_FixTextOut) t->flags |= FIXTEXTOUT;
	if(dlg->m_SharedDC) t->flags6 |= SHAREDDC;
	if(dlg->m_HookGlide) t->flags4 |= HOOKGLIDE;
	if(dlg->m_RemapMCI) t->flags5 |= REMAPMCI;
	if(dlg->m_NoMovies) t->flags6 |= NOMOVIES;
	if(dlg->m_FixMoviesColor) t->flags6 |= FIXMOVIESCOLOR;
	if(dlg->m_StretchMovies) t->flags6 |= STRETCHMOVIES;
	if(dlg->m_BypassMCI) t->flags6 |= BYPASSMCI;
	if(dlg->m_SuppressRelease) t->flags6 |= SUPPRESSRELEASE;
	if(dlg->m_KeepCursorWithin) t->flags |= KEEPCURSORWITHIN;
	if(dlg->m_KeepCursorFixed) t->flags2 |= KEEPCURSORFIXED;
	if(dlg->m_UseRGB565) t->flags |= USERGB565;
	if(dlg->m_SuppressDXErrors) t->flags |= SUPPRESSDXERRORS;
	if(dlg->m_FlipEmulation) t->flags6 |= FLIPEMULATION;
	if(dlg->m_MarkBlit) t->flags3 |= MARKBLIT;
	if(dlg->m_MarkLock) t->flags3 |= MARKLOCK;
	if(dlg->m_NoSysMemPrimary) t->flags6 |= NOSYSMEMPRIMARY;
	if(dlg->m_NoSysMemBackBuf) t->flags6 |= NOSYSMEMBACKBUF;
	if(dlg->m_NoBlt) t->flags5 |= NOBLT;
	if(dlg->m_FastBlt) t->flags5 |= DOFASTBLT;
	if(dlg->m_GDIColorConv) t->flags5 |= GDICOLORCONV;
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
	//if(dlg->m_NoMouseProc) t->flags6 |= NOMOUSEPROC;
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
	if(dlg->m_StretchTimers) t->flags4 |= STRETCHTIMERS;
	if(dlg->m_NormalizePerfCount) t->flags5 |= NORMALIZEPERFCOUNT;
	if(dlg->m_QuarterBlt) t->flags5 |= QUARTERBLT;
	if(dlg->m_FineTiming) t->flags4 |= FINETIMING;
	if(dlg->m_ReleaseMouse) t->flags4 |= RELEASEMOUSE;
	if(dlg->m_EnableTimeFreeze) t->flags4 |= ENABLETIMEFREEZE;
	if(dlg->m_EnableHotKeys) t->flags4 |= ENABLEHOTKEYS;
	if(dlg->m_InterceptRDTSC) t->flags4 |= INTERCEPTRDTSC;
	if(dlg->m_HookOpenGL) t->flags2 |= HOOKOPENGL;
	if(dlg->m_ForceHookOpenGL) t->flags3 |= FORCEHOOKOPENGL;
	if(dlg->m_FixPixelZoom) t->flags6 |= FIXPIXELZOOM;
	if(dlg->m_WireFrame) t->flags2 |= WIREFRAME;
	if(dlg->m_NoTextures) t->flags4 |= NOTEXTURES;
	if(dlg->m_BlackWhite) t->flags3 |= BLACKWHITE;
	if(dlg->m_FakeVersion) t->flags2 |= FAKEVERSION;
	if(dlg->m_FullRectBlt) t->flags2 |= FULLRECTBLT;
	if(dlg->m_CenterToWin) t->flags5 |= CENTERTOWIN;
	if(dlg->m_Deinterlace) t->flags5 |= DEINTERLACE;
	if(dlg->m_NoPaletteUpdate) t->flags2 |= NOPALETTEUPDATE;
	if(dlg->m_SurfaceWarn) t->flags3 |= SURFACEWARN;
	if(dlg->m_CapMask) t->flags3 |= CAPMASK;
	if(dlg->m_NoWindowHooks) t->flags6 |= NOWINDOWHOOKS;
	if(dlg->m_NoDDRAWBlt) t->flags3 |= NODDRAWBLT;
	if(dlg->m_NoDDRAWFlip) t->flags3 |= NODDRAWFLIP;
	if(dlg->m_NoGDIBlt) t->flags3 |= NOGDIBLT;
	if(dlg->m_NoFillRect) t->flags4 |= NOFILLRECT;
	if(dlg->m_ReuseEmulatedDC) t->flags6 |= REUSEEMULATEDDC;
	if(dlg->m_CreateDesktop) t->flags6 |= CREATEDESKTOP;
	if(dlg->m_SyncPalette) t->flags6 |= SYNCPALETTE;
	if(dlg->m_AnalyticMode) t->flags3 |= ANALYTICMODE;
	if(dlg->m_ReplacePrivOps) t->flags5 |= REPLACEPRIVOPS;
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
	t->MaxScreenRes = dlg->m_MaxScreenRes;
	t->SwapEffect = dlg->m_SwapEffect;
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
	dlg->m_HotPatch = t->flags4 & HOTPATCH ? 1 : 0;
	dlg->m_HookDLLs = t->flags3 & HOOKDLLS ? 1 : 0;
	dlg->m_TerminateOnClose = t->flags6 & TERMINATEONCLOSE ? 1 : 0;
	dlg->m_ConfirmOnClose = t->flags6 & CONFIRMONCLOSE ? 1 : 0;
	dlg->m_EmulateRegistry = t->flags3 & EMULATEREGISTRY ? 1 : 0;
	dlg->m_OverrideRegistry = t->flags4 & OVERRIDEREGISTRY ? 1 : 0;
	dlg->m_Wow64Registry = t->flags6 & WOW64REGISTRY ? 1 : 0;
	dlg->m_Wow32Registry = t->flags6 & WOW32REGISTRY ? 1 : 0;
	dlg->m_HookEnabled = t->flags3 & HOOKENABLED ? 1 : 0;
	dlg->m_NoBanner = t->flags2 & NOBANNER ? 1 : 0;
	dlg->m_StartDebug = t->flags2 & STARTDEBUG ? 1 : 0;
	dlg->m_FullScreenOnly = t->flags3 & FULLSCREENONLY ? 1 : 0;
	dlg->m_FilterMessages = t->flags3 & FILTERMESSAGES ? 1 : 0;
	dlg->m_PeekAllMessages = t->flags3 & PEEKALLMESSAGES ? 1 : 0;
	dlg->m_NoWinPosChanges = t->flags5 & NOWINPOSCHANGES ? 1 : 0;
	dlg->m_MessagePump = t->flags5 & MESSAGEPUMP ? 1 : 0;

	dlg->m_DxEmulationMode = 0; // none
	//if(t->flags & AUTOMATIC) dlg->m_DxEmulationMode = 0;
	if(t->flags & EMULATEBUFFER) dlg->m_DxEmulationMode = 1;
	if(t->flags & LOCKEDSURFACE) dlg->m_DxEmulationMode = 2;
	if(t->flags & EMULATESURFACE) dlg->m_DxEmulationMode = 3;
	if(t->flags5 & HYBRIDMODE) dlg->m_DxEmulationMode = 4;
	if(t->flags5 & GDIMODE) dlg->m_DxEmulationMode = 5;

	dlg->m_DxFilterMode = 0;
	if(t->flags4 & BILINEAR2XFILTER) dlg->m_DxFilterMode = 1;
	if(t->flags5 & BILINEARFILTER) dlg->m_DxFilterMode = 2;

	dlg->m_DCEmulationMode = 0;
	if(t->flags2 & GDISTRETCHED) dlg->m_DCEmulationMode = 1;
	if(t->flags3 & GDIEMULATEDC) dlg->m_DCEmulationMode = 2;
	//if(t->flags & MAPGDITOPRIMARY) dlg->m_DCEmulationMode = 3;

	dlg->m_ResTypes = 0;
	if(t->flags4 & SUPPORTSVGA) dlg->m_ResTypes = 0;
	if(t->flags4 & SUPPORTHDTV) dlg->m_ResTypes = 1;
	if(t->flags4 & NATIVERES) dlg->m_ResTypes = 2;

	dlg->m_MouseVisibility = 0;
	if(t->flags & HIDEHWCURSOR) dlg->m_MouseVisibility = 1;
	if(t->flags2 & SHOWHWCURSOR) dlg->m_MouseVisibility = 2;

	dlg->m_TextureHandling = 0;
	if(t->flags5 & TEXTUREHIGHLIGHT) dlg->m_TextureHandling = 1;
	if(t->flags5 & TEXTUREDUMP) dlg->m_TextureHandling = 2;
	if(t->flags5 & TEXTUREHACK) dlg->m_TextureHandling = 3;

	dlg->m_SonProcessMode = 0;
	if(t->flags4 & SUPPRESSCHILD) dlg->m_SonProcessMode = 1;
	if(t->flags5 & ENABLESONHOOK) dlg->m_SonProcessMode = 2;
	if(t->flags5 & INJECTSON) dlg->m_SonProcessMode = 3;

	dlg->m_HookDI = t->flags & HOOKDI ? 1 : 0;
	dlg->m_HookDI8 = t->flags & HOOKDI8 ? 1 : 0;
	dlg->m_ModifyMouse = t->flags & MODIFYMOUSE ? 1 : 0;
	dlg->m_VirtualJoystick = t->flags6 & VIRTUALJOYSTICK ? 1 : 0;
	dlg->m_Unacquire = t->flags6 & UNACQUIRE ? 1 : 0;
	dlg->m_OutProxyTrace = t->tflags & OUTPROXYTRACE ? 1 : 0;
	dlg->m_OutDebug = t->tflags & OUTDEBUG ? 1 : 0;
	dlg->m_CursorTrace = t->tflags & OUTCURSORTRACE ? 1 : 0;
	dlg->m_LogEnabled = t->tflags & OUTTRACE ? 1 : 0;
	dlg->m_OutDebugString = t->tflags & OUTDEBUGSTRING ? 1 : 0;
	dlg->m_EraseLogFile = t->tflags & ERASELOGFILE ? 1 : 0;
	dlg->m_AddTimeStamp = t->tflags & ADDTIMESTAMP ? 1 : 0;
	dlg->m_OutWinMessages = t->tflags & OUTWINMESSAGES ? 1 : 0;
	dlg->m_OutDWTrace = t->tflags & OUTDXWINTRACE ? 1 : 0;
	dlg->m_OutD3DTrace = t->tflags & OUTD3DTRACE ? 1 : 0;
	dlg->m_OutDDRAWTrace = t->tflags & OUTDDRAWTRACE ? 1 : 0;
	dlg->m_DXProxed = t->tflags & DXPROXED ? 1 : 0;
	dlg->m_AssertDialog = t->tflags & ASSERTDIALOG ? 1 : 0;
	dlg->m_ImportTable = t->tflags & OUTIMPORTTABLE ? 1 : 0;
	dlg->m_RegistryOp = t->tflags & OUTREGISTRY ? 1 : 0;
	dlg->m_TraceHooks = t->tflags & TRACEHOOKS ? 1 : 0;
	//dlg->m_HandleDC = t->flags & HANDLEDC ? 1 : 0;
	dlg->m_HandleExceptions = t->flags & HANDLEEXCEPTIONS ? 1 : 0;
	dlg->m_SuppressIME = t->flags2 & SUPPRESSIME ? 1 : 0;
	dlg->m_SuppressD3DExt = t->flags3 & SUPPRESSD3DEXT ? 1 : 0;
	dlg->m_SetCompatibility = t->flags2 & SETCOMPATIBILITY ? 1 : 0;
	dlg->m_AEROBoost = t->flags5 & AEROBOOST ? 1 : 0;
	dlg->m_DiabloTweak = t->flags5 & DIABLOTWEAK ? 1 : 0;
	dlg->m_EASportsHack = t->flags5 & EASPORTSHACK ? 1 : 0;
	dlg->m_LegacyAlloc = t->flags6 & LEGACYALLOC ? 1 : 0;
	dlg->m_DisableMaxWinMode = t->flags6 & DISABLEMAXWINMODE ? 1 : 0;
	dlg->m_NoImagehlp = t->flags5 & NOIMAGEHLP ? 1 : 0;
	dlg->m_ForcesHEL = t->flags3 & FORCESHEL ? 1 : 0;
	dlg->m_SetZBufferBitDepths = t->flags6 & SETZBUFFERBITDEPTHS ? 1 : 0;
	dlg->m_ForcesSwapEffect = t->flags6 & FORCESWAPEFFECT ? 1 : 0;
	dlg->m_ColorFix = t->flags3 & COLORFIX ? 1 : 0;
	dlg->m_NoPixelFormat = t->flags3 & NOPIXELFORMAT ? 1 : 0;
	dlg->m_NoAlphaChannel = t->flags4 & NOALPHACHANNEL ? 1 : 0;
	dlg->m_FixRefCounter = t->flags4 & FIXREFCOUNTER ? 1 : 0;
	dlg->m_ReturnNullRef = t->flags4 & RETURNNULLREF ? 1 : 0;
	dlg->m_NoD3DReset = t->flags4 & NOD3DRESET ? 1 : 0;
	dlg->m_HideDesktop = t->flags4 & HIDEDESKTOP ? 1 : 0;
	dlg->m_HideTaskbar = t->flags6 & HIDETASKBAR ? 1 : 0;
	dlg->m_ActivateApp = t->flags6 & ACTIVATEAPP ? 1 : 0;
	dlg->m_UnlockZOrder = t->flags5 & UNLOCKZORDER ? 1 : 0;
	dlg->m_NoDestroyWindow = t->flags6 & NODESTROYWINDOW ? 1 : 0;
	dlg->m_LockSysColors = t->flags3 & LOCKSYSCOLORS ? 1 : 0;
	dlg->m_LockReservedPalette = t->flags5 & LOCKRESERVEDPALETTE ? 1 : 0;
	dlg->m_ForceRGBtoYUV = t->flags3 & RGB2YUV ? 1 : 0;
	dlg->m_ForceYUVtoRGB = t->flags3 & YUV2RGB ? 1 : 0;
	dlg->m_LimitScreenRes = t->flags4 & LIMITSCREENRES ? 1 : 0;
	dlg->m_SingleProcAffinity = t->flags3 & SINGLEPROCAFFINITY ? 1 : 0;
	dlg->m_LimitResources = t->flags2 & LIMITRESOURCES ? 1 : 0;
	dlg->m_CDROMDriveType = t->flags3 & CDROMDRIVETYPE ? 1 : 0;
	dlg->m_HideCDROMEmpty = t->flags4 & HIDECDROMEMPTY ? 1 : 0;
	dlg->m_HookGOGLibs = t->flags6 & HOOKGOGLIBS ? 1 : 0;
	dlg->m_BypassGOGLibs = t->flags6 & BYPASSGOGLIBS ? 1 : 0;
	dlg->m_FontBypass = t->flags3 & FONTBYPASS ? 1 : 0;
	dlg->m_BufferedIOFix = t->flags3 & BUFFEREDIOFIX ? 1 : 0;
	dlg->m_ZBufferClean = t->flags4 & ZBUFFERCLEAN ? 1 : 0;
	dlg->m_ZBuffer0Clean = t->flags4 & ZBUFFER0CLEAN ? 1 : 0;
	dlg->m_ZBufferAlways = t->flags4 & ZBUFFERALWAYS ? 1 : 0;
	dlg->m_HotPatchAlways = t->flags4 & HOTPATCHALWAYS ? 1 : 0;
	dlg->m_FreezeInjectedSon = t->flags5 & FREEZEINJECTEDSON ? 1 : 0;
	dlg->m_StressResources = t->flags5 & STRESSRESOURCES ? 1 : 0;
	dlg->m_NoPower2Fix = t->flags4 & NOPOWER2FIX ? 1 : 0;
	dlg->m_NoPerfCounter = t->flags4 & NOPERFCOUNTER ? 1 : 0;
	dlg->m_DisableFogging = t->flags4 & DISABLEFOGGING ? 1 : 0;
	dlg->m_ClearTarget = t->flags5 & CLEARTARGET ? 1 : 0;
	dlg->m_FixPitch = t->flags6 & FIXPITCH ? 1 : 0;
	dlg->m_Power2Width = t->flags6 & POWER2WIDTH ? 1 : 0;
	dlg->m_SaveLoad = t->flags & SAVELOAD ? 1 : 0;
	dlg->m_SlowDown = t->flags & SLOWDOWN ? 1 : 0;
	dlg->m_BlitFromBackBuffer = t->flags & BLITFROMBACKBUFFER ? 1 : 0;
	dlg->m_NoFlipEmulation = t->flags4 & NOFLIPEMULATION ? 1 : 0;
	dlg->m_SuppressClipping = t->flags & SUPPRESSCLIPPING ? 1 : 0;
	dlg->m_DisableGammaRamp = t->flags2 & DISABLEGAMMARAMP ? 1 : 0;
	dlg->m_AutoRefresh = t->flags & AUTOREFRESH ? 1 : 0;
	dlg->m_TextureFormat = t->flags5 & TEXTUREFORMAT ? 1 : 0;
	dlg->m_FixWinFrame = t->flags & FIXWINFRAME ? 1 : 0;
	dlg->m_EnableClipping = t->flags & ENABLECLIPPING ? 1 : 0;
	dlg->m_CursorClipping = t->flags & CLIPCURSOR ? 1 : 0;
	dlg->m_VideoToSystemMem = t->flags & SWITCHVIDEOMEMORY ? 1 : 0;
	dlg->m_FixTextOut = t->flags & FIXTEXTOUT ? 1 : 0;
	dlg->m_SharedDC = t->flags6 & SHAREDDC ? 1 : 0;
	dlg->m_HookGlide = t->flags4 & HOOKGLIDE ? 1 : 0;
	dlg->m_RemapMCI = t->flags5 & REMAPMCI ? 1 : 0;
	dlg->m_NoMovies = t->flags6 & NOMOVIES ? 1 : 0;
	dlg->m_FixMoviesColor = t->flags6 & FIXMOVIESCOLOR ? 1 : 0;
	dlg->m_StretchMovies = t->flags6 & STRETCHMOVIES ? 1 : 0;
	dlg->m_BypassMCI = t->flags6 & BYPASSMCI ? 1 : 0;
	dlg->m_SuppressRelease = t->flags6 & SUPPRESSRELEASE ? 1 : 0;
	dlg->m_KeepCursorWithin = t->flags & KEEPCURSORWITHIN ? 1 : 0;
	dlg->m_KeepCursorFixed = t->flags2 & KEEPCURSORFIXED ? 1 : 0;
	dlg->m_UseRGB565 = t->flags & USERGB565 ? 1 : 0;
	dlg->m_SuppressDXErrors = t->flags & SUPPRESSDXERRORS ? 1 : 0;
	dlg->m_FlipEmulation = t->flags6 & FLIPEMULATION ? 1 : 0;
	dlg->m_MarkBlit = t->flags3 & MARKBLIT ? 1 : 0;
	dlg->m_MarkLock = t->flags3 & MARKLOCK ? 1 : 0;
	dlg->m_NoSysMemPrimary = t->flags6 & NOSYSMEMPRIMARY ? 1 : 0;
	dlg->m_NoSysMemBackBuf = t->flags6 & NOSYSMEMBACKBUF ? 1 : 0;
	dlg->m_NoBlt = t->flags5 & NOBLT ? 1 : 0;
	dlg->m_FastBlt = t->flags5 & DOFASTBLT ? 1 : 0;
	dlg->m_GDIColorConv = t->flags5 & GDICOLORCONV ? 1 : 0;
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
	//dlg->m_NoMouseProc = t->flags6 & NOMOUSEPROC ? 1 : 0;
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
	dlg->m_StretchTimers = t->flags4 & STRETCHTIMERS ? 1 : 0;
	dlg->m_NormalizePerfCount = t->flags5 & NORMALIZEPERFCOUNT ? 1 : 0;
	dlg->m_QuarterBlt = t->flags5 & QUARTERBLT ? 1 : 0;
	dlg->m_FineTiming = t->flags4 & FINETIMING ? 1 : 0;
	dlg->m_ReleaseMouse = t->flags4 & RELEASEMOUSE ? 1 : 0;
	dlg->m_EnableTimeFreeze = t->flags4 & ENABLETIMEFREEZE ? 1 : 0;
	dlg->m_EnableHotKeys = t->flags4 & ENABLEHOTKEYS ? 1 : 0;
	dlg->m_InterceptRDTSC = t->flags4 & INTERCEPTRDTSC ? 1 : 0;
	dlg->m_HookOpenGL = t->flags2 & HOOKOPENGL ? 1 : 0;
	dlg->m_ForceHookOpenGL = t->flags3 & FORCEHOOKOPENGL ? 1 : 0;
	dlg->m_FixPixelZoom = t->flags6 & FIXPIXELZOOM ? 1 : 0;
	dlg->m_WireFrame = t->flags2 & WIREFRAME ? 1 : 0;
	dlg->m_NoTextures = t->flags4 & NOTEXTURES ? 1 : 0;
	dlg->m_BlackWhite = t->flags3 & BLACKWHITE ? 1 : 0;
	dlg->m_FakeVersion = t->flags2 & FAKEVERSION ? 1 : 0;
	dlg->m_FullRectBlt = t->flags2 & FULLRECTBLT ? 1 : 0;
	dlg->m_CenterToWin = t->flags5 & CENTERTOWIN ? 1 : 0;
	dlg->m_Deinterlace = t->flags5 & DEINTERLACE ? 1 : 0;
	dlg->m_NoPaletteUpdate = t->flags2 & NOPALETTEUPDATE ? 1 : 0;
	dlg->m_SurfaceWarn = t->flags3 & SURFACEWARN ? 1 : 0;
	dlg->m_CapMask = t->flags3 & CAPMASK ? 1 : 0;
	dlg->m_NoWindowHooks = t->flags6 & NOWINDOWHOOKS ? 1 : 0;
	dlg->m_NoDDRAWBlt = t->flags3 & NODDRAWBLT ? 1 : 0;
	dlg->m_NoDDRAWFlip = t->flags3 & NODDRAWFLIP ? 1 : 0;
	dlg->m_NoGDIBlt = t->flags3 & NOGDIBLT ? 1 : 0;
	dlg->m_NoFillRect = t->flags4 & NOFILLRECT ? 1 : 0;
	dlg->m_ReuseEmulatedDC = t->flags6 & REUSEEMULATEDDC ? 1 : 0;
	dlg->m_CreateDesktop = t->flags6 & CREATEDESKTOP ? 1 : 0;
	dlg->m_SyncPalette = t->flags6 & SYNCPALETTE ? 1 : 0;
	dlg->m_AnalyticMode = t->flags3 & ANALYTICMODE ? 1 : 0;
	dlg->m_ReplacePrivOps = t->flags5 & REPLACEPRIVOPS ? 1 : 0;
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
	dlg->m_MaxScreenRes = t->MaxScreenRes;
	dlg->m_SwapEffect = t->SwapEffect;
}

static void SaveConfigItem(TARGETMAP *TargetMap, PRIVATEMAP *PrivateMap, int i, char *InitPath)
{
	char key[32], val[32];
	char *EscBuf = NULL;
	sprintf_s(key, sizeof(key), "title%i", i);
	WritePrivateProfileString("target", key, PrivateMap->title, InitPath);
	sprintf_s(key, sizeof(key), "path%i", i);
	WritePrivateProfileString("target", key, TargetMap->path, InitPath);
	sprintf_s(key, sizeof(key), "launchpath%i", i);
	WritePrivateProfileString("target", key, PrivateMap->launchpath, InitPath);
	sprintf_s(key, sizeof(key), "module%i", i);
	WritePrivateProfileString("target", key, TargetMap->module, InitPath);
	sprintf_s(key, sizeof(key), "opengllib%i", i);
	WritePrivateProfileString("target", key, TargetMap->OpenGLLib, InitPath);
	sprintf_s(key, sizeof(key), "notes%i", i);
	WritePrivateProfileString("target", key, Escape(PrivateMap->notes, &EscBuf), InitPath);
	sprintf_s(key, sizeof(key), "registry%i", i);
	WritePrivateProfileString("target", key, Escape(PrivateMap->registry, &EscBuf), InitPath);
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
	sprintf_s(key, sizeof(key), "flagj%i", i);
	sprintf_s(val, sizeof(val), "%i", TargetMap->flags5);
	WritePrivateProfileString("target", key, val, InitPath);
	sprintf_s(key, sizeof(key), "flagk%i", i);
	sprintf_s(val, sizeof(val), "%i", TargetMap->flags6);
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

	sprintf_s(key, sizeof(key), "winver%i", i);
	sprintf_s(val, sizeof(val), "%i", TargetMap->FakeVersionId);
	WritePrivateProfileString("target", key, val, InitPath);
	sprintf_s(key, sizeof(key), "maxres%i", i);
	sprintf_s(val, sizeof(val), "%i", TargetMap->MaxScreenRes);
	WritePrivateProfileString("target", key, val, InitPath);
	sprintf_s(key, sizeof(key), "swapeffect%i", i);
	sprintf_s(val, sizeof(val), "%i", TargetMap->SwapEffect);
	WritePrivateProfileString("target", key, val, InitPath);

	free(EscBuf);
	EscBuf = NULL;
}

static void ClearTarget(int i, char *InitPath)
{
	char key[32];
	sprintf_s(key, sizeof(key), "path%i", i);
	WritePrivateProfileString("target", key, 0, InitPath);
	sprintf_s(key, sizeof(key), "launchpath%i", i);
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
	sprintf_s(key, sizeof(key), "flagj%i", i);
	WritePrivateProfileString("target", key, 0, InitPath);
	sprintf_s(key, sizeof(key), "flagk%i", i);
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
	sprintf_s(key, sizeof(key), "swapeffect%i", i);
	WritePrivateProfileString("target", key, 0, InitPath);
	sprintf_s(key, sizeof(key), "initts%i", i);
	WritePrivateProfileString("target", key, 0, InitPath);

	sprintf_s(key, sizeof(key), "winver%i", i);
	WritePrivateProfileString("target", key, 0, InitPath);
	sprintf_s(key, sizeof(key), "maxres%i", i);
	WritePrivateProfileString("target", key, 0, InitPath);
	sprintf_s(key, sizeof(key), "notes%i", i);
	WritePrivateProfileString("target", key, 0, InitPath);
	sprintf_s(key, sizeof(key), "registry%i", i);
	WritePrivateProfileString("target", key, 0, InitPath);
}

static int LoadConfigItem(TARGETMAP *TargetMap, PRIVATEMAP *PrivateMap, int i, char *InitPath)
{
	char key[32];
	char *EscBuf = NULL;
	char *sBuf;
	sBuf = (char *)malloc(1000000);
	extern BOOL gbDebug;
	// -------
	sprintf_s(key, sizeof(key), "path%i", i);
	GetPrivateProfileString("target", key, "", TargetMap->path, MAX_PATH, InitPath);
	if(!TargetMap->path[0]) return FALSE;
	// -------
	sprintf_s(key, sizeof(key), "launchpath%i", i);
	GetPrivateProfileString("target", key, "", PrivateMap->launchpath, MAX_PATH, InitPath);
	// -------
	sprintf_s(key, sizeof(key), "title%i", i);
	GetPrivateProfileString("target", key, "", PrivateMap->title, sizeof(PRIVATEMAP)-1, InitPath);
	// -------
	sprintf_s(key, sizeof(key), "module%i", i);
	GetPrivateProfileString("target", key, "", TargetMap->module, sizeof(TargetMap->module)-1, InitPath);
	// -------
	sprintf_s(key, sizeof(key), "opengllib%i", i);
	GetPrivateProfileString("target", key, "", TargetMap->OpenGLLib, sizeof(TargetMap->OpenGLLib)-1, InitPath);
	// -------
	sprintf_s(key, sizeof(key), "notes%i", i);
	GetPrivateProfileString("target", key, "", sBuf, 1000000, InitPath);
	Unescape(sBuf, &EscBuf);
	PrivateMap->notes = (char *)malloc(strlen(EscBuf)+1);
	strcpy(PrivateMap->notes, EscBuf);
	// -------
	sprintf_s(key, sizeof(key), "registry%i", i);
	GetPrivateProfileString("target", key, "", sBuf, 1000000, InitPath);
	Unescape(sBuf, &EscBuf);
	PrivateMap->registry = (char *)malloc(strlen(EscBuf)+1);
	strcpy(PrivateMap->registry, EscBuf);
	// -------
	sprintf_s(key, sizeof(key), "ver%i", i);
	TargetMap->dxversion = GetPrivateProfileInt("target", key, 0, InitPath);
	// -------
	sprintf_s(key, sizeof(key), "coord%i", i);
	TargetMap->coordinates = GetPrivateProfileInt("target", key, 0, InitPath);
	// -------
	sprintf_s(key, sizeof(key), "flag%i", i);
	TargetMap->flags = GetPrivateProfileInt("target", key, 0, InitPath);
	// -------
	sprintf_s(key, sizeof(key), "flagg%i", i);
	TargetMap->flags2 = GetPrivateProfileInt("target", key, 0, InitPath);
	// -------
	sprintf_s(key, sizeof(key), "flagh%i", i);
	TargetMap->flags3 = GetPrivateProfileInt("target", key, 0, InitPath);
	// -------
	sprintf_s(key, sizeof(key), "flagi%i", i);
	TargetMap->flags4 = GetPrivateProfileInt("target", key, 0, InitPath);
	// -------
	sprintf_s(key, sizeof(key), "flagj%i", i);
	TargetMap->flags5 = GetPrivateProfileInt("target", key, 0, InitPath);
	// -------
	sprintf_s(key, sizeof(key), "flagk%i", i);
	TargetMap->flags6 = GetPrivateProfileInt("target", key, 0, InitPath);
	// -------
	sprintf_s(key, sizeof(key), "tflag%i", i);
	TargetMap->tflags = GetPrivateProfileInt("target", key, 0, InitPath);
	// -------
	sprintf_s(key, sizeof(key), "initx%i", i);
	TargetMap->initx = GetPrivateProfileInt("target", key, 0, InitPath);
	// -------
	sprintf_s(key, sizeof(key), "inity%i", i);
	TargetMap->inity = GetPrivateProfileInt("target", key, 0, InitPath);
	// -------
	sprintf_s(key, sizeof(key), "minx%i", i);
	TargetMap->minx = GetPrivateProfileInt("target", key, 0, InitPath);
	// -------
	sprintf_s(key, sizeof(key), "miny%i", i);
	TargetMap->miny = GetPrivateProfileInt("target", key, 0, InitPath);
	// -------
	sprintf_s(key, sizeof(key), "maxx%i", i);
	TargetMap->maxx = GetPrivateProfileInt("target", key, 0, InitPath);
	// -------
	sprintf_s(key, sizeof(key), "maxy%i", i);
	TargetMap->maxy = GetPrivateProfileInt("target", key, 0, InitPath);
	// -------
	sprintf_s(key, sizeof(key), "posx%i", i);
	TargetMap->posx = GetPrivateProfileInt("target", key, 0, InitPath);
	// -------
	sprintf_s(key, sizeof(key), "posy%i", i);
	TargetMap->posy = GetPrivateProfileInt("target", key, 0, InitPath);
	// -------
	sprintf_s(key, sizeof(key), "sizx%i", i);
	TargetMap->sizx = GetPrivateProfileInt("target", key, 0, InitPath);
	// -------
	sprintf_s(key, sizeof(key), "sizy%i", i);
	TargetMap->sizy = GetPrivateProfileInt("target", key, 0, InitPath);
	// -------
	sprintf_s(key, sizeof(key), "maxfps%i", i);
	TargetMap->MaxFPS = GetPrivateProfileInt("target", key, 0, InitPath);
	// -------
	sprintf_s(key, sizeof(key), "initts%i", i);
	TargetMap->InitTS = GetPrivateProfileInt("target", key, 0, InitPath);
	// -------
	sprintf_s(key, sizeof(key), "swapeffect%i", i);
	TargetMap->SwapEffect = GetPrivateProfileInt("target", key, 0, InitPath);
	// -------
	sprintf_s(key, sizeof(key), "winver%i", i);
	TargetMap->FakeVersionId = GetPrivateProfileInt("target", key, 0, InitPath);
	// -------
	sprintf_s(key, sizeof(key), "maxres%i", i);
	TargetMap->MaxScreenRes = GetPrivateProfileInt("target", key, 0, InitPath);
	
	if (!gbDebug){
		// clear debug flags
		TargetMap->flags &= ~(0);
		TargetMap->flags3 &= ~(YUV2RGB|RGB2YUV|SURFACEWARN|ANALYTICMODE|NODDRAWBLT|NODDRAWFLIP|NOGDIBLT);
	}
	free(EscBuf);
	EscBuf = NULL;
	free(sBuf);
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
		SaveConfigItem(&TargetMaps[i], &PrivateMaps[i], i, gInitPath);
	}
	for(; i < MAXTARGETS; i ++) ClearTarget(i, gInitPath);
	this->isUpdated=FALSE;
}

CDxwndhostView::~CDxwndhostView()
{
	EndHook();

	RevertScreenChanges(&this->InitDevMode);

	if (this->isUpdated){
		if (MessageBoxLang(DXW_STRING_LISTUPDATE, DXW_STRING_WARNING, MB_YESNO | MB_ICONQUESTION)==IDYES) 
			this->SaveConfigFile();
	}

	if(this->SystemTray.Enabled()){
		this->SystemTray.StopAnimation();
		this->SystemTray.HideIcon();
	}
}

void CDxwndhostView::OnExit()
{
	// check for running apps ....
	if (GetHookStatus(NULL)==DXW_RUNNING){
		if (MessageBoxLang(DXW_STRING_EXIT, DXW_STRING_WARNING, MB_OKCANCEL | MB_ICONQUESTION)==IDCANCEL) 
			return;
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
	GetCurrentDirectory(MAX_PATH, gInitPath);
	strcat_s(gInitPath, sizeof(gInitPath), "\\");
	strcat_s(gInitPath, sizeof(gInitPath), m_ConfigFileName);
	listctrl.InsertColumn(0, &listcol);

	for(i = 0; i < MAXTARGETS; i ++){
		if (!LoadConfigItem(&TargetMaps[i], &PrivateMaps[i], i, gInitPath)) break;
		listitem.mask = LVIF_TEXT | LVIF_IMAGE;
		listitem.iItem = i;
		listitem.iSubItem = 0;
		listitem.pszText = PrivateMaps[i].title;
		listitem.iImage = SetTargetIcon(TargetMaps[i]);
		listctrl.InsertItem(&listitem);
	}
	for(; i < MAXTARGETS; i ++) {
		TargetMaps[i].path[0] = 0;
		PrivateMaps[i].title[0] = 0;
	}
	Resize();
	SetTarget(TargetMaps);
	if(m_InitialState == DXW_ACTIVE)
		this->OnHookStart();
	else
		this->OnHookStop();
	if(m_StartToTray) this->OnGoToTrayIcon();
	this->isUpdated=FALSE;
	pTitles = &PrivateMaps[0];
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

//static char ImportExportPath[4096] = {0};

void CDxwndhostView::OnExport()
{
	int i;
    char path[MAX_PATH];
	CListCtrl& listctrl = GetListCtrl();
	POSITION pos;
	if(!listctrl.GetSelectedCount()) return;
	pos = listctrl.GetFirstSelectedItemPosition();
	i = listctrl.GetNextSelectedItem(pos);
	GetPrivateProfileString("window", "exportpath", NULL, path, MAX_PATH, gInitPath);
	//strcat_s(path, MAX_PATH, "\\");
	strcat_s(path, MAX_PATH, PrivateMaps[i].title);
	CFileDialog dlg( FALSE, "*.dxw", path, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
        "dxwnd task config (*.dxw)|*.dxw|All Files (*.*)|*.*||",  this);
	if( dlg.DoModal() == IDOK) {
		DWORD TFlags;
		TARGETMAP *TargetMap;
		strcpy(path, dlg.GetPathName().GetBuffer());
		//MessageBox(path, "PathName", MB_OK);
		// export with no trace flags active
		TargetMap = &TargetMaps[i];
		TFlags = TargetMap->tflags;
		TargetMap->tflags = 0;
		SaveConfigItem(&TargetMaps[i], &PrivateMaps[i], 0, path);
		TargetMap->tflags = TFlags;
		if(GetPrivateProfileInt("window", "updatepaths", 1, gInitPath)) {
			GetFolderFromPath(path);
			WritePrivateProfileString("window", "exportpath", path, gInitPath);
		}
	}
}

void CDxwndhostView::OnImport()
{
	LV_ITEM listitem;
	int i;
    char path[MAX_PATH];
	char ImportExportPath[4096] = {0};
	for (i=0; strlen(TargetMaps[i].path) && i<MAXTARGETS; i++)
		;
	if (i==MAXTARGETS) {
		MessageBoxLang(DXW_STRING_MAXENTRIES, DXW_STRING_WARNING, MB_OK | MB_ICONEXCLAMATION);
		return;
	}
	path[0]=0;
	CListCtrl& listctrl = GetListCtrl();
	
	char folder[MAX_PATH+1];
	char pathname[MAX_PATH+1];
	OPENFILENAME ofn = {0};
	GetPrivateProfileString("window", "exportpath", NULL, pathname, MAX_PATH, gInitPath);
	ofn.lpstrInitialDir = pathname;
	ofn.lStructSize = sizeof(ofn);
	ofn.lpstrFilter = "DxWnd export file\0*.dxw\0\0";
	ofn.lpstrFile = (LPSTR)ImportExportPath;
	ofn.nMaxFile = 4096;
	ofn.Flags = OFN_ALLOWMULTISELECT | OFN_EXPLORER;
	ofn.lpstrDefExt = "dxw";

	if(GetOpenFileName(&ofn)){
		if(ImportExportPath[ofn.nFileOffset - 1] != '\0'){
			// Single-Select
			// "buffer" - name of file
			if(LoadConfigItem(&TargetMaps[i], &PrivateMaps[i], 0, ImportExportPath)){
				listitem.mask = LVIF_TEXT | LVIF_IMAGE;
				listitem.iItem = i;
				listitem.iSubItem = 0;
				listitem.iImage = SetTargetIcon(TargetMaps[i]);
				listitem.pszText = PrivateMaps[i].title;
				listctrl.InsertItem(&listitem);
				if(GetPrivateProfileInt("window", "updatepaths", 1, gInitPath)) {
					GetFolderFromPath(ImportExportPath);
					WritePrivateProfileString("window", "exportpath", ImportExportPath, gInitPath);
				}
			}
		}
		else{
			// Multi-Select
			char* p = ImportExportPath;
			strcpy(folder, p);
			strcat(folder, "\\");
			if(GetPrivateProfileInt("window", "updatepaths", 1, gInitPath)) 
				WritePrivateProfileString("window", "exportpath", folder, gInitPath);
			p += lstrlen((LPSTR)p) + 1;
			while(*p && (i<MAXTARGETS)){
				// "p" - name of each file, NULL to terminate
				if(!*p) break;
				if(i==MAXTARGETS) break;
				strcpy(pathname, folder);
				strcat(pathname, p);
				if (LoadConfigItem(&TargetMaps[i], &PrivateMaps[i], 0, pathname)){
					listitem.mask = LVIF_TEXT | LVIF_IMAGE;
					listitem.iItem = i;
					listitem.iSubItem = 0;
					listitem.iImage = SetTargetIcon(TargetMaps[i]);
					listitem.pszText = PrivateMaps[i].title;
					listctrl.InsertItem(&listitem);
					i++;
				}
				p += lstrlen((LPSTR)p) + 1;
			}
		}
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
	dlg.m_Title = PrivateMaps[i].title;
	dlg.m_Notes = CString(PrivateMaps[i].notes);
	dlg.m_Registry = CString(PrivateMaps[i].registry);
	dlg.m_LaunchPath = PrivateMaps[i].launchpath;
	SetDlgFromTarget(&TargetMaps[i], &dlg);
	if(dlg.DoModal() == IDOK && dlg.m_FilePath.GetLength()){
		strnncpy(PrivateMaps[i].title, (char *)dlg.m_Title.GetString(), MAX_TITLE); 
		PrivateMaps[i].notes = (char *)realloc(PrivateMaps[i].notes, strlen(dlg.m_Notes.GetString())+1);
		strcpy(PrivateMaps[i].notes, (char *)dlg.m_Notes.GetString());
		PrivateMaps[i].registry = (char *)realloc(PrivateMaps[i].registry, strlen(dlg.m_Registry.GetString())+1);
		strcpy(PrivateMaps[i].registry, (char *)dlg.m_Registry.GetString());
		strnncpy(PrivateMaps[i].launchpath, (char *)dlg.m_LaunchPath.GetString(), MAX_PATH);
		SetTargetFromDlg(&TargetMaps[i], &dlg);
		CListCtrl& listctrl = GetListCtrl();
		listitem.mask = LVIF_TEXT | LVIF_IMAGE;
		listitem.iItem = i;
		listitem.iSubItem = 0;
		listitem.iImage = SetTargetIcon(TargetMaps[i]);
		listitem.pszText = PrivateMaps[i].title;
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
		res=MessageBoxLang(DXW_STRING_CLEARLOG, DXW_STRING_WARNING, MB_YESNO | MB_ICONQUESTION);
		if(res!=IDYES) return;
	}

	_unlink(FilePath);
}

void CDxwndhostView::OnDebugView() 
{
	PROCESS_INFORMATION pinfo;
	STARTUPINFO sinfo;
	char exepath[MAX_PATH+1];
	char folderpath[MAX_PATH+1];
	ZeroMemory(&sinfo, sizeof(sinfo));
	sinfo.cb = sizeof(sinfo); 
	GetPrivateProfileString("window", "debugview", "DbgView.exe", exepath, MAX_PATH, gInitPath);
	strcpy_s(folderpath, sizeof(folderpath), exepath);
	PathRemoveFileSpec(folderpath);
	if(strlen(folderpath)==0) strcpy(folderpath, ".\\");
	if(!CreateProcessA(NULL, exepath, 0, 0, false, CREATE_DEFAULT_ERROR_MODE, NULL, folderpath, &sinfo, &pinfo)){
		char sInfo[81];
		sprintf(sInfo, "Error %d starting DebugView", GetLastError());
		MessageBox(sInfo, "Error", MB_ICONERROR|MB_OK);
		return;
	}
	CloseHandle(pinfo.hProcess);
	CloseHandle(pinfo.hThread);
}

void CDxwndhostView::OnSetRegistry() 
{
	int i;
	CTargetDlg dlg;
	POSITION pos;
	char *Registry;
	FILE *regfp;

	CListCtrl& listctrl = GetListCtrl();

	if(!listctrl.GetSelectedCount()) return;
	pos = listctrl.GetFirstSelectedItemPosition();
	i = listctrl.GetNextSelectedItem(pos);
	Registry = PrivateMaps[i].registry;

	regfp=fopen("dxwnd.reg", "w");
	if(regfp==NULL){
		MessageBox("Error writing virtual registry file", "Error", MB_ICONERROR|MB_OK);
		return;
	}

	fwrite(Registry, strlen(Registry), 1, regfp);
	fclose(regfp);
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
	res=MessageBoxLang(DXW_STRING_SORTLIST, DXW_STRING_WARNING, MB_YESNO | MB_ICONQUESTION);
	if(res!=IDYES) return;

	// bubble sort the list
	swapped=1;
	while(swapped){
		swapped=0;
		for(i=0; i<itemcount-1; i++){
			if(strcasecmp(PrivateMaps[i].title, PrivateMaps[i+1].title)>0){
				// swap entries
				MapEntry=TargetMaps[i];
				TargetMaps[i]=TargetMaps[i+1];
				TargetMaps[i+1]=MapEntry;
				TitEntry=PrivateMaps[i];
				PrivateMaps[i]=PrivateMaps[i+1];
				PrivateMaps[i+1]=TitEntry;
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
		listitem.pszText = PrivateMaps[i].title;
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
	DXWNDSTATUS DxWndStatus;
	if ((GetHookStatus(&DxWndStatus) != DXW_RUNNING) || (DxWndStatus.hWnd==NULL)) {
		MessageBoxLang(DXW_STRING_NOPAUSETASK, DXW_STRING_INFO, MB_ICONEXCLAMATION);
	}
	else {
		wchar_t *wcstring = new wchar_t[48+1];
		mbstowcs_s(NULL, wcstring, 48, PrivateMaps[DxWndStatus.TaskIdx].title, _TRUNCATE);
		res=MessageBoxLangArg(DXW_STRING_PAUSETASK, DXW_STRING_INFO, MB_YESNO | MB_ICONQUESTION, wcstring);
		if(res!=IDYES) return;
		PauseResumeThreadList(DxWndStatus.dwPid, FALSE);
	}
}

void CDxwndhostView::OnResume() 
{
	CTargetDlg dlg;
	HRESULT res;
	DXWNDSTATUS DxWndStatus;
	if ((GetHookStatus(&DxWndStatus) != DXW_RUNNING) || (DxWndStatus.hWnd==NULL)) {
		MessageBoxLang(DXW_STRING_NORESUMETASK, DXW_STRING_INFO, MB_ICONEXCLAMATION);
	}
	else {
		wchar_t *wcstring = new wchar_t[48+1];
		mbstowcs_s(NULL, wcstring, 48, PrivateMaps[DxWndStatus.TaskIdx].title, _TRUNCATE);
		res=MessageBoxLangArg(DXW_STRING_RESUMETASK, DXW_STRING_WARNING, MB_YESNO | MB_ICONQUESTION, wcstring);
		if(res!=IDYES) return;
		PauseResumeThreadList(DxWndStatus.dwPid, TRUE);
	}
}

extern HWND find_main_window(unsigned long);

void CDxwndhostView::OnWindowMinimize() 
{
	DXWNDSTATUS DxWndStatus;
	if ((GetHookStatus(&DxWndStatus) == DXW_RUNNING) && (DxWndStatus.hWnd!=NULL))
		//::PostMessage(DxWndStatus.hWnd, WM_SYSCOMMAND, SC_MINIMIZE, 0);
		::PostMessage(find_main_window(DxWndStatus.dwPid), WM_SYSCOMMAND, SC_MINIMIZE, 0);
}

void CDxwndhostView::OnWindowRestore() 
{
	DXWNDSTATUS DxWndStatus;
	if ((GetHookStatus(&DxWndStatus) == DXW_RUNNING) && (DxWndStatus.hWnd!=NULL))
		//::PostMessage(DxWndStatus.hWnd, WM_SYSCOMMAND, SC_RESTORE, 0);
		::PostMessage(find_main_window(DxWndStatus.dwPid), WM_SYSCOMMAND, SC_RESTORE, 0);
}

void CDxwndhostView::OnWindowClose() 
{
	DXWNDSTATUS DxWndStatus;
	if ((GetHookStatus(&DxWndStatus) == DXW_RUNNING) && (DxWndStatus.hWnd!=NULL))
		//::PostMessage(DxWndStatus.hWnd, WM_SYSCOMMAND, SC_CLOSE, 0);
		::PostMessage(find_main_window(DxWndStatus.dwPid), WM_SYSCOMMAND, SC_CLOSE, 0);
}

void CDxwndhostView::OnTaskbarHide() 
{
	gShowHideTaskBar(TRUE);
}

void CDxwndhostView::OnTaskbarShow() 
{
	gShowHideTaskBar(FALSE);
}

void CDxwndhostView::OnKill() 
{
	CTargetDlg dlg;
	HRESULT res;
	HANDLE TargetHandle;
	DXWNDSTATUS DxWndStatus;
	if ((GetHookStatus(&DxWndStatus) != DXW_RUNNING) || (DxWndStatus.hWnd==NULL)) {
		MessageBoxLang(DXW_STRING_NOKILLTASK, DXW_STRING_INFO, MB_ICONEXCLAMATION);
	}
	else {
		wchar_t *wcstring = new wchar_t[48+1];
		mbstowcs_s(NULL, wcstring, 48, PrivateMaps[DxWndStatus.TaskIdx].title, _TRUNCATE);
		res=MessageBoxLangArg(DXW_STRING_KILLTASK, DXW_STRING_WARNING, MB_YESNO | MB_ICONQUESTION, wcstring);
		if(res!=IDYES) return;
		DxWndStatus.dwPid;

		TargetHandle = OpenProcess(PROCESS_TERMINATE, FALSE, DxWndStatus.dwPid);
		if(!TargetHandle){
			MessageBoxLangArg(DXW_STRING_OPENPROCESS, DXW_STRING_ERROR, MB_ICONEXCLAMATION, DxWndStatus.dwPid, GetLastError());
			return;
		}
		if(!TerminateProcess(TargetHandle, 0)){
			MessageBoxLangArg(DXW_STRING_TERMINATE, DXW_STRING_ERROR, MB_ICONEXCLAMATION, TargetHandle, GetLastError());
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

	if(!listctrl.GetSelectedCount()) return ;
	pos = listctrl.GetFirstSelectedItemPosition();
	i = listctrl.GetNextSelectedItem(pos);

	strnncpy(FilePath, TargetMaps[i].path, MAX_PATH);
	lpProcName=FilePath;
	while (lpNext=strchr(lpProcName,'\\')) lpProcName=lpNext+1;

	if(!KillProcByName(lpProcName, FALSE)){
		wchar_t *wcstring = new wchar_t[48+1];
		mbstowcs_s(NULL, wcstring, 48, PrivateMaps[i].title, _TRUNCATE);
		res=MessageBoxLangArg(DXW_STRING_KILLTASK, DXW_STRING_WARNING, MB_YESNO | MB_ICONQUESTION, wcstring);
		if(res!=IDYES) return;
		KillProcByName(lpProcName, TRUE);
	}
	else{
		MessageBoxLang(DXW_STRING_NOKILLTASK, DXW_STRING_INFO, MB_ICONEXCLAMATION);
	}

	ClipCursor(NULL);
	RevertScreenChanges(&this->InitDevMode);
}

void CDxwndhostView::OnAdd() 
{
	// TODO: Please add a command handler code here.
	int i;
	CTargetDlg dlg;
	LV_ITEM listitem;

	for(i = 0; i < MAXTARGETS; i ++) if(!TargetMaps[i].path[0]) break;
	if(i>=MAXTARGETS){
		MessageBoxLang(DXW_STRING_MAXENTRIES, DXW_STRING_WARNING, MB_OK | MB_ICONEXCLAMATION);
		return;
	}
	memset(&TargetMaps[i],0,sizeof(TARGETMAP)); // clean up, just in case....
	if(dlg.DoModal() == IDOK && dlg.m_FilePath.GetLength()){
		strnncpy(PrivateMaps[i].title, (char *)dlg.m_Title.GetString(), MAX_TITLE);
		PrivateMaps[i].notes = (char *)malloc(strlen(dlg.m_Notes.GetString())+1);
		strcpy(PrivateMaps[i].notes, (char *)dlg.m_Notes.GetString());
		PrivateMaps[i].registry = (char *)malloc(strlen(dlg.m_Registry.GetString())+1);
		strcpy(PrivateMaps[i].registry, (char *)dlg.m_Registry.GetString());
		strnncpy(PrivateMaps[i].launchpath, (char *)dlg.m_LaunchPath.GetString(), MAX_PATH);
		SetTargetFromDlg(&TargetMaps[i], &dlg);
		CListCtrl& listctrl = GetListCtrl();
		listitem.mask = LVIF_TEXT | LVIF_IMAGE;
		listitem.iItem = i;
		listitem.iSubItem = 0;
		listitem.iImage = SetTargetIcon(TargetMaps[i]);
		if (strlen(PrivateMaps[i].title)==0){
			int len;
			CString	FilePath;
			FilePath=TargetMaps[i].path;
			len=FilePath.ReverseFind('\\');	
			FilePath=FilePath.Right(FilePath.GetLength()-len-1);
			strncpy_s(PrivateMaps[i].title, sizeof(PrivateMaps[i].title), FilePath.GetString(), sizeof(PrivateMaps[i].title)-1);
		}
		listitem.pszText = PrivateMaps[i].title;
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

	FilePath=PrivateMaps[i].title;
	if (FilePath.GetLength()==0){
		FilePath = TargetMaps[i].path;
		len=FilePath.ReverseFind('\\');	
		if (len==0) return;
		FilePath=FilePath.Right(FilePath.GetLength()-len-1);
	}

	wchar_t *wcstring = new wchar_t[MAX_PATH+1];
	mbstowcs_s(NULL, wcstring, MAX_PATH, FilePath, _TRUNCATE);
	res=MessageBoxLangArg(DXW_STRING_DELENTRY, DXW_STRING_WARNING, MB_YESNO | MB_ICONQUESTION, wcstring);
	if(res!=IDYES) return;
	listctrl.DeleteItem(i);
	for(; i < MAXTARGETS  - 1; i ++) {
		PrivateMaps[i] = PrivateMaps[i + 1]; // V2.1.74 fix
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

	res=MessageBoxLang(DXW_STRING_CLEARALL, DXW_STRING_WARNING, MB_YESNO | MB_ICONQUESTION);
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
			MessageBoxLang(DXW_STRING_TRAYFAIL, DXW_STRING_ERROR, MB_OK);
			return;
		}
		IconId=(menu->GetMenuState(ID_HOOK_START, MF_BYCOMMAND)==MF_CHECKED)?IDI_DXWAIT:IDI_DXIDLE;
		if(!this->SystemTray.SetIcon(IconId)){
			MessageBoxLang(DXW_STRING_ICONFAIL, DXW_STRING_ERROR, MB_OK);
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
	if (MessageBoxLang(DXW_STRING_SAVELIST, DXW_STRING_WARNING, MB_YESNO | MB_ICONQUESTION)==IDYES) 
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

void CDxwndhostView::OnViewDesktop()
{
	CDesktopDialog *pDlg = new CDesktopDialog();
	BOOL ret = pDlg->Create(CDesktopDialog::IDD, this); 
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
		if(strlen(TargetMaps[i].path) == 0) break;
		tmp = listctrl.GetStringWidth(PrivateMaps[i].title);
		if(size < tmp) size = tmp;
	}
	
	listcol.mask = LVCF_WIDTH;
	listcol.cx = size + 32;
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
	case ID_PLOG_DEBUGVIEW:
		OnDebugView();
		break;
	case ID_SETREGISTRY:
		OnSetRegistry();
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

static char *ExceptionCaption(DWORD ec)
{
	char *c;
	switch(ec){
		case EXCEPTION_ACCESS_VIOLATION:		c="Access Violation"; break;
		case EXCEPTION_DATATYPE_MISALIGNMENT:	c="Datatype Misalignment"; break;
		case EXCEPTION_BREAKPOINT:				c="Breakpoint"; break;
		case EXCEPTION_SINGLE_STEP:				c="Single Step"; break;
		case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:	c="Array Bouds Exceeded"; break;
		case EXCEPTION_FLT_DENORMAL_OPERAND:	c="Float Denormal Operand"; break;
		case EXCEPTION_FLT_DIVIDE_BY_ZERO:		c="Divide by Zero"; break;
		case EXCEPTION_FLT_INEXACT_RESULT:		c="Inexact Result"; break;
		case EXCEPTION_FLT_INVALID_OPERATION:	c="Invalid Operation"; break;
		case EXCEPTION_FLT_OVERFLOW:			c="Float Overflow"; break;
		case EXCEPTION_FLT_STACK_CHECK:			c="Float Stack Check"; break;
		case EXCEPTION_FLT_UNDERFLOW:			c="Float Undeflow"; break;
		case EXCEPTION_INT_DIVIDE_BY_ZERO:		c="Integer Divide by Zero"; break;
		case EXCEPTION_INT_OVERFLOW:			c="Integer Overflow"; break;
		case EXCEPTION_PRIV_INSTRUCTION:		c="Priviliged Instruction"; break;
		case EXCEPTION_IN_PAGE_ERROR:			c="In Page Error"; break;
		case EXCEPTION_ILLEGAL_INSTRUCTION:		c="Illegal Instruction"; break;
		case EXCEPTION_NONCONTINUABLE_EXCEPTION:c="Non-continuable exception"; break;
		case EXCEPTION_STACK_OVERFLOW:			c="Stack Overflow"; break;
		case EXCEPTION_INVALID_DISPOSITION:		c="Invalid Disposition"; break;
		case EXCEPTION_GUARD_PAGE:				c="Guard Page Violation"; break;
		case EXCEPTION_INVALID_HANDLE:			c="Invalid Handle"; break;
		//case EXCEPTION_POSSIBLE_DEADLOCK:		c="Possible Deadlock"; break;
		default:								c="unknown"; break;
	}
	return c;
}

// For thread messaging
#define DEBUG_EVENT_MESSAGE		WM_APP + 0x100

typedef struct {
	TARGETMAP *TM;
	PRIVATEMAP *PM;
} ThreadInfo_Type;
ThreadInfo_Type ThreadInfo;

DWORD WINAPI StartDebug(void *p)
{
	ThreadInfo_Type *ThInfo;
	STARTUPINFO sinfo;
	PROCESS_INFORMATION pinfo;
	char path[MAX_PATH];
#ifdef DXWDEBUGSTEPPING
	PROCESS_INFORMATION *pi;
	CREATE_THREAD_DEBUG_INFO *ti;
	LOAD_DLL_DEBUG_INFO *li;
	UNLOAD_DLL_DEBUG_INFO *ui;
	EXCEPTION_DEBUG_INFO *ei;
	EXIT_PROCESS_DEBUG_INFO *xpi;
	EXIT_THREAD_DEBUG_INFO *xti;
	int res;
	BOOL step=TRUE; // initialize to TRUE to enable
	BOOL stepdll=FALSE; // initialize to TRUE to enable
	extern char *GetFileNameFromHandle(HANDLE);
#endif
#ifdef LOCKINJECTIONTHREADS
	DWORD StartingCode;
	LPVOID StartAddress = 0;
	HANDLE TargetHandle = NULL;
#endif
	bool bContinueDebugging;
	char DebugMessage[256+1];

	ThInfo = (ThreadInfo_Type *)p;
	ZeroMemory(&sinfo, sizeof(sinfo));
	sinfo.cb = sizeof(sinfo);
	strcpy_s(path, sizeof(path), ThInfo->TM->path);
	PathRemoveFileSpec(path);
	if(!CreateProcess(NULL, 
		(strlen(ThInfo->PM->launchpath)>0) ? ThInfo->PM->launchpath : ThInfo->TM->path, 
		0, 0, false, DEBUG_PROCESS|DEBUG_ONLY_THIS_PROCESS, NULL, path, &sinfo, &pinfo)){
		sprintf(DebugMessage, "CREATE PROCESS error=%d", GetLastError());
		MessageBoxEx(0, DebugMessage, "ERROR", MB_YESNO | MB_ICONQUESTION, NULL);
	}

	CString strEventMessage;
	DEBUG_EVENT debug_event ={0};
	bContinueDebugging = true;
	DWORD dwContinueStatus = DBG_CONTINUE;
	while(bContinueDebugging)
	{ 
		dwContinueStatus = DBG_EXCEPTION_NOT_HANDLED;
		if (!WaitForDebugEvent(&debug_event, INFINITE)) break; // must release pinfo handles
		switch(debug_event.dwDebugEventCode){
		case EXIT_PROCESS_DEBUG_EVENT:
			bContinueDebugging=false;
			break;
		case CREATE_PROCESS_DEBUG_EVENT:
			GetFullPathName("dxwnd.dll", MAX_PATH, path, NULL);
			if(!Inject(pinfo.dwProcessId, path)){
				// DXW_STRING_INJECTION
				sprintf(DebugMessage,"Injection error: pid=%x dll=%s", pinfo.dwProcessId, path);
				MessageBoxEx(0, DebugMessage, "Injection", MB_ICONEXCLAMATION, NULL);
			}
#ifdef LOCKINJECTIONTHREADS
			extern LPVOID GetThreadStartAddress(HANDLE);
			DWORD EndlessLoop;
			EndlessLoop=0x9090FEEB; // careful: it's BIG ENDIAN: EB FE 90 90
			SIZE_T BytesCount;
			TargetHandle = OpenProcess(PROCESS_QUERY_INFORMATION|PROCESS_VM_OPERATION|PROCESS_VM_READ|PROCESS_VM_WRITE, FALSE, pinfo.dwProcessId);
			if(TargetHandle){
				StartAddress = GetThreadStartAddress(pinfo.hThread);
				if(StartAddress){
					if(!ReadProcessMemory(pinfo.hProcess, StartAddress, &StartingCode, 4, &BytesCount)){ 
						sprintf(DebugMessage,"ReadProcessMemory error=%d", GetLastError());
						MessageBoxEx(0, DebugMessage, "Injection", MB_ICONEXCLAMATION, NULL);
					}
					if(!WriteProcessMemory(pinfo.hProcess, StartAddress, &EndlessLoop, 4, &BytesCount)){
						sprintf(DebugMessage,"WriteProcessMemory error=%d", GetLastError());
						MessageBoxEx(0, DebugMessage, "Injection", MB_ICONEXCLAMATION, NULL);
					}
				}
			}
#endif
			CloseHandle(((CREATE_PROCESS_DEBUG_INFO *)&debug_event.u)->hProcess);
			CloseHandle(((CREATE_PROCESS_DEBUG_INFO *)&debug_event.u)->hThread);
			CloseHandle(((CREATE_PROCESS_DEBUG_INFO *)&debug_event.u)->hFile);
			break;
		case CREATE_THREAD_DEBUG_EVENT:
			CloseHandle(((CREATE_THREAD_DEBUG_INFO *)&debug_event.u)->hThread);
			break;
		case EXIT_THREAD_DEBUG_EVENT:
#ifdef LOCKINJECTIONTHREADS
			if(TargetHandle && StartAddress){
				if(!WriteProcessMemory(pinfo.hProcess, StartAddress, &StartingCode, 4, &BytesCount)){
					sprintf(DebugMessage,"WriteProcessMemory error=%d", GetLastError());
					MessageBoxEx(0, DebugMessage, "Injection", MB_ICONEXCLAMATION, NULL);
				}
			}
			if(TargetHandle) CloseHandle((HANDLE)TargetHandle);
#endif
			bContinueDebugging=false;
			break;
		case LOAD_DLL_DEBUG_EVENT:
			CloseHandle(((LOAD_DLL_DEBUG_INFO *)&debug_event.u)->hFile);
			break;
		case UNLOAD_DLL_DEBUG_EVENT:
			break;
		case OUTPUT_DEBUG_STRING_EVENT: 				
			break;
		case EXCEPTION_DEBUG_EVENT:
			//sprintf(DebugMessage, "Exception %x(%s) caught at addr=%x",
			//	debug_event.u.Exception.ExceptionRecord.ExceptionCode, 
			//	ExceptionCaption(debug_event.u.Exception.ExceptionRecord.ExceptionCode),
			//	debug_event.u.Exception.ExceptionRecord.ExceptionAddress);
			//MessageBoxEx(0, DebugMessage, "EXCEPTION", MB_ICONEXCLAMATION, NULL);
			break;
		default:
			sprintf(DebugMessage,"Unknown eventcode=%x", debug_event.dwDebugEventCode);
			MessageBoxEx(0, DebugMessage, "Injection", MB_ICONEXCLAMATION, NULL);
			break;
		}
		if(bContinueDebugging){
			ContinueDebugEvent(debug_event.dwProcessId, debug_event.dwThreadId, dwContinueStatus);
		}
		else{
			DebugSetProcessKillOnExit(FALSE);
			ContinueDebugEvent(debug_event.dwProcessId,debug_event.dwThreadId, DBG_CONTINUE); 
			DebugActiveProcessStop(debug_event.dwProcessId);
		}
	}
	CloseHandle(pinfo.hThread); // no longer needed, avoid handle leakage
	CloseHandle(pinfo.hProcess); // no longer needed, avoid handle leakage
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
		ThreadInfo.TM=&TargetMaps[i];
		ThreadInfo.PM=&PrivateMaps[i];
		CloseHandle(CreateThread( NULL, 0, StartDebug, &ThreadInfo, 0, NULL)); 
	}
	else{
		CreateProcess(NULL, 
			(strlen(PrivateMaps[i].launchpath)>0) ? PrivateMaps[i].launchpath: TargetMaps[i].path, 
			0, 0, false, CREATE_DEFAULT_ERROR_MODE, NULL, path, &sinfo, &pinfo);
		CloseHandle(pinfo.hProcess); // no longer needed, avoid handle leakage
		CloseHandle(pinfo.hThread); // no longer needed, avoid handle leakage
	}
}

void SwitchToColorDepth(int bpp)
{
	DEVMODE CurrentDevMode;
	BOOL res;
	EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &CurrentDevMode);

	if(MessageBoxLangArg(DXW_STRING_NEWCOLOR, DXW_STRING_WARNING, MB_OKCANCEL | MB_ICONQUESTION, CurrentDevMode.dmBitsPerPel, bpp)!=IDOK) 
		return;

	//OutTraceDW("ChangeDisplaySettings: CURRENT wxh=(%dx%d) BitsPerPel=%d -> 16\n", 
	//	CurrentDevMode.dmPelsWidth, CurrentDevMode.dmPelsHeight, CurrentDevMode.dmBitsPerPel);
	CurrentDevMode.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;
	CurrentDevMode.dmBitsPerPel = bpp;
	res=ChangeDisplaySettings(&CurrentDevMode, CDS_UPDATEREGISTRY);
	if(res!=DISP_CHANGE_SUCCESSFUL) {
		WCHAR *err;
		switch(res){
			case DISP_CHANGE_BADDUALVIEW: err=L"BADDUALVIEW"; break;
			case DISP_CHANGE_BADFLAGS: err=L"BADFLAGS"; break;
			case DISP_CHANGE_BADMODE: err=L"BADMODE"; break;
			case DISP_CHANGE_BADPARAM: err=L"BADPARAM"; break;
			case DISP_CHANGE_FAILED: err=L"FAILED"; break;
			case DISP_CHANGE_NOTUPDATED: err=L"NOTUPDATED"; break;
			case DISP_CHANGE_RESTART: err=L"RESTART"; break;
			default: err=L"???"; break;
		}
		MessageBoxLangArg(DXW_STRING_ERRCOLOR, DXW_STRING_ERROR, MB_OK | MB_ICONEXCLAMATION, err, GetLastError());
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
