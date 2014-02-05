#define _WIN32_WINNT 0x0600
#define WIN32_LEAN_AND_MEAN
#define _CRT_SECURE_NO_DEPRECATE 1

#include <windows.h>
#include <stdio.h>
#include <stdarg.h>
#include <psapi.h>
#include "dxwnd.h"
#include "dxwcore.hpp"
#include "dxhook.h"
#include "glhook.h"
#include "glidehook.h"
#include "msvfwhook.h"
#define DXWDECLARATIONS 1
#include "syslibs.h"
#undef DXWDECLARATIONS
#include "dxhelper.h"
#include "Ime.h"
#include "Winnls32.h"
#include "Mmsystem.h"
#include "disasm.h"

dxwCore dxw;

typedef char *(*Geterrwarnmessage_Type)(unsigned long, unsigned long);
typedef int (*Preparedisasm_Type)(void);
typedef void (*Finishdisasm_Type)(void);
typedef unsigned long (*Disasm_Type)(const unsigned char *, unsigned long, unsigned long, t_disasm *, int, t_config *, int (*)(tchar *, unsigned long));

Geterrwarnmessage_Type pGeterrwarnmessage;
Preparedisasm_Type pPreparedisasm;
Finishdisasm_Type pFinishdisasm;
Disasm_Type pDisasm;

extern void InitScreenParameters();
void HookModule(HMODULE, int);
static void RecoverScreenMode();
static void LockScreenMode(DWORD, DWORD, DWORD);
DEVMODE SetDevMode;
DEVMODE *pSetDevMode=NULL;

extern HANDLE hTraceMutex;

CRITICAL_SECTION TraceCS; 

static char *FlagNames[32]={
	"UNNOTIFY", "EMULATESURFACE", "CLIPCURSOR", "RESETPRIMARY",
	"HOOKDI", "MODIFYMOUSE", "HANDLEEXCEPTIONS", "SAVELOAD",
	"EMULATEBUFFER", "AUTOMATIC", "BLITFROMBACKBUFFER", "SUPPRESSCLIPPING",
	"AUTOREFRESH", "FIXWINFRAME", "HIDEHWCURSOR", "SLOWDOWN",
	"ENABLECLIPPING", "LOCKWINSTYLE", "MAPGDITOPRIMARY", "FIXTEXTOUT",
	"KEEPCURSORWITHIN", "USERGB565", "SUPPRESSDXERRORS", "PREVENTMAXIMIZE",
	"LOCKEDSURFACE", "FIXPARENTWIN", "SWITCHVIDEOMEMORY", "CLIENTREMAPPING",
	"HANDLEALTF4", "LOCKWINPOS", "HOOKCHILDWIN", "MESSAGEPROC"
};

static char *Flag2Names[32]={
	"RECOVERSCREENMODE", "REFRESHONRESIZE", "BACKBUFATTACH", "MODALSTYLE",
	"KEEPASPECTRATIO", "INIT8BPP", "FORCEWINRESIZE", "INIT16BPP",
	"KEEPCURSORFIXED", "DISABLEGAMMARAMP", "DIFFERENTIALMOUSE", "FIXNCHITTEST",
	"LIMITFPS", "SKIPFPS", "SHOWFPS", "HIDEMULTIMONITOR",
	"TIMESTRETCH", "HOOKOPENGL", "WALLPAPERMODE", "SHOWHWCURSOR",
	"GDISTRETCHED", "SHOWFPSOVERLAY", "FAKEVERSION", "FULLRECTBLT",
	"NOPALETTEUPDATE", "SUPPRESSIME", "NOBANNER", "WINDOWIZE",
	"LIMITRESOURCES", "STARTDEBUG", "SETCOMPATIBILITY", "WIREFRAME",
};

static char *Flag3Names[32]={
	"FORCEHOOKOPENGL", "MARKBLIT", "HOOKDLLS", "SUPPRESSD3DEXT",
	"HOOKENABLED", "FIXD3DFRAME", "FORCE16BPP", "BLACKWHITE",
	"SAVECAPS", "SINGLEPROCAFFINITY", "EMULATEREGISTRY", "CDROMDRIVETYPE",
	"NOWINDOWMOVE", "--DISABLEHAL--", "LOCKSYSCOLORS", "GDIEMULATEDC",
	"FULLSCREENONLY", "FONTBYPASS", "YUV2RGB", "RGB2YUV",
	"BUFFEREDIOFIX", "FILTERMESSAGES", "PEEKALLMESSAGES", "SURFACEWARN",
	"ANALYTICMODE", "FORCESHEL", "CAPMASK", "COLORFIX",
	"NODDRAWBLT", "NODDRAWFLIP", "NOGDIBLT", "NOPIXELFORMAT",
};

static char *Flag4Names[32]={
	"NOALPHACHANNEL", "SUPPRESSCHILD", "FIXREFCOUNTER", "SHOWTIMESTRETCH",
	"ZBUFFERCLEAN", "ZBUFFER0CLEAN", "ZBUFFERALWAYS", "DISABLEFOGGING",
	"NOPOWER2FIX", "NOPERFCOUNTER", "ADDPROXYLIBS", "INTERCEPTRDTSC",
	"LIMITSCREENRES", "NOFILLRECT", "HOOKGLIDE", "HIDEDESKTOP",
	"STRETCHTIMERS", "NOFLIPEMULATION", "NOTEXTURES", "RETURNNULLREF",
	"FINETIMING", "NATIVERES", "SUPPORTSVGA", "SUPPORTHDTV",
	"RELEASEMOUSE", "", "", "",
	"", "", "", "",
};

static char *TFlagNames[32]={
	"OUTTRACE", "OUTDDRAWTRACE", "OUTWINMESSAGES", "OUTCURSORTRACE",
	"OUTPROXYTRACE", "DXPROXED", "ASSERTDIALOG", "OUTIMPORTTABLE",
	"OUTDEBUG", "OUTREGISTRY", "TRACEHOOKS", "",
	"", "", "", "",
	"", "", "", "",
	"", "", "", "",
	"", "", "", "",
	"", "", "", "",
};

static void OutTraceHeader(FILE *fp)
{
	SYSTEMTIME Time;
	char Version[20+1];
	int i;
	DWORD dword;
	GetLocalTime(&Time);
	GetDllVersion(Version);
	fprintf(fp, "*** DxWnd %s log BEGIN: %02d-%02d-%04d %02d:%02d:%02d ***\n",
		Version, Time.wDay, Time.wMonth, Time.wYear, Time.wHour, Time.wMinute, Time.wSecond);
	fprintf(fp, "*** Flags= ");
	for(i=0, dword=dxw.dwFlags1;  i<32; i++, dword>>=1) if(dword & 0x1) fprintf(fp, "%s ", FlagNames[i]);
	for(i=0, dword=dxw.dwFlags2; i<32; i++, dword>>=1) if(dword & 0x1) fprintf(fp, "%s ", Flag2Names[i]);
	for(i=0, dword=dxw.dwFlags3; i<32; i++, dword>>=1) if(dword & 0x1) fprintf(fp, "%s ", Flag3Names[i]);
	for(i=0, dword=dxw.dwFlags4; i<32; i++, dword>>=1) if(dword & 0x1) fprintf(fp, "%s ", Flag4Names[i]);
	for(i=0, dword=dxw.dwTFlags; i<32; i++, dword>>=1) if(dword & 0x1) fprintf(fp, "%s ", TFlagNames[i]);
	fprintf(fp, "***\n");
}

void OutTrace(const char *format, ...)
{
	va_list al;
	static char path[MAX_PATH];
	static FILE *fp=NULL; // GHO: thread safe???

	// check global log flag
	if(!(dxw.dwTFlags & OUTTRACE)) return;

	WaitForSingleObject(hTraceMutex, INFINITE);
	if (fp == NULL){
		GetCurrentDirectory(MAX_PATH, path);
		strcat(path, "\\dxwnd.log");
		fp = fopen(path, "a+");
		if (fp==NULL){ // in case of error (e.g. current dir on unwritable CD unit)... 
			strcpy(path, getenv("TEMP"));
			strcat(path, "\\dxwnd.log");
			fp = fopen(path, "a+");
		}
		if (fp==NULL){ // last chance: do not log... 
			dxw.dwTFlags &= ~OUTTRACE; // turn flag OFF
			return;
		}
		else 
			OutTraceHeader(fp);
	}
	va_start(al, format);
	vfprintf(fp, format, al);
	va_end(al);
	ReleaseMutex(hTraceMutex);

	fflush(fp); 
}

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

static void SuppressIMEWindow()
{
	OutTraceDW("WindowProc: SUPPRESS IME\n");
	typedef BOOL (WINAPI *ImmDisableIME_Type)(DWORD);
	ImmDisableIME_Type pImmDisableIME;
	HMODULE ImmLib;
	ImmLib=(*pLoadLibraryA)("Imm32");
	if(ImmLib){
		pImmDisableIME=(ImmDisableIME_Type)(*pGetProcAddress)(ImmLib,"ImmDisableIME");
		if(pImmDisableIME)(*pImmDisableIME)(-1);
		FreeLibrary(ImmLib);
	}
}

void HookDlls(HMODULE module)
{
	PIMAGE_NT_HEADERS pnth;
	PIMAGE_IMPORT_DESCRIPTOR pidesc;
	DWORD base, rva;
	PSTR impmodule;
	PIMAGE_THUNK_DATA ptname;
	extern char *SysNames[];

	base=(DWORD)module;
	OutTraceB("HookDlls: base=%x\n", base);
	__try{
		pnth = PIMAGE_NT_HEADERS(PBYTE(base) + PIMAGE_DOS_HEADER(base)->e_lfanew);
		if(!pnth) {
			OutTraceH("HookDlls: ERROR no pnth at %d\n", __LINE__);
			return;
		}
		rva = pnth->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress;
		if(!rva) {
			OutTraceH("HookDlls: ERROR no rva at %d\n", __LINE__);
			return;
		}

		for(pidesc = (PIMAGE_IMPORT_DESCRIPTOR)(base + rva); pidesc->FirstThunk; pidesc++){
			HMODULE DllBase;
			int idx;
			extern HMODULE SysLibs[];

			impmodule = (PSTR)(base + pidesc->Name);

			// skip dxwnd and system dll
			if(!lstrcmpi(impmodule, "DxWnd")) continue; 
			idx=dxw.GetDLLIndex(impmodule);
			if(!lstrcmpi(impmodule,dxw.CustomOpenGLLib))idx=SYSLIBIDX_OPENGL;
			if(idx != -1) {
				DllBase=GetModuleHandle(impmodule);
				SysLibs[idx]=DllBase;
				OutTraceH("HookDlls: system module %s at %x\n", impmodule, DllBase);
				continue;
			}

			OutTraceH("HookDlls: ENTRY timestamp=%x module=%s forwarderchain=%x\n", 
				pidesc->TimeDateStamp, impmodule, pidesc->ForwarderChain);
			if(pidesc->OriginalFirstThunk) {
				ptname = (PIMAGE_THUNK_DATA)(base + (DWORD)pidesc->OriginalFirstThunk);
			}
			else{
				ptname = 0;
				OutTraceH("HookDlls: no PE OFTs - stripped module=%s\n", impmodule);
			}

			DllBase=GetModuleHandle(impmodule);
			if(DllBase) HookModule(DllBase, 0);
		}
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{       
		OutTraceDW("HookDlls: EXCEPTION\n");
	}
	return;
}

void DumpImportTable(HMODULE module)
{
	PIMAGE_NT_HEADERS pnth;
	PIMAGE_IMPORT_DESCRIPTOR pidesc;
	DWORD base, rva;
	PSTR impmodule;
	PIMAGE_THUNK_DATA ptaddr;
	PIMAGE_THUNK_DATA ptname;
	PIMAGE_IMPORT_BY_NAME piname;

	base=(DWORD)module;
	OutTrace("DumpImportTable: base=%x\n", base);
	__try{
		pnth = PIMAGE_NT_HEADERS(PBYTE(base) + PIMAGE_DOS_HEADER(base)->e_lfanew);
		if(!pnth) {
			OutTrace("DumpImportTable: ERROR no pnth at %d\n", __LINE__);
			return;
		}
		rva = pnth->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress;
		if(!rva) {
			OutTrace("DumpImportTable: ERROR no rva at %d\n", __LINE__);
			return;
		}
		pidesc = (PIMAGE_IMPORT_DESCRIPTOR)(base + rva);

		while(pidesc->FirstThunk){
			impmodule = (PSTR)(base + pidesc->Name);
			OutTrace("DumpImportTable: ENTRY timestamp=%x module=%s forwarderchain=%x\n", 
				pidesc->TimeDateStamp, impmodule, pidesc->ForwarderChain);
			if(pidesc->OriginalFirstThunk) {
				ptname = (PIMAGE_THUNK_DATA)(base + (DWORD)pidesc->OriginalFirstThunk);
			}
			else{
				ptname = 0;
				OutTrace("DumpImportTable: no PE OFTs - stripped module=%s\n", impmodule);
			}
			ptaddr = (PIMAGE_THUNK_DATA)(base + (DWORD)pidesc->FirstThunk);
			while(ptaddr->u1.Function){
				OutTrace("addr=%x", ptaddr->u1.Function); 
				ptaddr ++;
				if(ptname){
					if(!IMAGE_SNAP_BY_ORDINAL(ptname->u1.Ordinal)){
						piname = (PIMAGE_IMPORT_BY_NAME)(base + (DWORD)ptname->u1.AddressOfData);
						OutTrace(" hint=%x name=%s", piname->Hint, piname->Name);
						ptname ++;
					}
				}
				OutTrace("\n");
			}
			OutTrace("*** EOT ***\n", ptaddr->u1.Function); 
			pidesc ++;
		}
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{       
		OutTraceDW("DumpImportTable: EXCEPTION\n");
	}
	return;
}

void SetHook(void *target, void *hookproc, void **hookedproc, char *hookname)
{
	void *tmp;
	char msg[81];
	DWORD dwTmp, oldprot;
	static DWORD MinHook=0xFFFFFFFF;
	static DWORD MaxHook=0;
	
	OutTraceH("SetHook: DEBUG target=%x, proc=%x name=%s\n", target, hookproc, hookname);
	// keep track of hooked call range to avoid re-hooking of hooked addresses !!!
	if ((DWORD)hookproc < MinHook) MinHook=(DWORD)hookproc;
	if ((DWORD)hookproc > MaxHook) MaxHook=(DWORD)hookproc;
	dwTmp = *(DWORD *)target;
	if(dwTmp == (DWORD)hookproc) return; // already hooked
	if((dwTmp <= MaxHook) && (dwTmp >= MinHook)) return; // already hooked
	if(dwTmp == 0){
		sprintf(msg,"SetHook ERROR: NULL target for %s\n", hookname);
		OutTraceDW(msg);
		MessageBox(0, msg, "SetHook", MB_OK | MB_ICONEXCLAMATION);
		return; // error condition
	}
	if(!VirtualProtect(target, 4, PAGE_READWRITE, &oldprot)) {
		sprintf(msg,"SetHook ERROR: target=%x err=%d\n", target, GetLastError());
		OutTraceDW(msg);
		if (IsAssertEnabled) MessageBox(0, msg, "SetHook", MB_OK | MB_ICONEXCLAMATION);
		return; // error condition
	}
	*(DWORD *)target = (DWORD)hookproc;
	if(!VirtualProtect(target, 4, oldprot, &oldprot)){
		OutTrace("SetHook: VirtualProtect ERROR target=%x, err=%x\n", target, GetLastError());
		return; // error condition
	}
	if(!FlushInstructionCache(GetCurrentProcess(), target, 4)){
		OutTrace("SetHook: FlushInstructionCache ERROR target=%x, err=%x\n", target, GetLastError());
		return; // error condition
	}
	tmp=(void *)dwTmp;

	if (*hookedproc && *hookedproc!=tmp) {
		sprintf(msg,"SetHook: proc=%s oldhook=%x newhook=%x\n", hookname, hookedproc, tmp);
		OutTraceDW(msg);
		if (IsAssertEnabled) MessageBox(0, msg, "SetHook", MB_OK | MB_ICONEXCLAMATION);
	}
	*hookedproc = tmp;
}

// v2.02.53: thorough scan - the IAT is scanned considering the possibility to have each dll module 
// replicated also many times. It may depend upon the compiling environment? 
// So far, it makes the difference for Dungeon Odissey

void *HookAPI(HMODULE module, char *dll, void *apiproc, const char *apiname, void *hookproc)
{
	PIMAGE_NT_HEADERS pnth;
	PIMAGE_IMPORT_DESCRIPTOR pidesc;
	DWORD base, rva;
	PSTR impmodule;
	PIMAGE_THUNK_DATA ptaddr;
	PIMAGE_THUNK_DATA ptname;
	PIMAGE_IMPORT_BY_NAME piname;
	DWORD oldprotect;
	void *org;

	if(dxw.dwTFlags & OUTIMPORTTABLE) OutTrace("HookAPI: module=%x dll=%s apiproc=%x apiname=%s hookproc=%x\n", 
		module, dll, apiproc, apiname, hookproc);

	if(!*apiname) { // check
		char *sMsg="HookAPI: NULL api name\n";
		OutTraceE(sMsg);
		if (IsAssertEnabled) MessageBox(0, sMsg, "HookAPI", MB_OK | MB_ICONEXCLAMATION);
		return 0;
	}

	base = (DWORD)module;
	org = 0; // by default, ret = 0 => API not found

	__try{
		pnth = PIMAGE_NT_HEADERS(PBYTE(base) + PIMAGE_DOS_HEADER(base)->e_lfanew);
		if(!pnth) {
			OutTraceH("HookAPI: ERROR no PNTH at %d\n", __LINE__);
			return 0;
		}
		rva = pnth->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress;
		if(!rva) {
			OutTraceH("HookAPI: ERROR no RVA at %d\n", __LINE__);
			return 0;
		}
		pidesc = (PIMAGE_IMPORT_DESCRIPTOR)(base + rva);

		while(pidesc->FirstThunk){
			impmodule = (PSTR)(base + pidesc->Name);
			if(!lstrcmpi(dll, impmodule)) {
				//OutTraceH("HookAPI: dll=%s found at %x\n", dll, impmodule);

				ptaddr = (PIMAGE_THUNK_DATA)(base + (DWORD)pidesc->FirstThunk);
				ptname = (pidesc->OriginalFirstThunk) ? (PIMAGE_THUNK_DATA)(base + (DWORD)pidesc->OriginalFirstThunk) : NULL;

				//if((apiproc==NULL) && (ptname==NULL)){
				//	OutTraceH("HookAPI: unreacheable api=%s dll=%s\n", apiname, dll);
				//	return 0;
				//}

				while(ptaddr->u1.Function){
					if (ptname){
						if(!IMAGE_SNAP_BY_ORDINAL(ptname->u1.Ordinal)){
							piname = (PIMAGE_IMPORT_BY_NAME)(base + (DWORD)ptname->u1.AddressOfData);
							//OutTrace("examining by ptname name=%s\n", (char *)piname->Name);
							if(!lstrcmpi(apiname, (char *)piname->Name)) break;
						}
					}
					if (apiproc){
						// log not working !!!
						//piname = (PIMAGE_IMPORT_BY_NAME)(base + (DWORD)ptaddr->u1.AddressOfData);
						//OutTrace("examining by addr name=%s\n", (char *)piname->Name);
						if(ptaddr->u1.Function == (DWORD)apiproc) break;
					}
					ptaddr ++;
					if (ptname) ptname ++;
				}

				if(ptaddr->u1.Function) {
					org = (void *)ptaddr->u1.Function;
					if(org == hookproc) return 0; // already hooked
						
					if(!VirtualProtect(&ptaddr->u1.Function, 4, PAGE_EXECUTE_READWRITE, &oldprotect)) {
						OutTraceDW("HookAPI: VirtualProtect error %d at %d\n", GetLastError(), __LINE__);
						return 0;
					}
					ptaddr->u1.Function = (DWORD)hookproc;
					if(!VirtualProtect(&ptaddr->u1.Function, 4, oldprotect, &oldprotect)) {
						OutTraceDW("HookAPI: VirtualProtect error %d at %d\n", GetLastError(), __LINE__);
						return 0;
					}
					if (!FlushInstructionCache(GetCurrentProcess(), &ptaddr->u1.Function, 4)) {
						OutTraceDW("HookAPI: FlushInstructionCache error %d at %d\n", GetLastError(), __LINE__);
						return 0;
					}
					OutTraceH("HookAPI hook=%s address=%x->%x\n", apiname, org, hookproc);

					return org;
				}
			}
			pidesc ++;
		}
		if(!pidesc->FirstThunk) {
			OutTraceH("HookAPI: PE unreferenced function %s:%s\n", dll, apiname);
			return 0;
		}
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{       
		OutTraceDW("HookAPI: EXCEPTION hook=%s:%s Hook Failed.\n", dll, apiname);
	}
	return org;
}

// v.2.1.80: unified positioning logic into CalculateWindowPos routine
// now taking in account for window menus (see "Alien Cabal")

void CalculateWindowPos(HWND hwnd, DWORD width, DWORD height, LPWINDOWPOS wp)
{
	RECT rect, desktop, workarea;
	DWORD dwStyle, dwExStyle;
	int MaxX, MaxY;
	HMENU hMenu;

	switch(dxw.Coordinates){
	case DXW_DESKTOP_CENTER:
		MaxX = dxw.iSizX;
		MaxY = dxw.iSizY;
		if (!MaxX) MaxX = width;
		if (!MaxY) MaxY = height;
		//GetClientRect(0, &desktop);
		(*pGetClientRect)(GetDesktopWindow(), &desktop);
		rect.left =  (desktop.right - MaxX) / 2;
		rect.top = (desktop.bottom - MaxY) / 2;
		rect.right = rect.left + MaxX;
		rect.bottom = rect.top + MaxY; //v2.02.09
		break;
	case DXW_DESKTOP_WORKAREA:
		(*pSystemParametersInfoA)(SPI_GETWORKAREA, NULL, &workarea, 0);
		rect = workarea;
		break;
	case DXW_DESKTOP_FULL:
		(*pGetClientRect)((*pGetDesktopWindow)(), &workarea);
		rect = workarea;
		break;
	case DXW_SET_COORDINATES:
	default:
		rect.left =  dxw.iPosX;
		rect.top = dxw.iPosY; //v2.02.09
		MaxX = dxw.iSizX;
		MaxY = dxw.iSizY;
		if (!MaxX) MaxX = width;
		if (!MaxY) MaxY = height;
		rect.right = dxw.iPosX + MaxX;
		rect.bottom = dxw.iPosY + MaxY; //v2.02.09
		break;
	}

	RECT UnmappedRect;
	UnmappedRect=rect;
	dwStyle=(*pGetWindowLongA)(hwnd, GWL_STYLE);
	dwExStyle=(*pGetWindowLongA)(hwnd, GWL_EXSTYLE);
	// BEWARE: from MSDN -  If the window is a child window, the return value is undefined. 
	hMenu = (dwStyle & WS_CHILD) ? NULL : GetMenu(hwnd);	
	AdjustWindowRectEx(&rect, dwStyle, (hMenu!=NULL), dwExStyle);
	if (hMenu) CloseHandle(hMenu);
	switch(dxw.Coordinates){
	case DXW_DESKTOP_WORKAREA:
	case DXW_DESKTOP_FULL:
		// if there's a menu, reduce height to fit area
		if(rect.top != UnmappedRect.top){
			rect.bottom = rect.bottom - UnmappedRect.top + rect.top;
		}
		break;
	default:
		break;
	}

	// shift down-right so that the border is visible
	// and also update the iPosX,iPosY upper-left coordinates of the client area
	if(rect.left < 0){
		rect.right -= rect.left;
		rect.left = 0;
	}
	if(rect.top < 0){
		rect.bottom -= rect.top;
		rect.top = 0;
	}

	wp->x=rect.left;
	wp->y=rect.top;
	wp->cx=rect.right-rect.left;
	wp->cy=rect.bottom-rect.top;
}

void AdjustWindowPos(HWND hwnd, DWORD width, DWORD height)
{
	WINDOWPOS wp;
	OutTraceDW("AdjustWindowPos: hwnd=%x, size=(%d,%d)\n", hwnd, width, height);
	CalculateWindowPos(hwnd, width, height, &wp);
	OutTraceDW("AdjustWindowPos: fixed pos=(%d,%d) size=(%d,%d)\n", wp.x, wp.y, wp.cx, wp.cy);
	//if(!pSetWindowPos) pSetWindowPos=SetWindowPos;
	//OutTraceDW("pSetWindowPos=%x\n", pSetWindowPos);
	OutTraceDW("hwnd=%x pos=(%d,%d) size=(%d,%d)\n", pSetWindowPos, wp.x, wp.y, wp.cx, wp.cy);
	if(!(*pSetWindowPos)(hwnd, 0, wp.x, wp.y, wp.cx, wp.cy, 0)){
		OutTraceE("AdjustWindowPos: ERROR err=%d at %d\n", GetLastError(), __LINE__);
	}

	if(dxw.dwFlags2 & SUPPRESSIME) SuppressIMEWindow();
	dxw.ShowBanner(hwnd);
	if(dxw.dwFlags4 & HIDEDESKTOP) dxw.HideDesktop(dxw.GethWnd());
	return;
}

void HookWindowProc(HWND hwnd)
{
	WNDPROC pWindowProc;
	pWindowProc = (WNDPROC)(*pGetWindowLongA)(hwnd, GWL_WNDPROC);
	if ((pWindowProc == extWindowProc) ||
		(pWindowProc == extChildWindowProc) ||
		(pWindowProc == extDialogWindowProc)){
		// hooked already !!!
		OutTraceDW("GetWindowLong: hwnd=%x WindowProc HOOK already in place\n", hwnd);
	}
	else {// don't hook twice ....
		long lres;
		WhndStackPush(hwnd, pWindowProc);
		lres=(*pSetWindowLongA)(hwnd, GWL_WNDPROC, (LONG)extWindowProc);
		OutTraceDW("SetWindowLong: hwnd=%x HOOK WindowProc=%x->%x\n", hwnd, lres, (LONG)extWindowProc);
	}
}

void AdjustWindowFrame(HWND hwnd, DWORD width, DWORD height)
{

	HRESULT res=0;
	LONG style;

	OutTraceDW("AdjustWindowFrame hwnd=%x, size=(%d,%d) coord=%d\n", hwnd, width, height, dxw.Coordinates); 

	dxw.SetScreenSize(width, height);
	if (hwnd==NULL) return;

	switch(dxw.Coordinates){
		case DXW_SET_COORDINATES:
		case DXW_DESKTOP_CENTER:
			style = (dxw.dwFlags2 & MODALSTYLE) ?  0 : WS_OVERLAPPEDWINDOW;
			break;
		case DXW_DESKTOP_WORKAREA:
		case DXW_DESKTOP_FULL:
			style = 0;
			break;
	}

	(*pSetWindowLongA)(hwnd, GWL_STYLE, style);
	(*pSetWindowLongA)(hwnd, GWL_EXSTYLE, 0); 
	(*pShowWindow)(hwnd, SW_SHOWNORMAL);
	OutTraceDW("AdjustWindowFrame hwnd=%x, set style=%s extstyle=0\n", hwnd, (style == 0) ? "0" : "WS_OVERLAPPEDWINDOW"); 
	AdjustWindowPos(hwnd, width, height);

	// fixing windows message handling procedure
	HookWindowProc(hwnd);

	// fixing cursor view and clipping region

	if (dxw.dwFlags1 & HIDEHWCURSOR) while ((*pShowCursor)(0) >= 0);
	if (dxw.dwFlags2 & SHOWHWCURSOR) while((*pShowCursor)(1) < 0);
	if (dxw.dwFlags1 & CLIPCURSOR) {
		OutTraceDW("AdjustWindowFrame: setting clip region\n");
		dxw.SetClipCursor();
	}

	(*pInvalidateRect)(hwnd, NULL, TRUE);
}

LRESULT CALLBACK extDialogWindowProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
{
	static int i=0;
	static WINDOWPOS *wp;
	WNDPROC pWindowProc;

	static int t = -1;
	if (t == -1)
		t = (*pGetTickCount)();
	int tn = (*pGetTickCount)();

	OutTraceW("DEBUG: DialogWinMsg [0x%x]%s(%x,%x)\n", message, ExplainWinMessage(message), wparam, lparam);

	// optimization: don't invalidate too often!
	// 200mSec seems a good compromise.
	if (tn-t > 200) {
		(*pInvalidateRect)(hwnd, NULL, TRUE);
		t=tn;
	}

	pWindowProc=WhndGetWindowProc(hwnd);
	if(pWindowProc) return(*pWindowProc)(hwnd, message, wparam, lparam);
	char *sMsg="ASSERT: DialogWinMsg pWindowProc=NULL !!!\n";
	OutTraceDW(sMsg);
	if (IsAssertEnabled) MessageBox(0, sMsg, "WindowProc", MB_OK | MB_ICONEXCLAMATION);
	return NULL;
}

LRESULT CALLBACK extChildWindowProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
{
	static int i=0;
	static WINDOWPOS *wp;
	WNDPROC pWindowProc;

	OutTraceW("DEBUG: ChildWinMsg [0x%x]%s(%x,%x)\n", message, ExplainWinMessage(message), wparam, lparam);
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
		if(dxw.dwFlags1 & MODIFYMOUSE){ // mouse processing  
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

	pWindowProc=WhndGetWindowProc(hwnd);
	
	if(pWindowProc) return(*pWindowProc)(hwnd, message, wparam, lparam);

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

LRESULT LastCursorPos;

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

	if(DoOnce){
		DoOnce=FALSE;
		IsToBeLocked=(dxw.dwFlags1 & LOCKWINPOS);
		LastTimeShift=dxw.TimeShift;
	}

	// v2.1.93: adjust clipping region

	OutTraceW("WindowProc[%x]: WinMsg=[0x%x]%s(%x,%x)\n", hwnd, message, ExplainWinMessage(message), wparam, lparam);

#if 0
	if(dxw.dwFlags2 & WALLPAPERMODE) {
		static int t=0;
		if ((*pGetTickCount)() - t > 100){
			//if (GetTopWindow(NULL)==hwnd){
			{
				(*pSetWindowPos)(hwnd, HWND_BOTTOM, 0, 0, 0, 0, SWP_NOMOVE);
				t = (*pGetTickCount)();
			}
		}
		//if ((message >= WM_MOUSEFIRST) && (message <= WM_MOUSELAST)) return 0;
		//if (message == WM_WINDOWPOSCHANGED) {
		//	t=0;
		//	return 0;
		//}
	}
#endif

	if(dxw.dwFlags3 & FILTERMESSAGES){
		LRESULT ret;
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
			OutTraceW("WindowProc[%x]: WinMsg=%x filtered message=%x(%s)\n", hwnd, message, ExplainWinMessage(message));
			ret=0;
			return ret;
		}
	}

	if(dxw.dwFlags4 & STRETCHTIMERS){
		if(LastTimeShift != dxw.TimeShift) dxw.RenewTimers();
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
			OutTraceDW("WindowProc: SUPPRESS WinMsg=[0x%x]%s(%x,%x)\n", message, ExplainWinMessage(message), wparam, lparam);
			return 0;
		}
		break;
	case WM_NCHITTEST:
		if((dxw.dwFlags2 & FIXNCHITTEST) && (dxw.dwFlags1 & MODIFYMOUSE)){ // mouse processing 
			POINT cursor;
			LRESULT ret;
			ret=(*pDefWindowProcA)(hwnd, message, wparam, lparam);
			if (ret==HTCLIENT) {
				cursor.x=LOWORD(lparam);
				cursor.y=HIWORD(lparam);
				dxw.FixNCHITCursorPos(&cursor);
				lparam = MAKELPARAM(cursor.x, cursor.y); 
			}
			else
				return ret;
		}
		break;
	case WM_ERASEBKGND:
		if(dxw.Windowize && dxw.IsDesktop(hwnd)){ 
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
			LPWINDOWPOS wp;
			wp = (LPWINDOWPOS)lparam;
			dxwFixWindowPos("WindowProc", hwnd, lparam);
			OutTraceDW("WindowProc: WM_WINDOWPOSCHANGING fixed size=(%d,%d)\n", wp->cx, wp->cy);
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
		if(dxw.dwFlags1 & HIDEHWCURSOR) while((*pShowCursor)(0) >= 0);
		if(dxw.dwFlags2 & SHOWHWCURSOR) while((*pShowCursor)(1) < 0);
		if(dxw.dwFlags1 & ENABLECLIPPING) extClipCursor(lpClipRegion);
		if(dxw.dwFlags2 & REFRESHONRESIZE) dxw.ScreenRefresh();
		if(dxw.dwFlags4 & HIDEDESKTOP) dxw.HideDesktop(dxw.GethWnd());
		break;
	case WM_ACTIVATE:
		dxw.bActive = (LOWORD(wparam) == WA_ACTIVE || LOWORD(wparam) == WA_CLICKACTIVE) ? 1 : 0;
	case WM_NCACTIVATE:
		if(message == WM_NCACTIVATE) dxw.bActive = wparam;
		(*pSetWindowPos)(hwnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
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
			if (dxw.dwFlags1 & HIDEHWCURSOR) {
				(*pGetClientRect)(hwnd, &rect);
				if(prev.x >= 0 && prev.x < rect.right && prev.y >= 0 && prev.y < rect.bottom)
					while((*pShowCursor)(0) >= 0);
				else
					while((*pShowCursor)(1) < 0);
			}
			else {
				while((*pShowCursor)(1) < 0);
			}
			if(dxw.dwFlags1 & MODIFYMOUSE){ // mouse processing 
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
			if(dxw.dwFlags1 & MODIFYMOUSE){ // mouse processing 
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
		if (dxw.dwFlags1 & ENABLECLIPPING) extClipCursor(lpClipRegion);
		break;
	case WM_KILLFOCUS:
		OutTraceDW("WindowProc: hwnd=%x LOST FOCUS\n", hwnd);
		if (dxw.dwFlags1 & CLIPCURSOR) dxw.EraseClipCursor();
		if (dxw.dwFlags1 & ENABLECLIPPING) (*pClipCursor)(NULL);
		break;
	// commented out: WM_CLOSE just issue a request to close the window, not the process! It should be WM_QUIT....
	//case WM_CLOSE:
	//	OutTraceDW("WindowProc: WM_CLOSE - terminating process\n");
	//	if(dxw.dwFlags3 & FORCE16BPP) RecoverScreenMode();
	//	TerminateProcess(GetCurrentProcess(),0);
	//	break;
	case WM_SYSKEYDOWN:
		OutTraceW("event WM_SYSKEYDOWN wparam=%x lparam=%x\n", wparam, lparam);
		switch (wparam){
		case VK_F12:
			if(dxw.dwFlags1 & CLIPCURSOR){
				OutTraceDW("WindowProc: WM_SYSKEYDOWN key=%x ToggleState=%x\n",wparam,ClipCursorToggleState);
				ClipCursorToggleState = !ClipCursorToggleState;
				ClipCursorToggleState ? dxw.SetClipCursor() : dxw.EraseClipCursor();
			}
		case VK_F11:
			dxw.ScreenRefresh();
			break;
		case VK_F10:
			dx_ToggleLogging();
			break;
		case VK_F9:
			dx_TogglePositionLock(hwnd);
			break;
		//case VK_F8:
		//	dx_ToggleDC();
		//	break;
		case VK_F7:
			dx_ToggleFPS();
			break;
		case VK_F6:
		case VK_F5:
			if (dxw.dwFlags2 & TIMESTRETCH) {
				if (wparam == VK_F5 && (dxw.TimeShift <  8)) dxw.TimeShift++;
				if (wparam == VK_F6 && (dxw.TimeShift > -8)) dxw.TimeShift--;
				OutTrace("Time Stretch: shift=%d speed=%s\n", dxw.TimeShift, dxw.GetTSCaption());
				GetHookInfo()->TimeShift=dxw.TimeShift;
			}
			break;
		case VK_F4:
			if (dxw.dwFlags1 & HANDLEALTF4) {
				OutTraceDW("WindowProc: WM_SYSKEYDOWN(ALT-F4) - terminating process\n");
				TerminateProcess(GetCurrentProcess(),0);
			}
			break;
		default:
			break;
		}
	default:
		break;
	}
	if (dxw.dwFlags1 & AUTOREFRESH) dxw.ScreenRefresh();

	pWindowProc=WhndGetWindowProc(hwnd);
	//OutTraceB("WindowProc: pWindowProc=%x extWindowProc=%x message=%x(%s) wparam=%x lparam=%x\n", 
	//	(*pWindowProc), extWindowProc, message, ExplainWinMessage(message), wparam, lparam);
	if(pWindowProc) {
		LRESULT ret;

		// v2.02.36: use CallWindowProc that handles WinProc handles
		ret=(*pCallWindowProc)(pWindowProc, hwnd, message, wparam, lparam);
		
		// save last NCHITTEST cursor position for use with KEEPASPECTRATIO scaling
		if(message==WM_NCHITTEST) LastCursorPos=ret;
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

void HookSysLibsInit()
{
	static BOOL DoOnce = FALSE;
	if(DoOnce) return;
	DoOnce=TRUE;

	HookKernel32Init();
	HookUser32Init();
	HookGDI32Init();
}

DEVMODE InitDevMode;

static void SaveScreenMode()
{
	static BOOL DoOnce=FALSE;
	if(DoOnce) return;
	DoOnce=TRUE;
	EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &InitDevMode);
	OutTraceDW("DXWND: Initial display mode WxH=(%dx%d) BitsPerPel=%d\n", 
		InitDevMode.dmPelsWidth, InitDevMode.dmPelsHeight, InitDevMode.dmBitsPerPel);
}

static void RecoverScreenMode()
{
	DEVMODE CurrentDevMode;
	BOOL res;
	EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &CurrentDevMode);
	OutTraceDW("ChangeDisplaySettings: recover CURRENT WxH=(%dx%d) BitsPerPel=%d TARGET WxH=(%dx%d) BitsPerPel=%d\n", 
		CurrentDevMode.dmPelsWidth, CurrentDevMode.dmPelsHeight, CurrentDevMode.dmBitsPerPel,
		InitDevMode.dmPelsWidth, InitDevMode.dmPelsHeight, InitDevMode.dmBitsPerPel);
	InitDevMode.dmFields = (DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT);
	res=(*pChangeDisplaySettingsA)(&InitDevMode, 0);
	if(res) OutTraceE("ChangeDisplaySettings: ERROR err=%d at %d\n", GetLastError(), __LINE__);
}

void SwitchTo16BPP()
{
	DEVMODE CurrentDevMode;
	BOOL res;
	EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &CurrentDevMode);
	OutTraceDW("ChangeDisplaySettings: CURRENT wxh=(%dx%d) BitsPerPel=%d -> 16\n", 
		CurrentDevMode.dmPelsWidth, CurrentDevMode.dmPelsHeight, CurrentDevMode.dmBitsPerPel);
	CurrentDevMode.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;
	CurrentDevMode.dmBitsPerPel = 16;
	res=(*pChangeDisplaySettingsA)(&CurrentDevMode, CDS_UPDATEREGISTRY);
	if(res) OutTraceE("ChangeDisplaySettings: ERROR err=%d at %d\n", GetLastError(), __LINE__);
}

static void LockScreenMode(DWORD dmPelsWidth, DWORD dmPelsHeight, DWORD dmBitsPerPel)
{
	DEVMODE InitDevMode;
	BOOL res;
	OutTraceDW("ChangeDisplaySettings: LOCK wxh=(%dx%d) BitsPerPel=%d -> wxh=(%dx%d) BitsPerPel=%d\n", 
		InitDevMode.dmPelsWidth, InitDevMode.dmPelsHeight, InitDevMode.dmBitsPerPel,
		dmPelsWidth, dmPelsHeight, dmBitsPerPel);
	if( (dmPelsWidth != InitDevMode.dmPelsWidth) ||
		(dmPelsHeight !=InitDevMode.dmPelsHeight) ||
		(dmBitsPerPel != InitDevMode.dmBitsPerPel)){
		res=(*pChangeDisplaySettingsA)(&InitDevMode, 0);
		if(res) OutTraceE("ChangeDisplaySettings: ERROR err=%d at %d\n", GetLastError(), __LINE__);
	}
}

static HMODULE LoadDisasm()
{
	HMODULE disasmlib;
	#define MAX_FILE_PATH 512
	char sSourcePath[MAX_FILE_PATH+1];
	char *p;
	DWORD dwAttrib;	
	
	dwAttrib = GetFileAttributes("dxwnd.dll");
	if (dwAttrib != INVALID_FILE_ATTRIBUTES && !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY)) return NULL;
	GetModuleFileName(GetModuleHandle("dxwnd"), sSourcePath, MAX_FILE_PATH);
	p=&sSourcePath[strlen(sSourcePath)-strlen("dxwnd.dll")];
	strcpy(p, "disasm.dll");
	disasmlib=(*pLoadLibraryA)(sSourcePath);
	if(!disasmlib) {
		OutTraceDW("DXWND: Load lib=\"%s\" failed err=%d\n", sSourcePath, GetLastError());
		return NULL;
	}
	pGeterrwarnmessage=(Geterrwarnmessage_Type)(*pGetProcAddress)(disasmlib, "Geterrwarnmessage");
	pPreparedisasm=(Preparedisasm_Type)(*pGetProcAddress)(disasmlib, "Preparedisasm");
	pFinishdisasm=(Finishdisasm_Type)(*pGetProcAddress)(disasmlib, "Finishdisasm");
	pDisasm=(Disasm_Type)(*pGetProcAddress)(disasmlib, "Disasm");
	//OutTraceDW("DXWND: Load disasm.dll ptrs=%x,%x,%x,%x\n", pGeterrwarnmessage, pPreparedisasm, pFinishdisasm, pDisasm);

	return disasmlib;
}

LONG WINAPI myUnhandledExceptionFilter(LPEXCEPTION_POINTERS ExceptionInfo)
{
	OutTrace("UnhandledExceptionFilter: exception code=%x flags=%x addr=%x\n",
		ExceptionInfo->ExceptionRecord->ExceptionCode,
		ExceptionInfo->ExceptionRecord->ExceptionFlags,
		ExceptionInfo->ExceptionRecord->ExceptionAddress);
	DWORD oldprot;
	static HMODULE disasmlib = NULL;
	PVOID target = ExceptionInfo->ExceptionRecord->ExceptionAddress;
	switch(ExceptionInfo->ExceptionRecord->ExceptionCode){
	case 0xc0000094: // IDIV reg (Ultim@te Race Pro)
	case 0xc0000095: // DIV by 0 (divide overflow) exception (SonicR)
	case 0xc0000096: // CLI Priviliged instruction exception (Resident Evil)
	case 0xc000001d: // FEMMS (eXpendable)
	case 0xc0000005: // Memory exception (Tie Fighter)
		int cmdlen;
		t_disasm da;
		if(!disasmlib){
			if (!(disasmlib=LoadDisasm())) return EXCEPTION_CONTINUE_SEARCH;
			(*pPreparedisasm)();
		}
		if(!VirtualProtect(target, 10, PAGE_READWRITE, &oldprot)) return EXCEPTION_CONTINUE_SEARCH; // error condition
		cmdlen=(*pDisasm)((BYTE *)target, 10, 0, &da, 0, NULL, NULL);
		OutTrace("UnhandledExceptionFilter: NOP opcode=%x len=%d\n", *(BYTE *)target, cmdlen);
		memset((BYTE *)target, 0x90, cmdlen); 
		VirtualProtect(target, 10, oldprot, &oldprot);
		if(!FlushInstructionCache(GetCurrentProcess(), target, cmdlen))
			OutTrace("UnhandledExceptionFilter: FlushInstructionCache ERROR target=%x, err=%x\n", target, GetLastError());
		return EXCEPTION_CONTINUE_EXECUTION;
		break;
	default:
		return EXCEPTION_CONTINUE_SEARCH;
	}
}

LPTOP_LEVEL_EXCEPTION_FILTER WINAPI extSetUnhandledExceptionFilter(LPTOP_LEVEL_EXCEPTION_FILTER lpTopLevelExceptionFilter)
{
	OutTraceDW("SetUnhandledExceptionFilter: lpExceptionFilter=%x\n", lpTopLevelExceptionFilter);
	extern LONG WINAPI myUnhandledExceptionFilter(LPEXCEPTION_POINTERS);
	return (*pSetUnhandledExceptionFilter)(myUnhandledExceptionFilter);
}

//#define USEPERFCOUNTERS

unsigned __int64 inline GetRDTSC() {
   __asm {
      ; Flush the pipeline
      XOR eax, eax
      CPUID
      ; Get RDTSC counter in edx:eax
      RDTSC
   }
}

LONG CALLBACK Int3Handler(PEXCEPTION_POINTERS ExceptionInfo)
{
	// Vectored Handler routine to intercept INT3 opcodes replacing RDTSC
 	if (ExceptionInfo->ExceptionRecord->ExceptionCode == 0x80000003){
		LARGE_INTEGER myPerfCount;
#ifdef USEPERFCOUNTERS
		if(!pQueryPerformanceCounter) pQueryPerformanceCounter=QueryPerformanceCounter;
		(*pQueryPerformanceCounter)(&myPerfCount);
#else
	   __asm {
		  ; Flush the pipeline
		  XOR eax, eax
		  CPUID
		  ; Get RDTSC counter in edx:eax
		  RDTSC 
		  mov myPerfCount.LowPart, eax
		  mov myPerfCount.HighPart, edx
	   }
#endif
		//myPerfCount = dxw.StretchCounter(myPerfCount);
		myPerfCount = dxw.StretchLargeCounter(myPerfCount);
		OutTraceDW("Int3Handler: INT3 at=%x tick=%x RDTSC=%x:%x\n", 
			ExceptionInfo->ExceptionRecord->ExceptionAddress, (*pGetTickCount)(), myPerfCount.HighPart, myPerfCount.LowPart);

		ExceptionInfo->ContextRecord->Edx = myPerfCount.HighPart;
		ExceptionInfo->ContextRecord->Eax = myPerfCount.LowPart;
		ExceptionInfo->ContextRecord->Eip++; // skip ahead one-byte ( jump over 0xCC op-code )
		ExceptionInfo->ContextRecord->Eip++; // skip ahead one-byte ( jump over 0x90 op-code )
		return EXCEPTION_CONTINUE_EXECUTION;
	}
	return EXCEPTION_CONTINUE_SEARCH;
}

void HookExceptionHandler(void)
{
	void *tmp;
	HMODULE base;

	OutTraceDW("Set exception handlers\n");
	base=GetModuleHandle(NULL);
	pSetUnhandledExceptionFilter = SetUnhandledExceptionFilter;
	//v2.1.75 override default exception handler, if any....
	LONG WINAPI myUnhandledExceptionFilter(LPEXCEPTION_POINTERS);
	tmp = HookAPI(base, "KERNEL32.dll", UnhandledExceptionFilter, "UnhandledExceptionFilter", myUnhandledExceptionFilter);
	// so far, no need to save the previous handler, but anyway...
	tmp = HookAPI(base, "KERNEL32.dll", SetUnhandledExceptionFilter, "SetUnhandledExceptionFilter", extSetUnhandledExceptionFilter);
	if(tmp) pSetUnhandledExceptionFilter = (SetUnhandledExceptionFilter_Type)tmp;

	SetErrorMode(SEM_NOGPFAULTERRORBOX|SEM_NOOPENFILEERRORBOX|SEM_FAILCRITICALERRORS);
	(*pSetUnhandledExceptionFilter)((LPTOP_LEVEL_EXCEPTION_FILTER)myUnhandledExceptionFilter);
}

void HookModule(HMODULE base, int dxversion)
{
	HookKernel32(base);
	HookUser32(base);
	HookOle32(base);
	HookWinMM(base);
	//if(dxw.dwFlags2 & SUPPRESSIME) HookImeLib(module);
	HookGDI32(base);
	if(dxw.dwFlags1 & HOOKDI) HookDirectInput(base, dxversion);
	HookDirectDraw(base, dxversion);
	HookDirect3D(base, dxversion);
	HookDirect3D7(base, dxversion);
	if(dxw.dwFlags2 & HOOKOPENGL) HookOpenGLLibs(base, dxw.CustomOpenGLLib); 
	if(dxw.dwFlags4 & HOOKGLIDE) HookGlideLibs(base); 
	if((dxw.dwFlags3 & EMULATEREGISTRY) || (dxw.dwTFlags & OUTREGISTRY)) HookAdvApi32(base);
	HookMSV4WLibs(base); // -- used by Aliens & Amazons demo: what for?
}

void DisableIME()
{
	BOOL res;
	HMODULE hm;
	hm=GetModuleHandle("User32");
	// here, GetProcAddress may be not hooked yet!
	if(!pGetProcAddress) pGetProcAddress=GetProcAddress;
#ifdef USEWINNLSENABLE
	typedef BOOL (WINAPI *WINNLSEnableIME_Type)(HWND, BOOL);
	WINNLSEnableIME_Type pWINNLSEnableIME;
	pWINNLSEnableIME=(WINNLSEnableIME_Type)(*pGetProcAddress)(hm, "WINNLSEnableIME");
	OutTrace("DisableIME: GetProcAddress(WINNLSEnableIME)=%x\n", pWINNLSEnableIME);
	if(!pWINNLSEnableIME) return;
	SetLastError(0);
	res=(*pWINNLSEnableIME)(NULL, FALSE);
	OutTrace("IME previous state=%x error=%d\n", res, GetLastError());
#else
	typedef LRESULT (WINAPI *SendIMEMessage_Type)(HWND, LPARAM);
	SendIMEMessage_Type pSendIMEMessage;
	pSendIMEMessage=(SendIMEMessage_Type)(*pGetProcAddress)(hm, "SendIMEMessage");
	OutTrace("DisableIME: GetProcAddress(SendIMEMessage)=%x\n", pSendIMEMessage);
	if(!pSendIMEMessage) return;
	HGLOBAL imeh;
	IMESTRUCT *imes;
	imeh=GlobalAlloc(GMEM_MOVEABLE|GMEM_SHARE, sizeof(IMESTRUCT));
	imes=(IMESTRUCT *)imeh;
	//imes->fnc=IME_SETLEVEL;
	imes->fnc=7;
	imes->wParam=1;
	SetLastError(0);
	res=(*pSendIMEMessage)(dxw.GethWnd(), (LPARAM)imeh);
	OutTrace("res=%x error=%d\n", res, GetLastError());
#endif
}

void SetSingleProcessAffinity(void)
{
	int i;
	DWORD ProcessAffinityMask, SystemAffinityMask;
	if(!GetProcessAffinityMask(GetCurrentProcess(), &ProcessAffinityMask, &SystemAffinityMask))
		OutTraceE("GetProcessAffinityMask: ERROR err=%d\n", GetLastError());
	OutTraceDW("Process affinity=%x\n", ProcessAffinityMask);
	for (i=0; i<(8 * sizeof(DWORD)); i++){
		if (ProcessAffinityMask & 0x1) break;
		ProcessAffinityMask >>= 1;
	}
	OutTraceDW("First process affinity bit=%d\n", i);
	ProcessAffinityMask &= 0x1;
	for (; i; i--) ProcessAffinityMask <<= 1;
	OutTraceDW("Process affinity=%x\n", ProcessAffinityMask);
	if (!SetProcessAffinityMask(GetCurrentProcess(), ProcessAffinityMask))
		OutTraceE("SetProcessAffinityMask: ERROR err=%d\n", GetLastError());
}

extern HHOOK hMouseHook;

LRESULT CALLBACK MessageHook(int code, WPARAM wParam, LPARAM lParam)
{
	static BOOL SizeMoving = FALSE;

	if(code == HC_ACTION){
		if(dxw.IsFullScreen()){
			MSG *msg;
			msg = (MSG *)lParam;
			OutTraceC("MessageHook: hwnd=%x message=%d(%s) remove=%d pt=(%d,%d)\n", 
				msg->hwnd, msg->message, ExplainWinMessage(msg->message), msg->wParam, msg->pt.x, msg->pt.y);

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
				msg->pt=dxw.FixMessagePt(dxw.GethWnd(), msg->pt);
				//msg->pt=dxw.FixMessagePt(msg->hwnd, msg->pt);
				// beware: needs fix for mousewheel?
				if((msg->message <= WM_MOUSELAST) && (msg->message >= WM_MOUSEFIRST)) msg->lParam = MAKELPARAM(msg->pt.x, msg->pt.y); 
				OutTraceC("MessageHook: fixed lparam/pt=(%d,%d)\n", msg->pt.x, msg->pt.y);
				GetHookInfo()->CursorX=(short)msg->pt.x;
				GetHookInfo()->CursorY=(short)msg->pt.y;
			}
		}
	}
	return CallNextHookEx(hMouseHook, code, wParam, lParam);
}

static void ReplaceRDTSC()
{
	typedef BOOL (WINAPI *GetModuleInformation_Type)(HANDLE, HMODULE, LPMODULEINFO, DWORD);
	HMODULE disasmlib;
	unsigned char *opcodes;
	t_disasm da;
	MODULEINFO mi;
	HMODULE psapilib;
	GetModuleInformation_Type pGetModuleInformation;
	DWORD dwSegSize;
	DWORD oldprot;

	if (!(disasmlib=LoadDisasm())) return;

	// getting segment size
	psapilib=(*pLoadLibraryA)("psapi.dll");
	if(!psapilib) {
		OutTraceDW("DXWND: Load lib=\"%s\" failed err=%d\n", "psapi.dll", GetLastError());
		return;
	}
	pGetModuleInformation=(GetModuleInformation_Type)(*pGetProcAddress)(psapilib, "GetModuleInformation");
	(*pGetModuleInformation)(GetCurrentProcess(), GetModuleHandle(NULL), &mi, sizeof(mi));
	dwSegSize = mi.SizeOfImage;
	FreeLibrary(psapilib);

	(*pPreparedisasm)();
	opcodes=(unsigned char *)GetModuleHandle(NULL);
	unsigned int offset = 0;
	BOOL cont = TRUE;
	OutTraceDW("DXWND: opcode starting at addr=%x size=%x\n", opcodes, dwSegSize);
	while (TRUE) {
		int cmdlen;
		__try{
			cmdlen=(*pDisasm)(opcodes+offset,20,offset,&da,0,NULL,NULL);
			//OutTrace("offset=%x opcode=%x\n", offset, *(opcodes+offset));
		}
		__except (EXCEPTION_EXECUTE_HANDLER){
			OutTrace("exception at offset=%x\n", offset);
			cont=FALSE;
		}		
		if (cmdlen==0) break;
		// search for RDTSC opcode 0x0F31
		if((*(opcodes+offset) == 0x0F) && (*(opcodes+offset+1) == 0x31)){
			OutTraceDW("DXWND: RDTSC opcode found at addr=%x\n", (opcodes+offset));
			if(!VirtualProtect((LPVOID)(opcodes+offset), 4, PAGE_READWRITE, &oldprot)) {
				OutTrace("VirtualProtect ERROR: target=%x err=%d at %d\n", opcodes+offset, GetLastError(), __LINE__);
				return; // error condition
			}
			*(opcodes+offset) = 0xCC;	// __asm INT3
			*(opcodes+offset+1) = 0x90; // __asm NOP
			if(!VirtualProtect((LPVOID)(opcodes+offset), 4, oldprot, &oldprot)){
				OutTrace("VirtualProtect ERROR; target=%x, err=%d at %d\n", opcodes+offset, GetLastError(), __LINE__);
				return; // error condition
			}
		}
		// search for RDTSCP opcode 0x0F01F9
		if((*(opcodes+offset) == 0x0F) && (*(opcodes+offset+1) == 0x01) && (*(opcodes+offset+2) == 0xF9)){
			OutTraceDW("DXWND: RDTSC opcode found at addr=%x\n", (opcodes+offset));
			if(!VirtualProtect((LPVOID)(opcodes+offset), 4, PAGE_READWRITE, &oldprot)) {
				OutTrace("VirtualProtect ERROR: target=%x err=%d at %d\n", opcodes+offset, GetLastError(), __LINE__);
				return; // error condition
			}
			*(opcodes+offset) = 0xCC;	// __asm INT3
			*(opcodes+offset+1) = 0x90; // __asm NOP
			*(opcodes+offset+2) = 0x90; // __asm NOP
			if(!VirtualProtect((LPVOID)(opcodes+offset), 4, oldprot, &oldprot)){
				OutTrace("VirtualProtect ERROR; target=%x, err=%d at %d\n", opcodes+offset, GetLastError(), __LINE__);
				return; // error condition
			}
		}
		offset+=cmdlen; 
		if((offset+0x10) > dwSegSize) break;
	}

	return;
	(*pFinishdisasm)();
	FreeLibrary(disasmlib);
}

void HookInit(TARGETMAP *target, HWND hwnd)
{
	HMODULE base;
	char *sModule;
	char sModuleBuf[60+1];
	static char *dxversions[14]={
		"Automatic", "DirectX1~6", "", "", "", "", "", 
		"DirectX7", "DirectX8", "DirectX9", "DirectX10", "DirectX11", "None", ""
	};
	static char *Resolutions[]={
		"unlimited", "320x200", "640x480", "800x600", "1024x768", "1280x960", "" // terminator
	};
	
	dxw.InitTarget(target);

	if(dxw.dwFlags1 & AUTOMATIC) dxw.dwFlags1 |= EMULATESURFACE; // if AUTOMATIC, try this first!

	if(hwnd){ // v2/02.32: skip this when in code injection mode.
		// v2.1.75: is it correct to set hWnd here?
		//dxw.SethWnd(hwnd);
		dxw.hParentWnd=GetParent(hwnd);
		dxw.hChildWnd=hwnd;
		// v2.02.31: set main win either this one or the parent!
		dxw.SethWnd((dxw.dwFlags1 & FIXPARENTWIN) ? GetParent(hwnd) : hwnd);
	}

	if(IsTraceDW){
		OutTrace("HookInit: path=\"%s\" module=\"%s\" dxversion=%s pos=(%d,%d) size=(%d,%d)", 
			target->path, target->module, dxversions[dxw.dwTargetDDVersion], 
			target->posx, target->posy, target->sizx, target->sizy);
		if(hwnd) OutTrace(" hWnd=%x(hdc=%x) dxw.hParentWnd=%x(hdc=%x) desktop=%x(hdc=%x)\n", 
			hwnd, GetDC(hwnd), dxw.hParentWnd, GetDC(dxw.hParentWnd), GetDesktopWindow(), GetDC(GetDesktopWindow()));
		else OutTrace("\n");
		if (dxw.dwFlags4 & LIMITSCREENRES) OutTrace("HookInit: max resolution=%s\n", (dxw.MaxScreenRes<6)?Resolutions[dxw.MaxScreenRes]:"unknown");
	}

	if (hwnd && IsDebug){
		DWORD dwStyle, dwExStyle;
		dwStyle=GetWindowLong(dxw.GethWnd(), GWL_STYLE);
		dwExStyle=GetWindowLong(dxw.GethWnd(), GWL_EXSTYLE);
		OutTrace("HookInit: hWnd style=%x(%s) exstyle=%x(%s)\n", dwStyle, ExplainStyle(dwStyle), dwExStyle, ExplainExStyle(dwExStyle));
		dwStyle=GetWindowLong(dxw.hParentWnd, GWL_STYLE);
		dwExStyle=GetWindowLong(dxw.hParentWnd, GWL_EXSTYLE);
		OutTrace("HookInit: dxw.hParentWnd style=%x(%s) exstyle=%x(%s)\n", dwStyle, ExplainStyle(dwStyle), dwExStyle, ExplainExStyle(dwExStyle));
		OutTrace("HookInit: target window pos=(%d,%d) size=(%d,%d)\n", dxw.iPosX, dxw.iPosY, dxw.iSizX, dxw.iSizY);
	}

	HookSysLibsInit(); // this just once...

	base=GetModuleHandle(NULL);
	if (dxw.dwFlags3 & SINGLEPROCAFFINITY) SetSingleProcessAffinity();
	if (dxw.dwFlags4 & INTERCEPTRDTSC) AddVectoredExceptionHandler(1, Int3Handler); // 1 = first call, 0 = call last
	if (dxw.dwFlags1 & HANDLEEXCEPTIONS) HookExceptionHandler();
	if (dxw.dwTFlags & OUTIMPORTTABLE) DumpImportTable(base);
	if (dxw.dwFlags2 & SUPPRESSIME) DisableIME();

	//if ((hwnd==0) && (dxw.dwFlags4 & INTERCEPTRDTSC)) ReplaceRDTSC();
	if (dxw.dwFlags4 & INTERCEPTRDTSC) ReplaceRDTSC();

	if (dxw.dwTFlags & DXPROXED){
		HookDDProxy(base, dxw.dwTargetDDVersion);
		//return 0;
		return;
	}

	// make InitPosition used for both DInput and DDraw
	if(dxw.Windowize){
		InitPosition(target->initx, target->inity,
			target->minx, target->miny, target->maxx, target->maxy);
		dxw.InitWindowPos(target->posx, target->posy, target->sizx, target->sizy);
	}

	OutTraceB("HookInit: base hmodule=%x\n", base);
	HookModule(base, dxw.dwTargetDDVersion);
	if (dxw.dwFlags3 & HOOKDLLS) HookDlls(base);

	strncpy(sModuleBuf, dxw.gsModules, 60);
	sModule=strtok(sModuleBuf," ;");
	while (sModule) {
		base=(*pLoadLibraryA)(sModule);
		if(!base){
			OutTraceE("HookInit: LoadLibrary ERROR module=%s err=%d\n", sModule, GetLastError());
		}
		else {
			OutTraceDW("HookInit: hooking additional module=%s base=%x\n", sModule, base);
			if (dxw.dwTFlags & OUTIMPORTTABLE) DumpImportTable(base);
			HookModule(base, dxw.dwTargetDDVersion);
		}
		sModule=strtok(NULL," ;");
	}

	SaveScreenMode();
	if(dxw.dwFlags2 & RECOVERSCREENMODE) RecoverScreenMode();
	if(dxw.dwFlags3 & FORCE16BPP) SwitchTo16BPP();

	if (dxw.dwFlags1 & MESSAGEPROC){
		extern HINSTANCE hInst;
		hMouseHook=SetWindowsHookEx(WH_GETMESSAGE, MessageHook, hInst, GetCurrentThreadId());
		if(hMouseHook==NULL) OutTraceE("SetWindowsHookEx WH_GETMESSAGE failed: error=%d\n", GetLastError());
	}

	InitScreenParameters();
	if(hwnd) HookWindowProc(hwnd);
	// in fullscreen mode, messages seem to reach and get processed by the parent window
	if(!dxw.Windowize && hwnd) HookWindowProc(GetParent(hwnd));

	// initialize window: if
	// 1) not in injection mode (hwnd != 0) and
	// 2) in Windowed mode and
	// 3) supposedly in fullscreen mode (dxw.IsFullScreen()) and
	// 4) configuration ask for a overlapped bordered window (dxw.dwFlags1 & FIXWINFRAME) then
	// update window styles: just this window or, when FIXPARENTWIN is set, the father one as well.

	if (hwnd && dxw.Windowize && dxw.IsFullScreen() && (dxw.dwFlags1 & FIXWINFRAME)) {
		dxw.FixWindowFrame(dxw.hChildWnd);
		AdjustWindowPos(dxw.hChildWnd, target->sizx, target->sizy);
		if(dxw.dwFlags1 & FIXPARENTWIN) {
			dxw.FixWindowFrame(dxw.hParentWnd);
			AdjustWindowPos(dxw.hParentWnd, target->sizx, target->sizy);
		}
	}
}

LPCSTR ProcToString(LPCSTR proc)
{
	static char sBuf[24+1];
	if(((DWORD)proc & 0xFFFF0000) == 0){
		sprintf_s(sBuf, 24, "#0x%x", proc);
		return sBuf;
	}
	else
		return proc;
}

FARPROC RemapLibrary(LPCSTR proc, HMODULE hModule, HookEntry_Type *Hooks)
{
	for(; Hooks->APIName; Hooks++){
		if (!strcmp(proc,Hooks->APIName)){
			if (Hooks->StoreAddress) *(Hooks->StoreAddress)=(*pGetProcAddress)(hModule, proc);
			OutTraceDW("GetProcAddress: hooking proc=%s addr=%x->%x\n", 
				ProcToString(proc), (Hooks->StoreAddress) ? *(Hooks->StoreAddress) : 0, Hooks->HookerAddress);
			return Hooks->HookerAddress;
		}
	}
	return NULL;
}

void HookLibrary(HMODULE hModule, HookEntry_Type *Hooks, char *DLLName)
{
	void *tmp;
	for(; Hooks->APIName; Hooks++){
		tmp = HookAPI(hModule, DLLName, Hooks->OriginalAddress, Hooks->APIName, Hooks->HookerAddress);
		if(tmp && Hooks->StoreAddress) *(Hooks->StoreAddress) = (FARPROC)tmp;
	}
}

void HookLibInit(HookEntry_Type *Hooks)
{
	for(; Hooks->APIName; Hooks++)
		if (Hooks->StoreAddress) *(Hooks->StoreAddress) = Hooks->OriginalAddress;
}
