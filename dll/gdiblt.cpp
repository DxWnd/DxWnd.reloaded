#define _CRT_SECURE_NO_WARNINGS
#define INITGUID
//#define FULLHEXDUMP

#include <windows.h>
#include <ddraw.h>
#include "dxwnd.h"
#include "dxhook.h"
#include "ddrawi.h"
#include "dxwcore.hpp"
#include "stdio.h" 
#include "hddraw.h"
#include "ddproxy.h"
#include "dxhelper.h"
#include "syslibs.h"

extern GetDC_Type pGetDC;
extern ReleaseDC_Type pReleaseDC;
extern Unlock1_Type pUnlock1;

//#define HDC_CACHE_OPTIMIZED

#ifdef HDC_CACHE_OPTIMIZED
void BlitToWindow(HWND w, LPDIRECTDRAWSURFACE s)
{
	static HDC thdc = NULL;
	HDC shdc;
	static HWND LastWindow = NULL;
	RECT client;
	HRESULT res;
	BOOL ret;

	if(!s) return; // for surface color fill 
	ret=(*pGetClientRect)(w, &client);
	if(!ret) OutTrace("GetClientRect error=%d\n", GetLastError());
		res=(*pGetDC)(s, &shdc);
		if(res) OutTrace("ddraw GetDC error lpdds=%x res=%x(%s)\n", s, res, ExplainDDError(res));
	if(w != LastWindow){
		if(LastWindow){
		ret=(*pGDIReleaseDC)(LastWindow, thdc);
		if(!ret) OutTrace("GDI ReleaseDC error=%d\n", GetLastError());
		}
		LastWindow = w;
		thdc=(*pGDIGetDC)(w);
		if(!thdc) OutTrace("GDI GetDC error=%d\n", GetLastError());
	}
	if(dxw.dwFlags5 & CENTERTOWIN){
		int x, y;
		x = (client.right - dxw.GetScreenWidth()) >> 1; // right-shift 1 bit means divide by 2!
		y = (client.bottom - dxw.GetScreenHeight()) >> 1;
		ret=(*pGDIBitBlt)(thdc, x, y, dxw.GetScreenWidth(), dxw.GetScreenHeight(), shdc, 0, 0, SRCCOPY);
		if(!ret) OutTrace("BitBlt error=%d\n", GetLastError());
	}
	else{
		if(dxw.dwFlags5 & BILINEARFILTER) {
			ret=SetStretchBltMode(thdc, HALFTONE);
			if((!ret) || (ret==ERROR_INVALID_PARAMETER)) OutTrace("GDI SetStretchBltMode error=%d\n", GetLastError());
		}
		ret=(*pGDIStretchBlt)(thdc, 0, 0, client.right, client.bottom, shdc, 0, 0, dxw.GetScreenWidth(), dxw.GetScreenHeight(), SRCCOPY);
		if(!ret) OutTrace("GDI StretchBlt error=%d\n", GetLastError());
	}
	dxw.ShowOverlay(thdc);
	res=(*pReleaseDC)(s, shdc);
	if(res) OutTrace("ddraw ReleaseDC error lpdds=%x res=%x(%s)\n", s, res, ExplainDDError(res));
}
#else
void BlitToWindow(HWND w, LPDIRECTDRAWSURFACE s)
{
	HDC shdc, thdc;
	RECT client;
	HRESULT res;
	BOOL ret;

	if(!s) return; // for surface color fill 
	ret=(*pGetClientRect)(w, &client);
	if(!ret) OutTrace("GetClientRect error=%d\n", GetLastError());
	res=(*pGetDC)(s, &shdc);
	if(res) OutTrace("ddraw GetDC error lpdds=%x res=%x(%s)\n", s, res, ExplainDDError(res));
	thdc=(*pGDIGetDC)(w);
	if(!thdc) OutTrace("GDI GetDC error=%d\n", GetLastError());
	if(dxw.dwFlags5 & CENTERTOWIN){
		int x, y;
		x = (client.right - dxw.GetScreenWidth()) >> 1; // right-shift 1 bit means divide by 2!
		y = (client.bottom - dxw.GetScreenHeight()) >> 1;
		ret=(*pGDIBitBlt)(thdc, x, y, dxw.GetScreenWidth(), dxw.GetScreenHeight(), shdc, 0, 0, SRCCOPY);
		if(!ret) OutTrace("BitBlt error=%d\n", GetLastError());
	}
	else{
		if(dxw.dwFlags5 & BILINEARFILTER) {
			ret=SetStretchBltMode(thdc, HALFTONE);
			if((!ret) || (ret==ERROR_INVALID_PARAMETER)) OutTrace("GDI SetStretchBltMode error=%d\n", GetLastError());
		}
		ret=(*pGDIStretchBlt)(thdc, 0, 0, client.right, client.bottom, shdc, 0, 0, dxw.GetScreenWidth(), dxw.GetScreenHeight(), SRCCOPY);
		if(!ret) OutTrace("GDI StretchBlt error=%d\n", GetLastError());
	}
	dxw.ShowOverlay(thdc);
	res=(*pReleaseDC)(s, shdc);
	if(res) OutTrace("ddraw ReleaseDC error lpdds=%x res=%x(%s)\n", s, res, ExplainDDError(res));
	ret=(*pGDIReleaseDC)(w, thdc);
	if(!ret) OutTrace("GDI ReleaseDC error=%d\n", GetLastError());
}
#endif
