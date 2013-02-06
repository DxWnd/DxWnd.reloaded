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

FARPROC Remap_smack_ProcAddress(LPCSTR proc, HMODULE hModule)
{
	if (!strcmp(proc,"_SmackOpen@12") && !pSmackOpen){
		pSmackOpen=(SmackOpen_Type)(*pGetProcAddress)(hModule, proc);
		OutTraceD("GetProcAddress: hooking proc=%s at addr=%x\n", ProcToString(proc), pSmackOpen);
		return (FARPROC)extSmackOpen;
	}
	// NULL -> keep the original call address
	return NULL;
}

SmackStruct * WINAPI extSmackOpen(HANDLE SmackFile, UINT32 flags, INT32 unknown)
{
	SmackStruct *ret;
	OutTraceD("SmackOpen: SmackFile=%x flags=%x unknown=%x\n", SmackFile, flags, unknown);
	if(!pSmackOpen) OutTraceE("ASSERT: NULL pSmackOpen\n");
	ret=(*pSmackOpen)(SmackFile, flags, unknown);
	OutTraceD("SmackOpen: ret=%x\n", ret);
	if (ret) {
		OutTraceD("SmackOpen: version=%x screen=(%dx%d) frame_count=%d frame_number=%d\n", 
			ret->version, ret->width, ret->height, ret->frame_count, ret->frame_number);
		//ret->width=800;
		//ret->height=600;
	}
	return ret;
}


