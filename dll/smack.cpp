#include <windows.h>
#include "dxwnd.h"
#include "dxwcore.hpp"
#include "syslibs.h"
#include "dxhook.h"

#pragma pack(1)

typedef struct {
  UINT32 version;
  UINT32 width;
  UINT32 height;
  UINT32 frame_count;
  UINT32 mspf;
  char unknown[864];
  UINT32 frame_number;
} SmackStruct;

typedef SmackStruct * (WINAPI *SmackOpen_Type)(HANDLE, UINT32, INT32);

SmackOpen_Type pSmackOpen = NULL;

SmackStruct * WINAPI extSmackOpen(HANDLE, UINT32, INT32);

static HookEntryEx_Type Hooks[]={
	{HOOK_IAT_CANDIDATE, 0, "_SmackOpen@12", (FARPROC)NULL, (FARPROC *)&pSmackOpen, (FARPROC)extSmackOpen},
	{HOOK_IAT_CANDIDATE, 0, 0, NULL, 0, 0} // terminator
};

FARPROC Remap_smack_ProcAddress(LPCSTR proc, HMODULE hModule)
{
	FARPROC addr;
	if (addr=RemapLibraryEx(proc, hModule, Hooks)) return addr;
	// NULL -> keep the original call address
	return NULL;
}

static char *libname = "smackw32.dll";

void HookSmackW32(HMODULE hModule)
{
	HookLibraryEx(hModule, Hooks, libname);
	return;
}

DWORD ThreadId = 0;

DWORD WINAPI DoScreenRefresh(LPVOID lpParameter)
{
	while(TRUE){
		Sleep(20);
		dxw.ScreenRefresh();
	}
}

SmackStruct * WINAPI extSmackOpen(HANDLE SmackFile, UINT32 flags, INT32 unknown)
{
	SmackStruct *ret;
	OutTraceDW("SmackOpen: SmackFile=%x flags=%x unknown=%x\n", SmackFile, flags, unknown);
	if(!pSmackOpen) OutTraceE("ASSERT: NULL pSmackOpen\n");
	ret=(*pSmackOpen)(SmackFile, flags, unknown);
	OutTraceDW("SmackOpen: ret=%x\n", ret);
	if (ret) {
		OutTraceDW("SmackOpen: version=%x screen=(%dx%d) frame_count=%d frame_number=%d\n", 
			ret->version, ret->width, ret->height, ret->frame_count, ret->frame_number);
	}
	return ret;
}


