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
#include "ShimsDialog.h"
#include "ViewFlagsDialog.h"
#include "CGlobalSettings.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define	WM_ICON_NOTIFY WM_APP+10
#define strcasecmp lstrcmpi

extern UINT m_StartToTray;
extern UINT m_InitialState;
extern char m_ConfigFileName[20+1];
extern BOOL Inject(DWORD, const char *);
extern int KillProcByName(char *, BOOL);
extern BOOL gTransientMode;
extern BOOL gAutoHideMode;
extern BOOL gbDebug;
extern BOOL gMustDie;
extern int iProgIndex;

PRIVATEMAP *pTitles; // global ptr: get rid of it!!
TARGETMAP *pTargets; // idem.

#define LOCKINJECTIONTHREADS

char gInitPath[MAX_PATH]; // don't put it into the class because it must be used after destructor
CWnd *pParent;

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
	ON_COMMAND(ID_PDUPLICATE, OnDuplicate)
	ON_COMMAND(ID_PEXPORT, OnExport)
	ON_COMMAND(ID_PKILL, OnProcessKill)
	ON_COMMAND(ID_FILE_IMPORT, OnImport)
	ON_COMMAND(ID_DELETE, OnDelete)
	ON_COMMAND(ID_FILE_SORTPROGRAMSLIST, OnSort)
	ON_COMMAND(ID_EDIT_GLOBALSETTINGS, OnGlobalSettings)
	ON_COMMAND(ID_OPTIONS_EXPERTMODE, OnExpertModeToggle)
	ON_COMMAND(ID_FILE_CLEARALLLOGS, OnClearAllLogs)
	ON_COMMAND(ID_FILE_GOTOTRAYICON, OnGoToTrayIcon)
	ON_COMMAND(ID_FILE_SAVE, OnSaveFile)
	ON_COMMAND(ID_HOOK_START, OnHookStart)
	ON_COMMAND(ID_HOOK_STOP, OnHookStop)
	ON_COMMAND(ID_DXAPP_EXIT, OnExit)
	ON_COMMAND(ID_RUN, OnRun)
	ON_COMMAND(ID_TRAY_RESTORE, OnTrayRestore)
	ON_COMMAND(ID_VIEW_STATUS, OnViewStatus)
	ON_COMMAND(ID_VIEW_SHIMS, OnViewShims)
	ON_COMMAND(ID_VIEW_DESKTOP, OnViewDesktop)
	ON_COMMAND(ID_VIEW_PALETTE, OnViewPalette)
	ON_COMMAND(ID_VIEW_TIMESLIDER, OnViewTimeSlider)
	ON_COMMAND(ID_DESKTOPCOLORDEPTH_8BPP, OnDesktopcolordepth8bpp)
	ON_COMMAND(ID_DESKTOPCOLORDEPTH_16BPP, OnDesktopcolordepth16bpp)
	ON_COMMAND(ID_DESKTOPCOLORDEPTH_24BPP, OnDesktopcolordepth24bpp)
	ON_COMMAND(ID_DESKTOPCOLORDEPTH_32BPP, OnDesktopcolordepth32bpp)
	ON_COMMAND(ID_TOOLS_RECOVERSCREENMODE, OnRecoverScreenMode)
	ON_COMMAND(ID_TOOLS_CLEARCOMPATIBILITYFLAGS, OnClearCompatibilityFlags)
	ON_COMMAND(ID_MOVE_TOP, OnMoveTop)
	ON_COMMAND(ID_MOVE_UP, OnMoveUp)
	ON_COMMAND(ID_MOVE_DOWN, OnMoveDown)
	ON_COMMAND(ID_MOVE_BOTTOM, OnMoveBottom)
	ON_COMMAND(ID_VIEW_FLAGS, OnViewFlags)
	ON_WM_RBUTTONDOWN()
	ON_WM_HOTKEY()
	ON_WM_DROPFILES()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// v2.1.68: dialog box for status check.
BOOL CheckStatus()
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

void SetTargetFromDlg(TARGETMAP *t, CTargetDlg *dlg)
{
	strcpy_s(t->path, sizeof(t->path), dlg->m_FilePath);
	strcpy_s(t->module, sizeof(t->module), dlg->m_Module);
	strcpy_s(t->OpenGLLib, sizeof(t->OpenGLLib), dlg->m_OpenGLLib);
	if(dlg->m_DXVersion > 1) dlg->m_DXVersion += 5;
	t->dxversion = dlg->m_DXVersion;
	t->monitorid = (dlg->m_MonitorId == -1) ? -1 : dlg->m_MonitorId - 1;
	t->MaxDdrawInterface = dlg->m_MaxDdrawInterface+1;
	t->SlowRatio = dlg->m_SlowRatio;
	t->coordinates = dlg->m_Coordinates;
	t->flags = 0;
	t->flags2 = 0;
	t->flags3 = 0;
	t->flags4 = 0;
	t->flags5 = 0;
	t->flags6 = 0;
	t->flags7 = 0;
	t->flags8 = 0;
	t->tflags = 0;
	t->dflags = 0;
	if(dlg->m_UnNotify) t->flags |= UNNOTIFY;
	if(dlg->m_Windowize) t->flags2 |= WINDOWIZE;
	if(dlg->m_HookDLLs) t->flags3 |= HOOKDLLS;
	if(dlg->m_AnsiWide) t->flags5 |= ANSIWIDE;
	if(dlg->m_HookNoRun) t->flags7 |= HOOKNORUN;
	if(dlg->m_CopyNoShims) t->flags7 |= COPYNOSHIMS;
	if(dlg->m_HookNoUpdate) t->flags7 |= HOOKNOUPDATE;
	if(dlg->m_SequencedIAT) t->flags8 |= SEQUENCEDIAT;
	if(dlg->m_TerminateOnClose) t->flags6 |= TERMINATEONCLOSE;
	if(dlg->m_ConfirmOnClose) t->flags6 |= CONFIRMONCLOSE;
	if(dlg->m_EmulateRegistry) t->flags3 |= EMULATEREGISTRY;
	if(dlg->m_OverrideRegistry) t->flags4 |= OVERRIDEREGISTRY;
	if(dlg->m_Wow64Registry) t->flags6 |= WOW64REGISTRY;
	if(dlg->m_Wow32Registry) t->flags6 |= WOW32REGISTRY;
	if(dlg->m_HookEnabled) t->flags3 |= HOOKENABLED;
	if(dlg->m_NeedAdminCaps) t->flags |= NEEDADMINCAPS;
	if(dlg->m_NoBanner) t->flags2 |= NOBANNER;
	if(dlg->m_HotPatch) t->flags4 |= HOTPATCH;
	if(dlg->m_FullScreenOnly) t->flags3 |= FULLSCREENONLY;
	if(dlg->m_ShowHints) t->flags7 |= SHOWHINTS;
	if(dlg->m_PeekAllMessages) t->flags3 |= PEEKALLMESSAGES;
	if(dlg->m_NoWinPosChanges) t->flags5 |= NOWINPOSCHANGES;
	if(dlg->m_MessagePump) t->flags5 |= MESSAGEPUMP;

	switch(dlg->m_InjectionMode){
		case 0: break;
		case 1: t->flags2 |= STARTDEBUG; break;
		case 2: t->flags7 |= INJECTSUSPENDED; break;
	}

	switch(dlg->m_DxEmulationMode){
		case 0: break;
		case 1: t->flags |= EMULATEBUFFER; break;
		case 2: t->flags |= LOCKEDSURFACE; break;
		case 3: t->flags |= EMULATESURFACE; break;
		case 4: t->flags5 |= HYBRIDMODE; break;
		case 5: t->flags5 |= GDIMODE; break;
	}

	switch(dlg->m_InitColorDepth){
		case 0: break;
		case 1: t->flags2 |= INIT8BPP; break;
		case 2: t->flags2 |= INIT16BPP; break;
		case 3: t->flags7 |= INIT24BPP; break;
		case 4: t->flags7 |= INIT32BPP; break;
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
		case 3: t->flags6 |= SHAREDDC; break;
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

	switch(dlg->m_MouseClipper){
		case 0: break;
		case 1: t->flags |= DISABLECLIPPING; break;
		case 2: t->flags |= CLIPCURSOR; break;
		case 3: t->flags |= CLIPCURSOR; t->flags8 |= CLIPLOCKED; break;
	}	

	switch(dlg->m_OffendingMessages){
		case 0: break;
		case 1: t->flags3 |= FILTERMESSAGES; break;
		case 2: t->flags3 |= DEFAULTMESSAGES; break;
	}	

	switch(dlg->m_TextureHandling){
		case 0: break;
		case 1: t->flags5 |= TEXTUREHIGHLIGHT; break;
		case 2: t->flags5 |= TEXTUREDUMP; break;
		case 3: t->flags5 |= TEXTUREHACK; break;
		case 4: t->flags5 |= TEXTURETRANSP; break;
	}

	switch(dlg->m_SonProcessMode){
		case 0: break;
		case 1: t->flags4 |= SUPPRESSCHILD; break;
		case 2: t->flags5 |= ENABLESONHOOK; break;
		case 3: t->flags5 |= INJECTSON; break;
	}	

	switch(dlg->m_LogMode){
		case 0: break;
		case 1: t->tflags |= (OUTTRACE|ERASELOGFILE); break;
		case 2: t->tflags |= OUTTRACE; break;
		case 3: t->tflags |= (OUTTRACE|OUTSEPARATED); break;
	}	

	switch(dlg->m_WinMovementType){
		case 0: break;
		case 1: t->flags |= LOCKWINPOS; break;
		case 2: t->flags |= LOCKWINPOS; t->flags2 |= LOCKEDSIZE; break;
		case 3: t->flags |= LOCKWINPOS; t->flags7 |= ANCHORED; break;
	}

	switch(dlg->m_VSyncMode){
		case 0: break;
		case 1: t->flags8 |= FORCEVSYNC; break;
		case 2: t->flags8 |= FORCENOVSYNC; break;
	}

	switch(dlg->m_VSyncImpl){
		case 0: break;
		case 1: t->flags8 |= VSYNCSCANLINE; break;
		case 2: t->flags |= SAVELOAD; break;
	}
	switch(dlg->m_WaitMode){
		case 0: break;
		case 1: t->flags8 |= FORCEWAIT; break;
		case 2: t->flags8 |= FORCENOWAIT; break;
	}

	switch(dlg->m_ClipperMode){
		case 0: break;
		case 1: t->flags3 |= FORCECLIPPER; break;
		case 2: t->flags |= SUPPRESSCLIPPING; break;
	}

	switch(dlg->m_TextureFileFormat){
		case 0: break;
		case 1: t->flags8 |= RAWFORMAT; break;
		case 2: t->flags8 |= DDSFORMAT; break;
	}

	if(dlg->m_HookDI) t->flags |= HOOKDI;
	if(dlg->m_HookDI8) t->flags |= HOOKDI8;
	if(dlg->m_EmulateRelMouse) t->flags6 |= EMULATERELMOUSE;
	if(dlg->m_SkipDevTypeHID) t->flags7 |= SKIPDEVTYPEHID;
	if(dlg->m_SuppressDIErrors) t->flags7 |= SUPPRESSDIERRORS;
	if(dlg->m_SharedKeyboard) t->flags7 |= SHAREDKEYBOARD;
	if(dlg->m_ModifyMouse) t->flags |= MODIFYMOUSE;
	if(dlg->m_VirtualJoystick) t->flags6 |= VIRTUALJOYSTICK;
	if(dlg->m_Unacquire) t->flags6 |= UNACQUIRE;
	if(dlg->m_OutDebug) t->tflags |= OUTDEBUG;
	if(dlg->m_CursorTrace) t->tflags |= OUTCURSORTRACE;
	if(dlg->m_OutDebugString) t->tflags |= OUTDEBUGSTRING;
	if(dlg->m_AddTimeStamp) t->tflags |= ADDTIMESTAMP;
	if(dlg->m_AddRelativeTime) t->tflags |= ADDRELATIVETIME;
	if(dlg->m_OutWinMessages) t->tflags |= OUTWINMESSAGES;
	if(dlg->m_OutDWTrace) t->tflags |= OUTDXWINTRACE;
	if(dlg->m_OutOGLTrace) t->tflags |= OUTOGLTRACE;
	if(dlg->m_OutWGTrace) t->tflags |= OUTWINGTRACE;
	if(dlg->m_OutDDRAWTrace) t->tflags |= OUTDDRAWTRACE;
	if(dlg->m_OutD3DTrace) t->tflags |= OUTD3DTRACE;
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
	if(dlg->m_DynamicZClean) t->flags8 |= DYNAMICZCLEAN;
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
	if(dlg->m_Enum16bitModes) t->flags7 |= ENUM16BITMODES;
	if(dlg->m_TrimTextureFormats) t->flags8 |= TRIMTEXTUREFORMATS;
	if(dlg->m_D3D8Back16) t->flags8 |= D3D8BACK16;
	if(dlg->m_SetCompatibility) t->flags2 |= SETCOMPATIBILITY;
	if(dlg->m_AEROBoost) t->flags5 |= AEROBOOST;
	if(dlg->m_DiabloTweak) t->flags5 |= DIABLOTWEAK;
	if(dlg->m_HookDirectSound) t->flags7 |= HOOKDIRECTSOUND;
	if(dlg->m_HookWinG32) t->flags8 |= HOOKWING32;
	if(dlg->m_HookSmackW32) t->flags7 |= HOOKSMACKW32;
	if(dlg->m_FixSmackLoop) t->flags7 |= FIXSMACKLOOP;
	if(dlg->m_BlockPriorityClass) t->flags7 |= BLOCKPRIORITYCLASS;
	if(dlg->m_EASportsHack) t->flags5 |= EASPORTSHACK;
	if(dlg->m_LegacyAlloc) t->flags6 |= LEGACYALLOC;
	if(dlg->m_DisableMaxWinMode) t->flags6 |= DISABLEMAXWINMODE;
	if(dlg->m_DisableDisableAltTab) t->flags7 |= DISABLEDISABLEALTTAB;
	if(dlg->m_NoImagehlp) t->flags5 |= NOIMAGEHLP;
	if(dlg->m_ForcesHEL) t->flags3 |= FORCESHEL;
	if(dlg->m_NoHALDevice) t->flags8 |= NOHALDEVICE;
	if(dlg->m_MinimalCaps) t->flags3 |= MINIMALCAPS;
	if(dlg->m_SetZBufferBitDepths) t->flags6 |= SETZBUFFERBITDEPTHS;
	if(dlg->m_ForcesSwapEffect) t->flags6 |= FORCESWAPEFFECT;
	if(dlg->m_ColorFix) t->flags3 |= COLORFIX;
	if(dlg->m_FixGlobalUnlock) t->flags7 |= FIXGLOBALUNLOCK;
	if(dlg->m_FixFreeLibrary) t->flags7 |= FIXFREELIBRARY;
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
	if(dlg->m_LimitScreenRes) t->flags4 |= LIMITSCREENRES;
	if(dlg->m_SingleProcAffinity) t->flags3 |= SINGLEPROCAFFINITY;
	if(dlg->m_UseLastCore) t->flags5 |= USELASTCORE;
	if(dlg->m_SlowDown) t->flags |= SLOWDOWN;
	if(dlg->m_BlitFromBackBuffer) t->flags |= BLITFROMBACKBUFFER;
	if(dlg->m_NoFlipEmulation) t->flags4 |= NOFLIPEMULATION;
	if(dlg->m_LockColorDepth) t->flags7 |= LOCKCOLORDEPTH;
	if(dlg->m_DisableGammaRamp) t->flags2 |= DISABLEGAMMARAMP;
	if(dlg->m_AutoRefresh) t->flags |= AUTOREFRESH;
	if(dlg->m_IndependentRefresh) t->flags2 |= INDEPENDENTREFRESH;
	if(dlg->m_TextureFormat) t->flags5 |= TEXTUREFORMAT;
	if(dlg->m_FixWinFrame) t->flags |= FIXWINFRAME;
	if(dlg->m_VideoToSystemMem) t->flags |= SWITCHVIDEOMEMORY;
	if(dlg->m_FixTextOut) t->flags |= FIXTEXTOUT;
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
	if(dlg->m_MarkWinG32) t->flags8 |= MARKWING32;
	if(dlg->m_MarkGDI32) t->flags8 |= MARKGDI32;
	if(dlg->m_DumpDIBSection) t->flags8 |= DUMPDIBSECTION;
	if(dlg->m_DumpDevContext) t->flags8 |= DUMPDEVCONTEXT;
	if(dlg->m_NoSysMemPrimary) t->flags6 |= NOSYSMEMPRIMARY;
	if(dlg->m_NoSysMemBackBuf) t->flags6 |= NOSYSMEMBACKBUF;
	if(dlg->m_NoBlt) t->flags5 |= NOBLT;
	if(dlg->m_FastBlt) t->flags5 |= DOFASTBLT;
	if(dlg->m_GDIColorConv) t->flags5 |= GDICOLORCONV;
	if(dlg->m_PreventMaximize) t->flags |= PREVENTMAXIMIZE;
	if(dlg->m_ClientRemapping) t->flags |= CLIENTREMAPPING;
	if(dlg->m_LockWinStyle) t->flags |= LOCKWINSTYLE;
	if(dlg->m_FixParentWin) t->flags |= FIXPARENTWIN;
	if(dlg->m_ModalStyle) t->flags2 |= MODALSTYLE;
	if(dlg->m_KeepAspectRatio) t->flags2 |= KEEPASPECTRATIO;
	if(dlg->m_ForceWinResize) t->flags2 |= FORCEWINRESIZE;
	if(dlg->m_HideMultiMonitor) t->flags2 |= HIDEMULTIMONITOR;
	if(dlg->m_FixD3DFrame) t->flags3 |= FIXD3DFRAME;
	if(dlg->m_NoWindowMove) t->flags3 |= NOWINDOWMOVE;
	if(dlg->m_HookChildWin) t->flags |= HOOKCHILDWIN;
	if(dlg->m_MessageProc) t->flags |= MESSAGEPROC;
	if(dlg->m_FixMouseHook) t->flags8 |= FIXMOUSEHOOK;
	if(dlg->m_FixNCHITTEST) t->flags2 |= FIXNCHITTEST;
	if(dlg->m_RecoverScreenMode) t->flags2 |= RECOVERSCREENMODE;
	if(dlg->m_RefreshOnResize) t->flags2 |= REFRESHONRESIZE;
	if(dlg->m_BackBufAttach) t->flags2 |= BACKBUFATTACH;
	if(dlg->m_ClearTextureFourCC) t->flags7 |= CLEARTEXTUREFOURCC;
	if(dlg->m_NoDDExclusiveMode) t->flags7 |= NODDEXCLUSIVEMODE;
	if(dlg->m_CreateDesktop) t->flags6 |= CREATEDESKTOP;
	if(dlg->m_AllowSysmemOn3DDev) t->flags8 |= ALLOWSYSMEMON3DDEV;
	if(dlg->m_HandleAltF4) t->flags |= HANDLEALTF4;
	if(dlg->m_LimitFPS) t->flags2 |= LIMITFPS;
	if(dlg->m_SkipFPS) t->flags2 |= SKIPFPS;
	if(dlg->m_ShowFPS) t->flags2 |= SHOWFPS;
	if(dlg->m_ShowFPSOverlay) t->flags2 |= SHOWFPSOVERLAY;
	if(dlg->m_ShowTimeStretch) t->flags4 |= SHOWTIMESTRETCH;
	if(dlg->m_TimeStretch) t->flags2 |= TIMESTRETCH;
	if(dlg->m_StretchTimers) t->flags4 |= STRETCHTIMERS;
	if(dlg->m_NormalizePerfCount) t->flags5 |= NORMALIZEPERFCOUNT;
	if(dlg->m_CPUSlowDown) t->flags7 |= CPUSLOWDOWN;
	if(dlg->m_CPUMaxUsage) t->flags7 |= CPUMAXUSAGE;
	if(dlg->m_QuarterBlt) t->flags5 |= QUARTERBLT;
	if(dlg->m_FineTiming) t->flags4 |= FINETIMING;
	if(dlg->m_ReleaseMouse) t->flags4 |= RELEASEMOUSE;
	if(dlg->m_EnableTimeFreeze) t->flags4 |= ENABLETIMEFREEZE;
	if(dlg->m_EnableHotKeys) t->flags4 |= ENABLEHOTKEYS;
	if(dlg->m_InterceptRDTSC) t->flags4 |= INTERCEPTRDTSC;
	if(dlg->m_HookOpenGL) t->flags2 |= HOOKOPENGL;
	if(dlg->m_ForceHookOpenGL) t->flags3 |= FORCEHOOKOPENGL;
	if(dlg->m_FixPixelZoom) t->flags6 |= FIXPIXELZOOM;
	if(dlg->m_FixBindTexture) t->flags7 |= FIXBINDTEXTURE;
	if(dlg->m_HookGlut32) t->flags7 |= HOOKGLUT32;
	if(dlg->m_WireFrame) t->flags2 |= WIREFRAME;
	if(dlg->m_NoTextures) t->flags4 |= NOTEXTURES;
	if(dlg->m_BlackWhite) t->flags3 |= BLACKWHITE;
	if(dlg->m_FakeVersion) t->flags2 |= FAKEVERSION;
	if(dlg->m_FullRectBlt) t->flags2 |= FULLRECTBLT;
	if(dlg->m_CenterToWin) t->flags5 |= CENTERTOWIN;
	if(dlg->m_Deinterlace) t->flags5 |= DEINTERLACE;
	if(dlg->m_LimitDdraw) t->flags7 |= LIMITDDRAW;
	if(dlg->m_SuppressOverlay) t->flags7 |= SUPPRESSOVERLAY;
	if(dlg->m_NoPaletteUpdate) t->flags2 |= NOPALETTEUPDATE;
	if(dlg->m_SurfaceWarn) t->flags3 |= SURFACEWARN;
	if(dlg->m_CapMask) t->flags3 |= CAPMASK;
	if(dlg->m_NoWindowHooks) t->flags6 |= NOWINDOWHOOKS;
	if(dlg->m_NoDDRAWBlt) t->flags3 |= NODDRAWBLT;
	if(dlg->m_NoDDRAWFlip) t->flags3 |= NODDRAWFLIP;
	if(dlg->m_NoGDIBlt) t->flags3 |= NOGDIBLT;
	if(dlg->m_NoFillRect) t->flags4 |= NOFILLRECT;
	if(dlg->m_FixClipperArea) t->flags7 |= FIXCLIPPERAREA;
	if(dlg->m_SyncPalette) t->flags6 |= SYNCPALETTE;
	if(dlg->m_NoWinErrors) t->flags7 |= NOWINERRORS;
	if(dlg->m_PretendVisible) t->flags8 |= PRETENDVISIBLE;
	if(dlg->m_WinInsulation) t->flags8 |= WININSULATION;
	if(dlg->m_QualityFonts) t->flags8 |= QUALITYFONTS;
	if(dlg->m_AnalyticMode) t->flags3 |= ANALYTICMODE;
	if(dlg->m_ReplacePrivOps) t->flags5 |= REPLACEPRIVOPS;
	if(dlg->m_InitialRes) t->flags7 |= INITIALRES;
	if(dlg->m_MaximumRes) t->flags7 |= MAXIMUMRES;
	t->posx = dlg->m_PosX;
	t->posy = dlg->m_PosY;
	t->sizx = dlg->m_SizX;
	t->sizy = dlg->m_SizY;
	t->ScanLine = dlg->m_ScanLine;
	t->MaxFPS = dlg->m_MaxFPS;
	t->InitTS = dlg->m_InitTS-8;
	t->FakeVersionId = dlg->m_FakeVersionId;
	t->MaxScreenRes = dlg->m_MaxScreenRes;
	t->SwapEffect = dlg->m_SwapEffect;
	t->resw = dlg->m_InitResW;
	t->resh = dlg->m_InitResH;
	strcpy_s(t->module, sizeof(t->module), dlg->m_Module);
	strcpy_s(t->OpenGLLib, sizeof(t->OpenGLLib), dlg->m_OpenGLLib);
}

static void SetDlgFromTarget(TARGETMAP *t, CTargetDlg *dlg)
{
	dlg->m_DXVersion = t->dxversion;
	dlg->m_MonitorId = t->monitorid + 1;
	dlg->m_MaxDdrawInterface = t->MaxDdrawInterface-1;
	dlg->m_SlowRatio = t->SlowRatio;
	if(dlg->m_DXVersion > 6) dlg->m_DXVersion -= 5;
	dlg->m_Coordinates = t->coordinates;
	dlg->m_FilePath = t->path;
	dlg->m_Module = t->module;
	dlg->m_OpenGLLib = t->OpenGLLib;
	dlg->m_UnNotify = t->flags & UNNOTIFY ? 1 : 0;
	dlg->m_Windowize = t->flags2 & WINDOWIZE ? 1 : 0;
	dlg->m_HotPatch = t->flags4 & HOTPATCH ? 1 : 0;
	dlg->m_HookDLLs = t->flags3 & HOOKDLLS ? 1 : 0;
	dlg->m_AnsiWide = t->flags5 & ANSIWIDE ? 1 : 0;
	dlg->m_HookNoRun = t->flags7 & HOOKNORUN ? 1 : 0;
	dlg->m_CopyNoShims = t->flags7 & COPYNOSHIMS ? 1 : 0;
	dlg->m_HookNoUpdate = t->flags7 & HOOKNOUPDATE ? 1 : 0;
	dlg->m_SequencedIAT = t->flags8 & SEQUENCEDIAT ? 1 : 0;
	dlg->m_TerminateOnClose = t->flags6 & TERMINATEONCLOSE ? 1 : 0;
	dlg->m_ConfirmOnClose = t->flags6 & CONFIRMONCLOSE ? 1 : 0;
	dlg->m_EmulateRegistry = t->flags3 & EMULATEREGISTRY ? 1 : 0;
	dlg->m_OverrideRegistry = t->flags4 & OVERRIDEREGISTRY ? 1 : 0;
	dlg->m_Wow64Registry = t->flags6 & WOW64REGISTRY ? 1 : 0;
	dlg->m_Wow32Registry = t->flags6 & WOW32REGISTRY ? 1 : 0;
	dlg->m_HookEnabled = t->flags3 & HOOKENABLED ? 1 : 0;
	dlg->m_NeedAdminCaps = t->flags & NEEDADMINCAPS ? 1 : 0;
	dlg->m_NoBanner = t->flags2 & NOBANNER ? 1 : 0;
	dlg->m_FullScreenOnly = t->flags3 & FULLSCREENONLY ? 1 : 0;
	dlg->m_ShowHints = t->flags7 & SHOWHINTS ? 1 : 0;
	dlg->m_PeekAllMessages = t->flags3 & PEEKALLMESSAGES ? 1 : 0;
	dlg->m_NoWinPosChanges = t->flags5 & NOWINPOSCHANGES ? 1 : 0;
	dlg->m_MessagePump = t->flags5 & MESSAGEPUMP ? 1 : 0;

	dlg->m_InjectionMode = 0;
	if(t->flags2 & STARTDEBUG) dlg->m_InjectionMode = 1;
	if(t->flags7 & INJECTSUSPENDED) dlg->m_InjectionMode = 2;

	dlg->m_DxEmulationMode = 0; // none
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
	if(t->flags6 & SHAREDDC    ) dlg->m_DCEmulationMode = 3;

	dlg->m_ResTypes = 0;
	if(t->flags4 & SUPPORTSVGA) dlg->m_ResTypes = 0;
	if(t->flags4 & SUPPORTHDTV) dlg->m_ResTypes = 1;
	if(t->flags4 & NATIVERES) dlg->m_ResTypes = 2;

	dlg->m_MouseVisibility = 0;
	if(t->flags & HIDEHWCURSOR) dlg->m_MouseVisibility = 1;
	if(t->flags2 & SHOWHWCURSOR) dlg->m_MouseVisibility = 2;

	dlg->m_MouseClipper = 0;
	if(t->flags & DISABLECLIPPING) dlg->m_MouseClipper = 1;
	if(t->flags & CLIPCURSOR) dlg->m_MouseClipper = 2;
	if(t->flags8 & CLIPLOCKED) dlg->m_MouseClipper = 3;

	dlg->m_OffendingMessages = 0;
	if(t->flags3 & FILTERMESSAGES) dlg->m_OffendingMessages = 1;
	if(t->flags3 & DEFAULTMESSAGES) dlg->m_OffendingMessages = 2;

	dlg->m_TextureHandling = 0;
	if(t->flags5 & TEXTUREHIGHLIGHT) dlg->m_TextureHandling = 1;
	if(t->flags5 & TEXTUREDUMP) dlg->m_TextureHandling = 2;
	if(t->flags5 & TEXTUREHACK) dlg->m_TextureHandling = 3;
	if(t->flags5 & TEXTURETRANSP) dlg->m_TextureHandling = 4;

	dlg->m_SonProcessMode = 0;
	if(t->flags4 & SUPPRESSCHILD) dlg->m_SonProcessMode = 1;
	if(t->flags5 & ENABLESONHOOK) dlg->m_SonProcessMode = 2;
	if(t->flags5 & INJECTSON) dlg->m_SonProcessMode = 3;

	dlg->m_InitColorDepth = 0;
	if(t->flags2 & INIT8BPP)  dlg->m_InitColorDepth = 1;
	if(t->flags2 & INIT16BPP) dlg->m_InitColorDepth = 2;
	if(t->flags7 & INIT24BPP) dlg->m_InitColorDepth = 3;
	if(t->flags7 & INIT32BPP) dlg->m_InitColorDepth = 4;

	dlg->m_LogMode = 0;
	if(t->tflags & OUTTRACE)		dlg->m_LogMode = 2; // must be first one ....
	if(t->tflags & ERASELOGFILE)	dlg->m_LogMode = 1; 
	if(t->tflags & OUTSEPARATED)	dlg->m_LogMode = 3;

	dlg->m_WinMovementType = 0;
	if(t->flags & LOCKWINPOS)		dlg->m_WinMovementType = 1;
	if(t->flags2 & LOCKEDSIZE)		dlg->m_WinMovementType = 2;
	if(t->flags7 & ANCHORED)		dlg->m_WinMovementType = 3;

	dlg->m_VSyncMode = 0;
	if(t->flags8 & FORCEVSYNC) dlg->m_VSyncMode = 1;
	if(t->flags8 & FORCENOVSYNC) dlg->m_VSyncMode = 2;

	dlg->m_VSyncImpl = 0;
	if(t->flags8 & VSYNCSCANLINE) dlg->m_VSyncImpl = 1;
	if(t->flags & SAVELOAD) dlg->m_VSyncImpl = 2;

	dlg->m_WaitMode = 0;
	if(t->flags8 & FORCEWAIT) dlg->m_WaitMode = 1;
	if(t->flags8 & FORCENOWAIT) dlg->m_WaitMode = 2;

	dlg->m_ClipperMode = 0;
	if (t->flags3 & FORCECLIPPER) dlg->m_ClipperMode = 1;
	if (t->flags & SUPPRESSCLIPPING) dlg->m_ClipperMode = 2;

	dlg->m_TextureFileFormat = 0;
	if( t->flags8 & RAWFORMAT) dlg->m_TextureFileFormat = 1;
	if( t->flags8 & DDSFORMAT) dlg->m_TextureFileFormat = 2;

	dlg->m_HookDI = t->flags & HOOKDI ? 1 : 0;
	dlg->m_HookDI8 = t->flags & HOOKDI8 ? 1 : 0;
	dlg->m_EmulateRelMouse = t->flags6 & EMULATERELMOUSE ? 1 : 0;
	dlg->m_SkipDevTypeHID = t->flags7 & SKIPDEVTYPEHID ? 1 : 0;
	dlg->m_SuppressDIErrors = t->flags7 & SUPPRESSDIERRORS ? 1 : 0;
	dlg->m_SharedKeyboard = t->flags7 & SHAREDKEYBOARD ? 1 : 0;
	dlg->m_ModifyMouse = t->flags & MODIFYMOUSE ? 1 : 0;
	dlg->m_VirtualJoystick = t->flags6 & VIRTUALJOYSTICK ? 1 : 0;
	dlg->m_Unacquire = t->flags6 & UNACQUIRE ? 1 : 0;
	dlg->m_OutDebug = t->tflags & OUTDEBUG ? 1 : 0;
	dlg->m_CursorTrace = t->tflags & OUTCURSORTRACE ? 1 : 0;
	dlg->m_OutDebugString = t->tflags & OUTDEBUGSTRING ? 1 : 0;
	dlg->m_AddTimeStamp = t->tflags & ADDTIMESTAMP ? 1 : 0;
	dlg->m_AddRelativeTime = t->tflags & ADDRELATIVETIME ? 1 : 0;
	dlg->m_OutWinMessages = t->tflags & OUTWINMESSAGES ? 1 : 0;
	dlg->m_OutDWTrace = t->tflags & OUTDXWINTRACE ? 1 : 0;
	dlg->m_OutOGLTrace = t->tflags & OUTOGLTRACE ? 1 : 0;
	dlg->m_OutWGTrace = t->tflags & OUTWINGTRACE ? 1 : 0;
	dlg->m_OutD3DTrace = t->tflags & OUTD3DTRACE ? 1 : 0;
	dlg->m_OutDDRAWTrace = t->tflags & OUTDDRAWTRACE ? 1 : 0;
	dlg->m_AssertDialog = t->tflags & ASSERTDIALOG ? 1 : 0;
	dlg->m_ImportTable = t->tflags & OUTIMPORTTABLE ? 1 : 0;
	dlg->m_RegistryOp = t->tflags & OUTREGISTRY ? 1 : 0;
	dlg->m_TraceHooks = t->tflags & TRACEHOOKS ? 1 : 0;
	dlg->m_HandleExceptions = t->flags & HANDLEEXCEPTIONS ? 1 : 0;
	dlg->m_SuppressIME = t->flags2 & SUPPRESSIME ? 1 : 0;
	dlg->m_SuppressD3DExt = t->flags3 & SUPPRESSD3DEXT ? 1 : 0;
	dlg->m_Enum16bitModes = t->flags7 & ENUM16BITMODES ? 1 : 0;
	dlg->m_TrimTextureFormats = t->flags8 & TRIMTEXTUREFORMATS ? 1 : 0;
	dlg->m_D3D8Back16 = t->flags8 & D3D8BACK16 ? 1 : 0;
	dlg->m_SetCompatibility = t->flags2 & SETCOMPATIBILITY ? 1 : 0;
	dlg->m_AEROBoost = t->flags5 & AEROBOOST ? 1 : 0;
	dlg->m_DiabloTweak = t->flags5 & DIABLOTWEAK ? 1 : 0;
	dlg->m_HookDirectSound = t->flags7 & HOOKDIRECTSOUND ? 1 : 0;
	dlg->m_HookWinG32 = t->flags8 & HOOKWING32 ? 1 : 0;
	dlg->m_HookSmackW32 = t->flags7 & HOOKSMACKW32 ? 1 : 0;
	dlg->m_FixSmackLoop = t->flags7 & FIXSMACKLOOP ? 1 : 0;
	dlg->m_BlockPriorityClass = t->flags7 & BLOCKPRIORITYCLASS ? 1 : 0;
	dlg->m_EASportsHack = t->flags5 & EASPORTSHACK ? 1 : 0;
	dlg->m_LegacyAlloc = t->flags6 & LEGACYALLOC ? 1 : 0;
	dlg->m_DisableMaxWinMode = t->flags6 & DISABLEMAXWINMODE ? 1 : 0;
	dlg->m_DisableDisableAltTab = t->flags7 & DISABLEDISABLEALTTAB ? 1 : 0;
	dlg->m_NoImagehlp = t->flags5 & NOIMAGEHLP ? 1 : 0;
	dlg->m_ForcesHEL = t->flags3 & FORCESHEL ? 1 : 0;
	dlg->m_NoHALDevice = t->flags8 & NOHALDEVICE ? 1 : 0;
	dlg->m_MinimalCaps = t->flags3 & MINIMALCAPS ? 1 : 0;
	dlg->m_SetZBufferBitDepths = t->flags6 & SETZBUFFERBITDEPTHS ? 1 : 0;
	dlg->m_ForcesSwapEffect = t->flags6 & FORCESWAPEFFECT ? 1 : 0;
	dlg->m_ColorFix = t->flags3 & COLORFIX ? 1 : 0;
	dlg->m_FixGlobalUnlock = t->flags7 & FIXGLOBALUNLOCK ? 1 : 0;
	dlg->m_FixFreeLibrary = t->flags7 & FIXFREELIBRARY ? 1 : 0;
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
	dlg->m_LimitScreenRes = t->flags4 & LIMITSCREENRES ? 1 : 0;
	dlg->m_SingleProcAffinity = t->flags3 & SINGLEPROCAFFINITY ? 1 : 0;
	dlg->m_UseLastCore = t->flags5 & USELASTCORE ? 1 : 0;
	dlg->m_LimitResources = t->flags2 & LIMITRESOURCES ? 1 : 0;
	dlg->m_CDROMDriveType = t->flags3 & CDROMDRIVETYPE ? 1 : 0;
	dlg->m_HideCDROMEmpty = t->flags4 & HIDECDROMEMPTY ? 1 : 0;
	dlg->m_HookGOGLibs = t->flags6 & HOOKGOGLIBS ? 1 : 0;
	dlg->m_BypassGOGLibs = t->flags6 & BYPASSGOGLIBS ? 1 : 0;
	dlg->m_FontBypass = t->flags3 & FONTBYPASS ? 1 : 0;
	dlg->m_BufferedIOFix = t->flags3 & BUFFEREDIOFIX ? 1 : 0;
	dlg->m_ZBufferClean = t->flags4 & ZBUFFERCLEAN ? 1 : 0;
	dlg->m_ZBuffer0Clean = t->flags4 & ZBUFFER0CLEAN ? 1 : 0;
	dlg->m_DynamicZClean = t->flags8 & DYNAMICZCLEAN ? 1 : 0;
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
	dlg->m_SlowDown = t->flags & SLOWDOWN ? 1 : 0;
	dlg->m_BlitFromBackBuffer = t->flags & BLITFROMBACKBUFFER ? 1 : 0;
	dlg->m_NoFlipEmulation = t->flags4 & NOFLIPEMULATION ? 1 : 0;
	dlg->m_LockColorDepth = t->flags7 & LOCKCOLORDEPTH ? 1 : 0;
	dlg->m_DisableGammaRamp = t->flags2 & DISABLEGAMMARAMP ? 1 : 0;
	dlg->m_AutoRefresh = t->flags & AUTOREFRESH ? 1 : 0;
	dlg->m_IndependentRefresh = t->flags2 & INDEPENDENTREFRESH ? 1 : 0;
	dlg->m_TextureFormat = t->flags5 & TEXTUREFORMAT ? 1 : 0;
	dlg->m_FixWinFrame = t->flags & FIXWINFRAME ? 1 : 0;
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
	dlg->m_MarkWinG32 = t->flags8 & MARKWING32 ? 1 : 0;
	dlg->m_MarkGDI32 = t->flags8 & MARKGDI32 ? 1 : 0;
	dlg->m_DumpDIBSection = t->flags8 & DUMPDIBSECTION ? 1 : 0;
	dlg->m_DumpDevContext = t->flags8 & DUMPDEVCONTEXT ? 1 : 0;
	dlg->m_NoSysMemPrimary = t->flags6 & NOSYSMEMPRIMARY ? 1 : 0;
	dlg->m_NoSysMemBackBuf = t->flags6 & NOSYSMEMBACKBUF ? 1 : 0;
	dlg->m_NoBlt = t->flags5 & NOBLT ? 1 : 0;
	dlg->m_FastBlt = t->flags5 & DOFASTBLT ? 1 : 0;
	dlg->m_GDIColorConv = t->flags5 & GDICOLORCONV ? 1 : 0;
	dlg->m_PreventMaximize = t->flags & PREVENTMAXIMIZE ? 1 : 0;
	dlg->m_ClientRemapping = t->flags & CLIENTREMAPPING ? 1 : 0;
	dlg->m_LockWinStyle = t->flags & LOCKWINSTYLE ? 1 : 0;
	dlg->m_FixParentWin = t->flags & FIXPARENTWIN ? 1 : 0;
	dlg->m_ModalStyle = t->flags2 & MODALSTYLE ? 1 : 0;
	dlg->m_KeepAspectRatio = t->flags2 & KEEPASPECTRATIO ? 1 : 0;
	dlg->m_ForceWinResize = t->flags2 & FORCEWINRESIZE ? 1 : 0;
	dlg->m_HideMultiMonitor = t->flags2 & HIDEMULTIMONITOR ? 1 : 0;
	dlg->m_FixD3DFrame = t->flags3 & FIXD3DFRAME ? 1 : 0;
	dlg->m_NoWindowMove = t->flags3 & NOWINDOWMOVE ? 1 : 0;
	dlg->m_HookChildWin = t->flags & HOOKCHILDWIN ? 1 : 0;
	dlg->m_MessageProc = t->flags & MESSAGEPROC ? 1 : 0;
	dlg->m_FixMouseHook = t->flags8 & FIXMOUSEHOOK ? 1 : 0;
	dlg->m_FixNCHITTEST = t->flags2 & FIXNCHITTEST ? 1 : 0;
	dlg->m_RecoverScreenMode = t->flags2 & RECOVERSCREENMODE ? 1 : 0;
	dlg->m_RefreshOnResize = t->flags2 & REFRESHONRESIZE ? 1 : 0;
	dlg->m_BackBufAttach = t->flags2 & BACKBUFATTACH ? 1 : 0;
	dlg->m_ClearTextureFourCC = t->flags7 & CLEARTEXTUREFOURCC ? 1 : 0;
	dlg->m_NoDDExclusiveMode = t->flags7 & NODDEXCLUSIVEMODE ? 1 : 0;
	dlg->m_CreateDesktop = t->flags6 & CREATEDESKTOP ? 1 : 0;
	dlg->m_AllowSysmemOn3DDev = t->flags8 & ALLOWSYSMEMON3DDEV ? 1 : 0;
	dlg->m_HandleAltF4 = t->flags & HANDLEALTF4 ? 1 : 0;
	dlg->m_LimitFPS = t->flags2 & LIMITFPS ? 1 : 0;
	dlg->m_SkipFPS = t->flags2 & SKIPFPS ? 1 : 0;
	dlg->m_ShowFPS = t->flags2 & SHOWFPS ? 1 : 0;
	dlg->m_ShowFPSOverlay = t->flags2 & SHOWFPSOVERLAY ? 1 : 0;
	dlg->m_ShowTimeStretch = t->flags4 & SHOWTIMESTRETCH ? 1 : 0;
	dlg->m_TimeStretch = t->flags2 & TIMESTRETCH ? 1 : 0;
	dlg->m_StretchTimers = t->flags4 & STRETCHTIMERS ? 1 : 0;
	dlg->m_NormalizePerfCount = t->flags5 & NORMALIZEPERFCOUNT ? 1 : 0;
	dlg->m_CPUSlowDown = t->flags7 & CPUSLOWDOWN ? 1 : 0;
	dlg->m_CPUMaxUsage = t->flags7 & CPUMAXUSAGE ? 1 : 0;
	dlg->m_QuarterBlt = t->flags5 & QUARTERBLT ? 1 : 0;
	dlg->m_FineTiming = t->flags4 & FINETIMING ? 1 : 0;
	dlg->m_ReleaseMouse = t->flags4 & RELEASEMOUSE ? 1 : 0;
	dlg->m_EnableTimeFreeze = t->flags4 & ENABLETIMEFREEZE ? 1 : 0;
	dlg->m_EnableHotKeys = t->flags4 & ENABLEHOTKEYS ? 1 : 0;
	dlg->m_InterceptRDTSC = t->flags4 & INTERCEPTRDTSC ? 1 : 0;
	dlg->m_HookOpenGL = t->flags2 & HOOKOPENGL ? 1 : 0;
	dlg->m_ForceHookOpenGL = t->flags3 & FORCEHOOKOPENGL ? 1 : 0;
	dlg->m_FixPixelZoom = t->flags6 & FIXPIXELZOOM ? 1 : 0;
	dlg->m_FixBindTexture = t->flags7 & FIXBINDTEXTURE ? 1 : 0;
	dlg->m_HookGlut32 = t->flags7 & HOOKGLUT32 ? 1 : 0;
	dlg->m_WireFrame = t->flags2 & WIREFRAME ? 1 : 0;
	dlg->m_NoTextures = t->flags4 & NOTEXTURES ? 1 : 0;
	dlg->m_BlackWhite = t->flags3 & BLACKWHITE ? 1 : 0;
	dlg->m_FakeVersion = t->flags2 & FAKEVERSION ? 1 : 0;
	dlg->m_FullRectBlt = t->flags2 & FULLRECTBLT ? 1 : 0;
	dlg->m_CenterToWin = t->flags5 & CENTERTOWIN ? 1 : 0;
	dlg->m_Deinterlace = t->flags5 & DEINTERLACE ? 1 : 0;
	dlg->m_LimitDdraw = t->flags7 & LIMITDDRAW ? 1 : 0;
	dlg->m_SuppressOverlay = t->flags7 & SUPPRESSOVERLAY ? 1 : 0;
	dlg->m_NoPaletteUpdate = t->flags2 & NOPALETTEUPDATE ? 1 : 0;
	dlg->m_SurfaceWarn = t->flags3 & SURFACEWARN ? 1 : 0;
	dlg->m_CapMask = t->flags3 & CAPMASK ? 1 : 0;
	dlg->m_NoWindowHooks = t->flags6 & NOWINDOWHOOKS ? 1 : 0;
	dlg->m_NoDDRAWBlt = t->flags3 & NODDRAWBLT ? 1 : 0;
	dlg->m_NoDDRAWFlip = t->flags3 & NODDRAWFLIP ? 1 : 0;
	dlg->m_NoGDIBlt = t->flags3 & NOGDIBLT ? 1 : 0;
	dlg->m_NoFillRect = t->flags4 & NOFILLRECT ? 1 : 0;
	dlg->m_FixClipperArea = t->flags7 & FIXCLIPPERAREA ? 1 : 0;
	dlg->m_SyncPalette = t->flags6 & SYNCPALETTE ? 1 : 0;
	dlg->m_NoWinErrors = t->flags7 & NOWINERRORS ? 1 : 0;
	dlg->m_PretendVisible = t->flags8 & PRETENDVISIBLE ? 1 : 0;
	dlg->m_WinInsulation = t->flags8 & WININSULATION ? 1 : 0;
	dlg->m_QualityFonts = t->flags8 & QUALITYFONTS ? 1 : 0;
	dlg->m_AnalyticMode = t->flags3 & ANALYTICMODE ? 1 : 0;
	dlg->m_ReplacePrivOps = t->flags5 & REPLACEPRIVOPS ? 1 : 0;
	dlg->m_InitialRes = t->flags7 & INITIALRES ? 1 : 0;
	dlg->m_MaximumRes = t->flags7 & MAXIMUMRES ? 1 : 0;
	dlg->m_PosX = t->posx;
	dlg->m_PosY = t->posy;
	dlg->m_SizX = t->sizx;
	dlg->m_SizY = t->sizy;
	dlg->m_ScanLine = t->ScanLine;
	dlg->m_MaxFPS = t->MaxFPS;
	dlg->m_InitTS = t->InitTS+8;
	dlg->m_FakeVersionId = t->FakeVersionId;
	dlg->m_MaxScreenRes = t->MaxScreenRes;
	dlg->m_InitResW = t->resw;
	dlg->m_InitResH = t->resh;
	dlg->m_SwapEffect = t->SwapEffect;
}

static void SaveConfigItem(TARGETMAP *TargetMap, PRIVATEMAP *PrivateMap, int i, char *InitPath)
{
	char key[32], val[32];
	char *EscBuf = NULL;
	sprintf_s(key, sizeof(key), "title%i", i);
	WritePrivateProfileString("target", key, PrivateMap->title, InitPath);
	// -------
	sprintf_s(key, sizeof(key), "path%i", i);
	WritePrivateProfileString("target", key, TargetMap->path, InitPath);
	// -------
	sprintf_s(key, sizeof(key), "startfolder%i", i);
	WritePrivateProfileString("target", key, PrivateMap->startfolder, InitPath);
	// -------
	sprintf_s(key, sizeof(key), "launchpath%i", i);
	WritePrivateProfileString("target", key, PrivateMap->launchpath, InitPath);
	// -------
	sprintf_s(key, sizeof(key), "module%i", i);
	WritePrivateProfileString("target", key, TargetMap->module, InitPath);
	// -------
	sprintf_s(key, sizeof(key), "opengllib%i", i);
	WritePrivateProfileString("target", key, TargetMap->OpenGLLib, InitPath);
	// -------
	sprintf_s(key, sizeof(key), "notes%i", i);
	WritePrivateProfileString("target", key, Escape(PrivateMap->notes, &EscBuf), InitPath);
	// -------
	sprintf_s(key, sizeof(key), "registry%i", i);
	WritePrivateProfileString("target", key, Escape(PrivateMap->registry, &EscBuf), InitPath);
	// -------
	sprintf_s(key, sizeof(key), "ver%i", i);
	sprintf_s(val, sizeof(val), "%i", TargetMap->dxversion);
	WritePrivateProfileString("target", key, val, InitPath);
	// -------
	sprintf_s(key, sizeof(key), "monitorid%i", i);
	sprintf_s(val, sizeof(val), "%i", TargetMap->monitorid);
	WritePrivateProfileString("target", key, val, InitPath);
	// -------
	sprintf_s(key, sizeof(key), "coord%i", i);
	sprintf_s(val, sizeof(val), "%i", TargetMap->coordinates);
	WritePrivateProfileString("target", key, val, InitPath);
	// -------
	sprintf_s(key, sizeof(key), "flag%i", i);
	sprintf_s(val, sizeof(val), "%i", TargetMap->flags);
	WritePrivateProfileString("target", key, val, InitPath);
	// -------
	sprintf_s(key, sizeof(key), "flagg%i", i);
	sprintf_s(val, sizeof(val), "%i", TargetMap->flags2);
	WritePrivateProfileString("target", key, val, InitPath);
	// -------
	sprintf_s(key, sizeof(key), "flagh%i", i);
	sprintf_s(val, sizeof(val), "%i", TargetMap->flags3);
	WritePrivateProfileString("target", key, val, InitPath);
	// -------
	sprintf_s(key, sizeof(key), "flagi%i", i);
	sprintf_s(val, sizeof(val), "%i", TargetMap->flags4);
	WritePrivateProfileString("target", key, val, InitPath);
	// -------
	sprintf_s(key, sizeof(key), "flagj%i", i);
	sprintf_s(val, sizeof(val), "%i", TargetMap->flags5);
	WritePrivateProfileString("target", key, val, InitPath);
	// -------
	sprintf_s(key, sizeof(key), "flagk%i", i);
	sprintf_s(val, sizeof(val), "%i", TargetMap->flags6);
	WritePrivateProfileString("target", key, val, InitPath);
	// -------
	sprintf_s(key, sizeof(key), "flagl%i", i);
	sprintf_s(val, sizeof(val), "%i", TargetMap->flags7);
	WritePrivateProfileString("target", key, val, InitPath);
	// -------
	sprintf_s(key, sizeof(key), "flagm%i", i);
	sprintf_s(val, sizeof(val), "%i", TargetMap->flags8);
	WritePrivateProfileString("target", key, val, InitPath);
	// -------
	sprintf_s(key, sizeof(key), "tflag%i", i);
	sprintf_s(val, sizeof(val), "%i", TargetMap->tflags);
	WritePrivateProfileString("target", key, val, InitPath);
	// -------
	sprintf_s(key, sizeof(key), "dflag%i", i);
	sprintf_s(val, sizeof(val), "%i", TargetMap->dflags);
	WritePrivateProfileString("target", key, val, InitPath);
	// -------
	sprintf_s(key, sizeof(key), "posx%i", i);
	sprintf_s(val, sizeof(val), "%i", TargetMap->posx);
	WritePrivateProfileString("target", key, val, InitPath);
	// -------
	sprintf_s(key, sizeof(key), "posy%i", i);
	sprintf_s(val, sizeof(val), "%i", TargetMap->posy);
	WritePrivateProfileString("target", key, val, InitPath);
	// -------
	sprintf_s(key, sizeof(key), "sizx%i", i);
	sprintf_s(val, sizeof(val), "%i", TargetMap->sizx);
	WritePrivateProfileString("target", key, val, InitPath);
	// -------
	sprintf_s(key, sizeof(key), "sizy%i", i);
	sprintf_s(val, sizeof(val), "%i", TargetMap->sizy);
	WritePrivateProfileString("target", key, val, InitPath);
	// -------
	sprintf_s(key, sizeof(key), "maxfps%i", i);
	sprintf_s(val, sizeof(val), "%i", TargetMap->MaxFPS);
	WritePrivateProfileString("target", key, val, InitPath);
	// -------
	sprintf_s(key, sizeof(key), "initts%i", i);
	sprintf_s(val, sizeof(val), "%i", TargetMap->InitTS);
	WritePrivateProfileString("target", key, val, InitPath);
	// -------
	sprintf_s(key, sizeof(key), "winver%i", i);
	sprintf_s(val, sizeof(val), "%i", TargetMap->FakeVersionId);
	WritePrivateProfileString("target", key, val, InitPath);
	// -------
	sprintf_s(key, sizeof(key), "maxres%i", i);
	sprintf_s(val, sizeof(val), "%i", TargetMap->MaxScreenRes);
	WritePrivateProfileString("target", key, val, InitPath);
	// -------
	sprintf_s(key, sizeof(key), "swapeffect%i", i);
	sprintf_s(val, sizeof(val), "%i", TargetMap->SwapEffect);
	WritePrivateProfileString("target", key, val, InitPath);
	// -------
	sprintf_s(key, sizeof(key), "maxddinterface%i", i);
	sprintf_s(val, sizeof(val), "%i", TargetMap->MaxDdrawInterface);
	WritePrivateProfileString("target", key, val, InitPath);
	// -------
	sprintf_s(key, sizeof(key), "slowratio%i", i);
	sprintf_s(val, sizeof(val), "%i", TargetMap->SlowRatio);
	WritePrivateProfileString("target", key, val, InitPath);
	// -------
	sprintf_s(key, sizeof(key), "scanline%i", i);
	sprintf_s(val, sizeof(val), "%i", TargetMap->ScanLine);
	WritePrivateProfileString("target", key, val, InitPath);
	// -------
	sprintf_s(key, sizeof(key), "initresw%i", i);
	sprintf_s(val, sizeof(val), "%i", TargetMap->resw);
	WritePrivateProfileString("target", key, val, InitPath);
	// -------
	sprintf_s(key, sizeof(key), "initresh%i", i);
	sprintf_s(val, sizeof(val), "%i", TargetMap->resh);
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
	sprintf_s(key, sizeof(key), "startfolder%i", i);
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
	sprintf_s(key, sizeof(key), "flagl%i", i);
	WritePrivateProfileString("target", key, 0, InitPath);
	sprintf_s(key, sizeof(key), "flagm%i", i);
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
	sprintf_s(key, sizeof(key), "maxddinterface%i", i);
	WritePrivateProfileString("target", key, 0, InitPath);
	sprintf_s(key, sizeof(key), "slowratio%i", i);
	WritePrivateProfileString("target", key, 0, InitPath);
}

static int LoadConfigItem(TARGETMAP *TargetMap, PRIVATEMAP *PrivateMap, int i, char *InitPath)
{
	char key[32];
	char *EscBuf = NULL;
	char *sBuf;
	sBuf = (char *)malloc(1000000);
	extern BOOL gbDebug;
	TargetMap->index = i;
	// -------
	sprintf_s(key, sizeof(key), "path%i", i);
	GetPrivateProfileString("target", key, "", TargetMap->path, MAX_PATH, InitPath);
	if(!TargetMap->path[0]) return FALSE;
	// -------
	sprintf_s(key, sizeof(key), "launchpath%i", i);
	GetPrivateProfileString("target", key, "", PrivateMap->launchpath, MAX_PATH, InitPath);
	// -------
	sprintf_s(key, sizeof(key), "startfolder%i", i);
	GetPrivateProfileString("target", key, "", PrivateMap->startfolder, MAX_PATH, InitPath);
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
	sprintf_s(key, sizeof(key), "monitorid%i", i);
	TargetMap->monitorid = GetPrivateProfileInt("target", key, 0, InitPath);
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
	sprintf_s(key, sizeof(key), "flagl%i", i);
	TargetMap->flags7 = GetPrivateProfileInt("target", key, 0, InitPath);
	// -------
	sprintf_s(key, sizeof(key), "flagm%i", i);
	TargetMap->flags8 = GetPrivateProfileInt("target", key, 0, InitPath);
	// -------
	sprintf_s(key, sizeof(key), "tflag%i", i);
	TargetMap->tflags = GetPrivateProfileInt("target", key, 0, InitPath);
	// -------
	sprintf_s(key, sizeof(key), "dflag%i", i);
	TargetMap->dflags = GetPrivateProfileInt("target", key, 0, InitPath);
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
	// -------
	sprintf_s(key, sizeof(key), "maxddinterface%i", i);
	TargetMap->MaxDdrawInterface = GetPrivateProfileInt("target", key, 7, InitPath);
	// -------
	sprintf_s(key, sizeof(key), "slowratio%i", i);
	TargetMap->SlowRatio = GetPrivateProfileInt("target", key, 1, InitPath);
	// -------
	sprintf_s(key, sizeof(key), "scanline%i", i);
	TargetMap->ScanLine = GetPrivateProfileInt("target", key, 1, InitPath);
	// -------
	sprintf_s(key, sizeof(key), "initresw%i", i);
	TargetMap->resw = GetPrivateProfileInt("target", key, 0, InitPath);
	// -------
	sprintf_s(key, sizeof(key), "initresh%i", i);
	TargetMap->resh = GetPrivateProfileInt("target", key, 0, InitPath);
	
	if (!gbDebug){
		// clear debug flags
		TargetMap->flags &= ~(0);
		TargetMap->flags3 &= ~(SURFACEWARN|ANALYTICMODE|NODDRAWBLT|NODDRAWFLIP|NOGDIBLT);
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
	if (tm.flags7 & HOOKNORUN) return 5; 
	if (tm.flags3 & HOOKENABLED) return ((tm.flags2 & STARTDEBUG)||(tm.flags7 & INJECTSUSPENDED)) ? 2 : 1;
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
	this->isRegistryUpdated=FALSE;
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
		if (MessageBoxLang(DXW_STRING_EXIT_BUSY, DXW_STRING_WARNING, MB_OKCANCEL | MB_ICONQUESTION)==IDCANCEL) 
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

void CDxwndhostView::OnHotKey(UINT nHotKeyId, UINT nKey1, UINT nKey2)
{ 
	switch(nHotKeyId){
		case 0: // minimize
			this->OnWindowMinimize();
			break;
		case 1: // restore
			this->OnWindowRestore();
			break;
		case 2: // kill
			this->OnProcessKill();
			break;
	}
}

void VKeyError(char *key)
{
	char Msg[81];
	sprintf(Msg, "RegisterHotKey(%s) failed err=%d", key, GetLastError());
	MessageBox(NULL, Msg, "DxWnd error", MB_OK+MB_ICONWARNING);
}

void CDxwndhostView::UpdateHotKeys()
{
	// Hot Keys
	DWORD dwKey;
	extern Key_Type HKeys[];
	extern KeyCombo_Type HKeyCombo[];
	// MessageBox("update global settings", "debug", MB_OK);
	for(int i=0; HKeys[i].iLabelResourceId; i++){
		dwKey = GetPrivateProfileInt("keymapping", HKeys[i].sIniLabel, 0, gInitPath);
		if(dwKey) {
			UnregisterHotKey(this->GetSafeHwnd(), i);
			if(!RegisterHotKey(this->GetSafeHwnd(), i, MOD_ALT+MOD_SHIFT+MOD_CONTROL, dwKey)) VKeyError(HKeys[i].sIniLabel);
		}
	}
}

void CDxwndhostView::OnInitialUpdate()
{
	CListView::OnInitialUpdate();
	CListCtrl& listctrl = GetListCtrl();
	LV_COLUMN listcol;
	LV_ITEM listitem;
	int i;
	typedef BOOL (WINAPI *ChangeWindowMessageFilter_Type)(UINT, DWORD);
	ChangeWindowMessageFilter_Type pChangeWindowMessageFilter;

	DragAcceptFiles();
	// the ChangeWindowMessageFilter is not available, nor necessary, on XP
	HMODULE hUser32;
	hUser32 = GetModuleHandle("user32.dll");
	pChangeWindowMessageFilter = (ChangeWindowMessageFilter_Type)GetProcAddress(hUser32, "ChangeWindowMessageFilter");
	if(pChangeWindowMessageFilter){
		(*pChangeWindowMessageFilter)(WM_DROPFILES, MSGFLT_ADD);
		(*pChangeWindowMessageFilter)(WM_COPYDATA, MSGFLT_ADD);
		(*pChangeWindowMessageFilter)(0x0049, MSGFLT_ADD);
	}

	// Create 256 color image lists
	HIMAGELIST hList = ImageList_Create(32,32, ILC_COLOR8 |ILC_MASK , 6, 1);
	m_cImageListNormal.Attach(hList);

	hList = ImageList_Create(16, 16, ILC_COLOR8 | ILC_MASK, 6, 1);
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
	this->isRegistryUpdated=FALSE;
	pTitles = &PrivateMaps[0];
	pTargets= &TargetMaps[0];

	UpdateHotKeys();

	// Transient mode
	if(gTransientMode){
		this->OnRun();
	}
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
		// XP fix:
		if(strlen(path)>4){
			char *p;
			p = &path[strlen(path)-4];
			if(strcasecmp(p, ".dxw")) strcat(path, ".dxw");
		}
		else
			strcat(path, ".dxw");
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

BOOL CDxwndhostView::OnImport(CString sFilePath)
{
	LV_ITEM listitem;
	int i;
	for (i=0; strlen(TargetMaps[i].path) && i<MAXTARGETS; i++)
		;
	if (i==MAXTARGETS) {
		MessageBoxLang(DXW_STRING_MAXENTRIES, DXW_STRING_WARNING, MB_OK | MB_ICONEXCLAMATION);
		return FALSE;
	}
	CListCtrl& listctrl = GetListCtrl();
	if(LoadConfigItem(&TargetMaps[i], &PrivateMaps[i], 0, (char *)sFilePath.GetString())){
		listitem.mask = LVIF_TEXT | LVIF_IMAGE;
		listitem.iItem = i;
		listitem.iSubItem = 0;
		listitem.iImage = SetTargetIcon(TargetMaps[i]);
		listitem.pszText = PrivateMaps[i].title;
		listctrl.InsertItem(&listitem);
	}
	Resize();
	SetTarget(TargetMaps);	
	this->isUpdated=TRUE;
	this->isRegistryUpdated=TRUE;
	return TRUE;
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
			while(*p){
				// "p" - name of each file, NULL to terminate
				if(!*p) break;
				if(i==MAXTARGETS) {
					MessageBoxLang(DXW_STRING_MAXENTRIES, DXW_STRING_WARNING, MB_OK | MB_ICONEXCLAMATION);
					break;
				}
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
		this->isRegistryUpdated=TRUE;
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
	dlg.m_StartFolder = PrivateMaps[i].startfolder;
	SetDlgFromTarget(&TargetMaps[i], &dlg);
	if(dlg.DoModal() == IDOK && dlg.m_FilePath.GetLength()){
		strnncpy(PrivateMaps[i].title, (char *)dlg.m_Title.GetString(), MAX_TITLE); 
		PrivateMaps[i].notes = (char *)realloc(PrivateMaps[i].notes, strlen(dlg.m_Notes.GetString())+1);
		strcpy(PrivateMaps[i].notes, (char *)dlg.m_Notes.GetString());
		if(strcmp((char *)dlg.m_Registry.GetString(), PrivateMaps[i].registry)) this->isRegistryUpdated=TRUE;
		PrivateMaps[i].registry = (char *)realloc(PrivateMaps[i].registry, strlen(dlg.m_Registry.GetString())+1);
		strcpy(PrivateMaps[i].registry, (char *)dlg.m_Registry.GetString());
		strnncpy(PrivateMaps[i].launchpath, (char *)dlg.m_LaunchPath.GetString(), MAX_PATH);
		strnncpy(PrivateMaps[i].startfolder, (char *)dlg.m_StartFolder.GetString(), MAX_PATH);
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
	HINSTANCE ret;

	CListCtrl& listctrl = GetListCtrl();

	if(!listctrl.GetSelectedCount()) return;
	pos = listctrl.GetFirstSelectedItemPosition();
	i = listctrl.GetNextSelectedItem(pos);
	FilePath = TargetMaps[i].path;
	len=FilePath.ReverseFind('\\');	
	if (len==0) return;
	FilePath.Truncate(len);
	//MessageBox(FilePath, "path", 0);

	extern BOOL IsWinXP(void);
	ret = ShellExecute(NULL, IsWinXP() ? "open" : "explore", FilePath, NULL, NULL, SW_SHOW);
	if((int)ret <= 32){
		char message[MAX_PATH];
		sprintf(message, "ShellExecute(\"%s\", \"%s\") failed.\nerror=%d",
			IsWinXP() ? "open" : "explore", FilePath, ret);
		MessageBox(message, "error", 0);
	}
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
	if(PrivateMaps[i].startfolder[0]){
		FilePath = PrivateMaps[i].startfolder;
		FilePath.Append("\\dxwnd.log");
	}
	else {
		FilePath = TargetMaps[i].path;
		len=FilePath.ReverseFind('\\');	
		if (len==0) return;
		FilePath.Truncate(len);
		FilePath.Append("\\dxwnd.log");
	}
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

void CDxwndhostView::OnSetPath() 
{
	int i;
	CTargetDlg dlg;
	POSITION pos;
	char *lpProcName, *lpNext;
	extern BOOL GetProcByName(char *, char *);
	char FullPath[MAX_PATH+1];
	char sMessage[1000];

	CListCtrl& listctrl = GetListCtrl();

	if(!listctrl.GetSelectedCount()) return;
	pos = listctrl.GetFirstSelectedItemPosition();
	i = listctrl.GetNextSelectedItem(pos);
	//if(TargetMaps[i].path[0] != '*') return;
	lpProcName = &(TargetMaps[i].path[1]);
	while (lpNext=strchr(lpProcName,'\\')) lpProcName=lpNext+1;
	strcpy(FullPath,"");

	if(GetProcByName(lpProcName, FullPath)==0){
		int res;
		if(strcmp(TargetMaps[i].path, FullPath)){
			sprintf(sMessage, "Found replacement for \"%s\", "
				"full path = \"%s\", "
				"do you want to update the configuration?", lpProcName, FullPath);
			res = MessageBox(sMessage, "DxWnd set path", MB_YESNO);
			if(res!=IDYES) return;
			strcpy(TargetMaps[i].path, FullPath);
			this->isUpdated=TRUE;
		}
		else {
			sprintf(sMessage, "Path \"%s\" was already set.", FullPath);
			MessageBox(sMessage, "DxWnd set path", MB_OK);
		}
	}
	else{
		sprintf(sMessage, "Replacement for \"%s\" not found, run the game!", lpProcName);
		MessageBox(sMessage, "DxWnd set path", MB_ICONEXCLAMATION);
	}
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
		MessageBoxLang(DXW_STRING_VREG_ERROR, DXW_STRING_ERROR, MB_ICONERROR|MB_OK);
		return;
	}

	fwrite(Registry, strlen(Registry), 1, regfp);
	fputs("\n", regfp);
	fclose(regfp);
}

void CDxwndhostView::OnSort() 
{
	int i, itemcount;
	TARGETMAP MapEntry;
	PRIVATEMAP TitEntry;
	BOOL swapped;
	HRESULT res;
	CListCtrl& listctrl = GetListCtrl();

	// find list entries number and ignore the command when less than 2 
	for(i=0; i<MAXTARGETS; i++) if(strlen(TargetMaps[i].path)==0) break;
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
		TargetMaps[i].index = i; // renumber
	}

	SetTarget(TargetMaps);
	this->isUpdated=TRUE;
	this->isRegistryUpdated=TRUE;
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
	if ((GetHookStatus(&DxWndStatus) != DXW_RUNNING) /*|| (DxWndStatus.hWnd==NULL)*/ ) {
		MessageBoxLang(DXW_STRING_NOPAUSETASK, DXW_STRING_INFO, MB_ICONEXCLAMATION);
	}
	else {
		wchar_t *wcstring = new wchar_t[48+1];
		mbstowcs_s(NULL, wcstring, 48, PrivateMaps[DxWndStatus.OrigIdx].title, _TRUNCATE);
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
	if ((GetHookStatus(&DxWndStatus) != DXW_RUNNING) /*|| (DxWndStatus.hWnd==NULL)*/) {
		MessageBoxLang(DXW_STRING_NORESUMETASK, DXW_STRING_INFO, MB_ICONEXCLAMATION);
	}
	else {
		wchar_t *wcstring = new wchar_t[48+1];
		mbstowcs_s(NULL, wcstring, 48, PrivateMaps[DxWndStatus.OrigIdx].title, _TRUNCATE);
		res=MessageBoxLangArg(DXW_STRING_RESUMETASK, DXW_STRING_WARNING, MB_YESNO | MB_ICONQUESTION, wcstring);
		if(res!=IDYES) return;
		PauseResumeThreadList(DxWndStatus.dwPid, TRUE);
	}
}

extern HWND find_main_window(unsigned long);

static void SendMessageToHookedWin(DWORD message)
{
	DXWNDSTATUS DxWndStatus;
	HWND TargethWnd;
	if (GetHookStatus(&DxWndStatus) == DXW_RUNNING){
		TargethWnd = (DxWndStatus.hWnd) ? DxWndStatus.hWnd : find_main_window(DxWndStatus.dwPid);
		::PostMessage(TargethWnd, WM_SYSCOMMAND, message, 0);
	}
}

void CDxwndhostView::OnWindowMinimize() 
{
	SendMessageToHookedWin(SC_MINIMIZE);
}

void CDxwndhostView::OnWindowRestore() 
{
	SendMessageToHookedWin(SC_RESTORE);
}

void CDxwndhostView::OnWindowClose() 
{
	SendMessageToHookedWin(SC_CLOSE);
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
	if ((GetHookStatus(&DxWndStatus) != DXW_RUNNING) /*|| (DxWndStatus.hWnd==NULL)*/) {
		MessageBoxLang(DXW_STRING_NOKILLTASK, DXW_STRING_INFO, MB_ICONEXCLAMATION);
	}
	else {
		wchar_t *wcstring = new wchar_t[48+1];
		mbstowcs_s(NULL, wcstring, 48, PrivateMaps[DxWndStatus.OrigIdx].title, _TRUNCATE);
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

	if(TargetMaps[i].flags7 & COPYNOSHIMS){
		strcat(lpProcName, ".noshim");
	}

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
	OnAdd(NULL);
}

void CDxwndhostView::OnAdd(char *sInitialPath) 
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
	if(sInitialPath) dlg.m_FilePath = CString(sInitialPath);
	
	dlg.m_Coordinates = GetPrivateProfileInt("window", "defaultcoord", 0, gInitPath);
	dlg.m_PosX = GetPrivateProfileInt("window", "defaultposx", 50, gInitPath);
	dlg.m_PosY = GetPrivateProfileInt("window", "defaultposy", 50, gInitPath);
	dlg.m_SizX = GetPrivateProfileInt("window", "defaultsizx", 800, gInitPath);
	dlg.m_SizY = GetPrivateProfileInt("window", "defaultsizy", 600, gInitPath);

	if(dlg.DoModal() == IDOK && dlg.m_FilePath.GetLength()){
		strnncpy(PrivateMaps[i].title, (char *)dlg.m_Title.GetString(), MAX_TITLE);
		PrivateMaps[i].notes = (char *)malloc(strlen(dlg.m_Notes.GetString())+1);
		strcpy(PrivateMaps[i].notes, (char *)dlg.m_Notes.GetString());
		PrivateMaps[i].registry = (char *)malloc(strlen(dlg.m_Registry.GetString())+1);
		strcpy(PrivateMaps[i].registry, (char *)dlg.m_Registry.GetString());
		strnncpy(PrivateMaps[i].launchpath, (char *)dlg.m_LaunchPath.GetString(), MAX_PATH);
		strnncpy(PrivateMaps[i].startfolder, (char *)dlg.m_StartFolder.GetString(), MAX_PATH);
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
			if(GetPrivateProfileInt("window", "namefromfolder", 0, gInitPath)){
				FilePath=FilePath.Left(len);
				len=FilePath.ReverseFind('\\');	
			}
			FilePath=FilePath.Right(FilePath.GetLength()-len-1);
			strncpy_s(PrivateMaps[i].title, sizeof(PrivateMaps[i].title), FilePath.GetString(), sizeof(PrivateMaps[i].title)-1);
		}
		TargetMaps[i].index = i;
		listitem.pszText = PrivateMaps[i].title;
		listctrl.InsertItem(&listitem);
		Resize();
		SetTarget(TargetMaps);	
		this->isUpdated=TRUE;
		this->isRegistryUpdated=TRUE;
	}
}

void CDxwndhostView::OnDuplicate() 
{
	POSITION pos;
	int i;
	CListCtrl& listctrl = GetListCtrl();

	if(!(pos = listctrl.GetFirstSelectedItemPosition())) return;
	i = listctrl.GetNextSelectedItem(pos);
	if (listctrl.GetItemCount()>=MAXTARGETS-1) return; // too many entries to duplicate one!

	for(int j=listctrl.GetItemCount(); j>i; j--){
		TargetMaps[j+1]=TargetMaps[j];
		PrivateMaps[j+1]=PrivateMaps[j];
	}
	TargetMaps[i+1] = TargetMaps[i];
	PrivateMaps[i+1] = PrivateMaps[i];

	listctrl.DeleteAllItems();
	for(i=0; TargetMaps[i].path[0]; i++) {
		LV_ITEM listitem;
		listitem.mask = LVIF_TEXT | LVIF_IMAGE;
		listitem.iItem = i;
		listitem.iSubItem = 0;
		listitem.iImage = SetTargetIcon(TargetMaps[i]);
		listitem.pszText = PrivateMaps[i].title;
		listctrl.SetItem(&listitem);
		listctrl.InsertItem(&listitem);
		TargetMaps[i].index = i;
	}
	SetTarget(TargetMaps);
	this->isUpdated=TRUE;
	this->isRegistryUpdated=TRUE;
}

void CDxwndhostView::OnMoveTop() 
{
	TARGETMAP MapEntry;
	PRIVATEMAP TitEntry;
	POSITION pos;
	int i;
	CListCtrl& listctrl = GetListCtrl();

	if(!(pos = listctrl.GetFirstSelectedItemPosition())) return;
	i = listctrl.GetNextSelectedItem(pos);
	if (i==0)return;

	MapEntry=TargetMaps[i];
	TitEntry=PrivateMaps[i];
	for(int j=i; j; j--){
		TargetMaps[j]=TargetMaps[j-1];
		PrivateMaps[j]=PrivateMaps[j-1];
	}
	TargetMaps[0]=MapEntry;
	PrivateMaps[0]=TitEntry;

	listctrl.DeleteAllItems();
	for(i=0; TargetMaps[i].path[0]; i++) {
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
	this->isRegistryUpdated=TRUE;
}

void CDxwndhostView::OnMoveUp() 
{
	TARGETMAP MapEntry;
	PRIVATEMAP TitEntry;
	POSITION pos;
	int i;
	CListCtrl& listctrl = GetListCtrl();

	if(!(pos = listctrl.GetFirstSelectedItemPosition())) return;
	i = listctrl.GetNextSelectedItem(pos);
	if (i==0)return;

	MapEntry=TargetMaps[i-1];
	TargetMaps[i-1]=TargetMaps[i];
	TargetMaps[i]=MapEntry;
	TitEntry=PrivateMaps[i-1];
	PrivateMaps[i-1]=PrivateMaps[i];
	PrivateMaps[i]=TitEntry;

	listctrl.DeleteAllItems();
	for(i=0; TargetMaps[i].path[0]; i++) {
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
	this->isRegistryUpdated=TRUE;
}

void CDxwndhostView::OnMoveDown() 
{
	TARGETMAP MapEntry;
	PRIVATEMAP TitEntry;
	POSITION pos;
	int i;
	CListCtrl& listctrl = GetListCtrl();

	if(!(pos = listctrl.GetFirstSelectedItemPosition())) return;
	i = listctrl.GetNextSelectedItem(pos);
	if (i==listctrl.GetItemCount()-1)return;

	MapEntry=TargetMaps[i+1];
	TargetMaps[i+1]=TargetMaps[i];
	TargetMaps[i]=MapEntry;
	TitEntry=PrivateMaps[i+1];
	PrivateMaps[i+1]=PrivateMaps[i];
	PrivateMaps[i]=TitEntry;

	listctrl.DeleteAllItems();
	for(i=0; TargetMaps[i].path[0]; i++) {
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
	this->isRegistryUpdated=TRUE;
}

void CDxwndhostView::OnMoveBottom() 
{
	TARGETMAP MapEntry;
	PRIVATEMAP TitEntry;
	POSITION pos;
	int i, last;
	CListCtrl& listctrl = GetListCtrl();

	if(!(pos = listctrl.GetFirstSelectedItemPosition())) return;
	i = listctrl.GetNextSelectedItem(pos);
	last = listctrl.GetItemCount()-1;
	if (i==last)return;

	MapEntry=TargetMaps[i];
	TitEntry=PrivateMaps[i];
	for(int j=i; j<last; j++){
		TargetMaps[j]=TargetMaps[j+1];
		PrivateMaps[j]=PrivateMaps[j+1];
	}
	TargetMaps[last]=MapEntry;
	PrivateMaps[last]=TitEntry;

	listctrl.DeleteAllItems();
	for(i=0; TargetMaps[i].path[0]; i++) {
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
	this->isRegistryUpdated=TRUE;
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
	TargetMaps[i].path[0]=0; // clear last one, in case there were MAXTARGETS entries
	Resize();
	SetTarget(TargetMaps);
	this->isUpdated=TRUE;
	this->isRegistryUpdated=TRUE;
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

DWORD WINAPI WaitForDeath(LPVOID arg)
{
	while(!gMustDie) Sleep(1000);
	exit(0);
}

DWORD WINAPI TrayIconUpdate(CSystemTray *Tray)
{
	int DxStatus, PrevDxStatus;
	UINT IconId;
	char sMsg[1024];
	char *Status;
	char DllVersion[21];
	int TickCount, IdleCount;
	PrevDxStatus=-1; // a different one...
	TickCount=0;
	IdleCount=0;
	while (TRUE) {
		// once a second ...
		Sleep(1000);
		if(gTransientMode && gMustDie) {
			Tray->HideIcon();
			delete(Tray->GetAncestor(GA_ROOTOWNER));
			exit(0);
		}
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
			IdleCount++;
			if(IdleCount == 2) {
				if(gAutoHideMode) {
					Tray->MaximiseFromTray(pParent, FALSE);				}
			}
		}
		else {
			// animation state machine ....
			TickCount++;
			IdleCount=0;
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
			pTitles[DxWndStatus.OrigIdx].title,
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
	CMenu *menu = this->GetParent()->GetMenu();
	UINT IconId;
	pParent = this->GetParent();
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
			// error path: if can't create a system tray icon, transient logic must be silently placed here
			if(gTransientMode) StatusThread= CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)WaitForDeath, (LPVOID)NULL, 0, &dwThrdId);
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

void CDxwndhostView::OnViewShims()
{
	POSITION pos;
	int i;
	char *ExePath;
	CListCtrl& listctrl = GetListCtrl();

	if(!(pos = listctrl.GetFirstSelectedItemPosition())) return;
	i = listctrl.GetNextSelectedItem(pos);
	ExePath = TargetMaps[i].path;

	CShimsDialog *pDlg = new CShimsDialog(NULL, ExePath);
	BOOL ret = pDlg->Create(CShimsDialog::IDD, this); 
	pDlg->ShowWindow(SW_SHOW);
}

TARGETMAP *ViewTarget; // dirty !!!!

void CDxwndhostView::OnViewFlags() 
{
	int i;
	CTargetDlg dlg;
	POSITION pos;
	CListCtrl& listctrl = GetListCtrl();

	if(!listctrl.GetSelectedCount()) return;
	pos = listctrl.GetFirstSelectedItemPosition();
	i = listctrl.GetNextSelectedItem(pos);
	ViewTarget = &TargetMaps[i];

	CViewFlagsDialog *pDlg = new CViewFlagsDialog();

	BOOL ret = pDlg->Create(CViewFlagsDialog::IDD, this); 
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

void CDxwndhostView::OnGlobalSettings() 
{
	CGlobalSettings *pDlg = new CGlobalSettings();
	BOOL ret = pDlg->Create(CGlobalSettings::IDD, this); 
	pDlg->ShowWindow(SW_SHOW);
}

void CDxwndhostView::OnExpertModeToggle() 
{
	CMenu *menu;
	gbDebug = !gbDebug;
	menu = this->GetParent()->GetMenu();
	::DestroyMenu(menu->GetSafeHmenu());
	menu->LoadMenu(gbDebug ? IDR_MAINFRAME_EX : IDR_MAINFRAME);	
	this->GetParent()->SetMenu(menu);
	menu->CheckMenuItem(ID_OPTIONS_EXPERTMODE, gbDebug ? MF_CHECKED : MF_UNCHECKED);
	WritePrivateProfileString("window", "debug", (gbDebug ? "1" : "0"), gInitPath);

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
		TargetMaps[i].index = i; // renumber the items
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
	popup.LoadMenu(gbDebug ? IDR_MENU_POPUP_EX : IDR_MENU_POPUP);

	res = popup.GetSubMenu(0)->TrackPopupMenu(TPM_LEFTALIGN | TPM_TOPALIGN | TPM_LEFTBUTTON | TPM_RETURNCMD, point.x, point.y, this);
	switch(res){
	case ID_PRUN:
		OnRun();
		break;
	case ID_RUNUNHOOKED:
		OnRun(TRUE);
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
	case ID_PDUPLICATE:
		OnDuplicate();
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
	case ID_SETPATH:
		OnSetPath();
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
	case ID_TOOLS_RECOVERSCREENMODE:
		OnRecoverScreenMode();
		break;
	case ID_TOOLS_CLEARCOMPATIBILITYFLAGS:
		OnClearCompatibilityFlags();
		break;
	case ID_MOVE_TOP:
		OnMoveTop();
		break;
	case ID_MOVE_UP:
		OnMoveUp();
		break;
	case ID_MOVE_DOWN:
		OnMoveDown();
		break;
	case ID_MOVE_BOTTOM:
		OnMoveBottom();
		break;
	case ID_VIEW_FLAGS:
		OnViewFlags();
		break;
	}
	CListView::OnRButtonDown(nFlags, point);
}

typedef struct {
	char *path;
	char *launch;
} PROCESSMAP;

static DWORD WINAPI TransientWaitForChildDeath(void *p)
{
	PROCESSMAP *ProcessMap = (PROCESSMAP *)p;
	BOOL bIsSomeoneAlive;
	int ret;
	char sPath[MAX_PATH];
	char sLaunch[MAX_PATH];
	char sTemp[MAX_PATH];
	char *lpProcName, *lpNext;

	// strip full pathname and keep executable name only
	strcpy(sPath, "");
	if(ProcessMap->path[0]){
		strncpy(sTemp, ProcessMap->path, MAX_PATH); 
		lpProcName=sTemp;
		while (lpNext=strchr(lpProcName,'\\')) lpProcName=lpNext+1;
		strncpy(sPath, lpProcName, MAX_PATH);
	}
	strcpy(sLaunch, "");
	if(ProcessMap->launch[0]){
		strncpy(sTemp, ProcessMap->launch, MAX_PATH); 
		lpProcName=sTemp;
		while (lpNext=strchr(lpProcName,'\\')) lpProcName=lpNext+1;
		strncpy(sLaunch, lpProcName, MAX_PATH);
	}

	Sleep(3000); // Wait for process creation - necessary?

	while(TRUE){
		Sleep(2000);
		bIsSomeoneAlive = FALSE;
		if(sPath[0]) {
			if (!(ret=KillProcByName(sPath, FALSE))) bIsSomeoneAlive = TRUE;
		}
		if(sLaunch[0]) {
			if (!(ret=KillProcByName(sLaunch, FALSE))) bIsSomeoneAlive = TRUE;
		}
		if(!bIsSomeoneAlive) {
			break;
		}
	}
	gMustDie = TRUE;
	return 0;
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

static char *sEventCode(DWORD ec)
{
	char *c;
	switch(ec){
		case EXCEPTION_DEBUG_EVENT:				c="exception"; break;
		case CREATE_THREAD_DEBUG_EVENT:			c="create thread"; break;
		case CREATE_PROCESS_DEBUG_EVENT:		c="create process"; break;
		case EXIT_THREAD_DEBUG_EVENT:			c="exit thread"; break;
		case EXIT_PROCESS_DEBUG_EVENT:			c="exit process"; break;
		case LOAD_DLL_DEBUG_EVENT:				c="load dll"; break;
		case UNLOAD_DLL_DEBUG_EVENT:			c="unload dll"; break;
		case OUTPUT_DEBUG_STRING_EVENT:			c="out debug"; break;
		case RIP_EVENT:							c="rip"; break;
		default:								c="unknown"; break;
	}
	return c;
}

// For thread messaging
#define DEBUG_EVENT_MESSAGE		WM_APP + 0x100

DWORD WINAPI StartDebug(void *p)
{
	ThreadInfo_Type *ThInfo;
	STARTUPINFO sinfo;
	PROCESS_INFORMATION pinfo;
	char path[MAX_PATH];
	extern char *GetFileNameFromHandle(HANDLE);
	char *sRunTargetPath;
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
	sRunTargetPath = ThInfo->ExePath;
	strcpy_s(path, sizeof(path), sRunTargetPath);
	PathRemoveFileSpec(path);
	if(!CreateProcess(NULL, 
		sRunTargetPath, 
		0, 0, false, DEBUG_PROCESS|DEBUG_ONLY_THIS_PROCESS, NULL, path, &sinfo, &pinfo)){
		DWORD dwLastErr = GetLastError();
		switch (dwLastErr){
			case 740:
				sprintf(DebugMessage, "Create process error=%d: DxWnd must run as administrator", dwLastErr);
				MessageBoxEx(0, DebugMessage, "ERROR", MB_ICONEXCLAMATION|MB_OK, NULL);
				if(gbDebug) OutTrace("%s\n", DebugMessage);
				break;
			default:
				sprintf(DebugMessage, "CREATE PROCESS error=%d", dwLastErr);
				MessageBoxEx(0, DebugMessage, "ERROR", MB_ICONEXCLAMATION|MB_OK, NULL);
				if(gbDebug) OutTrace("%s\n", DebugMessage);
				break;
		}
	}

	if(gbDebug) OutTrace("create process: path=\"%s\"\n", path);
	CString strEventMessage;
	DEBUG_EVENT debug_event ={0};
	bContinueDebugging = true;
	DWORD dwContinueStatus = DBG_CONTINUE;
	while(bContinueDebugging)
	{ 
		dwContinueStatus = DBG_EXCEPTION_NOT_HANDLED;
		if (!WaitForDebugEvent(&debug_event, INFINITE)) {
			sprintf(DebugMessage, "Injection error: WaitForDebugEvent error=%d",  GetLastError());
			MessageBoxEx(0, DebugMessage, "Injection", MB_ICONEXCLAMATION|MB_OK, NULL);
			if(gbDebug) OutTrace("%s\n", DebugMessage);
			break; // must release pinfo handles
		}
		//OutTrace("Injection: WaitForDebugEvent code=%d(%s)\n", debug_event.dwDebugEventCode, sEventCode(debug_event.dwDebugEventCode));
		switch(debug_event.dwDebugEventCode){
		case EXIT_PROCESS_DEBUG_EVENT:
			if(gbDebug) OutTrace("exit process\n", DebugMessage);
			bContinueDebugging=false;
			break;
		case CREATE_PROCESS_DEBUG_EVENT:
			if (gbDebug) OutTrace("create process: base=0x%X path=\"%s\"\n", 
				debug_event.u.CreateProcessInfo.lpBaseOfImage,
				GetFileNameFromHandle(debug_event.u.CreateProcessInfo.hFile));
			GetFullPathName("dxwnd.dll", MAX_PATH, path, NULL);
			if(!Inject(pinfo.dwProcessId, path)){
				// DXW_STRING_INJECTION
				sprintf(DebugMessage,"Injection error: pid=%x dll=%s", pinfo.dwProcessId, path);
				MessageBoxEx(0, DebugMessage, "Injection", MB_ICONEXCLAMATION|MB_OK, NULL);
				if (gbDebug) OutTrace("%s\n", DebugMessage);
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
						MessageBoxEx(0, DebugMessage, "Injection", MB_ICONEXCLAMATION|MB_OK, NULL);
						if (gbDebug) OutTrace("%s\n", DebugMessage);
					}
					if(!WriteProcessMemory(pinfo.hProcess, StartAddress, &EndlessLoop, 4, &BytesCount)){
						sprintf(DebugMessage,"WriteProcessMemory error=%d", GetLastError());
						MessageBoxEx(0, DebugMessage, "Injection", MB_ICONEXCLAMATION|MB_OK, NULL);
						if (gbDebug) OutTrace("%s\n", DebugMessage);
					}
				}
			}
#endif
			CloseHandle(debug_event.u.CreateProcessInfo.hProcess);
			CloseHandle(debug_event.u.CreateProcessInfo.hThread);
			CloseHandle(debug_event.u.CreateProcessInfo.hFile);
			break;
		case CREATE_THREAD_DEBUG_EVENT:
			if (gbDebug) OutTrace("create thread: th=0x%X base=0x%X start=0x%X\n", 
				debug_event.u.CreateThread.hThread,
				debug_event.u.CreateThread.lpThreadLocalBase,
				debug_event.u.CreateThread.lpStartAddress);
			//SuspendThread(debug_event.u.CreateThread.hThread);
			CloseHandle(debug_event.u.CreateThread.hThread);
			break;
		case EXIT_THREAD_DEBUG_EVENT:
			if (gbDebug) OutTrace("exit thread: exitcode=0x%X\n", 
				debug_event.u.ExitThread.dwExitCode);
#ifdef LOCKINJECTIONTHREADS
			if(TargetHandle && StartAddress){
				if(!WriteProcessMemory(pinfo.hProcess, StartAddress, &StartingCode, 4, &BytesCount)){
					sprintf(DebugMessage,"WriteProcessMemory error=%d", GetLastError());
					MessageBoxEx(0, DebugMessage, "Injection", MB_ICONEXCLAMATION|MB_OK, NULL);
					if (gbDebug) OutTrace("%s\n", DebugMessage);
				}
			}
			if(TargetHandle) CloseHandle((HANDLE)TargetHandle);
#endif
			// commented out: apparently, there must be some previous thread that starts before the hooker
			// bContinueDebugging=false;
			break;
		case LOAD_DLL_DEBUG_EVENT:
			if (gbDebug) OutTrace("load dll: base=0x%X path=\"%s\"\n", 
				debug_event.u.LoadDll.lpBaseOfDll,
				GetFileNameFromHandle(debug_event.u.LoadDll.hFile));
			CloseHandle(debug_event.u.LoadDll.hFile);
			break;
		case UNLOAD_DLL_DEBUG_EVENT:
			if (gbDebug) OutTrace("unload dll: base=0x%X\n", 
				debug_event.u.UnloadDll.lpBaseOfDll);
			break;
		case OUTPUT_DEBUG_STRING_EVENT: 				
			if (gbDebug) OutTrace("output debug: len=%d unicode=%x\n", 
				debug_event.u.DebugString.nDebugStringLength,
				debug_event.u.DebugString.fUnicode);
			break;
		case EXCEPTION_DEBUG_EVENT:
			//sprintf(DebugMessage, "Exception %x(%s) caught at addr=%x",
			//	debug_event.u.Exception.ExceptionRecord.ExceptionCode, 
			//	ExceptionCaption(debug_event.u.Exception.ExceptionRecord.ExceptionCode),
			//	debug_event.u.Exception.ExceptionRecord.ExceptionAddress);
			//MessageBoxEx(0, DebugMessage, "EXCEPTION", MB_ICONEXCLAMATION, NULL);
			if (gbDebug) OutTrace("exception: code=0x%X(%s) addr=0x%X first=%x\n", 
				debug_event.u.Exception.ExceptionRecord.ExceptionCode, 
				ExceptionCaption(debug_event.u.Exception.ExceptionRecord.ExceptionCode),
				debug_event.u.Exception.ExceptionRecord.ExceptionAddress,
				debug_event.u.Exception.dwFirstChance);
			break;
		default:
			sprintf(DebugMessage,"Unknown eventcode=%x", debug_event.dwDebugEventCode);
			MessageBoxEx(0, DebugMessage, "Injection", MB_ICONEXCLAMATION, NULL);
			if (gbDebug) OutTrace("%s\n", DebugMessage);
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

void InjectSuspended(char *exepath, char *dirpath)
{
	STARTUPINFO sinfo;
	PROCESS_INFORMATION pinfo;
	char StartingCode[4];
	DWORD EndlessLoop;
	EndlessLoop=0x9090FEEB; // careful: it's BIG ENDIAN: EB FE 90 90
	DWORD BytesCount;
	char DebugMessage[1024];
	DWORD OldProt;
	DWORD PEHeader[0x70];
	char dllpath[MAX_PATH];
	LPVOID StartAddress;
	HANDLE TargetHandle;
	FILE *fExe = NULL;
	BOOL bKillProcess = FALSE;

	OutTrace("InjectSuspended: exe=\"%s\" dir=\"%s\"\n",exepath, dirpath);
	ZeroMemory(&sinfo, sizeof(sinfo));
	sinfo.cb = sizeof(sinfo);
	// attempt to load the specified target
	if (!CreateProcess(NULL, exepath, 0, 0, false, CREATE_SUSPENDED, NULL, dirpath, &sinfo, &pinfo)){
		sprintf(DebugMessage,"CreateProcess \"%s\" error=%d", exepath, GetLastError());
		MessageBoxEx(0, DebugMessage, "Injection", MB_ICONEXCLAMATION|MB_OK, NULL);
		if (gbDebug) OutTrace("%s\n", DebugMessage);
	}

	while(TRUE) { // fake loop
		bKillProcess = TRUE;

		// locate the entry point
		TargetHandle = OpenProcess(
			PROCESS_QUERY_INFORMATION|PROCESS_VM_OPERATION|PROCESS_VM_READ|PROCESS_VM_WRITE|PROCESS_SUSPEND_RESUME, 
			FALSE, 
			pinfo.dwProcessId);
		if (gbDebug) OutTrace("Target handle=%x\n", TargetHandle);

		FILE *fExe = fopen(exepath, "rb");
		if(fExe==NULL){
			sprintf(DebugMessage,"fopen %s error=%d", exepath, GetLastError());
			MessageBoxEx(0, DebugMessage, "Injection", MB_ICONEXCLAMATION|MB_OK, NULL);
			if (gbDebug) OutTrace("%s\n", DebugMessage);
			break;
		}
		// read DOS header
		if(fread((void *)PEHeader, sizeof(DWORD), 0x10, fExe)!=0x10){
			sprintf(DebugMessage,"fread error=%d", GetLastError());
			MessageBoxEx(0, DebugMessage, "Injection", MB_ICONEXCLAMATION|MB_OK, NULL);
			if (gbDebug) OutTrace("%s\n", DebugMessage);
			break;
		}
		OutTrace("NT Header offset=%X\n", PEHeader[0xF]);
		fseek(fExe, PEHeader[0xF], 0);
		// read File header + Optional header
		if(fread((void *)PEHeader, sizeof(DWORD), 0x70, fExe)!=0x70){
			sprintf(DebugMessage,"fread error=%d", GetLastError());
			MessageBoxEx(0, DebugMessage, "Injection", MB_ICONEXCLAMATION|MB_OK, NULL);
			if (gbDebug) OutTrace("%s\n", DebugMessage);
			break;
		}

		OutTrace("AddressOfEntryPoint=%X ImageBase=%X\n", PEHeader[0xA], PEHeader[0xD]);
		StartAddress = (LPVOID)(PEHeader[0xA] + PEHeader[0xD]);
		if (gbDebug) OutTrace("Thread start address=%x\n", StartAddress);

		// patch the entry point with infinite loop
		if(!VirtualProtectEx(TargetHandle, StartAddress, 4, PAGE_EXECUTE_READWRITE, &OldProt )){
			sprintf(DebugMessage,"VirtualProtectEx error=%d", GetLastError());
			MessageBoxEx(0, DebugMessage, "Injection", MB_ICONEXCLAMATION|MB_OK, NULL);
			if (gbDebug) OutTrace("%s\n", DebugMessage);
			break;
		}

		if(!ReadProcessMemory(TargetHandle, StartAddress, &StartingCode, 4, &BytesCount)){ 
			sprintf(DebugMessage,"ReadProcessMemory error=%d", GetLastError());
			MessageBoxEx(0, DebugMessage, "Injection", MB_ICONEXCLAMATION|MB_OK, NULL);
			if (gbDebug) OutTrace("%s\n", DebugMessage);
			break;
		}

		if(!WriteProcessMemory(TargetHandle, StartAddress, &EndlessLoop, 4, &BytesCount)){
			sprintf(DebugMessage,"WriteProcessMemory error=%d", GetLastError());
			MessageBoxEx(0, DebugMessage, "Injection", MB_ICONEXCLAMATION|MB_OK, NULL);
			if (gbDebug) OutTrace("%s\n", DebugMessage);
			break;
		}

		if(!FlushInstructionCache(TargetHandle, StartAddress, 4)){
			sprintf(DebugMessage,"FlushInstructionCache error=%d", GetLastError());
			MessageBoxEx(0, DebugMessage, "Injection", MB_ICONEXCLAMATION|MB_OK, NULL);
			if (gbDebug) OutTrace("%s\n", DebugMessage);
			break; // error condition
		}

		// resume the main thread
		if(ResumeThread(pinfo.hThread)==(DWORD)-1){
			sprintf(DebugMessage,"ResumeThread error=%d at:%d", GetLastError(), __LINE__);
			MessageBoxEx(0, DebugMessage, "Injection", MB_ICONEXCLAMATION|MB_OK, NULL);
			if (gbDebug) OutTrace("%s\n", DebugMessage);
			break;
		}

		// wait until the thread stuck at entry point
		CONTEXT context;
		context.Eip = (DWORD)0; // initialize to impossible value
		for ( unsigned int i = 0; i < 80 && context.Eip != (DWORD)StartAddress; ++i ){
			// patience.
			Sleep(50);

			// read the thread context
			context.ContextFlags = CONTEXT_CONTROL;
			if(!GetThreadContext(pinfo.hThread, &context)){
				sprintf(DebugMessage,"GetThreadContext error=%d", GetLastError());
				MessageBoxEx(0, DebugMessage, "Injection", MB_ICONEXCLAMATION|MB_OK, NULL);
				if (gbDebug) OutTrace("%s\n", DebugMessage);
				break;
			}
			OutTrace("wait cycle %d eip=%x\n", i, context.Eip);
		}

		if (context.Eip != (DWORD)StartAddress){
			// wait timed out
			sprintf(DebugMessage,"thread blocked eip=%x addr=%x", context.Eip, StartAddress);
			MessageBoxEx(0, DebugMessage, "Injection", MB_ICONEXCLAMATION|MB_OK, NULL);
			if (gbDebug) OutTrace("%s\n", DebugMessage);
		}

		// inject DLL payload into remote process
		GetFullPathName("dxwnd.dll", MAX_PATH, dllpath, NULL);
		if(!Inject(pinfo.dwProcessId, dllpath)){
			// DXW_STRING_INJECTION
			sprintf(DebugMessage,"Injection error: pid=%x dll=%s", pinfo.dwProcessId, dllpath);
			MessageBoxEx(0, DebugMessage, "Injection", MB_ICONEXCLAMATION|MB_OK, NULL);
			if (gbDebug) OutTrace("%s\n", DebugMessage);
		}

		// pause 
		if(SuspendThread(pinfo.hThread)==(DWORD)-1){
			sprintf(DebugMessage,"SuspendThread error=%d", GetLastError());
			MessageBoxEx(0, DebugMessage, "Injection", MB_ICONEXCLAMATION|MB_OK, NULL);
			if (gbDebug) OutTrace("%s\n", DebugMessage);
			break;
		}
		// restore original entry point
		if(!WriteProcessMemory(TargetHandle, StartAddress, &StartingCode, 4, &BytesCount)){
			sprintf(DebugMessage,"WriteProcessMemory error=%d", GetLastError());
			MessageBoxEx(0, DebugMessage, "Injection", MB_ICONEXCLAMATION|MB_OK, NULL);
			if (gbDebug) OutTrace("%s\n", DebugMessage);
			throw;
		}

		if(!FlushInstructionCache(TargetHandle, StartAddress, 4)){
			sprintf(DebugMessage,"FlushInstructionCache error=%d", GetLastError());
			MessageBoxEx(0, DebugMessage, "Injection", MB_ICONEXCLAMATION|MB_OK, NULL);
			if (gbDebug) OutTrace("%s\n", DebugMessage);
			break; // error condition
		}

		// you are ready to go
		// pause and restore original entry point
		if(ResumeThread(pinfo.hThread)==(DWORD)-1){
			sprintf(DebugMessage,"ResumeThread error=%d at:%d", GetLastError(), __LINE__);
			MessageBoxEx(0, DebugMessage, "Injection", MB_ICONEXCLAMATION|MB_OK, NULL);
			if (gbDebug) OutTrace("%s\n", DebugMessage);
			break;
		}
		bKillProcess = FALSE;
		break; // exit fake loop
	}

	// cleanup ....
	if(fExe) fclose(fExe);
	if(TargetHandle) CloseHandle(TargetHandle);
	// terminate the newly spawned process
	if(bKillProcess){
		if(!TerminateProcess( pinfo.hProcess, -1 )){
			if (gbDebug) OutTrace("failed to kill hproc=%x err=%d\n", pinfo.hProcess, GetLastError());
		}
	}
}

DWORD RecoverTargetMaps(LPVOID TargetMaps)
{
	Sleep(5000);
	SetTarget((TARGETMAP *)TargetMaps);	
	return 0;
}

// from https://www.winehq.org/pipermail/wine-users/2002-April/007910.html 
//
// There is no publicaly available version numbering for SafeDisc. However, it 
// seems that the version number is stored in the executable as 3 unsigned 32-bit 
// integers. Using an hexadecimal editor, locate the following byte pattern in 
// the wrapper (game.exe)
//
// > 426f475f 202a3930 2e302621 21202059   BoG_ *90.0&!!  Y
// > 793e0000                              y>..
//
// There should be 3 unsigned integers right after that, which are respectively 
// the version, subversion an revision number.
//
// On some versions of SafeDisc there are 3 null integers following the pattern, 
// before the version number. You'll then have to look at the 3 unsigned 32-bit 
// integers right after
//
// > 426f475f 202a3930 2e302621 21202059   BoG_ *90.0&!!  Y
// > 793e0000 00000000 00000000 00000000   y>..............

static void CheckSafeDiscVersion(char *path)
{
	unsigned char opcode[44+1];
	char sMessage[81];
	DWORD dwVersion, dwSubversion, dwRevision;
	FILE *exe;
	long seekpos;

	exe=fopen(path, "rb");
	if(!exe) return;

	while(TRUE) {
		seekpos=ftell(exe); 
		if(fread(opcode, 1, 44, exe) != 44) break;
		//MessageBox(0, "check", "debug", 0);
		// fast way to make 20 char comparisons .....
		if(*(DWORD *)opcode     ==0x5F476F42)
		if(*(DWORD *)(opcode+4) ==0x30392A20)
		if(*(DWORD *)(opcode+8) ==0x2126302E)
		if(*(DWORD *)(opcode+12)==0x59202021)
		if(*(DWORD *)(opcode+16)==0x00003E79){
			dwVersion = *(DWORD *)(opcode+20);
			dwSubversion = *(DWORD *)(opcode+24);
			dwRevision = *(DWORD *)(opcode+28);
			if(dwVersion == 0){
				dwVersion = *(DWORD *)(opcode+32);
				dwSubversion = *(DWORD *)(opcode+36);
				dwRevision = *(DWORD *)(opcode+40);	
			}
			sprintf(sMessage,  "Safedisk version %d.%d.%d detected", dwVersion, dwSubversion, dwRevision);
			MessageBox(NULL, sMessage, "DxWnd hint", MB_OKCANCEL|MB_ICONWARNING|MB_TOPMOST);
			break;
		}
		if(fseek(exe, seekpos+4, SEEK_SET)) break;
	}
	fclose(exe);
}

#define FILECOPYBUFSIZE 1024

static void MakeHiddenFile(char *sTargetPath)
{
	HANDLE hFile     = INVALID_HANDLE_VALUE;
    HANDLE hTempFile = INVALID_HANDLE_VALUE; 
    BOOL fSuccess  = FALSE;
    DWORD dwRetVal = 0;
    UINT uRetVal   = 0;
    DWORD dwBytesRead    = 0;
    DWORD dwBytesWritten = 0; 
    char  chBuffer[FILECOPYBUFSIZE]; 

	hFile = CreateFile(sTargetPath,           // file name 
		GENERIC_READ,          // open for reading 
		0,                     // do not share 
		NULL,                  // default security 
		OPEN_EXISTING,         // existing file only 
		FILE_ATTRIBUTE_NORMAL, // normal file 
		NULL);                 // no template 
    if (hFile == INVALID_HANDLE_VALUE) { 
        //PrintError(TEXT("First CreateFile failed"));
        return;
    }

	strcat(sTargetPath, ".noshim");
	
    //  Deletes last copy of the file, just in case it was updated (patched?)
	// DeleteFile(sTargetPath);

    //  Creates the new file to write to for the upper-case version.
    hTempFile = CreateFile((LPTSTR) sTargetPath,	// file name 
		GENERIC_WRITE,			// open for write 
		0,						// do not share 
		NULL,					// default security 
		CREATE_ALWAYS,			// overwrite existing
		//FILE_ATTRIBUTE_HIDDEN,	// hidden file 
		FILE_ATTRIBUTE_NORMAL, // normal file 
		NULL);					// no template 
    if (hTempFile == INVALID_HANDLE_VALUE) {
		char msg[120];
		sprintf(msg, "Creation of hidden copy of target file with no SHIMs failed\nError=%d", GetLastError());
		MessageBox(0, msg, "Warning", 0); 
        //PrintError(TEXT("Second CreateFile failed"));
        if (!CloseHandle(hFile)){
			sprintf(msg, "CloseHandle failed\nError=%d", GetLastError());
            MessageBox(0, msg, "Error", 0);
        }
        return;
    } 
    //  Reads BUFSIZE blocks to the buffer and copy to the temporary 
    //  file. 
    do {
        if (ReadFile(hFile, chBuffer, FILECOPYBUFSIZE, &dwBytesRead, NULL)) {
            fSuccess = WriteFile(hTempFile, chBuffer, dwBytesRead, &dwBytesWritten, NULL); 
            if (!fSuccess) {
                //PrintError(TEXT("WriteFile failed"));
                break;
            }
        } 
        else {
            //PrintError(TEXT("ReadFile failed"));
            break;
        }
    //  Continues until the whole file is processed.
    } while (dwBytesRead == FILECOPYBUFSIZE); 

	CloseHandle(hFile); 
	CloseHandle(hTempFile); 
}

PROCESSMAP pm;

void CDxwndhostView::OnRun() 
{
	OnRun(FALSE); 
}

void CDxwndhostView::OnRun(BOOL bForceNoHook) 
{
	CListCtrl& listctrl = GetListCtrl();
	POSITION pos;
	int i;
	STARTUPINFO sinfo;
	PROCESS_INFORMATION pinfo;
	char folderpath[MAX_PATH+20]; // max + space for ".noshim"
	char exepath[MAX_PATH+20]; // max + space for ".noshim"
	TARGETMAP RestrictedMaps[2];
	char *sRunTargetPath;

	if(gTransientMode){
		i=iProgIndex-1;
		if(i<0) i=0;
	}
	else {
		if(!listctrl.GetSelectedCount()) return;
		pos = listctrl.GetFirstSelectedItemPosition();
		i = listctrl.GetNextSelectedItem(pos);
	}
	sRunTargetPath = (strlen(PrivateMaps[i].launchpath)>0) ? PrivateMaps[i].launchpath : TargetMaps[i].path;
	ZeroMemory(&sinfo, sizeof(sinfo));
	sinfo.cb = sizeof(sinfo);
	// create a virtually single entry in the targetmap array
	memcpy(&RestrictedMaps[0], &TargetMaps[i], sizeof(TARGETMAP));
	memset(&RestrictedMaps[1], 0, sizeof(TARGETMAP));
	if(!(PrivateMaps[i].startfolder[0])){
		strcpy_s(folderpath, sizeof(folderpath), sRunTargetPath);
		strcpy_s(exepath, sizeof(exepath), sRunTargetPath);
		PathRemoveFileSpec(folderpath);
	}else{
		strcpy_s(folderpath, sizeof(folderpath), PrivateMaps[i].startfolder);
		strcpy_s(exepath, sizeof(exepath), sRunTargetPath);
	}
	if(TargetMaps[i].flags7 & COPYNOSHIMS){
		MakeHiddenFile(exepath);
		strncpy(RestrictedMaps[0].path, exepath, MAX_PATH);
	}
	if(bForceNoHook) RestrictedMaps[0].flags3 &= ~HOOKENABLED;
	SetTarget(RestrictedMaps);	
	OutTrace("OnRun idx=%d prog=\"%s\" unhooked=%x\n", i, TargetMaps[i].path, bForceNoHook);

	if(TargetMaps[i].flags7 & HOOKNORUN){
		MessageBoxLang(DXW_STRING_CANT_RUN, DXW_STRING_WARNING, MB_ICONERROR|MB_OK);
		return;
	}

	// self-elevation if configured and necessary
	if(TargetMaps[i].flags & NEEDADMINCAPS){
		extern BOOL DxSelfElevate(CDxwndhostView *);
		OSVERSIONINFO osver = { sizeof(osver) };
		if (GetVersionEx(&osver) && (osver.dwMajorVersion >= 6)){
			OutTrace("self elevation\n");
			DxSelfElevate(this);
		}
	}

	if((TargetMaps[i].flags3 & EMULATEREGISTRY) || (TargetMaps[i].flags4 & OVERRIDEREGISTRY)){
		if(this->isRegistryUpdated){
			if(MessageBoxLang(DXW_STRING_VREG_UPDATE,DXW_STRING_WARNING, MB_OKCANCEL|MB_ICONINFORMATION)==IDOK)
				this->SaveConfigFile();
		}
		OutTrace("export virtual registry entry[%d]\n");
		FILE *regfp;
		char *Registry;
		Registry = PrivateMaps[i].registry;
		regfp=fopen("dxwnd.reg", "w");
		if(regfp==NULL)MessageBoxLang(DXW_STRING_VREG_ERROR, DXW_STRING_ERROR, MB_ICONERROR|MB_OK);
		fwrite(Registry, strlen(Registry), 1, regfp);
		fputs("\n", regfp);
		fclose(regfp);	
	}

	if(TargetMaps[i].flags7 & SHOWHINTS){
		OutTrace("checksafediscversion\n");
		CheckSafeDiscVersion(TargetMaps[i].path);
	}

	// v2.04.04: fix - STARTDEBUG and INJECTSUSPENDED must take place only when HOOKENABLED
	if ((TargetMaps[i].flags2 & STARTDEBUG) && 
		(TargetMaps[i].flags3 & HOOKENABLED)){
		OutTrace("debugger mode\n");
		ThreadInfo_Type ThreadInfo;
		ThreadInfo.TM=&TargetMaps[i];
		ThreadInfo.PM=&PrivateMaps[i];
		ThreadInfo.ExePath=exepath;
		CloseHandle(CreateThread( NULL, 0, StartDebug, &ThreadInfo, 0, NULL)); 
	}
	else
	if ((TargetMaps[i].flags7 & INJECTSUSPENDED) &&
		(TargetMaps[i].flags3 & HOOKENABLED)){
		OutTrace("injectsuspended mode\n");
		InjectSuspended(exepath, folderpath); 
	}
	else{
		OutTrace("setwindowshook mode\n");
		CreateProcess(NULL, 
			exepath, 
			0, 0, false, CREATE_DEFAULT_ERROR_MODE, NULL, folderpath, &sinfo, &pinfo);
		CloseHandle(pinfo.hProcess); // no longer needed, avoid handle leakage
		CloseHandle(pinfo.hThread); // no longer needed, avoid handle leakage
	}

	if(gTransientMode){
		pm.launch = &(PrivateMaps[i].launchpath[0]);
		pm.path   = &(TargetMaps[i].path[0]);
		CreateThread( NULL, 0, TransientWaitForChildDeath, &pm, 0, NULL); 
	}

	// wait & recover
	CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)RecoverTargetMaps, (LPVOID)TargetMaps, 0, NULL);
	// go to tray icon mode when autohide is set, but only if hooking is enabled. If not enable it 
	// is not possible to monitor the process status, so the window will never show automatically!
	if(gAutoHideMode && !bForceNoHook) this->OnGoToTrayIcon();

	// not working: the file is opened, can't be deleted
	//if(TargetMaps[i].flags7 & COPYNOSHIMS){
	//	DeleteFile(exepath);
	//}
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
	SwitchToColorDepth(8);
}

void CDxwndhostView::OnDesktopcolordepth16bpp()
{
	SwitchToColorDepth(16);
}

void CDxwndhostView::OnDesktopcolordepth24bpp()
{
	SwitchToColorDepth(24);
}

void CDxwndhostView::OnDesktopcolordepth32bpp()
{
	SwitchToColorDepth(32);
}

void CDxwndhostView::OnRecoverScreenMode()
{
	RevertScreenChanges(&this->InitDevMode);
}

void CDxwndhostView::OnClearCompatibilityFlags()
{
	int i;
	POSITION pos;
	CListCtrl& listctrl = GetListCtrl();
	CString	FilePath;
	HRESULT res;
	char sMessage[500];
	DWORD lKeyLength;
	LONG ret;

	if(!listctrl.GetSelectedCount()) return ;
	pos = listctrl.GetFirstSelectedItemPosition();
	i = listctrl.GetNextSelectedItem(pos);

	//sprintf(sMessage, "Clear all compatibility flags for \"%s\"?", PrivateMaps[i].title);
	//res=MessageBox(sMessage, "DxWnd", MB_YESNO | MB_ICONQUESTION);
	res=MessageBoxLangArg(DXW_STRING_CLEAR_COMP, DXW_STRING_DXWND, MB_YESNO | MB_ICONQUESTION, PrivateMaps[i].title);
	if(res!=IDYES) return;

	FilePath=TargetMaps[i].path;
	if (FilePath.GetLength()==0) return;

	HKEY hk;
	lKeyLength = 0L;
	DWORD lType;
	ret=RegOpenKeyEx(HKEY_CURRENT_USER, 
		"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\AppCompatFlags\\Layers", 0, KEY_ALL_ACCESS, &hk);
	if(ret){
		if(ret != ERROR_FILE_NOT_FOUND){		
			sprintf(sMessage, "error %ld in RegOpenKeyEx HKEY_CURRENT_USER", ret);
			MessageBox(sMessage, "DxWnd", MB_OK);
		}
	}
	else {
		ret=RegQueryValueEx(hk, TargetMaps[i].path, NULL, &lType, NULL, &lKeyLength);
		if(ret && (ret != ERROR_FILE_NOT_FOUND)){
			sprintf(sMessage, "error %ld in RegQueryValue hk=%lx \"%s\"", ret, hk, TargetMaps[i].path);
			MessageBox(sMessage, "DxWnd", MB_OK);
		}
		if(lKeyLength > 0L) {
			MessageBox("Found compatibility flag in HKEY_CURRENT_USER", "DxWnd", MB_OK);
			if(ret = RegDeleteValue(hk, TargetMaps[i].path)) {
				sprintf(sMessage, "error %ld in RegDeleteValue hk=%lx \"%s\"", ret, hk, TargetMaps[i].path);
				MessageBox(sMessage, "DxWnd", MB_OK);
			}
		}
		RegCloseKey(hk);
	}

	hk=0L;
	lKeyLength = 0L;
	ret=RegOpenKeyEx(HKEY_LOCAL_MACHINE, 
		"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\AppCompatFlags\\Layers", 0, KEY_ALL_ACCESS, &hk);
	if(ret){
		if(ret != ERROR_FILE_NOT_FOUND){
			sprintf(sMessage, "error %ld in RegOpenKeyEx HKEY_LOCAL_MACHINE", ret);
			MessageBox(sMessage, "DxWnd", MB_OK);
		}
	}
	else{
		ret=RegQueryValueEx(hk, TargetMaps[i].path, NULL, &lType, NULL, &lKeyLength);
		if(ret && (ret != ERROR_FILE_NOT_FOUND)){
				sprintf(sMessage, "error %ld in RegQueryValue hk=%lx \"%s\"", ret, hk, TargetMaps[i].path);
				MessageBox(sMessage, "DxWnd", MB_OK);
		}
		if(lKeyLength > 0L) {
			MessageBox("Found compatibility flag in HKEY_LOCAL_MACHINE", "DxWnd", MB_OK);
			if(ret = RegDeleteValue(hk, TargetMaps[i].path)) {
				sprintf(sMessage, "error %ld in RegDeleteValue hk=%lx \"%s\"", ret, hk, TargetMaps[i].path);
				MessageBox(sMessage, "DxWnd", MB_OK);
			}
		}
		RegCloseKey(hk);
	}
}

void CDxwndhostView::OnDropFiles(HDROP dropInfo)
{
	CString sFile;
	DWORD nBuffer = 0;
	char *p;
	// Get number of files
	UINT nFilesDropped = DragQueryFile(dropInfo, 0xFFFFFFFF, NULL, 0);
	if(nFilesDropped > 0){
		nBuffer = DragQueryFile(dropInfo, 0, NULL, 0);
		DragQueryFile(dropInfo, 0, sFile.GetBuffer(nBuffer+1), nBuffer+1);
		p = (char *)sFile.GetString();
		p += (strlen(p)-4);
		if(!_strnicmp(p, ".dxw", 4)){
			for(UINT i=1; i<=nFilesDropped; i++){
				if(!this->OnImport(sFile)) break;
				nBuffer = DragQueryFile(dropInfo, i, NULL, 0);
				DragQueryFile(dropInfo, i, sFile.GetBuffer(nBuffer+1), nBuffer+1);
				p = (char *)sFile.GetString();
				p += (strlen(p)-4);
				if(_strnicmp(p, ".dxw", 4)) break;
			}
		}
		else {
			this->OnAdd(sFile.GetBuffer());
		}
		sFile.ReleaseBuffer();
	}
	DragFinish(dropInfo);
}

