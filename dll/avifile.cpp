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
typedef PGETFRAME (WINAPI *AVIStreamGetFrameOpen_Type)(PAVISTREAM, LPBITMAPINFOHEADER);

AVIFileRelease_Type pAVIFileRelease = NULL;
AVIStreamRelease_Type pAVIStreamRelease = NULL;
AVIStreamGetFrameOpen_Type pAVIStreamGetFrameOpen = NULL;

LONG WINAPI extAVIFileRelease(PAVIFILE);
LONG WINAPI extAVIStreamRelease(PAVISTREAM);
PGETFRAME WINAPI extAVIStreamGetFrameOpen(PAVISTREAM, LPBITMAPINFOHEADER);

static HookEntryEx_Type Hooks[]={
	//{HOOK_IAT_CANDIDATE, 0, "AVIFileClose", NULL, (FARPROC *)&pAVIFileClose, (FARPROC)extAVIFileClose},
	//{HOOK_IAT_CANDIDATE, 0, "AVIFileRelease", NULL, (FARPROC *)&pAVIFileRelease, (FARPROC)extAVIFileRelease},
	//{HOOK_IAT_CANDIDATE, 0, "AVIStreamRelease", NULL, (FARPROC *)&pAVIStreamRelease, (FARPROC)extAVIStreamRelease},
	{HOOK_IAT_CANDIDATE, 0, "AVIStreamGetFrameOpen", NULL, (FARPROC *)&pAVIStreamGetFrameOpen, (FARPROC)extAVIStreamGetFrameOpen},
	{HOOK_IAT_CANDIDATE, 0, 0, NULL, 0, 0} // terminator
};

void HookAVIFil32(HMODULE module)
{
	HookLibraryEx(module, Hooks, "AVIFIL32.dll");
}

FARPROC Remap_AVIFil32_ProcAddress(LPCSTR proc, HMODULE hModule)
{
	FARPROC addr;
	if (addr=RemapLibraryEx(proc, hModule, Hooks)) return addr;
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

PGETFRAME WINAPI extAVIStreamGetFrameOpen(PAVISTREAM pavi, LPBITMAPINFOHEADER lpbiWanted)
{
	if(IsTraceDW){
		char sColor[12];
		switch ((DWORD)lpbiWanted){
			case 0: strcpy(sColor, "DEFAULT"); break;
			case AVIGETFRAMEF_BESTDISPLAYFMT: strcpy(sColor, "BEST"); break;
			default: sprintf(sColor,"%d", lpbiWanted->biBitCount);
		}
		OutTrace("AVIStreamGetFrameOpen: pavi=%x lpbiwanted=%x bitcount=%s\n", pavi, lpbiWanted, sColor);
	}

	if((dxw.dwFlags6 & FIXMOVIESCOLOR) && (((DWORD)lpbiWanted == 0) || ((DWORD)lpbiWanted == AVIGETFRAMEF_BESTDISPLAYFMT))){
		BITMAPINFOHEADER biWanted;
		memset(&biWanted, 0, sizeof(BITMAPINFOHEADER));
		biWanted.biSize = sizeof(BITMAPINFOHEADER);
		biWanted.biBitCount = (WORD)dxw.VirtualPixelFormat.dwRGBBitCount;
		biWanted.biPlanes = 1;
		if(biWanted.biBitCount < 32) 
		biWanted.biClrUsed = (0x1 << biWanted.biBitCount); // 8 -> 256;
		else
			biWanted.biClrUsed = 0;
		biWanted.biClrImportant = biWanted.biClrUsed;
		return (*pAVIStreamGetFrameOpen)(pavi, &biWanted);
	}

	return (*pAVIStreamGetFrameOpen)(pavi, lpbiWanted);
}


