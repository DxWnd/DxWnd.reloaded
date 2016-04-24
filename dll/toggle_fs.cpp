#define _WIN32_WINNT 0x0600
#define WIN32_LEAN_AND_MEAN
#define _CRT_SECURE_NO_DEPRECATE 1

#include <windows.h>
#include <stdio.h>
#include <stdarg.h>
#include "dxwnd.h"
#include "dxwcore.hpp"
#include "dxhelper.h"

extern void RecoverScreenMode();
extern void RestoreDDrawSurfaces();
extern void RestoreD3DSurfaces(BOOL);

void dx_FullScreenToggle(HWND hwnd)
{
	static BOOL bFullScreen = FALSE;
	static RECT WinRect = {0, 0, 0, 0};
	static DWORD OldStyle, OldExtStyle;
	static DEVMODE oldDisplayMode;
	static DWORD OrigFlags;
	static char szDevice[32];

	// toggle ....
	if (bFullScreen){
		OutTraceDW("DxWnd: exiting fullscreen mode: style=%x extstyle=%x pos=(%d,%d)-(%d,%d)\n",
			OldStyle, OldExtStyle, WinRect.left, WinRect.top, WinRect.right, WinRect.bottom);
		int ChangeDisplayResult = (*pChangeDisplaySettingsExA)(szDevice, &oldDisplayMode, NULL, CDS_FULLSCREEN, NULL);
		if(ChangeDisplayResult != DISP_CHANGE_SUCCESSFUL){
			OutTraceE("ChangeDisplaySettingsEx ERROR: res=%d at %d\n", ChangeDisplayResult, __LINE__);
			MessageBox(NULL,"Error: Failed to recover display mode.", "Error", 0);
		}
		// MoveWindow doesn't recover the exact position!!!
		(*pSetWindowLong)(hwnd, GWL_STYLE, OldStyle);
		(*pSetWindowLong)(hwnd, GWL_EXSTYLE, OldExtStyle);
		(*pSetWindowPos)(hwnd, HWND_TOP, 
			WinRect.left, WinRect.top, (WinRect.right-WinRect.left), (WinRect.bottom-WinRect.top), 
			SWP_DRAWFRAME|SWP_FRAMECHANGED|SWP_SHOWWINDOW);
		memset(&WinRect, 0, sizeof(WinRect));
	}
	else {
		OutTraceDW("DxWnd: entering fullscreen mode\n");
		int BestIndex, iCost, iBestCost;
		(*pGetWindowRect)(hwnd, &WinRect);
		OldStyle = (*pGetWindowLongA)(hwnd, GWL_STYLE);
		OldExtStyle = (*pGetWindowLongA)(hwnd, GWL_EXSTYLE);
		(*pSetWindowLong)(hwnd, GWL_STYLE, WS_VISIBLE|WS_CLIPSIBLINGS|WS_OVERLAPPED);
		(*pSetWindowLong)(hwnd, GWL_EXSTYLE, 0);

		DEVMODE DisplayMode;
		HMONITOR hBestMonitor;
		hBestMonitor = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);
		MONITORINFOEX mi;
		memset(&mi, 0, sizeof(mi));
		mi.cbSize=sizeof(mi);
		GetMonitorInfo(hBestMonitor, &mi);
		OutTraceDW("Using monitor=\"%s\", rect=(%d,%d)-(%d,%d) type=%s\n",
			mi.szDevice, 
			mi.rcMonitor.left, mi.rcMonitor.top, mi.rcMonitor.right, mi.rcMonitor.bottom,
			(mi.dwFlags & MONITORINFOF_PRIMARY) ? "PRIMARY" : "SECONDARY");
		memset(&oldDisplayMode, 0, sizeof(DEVMODE));
		if(!(*pEnumDisplaySettings)(mi.szDevice, ENUM_CURRENT_SETTINGS, &oldDisplayMode)){
			MessageBox(NULL, "EnumDisplaySettings Failed ???", "Error!", 0);
		}
		iBestCost=1000000; // huge
		for (int i=0; ;i++){
			iCost=0;
			memset(&DisplayMode, 0, sizeof(DEVMODE));
			DisplayMode.dmSize = sizeof(DEVMODE);
			if(!(*pEnumDisplaySettings)(mi.szDevice, i, &DisplayMode))break; // no more modes
			if(DisplayMode.dmPelsWidth < dxw.GetScreenWidth()) continue; // bad: too narrow
			if(DisplayMode.dmPelsHeight < dxw.GetScreenHeight()) continue; // bad: too low
			if (DisplayMode.dmBitsPerPel != oldDisplayMode.dmBitsPerPel) continue; // bad: different color depth
			iCost = 
				(DisplayMode.dmPelsWidth - dxw.GetScreenWidth()) +
				(DisplayMode.dmPelsHeight - dxw.GetScreenHeight()) +
				(DisplayMode.dmDisplayFrequency == oldDisplayMode.dmDisplayFrequency) ? 1 : 0;
			if(iCost < iBestCost){
				iBestCost = iCost;
				BestIndex = i;
			}
			if(iBestCost == 0) break; // got the perfect one!
		}
		memset(&DisplayMode, 0, sizeof(DEVMODE));
		strncpy(szDevice, mi.szDevice, 32);
		(*pEnumDisplaySettings)(mi.szDevice, BestIndex, &DisplayMode);
		OutTraceDW("DxWnd: selected mode bpp=%d size=(%dx%d) freq=%d\n",
			DisplayMode.dmBitsPerPel, DisplayMode.dmPelsWidth, DisplayMode.dmPelsHeight, DisplayMode.dmDisplayFrequency);
		DisplayMode.dmFields = DM_BITSPERPEL|DM_PELSWIDTH|DM_PELSHEIGHT|DM_DISPLAYFLAGS|DM_DISPLAYFREQUENCY|DM_POSITION;
		int ChangeDisplayResult = (*pChangeDisplaySettingsExA)(mi.szDevice, &DisplayMode, NULL, CDS_FULLSCREEN, NULL);
		if(ChangeDisplayResult != DISP_CHANGE_SUCCESSFUL){
			OutTraceE("ChangeDisplaySettingsEx ERROR: res=%d at %d\n", ChangeDisplayResult, __LINE__);
			MessageBox(NULL,"Error: Failed to change display mode.", "Error", 0);
		} 
		// query again the NEW screen coordinates ....
		memset(&mi, 0, sizeof(mi));
		mi.cbSize=sizeof(mi);
		GetMonitorInfo(hBestMonitor, &mi);
		OutTraceDW("Updated monitor=\"%s\", rect=(%d,%d)-(%d,%d) type=%s\n",
			mi.szDevice, 
			mi.rcMonitor.left, mi.rcMonitor.top, mi.rcMonitor.right, mi.rcMonitor.bottom,
			(mi.dwFlags & MONITORINFOF_PRIMARY) ? "PRIMARY" : "SECONDARY");
		(*pMoveWindow)(hwnd, 
			mi.rcMonitor.left, mi.rcMonitor.top,	// x, y
			DisplayMode.dmPelsWidth,				// width
			DisplayMode.dmPelsHeight,				// height
			TRUE);
	}
	(*pUpdateWindow)(hwnd);
	RestoreDDrawSurfaces();
	RestoreD3DSurfaces(bFullScreen);
	bFullScreen = !bFullScreen; // switch toggle
}

void dx_DesktopToggle(HWND hwnd, BOOL bWorkArea)
{
	static BOOL bDesktopToggle = FALSE;
	static RECT WinRect = {0, 0, 0, 0};
	static DWORD OldStyle, OldExtStyle;

	if (bDesktopToggle){ 	// toggle ....
		OutTraceDW("DxWnd: exiting desktop mode: style=%x extstyle=%x pos=(%d,%d)-(%d,%d)\n",
			OldStyle, OldExtStyle, WinRect.left, WinRect.top, WinRect.right, WinRect.bottom);
		(*pSetWindowLong)(hwnd, GWL_STYLE, OldStyle);
		(*pSetWindowLong)(hwnd, GWL_EXSTYLE, OldExtStyle);
		// MoveWindow doesn't recover the exact position!!!
		(*pSetWindowPos)(hwnd, HWND_TOP, 
			WinRect.left, WinRect.top, (WinRect.right-WinRect.left), (WinRect.bottom-WinRect.top), 
			SWP_DRAWFRAME|SWP_FRAMECHANGED|SWP_SHOWWINDOW);
		memset(&WinRect, 0, sizeof(WinRect));
	}
	else {
		RECT DesktopRect;
		HWND DesktopWnd;
		HDC hClientDC;
		OutTraceDW("DxWnd: entering desktop mode\n");
		if((WinRect.left==0) && (WinRect.right==0) && (WinRect.top==0) && (WinRect.bottom==0)) (*pGetWindowRect)(hwnd, &WinRect);
		OldStyle = (*pGetWindowLongA)(hwnd, GWL_STYLE);
		OldExtStyle = (*pGetWindowLongA)(hwnd, GWL_EXSTYLE);
		(*pSetWindowLong)(hwnd, GWL_STYLE, WS_VISIBLE|WS_CLIPSIBLINGS|WS_OVERLAPPED);
		(*pSetWindowLong)(hwnd, GWL_EXSTYLE, 0);
		(*pUpdateWindow)(hwnd);
		DesktopWnd = (*pGetDesktopWindow)();
		hClientDC=(*pGDIGetDC)(hwnd);

		HMONITOR hBestMonitor;
		hBestMonitor = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);
		MONITORINFOEX mi;
		memset(&mi, 0, sizeof(mi));
		mi.cbSize=sizeof(mi);
		GetMonitorInfo(hBestMonitor, &mi);
		OutTraceDW("Using monitor=\"%s\", rect=(%d,%d)-(%d,%d) type=%s\n",
			mi.szDevice, 
			mi.rcMonitor.left, mi.rcMonitor.top, mi.rcMonitor.right, mi.rcMonitor.bottom,
			(mi.dwFlags & MONITORINFOF_PRIMARY) ? "PRIMARY" : "SECONDARY");

		DesktopRect = bWorkArea ? mi.rcWork : mi.rcMonitor;
		OutTraceDW("DxWnd: desktop=(%d,%d)-(%d,%d)\n");
		(*pSetWindowPos)(hwnd, HWND_TOP, 
			DesktopRect.left, DesktopRect.top, (DesktopRect.right-DesktopRect.left), (DesktopRect.bottom-DesktopRect.top), 
			SWP_DRAWFRAME|SWP_FRAMECHANGED|SWP_SHOWWINDOW);
		(*pGDIBitBlt)(hClientDC, DesktopRect.left, DesktopRect.top,  DesktopRect.right, DesktopRect.bottom, NULL, 0, 0, BLACKNESS);
	}
	bDesktopToggle = !bDesktopToggle; // switch toggle
	(*pUpdateWindow)(hwnd);
	dxw.ScreenRefresh();
	(*pInvalidateRect)(hwnd, NULL, FALSE); // force window update
}
