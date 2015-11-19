#define _CRT_SECURE_NO_WARNINGS 
#define _CRT_NON_CONFORMING_SWPRINTFS

#include "dxwnd.h"
#include "dxwcore.hpp"
#include "syslibs.h"
#include "dxhook.h"
#include "dxhelper.h"
#include "resource.h"

#include "MMSystem.h"
#include <stdio.h>

#define SUPPRESSMCIERRORS FALSE
#define EMULATEJOY TRUE
#define INVERTJOYAXIS TRUE

BOOL IsWithinMCICall = FALSE;

typedef MCIDEVICEID (WINAPI *mciGetDeviceIDA_Type)(LPCTSTR);
mciGetDeviceIDA_Type pmciGetDeviceIDA = NULL;
MCIDEVICEID WINAPI extmciGetDeviceIDA(LPCTSTR);
typedef MCIDEVICEID (WINAPI *mciGetDeviceIDW_Type)(LPCWSTR);
mciGetDeviceIDW_Type pmciGetDeviceIDW = NULL;
MCIDEVICEID WINAPI extmciGetDeviceIDW(LPCWSTR);
typedef DWORD (WINAPI *joyGetNumDevs_Type)(void);
joyGetNumDevs_Type pjoyGetNumDevs = NULL;
DWORD WINAPI extjoyGetNumDevs(void);
typedef MMRESULT (WINAPI *joyGetDevCapsA_Type)(DWORD, LPJOYCAPS, UINT);
joyGetDevCapsA_Type pjoyGetDevCapsA = NULL;
MMRESULT WINAPI extjoyGetDevCapsA(DWORD, LPJOYCAPS, UINT);
typedef MMRESULT (WINAPI *joyGetPosEx_Type)(DWORD, LPJOYINFOEX);
joyGetPosEx_Type pjoyGetPosEx = NULL;
MMRESULT WINAPI extjoyGetPosEx(DWORD, LPJOYINFOEX);

static HookEntry_Type Hooks[]={
	{HOOK_IAT_CANDIDATE, "mciSendCommandA", NULL, (FARPROC *)&pmciSendCommandA, (FARPROC)extmciSendCommandA},
	{HOOK_IAT_CANDIDATE, "mciSendCommandW", NULL, (FARPROC *)&pmciSendCommandW, (FARPROC)extmciSendCommandW},
	{HOOK_HOT_CANDIDATE, "mciGetDeviceIDA", NULL, (FARPROC *)&pmciGetDeviceIDA, (FARPROC)extmciGetDeviceIDA},
	{HOOK_HOT_CANDIDATE, "mciGetDeviceIDW", NULL, (FARPROC *)&pmciGetDeviceIDW, (FARPROC)extmciGetDeviceIDW},
	{HOOK_IAT_CANDIDATE, 0, NULL, 0, 0} // terminator
};

static HookEntry_Type TimeHooks[]={
	{HOOK_HOT_CANDIDATE, "timeGetTime", NULL, (FARPROC *)&ptimeGetTime, (FARPROC)exttimeGetTime},
	{HOOK_HOT_CANDIDATE, "timeKillEvent", NULL, (FARPROC *)&ptimeKillEvent, (FARPROC)exttimeKillEvent},
	{HOOK_HOT_CANDIDATE, "timeSetEvent", NULL, (FARPROC *)&ptimeSetEvent, (FARPROC)exttimeSetEvent},
	{HOOK_IAT_CANDIDATE, 0, NULL, 0, 0} // terminator
};

static HookEntry_Type RemapHooks[]={
	{HOOK_HOT_CANDIDATE, "mciSendStringA", NULL, (FARPROC *)&pmciSendStringA, (FARPROC)extmciSendStringA},
	{HOOK_HOT_CANDIDATE, "mciSendStringW", NULL, (FARPROC *)&pmciSendStringW, (FARPROC)extmciSendStringW},
	{HOOK_IAT_CANDIDATE, 0, NULL, 0, 0} // terminator
};

static HookEntry_Type JoyHooks[]={
	{HOOK_IAT_CANDIDATE, "joyGetNumDevs", NULL, (FARPROC *)&pjoyGetNumDevs, (FARPROC)extjoyGetNumDevs},
	{HOOK_IAT_CANDIDATE, "joyGetDevCapsA", NULL, (FARPROC *)&pjoyGetDevCapsA, (FARPROC)extjoyGetDevCapsA},
	{HOOK_IAT_CANDIDATE, "joyGetPosEx", NULL, (FARPROC *)&pjoyGetPosEx, (FARPROC)extjoyGetPosEx},
	{HOOK_IAT_CANDIDATE, 0, NULL, 0, 0} // terminator
};

void HookWinMM(HMODULE module, char *libname)
{
	HookLibrary(module, Hooks, libname);
	if(dxw.dwFlags2 & TIMESTRETCH) HookLibrary(module, TimeHooks, libname);
	if(dxw.dwFlags5 & REMAPMCI) HookLibrary(module, RemapHooks, libname);
	if(dxw.dwFlags6 & VIRTUALJOYSTICK) HookLibrary(module, JoyHooks, libname);
}

FARPROC Remap_WinMM_ProcAddress(LPCSTR proc, HMODULE hModule)
{
	FARPROC addr;

	if (addr=RemapLibrary(proc, hModule, Hooks)) return addr;
	if(dxw.dwFlags2 & TIMESTRETCH)
		if (addr=RemapLibrary(proc, hModule, TimeHooks)) return addr;
	if(dxw.dwFlags5 & REMAPMCI)
		if (addr=RemapLibrary(proc, hModule, RemapHooks)) return addr;
	if(dxw.dwFlags6 & VIRTUALJOYSTICK)
		if (addr=RemapLibrary(proc, hModule, JoyHooks)) return addr;

	return NULL;
}

DWORD WINAPI exttimeGetTime(void)
{
	DWORD ret;
	ret = dxw.GetTickCount();
	if (IsDebug) OutTrace("timeGetTime: time=%x\n", ret);
	return ret;
}

MMRESULT WINAPI exttimeSetEvent(UINT uDelay, UINT uResolution, LPTIMECALLBACK lpTimeProc, DWORD_PTR dwUser, UINT fuEvent)
{
	MMRESULT res;
	UINT NewDelay;
	OutTraceDW("timeSetEvent: Delay=%d Resolution=%d Event=%x\n", uDelay, uResolution, fuEvent);
	if(dxw.dwFlags4 & STRETCHTIMERS) NewDelay = dxw.StretchTime(uDelay);
	else NewDelay = uDelay;
	res=(*ptimeSetEvent)(NewDelay, uResolution, lpTimeProc, dwUser, fuEvent);
	if(res) dxw.PushTimer(res, uDelay, uResolution, lpTimeProc, dwUser, fuEvent);
	OutTraceDW("timeSetEvent: ret=%x\n", res);
	return res;
}

MMRESULT WINAPI exttimeKillEvent(UINT uTimerID)
{
	MMRESULT res;
	OutTraceDW("timeKillEvent: TimerID=%x\n", uTimerID);
	res=(*ptimeKillEvent)(uTimerID);
	if(res==TIMERR_NOERROR) dxw.PopTimer(uTimerID);
	OutTraceDW("timeKillEvent: ret=%x\n", res);
	return res;
}

/* MCI_DGV_PUT_FRAME

    The rectangle defined for MCI_DGV_RECT applies to the frame rectangle. 
	The frame rectangle specifies the portion of the frame buffer used as the destination of the video images obtained from the video rectangle. 
	The video should be scaled to fit within the frame buffer rectangle.
    The rectangle is specified in frame buffer coordinates. 
	The default rectangle is the full frame buffer. 
	Specifying this rectangle lets the device scale the image as it digitizes the data. 
	Devices that cannot scale the image reject this command with MCIERR_UNSUPPORTED_FUNCTION. 
	You can use the MCI_GETDEVCAPS_CAN_STRETCH flag with the MCI_GETDEVCAPS command to determine if a device scales the image. A device returns FALSE if it cannot scale the image.
*/

MCIERROR WINAPI extmciSendCommand(mciSendCommand_Type pmciSendCommand, MCIDEVICEID IDDevice, UINT uMsg, DWORD_PTR fdwCommand, DWORD_PTR dwParam)
{
	RECT saverect;
	MCIERROR ret;
	MCI_ANIM_RECT_PARMS *pr;
	MCI_OVLY_WINDOW_PARMSW *pw;

	OutTraceDW("mciSendCommand: IDDevice=%x msg=%x(%s) Command=%x(%s)\n",
		IDDevice, uMsg, ExplainMCICommands(uMsg), fdwCommand, ExplainMCIFlags(uMsg, fdwCommand));

	if(dxw.dwFlags6 & BYPASSMCI){
		if((uMsg == MCI_STATUS) && (fdwCommand == MCI_STATUS_ITEM)){
			// fix for Tie Fighter 95: when bypassing, let the caller know you have no CD tracks
			// otherwise you risk an almost endless loop going through the unassigned returned 
			// number of ghost tracks
			MCI_STATUS_PARMS *p = (MCI_STATUS_PARMS *)dwParam;
			p->dwItem = 0;
			p->dwTrack = 0; 
			p->dwReturn = 0;
			OutTraceDW("mciSendCommand: BYPASS fixing MCI_STATUS\n");
		}
		else{
			OutTraceDW("mciSendCommand: BYPASS\n");
		}
		return 0;
	}

	if(dxw.IsFullScreen()){
		switch(uMsg){
		case MCI_WINDOW:
			pw = (MCI_OVLY_WINDOW_PARMSW *)dwParam;
			OutTraceB("mciSendCommand: hwnd=%x CmdShow=%x\n",
				pw->hWnd, pw->nCmdShow);
			//fdwCommand |= MCI_ANIM_WINDOW_ENABLE_STRETCH;
			//fdwCommand &= ~MCI_ANIM_WINDOW_DISABLE_STRETCH;
			if(dxw.IsRealDesktop(pw->hWnd)) {
				pw->hWnd = dxw.GethWnd();
				OutTraceB("mciSendCommand: REDIRECT hwnd=%x\n", pw->hWnd);
			}
			break;
		case MCI_PUT:
			RECT client;
			pr = (MCI_ANIM_RECT_PARMS *)dwParam;
			OutTraceB("mciSendCommand: rect=(%d,%d),(%d,%d)\n",
				pr->rc.left, pr->rc.top, pr->rc.right, pr->rc.bottom);
			(*pGetClientRect)(dxw.GethWnd(), &client);
			//fdwCommand |= MCI_ANIM_PUT_DESTINATION;
			fdwCommand |= MCI_ANIM_RECT;
			saverect=pr->rc;
			pr->rc.top = (pr->rc.top * client.bottom) / dxw.GetScreenHeight();
			pr->rc.bottom = (pr->rc.bottom * client.bottom) / dxw.GetScreenHeight();
			pr->rc.left = (pr->rc.left * client.right) / dxw.GetScreenWidth();
			pr->rc.right = (pr->rc.right * client.right) / dxw.GetScreenWidth();
			OutTraceB("mciSendCommand: fixed rect=(%d,%d),(%d,%d)\n",
				pr->rc.left, pr->rc.top, pr->rc.right, pr->rc.bottom);
			break;
		case MCI_PLAY:
			if(dxw.dwFlags6 & NOMOVIES) return 0; // ???
			break;
		case MCI_OPEN:
			if(dxw.dwFlags6 & NOMOVIES) return 275; // quite brutal, but working ....
			break;
		}
	}

	ret=(*pmciSendCommand)(IDDevice, uMsg, fdwCommand, dwParam);

	if(ret == 0){
		switch(uMsg){
		case MCI_STATUS:
			MCI_STATUS_PARMS *p = (MCI_STATUS_PARMS *)dwParam;
			OutTrace("mciSendCommand: Item=%d Track=%d return=%x\n", p->dwItem, p->dwTrack, p->dwReturn);
			break;
		}
	}

	if(dxw.IsFullScreen() && uMsg==MCI_PUT) pr->rc=saverect;
	if (ret) OutTraceE("mciSendCommand: ERROR res=%d\n", ret);
	return ret;
}

MCIERROR WINAPI extmciSendCommandA(MCIDEVICEID IDDevice, UINT uMsg, DWORD_PTR fdwCommand, DWORD_PTR dwParam)
{
	return extmciSendCommand(pmciSendCommandA, IDDevice, uMsg, fdwCommand, dwParam);
}

MCIERROR WINAPI extmciSendCommandW(MCIDEVICEID IDDevice, UINT uMsg, DWORD_PTR fdwCommand, DWORD_PTR dwParam)
{
	return extmciSendCommand(pmciSendCommandW, IDDevice, uMsg, fdwCommand, dwParam);
}

MCIERROR WINAPI extmciSendStringA(LPCTSTR lpszCommand, LPTSTR lpszReturnString, UINT cchReturn, HANDLE hwndCallback)
{
	MCIERROR ret;
	if(IsWithinMCICall) return(*pmciSendStringA)(lpszCommand, lpszReturnString, cchReturn, hwndCallback); // just proxy ...
	OutTraceDW("mciSendStringA: Command=\"%s\" Return=%x Callback=%x\n", lpszCommand, cchReturn, hwndCallback);
	char sMovieNickName[81];
	char sTail[81];
	RECT rect;
	sTail[0]=0;
	if (sscanf(lpszCommand, "put %s destination at %ld %ld %ld %ld %s", 
		sMovieNickName, &(rect.left), &(rect.top), &(rect.right), &(rect.bottom), sTail)>=5){
		char NewCommand[256];
		// v2.03.19 height / width fix
		rect.right += rect.left; // convert width to position
		rect.bottom += rect.top; // convert height to position
		rect=dxw.MapClientRect(&rect);
		rect.right -= rect.left; // convert position to width
		rect.bottom -= rect.top; // convert position to height
		sprintf(NewCommand, "put %s destination at %d %d %d %d %s", sMovieNickName, rect.left, rect.top, rect.right, rect.bottom, sTail);
		lpszCommand=NewCommand;
		OutTraceDW("mciSendStringA: replaced Command=\"%s\"\n", lpszCommand);
	}
	IsWithinMCICall=TRUE;
	ret=(*pmciSendStringA)(lpszCommand, lpszReturnString, cchReturn, hwndCallback);
	IsWithinMCICall=FALSE;
	if(ret) OutTraceDW("mciSendStringA ERROR: ret=%x\n", ret);
	OutTraceDW("mciSendStringA: RetString=\"%s\"\n", lpszReturnString);
	return ret;
}

MCIERROR WINAPI extmciSendStringW(LPCWSTR lpszCommand, LPWSTR lpszReturnString, UINT cchReturn, HANDLE hwndCallback)
{
	MCIERROR ret;
	WCHAR *target=L"put movie destination at ";
	if(IsWithinMCICall) return(*pmciSendStringW)(lpszCommand, lpszReturnString, cchReturn, hwndCallback); // just proxy ...
	OutTraceDW("mciSendStringW: Command=\"%ls\" Return=%x Callback=%x\n", lpszCommand, cchReturn, hwndCallback);
	WCHAR sMovieNickName[81];
	RECT rect;
	if (swscanf(lpszCommand, L"put %ls destination at %ld %ld %ld %ld", 
		sMovieNickName, &(rect.left), &(rect.top), &(rect.right), &(rect.bottom))==5){
		WCHAR NewCommand[256];
		// v2.03.19 height / width fix
		rect.right += rect.left; // convert width to position
		rect.bottom += rect.top; // convert height to position
		rect=dxw.MapClientRect(&rect);
		rect.right -= rect.left; // convert position to width
		rect.bottom -= rect.top; // convert position to height
		swprintf(NewCommand, L"put %ls destination at %d %d %d %d", sMovieNickName, rect.left, rect.top, rect.right, rect.bottom);
		lpszCommand=NewCommand;
		OutTraceDW("mciSendStringW: replaced Command=\"%ls\"\n", lpszCommand);
	}
	IsWithinMCICall=TRUE;
	ret=(*pmciSendStringW)(lpszCommand, lpszReturnString, cchReturn, hwndCallback);
	IsWithinMCICall=FALSE;
	if(ret) OutTraceDW("mciSendStringW ERROR: ret=%x\n", ret);
	OutTraceDW("mciSendStringW: RetString=\"%ls\"\n", lpszReturnString);
	return ret;
}

MCIDEVICEID WINAPI extmciGetDeviceIDA(LPCTSTR lpszDevice)
{
	MCIDEVICEID ret;
	ret = (*pmciGetDeviceIDA)(lpszDevice);
	OutTraceDW("mciGetDeviceIDA: device=\"%s\" ret=%x\n", lpszDevice, ret);
	return ret;
}

MCIDEVICEID WINAPI extmciGetDeviceIDW(LPCWSTR lpszDevice)
{
	MCIDEVICEID ret;
	ret = (*pmciGetDeviceIDW)(lpszDevice);
	OutTraceDW("mciGetDeviceIDW: device=\"%ls\" ret=%x\n", lpszDevice, ret);
	return ret;
}

DWORD WINAPI extjoyGetNumDevs(void)
{
	OutTraceDW("joyGetNumDevs: emulate joystick ret=1\n");
	return 1;
}

#define XSPAN 128
#define YSPAN 128
static void ShowJoystick(LONG, LONG, DWORD);

MMRESULT WINAPI extjoyGetDevCapsA(DWORD uJoyID, LPJOYCAPS pjc, UINT cbjc)
{
	OutTraceDW("joyGetDevCaps: joyid=%d size=%d\n", uJoyID, cbjc);
	if((uJoyID != -1) && (uJoyID != 0)) return MMSYSERR_NODRIVER;
	if(cbjc != sizeof(JOYCAPS)) return MMSYSERR_INVALPARAM;
	uJoyID = 0; // always first (unique) one ...

	// set Joystick capability structure
	memset(pjc, 0, sizeof(JOYCAPS));
	strncpy(pjc->szPname, "DxWnd Joystick Emulator", MAXPNAMELEN);
	pjc->wXmin = 0;
	pjc->wXmax = XSPAN;
	pjc->wYmin = 0;
	pjc->wYmax = YSPAN;
	pjc->wNumButtons = 2;
	pjc->wMaxButtons = 2;
	pjc->wPeriodMin = 60;
	pjc->wPeriodMax = 600;
	pjc->wCaps = 0;
	pjc->wMaxAxes = 2;
	pjc->wNumAxes = 2;

	return JOYERR_NOERROR;
}

MMRESULT WINAPI extjoyGetPosEx(DWORD uJoyID, LPJOYINFOEX pji)
{
	OutTraceC("joyGetPosEx: joyid=%d\n", uJoyID);
	if(uJoyID != 0) return JOYERR_PARMS;
	LONG x, y;
	HWND hwnd;
	DWORD dwButtons;
	static BOOL bJoyLock = FALSE;
	static DWORD dwLastClick = 0;

	dwButtons = 0;
	if (GetKeyState(VK_LBUTTON) < 0) dwButtons |= JOY_BUTTON1;
	if (GetKeyState(VK_RBUTTON) < 0) dwButtons |= JOY_BUTTON2;
	if (GetKeyState(VK_MBUTTON) < 0) dwButtons |= JOY_BUTTON3;
	OutTraceB("joyGetPosEx: Virtual Joystick buttons=%x\n", dwButtons);

	if(dwButtons == JOY_BUTTON3){
		if(((*pGetTickCount)() - dwLastClick) > 200){
			bJoyLock = !bJoyLock;
			dwButtons &= ~JOY_BUTTON3;
			dwLastClick = (*pGetTickCount)();
		}
	}

	x=(XSPAN>>1);
	y=(YSPAN>>1);
	if(hwnd=dxw.GethWnd()){
		POINT pt;
		RECT client;
		POINT upleft = {0,0};
		(*pGetClientRect)(hwnd, &client);
		(*pClientToScreen)(hwnd, &upleft);
		(*pGetCursorPos)(&pt);
		pt.x -= upleft.x;
		pt.y -= upleft.y;
		if(bJoyLock || !dxw.bActive){
			// when the joystick is "locked" (bJoyLock) or when the window lost focus
			// (dxw.bActive == FALSE) place the joystick in the central position
			OutTraceB("joyGetPosEx: CENTERED lock=%x active=%x\n", bJoyLock, dxw.bActive);
			x=(XSPAN>>1);
			y=(YSPAN>>1);
			pt.x = client.right >> 1;
			pt.y = client.bottom >> 1;
			dwButtons = JOY_BUTTON3;
		}
		else{
			OutTraceB("joyGetPosEx: ACTIVE mouse=(%d,%d)\n", pt.x, pt.y);
			if(pt.x < client.left) pt.x = client.left;
			if(pt.x > client.right) pt.x = client.right;
			if(pt.y < client.top) pt.y = client.top;
			if(pt.y > client.bottom) pt.y = client.bottom;
			x = (pt.x * XSPAN) / client.right;
			if(INVERTJOYAXIS)
				y = ((client.bottom - pt.y) * YSPAN) / client.bottom; // inverted y axis
			else
				y = (pt.y * YSPAN) / client.bottom;
		}
		ShowJoystick(pt.x, pt.y, dwButtons);
	}
	else {
		x=(XSPAN>>1);
		y=(YSPAN>>1);
	}

	// set Joystick info structure
	memset(pji, 0, sizeof(JOYINFOEX));
	pji->dwSize = sizeof(JOYINFOEX);
	pji->dwFlags = 0;
	pji->dwXpos = x;
	pji->dwYpos = y;
	pji->dwButtons = dwButtons;
	pji->dwFlags = JOY_RETURNX|JOY_RETURNY|JOY_RETURNBUTTONS;

	OutTraceC("joyGetPosEx: joyid=%d pos=(%d,%d)\n", uJoyID, pji->dwXpos, pji->dwYpos);
	return JOYERR_NOERROR;
}

static void ShowJoystick(LONG x, LONG y, DWORD dwButtons)
{
	static BOOL JustOnce=FALSE;
	extern HMODULE hInst;
	BITMAP bm;
	HDC hClientDC;
	static HBITMAP g_hbmJoyCross;
	static HBITMAP g_hbmJoyFire1;
	static HBITMAP g_hbmJoyFire2;
	static HBITMAP g_hbmJoyFire3;
	static HBITMAP g_hbmJoyCenter;
	HBITMAP g_hbmJoy;
	RECT client;
	RECT win;
	POINT PrevViewPort;

	// don't show when system cursor is visible
	CURSORINFO ci;
	ci.cbSize = sizeof(CURSORINFO);
	GetCursorInfo(&ci);
	if(ci.flags == CURSOR_SHOWING) return;

	hClientDC=(*pGDIGetDC)(dxw.GethWnd());
	(*pGetClientRect)(dxw.GethWnd(), &client);

	if(!JustOnce){
		g_hbmJoyCross = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_CROSS));
		g_hbmJoyFire1 = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_FIRE1));
		g_hbmJoyFire2 = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_FIRE2));
		g_hbmJoyFire3 = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_FIRE3));
		g_hbmJoyCenter = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_JOYCENTER));
		JustOnce=TRUE;
	}

    HDC hdcMem = CreateCompatibleDC(hClientDC);
 	switch(dwButtons){
		case 0: g_hbmJoy = g_hbmJoyCross; break;
		case JOY_BUTTON1: g_hbmJoy = g_hbmJoyFire1; break;
		case JOY_BUTTON2: g_hbmJoy = g_hbmJoyFire2; break;
		case JOY_BUTTON1|JOY_BUTTON2: g_hbmJoy = g_hbmJoyFire3; break;
		case JOY_BUTTON3: g_hbmJoy = g_hbmJoyCenter; break;
		default: g_hbmJoy = NULL; break;
	}

	if(g_hbmJoy == NULL) return; // show nothing ...

    HBITMAP hbmOld = (HBITMAP)SelectObject(hdcMem, g_hbmJoy);
	GetObject(g_hbmJoy, sizeof(bm), &bm);

	(*pGetWindowRect)(dxw.GethWnd(), &win);

	(*pSetViewportOrgEx)(hClientDC, 0, 0, &PrevViewPort);
	int w, h;
	w=bm.bmWidth;
	h=bm.bmHeight; 
	(*pGDIBitBlt)(hClientDC, x-(w>>1), y-(h>>1), w, h, hdcMem, 0, 0, SRCPAINT);

	(*pSetViewportOrgEx)(hClientDC, PrevViewPort.x, PrevViewPort.y, NULL);
    SelectObject(hdcMem, hbmOld);
    DeleteDC(hdcMem);
}
