#define _CRT_SECURE_NO_WARNINGS
#define SYSLIBNAMES_DEFINES

#include <stdio.h>
#include "dxwnd.h"
#include "dxwcore.hpp"
#include "syslibs.h"
#include "dxhelper.h"
#include "resource.h"
#include "hddraw.h"
extern GetDC_Type pGetDC;
extern ReleaseDC_Type pReleaseDC;
extern HandleDDThreadLock_Type pReleaseDDThreadLock;

#define OutTraceTMP OutTrace

HDC dxwVDC::GetPrimaryDC()
{
	HDC PrimaryDC;
	LPDIRECTDRAWSURFACE lpDDSPrim;
	PrimaryDC = NULL;
	lpDDSPrim = dxwss.GetPrimarySurface();
	if (lpDDSPrim) {
		if(pReleaseDDThreadLock)(*pReleaseDDThreadLock)();
		(*pGetDC)(lpDDSPrim, &PrimaryDC);
		while((PrimaryDC == NULL) && lpDDSPrim) { 
			OutTraceDW("GetFlippedDC: found primary surface with no DC, unref lpdds=%x\n", lpDDSPrim);
			dxwss.UnrefSurface(lpDDSPrim);
			lpDDSPrim = dxwss.GetPrimarySurface();
			if (lpDDSPrim) (*pGetDC)(lpDDSPrim, &PrimaryDC);
		}
	}
	OutTrace("GetFlippedDC: return primary surface dc=%x\n", PrimaryDC);
	return PrimaryDC;
}

HDC dxwVDC::AcquireEmulatedDC(HWND hwnd, HDC *PrimaryDC)
{
	HDC wdc;
	RECT WinRect;
	HDC RealHDC;

/*---------------------------------
	extern HDC hFlippedDC;
	LPDIRECTDRAWSURFACE lpDDSPrim;
	hFlippedDC = GetPrimaryDC();

----------------------------------*/
	if(!(wdc=(*pGDIGetDC)(hwnd))){
		OutTraceE("GetDC: ERROR err=%d at=%d\n", GetLastError(), __LINE__);
		return NULL;
	}
	RealHDC=wdc;

	RECT LastVRect;
	LastVRect = VirtualPicRect;

	if(!(hwnd=WindowFromDC(wdc)))
		OutTraceE("WindowFromDC: ERROR err=%d at=%d\n", GetLastError(), __LINE__);
	(*pGetClientRect)(hwnd, &WinRect);
	if(dxw.IsDesktop(hwnd)){
		// when screen resolution changes, better renew service resources
		VirtualPicRect = dxw.GetScreenRect();
		if((LastVRect.right != VirtualPicRect.right) || (LastVRect.bottom != VirtualPicRect.bottom)) {
			DeleteObject(VirtualHDC);
			VirtualHDC = NULL;
			DeleteObject(VirtualPic);
			VirtualPic = NULL;
		}
	}
	else {
		VirtualPicRect = WinRect;
		dxw.UnmapClient(&VirtualPicRect);
	}


	OutTraceB("AcquireEmulatedDC: hwnd=%x Desktop=%x WinRect=(%d,%d)(%d,%d) VirtRect=(%d,%d)(%d,%d)\n",
		hwnd, dxw.IsDesktop(hwnd), 
		WinRect.left, WinRect.top, WinRect.right, WinRect.bottom,
		VirtualPicRect.left, VirtualPicRect.top, VirtualPicRect.right, VirtualPicRect.bottom);


	if (!VirtualHDC){
		OutTraceB("AcquireEmulatedDC: INITIALIZE\n");
		if(!(VirtualHDC=CreateCompatibleDC(wdc)))
			OutTraceE("CreateCompatibleDC: ERROR err=%d at=%d\n", GetLastError(), __LINE__);

		if(!(VirtualPic=CreateCompatibleBitmap(wdc, dxw.GetScreenWidth(), dxw.GetScreenHeight())))
			OutTraceE("CreateCompatibleBitmap: ERROR err=%d at=%d\n", GetLastError(), __LINE__);

		if(!SelectObject(VirtualHDC, VirtualPic))
			OutTraceE("SelectObject: ERROR err=%d at=%d\n", GetLastError(), __LINE__);

/*---------------------------------*/
	//HRGN hRgn;
	//RECT screen;
	//screen = dxw.GetScreenRect();
	//hRgn = CreateRectRgnIndirect(&screen);
	//SelectClipRgn (VirtualHDC, hRgn);
	//screen = dxw.GetScreenRect();
	//hRgn = CreateRectRgnIndirect(&screen);
	//SelectClipRgn (wdc, hRgn);
/*---------------------------------*/
/*---------------------------------*/
	if (!(hwnd == dxw.GethWnd())) {
		POINT father, child, offset;
		father.x = father.y = 0;
		child.x = child.y = 0;
		(*pClientToScreen)(dxw.GethWnd(),&father);
		(*pClientToScreen)(hwnd,&child);
		offset.x = child.x - father.x;
		offset.y = child.y - father.y;
		dxw.UnmapClient(&offset);
		OutTraceDW("AcquireEmulatedDC: child window hwnd=%x offset=(%d,%d)\n", hwnd, offset.x, offset.y);		
		(*pSetViewportOrgEx)(VirtualHDC, offset.x, offset.y, NULL);
	}
	else
		(*pSetViewportOrgEx)(VirtualHDC, 0, 0, NULL);


		if(*PrimaryDC = GetPrimaryDC()){ // better copy from virtual primary ....
			OutTraceTMP("AcquireEmulatedDC: intialize from primary dc=%x\n", PrimaryDC);
			if(!(*pGDIBitBlt)(VirtualHDC, 0, 0, VirtualPicRect.right, VirtualPicRect.bottom, *PrimaryDC, 0, 0, SRCCOPY))
				OutTraceE("StretchBlt: ERROR err=%d at=%d\n", GetLastError(), __LINE__);
		}
		else { // otherwise stretch from current windows DC
			OutTraceTMP("AcquireEmulatedDC: intialize from windows dc=%x\n", wdc);
			if(!(*pGDIStretchBlt)(VirtualHDC, 0, 0, VirtualPicRect.right, VirtualPicRect.bottom, wdc, 0, 0, WinRect.right, WinRect.bottom, SRCCOPY))
				OutTraceE("StretchBlt: ERROR err=%d at=%d\n", GetLastError(), __LINE__);
		}
	}

	return VirtualHDC;
}

BOOL dxwVDC::ReleaseEmulatedDC(HWND hwnd)
{
	HDC wdc;
	RECT WinRect;
	HDC PrimaryDC;

	(*pGetClientRect)(hwnd, &WinRect);

	OutTraceB("ReleaseEmulatedDC: hwnd=%x Desktop=%x WinRect=(%d,%d)(%d,%d) VirtRect=(%d,%d)(%d,%d)\n",
		hwnd, dxw.IsDesktop(hwnd), 
		WinRect.left, WinRect.top, WinRect.right, WinRect.bottom,
		VirtualPicRect.left, VirtualPicRect.top, VirtualPicRect.right, VirtualPicRect.bottom);

	if(PrimaryDC = GetPrimaryDC()){ // better copy from virtual primary ....
		OutTraceTMP("AcquireEmulatedDC: flush to from primary dc=%x\n", PrimaryDC);
		if(!(*pGDIBitBlt)(PrimaryDC, VirtualPicRect.left, VirtualPicRect.top, VirtualPicRect.right, VirtualPicRect.bottom, VirtualHDC, 0, 0, SRCCOPY))
			OutTraceE("StretchBlt: ERROR err=%d at=%d\n", GetLastError(), __LINE__);
	}
	else
	{
		if(!(wdc=(*pGDIGetDC)(hwnd)))
			OutTraceE("GetDC: ERROR err=%d at=%d\n", GetLastError(), __LINE__);
		OutTraceTMP("AcquireEmulatedDC: flush to windows dc=%x\n", wdc);
		SetStretchBltMode(wdc, HALFTONE);
		if(!(*pGDIStretchBlt)(wdc, 0, 0, WinRect.right, WinRect.bottom, VirtualHDC, 0, 0, VirtualPicRect.right, VirtualPicRect.bottom, SRCCOPY))
			OutTraceE("StretchBlt: ERROR err=%d at=%d\n", GetLastError(), __LINE__);
	}
	(*pGDIReleaseDC)(hwnd, VirtualHDC);

	return TRUE;
}

void dxwVDC::ResetEmulatedDC()
{
	VirtualHDC=NULL;
	VirtualPic=NULL;
	VirtualOffsetX=0;
	VirtualOffsetY=0;
}

BOOL dxwVDC::IsVirtual(HDC hdc)
{
	return (hdc==VirtualHDC) /* && (dwFlags3 & GDIEMULATEDC)*/;
}

#if 0
BOOL dxwCore::ReleaseSharedDC(HWND hwnd, HDC hDC)
{
	HRESULT ret;
	LPDIRECTDRAWSURFACE lpDDSPrim;
	lpDDSPrim = dxwss.GetPrimarySurface();
	if(!lpDDSPrim) return(TRUE);
	OutTraceDW("GDI.ReleaseDC: releasing flipped GDI hdc=%x\n", hDC);
	ret=(*pReleaseDC)(dxwss.GetPrimarySurface(), hDC);
	if (!(hwnd == dxw.GethWnd())) {
		POINT father, child, offset;
		RECT rect;
		HDC hdc;
		father.x = father.y = 0;
		child.x = child.y = 0;
		(*pClientToScreen)(dxw.GethWnd(),&father);
		(*pClientToScreen)(hwnd,&child);
		offset.x = child.x - father.x;
		offset.y = child.y - father.y;
		if(offset.x || offset.y){
			// if the graphis was blitted to primary but below a modal child window,
			// bring that up to the window surface to make it visible.
			BOOL ret2;
			(*pGetClientRect)(hwnd, &rect);
			hdc=(*pGDIGetDC)(hwnd);
			if(!hdc) OutTrace("GDI.ReleaseDC: GetDC ERROR=%d at %d\n", GetLastError(), __LINE__);
			ret2=(*pGDIBitBlt)(hdc, rect.left, rect.top, rect.right, rect.bottom, hDC, offset.x, offset.y, SRCCOPY);
			if(!ret2) OutTrace("GDI.ReleaseDC: BitBlt ERROR=%d at %d\n", GetLastError(), __LINE__);
			ret2=(*pGDIReleaseDC)(hwnd, hdc);
			if(!ret2)OutTrace("GDI.ReleaseDC: ReleaseDC ERROR=%d at %d\n", GetLastError(), __LINE__);
			// this may flicker ....
			(*pInvalidateRect)(hwnd, NULL, FALSE); 
		}
	}
	if (ret) OutTraceE("GDI.ReleaseDC ERROR: err=%x(%s) at %d\n", ret, ExplainDDError(ret), __LINE__);
	else dxw.ScreenRefresh();
	return (ret == DD_OK);
}
#endif