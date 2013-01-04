#include <windows.h>
#include <d3d9.h>
#include "dxwnd.h"
#include "dxwcore.hpp"

typedef void* (WINAPI *Direct3DCreate8_Type)(UINT);
typedef void* (WINAPI *Direct3DCreate9_Type)(UINT);
typedef HRESULT (WINAPI *CreateDevice_Type)(void *, UINT, D3DDEVTYPE, HWND, DWORD, void *, void **);
typedef HRESULT (WINAPI *EnumAdapterModes8_Type)(void *, UINT, UINT, D3DDISPLAYMODE *);
typedef HRESULT (WINAPI *EnumAdapterModes9_Type)(void *, UINT, D3DFORMAT ,UINT, D3DDISPLAYMODE *);
typedef HRESULT (WINAPI *GetAdapterDisplayMode_Type)(void *, UINT, D3DDISPLAYMODE *);
typedef HRESULT (WINAPI *GetDisplayMode_Type)(void *, D3DDISPLAYMODE *);

void* WINAPI extDirect3DCreate8(UINT);
void* WINAPI extDirect3DCreate9(UINT);
HRESULT WINAPI extCreateDevice(void *, UINT, D3DDEVTYPE, HWND, DWORD, D3DPRESENT_PARAMETERS *, void **);
HRESULT WINAPI extEnumAdapterModes8(void *, UINT, UINT , D3DDISPLAYMODE *);
HRESULT WINAPI extEnumAdapterModes9(void *, UINT, D3DFORMAT, UINT , D3DDISPLAYMODE *);
HRESULT WINAPI extGetAdapterDisplayMode(void *, UINT, D3DDISPLAYMODE *);
HRESULT WINAPI extGetDisplayMode(void *, D3DDISPLAYMODE *);

extern char *ExplainDDError(DWORD);

Direct3DCreate8_Type pDirect3DCreate8 = 0;
Direct3DCreate9_Type pDirect3DCreate9 = 0;
CreateDevice_Type pCreateDevice;
EnumAdapterModes8_Type pEnumAdapterModes8;
EnumAdapterModes9_Type pEnumAdapterModes9;
GetAdapterDisplayMode_Type pGetAdapterDisplayMode;
GetDisplayMode_Type pGetDisplayMode;
DWORD dwD3DVersion;

int HookDirect3D(int version){
	HINSTANCE hinst;
	void *tmp;
	LPDIRECT3D9 lpd3d;

	switch(version){
	case 0:
		tmp = HookAPI("d3d8.dll", NULL, "Direct3DCreate8", extDirect3DCreate8);
		if(tmp) pDirect3DCreate8 = (Direct3DCreate8_Type)tmp;
		tmp = HookAPI("d3d9.dll", NULL, "Direct3DCreate9", extDirect3DCreate9);
		if(tmp) pDirect3DCreate9 = (Direct3DCreate9_Type)tmp;
		break;
	case 8:
		hinst = LoadLibrary("d3d8.dll");
		pDirect3DCreate8 =
			(Direct3DCreate8_Type)GetProcAddress(hinst, "Direct3DCreate8");
		if(pDirect3DCreate8){
			lpd3d = (LPDIRECT3D9)extDirect3DCreate8(220);
			if(lpd3d) lpd3d->Release();
		}
		break;
	case 9:
		hinst = LoadLibrary("d3d9.dll");
		pDirect3DCreate9 =
			(Direct3DCreate9_Type)GetProcAddress(hinst, "Direct3DCreate9");
		if(pDirect3DCreate9){
			lpd3d = (LPDIRECT3D9)extDirect3DCreate9(31);
			if(lpd3d) lpd3d->Release();
		}
	break;
	}
	if(pDirect3DCreate8 || pDirect3DCreate9) return 1;
	return 0;
}

void* WINAPI extDirect3DCreate8(UINT sdkversion)
{
	void *lpd3d;

	dwD3DVersion = 8;
	lpd3d = (*pDirect3DCreate8)(sdkversion);
	if(!lpd3d) return 0;
	//OutTrace("DEBUG: Hooking lpd3d=%x\n", lpd3d);
	//OutTrace("DEBUG: Hooking %x -> %x as CreateDevice(D8)\n", (void *)(*(DWORD *)lpd3d + 60), extCreateDevice);
	SetHook((void *)(*(DWORD *)lpd3d + 28), extEnumAdapterModes8, (void **)&pEnumAdapterModes8, "EnumAdapterModes(D8)");
	SetHook((void *)(*(DWORD *)lpd3d + 32), extGetAdapterDisplayMode, (void **)&pGetAdapterDisplayMode, "GetAdapterDisplayMode(D8)");
	SetHook((void *)(*(DWORD *)lpd3d + 60), extCreateDevice, (void **)&pCreateDevice, "CreateDevice(D8)");

	OutTraceD("Direct3DCreate8: SDKVERSION=%x pCreateDevice=%x\n",
		sdkversion, pCreateDevice);
	return lpd3d;
}

void* WINAPI extDirect3DCreate9(UINT sdkversion)
{
	void *lpd3d;

	dwD3DVersion = 9;
	lpd3d = (*pDirect3DCreate9)(sdkversion);
	if(!lpd3d) return 0;
	SetHook((void *)(*(DWORD *)lpd3d + 28), extEnumAdapterModes9, (void **)&pEnumAdapterModes9, "EnumAdapterModes(D9)");
	SetHook((void *)(*(DWORD *)lpd3d + 32), extGetAdapterDisplayMode, (void **)&pGetAdapterDisplayMode, "GetAdapterDisplayMode(D9)");
	SetHook((void *)(*(DWORD *)lpd3d + 64), extCreateDevice, (void **)&pCreateDevice, "CreateDevice(D9)");

	OutTraceD("Direct3DCreate9: SDKVERSION=%x pCreateDevice=%x\n",
		sdkversion, pCreateDevice);

	return lpd3d;
}

HRESULT WINAPI extReset(void *pd3dd, D3DPRESENT_PARAMETERS* pPresentationParameters)
{
	OutTraceD("DEBUG: neutralizing pd3dd->Reset()\n");
	return D3D_OK;
}

HRESULT WINAPI extGetDisplayMode(void *lpd3d, D3DDISPLAYMODE *pMode)
{
	HRESULT res;
	extern short iSizX, iSizY;
	res=(*pGetDisplayMode)(lpd3d, pMode);
	OutTraceD("DEBUG: GetDisplayMode: size=(%dx%d) RefreshRate=%d Format=%d\n",
		pMode->Width, pMode->Height, pMode->RefreshRate, pMode->Format);
	if(dxw.dwFlags2 & KEEPASPECTRATIO){
		pMode->Width=iSizX;
		pMode->Height=iSizY;
		OutTraceD("DEBUG: GetDisplayMode: fixed size=(%dx%d)\n", pMode->Width, pMode->Height);
	}
	return res;
}

HRESULT WINAPI extEnumAdapterModes8(void *lpd3d, UINT Adapter, UINT Mode, D3DDISPLAYMODE* pMode)
{
	HRESULT res;
	OutTraceD("EnumAdapterModes(8): adapter=%d mode=%d pMode=%x\n", Adapter, Mode, pMode);
	res=(*pEnumAdapterModes8)(lpd3d, Adapter, Mode, pMode);
	if(res) OutTraceE("EnumAdapterModes ERROR: err=%x(%s)\n", res, ExplainDDError(res));
	return res;
}

HRESULT WINAPI extEnumAdapterModes9(void *lpd3d, UINT Adapter, D3DFORMAT Format, UINT Mode, D3DDISPLAYMODE* pMode)
{
	HRESULT res;
	OutTraceD("EnumAdapterModes(9): adapter=%d format=%x mode=%d pMode=%x\n", Adapter, Format, Mode, pMode);
	res=(*pEnumAdapterModes9)(lpd3d, Adapter, Format, Mode, pMode);
	if(res) OutTraceE("EnumAdapterModes ERROR: err=%x(%s)\n", res, ExplainDDError(res));
	return res;
}

HRESULT WINAPI extGetAdapterDisplayMode(void *lpd3d, UINT Adapter, D3DDISPLAYMODE *pMode)
{
	HRESULT res;
	extern short iSizX, iSizY;
	res=(*pGetAdapterDisplayMode)(lpd3d, Adapter, pMode);
	OutTraceD("DEBUG: GetAdapterDisplayMode: size=(%dx%d) RefreshRate=%d Format=%d\n",
		pMode->Width, pMode->Height, pMode->RefreshRate, pMode->Format);
	if(dxw.dwFlags2 & KEEPASPECTRATIO){
		pMode->Width=iSizX;
		pMode->Height=iSizY;
		OutTraceD("DEBUG: GetDisplayMode: fixed size=(%dx%d)\n", pMode->Width, pMode->Height);
	}
	return res;
}

HRESULT WINAPI extCreateDevice(void *lpd3d, UINT adapter, D3DDEVTYPE devicetype,
	HWND hfocuswindow, DWORD behaviorflags, D3DPRESENT_PARAMETERS *ppresentparam, void **ppd3dd)
{
	HRESULT res;
	DWORD param[64], *tmp;
	D3DDISPLAYMODE mode;
	int Windowed;

	if(dwD3DVersion == 9){
		memcpy(param, ppresentparam, 56);
		OutTraceD("D3D9::CreateDevice\n");
	}
	else{
		memcpy(param, ppresentparam, 52);
		OutTraceD("D3D8::CreateDevice\n");
	}

	dxw.SethWnd(hfocuswindow);
	dxw.SetScreenSize(param[0], param[1]);
	AdjustWindowFrame(dxw.GethWnd(), dxw.GetScreenWidth(), dxw.GetScreenHeight());

	tmp = param;
	OutTraceD("  Adapter = %i\n", adapter);
	OutTraceD("  DeviceType = %i\n", devicetype);
	OutTraceD("  hFocusWindow = 0x%x\n", hfocuswindow);
	OutTraceD("  BehaviorFlags = 0x%x\n", behaviorflags);
	OutTraceD("    BackBufferWidth = %i\n", *(tmp ++));
	OutTraceD("    BackBufferHeight = %i\n", *(tmp ++));
	OutTraceD("    BackBufferFormat = %i\n", *(tmp ++));
	OutTraceD("    BackBufferCount = %i\n", *(tmp ++));
	OutTraceD("    MultiSampleType = %i\n", *(tmp ++));
	if(dwD3DVersion == 9) OutTraceD("    MultiSampleQuality = %i\n", *(tmp ++));
	OutTraceD("    SwapEffect = 0x%x\n", *(tmp ++));
	OutTraceD("    hDeviceWindow = 0x%x\n", *(tmp ++));
	OutTraceD("    Windowed = %i\n", (Windowed=*(tmp ++)));
	OutTraceD("    EnableAutoDepthStencil = %i\n", *(tmp ++));
	OutTraceD("    AutoDepthStencilFormat = %i\n", *(tmp ++));
	OutTraceD("    Flags = 0x%x\n", *(tmp ++));
	OutTraceD("    FullScreen_RefreshRateInHz = %i\n", *(tmp ++));
	OutTraceD("    PresentationInterval = 0x%x\n", *(tmp ++));

	//((LPDIRECT3D9)lpd3d)->GetAdapterDisplayMode(0, &mode);
	(*pGetAdapterDisplayMode)(lpd3d, 0, &mode);
	param[2] = mode.Format;
	OutTraceD("    Current Format = 0x%x\n", mode.Format);


	if(dwD3DVersion == 9){
		param[7] = 0;			//hDeviceWindow
		dxw.SetFullScreen(~param[8]?TRUE:FALSE); 
		param[8] = 1;			//Windowed
		param[12] = 0;			//FullScreen_RefreshRateInHz;
		param[13] = D3DPRESENT_INTERVAL_DEFAULT;	//PresentationInterval
	}
	else{
		param[6] = 0;			//hDeviceWindow
		dxw.SetFullScreen(~param[7]?TRUE:FALSE); 
		param[7] = 1;			//Windowed
		param[11] = 0;			//FullScreen_RefreshRateInHz;
		param[12] = D3DPRESENT_INTERVAL_DEFAULT;	//PresentationInterval
	}

	res = (*pCreateDevice)(lpd3d, 0, devicetype, hfocuswindow, behaviorflags, param, ppd3dd);
	if(res){
		OutTraceD("FAILED! %x\n", res);
		return res;
	}
	OutTraceD("SUCCESS!\n");

	if(dwD3DVersion == 8){
		void *pReset;
		pReset=NULL; // to avoid assert condition
		SetHook((void *)(*(DWORD *)lpd3d + 32), extGetDisplayMode, (void **)&pGetDisplayMode, "GetDisplayMode(D8)");
		SetHook((void *)(**(DWORD **)ppd3dd + 56), extReset, (void **)&pReset, "Reset(D8)");
	}
	else {
		void *pReset;
		pReset=NULL; // to avoid assert condition
		SetHook((void *)(*(DWORD *)lpd3d + 32), extGetDisplayMode, (void **)&pGetDisplayMode, "GetDisplayMode(D9)");
		SetHook((void *)(**(DWORD **)ppd3dd + 64), extReset, (void **)&pReset, "Reset(D9)");
	}

	DxWndStatus.IsFullScreen = dxw.IsFullScreen();
	DxWndStatus.DXVersion=(short)dwD3DVersion;
	DxWndStatus.Height=(short)dxw.GetScreenHeight();
	DxWndStatus.Width=(short)dxw.GetScreenWidth();
	DxWndStatus.ColorDepth=(short)dxw.VirtualPixelFormat.dwRGBBitCount;
	SetHookStatus(&DxWndStatus);
	
	return 0;
}

