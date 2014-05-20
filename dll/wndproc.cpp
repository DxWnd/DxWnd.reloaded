#define _WIN32_WINNT 0x0600
#define WIN32_LEAN_AND_MEAN
#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <stdio.h>
#include <malloc.h>
#include <ddraw.h>
#include "dxwnd.h"
#include "dxwcore.hpp"
#include "dxhook.h"
#include "syslibs.h"
#include "dxhelper.h"

typedef struct {
	HWND hwnd;
	WNDPROC wndproc;
} wndstack_entry;

#define MAXWNDHSTACK 256

wndstack_entry *WhndStack;
static int WhndTOS = 0;
static int WhndSize = 0;

void WhndStackInit()
{
	WhndSize = MAXWNDHSTACK;
	WhndStack = (wndstack_entry *)malloc(WhndSize * sizeof(wndstack_entry));
}

void WhndStackPush(HWND hwnd, WNDPROC wndproc)
{
	int StackIdx;

	if(WhndTOS == WhndSize){
		WhndSize += MAXWNDHSTACK;
		WhndStack = (wndstack_entry *)realloc(WhndStack, WhndSize * sizeof(wndstack_entry));
		//OutTraceDW("DEBUG: WNDPROC STACK new size=%d\n", WhndSize);
	}
	// wndproc values of 0xFFFFxxxx type seems to be error codes rather than valid callback addresses ....
	// v2.02.36 using CallWindowProc you can pass WinProc handles, so you don't need to eliminate them!
	//if (((DWORD)wndproc & 0xFFFF0000) == 0xFFFF0000) return;
	//OutTraceDW("DEBUG: WNDPROC STACK push hwnd=%x, wndproc=%x\n", hwnd, wndproc);
	// try update first...
	for(StackIdx=0; StackIdx<WhndTOS; StackIdx++) 
		if (WhndStack[StackIdx].hwnd==hwnd) {
			WhndStack[StackIdx].wndproc=wndproc;
			return;
		}
	// push if not already there.
	WhndStack[WhndTOS].hwnd=hwnd;
	WhndStack[WhndTOS].wndproc=wndproc;
	WhndTOS++;
}

WNDPROC WhndGetWindowProc(HWND hwnd)
{
	int StackIdx;
	for(StackIdx=0; StackIdx<WhndTOS; StackIdx++) if (WhndStack[StackIdx].hwnd==hwnd) {
		//OutTraceDW("DEBUG: WNDPROC STACK pop hwnd=%x, wndproc=%x\n", hwnd, WhndStack[StackIdx].wndproc);
		return WhndStack[StackIdx].wndproc;
	}
	//OutTraceDW("DEBUG: WNDPROC STACK pop hwnd=%x, wndproc=NULL\n", hwnd);
	return NULL;
}
