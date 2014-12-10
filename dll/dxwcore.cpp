#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include "dxwnd.h"
#include "dxwcore.hpp"
#include "syslibs.h"
#include "dxhelper.h"
#include "resource.h"

/* ------------------------------------------------------------------ */
// Internal function pointers
/* ------------------------------------------------------------------ */

typedef DWORD (*TimeShifter_Type)(DWORD, int);
typedef LARGE_INTEGER (*TimeShifter64_Type)(LARGE_INTEGER, int);

TimeShifter_Type pTimeShifter;
TimeShifter64_Type pTimeShifter64;

static DWORD TimeShifterFine(DWORD, int);
static LARGE_INTEGER TimeShifter64Fine(LARGE_INTEGER, int);
static DWORD TimeShifterCoarse(DWORD, int);
static LARGE_INTEGER TimeShifter64Coarse(LARGE_INTEGER, int);

/* ------------------------------------------------------------------ */
// Constructor, destructor, initialization....
/* ------------------------------------------------------------------ */

dxwCore::dxwCore()
{
	// initialization stuff ....
	extern void WinDBInit();
	FullScreen=FALSE;
	SethWnd(NULL);
	SetScreenSize();
	dwMaxDDVersion=7;
	hParentWnd = 0;
	hChildWnd = 0;
	bActive = TRUE;
	bDInputAbs = 0;
	TimeShift = 0;
	lpDDSPrimHDC = NULL;
	memset(PrimSurfaces, 0, sizeof(PrimSurfaces));
	ResetEmulatedDC();
	MustShowOverlay=FALSE;
	TimerEvent.dwTimerType = TIMER_TYPE_NONE;
	WinDBInit();
	// initialization of default vsync emulation array
	iRefreshDelays[0]=16;
	iRefreshDelays[1]=17;
	iRefreshDelayCount=2;
}

dxwCore::~dxwCore()
{
}

void dxwCore::SetFullScreen(BOOL fs, int line) 
{
	OutTraceDW("SetFullScreen: %s at %d\n", fs?"FULLSCREEN":"WINDOWED", line);
	FullScreen=fs;
}

void dxwCore::SetFullScreen(BOOL fs) 
{
	if(dxw.dwFlags3 & FULLSCREENONLY) fs=TRUE;
	OutTraceDW("SetFullScreen: %s\n", fs?"FULLSCREEN":"WINDOWED");
	FullScreen=fs;
}

BOOL dxwCore::IsFullScreen()
{
	//if(!Windowize) return FALSE;
	return FullScreen;
}

void dxwCore::InitTarget(TARGETMAP *target)
{
	dwFlags1 = target->flags;
	dwFlags2 = target->flags2;
	dwFlags3 = target->flags3;
	dwFlags4 = target->flags4;
	dwFlags5 = target->flags5;
	dwTFlags = target->tflags;
	Windowize = (dwFlags2 & WINDOWIZE) ? TRUE : FALSE;
	if(dwFlags3 & FULLSCREENONLY) FullScreen=TRUE;
	gsModules = target->module;
	MaxFPS = target->MaxFPS;
	CustomOpenGLLib = target->OpenGLLib;
	if(!strlen(CustomOpenGLLib)) CustomOpenGLLib=NULL;
	// bounds control
	dwTargetDDVersion = target->dxversion;
	if(dwTargetDDVersion<0) dwTargetDDVersion=0;
	if(dwTargetDDVersion>12) dwTargetDDVersion=12;
	TimeShift = target->InitTS;
	if(TimeShift < -8) TimeShift = -8;
	if(TimeShift >  8) TimeShift =  8;
	FakeVersionId = target->FakeVersionId;
	MaxScreenRes = target->MaxScreenRes;
	Coordinates = target->coordinates;
	MustShowOverlay=((dwFlags2 & SHOWFPSOVERLAY) || (dwFlags4 & SHOWTIMESTRETCH));
	if(dwFlags4 & FINETIMING){
		pTimeShifter = TimeShifterFine;
		pTimeShifter64 = TimeShifter64Fine;
	}
	else{
		pTimeShifter = TimeShifterCoarse;
		pTimeShifter64 = TimeShifter64Coarse;
	}
	iSizX = target->sizx;
	iSizY = target->sizy;
	iPosX = target->posx;
	iPosY = target->posy;
	// Aspect Ratio from window size, or traditional 4:3 by default
	iRatioX = iSizX ? iSizX : 800;
	iRatioY = iSizY ? iSizY : 600;
}

void dxwCore::SetScreenSize(void) 
{
	if(dxw.Windowize)
		SetScreenSize(800, 600); // set to default screen resolution
	else{
		int sizx, sizy;
		sizx = GetSystemMetrics(SM_CXSCREEN);
		sizy = GetSystemMetrics(SM_CYSCREEN);
		SetScreenSize(sizx, sizy);
	}
}

void dxwCore::SetScreenSize(int x, int y) 
{
	DXWNDSTATUS *p;
	OutTraceDW("DXWND: set screen size=(%d,%d)\n", x, y);
	if(x) dwScreenWidth=x; 
	if(y) dwScreenHeight=y;
	p = GetHookInfo();
	if(p) {
		p->Width = (short)dwScreenWidth;
		p->Height = (short)dwScreenHeight;
	}
	if(dwFlags4 & LIMITSCREENRES){
		#define HUGE 100000
		DWORD maxw, maxh;
		maxw=HUGE; maxh=HUGE;
		switch(dxw.MaxScreenRes){
			case DXW_LIMIT_320x200: maxw=320; maxh=200; break;
			case DXW_LIMIT_400x300: maxw=400; maxh=300; break;
			case DXW_LIMIT_640x480: maxw=640; maxh=480; break;
			case DXW_LIMIT_800x600: maxw=800; maxh=600; break;
			case DXW_LIMIT_1024x768: maxw=1024; maxh=768; break;
			case DXW_LIMIT_1280x960: maxw=1280; maxh=960; break;
		}
		if(((DWORD)p->Width > maxw) || ((DWORD)p->Height > maxh)){
			OutTraceDW("DXWND: limit device size=(%d,%d)\n", maxw, maxh);
			// v2.02.95 setting new virtual desktop size 
			dwScreenWidth = p->Width = (short)maxw;
			dwScreenHeight= p->Height = (short)maxh;
		}
	}
}

void dxwCore::DumpDesktopStatus()
{
	HDC hDC;
	HWND hDesktop;
	RECT desktop;
	PIXELFORMATDESCRIPTOR pfd;
	int  iPixelFormat, iBPP;
	char ColorMask[32+1]; 

	// get the current pixel format index
	hDesktop = GetDesktopWindow();
	hDC = GetDC(hDesktop);
	::GetWindowRect(hDesktop, &desktop);
	iBPP = GetDeviceCaps(hDC, BITSPIXEL);
	iPixelFormat = GetPixelFormat(hDC); 
	if(!iPixelFormat) iPixelFormat=1; // why returns 0???
	// obtain a detailed description of that pixel format  
	if(!DescribePixelFormat(hDC, iPixelFormat, sizeof(PIXELFORMATDESCRIPTOR), &pfd)){
		OutTrace("DescribePixelFormat ERROR: err=%d\n", GetLastError());
		return;
	}

	memset(ColorMask, ' ', 32); // blank fill
	ColorMask[32] = 0; // terminate
	if ((pfd.cRedShift+pfd.cRedBits <= 32) &&
		(pfd.cGreenShift+pfd.cGreenBits <= 32) &&
		(pfd.cBlueShift+pfd.cBlueBits <= 32) &&
		(pfd.cAlphaShift+pfd.cAlphaBits <= 32)){ // everything within the 32 bits ...
		for (int i=pfd.cRedShift; i<pfd.cRedShift+pfd.cRedBits; i++) ColorMask[i]='R';
		for (int i=pfd.cGreenShift; i<pfd.cGreenShift+pfd.cGreenBits; i++) ColorMask[i]='G';
		for (int i=pfd.cBlueShift; i<pfd.cBlueShift+pfd.cBlueBits; i++) ColorMask[i]='B';
		for (int i=pfd.cAlphaShift; i<pfd.cAlphaShift+pfd.cAlphaBits; i++) ColorMask[i]='A';
	}
	else
		strcpy(ColorMask, "???");
	OutTrace( 
		"Desktop"
		"\tSize (W x H)=(%d x %d)\n" 
		"\tColor depth = %d (color bits = %d)\n"
		"\tPixel format = %d\n"
		"\tColor mask  (RGBA)= (%d,%d,%d,%d)\n" 
		"\tColor shift (RGBA)= (%d,%d,%d,%d)\n"
		"\tColor mask = \"%s\"\n"
		, 
		desktop.right, desktop.bottom, 
		iBPP, pfd.cColorBits,
		iPixelFormat,
		pfd.cRedBits, pfd.cGreenBits, pfd.cBlueBits, pfd.cAlphaBits,
		pfd.cRedShift, pfd.cGreenShift, pfd.cBlueShift, pfd.cAlphaShift,
		ColorMask
	);
}



/* ------------------------------------------------------------------ */
// Primary surfaces auxiliary functions
/* ------------------------------------------------------------------ */

void dxwCore::UnmarkPrimarySurface(LPDIRECTDRAWSURFACE ps)
{
	int i;
	// OutTraceDW("PRIMARYSURFACE del %x\n",ps);
	for (i=0;i<DDSQLEN;i++) {
		if (PrimSurfaces[i]==(DWORD)ps) break; 
		if (PrimSurfaces[i]==0) break;
	}
	if (PrimSurfaces[i]==(DWORD)ps){
		for (; i<DDSQLEN; i++){
			PrimSurfaces[i]=PrimSurfaces[i+1];
			if (PrimSurfaces[i]==0) break;
		}
		PrimSurfaces[DDSQLEN]=0;
	}
}

void dxwCore::UnmarkBackBufferSurface(LPDIRECTDRAWSURFACE ps)
{
	int i;
	// OutTraceDW("PRIMARYSURFACE del %x\n",ps);
	for (i=0;i<DDSQLEN;i++) {
		if (BackSurfaces[i]==(DWORD)ps) break; 
		if (BackSurfaces[i]==0) break;
	}
	if (BackSurfaces[i]==(DWORD)ps){
		for (; i<DDSQLEN; i++){
			BackSurfaces[i]=BackSurfaces[i+1];
			if (BackSurfaces[i]==0) break;
		}
		BackSurfaces[DDSQLEN]=0;
	}
}

void dxwCore::MarkPrimarySurface(LPDIRECTDRAWSURFACE ps)
{
	int i;
	// OutTraceDW("PRIMARYSURFACE add %x\n",ps);
	for (i=0;i<DDSQLEN;i++) {
		if (PrimSurfaces[i]==(DWORD)ps) return; // if already there ....
		if (PrimSurfaces[i]==(DWORD)0) break; // got end of list
	}
	PrimSurfaces[i]=(DWORD)ps;
	UnmarkBackBufferSurface(ps);
}

void dxwCore::MarkBackBufferSurface(LPDIRECTDRAWSURFACE ps)
{
	int i;
	// OutTraceDW("PRIMARYSURFACE add %x\n",ps);
	for (i=0;i<DDSQLEN;i++) {
		if (BackSurfaces[i]==(DWORD)ps) return; // if already there ....
		if (BackSurfaces[i]==(DWORD)0) break; // got end of list
	}
	BackSurfaces[i]=(DWORD)ps;
	UnmarkPrimarySurface(ps);
}

void dxwCore::MarkRegularSurface(LPDIRECTDRAWSURFACE ps)
{
	UnmarkBackBufferSurface(ps);
	UnmarkPrimarySurface(ps);
}

// Note: since MS itself declares that the object refcount is not reliable and should
// be used for debugging only, it's not safe to rely on refcount==0 when releasing a
// surface to terminate its classification as primary. As an alternate and more reliable
// way, we use UnmarkPrimarySurface each time you create a new not primary surface, 
// eliminating the risk that a surface previously classified as primary and then closed
// had the same surface handle than this new one that is not primary at all.

BOOL dxwCore::IsAPrimarySurface(LPDIRECTDRAWSURFACE ps)
{
	int i;
	// treat NULL surface ptr as a non primary
	if(!ps) return FALSE;
	for (i=0;i<DDSQLEN;i++) {
		if (PrimSurfaces[i]==0) return FALSE;
		if (PrimSurfaces[i]==(DWORD)ps) return TRUE;
	}
	return FALSE;
}

BOOL dxwCore::IsABackBufferSurface(LPDIRECTDRAWSURFACE ps)
{
	int i;
	// treat NULL surface ptr as a non primary
	if(!ps) return FALSE;
	for (i=0;i<DDSQLEN;i++) {
		if (BackSurfaces[i]==0) return FALSE;
		if (BackSurfaces[i]==(DWORD)ps) return TRUE;
	}
	return FALSE;
}

LPDIRECTDRAWSURFACE dxwCore::GetPrimarySurface(void)
{
	// return last opened one....
	int i;
	for (i=0;i<DDSQLEN;i++) {
		if (PrimSurfaces[i]==0) break;
	}
	if (i) return((LPDIRECTDRAWSURFACE)PrimSurfaces[i-1]);
	return NULL;
}

LPDIRECTDRAWSURFACE dxwCore::GetBackBufferSurface(void)
{
	// return last opened one....
	int i;
	for (i=0;i<DDSQLEN;i++) {
		if (BackSurfaces[i]==0) break;
	}
	if (i) return((LPDIRECTDRAWSURFACE)BackSurfaces[i-1]);
	return NULL;
}

void dxwCore::SetPrimarySurface(void)
{
	if (!lpDDSPrimHDC) lpDDSPrimHDC=GetPrimarySurface();
}

void dxwCore::ResetPrimarySurface(void)
{
	lpDDSPrimHDC=NULL;
}

void dxwCore::InitWindowPos(int x, int y, int w, int h)
{
	iPosX = x;
	iPosY = y; //v2.02.09
	iSizX = w;
	iSizY = h;
}

RECT dxwCore::GetScreenRect()
{
	static RECT Screen;
	Screen.left=0;
	Screen.top=0;
	Screen.right=dwScreenWidth;
	Screen.bottom=dwScreenHeight;
	return Screen;
}

RECT dxwCore::GetUnmappedScreenRect()
{
	static RECT Screen;
	POINT upleft = {0, 0};
	memset(&Screen, 0, sizeof(RECT));
	(*pGetClientRect)(hWnd, &Screen);
	(*pClientToScreen)(hWnd, &upleft);
	Screen.top += upleft.x;
	Screen.bottom += upleft.x;
	Screen.left += upleft.y;
	Screen.right += upleft.y;
	return Screen;
}

POINT dxwCore::GetFrameOffset()
{
	RECT wrect;
	POINT FrameOffset={0, 0};
	(*pGetWindowRect)(hWnd, &wrect);
	(*pClientToScreen)(hWnd, &FrameOffset);
	FrameOffset.x -= wrect.left;
	FrameOffset.y -= wrect.top;
	OutTraceB("GetFrameOffset: offset=(%d,%d)\n", FrameOffset.x, FrameOffset.y);
	return FrameOffset;
}

BOOL dxwCore::IsDesktop(HWND hwnd)
{
	return (
		(hwnd == 0)
		||
		(hwnd == (*pGetDesktopWindow)())
		||
		(hwnd == hWnd)
		);
}

BOOL dxwCore::IsRealDesktop(HWND hwnd)
{
	return (
		(hwnd == 0)
		||
		(hwnd == (*pGetDesktopWindow)())
		);
}

// v2.1.93: FixCursorPos completely revised to introduce a clipping tolerance in
// clipping regions as well as in normal operations

#define CLIP_TOLERANCE 4

POINT dxwCore::FixCursorPos(POINT prev)
{
	POINT curr;
	RECT rect;
	extern LPRECT lpClipRegion;

	curr=prev;

	// scale mouse coordinates
	// remember: rect from GetClientRect always start at 0,0!
	if(dxw.dwFlags1 & MODIFYMOUSE){
		int w, h, b; // width, height and border

		if (!(*pGetClientRect)(hWnd, &rect)) { // v2.02.30: always use desktop win
			OutTraceDW("GetClientRect ERROR %d at %d\n", GetLastError(),__LINE__);
			curr.x = curr.y = 0;
		}
		w = rect.right - rect.left;
		h = rect.bottom - rect.top;

		if (dxw.dwFlags2 & KEEPASPECTRATIO) {
			if ((w * iRatioY) > (h * iRatioX)){
				b = (w - (h * iRatioX / iRatioY))/2;
				curr.x -= b;
				w -= 2*b;
			}
			else {
				b = (h - (w * iRatioY / iRatioX))/2;
				curr.y -= b;
				h -= 2*b;
			}
		}

		if(dxw.dwFlags4 & RELEASEMOUSE){
			if ((curr.x < 0) ||
				(curr.y < 0) ||
				(curr.x > w) ||
				(curr.y > h)){
				curr.x = w / 2;
				curr.y = h / 2;
			}
		}
		else {
			if (curr.x < 0) curr.x = 0;
			if (curr.y < 0) curr.y = 0;
			if (curr.x > w) curr.x = w;
			if (curr.y > h) curr.y = h;
		}

		if (w) curr.x = (curr.x * dxw.GetScreenWidth()) / w;
		if (h) curr.y = (curr.y * dxw.GetScreenHeight()) / h;
	}

	if(dxw.dwFlags4 & FRAMECOMPENSATION){
		static int dx, dy, todo=TRUE;
		if (todo){
			POINT FrameOffset = dxw.GetFrameOffset();
			dx=FrameOffset.x;
			dy=FrameOffset.y;
			OutTraceC("GetCursorPos: frame compensation=(%d,%d)\n", dx, dy);
			todo=FALSE;
		}
		curr.x += dx;
		curr.y += dy;
	}

	if((dxw.dwFlags1 & ENABLECLIPPING) && lpClipRegion){
		// v2.1.93:
		// in clipping mode, avoid the cursor position to lay outside the valid rect
		// note 1: the rect follow the convention and valid coord lay between left to righ-1,
		// top to bottom-1
		// note 2: CLIP_TOLERANCE is meant to handle possible integer divide tolerance errors
		// that may prevent reaching the clip rect borders. The smaller you shrink the window, 
		// the bigger tolerance is required
		if (curr.x < lpClipRegion->left+CLIP_TOLERANCE) curr.x=lpClipRegion->left;
		if (curr.y < lpClipRegion->top+CLIP_TOLERANCE) curr.y=lpClipRegion->top;
		if (curr.x >= lpClipRegion->right-CLIP_TOLERANCE) curr.x=lpClipRegion->right-1;
		if (curr.y >= lpClipRegion->bottom-CLIP_TOLERANCE) curr.y=lpClipRegion->bottom-1;
	}
	else{
		if (curr.x < CLIP_TOLERANCE) curr.x=0;
		if (curr.y < CLIP_TOLERANCE) curr.y=0;
		if (curr.x >= (LONG)dxw.GetScreenWidth()-CLIP_TOLERANCE) curr.x=dxw.GetScreenWidth()-1;
		if (curr.y >= (LONG)dxw.GetScreenHeight()-CLIP_TOLERANCE) curr.y=dxw.GetScreenHeight()-1;
	}

	return curr;
}

POINT dxwCore::ScreenToClient(POINT point)
{
	// convert absolute screen coordinates to frame relative
	if (!(*pScreenToClient)(hWnd, &point)) {
		OutTraceE("ScreenToClient(%x) ERROR %d at %d\n", hWnd, GetLastError(), __LINE__);
		point.x =0; point.y=0;
	}

	return point;
}

void dxwCore::FixNCHITCursorPos(LPPOINT lppoint)
{
	RECT rect;
	POINT point;

	point=*lppoint;
	(*pGetClientRect)(dxw.GethWnd(), &rect);
	(*pScreenToClient)(dxw.GethWnd(), &point);

	if (point.x < 0) return;
	if (point.y < 0) return;
	if (point.x > rect.right) return;
	if (point.y > rect.bottom) return;

	*lppoint=point;
	lppoint->x = (lppoint->x * dxw.GetScreenWidth()) / rect.right;
	lppoint->y = (lppoint->y * dxw.GetScreenHeight()) / rect.bottom;
	if(lppoint->x < CLIP_TOLERANCE) lppoint->x=0;
	if(lppoint->y < CLIP_TOLERANCE) lppoint->y=0;
	if(lppoint->x > (LONG)dxw.GetScreenWidth()-CLIP_TOLERANCE) lppoint->x=dxw.GetScreenWidth()-1;
	if(lppoint->y > (LONG)dxw.GetScreenHeight()-CLIP_TOLERANCE) lppoint->y=dxw.GetScreenHeight()-1;
}

void dxwCore::SetClipCursor()
{
	RECT Rect;
	POINT UpLeftCorner={0,0};

	OutTraceDW("SetClipCursor:\n");
	if (hWnd==NULL) {
		OutTraceDW("SetClipCursor: ASSERT hWnd==NULL\n");
		return;
	}
	if(!(*pGetClientRect)(hWnd, &Rect))
		OutTraceE("GetClientRect: ERROR err=%d at %d\n", GetLastError(), __LINE__);
	if(!(*pClientToScreen)(hWnd, &UpLeftCorner))
		OutTraceE("ClientToScreen: ERROR err=%d at %d\n", GetLastError(), __LINE__);
	Rect.left+=UpLeftCorner.x;
	Rect.right+=UpLeftCorner.x;
	Rect.top+=UpLeftCorner.y;
	Rect.bottom+=UpLeftCorner.y;
	(*pClipCursor)(NULL);
	if(!(*pClipCursor)(&Rect))
		OutTraceE("ClipCursor: ERROR err=%d at %d\n", GetLastError(), __LINE__);
	OutTraceDW("SetClipCursor: rect=(%d,%d)-(%d,%d)\n",
		Rect.left, Rect.top, Rect.right, Rect.bottom);
}

void dxwCore::EraseClipCursor()
{
	OutTraceDW("EraseClipCursor:\n");
	(*pClipCursor)(NULL);
}

void dxwCore::SethWnd(HWND hwnd) 
{
	RECT WinRect;
	if(!pGetWindowRect) pGetWindowRect=::GetWindowRect;
	if(!pGDIGetDC)		pGDIGetDC=::GetDC;

	hWnd=hwnd; 
	hWndFPS=hwnd;	
	RealHDC=(*pGDIGetDC)(hwnd);

	(*pGetWindowRect)(hwnd, &WinRect);
	OutTraceDW("SethWnd: setting main win=%x hdc=%x pos=(%d,%d)-(%d,%d)\n", 
		hwnd, RealHDC, WinRect.left, WinRect.top, WinRect.right, WinRect.bottom);
}

void dxwCore::FixWorkarea(LPRECT workarea)
{
	int w, h, b; // width, height and border

	w = workarea->right - workarea->left;
	h = workarea->bottom - workarea->top;
	if ((w * iRatioY) > (h * iRatioX)){
		b = (w - (h * iRatioX / iRatioY))/2;
		workarea->left += b;
		workarea->right -= b;
	}
	else {
		b = (h - (w * iRatioY / iRatioX))/2;
		workarea->top += b;
		workarea->bottom -= b;
	}
}

RECT dxwCore::MapWindowRect(LPRECT lpRect)
{
	POINT UpLeft={0,0};
	RECT RetRect;
	RECT ClientRect;
	int w, h, bx, by; // width, height and x,y borders

	if (!(*pGetClientRect)(hWnd, &ClientRect)){
		OutTraceE("GetClientRect ERROR: err=%d hwnd=%x at %d\n", GetLastError(), hWnd, __LINE__);
		// v2.02.71: return a void area to prevent blitting to wrong area
		ClientRect.top=ClientRect.left=ClientRect.right=ClientRect.bottom=0;
		return ClientRect;
	}

	if(!Windowize){
		if(lpRect) 
			RetRect=*lpRect;
		else{
			RetRect.left = RetRect.top = 0;
			RetRect.right = dwScreenWidth;
			RetRect.bottom = dwScreenHeight;
		}
		return RetRect;
	}
	
	RetRect=ClientRect;
	bx = by = 0;
	if (dwFlags2 & KEEPASPECTRATIO){
		w = RetRect.right - RetRect.left;
		h = RetRect.bottom - RetRect.top;
		if ((w * iRatioY) > (h * iRatioX)){
			bx = (w - (h * iRatioX / iRatioY))/2;
		}
		else {
			by = (h - (w * iRatioY / iRatioX))/2;
		}
	}

	if(lpRect){ // v2.02.41 - fixed coordinates for KEEPASPECTRATIO option
		LONG Width, Height;
		Width = ClientRect.right - (2*bx);
		Height = ClientRect.bottom - (2*by);
		RetRect.left = bx + (lpRect->left * Width / dwScreenWidth);
		RetRect.right = bx + (lpRect->right * Width / dwScreenWidth);
		RetRect.top = by + (lpRect->top * Height / dwScreenHeight);
		RetRect.bottom = by + (lpRect->bottom * Height / dwScreenHeight);
	}
	else{
		RetRect.left = ClientRect.left + bx;
		RetRect.right = ClientRect.right - bx;
		RetRect.top = ClientRect.top + by;
		RetRect.bottom = ClientRect.bottom - by;
	}

	if(!(*pClientToScreen)(hWnd, &UpLeft)){
		OutTraceE("ClientToScreen ERROR: err=%d hwnd=%x at %d\n", GetLastError(), hWnd, __LINE__);
	}
	if(!OffsetRect(&RetRect ,UpLeft.x, UpLeft.y)){
		OutTraceE("OffsetRect ERROR: err=%d hwnd=%x at %d\n", GetLastError(), hWnd, __LINE__);
	}
	return RetRect;
}

void dxwCore::MapClient(LPRECT rect)
{
	RECT client;
	int w, h;
	if(!(*pGetClientRect)(hWnd, &client)) return;
	w = client.right ? client.right : iSizX;
	h = client.bottom ? client.bottom : iSizY;
	rect->left= ((rect->left * w)+(dwScreenWidth >> 1)) / (int)dwScreenWidth;
	rect->top= ((rect->top * h)+(dwScreenHeight >> 1)) / (int)dwScreenHeight;
	rect->right= ((rect->right * w)+(dwScreenWidth >> 1)) / (int)dwScreenWidth;
	rect->bottom= ((rect->bottom * h)+(dwScreenHeight >> 1)) / (int)dwScreenHeight;
}

void dxwCore::MapClient(int *nXDest, int *nYDest, int *nWDest, int *nHDest)
{
	RECT client;
	int w, h;
	if(!(*pGetClientRect)(hWnd, &client)) return;
	w = client.right ? client.right : iSizX;
	h = client.bottom ? client.bottom : iSizY;
	*nXDest= ((*nXDest * w)+(dwScreenWidth >> 1)) / (int)dwScreenWidth;
	*nYDest= ((*nYDest * h)+(dwScreenHeight >> 1)) / (int)dwScreenHeight;
	*nWDest= ((*nWDest * w)+(dwScreenWidth >> 1)) / (int)dwScreenWidth;
	*nHDest= ((*nHDest * h)+(dwScreenHeight >> 1)) / (int)dwScreenHeight;
}

void dxwCore::MapClient(LPPOINT lppoint)
{
	RECT client;
	int w, h;
	if(!(*pGetClientRect)(hWnd, &client)) return;
	w = client.right ? client.right : iSizX;
	h = client.bottom ? client.bottom : iSizY;
	lppoint->x= ((lppoint->x * w)+(dwScreenWidth >> 1)) / (int)dwScreenWidth;
	lppoint->y= ((lppoint->y * h)+(dwScreenHeight >> 1)) / (int)dwScreenHeight;
}

void dxwCore::MapClient(int *nXDest, int *nYDest)
{
	RECT client;
	int w, h;
	if(!(*pGetClientRect)(hWnd, &client)) return;
	w = client.right ? client.right : iSizX;
	h = client.bottom ? client.bottom : iSizY;
	*nXDest= ((*nXDest * w)+(dwScreenWidth >> 1)) / (int)dwScreenWidth;
	*nYDest= ((*nYDest * h)+(dwScreenHeight >> 1)) / (int)dwScreenHeight;
}

void dxwCore::UnmapClient(LPPOINT lppoint)
{
	RECT client;
	int w, h;
	if(!(*pGetClientRect)(hWnd, &client)) return;
	w = client.right ? client.right : iSizX;
	h = client.bottom ? client.bottom : iSizY;
	if(w) lppoint->x = ((lppoint->x * (int)dwScreenWidth) + (w >> 1)) / w;
	if(h) lppoint->y = ((lppoint->y * (int)dwScreenHeight) + (h >> 1)) / h;
}

void dxwCore::UnmapClient(int *nXDest, int *nYDest)
{
	RECT client;
	int w, h;
	if(!(*pGetClientRect)(hWnd, &client)) return;
	w = client.right ? client.right : iSizX;
	h = client.bottom ? client.bottom : iSizY;
	if(w) *nXDest = ((*nXDest * (int)dwScreenWidth) + (w >> 1)) / w;
	if(h) *nYDest = ((*nYDest * (int)dwScreenHeight) + (h >> 1)) / h;
}

void dxwCore::UnmapClient(LPRECT lpRect)
{
	RECT client;
	int w, h;
	if(!(*pGetClientRect)(hWnd, &client)) return;
	w = client.right ? client.right : iSizX;
	h = client.bottom ? client.bottom : iSizY;
	if(w) {
		lpRect->left = ((lpRect->left * (int)dwScreenWidth) + (w >> 1)) / w;
		lpRect->right = ((lpRect->right * (int)dwScreenWidth) + (w >> 1)) / w;
	}
	if(h) {
		lpRect->top = ((lpRect->top * (int)dwScreenHeight) + (h >> 1)) / h;
		lpRect->bottom = ((lpRect->bottom * (int)dwScreenHeight) + (h >> 1)) / h;
	}
}

void dxwCore::MapWindow(LPRECT rect)
{
	RECT client;
	POINT upleft = {0,0};
	if(!(*pGetClientRect)(hWnd, &client)) return;
	(*pClientToScreen)(hWnd, &upleft);
	rect->left= upleft.x + (((rect->left * client.right) + (dwScreenWidth >> 1)) / (int)dwScreenWidth);
	rect->top= upleft.y + (((rect->top * client.bottom) + (dwScreenHeight >> 1)) / (int)dwScreenHeight);
	rect->right= upleft.x + (((rect->right * client.right) + (dwScreenWidth >> 1)) / (int)dwScreenWidth);
	rect->bottom= upleft.y + (((rect->bottom * client.bottom) + (dwScreenHeight >> 1)) / (int)dwScreenHeight);
}

void dxwCore::MapWindow(int *nXDest, int *nYDest, int *nWDest, int *nHDest)
{
	RECT client;
	POINT upleft = {0,0};
	if(!(*pGetClientRect)(hWnd, &client)) return;
	(*pClientToScreen)(hWnd, &upleft);
	*nXDest= upleft.x + (((*nXDest * client.right) + (dwScreenWidth >> 1)) / (int)dwScreenWidth);
	*nYDest= upleft.y + (((*nYDest * client.bottom) + (dwScreenHeight >> 1)) / (int)dwScreenHeight);
	*nWDest= ((*nWDest * client.right) + (dwScreenWidth >> 1)) / (int)dwScreenWidth;
	*nHDest= ((*nHDest * client.bottom) + (dwScreenHeight >> 1)) / (int)dwScreenHeight;
}

void dxwCore::MapWindow(LPPOINT lppoint)
{
	RECT client;
	POINT upleft = {0,0};
	if(!(*pGetClientRect)(hWnd, &client)) return;
	(*pClientToScreen)(hWnd, &upleft);
	lppoint->x = upleft.x + (((lppoint->x * client.right) + (dwScreenWidth >> 1)) / dwScreenWidth);
	lppoint->y = upleft.y + (((lppoint->y * client.bottom) + (dwScreenHeight >> 1)) / dwScreenHeight);
}

POINT dxwCore::ClientOffset(HWND hwnd)
{
	RECT desktop;
	POINT upleft, win0, desk0, ret;
	ret.x = ret.y = 0;

	(*pGetClientRect)(hWnd,&desktop);
	if(!desktop.right || !desktop.bottom) return ret;

	upleft.x = upleft.y = 0;
	(*pClientToScreen)(hwnd, &upleft);
	win0 = upleft;
	upleft.x = upleft.y = 0;
	(*pClientToScreen)(hWnd, &upleft);
	desk0 = upleft;
	if (desktop.right) ret.x = (((win0.x - desk0.x) * (LONG)dwScreenWidth) + (desktop.right >> 1)) / desktop.right;
	if (desktop.bottom) ret.y = (((win0.y - desk0.y) * (LONG)dwScreenHeight) + (desktop.bottom >> 1)) / desktop.bottom;
	OutTraceB("ClientOffset: hwnd=%x offset=(%d,%d)\n", hwnd, ret.x, ret.y);
	return ret;
}

RECT dxwCore::GetWindowRect(RECT win)
{
	RECT desktop;
	POINT desk0;
	desk0.x = desk0.y = 0;

	(*pGetClientRect)(hWnd, &desktop);
	(*pClientToScreen)(hWnd,&desk0);

	if(!desktop.right || !desktop.bottom) return win;

	win.left = (((win.left - desk0.x) * (LONG)dwScreenWidth) + (desktop.right >> 1)) / desktop.right;
	win.top = (((win.top - desk0.y) * (LONG)dwScreenHeight) + (desktop.bottom >> 1)) / desktop.bottom;
	win.right = (((win.right - desk0.x) * (LONG)dwScreenWidth) + (desktop.right >> 1)) / desktop.right;
	win.bottom = (((win.bottom - desk0.y) * (LONG)dwScreenHeight) + (desktop.bottom >> 1)) / desktop.bottom;

	return win;
}

RECT dxwCore::GetClientRect(RECT win)
{
	RECT desktop;
	(*pGetClientRect)(hWnd, &desktop);

	if(!desktop.right || !desktop.bottom) return win;

	win.left = ((win.left * dwScreenWidth) + (desktop.right >> 1)) / desktop.right;
	win.top = ((win.top * dwScreenHeight) + (desktop.bottom >> 1)) / desktop.bottom;
	win.right = ((win.right * dwScreenWidth) + (desktop.right >> 1)) / desktop.right;
	win.bottom = ((win.bottom * dwScreenHeight) + (desktop.bottom >> 1)) / desktop.bottom;

	return win;
}

POINT dxwCore::AddCoordinates(POINT p1, POINT p2)
{
	POINT ps;
	ps.x = p1.x + p2.x;
	ps.y = p1.y + p2.y;
	return ps;
}

RECT dxwCore::AddCoordinates(RECT r1, POINT p2)
{
	RECT rs;
	rs.left = r1.left + p2.x;
	rs.right = r1.right + p2.x;
	rs.top = r1.top + p2.y;
	rs.bottom = r1.bottom + p2.y;
	return rs;
}

POINT dxwCore::SubCoordinates(POINT p1, POINT p2)
{
	POINT ps;
	ps.x = p1.x - p2.x;
	ps.y = p1.y - p2.y;
	return ps;
}

void dxwCore::DumpPalette(DWORD dwcount, LPPALETTEENTRY lpentries)
{
	for(DWORD idx=0; idx<dwcount; idx++) 
		OutTrace("(%02x.%02x.%02x:%02x)", 
		lpentries[idx].peRed, lpentries[idx].peGreen, lpentries[idx].peBlue, lpentries[idx].peFlags);
	OutTrace("\n");
}

void dxwCore::ScreenRefresh(void)
{
	// optimization: don't blit too often!
	// 20mSec seems a good compromise.
	#define DXWREFRESHINTERVAL 20

	LPDIRECTDRAWSURFACE lpDDSPrim;
	extern HRESULT WINAPI extBlt(LPDIRECTDRAWSURFACE lpdds, LPRECT lpdestrect, LPDIRECTDRAWSURFACE lpddssrc, LPRECT lpsrcrect, DWORD dwflags, LPDDBLTFX lpddbltfx);

	static int t = -1;
	if (t == -1)
		t = (*pGetTickCount)()-(DXWREFRESHINTERVAL+1); // V.2.1.69: trick - subtract 
	int tn = (*pGetTickCount)();

	if (tn-t < DXWREFRESHINTERVAL) return;
	t = tn;

	// if not too early, refresh primary surface ....
	lpDDSPrim=dxw.GetPrimarySurface();
	if (lpDDSPrim) extBlt(lpDDSPrim, NULL, lpDDSPrim, NULL, 0, NULL);

	// v2.02.44 - used for what? Commenting out seems to fix the palette update glitches  
	// and make the "Palette updates don't blit"option useless....
	//(*pInvalidateRect)(hWnd, NULL, FALSE); 
}

void dxwCore::DoSlow(int delay)
{
	MSG uMsg;
	int t, tn;
	t = (*pGetTickCount)();

	uMsg.message=0; // initialize somehow...
	while((tn = (*pGetTickCount)())-t < delay){
		while (PeekMessage (&uMsg, NULL, 0, 0, PM_REMOVE) > 0){
			if(WM_QUIT == uMsg.message) break;
			TranslateMessage (&uMsg);
			DispatchMessage (&uMsg);
		}	
		(*pSleep)(1);
	}
}

static void CountFPS(HWND hwnd)
{
	static DWORD time = 0xFFFFFFFF;
	static DWORD FPSCount = 0;
	extern void SetFPS(int);
	//DXWNDSTATUS Status;
	DWORD tmp;
	tmp = (*pGetTickCount)();
	if((tmp - time) > 1000) {
		char sBuf[80+15+1]; // title + fps string + terminator
		char *fpss;
		// log fps count
		// OutTrace("FPS: Delta=%x FPSCount=%d\n", (tmp-time), FPSCount);
		// show fps count on status win
		GetHookInfo()->FPSCount = FPSCount; // for overlay display
		// show fps on win title bar
		if (dxw.dwFlags2 & SHOWFPS){
			GetWindowText(hwnd, sBuf, 80);
			fpss=strstr(sBuf," ~ (");
			if(fpss==NULL) fpss=&sBuf[strlen(sBuf)];
			sprintf_s(fpss, 15, " ~ (%d FPS)", FPSCount);
			SetWindowText(hwnd, sBuf);
		}
		// reset
		FPSCount=0;
		time = tmp;
	}
	else {
		FPSCount++;
		OutTraceB("FPS: Delta=%x FPSCount++=%d\n", (tmp-time), FPSCount);
	}
}

static void LimitFrameCount(DWORD delay)
{
	static DWORD oldtime=(*pGetTickCount)();
	DWORD newtime;
	newtime = (*pGetTickCount)();
	// use '<' and not '<=' to avoid the risk of sleeping forever....
	if(newtime < oldtime+delay) {
		if(IsDebug) OutTrace("FPS limit: old=%x new=%x delay=%d sleep=%d\n", 
			oldtime, newtime, delay, (oldtime+delay-newtime));
		(*pSleep)(oldtime+delay-newtime);
		// no good processing messages in the meanwhile: AoE series won't work at all!
		// don't use DoSlow(oldtime+delay-newtime); 
		oldtime += delay; // same as doing "oldtime=(*pGetTickCount)();" now
	}
	else
		oldtime = newtime;
}

static BOOL SkipFrameCount(DWORD delay)
{
	static DWORD oldtime=(*pGetTickCount)();
	DWORD newtime;
	newtime = (*pGetTickCount)();
	if(newtime < oldtime+delay) return TRUE; // TRUE => skip the screen refresh
	oldtime = newtime;
	return FALSE; // don't skip, do the update

}

BOOL dxwCore::HandleFPS()
{
	if(dwFlags2 & (SHOWFPS|SHOWFPSOVERLAY)) CountFPS(hWndFPS);
	if(dwFlags2 & LIMITFPS) LimitFrameCount(dxw.MaxFPS);
	if(dwFlags2 & SKIPFPS) if(SkipFrameCount(dxw.MaxFPS)) return TRUE;
	return FALSE;
}

// auxiliary functions ...

void dxwCore::SetVSyncDelays(UINT RefreshRate)
{
	int Reminder;

	if((RefreshRate < 10) || (RefreshRate > 100)) return; 

	gdwRefreshRate = RefreshRate;
	if(!gdwRefreshRate) return;
	iRefreshDelayCount=0;
	Reminder=0;
	do{
		iRefreshDelays[iRefreshDelayCount]=(1000+Reminder)/gdwRefreshRate;
		Reminder=(1000+Reminder)-(iRefreshDelays[iRefreshDelayCount]*gdwRefreshRate);
		iRefreshDelayCount++;
	} while(Reminder && (iRefreshDelayCount<MAXREFRESHDELAYCOUNT));
	OutTraceDW("Refresh rate=%d: delay=", gdwRefreshRate);
	for(int i=0; i<iRefreshDelayCount; i++) OutTraceDW("%d ", iRefreshDelays[i]);
	OutTraceDW("\n");
}

void dxwCore::VSyncWait()
{
	static DWORD time = 0;
	static BOOL step = 0;
	DWORD tmp;
	tmp = (*pGetTickCount)();
	if((time - tmp) > 32) time = tmp;
	(*pSleep)(time - tmp);
	time += iRefreshDelays[step++];
	if(step >= iRefreshDelayCount) step=0;
}


static float fMul[17]={2.14F, 1.95F, 1.77F, 1.61F, 1.46F, 1.33F, 1.21F, 1.10F, 1.00F, 0.91F, 0.83F, 0.75F, 0.68F, 0.62F, 0.56F, 0.51F, 0.46F};
//static float fMul[17]={0.46F, 0.51F, 0.56F, 0.62F, 0.68F, 0.75F, 0.83F, 0.91F, 1.00F, 1.10F, 1.21F, 1.33F, 1.46F, 1.61F, 1.77F, 1.95F, 2.14F};

static DWORD TimeShifterFine(DWORD val, int shift)
{
	float fVal;
	fVal = (float)val * fMul[shift+8];
	return (DWORD)fVal;
}

static DWORD TimeShifterCoarse(DWORD val, int shift)
{
	int exp, reminder;
	if (shift > 0) {
		exp = shift >> 1;
		reminder = shift & 0x1;
		if (reminder) val -= (val >> 2); // val * (1-1/4) = val * 3/4
		val >>= exp; // val * 2^exp
	}
	if (shift < 0) {
		exp = (-shift) >> 1;
		reminder = (-shift) & 0x1;
		val <<= exp; // val / 2^exp
		if (reminder) val += (val >> 1); // val * (1+1/2) = val * 3/2
	}
	return val;
}

static LARGE_INTEGER TimeShifter64Fine(LARGE_INTEGER val, int shift)
{
	float fVal;
	fVal = (float)val.LowPart * fMul[shift+8];
	val.HighPart = 0;
	val.LowPart = (DWORD)fVal;
	return val;	
}

static LARGE_INTEGER TimeShifter64Coarse(LARGE_INTEGER val, int shift)
{
	int exp, reminder;
	if (shift > 0) {
		exp = shift >> 1;
		reminder = shift & 0x1;
		if (reminder) val.QuadPart -= (val.QuadPart >> 2); // val * (1-1/4) = val * 3/4
		val.QuadPart >>= exp; // val * 2^exp
	}
	if (shift < 0) {
		exp = (-shift) >> 1;
		reminder = (-shift) & 0x1;
		val.QuadPart <<= exp; // val / 2^exp
		if (reminder) val.QuadPart += (val.QuadPart >> 1); // val * (1+1/2) = val * 3/2
	}
	return val;
}

DWORD dxwCore::GetTickCount(void)
{
	DWORD dwTick;
	static DWORD dwLastRealTick=0;
	static DWORD dwLastFakeTick=0;
	DWORD dwNextRealTick;
	static BOOL FirstTime = TRUE;

	if(FirstTime){
		dwLastRealTick=(*pGetTickCount)();
		dwLastFakeTick=dwLastRealTick;
		FirstTime=FALSE;
	}
	dwNextRealTick=(*pGetTickCount)();
	dwTick=(dwNextRealTick-dwLastRealTick);
	TimeShift=GetHookInfo()->TimeShift;
	dwTick = (*pTimeShifter)(dwTick, TimeShift);
	dwLastFakeTick += dwTick;
	dwLastRealTick = dwNextRealTick;
	return dwLastFakeTick;
}

DWORD dxwCore::StretchTime(DWORD dwTimer)
{
	TimeShift=GetHookInfo()->TimeShift;
	dwTimer = (*pTimeShifter)(dwTimer, -TimeShift);
	return dwTimer;
}

DWORD dxwCore::StretchCounter(DWORD dwTimer)
{
	TimeShift=GetHookInfo()->TimeShift;
	dwTimer = (*pTimeShifter)(dwTimer, TimeShift);
	return dwTimer;
}

LARGE_INTEGER dxwCore::StretchCounter(LARGE_INTEGER dwTimer)
{
	static int Reminder = 0;
	LARGE_INTEGER ret;
	TimeShift=GetHookInfo()->TimeShift;
	dwTimer.QuadPart += Reminder;
	ret = (*pTimeShifter64)(dwTimer, TimeShift);
	Reminder = (ret.QuadPart==0) ? dwTimer.LowPart : 0;
	return ret;
}

void dxwCore::GetSystemTimeAsFileTime(LPFILETIME lpSystemTimeAsFileTime)
{
	DWORD dwTick;
	DWORD dwCurrentTick;
	FILETIME CurrFileTime;
	static DWORD dwStartTick=0;
	static DWORD dwUpdateTick=0;
	static FILETIME StartFileTime;
//	extern DXWNDSTATUS *pStatus;

	if(dwStartTick==0) {
		SYSTEMTIME StartingTime;
		// first time through, initialize & return true time
		dwStartTick = (*pGetTickCount)();
		(*pGetSystemTime)(&StartingTime);
		SystemTimeToFileTime(&StartingTime, &StartFileTime);
		*lpSystemTimeAsFileTime = StartFileTime;
	}
	else {
		dwCurrentTick=(*pGetTickCount)();
		dwTick=(dwCurrentTick-dwStartTick);
		TimeShift=GetHookInfo()->TimeShift;
		dwTick = (*pTimeShifter)(dwTick, TimeShift);
		// From MSDN: Contains a 64-bit value representing the number of 
		// 100-nanosecond intervals since January 1, 1601 (UTC).
		// So, since 1mSec = 10.000 * 100nSec, you still have to multiply by 10.000.
		CurrFileTime.dwHighDateTime = StartFileTime.dwHighDateTime; // wrong !!!!
		CurrFileTime.dwLowDateTime = StartFileTime.dwLowDateTime + (10000 * dwTick); // wrong !!!!
		*lpSystemTimeAsFileTime=CurrFileTime;
		// reset to avoid time jumps on TimeShift changes...
		StartFileTime = CurrFileTime;
		dwStartTick = dwCurrentTick;
	}
}

void dxwCore::GetSystemTime(LPSYSTEMTIME lpSystemTime)
{
	DWORD dwTick;
	DWORD dwCurrentTick;
	FILETIME CurrFileTime;
	static DWORD dwStartTick=0;
	static DWORD dwUpdateTick=0;
	static FILETIME StartFileTime;
//	extern DXWNDSTATUS *pStatus;

	if(dwStartTick==0) {
		SYSTEMTIME StartingTime;
		// first time through, initialize & return true time
		dwStartTick = (*pGetTickCount)();
		(*pGetSystemTime)(&StartingTime);
		SystemTimeToFileTime(&StartingTime, &StartFileTime);
		*lpSystemTime = StartingTime;
	}
	else {
		dwCurrentTick=(*pGetTickCount)();
		dwTick=(dwCurrentTick-dwStartTick);
		TimeShift=GetHookInfo()->TimeShift;
		dwTick = (*pTimeShifter)(dwTick, TimeShift);
		// From MSDN: Contains a 64-bit value representing the number of 
		// 100-nanosecond intervals since January 1, 1601 (UTC).
		// So, since 1mSec = 10.000 * 100nSec, you still have to multiply by 10.000.
		CurrFileTime.dwHighDateTime = StartFileTime.dwHighDateTime; // wrong !!!!
		CurrFileTime.dwLowDateTime = StartFileTime.dwLowDateTime + (10000 * dwTick); // wrong !!!!
		FileTimeToSystemTime(&CurrFileTime, lpSystemTime);
		// reset to avoid time jumps on TimeShift changes...
		StartFileTime = CurrFileTime;
		dwStartTick = dwCurrentTick;
	}
}

void dxwCore::ShowOverlay()
{
	if (MustShowOverlay) {
		RECT rect;
		(*pGetClientRect)(hWnd, &rect);
		this->ShowOverlay(GetDC(hWnd), rect.right, rect.bottom);
	}
}

void dxwCore::ShowOverlay(LPDIRECTDRAWSURFACE lpdds)
{
	if (MustShowOverlay) {
		HDC hdc; // the working dc
		int h, w;
		if(!lpdds) return;
		if (FAILED(lpdds->GetDC(&hdc))) return;
		w = this->GetScreenWidth();
		h = this->GetScreenHeight();
		if(this->dwFlags4 & BILINEAR2XFILTER) {
			w <<=1;
			h <<=1;
		}
		this->ShowOverlay(hdc, w, h);
		lpdds->ReleaseDC(hdc);
	}
}

void dxwCore::ShowOverlay(HDC hdc)
{
	if(!hdc) return;
	RECT rect;
	(*pGetClientRect)(hWnd, &rect);
	this->ShowOverlay(GetDC(hWnd), rect.right, rect.bottom);
}

void dxwCore::ShowOverlay(HDC hdc, int w, int h)
{
	if(!hdc) return;
	if (dwFlags2 & SHOWFPSOVERLAY) ShowFPS(hdc, w, h);
	if (dwFlags4 & SHOWTIMESTRETCH) ShowTimeStretching(hdc, w, h);
}

// nasty global to ensure that the corner is picked semi-random, but never overlapped
// between FPS and TimeStretch (and, as a side effect, never twice the same!)
static int LastCorner;

void dxwCore::ShowFPS(HDC xdc, int w, int h)
{
	char sBuf[81];
	static DWORD dwTimer = 0;
	static int corner = 0;
	static int x, y;
	static DWORD color;

	if((*pGetTickCount)()-dwTimer > 6000){
		dwTimer = (*pGetTickCount)();
		corner = dwTimer % 4;
		if(corner==LastCorner) corner = (corner+1) % 4;
		LastCorner = corner;
		color=0xFF0000; // blue
		//(*pGetClientRect)(hWnd, &rect);
		//switch (corner) {
		//case 0: x=10; y=10; break;
		//case 1: x=rect.right-60; y=10; break;
		//case 2: x=rect.right-60; y=rect.bottom-20; break;
		//case 3: x=10; y=rect.bottom-20; break;
		//}
		switch (corner) {
		case 0: x=10; y=10; break;
		case 1: x=w-60; y=10; break;
		case 2: x=w-60; y=h-20; break;
		case 3: x=10; y=h-20; break;
		}
	} 

	SetTextColor(xdc,color);
	//SetBkMode(xdc, TRANSPARENT);
	SetBkMode(xdc, OPAQUE);
	sprintf_s(sBuf, 80, "FPS: %d", GetHookInfo()->FPSCount);
	TextOut(xdc, x, y, sBuf, strlen(sBuf));
}

void dxwCore::ShowTimeStretching(HDC xdc, int w, int h)
{
	char sBuf[81];
	static DWORD dwTimer = 0;
	static int corner = 0;
	static int x, y;
	static DWORD color;
	static int LastTimeShift = 1000; // any initial number different from -8 .. +8

	if((*pGetTickCount)()-dwTimer > 4000){
		if(LastTimeShift==TimeShift) return; // after a while, stop the show
		dwTimer = (*pGetTickCount)();
		LastTimeShift=TimeShift;
		corner = dwTimer % 4;
		if(corner==LastCorner) corner = (corner+1) % 4;
		LastCorner = corner;
		color=0x0000FF; // red
		//(*pGetClientRect)(hWnd, &rect);
		//switch (corner) {
		//case 0: x=10; y=10; break;
		//case 1: x=rect.right-60; y=10; break;
		//case 2: x=rect.right-60; y=rect.bottom-20; break;
		//case 3: x=10; y=rect.bottom-20; break;
		switch (corner) {
		case 0: x=10; y=10; break;
		case 1: x=w-60; y=10; break;
		case 2: x=w-60; y=h-20; break;
		case 3: x=10; y=h-20; break;
		}
	} 

	SetTextColor(xdc,color);
	//SetBkMode(xdc, TRANSPARENT);
	SetBkMode(xdc, OPAQUE);
	sprintf_s(sBuf, 80, "t%s", dxw.GetTSCaption());
	TextOut(xdc, x, y, sBuf, strlen(sBuf));
}

char *dxwCore::GetTSCaption(int shift)
{
	static char *sTSCaptionCoarse[17]={
		"x16","x12","x8","x6",
		"x4","x3","x2","x1.5",
		"x1",
		":1.5",":2",":3",":4",
		":6",":8",":12",":16"};
	static char *sTSCaptionFine[17]={
		"x2.14","x1.95","x1.77","x1.61",
		"x1.46","x1.33","x1.21","x1.10",
		"x1.00",
		":1.10",":1.21",":1.33",":1.46",
		":1.61",":1.77",":1.95",":2.14"};
	if (shift<(-8) || shift>(+8)) return "???";
	shift += 8;
	return (dxw.dwFlags4 & FINETIMING) ? sTSCaptionFine[shift] : sTSCaptionCoarse[shift];
}

char *dxwCore::GetTSCaption(void)
{
	return GetTSCaption(TimeShift);
}

void dxwCore::ShowBanner(HWND hwnd)
{
	static BOOL JustOnce=FALSE;
	extern HMODULE hInst;
	BITMAP bm;
	HDC hClientDC;
	HBITMAP g_hbmBall;
	RECT client;
	RECT win;
	POINT PrevViewPort;

	hClientDC=(*pGDIGetDC)(hwnd);
	(*pGetClientRect)(hwnd, &client);
	(*pGDIBitBlt)(hClientDC, 0, 0,  client.right, client.bottom, NULL, 0, 0, BLACKNESS);

	if(JustOnce || (dwFlags2 & NOBANNER)) return;
	JustOnce=TRUE;

    g_hbmBall = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_BANNER));
    HDC hdcMem = CreateCompatibleDC(hClientDC);
    HBITMAP hbmOld = (HBITMAP)SelectObject(hdcMem, g_hbmBall);
    GetObject(g_hbmBall, sizeof(bm), &bm);

	(*pGetWindowRect)(hwnd, &win);
	OutTraceDW("ShowBanner: hwnd=%x win=(%d,%d)-(%d,%d) banner size=(%dx%d)\n", 
		hwnd, win.left, win.top, win.right, win.bottom, bm.bmWidth, bm.bmHeight);

	//if(!pSetViewportOrgEx) pSetViewportOrgEx=SetViewportOrgEx;
	(*pSetViewportOrgEx)(hClientDC, 0, 0, &PrevViewPort);
	for (int i=1; i<=16; i++){
		int w, h;
		w=(bm.bmWidth*i)/8;
		h=(bm.bmHeight*i)/8;
		(*pGDIStretchBlt)(hClientDC, (client.right-w)/2, (client.bottom-h)/2, w, h, hdcMem, 0, 0, bm.bmWidth, bm.bmHeight, SRCCOPY);
		Sleep(40);
	}
	for (int i=16; i>=8; i--){
		int w, h;
		w=(bm.bmWidth*i)/8;
		h=(bm.bmHeight*i)/8;
		(*pGDIBitBlt)(hClientDC, 0, 0,  client.right, client.bottom, NULL, 0, 0, BLACKNESS);
		(*pGDIStretchBlt)(hClientDC, (client.right-w)/2, (client.bottom-h)/2, w, h, hdcMem, 0, 0, bm.bmWidth, bm.bmHeight, SRCCOPY);
		Sleep(40);
	}
	(*pSetViewportOrgEx)(hClientDC, PrevViewPort.x, PrevViewPort.y, NULL);
    SelectObject(hdcMem, hbmOld);
    DeleteDC(hdcMem);
	Sleep(200);
}

int dxwCore::GetDLLIndex(char *lpFileName)
{
	int idx;
	char *lpName, *lpNext;
	char *SysNames[]={
		"kernel32",
		"USER32",
		"GDI32",
		"imelib",
		"ADVAPI32",
		"ole32",
		"ddraw",
		"d3d8",
		"d3d9",
		"d3d10",
		"d3d10_1",
		"d3d11",
		"opengl32",
 		"msvfw32",
		"smackw32",
		"version",
		"dplayx",
		"dsound",
		"winmm",
		"imm32",
		"wsock32",
		"dinput",
		"dinput8",
		"shfolder",
		"shell32",
		"ws2_32",
		"tapi32",
		"netapi32",
		"wintrust",
		"d3dim",
		"d3dim700",
		"imagehlp",
//		"+glide",
//		"+glide2x",
//		"+glide3x",
		NULL
	};	
	
	lpName=lpFileName;
	while (lpNext=strchr(lpName,'\\')) lpName=lpNext+1;
	for(idx=0; SysNames[idx]; idx++){
		char SysNameExt[81];
		strcpy(SysNameExt, SysNames[idx]);
		strcat(SysNameExt, ".dll");
		if(
			(!lstrcmpi(lpName,SysNames[idx])) ||
			(!lstrcmpi(lpName,SysNameExt))
		){
			OutTraceDW("Registered DLL FileName=%s\n", lpFileName);
			break;
		}
	}
	if (!SysNames[idx]) return -1;
	return idx;
}

void dxwCore::FixWindowFrame(HWND hwnd)
{
	LONG nOldStyle;

	OutTraceDW("FixWindowFrame: hwnd=%x\n", hwnd);

	nOldStyle=(*pGetWindowLongA)(hwnd, GWL_STYLE);
	if (!nOldStyle){
		OutTraceE("FixWindowFrame: GetWindowLong ERROR %d at %d\n",GetLastError(),__LINE__);
		return;
	}

	OutTraceDW("FixWindowFrame: style=%x(%s)\n",nOldStyle,ExplainStyle(nOldStyle));

	// fix style
	if (!(*pSetWindowLongA)(hwnd, GWL_STYLE, WS_OVERLAPPEDWINDOW)){
		OutTraceE("FixWindowFrame: SetWindowLong ERROR %d at %d\n",GetLastError(),__LINE__);
		return;
	}
	// fix exstyle
	if (!(*pSetWindowLongA)(hwnd, GWL_EXSTYLE, 0)){
		OutTraceE("FixWindowFrame: SetWindowLong ERROR %d at %d\n",GetLastError(),__LINE__);
		return;
	}

	// ShowWindow retcode means in no way an error code! Better ignore it.
	(*pShowWindow)(hwnd, SW_RESTORE);
	return;
}

void dxwCore::FixStyle(char *ApiName, HWND hwnd, WPARAM wParam, LPARAM lParam)
{
	LPSTYLESTRUCT lpSS;
	lpSS = (LPSTYLESTRUCT) lParam;

	switch (wParam) {
	case GWL_STYLE:
		OutTraceDW("%s: new Style=%x(%s)\n", 
			ApiName, lpSS->styleNew, ExplainStyle(lpSS->styleNew));
		if (dxw.dwFlags1 & FIXWINFRAME){ // set canonical style
			lpSS->styleNew= WS_OVERLAPPEDWINDOW;
		}
		if (dxw.dwFlags1 & LOCKWINSTYLE){ // set to current value
			lpSS->styleNew= (*pGetWindowLongA)(hwnd, GWL_STYLE);
		}
		if (dxw.dwFlags1 & PREVENTMAXIMIZE){ // disable maximize settings
			if (lpSS->styleNew & WS_MAXIMIZE){
				OutTraceDW("%s: prevent maximize style\n", ApiName);
				lpSS->styleNew &= ~WS_MAXIMIZE;
			}
		}
		break;
	case GWL_EXSTYLE:
		OutTraceDW("%s: new ExStyle=%x(%s)\n", 
			ApiName, lpSS->styleNew, ExplainExStyle(lpSS->styleNew));
		if (dxw.dwFlags1 & FIXWINFRAME){ // set canonical style
			lpSS->styleNew= 0;
		}
		if (dxw.dwFlags1 & LOCKWINSTYLE){ // set to current value
				lpSS->styleNew= (*pGetWindowLongA)(hwnd, GWL_EXSTYLE);
		}
		if ((dxw.dwFlags1 & PREVENTMAXIMIZE) && (hwnd==hWnd)){ // disable maximize settings
			if (lpSS->styleNew & WS_EX_TOPMOST){
				OutTraceDW("%s: prevent EXSTYLE topmost style\n", ApiName);
				lpSS->styleNew &= ~WS_EX_TOPMOST;
			}
		}
		break;
	default:
		break;
	}
}

HDC dxwCore::AcquireEmulatedDC(HWND hwnd)
{
	HDC wdc;
	if(!(wdc=(*pGDIGetDC)(hwnd))){
		OutTraceE("GetDC: ERROR err=%d at=%d\n", GetLastError(), __LINE__);
		return NULL;
	}
	return AcquireEmulatedDC(wdc);
}

HDC dxwCore::AcquireEmulatedDC(LPDIRECTDRAWSURFACE lpdds)
{
	HDC ddc;
	typedef HRESULT (WINAPI *GetDC_Type) (LPDIRECTDRAWSURFACE, HDC FAR *);
	extern GetDC_Type pGetDC;
	if((*pGetDC)(lpdds, &ddc))
	OutTraceE("GetDC: ERROR err=%d at=%d\n", GetLastError(), __LINE__);
	return AcquireEmulatedDC(ddc);
}

HDC dxwCore::AcquireEmulatedDC(HDC wdc)
{
	RealHDC=wdc;
	HWND hwnd;
	RECT WinRect;
	if(!(hwnd=WindowFromDC(wdc)))
		OutTraceE("WindowFromDC: ERROR err=%d at=%d\n", GetLastError(), __LINE__);
	(*pGetClientRect)(hwnd, &WinRect);
	if(dxw.IsDesktop(hwnd)){
		dxw.VirtualPicRect = dxw.GetScreenRect();
	}
	else {
		VirtualPicRect = WinRect;
		dxw.UnmapClient(&VirtualPicRect);
	}
	if(!(VirtualHDC=CreateCompatibleDC(wdc)))
		OutTraceE("CreateCompatibleDC: ERROR err=%d at=%d\n", GetLastError(), __LINE__);
	if(!(VirtualPic=CreateCompatibleBitmap(wdc, VirtualPicRect.right, VirtualPicRect.bottom)))
		OutTraceE("CreateCompatibleBitmap: ERROR err=%d at=%d\n", GetLastError(), __LINE__);
	if(!SelectObject(VirtualHDC, VirtualPic)){
		if(!DeleteObject(VirtualPic))
			OutTraceE("DeleteObject: ERROR err=%d at=%d\n", GetLastError(), __LINE__);
		if(!(VirtualPic=CreateCompatibleBitmap(wdc, dxw.GetScreenWidth(), dxw.GetScreenHeight())))
			OutTraceE("CreateCompatibleBitmap: ERROR err=%d at=%d\n", GetLastError(), __LINE__);
		if(!SelectObject(VirtualHDC, VirtualPic))
			OutTraceE("SelectObject: ERROR err=%d at=%d\n", GetLastError(), __LINE__);
	}
	if(!(*pGDIStretchBlt)(VirtualHDC, 0, 0, VirtualPicRect.right, VirtualPicRect.bottom, wdc, 0, 0, WinRect.right, WinRect.bottom, SRCCOPY))
		OutTraceE("StretchBlt: ERROR err=%d at=%d\n", GetLastError(), __LINE__);
	return VirtualHDC;
}

BOOL dxwCore::ReleaseEmulatedDC(HWND hwnd)
{
	BOOL ret;
	HDC wdc;
	RECT client;
	(*pGetClientRect)(hwnd, &client);
	if(!(wdc=(*pGDIGetDC)(hwnd)))
		OutTraceE("GetDC: ERROR err=%d at=%d\n", GetLastError(), __LINE__);
	//OutTrace("StretchBlt: destdc=%x destrect=(0,0)-(%d,%d) srcdc=%x srcrect=(0,0)-(%d,%d)\n", wdc, client.right, client.bottom, VirtualHDC, VirtualPicRect.right, VirtualPicRect.bottom);
	// v2.02.94: set HALFTONE stretching. But causes problems with Imperialism II
	// SetStretchBltMode(wdc,HALFTONE);
	if(!(*pGDIStretchBlt)(wdc, 0, 0, client.right, client.bottom, VirtualHDC, 0, 0, VirtualPicRect.right, VirtualPicRect.bottom, SRCCOPY))
		OutTraceE("StretchBlt: ERROR err=%d at=%d\n", GetLastError(), __LINE__);
	//(*pInvalidateRect)(hwnd, NULL, 0);
	if(!DeleteObject(VirtualPic))
		OutTraceE("DeleteObject: ERROR err=%d at=%d\n", GetLastError(), __LINE__);	
	if(!DeleteObject(VirtualHDC))
		OutTraceE("DeleteObject: ERROR err=%d at=%d\n", GetLastError(), __LINE__);	
	ret=TRUE;
	return ret;
}

void dxwCore::ResetEmulatedDC()
{
	VirtualHDC=NULL;
	VirtualPic=NULL;
	VirtualOffsetX=0;
	VirtualOffsetY=0;
}

BOOL dxwCore::IsVirtual(HDC hdc)
{
	return (hdc==VirtualHDC) && (dwFlags3 & GDIEMULATEDC);
}

void dxwCore::PushTimer(UINT uTimerId, UINT uDelay, UINT uResolution, LPTIMECALLBACK lpTimeProc, DWORD_PTR dwUser, UINT fuEvent)
{
		// save current timer
		TimerEvent.dwTimerType = TIMER_TYPE_WINMM;
		TimerEvent.t.uTimerId = uTimerId;
		TimerEvent.t.uDelay = uDelay;
		TimerEvent.t.uResolution = uResolution;
		TimerEvent.t.lpTimeProc = lpTimeProc;
		TimerEvent.t.dwUser = dwUser;
		TimerEvent.t.fuEvent = fuEvent;
}

void dxwCore::PushTimer(HWND hWnd, UINT_PTR nIDEvent, UINT uElapse, TIMERPROC lpTimerFunc)
{
		// save current timer
		TimerEvent.dwTimerType = TIMER_TYPE_USER32;
		TimerEvent.t.hWnd = hWnd;
		TimerEvent.t.nIDEvent = nIDEvent;
		TimerEvent.t.uElapse = uElapse;
		TimerEvent.t.lpTimerFunc = lpTimerFunc;
}

void dxwCore::PopTimer(UINT uTimerId)
{
	// clear current timer
	if(TimerEvent.dwTimerType != TIMER_TYPE_WINMM) {
		// this should never happen, unless there are more than 1 timer!
		char msg[256];
		sprintf(msg,"PopTimer: TimerType=%x last=%x\n", TIMER_TYPE_WINMM, TimerEvent.dwTimerType);
		//MessageBox(0, msg, "PopTimer", MB_OK | MB_ICONEXCLAMATION);
		OutTraceE(msg);
		return;
	}
	if(uTimerId != TimerEvent.t.uTimerId){
		// this should never happen, unless there are more than 1 timer!
		char msg[256];
		sprintf(msg,"PopTimer: TimerId=%x last=%x\n", uTimerId, TimerEvent.t.uTimerId);
		//MessageBox(0, msg, "PopTimer", MB_OK | MB_ICONEXCLAMATION);
		OutTraceE(msg);
		return;
	}
	TimerEvent.dwTimerType = TIMER_TYPE_NONE;
}

void dxwCore::PopTimer(HWND hWnd, UINT_PTR nIDEvent)
{
	// clear current timer
	if(TimerEvent.dwTimerType != TIMER_TYPE_USER32) {
		// this should never happen, unless there are more than 1 timer!
		char msg[256];
		sprintf(msg,"PopTimer: TimerType=%x last=%x\n", TIMER_TYPE_WINMM, TimerEvent.dwTimerType);
		//MessageBox(0, msg, "PopTimer", MB_OK | MB_ICONEXCLAMATION);
		OutTraceE(msg);
		return;
	}
	if(nIDEvent != TimerEvent.t.nIDEvent){
		// this should never happen, unless there are more than 1 timer!
		char msg[256];
		sprintf(msg,"PopTimer: TimerId=%x last=%x\n", nIDEvent, TimerEvent.t.nIDEvent);
		//MessageBox(0, msg, "PopTimer", MB_OK | MB_ICONEXCLAMATION);
		OutTraceE(msg);
		return;
	}
	TimerEvent.dwTimerType = TIMER_TYPE_NONE;
}

void dxwCore::RenewTimers()
{
	OutTraceE("DXWND: RenewTimers type=%x\n", TimerEvent.dwTimerType);
	switch(TimerEvent.dwTimerType){
	case TIMER_TYPE_NONE:
		OutTraceDW("DXWND: RenewTimers type=NONE\n");
		break;
	case TIMER_TYPE_USER32:
		if(pSetTimer && pKillTimer){
			UINT uElapse;
			UINT_PTR res;
			res=(*pKillTimer)(TimerEvent.t.hWnd, TimerEvent.t.nIDEvent);
			if(!res) OutTraceE("DXWND: KillTimer ERROR hWnd=%x IDEvent=%x err=%d\n", TimerEvent.t.hWnd, TimerEvent.t.nIDEvent, GetLastError());
			uElapse = dxw.StretchTime(TimerEvent.t.uElapse);
			res=(*pSetTimer)(TimerEvent.t.hWnd, TimerEvent.t.nIDEvent, uElapse, TimerEvent.t.lpTimerFunc);
			TimerEvent.t.nIDEvent = res;
			if(res)
			OutTraceDW("DXWND: RenewTimers type=USER32 Elsapse=%d IDEvent=%x\n", uElapse, res);
			else
				OutTraceE("DXWND: SetTimer ERROR hWnd=%x Elapse=%d TimerFunc=%x err=%d\n", TimerEvent.t.hWnd, uElapse, TimerEvent.t.lpTimerFunc, GetLastError());
		}
		break;
	case TIMER_TYPE_WINMM:
		if(ptimeKillEvent && ptimeSetEvent){
			UINT NewDelay;
			MMRESULT res;
			(*ptimeKillEvent)(TimerEvent.t.uTimerId);
			NewDelay = dxw.StretchTime(TimerEvent.t.uDelay);
			res=(*ptimeSetEvent)(NewDelay, TimerEvent.t.uResolution, TimerEvent.t.lpTimeProc, TimerEvent.t.dwUser, TimerEvent.t.fuEvent);
			TimerEvent.t.uTimerId = res;
			OutTraceDW("DXWND: RenewTimers type=WINMM Delay=%d TimerId=%x\n", NewDelay, res);
		}
		break;
	default:
		OutTraceE("DXWND: RenewTimers type=%x(UNKNOWN)\n", TimerEvent.dwTimerType);
		break;
	}
}

LARGE_INTEGER dxwCore::StretchLargeCounter(LARGE_INTEGER CurrentInCount)
{
	LARGE_INTEGER CurrentOutPerfCount;
	static LARGE_INTEGER LastInPerfCount;
	static LARGE_INTEGER LastOutPerfCount;
	static BOOL FirstTime = TRUE;
	LARGE_INTEGER ElapsedCount;

	if(FirstTime){
		// first time through, initialize both inner and output per counters with real values
		LastInPerfCount.QuadPart = CurrentInCount.QuadPart;
		LastOutPerfCount.QuadPart = CurrentInCount.QuadPart;
		FirstTime=FALSE;
	}

	ElapsedCount.QuadPart = CurrentInCount.QuadPart - LastInPerfCount.QuadPart;
	ElapsedCount = dxw.StretchCounter(ElapsedCount);
	CurrentOutPerfCount.QuadPart = LastOutPerfCount.QuadPart + ElapsedCount.QuadPart;
	LastInPerfCount = CurrentInCount;
	LastOutPerfCount = CurrentOutPerfCount;

	OutTraceB("dxw::StretchCounter: Count=[%x-%x]\n", CurrentOutPerfCount.HighPart, CurrentOutPerfCount.LowPart);
	return CurrentOutPerfCount;
}

BOOL dxwCore::CheckScreenResolution(unsigned int w, unsigned int h)
{
	#define HUGE 100000
	if(dxw.dwFlags4 & LIMITSCREENRES){
		DWORD maxw, maxh;
		switch(MaxScreenRes){
			case DXW_NO_LIMIT: maxw=HUGE; maxh=HUGE; break;
			case DXW_LIMIT_320x200: maxw=320; maxh=200; break;
			case DXW_LIMIT_640x480: maxw=640; maxh=480; break;
			case DXW_LIMIT_800x600: maxw=800; maxh=600; break;
			case DXW_LIMIT_1024x768: maxw=1024; maxh=768; break;
			case DXW_LIMIT_1280x960: maxw=1280; maxh=960; break;
		}
		if((w > maxw) || (h > maxh)) return FALSE;
	}
	return TRUE;
}

UINT VKeyConfig[DXVK_SIZE];

static char *VKeyLabels[DXVK_SIZE]={
	"none",
	"cliptoggle", 
	"refresh", 
	"logtoggle", 
	"plocktoggle",
	"fpstoggle", 
	"timefast", 
	"timeslow", 
	"timetoggle", 
	"altf4"
};

void dxwCore::MapKeysInit()
{
	char InitPath[MAX_PATH];
	char *p;
	DWORD dwAttrib;	
	int KeyIdx;
	dwAttrib = GetFileAttributes("dxwnd.dll");
	if (dwAttrib != INVALID_FILE_ATTRIBUTES && !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY)) return;
	GetModuleFileName(GetModuleHandle("dxwnd"), InitPath, MAX_PATH);
	p=&InitPath[strlen(InitPath)-strlen("dxwnd.dll")];
	strcpy(p, "dxwnd.ini");
	VKeyConfig[DXVK_NONE]=DXVK_NONE;
	for(KeyIdx=1; KeyIdx<DXVK_SIZE; KeyIdx++){
		VKeyConfig[KeyIdx]=GetPrivateProfileInt("keymapping", VKeyLabels[KeyIdx], KeyIdx==DXVK_ALTF4 ? 0x73 : 0x00, InitPath);
		OutTrace("keymapping[%d](%s)=%x\n", KeyIdx, VKeyLabels[KeyIdx], VKeyConfig[KeyIdx]);
	}
}

UINT dxwCore::MapKeysConfig(UINT message, LPARAM lparam, WPARAM wparam)
{
	if(message!=WM_SYSKEYDOWN) return DXVK_NONE;
	for(int idx=1; idx<DXVK_SIZE; idx++)
		if(VKeyConfig[idx]==wparam) {
			OutTrace("keymapping GOT=%d\n", idx);
			return idx;
		}
	return DXVK_NONE;
}
