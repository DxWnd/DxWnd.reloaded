#include "dxwnd.h"
#include "dxwcore.hpp"
#include "syslibs.h"
#include "dxhook.h"
#include "Vfw.h"

#define DXWDECLARATIONS TRUE
#include "msvfwhook.h"
#undef DXWDECLARATIONS

static HookEntry_Type Hooks[]={
	{"ICSendMessage", (FARPROC)NULL, (FARPROC *)&pICSendMessage, (FARPROC)extICSendMessage},
	{"ICOpen", (FARPROC)NULL, (FARPROC *)&pICOpen, (FARPROC)extICOpen},
	{0, NULL, 0, 0} // terminator
};

FARPROC Remap_vfw_ProcAddress(LPCSTR proc, HMODULE hModule)
{
	FARPROC addr;
	if (addr=RemapLibrary(proc, hModule, Hooks)) return addr;
	// NULL -> keep the original call address
	return NULL;
}

void HookMSV4WLibs(HMODULE module)
{
	HookLibrary(module, Hooks, "MSVFW32.dll");
}

LRESULT WINAPI extICSendMessage(HIC hic, UINT wMsg, DWORD_PTR dw1, DWORD_PTR dw2)
{
	LRESULT res;
	OutTraceD("ICSendMessage: hic=%x wMsg=%x dw1=%x dw2=%x\n", hic, wMsg, dw1, dw2);
	return 0;
	res=(*pICSendMessage)(hic, wMsg, dw1, dw2);
	return res;
}

HIC WINAPI extICOpen(DWORD fccType, DWORD fccHandler, UINT wMode)
{
	OutTraceD("ICOpen: fccType=%x fccHandler=%x wMode=%x\n", fccType, fccHandler, wMode);
	return (HIC)0;
}
