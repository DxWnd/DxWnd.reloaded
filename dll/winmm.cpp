#include "dxwnd.h"
#include "dxwcore.hpp"
#include "syslibs.h"
#include "dxhook.h"
#include "dxhelper.h"

#include "MMSystem.h"

//#undef OutTraceD
//#define OutTraceD OutTrace

static HookEntry_Type Hooks[]={
	{"mciSendCommandA", NULL, (FARPROC *)&pmciSendCommand, (FARPROC)extmciSendCommand},
	{0, NULL, 0, 0} // terminator
};

static HookEntry_Type TimeHooks[]={
	{"timeGetTime", NULL, (FARPROC *)&ptimeGetTime, (FARPROC)exttimeGetTime},
	{"timeKillEvent", NULL, (FARPROC *)&ptimeKillEvent, (FARPROC)exttimeKillEvent},
	{"timeSetEvent", NULL, (FARPROC *)&ptimeSetEvent, (FARPROC)exttimeSetEvent},
	{0, NULL, 0, 0} // terminator
};

void HookWinMM(HMODULE module)
{
	HookLibrary(module, Hooks, "winmm.dll");
	if(dxw.dwFlags2 & TIMESTRETCH) HookLibrary(module, TimeHooks, "winmm.dll");
}

FARPROC Remap_WinMM_ProcAddress(LPCSTR proc, HMODULE hModule)
{
	FARPROC addr;

	if (addr=RemapLibrary(proc, hModule, Hooks)) return addr;
	if(dxw.dwFlags2 & TIMESTRETCH)
		if (addr=RemapLibrary(proc, hModule, TimeHooks)) return addr;

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
	uDelay = uDelay * 8;
	OutTraceD("timeSetEvent: Delay=%d Resolution=%d Event=%x\n", uDelay, uResolution, fuEvent);
	res=(*ptimeSetEvent)(uDelay, uResolution, lpTimeProc, dwUser, fuEvent);
	OutTraceD("timeSetEvent: ret=%x\n", res);
	return res;
}

MMRESULT WINAPI exttimeKillEvent(UINT uTimerID)
{
	MMRESULT res;
	OutTraceD("timeKillEvent: TimerID=%x\n", uTimerID);
	res=(*ptimeKillEvent)(uTimerID);
	OutTraceD("timeKillEvent: ret=%x\n", res);
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

MCIERROR WINAPI extmciSendCommand(MCIDEVICEID IDDevice, UINT uMsg, DWORD_PTR fdwCommand, DWORD_PTR dwParam)
{
	RECT saverect;
	MCIERROR ret;
	MCI_ANIM_RECT_PARMS *pr;
	MCI_OVLY_WINDOW_PARMSW *pw;

	OutTraceD("mciSendCommand: IDDevice=%x msg=%x(%s) Command=%x(%s)\n",
		IDDevice, uMsg, ExplainMCICommands(uMsg), fdwCommand, ExplainMCIFlags(uMsg, fdwCommand));

	if(dxw.IsFullScreen()){
		switch(uMsg){
		case MCI_WINDOW:
			pw = (MCI_OVLY_WINDOW_PARMSW *)dwParam;
			OutTraceB("mciSendCommand: hwnd=%x CmdShow=%x\n",
				pw->hWnd, pw->nCmdShow);
			//fdwCommand |= MCI_ANIM_WINDOW_ENABLE_STRETCH;
			//fdwCommand &= ~MCI_ANIM_WINDOW_DISABLE_STRETCH;
			if(dxw.IsDesktop(pw->hWnd)) pw->hWnd = dxw.GethWnd();
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
		}
	}

	ret=(*pmciSendCommand)(IDDevice, uMsg, fdwCommand, dwParam);
	if(dxw.IsFullScreen() && uMsg==MCI_PUT) pr->rc=saverect;
	if (ret) OutTraceE("mciSendCommand: ERROR res=%x\n", ret);
	return ret;
}
