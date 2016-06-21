#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include "dxwnd.h"
#include "dxwcore.hpp"
#include "syslibs.h"
#include "dxhook.h"
#include "dxhelper.h"

typedef BOOL (WINAPI *InitializeFlatSB_Type)(HWND);
InitializeFlatSB_Type pInitializeFlatSB = NULL;
BOOL WINAPI extInitializeFlatSB(HWND);
typedef BOOL (WINAPI *UninitializeFlatSB_Type)(HWND);
InitializeFlatSB_Type pUninitializeFlatSB = NULL;
BOOL WINAPI extUninitializeFlatSB(HWND);

static HookEntryEx_Type Hooks[]={
	{HOOK_IAT_CANDIDATE, 0, "InitializeFlatSB", NULL, (FARPROC *)&pInitializeFlatSB, (FARPROC)extInitializeFlatSB},
	{HOOK_IAT_CANDIDATE, 0, "UninitializeFlatSB", NULL, (FARPROC *)&pUninitializeFlatSB, (FARPROC)extUninitializeFlatSB},
	{HOOK_IAT_CANDIDATE, 0, 0, NULL, 0, 0} // terminator
};

void HookComCtl32(HMODULE module)
{
	HookLibraryEx(module, Hooks, "comtl32.dll");
}

FARPROC Remap_ComCtl32_ProcAddress(LPCSTR proc, HMODULE hModule)
{
	FARPROC addr;
	if (addr=RemapLibraryEx(proc, hModule, Hooks)) return addr;
	return NULL;
}

BOOL WINAPI extInitializeFlatSB(HWND hwnd)
{
	BOOL ret;
	OutTraceDW("InitializeFlatSB: hwnd=%x\n", hwnd);
	if(dxw.IsFullScreen() && dxw.IsRealDesktop(hwnd)) {
		OutTraceDW("InitializeFlatSB: hwnd=%x->%x\n", hwnd, dxw.GethWnd());
		hwnd = dxw.GethWnd();
	}
	ret = (*pInitializeFlatSB)(hwnd);
	if(!ret)OutTraceDW("InitializeFlatSB: ret=%x\n", ret);
	return ret;
}

BOOL WINAPI extUninitializeFlatSB(HWND hwnd)
{
	BOOL ret;
	OutTraceDW("InitializeFlatSB: hwnd=%x\n", hwnd);
	if(dxw.IsFullScreen() && dxw.IsRealDesktop(hwnd)) {
		OutTraceDW("UninitializeFlatSB: hwnd=%x->%x\n", hwnd, dxw.GethWnd());
		hwnd = dxw.GethWnd();
	}
	ret = (*pUninitializeFlatSB)(hwnd);
	if(!ret)OutTraceDW("UninitializeFlatSB: ret=%x\n", ret);
	return ret;
}
