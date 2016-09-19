#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include "dxwnd.h"
#include "dxwcore.hpp"
#include "syslibs.h"
#include "dxhelper.h"

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
	POINT upleft = {0, 0};
	memset(&Screen, 0, sizeof(RECT));
	(*pGetClientRect)(hWnd, &Screen);
	(*pClientToScreen)(hWnd, &upleft);
	Screen.top += upleft.x;
	Screen.bottom += upleft.x;
	Screen.left += upleft.y;
	Screen.right += upleft.y;
	return Screen;
}

// GetFrameOffset: return the POINT coordinates of the upper-left corner of the main window frame

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
	RECT desktop;
	POINT upleft, win0, desk0, ret;
	ret.x = ret.y = 0;

	(*pGetClientRect)(hWnd,&desktop);
	if(!desktop.right || !desktop.bottom) return ret;

	upleft.x = upleft.y = 0;
	(*pClientToScreen)(hwnd, &upleft);
	win0 = upleft;
	upleft.x = upleft.y = 0;
	(*pClientToScreen)(hWnd, &upleft);
	desk0 = upleft;
	if (desktop.right) ret.x = (((win0.x - desk0.x) * dwScreenWidth) + (desktop.right >> 1)) / desktop.right;
	if (desktop.bottom) ret.y = (((win0.y - desk0.y) * dwScreenHeight) + (desktop.bottom >> 1)) / desktop.bottom;
	if(ret.x < 0) ret.x = 0;
	if(ret.y < 0) ret.y = 0;
	OutTraceB("ClientOffset: hwnd=%x offset=(%d,%d)\n", hwnd, ret.x, ret.y);
	return ret;
}

// GetWindowRect: returns the virtual coordinates of a window within the virtual desktop

RECT dxwCore::GetWindowRect(RECT win)
{
	RECT desktop;
	POINT desk0;
	desk0.x = desk0.y = 0;

	(*pGetClientRect)(hWnd, &desktop);
	(*pClientToScreen)(hWnd,&desk0);

	if(!desktop.right || !desktop.bottom) return win;

	win.left = (((win.left - desk0.x) * (LONG)dwScreenWidth) + (desktop.right >> 1)) / desktop.right;
	win.top = (((win.top - desk0.y) * (LONG)dwScreenHeight) + (desktop.bottom >> 1)) / desktop.bottom;
	win.right = (((win.right - desk0.x) * (LONG)dwScreenWidth) + (desktop.right >> 1)) / desktop.right;
	win.bottom = (((win.bottom - desk0.y) * (LONG)dwScreenHeight) + (desktop.bottom >> 1)) / desktop.bottom;

	return win;
}

// GetClientRect: returns the virtual client coordinates of a window

RECT dxwCore::GetClientRect(RECT win)
{
	RECT desktop;
	(*pGetClientRect)(hWnd, &desktop);

	if(!desktop.right || !desktop.bottom) return win;

	win.left = ((win.left * dwScreenWidth) + (desktop.right >> 1)) / desktop.right;
	win.top = ((win.top * dwScreenHeight) + (desktop.bottom >> 1)) / desktop.bottom;
	win.right = ((win.right * dwScreenWidth) + (desktop.right >> 1)) / desktop.right;
	win.bottom = ((win.bottom * dwScreenHeight) + (desktop.bottom >> 1)) / desktop.bottom;

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
	POINT UpLeft={0,0};
	RECT RetRect;
	RECT ClientRect;

	if (!(*pGetClientRect)(hWnd, &ClientRect)){
		OutTraceE("GetClientRect ERROR: err=%d hwnd=%x at %d\n", GetLastError(), hWnd, __LINE__);
		// v2.02.71: return a void area to prevent blitting to wrong area
		ClientRect.top=ClientRect.left=ClientRect.right=ClientRect.bottom=0;
		return ClientRect;
	}

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
	
	RetRect=ClientRect;

	if(lpRect){ 
		LONG Width, Height;
		Width = ClientRect.right;
		Height = ClientRect.bottom;
		RetRect.left = (lpRect->left * Width / dwScreenWidth);
		RetRect.right = (lpRect->right * Width / dwScreenWidth);
		RetRect.top = (lpRect->top * Height / dwScreenHeight);
		RetRect.bottom = (lpRect->bottom * Height / dwScreenHeight);
	}
	else{
		RetRect.left = ClientRect.left;
		RetRect.right = ClientRect.right;
		RetRect.top = ClientRect.top;
		RetRect.bottom = ClientRect.bottom;
	}

	if(!(*pClientToScreen)(hWnd, &UpLeft)){
		OutTraceE("ClientToScreen ERROR: err=%d hwnd=%x at %d\n", GetLastError(), hWnd, __LINE__);
	}
	if(!OffsetRect(&RetRect ,UpLeft.x, UpLeft.y)){
		OutTraceE("OffsetRect ERROR: err=%d hwnd=%x at %d\n", GetLastError(), hWnd, __LINE__);
	}
	return RetRect;
}

void dxwCore::MapWindow(LPRECT rect)
{
	RECT client;
	POINT upleft = {0,0};
	if(!(*pGetClientRect)(hWnd, &client)) {
		OutTraceE("dxwCore::MapClient ERROR: GetClientRect hwnd=%x err=%d\n", hWnd, GetLastError());
		return;
	}
	(*pClientToScreen)(hWnd, &upleft);
	rect->left= upleft.x + (((rect->left * client.right) + (dwScreenWidth >> 1)) / (int)dwScreenWidth);
	rect->top= upleft.y + (((rect->top * client.bottom) + (dwScreenHeight >> 1)) / (int)dwScreenHeight);
	rect->right= upleft.x + (((rect->right * client.right) + (dwScreenWidth >> 1)) / (int)dwScreenWidth);
	rect->bottom= upleft.y + (((rect->bottom * client.bottom) + (dwScreenHeight >> 1)) / (int)dwScreenHeight);
}

void dxwCore::MapWindow(int *nXDest, int *nYDest, int *nWDest, int *nHDest)
{
	RECT client;
	POINT upleft = {0,0};
	if(!(*pGetClientRect)(hWnd, &client)) {
		OutTraceE("dxwCore::MapClient ERROR: GetClientRect hwnd=%x err=%d\n", hWnd, GetLastError());
		return;
	}
	if(!(*pClientToScreen)(hWnd, &upleft)) {
		OutTraceE("dxwCore::MapClient ERROR: ClientToScreen hwnd=%x err=%d\n", hWnd, GetLastError());
		return;
	}
	*nXDest= upleft.x + (((*nXDest * client.right) + (dwScreenWidth >> 1)) / (int)dwScreenWidth);
	*nYDest= upleft.y + (((*nYDest * client.bottom) + (dwScreenHeight >> 1)) / (int)dwScreenHeight);
	*nWDest= ((*nWDest * client.right) + (dwScreenWidth >> 1)) / (int)dwScreenWidth;
	*nHDest= ((*nHDest * client.bottom) + (dwScreenHeight >> 1)) / (int)dwScreenHeight;
}

void dxwCore::MapWindow(LPPOINT lppoint) 
{
	RECT client;
	POINT upleft = {0,0};
	if(!(*pGetClientRect)(hWnd, &client)) {
		OutTraceE("dxwCore::MapClient ERROR: GetClientRect hwnd=%x err=%d\n", hWnd, GetLastError());
		return;
	}
	(*pClientToScreen)(hWnd, &upleft);
	lppoint->x = upleft.x + (((lppoint->x * client.right) + (dwScreenWidth >> 1)) / dwScreenWidth);
	lppoint->y = upleft.y + (((lppoint->y * client.bottom) + (dwScreenHeight >> 1)) / dwScreenHeight);
}

// MapClient, MapClientRect: transforms the client virtual coordinates of the virtual desktop into real ones

RECT dxwCore::MapClientRect(LPRECT lpRect)
{
	// same as MapClient, but taking in proper account aspect ratio & virtual desktop position
	RECT RetRect;
	RECT ClientRect;
	RECT NullRect={0, 0, 0, 0};

	if (!(*pGetClientRect)(hWnd, &ClientRect)) return NullRect;
	
	RetRect=ClientRect;

	if(lpRect){ 
		LONG Width, Height;
		Width = ClientRect.right;
		Height = ClientRect.bottom;
		RetRect.left = (lpRect->left * Width / dwScreenWidth);
		RetRect.right = (lpRect->right * Width / dwScreenWidth);
		RetRect.top = (lpRect->top * Height / dwScreenHeight);
		RetRect.bottom = (lpRect->bottom * Height / dwScreenHeight);
	}
	else{
		RetRect.left = ClientRect.left;
		RetRect.right = ClientRect.right;
		RetRect.top = ClientRect.top;
		RetRect.bottom = ClientRect.bottom;
	}

	return RetRect;
}

void dxwCore::MapClient(LPRECT rect)
{
	RECT client;
	int w, h;
	if(!(*pGetClientRect)(hWnd, &client)) {
		OutTraceE("dxwCore::MapClient ERROR: GetClientRect hwnd=%x err=%d\n", hWnd, GetLastError());
		return;
	}
	w = client.right ? client.right : iSizX;
	h = client.bottom ? client.bottom : iSizY;
	rect->left= ((rect->left * w)+(dwScreenWidth >> 1)) / (int)dwScreenWidth;
	rect->top= ((rect->top * h)+(dwScreenHeight >> 1)) / (int)dwScreenHeight;
	rect->right= ((rect->right * w)+(dwScreenWidth >> 1)) / (int)dwScreenWidth;
	rect->bottom= ((rect->bottom * h)+(dwScreenHeight >> 1)) / (int)dwScreenHeight;
}

void dxwCore::MapClient(int *nXDest, int *nYDest, int *nWDest, int *nHDest)
{
	RECT client;
	int w, h;
	if(!(*pGetClientRect)(hWnd, &client)) {
		OutTraceE("dxwCore::MapClient ERROR: GetClientRect hwnd=%x err=%d\n", hWnd, GetLastError());
		return;
	}
	w = client.right ? client.right : iSizX;
	h = client.bottom ? client.bottom : iSizY;
	*nXDest= ((*nXDest * w)+(dwScreenWidth >> 1)) / (int)dwScreenWidth;
	*nYDest= ((*nYDest * h)+(dwScreenHeight >> 1)) / (int)dwScreenHeight;
	*nWDest= ((*nWDest * w)+(dwScreenWidth >> 1)) / (int)dwScreenWidth;
	*nHDest= ((*nHDest * h)+(dwScreenHeight >> 1)) / (int)dwScreenHeight;
}

void dxwCore::MapClient(LPPOINT lppoint)
{
	RECT client;
	int w, h;
	if(!(*pGetClientRect)(hWnd, &client)) {
		OutTraceE("dxwCore::MapClient ERROR: GetClientRect hwnd=%x err=%d\n", hWnd, GetLastError());
		return;
	}
	w = client.right ? client.right : iSizX;
	h = client.bottom ? client.bottom : iSizY;
	lppoint->x= ((lppoint->x * w)+(dwScreenWidth >> 1)) / (int)dwScreenWidth;
	lppoint->y= ((lppoint->y * h)+(dwScreenHeight >> 1)) / (int)dwScreenHeight;
}

void dxwCore::MapClient(int *nXDest, int *nYDest)
{
	RECT client;
	int w, h;
	if(!(*pGetClientRect)(hWnd, &client)) {
		OutTraceE("dxwCore::MapClient ERROR: GetClientRect hwnd=%x err=%d\n", hWnd, GetLastError());
		return;
	}
	w = client.right ? client.right : iSizX;
	h = client.bottom ? client.bottom : iSizY;
	*nXDest= ((*nXDest * w)+(dwScreenWidth >> 1)) / (int)dwScreenWidth;
	*nYDest= ((*nYDest * h)+(dwScreenHeight >> 1)) / (int)dwScreenHeight;
}

// UnmapWindow: transforms the window real coordinates of the real desktop into virtual ones

void dxwCore::UnmapWindow(LPRECT rect)
{
	RECT client;
	POINT upleft = {0,0};
	if(!(*pGetClientRect)(hWnd, &client)) {
		OutTraceE("dxwCore::MapClient ERROR: GetClientRect hwnd=%x err=%d\n", hWnd, GetLastError());
		return;
	}
	(*pClientToScreen)(hWnd, &upleft);
	if((client.right == 0) || (client.bottom == 0)) return;
	rect->left= ((rect->left  - upleft.x) * (int)dwScreenWidth) / client.right;
	rect->top= ((rect->top  - upleft.y) * (int)dwScreenHeight) / client.bottom;
	rect->right= ((rect->right  - upleft.x) * (int)dwScreenWidth) / client.right;
	rect->bottom= ((rect->bottom  - upleft.y) * (int)dwScreenHeight) / client.bottom;
}

void dxwCore::UnmapWindow(LPPOINT point)
{
	RECT client;
	POINT upleft = {0,0};
	if(!(*pGetClientRect)(hWnd, &client)) return;
	(*pClientToScreen)(hWnd, &upleft);
	if((client.right == 0) || (client.bottom == 0)) return;
	point->x= ((point->x  - upleft.x) * (int)dwScreenWidth) / client.right;
	point->y= ((point->y  - upleft.y) * (int)dwScreenHeight) / client.bottom;
}

// UnmapClient: transforms the client real coordinates of the real desktop into virtual ones

void dxwCore::UnmapClient(LPPOINT lppoint)
{
	RECT client;
	int w, h;
	if(!(*pGetClientRect)(hWnd, &client)) {
		OutTraceE("dxwCore::MapClient ERROR: GetClientRect hwnd=%x err=%d\n", hWnd, GetLastError());
		return;
	}
	w = client.right ? client.right : iSizX;
	h = client.bottom ? client.bottom : iSizY;
	if(w) lppoint->x = ((lppoint->x * (int)dwScreenWidth) + (w >> 1)) / w;
	if(h) lppoint->y = ((lppoint->y * (int)dwScreenHeight) + (h >> 1)) / h;
}

void dxwCore::UnmapClient(int *nXDest, int *nYDest)
{
	RECT client;
	int w, h;
	if(!(*pGetClientRect)(hWnd, &client)) {
		OutTraceE("dxwCore::MapClient ERROR: GetClientRect hwnd=%x err=%d\n", hWnd, GetLastError());
		return;
	}
	w = client.right ? client.right : iSizX;
	h = client.bottom ? client.bottom : iSizY;
	if(w) *nXDest = ((*nXDest * (int)dwScreenWidth) + (w >> 1)) / w;
	if(h) *nYDest = ((*nYDest * (int)dwScreenHeight) + (h >> 1)) / h;
}

void dxwCore::UnmapClient(int *nXDest, int *nYDest, int *nWidth, int *nHeight)
{
	RECT client;
	int w, h;
	if(!(*pGetClientRect)(hWnd, &client)) {
		OutTraceE("dxwCore::MapClient ERROR: GetClientRect hwnd=%x err=%d\n", hWnd, GetLastError());
		return;
	}
	w = client.right ? client.right : iSizX;
	h = client.bottom ? client.bottom : iSizY;
	if(w) *nXDest = ((*nXDest * (int)dwScreenWidth) + (w >> 1)) / w;
	if(h) *nYDest = ((*nYDest * (int)dwScreenHeight) + (h >> 1)) / h;
	if(w) *nWidth = ((*nWidth * (int)dwScreenWidth) + (w >> 1)) / w;
	if(h) *nHeight = ((*nHeight * (int)dwScreenHeight) + (h >> 1)) / h;
}

void dxwCore::UnmapClient(LPRECT lpRect)
{
	RECT client;
	int w, h;
	if(!(*pGetClientRect)(hWnd, &client)) {
		OutTraceE("dxwCore::MapClient ERROR: GetClientRect hwnd=%x err=%d\n", hWnd, GetLastError());
		return;
	}
	w = client.right ? client.right : iSizX;
	h = client.bottom ? client.bottom : iSizY;
	if(w) {
		lpRect->left = ((lpRect->left * (int)dwScreenWidth) + (w >> 1)) / w;
		lpRect->right = ((lpRect->right * (int)dwScreenWidth) + (w >> 1)) / w;
	}
	if(h) {
		lpRect->top = ((lpRect->top * (int)dwScreenHeight) + (h >> 1)) / h;
		lpRect->bottom = ((lpRect->bottom * (int)dwScreenHeight) + (h >> 1)) / h;
	}
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

}

// v.2.1.80: unified positioning logic into CalculateWindowPos routine
// now taking in account for window menus (see "Alien Cabal")

void dxwCore::CalculateWindowPos(HWND hwnd, DWORD width, DWORD height, LPWINDOWPOS wp)
{
	RECT rect, desktop;
	DWORD dwStyle, dwExStyle;
	int MaxX, MaxY;
	HMENU hMenu;

	switch(dxw.Coordinates){
	case DXW_DESKTOP_CENTER:
		MaxX = dxw.iSizX;
		MaxY = dxw.iSizY;
		if (!MaxX) {
			MaxX = width;
			if(dxw.dwFlags4 & BILINEAR2XFILTER) MaxX <<= 1; // double
		}
		if (!MaxY) {
			MaxY = height;
			if(dxw.dwFlags4 & BILINEAR2XFILTER) MaxY <<= 1; // double
		}
		dxw.GetMonitorWorkarea(&desktop, TRUE);
		rect.left =  (desktop.right - desktop.left - MaxX) / 2;
		rect.top = (desktop.bottom - desktop.top - MaxY) / 2;
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
		rect.left = dxw.iPosX;
		rect.top = dxw.iPosY; //v2.02.09
		MaxX = dxw.iSizX;
		MaxY = dxw.iSizY;
		if (!MaxX) MaxX = width;
		if (!MaxY) MaxY = height;
		rect.right = dxw.iPosX + MaxX;
		rect.bottom = dxw.iPosY + MaxY; //v2.02.09
		break;
	}

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

	// update the arguments for the window creation
	wp->x=rect.left;
	wp->y=rect.top;
	wp->cx=rect.right-rect.left;
	wp->cy=rect.bottom-rect.top;
}
