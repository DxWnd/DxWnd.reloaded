#include "dxwnd.h"
#include "dxwcore.hpp"
#include "syslibs.h"

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

	// preserved syslibs pointers
	pClientToScreen=ClientToScreen;
	pClipCursor=ClipCursor;
	pGetClientRect=GetClientRect;
	pGetCursorPos=GetCursorPos;
	pInvalidateRect=InvalidateRect;
	pScreenToClient=ScreenToClient;
}

dxwCore::~dxwCore()
{
}

void dxwCore::InitTarget(TARGETMAP *target)
{
	dwFlags1 = target->flags;
	dwFlags2 = target->flags2;
	dwTFlags = target->tflags;
	gsModules = target->module;
	MaxFPS = target->MaxFPS;
	CustomOpenGLLib = target->OpenGLLib;
	if(!strlen(CustomOpenGLLib)) CustomOpenGLLib=NULL;
	// bounds control
	dwTargetDDVersion = target->dxversion;
	if(dwTargetDDVersion<0) dwTargetDDVersion=0;
	if(dwTargetDDVersion>10) dwTargetDDVersion=10;
	TimeShift = target->InitTS;
	if(TimeShift < -4) TimeShift = -4;
	if(TimeShift >  4) TimeShift =  4;
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
	iPosX = y;
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

// MapWindow Rect: returns a rectangle in the real coordinate system from the virtual coordinates 
// of an emulated fullscreen window. NULL or void returns the rectangle of the whole client area.

RECT dxwCore::MapWindowRect(void)
{
	return MapWindowRect(NULL);
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

void dxwCore::MapRect(int *nXDest, int *nYDest, int *nWDest, int *nHDest)
{
		RECT client;
		(*pGetClientRect)(hWnd, &client);
		*nXDest= *nXDest * client.right / dwScreenWidth;
		*nYDest= *nYDest * client.bottom / dwScreenHeight;
		*nWDest= *nWDest * client.right / dwScreenWidth;
		*nHDest= *nHDest * client.bottom / dwScreenHeight;
}

void dxwCore::ScreenRefresh(void)
{
	// optimization: don't blit too often!
	// 20mSec seems a good compromise.
	#define DXWREFRESHINTERVAL 20

	LPDIRECTDRAWSURFACE lpDDSPrim;
//	extern LPDIRECTDRAWSURFACE GetPrimarySurface();
	extern HRESULT WINAPI extBlt(LPDIRECTDRAWSURFACE lpdds, LPRECT lpdestrect, LPDIRECTDRAWSURFACE lpddssrc, LPRECT lpsrcrect, DWORD dwflags, LPDDBLTFX lpddbltfx);

	static int t = -1;
	if (t == -1)
		t = GetTickCount()-(DXWREFRESHINTERVAL+1); // V.2.1.69: trick - subtract 
	int tn = GetTickCount();

	if (tn-t < DXWREFRESHINTERVAL) return;

	lpDDSPrim=dxw.GetPrimarySurface();
	// if too early ....
	if (lpDDSPrim)
		extBlt(lpDDSPrim, NULL, lpDDSPrim, NULL, 0, NULL);

	(*pInvalidateRect)(hWnd, NULL, FALSE);
	t = tn;
}


static void ShowFPS()
{
	static DWORD time = 0xFFFFFFFF;
	static DWORD FPSCount = 0;
	DWORD tmp;
	tmp = GetTickCount();
	if((tmp - time) > 1000) {
		// log fps count
		OutTrace("FPSCount=%d\n", FPSCount);
		// show fps count on status win
		DxWndStatus.FPSCount = FPSCount;
		SetHookStatus(&DxWndStatus);
		// reset
		FPSCount=0;
		time = tmp;
	}
	else {
		FPSCount++;
	}
}

static void LimitFrameCount(int delay)
{
	static DWORD time = 0xFFFFFFFF;
	extern void do_slow(int);
	DWORD tmp;
	tmp = GetTickCount();
	if((tmp - time) > (DWORD)delay) {
		time = tmp;
	}
	else
		Sleep(tmp - time);
		//do_sslow(tmp - time);
}

static BOOL SkipFrameCount(int delay)
{
	static DWORD time = 0xFFFFFFFF;
	DWORD tmp;
	tmp = GetTickCount();
	if((tmp - time) > (DWORD)delay) {
		time = tmp;
		return FALSE;
	}
	return TRUE;
}

BOOL dxwCore::HandleFPS()
{
	if(dwFlags2 & SHOWFPS) ShowFPS();
	if(dwFlags2 & LIMITFPS) LimitFrameCount(dxw.MaxFPS);
	if(dwFlags2 & SKIPFPS) if(SkipFrameCount(dxw.MaxFPS)) return TRUE;
	return FALSE;
}

DWORD dxwCore::GetTickCount(void)
{
	DWORD dwTick;
	static DWORD dwLastRealTick=0;
	static DWORD dwLastFakeTick=0;
	DWORD dwNextRealTick;
//	extern DXWNDSTATUS *pStatus;

	dwNextRealTick=(*pGetTickCount)();
	dwTick=(dwNextRealTick-dwLastRealTick);
//	TimeShift=pStatus->iTimeShift;
	TimeShift=GetTimeShift();
	if (TimeShift > 0) dwTick >>= TimeShift;
	if (TimeShift < 0) dwTick <<= -TimeShift;
	dwLastFakeTick += dwTick;
	dwLastRealTick = dwNextRealTick;
	return dwLastFakeTick;
}

DWORD dxwCore::StretchTime(DWORD dwTimer)
{
	//extern DXWNDSTATUS *pStatus;
	//TimeShift=pStatus->iTimeShift;
	TimeShift=GetTimeShift();
	if (TimeShift > 0) dwTimer <<= TimeShift;
	if (TimeShift < 0) dwTimer >>= -TimeShift;
	return dwTimer;
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

	//TimeShift=pStatus->iTimeShift;
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
		TimeShift=GetTimeShift();
		if (TimeShift > 0) dwTick >>= TimeShift;
		if (TimeShift < 0) dwTick <<= -TimeShift;
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