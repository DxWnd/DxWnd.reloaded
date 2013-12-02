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

void HookImeLib(char *module)
{
	void *tmp;

	tmp = HookAPI(module, "IMM32.dll", NULL, "ImmNotifyIME", extImmNotifyIME);
	if(tmp) pImmNotifyIME = (ImmNotifyIME_Type)tmp;
	tmp = HookAPI(module, "IMM32.dll", NULL, "ImmCreateContext", extImmCreateContext);
	if(tmp) pImmCreateContext = (ImmCreateContext_Type)tmp;
	tmp = HookAPI(module, "IMM32.dll", NULL, "ImmDestroyContext", extImmDestroyContext);
	if(tmp) pImmDestroyContext = (ImmDestroyContext_Type)tmp;
	tmp = HookAPI(module, "IMM32.dll", NULL, "ImmSetOpenStatus", extImmSetOpenStatus);
	if(tmp) pImmSetOpenStatus = (ImmSetOpenStatus_Type)tmp;
	tmp = HookAPI(module, "IMM32.dll", NULL, "ImmAssociateContext", extImmAssociateContext);
	if(tmp) pImmAssociateContext = (ImmAssociateContext_Type)tmp;
	tmp = HookAPI(module, "IMM32.dll", NULL, "ImmSetCompositionWindow", extImmSetCompositionWindow);
	if(tmp) pImmSetCompositionWindow = (ImmSetCompositionWindow_Type)tmp;
	tmp = HookAPI(module, "IMM32.dll", NULL, "ImmSetCompositionStringA", extImmSetCompositionString);
	if(tmp) pImmSetCompositionString = (ImmSetCompositionString_Type)tmp;
	tmp = HookAPI(module, "IMM32.dll", NULL, "ImmGetOpenStatus", extImmGetOpenStatus);
	if(tmp) pImmGetOpenStatus = (ImmGetOpenStatus_Type)tmp;
}
