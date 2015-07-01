#define  _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <tlhelp32.h>
#include <shlwapi.h>
#include <conio.h>
#include <stdio.h>

#define WIN32_LEAN_AND_MEAN

#define true 1
#define false 0

#include "Winternl.h"

BOOL Inject(DWORD pID, const char * DLL_NAME)
{
	HANDLE Proc;
	char buf[50] = {0};
	LPVOID RemoteString, LoadLibAddy;
	if(!pID) return false;
	//Proc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pID); // not working on Win XP
	Proc = OpenProcess(PROCESS_CREATE_THREAD|PROCESS_QUERY_INFORMATION|PROCESS_VM_OPERATION|PROCESS_VM_READ|PROCESS_VM_WRITE, FALSE, pID);
	if(!Proc)
	{
		sprintf(buf, "OpenProcess() failed: pid=%x err=%d", pID, GetLastError());
		MessageBox(NULL, buf, "Loader", MB_OK);
		printf(buf);
		return false;
	}
	LoadLibAddy = (LPVOID)GetProcAddress(GetModuleHandle("kernel32.dll"), "LoadLibraryA");
	// Allocate space in the process for our DLL
	RemoteString = (LPVOID)VirtualAllocEx(Proc, NULL, strlen(DLL_NAME), MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
	// Write the string name of our DLL in the memory allocated
	WriteProcessMemory(Proc, (LPVOID)RemoteString, DLL_NAME, strlen(DLL_NAME), NULL);
	// Load our DLL
	if(!CreateRemoteThread(Proc, NULL, 0, (LPTHREAD_START_ROUTINE)LoadLibAddy, (LPVOID)RemoteString, 0, NULL)){
		sprintf(buf, "CreateRemoteThread() failed: pid=%x err=%d", pID, GetLastError());
		MessageBox(NULL, buf, "Loader", MB_OK);
		printf(buf);
		return false;
	}
	CloseHandle(Proc);
	return true;
}

#if 0
DWORD GetTargetThreadIDFromProcName(const char * ProcName)
{
	PROCESSENTRY32 pe;
	HANDLE thSnapShot;
	BOOL retval, ProcFound = false;
	thSnapShot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if(thSnapShot == INVALID_HANDLE_VALUE)
	{
		MessageBox(NULL, "Error: Unable to create toolhelp snapshot!", "2MLoader", MB_OK);
		//printf("Error: Unable to create toolhelp snapshot!");
		return false;
	}
	pe.dwSize = sizeof(PROCESSENTRY32);
	retval = Process32First(thSnapShot, &pe);
	while(retval)
	{
		if(StrStrI(pe.szExeFile, ProcName))
		{
			return pe.th32ProcessID;
		}
		retval = Process32Next(thSnapShot, &pe);
	}
	return 0;
}
#endif

#define STATUS_SUCCESS    ((NTSTATUS)0x000 00000L)
#define ThreadQuerySetWin32StartAddress 9

LPVOID GetThreadStartAddress(HANDLE hThread)
{
    NTSTATUS ntStatus;
    HANDLE hDupHandle;
	HMODULE hLibNTHandle;
    LPVOID dwStartAddress;

	typedef NTSTATUS (WINAPI *NtQueryInformationThread_Type)(HANDLE, THREADINFOCLASS, PVOID, ULONG, PULONG);
	hLibNTHandle = GetModuleHandle("ntdll.dll");
	if(!hLibNTHandle) return 0;
	
	NtQueryInformationThread_Type NtQueryInformationThread = 
		(NtQueryInformationThread_Type)GetProcAddress(hLibNTHandle, "NtQueryInformationThread");

    if(NtQueryInformationThread == NULL) return 0;

    HANDLE hCurrentProcess = GetCurrentProcess();
    if(!DuplicateHandle(hCurrentProcess, hThread, hCurrentProcess, &hDupHandle, THREAD_QUERY_INFORMATION, FALSE, 0)){
        SetLastError(ERROR_ACCESS_DENIED);
        return 0;
    }

    ntStatus = NtQueryInformationThread(hDupHandle, (THREADINFOCLASS)ThreadQuerySetWin32StartAddress, &dwStartAddress, sizeof(DWORD), NULL);
    CloseHandle(hDupHandle);
	CloseHandle(hLibNTHandle);
    //if(ntStatus != STATUS_SUCCESS) return 0;

    return dwStartAddress;
}