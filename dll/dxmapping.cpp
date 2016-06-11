#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include "dxwnd.h"
#include "dxwcore.hpp"
#include "syslibs.h"
#include "dxhelper.h"

// IsValidMainWindow: returns TRUE if the main window can be queried for coordinates, 
// FALSE otherwise (e.g. when minimized)

BOOL dxwCore::IsValidMainWindow()
{
	RECT Client;
	POINT UpLeft = {0, 0};
	if(!(*pGetClientRect)(hWnd, &Client)) return FALSE;
	if((Client.right == 0) || (Client.bottom == 0)) return FALSE;
	if(!(*pClientToScreen)(hWnd, &UpLeft)) return FALSE;
	return TRUE;
}

// if the main window coordinates are still valid updates the window placement values

void dxwCore::UpdateDesktopCoordinates()
{
	RECT Client;
	POINT UpLeft = {0, 0};

	OutTraceB("dxwCore::UpdateDesktopCoordinates: OLD pos=(%d,%d) size=(%dx%d)\n", iPosX, iPosY, iSizX, iSizY);
	if(!(*pGetClientRect)(hWnd, &Client)) return;
	if((Client.right == 0) || (Client.bottom == 0)) return;
	if(!(*pClientToScreen)(hWnd, &UpLeft)) return;
	// update coordinates ....
	iPosX = UpLeft.x;
	iPosY = UpLeft.y;
	iSizX = Client.right - Client.left;
	iSizY = Client.bottom - Client.top;
	OutTraceB("dxwCore::UpdateDesktopCoordinates: NEW pos=(%d,%d) size=(%dx%d)\n", iPosX, iPosY, iSizX, iSizY);
}

// GetScreenRect: returns a RECT sized as the virtual desktop

RECT dxwCore::GetScreenRect()
{
	static RECT Screen;
	Screen.left=0;
	Screen.top=0;
	Screen.right=dwScreenWidth;
	Screen.bottom=dwScreenHeight;
	return Screen;
}

// GetUnmappedScreenRect: returns the RECT of the real coordinates of the virtual desktop, 
// that is the client area of the main window 

RECT dxwCore::GetUnmappedScreenRect()
{
	RECT Screen;
	Screen.top = iPosY;
	Screen.bottom = iPosY + iSizX;
	Screen.left = iPosX;
	Screen.right = iPosX + iSizX;
	return Screen;
}

// GetFrameOffset: return the POINT coordinates of the upper-left corner of the main window frame
// taking in account of the window frame and title bar, then it needs to query hWnd

POINT dxwCore::GetFrameOffset()
{
	RECT wrect;
	POINT FrameOffset={0, 0};
	(*pGetWindowRect)(hWnd, &wrect);
	(*pClientToScreen)(hWnd, &FrameOffset);
	FrameOffset.x -= wrect.left;
	FrameOffset.y -= wrect.top;
	OutTraceB("GetFrameOffset: offset=(%d,%d)\n", FrameOffset.x, FrameOffset.y);
	return FrameOffset;
}

// ClientOffset: returns the virtual offset of a window within the virtual desktop

POINT dxwCore::ClientOffset(HWND hwnd)
{
	POINT upleft, ret;

	upleft.x = upleft.y = 0;
	(*pClientToScreen)(hwnd, &upleft);
	ret.x = (((upleft.x - iPosX) * dwScreenWidth) + (iSizX >> 1)) / iSizX;
	ret.y = (((upleft.y - iPosY) * dwScreenHeight) + (iSizY >> 1)) / iSizY;
	if(ret.x < 0) ret.x = 0;
	if(ret.y < 0) ret.y = 0;
	OutTraceB("ClientOffset: hwnd=%x offset=(%d,%d)\n", hwnd, ret.x, ret.y);
	return ret;
}

// GetWindowRect: returns the virtual coordinates of a window within the virtual desktop

RECT dxwCore::GetWindowRect(RECT win)
{
	win.left = (((win.left - iPosX) * (LONG)dwScreenWidth) + (iSizX >> 1)) / iSizX;
	win.top = (((win.top - iPosY) * (LONG)dwScreenHeight) + (iSizY >> 1)) / iSizY;
	win.right = (((win.right - iPosX) * (LONG)dwScreenWidth) + (iSizX >> 1)) / iSizX;
	win.bottom = (((win.bottom - iPosY) * (LONG)dwScreenHeight) + (iSizY >> 1)) / iSizY;

	return win;
}

// GetClientRect: returns the virtual client coordinates of a window

RECT dxwCore::GetClientRect(RECT win)
{
	win.left = ((win.left * dwScreenWidth) + (iSizX >> 1)) / iSizX;
	win.top = ((win.top * dwScreenHeight) + (iSizY >> 1)) / iSizY;
	win.right = ((win.right * dwScreenWidth) + (iSizX >> 1)) / iSizX;
	win.bottom = ((win.bottom * dwScreenHeight) + (iSizY >> 1)) / iSizY;

	return win;
}

// AddCoordinates, SubCoordinates: adds or subtracts a POINT displacement

POINT dxwCore::AddCoordinates(POINT p1, POINT p2)
{
	POINT ps;
	ps.x = p1.x + p2.x;
	ps.y = p1.y + p2.y;
	return ps;
}

RECT dxwCore::AddCoordinates(RECT r1, POINT p2)
{
	RECT rs;
	rs.left = r1.left + p2.x;
	rs.right = r1.right + p2.x;
	rs.top = r1.top + p2.y;
	rs.bottom = r1.bottom + p2.y;
	return rs;
}

POINT dxwCore::SubCoordinates(POINT p1, POINT p2)
{
	POINT ps;
	ps.x = p1.x - p2.x;
	ps.y = p1.y - p2.y;
	return ps;
}

// MapWindow, MapWindowRect: transforms the virtual window coordinates of the virtual desktop into real ones

RECT dxwCore::MapWindowRect(LPRECT lpRect)
{
	RECT RetRect;

	if(!Windowize){
		if(lpRect) 
			RetRect=*lpRect;
		else{
			RetRect.left = RetRect.top = 0;
			RetRect.right = dwScreenWidth;
			RetRect.bottom = dwScreenHeight;
		}
		return RetRect;
	}
	
	if(lpRect){ 
		RetRect.left = iPosX + (lpRect->left * iSizX / dwScreenWidth);
		RetRect.right = iPosX + (lpRect->right * iSizX / dwScreenWidth);
		RetRect.top = iPosY + (lpRect->top * iSizY / dwScreenHeight);
		RetRect.bottom = iPosY + (lpRect->bottom * iSizY / dwScreenHeight);
	}
	else{
		RetRect.left = iPosX;
		RetRect.right = iPosX + iSizX;
		RetRect.top = iPosY;
		RetRect.bottom = iPosY + iSizY;
	}
	return RetRect;
}

void dxwCore::MapWindow(LPRECT rect)
{
	rect->left= iPosX + (((rect->left * iSizX) + (dwScreenWidth >> 1)) / (int)dwScreenWidth);
	rect->top= iPosY + (((rect->top * iSizY) + (dwScreenHeight >> 1)) / (int)dwScreenHeight);
	rect->right= iPosX + (((rect->right * iSizX) + (dwScreenWidth >> 1)) / (int)dwScreenWidth);
	rect->bottom= iPosY + (((rect->bottom * iSizY) + (dwScreenHeight >> 1)) / (int)dwScreenHeight);
}

void dxwCore::MapWindow(int *nXDest, int *nYDest, int *nWDest, int *nHDest)
{
	*nXDest= iPosX + (((*nXDest * iSizX) + (dwScreenWidth >> 1)) / (int)dwScreenWidth);
	*nYDest= iPosY + (((*nYDest * iSizY) + (dwScreenHeight >> 1)) / (int)dwScreenHeight);
	*nWDest= ((*nWDest * iSizX) + (dwScreenWidth >> 1)) / (int)dwScreenWidth;
	*nHDest= ((*nHDest * iSizY) + (dwScreenHeight >> 1)) / (int)dwScreenHeight;
}

void dxwCore::MapWindow(LPPOINT lppoint) 
{
	lppoint->x = iPosX + (((lppoint->x * iSizX) + (dwScreenWidth >> 1)) / dwScreenWidth);
	lppoint->y = iPosY + (((lppoint->y * iSizY) + (dwScreenHeight >> 1)) / dwScreenHeight);
}

// MapClient, MapClientRect: transforms the client virtual coordinates of the virtual desktop into real ones

RECT dxwCore::MapClientRect(LPRECT lpRect)
{
	RECT RetRect;
	if(lpRect){ 
		RetRect.left = (lpRect->left * iSizX / dwScreenWidth);
		RetRect.right = (lpRect->right * iSizX / dwScreenWidth);
		RetRect.top = (lpRect->top * iSizY / dwScreenHeight);
		RetRect.bottom = (lpRect->bottom * iSizY / dwScreenHeight);
	}
	else{
		RetRect.left = 0;
		RetRect.right = iSizX;
		RetRect.top = 0;
		RetRect.bottom = iSizY;
	}
	return RetRect;
}

void dxwCore::MapClient(LPRECT rect)
{
	rect->left= ((rect->left * iSizX)+(dwScreenWidth >> 1)) / (int)dwScreenWidth;
	rect->top= ((rect->top * iSizY)+(dwScreenHeight >> 1)) / (int)dwScreenHeight;
	rect->right= ((rect->right * iSizX)+(dwScreenWidth >> 1)) / (int)dwScreenWidth;
	rect->bottom= ((rect->bottom * iSizY)+(dwScreenHeight >> 1)) / (int)dwScreenHeight;
}

void dxwCore::MapClient(int *nXDest, int *nYDest, int *nWDest, int *nHDest)
{
	*nXDest= ((*nXDest * iSizX)+(dwScreenWidth >> 1)) / (int)dwScreenWidth;
	*nYDest= ((*nYDest * iSizY)+(dwScreenHeight >> 1)) / (int)dwScreenHeight;
	*nWDest= ((*nWDest * iSizX)+(dwScreenWidth >> 1)) / (int)dwScreenWidth;
	*nHDest= ((*nHDest * iSizY)+(dwScreenHeight >> 1)) / (int)dwScreenHeight;
}

void dxwCore::MapClient(LPPOINT lppoint)
{
	lppoint->x= ((lppoint->x * iSizX)+(dwScreenWidth >> 1)) / (int)dwScreenWidth;
	lppoint->y= ((lppoint->y * iSizY)+(dwScreenHeight >> 1)) / (int)dwScreenHeight;
}

void dxwCore::MapClient(int *nXDest, int *nYDest)
{
	*nXDest= ((*nXDest * iSizX)+(dwScreenWidth >> 1)) / (int)dwScreenWidth;
	*nYDest= ((*nYDest * iSizY)+(dwScreenHeight >> 1)) / (int)dwScreenHeight;
}

// UnmapWindow: transforms the window real coordinates of the real desktop into virtual ones

void dxwCore::UnmapWindow(LPRECT rect)
{
	rect->left= ((rect->left  - iPosX) * (int)dwScreenWidth) / iSizX;
	rect->top= ((rect->top  - iPosY) * (int)dwScreenHeight) / iSizY;
	rect->right= ((rect->right  - iPosX) * (int)dwScreenWidth) / iSizX;
	rect->bottom= ((rect->bottom  - iPosY) * (int)dwScreenHeight) / iSizY;
}

void dxwCore::UnmapWindow(LPPOINT point)
{
	point->x= ((point->x  - iPosX) * (int)dwScreenWidth) / iSizX;
	point->y= ((point->y  - iPosY) * (int)dwScreenHeight) / iSizY;
}

// UnmapClient: transforms the client real coordinates of the real desktop into virtual ones

void dxwCore::UnmapClient(LPPOINT lppoint)
{
	lppoint->x = ((lppoint->x * (int)dwScreenWidth) + (iSizX >> 1)) / iSizX;
	lppoint->y = ((lppoint->y * (int)dwScreenHeight) + (iSizY >> 1)) / iSizY;
}

void dxwCore::UnmapClient(int *nXDest, int *nYDest)
{
	*nXDest = ((*nXDest * (int)dwScreenWidth) + (iSizX >> 1)) / iSizX;
	*nYDest = ((*nYDest * (int)dwScreenHeight) + (iSizY >> 1)) / iSizY;
}

void dxwCore::UnmapClient(int *nXDest, int *nYDest, int *nWidth, int *nHeight)
{
	*nXDest = ((*nXDest * (int)dwScreenWidth) + (iSizX >> 1)) / iSizX;
	*nYDest = ((*nYDest * (int)dwScreenHeight) + (iSizY >> 1)) / iSizY;
	*nWidth = ((*nWidth * (int)dwScreenWidth) + (iSizX >> 1)) / iSizX;
	*nHeight = ((*nHeight * (int)dwScreenHeight) + (iSizY >> 1)) / iSizY;
}

void dxwCore::UnmapClient(LPRECT lpRect)
{
	lpRect->left = ((lpRect->left * (int)dwScreenWidth) + (iSizX >> 1)) / iSizX;
	lpRect->right = ((lpRect->right * (int)dwScreenWidth) + (iSizX >> 1)) / iSizX;
	lpRect->top = ((lpRect->top * (int)dwScreenHeight) + (iSizY >> 1)) / iSizY;
	lpRect->bottom = ((lpRect->bottom * (int)dwScreenHeight) + (iSizY >> 1)) / iSizY;
}

// GetMonitorWorkarea: retrieves the desktop coordinates of the whole desktop (id == -1) or of a given monitor (id >= 0)
// returning either the whole desktop area (WorkArea == FALSE) or the workarea (WorkArea == TRUE)

// callback function called by EnumDisplayMonitors for each enabled monitor 

struct ENUM_DISP_ARG { 
	int TargetId; 
	int MonId; 
	RECT rect;
	HMONITOR hMon;
};

BOOL CALLBACK EnumDispProc(HMONITOR hMon, HDC dcMon, RECT* pRcMon, LPARAM lParam) 
{ 
	ENUM_DISP_ARG* pArg = reinterpret_cast<ENUM_DISP_ARG*>(lParam); 
	if (pArg->MonId >= pArg->TargetId){
		pArg->rect = *pRcMon; 
		pArg->hMon = hMon;
		return FALSE;
	}
	else {
		pArg->MonId++; 
		return TRUE; 
	}
}

void dxwCore::GetMonitorWorkarea(LPRECT lpRect, BOOL WorkArea)
{
	if(MonitorId == -1) { // whole desktop from all monitors
		if(WorkArea) {
			(*pSystemParametersInfoA)(SPI_GETWORKAREA, NULL, lpRect, 0);
		}
		else {
			(*pGetClientRect)((*pGetDesktopWindow)(), lpRect);
		}
		return;
	}

	MONITORINFO mi;
	memset((void *)&mi, 0, sizeof(MONITORINFO));
	mi.cbSize = sizeof(MONITORINFO);
	ENUM_DISP_ARG arg = { 0 };
	arg.TargetId = MonitorId; // monitor to look for
	EnumDisplayMonitors(0, 0, EnumDispProc, reinterpret_cast<LPARAM>(&arg));

	if(GetMonitorInfo(arg.hMon, &mi)){
		*lpRect = WorkArea ? mi.rcWork : mi.rcMonitor; 
	}
	else{
		// recurse with undefined id
		MonitorId = -1;
		GetMonitorWorkarea(lpRect, WorkArea);
	}
	OutTraceB("dxwCore::GetMonitorWorkarea: id=%d workarea=%x rect=(%d,%d)-(%d,%d)\n",
		MonitorId, WorkArea, lpRect->left, lpRect->top, lpRect->right, lpRect->bottom);
}

// v.2.1.80: unified positioning logic into CalculateWindowPos routine
// now taking in account for window menus (see "Alien Cabal")
// sets the correct values in wp LPWINDOWPOS structure, but doesn't do any window update

void dxwCore::CalculateWindowPos(HWND hwnd, DWORD width, DWORD height, LPWINDOWPOS wp)
{
	RECT rect, desktop;
	DWORD dwStyle, dwExStyle;
	int MaxX, MaxY;
	HMENU hMenu;

	switch(Coordinates){
	case DXW_DESKTOP_CENTER:
		if(bAutoScale){
			MaxX = GetScreenWidth();
			MaxY = GetScreenHeight();
			if(dxw.dwFlags4 & BILINEAR2XFILTER) MaxX <<= 1; // double
			if(dxw.dwFlags4 & BILINEAR2XFILTER) MaxY <<= 1; // double
		}
		else{
			MaxX = iSizX;
			MaxY = iSizY;
		}
		if (!MaxX) {
			MaxX = width;
			if(dxw.dwFlags4 & BILINEAR2XFILTER) MaxX <<= 1; // double
		}
		if (!MaxY) {
			MaxY = height;
			if(dxw.dwFlags4 & BILINEAR2XFILTER) MaxY <<= 1; // double
		}
		dxw.GetMonitorWorkarea(&desktop, TRUE);
		rect.left =  (desktop.right + desktop.left - MaxX) / 2; // v2.03.89 - fixed
		rect.top = (desktop.bottom + desktop.top - MaxY) / 2;	// v2.03.89 - fixed
		rect.right = rect.left + MaxX;
		rect.bottom = rect.top + MaxY; //v2.02.09
		// fixed ....
		if(rect.left < desktop.left)		rect.left = desktop.left;
		if(rect.top < desktop.top)			rect.top = desktop.top;
		if(rect.bottom > desktop.bottom)	rect.bottom = desktop.bottom;
		if(rect.right > desktop.right)		rect.right = desktop.right;
		break;
	case DXW_DESKTOP_WORKAREA:
		dxw.GetMonitorWorkarea(&rect, TRUE);
		if(dxw.dwFlags2 & KEEPASPECTRATIO) dxw.FixWorkarea(&rect);
		break;
	case DXW_DESKTOP_FULL:
		dxw.GetMonitorWorkarea(&rect, FALSE);
		if(dxw.dwFlags2 & KEEPASPECTRATIO) dxw.FixWorkarea(&rect);
		break;
	case DXW_SET_COORDINATES:
	default:
		if(bAutoScale){
			MaxX = GetScreenWidth();
			MaxY = GetScreenHeight();
			if(dxw.dwFlags4 & BILINEAR2XFILTER) MaxX <<= 1; // double
			if(dxw.dwFlags4 & BILINEAR2XFILTER) MaxY <<= 1; // double
		}
		else{
			if((dxw.dwFlags2 & LOCKEDSIZE) || (dxw.dwFlags7 & ANCHORED)){
				iSizX = iSiz0X;
				iSizY = iSiz0Y;
			}
			MaxX = iSizX;
			MaxY = iSizY;
		}
		if (!MaxX) {
			MaxX = width;
			if(dxw.dwFlags4 & BILINEAR2XFILTER) MaxX <<= 1; // double
		}
		if (!MaxY) {
			MaxY = height;
			if(dxw.dwFlags4 & BILINEAR2XFILTER) MaxY <<= 1; // double
		}
		rect.left = iPosX;
		rect.top = iPosY; //v2.02.09
		rect.right = iPosX + MaxX;
		rect.bottom = iPosY + MaxY; //v2.02.09
		break;
	}
	OutTraceB("dxwCore::CalculateWindowPos: coord=%d client rect=(%d,%d)-(%d,%d)\n",
		Coordinates, rect.left, rect.top, rect.right, rect.bottom);

	if(hwnd){
		RECT UnmappedRect;
		UnmappedRect=rect;
		dwStyle=(*pGetWindowLong)(hwnd, GWL_STYLE);
		dwExStyle=(*pGetWindowLong)(hwnd, GWL_EXSTYLE);
		// BEWARE: from MSDN -  If the window is a child window, the return value is undefined. 
		hMenu = (dwStyle & WS_CHILD) ? NULL : GetMenu(hwnd);	
		AdjustWindowRectEx(&rect, dwStyle, (hMenu!=NULL), dwExStyle);
		if (hMenu) __try {CloseHandle(hMenu);} __except(EXCEPTION_EXECUTE_HANDLER){};
		switch(dxw.Coordinates){
		case DXW_DESKTOP_WORKAREA:
		case DXW_DESKTOP_FULL:
			// if there's a menu, reduce height to fit area
			if(rect.top != UnmappedRect.top){
				rect.bottom = rect.bottom - UnmappedRect.top + rect.top;
			}
			break;
		default:
			break;
		}

		// shift down-right so that the border is visible
		if(rect.left < dxw.VirtualDesktop.left){
			rect.right = dxw.VirtualDesktop.left - rect.left + rect.right;
			rect.left = dxw.VirtualDesktop.left;
		}
		if(rect.top < dxw.VirtualDesktop.top){
			rect.bottom = dxw.VirtualDesktop.top - rect.top + rect.bottom;
			rect.top = dxw.VirtualDesktop.top;
		}

		// shift up-left so that the windows doesn't exceed on the other side
		if(rect.right > dxw.VirtualDesktop.right){
			rect.left = dxw.VirtualDesktop.right - rect.right + rect.left;
			rect.right = dxw.VirtualDesktop.right;
		}
		if(rect.bottom > dxw.VirtualDesktop.bottom){
			rect.top = dxw.VirtualDesktop.bottom - rect.bottom + rect.top;
			rect.bottom = dxw.VirtualDesktop.bottom;
		}
	}

	OutTraceB("dxwCore::CalculateWindowPos: coord=%d window rect=(%d,%d)-(%d,%d)\n",
	Coordinates, rect.left, rect.top, rect.right, rect.bottom);

	// update the arguments for the window creation
	wp->x=rect.left;
	wp->y=rect.top;
	wp->cx=rect.right-rect.left;
	wp->cy=rect.bottom-rect.top;
}

void dxwCore::AutoScale()
{
	WINDOWPOS wp;
	if(!dxw.Windowize) return;
	CalculateWindowPos(hWnd, dwScreenWidth, dwScreenHeight, &wp);
	OutTrace("AutoScale: new pos=(%d,%d) size=(%dx%d)\n", wp.x, wp.y, wp.cx, wp.cy);
	if(!(*pSetWindowPos)(hWnd, 0, wp.x, wp.y, wp.cx, wp.cy, 0)){
		OutTraceE("AutoScale: SetWindowPos ERROR err=%d at %d\n", GetLastError(), __LINE__);
	}
	// need to ShowWindow(SW_SHOW) to make new coordinates active
	(*pShowWindow)(hWnd, SW_SHOW);
	UpdateDesktopCoordinates();
}