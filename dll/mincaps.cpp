#define _CRT_SECURE_NO_WARNINGS
#define INITGUID

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

#define VIRTUAL_CAPS_VIRTUALPC TRUE

#if 0
typedef struct _DDCAPS_DX7
{
/*  0*/ DWORD   dwSize;                 // size of the DDDRIVERCAPS structure
/*  4*/ DWORD   dwCaps;                 // driver specific capabilities
/*  8*/ DWORD   dwCaps2;                // more driver specific capabilites
/*  c*/ DWORD   dwCKeyCaps;             // color key capabilities of the surface
/* 10*/ DWORD   dwFXCaps;               // driver specific stretching and effects capabilites
/* 14*/ DWORD   dwFXAlphaCaps;          // alpha driver specific capabilities
/* 18*/ DWORD   dwPalCaps;              // palette capabilities
/* 1c*/ DWORD   dwSVCaps;               // stereo vision capabilities
/* 20*/ DWORD   dwAlphaBltConstBitDepths;       // DDBD_2,4,8
/* 24*/ DWORD   dwAlphaBltPixelBitDepths;       // DDBD_1,2,4,8
/* 28*/ DWORD   dwAlphaBltSurfaceBitDepths;     // DDBD_1,2,4,8
/* 2c*/ DWORD   dwAlphaOverlayConstBitDepths;   // DDBD_2,4,8
/* 30*/ DWORD   dwAlphaOverlayPixelBitDepths;   // DDBD_1,2,4,8
/* 34*/ DWORD   dwAlphaOverlaySurfaceBitDepths; // DDBD_1,2,4,8
/* 38*/ DWORD   dwZBufferBitDepths;             // DDBD_8,16,24,32
/* 3c*/ DWORD   dwVidMemTotal;          // total amount of video memory
/* 40*/ DWORD   dwVidMemFree;           // amount of free video memory
/* 44*/ DWORD   dwMaxVisibleOverlays;   // maximum number of visible overlays
/* 48*/ DWORD   dwCurrVisibleOverlays;  // current number of visible overlays
/* 4c*/ DWORD   dwNumFourCCCodes;       // number of four cc codes
/* 50*/ DWORD   dwAlignBoundarySrc;     // source rectangle alignment
/* 54*/ DWORD   dwAlignSizeSrc;         // source rectangle byte size
/* 58*/ DWORD   dwAlignBoundaryDest;    // dest rectangle alignment
/* 5c*/ DWORD   dwAlignSizeDest;        // dest rectangle byte size
/* 60*/ DWORD   dwAlignStrideAlign;     // stride alignment
/* 64*/ DWORD   dwRops[DD_ROP_SPACE];   // ROPS supported
/* 84*/ DDSCAPS ddsOldCaps;             // Was DDSCAPS  ddsCaps. ddsCaps is of type DDSCAPS2 for DX6
/* 88*/ DWORD   dwMinOverlayStretch;    // minimum overlay stretch factor multiplied by 1000, eg 1000 == 1.0, 1300 == 1.3
/* 8c*/ DWORD   dwMaxOverlayStretch;    // maximum overlay stretch factor multiplied by 1000, eg 1000 == 1.0, 1300 == 1.3
/* 90*/ DWORD   dwMinLiveVideoStretch;  // minimum live video stretch factor multiplied by 1000, eg 1000 == 1.0, 1300 == 1.3
/* 94*/ DWORD   dwMaxLiveVideoStretch;  // maximum live video stretch factor multiplied by 1000, eg 1000 == 1.0, 1300 == 1.3
/* 98*/ DWORD   dwMinHwCodecStretch;    // minimum hardware codec stretch factor multiplied by 1000, eg 1000 == 1.0, 1300 == 1.3
/* 9c*/ DWORD   dwMaxHwCodecStretch;    // maximum hardware codec stretch factor multiplied by 1000, eg 1000 == 1.0, 1300 == 1.3
/* a0*/ DWORD   dwReserved1;            // reserved
/* a4*/ DWORD   dwReserved2;            // reserved
/* a8*/ DWORD   dwReserved3;            // reserved
/* ac*/ DWORD   dwSVBCaps;              // driver specific capabilities for System->Vmem blts
/* b0*/ DWORD   dwSVBCKeyCaps;          // driver color key capabilities for System->Vmem blts
/* b4*/ DWORD   dwSVBFXCaps;            // driver FX capabilities for System->Vmem blts
/* b8*/ DWORD   dwSVBRops[DD_ROP_SPACE];// ROPS supported for System->Vmem blts
/* d8*/ DWORD   dwVSBCaps;              // driver specific capabilities for Vmem->System blts
/* dc*/ DWORD   dwVSBCKeyCaps;          // driver color key capabilities for Vmem->System blts
/* e0*/ DWORD   dwVSBFXCaps;            // driver FX capabilities for Vmem->System blts
/* e4*/ DWORD   dwVSBRops[DD_ROP_SPACE];// ROPS supported for Vmem->System blts
/*104*/ DWORD   dwSSBCaps;              // driver specific capabilities for System->System blts
/*108*/ DWORD   dwSSBCKeyCaps;          // driver color key capabilities for System->System blts
/*10c*/ DWORD   dwSSBFXCaps;            // driver FX capabilities for System->System blts
/*110*/ DWORD   dwSSBRops[DD_ROP_SPACE];// ROPS supported for System->System blts
/*130*/ DWORD   dwMaxVideoPorts;        // maximum number of usable video ports
/*134*/ DWORD   dwCurrVideoPorts;       // current number of video ports used
/*138*/ DWORD   dwSVBCaps2;             // more driver specific capabilities for System->Vmem blts
/*13c*/ DWORD   dwNLVBCaps;               // driver specific capabilities for non-local->local vidmem blts
/*140*/ DWORD   dwNLVBCaps2;              // more driver specific capabilities non-local->local vidmem blts
/*144*/ DWORD   dwNLVBCKeyCaps;           // driver color key capabilities for non-local->local vidmem blts
/*148*/ DWORD   dwNLVBFXCaps;             // driver FX capabilities for non-local->local blts
/*14c*/ DWORD   dwNLVBRops[DD_ROP_SPACE]; // ROPS supported for non-local->local blts
// Members added for DX6 release
/*16c*/ DDSCAPS2 ddsCaps;               // Surface Caps
} DDCAPS_DX7;
typedef DDCAPS_DX7 FAR* LPDDCAPS_DX7;
#endif

void SetMinimalCaps(LPDDCAPS c1, LPDDCAPS c2)
{
#ifdef VIRTUAL_CAPS_VMWARE
	if(c1){
		int dwSize = c1->dwSize;
		//memset(&c1, 0, dwSize);
		c1->dwSize		= dwSize;
		c1->dwCaps		= (DDCAPS_3D|DDCAPS_BLT|DDCAPS_BLTQUEUE|DDCAPS_OVERLAY|DDCAPS_OVERLAYFOURCC|DDCAPS_OVERLAYSTRETCH|DDCAPS_COLORKEY|DDCAPS_BLTDEPTHFILL|DDCAPS_CANBLTSYSMEM);
		c1->dwCaps2		= (DDCAPS2_WIDESURFACES|DDCAPS2_CANRENDERWINDOWED|DDCAPS2_FLIPNOVSYNC);
		c1->dwCKeyCaps	= (DDCKEYCAPS_DESTOVERLAY);
		c1->dwFXCaps	= (DDFXCAPS_OVERLAYSHRINKX|DDFXCAPS_OVERLAYSHRINKY|DDFXCAPS_OVERLAYSTRETCHX|DDFXCAPS_OVERLAYSTRETCHY);
		c1->dwZBufferBitDepths = 0x600;
		c1->dwMaxVisibleOverlays = 0x20;
		for(int j=0; j<DD_ROP_SPACE; j++) c1->dwRops[j]=0;
		c1->dwRops[6]	= 0x100;
		if(dwSize >= (int)(0x16C)) {
			c1->ddsCaps.dwCaps = 0x42727A;
			c1->ddsCaps.dwCaps2 = 0x200200;
		}
		// c1->dwNumFourCCCodes - untouched (03 in VmWare)
	}
	if(c2){
		int dwSize = c2->dwSize;
		//memset(&c2, 0, dwSize);
		c2->dwSize		= dwSize;
		c2->dwCaps		= (DDCAPS_3D|DDCAPS_BLT|DDCAPS_BLTSTRETCH|DDCAPS_PALETTE|DDCAPS_COLORKEY|DDCAPS_ALPHA|DDCAPS_BLTCOLORFILL|DDCAPS_BLTDEPTHFILL|DDCAPS_CANCLIP|DDCAPS_CANCLIPSTRETCHED|DDCAPS_CANBLTSYSMEM);
		c2->dwCaps2		= (DDCAPS2_CERTIFIED);
		c2->dwCKeyCaps	=(DDCKEYCAPS_SRCBLT);
		c2->dwPalCaps	= 0x347; // ???
		c2->dwFXCaps	= (DDFXCAPS_BLTARITHSTRETCHY|DDFXCAPS_BLTMIRRORLEFTRIGHT|DDFXCAPS_BLTMIRRORUPDOWN|DDFXCAPS_BLTSHRINKX|DDFXCAPS_BLTSHRINKXN|DDFXCAPS_BLTSHRINKY|DDFXCAPS_BLTSHRINKYN|DDFXCAPS_BLTSTRETCHX|DDFXCAPS_BLTSTRETCHXN|DDFXCAPS_BLTSTRETCHY|DDFXCAPS_BLTSTRETCHYN|DDFXCAPS_BLTALPHA|DDFXCAPS_BLTFILTER|DDFXCAPS_BLTARITHSTRETCHY);
		for(int j=0; j<DD_ROP_SPACE; j++) c2->dwRops[j]=0;
		c2->dwRops[0]	= 0x1;
		c2->dwRops[6]	= 0x100;
		c2->dwRops[7]	= 0x8000000;
		c2->ddsOldCaps.dwCaps	= 0x421350;
		// VMWare specific?
		c2->dwSVBCaps	= 0xF4C08241;
		c2->dwSVBCKeyCaps = 0x200; 
		c2->dwSVBFXCaps = 0x3FCE3 ;
		for(int j=0; j<DD_ROP_SPACE; j++) c2->dwSVBRops[j]=0;
		c2->dwSVBRops[0] = 0x1;
		c2->dwSVBRops[6]	= 0x100;
		c2->dwSVBRops[7]	= 0x8000000;
		c2->dwVSBCaps = 0xF4C08241;
		c2->dwVSBCKeyCaps = 0x200;
		c2->dwVSBFXCaps = 0x3FCE3;
		for(int j=0; j<DD_ROP_SPACE; j++) c2->dwVSBRops[j]=0;
		c2->dwVSBRops[0] = 0x1;
		c2->dwVSBRops[6]	= 0x100;
		c2->dwVSBRops[7]	= 0x8000000;
		c2->dwSSBCaps = 0xF4C08241;
		c2->dwSSBFXCaps = 0x3FCE3;
		for(int j=0; j<DD_ROP_SPACE; j++) c2->dwSSBRops[j]=0;
		c2->dwSSBRops[0] = 1;
		c2->dwSSBRops[6]	= 0x100;
		c2->dwSSBRops[7]	= 0x8000000;
		if(dwSize > (int)0x16C) {
			c2->ddsCaps.dwCaps = 0x421350;
			c2->ddsCaps.dwCaps2 = 0x200;
		}
	}
#endif
#ifdef VIRTUAL_CAPS_VIRTUALPC
/*
CapsDump(c1:HW)=
------- 00 -------- 04 -------- 08 -------- 0c --------
0x0000: 7C,01,00,00,C1,68,40,90,00,10,48,00,10,00,00,00,
0x0010: 00,00,A8,02,00,00,00,00,00,00,00,00,00,00,00,00,
0x0020: 00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,
0x0030: 00,00,00,00,00,00,00,00,00,06,00,00,C0,21,55,00,
0x0040: C0,21,55,00,20,00,00,00,00,00,00,00,00,00,00,00,
0x0050: 00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,
0x0060: 00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,
0x0070: 00,00,00,00,00,00,00,00,00,00,00,00,00,01,00,00,
0x0080: 00,00,00,00,50,02,00,00,00,00,00,00,00,00,00,00,
0x0090: 00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,
0x00a0: 00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,
0x00b0: 00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,
0x00c0: 00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,
0x00d0: 00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,
0x00e0: 00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,
0x00f0: 00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,
0x0100: 00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,
0x0110: 00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,
0x0120: 00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,
0x0130: 00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,
0x0140: 00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,
0x0150: 00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,
0x0160: 00,00,00,00,00,00,00,00,00,00,00,00,7A,72,42,00,
0x0170: 00,02,20,00,00,00,00,00,00,00,00,00,
CapsDump(c2:SW)=
------- 00 -------- 04 -------- 08 -------- 0c --------
0x0000: 7C,01,00,00,41,82,C0,F4,01,00,00,00,00,02,00,00,
0x0010: E1,FC,03,00,00,00,00,00,47,03,00,00,00,00,00,00,
0x0020: 00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,
0x0030: 00,00,00,00,00,00,00,00,00,04,00,00,00,00,00,00,
0x0040: 00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,
0x0050: 00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,
0x0060: 00,00,00,00,01,00,00,00,00,00,00,00,00,00,00,00,
0x0070: 00,00,00,00,00,00,00,00,00,00,00,00,00,01,00,00,
0x0080: 00,00,00,08,50,13,42,00,00,00,00,00,00,00,00,00,
0x0090: 00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,
0x00a0: 00,00,00,00,00,00,00,00,00,00,00,00,41,82,C0,F4,
0x00b0: 00,02,00,00,E3,FC,03,00,01,00,00,00,00,00,00,00,
0x00c0: 00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,
0x00d0: 00,01,00,00,00,00,00,08,41,82,C0,F4,00,02,00,00,
0x00e0: E3,FC,03,00,01,00,00,00,00,00,00,00,00,00,00,00,
0x00f0: 00,00,00,00,00,00,00,00,00,00,00,00,00,01,00,00,
0x0100: 00,00,00,08,41,82,C0,F4,00,02,00,00,E3,FC,03,00,
0x0110: 01,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,
0x0120: 00,00,00,00,00,00,00,00,00,01,00,00,00,00,00,08,
0x0130: 00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,
0x0140: 00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,
0x0150: 00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,
0x0160: 00,00,00,00,00,00,00,00,00,00,00,00,50,13,42,00,
0x0170: 00,02,00,00,00,00,00,00,00,00,00,00,
*/ 
	if(c1){
		DWORD dwSize = c1->dwSize;
		DWORD dwVidMemTotal = c1->dwVidMemTotal;
		DWORD dwVidMemFree = c1->dwVidMemFree;
		memset((void *)c1, 0, dwSize);
		c1->dwSize		= dwSize;
		c1->dwCaps		= 0x904068c1;
		c1->dwCaps2		= 0x481000;
		c1->dwCKeyCaps	= 0x1;
		c1->dwFXCaps	= 0x2a80000;
		c1->dwZBufferBitDepths = 0x600;
		c1->dwVidMemTotal = dwVidMemTotal;
		c1->dwVidMemFree = dwVidMemFree;
		c1->dwMaxVisibleOverlays = 0x20;
		//for(int j=0; j<DD_ROP_SPACE; j++) c1->dwRops[j]=0;
		c1->dwRops[6]	= 0x100;
		c1->ddsOldCaps.dwCaps = 0x250;
		if(dwSize >= (int)(0x16C)) {
			c1->ddsCaps.dwCaps = 0x42727A;
			c1->ddsCaps.dwCaps2 = 0x200200;
		}
	}
	if(c2){
		int dwSize = c2->dwSize;
		DWORD dwVidMemTotal = c2->dwVidMemTotal;
		DWORD dwVidMemFree = c2->dwVidMemFree;
		memset((void *)c2, 0, dwSize);
		c2->dwSize		= dwSize;
		c2->dwCaps		= 0xf4c08241;
		c2->dwCaps2		= 0x1;
		c2->dwCKeyCaps	= 0x200;
		c2->dwFXCaps	= 0x3fce1;
		c2->dwPalCaps	= 0x347; 
		c2->dwVidMemTotal = dwVidMemTotal;
		c2->dwVidMemFree = dwVidMemFree;
		if(dwSize > (int)0xac){
			//for(int j=0; j<DD_ROP_SPACE; j++) c2->dwRops[j]=0;
			c2->dwRops[0]	= 0x1;
			c1->dwRops[6]	= 0x100;
			c2->dwRops[7]	= 0x8000000;
			c2->ddsOldCaps.dwCaps	= 0x421350;
			c2->dwSVBCaps	= 0xF4C08241;
			c2->dwSVBCKeyCaps = 0x200; 
			c2->dwSVBFXCaps = 0x3FCE3;
			//for(int j=0; j<DD_ROP_SPACE; j++) c2->dwSVBRops[j]=0;
			c2->dwSVBRops[0] = 0x1;
			c2->dwSVBRops[6]	= 0x100;
			c2->dwSVBRops[7]	= 0x8000000;
			c2->dwVSBCaps = 0xF4C08241;
			c2->dwVSBCKeyCaps = 0x200;
			c2->dwVSBFXCaps = 0x3FCE3;
			//for(int j=0; j<DD_ROP_SPACE; j++) c2->dwVSBRops[j]=0;
			c2->dwVSBRops[0] = 0x1;
			c2->dwVSBRops[6]	= 0x100;
			c2->dwVSBRops[7]	= 0x8000000;
			c2->dwSSBCaps = 0xF4C08241;
			c2->dwSSBCKeyCaps = 0x200;
			c2->dwSSBFXCaps = 0x3FCE3;
		}
		if(dwSize > (int)0x13c) {
			//for(int j=0; j<DD_ROP_SPACE; j++) c2->dwSSBRops[j]=0;
			c2->dwSSBRops[0] = 1;
			c2->dwSSBRops[6]	= 0x100;
			c2->dwSSBRops[7]	= 0x8000000;
		}
		if(dwSize > (int)0x16C) {
			c2->ddsCaps.dwCaps = 0x421350;
			c2->ddsCaps.dwCaps2 = 0x200;
		}
	}
#endif
}
