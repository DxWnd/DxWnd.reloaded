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
#include "dxhelper.h"
#include "syslibs.h"

#define MAXBACKBUFFERS 4

extern void SetPixFmt(LPDDSURFACEDESC2);
extern void GetPixFmt(LPDDSURFACEDESC2);
extern CHAR *LogSurfaceAttributes(LPDDSURFACEDESC, char *, int);
extern void DumpPixFmt(LPDDSURFACEDESC2);
extern void DescribeSurface(LPDIRECTDRAWSURFACE, int, char *, int);
extern void HookDDSurface(LPDIRECTDRAWSURFACE *, int,  BOOL);
extern void RegisterPixelFormat(int, LPDIRECTDRAWSURFACE);
extern void SetVSyncDelays(int, LPDIRECTDRAW);
extern char *DumpPixelFormat(LPDDSURFACEDESC2);
extern void FixSurfaceCaps(LPDDSURFACEDESC2, int);
extern SetPalette_Type pSetPaletteMethod(int);
extern ReleaseS_Type pReleaseSMethod(int);
extern CreatePalette_Type pCreatePaletteMethod(int);
extern GetGDISurface_Type pGetGDISurfaceMethod(int);

extern int iBakBufferVersion;
extern LPDIRECTDRAWSURFACE lpDDSEmu_Prim;
extern LPDIRECTDRAWSURFACE lpDDSEmu_Back;
extern LPDIRECTDRAWSURFACE lpDDZBuffer;
extern DDSURFACEDESC2 DDSD_Prim;
extern LPDIRECTDRAWPALETTE lpDDP;
extern LPDIRECTDRAWCLIPPER lpddC;
extern LPDIRECTDRAW lpPrimaryDD;
extern int iDDPExtraRefCounter;
extern DWORD dwBackBufferCaps;
extern BOOL bFlippedDC;
extern PALETTEENTRY DefaultSystemPalette[];
extern GetGDISurface_Type pGetGDISurface1, pGetGDISurface2, pGetGDISurface3, pGetGDISurface4, pGetGDISurface7;
extern GetAttachedSurface_Type pGetAttachedSurface1, pGetAttachedSurface2, pGetAttachedSurface3, pGetAttachedSurface4, pGetAttachedSurface7;
extern CreateSurface1_Type pCreateSurface1, pCreateSurface2, pCreateSurface3, pCreateSurface4, pCreateSurface7;
extern ReleaseS_Type pReleaseS1, pReleaseS2, pReleaseS3, pReleaseS4, pReleaseS7;
extern GetPixelFormat_Type pGetPixelFormat1, pGetPixelFormat2, pGetPixelFormat3, pGetPixelFormat4, pGetPixelFormat7;

void InitDSScreenParameters(int dxversion, LPDIRECTDRAWSURFACE lpdds)
{
	HRESULT res;
	DDPIXELFORMAT p;
	DDSURFACEDESC2 ddsd;
	GetPixelFormat_Type pGetPixelFormat;
	switch(dxversion){
		case 1: pGetPixelFormat=pGetPixelFormat1; break;
		case 2: pGetPixelFormat=pGetPixelFormat2; break;
		case 3: pGetPixelFormat=pGetPixelFormat3; break;
		case 4: pGetPixelFormat=pGetPixelFormat4; break;
		case 7: pGetPixelFormat=pGetPixelFormat7; break;
	}
	//OutTrace("dxversion=%d pGetPixelFormat=%x\n", dxversion, pGetPixelFormat);
	p.dwSize=sizeof(DDPIXELFORMAT);
	if(res=(*pGetPixelFormat)(lpdds, &p)){
		OutTraceE("GetPixelFormat: ERROR res=%x(%s) at %d\n", res, ExplainDDError(res), __LINE__);
		return;
	}

	ddsd.ddpfPixelFormat = p;
	OutTraceDW("InitDSScreenParameters: version=%d Actual %s\n", dxversion, DumpPixelFormat(&ddsd));
	dxw.ActualPixelFormat = p;
	SetBltTransformations(dxversion);
	return;
}

static void ClearSurfaceDesc(void *ddsd, int dxversion)
{
	int size;
	size = (dxversion < 4) ? sizeof(DDSURFACEDESC) : sizeof(DDSURFACEDESC2);
	memset(ddsd, 0, size); // Clean all
	((LPDDSURFACEDESC)ddsd)->dwSize = size;
}

static void BuildRealSurfaces(LPDIRECTDRAW lpdd, CreateSurface_Type pCreateSurface, int dxversion)
{
	HRESULT res;
	DDSURFACEDESC2 ddsd;

	OutTraceDW("DEBUG: BuildRealSurfaces: lpdd=%x pCreateSurface=%x version=%d\n", lpdd, pCreateSurface, dxversion);
	if(lpDDSEmu_Prim==NULL){
		ClearSurfaceDesc((void *)&ddsd, dxversion);
		ddsd.dwFlags = DDSD_CAPS; 
		// try DDSCAPS_SYSTEMMEMORY first, then suppress it if not supported
		// no, DDSCAPS_SYSTEMMEMORY cause screen flickering while moving the window (and other troubles?)
		ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;
		OutTraceDW("CreateSurface: %s\n", LogSurfaceAttributes((LPDDSURFACEDESC)&ddsd, "[EmuPrim]", __LINE__));
		res=(*pCreateSurface)(lpdd, &ddsd, &lpDDSEmu_Prim, 0);
		if(res==DDERR_PRIMARYSURFACEALREADYEXISTS){
			OutTraceDW("CreateSurface: ASSERT DDSEmu_Prim already exists\n");
			if(dxw.Windowize){
				// in Windowize mode, the desktop properties are untouched, then the current primary surface can be recycled
				res=(*pGetGDISurfaceMethod(dxversion))(lpdd, &lpDDSEmu_Prim); 
			}
			else {
				// in non-Windowized mode, the primary surface must be released and rebuilt with the proper properties
				res=(*pGetGDISurfaceMethod(dxversion))(lpdd, &lpDDSEmu_Prim); 
				if (lpDDSEmu_Prim) while((*pReleaseSMethod(dxversion))(lpDDSEmu_Prim));
				res=(*pCreateSurface)(lpdd, &ddsd, &lpDDSEmu_Prim, 0);
			}
		}
		if(res){
			OutTraceE("CreateSurface: ERROR on DDSEmu_Prim res=%x(%s) at %d\n", res, ExplainDDError(res), __LINE__);
			if(res==DDERR_INVALIDPIXELFORMAT) DumpPixFmt(&ddsd);
			return;
		}
		OutTraceDW("CreateSurface: created new DDSEmu_Prim=%x\n",lpDDSEmu_Prim);
		if(IsDebug) DescribeSurface(lpDDSEmu_Prim, dxversion, "DDSEmu_Prim", __LINE__);
		InitDSScreenParameters(dxversion, lpDDSEmu_Prim);
		dxwss.PopSurface(lpDDSEmu_Prim);

		if (dxw.dwFlags3 & FORCECLIPPER){
			OutTraceDW("CreateSurface: FORCE SetClipper on primary hwnd=%x lpdds=%x\n", dxw.GethWnd(), lpDDSEmu_Prim);
			res=lpdd->CreateClipper(0, &lpddC, NULL);
			if (res) OutTraceE("CreateSurface: CreateClipper ERROR res=%x(%s)\n", res, ExplainDDError(res));
			res=lpddC->SetHWnd(0, dxw.GethWnd());
			if (res) OutTraceE("CreateSurface: SetHWnd ERROR res=%x(%s)\n", res, ExplainDDError(res));
			res=lpDDSEmu_Prim->SetClipper(lpddC);
			if (res) OutTraceE("CreateSurface: SetClipper ERROR res=%x(%s)\n", res, ExplainDDError(res));
		}
		// can't hook lpDDSEmu_Prim as generic, since the Flip method is unimplemented for a PRIMARY surface!
		// better avoid it or hook just useful methods.
		//if (dxw.dwTFlags & OUTPROXYTRACE) HookDDSurfaceGeneric(&lpDDSEmu_Prim, dxw.dwDDVersion);
	}

	if(lpDDSEmu_Back==NULL){
		ClearSurfaceDesc((void *)&ddsd, dxversion);
		ddsd.dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT;
		ddsd.ddsCaps.dwCaps = dwBackBufferCaps;
		ddsd.dwWidth = dxw.GetScreenWidth();
		ddsd.dwHeight = dxw.GetScreenHeight();
		if(dxw.dwFlags4 & BILINEAR2XFILTER){
			// double backbuffer size
			ddsd.dwWidth = dxw.GetScreenWidth() << 1;
			ddsd.dwHeight = dxw.GetScreenHeight() << 1;
		}

		OutTraceDW("CreateSurface: %s\n", LogSurfaceAttributes((LPDDSURFACEDESC)&ddsd, "[EmuBack]", __LINE__));
		res=(*pCreateSurface)(lpdd, &ddsd, &lpDDSEmu_Back, 0);
		if(res) {
			ddsd.ddsCaps.dwCaps &= ~DDSCAPS_SYSTEMMEMORY;
			OutTraceDW("CreateSurface: %s\n", LogSurfaceAttributes((LPDDSURFACEDESC)&ddsd, "[EmuBack]", __LINE__));
			res=(*pCreateSurface)(lpdd, &ddsd, &lpDDSEmu_Back, 0);
		}
		if(res){
			OutTraceE("CreateSurface: CreateSurface ERROR on DDSEmuBack : res=%x(%s) at %d\n", res, ExplainDDError(res), __LINE__);
			if(res==DDERR_INVALIDPIXELFORMAT) DumpPixFmt(&ddsd);
			return;
		}
		OutTraceDW("CreateSurface: created new DDSEmu_Back=%x\n", lpDDSEmu_Back);
		if(IsDebug) DescribeSurface(lpDDSEmu_Back, dxversion, "DDSEmu_Back", __LINE__);
		dxwss.PopSurface(lpDDSEmu_Back);
		//if (dxw.dwTFlags & OUTPROXYTRACE) HookDDSurfaceGeneric(&lpDDSEmu_Back, dxversion);
	}
}

void RestoreDDrawSurfaces()
{
	// if it's a ddraw game ....
	if(lpPrimaryDD){
		OutTraceDW("RestoreDDrawSurfaces: rebuilding surfaces for dd session %x\n", lpPrimaryDD);
		ReleaseS_Type pReleaseS;
		CreateSurface_Type pCreateSurface;
		switch(iBakBufferVersion){
			case 1: pCreateSurface=(CreateSurface_Type)pCreateSurface1; pReleaseS=pReleaseS1; break;
			case 2: pCreateSurface=(CreateSurface_Type)pCreateSurface2; pReleaseS=pReleaseS2; break;
			case 3: pCreateSurface=(CreateSurface_Type)pCreateSurface3; pReleaseS=pReleaseS3; break;
			case 4: pCreateSurface=(CreateSurface_Type)pCreateSurface4; pReleaseS=pReleaseS4; break;
			case 7: pCreateSurface=(CreateSurface_Type)pCreateSurface7; pReleaseS=pReleaseS7; break;
		}
		if(lpDDSEmu_Back) while((*pReleaseS)(lpDDSEmu_Back)); lpDDSEmu_Back=0;
		if(lpDDSEmu_Prim) while((*pReleaseS)(lpDDSEmu_Prim)); lpDDSEmu_Prim=0;
		BuildRealSurfaces(lpPrimaryDD, pCreateSurface, iBakBufferVersion);
	}
}

static HRESULT BuildPrimaryEmu(LPDIRECTDRAW lpdd, CreateSurface_Type pCreateSurface, LPDDSURFACEDESC2 lpddsd, int dxversion, LPDIRECTDRAWSURFACE *lplpdds, void *pu)
{
	DDSURFACEDESC2 ddsd;
	HRESULT res;

	OutTraceDW("DEBUG: BuildPrimaryEmu: lpdd=%x pCreateSurface=%x lpddsd=%x version=%d\n", lpdd, pCreateSurface, lpddsd, dxversion);
	// emulated primary surface
	memcpy((void *)&ddsd, lpddsd, lpddsd->dwSize);

	// handle the surface attributes before the ddsd.dwFlags gets updated:
	// if a surface desc is NOT specified, build one
	if(!(ddsd.dwFlags & DDSD_PIXELFORMAT)) SetPixFmt((LPDDSURFACEDESC2)&ddsd);
	// then save it
	dxw.VirtualPixelFormat = ddsd.ddpfPixelFormat;

	OutTraceDW("DDSD_PIXELFORMAT: color=%d flags=%x\n", dxw.VirtualPixelFormat.dwRGBBitCount, dxw.VirtualPixelFormat.dwFlags);
	ddsd.dwFlags &= ~(DDSD_BACKBUFFERCOUNT|DDSD_REFRESHRATE);
	ddsd.dwFlags |= (DDSD_CAPS|DDSD_WIDTH|DDSD_HEIGHT|DDSD_PIXELFORMAT);
	ddsd.ddsCaps.dwCaps &= ~(DDSCAPS_PRIMARYSURFACE|DDSCAPS_FLIP|DDSCAPS_COMPLEX|DDSCAPS_VIDEOMEMORY|DDSCAPS_LOCALVIDMEM);
	// DDSCAPS_OFFSCREENPLAIN seems required to support the palette in memory surfaces
	ddsd.ddsCaps.dwCaps |= (DDSCAPS_OFFSCREENPLAIN|DDSCAPS_SYSTEMMEMORY);
	// on WinXP Fifa 99 doesn't like DDSCAPS_SYSTEMMEMORY cap, so better to leave a way to unset it....
	if(dxw.dwFlags6 & NOSYSMEMPRIMARY) ddsd.ddsCaps.dwCaps &= ~DDSCAPS_SYSTEMMEMORY;

	ddsd.dwWidth = dxw.GetScreenWidth();
	ddsd.dwHeight = dxw.GetScreenHeight();

	// create Primary surface
	OutTraceDW("CreateSurface: %s\n", LogSurfaceAttributes((LPDDSURFACEDESC)&ddsd, "[Primary]" , __LINE__));
	res=(*pCreateSurface)(lpdd, &ddsd, lplpdds, 0);
	if(res){
		OutTraceE("CreateSurface: ERROR on DDSPrim res=%x(%s) at %d\n", res, ExplainDDError(res), __LINE__);
		if(res==DDERR_INVALIDPIXELFORMAT) DumpPixFmt(&ddsd);
		return res;
	}
	iBakBufferVersion=dxversion; // v2.03.01

	OutTraceDW("CreateSurface: created PRIMARY DDSPrim=%x\n", *lplpdds);
    if(IsDebug) DescribeSurface(*lplpdds, dxversion, "DDSPrim", __LINE__);
	HookDDSurface(lplpdds, dxversion, TRUE);
	// "Hoyle Casino Empire" opens a primary surface and NOT a backbuffer ....

	// build a default System palette and apply it to primary surface
	if((ddsd.ddpfPixelFormat.dwFlags & DDPF_PALETTEINDEXED8) &&
		(dxw.dwFlags6 & SYNCPALETTE)){ 
		if(lpDDP == NULL){
			res=(*pCreatePaletteMethod(dxversion))(lpdd, DDPCAPS_8BIT|DDPCAPS_ALLOW256, DefaultSystemPalette, &lpDDP, NULL);
			if(res) OutTrace("CreateSurface: CreatePalette ERROR err=%x at %d\n", res, __LINE__); 
		}
		// this must be done after hooking - who knows why?
		res=(*pSetPaletteMethod(dxversion))(*lplpdds, lpDDP);
		if(res) {
			OutTraceE("CreateSurface: SetPalette ERROR err=%x at %d\n", res, __LINE__);
		}
		else iDDPExtraRefCounter++;
	}

	// set a global capability value for surfaces that have to blit to primary
	// DDSCAPS_OFFSCREENPLAIN seems required to support the palette in memory surfaces
	// DDSCAPS_SYSTEMMEMORY makes operations faster, but it is not always good...
	dwBackBufferCaps = (DDSCAPS_OFFSCREENPLAIN|DDSCAPS_SYSTEMMEMORY);
	// on WinXP Fifa 99 doesn't like DDSCAPS_SYSTEMMEMORY cap, so better to leave a way to unset it....
	// this is important to avoid that certain D3D operations will abort - see "Forsaken" problem
	if(dxw.dwFlags6 & NOSYSMEMBACKBUF) dwBackBufferCaps = DDSCAPS_OFFSCREENPLAIN;

	if(dxw.dwFlags6 & SHAREDDC) bFlippedDC = TRUE;

	if(dxw.dwFlags5 & GDIMODE) return DD_OK;

	BuildRealSurfaces(lpdd, pCreateSurface, dxversion);
	return DD_OK;
}

static HRESULT BuildPrimaryFlippable(LPDIRECTDRAW lpdd, CreateSurface_Type pCreateSurface, LPDDSURFACEDESC2 lpddsd, int dxversion, LPDIRECTDRAWSURFACE *lplpdds, void *pu)
{
	DDSURFACEDESC2 ddsd;
	HRESULT res;

	OutTraceDW("DEBUG: BuildPrimaryFlippable: lpdd=%x pCreateSurface=%x lpddsd=%x version=%d\n", lpdd, pCreateSurface, lpddsd, dxversion);
	// emulated primary surface
	memcpy((void *)&ddsd, lpddsd, lpddsd->dwSize);

	// handle the surface attributes before the ddsd.dwFlags gets updated:
	// if a surface desc is NOT specified, build one
	if(!(ddsd.dwFlags & DDSD_PIXELFORMAT)) SetPixFmt((LPDDSURFACEDESC2)&ddsd);
	// then save it
	dxw.VirtualPixelFormat = ddsd.ddpfPixelFormat;

	OutTraceDW("DDSD_PIXELFORMAT: color=%d flags=%x\n", dxw.VirtualPixelFormat.dwRGBBitCount, dxw.VirtualPixelFormat.dwFlags);

	// dwFlags
	ddsd.dwFlags &= ~(DDSD_REFRESHRATE);
	ddsd.dwFlags |= (DDSD_CAPS|DDSD_WIDTH|DDSD_HEIGHT|DDSD_PIXELFORMAT|DDSD_BACKBUFFERCOUNT);

	// dwBackBufferCount: set to at least 1
	if(!(lpddsd->dwFlags & DDSD_BACKBUFFERCOUNT) || (lpddsd->dwBackBufferCount == 0)) ddsd.dwBackBufferCount = 1;

	// dwCaps
	ddsd.ddsCaps.dwCaps &= ~(DDSCAPS_PRIMARYSURFACE|DDSCAPS_VIDEOMEMORY|DDSCAPS_LOCALVIDMEM|DDSCAPS_FRONTBUFFER);
	ddsd.ddsCaps.dwCaps |= (DDSCAPS_COMPLEX|DDSCAPS_FLIP|DDSCAPS_OFFSCREENPLAIN|DDSCAPS_SYSTEMMEMORY);
	// on WinXP Fifa 99 doesn't like DDSCAPS_SYSTEMMEMORY cap, so better to leave a way to unset it....
	if(dxw.dwFlags6 & NOSYSMEMPRIMARY) ddsd.ddsCaps.dwCaps &= ~DDSCAPS_SYSTEMMEMORY;

	// dwWidth & dwHeight
	ddsd.dwWidth = dxw.GetScreenWidth();
	ddsd.dwHeight = dxw.GetScreenHeight();

	// create Primary surface
	OutTraceDW("CreateSurface: %s\n", LogSurfaceAttributes((LPDDSURFACEDESC)&ddsd, "[Primary]" , __LINE__));
	res=(*pCreateSurface)(lpdd, &ddsd, lplpdds, 0);
	if(res){
		OutTraceE("CreateSurface: ERROR on DDSPrim res=%x(%s) at %d\n", res, ExplainDDError(res), __LINE__);
		if(res==DDERR_INVALIDPIXELFORMAT) DumpPixFmt(&ddsd);
		return res;
	}
	iBakBufferVersion=dxversion; // v2.03.01

	OutTraceDW("CreateSurface: created PRIMARY DDSPrim=%x\n", *lplpdds);
    if(IsDebug) DescribeSurface(*lplpdds, dxversion, "DDSPrim", __LINE__);
	HookDDSurface(lplpdds, dxversion, TRUE);
	// "Hoyle Casino Empire" opens a primary surface and NOT a backbuffer ....

	// build a default System palette and apply it to primary surface
	if((ddsd.ddpfPixelFormat.dwFlags & DDPF_PALETTEINDEXED8) &&
		(dxw.dwFlags6 & SYNCPALETTE)){ 
		if(lpDDP == NULL){
			res=(*pCreatePaletteMethod(dxversion))(lpdd, DDPCAPS_8BIT|DDPCAPS_ALLOW256, DefaultSystemPalette, &lpDDP, NULL);
			if(res) OutTrace("CreateSurface: CreatePalette ERROR err=%x at %d\n", res, __LINE__); 
		}
		// this must be done after hooking - who knows why?
		res=(*pSetPaletteMethod(dxversion))(*lplpdds, lpDDP);
		if(res) {
			OutTraceE("CreateSurface: SetPalette ERROR err=%x at %d\n", res, __LINE__);
		}
		else iDDPExtraRefCounter++;
	}

	// set a global capability value for surfaces that have to blit to primary
	dwBackBufferCaps = (DDSCAPS_OFFSCREENPLAIN|DDSCAPS_SYSTEMMEMORY);
	// on WinXP Fifa 99 doesn't like DDSCAPS_SYSTEMMEMORY cap, so better to leave a way to unset it....
	// this is important to avoid that certain D3D operations will abort - see "Forsaken" problem
	if(dxw.dwFlags6 & NOSYSMEMBACKBUF) dwBackBufferCaps = DDSCAPS_OFFSCREENPLAIN;

	if(dxw.dwFlags6 & SHAREDDC) bFlippedDC = TRUE;

	if(dxw.dwFlags5 & GDIMODE) return DD_OK;

	BuildRealSurfaces(lpdd, pCreateSurface, dxversion);
	return DD_OK;
}

static HRESULT BuildPrimaryFullscreen(LPDIRECTDRAW lpdd, CreateSurface_Type pCreateSurface, LPDDSURFACEDESC2 lpddsd, int dxversion, LPDIRECTDRAWSURFACE *lplpdds, void *pu)
{
	DDSURFACEDESC2 ddsd;
	HRESULT res;

	OutTraceDW("DEBUG: BuildPrimaryFullscreen: lpdd=%x pCreateSurface=%x lpddsd=%x version=%d\n", lpdd, pCreateSurface, lpddsd, dxversion);
	// genuine primary surface
	memcpy((void *)&ddsd, lpddsd, lpddsd->dwSize);

	// create Primary surface
	OutTraceDW("CreateSurface: %s\n", LogSurfaceAttributes((LPDDSURFACEDESC)&ddsd, "[Primary]", __LINE__));
	res=(*pCreateSurface)(lpdd, &ddsd, lplpdds, 0);
	if(res){
		if (res==DDERR_PRIMARYSURFACEALREADYEXISTS){
			LPDIRECTDRAWSURFACE lpPrim;
			GetGDISurface_Type pGetGDISurface;
			switch(dxversion){
				default: 
				case 1: pGetGDISurface = pGetGDISurface1; break;
				case 2: pGetGDISurface = pGetGDISurface2; break;
				case 3: pGetGDISurface = pGetGDISurface3; break;
				case 4: pGetGDISurface = pGetGDISurface4; break;
				case 7: pGetGDISurface = pGetGDISurface7; break;
			}
			OutTraceE("CreateSurface: CreateSurface DDERR_PRIMARYSURFACEALREADYEXISTS workaround\n");
			(*pGetGDISurface)(lpPrimaryDD, &lpPrim);
			while ((*pReleaseSMethod(dxversion))(lpPrim));
			res = (*pCreateSurface)(lpdd, &ddsd, lplpdds, 0);
		}
		/* fall through */
		if(res){
			OutTraceE("CreateSurface: ERROR on DDSPrim res=%x(%s) at %d\n", res, ExplainDDError(res), __LINE__);
			if(res==DDERR_INVALIDPIXELFORMAT) DumpPixFmt(&ddsd);
			return res;
		}
	}

	OutTraceDW("CreateSurface: created PRIMARY DDSPrim=%x\n", *lplpdds);
    if(IsDebug) DescribeSurface(*lplpdds, dxversion, "DDSPrim", __LINE__);

	iBakBufferVersion=dxversion; 
	HookDDSurface(lplpdds, dxversion, TRUE);
	if(dxw.dwFlags1 & CLIPCURSOR) dxw.SetClipCursor();

	return DD_OK;
}

static HRESULT BuildPrimaryDir(LPDIRECTDRAW lpdd, CreateSurface_Type pCreateSurface, LPDDSURFACEDESC2 lpddsd, int dxversion, LPDIRECTDRAWSURFACE *lplpdds, void *pu)
{
	DDSURFACEDESC2 ddsd;
	HRESULT res;

	OutTraceDW("DEBUG: BuildPrimaryDir: lpdd=%x pCreateSurface=%x lpddsd=%x version=%d\n", lpdd, pCreateSurface, lpddsd, dxversion);
	// genuine primary surface
	memcpy((void *)&ddsd, lpddsd, lpddsd->dwSize);
	// v2.03.98 - when going to fullscreen mode and no emulation do not alter the capability masks, or in other words ...
	// if doing emulation or in window mode, fix the capability masks (De Morgan docet)
	ddsd.dwFlags &= ~(DDSD_WIDTH|DDSD_HEIGHT|DDSD_BACKBUFFERCOUNT|DDSD_REFRESHRATE|DDSD_PIXELFORMAT);
	ddsd.ddsCaps.dwCaps &= ~(DDSCAPS_FLIP|DDSCAPS_COMPLEX);
	// v2.02.93: don't move primary / backbuf surfaces on systemmemory when 3DDEVICE is requested
	// this impact also on capabilities for temporary surfaces for AERO optimized handling
	if ((lpddsd->dwFlags & DDSD_CAPS) && (lpddsd->ddsCaps.dwCaps & DDSCAPS_3DDEVICE)) ddsd.ddsCaps.dwCaps &= ~DDSCAPS_SYSTEMMEMORY;

	// create Primary surface
	OutTraceDW("CreateSurface: %s\n", LogSurfaceAttributes((LPDDSURFACEDESC)&ddsd, "[Primary]", __LINE__));
	res=(*pCreateSurface)(lpdd, &ddsd, lplpdds, 0);
	if(res){
		if (res==DDERR_PRIMARYSURFACEALREADYEXISTS){
			LPDIRECTDRAWSURFACE lpPrim;
			GetGDISurface_Type pGetGDISurface;
			switch(dxversion){
				default: 
				case 1: pGetGDISurface = pGetGDISurface1; break;
				case 2: pGetGDISurface = pGetGDISurface2; break;
				case 3: pGetGDISurface = pGetGDISurface3; break;
				case 4: pGetGDISurface = pGetGDISurface4; break;
				case 7: pGetGDISurface = pGetGDISurface7; break;
			}
			OutTraceE("CreateSurface: CreateSurface DDERR_PRIMARYSURFACEALREADYEXISTS workaround\n");
			(*pGetGDISurface)(lpPrimaryDD, &lpPrim);
			while ((*pReleaseSMethod(dxversion))(lpPrim));
			res = (*pCreateSurface)(lpdd, &ddsd, lplpdds, 0);
		}
		/* fall through */
		if(res){
			OutTraceE("CreateSurface: ERROR on DDSPrim res=%x(%s) at %d\n", res, ExplainDDError(res), __LINE__);
			if(res==DDERR_INVALIDPIXELFORMAT) DumpPixFmt(&ddsd);
			return res;
		}
	}

	OutTraceDW("CreateSurface: created PRIMARY DDSPrim=%x\n", *lplpdds);
    if(IsDebug) DescribeSurface(*lplpdds, dxversion, "DDSPrim", __LINE__);

	if(dxw.dwFlags1 & EMULATEBUFFER){
		lpDDSEmu_Prim = *lplpdds;
		dxwss.PopSurface(lpDDSEmu_Prim);

		ddsd.dwFlags = DDSD_WIDTH | DDSD_HEIGHT | DDSD_CAPS;
		// warning: can't create zero sized backbuffer surface !!!!
		ddsd.dwWidth = dxw.GetScreenWidth();
		ddsd.dwHeight = dxw.GetScreenHeight();
		ddsd.ddsCaps.dwCaps = 0;
		if (dxversion >= 4) ddsd.ddsCaps.dwCaps |= DDSCAPS_OFFSCREENPLAIN;
		OutTraceDW("CreateSurface: %s\n", LogSurfaceAttributes((LPDDSURFACEDESC)&ddsd, "[Dir FixBuf]", __LINE__));
		res=(*pCreateSurface)(lpdd, &ddsd, lplpdds, 0);
		if(res){
			OutTraceE("CreateSurface: ERROR on DDSPrim res=%x(%s) at %d\n",res, ExplainDDError(res), __LINE__);
			return res;
		}
		OutTraceDW("CreateSurface: created FIX DDSPrim=%x\n", *lplpdds);
		if(IsDebug) DescribeSurface(*lplpdds, dxversion, "DDSPrim(2)", __LINE__);
	}

	iBakBufferVersion=dxversion; // v2.03.37
	HookDDSurface(lplpdds, dxversion, TRUE);
	if(dxw.dwFlags1 & CLIPCURSOR) dxw.SetClipCursor();

	return DD_OK;
}

static HRESULT BuildBackBufferEmu(LPDIRECTDRAW lpdd, CreateSurface_Type pCreateSurface, LPDDSURFACEDESC2 lpddsd, int dxversion, LPDIRECTDRAWSURFACE *lplpdds, void *pu)
{
	DDSURFACEDESC2 ddsd;
	HRESULT res;

	OutTraceDW("DEBUG: BuildBackBufferEmu: lpdd=%x pCreateSurface=%x lpddsd=%x version=%d\n", lpdd, pCreateSurface, lpddsd, dxversion);
	// create BackBuffer surface
	memcpy(&ddsd, lpddsd, lpddsd->dwSize);
	ddsd.dwFlags &= ~(DDSD_BACKBUFFERCOUNT|DDSD_REFRESHRATE);
	ddsd.dwFlags |= (DDSD_CAPS|DDSD_HEIGHT|DDSD_WIDTH|DDSD_PIXELFORMAT);
	ddsd.ddsCaps.dwCaps &= ~(DDSCAPS_BACKBUFFER|DDSCAPS_PRIMARYSURFACE|DDSCAPS_FLIP|DDSCAPS_COMPLEX|DDSCAPS_VIDEOMEMORY|DDSCAPS_LOCALVIDMEM);

	// v2.03.75: if a surface desc is NOT specified, build one. This will allow ZBUF attach.
	if(!(lpddsd->dwFlags & DDSD_PIXELFORMAT)) SetPixFmt((LPDDSURFACEDESC2)&ddsd);

	// DDSCAPS_OFFSCREENPLAIN seems required to support the palette in memory surfaces
	ddsd.ddsCaps.dwCaps |= (DDSCAPS_SYSTEMMEMORY|DDSCAPS_OFFSCREENPLAIN);
	if(ddsd.ddsCaps.dwCaps & DDSCAPS_3DDEVICE) ddsd.ddsCaps.dwCaps &= ~DDSCAPS_SYSTEMMEMORY; // necessary: Martian Gotic crashes otherwise
	// on WinXP Fifa 99 doesn't like DDSCAPS_SYSTEMMEMORY cap, so better to leave a way to unset it....
	if(dxw.dwFlags6 & NOSYSMEMBACKBUF) ddsd.ddsCaps.dwCaps &= ~DDSCAPS_SYSTEMMEMORY;
	ddsd.dwWidth = dxw.GetScreenWidth();
	ddsd.dwHeight = dxw.GetScreenHeight();
	GetPixFmt(&ddsd);

	OutTraceDW("CreateSurface: %s\n", LogSurfaceAttributes((LPDDSURFACEDESC)&ddsd, "[Backbuf]", __LINE__));
	res=(*pCreateSurface)(lpdd, &ddsd, lplpdds, 0);
	if(res) {
		OutTraceE("CreateSurface ERROR: res=%x(%s) at %d\n", res, ExplainDDError(res), __LINE__);
		if(res==DDERR_INVALIDPIXELFORMAT) DumpPixFmt(&ddsd);
		return res;
	}

	OutTraceDW("CreateSurface: created BACK DDSBack=%x\n", *lplpdds);
    if(IsDebug) DescribeSurface(*lplpdds, dxversion, "DDSBack", __LINE__);
	HookDDSurface(lplpdds, dxversion, FALSE); // added !!!
	iBakBufferVersion=dxversion; // v2.02.31

	if((ddsd.ddpfPixelFormat.dwFlags & DDPF_PALETTEINDEXED8) &&
		(dxw.dwFlags6 & SYNCPALETTE)){ 
		if(lpDDP == NULL){
			res=(*pCreatePaletteMethod(dxversion))(lpdd, DDPCAPS_8BIT|DDPCAPS_ALLOW256, DefaultSystemPalette, &lpDDP, NULL);
			if(res) OutTrace("CreateSurface: CreatePalette ERROR err=%x at %d\n", res, __LINE__); 
		}
		// this must be done after hooking - who knows why?
		res=(*pSetPaletteMethod(dxversion))(*lplpdds, lpDDP);
		if(res) {
			OutTraceE("CreateSurface: SetPalette ERROR err=%x at %d\n", res, __LINE__);
		}
		else iDDPExtraRefCounter++;
	}

	// V2.1.85/V2.2.34: tricky !!!!
	// When a real backbuffer is created, it has a reference to its frontbuffer.
	// some games (Monopoly 3D) may depend on this setting - i.e. they could close
	// the exceeding references - so this is better be replicated adding an initial
	// reference to the zero count. But you don't have to do this if the backbuffer
	// is created independently by the primary surface.
	(*lplpdds)->AddRef(); // should it be repeated BBCount times????

	return DD_OK;
}

static HRESULT BuildBackBufferFlippable(LPDIRECTDRAW lpdd, CreateSurface_Type pCreateSurface, LPDDSURFACEDESC2 lpddsd, int dxversion, LPDIRECTDRAWSURFACE *lplpdds, void *pu)
{
	DDSURFACEDESC2 ddsd;
	HRESULT res;

	OutTraceDW("DEBUG: BuildBackBufferFlippable: lpdd=%x pCreateSurface=%x lpddsd=%x version=%d\n", lpdd, pCreateSurface, lpddsd, dxversion);
	//MessageBox(NULL, "BuildBackBufferFlippable", "DxWnd", MB_OK);
	
	// create BackBuffer surface
	memcpy(&ddsd, lpddsd, lpddsd->dwSize);

	ddsd.dwFlags &= ~(DDSD_BACKBUFFERCOUNT|DDSD_REFRESHRATE);
	ddsd.dwFlags |= (DDSD_CAPS|DDSD_HEIGHT|DDSD_WIDTH|DDSD_PIXELFORMAT);

	ddsd.ddsCaps.dwCaps &= ~(DDSCAPS_PRIMARYSURFACE|DDSCAPS_FRONTBUFFER|DDSCAPS_BACKBUFFER|DDSCAPS_VIDEOMEMORY|DDSCAPS_LOCALVIDMEM);
	ddsd.ddsCaps.dwCaps |= (DDSCAPS_SYSTEMMEMORY|DDSCAPS_OFFSCREENPLAIN);

	ddsd.dwWidth = dxw.GetScreenWidth();
	ddsd.dwHeight = dxw.GetScreenHeight();
	GetPixFmt(&ddsd);

	OutTraceDW("CreateSurface: %s\n", LogSurfaceAttributes((LPDDSURFACEDESC)&ddsd, "[Backbuf]", __LINE__));
	res=(*pCreateSurface)(lpdd, &ddsd, lplpdds, 0);
	if(res) {
		OutTraceE("CreateSurface ERROR: res=%x(%s) at %d\n", res, ExplainDDError(res), __LINE__);
		if(res==DDERR_INVALIDPIXELFORMAT) DumpPixFmt(&ddsd);
		return res;
	}

	OutTraceDW("CreateSurface: created BACK DDSBack=%x\n", *lplpdds);
    if(IsDebug) DescribeSurface(*lplpdds, dxversion, "DDSBack", __LINE__);
	HookDDSurface(lplpdds, dxversion, FALSE); // added !!!
	iBakBufferVersion=dxversion; // v2.02.31

	if((ddsd.ddpfPixelFormat.dwFlags & DDPF_PALETTEINDEXED8) &&
		(dxw.dwFlags6 & SYNCPALETTE)){ 
		if(lpDDP == NULL){
			res=(*pCreatePaletteMethod(dxversion))(lpdd, DDPCAPS_8BIT|DDPCAPS_ALLOW256, DefaultSystemPalette, &lpDDP, NULL);
			if(res) OutTrace("CreateSurface: CreatePalette ERROR err=%x at %d\n", res, __LINE__); 
		}
		// this must be done after hooking - who knows why?
		res=(*pSetPaletteMethod(dxversion))(*lplpdds, lpDDP);
		if(res) {
			OutTraceE("CreateSurface: SetPalette ERROR err=%x at %d\n", res, __LINE__);
		}
		else iDDPExtraRefCounter++;
	}

	return DD_OK;
}

static HRESULT AttachBackBufferFlippable(LPDIRECTDRAW lpdd, CreateSurface_Type pCreateSurface, LPDDSURFACEDESC2 lpddsd, int dxversion, LPDIRECTDRAWSURFACE *lplpdds, void *pu)
{
	HRESULT res;
	LPDIRECTDRAWSURFACE lpDDSPrim;
	OutTraceDW("DEBUG: AttachBackBufferFlippable: lpdd=%x pCreateSurface=%x lpddsd=%x version=%d\n", lpdd, pCreateSurface, lpddsd, dxversion);

	// retrieve the attached backbuffer surface and hook it

	if(lpddsd->dwBackBufferCount == 0) return DD_OK; // nothing to retrieve

	GetAttachedSurface_Type pGetAttachedSurface;
	DDSCAPS2 caps;
	switch(dxversion){
		case 1: pGetAttachedSurface = pGetAttachedSurface1; break;
		case 2: pGetAttachedSurface = pGetAttachedSurface2; break;
		case 3: pGetAttachedSurface = pGetAttachedSurface3; break;
		case 4: pGetAttachedSurface = pGetAttachedSurface4; break;
		case 7: pGetAttachedSurface = pGetAttachedSurface7; break;
	}
	memset(&caps, 0, sizeof(caps));
	caps.dwCaps = DDSCAPS_BACKBUFFER;
	lpDDSPrim = dxwss.GetPrimarySurface();
	res = (*pGetAttachedSurface)(lpDDSPrim, (LPDDSCAPS)&caps, lplpdds);
	if(res){
		OutTraceE("CreateSurface: GetAttachedSurface ERROR on DDSPrim res=%x(%s) at %d\n", res, ExplainDDError(res), __LINE__);
		return res;
	}

	OutTraceDW("CreateSurface: retrieved BACK DDSBack=%x\n", *lplpdds);
    if(IsDebug) DescribeSurface(*lplpdds, dxversion, "DDSBack", __LINE__);
	HookDDSurface(lplpdds, dxversion, FALSE); // added !!!
	iBakBufferVersion=dxversion; // v2.02.31

	return DD_OK;
}

static HRESULT BuildBackBufferFullscreen(LPDIRECTDRAW lpdd, CreateSurface_Type pCreateSurface, LPDDSURFACEDESC2 lpddsd, int dxversion, LPDIRECTDRAWSURFACE *lplpdds, void *pu)
{
	OutTraceDW("DEBUG: BuildBackBufferFullscreen: lpdd=%x pCreateSurface=%x lpddsd=%x version=%d\n", lpdd, pCreateSurface, lpddsd, dxversion);
	return DD_OK;
}

static HRESULT AttachBackBufferFullscreen(LPDIRECTDRAW lpdd, CreateSurface_Type pCreateSurface, LPDDSURFACEDESC2 lpddsd, int dxversion, LPDIRECTDRAWSURFACE *lplpdds, void *pu)
{
	OutTraceDW("DEBUG: AttachBackBufferFullscreen: lpdd=%x pCreateSurface=%x lpddsd=%x version=%d\n", lpdd, pCreateSurface, lpddsd, dxversion);
	return DD_OK;
}

static HRESULT BuildBackBufferDir(LPDIRECTDRAW lpdd, CreateSurface_Type pCreateSurface, LPDDSURFACEDESC2 lpddsd, int dxversion, LPDIRECTDRAWSURFACE *lplpdds, void *pu)
{
	DDSURFACEDESC2 ddsd;
	HRESULT res;

	OutTraceDW("DEBUG: BuildBackBufferDir: lpdd=%x pCreateSurface=%x lpddsd=%x version=%d\n", lpdd, pCreateSurface, lpddsd, dxversion);
	// create BackBuffer surface
	// ClearSurfaceDesc((void *)&ddsd, dxversion);
	memcpy(&ddsd, lpddsd, lpddsd->dwSize);
	if(dxw.IsEmulated || dxw.Windowize){
		ddsd.dwFlags &= ~(DDSD_WIDTH|DDSD_HEIGHT|DDSD_BACKBUFFERCOUNT|DDSD_REFRESHRATE|DDSD_PIXELFORMAT);
		ddsd.dwFlags |= (DDSD_CAPS|DDSD_HEIGHT|DDSD_WIDTH);
		ddsd.ddsCaps.dwCaps &= ~(DDSCAPS_PRIMARYSURFACE|DDSCAPS_FLIP|DDSCAPS_COMPLEX);
		// v2.02.93: don't move primary / backbuf surfaces on systemmemory when 3DDEVICE is requested
		if(lpddsd->ddsCaps.dwCaps & DDSCAPS_3DDEVICE) {
			ddsd.ddsCaps.dwCaps &= ~DDSCAPS_SYSTEMMEMORY;
		}
		else {
			ddsd.ddsCaps.dwCaps |= DDSCAPS_SYSTEMMEMORY; 
			if (dxversion >= 4) ddsd.ddsCaps.dwCaps |= DDSCAPS_OFFSCREENPLAIN;
			ddsd.ddsCaps.dwCaps &= ~(DDSCAPS_VIDEOMEMORY|DDSCAPS_LOCALVIDMEM);
		}
	}
	if(dxw.dwFlags6 & NOSYSMEMBACKBUF) ddsd.ddsCaps.dwCaps &= ~DDSCAPS_SYSTEMMEMORY;
	ddsd.dwWidth = dxw.GetScreenWidth();
	ddsd.dwHeight = dxw.GetScreenHeight();

	if (dxw.dwFlags2 & BACKBUFATTACH) {
		LPDIRECTDRAWSURFACE lpPrim;
		DDSURFACEDESC2 prim;
		GetGDISurface_Type pGetGDISurface;
		switch(dxversion){
			default: 
			case 1: pGetGDISurface = pGetGDISurface1; break;
			case 2: pGetGDISurface = pGetGDISurface2; break;
			case 3: pGetGDISurface = pGetGDISurface3; break;
			case 4: pGetGDISurface = pGetGDISurface4; break;
			case 7: pGetGDISurface = pGetGDISurface7; break;
		}
		(*pGetGDISurface)(lpPrimaryDD, &lpPrim);
		memset(&prim, 0, sizeof(DDSURFACEDESC2));
		prim.dwSize = (dxversion >= 4) ? sizeof(DDSURFACEDESC2) : sizeof(DDSURFACEDESC);
		res=lpPrim->GetSurfaceDesc((DDSURFACEDESC *)&prim);
		(*pReleaseSMethod(dxversion))(lpPrim);
		ddsd.dwWidth = prim.dwWidth;
		ddsd.dwHeight = prim.dwHeight;
		OutTraceDW("BMX FIX: res=%x(%s) wxh=(%dx%d)\n", res, ExplainDDError(res),ddsd.dwWidth, ddsd.dwHeight);
	}
	OutTraceDW("CreateSurface: %s\n", LogSurfaceAttributes((LPDDSURFACEDESC)&ddsd, "[Backbuf]", __LINE__));
	res=(*pCreateSurface)(lpdd, &ddsd, lplpdds, 0);
	if(res) {
		if ((dxw.dwFlags1 & SWITCHVIDEOMEMORY) && (res==DDERR_OUTOFVIDEOMEMORY)){
			OutTraceDW("CreateSurface: CreateSurface DDERR_OUTOFVIDEOMEMORY ERROR at %d, retry in SYSTEMMEMORY\n", __LINE__);
			ddsd.ddsCaps.dwCaps &= ~DDSCAPS_VIDEOMEMORY; 
			ddsd.ddsCaps.dwCaps |= DDSCAPS_SYSTEMMEMORY; 
			res=(*pCreateSurface)(lpdd, &ddsd, lplpdds, 0);
		}
		if(res){
			OutTraceE("CreateSurface ERROR: res=%x(%s) at %d\n", res, ExplainDDError(res), __LINE__);
			return res;
		}
	}

	OutTraceDW("CreateSurface: created BACK DDSBack=%x\n", *lplpdds);
    if(IsDebug) DescribeSurface(*lplpdds, dxversion, "DDSBack", __LINE__);
	HookDDSurface(lplpdds, dxversion, FALSE); // added !!!
	iBakBufferVersion=dxversion; // v2.02.31

	return DD_OK;
}		

static HRESULT BuildGenericEmu(LPDIRECTDRAW lpdd, CreateSurface_Type pCreateSurface, LPDDSURFACEDESC2 lpddsd, int dxversion, LPDIRECTDRAWSURFACE *lplpdds, void *pu)
{
	DDSURFACEDESC2 ddsd;
	HRESULT res;

	OutTraceDW("DEBUG: BuildGenericEmu: lpdd=%x pCreateSurface=%x lpddsd=%x version=%d\n", lpdd, pCreateSurface, lpddsd, dxversion);
	memcpy(&ddsd, lpddsd, lpddsd->dwSize); // Copy over ....
	FixSurfaceCaps(&ddsd, dxversion);
	// It looks that DDSCAPS_SYSTEMMEMORY surfaces can perfectly be DDSCAPS_3DDEVICE as well. 
	// For "Risk II" it is necessary that both the primary surface and the offscreen surfaces are generated
	// with the same type, so that assuming an identical lPitch and memcopy-ing from one buffer to the 
	// other is a legitimate operation. 

	if(dxw.dwFlags6 & POWER2WIDTH){ // v2.03.28: POWER2WIDTH to fix "Midtown Madness" in surface emulation mode
		if(((ddsd.dwFlags & (DDSD_CAPS|DDSD_HEIGHT|DDSD_WIDTH)) == (DDSD_CAPS|DDSD_HEIGHT|DDSD_WIDTH)) &&
			(ddsd.ddsCaps.dwCaps & DDSCAPS_OFFSCREENPLAIN)
		){
			DWORD dwWidth;
			dwWidth = ((ddsd.dwWidth + 3) >> 2) << 2;
			if(dwWidth != ddsd.dwWidth) OutTraceDW("CreateSurface: fixed surface width %d->%d\n", ddsd.dwWidth, dwWidth);
			ddsd.dwWidth = dwWidth;
		}
	}

	res=(*pCreateSurface)(lpdd, &ddsd, lplpdds, pu);
	if ((dxw.dwFlags1 & SWITCHVIDEOMEMORY) && (res!=DD_OK)){
		OutTraceDW("CreateSurface ERROR: res=%x(%s) at %d, retry\n", res, ExplainDDError(res), __LINE__);
		ddsd.ddsCaps.dwCaps &= ~DDSCAPS_VIDEOMEMORY;
		ddsd.ddsCaps.dwCaps |= DDSCAPS_SYSTEMMEMORY;
		res=(*pCreateSurface)(lpdd, &ddsd, lplpdds, pu);
	}
	if (res) {
		OutTraceE("CreateSurface: ERROR on Emu_Generic res=%x(%s) at %d\n", res, ExplainDDError(res), __LINE__);
		return res;
	}

	OutTraceDW("CreateSurface: CREATED lpddsd=%x version=%d %s\n", 
		*lplpdds, dxversion, LogSurfaceAttributes((LPDDSURFACEDESC)&ddsd, "[Emu Generic]", __LINE__));
		
	// v2.02.66: if 8BPP paletized surface and a primary palette exixts, apply.
	// fixes "Virtua Fighter PC" palette bug
	if(lpDDP && (ddsd.ddpfPixelFormat.dwFlags & DDPF_PALETTEINDEXED8)){
		res=(*pSetPaletteMethod(dxversion))(*lplpdds, lpDDP);
		if(res)
			OutTraceE("SetPalette: ERROR on lpdds=%x(Emu_Generic) res=%x(%s) at %d\n", *lplpdds, res, ExplainDDError(res), __LINE__);
		else {
			OutTraceDW("CreateSurface: applied lpddp=%x to lpdds=%x\n", lpDDP, *lplpdds);
			iDDPExtraRefCounter++;
		}
	}

	// diagnostic hooks ....
	HookDDSurface(lplpdds, dxversion, FALSE);

	return DD_OK;
}

static HRESULT BuildGenericFlippable(LPDIRECTDRAW lpdd, CreateSurface_Type pCreateSurface, LPDDSURFACEDESC2 lpddsd, int dxversion, LPDIRECTDRAWSURFACE *lplpdds, void *pu)
{
	DDSURFACEDESC2 ddsd;
	HRESULT res;

	OutTraceDW("DEBUG: BuildGenericFlippable: lpdd=%x pCreateSurface=%x lpddsd=%x version=%d\n", lpdd, pCreateSurface, lpddsd, dxversion);
	memcpy(&ddsd, lpddsd, lpddsd->dwSize); // Copy over ....
	FixSurfaceCaps(&ddsd, dxversion);

	if(dxw.dwFlags6 & POWER2WIDTH){ // v2.03.28: POWER2WIDTH to fix "Midtown Madness" in surface emulation mode
		if(((ddsd.dwFlags & (DDSD_CAPS|DDSD_HEIGHT|DDSD_WIDTH)) == (DDSD_CAPS|DDSD_HEIGHT|DDSD_WIDTH)) &&
			(ddsd.ddsCaps.dwCaps & DDSCAPS_OFFSCREENPLAIN)
		){
			DWORD dwWidth;
			dwWidth = ((ddsd.dwWidth + 3) >> 2) << 2;
			if(dwWidth != ddsd.dwWidth) OutTraceDW("CreateSurface: fixed surface width %d->%d\n", ddsd.dwWidth, dwWidth);
			ddsd.dwWidth = dwWidth;
		}
	}

	res=(*pCreateSurface)(lpdd, &ddsd, lplpdds, pu);
	if ((dxw.dwFlags1 & SWITCHVIDEOMEMORY) && (res!=DD_OK)){
		OutTraceDW("CreateSurface ERROR: res=%x(%s) at %d, retry\n", res, ExplainDDError(res), __LINE__);
		ddsd.ddsCaps.dwCaps &= ~DDSCAPS_VIDEOMEMORY;
		ddsd.ddsCaps.dwCaps |= DDSCAPS_SYSTEMMEMORY;
		res=(*pCreateSurface)(lpdd, &ddsd, lplpdds, pu);
	}
	if (res) {
		OutTraceE("CreateSurface: ERROR on Emu_Generic res=%x(%s) at %d\n", res, ExplainDDError(res), __LINE__);
		return res;
	}

	OutTraceDW("CreateSurface: CREATED lpddsd=%x version=%d %s\n", 
		*lplpdds, dxversion, LogSurfaceAttributes((LPDDSURFACEDESC)&ddsd, "[Emu Generic]", __LINE__));
		
	// v2.02.66: if 8BPP paletized surface and a primary palette exixts, apply.
	// fixes "Virtua Fighter PC" palette bug
	if(lpDDP && (ddsd.ddpfPixelFormat.dwFlags & DDPF_PALETTEINDEXED8)){
		res=(*pSetPaletteMethod(dxversion))(*lplpdds, lpDDP);
		if(res)
			OutTraceE("SetPalette: ERROR on lpdds=%x(Emu_Generic) res=%x(%s) at %d\n", *lplpdds, res, ExplainDDError(res), __LINE__);
		else {
			OutTraceDW("CreateSurface: applied lpddp=%x to lpdds=%x\n", lpDDP, *lplpdds);
			iDDPExtraRefCounter++;
		}
	}

	// diagnostic hooks ....
	HookDDSurface(lplpdds, dxversion, FALSE);

	return DD_OK;
}

static HRESULT BuildGenericDir(LPDIRECTDRAW lpdd, CreateSurface_Type pCreateSurface, LPDDSURFACEDESC2 lpddsd, int dxversion, LPDIRECTDRAWSURFACE *lplpdds, void *pu)
{
	HRESULT res;

	OutTraceDW("DEBUG: BuildGenericDir: lpdd=%x pCreateSurface=%x lpddsd=%x version=%d\n", lpdd, pCreateSurface, lpddsd, dxversion);
	OutTraceDW("CreateSurface: %s\n", LogSurfaceAttributes((LPDDSURFACEDESC)lpddsd, "[Dir Generic]", __LINE__));

	res = (*pCreateSurface)(lpdd, lpddsd, lplpdds, 0); 
	if(res){
		// v2.02.60: Ref. game Incoming GOG release, post by Marek, error DDERR_UNSUPPORTED while trying to create ZBUFFER surface 
		if ((dxw.dwFlags1 & SWITCHVIDEOMEMORY) && ((res==DDERR_OUTOFVIDEOMEMORY)||(res==DDERR_UNSUPPORTED))){
			OutTraceDW("CreateSurface ERROR: res=%x(%s) at %d, retry\n", res, ExplainDDError(res), __LINE__);
			lpddsd->ddsCaps.dwCaps &= ~DDSCAPS_VIDEOMEMORY;
			lpddsd->ddsCaps.dwCaps |= DDSCAPS_SYSTEMMEMORY;
			res = (*pCreateSurface)(lpdd, lpddsd, lplpdds, 0); 
		}
		if(res){
			OutTraceE("CreateSurface: CreateSurface ERROR res=%x(%s) at %d\n", res, ExplainDDError(res), __LINE__);
			return res;
		}
	}

	OutTraceDW("CreateSurface: CREATED lpddsd=%x version=%d %s\n", 
		*lplpdds, dxversion, LogSurfaceAttributes((LPDDSURFACEDESC)lpddsd, "[Dir Generic]", __LINE__));

	// hooks ....
	HookDDSurface(lplpdds, dxversion, FALSE);

	return DD_OK;
}

HRESULT WINAPI extCreateSurface(int dxversion, CreateSurface_Type pCreateSurface, LPDIRECTDRAW lpdd, DDSURFACEDESC2 *lpddsd,
	LPDIRECTDRAWSURFACE *lplpdds, void *pu)
{
	HRESULT res;
	DDSURFACEDESC2 ddsd;
	LPDIRECTDRAWSURFACE lpDDSPrim;
	LPDIRECTDRAWSURFACE lpDDSBack = NULL;
	DWORD CurFlags;
	int TargetSize;
	typedef HRESULT (*BuildSurface_Type)(LPDIRECTDRAW, CreateSurface_Type, LPDDSURFACEDESC2, int, LPDIRECTDRAWSURFACE *, void *);
	static BuildSurface_Type BuildPrimary = 0;
	static BuildSurface_Type BuildBackBuffer = 0;
	static BuildSurface_Type AttachBackBuffer = 0;
	static BuildSurface_Type BuildGeneric = 0;
	static enum  {
		PRIMARY_FULLSCREEN = 0,
		PRIMARY_DIRECT,
		PRIMARY_FLIPPABLE,	
		PRIMARY_EMULATED
	} SurfaceMode;

	OutTraceDDRAW("CreateSurface(%d): lpdd=%x %s\n", 
		dxversion, lpdd, LogSurfaceAttributes((LPDDSURFACEDESC)lpddsd, "[CreateSurface]", __LINE__));
	
	// v2.03.95.fx1 - deleted: some texture handling REQUIRES a proper FourCC codec. 
	// maybe it could be suppressed by a dedicated config. flag and on primary surfaces only?
	//lpddsd->ddpfPixelFormat.dwFourCC = 0;

	// if not initialized yet ....
	if(BuildPrimary == 0){ 
		char *sLabel;
		SurfaceMode = PRIMARY_FULLSCREEN;
		if(dxw.IsEmulated || dxw.Windowize){
			SurfaceMode = (dxw.dwFlags1 & EMULATESURFACE) ? 
				((dxw.dwFlags6 & FLIPEMULATION) ? 
					PRIMARY_EMULATED : 
					PRIMARY_FLIPPABLE) : 
				PRIMARY_DIRECT;
		}

		switch(SurfaceMode)	{
			case PRIMARY_FULLSCREEN:
				BuildPrimary = BuildPrimaryFullscreen;
				BuildBackBuffer = BuildBackBufferFullscreen;
				AttachBackBuffer = AttachBackBufferFullscreen;
				BuildGeneric = BuildGenericDir;
				sLabel="FULLSCR";
				break;
			case PRIMARY_DIRECT: 
				BuildPrimary = BuildPrimaryDir;
				BuildBackBuffer = BuildBackBufferDir;
				AttachBackBuffer = BuildBackBufferDir;
				BuildGeneric = BuildGenericDir;
				sLabel="DIRECT";
				break;
			case PRIMARY_FLIPPABLE:
				BuildPrimary = BuildPrimaryFlippable;
				BuildBackBuffer = BuildBackBufferFlippable;
				AttachBackBuffer = AttachBackBufferFlippable;
				BuildGeneric = BuildGenericFlippable;
				sLabel="FLIPPABLE";
				break;
			case PRIMARY_EMULATED:
				BuildPrimary = BuildPrimaryEmu;
				BuildBackBuffer = BuildBackBufferEmu;
				AttachBackBuffer = BuildBackBufferEmu;
				BuildGeneric = BuildGenericEmu;
				sLabel="EMULATED";
				break;	
		}
		OutTraceDW("CreateSurface: MODE INITIALIZED mode=%d(%s)\n", SurfaceMode, sLabel);
	}

	// check for lpddsd->dwSize value
	TargetSize=(dxversion<4)?sizeof(DDSURFACEDESC):sizeof(DDSURFACEDESC2);
	if(lpddsd->dwSize != TargetSize){
		char sMsg[81];
		sprintf_s(sMsg,80, "CreateSurface: ASSERT bad dwSize=%d dxversion=%d\n", 
			lpddsd->dwSize, dxversion);
		OutTraceDW(sMsg);
		if(IsAssertEnabled) MessageBox(0, sMsg, "CreateSurface", MB_OK | MB_ICONEXCLAMATION);
		return DDERR_INVALIDPARAMS;
	}

	//GHO workaround (needed for WarWind, Rogue Spear):
	if (lpddsd->dwFlags && !(lpddsd->dwFlags & 0x1)){
		OutTraceDW("CreateSurface: fixing illegal dwFlags value: %x -> %x\n",
			lpddsd->dwFlags, (lpddsd->dwFlags | DDSD_CAPS));
		lpddsd->dwFlags |= DDSD_CAPS;
	}

	memcpy(&ddsd, lpddsd, lpddsd->dwSize); // Copy

	// v2.02.38: this is odd: in "Star Force Deluxe" there is no PRIMARY surface, but a surface with 
	// 0 flags and 0 capabilities serves for this purpose. Is it a side-effect of old ddraw releases?
	if((dxversion == 1) && ((ddsd.dwFlags & ~DDSD_BACKBUFFERCOUNT) == 0)){ // Star Force Deluxe
		ddsd.dwFlags = DDSD_CAPS|DDSD_HEIGHT|DDSD_WIDTH;
		ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;
		//if(dxw.VirtualPixelFormat.dwRGBBitCount == 8) ddsd.ddsCaps.dwCaps |= DDSCAPS_PALETTE;
	}

	// creation of the primary surface....
	if(ddsd.dwFlags & DDSD_CAPS && ddsd.ddsCaps.dwCaps & DDSCAPS_PRIMARYSURFACE){
		dxwss.ClearSurfaceList(); // v2.03.83: "Daytona USA" would saturate the list otherwise
		SetVSyncDelays(dxversion, lpdd);
		GetHookInfo()->Height=(short)dxw.GetScreenHeight();
		GetHookInfo()->Width=(short)dxw.GetScreenWidth();
		GetHookInfo()->ColorDepth=(short)dxw.VirtualPixelFormat.dwRGBBitCount;
		GetHookInfo()->DXVersion=dxversion;
		lpPrimaryDD = lpdd; // v2.1.87
		memcpy(&DDSD_Prim, lpddsd, sizeof(DDSD_Prim)); // v2.02.37

		// beware of the different behaviour between older and newer directdraw releases...
		if(dxversion >= 4){
			if (lpDDSEmu_Back) while(lpDDSEmu_Back->Release());
			if (lpDDSEmu_Prim) while(lpDDSEmu_Prim->Release());
			if (ddsd.dwFlags & DDSD_BACKBUFFERCOUNT) { // Praetorians !!!!
				lpDDSBack = dxwss.GetBackBufferSurface();
				if (lpDDSBack) {
					if(dxw.dwFlags6 & FLIPEMULATION) while(lpDDSBack->Release());
					dxwss.PopSurface(lpDDSBack);
					lpDDSBack = NULL;
				}
			}
		}
		lpDDSEmu_Back=NULL;
		lpDDSEmu_Prim=NULL;

		int BBCount=0; // or 1 ??
		if (ddsd.dwFlags & DDSD_BACKBUFFERCOUNT) BBCount=ddsd.dwBackBufferCount;
		if ((BBCount > 0) && (iBakBufferVersion < 4)){ 
			lpDDSBack=NULL;
			OutTraceDW("CreateSurface: backbuffer cleared - BackBufferCount=%d\n", BBCount);
		}

		if (BBCount > MAXBACKBUFFERS){
			char sMsg[81];
			sprintf_s(sMsg, 80, "CreateSurface: BackBufferCount=%d\n", BBCount);
			OutTraceDW(sMsg);
			if (IsAssertEnabled) MessageBox(0, sMsg, "CreateSurface", MB_OK | MB_ICONEXCLAMATION);
			// recover ...
			BBCount = MAXBACKBUFFERS;
		}

		// build emulated primary surface, real primary and backbuffer surfaces
		CurFlags=ddsd.dwFlags;
		res=BuildPrimary(lpdd, pCreateSurface, lpddsd, dxversion, lplpdds, NULL);
		if(res) return res;
		lpDDSPrim = *lplpdds;
		dxwss.PushPrimarySurface(lpDDSPrim, dxversion);
		RegisterPixelFormat(dxversion, lpDDSPrim);

		if (BBCount){
			// build emulated backbuffer surface
			res=AttachBackBuffer(lpdd, pCreateSurface, lpddsd, dxversion, &lpDDSBack, NULL);
			if(res) return res;
			dxwss.PushBackBufferSurface(lpDDSBack, dxversion);
		}

		if(IsTraceDDRAW){
			char sInfo[256+1];
			sprintf(sInfo, "CreateSurface: created DDSPrim=%x DDSBack=%x", lpDDSPrim, lpDDSBack);
			if(dxw.IsEmulated) sprintf(sInfo, "%s DDSEmu_Prim=%x", sInfo, lpDDSEmu_Prim);
			if(dxw.dwFlags1 & EMULATESURFACE) sprintf(sInfo, "%s DDSEmu_Back=%x", sInfo, lpDDSEmu_Back);
			strcat(sInfo, "\n");
			OutTrace(sInfo);
		}
 
		// rebuild the clipper area
		if(dxw.dwFlags1 & CLIPCURSOR) dxw.SetClipCursor();

		// v2.2.64: added extra ref needed to preserve ddraw session for later use. Is it a ddraw1 legacy?
		// seems to fix problems in "Warhammer 40K Rites Of War" that uses a ddraw session after reaching 0 refcount.
		// v2.2.84: avoid the extra referenced in non windowed mode since it causes the window shift reported by gsky916
		// for Wind Fantasy SP.
		// v2.3.59: same extra reference is needed by "Wahammer Chaos Gate" that uses ddraw interface release 2
		// v2.3.72: fixed previous fix: condition is <=2, not >=2 ! 
		// Be aware that it may perhaps become <=3, if we get the same problem elsewhere
		// v2.3.96: deleted the if(Windowized) condition: AddRef is needed also in fullscreen mode.
		if(dxw.dwDDVersion<=2) lpdd->AddRef();

		return DD_OK;
	}

	// a request for a separate (not complex) backbuffer to attach later on, maybe.
	if ((ddsd.dwFlags & DDSD_CAPS) && (ddsd.ddsCaps.dwCaps & DDSCAPS_BACKBUFFER)){
		if (lpDDSBack) {
			OutTraceDW("CreateSurface: returning current DDSBack=%x\n", lpDDSBack);
			*lplpdds = lpDDSBack;
			return DD_OK;
		}

		res=BuildBackBuffer(lpdd, pCreateSurface, lpddsd, dxversion, lplpdds, NULL);
		if(res == DD_OK) {
			dxwss.PushBackBufferSurface(*lplpdds, dxversion);
			dxwcdb.PushCaps(*lplpdds, lpddsd->ddsCaps.dwCaps);
		}
		return res;
	}

	// if nothing else, it's a generic/zbuffer surface

	res=BuildGeneric(lpdd, pCreateSurface, lpddsd, dxversion, lplpdds, pu);
	if(!res) {
		dxwss.PopSurface(*lplpdds);
		if(lpddsd->ddsCaps.dwCaps & DDSCAPS_ZBUFFER) {
			// v2.03.82: save ZBUFFER capabilities for later fix in D3D CreateDevice
			if(lpddsd->ddsCaps.dwCaps & DDSCAPS_ZBUFFER) dxwcdb.PushCaps(*lplpdds, lpddsd->ddsCaps.dwCaps);
			OutTraceDW("CreateSurface: lpDDZBuffer=%x save ZBUFFER caps=%x(%s)\n", *lplpdds, lpddsd->ddsCaps.dwCaps, ExplainDDSCaps(lpddsd->ddsCaps.dwCaps));
		}
	}

	return res;
}