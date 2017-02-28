#if !defined(AFX_TARGETDLG_H__683E529D_7282_484C_A403_367A6A90CC57__INCLUDED_)
#define AFX_TARGETDLG_H__683E529D_7282_484C_A403_367A6A90CC57__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// TargetDlg.h : Header file
//
#include "dxTabCtrl.h"
#include "SpecialEdit.h"
#include "CDragEdit.h"

/////////////////////////////////////////////////////////////////////////////
// CTargetDlg Dialog

class CTargetDlg : public CDialog
{
// Constructor
public:
	CTargetDlg(CWnd* pParent = NULL);   // Standard constructor
	void OnOK();

// Dialog Data
	//{{AFX_DATA(CTargetDlg)
	enum { IDD = IDD_TARGET };
	CDXTabCtrl	m_tabdxTabCtrl;
	CDragEdit	m_File;
	CDragEdit	m_Launch;
	int		m_DXVersion;
	int		m_Coordinates;
	int		m_InitColorDepth;
	int		m_DxEmulationMode;
	int		m_DxFilterMode;
	int		m_DCEmulationMode;
	int		m_MouseVisibility;
	int		m_MouseClipper;
	int		m_OffendingMessages;
	int		m_TextureHandling;
	int		m_SonProcessMode;
	int		m_MaxDdrawInterface;
	int		m_SlowRatio;
	int		m_LogMode;
	int		m_MonitorId;
	int		m_WinMovementType;
	BOOL	m_HookDI;
	BOOL	m_HookDI8;
	BOOL	m_EmulateRelMouse;
	BOOL	m_SkipDevTypeHID;
	BOOL	m_SuppressDIErrors;
	BOOL	m_SharedKeyboard;
	BOOL	m_ModifyMouse;
	BOOL	m_VirtualJoystick;
	BOOL	m_Unacquire;
	BOOL	m_OutDebug;
	BOOL	m_CursorTrace;
//	BOOL	m_LogEnabled;
//	BOOL	m_EraseLogFile;
	BOOL	m_AddTimeStamp;
	BOOL	m_AddRelativeTime;
	BOOL	m_OutDebugString;
	BOOL	m_OutWinMessages;
	BOOL	m_OutDWTrace;
	BOOL	m_OutOGLTrace;
	BOOL	m_OutWGTrace;
	BOOL	m_OutD3DTrace;
	BOOL	m_OutDDRAWTrace;
	BOOL	m_AssertDialog;
	BOOL	m_ImportTable;
	BOOL	m_RegistryOp;
	BOOL	m_TraceHooks;
	BOOL	m_UnNotify;
	BOOL	m_Windowize;
	BOOL	m_HotPatch;
	BOOL	m_HookDLLs;
	BOOL	m_AnsiWide;
	BOOL	m_HookNoRun;
	BOOL	m_CopyNoShims;
	BOOL	m_HookNoUpdate;
	BOOL	m_SequencedIAT;
	BOOL	m_TerminateOnClose;
	BOOL	m_ConfirmOnClose;
	BOOL	m_EmulateRegistry;
	BOOL	m_OverrideRegistry;
	BOOL	m_Wow64Registry;
	BOOL	m_Wow32Registry;
	BOOL	m_FullScreenOnly;
	BOOL	m_ShowHints;
	BOOL	m_BackgroundPriority;
	BOOL	m_NeedAdminCaps;
	BOOL	m_PeekAllMessages;
	BOOL	m_NoWinPosChanges;
	BOOL	m_MessagePump;
	BOOL	m_ClipMenu;
	BOOL	m_NoBanner;
	int		m_InjectionMode;
	BOOL	m_HookEnabled;
	BOOL	m_ReplacePrivOps;
	CString	m_FilePath;
	CString	m_LaunchPath;
	CString	m_StartFolder;
	CString	m_Module;
	CString	m_Title;
	CString	m_OpenGLLib;
	CString	m_Notes;
	CString	m_Registry;
	BOOL	m_SlowDown;
	BOOL	m_BlitFromBackBuffer;
	BOOL	m_NoFlipEmulation;
	BOOL	m_OffscreenZBuffer;
	BOOL	m_LockColorDepth;
	BOOL	m_DisableGammaRamp;
	BOOL	m_AutoRefresh;
	BOOL	m_IndependentRefresh;
	BOOL	m_TextureFormat;
	BOOL	m_FixWinFrame;
	BOOL	m_VideoToSystemMem;
	BOOL	m_FixTextOut;
	BOOL	m_SharedDC;
	BOOL	m_HookGlide;
	BOOL	m_RemapMCI;
	BOOL	m_NoMovies;
	BOOL	m_FixMoviesColor;
	BOOL	m_StretchMovies;
	BOOL	m_BypassMCI;
	BOOL	m_SuppressRelease;
	BOOL	m_KeepCursorWithin;
	BOOL	m_KeepCursorFixed;
	BOOL	m_UseRGB565;
	BOOL	m_SuppressDXErrors;
	BOOL	m_FlipEmulation;
	BOOL	m_MarkBlit;
	BOOL	m_MarkLock;
	BOOL	m_MarkWinG32;
	BOOL	m_MarkGDI32;
	BOOL	m_DumpDIBSection;
	BOOL	m_DumpDevContext;
	BOOL	m_NoSysMemPrimary;
	BOOL	m_NoSysMemBackBuf;
	BOOL	m_NoBlt;
	BOOL	m_BilinearBlt;
	BOOL	m_FastBlt;
	BOOL	m_GDIColorConv;
	BOOL	m_PreventMaximize;
	BOOL	m_ClientRemapping;
	BOOL	m_LockWinStyle;
	BOOL	m_FixParentWin;
	BOOL	m_ModalStyle;
	BOOL	m_KeepAspectRatio;
	BOOL	m_ForceWinResize;
	BOOL	m_HideMultiMonitor;
	BOOL	m_FixD3DFrame;
	BOOL	m_NoWindowMove;
	BOOL	m_HookChildWin;
	BOOL	m_MessageProc;
	BOOL	m_FixMouseHook;
	BOOL	m_FixNCHITTEST;
	BOOL	m_RecoverScreenMode;
	BOOL	m_RefreshOnResize;
	BOOL	m_Init8BPP;
	BOOL	m_Init16BPP;
	BOOL	m_BackBufAttach;
	BOOL	m_ClearTextureFourCC;
	BOOL	m_NoDDExclusiveMode;
	BOOL	m_CreateDesktop;
	BOOL	m_AllowSysmemOn3DDev;
	BOOL	m_HandleAltF4;
	BOOL	m_HandleExceptions;
	BOOL	m_VirtualHeap;
	BOOL	m_SkipFPS;
	BOOL	m_LimitFPS;
	BOOL	m_ShowFPS;
	BOOL	m_ShowFPSOverlay;
	BOOL	m_ShowTimeStretch;
	BOOL	m_TimeStretch;
	BOOL	m_StretchTimers;
	BOOL	m_NormalizePerfCount;
	BOOL	m_CPUSlowDown;
	BOOL	m_CPUMaxUsage;
	BOOL	m_QuarterBlt;
	BOOL	m_FineTiming;
	BOOL	m_EnableTimeFreeze;
	BOOL	m_ReleaseMouse;
	BOOL	m_EnableHotKeys;
	BOOL	m_InterceptRDTSC;
	BOOL	m_HookOpenGL;
	BOOL	m_ForceHookOpenGL;
	BOOL	m_FixPixelZoom;
	BOOL	m_FixBindTexture;
	BOOL	m_HookGlut32;
	BOOL	m_FakeVersion;
	BOOL	m_FullRectBlt;
	BOOL	m_CenterToWin;
	BOOL	m_Deinterlace;
	BOOL	m_LimitDdraw;
	BOOL	m_SuppressOverlay;
	BOOL	m_SurfaceWarn;
	BOOL	m_CapMask;
	BOOL	m_NoWindowHooks;
	BOOL	m_NoDDRAWBlt;
	BOOL	m_NoDDRAWFlip;
	BOOL	m_NoGDIBlt;
	BOOL	m_NoFillRect;
	BOOL	m_FixClipperArea;
	BOOL	m_SharedDCHybrid;
	BOOL	m_SyncPalette;
	BOOL	m_NoWinErrors;
	BOOL	m_PretendVisible;
	BOOL	m_WinInsulation;
	BOOL	m_QualityFonts;
	BOOL	m_AnalyticMode;
	BOOL	m_NoPaletteUpdate;
	BOOL	m_LimitResources;
	BOOL	m_CDROMDriveType;
	BOOL	m_HideCDROMEmpty;
	BOOL	m_HookGOGLibs;
	BOOL	m_BypassGOGLibs;
	BOOL	m_FontBypass;
	BOOL	m_BufferedIOFix;
	BOOL	m_ZBufferClean;
	BOOL	m_ZBuffer0Clean;
	BOOL	m_DynamicZClean;
	BOOL	m_ZBufferHardClean;
	BOOL	m_ZBufferAlways;
	BOOL	m_HotPatchAlways;
	BOOL	m_FreezeInjectedSon;
	BOOL	m_StressResources;
	BOOL	m_NoPower2Fix;
	BOOL	m_NoPerfCounter;
	BOOL	m_DisableFogging;
	BOOL	m_ClearTarget;
	BOOL	m_FixPitch;
	BOOL	m_Power2Width;
	BOOL	m_SuppressIME;
	BOOL	m_SetCompatibility;
	BOOL	m_AEROBoost;
	BOOL	m_DiabloTweak;
	BOOL	m_HookDirectSound;
	BOOL	m_HookWinG32;
	BOOL	m_HookSmackW32;
	BOOL	m_FixSmackLoop;
	BOOL	m_BlockPriorityClass;
	BOOL	m_EASportsHack;
	BOOL	m_LegacyAlloc;
	BOOL	m_DisableMaxWinMode;
	BOOL	m_DisableDisableAltTab;
	BOOL	m_NoImagehlp;
	BOOL	m_ForcesHEL;
	BOOL	m_NoHALDevice;
	BOOL	m_MinimalCaps;
	BOOL	m_SetZBufferBitDepths;
	BOOL	m_ForcesSwapEffect;
	BOOL	m_ColorFix;
	BOOL	m_FixGlobalUnlock;
	BOOL	m_FixFreeLibrary;
	BOOL	m_LoadLibraryErr;
	BOOL	m_NoPixelFormat;
	BOOL	m_NoAlphaChannel;
	BOOL	m_FixRefCounter;
	BOOL	m_ReturnNullRef;
	BOOL	m_NoD3DReset;
	BOOL	m_HideDesktop;
	BOOL	m_HideTaskbar;
	BOOL	m_ActivateApp;
	BOOL	m_UnlockZOrder;
	BOOL	m_NoDestroyWindow;
	BOOL	m_LockSysColors;
	BOOL	m_LockReservedPalette;
	BOOL	m_SingleProcAffinity;
	BOOL	m_UseLastCore;
	BOOL	m_WireFrame;
	BOOL	m_NoTextures;
	BOOL	m_BlackWhite;
	BOOL	m_SuppressD3DExt;
	BOOL	m_Enum16bitModes;
	BOOL	m_TrimTextureFormats;
	BOOL	m_D3D8Back16;
	//BOOL	m_RawFormat;
	BOOL	m_LimitScreenRes;
	BOOL	m_InitialRes;
	BOOL	m_MaximumRes;
	int		m_VSyncMode;
	int		m_VSyncImpl;
	int		m_WaitMode;
	int		m_PosX;
	int		m_PosY;
	int		m_SizX;
	int		m_SizY;
	int		m_InitResW;
	int		m_InitResH;
	int		m_MaxFPS;
	int		m_InitTS;
	int		m_FakeVersionId;
	int		m_MaxScreenRes;
	int		m_ResTypes;
	int		m_SwapEffect;
	int		m_ClipperMode;
	int		m_ScanLine;
	int		m_TextureFileFormat;
	//}}AFX_DATA


// Override
	// ClassWizard Generates an overriding virtual function.
	//{{AFX_VIRTUAL(CTargetDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV Support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CTargetDlg)
	afx_msg void OnOpen();
	afx_msg void OnOpenLaunch();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:
	virtual BOOL OnInitDialog();

	afx_msg void OnBnClickedTry();
	afx_msg void OnBnClickedKill();
};
//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ Will insert additional declarations immediately before the previous line.

#endif