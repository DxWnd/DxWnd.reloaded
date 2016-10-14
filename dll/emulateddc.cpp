#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include "dxwnd.h"
#include "dxwcore.hpp"
#include "syslibs.h"
#include "dxhelper.h"
#include "resource.h"

// SAVELASTDC: optimized mode, if the last window handle and size didn't change, 
// it is not necessary to create a new VirtualHDC and attached bitmap, you can reuse 
// the last one, with the further advantage of avoiding multiple scaling to and from 
// the virtual image with quality loss (see "BattleZone 1998").
// Unfortunately something is wrong with Imperialism II, so better not activate ....
#ifdef DXWND_SAVELASTDC
static HWND gLasthWnd = NULL;
static RECT gLastRect = {0,0,0,0};
#endif

HDC dxwCore::AcquireEmulatedDC(HWND hwnd)
{
	HDC wdc;
	RECT WinRect;
	HGDIOBJ PrevSelection;
	//OutTrace("ACQUIRE hwnd=%x vhdc=%x pic=%x\n", hwnd, VirtualHDC, VirtualPic);

	if(!(wdc=(*pGDIGetDC)(hwnd))){ // potential DC leakage
		OutTraceE("AcquireEmulatedDC: GetDC ERROR hwnd=%x err=%d at=%d\n", hwnd, GetLastError(), __LINE__);
		return NULL;
	}

	(*pGetClientRect)(hwnd, &WinRect);
	VirtualPicRect = WinRect;
	dxw.UnmapClient(&VirtualPicRect);

#ifdef DXWND_SAVELASTDC
	if((hwnd == gLasthWnd) && (WinRect.right == gLastRect.right) && (WinRect.bottom == gLastRect.bottom)) {
		if(!(*pGDIStretchBlt)(VirtualHDC, VirtualPicRect.left, VirtualPicRect.top, VirtualPicRect.right, VirtualPicRect.bottom, wdc, 0, 0, WinRect.right, WinRect.bottom, SRCCOPY))
			OutTraceE("AcquireEmulatedDC: StretchBlt ERROR err=%d at=%d\n", GetLastError(), __LINE__);
		if(!(pGDIReleaseDC)(hwnd, wdc))
			OutTraceE("AcquireEmulatedDC: ReleaseDC ERROR hdc=%x err=%d at=%d\n", wdc, GetLastError(), __LINE__);
		return VirtualHDC;
	}
	gLasthWnd = hwnd;
	gLastRect = WinRect;
#endif

	//OutTrace("WINDOW picrect=(%d,%d)-(%d,%d)\n", 
	//	VirtualPicRect.left, VirtualPicRect.top, VirtualPicRect.right, VirtualPicRect.bottom);

	OutTraceB("AcquireEmulatedDC: hwnd=%x Desktop=%x WinRect=(%d,%d)(%d,%d) VirtRect=(%d,%d)(%d,%d)\n",
			hwnd, dxw.IsDesktop(hwnd), 
			WinRect.left, WinRect.top, WinRect.right, WinRect.bottom,
			VirtualPicRect.left, VirtualPicRect.top, VirtualPicRect.right, VirtualPicRect.bottom);

	// v2.03.91.fx3: loop required to eliminate resource leakage ("Yu No")
	if(VirtualHDC) {
		while(TRUE){
			if(!DeleteObject(VirtualHDC))
				OutTraceE("AcquireEmulatedDC: DeleteObject ERROR hdc=%x err=%d at=%d\n", VirtualHDC, GetLastError(), __LINE__);
			else break;
			(*pSleep)(1);
		}
	}
	if(VirtualPic) {
		while(TRUE){
			if(!DeleteObject(VirtualPic))
				OutTraceE("AcquireEmulatedDC: DeleteObject ERROR pic=%x err=%d at=%d\n", VirtualPic, GetLastError(), __LINE__);
			else break;
			(*pSleep)(1);
		}
	}

	if(!(VirtualHDC=CreateCompatibleDC(wdc)))
		OutTraceE("AcquireEmulatedDC: CreateCompatibleDC ERROR err=%d at=%d\n", GetLastError(), __LINE__);

	if(!(VirtualPic=CreateCompatibleBitmap(wdc, dxw.GetScreenWidth(), dxw.GetScreenHeight())))
		OutTraceE("AcquireEmulatedDC: CreateCompatibleBitmap ERROR err=%d at=%d\n", GetLastError(), __LINE__);

	if(!(PrevSelection=SelectObject(VirtualHDC, VirtualPic)))
		OutTraceE("AcquireEmulatedDC: SelectObject ERROR err=%d at=%d\n", GetLastError(), __LINE__);
	else {
		if(!DeleteObject(PrevSelection))
			OutTraceE("AcquireEmulatedDC: DeleteObject ERROR pic=%x err=%d at=%d\n", PrevSelection, GetLastError(), __LINE__);;
	}

	//OutTrace("RENEW_IMAGE hdc=%x pic=%x\n", VirtualHDC, VirtualPic);

	if(!(*pGDIStretchBlt)(VirtualHDC, VirtualPicRect.left, VirtualPicRect.top, VirtualPicRect.right, VirtualPicRect.bottom, wdc, 0, 0, WinRect.right, WinRect.bottom, SRCCOPY))
		OutTraceE("AcquireEmulatedDC: StretchBlt ERROR err=%d at=%d\n", GetLastError(), __LINE__);

	if(!(pGDIReleaseDC)(hwnd, wdc))
		OutTraceE("AcquireEmulatedDC: ReleaseDC ERROR hdc=%x err=%d at=%d\n", wdc, GetLastError(), __LINE__);

	return VirtualHDC;
}

BOOL dxwCore::ReleaseEmulatedDC(HWND hwnd)
{
	HDC wdc;
	RECT WinRect;
	//OutTrace("RELEASE hwnd=%x\n", hwnd);

	(*pGetClientRect)(hwnd, &WinRect);

	OutTraceB("ReleaseEmulatedDC: hwnd=%x Desktop=%x WinRect=(%d,%d)(%d,%d) VirtRect=(%d,%d)(%d,%d)\n",
		hwnd, dxw.IsDesktop(hwnd), 
		WinRect.left, WinRect.top, WinRect.right, WinRect.bottom,
		VirtualPicRect.left, VirtualPicRect.top, VirtualPicRect.right, VirtualPicRect.bottom);

	if(!(wdc=(*pGDIGetDC)(hwnd))) { // potential DC leakage
		OutTraceE("ReleaseEmulatedDC: GetDC ERROR err=%d at=%d\n", GetLastError(), __LINE__);
		return FALSE;
	}
	SetStretchBltMode(VirtualHDC, HALFTONE);
	if(!(*pGDIStretchBlt)(wdc, 0, 0, WinRect.right, WinRect.bottom, VirtualHDC, 0, 0, VirtualPicRect.right, VirtualPicRect.bottom, SRCCOPY))
		OutTraceE("ReleaseEmulatedDC: StretchBlt ERROR err=%d at=%d\n", GetLastError(), __LINE__);

	if(!(*pGDIReleaseDC)(hwnd, wdc))// fixed DC leakage
		OutTraceE("ReleaseEmulatedDC: ReleaseDC ERROR hwnd=%x hdc=%x err=%d at=%d\n", hwnd, wdc, GetLastError(), __LINE__);
	
	return TRUE;
}

void dxwCore::ResetEmulatedDC()
{
	//OutTrace("RESET\n");
	VirtualHDC=NULL;
	VirtualPic=NULL;
	VirtualOffsetX=0;
	VirtualOffsetY=0;
}

BOOL dxwCore::IsVirtual(HDC hdc)
{
	return (hdc==VirtualHDC) && (GDIEmulationMode == GDIMODE_EMULATED);
}
