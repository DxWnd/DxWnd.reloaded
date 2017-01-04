#include <windows.h>
#include <ddraw.h>
#include <stdio.h>
#include "dxwnd.h"
#include "dxwcore.hpp"
#include "hddraw.h"

//#define USEOPENMP

typedef HRESULT (WINAPI *Lock_Type)(LPDIRECTDRAWSURFACE, LPRECT, LPDDSURFACEDESC, DWORD, HANDLE);
typedef HRESULT (WINAPI *Unlock4_Type)(LPDIRECTDRAWSURFACE, LPRECT);
typedef HRESULT (WINAPI *Unlock1_Type)(LPDIRECTDRAWSURFACE, LPVOID);

extern Lock_Type pLockMethod(int);
extern Unlock4_Type pUnlockMethod(int);

extern DWORD PaletteEntries[256];
extern DWORD *Palette16BPP;
extern char *ExplainDDError(DWORD);

// just in case ....
#define SwitchdwSize(s) s.dwSize=(s.dwSize==sizeof(DDSURFACEDESC))?sizeof(DDSURFACEDESC2):sizeof(DDSURFACEDESC)

#define MARKBLITCOLOR32 0x00FFFF00
#define MARKBLITCOLOR16 0x0FF0
EmuBlt_Type pEmuBlt;
PrimaryBlt_Type pPrimaryBlt;

extern Blt_Type pBlt;

//--------------------------------------------------------------------------------------------//
// Emulated blitting procedures: fills a 32BPP surface from the content of 8BPP or 16BPP ones.
//--------------------------------------------------------------------------------------------//

static void MarkRect32(DWORD *dest, int w, int h, int destpitch)
{
	int x, y;

	for(x = 0; x < w; x ++) *(dest ++) = MARKBLITCOLOR32;
	if (h<3) return;
	dest += destpitch;
	for(y = 0; y < h-2; y ++){
		*dest = MARKBLITCOLOR32;
		dest += w-1;
		*dest = MARKBLITCOLOR32;
		dest += destpitch + 1;
	}
	for(x = 0; x < w; x ++) *(dest ++) = MARKBLITCOLOR32;	
	return;
}

static void MarkRect16(SHORT *dest, int w, int h, int destpitch)
{
	int x, y;

	for(x = 0; x < w; x ++) *(dest ++) = MARKBLITCOLOR16;	
	if (h<3) return;
	dest += destpitch;
	for(y = 0; y < h-2; y ++){
		*dest = MARKBLITCOLOR16;
		dest += w-1;
		*dest = MARKBLITCOLOR16;
		dest += destpitch + 1;
	}
	for(x = 0; x < w; x ++) *(dest ++) = MARKBLITCOLOR16;	
	return;
}

static DWORD Melt32(DWORD c1, DWORD c2)
{
	DWORD ret;
	ret = 
		((((c1 & 0x0000FF) + (c2 & 0x0000FF)) >> 1) & 0x0000FF) |
		((((c1 & 0x00FF00) + (c2 & 0x00FF00)) >> 1) & 0x00FF00) |
		((((c1 & 0xFF0000) + (c2 & 0xFF0000)) >> 1) & 0xFF0000);
	return ret;
}

static DWORD Melt16_555(DWORD c1, DWORD c2)
{
	DWORD ret;
	ret = 
		((((c1 & 0x00001F) + (c2 & 0x00001F)) >> 1) & 0x00001F) |
		((((c1 & 0x0003E0) + (c2 & 0x0003E0)) >> 1) & 0x0003E0) |
		((((c1 & 0x007C00) + (c2 & 0x007C00)) >> 1) & 0x007C00);
	return ret;
}

static DWORD Melt16_565(DWORD c1, DWORD c2)
{
	DWORD ret;
	ret = 
		((((c1 & 0x00001F) + (c2 & 0x00001F)) >> 1) & 0x00001F) |
		((((c1 & 0x0007E0) + (c2 & 0x0007E0)) >> 1) & 0x0007E0) |
		((((c1 & 0x00F800) + (c2 & 0x00F800)) >> 1) & 0x00F800);
	return ret;
}

void SetPalette16BPP()
{
// OutTraceDW("DEBUG: h=%d w=%d src=%x dst=%x spitch=%d dpitch=%d\n",h,w,src16,dest,srcpitch,destpitch);
	unsigned int pi;
	Palette16BPP = (DWORD *)malloc(0x10000 * sizeof(DWORD));
	if (dxw.dwFlags3 & BLACKWHITE){
		// actually, it should be like this: R/G/B = (red * 0.30) + (green * 0.59) + (blue * 0.11) 
		// (http://www.codeproject.com/Articles/66253/Converting-Colors-to-Gray-Shades)
		DWORD grey;
		if (dxw.dwFlags1 & USERGB565){
			for (pi=0; pi<0x10000; pi++) {
				//grey = ((((pi & 0x1F)<<3) + ((pi & 0x7E0)>>3) + ((pi & 0xF800)>>8)) / 3) & 0xFF;
				grey = (((((pi & 0x1F)<<3) * 30) + (((pi & 0x7E0)>>3) * 59) + (((pi & 0xF800)>>8) * 11)) / 100) & 0xFF;
				Palette16BPP[pi] = (grey) + (grey<<8) + (grey<<16);				
			}
		}
		else {
			for (pi=0; pi<0x10000; pi++) {
				//grey = ((((pi & 0x1F)<<3) + ((pi & 0x3E0)>>2) + ((pi & 0x7C00)>>7)) / 3) & 0xFF;
				grey = (((((pi & 0x1F)<<3) * 30) + (((pi & 0x3E0)>>2) * 59) + (((pi & 0x7C00)>>7) * 11)) / 100) & 0xFF;
				Palette16BPP[pi] = grey + (grey<<8) + (grey<<16);
			}
		}
	}
	else {
		if (dxw.dwFlags1 & USERGB565){
			for (pi=0; pi<0x10000; pi++) {
				Palette16BPP[pi]=(pi & 0x1F)<<3 | (pi & 0x7E0)<<5 | (pi & 0xF800)<<8; // RGB565
			}
		}
		else {
			for (pi=0; pi<0x10000; pi++) {
				Palette16BPP[pi]=(pi & 0x1F)<<3 | (pi & 0x3E0)<<6 | (pi & 0x7C00)<<9; // RGB555
			}
		}
	}
}

static HRESULT WINAPI EmuBlt_8_to_32(int dxversion, Blt_Type pBlt, LPDIRECTDRAWSURFACE lpddsdst, LPRECT lpdestrect,
	LPDIRECTDRAWSURFACE lpddssrc, LPRECT lpsrcrect, DWORD dwflags, LPVOID lpsurface)
{
	HRESULT res;
	BYTE *src8;
	DWORD *dest, *dest0;
	DDSURFACEDESC2 ddsd_src, ddsd_dst;
	long srcpitch, destpitch;
	int x, y, w, h;

	w = lpdestrect->right - lpdestrect->left; 
	h = lpdestrect->bottom - lpdestrect->top; 

	memset(&ddsd_dst,0,sizeof(DDSURFACEDESC2));
	ddsd_dst.dwSize = Set_dwSize_From_Surface();
	ddsd_dst.dwFlags = DDSD_LPSURFACE | DDSD_PITCH;
	if(res=(*pLockMethod(dxversion))(lpddsdst, 0, (LPDDSURFACEDESC)&ddsd_dst, DDLOCK_SURFACEMEMORYPTR|DDLOCK_WRITEONLY|DDLOCK_WAIT, 0)){	
		OutTraceE("EmuBlt8_32: Lock ERROR res=%x(%s) at %d\n", res, ExplainDDError(res), __LINE__);
		return res;
	}

	memset(&ddsd_src,0,sizeof(DDSURFACEDESC2));
	ddsd_src.dwSize = Set_dwSize_From_Surface();
	ddsd_src.dwFlags = DDSD_LPSURFACE | DDSD_PITCH;
	if (lpsurface) { // already locked, just get info ....
		if(res=lpddssrc->GetSurfaceDesc((LPDDSURFACEDESC)&ddsd_src)) {
			OutTraceE("EmuBlt8_32: GetSurfaceDesc ERROR res=%x(%s) at %d\n", res, ExplainDDError(res), __LINE__);
			(*pUnlockMethod(dxversion))(lpddsdst, lpdestrect);
			(*pUnlockMethod(dxversion))(lpddssrc, lpsrcrect);
			return 0;
		}
	}
	else {
		if(res=(*pLockMethod(dxversion))(lpddssrc, 0, (LPDDSURFACEDESC)&ddsd_src, DDLOCK_SURFACEMEMORYPTR|DDLOCK_READONLY, 0)) {
			OutTraceE("EmuBlt8_32: Lock ERROR res=%x(%s) at %d\n", res, ExplainDDError(res), __LINE__);
			(*pUnlockMethod(dxversion))(lpddsdst, 0);
			(*pUnlockMethod(dxversion))(lpddssrc, 0);
			return 0;
		}
		lpsurface=ddsd_src.lpSurface;
	}

	ddsd_dst.lPitch >>= 2;
	dest = (DWORD *)ddsd_dst.lpSurface;
	dest += lpdestrect->top*ddsd_dst.lPitch;
	dest += lpdestrect->left;
	destpitch = ddsd_dst.lPitch - w;
	dest0 = dest;

	src8 = (BYTE *)lpsurface;
	src8 += lpsrcrect->top*ddsd_src.lPitch;
	src8 += lpsrcrect->left;
	srcpitch = ddsd_src.lPitch - w;

	// OutTraceDW("DEBUG: h=%d w=%d src=%x dst=%x spitch=%d dpitch=%d\n",h,w,src8,dest,srcpitch,destpitch);
	for(y = 0; y < h; y ++){
		for(x = 0; x < w; x ++){
			*(dest ++) = PaletteEntries[*(src8 ++)];
		}
		dest += destpitch;
		src8 += srcpitch;
	}

	if(dxw.dwFlags3 & MARKBLIT) MarkRect32(dest0, w, h, destpitch);

	res=(*pUnlockMethod(dxversion))(lpddsdst, lpdestrect);
	if (res) OutTraceE("EmuBlt8_32: Unlock ERROR dds=%x res=%x(%s) at %d\n", lpddsdst, res, ExplainDDError(res), __LINE__);
	res=(*pUnlockMethod(dxversion))(lpddssrc, lpsrcrect);
	if (res) OutTraceE("EmuBlt8_32: Unlock ERROR dds=%x res=%x(%s) at %d\n", lpddssrc, res, ExplainDDError(res), __LINE__);
	return res;
}

static HRESULT WINAPI BilinearBlt_8_to_32(int dxversion, Blt_Type pBlt, LPDIRECTDRAWSURFACE lpddsdst, LPRECT lpdestrect,
	LPDIRECTDRAWSURFACE lpddssrc, LPRECT lpsrcrect, DWORD dwflags, LPVOID lpsurface)
{
	HRESULT res;
	BYTE *src8, *src0;
	DWORD *dest, *dest0;
	DDSURFACEDESC2 ddsd_src, ddsd_dst;
	long srcpitch, destpitch;
	int w, h;

	w = lpdestrect->right - lpdestrect->left; 
	h = lpdestrect->bottom - lpdestrect->top; 

	lpdestrect->left <<= 1;
	lpdestrect->top <<= 1;
	lpdestrect->right <<= 1;
	lpdestrect->bottom <<= 1;

	memset(&ddsd_dst,0,sizeof(DDSURFACEDESC2));
	ddsd_dst.dwSize = Set_dwSize_From_Surface();
	ddsd_dst.dwFlags = DDSD_LPSURFACE | DDSD_PITCH;
	if(res=(*pLockMethod(dxversion))(lpddsdst, 0, (LPDDSURFACEDESC)&ddsd_dst, DDLOCK_SURFACEMEMORYPTR|DDLOCK_WRITEONLY|DDLOCK_WAIT, 0)){	
		OutTraceE("BilBlt8_32: Lock ERROR res=%x(%s) at %d\n", res, ExplainDDError(res), __LINE__);
		return res;
	}

	memset(&ddsd_src,0,sizeof(DDSURFACEDESC2));
	ddsd_src.dwSize = Set_dwSize_From_Surface();
	ddsd_src.dwFlags = DDSD_LPSURFACE | DDSD_PITCH;
	if (lpsurface) { // already locked, just get info ....
		if(res=lpddssrc->GetSurfaceDesc((LPDDSURFACEDESC)&ddsd_src)) {
			OutTraceE("BilBlt8_32: GetSurfaceDesc ERROR res=%x(%s) at %d\n", res, ExplainDDError(res), __LINE__);
			(*pUnlockMethod(dxversion))(lpddsdst, lpdestrect);
			(*pUnlockMethod(dxversion))(lpddssrc, lpsrcrect);
			return 0;
		}
	}
	else {
		if(res=(*pLockMethod(dxversion))(lpddssrc, 0, (LPDDSURFACEDESC)&ddsd_src, DDLOCK_SURFACEMEMORYPTR|DDLOCK_READONLY, 0)) {
			OutTraceE("BilBlt8_32: Lock ERROR res=%x(%s) at %d\n", res, ExplainDDError(res), __LINE__);
			(*pUnlockMethod(dxversion))(lpddsdst,0);
			(*pUnlockMethod(dxversion))(lpddssrc, 0);
			return 0;
		}
		lpsurface=ddsd_src.lpSurface;
	}

	ddsd_dst.lPitch >>= 2;
	dest = (DWORD *)ddsd_dst.lpSurface;
	dest += lpdestrect->top*ddsd_dst.lPitch;
	dest += lpdestrect->left;
	destpitch = ddsd_dst.lPitch - (2 * w);
	dest0 = dest;

	src8 = (BYTE *)lpsurface;
	src8 += (lpsrcrect->top >> 1)*ddsd_src.lPitch;
	src8 += (lpsrcrect->left >> 1);
	srcpitch = ddsd_src.lPitch - w;
	src0 = src8;

	// OutTraceDW("DEBUG: h=%d w=%d src=%x dst=%x spitch=%d dpitch=%d\n",h,w,src8,dest,srcpitch,destpitch);
#ifdef USEOPENMP
#pragma omp parallel for schedule(static)
#endif
	for(int y = 0; y < h; y ++){ 
		register DWORD Q1, Q2, Q3, Q4, Q5;
		src8 = src0 + (y * ddsd_src.lPitch);
		dest = dest0 + ((y * ddsd_dst.lPitch) << 1);
		Q5 = Melt32(PaletteEntries[*(src8)], PaletteEntries[*(src8+ddsd_src.lPitch)]);
        for(int x = 0; x < w; x ++){
			Q1 = PaletteEntries[*(src8)];
			Q2 = Melt32(Q1, PaletteEntries[*(src8+1)]);
			Q3 = Q5;
			if(y == h-1){
				Q3 = Q1;
				Q4 = Q2;
			}
			else{
				Q5 = Melt32(PaletteEntries[*(src8+1)], PaletteEntries[*(src8+ddsd_src.lPitch+1)]); // to be used in next for cycle
				Q4 = Melt32(Q3, Q5);
			}

			*(dest) = Q1;
			*(dest+1) = Q2;
			*(dest+ddsd_dst.lPitch) = Q3;
			*(dest+ddsd_dst.lPitch+1) = Q4;
			src8++;
			dest+=2;
        }
	}

	if(dxw.dwFlags3 & MARKBLIT) MarkRect32(dest0, 2*w, 2*h, destpitch);

	res=(*pUnlockMethod(dxversion))(lpddsdst, lpdestrect);
	if (res) OutTraceE("BilBlt8_32: Unlock ERROR dds=%x res=%x(%s) at %d\n", lpddsdst, res, ExplainDDError(res), __LINE__);
	res=(*pUnlockMethod(dxversion))(lpddssrc, lpsrcrect);
	if (res) OutTraceE("BilBlt8_32: Unlock ERROR dds=%x res=%x(%s) at %d\n", lpddssrc, res, ExplainDDError(res), __LINE__);
	return res;
}

static HRESULT WINAPI EmuBlt_16_to_32(int dxversion, Blt_Type pBlt, LPDIRECTDRAWSURFACE lpddsdst, LPRECT lpdestrect,
	LPDIRECTDRAWSURFACE lpddssrc, LPRECT lpsrcrect, DWORD dwflags, LPVOID lpsurface)
{
	HRESULT res;
	WORD *src16;
	DWORD *dest, *dest0;
	DDSURFACEDESC2 ddsd_src, ddsd_dst;
	long srcpitch, destpitch;
	DWORD x, y, w, h;

	w = lpdestrect->right - lpdestrect->left; 
	h = lpdestrect->bottom - lpdestrect->top; 

	memset(&ddsd_dst,0,sizeof(DDSURFACEDESC2));
	ddsd_dst.dwSize = Set_dwSize_From_Surface();
	ddsd_dst.dwFlags = DDSD_LPSURFACE | DDSD_PITCH;
	if(res=(*pLockMethod(dxversion))(lpddsdst, 0, (LPDDSURFACEDESC)&ddsd_dst, DDLOCK_SURFACEMEMORYPTR|DDLOCK_WRITEONLY|DDLOCK_WAIT, 0)){	
		OutTraceE("EmuBlt16_32: Lock ERROR res=%x(%s) at %d\n", res, ExplainDDError(res), __LINE__);
		return res;
	}

	memset(&ddsd_src,0,sizeof(DDSURFACEDESC2));
	ddsd_src.dwSize = Set_dwSize_From_Surface();
	ddsd_src.dwFlags = DDSD_LPSURFACE | DDSD_PITCH;
	if (lpsurface) { // already locked, just get info ....
		if(res=lpddssrc->GetSurfaceDesc((LPDDSURFACEDESC)&ddsd_src)) {
			OutTraceE("EmuBlt16_32: GetSurfaceDesc ERROR res=%x(%s) at %d\n", res, ExplainDDError(res), __LINE__);
			(*pUnlockMethod(dxversion))(lpddsdst, lpdestrect);
			(*pUnlockMethod(dxversion))(lpddssrc, lpsrcrect);
			return 0;
		}
	}
	else {
		if(res=(*pLockMethod(dxversion))(lpddssrc, 0, (LPDDSURFACEDESC)&ddsd_src, DDLOCK_SURFACEMEMORYPTR|DDLOCK_READONLY|DDLOCK_WAIT, 0)) {
			OutTraceE("EmuBlt16_32: Lock ERROR res=%x(%s) at %d\n", res, ExplainDDError(res), __LINE__);
			(*pUnlockMethod(dxversion))(lpddsdst,0);
			(*pUnlockMethod(dxversion))(lpddssrc, 0);
			return res;
		}
		lpsurface=ddsd_src.lpSurface;
	}

	ddsd_dst.lPitch >>= 2;
	dest = (DWORD *)ddsd_dst.lpSurface;
	dest += lpdestrect->top*ddsd_dst.lPitch;
	dest += lpdestrect->left;
	destpitch = ddsd_dst.lPitch - w;
	dest0 = dest;

	ddsd_src.lPitch >>= 1;
	src16 = (WORD *)(lpsurface ? lpsurface:ddsd_src.lpSurface);
	src16 += lpsrcrect->top*ddsd_src.lPitch;
	src16 += lpsrcrect->left;
	srcpitch = ddsd_src.lPitch - w;

	if (!Palette16BPP) SetPalette16BPP();

	for(y = 0; y < h; y ++){
		for(x = 0; x < w; x ++){
			//if (!(*src16 & 0x8000)) // try implement alpha bit
			*(dest ++) = Palette16BPP[*(src16 ++)]; 
		}
		dest += destpitch;
		src16 += srcpitch;
	}

	if(dxw.dwFlags3 & MARKBLIT) MarkRect32(dest0, w, h, destpitch);

	res=(*pUnlockMethod(dxversion))(lpddsdst, lpdestrect);
	if (res) OutTraceE("EmuBlt16_32: Unlock ERROR dds=%x res=%x(%s) at %d\n", lpddsdst, res, ExplainDDError(res), __LINE__);
	res=(*pUnlockMethod(dxversion))(lpddssrc, lpsrcrect);
	if (res) OutTraceE("EmuBlt16_32: Unlock ERROR dds=%x res=%x(%s) at %d\n", lpddssrc, res, ExplainDDError(res), __LINE__);
	return res;
}

static HRESULT WINAPI Deinterlace_16_to_32(int dxversion, Blt_Type pBlt, LPDIRECTDRAWSURFACE lpddsdst, LPRECT lpdestrect,
	LPDIRECTDRAWSURFACE lpddssrc, LPRECT lpsrcrect, DWORD dwflags, LPVOID lpsurface)
{
	HRESULT res;
	WORD *src16;
	DWORD *dest, *dest0;
	DDSURFACEDESC2 ddsd_src, ddsd_dst;
	long srcpitch, destpitch;
	DWORD x, y, w, h;

	w = lpdestrect->right - lpdestrect->left; 
	h = lpdestrect->bottom - lpdestrect->top; 

	memset(&ddsd_dst,0,sizeof(DDSURFACEDESC2));
	ddsd_dst.dwSize = Set_dwSize_From_Surface();
	ddsd_dst.dwFlags = DDSD_LPSURFACE | DDSD_PITCH;
	if(res=(*pLockMethod(dxversion))(lpddsdst, 0, (LPDDSURFACEDESC)&ddsd_dst, DDLOCK_SURFACEMEMORYPTR|DDLOCK_WRITEONLY|DDLOCK_WAIT, 0)){	
		OutTraceE("EmuBlt16_32: Lock ERROR res=%x(%s) at %d\n", res, ExplainDDError(res), __LINE__);
		return res;
	}

	memset(&ddsd_src,0,sizeof(DDSURFACEDESC2));
	ddsd_src.dwSize = Set_dwSize_From_Surface();
	ddsd_src.dwFlags = DDSD_LPSURFACE | DDSD_PITCH;
	if (lpsurface) { // already locked, just get info ....
		if(res=lpddssrc->GetSurfaceDesc((LPDDSURFACEDESC)&ddsd_src)) {
			OutTraceE("EmuBlt16_32: GetSurfaceDesc ERROR res=%x(%s) at %d\n", res, ExplainDDError(res), __LINE__);
			(*pUnlockMethod(dxversion))(lpddsdst, lpdestrect);
			(*pUnlockMethod(dxversion))(lpddssrc, lpsrcrect);
			return 0;
		}
	}
	else {
		if(res=(*pLockMethod(dxversion))(lpddssrc, 0, (LPDDSURFACEDESC)&ddsd_src, DDLOCK_SURFACEMEMORYPTR|DDLOCK_READONLY|DDLOCK_WAIT, 0)) {
			OutTraceE("EmuBlt16_32: Lock ERROR res=%x(%s) at %d\n", res, ExplainDDError(res), __LINE__);
			(*pUnlockMethod(dxversion))(lpddsdst,0);
			(*pUnlockMethod(dxversion))(lpddssrc, 0);
			return res;
		}
		lpsurface=ddsd_src.lpSurface;
	}

	ddsd_dst.lPitch >>= 2;
	dest = (DWORD *)ddsd_dst.lpSurface;
	dest += lpdestrect->top*ddsd_dst.lPitch;
	dest += lpdestrect->left;
	destpitch = ddsd_dst.lPitch - w;
	dest0 = dest;

	ddsd_src.lPitch >>= 1;
	src16 = (WORD *)(lpsurface ? lpsurface:ddsd_src.lpSurface);
	src16 += lpsrcrect->top*ddsd_src.lPitch;
	src16 += lpsrcrect->left;
	srcpitch = ddsd_src.lPitch - w;

	if (!Palette16BPP) SetPalette16BPP();

	BOOL allblack = TRUE;
	WORD *src016 = src16;
	y = ((h >> 2) << 1) + 1;
	for(x = 0; x < w; x ++)
		if(Palette16BPP[*((src16 ++) + (y * ddsd_src.lPitch))]) allblack = FALSE; 
	dest = dest0;
	src16 = src016;
	if(allblack){
		for(y = 0; y < h; y += 2){
			for(x = 0; x < w; x ++){
				*(dest ++) = Palette16BPP[*(src16 ++)]; 
			}
			dest += destpitch+ddsd_dst.lPitch;
			src16 += srcpitch+ddsd_src.lPitch;
		}
		dest = dest0 + ddsd_dst.lPitch;
		src16 = src016 + ddsd_src.lPitch;
		for(y = 1; y < h; y += 2){
			for(x = 0; x < w; x ++){
				*(dest ++) = (*(dest + ddsd_dst.lPitch) >> 1) + (*(dest - ddsd_dst.lPitch) >> 1) ; 
			}
			dest += destpitch+ddsd_dst.lPitch;
			src16 += srcpitch+ddsd_src.lPitch;
		}
	}
	else {
		for(y = 0; y < h; y ++){
			for(x = 0; x < w; x ++){
				//if (!(*src16 & 0x8000)) // try implement alpha bit
				*(dest ++) = Palette16BPP[*(src16 ++)]; 
			}
			dest += destpitch;
			src16 += srcpitch;
		}
	}

	if(dxw.dwFlags3 & MARKBLIT) MarkRect32(dest0, w, h, destpitch);

	res=(*pUnlockMethod(dxversion))(lpddsdst, lpdestrect);
	if (res) OutTraceE("EmuBlt16_32: Unlock ERROR dds=%x res=%x(%s) at %d\n", lpddsdst, res, ExplainDDError(res), __LINE__);
	res=(*pUnlockMethod(dxversion))(lpddssrc, lpsrcrect);
	if (res) OutTraceE("EmuBlt16_32: Unlock ERROR dds=%x res=%x(%s) at %d\n", lpddssrc, res, ExplainDDError(res), __LINE__);
	return res;
}

static HRESULT WINAPI BilinearBlt_16_to_32(int dxversion, Blt_Type pBlt, LPDIRECTDRAWSURFACE lpddsdst, LPRECT lpdestrect,
	LPDIRECTDRAWSURFACE lpddssrc, LPRECT lpsrcrect, DWORD dwflags, LPVOID lpsurface)
{
	HRESULT res;
	WORD *src16, *src0;
	DWORD *dest, *dest0;
	DDSURFACEDESC2 ddsd_src, ddsd_dst;
	long srcpitch, destpitch;
	int w, h;

	w = lpdestrect->right - lpdestrect->left; 
	h = lpdestrect->bottom - lpdestrect->top; 

	lpdestrect->left <<= 1;
	lpdestrect->top <<= 1;
	lpdestrect->right <<= 1;
	lpdestrect->bottom <<= 1;

	memset(&ddsd_dst,0,sizeof(DDSURFACEDESC2));
	ddsd_dst.dwSize = Set_dwSize_From_Surface();
	ddsd_dst.dwFlags = DDSD_LPSURFACE | DDSD_PITCH;
	if(res=(*pLockMethod(dxversion))(lpddsdst, 0, (LPDDSURFACEDESC)&ddsd_dst, DDLOCK_SURFACEMEMORYPTR|DDLOCK_WRITEONLY|DDLOCK_WAIT, 0)){	
		OutTraceE("BilBlt16_32: Lock ERROR res=%x(%s) at %d\n", res, ExplainDDError(res), __LINE__);
		return res;
	}

	memset(&ddsd_src,0,sizeof(DDSURFACEDESC2));
	ddsd_src.dwSize = Set_dwSize_From_Surface();
	ddsd_src.dwFlags = DDSD_LPSURFACE | DDSD_PITCH;
	if (lpsurface) { // already locked, just get info ....
		if(res=lpddssrc->GetSurfaceDesc((LPDDSURFACEDESC)&ddsd_src)) {
			OutTraceE("BilBlt16_32: GetSurfaceDesc ERROR res=%x(%s) at %d\n", res, ExplainDDError(res), __LINE__);
			(*pUnlockMethod(dxversion))(lpddsdst, lpdestrect);
			(*pUnlockMethod(dxversion))(lpddssrc, lpsrcrect);
			return 0;
		}
	}
	else {
		if(res=(*pLockMethod(dxversion))(lpddssrc, 0, (LPDDSURFACEDESC)&ddsd_src, DDLOCK_SURFACEMEMORYPTR|DDLOCK_READONLY|DDLOCK_WAIT, 0)) {
			OutTraceE("BilBlt16_32: Lock ERROR res=%x(%s) at %d\n", res, ExplainDDError(res), __LINE__);
			(*pUnlockMethod(dxversion))(lpddsdst,0);
			(*pUnlockMethod(dxversion))(lpddssrc, 0);
			return res;
		}
		lpsurface=ddsd_src.lpSurface;
	}

	ddsd_dst.lPitch >>= 2;
	dest = (DWORD *)ddsd_dst.lpSurface;
	dest += lpdestrect->top*ddsd_dst.lPitch;
	dest += lpdestrect->left;
	destpitch = ddsd_dst.lPitch - (2 * w);
	dest0 = dest;

	ddsd_src.lPitch >>= 1;
	src16 = (WORD *)(lpsurface ? lpsurface:ddsd_src.lpSurface);
	src16 += (lpsrcrect->top >> 1)*ddsd_src.lPitch;
	src16 += (lpsrcrect->left >> 1);
	srcpitch = ddsd_src.lPitch - w;
	src0 = src16;

	if (!Palette16BPP) SetPalette16BPP();

	// OutTraceDW("DEBUG: h=%d w=%d src=%x dst=%x spitch=%d dpitch=%d\n",h,w,src8,dest,srcpitch,destpitch);
#ifdef USEOPENMP
#pragma omp parallel for schedule(static)
#endif
    for(int y = 0; y < h; y ++){ 
		register DWORD Q1, Q2, Q3, Q4, Q5;
		src16 = src0 + (y * ddsd_src.lPitch);
		dest = dest0 + ((y * ddsd_dst.lPitch) << 1);
		Q5 = Melt32(Palette16BPP[*(src16)], Palette16BPP[*(src16+ddsd_src.lPitch)]);
        for(int x = 0; x < w; x ++){
			Q1 = Palette16BPP[*(src16)];
			Q2 = Melt32(Q1, Palette16BPP[*(src16+1)]);
			Q3 = Q5;
			if(y == h-1){
				Q3 = Q1;
				Q4 = Q2;
			}
			else{
				Q5 = Melt32(Palette16BPP[*(src16+1)], Palette16BPP[*(src16+ddsd_src.lPitch+1)]); // to be used in next for cycle
				Q4 = Melt32(Q3, Q5);
			}
			*(dest) = Q1;
			*(dest+1) = Q2;
			*(dest+ddsd_dst.lPitch) = Q3;
			*(dest+ddsd_dst.lPitch+1) = Q4;
			src16++;
			dest+=2;
        }
	}

	if(dxw.dwFlags3 & MARKBLIT) MarkRect32(dest0, 2*w, 2*h, destpitch);

	res=(*pUnlockMethod(dxversion))(lpddsdst, lpdestrect);
	if (res) OutTraceE("BilBlt16_32: Unlock ERROR dds=%x res=%x(%s) at %d\n", lpddsdst, res, ExplainDDError(res), __LINE__);
	res=(*pUnlockMethod(dxversion))(lpddssrc, lpsrcrect);
	if (res) OutTraceE("BilBlt16_32: Unlock ERROR dds=%x res=%x(%s) at %d\n", lpddssrc, res, ExplainDDError(res), __LINE__);
	return res;
}

static HRESULT WINAPI EmuBlt_24_to_32(int dxversion, Blt_Type pBlt, LPDIRECTDRAWSURFACE lpddsdst, LPRECT lpdestrect,
	LPDIRECTDRAWSURFACE lpddssrc, LPRECT lpsrcrect, DWORD dwflags, LPVOID lpsurface)
{
	HRESULT res;
	BYTE *src24;
	DWORD *dest, *dest0;
	DDSURFACEDESC2 ddsd_src, ddsd_dst;
	long srcpitch, destpitch;
	DWORD x, y, w, h;

	w = lpdestrect->right - lpdestrect->left; 
	h = lpdestrect->bottom - lpdestrect->top; 

	memset(&ddsd_dst,0,sizeof(DDSURFACEDESC2));
	ddsd_dst.dwSize = Set_dwSize_From_Surface();
	ddsd_dst.dwFlags = DDSD_LPSURFACE | DDSD_PITCH;
	if(res=(*pLockMethod(dxversion))(lpddsdst, 0, (LPDDSURFACEDESC)&ddsd_dst, DDLOCK_SURFACEMEMORYPTR|DDLOCK_WRITEONLY|DDLOCK_WAIT, 0)){	
		OutTraceE("EmuBlt24_32: Lock ERROR res=%x(%s) at %d\n", res, ExplainDDError(res), __LINE__);
		return res;
	}

	memset(&ddsd_src,0,sizeof(DDSURFACEDESC2));
	ddsd_src.dwSize = Set_dwSize_From_Surface();
	ddsd_src.dwFlags = DDSD_LPSURFACE | DDSD_PITCH;
	if (lpsurface) { // already locked, just get info ....
		if(res=lpddssrc->GetSurfaceDesc((LPDDSURFACEDESC)&ddsd_src)) {
			OutTraceE("EmuBlt24_32: GetSurfaceDesc ERROR res=%x(%s) at %d\n", res, ExplainDDError(res), __LINE__);
			(*pUnlockMethod(dxversion))(lpddsdst, lpdestrect);
			(*pUnlockMethod(dxversion))(lpddssrc, lpsrcrect);
			return 0;
		}
	}
	else {
		if(res=(*pLockMethod(dxversion))(lpddssrc, 0, (LPDDSURFACEDESC)&ddsd_src, DDLOCK_SURFACEMEMORYPTR|DDLOCK_READONLY, 0)) {
			OutTraceE("EmuBlt24_32: Lock ERROR res=%x(%s) at %d\n", res, ExplainDDError(res), __LINE__);
			(*pUnlockMethod(dxversion))(lpddsdst,0);
			(*pUnlockMethod(dxversion))(lpddssrc, 0);
			return res;
		}
		lpsurface=ddsd_src.lpSurface;
	}

	ddsd_dst.lPitch >>= 2;
	dest = (DWORD *)ddsd_dst.lpSurface;
	dest += lpdestrect->top*ddsd_dst.lPitch;
	dest += lpdestrect->left;
	destpitch = ddsd_dst.lPitch - w;
	dest0 = dest;

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

	if(dxw.dwFlags3 & MARKBLIT) MarkRect32(dest0, w, h, destpitch);

	res=(*pUnlockMethod(dxversion))(lpddsdst, lpdestrect);
	if (res) OutTraceE("EmuBlt24_32: Unlock ERROR dds=%x res=%x(%s) at %d\n", lpddsdst, res, ExplainDDError(res), __LINE__);
	res=(*pUnlockMethod(dxversion))(lpddssrc, lpsrcrect);
	if (res) OutTraceE("EmuBlt24_32: Unlock ERROR dds=%x res=%x(%s) at %d\n", lpddssrc, res, ExplainDDError(res), __LINE__);
	return res;
}

// note: better avoid direct blitting in case of identical color depth (e.g. EmuBlt_32_to_32, EmuBlt_16_to_16)
// because it does not work between complex surfaces when DDSDCAPS_SYSTEMMEMORY is not omogeneous!

static HRESULT WINAPI EmuBlt_32_to_32(int dxversion, Blt_Type pBlt, LPDIRECTDRAWSURFACE lpddsdst, LPRECT lpdestrect,
	LPDIRECTDRAWSURFACE lpddssrc, LPRECT lpsrcrect, DWORD dwflags, LPVOID lpsurface)
{
	DWORD x, y, w, h;
	long srcpitch, destpitch;
	HRESULT res;
	DWORD *src32;
	DWORD *dest, *dest0;
	DDSURFACEDESC2 ddsd_src, ddsd_dst;

	w = lpdestrect->right - lpdestrect->left; 
	h = lpdestrect->bottom - lpdestrect->top; 

	memset(&ddsd_dst,0,sizeof(DDSURFACEDESC2));
	ddsd_dst.dwSize = Set_dwSize_From_Surface();
	ddsd_dst.dwFlags = DDSD_LPSURFACE | DDSD_PITCH;
	if(res=(*pLockMethod(dxversion))(lpddsdst, 0, (LPDDSURFACEDESC)&ddsd_dst, DDLOCK_SURFACEMEMORYPTR|DDLOCK_WRITEONLY|DDLOCK_WAIT, 0)){	
		OutTraceE("EmuBlt32_32: Lock ERROR res=%x(%s) at %d\n", res, ExplainDDError(res), __LINE__);
		return res;
	}

	memset(&ddsd_src,0,sizeof(DDSURFACEDESC2));
	ddsd_src.dwSize = Set_dwSize_From_Surface();
	ddsd_src.dwFlags = DDSD_LPSURFACE | DDSD_PITCH;
	if (lpsurface) { // already locked, just get info ....
		if(res=lpddssrc->GetSurfaceDesc((LPDDSURFACEDESC)&ddsd_src)) {
			OutTraceE("EmuBlt32_32: GetSurfaceDesc ERROR res=%x(%s) at %d\n", res, ExplainDDError(res), __LINE__);
			(*pUnlockMethod(dxversion))(lpddsdst, lpdestrect);
			(*pUnlockMethod(dxversion))(lpddssrc, lpsrcrect);
			return res;
		}
	}
	else {
		if(res=(*pLockMethod(dxversion))(lpddssrc, 0, (LPDDSURFACEDESC)&ddsd_src, DDLOCK_SURFACEMEMORYPTR|DDLOCK_READONLY, 0)) {
			OutTraceE("EmuBlt32_32: Lock ERROR res=%x(%s) at %d\n", res, ExplainDDError(res), __LINE__);
			(*pUnlockMethod(dxversion))(lpddsdst,0);
			(*pUnlockMethod(dxversion))(lpddssrc, 0);
			return res;
		}
		lpsurface=ddsd_src.lpSurface;
	}

	ddsd_dst.lPitch >>= 2;
	dest = (DWORD *)ddsd_dst.lpSurface;
	dest += lpdestrect->top*ddsd_dst.lPitch;
	dest += lpdestrect->left;
	destpitch = ddsd_dst.lPitch - w;
	dest0 = dest;

	ddsd_src.lPitch >>= 2;
	src32 = (DWORD *)(lpsurface ? lpsurface:ddsd_src.lpSurface);
	src32 += lpsrcrect->top*ddsd_src.lPitch;
	src32 += lpsrcrect->left;
	srcpitch = ddsd_src.lPitch - w;

	// OutTraceDW("DEBUG: h=%d w=%d src=%x dst=%x spitch=%d dpitch=%d\n",h,w,src32,dest,srcpitch,destpitch);
	for(y = 0; y < h; y ++){
		for(x = 0; x < w; x ++)
			*(dest ++) = *(src32 ++);
		dest += destpitch;
		src32 += srcpitch;
	}

	if(dxw.dwFlags3 & MARKBLIT) MarkRect32(dest0, w, h, destpitch);

	res=(*pUnlockMethod(dxversion))(lpddsdst,lpdestrect);
	if (res) OutTraceE("EmuBlt32_32: Unlock ERROR dds=%x res=%x(%s) at %d\n", lpddsdst, res, ExplainDDError(res), __LINE__);
	res=(*pUnlockMethod(dxversion))(lpddssrc, lpsrcrect);
	if (res) OutTraceE("EmuBlt32_32: Unlock ERROR dds=%x res=%x(%s) at %d\n", lpddssrc, res, ExplainDDError(res), __LINE__);
	return res;
}

static HRESULT WINAPI EmuBlt_8_to_16(int dxversion, Blt_Type pBlt, LPDIRECTDRAWSURFACE lpddsdst, LPRECT lpdestrect,
	LPDIRECTDRAWSURFACE lpddssrc, LPRECT lpsrcrect, DWORD dwflags, LPVOID lpsurface)
{
	HRESULT res;
	BYTE *src8;
	SHORT *dest, *dest0;
	DDSURFACEDESC2 ddsd_src, ddsd_dst;
	long srcpitch, destpitch;
	DWORD x, y, w, h;

	w = lpdestrect->right - lpdestrect->left; 
	h = lpdestrect->bottom - lpdestrect->top; 

	memset(&ddsd_dst,0,sizeof(DDSURFACEDESC2));
	ddsd_dst.dwSize = Set_dwSize_From_Surface();
	ddsd_dst.dwFlags = DDSD_LPSURFACE | DDSD_PITCH;
	if(res=(*pLockMethod(dxversion))(lpddsdst, 0, (LPDDSURFACEDESC)&ddsd_dst, DDLOCK_SURFACEMEMORYPTR|DDLOCK_WRITEONLY|DDLOCK_WAIT, 0)){	
		OutTraceE("EmuBlt8_16: Lock ERROR res=%x(%s) at %d\n", res, ExplainDDError(res), __LINE__);
		return res;
	}

	memset(&ddsd_src,0,sizeof(DDSURFACEDESC2));
	ddsd_src.dwSize = Set_dwSize_From_Surface();
	ddsd_src.dwFlags = DDSD_LPSURFACE | DDSD_PITCH;
	if (lpsurface) { // already locked, just get info ....
		if(res=lpddssrc->GetSurfaceDesc((LPDDSURFACEDESC)&ddsd_src)) {
			OutTraceE("EmuBlt8_16: GetSurfaceDesc ERROR res=%x(%s) at %d\n", res, ExplainDDError(res), __LINE__);
			(*pUnlockMethod(dxversion))(lpddsdst, lpdestrect);
			(*pUnlockMethod(dxversion))(lpddssrc, lpsrcrect);
			return 0;
		}
	}
	else {
		if(res=(*pLockMethod(dxversion))(lpddssrc, 0, (LPDDSURFACEDESC)&ddsd_src, DDLOCK_SURFACEMEMORYPTR|DDLOCK_READONLY, 0)) {
			OutTraceE("EmuBlt8_16: Lock ERROR res=%x(%s) at %d\n", res, ExplainDDError(res), __LINE__);
			(*pUnlockMethod(dxversion))(lpddsdst,0);
			(*pUnlockMethod(dxversion))(lpddssrc, 0);
			return 0;
		}
		lpsurface=ddsd_src.lpSurface;
	}

	ddsd_dst.lPitch >>= 1;
	dest = (SHORT *)ddsd_dst.lpSurface;
	dest += lpdestrect->top*ddsd_dst.lPitch;
	dest += lpdestrect->left;
	destpitch = ddsd_dst.lPitch - w;
	dest0 = dest;

	src8 = (BYTE *)lpsurface;
	src8 += lpsrcrect->top*ddsd_src.lPitch;
	src8 += lpsrcrect->left;
	srcpitch = ddsd_src.lPitch - w;

	// OutTraceDW("DEBUG: h=%d w=%d src=%x dst=%x spitch=%d dpitch=%d\n",h,w,src8,dest,srcpitch,destpitch);
	for(y = 0; y < h; y ++){
		for(x = 0; x < w; x ++){
			*(dest ++) = (SHORT)PaletteEntries[*(src8 ++)];
		}
		dest += destpitch;
		src8 += srcpitch;
	}

	if(dxw.dwFlags3 & MARKBLIT) MarkRect16(dest0, w, h, destpitch);

	res=(*pUnlockMethod(dxversion))(lpddsdst, lpdestrect);
	if (res) OutTraceE("EmuBlt8_16: Unlock ERROR dds=%x res=%x(%s) at %d\n", lpddsdst, res, ExplainDDError(res), __LINE__);
	res=(*pUnlockMethod(dxversion))(lpddssrc, lpsrcrect);
	if (res) OutTraceE("EmuBlt8_16: Unlock ERROR dds=%x res=%x(%s) at %d\n", lpddssrc, res, ExplainDDError(res), __LINE__);
	return res;
}

static HRESULT WINAPI BilinearBlt_8_to_16(int dxversion, Blt_Type pBlt, LPDIRECTDRAWSURFACE lpddsdst, LPRECT lpdestrect,
	LPDIRECTDRAWSURFACE lpddssrc, LPRECT lpsrcrect, DWORD dwflags, LPVOID lpsurface)
{
	HRESULT res;
	BYTE *src8, *src0;
	SHORT *dest, *dest0;
	DDSURFACEDESC2 ddsd_src, ddsd_dst;
	long srcpitch, destpitch;
	int w, h;
	typedef DWORD (*Melt16_Type)(DWORD, DWORD);
	Melt16_Type Melt16;

	w = lpdestrect->right - lpdestrect->left; 
	h = lpdestrect->bottom - lpdestrect->top; 

	lpdestrect->left <<= 1;
	lpdestrect->top <<= 1;
	lpdestrect->right <<= 1;
	lpdestrect->bottom <<= 1;

	memset(&ddsd_dst,0,sizeof(DDSURFACEDESC2));
	ddsd_dst.dwSize = Set_dwSize_From_Surface();
	ddsd_dst.dwFlags = DDSD_LPSURFACE | DDSD_PITCH;
	if(res=(*pLockMethod(dxversion))(lpddsdst, 0, (LPDDSURFACEDESC)&ddsd_dst, DDLOCK_SURFACEMEMORYPTR|DDLOCK_WRITEONLY|DDLOCK_WAIT, 0)){	
		OutTraceE("BilBlt8_16: Lock ERROR res=%x(%s) at %d\n", res, ExplainDDError(res), __LINE__);
		return res;
	}

	Melt16 = (ddsd_dst.ddpfPixelFormat.dwGBitMask == 0x3E0) ? Melt16_555 : Melt16_565;

	memset(&ddsd_src,0,sizeof(DDSURFACEDESC2));
	ddsd_src.dwSize = Set_dwSize_From_Surface();
	ddsd_src.dwFlags = DDSD_LPSURFACE | DDSD_PITCH;
	if (lpsurface) { // already locked, just get info ....
		if(res=lpddssrc->GetSurfaceDesc((LPDDSURFACEDESC)&ddsd_src)) {
			OutTraceE("BilBlt8_16: GetSurfaceDesc ERROR res=%x(%s) at %d\n", res, ExplainDDError(res), __LINE__);
			(*pUnlockMethod(dxversion))(lpddsdst, lpdestrect);
			(*pUnlockMethod(dxversion))(lpddssrc, lpsrcrect);
			return 0;
		}
	}
	else {
		if(res=(*pLockMethod(dxversion))(lpddssrc, 0, (LPDDSURFACEDESC)&ddsd_src, DDLOCK_SURFACEMEMORYPTR|DDLOCK_READONLY, 0)) {
			OutTraceE("BilBlt8_16: Lock ERROR res=%x(%s) at %d\n", res, ExplainDDError(res), __LINE__);
			(*pUnlockMethod(dxversion))(lpddsdst,0);
			(*pUnlockMethod(dxversion))(lpddssrc, 0);
			return 0;
		}
		lpsurface=ddsd_src.lpSurface;
	}

	ddsd_dst.lPitch >>= 1;
	dest = (SHORT *)ddsd_dst.lpSurface;
	dest += lpdestrect->top*ddsd_dst.lPitch;
	dest += lpdestrect->left;
	destpitch = ddsd_dst.lPitch - (2 * w);
	dest0 = dest;

	src8 = (BYTE *)lpsurface;
	src8 += (lpsrcrect->top >> 1)*ddsd_src.lPitch;
	src8 += (lpsrcrect->left >> 1);
	srcpitch = ddsd_src.lPitch - w;
	src0 = src8;

	// OutTraceDW("DEBUG: h=%d w=%d src=%x dst=%x spitch=%d dpitch=%d\n",h,w,src8,dest,srcpitch,destpitch);
#ifdef USEOPENMP
#pragma omp parallel for schedule(static)
#endif
    for(int y = 0; y < h; y ++){ 
		register DWORD Q1, Q2, Q3, Q4, Q5;
		src8 = src0 + (y * ddsd_src.lPitch);
		dest = dest0 + ((y * ddsd_dst.lPitch) << 1);	
		Q5 = Melt16(PaletteEntries[*(src8)], PaletteEntries[*(src8+ddsd_src.lPitch)]);
        for(int x = 0; x < w; x ++){
			Q1 = PaletteEntries[*(src8)];
			Q2 = Melt16(Q1, PaletteEntries[*(src8+1)]);
			Q3 = Q5;
			if(y == h-1){
				Q3 = Q1;
				Q4 = Q2;
			}
			else{
				Q5 = Melt16(PaletteEntries[*(src8+1)], PaletteEntries[*(src8+ddsd_src.lPitch+1)]); // to be used in next for cycle
				Q4 = Melt16(Q3, Q5);
			}
			*(dest) = (SHORT)Q1;
			*(dest+1) = (SHORT)Q2;
			*(dest+ddsd_dst.lPitch) = (SHORT)Q3;
			*(dest+ddsd_dst.lPitch+1) = (SHORT)Q4;
			src8++;
			dest+=2;
        }
	}

	if(dxw.dwFlags3 & MARKBLIT) MarkRect16(dest0, 2*w, 2*h, destpitch);

	res=(*pUnlockMethod(dxversion))(lpddsdst, lpdestrect);
	if (res) OutTraceE("BilBlt8_16: Unlock ERROR dds=%x res=%x(%s) at %d\n", lpddsdst, res, ExplainDDError(res), __LINE__);
	res=(*pUnlockMethod(dxversion))(lpddssrc, lpsrcrect);
	if (res) OutTraceE("BilBlt8_16: Unlock ERROR dds=%x res=%x(%s) at %d\n", lpddssrc, res, ExplainDDError(res), __LINE__);
	return res;
}

static HRESULT WINAPI EmuBlt_16_to_16(int dxversion, Blt_Type pBlt, LPDIRECTDRAWSURFACE lpddsdst, LPRECT lpdestrect,
	LPDIRECTDRAWSURFACE lpddssrc, LPRECT lpsrcrect, DWORD dwflags, LPVOID lpsurface)
{
#if 0
	return (*pBlt)(lpddsdst, lpdestrect, lpddssrc, lpsrcrect, dwflags, NULL);
#endif
	HRESULT res;
	SHORT *src16;
	SHORT *dest, *dest0;
	DDSURFACEDESC2 ddsd_src, ddsd_dst;
	long srcpitch, destpitch;
	DWORD x, y, w, h;
	BOOL is555src, is555dst;

	w = lpdestrect->right - lpdestrect->left; 
	h = lpdestrect->bottom - lpdestrect->top; 

	memset(&ddsd_dst,0,sizeof(DDSURFACEDESC2));
	ddsd_dst.dwSize = Set_dwSize_From_Surface();
	ddsd_dst.dwFlags = DDSD_LPSURFACE | DDSD_PITCH;
	if(res=(*pLockMethod(dxversion))(lpddsdst, 0, (LPDDSURFACEDESC)&ddsd_dst, DDLOCK_SURFACEMEMORYPTR|DDLOCK_WRITEONLY|DDLOCK_WAIT, 0)){	
		OutTraceE("EmuBlt16_16: Lock ERROR res=%x(%s) at %d\n", res, ExplainDDError(res), __LINE__);
		return res;
	}

	memset(&ddsd_src,0,sizeof(DDSURFACEDESC2));
	ddsd_src.dwSize = Set_dwSize_From_Surface();
	ddsd_src.dwFlags = DDSD_LPSURFACE | DDSD_PITCH;
	if (lpsurface) { // already locked, just get info ....
		if(res=lpddssrc->GetSurfaceDesc((LPDDSURFACEDESC)&ddsd_src)) {
			OutTraceE("EmuBlt16_16: GetSurfaceDesc ERROR res=%x(%s) at %d\n", res, ExplainDDError(res), __LINE__);
			(*pUnlockMethod(dxversion))(lpddsdst, lpdestrect);
			(*pUnlockMethod(dxversion))(lpddssrc, lpsrcrect);
			return 0;
		}
	}
	else {
		if(res=(*pLockMethod(dxversion))(lpddssrc, 0, (LPDDSURFACEDESC)&ddsd_src, DDLOCK_SURFACEMEMORYPTR|DDLOCK_READONLY, 0)) {
			OutTraceE("EmuBlt16_16: Lock ERROR res=%x(%s) at %d\n", res, ExplainDDError(res), __LINE__);
			(*pUnlockMethod(dxversion))(lpddsdst,0);
			(*pUnlockMethod(dxversion))(lpddssrc, 0);
			return 0;
		}
		lpsurface=ddsd_src.lpSurface;
	}

	ddsd_dst.lPitch >>= 1;
	dest = (SHORT *)ddsd_dst.lpSurface;
	dest += lpdestrect->top*ddsd_dst.lPitch;
	dest += lpdestrect->left;
	destpitch = ddsd_dst.lPitch - w;
	dest0 = dest;
	is555dst = (ddsd_dst.ddpfPixelFormat.dwGBitMask == 0x3E0);

	ddsd_src.lPitch >>= 1;
	src16 = (SHORT *)lpsurface;
	src16 += lpsrcrect->top*ddsd_src.lPitch;
	src16 += lpsrcrect->left;
	srcpitch = ddsd_src.lPitch - w;
	is555src = (ddsd_dst.ddpfPixelFormat.dwGBitMask == 0x3E0);

	// OutTraceDW("DEBUG: h=%d w=%d src=%x dst=%x spitch=%d dpitch=%d\n",h,w,src8,dest,srcpitch,destpitch);
	if (is555dst == is555src){
		for(y = 0; y < h; y ++){
			// copy one whole pixel line at a time....
			memcpy(dest, src16, w<<1);
			dest += (w + destpitch);
			src16 += (w + srcpitch);
		}
	}
	else {
	if(is555dst){
		for(y = 0; y < h; y ++){
			for(x = 0; x < w; x ++){
				SHORT pi;
				pi = *(src16 ++);
				pi = (pi & 0x1F) | (pi & 0x7C0)>>1 | (pi & 0xF800)>>1;
				*(dest ++) = pi;
			}
			dest += destpitch;
			src16 += srcpitch;
		}
	}
	else {
		for(y = 0; y < h; y ++){
			for(x = 0; x < w; x ++){
				SHORT pi;
				pi = *(src16 ++);
				pi = (pi & 0x1F) | (pi & 0x3E0)<<1 | (pi & 0x7C00)<<1;
				*(dest ++) = pi;
			}
			dest += destpitch;
			src16 += srcpitch;
		}
	}
	}

	if(dxw.dwFlags3 & MARKBLIT) MarkRect16(dest0, w, h, destpitch);

	res=(*pUnlockMethod(dxversion))(lpddsdst, lpdestrect);
	if (res) OutTraceE("EmuBlt16_16: Unlock ERROR dds=%x res=%x(%s) at %d\n", lpddsdst, res, ExplainDDError(res), __LINE__);
	res=(*pUnlockMethod(dxversion))(lpddssrc, lpsrcrect);
	if (res) OutTraceE("EmuBlt16_16: Unlock ERROR dds=%x res=%x(%s) at %d\n", lpddssrc, res, ExplainDDError(res), __LINE__);
	return res;
}

static HRESULT WINAPI BilinearBlt_16_to_16(int dxversion, Blt_Type pBlt, LPDIRECTDRAWSURFACE lpddsdst, LPRECT lpdestrect,
	LPDIRECTDRAWSURFACE lpddssrc, LPRECT lpsrcrect, DWORD dwflags, LPVOID lpsurface)
{
	HRESULT res;
	WORD *src16, *src0;
	WORD *dest, *dest0;
	DDSURFACEDESC2 ddsd_src, ddsd_dst;
	long srcpitch, destpitch;
	int w, h;
	typedef DWORD (*Melt16_Type)(DWORD, DWORD);
	Melt16_Type Melt16;

	w = lpdestrect->right - lpdestrect->left; 
	h = lpdestrect->bottom - lpdestrect->top; 

	lpdestrect->left <<= 1;
	lpdestrect->top <<= 1;
	lpdestrect->right <<= 1;
	lpdestrect->bottom <<= 1;

	memset(&ddsd_dst,0,sizeof(DDSURFACEDESC2));
	ddsd_dst.dwSize = Set_dwSize_From_Surface();
	ddsd_dst.dwFlags = DDSD_LPSURFACE | DDSD_PITCH;
	if(res=(*pLockMethod(dxversion))(lpddsdst, 0, (LPDDSURFACEDESC)&ddsd_dst, DDLOCK_SURFACEMEMORYPTR|DDLOCK_WRITEONLY|DDLOCK_WAIT, 0)){	
		OutTraceE("BilBlt16_16: Lock ERROR res=%x(%s) at %d\n", res, ExplainDDError(res), __LINE__);
		return res;
	}

	Melt16 = (ddsd_dst.ddpfPixelFormat.dwGBitMask == 0x3E0) ? Melt16_555 : Melt16_565;

	memset(&ddsd_src,0,sizeof(DDSURFACEDESC2));
	ddsd_src.dwSize = Set_dwSize_From_Surface();
	ddsd_src.dwFlags = DDSD_LPSURFACE | DDSD_PITCH;
	if (lpsurface) { // already locked, just get info ....
		if(res=lpddssrc->GetSurfaceDesc((LPDDSURFACEDESC)&ddsd_src)) {
			OutTraceE("BilBlt16_16: GetSurfaceDesc ERROR res=%x(%s) at %d\n", res, ExplainDDError(res), __LINE__);
			(*pUnlockMethod(dxversion))(lpddsdst, lpdestrect);
			(*pUnlockMethod(dxversion))(lpddssrc, lpsrcrect);
			return 0;
		}
	}
	else {
		if(res=(*pLockMethod(dxversion))(lpddssrc, 0, (LPDDSURFACEDESC)&ddsd_src, DDLOCK_SURFACEMEMORYPTR|DDLOCK_READONLY|DDLOCK_WAIT, 0)) {
			OutTraceE("BilBlt16_16: Lock ERROR res=%x(%s) at %d\n", res, ExplainDDError(res), __LINE__);
			(*pUnlockMethod(dxversion))(lpddsdst,0);
			(*pUnlockMethod(dxversion))(lpddssrc, 0);
			return res;
		}
		lpsurface=ddsd_src.lpSurface;
	}

	ddsd_dst.lPitch >>= 1;
	dest = (WORD *)ddsd_dst.lpSurface;
	dest += lpdestrect->top*ddsd_dst.lPitch;
	dest += lpdestrect->left;
	destpitch = ddsd_dst.lPitch - (2 * w);
	dest0 = dest;

	ddsd_src.lPitch >>= 1;
	src16 = (WORD *)(lpsurface ? lpsurface:ddsd_src.lpSurface);
	src16 += (lpsrcrect->top >> 1)*ddsd_src.lPitch;
	src16 += (lpsrcrect->left >> 1);
	srcpitch = ddsd_src.lPitch - w;
	src0 = src16;

	// OutTraceDW("DEBUG: h=%d w=%d src=%x dst=%x spitch=%d dpitch=%d\n",h,w,src8,dest,srcpitch,destpitch);
#ifdef USEOPENMP
#pragma omp parallel for schedule(static)
#endif
    for(int y = 0; y < h; y ++){ 
		register DWORD Q1, Q2, Q3, Q4, Q5;
		src16 = src0 + (y * ddsd_src.lPitch);
		dest = dest0 + ((y * ddsd_dst.lPitch) << 1);	
		Q5 = Melt16(*(src16), *(src16+ddsd_src.lPitch));
        for(int x = 0; x < w; x ++){
			Q1 = *(src16);
			Q2 = Melt16(Q1, *(src16+1));
			Q3 = Q5;
			if(y == h-1){
				Q3 = Q1;
				Q4 = Q2;
			}
			else{
				Q5 = Melt16(*(src16+1), *(src16+ddsd_src.lPitch+1)); // to be used in next for cycle
				Q4 = Melt16(Q3, Q5);
			}
			*(dest) = (WORD)Q1;
			*(dest+1) = (WORD)Q2;
			*(dest+ddsd_dst.lPitch) = (WORD)Q3;
			*(dest+ddsd_dst.lPitch+1) = (WORD)Q4;
			src16++;
			dest+=2;
        }
	}

	if(dxw.dwFlags3 & MARKBLIT) MarkRect16((SHORT *)dest0, 2*w, 2*h, destpitch);

	res=(*pUnlockMethod(dxversion))(lpddsdst, lpdestrect);
	if (res) OutTraceE("BilBlt16_16: Unlock ERROR dds=%x res=%x(%s) at %d\n", lpddsdst, res, ExplainDDError(res), __LINE__);
	res=(*pUnlockMethod(dxversion))(lpddssrc, lpsrcrect);
	if (res) OutTraceE("BilBlt16_16: Unlock ERROR dds=%x res=%x(%s) at %d\n", lpddssrc, res, ExplainDDError(res), __LINE__);
	return res;
}


static HRESULT WINAPI EmuBlt_24_to_16(int dxversion, Blt_Type pBlt, LPDIRECTDRAWSURFACE lpddsdst, LPRECT lpdestrect,
	LPDIRECTDRAWSURFACE lpddssrc, LPRECT lpsrcrect, DWORD dwflags, LPVOID lpsurface)
{
	HRESULT res;
	BYTE *src24;
	SHORT *dest, *dest0;
	DDSURFACEDESC2 ddsd_src, ddsd_dst;
	long srcpitch, destpitch;
	DWORD x, y, w, h;

	w = lpdestrect->right - lpdestrect->left; 
	h = lpdestrect->bottom - lpdestrect->top; 

	memset(&ddsd_dst,0,sizeof(DDSURFACEDESC2));
	ddsd_dst.dwSize = Set_dwSize_From_Surface();
	ddsd_dst.dwFlags = DDSD_LPSURFACE | DDSD_PITCH;
	if(res=(*pLockMethod(dxversion))(lpddsdst, 0, (LPDDSURFACEDESC)&ddsd_dst, DDLOCK_SURFACEMEMORYPTR|DDLOCK_WRITEONLY|DDLOCK_WAIT, 0)){	
		OutTraceE("EmuBlt24_16: Lock ERROR res=%x(%s) at %d\n", res, ExplainDDError(res), __LINE__);
		return res;
	}

	memset(&ddsd_src,0,sizeof(DDSURFACEDESC2));
	ddsd_src.dwSize = Set_dwSize_From_Surface();
	ddsd_src.dwFlags = DDSD_LPSURFACE | DDSD_PITCH;
	if (lpsurface) { // already locked, just get info ....
		if(res=lpddssrc->GetSurfaceDesc((LPDDSURFACEDESC)&ddsd_src)) {
			OutTraceE("EmuBlt24_16: GetSurfaceDesc ERROR res=%x(%s) at %d\n", res, ExplainDDError(res), __LINE__);
			(*pUnlockMethod(dxversion))(lpddsdst, lpdestrect);
			(*pUnlockMethod(dxversion))(lpddssrc, lpsrcrect);
			return 0;
		}
	}
	else {
		if(res=(*pLockMethod(dxversion))(lpddssrc, 0, (LPDDSURFACEDESC)&ddsd_src, DDLOCK_SURFACEMEMORYPTR|DDLOCK_READONLY, 0)) {
			OutTraceE("EmuBlt24_16: Lock ERROR res=%x(%s) at %d\n", res, ExplainDDError(res), __LINE__);
			(*pUnlockMethod(dxversion))(lpddsdst,0);
			(*pUnlockMethod(dxversion))(lpddssrc, 0);
			return 0;
		}
		lpsurface=ddsd_src.lpSurface;
	}

	ddsd_dst.lPitch >>= 1;
	dest = (SHORT *)ddsd_dst.lpSurface;
	dest += lpdestrect->top*ddsd_dst.lPitch;
	dest += lpdestrect->left;
	destpitch = ddsd_dst.lPitch - w;
	dest0 = dest;

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

	if(dxw.dwFlags3 & MARKBLIT) MarkRect16(dest0, w, h, destpitch);

	res=(*pUnlockMethod(dxversion))(lpddsdst, lpdestrect);
	if (res) OutTraceE("EmuBlt24_16: Unlock ERROR dds=%x res=%x(%s) at %d\n", lpddsdst, res, ExplainDDError(res), __LINE__);
	res=(*pUnlockMethod(dxversion))(lpddssrc, lpsrcrect);
	if (res) OutTraceE("EmuBlt24_16: Unlock ERROR dds=%x res=%x(%s) at %d\n", lpddssrc, res, ExplainDDError(res), __LINE__);
	return res;
}

static HRESULT WINAPI EmuBlt_32_to_16(int dxversion, Blt_Type pBlt, LPDIRECTDRAWSURFACE lpddsdst, LPRECT lpdestrect,
	LPDIRECTDRAWSURFACE lpddssrc, LPRECT lpsrcrect, DWORD dwflags, LPVOID lpsurface)
{
	HRESULT res;
	BYTE *src32;
	SHORT *dest, *dest0;
	DDSURFACEDESC2 ddsd_src, ddsd_dst;
	long srcpitch, destpitch;
	DWORD x, y, w, h;

	w = lpdestrect->right - lpdestrect->left; 
	h = lpdestrect->bottom - lpdestrect->top; 

	memset(&ddsd_dst,0,sizeof(DDSURFACEDESC2));
	ddsd_dst.dwSize = Set_dwSize_From_Surface();
	ddsd_dst.dwFlags = DDSD_LPSURFACE | DDSD_PITCH;
	if(res=(*pLockMethod(dxversion))(lpddsdst, 0, (LPDDSURFACEDESC)&ddsd_dst, DDLOCK_SURFACEMEMORYPTR|DDLOCK_WRITEONLY|DDLOCK_WAIT, 0)){	
		OutTraceE("EmuBlt32_16: Lock ERROR res=%x(%s) at %d\n", res, ExplainDDError(res), __LINE__);
		return res;
	}

	memset(&ddsd_src,0,sizeof(DDSURFACEDESC2));
	ddsd_src.dwSize = Set_dwSize_From_Surface();
	ddsd_src.dwFlags = DDSD_LPSURFACE | DDSD_PITCH;
	if (lpsurface) { // already locked, just get info ....
		if(res=lpddssrc->GetSurfaceDesc((LPDDSURFACEDESC)&ddsd_src)) {
			OutTraceE("EmuBlt32_16: GetSurfaceDesc ERROR res=%x(%s) at %d\n", res, ExplainDDError(res), __LINE__);
			(*pUnlockMethod(dxversion))(lpddsdst, lpdestrect);
			(*pUnlockMethod(dxversion))(lpddssrc, lpsrcrect);
			return 0;
		}
	}
	else {
		if(res=(*pLockMethod(dxversion))(lpddssrc, 0, (LPDDSURFACEDESC)&ddsd_src, DDLOCK_SURFACEMEMORYPTR|DDLOCK_READONLY, 0)) {
			OutTraceE("EmuBlt32_16: Lock ERROR res=%x(%s) at %d\n", res, ExplainDDError(res), __LINE__);
			(*pUnlockMethod(dxversion))(lpddsdst,0);
			(*pUnlockMethod(dxversion))(lpddssrc, 0);
			return 0;
		}
		lpsurface=ddsd_src.lpSurface;
	}

	ddsd_dst.lPitch >>= 1;
	dest = (SHORT *)ddsd_dst.lpSurface;
	dest += lpdestrect->top*ddsd_dst.lPitch;
	dest += lpdestrect->left;
	destpitch = ddsd_dst.lPitch - w;
	dest0 = dest;

	src32 = (BYTE *)lpsurface;
	src32 += lpsrcrect->top*ddsd_src.lPitch;
	src32 += lpsrcrect->left;
	srcpitch = ddsd_src.lPitch - 4*w;

	for(y = 0; y < h; y ++){
		for(x = 0; x < w; x ++){
			*(dest ++) = (*(src32+2) & 0xF8)<<8 | (*(src32+1) & 0xFC)<<3 | (*(src32+0) & 0xF8)>>3;
			src32 += 4; // 4 bytes = 32 bits
		}
		dest += destpitch;
		src32 += srcpitch;
	}

	if(dxw.dwFlags3 & MARKBLIT) MarkRect16(dest0, w, h, destpitch);

	res=(*pUnlockMethod(dxversion))(lpddsdst, lpdestrect);
	if (res) OutTraceE("EmuBlt32_16: Unlock ERROR dds=%x res=%x(%s) at %d\n", lpddsdst, res, ExplainDDError(res), __LINE__);
	res=(*pUnlockMethod(dxversion))(lpddssrc, lpsrcrect);
	if (res) OutTraceE("EmuBlt32_16: Unlock ERROR dds=%x res=%x(%s) at %d\n", lpddssrc, res, ExplainDDError(res), __LINE__);
	return res;
}

static HRESULT WINAPI BilinearBlt_32_to_32(int dxversion, Blt_Type pBlt, LPDIRECTDRAWSURFACE lpddsdst, LPRECT lpdestrect,
	LPDIRECTDRAWSURFACE lpddssrc, LPRECT lpsrcrect, DWORD dwflags, LPVOID lpsurface)
{
	HRESULT res;
	DWORD *src32;
	DWORD *dest, *dest0;
	DDSURFACEDESC2 ddsd_src, ddsd_dst;
	long srcpitch, destpitch;
	DWORD x, y, w, h;

	w = lpdestrect->right - lpdestrect->left; 
	h = lpdestrect->bottom - lpdestrect->top; 

	lpdestrect->left <<= 1;
	lpdestrect->top <<= 1;
	lpdestrect->right <<= 1;
	lpdestrect->bottom <<= 1;

	memset(&ddsd_dst,0,sizeof(DDSURFACEDESC2));
	ddsd_dst.dwSize = Set_dwSize_From_Surface();
	ddsd_dst.dwFlags = DDSD_LPSURFACE | DDSD_PITCH;
	if(res=(*pLockMethod(dxversion))(lpddsdst, 0, (LPDDSURFACEDESC)&ddsd_dst, DDLOCK_SURFACEMEMORYPTR|DDLOCK_WRITEONLY|DDLOCK_WAIT, 0)){	
		OutTraceE("BilBlt32_32: Lock ERROR res=%x(%s) at %d\n", res, ExplainDDError(res), __LINE__);
		return res;
	}

	memset(&ddsd_src,0,sizeof(DDSURFACEDESC2));
	ddsd_src.dwSize = Set_dwSize_From_Surface();
	ddsd_src.dwFlags = DDSD_LPSURFACE | DDSD_PITCH;
	if (lpsurface) { // already locked, just get info ....
		if(res=lpddssrc->GetSurfaceDesc((LPDDSURFACEDESC)&ddsd_src)) {
			OutTraceE("BilBlt32_32: GetSurfaceDesc ERROR res=%x(%s) at %d\n", res, ExplainDDError(res), __LINE__);
			(*pUnlockMethod(dxversion))(lpddsdst, lpdestrect);
			(*pUnlockMethod(dxversion))(lpddssrc, lpsrcrect);
			return 0;
		}
	}
	else {
		if(res=(*pLockMethod(dxversion))(lpddssrc, 0, (LPDDSURFACEDESC)&ddsd_src, DDLOCK_SURFACEMEMORYPTR|DDLOCK_READONLY|DDLOCK_WAIT, 0)) {
			OutTraceE("BilBlt32_32: Lock ERROR res=%x(%s) at %d\n", res, ExplainDDError(res), __LINE__);
			(*pUnlockMethod(dxversion))(lpddsdst,0);
			(*pUnlockMethod(dxversion))(lpddssrc, 0);
			return res;
		}
		lpsurface=ddsd_src.lpSurface;
	}

	ddsd_dst.lPitch >>= 2;
	dest = (DWORD *)ddsd_dst.lpSurface;
	dest += lpdestrect->top*ddsd_dst.lPitch;
	dest += lpdestrect->left;
	destpitch = ddsd_dst.lPitch - (2 * w);
	dest0 = dest;

	ddsd_src.lPitch >>= 2;
	src32 = (DWORD *)(lpsurface ? lpsurface:ddsd_src.lpSurface);
	src32 += lpsrcrect->top*ddsd_src.lPitch;
	src32 += lpsrcrect->left;
	srcpitch = ddsd_src.lPitch - w;

	//OutTraceDW("DEBUG: h=%d w=%d src=%x dst=%x spitch=%d dpitch=%d\n",h,w,src32,dest,srcpitch,destpitch);
	for(y = 0; y < h-1; y ++){ 
		register DWORD Q1, Q2, Q3, Q4, Q5;
		Q5 = Melt32(*(src32), *(src32+ddsd_src.lPitch));
        for(x = 0; x < w; x ++){
			Q1 = *(src32);
			Q2 = Melt32(Q1, *(src32+1));
			Q3 = Q5;
			Q5 = Melt32(*(src32+1), *(src32+ddsd_src.lPitch+1)); // to be used in next for cycle
			Q4 = Melt32(Q3, Q5);

			*(dest) = Q1;
			*(dest+1) = Q2;
			*(dest+ddsd_dst.lPitch) = Q3;
			*(dest+ddsd_dst.lPitch+1) = Q4;
			src32++;
			dest+=2;
        }
        src32 += srcpitch;
        dest += (ddsd_dst.lPitch + destpitch);
	}
	for(x = 0; x < w; x ++){ // last line (there's no next line to melt...)
		register DWORD Q1, Q2;
		Q1 = *(src32);
		Q2 = Melt32(Q1, *(src32+1));

		*(dest) = Q1;
		*(dest+1) = Q2;
		*(dest+ddsd_dst.lPitch) = Q1;
		*(dest+ddsd_dst.lPitch+1) = Q2;
		src32++;
		dest+=2;
    }

	if(dxw.dwFlags3 & MARKBLIT) MarkRect32(dest0, 2*w, 2*h, destpitch);

	res=(*pUnlockMethod(dxversion))(lpddsdst, lpdestrect);
	if (res) OutTraceE("BilBlt32_32: Unlock ERROR dds=%x res=%x(%s) at %d\n", lpddsdst, res, ExplainDDError(res), __LINE__);
	res=(*pUnlockMethod(dxversion))(lpddssrc, lpsrcrect);
	if (res) OutTraceE("BilBlt32_32: Unlock ERROR dds=%x res=%x(%s) at %d\n", lpddssrc, res, ExplainDDError(res), __LINE__);
	return res;
}

static HRESULT WINAPI EmuBlt_Null(int dxversion, Blt_Type pBlt, LPDIRECTDRAWSURFACE lpddsdst, LPRECT lpdestrect,
	LPDIRECTDRAWSURFACE lpddssrc, LPRECT lpsrcrect, DWORD dwflags, LPVOID lpsurface)
{
	char *sMsg="EmuBlt: undefined color depth\n";
	OutTraceE(sMsg);
	if(IsAssertEnabled) MessageBox(0, sMsg, "EmuBlt", MB_OK | MB_ICONEXCLAMATION);
	return -1;
}

//--------------------------------------------------------------------------------------------//
// exported methods
//--------------------------------------------------------------------------------------------//

void SetBltTransformations(int dxversion)
{
	pPrimaryBlt = PrimaryBlt;
	if(dxw.dwFlags5 & AEROBOOST) pPrimaryBlt = PrimaryStretchBlt;
	if(dxw.dwFlags5 & BILINEARFILTER) pPrimaryBlt = PrimaryBilinearBlt; 
	if(dxw.dwFlags5 & DOFASTBLT) pPrimaryBlt = PrimaryFastBlt; // debug opt
	if(dxw.dwFlags5 & CENTERTOWIN) pPrimaryBlt = PrimaryFastBlt; 
	if(dxw.dwFlags5 & NOBLT) pPrimaryBlt = PrimaryNoBlt; // debug opt

	extern HRESULT WINAPI ColorConversionDDRAW(int, LPDIRECTDRAWSURFACE, RECT, LPDIRECTDRAWSURFACE *);
	extern HRESULT WINAPI ColorConversionGDI(int, LPDIRECTDRAWSURFACE, RECT, LPDIRECTDRAWSURFACE *);
	extern HRESULT WINAPI ColorConversionEmulated(int, LPDIRECTDRAWSURFACE, RECT, LPDIRECTDRAWSURFACE *);
	pColorConversion = ColorConversionDDRAW; // default for no emulation mode
	if(dxw.dwFlags1 & EMULATESURFACE) {
		pColorConversion = ColorConversionEmulated; // default for emulation mode
		if(dxw.dwFlags5 & HYBRIDMODE) pColorConversion = ColorConversionDDRAW;
		if(dxw.dwFlags5 & GDICOLORCONV) pColorConversion = ColorConversionGDI;
	}
	char *s = "???";
	if(pColorConversion == ColorConversionDDRAW) s="DDRAW"; 
	if(pColorConversion == ColorConversionGDI) s="GDI";
	if(pColorConversion == ColorConversionEmulated) s="EMULATED";
	OutTraceDW("SetBltTransformations: color conversion %s BPP %d->%d\n", 
		s, dxw.VirtualPixelFormat.dwRGBBitCount, dxw.ActualPixelFormat.dwRGBBitCount);

	if(pColorConversion != ColorConversionEmulated) return;

	/* default (bad) setting */
	pEmuBlt=EmuBlt_Null;

	switch (dxw.ActualPixelFormat.dwRGBBitCount){
	case 32:
		switch(dxw.VirtualPixelFormat.dwRGBBitCount){
		case 8:
			pEmuBlt=EmuBlt_8_to_32;
			if(dxw.dwFlags4 & BILINEAR2XFILTER) pEmuBlt=BilinearBlt_8_to_32;
			OutTraceDW("set color transformation 8<->32\n");
			break;
		case 16: 
			pEmuBlt=EmuBlt_16_to_32;
			if(dxw.dwFlags4 & BILINEAR2XFILTER) pEmuBlt=BilinearBlt_16_to_32;
			if(dxw.dwFlags5 & DEINTERLACE) pEmuBlt=Deinterlace_16_to_32;
			OutTraceDW("set color transformation 16<->32\n");
			break;
		case 24: 
			pEmuBlt=EmuBlt_24_to_32;
			OutTraceDW("set color transformation 24->32\n");
			break;
		case 32: 
			pEmuBlt=EmuBlt_32_to_32;
			if(dxw.dwFlags4 & BILINEAR2XFILTER) pEmuBlt=BilinearBlt_32_to_32;
			OutTraceDW("set color transformation 32->32\n");
			break;
		default:
			OutTraceDW("unsupported color transformation %d->32\n", dxw.VirtualPixelFormat.dwRGBBitCount);
			break;
		}
		break;
	case 16:
		switch(dxw.VirtualPixelFormat.dwRGBBitCount){
		case 8:
			pEmuBlt=EmuBlt_8_to_16;
			if(dxw.dwFlags4 & BILINEAR2XFILTER) pEmuBlt=BilinearBlt_8_to_16;
			OutTraceDW("set color transformation 8<->16\n");
			break;
		case 16:
			pEmuBlt=EmuBlt_16_to_16;
			if(dxw.dwFlags4 & BILINEAR2XFILTER) pEmuBlt=BilinearBlt_16_to_16;
			OutTraceDW("set color transformation 16<->16\n");
			break;
		case 24: 
			pEmuBlt=EmuBlt_24_to_16;
			OutTraceDW("set color transformation 24<->16\n");
			break;
		case 32:
			pEmuBlt=EmuBlt_32_to_16;
			break;
		default:
			OutTraceDW("unsupported color transformation %d->16\n", dxw.VirtualPixelFormat.dwRGBBitCount);
			break;
		}
		break;
	default:
		OutTraceDW("unsupported color transformation %d->%d\n",
			dxw.VirtualPixelFormat.dwRGBBitCount,
			dxw.ActualPixelFormat.dwRGBBitCount);
		break;
	}
}
