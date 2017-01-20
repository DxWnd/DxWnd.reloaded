#define _CRT_SECURE_NO_WARNINGS

#include "dxwnd.h"
#include "dxwcore.hpp"
#include "syslibs.h"
#include "hddraw.h"
#include "dxhook.h"
#include "dxhelper.h"
#include "shareddc.hpp"

#include "stdio.h"

void DumpDibSection(const BITMAPINFO *pbmi, UINT iUsage, VOID *pvBits)
{
	static int prog=0;
	FILE *fdump;
	char path[81];
	BITMAPFILEHEADER hdr;       // bitmap file-header 
	BITMAPV4HEADER pbi;			// bitmap info-header  
	int iScanLineSize;

	if(iUsage != DIB_RGB_COLORS) return;
	if(prog==0) CreateDirectory(".\\bmp.out", NULL);
	sprintf(path,".\\bmp.out\\dib.%08.8d.bmp", prog);
	fdump=fopen(path, "wb");
	if(!fdump) return;
	memset((void *)&pbi, 0, sizeof(BITMAPV4HEADER));
	pbi.bV4Size = sizeof(BITMAPV4HEADER); 
	pbi.bV4Width = pbmi->bmiHeader.biWidth;
	pbi.bV4Height = pbmi->bmiHeader.biHeight;
	pbi.bV4BitCount = pbmi->bmiHeader.biBitCount;
	pbi.bV4SizeImage = ((pbi.bV4Width * pbi.bV4BitCount + 0x1F) & ~0x1F)/8 * pbi.bV4Height; 
	pbi.bV4Height = - pbi.bV4Height;
	pbi.bV4Planes = pbmi->bmiHeader.biPlanes;
	pbi.bV4V4Compression = pbmi->bmiHeader.biCompression;
	pbi.bV4XPelsPerMeter = 1;
	pbi.bV4YPelsPerMeter = 1;
	pbi.bV4ClrUsed = pbmi->bmiHeader.biClrUsed;
	if(!pbi.bV4ClrUsed) pbi.bV4ClrUsed = 1 << pbi.bV4BitCount;
	pbi.bV4ClrImportant = pbmi->bmiHeader.biClrImportant;
	pbi.bV4RedMask = 0;
	pbi.bV4RedMask = 0;
	pbi.bV4GreenMask = 0;
	pbi.bV4BlueMask = 0;
	pbi.bV4AlphaMask = 0;
	pbi.bV4CSType = LCS_CALIBRATED_RGB;
	iScanLineSize = ((pbi.bV4Width * pbi.bV4BitCount + 0x1F) & ~0x1F)/8;
	OutTrace("DumpDibSection: prog=%d size=%d wxh=(%dx%d) bc=%d sizeimg=%d planes=%d comp=%x ppm=(%dx%d) colors=%d imp=%d\n",
		prog,
		pbi.bV4Size, pbi.bV4Width, pbi.bV4Height, pbi.bV4BitCount, pbi.bV4SizeImage,
		pbi.bV4Planes, pbi.bV4V4Compression, pbi.bV4XPelsPerMeter, pbi.bV4YPelsPerMeter,
		pbi.bV4ClrUsed, pbi.bV4ClrImportant);

	prog++;
	hdr.bfType = 0x4d42;        // 0x42 = "B" 0x4d = "M"  
	// Compute the size of the entire file.  
	hdr.bfSize = (DWORD) (sizeof(BITMAPFILEHEADER) + pbi.bV4Size + pbi.bV4ClrUsed * sizeof(RGBQUAD) + pbi.bV4SizeImage); 
	hdr.bfReserved1 = 0; 
	hdr.bfReserved2 = 0; 

	// Compute the offset to the array of color indices.  
	hdr.bfOffBits = (DWORD) sizeof(BITMAPFILEHEADER) + pbi.bV4Size + pbi.bV4ClrUsed * sizeof (RGBQUAD); 

	// Copy the BITMAPFILEHEADER into the .BMP file.  
	fwrite((LPVOID)&hdr, sizeof(BITMAPFILEHEADER), 1, fdump);

	// Copy the BITMAPINFOHEADER array into the file.  
	fwrite((LPVOID)&pbi, sizeof(BITMAPV4HEADER), 1, fdump);

	// Copy the RGBQUAD array into the file.  
	if(pbi.bV4ClrUsed) fwrite(&pbmi->bmiColors[0], pbi.bV4ClrUsed * sizeof (RGBQUAD), 1, fdump);

	// Copy the array of color indices into the .BMP file.  
	//for(int y=0; y<(int)ddsd.dwHeight; y++)
	//	fwrite((BYTE *)ddsd.lpSurface + (y*ddsd.lPitch), iScanLineSize, 1, fdump);
	fwrite((BYTE *)pvBits, pbi.bV4SizeImage, 1, fdump);

	// Close the .BMP file.  
	fclose(fdump);
}

void DumpHDC(HDC hdc, int x0, int y0, int w, int h)
{
	FILE *fdump;
	static int prog = 0;
	char path[81];
	BITMAPFILEHEADER  hdr;
	BITMAPV4HEADER pbi;			// bitmap info-header  

	if (w<0) w = -w;
	if (h<0) h = -h;

	if(prog==0) CreateDirectory(".\\bmp.out", NULL);
	sprintf(path,".\\bmp.out\\hdc.%08.8d.bmp", prog);
	fdump=fopen(path, "wb");
	if(!fdump) {
		OutTrace("DumpHDC: err=%d\n", GetLastError());
		return;
	}

	// this bitmap is not created in the same format of the original HDC: using the GetPixel call to read the pixels
	// means that the resulting image will be 32bpp color depth

	memset((void *)&pbi, 0, sizeof(BITMAPV4HEADER));
	pbi.bV4Size = sizeof(BITMAPV4HEADER); 
	pbi.bV4Width = w;
	pbi.bV4Height = h;
	pbi.bV4BitCount = 32;
	pbi.bV4SizeImage = ((pbi.bV4Width * pbi.bV4BitCount + 0x1F) & ~0x1F)/8 * pbi.bV4Height; 
	pbi.bV4Height = -pbi.bV4Height;
	pbi.bV4Planes = 1;
	pbi.bV4V4Compression = BI_BITFIELDS;
	pbi.bV4XPelsPerMeter = 1;
	pbi.bV4YPelsPerMeter = 1;
	pbi.bV4ClrUsed = 0;
	pbi.bV4ClrImportant = 0;
	pbi.bV4RedMask = 0x000000FF;
	pbi.bV4GreenMask = 0x0000FF00;
	pbi.bV4BlueMask = 0x00FF0000;
	pbi.bV4AlphaMask = 0x00000000;
	pbi.bV4CSType = LCS_CALIBRATED_RGB;
	int iScanLineSize = ((pbi.bV4Width * pbi.bV4BitCount + 0x1F) & ~0x1F)/8;
	OutTrace("DumpDevContext: prog=%08.8d size=%d wxh=(%dx%d) bc=%d sizeimg=%d planes=%d comp=%x ppm=(%dx%d) colors=%d imp=%d\n",
		prog,
		pbi.bV4Size, pbi.bV4Width, pbi.bV4Height, pbi.bV4BitCount, pbi.bV4SizeImage,
		pbi.bV4Planes, pbi.bV4V4Compression, pbi.bV4XPelsPerMeter, pbi.bV4YPelsPerMeter,
		pbi.bV4ClrUsed, pbi.bV4ClrImportant);

	prog++;
	// Fill in the fields of the file header 
	hdr.bfType        = ((WORD) ('M' << 8) | 'B');    // is always "BM"
	hdr.bfSize        = (w * h) + sizeof( hdr );
	hdr.bfReserved1   = 0;
	hdr.bfReserved2   = 0;
	hdr.bfOffBits     = (DWORD)(sizeof(hdr) + sizeof(pbi));

	// Write the file header 
	fwrite( &hdr, sizeof(hdr), 1, fdump);
	// Write the DIB header
	fwrite( &pbi, sizeof(pbi), 1, fdump );
	// Write the bits 
	for(int y=y0; y<y0+h; y++)
		for(int x=x0; x<x0+w; x++){
			DWORD pixel=GetPixel(hdc, x, y);
			fwrite(&pixel, sizeof(DWORD), 1, fdump);
		}

	fclose(fdump);
}

// eof