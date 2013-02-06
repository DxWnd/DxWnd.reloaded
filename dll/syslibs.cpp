#define _WIN32_WINNT 0x0600
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <malloc.h>
#include <ddraw.h>
#include "dxwnd.h"
#include "dxwcore.hpp"
#include "dxhook.h"
#include "glhook.h"
#include "msvfwhook.h"
#include "syslibs.h"
#include "dxhelper.h"
#include "hddraw.h"
#include "hddproxy.h"

#define WINDOWDC 0xFFFFFFFF

extern DWORD PaletteEntries[256];
extern LPDIRECTDRAW lpDD;
extern Unlock4_Type pUnlockMethod(LPDIRECTDRAWSURFACE);

extern GetDC_Type pGetDC;
extern ReleaseDC_Type pReleaseDC;

DEVMODE SetDevMode;
DEVMODE *pSetDevMode=NULL;

extern HRESULT WINAPI extBlt(LPDIRECTDRAWSURFACE, LPRECT, LPDIRECTDRAWSURFACE, LPRECT, DWORD, LPDDBLTFX);
extern HRESULT WINAPI sBlt(char *, LPDIRECTDRAWSURFACE, LPRECT, LPDIRECTDRAWSURFACE, LPRECT, DWORD, LPDDBLTFX, BOOL);

extern DirectDrawEnumerate_Type pDirectDrawEnumerate;
extern DirectDrawEnumerateEx_Type pDirectDrawEnumerateEx;

extern LRESULT CALLBACK extChildWindowProc(HWND, UINT, WPARAM, LPARAM);
extern INT_PTR CALLBACK extDialogWindowProc(HWND, UINT, WPARAM, LPARAM);

/* ------------------------------------------------------------------ */

static POINT FixMessagePt(HWND hwnd, POINT point)
{
	RECT rect;
	static POINT curr;
	curr=point;

	if(!(*pScreenToClient)(hwnd, &curr)){
		OutTraceE("ScreenToClient ERROR=%d hwnd=%x at %d\n", GetLastError(), hwnd, __LINE__);
		curr.x = curr.y = 0;
	}

	if (!(*pGetClientRect)(hwnd, &rect)) {
		OutTraceE("GetClientRect ERROR=%d hwnd=%x at %d\n", GetLastError(), hwnd, __LINE__);
		curr.x = curr.y = 0;
	}

#ifdef ISDEBUG
	if(IsDebug) OutTrace("FixMessagePt point=(%d,%d) hwnd=%x win pos=(%d,%d) size=(%d,%d)\n",
		point.x, point.y, hwnd, point.x-curr.x, point.y-curr.y, rect.right, rect.bottom);
#endif

	if (curr.x < 0) curr.x=0;
	if (curr.y < 0) curr.y=0;
	if (curr.x > rect.right) curr.x=rect.right;
	if (curr.y > rect.bottom) curr.y=rect.bottom;
	if (rect.right)  curr.x = (curr.x * dxw.GetScreenWidth()) / rect.right;
	if (rect.bottom) curr.y = (curr.y * dxw.GetScreenHeight()) / rect.bottom;

	return curr;
}

/* ------------------------------------------------------------------ */

static COLORREF GetMatchingColor(COLORREF crColor)
{
	int iDistance, iMinDistance;
	int iColorIndex, iMinColorIndex;
	COLORREF PalColor;

	iMinDistance=0xFFFFFF;
	iMinColorIndex=0;

	for(iColorIndex=0; iColorIndex<256; iColorIndex++){
		int iDist;
		iDistance=0;

		PalColor=PaletteEntries[iColorIndex];
		switch(dxw.ActualPixelFormat.dwRGBBitCount){
		case 32:
			PalColor = ((PalColor & 0x00FF0000) >> 16) | (PalColor & 0x0000FF00) | ((PalColor & 0x000000FF) << 16);
			break;
		case 16:
			if(dxw.ActualPixelFormat.dwGBitMask==0x03E0){
				// RGB555 screen settings
				PalColor = ((PalColor & 0x7C00) >> 7) | ((PalColor & 0x03E0) << 6) | ((PalColor & 0x001F) << 19);
			}
			else {
				// RGB565 screen settings
				PalColor = ((PalColor & 0xF800) >> 8) | ((PalColor & 0x07E0) << 5) | ((PalColor & 0x001F) << 19);
			}
			break;
		}

		iDist = (crColor & 0x00FF0000) - (PalColor & 0x00FF0000);
		iDist >>= 16;
		if (iDist<0) iDist=-iDist;
		iDist *= iDist;
		iDistance += iDist;

		iDist = (crColor & 0x0000FF00) - (PalColor & 0x0000FF00);
		iDist >>= 8;
		if (iDist<0) iDist=-iDist;
		iDist *= iDist;
		iDistance += iDist;

		iDist = (crColor & 0x000000FF) - (PalColor & 0x000000FF);
		// iDist >>= 0;
		if (iDist<0) iDist=-iDist;
		iDist *= iDist;
		iDistance += iDist;

		if (iDistance < iMinDistance) {
			iMinDistance = iDistance;
			iMinColorIndex = iColorIndex;
		}

		if (iMinDistance==0) break; // got the perfect match!
	}
	OutTraceD("GetMatchingColor: color=%x matched with palette[%d]=%x dist=%d\n", 
		crColor, iMinColorIndex, PaletteEntries[iMinColorIndex], iDistance);
	PalColor=PaletteEntries[iMinColorIndex];
	switch(dxw.ActualPixelFormat.dwRGBBitCount){
	case 32:
		crColor = ((PalColor & 0x00FF0000) >> 16) | (PalColor & 0x0000FF00) | ((PalColor & 0x000000FF) << 16); 
		break;
	case 16:
		if(dxw.ActualPixelFormat.dwGBitMask==0x03E0){
			// RGB555 screen settings
			crColor = ((PalColor & 0x7C00) >> 7) | ((PalColor & 0x03E0) << 6) | ((PalColor & 0x001F) << 19);
		}
		else {
			// RGB565 screen settings
			crColor = ((PalColor & 0xF800) >> 8) | ((PalColor & 0x07E0) << 5) | ((PalColor & 0x001F) << 19);
		}
		break;
	}
	return crColor;
}

extern void FixWindowFrame(HWND);

// GHO: pro Diablo
HWND WINAPI extCreateWindowExA(
  DWORD dwExStyle,
  LPCTSTR lpClassName,
  LPCTSTR lpWindowName,
  DWORD dwStyle,
  int x,
  int y,
  int nWidth,
  int nHeight,
  HWND hWndParent,
  HMENU hMenu,
  HINSTANCE hInstance,
  LPVOID lpParam) 
{
	HWND wndh;
	WNDPROC pWindowProc;
	BOOL isValidHandle=TRUE;

	OutTraceD("CreateWindowEx: class=\"%s\" wname=\"%s\" pos=(%d,%d) size=(%d,%d) Style=%x(%s) ExStyle=%x(%s)\n",
		lpClassName, lpWindowName, x, y, nWidth, nHeight, 
		dwStyle, ExplainStyle(dwStyle), dwExStyle, ExplainExStyle(dwExStyle));
	if(IsDebug) OutTrace("CreateWindowEx: DEBUG screen=(%d,%d)\n", dxw.GetScreenWidth(), dxw.GetScreenHeight());

	// no maximized windows in any case
	if (dxw.dwFlags1 & PREVENTMAXIMIZE){
		OutTraceD("CreateWindowEx: handling PREVENTMAXIMIZE mode\n");
		dwStyle &= ~(WS_MAXIMIZE | WS_POPUP);
		dwExStyle &= ~WS_EX_TOPMOST;
	}

	// v2.1.92: fixes size & position for auxiliary big window, often used
	// for intro movies etc. : needed for ......
	// evidently, this was supposed to be a fullscreen window....
	// v2.1.100: fixes for "The Grinch": this game creates a new main window for OpenGL
	// rendering using CW_USEDEFAULT placement and 800x600 size while the previous
	// main win was 640x480 only!
	// v2.02.13: if it's a WS_CHILD window, don't reposition the x,y, placement for BIG win.
	if	(
			(
				((x==0)&&(y==0)) || ((x==CW_USEDEFAULT)&&(y==CW_USEDEFAULT))
			)
		&&
			(((DWORD)nWidth>=dxw.GetScreenWidth())&&((DWORD)nHeight>=dxw.GetScreenHeight()))
		&&
			!(dwExStyle & WS_EX_CONTROLPARENT) // Diablo fix
		&&
			!(dwStyle & WS_CHILD) // Diablo fix
		){
		RECT screen;
		POINT upleft = {0,0};
		// update virtual screen size if it has grown 
		dxw.SetScreenSize(nWidth, nHeight);
		// inserted some checks here, since the main window could be destroyed
		// or minimized (see "Jedi Outcast") so that you may get a dangerous 
		// zero size. In this case, better renew the hWnd assignement and its coordinates.
		do { // fake loop
			isValidHandle = FALSE;
			if (!(*pGetClientRect)(dxw.GethWnd(),&screen)) break;
			if (!(*pClientToScreen)(dxw.GethWnd(),&upleft)) break;
			if (screen.right==0 || screen.bottom==0) break;
			isValidHandle = TRUE;
		} while(FALSE);
		if (isValidHandle){
			if (!(dwStyle & WS_CHILD)){ 
				x=upleft.x;
				y=upleft.y;
			}
			nWidth=screen.right;
			nHeight=screen.bottom;
			OutTraceD("CreateWindowEx: fixed BIG win pos=(%d,%d) size=(%d,%d)\n", x, y, nWidth, nHeight);
		}
		else {
			// invalid parent coordinates: use initial placement, but leave the size.
			// should also fix the window style and compensate for borders here?
			if (!(dwStyle & WS_CHILD)){ 
				x=dxw.iPosX;
				y=dxw.iPosY;
			}
			nWidth=dxw.iSizX;
			nHeight=dxw.iSizY;
			OutTraceD("CreateWindowEx: renewed BIG win pos=(%d,%d) size=(%d,%d)\n", x, y, nWidth, nHeight);
		}
		dxw.SetFullScreen(TRUE);
	}

	if(!dxw.IsFullScreen()){ // v2.1.63: needed for "Monster Truck Madness"
		wndh= (*pCreateWindowExA)(dwExStyle, lpClassName, lpWindowName, dwStyle, x, y, nWidth, nHeight, 
			hWndParent, hMenu, hInstance, lpParam);
		OutTraceD("CreateWindowEx: windowed mode ret=%x\n", wndh);
		return wndh;
	}

	// tested on Gangsters: coordinates must be window-relative!!!
	// Age of Empires....
	if (dwStyle & WS_CHILD){ 
		dxw.MapClient(&x, &y, &nWidth, &nHeight);
		OutTraceD("CreateWindowEx: fixed WS_CHILD pos=(%d,%d) size=(%d,%d)\n",
			x, y, nWidth, nHeight);
	}
	// needed for Diablo, that creates a new control parent window that must be
	// overlapped to the directdraw surface.
	else if (dwExStyle & WS_EX_CONTROLPARENT){
		dxw.MapWindow(&x, &y, &nWidth, &nHeight);
		OutTraceD("CreateWindowEx: fixed WS_EX_CONTROLPARENT pos=(%d,%d) size=(%d,%d)\n",
			x, y, nWidth, nHeight);
	}

	OutTraceB("CreateWindowEx: fixed pos=(%d,%d) size=(%d,%d) Style=%x(%s) ExStyle=%x(%s)\n",
		x, y, nWidth, nHeight, dwStyle, ExplainStyle(dwStyle), dwExStyle, ExplainExStyle(dwExStyle));

	wndh= (*pCreateWindowExA)(dwExStyle, lpClassName, lpWindowName, dwStyle, x, y, nWidth, nHeight, 
		hWndParent, hMenu, hInstance, lpParam);
	if (wndh==(HWND)NULL){
		OutTraceE("CreateWindowEx: ERROR err=%d Style=%x(%s) ExStyle=%x\n",
			GetLastError(), dwStyle, ExplainStyle(dwStyle), dwExStyle);
		return wndh;
	}

	if ((!isValidHandle) && dxw.IsFullScreen()) {
		dxw.SethWnd(wndh);
		extern void AdjustWindowPos(HWND, DWORD, DWORD);
		(*pSetWindowLong)(wndh, GWL_STYLE, (dxw.dwFlags2 & MODALSTYLE) ? 0 : WS_OVERLAPPEDWINDOW);
		(*pSetWindowLong)(wndh, GWL_EXSTYLE, 0); 
		OutTraceD("CreateWindow: hwnd=%x, set style=WS_OVERLAPPEDWINDOW extstyle=0\n", wndh); 
		AdjustWindowPos(wndh, nWidth, nHeight);
		(*pShowWindow)(wndh, SW_SHOWNORMAL);
	}

	if ((dxw.dwFlags1 & FIXWINFRAME) && !(dwStyle & WS_CHILD))
		FixWindowFrame(wndh);

	// to do: handle inner child, and leave dialogue & modal child alone!!!
	if (dwStyle & WS_CHILD){
		long res;
		pWindowProc = (WNDPROC)(*pGetWindowLong)(wndh, GWL_WNDPROC);
		OutTraceD("Hooking CHILD wndh=%x WindowProc %x->%x\n", wndh, pWindowProc, extChildWindowProc);
		res=(*pSetWindowLong)(wndh, GWL_WNDPROC, (LONG)extChildWindowProc);
		WhndStackPush(wndh, pWindowProc);
		if(!res) OutTraceE("CreateWindowExA: SetWindowLong ERROR %x\n", GetLastError());
	}

	OutTraceD("CreateWindowEx: ret=%x\n", wndh);
	return wndh;
}

COLORREF WINAPI extSetTextColor(HDC hdc, COLORREF crColor)
{
	COLORREF res;

	if ((dxw.dwFlags1 & EMULATESURFACE) && (dxw.dwFlags1 & HANDLEDC) && (dxw.VirtualPixelFormat.dwRGBBitCount==8))
		crColor=GetMatchingColor(crColor);

	res=(*pGDISetTextColor)(hdc, crColor);
	OutTraceD("SetTextColor: color=%x res=%x%s\n", crColor, res, (res==CLR_INVALID)?"(CLR_INVALID)":"");
	return res;
}

COLORREF WINAPI extSetBkColor(HDC hdc, COLORREF crColor)
{
	COLORREF res;

	if ((dxw.dwFlags1 & EMULATESURFACE) && (dxw.dwFlags1 & HANDLEDC) && (dxw.VirtualPixelFormat.dwRGBBitCount==8))
		crColor=GetMatchingColor(crColor);

	res=(*pGDISetBkColor)(hdc, crColor);
	OutTraceD("SetBkColor: color=%x res=%x%s\n", crColor, res, (res==CLR_INVALID)?"(CLR_INVALID)":"");
	return res;
}

LPRECT lpClipRegion=NULL;
RECT ClipRegion;

BOOL WINAPI extClipCursor(RECT *lpRectArg)
{
	// reference: hooking and setting ClipCursor is mandatori in "Emergency: Fighters for Life"
	// where the application expects the cursor to be moved just in a inner rect within the 
	// main window surface.

	BOOL res;
	RECT *lpRect;
	RECT Rect;

	if(IsTraceC){
		if (lpRectArg)
			OutTrace("ClipCursor: rect=(%d,%d)-(%d,%d)\n", 
				lpRectArg->left,lpRectArg->top,lpRectArg->right,lpRectArg->bottom);
		else 
			OutTrace("ClipCursor: rect=(NULL)\n");
	}

 	if (!(dxw.dwFlags1 & ENABLECLIPPING)) return 1;

	if(lpRectArg){
		Rect=*lpRectArg;
		lpRect=&Rect;
	}
	else
		lpRect=NULL;

	if(dxw.dwFlags1 & MODIFYMOUSE){
		// save desired clip region
		if (lpRect) {
			ClipRegion=*lpRectArg;
			lpClipRegion=&ClipRegion;
		}
		else
			lpClipRegion=NULL;

		*lpRect=dxw.MapWindowRect(lpRect);
	}

	if (pClipCursor) res=(*pClipCursor)(lpRect);
	OutTraceD("ClipCursor: rect=(%d,%d)-(%d,%d) res=%x\n", 
		lpRect->left,lpRect->top,lpRect->right,lpRect->bottom, res);

	return TRUE;
}

BOOL WINAPI extGetClipCursor(LPRECT lpRect)
{
	// v2.1.93: if ENABLECLIPPING, return the saved clip rect coordinates

	BOOL ret;

	// proxy....
	if (!(dxw.dwFlags1 & ENABLECLIPPING)) {
		ret=(*pGetClipCursor)(lpRect);
		if(IsTraceD){
			if (lpRect)
				OutTrace("ClipCursor: PROXED rect=(%d,%d)-(%d,%d) ret=%d\n", 
					lpRect->left,lpRect->top,lpRect->right,lpRect->bottom, ret);
			else 
				OutTrace("ClipCursor: PROXED rect=(NULL) ret=%d\n", ret);
		}		
		return ret;
	}

	if(lpRect){
		if(lpClipRegion)
			*lpRect=ClipRegion;
		else{
			lpRect->top = lpRect->left = 0;
			lpRect->right = dxw.GetScreenWidth();
			lpRect->bottom = dxw.GetScreenHeight();
		}
		OutTraceD("ClipCursor: rect=(%d,%d)-(%d,%d) ret=%d\n", 
			lpRect->left,lpRect->top,lpRect->right,lpRect->bottom, TRUE);
	}

	return TRUE;
}

int LastCurPosX, LastCurPosY;

BOOL WINAPI extGetCursorPos(LPPOINT lppoint)
{
	HRESULT res;
	static int PrevX, PrevY;
	POINT prev;

	if(dxw.dwFlags1 & SLOWDOWN) dxw.DoSlow(2);

	if (pGetCursorPos) {
		res=(*pGetCursorPos)(lppoint);
	}
	else {
		lppoint->x =0; lppoint->y=0;
		res=1;
	}

	prev=*lppoint;
	*lppoint=dxw.ScreenToClient(*lppoint);
	*lppoint=dxw.FixCursorPos(*lppoint);
	GetHookInfo()->CursorX=(short)lppoint->x;
	GetHookInfo()->CursorY=(short)lppoint->y;
	OutTraceC("GetCursorPos: FIXED pos=(%d,%d)->(%d,%d)\n", prev.x, prev.y, lppoint->x, lppoint->y);

	return res;
}

BOOL WINAPI extSetCursorPos(int x, int y)
{
	BOOL res;
	int PrevX, PrevY;

	PrevX=x;
	PrevY=y;

	if(dxw.dwFlags2 & KEEPCURSORFIXED) {
		OutTraceC("SetCursorPos: FIXED pos=(%d,%d)\n", x, y);
		LastCurPosX=x;
		LastCurPosY=y;
		return 1;
	}

	if(dxw.dwFlags1 & SLOWDOWN) dxw.DoSlow(2);

	if(dxw.dwFlags1 & KEEPCURSORWITHIN){
		// Intercept SetCursorPos outside screen boundaries (used as Cursor OFF in some games)
		if ((y<0)||(y>=(int)dxw.GetScreenHeight())||(x<0)||(x>=(int)dxw.GetScreenWidth())) return 1;
	}

	if(dxw.dwFlags1 & MODIFYMOUSE){
		POINT cur;
		RECT rect;

		// find window metrics
		if (!(*pGetClientRect)(dxw.GethWnd(), &rect)) {
			// report error and ignore ...
			OutTraceE("GetClientRect(%x) ERROR %d at %d\n", dxw.GethWnd(), GetLastError(), __LINE__);
			return 0;
		}

		x= x * rect.right / dxw.GetScreenWidth();
		y= y * rect.bottom / dxw.GetScreenHeight();

		// check for boundaries (???)
		if (x >= rect.right) x=rect.right-1;
		if (x<0) x=0;
		if (y >= rect.bottom) y=rect.bottom-1;
		if (y<0) y=0;

		// make it screen absolute
		cur.x = x;
		cur.y = y;
		if (!(*pClientToScreen)(dxw.GethWnd(), &cur)) {
			OutTraceE("ClientToScreen(%x) ERROR %d at %d\n", dxw.GethWnd(), GetLastError(), __LINE__);
			return 0;
		}
		x = cur.x;
		y = cur.y;
	}

	res=0;
	if (pSetCursorPos) res=(*pSetCursorPos)(x,y);

	OutTraceC("SetCursorPos: res=%x XY=(%d,%d)->(%d,%d)\n",res, PrevX, PrevY, x, y);
	return res;
}

BOOL WINAPI extTextOutA(HDC hdc, int nXStart, int nYStart, LPCTSTR lpString, int cchString)
{
	BOOL res;
	OutTraceD("TextOut: hdc=%x xy=(%d,%d) str=(%d)\"%s\"\n", hdc, nXStart, nYStart, cchString, lpString);
	if (dxw.dwFlags1 & FIXTEXTOUT) {
		POINT anchor;
		anchor.x=nXStart;
		anchor.y=nYStart;
		(*pClientToScreen)(dxw.GethWnd(), &anchor);
		nXStart=anchor.x;
		nYStart=anchor.y;
	}
	res=(*pGDITextOutA)(hdc, nXStart, nYStart, lpString, cchString);
	return res;
}

BOOL WINAPI extRectangle(HDC hdc, int nLeftRect, int nTopRect, int nRightRect, int nBottomRect)
{
	OutTraceD("Rectangle: hdc=%x xy=(%d,%d)-(%d,%d)\n", hdc, nLeftRect, nTopRect, nRightRect, nBottomRect);
	if (dxw.dwFlags1 & FIXTEXTOUT) {
		POINT anchor;
		anchor.x=nLeftRect;
		anchor.y=nTopRect;
		(*pClientToScreen)(dxw.GethWnd(), &anchor);
		nLeftRect=anchor.x;
		nTopRect=anchor.y;
		anchor.x=nRightRect;
		anchor.y=nBottomRect;
		(*pClientToScreen)(dxw.GethWnd(), &anchor);
		nRightRect=anchor.x;
		nBottomRect=anchor.y;
	}
	return (*pGDIRectangle)(hdc, nLeftRect, nTopRect, nRightRect, nBottomRect);
}

int WINAPI extFillRect(HDC hdc, const RECT *lprc, HBRUSH hbr)
{
	RECT rc, trim;
	HWND hWnd;
	OutTraceD("FillRect: hdc=%x xy=(%d,%d)-(%d,%d)\n", hdc, lprc->left, lprc->top, lprc->right, lprc->bottom);
	memcpy(&rc, lprc, sizeof(rc));
	hWnd = WindowFromDC(hdc);
	if((hWnd == dxw.GethWnd()) ||
		(hWnd == 0) ||
		(hWnd == GetDesktopWindow())){
		// trim: some games (Player Manager 98) clear the screen by filling an exagerated rect
		(*pGetClientRect)(dxw.GethWnd(), &trim);
		hdc=GetDC(dxw.GethWnd());
		dxw.MapWindowRect(&rc);
		if(rc.left < trim.left) rc.left = trim.left;
		if(rc.top < trim.top) rc.top = trim.top;
		if(rc.right > trim.right) rc.right = trim.right;
		if(rc.bottom > trim.bottom) rc.bottom = trim.bottom;
		OutTraceD("FillRect: hwnd=%x hdc=%x fixed xy=(%d,%d)-(%d,%d)\n", hWnd, hdc, rc.left, rc.top, rc.right, rc.bottom);
	}
	if (dxw.dwFlags1 & FIXTEXTOUT) {
		// to be verified: why shifting and not scaling?
		POINT anchor;
		anchor.x=rc.left;
		anchor.y=rc.top;
		(*pClientToScreen)(dxw.GethWnd(), &anchor);
		rc.left=anchor.x;
		rc.top=anchor.y;
		anchor.x=rc.right;
		anchor.y=rc.bottom;
		(*pClientToScreen)(dxw.GethWnd(), &anchor);
		rc.right=anchor.x;
		rc.bottom=anchor.y;
	}
	return (*pFillRect)(hdc, &rc, hbr);
}

HFONT WINAPI extCreateFont(int nHeight, int nWidth, int nEscapement, int nOrientation, int fnWeight,
				 DWORD fdwItalic, DWORD fdwUnderline, DWORD fdwStrikeOut, DWORD fdwCharSet,
				 DWORD fdwOutputPrecision, DWORD fdwClipPrecision, DWORD fdwQuality,
				 DWORD fdwPitchAndFamily, LPCTSTR lpszFace)
{
	OutTraceD("CreateFont: h=%d w=%d face=\"%s\"\n", nHeight, nWidth, lpszFace);
	return (*pGDICreateFont)(nHeight, nWidth, nEscapement, nOrientation, fnWeight,
				 fdwItalic, fdwUnderline, fdwStrikeOut, fdwCharSet,
				 fdwOutputPrecision, fdwClipPrecision, NONANTIALIASED_QUALITY,
				 fdwPitchAndFamily, lpszFace);
}

// CreateFontIndirect hook routine to avoid font aliasing that prevents reverse blitting working on palettized surfaces

HFONT WINAPI extCreateFontIndirect(const LOGFONT* lplf)
{
	LOGFONT lf;
	HFONT retHFont;
	OutTraceD("CreateFontIndirect: h=%d w=%d face=\"%s\"\n", lplf->lfHeight, lplf->lfWidth, lplf->lfFaceName);
	memcpy((char *)&lf, (char *)lplf, sizeof(LOGFONT));
	lf.lfQuality=NONANTIALIASED_QUALITY;
	retHFont=((*pGDICreateFontIndirect)(&lf));
	if(retHFont)
		OutTraceD("CreateFontIndirect: hfont=%x\n", retHFont);
	else
		OutTraceD("CreateFontIndirect: error=%d at %d\n", GetLastError(), __LINE__);
	return retHFont;
}

BOOL WINAPI extShowWindow(HWND hwnd, int nCmdShow)
{
	BOOL res;

	OutTraceD("ShowWindow: hwnd=%x, CmdShow=%x(%s)\n", hwnd, nCmdShow, ExplainShowCmd(nCmdShow));
	if (dxw.dwFlags1 & PREVENTMAXIMIZE){
		if(nCmdShow==SW_MAXIMIZE){
			OutTraceD("ShowWindow: suppress maximize\n");
			nCmdShow=SW_SHOWNORMAL;
		}
	}

	res=pShowWindow(hwnd, nCmdShow);

	return res;
}

LONG WINAPI extGetWindowLong(HWND hwnd, int nIndex)
{
	LONG res;

	res=(*pGetWindowLong)(hwnd, nIndex);

	OutTraceD("GetWindowLong: hwnd=%x, Index=%x(%s) res=%x\n", hwnd, nIndex, ExplainSetWindowIndex(nIndex), res);

	if(nIndex==GWL_WNDPROC){
		WNDPROC wp;
		wp=WhndGetWindowProc(hwnd);
		OutTraceD("GetWindowLong: remapping WindowProc res=%x -> %x\n", res, (LONG)wp);
		if(wp) res=(LONG)wp; // if not found, don't alter the value.
	}

	return res;
}

LONG WINAPI extSetWindowLong(HWND hwnd, int nIndex, LONG dwNewLong)
{
	LONG res;

	OutTraceD("SetWindowLong: hwnd=%x, Index=%x(%s) Val=%x\n", 
		hwnd, nIndex, ExplainSetWindowIndex(nIndex), dwNewLong);

	//if(!hwnd) hwnd=dxw.GethWnd();

	if (dxw.dwFlags1 & LOCKWINSTYLE){
		if(nIndex==GWL_STYLE){
			OutTraceD("SetWindowLong: Lock GWL_STYLE=%x\n", dwNewLong);
			//return 1;
			return (*pGetWindowLong)(hwnd, nIndex);
		}
		if(nIndex==GWL_EXSTYLE){
			OutTraceD("SetWindowLong: Lock GWL_EXSTYLE=%x\n", dwNewLong);
			//return 1;
			return (*pGetWindowLong)(hwnd, nIndex);
		}
	}

	if (dxw.dwFlags1 & PREVENTMAXIMIZE){
		if(nIndex==GWL_STYLE){
			OutTraceD("SetWindowLong: GWL_STYLE %x suppress MAXIMIZE\n", dwNewLong);
			dwNewLong |= WS_OVERLAPPEDWINDOW; 
			dwNewLong &= ~(WS_DLGFRAME|WS_MAXIMIZE|WS_POPUP|WS_VSCROLL|WS_HSCROLL|WS_CLIPSIBLINGS); 
		}
		if(nIndex==GWL_EXSTYLE){
			OutTraceD("SetWindowLong: GWL_EXSTYLE %x suppress TOPMOST\n", dwNewLong);
			dwNewLong = dwNewLong & ~(WS_EX_TOPMOST); 
		}
	}

	if (dxw.dwFlags1 & FIXWINFRAME){
		if((nIndex==GWL_STYLE) && !(dwNewLong & WS_CHILD)){
			OutTraceD("SetWindowLong: GWL_STYLE %x force OVERLAPPEDWINDOW\n", dwNewLong);
			dwNewLong |= WS_OVERLAPPEDWINDOW; 
			dwNewLong &= ~WS_CLIPSIBLINGS; 
		}
	}

	if (nIndex==GWL_WNDPROC){
		long lres;
		// GPL fix
		if(hwnd==0) {
			hwnd=dxw.GethWnd();
			OutTrace("SetWindowLong: NULL hwnd, FIXING hwnd=%x\n",hwnd);
		}
		// end of GPL fix
		res=(LONG)WhndGetWindowProc(hwnd);		
		WhndStackPush(hwnd, (WNDPROC)dwNewLong);
		SetLastError(0);
		lres=(*pSetWindowLong)(hwnd, GWL_WNDPROC, (LONG)extWindowProc);
		if(!lres && GetLastError())OutTraceE("SetWindowLong: ERROR err=%d at %d\n", GetLastError(), __LINE__);
	}
	else {
		res=(*pSetWindowLong)(hwnd, nIndex, dwNewLong);
	}

	OutTraceD("SetWindowLong: hwnd=%x, nIndex=%x, Val=%x, res=%x\n", hwnd, nIndex, dwNewLong, res);
	return res;
}

BOOL WINAPI extSetWindowPos(HWND hwnd, HWND hWndInsertAfter, int X, int Y, int cx, int cy, UINT uFlags)
{
	BOOL res;

	OutTraceD("SetWindowPos: hwnd=%x%s pos=(%d,%d) dim=(%d,%d) Flags=%x\n", 
		hwnd, dxw.IsFullScreen()?"(FULLSCREEN)":"", X, Y, cx, cy, uFlags);

	if ((hwnd != dxw.GethWnd()) || !dxw.IsFullScreen()){
		// just proxy
		res=(*pSetWindowPos)(hwnd, hWndInsertAfter, X, Y, cx, cy, uFlags);
		if(!res)OutTraceE("SetWindowPos: ERROR err=%d at %d\n", GetLastError(), __LINE__);
		return res;
	}

	if ((dxw.dwFlags1 & LOCKWINPOS) && dxw.IsFullScreen()){
		// Note: any attempt to change the window position, no matter where and how, through the
		// SetWindowPos API is causing resizing to the default 1:1 pixed size in Commandos. 
		// in such cases, there is incompatibility between LOCKWINPOS and LOCKWINSTYLE.
		return 1;
	}

	if (dxw.dwFlags1 & PREVENTMAXIMIZE){
		int UpdFlag =0;
		int MaxX, MaxY;
		MaxX = dxw.iSizX;
		MaxY = dxw.iSizY;
		if (!MaxX) MaxX = dxw.GetScreenWidth();
		if (!MaxY) MaxY = dxw.GetScreenHeight();
		if(cx>MaxX) { cx=MaxX; UpdFlag=1; }
		if(cy>MaxY) { cy=MaxY; UpdFlag=1; }
		if (UpdFlag) 
			OutTraceD("SetWindowPos: using max dim=(%d,%d)\n", cx, cy);
	}

	// useful??? to be demonstrated....
	// when altering main window in fullscreen mode, fix the coordinates for borders
	DWORD dwCurStyle;
	RECT rect;
	rect.top=rect.left=0;
	rect.right=cx; rect.bottom=cy;
	dwCurStyle=(*pGetWindowLong)(hwnd, GWL_STYLE);
	AdjustWindowRect(&rect, dwCurStyle, FALSE);
	cx=rect.right; cy=rect.bottom;
	OutTraceD("SetWindowPos: main form hwnd=%x fixed size=(%d,%d)\n", hwnd, cx, cy);

	res=(*pSetWindowPos)(hwnd, hWndInsertAfter, X, Y, cx, cy, uFlags);
	if(!res)OutTraceE("SetWindowPos: ERROR err=%d at %d\n", GetLastError(), __LINE__);
	return res;
}

HDWP WINAPI extDeferWindowPos(HDWP hWinPosInfo, HWND hwnd, HWND hWndInsertAfter, int X, int Y, int cx, int cy, UINT uFlags)
{
	HDWP res;

	OutTraceD("DeferWindowPos: hwnd=%x%s pos=(%d,%d) dim=(%d,%d) Flags=%x\n", 
		hwnd, dxw.IsFullScreen()?"(FULLSCREEN)":"", X, Y, cx, cy, uFlags);

	if ((hwnd != dxw.GethWnd()) || !dxw.IsFullScreen()){
		// just proxy
		res=(*pGDIDeferWindowPos)(hWinPosInfo, hwnd, hWndInsertAfter, X, Y, cx, cy, uFlags);
		if(!res)OutTraceE("SetWindowPos: ERROR err=%d at %d\n", GetLastError(), __LINE__);
		return res;
	}

	if (dxw.dwFlags1 & LOCKWINPOS){
		return hWinPosInfo;
	}

	if (dxw.dwFlags1 & PREVENTMAXIMIZE){
		int UpdFlag =0;
		int MaxX, MaxY;
		MaxX = dxw.iSizX;
		MaxY = dxw.iSizY;
		if (!MaxX) MaxX = dxw.GetScreenWidth();
		if (!MaxY) MaxY = dxw.GetScreenHeight();
		if(cx>MaxX) { cx=MaxX; UpdFlag=1; }
		if(cy>MaxY) { cy=MaxY; UpdFlag=1; }
		if (UpdFlag) 
			OutTraceD("SetWindowPos: using max dim=(%d,%d)\n", cx, cy);
	}

	// useful??? to be demonstrated....
	// when altering main window in fullscreen mode, fix the coordinates for borders
	DWORD dwCurStyle;
	RECT rect;
	rect.top=rect.left=0;
	rect.right=cx; rect.bottom=cy;
	dwCurStyle=(*pGetWindowLong)(hwnd, GWL_STYLE);
	AdjustWindowRect(&rect, dwCurStyle, FALSE);
	cx=rect.right; cy=rect.bottom;
	OutTraceD("SetWindowPos: main form hwnd=%x fixed size=(%d,%d)\n", hwnd, cx, cy);

	res=(*pGDIDeferWindowPos)(hWinPosInfo, hwnd, hWndInsertAfter, X, Y, cx, cy, uFlags);
	if(!res)OutTraceE("DeferWindowPos: ERROR err=%d at %d\n", GetLastError(), __LINE__);
	return res;
}

void dxwFixWindowPos(char *ApiName, HWND hwnd, LPARAM lParam)
{
	LPWINDOWPOS wp;
	int MaxX, MaxY;
	wp = (LPWINDOWPOS)lParam;
	MaxX = dxw.iSizX;
	MaxY = dxw.iSizY;
	if (!MaxX) MaxX = dxw.GetScreenWidth();
	if (!MaxY) MaxY = dxw.GetScreenHeight();
	static int iLastCX, iLastCY;
	static int BorderX=-1;
	static int BorderY=-1;
	int cx, cy;

	OutTraceD("%s: GOT hwnd=%x pos=(%d,%d) dim=(%d,%d) Flags=%x(%s)\n", 
		ApiName, hwnd, wp->x, wp->y, wp->cx, wp->cy, wp->flags, ExplainWPFlags(wp->flags));

	if ((wp->flags & (SWP_NOMOVE|SWP_NOSIZE))==(SWP_NOMOVE|SWP_NOSIZE)) return; //v2.02.13
	//if (wp->flags & (SWP_NOMOVE|SWP_NOSIZE)) return; //v2.02.10

	if ((dxw.dwFlags1 & LOCKWINPOS) && dxw.IsFullScreen() && (hwnd==dxw.GethWnd())){ 
		extern void CalculateWindowPos(HWND, DWORD, DWORD, LPWINDOWPOS);
		CalculateWindowPos(hwnd, MaxX, MaxY, wp);
		OutTraceD("%s: LOCK pos=(%d,%d) dim=(%d,%d)\n", ApiName, wp->x, wp->y, wp->cx, wp->cy);
	}

	if ((dxw.dwFlags2 & KEEPASPECTRATIO) && dxw.IsFullScreen() && (hwnd==dxw.GethWnd())){ 
		// note: while keeping aspect ration, resizing from one corner doesn't tell
		// which coordinate is prevalent to the other. We made an arbitrary choice.
		// note: v2.1.93: compensation must refer to the client area, not the wp
		// window dimensions that include the window borders.
		if(BorderX==-1){
			RECT client, full;
			(*pGetClientRect)(hwnd, &client);
			(*pGetWindowRect)(hwnd, &full);
			BorderX= full.right - full.left - client.right;
			BorderY= full.bottom - full.top - client.bottom;
			OutTraceD("%s: KEEPASPECTRATIO window borders=(%d,%d)\n", ApiName, BorderX, BorderY);
		}
		extern LRESULT LastCursorPos;
		switch (LastCursorPos){
			case HTBOTTOM:
			case HTTOP:
			case HTBOTTOMLEFT:
			case HTBOTTOMRIGHT:
			case HTTOPLEFT:
			case HTTOPRIGHT:
				cx = BorderX + ((wp->cy - BorderY) * dxw.GetScreenWidth()) / dxw.GetScreenHeight();
				if(cx!=wp->cx){
					OutTraceD("%s: KEEPASPECTRATIO adjusted cx=%d->%d\n", ApiName, wp->cx, cx);
					wp->cx = cx;
				}
				break;
			case HTLEFT:
			case HTRIGHT:
				cy = BorderY + ((wp->cx - BorderX) * dxw.GetScreenHeight()) / dxw.GetScreenWidth();
				if(cy!=wp->cy){
					OutTraceD("%s: KEEPASPECTRATIO adjusted cy=%d->%d\n", ApiName, wp->cy, cy);
					wp->cy = cy;
				}
				break;
		}
	}

	if (dxw.dwFlags1 & PREVENTMAXIMIZE){
		int UpdFlag = 0;

		if(wp->cx>MaxX) { wp->cx=MaxX; UpdFlag=1; }
		if(wp->cy>MaxY) { wp->cy=MaxY; UpdFlag=1; }
		if (UpdFlag) 
			OutTraceD("%s: SET max dim=(%d,%d)\n", ApiName, wp->cx, wp->cy);
	}

	iLastCX= wp->cx;
	iLastCY= wp->cy;
}

void dxwFixMinMaxInfo(char *ApiName, HWND hwnd, LPARAM lParam)
{
	if (dxw.dwFlags1 & PREVENTMAXIMIZE){
		LPMINMAXINFO lpmmi;
		lpmmi=(LPMINMAXINFO)lParam;
		OutTraceD("%s: GOT MaxPosition=(%d,%d) MaxSize=(%d,%d)\n", ApiName, 
			lpmmi->ptMaxPosition.x, lpmmi->ptMaxPosition.y, lpmmi->ptMaxSize.x, lpmmi->ptMaxSize.y);
		lpmmi->ptMaxPosition.x=0;
		lpmmi->ptMaxPosition.y=0;
		if(pSetDevMode){
			lpmmi->ptMaxSize.x = pSetDevMode->dmPelsWidth;
			lpmmi->ptMaxSize.y = pSetDevMode->dmPelsHeight;
		}
		else{
			lpmmi->ptMaxSize.x = dxw.GetScreenWidth();
			lpmmi->ptMaxSize.y = dxw.GetScreenHeight();
		}
		OutTraceD("%s: SET PREVENTMAXIMIZE MaxPosition=(%d,%d) MaxSize=(%d,%d)\n", ApiName, 
			lpmmi->ptMaxPosition.x, lpmmi->ptMaxPosition.y, lpmmi->ptMaxSize.x, lpmmi->ptMaxSize.y);
	}
	// v2.1.75: added logic to fix win coordinates to selected ones. 
	// fixes the problem with "Achtung Spitfire", that can't be managed through PREVENTMAXIMIZE flag.
	if (dxw.dwFlags1 & LOCKWINPOS){
		LPMINMAXINFO lpmmi;
		lpmmi=(LPMINMAXINFO)lParam;
		OutTraceD("%s: GOT MaxPosition=(%d,%d) MaxSize=(%d,%d)\n", ApiName, 
			lpmmi->ptMaxPosition.x, lpmmi->ptMaxPosition.y, lpmmi->ptMaxSize.x, lpmmi->ptMaxSize.y);
		lpmmi->ptMaxPosition.x=dxw.iPosX;
		lpmmi->ptMaxPosition.y=dxw.iPosY;
		lpmmi->ptMaxSize.x = dxw.iSizX ? dxw.iSizX : dxw.GetScreenWidth();
		lpmmi->ptMaxSize.y = dxw.iSizY ? dxw.iSizY : dxw.GetScreenHeight();
		OutTraceD("%s: SET LOCKWINPOS MaxPosition=(%d,%d) MaxSize=(%d,%d)\n", ApiName, 
			lpmmi->ptMaxPosition.x, lpmmi->ptMaxPosition.y, lpmmi->ptMaxSize.x, lpmmi->ptMaxSize.y);
	}
}

void dxwFixStyle(char *ApiName, HWND hwnd, LPARAM lParam)
{
	LPSTYLESTRUCT lpSS;
	lpSS = (LPSTYLESTRUCT) lParam;

	OutTraceD("%s: new Style=%x(%s)\n", 
		ApiName, lpSS->styleNew, ExplainStyle(lpSS->styleNew));

	if (dxw.dwFlags1 & FIXWINFRAME){ // set canonical style
		lpSS->styleNew= WS_OVERLAPPEDWINDOW;
	}
	if (dxw.dwFlags1 & LOCKWINSTYLE){ // set to current value
		lpSS->styleNew= (*pGetWindowLong)(hwnd, GWL_STYLE);
	}
	if (dxw.dwFlags1 & PREVENTMAXIMIZE){ // disable maximize settings
		if (lpSS->styleNew & WS_MAXIMIZE){
			OutTraceD("%s: prevent maximize style\n", ApiName);
			lpSS->styleNew &= ~WS_MAXIMIZE;
		}
	}
}

void dxwFixExStyle(char *ApiName, HWND hwnd, LPARAM lParam)
{
	LPSTYLESTRUCT lpSS;
	lpSS = (LPSTYLESTRUCT) lParam;

	OutTraceD("%s: new ExStyle=%x(%s)\n", 
		ApiName, lpSS->styleNew, ExplainExStyle(lpSS->styleNew));

	if (dxw.dwFlags1 & FIXWINFRAME){ // set canonical style
		lpSS->styleNew= 0;
	}
	if (dxw.dwFlags1 & LOCKWINSTYLE){ // set to current value
			lpSS->styleNew= (*pGetWindowLong)(hwnd, GWL_EXSTYLE);
	}
	if (dxw.dwFlags1 & PREVENTMAXIMIZE){ // disable maximize settings
		if (lpSS->styleNew & WS_EX_TOPMOST){
			OutTraceD("%s: prevent EXSTYLE topmost style\n", ApiName);
			lpSS->styleNew &= ~WS_EX_TOPMOST;
		}
	}
}

static LRESULT WINAPI FixWindowProc(char *ApiName, HWND hwnd, UINT Msg, WPARAM wParam, LPARAM *lpParam)
{
	LPARAM lParam;

	lParam=*lpParam;
	OutTraceW("%s: hwnd=%x msg=[0x%x]%s(%x,%x)\n",
		ApiName, hwnd, Msg, ExplainWinMessage(Msg), wParam, lParam);

	switch(Msg){
	// attempt to fix Sleepwalker
	//case WM_NCCALCSIZE:
	//	if (dxw.dwFlags1 & PREVENTMAXIMIZE)
	//		return 0;
	//	break;
	case WM_ERASEBKGND:
		OutTraceD("%s: prevent erase background\n", ApiName);
		return 1; // 1=erased
		break; // useless
	case WM_GETMINMAXINFO:
		dxwFixMinMaxInfo(ApiName, hwnd, lParam);
		break;
	case WM_WINDOWPOSCHANGING:
	case WM_WINDOWPOSCHANGED:
		dxwFixWindowPos(ApiName, hwnd, lParam);
		break;
	case WM_STYLECHANGING:
	case WM_STYLECHANGED:
		if (wParam==GWL_STYLE) 
			dxwFixStyle(ApiName, hwnd, lParam);
		else
			dxwFixExStyle(ApiName, hwnd, lParam);
		break;
	case WM_DISPLAYCHANGE:
		// too late? to be deleted....
		if ((dxw.dwFlags1 & LOCKWINPOS) && dxw.IsFullScreen()) return 0;
		if (dxw.dwFlags1 & PREVENTMAXIMIZE){
			OutTraceD("%s: WM_DISPLAYCHANGE depth=%d size=(%d,%d)\n",
				ApiName, wParam, HIWORD(lParam), LOWORD(lParam));
			return 0;
		}
		break;
	case WM_SIZE:
		if ((dxw.dwFlags1 & LOCKWINPOS) && dxw.IsFullScreen()) return 0;
		if (dxw.dwFlags1 & PREVENTMAXIMIZE){
			if ((wParam == SIZE_MAXIMIZED)||(wParam == SIZE_MAXSHOW)){
				OutTraceD("%s: prevent screen SIZE to fullscreen wparam=%d(%s) size=(%d,%d)\n", ApiName,
					wParam, ExplainResizing(wParam), HIWORD(lParam), LOWORD(lParam));
				return 0; // checked
				//lParam = MAKELPARAM(dxw.GetScreenWidth(), dxw.GetScreenHeight()); 
				//OutTraceD("%s: updated SIZE wparam=%d(%s) size=(%d,%d)\n", ApiName,
				//	wParam, ExplainResizing(wParam), HIWORD(lParam), LOWORD(lParam));
			}
		}
		break;	
	default:
		break;
	}
	
	// marker to run hooked function
	return(-1);
}

LRESULT WINAPI extCallWindowProc(WNDPROC lpPrevWndFunc, HWND hwnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	HRESULT res;

	res=FixWindowProc("CallWindowProc", hwnd, Msg, wParam, &lParam);

	if (res==(HRESULT)-1)
		return (*pCallWindowProc)(lpPrevWndFunc, hwnd, Msg, wParam, lParam);
	else
		return res;
}

LRESULT WINAPI extDefWindowProc(HWND hwnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	HRESULT res;

	res=FixWindowProc("DefWindowProc", hwnd, Msg, wParam, &lParam);

	if (res==(HRESULT)-1)
		return (*pDefWindowProc)(hwnd, Msg, wParam, lParam);
	else
		return res;
}

int WINAPI extGetDeviceCaps(HDC hdc, int nindex)
{
	DWORD res;
	
	res = (*pGDIGetDeviceCaps)(hdc, nindex);
	OutTraceD("GetDeviceCaps: hdc=%x index=%x(%s) res=%x\n",
		hdc, nindex, ExplainDeviceCaps(nindex), res);

	// if you have a bypassed setting, use it first!
	if(pSetDevMode){
		switch(nindex){
		case BITSPIXEL:
		case COLORRES:
			res = pSetDevMode->dmBitsPerPel;
			OutTraceD("GetDeviceCaps: fix BITSPIXEL/COLORRES cap=%x\n",res);
			return res;
		case HORZRES:
			res = pSetDevMode->dmPelsWidth;
			OutTraceD("GetDeviceCaps: fix HORZRES cap=%d\n", res);
			return res;
		case VERTRES:
			res = pSetDevMode->dmPelsHeight;
			OutTraceD("GetDeviceCaps: fix VERTRES cap=%d\n", res);
			return res;
		}
	}

	switch(nindex){
	case VERTRES:
		res= dxw.GetScreenHeight();
		OutTraceD("GetDeviceCaps: fix VERTRES cap=%d\n", res);
		break;
	case HORZRES:
		res= dxw.GetScreenWidth();
		OutTraceD("GetDeviceCaps: fix HORZRES cap=%d\n", res);
		break;
	// WARNING: in no-emu mode, the INIT8BPP and INIT16BPP flags expose capabilities that
	// are NOT implemented and may cause later troubles!
	case RASTERCAPS:
		if(dxw.dwFlags2 & INIT8BPP) {
			res |= RC_PALETTE; // v2.02.12
			OutTraceD("GetDeviceCaps: fix RASTERCAPS setting RC_PALETTE cap=%x\n",res);
		}
		break;
	case BITSPIXEL:
	case COLORRES:
		if(dxw.dwFlags2 & INIT8BPP|INIT16BPP){
			if(dxw.dwFlags2 & INIT8BPP) res = 8;
			if(dxw.dwFlags2 & INIT16BPP) res = 16;
			OutTraceD("GetDeviceCaps: fix BITSPIXEL/COLORRES cap=%d\n",res);
		}
		break;
	}

	if(dxw.dwFlags1 & EMULATESURFACE){
		switch(nindex){
		case RASTERCAPS:
			if((dxw.VirtualPixelFormat.dwRGBBitCount==8) || (dxw.dwFlags2 & INIT8BPP)){
				res = RC_PALETTE;
				OutTraceD("GetDeviceCaps: fix RASTERCAPS setting RC_PALETTE cap=%x\n",res);
			}
			break;
		case BITSPIXEL:
		case COLORRES:
			int PrevRes;
			PrevRes=res;
			if(dxw.VirtualPixelFormat.dwRGBBitCount!=0) res = dxw.VirtualPixelFormat.dwRGBBitCount;
			if(dxw.dwFlags2 & INIT8BPP) res = 8;
			if(dxw.dwFlags2 & INIT16BPP) res = 16;
			if(PrevRes != res) OutTraceD("GetDeviceCaps: fix BITSPIXEL/COLORRES cap=%d\n",res);
			break;
		case SIZEPALETTE:
			res = 256;
			OutTraceD("GetDeviceCaps: fix SIZEPALETTE cap=%x\n",res);
			break;
		case NUMRESERVED:
			res = 0;
			OutTraceD("GetDeviceCaps: fix NUMRESERVED cap=%x\n",res);
			break;
		}
	}
	return res;
}

int WINAPI extGetSystemMetrics(int nindex)
{
	HRESULT res;

	res=(*pGetSystemMetrics)(nindex);
	OutTraceD("GetSystemMetrics: index=%x(%s), res=%d\n", nindex, ExplainsSystemMetrics(nindex), res);

	// if you have a bypassed setting, use it first!
	if(pSetDevMode){
		switch(nindex){
		case SM_CXFULLSCREEN:
		case SM_CXSCREEN:
			res = pSetDevMode->dmPelsWidth;
			OutTraceD("GetDeviceCaps: fix HORZRES cap=%d\n", res);
			return res;
		case SM_CYFULLSCREEN:
		case SM_CYSCREEN:
			res = pSetDevMode->dmPelsHeight;
			OutTraceD("GetDeviceCaps: fix VERTRES cap=%d\n", res);
			return res;
		}
	}

	switch(nindex){
	case SM_CXFULLSCREEN:
	case SM_CXSCREEN:
		res= dxw.GetScreenWidth();
		OutTraceD("GetSystemMetrics: fix SM_CXSCREEN=%d\n", res);
		break;
	case SM_CYFULLSCREEN:
	case SM_CYSCREEN:
		res= dxw.GetScreenHeight();
		OutTraceD("GetSystemMetrics: fix SM_CYSCREEN=%d\n", res);
		break;
	case SM_CMONITORS:
		if((dxw.dwFlags2 & HIDEMULTIMONITOR) && res>1) {
			res=1;
			OutTraceD("GetSystemMetrics: fix SM_CMONITORS=%d\n", res);
		}
		break;
	}

	return res;
}

BOOL WINAPI extScaleWindowExtEx(HDC hdc, int Xnum, int Xdenom, int Ynum, int Ydenom, LPSIZE lpSize)
{
	OutTraceD("ScaleWindowExtEx: hdc=%x num=(%d,%d) denom=(%d,%d) lpSize=%d\n",
		hdc, Xnum, Ynum, Xdenom, Ydenom, lpSize);

	if ((dxw.dwFlags1 & LOCKWINPOS) && dxw.IsFullScreen()) return 1;

	return (*pGDIScaleWindowExtEx)(hdc, Xnum, Xdenom, Ynum, Ydenom, lpSize);
}

LONG WINAPI MyChangeDisplaySettings(char *fname, DEVMODE *lpDevMode, DWORD dwflags)
{
	HRESULT res;

	// save desired settings first v.2.1.89
	// v2.1.95 protect when lpDevMode is null (closing game... Jedi Outcast 
	if(lpDevMode)
		dxw.SetScreenSize(lpDevMode->dmPelsWidth, lpDevMode->dmPelsHeight);

	if ((dwflags==0 || dwflags==CDS_FULLSCREEN) && lpDevMode){

		// v2.2.21: save desired mode to possible use in EnumDisplaySettings wrapper v2.2.21
		SetDevMode=*lpDevMode;
		pSetDevMode=&SetDevMode;

		if (dxw.dwFlags1 & EMULATESURFACE){
			OutTraceD("%s: BYPASS res=DISP_CHANGE_SUCCESSFUL\n", fname);
			return DISP_CHANGE_SUCCESSFUL;
		}
		else{
			DEVMODE NewMode, TryMode;
			int i;

			// set the proper mode
			NewMode = *lpDevMode;
			NewMode.dmPelsHeight = (*GetSystemMetrics)(SM_CYSCREEN);
			NewMode.dmPelsWidth = (*GetSystemMetrics)(SM_CXSCREEN);
			TryMode.dmSize = sizeof(TryMode);
			 OutTraceD("ChangeDisplaySettings: DEBUG looking for size=(%d x %d) bpp=%d\n",
				NewMode.dmPelsWidth, NewMode.dmPelsHeight, NewMode.dmBitsPerPel);
			for(i=0; ;i++){
				if (pEnumDisplaySettings)
					res=(*pEnumDisplaySettings)(NULL, i, &TryMode);
				else
					res=EnumDisplaySettings(NULL, i, &TryMode);
				if(res==0) {
					OutTraceE("%s: ERROR unable to find a matching video mode among %d ones\n", fname, i);
					return DISP_CHANGE_FAILED;
				}
				//OutTraceD("ChangeDisplaySettings: DEBUG index=%d size=(%d x %d) bpp=%x\n",
				//	i, TryMode.dmPelsWidth, TryMode.dmPelsHeight, TryMode.dmBitsPerPel);
				if((NewMode.dmBitsPerPel==TryMode.dmBitsPerPel) &&
					(NewMode.dmPelsHeight==TryMode.dmPelsHeight) &&
					(NewMode.dmPelsWidth==TryMode.dmPelsWidth)) break;
			}
			if(dwflags==CDS_FULLSCREEN) dwflags=0; // no FULLSCREEN
			res=(*ChangeDisplaySettings)(&TryMode, dwflags);
			OutTraceD("%s: fixed size=(%d x %d) bpp=%d res=%x(%s)\n",
				fname, NewMode.dmPelsWidth, NewMode.dmPelsHeight, NewMode.dmBitsPerPel, 
				res, ExplainDisplaySettingsRetcode(res));
			return res;
		}
	}
	else
		return (*ChangeDisplaySettings)(lpDevMode, dwflags);
}

LONG WINAPI extChangeDisplaySettings(DEVMODE *lpDevMode, DWORD dwflags)
{
	if(IsTraceD){
		OutTrace("ChangeDisplaySettings: lpDevMode=%x flags=%x", lpDevMode, dwflags);
		if (lpDevMode) OutTrace(" size=(%d x %d) bpp=%x", 
			lpDevMode->dmPelsWidth, lpDevMode->dmPelsHeight, lpDevMode->dmBitsPerPel);
		OutTrace("\n");
	}

	return MyChangeDisplaySettings("ChangeDisplaySettings", lpDevMode, dwflags);
}

LONG WINAPI extChangeDisplaySettingsEx(LPCTSTR lpszDeviceName, DEVMODE *lpDevMode, HWND hwnd, DWORD dwflags, LPVOID lParam)
{
	if(IsTraceD){
		OutTrace("ChangeDisplaySettingsEx: DeviceName=%s lpDevMode=%x flags=%x", lpszDeviceName, lpDevMode, dwflags);
		if (lpDevMode) OutTrace(" size=(%d x %d) bpp=%x", 
			lpDevMode->dmPelsWidth, lpDevMode->dmPelsHeight, lpDevMode->dmBitsPerPel);
		OutTrace("\n");
	}

	return MyChangeDisplaySettings("ChangeDisplaySettingsEx", lpDevMode, dwflags);
}

LONG WINAPI extEnumDisplaySettings(LPCTSTR lpszDeviceName, DWORD iModeNum, DEVMODE *lpDevMode)
{
	OutTraceD("EnumDisplaySettings: Devicename=%s ModeNum=%x\n", lpszDeviceName, iModeNum);
	if(pSetDevMode && iModeNum==ENUM_CURRENT_SETTINGS){
		lpDevMode=pSetDevMode;
		return 1;
	}
	else
		return (*pEnumDisplaySettings)(lpszDeviceName, iModeNum, lpDevMode);
}

BOOL WINAPI extSetWindowPlacement(const WINDOWPLACEMENT*lpwndpl)
{
	OutTraceD("SetWindowPlacement: BYPASS\n");
	return 1;
}

ATOM WINAPI extRegisterClassExA(WNDCLASSEX *lpwcx)
{
	OutTraceD("RegisterClassEx: PROXED ClassName=%s style=%x(%s)\n", 
		lpwcx->lpszClassName, lpwcx->style, ExplainStyle(lpwcx->style));
	return (*pRegisterClassExA)(lpwcx);
}

BOOL WINAPI extClientToScreen(HWND hwnd, LPPOINT lppoint)
{
	// v2.02.10: fully revised to handle scaled windows
	BOOL res;

	OutTraceB("ClientToScreen: hwnd=%x hWnd=%x FullScreen=%x point=(%d,%d)\n", 
		hwnd, dxw.GethWnd(), dxw.IsFullScreen(), lppoint->x, lppoint->y);
	if (lppoint && dxw.IsFullScreen()){
		*lppoint = dxw.AddCoordinates(*lppoint, dxw.ClientOffset(hwnd));
		OutTraceB("ClientToScreen: FIXED point=(%d,%d)\n", lppoint->x, lppoint->y);
		res=TRUE;
	}
	else {
		res=(*pClientToScreen)(hwnd, lppoint);
	}
	return res;
}

BOOL WINAPI extScreenToClient(HWND hwnd, LPPOINT lppoint)
{
	// v2.02.10: fully revised to handle scaled windows
	BOOL res;
	OutTraceB("ScreenToClient: hwnd=%x hWnd=%x FullScreen=%x point=(%d,%d)\n", 
		hwnd, dxw.GethWnd(), dxw.IsFullScreen(), lppoint->x, lppoint->y);

	if (lppoint && (lppoint->x == -32000) && (lppoint->y == -32000)) return 1;

	if (lppoint && dxw.IsFullScreen()){
		*lppoint = dxw.SubCoordinates(*lppoint, dxw.ClientOffset(hwnd));
		OutTraceB("ScreenToClient: FIXED point=(%d,%d)\n", lppoint->x, lppoint->y);
		res=TRUE;
	}
	else {
		res=(*pScreenToClient)(hwnd, lppoint);
	}
	return res;
}

BOOL WINAPI extGetClientRect(HWND hwnd, LPRECT lpRect)
{
	BOOL ret;
	OutTraceB("GetClientRect: whnd=%x FullScreen=%x\n", hwnd, dxw.IsFullScreen());

	if(!lpRect) return 0;

	// proxed call
	ret=(*pGetClientRect)(hwnd, lpRect);
	if(!ret) {
		OutTraceE("GetClientRect: ERROR hwnd=%x err=%d at %d\n", hwnd, GetLastError(), __LINE__);
		return ret;
	}
	OutTraceB("GetClientRect: actual rect=(%d,%d)-(%d,%d)\n", lpRect->left, lpRect->top, lpRect->right, lpRect->bottom);

	if (dxw.IsDesktop(hwnd)){
		*lpRect = dxw.GetScreenRect();
		OutTraceB("GetClientRect: desktop rect=(%d,%d)-(%d,%d)\n", lpRect->left, lpRect->top, lpRect->right, lpRect->bottom);
	}
	else 
	if (dxw.IsFullScreen()){
		*lpRect=dxw.GetClientRect(*lpRect);
		OutTraceB("GetClientRect: fixed rect=(%d,%d)-(%d,%d)\n", lpRect->left, lpRect->top, lpRect->right, lpRect->bottom);
	}
	return ret;
}

BOOL WINAPI extGetWindowRect(HWND hwnd, LPRECT lpRect)
{
	BOOL ret;
	OutTraceB("GetWindowRect: hwnd=%x hWnd=%x FullScreen=%x\n", hwnd, dxw.GethWnd(), dxw.IsFullScreen());
	ret=(*pGetWindowRect)(hwnd, lpRect);
	if(!ret) {
		OutTraceE("GetWindowRect: GetWindowRect hwnd=%x error %d at %d\n", hwnd, GetLastError(), __LINE__);
		return ret;
	}
	OutTraceB("GetWindowRect: rect=(%d,%d)-(%d,%d)\n", lpRect->left, lpRect->top, lpRect->right, lpRect->bottom);
	
	// minimized windows behaviour
	if((lpRect->left == -32000)||(lpRect->top == -32000)) return ret;

	if (dxw.IsDesktop(hwnd)){
		// to avoid keeping track of window frame
		*lpRect = dxw.GetScreenRect();
		OutTraceB("GetWindowRect: desktop rect=(%d,%d)-(%d,%d)\n", lpRect->left, lpRect->top, lpRect->right, lpRect->bottom);
	}
	else
	if (dxw.IsFullScreen()){
		*lpRect=dxw.GetWindowRect(*lpRect);

		// Diablo fix: it retrieves coordinates for the explorer window, that are as big as the real desktop!!!
		if(lpRect->left < 0) lpRect->left=0;
		if(lpRect->right > (LONG)dxw.GetScreenWidth()) lpRect->right=dxw.GetScreenWidth();
		if(lpRect->top < 0) lpRect->top=0;
		if(lpRect->bottom > (LONG)dxw.GetScreenHeight()) lpRect->bottom=dxw.GetScreenHeight();

		OutTraceB("GetWindowRect: fixed rect=(%d,%d)-(%d,%d)\n", lpRect->left, lpRect->top, lpRect->right, lpRect->bottom);
	}

	return ret;
}

int WINAPI extMapWindowPoints(HWND hWndFrom, HWND hWndTo, LPPOINT lpPoints, UINT cPoints)
{
	// a rarely used API, but responsible for a painful headache: needs hooking for "Commandos 2".

	OutTraceD("MapWindowPoints: hWndFrom=%x hWndTo=%x cPoints=%d FullScreen=%x\n", 
		hWndFrom, hWndTo, cPoints, dxw.IsFullScreen());
	if(IsDebug){
		UINT pi;
		OutTrace("Points: ");
		for(pi=0; pi<cPoints; pi++) OutTrace("(%d,%d)", lpPoints[pi].x, lpPoints[pi].y);
		OutTrace("\n");
	}

	if(hWndTo==HWND_DESKTOP && dxw.IsFullScreen()){
		OutTraceD("MapWindowPoints: fullscreen condition\n"); 
		SetLastError(0);
		return 0; // = 0,0 shift
	}
	else{
		// just proxy it
		return (*pMapWindowPoints)(hWndFrom, hWndTo, lpPoints, cPoints);
	}
}

BOOL WINAPI extPeekMessage(LPMSG lpMsg, HWND hwnd, UINT wMsgFilterMin, UINT wMsgFilterMax, UINT wRemoveMsg)
{
	BOOL res;

	res=(*pPeekMessage)(lpMsg, hwnd, wMsgFilterMin, wMsgFilterMax, wRemoveMsg);
	
	OutTraceW("PeekMessage: lpmsg=%x hwnd=%x filter=(%x-%x) remove=%x msg=%x(%s) wparam=%x, lparam=%x pt=(%d,%d) res=%x\n", 
		lpMsg, lpMsg->hwnd, wMsgFilterMin, wMsgFilterMax, wRemoveMsg, 
		lpMsg->message, ExplainWinMessage(lpMsg->message & 0xFFFF), 
		lpMsg->wParam, lpMsg->lParam, lpMsg->pt.x, lpMsg->pt.y, res);

	// v2.1.74: skip message fix for WM_CHAR to avoid double typing bug
	switch(lpMsg->message){
		//case WM_CHAR:
		case WM_KEYUP:
		case WM_KEYDOWN:
			return res;
	}

	// fix to avoid crash in Warhammer Final Liberation, that evidently intercepts mouse position by 
	// peeking & removing messages from window queue and considering the lParam parameter.
	// v2.1.100 - never alter the mlMsg, otherwise the message is duplicated in the queue! Work on a copy of it.
	if(wRemoveMsg){
		static MSG MsgCopy;
		MsgCopy=*lpMsg;
		MsgCopy.pt=FixMessagePt(dxw.GethWnd(), MsgCopy.pt);
		if((MsgCopy.message <= WM_MOUSELAST) && (MsgCopy.message >= WM_MOUSEFIRST)) MsgCopy.lParam = MAKELPARAM(MsgCopy.pt.x, MsgCopy.pt.y); 
		OutTraceC("PeekMessage: fixed lparam/pt=(%d,%d)\n", MsgCopy.pt.x, MsgCopy.pt.y);
		lpMsg=&MsgCopy;
		GetHookInfo()->CursorX=(short)MsgCopy.pt.x;
		GetHookInfo()->CursorY=(short)MsgCopy.pt.y;
	}

	return res;
}

BOOL WINAPI extGetMessage(LPMSG lpMsg, HWND hwnd, UINT wMsgFilterMin, UINT wMsgFilterMax)
{
	BOOL res;
	HWND FixedHwnd;

	res=(*pGetMessage)(lpMsg, hwnd, wMsgFilterMin, wMsgFilterMax);

	OutTraceW("GetMessage: lpmsg=%x hwnd=%x filter=(%x-%x) msg=%x(%s) wparam=%x, lparam=%x pt=(%d,%d) res=%x\n", 
		lpMsg, lpMsg->hwnd, wMsgFilterMin, wMsgFilterMax, 
		lpMsg->message, ExplainWinMessage(lpMsg->message & 0xFFFF), 
		lpMsg->wParam, lpMsg->lParam, lpMsg->pt.x, lpMsg->pt.y, res);

	// V2.1.68: processing ALL mouse events, to sync mouse over and mouse click events
	// in "Uprising 2", now perfectly working.
	DWORD Message;
	Message=lpMsg->message & 0xFFFF;
	if((Message <= WM_MOUSELAST) && (Message >= WM_MOUSEFIRST)){
		FixedHwnd=(hwnd)?hwnd:dxw.GethWnd();
		lpMsg->pt=FixMessagePt(FixedHwnd, lpMsg->pt);
		lpMsg->lParam = MAKELPARAM(lpMsg->pt.x, lpMsg->pt.y); 
		OutTraceC("PeekMessage: fixed lparam/pt=(%d,%d)\n", lpMsg->pt.x, lpMsg->pt.y);
		GetHookInfo()->CursorX=(short)lpMsg->pt.x;
		GetHookInfo()->CursorY=(short)lpMsg->pt.y;
	}
	return res;
}

// intercept GetProcAddress to initialize DirectDraw hook pointers.
// This is necessary in "The Sims" game, that loads DirectDraw dinamically


extern void HookModule(HMODULE, int);
extern void HookSysLibs(HMODULE);

HMODULE SysLibs[SYSLIBIDX_MAX];

HMODULE WINAPI LoadLibraryExWrapper(LPCTSTR lpFileName, HANDLE hFile, DWORD dwFlags, char *api)
{
	HMODULE libhandle;
	int idx;
	
	//if(!strcmp(lpFileName, "d3d9.dll") && GetModuleHandle(lpFileName)) return GetModuleHandle(lpFileName); // attempt to avoid loading same dll twice....

	libhandle=(*pLoadLibraryExA)(lpFileName, hFile, dwFlags);
	OutTraceD("%s: FileName=%s hFile=%x Flags=%x(%s) hmodule=%x\n", api, lpFileName, hFile, dwFlags, ExplainLoadLibFlags(dwFlags), libhandle);
	if(!libhandle){
		OutTraceE("%s: ERROR FileName=%s err=%d\n", api, lpFileName, GetLastError());
		return libhandle;
	}

	// when loaded with LOAD_LIBRARY_AS_DATAFILE or LOAD_LIBRARY_AS_DATAFILE_EXCLUSIVE flags, 
	// there's no symbol map, then itìs no possible to hook function calls.
	if(dwFlags & (LOAD_LIBRARY_AS_DATAFILE_EXCLUSIVE|LOAD_LIBRARY_AS_DATAFILE)) return libhandle;

	idx=dxw.GetDLLIndex((char *)lpFileName);
	if(idx != -1) SysLibs[idx]=libhandle;
	// handle custom OpenGL library
	if(!lstrcmpi(lpFileName,dxw.CustomOpenGLLib)){
		idx=SYSLIBIDX_OPENGL;
		SysLibs[idx]=libhandle;
	}
	//if (idx == SYSLIBIDX_MAX) HookModule(libhandle, 0);
	if (idx == -1) HookModule(libhandle, 0);
	return libhandle;
}

HMODULE WINAPI extLoadLibraryA(LPCTSTR lpFileName)
{
	return LoadLibraryExWrapper(lpFileName, NULL, 0, "LoadLibraryA");
}

HMODULE WINAPI extLoadLibraryW(LPCWSTR lpFileName)
{
	char sFileName[256+1];
	wcstombs_s(NULL, sFileName, lpFileName, 80);
	return LoadLibraryExWrapper(sFileName, NULL, 0, "LoadLibraryW");;
}

HMODULE WINAPI extLoadLibraryExA(LPCTSTR lpFileName, HANDLE hFile, DWORD dwFlags)
{
	return LoadLibraryExWrapper(lpFileName, hFile, dwFlags, "LoadLibraryExA");
}

HMODULE WINAPI extLoadLibraryExW(LPCWSTR lpFileName, HANDLE hFile, DWORD dwFlags)
{
	char sFileName[256+1];
	wcstombs_s(NULL, sFileName, lpFileName, 80);
	return LoadLibraryExWrapper(sFileName, hFile, dwFlags, "LoadLibraryExW");;
}

extern DirectDrawCreate_Type pDirectDrawCreate;
extern DirectDrawCreateEx_Type pDirectDrawCreateEx;
extern HRESULT WINAPI extDirectDrawCreate(GUID FAR *, LPDIRECTDRAW FAR *, IUnknown FAR *);
extern HRESULT WINAPI extDirectDrawCreateEx(GUID FAR *, LPDIRECTDRAW FAR *, REFIID, IUnknown FAR *);
extern GetProcAddress_Type pGetProcAddress;
//extern HRESULT STDAPICALLTYPE extCoCreateInstance(REFCLSID, LPUNKNOWN, DWORD, REFIID, LPVOID FAR*);

FARPROC WINAPI extGetProcAddress(HMODULE hModule, LPCSTR proc)
{
	FARPROC ret;
	int idx;

	// WARNING: seems to be called with bad LPCSTR value....
	// from MSDN:
	// The function or variable name, or the function's ordinal value. 
	// If this parameter is an ordinal value, it must be in the low-order word; 
	// the high-order word must be zero.

	OutTraceD("GetProcAddress: hModule=%x proc=%s\n", hModule, ProcToString(proc));

	for(idx=0; idx<SYSLIBIDX_MAX; idx++){
		if(SysLibs[idx]==hModule) break;
	}

	// to do: the else condition: the program COULD load addresses by ordinal value ... done ??
	// to do: CoCreateInstanceEx
	if((DWORD)proc & 0xFFFF0000){
		FARPROC remap;
		switch(idx){
		case SYSLIBIDX_DIRECTDRAW:
			if (remap=Remap_ddraw_ProcAddress(proc, hModule)) return remap;
			break;
		case SYSLIBIDX_USER32:
			if (remap=Remap_user32_ProcAddress(proc, hModule)) return remap;
			break;
		case SYSLIBIDX_KERNEL32:
			if (remap=Remap_kernel32_ProcAddress(proc, hModule)) return remap;
			break;
		case SYSLIBIDX_OLE32: 
			if (remap=Remap_ole32_ProcAddress(proc, hModule)) return remap;
			break;
		case SYSLIBIDX_DIRECT3D8:
			if (remap=Remap_d3d8_ProcAddress(proc, hModule)) return remap;
			break;
		case SYSLIBIDX_DIRECT3D9:
			if (remap=Remap_d3d9_ProcAddress(proc, hModule)) return remap;
			break;
		case SYSLIBIDX_DIRECT3D10:
			if (remap=Remap_d3d10_ProcAddress(proc, hModule)) return remap;
			break;
		case SYSLIBIDX_DIRECT3D10_1:
			if (remap=Remap_d3d10_1_ProcAddress(proc, hModule)) return remap;
			break;
		case SYSLIBIDX_DIRECT2D11:
			if (remap=Remap_d3d11_ProcAddress(proc, hModule)) return remap;
			break;
		case SYSLIBIDX_OPENGL:
			if (remap=Remap_gl_ProcAddress(proc, hModule)) return remap;
			break;
		case SYSLIBIDX_MSVFW:
			if (remap=Remap_vfw_ProcAddress(proc, hModule)) return remap;
			break;
		case SYSLIBIDX_SMACK:
			if (remap=Remap_smack_ProcAddress(proc, hModule)) return remap;
			break;
		default:
			break; 
		}
	}
	else {
		switch(idx){
		case SYSLIBIDX_DIRECTDRAW:
			switch((DWORD)proc){
			case 0x0008: // DirectDrawCreate
				pDirectDrawCreate=(DirectDrawCreate_Type)(*pGetProcAddress)(hModule, proc);
				OutTraceD("GetProcAddress: hooking proc=%s at addr=%x\n", ProcToString(proc), pDirectDrawCreate);
				return (FARPROC)extDirectDrawCreate;
				break;
			case 0x000A: // DirectDrawCreateEx
				pDirectDrawCreateEx=(DirectDrawCreateEx_Type)(*pGetProcAddress)(hModule, proc);
				OutTraceD("GetProcAddress: hooking proc=%s at addr=%x\n", ProcToString(proc), pDirectDrawCreateEx);
				return (FARPROC)extDirectDrawCreateEx;
				break;
			case 0x000B: // DirectDrawEnumerateA
				pDirectDrawEnumerate=(DirectDrawEnumerate_Type)(*pGetProcAddress)(hModule, proc);
				OutTraceP("GetProcAddress: hooking proc=%s at addr=%x\n", proc, pDirectDrawEnumerate);
				return (FARPROC)extDirectDrawEnumerateProxy;
				break;
			case 0x000C: // DirectDrawEnumerateExA
				pDirectDrawEnumerateEx=(DirectDrawEnumerateEx_Type)(*pGetProcAddress)(hModule, proc);
				OutTraceP("GetProcAddress: hooking proc=%s at addr=%x\n", proc, pDirectDrawEnumerateEx);
				return (FARPROC)extDirectDrawEnumerateExProxy;
				break;
			}
			break;
		case SYSLIBIDX_USER32:
			if ((DWORD)proc == 0x0020){ // ChangeDisplaySettingsA
				pChangeDisplaySettings=(ChangeDisplaySettings_Type)(*pGetProcAddress)(hModule, proc);
				OutTraceD("GetProcAddress: hooking proc=%s at addr=%x\n", ProcToString(proc), pChangeDisplaySettings);
				return (FARPROC)extChangeDisplaySettings;
			}
			break;
#ifndef ANTICHEATING
		case SYSLIBIDX_KERNEL32:
			if ((DWORD)proc == 0x022D){ // "IsDebuggerPresent"
				OutTraceD("GetProcAddress: hooking proc=%s at addr=%x\n", ProcToString(proc), extIsDebuggerPresent);
				return (FARPROC)extIsDebuggerPresent;
			}
#endif
		case SYSLIBIDX_OLE32:
			if ((DWORD)proc == 0x0011){ // "CoCreateInstance"
				pCoCreateInstance=(CoCreateInstance_Type)(*pGetProcAddress)(hModule, proc);
				OutTraceD("GetProcAddress: hooking proc=%s at addr=%x\n", ProcToString(proc), pCoCreateInstance);
				return (FARPROC)extCoCreateInstance;
			}
			break;
		}
	}

	ret=(*pGetProcAddress)(hModule, proc);
	OutTraceD("GetProcAddress: ret=%x\n", ret);
	return ret;
}

HDC WINAPI extGDIGetDC(HWND hwnd)
{
	HDC ret;
	HWND lochwnd;

	OutTraceD("GDI.GetDC: hwnd=%x\n", hwnd);
	lochwnd=hwnd;
	if (dxw.IsFullScreen() && ((hwnd==0) || (hwnd==(*pGetDesktopWindow)()))) {
		OutTraceD("GDI.GetDC: desktop remapping hwnd=%x->%x\n", hwnd, dxw.GethWnd());
		lochwnd=dxw.GethWnd();
	}
	ret=(*pGDIGetDC)(lochwnd);
	if(ret){
		OutTraceD("GDI.GetDC: hwnd=%x ret=%x\n", lochwnd, ret);
	}
	else{
		int err;
		err=GetLastError();
		OutTraceE("GDI.GetDC ERROR: hwnd=%x err=%d at %d\n", lochwnd, err, __LINE__);
		if((err==ERROR_INVALID_WINDOW_HANDLE) && (lochwnd!=hwnd)){
			ret=(*pGDIGetDC)(hwnd);	
			if(ret)
				OutTraceD("GDI.GetDC: hwnd=%x ret=%x\n", hwnd, ret);
			else
				OutTraceE("GDI.GetDC ERROR: hwnd=%x err=%d at %d\n", hwnd, GetLastError(), __LINE__);
		}
	}

	return ret;
}

HDC WINAPI extGDIGetWindowDC(HWND hwnd)
{
	HDC ret;
	HWND lochwnd;
	OutTraceD("GDI.GetWindowDC: hwnd=%x\n", hwnd);
	lochwnd=hwnd;
	if ((hwnd==0) || (hwnd==(*pGetDesktopWindow)())) {
		OutTraceD("GDI.GetWindowDC: desktop remapping hwnd=%x->%x\n", hwnd, dxw.GethWnd());
		lochwnd=dxw.GethWnd();
	}
	ret=(*pGDIGetWindowDC)(lochwnd);
	if(ret){
		OutTraceD("GDI.GetWindowDC: hwnd=%x ret=%x\n", lochwnd, ret);
	}
	else{
		int err;
		err=GetLastError();
		OutTraceE("GDI.GetWindowDC ERROR: hwnd=%x err=%d at %d\n", lochwnd, err, __LINE__);
		if((err==ERROR_INVALID_WINDOW_HANDLE) && (lochwnd!=hwnd)){
			ret=(*pGDIGetWindowDC)(hwnd);
			if(ret)
				OutTraceD("GDI.GetWindowDC: hwnd=%x ret=%x\n", hwnd, ret);
			else
				OutTraceE("GDI.GetWindowDC ERROR: hwnd=%x err=%d at %d\n", hwnd, GetLastError(), __LINE__);
		}
	}
	return ret;
}

int WINAPI extGDIReleaseDC(HWND hwnd, HDC hDC)
{
	int res;

	OutTraceD("GDI.ReleaseDC: hwnd=%x hdc=%x\n", hwnd, hDC);
	if (hwnd==0) hwnd=dxw.GethWnd();
	res=(*pGDIReleaseDC)(hwnd, hDC);
	if (!res) OutTraceE("GDI.ReleaseDC ERROR: err=%d at %d\n", GetLastError(), __LINE__);
	return(res);
}

HDC WINAPI extGDICreateDC(LPSTR Driver, LPSTR Device, LPSTR Output, CONST DEVMODE *InitData)
{
	HDC WinHDC, RetHDC;
	OutTraceD("GDI.CreateDC: Driver=%s Device=%s Output=%s InitData=%x\n", 
		Driver?Driver:"(NULL)", Device?Device:"(NULL)", Output?Output:"(NULL)", InitData);

	if (!Driver || !strncmp(Driver,"DISPLAY",7)) {
		OutTraceD("GDI.CreateDC: returning window surface DC\n");
		WinHDC=(*pGDIGetDC)(dxw.GethWnd());
		RetHDC=(*pGDICreateCompatibleDC)(WinHDC);
		(*pGDIReleaseDC)(dxw.GethWnd(), WinHDC);
	}
	else{
		RetHDC=(*pGDICreateDC)(Driver, Device, Output, InitData);
	}
	if(RetHDC)
		OutTraceD("GDI.CreateDC: returning HDC=%x\n", RetHDC);
	else
		OutTraceE("GDI.CreateDC ERROR: err=%d at %d\n", GetLastError(), __LINE__);
	return RetHDC;
}

HDC WINAPI extGDICreateCompatibleDC(HDC hdc)
{
	HDC RetHdc, SrcHdc;
	extern LPDIRECTDRAWSURFACE lpDDSHDC;
	extern GetDC_Type pGetDC;
	DWORD LastError;

	OutTraceD("GDI.CreateCompatibleDC: hdc=%x\n", hdc);
	if(hdc==0){
		SrcHdc=(*pGDIGetDC)(dxw.GethWnd());
		OutTraceD("GDI.CreateCompatibleDC: duplicating win HDC hWnd=%x\n", dxw.GethWnd()); 
	}

	// eliminated error message for errorcode 0.
	SetLastError(0);
	RetHdc=(*pGDICreateCompatibleDC)(hdc);
	LastError=GetLastError();
	if(!LastError)
		OutTraceD("GDI.CreateCompatibleDC: returning HDC=%x\n", RetHdc);
	else
		OutTraceE("GDI.CreateCompatibleDC ERROR: err=%d at %d\n", LastError, __LINE__);
	return RetHdc;
}

BOOL WINAPI extGDIBitBlt(HDC hdcDest, int nXDest, int nYDest, int nWidth, int nHeight, HDC hdcSrc, int nXSrc, int nYSrc, DWORD dwRop)
{
	BOOL res;
	extern BOOL isWithinDialog;

	OutTraceD("GDI.BitBlt: HDC=%x nXDest=%d nYDest=%d nWidth=%d nHeight=%d hdcSrc=%x nXSrc=%d nYSrc=%d dwRop=%x(%s)\n", 
		hdcDest, nXDest, nYDest, nWidth, nHeight, hdcSrc, nXSrc, nYSrc, dwRop, ExplainROP(dwRop));

	if (dxw.HandleFPS()) return TRUE;

	// beware: HDC could refer to screen DC that are written directly on screen, or memory DC that will be scaled to
	// the screen surface later on, on ReleaseDC or ddraw Blit / Flip operation. Scaling of rect coordinates is 
	// needed only in the first case, and must be avoided on the second, otherwise the image would be scaled twice!

	if (dxw.IsFullScreen() && (OBJ_DC == GetObjectType(hdcDest))){
		int nWDest, nHDest;
		nWDest= nWidth;
		nHDest= nHeight;
		dxw.MapClient(&nXDest, &nYDest, &nWDest, &nHDest);
		if (dxw.dwFlags2 & SHOWFPSOVERLAY) dxw.ShowFPS(hdcDest);		
		res=(*pGDIStretchBlt)(hdcDest, nXDest, nYDest, nWDest, nHDest, hdcSrc, nXSrc, nYSrc, nWidth, nHeight, dwRop);
	}
	else {
		res=(*pGDIBitBlt)(hdcDest, nXDest, nYDest, nWidth, nHeight, hdcSrc, nXSrc, nYSrc, dwRop);
	}
	if(!res) OutTraceE("GDI.BitBlt: ERROR err=%d at %d\n", GetLastError(), __LINE__);

	return res;
}

BOOL WINAPI extGDIPatBlt(HDC hdcDest, int nXDest, int nYDest, int nWidth, int nHeight, DWORD dwRop)
{
	BOOL res;

	OutTraceD("GDI.PatBlt: HDC=%x nXDest=%d nYDest=%d nWidth=%d nHeight=%d dwRop=%x(%s)\n", 
		hdcDest, nXDest, nYDest, nWidth, nHeight, dwRop, ExplainROP(dwRop));

	if (dxw.HandleFPS()) return TRUE;

	if (dxw.IsFullScreen() && (OBJ_DC == GetObjectType(hdcDest))){
		int nWDest, nHDest;
		dxw.MapClient(&nXDest, &nYDest, &nWDest, &nHDest);
		if (dxw.dwFlags2 & SHOWFPSOVERLAY) dxw.ShowFPS(hdcDest);		
		res=(*pGDIPatBlt)(hdcDest, nXDest, nYDest, nWDest, nHDest, dwRop);
	}
	else {
		res=(*pGDIPatBlt)(hdcDest, nXDest, nYDest, nWidth, nHeight, dwRop);
	}
	if(!res) OutTraceE("GDI.PatBlt: ERROR err=%d at %d\n", GetLastError(), __LINE__);

	return res;
}

BOOL WINAPI extGDIStretchBlt(HDC hdcDest, int nXDest, int nYDest, int nWidth, int nHeight, 
							 HDC hdcSrc, int nXSrc, int nYSrc, int nWSrc, int nHSrc, DWORD dwRop)
{
	BOOL res;

	OutTraceD("GDI.StretchBlt: HDC=%x nXDest=%d nYDest=%d nWidth=%d nHeight=%d hdcSrc=%x nXSrc=%d nYSrc=%d nWSrc=%d nHSrc=%d dwRop=%x(%s)\n", 
		hdcDest, nXDest, nYDest, nWidth, nHeight, hdcSrc, nXSrc, nYSrc, nWSrc, nHSrc, dwRop, ExplainROP(dwRop));

	if (dxw.HandleFPS()) return TRUE;

	// to do: what happend if StretchBlt is applied on screen DC ?

	if (dxw.dwFlags2 & SHOWFPSOVERLAY) dxw.ShowFPS(hdcDest);		
	res=(*pGDIStretchBlt)(hdcDest, nXDest, nYDest, nWidth, nHeight, hdcSrc, nXSrc, nYSrc, nWSrc, nHSrc, dwRop);
	if(!res) OutTraceE("GDI.StretchBlt: ERROR err=%d at %d\n", GetLastError(), __LINE__);
	return res;
}

BOOL WINAPI extGDIDeleteDC(HDC hdc)
{
	BOOL res;
	OutTraceD("GDI.DeleteDC: hdc=%x\n", hdc);
	res=(*pGDIDeleteDC)(hdc);
	if(!res) OutTraceE("GDI.DeleteDC: ERROR err=%d at %d\n", GetLastError(), __LINE__);
	return res;
}

static HANDLE AutoRefreshThread;
static DWORD dwThrdId;
void AutoRefresh(HDC hdc)
{
	while(1){
		(*pSleep)(10);
		(*pInvalidateRect)(dxw.GethWnd(), 0, FALSE);
	}
}

int WINAPI extGDISaveDC(HDC hdc)
{
	int ret;

	ret=(*pGDISaveDC)(hdc);
	//ret=1;
	OutTraceD("GDI.SaveDC: hdc=%x ret=%x\n", hdc, ret);
	//AutoRefreshThread=CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)AutoRefresh, (LPVOID)hdc, 0, &dwThrdId);
	return ret;
}

BOOL WINAPI extGDIRestoreDC(HDC hdc, int nSavedDC)
{
	BOOL ret;

	ret=(*pGDIRestoreDC)(hdc, nSavedDC);
	//ret=1;
	OutTraceD("GDI.RestoreDC: hdc=%x nSavedDC=%x ret=%x\n", hdc, nSavedDC, ret);
	//TerminateThread(AutoRefreshThread, 0);
	return ret;
}

/* -------------------------------------------------------------------- */
// directdraw supported GDI calls
/* -------------------------------------------------------------------- */

// PrimHDC: DC handle of the selected DirectDraw primary surface. NULL when invalid.
static HDC PrimHDC=NULL;

HDC WINAPI extDDCreateCompatibleDC(HDC hdc)
{
	HDC RetHdc, SrcHdc;
	extern GetDC_Type pGetDC;

	OutTraceD("GDI.CreateCompatibleDC: hdc=%x\n", hdc);

	if(hdc==0 && pGetDC && dxw.IsFullScreen()){
		dxw.SetPrimarySurface();
		(*pGetDC)(dxw.lpDDSPrimHDC,&SrcHdc);
		OutTraceD("GDI.CreateCompatibleDC: duplicating screen HDC lpDDSPrimHDC=%x\n", dxw.lpDDSPrimHDC); 
		RetHdc=(*pGDICreateCompatibleDC)(SrcHdc);
		(*pReleaseDC)(dxw.lpDDSPrimHDC,SrcHdc);
	}
	else
		RetHdc=(*pGDICreateCompatibleDC)(hdc);

	if(RetHdc)
		OutTraceD("GDI.CreateCompatibleDC: returning HDC=%x\n", RetHdc);
	else
		OutTraceE("GDI.CreateCompatibleDC ERROR: err=%d at %d\n", GetLastError(), __LINE__);

	return RetHdc;
}

BOOL WINAPI extDDDeleteDC(HDC hdc)
{
	BOOL res;

	OutTraceD("GDI.DeleteDC: hdc=%x\n", hdc);

	res=(*pGDIDeleteDC)(hdc);
	if(!res) OutTraceE("GDI.DeleteDC: ERROR err=%d at %d\n", GetLastError(), __LINE__);
	return res;
}

static HDC WINAPI winDDGetDC(HWND hwnd, char *api)
{
	HDC hdc;
	HRESULT res;
	extern HRESULT WINAPI extGetDC(LPDIRECTDRAWSURFACE, HDC FAR *);

	OutTraceD("%s: hwnd=%x\n", api, hwnd);

	dxw.ResetPrimarySurface();
	dxw.SetPrimarySurface();

	if(dxw.lpDDSPrimHDC){ 
		if (PrimHDC){
			OutTraceD("%s: reusing primary hdc\n", api);
			(*pUnlockMethod(dxw.lpDDSPrimHDC))(dxw.lpDDSPrimHDC, NULL);
			hdc=PrimHDC;
		}
		else{
			OutTraceD("%s: get hdc from PRIMARY surface lpdds=%x\n", api, dxw.lpDDSPrimHDC);
			res=extGetDC(dxw.lpDDSPrimHDC,&hdc);
			if(res) {
				OutTraceE("%s: GetDC(%x) ERROR %x(%s) at %d\n", api, dxw.lpDDSPrimHDC, res, ExplainDDError(res), __LINE__);
				if(res==DDERR_DCALREADYCREATED){
					// try recovery....
					(*pReleaseDC)(dxw.lpDDSPrimHDC,NULL);
					res=extGetDC(dxw.lpDDSPrimHDC,&hdc);
				}
				if(res)return 0;
			}
			PrimHDC=hdc;
		}
	}
	else {
		hdc=(*pGDIGetDC)(hwnd ? hwnd : dxw.GethWnd());
		OutTraceD("%s: returning window DC handle hwnd=%x hdc=%x\n", api, hwnd, hdc);
		PrimHDC=NULL;
	}

	if(hdc)
		OutTraceD("%s: hwnd=%x hdc=%x\n", api, hwnd, hdc);
	else
		OutTraceE("%s: ERROR err=%d at %d\n", api, GetLastError, __LINE__);
	return(hdc);
}

HDC WINAPI extDDCreateDC(LPSTR Driver, LPSTR Device, LPSTR Output, CONST DEVMODE *InitData)
{
	HDC RetHDC;
	OutTraceD("GDI.CreateDC: Driver=%s Device=%s Output=%s InitData=%x\n", 
		Driver?Driver:"(NULL)", Device?Device:"(NULL)", Output?Output:"(NULL)", InitData);

	if (!Driver || !strncmp(Driver,"DISPLAY",7)) {
		//HDC PrimHDC;
		LPDIRECTDRAWSURFACE lpdds;
		OutTraceD("GDI.CreateDC: returning primary surface DC\n");
		lpdds=dxw.GetPrimarySurface();
		(*pGetDC)(lpdds, &PrimHDC);
		RetHDC=(*pGDICreateCompatibleDC)(PrimHDC);
		(*pReleaseDC)(lpdds, PrimHDC);
	}
	else{
		RetHDC=(*pGDICreateDC)(Driver, Device, Output, InitData);
	}
	if(RetHDC)
		OutTraceD("GDI.CreateDC: returning HDC=%x\n", RetHDC);
	else
		OutTraceE("GDI.CreateDC ERROR: err=%d at %d\n", GetLastError(), __LINE__);
	return RetHDC;
}

HDC WINAPI extDDGetDC(HWND hwnd)
{
	HDC ret;
	ret=winDDGetDC(hwnd, "GDI.GetDC");
	return ret;
}

HDC WINAPI extDDGetWindowDC(HWND hwnd)
{
	HDC ret;
	ret=winDDGetDC(hwnd, "GDI.GetWindowDC");
	return ret;
}

int WINAPI extDDReleaseDC(HWND hwnd, HDC hDC)
{
	int res;
	extern HRESULT WINAPI extReleaseDC(LPDIRECTDRAWSURFACE, HDC);

	OutTraceD("GDI.ReleaseDC: hwnd=%x hdc=%x\n", hwnd, hDC);
	res=0;
	if ((hDC == PrimHDC) || (hwnd==0)){
		dxw.SetPrimarySurface();
		OutTraceD("GDI.ReleaseDC: refreshing primary surface lpdds=%x\n",dxw.lpDDSPrimHDC);
		if(!dxw.lpDDSPrimHDC) return 0;
		extReleaseDC(dxw.lpDDSPrimHDC, hDC);
		PrimHDC=NULL;
		res=1; // 1 = OK
	}
	else {
		res=(*pGDIReleaseDC)(hwnd, hDC);
		if (!res) OutTraceE("GDI.ReleaseDC: ERROR err=%d at %d\n", GetLastError(), __LINE__);
	}
	return(res);
}

BOOL WINAPI extDDBitBlt(HDC hdcDest, int nXDest, int nYDest, int nWidth, int nHeight, HDC hdcSrc, int nXSrc, int nYSrc, DWORD dwRop)
{
	BOOL ret;
	HRESULT res;
	extern HRESULT WINAPI extGetDC(LPDIRECTDRAWSURFACE, HDC FAR *);

	OutTraceD("GDI.BitBlt: HDC=%x nXDest=%d nYDest=%d nWidth=%d nHeight=%d hdcSrc=%x nXSrc=%d nYSrc=%d dwRop=%x(%s)\n", 
		hdcDest, nXDest, nYDest, nWidth, nHeight, hdcSrc, nXSrc, nYSrc, dwRop, ExplainROP(dwRop));

	ret=1; // OK

	if(hdcDest==0) hdcDest=PrimHDC;
	if(hdcDest==0) {
		dxw.ResetPrimarySurface();
		dxw.SetPrimarySurface();
		res=extGetDC(dxw.lpDDSPrimHDC, &PrimHDC);
		hdcDest=PrimHDC;
	}

	res=(*pGDIBitBlt)(hdcDest, nXDest, nYDest, nWidth, nHeight, hdcSrc, nXSrc, nYSrc, dwRop);
	if(!res) OutTraceE("GDI.BitBlt: ERROR err=%d at %d\n", GetLastError(), __LINE__);

	res=(*pGDIBitBlt)(NULL, nXDest, nYDest, nWidth, nHeight, hdcSrc, nXSrc, nYSrc, dwRop);

	if(!res) ret=0;
	return ret;
}

BOOL WINAPI extDDStretchBlt(HDC hdcDest, int nXDest, int nYDest, int nWidth, int nHeight, 
							 HDC hdcSrc, int nXSrc, int nYSrc, int nWSrc, int nHSrc, DWORD dwRop)
{
	BOOL ret;
	HRESULT res;
	RECT ClientRect;

	OutTraceD("GDI.StretchBlt: HDC=%x nXDest=%d nYDest=%d nWidth=%d nHeight=%d hdcSrc=%x nXSrc=%d nYSrc=%d nWSrc=%x nHSrc=%x dwRop=%x\n", 
		hdcDest, nXDest, nYDest, nWidth, nHeight, hdcSrc, nXSrc, nYSrc, nWSrc, nHSrc, dwRop);

	if(hdcDest != hdcSrc){
		(*pGetClientRect)(dxw.GethWnd(),&ClientRect);
		ret=(*pGDIStretchBlt)(hdcDest, nXDest, nYDest, nWidth, nHeight, hdcSrc, nXSrc, nYSrc, nWidth, nHeight, dwRop);
		if(!ret) {
			OutTraceE("GDI.StretchBlt: ERROR err=%d at %d\n", GetLastError(), __LINE__);
			return ret;
		}
	}
	dxw.SetPrimarySurface();
	OutTraceD("GDI.StretchBlt: refreshing primary surface lpdds=%x\n",dxw.lpDDSPrimHDC);
	sBlt("GDI.StretchBlt", dxw.lpDDSPrimHDC, NULL, dxw.lpDDSPrimHDC, NULL, 0, NULL, 0);
	res=(*pUnlockMethod(dxw.lpDDSPrimHDC))(dxw.lpDDSPrimHDC, NULL);
	return ret;
}

HDC WINAPI extBeginPaint(HWND hwnd, LPPAINTSTRUCT lpPaint)
{
	HDC hdc;
	extern HRESULT WINAPI extGetDC(LPDIRECTDRAWSURFACE, HDC FAR *);

	// proxy part ...
	OutTraceD("GDI.BeginPaint: hwnd=%x lpPaint=%x FullScreen=%x\n", hwnd, lpPaint, dxw.IsFullScreen());
	hdc=(*pBeginPaint)(hwnd, lpPaint);

	// if not in fullscreen mode, that's all!
	if(!dxw.IsFullScreen()) return hdc;

	// on MAPGDITOPRIMARY, return the PrimHDC handle instead of the window DC
	if(dxw.dwFlags1 & MAPGDITOPRIMARY) {
		if(pGetDC && dxw.lpDDSPrimHDC){
			extGetDC(dxw.lpDDSPrimHDC,&PrimHDC);
			OutTraceD("GDI.BeginPaint: redirect hdc=%x -> PrimHDC=%x\n", hdc, PrimHDC);
			hdc=PrimHDC;
		}
		else {
			OutTraceD("GDI.BeginPaint: hdc=%x\n", hdc);
		}
	}

	// on CLIENTREMAPPING, resize the paint area to virtual screen size
	if(dxw.dwFlags1 & CLIENTREMAPPING){
		lpPaint->rcPaint.top=0;
		lpPaint->rcPaint.left=0;
		lpPaint->rcPaint.right=dxw.GetScreenWidth();
		lpPaint->rcPaint.bottom=dxw.GetScreenHeight();
	}

	return hdc;
}

BOOL WINAPI extEndPaint(HWND hwnd, const PAINTSTRUCT *lpPaint)
{
	BOOL ret;
	HRESULT WINAPI extReleaseDC(LPDIRECTDRAWSURFACE lpdds, HDC FAR hdc);

	// proxy part ...
	OutTraceD("GDI.EndPaint: hwnd=%x lpPaint=%x\n", hwnd, lpPaint);
	ret=(*pEndPaint)(hwnd, lpPaint);
	OutTraceD("GDI.EndPaint: hwnd=%x ret=%x\n", hwnd, ret);
	if(!ret) OutTraceE("GDI.EndPaint ERROR: err=%d at %d\n", GetLastError(), __LINE__);

	// if not in fullscreen mode, that's all!
	if(!dxw.IsFullScreen()) return ret;

	// v2.02.09: on MAPGDITOPRIMARY, release the PrimHDC handle 
	if(dxw.dwFlags1 & MAPGDITOPRIMARY) {
		if(pReleaseDC && dxw.lpDDSPrimHDC){
			extReleaseDC(dxw.lpDDSPrimHDC, PrimHDC);
			OutTraceD("GDI.EndPaint: released hdc=%x\n", PrimHDC);
		}
	}
	
	return ret;
}

/* --------------------------------------------------------------------------- */
// C&C Tiberian Sun: mixes DirectDraw with GDI Dialogues.
// To make them visible, the lpDialog call had to be hooked to insert a periodic 
// InvalidateRect call to make GDI appear on screen
/* --------------------------------------------------------------------------- */

BOOL __cdecl TraceChildWin(HWND hwnd, LPARAM lParam)
{
	POINT pos={0,0};
	RECT child;
	(*pGetClientRect)(hwnd, &child);
	(*pClientToScreen)(hwnd,&pos);
	OutTraceD("Father hwnd=%x has child=%x pos=(%d,%d) size=(%d,%d)\n",
		HWND(lParam), hwnd, pos.x, pos.y, child.right, child.bottom);
	return TRUE;
}

BOOL isWithinDialog=FALSE;

HWND WINAPI extCreateDialogIndirectParam(HINSTANCE hInstance, LPCDLGTEMPLATE lpTemplate, HWND hWndParent, DLGPROC lpDialogFunc, LPARAM lParamInit)
{
	HWND RetHWND;
	isWithinDialog=TRUE;
	OutTraceD("CreateDialogIndirectParam: hInstance=%x lpTemplate=%s hWndParent=%x lpDialogFunc=%x lParamInit=%x\n",
		hInstance, "tbd", hWndParent, lpDialogFunc, lParamInit);
	if(hWndParent==NULL) hWndParent=dxw.GethWnd();
	RetHWND=(*pCreateDialogIndirectParam)(hInstance, lpTemplate, hWndParent, lpDialogFunc, lParamInit);

	WhndStackPush(RetHWND, (WNDPROC)lpDialogFunc);
	if(!(*pSetWindowLong)(RetHWND, DWL_DLGPROC, (LONG)extDialogWindowProc))
		OutTraceE("SetWindowLong: ERROR err=%d at %d\n", GetLastError(), __LINE__);

	OutTraceD("CreateDialogIndirectParam: hwnd=%x\n", RetHWND);
	isWithinDialog=FALSE;
	//if (IsDebug) EnumChildWindows(RetHWND, (WNDENUMPROC)TraceChildWin, (LPARAM)RetHWND);
	return RetHWND;
}

HWND WINAPI extCreateDialogParam(HINSTANCE hInstance, LPCTSTR lpTemplateName, HWND hWndParent, DLGPROC lpDialogFunc, LPARAM lParamInit)
{
	HWND RetHWND;
	isWithinDialog=TRUE;
	OutTraceD("CreateDialogParam: hInstance=%x lpTemplateName=%s hWndParent=%x lpDialogFunc=%x lParamInit=%x\n",
		hInstance, "tbd", hWndParent, lpDialogFunc, lParamInit);
	if(hWndParent==NULL) hWndParent=dxw.GethWnd();
	RetHWND=(*pCreateDialogParam)(hInstance, lpTemplateName, hWndParent, lpDialogFunc, lParamInit);

	WhndStackPush(RetHWND, (WNDPROC)lpDialogFunc);
	if(!(*pSetWindowLong)(RetHWND, DWL_DLGPROC, (LONG)extDialogWindowProc))
		OutTraceE("SetWindowLong: ERROR err=%d at %d\n", GetLastError(), __LINE__);

	OutTraceD("CreateDialogParam: hwnd=%x\n", RetHWND);
	isWithinDialog=FALSE;
	//if (IsDebug) EnumChildWindows(RetHWND, (WNDENUMPROC)TraceChildWin, (LPARAM)RetHWND);
	return RetHWND;
}

BOOL WINAPI extDDInvalidateRect(HWND hwnd, RECT *lpRect, BOOL bErase)
{
	if(lpRect)
		OutTraceD("InvalidateRect: hwnd=%x rect=(%d,%d)-(%d,%d) erase=%x\n",
		hwnd, lpRect->left, lpRect->top, lpRect->right, lpRect->bottom, bErase);
	else
		OutTraceD("InvalidateRect: hwnd=%x rect=NULL erase=%x\n",
		hwnd, bErase);

	return (*pInvalidateRect)(hwnd, NULL, bErase);
}

BOOL WINAPI extInvalidateRect(HWND hwnd, RECT *lpRect, BOOL bErase)
{
	if(lpRect)
		OutTraceD("InvalidateRect: hwnd=%x rect=(%d,%d)-(%d,%d) erase=%x\n",
		hwnd, lpRect->left, lpRect->top, lpRect->right, lpRect->bottom, bErase);
	else
		OutTraceD("InvalidateRect: hwnd=%x rect=NULL erase=%x\n",
		hwnd, bErase);

	return (*pInvalidateRect)(hwnd, NULL, bErase);
}

/* --------------------------------------------------------------------------- */

// v2.1.75: Hooking for GDI32 CreatePalette, SelectPalette, RealizePalette: 
// maps the GDI palette to the buffered DirectDraw one. This fixes the screen 
// output for "Dementia" (a.k.a. "Armed & Delirious").

HPALETTE WINAPI extGDICreatePalette(CONST LOGPALETTE *plpal)
{
	HPALETTE ret;
	int idx;

	dxw.IsGDIPalette=TRUE;	
	OutTraceD("GDI.CreatePalette: plpal=%x version=%x NumEntries=%x\n", plpal, plpal->palVersion, plpal->palNumEntries);
	ret=(*pGDICreatePalette)(plpal);
	if(IsDebug){
		OutTraceD("PalEntry[%x]= ", plpal->palNumEntries);
		for(idx=0; idx<plpal->palNumEntries; idx++) OutTraceD("(%x)", plpal->palPalEntry[idx]);
		OutTraceD("\n");
	}
	dxw.palVersion=plpal->palVersion;
	dxw.palNumEntries=plpal->palNumEntries;
	if(dxw.palNumEntries>256) dxw.palNumEntries=256;
	for(idx=0; idx<dxw.palNumEntries; idx++) dxw.palPalEntry[idx]=plpal->palPalEntry[idx];
	OutTraceD("GDI.CreatePalette: hPalette=%x\n", ret);
	return ret;
}

HPALETTE WINAPI extSelectPalette(HDC hdc, HPALETTE hpal, BOOL bForceBackground)
{
	HPALETTE ret;

	ret=(*pGDISelectPalette)(hdc, hpal, bForceBackground);
	OutTraceD("GDI.SelectPalette: hdc=%x hpal=%x ForceBackground=%x ret=%x\n", hdc, hpal, bForceBackground, ret);
	return ret;
}

UINT WINAPI extRealizePalette(HDC hdc)
{
	UINT ret;
	extern void mySetPalette(int, int, LPPALETTEENTRY);

	ret=(*pGDIRealizePalette)(hdc);
	OutTraceD("GDI.RealizePalette: hdc=%x ret=%x\n", hdc, ret);

	if(!dxw.IsGDIPalette) return ret;

	// quick & dirty implementation through a nasty global:
	// if the SelectPalette didn't force to the background (arg bForceBackground==FALSE)
	// then don't override the current palette set by the DirectDrawPalette class.
	// should be cleaned up a little....
	// maybe not: now both Diablo & Dementia colors are working...
	if(dxw.dwFlags1 & EMULATESURFACE)
		mySetPalette(0, dxw.palNumEntries, dxw.palPalEntry);
	// DEBUGGING
	if(IsDebug){
		int idx;
		OutTraceD("PaletteEntries[%x]= ", dxw.palNumEntries);
		for(idx=0; idx<dxw.palNumEntries; idx++) OutTraceD("(%x)", PaletteEntries[idx]);
		OutTraceD("\n");
	}
	return ret;
}

// In emulated mode (when color depyth is 8BPP ?) it may happen that the game
// expects to get the requested system palette entries, while the 32BPP screen
// returns 0. "Mission Frce Cyberstorm" is one of these. Returning the same
// value as nEntries, even though lppe is untouched, fixes the problem.

UINT WINAPI extGetSystemPaletteEntries(HDC hdc, UINT iStartIndex, UINT nEntries, LPPALETTEENTRY lppe)
{
	int ret;
	OutTraceD("GetSystemPaletteEntries: hdc=%x start=%d num=%d\n", hdc, iStartIndex, nEntries);
	ret=(*pGDIGetSystemPaletteEntries)(hdc, iStartIndex, nEntries, lppe);
	OutTraceD("GetSystemPaletteEntries: ret=%d\n", ret);
	if((ret == 0) && (dxw.dwFlags1 & EMULATESURFACE)) {
		OutTraceD("GetSystemPaletteEntries: fixing ret=%d\n", nEntries);
		ret = nEntries;
	}
	return ret;
}

HCURSOR WINAPI extSetCursor(HCURSOR hCursor)
{
	HCURSOR ret;

	ret=(*pSetCursor)(hCursor);
	OutTraceD("GDI.SetCursor: Cursor=%x, ret=%x\n", hCursor, ret);
	//MessageBox(0, "SelectPalette", "GDI32.dll", MB_OK | MB_ICONEXCLAMATION);
	return ret;
}

BOOL WINAPI extMoveWindow(HWND hwnd, int X, int Y, int nWidth, int nHeight, BOOL bRepaint)
{
	BOOL ret;
	OutTraceD("MoveWindow: hwnd=%x xy=(%d,%d) size=(%d,%d) repaint=%x indialog=%x\n",
		hwnd, X, Y, nWidth, nHeight, bRepaint, isWithinDialog);

	if((hwnd==dxw.GethWnd()) || (hwnd==dxw.hParentWnd)){
		OutTraceD("MoveWindow: prevent moving main win\n");
		if (nHeight && nWidth){
			OutTraceD("MoveWindow: setting screen size=(%d,%d)\n", nHeight, nWidth);
			dxw.SetScreenSize(nWidth, nHeight);
		}
		return TRUE;
	}

	if((hwnd==0) || (hwnd==(*pGetDesktopWindow)())){
		// v2.1.93: happens in "Emergency Fighters for Life" ...
		OutTraceD("MoveWindow: prevent moving desktop win\n");
		// v2.1.93: moving the desktop seems a way to change its resolution?
		if (nHeight && nWidth){
			OutTraceD("MoveWindow: setting screen size=(%d,%d)\n", nHeight, nWidth);
			dxw.SetScreenSize(nWidth, nHeight);
		}
		return TRUE;
	}

	if (dxw.IsFullScreen()){
		POINT upleft={0,0};
		RECT client;
		BOOL isChild;
		(*pClientToScreen)(dxw.GethWnd(),&upleft);
		(*pGetClientRect)(dxw.GethWnd(),&client);
		if ((*pGetWindowLong)(hwnd, GWL_STYLE) & WS_CHILD){
			isChild=TRUE;
			// child coordinate adjustement
			X = (X * client.right) / dxw.GetScreenWidth();
			Y = (Y * client.bottom) / dxw.GetScreenHeight();
			nWidth = (nWidth * client.right) / dxw.GetScreenWidth();
			nHeight = (nHeight * client.bottom) / dxw.GetScreenHeight();
		}
		else {
			isChild=FALSE;
			// regular win coordinate adjustement
			X = upleft.x + (X * client.right) / dxw.GetScreenWidth();
			Y = upleft.y + (Y * client.bottom) / dxw.GetScreenHeight();
			nWidth = (nWidth * client.right) / dxw.GetScreenWidth();
			nHeight = (nHeight * client.bottom) / dxw.GetScreenHeight();
		}
		OutTraceD("MoveWindow: DEBUG client=(%d,%d) screen=(%d,%d)\n",
			client.right, client.bottom, dxw.GetScreenWidth(), dxw.GetScreenHeight());
		OutTraceD("MoveWindow: hwnd=%x child=%x relocated to xy=(%d,%d) size=(%d,%d)\n",
			hwnd, isChild, X, Y, nWidth, nHeight);
	}
	else{
		if((X==0)&&(Y==0)&&(nWidth==dxw.GetScreenWidth())&&(nHeight==dxw.GetScreenHeight())){
			// evidently, this was supposed to be a fullscreen window....
			RECT screen;
			POINT upleft = {0,0};
			(*pGetClientRect)(dxw.GethWnd(),&screen);
			(*pClientToScreen)(dxw.GethWnd(),&upleft);
			X=upleft.x;
			Y=upleft.y;
			nWidth=screen.right;
			nHeight=screen.bottom;
			OutTraceD("MoveWindow: fixed BIG win pos=(%d,%d) size=(%d,%d)\n", X, Y, nWidth, nHeight);
		}
	}


	ret=(*pMoveWindow)(hwnd, X, Y, nWidth, nHeight, bRepaint);
	if(!ret) OutTraceE("MoveWindow: ERROR err=%d at %d\n", GetLastError(), __LINE__);
	return ret;
} 

LPTOP_LEVEL_EXCEPTION_FILTER WINAPI extSetUnhandledExceptionFilter(LPTOP_LEVEL_EXCEPTION_FILTER lpTopLevelExceptionFilter)
{
	OutTraceD("SetUnhandledExceptionFilter: lpExceptionFilter=%x\n", lpTopLevelExceptionFilter);
	extern LONG WINAPI myUnhandledExceptionFilter(LPEXCEPTION_POINTERS);
	return (*pSetUnhandledExceptionFilter)(myUnhandledExceptionFilter);
}

BOOL WINAPI extSetDeviceGammaRamp(HDC hDC, LPVOID lpRamp)
{
	BOOL ret;
	OutTraceD("SetDeviceGammaRamp: hdc=%x\n", hDC);
	if(dxw.dwFlags2 & DISABLEGAMMARAMP) {
		OutTraceD("SetDeviceGammaRamp: SUPPRESSED\n");
		return TRUE;
	}
	ret=(*pGDISetDeviceGammaRamp)(hDC, lpRamp);
	if(!ret) OutTraceE("SetDeviceGammaRamp: ERROR err=%d\n", GetLastError());
	return ret;
}

BOOL WINAPI extGetDeviceGammaRamp(HDC hDC, LPVOID lpRamp)
{
	BOOL ret;
	OutTraceD("GetDeviceGammaRamp: hdc=%x\n", hDC);
	ret=(*pGDIGetDeviceGammaRamp)(hDC, lpRamp);
	if(!ret) OutTraceE("GetDeviceGammaRamp: ERROR err=%d\n", GetLastError());
	return ret;
}

LRESULT WINAPI extSendMessage(HWND hwnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	LRESULT ret;
	OutTraceW("SendMessage: hwnd=%x WinMsg=[0x%x]%s(%x,%x)\n", 
		hwnd, Msg, ExplainWinMessage(Msg), wParam, lParam);
	if(dxw.dwFlags1 & MODIFYMOUSE){
		switch (Msg){
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
			// revert here the WindowProc mouse correction
			POINT prev, curr;
			RECT rect;
			prev.x = LOWORD(lParam);
			prev.y = HIWORD(lParam);
			(*pGetClientRect)(dxw.GethWnd(), &rect);
			curr.x = (prev.x * rect.right) / dxw.GetScreenWidth();
			curr.y = (prev.y * rect.bottom) / dxw.GetScreenHeight();
			lParam = MAKELPARAM(curr.x, curr.y); 
			OutTraceC("SendMessage: hwnd=%x pos XY=(%d,%d)->(%d,%d)\n", hwnd, prev.x, prev.y, curr.x, curr.y);
			break;
		default:
			break;
		}
	}
	ret=(*pSendMessage)(hwnd, Msg, wParam, lParam);
	OutTraceW("SendMessage: lresult=%x\n", ret); 
	return ret;
}

DWORD WINAPI exttimeGetTime(void)
{
	DWORD ret;
	ret = dxw.GetTickCount();
	if (IsDebug) OutTrace("timeGetTime: time=%x\n", ret);
	return ret;
}

int WINAPI extShowCursor(BOOL bShow)
{
	static int iFakeCounter;
	int ret;

	OutTraceC("ShowCursor: bShow=%x\n", bShow);
	if (bShow){
		if (dxw.dwFlags1 & HIDEHWCURSOR){
			iFakeCounter++;
			OutTraceC("ShowCursor: HIDEHWCURSOR ret=%x\n", iFakeCounter);
			return iFakeCounter;
		}
	}
	else {
		if (dxw.dwFlags2 & SHOWHWCURSOR){
			iFakeCounter--;
			OutTraceC("ShowCursor: SHOWHWCURSOR ret=%x\n", iFakeCounter);
			return iFakeCounter;
		}
	}
	ret=(*pShowCursor)(bShow);
	OutTraceC("ShowCursor: ret=%x\n", ret);
	return ret;
}

