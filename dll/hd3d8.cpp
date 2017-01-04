#include <D3D8.h>
#include "dxwnd.h"
#include "dxwcore.hpp"
#include "dxhook.h"
#include "dxhelper.h"
#include "syslibs.h"
#include "stdio.h"

extern void TextureHandling(LPDIRECTDRAWSURFACE);

typedef HRESULT (WINAPI *LockRect_Type)(void *, UINT, D3DLOCKED_RECT *, CONST RECT *, DWORD);
typedef HRESULT (WINAPI *UnlockRect_Type)(void *, UINT);
typedef HRESULT (WINAPI *GetFrontBuffer_Type)(void *, LPDIRECTDRAWSURFACE);
typedef HRESULT (WINAPI *GetAdapterDisplayMode_Type)(void *, UINT, D3DDISPLAYMODE *);
typedef HRESULT	(WINAPI *CopyRects_Type)(void *, LPDIRECTDRAWSURFACE, CONST RECT *, UINT, LPDIRECTDRAWSURFACE, CONST POINT *);
typedef HRESULT (WINAPI *GetDirect3D8_Type)(void *, void **);

extern LockRect_Type pLockRect8;
extern UnlockRect_Type pUnlockRect8;
extern GetAdapterDisplayMode_Type pGetAdapterDisplayMode8;
extern CopyRects_Type pCopyRects;
extern GetFrontBuffer_Type pGetFrontBuffer;
extern GetDirect3D8_Type pGetDirect3D8;

extern void D3DTextureDump(D3DSURFACE_DESC, D3DLOCKED_RECT);
extern void D3DTextureHighlight(D3DSURFACE_DESC, D3DLOCKED_RECT);
extern void D3DTextureHack(D3DSURFACE_DESC, D3DLOCKED_RECT);
extern void D3DTextureTransp(D3DSURFACE_DESC, D3DLOCKED_RECT);
extern char *ExplainD3DSurfaceFormat(DWORD);

void D3D8TextureHandling(void *arg, int Level)
{
	HRESULT res;
	LPDIRECT3DTEXTURE8 lpd3dtex = (LPDIRECT3DTEXTURE8)arg;
	IDirect3DSurface8 *pSurfaceLevel;
	D3DSURFACE_DESC Desc;
	D3DLOCKED_RECT LockedRect;
	OutTraceB("D3D8TextureHandling: arg=%x level=%d\n", (DWORD)arg, Level);
	// Beware: attempts to dump surfaces at level > 0 result in stack corruption!!!
	if(Level > 0) return;
	if(res=lpd3dtex->GetSurfaceLevel(Level, &pSurfaceLevel)){
		OutTraceE("Texture::GetSurfaceLevel ERROR: res=%d(%s)\n", res, ExplainDDError(res));
		return;
	}
	if(res=lpd3dtex->GetLevelDesc(Level, &Desc)){
		OutTraceE("Texture::GetLevelDesc ERROR: res=%d(%s)\n", res, ExplainDDError(res));
		return;
	}
	pSurfaceLevel->Release();
	switch(Desc.Type){
		case D3DRTYPE_SURFACE:
		case D3DRTYPE_TEXTURE:
			break;
		default:
			return;
			break;
	}
	if(Desc.Usage == D3DUSAGE_RENDERTARGET)return;
	//pSurfaceLevel->GetRenderTargetData(&pRenderTarget, &pDestSurface);	
	res=(*pLockRect8)(lpd3dtex, Level, &LockedRect, NULL, 0);
	OutTrace("D3D8TextureHandling: lpd3dtex=%x level=%d format=0x%x(%s) size=(%dx%d) bits=%x pitch=%d\n", 
		lpd3dtex, Level, Desc.Format, ExplainD3DSurfaceFormat(Desc.Format), 
		Desc.Width, Desc.Height, LockedRect.pBits, LockedRect.Pitch);
	switch(dxw.dwFlags5 & TEXTUREMASK){
		case TEXTUREHIGHLIGHT: 
			D3DTextureHighlight(Desc, LockedRect);
			break;
		case TEXTUREDUMP: 
			D3DTextureDump(Desc, LockedRect);
			break;
		case TEXTUREHACK:
			D3DTextureHack(Desc, LockedRect);
			break;
		case TEXTURETRANSP:
			D3DTextureTransp(Desc, LockedRect);
			break;
	}
	res=(*pUnlockRect8)(lpd3dtex, Level);
}

/*
from http://realmike.org/blog/projects/taking-screenshots-with-direct3d-8/

Accessing the Front Buffer

The IDirect3DDevice8 interface provides the GetFrontBuffer and GetBackBuffer methods to gain access to the swap chain of a Direct3D 8 application. These methods have the 
following characteristics:

    GetBackBuffer – By using this method you can obtain an IDirect3DSurface8 interface pointer for each of the buffers in the swap chain. 
	However, unless you explicitily requested a lockable back buffer when creating the device (by using the D3DPRESENTFLAG_LOCKABLE_BACKBUFFER flag), 
	you are not allowed to lock the surface. The SDK docs mention a “performance cost” when using lockable back buffers, even if they’re not actually locked. 
	Our Screenshot function should not require you to rewrite your device creation code, let alone degrade performance, therefore we won’t use the GetBackBuffer method here.
    GetFrontBuffer – This method copies the contents of the front buffer to a system-memory surface that is provided by the application. 
	What makes the GetFrontBuffer method especially useful for our purposes is that it converts the data into a 32-bit ARGB format so that we don’t have to handle 
	different formats manually.

Note: When using the GetFrontBuffer method, we’ll always capture the entire screen, which might be undesired in a windowed application. However, the majority of 
applications are full-screen. Therefore, we’ll ignore this issue.

The GetFrontBuffer method requires us to provide a system-memory surface of the same dimensions as the screen. This surface will be filled with a copy of the front buffer’s 
contents. So, how do we retrieve the screen dimensions when all we have is a pointer to the IDirect3DDevice8 interface? We can use the GetAdapterDisplayMode method of the 
IDirect3D8 interface to query information about the current display mode of a given adapter. A pointer to the IDirect3D8 interface can be obtained by calling the GetDirect3D 
method of the IDirect3DDevice8 interface. The adapter identifier that is expected by the GetAdapterDisplayMode method can be obtained by using the 
IDirect3DDevice8::GetCreationParameters method. To summarize, these are the required steps to retrieve the screen dimensions:

    Call the IDirect3DDevice8::GetDirect3D method to retrieve a pointer to an IDirect3D8 interface.
    Call the IDirect3DDevice8::GetCreationParameters method, which returns the identifier of the adapter that the Direct3D device uses.
    Call the IDirect3D8::GetAdapterDisplayMode with the adapter identifier that we retrieved in Step 2.

The following code snippet performs these three steps:

D3DDEVICE_CREATION_PARAMETERS dcp;
dcp.AdapterOrdinal = D3DADAPTER_DEFAULT;
lpDevice->GetCreationParameters(&dcp);

D3DDISPLAYMODE dm;
dm.Width = dm.Height = 0;

// retrieve pointer to IDirect3D8 interface,
// which provides the GetAdapterDisplayMode method
LPDIRECT3D8 lpD3D = NULL;
lpDevice->GetDirect3D(&lpD3D);
if (lpD3D)
{
    // query the screen dimensions of the current adapter
    lpD3D->GetAdapterDisplayMode(dcp.AdapterOrdinal, &dm);
    SAFERELEASE(lpD3D);
}

Now we can pass the values in dm.Width and dm.Height to the IDirect3DDevice8::CreateImageSurface method to create a system-memory surface that can be used by the GetFrontBuffer method. The expected format of the surface is D3DFMT_A8R8G8B8, which means that there are 8 bits each for the blue, green, red, and alpha components of the colors. The following code snippet creates the surface and calls the GetFrontBuffer method to fill the surface:

LPDIRECT3DSURFACE8 lpSurface = NULL;
lpDevice->CreateImageSurface(
    dm.Width, dm.Height,
    D3DFMT_A8R8G8B8,
    &lpSurface
);

lpDevice->GetFrontBuffer(lpSurface);
*/

HRESULT dxGetFrontBuffer(void *lpd3dd, LPDIRECTDRAWSURFACE xdest)
{
	HRESULT res;
	D3DDEVICE_CREATION_PARAMETERS dcp;
	LPDIRECT3DSURFACE8 lpSurface = NULL;
	IDirect3DDevice8 *lpDevice = (IDirect3DDevice8 *)lpd3dd;
	IDirect3DSurface8 *dest = (IDirect3DSurface8 *)xdest;

	dcp.AdapterOrdinal = D3DADAPTER_DEFAULT;
	lpDevice->GetCreationParameters(&dcp);

	D3DDISPLAYMODE dm;
	dm.Width = dm.Height = 0;

	// retrieve pointer to IDirect3D8 interface,
	// which provides the GetAdapterDisplayMode method
	LPDIRECT3D8 lpD3D = NULL;
	//res = lpDevice->GetDirect3D(&lpD3D);
	res = (*pGetDirect3D8)(lpDevice, (void **)&lpD3D);
	if(res) {
		OutTraceE("GetFrontBuffer: GetDirect3D ERROR res=%x at %d\n", res, __LINE__);
		return res;
	}

	// query the screen dimensions of the current adapter
	//res = lpD3D->GetAdapterDisplayMode(dcp.AdapterOrdinal, &dm);
	res = (*pGetAdapterDisplayMode8)(lpD3D, dcp.AdapterOrdinal, &dm);
	lpD3D->Release();
	if(res) {
		OutTraceE("GetFrontBuffer: GetAdapterDisplayMode ERROR res=%x at %d\n", res, __LINE__);
		return res;
	}
	else {
		OutTraceB("GetFrontBuffer: screen size=(%dx%d)\n", dm.Width, dm.Height);
	}

	res = lpDevice->CreateImageSurface(dm.Width, dm.Height, D3DFMT_A8R8G8B8, &lpSurface);
	if(res) {
		OutTraceE("GetFrontBuffer: CreateImageSurface ERROR res=%x at %d\n", res, __LINE__);
		return res;
	}

	//res = lpDevice->GetFrontBuffer(lpSurface);
	res = (*pGetFrontBuffer)(lpDevice, (LPDIRECTDRAWSURFACE)lpSurface);
	if(res) {
		OutTraceE("GetFrontBuffer: GetFrontBuffer ERROR res=%x at %d\n", res, __LINE__);
		lpSurface->Release();
		return res;
	}

	RECT p0 = dxw.GetUnmappedScreenRect();
	RECT srcrect = dxw.GetScreenRect();
	OffsetRect(&srcrect, p0.left, p0.top);
	OutTraceB("GetFrontBuffer: screen rect=(%d,%d)-(%d,%d)\n", srcrect.left, srcrect.top, srcrect.right, srcrect.bottom);
	POINT destpoint = {0, 0};
	//res = lpDevice->CopyRects(lpSurface, (CONST RECT *)&srcrect, 1, dest, (CONST POINT *)&destpoint);
	res = (*pCopyRects)(lpDevice, (LPDIRECTDRAWSURFACE)lpSurface, &srcrect, 1, (LPDIRECTDRAWSURFACE)dest, &destpoint);
	lpSurface->Release();
	if(res) {
		OutTraceE("GetFrontBuffer: CopyRects ERROR res=%x at %d\n", res, __LINE__);
		return res;
	}

	return DD_OK;
}

HRESULT  dxCopyRects(void *lpd3dd, LPDIRECTDRAWSURFACE psrc, LPDIRECTDRAWSURFACE pdst)
{
	HRESULT res;
	IDirect3DSurface8 *lpsrc = (IDirect3DSurface8 *)psrc;
	IDirect3DSurface8 *lpdst = (IDirect3DSurface8 *)pdst;
	IDirect3DDevice8 *lpDevice = (IDirect3DDevice8 *)lpd3dd;
	LPDIRECT3DSURFACE8 lpImageSrc = NULL;
	LPDIRECT3DSURFACE8 lpImageDst = NULL;
	D3DSURFACE_DESC SrcDesc, DstDesc;
	D3DLOCKED_RECT SrcLockedRect, DstLockedRect;

	// first, build  source image
	if(res = lpsrc->GetDesc(&SrcDesc)){
		OutTraceE("dxCopyRects: GetDesc ERROR res=%x at %d\n", res, __LINE__);
		return res;
	}
	else{
		OutTraceB("dxCopyRects: source size=(%dx%d) format=%d\n", SrcDesc.Width, SrcDesc.Height, SrcDesc.Format);
	}

	if(res = lpDevice->CreateImageSurface(SrcDesc.Width, SrcDesc.Height, SrcDesc.Format, &lpImageSrc)){
		OutTraceE("dxCopyRects: CreateImageSurface ERROR res=%x at %d\n", res, __LINE__);
		return res;
	}

	// get source image
	if(res = (*pCopyRects)(lpDevice, (LPDIRECTDRAWSURFACE)psrc, NULL, 0, (LPDIRECTDRAWSURFACE)lpImageSrc, NULL)){
		OutTraceE("dxCopyRects: CopyRects ERROR res=%x at %d\n", res, __LINE__);
		return res;
	}

	// build a target image similar to destination 
	if(res = lpdst->GetDesc(&DstDesc)){
		OutTraceE("dxCopyRects: GetDesc ERROR res=%x at %d\n", res, __LINE__);
		return res;
	}
	else{
		OutTraceB("dxCopyRects: dest size=(%dx%d) format=%d\n", DstDesc.Width, DstDesc.Height, DstDesc.Format);
	}

	if(res = lpDevice->CreateImageSurface(DstDesc.Width, DstDesc.Height, DstDesc.Format, &lpImageDst)){
		OutTraceE("dxCopyRects: GetDesc ERROR res=%x at %d\n", res, __LINE__);
		return res;
	}

	// make the conversion here ....
	if (res=lpImageSrc->LockRect(&SrcLockedRect, NULL, D3DLOCK_READONLY)){
		OutTraceE("dxCopyRects: LockRect ERROR res=%x at %d\n", res, __LINE__);
		return res;
	}
	if(res = lpImageDst->LockRect(&DstLockedRect, NULL, 0)){
		OutTraceE("dxCopyRects: LockRect ERROR res=%x at %d\n", res, __LINE__);
		return res;
	}

	// pixel conversion here
	switch(SrcDesc.Format){
		case D3DFMT_A8R8G8B8:
		case D3DFMT_X8R8G8B8:
			switch(DstDesc.Format){
				case D3DFMT_R5G6B5:
					{
						OutTraceB("dxCopyRects: converting 32 to 16 BPP\n", res, __LINE__);
						DWORD *srcpix;
						WORD *dstpix;
						for(unsigned int y=0; y<min(DstDesc.Height,SrcDesc.Height); y++){
							srcpix = (DWORD *)SrcLockedRect.pBits + (y * SrcLockedRect.Pitch / 4);
							dstpix = (WORD *)DstLockedRect.pBits + (y * DstLockedRect.Pitch / 2);
							for(unsigned int x=0; x<min(DstDesc.Width,SrcDesc.Width); x++){
								// to do ....
								*dstpix++ = *srcpix++ & 0xFFFF;
							}
						}
					}
					break;
			}
			break;
		case D3DFMT_R5G6B5:
			switch(DstDesc.Format){
				case D3DFMT_A8R8G8B8:
				case D3DFMT_X8R8G8B8:
					{
						OutTraceB("dxCopyRects: converting 16 to 32 BPP\n", res, __LINE__);
						WORD *srcpix;
						DWORD *dstpix;
						extern void SetPalette16BPP(void);
						extern DWORD *Palette16BPP;
						if (!Palette16BPP) SetPalette16BPP();
						for(unsigned int y=0; y<min(DstDesc.Height,SrcDesc.Height); y++){
							srcpix = (WORD *)SrcLockedRect.pBits + (y * SrcLockedRect.Pitch / 2);
							dstpix = (DWORD *)DstLockedRect.pBits + (y * DstLockedRect.Pitch / 4);
							for(unsigned int x=0; x<min(DstDesc.Width,SrcDesc.Width); x++)
								*(dstpix ++) = Palette16BPP[*(srcpix ++)]; *srcpix++;
						}
					}
					break;
			}
	}

	lpImageSrc->UnlockRect();
	lpImageDst->UnlockRect();

	// copy to target surface
	if(res = (*pCopyRects)(lpDevice, (LPDIRECTDRAWSURFACE)lpImageDst, NULL, 0, (LPDIRECTDRAWSURFACE)lpdst, NULL)){
		OutTraceE("dxCopyRects: CopyRects ERROR res=%x at %d\n", res, __LINE__);
		return res;
	}

	// clean up
	lpImageSrc->Release();
	lpImageDst->Release();
	return res;
}


