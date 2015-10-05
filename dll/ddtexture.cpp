#define  _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <d3d.h>
#include <stdio.h>
#include "dxwnd.h"
#include "dxwcore.hpp"
#include "dxhook.h"
#include "syslibs.h"
#include "dxhelper.h"

extern char *ExplainDDError(DWORD);

typedef HRESULT (WINAPI *Lock_Type)(LPDIRECTDRAWSURFACE, LPRECT, LPDDSURFACEDESC, DWORD, HANDLE);
typedef HRESULT (WINAPI *Unlock4_Type)(LPDIRECTDRAWSURFACE, LPRECT);
typedef HRESULT (WINAPI *Unlock1_Type)(LPDIRECTDRAWSURFACE, LPVOID);

extern Lock_Type pLock;
extern Unlock4_Type pUnlockMethod(LPDIRECTDRAWSURFACE);
extern int Set_dwSize_From_Surface(LPDIRECTDRAWSURFACE);

#define GRIDSIZE 16

/* RS Hash Function */

static unsigned int Hash(BYTE *buf, int len)
{
   unsigned int b    = 378551;
   unsigned int a    = 63689;
   DWORD hash = 0;
   for(int i = 0; i < len; i++){
      hash = hash * a + buf[i];
      a    = a * b;
   }
   return hash;
}

unsigned int HashSurface(BYTE *buf, int pitch, int width, int height)
{
	unsigned int b    = 378551;
	unsigned int a    = 63689;
	int pixelsize;
	DWORD hash = 0;
	// integer divide, intentionally throwing reminder away
	pixelsize = pitch / width; 
	for(int y = 0; y < height; y++){
		BYTE *p = buf + (y * pitch);
		for(int x = 0; x < width; x++){
			for(int pixelbyte = 0; pixelbyte < pixelsize; pixelbyte++){
				hash = (hash * a) + (*p++);
				a    = a * b;
			}
		}
	}
	return hash;
}

static char *SurfaceType(DDPIXELFORMAT ddpfPixelFormat)
{
	static char sSurfaceType[81];
	char sColorType[21];
	DWORD mask;
	int i, count;

	if(ddpfPixelFormat.dwRGBBitCount == 8) return "RGB8";

	strcpy(sSurfaceType, "");
	// red
	mask=ddpfPixelFormat.dwRBitMask;
	for (i=0, count=0; i<32; i++) {
		if(mask & 0x1) count++;
		mask >>= 1;
	}
	sprintf(sColorType, "R%d", count);
	strcat(sSurfaceType, sColorType);
	// green
	mask=ddpfPixelFormat.dwGBitMask;
	for (i=0, count=0; i<32; i++) {
		if(mask & 0x1) count++;
		mask >>= 1;
	}
	sprintf(sColorType, "G%d", count);
	strcat(sSurfaceType, sColorType);
	// blue
	mask=ddpfPixelFormat.dwBBitMask;
	for (i=0, count=0; i<32; i++) {
		if(mask & 0x1) count++;
		mask >>= 1;
	}
	sprintf(sColorType, "B%d", count);
	strcat(sSurfaceType, sColorType);
	// alpha channel
	mask=ddpfPixelFormat.dwRGBAlphaBitMask;
	if(mask){
		for (i=0, count=0; i<32; i++) {
			if(mask & 0x1) count++;
			mask >>= 1;
		}
		sprintf(sColorType, "A%d", count);
		strcat(sSurfaceType, sColorType);
	}
	return sSurfaceType;
}

void TextureHighlight(LPDIRECTDRAWSURFACE s)
{
	DDSURFACEDESC2 ddsd;
	int x, y, w, h;
	HRESULT res;
	memset(&ddsd,0,sizeof(DDSURFACEDESC2));
	ddsd.dwSize = Set_dwSize_From_Surface(s);
	ddsd.dwFlags = DDSD_LPSURFACE | DDSD_PITCH;
	if(res=(*pLock)(s, 0, (LPDDSURFACEDESC)&ddsd, DDLOCK_SURFACEMEMORYPTR|DDLOCK_WRITEONLY|DDLOCK_WAIT, 0)){	
		OutTraceE("TextureHigh: Lock ERROR res=%x(%s) at %d\n", res, ExplainDDError(res), __LINE__);
		return;
	}
	extern LPDIRECTDRAWSURFACE lpDDSBack;
	if((ddsd.ddsCaps.dwCaps & DDSCAPS_TEXTURE) && (s != lpDDSBack)) {
		OutTrace("TextureHigh: lpdds=%x BitCount=%d size=(%dx%d)\n", 
			s, ddsd.ddpfPixelFormat.dwRGBBitCount, ddsd.dwWidth, ddsd.dwHeight);
		w = ddsd.dwWidth;
		h = ddsd.dwHeight;
		switch (ddsd.ddpfPixelFormat.dwRGBBitCount){
			case 8:
				{ 
					BYTE *p;
					BYTE color;
					color=(BYTE)(rand() & 0xFF);
					for(y=0; y<h; y++){
						p = (BYTE *)ddsd.lpSurface + (y * ddsd.lPitch);
						for(x=0; x<w; x++) *(p+x) = color;
					}
					for(y=0; y<h; y++){
						p = (BYTE *)ddsd.lpSurface + (y * ddsd.lPitch);
						for(x=0; x<w; x+=GRIDSIZE) *(p+x) = 0;
						if((y%GRIDSIZE)==0) for(x=0; x<w; x++) *(p++) = 0;
					}
				}
				break;
			case 16: 
				{
					SHORT *p;
					SHORT color;
					color=(SHORT)(rand() & 0x7FFF);
					for(y=0; y<h; y++){
						p = (SHORT *)ddsd.lpSurface + ((y * ddsd.lPitch) >> 1);
						for(x=0; x<w; x++) *(p+x) = color;
					}
					for(y=0; y<h; y++){
						p = (SHORT *)ddsd.lpSurface + ((y * ddsd.lPitch) >> 1);
						for(x=0; x<w; x+=GRIDSIZE) *(p+x) = 0;
						if((y%GRIDSIZE)==0) for(x=0; x<w; x++) *(p++) = 0;
					}
				}
				break;
			case 32: 
				{
					DWORD *p;
					DWORD color;
					color=(DWORD)(rand() & 0xFFFFFFFF);
					for(y=0; y<h; y++){
						p = (DWORD *)ddsd.lpSurface + ((y * ddsd.lPitch) >> 2);
						for(x=0; x<w; x++) *(p+x) = color;
					}
					for(y=0; y<h; y++){
						p = (DWORD *)ddsd.lpSurface + ((y * ddsd.lPitch) >> 2);
						for(x=0; x<w; x+=GRIDSIZE) *(p+x) = 0;
						if((y%GRIDSIZE)==0) for(x=0; x<w; x++) *(p++) = 0;
					}
				}
				break;
		}
	}
	res=(*pUnlockMethod(s))(s, NULL);
	if (res) OutTraceE("TextureHigh: Unlock ERROR lpdds=%x res=%x(%s) at %d\n", s, res, ExplainDDError(res), __LINE__);
}

static void TextureDump(LPDIRECTDRAWSURFACE s)
{
	DDSURFACEDESC2 ddsd;
	int w, h, iSurfaceSize, iScanLineSize;
	HRESULT res;
	static int MinTexX, MinTexY, MaxTexX, MaxTexY;
	static BOOL DoOnce = TRUE;
	char pszFile[MAX_PATH];

	if(DoOnce){
		char sProfilePath[MAX_PATH];
		sprintf(sProfilePath, "%s\\dxwnd.ini", GetDxWndPath());
		MinTexX=GetPrivateProfileInt("Texture", "MinTexX", 0, sProfilePath);
		MaxTexX=GetPrivateProfileInt("Texture", "MaxTexX", 0, sProfilePath);
		MinTexY=GetPrivateProfileInt("Texture", "MinTexY", 0, sProfilePath);
		MaxTexY=GetPrivateProfileInt("Texture", "MaxTexY", 0, sProfilePath);
		OutTrace("TextureDump: size min=(%dx%d) max=(%dx%d)\n", MinTexX, MinTexY, MaxTexX, MaxTexY);
		sprintf_s(pszFile, MAX_PATH, "%s\\texture.out", GetDxWndPath());
		CreateDirectory(pszFile, NULL);
		DoOnce = FALSE;
	}

	memset(&ddsd,0,sizeof(DDSURFACEDESC2));
	ddsd.dwSize = Set_dwSize_From_Surface(s);
	ddsd.dwFlags = DDSD_LPSURFACE | DDSD_PITCH;
	if(res=(*pLock)(s, 0, (LPDDSURFACEDESC)&ddsd, DDLOCK_SURFACEMEMORYPTR|DDLOCK_WRITEONLY|DDLOCK_WAIT, 0)){	
		OutTraceE("TextureDump: Lock ERROR res=%x(%s) at %d\n", res, ExplainDDError(res), __LINE__);
		return;
	}

	extern LPDIRECTDRAWSURFACE lpDDSBack;
	if((ddsd.ddsCaps.dwCaps & DDSCAPS_TEXTURE) && (s != lpDDSBack)) while (TRUE) {
		OutTrace("TextureDump: lpdds=%x BitCount=%d size=(%dx%d)\n", 
			s, ddsd.ddpfPixelFormat.dwRGBBitCount, ddsd.dwWidth, ddsd.dwHeight);
		w = ddsd.dwWidth;
		h = ddsd.dwHeight;
		if((MinTexX && (w<MinTexX)) || (MinTexY && (h<MinTexY))) {
			OutTrace("TextureDump: SKIP small texture\n");
			break;
		}
		if((MaxTexX && (w>MaxTexX)) || (MaxTexY && (h>MaxTexY))) {
			OutTrace("TextureDump: SKIP big texture\n");
			break;
		}
		if(ddsd.ddpfPixelFormat.dwRGBBitCount == 0) {
			OutTrace("TextureDump: SKIP 0BPP texture\n");
			break;
		}

		iSurfaceSize = ddsd.dwHeight * ddsd.lPitch;

		FILE *hf;
		BITMAPFILEHEADER hdr;       // bitmap file-header 
		BITMAPV4HEADER pbi;			// bitmap info-header  

		memset((void *)&pbi, 0, sizeof(BITMAPV4HEADER));
		pbi.bV4Size = sizeof(BITMAPV4HEADER); 
		pbi.bV4Width = ddsd.dwWidth;
		pbi.bV4Height = ddsd.dwHeight;
		pbi.bV4BitCount = (WORD)ddsd.ddpfPixelFormat.dwRGBBitCount;
		pbi.bV4SizeImage = ((pbi.bV4Width * pbi.bV4BitCount + 0x1F) & ~0x1F)/8 * pbi.bV4Height; 
		pbi.bV4Height = - pbi.bV4Height;
		pbi.bV4Planes = 1;
		pbi.bV4V4Compression = BI_BITFIELDS;
		if(pbi.bV4BitCount == 8) pbi.bV4V4Compression = BI_RGB;
		pbi.bV4XPelsPerMeter = 1;
		pbi.bV4YPelsPerMeter = 1;
		pbi.bV4ClrUsed = 0;
		if(pbi.bV4BitCount == 8) pbi.bV4ClrUsed = 256;
		pbi.bV4ClrImportant = 0;
		pbi.bV4RedMask = ddsd.ddpfPixelFormat.dwRBitMask;
		pbi.bV4GreenMask = ddsd.ddpfPixelFormat.dwGBitMask;
		pbi.bV4BlueMask = ddsd.ddpfPixelFormat.dwBBitMask;
		pbi.bV4AlphaMask = ddsd.ddpfPixelFormat.dwRGBAlphaBitMask;
		pbi.bV4CSType = LCS_CALIBRATED_RGB;
		iScanLineSize = ((pbi.bV4Width * pbi.bV4BitCount + 0x1F) & ~0x1F)/8;

		// calculate the bitmap hash
		DWORD hash;
		hash = HashSurface((BYTE *)ddsd.lpSurface, ddsd.lPitch, ddsd.dwWidth, ddsd.dwHeight); 
		if(!hash) {
			OutTrace("TextureDump: lpdds=%x hash=NULL\n", s); 
			break; // almost certainly, an empty black surface!
		}

		// Create the .BMP file. 
		sprintf_s(pszFile, MAX_PATH, "%s\\texture.out\\texture.%03d.%03d.%s.%08X.bmp", 
			GetDxWndPath(), ddsd.dwWidth, ddsd.dwHeight, SurfaceType(ddsd.ddpfPixelFormat), hash);
		hf = fopen(pszFile, "wb");
		if(!hf) break;

		hdr.bfType = 0x4d42;        // 0x42 = "B" 0x4d = "M"  
		// Compute the size of the entire file.  
		hdr.bfSize = (DWORD) (sizeof(BITMAPFILEHEADER) + pbi.bV4Size + pbi.bV4ClrUsed * sizeof(RGBQUAD) + pbi.bV4SizeImage); 
		hdr.bfReserved1 = 0; 
		hdr.bfReserved2 = 0; 

		// Compute the offset to the array of color indices.  
		hdr.bfOffBits = (DWORD) sizeof(BITMAPFILEHEADER) + pbi.bV4Size + pbi.bV4ClrUsed * sizeof (RGBQUAD); 

		// Copy the BITMAPFILEHEADER into the .BMP file.  
		fwrite((LPVOID)&hdr, sizeof(BITMAPFILEHEADER), 1, hf);

		// Copy the BITMAPINFOHEADER array into the file.  
		fwrite((LPVOID)&pbi, sizeof(BITMAPV4HEADER), 1, hf);

		// Copy the RGBQUAD array into the file.  
		if(pbi.bV4ClrUsed){
			//DWORD PaletteEntries[256];
			//LPDIRECTDRAWSURFACE lpDDS=NULL;
			//LPDIRECTDRAWPALETTE lpDDP=NULL;
			//lpDDS=dxw.GetPrimarySurface();
			//if(lpDDS) lpDDS->GetPalette(&lpDDP);
			//if(lpDDP) lpDDP->GetEntries(0, 0, 256, (LPPALETTEENTRY)PaletteEntries);
			//for(int i=0; i<256; i++) PaletteEntries[i]=0xff0000;
			extern DWORD PaletteEntries[256];
			fwrite((LPVOID)PaletteEntries, pbi.bV4ClrUsed * sizeof (RGBQUAD), 1, hf);
			//fwrite((LPBYTE)PaletteEntries, pbi.bV4ClrUsed * sizeof (RGBQUAD), 1, hf);
		}

		// Copy the array of color indices into the .BMP file.  
		for(int y=0; y<(int)ddsd.dwHeight; y++)
			fwrite((BYTE *)ddsd.lpSurface + (y*ddsd.lPitch), iScanLineSize, 1, hf);

		// Close the .BMP file.  
		fclose(hf);
		break;
	}
	res=(*pUnlockMethod(s))(s, NULL);
	if (res) OutTraceE("TextureDump: Unlock ERROR lpdds=%x res=%x(%s) at %d\n", s, res, ExplainDDError(res), __LINE__);
}

static void TextureHack(LPDIRECTDRAWSURFACE s)
{
	DDSURFACEDESC2 ddsd;
	int w, h, iSurfaceSize, iScanLineSize;
	HRESULT res;

	memset(&ddsd,0,sizeof(DDSURFACEDESC2));
	ddsd.dwSize = Set_dwSize_From_Surface(s);
	ddsd.dwFlags = DDSD_LPSURFACE | DDSD_PITCH;
	if(res=(*pLock)(s, 0, (LPDDSURFACEDESC)&ddsd, DDLOCK_SURFACEMEMORYPTR|DDLOCK_WRITEONLY|DDLOCK_WAIT, 0)){	
		OutTraceE("TextureHack: Lock ERROR res=%x(%s) at %d\n", res, ExplainDDError(res), __LINE__);
		return;
	}
	extern LPDIRECTDRAWSURFACE lpDDSBack;
	if((ddsd.ddsCaps.dwCaps & DDSCAPS_TEXTURE) && (s != lpDDSBack)) while (TRUE) { // fake loop to ensure final Unlock
		OutTrace("TextureHack: lpdds=%x BitCount=%d size=(%dx%d)\n", 
			s, ddsd.ddpfPixelFormat.dwRGBBitCount, ddsd.dwWidth, ddsd.dwHeight);
		w = ddsd.dwWidth;
		h = ddsd.dwHeight;
		iSurfaceSize = ddsd.dwHeight * ddsd.lPitch;

		FILE *hf;
		BITMAPFILEHEADER hdr;       // bitmap file-header 
		BITMAPINFOHEADER pbi;		// bitmap info-header  
		char pszFile[81];
		int iSizeImage;

		memset((void *)&pbi, 0, sizeof(BITMAPINFOHEADER));
		pbi.biSize = sizeof(BITMAPINFOHEADER); 
		pbi.biWidth = ddsd.dwWidth;
		pbi.biHeight = ddsd.dwHeight;
		pbi.biBitCount = (WORD)ddsd.ddpfPixelFormat.dwRGBBitCount;
		pbi.biSizeImage = ((pbi.biWidth * pbi.biBitCount + 0x1F) & ~0x1F)/8 * pbi.biHeight; 
		iSizeImage = pbi.biSizeImage;
		iScanLineSize = ((pbi.biWidth * pbi.biBitCount + 0x1F) & ~0x1F)/8;

		// calculate the bitmap hash
		DWORD hash;
		hash = HashSurface((BYTE *)ddsd.lpSurface, ddsd.lPitch, ddsd.dwWidth, ddsd.dwHeight); 
		if(!hash) break; // almost certainly, an empty black surface!

		// Look for the .BMP file. 
		sprintf_s(pszFile, MAX_PATH, "%s\\texture.out\\texture.%03d.%03d.%s.%08X.bmp", 
			GetDxWndPath(), ddsd.dwWidth, ddsd.dwHeight, SurfaceType(ddsd.ddpfPixelFormat), hash);
		hf = fopen(pszFile, "rb");
		if(!hf) break; // no updated texture to load

		OutTrace("HASH bmp file %x\n", hf);

		while(TRUE) { // fake loop to ensure final fclose
			// Read the BITMAPFILEHEADER from the .BMP file (and throw away ...).  
			if(fread((LPVOID)&hdr, sizeof(BITMAPFILEHEADER), 1, hf) != 1)break;

			// Read the BITMAPINFOHEADER (and throw away ...).  
			// If the file contains BITMAPV4HEADER or BITMAPV5HEADER, no problem: next fseek will settle things
			if(fread((LPVOID)&pbi, sizeof(BITMAPINFOHEADER), 1, hf) != 1) break;

			// skip the RGBQUAD array if the editor inserted one
			fseek(hf, hdr.bfOffBits, SEEK_SET);

			// Read the new texture  from the .BMP file.  
			if(pbi.biHeight < 0){
				// biHeight < 0 -> scan lines from top to bottom, same as surface/texture convention
				for(int y=0; y<(int)ddsd.dwHeight; y++){
					BYTE *p = (BYTE *)ddsd.lpSurface + (ddsd.lPitch * y);
					fseek(hf, hdr.bfOffBits + (iScanLineSize * y), SEEK_SET);
					if(fread((LPVOID)p, ddsd.lPitch, 1, hf) != 1) break;
				}
			}
			else {
				// biHeight > 0 -> scan lines from bottom to top, inverse order as surface/texture convention
				for(int y=0; y<(int)ddsd.dwHeight; y++){
					BYTE *p = (BYTE *)ddsd.lpSurface + (ddsd.lPitch * ((ddsd.dwHeight-1) - y));
					fseek(hf, hdr.bfOffBits + (iScanLineSize * y), SEEK_SET);
					if(fread((LPVOID)p, ddsd.lPitch, 1, hf) != 1) break;
				}
			}
			OutTrace("TextureHack: TEXTURE LOAD DONE\n");
			break;
		}

		// Close the .BMP file.  
		fclose(hf);
		break;
	}
	res=(*pUnlockMethod(s))(s, NULL);
	if (res) OutTraceE("TextureHack: Unlock ERROR lpdds=%x res=%x(%s) at %d\n", s, res, ExplainDDError(res), __LINE__);
}

void TextureHandling(LPDIRECTDRAWSURFACE s)
{
	//OutTrace("TextureHandling(1-7): dxw.dwFlags5 = %x\n", dxw.dwFlags5 & (TEXTUREHIGHLIGHT|TEXTUREDUMP|TEXTUREHACK));
	switch(dxw.dwFlags5 & TEXTUREMASK){
		default:
		case TEXTUREHIGHLIGHT: 
			TextureHighlight(s);
			break;
		case TEXTUREDUMP: 
			TextureDump(s);
			break;
		case TEXTUREHACK:
			TextureHack(s);
			break;
		case TEXTURETRANSP:
			//TextureTransp(s);
			break;
	}
}
