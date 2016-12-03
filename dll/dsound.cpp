#define _WIN32_WINNT 0x0600
#define WIN32_LEAN_AND_MEAN
#define _CRT_SECURE_NO_WARNINGS
#define _CRT_NON_CONFORMING_SWPRINTFS

#include <stdio.h>
#include <stdlib.h>
#include "dxwnd.h"
#include "dxwcore.hpp"
#include "syslibs.h"
#include "dxhook.h"
#include "dsound.h"
#include "dxhelper.h"

typedef HRESULT	(WINAPI *DirectSoundCreate_Type)(LPGUID, LPDIRECTSOUND *, LPUNKNOWN);
typedef HRESULT (WINAPI *DirectSoundCreate8_Type)(LPCGUID, LPDIRECTSOUND8 *, LPUNKNOWN);
typedef HRESULT	(WINAPI *SetCooperativeLevel_Type)  (void *, HWND, DWORD);

DirectSoundCreate_Type pDirectSoundCreate = NULL;
DirectSoundCreate8_Type pDirectSoundCreate8 = NULL;
SetCooperativeLevel_Type pDSSetCooperativeLevel = NULL;

HRESULT WINAPI extDirectSoundCreate(LPGUID, LPDIRECTSOUND *, LPUNKNOWN);
HRESULT WINAPI extDirectSoundCreate8(LPCGUID, LPDIRECTSOUND8 *, LPUNKNOWN);
HRESULT WINAPI extDSSetCooperativeLevel(void *, HWND, DWORD);

static HookEntryEx_Type Hooks[]={
	{HOOK_HOT_CANDIDATE, 0x0001, "DirectSoundCreate", (FARPROC)NULL, (FARPROC *)&pDirectSoundCreate, (FARPROC)extDirectSoundCreate},
	{HOOK_HOT_CANDIDATE, 0x000B, "DirectSoundCreate8", (FARPROC)NULL, (FARPROC *)&pDirectSoundCreate8, (FARPROC)extDirectSoundCreate8},
	{HOOK_IAT_CANDIDATE, 0, 0, NULL, 0, 0} // terminator
};

FARPROC Remap_DSound_ProcAddress(LPCSTR proc, HMODULE hModule)
{
	FARPROC addr;
	if (addr=RemapLibraryEx(proc, hModule, Hooks)) return addr;
	return NULL;
}

static char *libname = "dsound.dll";

void HookDirectSound(HMODULE hModule)
{
	HookLibraryEx(hModule, Hooks, libname);
}

void HookDirectSoundInit()
{
	HookLibInitEx(Hooks);
}

void HookDirectSoundObj(LPDIRECTSOUND *lpds)
{
	// IDIrectSound::SetCooperativeLevel
	SetHook((void *)(**(DWORD **)lpds + 24), extDSSetCooperativeLevel, (void **)&pDSSetCooperativeLevel, "SetCooperativeLevel(DSound)");
}

HRESULT WINAPI extDirectSoundCreate(LPGUID guid, LPDIRECTSOUND *lpds, LPUNKNOWN unk)
{
	HRESULT res;

	OutTraceDW("DirectSoundCreate: guid=%x\n", guid);
	res = (*pDirectSoundCreate)(guid, lpds, unk);
	if(res){
		OutTraceE("DirectSoundCreate ERROR: res=%x(%s)\n", res, ExplainDDError(res));
	}
	else {
		OutTraceDW("DirectSoundCreate lpDS=%x\n", *lpds);
		HookDirectSoundObj(lpds);
	}
	return res;
}

HRESULT WINAPI extDirectSoundCreate8(LPCGUID lpcGuidDevice, LPDIRECTSOUND8 *ppDS8, LPUNKNOWN pUnkOuter)
{
	HRESULT res;

	OutTraceDW("DirectSoundCreate8: guid=%x\n", lpcGuidDevice);
	res = (*pDirectSoundCreate8)(lpcGuidDevice, ppDS8, pUnkOuter);
	res = DD_OK;
	if(res){
		OutTraceE("DirectSoundCreate8 ERROR: res=%x(%s)\n", res, ExplainDDError(res));
	}
	else {
		OutTraceDW("DirectSoundCreate8 lpDS8=%x\n", *ppDS8);
		HookDirectSoundObj((LPDIRECTSOUND *)ppDS8);
	}
	return res;
}

#define DSSCL_NORMAL                0x00000001
#define DSSCL_PRIORITY              0x00000002
#define DSSCL_EXCLUSIVE             0x00000003
#define DSSCL_WRITEPRIMARY          0x00000004

static char *ExplainLevel(DWORD lev)
{
	char *s;
	switch(lev){
		case DSSCL_NORMAL: s="NORMAL"; break;
		case DSSCL_PRIORITY: s="PRIORITY"; break;
		case DSSCL_EXCLUSIVE: s="EXCLUSIVE"; break;
		case DSSCL_WRITEPRIMARY: s="WRITEPRIMARY"; break;
		default: s="invalid"; break;
	}
	return s;
}

HRESULT WINAPI extDSSetCooperativeLevel(void *lpds, HWND hwnd, DWORD dwLevel)
{
	HRESULT res;

	OutTrace("DirectSound::SetCooperativeLevel: lpds=%x hwnd=%x level=%x(%s)\n", lpds, hwnd, dwLevel, ExplainLevel(dwLevel));
	if(dwLevel == DSSCL_EXCLUSIVE) {
		OutTrace("DirectSound::SetCooperativeLevel: PATCH\n");
		dwLevel=DSSCL_PRIORITY; // Arxel Tribe patch
		//dwLevel=DSSCL_NORMAL; // Arxel Tribe patch
		hwnd=dxw.GethWnd();
		//hwnd=(*pGetDesktopWindow)();
		//hwnd=0;
	}
	res = (*pDSSetCooperativeLevel)(lpds, hwnd, dwLevel);
	if(res){
		OutTraceE("DirectSound::SetCooperativeLevel ERROR: res=%x(%s)\n", res, ExplainDDError(res));
	}
	return res;
}
