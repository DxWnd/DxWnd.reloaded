#include "dxwnd.h"
#include "dxwcore.hpp"
#include "syslibs.h"
#include "dxhook.h"
#include "Vfw.h"

#define DXWDECLARATIONS TRUE
#include "msvfwhook.h"
#undef DXWDECLARATIONS

FARPROC Remap_vfw_ProcAddress(LPCSTR proc, HMODULE hModule)
{
	if (!strcmp(proc,"ICSendMessage")){
		pICSendMessage=(ICSendMessage_Type)(*pGetProcAddress)(hModule, proc);
		OutTraceD("GetProcAddress: hooking proc=%s at addr=%x\n", ProcToString(proc), pICSendMessage);
		return (FARPROC)extICSendMessage;
	}
	if (!strcmp(proc,"ICOpen")){
		pICOpen=(ICOpen_Type)(*pGetProcAddress)(hModule, proc);
		OutTraceD("GetProcAddress: hooking proc=%s at addr=%x\n", ProcToString(proc), pICOpen);
		return (FARPROC)extICOpen;
	}

	// NULL -> keep the original call address
	return NULL;
}

void HookMSV4WLibs(HMODULE module)
{
	void *tmp;
	tmp = HookAPI(module, "MSVFW32.dll", NULL, "ICSendMessage", extICSendMessage);
	if(tmp) pICSendMessage = (ICSendMessage_Type)tmp;
	tmp = HookAPI(module, "MSVFW32.dll", NULL, "ICOpen", extICOpen);
	if(tmp) pICOpen = (ICOpen_Type)tmp;

	return;
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
