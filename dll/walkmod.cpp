#include <windows.h>
#include <stdio.h>
#include <stdarg.h>
#include "dxwnd.h"
#include "dxwcore.hpp"

//#include <windows.h> 
#include <tlhelp32.h> 
//#include <tchar.h> 
//#include "dxwnd.h"

extern void HookModule(HMODULE, int);
extern void HookOpenGLLibs(HMODULE, char *);
extern void DumpImportTable(HMODULE);

BOOL ListProcessModules(BOOL hook) 
{ 
	HANDLE hModuleSnap = INVALID_HANDLE_VALUE; 
	MODULEENTRY32 me32; 

	//  Take a snapshot of all modules in the specified process. 
	hModuleSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, GetCurrentProcessId()); 
	if( hModuleSnap == INVALID_HANDLE_VALUE){ 
		OutTraceD("CreateToolhelp32Snapshot ERROR: err=%d\n", GetLastError()); 
		return false;
	} 

	//  Set the size of the structure before using it. 
	me32.dwSize = sizeof( MODULEENTRY32 ); 
 
	//  Retrieve information about the first module, 
	//  and exit if unsuccessful 
	if( !Module32First(hModuleSnap, &me32)){ 
		OutTraceE("Module32First ERROR: err=%d\n", GetLastError());  // Show cause of failure 
		CloseHandle(hModuleSnap);     // Must clean up the snapshot object! 
		return false; 
	} 
 
	//  Now walk the module list of the process, 
	//  and display information about each module 
	do { 
		if(IsDebug){ 
			OutTraceD("MODULE NAME:%s\n", me32.szModule ); 
			OutTraceD("     executable     = %s\n",         me32.szExePath ); 
			OutTraceD("     process ID     = 0x%08X\n",     me32.th32ProcessID ); 
			OutTraceD("     ref count (g)  = 0x%04X\n",     me32.GlblcntUsage ); 
			OutTraceD("     ref count (p)  = 0x%04X\n",     me32.ProccntUsage ); 
			OutTraceD("     base address   = 0x%08X\n",		(DWORD) me32.modBaseAddr ); 
			OutTraceD("     base size      = %d\n",         me32.modBaseSize ); 
		}

		extern void HookSysLibs(HMODULE);
		if(hook) HookSysLibs((HMODULE)me32.modBaseAddr);
		
		if (IsDebug) DumpImportTable((HMODULE)me32.modBaseAddr);
	} while( Module32Next( hModuleSnap, &me32 ) ); 
	//  Do not forget to clean up the snapshot object. 
	CloseHandle( hModuleSnap ); 
	return true; 
} 
 