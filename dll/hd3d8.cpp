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

extern LockRect_Type pLockRect8;
extern UnlockRect_Type pUnlockRect8;

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
