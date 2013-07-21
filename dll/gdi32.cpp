#include "dxwnd.h"
#include "dxwcore.hpp"
#include "syslibs.h"
#include "hddraw.h"
#include "dxhook.h"
#include "dxhelper.h"

static HookEntry_Type Hooks[]={
	{"GetDeviceCaps", (FARPROC)GetDeviceCaps, (FARPROC *)&pGDIGetDeviceCaps, (FARPROC)extGetDeviceCaps},
	{"TextOutA", (FARPROC)TextOutA, (FARPROC *)&pGDITextOutA, (FARPROC)extTextOutA},
	{"TabbedTextOutA", (FARPROC)TabbedTextOutA, (FARPROC *)&pTabbedTextOutA, (FARPROC)extTabbedTextOutA},
	{"ScaleWindowExtEx", (FARPROC)ScaleWindowExtEx, (FARPROC *)&pGDIScaleWindowExtEx, (FARPROC)extScaleWindowExtEx},
	{"Rectangle", (FARPROC)Rectangle, (FARPROC *)&pGDIRectangle, (FARPROC)extRectangle},
	{"SaveDC", (FARPROC)SaveDC, (FARPROC *)&pGDISaveDC, (FARPROC)extGDISaveDC},
	{"RestoreDC", (FARPROC)RestoreDC, (FARPROC *)&pGDIRestoreDC, (FARPROC)extGDIRestoreDC},
	{"CreatePalette", (FARPROC)CreatePalette, (FARPROC *)&pGDICreatePalette, (FARPROC)extGDICreatePalette},
	{"SelectPalette", (FARPROC)SelectPalette, (FARPROC *)&pGDISelectPalette, (FARPROC)extSelectPalette},
	{"RealizePalette", (FARPROC)RealizePalette, (FARPROC *)&pGDIRealizePalette, (FARPROC)extRealizePalette},
	{"GetSystemPaletteEntries", (FARPROC)GetSystemPaletteEntries, (FARPROC *)&pGDIGetSystemPaletteEntries, (FARPROC)extGetSystemPaletteEntries},
	{0, NULL, 0, 0} // terminator
};

static HookEntry_Type DDHooks[]={
	{"CreateCompatibleDC", (FARPROC)CreateCompatibleDC, (FARPROC *)&pGDICreateCompatibleDC, (FARPROC)extDDCreateCompatibleDC},
	{"DeleteDC", (FARPROC)DeleteDC, (FARPROC *)&pGDIDeleteDC, (FARPROC)extDDDeleteDC},
	{"CreateDCA", (FARPROC)CreateDCA, (FARPROC *)&pGDICreateDC, (FARPROC)extDDCreateDC},
	{"BitBlt", (FARPROC)BitBlt, (FARPROC *)&pGDIBitBlt, (FARPROC)extDDBitBlt},
	{"StretchBlt", (FARPROC)StretchBlt, (FARPROC *)&pGDIStretchBlt, (FARPROC)extDDStretchBlt},
	// {"PatBlt", (FARPROC)PatBlt, (FARPROC *)&pGDIPatBlt, (FARPROC)extDDPatBlt}, // missing one ...
	{0, NULL, 0, 0} // terminator
};

static HookEntry_Type GDIHooks[]={
	{"CreateCompatibleDC", (FARPROC)CreateCompatibleDC, (FARPROC *)&pGDICreateCompatibleDC, (FARPROC)extGDICreateCompatibleDC},
	{"DeleteDC", (FARPROC)DeleteDC, (FARPROC *)&pGDIDeleteDC, (FARPROC)extGDIDeleteDC},
	{"CreateDCA", (FARPROC)CreateDCA, (FARPROC *)&pGDICreateDC, (FARPROC)extGDICreateDC},
	{"BitBlt", (FARPROC)BitBlt, (FARPROC *)&pGDIBitBlt, (FARPROC)extGDIBitBlt},
	{"StretchBlt", (FARPROC)StretchBlt, (FARPROC *)&pGDIStretchBlt, (FARPROC)extGDIStretchBlt},
	{"PatBlt", (FARPROC)PatBlt, (FARPROC *)&pGDIPatBlt, (FARPROC)extGDIPatBlt},
	{0, NULL, 0, 0} // terminator
};

static HookEntry_Type EmuHooks[]={
	{"SetTextColor", (FARPROC)SetTextColor, (FARPROC *)&pGDISetTextColor, (FARPROC)extSetTextColor},
	{"SetBkColor", (FARPROC)SetBkColor, (FARPROC *)&pGDISetBkColor, (FARPROC)extSetBkColor},
	{"CreateFont", (FARPROC)CreateFont, (FARPROC *)&pGDICreateFont, (FARPROC)extCreateFont},
	{"CreateFontIndirectA", (FARPROC)CreateFontIndirectA, (FARPROC *)&pGDICreateFontIndirect, (FARPROC)extCreateFontIndirect},
	{0, NULL, 0, 0} // terminator
};

static HookEntry_Type GammaHooks[]={
	{"SetDeviceGammaRamp", (FARPROC)SetDeviceGammaRamp, (FARPROC *)&pGDISetDeviceGammaRamp, (FARPROC)extSetDeviceGammaRamp},
	{"GetDeviceGammaRamp", (FARPROC)GetDeviceGammaRamp, (FARPROC *)&pGDIGetDeviceGammaRamp, (FARPROC)extGetDeviceGammaRamp},
	{0, NULL, 0, 0} // terminator
};

extern HRESULT WINAPI extDirectDrawCreate(GUID FAR *, LPDIRECTDRAW FAR *, IUnknown FAR *);
extern HRESULT WINAPI extDirectDrawCreateEx(GUID FAR *, LPDIRECTDRAW FAR *, REFIID, IUnknown FAR *);

static char *libname = "gdi32.dll";

void HookGDI32(HMODULE module)
{
	HookLibrary(module, Hooks, libname);
	if(dxw.dwFlags1 & MAPGDITOPRIMARY)
		HookLibrary(module, DDHooks, libname);
	else
		HookLibrary(module, GDIHooks, libname);
	if ((dxw.dwFlags1 & EMULATESURFACE) && (dxw.dwFlags1 & HANDLEDC))
		HookLibrary(module, EmuHooks, libname);
	if(dxw.dwFlags2 & DISABLEGAMMARAMP)
		HookLibrary(module, GammaHooks, libname);
}

void HookGDI32Init()
{
	HookLibInit(Hooks);
	HookLibInit(DDHooks);
	HookLibInit(EmuHooks);
	HookLibInit(GammaHooks);
}

FARPROC Remap_GDI32_ProcAddress(LPCSTR proc, HMODULE hModule)
{
	FARPROC addr;
	if (addr=RemapLibrary(proc, hModule, Hooks)) return addr;
	if(dxw.dwFlags1 & MAPGDITOPRIMARY)
		if(addr=RemapLibrary(proc, hModule, DDHooks)) return addr;
	else
		if(addr=RemapLibrary(proc, hModule, GDIHooks)) return addr;
	if ((dxw.dwFlags1 & EMULATESURFACE) && (dxw.dwFlags1 & HANDLEDC))
		if(addr=RemapLibrary(proc, hModule, EmuHooks)) return addr;
	if(dxw.dwFlags2 & DISABLEGAMMARAMP)
		if(addr=RemapLibrary(proc, hModule, GammaHooks)) return addr;
	return NULL;
}

//--------------------------------------------------------------------------------------------
//
// extern and common functions
//
//--------------------------------------------------------------------------------------------

extern DEVMODE *pSetDevMode;
extern DWORD PaletteEntries[256];
extern Unlock4_Type pUnlockMethod(LPDIRECTDRAWSURFACE);
extern HRESULT WINAPI sBlt(char *, LPDIRECTDRAWSURFACE, LPRECT, LPDIRECTDRAWSURFACE, LPRECT, DWORD, LPDDBLTFX, BOOL);

extern GetDC_Type pGetDC;
extern ReleaseDC_Type pReleaseDC;

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

//--------------------------------------------------------------------------------------------
//
// API hookers
//
//--------------------------------------------------------------------------------------------

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

LONG WINAPI extTabbedTextOutA(HDC hDC, int X, int Y, LPCTSTR lpString, int nCount, int nTabPositions, const LPINT lpnTabStopPositions, int nTabOrigin)
{
	BOOL res;
	OutTraceD("TabbedTextOut: hdc=%x xy=(%d,%d) nCount=%d nTP=%d nTOS=%d str=(%d)\"%s\"\n", 
		hDC, X, Y, nCount, nTabPositions, nTabOrigin, lpString);
	if (dxw.dwFlags1 & FIXTEXTOUT) {
		POINT anchor;
		anchor.x=X;
		anchor.y=Y;
		(*pClientToScreen)(dxw.GethWnd(), &anchor);
		X=anchor.x;
		Y=anchor.y;
	}
	res=(*pTabbedTextOutA)(hDC, X, Y, lpString, nCount, nTabPositions, lpnTabStopPositions, nTabOrigin);
	return res;
}

BOOL WINAPI extScaleWindowExtEx(HDC hdc, int Xnum, int Xdenom, int Ynum, int Ydenom, LPSIZE lpSize)
{
	OutTraceD("ScaleWindowExtEx: hdc=%x num=(%d,%d) denom=(%d,%d) lpSize=%d\n",
		hdc, Xnum, Ynum, Xdenom, Ydenom, lpSize);

	if ((dxw.dwFlags1 & LOCKWINPOS) && dxw.IsFullScreen()) return 1;

	return (*pGDIScaleWindowExtEx)(hdc, Xnum, Xdenom, Ynum, Ydenom, lpSize);
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

int WINAPI extGDISaveDC(HDC hdc)
{
	int ret;

	ret=(*pGDISaveDC)(hdc);
	OutTraceD("GDI.SaveDC: hdc=%x ret=%x\n", hdc, ret);
	//AutoRefreshThread=CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)AutoRefresh, (LPVOID)hdc, 0, &dwThrdId);
	return ret;
}

BOOL WINAPI extGDIRestoreDC(HDC hdc, int nSavedDC)
{
	BOOL ret;

	ret=(*pGDIRestoreDC)(hdc, nSavedDC);
	OutTraceD("GDI.RestoreDC: hdc=%x nSavedDC=%x ret=%x\n", hdc, nSavedDC, ret);
	//TerminateThread(AutoRefreshThread, 0);
	return ret;
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

/* -------------------------------------------------------------------- */
// directdraw supported GDI calls
/* -------------------------------------------------------------------- */

// PrimHDC: DC handle of the selected DirectDraw primary surface. NULL when invalid.
extern HDC PrimHDC;

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

	if(dxw.IsDesktop(hwnd)) hwnd=dxw.GethWnd();

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

