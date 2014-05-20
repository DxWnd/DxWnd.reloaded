//#define D3D10_IGNORE_SDK_LAYERS 1
#include <windows.h>
#include <d3d9.h>
#include <D3D10_1.h>
#include <D3D10Misc.h>
#include <D3D11.h>
#include "dxwnd.h"
#include "dxwcore.hpp"
#include "dxhook.h"
#include "dxhelper.h"
#include "syslibs.h"

#define HOOKD3D10ANDLATER 1

typedef HRESULT (WINAPI *QueryInterface_Type)(void *, REFIID riid, void** ppvObj);

typedef void* (WINAPI *Direct3DCreate8_Type)(UINT);
typedef void* (WINAPI *Direct3DCreate9_Type)(UINT);
typedef HRESULT (WINAPI *Direct3DCreate9Ex_Type)(UINT, IDirect3D9Ex **);
typedef HRESULT (WINAPI *CheckFullScreen_Type)(void);
typedef BOOL (WINAPI * DisableD3DSpy_Type)(void);

typedef UINT (WINAPI *GetAdapterCount_Type)(void *);
typedef HRESULT (WINAPI *GetAdapterIdentifier_Type)(void *, UINT, DWORD, D3DADAPTER_IDENTIFIER9 *);
typedef HRESULT (WINAPI *CreateDevice_Type)(void *, UINT, D3DDEVTYPE, HWND, DWORD, void *, void **);
typedef HRESULT (WINAPI *CreateDeviceEx_Type)(void *, UINT, D3DDEVTYPE, HWND, DWORD, void *, D3DDISPLAYMODEEX *, void **);
typedef HRESULT (WINAPI *EnumAdapterModes8_Type)(void *, UINT, UINT, D3DDISPLAYMODE *);
typedef HRESULT (WINAPI *EnumAdapterModes9_Type)(void *, UINT, D3DFORMAT ,UINT, D3DDISPLAYMODE *);
typedef HRESULT (WINAPI *GetAdapterDisplayMode_Type)(void *, UINT, D3DDISPLAYMODE *);
typedef HRESULT (WINAPI *GetDisplayMode8_Type)(void *, D3DDISPLAYMODE *);
typedef HRESULT (WINAPI *GetDisplayMode9_Type)(void *, UINT, D3DDISPLAYMODE *);
typedef HRESULT (WINAPI *Present_Type)(void *, CONST RECT *, CONST RECT *, HWND, CONST RGNDATA *);
typedef HRESULT (WINAPI *SetRenderState_Type)(void *, D3DRENDERSTATETYPE, DWORD);
typedef HRESULT (WINAPI *GetRenderState_Type)(void *, D3DRENDERSTATETYPE, DWORD );
typedef HRESULT (WINAPI *CreateAdditionalSwapChain_Type)(void *, D3DPRESENT_PARAMETERS *, IDirect3DSwapChain9 **);
typedef HRESULT (WINAPI *GetDirect3D8_Type)(void *, void **);
typedef HRESULT (WINAPI *GetDirect3D9_Type)(void *, void **);
typedef HRESULT (WINAPI *GetViewport_Type)(void *, D3DVIEWPORT9 *);
typedef HRESULT (WINAPI *SetViewport_Type)(void *, D3DVIEWPORT9 *);
typedef void	(WINAPI *SetGammaRamp_Type)(void *, UINT, DWORD, D3DGAMMARAMP *);
typedef void	(WINAPI *GetGammaRamp_Type)(void *, UINT, D3DGAMMARAMP *);
typedef void	(WINAPI *SetCursorPosition9_Type)(void *, int, int, DWORD);
typedef void	(WINAPI *SetCursorPosition8_Type)(void *, int, int, DWORD);
typedef BOOL	(WINAPI *ShowCursor8_Type)(void *, BOOL);
typedef BOOL	(WINAPI *ShowCursor9_Type)(void *, BOOL);
typedef HRESULT (WINAPI *SetTexture8_Type)(void *, DWORD, void *);
typedef HRESULT (WINAPI *SetTexture9_Type)(void *, DWORD, void *);

//typedef ULONG	(WINAPI *CreateRenderTarget8_Type)(void *, UINT, UINT, D3DFORMAT, D3DMULTISAMPLE_TYPE, BOOL, IDirect3DSurface8**);
typedef ULONG	(WINAPI *CreateRenderTarget8_Type)(void *, UINT, UINT, D3DFORMAT, D3DMULTISAMPLE_TYPE, BOOL, void**);
typedef ULONG	(WINAPI *BeginScene_Type)(void *);
typedef ULONG	(WINAPI *EndScene_Type)(void *);

typedef HRESULT (WINAPI *D3D10CreateDevice_Type)(IDXGIAdapter *, D3D10_DRIVER_TYPE, HMODULE, UINT, UINT, ID3D10Device **);
typedef HRESULT (WINAPI *D3D10CreateDeviceAndSwapChain_Type)(IDXGIAdapter *, D3D10_DRIVER_TYPE, HMODULE, UINT, UINT, DXGI_SWAP_CHAIN_DESC *, IDXGISwapChain **, ID3D10Device **);
typedef HRESULT (WINAPI *D3D10CreateDevice1_Type)(IDXGIAdapter *, D3D10_DRIVER_TYPE, HMODULE, UINT, D3D10_FEATURE_LEVEL1, UINT, ID3D10Device **);
typedef HRESULT (WINAPI *D3D10CreateDeviceAndSwapChain1_Type)(IDXGIAdapter *, D3D10_DRIVER_TYPE, HMODULE, UINT, UINT, DXGI_SWAP_CHAIN_DESC *, IDXGISwapChain **, ID3D10Device **);
typedef HRESULT (WINAPI *D3D11CreateDevice_Type)(IDXGIAdapter *, D3D_DRIVER_TYPE, HMODULE, UINT, const D3D_FEATURE_LEVEL *, UINT, UINT, ID3D11Device **, D3D_FEATURE_LEVEL *, ID3D11DeviceContext **);
typedef HRESULT (WINAPI *D3D11CreateDeviceAndSwapChain_Type)(IDXGIAdapter *, D3D_DRIVER_TYPE, HMODULE, UINT, const D3D_FEATURE_LEVEL *, UINT, UINT, const DXGI_SWAP_CHAIN_DESC *, IDXGISwapChain **, ID3D11Device **, D3D_FEATURE_LEVEL *, ID3D11DeviceContext **);
typedef void	(WINAPI *RSSetViewports_Type)(void *, UINT, D3D11_VIEWPORT *);
typedef ULONG	(WINAPI *AddRef_Type)(void *);
typedef ULONG	(WINAPI *Release_Type)(void *);
typedef HRESULT (WINAPI *Reset_Type)(void *, D3DPRESENT_PARAMETERS*);

HRESULT WINAPI extQueryInterfaceD3D8(void *, REFIID, void** );
HRESULT WINAPI extQueryInterfaceDev8(void *, REFIID, void** );
HRESULT WINAPI extQueryInterfaceD3D9(void *, REFIID, void** );
HRESULT WINAPI extQueryInterfaceDev9(void *, REFIID, void** );
HRESULT WINAPI extGetDirect3D8(void *, void **);
HRESULT WINAPI extGetDirect3D9(void *, void **);

void* WINAPI extDirect3DCreate8(UINT);
void* WINAPI extDirect3DCreate9(UINT);
HRESULT WINAPI extDirect3DCreate9Ex(UINT, IDirect3D9Ex **);
HRESULT WINAPI extCheckFullScreen(void);

UINT WINAPI extGetAdapterCount8(void *);
UINT WINAPI extGetAdapterCount9(void *);
HRESULT WINAPI extGetAdapterIdentifier8(void *, UINT, DWORD, D3DADAPTER_IDENTIFIER9 *);
HRESULT WINAPI extGetAdapterIdentifier9(void *, UINT, DWORD, D3DADAPTER_IDENTIFIER9 *);
HRESULT WINAPI extCreateDevice(void *, UINT, D3DDEVTYPE, HWND, DWORD, D3DPRESENT_PARAMETERS *, void **);
HRESULT WINAPI extCreateDeviceEx(void *, UINT, D3DDEVTYPE, HWND, DWORD, D3DPRESENT_PARAMETERS *, D3DDISPLAYMODEEX *, void **);
HRESULT WINAPI extEnumAdapterModes8(void *, UINT, UINT , D3DDISPLAYMODE *);
HRESULT WINAPI extEnumAdapterModes9(void *, UINT, D3DFORMAT, UINT , D3DDISPLAYMODE *);
HRESULT WINAPI extGetAdapterDisplayMode8(void *, UINT, D3DDISPLAYMODE *);
HRESULT WINAPI extGetAdapterDisplayMode9(void *, UINT, D3DDISPLAYMODE *);
HRESULT WINAPI extGetDisplayMode8(void *, D3DDISPLAYMODE *);
HRESULT WINAPI extGetDisplayMode9(void *, UINT, D3DDISPLAYMODE *);
HRESULT WINAPI extPresent(void *, CONST RECT *, CONST RECT *, HWND, CONST RGNDATA *);
HRESULT WINAPI extSetRenderState(void *, D3DRENDERSTATETYPE, DWORD);
HRESULT WINAPI extGetRenderState(void *, D3DRENDERSTATETYPE, DWORD);
HRESULT WINAPI extCreateAdditionalSwapChain(void *, D3DPRESENT_PARAMETERS *, IDirect3DSwapChain9 **);
HRESULT WINAPI extGetViewport(void *, D3DVIEWPORT9 *);
HRESULT WINAPI extSetViewport(void *, D3DVIEWPORT9 *);
HRESULT WINAPI extReset(void *, D3DPRESENT_PARAMETERS *);
void	WINAPI extSetGammaRamp(void *, UINT, DWORD, D3DGAMMARAMP *);
void	WINAPI extGetGammaRamp(void *, UINT, D3DGAMMARAMP *);
void	WINAPI extSetCursorPosition9(void *, int, int, DWORD);
void	WINAPI extSetCursorPosition8(void *, int, int, DWORD);
BOOL	WINAPI extShowCursor8(void *, BOOL);
BOOL	WINAPI extShowCursor9(void *, BOOL);
ULONG WINAPI extAddRef9(void *);
ULONG WINAPI extRelease9(void *);
BOOL  WINAPI voidDisableD3DSpy(void);

//ULONG WINAPI extCreateRenderTarget8(void *, UINT, UINT, D3DFORMAT, D3DMULTISAMPLE_TYPE, BOOL, IDirect3DSurface8**);
ULONG WINAPI extCreateRenderTarget8(void *, UINT, UINT, D3DFORMAT, D3DMULTISAMPLE_TYPE, BOOL, void**);
ULONG WINAPI extBeginScene8(void *);
ULONG WINAPI extEndScene8(void *);
ULONG WINAPI extBeginScene9(void *);
ULONG WINAPI extEndScene9(void *);
ULONG WINAPI extSetTexture8(void *, DWORD, void *);
ULONG WINAPI extSetTexture9(void *, DWORD, void *);


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

GetDirect3D8_Type pGetDirect3D8 = 0;
GetDirect3D9_Type pGetDirect3D9 = 0;
Direct3DCreate8_Type pDirect3DCreate8 = 0;
Direct3DCreate9_Type pDirect3DCreate9 = 0;
Direct3DCreate9Ex_Type pDirect3DCreate9Ex = 0;
CheckFullScreen_Type pCheckFullScreen = 0;

GetAdapterCount_Type pGetAdapterCount8 = 0;
GetAdapterCount_Type pGetAdapterCount9 = 0;
GetAdapterIdentifier_Type pGetAdapterIdentifier8 = 0;
GetAdapterIdentifier_Type pGetAdapterIdentifier9 = 0;
CreateDevice_Type pCreateDevice8 = 0;
CreateDevice_Type pCreateDevice9 = 0;
CreateDeviceEx_Type pCreateDeviceEx = 0;
EnumAdapterModes8_Type pEnumAdapterModes8 = 0;
EnumAdapterModes9_Type pEnumAdapterModes9 = 0;
GetAdapterDisplayMode_Type pGetAdapterDisplayMode8 = 0;
GetAdapterDisplayMode_Type pGetAdapterDisplayMode9 = 0;
GetDisplayMode8_Type pGetDisplayMode8 = 0;
GetDisplayMode9_Type pGetDisplayMode9 = 0;
Present_Type pPresent = 0;
SetRenderState_Type pSetRenderState = 0;
GetRenderState_Type pGetRenderState = 0;
CreateAdditionalSwapChain_Type pCreateAdditionalSwapChain = 0;
GetViewport_Type pGetViewport = 0;
SetViewport_Type pSetViewport = 0;
SetGammaRamp_Type pSetGammaRamp = 0;
GetGammaRamp_Type pGetGammaRamp = 0;
SetCursorPosition9_Type pSetCursorPosition9 = 0;
SetCursorPosition8_Type pSetCursorPosition8 = 0;
ShowCursor8_Type pShowCursor8 = 0;
ShowCursor9_Type pShowCursor9 = 0;

CreateRenderTarget8_Type pCreateRenderTarget8 = 0;
BeginScene_Type pBeginScene8 = 0;
EndScene_Type pEndScene8 = 0;
BeginScene_Type pBeginScene9 = 0;
EndScene_Type pEndScene9 = 0;
Reset_Type pReset = 0;
SetTexture8_Type pSetTexture8 = 0;
SetTexture9_Type pSetTexture9 = 0;

D3D10CreateDevice_Type pD3D10CreateDevice = 0;
D3D10CreateDeviceAndSwapChain_Type pD3D10CreateDeviceAndSwapChain = 0;
D3D10CreateDevice1_Type pD3D10CreateDevice1 = 0;
D3D10CreateDeviceAndSwapChain1_Type pD3D10CreateDeviceAndSwapChain1 = 0;
D3D11CreateDevice_Type pD3D11CreateDevice = 0;
D3D11CreateDeviceAndSwapChain_Type pD3D11CreateDeviceAndSwapChain = 0;
RSSetViewports_Type pRSSetViewports = 0;

AddRef_Type pAddRef9 = 0;
Release_Type pRelease9 = 0;

DWORD dwD3DVersion;

FARPROC Remap_d3d8_ProcAddress(LPCSTR proc, HMODULE hModule)
{
	if (!strcmp(proc,"Direct3DCreate8") && !pDirect3DCreate8){
		pDirect3DCreate8=(Direct3DCreate8_Type)(*pGetProcAddress)(hModule, proc);
		OutTraceDW("GetProcAddress: hooking proc=%s at addr=%x\n", ProcToString(proc), pDirect3DCreate8);
		return (FARPROC)extDirect3DCreate8;
	}
	// NULL -> keep the original call address
	return NULL;
}

HRESULT WINAPI voidDirect3DShaderValidatorCreate9(void) 
{
	OutTraceDW("Direct3DShaderValidatorCreate9: SUPPRESSED\n");
	return 0;
}

void WINAPI voidDebugSetLevel(void) 
{
	OutTraceDW("DebugSetLevel: SUPPRESSED\n");
}

void WINAPI voidDebugSetMute(void) 
{
	OutTraceDW("DebugSetMute: SUPPRESSED\n");
}

BOOL  WINAPI voidDisableD3DSpy(void)
{
	OutTraceDW("DisableD3DSpy: SUPPRESSED\n");
	return FALSE;
}

FARPROC Remap_d3d9_ProcAddress(LPCSTR proc, HMODULE hModule)
{
	if (!strcmp(proc,"Direct3DCreate9") && !pDirect3DCreate9){
		pDirect3DCreate9=(Direct3DCreate9_Type)(*pGetProcAddress)(hModule, proc);
		OutTraceDW("GetProcAddress: hooking proc=%s at addr=%x\n", ProcToString(proc), pDirect3DCreate9);
		return (FARPROC)extDirect3DCreate9;
	}
	if (!strcmp(proc,"Direct3DCreate9Ex") && !pDirect3DCreate9Ex){
		pDirect3DCreate9Ex=(Direct3DCreate9Ex_Type)(*pGetProcAddress)(hModule, proc);
		OutTraceDW("GetProcAddress: hooking proc=%s at addr=%x\n", ProcToString(proc), pDirect3DCreate9Ex);
		return (FARPROC)extDirect3DCreate9Ex;
	}
	if (!strcmp(proc,"CheckFullScreen") && !pCheckFullScreen){
		pCheckFullScreen=(CheckFullScreen_Type)(*pGetProcAddress)(hModule, proc);
		OutTraceDW("GetProcAddress: hooking proc=%s at addr=%x\n", ProcToString(proc), pCheckFullScreen);
		return (FARPROC)extCheckFullScreen;
	}

	if (!(dxw.dwFlags3 & SUPPRESSD3DEXT)) return NULL;

	if (!strcmp(proc,"Direct3DShaderValidatorCreate9")){
		OutTraceDW("GetProcAddress: suppressing Direct3DShaderValidatorCreate9\n");
		return (FARPROC)voidDirect3DShaderValidatorCreate9;
	}
	if (!strcmp(proc,"DebugSetLevel")){
		OutTraceDW("GetProcAddress: suppressing DebugSetLevel\n");
		return (FARPROC)voidDebugSetLevel;
	}
	if (!strcmp(proc,"DebugSetMute")){
		OutTraceDW("GetProcAddress: suppressing DebugSetMute\n");
		return (FARPROC)voidDebugSetMute;
	}
	if (!strcmp(proc,"DisableD3DSpy")){
		OutTraceDW("GetProcAddress: suppressing DisableD3DSpy\n");
		return (FARPROC)voidDisableD3DSpy;	
	}
	return NULL;
}

FARPROC Remap_d3d10_ProcAddress(LPCSTR proc, HMODULE hModule)
{
	if (!strcmp(proc,"D3D10CreateDevice") && !pD3D10CreateDevice){
		pD3D10CreateDevice=(D3D10CreateDevice_Type)(*pGetProcAddress)(hModule, proc);
		OutTraceDW("GetProcAddress: hooking proc=%s at addr=%x\n", ProcToString(proc), pD3D10CreateDevice);
		return (FARPROC)extD3D10CreateDevice;
	}
	if (!strcmp(proc,"D3D10CreateDeviceAndSwapChain") && !pD3D10CreateDeviceAndSwapChain){
		pD3D10CreateDeviceAndSwapChain=(D3D10CreateDeviceAndSwapChain_Type)(*pGetProcAddress)(hModule, proc);
		OutTraceDW("GetProcAddress: hooking proc=%s at addr=%x\n", ProcToString(proc), pD3D10CreateDeviceAndSwapChain);
		return (FARPROC)extD3D10CreateDeviceAndSwapChain;
	}
	// NULL -> keep the original call address
	return NULL;
}

FARPROC Remap_d3d10_1_ProcAddress(LPCSTR proc, HMODULE hModule)
{
	if (!strcmp(proc,"D3D10CreateDevice1") && !pD3D10CreateDevice1){
		pD3D10CreateDevice1=(D3D10CreateDevice1_Type)(*pGetProcAddress)(hModule, proc);
		OutTraceDW("GetProcAddress: hooking proc=%s at addr=%x\n", ProcToString(proc), pD3D10CreateDevice1);
		return (FARPROC)extD3D10CreateDevice1;
	}
	if (!strcmp(proc,"D3D10CreateDeviceAndSwapChain1") && !pD3D10CreateDeviceAndSwapChain1){
		pD3D10CreateDeviceAndSwapChain1=(D3D10CreateDeviceAndSwapChain1_Type)(*pGetProcAddress)(hModule, proc);
		OutTraceDW("GetProcAddress: hooking proc=%s at addr=%x\n", ProcToString(proc), pD3D10CreateDeviceAndSwapChain1);
		return (FARPROC)extD3D10CreateDeviceAndSwapChain1;
	}
	// NULL -> keep the original call address
	return NULL;
}

FARPROC Remap_d3d11_ProcAddress(LPCSTR proc, HMODULE hModule)
{
	if (!strcmp(proc,"D3D11CreateDevice") && !pD3D11CreateDevice){
		pD3D11CreateDevice=(D3D11CreateDevice_Type)(*pGetProcAddress)(hModule, proc);
		OutTraceDW("GetProcAddress: hooking proc=%s at addr=%x\n", ProcToString(proc), pD3D11CreateDevice);
		return (FARPROC)extD3D11CreateDevice;
	}
	if (!strcmp(proc,"D3D11CreateDeviceAndSwapChain") && !pD3D11CreateDeviceAndSwapChain){
		pD3D11CreateDeviceAndSwapChain=(D3D11CreateDeviceAndSwapChain_Type)(*pGetProcAddress)(hModule, proc);
		OutTraceDW("GetProcAddress: hooking proc=%s at addr=%x\n", ProcToString(proc), pD3D11CreateDeviceAndSwapChain);
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
		hinst = (*pLoadLibraryA)("d3d9.dll");
		pDirect3DCreate9 =
			(Direct3DCreate9_Type)(*pGetProcAddress)(hinst, "Direct3DCreate9");
		pDirect3DCreate9Ex =
			(Direct3DCreate9Ex_Type)(*pGetProcAddress)(hinst, "Direct3DCreate9Ex");
		pCheckFullScreen = 
			(CheckFullScreen_Type)(*pGetProcAddress)(hinst, "CheckFullScreen");
		if(pDirect3DCreate9){
			lpd3d = (LPDIRECT3D9)extDirect3DCreate9(31);
			if(lpd3d) lpd3d->Release();
		}
		break;
#ifdef HOOKD3D10ANDLATER
	case 10:
		hinst = (*pLoadLibraryA)("d3d10.dll");
		pD3D10CreateDevice =
			(D3D10CreateDevice_Type)(*pGetProcAddress)(hinst, "D3D10CreateDevice");
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
		hinst = (*pLoadLibraryA)("d3d10_1.dll");
		pD3D10CreateDevice1 =
			(D3D10CreateDevice1_Type)(*pGetProcAddress)(hinst, "D3D10CreateDevice1");
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
			(D3D10CreateDeviceAndSwapChain_Type)(*pGetProcAddress)(hinst, "D3D10CreateDeviceAndSwapChain");
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
		hinst = (*pLoadLibraryA)("d3d11.dll");
		pD3D11CreateDevice =
			(D3D11CreateDevice_Type)(*pGetProcAddress)(hinst, "D3D11CreateDevice");
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
			(D3D11CreateDeviceAndSwapChain_Type)(*pGetProcAddress)(hinst, "D3D11CreateDeviceAndSwapChain");
		break;
#endif
	}
	if(pDirect3DCreate8 || pDirect3DCreate9) return 1;
	return 0;
}

void HookD3DDevice8(void** ppD3Ddev8)
{
	OutTraceDW("Device hook for IID_IDirect3DDevice8 interface\n");
	SetHook((void *)(**(DWORD **)ppD3Ddev8 +  0), extQueryInterfaceDev8, (void **)&pQueryInterfaceDev8, "QueryInterface(D8)");
	SetHook((void *)(**(DWORD **)ppD3Ddev8 + 24), extGetDirect3D8, (void **)&pGetDirect3D8, "GetDirect3D(D8)");
	SetHook((void *)(**(DWORD **)ppD3Ddev8 + 32), extGetDisplayMode8, (void **)&pGetDisplayMode8, "GetDisplayMode(D8)");
	SetHook((void *)(**(DWORD **)ppD3Ddev8 + 44), extSetCursorPosition8, (void **)&pSetCursorPosition8, "SetCursorPosition(D8)");
	SetHook((void *)(**(DWORD **)ppD3Ddev8 + 48), extShowCursor8, (void **)&pShowCursor8, "ShowCursor(D8)");
	SetHook((void *)(**(DWORD **)ppD3Ddev8 + 52), extCreateAdditionalSwapChain, (void **)&pCreateAdditionalSwapChain, "CreateAdditionalSwapChain(D8)");
	SetHook((void *)(**(DWORD **)ppD3Ddev8 + 56), extReset, (void **)&pReset, "Reset(D8)");
	SetHook((void *)(**(DWORD **)ppD3Ddev8 + 60), extPresent, (void **)&pPresent, "Present(D8)");
	if(dxw.dwFlags2 & DISABLEGAMMARAMP){
		SetHook((void *)(**(DWORD **)ppD3Ddev8 + 72), extSetGammaRamp, (void **)&pSetGammaRamp, "SetGammaRamp(D8)");
		SetHook((void *)(**(DWORD **)ppD3Ddev8 + 76), extGetGammaRamp, (void **)&pGetGammaRamp, "GetGammaRamp(D8)");
	}
	SetHook((void *)(**(DWORD **)ppD3Ddev8 + 100), extCreateRenderTarget8, (void **)&pCreateRenderTarget8, "CreateRenderTarget(D8)");
	SetHook((void *)(**(DWORD **)ppD3Ddev8 + 136), extBeginScene8, (void **)&pBeginScene8, "BeginScene(D8)");
	SetHook((void *)(**(DWORD **)ppD3Ddev8 + 140), extEndScene8, (void **)&pEndScene8, "EndScene(D8)");
	if((dxw.dwFlags2 & WIREFRAME) || (dxw.dwFlags4 & DISABLEFOGGING) || (dxw.dwFlags4 & ZBUFFERALWAYS)){
		SetHook((void *)(**(DWORD **)ppD3Ddev8 + 200), extSetRenderState, (void **)&pSetRenderState, "SetRenderState(D8)");
		SetHook((void *)(**(DWORD **)ppD3Ddev8 + 204), extGetRenderState, (void **)&pGetRenderState, "GetRenderState(D8)");
		if(dxw.dwFlags2 & WIREFRAME) (*pSetRenderState)((void *)*ppD3Ddev8, D3DRS_FILLMODE, D3DFILL_WIREFRAME);
		if(dxw.dwFlags4 & DISABLEFOGGING) (*pSetRenderState)((void *)*ppD3Ddev8, D3DRS_FOGENABLE, FALSE);
		if(dxw.dwFlags4 & ZBUFFERALWAYS) (*pSetRenderState)((void *)*ppD3Ddev8, D3DRS_ZFUNC, D3DCMP_ALWAYS);
		//if(1) (*pSetRenderState)((void *)*ppD3Ddev8, D3DRS_SPECULARENABLE, TRUE);
	}
	if (dxw.dwFlags4 & NOTEXTURES) SetHook((void *)(**(DWORD **)ppD3Ddev8 + 244), extSetTexture8, (void **)&pSetTexture8, "SetTexture(D8)");
	//if (!(dxw.dwTFlags & OUTPROXYTRACE)) return;
	//SetHook((void *)(**(DWORD **)ppD3Ddev8 +  4), extAddRef8, (void **)&pAddRef8, "AddRef(D8)");
	//SetHook((void *)(**(DWORD **)ppD3Ddev8 +  8), extRelease8, (void **)&pRelease8, "Release(D8)");
}

void HookD3DDevice9(void** ppD3Ddev9)
{
	OutTraceDW("Device hook for IID_IDirect3DDevice9 interface\n");
	SetHook((void *)(**(DWORD **)ppD3Ddev9 +  0), extQueryInterfaceDev9, (void **)&pQueryInterfaceDev9, "QueryInterface(D9)");
	SetHook((void *)(**(DWORD **)ppD3Ddev9 + 24), extGetDirect3D9, (void **)&pGetDirect3D9, "GetDirect3D(D9)");
	SetHook((void *)(**(DWORD **)ppD3Ddev9 + 32), extGetDisplayMode9, (void **)&pGetDisplayMode9, "GetDisplayMode(D9)");
	SetHook((void *)(**(DWORD **)ppD3Ddev9 + 44), extSetCursorPosition9, (void **)&pSetCursorPosition9, "SetCursorPosition(D9)");
	SetHook((void *)(**(DWORD **)ppD3Ddev9 + 48), extShowCursor9, (void **)&pShowCursor9, "ShowCursor(D9)");
	SetHook((void *)(**(DWORD **)ppD3Ddev9 + 52), extCreateAdditionalSwapChain, (void **)&pCreateAdditionalSwapChain, "CreateAdditionalSwapChain(D9)");
	SetHook((void *)(**(DWORD **)ppD3Ddev9 + 64), extReset, (void **)&pReset, "Reset(D9)");
	SetHook((void *)(**(DWORD **)ppD3Ddev9 + 68), extPresent, (void **)&pPresent, "Present(D9)");
	if(dxw.dwFlags2 & DISABLEGAMMARAMP){
		SetHook((void *)(**(DWORD **)ppD3Ddev9 + 84), extSetGammaRamp, (void **)&pSetGammaRamp, "SetGammaRamp(D9)");
		SetHook((void *)(**(DWORD **)ppD3Ddev9 + 88), extGetGammaRamp, (void **)&pGetGammaRamp, "GetGammaRamp(D9)");
	}
	SetHook((void *)(**(DWORD **)ppD3Ddev9 + 164), extBeginScene9, (void **)&pBeginScene9, "BeginScene(D9)");
	SetHook((void *)(**(DWORD **)ppD3Ddev9 + 168), extEndScene9, (void **)&pEndScene9, "EndScene(D9)");
	//SetHook((void *)(**(DWORD **)ppD3Ddev9 +188), extSetViewport, (void **)&pSetViewport, "SetViewport(D9)");
	//SetHook((void *)(**(DWORD **)ppD3Ddev9 +192), extGetViewport, (void **)&pGetViewport, "GetViewport(D9)");
	if((dxw.dwFlags2 & WIREFRAME) || (dxw.dwFlags4 & DISABLEFOGGING) || (dxw.dwFlags4 & ZBUFFERALWAYS)){
		SetHook((void *)(**(DWORD **)ppD3Ddev9 + 228), extSetRenderState, (void **)&pSetRenderState, "SetRenderState(D9)");
		SetHook((void *)(**(DWORD **)ppD3Ddev9 + 232), extGetRenderState, (void **)&pGetRenderState, "GetRenderState(D9)");
		if(dxw.dwFlags2 & WIREFRAME) (*pSetRenderState)((void *)*ppD3Ddev9, D3DRS_FILLMODE, D3DFILL_WIREFRAME);
		if(dxw.dwFlags4 & DISABLEFOGGING) (*pSetRenderState)((void *)*ppD3Ddev9, D3DRS_FOGENABLE, FALSE);
		if(dxw.dwFlags4 & ZBUFFERALWAYS) (*pSetRenderState)((void *)*ppD3Ddev9, D3DRS_ZFUNC, D3DCMP_ALWAYS);
		//if(1) (*pSetRenderState)((void *)*ppD3Ddev9, D3DRS_SPECULARENABLE, TRUE);
	}
	if (dxw.dwFlags4 & NOTEXTURES) SetHook((void *)(**(DWORD **)ppD3Ddev9 + 260), extSetTexture9, (void **)&pSetTexture9, "SetTexture(D9)");
	if (!(dxw.dwTFlags & OUTPROXYTRACE)) return;
	SetHook((void *)(**(DWORD **)ppD3Ddev9 +  4), extAddRef9, (void **)&pAddRef9, "AddRef(D9)");
	SetHook((void *)(**(DWORD **)ppD3Ddev9 +  8), extRelease9, (void **)&pRelease9, "Release(D9)");
}

void HookDirect3D8(void *lpd3d)
{
	SetHook((void *)(*(DWORD *)lpd3d +  0), extQueryInterfaceD3D8, (void **)&pQueryInterfaceD3D8, "QueryInterface(D8)");
	SetHook((void *)(*(DWORD *)lpd3d + 16), extGetAdapterCount8, (void **)&pGetAdapterCount8, "GetAdapterCount(D8)");
	SetHook((void *)(*(DWORD *)lpd3d + 20), extGetAdapterIdentifier8, (void **)&pGetAdapterIdentifier8, "GetAdapterIdentifier(D8)");
	SetHook((void *)(*(DWORD *)lpd3d + 28), extEnumAdapterModes8, (void **)&pEnumAdapterModes8, "EnumAdapterModes(D8)");
	SetHook((void *)(*(DWORD *)lpd3d + 32), extGetAdapterDisplayMode8, (void **)&pGetAdapterDisplayMode8, "GetAdapterDisplayMode(D8)");
	SetHook((void *)(*(DWORD *)lpd3d + 60), extCreateDevice, (void **)&pCreateDevice8, "CreateDevice(D8)");
}

void* WINAPI extDirect3DCreate8(UINT sdkversion)
{
	void *lpd3d;

	OutTraceD3D("Direct3DCreate8: sdkversion=%x\n", sdkversion);
	lpd3d = (*pDirect3DCreate8)(sdkversion);
	if(!lpd3d) {
		OutTraceE("Direct3DCreate8: ERROR err=%d\n", GetLastError());
		return 0;
	}
	dwD3DVersion = 8;
	HookDirect3D8(lpd3d);
	OutTraceD3D("Direct3DCreate8: d3d=%x\n", lpd3d);

	return lpd3d;
}

void HookDirect3D9(void *lpd3d, BOOL ex)
{
	SetHook((void *)(*(DWORD *)lpd3d +  0), extQueryInterfaceD3D9, (void **)&pQueryInterfaceD3D9, "QueryInterface(D9)");
	SetHook((void *)(*(DWORD *)lpd3d + 16), extGetAdapterCount9, (void **)&pGetAdapterCount9, "GetAdapterCount(D9)");
	SetHook((void *)(*(DWORD *)lpd3d + 20), extGetAdapterIdentifier9, (void **)&pGetAdapterIdentifier9, "GetAdapterIdentifier(D9)");
	SetHook((void *)(*(DWORD *)lpd3d + 28), extEnumAdapterModes9, (void **)&pEnumAdapterModes9, "EnumAdapterModes(D9)");
	SetHook((void *)(*(DWORD *)lpd3d + 32), extGetAdapterDisplayMode9, (void **)&pGetAdapterDisplayMode9, "GetAdapterDisplayMode(D9)");
	SetHook((void *)(*(DWORD *)lpd3d + 64), extCreateDevice, (void **)&pCreateDevice9, "CreateDevice(D9)");
	if(ex) SetHook((void *)(*(DWORD *)lpd3d + 80), extCreateDeviceEx, (void **)&pCreateDeviceEx, "CreateDeviceEx(D9)");
}

void* WINAPI extDirect3DCreate9(UINT sdkversion)
{
	void *lpd3d; 

	OutTraceD3D("Direct3DCreate9: sdkversion=%x\n", sdkversion);
	lpd3d = (*pDirect3DCreate9)(sdkversion);
	if(!lpd3d) {
		OutTraceE("Direct3DCreate9: ERROR err=%d\n", GetLastError());
		return 0;
	}

	dwD3DVersion = 9;
	HookDirect3D9(lpd3d, FALSE);
	OutTraceD3D("Direct3DCreate9: d3d=%x\n", lpd3d);

	return lpd3d;
}

HRESULT WINAPI extDirect3DCreate9Ex(UINT sdkversion, IDirect3D9Ex **ppD3D)
{
	void *lpd3d;
	HRESULT res;

	OutTraceD3D("Direct3DCreate9Ex: sdkversion=%x\n", sdkversion);
	res = (*pDirect3DCreate9Ex)(sdkversion, ppD3D);
	if(res) {
		OutTraceE("Direct3DCreate9Ex: ERROR res=%x(%s)\n", res, ExplainDDError(res));
		return res;
	}
	dwD3DVersion = 9;
	lpd3d = *ppD3D;
	HookDirect3D9(lpd3d, TRUE);
	OutTraceD3D("Direct3DCreate9Ex: d3d=%x\n", lpd3d);
	return res;
}

UINT WINAPI extGetAdapterCount8(void *lpd3d)
{
	UINT res;
	res=(*pGetAdapterCount8)(lpd3d);
	OutTraceD3D("GetAdapterCount(8): count=%d\n", res);
	if(dxw.dwFlags2 & HIDEMULTIMONITOR) {
		OutTraceDW("GetAdapterCount: HIDEMULTIMONITOR count=1\n");
		res=1;
	}
	return res;
}

UINT WINAPI extGetAdapterCount9(void *lpd3d)
{
	UINT res;
	res=(*pGetAdapterCount9)(lpd3d);
	OutTraceD3D("GetAdapterCount(9): count=%d\n", res);
	if(dxw.dwFlags2 & HIDEMULTIMONITOR) {
		OutTraceDW("GetAdapterCount: HIDEMULTIMONITOR count=1\n");
		res=1;
	}
	return res;
}

HRESULT WINAPI extGetAdapterIdentifier8(void *pd3dd, UINT Adapter, DWORD Flags, D3DADAPTER_IDENTIFIER9 *pIdentifier)
{
	HRESULT res;
	OutTraceD3D("GetAdapterIdentifier(8): Adapter=%d flags=%x\n", Adapter, Flags);
	res=pGetAdapterIdentifier8(pd3dd, Adapter, Flags, pIdentifier);
	OutTraceD3D("GetAdapterIdentifier(8): ret=%x\n", res);
	return res;
}

HRESULT WINAPI extGetAdapterIdentifier9(void *pd3dd, UINT Adapter, DWORD Flags, D3DADAPTER_IDENTIFIER9 *pIdentifier)
{
	HRESULT res;
	OutTraceD3D("GetAdapterIdentifier(9): Adapter=%d flags=%x\n", Adapter, Flags);
	res=pGetAdapterIdentifier9(pd3dd, Adapter, Flags, pIdentifier);
	OutTraceD3D("GetAdapterIdentifier(9): ret=%x\n", res);
	return res;
}

HRESULT WINAPI extReset(void *pd3dd, D3DPRESENT_PARAMETERS* pPresParam)
{
	HRESULT res;
	DWORD param[64], *tmp;
	D3DDISPLAYMODE mode;
	int Windowed;
	void *pD3D;

	memcpy(param, pPresParam, (dwD3DVersion == 9)?56:52);

	if(IsTraceDW){
		tmp = param;
		OutTrace("D3D%d::Reset\n", dwD3DVersion);
		OutTrace("    BackBufferWidth = %i\n", *(tmp ++));
		OutTrace("    BackBufferHeight = %i\n", *(tmp ++));
		OutTrace("    BackBufferFormat = %i\n", *(tmp ++));
		OutTrace("    BackBufferCount = %i\n", *(tmp ++));
		OutTrace("    MultiSampleType = %i\n", *(tmp ++));
		if(dwD3DVersion == 9) OutTrace("    MultiSampleQuality = %i\n", *(tmp ++));
		OutTrace("    SwapEffect = 0x%x\n", *(tmp ++));
		OutTrace("    hDeviceWindow = 0x%x\n", *(tmp ++));
		OutTrace("    Windowed = %i\n", (Windowed=*(tmp ++)));
		OutTrace("    EnableAutoDepthStencil = %i\n", *(tmp ++));
		OutTrace("    AutoDepthStencilFormat = %i\n", *(tmp ++));
		OutTrace("    Flags = 0x%x\n", *(tmp ++));
		OutTrace("    FullScreen_RefreshRateInHz = %i\n", *(tmp ++));
		OutTrace("    PresentationInterval = 0x%x\n", *(tmp ++));
	}

	if(dxw.Windowize){
		if(Windowed) {
			// do not attempt to reset when returning to windowed mode: it is useless (you are windowed already)
			// and the code below (GetDirect3D, ...) doesn't work.
			OutTraceDW("SKIPPED!\n");
			return DD_OK;
		}

		if (dwD3DVersion == 9){
			res=(*pGetDirect3D9)(pd3dd, &pD3D);
			if(res) {
				OutTraceDW("GetDirect3D FAILED! %x\n", res);
				return(DD_OK);
			}
			OutTraceDW("GetDirect3D pd3d=%x\n", pD3D);
			res=(*pGetAdapterDisplayMode9)(pD3D, 0, &mode);
			if(res) {
				OutTraceDW("GetAdapterDisplayMode FAILED! %x\n", res);
				return(DD_OK);
			}
			param[2] = mode.Format;
			param[7] = 0;			//hDeviceWindow
			dxw.SetFullScreen(~param[8]?TRUE:FALSE); 
			param[8] = 1;			//Windowed
			//param[11] = D3DPRESENTFLAG_DEVICECLIP;			//Flags;
			param[12] = 0;			//FullScreen_RefreshRateInHz;
			param[13] = D3DPRESENT_INTERVAL_DEFAULT;	//PresentationInterval
		}
		else{
			res=(*pGetDirect3D8)(pd3dd, &pD3D);
			if(res) {
				OutTraceDW("GetDirect3D FAILED! %x\n", res);
				return(DD_OK);
			}
			OutTraceDW("GetDirect3D pd3d=%x\n", pD3D);
			res=(*pGetAdapterDisplayMode8)(pD3D, 0, &mode);
			if(res) {
				OutTraceDW("GetAdapterDisplayMode FAILED! %x\n", res);
				return(DD_OK);
			}
			param[2] = mode.Format;
			param[6] = 0;			//hDeviceWindow
			dxw.SetFullScreen(~param[7]?TRUE:FALSE); 
			param[7] = 1;			//Windowed
			//param[10] = D3DPRESENTFLAG_DEVICECLIP;			//Flags;
			param[11] = 0;			//FullScreen_RefreshRateInHz;
			param[12] = D3DPRESENT_INTERVAL_DEFAULT;	//PresentationInterval
		}
	}

	if(!(dxw.dwFlags4 & NOD3DRESET)){ 
		res = (*pReset)(pd3dd, (D3DPRESENT_PARAMETERS *)param);
		if(res == D3DERR_INVALIDCALL){
			OutTraceDW("FAILED! D3DERR_INVALIDCALL\n", res);
			return D3DERR_INVALIDCALL;
		}
		if(res){
			OutTraceDW("FAILED! %x\n", res);
			return res;
		}
		OutTraceDW("SUCCESS!\n");
		(dwD3DVersion == 8) ? HookD3DDevice8(&pd3dd) : HookD3DDevice9(&pd3dd);
	}
	else{
		OutTraceDW("SKIPPED!\n");
		res=D3D_OK;
	}

	dxw.SetScreenSize(pPresParam->BackBufferWidth, pPresParam->BackBufferHeight);

	GetHookInfo()->IsFullScreen = dxw.IsFullScreen();
	GetHookInfo()->DXVersion=(short)dwD3DVersion;
	GetHookInfo()->Height=(short)dxw.GetScreenHeight();
	GetHookInfo()->Width=(short)dxw.GetScreenWidth();
	GetHookInfo()->ColorDepth=(short)dxw.VirtualPixelFormat.dwRGBBitCount;
	
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
	dxw.ShowOverlay();
	return res;
}

HRESULT WINAPI extGetDisplayMode8(void *lpd3d, D3DDISPLAYMODE *pMode)
{
	HRESULT res;
	res=(*pGetDisplayMode8)(lpd3d, pMode);
	OutTraceD3D("GetDisplayMode(8): size=(%dx%d) RefreshRate=%d Format=%d\n",
		pMode->Width, pMode->Height, pMode->RefreshRate, pMode->Format);
	if(dxw.dwFlags2 & KEEPASPECTRATIO){
		pMode->Width=dxw.iSizX;
		pMode->Height=dxw.iSizY;
	}
	else{
		pMode->Width = dxw.GetScreenWidth();
		pMode->Height = dxw.GetScreenHeight();
	}
	OutTraceDW("GetDisplayMode(8): fixed size=(%dx%d)\n", pMode->Width, pMode->Height);
	return res;
}

HRESULT WINAPI extGetDisplayMode9(void *lpd3d, UINT iSwapChain, D3DDISPLAYMODE *pMode)
{
	HRESULT res;
	res=(*pGetDisplayMode9)(lpd3d, iSwapChain, pMode);
	OutTraceD3D("GetDisplayMode(9): SwapChain=%d size=(%dx%d) RefreshRate=%d Format=%d\n",
		iSwapChain, pMode->Width, pMode->Height, pMode->RefreshRate, pMode->Format);
	if(dxw.dwFlags2 & KEEPASPECTRATIO){
		pMode->Width=dxw.iSizX;
		pMode->Height=dxw.iSizY;
	}
	else{
		pMode->Width = dxw.GetScreenWidth();
		pMode->Height = dxw.GetScreenHeight();
	}
	OutTraceDW("GetDisplayMode(9): fixed size=(%dx%d)\n", pMode->Width, pMode->Height);
	return res;
}

HRESULT WINAPI extEnumAdapterModes8(void *lpd3d, UINT Adapter, UINT Mode, D3DDISPLAYMODE* pMode)
{
	HRESULT res;
	OutTraceD3D("EnumAdapterModes(8): adapter=%d mode=%d pMode=%x\n", Adapter, Mode, pMode);
	res=(*pEnumAdapterModes8)(lpd3d, Adapter, Mode, pMode);
	if(res) OutTraceE("EnumAdapterModes ERROR: err=%x(%s)\n", res, ExplainDDError(res));
	return res;
}

HRESULT WINAPI extEnumAdapterModes9(void *lpd3d, UINT Adapter, D3DFORMAT Format, UINT Mode, D3DDISPLAYMODE* pMode)
{
	HRESULT res;
	OutTraceD3D("EnumAdapterModes(9): adapter=%d format=%x mode=%d pMode=%x\n", Adapter, Format, Mode, pMode);
	res=(*pEnumAdapterModes9)(lpd3d, Adapter, Format, Mode, pMode);
	if(res) OutTraceE("EnumAdapterModes ERROR: err=%x(%s)\n", res, ExplainDDError(res));
	return res;
}

HRESULT WINAPI extGetAdapterDisplayMode8(void *lpd3d, UINT Adapter, D3DDISPLAYMODE *pMode)
{
	HRESULT res;
	res=(*pGetAdapterDisplayMode8)(lpd3d, Adapter, pMode);
	OutTraceD3D("GetAdapterDisplayMode(8): size=(%dx%d) RefreshRate=%d Format=%d\n",
		pMode->Width, pMode->Height, pMode->RefreshRate, pMode->Format);
	if(dxw.dwFlags2 & KEEPASPECTRATIO){
		pMode->Width=dxw.iSizX;
		pMode->Height=dxw.iSizY;
	}
	else{
		pMode->Width = dxw.GetScreenWidth();
		pMode->Height = dxw.GetScreenHeight();
	}
	OutTraceDW("GetAdapterDisplayMode(8): fixed size=(%dx%d)\n", pMode->Width, pMode->Height);
	return res;
}

HRESULT WINAPI extGetAdapterDisplayMode9(void *lpd3d, UINT Adapter, D3DDISPLAYMODE *pMode)
{
	HRESULT res;
	res=(*pGetAdapterDisplayMode9)(lpd3d, Adapter, pMode);
	OutTraceD3D("GetAdapterDisplayMode(9): size=(%dx%d) RefreshRate=%d Format=%d\n",
		pMode->Width, pMode->Height, pMode->RefreshRate, pMode->Format);
	if(dxw.dwFlags2 & KEEPASPECTRATIO){
		pMode->Width=dxw.iSizX;
		pMode->Height=dxw.iSizY;
	}
	else{
		pMode->Width = dxw.GetScreenWidth();
		pMode->Height = dxw.GetScreenHeight();
	}
	OutTraceDW("GetAdapterDisplayMode(9): fixed size=(%dx%d)\n", pMode->Width, pMode->Height);
	return res;
}

void FixD3DWindowFrame(HWND hfocuswindow)
{
	if(!(dxw.dwFlags3 & NOWINDOWMOVE)) 
		AdjustWindowFrame(dxw.GethWnd(), dxw.GetScreenWidth(), dxw.GetScreenHeight());

	if(dxw.dwFlags3 & FIXD3DFRAME){
		char ClassName[81];
		RECT workarea;
		GetClassName(dxw.GethWnd(), ClassName, 80);
		(*pGetClientRect)(dxw.GethWnd(), &workarea);
		if (dxw.dwFlags2 & KEEPASPECTRATIO) dxw.FixWorkarea(&workarea);
		hfocuswindow=(*pCreateWindowExA)(
			0, ClassName, "child", 
			WS_CHILD|WS_VISIBLE, 
			//GetSystemMetrics(SM_CXSIZEFRAME), GetSystemMetrics(SM_CYSIZEFRAME)+GetSystemMetrics(SM_CYCAPTION), 
			workarea.left, workarea.top, workarea.right-workarea.left, workarea.bottom-workarea.top, 
			dxw.GethWnd(), NULL, NULL, NULL);
		if (hfocuswindow) 
			OutTraceDW("CreateDevice: updated hfocuswindow=%x pos=(%d,%d) size=(%d,%d)\n", 
				hfocuswindow, workarea.left, workarea.top, workarea.right-workarea.left, workarea.bottom-workarea.top);
		else
			OutTraceE("CreateDevice: CreateWindowEx ERROR err=%d\n", GetLastError());
		dxw.SethWnd(hfocuswindow, dxw.GethWnd());
	}
}

HRESULT WINAPI extCreateDevice(void *lpd3d, UINT adapter, D3DDEVTYPE devicetype,
	HWND hfocuswindow, DWORD behaviorflags, D3DPRESENT_PARAMETERS *ppresentparam, void **ppd3dd)
{
	HRESULT res;
	DWORD param[64], *tmp;
	D3DDISPLAYMODE mode;
	int Windowed;

	OutTraceD3D("CreateDevice: D3DVersion=%d lpd3d=%x adapter=%x hFocusWnd=%x behavior=%x, size=(%d,%d)\n",
		dwD3DVersion, lpd3d, adapter, hfocuswindow, behaviorflags, ppresentparam->BackBufferWidth, ppresentparam->BackBufferHeight);

	memcpy(param, ppresentparam, (dwD3DVersion == 9)?56:52);
	dxw.SethWnd(hfocuswindow);
	dxw.SetScreenSize(param[0], param[1]);

	if(dxw.Windowize) FixD3DWindowFrame(hfocuswindow);

	if(IsTraceDW){
		tmp = param;
		OutTrace("D3D%d::CreateDevice\n", dwD3DVersion);
		OutTrace("  Adapter = %i\n", adapter);
		OutTrace("  DeviceType = %i\n", devicetype);
		OutTrace("  hFocusWindow = 0x%x\n", hfocuswindow);
		OutTrace("  BehaviorFlags = 0x%x\n", behaviorflags);
		OutTrace("    BackBufferWidth = %i\n", *(tmp ++));
		OutTrace("    BackBufferHeight = %i\n", *(tmp ++));
		OutTrace("    BackBufferFormat = %i\n", *(tmp ++));
		OutTrace("    BackBufferCount = %i\n", *(tmp ++));
		OutTrace("    MultiSampleType = %i\n", *(tmp ++));
		if(dwD3DVersion == 9) OutTrace("    MultiSampleQuality = %i\n", *(tmp ++));
		OutTrace("    SwapEffect = 0x%x\n", *(tmp ++));
		OutTrace("    hDeviceWindow = 0x%x\n", *(tmp ++));
		OutTrace("    Windowed = %i\n", (Windowed=*(tmp ++)));
		OutTrace("    EnableAutoDepthStencil = %i\n", *(tmp ++));
		OutTrace("    AutoDepthStencilFormat = %i\n", *(tmp ++));
		OutTrace("    Flags = 0x%x\n", *(tmp ++));
		OutTrace("    FullScreen_RefreshRateInHz = %i\n", *(tmp ++));
		OutTrace("    PresentationInterval = 0x%x\n", *(tmp ++));
	}

	if(dwD3DVersion == 9)
		(*pGetAdapterDisplayMode9)(lpd3d, 0, &mode);
	else
		(*pGetAdapterDisplayMode8)(lpd3d, 0, &mode);
	param[2] = mode.Format;
	OutTraceDW("    Current Format = 0x%x\n", mode.Format);
	OutTraceDW("    Current ScreenSize = (%dx%d)\n", mode.Width, mode.Height);
	OutTraceDW("    Current Refresh Rate = %d\n", mode.RefreshRate);

	if(dwD3DVersion == 9){
		if(dxw.Windowize){
			param[7] = 0;			//hDeviceWindow
			dxw.SetFullScreen(~param[8]?TRUE:FALSE); 
			param[8] = 1;			//Windowed
			//param[11] = D3DPRESENTFLAG_DEVICECLIP;			//Flags;
			param[12] = 0;			//FullScreen_RefreshRateInHz;
			param[13] = D3DPRESENT_INTERVAL_DEFAULT;	//PresentationInterval
		}
		res = (*pCreateDevice9)(lpd3d, 0, devicetype, hfocuswindow, behaviorflags, param, ppd3dd);
	}
	else{
		if(dxw.Windowize){
			param[6] = 0;			//hDeviceWindow
			dxw.SetFullScreen(~param[7]?TRUE:FALSE); 
			param[7] = 1;			//Windowed
			//param[10] = D3DPRESENTFLAG_DEVICECLIP;			//Flags;
			param[11] = 0;			//FullScreen_RefreshRateInHz;
			param[12] = D3DPRESENT_INTERVAL_DEFAULT;	//PresentationInterval
		}
		res = (*pCreateDevice8)(lpd3d, 0, devicetype, hfocuswindow, behaviorflags, param, ppd3dd);
	}

	if(res){
		OutTraceDW("FAILED! %x\n", res);
		return res;
	}
	OutTraceDW("SUCCESS! device=%x\n", *ppd3dd);

	if(dwD3DVersion == 8){ 
		HookD3DDevice8(ppd3dd);
	}
	else {
		HookD3DDevice9(ppd3dd);
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

	OutTraceD3D("CreateDeviceEx: D3DVersion=%d lpd3d=%x adapter=%x hFocusWnd=%x behavior=%x, size=(%d,%d)\n",
		dwD3DVersion, lpd3d, adapter, hfocuswindow, behaviorflags, ppresentparam->BackBufferWidth, ppresentparam->BackBufferHeight);

	memcpy(param, ppresentparam, 56);
	dxw.SethWnd(hfocuswindow);
	dxw.SetScreenSize(param[0], param[1]);

	if(dxw.Windowize) FixD3DWindowFrame(hfocuswindow);

	tmp = param;
	if(IsTraceDW){
		OutTrace("D3D%d::CreateDeviceEx\n", dwD3DVersion);
		OutTrace("  Adapter = %i\n", adapter);
		OutTrace("  DeviceType = %i\n", devicetype);
		OutTrace("  hFocusWindow = 0x%x\n", hfocuswindow);
		OutTrace("  BehaviorFlags = 0x%x\n", behaviorflags);
		OutTrace("    BackBufferWidth = %i\n", *(tmp ++));
		OutTrace("    BackBufferHeight = %i\n", *(tmp ++));
		OutTrace("    BackBufferFormat = %i\n", *(tmp ++));
		OutTrace("    BackBufferCount = %i\n", *(tmp ++));
		OutTrace("    MultiSampleType = %i\n", *(tmp ++));
		OutTrace("    MultiSampleQuality = %i\n", *(tmp ++));
		OutTrace("    SwapEffect = 0x%x\n", *(tmp ++));
		OutTrace("    hDeviceWindow = 0x%x\n", *(tmp ++));
		OutTrace("    Windowed = %i\n", (Windowed=*(tmp ++)));
		OutTrace("    EnableAutoDepthStencil = %i\n", *(tmp ++));
		OutTrace("    AutoDepthStencilFormat = %i\n", *(tmp ++));
		OutTrace("    Flags = 0x%x\n", *(tmp ++));
		OutTrace("    FullScreen_RefreshRateInHz = %i\n", *(tmp ++));
		OutTrace("    PresentationInterval = 0x%x\n", *(tmp ++));
	}

	//((LPDIRECT3D9)lpd3d)->GetAdapterDisplayMode(0, &mode);
	(*pGetAdapterDisplayMode9)(lpd3d, 0, &mode);
	param[2] = mode.Format;
	OutTraceDW("    Current Format = 0x%x\n", mode.Format);
	OutTraceDW("    Current ScreenSize = (%dx%d)\n", mode.Width, mode.Height);
	OutTraceDW("    Current Refresh Rate = %d\n", mode.RefreshRate);

	if(dxw.Windowize){
		//param[7] = 0;			//hDeviceWindow
		param[7] = (DWORD)dxw.GethWnd();			//hDeviceWindow
		dxw.SetFullScreen(~param[8]?TRUE:FALSE); 
		param[8] = 1;			//Windowed
		param[12] = 0;			//FullScreen_RefreshRateInHz;
		param[13] = D3DPRESENT_INTERVAL_DEFAULT;	//PresentationInterval
	}

	res = (*pCreateDeviceEx)(lpd3d, 0, devicetype, hfocuswindow, behaviorflags, param, pFullscreenDisplayMode, ppd3dd);
	if(res){
		OutTraceDW("FAILED! %x\n", res);
		return res;
	}
	OutTraceDW("SUCCESS!\n"); 

	HookD3DDevice9(ppd3dd);

	GetHookInfo()->IsFullScreen = dxw.IsFullScreen();
	GetHookInfo()->DXVersion=(short)dwD3DVersion;
	GetHookInfo()->Height=(short)dxw.GetScreenHeight();
	GetHookInfo()->Width=(short)dxw.GetScreenWidth();
	GetHookInfo()->ColorDepth=(short)dxw.VirtualPixelFormat.dwRGBBitCount;
	
	return 0;
}

extern char *ExplainRenderstateValue(DWORD Value);

HRESULT WINAPI extSetRenderState(void *pd3dd, D3DRENDERSTATETYPE State, DWORD Value) 
{
	HRESULT res;
	OutTraceD3D("SetRenderState: d3dd=%x State=%x(%s) Value=%x\n", pd3dd, State, ExplainD3DRenderState(State), Value);

	if((dxw.dwFlags4 & ZBUFFERALWAYS) && (State == D3DRS_ZFUNC)) {
		OutTraceD3D("SetRenderState: FIXED State=ZFUNC Value=%s->D3DCMP_ALWAYS\n", ExplainRenderstateValue(Value));
		Value = D3DCMP_ALWAYS;
	}
	if((dxw.dwFlags2 & WIREFRAME) && (State == D3DRS_FILLMODE)){
		OutTraceD3D("SetRenderState: FIXED State=FILLMODE Value=%x->D3DFILL_WIREFRAME\n", Value);
		Value = D3DFILL_WIREFRAME;
	}
	if((dxw.dwFlags4 & DISABLEFOGGING) && (State == D3DRS_FOGENABLE)){
		OutTraceD3D("SetRenderState: FIXED State=FOGENABLE Value=%x->FALSE\n", Value);
		Value = FALSE;
	}

	res=(*pSetRenderState)(pd3dd, State, Value);
	if(res) OutTraceE("SetRenderState: res=%x(%s)\n", res, ExplainDDError(res));
	return res;
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
	OutTraceD3D("SetViewport: declared pos=(%d,%d) size=(%d,%d) depth=(%f;%f)\n", 
		pViewport->X, pViewport->Y, pViewport->Width, pViewport->Height, pViewport->MinZ, pViewport->MaxZ);
	if(IsDebug) OutTrace("SetViewport: DEBUG win=(%d,%d) screen=(%d,%d)\n",
		client.right, client.bottom, dxw.GetScreenWidth(), dxw.GetScreenHeight());
	pViewport->X = (pViewport->X * (int)client.right) / (int)dxw.GetScreenWidth();
	pViewport->Y = (pViewport->Y * (int)client.bottom) / (int)dxw.GetScreenHeight();
	pViewport->Width = (pViewport->Width * (int)client.right) / (int)dxw.GetScreenWidth();
	pViewport->Height = (pViewport->Height * (int)client.bottom) / (int)dxw.GetScreenHeight();
	OutTraceDW("SetViewport: remapped pos=(%d,%d) size=(%d,%d)\n", 
		pViewport->X, pViewport->Y, pViewport->Width, pViewport->Height);
	return (*pSetViewport)(pd3dd, pViewport);
}

HRESULT WINAPI extCreateAdditionalSwapChain(void *lpd3d, D3DPRESENT_PARAMETERS *pPresentationParameters, IDirect3DSwapChain9 **ppSwapChain)
{
	HRESULT res;
	DWORD param[64], *tmp;
	D3DDISPLAYMODE mode;
	int Windowed;

	OutTraceD3D("CreateAdditionalSwapChain: d3d=%x\n", lpd3d);

	memcpy(param, pPresentationParameters, (dwD3DVersion == 9)?56:52);
	dxw.SetScreenSize(param[0], param[1]);
	if(dxw.Windowize) FixD3DWindowFrame(dxw.GethWnd());

	tmp = param;
	if(IsTraceDW){
		OutTrace("D3D%d::CreateAdditionalSwapChain\n", dwD3DVersion);
		OutTrace("    BackBufferWidth = %i\n", *(tmp ++));
		OutTrace("    BackBufferHeight = %i\n", *(tmp ++));
		OutTrace("    BackBufferFormat = %i\n", *(tmp ++));
		OutTrace("    BackBufferCount = %i\n", *(tmp ++));
		OutTrace("    MultiSampleType = %i\n", *(tmp ++));
		if(dwD3DVersion == 9) OutTrace("    MultiSampleQuality = %i\n", *(tmp ++));
		OutTrace("    SwapEffect = 0x%x\n", *(tmp ++));
		OutTrace("    hDeviceWindow = 0x%x\n", *(tmp ++));
		OutTrace("    Windowed = %i\n", (Windowed=*(tmp ++)));
		OutTrace("    EnableAutoDepthStencil = %i\n", *(tmp ++));
		OutTrace("    AutoDepthStencilFormat = %i\n", *(tmp ++));
		OutTrace("    Flags = 0x%x\n", *(tmp ++));
		OutTrace("    FullScreen_RefreshRateInHz = %i\n", *(tmp ++));
		OutTrace("    PresentationInterval = 0x%x\n", *(tmp ++));
	}

	if (dwD3DVersion == 9)
		(*pGetAdapterDisplayMode9)(lpd3d, 0, &mode);
	else
		(*pGetAdapterDisplayMode8)(lpd3d, 0, &mode);
	param[2] = mode.Format;
	OutTraceDW("    Current Format = 0x%x\n", mode.Format);
	OutTraceDW("    Current ScreenSize = (%dx%d)\n", mode.Width, mode.Height);
	OutTraceDW("    Current Refresh Rate = %d\n", mode.RefreshRate);

	if(dxw.Windowize){
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
	OutTraceD3D("D3D10CreateDevice: DriverType=%x Flags=%x SDKVersion=%x\n", DriverType, Flags, SDKVersion);
	//return 0x887a0004;
	res=(*pD3D10CreateDevice)(pAdapter, DriverType, Software, Flags, SDKVersion, ppDevice);
	if(res){
		OutTraceE("D3D10CreateDevice: ret=%x(%s)\n", res, ExplainDDError(res));
		return res;
	}
	SetHook((void *)(*(DWORD *)*ppDevice + 100), extRSSetViewports, (void **)&pRSSetViewports, "RSSetViewports(D10)");

	if(dxw.Windowize){
		ViewPort.TopLeftX=dxw.iPosX;
		ViewPort.TopLeftY=dxw.iPosY;
		ViewPort.Width=dxw.iSizX;
		ViewPort.Height=dxw.iSizY;
		ViewPort.MinDepth=1.0;
		ViewPort.MaxDepth=1.0;
		(*pRSSetViewports)((void *)*ppDevice, 1, (D3D11_VIEWPORT *)&ViewPort);
	}
	OutTraceD3D("D3D10CreateDevice: ret=%x\n", res);
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
	OutTraceD3D("D3D10CreateDevice1: DriverType=%x Flags=%x HardwareLevel=%x SDKVersion=%x\n", DriverType, Flags, HardwareLevel, SDKVersion);
	res=(*pD3D10CreateDevice1)(pAdapter, DriverType, Software, Flags, HardwareLevel, SDKVersion, ppDevice);
	SetHook((void *)(*(DWORD *)*ppDevice + 100), extRSSetViewports, (void **)&pRSSetViewports, "RSSetViewports(D10)");
	OutTraceD3D("D3D10CreateDevice1: ret=%x\n", res);
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
	OutTraceD3D("D3D10CreateDeviceAndSwapChain: DriverType=%x Flags=%x SDKVersion=%x\n", DriverType, Flags, SDKVersion);
	//return 0x887a0004;
	if(dxw.Windowize){
		pSwapChainDesc->OutputWindow = dxw.GethWnd();
		pSwapChainDesc->Windowed = true;	
	}
	res=(*pD3D10CreateDeviceAndSwapChain)(pAdapter, DriverType, Software, Flags, SDKVersion, pSwapChainDesc, ppSwapChain, ppDevice);
	OutTraceD3D("D3D10CreateDeviceAndSwapChain ret=%x\n", res);
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
	OutTraceD3D("D3D10CreateDeviceAndSwapChain1: DriverType=%x Flags=%x SDKVersion=%x\n", DriverType, Flags, SDKVersion);
	//return 0x887a0004;
	if(dxw.Windowize){
		pSwapChainDesc->OutputWindow = dxw.GethWnd();
		pSwapChainDesc->Windowed = true;	
	}
	res=(*pD3D10CreateDeviceAndSwapChain1)(pAdapter, DriverType, Software, Flags, SDKVersion, pSwapChainDesc, ppSwapChain, ppDevice);
	OutTraceD3D("D3D10CreateDeviceAndSwapChain1 ret=%x\n", res);
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
	OutTraceD3D("D3D11CreateDevice: DriverType=%x Flags=%x FeatureLevels=%x SDKVersion=%x\n", DriverType, Flags, FeatureLevels, SDKVersion);
	//return 0x887a0004;
	res=(*pD3D11CreateDevice)(pAdapter, DriverType, Software, Flags, pFeatureLevels, FeatureLevels, SDKVersion, ppDevice, pFeatureLevel, ppImmediateContext);
	if(res){
		OutTraceE("D3D11CreateDevice: ret=%x\n", res);
		return res;
	}
	SetHook((void *)(*(DWORD *)ppImmediateContext + 148), extRSSetViewports, (void **)&pRSSetViewports, "RSSetViewports(D11)");
	//SetHook((void *)(*(DWORD *)ppImmediateContext + 152), extRSSetScissorRects, (void **)&pRSSetScissorRects, "RSSetScissorRects(D11)");
	OutTraceD3D("D3D11CreateDevice ret=%x\n", res);
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
	OutTraceD3D("D3D11CreateDeviceAndSwapChain\n");
	res=(*pD3D11CreateDeviceAndSwapChain)(pAdapter, DriverType, Software, Flags, pFeatureLevels, FeatureLevels, SDKVersion, pSwapChainDesc, ppSwapChain, ppDevice, pFeatureLevel, ppImmediateContext);
	OutTraceD3D("D3D11CreateDeviceAndSwapChain ret=%x\n", res);
	return res;
}

void WINAPI extRSSetViewports(ID3D11DeviceContext *This, UINT NumViewports, D3D11_VIEWPORT *pViewports)
{
	OutTraceD3D("RSSetViewports: NumViewports=%d\n", NumViewports);

	if(dxw.Windowize && (NumViewports==1)){
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
	OutTraceD3D("D3D::QueryInterface(8): d3d=%x riid=%x\n", obj, riid.Data1);
	res=pQueryInterfaceD3D8(obj, riid, ppvObj);
	if(res)
		OutTraceE("D3D::QueryInterface(8): ERROR ret=%x(%s)\n", res, ExplainDDError(res));
	else
		OutTraceD3D("D3D::QueryInterface(8): obp=%x\n", *ppvObj);
	return res;
}

HRESULT WINAPI extQueryInterfaceDev8(void *obj, REFIID riid, void** ppvObj)
{
	HRESULT res;
	OutTraceD3D("Device::QueryInterface(8): d3d=%x riid=%x\n", obj, riid.Data1);
	res=pQueryInterfaceDev8(obj, riid, ppvObj);
	if(res){
		OutTraceE("Device::QueryInterface(8): ERROR ret=%x(%s)\n", res, ExplainDDError(res));
		return res;
	}
	OutTraceD3D("Device::QueryInterface(8): obp=%x\n", *ppvObj);

	switch(*(DWORD *)&riid){
	case 0x7385E5DF: // IID_IDirect3DDevice8
		HookD3DDevice8(ppvObj);
		break;
	case 0xD0223B96: // IID_IDirect3DDevice9
		HookD3DDevice9(ppvObj);
		break;
	}
	return res;
}

HRESULT WINAPI extQueryInterfaceD3D9(void *obj, REFIID riid, void** ppvObj)
{
	HRESULT res;
	OutTraceD3D("D3D::QueryInterface(9): d3d=%x riid=%x\n", obj, riid.Data1);
	res=pQueryInterfaceD3D9(obj, riid, ppvObj);
	if(res)
		OutTraceE("D3D::QueryInterface(9): ERROR ret=%x(%s)\n", res, ExplainDDError(res));
	else
		OutTraceD3D("D3D::QueryInterface(9): obp=%x\n", *ppvObj);
	return res;
}

HRESULT WINAPI extQueryInterfaceDev9(void *obj, REFIID riid, void** ppvObj)
{
	HRESULT res;

	OutTraceD3D("Device::QueryInterface(9): d3d=%x riid=%x\n", obj, riid.Data1);
	res=pQueryInterfaceDev9(obj, riid, ppvObj);
	if(res){
		OutTraceD3D("Device::QueryInterface(9): ERROR ret=%x\n", res);
		return res;
	}
	OutTraceD3D("Device::QueryInterface(9): ppvObj=%x\n", *ppvObj);

	switch(*(DWORD *)&riid){
	case 0x7385E5DF: // IID_IDirect3DDevice8
		HookD3DDevice8(ppvObj);
		break;
	case 0xD0223B96: // IID_IDirect3DDevice9
		HookD3DDevice9(ppvObj);
		break;
	}

	return res;
}

HRESULT WINAPI extGetDirect3D8(void *lpdd3dd, void **ppD3D8)
{
	HRESULT res;
	OutTraceD3D("Device::GetDirect3D(8): d3dd=%x\n", lpdd3dd);
	res=(*pGetDirect3D8)(lpdd3dd, ppD3D8);
	if(res)
		OutTraceE("Device::GetDirect3D ERROR: d3dd=%x ret=%x\n", lpdd3dd, res);
	else{
		OutTraceD3D("Device::GetDirect3D: d3dd=%x d3d=%x\n", lpdd3dd, *ppD3D8);
		HookDirect3D8(*ppD3D8);
	}
	return res;
}

HRESULT WINAPI extGetDirect3D9(void *lpdd3dd, void **ppD3D9)
{
	HRESULT res;
	OutTraceD3D("Device::GetDirect3D(9): d3dd=%x\n", lpdd3dd);
	res=(*pGetDirect3D9)(lpdd3dd, ppD3D9);
	if(res)
		OutTraceE("Device::GetDirect3D ERROR: d3dd=%x ret=%x\n", lpdd3dd, res);
	else{
		OutTraceD3D("Device::GetDirect3D: d3dd=%x d3d=%x\n", lpdd3dd, *ppD3D9);
		// re-hook d3d session: neeeded for Need for Speed Underground
		HookDirect3D9(*ppD3D9, FALSE);
	}
	return res;
}

HRESULT WINAPI extCheckFullScreen(void)
{
	HRESULT res;
	if(dxw.Windowize){
		OutTraceDW("CheckFullScreen: return OK\n");
		res=0;
	}
	else{
		res=(*pCheckFullScreen)();
		OutTraceDW("CheckFullScreen: return %x\n", res);
	}
	return res;
}

void WINAPI extSetGammaRamp(void *lpdd3dd, UINT iSwapChain, DWORD Flags, D3DGAMMARAMP *pRamp)
{
	OutTraceD3D("SetGammaRamp: dd3dd=%x SwapChain=%d flags=%x\n", lpdd3dd, iSwapChain, Flags);
	if (dxw.dwFlags2 & DISABLEGAMMARAMP) return;
	(*pSetGammaRamp)(lpdd3dd, iSwapChain, Flags, pRamp);
}

void WINAPI extGetGammaRamp(void *lpdd3dd, UINT iSwapChain, D3DGAMMARAMP *pRamp)
{
	OutTraceD3D("GetGammaRamp: dd3dd=%x SwapChain=%d\n", lpdd3dd, iSwapChain);
	(*pGetGammaRamp)(lpdd3dd, iSwapChain, pRamp);
}

void WINAPI extSetCursorPosition9(void *lpdd3dd, int X, int Y, DWORD Flags)
{
	OutTraceD3D("SetCursorPosition: dd3dd=%x pos=(%d,%d) flags=%x\n", lpdd3dd, X, Y, Flags);
	if(dxw.dwFlags2 & KEEPCURSORFIXED) return;
	// to do: proper scaling.....
	//(*pSetCursorPosition9)(lpdd3dd, X, Y, Flags);
}

void WINAPI extSetCursorPosition8(void *lpdd3dd, int X, int Y, DWORD Flags)
{
	OutTraceD3D("SetCursorPosition: dd3dd=%x pos=(%d,%d) flags=%x\n", lpdd3dd, X, Y, Flags);
	if(dxw.dwFlags2 & KEEPCURSORFIXED) return;
	// to do: proper scaling.....
	//(*pSetCursorPosition8)(lpdd3dd, X, Y, Flags);
}

ULONG WINAPI extAddRef9(void *lpdd3dd)
{
	ULONG res;
	res=(*pAddRef9)(lpdd3dd);
	OutTraceD3D("Device::AddRef(9): dd3dd=%x res=%x\n", lpdd3dd, res);
	return res;
}

ULONG WINAPI extRelease9(void *lpdd3dd)
{
	ULONG res;
	res=(*pRelease9)(lpdd3dd);
	OutTraceD3D("Device::Release(9): dd3dd=%x res=%x\n", lpdd3dd, res);
	return res;
}

//ULONG WINAPI extCreateRenderTarget8(void *lpdd3dd, UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, BOOL Lockable, IDirect3DSurface8** ppSurface) 
ULONG WINAPI extCreateRenderTarget8(void *lpdd3dd, UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, BOOL Lockable, void** ppSurface) 
{
	ULONG res;

	OutTraceD3D("Device::CreateRenderTarget(8): dd3dd=%x dim=(%dx%d) Format=%x Lockable=%x\n", 
		lpdd3dd, Width, Height, Format, Lockable);
	res=(*pCreateRenderTarget8)(lpdd3dd, Width, Height, Format, MultiSample, Lockable, ppSurface);
	OutTraceD3D("Device::CreateRenderTarget(8): res=%x\n", res);
	return res;
}

ULONG WINAPI extBeginScene8(void *lpdd3dd)
{
	ULONG res;
	OutTraceD3D("Device::BeginScene(8): d3dd=%x\n", lpdd3dd);
	res=(*pBeginScene8)(lpdd3dd);
	if (res) OutTraceE("Device::BeginScene(8) ERROR: err=%x\n", res);
	return res;
}

ULONG WINAPI extBeginScene9(void *lpdd3dd)
{
	ULONG res;
	OutTraceD3D("Device::BeginScene(9): d3dd=%x\n", lpdd3dd);
	res=(*pBeginScene9)(lpdd3dd);
	if (res) OutTraceE("Device::BeginScene(9) ERROR: err=%x\n", res);
	return res;
}

ULONG WINAPI extEndScene8(void *lpdd3dd)
{
	ULONG res;
	res=(*pEndScene8)(lpdd3dd);
	if (res) OutTraceE("Device::EndScene(8) ERROR: err=%x\n", res);
	return res;
}

ULONG WINAPI extEndScene9(void *lpdd3dd)
{
	ULONG res;
	res=(*pEndScene9)(lpdd3dd);
	if (res) OutTraceE("Device::EndScene(9) ERROR: err=%x\n", res);
	return res;
}

ULONG WINAPI extSetTexture8(void *lpd3dd, DWORD Stage, void* pTexture)
{
	(*pSetTexture8)(lpd3dd, Stage, NULL);
	OutTraceD3D("Device::SetTexture(8): d3dd=%x stage=%x\n", lpd3dd, Stage);
	return DD_OK;
}

ULONG WINAPI extSetTexture9(void *lpd3dd, DWORD Stage, void* pTexture)
{
	(*pSetTexture9)(lpd3dd, Stage, NULL);
	OutTraceD3D("Device::SetTexture(9): d3dd=%x stage=%x\n", lpd3dd, Stage);
	return DD_OK;
}

BOOL WINAPI extShowCursor8(void *lpd3dd, BOOL bShow)
{
	BOOL res, bNew;
	bNew=bShow;
	if(dxw.dwFlags2 & SHOWHWCURSOR) bNew=TRUE;
	if(dxw.dwFlags1 & HIDEHWCURSOR) bNew=FALSE;
	res=(*pShowCursor8)(lpd3dd, bNew);
	if(bNew==bShow)
		OutTraceD3D("Device::ShowCursor(8): d3dd=%x show=%x res=%x\n", lpd3dd, bNew, res);
	else
		OutTraceD3D("Device::ShowCursor(8): d3dd=%x show=%x->%x res=%x\n", lpd3dd, bShow, bNew, res);
	return res;
}

BOOL WINAPI extShowCursor9(void *lpd3dd, BOOL bShow)
{
	BOOL res, bNew;
	bNew=bShow;
	if(dxw.dwFlags2 & SHOWHWCURSOR) bNew=TRUE;
	if(dxw.dwFlags1 & HIDEHWCURSOR) bNew=FALSE;
	res=(*pShowCursor9)(lpd3dd, bNew);
	if(bNew==bShow)
		OutTraceD3D("Device::ShowCursor(9): d3dd=%x show=%x res=%x\n", lpd3dd, bNew, res);
	else
		OutTraceD3D("Device::ShowCursor(9): d3dd=%x show=%x->%x res=%x\n", lpd3dd, bShow, bNew, res);
	return res;
}
