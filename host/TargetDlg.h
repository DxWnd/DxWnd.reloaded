#if !defined(AFX_TARGETDLG_H__683E529D_7282_484C_A403_367A6A90CC57__INCLUDED_)
#define AFX_TARGETDLG_H__683E529D_7282_484C_A403_367A6A90CC57__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// TargetDlg.h : Header file
//


/////////////////////////////////////////////////////////////////////////////
// CTargetDlg Dialog

class CTargetDlg : public CDialog
{
// Constructor
public:
	CTargetDlg(CWnd* pParent = NULL);   // Standard constructor

// Dialog Data
	//{{AFX_DATA(CTargetDlg)
	enum { IDD = IDD_TARGET };
	CEdit	m_File;
	int		m_DXVersion;
	BOOL	m_EmulateSurface;
	BOOL	m_NoEmulateSurface;
	BOOL	m_EmulateBuffer;
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
	BOOL	m_HandleDC;
	BOOL	m_UnNotify;
	CString	m_FilePath;
	CString	m_Module;
	CString	m_Title;
	BOOL	m_SaveLoad;
	BOOL	m_SlowDown;
	BOOL	m_BlitFromBackBuffer;
	BOOL	m_SuppressClipping;
	BOOL	m_DisableGammaRamp;
	BOOL	m_AutoRefresh;
	BOOL	m_FixWinFrame;
	BOOL	m_HideHwCursor;
	BOOL	m_EnableClipping;
	BOOL	m_CursorClipping;
	BOOL	m_VideoToSystemMem;
	BOOL	m_FixTextOut;
	BOOL	m_KeepCursorWithin;
	BOOL	m_KeepCursorFixed;
	BOOL	m_UseRGB565;
	BOOL	m_SuppressDXErrors;
	BOOL	m_PreventMaximize;
	BOOL	m_ClientRemapping;
	BOOL	m_MapGDIToPrimary;
	BOOL	m_LockWinPos;
	BOOL	m_LockWinStyle;
	BOOL	m_FixParentWin;
	BOOL	m_ModalStyle;
	BOOL	m_KeepAspectRatio;
	BOOL	m_ForceWinResize;
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
	afx_msg void OnBnClickedUnnotify();
	afx_msg void OnBnClickedEmulateBuffer();
	afx_msg void OnBnClickedEmulateSurface();
	afx_msg void OnBnClickedNoEmulateSurface();
	afx_msg void OnBnClickedOuttrace();
	afx_msg void OnBnClickedLogEnabled();
	afx_msg void OnBnClickedHandleDC();
	afx_msg void OnEnChangeFile2();
	afx_msg void OnBnClickedAutorefresh();
	afx_msg void OnBnClickedFixWinFrame();
	afx_msg void OnBnClickedHideHwCursor();
	afx_msg void OnBnClickedEnableClipping();
	afx_msg void OnBnClickedHandleCursorPos();
	afx_msg void OnBnClickedFixTextOut();
	afx_msg void OnBnClickedKeepCursorWithin();
	afx_msg void OnBnClickedKeepCursorFixed();
	afx_msg void OnBnClickedUseRGB565();
	afx_msg void OnBnClickedCursorScaling();
	afx_msg void OnBnClickedPreventMaximize();
	afx_msg void OnBnClickedClientRemapping();
	afx_msg void OnBnClickedMapGDIToPrimary();
	afx_msg void OnBnClickedLockWinPos();
	afx_msg void OnBnClickedLockWinStyle();
	afx_msg void OnBnClickedHookChildWin();
	afx_msg void OnBnClickedMessageProc();
	afx_msg void OnBnClickedRecoverScreenMode();
	afx_msg void OnBnClickedRefreshOnResize();
	afx_msg void OnBnClickedInit8BPP();
	afx_msg void OnBnClickedInit16BPP();
	afx_msg void OnBnClickedBackBufAttach();
	afx_msg void OnBnClickedHandleAltF4();
	afx_msg void OnBnClickedHandleExceptions();
	afx_msg void OnEnChangeMaxx();
	afx_msg void OnEnChangeInitx();
};
//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ Will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TARGETDLG_H__683E529D_7282_484C_A403_367A6A90CC57__INCLUDED_)
