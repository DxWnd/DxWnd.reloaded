#if !defined(AFX_TabDirect3D_H__798A9124_C906_446C_822D_322B5AB6C4C4__INCLUDED_)
#define AFX_TabDirect3D_H__798A9124_C906_446C_822D_322B5AB6C4C4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// TabDirect3D.h : header file
//
#include "resource.h"

/////////////////////////////////////////////////////////////////////////////
// CTabDirect3D dialog

class CTabDirect3D : public CDialog
{
// Construction
public:
	CTabDirect3D(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CTabDirect3D)
	enum { IDD = IDD_TAB_DIRECTX };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTabDirect3D)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CTabDirect3D)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif
