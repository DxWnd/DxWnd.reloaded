#include "dxwnd.h"
#include "dxwcore.hpp"
#include "syslibs.h"
#include "dxhook.h"
#include "dxhelper.h"
#include "hddraw.h"
#include "ddproxy.h"

//#undef IsTraceDW
//#define IsTraceDW TRUE

static HookEntry_Type Hooks[]={
	{"IsDebuggerPresent", (FARPROC)NULL, (FARPROC *)NULL, (FARPROC)extIsDebuggerPresent},
	{"GetProcAddress", (FARPROC)GetProcAddress, (FARPROC *)&pGetProcAddress, (FARPROC)extGetProcAddress},
	{"LoadLibraryA", (FARPROC)LoadLibraryA, (FARPROC *)&pLoadLibraryA, (FARPROC)extLoadLibraryA},
	{"LoadLibraryExA", (FARPROC)LoadLibraryExA, (FARPROC *)&pLoadLibraryExA, (FARPROC)extLoadLibraryExA},
	{"LoadLibraryW", (FARPROC)LoadLibraryW, (FARPROC *)&pLoadLibraryW, (FARPROC)extLoadLibraryW},
	{"LoadLibraryExW", (FARPROC)LoadLibraryExW, (FARPROC *)&pLoadLibraryExW, (FARPROC)extLoadLibraryExW},
	{"GetDriveTypeA", (FARPROC)NULL, (FARPROC *)&pGetDriveType, (FARPROC)extGetDriveType},
	{0, NULL, 0, 0} // terminator
};

static HookEntry_Type FixIOHooks[]={
	{"ReadFile", (FARPROC)NULL, (FARPROC *)&pReadFile, (FARPROC)extReadFile},
	{"CreateFileA", (FARPROC)NULL, (FARPROC *)&pCreateFile, (FARPROC)extCreateFile},
	{"SetFilePointer", (FARPROC)NULL, (FARPROC *)&pSetFilePointer, (FARPROC)extSetFilePointer},
	{"CloseHandle", (FARPROC)NULL, (FARPROC *)&pCloseHandle, (FARPROC)extCloseHandle},
	{0, NULL, 0, 0} // terminator
};

static HookEntry_Type LimitHooks[]={
	{"GetDiskFreeSpaceA", (FARPROC)GetDiskFreeSpaceA, (FARPROC *)&pGetDiskFreeSpaceA, (FARPROC)extGetDiskFreeSpaceA},
	{"GlobalMemoryStatus", (FARPROC)GlobalMemoryStatus, (FARPROC *)&pGlobalMemoryStatus, (FARPROC)extGlobalMemoryStatus},
	{0, NULL, 0, 0} // terminator
};

static HookEntry_Type TimeHooks[]={
	{"GetTickCount", (FARPROC)GetTickCount, (FARPROC *)&pGetTickCount, (FARPROC)extGetTickCount},
	{"GetLocalTime", (FARPROC)GetLocalTime, (FARPROC *)&pGetLocalTime, (FARPROC)extGetLocalTime},
	{"GetSystemTime", (FARPROC)GetSystemTime, (FARPROC *)&pGetSystemTime, (FARPROC)extGetSystemTime},
	{"GetSystemTimeAsFileTime", (FARPROC)GetSystemTimeAsFileTime, (FARPROC *)&pGetSystemTimeAsFileTime, (FARPROC)extGetSystemTimeAsFileTime},
	{"Sleep", (FARPROC)Sleep, (FARPROC *)&pSleep, (FARPROC)extSleep},
	{"SleepEx", (FARPROC)SleepEx, (FARPROC *)&pSleepEx, (FARPROC)extSleepEx},
	{"QueryPerformanceCounter", (FARPROC)NULL, (FARPROC *)&pQueryPerformanceCounter, (FARPROC)extQueryPerformanceCounter},
	{"QueryPerformanceFrequency", (FARPROC)NULL, (FARPROC *)&pQueryPerformanceFrequency, (FARPROC)extQueryPerformanceFrequency},
	{0, NULL, 0, 0} // terminator
};

static HookEntry_Type VersionHooks[]={
	{"GetVersion", (FARPROC)GetVersion, (FARPROC *)&pGetVersion, (FARPROC)extGetVersion},
	{"GetVersionExA", (FARPROC)GetVersionExA, (FARPROC *)&pGetVersionExA, (FARPROC)extGetVersionExA},
	{"GetVersionExW", (FARPROC)GetVersionExW, (FARPROC *)&pGetVersionExW, (FARPROC)extGetVersionExW},
	{0, NULL, 0, 0} // terminator
};

static HookEntry_Type SuppressChildHooks[]={
	{"CreateProcessA", (FARPROC)NULL, (FARPROC *)NULL, (FARPROC)extCreateProcessA},
	{0, NULL, 0, 0} // terminator
};

static char *libname = "kernel32.dll";

void HookKernel32(HMODULE module)
{
	HookLibrary(module, Hooks, libname);
	if(dxw.dwFlags3 & BUFFEREDIOFIX) HookLibrary(module, FixIOHooks, libname);
	if(dxw.dwFlags2 & LIMITRESOURCES) HookLibrary(module, LimitHooks, libname);
	if(dxw.dwFlags2 & TIMESTRETCH) HookLibrary(module, TimeHooks, libname);
	if(dxw.dwFlags2 & FAKEVERSION) HookLibrary(module, VersionHooks, libname);
	if(dxw.dwFlags4 & SUPPRESSCHILD) HookLibrary(module, SuppressChildHooks, libname);
}

void HookKernel32Init()
{
	HookLibInit(Hooks);
	HookLibInit(LimitHooks);
	HookLibInit(TimeHooks);
	HookLibInit(VersionHooks);
}

FARPROC Remap_kernel32_ProcAddress(LPCSTR proc, HMODULE hModule)
{
	FARPROC addr;

	if (dxw.dwFlags4 & NOPERFCOUNTER){
		if( !strcmp(proc, "QueryPerformanceCounter") ||
			!strcmp(proc, "QueryPerformanceFrequency")){
				OutTraceDW("GetProcAddress: HIDING proc=%s\n", proc);
			return NULL;
		}
	}
			
	if (addr=RemapLibrary(proc, hModule, Hooks)) return addr;

	if(dxw.dwFlags3 & BUFFEREDIOFIX)
		if (addr=RemapLibrary(proc, hModule, FixIOHooks)) return addr;

	if(dxw.dwFlags2 & LIMITRESOURCES)
		if (addr=RemapLibrary(proc, hModule, LimitHooks)) return addr;

	if(dxw.dwFlags2 & TIMESTRETCH)
		if (addr=RemapLibrary(proc, hModule, TimeHooks)) return addr;

	if(dxw.dwFlags2 & FAKEVERSION)
		if (addr=RemapLibrary(proc, hModule, VersionHooks)) return addr;

	if(dxw.dwFlags4 & SUPPRESSCHILD)
		if (addr=RemapLibrary(proc, hModule, SuppressChildHooks)) return addr;

	//if (addr=RemapLibrary(proc, hModule, SuppressPerfCountersHooks)) return addr;

	return NULL;
}

extern DirectDrawEnumerate_Type pDirectDrawEnumerate;
extern DirectDrawEnumerateEx_Type pDirectDrawEnumerateEx;
extern void HookModule(HMODULE, int);

int WINAPI extIsDebuggerPresent(void)
{
	OutTraceDW("extIsDebuggerPresent: return FALSE\n");
	return FALSE;
}

BOOL WINAPI extGetDiskFreeSpaceA(LPCSTR lpRootPathName, LPDWORD lpSectorsPerCluster, LPDWORD lpBytesPerSector, LPDWORD lpNumberOfFreeClusters, LPDWORD lpTotalNumberOfClusters)
{
	BOOL ret;
	OutTraceDW("GetDiskFreeSpace: RootPathName=\"%s\"\n", lpRootPathName);
	ret=(*pGetDiskFreeSpaceA)(lpRootPathName, lpSectorsPerCluster, lpBytesPerSector, lpNumberOfFreeClusters, lpTotalNumberOfClusters);
	if(!ret) OutTraceE("GetDiskFreeSpace: ERROR err=%d at %d\n", GetLastError(), __LINE__);
	*lpNumberOfFreeClusters = 16000;
	return ret;
}

/* -------------------------------------------------------------------------------

GlobalMemoryStatus: MSDN documents that on modern PCs that have more than DWORD
memory values the GlobalMemoryStatus sets the fields to -1 (0xFFFFFFFF) and you 
should use GlobalMemoryStatusEx instead. 
But in some cases the value is less that DWORD max, but greater that DWORD>>1, that
is the calling application may get a big value and see it as a signed negative
value, as it happened to Nocturne on my PC. That's why it's not adviseable to write: 
if(lpBuffer->dwTotalPhys== -1) ...
but this way:
if ((int)lpBuffer->dwTotalPhys < 0) ...
and also don't set 
BIGENOUGH 0x80000000 // possibly negative!!!
but:
BIGENOUGH 0x20000000 // surely positive !!!

/* ---------------------------------------------------------------------------- */
#define BIGENOUGH 0x20000000

void WINAPI extGlobalMemoryStatus(LPMEMORYSTATUS lpBuffer)
{
	(*pGlobalMemoryStatus)(lpBuffer);
	OutTraceDW("GlobalMemoryStatus: Length=%x MemoryLoad=%x "
		"TotalPhys=%x AvailPhys=%x TotalPageFile=%x AvailPageFile=%x TotalVirtual=%x AvailVirtual=%x\n",
		lpBuffer->dwMemoryLoad, lpBuffer->dwTotalPhys, lpBuffer->dwAvailPhys,
		lpBuffer->dwTotalPageFile, lpBuffer->dwAvailPageFile, lpBuffer->dwTotalVirtual, lpBuffer->dwAvailVirtual);
	if(lpBuffer->dwLength==sizeof(MEMORYSTATUS)){
		if ((int)lpBuffer->dwTotalPhys < 0) lpBuffer->dwTotalPhys = BIGENOUGH;
		if ((int)lpBuffer->dwAvailPhys < 0) lpBuffer->dwAvailPhys = BIGENOUGH;
		if ((int)lpBuffer->dwTotalPageFile < 0) lpBuffer->dwTotalPageFile = BIGENOUGH;
		if ((int)lpBuffer->dwAvailPageFile < 0) lpBuffer->dwAvailPageFile = BIGENOUGH;
		if ((int)lpBuffer->dwTotalVirtual < 0) lpBuffer->dwTotalVirtual = BIGENOUGH;
		if ((int)lpBuffer->dwAvailVirtual < 0) lpBuffer->dwAvailVirtual = BIGENOUGH;
	}
}

/*
From MSDN:
Operating system		Version number	dwMajorVersion	dwMinorVersion	Other
Windows 8				6.2		6		2		OSVERSIONINFOEX.wProductType == VER_NT_WORKSTATION
Windows Server 2012		6.2		6		2		OSVERSIONINFOEX.wProductType != VER_NT_WORKSTATION
Windows 7				6.1		6		1		OSVERSIONINFOEX.wProductType == VER_NT_WORKSTATION
Windows Server 2008 R2	6.1		6		1		OSVERSIONINFOEX.wProductType != VER_NT_WORKSTATION
Windows Server 2008		6.0		6		0		OSVERSIONINFOEX.wProductType != VER_NT_WORKSTATION
Windows Vista			6.0		6		0		OSVERSIONINFOEX.wProductType == VER_NT_WORKSTATION
Windows Server 2003 R2	5.2		5		2		GetSystemMetrics(SM_SERVERR2) != 0
Windows Home Server		5.2		5		2		OSVERSIONINFOEX.wSuiteMask & VER_SUITE_WH_SERVER
Windows Server 2003		5.2		5		2		GetSystemMetrics(SM_SERVERR2) == 0
Windows XP Pro x64 Ed.	5.2		5		2		(OSVERSIONINFOEX.wProductType == VER_NT_WORKSTATION) && (SYSTEM_INFO.wProcessorArchitecture==PROCESSOR_ARCHITECTURE_AMD64)
Windows XP				5.1		5		1		Not applicable
Windows 2000			5.0		5		0		Not applicable
From http://delphi.about.com/cs/adptips2000/a/bltip1100_2.htm 
Windows 95				4.0		4		0
Windows 98/SE"			4.10	4		10		if osVerInfo.szCSDVersion[1] = 'A' then Windows98SE
Windows ME				4.90	4		90
*/

static struct {char bMajor; char bMinor; char *sName;} WinVersions[9]=
{
	{4, 0, "Windows 95"},
	{4,10, "Windows 98/SE"},
	{4,90, "Windows ME"},
	{5, 0, "Windows 2000"},
	{5, 1, "Windows XP"},
	{5, 2, "Windows Server 2003"},
	{6, 0, "Windows Vista"},
	{6, 1, "Windows 7"},
	{6, 2, "Windows 8"}
};

BOOL WINAPI extGetVersionExA(LPOSVERSIONINFOA lpVersionInfo)
{
	BOOL ret;

	ret=(*pGetVersionExA)(lpVersionInfo);
	if(!ret) {
		OutTraceE("GetVersionEx: ERROR err=%d\n", GetLastError());
		return ret;
	}

	OutTraceDW("GetVersionEx: version=%d.%d build=(%d)\n", 
		lpVersionInfo->dwMajorVersion, lpVersionInfo->dwMinorVersion, lpVersionInfo->dwBuildNumber);

	if(dxw.dwFlags2 & FAKEVERSION) {
		// fake Win XP build 0
		lpVersionInfo->dwMajorVersion = WinVersions[dxw.FakeVersionId].bMajor;
		lpVersionInfo->dwMinorVersion = WinVersions[dxw.FakeVersionId].bMinor;
		lpVersionInfo->dwBuildNumber = 0;
		OutTraceDW("GetVersionEx: FIXED version=%d.%d build=(%d) os=\"%s\"\n", 
			lpVersionInfo->dwMajorVersion, lpVersionInfo->dwMinorVersion, lpVersionInfo->dwBuildNumber,
			WinVersions[dxw.FakeVersionId].sName);
	}
	return TRUE;
}

BOOL WINAPI extGetVersionExW(LPOSVERSIONINFOW lpVersionInfo)
{
	BOOL ret;

	ret=(*pGetVersionExW)(lpVersionInfo);
	if(!ret) {
		OutTraceE("GetVersionEx: ERROR err=%d\n", GetLastError());
		return ret;
	}

	OutTraceDW("GetVersionEx: version=%d.%d build=(%d)\n", 
		lpVersionInfo->dwMajorVersion, lpVersionInfo->dwMinorVersion, lpVersionInfo->dwBuildNumber);

	if(dxw.dwFlags2 & FAKEVERSION) {
		// fake Win XP build 0
		lpVersionInfo->dwMajorVersion = WinVersions[dxw.FakeVersionId].bMajor;
		lpVersionInfo->dwMinorVersion = WinVersions[dxw.FakeVersionId].bMinor;
		lpVersionInfo->dwBuildNumber = 0;
		OutTraceDW("GetVersionEx: FIXED version=%d.%d build=(%d) os=\"%ls\"\n", 
			lpVersionInfo->dwMajorVersion, lpVersionInfo->dwMinorVersion, lpVersionInfo->dwBuildNumber,
			WinVersions[dxw.FakeVersionId].sName);
	}
	return TRUE;
}

DWORD WINAPI extGetVersion(void)
{
    DWORD dwVersion; 
    DWORD dwMajorVersion;
    DWORD dwMinorVersion; 
    DWORD dwBuild = 0;

    dwVersion = (*pGetVersion)();
 
    // Get the Windows version.

    dwMajorVersion = (DWORD)(LOBYTE(LOWORD(dwVersion)));
    dwMinorVersion = (DWORD)(HIBYTE(LOWORD(dwVersion)));

    // Get the build number.

    if (dwVersion < 0x80000000)              
        dwBuild = (DWORD)(HIWORD(dwVersion));

	OutTraceDW("GetVersion: version=%d.%d build=(%d)\n", dwMajorVersion, dwMinorVersion, dwBuild);

	if(dxw.dwFlags2 & FAKEVERSION) {
		dwVersion = WinVersions[dxw.FakeVersionId].bMajor | (WinVersions[dxw.FakeVersionId].bMinor << 8);
		dwMajorVersion = (DWORD)(LOBYTE(LOWORD(dwVersion)));
	    dwMinorVersion = (DWORD)(HIBYTE(LOWORD(dwVersion)));
		dwBuild = (DWORD)(HIWORD(dwVersion));
		OutTraceDW("GetVersion: FIXED version=%d.%d build=(%d) os=\"%s\"\n", 
			dwMajorVersion, dwMinorVersion, dwBuild, WinVersions[dxw.FakeVersionId].sName);
	}

	return dwVersion;
}

/* -------------------------------------------------------------------------------

time related APIs

/* ---------------------------------------------------------------------------- */

DWORD WINAPI extGetTickCount(void)
{
	return dxw.GetTickCount();
}

void WINAPI extGetSystemTime(LPSYSTEMTIME lpSystemTime)
{
	dxw.GetSystemTime(lpSystemTime);
	if (IsDebug) OutTrace("GetSystemTime: %02d:%02d:%02d.%03d\n", 
		lpSystemTime->wHour, lpSystemTime->wMinute, lpSystemTime->wSecond, lpSystemTime->wMilliseconds);
}

void WINAPI extGetLocalTime(LPSYSTEMTIME lpLocalTime)
{
	SYSTEMTIME SystemTime;
	dxw.GetSystemTime(&SystemTime);
	SystemTimeToTzSpecificLocalTime(NULL, &SystemTime, lpLocalTime);
	if (IsDebug) OutTrace("GetLocalTime: %02d:%02d:%02d.%03d\n", 
		lpLocalTime->wHour, lpLocalTime->wMinute, lpLocalTime->wSecond, lpLocalTime->wMilliseconds);
}

VOID WINAPI extSleep(DWORD dwMilliseconds)
{
	DWORD dwNewDelay;
	dwNewDelay=dwMilliseconds;
	if (dwMilliseconds!=INFINITE && dwMilliseconds!=0){
		dwNewDelay = dxw.StretchTime(dwMilliseconds);
		if (dwNewDelay==0){ // oh oh! troubles...
			if (dxw.TimeShift > 0) dwNewDelay=1; // minimum allowed...
			else dwNewDelay = INFINITE-1; // maximum allowed !!!
		}
	}
	if (IsDebug) OutTrace("Sleep: msec=%d->%d timeshift=%d\n", dwMilliseconds, dwNewDelay, dxw.TimeShift);
	(*pSleep)(dwNewDelay);
}

DWORD WINAPI extSleepEx(DWORD dwMilliseconds, BOOL bAlertable)
{
	DWORD dwNewDelay;
	dwNewDelay=dwMilliseconds;
	if (dwMilliseconds!=INFINITE && dwMilliseconds!=0){
		dwNewDelay = dxw.StretchTime(dwMilliseconds);
		if (dwNewDelay==0){ // oh oh! troubles...
			if (dxw.TimeShift > 0) dwNewDelay=1; // minimum allowed...
			else dwNewDelay = INFINITE-1; // maximum allowed !!!
		}
	}
	if (IsDebug) OutTrace("SleepEx: msec=%d->%d alertable=%x, timeshift=%d\n", dwMilliseconds, dwNewDelay, bAlertable, dxw.TimeShift);
	return (*pSleepEx)(dwNewDelay, bAlertable);
}

void WINAPI extGetSystemTimeAsFileTime(LPFILETIME lpSystemTimeAsFileTime)
{
	if (IsDebug) OutTrace("GetSystemTimeAsFileTime\n");
	dxw.GetSystemTimeAsFileTime(lpSystemTimeAsFileTime);
}

BOOL WINAPI extQueryPerformanceCounter(LARGE_INTEGER *lpPerformanceCount)
{
	BOOL ret;

	if(dxw.dwFlags4 & NOPERFCOUNTER){
		ret=0;
		(*lpPerformanceCount).QuadPart = 0;
	}
	else{
		LARGE_INTEGER CurrentInCount;
		ret=(*pQueryPerformanceCounter)(&CurrentInCount);
		*lpPerformanceCount = dxw.StretchLargeCounter(CurrentInCount);
	}

	OutTraceB("QueryPerformanceCounter: ret=%x Count=[%x-%x]\n", ret, lpPerformanceCount->HighPart, lpPerformanceCount->LowPart);
	return ret;
}

BOOL WINAPI extQueryPerformanceFrequency(LARGE_INTEGER *lpPerformanceFrequency)
{
	BOOL ret;
	if(dxw.dwFlags4 & NOPERFCOUNTER){
		LARGE_INTEGER myPerfFrequency;
		myPerfFrequency.LowPart = 0L;
		myPerfFrequency.HighPart = 0L;
		*lpPerformanceFrequency=myPerfFrequency;
		ret = 0;
	}
	else
		ret=(*pQueryPerformanceFrequency)(lpPerformanceFrequency);
	OutTraceDW("QueryPerformanceFrequency: ret=%x Frequency=%x-%x\n", ret, lpPerformanceFrequency->HighPart, lpPerformanceFrequency->LowPart);
	return ret;
}

/* -------------------------------------------------------------------------------

LoadLibrary (hooking) related APIs

/* ---------------------------------------------------------------------------- */

HMODULE SysLibs[SYSLIBIDX_MAX];

HMODULE WINAPI LoadLibraryExWrapper(LPCTSTR lpFileName, HANDLE hFile, DWORD dwFlags, char *api)
{
	HMODULE libhandle;
	int idx;
	
	libhandle=(*pLoadLibraryExA)(lpFileName, hFile, dwFlags);
	OutTraceDW("%s: FileName=%s hFile=%x Flags=%x(%s) hmodule=%x\n", api, lpFileName, hFile, dwFlags, ExplainLoadLibFlags(dwFlags), libhandle);
	if(!libhandle){
		OutTraceE("%s: ERROR FileName=%s err=%d\n", api, lpFileName, GetLastError());
		return libhandle;
	}

	// when loaded with LOAD_LIBRARY_AS_DATAFILE or LOAD_LIBRARY_AS_DATAFILE_EXCLUSIVE flags, 
	// there's no symbol map, then itìs no possible to hook function calls.
	if(dwFlags & (LOAD_LIBRARY_AS_DATAFILE_EXCLUSIVE|LOAD_LIBRARY_AS_DATAFILE)) return libhandle;

	idx=dxw.GetDLLIndex((char *)lpFileName);
	if(idx != -1) {
		OutTraceDW("%s: push idx=%x library=%s hdl=%x\n", api, idx, lpFileName, libhandle);
		SysLibs[idx]=libhandle;
	}
	// handle custom OpenGL library
	if(!lstrcmpi(lpFileName,dxw.CustomOpenGLLib)){
		idx=SYSLIBIDX_OPENGL;
		SysLibs[idx]=libhandle;
	}
	if (idx == -1) {
		OutTraceDW("%s: hooking lib=\"%s\" handle=%x\n", api, lpFileName, libhandle);
		HookModule(libhandle, 0);
	}
#if 0
	switch(idx){
		case SYSLIBIDX_DIRECTDRAW: HookDirectDraw(libhandle, 0); break;
		case SYSLIBIDX_DIRECT3D8: HookDirect3D(libhandle, 8); break;
		case SYSLIBIDX_DIRECT3D9: HookDirect3D(libhandle, 9); break;
	}
#endif
	return libhandle;
}

HMODULE WINAPI extLoadLibraryA(LPCTSTR lpFileName)
{
	return LoadLibraryExWrapper(lpFileName, NULL, 0, "LoadLibraryA");
}

HMODULE WINAPI extLoadLibraryW(LPCWSTR lpFileName)
{
	char sFileName[256+1];
	wcstombs_s(NULL, sFileName, lpFileName, 80);
	return LoadLibraryExWrapper(sFileName, NULL, 0, "LoadLibraryW");;
}

HMODULE WINAPI extLoadLibraryExA(LPCTSTR lpFileName, HANDLE hFile, DWORD dwFlags)
{
	return LoadLibraryExWrapper(lpFileName, hFile, dwFlags, "LoadLibraryExA");
}

HMODULE WINAPI extLoadLibraryExW(LPCWSTR lpFileName, HANDLE hFile, DWORD dwFlags)
{
	char sFileName[256+1];
	wcstombs_s(NULL, sFileName, lpFileName, 80);
	return LoadLibraryExWrapper(sFileName, hFile, dwFlags, "LoadLibraryExW");;
}

extern DirectDrawCreate_Type pDirectDrawCreate;
extern DirectDrawCreateEx_Type pDirectDrawCreateEx;
extern HRESULT WINAPI extDirectDrawCreate(GUID FAR *, LPDIRECTDRAW FAR *, IUnknown FAR *);
extern HRESULT WINAPI extDirectDrawCreateEx(GUID FAR *, LPDIRECTDRAW FAR *, REFIID, IUnknown FAR *);
extern GetProcAddress_Type pGetProcAddress;
//extern HRESULT STDAPICALLTYPE extCoCreateInstance(REFCLSID, LPUNKNOWN, DWORD, REFIID, LPVOID FAR*);

FARPROC WINAPI extGetProcAddress(HMODULE hModule, LPCSTR proc)
{
	FARPROC ret;
	int idx;

	// WARNING: seems to be called with bad LPCSTR value....
	// from MSDN:
	// The function or variable name, or the function's ordinal value. 
	// If this parameter is an ordinal value, it must be in the low-order word; 
	// the high-order word must be zero.

	OutTraceDW("GetProcAddress: hModule=%x proc=%s\n", hModule, ProcToString(proc));

	for(idx=0; idx<SYSLIBIDX_MAX; idx++){
		if(SysLibs[idx]==hModule) break;
	}

	// to do: the else condition: the program COULD load addresses by ordinal value ... done ??
	// to do: CoCreateInstanceEx
	if((DWORD)proc & 0xFFFF0000){
		FARPROC remap;
		switch(idx){
		case SYSLIBIDX_DIRECTDRAW:
			if (remap=Remap_ddraw_ProcAddress(proc, hModule)) return remap;
			break;
		case SYSLIBIDX_USER32:
			if (remap=Remap_user32_ProcAddress(proc, hModule)) return remap;
			break;
		case SYSLIBIDX_KERNEL32:
			if (remap=Remap_kernel32_ProcAddress(proc, hModule)) return remap;
			break;
		case SYSLIBIDX_IMELIB:
			if (remap=Remap_ImeLib_ProcAddress(proc, hModule)) return remap;
			break;
		case SYSLIBIDX_WINMM:
			if (remap=Remap_WinMM_ProcAddress(proc, hModule)) return remap;
			break;
		case SYSLIBIDX_OLE32: 
			if (remap=Remap_ole32_ProcAddress(proc, hModule)) return remap;
			break;
		case SYSLIBIDX_DIRECT3D8:
			if (remap=Remap_d3d8_ProcAddress(proc, hModule)) return remap;
			break;
		case SYSLIBIDX_DIRECT3D9:
			if (remap=Remap_d3d9_ProcAddress(proc, hModule)) return remap;
			break;
		case SYSLIBIDX_DIRECT3D10:
			if (remap=Remap_d3d10_ProcAddress(proc, hModule)) return remap;
			break;
		case SYSLIBIDX_DIRECT3D10_1:
			if (remap=Remap_d3d10_1_ProcAddress(proc, hModule)) return remap;
			break;
		case SYSLIBIDX_DIRECT3D11:
			if (remap=Remap_d3d11_ProcAddress(proc, hModule)) return remap;
			break;
		case SYSLIBIDX_OPENGL:
			if (remap=Remap_gl_ProcAddress(proc, hModule)) return remap;
			break;
//		case SYSLIBIDX_GLIDE:
//		case SYSLIBIDX_GLIDE2X:
//		case SYSLIBIDX_GLIDE3X:
//			if (remap=Remap_Glide_ProcAddress(proc, hModule)) return remap;
//			break;
		case SYSLIBIDX_MSVFW:
			if (remap=Remap_vfw_ProcAddress(proc, hModule)) return remap;
			break;
		case SYSLIBIDX_SMACK:
			if (remap=Remap_smack_ProcAddress(proc, hModule)) return remap;
			break;
		case SYSLIBIDX_WINTRUST:
			if (remap=Remap_trust_ProcAddress(proc, hModule)) return remap;
			break;
		case SYSLIBIDX_ADVAPI32:
			if (remap=Remap_AdvApi32_ProcAddress(proc, hModule)) return remap;
			break;
		case SYSLIBIDX_DIRECT3D:
			if (remap=Remap_d3d7_ProcAddress(proc, hModule)) return remap;
			break;
		case SYSLIBIDX_DIRECT3D700:
			if (remap=Remap_d3d7_ProcAddress(proc, hModule)) return remap;
			break;
		default:
			break;
		}
	}
	else {
		switch(idx){
		case SYSLIBIDX_DIRECTDRAW:
			switch((DWORD)proc){
			case 0x0008: // DirectDrawCreate
				pDirectDrawCreate=(DirectDrawCreate_Type)(*pGetProcAddress)(hModule, proc);
				OutTraceDW("GetProcAddress: hooking proc=%s at addr=%x\n", ProcToString(proc), pDirectDrawCreate);
				return (FARPROC)extDirectDrawCreate;
				break;
			case 0x000A: // DirectDrawCreateEx
				pDirectDrawCreateEx=(DirectDrawCreateEx_Type)(*pGetProcAddress)(hModule, proc);
				OutTraceDW("GetProcAddress: hooking proc=%s at addr=%x\n", ProcToString(proc), pDirectDrawCreateEx);
				return (FARPROC)extDirectDrawCreateEx;
				break;
			case 0x000B: // DirectDrawEnumerateA
				pDirectDrawEnumerate=(DirectDrawEnumerate_Type)(*pGetProcAddress)(hModule, proc);
				OutTraceP("GetProcAddress: hooking proc=%s at addr=%x\n", proc, pDirectDrawEnumerate);
				return (FARPROC)extDirectDrawEnumerateProxy;
				break;
			case 0x000C: // DirectDrawEnumerateExA
				pDirectDrawEnumerateEx=(DirectDrawEnumerateEx_Type)(*pGetProcAddress)(hModule, proc);
				OutTraceP("GetProcAddress: hooking proc=%s at addr=%x\n", proc, pDirectDrawEnumerateEx);
				return (FARPROC)extDirectDrawEnumerateExProxy;
				break;
			}
			break;
		case SYSLIBIDX_USER32:
			if ((DWORD)proc == 0x0020){ // ChangeDisplaySettingsA
				pChangeDisplaySettingsA=(ChangeDisplaySettingsA_Type)(*pGetProcAddress)(hModule, proc);
				OutTraceDW("GetProcAddress: hooking proc=%s at addr=%x\n", ProcToString(proc), pChangeDisplaySettingsA);
				return (FARPROC)extChangeDisplaySettingsA;
			}
			break;
#ifndef ANTICHEATING
		case SYSLIBIDX_KERNEL32:
			if ((DWORD)proc == 0x022D){ // "IsDebuggerPresent"
				OutTraceDW("GetProcAddress: hooking proc=%s at addr=%x\n", ProcToString(proc), extIsDebuggerPresent);
				return (FARPROC)extIsDebuggerPresent;
			}
#endif
		case SYSLIBIDX_OLE32:
			if ((DWORD)proc == 0x0011){ // "CoCreateInstance"
				pCoCreateInstance=(CoCreateInstance_Type)(*pGetProcAddress)(hModule, proc);
				OutTraceDW("GetProcAddress: hooking proc=%s at addr=%x\n", ProcToString(proc), pCoCreateInstance);
				return (FARPROC)extCoCreateInstance;
			}
			break;
		}
	}

	ret=(*pGetProcAddress)(hModule, proc);
	OutTraceDW("GetProcAddress: ret=%x\n", ret);
	return ret;
}

/* -------------------------------------------------------------------------------

I/O related APIs

/* ---------------------------------------------------------------------------- */

UINT WINAPI extGetDriveType(LPCTSTR lpRootPathName)
{
	OutTraceDW("GetDriveType: path=\"%s\"\n", lpRootPathName);
	if (dxw.dwFlags3 & CDROMDRIVETYPE) return DRIVE_CDROM;
	return (*pGetDriveType)(lpRootPathName);
}

BOOL WINAPI extReadFile(HANDLE hFile, LPVOID lpBuffer, DWORD nNumberOfBytesToRead, LPDWORD lpNumberOfBytesRead, LPOVERLAPPED lpOverlapped)
{
	BOOL ret;
	static char *IOBuffer=NULL;


	if(IsTraceDW){
		OutTrace("ReadFile: hFile=%x Buffer=%x BytesToRead=%d Overlapped=%x", hFile, lpBuffer, nNumberOfBytesToRead, lpOverlapped);
		if(lpOverlapped) OutTrace("->(Offset=0x%x OffsetHigh=0x%x)", lpOverlapped->Offset, lpOverlapped->OffsetHigh);
		OutTrace("\n");
	}

	ret = (*pReadFile)(hFile, lpBuffer, nNumberOfBytesToRead, lpNumberOfBytesRead, lpOverlapped);
	if(IsTraceDW){
		if(ret){
			OutTrace("ReadFile: NumberOfBytesRead=%d\n", *lpNumberOfBytesRead);
			OutTrace("ReadFile: ");
			for(unsigned int i=0; i<*lpNumberOfBytesRead; i++) OutTrace("%02X,", *((unsigned char *)lpBuffer+i));
			OutTrace("\n");
		}
		else
			OutTrace("ReadFile: ERROR err=%d\n", GetLastError());
	}
	return ret;
}

HANDLE WINAPI extCreateFile(LPCTSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, 
							LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition,
							DWORD dwFlagsAndAttributes, HANDLE hTemplateFile)
{
	HANDLE ret;
	OutTraceDW("CreateFile: FileName=%s DesiredAccess=%x SharedMode=%x Disposition=%x Flags=%x\n", 
		lpFileName, dwDesiredAccess, dwShareMode, dwCreationDisposition, dwFlagsAndAttributes);

	// just proxy
	if(!(dxw.dwFlags3 & BUFFEREDIOFIX))
		return (*pCreateFile)(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);

	if((dxw.dwFlags3 & BUFFEREDIOFIX) && (dwFlagsAndAttributes & FILE_FLAG_NO_BUFFERING)){
		OutTraceDW("CreateFile: suppress FILE_FLAG_NO_BUFFERING\n"); 
		dwFlagsAndAttributes &= ~FILE_FLAG_NO_BUFFERING;
	}

	ret=(*pCreateFile)(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
	if(IsTraceDW){
		if(ret && (ret != (HANDLE)INVALID_SET_FILE_POINTER)) 
			OutTrace("CreateFile: ret=%x\n", ret);
		else
			OutTrace("CreateFile ERROR: err=%d\n", GetLastError());
	}
	return ret;
} 

BOOL WINAPI extCloseHandle(HANDLE hObject)
{
	OutTrace("CloseHandle: hFile=%x\n", hObject);

	return (*pCloseHandle)(hObject);
}

DWORD WINAPI extSetFilePointer(HANDLE hFile, LONG lDistanceToMove, PLONG lpDistanceToMoveHigh, DWORD dwMoveMethod)
{
	OutTrace("SetFilePointer: hFile=%x DistanceToMove=0x%lx DistanceToMoveHigh=0x%x MoveMethod=%x\n", 
		hFile, lDistanceToMove, lpDistanceToMoveHigh, dwMoveMethod);

	return (*pSetFilePointer)(hFile, lDistanceToMove, lpDistanceToMoveHigh, dwMoveMethod);
}

BOOL WINAPI extCreateProcessA(
	LPCTSTR lpApplicationName, 
	LPTSTR lpCommandLine, 
	LPSECURITY_ATTRIBUTES lpProcessAttributes,
	LPSECURITY_ATTRIBUTES lpThreadAttributes,
	BOOL bInheritHandles,
	DWORD dwCreationFlags,
	LPVOID lpEnvironment,
	LPCTSTR lpCurrentDirectory,
	LPSTARTUPINFO lpStartupInfo,
	LPPROCESS_INFORMATION lpProcessInformation
)
{
	OutTraceDW("CreateProcess: SUPPRESS ApplicationName=%s CommandLine=\"%s\"\n", lpApplicationName, lpCommandLine);
	return TRUE;
}
