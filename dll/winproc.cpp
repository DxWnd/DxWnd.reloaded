#define _WIN32_WINNT 0x0600
#define WIN32_LEAN_AND_MEAN
#define _CRT_SECURE_NO_DEPRECATE 1

#include <windows.h>
#include <stdio.h>
#include <stdarg.h>
#include "dxwnd.h"
#include "dxwcore.hpp"
#include "dxhelper.h"

extern void SuppressIMEWindow();
extern void RecoverScreenMode();
extern void dx_FullScreenToggle(HWND);
extern void dx_DesktopToggle(HWND, BOOL);

static void dx_ToggleLogging()
{
	// toggle LOGGING
	if(dxw.dwTFlags & OUTTRACE){
		OutTraceDW("Toggle logging OFF\n");
		dxw.dwTFlags &= ~OUTTRACE;
	}
	else {
		dxw.dwTFlags |= OUTTRACE;
		OutTraceDW("Toggle logging ON\n");
	}
	GetHookInfo()->isLogging=(dxw.dwTFlags & OUTTRACE);
}

LRESULT CALLBACK extDialogWindowProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
{
	static int i=0;
	static WINDOWPOS *wp;
	WNDPROC pWindowProc;
	LRESULT res;
	static int t = -1;
	static int iRecursion = 0;

	if(iRecursion) {
		iRecursion=0;
		return NULL;
	}
	iRecursion++;

	if (t == -1)
		t = (*pGetTickCount)();
	int tn = (*pGetTickCount)();

	OutTraceW("DEBUG: DialogWinMsg hwnd=%x msg=[0x%x]%s(%x,%x)\n", hwnd, message, ExplainWinMessage(message), wparam, lparam);

	// optimization: don't invalidate too often!
	// 200mSec seems a good compromise.
	if (tn-t > 200) {
		(*pInvalidateRect)(hwnd, NULL, TRUE);
		t=tn;
	}

	pWindowProc=dxwws.GetProc(hwnd);
	if(pWindowProc) {
		res =(*pCallWindowProcA)(pWindowProc, hwnd, message, wparam, lparam);
	}
	else {
		char *sMsg="ASSERT: DialogWinMsg pWindowProc=NULL !!!\n";
		OutTraceDW(sMsg);
		if (IsAssertEnabled) MessageBox(0, sMsg, "WindowProc", MB_OK | MB_ICONEXCLAMATION);
		res = NULL;
	}
	iRecursion=0;
	return res;
}

LRESULT CALLBACK extChildWindowProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
{
	static int i=0;
	static WINDOWPOS *wp;
	WNDPROC pWindowProc;

	OutTraceW("DEBUG: ChildWinMsg hwnd=%x msg=[0x%x]%s(%x,%x)\n", hwnd, message, ExplainWinMessage(message), wparam, lparam);

	if(dxw.Windowize){
		switch(message){
		// Cybermercs: it seems that all game menus are conveniently handled by the WindowProc routine,
		// while the action screen get messages processed by the ChildWindowProc, that needs some different
		// setting ..........
		// Beware: Cybermercs handles some static info about cursor position handling, so that if you resize
		// a menu it doesn't work correctly until you don't change screen.
		case WM_MOUSEMOVE:
		case WM_MOUSEWHEEL:
		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_LBUTTONDBLCLK:
		case WM_RBUTTONDOWN:
		case WM_RBUTTONUP:
		case WM_RBUTTONDBLCLK:
		case WM_MBUTTONDOWN:
		case WM_MBUTTONUP:
		case WM_MBUTTONDBLCLK:
			// if(dxw.dwFlags1 & MODIFYMOUSE){ // mouse processing  
			//if((dxw.dwFlags1 & MODIFYMOUSE) && !(dxw.dwFlags6 & NOMOUSEPROC)){ // mouse processing  
			if((dxw.dwFlags1 & MODIFYMOUSE) && !(dxw.dwFlags1 & MESSAGEPROC)){ // mouse processing  
				POINT prev, curr;
				// scale mouse coordinates
				prev.x = LOWORD(lparam);
				prev.y = HIWORD(lparam);
				curr = prev;
				if(message == WM_MOUSEWHEEL){ // v2.02.33 mousewheel fix
					POINT upleft={0,0};
					(*pClientToScreen)(dxw.GethWnd(), &upleft);
					curr = dxw.SubCoordinates(curr, upleft);
				}
				//OutTraceC("ChildWindowProc: hwnd=%x pos XY prev=(%d,%d)\n", hwnd, prev.x, prev.y);
				curr=dxw.FixCursorPos(curr); // Warn! the correction must refer to the main window hWnd, not the current hwnd one !!!
				lparam = MAKELPARAM(curr.x, curr.y); 
				OutTraceC("ChildWindowProc: hwnd=%x pos XY=(%d,%d)->(%d,%d)\n", hwnd, prev.x, prev.y, curr.x, curr.y);
			}
			break;	
		default:
			break;
		}
	}

	pWindowProc=dxwws.GetProc(hwnd);
	
	// v2.02.82: use CallWindowProc that handles WinProc handles
	if(pWindowProc) return(*pCallWindowProcA)(pWindowProc, hwnd, message, wparam, lparam);
	// should never get here ....
	OutTraceDW("ChildWindowProc: no WndProc for CHILD hwnd=%x\n", hwnd);
	return DefWindowProc(hwnd, message, wparam, lparam);
}

static void dx_UpdatePositionLock(HWND hwnd)
{
	RECT rect;
	POINT p={0,0};
	(*pGetClientRect)(hwnd,&rect);
	(*pClientToScreen)(hwnd,&p);
	dxw.dwFlags1 |= LOCKWINPOS;
	OutTraceDW("Toggle position lock ON\n");
	dxw.InitWindowPos(p.x, p.y, rect.right-rect.left, rect.bottom-rect.top);
}

static void dx_TogglePositionLock(HWND hwnd)
{
	// toggle position locking
	if(dxw.dwFlags1 & LOCKWINPOS){
		// unlock
		OutTraceDW("Toggle position lock OFF\n");
		dxw.dwFlags1 &= ~LOCKWINPOS;
	}
	else {
		OutTraceDW("Toggle position lock ON\n");
		dxw.dwFlags1 |= LOCKWINPOS;
		dx_UpdatePositionLock(hwnd);
	}
}

static void dx_ToggleFPS()
{
	if(dxw.dwFlags2 & SHOWFPS){
		dxw.dwFlags2 &= ~SHOWFPS;
		OutTrace("ToggleFPS: SHOWFPS mode OFF\n");
	}
	else {
		dxw.dwFlags2 |= SHOWFPS;
		OutTrace("ToggleFPS: SHOWFPS mode ON\n");
	}
}

static void dx_Cornerize(HWND hwnd)
{
	static BOOL bCornerized = FALSE;
	static RECT WinRect = {0, 0, 0, 0};
	static DWORD OldStyle, OldExtStyle;

	if (bCornerized){ 	// toggle ....
		OutTraceDW("DxWnd: exiting corner mode\n");
		(*pSetWindowLong)(hwnd, GWL_STYLE, OldStyle);
		(*pSetWindowLong)(hwnd, GWL_EXSTYLE, OldExtStyle);
		(*pMoveWindow)(hwnd, WinRect.left, WinRect.top, WinRect.right, WinRect.bottom, TRUE);
		memset(&WinRect, 0, sizeof(WinRect));
	}
	else {
		OutTraceDW("DxWnd: entering corner mode\n");
		(*pGetWindowRect)(hwnd, &WinRect);
		OldStyle = (*pGetWindowLong)(hwnd, GWL_STYLE);
		OldExtStyle = (*pGetWindowLong)(hwnd, GWL_EXSTYLE);
		(*pSetWindowLong)(hwnd, GWL_STYLE, WS_VISIBLE|WS_CLIPSIBLINGS|WS_OVERLAPPED);
		(*pSetWindowLong)(hwnd, GWL_EXSTYLE, 0);
		(*pMoveWindow)(hwnd, 0, 0, dxw.GetScreenWidth(), dxw.GetScreenHeight(), TRUE);
	}
	bCornerized = !bCornerized; // switch toggle
	(*pUpdateWindow)(hwnd);
	dxw.ScreenRefresh();
}

LRESULT LastCursorPos;

void ExplainMsg(char *ApiName, HWND hwnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	char sPos[161];
	sPos[160]=0;
	sPos[0]=0;
	switch(Msg){
	case WM_WINDOWPOSCHANGING:
	case WM_WINDOWPOSCHANGED:
		LPWINDOWPOS wp;
		wp = (LPWINDOWPOS)lParam;
		sprintf_s(sPos, 160, " pos=(%d,%d) size=(%dx%d) flags=%x(%s)", wp->x, wp->y, wp->cx, wp->cy, wp->flags, ExplainWPFlags(wp->flags));
		break;
	case WM_MOVE:
		sprintf_s(sPos, 160, " pos=(%d,%d)", HIWORD(lParam), LOWORD(lParam));
		break;
	case WM_SIZE:
		static char *modes[5]={"RESTORED", "MINIMIZED", "MAXIMIZED", "MAXSHOW", "MAXHIDE"};
		sprintf_s(sPos, 160, " mode=SIZE_%s size=(%dx%d)", modes[wParam % 5], HIWORD(lParam), LOWORD(lParam));
		break;	
	}
	OutTrace("%s[%x]: WinMsg=[0x%x]%s(%x,%x) %s\n", ApiName, hwnd, Msg, ExplainWinMessage(Msg), wParam, lParam, sPos);
}

LRESULT CALLBACK extWindowProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
{
	POINT prev, curr;
	RECT rect;
	static int i=0;
	static int ClipCursorToggleState = 1;
	WNDPROC pWindowProc;
	extern void dxwFixWindowPos(char *, HWND, LPARAM);
	extern LPRECT lpClipRegion;
	static BOOL DoOnce = TRUE;
	static BOOL IsToBeLocked;
	static int LastTimeShift;
	static int SaveTimeShift;
	static BOOL TimeShiftToggle=TRUE;
	extern void DDrawScreenShot(int);

	if(DoOnce){
		DoOnce=FALSE;
		IsToBeLocked=(dxw.dwFlags1 & LOCKWINPOS);
		LastTimeShift=SaveTimeShift=dxw.TimeShift;
	}

	// v2.1.93: adjust clipping region

	if(IsTraceW) ExplainMsg("WindowProc", hwnd, message, wparam, lparam);

	if(dxw.dwFlags3 & FILTERMESSAGES){
		switch(message){
		case WM_NCMOUSEMOVE:
		case WM_NCLBUTTONDOWN:
		case WM_NCLBUTTONUP:
		case WM_NCLBUTTONDBLCLK:
		case WM_NCRBUTTONDOWN:
		case WM_NCRBUTTONUP:
		case WM_NCRBUTTONDBLCLK:
		case WM_NCMBUTTONDOWN:
		case WM_NCMBUTTONUP:
		case WM_NCMBUTTONDBLCLK:
			OutTraceDW("WindowProc[%x]: SUPPRESS WinMsg=[0x%x]%s(%x,%x)\n", hwnd, message, ExplainWinMessage(message), wparam, lparam);
			return 0;
		}
	}

	if(LastTimeShift != dxw.TimeShift){
		extern void SetVSyncDelays(int, LPDIRECTDRAW);
		extern LPDIRECTDRAW lpPrimaryDD;
		extern int iBakBufferVersion;
		if(dxw.dwFlags4 & STRETCHTIMERS) dxw.RenewTimers();
		if(lpPrimaryDD) SetVSyncDelays(iBakBufferVersion, lpPrimaryDD);
		LastTimeShift=dxw.TimeShift;
	}

	switch(message){
	// v2.02.13: added WM_GETMINMAXINFO/WM_NCCALCSIZE interception - (see Actua Soccer 3 problems...)
	//case WM_NCDESTROY:
	//	return 0;
	case WM_GETMINMAXINFO: 
		if(dxw.dwFlags1 & LOCKWINPOS){
			extern void dxwFixMinMaxInfo(char *, HWND, LPARAM);
			dxwFixMinMaxInfo("WindowProc", hwnd, lparam);
			return 0;
		}
		break;
	case WM_NCCALCSIZE:
	case WM_NCPAINT:
		if((dxw.dwFlags1 & LOCKWINPOS) && (hwnd == dxw.GethWnd()) && dxw.IsFullScreen()){ // v2.02.30: don't alter child and other windows....
			OutTraceDW("WindowProc: %s wparam=%x\n", ExplainWinMessage(message), wparam);
			return (*pDefWindowProcA)(hwnd, message, wparam, lparam);
		}
		break;
	case WM_NCCREATE:
		if(dxw.dwFlags2 & SUPPRESSIME) SuppressIMEWindow();
		break;
	case WM_IME_SETCONTEXT:
	case WM_IME_NOTIFY:
	case WM_IME_CONTROL:
	case WM_IME_COMPOSITIONFULL:
	case WM_IME_SELECT:
	case WM_IME_CHAR:
	case WM_IME_REQUEST:
	case WM_IME_KEYDOWN:
	case WM_IME_KEYUP:
		if(dxw.dwFlags2 & SUPPRESSIME){
			OutTraceDW("WindowProc[%x]: SUPPRESS IME WinMsg=[0x%x]%s(%x,%x)\n", hwnd, message, ExplainWinMessage(message), wparam, lparam);
			return 0;
		}
		break;
	case WM_NCHITTEST:
		// which one is good ?
		if((dxw.dwFlags2 & FIXNCHITTEST) && (dxw.dwFlags1 & MODIFYMOUSE)){ // mouse processing 
		//if((dxw.dwFlags2 & FIXNCHITTEST) && (dxw.dwFlags1 & MODIFYMOUSE) && !(dxw.dwFlags1 & MESSAGEPROC)){ // mouse processing 
			POINT cursor;
			LRESULT ret;
			ret=(*pDefWindowProcA)(hwnd, message, wparam, lparam);
			if (ret==HTCLIENT) {
				cursor.x=LOWORD(lparam);
				cursor.y=HIWORD(lparam);
				dxw.FixNCHITCursorPos(&cursor);
				lparam = MAKELPARAM(cursor.x, cursor.y); 
				OutTraceC("WindowProc[%x]: fixed WM_NCHITTEST pt=(%d,%d)\n", hwnd, cursor.x, cursor.y);
			}
			else
				return ret;
		}
		break;
	case WM_ERASEBKGND:
		// v2.03.97: fix for Adrenix lost backgrounds, thanks to Riitaoja hunt!
		if(dxw.Windowize && dxw.IsRealDesktop(hwnd)){ 
			OutTraceDW("WindowProc: WM_ERASEBKGND(%x,%x) - suppressed\n", wparam, lparam);
			return 1; // 1 == OK, erased
		}
		break;
	case WM_DISPLAYCHANGE:
		if (dxw.Windowize && (dxw.dwFlags1 & LOCKWINPOS) && dxw.IsFullScreen()){
			OutTraceDW("WindowProc: prevent WM_DISPLAYCHANGE depth=%d size=(%d,%d)\n",
				wparam, LOWORD(lparam), HIWORD(lparam));
			// v2.02.43: unless EMULATESURFACE is set, lock the screen resolution only, but not the color depth!
			if(dxw.dwFlags1 & EMULATESURFACE) return 0;
			// let rparam (color depth) change, but override lparam (screen width & height.)
			lparam = MAKELPARAM((LONG)dxw.GetScreenWidth(), (LONG)dxw.GetScreenHeight());
			//return 0;
		}
		break;
	case WM_WINDOWPOSCHANGING:
	case WM_WINDOWPOSCHANGED:
		if(dxw.Windowize && dxw.IsFullScreen()){
			LPWINDOWPOS wp = (LPWINDOWPOS)lparam;
			extern HWND hControlParentWnd;
			if(dxw.dwFlags5 & NOWINPOSCHANGES){
				OutTraceDW("WindowProc: %s - suppressed\n", message==WM_WINDOWPOSCHANGED ? "WM_WINDOWPOSCHANGED" : "WM_WINDOWPOSCHANGING");
				return 0;
			}
			wp = (LPWINDOWPOS)lparam;
			dxwFixWindowPos("WindowProc", hwnd, lparam);
			OutTraceDW("WindowProc: %s fixed size=(%d,%d)\n", 
				(message == WM_WINDOWPOSCHANGED) ? "WM_WINDOWPOSCHANGED" : "WM_WINDOWPOSCHANGING", wp->cx, wp->cy);
			if(message==WM_WINDOWPOSCHANGED) {
				// try to lock main wind & control parent together
				if(dxw.IsDesktop(hwnd) && hControlParentWnd) {
					POINT fo = dxw.GetFrameOffset();
					(*pMoveWindow)(hControlParentWnd, wp->x+fo.x, wp->y+fo.y, wp->cx, wp->cy, TRUE);
				}
				// v2.03.30: in window mode, it seems that the WM_ACTIVATEAPP message is not sent to the main win.
				// this PostMessage call recovers "Thorgal" block at the end of intro movie and "Championship Manager 03 04" cursor
				if (dxw.dwFlags6 & ACTIVATEAPP){
					PostMessage(hwnd, WM_ACTIVATEAPP, 1, 0);
				}
				// v2.03.91.fx4: keep position coordinates updated!
				if(!(wp->flags & (SWP_NOMOVE|SWP_NOSIZE))) dxw.UpdateDesktopCoordinates();
			}
		}
		break;
	case WM_ENTERSIZEMOVE:
		if(IsToBeLocked){
			dxw.dwFlags1 &= ~LOCKWINPOS;
		}
		while((*pShowCursor)(1) < 0);
		if(dxw.dwFlags1 & CLIPCURSOR) dxw.EraseClipCursor();
		if(dxw.dwFlags1 & ENABLECLIPPING) (*pClipCursor)(NULL);
		break;
	case WM_EXITSIZEMOVE:
		if(IsToBeLocked){
			dxw.dwFlags1 |= LOCKWINPOS;
			dx_UpdatePositionLock(hwnd);
		}
		if((dxw.dwFlags1 & HIDEHWCURSOR) && dxw.IsFullScreen()) while((*pShowCursor)(0) >= 0);
		if(dxw.dwFlags2 & SHOWHWCURSOR) while((*pShowCursor)(1) < 0);
		if(dxw.dwFlags1 & ENABLECLIPPING) extClipCursor(lpClipRegion);
		if(dxw.dwFlags2 & REFRESHONRESIZE) dxw.ScreenRefresh();
		if(dxw.dwFlags4 & HIDEDESKTOP) dxw.HideDesktop(dxw.GethWnd());
		if(dxw.dwFlags5 & CENTERTOWIN) {
			HDC thdc;
			HWND w = dxw.GethWnd();
			RECT client;
			(*pGetClientRect)(w, &client);
			thdc=(*pGDIGetDC)(w);
			if(thdc) (*pGDIBitBlt)(thdc, client.left, client.top, client.right, client.bottom, 0, 0, 0, BLACKNESS);
		}
		dxw.UpdateDesktopCoordinates();
		break;
	case WM_ACTIVATE:
		// turn DirectInput bActive flag on & off .....
		dxw.bActive = (LOWORD(wparam) == WA_ACTIVE || LOWORD(wparam) == WA_CLICKACTIVE) ? 1 : 0;
	case WM_NCACTIVATE:
		// turn DirectInput bActive flag on & off .....
		if(message == WM_NCACTIVATE) dxw.bActive = wparam;
		if(dxw.bActive) (*pSetWindowPos)(hwnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
		if(dxw.dwFlags6 & UNACQUIRE){
			ToggleAcquiredDevices(dxw.bActive);
		}
		if(dxw.dwFlags1 & UNNOTIFY){
			DefWindowProc(hwnd, message, wparam, lparam);
			return false;
		}
		break;
	case WM_NCMOUSEMOVE:
		// Posted to a window when the cursor is moved within the nonclient area of the window. 
		// This message is posted to the window that contains the cursor. 
		// If a window has captured the mouse, this message is not posted.
		// V2.1.90: on nonclient areas the cursor is always shown.
		while((*pShowCursor)(1) < 0);
		break;
	case WM_MOUSEMOVE:
		if(dxw.Windowize){
			prev.x = LOWORD(lparam);
			prev.y = HIWORD(lparam);
			if(dxw.IsFullScreen()){
				if (dxw.dwFlags1 & HIDEHWCURSOR){
					(*pGetClientRect)(hwnd, &rect);
					if(prev.x >= 0 && prev.x < rect.right && prev.y >= 0 && prev.y < rect.bottom)
						while((*pShowCursor)(0) >= 0);
					else
						while((*pShowCursor)(1) < 0);
				}
				if (dxw.dwFlags1 & SHOWHWCURSOR){
					while((*pShowCursor)(1) < 0);
				}
			}
			if((dxw.dwFlags1 & MODIFYMOUSE) && !(dxw.dwFlags1 & MESSAGEPROC)){ // mouse processing  
				// scale mouse coordinates
				curr=dxw.FixCursorPos(prev); //v2.02.30
				lparam = MAKELPARAM(curr.x, curr.y); 
				OutTraceC("WindowProc: hwnd=%x pos XY=(%d,%d)->(%d,%d)\n", hwnd, prev.x, prev.y, curr.x, curr.y);
			}
			GetHookInfo()->CursorX=LOWORD(lparam);
			GetHookInfo()->CursorY=HIWORD(lparam);
		}
		break;	
	// fall through cases:
	case WM_MOUSEWHEEL:
	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_LBUTTONDBLCLK:
	case WM_RBUTTONDOWN:
	case WM_RBUTTONUP:
	case WM_RBUTTONDBLCLK:
	case WM_MBUTTONDOWN:
	case WM_MBUTTONUP:
	case WM_MBUTTONDBLCLK:
		if(dxw.Windowize){
			if((dxw.dwFlags1 & CLIPCURSOR) && ClipCursorToggleState) dxw.SetClipCursor();
			if((dxw.dwFlags1 & MODIFYMOUSE) && !(dxw.dwFlags1 & MESSAGEPROC)){ // mouse processing  
				// scale mouse coordinates
				prev.x = LOWORD(lparam);
				prev.y = HIWORD(lparam);
				curr = prev;
				if(message == WM_MOUSEWHEEL){ // v2.02.33 mousewheel fix
					POINT upleft={0,0};
					(*pClientToScreen)(dxw.GethWnd(), &upleft);
					curr = dxw.SubCoordinates(curr, upleft);
				}
				curr=dxw.FixCursorPos(curr); //v2.02.30
				lparam = MAKELPARAM(curr.x, curr.y); 
				OutTraceC("WindowProc: hwnd=%x pos XY=(%d,%d)->(%d,%d)\n", hwnd, prev.x, prev.y, curr.x, curr.y);
			}
			GetHookInfo()->CursorX=LOWORD(lparam);
			GetHookInfo()->CursorY=HIWORD(lparam);
		}
		break;	
	case WM_SETFOCUS:
		OutTraceDW("WindowProc: hwnd=%x GOT FOCUS\n", hwnd);
		if(dxw.dwFlags1 & CLIPCURSOR) dxw.SetClipCursor();
		if (dxw.dwFlags1 & ENABLECLIPPING) extClipCursor(lpClipRegion);
		break;
	case WM_KILLFOCUS:
		OutTraceDW("WindowProc: hwnd=%x LOST FOCUS\n", hwnd);
		if (dxw.dwFlags1 & CLIPCURSOR) dxw.EraseClipCursor();
		if (dxw.dwFlags1 & ENABLECLIPPING) (*pClipCursor)(NULL);
		break;
	case WM_SYSCOMMAND:
		// v2.03.56.fix1 by FunkyFr3sh: ensure that "C&C Red Alert 2" receives the WM_SYSCOMMAND / SC_CLOSE message
		// that likely is filtered by the application logic
		// v2.03.91: from msdn - In WM_SYSCOMMAND messages, the four low-order bits of the wParam parameter are used 
		// internally by the system. To obtain the correct result when testing the value of wParam, an application 
		// must combine the value 0xFFF0 with the wParam value by using the bitwise AND operator. 
		if(dxw.Windowize && ((wparam & 0xFFF0)== SC_CLOSE) && (dxw.dwFlags6 & TERMINATEONCLOSE)) {
			LRESULT lres;
			lres = (*pDefWindowProcA)(hwnd, message, wparam, lparam);
			return lres;
		}
		if(dxw.Windowize){
			static int iLastX, iLastY, iLastW, iLastH;
			switch(wparam & 0xFFF0){
				case SC_MINIMIZE:
					dxw.IsVisible = FALSE;
					iLastX = dxw.iPosX; iLastY = dxw.iPosY;
					iLastW = dxw.iSizX; iLastH = dxw.iSizY;
					dxw.iPosX = dxw.iPosY = dxw.iSizX = dxw.iSizY = 0;
					break;
				case SC_RESTORE:
					dxw.IsVisible = TRUE;
					dxw.iPosX = iLastX; dxw.iPosY = iLastY;
					dxw.iSizX = iLastW; dxw.iSizY = iLastH;
					break;
				case SC_MAXIMIZE:
					dxw.IsVisible = TRUE;
					break;
			}
		}
		break;
	case WM_CLOSE:
		// Beware: closing main window does not always mean that the program is about to terminate!!!
		extern void gShowHideTaskBar(BOOL);
		if(dxw.dwFlags6 & CONFIRMONCLOSE){
			OutTraceDW("WindowProc: WM_CLOSE - terminating process?\n");
			if (MessageBoxA(NULL, "Do you really want to exit the game?", "DxWnd", MB_YESNO | MB_TASKMODAL) != IDYES) return FALSE;
		}			
		if(dxw.dwFlags6 & HIDETASKBAR) gShowHideTaskBar(FALSE);
		if(dxw.dwFlags3 & FORCE16BPP) RecoverScreenMode();
		if(dxw.dwFlags6 & TERMINATEONCLOSE) TerminateProcess(GetCurrentProcess(),0);
		break;
	case WM_SYSKEYDOWN:
		if ((dxw.dwFlags1 & HANDLEALTF4) && (wparam == VK_F4)) {
			OutTraceDW("WindowProc: WM_SYSKEYDOWN(ALT-F4) - terminating process\n");
			TerminateProcess(GetCurrentProcess(),0);
		}
		// fall through
	case WM_KEYDOWN:
		if(!(dxw.dwFlags4 & ENABLEHOTKEYS)) break;
		OutTraceW("event %s wparam=%x lparam=%x\n", (message==WM_SYSKEYDOWN)?"WM_SYSKEYDOWN":"WM_KEYDOWN", wparam, lparam);
		UINT DxWndKey;
		DxWndKey=dxw.MapKeysConfig(message, lparam, wparam);
		switch (DxWndKey){
		case DXVK_CLIPTOGGLE:
			if(dxw.dwFlags1 & CLIPCURSOR){
				OutTraceDW("WindowProc: WM_SYSKEYDOWN key=%x ToggleState=%x\n",wparam,ClipCursorToggleState);
				ClipCursorToggleState = !ClipCursorToggleState;
				ClipCursorToggleState ? dxw.SetClipCursor() : dxw.EraseClipCursor();
			}
			break;
		case DXVK_REFRESH:
			dxw.ScreenRefresh();
			break;
		case DXVK_LOGTOGGLE:
			dx_ToggleLogging();
			break;
		case DXVK_PLOCKTOGGLE:
			dx_TogglePositionLock(hwnd);
			break;
		case DXVK_FPSTOGGLE:
			dx_ToggleFPS();
			break;
		//case DXVK_FREEZETIME:
		//	dxw.ToggleFreezedTime();
		//	break;
		case DXVK_TIMEFAST:
		case DXVK_TIMESLOW:
			if (dxw.dwFlags2 & TIMESTRETCH) {
				if (DxWndKey == DXVK_TIMESLOW && (dxw.TimeShift <  8)) dxw.TimeShift++;
				if (DxWndKey == DXVK_TIMEFAST && (dxw.TimeShift > -8)) dxw.TimeShift--;
				GetHookInfo()->TimeShift=dxw.TimeShift;
				OutTrace("Time Stretch: shift=%d speed=%s\n", dxw.TimeShift, dxw.GetTSCaption());
			}
			break;
		case DXVK_TIMETOGGLE:
			if (dxw.dwFlags2 & TIMESTRETCH) {
				if(TimeShiftToggle){
					SaveTimeShift=dxw.TimeShift;
					dxw.TimeShift=0;
				}
				else{
					dxw.TimeShift=SaveTimeShift;
				}
				TimeShiftToggle = !TimeShiftToggle;
				GetHookInfo()->TimeShift=dxw.TimeShift;
		}
			break;
		case DXVK_ALTF4:
			if (dxw.dwFlags1 & HANDLEALTF4) {
				OutTraceDW("WindowProc: WM_SYSKEYDOWN(virtual Alt-F4) - terminating process\n");
				TerminateProcess(GetCurrentProcess(),0);
			}
			break;
		case DXVK_PRINTSCREEN:
			DDrawScreenShot(dxw.dwDDVersion);
			break;
		case DXVK_CORNERIZE:
			dx_Cornerize(hwnd);
			break;
		case DXVK_FULLSCREEN:
			dx_FullScreenToggle(hwnd);
			break;
		case DXVK_FAKEDESKTOP:
			dx_DesktopToggle(hwnd, FALSE);
			break;
		case DXVK_FAKEWORKAREA:
			dx_DesktopToggle(hwnd, TRUE);
		default:
			break;
		}
	default:
		break;
	}
	if (dxw.dwFlags1 & AUTOREFRESH) dxw.ScreenRefresh();

	pWindowProc=dxwws.GetProc(hwnd);
	//OutTraceB("WindowProc: pWindowProc=%x extWindowProc=%x message=%x(%s) wparam=%x lparam=%x\n", 
	//	(*pWindowProc), extWindowProc, message, ExplainWinMessage(message), wparam, lparam);
	if(pWindowProc) {
		LRESULT ret;

		// v2.02.36: use CallWindowProc that handles WinProc handles
		ret=(*pCallWindowProcA)(pWindowProc, hwnd, message, wparam, lparam);


		switch(message){
			case WM_SIZE:
			//case WM_WINDOWPOSCHANGED: - no good!!!!
				// update new coordinates
				if (dxw.IsFullScreen()) dxw.UpdateDesktopCoordinates();
				break;
			case WM_NCHITTEST:
				// save last NCHITTEST cursor position for use with KEEPASPECTRATIO scaling
				LastCursorPos=ret;
				break;
		}

		// v2.1.89: if FORCEWINRESIZE add standard processing for the missing WM_NC* messages
		if(dxw.dwFlags2 & FORCEWINRESIZE){ 
			switch(message){
			//case WM_NCHITTEST:
			//case WM_NCPAINT:
			//case WM_NCMOUSEMOVE:
			//case WM_NCCALCSIZE:
			//case WM_NCACTIVATE:
			case WM_SETCURSOR:		// shows a different cursor when moving on borders
			case WM_NCLBUTTONDOWN:	// intercepts mouse down on borders
			case WM_NCLBUTTONUP:	// intercepts mouse up on borders
				ret=(*pDefWindowProcA)(hwnd, message, wparam, lparam);
				break;
			}
		}

		return ret;
	}

	//OutTraceDW("ASSERT: WindowProc mismatch hwnd=%x\n", hwnd);
	// ??? maybe it's a normal condition, whenever you don't have a WindowProc routine
	// like in Commandos 2. Flag it?
	char sMsg[81];
	sprintf(sMsg,"ASSERT: WindowProc mismatch hwnd=%x\n", hwnd);
	OutTraceDW(sMsg);
	if (IsAssertEnabled) MessageBox(0, sMsg, "WindowProc", MB_OK | MB_ICONEXCLAMATION);	
	return (*pDefWindowProcA)(hwnd, message, wparam, lparam);
}