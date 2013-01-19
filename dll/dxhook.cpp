#define _WIN32_WINNT 0x0400
#define WIN32_LEAN_AND_MEAN
#define _CRT_SECURE_NO_DEPRECATE 1

#include <windows.h>
#include <stdio.h>
#include <stdarg.h>
#include "dxwnd.h"
#include "dxwcore.hpp"
#include "dxhook.h"
#include "glhook.h"
#define DXWDECLARATIONS 1
#include "syslibs.h"
#undef DXWDECLARATIONS
#include "dxhelper.h"

dxwCore dxw;

extern void InitScreenParameters();

extern HANDLE hTraceMutex;

CRITICAL_SECTION TraceCS; 

static char *FlagNames[32]={
	"UNNOTIFY", "EMULATESURFACE", "CLIPCURSOR", "RESETPRIMARY",
	"HOOKDI", "MODIFYMOUSE", "HANDLEEXCEPTIONS", "SAVELOAD",
	"EMULATEBUFFER", "HANDLEDC", "BLITFROMBACKBUFFER", "SUPPRESSCLIPPING",
	"AUTOREFRESH", "FIXWINFRAME", "HIDEHWCURSOR", "SLOWDOWN",
	"ENABLECLIPPING", "LOCKWINSTYLE", "MAPGDITOPRIMARY", "FIXTEXTOUT",
	"KEEPCURSORWITHIN", "USERGB565", "SUPPRESSDXERRORS", "PREVENTMAXIMIZE",
	"ONEPIXELFIX", "FIXPARENTWIN", "SWITCHVIDEOMEMORY", "CLIENTREMAPPING",
	"HANDLEALTF4", "LOCKWINPOS", "HOOKCHILDWIN", "MESSAGEPROC"
};

static char *Flag2Names[32]={
	"RECOVERSCREENMODE", "REFRESHONRESIZE", "BACKBUFATTACH", "MODALSTYLE",
	"KEEPASPECTRATIO", "INIT8BPP", "FORCEWINRESIZE", "INIT16BPP",
	"KEEPCURSORFIXED", "DISABLEGAMMARAMP", "DIFFERENTIALMOUSE", "FIXNCHITTEST",
	"LIMITFPS", "SKIPFPS", "SHOWFPS", "HIDEMULTIMONITOR",
	"TIMESTRETCH", "HOOKOPENGL", "WALLPAPERMODE", "SHOWHWCURSOR",
	"HOOKGDI", "", "", "",
	"", "", "", "",
	"", "", "", "",
};

static char *TFlagNames[32]={
	"OUTTRACE", "OUTDDRAWTRACE", "OUTWINMESSAGES", "OUTCURSORTRACE",
	"OUTPROXYTRACE", "DXPROXED", "ASSERTDIALOG", "OUTIMPORTTABLE",
	"OUTDEBUG", "", "", "",
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
		OutTraceD("Toggle logging OFF\n");
		dxw.dwTFlags &= ~OUTTRACE;
	}
	else {
		dxw.dwTFlags |= OUTTRACE;
		OutTraceD("Toggle logging ON\n");
	}
	DxWndStatus.isLogging=(dxw.dwTFlags & OUTTRACE);
	SetHookStatus(&DxWndStatus);
}

void DumpImportTable(char *module)
{
	DWORD base;
	PIMAGE_NT_HEADERS pnth;
	PIMAGE_IMPORT_DESCRIPTOR pidesc;
	DWORD rva;
	PSTR impmodule;
	PIMAGE_THUNK_DATA ptaddr;
	PIMAGE_THUNK_DATA ptname;
	PIMAGE_IMPORT_BY_NAME piname;

	base = (DWORD)GetModuleHandle(module);
	OutTrace("DumpImportTable: base=%x\n", base);
	if(!base) {
		OutTrace("DumpImportTable: GetModuleHandle failed, err=%d at %d\n",GetLastError(), __LINE__);
		return;
	}
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
		OutTraceD("DumpImportTable: EXCEPTION\n");
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
	
	// keep track of hooked call range to avoid re-hooking of hooked addresses !!!
	if ((DWORD)hookproc < MinHook) MinHook=(DWORD)hookproc;
	if ((DWORD)hookproc > MaxHook) MaxHook=(DWORD)hookproc;
	dwTmp = *(DWORD *)target;
	if(dwTmp == (DWORD)hookproc) return; // already hooked
	if((dwTmp <= MaxHook) && (dwTmp >= MinHook)) return; // already hooked
	if(!VirtualProtect(target, 4, PAGE_READWRITE, &oldprot)) return; // error condition
	*(DWORD *)target = (DWORD)hookproc;
	VirtualProtect(target, 4, oldprot, &oldprot);
	tmp=(void *)dwTmp;

	if (*hookedproc && *hookedproc!=tmp) {
		sprintf(msg,"SetHook: proc=%s oldhook=%x newhook=%x\n", hookname, hookedproc, tmp);
		OutTraceD(msg);
		if (IsAssertEnabled) MessageBox(0, msg, "SetHook", MB_OK | MB_ICONEXCLAMATION);
	}
	*hookedproc = tmp;
}

void *HookAPI(const char *module, char *dll, void *apiproc, const char *apiname, void *hookproc)
{
	DWORD base;
	PIMAGE_NT_HEADERS pnth;
	PIMAGE_IMPORT_DESCRIPTOR pidesc;
	DWORD rva;
	PSTR impmodule;
	PIMAGE_THUNK_DATA ptaddr;
	PIMAGE_THUNK_DATA ptname;
	PIMAGE_IMPORT_BY_NAME piname;
	DWORD oldprotect;
	void *org;

	if(!*apiname) { // check
		char *sMsg="HookAPI: NULL api name\n";
		OutTraceE(sMsg);
		if (IsAssertEnabled) MessageBox(0, sMsg, "HookAPI", MB_OK | MB_ICONEXCLAMATION);
		return 0;
	}

	base = (DWORD)GetModuleHandle(module);
	if(!base) {
		OutTraceD("HookAPI: GetModuleHandle failed, error=%d\n",GetLastError());
		return 0;
	}
	__try{
		pnth = PIMAGE_NT_HEADERS(PBYTE(base) + PIMAGE_DOS_HEADER(base)->e_lfanew);
		if(!pnth) {
			OutTraceE("HookAPI: ERROR no pnth at %d\n", __LINE__);
			return 0;
		}
		rva = pnth->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress;
		if(!rva) {
			OutTraceE("HookAPI: ERROR no rva at %d\n", __LINE__);
			return 0;
		}
		pidesc = (PIMAGE_IMPORT_DESCRIPTOR)(base + rva);

		while(pidesc->FirstThunk){
			impmodule = (PSTR)(base + pidesc->Name);
			if(!lstrcmpi(dll, impmodule)) break;
			pidesc ++;
		}
		if(!pidesc->FirstThunk) {
			if (IsDebug) OutTraceD("HookAPI: PE unreferenced dll=%s\n", dll);
			return 0;
		}

		ptaddr = (PIMAGE_THUNK_DATA)(base + (DWORD)pidesc->FirstThunk);
		ptname = (pidesc->OriginalFirstThunk) ? (PIMAGE_THUNK_DATA)(base + (DWORD)pidesc->OriginalFirstThunk) : NULL;

		if((apiproc==NULL) && (ptname==NULL)){
			if (IsDebug) OutTraceD("HookAPI: unreacheable api=%s dll=%s\n", apiname, dll);
			return 0;
		}

		while(ptaddr->u1.Function){
			if (ptname){
				if(!IMAGE_SNAP_BY_ORDINAL(ptname->u1.Ordinal)){
					piname = (PIMAGE_IMPORT_BY_NAME)(base + (DWORD)ptname->u1.AddressOfData);
					if(!lstrcmpi(apiname, (char *)piname->Name)) break;
				}
			}
			if (apiproc){
				if(ptaddr->u1.Function == (DWORD)apiproc) break;
			}
			ptaddr ++;
			if (ptname) ptname ++;
		}
		if(!ptaddr->u1.Function) return 0;

		org = (void *)ptaddr->u1.Function;
		if(org == hookproc) return 0; // already hooked
			
		if(!VirtualProtect(&ptaddr->u1.Function, 4, PAGE_EXECUTE_READWRITE, &oldprotect)) {
			OutTraceD("HookAPI: VirtualProtect error %d at %d\n", GetLastError(), __LINE__);
			return 0;
		}
		ptaddr->u1.Function = (DWORD)hookproc;
		if(!VirtualProtect(&ptaddr->u1.Function, 4, oldprotect, &oldprotect)) {
			OutTraceD("HookAPI: VirtualProtect error %d at %d\n", GetLastError(), __LINE__);
			return 0;
		}
		if (!FlushInstructionCache(GetCurrentProcess(), &ptaddr->u1.Function, 4)) {
			OutTraceD("HookAPI: FlushInstructionCache error %d at %d\n", GetLastError(), __LINE__);
			return 0;
		}
		//OutTraceD("HookAPI hook=%s.%s address=%x->%x\n", module, apiname, org, hookproc);
	 }
	__except(EXCEPTION_EXECUTE_HANDLER)
	{       
		OutTraceD("HookAPI: EXCEPTION hook=%s:%s Hook Failed.\n", dll, apiname);
		org = 0;
	}
	return org;
}

// v.2.1.80: unified positioning logic into CalculateWindowPos routine
// now taking in account for window menus (see "Alien Cabal")

void CalculateWindowPos(HWND hwnd, DWORD width, DWORD height, LPWINDOWPOS wp)
{
	RECT rect;
	DWORD dwStyle;
	int MaxX, MaxY;
	HMENU hMenu;

	rect.left =  dxw.iPosX;
	rect.top = dxw.iPosX;
	MaxX = dxw.iSizX;
	MaxY = dxw.iSizY;
	if (!MaxX) MaxX = width;
	if (!MaxY) MaxY = height;
	rect.right = dxw.iPosX+MaxX;
	rect.bottom = dxw.iPosX+MaxY;

	dwStyle=(*pGetWindowLong)(hwnd, GWL_STYLE);
	hMenu = GetMenu(hwnd);	
	AdjustWindowRect(&rect, dwStyle, (hMenu!=NULL));

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
	OutTraceD("AdjustWindowPos: hwnd=%x, size=(%d,%d)\n", hwnd, width, height);
	CalculateWindowPos(hwnd, width, height, &wp);
	OutTraceD("AdjustWindowPos: fixed pos=(%d,%d) size=(%d,%d)\n", wp.x, wp.y, wp.cx, wp.cy);
	if(!(*pSetWindowPos)(hwnd, 0, wp.x, wp.y, wp.cx, wp.cy, 0)){
		OutTraceE("AdjustWindowPos: ERROR err=%d at %d\n", GetLastError(), __LINE__);
	}
	return;
}

void AdjustWindowFrame(HWND hwnd, DWORD width, DWORD height)
{

	HRESULT res=0;
	WNDPROC pWindowProc;

	OutTraceD("AdjustWindowFrame hwnd=%x, wxh=%dx%d\n", hwnd, width, height); 

	dxw.SetScreenSize(width, height);
	if (hwnd==NULL) return;

	(*pSetWindowLong)(hwnd, GWL_STYLE, (dxw.dwFlags2 & MODALSTYLE) ? 0 : WS_OVERLAPPEDWINDOW);
	(*pSetWindowLong)(hwnd, GWL_EXSTYLE, 0); 
	(*pShowWindow)(hwnd, SW_SHOWNORMAL);
	OutTraceD("AdjustWindowFrame hwnd=%x, set style=WS_OVERLAPPEDWINDOW extstyle=0\n", hwnd); 
	AdjustWindowPos(hwnd, width, height);

	// fixing windows message handling procedure

	pWindowProc = (WNDPROC)(*pGetWindowLong)(hwnd, GWL_WNDPROC);
	if (((DWORD)pWindowProc & 0xFFFF0000) == 0xFFFF0000){
		// don't hook pseudo-callbacks (v2.1.71: Commandos 2)
		OutTraceD("GetWindowLong: no valid WindowProc routine detected, hwnd=%x WindowProc=%x\n", hwnd, (DWORD)pWindowProc);
	}
	else
	if (pWindowProc == extWindowProc){
		// hooked already !!!
		OutTraceD("GetWindowLong: extWindowProc already in place, hwnd=%x\n", hwnd);
	}
	else {// don't hook twice ....
		long lres;
		WhndStackPush(hwnd, pWindowProc);
		lres=(*pSetWindowLong)(hwnd, GWL_WNDPROC, (LONG)extWindowProc);
		OutTraceD("AdjustWindowFrame: fixing hwnd=%x WindowProc=%x->%x\n", hwnd, lres, (LONG)extWindowProc);
	}

	// fixing cursor view and clipping region

	if (dxw.dwFlags1 & HIDEHWCURSOR) while ((*pShowCursor)(0) >= 0);
	if (dxw.dwFlags2 & SHOWHWCURSOR) while((*pShowCursor)(1) < 0);
	if (dxw.dwFlags1 & CLIPCURSOR) {
		OutTraceD("AdjustWindowFrame: setting clip region\n");
		dxw.SetClipCursor();
	}

	(*pInvalidateRect)(hwnd, NULL, TRUE);
}

INT_PTR CALLBACK extDialogWindowProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
{
	static int i=0;
	static WINDOWPOS *wp;
	WNDPROC pWindowProc;

	static int t = -1;
	if (t == -1)
		t = GetTickCount();
	int tn = GetTickCount();

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
	OutTraceD(sMsg);
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
	// Beware: Cybermercs handles some static info about curror position handling, so that if you resize
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
			//OutTraceC("ChildWindowProc: hwnd=%x pos XY prev=(%d,%d)\n", hwnd, prev.x, prev.y);
			curr=dxw.FixCursorPos(prev); // Warn! the correction must refer to the main window hWnd, not the current hwnd one !!!
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

static void dx_TogglePositionLock(HWND hwnd)
{
	// toggle position locking
	if(dxw.dwFlags1 & LOCKWINPOS){
		// unlock
		OutTraceD("Toggle position lock OFF\n");
		dxw.dwFlags1 &= ~LOCKWINPOS;
	}
	else {
		// lock and update window position!!!
		// v2.1.80: fixed 
		RECT rect;
		POINT p={0,0};
		(*pGetClientRect)(hwnd,&rect);
		(*pClientToScreen)(hwnd,&p);
		dxw.dwFlags1 |= LOCKWINPOS;
		OutTraceD("Toggle position lock ON\n");
		dxw.InitWindowPos(p.x, p.y, rect.right-rect.left, rect.bottom-rect.top);
	}
}

void dx_ToggleDC()
{
	if(dxw.dwFlags1 & HANDLEDC){
		dxw.dwFlags1 &= ~HANDLEDC;
		OutTrace("ToggleDC: HANDLEDC mode OFF\n");
	}
	else {
		dxw.dwFlags1 |= HANDLEDC;
		OutTrace("ToggleDC: HANDLEDC mode ON\n");
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

	// v2.1.93: adjust clipping region

	OutTraceW("WindowProc: WinMsg=[0x%x]%s(%x,%x)\n", message, ExplainWinMessage(message), wparam, lparam);

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

	switch(message){
	case WM_NCHITTEST:
		if((dxw.dwFlags2 & FIXNCHITTEST) && (dxw.dwFlags1 & MODIFYMOUSE)){ // mouse processing 
			POINT cursor;
			LRESULT ret;
			ret=(*pDefWindowProc)(hwnd, message, wparam, lparam);
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
		OutTraceD("WindowProc: WM_ERASEBKGND(%x,%x) - suppressed\n", wparam, lparam);
		return 1; // 1 == OK, erased
		break;
	case WM_DISPLAYCHANGE:
		if ((dxw.dwFlags1 & LOCKWINPOS) && dxw.IsFullScreen()){
			OutTraceD("WindowProc: prevent WM_DISPLAYCHANGE depth=%d size=(%d,%d)\n",
				wparam, HIWORD(lparam), LOWORD(lparam));
			return 0;
		}
		break;
	case WM_WINDOWPOSCHANGING:
		dxwFixWindowPos("WindowProc", hwnd, lparam);
		break;
	case WM_WINDOWPOSCHANGED:
		dxwFixWindowPos("WindowProc", hwnd, lparam);
		break;
	case WM_ENTERSIZEMOVE:
		while((*pShowCursor)(1) < 0);
		if(dxw.dwFlags1 & CLIPCURSOR) dxw.EraseClipCursor();
		if(dxw.dwFlags1 & ENABLECLIPPING) (*pClipCursor)(NULL);
		break;
	case WM_EXITSIZEMOVE:
		if (dxw.dwFlags1 & HIDEHWCURSOR) while((*pShowCursor)(0) >= 0);
		if (dxw.dwFlags2 & SHOWHWCURSOR) while((*pShowCursor)(1) < 0);
		if(dxw.dwFlags1 & ENABLECLIPPING) extClipCursor(lpClipRegion);
		if(dxw.dwFlags2 & REFRESHONRESIZE) dxw.ScreenRefresh();
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
			curr=dxw.FixCursorPos(hwnd, prev);
			lparam = MAKELPARAM(curr.x, curr.y); 
			OutTraceC("WindowProc: hwnd=%x pos XY=(%d,%d)->(%d,%d)\n", hwnd, prev.x, prev.y, curr.x, curr.y);
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
		if((dxw.dwFlags1 & CLIPCURSOR) && ClipCursorToggleState) dxw.SetClipCursor();
		if(dxw.dwFlags1 & MODIFYMOUSE){ // mouse processing 
			// scale mouse coordinates
			prev.x = LOWORD(lparam);
			prev.y = HIWORD(lparam);
			curr=dxw.FixCursorPos(hwnd, prev);
			lparam = MAKELPARAM(curr.x, curr.y); 
			OutTraceC("WindowProc: hwnd=%x pos XY=(%d,%d)->(%d,%d)\n", hwnd, prev.x, prev.y, curr.x, curr.y);
		}
		break;	
	case WM_SETFOCUS:
		if (dxw.dwFlags1 & ENABLECLIPPING) extClipCursor(lpClipRegion);
		break;
	case WM_KILLFOCUS:
		if (dxw.dwFlags1 & CLIPCURSOR) dxw.EraseClipCursor();
		if (dxw.dwFlags1 & ENABLECLIPPING) (*pClipCursor)(NULL);
		break;
	case WM_CLOSE:
		OutTraceD("WindowProc: WM_CLOSE - terminating process\n");
		TerminateProcess(GetCurrentProcess(),0);
		break;
	case WM_SYSKEYDOWN:
		OutTraceW("event WM_SYSKEYDOWN wparam=%x lparam=%x\n", wparam, lparam);
		switch (wparam){
		case VK_F12:
			if(dxw.dwFlags1 & CLIPCURSOR){
				OutTraceD("WindowProc: WM_SYSKEYDOWN key=%x ToggleState=%x\n",wparam,ClipCursorToggleState);
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
		case VK_F8:
			dx_ToggleDC();
			break;
		case VK_F4:
			if (dxw.dwFlags1 & HANDLEALTF4) {
				OutTraceD("WindowProc: WM_SYSKEYDOWN(ALT-F4) - terminating process\n");
				TerminateProcess(GetCurrentProcess(),0);
			}
			break;
		case VK_F5:
		case VK_F6:
			if (dxw.dwFlags2 & TIMESTRETCH) {
				char *sTSCaption[9]={"x16","x8","x4","x2","x1",":2",":4",":8",":16"};
				if (wparam == VK_F5 && (dxw.TimeShift <  4)) dxw.TimeShift++;
				if (wparam == VK_F6 && (dxw.TimeShift > -4)) dxw.TimeShift--;
				OutTrace("Time Stretch: shift=%d speed=%s\n", dxw.TimeShift, sTSCaption[dxw.TimeShift+4]);
				DxWndStatus.iTimeShift=dxw.TimeShift;
				SetHookStatus(&DxWndStatus);
			}
			break;
		default:
			break;
		}
	default:
		break;
	}
	if (dxw.dwFlags1 & AUTOREFRESH)
	{
		dxw.ScreenRefresh();
	}

	pWindowProc=WhndGetWindowProc(hwnd);
	if(pWindowProc) {
		LRESULT ret;
		ret=(*pWindowProc)(hwnd, message, wparam, lparam);
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
				ret=(*pDefWindowProc)(hwnd, message, wparam, lparam);
				break;
			}
		}

		return ret;
	}

	//OutTraceD("ASSERT: WindowProc mismatch hwnd=%x\n", hwnd);
	// ??? maybe it's a normal condition, whenever you don't have a WindowProc routine
	// like in Commandos 2. Flag it?
	char sMsg[81];
	sprintf(sMsg,"ASSERT: WindowProc mismatch hwnd=%x\n", hwnd);
	OutTraceD(sMsg);
	if (IsAssertEnabled) MessageBox(0, sMsg, "WindowProc", MB_OK | MB_ICONEXCLAMATION);	
	return (*pDefWindowProc)(hwnd, message, wparam, lparam);
}

void HookSysLibsInit()
{
	pLoadLibraryA = LoadLibrary;
	pGetProcAddress = (GetProcAddress_Type)GetProcAddress;
	pGDICreateCompatibleDC=CreateCompatibleDC;
	pGDIDeleteDC=DeleteDC;
	pGDIGetDC=GetDC;
	pGDIGetWindowDC=GetWindowDC;
	pGDIReleaseDC=ReleaseDC;
	pGDICreateDC=CreateDC;
	pGDIBitBlt=BitBlt;
	pGDIStretchBlt=StretchBlt;
	pBeginPaint=BeginPaint;
	pEndPaint=EndPaint;
	pInvalidateRect=InvalidateRect;
	pScreenToClient = ScreenToClient;
	pClientToScreen = ClientToScreen;
	pGetClientRect  = GetClientRect;
	pGetWindowRect  = GetWindowRect;
	pMapWindowPoints= MapWindowPoints;
	pChangeDisplaySettings=ChangeDisplaySettingsA;
	pChangeDisplaySettingsEx=ChangeDisplaySettingsExA;
	pClipCursor = ClipCursor;
	pFillRect = FillRect;
	pPeekMessage = PeekMessage;
	pGetMessage = GetMessage;
	pDefWindowProc = DefWindowProc;
	pGDIGetDeviceCaps = GetDeviceCaps;
	pGDITextOutA = TextOutA;
	pGDIScaleWindowExtEx = ScaleWindowExtEx;
	pCreateWindowExA = CreateWindowExA;
	pRegisterClassExA = (RegisterClassExA_Type)RegisterClassExA;
	pGDIRectangle = Rectangle;
	pSetWindowPos=SetWindowPos;
	pGDIDeferWindowPos=DeferWindowPos;
	pSetWindowLong=SetWindowLongA;
	pGetWindowLong=GetWindowLongA;
	pCallWindowProc=CallWindowProcA;
	pShowWindow=ShowWindow;
	pGDISetTextColor = SetTextColor;
	pGDISetBkColor = SetBkColor;
	pGDICreateFont = CreateFont;
	pGDICreateFontIndirect = CreateFontIndirect;
	pGetSystemMetrics = GetSystemMetrics;
	pGetCursorPos = GetCursorPos;
	pSetCursorPos = SetCursorPos;
	pSetCursor    = SetCursor;
	pCreateDialogIndirectParam=CreateDialogIndirectParamA;
	pCreateDialogParam=CreateDialogParamA;
	pMoveWindow=MoveWindow;
	pGetDesktopWindow=GetDesktopWindow;
	pShowCursor=ShowCursor;
	pGetTickCount=GetTickCount;
	pSleep=Sleep;
	pSleepEx=SleepEx;
	pGetSystemTime=GetSystemTime;
	pGetLocalTime=GetLocalTime;
	pSetTimer=SetTimer;
}

void HookGDILib(char *module)
{
	void *tmp;

	if(dxw.dwFlags1 & MAPGDITOPRIMARY){
		tmp = HookAPI(module, "GDI32.dll", CreateCompatibleDC, "CreateCompatibleDC", extDDCreateCompatibleDC);
		if(tmp) pGDICreateCompatibleDC = (CreateCompatibleDC_Type)tmp;
		tmp = HookAPI(module, "GDI32.dll", DeleteDC, "DeleteDC", extDDDeleteDC);
		if(tmp) pGDIDeleteDC = (DeleteDC_Type)tmp;
		tmp = HookAPI(module, "GDI32.dll", CreateDCA, "CreateDCA", extDDCreateDC);
		if(tmp) pGDICreateDC = (CreateDC_Type)tmp;
		tmp = HookAPI(module, "GDI32.dll", BitBlt, "BitBlt", extDDBitBlt);
		if(tmp) pGDIBitBlt = (BitBlt_Type)tmp;
		tmp = HookAPI(module, "GDI32.dll", StretchBlt, "StretchBlt", extDDStretchBlt);
		if(tmp) pGDIStretchBlt = (StretchBlt_Type)tmp;
	}
	else {
		tmp = HookAPI(module, "GDI32.dll", CreateCompatibleDC, "CreateCompatibleDC", extGDICreateCompatibleDC);
		if(tmp) pGDICreateCompatibleDC = (CreateCompatibleDC_Type)tmp;
		tmp = HookAPI(module, "GDI32.dll", DeleteDC, "DeleteDC", extGDIDeleteDC);
		if(tmp) pGDIDeleteDC = (DeleteDC_Type)tmp;
		tmp = HookAPI(module, "GDI32.dll", CreateDCA, "CreateDCA", extGDICreateDC);
		if(tmp) pGDICreateDC = (CreateDC_Type)tmp;
		tmp = HookAPI(module, "GDI32.dll", BitBlt, "BitBlt", extGDIBitBlt);
		if(tmp) pGDIBitBlt = (BitBlt_Type)tmp;
		tmp = HookAPI(module, "GDI32.dll", StretchBlt, "StretchBlt", extGDIStretchBlt);
		if(tmp) pGDIStretchBlt = (StretchBlt_Type)tmp;
		tmp = HookAPI(module, "GDI32.dll", PatBlt, "PatBlt", extGDIPatBlt);
		if(tmp) pGDIPatBlt = (PatBlt_Type)tmp;
	}
	tmp = HookAPI(module, "GDI32.dll", GetDeviceCaps, "GetDeviceCaps", extGetDeviceCaps); // GHO: added for caesar3
	if(tmp) pGDIGetDeviceCaps = (GetDeviceCaps_Type)tmp; 
	tmp = HookAPI(module, "GDI32.dll", TextOutA, "TextOutA", extTextOutA);
	if(tmp) pGDITextOutA = (TextOut_Type)tmp;
	tmp = HookAPI(module, "GDI32.dll", ScaleWindowExtEx, "ScaleWindowExtEx", extScaleWindowExtEx);
	if(tmp) pGDIScaleWindowExtEx = (ScaleWindowExtEx_Type)tmp;
	tmp = HookAPI(module, "GDI32.dll", Rectangle, "Rectangle", extRectangle);
	if(tmp) pGDIRectangle = (Rectangle_Type)tmp;
	tmp = HookAPI(module, "GDI32.dll", SaveDC, "SaveDC", extGDISaveDC);
	if(tmp) pGDISaveDC = (SaveDC_Type)tmp;
	tmp = HookAPI(module, "GDI32.dll", RestoreDC, "RestoreDC", extGDIRestoreDC);
	if(tmp) pGDIRestoreDC = (RestoreDC_Type)tmp;
	tmp = HookAPI(module, "GDI32.dll", CreatePalette, "CreatePalette", extGDICreatePalette);
	if(tmp) pGDICreatePalette = (GDICreatePalette_Type)tmp;
	tmp = HookAPI(module, "GDI32.dll", SelectPalette, "SelectPalette", extSelectPalette);
	if(tmp) pGDISelectPalette = (SelectPalette_Type)tmp;
	tmp = HookAPI(module, "GDI32.dll", RealizePalette, "RealizePalette", extRealizePalette);
	if(tmp) pGDIRealizePalette = (RealizePalette_Type)tmp;
	tmp = HookAPI(module, "GDI32.dll", GetSystemPaletteEntries, "GetSystemPaletteEntries", extGetSystemPaletteEntries);
	if(tmp) pGDIGetSystemPaletteEntries = (GetSystemPaletteEntries_Type)tmp;
	if ((dxw.dwFlags1 & EMULATESURFACE) && (dxw.dwFlags1 & HANDLEDC)){
		tmp = HookAPI(module, "GDI32.dll", SetTextColor, "SetTextColor", extSetTextColor);
		if(tmp) pGDISetTextColor = (SetTextColor_Type)tmp;
		tmp = HookAPI(module, "GDI32.dll", SetBkColor, "SetBkColor", extSetBkColor);
		if(tmp) pGDISetBkColor = (SetBkColor_Type)tmp;
		tmp = HookAPI(module, "GDI32.dll", CreateFont, "CreateFont", extCreateFont);
		if(tmp) pGDICreateFont = (CreateFont_Type)tmp;
		tmp = HookAPI(module, "GDI32.dll", CreateFontIndirectA, "CreateFontIndirectA", extCreateFontIndirect);
		if(tmp) pGDICreateFontIndirect = (CreateFontIndirect_Type)tmp;
	}

	if(dxw.dwFlags2 & DISABLEGAMMARAMP){
		tmp = HookAPI(module, "GDI32.dll", SetDeviceGammaRamp, "SetDeviceGammaRamp", extSetDeviceGammaRamp);
		if(tmp) pGDISetDeviceGammaRamp = (SetDeviceGammaRamp_Type)tmp;
		tmp = HookAPI(module, "GDI32.dll", GetDeviceGammaRamp, "GetDeviceGammaRamp", extGetDeviceGammaRamp);
		if(tmp) pGDIGetDeviceGammaRamp = (GetDeviceGammaRamp_Type)tmp;
	}
}

void HookSysLibs(char *module)
{
	void *tmp;

	tmp = HookAPI(module, "kernel32.dll", GetProcAddress, "GetProcAddress", extGetProcAddress);
	if(tmp) pGetProcAddress = (GetProcAddress_Type)tmp;
	tmp = HookAPI(module, "kernel32.dll", LoadLibraryA, "LoadLibraryA", extLoadLibraryA);
	if(tmp) pLoadLibraryA = (LoadLibraryA_Type)tmp;
	tmp = HookAPI(module, "kernel32.dll", LoadLibraryExA, "LoadLibraryExA", extLoadLibraryExA);
	if(tmp) pLoadLibraryExA = (LoadLibraryExA_Type)tmp;

	tmp = HookAPI(module, "user32.dll", BeginPaint, "BeginPaint", extBeginPaint);
	if(tmp) pBeginPaint = (BeginPaint_Type)tmp;
	tmp = HookAPI(module, "user32.dll", EndPaint, "EndPaint", extEndPaint);
	if(tmp) pEndPaint = (EndPaint_Type)tmp;
	tmp = HookAPI(module, "user32.dll", ShowCursor, "ShowCursor", extShowCursor);
	if(tmp) pShowCursor = (ShowCursor_Type)tmp;
	if(dxw.dwFlags1 & MAPGDITOPRIMARY){
		tmp = HookAPI(module, "user32.dll", GetDC, "GetDC", extDDGetDC);
		if(tmp) pGDIGetDC = (GDIGetDC_Type)tmp;
		tmp = HookAPI(module, "user32.dll", GetWindowDC, "GetWindowDC", extDDGetWindowDC);
		if(tmp) pGDIGetWindowDC = (GDIGetDC_Type)tmp;
		tmp = HookAPI(module, "user32.dll", ReleaseDC, "ReleaseDC", extDDReleaseDC);
		if(tmp) pGDIReleaseDC = (GDIReleaseDC_Type)tmp;
		tmp = HookAPI(module, "user32.dll", InvalidateRect, "InvalidateRect", extDDInvalidateRect);
		if(tmp) pInvalidateRect = (InvalidateRect_Type)tmp;
	}
	else{
		tmp = HookAPI(module, "user32.dll", GetDC, "GetDC", extGDIGetDC);
		if(tmp) pGDIGetDC = (GDIGetDC_Type)tmp;
		tmp = HookAPI(module, "user32.dll", GetWindowDC, "GetWindowDC", extGDIGetWindowDC);
		if(tmp) pGDIGetWindowDC = (GDIGetDC_Type)tmp;
		tmp = HookAPI(module, "user32.dll", ReleaseDC, "ReleaseDC", extGDIReleaseDC);
		if(tmp) pGDIReleaseDC = (GDIReleaseDC_Type)tmp;
		tmp = HookAPI(module, "user32.dll", InvalidateRect, "InvalidateRect", extInvalidateRect);
		if(tmp) pInvalidateRect = (InvalidateRect_Type)tmp;
	}

	if (dxw.dwFlags1 & CLIENTREMAPPING){
		tmp = HookAPI(module, "user32.dll", ScreenToClient, "ScreenToClient", extScreenToClient);
		if(tmp) pScreenToClient = (ScreenToClient_Type)tmp;
		tmp = HookAPI(module, "user32.dll", ClientToScreen, "ClientToScreen", extClientToScreen);
		if(tmp) pClientToScreen = (ClientToScreen_Type)tmp;
		tmp = HookAPI(module, "user32.dll", GetClientRect, "GetClientRect", extGetClientRect);
		if(tmp) pGetClientRect = (GetClientRect_Type)tmp;
		tmp = HookAPI(module, "user32.dll", GetWindowRect, "GetWindowRect", extGetWindowRect);
		if(tmp) pGetWindowRect = (GetWindowRect_Type)tmp;
		tmp = HookAPI(module, "user32.dll", MapWindowPoints, "MapWindowPoints", extMapWindowPoints);
		if(tmp) pMapWindowPoints = (MapWindowPoints_Type)tmp;
	}

	// get / change display settings
	tmp = HookAPI(module, "user32.dll", ChangeDisplaySettingsA, "ChangeDisplaySettingsA", extChangeDisplaySettings);
	if(tmp) pChangeDisplaySettings = (ChangeDisplaySettings_Type)tmp;
	tmp = HookAPI(module, "user32.dll", ChangeDisplaySettingsExA, "ChangeDisplaySettingsExA", extChangeDisplaySettingsEx);
	if(tmp) pChangeDisplaySettingsEx = (ChangeDisplaySettingsEx_Type)tmp;
	tmp = HookAPI(module, "user32.dll", EnumDisplaySettingsA, "EnumDisplaySettingsA", extEnumDisplaySettings);
	if(tmp) pEnumDisplaySettings = (EnumDisplaySettings_Type)tmp;

	// handle cursor clipping
	tmp = HookAPI(module, "user32.dll", GetClipCursor, "GetClipCursor", extGetClipCursor);
	if(tmp) pGetClipCursor = (GetClipCursor_Type)tmp;

	tmp = HookAPI(module, "user32.dll", ClipCursor, "ClipCursor", extClipCursor);
	if(tmp) pClipCursor = (ClipCursor_Type)tmp;
	tmp = HookAPI(module, "user32.dll", FillRect, "FillRect", extFillRect);
	if(tmp) pFillRect = (FillRect_Type)tmp;
	if (dxw.dwFlags1 & MESSAGEPROC) {
		tmp = HookAPI(module, "user32.dll", PeekMessageA, "PeekMessageA", extPeekMessage); // added for GPL 
		if(tmp) pPeekMessage = (PeekMessage_Type)tmp;
		tmp = HookAPI(module, "user32.dll", GetMessageA, "GetMessageA", extGetMessage); // added for GPL 
		if(tmp) pGetMessage = (GetMessage_Type)tmp;
	}
	tmp = HookAPI(module, "user32.dll", DefWindowProcA, "DefWindowProcA", extDefWindowProc); // added for WarWind background erase 
	if(tmp) pDefWindowProc = (DefWindowProc_Type)tmp;
	tmp = HookAPI(module, "user32.dll", CreateWindowExA, "CreateWindowExA", extCreateWindowExA);
	if(tmp) pCreateWindowExA = (CreateWindowExA_Type)tmp;
	tmp = HookAPI(module, "user32.dll", RegisterClassExA, "RegisterClassExA", extRegisterClassExA);
	if(tmp) pRegisterClassExA = (RegisterClassExA_Type)tmp;
	if (dxw.dwFlags1 & (PREVENTMAXIMIZE|FIXWINFRAME|LOCKWINPOS|LOCKWINSTYLE)){
		tmp = HookAPI(module, "user32.dll", ShowWindow, "ShowWindow", extShowWindow);
		if(tmp) pShowWindow = (ShowWindow_Type)tmp;
		tmp = HookAPI(module, "user32.dll", SetWindowLongA, "SetWindowLongA", extSetWindowLong);
		if(tmp) pSetWindowLong = (SetWindowLong_Type)tmp;
		tmp = HookAPI(module, "user32.dll", GetWindowLongA, "GetWindowLongA", extGetWindowLong);
		if(tmp) pGetWindowLong = (GetWindowLong_Type)tmp;
		tmp = HookAPI(module, "user32.dll", SetWindowPos, "SetWindowPos", extSetWindowPos);
		if(tmp) pSetWindowPos = (SetWindowPos_Type)tmp;
		tmp = HookAPI(module, "user32.dll", DeferWindowPos, "DeferWindowPos", extDeferWindowPos);
		if(tmp) pGDIDeferWindowPos = (DeferWindowPos_Type)tmp;
		tmp = HookAPI(module, "user32.dll", CallWindowProcA, "CallWindowProcA", extCallWindowProc);
		if(tmp) pCallWindowProc = (CallWindowProc_Type)tmp;
	}
	tmp = HookAPI(module, "user32.dll", GetSystemMetrics, "GetSystemMetrics", extGetSystemMetrics);
	if(tmp) pGetSystemMetrics = (GetSystemMetrics_Type)tmp;

	tmp = HookAPI(module, "user32.dll", GetDesktopWindow, "GetDesktopWindow", extGetDesktopWindow);
	if(tmp) pGetDesktopWindow = (GetDesktopWindow_Type)tmp;

	if(dxw.dwFlags1 & MODIFYMOUSE){
		tmp = HookAPI(module, "user32.dll", GetCursorPos, "GetCursorPos", extGetCursorPos);
		if(tmp) pGetCursorPos = (GetCursorPos_Type)tmp;
		//tmp = HookAPI(module, "user32.dll", GetPhysicalCursorPos, "", extGetCursorPos);
		tmp = HookAPI(module, "user32.dll", SetCursor, "SetCursor", extSetCursor);
		if(tmp) pSetCursor = (SetCursor_Type)tmp;
		//tmp = HookAPI(module, "user32.dll", SetPhysicalCursorPos, "", extSetCursorPos);
		tmp = HookAPI(module, "user32.dll", SendMessageA, "SendMessageA", extSendMessage);
		if(tmp) pSendMessage = (SendMessage_Type)tmp;
	}

	if((dxw.dwFlags1 & (MODIFYMOUSE|SLOWDOWN|KEEPCURSORWITHIN)) || (dxw.dwFlags2 & KEEPCURSORFIXED)){ 
		tmp = HookAPI(module, "user32.dll", SetCursorPos, "SetCursorPos", extSetCursorPos);
		if(tmp) pSetCursorPos = (SetCursorPos_Type)tmp;
	}

	tmp = HookAPI(module, "user32.dll", CreateDialogIndirectParamA, "CreateDialogIndirectParamA", extCreateDialogIndirectParam);
	if(tmp) pCreateDialogIndirectParam = (CreateDialogIndirectParam_Type)tmp;
	tmp = HookAPI(module, "user32.dll", CreateDialogParamA, "CreateDialogParamA", extCreateDialogParam);
	if(tmp) pCreateDialogParam = (CreateDialogParam_Type)tmp;
	tmp = HookAPI(module, "user32.dll", MoveWindow, "MoveWindow", extMoveWindow);
	if(tmp) pMoveWindow = (MoveWindow_Type)tmp;

#define TRAPLOWRESOURCES 0
	if(TRAPLOWRESOURCES){
		tmp = HookAPI(module, "kernel32.dll", GetDiskFreeSpaceA, "GetDiskFreeSpaceA", extGetDiskFreeSpaceA);
		if(tmp) pGetDiskFreeSpaceA = (GetDiskFreeSpaceA_Type)tmp;
	}

	if(dxw.dwFlags2 & TIMESTRETCH){
		tmp = HookAPI(module, "kernel32.dll", GetTickCount, "GetTickCount", extGetTickCount);
		if(tmp) pGetTickCount = (GetTickCount_Type)tmp;
		tmp = HookAPI(module, "kernel32.dll", GetLocalTime, "GetLocalTime", extGetLocalTime);
		if(tmp) pGetLocalTime = (GetLocalTime_Type)tmp;
		tmp = HookAPI(module, "kernel32.dll", GetSystemTime, "GetSystemTime", extGetSystemTime);
		if(tmp) pGetSystemTime = (GetSystemTime_Type)tmp;
		tmp = HookAPI(module, "kernel32.dll", Sleep, "Sleep", extSleep);
		if(tmp) pSleep = (Sleep_Type)tmp;
		tmp = HookAPI(module, "kernel32.dll", SleepEx, "SleepEx", extSleepEx);
		if(tmp) pSleepEx = (SleepEx_Type)tmp;
		tmp = HookAPI(module, "user32.dll", SetTimer, "SetTimer", extSetTimer);
		if(tmp) pSetTimer = (SetTimer_Type)tmp;
	}
	return;
}



static void RecoverScreenMode()
{
	DEVMODE InitDevMode;
	BOOL res;
	EnumDisplaySettings(NULL, ENUM_REGISTRY_SETTINGS, &InitDevMode);
	OutTraceD("ChangeDisplaySettings: RECOVER wxh=(%dx%d) BitsPerPel=%d\n", 
		InitDevMode.dmPelsWidth, InitDevMode.dmPelsHeight, InitDevMode.dmBitsPerPel);
	res=(*pChangeDisplaySettings)(&InitDevMode, 0);
	if(res) OutTraceE("ChangeDisplaySettings: ERROR err=%d at %d\n", GetLastError(), __LINE__);
}

// to do: find a logic in the exception codes (0xc0000095 impies a bitmask ?)
// idem for ExceptionFlags
// generalize, find OPCODE length and set number of NOPs accordingly!

LONG WINAPI myUnhandledExceptionFilter(LPEXCEPTION_POINTERS ExceptionInfo)
{
	OutTrace("UnhandledExceptionFilter: exception code=%x at=%x\n",
		ExceptionInfo->ExceptionRecord->ExceptionCode,
		ExceptionInfo->ExceptionRecord->ExceptionAddress);
	DWORD oldprot;
	PVOID target = ExceptionInfo->ExceptionRecord->ExceptionAddress;
	switch(ExceptionInfo->ExceptionRecord->ExceptionCode){
	case 0xc0000094: // ??? 
		if(!VirtualProtect(target, 4, PAGE_READWRITE, &oldprot)) return EXCEPTION_CONTINUE_SEARCH; // error condition
		//if (*(char *)target==0xF7) // ????
		*(USHORT *)target = (USHORT)0x9090; // IDIV reg -> NOP, NOP (Ultim@te Race Pro)
		VirtualProtect(target, 4, oldprot, &oldprot);
		return EXCEPTION_CONTINUE_EXECUTION;
		break;
	case 0xc0000095: // DIV by 0 (divide overflow) exception 
		if(!VirtualProtect(target, 4, PAGE_READWRITE, &oldprot)) return EXCEPTION_CONTINUE_SEARCH; // error condition
		//if (*(char *)target==0xF7) // ????
		*(USHORT *)target = (USHORT)0x9090; // IDIV reg -> NOP, NOP (SonicR)
		VirtualProtect(target, 4, oldprot, &oldprot);
		return EXCEPTION_CONTINUE_EXECUTION;
		break;
	case 0xc0000096: // Priviliged instruction exception
		if(!VirtualProtect(target, 4, PAGE_READWRITE, &oldprot)) return EXCEPTION_CONTINUE_SEARCH; // error condition
		//if (*(char *)target==0xFA) // ????
		*(char *)target = (char)0x90; // CLI -> NOP (Resident Evil)
		VirtualProtect(target, 4, oldprot, &oldprot);
		return EXCEPTION_CONTINUE_EXECUTION;
		break;
	case 0xc000001d: // eXpendable (instruction FEMMS)
		if(!VirtualProtect(target, 4, PAGE_READWRITE, &oldprot)) return EXCEPTION_CONTINUE_SEARCH; // error condition
		*(USHORT *)target = (USHORT)0x9090; // FEMMS -> NOP, NOP (eXpendable)
		VirtualProtect(target, 4, oldprot, &oldprot);
		return EXCEPTION_CONTINUE_EXECUTION;
		break;
	default:
		return EXCEPTION_CONTINUE_SEARCH;
	}
}

void HookExceptionHandler(void)
{
	void *tmp;

	OutTraceD("Set exception handlers\n");
	pSetUnhandledExceptionFilter = SetUnhandledExceptionFilter;
	//v2.1.75 override default exception handler, if any....
	LONG WINAPI myUnhandledExceptionFilter(LPEXCEPTION_POINTERS);
	tmp = HookAPI(NULL, "KERNEL32.dll", UnhandledExceptionFilter, "UnhandledExceptionFilter", myUnhandledExceptionFilter);
	// so far, no need to save the previous handler.
	//if(tmp) pUnhandledExceptionFilter = (UnhandledExceptionFilter_Type)tmp;
	tmp = HookAPI(NULL, "KERNEL32.dll", SetUnhandledExceptionFilter, "SetUnhandledExceptionFilter", extSetUnhandledExceptionFilter);
	//tmp = HookAPI("KERNEL32.dll", SetUnhandledExceptionFilter, "SetUnhandledExceptionFilter", myUnhandledExceptionFilter);
	if(tmp) pSetUnhandledExceptionFilter = (SetUnhandledExceptionFilter_Type)tmp;

	SetErrorMode(SEM_NOGPFAULTERRORBOX|SEM_NOOPENFILEERRORBOX|SEM_FAILCRITICALERRORS);
	(*pSetUnhandledExceptionFilter)((LPTOP_LEVEL_EXCEPTION_FILTER)myUnhandledExceptionFilter);
	//(*pSetUnhandledExceptionFilter)(NULL);
}

void HookModule(char *module, int dxversion)
{
	HookSysLibs(module);
	if(dxw.dwFlags2 & HOOKGDI) HookGDILib(module);
	if(dxw.dwFlags1 & HOOKDI) HookDirectInput(module, dxversion);
	HookDirectDraw(module, dxversion);
	HookDirect3D(module, dxversion);
	HookOle32(module, dxversion); // unfinished business
	if(dxw.dwFlags2 & HOOKOPENGL) HookOpenGLLibs(module, dxw.CustomOpenGLLib); 
}

int HookInit(TARGETMAP *target, HWND hwnd)
{
	BOOL res;
	WINDOWPOS wp;
	char *sModule;
	static char *dxversions[14]={
		"Automatic", "DirectX1~6", "", "", "", "", "", 
		"DirectX7", "DirectX8", "DirectX9", "None\\OpenGL", "", "", ""
	};

	dxw.InitTarget(target);

	// v2.1.75: is it correct to set hWnd here?
	dxw.SethWnd(hwnd);
	dxw.hParentWnd=GetParent(hwnd);
	dxw.hChildWnd=hwnd;

	OutTraceD("HookInit: path=\"%s\" module=\"%s\" dxversion=%s hWnd=%x dxw.hParentWnd=%x\n", 
		target->path, target->module, dxversions[dxw.dwTargetDDVersion], hwnd, dxw.hParentWnd);
	if (IsDebug){
		DWORD dwStyle, dwExStyle;
		dwStyle=GetWindowLong(dxw.GethWnd(), GWL_STYLE);
		dwExStyle=GetWindowLong(dxw.GethWnd(), GWL_EXSTYLE);
		OutTrace("HookInit: hWnd style=%x(%s) exstyle=%x(%s)\n", dwStyle, ExplainStyle(dwStyle), dwExStyle, ExplainExStyle(dwExStyle));
		dwStyle=GetWindowLong(dxw.hParentWnd, GWL_STYLE);
		dwExStyle=GetWindowLong(dxw.hParentWnd, GWL_EXSTYLE);
		OutTrace("HookInit: dxw.hParentWnd style=%x(%s) exstyle=%x(%s)\n", dwStyle, ExplainStyle(dwStyle), dwExStyle, ExplainExStyle(dwExStyle));
	}

	if(dxw.dwFlags1 & HANDLEEXCEPTIONS) HookExceptionHandler();

	if (dxw.dwTFlags & OUTIMPORTTABLE) DumpImportTable(NULL);

	if (dxw.dwTFlags & DXPROXED){
		HookDDProxy(NULL, dxw.dwTargetDDVersion);
		return 0;
	}

	// make InitPosition used for both DInput and DDraw
	InitPosition(target->initx, target->inity,
		target->minx, target->miny, target->maxx, target->maxy);
	dxw.InitWindowPos(target->posx, target->posy, target->sizx, target->sizy);

	HookSysLibsInit(); // this just once...

	HookModule(NULL, dxw.dwTargetDDVersion);
	sModule=strtok(dxw.gsModules," ");
	while (sModule) {
		OutTraceD("HookInit: hooking additional module=%s\n", sModule);
		if (dxw.dwTFlags & OUTIMPORTTABLE) DumpImportTable(sModule);
		HookModule(sModule, dxw.dwTargetDDVersion);
		sModule=strtok(NULL," ");
	}

	if(dxw.dwFlags2 & RECOVERSCREENMODE) RecoverScreenMode();

	InitScreenParameters();

	if (IsDebug) OutTraceD("MoveWindow: target pos=(%d,%d) size=(%d,%d)\n", dxw.iPosX, dxw.iPosX, dxw.iSizX, dxw.iSizY);
	if(dxw.dwFlags1 & FIXPARENTWIN){
		CalculateWindowPos(hwnd, dxw.iSizX, dxw.iSizY, &wp);
		if (IsDebug) OutTraceD("MoveWindow: dxw.hParentWnd=%x pos=(%d,%d) size=(%d,%d)\n", dxw.hParentWnd, wp.x, wp.y, wp.cx, wp.cy);
		res=(*pMoveWindow)(dxw.hParentWnd, wp.x, wp.y, wp.cx, wp.cy, FALSE);
		if(!res) OutTraceE("MoveWindow ERROR: dxw.hParentWnd=%x err=%d at %d\n", dxw.hParentWnd, GetLastError(), __LINE__);
	}

	return 0;
}

HWND WINAPI extGetDesktopWindow(void)
{
	// V2.1.73: correct ???
	HWND res;

	OutTraceD("GetDesktopWindow: FullScreen=%x\n", dxw.IsFullScreen());
	if (dxw.IsFullScreen()){
		OutTraceD("GetDesktopWindow: returning main window hwnd=%x\n", dxw.GethWnd());
		return dxw.GethWnd();
	}
	else{
		res=(*pGetDesktopWindow)();
		OutTraceD("GetDesktopWindow: returning desktop window hwnd=%x\n", res);
		return res;
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
