#define _CRT_SECURE_NO_DEPRECATE 1
#include <windows.h>
#include <ddraw.h>
#include "dxwnd.h"


// debug functions to make the log more readable

void DumpRect(char *what, LPRECT r, int line)
{
	OutTrace("DEBUG: RECT %s at %d:", what, line);
	if (r)
		OutTrace("(%d,%d)-(%d,%d)\n",r->left,r->top,r->right,r->bottom);
	else
		OutTrace("(NULL)\n");
}

char *ExplainFlags(DWORD c)
{
	static char eb[256];
	unsigned int l;
	strcpy(eb,"DDSD_");
	if (c & DDSD_CAPS) strcat(eb, "CAPS+");
	if (c & DDSD_HEIGHT) strcat(eb, "HEIGHT+");
	if (c & DDSD_WIDTH) strcat(eb, "WIDTH+");
	if (c & DDSD_PITCH) strcat(eb, "PITCH+");
	if (c & DDSD_BACKBUFFERCOUNT) strcat(eb, "BACKBUFFERCOUNT+");
	if (c & DDSD_ZBUFFERBITDEPTH) strcat(eb, "ZBUFFERBITDEPTH+");
	if (c & DDSD_ALPHABITDEPTH) strcat(eb, "ALPHABITDEPTH+");
	if (c & DDSD_LPSURFACE) strcat(eb, "LPSURFACE+");
	if (c & DDSD_PIXELFORMAT) strcat(eb, "PIXELFORMAT+");
	if (c & DDSD_CKDESTOVERLAY) strcat(eb, "CKDESTOVERLAY+");
	if (c & DDSD_CKDESTBLT) strcat(eb, "CKDESTBLT+");
	if (c & DDSD_CKSRCOVERLAY) strcat(eb, "CKSRCOVERLAY+");
	if (c & DDSD_CKSRCBLT) strcat(eb, "CKSRCBLT+");
	if (c & DDSD_MIPMAPCOUNT) strcat(eb, "MIPMAPCOUNT+");
	if (c & DDSD_REFRESHRATE) strcat(eb, "REFRESHRATE+");
	if (c & DDSD_LINEARSIZE) strcat(eb, "LINEARSIZE+");
	if (c & DDSD_TEXTURESTAGE) strcat(eb, "TEXTURESTAGE+");
	if (c & DDSD_SRCVBHANDLE) strcat(eb, "SRCVBHANDLE+");
	if (c & DDSD_DEPTH) strcat(eb, "DEPTH+");
	l=strlen(eb);
	if (l>strlen("DDSD_")) eb[l-1]=0; // delete last '+' if any
	else eb[0]=0;
	return(eb);
}

char *ExplainDDSCaps(DWORD c)
{
	static char eb[256];
	unsigned int l;
	strcpy(eb,"DDSCAPS_");
	if (c & DDSCAPS_RESERVED1) strcat(eb, "RESERVED1+");
	if (c & DDSCAPS_ALPHA) strcat(eb, "ALPHA+");
	if (c & DDSCAPS_BACKBUFFER) strcat(eb, "BACKBUFFER+");
	if (c & DDSCAPS_COMPLEX) strcat(eb, "COMPLEX+");
	if (c & DDSCAPS_FLIP) strcat(eb, "FLIP+");
	if (c & DDSCAPS_FRONTBUFFER) strcat(eb, "FRONTBUFFER+");
	if (c & DDSCAPS_OFFSCREENPLAIN) strcat(eb, "OFFSCREENPLAIN+");
	if (c & DDSCAPS_OVERLAY) strcat(eb, "OVERLAY+");
	if (c & DDSCAPS_PALETTE) strcat(eb, "PALETTE+");
	if (c & DDSCAPS_PRIMARYSURFACE) strcat(eb, "PRIMARYSURFACE+");
	if (c & DDSCAPS_SYSTEMMEMORY) strcat(eb, "SYSTEMMEMORY+");
	if (c & DDSCAPS_TEXTURE) strcat(eb, "TEXTURE+");
	if (c & DDSCAPS_3DDEVICE) strcat(eb, "3DDEVICE+");
	if (c & DDSCAPS_VIDEOMEMORY) strcat(eb, "VIDEOMEMORY+");
	if (c & DDSCAPS_VISIBLE) strcat(eb, "VISIBLE+");
	if (c & DDSCAPS_WRITEONLY) strcat(eb, "WRITEONLY+");
	if (c & DDSCAPS_ZBUFFER) strcat(eb, "ZBUFFER+");
	if (c & DDSCAPS_OWNDC) strcat(eb, "OWNDC+");
	if (c & DDSCAPS_LIVEVIDEO) strcat(eb, "LIVEVIDEO+");
	if (c & DDSCAPS_HWCODEC) strcat(eb, "HWCODEC+");
	if (c & DDSCAPS_MODEX) strcat(eb, "MODEX+");
	if (c & DDSCAPS_MIPMAP) strcat(eb, "MIPMAP+");
	if (c & DDSCAPS_ALLOCONLOAD) strcat(eb, "ALLOCONLOAD+");
	if (c & DDSCAPS_VIDEOPORT) strcat(eb, "VIDEOPORT+");
	if (c & DDSCAPS_LOCALVIDMEM) strcat(eb, "LOCALVIDMEM+");
	if (c & DDSCAPS_NONLOCALVIDMEM) strcat(eb, "NONLOCALVIDMEM+");
	if (c & DDSCAPS_STANDARDVGAMODE) strcat(eb, "STANDARDVGAMODE+");
	if (c & DDSCAPS_OPTIMIZED) strcat(eb, "OPTIMIZED+");
	l=strlen(eb);
	if (l>strlen("DDSCAPS_")) eb[l-1]=0; // delete last '+' if any
	else eb[0]=0;
	return(eb);
}

char *ExplainCoopFlags(DWORD c)
{
	static char eb[256];
	unsigned int l;
	strcpy(eb,"DDSCL_");
	if (c & DDSCL_FULLSCREEN) strcat(eb, "FULLSCREEN+");
	if (c & DDSCL_ALLOWREBOOT) strcat(eb, "ALLOWREBOOT+");
	if (c & DDSCL_NOWINDOWCHANGES) strcat(eb, "NOWINDOWCHANGES+");
	if (c & DDSCL_NORMAL) strcat(eb, "NORMAL+");
	if (c & DDSCL_EXCLUSIVE) strcat(eb, "EXCLUSIVE+");
	if (c & DDSCL_ALLOWMODEX) strcat(eb, "ALLOWMODEX+");
	if (c & DDSCL_SETFOCUSWINDOW) strcat(eb, "SETFOCUSWINDOW+");
	if (c & DDSCL_SETDEVICEWINDOW) strcat(eb, "SETDEVICEWINDOW+");
	if (c & DDSCL_CREATEDEVICEWINDOW) strcat(eb, "CREATEDEVICEWINDOW+");
	if (c & DDSCL_MULTITHREADED) strcat(eb, "MULTITHREADED+");
	if (c & DDSCL_FPUSETUP) strcat(eb, "FPUSETUP+");
	if (c & DDSCL_FPUPRESERVE) strcat(eb, "FPUPRESERVE+");
	l=strlen(eb);
	if (l>strlen("DDSCL_")) eb[l-1]=0; // delete last '+' if any
	else eb[0]=0;
	return(eb);
}

char *ExplainPixelFormatFlags(DWORD c)
{
	static char eb[256];
	unsigned int l;
	strcpy(eb,"DDPF_");
	if (c & DDPF_ALPHAPIXELS) strcat(eb, "ALPHAPIXELS+");
	if (c & DDPF_ALPHA) strcat(eb, "ALPHA+");
	if (c & DDPF_FOURCC) strcat(eb, "FOURCC+");
	if (c & DDPF_PALETTEINDEXED4) strcat(eb, "PALETTEINDEXED4+");
	if (c & DDPF_PALETTEINDEXEDTO8) strcat(eb, "PALETTEINDEXEDTO8+");
	if (c & DDPF_PALETTEINDEXED8) strcat(eb, "PALETTEINDEXED8+");
	if (c & DDPF_RGB) strcat(eb, "RGB+");
	if (c & DDPF_COMPRESSED) strcat(eb, "COMPRESSED+");
	if (c & DDPF_RGBTOYUV) strcat(eb, "RGBTOYUV+");
	if (c & DDPF_YUV) strcat(eb, "YUV+");
	if (c & DDPF_ZBUFFER) strcat(eb, "ZBUFFER+");
	if (c & DDPF_PALETTEINDEXED1) strcat(eb, "PALETTEINDEXED1+");
	if (c & DDPF_PALETTEINDEXED2) strcat(eb, "PALETTEINDEXED2+");
	if (c & DDPF_ZPIXELS) strcat(eb, "ZPIXELS+");
	if (c & DDPF_STENCILBUFFER) strcat(eb, "STENCILBUFFER+");
	if (c & DDPF_ALPHAPREMULT) strcat(eb, "ALPHAPREMULT+");
	if (c & DDPF_LUMINANCE) strcat(eb, "LUMINANCE+");
	if (c & DDPF_BUMPLUMINANCE) strcat(eb, "BUMPLUMINANCE+");
	if (c & DDPF_BUMPDUDV) strcat(eb, "BUMPDUDV+");
	l=strlen(eb);
	if (l>strlen("DDPF_")) eb[l-1]=0; // delete last '+' if any
	else eb[0]=0;
	return(eb);
}

char *ExplainFlipFlags(DWORD c)
{
	static char eb[512];
	unsigned int l;
	strcpy(eb,"DDFLIP_");
	if (c & DDFLIP_WAIT) strcat(eb, "WAIT+");
	if (c & DDFLIP_EVEN) strcat(eb, "EVEN+");
	if (c & DDFLIP_ODD) strcat(eb, "ODD+");
	if (c & DDFLIP_NOVSYNC) strcat(eb, "NOVSYNC+");
	if (c & DDFLIP_INTERVAL3) strcat(eb, "INTERVAL3+");
	else if (c & DDFLIP_INTERVAL2) strcat(eb, "INTERVAL2+");
	if (c & DDFLIP_INTERVAL4) strcat(eb, "INTERVAL4+");
	if (c & DDFLIP_STEREO) strcat(eb, "STEREO+");
	if (c & DDFLIP_DONOTWAIT) strcat(eb, "DONOTWAIT+");
	l=strlen(eb);
	if (l>strlen("DDFLIP_")) eb[l-1]=0; // delete last '+' if any
	else eb[0]=0;
	return(eb);
}

char *ExplainFlipStatusFlags(DWORD c)
{
	char *ep;
	switch(c){
		case DDGFS_CANFLIP: ep="DDGFS_CANFLIP"; break;
		case DDGFS_ISFLIPDONE: ep="DDGFS_ISFLIPDONE"; break;
		default: ep="unknown"; break;
	}
	return ep;
}

char *ExplainBltFlags(DWORD c)
{
	static char eb[512];
	unsigned int l;
	strcpy(eb,"DDBLT_");
	if (c & DDBLT_ALPHADEST) strcat(eb, "ALPHADEST+");
	if (c & DDBLT_ALPHADESTCONSTOVERRIDE) strcat(eb, "ALPHADESTCONSTOVERRIDE+");
	if (c & DDBLT_ALPHADESTNEG) strcat(eb, "ALPHADESTNEG+");
	if (c & DDBLT_ALPHADESTSURFACEOVERRIDE) strcat(eb, "ALPHADESTSURFACEOVERRIDE+");
	if (c & DDBLT_ALPHAEDGEBLEND) strcat(eb, "ALPHAEDGEBLEND+");
	if (c & DDBLT_ALPHASRC) strcat(eb, "ALPHASRC+");
	if (c & DDBLT_ALPHASRCCONSTOVERRIDE) strcat(eb, "ALPHASRCCONSTOVERRIDE+");
	if (c & DDBLT_ALPHASRCNEG) strcat(eb, "ALPHASRCNEG+");
	if (c & DDBLT_ALPHASRCSURFACEOVERRIDE) strcat(eb, "ALPHASRCSURFACEOVERRIDE+");
	if (c & DDBLT_ASYNC) strcat(eb, "ASYNC+");
	if (c & DDBLT_COLORFILL) strcat(eb, "COLORFILL+");
	if (c & DDBLT_DDFX) strcat(eb, "DDFX+");
	if (c & DDBLT_DDROPS) strcat(eb, "DDROPS+");
	if (c & DDBLT_KEYDEST) strcat(eb, "KEYDEST+");
	if (c & DDBLT_KEYDESTOVERRIDE) strcat(eb, "KEYDESTOVERRIDE+");
	if (c & DDBLT_KEYSRC) strcat(eb, "KEYSRC+");
	if (c & DDBLT_KEYSRCOVERRIDE) strcat(eb, "KEYSRCOVERRIDE+");
	if (c & DDBLT_ROP) strcat(eb, "ROP+");
	if (c & DDBLT_ROTATIONANGLE) strcat(eb, "ROTATIONANGLE+");
	if (c & DDBLT_ZBUFFER) strcat(eb, "ZBUFFER+");
	if (c & DDBLT_ZBUFFERDESTCONSTOVERRIDE) strcat(eb, "ZBUFFERDESTCONSTOVERRIDE+");
	if (c & DDBLT_ZBUFFERDESTOVERRIDE) strcat(eb, "ZBUFFERDESTOVERRIDE+");
	if (c & DDBLT_ZBUFFERSRCCONSTOVERRIDE) strcat(eb, "ZBUFFERSRCCONSTOVERRIDE+");
	if (c & DDBLT_ZBUFFERSRCOVERRIDE) strcat(eb, "ZBUFFERSRCOVERRIDE+");
	if (c & DDBLT_WAIT) strcat(eb, "WAIT+");
	if (c & DDBLT_DEPTHFILL) strcat(eb, "DEPTHFILL+");
	if (c & DDBLT_DONOTWAIT) strcat(eb, "DONOTWAIT+");
	if (c & DDBLT_ROTATIONANGLE) strcat(eb, "ROTATIONANGLE+");
	l=strlen(eb);
	if (l>strlen("DDBLT_")) eb[l-1]=0; // delete last '+' if any
	else eb[0]=0;
	return(eb);
}

char *ExplainBltFastFlags(DWORD c)
{
	static char eb[256];
	unsigned int l;
	strcpy(eb,"DDBLTFAST_");
	if (!(c & (DDBLTFAST_SRCCOLORKEY|DDBLTFAST_DESTCOLORKEY))) strcat(eb, "NOCOLORKEY+");
	if (c & DDBLTFAST_SRCCOLORKEY) strcat(eb, "SRCCOLORKEY+");
	if (c & DDBLTFAST_DESTCOLORKEY) strcat(eb, "DESTCOLORKEY+");
	if (c & DDBLTFAST_WAIT) strcat(eb, "WAIT+");
	if (c & DDBLTFAST_DONOTWAIT) strcat(eb, "DONOTWAIT+");
	l=strlen(eb);
	if (l>strlen("DDBLTFAST_")) eb[l-1]=0; // delete last '+' if any
	else eb[0]=0;
	return(eb);
}

char *ExplainCreatePaletteFlags(DWORD c)
{
	static char eb[256];
	unsigned int l;
	strcpy(eb,"DDPCAPS_");
	if (c & DDPCAPS_4BIT) strcat(eb, "4BIT+");
	if (c & DDPCAPS_8BITENTRIES) strcat(eb, "8BITENTRIES+");
	if (c & DDPCAPS_8BIT) strcat(eb, "8BIT+");
	if (c & DDPCAPS_INITIALIZE) strcat(eb, "INITIALIZE+");
	if (c & DDPCAPS_PRIMARYSURFACE) strcat(eb, "PRIMARYSURFACE+");
	if (c & DDPCAPS_PRIMARYSURFACELEFT) strcat(eb, "PRIMARYSURFACELEFT+");
	if (c & DDPCAPS_ALLOW256) strcat(eb, "ALLOW256+");
	if (c & DDPCAPS_VSYNC) strcat(eb, "VSYNC+");
	if (c & DDPCAPS_1BIT) strcat(eb, "1BIT+");
	if (c & DDPCAPS_2BIT) strcat(eb, "2BIT+");
	if (c & DDPCAPS_ALPHA) strcat(eb, "ALPHA+");
	l=strlen(eb);
	if (l>strlen("DDPCAPS_")) eb[l-1]=0; // delete last '+' if any
	else eb[0]=0;
	return(eb);
}

char *ExplainROP(DWORD c)
{
	static char *eb;
	switch(c){
		case SRCCOPY: eb="SRCCOPY"; break; 
		case SRCPAINT: eb="SRCPAINT"; break; 
		case SRCAND: eb="SRCAND"; break; 
		case SRCINVERT: eb="SRCINVERT"; break; 
		case SRCERASE: eb="SRCERASE"; break; 
		case NOTSRCCOPY: eb="NOTSRCCOPY"; break; 
		case NOTSRCERASE: eb="NOTSRCERASE"; break; 
		case MERGECOPY: eb="MERGECOPY"; break; 
		case MERGEPAINT: eb="MERGEPAINT"; break; 
		case PATCOPY: eb="PATCOPY"; break; 
		case PATPAINT: eb="PATPAINT"; break; 
		case PATINVERT: eb="PATINVERT"; break; 
		case DSTINVERT: eb="DSTINVERT"; break; 
		case BLACKNESS: eb="BLACKNESS"; break; 
		case WHITENESS: eb="WHITENESS"; break; 
		case NOMIRRORBITMAP: eb="NOMIRRORBITMAP"; break; 
		case CAPTUREBLT: eb="CAPTUREBLT"; break; 
		default: eb="unknown"; break; 
	}
	return(eb);
}

char *ExplainLockFlags(DWORD c)
{
	static char eb[256];
	unsigned int l;
	strcpy(eb,"DDLOCK_");
	if (c & DDLOCK_WAIT) strcat(eb, "WAIT+");
	if (c & DDLOCK_EVENT) strcat(eb, "EVENT+");
	if (c & DDLOCK_READONLY) strcat(eb, "READONLY+");
	if (c & DDLOCK_WRITEONLY) strcat(eb, "WRITEONLY+");
	if (c & DDLOCK_NOSYSLOCK) strcat(eb, "NOSYSLOCK+");
	if (c & DDLOCK_NOOVERWRITE) strcat(eb, "NOOVERWRITE+");
	if (c & DDLOCK_DISCARDCONTENTS) strcat(eb, "DISCARDCONTENTS+");
	if (c & DDLOCK_DONOTWAIT) strcat(eb, "DONOTWAIT+");
	if (c & DDLOCK_HASVOLUMETEXTUREBOXRECT) strcat(eb, "HASVOLUMETEXTUREBOXRECT+");
	if (c & DDLOCK_NODIRTYUPDATE) strcat(eb, "NODIRTYUPDATE+");
	l=strlen(eb);
	if (l>strlen("DDLOCK_")) eb[l-1]=0; // delete last '+' if any
	else strcpy(eb,"DDLOCK_SURFACEMEMORYPTR"); // when zero...
	return(eb);
}

char *ExplainStyle(DWORD c)
{
	static char eb[256];
	unsigned int l;
	strcpy(eb,"WS_");
	if (c & WS_BORDER) strcat(eb, "BORDER+");
	if (c & WS_CAPTION) strcat(eb, "CAPTION+");
	if (c & WS_CHILD) strcat(eb, "CHILD+");
	if (c & WS_CLIPCHILDREN) strcat(eb, "CLIPCHILDREN+");
	if (c & WS_CLIPSIBLINGS) strcat(eb, "CLIPSIBLINGS+");
	if (c & WS_DISABLED) strcat(eb, "DISABLED+");
	if (c & WS_DLGFRAME) strcat(eb, "DLGFRAME+");
	if (c & WS_GROUP) strcat(eb, "GROUP+");
	if (c & WS_HSCROLL) strcat(eb, "HSCROLL+");
	if (c & WS_MAXIMIZE) strcat(eb, "MAXIMIZE+");
	if (c & WS_MAXIMIZEBOX) strcat(eb, "MAXIMIZEBOX+");
	if (c & WS_MINIMIZE) strcat(eb, "MINIMIZE+");
	if (c & WS_MINIMIZEBOX) strcat(eb, "MINIMIZEBOX+");
	if (c & WS_POPUP) strcat(eb, "POPUP+");
	if (c & WS_SIZEBOX) strcat(eb, "SIZEBOX+");
	if (c & WS_SYSMENU) strcat(eb, "SYSMENU+");
	if (c & WS_TABSTOP) strcat(eb, "TABSTOP+");
	if (c & WS_THICKFRAME) strcat(eb, "THICKFRAME+");
	if (c & WS_TILED) strcat(eb, "TILED+");
	if (c & WS_VISIBLE) strcat(eb, "VISIBLE+");
	if (c & WS_VSCROLL) strcat(eb, "VSCROLL+");
	l=strlen(eb);
	if (l>strlen("WS_")) eb[l-1]=0; // delete last '+' if any
	else strcpy(eb,"WS_OVERLAPPED"); // when zero ...
	return(eb);
}

char *ExplainExStyle(DWORD c)
{
	static char eb[256];
	unsigned int l;
	strcpy(eb,"WS_EX_");
	if (c & WS_EX_ACCEPTFILES) strcat(eb, "ACCEPTFILES+");
	if (c & WS_EX_APPWINDOW) strcat(eb, "APPWINDOW+");
	if (c & WS_EX_CLIENTEDGE) strcat(eb, "CLIENTEDGE+");
	//if (c & WS_EX_COMPOSITED) strcat(eb, "COMPOSITED+");
	if (c & WS_EX_CONTEXTHELP) strcat(eb, "CONTEXTHELP+");
	if (c & WS_EX_CONTROLPARENT) strcat(eb, "CONTROLPARENT+");
	if (c & WS_EX_DLGMODALFRAME) strcat(eb, "DLGMODALFRAME+");
	//if (c & WS_EX_LAYERED) strcat(eb, "LAYERED+");
	if (c & WS_EX_LAYOUTRTL) strcat(eb, "LAYOUTRTL+");
	if (c & WS_EX_LEFT) strcat(eb, "LEFT+");
	if (c & WS_EX_LEFTSCROLLBAR) strcat(eb, "LEFTSCROLLBAR+");
	if (c & WS_EX_LTRREADING) strcat(eb, "LTRREADING+");
	if (c & WS_EX_MDICHILD) strcat(eb, "MDICHILD+");
	//if (c & WS_EX_NOACTIVATE) strcat(eb, "NOACTIVATE+");
	if (c & WS_EX_NOINHERITLAYOUT) strcat(eb, "NOINHERITLAYOUT+");
	if (c & WS_EX_NOPARENTNOTIFY) strcat(eb, "NOPARENTNOTIFY+");
	if (c & WS_EX_RIGHT) strcat(eb, "RIGHT+");
	if (c & WS_EX_RTLREADING) strcat(eb, "RTLREADING+");
	if (c & WS_EX_STATICEDGE) strcat(eb, "STATICEDGE+");
	if (c & WS_EX_TOOLWINDOW) strcat(eb, "TOOLWINDOW+");
	if (c & WS_EX_TOPMOST) strcat(eb, "TOPMOST+");
	if (c & WS_EX_TRANSPARENT) strcat(eb, "TRANSPARENT+");
	if (c & WS_EX_WINDOWEDGE) strcat(eb, "WINDOWEDGE+");
	l=strlen(eb);
	if (l>strlen("WS_EX_")) eb[l-1]=0; // delete last '+' if any
	else strcpy(eb,"WS_EX_RIGHTSCROLLBAR"); // when zero ...
	return(eb);
}

char *ExplainShowCmd(int c)
{
	static char *eb;
	switch(c)
	{
	case SW_HIDE: eb="SW_HIDE"; break; // 0
	case SW_SHOWNORMAL: eb="SW_SHOWNORMAL"; break; // 1
	case SW_SHOWMINIMIZED: eb="SW_SHOWMINIMIZED"; break; // 2
	case SW_MAXIMIZE: eb="SW_MAXIMIZE"; break; // 3
	//case SW_SHOWMAXIMIZED: eb="SW_SHOWMAXIMIZED"; break; // 3 = SW_MAXIMIZE
	case SW_SHOWNOACTIVATE: eb="SW_SHOWNOACTIVATE"; break; // 4
	case SW_SHOW: eb="SW_SHOW"; break; // 5
	case SW_MINIMIZE: eb="SW_MINIMIZE"; break; // 6
	case SW_SHOWMINNOACTIVE: eb="SW_SHOWMINNOACTIVE"; break; // 7
	case SW_SHOWNA: eb="SW_SHOWNA"; break; // 8
	case SW_RESTORE: eb="SW_RESTORE"; break; // 9
	case SW_SHOWDEFAULT: eb="SW_SHOWDEFAULT"; break; // 10
	case SW_FORCEMINIMIZE: eb="SW_FORCEMINIMIZE"; break; // 11
	default: eb="unknown"; break;
	}
	return(eb);
}

char *ExplainBltStatus(DWORD c)
{
	static char *eb;
	switch(c)
	{
	case DDGBS_CANBLT:		eb="DDGBS_CANBLT"; break;
	case DDGBS_ISBLTDONE:	eb="DDGBS_ISBLTDONE"; break;
	default:				eb="invalid"; break;
	}
	return(eb);
}

char *ExplainDDError(DWORD c)
{
	static char *eb;
	switch(c)
	{
		case DD_OK: 							eb="DD_OK"; break;
		case DDERR_ALREADYINITIALIZED:			eb="DDERR_ALREADYINITIALIZED"; break;
		case DDERR_BLTFASTCANTCLIP:				eb="DDERR_BLTFASTCANTCLIP"; break;
		case DDERR_CANNOTATTACHSURFACE: 		eb="DDERR_CANNOTATTACHSURFACE"; break;
		case DDERR_CANNOTDETACHSURFACE: 		eb="DDERR_CANNOTDETACHSURFACE"; break;
		case DDERR_CANTCREATEDC: 				eb="DDERR_CANTCREATEDC"; break;
		case DDERR_CANTDUPLICATE: 				eb="DDERR_CANTDUPLICATE"; break;
		case DDERR_CANTLOCKSURFACE: 			eb="DDERR_CANTLOCKSURFACE"; break;
		case DDERR_CANTPAGELOCK: 				eb="DDERR_CANTPAGELOCK"; break;
		case DDERR_CANTPAGEUNLOCK: 				eb="DDERR_CANTPAGEUNLOCK"; break;
		case DDERR_CLIPPERISUSINGHWND: 			eb="DDERR_CLIPPERISUSINGHWND"; break;
		case DDERR_COLORKEYNOTSET: 				eb="DDERR_COLORKEYNOTSET"; break;
		case DDERR_CURRENTLYNOTAVAIL: 			eb="DDERR_CURRENTLYNOTAVAIL"; break;
		case DDERR_DCALREADYCREATED: 			eb="DDERR_DCALREADYCREATED"; break;
		case DDERR_DEVICEDOESNTOWNSURFACE: 		eb="DDERR_DEVICEDOESNTOWNSURFACE"; break;
		case DDERR_DIRECTDRAWALREADYCREATED: 	eb="DDERR_DIRECTDRAWALREADYCREATED"; break;
		case DDERR_EXCEPTION: 					eb="DDERR_EXCEPTION"; break;
		case DDERR_EXCLUSIVEMODEALREADYSET: 	eb="DDERR_EXCLUSIVEMODEALREADYSET"; break;
		case DDERR_EXPIRED: 					eb="DDERR_EXPIRED"; break;
		case DDERR_GENERIC: 					eb="DDERR_GENERIC"; break;
		case DDERR_HEIGHTALIGN: 				eb="DDERR_HEIGHTALIGN"; break; 	
		case DDERR_HWNDALREADYSET: 	 			eb="DDERR_HWNDALREADYSET"; break;
		case DDERR_HWNDSUBCLASSED: 	 			eb="DDERR_HWNDSUBCLASSED"; break;
		case DDERR_IMPLICITLYCREATED: 			eb="DDERR_IMPLICITLYCREATED"; break;
		case DDERR_INCOMPATIBLEPRIMARY: 	 	eb="DDERR_INCOMPATIBLEPRIMARY"; break;
		case DDERR_INVALIDCAPS: 		 		eb="DDERR_INVALIDCAPS"; break;
		case DDERR_INVALIDCLIPLIST:	 			eb="DDERR_INVALIDCLIPLIST"; break;
		case DDERR_INVALIDDIRECTDRAWGUID:	 	eb="DDERR_INVALIDDIRECTDRAWGUID"; break;
		case DDERR_INVALIDMODE: 	 			eb="DDERR_INVALIDMODE"; break;
		case DDERR_INVALIDOBJECT: 	 			eb="DDERR_INVALIDOBJECT"; break;
		case DDERR_INVALIDPARAMS: 	 			eb="DDERR_INVALIDPARAMS"; break;
		case DDERR_INVALIDPIXELFORMAT:	 		eb="DDERR_INVALIDPIXELFORMAT"; break;
		case DDERR_INVALIDPOSITION: 		 	eb="DDERR_INVALIDPOSITION"; break;
		case DDERR_INVALIDRECT: 		 		eb="DDERR_INVALIDRECT"; break;
		case DDERR_INVALIDSTREAM:	 			eb="DDERR_INVALIDSTREAM"; break;
		case DDERR_INVALIDSURFACETYPE:	 		eb="DDERR_INVALIDSURFACETYPE"; break;
		case DDERR_LOCKEDSURFACES: 		 		eb="DDERR_LOCKEDSURFACES"; break;
		case DDERR_MOREDATA: 		 			eb="DDERR_MOREDATA"; break;
		case DDERR_NO3D: 		 				eb="DDERR_NO3D"; break;
		case DDERR_NOALPHAHW:	 				eb="DDERR_NOALPHAHW"; break;
		case DDERR_NOBLTHW: 	 				eb="DDERR_NOBLTHW"; break;
		case DDERR_NOCLIPLIST: 	 				eb="DDERR_NOCLIPLIST"; break;
		case DDERR_NOCLIPPERATTACHED: 	 		eb="DDERR_NOCLIPPERATTACHED"; break;
		case DDERR_NOCOLORCONVHW: 		 		eb="DDERR_NOCOLORCONVHW"; break;
		case DDERR_NOCOLORKEY: 		 			eb="DDERR_NOCOLORKEY"; break;
		case DDERR_NOCOLORKEYHW: 	 			eb="DDERR_NOCOLORKEYHW"; break;
		case DDERR_NOCOOPERATIVELEVELSET: 	 	eb="DDERR_NOCOOPERATIVELEVELSET"; break;
		case DDERR_NODC: 		 				eb="DDERR_NODC"; break;
		case DDERR_NODDROPSHW:	 				eb="DDERR_NODDROPSHW"; break;
		case DDERR_NODIRECTDRAWHW:	 			eb="DDERR_NODIRECTDRAWHW"; break;
		case DDERR_NODIRECTDRAWSUPPORT: 	 	eb="DDERR_NODIRECTDRAWSUPPORT"; break;
		case DDERR_NOEMULATION: 		 		eb="DDERR_NOEMULATION"; break;
		case DDERR_NOEXCLUSIVEMODE: 	 		eb="DDERR_NOEXCLUSIVEMODE"; break;
		case DDERR_NOFLIPHW: 		 			eb="DDERR_NOFLIPHW"; break;
		case DDERR_NOFOCUSWINDOW: 		 		eb="DDERR_NOFOCUSWINDOW"; break;
		case DDERR_NOGDI: 		 				eb="DDERR_NOGDI"; break;
		case DDERR_NOHWND: 		 				eb="DDERR_NOHWND"; break;
		case DDERR_NOMIPMAPHW: 	 				eb="DDERR_NOMIPMAPHW"; break;
		case DDERR_NOMIRRORHW: 	 				eb="DDERR_NOMIRRORHW"; break;
		case DDERR_NONONLOCALVIDMEM: 	 		eb="DDERR_NONONLOCALVIDMEM"; break;
		case DDERR_NOOPTIMIZEHW: 		 		eb="DDERR_NOOPTIMIZEHW"; break;
		case DDERR_NOOVERLAYDEST: 		 		eb="DDERR_NOOVERLAYDEST"; break;
		case DDERR_NOOVERLAYHW: 		 		eb="DDERR_NOOVERLAYHW"; break;
		case DDERR_NOPALETTEATTACHED:	 		eb="DDERR_NOPALETTEATTACHED"; break;
		case DDERR_NOPALETTEHW: 		 		eb="DDERR_NOPALETTEHW"; break;
		case DDERR_NORASTEROPHW: 	 			eb="DDERR_NORASTEROPHW"; break;
		case DDERR_NOROTATIONHW: 	 			eb="DDERR_NOROTATIONHW"; break;
		case DDERR_NOSTRETCHHW: 		 		eb="DDERR_NOSTRETCHHW"; break;
		case DDERR_NOT4BITCOLOR: 	 			eb="DDERR_NOT4BITCOLOR"; break;
		case DDERR_NOT4BITCOLORINDEX: 	 		eb="DDERR_NOT4BITCOLORINDEX"; break;
		case DDERR_NOT8BITCOLOR: 		 		eb="DDERR_NOT8BITCOLOR"; break;
		case DDERR_NOTAOVERLAYSURFACE: 	 		eb="DDERR_NOTAOVERLAYSURFACE"; break;
		case DDERR_NOTEXTUREHW: 	 			eb="DDERR_NOTEXTUREHW"; break;
		case DDERR_NOTFLIPPABLE: 	 			eb="DDERR_NOTFLIPPABLE"; break;
		case DDERR_NOTFOUND: 	 				eb="DDERR_NOTFOUND"; break;
		case DDERR_NOTINITIALIZED:	 			eb="DDERR_NOTINITIALIZED"; break;
		case DDERR_NOTLOADED: 		 			eb="DDERR_NOTLOADED"; break;
		case DDERR_NOTLOCKED: 		 			eb="DDERR_NOTLOCKED"; break;
		case DDERR_NOTPAGELOCKED: 		 		eb="DDERR_NOTPAGELOCKED"; break;
		case DDERR_NOTPALETTIZED: 		 		eb="DDERR_NOTPALETTIZED"; break;
		case DDERR_NOVSYNCHW: 		 			eb="DDERR_NOVSYNCHW"; break;
		case DDERR_NOZBUFFERHW: 		 		eb="DDERR_NOZBUFFERHW"; break;
		case DDERR_NOZOVERLAYHW: 	 			eb="DDERR_NOZOVERLAYHW"; break;
		case DDERR_OUTOFCAPS: 		 			eb="DDERR_OUTOFCAPS"; break;
		case DDERR_OUTOFMEMORY: 		 		eb="DDERR_OUTOFMEMORY"; break;
		case DDERR_OUTOFVIDEOMEMORY: 	 		eb="DDERR_OUTOFVIDEOMEMORY"; break;
		case DDERR_OVERLAPPINGRECTS: 	 		eb="DDERR_OVERLAPPINGRECTS"; break;
		case DDERR_OVERLAYCANTCLIP: 		 	eb="DDERR_OVERLAYCANTCLIP"; break;
		case DDERR_OVERLAYCOLORKEYONLYONEACTIVE:eb="DDERR_OVERLAYCOLORKEYONLYONEACTIVE"; break;
		case DDERR_OVERLAYNOTVISIBLE: 		 	eb="DDERR_OVERLAYNOTVISIBLE"; break;
		case DDERR_PALETTEBUSY: 		 		eb="DDERR_PALETTEBUSY"; break;
		case DDERR_PRIMARYSURFACEALREADYEXISTS: eb="DDERR_PRIMARYSURFACEALREADYEXISTS"; break;
		case DDERR_REGIONTOOSMALL: 	 			eb="DDERR_REGIONTOOSMALL"; break;
		case DDERR_SURFACEALREADYATTACHED: 	 	eb="DDERR_SURFACEALREADYATTACHED"; break;
		case DDERR_SURFACEALREADYDEPENDENT: 	eb="DDERR_SURFACEALREADYDEPENDENT"; break;
		case DDERR_SURFACEBUSY: 		 		eb="DDERR_SURFACEBUSY"; break;
		case DDERR_SURFACEISOBSCURED: 	 		eb="DDERR_SURFACEISOBSCURED"; break;
		case DDERR_SURFACELOST: 		 		eb="DDERR_SURFACELOST"; break;
		case DDERR_SURFACENOTATTACHED: 		 	eb="DDERR_SURFACENOTATTACHED"; break;
		case DDERR_TOOBIGHEIGHT: 		 		eb="DDERR_TOOBIGHEIGHT"; break;
		case DDERR_TOOBIGSIZE: 		 			eb="DDERR_TOOBIGSIZE"; break;
		case DDERR_TOOBIGWIDTH: 		 		eb="DDERR_TOOBIGWIDTH"; break;
		case DDERR_UNSUPPORTED: 		 		eb="DDERR_UNSUPPORTED"; break;
		case DDERR_UNSUPPORTEDFORMAT: 	 		eb="DDERR_UNSUPPORTEDFORMAT"; break;
		case DDERR_UNSUPPORTEDMASK: 		 	eb="DDERR_UNSUPPORTEDMASK"; break;
		case DDERR_UNSUPPORTEDMODE: 		 	eb="DDERR_UNSUPPORTEDMODE"; break;
		case DDERR_VERTICALBLANKINPROGRESS: 	eb="DDERR_VERTICALBLANKINPROGRESS"; break;
		case DDERR_VIDEONOTACTIVE: 		 		eb="DDERR_VIDEONOTACTIVE"; break;
		case DDERR_WASSTILLDRAWING: 		 	eb="DDERR_WASSTILLDRAWING"; break;
		case DDERR_WRONGMODE: 		 			eb="DDERR_WRONGMODE"; break;
		case DDERR_XALIGN: 		 				eb="DDERR_XALIGN"; break;
		default:								eb="unknown"; break;
	}
	return eb;
}

char *ExplainWinMessage(DWORD c)
{
	static char *eb;
	switch(c)
	{
		case WM_NULL: 		 			eb="WM_NULL"; break;
		case WM_CREATE: 		 		eb="WM_CREATE"; break;
		case WM_DESTROY: 		 		eb="WM_DESTROY"; break;
		case WM_MOVE: 		 			eb="WM_MOVE"; break;
		case WM_SIZE: 		 			eb="WM_SIZE"; break;
		case WM_ACTIVATE: 		 		eb="WM_ACTIVATE"; break;
		case WM_SETFOCUS: 		 		eb="WM_SETFOCUS"; break;
		case WM_KILLFOCUS: 		 		eb="WM_KILLFOCUS"; break;
		case WM_ENABLE: 		 		eb="WM_ENABLE"; break;
		case WM_SETREDRAW: 		 		eb="WM_SETREDRAW"; break;
		case WM_SETTEXT: 		 		eb="WM_SETTEXT"; break;
		case WM_GETTEXT: 		 		eb="WM_GETTEXT"; break;
		case WM_GETTEXTLENGTH: 		 	eb="WM_GETTEXTLENGTH"; break;
		case WM_PAINT: 		 			eb="WM_PAINT"; break;
		case WM_CLOSE: 		 			eb="WM_CLOSE"; break;
		case WM_QUERYENDSESSION: 		 eb="WM_QUERYENDSESSION"; break;
		case WM_QUERYOPEN: 		 		eb="WM_QUERYOPEN"; break;
		case WM_ENDSESSION: 		 	eb="WM_ENDSESSION"; break;
		case WM_QUIT: 		 			eb="WM_QUIT"; break;
		case WM_ERASEBKGND: 		 	eb="WM_ERASEBKGND"; break;
		case WM_SYSCOLORCHANGE: 		eb="WM_SYSCOLORCHANGE"; break;
		case WM_SHOWWINDOW: 		 	eb="WM_SHOWWINDOW"; break;
		case WM_WININICHANGE: 		 	eb="WM_WININICHANGE"; break;
		case WM_DEVMODECHANGE: 		 	eb="WM_DEVMODECHANGE"; break;
		case WM_ACTIVATEAPP: 		 	eb="WM_ACTIVATEAPP"; break;
		case WM_FONTCHANGE: 		 	eb="WM_FONTCHANGE"; break;
		case WM_CANCELMODE: 		 	eb="WM_CANCELMODE"; break;
		case WM_SETCURSOR: 		 		eb="WM_SETCURSOR"; break;
		case WM_MOUSEACTIVATE: 		 	eb="WM_MOUSEACTIVATE"; break;
		case WM_CHILDACTIVATE: 		 	eb="WM_CHILDACTIVATE"; break;
		case WM_QUEUESYNC: 		 		eb="WM_QUEUESYNC"; break;
		case WM_GETMINMAXINFO: 		 	eb="WM_GETMINMAXINFO"; break;
		case WM_PAINTICON: 		 		eb="WM_PAINTICON"; break;
		case WM_ICONERASEBKGND: 		eb="WM_ICONERASEBKGND"; break;
		case WM_NEXTDLGCTL: 		 	eb="WM_NEXTDLGCTL"; break;
		case WM_SPOOLERSTATUS: 		 	eb="WM_SPOOLERSTATUS"; break;
		case WM_DRAWITEM: 		 		eb="WM_DRAWITEM"; break;
		case WM_MEASUREITEM: 		 	eb="WM_MEASUREITEM"; break;
		case WM_DELETEITEM: 		 	eb="WM_DELETEITEM"; break;
		case WM_VKEYTOITEM: 		 	eb="WM_VKEYTOITEM"; break;
		case WM_CHARTOITEM: 		 	eb="WM_CHARTOITEM"; break;
		case WM_SETFONT: 		 		eb="WM_SETFONT"; break;
		case WM_GETFONT: 		 		eb="WM_GETFONT"; break;
		case WM_SETHOTKEY: 		 		eb="WM_SETHOTKEY"; break;
		case WM_GETHOTKEY: 		 		eb="WM_GETHOTKEY"; break;
		case WM_QUERYDRAGICON: 		 	eb="WM_QUERYDRAGICON"; break;
		case WM_COMPAREITEM: 		 	eb="WM_COMPAREITEM"; break;
		case WM_GETOBJECT: 		 		eb="WM_GETOBJECT"; break;
		case WM_COMPACTING: 		 	eb="WM_COMPACTING"; break;
		case WM_COMMNOTIFY: 		 	eb="WM_COMMNOTIFY"; break;
		case WM_WINDOWPOSCHANGING: 		eb="WM_WINDOWPOSCHANGING"; break;
		case WM_WINDOWPOSCHANGED: 		eb="WM_WINDOWPOSCHANGED"; break;
		case WM_POWER: 		 			eb="WM_POWER"; break;
		case WM_COPYDATA: 		 		eb="WM_COPYDATA"; break;
		case WM_CANCELJOURNAL: 		 	eb="WM_CANCELJOURNAL"; break;
		case WM_NOTIFY: 		 		eb="WM_NOTIFY"; break;
		case WM_INPUTLANGCHANGEREQUEST: eb="WM_INPUTLANGCHANGEREQUEST"; break;
		case WM_INPUTLANGCHANGE: 		eb="WM_INPUTLANGCHANGE"; break;
		case WM_TCARD: 		 			eb="WM_TCARD"; break;
		case WM_HELP: 		 			eb="WM_HELP"; break;
		case WM_USERCHANGED: 		 	eb="WM_USERCHANGED"; break;
		case WM_NOTIFYFORMAT: 		 	eb="WM_NOTIFYFORMAT"; break;
		case WM_CONTEXTMENU: 		 	eb="WM_CONTEXTMENU"; break;
		case WM_STYLECHANGING: 		 	eb="WM_STYLECHANGING"; break;
		case WM_STYLECHANGED: 		 	eb="WM_STYLECHANGED"; break;
		case WM_DISPLAYCHANGE: 		 	eb="WM_DISPLAYCHANGE"; break;
		case WM_GETICON: 		 		eb="WM_GETICON"; break;
		case WM_SETICON: 		 		eb="WM_SETICON"; break;
		case WM_NCCREATE: 		 		eb="WM_NCCREATE"; break;
		case WM_NCDESTROY: 		 		eb="WM_NCDESTROY"; break;
		case WM_NCCALCSIZE: 		 	eb="WM_NCCALCSIZE"; break;
		case WM_NCHITTEST: 		 		eb="WM_NCHITTEST"; break;
		case WM_NCPAINT: 		 		eb="WM_NCPAINT"; break;
		case WM_NCACTIVATE: 		 	eb="WM_NCACTIVATE"; break;
		case WM_GETDLGCODE: 		 	eb="WM_GETDLGCODE"; break;
#ifndef _WIN32_WCE
		case WM_SYNCPAINT: 		 		eb="WM_SYNCPAINT"; break;
#endif
		case WM_NCMOUSEMOVE: 		 	eb="WM_NCMOUSEMOVE"; break;
		case WM_NCLBUTTONDOWN: 		 	eb="WM_NCLBUTTONDOWN"; break;
		case WM_NCLBUTTONUP: 		 	eb="WM_NCLBUTTONUP"; break;
		case WM_NCLBUTTONDBLCLK: 		eb="WM_NCLBUTTONDBLCLK"; break;
		case WM_NCRBUTTONDOWN: 		 	eb="WM_NCRBUTTONDOWN"; break;
		case WM_NCRBUTTONUP: 		 	eb="WM_NCRBUTTONUP"; break;
		case WM_NCRBUTTONDBLCLK: 		eb="WM_NCRBUTTONDBLCLK"; break;
		case WM_NCMBUTTONDOWN: 		 	eb="WM_NCMBUTTONDOWN"; break;
		case WM_NCMBUTTONUP: 		 	eb="WM_NCMBUTTONUP"; break;
		case WM_NCMBUTTONDBLCLK: 		eb="WM_NCMBUTTONDBLCLK"; break;
#if(_WIN32_WINNT >= 0x0500)
		case WM_NCXBUTTONDOWN: 		 	eb="WM_NCXBUTTONDOWN"; break;
		case WM_NCXBUTTONUP: 		 	eb="WM_NCXBUTTONUP"; break;
		case WM_NCXBUTTONDBLCLK: 		eb="WM_NCXBUTTONDBLCLK"; break;
#endif /* _WIN32_WINNT >= 0x0500 */
#if(_WIN32_WINNT >= 0x0501)
		case WM_INPUT: 		 			eb="WM_INPUT"; break;
#endif /* _WIN32_WINNT >= 0x0501 */
		case WM_KEYDOWN: 		 		eb="WM_KEYDOWN/FIRST"; break;
		case WM_KEYUP: 		 			eb="WM_KEYUP"; break;
		case WM_CHAR: 		 			eb="WM_CHAR"; break;
		case WM_DEADCHAR: 		 		eb="WM_DEADCHAR"; break;
		case WM_SYSKEYDOWN: 		 	eb="WM_SYSKEYDOWN"; break;
		case WM_SYSKEYUP: 		 		eb="WM_SYSKEYUP"; break;
		case WM_SYSCHAR: 		 		eb="WM_SYSCHAR"; break;
		case WM_SYSDEADCHAR: 		 	eb="WM_SYSDEADCHAR"; break;
#if(WINVER >= 0x0400)
		case WM_IME_STARTCOMPOSITION: 	eb="WM_IME_STARTCOMPOSITION"; break;
		case WM_IME_ENDCOMPOSITION: 	eb="WM_IME_ENDCOMPOSITION"; break;
		case WM_IME_COMPOSITION: 		eb="WM_IME_COMPOSITION/KEYLAST"; break;
#endif /* WINVER >= 0x0400 */
		case WM_INITDIALOG: 		 	eb="WM_INITDIALOG"; break;
		case WM_COMMAND: 		 		eb="WM_COMMAND"; break;
		case WM_SYSCOMMAND: 		 	eb="WM_SYSCOMMAND"; break;
		case WM_TIMER: 		 			eb="WM_TIMER"; break;
		case WM_HSCROLL: 		 		eb="WM_HSCROLL"; break;
		case WM_VSCROLL: 		 		eb="WM_VSCROLL"; break;
		case WM_INITMENU: 		 		eb="WM_INITMENU"; break;
		case WM_INITMENUPOPUP: 		 	eb="WM_INITMENUPOPUP"; break;
		case WM_MENUSELECT: 		 	eb="WM_MENUSELECT"; break;
		case WM_MENUCHAR: 		 		eb="WM_MENUCHAR"; break;
		case WM_ENTERIDLE: 		 		eb="WM_ENTERIDLE"; break;
		case WM_CTLCOLORMSGBOX: 		eb="WM_CTLCOLORMSGBOX"; break;
		case WM_CTLCOLOREDIT: 		 	eb="WM_CTLCOLOREDIT"; break;
		case WM_CTLCOLORLISTBOX: 		eb="WM_CTLCOLORLISTBOX"; break;
		case WM_CTLCOLORBTN: 			eb="WM_CTLCOLORBTN"; break;
		case WM_CTLCOLORDLG: 		 	eb="WM_CTLCOLORDLG"; break;
		case WM_CTLCOLORSCROLLBAR: 		eb="WM_CTLCOLORSCROLLBAR"; break;
		case WM_CTLCOLORSTATIC: 		eb="WM_CTLCOLORSTATIC"; break;
		case MN_GETHMENU: 		 		eb="MN_GETHMENU"; break;
		case WM_MOUSEFIRST: 		 	eb="WM_MOUSEFIRST/MOUSEMOVE"; break;
		case WM_LBUTTONDOWN: 		 	eb="WM_LBUTTONDOWN"; break;
		case WM_LBUTTONUP: 		 		eb="WM_LBUTTONUP"; break;
		case WM_LBUTTONDBLCLK: 		 	eb="WM_LBUTTONDBLCLK"; break;
		case WM_RBUTTONDOWN: 		 	eb="WM_RBUTTONDOWN"; break;
		case WM_RBUTTONUP: 		 		eb="WM_RBUTTONUP"; break;
		case WM_RBUTTONDBLCLK: 		 	eb="WM_RBUTTONDBLCLK"; break;
		case WM_MBUTTONDOWN: 		 	eb="WM_MBUTTONDOWN"; break;
		case WM_MBUTTONUP: 		 		eb="WM_MBUTTONUP"; break;
		case WM_PARENTNOTIFY: 		 	eb="WM_PARENTNOTIFY"; break;
		case WM_ENTERMENULOOP: 		 	eb="WM_ENTERMENULOOP"; break;
		case WM_EXITMENULOOP: 		 	eb="WM_EXITMENULOOP"; break;
#if(WINVER >= 0x0400)
		case WM_NEXTMENU: 		 		eb="WM_NEXTMENU"; break;
		case WM_SIZING: 		 		eb="WM_SIZING"; break;
		case WM_CAPTURECHANGED: 		eb="WM_CAPTURECHANGED"; break;
		case WM_MOVING: 		 		eb="WM_MOVING"; break;
		case WM_POWERBROADCAST: 		eb="WM_POWERBROADCAST"; break;
		case WM_DEVICECHANGE: 		 	eb="WM_DEVICECHANGE"; break;
#endif /* WINVER >= 0x0400 */
		case WM_MDICREATE: 		 		eb="WM_MDICREATE"; break;
		case WM_MDIDESTROY: 		 	eb="WM_MDIDESTROY"; break;
		case WM_MDIACTIVATE: 		 	eb="WM_MDIACTIVATE"; break;
		case WM_MDIRESTORE: 		 	eb="WM_MDIRESTORE"; break;
		case WM_MDINEXT: 		 		eb="WM_MDINEXT"; break;
		case WM_MDIMAXIMIZE: 		 	eb="WM_MDIMAXIMIZE"; break;
		case WM_MDITILE: 		 		eb="WM_MDITILE"; break;
		case WM_MDICASCADE: 		 	eb="WM_MDICASCADE"; break;
		case WM_MDIICONARRANGE: 		eb="WM_MDIICONARRANGE"; break;
		case WM_MDIGETACTIVE: 		 	eb="WM_MDIGETACTIVE"; break;
		case WM_MDISETMENU: 		 	eb="WM_MDISETMENU"; break;
		case WM_ENTERSIZEMOVE: 		 	eb="WM_ENTERSIZEMOVE"; break;
		case WM_EXITSIZEMOVE: 		 	eb="WM_EXITSIZEMOVE"; break;
		case WM_DROPFILES: 		 		eb="WM_DROPFILES"; break;
		case WM_MDIREFRESHMENU: 		eb="WM_MDIREFRESHMENU"; break;
#if(WINVER >= 0x0400)
		case WM_IME_SETCONTEXT: 		eb="WM_IME_SETCONTEXT"; break;
		case WM_IME_NOTIFY: 		 	eb="WM_IME_NOTIFY"; break;
		case WM_IME_CONTROL: 		 	eb="WM_IME_CONTROL"; break;
		case WM_IME_COMPOSITIONFULL: 	eb="WM_IME_COMPOSITIONFULL"; break;
		case WM_IME_SELECT: 		 	eb="WM_IME_SELECT"; break;
		case WM_IME_CHAR: 		 		eb="WM_IME_CHAR"; break;
		case WM_IME_KEYDOWN: 		 	eb="WM_IME_KEYDOWN"; break;
		case WM_IME_KEYUP: 		 		eb="WM_IME_KEYUP"; break;
#endif /* WINVER >= 0x0400 */
		default:						eb="???"; break;
	}
	return eb;
}

char *ExplainResizing(DWORD c)
{
	static char *eb;
	switch(c)
	{
		case SIZE_MAXHIDE: 		 		eb="SIZE_MAXHIDE"; break;
		case SIZE_MAXIMIZED: 		 	eb="SIZE_MAXIMIZED"; break;
		case SIZE_MAXSHOW: 		 		eb="SIZE_MAXSHOW"; break;
		case SIZE_MINIMIZED: 		 	eb="SIZE_MINIMIZED"; break;
		case SIZE_RESTORED: 		 	eb="SIZE_RESTORED"; break;
		default:						eb="???"; break;
	}
	return eb;
}

char *ExplainDeviceCaps(DWORD c)
{
	static char *eb;
	switch(c)
	{
		case DRIVERVERSION: 		 	eb="DRIVERVERSION"; break;
		case TECHNOLOGY: 		 		eb="TECHNOLOGY"; break;
		case HORZSIZE: 		 			eb="HORZSIZE"; break;
		case VERTSIZE: 		 			eb="VERTSIZE"; break;
		case HORZRES: 		 			eb="HORZRES"; break;
		case VERTRES: 		 			eb="VERTRES"; break;
		case LOGPIXELSX: 		 		eb="LOGPIXELSX"; break;
		case LOGPIXELSY: 		 		eb="LOGPIXELSY"; break;
		case BITSPIXEL: 		 		eb="BITSPIXEL"; break;
		case PLANES: 		 			eb="PLANES"; break;
		case NUMBRUSHES: 		 		eb="NUMBRUSHES"; break;
		case NUMPENS: 		 			eb="NUMPENS"; break;
		case NUMFONTS: 		 			eb="NUMFONTS"; break;
		case NUMCOLORS: 		 		eb="NUMCOLORS"; break;
		case ASPECTX: 		 			eb="ASPECTX"; break;
		case ASPECTY: 		 			eb="ASPECTY"; break;
		case ASPECTXY: 		 			eb="ASPECTXY"; break;
		case PDEVICESIZE: 		 		eb="PDEVICESIZE"; break;
		case CLIPCAPS: 		 			eb="CLIPCAPS"; break;
		case SIZEPALETTE: 		 		eb="SIZEPALETTE"; break;
		case NUMRESERVED: 		 		eb="NUMRESERVED"; break;
		case COLORRES: 		 			eb="COLORRES"; break;
		case PHYSICALWIDTH: 		 	eb="PHYSICALWIDTH"; break;
		case PHYSICALHEIGHT: 		 	eb="PHYSICALHEIGHT"; break;
		case PHYSICALOFFSETX: 		 	eb="PHYSICALOFFSETX"; break;
		case PHYSICALOFFSETY: 		 	eb="PHYSICALOFFSETY"; break;
		case VREFRESH: 		 			eb="VREFRESH"; break;
		case SCALINGFACTORX: 		 	eb="SCALINGFACTORX"; break;
		case SCALINGFACTORY: 		 	eb="SCALINGFACTORY"; break;
		case BLTALIGNMENT: 		 		eb="BLTALIGNMENT"; break;
		case SHADEBLENDCAPS: 		 	eb="SHADEBLENDCAPS"; break;
		case RASTERCAPS: 		 		eb="RASTERCAPS"; break;
		case CURVECAPS: 		 		eb="CURVECAPS"; break;
		case LINECAPS: 		 			eb="LINECAPS"; break;
		case POLYGONALCAPS: 		 	eb="POLYGONALCAPS"; break;
		case TEXTCAPS: 		 			eb="TEXTCAPS"; break;
		case COLORMGMTCAPS: 		 	eb="COLORMGMTCAPS"; break;
		default:						eb="???"; break;
	}
	return eb;
}

char *ExplainDisplaySettingsRetcode(DWORD c)
{
	static char *eb;
	switch(c)
	{
		case DISP_CHANGE_SUCCESSFUL:	eb="DISP_CHANGE_SUCCESSFUL"; break;
		//case DISP_CHANGE_BADDUALVIEW:	eb="DISP_CHANGE_BADDUALVIEW"; break;
		case DISP_CHANGE_BADFLAGS:		eb="DISP_CHANGE_BADFLAGS"; break;
		case DISP_CHANGE_BADMODE:		eb="DISP_CHANGE_BADMODE"; break;
		case DISP_CHANGE_BADPARAM:		eb="DISP_CHANGE_BADPARAM"; break;
		case DISP_CHANGE_FAILED:		eb="DISP_CHANGE_FAILED"; break;
		case DISP_CHANGE_NOTUPDATED:	eb="DISP_CHANGE_NOTUPDATED"; break;
		case DISP_CHANGE_RESTART:		eb="DISP_CHANGE_RESTART"; break;
	}
	return eb;
}

char *ExplainSetWindowIndex(DWORD c)
{
	static char *eb;
	switch(c)
	{
		case GWL_EXSTYLE:		eb="GWL_EXSTYLE"; break;
		case GWL_HINSTANCE:		eb="GWL_HINSTANCE"; break;
		case GWL_ID:			eb="GWL_ID"; break;
		case GWL_STYLE:			eb="GWL_STYLE"; break;
		case GWL_USERDATA:		eb="GWL_USERDATA"; break;
		case GWL_WNDPROC:		eb="GWL_WNDPROC"; break;
		case DWL_DLGPROC:		eb="DWL_DLGPROC"; break;
		case DWL_MSGRESULT:		eb="DWL_MSGRESULT"; break;
		case DWL_USER:			eb="DWL_USER"; break;
		default:				eb=""; break;
	}
	return eb;
}

char *ExplainColorKeyFlag(DWORD c)
{
	static char *eb;
	switch(c)
	{
		case DDCKEY_COLORSPACE:		eb="DDCKEY_COLORSPACE"; break;
		case DDCKEY_DESTBLT:		eb="DDCKEY_DESTBLT"; break;
		case DDCKEY_DESTOVERLAY:	eb="DDCKEY_DESTOVERLAY"; break;
		case DDCKEY_SRCBLT:			eb="DDCKEY_SRCBLT"; break;
		case DDCKEY_SRCOVERLAY:		eb="DDCKEY_SRCOVERLAY"; break;
		default:					eb=""; break;
	}
	return eb;
}

char *ExplainGUID(GUID FAR *lpguid)
{
	static char *sguid;
	switch ((DWORD)lpguid){
		case NULL:						return "NULL"; break;
		case DDCREATE_HARDWAREONLY:		return "DDCREATE_HARDWAREONLY"; break;
		case DDCREATE_EMULATIONONLY:	return "DDCREATE_EMULATIONONLY"; break;
	}
	switch ((*(DWORD *)lpguid)){
		case 0x00000000: sguid="IID_IUnknown"; break;
		case 0x6C14DB80: sguid="IID_IDirectDraw"; break;
		case 0xB3A6F3E0: sguid="IID_IDirectDraw2"; break;
		case 0x9c59509a: sguid="IID_IDirectDraw4"; break;
		case 0x15e65ec0: sguid="IID_IDirectDraw7"; break;
		case 0x6C14DB81: sguid="IID_IDirectDrawSurface"; break;
		case 0x57805885: sguid="IID_IDirectDrawSurface2"; break;
		case 0xDA044E00: sguid="IID_IDirectDrawSurface3"; break;
		case 0x0B2B8630: sguid="IID_IDirectDrawSurface4"; break;
		case 0x06675a80: sguid="IID_IDirectDrawSurface7"; break;
		case 0x6C14DB84: sguid="IID_IDirectDrawPalette"; break;
		case 0x6C14DB85: sguid="IID_IDirectDrawClipper"; break;
		case 0x4B9F0EE0: sguid="IID_IDirectDrawColorControl"; break;
		case 0x69C11C3E: sguid="IID_IDirectDrawGammaControl"; break;
		case 0xA4665C60: sguid="IID_IDirect3DRGBDevice"; break;
		case 0x84E63dE0: sguid="IID_IDirect3DHALDevice"; break;
		case 0x3BBA0080: sguid="IID_IDirect3D"; break;
		case 0x6aae1ec1: sguid="IID_IDirect3D2"; break;
		case 0xbb223240: sguid="IID_IDirect3D3"; break;
		case 0xf5049e77: sguid="IID_IDirect3D7"; break;
		case 0x64108800: sguid="IID_IDirect3DDevice"; break;
		case 0x93281501: sguid="IID_IDirect3DDevice2"; break;
		case 0xb0ab3b60: sguid="IID_IDirect3DDevice3"; break;
		case 0xf5049e79: sguid="IID_IDirect3DDevice7"; break;
		case 0x2CDCD9E0: sguid="IID_IDirect3DTexture"; break;
		case 0x93281502: sguid="IID_IDirect3DTexture2"; break;
		case 0x4417C142: sguid="IID_IDirect3DLight"; break;
		case 0x4417C144: sguid="IID_IDirect3DMaterial"; break;
		case 0x93281503: sguid="IID_IDirect3DMaterial2"; break;
		case 0xca9c46f4: sguid="IID_IDirect3DMaterial3"; break;
		case 0x4417C145: sguid="IID_IDirect3DExecuteBuffer"; break;
		case 0x4417C146: sguid="IID_IDirect3DViewport"; break;
		case 0x93281500: sguid="IID_IDirect3DViewport2"; break;
		case 0xb0ab3b61: sguid="IID_IDirect3DViewport3"; break;
		case 0x7a503555: sguid="IID_IDirect3DVertexBuffer"; break;
		case 0xf5049e7d: sguid="IID_IDirect3DVertexBuffer7"; break;
		case 0xD7B70EE0: sguid="CLSID_DirectDraw"; break;
		case 0x3c305196: sguid="CLSID_DirectDraw7"; break;
		case 0x593817A0: sguid="CLSID_DirectDrawClipper"; break;
		default: sguid="UNKNOWN"; break;
	}
	return sguid;
}

char *ExplainNChitTest(DWORD c)
{
	static char *eb;
	switch(c)
	{
		case HTERROR:			eb="HTERROR"; break;
		case HTTRANSPARENT:		eb="HTTRANSPARENT"; break;
		case HTNOWHERE:			eb="HTNOWHERE"; break;
		case HTCLIENT:			eb="HTCLIENT"; break;
		case HTCAPTION:			eb="HTCAPTION"; break;
		case HTSYSMENU:			eb="HTSYSMENU"; break;
		case HTGROWBOX:			eb="HTGROWBOX"; break;
		case HTMENU:			eb="HTMENU"; break;
		case HTHSCROLL:			eb="HTHSCROLL"; break;
		case HTVSCROLL:			eb="HTVSCROLL"; break;
		case HTMINBUTTON:		eb="HTMINBUTTON"; break;
		case HTMAXBUTTON:		eb="HTMAXBUTTON"; break;
		case HTLEFT:			eb="HTLEFT"; break;
		case HTRIGHT:			eb="HTRIGHT"; break;
		case HTTOP:				eb="HTTOP"; break;
		case HTTOPLEFT:			eb="HTTOPLEFT"; break;
		case HTTOPRIGHT:		eb="HTTOPRIGHT"; break;
		case HTBOTTOM:			eb="HTBOTTOM"; break;
		case HTBOTTOMLEFT:		eb="HTBOTTOMLEFT"; break;
		case HTBOTTOMRIGHT:		eb="HTBOTTOMRIGHT"; break;
		case HTBORDER:			eb="HTBORDER"; break;
		case HTOBJECT:			eb="HTOBJECT"; break;
		case HTCLOSE:			eb="HTCLOSE"; break;
		case HTHELP:			eb="HTHELP"; break;
		default:				eb="Unknown"; break;
	}
	return eb;
}

char *ExplainDDEnumerateFlags(DWORD c)
{
	static char eb[256];
	unsigned int l;
	strcpy(eb,"DDENUM_");
	if (c & DDENUM_ATTACHEDSECONDARYDEVICES) strcat(eb, "ATTACHEDSECONDARYDEVICES+");
	if (c & DDENUM_DETACHEDSECONDARYDEVICES) strcat(eb, "DETACHEDSECONDARYDEVICES+");
	if (c & DDENUM_NONDISPLAYDEVICES) strcat(eb, "NONDISPLAYDEVICES+");
	l=strlen(eb);
	if (l>strlen("DDENUM_")) eb[l-1]=0; // delete last '+' if any
	else strcpy(eb,"NULL");
	return(eb);
}

 char *ExplainsSystemMetrics(DWORD c)
{
	char *Captions[94]={
		"SM_CXSCREEN", "SM_CYSCREEN", "SM_CXVSCROLL", "SM_CYHSCROLL", "SM_CYCAPTION",
		"SM_CXBORDER", "SM_CYBORDER", "SM_CXDLGFRAME", "SM_CYDLGFRAME", "SM_CYVTHUMB",
		"SM_CXHTHUMB", "SM_CXICON", "SM_CYICON", "SM_CXCURSOR", "SM_CYCURSOR",
		"SM_CYMENU", "SM_CXFULLSCREEN", "SM_CYFULLSCREEN", "SM_CYKANJIWINDOW", "SM_MOUSEPRESENT",
		"SM_CYVSCROLL", "SM_CXHSCROLL", "SM_DEBUG", "SM_SWAPBUTTON", "SM_RESERVED1",
		"SM_RESERVED2", "SM_RESERVED3", "SM_RESERVED4", "SM_CXMIN", "SM_CYMIN",
		"SM_CXSIZE", "SM_CYSIZE", "SM_CXFRAME", "SM_CYFRAME", "SM_CXMINTRACK",
		"SM_CYMINTRACK", "SM_CXDOUBLECLK", "SM_CYDOUBLECLK", "SM_CXICONSPACING", "SM_CYICONSPACING",
		"SM_MENUDROPALIGNMENT", "SM_PENWINDOWS", "SM_DBCSENABLED", "SM_CMOUSEBUTTONS", "SM_SECURE",
		"SM_CXEDGE", "SM_CYEDGE", "SM_CXMINSPACING", "SM_CYMINSPACING", "SM_CXSMICON",
		"SM_CYSMICON", "SM_CYSMCAPTION", "SM_CXSMSIZE", "SM_CYSMSIZE", "SM_CXMENUSIZE",
		"SM_CYMENUSIZE", "SM_ARRANGE", "SM_CXMINIMIZED", "SM_CYMINIMIZED", "SM_CXMAXTRACK",
		"SM_CYMAXTRACK", "SM_CXMAXIMIZED", "SM_CYMAXIMIZED", "SM_NETWORK", "64???",
		"65???", "66???", "SM_CLEANBOOT", "SM_CXDRAG", "SM_CYDRAG",
		"SM_SHOWSOUNDS", "SM_CXMENUCHECK", "SM_CYMENUCHECK", "SM_SLOWMACHINE", "SM_MIDEASTENABLED",
		"SM_MOUSEWHEELPRESENT", "SM_XVIRTUALSCREEN", "SM_YVIRTUALSCREEN", "SM_CXVIRTUALSCREEN", "SM_CYVIRTUALSCREEN",
		"SM_CMONITORS", "SM_SAMEDISPLAYFORMAT", "SM_IMMENABLED", "SM_CXFOCUSBORDER", "SM_CYFOCUSBORDER",
		"85???", "SM_TABLETPC", "SM_MEDIACENTER", "SM_STARTER", "SM_SERVERR2",
		"SM_CMETRICS(Win 0x501)", "SM_MOUSEHORIZONTALWHEELPRESENT", "SM_CXPADDEDBORDER", "SM_CMETRICS"};

	if (c>=0 && c<94) return Captions[c];
	switch(c){
		case SM_REMOTESESSION: return "SM_REMOTESESSION";
		case SM_SHUTTINGDOWN: return "SM_SHUTTINGDOWN";
		case SM_REMOTECONTROL: return "SM_REMOTECONTROL";
		case SM_CARETBLINKINGENABLED: return "SM_CARETBLINKINGENABLED";
	}
	return "???";
 }

char *ExplainWPFlags(DWORD c)
{
	static char eb[256];
	unsigned int l;
	strcpy(eb,"SWP_");
	if (c & SWP_NOSIZE) strcat(eb, "NOSIZE+");
	if (c & SWP_NOMOVE) strcat(eb, "NOMOVE+");
	if (c & SWP_NOZORDER) strcat(eb, "NOZORDER+");
	if (c & SWP_NOREDRAW) strcat(eb, "NOREDRAW+");
	if (c & SWP_NOACTIVATE) strcat(eb, "NOACTIVATE+");
	if (c & SWP_FRAMECHANGED) strcat(eb, "FRAMECHANGED+");
	if (c & SWP_SHOWWINDOW) strcat(eb, "SHOWWINDOW+");
	if (c & SWP_HIDEWINDOW) strcat(eb, "HIDEWINDOW+");
	if (c & SWP_NOCOPYBITS) strcat(eb, "NOCOPYBITS+");
	if (c & SWP_NOOWNERZORDER) strcat(eb, "NOOWNERZORDER+");
	if (c & SWP_NOSENDCHANGING) strcat(eb, "NOSENDCHANGING+");
	l=strlen(eb);
	if (l>strlen("SWP_")) eb[l-1]=0; // delete last '+' if any
	else strcpy(eb,"NULL");
	return(eb);
}

char *ExplainLoadLibFlags(DWORD c)
{
	static char eb[256];
	unsigned int l;
	strcpy(eb,"");
	if (c & DONT_RESOLVE_DLL_REFERENCES) strcat(eb, "DONT_RESOLVE_DLL_REFERENCES+");
	if (c & LOAD_LIBRARY_AS_DATAFILE) strcat(eb, "LOAD_LIBRARY_AS_DATAFILE+");
	if (c & LOAD_WITH_ALTERED_SEARCH_PATH) strcat(eb, "LOAD_WITH_ALTERED_SEARCH_PATH+");
	if (c & LOAD_IGNORE_CODE_AUTHZ_LEVEL) strcat(eb, "LOAD_IGNORE_CODE_AUTHZ_LEVEL+");
	if (c & LOAD_LIBRARY_AS_IMAGE_RESOURCE) strcat(eb, "LOAD_LIBRARY_AS_IMAGE_RESOURCE+");
	if (c & LOAD_LIBRARY_AS_DATAFILE_EXCLUSIVE) strcat(eb, "LOAD_LIBRARY_AS_DATAFILE_EXCLUSIVE+");
	l=strlen(eb);
	if (l>0) eb[l-1]=0; // delete last '+' if any
	else strcpy(eb,"NULL");
	return(eb);
}

char *ExplainDevModeFields(DWORD c)
{
	static char eb[512];
	unsigned int l;
	strcpy(eb,"DM_");
	if (c & DM_ORIENTATION) strcat(eb, "ORIENTATION+");
	if (c & DM_PAPERSIZE) strcat(eb, "PAPERSIZE+");
	if (c & DM_PAPERLENGTH) strcat(eb, "PAPERLENGTH+");
	if (c & DM_PAPERWIDTH) strcat(eb, "PAPERWIDTH+");
	if (c & DM_SCALE) strcat(eb, "SCALE+");
	if (c & DM_COPIES) strcat(eb, "COPIES+");
	if (c & DM_DEFAULTSOURCE) strcat(eb, "DEFAULTSOURCE+");
	if (c & DM_PRINTQUALITY) strcat(eb, "PRINTQUALITY+");
	if (c & DM_POSITION) strcat(eb, "POSITION+");
	if (c & DM_DISPLAYORIENTATION) strcat(eb, "DISPLAYORIENTATION+");
	if (c & DM_DISPLAYFIXEDOUTPUT) strcat(eb, "DISPLAYFIXEDOUTPUT+");
	if (c & DM_COLOR) strcat(eb, "COLOR+");
	if (c & DM_DUPLEX) strcat(eb, "DUPLEX+");
	if (c & DM_YRESOLUTION) strcat(eb, "YRESOLUTION+");
	if (c & DM_TTOPTION) strcat(eb, "TTOPTION+");
	if (c & DM_COLLATE) strcat(eb, "COLLATE+");
	if (c & DM_FORMNAME) strcat(eb, "FORMNAME+");
	if (c & DM_LOGPIXELS) strcat(eb, "LOGPIXELS+");
	if (c & DM_BITSPERPEL) strcat(eb, "BITSPERPEL+");
	if (c & DM_PELSWIDTH) strcat(eb, "PELSWIDTH+");
	if (c & DM_PELSHEIGHT) strcat(eb, "PELSHEIGHT+");
	if (c & DM_DISPLAYFLAGS) strcat(eb, "DISPLAYFLAGS+");
	if (c & DM_NUP) strcat(eb, "NUP+");
	if (c & DM_DISPLAYFREQUENCY) strcat(eb, "DISPLAYFREQUENCY+");
	if (c & DM_ICMMETHOD) strcat(eb, "ICMMETHOD+");
	if (c & DM_ICMINTENT) strcat(eb, "ICMINTENT+");
	if (c & DM_MEDIATYPE) strcat(eb, "MEDIATYPE+");
	if (c & DM_DITHERTYPE) strcat(eb, "DITHERTYPE+");
	if (c & DM_PANNINGWIDTH) strcat(eb, "PANNINGWIDTH+");
	if (c & DM_PANNINGHEIGHT) strcat(eb, "PANNINGHEIGHT+");
	l=strlen(eb);
	if (l>strlen("DM_")) eb[l-1]=0; // delete last '+' if any
	else strcpy(eb,"NULL");
	return(eb);
}

char *ExplainMCICommands(DWORD c)
{
	switch(c){
		case MCI_OPEN: return "MCI_OPEN";
		case MCI_CLOSE: return "MCI_CLOSE";
		case MCI_ESCAPE: return "MCI_ESCAPE";
		case MCI_PLAY: return "MCI_PLAY";
		case MCI_SEEK: return "MCI_SEEK";
		case MCI_STOP: return "MCI_STOP";
		case MCI_PAUSE: return "MCI_PAUSE";
		case MCI_INFO: return "MCI_INFO";
		case MCI_GETDEVCAPS: return "MCI_GETDEVCAPS";
		case MCI_SPIN: return "MCI_SPIN";
		case MCI_SET: return "MCI_SET";
		case MCI_STEP: return "MCI_STEP";
		case MCI_RECORD: return "MCI_RECORD";
		case MCI_SYSINFO: return "MCI_SYSINFO";
		case MCI_BREAK: return "MCI_BREAK";
		case MCI_SAVE: return "MCI_SAVE";
		case MCI_STATUS: return "MCI_STATUS";
		case MCI_CUE: return "MCI_CUE";
		case MCI_REALIZE: return "MCI_REALIZE";
		case MCI_WINDOW: return "MCI_WINDOW";
		case MCI_PUT: return "MCI_PUT";
		case MCI_WHERE: return "MCI_WHERE";
		case MCI_FREEZE: return "MCI_FREEZE";
		case MCI_UNFREEZE: return "MCI_UNFREEZE";
		case MCI_LOAD: return "MCI_LOAD";
		case MCI_CUT: return "MCI_CUT";
		case MCI_COPY: return "MCI_COPY";
		case MCI_PASTE: return "MCI_PASTE";
		case MCI_UPDATE: return "MCI_UPDATE";
		case MCI_RESUME: return "MCI_RESUME";
		case MCI_DELETE: return "MCI_DELETE";
		}
	return "???";
}

char *ExplainMCIFlags(DWORD cmd, DWORD c)
{
	static char eb[512];
	unsigned int l;
	strcpy(eb,"MCI_");
	// common flags
	if (c & MCI_NOTIFY) strcat(eb, "NOTIFY+");
	if (c & MCI_WAIT) strcat(eb, "WAIT+");
	if (c & MCI_FROM) strcat(eb, "FROM+");
	if (c & MCI_TO) strcat(eb, "TO+");
	if (c & MCI_TRACK) strcat(eb, "TRACK+");
	switch(cmd){
	case MCI_OPEN:
		if (c & MCI_OPEN_SHAREABLE) strcat(eb, "OPEN_SHAREABLE+");
		if (c & MCI_OPEN_ELEMENT) strcat(eb, "OPEN_ELEMENT+");
		if (c & MCI_OPEN_ALIAS) strcat(eb, "OPEN_ALIAS+");
		if (c & MCI_OPEN_ELEMENT_ID) strcat(eb, "OPEN_ELEMENT_ID+");
		if (c & MCI_OPEN_TYPE_ID) strcat(eb, "OPEN_TYPE_ID+");
		if (c & MCI_OPEN_TYPE) strcat(eb, "OPEN_TYPE+");
		if (c & MCI_WAVE_OPEN_BUFFER) strcat(eb, "WAVE_OPEN_BUFFER+");
		break;
	case MCI_SEEK:
		if (c & MCI_SEEK_TO_START) strcat(eb, "SEEK_TO_START+");
		if (c & MCI_STATUS_START) strcat(eb, "STATUS_START+");
		if (c & MCI_VD_SEEK_REVERSE) strcat(eb, "VD_SEEK_REVERSE+");
		break;
	case MCI_STATUS:
		if (c & MCI_STATUS_ITEM) strcat(eb, "STATUS_ITEM+");
		if (c & MCI_STATUS_START) strcat(eb, "STATUS_START+");
		break;
	case MCI_INFO:
		if (c & MCI_INFO_PRODUCT) strcat(eb, "INFO_PRODUCT+");
		if (c & MCI_INFO_FILE) strcat(eb, "INFO_FILE+");
		if (c & MCI_INFO_MEDIA_UPC) strcat(eb, "INFO_MEDIA_UPC+");
		if (c & MCI_INFO_MEDIA_IDENTITY) strcat(eb, "INFO_MEDIA_IDENTITY+");
		if (c & MCI_INFO_NAME) strcat(eb, "INFO_NAME+");
		if (c & MCI_INFO_COPYRIGHT) strcat(eb, "INFO_COPYRIGHT+");
		break;
	case MCI_GETDEVCAPS:
		if (c & MCI_VD_GETDEVCAPS_CLV) strcat(eb, "VD_GETDEVCAPS_CLV+");
		if (c & MCI_VD_GETDEVCAPS_CAV) strcat(eb, "VD_GETDEVCAPS_CAV+");
		if (c & MCI_GETDEVCAPS_ITEM) strcat(eb, "GETDEVCAPS_ITEM+");
		break;
	case MCI_SYSINFO:
		if (c & MCI_SYSINFO_QUANTITY) strcat(eb, "SYSINFO_QUANTITY+");
		if (c & MCI_SYSINFO_OPEN) strcat(eb, "SYSINFO_OPEN+");
		if (c & MCI_SYSINFO_NAME) strcat(eb, "SYSINFO_NAME+");
		if (c & MCI_SYSINFO_INSTALLNAME) strcat(eb, "SYSINFO_INSTALLNAME+");
		break;
	case MCI_SET:
		if (c & MCI_SET_DOOR_OPEN) strcat(eb, "SET_DOOR_OPEN+");
		if (c & MCI_SET_DOOR_CLOSED) strcat(eb, "SET_DOOR_CLOSED+");
		if (c & MCI_SET_TIME_FORMAT) strcat(eb, "SET_TIME_FORMAT+");
		if (c & MCI_SET_AUDIO) strcat(eb, "SET_AUDIO+");
		if (c & MCI_SET_VIDEO) strcat(eb, "SET_VIDEO+");
		if (c & MCI_SET_ON) strcat(eb, "SET_ON+");
		if (c & MCI_SET_OFF) strcat(eb, "SET_OFF+");
		if (c & MCI_SEQ_SET_TEMPO) strcat(eb, "SEQ_SET_TEMPO+");
		if (c & MCI_SEQ_SET_PORT) strcat(eb, "SEQ_SET_PORT+");
		if (c & MCI_SEQ_SET_SLAVE) strcat(eb, "SEQ_SET_SLAVE+");
		if (c & MCI_SEQ_SET_MASTER) strcat(eb, "SEQ_SET_MASTER+");
		if (c & MCI_SEQ_SET_OFFSET) strcat(eb, "SEQ_SET_OFFSET+");
		break;
	case MCI_BREAK:
		if (c & MCI_BREAK_KEY) strcat(eb, "BREAK_KEY+");
		if (c & MCI_BREAK_HWND) strcat(eb, "BREAK_HWND+");
		if (c & MCI_BREAK_OFF) strcat(eb, "BREAK_OFF+");
		break;
	case MCI_RECORD:
		if (c & MCI_RECORD_INSERT) strcat(eb, "RECORD_INSERT+");
		if (c & MCI_RECORD_OVERWRITE) strcat(eb, "RECORD_OVERWRITE+");
		break;
	case MCI_SAVE:
		if (c & MCI_SAVE_FILE) strcat(eb, "SAVE_FILE+");
		break;
	case MCI_LOAD:
		if (c & MCI_LOAD_FILE) strcat(eb, "SAVE_FILE+");
		break;
	case MCI_PLAY:
		if (c & MCI_VD_PLAY_REVERSE) strcat(eb, "VD_PLAY_REVERSE+");
		if (c & MCI_VD_PLAY_FAST) strcat(eb, "VD_PLAY_FAST+");
		if (c & MCI_VD_PLAY_SPEED) strcat(eb, "VD_PLAY_SPEED+");
		if (c & MCI_VD_PLAY_SCAN) strcat(eb, "VD_PLAY_SCAN+");
		if (c & MCI_VD_PLAY_SLOW) strcat(eb, "VD_PLAY_SLOW+");
		break;
	case MCI_STEP:
		if (c & MCI_VD_STEP_FRAMES) strcat(eb, "VD_STEP_FRAMES+");
		if (c & MCI_VD_STEP_REVERSE) strcat(eb, "VD_STEP_REVERSE+");
		break;
	case MCI_WINDOW:
		if (c & MCI_ANIM_WINDOW_HWND) strcat(eb, "ANIM_WINDOW_HWND+");
		if (c & MCI_ANIM_WINDOW_STATE) strcat(eb, "ANIM_WINDOW_STATE+");
		if (c & MCI_ANIM_WINDOW_TEXT) strcat(eb, "ANIM_WINDOW_TEXT+");
		if (c & MCI_ANIM_WINDOW_ENABLE_STRETCH) strcat(eb, "ANIM_WINDOW_ENABLE_STRETCH+");
		if (c & MCI_ANIM_WINDOW_DISABLE_STRETCH) strcat(eb, "ANIM_WINDOW_DISABLE_STRETCH+");
		break;
	case MCI_PUT:
		if (c & MCI_ANIM_RECT) strcat(eb, "ANIM_RECT+");
		if (c & MCI_ANIM_PUT_SOURCE) strcat(eb, "ANIM_PUT_SOURCE+");
		if (c & MCI_ANIM_PUT_DESTINATION) strcat(eb, "ANIM_PUT_DESTINATION+");
		break;
	case MCI_WHERE:
		if (c & MCI_ANIM_WHERE_SOURCE) strcat(eb, "ANIM_WHERE_SOURCE+");
		if (c & MCI_ANIM_WHERE_DESTINATION) strcat(eb, "ANIM_WHERE_DESTINATION+");
		break;
	case MCI_UPDATE:
		if (c & MCI_ANIM_UPDATE_HDC) strcat(eb, "ANIM_UPDATE_HDC+");
		break;
	}
	/*
	if (c & MCI_OVLY_OPEN_WS) strcat(eb, "OVLY_OPEN_WS+");
	if (c & MCI_OVLY_OPEN_PARENT) strcat(eb, "OVLY_OPEN_PARENT+");
	if (c & MCI_OVLY_STATUS_HWND) strcat(eb, "OVLY_STATUS_HWND+");
	if (c & MCI_OVLY_STATUS_STRETCH) strcat(eb, "OVLY_STATUS_STRETCH+");
	if (c & MCI_OVLY_INFO_TEXT) strcat(eb, "OVLY_INFO_TEXT+");
	if (c & MCI_OVLY_WINDOW_HWND) strcat(eb, "OVLY_WINDOW_HWND+");
	if (c & MCI_OVLY_WINDOW_STATE) strcat(eb, "OVLY_WINDOW_STATE+");
	if (c & MCI_OVLY_WINDOW_TEXT) strcat(eb, "OVLY_WINDOW_TEXT+");
	if (c & MCI_OVLY_WINDOW_ENABLE_STRETCH) strcat(eb, "OVLY_WINDOW_ENABLE_STRETCH+");
	if (c & MCI_OVLY_WINDOW_DISABLE_STRETCH) strcat(eb, "OVLY_WINDOW_DISABLE_STRETCH+");
	if (c & MCI_OVLY_WINDOW_DEFAULT) strcat(eb, "OVLY_WINDOW_DEFAULT+");
	if (c & MCI_OVLY_RECT) strcat(eb, "OVLY_RECT+");
	if (c & MCI_OVLY_PUT_SOURCE) strcat(eb, "OVLY_PUT_SOURCE+");
	if (c & MCI_OVLY_PUT_DESTINATION) strcat(eb, "OVLY_PUT_DESTINATION+");
	if (c & MCI_OVLY_PUT_FRAME) strcat(eb, "OVLY_PUT_FRAME+");
	if (c & MCI_OVLY_PUT_VIDEO) strcat(eb, "OVLY_PUT_VIDEO+");
	if (c & MCI_OVLY_WHERE_SOURCE) strcat(eb, "OVLY_WHERE_SOURCE+");
	if (c & MCI_OVLY_WHERE_DESTINATION) strcat(eb, "OVLY_WHERE_DESTINATION+");
	if (c & MCI_OVLY_WHERE_FRAME) strcat(eb, "OVLY_WHERE_FRAME+");
	if (c & MCI_OVLY_WHERE_VIDEO) strcat(eb, "OVLY_WHERE_VIDEO+");
	*/
	l=strlen(eb);
	if (l>strlen("MCI_")) eb[l-1]=0; // delete last '+' if any
	else strcpy(eb,"NULL");
	return(eb);
}

char *ExplainRegType(DWORD c)
{
	char *Captions[12]={
		"REG_NONE", "REG_SZ", "REG_EXPAND_SZ", "REG_BINARY", 
		"REG_DWORD", "REG_DWORD_BIG_ENDIAN", "REG_LINK", "REG_MULTI_SZ", 
		"REG_RESOURCE_LIST", "REG_FULL_RESOURCE_DESCRIPTOR",
		"REG_RESOURCE_REQUIREMENTS_LIST", "REG_QWORD"};

	if (c>=0 && c<12) return Captions[c];
	return "???";
}

char *ExplainDCType(DWORD c)
{
	char *Captions[GDI_OBJ_LAST+1]={
		"NULL", "OBJ_PEN", "OBJ_BRUSH", "OBJ_DC", 
		"OBJ_METADC", "OBJ_PAL", "OBJ_FONT", "OBJ_BITMAP", 
		"OBJ_REGION", "OBJ_METAFILE", "OBJ_MEMDC", "OBJ_EXTPEN",
		"OBJ_ENHMETADC", "OBJ_ENHMETAFILE", "OBJ_COLORSPACE"};

	if (c>=0 && c<=GDI_OBJ_LAST) return Captions[c];
	return "???";
}
