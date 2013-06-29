#if !defined(AFX_TABLOGS_H__7E062B52_3B6E_44C4_B58E_AAD73592C888__INCLUDED_)
#define AFX_TABLOGS_H__7E062B52_3B6E_44C4_B58E_AAD73592C888__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// TabColor.h : header file
//
#include "resource.h"

/////////////////////////////////////////////////////////////////////////////
// CTabLogs dialog

class CTabColor : public CDialog
{
// Construction
public:
	CTabColor(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CTabLogs)
	enum { IDD = IDD_TAB_LOG };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTabLogs)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CTabLogs)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif
