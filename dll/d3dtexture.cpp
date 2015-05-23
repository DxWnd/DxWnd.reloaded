#define _CRT_SECURE_NO_WARNINGS

#include <d3d9.h>
#include "dxwnd.h"
#include "dxwcore.hpp"
#include "dxhook.h"
#include "dxhelper.h"
#include "syslibs.h"
#include "stdio.h"

extern unsigned int HashSurface(BYTE *, int, int, int);

char *ExplainD3DSurfaceFormat(DWORD dwFormat)
{
	char *s;
	switch(dwFormat){
		case D3DFMT_UNKNOWN: s = "UNKNOWN"; break;
		case D3DFMT_R8G8B8: s = "R8G8B8"; break;
		case D3DFMT_A8R8G8B8: s = "A8R8G8B8"; break;
		case D3DFMT_X8R8G8B8: s = "X8R8G8B8"; break;
		case D3DFMT_R5G6B5: s = "R5G6B5"; break;
		case D3DFMT_X1R5G5B5: s = "X1R5G5B5"; break;
		case D3DFMT_A1R5G5B5: s = "A1R5G5B5"; break;
		case D3DFMT_A4R4G4B4: s = "A4R4G4B4"; break;
		case D3DFMT_R3G3B2: s = "R3G3B2"; break;
		case D3DFMT_A8: s = "A8"; break;
		case D3DFMT_A8R3G3B2: s = "A8R3G3B2"; break;
		case D3DFMT_X4R4G4B4: s = "X4R4G4B4"; break;
		case D3DFMT_A2B10G10R10: s = "A2B10G10R10"; break;
		case D3DFMT_A8B8G8R8: s = "A8B8G8R8"; break;
		case D3DFMT_X8B8G8R8: s = "X8B8G8R8"; break;
		case D3DFMT_G16R16: s = "G16R16"; break;
		case D3DFMT_A2R10G10B10: s = "A2R10G10B10"; break;
		case D3DFMT_A16B16G16R16: s = "A16B16G16R16"; break;
		case D3DFMT_A8P8: s = "A8P8"; break;
		case D3DFMT_P8: s = "P8"; break;
		case D3DFMT_L8: s = "L8"; break;
		case D3DFMT_A8L8: s = "A8L8"; break;
		case D3DFMT_A4L4: s = "A4L4"; break;
		case D3DFMT_V8U8: s = "V8U8"; break;
		case D3DFMT_L6V5U5: s = "L6V5U5"; break;
		case D3DFMT_X8L8V8U8: s = "X8L8V8U8"; break;
		case D3DFMT_Q8W8V8U8: s = "Q8W8V8U8"; break;
		case D3DFMT_V16U16: s = "V16U16"; break;
		case D3DFMT_A2W10V10U10: s = "A2W10V10U10"; break;
		case D3DFMT_UYVY: s = "UYVY"; break;
		case D3DFMT_R8G8_B8G8: s = "R8G8_B8G8"; break;
		case D3DFMT_YUY2: s = "YUY2"; break;
		case D3DFMT_G8R8_G8B8: s = "G8R8_G8B8"; break;
		case D3DFMT_DXT1: s = "DXT1"; break;
		case D3DFMT_DXT2: s = "DXT2"; break;
		case D3DFMT_DXT3: s = "DXT3"; break;
		case D3DFMT_DXT4: s = "DXT4"; break;
		case D3DFMT_DXT5: s = "DXT5"; break;
		case D3DFMT_D16_LOCKABLE: s = "D16_LOCKABLE"; break;
		case D3DFMT_D32: s = "D32"; break;
		case D3DFMT_D15S1: s = "D15S1"; break;
		case D3DFMT_D24S8: s = "D24S8"; break;
		case D3DFMT_D24X8: s = "D24X8"; break;
		case D3DFMT_D24X4S4: s = "D24X4S4"; break;
		case D3DFMT_D16: s = "D16"; break;
		case D3DFMT_D32F_LOCKABLE: s = "D32F_LOCKABLE"; break;
		case D3DFMT_D24FS8: s = "D24FS8"; break;
		case D3DFMT_D32_LOCKABLE: s = "D32_LOCKABLE"; break;
		case D3DFMT_S8_LOCKABLE: s = "S8_LOCKABLE"; break;
		case D3DFMT_L16: s = "L16"; break;
		case D3DFMT_VERTEXDATA: s = "VERTEXDATA"; break;
		case D3DFMT_INDEX16: s = "INDEX16"; break;
		case D3DFMT_INDEX32: s = "INDEX32"; break;
		case D3DFMT_Q16W16V16U16: s = "Q16W16V16U16"; break;
		case D3DFMT_MULTI2_ARGB8: s = "MULTI2_ARGB8"; break;
		case D3DFMT_R16F: s = "R16F"; break;
		case D3DFMT_G16R16F: s = "G16R16F"; break;
		case D3DFMT_A16B16G16R16F: s = "A16B16G16R16F"; break;
		case D3DFMT_R32F: s = "R32F"; break;
		case D3DFMT_G32R32F: s = "G32R32F"; break;
		case D3DFMT_A32B32G32R32F: s = "A32B32G32R32F"; break;
		case D3DFMT_A1: s = "A1"; break;
		case D3DFMT_A2B10G10R10_XR_BIAS: s = "A2B10G10R10_XR_BIAS"; break;
		case D3DFMT_BINARYBUFFER: s = "BINARYBUFFER"; break;
		default: s = "Unknown"; break;
	}
	return s;
}

static WORD Melt_123(WORD c1, WORD c2)
{
	WORD r1, r2, g1, g2, b1, b2;
	WORD r, g, b;
	r1 = (c1 & 0xF800) >> 11;
	r2 = (c2 & 0xF800) >> 11;
	g1 = (c1 & 0x07E0) >> 5;
	g2 = (c2 & 0x07E0) >> 5;
	b1 = (c1 & 0x001F) >> 0;
	b2 = (c2 & 0x001F) >> 0;
	r = (((r2 * 2) + r1) / 3) & 0x1F;
	g = (((g2 * 2) + g1) / 3) & 0x3F;
	b = (((b2 * 2) + b1) / 3) & 0x1F;
	return (r << 11) | (g << 5) | (b << 0);
}

static DWORD Melt32_123(WORD c1, WORD c2)
{
	WORD r1, r2, g1, g2, b1, b2;
	DWORD r, g, b;
	r1 = (c1 & 0xF800) >> 11;
	r2 = (c2 & 0xF800) >> 11;
	g1 = (c1 & 0x07E0) >> 5;
	g2 = (c2 & 0x07E0) >> 5;
	b1 = (c1 & 0x001F) >> 0;
	b2 = (c2 & 0x001F) >> 0;
	r = (((r2 * 2) + r1) / 3) & 0x1F;
	g = (((g2 * 2) + g1) / 3) & 0x3F;
	b = (((b2 * 2) + b1) / 3) & 0x1F;
	return (r << (16+3)) | (g << (8+2)) | (b << (0+3));
}

static DWORD Conv32(WORD c)
{
	DWORD r, g, b;
	r = ((c & 0xF800) >> 11) & 0x1F;
	g = ((c & 0x07E0) >> 5) & 0x3F;
	b = ((c & 0x001F) >> 0) & 0x1F;
	return (r << (16+3)) | (g << (8+2)) | (b << (0+3));
}

#define GRIDSIZE 16

void D3DTextureHighlight(D3DSURFACE_DESC Desc, D3DLOCKED_RECT LockedRect)
{
	switch (Desc.Format){
		case D3DFMT_X8R8G8B8: 
		case D3DFMT_A8R8G8B8:
			{
				DWORD *p;
				DWORD color;
				color=(DWORD)(rand() & 0x00FFFFFF);
				for(UINT y=0; y<Desc.Height; y++){
					p = (DWORD *)LockedRect.pBits + ((y * LockedRect.Pitch) >> 2);
					for(UINT x=0; x<Desc.Width; x++) *(p+x) = color;
				}
				for(UINT y=0; y<Desc.Height; y++){
					p = (DWORD *)LockedRect.pBits + ((y * LockedRect.Pitch) >> 2);
					for(UINT x=0; x<Desc.Width; x+=GRIDSIZE) *(p+x) = 0;
					if((y%GRIDSIZE)==0) for(UINT x=0; x<Desc.Width; x++) *(p++) = 0;
				}
			}
			break;
		case D3DFMT_DXT3:
			//if(Desc.Height) for(UINT y=0; y<(Desc.Height>>2); y+=2)
			//	memset((BYTE *)LockedRect.pBits + (y * LockedRect.Pitch), 0x00, LockedRect.Pitch);
		default:
			break;
	}
}

void D3DTextureDump(D3DSURFACE_DESC Desc, D3DLOCKED_RECT LockedRect)
{
	static BOOL DoOnce = TRUE;
	static int MinTexX, MinTexY, MaxTexX, MaxTexY;
	FILE *hf;
	BITMAPFILEHEADER hdr;       // bitmap file-header 
	BITMAPV4HEADER pbi;			// bitmap info-header  
	int w, h; 
	int iSurfaceSize, iScanLineSize;
	char pszFile[MAX_PATH];
	DWORD hash;

	if(DoOnce){
		char sProfilePath[MAX_PATH];
		extern char *GetDxWndPath();
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
	
	while (TRUE){ // fake loop 1
		w = Desc.Width;
		h = Desc.Height;
		if((MinTexX && (w<MinTexX)) || (MinTexY && (h<MinTexY))) {
			OutTrace("TextureDump: SKIP small texture\n");
			break;
		}
		if((MaxTexX && (w>MaxTexX)) || (MaxTexY && (h>MaxTexY))) {
			OutTrace("TextureDump: SKIP big texture\n");
			break;
		}
		iSurfaceSize = Desc.Height * LockedRect.Pitch;

		// calculate the bitmap hash
		hash = 0;
		switch (Desc.Format){
			case D3DFMT_A4R4G4B4:
			case D3DFMT_X4R4G4B4:
			case D3DFMT_X8R8G8B8: 
			case D3DFMT_A8R8G8B8: 
			case D3DFMT_R5G6B5:
			case D3DFMT_X1R5G5B5:
			case D3DFMT_A1R5G5B5:
			case D3DFMT_A8:
			case D3DFMT_L8:
				hash = HashSurface((BYTE *)LockedRect.pBits, LockedRect.Pitch, Desc.Width, Desc.Height);
				break;
			case D3DFMT_DXT1: 
				hash = HashSurface((BYTE *)LockedRect.pBits, LockedRect.Pitch / 6, Desc.Width / 6, Desc.Height); 
				break;
			case D3DFMT_DXT2: 
			case D3DFMT_DXT3: 
				hash = HashSurface((BYTE *)LockedRect.pBits, LockedRect.Pitch / 4, Desc.Width / 4, Desc.Height); 
				break;
			case D3DFMT_DXT4: 
			case D3DFMT_DXT5: 
				hash = HashSurface((BYTE *)LockedRect.pBits, LockedRect.Pitch / 4, Desc.Width / 4, Desc.Height); 
				break;
			case D3DFMT_V8U8:
			case D3DFMT_Q8W8V8U8: // Tiger Woods PGA Tour 08
			case D3DFMT_V16U16:
			case D3DFMT_Q16W16V16U16:
			case D3DFMT_CxV8U8:
			case D3DFMT_L6V5U5:
			case D3DFMT_X8L8V8U8:
			case D3DFMT_A2W10V10U10:
				// Bumpmap surfaces, dump is meaningless .....
				break;
			default:
				char sMsg[80+1];
				static BOOL DoOnce = TRUE;
				if(DoOnce){
					sprintf_s(sMsg, 80, "Unhandled texture type=%d(%s)", Desc.Format, ExplainD3DSurfaceFormat(Desc.Format));
					MessageBox(0, sMsg, "WARN", MB_OK | MB_ICONEXCLAMATION);
					DoOnce = FALSE;
				}
				break;
		}

		if(!hash) {
			OutTrace("TextureDump: hash=NULL\n"); 
			break; // almost certainly, an empty black surface!
		}

		// Create the .BMP file. 
		extern char *GetDxWndPath();
		sprintf_s(pszFile, MAX_PATH, "%s\\texture.out\\texture.%03d.%03d.%s.%08X.bmp", 
			GetDxWndPath(), Desc.Width, Desc.Height, ExplainD3DSurfaceFormat(Desc.Format), hash);
		hf = fopen(pszFile, "wb");
		if(!hf) break;

		// set bmp invariant parameters
		memset((void *)&pbi, 0, sizeof(BITMAPV4HEADER));
		pbi.bV4Size = sizeof(BITMAPV4HEADER); 
		pbi.bV4Width = Desc.Width;
		pbi.bV4Height = Desc.Height;
		pbi.bV4Planes = 1;
		pbi.bV4V4Compression = BI_BITFIELDS;
		pbi.bV4XPelsPerMeter = 1;
		pbi.bV4YPelsPerMeter = 1;
		pbi.bV4ClrUsed = 0;
		pbi.bV4ClrImportant = 0;
		pbi.bV4CSType = LCS_CALIBRATED_RGB;
		hdr.bfType = 0x4d42;        // 0x42 = "B" 0x4d = "M"  
		hdr.bfReserved1 = 0; 
		hdr.bfReserved2 = 0; 

		switch (Desc.Format){
			case D3DFMT_X8R8G8B8: 
			case D3DFMT_A8R8G8B8: 
			case D3DFMT_DXT2:
			case D3DFMT_DXT3:
			case D3DFMT_DXT4:
			case D3DFMT_DXT5:
			case D3DFMT_A8:
			case D3DFMT_L8:
				pbi.bV4BitCount = 32;
				pbi.bV4RedMask = 0x00FF0000;
				pbi.bV4GreenMask = 0x0000FF00;
				pbi.bV4BlueMask = 0x000000FF;
				pbi.bV4AlphaMask = 0xFF000000;
				break;
			case D3DFMT_DXT1:
				pbi.bV4BitCount = 16;
				pbi.bV4RedMask = 0xF800;
				pbi.bV4GreenMask = 0x07E0;
				pbi.bV4BlueMask = 0x001F;
				pbi.bV4AlphaMask = 0x0000;
				break;
			case D3DFMT_A4R4G4B4: // AoE III
			case D3DFMT_X4R4G4B4:
				pbi.bV4BitCount = 16;
				pbi.bV4RedMask = 0x0F00;
				pbi.bV4GreenMask = 0x00F0;
				pbi.bV4BlueMask = 0x000F;
				pbi.bV4AlphaMask = 0xF000;
				break;
			case D3DFMT_A1R5G5B5: //  AoE III
			case D3DFMT_X1R5G5B5:
				pbi.bV4BitCount = 16;
				pbi.bV4RedMask = 0x7C00;
				pbi.bV4GreenMask = 0x03E0;
				pbi.bV4BlueMask = 0x001F;
				pbi.bV4AlphaMask = 0x8000;
				break;		
			case D3DFMT_R5G6B5:
				pbi.bV4BitCount = 16;
				pbi.bV4RedMask = 0x7C00;
				pbi.bV4GreenMask = 0x03E0;
				pbi.bV4BlueMask = 0x001F;
				pbi.bV4AlphaMask = 0x0000;
				break;		
		}

		pbi.bV4SizeImage = ((pbi.bV4Width * pbi.bV4BitCount + 0x1F) & ~0x1F)/8 * pbi.bV4Height; 
		iScanLineSize = ((pbi.bV4Width * pbi.bV4BitCount + 0x1F) & ~0x1F)/8;
		pbi.bV4Height = - pbi.bV4Height;

		// Compute the size of the entire file.  
		hdr.bfSize = (DWORD) (sizeof(BITMAPFILEHEADER) + pbi.bV4Size + pbi.bV4ClrUsed * sizeof(RGBQUAD) + pbi.bV4SizeImage); 

		// Compute the offset to the array of color indices.  
		hdr.bfOffBits = (DWORD) sizeof(BITMAPFILEHEADER) + pbi.bV4Size + pbi.bV4ClrUsed * sizeof (RGBQUAD); 

		// Copy the BITMAPFILEHEADER into the .BMP file.  
		fwrite((LPVOID)&hdr, sizeof(BITMAPFILEHEADER), 1, hf);

		// Copy the BITMAPINFOHEADER and RGBQUAD array into the file.  
		fwrite((LPVOID)&pbi, sizeof(BITMAPV4HEADER) + pbi.bV4ClrUsed * sizeof (RGBQUAD), 1, hf);

		switch (Desc.Format){
			case D3DFMT_X8R8G8B8: 
			case D3DFMT_A8R8G8B8: 
			case D3DFMT_A4R4G4B4:
			case D3DFMT_X4R4G4B4:
			case D3DFMT_A1R5G5B5:	//  AoE III
			case D3DFMT_R5G6B5:		//  AoE III
			case D3DFMT_X1R5G5B5:
				{
					// Copy the array of color indices into the .BMP file.  
					for(int y=0; y<(int)Desc.Height; y++)
						fwrite((BYTE *)LockedRect.pBits + (y*LockedRect.Pitch), iScanLineSize, 1, hf);
				}
				break;
			case D3DFMT_A8:
			case D3DFMT_L8:
				{
					// Copy the array of color indices into the .BMP file.  
					BYTE *p = (BYTE *)LockedRect.pBits;
					for(int y=0; y<(int)Desc.Height; y++)
						for(int x=0; x<(int)Desc.Width; x++){
							DWORD pixel;
							pixel = 0xFF000000 | *p | (*p << 8) | (*p << 16); // gray color
							fwrite((BYTE *)&pixel, sizeof(DWORD), 1, hf);
							p++;
						}
				}
				break;
			case D3DFMT_DXT1: 
				{
					// Copy the array of color indices into the .BMP file.  
					WORD *bm;
					WORD *c;
					int bmsize;
					c = (WORD *)LockedRect.pBits;
					bmsize = Desc.Width * Desc.Height * sizeof(WORD);
					bm = (WORD *)malloc(bmsize);
					for(int y=0; y<(int)Desc.Height; y+=4){
						for(int x=0; x<(int)Desc.Width; x+=4){
							WORD color_0, color_1, color_2, color_3;
							color_0 = *c++;
							color_1 = *c++;
							color_2 = Melt_123(color_1, color_0);
							color_3 = Melt_123(color_0, color_1);
							for(int n=0; n<2; n++){
								int dy;
								WORD color_indexes = *c++;
								WORD color;
								for (int m=0; m<8; m++){
									switch (color_indexes & 0x3){
										case 0x00: color = color_0; break;
										case 0x01: color = color_1; break;
										case 0x02: color = color_2; break;
										case 0x03: color = color_3; break;
									}
									dy = (m<4) ? 0 : 1;
									color_indexes >>= 2;
									int index = ((y+(2*n)+dy)*Desc.Width) + (x+(m%4));
									if(index < bmsize/2) bm[index]=color;
								}
							}
						}
					}
					fwrite((BYTE *)bm, bmsize, 1, hf);
					free(bm);
				}
				break;
			case D3DFMT_DXT2:
			case D3DFMT_DXT3:
				{
					// Copy the array of color indices into the .BMP file.  
					DWORD *bm;
					WORD *c;
					int bmsize;
					c = (WORD *)LockedRect.pBits;
					bmsize = Desc.Width * Desc.Height * sizeof(DWORD);
					bm = (DWORD *)malloc(bmsize);
					memset(bm, 0, bmsize);
					for(int y=0; y<(int)Desc.Height; y+=4){
						for(int x=0; x<(int)Desc.Width; x+=4){
							WORD color_0, color_1;
							DWORD dwcolor[4];
							BYTE alpha[16];
							WORD *pAlpha;
							pAlpha = (WORD *)c;
							for(int row=0; row<4; row++){
								WORD a = *c++;
								for(int col=0; col<4; col++){
									alpha[(row<<2)+col] = (a & 0xF);
									a >>= 4;
								}
							}
							color_0 = *c++;
							color_1 = *c++;
							dwcolor[0] = Conv32(color_0);
							dwcolor[1] = Conv32(color_1);
							dwcolor[2] = Melt32_123(color_1, color_0);
							dwcolor[3] = Melt32_123(color_0, color_1);
							for(int n=0; n<2; n++){
								int dy;
								WORD color_indexes = *c++;
								DWORD color;
								for (int m=0; m<8; m++){
									dy = (m<4) ? 0 : 1;
									color = dwcolor[color_indexes & 0x3] & 0x00FFFFFF;
									dy = (m<4) ? 0 : 1;
									color_indexes >>= 2;
									int index = ((y+(2*n)+dy)*Desc.Width) + (x+(m%4));
									int alpha_index = (((2*n) + dy)<<2) + (m % 4);
									//if(index < bmsize/4) bm[index] = color | 0xFF000000; // uncomment to get rid of alpha channel
									if(index < bmsize/4) bm[index] = color | (alpha[alpha_index]<<28);
								}
							}						
						}
					}
					fwrite((BYTE *)bm, bmsize, 1, hf);
					free(bm);
				}
				break;
			case D3DFMT_DXT4:
			case D3DFMT_DXT5:
				{
					// Copy the array of color indices into the .BMP file.  
					DWORD *bm;
					WORD *c;
					int bmsize;
					c = (WORD *)LockedRect.pBits;
					bmsize = Desc.Width * Desc.Height * sizeof(DWORD);
					bm = (DWORD *)malloc(bmsize);
					memset(bm, 0, bmsize);
					for(int y=0; y<(int)Desc.Height; y+=4){
						for(int x=0; x<(int)Desc.Width; x+=4){
							WORD color_0, color_1;
							DWORD dwcolor[4];
							DWORD alpha[8];
							//  alpha section (4 words) ....
							alpha[1] = ((*c) >> 8) & 0xFF;
							alpha[0] = (*c) & 0xFF;
							c++;
							if(alpha[0] > alpha[1])
							{
								// 6 interpolated alpha values.
								alpha[2] = ((6*alpha[0]) + (1*alpha[1])) / 7; // bit code 010
								alpha[3] = ((5*alpha[0]) + (2*alpha[1])) / 7; // bit code 011
								alpha[4] = ((4*alpha[0]) + (3*alpha[1])) / 7; // bit code 100
								alpha[5] = ((3*alpha[0]) + (4*alpha[1])) / 7; // bit code 101
								alpha[6] = ((2*alpha[0]) + (5*alpha[1])) / 7; // bit code 110
								alpha[7] = ((1*alpha[0]) + (6*alpha[1])) / 7; // bit code 111
							}
							else 
							{
								// 4 interpolated alpha values.
								alpha[2] = ((4*alpha[0]) + (1*alpha[1])) / 5; // bit code 010
								alpha[3] = ((3*alpha[0]) + (2*alpha[1])) / 5; // bit code 011
								alpha[4] = ((2*alpha[0]) + (3*alpha[1])) / 5; // bit code 100
								alpha[5] = ((1*alpha[0]) + (4*alpha[1])) / 5; // bit code 101
								alpha[6] = 0x00;                              // bit code 110 (fully transparent)
								alpha[7] = 0xFF;                              // bit code 111 (fully opaque)
							}
							BYTE *pAlpha = (BYTE *)c;
							c += 3;
							color_0 = *c++;
							color_1 = *c++;
							dwcolor[0] = Conv32(color_0);
							dwcolor[1] = Conv32(color_1);
							dwcolor[2] = Melt32_123(color_1, color_0);
							dwcolor[3] = Melt32_123(color_0, color_1);
							for(int n=0; n<2; n++){
								int dy;
								DWORD dwAlpha_indexes = (*(pAlpha+0) & 0x0000FF) | ((*(pAlpha+1)<<8) & 0x00FF00) | ((*(pAlpha+2)<<16) & 0xFF0000);
								pAlpha += 3;
								DWORD alpha_color;
								WORD color_indexes = *c++;
								DWORD color;
								for (int m=0; m<8; m++){
									alpha_color = (alpha[dwAlpha_indexes & 0x7] << 24) & 0xFF000000;
									dy = (m<4) ? 0 : 1;
									dwAlpha_indexes >>= 3;
									color = dwcolor[color_indexes & 0x3] & 0x00FFFFFF;
									dy = (m<4) ? 0 : 1;
									color_indexes >>= 2;
									int index = ((y+(2*n)+dy)*Desc.Width) + (x+(m%4));
									if(index < bmsize/4) bm[index] = color | alpha_color;
								}
							}						
						}
					}
					fwrite((BYTE *)bm, bmsize, 1, hf);
					free(bm);	
				}
			default:
				break;
		}
		// Close the .BMP file.  
		if(hf) fclose(hf);
		break;
	} // end of fake loop 1
}

void D3DTextureHack(D3DSURFACE_DESC Desc, D3DLOCKED_RECT LockedRect)
{
}

void D3DTextureTransp(D3DSURFACE_DESC Desc, D3DLOCKED_RECT LockedRect)
{
}
