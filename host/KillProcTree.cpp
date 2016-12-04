#include "stdafx.h"
#include <windows.h>
#include <tlhelp32.h>

void KillProcessTree(DWORD myprocID)
{
	BOOL bRet = TRUE;
	PROCESSENTRY32 pe;

	memset(&pe, 0, sizeof(PROCESSENTRY32));
	pe.dwSize = sizeof(PROCESSENTRY32);

	HANDLE hSnap = ::CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

	if (::Process32First(hSnap, &pe)){
		do { // recursion 
			if (pe.th32ParentProcessID == myprocID)
				KillProcessTree(pe.th32ProcessID);
		} while(::Process32Next(hSnap, &pe));
	}
		// kill the main process
	HANDLE hProc = ::OpenProcess(PROCESS_ALL_ACCESS, FALSE, myprocID);
	if (hProc){
		::TerminateProcess(hProc, 1);
		::CloseHandle(hProc);
	}
}