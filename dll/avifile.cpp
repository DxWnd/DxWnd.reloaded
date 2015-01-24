#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include "dxwnd.h"
#include "dxwcore.hpp"
//#include "syslibs.h"
#include "dxhook.h"
#include "dxhelper.h"
#include <Vfw.h>

typedef LONG	(WINAPI *AVIFileRelease_Type)(PAVIFILE);
typedef LONG	(WINAPI *AVIStreamRelease_Type)(PAVISTREAM);

AVIFileRelease_Type pAVIFileRelease = NULL;
AVIStreamRelease_Type pAVIStreamRelease = NULL;

LONG WINAPI extAVIFileRelease(PAVIFILE);
LONG WINAPI extAVIStreamRelease(PAVISTREAM);

static HookEntry_Type Hooks[]={
	//{HOOK_IAT_CANDIDATE, "AVIFileClose", NULL, (FARPROC *)&pAVIFileClose, (FARPROC)extAVIFileClose},
	{HOOK_IAT_CANDIDATE, "AVIFileRelease", NULL, (FARPROC *)&pAVIFileRelease, (FARPROC)extAVIFileRelease},
	{HOOK_IAT_CANDIDATE, "AVIStreamRelease", NULL, (FARPROC *)&pAVIStreamRelease, (FARPROC)extAVIStreamRelease},
	{HOOK_IAT_CANDIDATE, 0, NULL, 0, 0} // terminator
};

void HookAVIFil32(HMODULE module)
{
	HookLibrary(module, Hooks, "AVIFIL32.dll");
}

FARPROC Remap_AVIFil32_ProcAddress(LPCSTR proc, HMODULE hModule)
{
	FARPROC addr;
	if (addr=RemapLibrary(proc, hModule, Hooks)) return addr;
	return NULL;
}

LONG WINAPI extAVIFileRelease(PAVIFILE pavi)
{
	if(pavi==NULL) {
		OutTraceE("AVIFileRelease: pavi=NULL condition - returns OK\n");
		return 0;
	}
	return (*pAVIFileRelease)(pavi);
}

LONG WINAPI extAVIStreamRelease(PAVISTREAM pavi)
{
	if(pavi==NULL) {
		OutTraceE("AVIFileRelease: pavi=NULL condition - returns OK\n");
		return 0;
	}
	return (*pAVIStreamRelease)(pavi);
}



