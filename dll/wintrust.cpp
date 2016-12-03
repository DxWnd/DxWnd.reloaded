#include <dxdiag.h>
#include "dxwnd.h"
#include "dxwcore.hpp"
#include "syslibs.h"
#include "dxhook.h"
#include "dxhelper.h"

typedef LONG	(WINAPI *WinVerifyTrust_Type)(HWND, GUID *, LPVOID);
WinVerifyTrust_Type pWinVerifyTrust;
extern LONG WINAPI extWinVerifyTrust(HWND, GUID *, LPVOID);

static HookEntryEx_Type Hooks[]={
	{HOOK_IAT_CANDIDATE, 0, "WinVerifyTrust", NULL, (FARPROC *)&pWinVerifyTrust, (FARPROC)extWinVerifyTrust},
	{HOOK_IAT_CANDIDATE, 0, 0, NULL, 0, 0} // terminator
};

void HookTrust(HMODULE module)
{
	HookLibraryEx(module, Hooks, "wintrust.dll");
}

FARPROC Remap_trust_ProcAddress(LPCSTR proc, HMODULE hModule)
{
	FARPROC addr;
	if (addr=RemapLibraryEx(proc, hModule, Hooks)) return addr;
	return NULL;}

LONG WINAPI extWinVerifyTrust(HWND hWnd, GUID *pgActionID, LPVOID pWVTData)
{
	LONG ret;
	OutTraceDW("WinVerifyTrust: hwnd=%x pgActionID=%x pWVTData=%x\n", hWnd, pgActionID, pWVTData);
	ret = (*pWinVerifyTrust)(hWnd, pgActionID, pWVTData);
	OutTraceDW("WinVerifyTrust: ret=%x\n", ret);
	return ret;
}
