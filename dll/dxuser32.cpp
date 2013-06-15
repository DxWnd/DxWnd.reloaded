#include "dxwnd.h"
#include "dxwcore.hpp"
#include "syslibs.h"
#include "dxhook.h"

static HookEntry_Type Hooks[]={
	{"ChangeDisplaySettingsA", NULL, (FARPROC *)&pChangeDisplaySettings, (FARPROC)extChangeDisplaySettings},
	{"ChangeDisplaySettingsExA", NULL, (FARPROC *)&pChangeDisplaySettingsEx, (FARPROC)extChangeDisplaySettingsEx},
	{"BeginPaint", NULL, (FARPROC *)&pBeginPaint, (FARPROC)&extBeginPaint},
	{"EndPaint", NULL, (FARPROC *)&pEndPaint, (FARPROC)extEndPaint},
	{"ShowCursor", NULL, (FARPROC *)&pShowCursor, (FARPROC)extShowCursor},
	{"CreateDialogIndirectParamA", NULL, (FARPROC *)&pCreateDialogIndirectParam, (FARPROC)extCreateDialogIndirectParam},
	{"CreateDialogParamA", NULL, (FARPROC *)&pCreateDialogParam, (FARPROC)extCreateDialogParam},
	{"MoveWindow", NULL, (FARPROC *)&pMoveWindow, (FARPROC)extMoveWindow},
	{"ChangeDisplaySettingsA", NULL, (FARPROC *)&pChangeDisplaySettings, (FARPROC)extChangeDisplaySettings},
	{"ChangeDisplaySettingsExA", NULL, (FARPROC *)&pChangeDisplaySettingsEx, (FARPROC)extChangeDisplaySettingsEx},
	{"EnumDisplaySettingsA", NULL, (FARPROC *)&pEnumDisplaySettings, (FARPROC)extEnumDisplaySettings},
	{"GetClipCursor", NULL, (FARPROC *)&pGetClipCursor, (FARPROC)extGetClipCursor},
	{"ClipCursor", NULL, (FARPROC *)&pClipCursor, (FARPROC)extClipCursor},
	{"FillRect", NULL, (FARPROC *)&pClipCursor, (FARPROC)extFillRect},
	{"DefWindowProcA", NULL, (FARPROC *)&pDefWindowProc, (FARPROC)extDefWindowProc},
	{"CreateWindowExA", NULL, (FARPROC *)&pCreateWindowExA, (FARPROC)extCreateWindowExA},
	{"RegisterClassExA", NULL, (FARPROC *)&pRegisterClassExA, (FARPROC)extRegisterClassExA},
	{"GetSystemMetrics", NULL, (FARPROC *)&pGetSystemMetrics, (FARPROC)extGetSystemMetrics},
	{"GetDesktopWindow", NULL, (FARPROC *)&pGetDesktopWindow, (FARPROC)extGetDesktopWindow},
	{0, NULL, 0, 0} // terminator
};

static HookEntry_Type DDHooks[]={
	{"GetDC", NULL, (FARPROC *)&pGDIGetDC, (FARPROC)extDDGetDC},
	{"GetWindowDC", NULL, (FARPROC *)&pGDIGetWindowDC, (FARPROC)extDDGetDC},
	{"ReleaseDC", NULL, (FARPROC *)&pGDIReleaseDC, (FARPROC)extDDReleaseDC},
	{"InvalidateRect", NULL, (FARPROC *)&pInvalidateRect, (FARPROC)extDDInvalidateRect},
	{0, NULL, 0, 0} // terminator
};

static HookEntry_Type GDIHooks[]={
	{"GetDC", NULL, (FARPROC *)&pGDIGetDC, (FARPROC)extGDIGetDC},
	{"GetWindowDC", NULL, (FARPROC *)&pGDIGetWindowDC, (FARPROC)extGDIGetDC},
	{"ReleaseDC", NULL, (FARPROC *)&pGDIReleaseDC, (FARPROC)extGDIReleaseDC},
	{"InvalidateRect", NULL, (FARPROC *)&pInvalidateRect, (FARPROC)extInvalidateRect},
	{0, NULL, 0, 0} // terminator
};

static HookEntry_Type RemapHooks[]={
	{"ScreenToClient", NULL, (FARPROC *)&pScreenToClient, (FARPROC)extScreenToClient},
	{"ClientToScreen", NULL, (FARPROC *)&pClientToScreen, (FARPROC)extClientToScreen},
	{"GetClientRect", NULL, (FARPROC *)&pGetClientRect, (FARPROC)extGetClientRect},
	{"GetWindowRect", NULL, (FARPROC *)&pGetWindowRect, (FARPROC)extGetWindowRect},
	{"MapWindowPoints", NULL, (FARPROC *)&pMapWindowPoints, (FARPROC)extMapWindowPoints},
	{0, NULL, 0, 0} // terminator
};

static HookEntry_Type MessageHooks[]={
	{"PeekMessageA", NULL, (FARPROC *)&pPeekMessage, (FARPROC)extPeekMessage},
	{"GetMessageA", NULL, (FARPROC *)&pGetMessage, (FARPROC)extGetMessage},
	{0, NULL, 0, 0} // terminator
};

static HookEntry_Type MouseHooks[]={
	{"GetCursorPos", NULL, (FARPROC *)&pGetCursorPos, (FARPROC)extGetCursorPos},
	{"SetCursor", NULL, (FARPROC *)&pSetCursor, (FARPROC)extSetCursor},
	{"SendMessageA", NULL, (FARPROC *)&pSendMessage, (FARPROC)extSendMessage}, // ???
	//{"SetPhysicalCursorPos", NULL, (FARPROC *)&pSetCursor, (FARPROC)extSetCursor}, // ???
	{0, NULL, 0, 0} // terminator
};

static HookEntry_Type WinHooks[]={
	{"ShowWindow", NULL, (FARPROC *)&pShowWindow, (FARPROC)extShowWindow},
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
 
	if (addr=RemapLibrary(proc, hModule, (dxw.dwFlags1 & MAPGDITOPRIMARY) ? DDHooks : GDIHooks)) return addr;

	if (dxw.dwFlags1 & CLIENTREMAPPING) 
		if (addr=RemapLibrary(proc, hModule, RemapHooks)) return addr;

	if (dxw.dwFlags1 & MESSAGEPROC) 
		if (addr=RemapLibrary(proc, hModule, MessageHooks)) return addr;

	if(dxw.dwFlags1 & MODIFYMOUSE)
		if (addr=RemapLibrary(proc, hModule, MouseHooks)) return addr;

	if (dxw.dwFlags1 & (PREVENTMAXIMIZE|FIXWINFRAME|LOCKWINPOS|LOCKWINSTYLE))
		if (addr=RemapLibrary(proc, hModule, WinHooks)) return addr;

	if((dxw.dwFlags1 & (MODIFYMOUSE|SLOWDOWN|KEEPCURSORWITHIN)) || (dxw.dwFlags2 & KEEPCURSORFIXED))
		if (addr=RemapLibrary(proc, hModule, MouseHooks2)) return addr;

	return NULL;
}

void HookUser32(HMODULE hModule)
{
	HookLibrary(hModule, Hooks, "user32.dll");

	HookLibrary(hModule, (dxw.dwFlags1 & MAPGDITOPRIMARY) ? DDHooks : GDIHooks, "user32.dll");
	if (dxw.dwFlags1 & CLIENTREMAPPING) HookLibrary(hModule, RemapHooks, "user32.dll");
	if (dxw.dwFlags1 & MESSAGEPROC) HookLibrary(hModule, MessageHooks, "user32.dll");
	if(dxw.dwFlags1 & MODIFYMOUSE)HookLibrary(hModule, MouseHooks, "user32.dll");
	if (dxw.dwFlags1 & (PREVENTMAXIMIZE|FIXWINFRAME|LOCKWINPOS|LOCKWINSTYLE))HookLibrary(hModule, WinHooks, "user32.dll");
	if((dxw.dwFlags1 & (MODIFYMOUSE|SLOWDOWN|KEEPCURSORWITHIN)) || (dxw.dwFlags2 & KEEPCURSORFIXED)) HookLibrary(hModule, MouseHooks2, "user32.dll");
	return;
}
