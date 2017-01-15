/*
 * WinG support
 *
 * Copyright 2007 Dmitry Timoshkov
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 */

#define _CRT_SECURE_NO_WARNINGS

#include "dxwnd.h"
#include "dxwcore.hpp"
#include "syslibs.h"
#include "hddraw.h"
#include "dxhook.h"
#include "dxhelper.h"
#include "shareddc.hpp"

#include "stdio.h"

extern int WINAPI extFrameRect(HDC, const RECT *, HBRUSH);

typedef BOOL (WINAPI *WinGBitBlt_Type)(HDC, INT, INT, INT, INT, HDC, INT, INT);
typedef BOOL (WINAPI *WinGStretchBlt_Type)(HDC, INT, INT, INT, INT, HDC, INT, INT, INT, INT);
typedef HBITMAP (WINAPI *WinGCreateBitmap_Type)(HDC, BITMAPINFO *, void **);
typedef HDC (WINAPI *WinGCreateDC_Type)(void);
typedef BOOL (WINAPI *WinGRecommendDIBFormat_Type)(BITMAPINFO *);
typedef void * (WINAPI *WinGGetDIBPointer_Type)(HBITMAP, BITMAPINFO *);
typedef UINT (WINAPI *WinGSetDIBColorTable_Type)(HDC, UINT, UINT, RGBQUAD *);
typedef UINT (WINAPI *WinGGetDIBColorTable_Type)(HDC, UINT, UINT, RGBQUAD *);
typedef HPALETTE (WINAPI *WinGCreateHalfTonePalette_Type)(void);

WinGBitBlt_Type pWinGBitBlt;
WinGStretchBlt_Type pWinGStretchBlt;
WinGCreateBitmap_Type pWinGCreateBitmap;
WinGCreateDC_Type pWinGCreateDC;
WinGRecommendDIBFormat_Type pWinGRecommendDIBFormat;
WinGGetDIBPointer_Type pWinGGetDIBPointer;
WinGSetDIBColorTable_Type pWinGSetDIBColorTable;
WinGGetDIBColorTable_Type pWinGGetDIBColorTable;
WinGCreateHalfTonePalette_Type pWinGCreateHalfTonePalette;

BOOL WINAPI extWinGBitBlt(HDC, INT, INT, INT, INT, HDC, INT, INT);
BOOL WINAPI extWinGStretchBlt(HDC, INT, INT, INT, INT, HDC, INT, INT, INT, INT);
HBITMAP WINAPI extWinGCreateBitmap(HDC, BITMAPINFO *, void **);
HDC WINAPI extWinGCreateDC(void);
BOOL WINAPI extWinGRecommendDIBFormat(BITMAPINFO *);
void * WINAPI extWinGGetDIBPointer(HBITMAP, BITMAPINFO *);
UINT WINAPI extWinGSetDIBColorTable(HDC, UINT, UINT, RGBQUAD *);
UINT WINAPI extWinGGetDIBColorTable(HDC, UINT, UINT, RGBQUAD *);
HPALETTE WINAPI extWinGCreateHalfTonePalette(void);

static HookEntryEx_Type Hooks[]={
	{HOOK_IAT_CANDIDATE, 0, "WinGBitBlt", (FARPROC)NULL, (FARPROC *)&pWinGBitBlt, (FARPROC)extWinGBitBlt},
	{HOOK_IAT_CANDIDATE, 0, "WinGStretchBlt", (FARPROC)NULL, (FARPROC *)&pWinGStretchBlt, (FARPROC)extWinGStretchBlt},
	{HOOK_IAT_CANDIDATE, 0, "WinGCreateBitmap", (FARPROC)NULL, (FARPROC *)&pWinGCreateBitmap, (FARPROC)extWinGCreateBitmap},
	{HOOK_IAT_CANDIDATE, 0, "WinGCreateDC", (FARPROC)NULL, (FARPROC *)&pWinGCreateDC, (FARPROC)extWinGCreateDC},
	{HOOK_IAT_CANDIDATE, 0, "WinGRecommendDIBFormat", (FARPROC)NULL, (FARPROC *)&pWinGRecommendDIBFormat, (FARPROC)extWinGRecommendDIBFormat},
	{HOOK_IAT_CANDIDATE, 0, "WinGGetDIBPointer", (FARPROC)NULL, (FARPROC *)&pWinGGetDIBPointer, (FARPROC)extWinGGetDIBPointer},
	{HOOK_IAT_CANDIDATE, 0, "WinGSetDIBColorTable", (FARPROC)NULL, (FARPROC *)&pWinGSetDIBColorTable, (FARPROC)extWinGSetDIBColorTable},
	{HOOK_IAT_CANDIDATE, 0, "WinGGetDIBColorTable", (FARPROC)NULL, (FARPROC *)&pWinGGetDIBColorTable, (FARPROC)extWinGGetDIBColorTable},
	{HOOK_IAT_CANDIDATE, 0, "WinGCreateHalfTonePalette", (FARPROC)NULL, (FARPROC *)&pWinGCreateHalfTonePalette, (FARPROC)extWinGCreateHalfTonePalette},
	{HOOK_IAT_CANDIDATE, 0, 0, NULL, 0, 0} // terminator
}; 

static char *libname = "wing32.dll";

void HookWinG32Init()
{
	//HookLibInitEx(Hooks);
}

void HookWinG32(HMODULE module)
{
	if (dxw.GDIEmulationMode != GDIMODE_NONE) HookLibraryEx(module, Hooks, libname);
}

FARPROC Remap_WinG32_ProcAddress(LPCSTR proc, HMODULE hModule)
{
	FARPROC addr;
	if (dxw.GDIEmulationMode != GDIMODE_NONE) if(addr=RemapLibraryEx(proc, hModule, Hooks)) return addr;
	return NULL;
}

// === wrappers ===

HDC WINAPI extWinGCreateDC(void)
{
	OutTraceWG("WinGCreateDC\n");
    return extGDICreateCompatibleDC(0);
}

HBITMAP WINAPI extWinGCreateBitmap(HDC hdc, BITMAPINFO *bmi, void **bits)
{
	HBITMAP ret;
	OutTraceWG("WinGCreateBitmap: hdc=%x bmi=%x\n", hdc, bmi);
    ret = extCreateDIBSection(hdc, bmi, 0, bits, 0, 0);
	OutTraceWG("WinGCreateBitmap: ret=%x\n", ret);
	return ret;
}

BOOL WINAPI extWinGBitBlt(HDC hdcDst, INT xDst, INT yDst, INT width, INT height, HDC hdcSrc, INT xSrc, INT ySrc )
{
	BOOL ret;
	OutTraceWG("WinGBitBlt: hdcdest=%x pos=(%d,%d) size=(%dx%d) hdcsrc=%x pos=(%d,%d)\n", 
		hdcDst, xDst, yDst, width, height, hdcSrc, xSrc, ySrc);
    ret = extGDIBitBlt(hdcDst, xDst, yDst, width, height, hdcSrc, xSrc, ySrc, SRCCOPY);

	if(dxw.dwFlags8 & MARKWING32) dxw.Mark(hdcDst, TRUE, RGB(0, 255, 0), xDst, yDst, width, height);
	return ret;
}

BOOL WINAPI extWinGStretchBlt(HDC hdcDst, INT xDst, INT yDst, INT widthDst, INT heightDst,
                            HDC hdcSrc, INT xSrc, INT ySrc, INT widthSrc, INT heightSrc )
{
    INT old_blt_mode;
    BOOL ret;
	
	OutTraceWG("WinGStretchBlt: hdcdest=%x pos=(%d,%d) size=(%dx%d) hdcsrc=%x pos=(%d,%d) size=(%dx%d)\n",
		hdcDst, xDst, yDst, widthDst, heightDst, hdcSrc, xSrc, ySrc, widthSrc, heightSrc);
	// wing hdc is always a screen DC.
    old_blt_mode = SetStretchBltMode( hdcDst, COLORONCOLOR );
    ret = extGDIStretchBlt( hdcDst, xDst, yDst, widthDst, heightDst, 
		hdcSrc, xSrc, ySrc, widthSrc, heightSrc, SRCCOPY );
    SetStretchBltMode( hdcDst, old_blt_mode );

	if(dxw.dwFlags8 & MARKWING32) dxw.Mark(hdcDst, TRUE, RGB(255, 0, 0), xDst, yDst, widthDst, heightDst);
    return ret;
}

BOOL WINAPI extWinGRecommendDIBFormat(BITMAPINFO *bmi)
{
	OutTraceWG("WinGRecommendDIBFormat\n");
    if (!bmi) return FALSE;

    bmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi->bmiHeader.biWidth = 320;
    bmi->bmiHeader.biHeight = -1;
    bmi->bmiHeader.biPlanes = 1;
    bmi->bmiHeader.biBitCount = 8;
    bmi->bmiHeader.biCompression = BI_RGB;
    bmi->bmiHeader.biSizeImage = 0;
    bmi->bmiHeader.biXPelsPerMeter = 0;
    bmi->bmiHeader.biYPelsPerMeter = 0;
    bmi->bmiHeader.biClrUsed = 0;
    bmi->bmiHeader.biClrImportant = 0;

    return TRUE;
}

void * WINAPI extWinGGetDIBPointer(HBITMAP hbmp, BITMAPINFO *bmi)
{
    DIBSECTION ds;

	OutTraceWG("WinGGetDIBPointer\n");
	if (GetObjectW( hbmp, sizeof(ds), &ds ) == sizeof(ds)) {
        memcpy( &bmi->bmiHeader, &ds.dsBmih, sizeof(*bmi) );
        return ds.dsBm.bmBits;
    }
    return NULL;
}

UINT WINAPI extWinGSetDIBColorTable(HDC hdc, UINT start, UINT end, RGBQUAD *colors)
{
	OutTraceWG("WinGSetDIBColorTable: hdc=%x start=%d end=%d\n", hdc, start, end);
	(*pInvalidateRect)(dxw.GethWnd(), NULL, FALSE);
	return SetDIBColorTable( hdc, start, end, colors );
}

UINT WINAPI extWinGGetDIBColorTable(HDC hdc, UINT start, UINT end, RGBQUAD *colors)
{
	OutTraceWG("WinGGetDIBColorTable: hdc=%x start=%d end=%d\n", hdc, start, end);
	(*pInvalidateRect)(dxw.GethWnd(), NULL, FALSE);
    return GetDIBColorTable( hdc, start, end, colors );
}

HPALETTE WINAPI extWinGCreateHalfTonePalette(void)
{
    HDC hdc;
    HPALETTE hpal;

	OutTraceWG("WinGCreateHalfTonePalette: void\n");
    hdc = GetDC(0);
    hpal = CreateHalftonePalette(hdc);
    ReleaseDC(0, hdc);
	(*pInvalidateRect)(dxw.GethWnd(), NULL, FALSE);

    return hpal;
}

HBRUSH WINAPI extWinGCreateHalfToneBrush(HDC hdc, COLORREF color, INT type)
{
	OutTraceWG("WinGCreateHalfToneBrush: hdc=%x color=%x type=%d\n", hdc, color, type);
	return CreateSolidBrush(color);
}
