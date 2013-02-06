#include "dxwnd.h"
#include "dxwcore.hpp"
#include "syslibs.h"
#include "dxhook.h"

void HookKernel32(HMODULE module)
{
	void *tmp;

	if(dxw.dwFlags2 & LIMITRESOURCES){
		tmp = HookAPI(module, "kernel32.dll", GetDiskFreeSpaceA, "GetDiskFreeSpaceA", extGetDiskFreeSpaceA);
		if(tmp) pGetDiskFreeSpaceA = (GetDiskFreeSpaceA_Type)tmp;
		tmp = HookAPI(module, "kernel32.dll", GlobalMemoryStatus, "GlobalMemoryStatus", extGlobalMemoryStatus);
		if(tmp) pGlobalMemoryStatus = (GlobalMemoryStatus_Type)tmp;
	}

	if(dxw.dwFlags2 & TIMESTRETCH){
		tmp = HookAPI(module, "kernel32.dll", GetTickCount, "GetTickCount", extGetTickCount);
		if(tmp) pGetTickCount = (GetTickCount_Type)tmp;
		tmp = HookAPI(module, "kernel32.dll", GetLocalTime, "GetLocalTime", extGetLocalTime);
		if(tmp) pGetLocalTime = (GetLocalTime_Type)tmp;
		tmp = HookAPI(module, "kernel32.dll", GetSystemTime, "GetSystemTime", extGetSystemTime);
		if(tmp) pGetSystemTime = (GetSystemTime_Type)tmp;
		tmp = HookAPI(module, "kernel32.dll", GetSystemTimeAsFileTime, "GetSystemTimeAsFileTime", extGetSystemTimeAsFileTime);
		if(tmp) pGetSystemTimeAsFileTime = (GetSystemTimeAsFileTime_Type)tmp;
		tmp = HookAPI(module, "kernel32.dll", Sleep, "Sleep", extSleep);
		if(tmp) pSleep = (Sleep_Type)tmp;
		tmp = HookAPI(module, "kernel32.dll", SleepEx, "SleepEx", extSleepEx);
		if(tmp) pSleepEx = (SleepEx_Type)tmp;
		tmp = HookAPI(module, "user32.dll", SetTimer, "SetTimer", extSetTimer);
		if(tmp) pSetTimer = (SetTimer_Type)tmp;
		tmp = HookAPI(module, "winmm.dll", NULL, "timeGetTime", exttimeGetTime);
		if(tmp) ptimeGetTime = (timeGetTime_Type)tmp;
	}

	if(dxw.dwFlags2 & FAKEVERSION){
		tmp = HookAPI(module, "kernel32.dll", GetVersion, "GetVersion", extGetVersion);
		if(tmp) pGetVersion = (GetVersion_Type)tmp;
		tmp = HookAPI(module, "kernel32.dll", GetVersionEx, "GetVersionEx", extGetVersionEx);
		if(tmp) pGetVersionEx = (GetVersionEx_Type)tmp;
	}
}

FARPROC Remap_kernel32_ProcAddress(LPCSTR proc, HMODULE hModule)
{
	if (!strcmp(proc,"IsDebuggerPresent")){
		OutTraceD("GetProcAddress: hooking proc=%s at addr=%x\n", ProcToString(proc), extIsDebuggerPresent);
		return (FARPROC)extIsDebuggerPresent;
	}
	if(dxw.dwFlags2 & LIMITRESOURCES){
		if (!strcmp(proc,"GetDiskFreeSpaceA")){
			pGetDiskFreeSpaceA=(GetDiskFreeSpaceA_Type)(*pGetProcAddress)(hModule, proc);
			OutTraceD("GetProcAddress: hooking proc=%s at addr=%x\n", ProcToString(proc), extGetDiskFreeSpaceA);
			return (FARPROC)extGetDiskFreeSpaceA;
		}
		if (!strcmp(proc,"GlobalMemoryStatus")){
			pGlobalMemoryStatus=(GlobalMemoryStatus_Type)(*pGetProcAddress)(hModule, proc);
			OutTraceD("GetProcAddress: hooking proc=%s at addr=%x\n", ProcToString(proc), extGlobalMemoryStatus);
			return (FARPROC)extGlobalMemoryStatus;
		}
	}
	if(dxw.dwFlags2 & TIMESTRETCH){
		if (!strcmp(proc,"GetTickCount")){
			pGetTickCount=(GetTickCount_Type)(*pGetProcAddress)(hModule, proc);
			OutTraceD("GetProcAddress: hooking proc=%s at addr=%x\n", ProcToString(proc), extGetTickCount);
			return (FARPROC)extGetTickCount;
		}
		if (!strcmp(proc,"GetLocalTime")){
			pGetLocalTime=(GetLocalTime_Type)(*pGetProcAddress)(hModule, proc);
			OutTraceD("GetProcAddress: hooking proc=%s at addr=%x\n", ProcToString(proc), extGetLocalTime);
			return (FARPROC)extGetLocalTime;
		}
		if (!strcmp(proc,"GetSystemTime")){
			pGetSystemTime=(GetSystemTime_Type)(*pGetProcAddress)(hModule, proc);
			OutTraceD("GetProcAddress: hooking proc=%s at addr=%x\n", ProcToString(proc), extGetSystemTime);
			return (FARPROC)extGetSystemTime;
		}
		if (!strcmp(proc,"GetSystemTimeAsFileTime")){
			pGetSystemTimeAsFileTime=(GetSystemTimeAsFileTime_Type)(*pGetProcAddress)(hModule, proc);
			OutTraceD("GetProcAddress: hooking proc=%s at addr=%x\n", ProcToString(proc), extGetSystemTimeAsFileTime);
			return (FARPROC)extGetSystemTimeAsFileTime;
		}
		if (!strcmp(proc,"Sleep")){
			pSleep=(Sleep_Type)(*pGetProcAddress)(hModule, proc);
			OutTraceD("GetProcAddress: hooking proc=%s at addr=%x\n", ProcToString(proc), extSleep);
			return (FARPROC)extSleep;
		}
		if (!strcmp(proc,"SleepEx")){
			pSleepEx=(SleepEx_Type)(*pGetProcAddress)(hModule, proc);
			OutTraceD("GetProcAddress: hooking proc=%s at addr=%x\n", ProcToString(proc), extSleepEx);
			return (FARPROC)extSleepEx;
		}
		if (!strcmp(proc,"SetTimer")){
			pSetTimer=(SetTimer_Type)(*pGetProcAddress)(hModule, proc);
			OutTraceD("GetProcAddress: hooking proc=%s at addr=%x\n", ProcToString(proc), extSetTimer);
			return (FARPROC)extSetTimer;
		}
	}
	if(dxw.dwFlags2 & FAKEVERSION){
		if (!strcmp(proc,"GetVersion")){
			pGetVersion=(GetVersion_Type)(*pGetProcAddress)(hModule, proc);
			OutTraceD("GetProcAddress: hooking proc=%s at addr=%x\n", ProcToString(proc), extGetVersion);
			return (FARPROC)extGetVersion;
		}
		if (!strcmp(proc,"GetVersionEx")){
			pGetVersionEx=(GetVersionEx_Type)(*pGetProcAddress)(hModule, proc);
			OutTraceD("GetProcAddress: hooking proc=%s at addr=%x\n", ProcToString(proc), extGetVersionEx);
			return (FARPROC)extGetVersionEx;
		}
	}
	return NULL;
}


int WINAPI extIsDebuggerPresent(void)
{
	OutTraceD("extIsDebuggerPresent: return FALSE\n");
	return FALSE;
}

BOOL WINAPI extGetDiskFreeSpaceA(LPCSTR lpRootPathName, LPDWORD lpSectorsPerCluster, LPDWORD lpBytesPerSector, LPDWORD lpNumberOfFreeClusters, LPDWORD lpTotalNumberOfClusters)
{
	BOOL ret;
	OutTraceD("GetDiskFreeSpace: RootPathName=\"%s\"\n", lpRootPathName);
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
	OutTraceD("GlobalMemoryStatus: Length=%x MemoryLoad=%x "
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

BOOL WINAPI extGetVersionEx(LPOSVERSIONINFO lpVersionInfo)
{
	BOOL ret;

	ret=(*pGetVersionEx)(lpVersionInfo);
	if(!ret) {
		OutTraceE("GetVersionEx: ERROR err=%d\n", GetLastError());
		return ret;
	}

	OutTraceD("GetVersionEx: version=%d.%d build=(%d)\n", 
		lpVersionInfo->dwMajorVersion, lpVersionInfo->dwMinorVersion, lpVersionInfo->dwBuildNumber);

	if(dxw.dwFlags2 & FAKEVERSION) {
		// fake Win XP build 0
		lpVersionInfo->dwMajorVersion = WinVersions[dxw.FakeVersionId].bMajor;
		lpVersionInfo->dwMinorVersion = WinVersions[dxw.FakeVersionId].bMinor;
		lpVersionInfo->dwBuildNumber = 0;
		OutTraceD("GetVersionEx: FIXED version=%d.%d build=(%d) os=\"%s\"\n", 
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

	OutTraceD("GetVersion: version=%d.%d build=(%d)\n", dwMajorVersion, dwMinorVersion, dwBuild);

	if(dxw.dwFlags2 & FAKEVERSION) {
		dwVersion = WinVersions[dxw.FakeVersionId].bMajor | (WinVersions[dxw.FakeVersionId].bMinor << 8);
		dwMajorVersion = (DWORD)(LOBYTE(LOWORD(dwVersion)));
	    dwMinorVersion = (DWORD)(HIBYTE(LOWORD(dwVersion)));
		dwBuild = (DWORD)(HIWORD(dwVersion));
		OutTraceD("GetVersion: FIXED version=%d.%d build=(%d) os=\"%s\"\n", 
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

UINT_PTR WINAPI extSetTimer(HWND hWnd, UINT_PTR nIDEvent, UINT uElapse, TIMERPROC lpTimerFunc)
{
	UINT uShiftedElapse;
	// beware: the quicker the time flows, the more the time clicks are incremented,
	// and the lesser the pauses must be lasting! Shift operations are reverted in
	// GetSystemTime vs. Sleep or SetTimer
	uShiftedElapse = dxw.StretchTime(uElapse);
	if (IsDebug) OutTrace("SetTimer: elapse=%d->%d timeshift=%d\n", uElapse, uShiftedElapse, dxw.TimeShift);
	return (*pSetTimer)(hWnd, nIDEvent, uShiftedElapse, lpTimerFunc);
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