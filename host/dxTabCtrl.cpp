// dxTabCtrl.cpp : implementation file
//
/////////////////////////////////////////////////////
// This class is provided as is and Ben Hill takes no
// responsibility for any loss of any kind in connection
// to this code.
/////////////////////////////////////////////////////
// Is is meant purely as a educational tool and may
// contain bugs.
/////////////////////////////////////////////////////
// ben@shido.fsnet.co.uk
// http://www.shido.fsnet.co.uk
/////////////////////////////////////////////////////
// Thanks to a mystery poster in the C++ forum on 
// www.codeguru.com I can't find your name to say thanks
// for your Control drawing code. If you are that person 
// thank you very much. I have been able to use some of 
// you ideas to produce this sample application.
/////////////////////////////////////////////////////

#include "stdafx.h"
#include "dxTabCtrl.h"

#include "TabProgram.h"
#include "TabLogs.h"
#include "TabDirectX.h"
#include "TabMouse.h"
#include "TabTiming.h"
#include "TabWindow.h"
#include "TabOpenGL.h"
#include "TabCompat.h"
#include "TabGDI.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDXTabCtrl

CDXTabCtrl::CDXTabCtrl()
{
	m_tabPages[0]=new CTabProgram;
	m_tabPages[1]=new CTabWindow;
	m_tabPages[2]=new CTabMouse;
	m_tabPages[3]=new CTabTiming;
	m_tabPages[4]=new CTabLogs;
	m_tabPages[5]=new CTabDirectX;
	m_tabPages[6]=new CTabOpenGL;
	m_tabPages[7]=new CTabGDI;
	m_tabPages[8]=new CTabCompat;

	m_nNumberOfPages=9;
}

CDXTabCtrl::~CDXTabCtrl()
{
	for(int nCount=0; nCount < m_nNumberOfPages; nCount++){
		delete m_tabPages[nCount];
	}
}

void CDXTabCtrl::Init()
{
	m_tabCurrent=0;

	m_tabPages[0]->Create(IDD_TAB_PROGRAM, this);
	m_tabPages[1]->Create(IDD_TAB_WINDOW, this);
	m_tabPages[2]->Create(IDD_TAB_MOUSE, this);
	m_tabPages[3]->Create(IDD_TAB_TIMING, this);
	m_tabPages[4]->Create(IDD_TAB_LOG, this);
	m_tabPages[5]->Create(IDD_TAB_DIRECTX, this);
	m_tabPages[6]->Create(IDD_TAB_OPENGL, this);
	m_tabPages[7]->Create(IDD_TAB_GDI, this);
	m_tabPages[8]->Create(IDD_TAB_COMPAT, this);

	for(int nCount=0; nCount < m_nNumberOfPages; nCount++){
		m_tabPages[nCount]->ShowWindow(nCount ? SW_HIDE:SW_SHOW);
	}

	SetRectangle();
}

void CDXTabCtrl::SetRectangle()
{
	CRect tabRect, itemRect;
	int nX, nY, nXc, nYc;

	GetClientRect(&tabRect);
	GetItemRect(0, &itemRect);

	nX=itemRect.left;
	nY=itemRect.bottom+1;
	nXc=tabRect.right-itemRect.left-1;
	nYc=tabRect.bottom-nY-1;

	m_tabPages[0]->SetWindowPos(&wndTop, nX, nY, nXc, nYc, SWP_SHOWWINDOW);
	for(int nCount=1; nCount < m_nNumberOfPages; nCount++){
		m_tabPages[nCount]->SetWindowPos(&wndTop, nX, nY, nXc, nYc, SWP_HIDEWINDOW);
	}
}

BEGIN_MESSAGE_MAP(CDXTabCtrl, CTabCtrl)
	//{{AFX_MSG_MAP(CDXTabCtrl)
	ON_WM_LBUTTONDOWN()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDXTabCtrl message handlers

void CDXTabCtrl::OnLButtonDown(UINT nFlags, CPoint point) 
{
	CTabCtrl::OnLButtonDown(nFlags, point);

	if(m_tabCurrent != GetCurFocus()){
		m_tabPages[m_tabCurrent]->ShowWindow(SW_HIDE);
		m_tabCurrent=GetCurFocus();
		m_tabPages[m_tabCurrent]->ShowWindow(SW_SHOW);
		m_tabPages[m_tabCurrent]->SetFocus();
	}
}

void CDXTabCtrl::OnOK()
{
	for(int nCount=0; nCount < m_nNumberOfPages; nCount++){
		m_tabPages[nCount]->UpdateData(TRUE);
	}
}
