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
#include "shareddc.hpp"
#include <Wingdi.h>

#define FIXCHILDSIZE FALSE
#define _Warn(s) MessageBox(0, s, "to do", MB_ICONEXCLAMATION)

BOOL IsChangeDisplaySettingsHotPatched = FALSE;
extern BOOL bFlippedDC;
extern HDC hFlippedDC;

//typedef BOOL (WINAPI *EnumDisplayMonitors_Type)(HDC, LPCRECT, MONITORENUMPROC, LPARAM);
//EnumDisplayMonitors_Type pEnumDisplayMonitors = NULL;
//BOOL WINAPI extEnumDisplayMonitors(HDC, LPCRECT, MONITORENUMPROC, LPARAM);

typedef BOOL (WINAPI *BringWindowToTop_Type)(HWND);
BringWindowToTop_Type pBringWindowToTop = NULL;
BOOL WINAPI extBringWindowToTop(HWND);
typedef BOOL (WINAPI *SetForegroundWindow_Type)(HWND);
SetForegroundWindow_Type pSetForegroundWindow = NULL;
BOOL WINAPI extSetForegroundWindow(HWND);
typedef HHOOK (WINAPI *SetWindowsHookEx_Type)(int, HOOKPROC, HINSTANCE, DWORD);
SetWindowsHookEx_Type pSetWindowsHookExA = NULL;
SetWindowsHookEx_Type pSetWindowsHookExW = NULL;
HHOOK WINAPI extSetWindowsHookExA(int, HOOKPROC, HINSTANCE, DWORD);
HHOOK WINAPI extSetWindowsHookExW(int, HOOKPROC, HINSTANCE, DWORD);
typedef BOOL (WINAPI *PostMessageA_Type)(HWND, UINT, WPARAM, LPARAM);
PostMessageA_Type pPostMessageA = NULL;
BOOL WINAPI extPostMessageA(HWND, UINT, WPARAM, LPARAM);
typedef HRESULT (WINAPI *MessageBoxTimeoutA_Type)(HWND, LPCSTR, LPCSTR, UINT, WORD, DWORD);
MessageBoxTimeoutA_Type pMessageBoxTimeoutA = NULL;
HRESULT WINAPI extMessageBoxTimeoutA(HWND, LPCSTR, LPCSTR, UINT, WORD, DWORD);
typedef HRESULT (WINAPI *MessageBoxTimeoutW_Type)(HWND, LPCWSTR, LPCWSTR, UINT, WORD, DWORD);
MessageBoxTimeoutW_Type pMessageBoxTimeoutW = NULL;
HRESULT WINAPI extMessageBoxTimeoutW(HWND, LPCWSTR, LPCWSTR, UINT, WORD, DWORD);
typedef BOOL (WINAPI *IsIconic_Type)(HWND);
IsIconic_Type pIsIconic = NULL;
BOOL WINAPI extIsIconic(HWND);
typedef BOOL (WINAPI *IsZoomed_Type)(HWND);
IsZoomed_Type pIsZoomed = NULL;
BOOL WINAPI extIsZoomed(HWND);
typedef HDESK (WINAPI *CreateDesktop_Type)(LPCTSTR, LPCTSTR, DEVMODE *, DWORD, ACCESS_MASK, LPSECURITY_ATTRIBUTES);
CreateDesktop_Type pCreateDesktop = NULL;
HDESK WINAPI extCreateDesktop(LPCTSTR, LPCTSTR, DEVMODE *, DWORD, ACCESS_MASK, LPSECURITY_ATTRIBUTES);
typedef BOOL (WINAPI *SwitchDesktop_Type)(HDESK);
SwitchDesktop_Type pSwitchDesktop = NULL;
BOOL WINAPI extSwitchDesktop(HDESK);
typedef HDESK (WINAPI *OpenDesktop_Type)(LPTSTR, DWORD, BOOL, ACCESS_MASK);
OpenDesktop_Type pOpenDesktop = NULL;
HDESK WINAPI extOpenDesktop(LPTSTR, DWORD, BOOL, ACCESS_MASK);
typedef BOOL (WINAPI *CloseDesktop_Type)(HDESK);
CloseDesktop_Type pCloseDesktop = NULL;
BOOL WINAPI extCloseDesktop(HDESK);
typedef int (WINAPI *ValidateRect_Type)(HWND, const RECT *);
ValidateRect_Type pValidateRect = NULL;
int WINAPI extValidateRect(HWND, const RECT *);
typedef BOOL (WINAPI *ScrollWindow_Type)(HWND, int, int, const RECT *, const RECT *);
ScrollWindow_Type pScrollWindow = NULL;
BOOL extScrollWindow(HWND, int, int, const RECT *, const RECT *);
typedef INT_PTR (WINAPI *DialogBoxParamA_Type)(HINSTANCE, LPCTSTR, HWND, DLGPROC, LPARAM);
DialogBoxParamA_Type pDialogBoxParamA = NULL;
INT_PTR WINAPI extDialogBoxParamA(HINSTANCE, LPCTSTR, HWND, DLGPROC, LPARAM);
typedef HWND (WINAPI *GetParent_Type)(HWND);
GetParent_Type pGetParent = NULL;
HWND WINAPI extGetParent(HWND);
typedef BOOL (WINAPI *InvalidateRgn_Type)(HWND, HRGN, BOOL);
InvalidateRgn_Type pInvalidateRgn = NULL;
BOOL WINAPI extInvalidateRgn(HWND, HRGN, BOOL);
typedef BOOL (WINAPI *InvertRect_Type)(HDC, const RECT *);
InvertRect_Type pInvertRect = NULL;
BOOL WINAPI extInvertRect(HDC, const RECT *);
typedef BOOL (WINAPI *ScrollDC_Type)(HDC, int, int, const RECT *, const RECT *, HRGN, LPRECT);
ScrollDC_Type pScrollDC = NULL;
BOOL WINAPI extScrollDC(HDC, int, int, const RECT *, const RECT *, HRGN, LPRECT);
typedef BOOL (WINAPI *DrawIcon_Type)(HDC hDC, int X, int Y, HICON hIcon); 
DrawIcon_Type pDrawIcon = NULL;
BOOL WINAPI extDrawIcon(HDC hDC, int X, int Y, HICON hIcon); 
typedef BOOL (WINAPI *DrawIconEx_Type)(HDC, int, int, HICON, int, int, UINT, HBRUSH, UINT);
DrawIconEx_Type pDrawIconEx = NULL;
BOOL WINAPI extDrawIconEx(HDC, int, int, HICON, int, int, UINT, HBRUSH, UINT);
typedef BOOL (WINAPI *DrawCaption_Type)(HWND, HDC, LPCRECT, UINT);
DrawCaption_Type pDrawCaption = NULL;
BOOL WINAPI extDrawCaption(HWND, HDC, LPCRECT, UINT);
typedef BOOL (WINAPI *PaintDesktop_Type)(HDC);
PaintDesktop_Type pPaintDesktop = NULL;
BOOL WINAPI extPaintDesktop(HDC);
typedef VOID (WINAPI *mouse_event_Type)(DWORD, DWORD, DWORD, DWORD, ULONG_PTR);
mouse_event_Type pmouse_event = NULL;
VOID WINAPI extmouse_event(DWORD, DWORD, DWORD, DWORD, ULONG_PTR);
typedef BOOL (WINAPI *ShowScrollBar_Type)(HWND, int, BOOL);
ShowScrollBar_Type pShowScrollBar = NULL;
BOOL WINAPI extShowScrollBar(HWND, int, BOOL);
typedef BOOL (WINAPI *DrawMenuBar_Type)(HWND);
DrawMenuBar_Type pDrawMenuBar = NULL;
BOOL WINAPI extDrawMenuBar(HWND);
//typedef BOOL (WINAPI *TranslateMessage_Type)(MSG *);
//TranslateMessage_Type pTranslateMessage = NULL;
//BOOL WINAPI extTranslateMessage(MSG *);


#ifdef TRACEPALETTE
typedef UINT (WINAPI *GetDIBColorTable_Type)(HDC, UINT, UINT, RGBQUAD *);
GetDIBColorTable_Type pGetDIBColorTable = NULL;
UINT WINAPI extGetDIBColorTable(HDC, UINT, UINT, RGBQUAD *);
typedef UINT (WINAPI *SetDIBColorTable_Type)(HDC, UINT, UINT, const RGBQUAD *);
SetDIBColorTable_Type pSetDIBColorTable = NULL;
UINT WINAPI extSetDIBColorTable(HDC, UINT, UINT, const RGBQUAD *);
#endif

static HookEntryEx_Type Hooks[]={
	
	//{HOOK_IAT_CANDIDATE, 0, "TranslateMessage", (FARPROC)TranslateMessage, (FARPROC *)&pTranslateMessage, (FARPROC)extTranslateMessage}, 
	
	{HOOK_IAT_CANDIDATE, 0, "UpdateWindow", (FARPROC)NULL, (FARPROC *)&pUpdateWindow, (FARPROC)extUpdateWindow},
	//{HOOK_IAT_CANDIDATE, 0, "GetWindowPlacement", (FARPROC)NULL, (FARPROC *)&pGetWindowPlacement, (FARPROC)extGetWindowPlacement},
	//{HOOK_IAT_CANDIDATE, 0, "SetWindowPlacement", (FARPROC)NULL, (FARPROC *)&pSetWindowPlacement, (FARPROC)extSetWindowPlacement},
	{HOOK_HOT_CANDIDATE, 0x25, "ChangeDisplaySettingsA", (FARPROC)ChangeDisplaySettingsA, (FARPROC *)&pChangeDisplaySettingsA, (FARPROC)extChangeDisplaySettingsA},
	{HOOK_HOT_CANDIDATE, 0x26, "ChangeDisplaySettingsExA", (FARPROC)ChangeDisplaySettingsExA, (FARPROC *)&pChangeDisplaySettingsExA, (FARPROC)extChangeDisplaySettingsExA},
	{HOOK_HOT_CANDIDATE, 0x28, "ChangeDisplaySettingsW", (FARPROC)NULL, (FARPROC *)&pChangeDisplaySettingsW, (FARPROC)extChangeDisplaySettingsW}, // ref. by Knights of Honor
	{HOOK_HOT_CANDIDATE, 0x27, "ChangeDisplaySettingsExW", (FARPROC)NULL, (FARPROC *)&pChangeDisplaySettingsExW, (FARPROC)extChangeDisplaySettingsExW},
	{HOOK_HOT_CANDIDATE, 0, "GetMonitorInfoA", (FARPROC)GetMonitorInfoA, (FARPROC *)&pGetMonitorInfoA, (FARPROC)extGetMonitorInfoA},
	{HOOK_HOT_CANDIDATE, 0, "GetMonitorInfoW", (FARPROC)GetMonitorInfoW, (FARPROC *)&pGetMonitorInfoW, (FARPROC)extGetMonitorInfoW},
	{HOOK_HOT_CANDIDATE, 0, "ShowCursor", (FARPROC)ShowCursor, (FARPROC *)&pShowCursor, (FARPROC)extShowCursor},
	{HOOK_IAT_CANDIDATE, 0, "CreateDialogIndirectParamA", (FARPROC)CreateDialogIndirectParamA, (FARPROC *)&pCreateDialogIndirectParam, (FARPROC)extCreateDialogIndirectParam},
	{HOOK_IAT_CANDIDATE, 0, "CreateDialogParamA", (FARPROC)CreateDialogParamA, (FARPROC *)&pCreateDialogParam, (FARPROC)extCreateDialogParam},
	{HOOK_IAT_CANDIDATE, 0, "MoveWindow", (FARPROC)MoveWindow, (FARPROC *)&pMoveWindow, (FARPROC)extMoveWindow},
	{HOOK_HOT_CANDIDATE, 0, "EnumDisplaySettingsA", (FARPROC)EnumDisplaySettingsA, (FARPROC *)&pEnumDisplaySettings, (FARPROC)extEnumDisplaySettings},
	{HOOK_IAT_CANDIDATE, 0, "GetClipCursor", (FARPROC)GetClipCursor, (FARPROC*)&pGetClipCursor, (FARPROC)extGetClipCursor},
	{HOOK_IAT_CANDIDATE, 0, "ClipCursor", (FARPROC)ClipCursor, (FARPROC *)&pClipCursor, (FARPROC)extClipCursor},
	{HOOK_IAT_CANDIDATE, 0, "DefWindowProcA", (FARPROC)DefWindowProcA, (FARPROC *)&pDefWindowProcA, (FARPROC)extDefWindowProcA},
	{HOOK_IAT_CANDIDATE, 0, "DefWindowProcW", (FARPROC)DefWindowProcW, (FARPROC *)&pDefWindowProcW, (FARPROC)extDefWindowProcW},
	{HOOK_HOT_CANDIDATE, 0, "CreateWindowExA", (FARPROC)CreateWindowExA, (FARPROC *)&pCreateWindowExA, (FARPROC)extCreateWindowExA},
	{HOOK_HOT_CANDIDATE, 0, "CreateWindowExW", (FARPROC)CreateWindowExW, (FARPROC *)&pCreateWindowExW, (FARPROC)extCreateWindowExW},
	{HOOK_IAT_CANDIDATE, 0, "RegisterClassExA", (FARPROC)RegisterClassExA, (FARPROC *)&pRegisterClassExA, (FARPROC)extRegisterClassExA},
	{HOOK_IAT_CANDIDATE, 0, "RegisterClassA", (FARPROC)RegisterClassA, (FARPROC *)&pRegisterClassA, (FARPROC)extRegisterClassA},
	{HOOK_IAT_CANDIDATE, 0, "RegisterClassExW", (FARPROC)RegisterClassExW, (FARPROC *)&pRegisterClassExW, (FARPROC)extRegisterClassExW},
	{HOOK_IAT_CANDIDATE, 0, "RegisterClassW", (FARPROC)RegisterClassW, (FARPROC *)&pRegisterClassW, (FARPROC)extRegisterClassW},
	{HOOK_HOT_CANDIDATE, 0, "GetSystemMetrics", (FARPROC)GetSystemMetrics, (FARPROC *)&pGetSystemMetrics, (FARPROC)extGetSystemMetrics},
	{HOOK_HOT_CANDIDATE, 0, "GetDesktopWindow", (FARPROC)GetDesktopWindow, (FARPROC *)&pGetDesktopWindow, (FARPROC)extGetDesktopWindow},
	{HOOK_IAT_CANDIDATE, 0, "CloseWindow", (FARPROC)NULL, (FARPROC *)&pCloseWindow, (FARPROC)extCloseWindow},
	{HOOK_IAT_CANDIDATE, 0, "DestroyWindow", (FARPROC)NULL, (FARPROC *)&pDestroyWindow, (FARPROC)extDestroyWindow},
	{HOOK_IAT_CANDIDATE, 0, "SetSysColors", (FARPROC)NULL, (FARPROC *)&pSetSysColors, (FARPROC)extSetSysColors},
	{HOOK_IAT_CANDIDATE, 0, "SetCapture", (FARPROC)NULL, (FARPROC *)&pSetCapture, (FARPROC)extSetCapture},
	{HOOK_HOT_CANDIDATE, 0, "SetWindowLongA", (FARPROC)SetWindowLongA, (FARPROC *)&pSetWindowLongA, (FARPROC)extSetWindowLongA},
	{HOOK_HOT_CANDIDATE, 0, "GetWindowLongA", (FARPROC)GetWindowLongA, (FARPROC *)&pGetWindowLongA, (FARPROC)extGetWindowLongA}, 
	{HOOK_HOT_CANDIDATE, 0, "SetWindowLongW", (FARPROC)SetWindowLongW, (FARPROC *)&pSetWindowLongW, (FARPROC)extSetWindowLongW},
	{HOOK_HOT_CANDIDATE, 0, "GetWindowLongW", (FARPROC)GetWindowLongW, (FARPROC *)&pGetWindowLongW, (FARPROC)extGetWindowLongW}, 
	{HOOK_IAT_CANDIDATE, 0, "IsWindowVisible", (FARPROC)NULL, (FARPROC *)&pIsWindowVisible, (FARPROC)extIsWindowVisible},
	// hot by MinHook since v2.03.07
	{HOOK_HOT_CANDIDATE, 0, "SystemParametersInfoA", (FARPROC)SystemParametersInfoA, (FARPROC *)&pSystemParametersInfoA, (FARPROC)extSystemParametersInfoA},
	{HOOK_HOT_CANDIDATE, 0, "SystemParametersInfoW", (FARPROC)SystemParametersInfoW, (FARPROC *)&pSystemParametersInfoW, (FARPROC)extSystemParametersInfoW},
	//{HOOK_HOT_CANDIDATE, 0, "GetActiveWindow", (FARPROC)NULL, (FARPROC *)&pGetActiveWindow, (FARPROC)extGetActiveWindow},
	//{HOOK_HOT_CANDIDATE, 0, "GetForegroundWindow", (FARPROC)GetForegroundWindow, (FARPROC *)&pGetForegroundWindow, (FARPROC)extGetForegroundWindow},
	//{HOOK_IAT_CANDIDATE, 0, "GetWindowTextA", (FARPROC)GetWindowTextA, (FARPROC *)&pGetWindowTextA, (FARPROC)extGetWindowTextA},
	//{HOOK_HOT_CANDIDATE, 0, "EnumDisplayMonitors", (FARPROC)EnumDisplayMonitors, (FARPROC *)&pEnumDisplayMonitors, (FARPROC)extEnumDisplayMonitors},
#ifdef TRACEPALETTE
	{HOOK_HOT_CANDIDATE, 0, "GetDIBColorTable", (FARPROC)GetDIBColorTable, (FARPROC *)&pGetDIBColorTable, (FARPROC)extGetDIBColorTable},
	{HOOK_HOT_CANDIDATE, 0, "SetDIBColorTable", (FARPROC)SetDIBColorTable, (FARPROC *)&pSetDIBColorTable, (FARPROC)extSetDIBColorTable},
#endif
	 
	{HOOK_HOT_CANDIDATE, 0, "BringWindowToTop", (FARPROC)BringWindowToTop, (FARPROC *)&pBringWindowToTop, (FARPROC)extBringWindowToTop},
	{HOOK_HOT_CANDIDATE, 0, "SetForegroundWindow", (FARPROC)SetForegroundWindow, (FARPROC *)&pSetForegroundWindow, (FARPROC)extSetForegroundWindow},
	{HOOK_HOT_CANDIDATE, 0, "ChildWindowFromPoint", (FARPROC)ChildWindowFromPoint, (FARPROC *)&pChildWindowFromPoint, (FARPROC)extChildWindowFromPoint},
	{HOOK_HOT_CANDIDATE, 0, "ChildWindowFromPointEx", (FARPROC)ChildWindowFromPointEx, (FARPROC *)&pChildWindowFromPointEx, (FARPROC)extChildWindowFromPointEx},
	{HOOK_HOT_CANDIDATE, 0, "WindowFromPoint", (FARPROC)WindowFromPoint, (FARPROC *)&pWindowFromPoint, (FARPROC)extWindowFromPoint},
	{HOOK_HOT_REQUIRED,  0 ,"SetWindowsHookExA", (FARPROC)SetWindowsHookExA, (FARPROC *)&pSetWindowsHookExA, (FARPROC)extSetWindowsHookExA},
	{HOOK_HOT_REQUIRED,  0 ,"SetWindowsHookExW", (FARPROC)SetWindowsHookExW, (FARPROC *)&pSetWindowsHookExW, (FARPROC)extSetWindowsHookExW},

	//{HOOK_HOT_CANDIDATE, 0, "MessageBoxTimeoutA", (FARPROC)NULL, (FARPROC *)&pMessageBoxTimeoutA, (FARPROC)extMessageBoxTimeoutA},
	//{HOOK_HOT_CANDIDATE, 0, "MessageBoxTimeoutW", (FARPROC)NULL, (FARPROC *)&pMessageBoxTimeoutW, (FARPROC)extMessageBoxTimeoutW},

	{HOOK_IAT_CANDIDATE, 0, "GetDC", (FARPROC)GetDC, (FARPROC *)&pGDIGetDC, (FARPROC)extGDIGetDC},
	{HOOK_IAT_CANDIDATE, 0, "GetDCEx", (FARPROC)GetDCEx, (FARPROC *)&pGDIGetDCEx, (FARPROC)extGDIGetDCEx},
	{HOOK_IAT_CANDIDATE, 0, "GetWindowDC", (FARPROC)GetWindowDC, (FARPROC *)&pGDIGetWindowDC, (FARPROC)extGDIGetWindowDC}, 
	{HOOK_IAT_CANDIDATE, 0, "ReleaseDC", (FARPROC)ReleaseDC, (FARPROC *)&pGDIReleaseDC, (FARPROC)extGDIReleaseDC},

	{HOOK_HOT_CANDIDATE, 0, "BeginPaint", (FARPROC)BeginPaint, (FARPROC *)&pBeginPaint, (FARPROC)extBeginPaint},
	{HOOK_HOT_CANDIDATE, 0, "EndPaint", (FARPROC)EndPaint, (FARPROC *)&pEndPaint, (FARPROC)extEndPaint},

	{HOOK_IAT_CANDIDATE, 0, "DialogBoxParamA", (FARPROC)NULL, (FARPROC *)&pDialogBoxParamA, (FARPROC)extDialogBoxParamA},

	//{HOOK_IAT_CANDIDATE, 0, "IsZoomed", (FARPROC)NULL, (FARPROC *)&pIsZoomed, (FARPROC)extIsZoomed},
	//{HOOK_HOT_CANDIDATE, 0, "IsIconic", (FARPROC)IsIconic, (FARPROC *)&pIsIconic, (FARPROC)extIsIconic},
	{HOOK_HOT_CANDIDATE, 0, "ScrollDC", (FARPROC)NULL, (FARPROC *)&pScrollDC, (FARPROC)extScrollDC},
	//{HOOK_IAT_CANDIDATE, 0, "mouse_event", (FARPROC)NULL, (FARPROC *)&pmouse_event, (FARPROC)extmouse_event}, 

	// both added to fix the Galapagos menu bar, but with no success !!!!
	{HOOK_HOT_CANDIDATE, 0, "ShowScrollBar", (FARPROC)ShowScrollBar, (FARPROC *)&pShowScrollBar, (FARPROC)extShowScrollBar},
	{HOOK_HOT_CANDIDATE, 0, "DrawMenuBar", (FARPROC)DrawMenuBar, (FARPROC *)&pDrawMenuBar, (FARPROC)extDrawMenuBar},

	{HOOK_IAT_CANDIDATE, 0, 0, NULL, 0, 0} // terminator
};

static HookEntryEx_Type RemapHooks[]={
	{HOOK_HOT_CANDIDATE, 0, "ScreenToClient", (FARPROC)ScreenToClient, (FARPROC *)&pScreenToClient, (FARPROC)extScreenToClient},
	{HOOK_HOT_CANDIDATE, 0, "ClientToScreen", (FARPROC)ClientToScreen, (FARPROC *)&pClientToScreen, (FARPROC)extClientToScreen},
	{HOOK_HOT_CANDIDATE, 0, "GetClientRect", (FARPROC)GetClientRect, (FARPROC *)&pGetClientRect, (FARPROC)extGetClientRect},
	{HOOK_HOT_CANDIDATE, 0, "GetWindowRect", (FARPROC)GetWindowRect, (FARPROC *)&pGetWindowRect, (FARPROC)extGetWindowRect},
	{HOOK_HOT_CANDIDATE, 0, "MapWindowPoints", (FARPROC)MapWindowPoints, (FARPROC *)&pMapWindowPoints, (FARPROC)extMapWindowPoints},
	{HOOK_HOT_CANDIDATE, 0, "GetUpdateRgn", (FARPROC)GetUpdateRgn, (FARPROC *)&pGetUpdateRgn, (FARPROC)extGetUpdateRgn},
	//{HOOK_IAT_CANDIDATE, 0, "GetUpdateRect", (FARPROC)GetUpdateRect, (FARPROC *)&pGetUpdateRect, (FARPROC)extGetUpdateRect},
	{HOOK_IAT_CANDIDATE, 0, "RedrawWindow", (FARPROC)RedrawWindow, (FARPROC *)&pRedrawWindow, (FARPROC)extRedrawWindow},
	{HOOK_IAT_CANDIDATE, 0, 0, NULL, 0, 0} // terminator
};

static HookEntryEx_Type SyscallHooks[]={
	{HOOK_IAT_CANDIDATE, 0, "FrameRect", (FARPROC)FrameRect, (FARPROC *)&pFrameRect, (FARPROC)extFrameRect}, 
	{HOOK_IAT_CANDIDATE, 0, "GetParent", (FARPROC)GetParent, (FARPROC *)&pGetParent, (FARPROC)extGetParent},
	{HOOK_HOT_CANDIDATE, 0, "InvalidateRgn", (FARPROC)InvalidateRgn, (FARPROC *)&pInvalidateRgn, (FARPROC)extInvalidateRgn},
	{HOOK_IAT_CANDIDATE, 0, "TabbedTextOutA", (FARPROC)TabbedTextOutA, (FARPROC *)&pTabbedTextOutA, (FARPROC)extTabbedTextOutA},
	{HOOK_IAT_CANDIDATE, 0, "TabbedTextOutW", (FARPROC)TabbedTextOutW, (FARPROC *)&pTabbedTextOutW, (FARPROC)extTabbedTextOutW},
	{HOOK_IAT_CANDIDATE, 0, "ScrollDC", (FARPROC)ScrollDC, (FARPROC *)&pScrollDC, (FARPROC)extScrollDC},
	{HOOK_IAT_CANDIDATE, 0, "InvalidateRect", (FARPROC)InvalidateRect, (FARPROC *)&pInvalidateRect, (FARPROC)extInvalidateRect},
	{HOOK_IAT_CANDIDATE, 0, "DrawTextA", (FARPROC)DrawTextA, (FARPROC *)&pDrawTextA, (FARPROC)extDrawTextA},
	{HOOK_IAT_CANDIDATE, 0, "DrawTextExA", (FARPROC)DrawTextExA, (FARPROC *)&pDrawTextExA, (FARPROC)extDrawTextExA},
	{HOOK_IAT_CANDIDATE, 0, "DrawTextW", (FARPROC)DrawTextW, (FARPROC *)&pDrawTextW, (FARPROC)extDrawTextW},
	{HOOK_IAT_CANDIDATE, 0, "DrawTextExW", (FARPROC)DrawTextExW, (FARPROC *)&pDrawTextExW, (FARPROC)extDrawTextExW},
	{HOOK_HOT_CANDIDATE, 0, "FillRect", (FARPROC)NULL, (FARPROC *)&pFillRect, (FARPROC)extFillRect},
	{HOOK_HOT_CANDIDATE, 0, "InvertRect", (FARPROC)NULL, (FARPROC *)&pInvertRect, (FARPROC)extInvertRect},
	{HOOK_HOT_CANDIDATE, 0, "DrawIcon", (FARPROC)NULL, (FARPROC *)&pDrawIcon, (FARPROC)extDrawIcon},
	{HOOK_IAT_CANDIDATE, 0, "DrawIconEx", (FARPROC)NULL, (FARPROC *)&pDrawIconEx, (FARPROC)extDrawIconEx},
	{HOOK_HOT_CANDIDATE, 0, "DrawCaption", (FARPROC)NULL, (FARPROC *)&pDrawCaption, (FARPROC)extDrawCaption},
	//TODO {HOOK_HOT_CANDIDATE, 0, "DrawEdge", (FARPROC)NULL, (FARPROC *)&pDrawEdge, (FARPROC)extDrawEdge},
	//TODO {HOOK_HOT_CANDIDATE, 0, "DrawFocusRect", (FARPROC)NULL, (FARPROC *)&pDrawFocusRect, (FARPROC)extDrawFocusRect},
	//TODO {HOOK_HOT_CANDIDATE, 0, "DrawFrameControl", (FARPROC)NULL, (FARPROC *)&pDrawFrameControl, (FARPROC)extDrawFrameControl},
	//TODO {HOOK_HOT_CANDIDATE, 0, "DrawStateA", (FARPROC)NULL, (FARPROC *)&pDrawStateA, (FARPROC)extDrawStateA},
	//TODO {HOOK_HOT_CANDIDATE, 0, "DrawStateW", (FARPROC)NULL, (FARPROC *)&pDrawStateW, (FARPROC)extDrawStateW},
	//TODO {HOOK_HOT_CANDIDATE, 0, "GrayStringA", (FARPROC)NULL, (FARPROC *)&pGrayStringA, (FARPROC)extGrayStringA},
	//TODO {HOOK_HOT_CANDIDATE, 0, "GrayStringW", (FARPROC)NULL, (FARPROC *)&pGrayStringW, (FARPROC)extGrayStringW},
	//TODO {HOOK_HOT_CANDIDATE, 0, "PaintDesktop", (FARPROC)NULL, (FARPROC *)&pPaintDesktop, (FARPROC)extPaintDesktop},
	{HOOK_IAT_CANDIDATE, 0, 0, NULL, 0, 0} // terminator
};

static HookEntryEx_Type ScaledHooks[]={
	{HOOK_IAT_CANDIDATE, 0, "ValidateRect", (FARPROC)ValidateRect, (FARPROC *)&pValidateRect, (FARPROC)extValidateRect},
	{HOOK_IAT_CANDIDATE, 0, "ScrollWindow", (FARPROC)ScrollWindow, (FARPROC *)&pScrollWindow, (FARPROC)extScrollWindow},
	{HOOK_IAT_CANDIDATE, 0, 0, NULL, 0, 0} // terminator
};

static HookEntryEx_Type PeekAllHooks[]={
	{HOOK_IAT_CANDIDATE, 0, "PeekMessageA", (FARPROC)PeekMessageA, (FARPROC *)&pPeekMessageA, (FARPROC)extPeekMessageA},
	{HOOK_IAT_CANDIDATE, 0, "PeekMessageW", (FARPROC)PeekMessageW, (FARPROC *)&pPeekMessageW, (FARPROC)extPeekMessageW},
	{HOOK_IAT_CANDIDATE, 0, 0, NULL, 0, 0} // terminator
};

static HookEntryEx_Type MouseHooks[]={
	{HOOK_HOT_CANDIDATE, 0, "GetCursorPos", (FARPROC)GetCursorPos, (FARPROC *)&pGetCursorPos, (FARPROC)extGetCursorPos},
	{HOOK_HOT_CANDIDATE, 0, "SetCursorPos", (FARPROC)SetCursorPos, (FARPROC *)&pSetCursorPos, (FARPROC)extSetCursorPos},
	{HOOK_IAT_CANDIDATE, 0, "GetCursorInfo", (FARPROC)GetCursorInfo, (FARPROC *)&pGetCursorInfo, (FARPROC)extGetCursorInfo},
	{HOOK_IAT_CANDIDATE, 0, "SetCursor", (FARPROC)SetCursor, (FARPROC *)&pSetCursor, (FARPROC)extSetCursor},
	{HOOK_IAT_CANDIDATE, 0, "SendMessageA", (FARPROC)SendMessageA, (FARPROC *)&pSendMessageA, (FARPROC)extSendMessageA}, 
	{HOOK_IAT_CANDIDATE, 0, "SendMessageW", (FARPROC)SendMessageW, (FARPROC *)&pSendMessageW, (FARPROC)extSendMessageW}, 
	//{HOOK_IAT_CANDIDATE, 0, "mouse_event", (FARPROC)NULL, (FARPROC *)&pmouse_event, (FARPROC)extmouse_event}, 
	//{HOOK_IAT_CANDIDATE, 0, "SetPhysicalCursorPos", NULL, (FARPROC *)&pSetCursor, (FARPROC)extSetCursor}, // ???
	{HOOK_IAT_CANDIDATE, 0, 0, NULL, 0, 0} // terminator
};

static HookEntryEx_Type WinHooks[]={
	{HOOK_HOT_CANDIDATE, 0, "ShowWindow", (FARPROC)ShowWindow, (FARPROC *)&pShowWindow, (FARPROC)extShowWindow},
	{HOOK_HOT_CANDIDATE, 0, "SetWindowPos", (FARPROC)SetWindowPos, (FARPROC *)&pSetWindowPos, (FARPROC)extSetWindowPos},
	{HOOK_HOT_CANDIDATE, 0, "DeferWindowPos", (FARPROC)DeferWindowPos, (FARPROC *)&pGDIDeferWindowPos, (FARPROC)extDeferWindowPos},
	{HOOK_HOT_CANDIDATE, 0, "CallWindowProcA", (FARPROC)CallWindowProcA, (FARPROC *)&pCallWindowProcA, (FARPROC)extCallWindowProcA},
	{HOOK_HOT_CANDIDATE, 0, "CallWindowProcW", (FARPROC)CallWindowProcW, (FARPROC *)&pCallWindowProcW, (FARPROC)extCallWindowProcW},
	{HOOK_IAT_CANDIDATE, 0, 0, NULL, 0, 0} // terminator
};

static HookEntryEx_Type TimeHooks[]={
	{HOOK_IAT_CANDIDATE, 0, "SetTimer", (FARPROC)SetTimer, (FARPROC *)&pSetTimer, (FARPROC)extSetTimer},
	{HOOK_IAT_CANDIDATE, 0, "KillTimer", (FARPROC)KillTimer, (FARPROC *)&pKillTimer, (FARPROC)extKillTimer},
	{HOOK_IAT_CANDIDATE, 0, 0, NULL, 0, 0} // terminator
};

static HookEntryEx_Type DesktopHooks[]={ // currently unused, needed for X-Files
	{HOOK_IAT_CANDIDATE, 0, "CreateDesktopA", (FARPROC)CreateDesktopA, (FARPROC *)&pCreateDesktop, (FARPROC)extCreateDesktop},
	{HOOK_IAT_CANDIDATE, 0, "SwitchDesktop", (FARPROC)SwitchDesktop, (FARPROC *)&pSwitchDesktop, (FARPROC)extSwitchDesktop},
	{HOOK_IAT_CANDIDATE, 0, "OpenDesktopA", (FARPROC)OpenDesktopA, (FARPROC *)&pOpenDesktop, (FARPROC)extOpenDesktop},
	{HOOK_IAT_CANDIDATE, 0, "CloseDesktop", (FARPROC)CloseDesktop, (FARPROC *)&pCloseDesktop, (FARPROC)extCloseDesktop},
	{HOOK_IAT_CANDIDATE, 0, 0, NULL, 0, 0} // terminator
};

static char *libname = "user32.dll";

void HookUser32(HMODULE hModule)
{

	HookLibraryEx(hModule, Hooks, libname);
	if (dxw.GDIEmulationMode != GDIMODE_NONE) HookLibraryEx(hModule, SyscallHooks, libname);
	if (dxw.dwFlags2 & GDISTRETCHED)	HookLibraryEx(hModule, ScaledHooks, libname);

	if (dxw.dwFlags1 & CLIENTREMAPPING) HookLibraryEx(hModule, RemapHooks, libname);
	if (dxw.dwFlags1 & (PREVENTMAXIMIZE|FIXWINFRAME|LOCKWINPOS|LOCKWINSTYLE)) HookLibraryEx(hModule, WinHooks, libname);
	if ((dxw.dwFlags1 & (MODIFYMOUSE|SLOWDOWN|KEEPCURSORWITHIN)) || (dxw.dwFlags2 & KEEPCURSORFIXED)) HookLibraryEx(hModule, MouseHooks, libname);
	if (dxw.dwFlags3 & PEEKALLMESSAGES) HookLibraryEx(hModule, PeekAllHooks, libname);
	if (dxw.dwFlags2 & TIMESTRETCH) HookLibraryEx(hModule, TimeHooks, libname);

	IsChangeDisplaySettingsHotPatched = IsHotPatchedEx(Hooks, "ChangeDisplaySettingsExA") || IsHotPatchedEx(Hooks, "ChangeDisplaySettingsExW");
	return;
}

void HookUser32Init()
{
	HookLibInitEx(Hooks);
	HookLibInitEx(SyscallHooks);
	HookLibInitEx(ScaledHooks);
	HookLibInitEx(RemapHooks);
	HookLibInitEx(MouseHooks);
	HookLibInitEx(WinHooks);
}

FARPROC Remap_user32_ProcAddress(LPCSTR proc, HMODULE hModule)
{
	FARPROC addr;
	if (addr=RemapLibraryEx(proc, hModule, Hooks)) return addr;
	if (dxw.dwFlags1 & CLIENTREMAPPING) if (addr=RemapLibraryEx(proc, hModule, RemapHooks)) return addr;
	if (dxw.GDIEmulationMode != GDIMODE_NONE) if(addr=RemapLibraryEx(proc, hModule, SyscallHooks)) return addr;

	if (dxw.dwFlags2 & GDISTRETCHED)	
		if (addr=RemapLibraryEx(proc, hModule, ScaledHooks)) return addr;  
	if (dxw.dwFlags1 & (PREVENTMAXIMIZE|FIXWINFRAME|LOCKWINPOS|LOCKWINSTYLE))
		if (addr=RemapLibraryEx(proc, hModule, WinHooks)) return addr;
	if ((dxw.dwFlags1 & (MODIFYMOUSE|SLOWDOWN|KEEPCURSORWITHIN)) || (dxw.dwFlags2 & KEEPCURSORFIXED))
		if (addr=RemapLibraryEx(proc, hModule, MouseHooks)) return addr;
	if (dxw.dwFlags3 & PEEKALLMESSAGES)
		if (addr=RemapLibraryEx(proc, hModule, PeekAllHooks)) return addr;
	if((dxw.dwFlags2 & TIMESTRETCH) && (dxw.dwFlags4 & STRETCHTIMERS)) 
		if (addr=RemapLibraryEx(proc, hModule, TimeHooks)) return addr;

	return NULL;
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

static LPCSTR sTemplateName(LPCSTR tn)
{
	static char sBuf[20+1];
	if((DWORD)tn >> 16)
		return tn;
	else {
		sprintf(sBuf, "ID:(%x)", ((DWORD)tn & 0x0000FFFF));
		return sBuf;
	}
}

// --------------------------------------------------------------------------
//
// globals, externs, static functions...
//
// --------------------------------------------------------------------------

// PrimHDC: DC handle of the selected DirectDraw primary surface. NULL when invalid.
HDC PrimHDC=NULL;

LPRECT lpClipRegion=NULL;
RECT ClipRegion;
int LastCurPosX, LastCurPosY;

extern GetDC_Type pGetDC;
extern ReleaseDC_Type pReleaseDC1;
//extern void FixWindowFrame(HWND);
extern HRESULT WINAPI sBlt(int, Blt_Type, char *, LPDIRECTDRAWSURFACE, LPRECT, LPDIRECTDRAWSURFACE, LPRECT, DWORD, LPDDBLTFX, BOOL);

LONG WINAPI MyChangeDisplaySettings(char *fname, BOOL WideChar, void *lpDevMode, DWORD dwflags)
{
	HRESULT res;
	DWORD dmFields, dmBitsPerPel, dmPelsWidth, dmPelsHeight;

	if(dwflags & CDS_TEST) {
		OutTraceDW("%s: TEST res=DISP_CHANGE_SUCCESSFUL\n", fname);
		return DISP_CHANGE_SUCCESSFUL;
	}

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
	
	// v2.03.61: bypass display mode changes also for CDS_UPDATEREGISTRY flag
	// used by "Severance: Blade of Darkness" OpenGL renderer
	if ((dwflags==0 || dwflags==CDS_FULLSCREEN || dwflags==CDS_UPDATEREGISTRY) && lpDevMode){
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
			extern GetWindowLong_Type pGetWindowLong;
			(*pGetClientRect)(hwnd, &client);
			full=client;
			dwStyle=(*pGetWindowLong)(hwnd, GWL_STYLE);
			dwExStyle=(*pGetWindowLong)(hwnd, GWL_EXSTYLE);
			hMenu = (dwStyle & WS_CHILD) ? NULL : GetMenu(hwnd);	
			AdjustWindowRectEx(&full, dwStyle, (hMenu!=NULL), dwExStyle);
			if (hMenu && (hMenu != (HMENU)-1)) __try {CloseHandle(hMenu);} __except(EXCEPTION_EXECUTE_HANDLER){};
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

	if ((dxw.dwFlags5 & CENTERTOWIN) && dxw.IsFullScreen() && (hwnd==dxw.GethWnd())){ 
		RECT wrect;
		LONG dwStyle, dwExStyle;
		HMENU hMenu;
		int minx, miny;
		wrect = dxw.GetScreenRect();
		dwStyle=(*pGetWindowLong)(hwnd, GWL_STYLE);
		dwExStyle=(*pGetWindowLong)(hwnd, GWL_EXSTYLE);
		hMenu = (dwStyle & WS_CHILD) ? NULL : GetMenu(hwnd);	
		AdjustWindowRectEx(&wrect, dwStyle, (hMenu!=NULL), dwExStyle);
		minx = wrect.right - wrect.left;
		miny = wrect.bottom - wrect.top;
		if(wp->cx < minx) wp->cx = minx;
		if(wp->cy < miny) wp->cy = miny;
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
		lpmmi->ptMaxSize.x = dxw.GetScreenWidth();
		lpmmi->ptMaxSize.y = dxw.GetScreenHeight();

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
	if(IsTraceDW){
		char sRect[81];
		if(lpRect) sprintf(sRect, "(%d,%d)-(%d,%d)", lpRect->left, lpRect->top, lpRect->right, lpRect->bottom);
		else strcpy(sRect, "NULL");
		OutTrace("InvalidateRect: hwnd=%x rect=%s erase=%x\n", hwnd, sRect, bErase);
	}

	RECT ScaledRect;
	if(dxw.IsFullScreen()) { 
		switch(dxw.GDIEmulationMode){
			case GDIMODE_STRETCHED:
			case GDIMODE_SHAREDDC:
			case GDIMODE_EMULATED:
				if(lpRect) {
					// v2.03.55: the lpRect area must NOT be altered by the call
					// effect visible in partial updates of Deadlock 2 main menu buttons
					ScaledRect = *lpRect;
					dxw.MapClient(&ScaledRect);
					lpRect = &ScaledRect;
				}
				break;
			default:
				break;
		}
	}

	return (*pInvalidateRect)(hwnd, lpRect, bErase);
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
	OutTraceDW("ShowWindow: res=%x\n", res);

	return res;
}

LONG WINAPI extGetWindowLong(GetWindowLong_Type pGetWindowLong, char *ApiName, HWND hwnd, int nIndex)
{
	LONG res;

	res=(*pGetWindowLong)(hwnd, nIndex);

	OutTraceB("%s: hwnd=%x, Index=%x(%s) res=%x\n", ApiName, hwnd, nIndex, ExplainSetWindowIndex(nIndex), res);

	if((nIndex==GWL_WNDPROC)||(nIndex==DWL_DLGPROC)){
		WNDPROC wp;
		wp=dxwws.GetProc(hwnd);
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

LONG WINAPI extSetWindowLong(HWND hwnd, int nIndex, LONG dwNewLong, SetWindowLong_Type pSetWindowLong, GetWindowLong_Type pGetWindowLong)
{
	LONG res;

	OutTraceDW("SetWindowLong: hwnd=%x, Index=%x(%s) Val=%x\n", 
		hwnd, nIndex, ExplainSetWindowIndex(nIndex), dwNewLong);

	if (dxw.Windowize){
		if(dxw.dwFlags1 & LOCKWINSTYLE){
			if(nIndex==GWL_STYLE){
				OutTraceDW("SetWindowLong: Lock GWL_STYLE=%x\n", dwNewLong);
				return (*pGetWindowLong)(hwnd, nIndex);
			}
			if(nIndex==GWL_EXSTYLE){
				OutTraceDW("SetWindowLong: Lock GWL_EXSTYLE=%x\n", dwNewLong);
				return (*pGetWindowLong)(hwnd, nIndex);
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

	if (((nIndex==GWL_WNDPROC)||(nIndex==DWL_DLGPROC)) && 
		dxw.IsFullScreen() &&			// v2.02.51 - see A10 Cuba....
		!(dxw.dwFlags6 & NOWINDOWHOOKS)){	// v2.03.41 - debug flag
		WNDPROC lres;
		WNDPROC OldProc;
		// GPL fix
		if(dxw.IsRealDesktop(hwnd) && dxw.Windowize) {
			hwnd=dxw.GethWnd();
			OutTraceDW("SetWindowLong: DESKTOP hwnd, FIXING hwnd=%x\n",hwnd);
		}
		// end of GPL fix

		OldProc = (WNDPROC)(*pGetWindowLong)(hwnd, nIndex);
		// v2.02.70 fix
		if((OldProc==extWindowProc) || 
			(OldProc==extChildWindowProc)||
			(OldProc==extDialogWindowProc)) 
			OldProc=dxwws.GetProc(hwnd);
		dxwws.PutProc(hwnd, (WNDPROC)dwNewLong);
		res=(LONG)OldProc;
		SetLastError(0);
		lres=(WNDPROC)(*pSetWindowLong)(hwnd, nIndex, (LONG)extWindowProc);
		if(!lres && GetLastError())OutTraceE("SetWindowLong: ERROR err=%d at %d\n", GetLastError(), __LINE__);
	}
	else {
		res=(*pSetWindowLong)(hwnd, nIndex, dwNewLong);
	}

	OutTraceDW("SetWindowLong: hwnd=%x, nIndex=%x, Val=%x, res=%x\n", hwnd, nIndex, dwNewLong, res);
	return res;
}

LONG WINAPI extSetWindowLongA(HWND hwnd, int nIndex, LONG dwNewLong)
{
	return extSetWindowLong(hwnd, nIndex, dwNewLong, pSetWindowLongA, pGetWindowLongA);
}

LONG WINAPI extSetWindowLongW(HWND hwnd, int nIndex, LONG dwNewLong)
{
	return extSetWindowLong(hwnd, nIndex, dwNewLong, pSetWindowLongW, pGetWindowLongW);
}

BOOL WINAPI extSetWindowPos(HWND hwnd, HWND hWndInsertAfter, int X, int Y, int cx, int cy, UINT uFlags)
{
	BOOL res;

	OutTraceDW("SetWindowPos: hwnd=%x%s pos=(%d,%d) dim=(%d,%d) Flags=%x\n", 
		hwnd, dxw.IsFullScreen()?"(FULLSCREEN)":"", X, Y, cx, cy, uFlags);

	// when not in fullscreen mode, just proxy the call
	if (!dxw.IsFullScreen()){
		res=(*pSetWindowPos)(hwnd, hWndInsertAfter, X, Y, cx, cy, uFlags);
		if(!res)OutTraceE("SetWindowPos: ERROR err=%d at %d\n", GetLastError(), __LINE__);
		return res;
	}

	// in fullscreen, but a child window inside .....
	if (!dxw.IsDesktop(hwnd)){
		RECT r;
		r.left = X;
		r.right = X + cx;
		r.top = Y;
		r.bottom = Y + cy;
		if ((*pGetWindowLong)(hwnd, GWL_STYLE) & WS_CHILD){
			r = dxw.MapClientRect(&r);
		}
		else {
			//r = dxw.MapWindowRect(&r);
		}		
		X = r.left;
		Y = r.top;
		cx = r.right - r.left;
		cy = r.bottom - r.top;

		res=(*pSetWindowPos)(hwnd, hWndInsertAfter, X, Y, cx, cy, uFlags);
		if(!res)OutTraceE("SetWindowPos: ERROR err=%d at %d\n", GetLastError(), __LINE__);

		//HFONT hFont;
		//hFont=CreateFont (
		//	30, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, 
		//	FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, 
		//	DEFAULT_PITCH | FF_SWISS, NULL); // "Arial");
		//SendMessage (hwnd, WM_SETFONT, WPARAM (hFont), TRUE);	

		return res;
	}

	if (dxw.dwFlags1 & LOCKWINPOS){
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
	dwCurStyle=(*pGetWindowLong)(hwnd, GWL_STYLE);
	dwExStyle=(*pGetWindowLong)(hwnd, GWL_EXSTYLE);
	// BEWARE: from MSDN -  If the window is a child window, the return value is undefined. 
	hMenu = (dwCurStyle & WS_CHILD) ? NULL : GetMenu(hwnd);	
	AdjustWindowRectEx(&rect, dwCurStyle, (hMenu!=NULL), dwExStyle);
	if (hMenu && (hMenu != (HMENU)-1)) __try {CloseHandle(hMenu);} __except(EXCEPTION_EXECUTE_HANDLER){};
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
{ return extSendMessage("SendMessageA", pSendMessageA, hwnd, Msg, wParam, lParam); }
LRESULT WINAPI extSendMessageW(HWND hwnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{ return extSendMessage("SendMessageW", pSendMessageW, hwnd, Msg, wParam, lParam); }

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

	if((dxw.dwFlags1 & HIDEHWCURSOR) && dxw.IsFullScreen()) while((*pShowCursor)(0) >= 0);
	if(dxw.dwFlags2 & SHOWHWCURSOR) while((*pShowCursor)(1) < 0);

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
		// v2.03.41
		POINT cur;
		cur.x = x;
		cur.y = y;
		dxw.MapWindow(&cur);
		x = cur.x;
		y = cur.y;
	}

	res=0;
	if (pSetCursorPos) res=(*pSetCursorPos)(x,y);

	OutTraceC("SetCursorPos: res=%x XY=(%d,%d)->(%d,%d)\n", res, PrevX, PrevY, x, y);
	return res;
}

static BOOL WINAPI extPeekMessage(PeekMessage_Type pPeekMessage, LPMSG lpMsg, HWND hwnd, UINT wMsgFilterMin, UINT wMsgFilterMax, UINT wRemoveMsg)
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

BOOL WINAPI extPeekMessageA(LPMSG lpMsg, HWND hwnd, UINT wMsgFilterMin, UINT wMsgFilterMax, UINT wRemoveMsg)
{ return extPeekMessage(pPeekMessageA, lpMsg, hwnd, wMsgFilterMin, wMsgFilterMax, wRemoveMsg); }
BOOL WINAPI extPeekMessageW(LPMSG lpMsg, HWND hwnd, UINT wMsgFilterMin, UINT wMsgFilterMax, UINT wRemoveMsg)
{ return extPeekMessage(pPeekMessageW, lpMsg, hwnd, wMsgFilterMin, wMsgFilterMax, wRemoveMsg); }

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
			if(dxwws.GetSize(hwnd, &w, &h)){
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

	if(dxw.IsRealDesktop(hwnd)) {
		// v2.03.52, v2.03.61: fix for "Storm Angel" and "Geneforge" :
		// replace the real desktop with the virtual one only if that doesn't cause troubles.
		HWND hwnd_try = dxw.GethWnd();
		if ((*pGetWindowRect)(hwnd, lpRect)) hwnd = hwnd_try;
	}

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
	// used also in "Full Pipe" activemovie
	// used also in "NBA Live 99" menu screen	

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
	
	ret=(*pMapWindowPoints)(hWndFrom, hWndTo, lpPoints, cPoints);
	// v2.03.16: now must scale every point (fixes "NBA Live 99")
	// v2.03.18: in some cases it should not! "New Your Race"...
	// v2.03.56: scale only on scaled modes
	switch(dxw.GDIEmulationMode){
		case GDIMODE_SHAREDDC:
		case GDIMODE_EMULATED:
		default:
			break;
		case GDIMODE_STRETCHED:
			for(pi=0; pi<cPoints; pi++){
				dxw.UnmapClient(&lpPoints[pi]);
			}			
			if(IsDebug){
				OutTrace("Mapped points: ");
				for(pi=0; pi<cPoints; pi++) OutTrace("(%d,%d)", lpPoints[pi].x, lpPoints[pi].y);
				OutTrace("\n");
			}
			break;
	}

	// If the function succeeds, the low-order word of the return value is the number of pixels 
	// added to the horizontal coordinate of each source point in order to compute the horizontal 
	// coordinate of each destination point. (In addition to that, if precisely one of hWndFrom 
	// and hWndTo is mirrored, then each resulting horizontal coordinate is multiplied by -1.) 
	// The high-order word is the number of pixels added to the vertical coordinate of each source
	// point in order to compute the vertical coordinate of each destination point.

	OutTraceDW("MapWindowPoints: ret=%x (%d,%d)\n", ret, (ret&0xFFFF0000)>>16, ret&0x0000FFFF);
	return ret;
}

HWND WINAPI extGetDesktopWindow(void)
{
	HWND res;

	if((!dxw.Windowize) || (dxw.dwFlags5 & DIABLOTWEAK)) {
		HWND ret;
		ret = (*pGetDesktopWindow)();
		OutTraceDW("GetDesktopWindow: BYPASS ret=%x\n", ret);
		return ret;
	}

	OutTraceDW("GetDesktopWindow: FullScreen=%x\n", dxw.IsFullScreen());
	if (dxw.IsFullScreen()){ 
		if(dxw.dwFlags6 & CREATEDESKTOP){
			extern HWND hDesktopWindow;
			OutTraceDW("GetDesktopWindow: returning desktop emulated hwnd=%x\n", hDesktopWindow);
			return hDesktopWindow;
		}
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

ATOM WINAPI extRegisterClassExA(WNDCLASSEXA *lpwcx)
{
	ATOM ret;
	OutTraceDW("RegisterClassExA: PROXED ClassName=%s style=%x(%s) WndProc=%x cbClsExtra=%d cbWndExtra=%d hInstance=%x\n", 
		lpwcx->lpszClassName, lpwcx->style, ExplainStyle(lpwcx->style), lpwcx->lpfnWndProc, lpwcx->cbClsExtra, lpwcx->cbWndExtra, lpwcx->hInstance);
	ret = (*pRegisterClassExA)(lpwcx);
	OutTraceDW("RegisterClassExA: atom=%x\n", ret);
	return ret;
}

ATOM WINAPI extRegisterClassA(WNDCLASSA *lpwcx)
{
	ATOM ret;
	// referenced by Syberia, together with RegisterClassExA
	OutTraceDW("RegisterClassA: PROXED ClassName=%s style=%x(%s) WndProc=%x cbClsExtra=%d cbWndExtra=%d hInstance=%x\n", 
		lpwcx->lpszClassName, lpwcx->style, ExplainStyle(lpwcx->style), lpwcx->lpfnWndProc, lpwcx->cbClsExtra, lpwcx->cbWndExtra, lpwcx->hInstance);
	ret = (*pRegisterClassA)(lpwcx);
	OutTraceDW("RegisterClassA: atom=%x\n", ret);
	return ret;
}

ATOM WINAPI extRegisterClassExW(WNDCLASSEXW *lpwcx)
{
	ATOM ret;
	OutTraceDW("RegisterClassExW: PROXED ClassName=%ls style=%x(%s) WndProc=%x cbClsExtra=%d cbWndExtra=%d hInstance=%x\n", 
		lpwcx->lpszClassName, lpwcx->style, ExplainStyle(lpwcx->style), lpwcx->lpfnWndProc, lpwcx->cbClsExtra, lpwcx->cbWndExtra, lpwcx->hInstance);
	ret = (*pRegisterClassExW)(lpwcx);
	OutTraceDW("RegisterClassExW: atom=%x\n", ret);
	return ret;
}

ATOM WINAPI extRegisterClassW(WNDCLASSW *lpwcx)
{
	ATOM ret;
	OutTraceDW("RegisterClassW: PROXED ClassName=%ls style=%x(%s) WndProc=%x cbClsExtra=%d cbWndExtra=%d hInstance=%x\n", 
		lpwcx->lpszClassName, lpwcx->style, ExplainStyle(lpwcx->style), lpwcx->lpfnWndProc, lpwcx->cbClsExtra, lpwcx->cbWndExtra, lpwcx->hInstance);
	ret = (*pRegisterClassW)(lpwcx);
	OutTraceDW("RegisterClassW: atom=%x\n", ret);
	return ret;
}

static void HookChildWndProc(HWND hwnd, DWORD dwStyle, LPCTSTR ApiName)
{
	// child window inherit the father's windproc, so if it's redirected to
	// a hooker (either extWindowProc or extChildWindowProc) you have to retrieve
	// the correct value (dxwws.GetProc) before saving it (dxwws.PutProc).
	long res;
	WNDPROC pWindowProc;

	if(dxw.dwFlags6 & NOWINDOWHOOKS) return;

	pWindowProc = (WNDPROC)(*pGetWindowLong)(hwnd, GWL_WNDPROC);
	if((pWindowProc == extWindowProc) || 
		(pWindowProc == extChildWindowProc) ||
		(pWindowProc == extDialogWindowProc)){ // avoid recursions 
		HWND Father;
		WNDPROC pFatherProc;
		Father=(*pGetParent)(hwnd);
		pFatherProc=dxwws.GetProc(Father);
		OutTraceDW("%s: WndProc=%s father=%x WndProc=%x\n", ApiName, 
			(pWindowProc == extWindowProc) ? "extWindowProc" : ((pWindowProc == extChildWindowProc) ? "extChildWindowProc" : "extDialogWindowProc"), 
			Father, pFatherProc);
		pWindowProc = pFatherProc;
	}
	dxwws.PutProc(hwnd, pWindowProc);
	if(dwStyle & WS_CHILD){
		OutTraceDW("%s: Hooking CHILD hwnd=%x father WindowProc %x->%x\n", ApiName, hwnd, pWindowProc, extChildWindowProc);
		res=(*pSetWindowLong)(hwnd, GWL_WNDPROC, (LONG)extChildWindowProc);
	}
	else { // must be dwStyle & WS_DLGFRAME
		OutTraceDW("%s: Hooking DLGFRAME hwnd=%x father WindowProc %x->%x\n", ApiName, hwnd, pWindowProc, extDialogWindowProc);
		res=(*pSetWindowLong)(hwnd, GWL_WNDPROC, (LONG)extDialogWindowProc);
	}
	if(!res) OutTraceE("%s: SetWindowLong ERROR %x\n", ApiName, GetLastError());
}

HWND hControlParentWnd = NULL;

// see https://msdn.microsoft.com/en-us/library/windows/desktop/ms632679%28v=vs.85%29.aspx

static BOOL IsFullscreenWindow(
	void *lpClassName, 
	DWORD dwStyle, 
	DWORD dwExStyle, 
	HWND hWndParent,
	int x, 
	int y, 
	int nWidth, 
	int nHeight)
{
	if (dwExStyle & WS_EX_CONTROLPARENT) return FALSE; // "Diablo" fix
	if ((dwStyle & WS_CHILD) && (!dxw.IsDesktop(hWndParent))) return FALSE; // Diablo fix
	// if maximized. 
	if(dwStyle & WS_MAXIMIZE) return TRUE; 
	// go through here only when WS_CHILD of desktop window
	if((x == CW_USEDEFAULT) && (dwStyle & (WS_POPUP|WS_CHILD))) x = y = 0;
	if(nWidth == CW_USEDEFAULT){
		if (dwStyle & (WS_POPUP|WS_CHILD)) nWidth = nHeight = 0;
		else nWidth = dxw.GetScreenWidth() - x;
		}
	// msdn undocumented case: x,y=(-1000, CW_USEDEFAULT) w,h=(CW_USEDEFAULT,CW_USEDEFAULT) in "Imperialism"
	if(nHeight == CW_USEDEFAULT){
		y = 0;
		nHeight = dxw.GetScreenHeight();
		}
	// if bigger than screen ...
	if((x<=0)&&
		(y<=0)&&
		(nWidth>=(int)dxw.GetScreenWidth())&&
		(nHeight>=(int)dxw.GetScreenHeight())) return TRUE;
	return FALSE;
}

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

	if(dxw.dwFlags6 & CREATEDESKTOP){
		extern HWND hDesktopWindow;
		if (dxw.IsRealDesktop(hWndParent)){
			OutTraceE("%s: new parent win %x->%x\n", ApiName, hWndParent, hDesktopWindow);
			hWndParent=hDesktopWindow;
		}
	}

	// v2.1.92: fixes size & position for auxiliary big window, often used
	// for intro movies etc. : needed for ......
	// evidently, this was supposed to be a fullscreen window....
	// v2.1.100: fixes for "The Grinch": this game creates a new main window for OpenGL
	// rendering using CW_USEDEFAULT placement and 800x600 size while the previous
	// main win was 640x480 only!
	// v2.02.13: if it's a WS_CHILD window, don't reposition the x,y, placement for BIG win.
	// v2.02.30: fix (Fable - lost chapters) Fable creates a bigger win with negative x,y coordinates. 
	// v2.03.53: revised code, logic moved to IsFullscreenWindow

	if(IsFullscreenWindow(lpClassName, dwStyle, dwExStyle, hWndParent, x, y, nWidth, nHeight)){
		RECT screen;
		POINT upleft = {0,0};

		// update virtual screen size if it has grown 
		// v2.03.58 fix: do't consider CW_USEDEFAULT ad a big unsigned integer!! Fixes "Imperialism".
		if((nWidth != CW_USEDEFAULT) && (nHeight != CW_USEDEFAULT)) dxw.SetScreenSize(nWidth, nHeight);

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
			x=dxw.iPosX;
			y=dxw.iPosY;
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

	// from here on, fullscreen is garanteed

	if (dwStyle & WS_CHILD){
		// tested on Gangsters: coordinates must be window-relative!!!
		// Age of Empires....
		dxw.MapClient(&x, &y, &nWidth, &nHeight);
		OutTraceDW("%s: fixed WS_CHILD pos=(%d,%d) size=(%d,%d)\n",
		ApiName, x, y, nWidth, nHeight);
	}
	else {
		if ((dwExStyle & WS_EX_CONTROLPARENT) || (dwStyle & WS_POPUP)){
			// needed for "Diablo", that creates a new WS_EX_CONTROLPARENT window that must be
			// overlapped to the directdraw surface.
			// needed for "Riven", that creates a new WS_POPUP window with the menu bar that must be
			// overlapped to the directdraw surface.
			dxw.MapWindow(&x, &y, &nWidth, &nHeight);
			OutTraceDW("%s: fixed pos=(%d,%d) size=(%d,%d)\n",
			ApiName, x, y, nWidth, nHeight);
		}
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
		(*pSetWindowLong)(hwnd, GWL_STYLE, (dxw.dwFlags2 & MODALSTYLE) ? 0 : WS_OVERLAPPEDWINDOW);
		(*pSetWindowLong)(hwnd, GWL_EXSTYLE, 0); 
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
		dxwws.PutSize(hwnd, iOrigW, iOrigH);

	// "Hoyle Casino Empire" needs to be in a maximized state to continue after the intro movie.
	// Sending a SW_MAXIMIZE message intercepted by the PREVENTMAXIMIZE handling fixes the problem.
	//if (dxw.IsFullScreen() && (dxw.dwFlags1 & PREVENTMAXIMIZE)){
	if ((hwnd == dxw.GethWnd()) && dxw.IsFullScreen() && (dxw.dwFlags1 & PREVENTMAXIMIZE)){
		OutTraceDW("%s: entering maximized state\n", ApiName); 
		(*pShowWindow)(hwnd, SW_MAXIMIZE);
	}

	if(dxw.dwFlags1 & CLIPCURSOR) dxw.SetClipCursor();

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
	OutTraceB("CreateWindowExW: DEBUG fullscreen=%x mainwin=%x screen=(%d,%d)\n", 
		dxw.IsFullScreen(), dxw.GethWnd(), dxw.GetScreenWidth(), dxw.GetScreenHeight());

	if((dxw.dwFlags6 & STRETCHMOVIES) && !wcscmp(lpWindowName, L"ActiveMovie Window")){
		RECT MainWin;
		(*pGetClientRect)(dxw.GethWnd(), &MainWin);
		nWidth = MainWin.right;
		nHeight = MainWin.bottom;
	}

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
	OutTraceB("CreateWindowExA: DEBUG fullscreen=%x mainwin=%x screen=(%d,%d)\n", 
		dxw.IsFullScreen(), dxw.GethWnd(), dxw.GetScreenWidth(), dxw.GetScreenHeight());

	if((dxw.dwFlags6 & STRETCHMOVIES) && !strcmp(lpWindowName, "ActiveMovie Window")){
		RECT MainWin;
		(*pGetClientRect)(dxw.GethWnd(), &MainWin);
		nWidth = MainWin.right;
		nHeight = MainWin.bottom;
	}

	return extCreateWindowCommon("CreateWindowExA", FALSE, dwExStyle, (void *)lpClassName, (void *)lpWindowName, dwStyle, x, y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam); 
}

LRESULT WINAPI extCallWindowProcA(WNDPROC lpPrevWndFunc, HWND hwnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	// v2.02.30: fix (Imperialism II): apply to main window only !!!
	HRESULT res;

	res = -1;
	if(hwnd == dxw.GethWnd()) res=FixWindowProc("CallWindowProcA", hwnd, Msg, wParam, &lParam);

	if (res==(HRESULT)-1)
		return (*pCallWindowProcA)(lpPrevWndFunc, hwnd, Msg, wParam, lParam);
	else
		return res;
}

LRESULT WINAPI extCallWindowProcW(WNDPROC lpPrevWndFunc, HWND hwnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	// v2.02.30: fix (Imperialism II): apply to main window only !!!
	HRESULT res;

	res = -1;
	if(hwnd == dxw.GethWnd()) res=FixWindowProc("CallWindowProcW", hwnd, Msg, wParam, &lParam);

	if (res==(HRESULT)-1)
		return (*pCallWindowProcW)(lpPrevWndFunc, hwnd, Msg, wParam, lParam);
	else
		return res;
}

LRESULT WINAPI extDefWindowProcA(HWND hwnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	// v2.02.30: fix (Imperialism II): apply to main window only !!!
	// v2.03.50: fix - do clip cursor only after the window has got focus
	HRESULT res;
	res = (HRESULT)-1;
	if(hwnd == dxw.GethWnd()) res=FixWindowProc("DefWindowProcA", hwnd, Msg, wParam, &lParam);
	if (res==(HRESULT)-1) res = (*pDefWindowProcA)(hwnd, Msg, wParam, lParam);
	if((Msg == WM_SETFOCUS) && (dxw.dwFlags1 & CLIPCURSOR)) dxw.SetClipCursor();
	return res;
}

LRESULT WINAPI extDefWindowProcW(HWND hwnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	// v2.02.30: fix (Imperialism II): apply to main window only !!!
	// v2.03.50: fix - do clip cursor only after the window has got focus
	HRESULT res;
	res = (HRESULT)-1;
	if(hwnd == dxw.GethWnd()) res=FixWindowProc("DefWindowProcW", hwnd, Msg, wParam, &lParam);
	if (res==(HRESULT)-1) res = (*pDefWindowProcW)(hwnd, Msg, wParam, lParam);
	if((Msg == WM_SETFOCUS) && (dxw.dwFlags1 & CLIPCURSOR)) dxw.SetClipCursor();
	return res;
}

static int HandleRect(char *ApiName, void *pFun, HDC hdc, const RECT *lprc, HBRUSH hbr)
{
	// used for both FillRect and FrameRect calls
	int res;
	RECT rc;
	OutTraceDW("%s: hdc=%x hbrush=%x rect=(%d,%d)-(%d,%d)\n", ApiName, hdc, hbr, lprc->left, lprc->top, lprc->right, lprc->bottom);

	if(dxw.dwFlags4 & NOFILLRECT) {
		OutTraceDW("%s: SUPPRESS\n", ApiName, hdc, hbr, lprc->left, lprc->top, lprc->right, lprc->bottom);
		return TRUE;
	}

	memcpy(&rc, lprc, sizeof(rc));

	// Be careful: when you call CreateCompatibleDC with NULL DC, it is created a memory DC
	// with same characteristics as desktop. That would return true from the call to
	// dxw.IsRealDesktop(WindowFromDC(hdc)) because WindowFromDC(hdc) is null.
	// So, it's fundamental to check also the hdc type (OBJ_DC is a window's DC)

	if((dxw.IsRealDesktop(WindowFromDC(hdc)) && (OBJ_DC == (*pGetObjectType)(hdc)))) {
		HWND VirtualDesktop;
		VirtualDesktop=dxw.GethWnd();
		if(VirtualDesktop==NULL){
			OutTraceDW("%s: no virtual desktop\n", ApiName);
			return TRUE;
		}
		OutTraceDW("%s: remapped hdc to virtual desktop hwnd=%x\n", ApiName, dxw.GethWnd());
		hdc=(*pGDIGetDC)(dxw.GethWnd());
	}

	if(dxw.IsToRemap(hdc)) {
		if(rc.left < 0) rc.left = 0;
		if(rc.top < 0) rc.top = 0;
		if((DWORD)rc.right > dxw.GetScreenWidth()) rc.right = dxw.GetScreenWidth();
		if((DWORD)rc.bottom > dxw.GetScreenHeight()) rc.bottom = dxw.GetScreenHeight();

		switch(dxw.GDIEmulationMode){
			case GDIMODE_SHAREDDC:
				sdc.GetPrimaryDC(hdc);
				res=(*(FillRect_Type)pFun)(sdc.GetHdc(), &rc, hbr);
				sdc.PutPrimaryDC(hdc, TRUE, rc.left, rc.top, rc.right-rc.left, rc.bottom-rc.top);
				return res;
				break;
			case GDIMODE_STRETCHED: 
				dxw.MapClient(&rc);
				OutTraceDW("%s: fixed rect=(%d,%d)-(%d,%d)\n", ApiName, rc.left, rc.top, rc.right, rc.bottom);	
				break;
			default:
				break;
		}
	}
	else {
		// when not in fullscreen mode, just proxy the call
		// but check coordinates: some games may use excessive coordinates: see "Premier Manager 98"
		RECT client;
		HWND hwnd;
		hwnd=WindowFromDC(hdc);
		(*pGetClientRect)(hwnd, &client);
		if(rc.left < 0) rc.left=0;
		if(rc.top < 0) rc.top=0;
		if(rc.right > client.right) rc.right=client.right;
		if(rc.bottom > client.bottom) rc.bottom=client.bottom;
		OutTraceDW("%s: remapped hdc from hwnd=%x to rect=(%d,%d)-(%d,%d)\n", ApiName, hwnd, rc.left, rc.top, rc.right, rc.bottom);
	}

	res=(*(FillRect_Type)pFun)(hdc, &rc, hbr);
	return res;
}

int WINAPI extFillRect(HDC hdc, const RECT *lprc, HBRUSH hbr)
{
	return HandleRect("FillRect", (void *)pFillRect, hdc, lprc, hbr);
}

int WINAPI extFrameRect(HDC hdc, const RECT *lprc, HBRUSH hbr)
{
	return HandleRect("FramelRect", (void *)pFrameRect, hdc, lprc, hbr);
}

BOOL WINAPI extInvertRect(HDC hdc, const RECT *lprc)
{
	int res;
	RECT rc;
	OutTraceDW("InvertRect: hdc=%x rect=(%d,%d)-(%d,%d)\n", hdc, lprc->left, lprc->top, lprc->right, lprc->bottom);

	memcpy(&rc, lprc, sizeof(rc));

	if(dxw.IsToRemap(hdc)) {
		switch(dxw.GDIEmulationMode){
			case GDIMODE_SHAREDDC:
				sdc.GetPrimaryDC(hdc);
				res=(*pInvertRect)(sdc.GetHdc(), &rc);
				sdc.PutPrimaryDC(hdc, TRUE, rc.left, rc.top, rc.right-rc.left, rc.bottom-rc.top);
				return res;
				break;
			case GDIMODE_STRETCHED: 
				dxw.MapClient(&rc);
				OutTraceDW("InvertRect: fixed rect=(%d,%d)-(%d,%d)\n", rc.left, rc.top, rc.right, rc.bottom);	
				break;
			default:
				break;
		}
	}

	res=(*pInvertRect)(hdc, &rc);
	return res;
}

int WINAPI extValidateRect(HWND hwnd, const RECT *lprc)
{
	int res;
	RECT rc;

	OutTraceDW("ValidateRect: hwnd=%x rect=(%d,%d)-(%d,%d)\n",
		hwnd, lprc->left, lprc->top, lprc->right, lprc->bottom);

	memcpy(&rc, lprc, sizeof(rc));

	if(dxw.IsFullScreen()) {
		if(dxw.IsRealDesktop(hwnd)) hwnd=dxw.GethWnd();		
		dxw.MapClient(&rc);
		OutTraceDW("ValidateRect: fixed rect=(%d,%d)-(%d,%d)\n", rc.left, rc.top, rc.right, rc.bottom);
	}

	res=(*pValidateRect)(hwnd, &rc);
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

	if(dxw.dwFlags1 & CLIENTREMAPPING){ //v2.03.61
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
		// v2.03.11: fix for "SubCulture" mouse movement
		if(lpRect && dxw.Windowize)	*lpRect = dxw.GetScreenRect();
		if(IsTraceDDRAW){
			if (lpRect)
				OutTrace("GetClipCursor: rect=(%d,%d)-(%d,%d) ret=%d\n", 
					lpRect->left,lpRect->top,lpRect->right,lpRect->bottom, ret);
			else 
				OutTrace("GetClipCursor: rect=(NULL) ret=%d\n", ret);
		}		
		return ret;
	}

	if(lpRect){
		if(lpClipRegion)
			*lpRect=ClipRegion;
		else 
			*lpRect=dxw.GetScreenRect();
		OutTraceDW("GetClipCursor: rect=(%d,%d)-(%d,%d) ret=%d\n", 
			lpRect->left,lpRect->top,lpRect->right,lpRect->bottom, TRUE);
	}

	return TRUE;
}

LONG WINAPI extEnumDisplaySettings(LPCTSTR lpszDeviceName, DWORD iModeNum, DEVMODE *lpDevMode)
{
	LONG res;
	OSVERSIONINFO osinfo;

	osinfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	(*pGetVersionExA)(&osinfo);
	OutTraceDW("EnumDisplaySettings: Devicename=%s ModeNum=%x OS=%d.%d\n", lpszDeviceName, iModeNum, osinfo.dwMajorVersion, osinfo.dwMinorVersion);

	if(dxw.dwFlags4 & NATIVERES){
		// lists video card native resolutions, though faking emulated color resolutions
		if((osinfo.dwMajorVersion >= 6) && (dxw.dwFlags1 & EMULATESURFACE)){
			switch(iModeNum){
			case ENUM_CURRENT_SETTINGS: 
			case ENUM_REGISTRY_SETTINGS: // lie ...
				res=(*pEnumDisplaySettings)(lpszDeviceName, iModeNum, lpDevMode);
				if(dxw.dwFlags2 & INIT8BPP) lpDevMode->dmBitsPerPel = 8;
				if(dxw.dwFlags2 & INIT16BPP) lpDevMode->dmBitsPerPel = 16;
				if(dxw.dwFlags3 & FORCE16BPP) lpDevMode->dmBitsPerPel = 16;
				break;
			default:
				res=(*pEnumDisplaySettings)(lpszDeviceName, iModeNum / SUPPORTED_DEPTHS_NUMBER, lpDevMode);
				lpDevMode->dmBitsPerPel = (DWORD)SupportedDepths[iModeNum % SUPPORTED_DEPTHS_NUMBER];
				break;
			}
		}
		else
			res=(*pEnumDisplaySettings)(lpszDeviceName, iModeNum, lpDevMode);

	}
	else { // simulated modes: VGA or HDTV
		//int SupportedDepths[5]={8,16,24,32,0};
		SupportedRes_Type *SupportedRes;
		SupportedRes = (dxw.dwFlags4 & SUPPORTHDTV) ? &SupportedHDTVRes[0] : &SupportedSVGARes[0];
		res=(*pEnumDisplaySettings)(lpszDeviceName, ENUM_CURRENT_SETTINGS, lpDevMode);
		switch(iModeNum){
		case ENUM_CURRENT_SETTINGS: 
		case ENUM_REGISTRY_SETTINGS: // lie ...
			lpDevMode->dmPelsHeight = 600;
			lpDevMode->dmPelsWidth = 800;
			if(dxw.dwFlags2 & INIT8BPP) lpDevMode->dmBitsPerPel = 8;
			if(dxw.dwFlags2 & INIT16BPP) lpDevMode->dmBitsPerPel = 16;
			if(dxw.dwFlags3 & FORCE16BPP) lpDevMode->dmBitsPerPel = 16;
			break;
		default:
			lpDevMode->dmPelsHeight = SupportedRes[iModeNum / 4].h;
			lpDevMode->dmPelsWidth  = SupportedRes[iModeNum / 4].w;
			lpDevMode->dmBitsPerPel = SupportedDepths[iModeNum % 4];
			if(lpDevMode->dmPelsHeight == 0) res = 0; // end of list
			break;
		}
	}

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

	OutTraceDW("EnumDisplaySettings: color=%dBPP size=(%dx%d) refresh=%dHz\n", 
		lpDevMode->dmBitsPerPel, lpDevMode->dmPelsWidth, lpDevMode->dmPelsHeight, lpDevMode->dmDisplayFrequency);
	return res;
}

LONG WINAPI extChangeDisplaySettingsA(DEVMODEA *lpDevMode, DWORD dwflags)
{
	if(IsTraceDDRAW){
		char sInfo[1024];
		strcpy(sInfo, "");
		if (lpDevMode) sprintf(sInfo, " DeviceName=%s fields=%x(%s) size=(%d x %d) bpp=%d", 
			lpDevMode->dmDeviceName, lpDevMode->dmFields, ExplainDevModeFields(lpDevMode->dmFields),
			lpDevMode->dmPelsWidth, lpDevMode->dmPelsHeight, lpDevMode->dmBitsPerPel);
		OutTrace("ChangeDisplaySettingsA: lpDevMode=%x flags=%x(%s)%s\n", 
			lpDevMode, dwflags, ExplainChangeDisplaySettingsFlags(dwflags), sInfo);
	}

	if(dxw.Windowize)
		return MyChangeDisplaySettings("ChangeDisplaySettingsA", FALSE, lpDevMode, dwflags);
	else
		return (*pChangeDisplaySettingsExA)(NULL, lpDevMode, NULL, dwflags, NULL);
}

LONG WINAPI extChangeDisplaySettingsW(DEVMODEW *lpDevMode, DWORD dwflags)
{
	if(IsTraceDDRAW){
		char sInfo[1024];
		strcpy(sInfo, "");
		if (lpDevMode) sprintf(sInfo, " DeviceName=%ls fields=%x(%s) size=(%d x %d) bpp=%d", 
			lpDevMode->dmDeviceName, lpDevMode->dmFields, ExplainDevModeFields(lpDevMode->dmFields),
			lpDevMode->dmPelsWidth, lpDevMode->dmPelsHeight, lpDevMode->dmBitsPerPel);
		OutTrace("ChangeDisplaySettingsW: lpDevMode=%x flags=%x(%s)%s\n", 
			lpDevMode, dwflags, ExplainChangeDisplaySettingsFlags(dwflags), sInfo);
	}

	if(dxw.Windowize)
		return MyChangeDisplaySettings("ChangeDisplaySettingsW", TRUE, lpDevMode, dwflags);
	else
		return (*pChangeDisplaySettingsW)(lpDevMode, dwflags);
}

LONG WINAPI extChangeDisplaySettingsExA(LPCTSTR lpszDeviceName, DEVMODEA *lpDevMode, HWND hwnd, DWORD dwflags, LPVOID lParam)
{
	if(IsTraceDDRAW){
		char sInfo[1024];
		strcpy(sInfo, "");
		if (lpDevMode) sprintf(sInfo, " DeviceName=%s fields=%x(%s) size=(%d x %d) bpp=%d", 
			lpDevMode->dmDeviceName, lpDevMode->dmFields, ExplainDevModeFields(lpDevMode->dmFields),
			lpDevMode->dmPelsWidth, lpDevMode->dmPelsHeight, lpDevMode->dmBitsPerPel);
		OutTrace("ChangeDisplaySettingsExA: DeviceName=%s lpDevMode=%x flags=%x(%s)%s\n", 
			lpszDeviceName, lpDevMode, dwflags, ExplainChangeDisplaySettingsFlags(dwflags), sInfo);
	}

	if(dxw.Windowize)
		return MyChangeDisplaySettings("ChangeDisplaySettingsExA", FALSE, lpDevMode, dwflags);
	else
		return (*pChangeDisplaySettingsExA)(lpszDeviceName, lpDevMode, hwnd, dwflags, lParam);
}

LONG WINAPI extChangeDisplaySettingsExW(LPCTSTR lpszDeviceName, DEVMODEW *lpDevMode, HWND hwnd, DWORD dwflags, LPVOID lParam)
{
	if(IsTraceDDRAW){
		char sInfo[1024];
		strcpy(sInfo, "");
		if (lpDevMode) sprintf(sInfo, " DeviceName=%ls fields=%x(%s) size=(%d x %d) bpp=%d", 
			lpDevMode->dmDeviceName, lpDevMode->dmFields, ExplainDevModeFields(lpDevMode->dmFields),
			lpDevMode->dmPelsWidth, lpDevMode->dmPelsHeight, lpDevMode->dmBitsPerPel);
		OutTrace("ChangeDisplaySettingsExW: DeviceName=%ls lpDevMode=%x flags=%x(%s)%s\n", 
			lpszDeviceName, lpDevMode, dwflags, ExplainChangeDisplaySettingsFlags(dwflags), sInfo);
	}

	if(dxw.Windowize)
		return MyChangeDisplaySettings("ChangeDisplaySettingsExW", TRUE, lpDevMode, dwflags);
	else
		return (*pChangeDisplaySettingsExW)(lpszDeviceName, lpDevMode, hwnd, dwflags, lParam);
}

static HDC WINAPI sGetDC(HWND hwnd, char *ApiName)
{
	// to do: add parameter and reference to pGDIGetDCEx to merge properly GetDC and GetDCEx
	HDC ret;
	HWND lochwnd;

	if(!dxw.IsFullScreen()) return(*pGDIGetDC)(hwnd);	

	lochwnd=hwnd;

	if (dxw.IsRealDesktop(hwnd)) {
		OutTraceDW("%s: desktop remapping hwnd=%x->%x\n", ApiName, hwnd, dxw.GethWnd());
		lochwnd=dxw.GethWnd();
	}

	switch(dxw.GDIEmulationMode){
		case GDIMODE_EMULATED:
			ret=dxw.AcquireEmulatedDC(lochwnd);
			break;
		case GDIMODE_STRETCHED:
		case GDIMODE_SHAREDDC:
		default:
			ret=(*pGDIGetDC)(lochwnd);
			break;
	}

	if(ret){
		OutTraceDW("%s: hwnd=%x ret=%x\n", ApiName, lochwnd, ret);
	}
	else{
		int err;
		err=GetLastError();
		OutTraceE("%s ERROR: hwnd=%x err=%d at %d\n", ApiName, lochwnd, err, __LINE__);
		if((err==ERROR_INVALID_WINDOW_HANDLE) && (lochwnd!=hwnd)){
			ret=(*pGDIGetDC)(hwnd);	
			if(ret)
				OutTraceDW("%s: hwnd=%x ret=%x\n", ApiName, hwnd, ret);
			else
				OutTraceE("%s ERROR: hwnd=%x err=%d at %d\n", ApiName, hwnd, GetLastError(), __LINE__);
		}
	}

	return ret;
}

HDC WINAPI extGDIGetDC(HWND hwnd)
{
	OutTraceDW("GDI.GetDC: hwnd=%x\n", hwnd);
	return sGetDC(hwnd, "GDI.GetDC");
}

HDC WINAPI extGDIGetDCEx(HWND hwnd, HRGN hrgnClip, DWORD flags)
{
	// used by Star Wars Shadow of the Empire
	OutTraceDW("GDI.GetDCEx: hwnd=%x hrgnClip=%x flags=%x(%s)\n", hwnd, hrgnClip, flags, ExplainGetDCExFlags(flags));
	return sGetDC(hwnd, "GDI.GetDCEx");
}

HDC WINAPI extGDIGetWindowDC(HWND hwnd)
{
	OutTraceDW("GDI.GetWindowDC: hwnd=%x\n", hwnd);

	// if not fullscreen or not desktop win, just proxy the call
	if(!dxw.IsFullScreen() || !dxw.IsDesktop(hwnd)){
		HDC ret;
		ret=(*pGDIGetWindowDC)(hwnd);
		OutTraceDW("GDI.GetWindowDC: hwnd=%x hdc=%x\n", hwnd, ret);
		return ret;
	}

	return sGetDC(hwnd, "GDI.GetWindowDC");
}

int WINAPI extGDIReleaseDC(HWND hwnd, HDC hDC)
{
	int res;

	OutTraceDW("GDI.ReleaseDC: hwnd=%x hdc=%x\n", hwnd, hDC);

	if (dxw.IsRealDesktop(hwnd)) hwnd=dxw.GethWnd();
	if(hwnd == 0) return(TRUE);

	switch(dxw.GDIEmulationMode){
		case GDIMODE_EMULATED:
			res=dxw.ReleaseEmulatedDC(hwnd);
			break;
		case GDIMODE_STRETCHED:
		case GDIMODE_SHAREDDC:
		default:
			res=(*pGDIReleaseDC)(hwnd, hDC);
			break;
	}

	if (!res) OutTraceE("GDI.ReleaseDC ERROR: err=%d at %d\n", GetLastError(), __LINE__);
	return(res);
}

HDC WINAPI extBeginPaint(HWND hwnd, LPPAINTSTRUCT lpPaint)
{
	HDC hdc;

	OutTraceDW("GDI.BeginPaint: hwnd=%x lpPaint=%x FullScreen=%x\n", hwnd, lpPaint, dxw.IsFullScreen());

	// avoid access to real desktop
	if(dxw.IsRealDesktop(hwnd)) hwnd=dxw.GethWnd();

	hdc=(*pBeginPaint)(hwnd, lpPaint);

	// if not in fullscreen mode, that's all!
	if(!dxw.IsFullScreen()) return hdc;

	switch(dxw.GDIEmulationMode){
		case GDIMODE_STRETCHED:
			if(dxw.dwFlags1 & CLIENTREMAPPING) dxw.UnmapClient(&(RECT)(lpPaint->rcPaint));
			break;
		case GDIMODE_EMULATED:
			HDC EmuHDC; 
			EmuHDC = dxw.AcquireEmulatedDC(hwnd); 
			lpPaint->hdc=EmuHDC;
			hdc = EmuHDC;
			break;
		case GDIMODE_SHAREDDC:
#if 0
			sdc.GetPrimaryDC(hdc);
			lpPaint->hdc = sdc.GetHdc();
			(*pBeginPaint)(hwnd, lpPaint);
			lpPaint->hdc = hdc;
			sdc.PutPrimaryDC(hdc, FALSE);
#endif
			break;
		default:
			break;
	}

	OutTraceDW("GDI.BeginPaint: hdc=%x rcPaint=(%d,%d)-(%d,%d)\n", 
		hdc, lpPaint->rcPaint.left, lpPaint->rcPaint.top, lpPaint->rcPaint.right, lpPaint->rcPaint.bottom);
	return hdc;
}

BOOL WINAPI extEndPaint(HWND hwnd, const PAINTSTRUCT *lpPaint)
{
	BOOL ret;

	OutTraceDW("GDI.EndPaint: hwnd=%x lpPaint=%x lpPaint.hdc=%x lpPaint.rcpaint=(%d,%d)-(%d-%d)\n", 
		hwnd, lpPaint, lpPaint->hdc, lpPaint->rcPaint.left, lpPaint->rcPaint.top, lpPaint->rcPaint.right, lpPaint->rcPaint.bottom);

	// if not fullscreen or not desktop win, just proxy the call
	if(!dxw.IsFullScreen()){
		ret=(*pEndPaint)(hwnd, lpPaint);
		return ret;
	}

	// avoid access to real desktop
	if(dxw.IsRealDesktop(hwnd)) hwnd=dxw.GethWnd();
	switch(dxw.GDIEmulationMode){
		case GDIMODE_EMULATED:
			ret=dxw.ReleaseEmulatedDC(hwnd);
			break;
		case GDIMODE_SHAREDDC:
#if 1
			if(lpPaint) dxw.MapClient((LPRECT)&(lpPaint->rcPaint));
			ret=(*pEndPaint)(hwnd, lpPaint);
#else
			PAINTSTRUCT Paint;
			Paint = *lpPaint;
			Paint.hdc = sdc.GetHdc();
			(*pEndPaint)(hwnd, &Paint);
			if(lpPaint) dxw.MapClient((LPRECT)&(lpPaint->rcPaint));
			ret=(*pEndPaint)(hwnd, lpPaint);
#endif
			break;
		default:
			ret=(*pEndPaint)(hwnd, lpPaint);
			break;
	}

	if(ret){
		OutTraceDW("GDI.EndPaint: hwnd=%x ret=%x\n", hwnd, ret);
	}
	else{
		OutTraceE("GDI.EndPaint ERROR: err=%d at %d\n", GetLastError(), __LINE__);
	}

	return ret;
}

HWND WINAPI extCreateDialogIndirectParam(HINSTANCE hInstance, LPCDLGTEMPLATE lpTemplate, HWND hWndParent, DLGPROC lpDialogFunc, LPARAM lParamInit)
{
	HWND RetHWND;
	BOOL FullScreen;
	FullScreen = dxw.IsFullScreen();
	OutTraceDW("CreateDialogIndirectParam: hInstance=%x lpTemplate=(style=%x extstyle=%x items=%d pos=(%d,%d) size=(%dx%d)) hWndParent=%x lpDialogFunc=%x lParamInit=%x\n",
		hInstance, 
		lpTemplate->style, lpTemplate->dwExtendedStyle, lpTemplate->cdit, lpTemplate->x, lpTemplate->y, lpTemplate->cx, lpTemplate->cy,
		hWndParent, lpDialogFunc, lParamInit);
	if(dxw.IsFullScreen() && hWndParent==NULL) hWndParent=dxw.GethWnd();
	dxw.SetFullScreen(FALSE);
	RetHWND=(*pCreateDialogIndirectParam)(hInstance, lpTemplate, hWndParent, lpDialogFunc, lParamInit);
	dxw.SetFullScreen(FullScreen);

	// v2.02.73: redirect lpDialogFunc only when it is nor NULL
	if(	lpDialogFunc &&
		!(dxw.dwFlags6 & NOWINDOWHOOKS)){	// v2.03.41 - debug option
		dxwws.PutProc(RetHWND, (WNDPROC)lpDialogFunc);
		if(!(*pSetWindowLong)(RetHWND, DWL_DLGPROC, (LONG)extDialogWindowProc))
			OutTraceE("SetWindowLong: ERROR err=%d at %d\n", GetLastError(), __LINE__);
	}

	OutTraceDW("CreateDialogIndirectParam: hwnd=%x\n", RetHWND);
	return RetHWND;
}

HWND WINAPI extCreateDialogParam(HINSTANCE hInstance, LPCTSTR lpTemplateName, HWND hWndParent, DLGPROC lpDialogFunc, LPARAM lParamInit)
{
	HWND RetHWND;
	BOOL FullScreen;
	FullScreen = dxw.IsFullScreen();
	OutTraceDW("CreateDialogParam: hInstance=%x lpTemplateName=%s hWndParent=%x lpDialogFunc=%x lParamInit=%x\n",
		hInstance, sTemplateName(lpTemplateName), hWndParent, lpDialogFunc, lParamInit);
	if(hWndParent==NULL) hWndParent=dxw.GethWnd();
	dxw.SetFullScreen(FALSE);
	RetHWND=(*pCreateDialogParam)(hInstance, lpTemplateName, hWndParent, lpDialogFunc, lParamInit);
	dxw.SetFullScreen(FullScreen);

	// v2.02.73: redirect lpDialogFunc only when it is nor NULL: fix for "LEGO Stunt Rally"
	if(	lpDialogFunc &&
		!(dxw.dwFlags6 & NOWINDOWHOOKS)){	// v2.03.41 - debug option
		dxwws.PutProc(RetHWND, (WNDPROC)lpDialogFunc);
		if(!(*pSetWindowLong)(RetHWND, DWL_DLGPROC, (LONG)extDialogWindowProc))
			OutTraceE("SetWindowLong: ERROR err=%d at %d\n", GetLastError(), __LINE__);
	}

	OutTraceDW("CreateDialogParam: hwnd=%x\n", RetHWND);
	return RetHWND;
}

BOOL WINAPI extMoveWindow(HWND hwnd, int X, int Y, int nWidth, int nHeight, BOOL bRepaint)
{
	BOOL ret;
	OutTraceDW("MoveWindow: hwnd=%x xy=(%d,%d) size=(%d,%d) repaint=%x fullscreen=%x\n",
		hwnd, X, Y, nWidth, nHeight, bRepaint, dxw.IsFullScreen());

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

		if (dxw.IsFullScreen() && (dxw.dwFlags1 & CLIENTREMAPPING)){
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
			OutTraceDW("MoveWindow: DEBUG client=(%d,%d) screen=(%d,%d)\n",
				client.right, client.bottom, dxw.GetScreenWidth(), dxw.GetScreenHeight());
			OutTraceDW("MoveWindow: hwnd=%x child=%x relocated to xy=(%d,%d) size=(%d,%d)\n",
				hwnd, isChild, X, Y, nWidth, nHeight);
		}
		else{
			if((X==0)&&(Y==0)&&(nWidth==dxw.GetScreenWidth())&&(nHeight==dxw.GetScreenHeight())){
				// evidently, this was supposed to be a fullscreen window....
				RECT screen;
				DWORD dwStyle;
				POINT upleft = {0,0};
				(*pGetClientRect)(dxw.GethWnd(),&screen);
				(*pClientToScreen)(dxw.GethWnd(),&upleft);
				if((dwStyle=(*pGetWindowLong)(hwnd, GWL_STYLE)) && WS_CHILDWINDOW){
					// Big main child window: see "Reah"
					X=Y=0;
				}
				else{
					// Regular big main window, usual case.
				X=upleft.x;
				Y=upleft.y;
				}
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

BOOL WINAPI extScrollDC(HDC hdc, int dx, int dy, const RECT *lprcScroll, const RECT *lprcClip, HRGN hrgnUpdate, LPRECT lprcUpdate)
{
	BOOL res;
	if(IsTraceDW){
		char sRect[81];
		if(lprcScroll) sprintf(sRect, "(%d,%d)-(%d,%d)", lprcScroll->left, lprcScroll->top, lprcScroll->right, lprcScroll->bottom);
		else strcpy(sRect, "NULL");
		char sClip[81];
		if(lprcClip) sprintf(sClip, "(%d,%d)-(%d,%d)", lprcClip->left, lprcClip->top, lprcClip->right, lprcClip->bottom);
		else strcpy(sClip, "NULL");
		OutTraceDW("ScrollDC: hdc=%x dxy=(%d,%d) scrollrect=%s cliprect=%s hrgn=%x\n", 
			hdc, dx, dy, sRect, sClip, hrgnUpdate);
	}

	if(dxw.IsToRemap(hdc)){
		switch(dxw.GDIEmulationMode){
			case GDIMODE_SHAREDDC:
				sdc.GetPrimaryDC(hdc);
				res=(*pScrollDC)(sdc.GetHdc(), dx, dy, lprcScroll, lprcClip, hrgnUpdate, lprcUpdate);
				sdc.PutPrimaryDC(hdc, TRUE, lprcUpdate->left, lprcUpdate->top, lprcUpdate->right-lprcUpdate->left, lprcUpdate->bottom-lprcUpdate->top);
				return res;
				break;
			case GDIMODE_EMULATED:
#if 0
				// not working with 688(I) sonar !!!
				if(dxw.IsVirtual(hdc)){
					RECT rcScroll, rcClip;
					if(lprcScroll) {
						rcScroll = *lprcScroll;
						OffsetRect(&rcScroll, dxw.VirtualOffsetX, dxw.VirtualOffsetY);
					}
					if(lprcClip) {
						rcClip = *lprcClip;
						OffsetRect(&rcClip, dxw.VirtualOffsetX, dxw.VirtualOffsetY);
					}
					res=(*pScrollDC)(hdc, dx, dy, &rcScroll, &rcClip, hrgnUpdate, lprcUpdate);
					return res;	
				}
#endif
				break;
			default:
				break;
		}
	}

	res=(*pScrollDC)(hdc, dx, dy, lprcScroll, lprcClip, hrgnUpdate, lprcUpdate);
	return res;	
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

	if(dxw.IsToRemap(hdc)){
		switch(dxw.GDIEmulationMode){
			case GDIMODE_SHAREDDC:
				sdc.GetPrimaryDC(hdc);
				res=(*pTabbedTextOutA)(sdc.GetHdc(), X, Y, lpString, nCount, nTabPositions, lpnTabStopPositions, nTabOrigin);
				sdc.PutPrimaryDC(hdc, TRUE);
				return res;
				break;
			case GDIMODE_STRETCHED: 
				dxw.MapClient(&X, &Y);
				break;
			case GDIMODE_EMULATED:
				if(dxw.IsVirtual(hdc)){
					X+=dxw.VirtualOffsetX;
					Y+=dxw.VirtualOffsetY;
				}
				break;
			default:
				break;
		}
		OutTraceDW("TabbedTextOutA: fixed dest=(%d,%d)\n", X, Y);
	}

	res=(*pTabbedTextOutA)(hdc, X, Y, lpString, nCount, nTabPositions, lpnTabStopPositions, nTabOrigin);
	return res;
}

LONG WINAPI extTabbedTextOutW(HDC hdc, int X, int Y, LPCWSTR lpString, int nCount, int nTabPositions, const LPINT lpnTabStopPositions, int nTabOrigin)
{
	BOOL res;
	OutTraceDW("TabbedTextOutW: hdc=%x xy=(%d,%d) nCount=%d nTP=%d nTOS=%d str=(%d)\"%ls\"\n", 
		hdc, X, Y, nCount, nTabPositions, nTabOrigin, lpString);

	if(dxw.IsToRemap(hdc)){
		switch(dxw.GDIEmulationMode){
			case GDIMODE_SHAREDDC:
				sdc.GetPrimaryDC(hdc);
				res=(*pTabbedTextOutW)(sdc.GetHdc(), X, Y, lpString, nCount, nTabPositions, lpnTabStopPositions, nTabOrigin);
				sdc.PutPrimaryDC(hdc, TRUE);
				return res;
				break;
			case GDIMODE_STRETCHED: 
				dxw.MapClient(&X, &Y);
				break;
			case GDIMODE_EMULATED:
				if(dxw.IsVirtual(hdc)){
					X+=dxw.VirtualOffsetX;
					Y+=dxw.VirtualOffsetY;
				}
				break;
			default:
				break;
		}
		OutTraceDW("TabbedTextOutW: fixed dest=(%d,%d)\n", X, Y);
	}

	res=(*pTabbedTextOutW)(hdc, X, Y, lpString, nCount, nTabPositions, lpnTabStopPositions, nTabOrigin);
	return res;
}

BOOL WINAPI extDestroyWindow(HWND hWnd)
{
	// v2.02.43: "Empire Earth" builds test surfaces that must be destroyed!
	// v2.03.20: "Prince of Persia 3D" destroys the main window that must be preserved! 
	BOOL res;

	OutTraceB("DestroyWindow: hwnd=%x\n", hWnd);
	if (hWnd == dxw.GethWnd()) {
		if(dxw.dwFlags6 & NODESTROYWINDOW) {
			OutTraceDW("DestroyWindow: do NOT destroy main hwnd=%x\n", hWnd);
			return TRUE;
		}
		OutTraceDW("DestroyWindow: destroy main hwnd=%x\n", hWnd);
		dxw.SethWnd(NULL);
	}
	if (hControlParentWnd && (hWnd == hControlParentWnd)) {
		OutTraceDW("DestroyWindow: destroy control parent hwnd=%x\n", hWnd);
		hControlParentWnd = NULL;
	}
	res=(*pDestroyWindow)(hWnd);
	if(!res)OutTraceE("DestroyWindow: ERROR err=%d\n", GetLastError());

	if(dxw.dwFlags7 & NOWINERRORS) return TRUE; // v2.03.69: suppress unessential errors
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
	OutTraceDW("DrawTextA: hdc=%x rect=(%d,%d)-(%d,%d) Format=%x(%s) Text=(%d)\"%s\"\n", 
		hdc, lpRect->left, lpRect->top, lpRect->right, lpRect->bottom, uFormat, ExplainDTFormat(uFormat), nCount, lpchText);
    gFixed = TRUE; // semaphore to avoid multiple scaling with HOT patching
	if(dxw.IsToRemap(hdc)){

		switch(dxw.GDIEmulationMode){
			case GDIMODE_SHAREDDC:
				sdc.GetPrimaryDC(hdc);
				ret=(*pDrawTextA)(sdc.GetHdc(), lpchText, nCount, lpRect, uFormat);
				if(nCount)
					sdc.PutPrimaryDC(hdc, TRUE, lpRect->left, lpRect->top, lpRect->right-lpRect->left, lpRect->bottom-lpRect->top);
				else
					sdc.PutPrimaryDC(hdc, FALSE); // Diablo makes a DrawText of nuull string in the intro ...
				return ret;
				break;
			case GDIMODE_STRETCHED: 
				dxw.MapClient((RECT *)lpRect);
				OutTraceDW("DrawTextA: fixed rect=(%d,%d)-(%d,%d)\n", lpRect->left, lpRect->top, lpRect->right, lpRect->bottom);
				ret=(*pDrawTextA)(hdc, lpchText, nCount, lpRect, uFormat);
				dxw.UnmapClient((RECT *)lpRect);
				OutTraceDW("DrawTextA: fixed output rect=(%d,%d)-(%d,%d)\n", lpRect->left, lpRect->top, lpRect->right, lpRect->bottom);
				break;
			default:
				ret=(*pDrawTextA)(hdc, lpchText, nCount, lpRect, uFormat);
				break;
		}
	}
	else {
		ret=(*pDrawTextA)(hdc, lpchText, nCount, lpRect, uFormat);
	}
	gFixed = FALSE;

	// if nCount is zero, DrawRect returns 0 as text heigth, but this is not an error! (ref. "Imperialism II")
	if(nCount && !ret) OutTraceE("DrawTextA: ERROR ret=%x err=%d\n", ret, GetLastError()); 
	return ret;
}

int WINAPI extDrawTextExA(HDC hdc, LPTSTR lpchText, int nCount, LPRECT lpRect, UINT dwDTFormat, LPDRAWTEXTPARAMS lpDTParams)
{
	int ret;
	OutTraceDW("DrawTextExA: hdc=%x rect=(%d,%d)-(%d,%d) DTFormat=%x Text=(%d)\"%s\"\n", 
		hdc, lpRect->left, lpRect->top, lpRect->right, lpRect->bottom, dwDTFormat, nCount, lpchText);
       if (IsDebug){
            if(lpDTParams)
                  OutTrace("DTParams: size=%d (L,R)margins=(%d,%d) TabLength=%d lDrawn=%d\n",
                  lpDTParams->cbSize, lpDTParams->iLeftMargin, lpDTParams->iRightMargin,
                  lpDTParams->iTabLength, lpDTParams->uiLengthDrawn);
            else
                  OutTrace("DTParams: NULL\n");
      }

    gFixed = TRUE; // semaphore to avoid multiple scaling with HOT patching
	if(dxw.IsToRemap(hdc)){
		switch(dxw.GDIEmulationMode){
			case GDIMODE_SHAREDDC:
				sdc.GetPrimaryDC(hdc);
				ret=(*pDrawTextExA)(sdc.GetHdc(), lpchText, nCount, lpRect, dwDTFormat, lpDTParams);
				sdc.PutPrimaryDC(hdc, TRUE, lpRect->left, lpRect->top, lpRect->right-lpRect->left, lpRect->bottom-lpRect->top);
				return ret;
				break;
			case GDIMODE_STRETCHED: 
				dxw.MapClient((RECT *)lpRect);
				OutTraceDW("DrawTextA: fixed rect=(%d,%d)-(%d,%d)\n", lpRect->left, lpRect->top, lpRect->right, lpRect->bottom);
				ret=(*pDrawTextExA)(hdc, lpchText, nCount, lpRect, dwDTFormat, lpDTParams);
				dxw.UnmapClient((RECT *)lpRect);
				OutTraceDW("DrawTextA: fixed output rect=(%d,%d)-(%d,%d)\n", lpRect->left, lpRect->top, lpRect->right, lpRect->bottom);
				break;
			default:
				ret=(*pDrawTextExA)(hdc, lpchText, nCount, lpRect, dwDTFormat, lpDTParams);
				break;
		}
	}
	else {
		ret=(*pDrawTextExA)(hdc, lpchText, nCount, lpRect, dwDTFormat, lpDTParams);
	}
	gFixed = FALSE;

	// if nCount is zero, DrawRect returns 0 as text heigth, but this is not an error! (ref. "Imperialism II")
	if(nCount && !ret) OutTraceE("DrawTextA: ERROR ret=%x err=%d\n", ret, GetLastError()); 
	return ret;
}

int WINAPI extDrawTextW(HDC hdc, LPCWSTR lpchText, int nCount, LPRECT lpRect, UINT uFormat)
{
	int ret;
	OutTraceDW("DrawTextW: hdc=%x rect=(%d,%d)-(%d,%d) Format=%x(%s) Text=(%d)\"%ls\"\n", 
		hdc, lpRect->left, lpRect->top, lpRect->right, lpRect->bottom, uFormat, ExplainDTFormat(uFormat), nCount, lpchText);

    gFixed = TRUE; // semaphore to avoid multiple scaling with HOT patching
	if(dxw.IsToRemap(hdc)){

		switch(dxw.GDIEmulationMode){
			case GDIMODE_SHAREDDC:
				sdc.GetPrimaryDC(hdc);
				ret=(*pDrawTextW)(sdc.GetHdc(), lpchText, nCount, lpRect, uFormat);
				sdc.PutPrimaryDC(hdc, TRUE, lpRect->left, lpRect->top, lpRect->right-lpRect->left, lpRect->bottom-lpRect->top);
				return ret;
				break;
			case GDIMODE_STRETCHED: 
				dxw.MapClient((RECT *)lpRect);
				OutTraceDW("DrawTextW: fixed rect=(%d,%d)-(%d,%d)\n", lpRect->left, lpRect->top, lpRect->right, lpRect->bottom);
				ret=(*pDrawTextW)(hdc, lpchText, nCount, lpRect, uFormat);
				dxw.UnmapClient((RECT *)lpRect);
				OutTraceDW("DrawTextW: fixed output rect=(%d,%d)-(%d,%d)\n", lpRect->left, lpRect->top, lpRect->right, lpRect->bottom);
				break;
			default:
				ret=(*pDrawTextW)(hdc, lpchText, nCount, lpRect, uFormat);
				break;
		}
	}
	else {
		ret=(*pDrawTextW)(hdc, lpchText, nCount, lpRect, uFormat);
	}
	gFixed = FALSE;

	// if nCount is zero, DrawRect returns 0 as text heigth, but this is not an error! (ref. "Imperialism II")
	if(nCount && !ret) OutTraceE("DrawTextW: ERROR ret=%x err=%d\n", ret, GetLastError()); 
	return ret;
}

int WINAPI extDrawTextExW(HDC hdc, LPCWSTR lpchText, int nCount, LPRECT lpRect, UINT dwDTFormat, LPDRAWTEXTPARAMS lpDTParams)
{
	int ret;
	OutTraceDW("DrawTextExW: hdc=%x rect=(%d,%d)-(%d,%d) DTFormat=%x Text=(%d)\"%ls\"\n", 
		hdc, lpRect->left, lpRect->top, lpRect->right, lpRect->bottom, dwDTFormat, nCount, lpchText);
       if (IsDebug){
            if(lpDTParams)
                  OutTrace("DTParams: size=%d (L,R)margins=(%d,%d) TabLength=%d lDrawn=%d\n",
                  lpDTParams->cbSize, lpDTParams->iLeftMargin, lpDTParams->iRightMargin,
                  lpDTParams->iTabLength, lpDTParams->uiLengthDrawn);
            else
                  OutTrace("DTParams: NULL\n");
      }

    gFixed = TRUE; // semaphore to avoid multiple scaling with HOT patching
	if(dxw.IsToRemap(hdc)){
		switch(dxw.GDIEmulationMode){
			case GDIMODE_SHAREDDC:
				sdc.GetPrimaryDC(hdc);
				ret=(*pDrawTextExW)(sdc.GetHdc(), lpchText, nCount, lpRect, dwDTFormat, lpDTParams);
				sdc.PutPrimaryDC(hdc, TRUE, lpRect->left, lpRect->top, lpRect->right-lpRect->left, lpRect->bottom-lpRect->top);
				return ret;
				break;
			case GDIMODE_STRETCHED: 
				dxw.MapClient((RECT *)lpRect);
				OutTraceDW("DrawTextExW: fixed rect=(%d,%d)-(%d,%d)\n", lpRect->left, lpRect->top, lpRect->right, lpRect->bottom);
				ret=(*pDrawTextExW)(hdc, lpchText, nCount, lpRect, dwDTFormat, lpDTParams);
				dxw.UnmapClient((RECT *)lpRect);
				OutTraceDW("DrawTextExW: fixed output rect=(%d,%d)-(%d,%d)\n", lpRect->left, lpRect->top, lpRect->right, lpRect->bottom);
				break;
			default:
				ret=(*pDrawTextExW)(hdc, lpchText, nCount, lpRect, dwDTFormat, lpDTParams);
				break;
		}
	}
	else {
		ret=(*pDrawTextExW)(hdc, lpchText, nCount, lpRect, dwDTFormat, lpDTParams);
	}
	gFixed = FALSE;

	// if nCount is zero, DrawRect returns 0 as text heigth, but this is not an error! (ref. "Imperialism II")
	if(nCount && !ret) OutTraceE("DrawTextExW: ERROR ret=%x err=%d\n", ret, GetLastError()); 
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
	RECT rcUpdate;
	BOOL ret;

	OutTraceDW("RedrawWindow: hwnd=%x hrgn=%x flags=%x\n", hWnd, hrgnUpdate, flags);

	// v2.03.64 fix: if hrgnUpdate is set, lprcUpdate is ignored, so it can't be scaled
	// beware: they both could be null, and that means the whole window
	if (!hrgnUpdate && lprcUpdate) rcUpdate = *lprcUpdate;
	// avoid redrawing the whole desktop
	if(dxw.Windowize && dxw.IsRealDesktop(hWnd)) hWnd=dxw.GethWnd();
	if(dxw.IsFullScreen()){
		// v2.03.64 fix: if hrgnUpdate is set, lprcUpdate is ignored, so it can't be scaled
		if (!hrgnUpdate && lprcUpdate) rcUpdate = dxw.MapClientRect((LPRECT)lprcUpdate);
	}

	ret = (*pRedrawWindow)(hWnd, lprcUpdate ? &rcUpdate : NULL, hrgnUpdate, flags);
	if(ret) OutTraceE("RedrawWindow ERROR: err=%d\n", GetLastError());
	return ret;
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
	if(dxw.Windowize && dxw.IsFullScreen()) {
		OutTraceDW("GetActiveWindow: ret=%x->%x\n", ret, dxw.GethWnd());
		return dxw.GethWnd();
	}
	return ret;
}

HWND WINAPI extGetForegroundWindow(void)
{
	HWND ret;
	ret=(*pGetForegroundWindow)();
	if(dxw.Windowize && dxw.IsFullScreen()) {
		OutTraceDW("GetForegroundWindow: ret=%x->%x\n", ret, dxw.GethWnd());
		return dxw.GethWnd();
	}
	return ret;
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
	OutTraceDW("SystemParametersInfoA: Action=%x Param=%x WinIni=%x\n", uiAction, uiParam, fWinIni);
	switch(uiAction){
		case SPI_SETKEYBOARDDELAY:
		case SPI_SETKEYBOARDSPEED:
			OutTraceDW("SystemParametersInfoA: bypass action=%x\n", uiAction);
			return TRUE;
			break;
	}
	ret=(*pSystemParametersInfoA)(uiAction, uiParam, pvParam, fWinIni);
	if(uiAction==SPI_GETWORKAREA){
		LPRECT cli = (LPRECT)pvParam;
		*cli = dxw.GetScreenRect();
		OutTraceDW("SystemParametersInfoA: resized client workarea rect=(%d,%d)-(%d,%d)\n", cli->left, cli->top, cli->right, cli->bottom);
	}
	if(!ret) OutTraceE("SystemParametersInfoA ERROR: err=%d\n", GetLastError());
	return ret;
}

BOOL WINAPI extSystemParametersInfoW(UINT uiAction, UINT uiParam, PVOID pvParam, UINT fWinIni)
{
	BOOL ret;
	OutTraceDW("SystemParametersInfoW: Action=%x Param=%x WinIni=%x\n", uiAction, uiParam, fWinIni);
	switch(uiAction){
		case SPI_SETKEYBOARDDELAY:
		case SPI_SETKEYBOARDSPEED:
			OutTraceDW("SystemParametersInfoW: bypass action=%x\n", uiAction);
			return TRUE;
			break;
	}
	ret=(*pSystemParametersInfoW)(uiAction, uiParam, pvParam, fWinIni);
	if(uiAction==SPI_GETWORKAREA){
		LPRECT cli = (LPRECT)pvParam;
		*cli = dxw.GetScreenRect();
		OutTraceDW("SystemParametersInfoW: resized client workarea rect=(%d,%d)-(%d,%d)\n", cli->left, cli->top, cli->right, cli->bottom);
	}
	if(!ret) OutTraceE("SystemParametersInfoW ERROR: err=%d\n", GetLastError());
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

HWND WINAPI extWindowFromPoint(POINT Point)
{
	HWND ret;
	OutTraceDW("WindowFromPoint: point=(%d,%d)\n", Point.x, Point.y); 
	if(dxw.IsFullScreen()){
		dxw.MapWindow(&Point); // v2.03.69 fix
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
	if(dxw.IsDesktop(hWndParent) && dxw.IsFullScreen() && dxw.Windowize){
		dxw.MapClient(&Point);
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
	if(dxw.IsDesktop(hWndParent) && dxw.IsFullScreen() && dxw.Windowize){
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
	OutTraceDW("GetMonitorInfo: hMonitor=%x mi=MONITORINFO%s\n", hMonitor, lpmi->cbSize==sizeof(MONITORINFO)?"":"EX");
	res=(*pGetMonitorInfo)(hMonitor, lpmi);
	//v2.03.15 - must fix the coordinates also in case of error: that may depend on the windowed mode.
	if(dxw.Windowize){
		OutTraceDW("GetMonitorInfo: FIX Work=(%d,%d)-(%d,%d) Monitor=(%d,%d)-(%d,%d) -> (%d,%d)-(%d,%d)\n", 
			lpmi->rcWork.left, lpmi->rcWork.top, lpmi->rcWork.right, lpmi->rcWork.bottom,
			lpmi->rcMonitor.left, lpmi->rcMonitor.top, lpmi->rcMonitor.right, lpmi->rcMonitor.bottom,
			0, 0, dxw.GetScreenWidth(), dxw.GetScreenHeight());
		lpmi->rcWork = dxw.GetScreenRect();
		lpmi->rcMonitor = dxw.GetScreenRect();
		res=TRUE;
	}
	else
		OutTraceE("GetMonitorInfo: ERROR err=%d\n", GetLastError());

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

	if(dxw.IsFullScreen()){
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

#ifdef TRACEPALETTE
UINT WINAPI extGetDIBColorTable(HDC hdc, UINT uStartIndex, UINT cEntries, RGBQUAD *pColors)
{
	UINT ret;
	OutTraceDW("GetDIBColorTable: hdc=%x start=%d entries=%d\n", hdc, uStartIndex, cEntries);

	//if((OBJ_DC == (*pGetObjectType)(hdc)) && (dxw.dwFlags1 & EMULATESURFACE)){
	//	//extern PALETTEENTRY PalEntries[256];
	//	extern DWORD *PaletteEntries;
	//	if((uStartIndex+cEntries) > 256) cEntries = 256 - uStartIndex;
	//	for(UINT i=0; i<cEntries; i++) {
	//		PALETTEENTRY p;
	//		memcpy(&p, &PaletteEntries[i+uStartIndex], sizeof(DWORD));
	//		pColors[i].rgbBlue = p.peBlue;
	//		pColors[i].rgbGreen = p.peGreen;
	//		pColors[i].rgbRed = p.peRed;
	//		pColors[i].rgbReserved = p.peFlags;
	//	}
	//	ret=cEntries;
	//}
	//else
	//	ret = (*pGetDIBColorTable)(hdc, uStartIndex, cEntries, pColors);

	ret = (*pGetDIBColorTable)(hdc, uStartIndex, cEntries, pColors);
	OutTraceDW("GetDIBColorTable: ret=%x\n", ret);
	if(IsDebug) dxw.DumpPalette(cEntries, (PALETTEENTRY *)pColors);
	return ret;
}

UINT WINAPI extSetDIBColorTable(HDC hdc, UINT uStartIndex, UINT cEntries, const RGBQUAD *pColors)
{
	UINT ret;
	OutTraceDW("SetDIBColorTable: hdc=%x start=%d entries=%d\n", hdc, uStartIndex, cEntries);
	if(IsDebug) dxw.DumpPalette(cEntries, (PALETTEENTRY *)pColors);

	//if((OBJ_DC == GetObjectType(hdc)) && (dxw.dwFlags1 & EMULATESURFACE)){
	//	//extern PALETTEENTRY PalEntries[256];
	//	extern DWORD *PaletteEntries;
	//	if((uStartIndex+cEntries) > 256) cEntries = 256 - uStartIndex;
	//	for(UINT i=0; i<cEntries; i++) {
	//		PALETTEENTRY p;
	//		memcpy(&p, &PaletteEntries[i+uStartIndex], sizeof(DWORD));
	//		pColors[i].rgbBlue = p.peBlue;
	//		pColors[i].rgbGreen = p.peGreen;
	//		pColors[i].rgbRed = p.peRed;
	//		pColors[i].rgbReserved = p.peFlags;
	//	}
	//	ret=cEntries;
	//}
	//else
	//	ret = (*pSetDIBColorTable)(hdc, uStartIndex, cEntries, pColors);
	
	ret = (*pSetDIBColorTable)(hdc, uStartIndex, cEntries, pColors);
	OutTraceDW("SetDIBColorTable: ret=%x\n", ret);
	return ret;
}
#endif

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

BOOL WINAPI extBringWindowToTop(HWND hwnd)
{
	BOOL res;
	OutTraceDW("BringWindowToTop: hwnd=%x\n", hwnd);
	if(dxw.dwFlags5 & UNLOCKZORDER) return TRUE;
	res=(*pBringWindowToTop)(hwnd);
	return res;
}

BOOL WINAPI extSetForegroundWindow(HWND hwnd)
{
	BOOL res;
	OutTraceDW("SetForegroundWindow: hwnd=%x\n", hwnd);
	if(dxw.dwFlags5 & UNLOCKZORDER) return TRUE;
	res=(*pSetForegroundWindow)(hwnd);
	return res;
}

/*
HOOKPROC glpMouseHookProcessFunction;
LRESULT CALLBACK extMouseHookProc(int code, WPARAM wParam, LPARAM lParam)
{
	LRESULT ret;
	OutTrace("HookProc intercepted: code=%x wParam=%x lParam=%x\n", code, wParam, lParam);
    MOUSEHOOKSTRUCT * pMouseStruct = (MOUSEHOOKSTRUCT *)lParam;
    if (pMouseStruct != NULL){
		dxw.UnmapWindow(&(pMouseStruct->pt));
    }	
	ret= (*glpMouseHookProcessFunction)(code, wParam, lParam);
	return ret;
}
*/

HOOKPROC glpMessageHookProcessFunction;

LRESULT CALLBACK extMessageHookProc(int code, WPARAM wParam, LPARAM lParam)
{
	LRESULT ret;
	OutTrace("MessageHookProc: code=%x wParam=%x lParam=%x\n", code, wParam, lParam);
    MSG * pMessage = (MSG *)lParam;
	ret = NULL;
	if(pMessage){
		UINT message = pMessage->message;
		if ((message >= 0x600) ||											// custom messages
			((message >= WM_KEYFIRST) && (message <= WM_KEYLAST)) ||		// keyboard messages
			((message >= WM_MOUSEFIRST) && (message <= WM_MOUSELAST))		// mouse messages
			)			
			ret = (*glpMessageHookProcessFunction)(code, wParam, lParam);
	}
	return ret;
}

static HHOOK WINAPI extSetWindowsHookEx(SetWindowsHookEx_Type pSetWindowsHookEx, int idHook, HOOKPROC lpfn, HINSTANCE hMod, DWORD dwThreadId)
{
	HHOOK ret;

	OutTraceDW("SetWindowsHookEx: id=%x threadid=%x\n", idHook, dwThreadId);

	if(dxw.dwFlags5 & EASPORTSHACK){
		OutTraceDW("SetWindowsHookEx: EASPORTSHACK bypass active\n");
		if(idHook == WH_MOUSE) return NULL;
		if(idHook == WH_GETMESSAGE) {
			glpMessageHookProcessFunction = lpfn;
			lpfn=extMessageHookProc;
		}
	}
	// v2.03.39: "One Must Fall Battlegrounds" keyboard fix
	if((idHook == WH_KEYBOARD) && (dwThreadId == NULL)) {
		dwThreadId = GetCurrentThreadId();
		OutTraceDW("SetWindowsHookEx: fixing WH_KEYBOARD thread=0->%x\n", dwThreadId);
	}

	// v2.03.54: disable the disable Alt-Tab fix
	if((dxw.dwFlags7 & DISABLEDISABLEALTTAB) && (idHook == WH_KEYBOARD_LL)) {
		OutTraceDW("SetWindowsHookEx: DISABLEDISABLEALTTAB bypass active\n");
		return NULL;
	}

	ret=(*pSetWindowsHookEx)(idHook, lpfn, hMod, dwThreadId);

	return ret;
}

HHOOK WINAPI extSetWindowsHookExA(int idHook, HOOKPROC lpfn, HINSTANCE hMod, DWORD dwThreadId)
{ return extSetWindowsHookEx(pSetWindowsHookExA, idHook, lpfn, hMod, dwThreadId); }
HHOOK WINAPI extSetWindowsHookExW(int idHook, HOOKPROC lpfn, HINSTANCE hMod, DWORD dwThreadId)
{ return extSetWindowsHookEx(pSetWindowsHookExW, idHook, lpfn, hMod, dwThreadId); }

HRESULT WINAPI extMessageBoxTimeoutA(HWND hWnd, LPCSTR lpText, LPCSTR lpCaption, UINT uType, WORD wLanguageId, DWORD dwMilliseconds)
{
	HRESULT res;
	if(1) dwMilliseconds=1000;
	res=(*pMessageBoxTimeoutA)(hWnd, lpText, lpCaption, uType, wLanguageId, dwMilliseconds);
	return res;
}

HRESULT WINAPI extMessageBoxTimeoutW(HWND hWnd, LPCWSTR lpText, LPCWSTR lpCaption, UINT uType, WORD wLanguageId, DWORD dwMilliseconds)
{
	HRESULT res;
	if(1) dwMilliseconds=1000;
	res=(*pMessageBoxTimeoutW)(hWnd, lpText, lpCaption, uType, wLanguageId, dwMilliseconds);
	return res;
}

HDESK WINAPI extCreateDesktop( LPCTSTR lpszDesktop, LPCTSTR lpszDevice, DEVMODE *pDevmode, DWORD dwFlags, ACCESS_MASK dwDesiredAccess, LPSECURITY_ATTRIBUTES lpsa)
{
	//OutTrace("CreateDesktop: SUPPRESS Desktop=%s Device=%s flags=%x access=%x\n", lpszDesktop, lpszDevice, dwFlags, dwDesiredAccess);
	OutTraceDW("CreateDesktop: SUPPRESS flags=%x access=%x\n", dwFlags, dwDesiredAccess);
	return (HDESK)0xDEADBEEF; // fake handle
	//return (HDESK)NULL; // fake handle
}

BOOL WINAPI extSwitchDesktop(HDESK hDesktop)
{
	OutTraceDW("SwitchDesktop: SUPPRESS hDesktop=%x\n", hDesktop);
	return TRUE;
}

HDESK WINAPI extOpenDesktop(LPTSTR lpszDesktop, DWORD dwFlags, BOOL fInherit, ACCESS_MASK dwDesiredAccess)
{
	OutTraceDW("CreateDesktop: SUPPRESS flags=%x access=%x\n", dwFlags, dwDesiredAccess);
	return (HDESK)0xDEADBEEF; // fake handle
	//return (HDESK)NULL; // fake handle
}

BOOL WINAPI extCloseDesktop(HDESK hDesktop)
{
	OutTraceDW("CloseDesktop: SUPPRESS hDesktop=%x\n", hDesktop);
	return TRUE;
}

INT_PTR WINAPI extDialogBoxParamA(HINSTANCE hInstance, LPCTSTR lpTemplateName, HWND hWndParent, DLGPROC lpDialogFunc, LPARAM dwInitParam)
{
	BOOL ret, FullScreen;
	FullScreen = dxw.IsFullScreen();
	OutTraceDW("DialogBoxParamA: FullScreen=%x TemplateName=\"%s\" WndParent=%x\n", 
		FullScreen, sTemplateName(lpTemplateName), hWndParent);
	dxw.SetFullScreen(FALSE);
	ret = (*pDialogBoxParamA)(hInstance, lpTemplateName, hWndParent, lpDialogFunc, dwInitParam);
	dxw.SetFullScreen(FullScreen);
	OutTraceDW("DialogBoxParamA: ret=%x\n", ret);
	return ret;
}

BOOL WINAPI extIsZoomed(HWND hWnd)
{
	BOOL ret;
	ret = (*pIsZoomed)(hWnd);
	OutTraceDW("IsZoomed: hwnd=%x ret=%x\n", hWnd, ret);
	//if(dxw.IsFullScreen()) ret = FALSE;
	return ret;
}

BOOL WINAPI extIsIconic(HWND hWnd)
{
	BOOL ret;
	ret = (*pIsIconic)(hWnd);
	OutTraceDW("IsIconic: hwnd=%x ret=%x\n", hWnd, ret);
	//return FALSE;
	return ret;
}

BOOL extScrollWindow(HWND hWnd, int XAmount, int YAmount, const RECT *lpRect, const RECT *lpClipRect)
{
	RECT Rect, ClipRect;
	BOOL res;

	OutTraceDW("ScrollWindow: hwnd=%x amount=(%d,%d) rect=(%d,%d)-(%d,%d) clip=(%d,%d)-(%d,%d)\n",
		hWnd, XAmount, YAmount, 
		lpRect->left, lpRect->top, lpRect->right, lpRect->bottom,
		lpClipRect->left, lpClipRect->top, lpClipRect->right, lpClipRect->bottom);

	Rect = *lpRect;
	ClipRect = *lpClipRect;
	if(dxw.Windowize && dxw.IsFullScreen()){
		dxw.MapClient(&XAmount, &YAmount);
		dxw.MapClient(&Rect);
		dxw.MapClient(&ClipRect);
	}

	res=(*pScrollWindow)(hWnd, XAmount, YAmount, (const RECT *)&Rect, (const RECT *)&ClipRect);
	if(!res) OutTraceE("ScrollWindow ERROR: err=%d\n", GetLastError());
	return res;
}

#if 0
// avoid invalidating whole desktop!!!
BOOL InvalidateRgn(
  _In_ HWND hWnd,
  _In_ HRGN hRgn,
  _In_ BOOL bErase
);

#endif

HWND WINAPI extGetParent(HWND hWnd)
{
	// Beware: can cause recursion on HOT PATCH mode
	HWND ret;

	ret = (*pGetParent)(hWnd);
	OutTraceB("GetParent: hwnd=%x ret=%x\n", hWnd, ret);

	if(dxw.IsFullScreen()){
		if(ret == dxw.GethWnd()) {
			OutTraceB("GetParent: setting desktop reached\n");
			ret = 0; // simulate reaching the desktop
		}
	}

	return ret;
}

BOOL WINAPI extInvalidateRgn(HWND hWnd, HRGN hRgn, BOOL bErase)
{
	OutTraceDW("InvalidateRgn: hwnd=%x hrgn=%x erase=%x\n", hWnd, hRgn, bErase);

	if(dxw.IsFullScreen()){
		if (dxw.IsRealDesktop(hWnd) && bErase) return true;
	}

	return (*pInvalidateRgn)(hWnd, hRgn, bErase);
}

BOOL WINAPI extDrawIcon(HDC hdc, int X, int Y, HICON hIcon)
{
	BOOL ret;
	OutTraceDW("DrawIcon: hdcdest=%x pos=(%d,%d) hicon=%x\n", hdc, X, Y, hIcon);
	if(dxw.IsToRemap(hdc)){
		switch(dxw.GDIEmulationMode){
			case GDIMODE_STRETCHED: 
				dxw.MapClient(&X, &Y);
				OutTraceDW("OffsetRgn: fixed STRETCHED pos=(%d,%d)\n", X, Y);
				break;
			case GDIMODE_SHAREDDC: 
				sdc.GetPrimaryDC(hdc);
				ret=(*pDrawIcon)(sdc.GetHdc(),  X, Y, hIcon);
				sdc.PutPrimaryDC(hdc, TRUE);
				return ret;
				break;
		default:
			break;
		}
	}

	ret = (*pDrawIcon)(hdc, X, Y, hIcon);
	if(!ret) OutTraceE("DrawIcon ERROR: err=%d\n", GetLastError());
	return ret;
}

// not working in HOT PATCH mode
BOOL WINAPI extDrawIconEx( HDC hdc, int xLeft, int yTop, HICON hIcon, int cxWidth, int cyWidth, UINT istepIfAniCur, HBRUSH hbrFlickerFreeDraw, UINT diFlags)
{
	BOOL ret;
	OutTraceDW("DrawIconEx: hdc=%x pos=(%d,%d) hicon=%x size=(%d,%d) istep=%x flags=%x\n",
		hdc, xLeft, yTop, hIcon, cxWidth, cyWidth, istepIfAniCur, diFlags);
	if(dxw.IsToRemap(hdc)){
		switch(dxw.GDIEmulationMode){
			case GDIMODE_STRETCHED: 
				dxw.MapClient(&xLeft, &yTop, &cxWidth, &cyWidth);
				OutTraceDW("DrawIconEx: fixed STRETCHED pos=(%d,%d) size=(%d,%d)\n", xLeft, yTop, cxWidth, cyWidth);
				break;
			case GDIMODE_SHAREDDC: 
				sdc.GetPrimaryDC(hdc);
				ret=(*pDrawIconEx)(sdc.GetHdc(), xLeft, yTop, hIcon, cxWidth, cyWidth, istepIfAniCur, hbrFlickerFreeDraw, diFlags);
				sdc.PutPrimaryDC(hdc, TRUE, xLeft, yTop, cxWidth, cyWidth);
				return ret;
				break;
		default:
			break;
		}
	}
	ret = (*pDrawIconEx)(hdc, xLeft, yTop, hIcon, cxWidth, cyWidth, istepIfAniCur, hbrFlickerFreeDraw, diFlags);
	if(!ret) OutTraceE("DrawIconEx ERROR: err=%d\n", GetLastError());
	return ret;
}

BOOL WINAPI extDrawCaption(HWND hwnd, HDC hdc, LPCRECT lprc, UINT uFlags)
{
	BOOL ret;
	OutTraceDW("DrawCaption: hwnd=%x hdc=%x rect=(%d,%d)-(%d,%d) flags=%x\n", hwnd, hdc, lprc->left, lprc->top, lprc->right, lprc->bottom, uFlags);
	if(dxw.IsToRemap(hdc)){
		switch(dxw.GDIEmulationMode){
			case GDIMODE_STRETCHED: 
				dxw.MapClient((LPRECT)lprc);
				OutTraceDW("DrawIconEx: fixed STRETCHED rect=(%d,%d)-(%d,%d)\n", lprc->left, lprc->top, lprc->right, lprc->bottom);
				break;
			case GDIMODE_SHAREDDC: 
				sdc.GetPrimaryDC(hdc);
				ret=(*pDrawCaption)(hwnd, sdc.GetHdc(), lprc, uFlags);
				sdc.PutPrimaryDC(hdc, TRUE, lprc->left, lprc->top, lprc->right, lprc->bottom);
				return ret;
				break;
		default:
			break;
		}
	}
	ret = (*pDrawCaption)(hwnd, hdc, lprc, uFlags);
	if(!ret) OutTraceE("DrawCaption ERROR: err=%d\n", GetLastError());
	return ret;
}

BOOL WINAPI extPaintDesktop(HDC hdc)
{
	BOOL ret;
	OutTraceDW("PaintDesktop: hdc=%x\n", hdc);
	if(dxw.IsToRemap(hdc)){
		switch(dxw.GDIEmulationMode){
			case GDIMODE_SHAREDDC: 
				sdc.GetPrimaryDC(hdc);
				ret=(*pPaintDesktop)(sdc.GetHdc());
				sdc.PutPrimaryDC(hdc, TRUE);
				return ret;
				break;
		default:
			break;
		}
	}
	ret = (*pPaintDesktop)(hdc);
	if(!ret) OutTraceE("PaintDesktop ERROR: err=%d\n", GetLastError());
	return ret;
}
VOID WINAPI extmouse_event(DWORD dwFlags, DWORD dx, DWORD dy, DWORD dwData, ULONG_PTR dwExtraInfo)
{
	OutTrace("mouse_event: flags=%x xy=(%d,%d) data=%x, extrainfo=%lx\n", dwFlags, dx, dy, dwData, dwExtraInfo);
	return (*pmouse_event)(dwFlags, dx, dy, dwData, dwExtraInfo);
}

BOOL WINAPI extShowScrollBar(HWND hWnd, int wBar, BOOL bShow)
{
	BOOL ret;
	OutTraceDW("ShowScrollBar: hwnd=%x wBar=%x show=%x\n", hWnd, wBar, bShow);
	if(dxw.Windowize && dxw.IsRealDesktop(hWnd)) hWnd=dxw.GethWnd();
	ret=(*pShowScrollBar)(hWnd, wBar, bShow);
	if(!ret) OutTraceE("ShowScrollBar ERROR: err=%d\n", GetLastError());
	return ret;
}

BOOL WINAPI extDrawMenuBar(HWND hWnd)
{
	BOOL ret;
	OutTraceDW("DrawMenuBar: hwnd=%x\n", hWnd);
	if(dxw.Windowize && dxw.IsRealDesktop(hWnd)) hWnd=dxw.GethWnd();
	ret=(*pDrawMenuBar)(hWnd);
	if(!ret) OutTraceE("DrawMenuBar ERROR: err=%d\n", GetLastError());
	return ret;
}

#if 0
BOOL WINAPI extTranslateMessage(MSG *pMsg)
{
	BOOL ret;
	OutTraceDW("TranslateMessage: type=%x pos=(%d,%d)\n", pMsg->message, pMsg->pt.x, pMsg->pt.y);
	if(dxw.Windowize){

		pMsg->pt=dxw.ScreenToClient(pMsg->pt);
		pMsg->pt=dxw.FixCursorPos(pMsg->pt);
		
		pMsg->pt.x *= 4;
		pMsg->pt.y *= 4;
		//if((pMsg->message <= WM_MOUSELAST) && (pMsg->message >= WM_MOUSEFIRST)) 
		//	pMsg->lParam = MAKELPARAM(pMsg->pt.x, pMsg->pt.y); 
		OutTraceDW("TranslateMessage: new pos=(%d,%d)\n", pMsg->pt.x, pMsg->pt.y);
	}
	ret=(*pTranslateMessage)(pMsg);
	return ret;
}
#endif