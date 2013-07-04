/*
DXWnd/dxwnd.cpp
DirectX Hook Module
Copyright(C) 2004-2011 SFB7/GHO

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <windows.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>
#include "dxwnd.h"

#define VERSION "2.02.15"

LRESULT CALLBACK HookProc(int ncode, WPARAM wparam, LPARAM lparam);

HINSTANCE hInst;
HHOOK hHook;
HANDLE hMapping;
TARGETMAP *pMapping;
DXWNDSTATUS *pStatus;
HANDLE hMutex;
HANDLE hTraceMutex;
HANDLE hLockMutex;
HANDLE hKillMutex;
int HookStatus=DXW_IDLE;
static int TaskIndex=-1;
DXWNDSTATUS DxWndStatus;

BOOL APIENTRY DllMain( HANDLE hmodule, 
                       DWORD  dwreason, 
                       LPVOID preserved
					 )
{
	if(dwreason == DLL_PROCESS_DETACH){
		UnmapViewOfFile(pMapping);
		CloseHandle(hMapping);
	}

    if(dwreason != DLL_PROCESS_ATTACH) return TRUE;

	hInst = (HINSTANCE)hmodule;
	hMapping = CreateFileMapping((HANDLE)0xffffffff, NULL, PAGE_READWRITE,
		0, sizeof(DxWndStatus)+sizeof(TARGETMAP)*MAXTARGETS, "UniWind_TargetList");
	pStatus = (DXWNDSTATUS *)MapViewOfFile(hMapping, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(DXWNDSTATUS)+sizeof(TARGETMAP)*MAXTARGETS);
	pMapping = (TARGETMAP *)((char *)pStatus + sizeof(DXWNDSTATUS));
	hMutex = OpenMutex(MUTEX_ALL_ACCESS, FALSE, "UniWind_Mutex");
	if(!hMutex) hMutex = CreateMutex(0, FALSE, "UniWind_Mutex");
	hTraceMutex = OpenMutex(MUTEX_ALL_ACCESS, FALSE, "Trace_Mutex");
	if(!hTraceMutex) hTraceMutex = CreateMutex(0, FALSE, "Trace_Mutex");
	hLockMutex = OpenMutex(MUTEX_ALL_ACCESS, FALSE, "Lock_Mutex");
	if(!hLockMutex) hLockMutex = CreateMutex(0, FALSE, "Lock_Mutex");

	return true;
}

int SetTarget(TARGETMAP *targets){
	int i, j;

	WaitForSingleObject(hMutex, INFINITE);
	pStatus->Status=DXW_IDLE;
	pStatus->IsFullScreen=FALSE;
	pStatus->TaskIdx=0;
	pStatus->hWnd=NULL;
	pStatus->ColorDepth=0;
	pStatus->Height = pStatus->Width = 0;
	pStatus->DXVersion = 0;
	for(i = 0; targets[i].path[0]; i ++){
		//OutTraceD("SetTarget entry %s\n",pMapping[i].path);
		pMapping[i] = targets[i];
		for(j = 0; pMapping[i].path[j]; j ++)
			pMapping[i].path[j] = tolower(pMapping[i].path[j]);
	}
	pMapping[i].path[0] = 0;
	ReleaseMutex(hMutex);
	return i;
}

int StartHook(void)
{
	hHook = SetWindowsHookEx(WH_CALLWNDPROC, HookProc, hInst, 0);
	HookStatus=DXW_ACTIVE;
	return 0;
}

int EndHook(void)
{
	UnhookWindowsHookEx(hHook);
	HookStatus=DXW_IDLE;
	return 0;
}

void GetDllVersion(char *dest)
{
	strcpy_s(dest, strlen(VERSION)+1, VERSION);
}

int GetHookStatus(DXWNDSTATUS *s)
{
	DWORD ret;
	ret=WaitForSingleObject(hLockMutex, 0);
	ReleaseMutex(hLockMutex);
	if(ret==WAIT_TIMEOUT) {
		if (s) *s = *pStatus;
		return DXW_RUNNING;
	}
	return HookStatus;
}

DXWNDSTATUS *GetHookInfo()
{
	return pStatus;
}

void SetFPS(int fps)
{
	pStatus->FPSCount=fps;
}

LRESULT CALLBACK HookProc(int ncode, WPARAM wparam, LPARAM lparam)
{
	char name[MAX_PATH];
	HWND hwnd;
	int i;
	static int DoOnce = FALSE;

	// take care here: if anything stops or delays the execution logic, the whole
	// operating system hangs, since it can't activate new windows!

	// could use WM_NCCREATE insteas of WM_CREATE. Are there differences?
	hwnd = ((CWPSTRUCT *)lparam)->hwnd;
	if(((CWPSTRUCT *)lparam)->message == WM_CREATE){
		GetModuleFileName(0, name, MAX_PATH);
		for(i = 0; name[i]; i ++) name[i] = tolower(name[i]);
		WaitForSingleObject(hMutex, INFINITE);
		for(i = 0; pMapping[i].path[0]; i ++){
			if(!strncmp(name, pMapping[i].path, strlen(name)))
			{
				// V.68 late fix:
				// check for locking thread (and hook) just once per process.
				// This callback is invoked per each process' thread.

				if(DoOnce) break;
				DoOnce = TRUE;

				// V.68: concurrency check. One game at a time, or exiting.
				// no good trying to insert fancy dialog boxes: the window
				// isn't ready yet, and the operation fails.

#ifdef DXWACTIVATESINGLETASK
				if(WaitForSingleObject(hLockMutex, 0)==WAIT_TIMEOUT){
					ReleaseMutex(hMutex);
					exit(0);
				}
				else {
					pStatus->Status=DXW_RUNNING;
					pStatus->TaskIdx=i;
					pStatus->IsFullScreen=FALSE;
					pStatus->hWnd=hwnd;
					pStatus->dwPid=GetProcessId(GetCurrentProcess());
					pStatus->TimeShift=pMapping[i].InitTS;
					DxWndStatus = *pStatus;
					HookInit(&pMapping[i], hwnd);
				}
#else
				pStatus->Status=DXW_RUNNING;
				pStatus->TaskIdx=i;
				pStatus->IsFullScreen=FALSE;
				pStatus->hWnd=hwnd;
				pStatus->dwPid=GetProcessId(GetCurrentProcess());
				DxWndStatus = *pStatus;
				WaitForSingleObject(hLockMutex, 0);
				HookInit(&pMapping[i], hwnd);
#endif				
			}
		}
		ReleaseMutex(hMutex);
	}
	return CallNextHookEx(hHook, ncode, wparam, lparam);
}
