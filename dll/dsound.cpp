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
typedef HRESULT	(WINAPI *SetCooperativeLevel_Type)  (void *, HWND, DWORD);

DirectSoundCreate_Type pDirectSoundCreate = NULL;
SetCooperativeLevel_Type pDSSetCooperativeLevel = NULL;

HRESULT WINAPI extDirectSoundCreate(LPGUID, LPDIRECTSOUND *, LPUNKNOWN);
HRESULT WINAPI extDSSetCooperativeLevel(void *, HWND, DWORD);

static HookEntry_Type Hooks[]={
	{HOOK_HOT_CANDIDATE, "DirectSoundCreate", (FARPROC)NULL, (FARPROC *)&pDirectSoundCreate, (FARPROC)extDirectSoundCreate},
	{HOOK_IAT_CANDIDATE, 0, NULL, 0, 0} // terminator
};

FARPROC Remap_DSound_ProcAddress(LPCSTR proc, HMODULE hModule)
{
	FARPROC addr;
	if (addr=RemapLibrary(proc, hModule, Hooks)) return addr;
	return NULL;
}

static char *libname = "dsound.dll";

void HookDirectSound(HMODULE hModule)
{
	HookLibrary(hModule, Hooks, libname);
}

void HookDirectSoundInit()
{
	HookLibInit(Hooks);
}

void HookDirectSoundObj(LPDIRECTSOUND *lpds)
{
	// IDIrectSound::SetCooperativeLevel
	SetHook((void *)(**(DWORD **)lpds + 24), extDSSetCooperativeLevel, (void **)&pDSSetCooperativeLevel, "SetCooperativeLevel(DSound)");
}

HRESULT WINAPI extDirectSoundCreate(LPGUID guid, LPDIRECTSOUND *lpds, LPUNKNOWN unk)
{
	HRESULT res;

	OutTrace("DirectSoundCreate: guid=%x\n", guid);
	res = (*pDirectSoundCreate)(guid, lpds, unk);
	if(res){
		OutTraceE("DirectSoundCreate ERROR: res=%x(%s)\n", res, ExplainDDError(res));
		return res;
	}

	HookDirectSoundObj(lpds);
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
