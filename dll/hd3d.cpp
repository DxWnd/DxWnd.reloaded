//#define D3D10_IGNORE_SDK_LAYERS 1
#include <windows.h>
#include <d3d9.h>
#include <D3D10_1.h>
#include <D3D10Misc.h>
#include <D3D11.h>
#include "dxwnd.h"
#include "dxwcore.hpp"
#include "dxhook.h"
#include "syslibs.h"

#define HOOKD3D10ANDLATER 1

typedef HRESULT (WINAPI *QueryInterface_Type)(void *, REFIID riid, void** ppvObj);

typedef void* (WINAPI *Direct3DCreate8_Type)(UINT);
typedef void* (WINAPI *Direct3DCreate9_Type)(UINT);
typedef HRESULT (WINAPI *Direct3DCreate9Ex_Type)(UINT, IDirect3D9Ex **);
typedef HRESULT (WINAPI *CheckFullScreen_Type)(void);

typedef UINT (WINAPI *GetAdapterCount_Type)(void *);
typedef HRESULT (WINAPI *GetAdapterIdentifier_Type)(void *, UINT, DWORD, D3DADAPTER_IDENTIFIER9 *);
typedef HRESULT (WINAPI *CreateDevice_Type)(void *, UINT, D3DDEVTYPE, HWND, DWORD, void *, void **);
typedef HRESULT (WINAPI *CreateDeviceEx_Type)(void *, UINT, D3DDEVTYPE, HWND, DWORD, void *, D3DDISPLAYMODEEX *, void **);
typedef HRESULT (WINAPI *EnumAdapterModes8_Type)(void *, UINT, UINT, D3DDISPLAYMODE *);
typedef HRESULT (WINAPI *EnumAdapterModes9_Type)(void *, UINT, D3DFORMAT ,UINT, D3DDISPLAYMODE *);
typedef HRESULT (WINAPI *GetAdapterDisplayMode_Type)(void *, UINT, D3DDISPLAYMODE *);
typedef HRESULT (WINAPI *GetDisplayMode_Type)(void *, D3DDISPLAYMODE *);
typedef HRESULT (WINAPI *Present_Type)(void *, CONST RECT *, CONST RECT *, HWND, CONST RGNDATA *);
typedef HRESULT (WINAPI *SetRenderState_Type)(void *, D3DRENDERSTATETYPE, DWORD);
typedef HRESULT (WINAPI *GetRenderState_Type)(void *, D3DRENDERSTATETYPE, DWORD );
typedef HRESULT (WINAPI *CreateAdditionalSwapChain_Type)(void *, D3DPRESENT_PARAMETERS *, IDirect3DSwapChain9 **);
typedef HRESULT (WINAPI *GetDirect3D_Type)(void *, IDirect3D9 **);
typedef HRESULT (WINAPI *GetViewport_Type)(void *, D3DVIEWPORT9 *);
typedef HRESULT (WINAPI *SetViewport_Type)(void *, D3DVIEWPORT9 *);
typedef void	(WINAPI *SetGammaRamp_Type)(UINT, DWORD, D3DGAMMARAMP *);

typedef HRESULT (WINAPI *D3D10CreateDevice_Type)(IDXGIAdapter *, D3D10_DRIVER_TYPE, HMODULE, UINT, UINT, ID3D10Device **);
typedef HRESULT (WINAPI *D3D10CreateDeviceAndSwapChain_Type)(IDXGIAdapter *, D3D10_DRIVER_TYPE, HMODULE, UINT, UINT, DXGI_SWAP_CHAIN_DESC *, IDXGISwapChain **, ID3D10Device **);
typedef HRESULT (WINAPI *D3D10CreateDevice1_Type)(IDXGIAdapter *, D3D10_DRIVER_TYPE, HMODULE, UINT, D3D10_FEATURE_LEVEL1, UINT, ID3D10Device **);
typedef HRESULT (WINAPI *D3D10CreateDeviceAndSwapChain1_Type)(IDXGIAdapter *, D3D10_DRIVER_TYPE, HMODULE, UINT, UINT, DXGI_SWAP_CHAIN_DESC *, IDXGISwapChain **, ID3D10Device **);
typedef HRESULT (WINAPI *D3D11CreateDevice_Type)(IDXGIAdapter *, D3D_DRIVER_TYPE, HMODULE, UINT, const D3D_FEATURE_LEVEL *, UINT, UINT, ID3D11Device **, D3D_FEATURE_LEVEL *, ID3D11DeviceContext **);
typedef HRESULT (WINAPI *D3D11CreateDeviceAndSwapChain_Type)(IDXGIAdapter *, D3D_DRIVER_TYPE, HMODULE, UINT, const D3D_FEATURE_LEVEL *, UINT, UINT, const DXGI_SWAP_CHAIN_DESC *, IDXGISwapChain **, ID3D11Device **, D3D_FEATURE_LEVEL *, ID3D11DeviceContext **);
typedef void (WINAPI *RSSetViewports_Type)(void *, UINT, D3D11_VIEWPORT *);

HRESULT WINAPI extQueryInterfaceD3D8(void *, REFIID, void** );
HRESULT WINAPI extQueryInterfaceDev8(void *, REFIID, void** );
HRESULT WINAPI extQueryInterfaceD3D9(void *, REFIID, void** );
HRESULT WINAPI extQueryInterfaceDev9(void *, REFIID, void** );
HRESULT WINAPI extGetDirect3D(void *, IDirect3D9 **);

void* WINAPI extDirect3DCreate8(UINT);
void* WINAPI extDirect3DCreate9(UINT);
HRESULT WINAPI extDirect3DCreate9Ex(UINT, IDirect3D9Ex **);
HRESULT WINAPI extCheckFullScreen(void);

UINT WINAPI extGetAdapterCount(void *);
HRESULT WINAPI extGetAdapterIdentifier(void *, UINT, DWORD, D3DADAPTER_IDENTIFIER9 *);
HRESULT WINAPI extCreateDevice(void *, UINT, D3DDEVTYPE, HWND, DWORD, D3DPRESENT_PARAMETERS *, void **);
HRESULT WINAPI extCreateDeviceEx(void *, UINT, D3DDEVTYPE, HWND, DWORD, D3DPRESENT_PARAMETERS *, D3DDISPLAYMODEEX *, void **);
HRESULT WINAPI extEnumAdapterModes8(void *, UINT, UINT , D3DDISPLAYMODE *);
HRESULT WINAPI extEnumAdapterModes9(void *, UINT, D3DFORMAT, UINT , D3DDISPLAYMODE *);
HRESULT WINAPI extGetAdapterDisplayMode(void *, UINT, D3DDISPLAYMODE *);
HRESULT WINAPI extGetDisplayMode(void *, D3DDISPLAYMODE *);
HRESULT WINAPI extPresent(void *, CONST RECT *, CONST RECT *, HWND, CONST RGNDATA *);
HRESULT WINAPI extSetRenderState(void *, D3DRENDERSTATETYPE, DWORD);
HRESULT WINAPI extGetRenderState(void *, D3DRENDERSTATETYPE, DWORD);
HRESULT WINAPI extCreateAdditionalSwapChain(void *, D3DPRESENT_PARAMETERS *, IDirect3DSwapChain9 **);
HRESULT WINAPI extGetViewport(void *, D3DVIEWPORT9 *);
HRESULT WINAPI extSetViewport(void *, D3DVIEWPORT9 *);
void	WINAPI extSetGammaRamp(UINT, DWORD, D3DGAMMARAMP *);

HRESULT WINAPI extD3D10CreateDevice(IDXGIAdapter *, D3D10_DRIVER_TYPE, HMODULE, UINT, UINT, ID3D10Device **);
HRESULT WINAPI extD3D10CreateDeviceAndSwapChain(IDXGIAdapter *, D3D10_DRIVER_TYPE, HMODULE, UINT, UINT, DXGI_SWAP_CHAIN_DESC *, IDXGISwapChain **, ID3D10Device **);
HRESULT WINAPI extD3D10CreateDevice1(IDXGIAdapter *, D3D10_DRIVER_TYPE, HMODULE, UINT, D3D10_FEATURE_LEVEL1, UINT, ID3D10Device **);
HRESULT WINAPI extD3D10CreateDeviceAndSwapChain1(IDXGIAdapter *, D3D10_DRIVER_TYPE, HMODULE, UINT, UINT, DXGI_SWAP_CHAIN_DESC *, IDXGISwapChain **, ID3D10Device **);
HRESULT WINAPI extD3D11CreateDevice(IDXGIAdapter *, D3D_DRIVER_TYPE, HMODULE, UINT, const D3D_FEATURE_LEVEL *, UINT, UINT, ID3D11Device **, D3D_FEATURE_LEVEL *, ID3D11DeviceContext **);
HRESULT WINAPI extD3D11CreateDeviceAndSwapChain(IDXGIAdapter *, D3D_DRIVER_TYPE, HMODULE, UINT, const D3D_FEATURE_LEVEL *, UINT, UINT, const DXGI_SWAP_CHAIN_DESC *, IDXGISwapChain **, ID3D11Device **, D3D_FEATURE_LEVEL *, ID3D11DeviceContext **);
void WINAPI extRSSetViewports(ID3D11DeviceContext *, UINT, D3D11_VIEWPORT *);

extern char *ExplainDDError(DWORD);

QueryInterface_Type pQueryInterfaceD3D8 = 0;
QueryInterface_Type pQueryInterfaceDev8 = 0;
QueryInterface_Type pQueryInterfaceD3D9 = 0;
QueryInterface_Type pQueryInterfaceDev9 = 0;

GetDirect3D_Type pGetDirect3D = 0;
Direct3DCreate8_Type pDirect3DCreate8 = 0;
Direct3DCreate9_Type pDirect3DCreate9 = 0;
Direct3DCreate9Ex_Type pDirect3DCreate9Ex = 0;
CheckFullScreen_Type pCheckFullScreen = 0;

GetAdapterCount_Type pGetAdapterCount = 0;
GetAdapterIdentifier_Type pGetAdapterIdentifier = 0;
CreateDevice_Type pCreateDevice = 0;
CreateDeviceEx_Type pCreateDeviceEx = 0;
EnumAdapterModes8_Type pEnumAdapterModes8 = 0;
EnumAdapterModes9_Type pEnumAdapterModes9 = 0;
GetAdapterDisplayMode_Type pGetAdapterDisplayMode = 0;
GetDisplayMode_Type pGetDisplayMode = 0;
Present_Type pPresent = 0;
SetRenderState_Type pSetRenderState = 0;
GetRenderState_Type pGetRenderState = 0;
CreateAdditionalSwapChain_Type pCreateAdditionalSwapChain = 0;
GetViewport_Type pGetViewport = 0;
SetViewport_Type pSetViewport = 0;
SetGammaRamp_Type pSetGammaRamp = 0;

D3D10CreateDevice_Type pD3D10CreateDevice = 0;
D3D10CreateDeviceAndSwapChain_Type pD3D10CreateDeviceAndSwapChain = 0;
D3D10CreateDevice1_Type pD3D10CreateDevice1 = 0;
D3D10CreateDeviceAndSwapChain1_Type pD3D10CreateDeviceAndSwapChain1 = 0;
D3D11CreateDevice_Type pD3D11CreateDevice = 0;
D3D11CreateDeviceAndSwapChain_Type pD3D11CreateDeviceAndSwapChain = 0;
RSSetViewports_Type pRSSetViewports = 0;

DWORD dwD3DVersion;

FARPROC Remap_d3d8_ProcAddress(LPCSTR proc, HMODULE hModule)
{
	if (!strcmp(proc,"Direct3DCreate8") && !pDirect3DCreate8){
		pDirect3DCreate8=(Direct3DCreate8_Type)(*pGetProcAddress)(hModule, proc);
		OutTraceD("GetProcAddress: hooking proc=%s at addr=%x\n", ProcToString(proc), pDirect3DCreate8);
		return (FARPROC)extDirect3DCreate8;
	}
	// NULL -> keep the original call address
	return NULL;
}

HRESULT WINAPI voidDirect3DShaderValidatorCreate9(void) 
{
	OutTraceD("Direct3DShaderValidatorCreate9: SUPPRESSED\n");
	return 0;
}

void WINAPI voidDebugSetLevel(void) 
{
	OutTraceD("DebugSetLevel: SUPPRESSED\n");
}

void WINAPI voidDebugSetMute(void) 
{
	OutTraceD("DebugSetMute: SUPPRESSED\n");
}

FARPROC Remap_d3d9_ProcAddress(LPCSTR proc, HMODULE hModule)
{
	// NULL -> keep the original call address
	// (FARPROC)-1 -> returns NULL

	if (!strcmp(proc,"Direct3DCreate9") && !pDirect3DCreate9){
		pDirect3DCreate9=(Direct3DCreate9_Type)(*pGetProcAddress)(hModule, proc);
		OutTraceD("GetProcAddress: hooking proc=%s at addr=%x\n", ProcToString(proc), pDirect3DCreate9);
		return (FARPROC)extDirect3DCreate9;
	}
	if (!strcmp(proc,"Direct3DCreate9Ex") && !pDirect3DCreate9Ex){
		pDirect3DCreate9Ex=(Direct3DCreate9Ex_Type)(*pGetProcAddress)(hModule, proc);
		OutTraceD("GetProcAddress: hooking proc=%s at addr=%x\n", ProcToString(proc), pDirect3DCreate9Ex);
		return (FARPROC)extDirect3DCreate9Ex;
	}
	if (!strcmp(proc,"CheckFullScreen") && !pCheckFullScreen){
		pCheckFullScreen=(CheckFullScreen_Type)(*pGetProcAddress)(hModule, proc);
		OutTraceD("GetProcAddress: hooking proc=%s at addr=%x\n", ProcToString(proc), pCheckFullScreen);
		return (FARPROC)extCheckFullScreen;
	}

	if (!(dxw.dwFlags3 & SUPPRESSD3DEXT)) return NULL;

	if (!strcmp(proc,"Direct3DShaderValidatorCreate9")){
		OutTraceD("GetProcAddress: suppressing Direct3DShaderValidatorCreate9\n");
		return (FARPROC)voidDirect3DShaderValidatorCreate9;
		//return (FARPROC)-1;
	}
	if (!strcmp(proc,"DebugSetLevel")){
		OutTraceD("GetProcAddress: suppressing DebugSetLevel\n");
		return (FARPROC)voidDebugSetLevel;
		//return (FARPROC)-1;
	}
	if (!strcmp(proc,"DebugSetMute")){
		OutTraceD("GetProcAddress: suppressing DebugSetMute\n");
		return (FARPROC)voidDebugSetMute;
		//return (FARPROC)-1;
	}
	return NULL;
}

FARPROC Remap_d3d10_ProcAddress(LPCSTR proc, HMODULE hModule)
{
	if (!strcmp(proc,"D3D10CreateDevice") && !pD3D10CreateDevice){
		pD3D10CreateDevice=(D3D10CreateDevice_Type)(*pGetProcAddress)(hModule, proc);
		OutTraceD("GetProcAddress: hooking proc=%s at addr=%x\n", ProcToString(proc), pD3D10CreateDevice);
		return (FARPROC)extD3D10CreateDevice;
	}
	if (!strcmp(proc,"D3D10CreateDeviceAndSwapChain") && !pD3D10CreateDeviceAndSwapChain){
		pD3D10CreateDeviceAndSwapChain=(D3D10CreateDeviceAndSwapChain_Type)(*pGetProcAddress)(hModule, proc);
		OutTraceD("GetProcAddress: hooking proc=%s at addr=%x\n", ProcToString(proc), pD3D10CreateDeviceAndSwapChain);
		return (FARPROC)extD3D10CreateDeviceAndSwapChain;
	}
	// NULL -> keep the original call address
	return NULL;
}

FARPROC Remap_d3d10_1_ProcAddress(LPCSTR proc, HMODULE hModule)
{
	if (!strcmp(proc,"D3D10CreateDevice1") && !pD3D10CreateDevice1){
		pD3D10CreateDevice1=(D3D10CreateDevice1_Type)(*pGetProcAddress)(hModule, proc);
		OutTraceD("GetProcAddress: hooking proc=%s at addr=%x\n", ProcToString(proc), pD3D10CreateDevice1);
		return (FARPROC)extD3D10CreateDevice1;
	}
	if (!strcmp(proc,"D3D10CreateDeviceAndSwapChain1") && !pD3D10CreateDeviceAndSwapChain1){
		pD3D10CreateDeviceAndSwapChain1=(D3D10CreateDeviceAndSwapChain1_Type)(*pGetProcAddress)(hModule, proc);
		OutTraceD("GetProcAddress: hooking proc=%s at addr=%x\n", ProcToString(proc), pD3D10CreateDeviceAndSwapChain1);
		return (FARPROC)extD3D10CreateDeviceAndSwapChain1;
	}
	// NULL -> keep the original call address
	return NULL;
}

FARPROC Remap_d3d11_ProcAddress(LPCSTR proc, HMODULE hModule)
{
	if (!strcmp(proc,"D3D11CreateDevice") && !pD3D11CreateDevice){
		pD3D11CreateDevice=(D3D11CreateDevice_Type)(*pGetProcAddress)(hModule, proc);
		OutTraceD("GetProcAddress: hooking proc=%s at addr=%x\n", ProcToString(proc), pD3D11CreateDevice);
		return (FARPROC)extD3D11CreateDevice;
	}
	if (!strcmp(proc,"D3D11CreateDeviceAndSwapChain") && !pD3D11CreateDeviceAndSwapChain){
		pD3D11CreateDeviceAndSwapChain=(D3D11CreateDeviceAndSwapChain_Type)(*pGetProcAddress)(hModule, proc);
		OutTraceD("GetProcAddress: hooking proc=%s at addr=%x\n", ProcToString(proc), pD3D11CreateDeviceAndSwapChain);
		return (FARPROC)extD3D11CreateDeviceAndSwapChain;
	}
	// NULL -> keep the original call address
	return NULL;
}

int HookDirect3D(HMODULE module, int version){
	HINSTANCE hinst;
	void *tmp;
	LPDIRECT3D9 lpd3d;
	ID3D10Device *lpd3d10;
	ID3D11Device *lpd3d11;
	HRESULT res;

	switch(version){
	case 0:
		// D3D8
		tmp = HookAPI(module, "d3d8.dll", NULL, "Direct3DCreate8", extDirect3DCreate8);
		if(tmp) pDirect3DCreate8 = (Direct3DCreate8_Type)tmp;
		// D3D9
		tmp = HookAPI(module, "d3d9.dll", NULL, "Direct3DCreate9", extDirect3DCreate9);
		if(tmp) pDirect3DCreate9 = (Direct3DCreate9_Type)tmp;
		tmp = HookAPI(module, "d3d9.dll", NULL, "Direct3DCreate9Ex", extDirect3DCreate9Ex);
		if(tmp) pDirect3DCreate9Ex = (Direct3DCreate9Ex_Type)tmp;
		tmp = HookAPI(module, "d3d9.dll", NULL, "CheckFullScreen", extCheckFullScreen);
		if(tmp) pCheckFullScreen = (CheckFullScreen_Type)tmp;
#ifdef HOOKD3D10ANDLATER
		// D3D10
		tmp = HookAPI(module, "d3d10.dll", NULL, "D3D10CreateDevice", extD3D10CreateDevice);
		if(tmp) pD3D10CreateDevice = (D3D10CreateDevice_Type)tmp;
		tmp = HookAPI(module, "d3d10.dll", NULL, "D3D10CreateDeviceAndSwapChain", extD3D10CreateDeviceAndSwapChain);
		if(tmp) pD3D10CreateDeviceAndSwapChain = (D3D10CreateDeviceAndSwapChain_Type)tmp;
		// D3D10.1
		tmp = HookAPI(module, "d3d10_1.dll", NULL, "D3D10CreateDevice1", extD3D10CreateDevice1);
		if(tmp) pD3D10CreateDevice1 = (D3D10CreateDevice1_Type)tmp;
		tmp = HookAPI(module, "d3d10_1.dll", NULL, "D3D10CreateDeviceAndSwapChain1", extD3D10CreateDeviceAndSwapChain);
		if(tmp) pD3D10CreateDeviceAndSwapChain1 = (D3D10CreateDeviceAndSwapChain1_Type)tmp;
		// D3D11
		tmp = HookAPI(module, "d3d11.dll", NULL, "D3D11CreateDevice", extD3D11CreateDevice);
		if(tmp) pD3D11CreateDevice = (D3D11CreateDevice_Type)tmp;
		tmp = HookAPI(module, "d3d11.dll", NULL, "D3D11CreateDeviceAndSwapChain", extD3D11CreateDeviceAndSwapChain);
		if(tmp) pD3D11CreateDeviceAndSwapChain = (D3D11CreateDeviceAndSwapChain_Type)tmp;
#endif
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
		pDirect3DCreate9Ex =
			(Direct3DCreate9Ex_Type)GetProcAddress(hinst, "Direct3DCreate9Ex");
		pCheckFullScreen = 
			(CheckFullScreen_Type)GetProcAddress(hinst, "CheckFullScreen");
		if(pDirect3DCreate9){
			lpd3d = (LPDIRECT3D9)extDirect3DCreate9(31);
			if(lpd3d) lpd3d->Release();
		}
		break;
#ifdef HOOKD3D10ANDLATER
	case 10:
		hinst = LoadLibrary("d3d10.dll");
		pD3D10CreateDevice =
			(D3D10CreateDevice_Type)GetProcAddress(hinst, "D3D10CreateDevice");
		if(pD3D10CreateDevice){
			res = extD3D10CreateDevice(
				NULL,
				D3D10_DRIVER_TYPE_HARDWARE,
				NULL,
				0,
				D3D10_SDK_VERSION,
				&lpd3d10);
			if(res==DD_OK) lpd3d10->Release();
		}
		hinst = LoadLibrary("d3d10_1.dll");
		pD3D10CreateDevice1 =
			(D3D10CreateDevice1_Type)GetProcAddress(hinst, "D3D10CreateDevice1");
		break;
		if(pD3D10CreateDevice1){
			res = extD3D10CreateDevice1(
				NULL,
				D3D10_DRIVER_TYPE_HARDWARE,
				NULL,
				0,
				D3D10_FEATURE_LEVEL_10_1,
				D3D10_SDK_VERSION,
				&lpd3d10);
			if(res==DD_OK) lpd3d10->Release();
		}
		pD3D10CreateDeviceAndSwapChain =
			(D3D10CreateDeviceAndSwapChain_Type)GetProcAddress(hinst, "D3D10CreateDeviceAndSwapChain");
		if(pD3D10CreateDeviceAndSwapChain){
			DXGI_SWAP_CHAIN_DESC swapChainDesc;
			IDXGISwapChain *pSwapChain;
			ZeroMemory(&swapChainDesc, sizeof(swapChainDesc));
			 
			//set buffer dimensions and format
			swapChainDesc.BufferCount = 2;
			swapChainDesc.BufferDesc.Width = dxw.GetScreenWidth();
			swapChainDesc.BufferDesc.Height = dxw.GetScreenHeight();
			swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
			swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;;
			 
			//set refresh rate
			swapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
			swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
			 
			//sampling settings
			swapChainDesc.SampleDesc.Quality = 0;
			swapChainDesc.SampleDesc.Count = 1;
			 
			//output window handle
			swapChainDesc.OutputWindow = dxw.GethWnd();
			swapChainDesc.Windowed = true;	
			res = extD3D10CreateDeviceAndSwapChain( 
				NULL,
                D3D10_DRIVER_TYPE_HARDWARE,
                NULL,
                0,
                D3D10_SDK_VERSION,
                &swapChainDesc,
                &pSwapChain,
                &lpd3d10);
			if(res==DD_OK) lpd3d10->Release();
		}
	break;
	case 11:
		hinst = LoadLibrary("d3d11.dll");
		pD3D11CreateDevice =
			(D3D11CreateDevice_Type)GetProcAddress(hinst, "D3D11CreateDevice");
		if(pD3D11CreateDevice){
			D3D_FEATURE_LEVEL FeatureLevel;
			ID3D11DeviceContext *pImmediateContext;
			res = extD3D11CreateDevice(
				NULL,
				D3D_DRIVER_TYPE_HARDWARE,
				NULL,
				0, // flags
				NULL, // FeatureLevels
				0,
				D3D11_SDK_VERSION,
				&lpd3d11,
				&FeatureLevel,
				&pImmediateContext);
			if(res==DD_OK) lpd3d11->Release();
		}
		pD3D11CreateDeviceAndSwapChain =
			(D3D11CreateDeviceAndSwapChain_Type)GetProcAddress(hinst, "D3D11CreateDeviceAndSwapChain");
		break;
#endif
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
	SetHook((void *)(*(DWORD *)lpd3d +  0), extQueryInterfaceD3D8, (void **)&pQueryInterfaceD3D8, "QueryInterface(D8)");
	SetHook((void *)(*(DWORD *)lpd3d + 16), extGetAdapterCount, (void **)&pGetAdapterCount, "GetAdapterCount(D8)");
	SetHook((void *)(*(DWORD *)lpd3d + 20), extGetAdapterIdentifier, (void **)&pGetAdapterIdentifier, "GetAdapterIdentifier(D8)");
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
	SetHook((void *)(*(DWORD *)lpd3d +  0), extQueryInterfaceD3D9, (void **)&pQueryInterfaceD3D9, "QueryInterface(D9)");
	SetHook((void *)(*(DWORD *)lpd3d + 16), extGetAdapterCount, (void **)&pGetAdapterCount, "GetAdapterCount(D9)");
	SetHook((void *)(*(DWORD *)lpd3d + 20), extGetAdapterIdentifier, (void **)&pGetAdapterIdentifier, "GetAdapterIdentifier(D9)");
	SetHook((void *)(*(DWORD *)lpd3d + 28), extEnumAdapterModes9, (void **)&pEnumAdapterModes9, "EnumAdapterModes(D9)");
	SetHook((void *)(*(DWORD *)lpd3d + 32), extGetAdapterDisplayMode, (void **)&pGetAdapterDisplayMode, "GetAdapterDisplayMode(D9)");
	SetHook((void *)(*(DWORD *)lpd3d + 64), extCreateDevice, (void **)&pCreateDevice, "CreateDevice(D9)");

	OutTraceD("Direct3DCreate9: SDKVERSION=%x pCreateDevice=%x\n",
		sdkversion, pCreateDevice);

	return lpd3d;
}

HRESULT WINAPI extDirect3DCreate9Ex(UINT sdkversion, IDirect3D9Ex **ppD3D)
{
	void *lpd3d;
	HRESULT res;

	dwD3DVersion = 9;
	res = (*pDirect3DCreate9Ex)(sdkversion, ppD3D);
	if(res) {
		OutTraceD("Direct3DCreate9Ex: ERROR res=%x SDKVERSION=%x\n", res, sdkversion);
		return res;
	}
	lpd3d = *ppD3D;
	SetHook((void *)(*(DWORD *)lpd3d +  0), extQueryInterfaceD3D9, (void **)&pQueryInterfaceD3D9, "QueryInterface(D9)");
	SetHook((void *)(*(DWORD *)lpd3d + 16), extGetAdapterCount, (void **)&pGetAdapterCount, "GetAdapterCount(D9)");
	SetHook((void *)(*(DWORD *)lpd3d + 20), extGetAdapterIdentifier, (void **)&pGetAdapterIdentifier, "GetAdapterIdentifier(D9)");
	SetHook((void *)(*(DWORD *)lpd3d + 28), extEnumAdapterModes9, (void **)&pEnumAdapterModes9, "EnumAdapterModes(D9)");
	SetHook((void *)(*(DWORD *)lpd3d + 32), extGetAdapterDisplayMode, (void **)&pGetAdapterDisplayMode, "GetAdapterDisplayMode(D9)");
	SetHook((void *)(*(DWORD *)lpd3d + 64), extCreateDevice, (void **)&pCreateDevice, "CreateDevice(D9)");
	SetHook((void *)(*(DWORD *)lpd3d + 80), extCreateDeviceEx, (void **)&pCreateDeviceEx, "CreateDeviceEx(D9)");

	OutTraceD("Direct3DCreate9Ex: SDKVERSION=%x pCreateDeviceEx=%x\n",
		sdkversion, pDirect3DCreate9Ex);

	return res;
}

UINT WINAPI extGetAdapterCount(void *lpd3d)
{
	UINT res;
	res=(*pGetAdapterCount)(lpd3d);
	OutTraceD("GetAdapterCount: count=%d\n", res);
	if(dxw.dwFlags2 & HIDEMULTIMONITOR) {
		OutTraceD("GetAdapterCount: HIDEMULTIMONITOR count=1\n");
		res=1;
	}
	return res;
}

HRESULT WINAPI extGetAdapterIdentifier(void *pd3dd, UINT Adapter, DWORD Flags, D3DADAPTER_IDENTIFIER9 *pIdentifier)
{
	HRESULT res;
	OutTraceD("GetAdapterIdentifier: Adapter=%d flags=%x\n", Adapter, Flags);
	res=pGetAdapterIdentifier(pd3dd, Adapter, Flags, pIdentifier);
	OutTraceD("GetAdapterIdentifier: ret=%x\n", res);
	return res;
}

HRESULT WINAPI extReset(void *pd3dd, D3DPRESENT_PARAMETERS* pPresentationParameters)
{
	OutTraceD("Reset: SUPPRESSED\n");
	return D3D_OK;
}

HRESULT WINAPI extPresent(void *pd3dd, CONST RECT *pSourceRect, CONST RECT *pDestRect, HWND hDestWindowOverride, CONST RGNDATA *pDirtyRegion)
{
	HRESULT res;
	OutTraceB("Present\n");
	// frame counter handling....
	if (dxw.HandleFPS()) return D3D_OK;
	// proxy ....
	res=(*pPresent)(pd3dd, pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion);
	if (dxw.dwFlags2 & SHOWFPSOVERLAY) dxw.ShowFPS();
	return res;
}

HRESULT WINAPI extGetDisplayMode(void *lpd3d, D3DDISPLAYMODE *pMode)
{
	HRESULT res;
	res=(*pGetDisplayMode)(lpd3d, pMode);
	OutTraceD("DEBUG: GetDisplayMode: size=(%dx%d) RefreshRate=%d Format=%d\n",
		pMode->Width, pMode->Height, pMode->RefreshRate, pMode->Format);
	if(dxw.dwFlags2 & KEEPASPECTRATIO){
		pMode->Width=dxw.iSizX;
		pMode->Height=dxw.iSizY;
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
	res=(*pGetAdapterDisplayMode)(lpd3d, Adapter, pMode);
	OutTraceD("DEBUG: GetAdapterDisplayMode: size=(%dx%d) RefreshRate=%d Format=%d\n",
		pMode->Width, pMode->Height, pMode->RefreshRate, pMode->Format);
	if(dxw.dwFlags2 & KEEPASPECTRATIO){
		pMode->Width=dxw.iSizX;
		pMode->Height=dxw.iSizY;
		OutTraceD("DEBUG: GetDisplayMode: fixed size=(%dx%d)\n", pMode->Width, pMode->Height);
	}
	return res;
}

//HRESULT extValidateDevice(void *, DWORD *pNumPasses);
//HRESULT extGetDeviceCaps(void *, D3DCAPS9 *pCaps);
//typedef HRESULT (WINAPI *ValidateDevice_Type)(void *, DWORD *);
//typedef HRESULT (WINAPI *GetDeviceCapsD3D_Type)(void *, D3DCAPS9 *);
//ValidateDevice_Type pValidateDevice;
//GetDeviceCapsD3D_Type pGetDeviceCapsD3D;
//HRESULT extValidateDevice(void *lpd3dd, DWORD *pNumPasses)
//{
//	OutTrace("ValidateDevice hooked\n");
//	return (*pValidateDevice)(lpd3dd, pNumPasses);
//}
//HRESULT extGetDeviceCapsD3D(void *lpd3dd, D3DCAPS9 *pCaps)
//{
//	OutTrace("GetDeviceCaps hooked\n");
//	return (*pGetDeviceCapsD3D)(lpd3dd, pCaps);
//}
HRESULT WINAPI extProbe(void *lpd3dd)
{
	OutTrace("Probe: %x\n", lpd3dd);
	return 0;
}

HRESULT WINAPI extCreateDevice(void *lpd3d, UINT adapter, D3DDEVTYPE devicetype,
	HWND hfocuswindow, DWORD behaviorflags, D3DPRESENT_PARAMETERS *ppresentparam, void **ppd3dd)
{
	HRESULT res;
	DWORD param[64], *tmp;
	D3DDISPLAYMODE mode;
	int Windowed;

	memcpy(param, ppresentparam, (dwD3DVersion == 9)?56:52);
	dxw.SethWnd(hfocuswindow);
	dxw.SetScreenSize(param[0], param[1]);
	AdjustWindowFrame(dxw.GethWnd(), dxw.GetScreenWidth(), dxw.GetScreenHeight());

	if(dxw.dwFlags3 & FIXD3DFRAME){
		char ClassName[81];
		RECT workarea;
		GetClassName(dxw.GethWnd(), ClassName, 80);
		(*pGetClientRect)(dxw.GethWnd(), &workarea);
		if ((dxw.dwFlags2 & KEEPASPECTRATIO) && (dxw.Coordinates == DXW_DESKTOP_WORKAREA)) {
			int w, h, b; // width, height and border
			w = workarea.right - workarea.left;
			h = workarea.bottom - workarea.top;
			if ((w * 600) > (h * 800)){
				b = (w - (h * 800 / 600))/2;
				workarea.left += b;
				workarea.right -= b;
			}
			else {
				b = (h - (w * 600 / 800))/2;
				workarea.top += b;
				workarea.bottom -= b;
			}
		}
		hfocuswindow=(*pCreateWindowExA)(
			0, ClassName, "child", 
			WS_CHILD|WS_VISIBLE, 
			//GetSystemMetrics(SM_CXSIZEFRAME), GetSystemMetrics(SM_CYSIZEFRAME)+GetSystemMetrics(SM_CYCAPTION), 
			workarea.left, workarea.top, workarea.right-workarea.left, workarea.bottom-workarea.top, 
			dxw.GethWnd(), NULL, NULL, NULL);
		if (hfocuswindow) 
			OutTraceD("CreateDevice: updated hfocuswindow=%x pos=(%d,%d) size=(%d,%d)\n", 
				hfocuswindow, workarea.left, workarea.top, workarea.right-workarea.left, workarea.bottom-workarea.top);
		else
			OutTraceD("CreateDevice: CreateWindowEx ERROR err=%d\n", GetLastError());
		dxw.SethWnd(hfocuswindow, dxw.GethWnd());
	}

	tmp = param;
	OutTraceD("D3D%d::CreateDevice\n", dwD3DVersion);
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

	//param[0]=param[1]=0;

	if(dwD3DVersion == 9){
		param[7] = 0;			//hDeviceWindow
		dxw.SetFullScreen(~param[8]?TRUE:FALSE); 
		param[8] = 1;			//Windowed
		//param[11] = D3DPRESENTFLAG_DEVICECLIP;			//Flags;
		param[12] = 0;			//FullScreen_RefreshRateInHz;
		param[13] = D3DPRESENT_INTERVAL_DEFAULT;	//PresentationInterval
	}
	else{
		param[6] = 0;			//hDeviceWindow
		dxw.SetFullScreen(~param[7]?TRUE:FALSE); 
		param[7] = 1;			//Windowed
		//param[10] = D3DPRESENTFLAG_DEVICECLIP;			//Flags;
		param[11] = 0;			//FullScreen_RefreshRateInHz;
		param[12] = D3DPRESENT_INTERVAL_DEFAULT;	//PresentationInterval
	}

	res = (*pCreateDevice)(lpd3d, 0, devicetype, hfocuswindow, behaviorflags, param, ppd3dd);
	if(res){
		OutTraceD("FAILED! %x\n", res);
		return res;
	}
	OutTraceD("SUCCESS! device=%x\n", *ppd3dd);

	if(dwD3DVersion == 8){ 
		void *pReset;
		pReset=NULL; // to avoid assert condition
		SetHook((void *)(**(DWORD **)ppd3dd +  0), extQueryInterfaceDev8, (void **)&pQueryInterfaceDev8, "QueryInterface(D8)");
		SetHook((void *)(**(DWORD **)ppd3dd + 24), extGetDirect3D, (void **)&pGetDirect3D, "GetDirect3D(D8)");
		//SetHook((void *)(**(DWORD **)ppd3dd + 32), extGetDisplayMode, (void **)&pGetDisplayMode, "GetDisplayMode(D8)");
		SetHook((void *)(**(DWORD **)ppd3dd + 52), extCreateAdditionalSwapChain, (void **)&pCreateAdditionalSwapChain, "CreateAdditionalSwapChain(D8)");
		SetHook((void *)(**(DWORD **)ppd3dd + 56), extReset, (void **)&pReset, "Reset(D8)");
		SetHook((void *)(**(DWORD **)ppd3dd + 60), extPresent, (void **)&pPresent, "Present(D8)");
		SetHook((void *)(**(DWORD **)ppd3dd + 72), extSetGammaRamp, (void **)&pSetGammaRamp, "SetGammaRamp(D8)");
		if(dxw.dwFlags2 & WIREFRAME){
			SetHook((void *)(**(DWORD **)ppd3dd + 200), extSetRenderState, (void **)&pSetRenderState, "SetRenderState(D8)");
			SetHook((void *)(**(DWORD **)ppd3dd + 204), extGetRenderState, (void **)&pGetRenderState, "GetRenderState(D8)");
			(*pSetRenderState)((void *)*ppd3dd, D3DRS_FILLMODE, D3DFILL_WIREFRAME);
		}
	}
	else {
		void *pReset;
		pReset=NULL; // to avoid assert condition
		SetHook((void *)(**(DWORD **)ppd3dd +  0), extQueryInterfaceDev9, (void **)&pQueryInterfaceDev9, "QueryInterface(D9)");
		SetHook((void *)(**(DWORD **)ppd3dd + 24), extGetDirect3D, (void **)&pGetDirect3D, "GetDirect3D(D9)");
		//SetHook((void *)(**(DWORD **)ppd3dd + 32), extGetDisplayMode, (void **)&pGetDisplayMode, "GetDisplayMode(D9)");
		SetHook((void *)(**(DWORD **)ppd3dd + 52), extCreateAdditionalSwapChain, (void **)&pCreateAdditionalSwapChain, "CreateAdditionalSwapChain(D9)");
		SetHook((void *)(**(DWORD **)ppd3dd + 64), extReset, (void **)&pReset, "Reset(D9)");
		SetHook((void *)(**(DWORD **)ppd3dd + 68), extPresent, (void **)&pPresent, "Present(D9)");
		SetHook((void *)(**(DWORD **)ppd3dd + 84), extSetGammaRamp, (void **)&pSetGammaRamp, "SetGammaRamp(D9)");
		//SetHook((void *)(**(DWORD **)ppd3dd +188), extSetViewport, (void **)&pSetViewport, "SetViewport(D9)");
		//SetHook((void *)(**(DWORD **)ppd3dd +192), extGetViewport, (void **)&pGetViewport, "GetViewport(D9)");
		if(dxw.dwFlags2 & WIREFRAME){
			SetHook((void *)(**(DWORD **)ppd3dd + 228), extSetRenderState, (void **)&pSetRenderState, "SetRenderState(D9)");
			SetHook((void *)(**(DWORD **)ppd3dd + 232), extGetRenderState, (void **)&pGetRenderState, "GetRenderState(D9)");
			(*pSetRenderState)((void *)*ppd3dd, D3DRS_FILLMODE, D3DFILL_WIREFRAME);
		}
		// experiments ....
		//SetHook((void *)(**(DWORD **)ppd3dd +280), extValidateDevice, (void **)&pValidateDevice, "ValidateDevice(D9)");
		//SetHook((void *)(**(DWORD **)ppd3dd + 28), extGetDeviceCapsD3D, (void **)&pGetDeviceCapsD3D, "GetDeviceCaps(D9)");
		//void *pNull=0;
		//SetHook((void *)(**(DWORD **)ppd3dd + 344), extProbe, &pNull, "Probe-CreateVertexDeclaration(D9)");
		//SetHook((void *)(**(DWORD **)ppd3dd + 28), extProbe, &pNull, "Probe-GetDeviceCaps(D9)");
	}

	GetHookInfo()->IsFullScreen = dxw.IsFullScreen();
	GetHookInfo()->DXVersion=(short)dwD3DVersion;
	GetHookInfo()->Height=(short)dxw.GetScreenHeight();
	GetHookInfo()->Width=(short)dxw.GetScreenWidth();
	GetHookInfo()->ColorDepth=(short)dxw.VirtualPixelFormat.dwRGBBitCount;
	
	return 0;
}

HRESULT WINAPI extCreateDeviceEx(void *lpd3d, UINT adapter, D3DDEVTYPE devicetype,
	HWND hfocuswindow, DWORD behaviorflags, D3DPRESENT_PARAMETERS *ppresentparam, D3DDISPLAYMODEEX *pFullscreenDisplayMode, void **ppd3dd)
{
	HRESULT res;
	DWORD param[64], *tmp;
	D3DDISPLAYMODE mode;
	int Windowed;

	memcpy(param, ppresentparam, 56);
	dxw.SethWnd(hfocuswindow);
	dxw.SetScreenSize(param[0], param[1]);
	AdjustWindowFrame(dxw.GethWnd(), dxw.GetScreenWidth(), dxw.GetScreenHeight());

	tmp = param;
	OutTraceD("D3D%d::CreateDeviceEx\n", dwD3DVersion);
	OutTraceD("  Adapter = %i\n", adapter);
	OutTraceD("  DeviceType = %i\n", devicetype);
	OutTraceD("  hFocusWindow = 0x%x\n", hfocuswindow);
	OutTraceD("  BehaviorFlags = 0x%x\n", behaviorflags);
	OutTraceD("    BackBufferWidth = %i\n", *(tmp ++));
	OutTraceD("    BackBufferHeight = %i\n", *(tmp ++));
	OutTraceD("    BackBufferFormat = %i\n", *(tmp ++));
	OutTraceD("    BackBufferCount = %i\n", *(tmp ++));
	OutTraceD("    MultiSampleType = %i\n", *(tmp ++));
	OutTraceD("    MultiSampleQuality = %i\n", *(tmp ++));
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

	//param[7] = 0;			//hDeviceWindow
	param[7] = (DWORD)dxw.GethWnd();			//hDeviceWindow
	dxw.SetFullScreen(~param[8]?TRUE:FALSE); 
	param[8] = 1;			//Windowed
	param[12] = 0;			//FullScreen_RefreshRateInHz;
	param[13] = D3DPRESENT_INTERVAL_DEFAULT;	//PresentationInterval

	res = (*pCreateDeviceEx)(lpd3d, 0, devicetype, hfocuswindow, behaviorflags, param, pFullscreenDisplayMode, ppd3dd);
	if(res){
		OutTraceD("FAILED! %x\n", res);
		return res;
	}
	OutTraceD("SUCCESS!\n");

	void *pReset;
	pReset=NULL; // to avoid assert condition
	SetHook((void *)(**(DWORD **)ppd3dd +  0), extQueryInterfaceDev9, (void **)&pQueryInterfaceDev9, "QueryInterface(D9)");
	SetHook((void *)(**(DWORD **)ppd3dd + 32), extGetDisplayMode, (void **)&pGetDisplayMode, "GetDisplayMode(D9)");
	SetHook((void *)(**(DWORD **)ppd3dd + 52), extCreateAdditionalSwapChain, (void **)&pCreateAdditionalSwapChain, "CreateAdditionalSwapChain(D9)");
	SetHook((void *)(**(DWORD **)ppd3dd + 64), extReset, (void **)&pReset, "Reset(D9)");
	SetHook((void *)(**(DWORD **)ppd3dd + 68), extPresent, (void **)&pPresent, "Present(D9)");
	SetHook((void *)(**(DWORD **)ppd3dd + 84), extSetGammaRamp, (void **)&pSetGammaRamp, "SetGammaRamp(D9)");
	//SetHook((void *)(**(DWORD **)ppd3dd +188), extSetViewport, (void **)&pSetViewport, "SetViewport(D9)");
	//SetHook((void *)(**(DWORD **)ppd3dd +192), extGetViewport, (void **)&pGetViewport, "GetViewport(D9)");
	if(dxw.dwFlags2 & WIREFRAME){
		SetHook((void *)(**(DWORD **)ppd3dd + 228), extSetRenderState, (void **)&pSetRenderState, "SetRenderState(D9)");
		SetHook((void *)(**(DWORD **)ppd3dd + 232), extGetRenderState, (void **)&pGetRenderState, "GetRenderState(D9)");
		(*pSetRenderState)((void *)*ppd3dd, D3DRS_FILLMODE, D3DFILL_WIREFRAME);
	}

	GetHookInfo()->IsFullScreen = dxw.IsFullScreen();
	GetHookInfo()->DXVersion=(short)dwD3DVersion;
	GetHookInfo()->Height=(short)dxw.GetScreenHeight();
	GetHookInfo()->Width=(short)dxw.GetScreenWidth();
	GetHookInfo()->ColorDepth=(short)dxw.VirtualPixelFormat.dwRGBBitCount;
	
	return 0;
}

HRESULT WINAPI extSetRenderState(void *pd3dd, D3DRENDERSTATETYPE State, DWORD Value) 
{
	if (State == D3DRS_FILLMODE) Value=D3DFILL_WIREFRAME;
	return (*pSetRenderState)(pd3dd, State, Value);
}

HRESULT WINAPI extGetRenderState(void *pd3dd, D3DRENDERSTATETYPE State, DWORD Value) 
{
	return (*pGetRenderState)(pd3dd, State, Value);
}

HRESULT WINAPI extGetViewport(void *pd3dd, D3DVIEWPORT9 *pViewport)
{
	return (*pGetViewport)(pd3dd, pViewport);
}

HRESULT WINAPI extSetViewport(void *pd3dd, D3DVIEWPORT9 *pViewport)
{
	RECT client;
	POINT p={0,0};
	(*pGetClientRect)(dxw.GethWnd(), &client);
	OutTraceD("SetViewport: declared pos=(%d,%d) size=(%d,%d) depth=(%f;%f)\n", 
		pViewport->X, pViewport->Y, pViewport->Width, pViewport->Height, pViewport->MinZ, pViewport->MaxZ);
	if(IsDebug) OutTrace("SetViewport: DEBUG win=(%d,%d) screen=(%d,%d)\n",
		client.right, client.bottom, dxw.GetScreenWidth(), dxw.GetScreenHeight());
	pViewport->X = (pViewport->X * (int)client.right) / (int)dxw.GetScreenWidth();
	pViewport->Y = (pViewport->Y * (int)client.bottom) / (int)dxw.GetScreenHeight();
	pViewport->Width = (pViewport->Width * (int)client.right) / (int)dxw.GetScreenWidth();
	pViewport->Height = (pViewport->Height * (int)client.bottom) / (int)dxw.GetScreenHeight();
	OutTraceD("SetViewport: remapped pos=(%d,%d) size=(%d,%d)\n", 
		pViewport->X, pViewport->Y, pViewport->Width, pViewport->Height);
	return (*pSetViewport)(pd3dd, pViewport);
}

HRESULT WINAPI extCreateAdditionalSwapChain(void *lpd3d, D3DPRESENT_PARAMETERS *pPresentationParameters, IDirect3DSwapChain9 **ppSwapChain)
{
	HRESULT res;
	DWORD param[64], *tmp;
	D3DDISPLAYMODE mode;
	int Windowed;

	memcpy(param, pPresentationParameters, (dwD3DVersion == 9)?56:52);
	dxw.SetScreenSize(param[0], param[1]);
	AdjustWindowFrame(dxw.GethWnd(), dxw.GetScreenWidth(), dxw.GetScreenHeight());

	tmp = param;
	OutTraceD("D3D%d::CreateAdditionalSwapChain\n", dwD3DVersion);
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
	res=(*pCreateAdditionalSwapChain)(lpd3d, (D3DPRESENT_PARAMETERS *)param, ppSwapChain);
	if(res) OutTraceE("CreateAdditionalSwapChain ERROR: res=%x\n", res);
	return res;
}

// to do:
// hook SetCursorPosition ShowCursor to handle cursor

HRESULT WINAPI extD3D10CreateDevice(
	IDXGIAdapter *pAdapter, 
	D3D10_DRIVER_TYPE DriverType, 
	HMODULE Software, 
	UINT Flags, 
	UINT SDKVersion, 
	ID3D10Device **ppDevice)
{
	HRESULT res;
	D3D10_VIEWPORT ViewPort;
	OutTraceD("D3D10CreateDevice: DriverType=%x Flags=%x SDKVersion=%x\n", DriverType, Flags, SDKVersion);
	//return 0x887a0004;
	res=(*pD3D10CreateDevice)(pAdapter, DriverType, Software, Flags, SDKVersion, ppDevice);
	if(res){
		OutTraceD("D3D10CreateDevice: ret=%x\n", res);
		return res;
	}
	SetHook((void *)(*(DWORD *)*ppDevice + 100), extRSSetViewports, (void **)&pRSSetViewports, "RSSetViewports(D10)");
	ViewPort.TopLeftX=dxw.iPosX;
    ViewPort.TopLeftY=dxw.iPosY;
    ViewPort.Width=dxw.iSizX;
    ViewPort.Height=dxw.iSizY;
    ViewPort.MinDepth=1.0;
    ViewPort.MaxDepth=1.0;
	(*pRSSetViewports)((void *)*ppDevice, 1, (D3D11_VIEWPORT *)&ViewPort);
	OutTraceD("D3D10CreateDevice: ret=%x\n", res);
	return res;
}

HRESULT WINAPI extD3D10CreateDevice1(
	IDXGIAdapter *pAdapter, 
	D3D10_DRIVER_TYPE DriverType, 
	HMODULE Software, 
	UINT Flags, 
	D3D10_FEATURE_LEVEL1 HardwareLevel,
	UINT SDKVersion, 
	ID3D10Device **ppDevice)
{
	HRESULT res;
	OutTraceD("D3D10CreateDevice1: DriverType=%x Flags=%x HardwareLevel=%x SDKVersion=%x\n", DriverType, Flags, HardwareLevel, SDKVersion);
	res=(*pD3D10CreateDevice1)(pAdapter, DriverType, Software, Flags, HardwareLevel, SDKVersion, ppDevice);
	SetHook((void *)(*(DWORD *)*ppDevice + 100), extRSSetViewports, (void **)&pRSSetViewports, "RSSetViewports(D10)");
	OutTraceD("D3D10CreateDevice1: ret=%x\n", res);
	return res;
}

HRESULT WINAPI extD3D10CreateDeviceAndSwapChain(
	IDXGIAdapter *pAdapter, 
	D3D10_DRIVER_TYPE DriverType, 
	HMODULE Software, 
	UINT Flags, 
	UINT SDKVersion, 
	DXGI_SWAP_CHAIN_DESC *pSwapChainDesc, 
	IDXGISwapChain **ppSwapChain, 
	ID3D10Device **ppDevice)
{
	HRESULT res;
	OutTraceD("D3D10CreateDeviceAndSwapChain: DriverType=%x Flags=%x SDKVersion=%x\n", DriverType, Flags, SDKVersion);
	//return 0x887a0004;
	pSwapChainDesc->OutputWindow = dxw.GethWnd();
	pSwapChainDesc->Windowed = true;	
	res=(*pD3D10CreateDeviceAndSwapChain)(pAdapter, DriverType, Software, Flags, SDKVersion, pSwapChainDesc, ppSwapChain, ppDevice);
	OutTraceD("D3D10CreateDeviceAndSwapChain ret=%x\n", res);
	return res;
}

HRESULT WINAPI extD3D10CreateDeviceAndSwapChain1(
	IDXGIAdapter *pAdapter, 
	D3D10_DRIVER_TYPE DriverType, 
	HMODULE Software, 
	UINT Flags, 
	UINT SDKVersion, 
	DXGI_SWAP_CHAIN_DESC *pSwapChainDesc, 
	IDXGISwapChain **ppSwapChain, 
	ID3D10Device **ppDevice)
{
	HRESULT res;
	OutTraceD("D3D10CreateDeviceAndSwapChain1: DriverType=%x Flags=%x SDKVersion=%x\n", DriverType, Flags, SDKVersion);
	//return 0x887a0004;
	pSwapChainDesc->OutputWindow = dxw.GethWnd();
	pSwapChainDesc->Windowed = true;	
	res=(*pD3D10CreateDeviceAndSwapChain1)(pAdapter, DriverType, Software, Flags, SDKVersion, pSwapChainDesc, ppSwapChain, ppDevice);
	OutTraceD("D3D10CreateDeviceAndSwapChain1 ret=%x\n", res);
	return res;
}

HRESULT WINAPI extD3D11CreateDevice(
	IDXGIAdapter *pAdapter, 
	D3D_DRIVER_TYPE DriverType,
	HMODULE Software,
	UINT Flags,
	const D3D_FEATURE_LEVEL *pFeatureLevels,
	UINT FeatureLevels,
	UINT SDKVersion,
	ID3D11Device **ppDevice,
	D3D_FEATURE_LEVEL *pFeatureLevel,
	ID3D11DeviceContext **ppImmediateContext)
{
	HRESULT res;
	OutTraceD("D3D11CreateDevice: DriverType=%x Flags=%x FeatureLevels=%x SDKVersion=%x\n", DriverType, Flags, FeatureLevels, SDKVersion);
	//return 0x887a0004;
	res=(*pD3D11CreateDevice)(pAdapter, DriverType, Software, Flags, pFeatureLevels, FeatureLevels, SDKVersion, ppDevice, pFeatureLevel, ppImmediateContext);
	if(res){
		OutTraceD("D3D11CreateDevice: ret=%x\n", res);
		return res;
	}
	SetHook((void *)(*(DWORD *)ppImmediateContext + 148), extRSSetViewports, (void **)&pRSSetViewports, "RSSetViewports(D11)");
	//SetHook((void *)(*(DWORD *)ppImmediateContext + 152), extRSSetScissorRects, (void **)&pRSSetScissorRects, "RSSetScissorRects(D11)");
	OutTraceD("D3D11CreateDevice ret=%x\n", res);
	return res;
}

HRESULT WINAPI extD3D11CreateDeviceAndSwapChain(
	IDXGIAdapter *pAdapter,
	D3D_DRIVER_TYPE DriverType,
	HMODULE Software,
	UINT Flags,
	const D3D_FEATURE_LEVEL *pFeatureLevels,
	UINT FeatureLevels,
	UINT SDKVersion,
	const DXGI_SWAP_CHAIN_DESC *pSwapChainDesc,
	IDXGISwapChain **ppSwapChain,
	ID3D11Device **ppDevice,
	D3D_FEATURE_LEVEL *pFeatureLevel,
	ID3D11DeviceContext **ppImmediateContext)
{
	HRESULT res;
	OutTraceD("D3D11CreateDeviceAndSwapChain\n");
	return 0x887a0004;
	res=(*pD3D11CreateDeviceAndSwapChain)(pAdapter, DriverType, Software, Flags, pFeatureLevels, FeatureLevels, SDKVersion, pSwapChainDesc, ppSwapChain, ppDevice, pFeatureLevel, ppImmediateContext);
	OutTraceD("D3D11CreateDeviceAndSwapChain ret=%x\n", res);
	return res;
}

void WINAPI extRSSetViewports(ID3D11DeviceContext *This, UINT NumViewports, D3D11_VIEWPORT *pViewports)
{
	OutTraceD("RSSetViewports: NumViewports=%d\n", NumViewports);

	if(NumViewports==1){
		pViewports->TopLeftX=dxw.iPosX;
		pViewports->TopLeftY=dxw.iPosY;
		pViewports->Width=dxw.iSizX;
		pViewports->Height=dxw.iSizY;
	}

	(*pRSSetViewports)(This, NumViewports, pViewports);
}

HRESULT WINAPI extQueryInterfaceD3D8(void *obj, REFIID riid, void** ppvObj)
{
	HRESULT res;
	OutTraceD("D3D::QueryInterface(8)\n");
	res=pQueryInterfaceD3D8(obj, riid, ppvObj);
	return res;
}

HRESULT WINAPI extQueryInterfaceDev8(void *obj, REFIID riid, void** ppvObj)
{
	HRESULT res;
	OutTraceD("Device::QueryInterface(8)\n");
	res=pQueryInterfaceDev8(obj, riid, ppvObj);
	return res;
}

HRESULT WINAPI extQueryInterfaceD3D9(void *obj, REFIID riid, void** ppvObj)
{
	HRESULT res;
	OutTraceD("D3D::QueryInterface(9)\n");
	res=pQueryInterfaceD3D9(obj, riid, ppvObj);
	return res;
}

HRESULT WINAPI extQueryInterfaceDev9(void *obj, REFIID riid, void** ppvObj)
{
	HRESULT res;
	void *pReset;

	OutTraceD("Device::QueryInterface(9): lpd3dd=%x refiid=%x\n", obj, riid);
	res=pQueryInterfaceDev9(obj, riid, ppvObj);
	if(res){
		OutTraceD("Device::QueryInterface(9): ERROR ret=%x\n", res);
		return res;
	}
	OutTraceD("Device::QueryInterface(9): ppvObj=%x\n", *ppvObj);

	switch(*(DWORD *)&riid){
	case 0xD0223B96: // IID_IDirect3DDevice9
		OutTraceD("Device hook for IID_IDirect3DDevice9 interface\n");
		pReset=NULL; // to avoid assert condition
		SetHook((void *)(**(DWORD **)ppvObj +  0), extQueryInterfaceDev9, (void **)&pQueryInterfaceDev9, "QueryInterface(D9)");
		SetHook((void *)(**(DWORD **)ppvObj + 32), extGetDisplayMode, (void **)&pGetDisplayMode, "GetDisplayMode(D9)");
		SetHook((void *)(**(DWORD **)ppvObj + 52), extCreateAdditionalSwapChain, (void **)&pCreateAdditionalSwapChain, "CreateAdditionalSwapChain(D9)");
		SetHook((void *)(**(DWORD **)ppvObj + 64), extReset, (void **)&pReset, "Reset(D9)");
		SetHook((void *)(**(DWORD **)ppvObj + 68), extPresent, (void **)&pPresent, "Present(D9)");
		//SetHook((void *)(**(DWORD **)ppvObj +188), extSetViewport, (void **)&pSetViewport, "SetViewport(D9)");
		//SetHook((void *)(**(DWORD **)ppvObj +192), extGetViewport, (void **)&pGetViewport, "GetViewport(D9)");
		if(dxw.dwFlags2 & WIREFRAME){
			SetHook((void *)(**(DWORD **)ppvObj + 228), extSetRenderState, (void **)&pSetRenderState, "SetRenderState(D9)");
			SetHook((void *)(**(DWORD **)ppvObj + 232), extGetRenderState, (void **)&pGetRenderState, "GetRenderState(D9)");
			(*pSetRenderState)((void *)*ppvObj, D3DRS_FILLMODE, D3DFILL_WIREFRAME);
		}
		break;
	}

	return res;
}

HRESULT WINAPI extGetDirect3D(void *lpdd3dd, IDirect3D9 **ppD3D9)
{
	OutTraceD("Device::GetDirect3D\n");
	return (*pGetDirect3D)(lpdd3dd, ppD3D9);
}

HRESULT WINAPI extCheckFullScreen(void)
{
	OutTraceD("CheckFullScreen\n");
	return 0;
}

void WINAPI extSetGammaRamp(UINT iSwapChain, DWORD Flags, D3DGAMMARAMP *pRamp)
{
	OutTraceD("SetGammaRamp: SwapChain=%d flags=%x\n", iSwapChain, Flags);
	if (dxw.dwFlags2 & DISABLEGAMMARAMP) return;
	(*pSetGammaRamp)(iSwapChain, Flags, pRamp);
}
