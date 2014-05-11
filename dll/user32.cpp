#define _WIN32_WINNT 0x0600
#define WIN32_LEAN_AND_MEAN
#define _CRT_SECURE_NO_WARNINGS
#define _CRT_NON_CONFORMING_SWPRINTFS

#include <stdio.h>
#include <stdlib.h>
#include "dxwnd.h"
#include "dxwcore.hpp"
#include "syslibs.h"
#include "dxhook.h"
#include "hddraw.h"
#include "dxhelper.h"

#define FIXCHILDSIZE TRUE

BOOL IsChangeDisplaySettingsHotPatched = FALSE;

//typedef BOOL (WINAPI *ValidateRect_Type)(HWND, const RECT *);
//BOOL WINAPI extValidateRect(HWND, const RECT *);
//ValidateRect_Type pValidateRect = NULL;

static HookEntry_Type Hooks[]={
	{HOOK_IAT_CANDIDATE, "UpdateWindow", (FARPROC)NULL, (FARPROC *)&pUpdateWindow, (FARPROC)extUpdateWindow},
	//{HOOK_IAT_CANDIDATE, "GetWindowPlacement", (FARPROC)NULL, (FARPROC *)&pGetWindowPlacement, (FARPROC)extGetWindowPlacement},
	//{HOOK_IAT_CANDIDATE, "SetWindowPlacement", (FARPROC)NULL, (FARPROC *)&pSetWindowPlacement, (FARPROC)extSetWindowPlacement},
	{HOOK_HOT_CANDIDATE, "ChangeDisplaySettingsA", (FARPROC)ChangeDisplaySettingsA, (FARPROC *)&pChangeDisplaySettingsA, (FARPROC)extChangeDisplaySettingsA},
	{HOOK_HOT_CANDIDATE, "ChangeDisplaySettingsExA", (FARPROC)ChangeDisplaySettingsExA, (FARPROC *)&pChangeDisplaySettingsExA, (FARPROC)extChangeDisplaySettingsExA},
	{HOOK_HOT_CANDIDATE, "ChangeDisplaySettingsW", (FARPROC)NULL, (FARPROC *)&pChangeDisplaySettingsW, (FARPROC)extChangeDisplaySettingsW}, // ref. by Knights of Honor
	{HOOK_HOT_CANDIDATE, "ChangeDisplaySettingsExW", (FARPROC)NULL, (FARPROC *)&pChangeDisplaySettingsExW, (FARPROC)extChangeDisplaySettingsExW},
	{HOOK_HOT_CANDIDATE, "GetMonitorInfoA", (FARPROC)NULL, (FARPROC *)&pGetMonitorInfoA, (FARPROC)extGetMonitorInfoA},
	{HOOK_HOT_CANDIDATE, "GetMonitorInfoW", (FARPROC)NULL, (FARPROC *)&pGetMonitorInfoW, (FARPROC)extGetMonitorInfoW},
	{HOOK_IAT_CANDIDATE, "ShowCursor", (FARPROC)ShowCursor, (FARPROC *)&pShowCursor, (FARPROC)extShowCursor},
	{HOOK_IAT_CANDIDATE, "CreateDialogIndirectParamA", (FARPROC)CreateDialogIndirectParamA, (FARPROC *)&pCreateDialogIndirectParam, (FARPROC)extCreateDialogIndirectParam},
	{HOOK_IAT_CANDIDATE, "CreateDialogParamA", (FARPROC)CreateDialogParamA, (FARPROC *)&pCreateDialogParam, (FARPROC)extCreateDialogParam},
	{HOOK_IAT_CANDIDATE, "MoveWindow", (FARPROC)MoveWindow, (FARPROC *)&pMoveWindow, (FARPROC)extMoveWindow},
	{HOOK_IAT_CANDIDATE, "EnumDisplaySettingsA", (FARPROC)EnumDisplaySettingsA, (FARPROC *)&pEnumDisplaySettings, (FARPROC)extEnumDisplaySettings},
	{HOOK_IAT_CANDIDATE, "GetClipCursor", (FARPROC)GetClipCursor, (FARPROC*)&pGetClipCursor, (FARPROC)extGetClipCursor},
	{HOOK_IAT_CANDIDATE, "ClipCursor", (FARPROC)ClipCursor, (FARPROC *)&pClipCursor, (FARPROC)extClipCursor},
	{HOOK_IAT_CANDIDATE, "DefWindowProcA", (FARPROC)DefWindowProcA, (FARPROC *)&pDefWindowProcA, (FARPROC)extDefWindowProcA},
	{HOOK_IAT_CANDIDATE, "DefWindowProcW", (FARPROC)DefWindowProcW, (FARPROC *)&pDefWindowProcW, (FARPROC)extDefWindowProcW},
	{HOOK_HOT_CANDIDATE, "CreateWindowExA", (FARPROC)CreateWindowExA, (FARPROC *)&pCreateWindowExA, (FARPROC)extCreateWindowExA},
	{HOOK_HOT_CANDIDATE, "CreateWindowExW", (FARPROC)CreateWindowExW, (FARPROC *)&pCreateWindowExW, (FARPROC)extCreateWindowExW},
	{HOOK_IAT_CANDIDATE, "RegisterClassExA", (FARPROC)RegisterClassExA, (FARPROC *)&pRegisterClassExA, (FARPROC)extRegisterClassExA},
	{HOOK_IAT_CANDIDATE, "RegisterClassA", (FARPROC)RegisterClassA, (FARPROC *)&pRegisterClassA, (FARPROC)extRegisterClassA},
	{HOOK_HOT_CANDIDATE, "GetSystemMetrics", (FARPROC)GetSystemMetrics, (FARPROC *)&pGetSystemMetrics, (FARPROC)extGetSystemMetrics},
	{HOOK_IAT_CANDIDATE, "GetDesktopWindow", (FARPROC)GetDesktopWindow, (FARPROC *)&pGetDesktopWindow, (FARPROC)extGetDesktopWindow},
	{HOOK_IAT_CANDIDATE, "CloseWindow", (FARPROC)NULL, (FARPROC *)&pCloseWindow, (FARPROC)extCloseWindow},
	{HOOK_IAT_CANDIDATE, "DestroyWindow", (FARPROC)NULL, (FARPROC *)&pDestroyWindow, (FARPROC)extDestroyWindow},
	{HOOK_IAT_CANDIDATE, "SetSysColors", (FARPROC)NULL, (FARPROC *)&pSetSysColors, (FARPROC)extSetSysColors},
	{HOOK_IAT_CANDIDATE, "SetCapture", (FARPROC)NULL, (FARPROC *)&pSetCapture, (FARPROC)extSetCapture},
	{HOOK_HOT_CANDIDATE, "SetWindowLongA", (FARPROC)SetWindowLongA, (FARPROC *)&pSetWindowLongA, (FARPROC)extSetWindowLongA},
	{HOOK_HOT_CANDIDATE, "GetWindowLongA", (FARPROC)GetWindowLongA, (FARPROC *)&pGetWindowLongA, (FARPROC)extGetWindowLongA}, 
	{HOOK_HOT_CANDIDATE, "SetWindowLongW", (FARPROC)SetWindowLongW, (FARPROC *)&pSetWindowLongW, (FARPROC)extSetWindowLongW},
	{HOOK_HOT_CANDIDATE, "GetWindowLongW", (FARPROC)GetWindowLongW, (FARPROC *)&pGetWindowLongW, (FARPROC)extGetWindowLongW}, 
	{HOOK_IAT_CANDIDATE, "IsWindowVisible", (FARPROC)NULL, (FARPROC *)&pIsWindowVisible, (FARPROC)extIsWindowVisible},
	{HOOK_IAT_CANDIDATE, "SystemParametersInfoA", (FARPROC)SystemParametersInfoA, (FARPROC *)&pSystemParametersInfoA, (FARPROC)extSystemParametersInfoA},
	{HOOK_IAT_CANDIDATE, "SystemParametersInfoW", (FARPROC)SystemParametersInfoW, (FARPROC *)&pSystemParametersInfoW, (FARPROC)extSystemParametersInfoW},
	//{HOOK_HOT_CANDIDATE, "GetActiveWindow", (FARPROC)NULL, (FARPROC *)&pGetActiveWindow, (FARPROC)extGetActiveWindow},
	//{HOOK_HOT_CANDIDATE, "GetForegroundWindow", (FARPROC)NULL, (FARPROC *)&pGetForegroundWindow, (FARPROC)extGetForegroundWindow},
	//{HOOK_IAT_CANDIDATE, "GetWindowTextA", (FARPROC)GetWindowTextA, (FARPROC *)&pGetWindowTextA, (FARPROC)extGetWindowTextA},
	{HOOK_IAT_CANDIDATE, 0, NULL, 0, 0} // terminator
};

static HookEntry_Type NoGDIHooks[]={
	{HOOK_IAT_CANDIDATE, "BeginPaint", (FARPROC)BeginPaint, (FARPROC *)&pBeginPaint, (FARPROC)extBeginPaint},
	{HOOK_IAT_CANDIDATE, "EndPaint", (FARPROC)EndPaint, (FARPROC *)&pEndPaint, (FARPROC)extEndPaint},
	{HOOK_IAT_CANDIDATE, 0, NULL, 0, 0} // terminator
};

static HookEntry_Type EmulateHooks[]={
	{HOOK_IAT_CANDIDATE, "BeginPaint", (FARPROC)BeginPaint, (FARPROC *)&pBeginPaint, (FARPROC)extEMUBeginPaint},
	{HOOK_IAT_CANDIDATE, "EndPaint", (FARPROC)EndPaint, (FARPROC *)&pEndPaint, (FARPROC)extEMUEndPaint},
	{HOOK_IAT_CANDIDATE, "GetDC", (FARPROC)GetDC, (FARPROC *)&pGDIGetDC, (FARPROC)extEMUGetDC},
	{HOOK_IAT_CANDIDATE, "GetDCEx", (FARPROC)GetDCEx, (FARPROC *)&pGDIGetDCEx, (FARPROC)extEMUGetDCEx},
	{HOOK_IAT_CANDIDATE, "GetWindowDC", (FARPROC)GetWindowDC, (FARPROC *)&pGDIGetWindowDC, (FARPROC)extEMUGetWindowDC}, 
	{HOOK_IAT_CANDIDATE, "ReleaseDC", (FARPROC)ReleaseDC, (FARPROC *)&pGDIReleaseDC, (FARPROC)extEMUReleaseDC},
	//{HOOK_IAT_CANDIDATE, "InvalidateRect", (FARPROC)InvalidateRect, (FARPROC *)&pInvalidateRect, (FARPROC)extInvalidateRect},
	{HOOK_IAT_CANDIDATE, 0, NULL, 0, 0} // terminator
};

static HookEntry_Type DDHooks[]={
	{HOOK_IAT_CANDIDATE, "BeginPaint", (FARPROC)BeginPaint, (FARPROC *)&pBeginPaint, (FARPROC)extDDBeginPaint},
	{HOOK_IAT_CANDIDATE, "EndPaint", (FARPROC)EndPaint, (FARPROC *)&pEndPaint, (FARPROC)extDDEndPaint},
	{HOOK_IAT_CANDIDATE, "GetDC", (FARPROC)GetDC, (FARPROC *)&pGDIGetDC, (FARPROC)extDDGetDC},
	{HOOK_IAT_CANDIDATE, "GetDCEx", (FARPROC)GetDCEx, (FARPROC *)&pGDIGetDCEx, (FARPROC)extDDGetDCEx},
	{HOOK_IAT_CANDIDATE, "GetWindowDC", (FARPROC)GetWindowDC, (FARPROC *)&pGDIGetWindowDC, (FARPROC)extDDGetDC},
	{HOOK_IAT_CANDIDATE, "ReleaseDC", (FARPROC)ReleaseDC, (FARPROC *)&pGDIReleaseDC, (FARPROC)extDDReleaseDC},
	{HOOK_IAT_CANDIDATE, "InvalidateRect", (FARPROC)InvalidateRect, (FARPROC *)&pInvalidateRect, (FARPROC)extInvalidateRect},
	{HOOK_IAT_CANDIDATE, 0, NULL, 0, 0} // terminator
};

static HookEntry_Type ScaledHooks[]={
	{HOOK_IAT_CANDIDATE, "FrameRect", (FARPROC)NULL, (FARPROC *)&pFrameRect, (FARPROC)extFrameRect},
	{HOOK_IAT_CANDIDATE, "TabbedTextOutA", (FARPROC)TabbedTextOutA, (FARPROC *)&pTabbedTextOutA, (FARPROC)extTabbedTextOutA},
	{HOOK_IAT_CANDIDATE, "DrawTextA", (FARPROC)DrawTextA, (FARPROC *)&pDrawText, (FARPROC)extDrawTextA},
	{HOOK_IAT_CANDIDATE, "DrawTextExA", (FARPROC)DrawTextExA, (FARPROC *)&pDrawTextEx, (FARPROC)extDrawTextExA},
	{HOOK_IAT_CANDIDATE, "FillRect", (FARPROC)NULL, (FARPROC *)&pFillRect, (FARPROC)extFillRect},
	{HOOK_IAT_CANDIDATE, "BeginPaint", (FARPROC)BeginPaint, (FARPROC *)&pBeginPaint, (FARPROC)extBeginPaint},
	{HOOK_IAT_CANDIDATE, "EndPaint", (FARPROC)EndPaint, (FARPROC *)&pEndPaint, (FARPROC)extEndPaint},
	{HOOK_IAT_CANDIDATE, "GetDC", (FARPROC)GetDC, (FARPROC *)&pGDIGetDC, (FARPROC)extGDIGetDC},
	{HOOK_IAT_CANDIDATE, "GetDCEx", (FARPROC)NULL, (FARPROC *)&pGDIGetDCEx, (FARPROC)extGDIGetDCEx},
	{HOOK_IAT_CANDIDATE, "GetWindowDC", (FARPROC)GetWindowDC, (FARPROC *)&pGDIGetWindowDC, (FARPROC)extGDIGetWindowDC},
	{HOOK_IAT_CANDIDATE, "ReleaseDC", (FARPROC)ReleaseDC, (FARPROC *)&pGDIReleaseDC, (FARPROC)extGDIReleaseDC},
	{HOOK_IAT_CANDIDATE, "InvalidateRect", (FARPROC)InvalidateRect, (FARPROC *)&pInvalidateRect, (FARPROC)extInvalidateRect},
	//{HOOK_IAT_CANDIDATE, "ValidateRect", (FARPROC)ValidateRect, (FARPROC *)&pValidateRect, (FARPROC)extValidateRect},
	{HOOK_IAT_CANDIDATE, 0, NULL, 0, 0} // terminator
};

static HookEntry_Type RemapHooks[]={
	{HOOK_IAT_CANDIDATE, "ScreenToClient", (FARPROC)ScreenToClient, (FARPROC *)&pScreenToClient, (FARPROC)extScreenToClient},
	{HOOK_IAT_CANDIDATE, "ClientToScreen", (FARPROC)ClientToScreen, (FARPROC *)&pClientToScreen, (FARPROC)extClientToScreen},
	{HOOK_IAT_CANDIDATE, "GetClientRect", (FARPROC)GetClientRect, (FARPROC *)&pGetClientRect, (FARPROC)extGetClientRect},
	{HOOK_IAT_CANDIDATE, "GetWindowRect", (FARPROC)GetWindowRect, (FARPROC *)&pGetWindowRect, (FARPROC)extGetWindowRect},
	{HOOK_IAT_CANDIDATE, "MapWindowPoints", (FARPROC)MapWindowPoints, (FARPROC *)&pMapWindowPoints, (FARPROC)extMapWindowPoints},
	{HOOK_IAT_CANDIDATE, "GetUpdateRgn", (FARPROC)GetUpdateRgn, (FARPROC *)&pGetUpdateRgn, (FARPROC)extGetUpdateRgn},
	//{HOOK_IAT_CANDIDATE, "GetUpdateRect", (FARPROC)GetUpdateRect, (FARPROC *)&pGetUpdateRect, (FARPROC)extGetUpdateRect},
	//{HOOK_IAT_CANDIDATE, "RedrawWindow", (FARPROC)RedrawWindow, (FARPROC *)&pRedrawWindow, (FARPROC)extRedrawWindow},
	{HOOK_IAT_CANDIDATE, 0, NULL, 0, 0} // terminator
};

static HookEntry_Type PeekAllHooks[]={
	{HOOK_IAT_CANDIDATE, "PeekMessageA", (FARPROC)NULL, (FARPROC *)&pPeekMessage, (FARPROC)extPeekMessage},
	{HOOK_IAT_CANDIDATE, "PeekMessageW", (FARPROC)NULL, (FARPROC *)&pPeekMessage, (FARPROC)extPeekMessage},
	{HOOK_IAT_CANDIDATE, 0, NULL, 0, 0} // terminator
};

static HookEntry_Type MouseHooks[]={
	{HOOK_IAT_CANDIDATE, "GetCursorPos", (FARPROC)GetCursorPos, (FARPROC *)&pGetCursorPos, (FARPROC)extGetCursorPos},
	{HOOK_IAT_CANDIDATE, "GetCursorInfo", (FARPROC)GetCursorInfo, (FARPROC *)&pGetCursorInfo, (FARPROC)extGetCursorInfo},
	{HOOK_IAT_CANDIDATE, "SetCursor", (FARPROC)SetCursor, (FARPROC *)&pSetCursor, (FARPROC)extSetCursor},
	{HOOK_IAT_CANDIDATE, "SendMessageA", (FARPROC)SendMessageA, (FARPROC *)&pSendMessageA, (FARPROC)extSendMessageA}, 
	{HOOK_IAT_CANDIDATE, "SendMessageW", (FARPROC)SendMessageW, (FARPROC *)&pSendMessageW, (FARPROC)extSendMessageW}, 
	//{HOOK_IAT_CANDIDATE, "SetPhysicalCursorPos", NULL, (FARPROC *)&pSetCursor, (FARPROC)extSetCursor}, // ???
	{HOOK_IAT_CANDIDATE, 0, NULL, 0, 0} // terminator
};

static HookEntry_Type WinHooks[]={
	{HOOK_HOT_CANDIDATE, "ShowWindow", (FARPROC)ShowWindow, (FARPROC *)&pShowWindow, (FARPROC)extShowWindow},
	{HOOK_HOT_CANDIDATE, "SetWindowPos", (FARPROC)SetWindowPos, (FARPROC *)&pSetWindowPos, (FARPROC)extSetWindowPos},
	{HOOK_HOT_CANDIDATE, "DeferWindowPos", (FARPROC)DeferWindowPos, (FARPROC *)&pGDIDeferWindowPos, (FARPROC)extDeferWindowPos},
	{HOOK_HOT_CANDIDATE, "CallWindowProcA", (FARPROC)CallWindowProcA, (FARPROC *)&pCallWindowProc, (FARPROC)extCallWindowProc},
	{HOOK_IAT_CANDIDATE, 0, NULL, 0, 0} // terminator
};

static HookEntry_Type MouseHooks2[]={
	{HOOK_HOT_CANDIDATE, "SetCursorPos", (FARPROC)SetCursorPos, (FARPROC *)&pSetCursorPos, (FARPROC)extSetCursorPos},
	{HOOK_IAT_CANDIDATE, 0, NULL, 0, 0} // terminator
};

static HookEntry_Type TimeHooks[]={
	{HOOK_IAT_CANDIDATE, "SetTimer", (FARPROC)SetTimer, (FARPROC *)&pSetTimer, (FARPROC)extSetTimer},
	{HOOK_IAT_CANDIDATE, "KillTimer", (FARPROC)KillTimer, (FARPROC *)&pKillTimer, (FARPROC)extKillTimer},
	{HOOK_IAT_CANDIDATE, 0, NULL, 0, 0} // terminator
};

FARPROC Remap_user32_ProcAddress(LPCSTR proc, HMODULE hModule)
{
	FARPROC addr;
	if (addr=RemapLibrary(proc, hModule, Hooks)) return addr;
	if (dxw.dwFlags1 & CLIENTREMAPPING) if (addr=RemapLibrary(proc, hModule, RemapHooks)) return addr;

	if (dxw.dwFlags2 & GDISTRETCHED)	if (addr=RemapLibrary(proc, hModule, ScaledHooks)) return addr;  
	if (dxw.dwFlags3 & GDIEMULATEDC)	if (addr=RemapLibrary(proc, hModule, EmulateHooks)) return addr; 
	if (dxw.dwFlags1 & MAPGDITOPRIMARY) if (addr=RemapLibrary(proc, hModule, DDHooks)) return addr; 
	if (!(dxw.dwFlags2 & GDISTRETCHED) && !(dxw.dwFlags3 & GDIEMULATEDC) && !(dxw.dwFlags1 & MAPGDITOPRIMARY))
		if (addr=RemapLibrary(proc, hModule, NoGDIHooks)) return addr;  

	if (dxw.dwFlags1 & MODIFYMOUSE)		if (addr=RemapLibrary(proc, hModule, MouseHooks)) return addr;
	if (dxw.dwFlags1 & (PREVENTMAXIMIZE|FIXWINFRAME|LOCKWINPOS|LOCKWINSTYLE))
		if (addr=RemapLibrary(proc, hModule, WinHooks)) return addr;
	if ((dxw.dwFlags1 & (MODIFYMOUSE|SLOWDOWN|KEEPCURSORWITHIN)) || (dxw.dwFlags2 & KEEPCURSORFIXED))
		if (addr=RemapLibrary(proc, hModule, MouseHooks2)) return addr;
	if (dxw.dwFlags3 & PEEKALLMESSAGES)
		if (addr=RemapLibrary(proc, hModule, PeekAllHooks)) return addr;
	if((dxw.dwFlags2 & TIMESTRETCH) && (dxw.dwFlags4 & STRETCHTIMERS)) 
		if (addr=RemapLibrary(proc, hModule, TimeHooks)) return addr;

	return NULL;
}

static char *libname = "user32.dll";

void HookUser32(HMODULE hModule)
{
	HookLibrary(hModule, Hooks, libname);
	if (!(dxw.dwFlags2 & GDISTRETCHED) && !(dxw.dwFlags3 & GDIEMULATEDC) && !(dxw.dwFlags1 & MAPGDITOPRIMARY))
		HookLibrary(hModule, NoGDIHooks, libname);
	if (dxw.dwFlags3 & GDIEMULATEDC)	HookLibrary(hModule, EmulateHooks, libname);
	if (dxw.dwFlags2 & GDISTRETCHED)	HookLibrary(hModule, ScaledHooks, libname);
	if (dxw.dwFlags1 & MAPGDITOPRIMARY) HookLibrary(hModule, DDHooks, libname);

	if (dxw.dwFlags1 & CLIENTREMAPPING) HookLibrary(hModule, RemapHooks, libname);
	if (dxw.dwFlags1 & MODIFYMOUSE) HookLibrary(hModule, MouseHooks, libname);
	if (dxw.dwFlags1 & (PREVENTMAXIMIZE|FIXWINFRAME|LOCKWINPOS|LOCKWINSTYLE)) HookLibrary(hModule, WinHooks, libname);
	if ((dxw.dwFlags1 & (MODIFYMOUSE|SLOWDOWN|KEEPCURSORWITHIN)) || (dxw.dwFlags2 & KEEPCURSORFIXED)) HookLibrary(hModule, MouseHooks2, libname);
	if (dxw.dwFlags3 & PEEKALLMESSAGES) HookLibrary(hModule, PeekAllHooks, libname);
	if (dxw.dwFlags2 & TIMESTRETCH) HookLibrary(hModule, TimeHooks, libname);

	IsChangeDisplaySettingsHotPatched = IsHotPatched(Hooks, "ChangeDisplaySettingsExA") || IsHotPatched(Hooks, "ChangeDisplaySettingsExW");
	return;
}

void HookUser32Init()
{
	HookLibInit(Hooks);
	HookLibInit(DDHooks);
	HookLibInit(RemapHooks);
	HookLibInit(MouseHooks);
	HookLibInit(WinHooks);
	HookLibInit(MouseHooks2);
}

/* ------------------------------------------------------------------------------ */
// auxiliary (static) functions
/* ------------------------------------------------------------------------------ */

static void Stopper(char *s, int line)
{
	char sMsg[81];
	sprintf(sMsg,"break: \"%s\"", s);
	MessageBox(0, sMsg, "break", MB_OK | MB_ICONEXCLAMATION);
}

//#define STOPPER_TEST // comment out to eliminate
#ifdef STOPPER_TEST
#define STOPPER(s) Stopper(s, __LINE__)
#else
#define STOPPER(s)
#endif

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
//extern void FixWindowFrame(HWND);
extern HRESULT WINAPI sBlt(char *, LPDIRECTDRAWSURFACE, LPRECT, LPDIRECTDRAWSURFACE, LPRECT, DWORD, LPDDBLTFX, BOOL);

LONG WINAPI MyChangeDisplaySettings(char *fname, BOOL WideChar, void *lpDevMode, DWORD dwflags)
{
	HRESULT res;
	DWORD dmFields, dmBitsPerPel, dmPelsWidth, dmPelsHeight;

	// v2.02.32: reset the emulated DC used in GDIEMULATEDC mode
	dxw.ResetEmulatedDC();

	if(lpDevMode){
		if(WideChar){
			dmFields=((DEVMODEW *)lpDevMode)->dmFields;
			dmPelsWidth=((DEVMODEW *)lpDevMode)->dmPelsWidth;
			dmPelsHeight=((DEVMODEW *)lpDevMode)->dmPelsHeight;
			dmBitsPerPel=((DEVMODEW *)lpDevMode)->dmBitsPerPel;
		}
		else{
			dmFields=((DEVMODEA *)lpDevMode)->dmFields;
			dmPelsWidth=((DEVMODEA *)lpDevMode)->dmPelsWidth;
			dmPelsHeight=((DEVMODEA *)lpDevMode)->dmPelsHeight;
			dmBitsPerPel=((DEVMODEA *)lpDevMode)->dmBitsPerPel;
		}
	}

	// save desired settings first v.2.1.89
	// v2.1.95 protect when lpDevMode is null (closing game... Jedi Outcast)
	// v2.2.23 consider new width/height only when dmFields flags are set.
	if(lpDevMode && (dmFields & (DM_PELSWIDTH | DM_PELSHEIGHT))){
		RECT client;
		dxw.SetScreenSize(dmPelsWidth, dmPelsHeight);

		// v2.02.31: when main win is bigger that expected resolution, you're in windowed fullscreen mode
		//(*pGetClientRect)((dxw.dwFlags1 & FIXPARENTWIN) ? dxw.hParentWnd : dxw.GethWnd(), &client);
		(*pGetClientRect)(dxw.GethWnd(), &client);
		OutTraceDW("%s: current hWnd=%x size=(%d,%d)\n", fname, dxw.GethWnd(), client.right, client.bottom);
		if((client.right>=(LONG)dmPelsWidth) && (client.bottom>=(LONG)dmPelsHeight)) {
			OutTraceDW("%s: entering FULLSCREEN mode\n", fname);
			dxw.SetFullScreen(TRUE);
		}
	}
	
	if ((dwflags==0 || dwflags==CDS_FULLSCREEN) && lpDevMode){
		if (dxw.dwFlags1 & EMULATESURFACE || !(dmFields & DM_BITSPERPEL)){
			OutTraceDW("%s: BYPASS res=DISP_CHANGE_SUCCESSFUL\n", fname);
			return DISP_CHANGE_SUCCESSFUL;
		}
		else{
			DEVMODEA NewMode;
			if(dwflags==CDS_FULLSCREEN) dwflags=0; // no FULLSCREEN
			(*pEnumDisplaySettings)(NULL, ENUM_CURRENT_SETTINGS, &NewMode);
			OutTraceDW("ChangeDisplaySettings: CURRENT wxh=(%dx%d) BitsPerPel=%d -> %d\n", 
				NewMode.dmPelsWidth, NewMode.dmPelsHeight, NewMode.dmBitsPerPel, dmBitsPerPel);
			NewMode.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;
			NewMode.dmBitsPerPel = dmBitsPerPel;
			res=(*pChangeDisplaySettingsExA)(NULL, &NewMode, NULL, 0, NULL);
			if(res) OutTraceE("ChangeDisplaySettings: ERROR err=%d at %d\n", GetLastError(), __LINE__);
			return res;
		}
	}
	else{
		if(WideChar)
			return (*pChangeDisplaySettingsExW)(NULL, (LPDEVMODEW)lpDevMode, NULL, dwflags, NULL);
		else
			return (*pChangeDisplaySettingsExA)(NULL, (LPDEVMODEA)lpDevMode, NULL, dwflags, NULL);
	}
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
	extern void CalculateWindowPos(HWND, DWORD, DWORD, LPWINDOWPOS);

	OutTraceDW("%s: GOT hwnd=%x pos=(%d,%d) dim=(%d,%d) Flags=%x(%s)\n", 
		ApiName, hwnd, wp->x, wp->y, wp->cx, wp->cy, wp->flags, ExplainWPFlags(wp->flags));

	if (dxw.dwFlags1 & PREVENTMAXIMIZE){
		int UpdFlag = 0;
		WINDOWPOS MaxPos;
		CalculateWindowPos(hwnd, MaxX, MaxY, &MaxPos);

		if(wp->cx>MaxPos.cx) { wp->cx=MaxPos.cx; UpdFlag=1; }
		if(wp->cy>MaxPos.cy) { wp->cy=MaxPos.cy; UpdFlag=1; }
		if (UpdFlag) 
			OutTraceDW("%s: SET max dim=(%d,%d)\n", ApiName, wp->cx, wp->cy);
	}

	if ((wp->flags & (SWP_NOMOVE|SWP_NOSIZE))==(SWP_NOMOVE|SWP_NOSIZE)) return; //v2.02.13

	if ((dxw.dwFlags1 & LOCKWINPOS) && dxw.IsFullScreen() && (hwnd==dxw.GethWnd())){ 
		CalculateWindowPos(hwnd, MaxX, MaxY, wp);
		OutTraceDW("%s: LOCK pos=(%d,%d) dim=(%d,%d)\n", ApiName, wp->x, wp->y, wp->cx, wp->cy);
	}

	if ((dxw.dwFlags2 & KEEPASPECTRATIO) && dxw.IsFullScreen() && (hwnd==dxw.GethWnd())){ 
		// note: while keeping aspect ration, resizing from one corner doesn't tell
		// which coordinate is prevalent to the other. We made an arbitrary choice.
		// note: v2.1.93: compensation must refer to the client area, not the wp
		// window dimensions that include the window borders.
		if(BorderX==-1){
			// v2.02.92: Fixed for AERO mode, where GetWindowRect substantially LIES!
			RECT client, full;
			LONG dwStyle, dwExStyle;
			HMENU hMenu;
			extern GetWindowLong_Type pGetWindowLongA;
			(*pGetClientRect)(hwnd, &client);
			full=client;
			dwStyle=(*pGetWindowLongA)(hwnd, GWL_STYLE);
			dwExStyle=(*pGetWindowLongA)(hwnd, GWL_EXSTYLE);
			hMenu = (dwStyle & WS_CHILD) ? NULL : GetMenu(hwnd);	
			AdjustWindowRectEx(&full, dwStyle, (hMenu!=NULL), dwExStyle);
			if (hMenu) __try {CloseHandle(hMenu);} __except(EXCEPTION_EXECUTE_HANDLER){};
			BorderX= full.right - full.left - client.right;
			BorderY= full.bottom - full.top - client.bottom;
			OutTraceDW("%s: KEEPASPECTRATIO window borders=(%d,%d)\n", ApiName, BorderX, BorderY);
		}
		extern LRESULT LastCursorPos;
		switch (LastCursorPos){
			case HTBOTTOM:
			case HTTOP:
			case HTBOTTOMLEFT:
			case HTBOTTOMRIGHT:
			case HTTOPLEFT:
			case HTTOPRIGHT:
				cx = BorderX + ((wp->cy - BorderY) * dxw.iRatioX) / dxw.iRatioY;
				if(cx!=wp->cx){
					OutTraceDW("%s: KEEPASPECTRATIO adjusted cx=%d->%d\n", ApiName, wp->cx, cx);
					wp->cx = cx;
				}
				break;
			case HTLEFT:
			case HTRIGHT:
				cy = BorderY + ((wp->cx - BorderX) * dxw.iRatioY) / dxw.iRatioX;
				if(cy!=wp->cy){
					OutTraceDW("%s: KEEPASPECTRATIO adjusted cy=%d->%d\n", ApiName, wp->cy, cy);
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
		OutTraceDW("%s: GOT MaxPosition=(%d,%d) MaxSize=(%d,%d)\n", ApiName, 
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

		// allow for initial dimensions ....
		//if(lpmmi->ptMaxSize.x < dxw.iSizX) lpmmi->ptMaxSize.x = dxw.iSizX;
		//if(lpmmi->ptMaxSize.y < dxw.iSizY) lpmmi->ptMaxSize.y = dxw.iSizY;

		OutTraceDW("%s: SET PREVENTMAXIMIZE MaxPosition=(%d,%d) MaxSize=(%d,%d)\n", ApiName, 
			lpmmi->ptMaxPosition.x, lpmmi->ptMaxPosition.y, lpmmi->ptMaxSize.x, lpmmi->ptMaxSize.y);
	}

	// v2.1.75: added logic to fix win coordinates to selected ones. 
	// fixes the problem with "Achtung Spitfire", that can't be managed through PREVENTMAXIMIZE flag.
	if (dxw.dwFlags1 & LOCKWINPOS){
		LPMINMAXINFO lpmmi;
		lpmmi=(LPMINMAXINFO)lParam;
		OutTraceDW("%s: GOT MaxPosition=(%d,%d) MaxSize=(%d,%d)\n", ApiName, 
			lpmmi->ptMaxPosition.x, lpmmi->ptMaxPosition.y, lpmmi->ptMaxSize.x, lpmmi->ptMaxSize.y);
		lpmmi->ptMaxPosition.x=dxw.iPosX;
		lpmmi->ptMaxPosition.y=dxw.iPosY;
		lpmmi->ptMaxSize.x = dxw.iSizX ? dxw.iSizX : dxw.GetScreenWidth();
		lpmmi->ptMaxSize.y = dxw.iSizY ? dxw.iSizY : dxw.GetScreenHeight();
		OutTraceDW("%s: SET LOCKWINPOS MaxPosition=(%d,%d) MaxSize=(%d,%d)\n", ApiName, 
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
	case WM_NCHITTEST:
		// v2.02.71 fix: when processing WM_NCHITTEST messages whith fixed coordinates avoid calling
		// the *pDefWindowProc call 
		// fixes "Microsoft Motocross Madness" mouse handling
		if((dxw.dwFlags2 & FIXNCHITTEST) && (dxw.dwFlags1 & MODIFYMOUSE)){ // mouse processing 
			OutTraceDW("%s: suppress WM_NCHITTEST\n", ApiName);
			return TRUE;
		}
		break;
	case WM_ERASEBKGND:
		OutTraceDW("%s: prevent erase background\n", ApiName);
		return TRUE; // 1=erased
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
	case WM_SIZE:
		if ((dxw.dwFlags1 & LOCKWINPOS) && dxw.IsFullScreen()) return 0;
		if (dxw.dwFlags1 & PREVENTMAXIMIZE){
			if ((wParam == SIZE_MAXIMIZED)||(wParam == SIZE_MAXSHOW)){
				OutTraceDW("%s: prevent screen SIZE to fullscreen wparam=%d(%s) size=(%d,%d)\n", ApiName,
					wParam, ExplainResizing(wParam), HIWORD(lParam), LOWORD(lParam));
				return 0; // checked
				//lParam = MAKELPARAM(dxw.GetScreenWidth(), dxw.GetScreenHeight()); 
				//OutTraceDW("%s: updated SIZE wparam=%d(%s) size=(%d,%d)\n", ApiName,
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

// --------------------------------------------------------------------------
//
// user32 API hookers
//
// --------------------------------------------------------------------------


BOOL WINAPI extInvalidateRect(HWND hwnd, RECT *lpRect, BOOL bErase)
{
	if(lpRect)
		OutTraceDW("InvalidateRect: hwnd=%x rect=(%d,%d)-(%d,%d) erase=%x\n",
		hwnd, lpRect->left, lpRect->top, lpRect->right, lpRect->bottom, bErase);
	else
		OutTraceDW("InvalidateRect: hwnd=%x rect=NULL erase=%x\n",
		hwnd, bErase);

	if(dxw.IsFullScreen() && dxw.IsRealDesktop(hwnd)) {
		hwnd=dxw.GethWnd();
		dxw.MapClient(lpRect);
		return (*pInvalidateRect)(hwnd, lpRect, bErase);
	}
	else{
		// just exagerate ...
		return (*pInvalidateRect)(hwnd, NULL, bErase);
	}
}

BOOL WINAPI extShowWindow(HWND hwnd, int nCmdShow)
{
	BOOL res;

	OutTraceDW("ShowWindow: hwnd=%x, CmdShow=%x(%s)\n", hwnd, nCmdShow, ExplainShowCmd(nCmdShow));
	if (dxw.dwFlags1 & PREVENTMAXIMIZE){
		if(nCmdShow==SW_MAXIMIZE){
			OutTraceDW("ShowWindow: suppress SW_MAXIMIZE maximize\n");
			nCmdShow=SW_SHOWNORMAL;
		}
		if(nCmdShow==SW_SHOWDEFAULT){
			STARTUPINFO StartupInfo;
			GetStartupInfo(&StartupInfo);
			OutTraceDW("DEBUG: StartupInfo dwFlags=%x ShowWindow=%x\n", StartupInfo.dwFlags, StartupInfo.wShowWindow);
			if((StartupInfo.dwFlags & STARTF_USESHOWWINDOW) && (StartupInfo.wShowWindow == SW_MAXIMIZE)){
				OutTraceDW("ShowWindow: suppress SW_SHOWDEFAULT maximize\n");
				nCmdShow=SW_SHOWNORMAL;
			}
		}
	}	

	res=(*pShowWindow)(hwnd, nCmdShow);

	return res;
}

LONG WINAPI extGetWindowLong(GetWindowLong_Type pGetWindowLong, char *ApiName, HWND hwnd, int nIndex)
{
	LONG res;

	res=(*pGetWindowLong)(hwnd, nIndex);

	OutTraceDW("%s: hwnd=%x, Index=%x(%s) res=%x\n", ApiName, hwnd, nIndex, ExplainSetWindowIndex(nIndex), res);

	if((nIndex==GWL_WNDPROC)||(nIndex==DWL_DLGPROC)){
		WNDPROC wp;
		wp=WinDBGetProc(hwnd);
		OutTraceDW("%s: remapping WindowProc res=%x -> %x\n", ApiName, res, (LONG)wp);
		if(wp) res=(LONG)wp; // if not found, don't alter the value.
	}

	return res;
}

LONG WINAPI extGetWindowLongA(HWND hwnd, int nIndex)
{
	return extGetWindowLong(pGetWindowLongA, "GetWindowLongA", hwnd, nIndex);
}

LONG WINAPI extGetWindowLongW(HWND hwnd, int nIndex)
{
	return extGetWindowLong(pGetWindowLongW, "GetWindowLongW", hwnd, nIndex);
}

LONG WINAPI extSetWindowLong(HWND hwnd, int nIndex, LONG dwNewLong, SetWindowLong_Type pSetWindowLong)
{
	LONG res;

	OutTraceDW("SetWindowLong: hwnd=%x, Index=%x(%s) Val=%x\n", 
		hwnd, nIndex, ExplainSetWindowIndex(nIndex), dwNewLong);

	if (dxw.Windowize){
		if(dxw.dwFlags1 & LOCKWINSTYLE){
			if(nIndex==GWL_STYLE){
				OutTraceDW("SetWindowLong: Lock GWL_STYLE=%x\n", dwNewLong);
				return (*pGetWindowLongA)(hwnd, nIndex);
			}
			if(nIndex==GWL_EXSTYLE){
				OutTraceDW("SetWindowLong: Lock GWL_EXSTYLE=%x\n", dwNewLong);
				return (*pGetWindowLongA)(hwnd, nIndex);
			}
		}

		if (dxw.dwFlags1 & PREVENTMAXIMIZE){
			if(nIndex==GWL_STYLE){
				dwNewLong &= ~WS_MAXIMIZE; 
				if(dxw.IsDesktop(hwnd)){
					OutTraceDW("SetWindowLong: GWL_STYLE %x suppress MAXIMIZE\n", dwNewLong);
					dwNewLong |= WS_OVERLAPPEDWINDOW; 
					dwNewLong &= ~(WS_DLGFRAME|WS_MAXIMIZE|WS_VSCROLL|WS_HSCROLL|WS_CLIPSIBLINGS); 
				}
			}
			// v2.02.32: disable topmost for main window only
			if(dxw.IsDesktop(hwnd) && (nIndex==GWL_EXSTYLE)){
				OutTraceDW("SetWindowLong: GWL_EXSTYLE %x suppress TOPMOST\n", dwNewLong);
				dwNewLong = dwNewLong & ~(WS_EX_TOPMOST); 
			}
		}

		if (dxw.dwFlags1 & FIXWINFRAME){
			//if((nIndex==GWL_STYLE) && !(dwNewLong & WS_CHILD)){
			if((nIndex==GWL_STYLE) && !(dwNewLong & WS_CHILD) && dxw.IsDesktop(hwnd)){
				OutTraceDW("SetWindowLong: GWL_STYLE %x force OVERLAPPEDWINDOW\n", dwNewLong);
				dwNewLong |= WS_OVERLAPPEDWINDOW; 
				dwNewLong &= ~WS_CLIPSIBLINGS; 
			}
		}
	}

	if (((nIndex==GWL_WNDPROC)||(nIndex==DWL_DLGPROC)) && dxw.IsFullScreen()){ // v2.02.51 - see A10 Cuba....
		WNDPROC lres;
		WNDPROC OldProc;
		// GPL fix
		if(dxw.IsRealDesktop(hwnd) && dxw.Windowize) {
			hwnd=dxw.GethWnd();
			OutTraceDW("SetWindowLong: DESKTOP hwnd, FIXING hwnd=%x\n",hwnd);
		}
		// end of GPL fix

		OldProc = (WNDPROC)(*pGetWindowLongA)(hwnd, nIndex);
		// v2.02.70 fix
		if((OldProc==extWindowProc) || 
			(OldProc==extChildWindowProc)||
			(OldProc==extDialogWindowProc)) 
			OldProc=WinDBGetProc(hwnd);
		WinDBPutProc(hwnd, (WNDPROC)dwNewLong);
		res=(LONG)OldProc;
		SetLastError(0);
		lres=(WNDPROC)(*pSetWindowLongA)(hwnd, nIndex, (LONG)extWindowProc);
		if(!lres && GetLastError())OutTraceE("SetWindowLong: ERROR err=%d at %d\n", GetLastError(), __LINE__);
	}
	else {
		res=(*pSetWindowLongA)(hwnd, nIndex, dwNewLong);
	}

	OutTraceDW("SetWindowLong: hwnd=%x, nIndex=%x, Val=%x, res=%x\n", hwnd, nIndex, dwNewLong, res);
	return res;
}

LONG WINAPI extSetWindowLongA(HWND hwnd, int nIndex, LONG dwNewLong)
{
	return extSetWindowLong(hwnd, nIndex, dwNewLong, pSetWindowLongA);
}

LONG WINAPI extSetWindowLongW(HWND hwnd, int nIndex, LONG dwNewLong)
{
	return extSetWindowLong(hwnd, nIndex, dwNewLong, pSetWindowLongW);
}

BOOL WINAPI extSetWindowPos(HWND hwnd, HWND hWndInsertAfter, int X, int Y, int cx, int cy, UINT uFlags)
{
	BOOL res;

	OutTraceDW("SetWindowPos: hwnd=%x%s pos=(%d,%d) dim=(%d,%d) Flags=%x\n", 
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
		OutTraceDW("SetWindowPos: locked position\n");
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
			OutTraceDW("SetWindowPos: using max dim=(%d,%d)\n", cx, cy);
	}

	// useful??? to be demonstrated....
	// when altering main window in fullscreen mode, fix the coordinates for borders
	DWORD dwCurStyle, dwExStyle;
	HMENU hMenu;
	RECT rect;
	rect.top=rect.left=0;
	rect.right=cx; rect.bottom=cy;
	dwCurStyle=(*pGetWindowLongA)(hwnd, GWL_STYLE);
	dwExStyle=(*pGetWindowLongA)(hwnd, GWL_EXSTYLE);
	// BEWARE: from MSDN -  If the window is a child window, the return value is undefined. 
	hMenu = (dwCurStyle & WS_CHILD) ? NULL : GetMenu(hwnd);	
	AdjustWindowRectEx(&rect, dwCurStyle, (hMenu!=NULL), dwExStyle);
	if (hMenu) CloseHandle(hMenu);
	cx=rect.right; cy=rect.bottom;
	OutTraceDW("SetWindowPos: main form hwnd=%x fixed size=(%d,%d)\n", hwnd, cx, cy);

	res=(*pSetWindowPos)(hwnd, hWndInsertAfter, X, Y, cx, cy, uFlags);
	if(!res)OutTraceE("SetWindowPos: ERROR err=%d at %d\n", GetLastError(), __LINE__);
	return res;
}

HDWP WINAPI extDeferWindowPos(HDWP hWinPosInfo, HWND hwnd, HWND hWndInsertAfter, int X, int Y, int cx, int cy, UINT uFlags)
{
	// v2.02.31: heavily used by "Imperialism II" !!!
	HDWP res;

	OutTraceDW("DeferWindowPos: hwnd=%x%s pos=(%d,%d) dim=(%d,%d) Flags=%x\n", 
		hwnd, dxw.IsFullScreen()?"(FULLSCREEN)":"", X, Y, cx, cy, uFlags);

	if(dxw.IsFullScreen()){
		dxw.MapClient(&X, &Y, &cx, &cy);
		OutTraceDW("DeferWindowPos: remapped pos=(%d,%d) dim=(%d,%d)\n", X, Y, cx, cy);
	}

	res=(*pGDIDeferWindowPos)(hWinPosInfo, hwnd, hWndInsertAfter, X, Y, cx, cy, uFlags);
	if(!res)OutTraceE("DeferWindowPos: ERROR err=%d at %d\n", GetLastError(), __LINE__);
	return res;
}

LRESULT WINAPI extSendMessage(char *apiname, SendMessage_Type pSendMessage, HWND hwnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	LRESULT ret;
	OutTraceW("%s: hwnd=%x WinMsg=[0x%x]%s(%x,%x)\n", 
		apiname, hwnd, Msg, ExplainWinMessage(Msg), wParam, lParam);

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
			OutTraceC("%s: hwnd=%x pos XY=(%d,%d)->(%d,%d)\n", apiname, hwnd, prev.x, prev.y, curr.x, curr.y);
			break;
		default:
			break;
		}
	}
	ret=(*pSendMessage)(hwnd, Msg, wParam, lParam);
	OutTraceW("%s: lresult=%x\n", apiname, ret); 
	return ret;
}

LRESULT WINAPI extSendMessageA(HWND hwnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	return extSendMessage("SendMessageA", pSendMessageA, hwnd, Msg, wParam, lParam);
}

LRESULT WINAPI extSendMessageW(HWND hwnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	return extSendMessage("SendMessageW", pSendMessageW, hwnd, Msg, wParam, lParam);
}

HCURSOR WINAPI extSetCursor(HCURSOR hCursor)
{
	HCURSOR ret;

	ret=(*pSetCursor)(hCursor);
	OutTraceDW("GDI.SetCursor: Cursor=%x, ret=%x\n", hCursor, ret);
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

	OutTraceC("SetCursorPos: res=%x XY=(%d,%d)->(%d,%d)\n", res, PrevX, PrevY, x, y);
	return res;
}

BOOL WINAPI extPeekMessage(LPMSG lpMsg, HWND hwnd, UINT wMsgFilterMin, UINT wMsgFilterMax, UINT wRemoveMsg)
{
	BOOL res;

	if((wMsgFilterMin==0) && (wMsgFilterMax == 0)){
		// no filtering, everything is good
		res=(*pPeekMessage)(lpMsg, hwnd, wMsgFilterMin, wMsgFilterMax, (wRemoveMsg & 0x000F));
	}
	else {
		MSG Dummy;
		// better eliminate all messages before and after the selected range !!!!
		//if(wMsgFilterMin)(*pPeekMessage)(&Dummy, hwnd, 0, wMsgFilterMin-1, TRUE);
		if(wMsgFilterMin>0x0F)(*pPeekMessage)(&Dummy, hwnd, 0x0F, wMsgFilterMin-1, TRUE);
		res=(*pPeekMessage)(lpMsg, hwnd, wMsgFilterMin, wMsgFilterMax, (wRemoveMsg & 0x000F));
		if(wMsgFilterMax<WM_KEYFIRST)(*pPeekMessage)(&Dummy, hwnd, wMsgFilterMax+1, WM_KEYFIRST-1, TRUE); // don't touch above WM_KEYFIRST !!!!
	}

	if(res)
		OutTraceW("PeekMessage: ANY lpmsg=%x hwnd=%x filter=(%x-%x) remove=%x(%s) msg=%x(%s) wparam=%x, lparam=%x pt=(%d,%d) res=%x\n", 
			lpMsg, lpMsg->hwnd, wMsgFilterMin, wMsgFilterMax, wRemoveMsg, ExplainPeekRemoveMsg(wRemoveMsg),
			lpMsg->message, ExplainWinMessage(lpMsg->message & 0xFFFF), 
			lpMsg->wParam, lpMsg->lParam, lpMsg->pt.x, lpMsg->pt.y, res);
	else
		OutTraceW("PeekMessage: ANY lpmsg=%x hwnd=%x filter=(%x-%x) remove=%x(%s) res=%x\n", 
			lpMsg, lpMsg->hwnd, wMsgFilterMin, wMsgFilterMax, wRemoveMsg, ExplainPeekRemoveMsg(wRemoveMsg), res);


	return res;
}

BOOL WINAPI extClientToScreen(HWND hwnd, LPPOINT lppoint)
{
	// v2.02.10: fully revised to handle scaled windows
	BOOL res;

	OutTraceB("ClientToScreen: hwnd=%x hWnd=%x FullScreen=%x point=(%d,%d)\n", 
		hwnd, dxw.GethWnd(), dxw.IsFullScreen(), lppoint->x, lppoint->y);
	if (lppoint && dxw.IsFullScreen()){
		// optimization: in fullscreen mode, coordinate conversion for the desktop window 
		// should always keep the same values inaltered
		if(hwnd != dxw.GethWnd()){
			*lppoint = dxw.AddCoordinates(*lppoint, dxw.ClientOffset(hwnd));
		}
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
		// optimization: in fullscreen mode, coordinate conversion for the desktop window 
		// should always keep the same values inaltered
		if(hwnd != dxw.GethWnd()){
			*lppoint = dxw.SubCoordinates(*lppoint, dxw.ClientOffset(hwnd));
			OutTraceB("ScreenToClient: FIXED point=(%d,%d)\n", lppoint->x, lppoint->y);
		}
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
		int w, h;
		if(FIXCHILDSIZE){
			if(WinDBGetSize(hwnd, &w, &h)){
				lpRect->top=lpRect->left=0;
				lpRect->right=w;
				lpRect->bottom=h;
				OutTraceB("GetClientRect: fixed rect=(%d,%d)-(%d,%d)\n", lpRect->left, lpRect->top, lpRect->right, lpRect->bottom);
				return TRUE;
			}
		}
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
//		if(lpRect->left > (LONG)dxw.GetScreenWidth()) lpRect->left=dxw.GetScreenWidth();
//		if(lpRect->right < 0) lpRect->right=0;
		if(lpRect->right > (LONG)dxw.GetScreenWidth()) lpRect->right=dxw.GetScreenWidth();
		if(lpRect->top < 0) lpRect->top=0;
//		if(lpRect->top > (LONG)dxw.GetScreenHeight()) lpRect->top=dxw.GetScreenHeight();
//		if(lpRect->bottom < 0) lpRect->bottom=0;
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

	OutTraceDW("MapWindowPoints: hWndFrom=%x%s hWndTo=%x%s cPoints=%d FullScreen=%x\n", 
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

	OutTraceDW("MapWindowPoints: ret=%x (%d,%d)\n", ret, (ret&0xFFFF0000)>>16, ret&0x0000FFFF);
	return ret;
}

HWND WINAPI extGetDesktopWindow(void)
{
	HWND res;

	if((!dxw.Windowize) || (dxw.dwFlags5 & DIABLOTWEAK)) return (*pGetDesktopWindow)();

	OutTraceDW("GetDesktopWindow: FullScreen=%x\n", dxw.IsFullScreen());
	if (dxw.IsFullScreen()){ 
#ifdef CREATEDESKTOP
		if(CREATEDESKTOP){
			extern HWND hDesktopWindow;
			OutTraceDW("GetDesktopWindow: returning desktop emulated hwnd=%x\n", hDesktopWindow);
			return hDesktopWindow;
		}
#endif
		OutTraceDW("GetDesktopWindow: returning main window hwnd=%x\n", dxw.GethWnd());
		return dxw.GethWnd();
	}
	else{
		res=(*pGetDesktopWindow)();
		OutTraceDW("GetDesktopWindow: returning desktop window hwnd=%x\n", res);
		return res;
	}
}

int WINAPI extGetSystemMetrics(int nindex)
{
	HRESULT res;

	res=(*pGetSystemMetrics)(nindex);
	OutTraceDW("GetSystemMetrics: index=%x(%s), res=%d\n", nindex, ExplainsSystemMetrics(nindex), res);

	if(!dxw.Windowize){
		// v2.02.95: if not in window mode, just implement the HIDEMULTIMONITOR flag
		if( (nindex ==SM_CMONITORS) &&
			(dxw.dwFlags2 & HIDEMULTIMONITOR) && 
			res>1) {
			res=1;
			OutTraceDW("GetSystemMetrics: fix SM_CMONITORS=%d\n", res);
		}
		return res;
	}

	// if you have a bypassed setting, use it first!
	if(pSetDevMode){
		switch(nindex){
		case SM_CXFULLSCREEN:
		case SM_CXSCREEN:
		case SM_CXVIRTUALSCREEN: // v2.02.31
			res = pSetDevMode->dmPelsWidth;
			OutTraceDW("GetDeviceCaps: fix HORZRES cap=%d\n", res);
			return res;
		case SM_CYFULLSCREEN:
		case SM_CYSCREEN:
		case SM_CYVIRTUALSCREEN: // v2.02.31
			res = pSetDevMode->dmPelsHeight;
			OutTraceDW("GetDeviceCaps: fix VERTRES cap=%d\n", res);
			return res;
		}
	}

	switch(nindex){
	case SM_CXFULLSCREEN:
	case SM_CXSCREEN:
	case SM_CXVIRTUALSCREEN: // v2.02.31
		res= dxw.GetScreenWidth();
		OutTraceDW("GetSystemMetrics: fix SM_CXSCREEN=%d\n", res);
		break;
	case SM_CYFULLSCREEN:
	case SM_CYSCREEN:
	case SM_CYVIRTUALSCREEN: // v2.02.31
		res= dxw.GetScreenHeight();
		OutTraceDW("GetSystemMetrics: fix SM_CYSCREEN=%d\n", res);
		break;
	case SM_CMONITORS:
		if((dxw.dwFlags2 & HIDEMULTIMONITOR) && res>1) {
			res=1;
			OutTraceDW("GetSystemMetrics: fix SM_CMONITORS=%d\n", res);
		}
		break;
	}

	return res;
}

ATOM WINAPI extRegisterClassExA(WNDCLASSEX *lpwcx)
{
	OutTraceDW("RegisterClassEx: PROXED ClassName=%s style=%x(%s) WndProc=%x cbClsExtra=%d cbWndExtra=%d hInstance=%x\n", 
		lpwcx->lpszClassName, lpwcx->style, ExplainStyle(lpwcx->style), lpwcx->lpfnWndProc, lpwcx->cbClsExtra, lpwcx->cbWndExtra, lpwcx->hInstance);
	return (*pRegisterClassExA)(lpwcx);
}

ATOM WINAPI extRegisterClassA(WNDCLASS *lpwcx)
{
	// referenced by Syberia, together with RegisterClassExA
	OutTraceDW("RegisterClass: PROXED ClassName=%s style=%x(%s) WndProc=%x cbClsExtra=%d cbWndExtra=%d hInstance=%x\n", 
		lpwcx->lpszClassName, lpwcx->style, ExplainStyle(lpwcx->style), lpwcx->lpfnWndProc, lpwcx->cbClsExtra, lpwcx->cbWndExtra, lpwcx->hInstance);
	return (*pRegisterClassA)(lpwcx);
}

static void HookChildWndProc(HWND hwnd, DWORD dwStyle, LPCTSTR ApiName)
{
	// child window inherit the father's windproc, so if it's redirected to
	// a hooker (either extWindowProc or extChildWindowProc) you have to retrieve
	// the correct value (WinDBGetProc) before saving it (WinDBPutProc).
	long res;
	WNDPROC pWindowProc;

	pWindowProc = (WNDPROC)(*pGetWindowLongA)(hwnd, GWL_WNDPROC);
	if((pWindowProc == extWindowProc) || 
		(pWindowProc == extChildWindowProc) ||
		(pWindowProc == extDialogWindowProc)){ // avoid recursions 
		HWND Father;
		WNDPROC pFatherProc;
		Father=GetParent(hwnd);
		pFatherProc=WinDBGetProc(Father);
		OutTraceDW("%s: WndProc=%s father=%x WndProc=%x\n", ApiName, 
			(pWindowProc == extWindowProc) ? "extWindowProc" : ((pWindowProc == extChildWindowProc) ? "extChildWindowProc" : "extDialogWindowProc"), 
			Father, pFatherProc);
		pWindowProc = pFatherProc;
	}
	WinDBPutProc(hwnd, pWindowProc);
	if(dwStyle & WS_CHILD){
		OutTraceDW("%s: Hooking CHILD hwnd=%x father WindowProc %x->%x\n", ApiName, hwnd, pWindowProc, extChildWindowProc);
		res=(*pSetWindowLongA)(hwnd, GWL_WNDPROC, (LONG)extChildWindowProc);
	}
	else { // must be dwStyle & WS_DLGFRAME
		OutTraceDW("%s: Hooking DLGFRAME hwnd=%x father WindowProc %x->%x\n", ApiName, hwnd, pWindowProc, extDialogWindowProc);
		res=(*pSetWindowLongA)(hwnd, GWL_WNDPROC, (LONG)extDialogWindowProc);
	}
	if(!res) OutTraceE("%s: SetWindowLong ERROR %x\n", ApiName, GetLastError());
}

HWND hControlParentWnd = NULL;

static HWND WINAPI extCreateWindowCommon(
  LPCTSTR ApiName,
  BOOL WideChar,
  DWORD dwExStyle,
  void *lpClassName,
  void *lpWindowName,
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
	HWND hwnd;
	BOOL isValidHandle=TRUE;
	int iOrigW, iOrigH;

	iOrigW=nWidth;
	iOrigH=nHeight;
	if(!dxw.Windowize || (hWndParent == HWND_MESSAGE)){ // v2.02.87: don't process message windows (hWndParent == HWND_MESSAGE)
		if(WideChar)
			hwnd= (*pCreateWindowExW)(dwExStyle, (LPCWSTR)lpClassName, (LPCWSTR)lpWindowName, dwStyle, x, y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam);
		else
			hwnd= (*pCreateWindowExA)(dwExStyle, (LPCSTR)lpClassName, (LPCSTR)lpWindowName, dwStyle, x, y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam);

		if ((dxw.dwFlags1 & HOOKCHILDWIN) && (dwStyle & (WS_CHILD|WS_DLGFRAME)))
			HookChildWndProc(hwnd, dwStyle, ApiName);

		OutTraceDW("%s: ret=%x\n", ApiName, hwnd);
		return hwnd;
	}

	// no maximized windows in any case
	if (dxw.dwFlags1 & PREVENTMAXIMIZE){
		OutTraceDW("%s: handling PREVENTMAXIMIZE mode\n", ApiName);
		dwStyle &= ~WS_MAXIMIZE;
	}

#ifdef CREATEDESKTOP
	if(CREATEDESKTOP){
		extern HWND hDesktopWindow;
		if (dxw.IsRealDesktop(hWndParent)){
			OutTraceE("%s: new parent win %x->%x\n", ApiName, hWndParent, hDesktopWindow);
			hWndParent=hDesktopWindow;
		}
	}
#endif

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
				((x<=0)&&(y<=0)) 
				|| 
				((x==CW_USEDEFAULT)&&(y==CW_USEDEFAULT))
			)
		&&
			(
				((nWidth>=(int)dxw.GetScreenWidth())&&(nHeight>=(int)dxw.GetScreenHeight()))
				||
				((nWidth==CW_USEDEFAULT)&&(nHeight==CW_USEDEFAULT)) // good for Imperialism, but is it general? 
			) 
		&&
			!(dwExStyle & WS_EX_CONTROLPARENT) // Diablo fix
		&&
			!(dwStyle & WS_CHILD) // Diablo fix
		) 
	{
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
			OutTraceDW("%s: fixed BIG win pos=(%d,%d) size=(%d,%d)\n", ApiName, x, y, nWidth, nHeight);
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
			OutTraceDW("%s: renewed BIG win pos=(%d,%d) size=(%d,%d)\n", ApiName, x, y, nWidth, nHeight);
		}
		dxw.SetFullScreen(TRUE);
		if(dxw.Coordinates==DXW_DESKTOP_WORKAREA){
			RECT workarea;
			(*pSystemParametersInfoA)(SPI_GETWORKAREA, NULL, &workarea, 0);
			x=0;
			y=0;
			nWidth=workarea.right;
			nHeight=workarea.bottom;
			dwStyle=0;
			OutTraceDW("%s: WORKAREA win pos=(%d,%d) size=(%d,%d)\n", ApiName, x, y, nWidth, nHeight);
		}
		else if(dxw.Coordinates==DXW_DESKTOP_FULL){
			RECT workarea;
			(*pGetClientRect)((*pGetDesktopWindow)(), &workarea);
			x=0;
			y=0;
			nWidth=workarea.right;
			nHeight=workarea.bottom;
			dwStyle=0;
			OutTraceDW("%s: FULLDESKTOP win pos=(%d,%d) size=(%d,%d)\n", ApiName, x, y, nWidth, nHeight);
		}
	}

	if(!dxw.IsFullScreen()){ // v2.1.63: needed for "Monster Truck Madness"
		if(WideChar)
			hwnd= (*pCreateWindowExW)(dwExStyle, (LPCWSTR)lpClassName, (LPCWSTR)lpWindowName, dwStyle, x, y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam);
		else
			hwnd= (*pCreateWindowExA)(dwExStyle, (LPCSTR)lpClassName, (LPCSTR)lpWindowName, dwStyle, x, y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam);
		OutTraceDW("%s: windowed mode ret=%x\n", ApiName, hwnd);
		return hwnd;
	}

	// tested on Gangsters: coordinates must be window-relative!!!
	// Age of Empires....
	if (dwStyle & WS_CHILD){ 
		dxw.MapClient(&x, &y, &nWidth, &nHeight);
		OutTraceDW("%s: fixed WS_CHILD pos=(%d,%d) size=(%d,%d)\n",
			ApiName, x, y, nWidth, nHeight);
	}
	// needed for Diablo, that creates a new control parent window that must be
	// overlapped to the directdraw surface.
	else if (dwExStyle & WS_EX_CONTROLPARENT){
		dxw.MapWindow(&x, &y, &nWidth, &nHeight);
		OutTraceDW("%s: fixed WS_EX_CONTROLPARENT pos=(%d,%d) size=(%d,%d)\n",
			ApiName, x, y, nWidth, nHeight);
	}

	OutTraceB("%s: fixed pos=(%d,%d) size=(%d,%d) Style=%x(%s) ExStyle=%x(%s)\n",
		ApiName, x, y, nWidth, nHeight, dwStyle, ExplainStyle(dwStyle), dwExStyle, ExplainExStyle(dwExStyle));

	if(WideChar)
		hwnd= (*pCreateWindowExW)(dwExStyle, (LPCWSTR)lpClassName, (LPCWSTR)lpWindowName, dwStyle, x, y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam);
	else
		hwnd= (*pCreateWindowExA)(dwExStyle, (LPCSTR)lpClassName, (LPCSTR)lpWindowName, dwStyle, x, y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam);
	if (hwnd==(HWND)NULL){
		OutTraceE("%s: ERROR err=%d Style=%x(%s) ExStyle=%x\n",
			ApiName, GetLastError(), dwStyle, ExplainStyle(dwStyle), dwExStyle);
		return hwnd;
	}

	if (dwExStyle & WS_EX_CONTROLPARENT) hControlParentWnd=hwnd;

	if ((!isValidHandle) && dxw.IsFullScreen()){
		dxw.SethWnd(hwnd);
		extern void AdjustWindowPos(HWND, DWORD, DWORD);
		(*pSetWindowLongA)(hwnd, GWL_STYLE, (dxw.dwFlags2 & MODALSTYLE) ? 0 : WS_OVERLAPPEDWINDOW);
		(*pSetWindowLongA)(hwnd, GWL_EXSTYLE, 0); 
		OutTraceDW("%s: hwnd=%x, set style=WS_OVERLAPPEDWINDOW extstyle=0\n", ApiName, hwnd); 
		AdjustWindowPos(hwnd, nWidth, nHeight);
		(*pShowWindow)(hwnd, SW_SHOWNORMAL);
	}

	//if ((dxw.dwFlags1 & FIXWINFRAME) && !(dwStyle & WS_CHILD))
	if ((dxw.dwFlags1 & FIXWINFRAME) && !(dwStyle & WS_CHILD) && dxw.IsDesktop(hwnd))
		dxw.FixWindowFrame(hwnd);

	if ((dxw.dwFlags1 & HOOKCHILDWIN) && (dwStyle & (WS_CHILD|WS_DLGFRAME)))
		HookChildWndProc(hwnd, dwStyle, ApiName);

	if ((FIXCHILDSIZE) && (dwStyle & (WS_CHILD|WS_DLGFRAME)))
		WinDBPutSize(hwnd, iOrigW, iOrigH);

	OutTraceDW("%s: ret=%x\n", ApiName, hwnd);
	return hwnd;
}

static LPCSTR ClassToStr(LPCSTR Class)
{
	static char AtomBuf[20+1];
	if(((DWORD)Class & 0xFFFF0000) == 0){
		sprintf(AtomBuf, "ATOM(%X)", (DWORD)Class);
		return AtomBuf;
	}
	return Class;
}

static LPCWSTR ClassToWStr(LPCWSTR Class)
{
	static WCHAR AtomBuf[20+1];
	if(((DWORD)Class & 0xFFFF0000) == 0){
		swprintf(AtomBuf, L"ATOM(%X)", (DWORD)Class);
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
	if(IsTraceDW){
		char xString[20], yString[20], wString[20], hString[20];
		if (x==CW_USEDEFAULT) strcpy(xString,"CW_USEDEFAULT"); 
		else sprintf(xString,"%d", x);
		if (y==CW_USEDEFAULT) strcpy(yString,"CW_USEDEFAULT"); 
		else sprintf(yString,"%d", y);
		if (nWidth==CW_USEDEFAULT) strcpy(wString,"CW_USEDEFAULT"); 
		else sprintf(wString,"%d", nWidth);
		if (nHeight==CW_USEDEFAULT) strcpy(hString,"CW_USEDEFAULT"); 
		else sprintf(hString,"%d", nHeight);
		OutTrace("CreateWindowExW: class=\"%ls\" wname=\"%ls\" pos=(%s,%s) size=(%s,%s) Style=%x(%s) ExStyle=%x(%s) hWndParent=%x%s hMenu=%x\n",
			ClassToWStr(lpClassName), lpWindowName, xString, yString, wString, hString, 
			dwStyle, ExplainStyle(dwStyle), dwExStyle, ExplainExStyle(dwExStyle),
			hWndParent, hWndParent==HWND_MESSAGE?"(HWND_MESSAGE)":"", hMenu);
	}
	if(IsDebug) OutTrace("CreateWindowExW: DEBUG screen=(%d,%d)\n", dxw.GetScreenWidth(), dxw.GetScreenHeight());

	return extCreateWindowCommon("CreateWindowExW", TRUE, dwExStyle, (void *)lpClassName, (void *)lpWindowName, dwStyle, x, y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam); 
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
	if(IsTraceDW){
		char xString[20], yString[20], wString[20], hString[20];
		if (x==CW_USEDEFAULT) strcpy(xString,"CW_USEDEFAULT"); 
		else sprintf(xString,"%d", x);
		if (y==CW_USEDEFAULT) strcpy(yString,"CW_USEDEFAULT"); 
		else sprintf(yString,"%d", y);
		if (nWidth==CW_USEDEFAULT) strcpy(wString,"CW_USEDEFAULT"); 
		else sprintf(wString,"%d", nWidth);
		if (nHeight==CW_USEDEFAULT) strcpy(hString,"CW_USEDEFAULT"); 
		else sprintf(hString,"%d", nHeight);
		OutTrace("CreateWindowExA: class=\"%s\" wname=\"%s\" pos=(%s,%s) size=(%s,%s) Style=%x(%s) ExStyle=%x(%s) hWndParent=%x%s hMenu=%x\n",
			ClassToStr(lpClassName), lpWindowName, xString, yString, wString, hString, 
			dwStyle, ExplainStyle(dwStyle), dwExStyle, ExplainExStyle(dwExStyle),
			hWndParent, hWndParent==HWND_MESSAGE?"(HWND_MESSAGE)":"", hMenu);
	}
	if(IsDebug) OutTrace("CreateWindowExA: DEBUG screen=(%d,%d)\n", dxw.GetScreenWidth(), dxw.GetScreenHeight());

	return extCreateWindowCommon("CreateWindowExA", FALSE, dwExStyle, (void *)lpClassName, (void *)lpWindowName, dwStyle, x, y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam); 
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

LRESULT WINAPI extDefWindowProcA(HWND hwnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	// v2.02.30: fix (Imperialism II): apply to main window only !!!
	HRESULT res;

	res = (HRESULT)-1;
	if(hwnd == dxw.GethWnd()) res=FixWindowProc("DefWindowProcA", hwnd, Msg, wParam, &lParam);

	if (res==(HRESULT)-1)
		return (*pDefWindowProcA)(hwnd, Msg, wParam, lParam);
	else
		return res;
}

LRESULT WINAPI extDefWindowProcW(HWND hwnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	// v2.02.30: fix (Imperialism II): apply to main window only !!!
	HRESULT res;

	res = (HRESULT)-1;
	if(hwnd == dxw.GethWnd()) res=FixWindowProc("DefWindowProcW", hwnd, Msg, wParam, &lParam);

	if (res==(HRESULT)-1)
		return (*pDefWindowProcW)(hwnd, Msg, wParam, lParam);
	else
		return res;
}

int WINAPI extFillRect(HDC hdc, const RECT *lprc, HBRUSH hbr)
{
	int res;
	RECT rc;
	OutTraceDW("FillRect: hdc=%x hbrush=%x rect=(%d,%d)-(%d,%d)\n", hdc, hbr, lprc->left, lprc->top, lprc->right, lprc->bottom);

	if(dxw.dwFlags4 & NOFILLRECT) {
		OutTraceDW("FillRect: SUPPRESS\n", hdc, hbr, lprc->left, lprc->top, lprc->right, lprc->bottom);
		return TRUE;
	}


	memcpy(&rc, lprc, sizeof(rc));

	if(dxw.IsRealDesktop(WindowFromDC(hdc))) {
		OutTraceDW("FillRect: remapped hdc to virtual desktop\n");
		hdc=(*pGDIGetDC)(dxw.GethWnd());
	}

	if(!dxw.IsFullScreen()) {
		// when not in fullscreen mode, just proxy the call
		// but check coordinates: some games may use excessive coordinates: see "Premier Manager 98"
		RECT client;
		(*pGetClientRect)(WindowFromDC(hdc), &client);
		if(rc.left < 0) rc.left=0;
		if(rc.top < 0) rc.top=0;
		if(rc.right > client.right) rc.right=client.right;
		if(rc.bottom > client.bottom) rc.bottom=client.bottom;
		return (*pFillRect)(hdc, &rc, hbr);
	}

	if(OBJ_DC == GetObjectType(hdc)){
		if(rc.left < 0) rc.left = 0;
		if(rc.top < 0) rc.top = 0;
		if((DWORD)rc.right > dxw.GetScreenWidth()) rc.right = dxw.GetScreenWidth();
		if((DWORD)rc.bottom > dxw.GetScreenHeight()) rc.bottom = dxw.GetScreenHeight();
		dxw.MapClient(&rc);
		//dxw.MapWindow(&rc);
		OutTraceDW("FillRect: fixed rect=(%d,%d)-(%d,%d)\n", rc.left, rc.top, rc.right, rc.bottom);
	}

	res=(*pFillRect)(hdc, &rc, hbr);
	return res;
}

int WINAPI extFrameRect(HDC hdc, const RECT *lprc, HBRUSH hbr)
{
	int res;
	RECT rc;
	OutTraceDW("FrameRect: hdc=%x hbrush=%x rect=(%d,%d)-(%d,%d)\n", hdc, hbr, lprc->left, lprc->top, lprc->right, lprc->bottom);

	// when not in fullscreen mode, just proxy the call
	if(!dxw.IsFullScreen()) return (*pFrameRect)(hdc, lprc, hbr);

	memcpy(&rc, lprc, sizeof(rc));
	if(OBJ_DC == GetObjectType(hdc)){
		if(rc.left < 0) rc.left = 0;
		if(rc.top < 0) rc.top = 0;
		if((DWORD)rc.right > dxw.GetScreenWidth()) rc.right = dxw.GetScreenWidth();
		if((DWORD)rc.bottom > dxw.GetScreenHeight()) rc.bottom = dxw.GetScreenHeight();
		dxw.MapClient(&rc);
		OutTraceDW("FrameRect: fixed rect=(%d,%d)-(%d,%d)\n", rc.left, rc.top, rc.right, rc.bottom);
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
		// v2.02.39: fix - do not attempt to write to NULL lpRect
		if (lpRect) {
			ClipRegion=*lpRectArg;
			lpClipRegion=&ClipRegion;
			*lpRect=dxw.MapWindowRect(lpRect);
		}
		else
			lpClipRegion=NULL;
	}

	if (pClipCursor) res=(*pClipCursor)(lpRect);
	if (lpRect) OutTraceDW("ClipCursor: REMAPPED rect=(%d,%d)-(%d,%d) res=%x\n", lpRect->left,lpRect->top,lpRect->right,lpRect->bottom, res);

	return TRUE;
}

BOOL WINAPI extGetClipCursor(LPRECT lpRect)
{
	// v2.1.93: if ENABLECLIPPING, return the saved clip rect coordinates

	BOOL ret;

	// proxy....
	if (!(dxw.dwFlags1 & ENABLECLIPPING)) {
		ret=(*pGetClipCursor)(lpRect);
		if(IsTraceDDRAW){
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
		OutTraceDW("GetClipCursor: rect=(%d,%d)-(%d,%d) ret=%d\n", 
			lpRect->left,lpRect->top,lpRect->right,lpRect->bottom, TRUE);
	}

	return TRUE;
}

LONG WINAPI extEnumDisplaySettings(LPCTSTR lpszDeviceName, DWORD iModeNum, DEVMODE *lpDevMode)
{
	LONG res;
	OutTraceDW("EnumDisplaySettings: Devicename=%s ModeNum=%x\n", lpszDeviceName, iModeNum);
	if(pSetDevMode && iModeNum==ENUM_CURRENT_SETTINGS){
		lpDevMode=pSetDevMode;
		return 1;
	}
	else{
		res=(*pEnumDisplaySettings)(lpszDeviceName, iModeNum, lpDevMode);
		if(dxw.dwFlags4 & LIMITSCREENRES){
			#define HUGE 100000
			DWORD maxw, maxh;
			maxw = maxh = HUGE;
			switch(dxw.MaxScreenRes){
				case DXW_NO_LIMIT: maxw=HUGE; maxh=HUGE; break;
				case DXW_LIMIT_320x200: maxw=320; maxh=200; break;
				case DXW_LIMIT_640x480: maxw=640; maxh=480; break;
				case DXW_LIMIT_800x600: maxw=800; maxh=600; break;
				case DXW_LIMIT_1024x768: maxw=1024; maxh=768; break;
				case DXW_LIMIT_1280x960: maxw=1280; maxh=960; break;
			}
			if((lpDevMode->dmPelsWidth > maxw) || (lpDevMode->dmPelsHeight > maxh)){
				OutTraceDW("EnumDisplaySettings: limit device size=(%d,%d)\n", maxw, maxh);
				lpDevMode->dmPelsWidth = maxw;
				lpDevMode->dmPelsHeight = maxh;
			}
		}
		return res;
	}
}

LONG WINAPI extChangeDisplaySettingsA(DEVMODEA *lpDevMode, DWORD dwflags)
{
	if(IsTraceDDRAW){
		OutTrace("ChangeDisplaySettingsA: lpDevMode=%x flags=%x(%s)", lpDevMode, dwflags, ExplainChangeDisplaySettingsFlags(dwflags));
		if (lpDevMode) OutTrace(" DeviceName=%s fields=%x(%s) size=(%d x %d) bpp=%d", 
			lpDevMode->dmDeviceName, lpDevMode->dmFields, ExplainDevModeFields(lpDevMode->dmFields),
			lpDevMode->dmPelsWidth, lpDevMode->dmPelsHeight, lpDevMode->dmBitsPerPel);
		OutTrace("\n");
	}

	if(dxw.Windowize)
		return MyChangeDisplaySettings("ChangeDisplaySettingsA", FALSE, lpDevMode, dwflags);
	else
		return (*pChangeDisplaySettingsExA)(NULL, lpDevMode, NULL, dwflags, NULL);
}

LONG WINAPI extChangeDisplaySettingsW(DEVMODEW *lpDevMode, DWORD dwflags)
{
	if(IsTraceDDRAW){
		OutTrace("ChangeDisplaySettingsW: lpDevMode=%x flags=%x(%s)", lpDevMode, dwflags, ExplainChangeDisplaySettingsFlags(dwflags));
		if (lpDevMode) OutTrace(" DeviceName=%ls fields=%x(%s) size=(%d x %d) bpp=%d", 
			lpDevMode->dmDeviceName, lpDevMode->dmFields, ExplainDevModeFields(lpDevMode->dmFields),
			lpDevMode->dmPelsWidth, lpDevMode->dmPelsHeight, lpDevMode->dmBitsPerPel);
		OutTrace("\n");
	}

	if(dxw.Windowize)
		return MyChangeDisplaySettings("ChangeDisplaySettingsW", TRUE, lpDevMode, dwflags);
	else
		return (*pChangeDisplaySettingsW)(lpDevMode, dwflags);
}

LONG WINAPI extChangeDisplaySettingsExA(LPCTSTR lpszDeviceName, DEVMODEA *lpDevMode, HWND hwnd, DWORD dwflags, LPVOID lParam)
{
	if(IsTraceDDRAW){
		OutTrace("ChangeDisplaySettingsExA: DeviceName=%s lpDevMode=%x flags=%x(%s)", lpszDeviceName, lpDevMode, dwflags, ExplainChangeDisplaySettingsFlags(dwflags));
		if (lpDevMode) OutTrace(" DeviceName=%s fields=%x(%s) size=(%d x %d) bpp=%d", 
			lpDevMode->dmDeviceName, lpDevMode->dmFields, ExplainDevModeFields(lpDevMode->dmFields),
			lpDevMode->dmPelsWidth, lpDevMode->dmPelsHeight, lpDevMode->dmBitsPerPel);
		OutTrace("\n");
	}

	if(dxw.Windowize)
		return MyChangeDisplaySettings("ChangeDisplaySettingsExA", FALSE, lpDevMode, dwflags);
	else
		return (*pChangeDisplaySettingsExA)(lpszDeviceName, lpDevMode, hwnd, dwflags, lParam);
}

LONG WINAPI extChangeDisplaySettingsExW(LPCTSTR lpszDeviceName, DEVMODEW *lpDevMode, HWND hwnd, DWORD dwflags, LPVOID lParam)
{
	if(IsTraceDDRAW){
		OutTrace("ChangeDisplaySettingsExW: DeviceName=%ls lpDevMode=%x flags=%x(%s)", lpszDeviceName, lpDevMode, dwflags, ExplainChangeDisplaySettingsFlags(dwflags));
		if (lpDevMode) OutTrace(" DeviceName=%ls fields=%x(%s) size=(%d x %d) bpp=%d", 
			lpDevMode->dmDeviceName, lpDevMode->dmFields, ExplainDevModeFields(lpDevMode->dmFields),
			lpDevMode->dmPelsWidth, lpDevMode->dmPelsHeight, lpDevMode->dmBitsPerPel);
		OutTrace("\n");
	}

	if(dxw.Windowize)
		return MyChangeDisplaySettings("ChangeDisplaySettingsExW", TRUE, lpDevMode, dwflags);
	else
		return (*pChangeDisplaySettingsExW)(lpszDeviceName, lpDevMode, hwnd, dwflags, lParam);
}

HDC WINAPI extGDIGetDC(HWND hwnd)
{
	HDC ret;
	HWND lochwnd;

	OutTraceDW("GDI.GetDC: hwnd=%x\n", hwnd);
	lochwnd=hwnd;
	if (dxw.IsRealDesktop(hwnd)) {
		OutTraceDW("GDI.GetDC: desktop remapping hwnd=%x->%x\n", hwnd, dxw.GethWnd());
		lochwnd=dxw.GethWnd();
	}

	ret=(*pGDIGetDC)(lochwnd);
	
	if(ret){
		OutTraceDW("GDI.GetDC: hwnd=%x ret=%x\n", lochwnd, ret);
	}
	else{
		int err;
		err=GetLastError();
		OutTraceE("GDI.GetDC ERROR: hwnd=%x err=%d at %d\n", lochwnd, err, __LINE__);
		if((err==ERROR_INVALID_WINDOW_HANDLE) && (lochwnd!=hwnd)){
			ret=(*pGDIGetDC)(hwnd);	
			if(ret)
				OutTraceDW("GDI.GetDC: hwnd=%x ret=%x\n", hwnd, ret);
			else
				OutTraceE("GDI.GetDC ERROR: hwnd=%x err=%d at %d\n", hwnd, GetLastError(), __LINE__);
		}
	}

	return ret;
}

HDC WINAPI extEMUGetDC(HWND hwnd)
{
	HDC ret;
	HWND lochwnd;

	OutTraceDW("GDI.GetDC: hwnd=%x\n", hwnd);

	lochwnd=hwnd;
	if (dxw.IsRealDesktop(hwnd)) {
		OutTraceDW("GDI.GetDC: desktop remapping hwnd=%x->%x\n", hwnd, dxw.GethWnd());
		lochwnd=dxw.GethWnd();
	}

	ret=dxw.AcquireEmulatedDC(lochwnd);
	OutTraceDW("GDI.GetDC: remapping hdc=%x->%x\n", (*pGDIGetDC)(hwnd), ret);
	dxw.VirtualHDC=ret;
	
	if(ret){
		OutTraceDW("GDI.GetDC: hwnd=%x ret=%x\n", lochwnd, ret);
	}
	else{
		int err;
		err=GetLastError();
		OutTraceE("GDI.GetDC ERROR: hwnd=%x err=%d at %d\n", lochwnd, err, __LINE__);
		if((err==ERROR_INVALID_WINDOW_HANDLE) && (lochwnd!=hwnd)){
			ret=(*pGDIGetDC)(hwnd);	
			if(ret)
				OutTraceDW("GDI.GetDC: hwnd=%x ret=%x\n", hwnd, ret);
			else
				OutTraceE("GDI.GetDC ERROR: hwnd=%x err=%d at %d\n", hwnd, GetLastError(), __LINE__);
		}
	}

	return ret;
}

HDC WINAPI extGDIGetDCEx(HWND hwnd, HRGN hrgnClip, DWORD flags)
{
	// used by Star Wars Shadow of the Empire
	HDC ret;
	HWND lochwnd;

	OutTraceDW("GDI.GetDCEx: hwnd=%x hrgnClip=%x flags=%x(%s)\n", hwnd, hrgnClip, flags, ExplainGetDCExFlags(flags));
	lochwnd=hwnd;
	if (dxw.IsRealDesktop(hwnd)) {
		OutTraceDW("GDI.GetDCEx: desktop remapping hwnd=%x->%x\n", hwnd, dxw.GethWnd());
		lochwnd=dxw.GethWnd();
	}

	ret=(*pGDIGetDC)(lochwnd);
	
	if(ret){
		OutTraceDW("GDI.GetDCEx: hwnd=%x ret=%x\n", lochwnd, ret);
	}
	else{
		int err;
		err=GetLastError();
		OutTraceE("GDI.GetDCEx ERROR: hwnd=%x err=%d at %d\n", lochwnd, err, __LINE__);
		if((err==ERROR_INVALID_WINDOW_HANDLE) && (lochwnd!=hwnd)){
			ret=(*pGDIGetDCEx)(hwnd, hrgnClip, flags);	
			if(ret)
				OutTraceDW("GDI.GetDCEx: hwnd=%x ret=%x\n", hwnd, ret);
			else
				OutTraceE("GDI.GetDCEx ERROR: hwnd=%x err=%d at %d\n", hwnd, GetLastError(), __LINE__);
		}
	}

	return ret;
}

HDC WINAPI extEMUGetDCEx(HWND hwnd, HRGN hrgnClip, DWORD flags)
{
	// used by Star Wars Shadow of the Empire
	HDC ret;
	HWND lochwnd;

	OutTraceDW("GDI.GetDCEx: hwnd=%x hrgnClip=%x flags=%x(%s)\n", hwnd, hrgnClip, flags, ExplainGetDCExFlags(flags));
	lochwnd=hwnd;
	if (dxw.IsRealDesktop(hwnd)) {
		OutTraceDW("GDI.GetDCEx: desktop remapping hwnd=%x->%x\n", hwnd, dxw.GethWnd());
		lochwnd=dxw.GethWnd();
	}

	ret=dxw.AcquireEmulatedDC(lochwnd);
	OutTraceDW("GDI.GetDCEx: remapping hdc=%x->%x\n", (*pGDIGetDC)(hwnd), ret);
	dxw.VirtualHDC=ret;
	
	if(ret){
		OutTraceDW("GDI.GetDCEx: hwnd=%x ret=%x\n", lochwnd, ret);
	}
	else{
		int err;
		err=GetLastError();
		OutTraceE("GDI.GetDCEx ERROR: hwnd=%x err=%d at %d\n", lochwnd, err, __LINE__);
		if((err==ERROR_INVALID_WINDOW_HANDLE) && (lochwnd!=hwnd)){
			ret=(*pGDIGetDCEx)(hwnd, hrgnClip, flags);	
			if(ret)
				OutTraceDW("GDI.GetDCEx: hwnd=%x ret=%x\n", hwnd, ret);
			else
				OutTraceE("GDI.GetDCEx ERROR: hwnd=%x err=%d at %d\n", hwnd, GetLastError(), __LINE__);
		}
	}

	return ret;
}

HDC WINAPI extGDIGetWindowDC(HWND hwnd)
{
	HDC ret;
	HWND lochwnd;
	OutTraceDW("GDI.GetWindowDC: hwnd=%x\n", hwnd);
	lochwnd=hwnd;
	if (dxw.IsRealDesktop(hwnd)) {
		OutTraceDW("GDI.GetWindowDC: desktop remapping hwnd=%x->%x\n", hwnd, dxw.GethWnd());
		lochwnd=dxw.GethWnd();
	}

	if(dxw.IsFullScreen()){
		ret=dxw.AcquireEmulatedDC(lochwnd);
		OutTraceDW("GDI.GetWindowDC: remapping hdc=%x->%x\n", (*pGDIGetDC)(hwnd), ret);
		dxw.VirtualHDC=ret;
	}
	else
		ret=(*pGDIGetWindowDC)(lochwnd);

	if(ret){
		OutTraceDW("GDI.GetWindowDC: hwnd=%x ret=%x\n", lochwnd, ret);
	}
	else{
		int err;
		err=GetLastError();
		OutTraceE("GDI.GetWindowDC ERROR: hwnd=%x err=%d at %d\n", lochwnd, err, __LINE__);
		if((err==ERROR_INVALID_WINDOW_HANDLE) && (lochwnd!=hwnd)){
			ret=(*pGDIGetWindowDC)(hwnd);
			if(ret)
				OutTraceDW("GDI.GetWindowDC: hwnd=%x ret=%x\n", hwnd, ret);
			else
				OutTraceE("GDI.GetWindowDC ERROR: hwnd=%x err=%d at %d\n", hwnd, GetLastError(), __LINE__);
		}
	}
	return ret;
}

HDC WINAPI extEMUGetWindowDC(HWND hwnd)
{
	HDC ret;
	HWND lochwnd;
	OutTraceDW("GDI.GetWindowDC: hwnd=%x\n", hwnd);
	lochwnd=hwnd;
	if (dxw.IsRealDesktop(hwnd)) {
		OutTraceDW("GDI.GetWindowDC: desktop remapping hwnd=%x->%x\n", hwnd, dxw.GethWnd());
		lochwnd=dxw.GethWnd();
	}

	if(dxw.IsDesktop(hwnd) && dxw.IsFullScreen())
		ret=(*pGDIGetDC)(lochwnd);
	else
		ret=(*pGDIGetWindowDC)(lochwnd);

	if(ret){
		OutTraceDW("GDI.GetWindowDC: hwnd=%x ret=%x\n", lochwnd, ret);
	}
	else{
		int err;
		err=GetLastError();
		OutTraceE("GDI.GetWindowDC ERROR: hwnd=%x err=%d at %d\n", lochwnd, err, __LINE__);
		if((err==ERROR_INVALID_WINDOW_HANDLE) && (lochwnd!=hwnd)){
			ret=(*pGDIGetWindowDC)(hwnd);
			if(ret)
				OutTraceDW("GDI.GetWindowDC: hwnd=%x ret=%x\n", hwnd, ret);
			else
				OutTraceE("GDI.GetWindowDC ERROR: hwnd=%x err=%d at %d\n", hwnd, GetLastError(), __LINE__);
		}
	}
	return ret;
}

int WINAPI extGDIReleaseDC(HWND hwnd, HDC hDC)
{
	int res;

	OutTraceDW("GDI.ReleaseDC: hwnd=%x hdc=%x\n", hwnd, hDC);

	if (dxw.IsRealDesktop(hwnd)) hwnd=dxw.GethWnd();
	res=(*pGDIReleaseDC)(hwnd, hDC);
	if (!res) OutTraceE("GDI.ReleaseDC ERROR: err=%d at %d\n", GetLastError(), __LINE__);
	return(res);
}

int WINAPI extEMUReleaseDC(HWND hwnd, HDC hDC)
{
	int res;
	HDC windc;

	OutTraceDW("GDI.ReleaseDC: hwnd=%x hdc=%x\n", hwnd, hDC);

	if (dxw.IsRealDesktop(hwnd)) hwnd=dxw.GethWnd();
	windc=(*pGDIGetDC)(hwnd);
	res=dxw.ReleaseEmulatedDC(hwnd);
	res=(*pGDIReleaseDC)(hwnd, windc);
	if (!res) OutTraceE("GDI.ReleaseDC ERROR: err=%d at %d\n", GetLastError(), __LINE__);
	return(res);
}

HDC WINAPI extBeginPaint(HWND hwnd, LPPAINTSTRUCT lpPaint)
{
	HDC hdc;

	OutTraceDW("GDI.BeginPaint: hwnd=%x lpPaint=%x FullScreen=%x\n", hwnd, lpPaint, dxw.IsFullScreen());
	hdc=(*pBeginPaint)(hwnd, lpPaint);

	// avoid access to real desktop
	if(dxw.IsRealDesktop(hwnd)) hwnd=dxw.GethWnd();

	// if not in fullscreen mode, that's all!
	if(!dxw.IsFullScreen()) return hdc;

	// on CLIENTREMAPPING, resize the paint area to virtual screen size
	if(dxw.dwFlags1 & CLIENTREMAPPING) lpPaint->rcPaint=dxw.GetScreenRect();

	OutTraceDW("GDI.BeginPaint: hdc=%x rcPaint=(%d,%d)-(%d,%d)\n", 
		hdc, lpPaint->rcPaint.left, lpPaint->rcPaint.top, lpPaint->rcPaint.right, lpPaint->rcPaint.bottom);
	return hdc;
}

HDC WINAPI extEMUBeginPaint(HWND hwnd, LPPAINTSTRUCT lpPaint)
{
	HDC hdc;
	HDC EmuHDC; 

	OutTraceDW("GDI.BeginPaint: hwnd=%x lpPaint=%x FullScreen=%x\n", hwnd, lpPaint, dxw.IsFullScreen());
	hdc=(*pBeginPaint)(hwnd, lpPaint);

	// avoid access to real desktop
	if(dxw.IsRealDesktop(hwnd)) hwnd=dxw.GethWnd();

	// if not in fullscreen mode, that's all!
	if(!dxw.IsFullScreen()) return hdc;

	// on CLIENTREMAPPING, resize the paint area to virtual screen size
	if(dxw.dwFlags1 & CLIENTREMAPPING) lpPaint->rcPaint=dxw.GetScreenRect();

	EmuHDC = dxw.AcquireEmulatedDC(hwnd); 
	lpPaint->hdc=EmuHDC;
	OutTraceDW("GDI.BeginPaint: hdc=%x -> %x\n", hdc, EmuHDC);
	return EmuHDC;
}

HDC WINAPI extDDBeginPaint(HWND hwnd, LPPAINTSTRUCT lpPaint)
{
	HDC hdc;
	extern HRESULT WINAPI extGetDC(LPDIRECTDRAWSURFACE, HDC FAR *);

	OutTraceDW("GDI.BeginPaint: hwnd=%x%s lpPaint=%x FullScreen=%x\n", 
		hwnd, dxw.IsDesktop(hwnd)?"(DESKTOP)":"", lpPaint, dxw.IsFullScreen());

	if(dxw.IsDesktop(hwnd)) hwnd=dxw.GethWnd();

	hdc=(*pBeginPaint)(hwnd, lpPaint);

	// if not in fullscreen mode, that's all!
	if(!dxw.IsFullScreen()) return hdc;

	// on MAPGDITOPRIMARY, return the PrimHDC handle instead of the window DC
	// if a primary surface has not been created yet, do it
	if(!pGetDC || !dxw.lpDDSPrimHDC){
		extern HRESULT WINAPI extDirectDrawCreate(GUID FAR *, LPDIRECTDRAW FAR *, IUnknown FAR *);
		HRESULT res;
		LPDIRECTDRAW lpDD;
		LPDIRECTDRAWSURFACE lpDDS;
		DDSURFACEDESC ddsd;
		res=extDirectDrawCreate(0, &lpDD, NULL);
		//res=extDirectDrawCreate((GUID *)&IID_IDirectDraw, &lpDD, NULL);
		lpDD->SetCooperativeLevel(dxw.GethWnd(), DDSCL_FULLSCREEN | DDSCL_EXCLUSIVE);
		//lpDD->SetCooperativeLevel(dxw.GethWnd(), DDSCL_NORMAL);
		memset((void *)&ddsd, 0, sizeof(DDSURFACEDESC));
		ddsd.dwSize = sizeof(DDSURFACEDESC);
		ddsd.dwFlags = DDSD_CAPS|DDSD_HEIGHT|DDSD_WIDTH;
		ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;
		ddsd.dwHeight = dxw.GetScreenHeight();
		ddsd.dwWidth = dxw.GetScreenWidth();
		res=lpDD->CreateSurface(&ddsd, &lpDDS, NULL);
		dxw.lpDDSPrimHDC = lpDDS;
		OutTraceDW("GDI.BeginPaint(MAPGDITOPRIMARY): dd=%x ddsPrim=%x\n", lpDD, lpDDS);
	}

	extGetDC(dxw.lpDDSPrimHDC,&PrimHDC);
	lpPaint->hdc=PrimHDC;
	// resize the paint area to virtual screen size (see CivIII clipped panels...)
	lpPaint->rcPaint=dxw.GetScreenRect();

	OutTraceDW("GDI.BeginPaint(MAPGDITOPRIMARY): hdc=%x -> %x\n", hdc, PrimHDC);
	return PrimHDC;
}

BOOL WINAPI extEndPaint(HWND hwnd, const PAINTSTRUCT *lpPaint)
{
	BOOL ret;

	OutTraceDW("GDI.EndPaint: hwnd=%x lpPaint=%x lpPaint.hdc=%x\n", hwnd, lpPaint, lpPaint->hdc);

	ret=(*pEndPaint)(hwnd, lpPaint);
	OutTraceDW("GDI.EndPaint: hwnd=%x ret=%x\n", hwnd, ret);
	if(!ret) OutTraceE("GDI.EndPaint ERROR: err=%d at %d\n", GetLastError(), __LINE__);
	return ret;
}

BOOL WINAPI extEMUEndPaint(HWND hwnd, const PAINTSTRUCT *lpPaint)
{
	BOOL ret;

	OutTraceDW("GDI.EndPaint: hwnd=%x lpPaint=%x lpPaint.hdc=%x\n", hwnd, lpPaint, lpPaint->hdc);

	if(dxw.IsFullScreen()){
		OutTraceDW("GDI.EndPaint(GDIEMULATEDC): hwnd=%x\n", hwnd);
		ret=dxw.ReleaseEmulatedDC(hwnd);
	}
	else
	// proxy part ...
	ret=(*pEndPaint)(hwnd, lpPaint);
	OutTraceDW("GDI.EndPaint: hwnd=%x ret=%x\n", hwnd, ret);
	if(!ret) OutTraceE("GDI.EndPaint ERROR: err=%d at %d\n", GetLastError(), __LINE__);
	return ret;
}

BOOL WINAPI extDDEndPaint(HWND hwnd, const PAINTSTRUCT *lpPaint)
{
	BOOL ret;
	extern HRESULT WINAPI extReleaseDC(LPDIRECTDRAWSURFACE lpdds, HDC FAR hdc);
	extern HRESULT WINAPI extGetDC(LPDIRECTDRAWSURFACE, HDC FAR *);
	extern HRESULT WINAPI extBlt(LPDIRECTDRAWSURFACE lpdds, LPRECT lpdestrect, LPDIRECTDRAWSURFACE lpddssrc, LPRECT lpsrcrect, DWORD dwflags, LPDDBLTFX lpddbltfx);

	OutTraceDW("GDI.EndPaint(DD): hwnd=%x%s lpPaint=%x lpPaint.hdc=%x\n", 
		hwnd, dxw.IsDesktop(hwnd)?"(DESKTOP)":"", lpPaint, lpPaint->hdc);

	// v2.02.53 ...
	//if(dxw.IsFullScreen() && dxw.IsDesktop(hwnd)){
	if(dxw.IsFullScreen()){
		RECT Rect;
		LPRECT lpRect;
		ret=(*pEndPaint)(hwnd, lpPaint);
		dxw.lpDDSPrimHDC->Unlock(NULL);
		//dxw.ScreenRefresh();
		if(dxw.IsDesktop(hwnd))
			lpRect=NULL;
		else{
#if 1
			POINT p={0,0};
			lpRect=&Rect;
			(*pGetClientRect)(hwnd, lpRect);
			p=dxw.ClientOffset(hwnd);
			lpRect->left += p.x;
			lpRect->right += p.x;
			lpRect->top += p.y;
			lpRect->bottom += p.y;
			//dxw.AddCoordinates(lpRect, p);
			dxw.UnmapClient(lpRect);
#else
			lpRect=&Rect;
			Rect = dxw.MapClientRect(lpRect);
#endif
		}
		extBlt(dxw.lpDDSPrimHDC, lpRect, dxw.lpDDSPrimHDC, NULL, 0, NULL);
		return TRUE;
	}

	// proxy part ...
	ret=(*pEndPaint)(hwnd, lpPaint);
	OutTraceDW("GDI.EndPaint: hwnd=%x ret=%x\n", hwnd, ret);
	if(!ret) OutTraceE("GDI.EndPaint ERROR: err=%d at %d\n", GetLastError(), __LINE__);
	return ret;
}

HWND WINAPI extCreateDialogIndirectParam(HINSTANCE hInstance, LPCDLGTEMPLATE lpTemplate, HWND hWndParent, DLGPROC lpDialogFunc, LPARAM lParamInit)
{
	HWND RetHWND;
	isWithinDialog=TRUE;
	OutTraceDW("CreateDialogIndirectParam: hInstance=%x lpTemplate=%s hWndParent=%x lpDialogFunc=%x lParamInit=%x\n",
		hInstance, "tbd", hWndParent, lpDialogFunc, lParamInit);
	if(dxw.IsFullScreen() && hWndParent==NULL) hWndParent=dxw.GethWnd();
	RetHWND=(*pCreateDialogIndirectParam)(hInstance, lpTemplate, hWndParent, lpDialogFunc, lParamInit);

	// v2.02.73: redirect lpDialogFunc only when it is nor NULL
	if(lpDialogFunc) {	
		WinDBPutProc(RetHWND, (WNDPROC)lpDialogFunc);
		if(!(*pSetWindowLongA)(RetHWND, DWL_DLGPROC, (LONG)extDialogWindowProc))
			OutTraceE("SetWindowLong: ERROR err=%d at %d\n", GetLastError(), __LINE__);
	}

	OutTraceDW("CreateDialogIndirectParam: hwnd=%x\n", RetHWND);
	isWithinDialog=FALSE;
	//if (IsDebug) EnumChildWindows(RetHWND, (WNDENUMPROC)TraceChildWin, (LPARAM)RetHWND);
	return RetHWND;
}

HWND WINAPI extCreateDialogParam(HINSTANCE hInstance, LPCTSTR lpTemplateName, HWND hWndParent, DLGPROC lpDialogFunc, LPARAM lParamInit)
{
	HWND RetHWND;
	isWithinDialog=TRUE;
	OutTraceDW("CreateDialogParam: hInstance=%x lpTemplateName=%s hWndParent=%x lpDialogFunc=%x lParamInit=%x\n",
		hInstance, "tbd", hWndParent, lpDialogFunc, lParamInit);
	if(hWndParent==NULL) hWndParent=dxw.GethWnd();
	RetHWND=(*pCreateDialogParam)(hInstance, lpTemplateName, hWndParent, lpDialogFunc, lParamInit);

	// v2.02.73: redirect lpDialogFunc only when it is nor NULL: fix for "LEGO Stunt Rally"
	if(lpDialogFunc) {
		WinDBPutProc(RetHWND, (WNDPROC)lpDialogFunc);
		if(!(*pSetWindowLongA)(RetHWND, DWL_DLGPROC, (LONG)extDialogWindowProc))
			OutTraceE("SetWindowLong: ERROR err=%d at %d\n", GetLastError(), __LINE__);
	}

	OutTraceDW("CreateDialogParam: hwnd=%x\n", RetHWND);
	isWithinDialog=FALSE;
	//if (IsDebug) EnumChildWindows(RetHWND, (WNDENUMPROC)TraceChildWin, (LPARAM)RetHWND);
	return RetHWND;
}

BOOL WINAPI extMoveWindow(HWND hwnd, int X, int Y, int nWidth, int nHeight, BOOL bRepaint)
{
	BOOL ret;
	OutTraceDW("MoveWindow: hwnd=%x xy=(%d,%d) size=(%d,%d) repaint=%x indialog=%x fullscreen=%x\n",
		hwnd, X, Y, nWidth, nHeight, bRepaint, isWithinDialog, dxw.IsFullScreen());

	if(dxw.Windowize){
		if(dxw.IsDesktop(hwnd)){
			// v2.1.93: happens in "Emergency Fighters for Life" ...
			// what is the meaning of this? is it related to video stretching?
				OutTraceDW("MoveWindow: prevent moving desktop win\n");
				return TRUE;
			}

		if((hwnd==dxw.GethWnd()) || (hwnd==dxw.hParentWnd)){
				OutTraceDW("MoveWindow: prevent moving main win\n");
				return TRUE;
			}

		if (dxw.IsFullScreen()){
			POINT upleft={0,0};
			RECT client;
			BOOL isChild;
			(*pClientToScreen)(dxw.GethWnd(),&upleft);
			(*pGetClientRect)(dxw.GethWnd(),&client);
			if ((*pGetWindowLongA)(hwnd, GWL_STYLE) & WS_CHILD){
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
			OutTraceDW("MoveWindow: DEBUG client=(%d,%d) screen=(%d,%d)\n",
				client.right, client.bottom, dxw.GetScreenWidth(), dxw.GetScreenHeight());
			OutTraceDW("MoveWindow: hwnd=%x child=%x relocated to xy=(%d,%d) size=(%d,%d)\n",
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
				OutTraceDW("MoveWindow: fixed BIG win pos=(%d,%d) size=(%d,%d)\n", X, Y, nWidth, nHeight);
			}
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
	OutTraceDW("GetTopWindow: hwnd=%x fullscreen=%x\n", hwnd, dxw.IsFullScreen()); 
	// a fullscreen program is supposed to be always top Z-order on the desktop!
	ret = (dxw.IsFullScreen() && dxw.IsDesktop(hwnd)) ? dxw.GethWnd() : (*pGetTopWindow)(hwnd);
	OutTraceDW("GetTopWindow: ret=%x\n", ret); 
	return ret;
}

LONG WINAPI extTabbedTextOutA(HDC hdc, int X, int Y, LPCTSTR lpString, int nCount, int nTabPositions, const LPINT lpnTabStopPositions, int nTabOrigin)
{
	BOOL res;
	OutTraceDW("TabbedTextOut: hdc=%x xy=(%d,%d) nCount=%d nTP=%d nTOS=%d str=(%d)\"%s\"\n", 
		hdc, X, Y, nCount, nTabPositions, nTabOrigin, lpString);

	if (dxw.IsFullScreen() && (OBJ_DC == GetObjectType(hdc))){
		dxw.MapClient(&X, &Y);
		OutTraceDW("TextOut: fixed dest=(%d,%d)\n", X, Y);
	}

	res=(*pTabbedTextOutA)(hdc, X, Y, lpString, nCount, nTabPositions, lpnTabStopPositions, nTabOrigin);
	return res;
}

BOOL WINAPI extDestroyWindow(HWND hWnd)
{
	// v2.02.43: Empire Earth builds test surfaces that must be destroyed!
	BOOL res;
	OutTraceB("DestroyWindow: hwnd=%x\n", hWnd);
	if (hWnd == dxw.GethWnd()) {
		OutTraceDW("DestroyWindow: destroy main hwnd=%x\n", hWnd);
		dxw.SethWnd(NULL);
	}
	if (hControlParentWnd && (hWnd == hControlParentWnd)) {
		OutTraceDW("DestroyWindow: destroy control parent hwnd=%x\n", hWnd);
		hControlParentWnd = NULL;
	}
	res=(*pDestroyWindow)(hWnd);
	if(!res)OutTraceE("DestroyWindow: ERROR err=%d\n", GetLastError());
	return res;
}

static char *ExplainTAAlign(UINT c)
{
	static char eb[256];
	unsigned int l;
	strcpy(eb,"TA_");
	strcat(eb, (c & TA_UPDATECP) ? "UPDATECP+" : "NOUPDATECP+");
	strcat(eb, (c & TA_RIGHT) ? (((c & TA_CENTER) == TA_CENTER) ? "CENTER+" : "RIGHT+") : "LEFT+");
	strcat(eb, (c & TA_BOTTOM) ? "BOTTOM+" : "TOP+");
	if ((c & TA_BASELINE)==TA_BASELINE) strcat(eb, "BASELINE+");
	if (c & TA_RTLREADING) strcat(eb, "RTLREADING+");
	l=strlen(eb);
	eb[l-1]=0; 
	return(eb);
}

static char *ExplainDTFormat(UINT c)
{
	static char eb[256];
	unsigned int l;
	strcpy(eb,"DT_");
	if(!(c & (DT_CENTER|DT_RIGHT))) strcat(eb, "LEFT+");
	if(c & DT_CENTER) strcat(eb, "CENTER+");
	if(c & DT_RIGHT) strcat(eb, "RIGHT+");
	if(!(c & (DT_VCENTER|DT_BOTTOM))) strcat(eb, "TOP+");
	if(c & DT_VCENTER) strcat(eb, "VCENTER+");
	if(c & DT_BOTTOM) strcat(eb, "BOTTOM+");
	if(c & DT_WORDBREAK) strcat(eb, "WORDBREAK+");
	if(c & DT_SINGLELINE) strcat(eb, "SINGLELINE+");
	if(c & DT_EXPANDTABS) strcat(eb, "EXPANDTABS+");
	if(c & DT_TABSTOP) strcat(eb, "TABSTOP+");
	if(c & DT_NOCLIP) strcat(eb, "NOCLIP+");
	if(c & DT_EXTERNALLEADING) strcat(eb, "EXTERNALLEADING+");
	if(c & DT_CALCRECT) strcat(eb, "CALCRECT+");
	if(c & DT_NOPREFIX) strcat(eb, "NOPREFIX+");
	if(c & DT_INTERNAL) strcat(eb, "INTERNAL+");
	l=strlen(eb);
	eb[l-1]=0; 
	return(eb);
}

BOOL gFixed;

int WINAPI extDrawTextA(HDC hdc, LPCTSTR lpchText, int nCount, LPRECT lpRect, UINT uFormat)
{
	int ret;
	OutTraceDW("DrawText: hdc=%x rect=(%d,%d)-(%d,%d) Format=%x(%s) Text=(%d)\"%s\"\n", 
		hdc, lpRect->left, lpRect->top, lpRect->right, lpRect->bottom, uFormat, ExplainDTFormat(uFormat), nCount, lpchText);

	gFixed = TRUE;
	if (dxw.IsFullScreen() && (OBJ_DC == GetObjectType(hdc))){
		dxw.MapClient((RECT *)lpRect);
		OutTraceDW("DrawText: fixed rect=(%d,%d)-(%d,%d)\n", lpRect->left, lpRect->top, lpRect->right, lpRect->bottom);
	}

	ret=(*pDrawText)(hdc, lpchText, nCount, lpRect, uFormat);
	gFixed=FALSE;
	// if nCount is zero, DrawRect returns 0 as text heigth, but this is not an error! (ref. "Imperialism II")
	if(nCount && !ret) OutTraceE("DrawText: ERROR ret=%x err=%d\n", ret, GetLastError()); 
	return ret;
}

int WINAPI extDrawTextExA(HDC hdc, LPTSTR lpchText, int nCount, LPRECT lpRect, UINT dwDTFormat, LPDRAWTEXTPARAMS lpDTParams)
{
	int ret;
	OutTraceDW("DrawTextEx: hdc=%x rect=(%d,%d)-(%d,%d) DTFormat=%x Text=(%d)\"%s\"\n", 
		hdc, lpRect->left, lpRect->top, lpRect->right, lpRect->bottom, dwDTFormat, nCount, lpchText);

	if (dxw.IsFullScreen() && (OBJ_DC == GetObjectType(hdc))){
		dxw.MapClient((RECT *)lpRect);
		OutTraceDW("DrawTextEx: fixed rect=(%d,%d)-(%d,%d)\n", lpRect->left, lpRect->top, lpRect->right, lpRect->bottom);
	}

	ret=(*pDrawTextEx)(hdc, lpchText, nCount, lpRect, dwDTFormat, lpDTParams);
	if(!ret) OutTraceE("DrawTextEx: ERROR ret=%x err=%d\n", ret, GetLastError()); 
	return ret;
}

BOOL WINAPI extCloseWindow(HWND hWnd)
{
	// from MSDN: Minimizes (but does not destroy) the specified window.
	BOOL res;
	OutTraceB("CloseWindow: hwnd=%x\n", hWnd);
	if (hWnd == dxw.GethWnd()) {
		OutTraceDW("CloseWindow: close main hwnd=%x\n", hWnd);
		// do not delete the reference to main hWnd.
	}
	res=(*pCloseWindow)(hWnd);
	if(!res)OutTraceE("CloseWindow: ERROR err=%d\n", GetLastError());
	return res;
}

BOOL WINAPI extSetSysColors(int cElements, const INT *lpaElements, const COLORREF *lpaRgbValues)
{
	// v2.02.32: added to avoid SysColors changes by "Western Front"
	BOOL ret;
	OutTraceDW("SetSysColors: Elements=%d\n", cElements);

	if(dxw.dwFlags3 & LOCKSYSCOLORS) return TRUE;

	ret=(*pSetSysColors)(cElements, lpaElements, lpaRgbValues);
	if(!ret) OutTraceE("SetSysColors: ERROR er=%d\n", GetLastError());
	return ret;
}

BOOL WINAPI extUpdateWindow(HWND hwnd)
{
	BOOL ret;
	OutTraceDW("UpdateWindow: hwnd=%x\n", hwnd);

	if(dxw.Windowize && dxw.IsRealDesktop(hwnd)){
		OutTraceDW("UpdateWindow: remapping hwnd=%x->%x\n", hwnd, dxw.GethWnd());
		hwnd=dxw.GethWnd();
	}

	ret=(*pUpdateWindow)(hwnd);
	if(!ret) OutTraceE("UpdateWindow: ERROR er=%d\n", GetLastError());
	return ret;
}

BOOL WINAPI extRedrawWindow(HWND hWnd, const RECT *lprcUpdate, HRGN hrgnUpdate, UINT flags)
{
	OutTraceDW("RedrawWindow: hwnd=%x flags=%x\n", hWnd, flags);
	return (*pRedrawWindow)(hWnd, lprcUpdate, hrgnUpdate, flags);
}


BOOL WINAPI extGetWindowPlacement(HWND hwnd, WINDOWPLACEMENT *lpwndpl)
{
	BOOL ret;
	OutTraceDW("GetWindowPlacement: hwnd=%x\n", hwnd);

	if(dxw.IsRealDesktop(hwnd)){
		OutTraceDW("GetWindowPlacement: remapping hwnd=%x->%x\n", hwnd, dxw.GethWnd());
		hwnd=dxw.GethWnd();
	}

	ret=(*pGetWindowPlacement)(hwnd, lpwndpl);
	OutTraceDW("GetWindowPlacement: flags=%x showCmd=%x MinPosition=(%d,%d) MaxPosition=(%d,%d) NormalPosition=(%d,%d)-(%d,%d)\n",
		lpwndpl->flags, lpwndpl->showCmd, 
		lpwndpl->ptMinPosition.x, lpwndpl->ptMinPosition.y,
		lpwndpl->ptMaxPosition.x, lpwndpl->ptMaxPosition.y,
		lpwndpl->rcNormalPosition.left, lpwndpl->rcNormalPosition.top, lpwndpl->rcNormalPosition.right, lpwndpl->rcNormalPosition.bottom);

	switch (lpwndpl->showCmd){
	case SW_SHOW:
		if (dxw.IsFullScreen()){
			lpwndpl->showCmd = SW_MAXIMIZE;
			OutTraceDW("GetWindowPlacement: forcing SW_MAXIMIZE state\n");
		}
		break;
	}
	if(!ret) OutTraceE("GetWindowPlacement: ERROR er=%d\n", GetLastError());
	return ret;
}

BOOL WINAPI extSetWindowPlacement(HWND hwnd, WINDOWPLACEMENT *lpwndpl)
{
	BOOL ret;
	OutTraceDW("SetWindowPlacement: hwnd=%x\n", hwnd);

	if(dxw.IsRealDesktop(hwnd)){
		OutTraceDW("SetWindowPlacement: remapping hwnd=%x->%x\n", hwnd, dxw.GethWnd());
		hwnd=dxw.GethWnd();
	}

	OutTraceDW("SetWindowPlacement: flags=%x showCmd=%x MinPosition=(%d,%d) MaxPosition=(%d,%d) NormalPosition=(%d,%d)-(%d,%d)\n",
		lpwndpl->flags, lpwndpl->showCmd, 
		lpwndpl->ptMinPosition.x, lpwndpl->ptMinPosition.y,
		lpwndpl->ptMaxPosition.x, lpwndpl->ptMaxPosition.y,
		lpwndpl->rcNormalPosition.left, lpwndpl->rcNormalPosition.top, lpwndpl->rcNormalPosition.right, lpwndpl->rcNormalPosition.bottom);

	switch (lpwndpl->showCmd){
	case SW_MAXIMIZE:
		if (dxw.IsFullScreen()){
			lpwndpl->showCmd = SW_SHOW;
			OutTraceDW("SetWindowPlacement: forcing SW_SHOW state\n");
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
	OutTraceDW("SetCapture: hwnd=%x\n", hwnd);
	ret=(*pSetCapture)(hwnd);
	OutTraceDW("SetCapture: ret=%x\n", ret);
	return ret;
}

HWND WINAPI extGetActiveWindow(void)
{
	HWND ret;
	ret=(*pGetActiveWindow)();
	OutTraceDW("GetActiveWindow: ret=%x->%x\n", ret, dxw.GethWnd());
	//STOPPER("GetActiveWindow");
	return dxw.GethWnd();
}

HWND WINAPI extGetForegroundWindow(void)
{
	HWND ret;
	ret=(*pGetForegroundWindow)();
	OutTraceDW("GetForegroundWindow: ret=%x->%x\n", ret, dxw.GethWnd());
	//STOPPER("GetForegroundWindow");
	return dxw.GethWnd();
}

BOOL WINAPI extIsWindowVisible(HWND hwnd)
{
	BOOL ret;
	ret=(*pIsWindowVisible)(hwnd);
	OutTraceB("IsWindowVisible: hwnd=%x ret=%x\n", hwnd, ret);
	if(dxw.IsDesktop(hwnd) && !ret){
		OutTraceDW("IsWindowVisible: FORCING ret=TRUE\n");
		ret=TRUE;
	}
	return ret;
}

BOOL WINAPI extSystemParametersInfoA(UINT uiAction, UINT uiParam, PVOID pvParam, UINT fWinIni)
{
	BOOL ret;
	OutTraceDW("SystemParametersInfo: Action=%x Param=%x WinIni=%x\n", uiAction, uiParam, fWinIni);
	ret=(*pSystemParametersInfoA)(uiAction, uiParam, pvParam, fWinIni);
	if(uiAction==SPI_GETWORKAREA){
		LPRECT cli = (LPRECT)pvParam;
		cli->top = 0;
		cli->left = 0;
		cli->bottom = dxw.GetScreenHeight();
		cli->right = dxw.GetScreenWidth();
		OutTraceDW("SystemParametersInfo: resized client workarea rect=(%d,%d)-(%d,%d)\n", cli->left, cli->top, cli->right, cli->bottom);
	}
	return ret;
}

BOOL WINAPI extSystemParametersInfoW(UINT uiAction, UINT uiParam, PVOID pvParam, UINT fWinIni)
{
	BOOL ret;
	OutTraceDW("SystemParametersInfo: Action=%x Param=%x WinIni=%x\n", uiAction, uiParam, fWinIni);
	ret=(*pSystemParametersInfoW)(uiAction, uiParam, pvParam, fWinIni);
	if(uiAction==SPI_GETWORKAREA){
		LPRECT cli = (LPRECT)pvParam;
		cli->top = 0;
		cli->left = 0;
		cli->bottom = dxw.GetScreenHeight();
		cli->right = dxw.GetScreenWidth();
		OutTraceDW("SystemParametersInfo: resized client workarea rect=(%d,%d)-(%d,%d)\n", cli->left, cli->top, cli->right, cli->bottom);
	}
	return ret;
}

#undef OutTraceDW
#define OutTraceDW OutTrace

UINT_PTR WINAPI extSetTimer(HWND hWnd, UINT_PTR nIDEvent, UINT uElapse, TIMERPROC lpTimerFunc)
{
	UINT uShiftedElapse;
	UINT_PTR ret;
	// beware: the quicker the time flows, the more the time clicks are incremented,
	// and the lesser the pauses must be lasting! Shift operations are reverted in
	// GetSystemTime vs. Sleep or SetTimer
	uShiftedElapse = dxw.StretchTime(uElapse);
	OutTraceDW("SetTimer: hwnd=%x TimerFunc=%x elapse=%d->%d timeshift=%d\n", hWnd, lpTimerFunc, uElapse, uShiftedElapse, dxw.TimeShift);
	ret = (*pSetTimer)(hWnd, nIDEvent, uShiftedElapse, lpTimerFunc);
	if(ret) dxw.PushTimer(hWnd, ret, uElapse, lpTimerFunc);
	OutTraceDW("SetTimer: IDEvent=%x ret=%x\n", nIDEvent, ret);
	return ret;
}

BOOL WINAPI extKillTimer(HWND hWnd, UINT_PTR uIDEvent)
{
	BOOL ret;
	OutTraceDW("KillTimer: hwnd=%x IDEvent=%x\n", hWnd, uIDEvent); 
	ret = (*pKillTimer)(hWnd, uIDEvent);
	OutTraceDW("KillTimer: ret=%x\n", ret);
	if(ret) dxw.PopTimer(hWnd, uIDEvent);
	return ret;
}

BOOL WINAPI extGetUpdateRect(HWND hWnd, LPRECT lpRect, BOOL bErase)
{
	BOOL ret;
	OutTraceDW("GetUpdateRect: hwnd=%x Erase=%x\n", hWnd, bErase); 
	ret = (*pGetUpdateRect)(hWnd, lpRect, bErase);
	if(ret){
		OutTraceDW("GetUpdateRect: rect=(%d,%d)-(%d,%d)\n", lpRect->left, lpRect->top, lpRect->right, lpRect->bottom);
		if(dxw.IsFullScreen()){
			dxw.UnmapClient(lpRect);
			OutTraceDW("GetUpdateRect: FIXED rect=(%d,%d)-(%d,%d)\n", lpRect->left, lpRect->top, lpRect->right, lpRect->bottom);
		}
	}
	else
		OutTraceE("GetUpdateRect ERROR: err=%d\n", GetLastError());
	return ret;
}

BOOL WINAPI extGetCursorInfo(PCURSORINFO pci)
{
	BOOL ret;
	OutTraceDW("GetCursorInfo\n"); 
	ret = (*pGetCursorInfo)(pci);
	if(ret){
		OutTraceDW("GetCursorInfo: flags=%x hcursor=%x pos=(%d,%d)\n", pci->flags, pci->hCursor, pci->ptScreenPos.x, pci->ptScreenPos.y);
		if(dxw.IsFullScreen()){
			dxw.UnmapClient(&(pci->ptScreenPos));
			OutTraceDW("GetCursorInfo: FIXED pos=(%d,%d)\n", pci->ptScreenPos.x, pci->ptScreenPos.y);
		}
	}
	else
		OutTraceE("GetCursorInfo ERROR: err=%d\n", GetLastError());
	return ret;
}

// --- to be hooked ....

HWND WINAPI extWindowFromPoint(POINT Point)
{
	HWND ret;
	OutTraceDW("WindowFromPoint: point=(%d,%d)\n", Point.x, Point.y); 
	if(dxw.IsFullScreen()){
		dxw.UnmapClient(&Point);
		OutTraceDW("WindowFromPoint: FIXED point=(%d,%d)\n", Point.x, Point.y);
	}
	ret = (*pWindowFromPoint)(Point);
	OutTraceDW("WindowFromPoint: hwnd=%x\n", ret);
	return ret;
}

HWND WINAPI extChildWindowFromPoint(HWND hWndParent, POINT Point)
{
	HWND ret;
	OutTraceDW("ChildWindowFromPoint: hWndParent=%x point=(%d,%d)\n", hWndParent, Point.x, Point.y); 
	if(dxw.IsDesktop(hWndParent) && dxw.IsFullScreen()){
		dxw.UnmapClient(&Point);
		OutTraceDW("ChildWindowFromPoint: FIXED point=(%d,%d)\n", Point.x, Point.y);
	}
	ret = (*pChildWindowFromPoint)(hWndParent, Point);
	OutTraceDW("ChildWindowFromPoint: hwnd=%x\n", ret);
	return ret;
}

HWND WINAPI extChildWindowFromPointEx(HWND hWndParent, POINT Point, UINT uFlags)
{
	HWND ret;
	OutTraceDW("ChildWindowFromPoint: hWndParent=%x point=(%d,%d) flags=%x\n", hWndParent, Point.x, Point.y, uFlags); 
	if(dxw.IsDesktop(hWndParent) && dxw.IsFullScreen()){
		dxw.UnmapClient(&Point);
		OutTraceDW("ChildWindowFromPointEx: FIXED point=(%d,%d)\n", Point.x, Point.y);
	}
	ret = (*pChildWindowFromPointEx)(hWndParent, Point, uFlags);
	OutTraceDW("ChildWindowFromPointEx: hwnd=%x\n", ret);
	return ret;
}

BOOL extGetMonitorInfo(HMONITOR hMonitor, LPMONITORINFO lpmi, GetMonitorInfo_Type pGetMonitorInfo)
{
	BOOL res;
	OutTrace("GetMonitorInfo: hMonitor=%x mi=MONITORINFO%s\n", hMonitor, lpmi->cbSize==sizeof(MONITORINFO)?"":"EX");
	res=(*pGetMonitorInfo)(hMonitor, lpmi);
	if(res && dxw.Windowize){
		OutTraceDW("GetMonitorInfo: FIX Work=(%d,%d)-(%d,%d) Monitor=(%d,%d)-(%d,%d) -> (%d,%d)-(%d,%d)\n", 
			lpmi->rcWork.left, lpmi->rcWork.top, lpmi->rcWork.right, lpmi->rcWork.bottom,
			lpmi->rcMonitor.left, lpmi->rcMonitor.top, lpmi->rcMonitor.right, lpmi->rcMonitor.bottom,
			0, 0, dxw.GetScreenWidth(), dxw.GetScreenHeight());
		lpmi->rcWork = dxw.GetScreenRect();
		lpmi->rcMonitor = dxw.GetScreenRect();
	}
	return res;
}

BOOL WINAPI extGetMonitorInfoA(HMONITOR hMonitor, LPMONITORINFO lpmi)
{
	return extGetMonitorInfo(hMonitor, lpmi, pGetMonitorInfoA);
}

BOOL WINAPI extGetMonitorInfoW(HMONITOR hMonitor, LPMONITORINFO lpmi)
{
	return extGetMonitorInfo(hMonitor, lpmi, pGetMonitorInfoW);
}

int WINAPI extGetUpdateRgn(HWND hWnd, HRGN hRgn, BOOL bErase)
{
	int regionType;
	regionType=(*pGetUpdateRgn)(hWnd, hRgn, bErase);
	OutTraceDW("GetUpdateRgn: hwnd=%x hrgn=%x erase=%x regionType=%x(%s)\n", 
		hWnd, hRgn, bErase, regionType, ExplainRegionType(regionType));    

	if(dxw.Windowize && dxw.IsFullScreen()){
		if(regionType == SIMPLEREGION){
			RECT rc;
			if(!pGetRgnBox) pGetRgnBox=GetRgnBox;
			regionType = (*pGetRgnBox)(hRgn, &rc);
			OutTraceDW("GetUpdateRgn: regionType=%x(%s) box=(%d,%d)-(%d,%d)\n", 
				regionType, ExplainRegionType(regionType), rc.left, rc.top, rc.right, rc.bottom);
			if(regionType == SIMPLEREGION){
				dxw.UnmapClient(&rc);
				if(SetRectRgn(hRgn, rc.left, rc.top, rc.right, rc.bottom )){
					// success
					OutTraceDW("GetUpdateRgn: FIXED box=(%d,%d)-(%d,%d)\n", rc.left, rc.top, rc.right, rc.bottom);
				}
			}
		}
#if 0
		if(regionType == COMPLEXREGION){
			RECT rc;
			if(!pGetRgnBox) pGetRgnBox=GetRgnBox;
			regionType = (*pGetRgnBox)(hRgn, &rc);
			OutTraceDW("GetUpdateRgn: regionType=%x(%s) box=(%d,%d)-(%d,%d)\n", 
				regionType, ExplainRegionType(regionType), rc.left, rc.top, rc.right, rc.bottom);
			if(regionType == COMPLEXREGION){
				//dxw.UnmapClient(&rc);
				//if(SetRectRgn(hRgn, rc.left, rc.top, rc.right, rc.bottom )){
				if(SetRectRgn(hRgn, 0, 0, dxw.GetScreenWidth(), dxw.GetScreenHeight())){
					// success
					OutTraceDW("GetUpdateRgn: FIXED box=(%d,%d)-(%d,%d)\n", rc.left, rc.top, rc.right, rc.bottom);
				}
			}
		}
#endif
   } 

    return regionType; 
}

#ifdef NOUNHOOKED
BOOL WINAPI extValidateRect(HWND hWnd, const RECT *lpRect)
{
	BOOL ret;
	if(IsTraceDW){
		if(lpRect)
			OutTrace("ValidateRect: hwnd=%x rect=(%d,%d)-(%d,%d)\n", 
				hWnd, lpRect->left, lpRect->top, lpRect->right, lpRect->bottom);
		else
			OutTrace("ValidateRect: hwnd=%x rect=NULL\n", hWnd);
	}
	ret = (*pValidateRect)(hWnd, lpRect);
	return ret;
}

int WINAPI extGetWindowTextA(HWND hWnd, LPTSTR lpString, int nMaxCount)
{
	// purpose of this wrapped call is to clear the FPS indicator (format " ~ (%d FPS)") 
	// from the window title, if present. It crashes games such as "Panzer General 3 Scorched Earth"
	// when FPS on window title is activated.
	int ret;
	OutTraceDW("GetWindowTextA: hwnd=%x MaxCount=%d\n", hWnd, nMaxCount);
	ret=(*pGetWindowTextA)(hWnd, lpString, nMaxCount);
	if(ret) OutTraceDW("GetWindowTextA: ret=%d String=\"%s\"\n", ret, lpString);
	if (ret && (dxw.dwFlags2 & SHOWFPS) && dxw.ishWndFPS(hWnd)){
		char *p;
		p=strstr(lpString, " ~ (");
		if(p){
			*p = NULL;
			ret = strlen(lpString);
			OutTraceDW("GetWindowTextA: FIXED ret=%d String=\"%s\"\n", ret, lpString);
		}
	}
	return ret;
}
#endif
