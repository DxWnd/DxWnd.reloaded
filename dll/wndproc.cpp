#define _WIN32_WINNT 0x0600
#define WIN32_LEAN_AND_MEAN
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

#define MAXWNDHSTACK 80

wndstack_entry WhndStack[MAXWNDHSTACK];
static int WhndTOS = 0;

void WhndStackInit()
{
}

void WhndStackPush(HWND hwnd, WNDPROC wndproc)
{
	int StackIdx;
	// wndproc values of 0xFFFFxxxx type seems to be error codes rather than valid callback addresses ....
	if (((DWORD)wndproc & 0xFFFF0000) == 0xFFFF0000) return;
	//OutTraceD("DEBUG: WNDPROC STACK push hwnd=%x, wndproc=%x\n", hwnd, wndproc);
	// try update first...
	for(StackIdx=0; StackIdx<WhndTOS; StackIdx++) 
		if (WhndStack[StackIdx].hwnd==hwnd) {
			WhndStack[StackIdx].wndproc=wndproc;
			return;
		}
	// push if not already there.
	if(WhndTOS>=MAXWNDHSTACK) return;
	WhndStack[WhndTOS].hwnd=hwnd;
	WhndStack[WhndTOS].wndproc=wndproc;
	WhndTOS++;
}

WNDPROC WhndGetWindowProc(HWND hwnd)
{
	int StackIdx;
	for(StackIdx=0; StackIdx<MAXWNDHSTACK; StackIdx++) if (WhndStack[StackIdx].hwnd==hwnd) {
		//OutTraceD("DEBUG: WNDPROC STACK pop hwnd=%x, wndproc=%x\n", hwnd, WhndStack[StackIdx].wndproc);
		return WhndStack[StackIdx].wndproc;
	}
	//OutTraceD("DEBUG: WNDPROC STACK pop hwnd=%x, wndproc=NULL\n", hwnd);
	return NULL;
}
