#include "dxwnd.h"
#include "dxwcore.hpp"
#include "syslibs.h"
#include "dxhook.h"
#include "dxhelper.h"

#include "stdio.h"
#include "Dbghelp.h"

typedef PIMAGE_DEBUG_INFORMATION (WINAPI *MapDebugInformation_Type)(HANDLE, PCSTR, PCSTR, ULONG);
typedef BOOL (WINAPI *UnmapDebugInformation_Type)(PIMAGE_DEBUG_INFORMATION);
typedef BOOL (WINAPI *SymInitialize_Type)(HANDLE, PCTSTR, BOOL);
typedef BOOL (WINAPI *SymCleanup_Type)(HANDLE);
typedef BOOL (WINAPI *StackWalk_Type)(DWORD, HANDLE, HANDLE, LPSTACKFRAME, LPVOID, PREAD_PROCESS_MEMORY_ROUTINE, PFUNCTION_TABLE_ACCESS_ROUTINE, PGET_MODULE_BASE_ROUTINE, PTRANSLATE_ADDRESS_ROUTINE);
typedef LPVOID (WINAPI *SymFunctionTableAccess_Type)(HANDLE, DWORD);
typedef LPVOID (WINAPI *SymGetModuleBase_Type)(HANDLE, DWORD);
typedef BOOL (WINAPI *SymGetSymFromAddr_Type)(HANDLE, DWORD, LPDWORD, PIMAGEHLP_SYMBOL);

PIMAGE_DEBUG_INFORMATION WINAPI extMapDebugInformation(HANDLE, PCSTR, PCSTR, ULONG);
BOOL WINAPI extUnmapDebugInformation(PIMAGE_DEBUG_INFORMATION);
BOOL WINAPI extSymInitialize(HANDLE, PCTSTR, BOOL);
BOOL WINAPI extSymCleanup(HANDLE);
BOOL WINAPI extStackWalk(DWORD, HANDLE, HANDLE, LPSTACKFRAME, LPVOID, PREAD_PROCESS_MEMORY_ROUTINE, PFUNCTION_TABLE_ACCESS_ROUTINE, PGET_MODULE_BASE_ROUTINE, PTRANSLATE_ADDRESS_ROUTINE);
LPVOID WINAPI extSymFunctionTableAccess(HANDLE, DWORD);
LPVOID WINAPI extSymGetModuleBase(HANDLE, DWORD);
BOOL WINAPI extSymGetSymFromAddr(HANDLE, DWORD, LPDWORD, PIMAGEHLP_SYMBOL);

MapDebugInformation_Type pMapDebugInformation = NULL;
UnmapDebugInformation_Type pUnmapDebugInformation = NULL;
SymInitialize_Type pSymInitialize;
SymCleanup_Type pSymCleanup;
StackWalk_Type pStackWalk;
SymFunctionTableAccess_Type pSymFunctionTableAccess;
SymGetModuleBase_Type pSymGetModuleBase;
SymGetSymFromAddr_Type pSymGetSymFromAddr;

static HookEntryEx_Type Hooks[]={
	{HOOK_IAT_CANDIDATE, 0, "MapDebugInformation", (FARPROC)NULL, (FARPROC *)&pMapDebugInformation, (FARPROC)extMapDebugInformation},
	{HOOK_IAT_CANDIDATE, 0, "UnmapDebugInformation", (FARPROC)NULL, (FARPROC *)&pUnmapDebugInformation, (FARPROC)extUnmapDebugInformation},
	{HOOK_IAT_CANDIDATE, 0, "SymInitialize", (FARPROC)NULL, (FARPROC *)&pSymInitialize, (FARPROC)extSymInitialize},
	{HOOK_IAT_CANDIDATE, 0, "SymCleanup", (FARPROC)NULL, (FARPROC *)&pSymCleanup, (FARPROC)extSymCleanup},
	{HOOK_IAT_CANDIDATE, 0, "StackWalk", (FARPROC)NULL, (FARPROC *)&pStackWalk, (FARPROC)extStackWalk},
	{HOOK_IAT_CANDIDATE, 0, "SymFunctionTableAccess", (FARPROC)NULL, (FARPROC *)&pSymFunctionTableAccess, (FARPROC)extSymFunctionTableAccess},
	{HOOK_IAT_CANDIDATE, 0, "SymGetModuleBase", (FARPROC)NULL, (FARPROC *)&pSymGetModuleBase, (FARPROC)extSymGetModuleBase},
	{HOOK_IAT_CANDIDATE, 0, "SymGetSymFromAddr", (FARPROC)NULL, (FARPROC *)&pSymGetSymFromAddr, (FARPROC)extSymGetSymFromAddr},
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

BOOL WINAPI extSymInitialize(HANDLE hProcess, PCTSTR UserSearchPath, BOOL fInvadeProcess)
{
	OutTrace("SymInitialize: hprocess=%x usrsearchpath=\"%s\" invadeproc=%x\n", hProcess, UserSearchPath, fInvadeProcess);
	return TRUE;
}

BOOL WINAPI extSymCleanup(HANDLE hProcess)
{
	OutTrace("SymCleanup: hprocess=%x\n", hProcess);
	return TRUE;
}

BOOL WINAPI extStackWalk(DWORD MachineType, HANDLE hProcess, HANDLE hThread, LPSTACKFRAME StackFrame, LPVOID ContextRecord, 
						 PREAD_PROCESS_MEMORY_ROUTINE ReadMemoryRoutine, PFUNCTION_TABLE_ACCESS_ROUTINE FunctionTableAccessRoutine,
						 PGET_MODULE_BASE_ROUTINE GetModuleBaseRoutine, PTRANSLATE_ADDRESS_ROUTINE TranslateAddress)
{
	OutTrace("SymCleanup: hprocess=%x\n", hProcess);
	// too complex, pretend we have an error
	return FALSE;
}

LPVOID WINAPI extSymFunctionTableAccess(HANDLE hProcess, DWORD AddrBase)
{
	OutTrace("SymFunctionTableAccess: hprocess=%x addrbase=%x\n", hProcess, AddrBase);
	// too dangerous, pretend we have an error
	return NULL;
}
	
LPVOID WINAPI extSymGetModuleBase(HANDLE hProcess, DWORD AddrBase)
{
	OutTrace("SymGetModuleBase: hprocess=%x addrbase=%x\n", hProcess, AddrBase);
	// too dangerous, pretend we have an error
	return NULL;
}

BOOL WINAPI extSymGetSymFromAddr(HANDLE hProcess, DWORD Address, LPDWORD Displacement, PIMAGEHLP_SYMBOL Symbol)
{
	OutTrace("SymGetSymFromAddr: hprocess=%x address=%x\n", hProcess, Address);
	// too dangerous, pretend we have an error
	return NULL;
}
