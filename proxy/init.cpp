#include <windows.h>
#include <stdio.h>
#include "dxwnd.h"

typedef int (*StartHook_)();
typedef int (*EndHook_)();
typedef int (*SetTarget_)(TARGETMAP *);

BOOL DxWndEnabled = FALSE;
char DxWndIniPath[] = ".\\dxwnd.dxw";

static TARGETMAP target;
static HMODULE hModule;

static int GetIntEntry(char *tag, int default)
{
	return GetPrivateProfileInt("target", tag, default, DxWndIniPath); 
}

void InitDxWnd(void)
{
	char DxWndPath[MAX_PATH];
	GetPrivateProfileString("system", "dxwndpath", ".", DxWndPath, MAX_PATH, DxWndIniPath);
	//MessageBox(0, DxWndPath, "dxwnd", MB_ICONERROR);
	sprintf(DxWndPath, "%s\\dxwnd.dll", DxWndPath);
	//MessageBox(0, DxWndPath, "dxwnd", MB_ICONERROR);

	hModule = LoadLibraryA(DxWndPath);  
	//hModule = LoadLibraryA("dxwnd.dll");  
    if (hModule)
    {
        StartHook_ startHook = (StartHook_)GetProcAddress(hModule, "StartHook");
        SetTarget_ setTarget = (SetTarget_)GetProcAddress(hModule, "SetTarget");
        if (startHook && setTarget)
        {
			// target.path must be set to current task pathname.
            GetModuleFileNameA(NULL, target.path, MAX_PATH);

			target.FakeVersionId = GetIntEntry("winver0", 0);
			target.flags		= GetIntEntry("flag0", EMULATESURFACE|MODIFYMOUSE|USERGB565);
			target.flags2		= GetIntEntry("flagg0", WINDOWIZE|SETCOMPATIBILITY);
			target.flags3		= GetIntEntry("flagh0", HOOKDLLS|HOOKENABLED);
			target.flags4		= GetIntEntry("flagi0", SUPPORTSVGA);
			target.flags5		= GetIntEntry("flagj0", AEROBOOST|CENTERTOWIN);
			target.flags6		= GetIntEntry("flagk0", 0);
			target.flags7		= GetIntEntry("flagl0", 0);
			target.flags8		= GetIntEntry("flagm0", 0);
			target.flags9		= GetIntEntry("flagn0", 0);
			target.flags10		= GetIntEntry("flago0", 0);
			target.tflags		= GetIntEntry("tflag0", 0);
			target.dflags		= GetIntEntry("dflag0", 0);
			target.posx			= GetIntEntry("posx0", 0);
			target.posy			= GetIntEntry("posy0", 0);
            target.sizx			= GetIntEntry("sizx0", 0);
            target.sizy			= GetIntEntry("sizy0", 0);
            target.coordinates	= GetIntEntry("coord0", 1); // centered to desktop
            target.MaxFPS		= GetIntEntry("maxfps0", 0);
			target.dxversion	= GetIntEntry("ver0", 0);
			if(target.dxversion > 1) target.dxversion += 5;
			target.FakeVersionId= GetIntEntry("winver0", 0);
			target.MaxScreenRes = GetIntEntry("maxres0", 0);
			target.SwapEffect	= GetIntEntry("swapeffect0", 0);
			target.InitTS		= GetIntEntry("initts0", 0);
            target.monitorid	= GetIntEntry("monitorid0", -1);
			target.resw			= GetIntEntry("initresw0", 800);
			target.resh			= GetIntEntry("initresh0", 600);
            target.MaxDdrawInterface = GetIntEntry("maxddinterface0", 7);    
			target.ScanLine		= GetIntEntry("scanline0", 0);

            setTarget((TARGETMAP *)&target);
            startHook();
        }
    }
	else
		MessageBox(0, "Cannot load dxwnd.dll library", "dxwnd", MB_ICONERROR);
}

void DxWndEndHook(void)
{
    if (hModule)
    {
        EndHook_ endHook = (EndHook_)GetProcAddress(hModule, "EndHook");
        if (endHook) endHook();
    }
}
