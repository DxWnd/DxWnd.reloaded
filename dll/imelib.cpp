#define _WIN32_WINNT 0x0600
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <malloc.h>
#include "dxwnd.h"
#include "dxwcore.hpp"
#include "dxhook.h"
#include "syslibs.h"
#include "dxhelper.h"

static HookEntry_Type Hooks[]={
	{"ImmNotifyIME", NULL, (FARPROC *)&pImmNotifyIME, (FARPROC)extImmNotifyIME},
	{"ImmCreateContext", NULL, (FARPROC *)&pImmCreateContext, (FARPROC)extImmCreateContext},
	{"ImmDestroyContext", NULL, (FARPROC *)&pImmDestroyContext, (FARPROC)extImmDestroyContext},
	{"ImmSetOpenStatus", NULL, (FARPROC *)&pImmSetOpenStatus, (FARPROC)extImmSetOpenStatus},
	{"ImmAssociateContext", NULL, (FARPROC *)&pImmAssociateContext, (FARPROC)extImmAssociateContext},
	{"ImmSetCompositionWindow", NULL, (FARPROC *)&pImmSetCompositionWindow, (FARPROC)extImmSetCompositionWindow},
	{"ImmSetCompositionStringA", NULL, (FARPROC *)&pImmSetCompositionString, (FARPROC)extImmSetCompositionString},
	{"ImmGetOpenStatus", NULL, (FARPROC *)&pImmGetOpenStatus, (FARPROC)extImmGetOpenStatus},
	{0, NULL, 0, 0} // terminator
};

void HookImeLib(HMODULE module)
{
	HookLibrary(module, Hooks, "IMM32.dll");
}

FARPROC Remap_ImeLib_ProcAddress(LPCSTR proc, HMODULE hModule)
{
	FARPROC addr;
	if (addr=RemapLibrary(proc, hModule, Hooks)) return addr;
	return NULL;
}

BOOL WINAPI extImmNotifyIME(HIMC hIMC, DWORD dwAction, DWORD dwIndex, DWORD dwValue)
{
	OutTraceD("ImmNotifyIME: hIMC=%x dwAction=%x dwIndex=%x dwValue=%x\n", hIMC, dwAction, dwIndex, dwValue);
	return(*pImmNotifyIME)(hIMC, dwAction, dwIndex, dwValue);
}

HIMC WINAPI extImmCreateContext(void)
{
	OutTraceD("ImmCreateContext\n");
	return(*pImmCreateContext)();
}

BOOL WINAPI extImmDestroyContext(HIMC hIMC)
{
	OutTraceD("ImmDestroyContext: hIMC=%x\n", hIMC);
	return(*pImmDestroyContext)(hIMC);
}

BOOL WINAPI extImmSetOpenStatus(HIMC hIMC, BOOL fOpen)
{
	OutTraceD("ImmSetOpenStatus: hIMC=%x fOpen=%x\n", hIMC, fOpen);
	return(*pImmSetOpenStatus)(hIMC, fOpen);
}

HIMC WINAPI extImmAssociateContext(HWND hwnd, HIMC hIMC)
{
	OutTraceD("ImmAssociateContext: hwnd=%x hIMC=%x\n", hwnd, hIMC);
	//return NULL;
	return(*pImmAssociateContext)(hwnd, hIMC);
}

BOOL WINAPI extImmSetCompositionWindow(HIMC hIMC, LPCOMPOSITIONFORM lpCompForm)
{
	OutTraceD("ImmSetCompositionWindow: hIMC=%x lpCompForm=%x\n", hIMC, lpCompForm);
	return(*pImmSetCompositionWindow)(hIMC, lpCompForm);
}

BOOL WINAPI extImmSetCompositionString(HIMC hIMC, DWORD dwIndex, LPVOID lpComp, DWORD dwCompLen, LPVOID lpRead, DWORD dwReadLen)
{
	OutTraceD("ImmSetCompositionString: hIMC=%x dwIndex=%x lpComp=%x dwCompLen=%x dwRead=%x dwReadLen=%x\n", hIMC, dwIndex, lpComp, dwCompLen, lpRead, dwReadLen);
	return (*pImmSetCompositionString)(hIMC, dwIndex, lpComp, dwCompLen, lpRead, dwReadLen);
}

BOOL WINAPI extImmGetOpenStatus(HIMC hIMC)
{
	OutTraceD("ImmGetOpenStatus: hIMC=%x\n", hIMC);
	return(*pImmGetOpenStatus)(hIMC);
}
