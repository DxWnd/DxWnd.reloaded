#define _CRT_SECURE_NO_DEPRECATE 1

// dxwndhostView.cpp : CDxwndhostView Define the behavior of the class.
//

#include "stdafx.h"
#include "shlwapi.h"

#include "dxwndhost.h"

#include "dxwndhostDoc.h"
#include "dxwndhostView.h"
#include "TargetDlg.h"
#include "SystemTray.h"
#include "StatusDialog.h"
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
	ON_COMMAND(ID_ADD, OnAdd)
	ON_COMMAND(ID_MODIFY, OnModify)
	ON_COMMAND(ID_DELETE, OnDelete)
	ON_COMMAND(ID_FILE_SORTPROGRAMSLIST, OnSort)
	ON_COMMAND(ID_FILE_CLEARALLLOGS, OnClearAllLogs)
	ON_COMMAND(ID_FILE_GOTOTRAYICON, OnGoToTrayIcon)
	ON_COMMAND(ID_FILE_SAVE, OnSaveFile)
	ON_COMMAND(ID_HOOK_START, OnHookStart)
	ON_COMMAND(ID_HOOK_STOP, OnHookStop)
	ON_COMMAND(ID_DXAPP_EXIT, OnExit)
	ON_WM_RBUTTONDOWN()
	ON_WM_DESTROY()
	ON_COMMAND(ID_RUN, OnRun)
	ON_COMMAND(ID_TRAY_RESTORE, OnTrayRestore)
	ON_COMMAND(ID_VIEW_STATUS, OnViewStatus)
	ON_COMMAND(ID_VIEW_TIMESLIDER, OnViewTimeSlider)
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

/////////////////////////////////////////////////////////////////////////////
// CDxwndhostView class constructor / destructor

CDxwndhostView::CDxwndhostView()
{
//	EnumDisplaySettings(NULL, ENUM_REGISTRY_SETTINGS, &this->InitDevMode);
	EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &this->InitDevMode);
}

void CDxwndhostView::SaveConfigFile()
{
	int i;
	char key[32], val[32];

	for(i = 0; i < MAXTARGETS; i ++){
		if(!TargetMaps[i].path[0]) break;
		sprintf_s(key, sizeof(key), "title%i", i);
		WritePrivateProfileString("target", key, TitleMaps[i].title, InitPath);
		sprintf_s(key, sizeof(key), "path%i", i);
		WritePrivateProfileString("target", key, TargetMaps[i].path, InitPath);
		sprintf_s(key, sizeof(key), "module%i", i);
		WritePrivateProfileString("target", key, TargetMaps[i].module, InitPath);
		sprintf_s(key, sizeof(key), "opengllib%i", i);
		WritePrivateProfileString("target", key, TargetMaps[i].OpenGLLib, InitPath);
		sprintf_s(key, sizeof(key), "ver%i", i);
		sprintf_s(val, sizeof(val), "%i", TargetMaps[i].dxversion);
		WritePrivateProfileString("target", key, val, InitPath);
		sprintf_s(key, sizeof(key), "flag%i", i);
		sprintf_s(val, sizeof(val), "%i", TargetMaps[i].flags);
		WritePrivateProfileString("target", key, val, InitPath);
		sprintf_s(key, sizeof(key), "flagg%i", i);
		sprintf_s(val, sizeof(val), "%i", TargetMaps[i].flags2);
		WritePrivateProfileString("target", key, val, InitPath);
		sprintf_s(key, sizeof(key), "tflag%i", i);
		sprintf_s(val, sizeof(val), "%i", TargetMaps[i].tflags);
		WritePrivateProfileString("target", key, val, InitPath);
		sprintf_s(key, sizeof(key), "initx%i", i);
		sprintf_s(val, sizeof(val), "%i", TargetMaps[i].initx);
		WritePrivateProfileString("target", key, val, InitPath);
		sprintf_s(key, sizeof(key), "inity%i", i);
		sprintf_s(val, sizeof(val), "%i", TargetMaps[i].inity);
		WritePrivateProfileString("target", key, val, InitPath);
		sprintf_s(key, sizeof(key), "minx%i", i);
		sprintf_s(val, sizeof(val), "%i", TargetMaps[i].minx);
		WritePrivateProfileString("target", key, val, InitPath);
		sprintf_s(key, sizeof(key), "miny%i", i);
		sprintf_s(val, sizeof(val), "%i", TargetMaps[i].miny);
		WritePrivateProfileString("target", key, val, InitPath);
		sprintf_s(key, sizeof(key), "maxx%i", i);
		sprintf_s(val, sizeof(val), "%i", TargetMaps[i].maxx);
		WritePrivateProfileString("target", key, val, InitPath);
		sprintf_s(key, sizeof(key), "maxy%i", i);
		sprintf_s(val, sizeof(val), "%i", TargetMaps[i].maxy);
		WritePrivateProfileString("target", key, val, InitPath);
		sprintf_s(key, sizeof(key), "posx%i", i);
		sprintf_s(val, sizeof(val), "%i", TargetMaps[i].posx);
		WritePrivateProfileString("target", key, val, InitPath);
		sprintf_s(key, sizeof(key), "posy%i", i);
		sprintf_s(val, sizeof(val), "%i", TargetMaps[i].posy);
		WritePrivateProfileString("target", key, val, InitPath);
		sprintf_s(key, sizeof(key), "sizx%i", i);
		sprintf_s(val, sizeof(val), "%i", TargetMaps[i].sizx);
		WritePrivateProfileString("target", key, val, InitPath);
		sprintf_s(key, sizeof(key), "sizy%i", i);
		sprintf_s(val, sizeof(val), "%i", TargetMaps[i].sizy);
		WritePrivateProfileString("target", key, val, InitPath);
		sprintf_s(key, sizeof(key), "maxfps%i", i);
		sprintf_s(val, sizeof(val), "%i", TargetMaps[i].MaxFPS);
		WritePrivateProfileString("target", key, val, InitPath);
		sprintf_s(key, sizeof(key), "initts%i", i);
		sprintf_s(val, sizeof(val), "%i", TargetMaps[i].InitTS);
		WritePrivateProfileString("target", key, val, InitPath);
	}
	for(; i < MAXTARGETS; i ++){
		sprintf_s(key, sizeof(key), "path%i", i);
		WritePrivateProfileString("target", key, 0, InitPath);
		sprintf_s(key, sizeof(key), "ver%i", i);
		WritePrivateProfileString("target", key, 0, InitPath);
		sprintf_s(key, sizeof(key), "flag%i", i);
		WritePrivateProfileString("target", key, 0, InitPath);
		sprintf_s(key, sizeof(key), "flagg%i", i);
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
	char key[32];

	listcol.mask = LVCF_WIDTH;
	listcol.cx = 100;
	
	listctrl.InsertColumn(0, &listcol);
	GetCurrentDirectory(MAX_PATH, InitPath);
	strcat_s(InitPath, sizeof(InitPath), "\\");
	strcat_s(InitPath, sizeof(InitPath), m_ConfigFileName);
	for(i = 0; i < MAXTARGETS; i ++){
		sprintf_s(key, sizeof(key), "path%i", i);
		GetPrivateProfileString("target", key, "", TargetMaps[i].path, MAX_PATH, InitPath);
		if(!TargetMaps[i].path[0]) break;
		sprintf_s(key, sizeof(key), "title%i", i);
		GetPrivateProfileString("target", key, "", TitleMaps[i].title, sizeof(TitleMaps[i].title)-1, InitPath);
		sprintf_s(key, sizeof(key), "module%i", i);
		GetPrivateProfileString("target", key, "", TargetMaps[i].module, sizeof(TargetMaps[i].module)-1, InitPath);
		sprintf_s(key, sizeof(key), "opengllib%i", i);
		GetPrivateProfileString("target", key, "", TargetMaps[i].OpenGLLib, sizeof(TargetMaps[i].OpenGLLib)-1, InitPath);
		sprintf_s(key, sizeof(key), "ver%i", i);
		TargetMaps[i].dxversion = GetPrivateProfileInt("target", key, 0, InitPath);
		sprintf_s(key, sizeof(key), "flag%i", i);
		TargetMaps[i].flags = GetPrivateProfileInt("target", key, 0, InitPath);
		sprintf_s(key, sizeof(key), "flagg%i", i);
		TargetMaps[i].flags2 = GetPrivateProfileInt("target", key, 0, InitPath);
		sprintf_s(key, sizeof(key), "tflag%i", i);
		TargetMaps[i].tflags = GetPrivateProfileInt("target", key, 0, InitPath);
		sprintf_s(key, sizeof(key), "initx%i", i);
		TargetMaps[i].initx = GetPrivateProfileInt("target", key, 0, InitPath);
		sprintf_s(key, sizeof(key), "inity%i", i);
		TargetMaps[i].inity = GetPrivateProfileInt("target", key, 0, InitPath);
		sprintf_s(key, sizeof(key), "minx%i", i);
		TargetMaps[i].minx = GetPrivateProfileInt("target", key, 0, InitPath);
		sprintf_s(key, sizeof(key), "miny%i", i);
		TargetMaps[i].miny = GetPrivateProfileInt("target", key, 0, InitPath);
		sprintf_s(key, sizeof(key), "maxx%i", i);
		TargetMaps[i].maxx = GetPrivateProfileInt("target", key, 0, InitPath);
		sprintf_s(key, sizeof(key), "maxy%i", i);
		TargetMaps[i].maxy = GetPrivateProfileInt("target", key, 0, InitPath);
		sprintf_s(key, sizeof(key), "posx%i", i);
		TargetMaps[i].posx = GetPrivateProfileInt("target", key, 0, InitPath);
		sprintf_s(key, sizeof(key), "posy%i", i);
		TargetMaps[i].posy = GetPrivateProfileInt("target", key, 0, InitPath);
		sprintf_s(key, sizeof(key), "sizx%i", i);
		TargetMaps[i].sizx = GetPrivateProfileInt("target", key, 0, InitPath);
		sprintf_s(key, sizeof(key), "sizy%i", i);
		TargetMaps[i].sizy = GetPrivateProfileInt("target", key, 0, InitPath);
		sprintf_s(key, sizeof(key), "maxfps%i", i);
		TargetMaps[i].MaxFPS = GetPrivateProfileInt("target", key, 0, InitPath);
		sprintf_s(key, sizeof(key), "initts%i", i);
		TargetMaps[i].InitTS = GetPrivateProfileInt("target", key, 0, InitPath);
		listitem.mask = LVIF_TEXT;
		listitem.iItem = i;
		listitem.iSubItem = 0;
		listitem.pszText = TitleMaps[i].title;
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

void CDxwndhostView::OnModify() 
{
	// TODO: Please add a command handler code here.
	int i;
	CTargetDlg dlg;
	POSITION pos;
	LV_ITEM listitem;
	CListCtrl& listctrl = GetListCtrl();

	if(!listctrl.GetSelectedCount()) return;
	pos = listctrl.GetFirstSelectedItemPosition();
	i = listctrl.GetNextSelectedItem(pos);
	dlg.m_DXVersion = TargetMaps[i].dxversion;
	if(dlg.m_DXVersion > 6) dlg.m_DXVersion -= 5;
	dlg.m_FilePath = TargetMaps[i].path;
	dlg.m_Module = TargetMaps[i].module;
	dlg.m_OpenGLLib = TargetMaps[i].OpenGLLib;
	dlg.m_Title = TitleMaps[i].title;
	dlg.m_UnNotify = TargetMaps[i].flags & UNNOTIFY ? 1 : 0;
	dlg.m_EmulateSurface = TargetMaps[i].flags & EMULATESURFACE ? 1 : 0;
	dlg.m_NoEmulateSurface = TargetMaps[i].flags & EMULATEFLAGS ? 0 : 1;
	dlg.m_EmulateBuffer = TargetMaps[i].flags & EMULATEBUFFER ? 1 : 0; 
	dlg.m_HookDI = TargetMaps[i].flags & HOOKDI ? 1 : 0;
	dlg.m_ModifyMouse = TargetMaps[i].flags & MODIFYMOUSE ? 1 : 0;
	dlg.m_OutTrace = TargetMaps[i].tflags & OUTDDRAWTRACE ? 1 : 0;
	dlg.m_OutDebug = TargetMaps[i].tflags & OUTDEBUG ? 1 : 0;
	dlg.m_CursorTrace = TargetMaps[i].tflags & OUTCURSORTRACE ? 1 : 0;
	dlg.m_LogEnabled = TargetMaps[i].tflags & OUTTRACE ? 1 : 0;
	dlg.m_OutWinMessages = TargetMaps[i].tflags & OUTWINMESSAGES ? 1 : 0;
	dlg.m_OutDXTrace = TargetMaps[i].tflags & OUTPROXYTRACE ? 1 : 0;
	dlg.m_DXProxed = TargetMaps[i].tflags & DXPROXED ? 1 : 0;
	dlg.m_AssertDialog = TargetMaps[i].tflags & ASSERTDIALOG ? 1 : 0;
	dlg.m_ImportTable = TargetMaps[i].tflags & OUTIMPORTTABLE ? 1 : 0;
	dlg.m_HandleDC = TargetMaps[i].flags & HANDLEDC ? 1 : 0;
	dlg.m_HandleExceptions = TargetMaps[i].flags & HANDLEEXCEPTIONS ? 1 : 0;
	dlg.m_SaveLoad = TargetMaps[i].flags & SAVELOAD ? 1 : 0;
	dlg.m_SlowDown = TargetMaps[i].flags & SLOWDOWN ? 1 : 0;
	dlg.m_BlitFromBackBuffer = TargetMaps[i].flags & BLITFROMBACKBUFFER ? 1 : 0;
	dlg.m_SuppressClipping = TargetMaps[i].flags & SUPPRESSCLIPPING ? 1 : 0;
	dlg.m_DisableGammaRamp = TargetMaps[i].flags2 & DISABLEGAMMARAMP ? 1 : 0;
	dlg.m_AutoRefresh = TargetMaps[i].flags & AUTOREFRESH ? 1 : 0;
	dlg.m_FixWinFrame = TargetMaps[i].flags & FIXWINFRAME ? 1 : 0;
	dlg.m_HideHwCursor = TargetMaps[i].flags & HIDEHWCURSOR ? 1 : 0;
	dlg.m_ShowHwCursor = TargetMaps[i].flags2 & SHOWHWCURSOR ? 1 : 0;
	dlg.m_EnableClipping = TargetMaps[i].flags & ENABLECLIPPING ? 1 : 0;
	dlg.m_CursorClipping = TargetMaps[i].flags & CLIPCURSOR ? 1 : 0;
	dlg.m_VideoToSystemMem = TargetMaps[i].flags & SWITCHVIDEOMEMORY ? 1 : 0;
	dlg.m_FixTextOut = TargetMaps[i].flags & FIXTEXTOUT ? 1 : 0;
	dlg.m_KeepCursorWithin = TargetMaps[i].flags & KEEPCURSORWITHIN ? 1 : 0;
	dlg.m_KeepCursorFixed = TargetMaps[i].flags2 & KEEPCURSORFIXED ? 1 : 0;
	dlg.m_UseRGB565 = TargetMaps[i].flags & USERGB565 ? 1 : 0;
	dlg.m_SuppressDXErrors = TargetMaps[i].flags & SUPPRESSDXERRORS ? 1 : 0;
	dlg.m_PreventMaximize = TargetMaps[i].flags & PREVENTMAXIMIZE ? 1 : 0;
	dlg.m_ClientRemapping = TargetMaps[i].flags & CLIENTREMAPPING ? 1 : 0;
	dlg.m_MapGDIToPrimary = TargetMaps[i].flags & MAPGDITOPRIMARY ? 1 : 0;
	dlg.m_LockWinPos = TargetMaps[i].flags & LOCKWINPOS ? 1 : 0;
	dlg.m_LockWinStyle = TargetMaps[i].flags & LOCKWINSTYLE ? 1 : 0;
	dlg.m_FixParentWin = TargetMaps[i].flags & FIXPARENTWIN ? 1 : 0;
	dlg.m_ModalStyle = TargetMaps[i].flags2 & MODALSTYLE ? 1 : 0;
	dlg.m_KeepAspectRatio = TargetMaps[i].flags2 & KEEPASPECTRATIO ? 1 : 0;
	dlg.m_ForceWinResize = TargetMaps[i].flags2 & FORCEWINRESIZE ? 1 : 0;
	dlg.m_HookGDI = TargetMaps[i].flags2 & HOOKGDI ? 1 : 0;
	dlg.m_HideMultiMonitor = TargetMaps[i].flags2 & HIDEMULTIMONITOR ? 1 : 0;
	dlg.m_WallpaperMode = TargetMaps[i].flags2 & WALLPAPERMODE ? 1 : 0;
	dlg.m_HookChildWin = TargetMaps[i].flags & HOOKCHILDWIN ? 1 : 0;
	dlg.m_MessageProc = TargetMaps[i].flags & MESSAGEPROC ? 1 : 0;
	dlg.m_FixNCHITTEST = TargetMaps[i].flags2 & FIXNCHITTEST ? 1 : 0;
	dlg.m_RecoverScreenMode = TargetMaps[i].flags2 & RECOVERSCREENMODE ? 1 : 0;
	dlg.m_RefreshOnResize = TargetMaps[i].flags2 & REFRESHONRESIZE ? 1 : 0;
	dlg.m_Init8BPP = TargetMaps[i].flags2 & INIT8BPP ? 1 : 0;
	dlg.m_Init16BPP = TargetMaps[i].flags2 & INIT16BPP ? 1 : 0;
	dlg.m_BackBufAttach = TargetMaps[i].flags2 & BACKBUFATTACH ? 1 : 0;
	dlg.m_HandleAltF4 = TargetMaps[i].flags & HANDLEALTF4 ? 1 : 0;
	dlg.m_LimitFPS = TargetMaps[i].flags2 & LIMITFPS ? 1 : 0;
	dlg.m_SkipFPS = TargetMaps[i].flags2 & SKIPFPS ? 1 : 0;
	dlg.m_ShowFPS = TargetMaps[i].flags2 & SHOWFPS ? 1 : 0;
	dlg.m_ShowFPSOverlay = TargetMaps[i].flags2 & SHOWFPSOVERLAY ? 1 : 0;
	dlg.m_TimeStretch = TargetMaps[i].flags2 & TIMESTRETCH ? 1 : 0;
	dlg.m_HookOpenGL = TargetMaps[i].flags2 & HOOKOPENGL ? 1 : 0;
	dlg.m_FakeVersion = TargetMaps[i].flags2 & FAKEVERSION ? 1 : 0;
	dlg.m_FullRectBlt = TargetMaps[i].flags2 & FULLRECTBLT ? 1 : 0;
	dlg.m_NoPaletteUpdate = TargetMaps[i].flags2 & NOPALETTEUPDATE ? 1 : 0;
	dlg.m_InitX = TargetMaps[i].initx;
	dlg.m_InitY = TargetMaps[i].inity;
	dlg.m_MinX = TargetMaps[i].minx;
	dlg.m_MinY = TargetMaps[i].miny;
	dlg.m_MaxX = TargetMaps[i].maxx;
	dlg.m_MaxY = TargetMaps[i].maxy;
	dlg.m_PosX = TargetMaps[i].posx;
	dlg.m_PosY = TargetMaps[i].posy;
	dlg.m_SizX = TargetMaps[i].sizx;
	dlg.m_SizY = TargetMaps[i].sizy;
	dlg.m_MaxFPS = TargetMaps[i].MaxFPS;
	dlg.m_InitTS = TargetMaps[i].InitTS+8;
	dlg.m_FakeVersionId = TargetMaps[i].FakeVersionId;
	if(dlg.DoModal() == IDOK && dlg.m_FilePath.GetLength()){
		strcpy_s(TargetMaps[i].path, sizeof(TargetMaps[i].path), dlg.m_FilePath);
		strcpy_s(TargetMaps[i].module, sizeof(TargetMaps[i].module), dlg.m_Module);
		strcpy_s(TargetMaps[i].OpenGLLib, sizeof(TargetMaps[i].OpenGLLib), dlg.m_OpenGLLib);
		strcpy_s(TitleMaps[i].title, sizeof(TitleMaps[i].title), dlg.m_Title);
		if(dlg.m_DXVersion > 1) dlg.m_DXVersion += 5;
		TargetMaps[i].dxversion = dlg.m_DXVersion;
		TargetMaps[i].flags = 0;
		TargetMaps[i].flags2 = 0;
		TargetMaps[i].tflags = 0;
		if(dlg.m_UnNotify) TargetMaps[i].flags |= UNNOTIFY;
		if(dlg.m_NoEmulateSurface) {
			dlg.m_EmulateSurface = FALSE;
			dlg.m_EmulateBuffer = FALSE;
			TargetMaps[i].flags &= ~EMULATEFLAGS;
		}
		if(dlg.m_EmulateSurface) {
			dlg.m_NoEmulateSurface = FALSE;
			dlg.m_EmulateBuffer = FALSE;
			TargetMaps[i].flags &= ~EMULATEFLAGS;
			TargetMaps[i].flags |= EMULATESURFACE;
		}
		if(dlg.m_EmulateBuffer) {
			dlg.m_NoEmulateSurface = FALSE;
			dlg.m_EmulateSurface = FALSE;
			TargetMaps[i].flags &= ~EMULATEFLAGS;
			TargetMaps[i].flags |= EMULATEBUFFER;
		}
		if(dlg.m_HookDI) TargetMaps[i].flags |= HOOKDI;
		if(dlg.m_ModifyMouse) TargetMaps[i].flags |= MODIFYMOUSE;
		if(dlg.m_OutTrace) TargetMaps[i].tflags |= OUTDDRAWTRACE;
		if(dlg.m_OutDebug) TargetMaps[i].tflags |= OUTDEBUG;
		if(dlg.m_CursorTrace) TargetMaps[i].tflags |= OUTCURSORTRACE;
		if(dlg.m_LogEnabled) TargetMaps[i].tflags |= OUTTRACE;
		if(dlg.m_OutWinMessages) TargetMaps[i].tflags |= OUTWINMESSAGES;
		if(dlg.m_OutDXTrace) TargetMaps[i].tflags |= OUTPROXYTRACE;
		if(dlg.m_DXProxed) TargetMaps[i].tflags |= DXPROXED;
		if(dlg.m_AssertDialog) TargetMaps[i].tflags |= ASSERTDIALOG;
		if(dlg.m_ImportTable) TargetMaps[i].tflags |= OUTIMPORTTABLE;
		if(dlg.m_HandleDC) TargetMaps[i].flags |= HANDLEDC;
		if(dlg.m_HandleExceptions) TargetMaps[i].flags |= HANDLEEXCEPTIONS;
		if(dlg.m_SaveLoad) TargetMaps[i].flags |= SAVELOAD;
		if(dlg.m_SlowDown) TargetMaps[i].flags |= SLOWDOWN;
		if(dlg.m_BlitFromBackBuffer) TargetMaps[i].flags |= BLITFROMBACKBUFFER;
		if(dlg.m_SuppressClipping) TargetMaps[i].flags |= SUPPRESSCLIPPING;
		if(dlg.m_DisableGammaRamp) TargetMaps[i].flags2 |= DISABLEGAMMARAMP;
		if(dlg.m_AutoRefresh) TargetMaps[i].flags |= AUTOREFRESH;
		if(dlg.m_FixWinFrame) TargetMaps[i].flags |= FIXWINFRAME;
		if(dlg.m_HideHwCursor) TargetMaps[i].flags |= HIDEHWCURSOR;
		if(dlg.m_ShowHwCursor) TargetMaps[i].flags2 |= SHOWHWCURSOR;
		if(dlg.m_EnableClipping) TargetMaps[i].flags |= ENABLECLIPPING;
		if(dlg.m_CursorClipping) TargetMaps[i].flags |= CLIPCURSOR;
		if(dlg.m_VideoToSystemMem) TargetMaps[i].flags |= SWITCHVIDEOMEMORY;
		if(dlg.m_FixTextOut) TargetMaps[i].flags |= FIXTEXTOUT;
		if(dlg.m_KeepCursorWithin) TargetMaps[i].flags |= KEEPCURSORWITHIN;
		if(dlg.m_KeepCursorFixed) TargetMaps[i].flags2 |= KEEPCURSORFIXED;
		if(dlg.m_UseRGB565) TargetMaps[i].flags |= USERGB565;
		if(dlg.m_SuppressDXErrors) TargetMaps[i].flags |= SUPPRESSDXERRORS;
		if(dlg.m_PreventMaximize) TargetMaps[i].flags |= PREVENTMAXIMIZE;
		if(dlg.m_ClientRemapping) TargetMaps[i].flags |= CLIENTREMAPPING;
		if(dlg.m_MapGDIToPrimary) TargetMaps[i].flags |= MAPGDITOPRIMARY;
		if(dlg.m_LockWinPos) TargetMaps[i].flags |= LOCKWINPOS;
		if(dlg.m_LockWinStyle) TargetMaps[i].flags |= LOCKWINSTYLE;
		if(dlg.m_FixParentWin) TargetMaps[i].flags |= FIXPARENTWIN;
		if(dlg.m_ModalStyle) TargetMaps[i].flags2 |= MODALSTYLE;
		if(dlg.m_KeepAspectRatio) TargetMaps[i].flags2 |= KEEPASPECTRATIO;
		if(dlg.m_ForceWinResize) TargetMaps[i].flags2 |= FORCEWINRESIZE;
		if(dlg.m_HookGDI) TargetMaps[i].flags2 |= HOOKGDI;
		if(dlg.m_HideMultiMonitor) TargetMaps[i].flags2 |= HIDEMULTIMONITOR;
		if(dlg.m_WallpaperMode) TargetMaps[i].flags2 |= WALLPAPERMODE;
		if(dlg.m_HookChildWin) TargetMaps[i].flags |= HOOKCHILDWIN;
		if(dlg.m_MessageProc) TargetMaps[i].flags |= MESSAGEPROC;
		if(dlg.m_FixNCHITTEST) TargetMaps[i].flags2 |= FIXNCHITTEST;
		if(dlg.m_RecoverScreenMode) TargetMaps[i].flags2 |= RECOVERSCREENMODE;
		if(dlg.m_RefreshOnResize) TargetMaps[i].flags2 |= REFRESHONRESIZE;
		if(dlg.m_Init8BPP) TargetMaps[i].flags2 |= INIT8BPP;
		if(dlg.m_Init16BPP) TargetMaps[i].flags2 |= INIT16BPP;
		if(dlg.m_BackBufAttach) TargetMaps[i].flags2 |= BACKBUFATTACH;
		if(dlg.m_HandleAltF4) TargetMaps[i].flags |= HANDLEALTF4;
		if(dlg.m_LimitFPS) TargetMaps[i].flags2 |= LIMITFPS;
		if(dlg.m_SkipFPS) TargetMaps[i].flags2 |= SKIPFPS;
		if(dlg.m_ShowFPS) TargetMaps[i].flags2 |= SHOWFPS;
		if(dlg.m_ShowFPSOverlay) TargetMaps[i].flags2 |= SHOWFPSOVERLAY;
		if(dlg.m_TimeStretch) TargetMaps[i].flags2 |= TIMESTRETCH;
		if(dlg.m_HookOpenGL) TargetMaps[i].flags2 |= HOOKOPENGL;
		if(dlg.m_FakeVersion) TargetMaps[i].flags2 |= FAKEVERSION;
		if(dlg.m_FullRectBlt) TargetMaps[i].flags2 |= FULLRECTBLT;
		if(dlg.m_NoPaletteUpdate) TargetMaps[i].flags2 |= NOPALETTEUPDATE;
		TargetMaps[i].initx = dlg.m_InitX;
		TargetMaps[i].inity = dlg.m_InitY;
		TargetMaps[i].minx = dlg.m_MinX;
		TargetMaps[i].miny = dlg.m_MinY;
		TargetMaps[i].maxx = dlg.m_MaxX;
		TargetMaps[i].maxy = dlg.m_MaxY;
		TargetMaps[i].posx = dlg.m_PosX;
		TargetMaps[i].posy = dlg.m_PosY;
		TargetMaps[i].sizx = dlg.m_SizX;
		TargetMaps[i].sizy = dlg.m_SizY;
		TargetMaps[i].MaxFPS = dlg.m_MaxFPS;
		TargetMaps[i].InitTS = dlg.m_InitTS-8;
		TargetMaps[i].FakeVersionId = dlg.m_FakeVersionId;
		strcpy_s(TargetMaps[i].module, sizeof(TargetMaps[i].module), dlg.m_Module);
		strcpy_s(TargetMaps[i].OpenGLLib, sizeof(TargetMaps[i].OpenGLLib), dlg.m_OpenGLLib);
		strcpy_s(TitleMaps[i].title, sizeof(TitleMaps[i].title), dlg.m_Title);
		CListCtrl& listctrl = GetListCtrl();
		listitem.mask = LVIF_TEXT;
		listitem.iItem = i;
		listitem.iSubItem = 0;
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
		listitem.mask = LVIF_TEXT;
		listitem.iItem = i;
		listitem.iSubItem = 0;
		listitem.pszText = TitleMaps[i].title;
		listctrl.SetItem(&listitem);
		listctrl.InsertItem(&listitem);
	}

	SetTarget(TargetMaps);
	this->isUpdated=TRUE;
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

void CDxwndhostView::OnAdd() 
{
	// TODO: Please add a command handler code here.
	int i;
	CTargetDlg dlg;
	LV_ITEM listitem;

	dlg.m_DXVersion = 0;
	dlg.m_MaxX = 0; //639;
	dlg.m_MaxY = 0; //479;
	for(i = 0; i < MAXTARGETS; i ++) if(!TargetMaps[i].path[0]) break;
	if(i>=MAXTARGETS){
		MessageBoxEx(0, "Maximum entries number reached.\nDelete some entry to add a new one.", "Warning", MB_OK | MB_ICONEXCLAMATION, NULL);
		return;
	}
	if(dlg.DoModal() == IDOK && dlg.m_FilePath.GetLength()){
		strcpy_s(TargetMaps[i].path,sizeof(TargetMaps[i].path),dlg.m_FilePath);
		strcpy_s(TargetMaps[i].module,sizeof(TargetMaps[i].module),dlg.m_Module);
		strcpy_s(TargetMaps[i].OpenGLLib,sizeof(TargetMaps[i].OpenGLLib),dlg.m_OpenGLLib);
		strcpy_s(TitleMaps[i].title, sizeof(TitleMaps[i].title), dlg.m_Title);
		if(dlg.m_DXVersion > 1) dlg.m_DXVersion += 5;
		TargetMaps[i].dxversion = dlg.m_DXVersion;
		TargetMaps[i].flags = 0;
		TargetMaps[i].flags2 = 0;
		TargetMaps[i].tflags = 0;
		if(dlg.m_UnNotify) TargetMaps[i].flags |= UNNOTIFY;
		if(dlg.m_NoEmulateSurface) {
			dlg.m_EmulateSurface = FALSE;
			dlg.m_EmulateBuffer = FALSE;
			TargetMaps[i].flags &= ~EMULATEFLAGS;
		}
		if(dlg.m_EmulateSurface) {
			dlg.m_NoEmulateSurface = FALSE;
			dlg.m_EmulateBuffer = FALSE;
			TargetMaps[i].flags &= ~EMULATEFLAGS;
			TargetMaps[i].flags |= EMULATESURFACE;
		}
		if(dlg.m_EmulateBuffer) {
			dlg.m_NoEmulateSurface = FALSE;
			dlg.m_EmulateSurface = FALSE;
			TargetMaps[i].flags &= ~EMULATEFLAGS;
			TargetMaps[i].flags |= EMULATEBUFFER;
		}
		if(dlg.m_HookDI) TargetMaps[i].flags |= HOOKDI;
		if(dlg.m_ModifyMouse) TargetMaps[i].flags |= MODIFYMOUSE;
		if(dlg.m_OutTrace) TargetMaps[i].tflags |= OUTDDRAWTRACE;
		if(dlg.m_OutDebug) TargetMaps[i].tflags |= OUTDEBUG;
		if(dlg.m_CursorTrace) TargetMaps[i].tflags |= OUTCURSORTRACE;
		if(dlg.m_LogEnabled) TargetMaps[i].tflags |= OUTTRACE;
		if(dlg.m_OutWinMessages) TargetMaps[i].tflags |= OUTWINMESSAGES;
		if(dlg.m_OutDXTrace) TargetMaps[i].tflags |= OUTPROXYTRACE;
		if(dlg.m_DXProxed) TargetMaps[i].tflags |= DXPROXED;
		if(dlg.m_AssertDialog) TargetMaps[i].tflags |= ASSERTDIALOG;
		if(dlg.m_ImportTable) TargetMaps[i].tflags |= OUTIMPORTTABLE;
		if(dlg.m_HandleDC) TargetMaps[i].flags |= HANDLEDC;
		if(dlg.m_HandleExceptions) TargetMaps[i].flags |= HANDLEEXCEPTIONS;
		if(dlg.m_SaveLoad) TargetMaps[i].flags |= SAVELOAD;
		if(dlg.m_SlowDown) TargetMaps[i].flags |= SLOWDOWN;
		if(dlg.m_BlitFromBackBuffer) TargetMaps[i].flags |= BLITFROMBACKBUFFER;
		if(dlg.m_SuppressClipping) TargetMaps[i].flags |= SUPPRESSCLIPPING;
		if(dlg.m_DisableGammaRamp) TargetMaps[i].flags2 |= DISABLEGAMMARAMP;
		if(dlg.m_AutoRefresh) TargetMaps[i].flags |= AUTOREFRESH;
		if(dlg.m_FixWinFrame) TargetMaps[i].flags |= FIXWINFRAME;
		if(dlg.m_HideHwCursor) TargetMaps[i].flags |= HIDEHWCURSOR;
		if(dlg.m_ShowHwCursor) TargetMaps[i].flags2 |= SHOWHWCURSOR;
		if(dlg.m_EnableClipping) TargetMaps[i].flags |= ENABLECLIPPING;
		if(dlg.m_CursorClipping) TargetMaps[i].flags |= CLIPCURSOR;
		if(dlg.m_VideoToSystemMem) TargetMaps[i].flags |= SWITCHVIDEOMEMORY;
		if(dlg.m_FixTextOut) TargetMaps[i].flags |= FIXTEXTOUT;
		if(dlg.m_KeepCursorWithin) TargetMaps[i].flags |= KEEPCURSORWITHIN;
		if(dlg.m_KeepCursorFixed) TargetMaps[i].flags2 |= KEEPCURSORFIXED;
		if(dlg.m_UseRGB565) TargetMaps[i].flags |= USERGB565;
		if(dlg.m_SuppressDXErrors) TargetMaps[i].flags |= SUPPRESSDXERRORS;
		if(dlg.m_PreventMaximize) TargetMaps[i].flags |= PREVENTMAXIMIZE;
		if(dlg.m_ClientRemapping) TargetMaps[i].flags |= CLIENTREMAPPING;
		if(dlg.m_MapGDIToPrimary) TargetMaps[i].flags |= MAPGDITOPRIMARY;
		if(dlg.m_LockWinPos) TargetMaps[i].flags |= LOCKWINPOS;
		if(dlg.m_LockWinStyle) TargetMaps[i].flags |= LOCKWINSTYLE;
		if(dlg.m_FixParentWin) TargetMaps[i].flags |= FIXPARENTWIN;
		if(dlg.m_ModalStyle) TargetMaps[i].flags2 |= MODALSTYLE;
		if(dlg.m_KeepAspectRatio) TargetMaps[i].flags2 |= KEEPASPECTRATIO;
		if(dlg.m_ForceWinResize) TargetMaps[i].flags2 |= FORCEWINRESIZE;
		if(dlg.m_HookGDI) TargetMaps[i].flags2 |= HOOKGDI;
		if(dlg.m_HideMultiMonitor) TargetMaps[i].flags2 |= HIDEMULTIMONITOR;
		if(dlg.m_WallpaperMode) TargetMaps[i].flags2 |= WALLPAPERMODE;
		if(dlg.m_HookChildWin) TargetMaps[i].flags |= HOOKCHILDWIN;
		if(dlg.m_MessageProc) TargetMaps[i].flags |= MESSAGEPROC;
		if(dlg.m_FixNCHITTEST) TargetMaps[i].flags2 |= FIXNCHITTEST;
		if(dlg.m_RecoverScreenMode) TargetMaps[i].flags2 |= RECOVERSCREENMODE;
		if(dlg.m_RefreshOnResize) TargetMaps[i].flags2 |= REFRESHONRESIZE;
		if(dlg.m_Init8BPP) TargetMaps[i].flags2 |= INIT8BPP;
		if(dlg.m_Init16BPP) TargetMaps[i].flags2 |= INIT16BPP;
		if(dlg.m_BackBufAttach) TargetMaps[i].flags2 |= BACKBUFATTACH;
		if(dlg.m_HandleAltF4) TargetMaps[i].flags |= HANDLEALTF4;
		if(dlg.m_LimitFPS) TargetMaps[i].flags2 |= LIMITFPS;
		if(dlg.m_SkipFPS) TargetMaps[i].flags2 |= SKIPFPS;
		if(dlg.m_ShowFPS) TargetMaps[i].flags2 |= SHOWFPS;
		if(dlg.m_ShowFPSOverlay) TargetMaps[i].flags2 |= SHOWFPSOVERLAY;
		if(dlg.m_TimeStretch) TargetMaps[i].flags2 |= TIMESTRETCH;
		if(dlg.m_HookOpenGL) TargetMaps[i].flags2 |= HOOKOPENGL;
		if(dlg.m_FakeVersion) TargetMaps[i].flags2 |= FAKEVERSION;
		if(dlg.m_FullRectBlt) TargetMaps[i].flags2 |= FULLRECTBLT;
		if(dlg.m_NoPaletteUpdate) TargetMaps[i].flags2 |= NOPALETTEUPDATE;
		TargetMaps[i].initx = dlg.m_InitX;
		TargetMaps[i].inity = dlg.m_InitY;
		TargetMaps[i].minx = dlg.m_MinX;
		TargetMaps[i].miny = dlg.m_MinY;
		TargetMaps[i].maxx = dlg.m_MaxX;
		TargetMaps[i].maxy = dlg.m_MaxY;
		TargetMaps[i].posx = dlg.m_PosX;
		TargetMaps[i].posy = dlg.m_PosY;
		TargetMaps[i].sizx = dlg.m_SizX;
		TargetMaps[i].sizy = dlg.m_SizY;
		TargetMaps[i].MaxFPS = dlg.m_MaxFPS;
		TargetMaps[i].FakeVersionId = dlg.m_FakeVersionId;
		if (dlg.m_InitTS>=-8 && dlg.m_InitTS<=8)
			TargetMaps[i].InitTS = dlg.m_InitTS-8;
		else
			MessageBoxEx(0, "Bad InitTS", "Warning", MB_OK, NULL);
		CListCtrl& listctrl = GetListCtrl();
		listitem.mask = LVIF_TEXT;
		listitem.iItem = i;
		listitem.iSubItem = 0;
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
	}
	CListView::OnRButtonDown(nFlags, point);
}

void CDxwndhostView::OnRun() 
{
	CListCtrl& listctrl = GetListCtrl();
	POSITION pos;
	int i;
	STARTUPINFO sinfo;
	PROCESS_INFORMATION pinfo;
	char path[MAX_PATH];

	if(!listctrl.GetSelectedCount()) return;
	pos = listctrl.GetFirstSelectedItemPosition();
	i = listctrl.GetNextSelectedItem(pos);
	ZeroMemory(&sinfo, sizeof(sinfo));
	sinfo.cb = sizeof(sinfo);
	strcpy_s(path, sizeof(path), TargetMaps[i].path);
	PathRemoveFileSpec(path);
	CreateProcess(0, TargetMaps[i].path, 0, 0, 0, CREATE_DEFAULT_ERROR_MODE, 0, path, &sinfo, &pinfo);
}
