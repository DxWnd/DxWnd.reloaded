#include "dxwnd.h"
#include "dxwcore.hpp"
#include "syslibs.h"
#include "dxhook.h"

FARPROC Remap_user32_ProcAddress(LPCSTR proc, HMODULE hModule)
{
	if (!strcmp(proc,"ChangeDisplaySettingsA")){
		pChangeDisplaySettings=(ChangeDisplaySettings_Type)(*pGetProcAddress)(hModule, proc);
		OutTraceD("GetProcAddress: hooking proc=%s at addr=%x\n", ProcToString(proc), pChangeDisplaySettings);
		return (FARPROC)extChangeDisplaySettings;
	}
	if (!strcmp(proc,"ChangeDisplaySettingsExA")){
		pChangeDisplaySettingsEx=(ChangeDisplaySettingsEx_Type)(*pGetProcAddress)(hModule, proc);
		OutTraceD("GetProcAddress: hooking proc=%s at addr=%x\n", ProcToString(proc), pChangeDisplaySettingsEx);
		return (FARPROC)extChangeDisplaySettingsEx;
	}
	if (!strcmp(proc,"BeginPaint")){
		pBeginPaint=(BeginPaint_Type)(*pGetProcAddress)(hModule, proc);
		OutTraceD("GetProcAddress: hooking proc=%s at addr=%x\n", ProcToString(proc), pBeginPaint);
		return (FARPROC)extBeginPaint;
	}
	if (!strcmp(proc,"EndPaint")){
		pEndPaint=(EndPaint_Type)(*pGetProcAddress)(hModule, proc);
		OutTraceD("GetProcAddress: hooking proc=%s at addr=%x\n", ProcToString(proc), pEndPaint);
		return (FARPROC)extEndPaint;
	}
	if (!strcmp(proc,"ShowCursor")){
		pShowCursor=(ShowCursor_Type)(*pGetProcAddress)(hModule, proc);
		OutTraceD("GetProcAddress: hooking proc=%s at addr=%x\n", ProcToString(proc), pShowCursor);
		return (FARPROC)extShowCursor;
	}
	/*
	if(dxw.dwFlags1 & MAPGDITOPRIMARY){
		tmp = HookAPI(module, "user32.dll", GetDC, "GetDC", extDDGetDC);
		if(tmp) pGDIGetDC = (GDIGetDC_Type)tmp;
		tmp = HookAPI(module, "user32.dll", GetWindowDC, "GetWindowDC", extDDGetWindowDC);
		if(tmp) pGDIGetWindowDC = (GDIGetDC_Type)tmp;
		tmp = HookAPI(module, "user32.dll", ReleaseDC, "ReleaseDC", extDDReleaseDC);
		if(tmp) pGDIReleaseDC = (GDIReleaseDC_Type)tmp;
		tmp = HookAPI(module, "user32.dll", InvalidateRect, "InvalidateRect", extDDInvalidateRect);
		if(tmp) pInvalidateRect = (InvalidateRect_Type)tmp;
	}
	else{
		tmp = HookAPI(module, "user32.dll", GetDC, "GetDC", extGDIGetDC);
		if(tmp) pGDIGetDC = (GDIGetDC_Type)tmp;
		tmp = HookAPI(module, "user32.dll", GetWindowDC, "GetWindowDC", extGDIGetWindowDC);
		if(tmp) pGDIGetWindowDC = (GDIGetDC_Type)tmp;
		tmp = HookAPI(module, "user32.dll", ReleaseDC, "ReleaseDC", extGDIReleaseDC);
		if(tmp) pGDIReleaseDC = (GDIReleaseDC_Type)tmp;
		tmp = HookAPI(module, "user32.dll", InvalidateRect, "InvalidateRect", extInvalidateRect);
		if(tmp) pInvalidateRect = (InvalidateRect_Type)tmp;
	}

	if (dxw.dwFlags1 & CLIENTREMAPPING){
		tmp = HookAPI(module, "user32.dll", ScreenToClient, "ScreenToClient", extScreenToClient);
		if(tmp) pScreenToClient = (ScreenToClient_Type)tmp;
		tmp = HookAPI(module, "user32.dll", ClientToScreen, "ClientToScreen", extClientToScreen);
		if(tmp) pClientToScreen = (ClientToScreen_Type)tmp;
		tmp = HookAPI(module, "user32.dll", GetClientRect, "GetClientRect", extGetClientRect);
		if(tmp) pGetClientRect = (GetClientRect_Type)tmp;
		tmp = HookAPI(module, "user32.dll", GetWindowRect, "GetWindowRect", extGetWindowRect);
		if(tmp) pGetWindowRect = (GetWindowRect_Type)tmp;
		tmp = HookAPI(module, "user32.dll", MapWindowPoints, "MapWindowPoints", extMapWindowPoints);
		if(tmp) pMapWindowPoints = (MapWindowPoints_Type)tmp;
	}

	// get / change display settings
	tmp = HookAPI(module, "user32.dll", ChangeDisplaySettingsA, "ChangeDisplaySettingsA", extChangeDisplaySettings);
	if(tmp) pChangeDisplaySettings = (ChangeDisplaySettings_Type)tmp;
	tmp = HookAPI(module, "user32.dll", ChangeDisplaySettingsExA, "ChangeDisplaySettingsExA", extChangeDisplaySettingsEx);
	if(tmp) pChangeDisplaySettingsEx = (ChangeDisplaySettingsEx_Type)tmp;
	tmp = HookAPI(module, "user32.dll", EnumDisplaySettingsA, "EnumDisplaySettingsA", extEnumDisplaySettings);
	if(tmp) pEnumDisplaySettings = (EnumDisplaySettings_Type)tmp;

	// handle cursor clipping
	tmp = HookAPI(module, "user32.dll", GetClipCursor, "GetClipCursor", extGetClipCursor);
	if(tmp) pGetClipCursor = (GetClipCursor_Type)tmp;

	tmp = HookAPI(module, "user32.dll", ClipCursor, "ClipCursor", extClipCursor);
	if(tmp) pClipCursor = (ClipCursor_Type)tmp;
	tmp = HookAPI(module, "user32.dll", FillRect, "FillRect", extFillRect);
	if(tmp) pFillRect = (FillRect_Type)tmp;
	if (dxw.dwFlags1 & MESSAGEPROC) {
		tmp = HookAPI(module, "user32.dll", PeekMessageA, "PeekMessageA", extPeekMessage); // added for GPL 
		if(tmp) pPeekMessage = (PeekMessage_Type)tmp;
		tmp = HookAPI(module, "user32.dll", GetMessageA, "GetMessageA", extGetMessage); // added for GPL 
		if(tmp) pGetMessage = (GetMessage_Type)tmp;
	}
	tmp = HookAPI(module, "user32.dll", DefWindowProcA, "DefWindowProcA", extDefWindowProc); // added for WarWind background erase 
	if(tmp) pDefWindowProc = (DefWindowProc_Type)tmp;
	tmp = HookAPI(module, "user32.dll", CreateWindowExA, "CreateWindowExA", extCreateWindowExA);
	if(tmp) pCreateWindowExA = (CreateWindowExA_Type)tmp;
	tmp = HookAPI(module, "user32.dll", RegisterClassExA, "RegisterClassExA", extRegisterClassExA);
	if(tmp) pRegisterClassExA = (RegisterClassExA_Type)tmp;
	if (dxw.dwFlags1 & (PREVENTMAXIMIZE|FIXWINFRAME|LOCKWINPOS|LOCKWINSTYLE)){
		tmp = HookAPI(module, "user32.dll", ShowWindow, "ShowWindow", extShowWindow);
		if(tmp) pShowWindow = (ShowWindow_Type)tmp;
		tmp = HookAPI(module, "user32.dll", SetWindowLongA, "SetWindowLongA", extSetWindowLong);
		if(tmp) pSetWindowLong = (SetWindowLong_Type)tmp;
		tmp = HookAPI(module, "user32.dll", GetWindowLongA, "GetWindowLongA", extGetWindowLong);
		if(tmp) pGetWindowLong = (GetWindowLong_Type)tmp;
		tmp = HookAPI(module, "user32.dll", SetWindowPos, "SetWindowPos", extSetWindowPos);
		if(tmp) pSetWindowPos = (SetWindowPos_Type)tmp;
		tmp = HookAPI(module, "user32.dll", DeferWindowPos, "DeferWindowPos", extDeferWindowPos);
		if(tmp) pGDIDeferWindowPos = (DeferWindowPos_Type)tmp;
		tmp = HookAPI(module, "user32.dll", CallWindowProcA, "CallWindowProcA", extCallWindowProc);
		if(tmp) pCallWindowProc = (CallWindowProc_Type)tmp;
	}
	tmp = HookAPI(module, "user32.dll", GetSystemMetrics, "GetSystemMetrics", extGetSystemMetrics);
	if(tmp) pGetSystemMetrics = (GetSystemMetrics_Type)tmp;

	tmp = HookAPI(module, "user32.dll", GetDesktopWindow, "GetDesktopWindow", extGetDesktopWindow);
	if(tmp) pGetDesktopWindow = (GetDesktopWindow_Type)tmp;

	if(dxw.dwFlags1 & MODIFYMOUSE){
		tmp = HookAPI(module, "user32.dll", GetCursorPos, "GetCursorPos", extGetCursorPos);
		if(tmp) pGetCursorPos = (GetCursorPos_Type)tmp;
		//tmp = HookAPI(module, "user32.dll", GetPhysicalCursorPos, "", extGetCursorPos);
		tmp = HookAPI(module, "user32.dll", SetCursor, "SetCursor", extSetCursor);
		if(tmp) pSetCursor = (SetCursor_Type)tmp;
		//tmp = HookAPI(module, "user32.dll", SetPhysicalCursorPos, "", extSetCursorPos);
		tmp = HookAPI(module, "user32.dll", SendMessageA, "SendMessageA", extSendMessage);
		if(tmp) pSendMessage = (SendMessage_Type)tmp;
	}

	if((dxw.dwFlags1 & (MODIFYMOUSE|SLOWDOWN|KEEPCURSORWITHIN)) || (dxw.dwFlags2 & KEEPCURSORFIXED)){ 
		tmp = HookAPI(module, "user32.dll", SetCursorPos, "SetCursorPos", extSetCursorPos);
		if(tmp) pSetCursorPos = (SetCursorPos_Type)tmp;
	}

	tmp = HookAPI(module, "user32.dll", CreateDialogIndirectParamA, "CreateDialogIndirectParamA", extCreateDialogIndirectParam);
	if(tmp) pCreateDialogIndirectParam = (CreateDialogIndirectParam_Type)tmp;
	tmp = HookAPI(module, "user32.dll", CreateDialogParamA, "CreateDialogParamA", extCreateDialogParam);
	if(tmp) pCreateDialogParam = (CreateDialogParam_Type)tmp;
	tmp = HookAPI(module, "user32.dll", MoveWindow, "MoveWindow", extMoveWindow);
	if(tmp) pMoveWindow = (MoveWindow_Type)tmp;
	*/
	return NULL;
}
