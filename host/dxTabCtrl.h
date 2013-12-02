#if !defined(AFX_MYTABCTRL_H__F3E8650F_019C_479F_9E0F_60FE1181F49F__INCLUDED_)
#define AFX_MYTABCTRL_H__F3E8650F_019C_479F_9E0F_60FE1181F49F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// dxTabCtrl.h : header file
//
#include "resource.h"

/////////////////////////////////////////////////////////////////////////////
// CDXTabCtrl window

class CDXTabCtrl : public CTabCtrl
{
// Construction
public:
	CDXTabCtrl();
	CDialog *m_tabPages[9];
	int m_tabCurrent;
	int m_nNumberOfPages;
	enum { IDD = IDC_TABPANEL };

// Attributes
public:

// Operations
public:
	void Init();
	void SetRectangle();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDXTabCtrl)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CDXTabCtrl();
	void OnOK();

	// Generated message map functions
protected:
	//{{AFX_MSG(CDXTabCtrl)
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MYTABCTRL_H__F3E8650F_019C_479F_9E0F_60FE1181F49F__INCLUDED_)
