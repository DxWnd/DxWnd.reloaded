// DesktopDialog.cpp : implementation file
//

#include "stdafx.h"
#include "dxwndhost.h"
#include "DesktopDialog.h"

// CDesktopDialog dialog

IMPLEMENT_DYNAMIC(CDesktopDialog, CDialog)

CDesktopDialog::CDesktopDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CDesktopDialog::IDD, pParent)
{
}

CDesktopDialog::~CDesktopDialog()
{
}

void CDesktopDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CDesktopDialog, CDialog)
	ON_WM_TIMER()
END_MESSAGE_MAP()

#define IDDesktopTIMER 2

// CDesktopDialog message handlers

void CDesktopDialog::OnTimer(UINT_PTR nIDEvent)
{
	char sMsg[1024];

	CDialog::OnTimer(nIDEvent);

	RECT desktop;
	HDC hDC;
	// Get a handle to the desktop window
	const CWnd *hDesktop = GetDesktopWindow();
	// Get the size of screen to the variable desktop
	hDesktop->GetWindowRect(&desktop);
	// The top left corner will have coordinates (0,0)
	// and the bottom right corner will have coordinates
	// (horizontal, vertical)
	hDC = ::GetDC(::GetDesktopWindow());
	int iBPP = GetDeviceCaps(hDC, BITSPIXEL);

	PIXELFORMATDESCRIPTOR pfd;
	int  iPixelFormat;
	// get the current pixel format index 
#if 0
	::DescribePixelFormat(hDC, 1, sizeof(PIXELFORMATDESCRIPTOR), &pfd);
	HMODULE ogl;
	//ogl=LoadLibrary("opengl32");
	ogl=LoadLibrary("gdi32");
	typedef BOOL (WINAPI *wglGetPixelFormat_Type)(HDC);
	wglGetPixelFormat_Type pwglGetPixelFormat;
	//pwglGetPixelFormat = (wglGetPixelFormat_Type)GetProcAddress(ogl, "wglGetPixelFormat");
	pwglGetPixelFormat = (wglGetPixelFormat_Type)GetProcAddress(ogl, "GetPixelFormat");
	iPixelFormat = (*pwglGetPixelFormat)(hDC); 
	iPixelFormat = (*pwglGetPixelFormat)(NULL); 
	//iPixelFormat = ::GetPixelFormat(NULL); 
#endif
	iPixelFormat = ::GetPixelFormat(hDC); 
	if(!iPixelFormat) iPixelFormat=1; // why returns 0???
	if(iPixelFormat){
		// obtain a detailed description of that pixel format  
		::DescribePixelFormat(hDC, iPixelFormat, sizeof(PIXELFORMATDESCRIPTOR), &pfd);
		sprintf_s(sMsg, 1024, 
			"Desktop size (W x H)=(%d x %d)\n" 
			"Color depth = %d (color bits = %d)\n"
			"Pixel format = %d\n"
			"Color bits  (RGBA)=(%d,%d,%d,%d)\n" 
			"Color shift (RGBA)=(%d,%d,%d,%d)\n"
			, 
			desktop.right, desktop.bottom, 
			iBPP, pfd.cColorBits,
			iPixelFormat,
			pfd.cRedBits, pfd.cGreenBits, pfd.cBlueBits, pfd.cAlphaBits,
			pfd.cRedShift, pfd.cGreenShift, pfd.cBlueShift, pfd.cAlphaShift
			);
	}
	else{
		sprintf_s(sMsg, 1024, "error %d", GetLastError());
	}

	this->SetDlgItemTextA(IDC_DESKTOPINFO, sMsg);
}

BOOL CDesktopDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

	SetTimer(IDDesktopTIMER, 1000, NULL);

	// TODO:  Add extra initialization here

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CDesktopDialog::OnOK()
{
	// TODO: Add your specialized code here and/or call the base class

	KillTimer(IDDesktopTIMER);
	// stop timer

	CDialog::OnOK();
}
