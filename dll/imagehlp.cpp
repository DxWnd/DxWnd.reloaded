#include "dxwnd.h"
#include "dxwcore.hpp"
#include "syslibs.h"
#include "dxhook.h"
#include "dxhelper.h"

#include "stdio.h"
#include "Dbghelp.h"

typedef PIMAGE_DEBUG_INFORMATION (WINAPI *MapDebugInformation_Type)(HANDLE, PCSTR, PCSTR, ULONG);
typedef BOOL (WINAPI *UnmapDebugInformation_Type)(PIMAGE_DEBUG_INFORMATION);
PIMAGE_DEBUG_INFORMATION WINAPI extMapDebugInformation(HANDLE, PCSTR, PCSTR, ULONG);
BOOL WINAPI extUnmapDebugInformation(PIMAGE_DEBUG_INFORMATION);
MapDebugInformation_Type pMapDebugInformation = NULL;
UnmapDebugInformation_Type pUnmapDebugInformation = NULL;

static HookEntryEx_Type Hooks[]={
	{HOOK_IAT_CANDIDATE, 0, "MapDebugInformation", (FARPROC)NULL, (FARPROC *)&pMapDebugInformation, (FARPROC)extMapDebugInformation},
	{HOOK_IAT_CANDIDATE, 0, "UnmapDebugInformation", (FARPROC)NULL, (FARPROC *)&pUnmapDebugInformation, (FARPROC)extUnmapDebugInformation},
	{HOOK_IAT_CANDIDATE, 0, 0, NULL, 0, 0} // terminator
}; 

static char *libname = "IMAGEHLP.DLL";

void HookImagehlpInit()
{
	HookLibInitEx(Hooks);
}

void HookImagehlp(HMODULE module)
{
	if(dxw.dwFlags5 & NOIMAGEHLP) HookLibraryEx(module, Hooks, libname);
}

FARPROC Remap_Imagehlp_ProcAddress(LPCSTR proc, HMODULE hModule)
{
	FARPROC addr;

	if(dxw.dwFlags5 & NOIMAGEHLP) {
		if(addr=RemapLibraryEx(proc, hModule, Hooks)) return addr;
	}

	return NULL;
}

// =================== rerouted calls ==========================

IMAGE_DEBUG_INFORMATION DummyDebugInfo;

PIMAGE_DEBUG_INFORMATION WINAPI extMapDebugInformation(HANDLE FileHandle, PCSTR FileName, PCSTR SymbolPath, ULONG ImageBase)
{
	OutTrace("MapDebugInformation: FileHandle=%x FileName=%s SymbolPath=%s ImageBase=%x\n", FileHandle, FileName, SymbolPath, ImageBase);
	memset(&DummyDebugInfo, 0, sizeof(IMAGE_DEBUG_INFORMATION));
	return &DummyDebugInfo;
}

BOOL WINAPI extUnmapDebugInformation(PIMAGE_DEBUG_INFORMATION DebugInfo)
{
	OutTrace("UnmapDebugInformation: DebugInfo=%x\n", DebugInfo);
	return TRUE;
}

