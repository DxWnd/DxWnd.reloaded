#define _WIN32_WINNT 0x0600
#define WIN32_LEAN_AND_MEAN
#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include "dxwnd.h"
#include "dxwcore.hpp"
#include "syslibs.h"
#include "dxhook.h"
#include "hddraw.h"
#include "dxhelper.h"

static HookEntry_Type Hooks[]={
	{"UpdateWindow", (FARPROC)NULL, (FARPROC *)&pUpdateWindow, (FARPROC)extUpdateWindow},
	{"GetWindowPlacement", (FARPROC)NULL, (FARPROC *)&pGetWindowPlacement, (FARPROC)extGetWindowPlacement},
	{"ChangeDisplaySettingsA", (FARPROC)ChangeDisplaySettingsA, (FARPROC *)&pChangeDisplaySettings, (FARPROC)extChangeDisplaySettings},
	{"ChangeDisplaySettingsExA", (FARPROC)ChangeDisplaySettingsA, (FARPROC *)&pChangeDisplaySettingsEx, (FARPROC)extChangeDisplaySettingsEx},
	{"BeginPaint", (FARPROC)BeginPaint, (FARPROC *)&pBeginPaint, (FARPROC)extBeginPaint},
	{"EndPaint", (FARPROC)EndPaint, (FARPROC *)&pEndPaint, (FARPROC)extEndPaint},
	{"ShowCursor", (FARPROC)ShowCursor, (FARPROC *)&pShowCursor, (FARPROC)extShowCursor},
	{"CreateDialogIndirectParamA", (FARPROC)CreateDialogIndirectParamA, (FARPROC *)&pCreateDialogIndirectParam, (FARPROC)extCreateDialogIndirectParam},
	{"CreateDialogParamA", (FARPROC)CreateDialogParamA, (FARPROC *)&pCreateDialogParam, (FARPROC)extCreateDialogParam},
	{"MoveWindow", (FARPROC)MoveWindow, (FARPROC *)&pMoveWindow, (FARPROC)extMoveWindow},
	{"EnumDisplaySettingsA", (FARPROC)EnumDisplaySettingsA, (FARPROC *)&pEnumDisplaySettings, (FARPROC)extEnumDisplaySettings},
	{"GetClipCursor", (FARPROC)GetClipCursor, (FARPROC*)&pGetClipCursor, (FARPROC)extGetClipCursor},
	{"ClipCursor", (FARPROC)ClipCursor, (FARPROC *)&pClipCursor, (FARPROC)extClipCursor},
	{"FillRect", (FARPROC)NULL, (FARPROC *)&pFillRect, (FARPROC)extFillRect},
	{"FrameRect", (FARPROC)NULL, (FARPROC *)&pFrameRect, (FARPROC)extFrameRect},
	{"DefWindowProcA", (FARPROC)DefWindowProcA, (FARPROC *)&pDefWindowProc, (FARPROC)extDefWindowProc},
	{"CreateWindowExA", (FARPROC)CreateWindowExA, (FARPROC *)&pCreateWindowExA, (FARPROC)extCreateWindowExA},
	{"CreateWindowExW", (FARPROC)CreateWindowExW, (FARPROC *)&pCreateWindowExW, (FARPROC)extCreateWindowExW},
	{"RegisterClassExA", (FARPROC)RegisterClassExA, (FARPROC *)&pRegisterClassExA, (FARPROC)extRegisterClassExA},
	{"GetSystemMetrics", (FARPROC)GetSystemMetrics, (FARPROC *)&pGetSystemMetrics, (FARPROC)extGetSystemMetrics},
	{"GetDesktopWindow", (FARPROC)GetDesktopWindow, (FARPROC *)&pGetDesktopWindow, (FARPROC)extGetDesktopWindow},
	{"TabbedTextOutA", (FARPROC)TabbedTextOutA, (FARPROC *)&pTabbedTextOutA, (FARPROC)extTabbedTextOutA},
	{"CloseWindow", (FARPROC)NULL, (FARPROC *)&pCloseWindow, (FARPROC)extCloseWindow},
	{"DestroyWindow", (FARPROC)NULL, (FARPROC *)&pDestroyWindow, (FARPROC)extDestroyWindow},
	{"SetSysColors", (FARPROC)NULL, (FARPROC *)&pSetSysColors, (FARPROC)extSetSysColors},
	{"SetCapture", (FARPROC)NULL, (FARPROC *)&pSetCapture, (FARPROC)extSetCapture},
	{0, NULL, 0, 0} // terminator
};

static HookEntry_Type EmulateHooks[]={
	{"GetDC", (FARPROC)GetDC, (FARPROC *)&pGDIGetDC, (FARPROC)extGDIGetDC},
	//{"GetWindowDC", (FARPROC)GetWindowDC, (FARPROC *)&pGDIGetWindowDC, (FARPROC)extGDIGetDC}, // not ready yet
	{"ReleaseDC", (FARPROC)ReleaseDC, (FARPROC *)&pGDIReleaseDC, (FARPROC)extGDIReleaseDC},
	{0, NULL, 0, 0} // terminator
};

static HookEntry_Type DDHooks[]={
	{"GetDC", (FARPROC)GetDC, (FARPROC *)&pGDIGetDC, (FARPROC)extDDGetDC},
	{"GetWindowDC", (FARPROC)GetWindowDC, (FARPROC *)&pGDIGetWindowDC, (FARPROC)extDDGetDC},
	{"ReleaseDC", (FARPROC)ReleaseDC, (FARPROC *)&pGDIReleaseDC, (FARPROC)extDDReleaseDC},
	{"InvalidateRect", (FARPROC)InvalidateRect, (FARPROC *)&pInvalidateRect, (FARPROC)extDDInvalidateRect},
	{0, NULL, 0, 0} // terminator
};

static HookEntry_Type GDIHooks[]={
	{"GetDC", (FARPROC)GetDC, (FARPROC *)&pGDIGetDC, (FARPROC)extGDIGetDC},
	{"GetDCEx", (FARPROC)NULL, (FARPROC *)&pGDIGetDCEx, (FARPROC)extGDIGetDCEx},
	{"GetWindowDC", (FARPROC)GetWindowDC, (FARPROC *)&pGDIGetWindowDC, (FARPROC)extGDIGetDC},
	{"ReleaseDC", (FARPROC)ReleaseDC, (FARPROC *)&pGDIReleaseDC, (FARPROC)extGDIReleaseDC},
	{"InvalidateRect", (FARPROC)InvalidateRect, (FARPROC *)&pInvalidateRect, (FARPROC)extInvalidateRect},
	{0, NULL, 0, 0} // terminator
};

static HookEntry_Type RemapHooks[]={
	{"ScreenToClient", (FARPROC)ScreenToClient, (FARPROC *)&pScreenToClient, (FARPROC)extScreenToClient},
	{"ClientToScreen", (FARPROC)ClientToScreen, (FARPROC *)&pClientToScreen, (FARPROC)extClientToScreen},
	{"GetClientRect", (FARPROC)GetClientRect, (FARPROC *)&pGetClientRect, (FARPROC)extGetClientRect},
	{"GetWindowRect", (FARPROC)GetWindowRect, (FARPROC *)&pGetWindowRect, (FARPROC)extGetWindowRect},
	{"MapWindowPoints", (FARPROC)MapWindowPoints, (FARPROC *)&pMapWindowPoints, (FARPROC)extMapWindowPoints},
	{0, NULL, 0, 0} // terminator
};

static HookEntry_Type MessageHooks[]={
	//{"PeekMessageA", (FARPROC)PeekMessageA, (FARPROC *)&pPeekMessage, (FARPROC)extPeekMessage},
	//{"GetMessageA", (FARPROC)GetMessageA, (FARPROC *)&pGetMessage, (FARPROC)extGetMessage},
	{0, NULL, 0, 0} // terminator
};

static HookEntry_Type PeekAllHooks[]={
	{"PeekMessageA", (FARPROC)NULL, (FARPROC *)&pPeekMessage, (FARPROC)extPeekAnyMessage},
	{"PeekMessageW", (FARPROC)NULL, (FARPROC *)&pPeekMessage, (FARPROC)extPeekAnyMessage},
	{0, NULL, 0, 0} // terminator
};

static HookEntry_Type MouseHooks[]={
	{"GetCursorPos", (FARPROC)GetCursorPos, (FARPROC *)&pGetCursorPos, (FARPROC)extGetCursorPos},
	{"SetCursor", (FARPROC)SetCursor, (FARPROC *)&pSetCursor, (FARPROC)extSetCursor},
	{"SendMessageA", (FARPROC)SendMessageA, (FARPROC *)&pSendMessageA, (FARPROC)extSendMessage}, // ???
	{"SendMessageW", (FARPROC)SendMessageW, (FARPROC *)&pSendMessageW, (FARPROC)extSendMessage}, // ???
	//{"SetPhysicalCursorPos", NULL, (FARPROC *)&pSetCursor, (FARPROC)extSetCursor}, // ???
	{0, NULL, 0, 0} // terminator
};

static HookEntry_Type WinHooks[]={
	{"ShowWindow", (FARPROC)ShowWindow, (FARPROC *)&pShowWindow, (FARPROC)extShowWindow},
	{"SetWindowLongA", (FARPROC)SetWindowLongA, (FARPROC *)&pSetWindowLong, (FARPROC)extSetWindowLong},
	{"GetWindowLongA", (FARPROC)GetWindowLongA, (FARPROC *)&pGetWindowLong, (FARPROC)extGetWindowLong}, 
	{"SetWindowPos", (FARPROC)SetWindowPos, (FARPROC *)&pSetWindowPos, (FARPROC)extSetWindowPos},
	{"DeferWindowPos", (FARPROC)DeferWindowPos, (FARPROC *)&pGDIDeferWindowPos, (FARPROC)extDeferWindowPos},
	{"CallWindowProcA", (FARPROC)CallWindowProcA, (FARPROC *)&pCallWindowProc, (FARPROC)extCallWindowProc},
	{0, NULL, 0, 0} // terminator
};

static HookEntry_Type MouseHooks2[]={
	{"SetCursorPos", (FARPROC)SetCursorPos, (FARPROC *)&pSetCursorPos, (FARPROC)extSetCursorPos},
	{0, NULL, 0, 0} // terminator
};

FARPROC Remap_user32_ProcAddress(LPCSTR proc, HMODULE hModule)
{
	FARPROC addr;
	if (addr=RemapLibrary(proc, hModule, Hooks)) return addr;
	if ((dxw.dwFlags3 & EMULATEDC) || (dxw.dwFlags2 & HOOKGDI) || (dxw.dwFlags1 & MAPGDITOPRIMARY))
		if (addr=RemapLibrary(proc, hModule, EmulateHooks)) return addr;
	if (addr=RemapLibrary(proc, hModule, (dxw.dwFlags1 & MAPGDITOPRIMARY) ? DDHooks : GDIHooks)) return addr;
	if (dxw.dwFlags1 & CLIENTREMAPPING) 
		if (addr=RemapLibrary(proc, hModule, RemapHooks)) return addr;
	// commented out since message processing was given to SetWindowHook callback
	// if (dxw.dwFlags1 & MESSAGEPROC) 
	//	if (addr=RemapLibrary(proc, hModule, MessageHooks)) return addr;
	if(dxw.dwFlags1 & MODIFYMOUSE)
		if (addr=RemapLibrary(proc, hModule, MouseHooks)) return addr;
	if (dxw.dwFlags1 & (PREVENTMAXIMIZE|FIXWINFRAME|LOCKWINPOS|LOCKWINSTYLE))
		if (addr=RemapLibrary(proc, hModule, WinHooks)) return addr;
	if((dxw.dwFlags1 & (MODIFYMOUSE|SLOWDOWN|KEEPCURSORWITHIN)) || (dxw.dwFlags2 & KEEPCURSORFIXED))
		if (addr=RemapLibrary(proc, hModule, MouseHooks2)) return addr;
	if(dxw.dwFlags3 & PEEKALLMESSAGES)
		if (addr=RemapLibrary(proc, hModule, PeekAllHooks)) return addr;
	return NULL;
}

static char *libname = "user32.dll";

void HookUser32(HMODULE hModule)
{
	HookLibrary(hModule, Hooks, libname);
	if ((dxw.dwFlags3 & EMULATEDC) || (dxw.dwFlags2 & HOOKGDI) || (dxw.dwFlags1 & MAPGDITOPRIMARY))
		HookLibrary(hModule, EmulateHooks, libname);
	HookLibrary(hModule, (dxw.dwFlags1 & MAPGDITOPRIMARY) ? DDHooks : GDIHooks, libname);
	if (dxw.dwFlags1 & CLIENTREMAPPING) HookLibrary(hModule, RemapHooks, libname);
	//if (dxw.dwFlags1 & MESSAGEPROC) HookLibrary(hModule, MessageHooks, libname);
	if(dxw.dwFlags1 & MODIFYMOUSE)HookLibrary(hModule, MouseHooks, libname);
	if (dxw.dwFlags1 & (PREVENTMAXIMIZE|FIXWINFRAME|LOCKWINPOS|LOCKWINSTYLE))HookLibrary(hModule, WinHooks, libname);
	if((dxw.dwFlags1 & (MODIFYMOUSE|SLOWDOWN|KEEPCURSORWITHIN)) || (dxw.dwFlags2 & KEEPCURSORFIXED)) HookLibrary(hModule, MouseHooks2, libname);
	if(dxw.dwFlags3 & PEEKALLMESSAGES) HookLibrary(hModule, PeekAllHooks, libname);
	return;
}

void HookUser32Init()
{
	HookLibInit(Hooks);
	HookLibInit(DDHooks);
	HookLibInit(RemapHooks);
	HookLibInit(MessageHooks);
	HookLibInit(MouseHooks);
	HookLibInit(WinHooks);
	HookLibInit(MouseHooks2);
}

// --------------------------------------------------------------------------
//
// globals, externs, static functions...
//
// --------------------------------------------------------------------------

// PrimHDC: DC handle of the selected DirectDraw primary surface. NULL when invalid.
HDC PrimHDC=NULL;

BOOL isWithinDialog=FALSE;
LPRECT lpClipRegion=NULL;
RECT ClipRegion;
int LastCurPosX, LastCurPosY;

extern GetDC_Type pGetDC;
extern ReleaseDC_Type pReleaseDC;
extern DEVMODE *pSetDevMode;
extern void FixWindowFrame(HWND);
extern LRESULT CALLBACK extChildWindowProc(HWND, UINT, WPARAM, LPARAM);
extern HRESULT WINAPI sBlt(char *, LPDIRECTDRAWSURFACE, LPRECT, LPDIRECTDRAWSURFACE, LPRECT, DWORD, LPDDBLTFX, BOOL);

extern INT_PTR CALLBACK extDialogWindowProc(HWND, UINT, WPARAM, LPARAM);

LONG WINAPI MyChangeDisplaySettings(char *fname, DEVMODE *lpDevMode, DWORD dwflags)
{
	HRESULT res;

	// v2.02.32: reset the emulated DC used in EMULATEDC mode
	dxw.ResetEmulatedDC();

	// save desired settings first v.2.1.89
	// v2.1.95 protect when lpDevMode is null (closing game... Jedi Outcast)
	// v2.2.23 consider new width/height only when dmFields flags are set.
	if(lpDevMode && (lpDevMode->dmFields & (DM_PELSWIDTH | DM_PELSHEIGHT))){
		RECT client;
		dxw.SetScreenSize(lpDevMode->dmPelsWidth, lpDevMode->dmPelsHeight);
		// v2.02.31: when main win is bigger that expected resolution, you're in windowed fullscreen mode
		//(*pGetClientRect)((dxw.dwFlags1 & FIXPARENTWIN) ? dxw.hParentWnd : dxw.GethWnd(), &client);
		(*pGetClientRect)(dxw.GethWnd(), &client);
		OutTraceD("%s: current hWnd=%x size=(%d,%d)\n", fname, dxw.GethWnd(), client.right, client.bottom);
		if((client.right>=(LONG)lpDevMode->dmPelsWidth) && (client.bottom>=(LONG)lpDevMode->dmPelsHeight)) {
			OutTraceD("%s: entering FULLSCREEN mode\n", fname);
			dxw.SetFullScreen(TRUE);
		}
	}

	if ((dwflags==0 || dwflags==CDS_FULLSCREEN) && lpDevMode){
		if (dxw.dwFlags1 & EMULATESURFACE || !(lpDevMode->dmFields & DM_BITSPERPEL)){
			OutTraceD("%s: BYPASS res=DISP_CHANGE_SUCCESSFUL\n", fname);
			return DISP_CHANGE_SUCCESSFUL;
		}
		else{
			DEVMODE NewMode;
			if(dwflags==CDS_FULLSCREEN) dwflags=0; // no FULLSCREEN
			EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &NewMode);
			OutTraceD("ChangeDisplaySettings: CURRENT wxh=(%dx%d) BitsPerPel=%d -> %d\n", 
				NewMode.dmPelsWidth, NewMode.dmPelsHeight, NewMode.dmBitsPerPel, 
				lpDevMode->dmBitsPerPel);
			NewMode.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;
			NewMode.dmBitsPerPel = lpDevMode->dmBitsPerPel;
			res=(*pChangeDisplaySettings)(&NewMode, 0);
			if(res) OutTraceE("ChangeDisplaySettings: ERROR err=%d at %d\n", GetLastError(), __LINE__);
			return res;
		}
	}
	else
		return (*pChangeDisplaySettings)(lpDevMode, dwflags);
}

void dxwFixWindowPos(char *ApiName, HWND hwnd, LPARAM lParam)
{
	LPWINDOWPOS wp;
	int MaxX, MaxY;
	wp = (LPWINDOWPOS)lParam;
	MaxX = dxw.iSizX;
	MaxY = dxw.iSizY;
	if (!MaxX) MaxX = dxw.GetScreenWidth();
	if (!MaxY) MaxY = dxw.GetScreenHeight();
	static int iLastCX, iLastCY;
	static int BorderX=-1;
	static int BorderY=-1;
	int cx, cy;

	OutTraceD("%s: GOT hwnd=%x pos=(%d,%d) dim=(%d,%d) Flags=%x(%s)\n", 
		ApiName, hwnd, wp->x, wp->y, wp->cx, wp->cy, wp->flags, ExplainWPFlags(wp->flags));

	if (dxw.dwFlags1 & PREVENTMAXIMIZE){
		int UpdFlag = 0;

		if(wp->cx>MaxX) { wp->cx=MaxX; UpdFlag=1; }
		if(wp->cy>MaxY) { wp->cy=MaxY; UpdFlag=1; }
		if (UpdFlag) 
			OutTraceD("%s: SET max dim=(%d,%d)\n", ApiName, wp->cx, wp->cy);
	}

	if ((wp->flags & (SWP_NOMOVE|SWP_NOSIZE))==(SWP_NOMOVE|SWP_NOSIZE)) return; //v2.02.13

	if ((dxw.dwFlags1 & LOCKWINPOS) && dxw.IsFullScreen() && (hwnd==dxw.GethWnd())){ 
		extern void CalculateWindowPos(HWND, DWORD, DWORD, LPWINDOWPOS);
		CalculateWindowPos(hwnd, MaxX, MaxY, wp);
		OutTraceD("%s: LOCK pos=(%d,%d) dim=(%d,%d)\n", ApiName, wp->x, wp->y, wp->cx, wp->cy);
	}

	if ((dxw.dwFlags2 & KEEPASPECTRATIO) && dxw.IsFullScreen() && (hwnd==dxw.GethWnd())){ 
		// note: while keeping aspect ration, resizing from one corner doesn't tell
		// which coordinate is prevalent to the other. We made an arbitrary choice.
		// note: v2.1.93: compensation must refer to the client area, not the wp
		// window dimensions that include the window borders.
		if(BorderX==-1){
			RECT client, full;
			(*pGetClientRect)(hwnd, &client);
			(*pGetWindowRect)(hwnd, &full);
			BorderX= full.right - full.left - client.right;
			BorderY= full.bottom - full.top - client.bottom;
			OutTraceD("%s: KEEPASPECTRATIO window borders=(%d,%d)\n", ApiName, BorderX, BorderY);
		}
		extern LRESULT LastCursorPos;
		switch (LastCursorPos){
			case HTBOTTOM:
			case HTTOP:
			case HTBOTTOMLEFT:
			case HTBOTTOMRIGHT:
			case HTTOPLEFT:
			case HTTOPRIGHT:
				cx = BorderX + ((wp->cy - BorderY) * dxw.GetScreenWidth()) / dxw.GetScreenHeight();
				if(cx!=wp->cx){
					OutTraceD("%s: KEEPASPECTRATIO adjusted cx=%d->%d\n", ApiName, wp->cx, cx);
					wp->cx = cx;
				}
				break;
			case HTLEFT:
			case HTRIGHT:
				cy = BorderY + ((wp->cx - BorderX) * dxw.GetScreenHeight()) / dxw.GetScreenWidth();
				if(cy!=wp->cy){
					OutTraceD("%s: KEEPASPECTRATIO adjusted cy=%d->%d\n", ApiName, wp->cy, cy);
					wp->cy = cy;
				}
				break;
		}
	}

	iLastCX= wp->cx;
	iLastCY= wp->cy;
}

void dxwFixMinMaxInfo(char *ApiName, HWND hwnd, LPARAM lParam)
{
	if (dxw.dwFlags1 & PREVENTMAXIMIZE){
		LPMINMAXINFO lpmmi;
		lpmmi=(LPMINMAXINFO)lParam;
		OutTraceD("%s: GOT MaxPosition=(%d,%d) MaxSize=(%d,%d)\n", ApiName, 
			lpmmi->ptMaxPosition.x, lpmmi->ptMaxPosition.y, lpmmi->ptMaxSize.x, lpmmi->ptMaxSize.y);
		lpmmi->ptMaxPosition.x=0;
		lpmmi->ptMaxPosition.y=0;
		if(pSetDevMode){
			lpmmi->ptMaxSize.x = pSetDevMode->dmPelsWidth;
			lpmmi->ptMaxSize.y = pSetDevMode->dmPelsHeight;
		}
		else{
			lpmmi->ptMaxSize.x = dxw.GetScreenWidth();
			lpmmi->ptMaxSize.y = dxw.GetScreenHeight();
		}
		OutTraceD("%s: SET PREVENTMAXIMIZE MaxPosition=(%d,%d) MaxSize=(%d,%d)\n", ApiName, 
			lpmmi->ptMaxPosition.x, lpmmi->ptMaxPosition.y, lpmmi->ptMaxSize.x, lpmmi->ptMaxSize.y);
	}
	// v2.1.75: added logic to fix win coordinates to selected ones. 
	// fixes the problem with "Achtung Spitfire", that can't be managed through PREVENTMAXIMIZE flag.
	if (dxw.dwFlags1 & LOCKWINPOS){
		LPMINMAXINFO lpmmi;
		lpmmi=(LPMINMAXINFO)lParam;
		OutTraceD("%s: GOT MaxPosition=(%d,%d) MaxSize=(%d,%d)\n", ApiName, 
			lpmmi->ptMaxPosition.x, lpmmi->ptMaxPosition.y, lpmmi->ptMaxSize.x, lpmmi->ptMaxSize.y);
		lpmmi->ptMaxPosition.x=dxw.iPosX;
		lpmmi->ptMaxPosition.y=dxw.iPosY;
		lpmmi->ptMaxSize.x = dxw.iSizX ? dxw.iSizX : dxw.GetScreenWidth();
		lpmmi->ptMaxSize.y = dxw.iSizY ? dxw.iSizY : dxw.GetScreenHeight();
		OutTraceD("%s: SET LOCKWINPOS MaxPosition=(%d,%d) MaxSize=(%d,%d)\n", ApiName, 
			lpmmi->ptMaxPosition.x, lpmmi->ptMaxPosition.y, lpmmi->ptMaxSize.x, lpmmi->ptMaxSize.y);
	}
}

static LRESULT WINAPI FixWindowProc(char *ApiName, HWND hwnd, UINT Msg, WPARAM wParam, LPARAM *lpParam)
{
	LPARAM lParam;

	lParam=*lpParam;
	OutTraceW("%s: hwnd=%x msg=[0x%x]%s(%x,%x)\n",
		ApiName, hwnd, Msg, ExplainWinMessage(Msg), wParam, lParam);

	switch(Msg){
	// attempt to fix Sleepwalker
	//case WM_NCCALCSIZE:
	//	if (dxw.dwFlags1 & PREVENTMAXIMIZE)
	//		return 0;
	//	break;
	case WM_ERASEBKGND:
		OutTraceD("%s: prevent erase background\n", ApiName);
		return 1; // 1=erased
		break; // useless
	case WM_GETMINMAXINFO:
		dxwFixMinMaxInfo(ApiName, hwnd, lParam);
		break;
	case WM_WINDOWPOSCHANGING:
	case WM_WINDOWPOSCHANGED:
		dxwFixWindowPos(ApiName, hwnd, lParam);
		break;
	case WM_STYLECHANGING:
	case WM_STYLECHANGED:
		dxw.FixStyle(ApiName, hwnd, wParam, lParam);
		break;
	case WM_DISPLAYCHANGE:
		// too late? to be deleted....
		if ((dxw.dwFlags1 & LOCKWINPOS) && dxw.IsFullScreen()) return 0;
		if (dxw.dwFlags1 & PREVENTMAXIMIZE){
			OutTraceD("%s: WM_DISPLAYCHANGE depth=%d size=(%d,%d)\n",
				ApiName, wParam, HIWORD(lParam), LOWORD(lParam));
			return 0;
		}
		break;
	case WM_SIZE:
		if ((dxw.dwFlags1 & LOCKWINPOS) && dxw.IsFullScreen()) return 0;
		if (dxw.dwFlags1 & PREVENTMAXIMIZE){
			if ((wParam == SIZE_MAXIMIZED)||(wParam == SIZE_MAXSHOW)){
				OutTraceD("%s: prevent screen SIZE to fullscreen wparam=%d(%s) size=(%d,%d)\n", ApiName,
					wParam, ExplainResizing(wParam), HIWORD(lParam), LOWORD(lParam));
				return 0; // checked
				//lParam = MAKELPARAM(dxw.GetScreenWidth(), dxw.GetScreenHeight()); 
				//OutTraceD("%s: updated SIZE wparam=%d(%s) size=(%d,%d)\n", ApiName,
				//	wParam, ExplainResizing(wParam), HIWORD(lParam), LOWORD(lParam));
			}
		}
		break;	
	default:
		break;
	}
	
	// marker to run hooked function
	return(-1);
}

static POINT FixMessagePt(HWND hwnd, POINT point)
{
	RECT rect;
	static POINT curr;
	curr=point;

	if(!(*pScreenToClient)(hwnd, &curr)){
		OutTraceE("ScreenToClient ERROR=%d hwnd=%x at %d\n", GetLastError(), hwnd, __LINE__);
		curr.x = curr.y = 0;
	}

	if (!(*pGetClientRect)(hwnd, &rect)) {
		OutTraceE("GetClientRect ERROR=%d hwnd=%x at %d\n", GetLastError(), hwnd, __LINE__);
		curr.x = curr.y = 0;
	}

#ifdef ISDEBUG
	if(IsDebug) OutTrace("FixMessagePt point=(%d,%d) hwnd=%x win pos=(%d,%d) size=(%d,%d)\n",
		point.x, point.y, hwnd, point.x-curr.x, point.y-curr.y, rect.right, rect.bottom);
#endif

	if (curr.x < 0) curr.x=0;
	if (curr.y < 0) curr.y=0;
	if (curr.x > rect.right) curr.x=rect.right;
	if (curr.y > rect.bottom) curr.y=rect.bottom;
	if (rect.right)  curr.x = (curr.x * dxw.GetScreenWidth()) / rect.right;
	if (rect.bottom) curr.y = (curr.y * dxw.GetScreenHeight()) / rect.bottom;

	return curr;
}

// --------------------------------------------------------------------------
//
// user32 API hookers
//
// --------------------------------------------------------------------------

BOOL WINAPI extDDInvalidateRect(HWND hwnd, RECT *lpRect, BOOL bErase)
{
	if(lpRect)
		OutTraceD("InvalidateRect: hwnd=%x rect=(%d,%d)-(%d,%d) erase=%x\n",
		hwnd, lpRect->left, lpRect->top, lpRect->right, lpRect->bottom, bErase);
	else
		OutTraceD("InvalidateRect: hwnd=%x rect=NULL erase=%x\n",
		hwnd, bErase);

	return (*pInvalidateRect)(hwnd, NULL, bErase);
}

BOOL WINAPI extInvalidateRect(HWND hwnd, RECT *lpRect, BOOL bErase)
{
	if(lpRect)
		OutTraceD("InvalidateRect: hwnd=%x rect=(%d,%d)-(%d,%d) erase=%x\n",
		hwnd, lpRect->left, lpRect->top, lpRect->right, lpRect->bottom, bErase);
	else
		OutTraceD("InvalidateRect: hwnd=%x rect=NULL erase=%x\n",
		hwnd, bErase);

	return (*pInvalidateRect)(hwnd, NULL, bErase);
}

BOOL WINAPI extShowWindow(HWND hwnd, int nCmdShow)
{
	BOOL res;

	OutTraceD("ShowWindow: hwnd=%x, CmdShow=%x(%s)\n", hwnd, nCmdShow, ExplainShowCmd(nCmdShow));
	if (dxw.dwFlags1 & PREVENTMAXIMIZE){
		if(nCmdShow==SW_MAXIMIZE){
			OutTraceD("ShowWindow: suppress maximize\n");
			nCmdShow=SW_SHOWNORMAL;
		}
	}

	res=(*pShowWindow)(hwnd, nCmdShow);

	return res;
}

LONG WINAPI extGetWindowLong(HWND hwnd, int nIndex)
{
	LONG res;

	res=(*pGetWindowLong)(hwnd, nIndex);

	OutTraceD("GetWindowLong: hwnd=%x, Index=%x(%s) res=%x\n", hwnd, nIndex, ExplainSetWindowIndex(nIndex), res);

	if(nIndex==GWL_WNDPROC){
		WNDPROC wp;
		wp=WhndGetWindowProc(hwnd);
		OutTraceD("GetWindowLong: remapping WindowProc res=%x -> %x\n", res, (LONG)wp);
		if(wp) res=(LONG)wp; // if not found, don't alter the value.
	}

	return res;
}

LONG WINAPI extSetWindowLong(HWND hwnd, int nIndex, LONG dwNewLong)
{
	LONG res;

	OutTraceD("SetWindowLong: hwnd=%x, Index=%x(%s) Val=%x\n", 
		hwnd, nIndex, ExplainSetWindowIndex(nIndex), dwNewLong);

	//if(!hwnd) hwnd=dxw.GethWnd();

	if (dxw.dwFlags1 & LOCKWINSTYLE){
		if(nIndex==GWL_STYLE){
			OutTraceD("SetWindowLong: Lock GWL_STYLE=%x\n", dwNewLong);
			//return 1;
			return (*pGetWindowLong)(hwnd, nIndex);
		}
		if(nIndex==GWL_EXSTYLE){
			OutTraceD("SetWindowLong: Lock GWL_EXSTYLE=%x\n", dwNewLong);
			//return 1;
			return (*pGetWindowLong)(hwnd, nIndex);
		}
	}

	if (dxw.dwFlags1 & PREVENTMAXIMIZE){
		if(nIndex==GWL_STYLE){
			dwNewLong &= ~WS_MAXIMIZE; 
			if(dxw.IsDesktop(hwnd)){
				OutTraceD("SetWindowLong: GWL_STYLE %x suppress MAXIMIZE\n", dwNewLong);
				dwNewLong |= WS_OVERLAPPEDWINDOW; 
				dwNewLong &= ~(WS_DLGFRAME|WS_MAXIMIZE|WS_VSCROLL|WS_HSCROLL|WS_CLIPSIBLINGS); 
			}
		}
		// v2.02.32: disable topmost for main window only
		if(dxw.IsDesktop(hwnd) && (nIndex==GWL_EXSTYLE)){
			OutTraceD("SetWindowLong: GWL_EXSTYLE %x suppress TOPMOST\n", dwNewLong);
			dwNewLong = dwNewLong & ~(WS_EX_TOPMOST); 
		}
	}

	if (dxw.dwFlags1 & FIXWINFRAME){
		if((nIndex==GWL_STYLE) && !(dwNewLong & WS_CHILD)){
			OutTraceD("SetWindowLong: GWL_STYLE %x force OVERLAPPEDWINDOW\n", dwNewLong);
			dwNewLong |= WS_OVERLAPPEDWINDOW; 
			dwNewLong &= ~WS_CLIPSIBLINGS; 
		}
	}

	if (nIndex==GWL_WNDPROC){
		WNDPROC lres;
		WNDPROC OldProc;
		// GPL fix
		if(dxw.IsRealDesktop(hwnd)) {
			hwnd=dxw.GethWnd();
			OutTraceD("SetWindowLong: DESKTOP hwnd, FIXING hwnd=%x\n",hwnd);
		}
		// end of GPL fix

		OldProc = (WNDPROC)(*pGetWindowLong)(hwnd, GWL_WNDPROC);
		if(OldProc==extWindowProc) OldProc=WhndGetWindowProc(hwnd);
		WhndStackPush(hwnd, (WNDPROC)dwNewLong);
		res=(LONG)OldProc;
		SetLastError(0);
		lres=(WNDPROC)(*pSetWindowLong)(hwnd, GWL_WNDPROC, (LONG)extWindowProc);
		if(!lres && GetLastError())OutTraceE("SetWindowLong: ERROR err=%d at %d\n", GetLastError(), __LINE__);
	}
	else {
		res=(*pSetWindowLong)(hwnd, nIndex, dwNewLong);
	}

	OutTraceD("SetWindowLong: hwnd=%x, nIndex=%x, Val=%x, res=%x\n", hwnd, nIndex, dwNewLong, res);
	return res;
}

BOOL WINAPI extSetWindowPos(HWND hwnd, HWND hWndInsertAfter, int X, int Y, int cx, int cy, UINT uFlags)
{
	BOOL res;

	OutTraceD("SetWindowPos: hwnd=%x%s pos=(%d,%d) dim=(%d,%d) Flags=%x\n", 
		hwnd, dxw.IsFullScreen()?"(FULLSCREEN)":"", X, Y, cx, cy, uFlags);

	//if ((hwnd != dxw.GethWnd()) || !dxw.IsFullScreen()){
	if (!dxw.IsDesktop(hwnd) || !dxw.IsFullScreen()){
		// just proxy
		res=(*pSetWindowPos)(hwnd, hWndInsertAfter, X, Y, cx, cy, uFlags);
		if(!res)OutTraceE("SetWindowPos: ERROR err=%d at %d\n", GetLastError(), __LINE__);
		return res;
	}

	if ((dxw.dwFlags1 & LOCKWINPOS) && dxw.IsFullScreen()){
		// Note: any attempt to change the window position, no matter where and how, through the
		// SetWindowPos API is causing resizing to the default 1:1 pixed size in Commandos. 
		// in such cases, there is incompatibility between LOCKWINPOS and LOCKWINSTYLE.
		OutTraceD("SetWindowPos: locked position\n");
		return 1;
	}

	if (dxw.dwFlags1 & PREVENTMAXIMIZE){
		int UpdFlag =0;
		int MaxX, MaxY;
		MaxX = dxw.iSizX;
		MaxY = dxw.iSizY;
		if (!MaxX) MaxX = dxw.GetScreenWidth();
		if (!MaxY) MaxY = dxw.GetScreenHeight();
		if(cx>MaxX) { cx=MaxX; UpdFlag=1; }
		if(cy>MaxY) { cy=MaxY; UpdFlag=1; }
		if (UpdFlag) 
			OutTraceD("SetWindowPos: using max dim=(%d,%d)\n", cx, cy);
	}

	// useful??? to be demonstrated....
	// when altering main window in fullscreen mode, fix the coordinates for borders
	DWORD dwCurStyle;
	RECT rect;
	rect.top=rect.left=0;
	rect.right=cx; rect.bottom=cy;
	dwCurStyle=(*pGetWindowLong)(hwnd, GWL_STYLE);
	AdjustWindowRect(&rect, dwCurStyle, FALSE);
	cx=rect.right; cy=rect.bottom;
	OutTraceD("SetWindowPos: main form hwnd=%x fixed size=(%d,%d)\n", hwnd, cx, cy);

	res=(*pSetWindowPos)(hwnd, hWndInsertAfter, X, Y, cx, cy, uFlags);
	if(!res)OutTraceE("SetWindowPos: ERROR err=%d at %d\n", GetLastError(), __LINE__);
	return res;
}

HDWP WINAPI extDeferWindowPos(HDWP hWinPosInfo, HWND hwnd, HWND hWndInsertAfter, int X, int Y, int cx, int cy, UINT uFlags)
{
	// v2.02.31: heavily used by "Imperialism II" !!!
	HDWP res;

	OutTraceD("DeferWindowPos: hwnd=%x%s pos=(%d,%d) dim=(%d,%d) Flags=%x\n", 
		hwnd, dxw.IsFullScreen()?"(FULLSCREEN)":"", X, Y, cx, cy, uFlags);

	if(dxw.IsFullScreen()){
		dxw.MapClient(&X, &Y, &cx, &cy);
		OutTraceD("DeferWindowPos: remapped pos=(%d,%d) dim=(%d,%d)\n", X, Y, cx, cy);
	}

	res=(*pGDIDeferWindowPos)(hWinPosInfo, hwnd, hWndInsertAfter, X, Y, cx, cy, uFlags);
	if(!res)OutTraceE("DeferWindowPos: ERROR err=%d at %d\n", GetLastError(), __LINE__);
	return res;
}

LRESULT WINAPI extSendMessage(HWND hwnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	LRESULT ret;
	OutTraceW("SendMessage: hwnd=%x WinMsg=[0x%x]%s(%x,%x)\n", 
		hwnd, Msg, ExplainWinMessage(Msg), wParam, lParam);

	//if(Msg==WM_NCDESTROY) return 1;
	
	if(dxw.dwFlags1 & MODIFYMOUSE){
		switch (Msg){
		case WM_MOUSEMOVE:
		case WM_MOUSEWHEEL:
		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_LBUTTONDBLCLK:
		case WM_RBUTTONDOWN:
		case WM_RBUTTONUP:
		case WM_RBUTTONDBLCLK:
		case WM_MBUTTONDOWN:
		case WM_MBUTTONUP:
		case WM_MBUTTONDBLCLK:
			// revert here the WindowProc mouse correction
			POINT prev, curr;
			RECT rect;
			prev.x = LOWORD(lParam);
			prev.y = HIWORD(lParam);
			(*pGetClientRect)(dxw.GethWnd(), &rect);
			curr.x = (prev.x * rect.right) / dxw.GetScreenWidth();
			curr.y = (prev.y * rect.bottom) / dxw.GetScreenHeight();
			if (Msg == WM_MOUSEWHEEL){ // v2.02.33 mousewheel fix
				POINT upleft={0,0};
				(*pClientToScreen)(dxw.GethWnd(), &upleft);
				curr = dxw.AddCoordinates(curr, upleft);
			}
			lParam = MAKELPARAM(curr.x, curr.y); 
			OutTraceC("SendMessage: hwnd=%x pos XY=(%d,%d)->(%d,%d)\n", hwnd, prev.x, prev.y, curr.x, curr.y);
			break;
		default:
			break;
		}
	}
	ret=(*pSendMessageA)(hwnd, Msg, wParam, lParam);
	OutTraceW("SendMessage: lresult=%x\n", ret); 
	return ret;
}

HCURSOR WINAPI extSetCursor(HCURSOR hCursor)
{
	HCURSOR ret;

	ret=(*pSetCursor)(hCursor);
	OutTraceD("GDI.SetCursor: Cursor=%x, ret=%x\n", hCursor, ret);
	//MessageBox(0, "SelectPalette", "GDI32.dll", MB_OK | MB_ICONEXCLAMATION);
	return ret;
}

BOOL WINAPI extGetCursorPos(LPPOINT lppoint)
{
	HRESULT res;
	static int PrevX, PrevY;
	POINT prev;

	if(dxw.dwFlags1 & SLOWDOWN) dxw.DoSlow(2);

	if (pGetCursorPos) {
		res=(*pGetCursorPos)(lppoint);
	}
	else {
		lppoint->x =0; lppoint->y=0;
		res=1;
	}

	prev=*lppoint;
	*lppoint=dxw.ScreenToClient(*lppoint);
	*lppoint=dxw.FixCursorPos(*lppoint);
	GetHookInfo()->CursorX=(short)lppoint->x;
	GetHookInfo()->CursorY=(short)lppoint->y;
	OutTraceC("GetCursorPos: FIXED pos=(%d,%d)->(%d,%d)\n", prev.x, prev.y, lppoint->x, lppoint->y);

	return res;
}

BOOL WINAPI extSetCursorPos(int x, int y)
{
	BOOL res;
	int PrevX, PrevY;

	PrevX=x;
	PrevY=y;

	if(dxw.dwFlags2 & KEEPCURSORFIXED) {
		OutTraceC("SetCursorPos: FIXED pos=(%d,%d)\n", x, y);
		LastCurPosX=x;
		LastCurPosY=y;
		return 1;
	}

	if(dxw.dwFlags1 & SLOWDOWN) dxw.DoSlow(2);

	if(dxw.dwFlags1 & KEEPCURSORWITHIN){
		// Intercept SetCursorPos outside screen boundaries (used as Cursor OFF in some games)
		if ((y<0)||(y>=(int)dxw.GetScreenHeight())||(x<0)||(x>=(int)dxw.GetScreenWidth())) return 1;
	}

	if(dxw.dwFlags1 & MODIFYMOUSE){
		POINT cur;
		RECT rect;

		// find window metrics
		if (!(*pGetClientRect)(dxw.GethWnd(), &rect)) {
			// report error and ignore ...
			OutTraceE("GetClientRect(%x) ERROR %d at %d\n", dxw.GethWnd(), GetLastError(), __LINE__);
			return 0;
		}

		x= x * rect.right / dxw.GetScreenWidth();
		y= y * rect.bottom / dxw.GetScreenHeight();

		// check for boundaries (???)
		if (x >= rect.right) x=rect.right-1;
		if (x<0) x=0;
		if (y >= rect.bottom) y=rect.bottom-1;
		if (y<0) y=0;

		// make it screen absolute
		cur.x = x;
		cur.y = y;
		if (!(*pClientToScreen)(dxw.GethWnd(), &cur)) {
			OutTraceE("ClientToScreen(%x) ERROR %d at %d\n", dxw.GethWnd(), GetLastError(), __LINE__);
			return 0;
		}
		x = cur.x;
		y = cur.y;
	}

	res=0;
	if (pSetCursorPos) res=(*pSetCursorPos)(x,y);

	OutTraceC("SetCursorPos: res=%x XY=(%d,%d)->(%d,%d)\n",res, PrevX, PrevY, x, y);
	return res;
}

BOOL WINAPI extPeekAnyMessage(LPMSG lpMsg, HWND hwnd, UINT wMsgFilterMin, UINT wMsgFilterMax, UINT wRemoveMsg)
{
	BOOL res;

	res=(*pPeekMessage)(lpMsg, hwnd, 0, 0, (wRemoveMsg & 0x000F));

	OutTraceW("PeekMessage: ANY lpmsg=%x hwnd=%x filter=(%x-%x) remove=%x msg=%x(%s) wparam=%x, lparam=%x pt=(%d,%d) res=%x\n", 
		lpMsg, lpMsg->hwnd, wMsgFilterMin, wMsgFilterMax, wRemoveMsg, 
		lpMsg->message, ExplainWinMessage(lpMsg->message & 0xFFFF), 
		lpMsg->wParam, lpMsg->lParam, lpMsg->pt.x, lpMsg->pt.y, res);

	return res;
}

BOOL WINAPI extPeekMessage(LPMSG lpMsg, HWND hwnd, UINT wMsgFilterMin, UINT wMsgFilterMax, UINT wRemoveMsg)
{
	BOOL res;
	UINT iRemoveMsg;

	iRemoveMsg = wRemoveMsg;
	if(1) iRemoveMsg &= 0x000F; // Peek all messages

	res=(*pPeekMessage)(lpMsg, hwnd, wMsgFilterMin, wMsgFilterMax, iRemoveMsg);
	
	OutTraceW("PeekMessage: lpmsg=%x hwnd=%x filter=(%x-%x) remove=%x msg=%x(%s) wparam=%x, lparam=%x pt=(%d,%d) res=%x\n", 
		lpMsg, lpMsg->hwnd, wMsgFilterMin, wMsgFilterMax, wRemoveMsg, 
		lpMsg->message, ExplainWinMessage(lpMsg->message & 0xFFFF), 
		lpMsg->wParam, lpMsg->lParam, lpMsg->pt.x, lpMsg->pt.y, res);

	// v2.1.74: skip message fix for WM_CHAR to avoid double typing bug
	switch(lpMsg->message){
		//case WM_CHAR:
		case WM_KEYUP:
		case WM_KEYDOWN:
			return res;
	}

	// fix to avoid crash in Warhammer Final Liberation, that evidently intercepts mouse position by 
	// peeking & removing messages from window queue and considering the lParam parameter.
	// v2.1.100 - never alter the lpMsg, otherwise the message is duplicated in the queue! Work on a copy of it.
	if(wRemoveMsg){
		static MSG MsgCopy;
		MsgCopy=*lpMsg;
		MsgCopy.pt=FixMessagePt(dxw.GethWnd(), MsgCopy.pt);
		if((MsgCopy.message <= WM_MOUSELAST) && (MsgCopy.message >= WM_MOUSEFIRST)) MsgCopy.lParam = MAKELPARAM(MsgCopy.pt.x, MsgCopy.pt.y); 
		OutTraceC("PeekMessage: fixed lparam/pt=(%d,%d)\n", MsgCopy.pt.x, MsgCopy.pt.y);
		lpMsg=&MsgCopy;
		GetHookInfo()->CursorX=(short)MsgCopy.pt.x;
		GetHookInfo()->CursorY=(short)MsgCopy.pt.y;
	}

	return res;
}

BOOL WINAPI extGetMessage(LPMSG lpMsg, HWND hwnd, UINT wMsgFilterMin, UINT wMsgFilterMax)
{
	BOOL res;
	HWND FixedHwnd;

	res=(*pGetMessage)(lpMsg, hwnd, wMsgFilterMin, wMsgFilterMax);

	OutTraceW("GetMessage: lpmsg=%x hwnd=%x filter=(%x-%x) msg=%x(%s) wparam=%x, lparam=%x pt=(%d,%d) res=%x\n", 
		lpMsg, lpMsg->hwnd, wMsgFilterMin, wMsgFilterMax, 
		lpMsg->message, ExplainWinMessage(lpMsg->message & 0xFFFF), 
		lpMsg->wParam, lpMsg->lParam, lpMsg->pt.x, lpMsg->pt.y, res);

	// V2.1.68: processing ALL mouse events, to sync mouse over and mouse click events
	// in "Uprising 2", now perfectly working.
	DWORD Message;
	Message=lpMsg->message & 0xFFFF;
	if((Message <= WM_MOUSELAST) && (Message >= WM_MOUSEFIRST)){
		FixedHwnd=(hwnd)?hwnd:dxw.GethWnd();
		if(dxw.IsRealDesktop(FixedHwnd)) FixedHwnd=dxw.GethWnd(); // GPL fix...
		lpMsg->pt=FixMessagePt(FixedHwnd, lpMsg->pt);
		lpMsg->lParam = MAKELPARAM(lpMsg->pt.x, lpMsg->pt.y); 
		OutTraceC("PeekMessage: fixed lparam/pt=(%d,%d)\n", lpMsg->pt.x, lpMsg->pt.y);
		GetHookInfo()->CursorX=(short)lpMsg->pt.x;
		GetHookInfo()->CursorY=(short)lpMsg->pt.y;
	}
	return res;
}

BOOL WINAPI extClientToScreen(HWND hwnd, LPPOINT lppoint)
{
	// v2.02.10: fully revised to handle scaled windows
	BOOL res;

	OutTraceB("ClientToScreen: hwnd=%x hWnd=%x FullScreen=%x point=(%d,%d)\n", 
		hwnd, dxw.GethWnd(), dxw.IsFullScreen(), lppoint->x, lppoint->y);
	if (lppoint && dxw.IsFullScreen()){
		*lppoint = dxw.AddCoordinates(*lppoint, dxw.ClientOffset(hwnd));
		OutTraceB("ClientToScreen: FIXED point=(%d,%d)\n", lppoint->x, lppoint->y);
		res=TRUE;
	}
	else {
		res=(*pClientToScreen)(hwnd, lppoint);
	}
	return res;
}

BOOL WINAPI extScreenToClient(HWND hwnd, LPPOINT lppoint)
{
	// v2.02.10: fully revised to handle scaled windows
	BOOL res;
	OutTraceB("ScreenToClient: hwnd=%x hWnd=%x FullScreen=%x point=(%d,%d)\n", 
		hwnd, dxw.GethWnd(), dxw.IsFullScreen(), lppoint->x, lppoint->y);

	if (lppoint && (lppoint->x == -32000) && (lppoint->y == -32000)) return 1;

	if (lppoint && dxw.IsFullScreen()){
		*lppoint = dxw.SubCoordinates(*lppoint, dxw.ClientOffset(hwnd));
		OutTraceB("ScreenToClient: FIXED point=(%d,%d)\n", lppoint->x, lppoint->y);
		res=TRUE;
	}
	else {
		res=(*pScreenToClient)(hwnd, lppoint);
	}
	return res;
}

BOOL WINAPI extGetClientRect(HWND hwnd, LPRECT lpRect)
{
	BOOL ret;
	OutTraceB("GetClientRect: whnd=%x FullScreen=%x\n", hwnd, dxw.IsFullScreen());

	if(!lpRect) return 0;

	// proxed call
	ret=(*pGetClientRect)(hwnd, lpRect);
	if(!ret) {
		OutTraceE("GetClientRect: ERROR hwnd=%x err=%d at %d\n", hwnd, GetLastError(), __LINE__);
		return ret;
	}
	OutTraceB("GetClientRect: actual rect=(%d,%d)-(%d,%d)\n", lpRect->left, lpRect->top, lpRect->right, lpRect->bottom);

	if (dxw.IsDesktop(hwnd)){
		*lpRect = dxw.GetScreenRect();
		OutTraceB("GetClientRect: desktop rect=(%d,%d)-(%d,%d)\n", lpRect->left, lpRect->top, lpRect->right, lpRect->bottom);
	}
	else 
	if (dxw.IsFullScreen()){
		*lpRect=dxw.GetClientRect(*lpRect);
		OutTraceB("GetClientRect: fixed rect=(%d,%d)-(%d,%d)\n", lpRect->left, lpRect->top, lpRect->right, lpRect->bottom);
	}
	return ret;
}

BOOL WINAPI extGetWindowRect(HWND hwnd, LPRECT lpRect)
{
	BOOL ret;
	OutTraceB("GetWindowRect: hwnd=%x hWnd=%x FullScreen=%x\n", hwnd, dxw.GethWnd(), dxw.IsFullScreen());
	ret=(*pGetWindowRect)(hwnd, lpRect);
	if(!ret) {
		OutTraceE("GetWindowRect: GetWindowRect hwnd=%x error %d at %d\n", hwnd, GetLastError(), __LINE__);
		return ret;
	}
	OutTraceB("GetWindowRect: rect=(%d,%d)-(%d,%d)\n", lpRect->left, lpRect->top, lpRect->right, lpRect->bottom);
	
	// minimized windows behaviour
	if((lpRect->left == -32000)||(lpRect->top == -32000)) return ret;

	if (dxw.IsDesktop(hwnd)){
		// to avoid keeping track of window frame
		*lpRect = dxw.GetScreenRect();
		OutTraceB("GetWindowRect: desktop rect=(%d,%d)-(%d,%d)\n", lpRect->left, lpRect->top, lpRect->right, lpRect->bottom);
	}
	else
	if (dxw.IsFullScreen()){
		*lpRect=dxw.GetWindowRect(*lpRect);

		// Diablo fix: it retrieves coordinates for the explorer window, that are as big as the real desktop!!!
		if(lpRect->left < 0) lpRect->left=0;
		if(lpRect->right > (LONG)dxw.GetScreenWidth()) lpRect->right=dxw.GetScreenWidth();
		if(lpRect->top < 0) lpRect->top=0;
		if(lpRect->bottom > (LONG)dxw.GetScreenHeight()) lpRect->bottom=dxw.GetScreenHeight();

		OutTraceB("GetWindowRect: fixed rect=(%d,%d)-(%d,%d)\n", lpRect->left, lpRect->top, lpRect->right, lpRect->bottom);
	}

	return ret;
}

int WINAPI extMapWindowPoints(HWND hWndFrom, HWND hWndTo, LPPOINT lpPoints, UINT cPoints)
{
	UINT pi;
	int ret;
	// a rarely used API, but responsible for a painful headache: needs hooking for "Commandos 2", "Alien Nations".

	OutTraceD("MapWindowPoints: hWndFrom=%x%s hWndTo=%x%s cPoints=%d FullScreen=%x\n", 
		hWndFrom, dxw.IsDesktop(hWndFrom)?"(DESKTOP)":"",
		hWndTo, dxw.IsDesktop(hWndTo)?"(DESKTOP)":"",
		cPoints, dxw.IsFullScreen());
	if(IsDebug){
		OutTrace("Points: ");
		for(pi=0; pi<cPoints; pi++) OutTrace("(%d,%d)", lpPoints[pi].x, lpPoints[pi].y);
		OutTrace("\n");
	}

	if(dxw.IsFullScreen()){
		if(dxw.IsRealDesktop(hWndTo)) hWndTo=dxw.GethWnd();
		if(dxw.IsRealDesktop(hWndFrom)) hWndFrom=dxw.GethWnd();
	}
	
	// should scale the retcode ???
	ret=(*pMapWindowPoints)(hWndFrom, hWndTo, lpPoints, cPoints);
	OutTraceD("MapWindowPoints: ret=%x (%d,%d)\n", ret, (ret&0xFFFF0000)>>16, ret&0x0000FFFF);
	return ret;
}

HWND WINAPI extGetDesktopWindow(void)
{
	// V2.1.73: correct ???
	HWND res;

	OutTraceD("GetDesktopWindow: FullScreen=%x\n", dxw.IsFullScreen());
	if (dxw.IsFullScreen()){ 
		OutTraceD("GetDesktopWindow: returning main window hwnd=%x\n", dxw.GethWnd());
		return dxw.GethWnd();
	}
	else{
		res=(*pGetDesktopWindow)();
		OutTraceD("GetDesktopWindow: returning desktop window hwnd=%x\n", res);
		return res;
	}
}

int WINAPI extGetSystemMetrics(int nindex)
{
	HRESULT res;

	res=(*pGetSystemMetrics)(nindex);
	OutTraceD("GetSystemMetrics: index=%x(%s), res=%d\n", nindex, ExplainsSystemMetrics(nindex), res);

	// if you have a bypassed setting, use it first!
	if(pSetDevMode){
		switch(nindex){
		case SM_CXFULLSCREEN:
		case SM_CXSCREEN:
		case SM_CXVIRTUALSCREEN: // v2.02.31
			res = pSetDevMode->dmPelsWidth;
			OutTraceD("GetDeviceCaps: fix HORZRES cap=%d\n", res);
			return res;
		case SM_CYFULLSCREEN:
		case SM_CYSCREEN:
		case SM_CYVIRTUALSCREEN: // v2.02.31
			res = pSetDevMode->dmPelsHeight;
			OutTraceD("GetDeviceCaps: fix VERTRES cap=%d\n", res);
			return res;
		}
	}

	switch(nindex){
	case SM_CXFULLSCREEN:
	case SM_CXSCREEN:
	case SM_CXVIRTUALSCREEN: // v2.02.31
		res= dxw.GetScreenWidth();
		OutTraceD("GetSystemMetrics: fix SM_CXSCREEN=%d\n", res);
		break;
	case SM_CYFULLSCREEN:
	case SM_CYSCREEN:
	case SM_CYVIRTUALSCREEN: // v2.02.31
		res= dxw.GetScreenHeight();
		OutTraceD("GetSystemMetrics: fix SM_CYSCREEN=%d\n", res);
		break;
	case SM_CMONITORS:
		if((dxw.dwFlags2 & HIDEMULTIMONITOR) && res>1) {
			res=1;
			OutTraceD("GetSystemMetrics: fix SM_CMONITORS=%d\n", res);
		}
		break;
	}

	return res;
}

ATOM WINAPI extRegisterClassExA(WNDCLASSEX *lpwcx)
{
	OutTraceD("RegisterClassEx: PROXED ClassName=%s style=%x(%s)\n", 
		lpwcx->lpszClassName, lpwcx->style, ExplainStyle(lpwcx->style));
	return (*pRegisterClassExA)(lpwcx);
}

static HWND WINAPI extCreateWindowCommon(
  LPCTSTR ApiName,
  DWORD dwExStyle,
  LPCTSTR lpClassName,
  LPCTSTR lpWindowName,
  DWORD dwStyle,
  int x,
  int y,
  int nWidth,
  int nHeight,
  HWND hWndParent,
  HMENU hMenu,
  HINSTANCE hInstance,
  LPVOID lpParam) 
{
	HWND wndh;
	WNDPROC pWindowProc;
	BOOL isValidHandle=TRUE;

	// no maximized windows in any case
	if (dxw.dwFlags1 & PREVENTMAXIMIZE){
		OutTraceD("%s: handling PREVENTMAXIMIZE mode\n", ApiName);
		dwStyle &= ~WS_MAXIMIZE;
		//dwStyle &= ~(WS_MAXIMIZE | WS_POPUP);
		//dwExStyle &= ~WS_EX_TOPMOST;
	}

	// v2.1.92: fixes size & position for auxiliary big window, often used
	// for intro movies etc. : needed for ......
	// evidently, this was supposed to be a fullscreen window....
	// v2.1.100: fixes for "The Grinch": this game creates a new main window for OpenGL
	// rendering using CW_USEDEFAULT placement and 800x600 size while the previous
	// main win was 640x480 only!
	// v2.02.13: if it's a WS_CHILD window, don't reposition the x,y, placement for BIG win.
	// v2.02.30: fix (Fable - lost chapters) Fable creates a bigger win with negative x,y coordinates. 
	if	(
			(
				((x<=0)&&(y<=0)) || ((x==CW_USEDEFAULT)&&(y==CW_USEDEFAULT))
			)
		&&
			((nWidth>=(int)dxw.GetScreenWidth())&&(nHeight>=(int)dxw.GetScreenHeight()))
		&&
			!(dwExStyle & WS_EX_CONTROLPARENT) // Diablo fix
		&&
			!(dwStyle & WS_CHILD) // Diablo fix
		){
		RECT screen;
		POINT upleft = {0,0};

		// v2.02.30: fix (Fable - lost chapters)
		if(nWidth==CW_USEDEFAULT) nWidth=dxw.GetScreenWidth();
		if(nHeight==CW_USEDEFAULT) nHeight=dxw.GetScreenHeight();

		// update virtual screen size if it has grown 
		dxw.SetScreenSize(nWidth, nHeight);
		// inserted some checks here, since the main window could be destroyed
		// or minimized (see "Jedi Outcast") so that you may get a dangerous 
		// zero size. In this case, better renew the hWnd assignement and its coordinates.
		do { // fake loop
			isValidHandle = FALSE;
			if (!(*pGetClientRect)(dxw.GethWnd(),&screen)) break;
			if (!(*pClientToScreen)(dxw.GethWnd(),&upleft)) break;
			if (screen.right==0 || screen.bottom==0) break;
			isValidHandle = TRUE;
		} while(FALSE);
		if (isValidHandle){ // use parent's coordinates
			if (!(dwStyle & WS_CHILD)){ 
				x=upleft.x;
				y=upleft.y;
			}
			nWidth=screen.right;
			nHeight=screen.bottom;
			OutTraceD("%s: fixed BIG win pos=(%d,%d) size=(%d,%d)\n", ApiName, x, y, nWidth, nHeight);
		}
		else {
			// invalid parent coordinates: use initial placement, but leave the size.
			// should also fix the window style and compensate for borders here?
			// if (!(dwStyle & WS_CHILD)){  // commented out: can't be! see if condition
				x=dxw.iPosX;
				y=dxw.iPosY;
			//}
			nWidth=dxw.iSizX;
			nHeight=dxw.iSizY;
			OutTraceD("%s: renewed BIG win pos=(%d,%d) size=(%d,%d)\n", ApiName, x, y, nWidth, nHeight);
		}
		dxw.SetFullScreen(TRUE);
		if(dxw.Coordinates==DXW_DESKTOP_WORKAREA){
			RECT workarea;
			SystemParametersInfo(SPI_GETWORKAREA, NULL, &workarea, 0);
			x=0;
			y=0;
			nWidth=workarea.right;
			nHeight=workarea.bottom;
			dwStyle=0;
			OutTraceD("%s: WORKAREA win pos=(%d,%d) size=(%d,%d)\n", ApiName, x, y, nWidth, nHeight);
		}
		else if(dxw.Coordinates==DXW_DESKTOP_FULL){
			RECT workarea;
			(*pGetClientRect)((*pGetDesktopWindow)(), &workarea);
			x=0;
			y=0;
			nWidth=workarea.right;
			nHeight=workarea.bottom;
			dwStyle=0;
			OutTraceD("%s: FULLDESKTOP win pos=(%d,%d) size=(%d,%d)\n", ApiName, x, y, nWidth, nHeight);
		}
	}

	if(!dxw.IsFullScreen()){ // v2.1.63: needed for "Monster Truck Madness"
		wndh= (*pCreateWindowExA)(dwExStyle, lpClassName, lpWindowName, dwStyle, x, y, nWidth, nHeight, 
			hWndParent, hMenu, hInstance, lpParam);
		OutTraceD("%s: windowed mode ret=%x\n", ApiName, wndh);
		return wndh;
	}

	// tested on Gangsters: coordinates must be window-relative!!!
	// Age of Empires....
	if (dwStyle & WS_CHILD){ 
		dxw.MapClient(&x, &y, &nWidth, &nHeight);
		OutTraceD("%s: fixed WS_CHILD pos=(%d,%d) size=(%d,%d)\n",
			ApiName, x, y, nWidth, nHeight);
	}
	// needed for Diablo, that creates a new control parent window that must be
	// overlapped to the directdraw surface.
	else if (dwExStyle & WS_EX_CONTROLPARENT){
		dxw.MapWindow(&x, &y, &nWidth, &nHeight);
		OutTraceD("%s: fixed WS_EX_CONTROLPARENT pos=(%d,%d) size=(%d,%d)\n",
			ApiName, x, y, nWidth, nHeight);
	}

	OutTraceB("%s: fixed pos=(%d,%d) size=(%d,%d) Style=%x(%s) ExStyle=%x(%s)\n",
		ApiName, x, y, nWidth, nHeight, dwStyle, ExplainStyle(dwStyle), dwExStyle, ExplainExStyle(dwExStyle));

	wndh= (*pCreateWindowExA)(dwExStyle, lpClassName, lpWindowName, dwStyle, x, y, nWidth, nHeight, 
		hWndParent, hMenu, hInstance, lpParam);
	if (wndh==(HWND)NULL){
		OutTraceE("%s: ERROR err=%d Style=%x(%s) ExStyle=%x\n",
			ApiName, GetLastError(), dwStyle, ExplainStyle(dwStyle), dwExStyle);
		return wndh;
	}

	if ((!isValidHandle) && dxw.IsFullScreen()) {
		dxw.SethWnd(wndh);
		extern void AdjustWindowPos(HWND, DWORD, DWORD);
		(*pSetWindowLong)(wndh, GWL_STYLE, (dxw.dwFlags2 & MODALSTYLE) ? 0 : WS_OVERLAPPEDWINDOW);
		(*pSetWindowLong)(wndh, GWL_EXSTYLE, 0); 
		OutTraceD("%s: hwnd=%x, set style=WS_OVERLAPPEDWINDOW extstyle=0\n", ApiName, wndh); 
		AdjustWindowPos(wndh, nWidth, nHeight);
		(*pShowWindow)(wndh, SW_SHOWNORMAL);
	}

	if ((dxw.dwFlags1 & FIXWINFRAME) && !(dwStyle & WS_CHILD))
		FixWindowFrame(wndh);

	// to do: handle inner child, and leave dialogue & modal child alone!!!
	if ((dwStyle & WS_CHILD) && (dxw.dwFlags1 & HOOKCHILDWIN)){
		long res;
		pWindowProc = (WNDPROC)(*pGetWindowLong)(wndh, GWL_WNDPROC);
		OutTraceD("Hooking CHILD wndh=%x WindowProc %x->%x\n", wndh, pWindowProc, extChildWindowProc);
		res=(*pSetWindowLong)(wndh, GWL_WNDPROC, (LONG)extChildWindowProc);
		WhndStackPush(wndh, pWindowProc);
		if(!res) OutTraceE("%s: SetWindowLong ERROR %x\n", ApiName, GetLastError());
	}

	OutTraceD("%s: ret=%x\n", ApiName, wndh);
	return wndh;
}

static LPCSTR ClassToStr(LPCSTR Class)
{
	static char AtomBuf[20+1];
	if(((DWORD)Class & 0xFFFF0000) == 0){
		sprintf(AtomBuf, "ATOM(%X)", Class);
		return AtomBuf;
	}
	return Class;
}

// to do: implement and use ClassToWStr() for widechar call

HWND WINAPI extCreateWindowExW(
  DWORD dwExStyle,
  LPCWSTR lpClassName,
  LPCWSTR lpWindowName,
  DWORD dwStyle,
  int x,
  int y,
  int nWidth,
  int nHeight,
  HWND hWndParent,
  HMENU hMenu,
  HINSTANCE hInstance,
  LPVOID lpParam) 
{
	char sClassName[256+1];
	char sWindowName[256+1];
	wcstombs_s(NULL, sClassName, lpClassName, 80);
	wcstombs_s(NULL, sWindowName, lpWindowName, 80);

	OutTraceD("CreateWindowExW: class=\"%ls\" wname=\"%ls\" pos=(%d,%d) size=(%d,%d) Style=%x(%s) ExStyle=%x(%s)\n",
		lpClassName, lpWindowName, x, y, nWidth, nHeight, 
		dwStyle, ExplainStyle(dwStyle), dwExStyle, ExplainExStyle(dwExStyle));
	if(IsDebug) OutTrace("CreateWindowExW: DEBUG screen=(%d,%d)\n", dxw.GetScreenWidth(), dxw.GetScreenHeight());

	return extCreateWindowCommon("CreateWindowExW", dwExStyle, sClassName, sWindowName, dwStyle, x, y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam); 
}

// GHO: pro Diablo
HWND WINAPI extCreateWindowExA(
  DWORD dwExStyle,
  LPCTSTR lpClassName,
  LPCTSTR lpWindowName,
  DWORD dwStyle,
  int x,
  int y,
  int nWidth,
  int nHeight,
  HWND hWndParent,
  HMENU hMenu,
  HINSTANCE hInstance,
  LPVOID lpParam) 
{
	OutTraceD("CreateWindowExA: class=\"%s\" wname=\"%s\" pos=(%d,%d) size=(%d,%d) Style=%x(%s) ExStyle=%x(%s)\n",
		ClassToStr(lpClassName), lpWindowName, x, y, nWidth, nHeight, 
		dwStyle, ExplainStyle(dwStyle), dwExStyle, ExplainExStyle(dwExStyle));
	if(IsDebug) OutTrace("CreateWindowExA: DEBUG screen=(%d,%d)\n", dxw.GetScreenWidth(), dxw.GetScreenHeight());

	return extCreateWindowCommon("CreateWindowExA", dwExStyle, lpClassName, lpWindowName, dwStyle, x, y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam); 
}

LRESULT WINAPI extCallWindowProc(WNDPROC lpPrevWndFunc, HWND hwnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	// v2.02.30: fix (Imperialism II): apply to main window only !!!
	HRESULT res;

	res = -1;
	if(hwnd == dxw.GethWnd()) res=FixWindowProc("CallWindowProc", hwnd, Msg, wParam, &lParam);

	if (res==(HRESULT)-1)
		return (*pCallWindowProc)(lpPrevWndFunc, hwnd, Msg, wParam, lParam);
	else
		return res;
}

LRESULT WINAPI extDefWindowProc(HWND hwnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	// v2.02.30: fix (Imperialism II): apply to main window only !!!
	HRESULT res;

	res = -1;
	if(hwnd == dxw.GethWnd()) res=FixWindowProc("DefWindowProc", hwnd, Msg, wParam, &lParam);

	if (res==(HRESULT)-1)
		return (*pDefWindowProc)(hwnd, Msg, wParam, lParam);
	else
		return res;
}

int WINAPI extFillRect(HDC hdc, const RECT *lprc, HBRUSH hbr)
{
	int res;
	RECT rc;
	OutTraceD("FillRect: hdc=%x hbrush=%x rect=(%d,%d)-(%d,%d)\n", hdc, hbr, lprc->left, lprc->top, lprc->right, lprc->bottom);

	// when not in fullscreen mode, just proxy the call
	if(!dxw.IsFullScreen()) return (*pFillRect)(hdc, lprc, hbr);

	memcpy(&rc, lprc, sizeof(rc));
	if(OBJ_DC == GetObjectType(hdc)){
		if(rc.left < 0) rc.left = 0;
		if(rc.top < 0) rc.top = 0;
		if((DWORD)rc.right > dxw.GetScreenWidth()) rc.right = dxw.GetScreenWidth();
		if((DWORD)rc.bottom > dxw.GetScreenHeight()) rc.bottom = dxw.GetScreenHeight();
		dxw.MapClient(&rc);
		OutTraceD("FillRect: fixed rect=(%d,%d)-(%d,%d)\n", rc.left, rc.top, rc.right, rc.bottom);
	}

	res=(*pFillRect)(hdc, &rc, hbr);
	return res;
}

int WINAPI extFrameRect(HDC hdc, const RECT *lprc, HBRUSH hbr)
{
	int res;
	RECT rc;
	OutTraceD("FrameRect: hdc=%x hbrush=%x rect=(%d,%d)-(%d,%d)\n", hdc, hbr, lprc->left, lprc->top, lprc->right, lprc->bottom);

	// when not in fullscreen mode, just proxy the call
	if(!dxw.IsFullScreen()) return (*pFrameRect)(hdc, lprc, hbr);

	memcpy(&rc, lprc, sizeof(rc));
	if(OBJ_DC == GetObjectType(hdc)){
		if(rc.left < 0) rc.left = 0;
		if(rc.top < 0) rc.top = 0;
		if((DWORD)rc.right > dxw.GetScreenWidth()) rc.right = dxw.GetScreenWidth();
		if((DWORD)rc.bottom > dxw.GetScreenHeight()) rc.bottom = dxw.GetScreenHeight();
		dxw.MapClient(&rc);
		OutTraceD("FrameRect: fixed rect=(%d,%d)-(%d,%d)\n", rc.left, rc.top, rc.right, rc.bottom);
	}

	res=(*pFrameRect)(hdc, &rc, hbr);
	return res;
}

BOOL WINAPI extClipCursor(RECT *lpRectArg)
{
	// reference: hooking and setting ClipCursor is mandatori in "Emergency: Fighters for Life"
	// where the application expects the cursor to be moved just in a inner rect within the 
	// main window surface.

	BOOL res;
	RECT *lpRect;
	RECT Rect;

	if(IsTraceC){
		if (lpRectArg)
			OutTrace("ClipCursor: rect=(%d,%d)-(%d,%d)\n", 
				lpRectArg->left,lpRectArg->top,lpRectArg->right,lpRectArg->bottom);
		else 
			OutTrace("ClipCursor: rect=(NULL)\n");
	}

 	if (!(dxw.dwFlags1 & ENABLECLIPPING)) return 1;

	if(lpRectArg){
		Rect=*lpRectArg;
		lpRect=&Rect;
	}
	else
		lpRect=NULL;

	if(dxw.dwFlags1 & MODIFYMOUSE){
		// save desired clip region
		if (lpRect) {
			ClipRegion=*lpRectArg;
			lpClipRegion=&ClipRegion;
		}
		else
			lpClipRegion=NULL;

		*lpRect=dxw.MapWindowRect(lpRect);
	}

	if (pClipCursor) res=(*pClipCursor)(lpRect);
	OutTraceD("ClipCursor: rect=(%d,%d)-(%d,%d) res=%x\n", 
		lpRect->left,lpRect->top,lpRect->right,lpRect->bottom, res);

	return TRUE;
}

BOOL WINAPI extGetClipCursor(LPRECT lpRect)
{
	// v2.1.93: if ENABLECLIPPING, return the saved clip rect coordinates

	BOOL ret;

	// proxy....
	if (!(dxw.dwFlags1 & ENABLECLIPPING)) {
		ret=(*pGetClipCursor)(lpRect);
		if(IsTraceD){
			if (lpRect)
				OutTrace("GetClipCursor: PROXED rect=(%d,%d)-(%d,%d) ret=%d\n", 
					lpRect->left,lpRect->top,lpRect->right,lpRect->bottom, ret);
			else 
				OutTrace("GetClipCursor: PROXED rect=(NULL) ret=%d\n", ret);
		}		
		return ret;
	}

	if(lpRect){
		if(lpClipRegion)
			*lpRect=ClipRegion;
		else{
			lpRect->top = lpRect->left = 0;
			lpRect->right = dxw.GetScreenWidth();
			lpRect->bottom = dxw.GetScreenHeight();
		}
		OutTraceD("GetClipCursor: rect=(%d,%d)-(%d,%d) ret=%d\n", 
			lpRect->left,lpRect->top,lpRect->right,lpRect->bottom, TRUE);
	}

	return TRUE;
}

LONG WINAPI extEnumDisplaySettings(LPCTSTR lpszDeviceName, DWORD iModeNum, DEVMODE *lpDevMode)
{
	OutTraceD("EnumDisplaySettings: Devicename=%s ModeNum=%x\n", lpszDeviceName, iModeNum);
	if(pSetDevMode && iModeNum==ENUM_CURRENT_SETTINGS){
		lpDevMode=pSetDevMode;
		return 1;
	}
	else
		return (*pEnumDisplaySettings)(lpszDeviceName, iModeNum, lpDevMode);
}

LONG WINAPI extChangeDisplaySettings(DEVMODE *lpDevMode, DWORD dwflags)
{
	if(IsTraceD){
		OutTrace("ChangeDisplaySettings: lpDevMode=%x flags=%x", lpDevMode, dwflags);
		if (lpDevMode) OutTrace(" fields=%x(%s) size=(%d x %d) bpp=%d", 
			lpDevMode->dmFields, ExplainDevModeFields(lpDevMode->dmFields),
			lpDevMode->dmPelsWidth, lpDevMode->dmPelsHeight, lpDevMode->dmBitsPerPel);
		OutTrace("\n");
	}

	return MyChangeDisplaySettings("ChangeDisplaySettings", lpDevMode, dwflags);
}

LONG WINAPI extChangeDisplaySettingsEx(LPCTSTR lpszDeviceName, DEVMODE *lpDevMode, HWND hwnd, DWORD dwflags, LPVOID lParam)
{
	if(IsTraceD){
		OutTrace("ChangeDisplaySettingsEx: DeviceName=%s lpDevMode=%x flags=%x", lpszDeviceName, lpDevMode, dwflags);
		if (lpDevMode) OutTrace(" size=(%d x %d) bpp=%d", 
			lpDevMode->dmPelsWidth, lpDevMode->dmPelsHeight, lpDevMode->dmBitsPerPel);
		OutTrace("\n");
	}

	return MyChangeDisplaySettings("ChangeDisplaySettingsEx", lpDevMode, dwflags);
}

HDC WINAPI extGDIGetDC(HWND hwnd)
{
	HDC ret;
	HWND lochwnd;

	OutTraceD("GDI.GetDC: hwnd=%x\n", hwnd);
	lochwnd=hwnd;
	if (dxw.IsRealDesktop(hwnd)) {
		OutTraceD("GDI.GetDC: desktop remapping hwnd=%x->%x\n", hwnd, dxw.GethWnd());
		lochwnd=dxw.GethWnd();
	}

	if(dxw.dwFlags3 & EMULATEDC)
		ret=dxw.AcquireEmulatedDC(lochwnd);
	else
		ret=(*pGDIGetDC)(lochwnd);
	
	if(ret){
		OutTraceD("GDI.GetDC: hwnd=%x ret=%x\n", lochwnd, ret);
	}
	else{
		int err;
		err=GetLastError();
		OutTraceE("GDI.GetDC ERROR: hwnd=%x err=%d at %d\n", lochwnd, err, __LINE__);
		if((err==ERROR_INVALID_WINDOW_HANDLE) && (lochwnd!=hwnd)){
			ret=(*pGDIGetDC)(hwnd);	
			if(ret)
				OutTraceD("GDI.GetDC: hwnd=%x ret=%x\n", hwnd, ret);
			else
				OutTraceE("GDI.GetDC ERROR: hwnd=%x err=%d at %d\n", hwnd, GetLastError(), __LINE__);
		}
	}

	return ret;
}

HDC WINAPI extGDIGetDCEx(HWND hwnd, HRGN hrgnClip, DWORD flags)
{
	MessageBox(0, "GetDCEx", "to fix", MB_OK | MB_ICONEXCLAMATION);
	return (HDC)NULL;
}

HDC WINAPI extGDIGetWindowDC(HWND hwnd)
{
	HDC ret;
	HWND lochwnd;
	OutTraceD("GDI.GetWindowDC: hwnd=%x\n", hwnd);
	lochwnd=hwnd;
	if (dxw.IsRealDesktop(hwnd)) {
		OutTraceD("GDI.GetWindowDC: desktop remapping hwnd=%x->%x\n", hwnd, dxw.GethWnd());
		lochwnd=dxw.GethWnd();
	}

	ret=(*pGDIGetWindowDC)(lochwnd);

	if(ret){
		OutTraceD("GDI.GetWindowDC: hwnd=%x ret=%x\n", lochwnd, ret);
	}
	else{
		int err;
		err=GetLastError();
		OutTraceE("GDI.GetWindowDC ERROR: hwnd=%x err=%d at %d\n", lochwnd, err, __LINE__);
		if((err==ERROR_INVALID_WINDOW_HANDLE) && (lochwnd!=hwnd)){
			ret=(*pGDIGetWindowDC)(hwnd);
			if(ret)
				OutTraceD("GDI.GetWindowDC: hwnd=%x ret=%x\n", hwnd, ret);
			else
				OutTraceE("GDI.GetWindowDC ERROR: hwnd=%x err=%d at %d\n", hwnd, GetLastError(), __LINE__);
		}
	}
	return ret;
}

int WINAPI extGDIReleaseDC(HWND hwnd, HDC hDC)
{
	int res;

	OutTraceD("GDI.ReleaseDC: hwnd=%x hdc=%x\n", hwnd, hDC);
	if (dxw.IsRealDesktop(hwnd)) hwnd=dxw.GethWnd();

	if(dxw.dwFlags3 & EMULATEDC) 
		res=dxw.ReleaseEmulatedDC(hwnd);
	else
		res=(*pGDIReleaseDC)(hwnd, hDC);
	
	if (!res) OutTraceE("GDI.ReleaseDC ERROR: err=%d at %d\n", GetLastError(), __LINE__);
	return(res);
}

HDC WINAPI extBeginPaint(HWND hwnd, LPPAINTSTRUCT lpPaint)
{
	HDC hdc;
	extern HRESULT WINAPI extGetDC(LPDIRECTDRAWSURFACE, HDC FAR *);

	// proxy part ...
	OutTraceD("GDI.BeginPaint: hwnd=%x lpPaint=%x FullScreen=%x\n", hwnd, lpPaint, dxw.IsFullScreen());
	hdc=(*pBeginPaint)(hwnd, lpPaint);

	// if not in fullscreen mode, that's all!
	if(!dxw.IsFullScreen()) return hdc;

	if(dxw.dwFlags3 & EMULATEDC) return dxw.AcquireEmulatedDC(hwnd);  

	// on MAPGDITOPRIMARY, return the PrimHDC handle instead of the window DC
	if(dxw.dwFlags1 & MAPGDITOPRIMARY) {
		if(pGetDC && dxw.lpDDSPrimHDC){
			extGetDC(dxw.lpDDSPrimHDC,&PrimHDC);
			OutTraceD("GDI.BeginPaint: redirect hdc=%x -> PrimHDC=%x\n", hdc, PrimHDC);
			hdc=PrimHDC;
		}
		else {
			OutTraceD("GDI.BeginPaint: hdc=%x\n", hdc);
		}
	}

	// on CLIENTREMAPPING, resize the paint area to virtual screen size
	if(dxw.dwFlags1 & CLIENTREMAPPING){
		lpPaint->rcPaint.top=0;
		lpPaint->rcPaint.left=0;
		lpPaint->rcPaint.right=dxw.GetScreenWidth();
		lpPaint->rcPaint.bottom=dxw.GetScreenHeight();
	}

	return hdc;
}

BOOL WINAPI extEndPaint(HWND hwnd, const PAINTSTRUCT *lpPaint)
{
	BOOL ret;
	HRESULT WINAPI extReleaseDC(LPDIRECTDRAWSURFACE lpdds, HDC FAR hdc);

	OutTraceD("GDI.EndPaint: hwnd=%x lpPaint=%x\n", hwnd, lpPaint);

	if((dxw.dwFlags3 & EMULATEDC) && dxw.IsFullScreen()){
		ret=dxw.ReleaseEmulatedDC(hwnd);
		ret=(*pEndPaint)(hwnd, lpPaint);
		OutTraceD("GDI.EndPaint: hwnd=%x ret=%x\n", hwnd, ret);
		if(!ret) OutTraceE("GDI.EndPaint ERROR: err=%d at %d\n", GetLastError(), __LINE__);
		return ret;
	}

	// proxy part ...
	ret=(*pEndPaint)(hwnd, lpPaint);
	OutTraceD("GDI.EndPaint: hwnd=%x ret=%x\n", hwnd, ret);
	if(!ret) OutTraceE("GDI.EndPaint ERROR: err=%d at %d\n", GetLastError(), __LINE__);

	//return ret;

	// if not in fullscreen mode, that's all!
	if(!dxw.IsFullScreen()) return ret;

	// v2.02.09: on MAPGDITOPRIMARY, release the PrimHDC handle 
	if(dxw.dwFlags1 & MAPGDITOPRIMARY) {
		if(pReleaseDC && dxw.lpDDSPrimHDC){
			extReleaseDC(dxw.lpDDSPrimHDC, PrimHDC);
			OutTraceD("GDI.EndPaint: released hdc=%x\n", PrimHDC);
		}
	}
	
	return ret;
}

HWND WINAPI extCreateDialogIndirectParam(HINSTANCE hInstance, LPCDLGTEMPLATE lpTemplate, HWND hWndParent, DLGPROC lpDialogFunc, LPARAM lParamInit)
{
	HWND RetHWND;
	isWithinDialog=TRUE;
	OutTraceD("CreateDialogIndirectParam: hInstance=%x lpTemplate=%s hWndParent=%x lpDialogFunc=%x lParamInit=%x\n",
		hInstance, "tbd", hWndParent, lpDialogFunc, lParamInit);
	if(hWndParent==NULL) hWndParent=dxw.GethWnd();
	RetHWND=(*pCreateDialogIndirectParam)(hInstance, lpTemplate, hWndParent, lpDialogFunc, lParamInit);

	WhndStackPush(RetHWND, (WNDPROC)lpDialogFunc);
	if(!(*pSetWindowLong)(RetHWND, DWL_DLGPROC, (LONG)extDialogWindowProc))
		OutTraceE("SetWindowLong: ERROR err=%d at %d\n", GetLastError(), __LINE__);

	OutTraceD("CreateDialogIndirectParam: hwnd=%x\n", RetHWND);
	isWithinDialog=FALSE;
	//if (IsDebug) EnumChildWindows(RetHWND, (WNDENUMPROC)TraceChildWin, (LPARAM)RetHWND);
	return RetHWND;
}

HWND WINAPI extCreateDialogParam(HINSTANCE hInstance, LPCTSTR lpTemplateName, HWND hWndParent, DLGPROC lpDialogFunc, LPARAM lParamInit)
{
	HWND RetHWND;
	isWithinDialog=TRUE;
	OutTraceD("CreateDialogParam: hInstance=%x lpTemplateName=%s hWndParent=%x lpDialogFunc=%x lParamInit=%x\n",
		hInstance, "tbd", hWndParent, lpDialogFunc, lParamInit);
	if(hWndParent==NULL) hWndParent=dxw.GethWnd();
	RetHWND=(*pCreateDialogParam)(hInstance, lpTemplateName, hWndParent, lpDialogFunc, lParamInit);

	WhndStackPush(RetHWND, (WNDPROC)lpDialogFunc);
	if(!(*pSetWindowLong)(RetHWND, DWL_DLGPROC, (LONG)extDialogWindowProc))
		OutTraceE("SetWindowLong: ERROR err=%d at %d\n", GetLastError(), __LINE__);

	OutTraceD("CreateDialogParam: hwnd=%x\n", RetHWND);
	isWithinDialog=FALSE;
	//if (IsDebug) EnumChildWindows(RetHWND, (WNDENUMPROC)TraceChildWin, (LPARAM)RetHWND);
	return RetHWND;
}

BOOL WINAPI extMoveWindow(HWND hwnd, int X, int Y, int nWidth, int nHeight, BOOL bRepaint)
{
	BOOL ret;
	OutTraceD("MoveWindow: hwnd=%x xy=(%d,%d) size=(%d,%d) repaint=%x indialog=%x fullscreen=%x\n",
		hwnd, X, Y, nWidth, nHeight, bRepaint, isWithinDialog, dxw.IsFullScreen());

	if(dxw.IsDesktop(hwnd)){
		// v2.1.93: happens in "Emergency Fighters for Life" ...
		// what is the meaning of this? is it related to video stretching?
		OutTraceD("MoveWindow: prevent moving desktop win\n");
		return TRUE;
	}

	if((hwnd==dxw.GethWnd()) || (hwnd==dxw.hParentWnd)){
		OutTraceD("MoveWindow: prevent moving main win\n");
		return TRUE;
	}

	if (dxw.IsFullScreen()){
		POINT upleft={0,0};
		RECT client;
		BOOL isChild;
		(*pClientToScreen)(dxw.GethWnd(),&upleft);
		(*pGetClientRect)(dxw.GethWnd(),&client);
		if ((*pGetWindowLong)(hwnd, GWL_STYLE) & WS_CHILD){
			isChild=TRUE;
			// child coordinate adjustement
			X = (X * client.right) / dxw.GetScreenWidth();
			Y = (Y * client.bottom) / dxw.GetScreenHeight();
			nWidth = (nWidth * client.right) / dxw.GetScreenWidth();
			nHeight = (nHeight * client.bottom) / dxw.GetScreenHeight();
		}
		else {
			isChild=FALSE;
			// regular win coordinate adjustement
			X = upleft.x + (X * client.right) / dxw.GetScreenWidth();
			Y = upleft.y + (Y * client.bottom) / dxw.GetScreenHeight();
			nWidth = (nWidth * client.right) / dxw.GetScreenWidth();
			nHeight = (nHeight * client.bottom) / dxw.GetScreenHeight();
		}
		OutTraceD("MoveWindow: DEBUG client=(%d,%d) screen=(%d,%d)\n",
			client.right, client.bottom, dxw.GetScreenWidth(), dxw.GetScreenHeight());
		OutTraceD("MoveWindow: hwnd=%x child=%x relocated to xy=(%d,%d) size=(%d,%d)\n",
			hwnd, isChild, X, Y, nWidth, nHeight);
	}
	else{
		if((X==0)&&(Y==0)&&(nWidth==dxw.GetScreenWidth())&&(nHeight==dxw.GetScreenHeight())){
			// evidently, this was supposed to be a fullscreen window....
			RECT screen;
			POINT upleft = {0,0};
			(*pGetClientRect)(dxw.GethWnd(),&screen);
			(*pClientToScreen)(dxw.GethWnd(),&upleft);
			X=upleft.x;
			Y=upleft.y;
			nWidth=screen.right;
			nHeight=screen.bottom;
			OutTraceD("MoveWindow: fixed BIG win pos=(%d,%d) size=(%d,%d)\n", X, Y, nWidth, nHeight);
		}
	}


	ret=(*pMoveWindow)(hwnd, X, Y, nWidth, nHeight, bRepaint);
	if(!ret) OutTraceE("MoveWindow: ERROR err=%d at %d\n", GetLastError(), __LINE__);
	return ret;
} 

int WINAPI extShowCursor(BOOL bShow)
{
	static int iFakeCounter;
	int ret;

	OutTraceC("ShowCursor: bShow=%x\n", bShow);
	if (bShow){
		if (dxw.dwFlags1 & HIDEHWCURSOR){
			iFakeCounter++;
			OutTraceC("ShowCursor: HIDEHWCURSOR ret=%x\n", iFakeCounter);
			return iFakeCounter;
		}
	}
	else {
		if (dxw.dwFlags2 & SHOWHWCURSOR){
			iFakeCounter--;
			OutTraceC("ShowCursor: SHOWHWCURSOR ret=%x\n", iFakeCounter);
			return iFakeCounter;
		}
	}
	ret=(*pShowCursor)(bShow);
	OutTraceC("ShowCursor: ret=%x\n", ret);
	return ret;
}

int WINAPI extDrawTextA(HDC hDC, LPCTSTR lpchText, int nCount, LPRECT lpRect, UINT uFormat)
{
	return 0;
}

int WINAPI extDrawTextExA(HDC hDC, LPCTSTR lpchText, int cchText, LPRECT lprc, UINT dwDTFormat, LPDRAWTEXTPARAMS lpDTParams)
{
	return 0;
}

BOOL WINAPI extDrawFocusRect(HDC hDC, const RECT *lprc)
{
	return TRUE;
}

BOOL WINAPI extScrollDC(HDC hDC, int dx, int dy, const RECT *lprcScroll, const RECT *lprcClip, HRGN hrgnUpdate, LPRECT lprcUpdate)
{
	return TRUE;
}

HWND WINAPI extGetTopWindow(HWND hwnd)
{
	HWND ret;
	OutTraceD("GetTopWindow: hwnd=%x fullscreen=%x\n", hwnd, dxw.IsFullScreen()); 
	// a fullscreen program is supposed to be always top Z-order on the desktop!
	ret = (dxw.IsFullScreen() && dxw.IsDesktop(hwnd)) ? dxw.GethWnd() : (*pGetTopWindow)(hwnd);
	OutTraceD("GetTopWindow: ret=%x\n", ret); 
	return ret;
}

LONG WINAPI extTabbedTextOutA(HDC hdc, int X, int Y, LPCTSTR lpString, int nCount, int nTabPositions, const LPINT lpnTabStopPositions, int nTabOrigin)
{
	BOOL res;
	OutTraceD("TabbedTextOut: hdc=%x xy=(%d,%d) nCount=%d nTP=%d nTOS=%d str=(%d)\"%s\"\n", 
		hdc, X, Y, nCount, nTabPositions, nTabOrigin, lpString);

	if (dxw.IsFullScreen() && (OBJ_DC == GetObjectType(hdc))){
		dxw.MapClient(&X, &Y);
		OutTraceD("TextOut: fixed dest=(%d,%d)\n", X, Y);
	}

	res=(*pTabbedTextOutA)(hdc, X, Y, lpString, nCount, nTabPositions, lpnTabStopPositions, nTabOrigin);
	return res;
}

BOOL WINAPI extDestroyWindow(HWND hWnd)
{
	BOOL res;
	OutTraceB("DestroyWindow: hwnd=%x\n", hWnd);
	if (hWnd == dxw.GethWnd()) {
		OutTraceD("DestroyWindow: destroy main hwnd=%x\n", hWnd);
		if(1) return 0;
		dxw.SethWnd(NULL);
	}
	res=(*pDestroyWindow)(hWnd);
	if(!res)OutTraceE("DestroyWindow: ERROR err=%d\n", GetLastError());
	return res;
}

BOOL WINAPI extCloseWindow(HWND hWnd)
{
	BOOL res;
	OutTraceB("CloseWindow: hwnd=%x\n", hWnd);
	if (hWnd == dxw.GethWnd()) {
		OutTraceD("CloseWindow: close main hwnd=%x\n", hWnd);
		dxw.SethWnd(NULL);
	}
	res=(*pCloseWindow)(hWnd);
	if(!res)OutTraceE("CloseWindow: ERROR err=%d\n", GetLastError());
	return res;
}

BOOL WINAPI extSetSysColors(int cElements, const INT *lpaElements, const COLORREF *lpaRgbValues)
{
	// v2.02.32: added to avoid SysColors changes by "Western Front"
	BOOL ret;
	OutTraceD("SetSysColors: Elements=%d\n", cElements);

	if(dxw.dwFlags3 & LOCKSYSCOLORS) return TRUE;

	ret=(*pSetSysColors)(cElements, lpaElements, lpaRgbValues);
	if(!ret) OutTraceE("SetSysColors: ERROR er=%d\n", GetLastError());
	return ret;
}

BOOL WINAPI extUpdateWindow(HWND hwnd)
{
	BOOL ret;
	OutTraceD("UpdateWindow: hwnd=%x\n", hwnd);

	if(dxw.IsRealDesktop(hwnd)){
		OutTraceD("UpdateWindow: remapping hwnd=%x->%x\n", hwnd, dxw.GethWnd());
		hwnd=dxw.GethWnd();
	}

	ret=(*pUpdateWindow)(hwnd);
	if(!ret) OutTraceE("UpdateWindow: ERROR er=%d\n", GetLastError());
	return ret;
}

BOOL WINAPI extGetWindowPlacement(HWND hwnd, WINDOWPLACEMENT *lpwndpl)
{
	BOOL ret;
	OutTraceD("GetWindowPlacement: hwnd=%x\n", hwnd);

	if(dxw.IsRealDesktop(hwnd)){
		OutTraceD("GetWindowPlacement: remapping hwnd=%x->%x\n", hwnd, dxw.GethWnd());
		hwnd=dxw.GethWnd();
	}

	ret=(*pGetWindowPlacement)(hwnd, lpwndpl);
	OutTraceD("GetWindowPlacement: flags=%x showCmd=%x MinPosition=(%d,%d) MaxPosition=(%d,%d) NormalPosition=(%d,%d)-(%d,%d)\n",
		lpwndpl->flags, lpwndpl->showCmd, 
		lpwndpl->ptMinPosition.x, lpwndpl->ptMinPosition.y,
		lpwndpl->ptMaxPosition.x, lpwndpl->ptMaxPosition.y,
		lpwndpl->rcNormalPosition.left, lpwndpl->rcNormalPosition.top, lpwndpl->rcNormalPosition.right, lpwndpl->rcNormalPosition.bottom);

	switch (lpwndpl->showCmd){
	case SW_SHOW:
		if (dxw.IsFullScreen()){
			lpwndpl->showCmd = SW_MAXIMIZE;
			OutTraceD("GetWindowPlacement: forcing SW_MAXIMIZE state\n");
		}
		break;
	}
	if(!ret) OutTraceE("GetWindowPlacement: ERROR er=%d\n", GetLastError());
	return ret;
}

BOOL WINAPI extSetWindowPlacement(HWND hwnd, WINDOWPLACEMENT *lpwndpl)
{
	BOOL ret;
	OutTraceD("SetWindowPlacement: hwnd=%x\n", hwnd);

	if(dxw.IsRealDesktop(hwnd)){
		OutTraceD("SetWindowPlacement: remapping hwnd=%x->%x\n", hwnd, dxw.GethWnd());
		hwnd=dxw.GethWnd();
	}

	OutTraceD("SetWindowPlacement: flags=%x showCmd=%x MinPosition=(%d,%d) MaxPosition=(%d,%d) NormalPosition=(%d,%d)-(%d,%d)\n",
		lpwndpl->flags, lpwndpl->showCmd, 
		lpwndpl->ptMinPosition.x, lpwndpl->ptMinPosition.y,
		lpwndpl->ptMaxPosition.x, lpwndpl->ptMaxPosition.y,
		lpwndpl->rcNormalPosition.left, lpwndpl->rcNormalPosition.top, lpwndpl->rcNormalPosition.right, lpwndpl->rcNormalPosition.bottom);

	switch (lpwndpl->showCmd){
	case SW_SHOW:
		if (dxw.IsFullScreen()){
			lpwndpl->showCmd = SW_MAXIMIZE;
			OutTraceD("SetWindowPlacement: forcing SW_MAXIMIZE state\n");
		}
		break;
	}
	ret=(*pSetWindowPlacement)(hwnd, lpwndpl);
	if(!ret) OutTraceE("SetWindowPlacement: ERROR er=%d\n", GetLastError());
	return ret;
}

HWND WINAPI extSetCapture(HWND hwnd)
{
	HWND ret;
	OutTraceD("SetCapture: hwnd=%x\n", hwnd);
	ret=(*pSetCapture)(hwnd);
	OutTraceD("SetCapture: ret=%x\n", ret);
	return ret;
}