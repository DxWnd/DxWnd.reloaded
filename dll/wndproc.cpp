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
	int w;
	int h;
} wndstack_entry;

#define MAXWNDHSTACK 256

wndstack_entry *WhndStack;
static int WhndTOS = 0;
static int WhndSize = 0;

void WinDBInit()
{
	WhndSize = MAXWNDHSTACK;
	WhndStack = (wndstack_entry *)malloc(WhndSize * sizeof(wndstack_entry));
}

void WinDBPut(HWND hwnd, WNDPROC wndproc, int w, int h)
{
	int StackIdx;

	// add extra space when necessary, in chunks of MAXWNDHSTACK entries
	if(WhndTOS == WhndSize){
		WhndSize += MAXWNDHSTACK;
		WhndStack = (wndstack_entry *)realloc(WhndStack, WhndSize * sizeof(wndstack_entry));
	}
	// wndproc values of 0xFFFFxxxx type seems to be error codes rather than valid callback addresses ....
	// v2.02.36 using CallWindowProc you can pass WinProc handles, so you don't need to eliminate them!
	//if (((DWORD)wndproc & 0xFFFF0000) == 0xFFFF0000) return;
	//OutTraceDW("DEBUG: WNDPROC STACK push hwnd=%x, wndproc=%x\n", hwnd, wndproc);
	// try update first...
	for(StackIdx=0; StackIdx<WhndTOS; StackIdx++) 
		if (WhndStack[StackIdx].hwnd==hwnd) {
			// update only valid fields
			if(wndproc) WhndStack[StackIdx].wndproc=wndproc;
			if(w) WhndStack[StackIdx].w=w;
			if(h) WhndStack[StackIdx].h=h;
			return;
		}
	WhndStack[WhndTOS].hwnd=hwnd;
	// initialize ...
	WhndStack[WhndTOS].wndproc=NULL; 
	WhndStack[WhndTOS].w=0;
	WhndStack[WhndTOS].h=0;
	// update only valid fields
	if(wndproc) WhndStack[WhndTOS].wndproc=wndproc;
	if(w) WhndStack[WhndTOS].w=w;
	if(h) WhndStack[WhndTOS].h=h;
	// increment TOS.
	WhndTOS++;
}

void WinDBPutProc(HWND hwnd, WNDPROC wndproc)
{
	int StackIdx;

	// add extra space when necessary, in chunks of MAXWNDHSTACK entries
	if(WhndTOS == WhndSize){
		WhndSize += MAXWNDHSTACK;
		WhndStack = (wndstack_entry *)realloc(WhndStack, WhndSize * sizeof(wndstack_entry));
	}
	// wndproc values of 0xFFFFxxxx type seems to be error codes rather than valid callback addresses ....
	// v2.02.36 using CallWindowProc you can pass WinProc handles, so you don't need to eliminate them!
	//if (((DWORD)wndproc & 0xFFFF0000) == 0xFFFF0000) return;
	//OutTraceDW("DEBUG: WNDPROC STACK push hwnd=%x, wndproc=%x\n", hwnd, wndproc);
	// try update first...
	for(StackIdx=0; StackIdx<WhndTOS; StackIdx++) 
		if (WhndStack[StackIdx].hwnd==hwnd) {
			// update only valid fields
			WhndStack[StackIdx].wndproc=wndproc;
			return;
		}
	WhndStack[WhndTOS].hwnd=hwnd;
	WhndStack[WhndTOS].wndproc=wndproc;
	WhndStack[WhndTOS].w=0; // unknown
	WhndStack[WhndTOS].h=0; // unknown
	// increment TOS.
	WhndTOS++;
}

void WinDBPutSize(HWND hwnd, int w, int h)
{
	int StackIdx;

	// add extra space when necessary, in chunks of MAXWNDHSTACK entries
	if(WhndTOS == WhndSize){
		WhndSize += MAXWNDHSTACK;
		WhndStack = (wndstack_entry *)realloc(WhndStack, WhndSize * sizeof(wndstack_entry));
	}
	// try update first...
	for(StackIdx=0; StackIdx<WhndTOS; StackIdx++) 
		if (WhndStack[StackIdx].hwnd==hwnd) {
			// update only valid fields
			WhndStack[StackIdx].w=w;
			WhndStack[StackIdx].h=h;
			return;
		}
	WhndStack[WhndTOS].hwnd=hwnd;
	WhndStack[WhndTOS].wndproc=NULL; // unknown
	WhndStack[WhndTOS].w=w;
	WhndStack[WhndTOS].h=h;
	// increment TOS.
	WhndTOS++;
}

BOOL WinDBGetSize(HWND hwnd, int *w, int *h)
{
	int StackIdx;
	for(StackIdx=0; StackIdx<WhndTOS; StackIdx++) if (WhndStack[StackIdx].hwnd==hwnd) {
		//OutTraceDW("DEBUG: WNDPROC STACK pop hwnd=%x, wndproc=%x\n", hwnd, WhndStack[StackIdx].wndproc);
		if((WhndStack[StackIdx].w==0) || (WhndStack[StackIdx].h==0)) return FALSE;
		if(w) *w=WhndStack[StackIdx].w;
		if(h) *h=WhndStack[StackIdx].h;
		return TRUE;
	}
	//OutTraceDW("DEBUG: WNDPROC STACK pop hwnd=%x, wndproc=NULL\n", hwnd);
	return FALSE;
}

WNDPROC WinDBGetProc(HWND hwnd)
{
	int StackIdx;
	for(StackIdx=0; StackIdx<WhndTOS; StackIdx++) if (WhndStack[StackIdx].hwnd==hwnd) {
		//OutTraceDW("DEBUG: WNDPROC STACK pop hwnd=%x, wndproc=%x\n", hwnd, WhndStack[StackIdx].wndproc);
		return WhndStack[StackIdx].wndproc; // either a good value, or NULL
	}
	//OutTraceDW("DEBUG: WNDPROC STACK pop hwnd=%x, wndproc=NULL\n", hwnd);
	return NULL;
}
