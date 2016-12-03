#define __MSC__
#include <stdio.h>
#include <stdlib.h>
#include "dxwnd.h"
#include "dxwcore.hpp"
#include "syslibs.h"
#include "dxhook.h"
#include "hddraw.h"
#include "dxhelper.h"
#include "glidehook.h"

void WINAPI extgrGlideInit(void);
void WINAPI extgrGlideShutdown(void);
void WINAPI extgrClipWindow(FxU32, FxU32, FxU32, FxU32);
GrContext_t WINAPI extgrSstWinOpen(FxU32, GrScreenResolution_t, GrScreenRefresh_t, GrColorFormat_t, GrOriginLocation_t, int, int);
FxI32 WINAPI extgrQueryResolutions (const GrResolution *, GrResolution *);

grGlideInit_Type pgrGlideInit = 0;
grGlideShutdown_Type pgrGlideShutdown = 0;
grClipWindow_Type pgrClipWindow = 0;
grSstWinOpen_Type pgrSstWinOpen = 0;
grQueryResolutions_Type pgrQueryResolutions = 0;

static HookEntryEx_Type Hooks[]={
	{HOOK_IAT_CANDIDATE, 0, "_grGlideInit@0", (FARPROC)NULL, (FARPROC *)&pgrGlideInit, (FARPROC)extgrGlideInit},
	{HOOK_IAT_CANDIDATE, 0, "_grGlideShutdown@0", (FARPROC)NULL, (FARPROC *)&pgrGlideShutdown, (FARPROC)extgrGlideShutdown},
	{HOOK_IAT_CANDIDATE, 0, "_grClipWindow@16", (FARPROC)NULL, (FARPROC *)&pgrClipWindow, (FARPROC)extgrClipWindow},
	{HOOK_IAT_CANDIDATE, 0, "_grSstWinOpen@28", (FARPROC)NULL, (FARPROC *)&pgrSstWinOpen, (FARPROC)extgrSstWinOpen},
	{HOOK_IAT_CANDIDATE, 0, "_grQueryResolutions@8", (FARPROC)NULL, (FARPROC *)&pgrQueryResolutions, (FARPROC)extgrQueryResolutions},
	{HOOK_IAT_CANDIDATE, 0, 0, NULL, 0, 0} // terminator
};

FARPROC Remap_Glide_ProcAddress(LPCSTR proc, HMODULE hModule)
{
	FARPROC addr;
	OutTrace("Remap_Glide_ProcAddress: proc=%s\n", proc);
	if(!(dxw.dwFlags4 & HOOKGLIDE)) return NULL; 
	if (addr=RemapLibraryEx(proc, hModule, Hooks)) return addr;
	// NULL -> keep the original call address
	return NULL;
}

void HookGlideLibs(HMODULE hModule)
{
	OutTraceDW("HookGlideLibs module=%x\n", hModule);
	HookLibraryEx(hModule, Hooks, "glide.dll");
	HookLibraryEx(hModule, Hooks, "glide2x.dll");
	HookLibraryEx(hModule, Hooks, "glide3x.dll");
	return;
}

// static utility 

static char *ExplainGlideRes(int res)
{
	char *p;
	switch(res){
		case GR_RESOLUTION_320x200: p="320x200"; break;
		case GR_RESOLUTION_320x240: p="320x240"; break;
		case GR_RESOLUTION_400x256: p="400x256"; break;
		case GR_RESOLUTION_512x384: p="512x384"; break;
		case GR_RESOLUTION_640x200: p="640x200"; break;
		case GR_RESOLUTION_640x350: p="640x350"; break;
		case GR_RESOLUTION_640x400: p="640x400"; break;
		case GR_RESOLUTION_640x480: p="640x480"; break;
		case GR_RESOLUTION_800x600: p="800x600"; break;
		case GR_RESOLUTION_960x720: p="960x720"; break;
		case GR_RESOLUTION_856x480: p="856x480"; break;
		case GR_RESOLUTION_512x256: p="512x256"; break;
		case GR_RESOLUTION_1024x768: p="1024x768"; break;
		case GR_RESOLUTION_1280x1024: p="1280x1024"; break;
		case GR_RESOLUTION_1600x1200: p="1600x1200"; break;
		case GR_RESOLUTION_400x300: p="400x300"; break;
		case GR_RESOLUTION_NONE: p="NONE"; break;
		default: p="unknown"; break;
	}
	return p;
}

// glide wrappers

void WINAPI extgrClipWindow(FxU32 minx, FxU32 miny, FxU32 maxx, FxU32 maxy)
{
	OutTrace("grClipWindow: win=(%d,%d)-(%d,%d)\n", minx, miny, maxx, maxy);
	return (*pgrClipWindow)(minx, miny, maxx, maxy);
}

GrContext_t WINAPI extgrSstWinOpen(
	FxU32 hWin,
	GrScreenResolution_t res,
	GrScreenRefresh_t ref,
	GrColorFormat_t cFormat,
	GrOriginLocation_t org_loc,
	int num_buffers,
	int num_aux_buffers)
{
	GrContext_t ret;

	OutTrace("grSstWinOpen: hWin=%x res=%x(%s)\n", hWin, res, ExplainGlideRes(res));

	if(dxw.IsDesktop((HWND)hWin)) hWin=(FxU32)dxw.GethWnd();
	ret=(*pgrSstWinOpen)(hWin, res, ref, cFormat, org_loc, num_buffers, num_aux_buffers);
	OutTrace("grSstWinOpen: ret=%x\n", ret);
	return ret;
}

FxI32 WINAPI extgrQueryResolutions (const GrResolution *resTemplate, GrResolution *output)
{
	FxI32 ret;

	OutTrace("grQueryResolutions: resolution=%x refresh=%x numColorBuffers=%d numAuxBuffers=%d\n", 
		resTemplate->resolution, resTemplate->refresh, resTemplate->numColorBuffers, resTemplate->numAuxBuffers);

	ret=(*pgrQueryResolutions)(resTemplate, output);
	OutTrace("grQueryResolutions: res=%d\n", ret); 
	return ret;
}

void WINAPI extgrGlideInit(void)
{
	OutTrace("grGlideInit\n");
	return;
}

void WINAPI extgrGlideShutdown(void)
{
	OutTrace("grGlideShutdown\n");
	return;
}

