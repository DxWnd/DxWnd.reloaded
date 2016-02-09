#define  _CRT_SECURE_NO_WARNINGS
#include "dxwnd.h"
#include "dxwcore.hpp"
#include "syslibs.h"
#include "dxhook.h"
#include "dxhelper.h"
#include "hddraw.h"
#include "stdio.h"

//#undef IsTraceDW
//#define IsTraceDW TRUE
//#define LOCKINJECTIONTHREADS
#define TRYFATNAMES TRUE

extern HRESULT WINAPI extDirectDrawEnumerateA(LPDDENUMCALLBACK, LPVOID);
extern HRESULT WINAPI extDirectDrawEnumerateExA(LPDDENUMCALLBACKEX, LPVOID, DWORD);

BOOL WINAPI extCheckRemoteDebuggerPresent(HANDLE, PBOOL);
LPVOID WINAPI extVirtualAlloc(LPVOID, SIZE_T, DWORD, DWORD);
UINT WINAPI extWinExec(LPCSTR, UINT);
BOOL WINAPI extSetPriorityClass(HANDLE, DWORD);
BOOL WINAPI extGlobalUnlock(HGLOBAL);

typedef LPVOID (WINAPI *VirtualAlloc_Type)(LPVOID, SIZE_T, DWORD, DWORD);
typedef BOOL (WINAPI *CreateProcessA_Type)(LPCTSTR, LPTSTR, LPSECURITY_ATTRIBUTES, LPSECURITY_ATTRIBUTES, 
										   BOOL, DWORD, LPVOID, LPCTSTR, LPSTARTUPINFO, LPPROCESS_INFORMATION);
typedef BOOL (WINAPI *SetPriorityClass_Type)(HANDLE, DWORD);
typedef UINT (WINAPI *WinExec_Type)(LPCSTR, UINT);
typedef BOOL (WINAPI *GlobalUnlock_Type)(HGLOBAL);

CreateProcessA_Type pCreateProcessA = NULL;
VirtualAlloc_Type pVirtualAlloc = NULL;
WinExec_Type pWinExec = NULL;
SetPriorityClass_Type pSetPriorityClass = NULL;
GlobalUnlock_Type pGlobalUnlock = NULL;

#ifdef NOFREELIBRARY
typedef BOOL (WINAPI *FreeLibrary_Type)(HMODULE);
FreeLibrary_Type pFreeLibrary = NULL;
BOOL WINAPI extFreeLibrary(HMODULE hModule)
{ 
	OutTrace("FreeLibrary: hModule=%x SUPPRESS\n", hModule);
	return TRUE; 
}
#endif

// v2.02.96: the GetSystemInfo API is NOT hot patchable on Win7. This can cause problems because it can't be hooked by simply
// enabling hot patch. A solution is making all LoadLibrary* calls hot patchable, so that when loading the module, the call
// can be hooked by the IAT lookup. This fixes a problem after movie playing in Wind Fantasy SP.

static HookEntryEx_Type Hooks[]={
	{HOOK_IAT_CANDIDATE, 0, "IsDebuggerPresent", (FARPROC)NULL, (FARPROC *)NULL, (FARPROC)extIsDebuggerPresent},
	{HOOK_IAT_CANDIDATE, 0, "CheckRemoteDebuggerPresent", (FARPROC)NULL, (FARPROC *)NULL, (FARPROC)extCheckRemoteDebuggerPresent},
	{HOOK_IAT_CANDIDATE, 0, "GetProcAddress", (FARPROC)GetProcAddress, (FARPROC *)&pGetProcAddress, (FARPROC)extGetProcAddress},
	{HOOK_HOT_CANDIDATE, 0, "LoadLibraryA", (FARPROC)LoadLibraryA, (FARPROC *)&pLoadLibraryA, (FARPROC)extLoadLibraryA},
	{HOOK_HOT_CANDIDATE, 0, "LoadLibraryExA", (FARPROC)LoadLibraryExA, (FARPROC *)&pLoadLibraryExA, (FARPROC)extLoadLibraryExA},
	{HOOK_HOT_CANDIDATE, 0, "LoadLibraryW", (FARPROC)LoadLibraryW, (FARPROC *)&pLoadLibraryW, (FARPROC)extLoadLibraryW},
	{HOOK_HOT_CANDIDATE, 0, "LoadLibraryExW", (FARPROC)LoadLibraryExW, (FARPROC *)&pLoadLibraryExW, (FARPROC)extLoadLibraryExW},
	{HOOK_IAT_CANDIDATE, 0, "GetDriveTypeA", (FARPROC)NULL, (FARPROC *)&pGetDriveType, (FARPROC)extGetDriveType},
	{HOOK_IAT_CANDIDATE, 0, "GetLogicalDrives", (FARPROC)NULL, (FARPROC *)&pGetLogicalDrives, (FARPROC)extGetLogicalDrives},
	{HOOK_IAT_CANDIDATE, 0, "GetTempFileNameA", (FARPROC)GetTempFileNameA, (FARPROC *)&pGetTempFileName, (FARPROC)extGetTempFileName},
	{HOOK_IAT_CANDIDATE, 0, "CreateProcessA", (FARPROC)NULL, (FARPROC *)&pCreateProcessA, (FARPROC)extCreateProcessA},
	//{HOOK_IAT_CANDIDATE, 0, "WinExec", (FARPROC)NULL, (FARPROC *)&pWinExec, (FARPROC)extWinExec},
	{HOOK_HOT_CANDIDATE, 0, "SetPriorityClass", (FARPROC)SetPriorityClass, (FARPROC *)&pSetPriorityClass, (FARPROC)extSetPriorityClass},
	{HOOK_HOT_CANDIDATE, 0, "GlobalUnlock", (FARPROC)GlobalUnlock, (FARPROC *)&pGlobalUnlock, (FARPROC)extGlobalUnlock},
#ifdef NOFREELIBRARY
	{HOOK_HOT_CANDIDATE, 0, "FreeLibrary", (FARPROC)FreeLibrary, (FARPROC *)&pFreeLibrary, (FARPROC)extFreeLibrary},
#endif
	{HOOK_IAT_CANDIDATE, 0, 0, NULL, 0, 0} // terminator
};

static HookEntryEx_Type FixAllocHooks[]={
	{HOOK_IAT_CANDIDATE, 0, "VirtualAlloc", (FARPROC)VirtualAlloc, (FARPROC *)&pVirtualAlloc, (FARPROC)extVirtualAlloc},
	{HOOK_IAT_CANDIDATE, 0, 0, NULL, 0, 0} // terminator
};

static HookEntryEx_Type FixIOHooks[]={
	{HOOK_IAT_CANDIDATE, 0, "ReadFile", (FARPROC)NULL, (FARPROC *)&pReadFile, (FARPROC)extReadFile},
	{HOOK_IAT_CANDIDATE, 0, "CreateFileA", (FARPROC)NULL, (FARPROC *)&pCreateFile, (FARPROC)extCreateFile},
	{HOOK_IAT_CANDIDATE, 0, "SetFilePointer", (FARPROC)NULL, (FARPROC *)&pSetFilePointer, (FARPROC)extSetFilePointer},
	{HOOK_IAT_CANDIDATE, 0, "CloseHandle", (FARPROC)NULL, (FARPROC *)&pCloseHandle, (FARPROC)extCloseHandle},
	{HOOK_IAT_CANDIDATE, 0, 0, NULL, 0, 0} // terminator
};

static HookEntryEx_Type LimitHooks[]={
	{HOOK_HOT_CANDIDATE, 0, "GetDiskFreeSpaceA", (FARPROC)GetDiskFreeSpaceA, (FARPROC *)&pGetDiskFreeSpaceA, (FARPROC)extGetDiskFreeSpaceA},
	{HOOK_HOT_CANDIDATE, 0, "GlobalMemoryStatus", (FARPROC)GlobalMemoryStatus, (FARPROC *)&pGlobalMemoryStatus, (FARPROC)extGlobalMemoryStatus},
	{HOOK_IAT_CANDIDATE, 0, 0, NULL, 0, 0} // terminator
};

static HookEntryEx_Type TimeHooks[]={
	{HOOK_HOT_CANDIDATE, 0, "GetTickCount", (FARPROC)GetTickCount, (FARPROC *)&pGetTickCount, (FARPROC)extGetTickCount},
	{HOOK_HOT_CANDIDATE, 0, "GetLocalTime", (FARPROC)GetLocalTime, (FARPROC *)&pGetLocalTime, (FARPROC)extGetLocalTime},
	{HOOK_HOT_CANDIDATE, 0, "GetSystemTime", (FARPROC)GetSystemTime, (FARPROC *)&pGetSystemTime, (FARPROC)extGetSystemTime},
	{HOOK_HOT_CANDIDATE, 0, "GetSystemTimeAsFileTime", (FARPROC)GetSystemTimeAsFileTime, (FARPROC *)&pGetSystemTimeAsFileTime, (FARPROC)extGetSystemTimeAsFileTime},
	{HOOK_HOT_CANDIDATE, 0, "Sleep", (FARPROC)Sleep, (FARPROC *)&pSleep, (FARPROC)extSleep},
	{HOOK_HOT_CANDIDATE, 0, "SleepEx", (FARPROC)SleepEx, (FARPROC *)&pSleepEx, (FARPROC)extSleepEx},
	{HOOK_HOT_CANDIDATE, 0, "QueryPerformanceCounter", (FARPROC)QueryPerformanceCounter, (FARPROC *)&pQueryPerformanceCounter, (FARPROC)extQueryPerformanceCounter},
	{HOOK_HOT_CANDIDATE, 0, "QueryPerformanceFrequency", (FARPROC)QueryPerformanceFrequency, (FARPROC *)&pQueryPerformanceFrequency, (FARPROC)extQueryPerformanceFrequency},
	{HOOK_IAT_CANDIDATE, 0, 0, NULL, 0, 0} // terminator
};

static HookEntryEx_Type VersionHooks[]={
	{HOOK_HOT_CANDIDATE, 0, "GetVersion", (FARPROC)GetVersion, (FARPROC *)&pGetVersion, (FARPROC)extGetVersion},
	{HOOK_HOT_CANDIDATE, 0, "GetVersionExA", (FARPROC)GetVersionExA, (FARPROC *)&pGetVersionExA, (FARPROC)extGetVersionExA},
	{HOOK_HOT_CANDIDATE, 0, "GetVersionExW", (FARPROC)GetVersionExW, (FARPROC *)&pGetVersionExW, (FARPROC)extGetVersionExW},
	{HOOK_IAT_CANDIDATE, 0, 0, NULL, 0, 0} // terminator
};

static char *libname = "kernel32.dll";

void HookKernel32(HMODULE module)
{
	HookLibraryEx(module, Hooks, libname);
	if(dxw.dwFlags3 & BUFFEREDIOFIX) HookLibraryEx(module, FixIOHooks, libname);
	if(dxw.dwFlags2 & LIMITRESOURCES) HookLibraryEx(module, LimitHooks, libname);
	if(dxw.dwFlags2 & TIMESTRETCH) HookLibraryEx(module, TimeHooks, libname);
	if(dxw.dwFlags2 & FAKEVERSION) HookLibraryEx(module, VersionHooks, libname);
	if(dxw.dwFlags6 & LEGACYALLOC) HookLibraryEx(module, FixAllocHooks, libname);
}

void HookKernel32Init()
{
	HookLibInitEx(Hooks);
	HookLibInitEx(FixIOHooks);
	HookLibInitEx(LimitHooks);
	HookLibInitEx(TimeHooks);
	HookLibInitEx(VersionHooks);
	HookLibInitEx(FixAllocHooks);
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
			
	if (addr=RemapLibraryEx(proc, hModule, Hooks)) return addr;

	if(dxw.dwFlags3 & BUFFEREDIOFIX)
		if (addr=RemapLibraryEx(proc, hModule, FixIOHooks)) return addr;

	if(dxw.dwFlags2 & LIMITRESOURCES)
		if (addr=RemapLibraryEx(proc, hModule, LimitHooks)) return addr;

	if(dxw.dwFlags2 & TIMESTRETCH)
		if (addr=RemapLibraryEx(proc, hModule, TimeHooks)) return addr;

	if(dxw.dwFlags2 & FAKEVERSION)
		if (addr=RemapLibraryEx(proc, hModule, VersionHooks)) return addr;

	if(dxw.dwFlags6 & LEGACYALLOC)
		if (addr=RemapLibraryEx(proc, hModule, FixAllocHooks)) return addr;
	return NULL;
}

extern DirectDrawEnumerateA_Type pDirectDrawEnumerateA;
extern DirectDrawEnumerateExA_Type pDirectDrawEnumerateExA;
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

// v2.03.20: "Talonsoft's Operational Art of War II" checks the dwPlatformId field
// v2.03.20: list revised according to Microsoft compatibility settings

static struct {DWORD bMajor; DWORD bMinor; DWORD dwPlatformId; DWORD build; char *sName;} WinVersions[9]=
{
	{4, 0, VER_PLATFORM_WIN32_WINDOWS,	950,		"Windows 95"},
	{4,10, VER_PLATFORM_WIN32_WINDOWS,	67766446,	"Windows 98/SE"},
	{4,90, VER_PLATFORM_WIN32_WINDOWS,	0,			"Windows ME"},
//	{4, 0, VER_PLATFORM_WIN32_NT,		1381,		"Windows NT4.0(sp5)"},
	{5, 0, VER_PLATFORM_WIN32_NT,		2195,		"Windows 2000"},
//	{5, 1, VER_PLATFORM_WIN32_NT,		2600,		"Windows XP(sp2)"},
	{5, 1, VER_PLATFORM_WIN32_NT,		2600,		"Windows XP(sp3)"},
	{5, 2, VER_PLATFORM_WIN32_NT,		3790,		"Windows Server 2003(sp1)"},
//	{6, 0, VER_PLATFORM_WIN32_NT,		6001,		"Windows Server 2008(sp1)"},
//	{6, 0, VER_PLATFORM_WIN32_NT,		6000,		"Windows Vista"},
//	{6, 0, VER_PLATFORM_WIN32_NT,		6001,		"Windows Vista(sp1)"},
	{6, 0, VER_PLATFORM_WIN32_NT,		6002,		"Windows Vista(sp2)"},
	{6, 1, VER_PLATFORM_WIN32_NT,		7600,		"Windows 7"},
	{6, 2, VER_PLATFORM_WIN32_NT,		0,			"Windows 8"}
};

BOOL WINAPI extGetVersionExA(LPOSVERSIONINFOA lpVersionInfo)
{
	BOOL ret;

	ret=(*pGetVersionExA)(lpVersionInfo);
	if(!ret) {
		OutTraceE("GetVersionExA: ERROR err=%d\n", GetLastError());
		return ret;
	}

	OutTraceDW("GetVersionExA: version=%d.%d platform=%x build=(%d)\n", 
		lpVersionInfo->dwMajorVersion, lpVersionInfo->dwMinorVersion, lpVersionInfo->dwPlatformId, lpVersionInfo->dwBuildNumber);

	if(dxw.dwFlags2 & FAKEVERSION) {
		// fake Win XP build 0
		lpVersionInfo->dwMajorVersion = WinVersions[dxw.FakeVersionId].bMajor;
		lpVersionInfo->dwMinorVersion = WinVersions[dxw.FakeVersionId].bMinor;
		lpVersionInfo->dwPlatformId = WinVersions[dxw.FakeVersionId].dwPlatformId;
		lpVersionInfo->dwBuildNumber = 0;
		OutTraceDW("GetVersionExA: FIXED version=%d.%d platform=%x build=(%d) os=\"%s\"\n", 
			lpVersionInfo->dwMajorVersion, lpVersionInfo->dwMinorVersion, lpVersionInfo->dwPlatformId, lpVersionInfo->dwBuildNumber,
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

	OutTraceDW("GetVersionExW: version=%d.%d platform=%x build=(%d)\n", 
		lpVersionInfo->dwMajorVersion, lpVersionInfo->dwMinorVersion, lpVersionInfo->dwPlatformId, lpVersionInfo->dwBuildNumber);

	if(dxw.dwFlags2 & FAKEVERSION) {
		// fake Win XP build 0
		lpVersionInfo->dwMajorVersion = WinVersions[dxw.FakeVersionId].bMajor;
		lpVersionInfo->dwMinorVersion = WinVersions[dxw.FakeVersionId].bMinor;
		lpVersionInfo->dwPlatformId = WinVersions[dxw.FakeVersionId].dwPlatformId;
		lpVersionInfo->dwBuildNumber = 0;
		OutTraceDW("GetVersionExW: FIXED version=%d.%d platform=%x build=(%d) os=\"%s\"\n", 
			lpVersionInfo->dwMajorVersion, lpVersionInfo->dwMinorVersion, lpVersionInfo->dwPlatformId, lpVersionInfo->dwBuildNumber,
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
	DWORD ret;
	ret=dxw.GetTickCount();
	//OutTraceB("GetTickCount: ret=%x\n", ret); 
	return ret;
}

void WINAPI extGetSystemTime(LPSYSTEMTIME lpSystemTime)
{
	dxw.GetSystemTime(lpSystemTime);
	OutTraceB("GetSystemTime: %02d:%02d:%02d.%03d\n", 
		lpSystemTime->wHour, lpSystemTime->wMinute, lpSystemTime->wSecond, lpSystemTime->wMilliseconds);
}

void WINAPI extGetLocalTime(LPSYSTEMTIME lpLocalTime)
{
	SYSTEMTIME SystemTime;
	dxw.GetSystemTime(&SystemTime);
	SystemTimeToTzSpecificLocalTime(NULL, &SystemTime, lpLocalTime);
	OutTraceB("GetLocalTime: %02d:%02d:%02d.%03d\n", 
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
	OutTraceB("Sleep: msec=%d->%d timeshift=%d\n", dwMilliseconds, dwNewDelay, dxw.TimeShift);
	(*pSleep)(dwNewDelay);
	while(dxw.TimeFreeze)(*pSleep)(40);
}

DWORD WINAPI extSleepEx(DWORD dwMilliseconds, BOOL bAlertable)
{
	DWORD ret;
	DWORD dwNewDelay;
	dwNewDelay=dwMilliseconds;
	if ((dwMilliseconds!=INFINITE) && (dwMilliseconds!=0)){
		dwNewDelay = dxw.StretchTime(dwMilliseconds);
		if (dwNewDelay==0) dwNewDelay=1; // minimum allowed...
	}
	OutTraceB("SleepEx: msec=%d->%d alertable=%x, timeshift=%d\n", dwMilliseconds, dwNewDelay, bAlertable, dxw.TimeShift);
	ret = (*pSleepEx)(dwNewDelay, bAlertable);
	while(dxw.TimeFreeze)(*pSleep)(40);
	return ret;
}

void WINAPI extGetSystemTimeAsFileTime(LPFILETIME lpSystemTimeAsFileTime)
{
	OutTraceB("GetSystemTimeAsFileTime\n");
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
	if((DWORD)proc & 0xFFFF0000){
		FARPROC remap;
		switch(idx){
			case SYSLIBIDX_AVIFIL32:
			if (remap=Remap_AVIFil32_ProcAddress(proc, hModule)) return remap;
			break;
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
		case SYSLIBIDX_DINPUT:
			if (remap=Remap_DInput_ProcAddress(proc, hModule)) return remap;
			break;
		case SYSLIBIDX_DINPUT8:
			if (remap=Remap_DInput8_ProcAddress(proc, hModule)) return remap;
			break;
		case SYSLIBIDX_COMCTL32:
			if (remap=Remap_ComCtl32_ProcAddress(proc, hModule)) return remap;
			break;
		case SYSLIBIDX_COMDLG32:
			if (remap=Remap_ComDlg32_ProcAddress(proc, hModule)) return remap;
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
					pDirectDrawEnumerateA=(DirectDrawEnumerateA_Type)(*pGetProcAddress)(hModule, proc);
					OutTraceP("GetProcAddress: hooking proc=%s at addr=%x\n", proc, pDirectDrawEnumerateA);
					return (FARPROC)extDirectDrawEnumerateA;
					break;
				case 0x000C: // DirectDrawEnumerateExA
					pDirectDrawEnumerateExA=(DirectDrawEnumerateExA_Type)(*pGetProcAddress)(hModule, proc);
					OutTraceP("GetProcAddress: hooking proc=%s at addr=%x\n", proc, pDirectDrawEnumerateExA);
					return (FARPROC)extDirectDrawEnumerateExA;
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
	memset(lpBuffer, 0, nNumberOfBytesToRead);

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

static char *ExplainFlagsAndAttributes(DWORD c)
{
	static char eb[256];
	unsigned int l;
	strcpy(eb,"");
	if (c & FILE_ATTRIBUTE_ARCHIVE) strcat(eb, "FILE_ATTRIBUTE_ARCHIVE+");
	if (c & FILE_ATTRIBUTE_ENCRYPTED) strcat(eb, "FILE_ATTRIBUTE_ENCRYPTED+");
	if (c & FILE_ATTRIBUTE_HIDDEN) strcat(eb, "FILE_ATTRIBUTE_HIDDEN+");
	if (c & FILE_ATTRIBUTE_NORMAL) strcat(eb, "FILE_ATTRIBUTE_NORMAL+");
	if (c & FILE_ATTRIBUTE_OFFLINE) strcat(eb, "FILE_ATTRIBUTE_OFFLINE+");
	if (c & FILE_ATTRIBUTE_READONLY) strcat(eb, "FILE_ATTRIBUTE_READONLY+");
	if (c & FILE_ATTRIBUTE_SYSTEM) strcat(eb, "FILE_ATTRIBUTE_SYSTEM+");
	if (c & FILE_ATTRIBUTE_TEMPORARY) strcat(eb, "FILE_ATTRIBUTE_TEMPORARY+");
	if (c & FILE_FLAG_BACKUP_SEMANTICS) strcat(eb, "FILE_FLAG_BACKUP_SEMANTICS+");
	if (c & FILE_FLAG_DELETE_ON_CLOSE) strcat(eb, "FILE_FLAG_DELETE_ON_CLOSE+");
	if (c & FILE_FLAG_NO_BUFFERING) strcat(eb, "FILE_FLAG_NO_BUFFERING+");
	if (c & FILE_FLAG_OPEN_NO_RECALL) strcat(eb, "FILE_FLAG_OPEN_NO_RECALL+");
	if (c & FILE_FLAG_OPEN_REPARSE_POINT) strcat(eb, "FILE_FLAG_OPEN_REPARSE_POINT+");
	if (c & FILE_FLAG_OVERLAPPED) strcat(eb, "FILE_FLAG_OVERLAPPED+");
	if (c & FILE_FLAG_POSIX_SEMANTICS) strcat(eb, "FILE_FLAG_POSIX_SEMANTICS+");
	if (c & FILE_FLAG_RANDOM_ACCESS) strcat(eb, "FILE_FLAG_RANDOM_ACCESS+");
	//if (c & FILE_FLAG_SESSION_AWARE) strcat(eb, "FILE_FLAG_SESSION_AWARE+");
	if (c & FILE_FLAG_SEQUENTIAL_SCAN) strcat(eb, "FILE_FLAG_SEQUENTIAL_SCAN+");
	if (c & FILE_FLAG_WRITE_THROUGH) strcat(eb, "FILE_FLAG_WRITE_THROUGH+");
	if (c & SECURITY_ANONYMOUS) strcat(eb, "SECURITY_ANONYMOUS+");
	if (c & SECURITY_CONTEXT_TRACKING) strcat(eb, "SECURITY_CONTEXT_TRACKING+");
	if (c & SECURITY_DELEGATION) strcat(eb, "SECURITY_DELEGATION+");
	if (c & SECURITY_EFFECTIVE_ONLY) strcat(eb, "SECURITY_EFFECTIVE_ONLY+");
	if (c & SECURITY_IDENTIFICATION) strcat(eb, "SECURITY_IDENTIFICATION+");
	if (c & SECURITY_IMPERSONATION) strcat(eb, "SECURITY_IMPERSONATION+");
	l=strlen(eb);
	if (l>strlen("")) eb[l-1]=0; // delete last '+' if any
	return(eb);
}

static char *ExplainDesiredAccess(DWORD c)
{
	static char eb[256];
	unsigned int l;
	strcpy(eb,"GENERIC_");
	if (c & GENERIC_READ) strcat(eb, "READ+");
	if (c & GENERIC_WRITE) strcat(eb, "WRITE+");
	if (c & GENERIC_EXECUTE) strcat(eb, "EXECUTE+");
	if (c & GENERIC_ALL) strcat(eb, "ALL+");
	l=strlen(eb);
	if (l>strlen("GENERIC_")) eb[l-1]=0; // delete last '+' if any
	else eb[0]=0;
	return(eb);
}

HANDLE WINAPI extCreateFile(LPCTSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, 
							LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition,
							DWORD dwFlagsAndAttributes, HANDLE hTemplateFile)
{
	HANDLE ret;
	int err=0;
	OutTraceDW("CreateFile: FileName=%s DesiredAccess=%x(%s) SharedMode=%x Disposition=%x Flags=%x(%s)\n", 
		lpFileName, dwDesiredAccess, ExplainDesiredAccess(dwDesiredAccess), dwShareMode, dwCreationDisposition, 
		dwFlagsAndAttributes, ExplainFlagsAndAttributes(dwFlagsAndAttributes));

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
			OutTrace("CreateFile ERROR: err=%d\n", err=GetLastError());
	}

#if 0
	if(TRYFATNAMES && (!ret) && (err==ERROR_FILE_NOT_FOUND)){
		char ShortPath[MAX_PATH+1];
		int iLastBackSlash, iFnameLength;
		char *sFileName;
		strncpy(ShortPath, lpFileName, MAX_PATH);
		iLastBackSlash = -1;
		for(size_t i=0; i<strlen(ShortPath); i++) if((ShortPath[i]=='\\') || (ShortPath[i]=='/')) iLastBackSlash=i;
		sFileName = &ShortPath[iLastBackSlash+1];
		for(size_t i=0; i<strlen(sFileName); i++) if(sFileName[i]=='.') iFnameLength=i;
		if(iFnameLength > 8){
			sFileName[6] = '~';
			sFileName[7] = '1';
			strcpy(&sFileName[8], &sFileName[iFnameLength]);
		}
		OutTrace("CreateFile: try FAT path=\"%s\"\n", ShortPath);
		ret=(*pCreateFile)(ShortPath, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
	}
#endif

	return ret;
} 

BOOL WINAPI extCloseHandle(HANDLE hObject)
{
	BOOL ret;
	OutTrace("CloseHandle: hFile=%x\n", hObject);
	if (hObject && (hObject != (HANDLE)-1)) __try {ret=CloseHandle(hObject);} __except(EXCEPTION_EXECUTE_HANDLER){};
	return ret;
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

static BOOL CreateProcessDebug(
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
	DEBUG_EVENT debug_event ={0};
	char path[MAX_PATH];
	DWORD dwContinueStatus = DBG_CONTINUE;
	extern BOOL Inject(DWORD, const char *);
	LPVOID LastExceptionPtr = 0;

	dwCreationFlags |= DEBUG_ONLY_THIS_PROCESS;

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
		OutTraceB("CreateProcess: WaitForDebugEvent pid=%x tid=%x event=%x(%s)\n", 
			debug_event.dwProcessId, debug_event.dwThreadId, debug_event.dwDebugEventCode, ExplainDebugEvent(debug_event.dwDebugEventCode));
		switch(debug_event.dwDebugEventCode){
			case EXIT_PROCESS_DEBUG_EVENT:
				bContinueDebugging=false;
				break;
			case CREATE_PROCESS_DEBUG_EVENT:
				GetModuleFileName(GetModuleHandle("dxwnd"), path, MAX_PATH);
				OutTrace("CreateProcess: injecting path=%s\n", path);
				if(!Inject(lpProcessInformation->dwProcessId, path)){
					OutTrace("CreateProcess: Injection ERROR pid=%x dll=%s\n", lpProcessInformation->dwProcessId, path);
				}
#ifdef LOCKINJECTIONTHREADS
					HANDLE TargetHandle;
					extern LPVOID GetThreadStartAddress(HANDLE);
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
				CloseHandle(debug_event.u.CreateProcessInfo.hFile);
				break;
			case EXCEPTION_DEBUG_EVENT:
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
				bContinueDebugging=false;
				break;
			case LOAD_DLL_DEBUG_EVENT:
				//OutTrace("CreateProcess: event=%x(%s) dll=%s address=%x\n", 
				//	debug_event.dwDebugEventCode, ExplainDebugEvent(debug_event.dwDebugEventCode),
				//	((LOAD_DLL_DEBUG_INFO *)&debug_event.u)->lpImageName, ((LOAD_DLL_DEBUG_INFO *)&debug_event.u)->lpBaseOfDll);
				CloseHandle(debug_event.u.LoadDll.hFile);
				break;
			case CREATE_THREAD_DEBUG_EVENT:
				OutTraceB("CreateProcess: THREAD %x\n", debug_event.u.CreateThread.hThread);
				break;
			case EXIT_THREAD_DEBUG_EVENT:
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
				OutTraceB("CreateProcess: thread exit code=%x\n", debug_event.u.ExitThread.dwExitCode);
				bContinueDebugging=false;
			default:
				break;
		}
		if(bContinueDebugging){
			ContinueDebugEvent(debug_event.dwProcessId, 
				debug_event.dwThreadId, 
				dwContinueStatus);
		}
		else{
			ContinueDebugEvent(debug_event.dwProcessId, debug_event.dwThreadId, DBG_CONTINUE); 
			if(!DebugSetProcessKillOnExit(FALSE)){
				OutTraceE("CreateProcess: DebugSetProcessKillOnExit ERROR err=%d\n", GetLastError());
			}
			if(!DebugActiveProcessStop(debug_event.dwProcessId)){
				OutTraceE("CreateProcess: DebugActiveProcessStop ERROR err=%d\n", GetLastError());
				MessageBox(NULL, "Error in DebugActiveProcessStop", "dxwnd", MB_OK);
			}
		}
	}
	OutTrace("CreateProcess: detached\n");
	return res;
}

static BOOL CreateProcessSuspended(
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
	extern BOOL Inject(DWORD, const char *);
	char StartingCode[4];
	DWORD EndlessLoop;
	EndlessLoop=0x9090FEEB; // careful: it's BIG ENDIAN: EB FE 90 90
	DWORD BytesCount;
	DWORD OldProt;
	DWORD PEHeader[0x70];
	char dllpath[MAX_PATH];
	LPVOID StartAddress;
	HANDLE TargetHandle;
	FILE *fExe = NULL;
	BOOL bKillProcess = FALSE;

	OutTrace("CreateProcessSuspended: appname=\"%s\" commandline=\"%s\" dir=\"%s\"\n", 
		lpApplicationName, lpCommandLine, lpCurrentDirectory);
	// attempt to load the specified target
	res=(*pCreateProcessA)(
		lpApplicationName, lpCommandLine, lpProcessAttributes, lpThreadAttributes, false, 
		dwCreationFlags|CREATE_SUSPENDED, lpEnvironment, lpCurrentDirectory, lpStartupInfo, lpProcessInformation);
	if (!res){
		OutTraceE("CreateProcess(CREATE_SUSPENDED) ERROR: err=%d\n", GetLastError());
		res=(*pCreateProcessA)(NULL, lpCommandLine, 0, 0, false, dwCreationFlags, NULL, lpCurrentDirectory, lpStartupInfo, lpProcessInformation);
		if(!res){ 
			OutTraceE("CreateProcess ERROR: err=%d\n", GetLastError());
		}
		return res;
	}

	while(TRUE){ // fake loop
		bKillProcess = TRUE;

		// locate the entry point
		TargetHandle = OpenProcess(
			PROCESS_QUERY_INFORMATION|PROCESS_VM_OPERATION|PROCESS_VM_READ|PROCESS_VM_WRITE|PROCESS_SUSPEND_RESUME, 
			FALSE, 
			lpProcessInformation->dwProcessId);

		FILE *fExe = fopen(lpCommandLine ? lpCommandLine : lpApplicationName, "rb");
		if(fExe==NULL){
			OutTraceE("CreateProcess: fopen %s error=%d\n", lpCommandLine, GetLastError());
			break;
		}
		// read DOS header
		if(fread((void *)PEHeader, sizeof(DWORD), 0x10, fExe)!=0x10){
			OutTraceE("CreateProcess: fread DOSHDR error=%d\n", GetLastError());
			break;
		}
		OutTraceB("CreateProcess: NT Header offset=%X\n", PEHeader[0xF]);
		fseek(fExe, PEHeader[0xF], 0);
		// read File header + Optional header
		if(fread((void *)PEHeader, sizeof(DWORD), 0x70, fExe)!=0x70){
			OutTraceE("CreateProcess: fread NTHDR error=%d\n", GetLastError());
			break;
		}

		StartAddress = (LPVOID)(PEHeader[0xA] + PEHeader[0xD]);
		OutTraceB("CreateProcess: AddressOfEntryPoint=%X ImageBase=%X startaddr=%X\n", PEHeader[0xA], PEHeader[0xD], StartAddress);

		// patch the entry point with infinite loop
		if(!VirtualProtectEx(TargetHandle, StartAddress, 4, PAGE_EXECUTE_READWRITE, &OldProt )){
			OutTraceE("CreateProcess: VirtualProtectEx error=%d\n", GetLastError());
			break;
		}

		if(!ReadProcessMemory(TargetHandle, StartAddress, &StartingCode, 4, &BytesCount)){ 
			OutTraceE("CreateProcess: ReadProcessMemory error=%d\n", GetLastError());
			break;
		}

		if(!WriteProcessMemory(TargetHandle, StartAddress, &EndlessLoop, 4, &BytesCount)){
			OutTraceE("CreateProcess: WriteProcessMemory error=%d\n", GetLastError());
			break;
		}

		if(!FlushInstructionCache(TargetHandle, StartAddress, 4)){
			OutTrace("CreateProcess: FlushInstructionCache ERROR err=%x\n", GetLastError());
			break; // error condition
		}

		// resume the main thread
		if(ResumeThread(lpProcessInformation->hThread)==(DWORD)-1){
			OutTraceE("CreateProcess: ResumeThread error=%d at:%d\n", GetLastError(), __LINE__);
			break;
		}

		// wait until the thread stuck at entry point
		CONTEXT context;
		context.Eip = (DWORD)0; // initialize to impossible value
		for ( unsigned int i = 0; i < 80 && context.Eip != (DWORD)StartAddress; ++i ){
			// patience.
			Sleep(50);

			// read the thread context
			context.ContextFlags = CONTEXT_CONTROL;
			if(!GetThreadContext(lpProcessInformation->hThread, &context)){
				OutTraceE("CreateProcess: GetThreadContext error=%d\n", GetLastError());
				break;
			}
			OutTraceB("wait cycle %d eip=%x\n", i, context.Eip);
		}

		if (context.Eip != (DWORD)StartAddress){
			// wait timed out
			OutTraceE("CreateProcess: thread blocked eip=%x addr=%x", context.Eip, StartAddress);
			break;
		}

		// inject DLL payload into remote process
		GetFullPathName("dxwnd.dll", MAX_PATH, dllpath, NULL);
		if(!Inject(lpProcessInformation->dwProcessId, dllpath)){
			// DXW_STRING_INJECTION
			OutTraceE("CreateProcess: Injection error: pid=%x dll=%s\n", lpProcessInformation->dwProcessId, dllpath);
			break;
		}

		// pause 
		if(SuspendThread(lpProcessInformation->hThread)==(DWORD)-1){
			OutTraceE("CreateProcess: SuspendThread error=%d\n", GetLastError());
			break;
		}

		// restore original entry point
		if(!WriteProcessMemory(TargetHandle, StartAddress, &StartingCode, 4, &BytesCount)){
			OutTraceE("CreateProcess: WriteProcessMemory error=%d\n", GetLastError());
			break;
		}

		if(!FlushInstructionCache(TargetHandle, StartAddress, 4)){
			OutTrace("CreateProcess: FlushInstructionCache ERROR err=%x\n", GetLastError());
			break; // error condition
		}

		// you are ready to go
		// pause and restore original entry point
		if(ResumeThread(lpProcessInformation->hThread)==(DWORD)-1){
			OutTraceE("CreateProcess: ResumeThread error=%d at:%d\n", GetLastError(), __LINE__);
			break;
		}

		bKillProcess = FALSE;
		break; // exit fake loop
	}

	// cleanup ....
	if(fExe) fclose(fExe);
	if(TargetHandle) CloseHandle(TargetHandle);
	// terminate the newly spawned process
	if(bKillProcess){
		OutTraceDW("CreateProcess: Kill son process hproc=%x pid=%x\n", lpProcessInformation->hProcess, lpProcessInformation->dwProcessId);
		if(!TerminateProcess( lpProcessInformation->hProcess, -1 )){
			OutTraceE("CreateProcess: failed to kill hproc=%x err=%d\n", lpProcessInformation->hProcess, GetLastError());
		}
	}
	OutTraceB("CreateProcess: resumed\n");
	return res;
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
	OutTraceDW("CreateProcess: ApplicationName=\"%s\" CommandLine=\"%s\" CreationFlags=%x CurrentDir=\"%s\"\n", 
		lpApplicationName, lpCommandLine, dwCreationFlags, lpCurrentDirectory);
	if(dxw.dwFlags4 & SUPPRESSCHILD) {
		OutTraceDW("CreateProcess: SUPPRESS\n");
		return TRUE;
	}

	if(dxw.dwFlags5 & (INJECTSON|ENABLESONHOOK)) {
		extern HANDLE hLockMutex;
		ReleaseMutex(hLockMutex);
	}

	if(dxw.dwFlags5 & INJECTSON) {
		res=CreateProcessDebug(
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
	else
	if(dxw.dwFlags7 & INJECTSUSPENDED) {
		res=CreateProcessSuspended(
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

LPVOID WINAPI extVirtualAlloc(LPVOID lpAddress, SIZE_T dwSize, DWORD flAllocationType, DWORD flProtect)
{
	// v2.03.20: handling of legacy memory segments.
	// Some games (Crusaders of Might and Magic, the demo and the GOG release) rely on the fact that the
	// program can VirtualAlloc-ate memory on certain tipically free segments (0x4000000, 0x5000000, 
	// 0x6000000, 0x7000000 and 0x8000000) but when the program is hooked by DxWnd these segments could
	// be allocated to extra dlls or allocated memory. 
	// The trick is ti pre-allocate this memory and free it upon error to make it certainly available to
	// the calling program.

	LPVOID ret;
	OutTraceB("VirtualAlloc: lpAddress=%x size=%x flag=%x protect=%x\n", lpAddress, dwSize, flAllocationType, flProtect);
	ret = (*pVirtualAlloc)(lpAddress, dwSize, flAllocationType, flProtect);
	if((ret == NULL) && lpAddress){
		OutTraceE("VirtualAlloc: RECOVERY lpAddress=%x size=%x flag=%x protect=%x\n", 
			lpAddress, dwSize, flAllocationType, flProtect);
		if (((DWORD)lpAddress & 0xF0FFFFFF) == 0){
			BOOL bret;
			bret = VirtualFree(lpAddress, 0x00000000, MEM_RELEASE);
			if(!bret) OutTraceE("VirtualFree: MEM_RELEASE err=%d\n", GetLastError());
			ret = (*pVirtualAlloc)(lpAddress, dwSize, flAllocationType, flProtect);
			if (ret == NULL) OutTraceE("VirtualAlloc: addr=%x err=%d\n", lpAddress, GetLastError());
		}
		if (!ret) ret = (*pVirtualAlloc)((LPVOID)0x00000000, dwSize, flAllocationType, flProtect);
		if(ret == NULL) OutTraceE("VirtualAlloc: addr=NULL err=%d\n", GetLastError());
	}
	OutTrace("VirtualAlloc: ret=%x\n", ret);
	return ret;
}

// WinExec: used by "Star Wars X-Wings Alliance" frontend, but fortunately it's not essential to hook it....
UINT WINAPI extWinExec(LPCSTR lpCmdLine, UINT uCmdShow)
{
	UINT ret;
	OutTraceDW("WinExec: lpCmdLine=%s CmdShow=%x\n", lpCmdLine, uCmdShow);
	ret=(*pWinExec)(lpCmdLine, uCmdShow);
	if(ret<31)
		OutTraceE("WinExec: ERROR ret=%x\n", ret);
	return ret;
}

BOOL WINAPI extSetPriorityClass(HANDLE hProcess, DWORD dwPriorityClass)
{
	OutTraceDW("SetPriorityClass: hProcess=%x class=%x\n", hProcess, dwPriorityClass);
	if(dxw.dwFlags7 & BLOCKPRIORITYCLASS) {
		OutTraceDW("SetPriorityClass: BLOCKED\n");
		return TRUE;
	}
	return (*pSetPriorityClass)(hProcess, dwPriorityClass);
}

BOOL WINAPI extGlobalUnlock(HGLOBAL hMem)
{
	BOOL ret;
	ret = (*pGlobalUnlock)(hMem);
	if((dxw.dwFlags7 & FIXGLOBALUNLOCK) && (ret == 1)){
		static HGLOBAL hLastMem = NULL;
		if(hMem == hLastMem){
			ret = 0;
			OutTraceDW("GlobalUnlock: FIXED RETCODE hMem=%x\n", hMem);
		}
		hLastMem = hMem;
	}
	return ret;
}