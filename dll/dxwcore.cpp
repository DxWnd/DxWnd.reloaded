#include "dxwnd.h"
#include "dxwcore.hpp"
#include "syslibs.h"

extern GetCursorPos_Type pGetCursorPos;
extern ClientToScreen_Type pClientToScreen;

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
}

dxwCore::~dxwCore()
{
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

	OutTraceD("Core::SetClipCursor:\n");
	if (hWnd==NULL) {
		OutTraceD("Core::SetClipCursor: ASSERT hWnd==NULL\n");
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
	OutTraceD("Core::SetClipCursor: rect=(%d,%d)-(%d,%d)\n",
		Rect.left, Rect.top, Rect.right, Rect.bottom);
}

void dxwCore::EraseClipCursor()
{
	OutTraceD("Core::EraseClipCursor:\n");
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

void dxwCore::ScreenRefresh(void)
{
	// optimization: don't blit too often!
	// 20mSec seems a good compromise.
	#define DXWREFRESHINTERVAL 20

	LPDIRECTDRAWSURFACE lpDDSPrim;
	extern LPDIRECTDRAWSURFACE GetPrimarySurface();
	extern HRESULT WINAPI extBlt(LPDIRECTDRAWSURFACE lpdds, LPRECT lpdestrect, LPDIRECTDRAWSURFACE lpddssrc, LPRECT lpsrcrect, DWORD dwflags, LPDDBLTFX lpddbltfx);

	static int t = -1;
	if (t == -1)
		t = GetTickCount()-(DXWREFRESHINTERVAL+1); // V.2.1.69: trick - subtract 
	int tn = GetTickCount();

	if (tn-t < DXWREFRESHINTERVAL) return;

	lpDDSPrim=GetPrimarySurface();
	// if too early ....
	if (lpDDSPrim)
		extBlt(lpDDSPrim, NULL, lpDDSPrim, NULL, 0, NULL);

	(*pInvalidateRect)(hWnd, NULL, FALSE);
	t = tn;
}