// dxwndhostView.h : CDxwndhostView class definition and interface declarations.
//
/////////////////////////////////////////////////////////////////////////////

#include "SystemTray.h"

#if !defined(AFX_DXWNDHOSTVIEW_H__E8E112CE_6FE9_4F01_AB94_D53F844D5A09__INCLUDED_)
#define AFX_DXWNDHOSTVIEW_H__E8E112CE_6FE9_4F01_AB94_D53F844D5A09__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CDxwndhostView : public CListView
{
protected: // Create from serialization only features.
	CDxwndhostView();
	DECLARE_DYNCREATE(CDxwndhostView)

private:
	void Resize(void);
	void SaveConfigFile();
	TARGETMAP TargetMaps[MAXTARGETS];
	PRIVATEMAP TitleMaps[MAXTARGETS];
	char InitPath[MAX_PATH];
	BOOL isUpdated;
	DEVMODE InitDevMode;
	CSystemTray SystemTray;
	int LastX, LastY, LastCX, LastCY;
	CImageList m_cImageListNormal, m_cImageListSmall;

// Attributes
public:
	CDxwndhostDoc* GetDocument();

// Operation
public:

// Override
	// ClassWizard Generates a virtual function overrides.
	//{{AFX_VIRTUAL(CDxwndhostView)
	public:
	virtual void OnDraw(CDC* pDC);  // Be overridden when rendering this view.
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	protected:
	virtual void OnInitialUpdate(); // Called once after the initial construction.
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CDxwndhostView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CDxwndhostView)
	afx_msg void OnDblclk(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnAdd();
	afx_msg void OnModify();
	afx_msg void OnExport();
	afx_msg void OnImport();
	afx_msg void OnDelete();
	afx_msg void OnExplore();
	afx_msg void OnKill();
	afx_msg void OnPause();
	afx_msg void OnResume();
	afx_msg void OnSort();
	afx_msg void OnViewLog();
	afx_msg void OnDeleteLog();
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRun();
	afx_msg void OnClearAllLogs();
	afx_msg void OnGoToTrayIcon();
	afx_msg void OnSaveFile();
	afx_msg void OnHookStart();
	afx_msg void OnInitMenu();
	afx_msg void OnHookStop();
	afx_msg void OnTrayRestore();
	afx_msg void OnViewStatus();
	afx_msg void OnViewTimeSlider();
	afx_msg void OnExit();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // dxwndhostView.cpp for debugging environment.
inline CDxwndhostDoc* CDxwndhostView::GetDocument()
   { return (CDxwndhostDoc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ Will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DXWNDHOSTVIEW_H__E8E112CE_6FE9_4F01_AB94_D53F844D5A09__INCLUDED_)
