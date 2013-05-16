#include <stdio.h>
#include "dxwnd.h"
#include "dxwcore.hpp"
#include "syslibs.h"
#include "resource.h"

/* ------------------------------------------------------------------ */
// Constructor, destructor, initialization....
/* ------------------------------------------------------------------ */

dxwCore::dxwCore()
{
	// initialization stuff ....
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
	//IsWithinDDraw = FALSE;
	IsGDIPalette = FALSE;
	memset(PrimSurfaces, 0, sizeof(PrimSurfaces));
}

dxwCore::~dxwCore()
{
}

void dxwCore::InitTarget(TARGETMAP *target)
{
	dwFlags1 = target->flags;
	dwFlags2 = target->flags2;
	dwFlags3 = target->flags3;
	dwFlags4 = target->flags4;
	dwTFlags = target->tflags;
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
}

/* ------------------------------------------------------------------ */
// Primary surfaces auxiliary functions
/* ------------------------------------------------------------------ */

void dxwCore::MarkPrimarySurface(LPDIRECTDRAWSURFACE ps)
{
	int i;
	// OutTraceD("PRIMARYSURFACE add %x\n",ps);
	for (i=0;i<DDSQLEN;i++) {
		if (PrimSurfaces[i]==(DWORD)ps) return; // if already there ....
		if (PrimSurfaces[i]==(DWORD)0) break; // got end of list
	}
	PrimSurfaces[i]=(DWORD)ps;
}

// Note: since MS itself declares that the object refcount is not reliable and should
// be used for debugging only, it's not safe to rely on refcount==0 when releasing a
// surface to terminate its classification as primary. As an alternate and more reliable
// way, we use UnmarkPrimarySurface each time you create a new not primary surface, 
// eliminating the risk that a surface previously classified as primary and then closed
// had the same surface handle than this new one that is not primary at all.

void dxwCore::UnmarkPrimarySurface(LPDIRECTDRAWSURFACE ps)
{
	int i;
	// OutTraceD("PRIMARYSURFACE del %x\n",ps);
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

BOOL dxwCore::IsAPrimarySurface(LPDIRECTDRAWSURFACE ps)
{
	int i;
	// treat NULL surface ptr as a non primary
	if(!ps) return FALSE;
	for (i=0;i<DDSQLEN;i++) {
		if (PrimSurfaces[i]==(DWORD)ps) return TRUE;
		if (PrimSurfaces[i]==0) return FALSE;
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

// v2.1.93: FixCursorPos completely revised to introduce a clipping tolerance in
// clipping regions as well as in normal operations

#define CLIP_TOLERANCE 4

POINT dxwCore::FixCursorPos(POINT prev)
{
	return FixCursorPos(hWnd, prev);
}

POINT dxwCore::FixCursorPos(HWND hwnd, POINT prev)
{
	POINT curr;
	RECT rect;
	extern LPRECT lpClipRegion;

	curr=prev;

	// scale mouse coordinates
	// remember: rect from GetClientRect always start at 0,0!
	if(dxw.dwFlags1 & MODIFYMOUSE){
		if (!(*pGetClientRect)(hwnd, &rect)) {
			OutTraceD("GetClientRect ERROR %d at %d\n", GetLastError(),__LINE__);
			curr.x = curr.y = 0;
		}

		if (curr.x < 0) curr.x = 0;
		if (curr.y < 0) curr.y = 0;
		if (curr.x > rect.right) curr.x = rect.right;
		if (curr.y > rect.bottom) curr.y = rect.bottom;

		if (rect.right)  curr.x = (curr.x * dxw.GetScreenWidth()) / rect.right;
		if (rect.bottom) curr.y = (curr.y * dxw.GetScreenHeight()) / rect.bottom;
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
	POINT UpLeftCorner;

	OutTraceD("SetClipCursor:\n");
	if (hWnd==NULL) {
		OutTraceD("SetClipCursor: ASSERT hWnd==NULL\n");
		return;
	}
	(*pGetClientRect)(hWnd, &Rect);
	UpLeftCorner.x=UpLeftCorner.y=0;
	(*pClientToScreen)(hWnd, &UpLeftCorner);
	Rect.left+=UpLeftCorner.x;
	Rect.right+=UpLeftCorner.x;
	Rect.top+=UpLeftCorner.y;
	Rect.bottom+=UpLeftCorner.y;
	(*pClipCursor)(NULL);
	if(!(*pClipCursor)(&Rect)){
		OutTraceE("ClipCursor: ERROR err=%d at %d\n", GetLastError(), __LINE__);
	}
	OutTraceD("SetClipCursor: rect=(%d,%d)-(%d,%d)\n",
		Rect.left, Rect.top, Rect.right, Rect.bottom);
}

void dxwCore::EraseClipCursor()
{
	OutTraceD("EraseClipCursor:\n");
	(*pClipCursor)(NULL);
}

RECT dxwCore::MapWindowRect(LPRECT lpRect)
{
	POINT UpLeft={0,0};
	RECT RetRect;
	RECT ClientRect;
	if (!(*pGetClientRect)(hWnd, &ClientRect)){
		OutTraceE("GetClientRect ERROR: err=%d hwnd=%x at %d\n", GetLastError(), hWnd, __LINE__);
	}
	if(lpRect){
		RetRect.left = lpRect->left * ClientRect.right / dwScreenWidth;
		RetRect.right = lpRect->right * ClientRect.right / dwScreenWidth;
		RetRect.top = lpRect->top * ClientRect.bottom / dwScreenHeight;
		RetRect.bottom = lpRect->bottom * ClientRect.bottom / dwScreenHeight;
	}
	else {
		RetRect=ClientRect;
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
	(*pGetClientRect)(hWnd, &client);
	rect->left= rect->left * client.right / dwScreenWidth;
	rect->top= rect->top * client.bottom / dwScreenHeight;
	rect->right= rect->right * client.right / dwScreenWidth;
	rect->bottom= rect->bottom * client.bottom / dwScreenHeight;
}

void dxwCore::MapClient(int *nXDest, int *nYDest, int *nWDest, int *nHDest)
{
	RECT client;
	(*pGetClientRect)(hWnd, &client);
	*nXDest= *nXDest * client.right / dwScreenWidth;
	*nYDest= *nYDest * client.bottom / dwScreenHeight;
	*nWDest= *nWDest * client.right / dwScreenWidth;
	*nHDest= *nHDest * client.bottom / dwScreenHeight;
}

void dxwCore::MapClient(LPPOINT lppoint)
{
	RECT client;
	(*pGetClientRect)(hWnd, &client);
	lppoint->x = (lppoint->x * client.right) / dwScreenWidth;
	lppoint->y = (lppoint->y * client.bottom) / dwScreenHeight;
}

void dxwCore::MapWindow(LPRECT rect)
{
	RECT client;
	POINT upleft = {0,0};
	(*pGetClientRect)(hWnd, &client);
	(*pClientToScreen)(hWnd, &upleft);
	rect->left= upleft.x + ((rect->left * client.right) / dwScreenWidth);
	rect->top= upleft.y + ((rect->top * client.bottom) / dwScreenHeight);
	rect->right= upleft.x + ((rect->right * client.right) / dwScreenWidth);
	rect->bottom= upleft.y + ((rect->bottom * client.bottom) / dwScreenHeight);
}

void dxwCore::MapWindow(int *nXDest, int *nYDest, int *nWDest, int *nHDest)
{
	RECT client;
	POINT upleft = {0,0};
	(*pGetClientRect)(hWnd, &client);
	(*pClientToScreen)(hWnd, &upleft);
	*nXDest= upleft.x + ((*nXDest * client.right) / dwScreenWidth);
	*nYDest= upleft.y + ((*nYDest * client.bottom) / dwScreenHeight);
	*nWDest= (*nWDest * client.right) / dwScreenWidth;
	*nHDest= (*nHDest * client.bottom) / dwScreenHeight;
}

void dxwCore::MapWindow(LPPOINT lppoint)
{
	RECT client;
	POINT upleft = {0,0};
	(*pGetClientRect)(hWnd, &client);
	(*pClientToScreen)(hWnd, &upleft);
	lppoint->x = upleft.x + ((lppoint->x * client.right) / dwScreenWidth);
	lppoint->y = upleft.y + ((lppoint->y * client.bottom) / dwScreenHeight);
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
	if (desktop.right) ret.x = ((win0.x - desk0.x) * (LONG)dwScreenWidth) / desktop.right;
	if (desktop.bottom) ret.y = ((win0.y - desk0.y) * (LONG)dwScreenHeight) / desktop.bottom;
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

	win.left = ((win.left - desk0.x) * (LONG)dwScreenWidth) / desktop.right;
	win.top = ((win.top - desk0.y) * (LONG)dwScreenHeight) / desktop.bottom;
	win.right = ((win.right - desk0.x) * (LONG)dwScreenWidth) / desktop.right;
	win.bottom = ((win.bottom - desk0.y) * (LONG)dwScreenWidth) / desktop.right;

	return win;
}

RECT dxwCore::GetClientRect(RECT win)
{
	RECT desktop;
	(*pGetClientRect)(hWnd, &desktop);

	if(!desktop.right || !desktop.bottom) return win;

	win.left = (win.left * dwScreenWidth) / desktop.right;
	win.top = (win.top * dwScreenHeight) / desktop.bottom;
	win.right = (win.right * dwScreenWidth) / desktop.right;
	win.bottom = (win.bottom * dwScreenHeight) / desktop.bottom;

	return win;
}

POINT dxwCore::AddCoordinates(POINT p1, POINT p2)
{
	POINT ps;
	ps.x = p1.x + p2.x;
	ps.y = p1.y + p2.y;
	return ps;
}

POINT dxwCore::SubCoordinates(POINT p1, POINT p2)
{
	POINT ps;
	ps.x = p1.x - p2.x;
	ps.y = p1.y - p2.y;
	return ps;
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

	lpDDSPrim=dxw.GetPrimarySurface();
	// if too early ....
	if (lpDDSPrim)
		extBlt(lpDDSPrim, NULL, lpDDSPrim, NULL, 0, NULL);

	(*pInvalidateRect)(hWnd, NULL, FALSE);
	t = tn;
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
		OutTrace("FPS: Delta=%x FPSCount=%d\n", (tmp-time), FPSCount);
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
	if(dwFlags2 & (SHOWFPS|SHOWFPSOVERLAY)) CountFPS(hWnd);
	if(dwFlags2 & LIMITFPS) LimitFrameCount(dxw.MaxFPS);
	if(dwFlags2 & SKIPFPS) if(SkipFrameCount(dxw.MaxFPS)) return TRUE;
	return FALSE;
}

static DWORD TimeShifter(DWORD val, int shift)
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

DWORD dxwCore::GetTickCount(void)
{
	DWORD dwTick;
	static DWORD dwLastRealTick=0;
	static DWORD dwLastFakeTick=0;
	DWORD dwNextRealTick;

	dwNextRealTick=(*pGetTickCount)();
	dwTick=(dwNextRealTick-dwLastRealTick);
	TimeShift=GetHookInfo()->TimeShift;
	dwTick = TimeShifter(dwTick, TimeShift);
	dwLastFakeTick += dwTick;
	dwLastRealTick = dwNextRealTick;
	return dwLastFakeTick;
}

DWORD dxwCore::StretchTime(DWORD dwTimer)
{
	TimeShift=GetHookInfo()->TimeShift;
	dwTimer = TimeShifter(dwTimer, -TimeShift);
	return dwTimer;
}

void dxwCore::GetSystemTimeAsFileTime(LPFILETIME lpSystemTimeAsFileTime)
{
	DWORD dwTick;
	DWORD dwCurrentTick;
	FILETIME CurrFileTime;
	static DWORD dwStartTick=0;
	static DWORD dwUpdateTick=0;
	static FILETIME StartFileTime;
	extern DXWNDSTATUS *pStatus;

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
		dwTick = TimeShifter(dwTick, TimeShift);
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
	extern DXWNDSTATUS *pStatus;

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
		dwTick = TimeShifter(dwTick, TimeShift);
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

void dxwCore::ShowFPS(HDC xdc)
{
	char sBuf[81];
	static DWORD dwTimer = 0;
	static int corner = 0;
	static int x, y;
	static DWORD color;

	if((*pGetTickCount)()-dwTimer > 4000){
		RECT rect;
		dwTimer = (*pGetTickCount)();
		corner = dwTimer % 4;
		color=0xFF0000; // blue
		(*pGetClientRect)(hWnd, &rect);
		switch (corner) {
		case 0: x=10; y=10; break;
		case 1: x=rect.right-60; y=10; break;
		case 2: x=rect.right-60; y=rect.bottom-20; break;
		case 3: x=10; y=rect.bottom-20; break;
		}
	} 

	SetTextColor(xdc,color);
	//SetBkMode(xdc, TRANSPARENT);
	SetBkMode(xdc, OPAQUE);
	sprintf_s(sBuf, 80, "FPS: %d", GetHookInfo()->FPSCount);
	TextOut(xdc, x, y, sBuf, strlen(sBuf));
}

void dxwCore::ShowFPS(LPDIRECTDRAWSURFACE lpdds)
{
	HDC xdc; // the working dc
	char sBuf[81];
	static DWORD dwTimer = 0;
	static int corner = 0;
	static int x, y;
	static DWORD color;

	if((*pGetTickCount)()-dwTimer > 4000){
		dwTimer = (*pGetTickCount)();
		corner = dwTimer % 4;
		color=0xFF0000; // blue
		switch (corner) {
		case 0: x=10; y=10; break;
		case 1: x=dwScreenWidth-60; y=10; break;
		case 2: x=dwScreenWidth-60; y=dwScreenHeight-20; break;
		case 3: x=10; y=dwScreenHeight-20; break;
		}
	}

	if (FAILED(lpdds->GetDC(&xdc))) return;
	SetTextColor(xdc,color);
	//SetBkMode(xdc, TRANSPARENT);
	SetBkMode(xdc, OPAQUE);
	sprintf_s(sBuf, 80, "FPS: %d", GetHookInfo()->FPSCount);
	TextOut(xdc, x, y, sBuf, strlen(sBuf));
	lpdds->ReleaseDC(xdc);
}

char *dxwCore::GetTSCaption(int shift)
{
	static char *sTSCaption[17]={
		"x16","x12","x8","x6",
		"x4","x3","x2","x1.5",
		"x1",
		":1.5",":2",":3",":4",
		":6",":8",":12",":16"};
	if (shift<0 || shift>16) return "???";
	return sTSCaption[shift+8];
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

	if(JustOnce || (dwFlags2 & NOBANNER)) return;
	JustOnce=TRUE;

	hClientDC=GetDC(hwnd);
	(*pGetClientRect)(hwnd, &client);
	//BitBlt(hClientDC, 0, 0,  wp.cx, wp.cy, NULL, 0, 0, BLACKNESS);
	BitBlt(hClientDC, 0, 0,  client.right, client.bottom, NULL, 0, 0, BLACKNESS);
    g_hbmBall = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_BANNER));
    HDC hdcMem = CreateCompatibleDC(hClientDC);
    HBITMAP hbmOld = (HBITMAP)SelectObject(hdcMem, g_hbmBall);
    GetObject(g_hbmBall, sizeof(bm), &bm);
	for (int i=1; i<=16; i++){
		int w, h;
		w=(bm.bmWidth*i)/8;
		h=(bm.bmHeight*i)/8;
		StretchBlt(hClientDC, (client.right-w)/2, (client.bottom-h)/2, w, h, hdcMem, 0, 0, bm.bmWidth, bm.bmHeight, SRCCOPY);
		Sleep(40);
	}
	for (int i=16; i>=8; i--){
		int w, h;
		w=(bm.bmWidth*i)/8;
		h=(bm.bmHeight*i)/8;
		BitBlt(hClientDC, 0, 0,  client.right, client.bottom, NULL, 0, 0, BLACKNESS);
		StretchBlt(hClientDC, (client.right-w)/2, (client.bottom-h)/2, w, h, hdcMem, 0, 0, bm.bmWidth, bm.bmHeight, SRCCOPY);
		Sleep(40);
	}
    SelectObject(hdcMem, hbmOld);
    DeleteDC(hdcMem);
	Sleep(200);
}