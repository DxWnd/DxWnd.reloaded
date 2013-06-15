#include "dxwnd.h"
#include "dxwcore.hpp"
#include "syslibs.h"
#include "dxhook.h"

static HookEntry_Type TimeHooks[]={
	{"timeGetTime", NULL, (FARPROC *)&ptimeGetTime, (FARPROC)exttimeGetTime},
	{0, NULL, 0, 0} // terminator
};

void HookWinMM(HMODULE module)
{
	if(dxw.dwFlags2 & TIMESTRETCH) HookLibrary(module, TimeHooks, "winmm.dll");
}

FARPROC Remap_WinMM_ProcAddress(LPCSTR proc, HMODULE hModule)
{
	FARPROC addr;

	if(dxw.dwFlags2 & TIMESTRETCH)
		if (addr=RemapLibrary(proc, hModule, TimeHooks)) return addr;

	return NULL;
}

DWORD WINAPI exttimeGetTime(void)
{
	DWORD ret;
	ret = dxw.GetTickCount();
	if (IsDebug) OutTrace("timeGetTime: time=%x\n", ret);
	return ret;
}
