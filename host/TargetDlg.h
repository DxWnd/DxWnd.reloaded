#if !defined(AFX_TARGETDLG_H__683E529D_7282_484C_A403_367A6A90CC57__INCLUDED_)
#define AFX_TARGETDLG_H__683E529D_7282_484C_A403_367A6A90CC57__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// TargetDlg.h : Header file
//
#include "dxTabCtrl.h"

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
	CEdit	m_File;
	int		m_DXVersion;
	int		m_Coordinates;
	int		m_DxEmulationMode;
	int		m_DCEmulationMode;
	BOOL	m_HookDI;
	BOOL	m_ModifyMouse;
	BOOL	m_OutTrace;
	BOOL	m_OutDebug;
	BOOL	m_CursorTrace;
	BOOL	m_LogEnabled;
	BOOL	m_OutWinMessages;
	BOOL	m_OutDXTrace;
	BOOL	m_DXProxed;
	BOOL	m_AssertDialog;
	BOOL	m_ImportTable;
	BOOL	m_RegistryOp;
	BOOL	m_TraceHooks;
	BOOL	m_HandleDC;
	BOOL	m_UnNotify;
	BOOL	m_Windowize;
	BOOL	m_HookDLLs;
	BOOL	m_EmulateRegistry;
	BOOL	m_FullScreenOnly;
	BOOL	m_FilterMessages;
	BOOL	m_PeekAllMessages;
	BOOL	m_NoBanner;
	BOOL	m_StartDebug;
	BOOL	m_HookEnabled;
	CString	m_FilePath;
	CString	m_Module;
	CString	m_Title;
	CString	m_OpenGLLib;
	BOOL	m_SaveLoad;
	BOOL	m_SlowDown;
	BOOL	m_BlitFromBackBuffer;
	BOOL	m_SuppressClipping;
	BOOL	m_DisableGammaRamp;
	BOOL	m_AutoRefresh;
	BOOL	m_FixWinFrame;
	BOOL	m_HideHwCursor;
	BOOL	m_ShowHwCursor;
	BOOL	m_EnableClipping;
	BOOL	m_CursorClipping;
	BOOL	m_VideoToSystemMem;
	BOOL	m_FixTextOut;
	BOOL	m_KeepCursorWithin;
	BOOL	m_KeepCursorFixed;
	BOOL	m_UseRGB565;
	BOOL	m_SuppressDXErrors;
	BOOL	m_MarkBlit;
	BOOL	m_PreventMaximize;
	BOOL	m_ClientRemapping;
	BOOL	m_LockWinPos;
	BOOL	m_LockWinStyle;
	BOOL	m_FixParentWin;
	BOOL	m_ModalStyle;
	BOOL	m_KeepAspectRatio;
	BOOL	m_ForceWinResize;
	BOOL	m_HideMultiMonitor;
	BOOL	m_WallpaperMode;
	BOOL	m_FixD3DFrame;
	BOOL	m_NoWindowMove;
	BOOL	m_HookChildWin;
	BOOL	m_MessageProc;
	BOOL	m_FixNCHITTEST;
	BOOL	m_RecoverScreenMode;
	BOOL	m_RefreshOnResize;
	BOOL	m_Init8BPP;
	BOOL	m_Init16BPP;
	BOOL	m_BackBufAttach;
	BOOL	m_HandleAltF4;
	BOOL	m_HandleExceptions;
	BOOL	m_SkipFPS;
	BOOL	m_LimitFPS;
	BOOL	m_ShowFPS;
	BOOL	m_ShowFPSOverlay;
	BOOL	m_ShowTimeStretch;
	BOOL	m_TimeStretch;
	BOOL	m_HookOpenGL;
	BOOL	m_ForceHookOpenGL;
	BOOL	m_FakeVersion;
	BOOL	m_FullRectBlt;
	BOOL	m_SurfaceWarn;
	BOOL	m_CapMask;
	BOOL	m_NoDDRAWBlt;
	BOOL	m_NoDDRAWFlip;
	BOOL	m_NoGDIBlt;
	BOOL	m_AnalyticMode;
	BOOL	m_NoPaletteUpdate;
	BOOL	m_LimitResources;
	BOOL	m_CDROMDriveType;
	BOOL	m_FontBypass;
	BOOL	m_BufferedIOFix;
	BOOL	m_ZBufferClean;
	BOOL	m_ZBuffer0Clean;
	BOOL	m_ZBufferAlways;
	BOOL	m_SuppressIME;
	BOOL	m_SetCompatibility;
	BOOL	m_DisableHAL;
	BOOL	m_ForcesHEL;
	BOOL	m_ColorFix;
	BOOL	m_NoPixelFormat;
	BOOL	m_NoAlphaChannel;
	BOOL	m_FixRefCounter;
	BOOL	m_SuppressChild;
	BOOL	m_LockSysColors;
	BOOL	m_SaveCaps;
	BOOL	m_SingleProcAffinity;
	BOOL	m_WireFrame;
	BOOL	m_BlackWhite;
	BOOL	m_SuppressD3DExt;
	BOOL	m_Force16BPP;
	BOOL	m_ForceYUVtoRGB;
	BOOL	m_ForceRGBtoYUV;
	int		m_InitX;
	int		m_InitY;
	int		m_MaxX;
	int		m_MaxY;
	int		m_MinX;
	int		m_MinY;
	int		m_PosX;
	int		m_PosY;
	int		m_SizX;
	int		m_SizY;
	int		m_MaxFPS;
	int		m_InitTS;
	int		m_FakeVersionId;
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
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:
	virtual BOOL OnInitDialog();

};
//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ Will insert additional declarations immediately before the previous line.

#endif