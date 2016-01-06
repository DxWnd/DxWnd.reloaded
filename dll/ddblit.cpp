#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <ddraw.h>
#include "dxwnd.h"
#include "dxwcore.hpp"
#include "stdio.h" 
#include "hddraw.h"
#include "dxhelper.h"

#define FIXBIGGERRECT 1

extern LPDIRECTDRAWSURFACE lpDDSEmu_Prim;
extern LPDIRECTDRAW lpPrimaryDD;
extern Blt_Type pBlt;
extern ReleaseS_Type pReleaseSMethod(int);
extern CreateSurface1_Type pCreateSurface1;
extern CreateSurface1_Type pCreateSurface2;
extern CreateSurface1_Type pCreateSurface3;
extern CreateSurface2_Type pCreateSurface4;
extern CreateSurface2_Type pCreateSurface7;
extern Unlock4_Type pUnlockMethod(int);
extern HDC hFlippedDC;
extern BOOL bFlippedDC;
extern ReleaseDC_Type pReleaseDC1;
extern ReleaseDC_Type pReleaseDC2;
extern ReleaseDC_Type pReleaseDC3;
extern ReleaseDC_Type pReleaseDC4;
extern ReleaseDC_Type pReleaseDC7;

extern void BlitError(HRESULT, LPRECT, LPRECT, int);
extern void BlitTrace(char *, LPRECT, LPRECT, int);
extern void DescribeSurface(LPDIRECTDRAWSURFACE, int, char *, int);
extern void TextureHandling(LPDIRECTDRAWSURFACE, int);
extern GetSurfaceDesc2_Type pGetSurfaceDescMethod();
extern GetSurfaceDesc2_Type GetSurfaceDescMethod();
extern Blt_Type pBltMethod();

static HRESULT sBltNoPrimary(int dxversion, Blt_Type pBlt, char *api, LPDIRECTDRAWSURFACE lpdds, LPRECT lpdestrect,
	LPDIRECTDRAWSURFACE lpddssrc, LPRECT lpsrcrect, DWORD dwflags, LPDDBLTFX lpddbltfx)
{
	RECT srcrect;
	HRESULT res;
	BOOL FromScreen;
	//extern PrimaryBlt_Type pPrimaryBlt;
	//CkArg arg;

	FromScreen=dxwss.IsAPrimarySurface(lpddssrc); 

	// make a working copy of srcrect if not NULL
	if (lpsrcrect){
		srcrect=*lpsrcrect;
	}
	// when blitting from a primary surface on screen (that is in non emulated mode), correct offsets
	// You should take account also for scaled primary surfaces, but that would be a hard task: 
	// a reduced primary surface (in not-emulated mode) would bring quality loss!!!
	// v2.1.83: BLITFROMBACKBUFFER mode, let you chose to blit from backbuffer, where the surface size
	// is fixed no matter how the window/primary surface is scaled. 
	// In "The Sims" there is no quality loss, but some scrolling artifact.
	//if(lpsrcrect && FromScreen){
	if(FromScreen){
		if ((dxw.dwFlags1 & EMULATESURFACE) || (dxw.dwFlags1 & EMULATEBUFFER)){
			if(dxw.dwFlags1 & BLITFROMBACKBUFFER){
				LPDIRECTDRAWSURFACE lpDDSBack;
				lpDDSBack = dxwss.GetBackBufferSurface();
				if(lpDDSBack) lpddssrc=lpDDSBack;
			}
		}
		else {
			if(dxw.dwFlags1 & BLITFROMBACKBUFFER){
				LPDIRECTDRAWSURFACE lpDDSBack;
				lpDDSBack = dxwss.GetBackBufferSurface();
				if(lpDDSBack) lpddssrc=lpDDSBack;
			}
			else{
				srcrect=dxw.MapWindowRect(lpsrcrect);
			}
		}
	}

	if (IsDebug) BlitTrace("NOPRIM", lpsrcrect, lpdestrect, __LINE__);
	res= (*pBlt)(lpdds, lpdestrect, lpddssrc, lpsrcrect ? &srcrect : NULL, dwflags, lpddbltfx);
	// Blitting compressed data may work to screen surfaces only. In this case, it may be worth
	// trying blitting directly to lpDDSEmu_Prim: it makes DK2 intro movies working.
	// Wrong guess!!! The cause was not compression, but simply a pixelformat mismatch. Better
	// configure things properly and avoid this branch.
	switch(res){
	// commented out: it was the cause of the "Divine Divinity" flickering.
	// commented in?: it seems useful in "Axis and Allies"....
	//case DDERR_UNSUPPORTED:
	//	if (dxw.dwFlags1 & EMULATESURFACE){
	//		RECT targetrect;
	//		if (IsDebug) BlitTrace("UNSUPP", lpsrcrect ? &srcrect : NULL, lpdestrect, __LINE__);
	//		targetrect = dxw.MapWindowRect(lpdestrect);
	//		res=(*pBlt)(lpDDSEmu_Prim, &targetrect, lpddssrc, lpsrcrect ? &srcrect : NULL, dwflags, lpddbltfx);
	//	}
	//	break;
	case DDERR_SURFACEBUSY:
		(*pUnlockMethod(dxversion))(lpdds, NULL);
		if (lpddssrc) (*pUnlockMethod(dxversion))(lpddssrc, NULL);	
		if (IsDebug) BlitTrace("BUSY", lpsrcrect ? &srcrect : NULL, lpdestrect, __LINE__);
		res=(*pBlt)(lpdds, lpdestrect, lpddssrc, lpsrcrect ? &srcrect : NULL, dwflags|DDBLT_WAIT, lpddbltfx);
		break;
	default:
		break;
	}
	if (res) BlitError(res, &srcrect, lpdestrect, __LINE__);
	if(IsDebug) {
		DescribeSurface(lpdds, 0, "[DST]" , __LINE__);
		if (lpddssrc) DescribeSurface(lpddssrc, 0, "[SRC]" , __LINE__); // lpddssrc could be NULL!!!
	}
	if(dxw.dwFlags1 & SUPPRESSDXERRORS) res=0;
	if(dxw.dwFlags5 & TEXTUREMASK) {
		// Texture Handling on Blt
		TextureHandling(lpdds, dxversion);
	}
	return res;
}

static HRESULT sBltToPrimary(int dxversion, Blt_Type pBlt, char *api, LPDIRECTDRAWSURFACE lpdds, LPRECT lpdestrect,
	LPDIRECTDRAWSURFACE lpddssrc, LPRECT lpsrcrect, DWORD dwflags, LPDDBLTFX lpddbltfx, BOOL isFlipping)
{
	HRESULT res;
	RECT destrect, emurect;
	extern PrimaryBlt_Type pPrimaryBlt;

	// debug suppressions
	if(isFlipping){
		if(dxw.dwFlags3 & NODDRAWFLIP) return DD_OK;
	}
	else {
		if(dxw.dwFlags3 & NODDRAWBLT) return DD_OK;
	}

#ifdef ONEPIXELFIX
	if (lpdestrect){
		if ((lpdestrect->top == 0) && (lpdestrect->bottom == dxw.GetScreenHeight() -1)) lpdestrect->bottom = dxw.GetScreenHeight();
		if ((lpdestrect->left == 0) && (lpdestrect->right  == dxw.GetScreenWidth()  -1)) lpdestrect->right  = dxw.GetScreenWidth();
	}
	if (lpsrcrect){
		if ((lpsrcrect->top == 0) && (lpsrcrect->bottom == dxw.GetScreenHeight() -1)) lpsrcrect->bottom = dxw.GetScreenHeight();
		if ((lpsrcrect->left == 0) && (lpsrcrect->right  == dxw.GetScreenWidth()  -1)) lpsrcrect->right  = dxw.GetScreenWidth();
	}
#endif

#if FIXBIGGERRECT
	if(lpdestrect){
		if((DWORD)lpdestrect->top < 0) lpdestrect->top = 0;
		if((DWORD)lpdestrect->left < 0) lpdestrect->left = 0;
		if((DWORD)lpdestrect->bottom > dxw.GetScreenHeight()) lpdestrect->bottom = dxw.GetScreenHeight();
		if((DWORD)lpdestrect->right > dxw.GetScreenWidth()) lpdestrect->right = dxw.GetScreenWidth();
	}
#endif

	if(dxw.dwFlags5 & QUARTERBLT){
		BOOL QuarterUpdate;
		QuarterUpdate = lpdestrect ? 
			(((lpdestrect->bottom - lpdestrect->top) * (lpdestrect->right - lpdestrect->left)) > ((LONG)(dxw.GetScreenHeight() * dxw.GetScreenWidth()) >> 2)) 
			: 
			TRUE;
		if(QuarterUpdate) if(dxw.HandleFPS()) return DD_OK;
	}
	else
		if(dxw.HandleFPS()) return DD_OK;
	if(dxw.dwFlags5 & NOBLT) return DD_OK;
	
	destrect=dxw.MapWindowRect(lpdestrect);
	OutTraceB("DESTRECT=(%d,%d)-(%d,%d) Screen=(%dx%d)\n", 
		destrect.left, destrect.top, destrect.right, destrect.bottom,
		dxw.GetScreenWidth(), dxw.GetScreenHeight());

	// v2.03.48: on WinXP it may happen (reported by Cloudstr) that alt tabbing produces
	// bad blit attempts where the client coordinates get the (-32000,-32000) - (-32000,-32000)
	// value. In such cases, it's adviseable to simulate an OK return code without attempting
	// any blit operation!
	if(destrect.left == -32000) return DD_OK; // no blit on invisible window

	//if(!(lpddssrc || (dwflags & DDBLT_COLORFILL))) {
	if((lpddssrc==0) && !(dwflags & DDBLT_COLORFILL)){
		lpddssrc = dxwss.GetBackBufferSurface();
		OutTraceDW("Flip: setting flip chain to lpdds=%x\n", lpddssrc);
	}

	// =========================
	// Blit to primary direct surface 
	// =========================

	if(!(dxw.dwFlags1 & (EMULATESURFACE|EMULATEBUFFER))){ 
		res=DD_OK;

		// blit only when source and dest surface are different. Should make ScreenRefresh faster.
		if (lpdds != lpddssrc) {
			dxw.ShowOverlay(lpddssrc);
			if (IsDebug) BlitTrace("PRIM-NOEMU", lpsrcrect, &destrect, __LINE__);
			res=(*pPrimaryBlt)(dxversion, pBlt, lpdds, &destrect, lpddssrc, lpsrcrect);
		}
		if(res){
			BlitError(res, lpsrcrect, &destrect, __LINE__);
			if(IsDebug) {
				DescribeSurface(lpdds, 0, "[DST]" , __LINE__);
				if (lpddssrc) DescribeSurface(lpddssrc, 0, "[SRC]" , __LINE__); // lpddssrc could be NULL!!!
			}
			// Try to handle HDC lock concurrency....		
			if(res==DDERR_SURFACEBUSY){
				(*pUnlockMethod(dxversion))(lpdds, NULL);
				if(lpddssrc) (*pUnlockMethod(dxversion))(lpdds, NULL);
				if (IsDebug) BlitTrace("BUSY", lpsrcrect, &destrect, __LINE__);
				res= (*pBlt)(lpdds, &destrect, lpddssrc, lpsrcrect, dwflags, lpddbltfx);
				if (res) BlitError(res, lpsrcrect, &destrect, __LINE__);
			}
			// Try to handle DDBLT_KEYSRC on primary surface
			if((res==DDERR_INVALIDPARAMS) && (dwflags & DDBLT_KEYSRC)){
				// to do: handle possible situations with surface 2 / 4 / 7 types
				DDSURFACEDESC2 ddsd;
				LPDIRECTDRAWSURFACE2 lpddsTmp;
				extern CreateSurface2_Type pCreateSurfaceMethod(int);
				if (IsDebug) BlitTrace("KEYSRC", lpsrcrect, &destrect, __LINE__);
				memset(&ddsd, 0, sizeof(ddsd));
				ddsd.dwSize = (dxversion < 4) ? sizeof(DDSURFACEDESC) : sizeof(DDSURFACEDESC2);
				(*pGetSurfaceDescMethod())((LPDIRECTDRAWSURFACE2)lpddssrc, &ddsd);
				res=(*pCreateSurfaceMethod(dxversion))(lpPrimaryDD, &ddsd, (LPDIRECTDRAWSURFACE *)&lpddsTmp, NULL);
				if(res) OutTraceE("CreateSurface: ERROR %x(%s) at %d", res, ExplainDDError(res), __LINE__);
				// copy background
				res= (*pBlt)((LPDIRECTDRAWSURFACE)lpddsTmp, lpsrcrect, lpdds, &destrect, DDBLT_WAIT, NULL);
				if(res) OutTraceE("Blt: ERROR %x(%s) at %d", res, ExplainDDError(res), __LINE__);
				// overlay texture
				res= (*pBlt)((LPDIRECTDRAWSURFACE)lpddsTmp, lpsrcrect, lpddssrc, lpsrcrect, dwflags, lpddbltfx);
				if(res) OutTraceE("Blt: ERROR %x(%s) at %d", res, ExplainDDError(res), __LINE__);
				// copy back to destination
				res= (*pBlt)(lpdds, &destrect, (LPDIRECTDRAWSURFACE)lpddsTmp, lpsrcrect, DDBLT_WAIT, lpddbltfx);
				if(res) OutTraceE("Blt: ERROR %x(%s) at %d", res, ExplainDDError(res), __LINE__);
				if (res) BlitError(res, lpsrcrect, &destrect, __LINE__);
				(*pReleaseSMethod(dxversion))((LPDIRECTDRAWSURFACE)lpddsTmp);
			}
			if(dxw.dwFlags1 & SUPPRESSDXERRORS) res=DD_OK;
		}

		return res;
	}

	// ... else blitting on emulated surface
	
	// =========================
	// Blit/Flip to emulated primary surface
	// =========================

	if(dxw.dwFlags5 & GDIMODE){
		extern void BlitToWindow(HWND, LPDIRECTDRAWSURFACE);
		//if (lpdds != lpddssrc) 
			BlitToWindow(dxw.GethWnd(), lpddssrc);
		return DD_OK;
	}

	if (lpdestrect){
		emurect=*lpdestrect;
	}
	else{
		// emurect: emulated rect is full surface (dwWidth x dwHeight)
		emurect.left = 0;
		emurect.top = 0;
		emurect.right = dxw.GetScreenWidth();
		emurect.bottom = dxw.GetScreenHeight();
	}

	res=DD_OK;
	// blit only when source and dest surface are different. Should make ScreenRefresh faster.
	if (lpdds != lpddssrc){
		if (IsDebug) BlitTrace("SRC2EMU", &emurect, &destrect, __LINE__);
		if(destrect.top == -32000) return DD_OK; // happens when window is minimized & do not notify on task switch ...
	    if(lpdds->IsLost()) lpdds->Restore(); // lpDDSEmu_Back could get lost .....
		res=(*pBlt)(lpdds, &emurect, lpddssrc, lpsrcrect, dwflags, lpddbltfx);
	}

	if (res) {
		BlitError(res, lpsrcrect, &emurect, __LINE__);
		DescribeSurface(lpdds,    0, "[DST]" , __LINE__);
		if (lpddssrc) DescribeSurface(lpddssrc, 0, "[SRC]" , __LINE__); // lpddssrc could be NULL!!!
		/* 
		Dungeon Keeper II intro movies bug ....
		it seems that you can't blit from compressed or different surfaces in memory,
		while the operation COULD be supported to video. As a mater of fact, it DOES
		work on my PC. The error code is DDERR_UNSUPPORTED.
		v2.02.98: The same thing happens with "New York Racer", but with DDERR_EXCEPTION error code.
		V2.03.15: The same thing happens with "Silent Hunter III", but with DDERR_INVALIDRECT error code.
		*/
		if((res==DDERR_UNSUPPORTED) || (res==DDERR_EXCEPTION) || (res==DDERR_INVALIDRECT)){
			//RECT targetrect;
			dxw.ShowOverlay(lpddssrc);
			if (IsDebug) BlitTrace("UNSUPP", &emurect, &destrect, __LINE__);
			//targetrect=dxw.MapWindowRect(&destrect); // v2.03.18
			//res=(*pBlt)(lpDDSEmu_Prim, &targetrect, lpddssrc, lpsrcrect, dwflags, lpddbltfx);
			res=(*pBlt)(lpDDSEmu_Prim, &destrect, lpddssrc, lpsrcrect, dwflags, lpddbltfx);
			if (res) BlitError(res, lpsrcrect, &destrect, __LINE__);
		}

		// Try to handle HDC lock concurrency....		
		if(res==DDERR_SURFACEBUSY){
			if (bFlippedDC) {
				ReleaseDC_Type pReleaseDC;
				switch(dxversion){
					case 1: pReleaseDC=pReleaseDC1; break;
					case 2: pReleaseDC=pReleaseDC2; break;
					case 3: pReleaseDC=pReleaseDC3; break;
					case 4: pReleaseDC=pReleaseDC4; break;
					case 7: pReleaseDC=pReleaseDC7; break;
				}
				(*pReleaseDC)(lpdds, hFlippedDC);
			}

			// v2.03.49: resumed because if fixes locked surfaces on "Red Alert 1" on WinXP as reported by cloudstr 
			if(lpddssrc) { // lpddssrc could be NULL!!!
			res=(*pUnlockMethod(dxversion))(lpddssrc, NULL);
			if(res && (res!=DDERR_NOTLOCKED)) OutTraceE("Unlock ERROR: lpdds=%x err=%x(%s)\n", lpddssrc, res, ExplainDDError(res));
			}
			res=(*pUnlockMethod(dxversion))(lpdds, NULL); // v2.03.24 reintroduced because of "Virtua Cop"
			if(res && (res!=DDERR_NOTLOCKED)) OutTraceE("Unlock ERROR: lpdds=%x err=%x(%s)\n", lpdds, res, ExplainDDError(res));

			if (IsDebug) BlitTrace("BUSY", &emurect, &destrect, __LINE__);
			res=(*pBlt)(lpdds, &emurect, lpddssrc, lpsrcrect, dwflags, lpddbltfx);
			if (res) BlitError(res, lpsrcrect, &destrect, __LINE__);
		}

		if(dxw.dwFlags1 & SUPPRESSDXERRORS) res=DD_OK;
		return res;
	}

	LPDIRECTDRAWSURFACE lpDDSSource;
	if (res=(*pColorConversion)(dxversion, lpdds, emurect, &lpDDSSource)) {
		OutTraceE("sBlt ERROR: Color conversion failed res=%x(%s)\n", res, ExplainDDError(res));
		if(dxw.dwFlags1 & SUPPRESSDXERRORS) res=DD_OK;
		return res;
	}

	if(lpDDSEmu_Prim->IsLost()) lpDDSEmu_Prim->Restore();

	dxw.ShowOverlay(lpDDSSource);
	if(dxw.dwFlags4 & BILINEAR2XFILTER){
		emurect.right <<= 1;
		emurect.bottom <<= 1;
	}
	if (IsDebug) BlitTrace("BACK2PRIM", &emurect, &destrect, __LINE__);
	res=(*pPrimaryBlt)(dxversion, pBlt, lpDDSEmu_Prim, &destrect, lpDDSSource, &emurect);

	if (res) BlitError(res, &emurect, &destrect, __LINE__);
	if(dxw.dwFlags1 & SUPPRESSDXERRORS) res=DD_OK;
	if (IsDebug) OutTrace("%s: done ret=%x at %d\n", api, res, __LINE__);
	return res;
}

HRESULT WINAPI sBlt(int dxversion, Blt_Type pBlt, char *api, LPDIRECTDRAWSURFACE lpdds, LPRECT lpdestrect,
	LPDIRECTDRAWSURFACE lpddssrc, LPRECT lpsrcrect, DWORD dwflags, LPDDBLTFX lpddbltfx, BOOL isFlipping)
{
	POINT p = {0, 0};
	HRESULT res;
	BOOL ToPrim, FromPrim, ToScreen, FromScreen;

	if(dxw.dwFlags5 & MESSAGEPUMP){
		MSG msg;
		while(PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE)){
			OutTraceW("MESSAGEPUMP: msg=%x l-wParam=(%x,%x)\n", msg.message, msg.lParam, msg.wParam);
			if((msg.message >= WM_KEYFIRST) && (msg.message <= WM_KEYLAST)) break; // do not consume keyboard inputs
			if((msg.message >= WM_MOUSEFIRST) && (msg.message <= WM_MOUSELAST)) break; // do not consume mouse inputs
			PeekMessage(&msg, NULL, 0, 0, PM_REMOVE);
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	ToPrim=dxwss.IsAPrimarySurface(lpdds);
	FromPrim=dxwss.IsAPrimarySurface(lpddssrc);
	ToScreen=ToPrim && !(dxw.dwFlags1 & EMULATESURFACE);
	FromScreen=FromPrim && !(dxw.dwFlags1 & EMULATESURFACE) && !(dxw.dwFlags1 & EMULATEBUFFER); // v2.02.77

	// log
	if(IsTraceDW){
		char sLog[256];
		char sInfo[128];
		sprintf(sLog, "%s: dest=%x%s src=%x%s dwFlags=%x(%s)",
			api, lpdds, (ToPrim ? "(PRIM)":""), lpddssrc, (FromPrim ? "(PRIM)":""), dwflags, ExplainBltFlags(dwflags));
		if (lpdestrect)
			sprintf(sInfo, " destrect=(%d,%d)-(%d,%d)", lpdestrect->left, lpdestrect->top, lpdestrect->right, lpdestrect->bottom);
		else
			sprintf(sInfo, " destrect=(NULL)");
		strcat(sLog, sInfo);
		if (lpsrcrect)
			sprintf(sInfo, " srcrect=(%d,%d)-(%d,%d)", lpsrcrect->left, lpsrcrect->top, lpsrcrect->right, lpsrcrect->bottom);
		else
			sprintf(sInfo, " srcrect=(NULL)");
		strcat(sLog, sInfo);
		if(lpddbltfx){
			if (dwflags & DDBLT_COLORFILL){
				sprintf(sInfo, " ddbltfx.FillColor=%x", lpddbltfx->dwFillColor); 
				strcat(sLog, sInfo);
			}
			if (dwflags & DDBLT_KEYDESTOVERRIDE){
				sprintf(sInfo, " ddbltfx.DestColorkey=%x", lpddbltfx->ddckDestColorkey); 
				strcat(sLog, sInfo);
			}
			if (dwflags & DDBLT_KEYSRCOVERRIDE){
				sprintf(sInfo, " ddbltfx.SrcColorkey=%x", lpddbltfx->ddckSrcColorkey); 
				strcat(sLog, sInfo);
			}
			if (dwflags & DDBLT_ROP){
				sprintf(sInfo, " ddbltfx.ROP=%x", lpddbltfx->dwROP);
				strcat(sLog, sInfo);
			}
			if (dwflags & DDBLT_DEPTHFILL){
				sprintf(sInfo, " ddbltfx.FillDepth=%x", lpddbltfx->dwFillDepth);
				strcat(sLog, sInfo);
			}
		}
		strcat(sLog,"\n");
		OutTrace(sLog);
	}

	if(ToPrim)
		res = sBltToPrimary(dxversion, pBlt, api, lpdds, lpdestrect, lpddssrc, lpsrcrect, dwflags, lpddbltfx, isFlipping);
	else
		res = sBltNoPrimary(dxversion, pBlt, api, lpdds, lpdestrect, lpddssrc, lpsrcrect, dwflags, lpddbltfx);

	return res;
}