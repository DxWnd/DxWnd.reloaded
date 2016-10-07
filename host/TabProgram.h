#if !defined(AFX_TABPROGRAM_H__4F1DD92C_C67D_48AE_A73F_02D7EDA0580E__INCLUDED_)
#define AFX_TABPROGRAM_H__4F1DD92C_C67D_48AE_A73F_02D7EDA0580E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// TabProgram.h : header file
//
#include "resource.h"
#include "afxwin.h"
#include "specialedit.h"

/////////////////////////////////////////////////////////////////////////////
// CTabProgram dialog

class CTabProgram : public CDialog
{
// Construction
public:
	CTabProgram(CWnd* pParent = NULL);   // standard constructor
	void OnOpen(void);
	void OnOpenLaunch(void);

// Dialog Data
	//{{AFX_DATA(CTabProgram)
	enum { IDD = IDD_TAB_PROGRAM };

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTabProgram)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CTabProgram)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	BOOL OnInitDialog();
protected:
	CSpecialEdit m_EditPosX;
	CSpecialEdit m_EditPosY;
	afx_msg void OnDropFiles(HDROP hDropInfo);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif
