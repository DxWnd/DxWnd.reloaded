#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <d3d9.h>
#include "dxwnd.h"
#include "dxwcore.hpp"
#include "syslibs.h"
#include "dxhelper.h"
#include "resource.h"
#include "hddraw.h"
#include "shareddc.hpp"

//#define D3D9TRY
#define SHAREDDCDEBUG FALSE
#if SHAREDDCDEBUG
#define _Warn(s) {char cap[80]; sprintf(cap, "Warn at %d", __LINE__); MessageBox(NULL, (s), cap, MB_OK);}
#else
#define _Warn(s) 
#endif

extern HandleDDThreadLock_Type pReleaseDDThreadLock;
extern GetDC_Type pGetDCMethod();
extern ReleaseDC_Type pReleaseDCMethod();
//void setClippingRegion(HDC, HDC, HWND, bool, const POINT&);

/*---------------------------------------------------------------------------------+
|                                                                                  |
| Constructor, Desctructor                                                         |
|                                                                                  |
+---------------------------------------------------------------------------------*/

dxwSDC::dxwSDC()
{
	OutTraceB("dxwSDC::dxwSDC: Initialize\n");
	PrimaryDC = NULL;
	lpDDSPrimary = NULL;
	LastScreenWidth = LastScreenHeight = 0;
	LastHDC = NULL;
	CurrenthWnd = NULL;
}
 
dxwSDC::~dxwSDC()
{
	OutTraceB("dxwSDC::~dxwSDC: Destroy\n");
}

/*---------------------------------------------------------------------------------+
|                                                                                  |
| GetPrimaryDC: builds a suitable DC to write to, according to the input DC        |
|                                                                                  |
+---------------------------------------------------------------------------------*/

static IDirect3DSurface9 *pDestSurface = NULL;

HDC dxwSDC::GetPrimaryDC(HDC hdc)
{
	HRESULT res;
	extern HandleDDThreadLock_Type pReleaseDDThreadLock;
	extern void *lpD3DActiveDevice;

	OutTraceB("dxwSDC::GetPrimaryDC: hdc=%x\n", hdc);

	// look for ddraw first
	//if(pReleaseDDThreadLock)(*pReleaseDDThreadLock)();
	lpDDSPrimary = dxwss.GetPrimarySurface();
	if (lpDDSPrimary) {
		if(pReleaseDDThreadLock)(*pReleaseDDThreadLock)();
		res=((*pGetDCMethod())(lpDDSPrimary, &PrimaryDC));
		while((PrimaryDC == NULL) && lpDDSPrimary) { 
			OutTraceB("dxwSDC::GetPrimaryDC: found primary surface with no DC, unref lpdds=%x\n", lpDDSPrimary);
			dxwss.UnrefSurface(lpDDSPrimary);
			lpDDSPrimary = dxwss.GetPrimarySurface();
			if (lpDDSPrimary) (*pGetDCMethod())(lpDDSPrimary, &PrimaryDC);
		}
		if (!PrimaryDC) {
			_Warn("No primary DC");
			OutTraceB("dxwSDC::GetPrimaryDC: no ddraw primary DC\n");
			return NULL;
		}
		// avoid double Getdc on same hdc and lock
		// if(PrimaryDC == hdc) (*pReleaseDC)(lpDDSPrimary, PrimaryDC);
		OutTraceB("dxwSDC::GetPrimaryDC: ddraw PrimaryDC=%x\n", PrimaryDC);
		VirtualSurfaceType = VIRTUAL_ON_DDRAW;
	}
	else {
		// finally, search GDI DC
		PrimaryDC = (*pGDIGetDC)(dxw.GethWnd());
		if (!PrimaryDC) {
			_Warn("No window DC");
			OutTraceB("dxwSDC::GetPrimaryDC: no windows DC\n");
			return NULL;
		}
		OutTraceB("dxwSDC::GetPrimaryDC: gdi PrimaryDC=%x\n", PrimaryDC);
		VirtualSurfaceType = VIRTUAL_ON_WINDOW;
	}	

	// whenever the hdc changes, rebuild the virtual DC
	if(hdc != LastHDC) do {
		LastHDC = hdc;
		RECT client;
		if(VirtualHDC){
			//(*pGDIReleaseDC)(dxw.GethWnd(), VirtualHDC);
			DeleteObject(VirtualHDC);
		}

		if(!(VirtualHDC=(*pGDICreateCompatibleDC)(PrimaryDC))){
			OutTraceE("CreateCompatibleDC: ERROR err=%d at=%d\n", GetLastError(), __LINE__);
			_Warn("CreateCompatibleDC ERROR");
			break;
		}

		if(!(CurrenthWnd = WindowFromDC(hdc))){
			OutTraceE("dxwSDC::GetPrimaryDC: WindowFromDC ERROR err=%d at=%d\n", GetLastError(), __LINE__);
			_Warn("WindowFromDC ERROR");
			break;
		}

		if(!(*pGetClientRect)(CurrenthWnd, &client)){
			OutTraceE("dxwSDC::GetPrimaryDC: GetClietError ERROR err=%d at=%d\n", GetLastError(), __LINE__);
			_Warn("GetClietError ERROR");
			break;
		}

		dxw.UnmapClient(&client);
		ScreenWidth = client.right;
		ScreenHeight = client.bottom;

		OutTraceB("dxwSDC::GetPrimaryDC: VirtualHDC INITIALIZE size=(%dx%d)\n", LastScreenWidth, LastScreenHeight);

		if(!(VirtualPic=(*pCreateCompatibleBitmap)(PrimaryDC, ScreenWidth, ScreenHeight))){
			OutTraceE("dxwSDC::GetPrimaryDC: CreateCompatibleBitmap ERROR err=%d at=%d\n", GetLastError(), __LINE__);
			_Warn("CreateCompatibleBitmap ERROR");
		}

		if(!SelectObject(VirtualHDC, VirtualPic)){
			OutTraceE("dxwSDC::GetPrimaryDC: SelectObject ERROR err=%d at=%d\n", GetLastError(), __LINE__);
			_Warn("SelectObject ERROR");
		}

		DeleteObject(VirtualPic);
		VirtualPic = 0;
	} while(0);

	if(CurrenthWnd && CurrenthWnd!=dxw.GethWnd()){
		POINT zero1 = {0, 0};
		POINT zero2 = {0, 0};
		(*pClientToScreen)(CurrenthWnd, &zero1);
		(*pClientToScreen)(dxw.GethWnd(), &zero2);
		WinOffset.x = zero1.x - zero2.x;
		WinOffset.y = zero1.y - zero2.y;
		VirtualOffset = WinOffset;
		dxw.UnmapClient(&VirtualOffset);
		OutTraceB("dxwSDC::GetPrimaryDC: WinOffset=(%d,%d)->(%d,%d)\n", WinOffset.x, WinOffset.y, VirtualOffset.x, VirtualOffset.y);
	}
	else {
		WinOffset.x = 0;
		WinOffset.y = 0;
		VirtualOffset.x = 0;
		VirtualOffset.y = 0;
		OutTraceB("dxwSDC::GetPrimaryDC: same window\n");
	}

	if(PrimaryDC){
		switch(VirtualSurfaceType){
			case VIRTUAL_ON_DDRAW:
				if(!(*pGDIBitBlt)(VirtualHDC, 0, 0, ScreenWidth, ScreenHeight, PrimaryDC, VirtualOffset.x, VirtualOffset.y, SRCCOPY)){
					OutTraceE("dxwSDC::GetPrimaryDC: BitBlt ERROR err=%d at=%d\n", GetLastError(), __LINE__);
					_Warn("BitBlt ERROR");
				}
				OutTraceB("dxwSDC::GetPrimaryDC: fill=(0,0)-(%dx%d) from=(%d,%d)\n", ScreenWidth, ScreenHeight, VirtualOffset.x, VirtualOffset.y);
				break;
			case VIRTUAL_ON_WINDOW:
				int w, h;
				dxw.MapClient(&VirtualOffset);
				w = ScreenWidth;
				h = ScreenHeight;
				dxw.MapClient(&w, &h);
				if(!(*pGDIStretchBlt)(VirtualHDC, 0, 0, ScreenWidth, ScreenHeight, PrimaryDC, VirtualOffset.x, VirtualOffset.y, w, h, SRCCOPY)){
					OutTraceE("dxwSDC::GetPrimaryDC: StretchBlt ERROR err=%d at=%d\n", GetLastError(), __LINE__);
					_Warn("StretchBlt ERROR");
				}
				OutTraceB("dxwSDC::GetPrimaryDC: fill=(0,0)-(%dx%d) from=(%d,%d)-(%dx%d)\n", ScreenWidth, ScreenHeight, VirtualOffset.x, VirtualOffset.y, w, h);
				break;
		}
	}

	POINT origin = {};
	POINT mainwin = {};
	GetDCOrgEx(hdc, &origin);
	GetDCOrgEx((*pGDIGetDC)(dxw.GethWnd()), &mainwin);
	origin.x -= mainwin.x;
	origin.y -= mainwin.y;
	OutTraceB("dxwSDC::GetPrimaryDC: origin=(%d,%d)\n", origin.x, origin.y);

	copyDcAttributes(VirtualHDC, hdc, origin); 
	setClippingRegion(VirtualHDC, hdc, dxw.GethWnd(), FALSE, origin); // no good

	return VirtualHDC;
}

/*---------------------------------------------------------------------------------+
|                                                                                  |
| GetHdc: returns the DC to write for the GDI call                                 |
|                                                                                  |
+---------------------------------------------------------------------------------*/

HDC	dxwSDC::GetHdc(void)
{
	return VirtualHDC;
}

/*---------------------------------------------------------------------------------+
|                                                                                  |
| PutPrimaryDC: transfers the DC content to the primary surface and the screen     |
|                                                                                  |
+---------------------------------------------------------------------------------*/

BOOL dxwSDC::PutPrimaryDC(HDC hdc, BOOL UpdateScreen, int XDest, int YDest, int nDestWidth, int nDestHeight)
{
	extern Unlock1_Type pUnlock1;
	BOOL ret;
	HRESULT res;

	ret = TRUE;
	if (nDestWidth == 0) nDestWidth=ScreenWidth-XDest;
	if (nDestHeight == 0) nDestHeight=ScreenHeight-YDest;

	if (IsDebug){
		char sRect[81];
		if(UpdateScreen) sprintf(sRect, "pos=(%d,%d) size=(%dx%d) winoffset=(%d,%d) virtoffset=(%d,%d)", 
			XDest, YDest, nDestWidth, nDestHeight, WinOffset.x, WinOffset.y, VirtualOffset.x, VirtualOffset.y);
		else strcpy(sRect, "");
		char *sType;
		switch(VirtualSurfaceType){
			case VIRTUAL_ON_D3D: sType="D3D"; break;
			case VIRTUAL_ON_DDRAW: sType="DDRAW"; break;
			case VIRTUAL_ON_WINDOW: sType="WINDOW"; break;
			default: sType="???"; break;
		}
		OutTrace("dxwSDC::PutPrimaryDC: hdc=%x type=%s update=%x %s\n", hdc, sType, UpdateScreen, sRect);
	}

	if(UpdateScreen){
		switch(VirtualSurfaceType){
			case VIRTUAL_ON_DDRAW: 
				ret=(*pGDIBitBlt)(PrimaryDC, XDest+VirtualOffset.x, YDest+VirtualOffset.y, nDestWidth, nDestHeight, VirtualHDC, XDest, YDest, SRCCOPY);
				if(!ret || (ret==GDI_ERROR)) {
					OutTraceE("dxwSDC::PutPrimaryDC: BitBlt ERROR ret=%x err=%d\n", ret, GetLastError()); 
				}
				res=(*pReleaseDCMethod())(lpDDSPrimary, PrimaryDC);
				if(res){
					OutTraceE("dxwSDC::PutPrimaryDC: ReleaseDC ERROR res=%x\n", res); 
				}
				dxw.ScreenRefresh();
				break;
			case VIRTUAL_ON_WINDOW:
				SetStretchBltMode(PrimaryDC, HALFTONE);
				RECT RealArea, VirtualArea;
				// some fullscreen games ("Imperialism II") blitted from negative coordinates -2,-2 !!
				if(XDest < 0) {
					nDestWidth += XDest;
					XDest=0;
				}
				if(YDest < 0) {
					nDestHeight += YDest;
					YDest=0;
				}
				VirtualArea.left = XDest;
				VirtualArea.top = YDest;
				VirtualArea.right = nDestWidth;
				VirtualArea.bottom = nDestHeight;
				RealArea = VirtualArea;
				dxw.MapClient(&RealArea);
				OffsetRect(&RealArea, WinOffset.x, WinOffset.y);
				ret=TRUE;
				if(PrimaryDC)ret=(*pGDIStretchBlt)(PrimaryDC, RealArea.left, RealArea.top, RealArea.right, RealArea.bottom, VirtualHDC, VirtualArea.left, VirtualArea.top, VirtualArea.right, VirtualArea.bottom, SRCCOPY);
				ret=(*pGDIReleaseDC)(dxw.GethWnd(), PrimaryDC);
				break;
		}
	}
	else {
		switch(VirtualSurfaceType){
			case VIRTUAL_ON_DDRAW: 
				res=(*pReleaseDCMethod())(lpDDSPrimary, PrimaryDC);
				if(res){
					OutTraceE("dxwSDC::PutPrimaryDC: ReleaseDC ERROR res=%x\n", res); 
				}
				break;
			case VIRTUAL_ON_WINDOW:
				ret=(*pGDIReleaseDC)(dxw.GethWnd(), PrimaryDC);
				if(!ret){
					OutTraceE("dxwSDC::PutPrimaryDC: ReleaseDC ERROR err=%d\n", GetLastError()); 
				}
				break;
		}
	}

	OutTraceB("dxwSDC::PutPrimaryDC: hdc=%x PrimaryDC=%x ret=%x\n", hdc, PrimaryDC, ret);
	return ret;
}

BOOL dxwSDC::PutPrimaryDC(HDC hdc, BOOL UpdateScreen)
{
	return PutPrimaryDC(hdc, UpdateScreen, 0, 0, LastScreenWidth, LastScreenHeight);
}

/*---------------------------------------------------------------------------------+
|                                                                                  |
| Service routines                                                                 |
|                                                                                  |
+---------------------------------------------------------------------------------*/

void dxwSDC::copyDcAttributes(HDC destDC, HDC origDc, POINT origin)
{
	origFont = SelectObject(destDC, GetCurrentObject(origDc, OBJ_FONT));
	origBrush = SelectObject(destDC, GetCurrentObject(origDc, OBJ_BRUSH));
	origPen = SelectObject(destDC, GetCurrentObject(origDc, OBJ_PEN));

	if (GM_ADVANCED == GetGraphicsMode(origDc)){
		SetGraphicsMode(destDC, GM_ADVANCED);
		XFORM transform = {};
		GetWorldTransform(origDc, &transform);
		SetWorldTransform(destDC, &transform);
	}

	SetMapMode(destDC, GetMapMode(origDc));

	POINT viewportOrg = {};
	GetViewportOrgEx(origDc, &viewportOrg);
	SetViewportOrgEx(destDC, viewportOrg.x + origin.x, viewportOrg.y + origin.y, NULL);
	SIZE viewportExt = {};
	GetViewportExtEx(origDc, &viewportExt);
	SetViewportExtEx(destDC, viewportExt.cx, viewportExt.cy, NULL);

	POINT windowOrg = {};
	GetWindowOrgEx(origDc, &windowOrg);
	SetWindowOrgEx(destDC, windowOrg.x, windowOrg.y, NULL);
	SIZE windowExt = {};
	GetWindowExtEx(origDc, &windowExt);
	SetWindowExtEx(destDC, windowExt.cx, windowExt.cy, NULL);

	SetArcDirection(destDC, GetArcDirection(origDc));
	SetBkColor(destDC, GetBkColor(origDc));
	SetBkMode(destDC, GetBkMode(origDc));
	SetDCBrushColor(destDC, GetDCBrushColor(origDc));
	SetDCPenColor(destDC, GetDCPenColor(origDc));
	SetLayout(destDC, GetLayout(origDc));
	SetPolyFillMode(destDC, GetPolyFillMode(origDc));
	SetROP2(destDC, GetROP2(origDc));
	SetStretchBltMode(destDC, GetStretchBltMode(origDc));
	SetTextAlign(destDC, GetTextAlign(origDc));
	SetTextCharacterExtra(destDC, GetTextCharacterExtra(origDc));
	SetTextColor(destDC, GetTextColor(origDc));

	OutTraceB("copyDcAttributes: orig=(%d,%d)\n", origin.x, origin.y);
	if(!(*pSetWindowOrgEx)(destDC, -origin.x, -origin.y, NULL))
		OutTraceE("copyDcAttributes: SetWindowOrgEx ERROR orig=(%d,%d) err=%d\n", origin.x, origin.y, GetLastError());

	POINT brushOrg = {};
	GetBrushOrgEx(origDc, &brushOrg);
	SetBrushOrgEx(destDC, brushOrg.x, brushOrg.y, NULL);

	POINT currentPos = {};
	(*pGetCurrentPositionEx)(origDc, &currentPos);
	dxw.MapClient(&currentPos);
	(*pMoveToEx)(destDC, currentPos.x, currentPos.y, NULL);
}

typedef struct 
{
	HDC compatDc;
	HWND rootWnd;
} ExcludeClipRectsData_Type;

static BOOL CALLBACK excludeClipRectForOverlappingWindow(HWND hwnd, LPARAM lParam)
{
	ExcludeClipRectsData_Type *excludeClipRectsData = (ExcludeClipRectsData_Type *)lParam;
	if (hwnd == excludeClipRectsData->rootWnd) return FALSE;
	if (!IsWindowVisible(hwnd)) return TRUE;

	RECT windowRect = {};
	GetWindowRect(hwnd, &windowRect);

	HRGN windowRgn = (*pCreateRectRgnIndirect)(&windowRect);
	ExtSelectClipRgn(excludeClipRectsData->compatDc, windowRgn, RGN_DIFF);
	DeleteObject(windowRgn);
	OutTraceB("dxwSDC::excludeClipRect: hwnd=%x rect=(%d,%d)-(%d,%d)\n", hwnd, windowRect.left, windowRect.top, windowRect.right, windowRect.bottom);

	return TRUE;
}

static void excludeClipRectsForOverlappingWindows(HWND hwnd, BOOL isMenuWindow, HDC compatDc)
{
	ExcludeClipRectsData_Type excludeClipRectsData = { compatDc, GetAncestor(hwnd, GA_ROOT) };
	if (!isMenuWindow){
		EnumWindows(&excludeClipRectForOverlappingWindow, (LPARAM)(&excludeClipRectsData));
	}

	HWND menuWindow = FindWindow((LPCSTR)(0x8000), NULL);
	while (menuWindow && menuWindow != hwnd){
		excludeClipRectForOverlappingWindow(menuWindow, (LPARAM)(&excludeClipRectsData));
		menuWindow = FindWindowEx(NULL, menuWindow, (LPCSTR)(0x8000), NULL);
	}
}

void dxwSDC::setClippingRegion(HDC compatDc, HDC origDc, HWND hwnd, bool isMenuWindow, const POINT& origin)
{
	HRGN clipRgn = (*pCreateRectRgn)(0, 0, 0, 0);
	if (1 == GetClipRgn(origDc, clipRgn)){
		OffsetRgn(clipRgn, origin.x, origin.y);
		SelectClipRgn(compatDc, clipRgn);
	}

	if ((hwnd) && (dxw.dwFlags7 & FIXCLIPPERAREA)){
		if (isMenuWindow || 1 != GetRandomRgn(origDc, clipRgn, SYSRGN)){
			RECT rect = {};
			(*pGetWindowRect)(hwnd, &rect);
			(*pSetRectRgn)(clipRgn, rect.left, rect.top, rect.right, rect.bottom);
		}

		excludeClipRectsForOverlappingWindows(hwnd, isMenuWindow, compatDc);
		ExtSelectClipRgn(compatDc, clipRgn, RGN_AND);
	}

	DeleteObject(clipRgn);
	SetMetaRgn(compatDc);
}

