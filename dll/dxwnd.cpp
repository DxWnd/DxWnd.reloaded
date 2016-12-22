/*
DXWnd/dxwnd.cpp
DirectX Hook Module
Copyright(C) 2004-2016 SFB7/GHO

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
#define _CRT_SECURE_NO_WARNINGS

#include <windows.h> 
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>
#include "dxwnd.h"
#include "dxwcore.hpp"

#include "TlHelp32.h"

#define VERSION "2.04.01"

#define DDTHREADLOCK 1

LRESULT CALLBACK HookProc(int ncode, WPARAM wparam, LPARAM lparam);

HINSTANCE hInst;
HHOOK hHook;
HHOOK hMouseHook;
HANDLE hMapping;
TARGETMAP *pMapping;
DXWNDSTATUS *pStatus;
HANDLE hMutex;
HANDLE hTraceMutex;
HANDLE hLockMutex;
HANDLE hDDLockMutex;
HANDLE hKillMutex;
int HookStatus=DXW_IDLE;
static int TaskIndex=-1;
DXWNDSTATUS DxWndStatus;

void InjectHook(); 

BOOL APIENTRY DllMain( HANDLE hmodule, 
                       DWORD  dwreason, 
                       LPVOID preserved
					 )
{
	HANDLE hCurrentThread;

	if(dwreason == DLL_PROCESS_DETACH){
		//if(pInvalidateRect && pGetDesktopWindow) (*pInvalidateRect)((*pGetDesktopWindow)(), NULL, FALSE); // invalidate full desktop, no erase.
		if(pInvalidateRect) (*pInvalidateRect)(0, NULL, FALSE); // invalidate full desktop, no erase.
		UnmapViewOfFile(pMapping);
		CloseHandle(hMapping);
	}

    if(dwreason != DLL_PROCESS_ATTACH) return TRUE;

	hCurrentThread = GetCurrentThread();
	SetThreadPriority(hCurrentThread, THREAD_PRIORITY_HIGHEST); // trick to reduce concurrency problems at program startup

	hInst = (HINSTANCE)hmodule;
	// optimization: disables DLL_THREAD_ATTACH and DLL_THREAD_DETACH notifications for the specified DLL
	DisableThreadLibraryCalls((HMODULE)hmodule);
	hMapping = CreateFileMapping((HANDLE)0xffffffff, NULL, PAGE_READWRITE,
		0, sizeof(DxWndStatus)+sizeof(TARGETMAP)*MAXTARGETS, "UniWind_TargetList");
	if(!hMapping) {
		SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_NORMAL);
		return false;
	}
	// v2.0.2.75: beware: some tasks (namely, Flash player) get dxwnd.dll loaded, but can't create the file mapping
	// this situation has to be intercepted, or it can cause the dll to cause faults that may crash the program.
	pStatus = (DXWNDSTATUS *)MapViewOfFile(hMapping, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(DXWNDSTATUS)+sizeof(TARGETMAP)*MAXTARGETS);
	if(!pStatus) return false;
	pMapping = (TARGETMAP *)((char *)pStatus + sizeof(DXWNDSTATUS));
	hMutex = OpenMutex(MUTEX_ALL_ACCESS, FALSE, "UniWind_Mutex");
	if(!hMutex) hMutex = CreateMutex(0, FALSE, "UniWind_Mutex");
	hTraceMutex = OpenMutex(MUTEX_ALL_ACCESS, FALSE, "Trace_Mutex");
	if(!hTraceMutex) hTraceMutex = CreateMutex(0, FALSE, "Trace_Mutex");
	hLockMutex = OpenMutex(MUTEX_ALL_ACCESS, FALSE, "Lock_Mutex");
	if(!hLockMutex) hLockMutex = CreateMutex(0, FALSE, "Lock_Mutex");
	if(DDTHREADLOCK){
		hDDLockMutex = OpenMutex(MUTEX_ALL_ACCESS, FALSE, "DDLock_Mutex");
		if(!hDDLockMutex) hDDLockMutex = CreateMutex(0, FALSE, "DDLock_Mutex");
	}
	InjectHook();

	SetThreadPriority(hCurrentThread, THREAD_PRIORITY_NORMAL);
	CloseHandle(hCurrentThread);
	return true;
}

static BOOL GetMultiTaskEnabling(){
	char inipath[MAX_PATH];
	GetModuleFileName(GetModuleHandle("dxwnd"), inipath, MAX_PATH);
	inipath[strlen(inipath)-strlen("dxwnd.dll")] = 0; // terminate the string just before "dxwnd.dll"
	strcat(inipath, "dxwnd.ini");
	return GetPrivateProfileInt("window", "multiprocesshook", 0, inipath);
}

int SetTarget(TARGETMAP *targets){
	int i;
	char path[MAX_PATH+1];

	WaitForSingleObject(hMutex, INFINITE);
	pStatus->Status=DXW_IDLE;
	pStatus->IsFullScreen=FALSE;
	pStatus->TaskIdx=0;
	pStatus->hWnd=NULL;
	pStatus->ColorDepth=0;
	memset((void *)&(pStatus->pfd), 0, sizeof(DDPIXELFORMAT));
	pStatus->Height = pStatus->Width = 0;
	pStatus->DXVersion = 0;
	pStatus->AllowMultiTask=GetMultiTaskEnabling();
	for(i = 0; targets[i].path[0]; i ++){
		char *c;
		pMapping[i] = targets[i];
		c = targets[i].path;
		if(*c == '*')
			strcpy(path, targets[i].path);
		else
			GetFullPathName(targets[i].path, MAX_PATH, path, NULL);
		for(c = path; *c; c++) *c = tolower(*c);
		strcpy(pMapping[i].path, path);
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
	UnhookWindowsHookEx(hMouseHook);
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
	char name[MAX_PATH+1];
	HWND hwnd;
	int i;
	static int DoOnce = FALSE;

	// don't do more than once per process
	if(DoOnce) return CallNextHookEx(hHook, ncode, wparam, lparam);

	// take care here: if anything stops or delays the execution logic, the whole
	// operating system hangs, since it can't activate new windows!

	// could use WM_NCCREATE instead of WM_CREATE. Are there differences?
	hwnd = ((CWPSTRUCT *)lparam)->hwnd;
	if(((CWPSTRUCT *)lparam)->message == WM_CREATE){
		int iNameLength;
		name[MAX_PATH]=0; // string terminator
		GetModuleFileName(0, name, MAX_PATH);
		for(i = 0; name[i]; i ++) name[i] = tolower(name[i]);
		iNameLength = strlen(name);
		WaitForSingleObject(hMutex, INFINITE);
		for(i = 0; pMapping[i].path[0] && (i<MAXTARGETS); i++){
			register BOOL bMatched;
			if (!(pMapping[i].flags3 & HOOKENABLED)) continue;
			if(pMapping[i].path[0]=='*')
				bMatched=!strncmp(&name[iNameLength-strlen(pMapping[i].path)+1], &pMapping[i].path[1], iNameLength);
			else
				bMatched=!strncmp(name, pMapping[i].path, iNameLength);
			if(bMatched)
			{
				// V.68 late fix:
				// check for locking thread (and hook) just once per process.
				// This callback is invoked per each process' thread.

				DoOnce = TRUE;
				extern HHOOK hHook;
				UnhookWindowsHookEx(hHook);

				// V.68: concurrency check. One game at a time, or exiting.
				// no good trying to insert fancy dialog boxes: the window
				// isn't ready yet, and the operation fails.

				// V2.03.07: allow multiple process hooking depending on config
				if(!(pStatus->AllowMultiTask)){
					if(WaitForSingleObject(hLockMutex, 0)==WAIT_TIMEOUT){
						ReleaseMutex(hMutex);
						exit(0);
					}
				}
				else
					WaitForSingleObject(hLockMutex, 0);

				pStatus->Status=DXW_RUNNING;
				pStatus->TaskIdx=i;
				pStatus->OrigIdx=pMapping[i].index;
				pStatus->IsFullScreen=FALSE;
				pStatus->hWnd=hwnd;
				pStatus->dwPid=GetProcessId(GetCurrentProcess());
				pStatus->TimeShift=pMapping[i].InitTS;
				pStatus->CursorX = pStatus->CursorY = 0;
				memset((void *)&(pStatus->pfd), 0, sizeof(DDPIXELFORMAT));
				DxWndStatus = *pStatus;
				HookInit(&pMapping[i], hwnd);
			}
		}
		ReleaseMutex(hMutex);
	}
	return CallNextHookEx(hHook, ncode, wparam, lparam);
}

void UnhookProc()
{
	// used to unhook DxWnd from the current process and allow another one (a son) to be managed
	//ReleaseMutex(hMutex);
	ReleaseMutex(hLockMutex);
	UnmapViewOfFile(pMapping);
	CloseHandle(hMapping);
}

void InjectHook()
{
	char name[MAX_PATH+1];
	int i;
	GetModuleFileName(0, name, MAX_PATH);
	name[MAX_PATH]=0; // terminator
	for(char *c = name; *c; c++) *c = tolower(*c);
	for(i = 0; pMapping[i].path[0]; i ++){
			if(pMapping[i].flags3 & HOOKENABLED){
				if(!strncmp(name, pMapping[i].path, strlen(name))){
				if ((pMapping[i].flags2 & STARTDEBUG) || (pMapping[i].flags7 & INJECTSUSPENDED)) {
					HookInit(&pMapping[i],NULL);
					// beware: logging is possible only AFTER HookInit execution
					OutTrace("InjectHook: task[%d]=\"%s\" hooked\n", i, pMapping[i].path);
				}
				break;
			}
		}
	}
}