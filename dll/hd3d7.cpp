#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <d3d.h>
#include <stdio.h>
#include "dxwnd.h"
#include "dxwcore.hpp"
#include "dxhook.h"
#include "syslibs.h"
#include "dxhelper.h"

//#undef OutTraceD3D
//#define OutTraceD3D OutTrace

// exported API

typedef HRESULT (WINAPI *Direct3DCreateDevice_Type)(GUID FAR *, LPDIRECT3D, LPDIRECTDRAWSURFACE, LPDIRECT3D *, LPUNKNOWN);
typedef HRESULT (WINAPI *Direct3DCreate_Type)(UINT, LPDIRECT3D *, LPUNKNOWN);

Direct3DCreateDevice_Type pDirect3DCreateDevice = NULL;
Direct3DCreate_Type pDirect3DCreate = NULL;

HRESULT WINAPI extDirect3DCreateDevice(GUID FAR *, LPDIRECT3D, LPDIRECTDRAWSURFACE, LPDIRECT3D *, LPUNKNOWN);
HRESULT WINAPI extDirect3DCreate(UINT, LPDIRECT3D *, LPUNKNOWN);

// IDirect3D-n interfaces

typedef HRESULT (WINAPI *QueryInterfaceD3_Type)(void *, REFIID, LPVOID *);
typedef HRESULT (WINAPI *Initialize_Type)(void *);
typedef HRESULT (WINAPI *EnumDevices_Type)(void *, LPD3DENUMDEVICESCALLBACK, LPVOID);
typedef HRESULT (WINAPI *EnumDevices7_Type)(void *, LPD3DENUMDEVICESCALLBACK7, LPVOID);
typedef HRESULT (WINAPI *CreateLight_Type)(void *, LPDIRECT3DLIGHT *, IUnknown *);
typedef HRESULT (WINAPI *CreateMaterial_Type)(void *, LPDIRECT3DMATERIAL *, IUnknown *);
typedef HRESULT (WINAPI *CreateViewport1_Type)(void *, LPDIRECT3DVIEWPORT *, IUnknown *);
typedef HRESULT (WINAPI *CreateViewport2_Type)(void *, LPDIRECT3DVIEWPORT2 *, IUnknown *);
typedef HRESULT (WINAPI *CreateViewport3_Type)(void *, LPDIRECT3DVIEWPORT3 *, IUnknown *);
typedef HRESULT (WINAPI *FindDevice_Type)(void *, LPD3DFINDDEVICESEARCH, LPD3DFINDDEVICERESULT);
typedef HRESULT (WINAPI *CreateDevice2_Type)(void *, REFCLSID, LPDIRECTDRAWSURFACE, LPDIRECT3DDEVICE2 *);
typedef HRESULT (WINAPI *CreateDevice3_Type)(void *, REFCLSID, LPDIRECTDRAWSURFACE4, LPDIRECT3DDEVICE3 *, LPUNKNOWN);
typedef HRESULT (WINAPI *CreateDevice7_Type)(void *, REFCLSID, LPDIRECTDRAWSURFACE7, LPDIRECT3DDEVICE7 *);
typedef HRESULT (WINAPI *EnumZBufferFormats_Type)(void *, REFCLSID, LPD3DENUMPIXELFORMATSCALLBACK, LPVOID);

QueryInterfaceD3_Type pQueryInterfaceD3 = NULL;
Initialize_Type pInitialize = NULL;
EnumDevices_Type pEnumDevices = NULL;
EnumDevices7_Type pEnumDevices7 = NULL;
CreateLight_Type pCreateLight = NULL;
CreateMaterial_Type pCreateMaterial = NULL;
CreateViewport1_Type pCreateViewport1 = NULL;
CreateViewport2_Type pCreateViewport2 = NULL;
CreateViewport3_Type pCreateViewport3 = NULL;
FindDevice_Type pFindDevice = NULL;
CreateDevice2_Type pCreateDevice2 = NULL;
CreateDevice3_Type pCreateDevice3 = NULL;
CreateDevice7_Type pCreateDevice7 = NULL;
EnumZBufferFormats_Type pEnumZBufferFormats = NULL;

HRESULT WINAPI extEnumZBufferFormats(void *, REFCLSID, LPD3DENUMPIXELFORMATSCALLBACK, LPVOID);

// Direct3DDevice-n interfaces

typedef HRESULT (WINAPI *QueryInterfaceD3D_Type)(void *, REFIID, LPVOID *);
typedef HRESULT (WINAPI *D3DInitialize_Type)(void *, LPDIRECT3D , LPGUID, LPD3DDEVICEDESC);
typedef HRESULT (WINAPI *D3DGetCaps_Type)(void *, LPD3DDEVICEDESC ,LPD3DDEVICEDESC);
typedef HRESULT (WINAPI *D3DGetCaps3_Type)(void *, LPD3DDEVICEDESC, LPD3DDEVICEDESC);
typedef HRESULT (WINAPI *AddViewport1_Type)(void *, LPDIRECT3DVIEWPORT);
typedef HRESULT (WINAPI *AddViewport2_Type)(void *, LPDIRECT3DVIEWPORT2);
typedef HRESULT (WINAPI *AddViewport3_Type)(void *, LPDIRECT3DVIEWPORT3);
typedef HRESULT (WINAPI *Scene3_Type)(void *); // BeginScene, EndScene
typedef HRESULT (WINAPI *SetRenderState3_Type)(void *, D3DRENDERSTATETYPE, DWORD);
typedef HRESULT (WINAPI *GetViewport7_Type)(void *, LPD3DVIEWPORT7);
typedef HRESULT (WINAPI *SetViewport7_Type)(void *, LPD3DVIEWPORT7);
typedef HRESULT (WINAPI *SetLightState_Type)(void *, D3DLIGHTSTATETYPE, DWORD);
typedef HRESULT (WINAPI *GetCurrentViewport2_Type)(void *, LPDIRECT3DVIEWPORT2 *);
typedef HRESULT (WINAPI *SetCurrentViewport2_Type)(void *, LPDIRECT3DVIEWPORT2);
typedef HRESULT (WINAPI *GetCurrentViewport3_Type)(void *, LPDIRECT3DVIEWPORT3 *);
typedef HRESULT (WINAPI *SetCurrentViewport3_Type)(void *, LPDIRECT3DVIEWPORT3);
typedef HRESULT (WINAPI *SetTexture3_Type)(void *, DWORD, LPDIRECT3DTEXTURE2);
typedef HRESULT (WINAPI *SetTexture7_Type)(void *, DWORD, LPDIRECTDRAWSURFACE7);
typedef HRESULT (WINAPI *SwapTextureHandles_Type)(void *, LPDIRECT3DTEXTURE, LPDIRECT3DTEXTURE);
typedef HRESULT (WINAPI *SwapTextureHandles2_Type)(void *, LPDIRECT3DTEXTURE2, LPDIRECT3DTEXTURE2);

QueryInterfaceD3_Type pQueryInterfaceD3D = NULL;
D3DInitialize_Type pD3DInitialize = NULL;
D3DGetCaps_Type pD3DGetCaps = NULL;
D3DGetCaps3_Type pGetCaps3 = NULL;
AddViewport1_Type pAddViewport1 = NULL;
AddViewport2_Type pAddViewport2 = NULL;
AddViewport3_Type pAddViewport3 = NULL;
Scene3_Type pBeginScene1 = NULL;
Scene3_Type pBeginScene2 = NULL;
Scene3_Type pBeginScene3 = NULL;
Scene3_Type pBeginScene7 = NULL;
Scene3_Type pEndScene1 = NULL;
Scene3_Type pEndScene2 = NULL;
Scene3_Type pEndScene3 = NULL;
Scene3_Type pEndScene7 = NULL;
SetRenderState3_Type pSetRenderState2 = NULL;
SetRenderState3_Type pSetRenderState3 = NULL;
SetRenderState3_Type pSetRenderState7 = NULL;
GetViewport7_Type pGetViewport7 = NULL;
SetViewport7_Type pSetViewport7 = NULL;
SetLightState_Type pSetLightState3 = NULL;
GetCurrentViewport2_Type pGetCurrentViewport2 = NULL;
SetCurrentViewport2_Type pSetCurrentViewport2 = NULL;
GetCurrentViewport3_Type pGetCurrentViewport3 = NULL;
SetCurrentViewport3_Type pSetCurrentViewport3 = NULL;
SetTexture3_Type pSetTexture3 = NULL;
SetTexture7_Type pSetTexture7 = NULL;
SwapTextureHandles_Type pSwapTextureHandles = NULL;
SwapTextureHandles2_Type pSwapTextureHandles2 = NULL;

// IDirect3DViewport-n interfaces

typedef HRESULT (WINAPI *InitializeVP_Type)(void *, LPDIRECT3D);
typedef HRESULT (WINAPI *SetViewport_Type)(void *, LPD3DVIEWPORT);
typedef HRESULT (WINAPI *SetMaterial_Type)(void *, LPD3DMATERIAL);
typedef HRESULT (WINAPI *GetMaterial_Type)(void *, LPD3DMATERIAL);
typedef HRESULT (WINAPI *GetViewport_Type)(void *, LPD3DVIEWPORT);
typedef HRESULT (WINAPI *GetViewport2_Type)(void *, LPD3DVIEWPORT);
typedef HRESULT (WINAPI *SetViewport2_Type)(void *, LPD3DVIEWPORT);
typedef HRESULT (WINAPI *GetViewport3_Type)(void *, LPD3DVIEWPORT);
typedef HRESULT (WINAPI *SetViewport3_Type)(void *, LPD3DVIEWPORT);
typedef HRESULT (WINAPI *GetViewport2_3_Type)(void *, LPD3DVIEWPORT2);
typedef HRESULT (WINAPI *SetViewport2_3_Type)(void *, LPD3DVIEWPORT2);
typedef HRESULT (WINAPI *DeleteViewport1_Type)(void *, LPDIRECT3DVIEWPORT);
typedef HRESULT (WINAPI *NextViewport1_Type)(void *, LPDIRECT3DVIEWPORT, LPDIRECT3DVIEWPORT *, DWORD);
typedef HRESULT (WINAPI *DeleteViewport2_Type)(void *, LPDIRECT3DVIEWPORT2);
typedef HRESULT (WINAPI *NextViewport2_Type)(void *, LPDIRECT3DVIEWPORT2, LPDIRECT3DVIEWPORT2 *, DWORD);
typedef HRESULT (WINAPI *ViewportClear_Type)(void *, DWORD, LPD3DRECT, DWORD);

InitializeVP_Type pInitializeVP = NULL;
SetViewport_Type pSetViewport = NULL;
SetMaterial_Type pSetMaterial = NULL;
GetMaterial_Type pGetMaterial = NULL;
GetViewport_Type pGetViewport = NULL;
GetViewport2_Type pGetViewport2 = NULL;
SetViewport2_Type pSetViewport2 = NULL;
GetViewport2_3_Type pGetViewport2_3 = NULL;
SetViewport2_3_Type pSetViewport2_3 = NULL;
GetViewport3_Type pGetViewport3 = NULL;
SetViewport3_Type pSetViewport3 = NULL;
DeleteViewport1_Type pDeleteViewport1 = NULL;
NextViewport1_Type pNextViewport1 = NULL;
DeleteViewport2_Type pDeleteViewport2 = NULL;
NextViewport2_Type pNextViewport2 = NULL;
ViewportClear_Type pViewportClear = NULL;

HRESULT WINAPI extInitialize(void *);
HRESULT WINAPI extEnumDevices(void *, LPD3DENUMDEVICESCALLBACK, LPVOID);
HRESULT WINAPI extEnumDevices7(void *, LPD3DENUMDEVICESCALLBACK7, LPVOID);
HRESULT WINAPI extCreateLight(void *, LPDIRECT3DLIGHT *, IUnknown *);
HRESULT WINAPI extCreateMaterial(void *, LPDIRECT3DMATERIAL *, IUnknown *);
HRESULT WINAPI extCreateViewport1(void *, LPDIRECT3DVIEWPORT *, IUnknown *);
HRESULT WINAPI extCreateViewport2(void *, LPDIRECT3DVIEWPORT2 *, IUnknown *);
HRESULT WINAPI extCreateViewport3(void *, LPDIRECT3DVIEWPORT3 *, IUnknown *);
HRESULT WINAPI extFindDevice(void *, LPD3DFINDDEVICESEARCH, LPD3DFINDDEVICERESULT);
HRESULT WINAPI extCreateDevice2(void *, REFCLSID, LPDIRECTDRAWSURFACE, LPDIRECT3DDEVICE2 *);
HRESULT WINAPI extCreateDevice3(void *, REFCLSID, LPDIRECTDRAWSURFACE4, LPDIRECT3DDEVICE3 *, LPUNKNOWN);
HRESULT WINAPI extCreateDevice7(void *, REFCLSID, LPDIRECTDRAWSURFACE7, LPDIRECT3DDEVICE7 *);
HRESULT WINAPI extDeleteViewport1(void *, LPDIRECT3DVIEWPORT);
HRESULT WINAPI extNextViewport1(void *, LPDIRECT3DVIEWPORT, LPDIRECT3DVIEWPORT *, DWORD);
HRESULT WINAPI extDeleteViewport2(void *, LPDIRECT3DVIEWPORT2);
HRESULT WINAPI extNextViewport2(void *, LPDIRECT3DVIEWPORT2, LPDIRECT3DVIEWPORT2 *, DWORD);
HRESULT WINAPI extViewportClear(void *, DWORD, LPD3DRECT, DWORD);

HRESULT WINAPI extInitializeVP(void *, LPDIRECT3D);
HRESULT WINAPI extSetViewport(void *, LPD3DVIEWPORT);
HRESULT WINAPI extGetViewport(void *, LPD3DVIEWPORT);
HRESULT WINAPI extSetMaterial(void *, LPD3DMATERIAL);
HRESULT WINAPI extGetMaterial(void *, LPD3DMATERIAL);
HRESULT WINAPI extQueryInterfaceD3(void *, REFIID, LPVOID *);
HRESULT WINAPI extQueryInterfaceD3D(void *, REFIID, LPVOID *);

HRESULT WINAPI extD3DInitialize(void *, LPDIRECT3D , LPGUID, LPD3DDEVICEDESC);
HRESULT WINAPI extD3DGetCaps(void *, LPD3DDEVICEDESC ,LPD3DDEVICEDESC);

HRESULT WINAPI extBeginScene1(void *);
HRESULT WINAPI extEndScene1(void *);
HRESULT WINAPI extBeginScene2(void *);
HRESULT WINAPI extEndScene2(void *);
HRESULT WINAPI extBeginScene3(void *);
HRESULT WINAPI extEndScene3(void *);
HRESULT WINAPI extBeginScene7(void *);
HRESULT WINAPI extEndScene7(void *);
HRESULT WINAPI extSetRenderState2(void *, D3DRENDERSTATETYPE, DWORD);
HRESULT WINAPI extSetRenderState3(void *, D3DRENDERSTATETYPE, DWORD);
HRESULT WINAPI extSetRenderState7(void *, D3DRENDERSTATETYPE, DWORD);
HRESULT WINAPI extGetCaps3(void *, LPD3DDEVICEDESC, LPD3DDEVICEDESC);
HRESULT WINAPI extSetLightState3(void *d3dd, D3DLIGHTSTATETYPE d3dls, DWORD t);
HRESULT WINAPI extSetViewport3(void *, LPD3DVIEWPORT);
HRESULT WINAPI extGetViewport3(void *, LPD3DVIEWPORT);
HRESULT WINAPI extAddViewport1(void *, LPDIRECT3DVIEWPORT);
HRESULT WINAPI extAddViewport2(void *, LPDIRECT3DVIEWPORT2);
HRESULT WINAPI extAddViewport3(void *, LPDIRECT3DVIEWPORT3);
HRESULT WINAPI extGetViewport2(void *, LPD3DVIEWPORT);
HRESULT WINAPI extSetViewport2(void *, LPD3DVIEWPORT);
HRESULT WINAPI extGetViewport2_3(void *, LPD3DVIEWPORT2);
HRESULT WINAPI extSetViewport2_3(void *, LPD3DVIEWPORT2);
HRESULT WINAPI extSetCurrentViewport2(void *, LPDIRECT3DVIEWPORT2);
HRESULT WINAPI extGetCurrentViewport2(void *, LPDIRECT3DVIEWPORT2 *);
HRESULT WINAPI extSetCurrentViewport3(void *, LPDIRECT3DVIEWPORT3);
HRESULT WINAPI extGetCurrentViewport3(void *, LPDIRECT3DVIEWPORT3 *);
HRESULT WINAPI extSetViewport7(void *, LPD3DVIEWPORT7);
HRESULT WINAPI extGetViewport7(void *, LPD3DVIEWPORT7);
HRESULT WINAPI extSetTexture3(void *, DWORD, LPDIRECT3DTEXTURE2);
HRESULT WINAPI extSetTexture7(void *, DWORD, LPDIRECTDRAWSURFACE7);
HRESULT WINAPI extSwapTextureHandles(void *, LPDIRECT3DTEXTURE, LPDIRECT3DTEXTURE);
HRESULT WINAPI extSwapTextureHandles2(void *, LPDIRECT3DTEXTURE2, LPDIRECT3DTEXTURE2);

// Texture

typedef HRESULT (WINAPI *TexInitialize_Type)(void *, LPDIRECT3DDEVICE, LPDIRECTDRAWSURFACE);
typedef HRESULT (WINAPI *TexGetHandle_Type)(void *, LPDIRECT3DDEVICE, LPD3DTEXTUREHANDLE);
typedef HRESULT (WINAPI *TexPaletteChanged_Type)(void *, DWORD, DWORD);
typedef HRESULT (WINAPI *TexLoad_Type)(void *, LPDIRECT3DTEXTURE);
typedef HRESULT (WINAPI *TexUnload_Type)(void *);

TexInitialize_Type pTInitialize = NULL;
TexGetHandle_Type pTGetHandle = NULL;
TexPaletteChanged_Type pTPaletteChanged = NULL;
TexLoad_Type pTLoad = NULL;
TexUnload_Type pTUnload = NULL;

HRESULT WINAPI extTexInitialize(void *, LPDIRECT3DDEVICE, LPDIRECTDRAWSURFACE);
HRESULT WINAPI extTexGetHandle(void *, LPDIRECT3DDEVICE, LPD3DTEXTUREHANDLE);
HRESULT WINAPI extTexPaletteChanged(void *, DWORD, DWORD);
HRESULT WINAPI extTexLoad(void *, LPDIRECT3DTEXTURE);
HRESULT WINAPI extTexUnload(void *);

extern char *ExplainDDError(DWORD);
int GD3DDeviceVersion;

static char *sFourCC(DWORD fcc)
{
	static char sRet[5];
	char c;
	int i;
	char *t=&sRet[0];
	for(i=0; i<4; i++){
		c = fcc & (0xFF);
		*t++ = isprint(c) ? c : '.';
		c = c >> 8;
	}
	*t = 0;
	return sRet;
}

char *DumpPixelFormat(LPDDPIXELFORMAT ddpfPixelFormat)
{
	static char sBuf[512];
	char sItem[256];
	DWORD flags=ddpfPixelFormat->dwFlags;
	sprintf(sBuf, " PixelFormat flags=%x(%s) BPP=%d", 
		flags, ExplainPixelFormatFlags(flags), ddpfPixelFormat->dwRGBBitCount);
	if (flags & DDPF_RGB) {
		if (flags & DDPF_ALPHAPIXELS) {
			sprintf(sItem, " RGBA=(%x,%x,%x,%x)", 
				ddpfPixelFormat->dwRBitMask,
				ddpfPixelFormat->dwGBitMask,
				ddpfPixelFormat->dwBBitMask,
				ddpfPixelFormat->dwRGBAlphaBitMask);
		}
		else {
			sprintf(sItem, " RGB=(%x,%x,%x)", 
				ddpfPixelFormat->dwRBitMask,
				ddpfPixelFormat->dwGBitMask,
				ddpfPixelFormat->dwBBitMask);
		}
		strcat(sBuf, sItem);
	}
	if (flags & DDPF_YUV) {
		sprintf(sItem, " YUVA=(%x,%x,%x,%x)", 
			ddpfPixelFormat->dwYBitMask,
			ddpfPixelFormat->dwUBitMask,
			ddpfPixelFormat->dwVBitMask,
			ddpfPixelFormat->dwYUVAlphaBitMask);
		strcat(sBuf, sItem);
	}
	if (flags & DDPF_ZBUFFER) {
		sprintf(sItem, " SdZSbL=(%x,%x,%x,%x)", 
			ddpfPixelFormat->dwStencilBitDepth,
			ddpfPixelFormat->dwZBitMask,
			ddpfPixelFormat->dwStencilBitMask,
			ddpfPixelFormat->dwLuminanceAlphaBitMask);
		strcat(sBuf, sItem);
	}
	if (flags & DDPF_ALPHA) {
		sprintf(sItem, " LBdBlZ=(%x,%x,%x,%x)", 
			ddpfPixelFormat->dwLuminanceBitMask,
			ddpfPixelFormat->dwBumpDvBitMask,
			ddpfPixelFormat->dwBumpLuminanceBitMask,
			ddpfPixelFormat->dwRGBZBitMask);
		strcat(sBuf, sItem);
	}
	if (flags & DDPF_LUMINANCE) {
		sprintf(sItem, " BMbMF=(%x,%x,%x,%x)", 
			ddpfPixelFormat->dwBumpDuBitMask,
			ddpfPixelFormat->MultiSampleCaps.wBltMSTypes,
			ddpfPixelFormat->MultiSampleCaps.wFlipMSTypes,
			ddpfPixelFormat->dwYUVZBitMask);
		strcat(sBuf, sItem);
	}
	if (flags & DDPF_BUMPDUDV) {
		sprintf(sItem, " O=(%x)", 
			ddpfPixelFormat->dwOperations);
		strcat(sBuf, sItem);
	}
	if (flags & DDPF_FOURCC) {
		sprintf(sItem, " FourCC=%x(%s)", 
			ddpfPixelFormat->dwFourCC, sFourCC(ddpfPixelFormat->dwFourCC));
		strcat(sBuf, sItem);
	}
	return sBuf;
}

int HookDirect3D7(HMODULE module, int version){
	void *tmp;
	HINSTANCE hinst;
	LPDIRECT3D lpd3d=NULL;
	HRESULT res;

	switch(version){
	case 0:
	case 1:
		tmp = HookAPI(module, "d3dim.dll", NULL, "Direct3DCreate", extDirect3DCreate);
		if(tmp) pDirect3DCreate = (Direct3DCreate_Type)tmp;
		tmp = HookAPI(module, "d3dim.dll", NULL, "Direct3DCreateDevice", extDirect3DCreateDevice);
		if(tmp) pDirect3DCreateDevice = (Direct3DCreateDevice_Type)tmp;
		tmp = HookAPI(module, "d3dim700.dll", NULL, "Direct3DCreate", extDirect3DCreate);
		if(tmp) pDirect3DCreate = (Direct3DCreate_Type)tmp;
		tmp = HookAPI(module, "d3dim700.dll", NULL, "Direct3DCreateDevice", extDirect3DCreateDevice);
		if(tmp) pDirect3DCreateDevice = (Direct3DCreateDevice_Type)tmp;
		break;
	case 7:
		hinst = (*pLoadLibraryA)("d3dim.dll");
		if (hinst){
			pDirect3DCreate =
				(Direct3DCreate_Type)(*pGetProcAddress)(hinst, "Direct3DCreate");
			if(pDirect3DCreate){
				res = extDirect3DCreate(0x0700, &lpd3d, NULL);
				if(res == DD_OK) lpd3d->Release();
			}
		}
		else {
			hinst = (*pLoadLibraryA)("d3dim700.dll");
			pDirect3DCreate =
				(Direct3DCreate_Type)(*pGetProcAddress)(hinst, "Direct3DCreate");
			if(pDirect3DCreate){
				res = extDirect3DCreate(0x0700, &lpd3d, NULL);
				if(res == DD_OK) lpd3d->Release();
			}
		}
		break;
	}
	return 0;
}

FARPROC Remap_d3d7_ProcAddress(LPCSTR proc, HMODULE hModule)
{
	if (!strcmp(proc,"Direct3DCreate") && !pDirect3DCreate){
		pDirect3DCreate=(Direct3DCreate_Type)(*pGetProcAddress)(hModule, proc);
		OutTraceD3D("GetProcAddress: hooking proc=%s at addr=%x\n", ProcToString(proc), pDirect3DCreate);
		return (FARPROC)extDirect3DCreate;
	}
	if (!strcmp(proc,"Direct3DCreateDevice") && !pDirect3DCreateDevice){
		pDirect3DCreateDevice=(Direct3DCreateDevice_Type)(*pGetProcAddress)(hModule, proc);
		OutTraceD3D("GetProcAddress: hooking proc=%s at addr=%x\n", ProcToString(proc), pDirect3DCreateDevice);
		return (FARPROC)extDirect3DCreateDevice;
	}
	// NULL -> keep the original call address
	return NULL;
}

void HookDirect3DSession(LPDIRECTDRAW *lplpdd, int d3dversion)
{
	OutTraceD3D("HookDirect3DSession: d3d=%x d3dversion=%d\n", *lplpdd, d3dversion);

	switch(d3dversion){
	case 1:
		SetHook((void *)(**(DWORD **)lplpdd +   0), extQueryInterfaceD3, (void **)&pQueryInterfaceD3, "QueryInterface(D3S)");
		SetHook((void *)(**(DWORD **)lplpdd +  12), extInitialize, (void **)&pInitialize, "Initialize(1)");
		SetHook((void *)(**(DWORD **)lplpdd +  16), extEnumDevices, (void **)&pEnumDevices, "EnumDevices");
		SetHook((void *)(**(DWORD **)lplpdd +  20), extCreateLight, (void **)&pCreateLight, "CreateLight");
		SetHook((void *)(**(DWORD **)lplpdd +  24), extCreateMaterial, (void **)&pCreateMaterial, "CreateMaterial");
		SetHook((void *)(**(DWORD **)lplpdd +  28), extCreateViewport1, (void **)&pCreateViewport1, "CreateViewport(1)");
		SetHook((void *)(**(DWORD **)lplpdd +  32), extFindDevice, (void **)&pFindDevice, "FindDevice");	
		break;
	case 2:
		SetHook((void *)(**(DWORD **)lplpdd +   0), extQueryInterfaceD3, (void **)&pQueryInterfaceD3, "QueryInterface(D3S)");
		SetHook((void *)(**(DWORD **)lplpdd +  12), extEnumDevices, (void **)&pEnumDevices, "EnumDevices(2)");
		SetHook((void *)(**(DWORD **)lplpdd +  16), extCreateLight, (void **)&pCreateLight, "CreateLight");
		SetHook((void *)(**(DWORD **)lplpdd +  20), extCreateMaterial, (void **)&pCreateMaterial, "CreateMaterial");
		SetHook((void *)(**(DWORD **)lplpdd +  24), extCreateViewport2, (void **)&pCreateViewport2, "CreateViewport(2)");
		SetHook((void *)(**(DWORD **)lplpdd +  28), extFindDevice, (void **)&pFindDevice, "FindDevice");
		SetHook((void *)(**(DWORD **)lplpdd +  32), extCreateDevice2, (void **)&pCreateDevice2, "CreateDevice(D3D2)");
		break;
	case 3:
		SetHook((void *)(**(DWORD **)lplpdd +   0), extQueryInterfaceD3, (void **)&pQueryInterfaceD3, "QueryInterface(D3S)");
		SetHook((void *)(**(DWORD **)lplpdd +  12), extEnumDevices, (void **)&pEnumDevices, "EnumDevices(3)");
		SetHook((void *)(**(DWORD **)lplpdd +  16), extCreateLight, (void **)&pCreateLight, "CreateLight");
		SetHook((void *)(**(DWORD **)lplpdd +  20), extCreateMaterial, (void **)&pCreateMaterial, "CreateMaterial");
		SetHook((void *)(**(DWORD **)lplpdd +  24), extCreateViewport3, (void **)&pCreateViewport3, "CreateViewport(3)");
		SetHook((void *)(**(DWORD **)lplpdd +  28), extFindDevice, (void **)&pFindDevice, "FindDevice");
		SetHook((void *)(**(DWORD **)lplpdd +  32), extCreateDevice3, (void **)&pCreateDevice3, "CreateDevice(D3D3)");
		SetHook((void *)(**(DWORD **)lplpdd +  40), extEnumZBufferFormats, (void **)&pEnumZBufferFormats, "EnumZBufferFormats(D3D3)");
		break;
	case 7:
		SetHook((void *)(**(DWORD **)lplpdd +   0), extQueryInterfaceD3, (void **)&pQueryInterfaceD3, "QueryInterface(D3S)");
		SetHook((void *)(**(DWORD **)lplpdd +  12), extEnumDevices7, (void **)&pEnumDevices7, "EnumDevices(7)");
		SetHook((void *)(**(DWORD **)lplpdd +  16), extCreateDevice7, (void **)&pCreateDevice7, "CreateDevice(D3D7)");
		break;
	}
} 

void HookDirect3DDevice(void **lpd3ddev, int d3dversion)
{
	OutTraceD3D("HookDirect3DDevice: d3ddev=%x d3dversion=%d\n", lpd3ddev, d3dversion);
	GD3DDeviceVersion = d3dversion;

	switch(d3dversion){
	case 1:
		//SetHook((void *)(**(DWORD **)lpd3ddev +   0), extQueryInterfaceD3D, (void **)&pQueryInterfaceD3D, "QueryInterface(D3D)");
		//SetHook((void *)(**(DWORD **)lpd3ddev +  16), extGetCaps1, (void **)&pGetCaps1, "GetCaps(1)");
		SetHook((void *)(**(DWORD **)lpd3ddev +  20), extSwapTextureHandles, (void **)&pSwapTextureHandles, "SwapTextureHandles(1)");
		SetHook((void *)(**(DWORD **)lpd3ddev +  36), extAddViewport1, (void **)&pAddViewport1, "AddViewport(1)");
		SetHook((void *)(**(DWORD **)lpd3ddev +  40), extDeleteViewport1, (void **)&pDeleteViewport1, "DeleteViewport(1)");
		SetHook((void *)(**(DWORD **)lpd3ddev +  44), extNextViewport1, (void **)&pNextViewport1, "NextViewport(1)");
		SetHook((void *)(**(DWORD **)lpd3ddev +  76), extBeginScene1, (void **)&pBeginScene1, "BeginScene(1)");
		SetHook((void *)(**(DWORD **)lpd3ddev +  80), extEndScene1, (void **)&pEndScene1, "EndScene(1)");
		break;
	case 2:
		//SetHook((void *)(**(DWORD **)lpd3ddev +   0), extQueryInterfaceD3D, (void **)&pQueryInterfaceD3D, "QueryInterface(D3D)");
		//SetHook((void *)(**(DWORD **)lpd3ddev +  12), extGetCaps2, (void **)&pGetCaps2, "GetCaps(2)");
		SetHook((void *)(**(DWORD **)lpd3ddev +  16), extSwapTextureHandles, (void **)&pSwapTextureHandles, "SwapTextureHandles(1)");
		SetHook((void *)(**(DWORD **)lpd3ddev +  24), extAddViewport2, (void **)&pAddViewport2, "AddViewport(2)");
		SetHook((void *)(**(DWORD **)lpd3ddev +  28), extDeleteViewport2, (void **)&pDeleteViewport2, "DeleteViewport(2)");
		SetHook((void *)(**(DWORD **)lpd3ddev +  32), extNextViewport2, (void **)&pNextViewport2, "NextViewport(2)");
		SetHook((void *)(**(DWORD **)lpd3ddev +  40), extBeginScene2, (void **)&pBeginScene2, "BeginScene(2)");
		SetHook((void *)(**(DWORD **)lpd3ddev +  44), extEndScene2, (void **)&pEndScene2, "EndScene(2)");
		SetHook((void *)(**(DWORD **)lpd3ddev +  52), extSetCurrentViewport2, (void **)&pSetCurrentViewport2, "SetCurrentViewport(2)");
		SetHook((void *)(**(DWORD **)lpd3ddev +  56), extGetCurrentViewport2, (void **)&pGetCurrentViewport2, "GetCurrentViewport(2)");
		SetHook((void *)(**(DWORD **)lpd3ddev +  92), extSetRenderState2, (void **)&pSetRenderState2, "SetRenderState(2)");
		if(pSetRenderState2){
			if(dxw.dwFlags2 & WIREFRAME)(*pSetRenderState2)(*lpd3ddev, D3DRENDERSTATE_FILLMODE, D3DFILL_WIREFRAME); 		
			if(dxw.dwFlags4 & DISABLEFOGGING) (*pSetRenderState2)(*lpd3ddev, D3DRENDERSTATE_FOGENABLE, FALSE); 
			if(dxw.dwFlags4 & ZBUFFERALWAYS) (*pSetRenderState2)(*lpd3ddev, D3DRENDERSTATE_ZFUNC, D3DCMP_ALWAYS);
		}		
		break;
	case 3:
		//SetHook((void *)(**(DWORD **)lpd3ddev +   0), extQueryInterfaceD3D, (void **)&pQueryInterfaceD3D, "QueryInterface(D3D)");
		SetHook((void *)(**(DWORD **)lpd3ddev +  12), extGetCaps3, (void **)&pGetCaps3, "GetCaps(3)");
		SetHook((void *)(**(DWORD **)lpd3ddev +  20), extAddViewport3, (void **)&pAddViewport3, "AddViewport(3)");
		SetHook((void *)(**(DWORD **)lpd3ddev +  36), extBeginScene3, (void **)&pBeginScene3, "BeginScene(3)");
		SetHook((void *)(**(DWORD **)lpd3ddev +  40), extEndScene3, (void **)&pEndScene3, "EndScene(3)");
		SetHook((void *)(**(DWORD **)lpd3ddev +  48), extSetCurrentViewport3, (void **)&pSetCurrentViewport3, "SetCurrentViewport(3)");
		SetHook((void *)(**(DWORD **)lpd3ddev +  52), extGetCurrentViewport3, (void **)&pGetCurrentViewport3, "GetCurrentViewport(3)");
		SetHook((void *)(**(DWORD **)lpd3ddev +  88), extSetRenderState3, (void **)&pSetRenderState3, "SetRenderState(3)");
		SetHook((void *)(**(DWORD **)lpd3ddev +  96), extSetLightState3, (void **)&pSetLightState3, "SetLightState(3)");
		if (dxw.dwFlags4 & NOTEXTURES) SetHook((void *)(**(DWORD **)lpd3ddev + 152), extSetTexture3, (void **)&pSetTexture3, "SetTexture(D3)");
		if(pSetRenderState3){
			if(dxw.dwFlags2 & WIREFRAME)(*pSetRenderState3)(*lpd3ddev, D3DRENDERSTATE_FILLMODE, D3DFILL_WIREFRAME); 		
			if(dxw.dwFlags4 & DISABLEFOGGING) (*pSetRenderState3)(*lpd3ddev, D3DRENDERSTATE_FOGENABLE, FALSE); 
			if(dxw.dwFlags4 & ZBUFFERALWAYS) (*pSetRenderState3)(*lpd3ddev, D3DRENDERSTATE_ZFUNC, D3DCMP_ALWAYS);
		}		
		break;
	case 7:
		//SetHook((void *)(**(DWORD **)lpd3ddev +   0), extQueryInterfaceD3D, (void **)&pQueryInterfaceD3D, "QueryInterface(D3D)");
		//SetHook((void *)(**(DWORD **)lpd3ddev +  20), extBeginScene7, (void **)&pBeginScene7, "BeginScene(7)");
		//SetHook((void *)(**(DWORD **)lpd3ddev +  24), extEndScene7, (void **)&pEndScene7, "EndScene(7)");
		//SetHook((void *)(**(DWORD **)lpd3ddev +  52), extSetViewport7, (void **)&pSetViewport7, "SetViewport(7)");
		//SetHook((void *)(**(DWORD **)lpd3ddev +  60), extGetViewport7, (void **)&pGetViewport7, "GetViewport(7)");
		SetHook((void *)(**(DWORD **)lpd3ddev +  80), extSetRenderState7, (void **)&pSetRenderState7, "SetRenderState(7)");
		if (dxw.dwFlags4 & NOTEXTURES) SetHook((void *)(**(DWORD **)lpd3ddev + 140), extSetTexture7, (void **)&pSetTexture7, "SetTexture(D7)");
		if(pSetRenderState7){
			if(dxw.dwFlags2 & WIREFRAME)(*pSetRenderState7)(*lpd3ddev, D3DRENDERSTATE_FILLMODE, D3DFILL_WIREFRAME); 		
			if(dxw.dwFlags4 & DISABLEFOGGING) (*pSetRenderState7)(*lpd3ddev, D3DRENDERSTATE_FOGENABLE, FALSE); 
			if(dxw.dwFlags4 & ZBUFFERALWAYS) (*pSetRenderState7)(*lpd3ddev, D3DRENDERSTATE_ZFUNC, D3DCMP_ALWAYS);
		}		
		break;
	}
} 

HRESULT WINAPI extDirect3DCreate(UINT SDKVersion, LPDIRECT3D *lplpd3d, LPUNKNOWN pUnkOuter)
{
	HRESULT res;
	UINT d3dversion; 

	OutTraceD3D("Direct3DCreate: SDKVersion=%x UnkOuter=%x\n", SDKVersion, pUnkOuter);
	res=(*pDirect3DCreate)(SDKVersion, lplpd3d, pUnkOuter);

	if(res) {
		OutTraceE("Direct3DCreate ERROR: err=%x(%s) at %d\n", res, ExplainDDError(res), __LINE__);
		return res;
	}

	d3dversion = 1;
	if(SDKVersion >= 0x0500) d3dversion = 2;
	if(SDKVersion >= 0x0600) d3dversion = 3;
	if(SDKVersion >= 0x0700) d3dversion = 7;

	HookDirect3DSession((LPDIRECTDRAW *)lplpd3d, d3dversion);
	OutTraceD3D("Direct3DCreate: d3d=%x\n", *lplpd3d);
	return res;
}

HRESULT WINAPI extDirect3DCreateDevice(GUID FAR *lpGUID, LPDIRECT3D lpd3ddevice, LPDIRECTDRAWSURFACE surf, LPDIRECT3D *lplpd3ddevice, LPUNKNOWN pUnkOuter)
{
	HRESULT res;

	OutTraceD3D("Direct3DCreateDevice: guid=%x d3ddevice=%x dds=%x%s UnkOuter=%x\n",
		lpGUID, lpd3ddevice, surf, dxwss.ExplainSurfaceRole(surf), pUnkOuter);
	res=(*pDirect3DCreateDevice)(lpGUID, lpd3ddevice, surf, lplpd3ddevice, pUnkOuter);
	if(res) OutTraceE("Direct3DCreateDevice ERROR: err=%x(%s) at %d\n", res, ExplainDDError(res), __LINE__);
	else OutTraceD3D("Direct3DCreateDevice: d3ddevice=%x\n", *lplpd3ddevice);

	switch(lpGUID->Data1){
	case 0x64108800: // IID_IDirect3DDevice
		HookDirect3DDevice((void **)lplpd3ddevice, 1);
		break;
	case 0x93281501: // IID_IDirect3DDevice2
		HookDirect3DDevice((void **)lplpd3ddevice, 2);
		break;
	case 0xb0ab3b60: // IID_IDirect3DDevice3
		HookDirect3DDevice((void **)lplpd3ddevice, 3);
		break;
	case 0xf5049e79: // IID_IDirect3DDevice7
		HookDirect3DDevice((void **)lplpd3ddevice, 7);
		break;
	}
	return res;
}

void HookViewport(LPDIRECT3DVIEWPORT *lpViewport, int d3dversion)
{
	OutTraceD3D("HookViewport: Viewport=%x d3dversion=%d\n", *lpViewport, d3dversion);

 	switch(d3dversion){
	case 1:
		SetHook((void *)(**(DWORD **)lpViewport +  12), extInitializeVP, (void **)&pInitializeVP, "Initialize(VP1)");
		SetHook((void *)(**(DWORD **)lpViewport +  16), extGetViewport, (void **)&pGetViewport, "GetViewport(1)");
		SetHook((void *)(**(DWORD **)lpViewport +  20), extSetViewport, (void **)&pSetViewport, "SetViewport(1)");

		// to do: why Clear method crashes in "Forsaken" in emulation and GDI mode???
		// SetHook((void *)(**(DWORD **)lpViewport +  48), extViewportClear, (void **)&pViewportClear, "Clear(1)");
		break;
	case 2:
		SetHook((void *)(**(DWORD **)lpViewport +  12), extInitializeVP, (void **)&pInitializeVP, "Initialize(VP2)");
		SetHook((void *)(**(DWORD **)lpViewport +  16), extGetViewport, (void **)&pGetViewport, "GetViewport(2)");
		SetHook((void *)(**(DWORD **)lpViewport +  20), extSetViewport, (void **)&pSetViewport, "SetViewport(2)");
		SetHook((void *)(**(DWORD **)lpViewport +  64), extGetViewport2_3, (void **)&pGetViewport2_3, "GetViewport2(2)");
		SetHook((void *)(**(DWORD **)lpViewport +  68), extSetViewport2_3, (void **)&pSetViewport2_3, "SetViewport2(2)");
		break;
	case 3:
		SetHook((void *)(**(DWORD **)lpViewport +  12), extInitializeVP, (void **)&pInitializeVP, "Initialize(VP3)");
		SetHook((void *)(**(DWORD **)lpViewport +  16), extGetViewport, (void **)&pGetViewport, "GetViewport(3)");
		SetHook((void *)(**(DWORD **)lpViewport +  20), extSetViewport, (void **)&pSetViewport, "SetViewport(3)");
		SetHook((void *)(**(DWORD **)lpViewport +  64), extGetViewport2_3, (void **)&pGetViewport2_3, "GetViewport2(3)");
		SetHook((void *)(**(DWORD **)lpViewport +  68), extSetViewport2_3, (void **)&pSetViewport2_3, "SetViewport2(3)");
		break;
	case 7:
		break;
	}
}

void HookMaterial(LPDIRECT3DMATERIAL *lpMaterial, int d3dversion)
{
	OutTraceD3D("HookMaterial: Material=%x d3dversion=%d\n", *lpMaterial, d3dversion);

 	switch(d3dversion){
	case 1:
		SetHook((void *)(**(DWORD **)lpMaterial +  16), extSetMaterial, (void **)&pSetMaterial, "SetMaterial");
		SetHook((void *)(**(DWORD **)lpMaterial +  20), extGetMaterial, (void **)&pGetMaterial, "GetMaterial");
		break;
	default:
		SetHook((void *)(**(DWORD **)lpMaterial +  12), extSetMaterial, (void **)&pSetMaterial, "SetMaterial");
		SetHook((void *)(**(DWORD **)lpMaterial +  16), extGetMaterial, (void **)&pGetMaterial, "GetMaterial");
		break;
	}
}

void HookTexture(LPVOID *lpTexture, int version)
{
	OutTraceD3D("HookTexture: Texture=%x version=%d\n", *lpTexture, version);
 	switch(version){
	case 1:
		SetHook((void *)(**(DWORD **)lpTexture +  12), extTexInitialize, (void **)&pTInitialize, "Initialize(T1)");
		SetHook((void *)(**(DWORD **)lpTexture +  16), extTexGetHandle, (void **)&pTGetHandle, "GetHandle(T1)");
		SetHook((void *)(**(DWORD **)lpTexture +  20), extTexPaletteChanged, (void **)&pTPaletteChanged, "PaletteChanged(T1)");
		SetHook((void *)(**(DWORD **)lpTexture +  24), extTexLoad, (void **)&pTLoad, "Load(T1)");
		SetHook((void *)(**(DWORD **)lpTexture +  28), extTexUnload, (void **)&pTUnload, "Unload(T1)");
		break;
	case 2:
		SetHook((void *)(**(DWORD **)lpTexture +  12), extTexGetHandle, (void **)&pTGetHandle, "GetHandle(T2)");
		SetHook((void *)(**(DWORD **)lpTexture +  16), extTexPaletteChanged, (void **)&pTPaletteChanged, "PaletteChanged(T2)");
		SetHook((void *)(**(DWORD **)lpTexture +  20), extTexLoad, (void **)&pTLoad, "Load(T2)");
		break;
	}
}

HRESULT WINAPI extQueryInterfaceD3(void *lpd3d, REFIID riid, LPVOID *ppvObj)
{
	HRESULT res;
	int d3dversion;

	OutTraceD3D("QueryInterface(D3): d3d=%x REFIID=%x obj=%x\n", lpd3d, riid.Data1, ppvObj);
	d3dversion=0;
	res=(*pQueryInterfaceD3)(lpd3d, riid, ppvObj);
	switch(riid.Data1){
		case 0x3BBA0080: d3dversion=1; break;
		case 0x6aae1ec1: d3dversion=2; break;
		case 0xbb223240: d3dversion=3; break;
		case 0xf5049e77: d3dversion=7; break;
	}
	if(d3dversion) OutTraceD3D("QueryInterface(D3): hooking version=%d\n", d3dversion);
	switch(d3dversion){
	case 1:
		SetHook((void *)(**(DWORD **)ppvObj +  12), extInitialize, (void **)&pInitialize, "Initialize");
		SetHook((void *)(**(DWORD **)ppvObj +  16), extEnumDevices, (void **)&pEnumDevices, "EnumDevices");
		SetHook((void *)(**(DWORD **)ppvObj +  20), extCreateLight, (void **)&pCreateLight, "CreateLight");
		SetHook((void *)(**(DWORD **)ppvObj +  24), extCreateMaterial, (void **)&pCreateMaterial, "CreateMaterial");
		SetHook((void *)(**(DWORD **)ppvObj +  28), extCreateViewport1, (void **)&pCreateViewport1, "CreateViewport(1)");
		SetHook((void *)(**(DWORD **)ppvObj +  32), extFindDevice, (void **)&pFindDevice, "FindDevice");	
		break;
	case 2:
		SetHook((void *)(**(DWORD **)ppvObj +  12), extEnumDevices, (void **)&pEnumDevices, "EnumDevices");
		SetHook((void *)(**(DWORD **)ppvObj +  16), extCreateLight, (void **)&pCreateLight, "CreateLight");
		SetHook((void *)(**(DWORD **)ppvObj +  20), extCreateMaterial, (void **)&pCreateMaterial, "CreateMaterial");
		SetHook((void *)(**(DWORD **)ppvObj +  24), extCreateViewport2, (void **)&pCreateViewport2, "CreateViewport(2)"); 
		SetHook((void *)(**(DWORD **)ppvObj +  28), extFindDevice, (void **)&pFindDevice, "FindDevice");
		break;
	case 3:
		SetHook((void *)(**(DWORD **)ppvObj +  12), extEnumDevices, (void **)&pEnumDevices, "EnumDevices");
		SetHook((void *)(**(DWORD **)ppvObj +  16), extCreateLight, (void **)&pCreateLight, "CreateLight");
		SetHook((void *)(**(DWORD **)ppvObj +  20), extCreateMaterial, (void **)&pCreateMaterial, "CreateMaterial");
		SetHook((void *)(**(DWORD **)ppvObj +  24), extCreateViewport3, (void **)&pCreateViewport3, "CreateViewport(3)"); 
		SetHook((void *)(**(DWORD **)ppvObj +  28), extFindDevice, (void **)&pFindDevice, "FindDevice");
		break;
	case 7:
		SetHook((void *)(**(DWORD **)ppvObj +  12), extEnumDevices7, (void **)&pEnumDevices7, "EnumDevices(7)");
		SetHook((void *)(**(DWORD **)ppvObj +  32), extCreateDevice7, (void **)&pCreateDevice7, "CreateDevice(D3D7)");
		break;
	}
	return res;
}

HRESULT WINAPI extQueryInterfaceD3D(void *lpd3ddev, REFIID riid, LPVOID *ppvObj)
{
	HRESULT res;
	OutTraceD3D("QueryInterface(D3D): d3ddev=%x REFIID=%x obj=%x\n", lpd3ddev, riid.Data1, ppvObj);
	res=(*pQueryInterfaceD3)(lpd3ddev, riid, ppvObj);
	return res;
}

HRESULT WINAPI extInitialize(void *lpd3d)
{
	HRESULT res;

	OutTraceD3D("Initialize: d3d=%x\n", lpd3d);
	res=(*pInitialize)(lpd3d);
	if(res) OutTraceE("Initialize ERROR: err=%x(%s) at %d\n", res, ExplainDDError(res), __LINE__);
	else OutTraceD3D("Initialize: OK\n");
	return res;
}

typedef struct {
	LPD3DENUMDEVICESCALLBACK *cb;
	LPVOID arg;
} CallbackArg;

typedef struct {
	LPD3DENUMDEVICESCALLBACK7 *cb;
	LPVOID arg;
} CallbackArg7;

static void DumpD3DDevideDesc(LPD3DDEVICEDESC d3, char *label)
{
	if(IsTraceD3D){
		if(d3){
			OutTrace("EnumDevices: CALLBACK dev=%s Size=%d Flags=%x ", label, d3->dwSize, d3->dwFlags);
			if(d3->dwFlags & D3DDD_COLORMODEL) OutTrace("ColorModel=%x ", d3->dcmColorModel);
			if(d3->dwFlags & D3DDD_DEVCAPS) OutTrace("DevCaps=%x ", d3->dwDevCaps);
			if(d3->dwFlags & D3DDD_TRANSFORMCAPS) OutTrace("TransformCaps=%x ", d3->dtcTransformCaps.dwCaps);
			if(d3->dwFlags & D3DDD_LIGHTINGCAPS) OutTrace("LightingCaps=%x ", d3->dlcLightingCaps);
			if(d3->dwFlags & D3DDD_BCLIPPING) OutTrace("Clipping=%x ", d3->bClipping);
			if(d3->dwFlags & D3DDD_LINECAPS) OutTrace("LineCaps=%x ", d3->dpcLineCaps);
			if(d3->dwFlags & D3DDD_TRICAPS) OutTrace("TriCaps=%x ", d3->dpcTriCaps);
			if(d3->dwFlags & D3DDD_DEVICERENDERBITDEPTH) OutTrace("DeviceRenderBitDepth=%d ", d3->dwDeviceRenderBitDepth);
			if(d3->dwFlags & D3DDD_DEVICEZBUFFERBITDEPTH) OutTrace("DeviceZBufferBitDepth=%d ", d3->dwDeviceZBufferBitDepth);
			if(d3->dwFlags & D3DDD_MAXBUFFERSIZE) OutTrace("MaxBufferSize=%d ", d3->dwMaxBufferSize);
			if(d3->dwFlags & D3DDD_MAXVERTEXCOUNT) OutTrace("MaxVertexCount=%d ", d3->dwMaxVertexCount);
			OutTrace("Texture min=(%dx%d) max=(%dx%d)\n", d3->dwMinTextureWidth, d3->dwMinTextureHeight, d3->dwMaxTextureWidth, d3->dwMaxTextureHeight);
			//OutTrace("\n");
		}
		else
			OutTrace("EnumDevices: CALLBACK dev=%s ddesc=NULL\n", label);
	}
}

HRESULT WINAPI extDeviceProxy(GUID FAR *lpGuid, LPSTR lpDeviceDescription, LPSTR lpDeviceName, LPD3DDEVICEDESC lpd3ddd1, LPD3DDEVICEDESC lpd3ddd2, LPVOID arg)
{
	HRESULT res;
	OutTraceD3D("EnumDevices: CALLBACK GUID=%x(%s) DeviceDescription=\"%s\", DeviceName=\"%s\", arg=%x\n", lpGuid->Data1, ExplainGUID(lpGuid), lpDeviceDescription, lpDeviceName, ((CallbackArg *)arg)->arg);
	DumpD3DDevideDesc(lpd3ddd1, "HWDEV");
	DumpD3DDevideDesc(lpd3ddd2, "SWDEV");
	if(dxw.dwFlags4 & NOPOWER2FIX){ 
		D3DDEVICEDESC lpd3ddd1fix, lpd3ddd2fix;
		if(lpd3ddd1) memcpy(&lpd3ddd1fix, lpd3ddd1, sizeof(D3DDEVICEDESC));
		if(lpd3ddd2) memcpy(&lpd3ddd2fix, lpd3ddd2, sizeof(D3DDEVICEDESC));
		lpd3ddd1fix.dpcLineCaps.dwTextureCaps|=D3DPTEXTURECAPS_NONPOW2CONDITIONAL|D3DPTEXTURECAPS_POW2;
		lpd3ddd1fix.dpcTriCaps.dwTextureCaps|=D3DPTEXTURECAPS_NONPOW2CONDITIONAL|D3DPTEXTURECAPS_POW2;
		lpd3ddd2fix.dpcLineCaps.dwTextureCaps|=D3DPTEXTURECAPS_NONPOW2CONDITIONAL|D3DPTEXTURECAPS_POW2;
		lpd3ddd2fix.dpcTriCaps.dwTextureCaps|=D3DPTEXTURECAPS_NONPOW2CONDITIONAL|D3DPTEXTURECAPS_POW2;
		res = (*(((CallbackArg *)arg)->cb))(lpGuid, lpDeviceDescription, lpDeviceName, lpd3ddd1?&lpd3ddd1fix:NULL, lpd3ddd2?&lpd3ddd2fix:NULL, ((CallbackArg *)arg)->arg);
	}
	else {
		res = (*(((CallbackArg *)arg)->cb))(lpGuid, lpDeviceDescription, lpDeviceName, lpd3ddd1, lpd3ddd2, ((CallbackArg *)arg)->arg);
	}
	OutTraceD3D("EnumDevices: CALLBACK ret=%x\n", res);
	return res;
}

HRESULT WINAPI extDeviceProxy7(LPSTR lpDeviceDescription, LPSTR lpDeviceName, LPD3DDEVICEDESC7 lpd3ddd, LPVOID arg)
{
	HRESULT res;
	OutTraceD3D("EnumDevices(D3D7): CALLBACK DeviceDescription=\"%s\", DeviceName=\"%s\", arg=%x\n", lpDeviceDescription, lpDeviceName, ((CallbackArg *)arg)->arg);
	DumpD3DDevideDesc((LPD3DDEVICEDESC)lpd3ddd, "DEV");
	if(dxw.dwFlags4 & NOPOWER2FIX){ 
		D3DDEVICEDESC7 lpd3dddfix;
		if(lpd3ddd) memcpy(&lpd3dddfix, lpd3ddd, sizeof(D3DDEVICEDESC7));
		lpd3dddfix.dpcLineCaps.dwTextureCaps|=D3DPTEXTURECAPS_NONPOW2CONDITIONAL|D3DPTEXTURECAPS_POW2;
		lpd3dddfix.dpcTriCaps.dwTextureCaps|=D3DPTEXTURECAPS_NONPOW2CONDITIONAL|D3DPTEXTURECAPS_POW2;
		res = (*(((CallbackArg7 *)arg)->cb))(lpDeviceDescription, lpDeviceName, lpd3ddd?&lpd3dddfix:NULL, ((CallbackArg7 *)arg)->arg);
	}
	else {
		res = (*(((CallbackArg7 *)arg)->cb))(lpDeviceDescription, lpDeviceName, lpd3ddd, ((CallbackArg7 *)arg)->arg);
	}
	OutTraceD3D("EnumDevices: CALLBACK ret=%x\n", res);
	return res;
}

HRESULT WINAPI extEnumDevices(void *lpd3d, LPD3DENUMDEVICESCALLBACK cb, LPVOID arg)
{
	HRESULT res;
	CallbackArg Arg;

	OutTraceD3D("EnumDevices: d3d=%x arg=%x\n", lpd3d, arg);
	Arg.cb= &cb;
	Arg.arg=arg;
	res=(*pEnumDevices)(lpd3d, (LPD3DENUMDEVICESCALLBACK)extDeviceProxy, (LPVOID)&Arg);
	if(res) OutTraceE("EnumDevices ERROR: err=%x(%s) at %d\n", res, ExplainDDError(res), __LINE__);
	else OutTraceD3D("EnumDevices: OK\n");
	return res;
}

HRESULT WINAPI extEnumDevices7(void *lpd3d, LPD3DENUMDEVICESCALLBACK7 cb, LPVOID arg)
{
	HRESULT res;
	CallbackArg7 Arg;

	OutTraceD3D("EnumDevices: d3d=%x arg=%x\n", lpd3d, arg);
	Arg.cb= &cb;
	Arg.arg=arg;
	res=(*pEnumDevices7)(lpd3d, (LPD3DENUMDEVICESCALLBACK7)extDeviceProxy7, (LPVOID)&Arg);
	if(res) OutTraceE("EnumDevices ERROR: err=%x(%s) at %d\n", res, ExplainDDError(res), __LINE__);
	else OutTraceD3D("EnumDevices: OK\n");
	return res;
}

HRESULT WINAPI extCreateLight(void *lpd3d, LPDIRECT3DLIGHT *lpLight, IUnknown *p0)
{
	HRESULT res;

	OutTraceD3D("CreateLight: d3d=%x\n", lpd3d);
	res=(*pCreateLight)(lpd3d, lpLight, p0);
	if(res) OutTraceE("CreateLight ERROR: err=%x(%s) at %d\n", res, ExplainDDError(res), __LINE__);
	else OutTraceD3D("CreateLight: OK\n");
	return res;
}

HRESULT WINAPI extCreateMaterial(void *lpd3d, LPDIRECT3DMATERIAL *lpMaterial, IUnknown *p0)
{
	HRESULT res;

	OutTraceD3D("CreateMaterial: d3d=%x\n", lpd3d);
	res=(*pCreateMaterial)(lpd3d, lpMaterial, p0);
	if(res) OutTraceE("CreateMaterial ERROR: err=%x(%s) at %d\n", res, ExplainDDError(res), __LINE__);
	else OutTraceD3D("CreateMaterial: OK\n");
	HookMaterial(lpMaterial, GD3DDeviceVersion);
	return res;
}

HRESULT WINAPI extCreateViewport1(void *lpd3d, LPDIRECT3DVIEWPORT *lpViewport, IUnknown *p0)
{
	HRESULT res;

	OutTraceD3D("CreateViewport(1): d3d=%x\n", lpd3d);
	res=(*pCreateViewport1)(lpd3d, lpViewport, p0);
	if(res) OutTraceE("CreateViewport ERROR: err=%x(%s) at %d\n", res, ExplainDDError(res), __LINE__);
	else OutTraceD3D("CreateViewport: Viewport=%x\n", *lpViewport);
	HookViewport(lpViewport, 1);
	return res;
}

HRESULT WINAPI extCreateViewport2(void *lpd3d, LPDIRECT3DVIEWPORT2 *lpViewport, IUnknown *p0)
{
	HRESULT res;

	OutTraceD3D("CreateViewport(2): d3d=%x\n", lpd3d);
	res=(*pCreateViewport2)(lpd3d, lpViewport, p0);
	if(res) OutTraceE("CreateViewport(2) ERROR: err=%x(%s) at %d\n", res, ExplainDDError(res), __LINE__);
	else OutTraceD3D("CreateViewport(2): Viewport=%x\n", *lpViewport);
	HookViewport((LPDIRECT3DVIEWPORT *)lpViewport, 2);
	return res;
}

HRESULT WINAPI extCreateViewport3(void *lpd3d, LPDIRECT3DVIEWPORT3 *lpViewport, IUnknown *p0)
{
	HRESULT res;

	OutTraceD3D("CreateViewport(3): d3d=%x\n", lpd3d);
	res=(*pCreateViewport3)(lpd3d, lpViewport, p0);
	if(res) OutTraceE("CreateViewport(3) ERROR: err=%x(%s) at %d\n", res, ExplainDDError(res), __LINE__);
	else OutTraceD3D("CreateViewport(3): Viewport=%x\n", *lpViewport);
	HookViewport((LPDIRECT3DVIEWPORT *)lpViewport, 3);
	if(IsDebug){
		HRESULT res2;
		D3DVIEWPORT2 vpdesc;
		vpdesc.dwSize = sizeof(D3DVIEWPORT2);
		res2=(*pGetViewport2_3)(*lpViewport, &vpdesc);
		if(res) 
			OutTraceE("CreateViewport(3) ERROR: err=%x(%s) at %d\n", res, ExplainDDError(res), __LINE__);
		else
			OutTraceD3D("CreateViewport(3): size=%d pos=(%d,%d) dim=(%dx%d)\n",
				vpdesc.dwSize, vpdesc.dwX, vpdesc.dwY, vpdesc.dwWidth, vpdesc.dwHeight);
	}
	return res;
}

HRESULT WINAPI extFindDevice(void *lpd3d, LPD3DFINDDEVICESEARCH p1, LPD3DFINDDEVICERESULT p2)
{
	HRESULT res;

	OutTraceD3D("FindDevice: d3d=%x devsearch=%x (size=%d flags=%x caps=%x primcaps=%x colormodel=%x hw=%x guid=%x) p2=%x\n", 
		lpd3d, p1, p1->dwSize, p1->dwFlags, p1->dwCaps, p1->dpcPrimCaps, p1->dcmColorModel, p1->bHardware, p1->guid, p2);
	res=(*pFindDevice)(lpd3d, p1, p2);
	if(res) OutTraceE("FindDevice ERROR: err=%x(%s) at %d\n", res, ExplainDDError(res), __LINE__);
	else {
		OutTraceD3D("FindDevice: GUID=%x.%x.%x.%x\n", p2->guid.Data1, p2->guid.Data2, p2->guid.Data3, p2->guid.Data4);
		DumpD3DDevideDesc(&(p2->ddHwDesc), "HWDEV");
		DumpD3DDevideDesc(&(p2->ddSwDesc), "SWDEV");
	}
	return res;
}

HRESULT WINAPI extSetViewport(void *lpvp, LPD3DVIEWPORT vpd)
{
	HRESULT res;

	OutTraceD3D("SetViewport: viewport=%x viewportd=%x size=%d pos=(%d,%d) dim=(%dx%d) scale=(%fx%f) maxXYZ=(%f,%f,%f) minZ=%f\n", 
		lpvp, vpd, vpd->dwSize, vpd->dwX, vpd->dwY, vpd->dwWidth, vpd->dwHeight, vpd->dvScaleX, vpd->dvScaleY, 
		vpd->dvMaxX, vpd->dvMaxY, vpd->dvMaxZ, vpd->dvMinZ);

	// v2.03.48: scaled dvScaleX/Y fields. Fixes "Dark Vengeance" viewport size when using D3D interface.
	// no.... see Forsaken
	//dxw.MapClient(&vpd->dvScaleX, &vpd->dvScaleY);
	//OutTraceDW("SetViewport: FIXED scale=(%fx%f)\n", vpd->dvScaleX, vpd->dvScaleY);

	res=(*pSetViewport)(lpvp, vpd);
	if(res) OutTraceE("SetViewport ERROR: err=%x(%s) at %d\n", res, ExplainDDError(res), __LINE__);
	else OutTraceD3D("SetViewport: OK\n");
	return res;
}

HRESULT WINAPI extGetViewport(void *lpvp, LPD3DVIEWPORT vpd)
{
	HRESULT res;

	OutTraceD3D("GetViewport: viewport=%x viewportd=%x\n", lpvp, vpd);
	res=(*pGetViewport)(lpvp, vpd);
	// v2.03.48: should the dvScaleX/Y fields be unscaled? 
	if(res) OutTraceE("GetViewport ERROR: err=%x(%s) at %d\n", res, ExplainDDError(res), __LINE__);
	else OutTraceD3D("GetViewport: OK size=%d pos=(%d,%d) dim=(%dx%d) scale=(%fx%f) maxXYZ=(%f,%f,%f) minZ=%f\n",
		vpd->dwSize, vpd->dwX, vpd->dwY, vpd->dwWidth, vpd->dwHeight, vpd->dvScaleX, vpd->dvScaleY, 
		vpd->dvMaxX, vpd->dvMaxY, vpd->dvMaxZ, vpd->dvMinZ);
	return res;
}

HRESULT WINAPI extInitializeVP(void *lpvp, LPDIRECT3D lpd3d)
{
	HRESULT res;

	OutTraceD3D("Initialize(VP): viewport=%x d3d=%x\n", lpvp, lpd3d);
	res=(*pInitializeVP)(lpvp, lpd3d);
	if(res) OutTraceE("Initialize(VP) ERROR: err=%x(%s) at %d\n", res, ExplainDDError(res), __LINE__);
	else OutTraceD3D("Initialize(VP): OK \n");
	return res;
}

HRESULT WINAPI extCreateDevice2(void *lpd3d, REFCLSID Guid, LPDIRECTDRAWSURFACE lpdds, LPDIRECT3DDEVICE2 *lplpd3dd)
{
	HRESULT res;

	OutTraceD3D("CreateDevice(D3D2): d3d=%x GUID=%x(%s) lpdds=%x%s\n", 
		lpd3d, Guid.Data1, ExplainGUID((GUID *)&Guid), lpdds, dxwss.ExplainSurfaceRole((LPDIRECTDRAWSURFACE)lpdds));

	res=(*pCreateDevice2)(lpd3d, Guid, lpdds, lplpd3dd);
	if(res!=DD_OK) {
		OutTraceE("CreateDevice(D3D2) ERROR: err=%x(%s) at %d\n", res, ExplainDDError(res), __LINE__);
	}
	else{
		OutTraceD3D("CreateDevice(D3D2): lpd3dd=%x\n", lpd3d, *lplpd3dd);
		HookDirect3DDevice((void **)lplpd3dd, 2); 
	}

	return res;
}

HRESULT WINAPI extCreateDevice3(void *lpd3d, REFCLSID Guid, LPDIRECTDRAWSURFACE4 lpdds, LPDIRECT3DDEVICE3 *lplpd3dd, LPUNKNOWN unk)
{
	HRESULT res;

	OutTraceD3D("CreateDevice(D3D3): d3d=%x GUID=%x(%s) lpdds=%x%s\n", 
		lpd3d, Guid.Data1, ExplainGUID((GUID *)&Guid), lpdds, dxwss.ExplainSurfaceRole((LPDIRECTDRAWSURFACE)lpdds));

	res=(*pCreateDevice3)(lpd3d, Guid, lpdds, lplpd3dd, unk);
	if(res!=DD_OK) {
		OutTraceE("CreateDevice(D3D3) ERROR: err=%x(%s) at %d\n", res, ExplainDDError(res), __LINE__);
	}
	else{
		OutTraceD3D("CreateDevice(D3D3): lpd3dd=%x\n", lpd3d, *lplpd3dd);
		HookDirect3DDevice((void **)lplpd3dd, 3); 
	}

	return res;
}

HRESULT WINAPI extCreateDevice7(void *lpd3d, REFCLSID Guid, LPDIRECTDRAWSURFACE7 lpdds, LPDIRECT3DDEVICE7 *lplpd3dd)
{
	// v2.02.83: D3D CreateDevice (version 7? all versions?) internally calls the Release method upon the backbuffer
	// surface, and this has to be avoided since it causes a crash. 

	HRESULT res;

	OutTraceD3D("CreateDevice(D3D7): d3d=%x GUID=%x(%s) lpdds=%x%s\n", 
		lpd3d, Guid.Data1, ExplainGUID((GUID *)&Guid), lpdds, dxwss.ExplainSurfaceRole((LPDIRECTDRAWSURFACE)lpdds));

	res=(*pCreateDevice7)(lpd3d, Guid, lpdds, lplpd3dd);

	if(res == DD_OK){
		OutTraceD3D("CreateDevice(D3D7): lpd3dd=%x\n", lpd3d, *lplpd3dd);
		HookDirect3DDevice((void **)lplpd3dd, 7); 
	}
	else
		OutTraceE("CreateDevice(D3D7) ERROR: err=%x(%s) at %d\n", res, ExplainDDError(res), __LINE__);

	return res;
}

HRESULT WINAPI extD3DInitialize(void *d3dd, LPDIRECT3D lpd3d, LPGUID lpGuid, LPD3DDEVICEDESC lpd3dd)
{
	HRESULT res;
	OutTraceD3D("Initialize: d3dd=%x lpd3d=%x GUID=%x lpd3ddd=%x\n", d3dd, lpd3d, lpGuid->Data1, lpd3dd);
	res=(*pD3DInitialize)(d3dd, lpd3d, lpGuid, lpd3dd);
	if(res) OutTraceE("Initialize ERROR: err=%x(%s) at %d\n", res, ExplainDDError(res), __LINE__);
	DumpD3DDevideDesc(lpd3dd, "INIT");
	return res;
}

HRESULT WINAPI extD3DGetCaps(void *d3dd, LPD3DDEVICEDESC lpd3dd ,LPD3DDEVICEDESC lpd3dd2)
{
	HRESULT res;
	OutTraceD3D("GetCaps(D3D): d3dd=%x lpd3dd=%x lpd3dd2=%x \n", d3dd, lpd3dd, lpd3dd2);
	res=(*pD3DGetCaps)(d3dd, lpd3dd, lpd3dd2);
	if(res) OutTraceE("GetCaps(D3D) ERROR: err=%x(%s) at %d\n", res, ExplainDDError(res), __LINE__);
	DumpD3DDevideDesc(lpd3dd, "HWDEV");
	DumpD3DDevideDesc(lpd3dd2, "SWDEV");
	return res;
}

HRESULT WINAPI extSetRenderState(SetRenderState3_Type pSetRenderState, int version, void *d3dd, D3DRENDERSTATETYPE State, DWORD Value)
{
	HRESULT res;
	OutTraceB("SetRenderState(%d): d3dd=%x State=%x(%s) Value=%x\n", version, d3dd, State, ExplainD3DRenderState(State), Value);

	if((dxw.dwFlags4 & ZBUFFERALWAYS) && (State == D3DRENDERSTATE_ZFUNC)) {
		OutTraceD3D("SetRenderState: FIXED State=ZFUNC Value=%s->D3DCMP_ALWAYS\n", ExplainRenderstateValue(Value));
		Value = D3DCMP_ALWAYS;
	}
	if((dxw.dwFlags2 & WIREFRAME) && (State == D3DRENDERSTATE_FILLMODE)){
		OutTraceD3D("SetRenderState: FIXED State=FILLMODE Value=%x->D3DFILL_WIREFRAME\n", Value);
		Value = D3DFILL_WIREFRAME;
	}
	if((dxw.dwFlags4 & DISABLEFOGGING) && (State == D3DRENDERSTATE_FOGENABLE)){
		OutTraceD3D("SetRenderState: FIXED State=FOGENABLE Value=%x->FALSE\n", Value);
		Value = FALSE;
	}

	res=(*pSetRenderState)(d3dd, State, Value);
	if(res) OutTraceE("SetRenderState: res=%x(%s)\n", res, ExplainDDError(res));
	return res;
}

HRESULT WINAPI extSetRenderState2(void *d3dd, D3DRENDERSTATETYPE State, DWORD Value)
{
	return extSetRenderState(pSetRenderState2, 2, d3dd, State, Value);
}

HRESULT WINAPI extSetRenderState3(void *d3dd, D3DRENDERSTATETYPE State, DWORD Value)
{
	return extSetRenderState(pSetRenderState3, 3, d3dd, State, Value);
}

HRESULT WINAPI extSetRenderState7(void *d3dd, D3DRENDERSTATETYPE State, DWORD Value)
{
	return extSetRenderState(pSetRenderState7, 7, d3dd, State, Value);
}

HRESULT WINAPI extBeginScene1(void *d3dd)
{
	HRESULT res;
	OutTraceD3D("BeginScene(1): d3dd=%x\n", d3dd);
	res=(*pBeginScene1)(d3dd);
	if(res) OutTraceE("BeginScene(1): res=%x(%s)\n", res, ExplainDDError(res));
	return res;
}

HRESULT WINAPI extBeginScene2(void *d3dd)
{
	HRESULT res;
	OutTraceD3D("BeginScene(2): d3dd=%x\n", d3dd);
	if((dxw.dwFlags4 & ZBUFFERCLEAN) || (dxw.dwFlags5 & CLEARTARGET)){
		HRESULT res2;
		LPDIRECT3DVIEWPORT2 vp;
		D3DVIEWPORT vpd;
		res2=((LPDIRECT3DDEVICE2)d3dd)->GetCurrentViewport(&vp);
		if(!res2){
			D3DRECT d3dRect;
			vpd.dwSize=sizeof(D3DVIEWPORT);
			vp->GetViewport(&vpd);
			d3dRect.x1 = vpd.dwX; 
			d3dRect.y1 = vpd.dwY;
			d3dRect.x2 = vpd.dwX + vpd.dwWidth;
			d3dRect.y2 = vpd.dwY + vpd.dwHeight;
			OutTraceD3D("d3dRect=(%d,%d)-(%d,%d)\n", d3dRect.x1, d3dRect.y1, d3dRect.x2, d3dRect.y2);
			if(dxw.dwFlags4 & ZBUFFERCLEAN) vp->Clear(1, &d3dRect, D3DCLEAR_ZBUFFER);	
			if(dxw.dwFlags5 & CLEARTARGET) vp->Clear(1, &d3dRect, D3DCLEAR_TARGET);	
		}
	}
	res=(*pBeginScene2)(d3dd);
	if(res) OutTraceE("BeginScene(2): res=%x(%s)\n", res, ExplainDDError(res));
	return res;
}

HRESULT WINAPI extBeginScene3(void *d3dd)
{
	HRESULT res;
	OutTraceD3D("BeginScene(3): d3dd=%x\n", d3dd);
	if((dxw.dwFlags4 & (ZBUFFERCLEAN|ZBUFFER0CLEAN)) || (dxw.dwFlags5 & CLEARTARGET)){
		HRESULT res2;
		LPDIRECT3DVIEWPORT3 vp;
		D3DVIEWPORT vpd;
		res2=((LPDIRECT3DDEVICE3)d3dd)->GetCurrentViewport(&vp);
		if(!res2){
			D3DRECT d3dRect;
			vpd.dwSize=sizeof(D3DVIEWPORT);
			vp->GetViewport(&vpd);
			d3dRect.x1 = vpd.dwX; 
			d3dRect.y1 = vpd.dwY;
			d3dRect.x2 = vpd.dwX + vpd.dwWidth;
			d3dRect.y2 = vpd.dwY + vpd.dwHeight;
			OutTraceD3D("d3dRect=(%d,%d)-(%d,%d)\n", d3dRect.x1, d3dRect.y1, d3dRect.x2, d3dRect.y2);
			if(dxw.dwFlags4 & ZBUFFERCLEAN )vp->Clear2(1, &d3dRect, D3DCLEAR_ZBUFFER, 0, 1.0, 0);	
			if(dxw.dwFlags4 & ZBUFFER0CLEAN)vp->Clear2(1, &d3dRect, D3DCLEAR_ZBUFFER, 0, 0.0, 0);	
			if(dxw.dwFlags5 & CLEARTARGET) vp->Clear(1, &d3dRect, D3DCLEAR_TARGET);	
		}
	}
	res=(*pBeginScene3)(d3dd);
	if(res) OutTraceE("BeginScene(3): res=%x(%s)\n", res, ExplainDDError(res));
	return res;
}

HRESULT WINAPI extBeginScene7(void *d3dd)
{
	HRESULT res;
	OutTraceD3D("BeginScene(7): d3dd=%x\n", d3dd);

	// there is no Clear method for Viewport object in D3D7 !!!

	res=(*pBeginScene7)(d3dd);
	if(res) OutTraceE("BeginScene(7): res=%x(%s)\n", res, ExplainDDError(res));
	return res;
}

HRESULT WINAPI extEndScene1(void *d3dd)
{
	HRESULT res;
	OutTraceD3D("EndScene(1): d3dd=%x\n", d3dd);
	res=(*pEndScene1)(d3dd);
	//dxw.ShowOverlay();
	if(res) OutTraceE("EndScene(1): res=%x(%s)\n", res, ExplainDDError(res));
	return res;
}

HRESULT WINAPI extEndScene2(void *d3dd)
{
	HRESULT res;
	OutTraceD3D("EndScene(2): d3dd=%x\n", d3dd);
	res=(*pEndScene2)(d3dd);
	//dxw.ShowOverlay();
	if(res) OutTraceE("EndScene(2): res=%x(%s)\n", res, ExplainDDError(res));
	return res;
}

HRESULT WINAPI extEndScene3(void *d3dd)
{
	HRESULT res;
	OutTraceD3D("EndScene(3): d3dd=%x\n", d3dd);
	res=(*pEndScene3)(d3dd);
	//dxw.ShowOverlay();
	if(res) OutTraceE("EndScene(3): res=%x(%s)\n", res, ExplainDDError(res));
	return res;
}

HRESULT WINAPI extEndScene7(void *d3dd)
{
	HRESULT res;
	OutTraceD3D("EndScene(7): d3dd=%x\n", d3dd);
	res=(*pEndScene7)(d3dd);
	//dxw.ShowOverlay();
	if(res) OutTraceE("EndScene(7): res=%x(%s)\n", res, ExplainDDError(res));
	return res;
}

HRESULT WINAPI extGetCaps3(void *d3dd, LPD3DDEVICEDESC hd, LPD3DDEVICEDESC sd)
{
	HRESULT res;
	OutTraceD3D("GetCaps(3): d3dd=%x hd=%x sd=%x\n", d3dd, hd, sd);
	res=(*pGetCaps3)(d3dd, hd, sd);
	if(res) {
		OutTraceE("GetCaps(3): res=%x(%s)\n", res, ExplainDDError(res));
		return res;
	}
	if(dxw.dwFlags4 & NOPOWER2FIX){
		if(hd) {
            hd->dpcLineCaps.dwTextureCaps|=D3DPTEXTURECAPS_NONPOW2CONDITIONAL|D3DPTEXTURECAPS_POW2;
            hd->dpcTriCaps.dwTextureCaps|=D3DPTEXTURECAPS_NONPOW2CONDITIONAL|D3DPTEXTURECAPS_POW2;
        }
        if(sd) {
            sd->dpcLineCaps.dwTextureCaps|=D3DPTEXTURECAPS_NONPOW2CONDITIONAL|D3DPTEXTURECAPS_POW2;
            sd->dpcTriCaps.dwTextureCaps|=D3DPTEXTURECAPS_NONPOW2CONDITIONAL|D3DPTEXTURECAPS_POW2;
        }
    }
	return res;
}

HRESULT WINAPI extSetLightState3(void *d3dd, D3DLIGHTSTATETYPE d3dls, DWORD t)
{
	HRESULT res;
	OutTraceD3D("SetLightState(3): d3d=%x lightstate=%x t=%x\n", d3dd, d3dls, t);
	res=(*pSetLightState3)(d3dd, d3dls, t);
	if(res) OutTraceE("SetLightState(3): ERROR res=%x(%s)\n", res, ExplainDDError(res));
	return res;
}

HRESULT WINAPI extSetViewport2(void *d3dd, LPD3DVIEWPORT lpd3dvp)
{
	HRESULT res;
	OutTraceD3D("SetViewport(2): d3d=%x d3dvp=%x\n", d3dd, lpd3dvp);
	res=(*pSetViewport2)(d3dd, lpd3dvp);
	if(res) OutTraceE("SetViewport(2): ERROR res=%x(%s)\n", res, ExplainDDError(res));
	return res;
}

HRESULT WINAPI extGetViewport2(void *d3dd, LPD3DVIEWPORT lpd3dvp)
{
	HRESULT res;
	OutTraceD3D("GetViewport(2): d3d=%x d3dvp=%x\n", d3dd, lpd3dvp);
	res=(*pGetViewport2)(d3dd, lpd3dvp);
	if(res) OutTraceE("GetViewport(2): ERROR res=%x(%s)\n", res, ExplainDDError(res));
	return res;
}

HRESULT WINAPI extSetViewport3(void *d3dd, LPD3DVIEWPORT lpd3dvp)
{
	HRESULT res;
	OutTraceD3D("SetViewport(3): d3d=%x d3dvp=%x\n", d3dd, lpd3dvp);
	res=(*pSetViewport3)(d3dd, lpd3dvp);
	if(res) OutTraceE("SetViewport(3): ERROR res=%x(%s)\n", res, ExplainDDError(res));
	return res;
}

HRESULT WINAPI extGetViewport3(void *d3dd, LPD3DVIEWPORT lpd3dvp)
{
	HRESULT res;
	OutTraceD3D("GetViewport(3): d3d=%x d3dvp=%x\n", d3dd, lpd3dvp);
	res=(*pGetViewport3)(d3dd, lpd3dvp);
	if(res) OutTraceE("GetViewport(3): ERROR res=%x(%s)\n", res, ExplainDDError(res));
	return res;
}

HRESULT WINAPI extSetViewport7(void *d3dd, LPD3DVIEWPORT7 lpd3dvp)
{
	HRESULT res;
	OutTraceD3D("SetViewport(7): d3d=%x d3dvp=%x\n", d3dd, lpd3dvp);
	res=(*pSetViewport7)(d3dd, lpd3dvp);
	if(res) OutTraceE("SetViewport(7): ERROR res=%x(%s)\n", res, ExplainDDError(res));
	return res;
}

HRESULT WINAPI extGetViewport7(void *d3dd, LPD3DVIEWPORT7 lpd3dvp)
{
	HRESULT res;
	OutTraceD3D("GetViewport(7): d3d=%x d3dvp=%x\n", d3dd, lpd3dvp);
	res=(*pGetViewport7)(d3dd, lpd3dvp);
	if(res) OutTraceE("GetViewport(7): ERROR res=%x(%s)\n", res, ExplainDDError(res));
	return res;
}

HRESULT WINAPI extAddViewport1(void *d3dd, LPDIRECT3DVIEWPORT lpd3dvp)
{
	HRESULT res;
	OutTraceD3D("AddViewport(1): d3d=%x d3dvp=%x\n", d3dd, lpd3dvp);
	res=(*pAddViewport1)(d3dd, lpd3dvp);
	if(res) {
		OutTraceE("AddViewport(1): ERROR res=%x(%s)\n", res, ExplainDDError(res));
		return res;
	}
	HookViewport(&lpd3dvp, 1);
	return res;
}

HRESULT WINAPI extAddViewport2(void *d3dd, LPDIRECT3DVIEWPORT2 lpd3dvp)
{
	HRESULT res;
	OutTraceD3D("AddViewport(2): d3d=%x d3dvp=%x\n", d3dd, lpd3dvp);
	res=(*pAddViewport2)(d3dd, lpd3dvp);
	if(res) {
		OutTraceE("AddViewport(2): ERROR res=%x(%s)\n", res, ExplainDDError(res));
		return res;
	}
	HookViewport((LPDIRECT3DVIEWPORT *)&lpd3dvp, 2);
	return res;
}

HRESULT WINAPI extAddViewport3(void *d3dd, LPDIRECT3DVIEWPORT3 lpd3dvp)
{
	HRESULT res;
	OutTraceD3D("AddViewport(3): d3d=%x d3dvp=%x\n", d3dd, lpd3dvp);
	res=(*pAddViewport3)(d3dd, lpd3dvp);
	if(res) {
		OutTraceE("AddViewport(3): ERROR res=%x(%s)\n", res, ExplainDDError(res));
		return res;
	}
	HookViewport((LPDIRECT3DVIEWPORT *)&lpd3dvp, 3);
	return res;
}

HRESULT WINAPI extSetViewport2_3(void *lpvp, LPD3DVIEWPORT2 vpd)
{
	HRESULT res;

	OutTraceD3D("SetViewport2(VP3): viewport=%x viewportd=%x size=%d pos=(%d,%d) dim=(%dx%d)\n", 
		lpvp, vpd, vpd->dwSize, vpd->dwX, vpd->dwY, vpd->dwWidth, vpd->dwHeight);
	res=(*pSetViewport2_3)(lpvp, vpd);
	if(res) OutTraceE("SetViewport2(VP3) ERROR: err=%x(%s) at %d\n", res, ExplainDDError(res), __LINE__);
	else OutTraceD3D("SetViewport2(VP3): OK\n");
	return res;
}

HRESULT WINAPI extGetViewport2_3(void *lpvp, LPD3DVIEWPORT2 vpd)
{
	HRESULT res;

	OutTraceD3D("GetViewport2(VP3): viewport=%x viewportd=%x\n", lpvp, vpd);
	res=(*pGetViewport2_3)(lpvp, vpd);
	if(res) OutTraceE("GetViewport2(VP3) ERROR: err=%x(%s) at %d\n", res, ExplainDDError(res), __LINE__);
	else OutTraceD3D("GetViewport2(VP3): OK size=%d pos=(%d,%d) dim=(%dx%d)\n",
		vpd->dwSize, vpd->dwX, vpd->dwY, vpd->dwWidth, vpd->dwHeight);
	return res;
}

HRESULT WINAPI extGetCurrentViewport2(void *d3dd, LPDIRECT3DVIEWPORT2 *lpd3dvp)
{
	HRESULT res;
	OutTraceD3D("GetCurrentViewport(D3DD2): d3dd=%x viewportd=%x\n", d3dd, lpd3dvp);
	res=(*pGetCurrentViewport2)(d3dd, lpd3dvp);
	if(res) {
		OutTraceE("GetCurrentViewport(D3DD2) ERROR: err=%x(%s) at %d\n", res, ExplainDDError(res), __LINE__);
		return res;
	}
	HookViewport((LPDIRECT3DVIEWPORT *)lpd3dvp, 2);
	return res;
}

HRESULT WINAPI extSetCurrentViewport2(void *d3dd, LPDIRECT3DVIEWPORT2 lpd3dvp)
{
	HRESULT res;

	OutTraceD3D("SetCurrentViewport(D3DD2): d3dd=%x viewportd=%x\n", d3dd, lpd3dvp);
	res=(*pSetCurrentViewport2)(d3dd, lpd3dvp);
	if(res) OutTraceE("SetCurrentViewport(D3DD2) ERROR: err=%x(%s) at %d\n", res, ExplainDDError(res), __LINE__);
	else OutTraceD3D("SetCurrentViewport(D3DD2): OK\n");
	return res;
}

HRESULT WINAPI extGetCurrentViewport3(void *d3dd, LPDIRECT3DVIEWPORT3 *lpd3dvp)
{
	HRESULT res;
	OutTraceD3D("GetCurrentViewport(D3DD3): d3dd=%x viewportd=%x\n", d3dd, lpd3dvp);
	res=(*pGetCurrentViewport3)(d3dd, lpd3dvp);
	if(res) {
		OutTraceE("GetCurrentViewport(D3DD3) ERROR: err=%x(%s) at %d\n", res, ExplainDDError(res), __LINE__);
		return res;
	}
	HookViewport((LPDIRECT3DVIEWPORT *)lpd3dvp, 3);
	if(IsDebug){
		HRESULT res2;
		D3DVIEWPORT2 vpdesc;
		vpdesc.dwSize = sizeof(D3DVIEWPORT2);
		res2=(*pGetViewport2_3)(lpd3dvp, &vpdesc);
		if(res) 
			OutTraceE("GetCurrentViewport(D3DD3) ERROR: err=%x(%s) at %d\n", res, ExplainDDError(res), __LINE__);
		else
			OutTraceD3D("GetCurrentViewport(D3DD3): size=%d pos=(%d,%d) dim=(%dx%d)\n",
				vpdesc.dwSize, vpdesc.dwX, vpdesc.dwY, vpdesc.dwWidth, vpdesc.dwHeight);
	}
	return res;
}

HRESULT WINAPI extSetCurrentViewport3(void *lpvp, LPDIRECT3DVIEWPORT3 lpd3dvp)
{
	HRESULT res;

	OutTraceD3D("SetCurrentViewport(D3DD3): viewport=%x\n", lpvp, lpd3dvp);
	res=(*pSetCurrentViewport3)(lpvp, lpd3dvp);
	if(res) OutTraceE("SetCurrentViewport(D3DD3) ERROR: err=%x(%s) at %d\n", res, ExplainDDError(res), __LINE__);
	else OutTraceD3D("SetCurrentViewport(D3DD3): OK\n");
	if(IsDebug){
		HRESULT res2;
		D3DVIEWPORT2 vpdesc;
		vpdesc.dwSize = sizeof(D3DVIEWPORT2);
		res2=(*pGetViewport2_3)(lpd3dvp, &vpdesc);
		if(res) 
			OutTraceE("SetCurrentViewport(D3DD3) ERROR: err=%x(%s) at %d\n", res, ExplainDDError(res), __LINE__);
		else
			OutTraceD3D("SetCurrentViewport(D3DD3): size=%d pos=(%d,%d) dim=(%dx%d)\n",
				vpdesc.dwSize, vpdesc.dwX, vpdesc.dwY, vpdesc.dwWidth, vpdesc.dwHeight);
	}
	return res;
}

HRESULT WINAPI extDeleteViewport1(void *d3dd, LPDIRECT3DVIEWPORT lpd3dvp)
{
	HRESULT res;
	OutTraceD3D("DeletetViewport(D3DD1): d3dd=%x viewport=%x\n", d3dd, lpd3dvp);
	res=(*pDeleteViewport1)(d3dd, lpd3dvp);
	if(res) OutTraceE("DeletetViewport(D3DD1) ERROR: err=%x(%s) at %d\n", res, ExplainDDError(res), __LINE__);
	else OutTraceD3D("DeletetViewport(D3DD1): OK\n");
	return res;
}

HRESULT WINAPI extNextViewport1(void *d3dd, LPDIRECT3DVIEWPORT lpd3dvp, LPDIRECT3DVIEWPORT *vpnext, DWORD dw)
{
	HRESULT res;
	OutTraceD3D("NextViewport(D3DD1): d3dd=%x viewport=%x dw=%x\n", d3dd, lpd3dvp);
	res=(*pNextViewport1)(d3dd, lpd3dvp, vpnext, dw);
	if(res) OutTraceE("NextViewport(D3DD1) ERROR: err=%x(%s) at %d\n", res, ExplainDDError(res), __LINE__);
	else OutTraceD3D("NextViewport(D3DD1): next=%x\n", *vpnext);
	return res;
}

HRESULT WINAPI extDeleteViewport2(void *d3dd, LPDIRECT3DVIEWPORT2 lpd3dvp)
{
	HRESULT res;
	OutTraceD3D("DeletetViewport(D3DD2): d3dd=%x viewport=%x\n", d3dd, lpd3dvp);
	res=(*pDeleteViewport2)(d3dd, lpd3dvp);
	if(res) OutTraceE("DeletetViewport(D3DD2) ERROR: err=%x(%s) at %d\n", res, ExplainDDError(res), __LINE__);
	else OutTraceD3D("DeletetViewport(D3DD2): OK\n");
	return res;
}

HRESULT WINAPI extNextViewport2(void *d3dd, LPDIRECT3DVIEWPORT2 lpd3dvp, LPDIRECT3DVIEWPORT2 *vpnext, DWORD dw)
{
	HRESULT res;
	OutTraceD3D("NextViewport(D3DD2): d3dd=%x viewport=%x dw=%x\n", d3dd, lpd3dvp);
	res=(*pNextViewport2)(d3dd, lpd3dvp, vpnext, dw);
	if(res) OutTraceE("NextViewport(D3DD2) ERROR: err=%x(%s) at %d\n", res, ExplainDDError(res), __LINE__);
	else OutTraceD3D("NextViewport(D3DD2): next=%x\n", *vpnext);
	return res;
}

HRESULT WINAPI extSetTexture3(void *d3dd, DWORD flags, LPDIRECT3DTEXTURE2 lptex)
{
	HRESULT res;
	OutTraceD3D("SetTexture(3): d3dd=%x flags=%x tex=%x\n", d3dd, flags, lptex);
	if (dxw.dwFlags4 & NOTEXTURES) return DD_OK;

	res=(*pSetTexture3)(d3dd, flags, lptex);
	if(res) OutTraceD3D("SetTexture(3): ERROR res=%x(%s)\n", res, ExplainDDError(res));
	return res;
}

HRESULT WINAPI extSetTexture7(void *d3dd, DWORD flags, LPDIRECTDRAWSURFACE7 lptex)
{
	HRESULT res;
	OutTraceD3D("SetTexture(7): d3dd=%x, flags=%x, tex=%x\n", d3dd, flags, lptex);
	if (dxw.dwFlags4 & NOTEXTURES) return DD_OK;

	res=(*pSetTexture7)(d3dd, flags, lptex);
	if(res) OutTraceD3D("SetTexture(7): ERROR res=%x(%s)\n", res, ExplainDDError(res));
	return res;
}

HRESULT WINAPI extSetMaterial(void *d3dd, LPD3DMATERIAL lpMaterial)
{
	HRESULT res;

	OutTraceD3D("SetMaterial: d3dd=%x material=%x\n", d3dd, lpMaterial);
	if(lpMaterial && IsDebug){
		OutTraceD3D("Material: Size=%d Texture=%x diffuse=(%f,%f,%f,%f) ambient=(%f,%f,%f,%f) specular=(%f,%f,%f,%f) emissive=(%f,%f,%f,%f) power=%f\n", 
			lpMaterial->dwSize, lpMaterial->hTexture, 
			lpMaterial->diffuse.a, lpMaterial->diffuse.r, lpMaterial->diffuse.g, lpMaterial->diffuse.b,
			lpMaterial->ambient.a, lpMaterial->ambient.r, lpMaterial->ambient.g, lpMaterial->ambient.b,
			lpMaterial->specular.a, lpMaterial->specular.r, lpMaterial->specular.g, lpMaterial->specular.b,
			lpMaterial->emissive.a, lpMaterial->emissive.r, lpMaterial->emissive.g, lpMaterial->emissive.b,
			lpMaterial->power
			);
		}
	res=(*pSetMaterial)(d3dd, lpMaterial);
	if(res) OutTraceD3D("SetMaterial: ERROR res=%x\n", res);
	return res;
}

HRESULT WINAPI extGetMaterial(void *d3dd, LPD3DMATERIAL lpMaterial)
{
	HRESULT res;

	res=(*pGetMaterial)(d3dd, lpMaterial);
	OutTraceD3D("GetMaterial: d3dd=%x material=%x res=%x\n", d3dd, lpMaterial, res);
	if(lpMaterial && IsDebug && (res==DD_OK)){
		OutTraceD3D("Material: Size=%d diffuse=(%f,%f,%f,%f) ambient=(%f,%f,%f,%f) specular=(%f,%f,%f,%f) emissive=(%f,%f,%f,%f) power=%f\n", 
			lpMaterial->dwSize, 
			lpMaterial->diffuse.a, lpMaterial->diffuse.r, lpMaterial->diffuse.g, lpMaterial->diffuse.b,
			lpMaterial->ambient.a, lpMaterial->ambient.r, lpMaterial->ambient.g, lpMaterial->ambient.b,
			lpMaterial->specular.a, lpMaterial->specular.r, lpMaterial->specular.g, lpMaterial->specular.b,
			lpMaterial->emissive.a, lpMaterial->emissive.r, lpMaterial->emissive.g, lpMaterial->emissive.b,
			lpMaterial->power
			);
		}
	return res;
}

HRESULT WINAPI extSwapTextureHandles(void *d3dd, LPDIRECT3DTEXTURE t1, LPDIRECT3DTEXTURE t2)
{
	HRESULT res;

	OutTraceD3D("SwapTextureHandles(1): d3dd=%x t1=%x t2=%x\n", d3dd, t1, t2);
	if (dxw.dwFlags4 & NOTEXTURES) return DD_OK;
	
	res=(*pSwapTextureHandles)(d3dd, t1, t2);
	if(res) OutTraceD3D("SwapTextureHandles(1): ERROR res=%x\n", res);
	return res;
}

HRESULT WINAPI extSwapTextureHandles2(void *d3dd, LPDIRECT3DTEXTURE2 t1, LPDIRECT3DTEXTURE2 t2)
{
	HRESULT res;

	OutTraceD3D("SwapTextureHandles(2): d3dd=%x t1=%x t2=%x\n", d3dd, t1, t2);
	if (dxw.dwFlags4 & NOTEXTURES) return DD_OK;
	
	res=(*pSwapTextureHandles2)(d3dd, t1, t2);
	if(res) OutTraceD3D("SwapTextureHandles(2): ERROR res=%x\n", res);
	return res;
}

HRESULT WINAPI extTexInitialize(void *t, LPDIRECT3DDEVICE lpd3dd, LPDIRECTDRAWSURFACE lpdds)
{
	OutTrace("Texture::Initialize\n");
	return (*pTInitialize)(t, lpd3dd, lpdds);
}

HRESULT WINAPI extTexGetHandle(void *t, LPDIRECT3DDEVICE lpd3dd, LPD3DTEXTUREHANDLE lpth)
{
	HRESULT ret;
	OutTrace("Texture::GetHandle lpt=%x lpd3dd=%x lpth=%x\n", t, lpd3dd, lpth);
	ret = (*pTGetHandle)(t, lpd3dd, lpth);
	if(ret) OutTraceE("Texture::Load ERROR res=%x(%s)\n", ret, ExplainDDError(ret));
	return ret;
}

HRESULT WINAPI extTexPaletteChanged(void *t, DWORD dw1, DWORD dw2)
{
	HRESULT ret;
	OutTrace("Texture::PaletteChanged lpt=%x dw1=%x dw2=%x\n", t, dw1, dw2);
	ret = (*pTPaletteChanged)(t, dw1, dw2);
	if(ret) OutTraceE("Texture::PaletteChanged ERROR res=%x(%s)\n", ret, ExplainDDError(ret));
	return ret;
}

HRESULT WINAPI extTexLoad(void *t, LPDIRECT3DTEXTURE lpt)
{
	HRESULT ret;
	OutTrace("Texture::Load lpt=%x lpd3dt=%x\n", t, lpt);
	ret = (*pTLoad)(t, lpt);
	if(ret) OutTraceE("Texture::Load ERROR res=%x(%s)\n", ret, ExplainDDError(ret));
	return ret;
}

HRESULT WINAPI extTexUnload(void *t)
{
	HRESULT ret;
	OutTrace("Texture::Unload lpt=%x\n", t);
	ret = (*pTUnload)(t);
	if(ret) OutTraceE("Texture::Load ERROR res=%x(%s)\n", ret, ExplainDDError(ret));
	return ret;
}

typedef struct {
	LPD3DENUMPIXELFORMATSCALLBACK *cb;
	LPVOID arg;
} CallbackZBufArg;

HRESULT WINAPI extZBufferProxy(LPDDPIXELFORMAT lpDDPixFmt, LPVOID lpContext)
{
	HRESULT res;
	OutTraceD3D("EnumZBufferFormats: CALLBACK context=%x %s \n", ((CallbackZBufArg *)lpContext)->arg, ExplainPixelFormat(lpDDPixFmt));
	res = (*(((CallbackZBufArg *)lpContext)->cb))(lpDDPixFmt, ((CallbackZBufArg *)lpContext)->arg);
	OutTraceD3D("EnumZBufferFormats: CALLBACK ret=%x\n", res);
	return res;
}

HRESULT WINAPI extEnumZBufferFormats(void *lpd3d, REFCLSID riidDevice, LPD3DENUMPIXELFORMATSCALLBACK lpEnumCallback, LPVOID lpContext)
{
	HRESULT ret;
	CallbackZBufArg Arg;
	OutTraceD3D("Direct3D::EnumZBufferFormats d3d=%x clsid=%x context=%x\n", lpd3d, riidDevice.Data1, lpContext);
	Arg.cb= &lpEnumCallback;
	Arg.arg=lpContext;
	ret = (*pEnumZBufferFormats)(lpd3d, riidDevice, (LPD3DENUMPIXELFORMATSCALLBACK)extZBufferProxy, (LPVOID)&Arg);
	OutTraceE("Direct3D::EnumZBufferFormats res=%x(%s)\n", ret, ExplainDDError(ret));
	return ret;
}

// Beware: using service surfaces with DDSCAPS_SYSTEMMEMORY capability may lead to crashes in D3D operations
// like Vievport::Clear() in "Forsaken" set in emulation AERO-friendly mode. To avoid the problem, you can 
// suppress the offending cap by use of the NOSYSMEMPRIMARY or NOSYSMEMBACKBUF flags

HRESULT WINAPI extViewportClear(void *lpd3dvp, DWORD p1, LPD3DRECT lpRect, DWORD p2)
{
	HRESULT ret;

	if(IsTraceD3D){
		char sRect[81];
		if (lpRect) sprintf(sRect, "(%d,%d)-(%d,%d)", lpRect->x1, lpRect->y1, lpRect->x2, lpRect->y2);
		else strcpy(sRect, "(NULL)");
		OutTrace("Viewport::Clear lpd3dvp=%x p1=%x p2=%x rect=%s\n", lpd3dvp, p1, p2, sRect);
	}

	// proxying the call ....
	ret = (*pViewportClear)(lpd3dvp, p1, lpRect, p2);

	OutTraceD3D("Viewport::Clear ret=%x\n", ret);
	return ret;
}
