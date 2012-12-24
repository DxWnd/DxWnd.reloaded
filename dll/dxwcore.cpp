#include "dxwnd.h"
//#include "dxwcore.hpp"
#include "syslibs.h"

extern GetCursorPos_Type pGetCursorPos;
extern ClientToScreen_Type pClientToScreen;

dxwCore::dxwCore()
{
}

dxwCore::~dxwCore()
{
}

void dxwCore::SethWnd(HWND hwnd)
{
	hWnd=hwnd;
}

POINT FixCursorPos(POINT prev)
{
	POINT curr;
	RECT rect;
	extern LPRECT lpClipRegion;

	curr=prev;

	// scale mouse coordinates
	// remember: rect from GetClientRect always start at 0,0!
	if(dwFlags & MODIFYMOUSE){
		if (!(*pGetClientRect)(hWnd, &rect)) {
			OutTraceD("GetClientRect ERROR %d at %d\n", GetLastError(),__LINE__);
			curr.x = curr.y = 0;
		}

		if (rect.right)  curr.x = (curr.x * VirtualScr.dwWidth) / rect.right;
		if (rect.bottom) curr.y = (curr.y * VirtualScr.dwHeight) / rect.bottom;
	}

	if((dwFlags & ENABLECLIPPING) && lpClipRegion){
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
		if (curr.x >= (LONG)VirtualScr.dwWidth-CLIP_TOLERANCE) curr.x=VirtualScr.dwWidth-1;
		if (curr.y >= (LONG)VirtualScr.dwHeight-CLIP_TOLERANCE) curr.y=VirtualScr.dwHeight-1;
	}

	return curr;
}