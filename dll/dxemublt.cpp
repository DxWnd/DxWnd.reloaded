#include <windows.h>
#include <ddraw.h>
#include <stdio.h>
#include "dxwnd.h"
#include "hddraw.h"

typedef HRESULT (WINAPI *Lock_Type)(LPDIRECTDRAWSURFACE, LPRECT, LPDIRECTDRAWSURFACE, DWORD, HANDLE);
typedef HRESULT (WINAPI *Unlock4_Type)(LPDIRECTDRAWSURFACE, LPRECT);
typedef HRESULT (WINAPI *Unlock1_Type)(LPDIRECTDRAWSURFACE, LPVOID);

extern Lock_Type pLock;
extern Unlock4_Type pUnlockMethod(LPDIRECTDRAWSURFACE);

extern DWORD PaletteEntries[256];
extern DWORD *Palette16BPP;
extern DWORD dwDDVersion;
extern DWORD dwFlags;
extern char *ExplainDDError(DWORD);
extern int Set_dwSize_From_Surface(LPDIRECTDRAWSURFACE);

// just in case ....
#define SwitchdwSize(s) s.dwSize=(s.dwSize==sizeof(DDSURFACEDESC))?sizeof(DDSURFACEDESC2):sizeof(DDSURFACEDESC)

#define DXWNDDIRECTBLITTING 1
extern Blt_Type pBlt;
EmuBlt_Type pEmuBlt;
RevBlt_Type pRevBlt;

//--------------------------------------------------------------------------------------------//
// Emulated blitting procedures: fills a 32BPP surface from the content of 8BPP or 16BPP ones.
//--------------------------------------------------------------------------------------------//

static HRESULT WINAPI EmuBlt_8_to_32(LPDIRECTDRAWSURFACE lpddsdst, LPRECT lpdestrect,
	LPDIRECTDRAWSURFACE lpddssrc, LPRECT lpsrcrect, DWORD dwflags, LPVOID lpsurface)
{
	HRESULT res;
	BYTE *src8;
	DWORD *dest;
	DDSURFACEDESC2 ddsd_src, ddsd_dst;
	long srcpitch, destpitch;
	DWORD x, y, w, h;

	w = lpdestrect->right - lpdestrect->left; 
	h = lpdestrect->bottom - lpdestrect->top; 

	memset(&ddsd_dst,0,sizeof(DDSURFACEDESC2));
	ddsd_dst.dwSize = Set_dwSize_From_Surface(lpddsdst);
	ddsd_dst.dwFlags = DDSD_LPSURFACE | DDSD_PITCH;
	if(res=(*pLock)(lpddsdst, 0, (LPDIRECTDRAWSURFACE)&ddsd_dst, DDLOCK_SURFACEMEMORYPTR|DDLOCK_WRITEONLY|DDLOCK_WAIT, 0)){	
		OutTraceE("EmuBlt8_32: Lock ERROR res=%x(%s) at %d\n", res, ExplainDDError(res), __LINE__);
		return res;
	}

	memset(&ddsd_src,0,sizeof(DDSURFACEDESC2));
	ddsd_src.dwSize = Set_dwSize_From_Surface(lpddssrc);
	ddsd_src.dwFlags = DDSD_LPSURFACE | DDSD_PITCH;
	if (lpsurface) { // already locked, just get info ....
		if(res=lpddssrc->GetSurfaceDesc((LPDDSURFACEDESC)&ddsd_src)) {
			OutTraceE("EmuBlt8_32: GetSurfaceDesc ERROR res=%x(%s) at %d\n", res, ExplainDDError(res), __LINE__);
			(*pUnlockMethod(lpddsdst))(lpddsdst, lpdestrect);
			(*pUnlockMethod(lpddssrc))(lpddssrc, lpsrcrect);
			return 0;
		}
	}
	else {
		if 	((*pLock)(lpddssrc, 0, (LPDIRECTDRAWSURFACE)&ddsd_src, DDLOCK_SURFACEMEMORYPTR|DDLOCK_READONLY, 0)) {
			(*pUnlockMethod(lpddsdst))(lpddsdst,0);
			OutTraceE("EmuBlt8_32: Lock ERROR res=%x(%s) at %d\n", res, ExplainDDError(res), __LINE__);
			return 0;
		}
		lpsurface=ddsd_src.lpSurface;
	}

	ddsd_dst.lPitch >>= 2;
	dest = (DWORD *)ddsd_dst.lpSurface;
	dest += lpdestrect->top*ddsd_dst.lPitch;
	dest += lpdestrect->left;
	destpitch = ddsd_dst.lPitch - w;

	src8 = (BYTE *)lpsurface;
	src8 += lpsrcrect->top*ddsd_src.lPitch;
	src8 += lpsrcrect->left;
	srcpitch = ddsd_src.lPitch - w;

	// OutTraceD("DEBUG: h=%d w=%d src=%x dst=%x spitch=%d dpitch=%d\n",h,w,src8,dest,srcpitch,destpitch);
	for(y = 0; y < h; y ++){
		for(x = 0; x < w; x ++){
			*(dest ++) = PaletteEntries[*(src8 ++)];
		}
		dest += destpitch;
		src8 += srcpitch;
	}

	res=(*pUnlockMethod(lpddsdst))(lpddsdst, lpdestrect);
	if (res) OutTraceE("EmuBlt8_32: Unlock ERROR dds=%x res=%x(%s) at %d\n", lpddsdst, res, ExplainDDError(res), __LINE__);
	res=(*pUnlockMethod(lpddssrc))(lpddssrc, lpsrcrect);
	if (res) OutTraceE("EmuBlt8_32: Unlock ERROR dds=%x res=%x(%s) at %d\n", lpddssrc, res, ExplainDDError(res), __LINE__);
	return res;
}


static HRESULT WINAPI EmuBlt_16_to_32(LPDIRECTDRAWSURFACE lpddsdst, LPRECT lpdestrect,
	LPDIRECTDRAWSURFACE lpddssrc, LPRECT lpsrcrect, DWORD dwflags, LPVOID lpsurface)
{
	HRESULT res;
	WORD *src16;
	DWORD *dest;
	DDSURFACEDESC2 ddsd_src, ddsd_dst;
	long srcpitch, destpitch;
	DWORD x, y, w, h;

	w = lpdestrect->right - lpdestrect->left; 
	h = lpdestrect->bottom - lpdestrect->top; 

	memset(&ddsd_dst,0,sizeof(DDSURFACEDESC2));
	ddsd_dst.dwSize = Set_dwSize_From_Surface(lpddsdst);
	ddsd_dst.dwFlags = DDSD_LPSURFACE | DDSD_PITCH;
	if(res=(*pLock)(lpddsdst, 0, (LPDIRECTDRAWSURFACE)&ddsd_dst, DDLOCK_SURFACEMEMORYPTR|DDLOCK_WRITEONLY|DDLOCK_WAIT, 0)){	
		OutTraceE("EmuBlt16_32: Lock ERROR res=%x(%s) at %d\n", res, ExplainDDError(res), __LINE__);
		return res;
	}

	memset(&ddsd_src,0,sizeof(DDSURFACEDESC2));
	ddsd_src.dwSize = Set_dwSize_From_Surface(lpddssrc);
	ddsd_src.dwFlags = DDSD_LPSURFACE | DDSD_PITCH;
	if (lpsurface) { // already locked, just get info ....
		if(res=lpddssrc->GetSurfaceDesc((LPDDSURFACEDESC)&ddsd_src)) {
			OutTraceE("EmuBlt16_32: GetSurfaceDesc ERROR res=%x(%s) at %d\n", res, ExplainDDError(res), __LINE__);
			(*pUnlockMethod(lpddsdst))(lpddsdst, lpdestrect);
			(*pUnlockMethod(lpddssrc))(lpddssrc, lpsrcrect);
			return 0;
		}
	}
	else {
		if 	(res=(*pLock)(lpddssrc, 0, (LPDIRECTDRAWSURFACE)&ddsd_src, DDLOCK_SURFACEMEMORYPTR|DDLOCK_READONLY|DDLOCK_WAIT, 0)) {
			(*pUnlockMethod(lpddsdst))(lpddsdst,0);
			OutTraceE("EmuBlt16_32: Lock ERROR res=%x(%s) at %d\n", res, ExplainDDError(res), __LINE__);
			return res;
		}
		lpsurface=ddsd_src.lpSurface;
	}

	ddsd_dst.lPitch >>= 2;
	dest = (DWORD *)ddsd_dst.lpSurface;
	dest += lpdestrect->top*ddsd_dst.lPitch;
	dest += lpdestrect->left;
	destpitch = ddsd_dst.lPitch - w;

	ddsd_src.lPitch >>= 1;
	src16 = (WORD *)(lpsurface ? lpsurface:ddsd_src.lpSurface);
	src16 += lpsrcrect->top*ddsd_src.lPitch;
	src16 += lpsrcrect->left;
	srcpitch = ddsd_src.lPitch - w;

	// OutTraceD("DEBUG: h=%d w=%d src=%x dst=%x spitch=%d dpitch=%d\n",h,w,src16,dest,srcpitch,destpitch);
	if (!Palette16BPP) { // first time through .....
		unsigned int pi;
		Palette16BPP = (DWORD *)malloc(0x10000 * sizeof(DWORD));
		if (dwFlags & USERGB565){
			for (pi=0; pi<0x10000; pi++) {
				Palette16BPP[pi]=(pi & 0x1F)<<3 | (pi & 0x7E0)<<5 | (pi & 0xF800)<<8; // RGB565
			}
		}
		else {
			for (pi=0; pi<0x10000; pi++) {
				Palette16BPP[pi]=(pi & 0x1F)<<3 | (pi & 0x3E0)<<6 | (pi & 0x7C00)<<9; // RGB555
			}
		}
#ifdef RGB655
		//default: GetPixelFormat: Flags=40(DDPF_RGB) FourCC=0 BitCount=16 RGBA=(7c00,3e0,1f,0)
		//DK2: GetPixelFormat: Flags=40(DDPF_RGB) FourCC=0 BitCount=16 RGBA=(f800,7e0,1f,0)
		for (pi=0; pi<0x10000; pi++) {
			Palette16BPP[pi]=(pi & 0x1F)<<3 | (pi & 0x3E0)<<6 | (pi & 0xFC00)<<8; // RGB655
		}
#endif
#ifdef RGB444
		for (pi=0; pi<0x10000; pi++) {
			Palette16BPP[pi]=(pi & 0x0F)<<4 | (pi & 0xF0)<<8 | (pi & 0xF00)<<12; // RGB444
		}
#endif
	}
	for(y = 0; y < h; y ++){
		for(x = 0; x < w; x ++){
			//if (!(*src16 & 0x8000)) // try implement alpha bit
			*(dest ++) = Palette16BPP[*(src16 ++)]; 
		}
		dest += destpitch;
		src16 += srcpitch;
	}

	res=(*pUnlockMethod(lpddsdst))(lpddsdst, lpdestrect);
	if (res) OutTraceE("EmuBlt16_32: Unlock ERROR dds=%x res=%x(%s) at %d\n", lpddsdst, res, ExplainDDError(res), __LINE__);
	res=(*pUnlockMethod(lpddssrc))(lpddssrc, lpsrcrect);
	if (res) OutTraceE("EmuBlt16_32: Unlock ERROR dds=%x res=%x(%s) at %d\n", lpddssrc, res, ExplainDDError(res), __LINE__);
	return res;
}

static HRESULT WINAPI EmuBlt_24_to_32(LPDIRECTDRAWSURFACE lpddsdst, LPRECT lpdestrect,
	LPDIRECTDRAWSURFACE lpddssrc, LPRECT lpsrcrect, DWORD dwflags, LPVOID lpsurface)
{
	HRESULT res;
	BYTE *src24;
	DWORD *dest;
	DDSURFACEDESC2 ddsd_src, ddsd_dst;
	long srcpitch, destpitch;
	DWORD x, y, w, h;

	w = lpdestrect->right - lpdestrect->left; 
	h = lpdestrect->bottom - lpdestrect->top; 

	memset(&ddsd_dst,0,sizeof(DDSURFACEDESC2));
	ddsd_dst.dwSize = Set_dwSize_From_Surface(lpddsdst);
	ddsd_dst.dwFlags = DDSD_LPSURFACE | DDSD_PITCH;
	if(res=(*pLock)(lpddsdst, 0, (LPDIRECTDRAWSURFACE)&ddsd_dst, DDLOCK_SURFACEMEMORYPTR|DDLOCK_WRITEONLY|DDLOCK_WAIT, 0)){	
		OutTraceE("EmuBlt24_32: Lock ERROR res=%x(%s) at %d\n", res, ExplainDDError(res), __LINE__);
		return res;
	}

	memset(&ddsd_src,0,sizeof(DDSURFACEDESC2));
	ddsd_src.dwSize = Set_dwSize_From_Surface(lpddssrc);
	ddsd_src.dwFlags = DDSD_LPSURFACE | DDSD_PITCH;
	if (lpsurface) { // already locked, just get info ....
		if(res=lpddssrc->GetSurfaceDesc((LPDDSURFACEDESC)&ddsd_src)) {
			OutTraceE("EmuBlt24_32: GetSurfaceDesc ERROR res=%x(%s) at %d\n", res, ExplainDDError(res), __LINE__);
			(*pUnlockMethod(lpddsdst))(lpddsdst, lpdestrect);
			(*pUnlockMethod(lpddssrc))(lpddssrc, lpsrcrect);
			return 0;
		}
	}
	else {
		if 	(res=(*pLock)(lpddssrc, 0, (LPDIRECTDRAWSURFACE)&ddsd_src, DDLOCK_SURFACEMEMORYPTR|DDLOCK_READONLY, 0)) {
			(*pUnlockMethod(lpddsdst))(lpddsdst,0);
			OutTraceE("EmuBlt24_32: Lock ERROR res=%x(%s) at %d\n", res, ExplainDDError(res), __LINE__);
			return res;
		}
		lpsurface=ddsd_src.lpSurface;
	}

	ddsd_dst.lPitch >>= 2;
	dest = (DWORD *)ddsd_dst.lpSurface;
	dest += lpdestrect->top*ddsd_dst.lPitch;
	dest += lpdestrect->left;
	destpitch = ddsd_dst.lPitch - w;

	src24 = (BYTE *)lpsurface;
	src24 += lpsrcrect->top*ddsd_src.lPitch;
	src24 += lpsrcrect->left;
	srcpitch = ddsd_src.lPitch - 3*w;

	for(y = 0; y < h; y ++){
		for(x = 0; x < w; x ++){
			DWORD dwPixel;
			dwPixel=0;
			memcpy((void *)&dwPixel, (void *)src24, 3);
			*(dest ++) = dwPixel;
			src24 += 3; // 3 bytes = 24 bits
		}
		dest += destpitch;
		src24 += srcpitch;
	}

	res=(*pUnlockMethod(lpddsdst))(lpddsdst, lpdestrect);
	if (res) OutTraceE("EmuBlt24_32: Unlock ERROR dds=%x res=%x(%s) at %d\n", lpddsdst, res, ExplainDDError(res), __LINE__);
	res=(*pUnlockMethod(lpddssrc))(lpddssrc, lpsrcrect);
	if (res) OutTraceE("EmuBlt24_32: Unlock ERROR dds=%x res=%x(%s) at %d\n", lpddssrc, res, ExplainDDError(res), __LINE__);
	return res;
}


static HRESULT WINAPI EmuBlt_32_to_32(LPDIRECTDRAWSURFACE lpddsdst, LPRECT lpdestrect,
	LPDIRECTDRAWSURFACE lpddssrc, LPRECT lpsrcrect, DWORD dwflags, LPVOID lpsurface)
{
#ifdef DXWNDDIRECTBLITTING
	return (*pBlt)(lpddsdst, lpdestrect, lpddssrc, lpsrcrect, dwflags, NULL);
#else
	DWORD x, y, w, h;
	long srcpitch, destpitch;
	HRESULT res;
	DWORD *src32;
	DWORD *dest;
	DDSURFACEDESC2 ddsd_src, ddsd_dst;

	w = lpdestrect->right - lpdestrect->left; 
	h = lpdestrect->bottom - lpdestrect->top; 

	memset(&ddsd_dst,0,sizeof(DDSURFACEDESC2));
	ddsd_dst.dwSize = Set_dwSize_From_Surface(lpddsdst);
	ddsd_dst.dwFlags = DDSD_LPSURFACE | DDSD_PITCH;
	if(res=(*pLock)(lpddsdst, 0, (LPDIRECTDRAWSURFACE)&ddsd_dst, DDLOCK_SURFACEMEMORYPTR|DDLOCK_WRITEONLY|DDLOCK_WAIT, 0)){	
		OutTraceE("EmuBlt32_32: Lock ERROR res=%x(%s) at %d\n", res, ExplainDDError(res), __LINE__);
		return res;
	}

	memset(&ddsd_src,0,sizeof(DDSURFACEDESC2));
	ddsd_src.dwSize = Set_dwSize_From_Surface(lpddssrc);
	ddsd_src.dwFlags = DDSD_LPSURFACE | DDSD_PITCH;
	if (lpsurface) { // already locked, just get info ....
		if(res=lpddssrc->GetSurfaceDesc((LPDDSURFACEDESC)&ddsd_src)) {
			OutTraceE("EmuBlt32_32: GetSurfaceDesc ERROR res=%x(%s) at %d\n", res, ExplainDDError(res), __LINE__);
			(*pUnlockMethod(lpddsdst))(lpddsdst, lpdestrect);
			(*pUnlockMethod(lpddssrc))(lpddssrc, lpsrcrect);
			return res;
		}
	}
	else {
		if 	((*pLock)(lpddssrc, 0, (LPDIRECTDRAWSURFACE)&ddsd_src, DDLOCK_SURFACEMEMORYPTR|DDLOCK_READONLY, 0)) {
			(*pUnlockMethod(lpddsdst))(lpddsdst,0);
			OutTraceE("EmuBlt32_32: Lock ERROR res=%x(%s) at %d\n", res, ExplainDDError(res), __LINE__);
			return res;
		}
		lpsurface=ddsd_src.lpSurface;
	}

	ddsd_dst.lPitch >>= 2;
	dest = (DWORD *)ddsd_dst.lpSurface;
	dest += lpdestrect->top*ddsd_dst.lPitch;
	dest += lpdestrect->left;
	destpitch = ddsd_dst.lPitch - w;

	ddsd_src.lPitch >>= 2;
	src32 = (DWORD *)(lpsurface ? lpsurface:ddsd_src.lpSurface);
	src32 += lpsrcrect->top*ddsd_src.lPitch;
	src32 += lpsrcrect->left;
	srcpitch = ddsd_src.lPitch - w;

	// OutTraceD("DEBUG: h=%d w=%d src=%x dst=%x spitch=%d dpitch=%d\n",h,w,src32,dest,srcpitch,destpitch);
	for(y = 0; y < h; y ++){
		for(x = 0; x < w; x ++)
			*(dest ++) = *(src32 ++);
		dest += destpitch;
		src32 += srcpitch;
	}

	res=(*pUnlockMethod(lpddsdst))(lpddsdst,lpdestrect);
	if (res) OutTraceE("EmuBlt32_32: Unlock ERROR dds=%x res=%x(%s) at %d\n", lpddsdst, res, ExplainDDError(res), __LINE__);
	res=(*pUnlockMethod(lpddssrc))(lpddssrc, lpsrcrect);
	if (res) OutTraceE("EmuBlt32_32: Unlock ERROR dds=%x res=%x(%s) at %d\n", lpddssrc, res, ExplainDDError(res), __LINE__);
	return res;
#endif
}

static HRESULT WINAPI EmuBlt_8_to_16(LPDIRECTDRAWSURFACE lpddsdst, LPRECT lpdestrect,
	LPDIRECTDRAWSURFACE lpddssrc, LPRECT lpsrcrect, DWORD dwflags, LPVOID lpsurface)
{
	HRESULT res;
	BYTE *src8;
	SHORT *dest;
	DDSURFACEDESC2 ddsd_src, ddsd_dst;
	long srcpitch, destpitch;
	DWORD x, y, w, h;

	w = lpdestrect->right - lpdestrect->left; 
	h = lpdestrect->bottom - lpdestrect->top; 

	memset(&ddsd_dst,0,sizeof(DDSURFACEDESC2));
	ddsd_dst.dwSize = Set_dwSize_From_Surface(lpddsdst);
	ddsd_dst.dwFlags = DDSD_LPSURFACE | DDSD_PITCH;
	if(res=(*pLock)(lpddsdst, 0, (LPDIRECTDRAWSURFACE)&ddsd_dst, DDLOCK_SURFACEMEMORYPTR|DDLOCK_WRITEONLY|DDLOCK_WAIT, 0)){	
		OutTraceE("EmuBlt8_16: Lock ERROR res=%x(%s) at %d\n", res, ExplainDDError(res), __LINE__);
		return res;
	}

	memset(&ddsd_src,0,sizeof(DDSURFACEDESC2));
	ddsd_src.dwSize = Set_dwSize_From_Surface(lpddssrc);
	ddsd_src.dwFlags = DDSD_LPSURFACE | DDSD_PITCH;
	if (lpsurface) { // already locked, just get info ....
		if(res=lpddssrc->GetSurfaceDesc((LPDDSURFACEDESC)&ddsd_src)) {
			OutTraceE("EmuBlt8_16: GetSurfaceDesc ERROR res=%x(%s) at %d\n", res, ExplainDDError(res), __LINE__);
			(*pUnlockMethod(lpddsdst))(lpddsdst, lpdestrect);
			(*pUnlockMethod(lpddssrc))(lpddssrc, lpsrcrect);
			return 0;
		}
	}
	else {
		if 	((*pLock)(lpddssrc, 0, (LPDIRECTDRAWSURFACE)&ddsd_src, DDLOCK_SURFACEMEMORYPTR|DDLOCK_READONLY, 0)) {
			(*pUnlockMethod(lpddsdst))(lpddsdst,0);
			OutTraceE("EmuBlt8_16: Lock ERROR res=%x(%s) at %d\n", res, ExplainDDError(res), __LINE__);
			return 0;
		}
		lpsurface=ddsd_src.lpSurface;
	}

	ddsd_dst.lPitch >>= 1;
	dest = (SHORT *)ddsd_dst.lpSurface;
	dest += lpdestrect->top*ddsd_dst.lPitch;
	dest += lpdestrect->left;
	destpitch = ddsd_dst.lPitch - w;

	src8 = (BYTE *)lpsurface;
	src8 += lpsrcrect->top*ddsd_src.lPitch;
	src8 += lpsrcrect->left;
	srcpitch = ddsd_src.lPitch - w;

	// OutTraceD("DEBUG: h=%d w=%d src=%x dst=%x spitch=%d dpitch=%d\n",h,w,src8,dest,srcpitch,destpitch);
	for(y = 0; y < h; y ++){
		for(x = 0; x < w; x ++){
			//DWORD pixel;
			//pixel = PaletteEntries[*(src8 ++)];
			//*(dest ++) = ((pixel&0x0000FF)>>3) | ((pixel&0x00FF00)>>7) | ((pixel&0xFF0000)>>19);
			*(dest ++) = (SHORT)PaletteEntries[*(src8 ++)];
		}
		dest += destpitch;
		src8 += srcpitch;
	}

	res=(*pUnlockMethod(lpddsdst))(lpddsdst, lpdestrect);
	if (res) OutTraceE("EmuBlt8_16: Unlock ERROR dds=%x res=%x(%s) at %d\n", lpddsdst, res, ExplainDDError(res), __LINE__);
	res=(*pUnlockMethod(lpddssrc))(lpddssrc, lpsrcrect);
	if (res) OutTraceE("EmuBlt8_16: Unlock ERROR dds=%x res=%x(%s) at %d\n", lpddssrc, res, ExplainDDError(res), __LINE__);
	return res;
}


static HRESULT WINAPI EmuBlt_16_to_16(LPDIRECTDRAWSURFACE lpddsdst, LPRECT lpdestrect,
	LPDIRECTDRAWSURFACE lpddssrc, LPRECT lpsrcrect, DWORD dwflags, LPVOID lpsurface)
{
#ifdef DXWNDDIRECTBLITTING
	return (*pBlt)(lpddsdst, lpdestrect, lpddssrc, lpsrcrect, dwflags, NULL);
#else
	OutTraceD("EmuBlt_16_to_16: UNSUPPORTED\n");
	return -1;
#endif
}

static HRESULT WINAPI EmuBlt_24_to_16(LPDIRECTDRAWSURFACE lpddsdst, LPRECT lpdestrect,
	LPDIRECTDRAWSURFACE lpddssrc, LPRECT lpsrcrect, DWORD dwflags, LPVOID lpsurface)
{
	HRESULT res;
	BYTE *src24;
	SHORT *dest;
	DDSURFACEDESC2 ddsd_src, ddsd_dst;
	long srcpitch, destpitch;
	DWORD x, y, w, h;

	w = lpdestrect->right - lpdestrect->left; 
	h = lpdestrect->bottom - lpdestrect->top; 

	memset(&ddsd_dst,0,sizeof(DDSURFACEDESC2));
	ddsd_dst.dwSize = Set_dwSize_From_Surface(lpddsdst);
	ddsd_dst.dwFlags = DDSD_LPSURFACE | DDSD_PITCH;
	if(res=(*pLock)(lpddsdst, 0, (LPDIRECTDRAWSURFACE)&ddsd_dst, DDLOCK_SURFACEMEMORYPTR|DDLOCK_WRITEONLY|DDLOCK_WAIT, 0)){	
		OutTraceE("EmuBlt24_16: Lock ERROR res=%x(%s) at %d\n", res, ExplainDDError(res), __LINE__);
		return res;
	}

	memset(&ddsd_src,0,sizeof(DDSURFACEDESC2));
	ddsd_src.dwSize = Set_dwSize_From_Surface(lpddssrc);
	ddsd_src.dwFlags = DDSD_LPSURFACE | DDSD_PITCH;
	if (lpsurface) { // already locked, just get info ....
		if(res=lpddssrc->GetSurfaceDesc((LPDDSURFACEDESC)&ddsd_src)) {
			OutTraceE("EmuBlt24_16: GetSurfaceDesc ERROR res=%x(%s) at %d\n", res, ExplainDDError(res), __LINE__);
			(*pUnlockMethod(lpddsdst))(lpddsdst, lpdestrect);
			(*pUnlockMethod(lpddssrc))(lpddssrc, lpsrcrect);
			return 0;
		}
	}
	else {
		if 	((*pLock)(lpddssrc, 0, (LPDIRECTDRAWSURFACE)&ddsd_src, DDLOCK_SURFACEMEMORYPTR|DDLOCK_READONLY, 0)) {
			(*pUnlockMethod(lpddsdst))(lpddsdst,0);
			OutTraceE("EmuBlt24_16: Lock ERROR res=%x(%s) at %d\n", res, ExplainDDError(res), __LINE__);
			return 0;
		}
		lpsurface=ddsd_src.lpSurface;
	}

	ddsd_dst.lPitch >>= 1;
	dest = (SHORT *)ddsd_dst.lpSurface;
	dest += lpdestrect->top*ddsd_dst.lPitch;
	dest += lpdestrect->left;
	destpitch = ddsd_dst.lPitch - w;

	src24 = (BYTE *)lpsurface;
	src24 += lpsrcrect->top*ddsd_src.lPitch;
	src24 += lpsrcrect->left;
	srcpitch = ddsd_src.lPitch - 3*w;

	for(y = 0; y < h; y ++){
		for(x = 0; x < w; x ++){
			*(dest ++) = (*(src24+2) & 0xF8)<<8 | (*(src24+1) & 0xFC)<<3 | (*(src24+0) & 0xF8)>>3;
			src24 += 3; // 3 bytes = 24 bits
		}
		dest += destpitch;
		src24 += srcpitch;
	}

	res=(*pUnlockMethod(lpddsdst))(lpddsdst, lpdestrect);
	if (res) OutTraceE("EmuBlt24_16: Unlock ERROR dds=%x res=%x(%s) at %d\n", lpddsdst, res, ExplainDDError(res), __LINE__);
	res=(*pUnlockMethod(lpddssrc))(lpddssrc, lpsrcrect);
	if (res) OutTraceE("EmuBlt24_16: Unlock ERROR dds=%x res=%x(%s) at %d\n", lpddssrc, res, ExplainDDError(res), __LINE__);
	return res;
}



static HRESULT WINAPI EmuBlt_32_to_16(LPDIRECTDRAWSURFACE lpddsdst, LPRECT lpdestrect,
	LPDIRECTDRAWSURFACE lpddssrc, LPRECT lpsrcrect, DWORD dwflags, LPVOID lpsurface)
{
	char *sMsg="EmuBlt32_16: CURRENTLY UNSUPPORTED\n";
	OutTraceE(sMsg);
	if(IsAssertEnabled) MessageBox(0, sMsg, "EmuBlt", MB_OK | MB_ICONEXCLAMATION);
	return -1;
}

static HRESULT WINAPI EmuBlt_Null(LPDIRECTDRAWSURFACE lpddsdst, LPRECT lpdestrect,
	LPDIRECTDRAWSURFACE lpddssrc, LPRECT lpsrcrect, DWORD dwflags, LPVOID lpsurface)
{
	char *sMsg="EmuBlt: undefined color depth\n";
	OutTraceE(sMsg);
	if(IsAssertEnabled) MessageBox(0, sMsg, "EmuBlt", MB_OK | MB_ICONEXCLAMATION);
	return -1;
}

//--------------------------------------------------------------------------------------------//
// Reverse blitting procedure: fills a 8BPP paletized surface from the content of a 32BPP one.
//--------------------------------------------------------------------------------------------//
#define REVPAL32SIZE 0x01000000
#define REVPAL32MASK 0x00FFFFFF
#define REVPAL16SIZE 0x00010000
#define REVPAL16MASK 0x0000FFFF

// GetMatchingPaletteEntry: retrieves the best matching palette entry close to the given
// input color crColor by using a minimum quadratic distance algorithm on each of the 3 
// RGB color components.

static int GetMatchingPaletteEntry32(DWORD crColor)
{
	int iDistance, iMinDistance;
	int iColorIndex, iComponentIndex, iMinColorIndex;
	DWORD PalColor, TargetColor;

	iMinDistance=0xFFFFFF;
	iMinColorIndex=0;

	// for each palette entry
	for(iColorIndex=0; iColorIndex<256; iColorIndex++){ 
		int iDist;
		iDistance=0;
		PalColor=PaletteEntries[iColorIndex];
		TargetColor=crColor;
		// for each of the R,G,B color components
		for (iComponentIndex=0; iComponentIndex<3; iComponentIndex++){ 
			iDist = (TargetColor & 0xFF) - (PalColor & 0xFF);
			iDist *= iDist;
			iDistance += iDist;
			PalColor >>= 8;
			TargetColor >>= 8;
		}

		if (iDistance < iMinDistance) {
			iMinDistance = iDistance;
			iMinColorIndex = iColorIndex;
		}

		if (iMinDistance==0) break; // got the perfect match!
	}
	OutTraceD("GetMatchingPaletteEntry32: color=%x matched with palette[%d]=%x dist=%d\n", 
		crColor, iMinColorIndex, PaletteEntries[iMinColorIndex], iDistance);

	return iMinColorIndex;
}

static int GetMatchingPaletteEntry16(DWORD crColor)
{
	int iDistance, iMinDistance;
	int iColorIndex, iComponentIndex, iMinColorIndex;
	DWORD PalColor, TargetColor;

	iMinDistance=0xFFFFFF;
	iMinColorIndex=0;

	// for each palette entry
	for(iColorIndex=0; iColorIndex<256; iColorIndex++){ 
		int iDist;
		iDistance=0;
		PalColor=PaletteEntries[iColorIndex];
		TargetColor=crColor;
		// for each of the R,G,B color components
		for (iComponentIndex=0; iComponentIndex<3; iComponentIndex++){ 
			iDist = (TargetColor & 0x1F) - (PalColor & 0x1F);
			iDist *= iDist;
			iDistance += iDist;
			PalColor >>= 5;
			TargetColor >>= 5;
		}

		if (iDistance < iMinDistance) {
			iMinDistance = iDistance;
			iMinColorIndex = iColorIndex;
		}

		if (iMinDistance==0) break; // got the perfect match!
	}
	OutTraceD("GetMatchingPaletteEntry: color=%x matched with palette[%d]=%x dist=%d\n", 
		crColor, iMinColorIndex, PaletteEntries[iMinColorIndex], iDistance);

	return iMinColorIndex;
}

// this is a real trick: since all 256 possible values of each PaletteRev32BPP entry are valid,
// there is no way to tell whether the entry is valid or not. In theory, you should add a separate
// status field to track this thing, but that would at least double the used space (REVPAL32SIZE bytes).
// So an alternate solution is to pretend that a particular (random) value means the entry in unassigned:
// this implies thet even if the entry is properly set, the RevBlt procedure will search for a 
// matching color anyway. So better avoid for this purpose any frequently used value: I would suggest to
// eliminate 0x00 and 0xFF. 0xF0 seems a god candidate, but any other randomly chosen value would
// probably be.

#define PALENTRY_FREE 0x1
#define PALENTRY_BUSY 0x0
//#define RANDOMENTRY 0xAA

static HRESULT WINAPI RevBlt_32_to_8(LPDIRECTDRAWSURFACE lpddsdst, LPRECT lpdestrect, LPDIRECTDRAWSURFACE lpddssrc, LPRECT lpsrcrect)
{
	HRESULT res;
	BYTE *src8;
	DWORD *dest;
	DDSURFACEDESC2 ddsd_src, ddsd_dst;
	long srcpitch, destpitch;
	DWORD x, y, w, h;
	static BYTE *PaletteRev32BPP = NULL;
	static BYTE *PaletteFlags = NULL;
	int pi;

	OutTraceD("RevBlt32_8: src=%x dst=%d\n", lpddssrc, lpddsdst);

	w = lpdestrect->right - lpdestrect->left; 
	h = lpdestrect->bottom - lpdestrect->top; 

	memset(&ddsd_dst,0,sizeof(DDSURFACEDESC2));
	ddsd_dst.dwSize = Set_dwSize_From_Surface(lpddsdst);
	ddsd_dst.dwFlags = DDSD_LPSURFACE | DDSD_PITCH;
	if(res=(*pLock)(lpddsdst, 0, (LPDIRECTDRAWSURFACE)&ddsd_dst, DDLOCK_SURFACEMEMORYPTR|DDLOCK_WRITEONLY|DDLOCK_WAIT, 0)){	
		OutTraceE("RevBlt32_8: Lock ERROR res=%x(%s) at %d\n", res, ExplainDDError(res), __LINE__);
		return res;
	}

	memset(&ddsd_src,0,sizeof(DDSURFACEDESC2));
	ddsd_src.dwSize = Set_dwSize_From_Surface(lpddssrc);
	ddsd_src.dwFlags = DDSD_LPSURFACE | DDSD_PITCH;
	if(res=(*pLock)(lpddssrc, 0, (LPDIRECTDRAWSURFACE)&ddsd_src, DDLOCK_SURFACEMEMORYPTR|DDLOCK_WRITEONLY|DDLOCK_WAIT, 0)){	
		OutTraceE("RevBlt32_8: Lock ERROR res=%x(%s) at %d\n", res, ExplainDDError(res), __LINE__);
		return res;
	}

	ddsd_dst.lPitch >>= 2;
	dest = (DWORD *)ddsd_dst.lpSurface;
	dest += lpdestrect->top*ddsd_dst.lPitch;
	dest += lpdestrect->left;
	destpitch = ddsd_dst.lPitch - w;

	src8 = (BYTE *)ddsd_src.lpSurface;
	src8 += lpsrcrect->top*ddsd_src.lPitch;
	src8 += lpsrcrect->left;
	srcpitch = ddsd_src.lPitch - w;

	// OutTraceD("DEBUG: h=%d w=%d src=%x dst=%x spitch=%d dpitch=%d\n",h,w,src8,dest,srcpitch,destpitch);
	if (!PaletteRev32BPP) { // first time through .....
		PaletteRev32BPP = (BYTE *)malloc(REVPAL32SIZE);
		PaletteFlags = (BYTE *)malloc(REVPAL32SIZE);
	}

	extern BOOL isPaletteUpdated;
	if (isPaletteUpdated){
		isPaletteUpdated=FALSE; // do this just once per palette
		memset(PaletteFlags, PALENTRY_FREE, REVPAL32SIZE); // necessary in build->release mode
		for (pi=0; pi<256; pi++){
			int RevIndex=PaletteEntries[pi] & REVPAL32MASK;
			PaletteRev32BPP[RevIndex]=pi;
			PaletteFlags[RevIndex]=PALENTRY_BUSY;
		}
	}

	for(y = 0; y < h; y ++){
		for(x = 0; x < w; x ++){
			int RevIndex = *dest++ & REVPAL32MASK;
			if(PaletteFlags[RevIndex]==PALENTRY_FREE){
				PaletteRev32BPP[RevIndex]=GetMatchingPaletteEntry32(RevIndex);
				PaletteFlags[RevIndex]=PALENTRY_BUSY;
			}
			*(src8 ++)= (BYTE)PaletteRev32BPP[RevIndex];
		}
		dest += destpitch;
		src8 += srcpitch;
	}

	res=(*pUnlockMethod(lpddsdst))(lpddsdst,lpdestrect);
	if (res) OutTraceE("RevBlt32_8: Unlock ERROR dds=%x res=%x(%s) at %d\n", lpddsdst, res, ExplainDDError(res), __LINE__);
	res=(*pUnlockMethod(lpddssrc))(lpddssrc, lpsrcrect);
	if (res) OutTraceE("RevBlt32_8: Unlock ERROR dds=%x res=%x(%s) at %d\n", lpddssrc, res, ExplainDDError(res), __LINE__);
	return res;
}

static HRESULT WINAPI RevBlt_32_to_16(LPDIRECTDRAWSURFACE lpddsdst, LPRECT lpdestrect, LPDIRECTDRAWSURFACE lpddssrc, LPRECT lpsrcrect)
{
	HRESULT res;
	WORD *src16;
	DWORD *dest;
	DDSURFACEDESC2 ddsd_src, ddsd_dst;
	long srcpitch, destpitch;
	DWORD x, y, w, h;

	OutTraceD("RevBlt32_16: src=%x dst=%d\n", lpddssrc, lpddsdst);

	w = lpdestrect->right - lpdestrect->left; 
	h = lpdestrect->bottom - lpdestrect->top; 

	memset(&ddsd_dst,0,sizeof(DDSURFACEDESC2));
	ddsd_dst.dwSize = Set_dwSize_From_Surface(lpddsdst);
	ddsd_dst.dwFlags = DDSD_LPSURFACE | DDSD_PITCH;
	if(res=(*pLock)(lpddsdst, 0, (LPDIRECTDRAWSURFACE)&ddsd_dst, DDLOCK_SURFACEMEMORYPTR|DDLOCK_WRITEONLY|DDLOCK_WAIT, 0)){	
		OutTraceE("RevBlt32_16: Lock ERROR res=%x(%s) at %d\n", res, ExplainDDError(res), __LINE__);
		return res;
	}

	memset(&ddsd_src,0,sizeof(DDSURFACEDESC2));
	ddsd_src.dwSize = Set_dwSize_From_Surface(lpddssrc);
	ddsd_src.dwFlags = DDSD_LPSURFACE | DDSD_PITCH;
	if(res=(*pLock)(lpddssrc, 0, (LPDIRECTDRAWSURFACE)&ddsd_src, DDLOCK_SURFACEMEMORYPTR|DDLOCK_WRITEONLY|DDLOCK_WAIT, 0)){	
		OutTraceE("RevBlt32_16: Lock ERROR res=%x(%s) at %d\n", res, ExplainDDError(res), __LINE__);
		return res;
	}

	ddsd_dst.lPitch >>= 2;
	dest = (DWORD *)ddsd_dst.lpSurface;
	dest += lpdestrect->top*ddsd_dst.lPitch;
	dest += lpdestrect->left;
	destpitch = ddsd_dst.lPitch - w;

	ddsd_src.lPitch >>= 1;
	src16 = (WORD *)ddsd_src.lpSurface;
	src16 += lpsrcrect->top*ddsd_src.lPitch;
	src16 += lpsrcrect->left;
	srcpitch = ddsd_src.lPitch - w;

	for(y = 0; y < h; y ++){
		for(x = 0; x < w; x ++){
			*(src16 ++)= (WORD)*(dest++) & 0xFFFF; // wrong, but visible.....
		}
		dest += destpitch;
		src16 += srcpitch;
	}

	res=(*pUnlockMethod(lpddsdst))(lpddsdst, lpdestrect);
	if (res) OutTraceE("RevBlt32_16: Unlock ERROR dds=%x res=%x(%s) at %d\n", lpddsdst, res, ExplainDDError(res), __LINE__);
	res=(*pUnlockMethod(lpddssrc))(lpddssrc, lpsrcrect);
	if (res) OutTraceE("RevBlt32_16: Unlock ERROR dds=%x res=%x(%s) at %d\n", lpddssrc, res, ExplainDDError(res), __LINE__);
	return res;
}

static HRESULT WINAPI RevBlt_16_to_8(LPDIRECTDRAWSURFACE lpddsdst, LPRECT lpdestrect, LPDIRECTDRAWSURFACE lpddssrc, LPRECT lpsrcrect)
{
	HRESULT res;
	BYTE *src8;
	USHORT *dest16;
	DDSURFACEDESC2 ddsd_src, ddsd_dst;
	long srcpitch, destpitch;
	DWORD x, y, w, h;
	static BYTE *PaletteRev16BPP = NULL;
	static BYTE *PaletteFlags = NULL;
	int pi;

	OutTraceD("RevBlt16_8: src=%x dst=%d\n", lpddssrc, lpddsdst);

	w = lpdestrect->right - lpdestrect->left; 
	h = lpdestrect->bottom - lpdestrect->top; 

	memset(&ddsd_dst,0,sizeof(DDSURFACEDESC2));
	ddsd_dst.dwSize = Set_dwSize_From_Surface(lpddsdst);
	ddsd_dst.dwFlags = DDSD_LPSURFACE | DDSD_PITCH;
	if(res=(*pLock)(lpddsdst, 0, (LPDIRECTDRAWSURFACE)&ddsd_dst, DDLOCK_SURFACEMEMORYPTR|DDLOCK_WRITEONLY|DDLOCK_WAIT, 0)){	
		OutTraceE("RevBlt16_8: Lock ERROR res=%x(%s) at %d\n", res, ExplainDDError(res), __LINE__);
		return res;
	}

	memset(&ddsd_src,0,sizeof(DDSURFACEDESC2));
	ddsd_src.dwSize = Set_dwSize_From_Surface(lpddssrc);
	ddsd_src.dwFlags = DDSD_LPSURFACE | DDSD_PITCH;
	if(res=(*pLock)(lpddssrc, 0, (LPDIRECTDRAWSURFACE)&ddsd_src, DDLOCK_SURFACEMEMORYPTR|DDLOCK_WRITEONLY|DDLOCK_WAIT, 0)){	
		OutTraceE("RevBlt16_8: Lock ERROR res=%x(%s) at %d\n", res, ExplainDDError(res), __LINE__);
		return res;
	}

	ddsd_dst.lPitch >>= 1;
	dest16 = (USHORT *)ddsd_dst.lpSurface;
	dest16 += lpdestrect->top*ddsd_dst.lPitch;
	dest16 += lpdestrect->left;
	destpitch = ddsd_dst.lPitch - w;

	src8 = (BYTE *)ddsd_src.lpSurface;
	src8 += lpsrcrect->top*ddsd_src.lPitch;
	src8 += lpsrcrect->left;
	srcpitch = ddsd_src.lPitch - w;

	// OutTraceD("DEBUG: h=%d w=%d src=%x dst=%x spitch=%d dpitch=%d\n",h,w,src8,dest,srcpitch,destpitch);
	if (!PaletteRev16BPP) { // first time through .....
		PaletteRev16BPP = (BYTE *)malloc(REVPAL16SIZE);
		PaletteFlags = (BYTE *)malloc(REVPAL16SIZE);
	}

	extern BOOL isPaletteUpdated;
	if (isPaletteUpdated){
		isPaletteUpdated=FALSE; // do this just once per palette
		memset(PaletteRev16BPP, PALENTRY_FREE, REVPAL16SIZE); // necessary in build->release mode
		for (pi=0; pi<256; pi++){
			int RevIndex=PaletteEntries[pi] & REVPAL16MASK;
			PaletteRev16BPP[RevIndex]=pi;
			PaletteFlags[RevIndex]=PALENTRY_BUSY;
		}
	}

	for(y = 0; y < h; y ++){
		for(x = 0; x < w; x ++){
			int RevIndex = *dest16++ & REVPAL16MASK;
			if(PaletteFlags[RevIndex]==PALENTRY_FREE){
				PaletteRev16BPP[RevIndex]=GetMatchingPaletteEntry16(RevIndex);
				PaletteFlags[RevIndex]=PALENTRY_BUSY;
			}
			*(src8 ++)= (BYTE)PaletteRev16BPP[RevIndex];
		}
		dest16 += destpitch;
		src8 += srcpitch;
	}

	res=(*pUnlockMethod(lpddsdst))(lpddsdst, lpdestrect);
	if (res) OutTraceE("RevBlt16_8: Unlock ERROR dds=%x res=%x(%s) at %d\n", lpddsdst, res, ExplainDDError(res), __LINE__);
	res=(*pUnlockMethod(lpddssrc))(lpddssrc, lpsrcrect);
	if (res) OutTraceE("RevBlt16_8: Unlock ERROR dds=%x res=%x(%s) at %d\n", lpddssrc, res, ExplainDDError(res), __LINE__);
	return res;
}

static HRESULT WINAPI RevBlt_16_to_16(LPDIRECTDRAWSURFACE lpddsdst, LPRECT lpdestrect, LPDIRECTDRAWSURFACE lpddssrc, LPRECT lpsrcrect)
{
	return (*pBlt)(lpddsdst, lpdestrect, lpddssrc, lpsrcrect, 0, NULL);
}

static HRESULT WINAPI RevBlt_Null(LPDIRECTDRAWSURFACE lpddsdst, LPRECT lpdestrect, LPDIRECTDRAWSURFACE lpddssrc, LPRECT lpsrcrect)
{
	OutTraceE("EmuBlt: undefined color depth\n");
	return -1;
}

//--------------------------------------------------------------------------------------------//
// exported methods
//--------------------------------------------------------------------------------------------//

void SetBltTransformations()
{
	OutTraceD("SetBltTransformations: color transformation %d->%d\n", 
		VirtualScr.PixelFormat.dwRGBBitCount, ActualScr.PixelFormat.dwRGBBitCount);

	/* default (bad) setting */
	pRevBlt=RevBlt_Null;
	pEmuBlt=EmuBlt_Null;

	switch (ActualScr.PixelFormat.dwRGBBitCount){
	case 32:
		switch(VirtualScr.PixelFormat.dwRGBBitCount){
		case 8:
			pRevBlt=RevBlt_32_to_8; 
			pEmuBlt=EmuBlt_8_to_32;
			OutTraceD("set color transformation 8<->32\n");
			break;
		case 16: 
			pRevBlt=RevBlt_32_to_16;
			pEmuBlt=EmuBlt_16_to_32;
			OutTraceD("set color transformation 16<->32\n");
			break;
		case 24: 
			//pRevBlt=RevBlt_24_to_32;
			pEmuBlt=EmuBlt_24_to_32;
			OutTraceD("set color transformation 24->32\n");
			break;
		case 32: 
			pEmuBlt=EmuBlt_32_to_32;
			OutTraceD("set color transformation 32->32\n");
			break;
		default:
			OutTraceD("unsupported color transformation %d->32\n", VirtualScr.PixelFormat.dwRGBBitCount);
			break;
		}
		break;
	case 16:
		switch(VirtualScr.PixelFormat.dwRGBBitCount){
		case 8:
			pRevBlt=RevBlt_16_to_8;
			pEmuBlt=EmuBlt_8_to_16;
			OutTraceD("set color transformation 8<->16\n");
			break;
		case 16:
			pRevBlt=RevBlt_16_to_16;
			pEmuBlt=EmuBlt_16_to_16;
			OutTraceD("set color transformation 16<->16\n");
			break;
		case 24: 
			//pRevBlt=RevBlt_24_to_16;
			pEmuBlt=EmuBlt_24_to_16;
			OutTraceD("set color transformation 24<->16\n");
			break;
		case 32:
			pEmuBlt=EmuBlt_32_to_16;
			break;
		default:
			OutTraceD("unsupported color transformation %d->16\n", VirtualScr.PixelFormat.dwRGBBitCount);
			break;
		}
		break;
	default:
		OutTraceD("unsupported color transformation %d->%d\n",
			VirtualScr.PixelFormat.dwRGBBitCount,
			ActualScr.PixelFormat.dwRGBBitCount);
		break;
	}
	return;
}
