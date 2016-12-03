#define _CRT_SECURE_NO_WARNINGS

#include "dxwnd.h"
#include "dxwcore.hpp"
#include "syslibs.h"
#include "hddraw.h"
#include "dxhook.h"
#include "dxhelper.h"
#include "shareddc.hpp"

#include "stdio.h"

static BOOL bGDIRecursionFlag = FALSE;

#define _Warn(s) MessageBox(0, s, "to do", MB_ICONEXCLAMATION)

typedef BOOL (WINAPI *AlphaBlend_Type)(HDC hdcDest, int xoriginDest, int yoriginDest, int wDest, int hDest, HDC hdcSrc, int xoriginSrc, int yoriginSrc, int wSrc, int hSrc, BLENDFUNCTION ftn);
AlphaBlend_Type pAlphaBlend = NULL;
BOOL WINAPI extAlphaBlend(HDC hdcDest, int xoriginDest, int yoriginDest, int wDest, int hDest, HDC hdcSrc, int xoriginSrc, int yoriginSrc, int wSrc, int hSrc, BLENDFUNCTION ftn);

typedef BOOL (WINAPI *GradientFill_Type)(HDC, PTRIVERTEX, ULONG, PVOID, ULONG, ULONG);
GradientFill_Type pGradientFill = NULL;
BOOL WINAPI extGradientFill(HDC, PTRIVERTEX, ULONG, PVOID, ULONG, ULONG);

typedef BOOL (WINAPI *TransparentBlt_Type)(HDC, int, int, int, int, HDC, int, int, int, int, UINT);
TransparentBlt_Type pTransparentBlt = NULL;
BOOL WINAPI extTransparentBlt(HDC, int, int, int, int, HDC, int, int, int, int, UINT);

static HookEntryEx_Type Hooks[]={

	{HOOK_IAT_CANDIDATE, 0, "AlphaBlend", (FARPROC)NULL, (FARPROC *)&pAlphaBlend, (FARPROC)extAlphaBlend},
	{HOOK_IAT_CANDIDATE, 0, "GradientFill", (FARPROC)NULL, (FARPROC *)&pGradientFill, (FARPROC)extGradientFill},
	{HOOK_IAT_CANDIDATE, 0, "TransparentBlt", (FARPROC)NULL, (FARPROC *)&pTransparentBlt, (FARPROC)extTransparentBlt},
	{HOOK_IAT_CANDIDATE, 0, 0, NULL, 0, 0} // terminator
}; 

static char *libname = "msimg32.dll";

void HookMSIMG32Init()
{
	HookLibInitEx(Hooks);
}

void HookMSIMG32(HMODULE module)
{
	HookLibraryEx(module, Hooks, libname);
}

FARPROC Remap_MSIMG32_ProcAddress(LPCSTR proc, HMODULE hModule)
{
	FARPROC addr;

	if(addr=RemapLibraryEx(proc, hModule, Hooks)) return addr;
	return NULL;
}

BOOL WINAPI extAlphaBlend(HDC hdcDest, int xoriginDest, int yoriginDest, int wDest, int hDest, HDC hdcSrc, int xoriginSrc, int yoriginSrc, int wSrc, int hSrc, BLENDFUNCTION ftn)
{
	_Warn("AlphaBlend");
	return TRUE;
}

BOOL WINAPI extGradientFill(HDC hdc, PTRIVERTEX pVertex, ULONG nVertex, PVOID pMesh, ULONG nMesh, ULONG ulMode)
{
	_Warn("GradientFill");
	return TRUE;
}

BOOL WINAPI extTransparentBlt(HDC hdcDest, int xoriginDest, int yoriginDest, int wDest, int hDest,
							  HDC  hdcSrc, int xoriginSrc, int yoriginSrc, int wSrc, int hSrc, UINT crTransparent)
{
	_Warn("TransparentBlt");
	return TRUE;
}
