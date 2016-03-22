#if !defined(AFX_TABMOUSE_H__12345678_C906_446C_822D_322B5AB6C4C4__INCLUDED_)
#define AFX_TABMOUSE_H__12345678_C906_446C_822D_322B5AB6C4C4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// TabHook.h : header file
//
#include "resource.h"
#include "TargetDlg.h"

/////////////////////////////////////////////////////////////////////////////
// CTabHook dialog

//class CTabHook : public CTargetDlg
class CTabHook : public CDialog
{
// Construction
public:
	CTabHook(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CTabDirectX)
	enum { IDD = IDD_TAB_HOOK };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTabDirectX)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CTabDirectX)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TABTHREE_H__798A9124_C906_446C_822D_322B5AB6C4C4__INCLUDED_)
