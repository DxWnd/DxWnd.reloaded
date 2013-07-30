#include <dxdiag.h>
#include "dxwnd.h"
#include "dxwcore.hpp"
#include "syslibs.h"
#include "dxhook.h"
#include "dxhelper.h"

typedef LONG	(WINAPI *WinVerifyTrust_Type)(HWND, GUID *, LPVOID);
WinVerifyTrust_Type pWinVerifyTrust;
extern LONG WINAPI extWinVerifyTrust(HWND, GUID *, LPVOID);

static HookEntry_Type Hooks[]={
	{"WinVerifyTrust", NULL, (FARPROC *)&pWinVerifyTrust, (FARPROC)extWinVerifyTrust},
	{0, NULL, 0, 0} // terminator
};

void HookTrust(HMODULE module)
{
	HookLibrary(module, Hooks, "wintrust.dll");
}

FARPROC Remap_trust_ProcAddress(LPCSTR proc, HMODULE hModule)
{
	FARPROC addr;
	if (addr=RemapLibrary(proc, hModule, Hooks)) return addr;
	return NULL;}

LONG WINAPI extWinVerifyTrust(HWND hWnd, GUID *pgActionID, LPVOID pWVTData)
{
	LONG ret;
	OutTraceD("WinVerifyTrust: hwnd=%x pgActionID=%x pWVTData=%x\n", hWnd, pgActionID, pWVTData);
	ret = (*pWinVerifyTrust)(hWnd, pgActionID, pWVTData);
	OutTraceD("WinVerifyTrust: ret=%x\n", ret);
	return ret;
}