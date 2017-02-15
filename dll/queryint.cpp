#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <d3d.h>
#include <stdio.h>
#include "dxwnd.h"
#include "dxwcore.hpp"
#include "dxhook.h"
#include "syslibs.h"
#include "dxhelper.h"
#include "hddraw.h"

extern void HookDDSurface(LPDIRECTDRAWSURFACE *, int, BOOL);
extern void HookGammaControl(LPVOID *);
extern void HookDDSession(LPDIRECTDRAW *, int);
extern void HookDirect3DSession(LPDIRECTDRAW *, int);
extern void HookDirect3DDevice(LPVOID *, int);
extern void HookViewport(LPDIRECT3DVIEWPORT *, int);

// extQueryInterfaceDX: this is the single procedure that manages all QueryInterface methods within the DirectX classes
// it is better to have it unique because of the transitive and reflexive properties of QueryInterface, that means
// that every DirectX COM object can QueryInterface itself to become another object type belonging to this family.

// some unhandled interfaces in emulation mode:
// REFIID={84e63de0-46aa-11cf-816f-0000c020156e}: IID_IDirect3DHALDevice

typedef enum {
	TYPE_OBJECT_UNKNOWN = 0,
	TYPE_OBJECT_DIRECTDRAW,
	TYPE_OBJECT_DDRAWSURFACE,
	TYPE_OBJECT_DIRECT3D,
	TYPE_OBJECT_D3DDEVICE,
	TYPE_OBJECT_GAMMARAMP,
	TYPE_OBJECT_TEXTURE,
	TYPE_OBJECT_VIEWPORT };

HRESULT WINAPI extQueryInterfaceDX(int dxversion, QueryInterface_Type pQueryInterface, void *lpdds, REFIID riid, LPVOID *obp)
{
	HRESULT res;
	BOOL IsPrim;
	BOOL IsBack;
	int iObjectType;
	int iObjectVersion;
	DWORD dwCaps;
	extern LPDIRECTDRAWSURFACE lpDDSEmu_Prim;

	IsPrim=dxwss.IsAPrimarySurface((LPDIRECTDRAWSURFACE)lpdds);
	IsBack=dxwss.IsABackBufferSurface((LPDIRECTDRAWSURFACE)lpdds);
	OutTraceDDRAW("QueryInterface(%d): lpdds=%x%s REFIID=%x(%s)\n",
		dxversion, lpdds, dxwss.ExplainSurfaceRole((LPDIRECTDRAWSURFACE)lpdds), riid.Data1, ExplainGUID((GUID *)&riid));	

	iObjectVersion = 0;
	iObjectType = TYPE_OBJECT_UNKNOWN;
	switch(riid.Data1){
	// DirectDraw
	case 0x6C14DB80: // IID_IDirectDraw
		iObjectType=TYPE_OBJECT_DIRECTDRAW; iObjectVersion=1; break;
	case 0xB3A6F3E0: // IID_IDirectDraw2
		iObjectType=TYPE_OBJECT_DIRECTDRAW; iObjectVersion=2; break;
	case 0x618f8ad4: // IID_IDirectDraw3
		iObjectType=TYPE_OBJECT_DIRECTDRAW; iObjectVersion=3; break;		
	case 0x9c59509a: // IID_IDirectDraw4
		iObjectType=TYPE_OBJECT_DIRECTDRAW; iObjectVersion=4; break;
	case 0x15e65ec0: // IID_IDirectDraw7
		iObjectType=TYPE_OBJECT_DIRECTDRAW; iObjectVersion=7; break;
	// DirectDrawSurface
	case 0x6C14DB81:
		iObjectType=TYPE_OBJECT_DDRAWSURFACE; iObjectVersion=1; break;
	case 0x57805885:		//DDSurface2 - WIP (Dark Reign)
		iObjectType=TYPE_OBJECT_DDRAWSURFACE; iObjectVersion=2; break;
	case 0xDA044E00:		
		iObjectType=TYPE_OBJECT_DDRAWSURFACE; iObjectVersion=3; break;
	case 0x0B2B8630:
		iObjectType=TYPE_OBJECT_DDRAWSURFACE; iObjectVersion=4; break;
	case 0x06675a80:
		iObjectType=TYPE_OBJECT_DDRAWSURFACE; iObjectVersion=7; break;
	// Direct3D
	case 0x3BBA0080: // IID_IDirect3D
		iObjectType=TYPE_OBJECT_DIRECT3D; iObjectVersion=1; break;
	case 0x6aae1ec1: // IID_IDirect3D2
		iObjectType=TYPE_OBJECT_DIRECT3D; iObjectVersion=2; break;
	case 0xbb223240: // IID_IDirect3D3
		iObjectType=TYPE_OBJECT_DIRECT3D; iObjectVersion=3; break;
	case 0xf5049e77: // IID_IDirect3D7
		iObjectType=TYPE_OBJECT_DIRECT3D; iObjectVersion=7; break;
	// Direct3DDevice 
	case 0x84e63de0: // IID_IDirect3DHALDevice
		iObjectType=TYPE_OBJECT_D3DDEVICE; iObjectVersion=1; break;
	case 0xA4665C60: // IID_IDirect3DRGBDevice
		iObjectType=TYPE_OBJECT_D3DDEVICE; iObjectVersion=1; break;
	case 0xF2086B20: // IID_IDirect3DRampDevice
		iObjectType=TYPE_OBJECT_D3DDEVICE; iObjectVersion=1; break;
	case 0x881949a1: // IID_IDirect3DMMXDevice
		iObjectType=TYPE_OBJECT_D3DDEVICE; iObjectVersion=1; break;
	case 0x50936643: // IID_IDirect3DRefDevice    
		iObjectType=TYPE_OBJECT_D3DDEVICE; iObjectVersion=2; break;
	case 0x8767df22: // IID_IDirect3DNullDevice
		iObjectType=TYPE_OBJECT_D3DDEVICE; iObjectVersion=2; break;
	case 0xf5049e78: // IID_IDirect3DTnLHalDevice, 
		iObjectType=TYPE_OBJECT_D3DDEVICE; iObjectVersion=3; break;
	case 0x64108800: // IID_IDirect3DDevice       
		iObjectType=TYPE_OBJECT_D3DDEVICE; iObjectVersion=1; break;
	case 0x93281501: // IID_IDirect3DDevice2
		iObjectType=TYPE_OBJECT_D3DDEVICE; iObjectVersion=2; break;
	case 0xb0ab3b60: // IID_IDirect3DDevice3 
		iObjectType=TYPE_OBJECT_D3DDEVICE; iObjectVersion=3; break;
	case 0xf5049e79: // IID_IDirect3DDevice7  
		iObjectType=TYPE_OBJECT_D3DDEVICE; iObjectVersion=7; break;
	case 0x4B9F0EE0:
		OutTraceDW("QueryInterface: IID_IDirectDrawColorControl\n");
		break;
	case 0x69C11C3E:
		iObjectType=TYPE_OBJECT_GAMMARAMP; iObjectVersion=1; break;
	// textures
	case 0x2CDCD9E0:
		iObjectType=TYPE_OBJECT_TEXTURE; iObjectVersion=1; break;
	case 0x93281502:
		iObjectType=TYPE_OBJECT_TEXTURE; iObjectVersion=2; break;
	case 0x4417C146: //IID_IDirect3DViewport
		iObjectType=TYPE_OBJECT_VIEWPORT; iObjectVersion=1; break;
	case 0x93281500: //IID_IDirect3DViewport2
		iObjectType=TYPE_OBJECT_VIEWPORT; iObjectVersion=2; break;
	case 0xb0ab3b61: //IID_IDirect3DViewport3
		iObjectType=TYPE_OBJECT_VIEWPORT; iObjectVersion=3; break;
	} 

	char *sLabel;
	switch(iObjectType){
		case TYPE_OBJECT_UNKNOWN: sLabel = "(unknown)"; break;
		case TYPE_OBJECT_DIRECTDRAW: sLabel = "IID_IDirectDraw"; break;
		case TYPE_OBJECT_DDRAWSURFACE: sLabel = "IID_IDirectDrawSurface"; break;
		case TYPE_OBJECT_DIRECT3D: sLabel = "IID_IDirect3D"; break;
		case TYPE_OBJECT_D3DDEVICE: sLabel = "IID_IDirect3DDevice"; break;
		case TYPE_OBJECT_GAMMARAMP: sLabel = "IID_IDirectDrawGammaRamp"; break;
		case TYPE_OBJECT_TEXTURE: sLabel = "IID_IDirect3DTexture"; break;
		case TYPE_OBJECT_VIEWPORT: sLabel = "IID_IDirect3DViewport"; break;
	}
	OutTraceDW("QueryInterface: Got interface for %s version %d\n", sLabel, iObjectVersion);

	switch(iObjectType){
		// simulate unavailable interfaces (useful?)
		case TYPE_OBJECT_DIRECTDRAW:
			if (iObjectVersion > (int)dxw.dwMaxDDVersion) {
				*obp = NULL;
				OutTraceDW("QueryInterface: DDVersion=%d SUPPRESS lpdds=%x(%s) REFIID=%x obp=(NULL) ret=0 at %d\n",
					iObjectVersion, lpdds, IsPrim?"":"(PRIM)", riid.Data1, __LINE__);
				return(0);
			}
			break;
		case TYPE_OBJECT_DDRAWSURFACE:
			if (iObjectVersion > (int)dxw.dwMaxDDVersion) {
				*obp = NULL;
				OutTraceDW("QueryInterface: DDSVersion=%d SUPPRESS lpdds=%x(%s) REFIID=%x obp=(NULL) ret=0 at %d\n",
					iObjectVersion, lpdds, IsPrim?"":"(PRIM)", riid.Data1, __LINE__);
				return(0);
			}
			break;
		case TYPE_OBJECT_GAMMARAMP:
			// fix the target for gamma ramp creation: if it is a primary surface, use the real one!!
			// v2.03.37: do this just when in esurface emulated mode!! 
			if((dxw.dwFlags1 & EMULATESURFACE) && 
				dxwss.IsAPrimarySurface((LPDIRECTDRAWSURFACE)lpdds)) 
				lpdds = lpDDSEmu_Prim; 
			break;
	}

	res = (*pQueryInterface)(lpdds, riid, obp);

	if(res) {
		OutTraceDW("QueryInterface: ERROR lpdds=%x%s REFIID=%x obp=%x ret=%x(%s) at %d\n",
			lpdds, IsPrim?"(PRIM)":"", riid.Data1, *obp, res, ExplainDDError(res), __LINE__);
		return res;
	}

	if (! *obp) {
		OutTraceDW("QueryInterface: Interface for object %x not found\n", riid.Data1);
		return res;
	}

	// added trace
	OutTraceDW("QueryInterface: lpdds=%x REFIID=%x obp=%x obj=%s version=%d ret=0\n",
		lpdds, riid.Data1, *obp, sLabel, iObjectVersion);

	switch(iObjectType){
		// simulate unavailable interfaces (useful?)
		case TYPE_OBJECT_DIRECTDRAW:
			HookDDSession((LPDIRECTDRAW *)obp, iObjectVersion);
			break;
		case TYPE_OBJECT_DDRAWSURFACE:
			dxw.dwDDVersion=iObjectVersion;
			if(IsPrim){
				OutTraceDW("QueryInterface(S): primary=%x new=%x\n", lpdds, *obp);
				dxwss.PushPrimarySurface((LPDIRECTDRAWSURFACE)*obp, iObjectVersion);
				HookDDSurface((LPDIRECTDRAWSURFACE *)obp, dxw.dwDDVersion, TRUE);
			}
			else{
				if(IsBack) dxwss.PushBackBufferSurface((LPDIRECTDRAWSURFACE)*obp, iObjectVersion);
				else dxwss.PopSurface((LPDIRECTDRAWSURFACE)*obp); // no primary, no backbuffer, then pop.
				// v2.02.13: seems that hooking inconditionally gives troubles. What is the proper safe hook condition?
				HookDDSurface((LPDIRECTDRAWSURFACE *)obp, dxw.dwDDVersion, FALSE);
			}
			dwCaps = dxwcdb.GetCaps((LPDIRECTDRAWSURFACE)lpdds);
			if (dwCaps) {
				OutTraceDW("QueryInterface(S): PASS lpdds=%x->%x caps=%x(%s)\n", lpdds, *obp, dwCaps, ExplainDDSCaps(dwCaps));
				dxwcdb.PushCaps(*(LPDIRECTDRAWSURFACE *)obp, dwCaps);
			}	
			else {
				OutTraceDW("QueryInterface(S): NO CAPS\n");
			}
			break;
		case TYPE_OBJECT_DIRECT3D:
			HookDirect3DSession((LPDIRECTDRAW *)obp, iObjectVersion);
			break;
		case TYPE_OBJECT_D3DDEVICE:
			HookDirect3DDevice(obp, iObjectVersion);
			break;
		case TYPE_OBJECT_GAMMARAMP:
			HookGammaControl(obp);
			break;
		case TYPE_OBJECT_TEXTURE:
			HookTexture(obp, iObjectVersion);
			break;
		case TYPE_OBJECT_VIEWPORT:
			HookViewport((LPDIRECT3DVIEWPORT *)obp, iObjectVersion);
			break;
	}

	return res;
}
