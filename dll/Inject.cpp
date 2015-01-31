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
	HANDLE hProc, hThread;
	HMODULE hLib;
	char buf[50] = {0};
	LPVOID RemoteString, LoadLibAddy;
	if(!pID) return false;
	//hProc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pID); // not working on Win XP
	hProc = OpenProcess(PROCESS_CREATE_THREAD|PROCESS_QUERY_INFORMATION|PROCESS_VM_OPERATION|PROCESS_VM_READ|PROCESS_VM_WRITE, FALSE, pID);
	if(!hProc)
	{
		sprintf(buf, "OpenProcess() failed: pid=%x err=%d", pID, GetLastError());
		MessageBox(NULL, buf, "Loader", MB_OK);
		return false;
	}
	hLib=GetModuleHandle("kernel32.dll");
	LoadLibAddy = (LPVOID)GetProcAddress(hLib, "LoadLibraryA");
	// Allocate space in the process for the DLL
	RemoteString = (LPVOID)VirtualAllocEx(hProc, NULL, strlen(DLL_NAME), MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
	// Write the string name of the DLL in the memory allocated
	WriteProcessMemory(hProc, (LPVOID)RemoteString, DLL_NAME, strlen(DLL_NAME), NULL);
	// Load the DLL
	hThread=CreateRemoteThread(hProc, NULL, 0, (LPTHREAD_START_ROUTINE)LoadLibAddy, (LPVOID)RemoteString, 0, NULL);
	// Free/Release/Close everything
	VirtualFreeEx(hProc, RemoteString, strlen(DLL_NAME), MEM_RELEASE);
	if(!hThread){
		sprintf(buf, "CreateRemoteThread() failed: pid=%x err=%d", pID, GetLastError());
		MessageBox(NULL, buf, "Loader", MB_OK);
		CloseHandle(hProc);
		return false;
	}
	CloseHandle(hThread);
	CloseHandle(hProc);
	CloseHandle(hLib);
	return true;
}

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