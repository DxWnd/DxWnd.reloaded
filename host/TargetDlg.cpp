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
	m_UnNotify = FALSE;
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



//void  TSCombo::OnEnable(BOOL bEnable)
//{
//	CComboBox::OnEnable(bEnable); 
//
//	// Get edit control which happens to be the first child window
//	CEdit* pEdit = (CEdit*)GetWindow(GW_CHILD);
//	
//	// Always have the edit box enabled
//	pEdit->EnableWindow(TRUE);
//	
//	// Set read only is combo box is disabled
//	pEdit->SetReadOnly(!bEnable);
//} 

static struct {char bMajor; char bMinor; char *sName;} WinVersions[6]=
{
	{5, 0, "Windows 2000"},
	{5, 1, "Windows XP"},
	{5, 2, "Windows Server 2003"},
	{6, 0, "Windows Vista"},
	{6, 1, "Windows 7"},
	{6, 2, "Windows 8"}
};

BOOL CTargetDlg::OnInitDialog()
{

	CListBox *List;
	CDialog::OnInitDialog();
	int i;
	extern char *GetTSCaption(int);
	List=(CListBox *)this->GetDlgItem(IDC_LISTTS);
	List->ResetContent();
	for(i=-8; i<=8; i++) List->AddString(GetTSCaption(i));
	List->SetCurSel(m_InitTS);
	List=(CListBox *)this->GetDlgItem(IDC_LISTFAKE);
	List->ResetContent();
	for(i=0; i<6; i++) List->AddString(WinVersions[i].sName);
	List->SetCurSel(m_FakeVersion);
	return TRUE;
}

void CTargetDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTargetDlg)
	DDX_Control(pDX, IDC_FILE, m_File);
	DDX_Radio(pDX, IDC_AUTO, m_DXVersion);
	DDX_Check(pDX, IDC_EMULATESURFACE, m_EmulateSurface);
	DDX_Check(pDX, IDC_NOEMULATESURFACE, m_NoEmulateSurface);
	DDX_Check(pDX, IDC_EMULATEBUFFER, m_EmulateBuffer);
	DDX_Check(pDX, IDC_HOOKDI, m_HookDI);
	DDX_Check(pDX, IDC_MODIFYMOUSE, m_ModifyMouse);
	DDX_Check(pDX, IDC_OUTTRACE, m_OutTrace);
	DDX_Check(pDX, IDC_OUTDEBUG, m_OutDebug);
	DDX_Check(pDX, IDC_CURSORTRACE, m_CursorTrace);
	DDX_Check(pDX, IDC_LOGENABLED, m_LogEnabled);
	DDX_Check(pDX, IDC_OUTWINMESSAGES, m_OutWinMessages);
	DDX_Check(pDX, IDC_OUTDXTRACE, m_OutDXTrace);
	DDX_Check(pDX, IDC_DXPROXED, m_DXProxed);
	DDX_Check(pDX, IDC_ASSERT, m_AssertDialog);
	DDX_Check(pDX, IDC_IMPORTTABLE, m_ImportTable);
	DDX_Check(pDX, IDC_HANDLEDC, m_HandleDC);
	DDX_Check(pDX, IDC_HANDLEEXCEPTIONS, m_HandleExceptions);
	DDX_Check(pDX, IDC_UNNOTIFY, m_UnNotify);
	DDX_Text(pDX, IDC_FILE, m_FilePath);
	DDX_Text(pDX, IDC_MODULE, m_Module);
	DDX_Text(pDX, IDC_TITLE, m_Title);
	DDX_Check(pDX, IDC_SAVELOAD, m_SaveLoad);
	DDX_Check(pDX, IDC_SLOW, m_SlowDown);
	DDX_Check(pDX, IDC_BLITFROMBACKBUFFER, m_BlitFromBackBuffer);
	DDX_Check(pDX, IDC_SUPPRESSCLIPPING, m_SuppressClipping);
	DDX_Check(pDX, IDC_DISABLEGAMMARAMP, m_DisableGammaRamp);
	DDX_Check(pDX, IDC_AUTOREFRESH, m_AutoRefresh);
	DDX_Check(pDX, IDC_FIXWINFRAME, m_FixWinFrame);
	DDX_Check(pDX, IDC_HIDEHWCURSOR, m_HideHwCursor);
	DDX_Check(pDX, IDC_SHOWHWCURSOR, m_ShowHwCursor);
	DDX_Check(pDX, IDC_ENABLECLIPPING, m_EnableClipping);
	DDX_Check(pDX, IDC_CLIPCURSOR, m_CursorClipping);
	DDX_Check(pDX, IDC_VIDEOTOSYSTEMMEM, m_VideoToSystemMem);
	DDX_Check(pDX, IDC_FIXTEXTOUT, m_FixTextOut);
	DDX_Check(pDX, IDC_KEEPCURSORWITHIN, m_KeepCursorWithin);
	DDX_Check(pDX, IDC_KEEPCURSORFIXED, m_KeepCursorFixed);
	DDX_Check(pDX, IDC_USERGB565, m_UseRGB565);
	DDX_Check(pDX, IDC_SUPPRESSDXERRORS, m_SuppressDXErrors);
	DDX_Check(pDX, IDC_PREVENTMAXIMIZE, m_PreventMaximize);
	DDX_Check(pDX, IDC_CLIENTREMAPPING, m_ClientRemapping);
	DDX_Check(pDX, IDC_MAPGDITOPRIMARY, m_MapGDIToPrimary);
	DDX_Check(pDX, IDC_LOCKWINPOS, m_LockWinPos);
	DDX_Check(pDX, IDC_LOCKWINSTYLE, m_LockWinStyle);
	DDX_Check(pDX, IDC_FIXPARENTWIN, m_FixParentWin);
	DDX_Check(pDX, IDC_MODALSTYLE, m_ModalStyle);
	DDX_Check(pDX, IDC_KEEPASPECTRATIO, m_KeepAspectRatio);
	DDX_Check(pDX, IDC_FORCEWINRESIZE, m_ForceWinResize);
	DDX_Check(pDX, IDC_HOOKGDI, m_HookGDI);
	DDX_Check(pDX, IDC_HIDEMULTIMONITOR, m_HideMultiMonitor);
	DDX_Check(pDX, IDC_WALLPAPERMODE, m_WallpaperMode);
	DDX_Check(pDX, IDC_HOOKCHILDWIN, m_HookChildWin);
	DDX_Check(pDX, IDC_MESSAGEPROC, m_MessageProc);
	DDX_Check(pDX, IDC_FIXNCHITTEST, m_FixNCHITTEST);
	DDX_Check(pDX, IDC_RECOVERSCREENMODE, m_RecoverScreenMode);
	DDX_Check(pDX, IDC_REFRESHONRESIZE, m_RefreshOnResize);
	DDX_Check(pDX, IDC_INIT8BPP, m_Init8BPP);
	DDX_Check(pDX, IDC_INIT16BPP, m_Init16BPP);
	DDX_Check(pDX, IDC_BACKBUFATTACH, m_BackBufAttach);
	DDX_Check(pDX, IDC_HANDLEALTF4, m_HandleAltF4);
	DDX_Check(pDX, IDC_LIMITFPS, m_LimitFPS);
	DDX_Check(pDX, IDC_SKIPFPS, m_SkipFPS);
	DDX_Check(pDX, IDC_SHOWFPS, m_ShowFPS);
	DDX_Check(pDX, IDC_SHOWFPSOVERLAY, m_ShowFPSOverlay);
	DDX_Check(pDX, IDC_TIMESTRETCH, m_TimeStretch);
	DDX_Check(pDX, IDC_HOOKOPENGL, m_HookOpenGL);
	DDX_Check(pDX, IDC_FAKEVERSION, m_FakeVersion);
	DDX_Check(pDX, IDC_FULLRECTBLT, m_FullRectBlt);
	DDX_Check(pDX, IDC_NOPALETTEUPDATE, m_NoPaletteUpdate);
	DDX_Text(pDX, IDC_INITX, m_InitX);
	DDX_Text(pDX, IDC_INITY, m_InitY);
	DDX_Text(pDX, IDC_MAXX, m_MaxX);
	DDX_Text(pDX, IDC_MAXY, m_MaxY);
	DDX_Text(pDX, IDC_MINX, m_MinX);
	DDX_Text(pDX, IDC_MINY, m_MinY);
	DDX_Text(pDX, IDC_POSX, m_PosX);
	DDX_Text(pDX, IDC_POSY, m_PosY);
	DDX_Text(pDX, IDC_SIZX, m_SizX);
	DDX_Text(pDX, IDC_SIZY, m_SizY);
	DDX_Text(pDX, IDC_MAXFPS, m_MaxFPS);
	//DDX_Text(pDX, IDC_INITTS, m_InitTS);
	DDX_LBIndex(pDX, IDC_LISTTS, m_InitTS);
	DDX_LBIndex(pDX, IDC_LISTFAKE, m_FakeVersionId);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CTargetDlg, CDialog)
	//{{AFX_MSG_MAP(CTargetDlg)
	ON_BN_CLICKED(IDC_OPEN, OnOpen)
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_UNNOTIFY, &CTargetDlg::OnBnClickedUnnotify)
	ON_BN_CLICKED(IDC_EMULATEBUFFER, &CTargetDlg::OnBnClickedEmulateBuffer)
	ON_BN_CLICKED(IDC_EMULATESURFACE, &CTargetDlg::OnBnClickedEmulateSurface)
	ON_BN_CLICKED(IDC_NOEMULATESURFACE, &CTargetDlg::OnBnClickedNoEmulateSurface)
	ON_BN_CLICKED(IDC_OUTTRACE, &CTargetDlg::OnBnClickedOuttrace)
	ON_BN_CLICKED(IDC_LOGENABLED, &CTargetDlg::OnBnClickedLogEnabled)
	ON_BN_CLICKED(IDC_HANDLEDC, &CTargetDlg::OnBnClickedHandleDC)
	ON_BN_CLICKED(IDC_HANDLEEXCEPTIONS, &CTargetDlg::OnBnClickedHandleExceptions)
	//ON_BN_CLICKED(IDC_TRANSPARENTWIN, &CTargetDlg::OnBnClickedBlitFromBackBuffer)
	//ON_BN_CLICKED(IDC_TRANSPARENTALL, &CTargetDlg::OnBnClickedTransparentAll)
	//ON_BN_CLICKED(IDC_ABSOLUTECOORD, &CTargetDlg::OnBnClickedAbsolutecoord)
	ON_BN_CLICKED(IDC_AUTOREFRESH, &CTargetDlg::OnBnClickedAutorefresh)
	ON_BN_CLICKED(IDC_FIXWINFRAME, &CTargetDlg::OnBnClickedFixWinFrame)
	//ON_BN_CLICKED(IDC_HIDEHWCURSOR, &CTargetDlg::OnBnClickedHideHwCursor)
	ON_BN_CLICKED(IDC_ENABLECLIPPING, &CTargetDlg::OnBnClickedEnableClipping)
	ON_BN_CLICKED(IDC_VIDEOTOSYSTEMMEM, &CTargetDlg::OnBnClickedHandleCursorPos)
	ON_BN_CLICKED(IDC_FIXTEXTOUT, &CTargetDlg::OnBnClickedFixTextOut)
	ON_BN_CLICKED(IDC_KEEPCURSORWITHIN, &CTargetDlg::OnBnClickedKeepCursorWithin)
	ON_BN_CLICKED(IDC_KEEPCURSORFIXED, &CTargetDlg::OnBnClickedKeepCursorFixed)
	ON_BN_CLICKED(IDC_USERGB565, &CTargetDlg::OnBnClickedUseRGB565)
	ON_BN_CLICKED(IDC_SUPPRESSDXERRORS, &CTargetDlg::OnBnClickedCursorScaling)
	ON_BN_CLICKED(IDC_PREVENTMAXIMIZE, &CTargetDlg::OnBnClickedPreventMaximize)
	ON_BN_CLICKED(IDC_CLIENTREMAPPING, &CTargetDlg::OnBnClickedClientRemapping)
	ON_BN_CLICKED(IDC_MAPGDITOPRIMARY, &CTargetDlg::OnBnClickedMapGDIToPrimary)
	ON_BN_CLICKED(IDC_LOCKWINPOS, &CTargetDlg::OnBnClickedLockWinPos)
	ON_BN_CLICKED(IDC_LOCKWINSTYLE, &CTargetDlg::OnBnClickedLockWinStyle)
	ON_BN_CLICKED(IDC_HOOKCHILDWIN, &CTargetDlg::OnBnClickedHookChildWin)
	ON_BN_CLICKED(IDC_MESSAGEPROC, &CTargetDlg::OnBnClickedMessageProc)
	ON_BN_CLICKED(IDC_RECOVERSCREENMODE, &CTargetDlg::OnBnClickedRecoverScreenMode)
	ON_BN_CLICKED(IDC_REFRESHONRESIZE, &CTargetDlg::OnBnClickedRefreshOnResize)
	ON_BN_CLICKED(IDC_INIT8BPP, &CTargetDlg::OnBnClickedInit8BPP)
	ON_BN_CLICKED(IDC_INIT16BPP, &CTargetDlg::OnBnClickedInit16BPP)
	ON_BN_CLICKED(IDC_BACKBUFATTACH, &CTargetDlg::OnBnClickedBackBufAttach)
	ON_BN_CLICKED(IDC_HANDLEALTF4, &CTargetDlg::OnBnClickedHandleAltF4)
	ON_EN_CHANGE(IDC_MAXX, &CTargetDlg::OnEnChangeMaxx)
	ON_EN_CHANGE(IDC_INITX, &CTargetDlg::OnEnChangeInitx)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTargetDlg Message Handler

void CTargetDlg::OnOpen() 
{
	// TODO: Please add your control notification handler code here
    char path[MAX_PATH];
	m_File.GetWindowText(path, MAX_PATH);
	CFileDialog dlg( TRUE, "*.*", path, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
        "Program (*.exe)|*.exe|All Files (*.*)|*.*||",  this);
    if( dlg.DoModal() == IDOK) m_File.SetWindowText(dlg.GetPathName());
}

void CTargetDlg::OnBnClickedUnnotify()
{
	// TODO: Add your control notification handler code here
}

void CTargetDlg::OnBnClickedEmulateSurface()
{
	// TODO: Add your control notification handler code here
}

void CTargetDlg::OnBnClickedNoEmulateSurface()
{
	// TODO: Add your control notification handler code here
}

void CTargetDlg::OnBnClickedEmulateBuffer()
{
	// TODO: Add your control notification handler code here
}
void CTargetDlg::OnBnClickedOuttrace()
{
	// TODO: Add your control notification handler code here
}
void CTargetDlg::OnBnClickedLogEnabled()
{
	// TODO: Add your control notification handler code here
}
void CTargetDlg::OnBnClickedHandleDC()
{
	// TODO: Add your control notification handler code here
}

void CTargetDlg::OnBnClickedHandleExceptions()
{
	// TODO: Add your control notification handler code here
}
//void CTargetDlg::OnBnClickedBlitFromBackBuffer()
//{
//	// TODO: Add your control notification handler code here
//}
//
//void CTargetDlg::OnBnClickedTransparentAll()
//{
//	// TODO: Add your control notification handler code here
//}

void CTargetDlg::OnEnChangeFile2()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO:  Add your control notification handler code here
}

//void CTargetDlg::OnBnClickedAbsolutecoord()
//{
//	// TODO: Add your control notification handler code here
//}

void CTargetDlg::OnBnClickedAutorefresh()
{
	// TODO: Add your control notification handler code here
}

void CTargetDlg::OnBnClickedFixWinFrame()
{
	// TODO: Add your control notification handler code here
}

//void CTargetDlg::OnBnClickedHideHwCursor()
//{
//	// TODO: Add your control notification handler code here
//}

void CTargetDlg::OnBnClickedEnableClipping()
{
	// TODO: Add your control notification handler code here
}

//void CTargetDlg::OnBnClickedMouseToScreen()
//{
//	// TODO: Add your control notification handler code here
//}

void CTargetDlg::OnBnClickedHandleCursorPos()
{
	// TODO: Add your control notification handler code here
}

void CTargetDlg::OnBnClickedFixTextOut()
{
	// TODO: Add your control notification handler code here
}

void CTargetDlg::OnBnClickedKeepCursorWithin()
{
	// TODO: Add your control notification handler code here
}

void CTargetDlg::OnBnClickedKeepCursorFixed()
{
	// TODO: Add your control notification handler code here
}

//void CTargetDlg::OnBnClickedNullRectBlit()
//{
//	// TODO: Add your control notification handler code here
//}

void CTargetDlg::OnBnClickedUseRGB565()
{
	// TODO: Add your control notification handler code here
}

void CTargetDlg::OnBnClickedCursorScaling()
{
	// TODO: Add your control notification handler code here
}

void CTargetDlg::OnBnClickedPreventMaximize()
{
	// TODO: Add your control notification handler code here
}

//void CTargetDlg::OnBnClickedResetPrimary()
//{
//	// TODO: Add your control notification handler code here
//}

void CTargetDlg::OnBnClickedClientRemapping()
{
	// TODO: Add your control notification handler code here
}

void CTargetDlg::OnBnClickedMapGDIToPrimary()
{
	// TODO: Add your control notification handler code here
}

void CTargetDlg::OnBnClickedLockWinPos()
{
	// TODO: Add your control notification handler code here
}

void CTargetDlg::OnBnClickedLockWinStyle()
{
	// TODO: Add your control notification handler code here
}

void CTargetDlg::OnBnClickedHookChildWin()
{
	// TODO: Add your control notification handler code here
}

void CTargetDlg::OnBnClickedMessageProc()
{
	// TODO: Add your control notification handler code here
}

void CTargetDlg::OnBnClickedRecoverScreenMode()
{
	// TODO: Add your control notification handler code here
}

void CTargetDlg::OnBnClickedRefreshOnResize()
{
	// TODO: Add your control notification handler code here
}

void CTargetDlg::OnBnClickedInit8BPP()
{
	// TODO: Add your control notification handler code here
}

void CTargetDlg::OnBnClickedInit16BPP()
{
	// TODO: Add your control notification handler code here
}

void CTargetDlg::OnBnClickedBackBufAttach()
{
	// TODO: Add your control notification handler code here
}

void CTargetDlg::OnBnClickedHandleAltF4()
{
	// TODO: Add your control notification handler code here
}

//void CTargetDlg::OnBnClickedEmulateModeX()
//{
//	// TODO: Add your control notification handler code here
//}

void CTargetDlg::OnEnChangeMaxx()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO:  Add your control notification handler code here
}

void CTargetDlg::OnEnChangeInitx()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO:  Add your control notification handler code here
}
