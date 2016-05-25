#define _WIN32_WINNT 0x0600
#define WIN32_LEAN_AND_MEAN
#define _CRT_SECURE_NO_DEPRECATE 1

#include <windows.h>
#include <stdio.h>
#include <stdarg.h>
#include "dxwnd.h"
#include "dxwcore.hpp"
#include "dxhelper.h"

extern HHOOK hMouseHook;

/*
typedef struct tagMSG {
  HWND   hwnd;			// A handle to the window whose window procedure receives the message. This member is NULL when the message is a thread message.    
  UINT   message;		// The message identifier. Applications can only use the low word; the high word is reserved by the system. 
  WPARAM wParam;        // Additional information about the message. The exact meaning depends on the value of the message member. 
  LPARAM lParam;        // Additional information about the message. The exact meaning depends on the value of the message member. 
  DWORD  time;			// The time at which the message was posted.
  POINT  pt;            // The cursor position, in screen coordinates, when the message was posted. 
} MSG, *PMSG, *LPMSG;
*/

LRESULT CALLBACK MessageHook(int code, WPARAM wParam, LPARAM lParam)
{
	static BOOL SizeMoving = FALSE;

	if(code == HC_ACTION){
		if(dxw.IsFullScreen()){
			MSG *msg;
			msg = (MSG *)lParam;
			OutTraceC("MessageHook: hwnd=%x message=%d(%s) remove=%d params=(L:%x,R:%x) pt=(%d,%d)\n", 
				msg->hwnd, msg->message, ExplainWinMessage(msg->message), wParam, msg->lParam, msg->wParam, msg->pt.x, msg->pt.y);

			switch(msg->message){
				case WM_ENTERSIZEMOVE: SizeMoving = TRUE; break;
				case WM_EXITSIZEMOVE: SizeMoving = FALSE; break;
			}

			// do not try to fix coordinates for points outside the client area!!
			// in theory, that should primarily depend on the message type, but this is the way it works ...
			// do not do the fixing also when in sizemove mode
			RECT client;
			client=dxw.GetUnmappedScreenRect();
			if ((SizeMoving) ||
				(msg->pt.x < client.left)		||
				(msg->pt.y < client.top)		||
				(msg->pt.x > client.right)		||
				(msg->pt.y > client.bottom)){
				// do nothing
			}
			else {
				// fix the message point coordinates
				POINT upleft={0,0};
				POINT pt;
                HWND hwnd;
                // v2.03.36: offset to be calculated from target window
                // v2-03.70: unless is a thread message (hwnd == NULL) !!
                hwnd=(msg->hwnd)?msg->hwnd:dxw.GethWnd();
                (*pClientToScreen)(hwnd, &upleft);
				pt = msg->pt;
				pt = dxw.SubCoordinates(pt, upleft);
				pt=dxw.FixCursorPos(pt);
				msg->pt = pt;
				// beware: needs fix for mousewheel?
				// if NOMOUSEPROC fix point x,y coordinates only to NON MOUSE messages that won't be fixed by WindowProc hooker
				// NOMOUSEPROC is required by "X-Files", must be off for "Star Trek Armada" !!!
				// if((msg->message <= WM_MOUSELAST) && (msg->message >= WM_MOUSEFIRST) && !(dxw.dwFlags6 & NOMOUSEPROC)) msg->lParam = MAKELPARAM(pt.x, pt.y); 
				if((msg->message <= WM_MOUSELAST) && (msg->message >= WM_MOUSEFIRST)) msg->lParam = MAKELPARAM(pt.x, pt.y); 
				OutTraceC("MessageHook: fixed hwnd=%x lparam/pt=(%d,%d)\n", msg->hwnd, pt.x, pt.y);
				GetHookInfo()->CursorX=(short)pt.x;
				GetHookInfo()->CursorY=(short)pt.y;
			}
		}
	}
	return CallNextHookEx(hMouseHook, code, wParam, lParam);
}
