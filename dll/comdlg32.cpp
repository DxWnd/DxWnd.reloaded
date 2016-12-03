#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include "dxwnd.h"
#include "dxwcore.hpp"
#include "syslibs.h"
#include "dxhook.h"
#include "dxhelper.h"

typedef BOOL (WINAPI *GetFileNameA_Type)(LPOPENFILENAMEA);
typedef BOOL (WINAPI *GetFileNameW_Type)(LPOPENFILENAMEW);
GetFileNameA_Type pGetOpenFileNameA = NULL;
GetFileNameA_Type pGetSaveFileNameA = NULL;
GetFileNameW_Type pGetOpenFileNameW = NULL;
GetFileNameW_Type pGetSaveFileNameW = NULL;
BOOL WINAPI extGetSaveFileNameA(LPOPENFILENAMEA);
BOOL WINAPI extGetOpenFileNameA(LPOPENFILENAMEA);
BOOL WINAPI extGetSaveFileNameW(LPOPENFILENAMEW);
BOOL WINAPI extGetOpenFileNameW(LPOPENFILENAMEW);

static HookEntryEx_Type Hooks[]={
	{HOOK_IAT_CANDIDATE, 0, "GetSaveFileNameA", NULL, (FARPROC *)&pGetSaveFileNameA, (FARPROC)extGetSaveFileNameA},
	{HOOK_IAT_CANDIDATE, 0, "GetOpenFileNameA", NULL, (FARPROC *)&pGetOpenFileNameA, (FARPROC)extGetOpenFileNameA},
	{HOOK_IAT_CANDIDATE, 0, "GetSaveFileNameW", NULL, (FARPROC *)&pGetSaveFileNameW, (FARPROC)extGetSaveFileNameW},
	{HOOK_IAT_CANDIDATE, 0, "GetOpenFileNameW", NULL, (FARPROC *)&pGetOpenFileNameW, (FARPROC)extGetOpenFileNameW},
	{HOOK_IAT_CANDIDATE, 0, 0, NULL, 0, 0} // terminator
};

void HookComDlg32(HMODULE module)
{
	HookLibraryEx(module, Hooks, "comdlg32.dll");
}

FARPROC Remap_ComDlg32_ProcAddress(LPCSTR proc, HMODULE hModule)
{
	FARPROC addr;
	if (addr=RemapLibraryEx(proc, hModule, Hooks)) return addr;
	return NULL;
}

BOOL WINAPI extGetSaveFileNameA(LPOPENFILENAMEA lpofn)
{
	BOOL ret, FullScreen;
	FullScreen = dxw.IsFullScreen();
	OutTraceDW("GetSaveFileNameA: FullScreen=%x\n", FullScreen);
	dxw.SetFullScreen(FALSE);
	ret = (*pGetSaveFileNameA)(lpofn);
	dxw.SetFullScreen(FullScreen);
	OutTraceDW("GetSaveFileNameA: ret=%x\n", ret);
	return ret;
}

BOOL WINAPI extGetOpenFileNameA(LPOPENFILENAMEA lpofn)
{
	BOOL ret, FullScreen;
	FullScreen = dxw.IsFullScreen();
	OutTraceDW("GetOpenFileNameA: FullScreen=%x\n", FullScreen);
	dxw.SetFullScreen(FALSE);
	ret = (*pGetOpenFileNameA)(lpofn);
	dxw.SetFullScreen(FullScreen);
	OutTraceDW("GetOpenFileNameA: ret=%x\n", ret);
	return ret;
}

BOOL WINAPI extGetSaveFileNameW(LPOPENFILENAMEW lpofn)
{
	BOOL ret, FullScreen;
	FullScreen = dxw.IsFullScreen();
	OutTraceDW("GetSaveFileNameW: FullScreen=%x\n", FullScreen);
	dxw.SetFullScreen(FALSE);
	ret = (*pGetSaveFileNameW)(lpofn);
	dxw.SetFullScreen(FullScreen);
	OutTraceDW("GetSaveFileNameW: ret=%x\n", ret);
	return ret;
}

BOOL WINAPI extGetOpenFileNameW(LPOPENFILENAMEW lpofn)
{
	BOOL ret, FullScreen;
	FullScreen = dxw.IsFullScreen();
	OutTraceDW("GetOpenFileNameW: FullScreen=%x\n", FullScreen);
	dxw.SetFullScreen(FALSE);
	ret = (*pGetOpenFileNameW)(lpofn);
	dxw.SetFullScreen(FullScreen);
	OutTraceDW("GetOpenFileNameW: ret=%x\n", ret);
	return ret;
}
