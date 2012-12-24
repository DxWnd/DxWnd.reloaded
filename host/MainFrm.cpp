/*
DXWnd/dxwnd.cpp
DirectX Hook Module
Copyright(C) 2004-2011 SFB7/GHO

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

// MainFrm.cpp : CMainFrame defines the class behavior.
//

#define _CRT_SECURE_NO_DEPRECATE 1

#include "stdafx.h"
#include "dxwndhost.h"

#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMainFrame

IMPLEMENT_DYNCREATE(CMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
	//{{AFX_MSG_MAP(CMainFrame)
	ON_WM_CREATE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMainFrame class constructor / destructor

CMainFrame::CMainFrame()
{
	// form constructor starts the service thread responsible to mantain a fixed screen settings
	// see above ....
	// duplicate activation check
	if(CreateSemaphore(NULL, 0, 1, "DxWnd LOCKER")==NULL){
		MessageBoxEx(0, "CreateSemaphore FAILED.\nExiting.", "Warning", MB_OK | MB_ICONEXCLAMATION, NULL);
		return;
	}
	if(GetLastError()==ERROR_ALREADY_EXISTS){
		MessageBoxEx(0, "DxWnd is already running.\nExiting.", "Warning", MB_OK | MB_ICONEXCLAMATION, NULL);
		exit(1);
	}
}

CMainFrame::~CMainFrame()
{
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	return 0;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	cs.cx = 320;
	cs.cy = 200;
	if( !CFrameWnd::PreCreateWindow(cs) )
		return FALSE;
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CMainFrame Diagnostic Class

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CFrameWnd::Dump(dc);
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CMainFrame Message Handler

