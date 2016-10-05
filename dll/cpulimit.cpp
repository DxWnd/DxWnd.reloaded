#include <windows.h>
#include <tlhelp32.h>
#include <tchar.h>
#include "dxwnd.h"
#include "dxwcore.hpp"

#define MAX_THREAD_ARRAY 40

// forward declarations
BOOL SlowCpuSpeed(DWORD, DWORD, int);
BOOL LimitCpuUsage(DWORD, DWORD, int);

DWORD WINAPI CpuSlow(LPVOID lpThreadParameter) 
{
	int iSlowDownRatio = dxw.SlowRatio;
	DWORD dwOwnerPID = GetCurrentProcessId();
	DWORD dwOwnerThread = GetCurrentThreadId();
	OutTrace("starting CPUSlow dwOwnerPID=%x Ratio=1:%d\n", dwOwnerPID, iSlowDownRatio);
	if(!dwOwnerPID) return FALSE;
	if(iSlowDownRatio < 1) return FALSE;
	Sleep(100);
	while(TRUE) SlowCpuSpeed(dwOwnerPID, dwOwnerThread, iSlowDownRatio);
	return TRUE;
}

DWORD WINAPI CpuLimit(LPVOID lpThreadParameter) 
{
	int iSlowDownRatio = dxw.SlowRatio;
	DWORD dwOwnerPID = GetCurrentProcessId();
	DWORD dwOwnerThread = GetCurrentThreadId();
	OutTrace("starting CPULimit dwOwnerPID=%x Ratio=1:%d\n", dwOwnerPID, iSlowDownRatio);
	if(!dwOwnerPID) return FALSE;
	if(iSlowDownRatio < 1) return FALSE;
	//while(TRUE) 
	return LimitCpuUsage(dwOwnerPID, dwOwnerThread, iSlowDownRatio);
}

BOOL SlowCpuSpeed(DWORD dwOwnerPID, DWORD dwOwnerThread, int iSlowDownRatio) 
{ 
	HANDLE hThreadSnap = INVALID_HANDLE_VALUE; 
	THREADENTRY32 te32; 
	HANDLE SuspThreads[MAX_THREAD_ARRAY];
	int iThreadIndex, iNumThreads;

	// Take a snapshot of all running threads  
	hThreadSnap = CreateToolhelp32Snapshot( TH32CS_SNAPTHREAD, 0 ); 
	if(hThreadSnap == INVALID_HANDLE_VALUE) {
		OutTrace("LimitCpuUsage: CreateToolhelp32Snapshot ERROR err=%d\n", GetLastError());
		return FALSE;
	}

	// Fill in the size of the structure before using it. 
	te32.dwSize = sizeof(THREADENTRY32); 

	// Retrieve information about the first thread, and exit if unsuccessful
	if(!Thread32First(hThreadSnap, &te32)){
		OutTrace("Thread32First ERROR: err=%d\n", GetLastError());  // Show cause of failure
		CloseHandle(hThreadSnap);     // Must clean up the snapshot object!
		return FALSE;
	}

	// Now walk the thread list of the system,
	// and stop each low-priority thread
	iThreadIndex = 0;
	do { 
		if((te32.th32OwnerProcessID == dwOwnerPID) && (te32.th32ThreadID != dwOwnerThread)){
			if (te32.tpBasePri < THREAD_PRIORITY_TIME_CRITICAL) {
				HANDLE hThread = OpenThread(THREAD_SUSPEND_RESUME, FALSE, te32.th32ThreadID);
				SuspendThread(hThread);
				SuspThreads[iThreadIndex++] = hThread;
			}
		}
	} while(Thread32Next(hThreadSnap, &te32) && (iThreadIndex<MAX_THREAD_ARRAY));
	iNumThreads = iThreadIndex;

	//  Don't forget to clean up the snapshot object.
	CloseHandle(hThreadSnap);

	Sleep(iSlowDownRatio);

	// Now resume all suspended threads
	for(iThreadIndex=0; iThreadIndex<iNumThreads; iThreadIndex++) {
		ResumeThread(SuspThreads[iThreadIndex]);
		CloseHandle(SuspThreads[iThreadIndex]);
	}

	//Sleep(Percentage);
	Sleep(1);
	return TRUE;
}

typedef struct{
	DWORD tid;
	HANDLE hThread;
	//ULARGE_INTEGER LastUsed;
	DWORD LastUsed;
	signed long DeltaUsed;
	BOOL Suspended;
} ThreadDesc_Type;

#define DELTA_TIME 2

BOOL LimitCpuUsage(DWORD dwOwnerPID, DWORD dwOwnerThread, int iSlowDownRatio) 
{ 
	HANDLE hThreadSnap = INVALID_HANDLE_VALUE; 
	THREADENTRY32 te32;
	ThreadDesc_Type ProcessThreads[MAX_THREAD_ARRAY];
	int iThreadIndex, iNumThreads;
	int cycle;
	FILETIME CreationTime, ExitTime, KernelTime, UserTime;
	//ULARGE_INTEGER iKernelTime, iUserTime;
	DWORD iKernelTime, iUserTime;
	DWORD iTimeLimit = (100000 * DELTA_TIME);
	DWORD iTimeSlot = (100000 * DELTA_TIME) /iSlowDownRatio;
	OutTrace("LimitCpuUsage: TimeLimit=%ld TimeSlot=%ld\n", iTimeLimit, iTimeSlot);

	// Fill in the size of the structure before using it. 
	te32.dwSize = sizeof(THREADENTRY32); 
	for(int i=0; i<MAX_THREAD_ARRAY; i++) {
		ProcessThreads[i].LastUsed = 0;
		ProcessThreads[i].DeltaUsed = 0;
		ProcessThreads[i].Suspended = FALSE;
		ProcessThreads[i].hThread = NULL;
		ProcessThreads[i].tid = 0;
	}

	iNumThreads = 0;
	for(cycle=0; TRUE; cycle++){
		if(cycle == 0){
			// Take a snapshot of all running threads  
			hThreadSnap = CreateToolhelp32Snapshot( TH32CS_SNAPTHREAD, 0 ); 
			if(hThreadSnap == INVALID_HANDLE_VALUE) {
				OutTrace("LimitCpuUsage: CreateToolhelp32Snapshot ERROR err=%d\n", GetLastError());
				return FALSE;
			}

			// Retrieve information about the first thread, and exit if unsuccessful
			if(!Thread32First(hThreadSnap, &te32)){
				OutTrace("LimitCpuUsage: Thread32First ERROR err=%d\n", GetLastError());  // Show cause of failure
				CloseHandle(hThreadSnap);     // Must clean up the snapshot object!
				return FALSE;
			}

			iThreadIndex = iNumThreads;
			// Now walk the thread list of the system threads,
			// and take a snapshot of each target low-priority thread
			do { 
				if( (te32.th32OwnerProcessID == dwOwnerPID) && 
					(te32.th32ThreadID != dwOwnerThread) &&
					(te32.tpBasePri < THREAD_PRIORITY_TIME_CRITICAL)) {

					// find threads already listed
					BOOL IsListed = FALSE;
					for(int j=0; j<iNumThreads; j++){
						if(te32.th32ThreadID == ProcessThreads[j].tid){
							IsListed = TRUE;
							break;
						}
					}

					// if not in the list, add
					if(!IsListed){
						HANDLE hThread = OpenThread(THREAD_SUSPEND_RESUME|THREAD_QUERY_INFORMATION, FALSE, te32.th32ThreadID);
						if(!GetThreadTimes(hThread, &CreationTime, &ExitTime, &KernelTime, &UserTime)) continue;
						iUserTime = UserTime.dwLowDateTime;
						iKernelTime = KernelTime.dwLowDateTime;
						ProcessThreads[iThreadIndex].hThread = hThread;
						ProcessThreads[iThreadIndex].LastUsed = iUserTime + iKernelTime;
						//OutTrace("Tid[%d]=%x init time=%d\n", iThreadIndex, hThread, ProcessThreads[iThreadIndex].LastUsed);
						iThreadIndex++;
					}
				}
			} while(Thread32Next(hThreadSnap, &te32) && (iThreadIndex<MAX_THREAD_ARRAY));
			iNumThreads = iThreadIndex;
			//OutTrace("Got %d threads\n", iNumThreads);
		}

		//  Don't forget to clean up the snapshot object.
		CloseHandle(hThreadSnap);

		if(cycle > 100) cycle=0; // every 100 cyces forces a thread list refresh

		Sleep(DELTA_TIME);

		for(iThreadIndex=0; iThreadIndex<iNumThreads; iThreadIndex++) {
			ThreadDesc_Type *t = &ProcessThreads[iThreadIndex];

			if (t->hThread == NULL) continue; // skip terminated ones
			if(!GetThreadTimes(t->hThread, &CreationTime, &ExitTime, &KernelTime, &UserTime)) {
				//OutTrace("Tid[%d]=%x died at %d\n", iThreadIndex, t->hThread, __LINE__);
				//CloseHandle(t->hThread);
				t->hThread = NULL;
				continue;
			}
			if (t->Suspended) {
				t->DeltaUsed -= iTimeSlot;
				//OutTrace("Tid[%d]=%x suspended delta=%ld\n", iThreadIndex, t->hThread, t->DeltaUsed);
				if(t->DeltaUsed < 0) {
					if (ResumeThread(t->hThread)== -1){
						//OutTrace("Tid[%d]=%x died at %d\n", iThreadIndex, t->hThread, __LINE__);
						//CloseHandle(t->hThread);
						t->hThread = NULL;
						continue;
					}
					t->Suspended = FALSE;
				}
			}
			else {
				iUserTime = UserTime.dwLowDateTime;
				iKernelTime = KernelTime.dwLowDateTime;
				t->DeltaUsed = t->DeltaUsed + (iUserTime + iKernelTime) - t->LastUsed;
				t->LastUsed = iUserTime + iKernelTime;
				//OutTrace("Tid[%d]=%x active delta=%ld\n", iThreadIndex, t->hThread, t->DeltaUsed);
				if (t->DeltaUsed > (signed long)iTimeLimit) {
					//OutTrace("Tid[%d]=%x stopped last=%ld\n", iThreadIndex, t->hThread, t->DeltaUsed);
					if (SuspendThread(t->hThread)== -1) {
						//OutTrace("Tid[%d]=%x died at %d\n", iThreadIndex, t->hThread, __LINE__);
						//CloseHandle(t->hThread);
						t->hThread = NULL;
						continue;
					}
					t->Suspended = TRUE;
					t->DeltaUsed -= iTimeSlot;
				}
			}
		}
	} 

	// should never go here ....
	OutTrace("end of cycle\n");
	for(int i=0; i<MAX_THREAD_ARRAY; i++) {
		HANDLE hThread = ProcessThreads[i].hThread;
		if(hThread){
			if(ProcessThreads[i].Suspended) ResumeThread(hThread);
			CloseHandle(hThread);
		}
	}

	return TRUE;
}
