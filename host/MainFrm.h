// MainFrm.h : CMainFrame class definition and interface declarations.
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_MAINFRM_H__B2E9EA08_683A_47A3_ADA1_C6EE97866607__INCLUDED_)
#define AFX_MAINFRM_H__B2E9EA08_683A_47A3_ADA1_C6EE97866607__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CMainFrame : public CFrameWnd
{
	
protected: // Create from serialization only features.
	CMainFrame();
	DECLARE_DYNCREATE(CMainFrame)

// Attributes
public:

// Operation
public:

// Override
	// ClassWizard Generates a virtual function overrides.
	//{{AFX_VIRTUAL(CMainFrame)
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CMainFrame();
	int x, y, cx, cy;
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

// Generated message map functions
protected:
	//{{AFX_MSG(CMainFrame)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnMove(int, int);
	afx_msg void OnSize(UINT, int, int);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ Will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MAINFRM_H__B2E9EA08_683A_47A3_ADA1_C6EE97866607__INCLUDED_)
