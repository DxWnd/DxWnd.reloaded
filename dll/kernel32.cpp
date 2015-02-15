#include "dxwnd.h"
#include "dxwcore.hpp"
#include "syslibs.h"
#include "dxhook.h"
#include "dxhelper.h"
#include "hddraw.h"
#include "ddproxy.h"
#include "stdio.h"

//#undef IsTraceDW
//#define IsTraceDW TRUE
#define LOCKINJECTIONTHREADS

BOOL WINAPI extCheckRemoteDebuggerPresent(HANDLE, PBOOL);

typedef BOOL (WINAPI *CreateProcessA_Type)(LPCTSTR, LPTSTR, LPSECURITY_ATTRIBUTES, LPSECURITY_ATTRIBUTES, 
										   BOOL, DWORD, LPVOID, LPCTSTR, LPSTARTUPINFO, LPPROCESS_INFORMATION);
CreateProcessA_Type pCreateProcessA = NULL;

// v2.02.96: the GetSystemInfo API is NOT hot patchable on Win7. This can cause problems because it can't be hooked by simply
// enabling hot patch. A solution is making all LiadLibrary* calls hot patchable, so that when loading the module, the call
// can be hooked by the IAT lookup. This fixes a problem after movie playing in Wind Fantasy SP.

static HookEntry_Type Hooks[]={
	{HOOK_IAT_CANDIDATE, "IsDebuggerPresent", (FARPROC)NULL, (FARPROC *)NULL, (FARPROC)extIsDebuggerPresent},
	{HOOK_IAT_CANDIDATE, "CheckRemoteDebuggerPresent", (FARPROC)NULL, (FARPROC *)NULL, (FARPROC)extCheckRemoteDebuggerPresent},
	{HOOK_IAT_CANDIDATE, "GetProcAddress", (FARPROC)GetProcAddress, (FARPROC *)&pGetProcAddress, (FARPROC)extGetProcAddress},
	{HOOK_HOT_CANDIDATE, "LoadLibraryA", (FARPROC)LoadLibraryA, (FARPROC *)&pLoadLibraryA, (FARPROC)extLoadLibraryA},
	{HOOK_HOT_CANDIDATE, "LoadLibraryExA", (FARPROC)LoadLibraryExA, (FARPROC *)&pLoadLibraryExA, (FARPROC)extLoadLibraryExA},
	{HOOK_HOT_CANDIDATE, "LoadLibraryW", (FARPROC)LoadLibraryW, (FARPROC *)&pLoadLibraryW, (FARPROC)extLoadLibraryW},
	{HOOK_HOT_CANDIDATE, "LoadLibraryExW", (FARPROC)LoadLibraryExW, (FARPROC *)&pLoadLibraryExW, (FARPROC)extLoadLibraryExW},
	{HOOK_IAT_CANDIDATE, "GetDriveTypeA", (FARPROC)NULL, (FARPROC *)&pGetDriveType, (FARPROC)extGetDriveType},
	{HOOK_IAT_CANDIDATE, "GetLogicalDrives", (FARPROC)NULL, (FARPROC *)&pGetLogicalDrives, (FARPROC)extGetLogicalDrives},
	{HOOK_IAT_CANDIDATE, "GetTempFileNameA", (FARPROC)GetTempFileNameA, (FARPROC *)&pGetTempFileName, (FARPROC)extGetTempFileName},
	{HOOK_IAT_CANDIDATE, "CreateProcessA", (FARPROC)NULL, (FARPROC *)&pCreateProcessA, (FARPROC)extCreateProcessA},
	{HOOK_IAT_CANDIDATE, 0, NULL, 0, 0} // terminator
};

static HookEntry_Type FixIOHooks[]={
	{HOOK_IAT_CANDIDATE, "ReadFile", (FARPROC)NULL, (FARPROC *)&pReadFile, (FARPROC)extReadFile},
	{HOOK_IAT_CANDIDATE, "CreateFileA", (FARPROC)NULL, (FARPROC *)&pCreateFile, (FARPROC)extCreateFile},
	{HOOK_IAT_CANDIDATE, "SetFilePointer", (FARPROC)NULL, (FARPROC *)&pSetFilePointer, (FARPROC)extSetFilePointer},
	{HOOK_IAT_CANDIDATE, "CloseHandle", (FARPROC)NULL, (FARPROC *)&pCloseHandle, (FARPROC)extCloseHandle},
	{HOOK_IAT_CANDIDATE, 0, NULL, 0, 0} // terminator
};

static HookEntry_Type LimitHooks[]={
	{HOOK_HOT_CANDIDATE, "GetDiskFreeSpaceA", (FARPROC)GetDiskFreeSpaceA, (FARPROC *)&pGetDiskFreeSpaceA, (FARPROC)extGetDiskFreeSpaceA},
	{HOOK_HOT_CANDIDATE, "GlobalMemoryStatus", (FARPROC)GlobalMemoryStatus, (FARPROC *)&pGlobalMemoryStatus, (FARPROC)extGlobalMemoryStatus},
	{HOOK_IAT_CANDIDATE, 0, NULL, 0, 0} // terminator
};

static HookEntry_Type TimeHooks[]={
	{HOOK_HOT_CANDIDATE, "GetTickCount", (FARPROC)GetTickCount, (FARPROC *)&pGetTickCount, (FARPROC)extGetTickCount},
	{HOOK_HOT_CANDIDATE, "GetLocalTime", (FARPROC)GetLocalTime, (FARPROC *)&pGetLocalTime, (FARPROC)extGetLocalTime},
	{HOOK_HOT_CANDIDATE, "GetSystemTime", (FARPROC)GetSystemTime, (FARPROC *)&pGetSystemTime, (FARPROC)extGetSystemTime},
	{HOOK_HOT_CANDIDATE, "GetSystemTimeAsFileTime", (FARPROC)GetSystemTimeAsFileTime, (FARPROC *)&pGetSystemTimeAsFileTime, (FARPROC)extGetSystemTimeAsFileTime},
	{HOOK_HOT_CANDIDATE, "Sleep", (FARPROC)Sleep, (FARPROC *)&pSleep, (FARPROC)extSleep},
	{HOOK_HOT_CANDIDATE, "SleepEx", (FARPROC)SleepEx, (FARPROC *)&pSleepEx, (FARPROC)extSleepEx},
	{HOOK_HOT_CANDIDATE, "QueryPerformanceCounter", (FARPROC)QueryPerformanceCounter, (FARPROC *)&pQueryPerformanceCounter, (FARPROC)extQueryPerformanceCounter},
	{HOOK_HOT_CANDIDATE, "QueryPerformanceFrequency", (FARPROC)QueryPerformanceFrequency, (FARPROC *)&pQueryPerformanceFrequency, (FARPROC)extQueryPerformanceFrequency},
	{HOOK_IAT_CANDIDATE, 0, NULL, 0, 0} // terminator
};

static HookEntry_Type VersionHooks[]={
	{HOOK_HOT_CANDIDATE, "GetVersion", (FARPROC)GetVersion, (FARPROC *)&pGetVersion, (FARPROC)extGetVersion},
	{HOOK_HOT_CANDIDATE, "GetVersionExA", (FARPROC)GetVersionExA, (FARPROC *)&pGetVersionExA, (FARPROC)extGetVersionExA},
	{HOOK_HOT_CANDIDATE, "GetVersionExW", (FARPROC)GetVersionExW, (FARPROC *)&pGetVersionExW, (FARPROC)extGetVersionExW},
	{HOOK_IAT_CANDIDATE, 0, NULL, 0, 0} // terminator
};

static char *libname = "kernel32.dll";

void HookKernel32(HMODULE module)
{
	HookLibrary(module, Hooks, libname);
	if(dxw.dwFlags3 & BUFFEREDIOFIX) HookLibrary(module, FixIOHooks, libname);
	if(dxw.dwFlags2 & LIMITRESOURCES) HookLibrary(module, LimitHooks, libname);
	if(dxw.dwFlags2 & TIMESTRETCH) HookLibrary(module, TimeHooks, libname);
	if(dxw.dwFlags2 & FAKEVERSION) HookLibrary(module, VersionHooks, libname);
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
BIGENOUGH 0x60000000 // surely positive !!!

v2.03.08: the "Jeff Gordon XS Racing demo" adds the dwAvailPhys to the dwAvailPageFile
value, so that the sum is negative. To avoid this, all available memory values are
divided by 2 (HALFBIG macro).
/* ---------------------------------------------------------------------------- */

#define BIGENOUGH 0x60000000
#define HALFBIG (BIGENOUGH >> 1)
#define TOOBIG	  0xFFFFFFFF

void WINAPI extGlobalMemoryStatus(LPMEMORYSTATUS lpBuffer)
{
	(*pGlobalMemoryStatus)(lpBuffer);
	OutTraceDW("GlobalMemoryStatus: Length=%d MemoryLoad=%d%c "
		"TotalPhys=%x AvailPhys=%x TotalPageFile=%x AvailPageFile=%x TotalVirtual=%x AvailVirtual=%x\n",
		lpBuffer->dwLength, lpBuffer->dwMemoryLoad, '%', lpBuffer->dwTotalPhys, lpBuffer->dwAvailPhys,
		lpBuffer->dwTotalPageFile, lpBuffer->dwAvailPageFile, lpBuffer->dwTotalVirtual, lpBuffer->dwAvailVirtual);
	if(lpBuffer->dwLength==sizeof(MEMORYSTATUS)){
		MEMORYSTATUS PrevMemoryStatus;
		memcpy(&PrevMemoryStatus, lpBuffer, sizeof(MEMORYSTATUS));
		if (((int)lpBuffer->dwTotalPhys < 0) || ((int)lpBuffer->dwTotalPhys > BIGENOUGH)) lpBuffer->dwTotalPhys = BIGENOUGH;
		if (((int)lpBuffer->dwAvailPhys < 0) || ((int)lpBuffer->dwAvailPhys > HALFBIG)) lpBuffer->dwAvailPhys = HALFBIG;
		if (((int)lpBuffer->dwTotalPageFile < 0) || ((int)lpBuffer->dwTotalPageFile > BIGENOUGH)) lpBuffer->dwTotalPageFile = BIGENOUGH;
		if (((int)lpBuffer->dwAvailPageFile < 0) || ((int)lpBuffer->dwAvailPageFile > HALFBIG)) lpBuffer->dwAvailPageFile = HALFBIG;
		if (((int)lpBuffer->dwTotalVirtual < 0) || ((int)lpBuffer->dwTotalVirtual > BIGENOUGH)) lpBuffer->dwTotalVirtual = BIGENOUGH;
		if (((int)lpBuffer->dwAvailVirtual < 0) || ((int)lpBuffer->dwAvailVirtual > HALFBIG)) lpBuffer->dwAvailVirtual = HALFBIG;
		if(dxw.dwFlags5 & STRESSRESOURCES){
			lpBuffer->dwTotalPhys = TOOBIG;
			lpBuffer->dwAvailPhys = TOOBIG;
			lpBuffer->dwTotalPageFile = TOOBIG;
			lpBuffer->dwAvailPageFile = TOOBIG;
			lpBuffer->dwTotalVirtual = TOOBIG;
			lpBuffer->dwAvailVirtual = TOOBIG;
		}
		if(memcmp(&PrevMemoryStatus, lpBuffer, sizeof(MEMORYSTATUS)))
			OutTraceDW("GlobalMemoryStatus UPDATED: Length=%d MemoryLoad=%d%c "
				"TotalPhys=%x AvailPhys=%x TotalPageFile=%x AvailPageFile=%x TotalVirtual=%x AvailVirtual=%x\n",
				lpBuffer->dwLength, lpBuffer->dwMemoryLoad, '%', lpBuffer->dwTotalPhys, lpBuffer->dwAvailPhys,
				lpBuffer->dwTotalPageFile, lpBuffer->dwAvailPageFile, lpBuffer->dwTotalVirtual, lpBuffer->dwAvailVirtual);
	}
	else{
		OutTraceDW("GlobalMemoryStatus WARNING: Length=%d sizeof(LPMEMORYSTATUS)=%d\n", lpBuffer->dwLength, sizeof(LPMEMORYSTATUS));
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

/*
Differences with older Windows versions
This function is provided even with older Windows versions with some significant differences than stated above:
The high order bit determins if it's NT based (NT, 2000, XP and newer) or not (Win 3.1, 95, 98, ME)
The remaining bits of the high order word specify the build number only on NT based Windows verions.

From older MSDN:
To distinguish between operating system platforms, use the high order bit and the low order byte, as shown in the following table:

Windows NT
    High order bit: 0
    Low order byte (major version number): 3 or 4
Windows 95 and Windows 98
    High order bit: 1
    Low order byte (major version number): 4
Win32s with Windows 3.1
    High order bit: 1
    Low order byte (major version number): 3

For Windows NT and Win32s, the remaining bits in the high order word specify the build number.
For Windows 95 and Windows 98, the remaining bits of the high order word are reserved.
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
		OutTraceE("GetVersionExA: ERROR err=%d\n", GetLastError());
		return ret;
	}

	OutTraceDW("GetVersionExA: version=%d.%d build=(%d)\n", 
		lpVersionInfo->dwMajorVersion, lpVersionInfo->dwMinorVersion, lpVersionInfo->dwBuildNumber);

	if(dxw.dwFlags2 & FAKEVERSION) {
		// fake Win XP build 0
		lpVersionInfo->dwMajorVersion = WinVersions[dxw.FakeVersionId].bMajor;
		lpVersionInfo->dwMinorVersion = WinVersions[dxw.FakeVersionId].bMinor;
		lpVersionInfo->dwBuildNumber = 0;
		OutTraceDW("GetVersionExA: FIXED version=%d.%d build=(%d) os=\"%s\"\n", 
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
		OutTraceE("GetVersionExW: ERROR err=%d\n", GetLastError());
		return ret;
	}

	OutTraceDW("GetVersionExW: version=%d.%d build=(%d)\n", 
		lpVersionInfo->dwMajorVersion, lpVersionInfo->dwMinorVersion, lpVersionInfo->dwBuildNumber);

	if(dxw.dwFlags2 & FAKEVERSION) {
		// fake Win XP build 0
		lpVersionInfo->dwMajorVersion = WinVersions[dxw.FakeVersionId].bMajor;
		lpVersionInfo->dwMinorVersion = WinVersions[dxw.FakeVersionId].bMinor;
		lpVersionInfo->dwBuildNumber = 0;
		OutTraceDW("GetVersionExW: FIXED version=%d.%d build=(%d) os=\"%ls\"\n", 
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
		if(WinVersions[dxw.FakeVersionId].bMajor == 4) dwVersion |= 0x80000000; // v2.03.11: fixes "Warhead"
		OutTraceDW("GetVersion: FIXED version=%x: Win%d.%d build=(%d) os=\"%s\"\n", 
			dwVersion, dwMajorVersion, dwMinorVersion, dwBuild, WinVersions[dxw.FakeVersionId].sName);
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
	if ((dwMilliseconds!=INFINITE) && (dwMilliseconds!=0)){
		dwNewDelay = dxw.StretchTime(dwMilliseconds);
		if (dwNewDelay==0) dwNewDelay=1; // minimum allowed...
	}
	if (IsDebug) OutTrace("Sleep: msec=%d->%d timeshift=%d\n", dwMilliseconds, dwNewDelay, dxw.TimeShift);
	(*pSleep)(dwNewDelay);
}

DWORD WINAPI extSleepEx(DWORD dwMilliseconds, BOOL bAlertable)
{
	DWORD dwNewDelay;
	dwNewDelay=dwMilliseconds;
	if ((dwMilliseconds!=INFINITE) && (dwMilliseconds!=0)){
		dwNewDelay = dxw.StretchTime(dwMilliseconds);
		if (dwNewDelay==0) dwNewDelay=1; // minimum allowed...
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
		if(dxw.dwFlags5 & NORMALIZEPERFCOUNT) {
			LARGE_INTEGER PerfFrequency;
			static LARGE_INTEGER StartCounter = {0LL};
			if (StartCounter.QuadPart == 0LL) StartCounter.QuadPart = CurrentInCount.QuadPart;
			(*pQueryPerformanceFrequency)(&PerfFrequency);
			CurrentInCount.QuadPart = ((CurrentInCount.QuadPart - StartCounter.QuadPart) * 1000000LL) / PerfFrequency.QuadPart;
		}
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
	else{
		if(dxw.dwFlags5 & NORMALIZEPERFCOUNT){
			lpPerformanceFrequency->QuadPart = 1000000LL;
			ret = TRUE;
		}
		else{
			ret = (*pQueryPerformanceFrequency)(lpPerformanceFrequency);
		}
	}
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
		case SYSLIBIDX_GDI32:
			if (remap=Remap_GDI32_ProcAddress(proc, hModule)) return remap;
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
			if(dxw.Windowize) if (remap=Remap_gl_ProcAddress(proc, hModule)) return remap;
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
		case SYSLIBIDX_IMAGEHLP:
			if (remap=Remap_Imagehlp_ProcAddress(proc, hModule)) return remap;
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
				/* if (!pChangeDisplaySettingsA) */ pChangeDisplaySettingsA=(ChangeDisplaySettingsA_Type)(*pGetProcAddress)(hModule, proc);
				OutTraceDW("GetProcAddress: hooking proc=%s at addr=%x\n", ProcToString(proc), pChangeDisplaySettingsA);
				return (FARPROC)extChangeDisplaySettingsA;
			}
			break;
#ifndef ANTICHEATING
		case SYSLIBIDX_KERNEL32:
			if ((DWORD)proc == 0x0305){ // "IsDebuggerPresent"
				OutTraceDW("GetProcAddress: hooking proc=%s at addr=%x\n", ProcToString(proc), extIsDebuggerPresent);
				return (FARPROC)extIsDebuggerPresent;
			}
			if ((DWORD)proc == 0x0050){ // "CheckRemoteDebuggerPresent"
				OutTraceDW("GetProcAddress: hooking proc=%s at addr=%x\n", ProcToString(proc), extCheckRemoteDebuggerPresent);
				return (FARPROC)extCheckRemoteDebuggerPresent;
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
	if (dxw.dwFlags4 & HIDECDROMEMPTY){ 
		BOOL Vol;
		Vol = GetVolumeInformation(lpRootPathName, NULL, NULL, NULL, 0, 0, 0, 0);
		OutTrace("Vol=%x\n", Vol);
		if(!Vol) return DRIVE_UNKNOWN;
	}
	return (*pGetDriveType)(lpRootPathName);
}

DWORD WINAPI extGetLogicalDrives(void)
{
	DWORD DevMask;
	OutTraceDW("GetLogicalDrives:\n");
	DevMask = (*pGetLogicalDrives)();
	if (dxw.dwFlags4 & HIDECDROMEMPTY){ 
		for(int i=0; i<32; i++){
			DWORD DevBit;
			BOOL Vol;
			DevBit = 0x1 << i;
			if(DevMask & DevBit){
				char RootPathName[10];
				sprintf_s(RootPathName, 4, "%c:\\", 'A'+i);
				Vol = GetVolumeInformation(RootPathName, NULL, NULL, NULL, 0, 0, 0, 0);
				OutTrace("Vol=%s status=%x\n", RootPathName, Vol);
				if(!Vol) DevMask &= ~DevBit;
			}
		}
	}
	return DevMask;
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

static char *ExplainDebugEvent(DWORD ec)
{
	char *e;
	switch(ec){
		case EXCEPTION_DEBUG_EVENT: e="EXCEPTION"; break;
		case CREATE_THREAD_DEBUG_EVENT: e="CREATE_THREAD"; break;
		case CREATE_PROCESS_DEBUG_EVENT: e="CREATE_PROCESS"; break;
		case EXIT_THREAD_DEBUG_EVENT: e="EXIT_THREAD"; break;
		case EXIT_PROCESS_DEBUG_EVENT: e="EXIT_PROCESS"; break;
		case LOAD_DLL_DEBUG_EVENT: e="LOAD_DLL"; break;
		case UNLOAD_DLL_DEBUG_EVENT: e="UNLOAD_DLL"; break;
		case OUTPUT_DEBUG_STRING_EVENT: e="OUTPUT_DEBUG"; break;
		case RIP_EVENT: e="RIP"; break;
		default: e="unknown"; break;
	}
	return e;
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
	BOOL res;
#ifdef LOCKINJECTIONTHREADS
	DWORD StartingCode;
	LPVOID StartAddress = 0;
	extern LPVOID GetThreadStartAddress(HANDLE);
#endif
	OutTraceDW("CreateProcess: ApplicationName=\"%s\" CommandLine=\"%s\"\n", lpApplicationName, lpCommandLine);
	if(dxw.dwFlags4 & SUPPRESSCHILD) {
		OutTraceDW("CreateProcess: SUPPRESS\n");
		return TRUE;
	}

	if(dxw.dwFlags5 & (INJECTSON|ENABLESONHOOK)) {
		extern HANDLE hLockMutex;
		ReleaseMutex(hLockMutex);
	}

	if(dxw.dwFlags5 & INJECTSON) {
		DEBUG_EVENT debug_event ={0};
		char path[MAX_PATH];
		extern char *GetFileNameFromHandle(HANDLE);
		DWORD dwContinueStatus = DBG_CONTINUE;
		extern BOOL Inject(DWORD, const char *);
		LPVOID LastExceptionPtr = 0;

		//dwCreationFlags |= DEBUG_ONLY_THIS_PROCESS;
		dwCreationFlags |= (DEBUG_ONLY_THIS_PROCESS|DEBUG_PROCESS);

		res=(*pCreateProcessA)(
			lpApplicationName, lpCommandLine,
			lpProcessAttributes, lpThreadAttributes, bInheritHandles,
			dwCreationFlags, lpEnvironment,
			lpCurrentDirectory, lpStartupInfo, lpProcessInformation
		);
		OutTrace("CreateProcess res=%x\n", res);
		BOOL bContinueDebugging = TRUE;
		while(bContinueDebugging)
		{ 
			if (!WaitForDebugEvent(&debug_event, INFINITE)) break;
			OutTrace("CreateProcess: event=%x(%s)\n", debug_event.dwDebugEventCode, ExplainDebugEvent(debug_event.dwDebugEventCode));
			switch(debug_event.dwDebugEventCode){
			case EXIT_PROCESS_DEBUG_EVENT:
				//OutTrace("CreateProcess: event=%x(%s) process terminated\n", debug_event.dwDebugEventCode, ExplainDebugEvent(debug_event.dwDebugEventCode));
				bContinueDebugging=false;
				break;
			case CREATE_PROCESS_DEBUG_EVENT:
				//OutTrace("CreateProcess: event=%x(%s) process started\n", debug_event.dwDebugEventCode, ExplainDebugEvent(debug_event.dwDebugEventCode));
				GetModuleFileName(GetModuleHandle("dxwnd"), path, MAX_PATH);
				OutTrace("CreateProcess: injecting path=%s\n", path);
				if(!Inject(lpProcessInformation->dwProcessId, path)){
					OutTrace("CreateProcess: Injection ERROR pid=%x dll=%s\n", lpProcessInformation->dwProcessId, path);
				}
#ifdef LOCKINJECTIONTHREADS
					extern LPVOID GetThreadStartAddress(HANDLE);
					DWORD TargetHandle;
					DWORD EndlessLoop;
					EndlessLoop=0x9090FEEB; // assembly for JMP to here, NOP, NOP
					SIZE_T BytesCount;
					TargetHandle = (DWORD)OpenProcess(
						PROCESS_QUERY_INFORMATION|PROCESS_VM_OPERATION|PROCESS_VM_READ|PROCESS_VM_WRITE, 
						FALSE, 
						lpProcessInformation->dwProcessId);
					if(TargetHandle){
						StartAddress = GetThreadStartAddress(lpProcessInformation->hThread);
						OutTrace("CreateProcess: StartAddress=%x\n", StartAddress);
						if(StartAddress){
							if(!ReadProcessMemory(lpProcessInformation->hProcess, StartAddress, &StartingCode, 4, &BytesCount)){ 
								OutTrace("CreateProcess: ReadProcessMemory error=%d\n", GetLastError());
							}
							OutTrace("CreateProcess: StartCode=%x\n", StartingCode);
							if(!WriteProcessMemory(lpProcessInformation->hProcess, StartAddress, &EndlessLoop, 4, &BytesCount)){
								OutTrace("CreateProcess: WriteProcessMemory error=%d\n", GetLastError());
							}
						}
					}
#endif				
				OutTrace("CreateProcess: injection started\n", res);
				CloseHandle(((CREATE_PROCESS_DEBUG_INFO *)&debug_event.u)->hProcess);
				CloseHandle(((CREATE_PROCESS_DEBUG_INFO *)&debug_event.u)->hThread);
				CloseHandle(((CREATE_PROCESS_DEBUG_INFO *)&debug_event.u)->hFile);
				break;
			case EXIT_THREAD_DEBUG_EVENT:
				//OutTrace("CreateProcess: event=%x(%s) injection terminated\n", debug_event.dwDebugEventCode, ExplainDebugEvent(debug_event.dwDebugEventCode));
#ifdef LOCKINJECTIONTHREADS
				if(TargetHandle && StartAddress){
					if(dxw.dwFlags5 & FREEZEINJECTEDSON){
						OutTrace("CreateProcess: FREEZEINJECTEDSON leaving son process in endless loop\n", GetLastError());
					}
					else{
					if(!WriteProcessMemory(lpProcessInformation->hProcess, StartAddress, &StartingCode, 4, &BytesCount)){
						OutTrace("CreateProcess: WriteProcessMemory error=%d\n", GetLastError());
					}
					}
					CloseHandle((HANDLE)TargetHandle);
					OutTrace("CreateProcess: injection terminated\n", res);
				}
#endif
				bContinueDebugging=false;
			case EXCEPTION_DEBUG_EVENT:
				//OutTrace("CreateProcess: event=%x(%s)\n", debug_event.dwDebugEventCode, ExplainDebugEvent(debug_event.dwDebugEventCode));
				{
					LPEXCEPTION_DEBUG_INFO ei;
					ei=(LPEXCEPTION_DEBUG_INFO)&debug_event.u;
					OutTraceE("CreateProcess: EXCEPTION code=%x flags=%x addr=%x firstchance=%x\n", 
						ei->ExceptionRecord.ExceptionCode, 
						ei->ExceptionRecord.ExceptionFlags, 
						ei->ExceptionRecord.ExceptionAddress,
						debug_event.u.Exception.dwFirstChance);
					// exception twice in same address, then do not continue.
					if(LastExceptionPtr == ei->ExceptionRecord.ExceptionAddress) bContinueDebugging = FALSE;
					//if(ei->dwFirstChance == 0) bContinueDebugging = FALSE;
					LastExceptionPtr = ei->ExceptionRecord.ExceptionAddress;
				}
				break;
			case LOAD_DLL_DEBUG_EVENT:
				//OutTrace("CreateProcess: event=%x(%s) dll=%s address=%x\n", 
				//	debug_event.dwDebugEventCode, ExplainDebugEvent(debug_event.dwDebugEventCode),
				//	((LOAD_DLL_DEBUG_INFO *)&debug_event.u)->lpImageName, ((LOAD_DLL_DEBUG_INFO *)&debug_event.u)->lpBaseOfDll);
				CloseHandle(((LOAD_DLL_DEBUG_INFO *)&debug_event.u)->hFile);
				break;
			case CREATE_THREAD_DEBUG_EVENT:
				CloseHandle(((CREATE_THREAD_DEBUG_INFO *)&debug_event.u)->hThread);
				break;
			default:
				break;
			}
			if(bContinueDebugging){
				ContinueDebugEvent(debug_event.dwProcessId, 
					debug_event.dwThreadId, 
					dwContinueStatus);
			}
			else{
				DebugSetProcessKillOnExit(FALSE);
				ContinueDebugEvent(debug_event.dwProcessId, debug_event.dwThreadId, DBG_CONTINUE); 
				DebugActiveProcessStop(debug_event.dwProcessId);
			}
		}
		OutTrace("CreateProcess: detached\n", res);
	}
	else{
		res=(*pCreateProcessA)(
			lpApplicationName, 
			lpCommandLine, 
			lpProcessAttributes,
			lpThreadAttributes,
			bInheritHandles,
			dwCreationFlags,
			lpEnvironment,
			lpCurrentDirectory,
			lpStartupInfo,
			lpProcessInformation
		);
	}

	if(!res) OutTraceE("CreateProcess: ERROR err=%d\n", GetLastError());
	return res;
}

BOOL WINAPI extGetExitCodeProcess(HANDLE hProcess, LPDWORD lpExitCode)
{
	BOOL res;

	OutTraceDW("GetExitCodeProcess: hProcess=%x\n", hProcess);

	if(dxw.dwFlags4 & SUPPRESSCHILD) {
		OutTraceDW("GetExitCodeProcess: FAKE exit code=0\n");
		lpExitCode = 0;
		return TRUE;
	}

	res=(*pGetExitCodeProcess)(hProcess, lpExitCode);
	if(dxw.dwFlags5 & (INJECTSON|ENABLESONHOOK)) {
		if(*lpExitCode != STILL_ACTIVE){
			OutTraceDW("GetExitCodeProcess: locking mutex\n");
			extern HANDLE hLockMutex;
			WaitForSingleObject(hLockMutex, 0);
		}
	}
	OutTraceDW("GetExitCodeProcess: hProcess=%x ExitCode=%x res=%x\n", hProcess, *lpExitCode, res);
	return res;
}

BOOL WINAPI extCheckRemoteDebuggerPresent(HANDLE hProcess, PBOOL pbDebuggerPresent)
{
	BOOL ret;
	if(pbDebuggerPresent) *pbDebuggerPresent = FALSE;
	ret= (hProcess==(HANDLE)0xFFFFFFFF) ? FALSE : TRUE;
	OutTraceDW("CheckRemoteDebuggerPresent: hProcess=%x ret=%x\n", hProcess, ret);
	return ret;
}

UINT WINAPI extGetTempFileName(LPCTSTR lpPathName, LPCTSTR lpPrefixString, UINT uUnique, LPTSTR lpTempFileName)
{
	UINT ret;
	OutTraceDW("GetTempFileName: PathName=\"%s\" PrefixString=%s Unique=%d\n", lpPathName, lpPrefixString, uUnique);
	ret = (*pGetTempFileName)(lpPathName, lpPrefixString, uUnique, lpTempFileName);
	if(ret == 0){
		// GetTempFileName patch to make "Powerslide" working
		OutTraceDW("GetTempFileName FAILED: error=%d at %d\n", GetLastError(), __LINE__);
		char sTmpDir[MAX_PATH+1];
		GetTempPath(sizeof(sTmpDir), sTmpDir);
		ret = (*pGetTempFileName)(sTmpDir, lpPrefixString, uUnique, lpTempFileName);
		if(ret == 0) OutTraceDW("GetTempFileName FAILED: PathName=\"%s\" error=%d line %d\n", sTmpDir, GetLastError(), __LINE__);
	}
	if(ret){
		OutTraceDW("GetTempFileName: TempFileName=\"%s\" ret=%d\n", lpTempFileName, ret);
	}
	return ret;
}
