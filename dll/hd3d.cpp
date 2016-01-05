//#define D3D10_IGNORE_SDK_LAYERS 1
#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <d3d9.h>
#include <d3d8caps.h>
#include <D3D10_1.h>
#include <D3D10Misc.h>
#include <D3D11.h>
#include "dxwnd.h"
#include "dxwcore.hpp"
#include "dxhook.h"
#include "dxhelper.h"
#include "syslibs.h"
#include "stdio.h"
#include "hd3d.doc.hpp" // fake include to hold documentation

#define HOOKD3D10ANDLATER 1
#define TRACEALLMETHODS 1

extern void D3D9TextureHandling(void *, int);
extern void D3D8TextureHandling(void *, int);

void *lpD3DActiveDevice = NULL;
typedef HRESULT (WINAPI *QueryInterface_Type)(void *, REFIID riid, void** ppvObj);

// D3D8/9 API

typedef void* (WINAPI *Direct3DCreate8_Type)(UINT);
typedef void* (WINAPI *Direct3DCreate9_Type)(UINT);
typedef HRESULT (WINAPI *Direct3DCreate9Ex_Type)(UINT, IDirect3D9Ex **);
typedef HRESULT (WINAPI *CheckFullScreen_Type)(void);
typedef BOOL (WINAPI * DisableD3DSpy_Type)(void);

void* WINAPI extDirect3DCreate8(UINT);
void* WINAPI extDirect3DCreate9(UINT);
HRESULT WINAPI extDirect3DCreate9Ex(UINT, IDirect3D9Ex **);
HRESULT WINAPI extCheckFullScreen(void);
HRESULT WINAPI voidDirect3DShaderValidatorCreate9(void);
void WINAPI voidDebugSetLevel(void);
void WINAPI voidDebugSetMute(void);
BOOL WINAPI voidDisableD3DSpy(void);
BOOL WINAPI extDisableD3DSpy(void);

Direct3DCreate8_Type pDirect3DCreate8 = 0;
Direct3DCreate9_Type pDirect3DCreate9 = 0;
Direct3DCreate9Ex_Type pDirect3DCreate9Ex = 0;
CheckFullScreen_Type pCheckFullScreen = 0;
DisableD3DSpy_Type pDisableD3DSpy = 0;

// IDirect3D8/9 methods 

typedef UINT	(WINAPI *GetAdapterCount_Type)(void *);
typedef HRESULT (WINAPI *GetAdapterIdentifier_Type)(void *, UINT, DWORD, D3DADAPTER_IDENTIFIER9 *);
typedef UINT	(WINAPI *GetAdapterModeCount_Type)(void *, UINT, D3DFORMAT);
typedef HRESULT (WINAPI *EnumAdapterModes8_Type)(void *, UINT, UINT, D3DDISPLAYMODE *);
typedef HRESULT (WINAPI *EnumAdapterModes9_Type)(void *, UINT, D3DFORMAT ,UINT, D3DDISPLAYMODE *);
typedef HRESULT (WINAPI *GetAdapterDisplayMode_Type)(void *, UINT, D3DDISPLAYMODE *);
typedef HRESULT (WINAPI *CheckDeviceType_Type)(void *, UINT, D3DDEVTYPE, D3DFORMAT, D3DFORMAT, BOOL);
typedef HRESULT (WINAPI *D3DGetDeviceCaps8_Type)(void *, UINT, D3DDEVTYPE, D3DCAPS8 *);
typedef HRESULT (WINAPI *D3DGetDeviceCaps9_Type)(void *, UINT, D3DDEVTYPE, D3DCAPS9 *);
typedef HMONITOR (WINAPI *GetAdapterMonitor_Type)(void *, UINT);
typedef HRESULT (WINAPI *CreateDevice_Type)(void *, UINT, D3DDEVTYPE, HWND, DWORD, void *, void **);
typedef HRESULT (WINAPI *CreateDeviceEx_Type)(void *, UINT, D3DDEVTYPE, HWND, DWORD, void *, D3DDISPLAYMODEEX *, void **);
typedef BOOL	(WINAPI *DisableD3DSpy_Type)(void);

UINT WINAPI extGetAdapterCount8(void *);
UINT WINAPI extGetAdapterCount9(void *);
HRESULT WINAPI extGetAdapterIdentifier8(void *, UINT, DWORD, D3DADAPTER_IDENTIFIER9 *);
HRESULT WINAPI extGetAdapterIdentifier9(void *, UINT, DWORD, D3DADAPTER_IDENTIFIER9 *);
HRESULT WINAPI extCheckDeviceType8(void *, UINT, D3DDEVTYPE, D3DFORMAT, D3DFORMAT, BOOL);
HRESULT WINAPI extCheckDeviceType9(void *, UINT, D3DDEVTYPE, D3DFORMAT, D3DFORMAT, BOOL);
HRESULT WINAPI extD3DGetDeviceCaps8(void *, UINT, D3DDEVTYPE, D3DCAPS8 *);
HRESULT WINAPI extD3DGetDeviceCaps9(void *, UINT, D3DDEVTYPE, D3DCAPS9 *);
HMONITOR WINAPI extGetAdapterMonitor8(void *, UINT);
HMONITOR WINAPI extGetAdapterMonitor9(void *, UINT);
HRESULT WINAPI extCreateDevice8(void *, UINT, D3DDEVTYPE, HWND, DWORD, D3DPRESENT_PARAMETERS *, void **);
HRESULT WINAPI extCreateDevice9(void *, UINT, D3DDEVTYPE, HWND, DWORD, D3DPRESENT_PARAMETERS *, void **);
HRESULT WINAPI extCreateDeviceEx(void *, UINT, D3DDEVTYPE, HWND, DWORD, D3DPRESENT_PARAMETERS *, D3DDISPLAYMODEEX *, void **);

GetAdapterCount_Type pGetAdapterCount8, pGetAdapterCount9;
GetAdapterIdentifier_Type pGetAdapterIdentifier8, pGetAdapterIdentifier9;
CheckDeviceType_Type pCheckDeviceType8, pCheckDeviceType9;
D3DGetDeviceCaps8_Type pD3DGetDeviceCaps8 = 0;
D3DGetDeviceCaps9_Type pD3DGetDeviceCaps9 = 0;
GetAdapterMonitor_Type pGetAdapterMonitor8, pGetAdapterMonitor9;
CreateDevice_Type pCreateDevice8, pCreateDevice9;
CreateDeviceEx_Type pCreateDeviceEx = 0;

// IDirect3DDevice8/9 methods

typedef UINT	(WINAPI *GetAvailableTextureMem_Type)(void *);
typedef HRESULT (WINAPI *TestCooperativeLevel_Type)(void *);
typedef HRESULT (WINAPI *GetDirect3D8_Type)(void *, void **);
typedef HRESULT (WINAPI *GetDirect3D9_Type)(void *, void **);
typedef HRESULT (WINAPI *GetDisplayMode8_Type)(void *, D3DDISPLAYMODE *);
typedef HRESULT (WINAPI *GetDisplayMode9_Type)(void *, UINT, D3DDISPLAYMODE *);
typedef void	(WINAPI *SetCursorPosition9_Type)(void *, int, int, DWORD);
typedef void	(WINAPI *SetCursorPosition8_Type)(void *, int, int, DWORD);
typedef BOOL	(WINAPI *ShowCursor8_Type)(void *, BOOL);
typedef BOOL	(WINAPI *ShowCursor9_Type)(void *, BOOL);
typedef HRESULT (WINAPI *CreateAdditionalSwapChain_Type)(void *, D3DPRESENT_PARAMETERS *, IDirect3DSwapChain9 **);
typedef HRESULT (WINAPI *GetSwapChain_Type)(void *, UINT, IDirect3DSwapChain9**);
typedef UINT (WINAPI *GetNumberOfSwapChains_Type)(void *);
typedef HRESULT (WINAPI *BeginStateBlock_Type)(void *);
typedef HRESULT (WINAPI *EndStateBlock8_Type)(void *, DWORD *);
typedef HRESULT (WINAPI *EndStateBlock9_Type)(void *, IDirect3DStateBlock9**);
typedef HRESULT (WINAPI *CreateTexture8_Type)(void *, UINT, UINT, UINT, DWORD, D3DFORMAT, D3DPOOL, void **);
typedef HRESULT (WINAPI *CreateTexture9_Type)(void *, UINT, UINT, UINT, DWORD, D3DFORMAT, D3DPOOL, void **, HANDLE *);

UINT	WINAPI extGetAvailableTextureMem8(void *);
UINT	WINAPI extGetAvailableTextureMem9(void *);
HRESULT WINAPI extTestCooperativeLevel8(void *);
HRESULT WINAPI extTestCooperativeLevel9(void *);
HRESULT WINAPI extGetDirect3D8(void *, void **);
HRESULT WINAPI extGetDirect3D9(void *, void **);
HRESULT WINAPI extGetDisplayMode8(void *, D3DDISPLAYMODE *);
HRESULT WINAPI extGetDisplayMode9(void *, UINT, D3DDISPLAYMODE *);
void	WINAPI extSetCursorPosition9(void *, int, int, DWORD);
void	WINAPI extSetCursorPosition8(void *, int, int, DWORD);
BOOL	WINAPI extShowCursor8(void *, BOOL);
BOOL	WINAPI extShowCursor9(void *, BOOL);
HRESULT WINAPI extCreateAdditionalSwapChain8(void *, D3DPRESENT_PARAMETERS *, IDirect3DSwapChain9 **);
HRESULT WINAPI extCreateAdditionalSwapChain9(void *, D3DPRESENT_PARAMETERS *, IDirect3DSwapChain9 **);
HRESULT WINAPI extGetSwapChain(void *, UINT, IDirect3DSwapChain9**);
UINT WINAPI extGetNumberOfSwapChains(void *);
HRESULT WINAPI extBeginStateBlock8(void *);
HRESULT WINAPI extBeginStateBlock9(void *);
HRESULT WINAPI extEndStateBlock8(void *, DWORD *);
HRESULT WINAPI extEndStateBlock9(void *, IDirect3DStateBlock9**);
HRESULT WINAPI extCreateTexture8(void *, UINT, UINT, UINT, DWORD, D3DFORMAT, D3DPOOL, void **);
HRESULT WINAPI extCreateTexture9(void *, UINT, UINT, UINT, DWORD, D3DFORMAT, D3DPOOL, void **, HANDLE *);

GetAvailableTextureMem_Type pGetAvailableTextureMem8, pGetAvailableTextureMem9;
TestCooperativeLevel_Type pTestCooperativeLevel8, pTestCooperativeLevel9;
GetDirect3D8_Type pGetDirect3D8 = 0;
GetDirect3D9_Type pGetDirect3D9 = 0;
GetDisplayMode8_Type pGetDisplayMode8 = 0;
GetDisplayMode9_Type pGetDisplayMode9 = 0;
SetCursorPosition9_Type pSetCursorPosition9 = 0;
SetCursorPosition8_Type pSetCursorPosition8 = 0;
ShowCursor8_Type pShowCursor8 = 0;
ShowCursor9_Type pShowCursor9 = 0;
CreateAdditionalSwapChain_Type pCreateAdditionalSwapChain8 = 0;
CreateAdditionalSwapChain_Type pCreateAdditionalSwapChain9 = 0;
GetSwapChain_Type pGetSwapChain = 0;
GetNumberOfSwapChains_Type pGetNumberOfSwapChains = 0;
BeginStateBlock_Type pBeginStateBlock8 = 0;
BeginStateBlock_Type pBeginStateBlock9 = 0;
EndStateBlock8_Type pEndStateBlock8 = 0;
EndStateBlock9_Type pEndStateBlock9 = 0;
CreateTexture8_Type pCreateTexture8 = 0;
CreateTexture9_Type pCreateTexture9 = 0;

// IDirect3DTexture8/9 methods

typedef HRESULT (WINAPI *LockRect_Type)(void *, UINT, D3DLOCKED_RECT *, CONST RECT *, DWORD);
typedef HRESULT (WINAPI *UnlockRect_Type)(void *, UINT);

HRESULT WINAPI extLockRect8(void *, UINT, D3DLOCKED_RECT *, CONST RECT *, DWORD);
HRESULT WINAPI extLockRect9(void *, UINT, D3DLOCKED_RECT *, CONST RECT *, DWORD);
HRESULT WINAPI extUnlockRect8(void *, UINT);
HRESULT WINAPI extUnlockRect9(void *, UINT);

LockRect_Type pLockRect8, pLockRect9;
UnlockRect_Type pUnlockRect8, pUnlockRect9;

// to sort ...

typedef HRESULT (WINAPI *Present_Type)(void *, CONST RECT *, CONST RECT *, HWND, CONST RGNDATA *);
typedef HRESULT (WINAPI *SetRenderState_Type)(void *, D3DRENDERSTATETYPE, DWORD);
typedef HRESULT (WINAPI *GetRenderState_Type)(void *, D3DRENDERSTATETYPE, DWORD );
typedef HRESULT (WINAPI *GetViewport_Type)(void *, D3DVIEWPORT9 *);
typedef HRESULT (WINAPI *SetViewport_Type)(void *, D3DVIEWPORT9 *);
typedef void	(WINAPI *SetGammaRamp_Type)(void *, UINT, DWORD, D3DGAMMARAMP *);
typedef void	(WINAPI *GetGammaRamp_Type)(void *, UINT, D3DGAMMARAMP *);
typedef HRESULT (WINAPI *SetTexture8_Type)(void *, DWORD, void *);
typedef HRESULT (WINAPI *SetTexture9_Type)(void *, DWORD, void *);

//typedef ULONG	(WINAPI *CreateRenderTarget8_Type)(void *, UINT, UINT, D3DFORMAT, D3DMULTISAMPLE_TYPE, BOOL, IDirect3DSurface8**);
typedef ULONG	(WINAPI *CreateRenderTarget8_Type)(void *, UINT, UINT, D3DFORMAT, D3DMULTISAMPLE_TYPE, BOOL, void**);
typedef ULONG	(WINAPI *CreateRenderTarget9_Type)(void *, UINT, UINT, D3DFORMAT, D3DMULTISAMPLE_TYPE, BOOL, void**);
typedef ULONG	(WINAPI *BeginScene_Type)(void *);
typedef ULONG	(WINAPI *EndScene_Type)(void *);

typedef HRESULT (WINAPI *D3D10CreateDevice_Type)(IDXGIAdapter *, D3D10_DRIVER_TYPE, HMODULE, UINT, UINT, ID3D10Device **);
typedef HRESULT (WINAPI *D3D10CreateDeviceAndSwapChain_Type)(IDXGIAdapter *, D3D10_DRIVER_TYPE, HMODULE, UINT, UINT, DXGI_SWAP_CHAIN_DESC *, IDXGISwapChain **, ID3D10Device **);
typedef HRESULT (WINAPI *D3D10CreateDevice1_Type)(IDXGIAdapter *, D3D10_DRIVER_TYPE, HMODULE, UINT, D3D10_FEATURE_LEVEL1, UINT, ID3D10Device **);
typedef HRESULT (WINAPI *D3D10CreateDeviceAndSwapChain1_Type)(IDXGIAdapter *, D3D10_DRIVER_TYPE, HMODULE, UINT, UINT, DXGI_SWAP_CHAIN_DESC *, IDXGISwapChain **, ID3D10Device **);
typedef HRESULT (WINAPI *D3D11CreateDevice_Type)(IDXGIAdapter *, D3D_DRIVER_TYPE, HMODULE, UINT, const D3D_FEATURE_LEVEL *, UINT, UINT, ID3D11Device **, D3D_FEATURE_LEVEL *, ID3D11DeviceContext **);
typedef HRESULT (WINAPI *D3D11CreateDeviceAndSwapChain_Type)(IDXGIAdapter *, D3D_DRIVER_TYPE, HMODULE, UINT, const D3D_FEATURE_LEVEL *, UINT, UINT, const DXGI_SWAP_CHAIN_DESC *, IDXGISwapChain **, ID3D11Device **, D3D_FEATURE_LEVEL *, ID3D11DeviceContext **);
typedef void	(WINAPI *RSSetViewports10_Type)(void *, UINT, D3D10_VIEWPORT *);
typedef void	(WINAPI *RSSetViewports11_Type)(void *, UINT, D3D11_VIEWPORT *);
typedef ULONG	(WINAPI *AddRef_Type)(void *);
typedef ULONG	(WINAPI *Release_Type)(void *);
typedef HRESULT (WINAPI *Reset_Type)(void *, D3DPRESENT_PARAMETERS*);

HRESULT WINAPI extQueryInterfaceD3D8(void *, REFIID, void** );
HRESULT WINAPI extQueryInterfaceDev8(void *, REFIID, void** );
HRESULT WINAPI extQueryInterfaceD3D9(void *, REFIID, void** );
HRESULT WINAPI extQueryInterfaceDev9(void *, REFIID, void** );

HRESULT WINAPI extEnumAdapterModes8(void *, UINT, UINT , D3DDISPLAYMODE *);
HRESULT WINAPI extEnumAdapterModes9(void *, UINT, D3DFORMAT, UINT , D3DDISPLAYMODE *);
HRESULT WINAPI extGetAdapterDisplayMode8(void *, UINT, D3DDISPLAYMODE *);
HRESULT WINAPI extGetAdapterDisplayMode9(void *, UINT, D3DDISPLAYMODE *);
HRESULT WINAPI extPresent8(void *, CONST RECT *, CONST RECT *, HWND, CONST RGNDATA *);
HRESULT WINAPI extPresent9(void *, CONST RECT *, CONST RECT *, HWND, CONST RGNDATA *);
HRESULT WINAPI extSetRenderState8(void *, D3DRENDERSTATETYPE, DWORD);
HRESULT WINAPI extSetRenderState9(void *, D3DRENDERSTATETYPE, DWORD);
HRESULT WINAPI extGetRenderState8(void *, D3DRENDERSTATETYPE, DWORD);
HRESULT WINAPI extGetRenderState9(void *, D3DRENDERSTATETYPE, DWORD);
HRESULT WINAPI extGetViewport(void *, D3DVIEWPORT9 *);
HRESULT WINAPI extSetViewport(void *, D3DVIEWPORT9 *);
HRESULT WINAPI extReset8(void *, D3DPRESENT_PARAMETERS *);
HRESULT WINAPI extReset9(void *, D3DPRESENT_PARAMETERS *);
void	WINAPI extSetGammaRamp8(void *, UINT, DWORD, D3DGAMMARAMP *);
void	WINAPI extSetGammaRamp9(void *, UINT, DWORD, D3DGAMMARAMP *);
void	WINAPI extGetGammaRamp8(void *, UINT, D3DGAMMARAMP *);
void	WINAPI extGetGammaRamp9(void *, UINT, D3DGAMMARAMP *);
ULONG WINAPI extAddRef9(void *);
ULONG WINAPI extRelease9(void *);
BOOL  WINAPI voidDisableD3DSpy(void);

ULONG WINAPI extCreateRenderTarget8(void *, UINT, UINT, D3DFORMAT, D3DMULTISAMPLE_TYPE, BOOL, void**);
ULONG WINAPI extCreateRenderTarget9(void *, UINT, UINT, D3DFORMAT, D3DMULTISAMPLE_TYPE, BOOL, void**);
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
void WINAPI extRSSetViewports10(void *, UINT, D3D10_VIEWPORT *);
void WINAPI extRSSetViewports11(ID3D11DeviceContext *, UINT, D3D11_VIEWPORT *);

extern char *ExplainDDError(DWORD);

QueryInterface_Type pQueryInterfaceD3D8 = 0;
QueryInterface_Type pQueryInterfaceDev8 = 0;
QueryInterface_Type pQueryInterfaceD3D9 = 0;
QueryInterface_Type pQueryInterfaceDev9 = 0;

EnumAdapterModes8_Type pEnumAdapterModes8 = 0;
EnumAdapterModes9_Type pEnumAdapterModes9 = 0;
GetAdapterDisplayMode_Type pGetAdapterDisplayMode8 = 0;
GetAdapterDisplayMode_Type pGetAdapterDisplayMode9 = 0;
Present_Type pPresent8 = 0;
Present_Type pPresent9 = 0;
SetRenderState_Type pSetRenderState8, pSetRenderState9;
GetRenderState_Type pGetRenderState8, pGetRenderState9;
GetViewport_Type pGetViewport = 0;
SetViewport_Type pSetViewport = 0;
SetGammaRamp_Type pSetGammaRamp8, pSetGammaRamp9;
GetGammaRamp_Type pGetGammaRamp8, pGetGammaRamp9;

CreateRenderTarget8_Type pCreateRenderTarget8 = 0;
CreateRenderTarget9_Type pCreateRenderTarget9 = 0;
BeginScene_Type pBeginScene8, pBeginScene9;
EndScene_Type pEndScene8, pEndScene9;
Reset_Type pReset8, pReset9;
SetTexture8_Type pSetTexture8 = 0;
SetTexture9_Type pSetTexture9 = 0;

D3D10CreateDevice_Type pD3D10CreateDevice = 0;
D3D10CreateDeviceAndSwapChain_Type pD3D10CreateDeviceAndSwapChain = 0;
D3D10CreateDevice1_Type pD3D10CreateDevice1 = 0;
D3D10CreateDeviceAndSwapChain1_Type pD3D10CreateDeviceAndSwapChain1 = 0;
D3D11CreateDevice_Type pD3D11CreateDevice = 0;
D3D11CreateDeviceAndSwapChain_Type pD3D11CreateDeviceAndSwapChain = 0;
RSSetViewports10_Type pRSSetViewports10 = 0;
RSSetViewports11_Type pRSSetViewports11 = 0;

AddRef_Type pAddRef9 = 0;
Release_Type pRelease9 = 0;

DWORD dwD3DVersion;
DWORD dwD3DSwapEffect;

static HookEntryEx_Type d3d8Hooks[]={
	{HOOK_HOT_CANDIDATE, 0, "Direct3DCreate8", (FARPROC)NULL, (FARPROC *)&pDirect3DCreate8, (FARPROC)extDirect3DCreate8},
	{HOOK_IAT_CANDIDATE, 0, 0, NULL, 0, 0} // terminator
};


static HookEntryEx_Type d3d9Hooks[]={
	{HOOK_HOT_CANDIDATE, 0, "Direct3DCreate9", (FARPROC)NULL, (FARPROC *)&pDirect3DCreate9, (FARPROC)extDirect3DCreate9},
	{HOOK_HOT_CANDIDATE, 0, "Direct3DCreate9Ex", (FARPROC)NULL, (FARPROC *)&pDirect3DCreate9Ex, (FARPROC)extDirect3DCreate9Ex},
	{HOOK_HOT_CANDIDATE, 0, "CheckFullScreen", (FARPROC)NULL, (FARPROC *)&pCheckFullScreen, (FARPROC)extCheckFullScreen},
	{HOOK_HOT_CANDIDATE, 0, "DisableD3DSpy", (FARPROC)NULL, (FARPROC *)&pDisableD3DSpy, (FARPROC)extDisableD3DSpy},
	{HOOK_IAT_CANDIDATE, 0, 0, NULL, 0, 0} // terminator
};

static HookEntryEx_Type d3d9Extra[]={
	{HOOK_HOT_CANDIDATE, 0, "Direct3DShaderValidatorCreate9", (FARPROC)NULL, (FARPROC *)NULL, (FARPROC)voidDirect3DShaderValidatorCreate9},
	{HOOK_HOT_CANDIDATE, 0, "DebugSetLevel", (FARPROC)NULL, (FARPROC *)NULL, (FARPROC)voidDebugSetLevel},
	{HOOK_HOT_CANDIDATE, 0, "DebugSetMute", (FARPROC)NULL, (FARPROC *)NULL, (FARPROC)voidDebugSetMute},
	{HOOK_HOT_CANDIDATE, 0, "DisableD3DSpy", (FARPROC)NULL, (FARPROC *)NULL, (FARPROC)voidDisableD3DSpy},
	{HOOK_IAT_CANDIDATE, 0, 0, NULL, 0, 0} // terminator
};

static HookEntryEx_Type d3d10Hooks[]={
	{HOOK_HOT_CANDIDATE, 0, "D3D10CreateDevice", (FARPROC)NULL, (FARPROC *)&pD3D10CreateDevice, (FARPROC)extD3D10CreateDevice},
	{HOOK_HOT_CANDIDATE, 0, "D3D10CreateDeviceAndSwapChain", (FARPROC)NULL, (FARPROC *)&pD3D10CreateDeviceAndSwapChain, (FARPROC)extD3D10CreateDeviceAndSwapChain},
	{HOOK_HOT_CANDIDATE, 0, "CheckFullScreen", (FARPROC)NULL, (FARPROC *)&pCheckFullScreen, (FARPROC)extCheckFullScreen},
	{HOOK_IAT_CANDIDATE, 0, 0, NULL, 0, 0} // terminator
};

static HookEntryEx_Type d3d10_1Hooks[]={
	{HOOK_HOT_CANDIDATE, 0, "D3D11CreateDevice", (FARPROC)NULL, (FARPROC *)&pD3D11CreateDevice, (FARPROC)extD3D11CreateDevice},
	{HOOK_HOT_CANDIDATE, 0, "D3D11CreateDeviceAndSwapChain", (FARPROC)NULL, (FARPROC *)&pD3D11CreateDeviceAndSwapChain, (FARPROC)extD3D11CreateDeviceAndSwapChain},
	{HOOK_IAT_CANDIDATE, 0, 0, NULL, 0, 0} // terminator
};

static HookEntryEx_Type d3d11Hooks[]={
	{HOOK_HOT_CANDIDATE, 0, "D3D10CreateDevice1", (FARPROC)NULL, (FARPROC *)&pD3D10CreateDevice1, (FARPROC)extD3D10CreateDevice1},
	{HOOK_HOT_CANDIDATE, 0, "D3D10CreateDeviceAndSwapChain1", (FARPROC)NULL, (FARPROC *)&pD3D10CreateDeviceAndSwapChain1, (FARPROC)extD3D10CreateDeviceAndSwapChain1},
	{HOOK_IAT_CANDIDATE, 0, 0, NULL, 0, 0} // terminator
};

FARPROC Remap_d3d8_ProcAddress(LPCSTR proc, HMODULE hModule)
{
	FARPROC addr;
	//if (dxw.dwTargetDDVersion == HOOKDDRAWNONE) return NULL;
	if (addr=RemapLibraryEx(proc, hModule, d3d8Hooks)) return addr;
	return NULL;
}

FARPROC Remap_d3d9_ProcAddress(LPCSTR proc, HMODULE hModule)
{
	FARPROC addr;
	//if (dxw.dwTargetDDVersion == HOOKDDRAWNONE) return NULL;
	if (addr=RemapLibraryEx(proc, hModule, d3d9Hooks)) return addr;
	if (dxw.dwFlags3 & SUPPRESSD3DEXT) if (addr=RemapLibraryEx(proc, hModule, d3d9Extra)) return addr;
	return NULL;
}

FARPROC Remap_d3d10_ProcAddress(LPCSTR proc, HMODULE hModule)
{
	FARPROC addr;
	//if (dxw.dwTargetDDVersion == HOOKDDRAWNONE) return NULL;
	if (addr=RemapLibraryEx(proc, hModule, d3d10Hooks)) return addr;
	return NULL;
}

FARPROC Remap_d3d10_1_ProcAddress(LPCSTR proc, HMODULE hModule) 
{
	FARPROC addr;
	//if (dxw.dwTargetDDVersion == HOOKDDRAWNONE) return NULL;
	if (addr=RemapLibraryEx(proc, hModule, d3d10_1Hooks)) return addr;
	return NULL;
}

FARPROC Remap_d3d11_ProcAddress(LPCSTR proc, HMODULE hModule)
{
	FARPROC addr;
	//if (dxw.dwTargetDDVersion == HOOKDDRAWNONE) return NULL;
	if (addr=RemapLibraryEx(proc, hModule, d3d11Hooks)) return addr;
	return NULL;
}

int HookDirect3D(HMODULE module, int version){
	LPDIRECT3D9 lpd3d;
	ID3D10Device *lpd3d10;
	ID3D11Device *lpd3d11;
	HRESULT res;

	OutTraceDW("HookDirect3D: module=%x version=%d\n", module, version);
	switch(version){
	case 0:
		HookLibraryEx(module, d3d8Hooks, "d3d8.dll");
		HookLibraryEx(module, d3d9Hooks, "d3d9.dll");
#ifdef HOOKD3D10ANDLATER
		HookLibraryEx(module, d3d10Hooks, "d3d10.dll");
		HookLibraryEx(module, d3d10_1Hooks, "d3d10_1.dll");
		HookLibraryEx(module, d3d11Hooks, "d3d11.dll");
#endif
		break;
	case 8:
		PinLibraryEx(d3d8Hooks, "d3d8.dll"); // pin for "Port Royale 2"
		HookLibraryEx(module, d3d8Hooks, "d3d8.dll");
		if(pDirect3DCreate8){
			lpd3d = (LPDIRECT3D9)extDirect3DCreate8(220);
			if(lpd3d) lpd3d->Release();
		}
		break;
	case 9:
		PinLibraryEx(d3d9Hooks, "d3d9.dll");// pin for "Affari tuoi"
		HookLibraryEx(module, d3d9Hooks, "d3d9.dll");
		if(pDirect3DCreate9){ 
			lpd3d = (LPDIRECT3D9)extDirect3DCreate9(31);
			if(lpd3d) lpd3d->Release();
		}
		break;
#ifdef HOOKD3D10ANDLATER
	case 10:
		PinLibraryEx(d3d10Hooks, "d3d10.dll");
		PinLibraryEx(d3d10_1Hooks, "d3d10_1.dll");
		HookLibraryEx(module, d3d10Hooks, "d3d10.dll");
		HookLibraryEx(module, d3d10_1Hooks, "d3d10_1.dll");
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
	break;
	case 11:
		PinLibraryEx(d3d11Hooks, "d3d11.dll");
		HookLibraryEx(module, d3d11Hooks, "d3d11.dll");
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
#endif
	}
	if(pDirect3DCreate8 || pDirect3DCreate9) return 1;
	return 0;
}

void HookD3DDevice8(void** ppD3Ddev8)
{
	OutTraceD3D("Device hook for IID_IDirect3DDevice8 interface\n");
	SetHook((void *)(**(DWORD **)ppD3Ddev8 +  0), extQueryInterfaceDev8, (void **)&pQueryInterfaceDev8, "QueryInterface(D8)");
	SetHook((void *)(**(DWORD **)ppD3Ddev8 + 12), extTestCooperativeLevel8, (void **)&pTestCooperativeLevel8, "TestCooperativeLevel(D8)");
	SetHook((void *)(**(DWORD **)ppD3Ddev8 + 16), extGetAvailableTextureMem8, (void **)&pGetAvailableTextureMem8, "GetAvailableTextureMem(D8)");
	SetHook((void *)(**(DWORD **)ppD3Ddev8 + 24), extGetDirect3D8, (void **)&pGetDirect3D8, "GetDirect3D(D8)");
	SetHook((void *)(**(DWORD **)ppD3Ddev8 + 32), extGetDisplayMode8, (void **)&pGetDisplayMode8, "GetDisplayMode(D8)");
	SetHook((void *)(**(DWORD **)ppD3Ddev8 + 44), extSetCursorPosition8, (void **)&pSetCursorPosition8, "SetCursorPosition(D8)");
	SetHook((void *)(**(DWORD **)ppD3Ddev8 + 48), extShowCursor8, (void **)&pShowCursor8, "ShowCursor(D8)");
	SetHook((void *)(**(DWORD **)ppD3Ddev8 + 52), extCreateAdditionalSwapChain8, (void **)&pCreateAdditionalSwapChain8, "CreateAdditionalSwapChain(D8)");
	SetHook((void *)(**(DWORD **)ppD3Ddev8 + 56), extReset8, (void **)&pReset8, "Reset(D8)");
	SetHook((void *)(**(DWORD **)ppD3Ddev8 + 60), extPresent8, (void **)&pPresent8, "Present(D8)");
	if(dxw.dwFlags2 & DISABLEGAMMARAMP){
		SetHook((void *)(**(DWORD **)ppD3Ddev8 + 72), extSetGammaRamp8, (void **)&pSetGammaRamp8, "SetGammaRamp(D8)");
		SetHook((void *)(**(DWORD **)ppD3Ddev8 + 76), extGetGammaRamp8, (void **)&pGetGammaRamp8, "GetGammaRamp(D8)");
	}
	if(dxw.dwFlags5 & TEXTUREMASK){
		SetHook((void *)(**(DWORD **)ppD3Ddev8 + 80), extCreateTexture8, (void **)&pCreateTexture8, "CreateTexture(D8)");
	}
	//SetHook((void *)(**(DWORD **)ppD3Ddev8 + 112), extCopyRects, (void **)&pCopyRects, "CopyRects(D8)");
	SetHook((void *)(**(DWORD **)ppD3Ddev8 + 100), extCreateRenderTarget8, (void **)&pCreateRenderTarget8, "CreateRenderTarget(D8)");
	SetHook((void *)(**(DWORD **)ppD3Ddev8 + 136), extBeginScene8, (void **)&pBeginScene8, "BeginScene(D8)");
	SetHook((void *)(**(DWORD **)ppD3Ddev8 + 140), extEndScene8, (void **)&pEndScene8, "EndScene(D8)");
	if((dxw.dwFlags2 & WIREFRAME) || (dxw.dwFlags4 & DISABLEFOGGING) || (dxw.dwFlags4 & ZBUFFERALWAYS)){
		SetHook((void *)(**(DWORD **)ppD3Ddev8 + 200), extSetRenderState8, (void **)&pSetRenderState8, "SetRenderState(D8)");
		SetHook((void *)(**(DWORD **)ppD3Ddev8 + 204), extGetRenderState8, (void **)&pGetRenderState8, "GetRenderState(D8)");
		if(dxw.dwFlags2 & WIREFRAME) (*pSetRenderState8)((void *)*ppD3Ddev8, D3DRS_FILLMODE, D3DFILL_WIREFRAME);
		if(dxw.dwFlags4 & DISABLEFOGGING) (*pSetRenderState8)((void *)*ppD3Ddev8, D3DRS_FOGENABLE, FALSE);
		if(dxw.dwFlags4 & ZBUFFERALWAYS) (*pSetRenderState8)((void *)*ppD3Ddev8, D3DRS_ZFUNC, D3DCMP_ALWAYS);
		//if(1) (*pSetRenderState8)((void *)*ppD3Ddev8, D3DRS_SPECULARENABLE, TRUE);
	}
	SetHook((void *)(**(DWORD **)ppD3Ddev8 + 208), extBeginStateBlock8, (void **)&pBeginStateBlock8, "BeginStateBlock(D8)");
	SetHook((void *)(**(DWORD **)ppD3Ddev8 + 212), extEndStateBlock8, (void **)&pEndStateBlock8, "EndStateBlock(D8)");
	if((dxw.dwFlags5 & TEXTUREMASK) || (dxw.dwFlags4 & NOTEXTURES)){
		SetHook((void *)(**(DWORD **)ppD3Ddev8 + 244), extSetTexture8, (void **)&pSetTexture8, "SetTexture(D8)");
	}
	//if (!(dxw.dwTFlags & OUTPROXYTRACE)) return;
	//SetHook((void *)(**(DWORD **)ppD3Ddev8 +  4), extAddRef8, (void **)&pAddRef8, "AddRef(D8)");
	//SetHook((void *)(**(DWORD **)ppD3Ddev8 +  8), extRelease8, (void **)&pRelease8, "Release(D8)");
}

void HookD3DDevice9(void** ppD3Ddev9)
{
	OutTraceD3D("Device hook for IID_IDirect3DDevice9 interface\n");
	SetHook((void *)(**(DWORD **)ppD3Ddev9 +  0), extQueryInterfaceDev9, (void **)&pQueryInterfaceDev9, "QueryInterface(D9)");
	SetHook((void *)(**(DWORD **)ppD3Ddev9 + 12), extTestCooperativeLevel9, (void **)&pTestCooperativeLevel9, "TestCooperativeLevel(D9)");
	SetHook((void *)(**(DWORD **)ppD3Ddev9 + 16), extGetAvailableTextureMem9, (void **)&pGetAvailableTextureMem9, "GetAvailableTextureMem(D9)");
	SetHook((void *)(**(DWORD **)ppD3Ddev9 + 24), extGetDirect3D9, (void **)&pGetDirect3D9, "GetDirect3D(D9)");
	SetHook((void *)(**(DWORD **)ppD3Ddev9 + 32), extGetDisplayMode9, (void **)&pGetDisplayMode9, "GetDisplayMode(D9)");
	SetHook((void *)(**(DWORD **)ppD3Ddev9 + 44), extSetCursorPosition9, (void **)&pSetCursorPosition9, "SetCursorPosition(D9)");
	SetHook((void *)(**(DWORD **)ppD3Ddev9 + 48), extShowCursor9, (void **)&pShowCursor9, "ShowCursor(D9)");
	SetHook((void *)(**(DWORD **)ppD3Ddev9 + 52), extCreateAdditionalSwapChain9, (void **)&pCreateAdditionalSwapChain9, "CreateAdditionalSwapChain(D9)");
	SetHook((void *)(**(DWORD **)ppD3Ddev9 + 56), extGetSwapChain, (void **)&pGetSwapChain, "GetSwapChain(D9)");
	SetHook((void *)(**(DWORD **)ppD3Ddev9 + 60), extGetNumberOfSwapChains, (void **)&pGetNumberOfSwapChains, "extGetNumberOfSwapChains(D9)");
	SetHook((void *)(**(DWORD **)ppD3Ddev9 + 64), extReset9, (void **)&pReset9, "Reset(D9)");
	SetHook((void *)(**(DWORD **)ppD3Ddev9 + 68), extPresent9, (void **)&pPresent9, "Present(D9)");
	if(dxw.dwFlags2 & DISABLEGAMMARAMP){
		SetHook((void *)(**(DWORD **)ppD3Ddev9 + 84), extSetGammaRamp9, (void **)&pSetGammaRamp9, "SetGammaRamp(D9)");
		SetHook((void *)(**(DWORD **)ppD3Ddev9 + 88), extGetGammaRamp9, (void **)&pGetGammaRamp9, "GetGammaRamp(D9)");
	}
	if(dxw.dwFlags5 & TEXTUREMASK){
		SetHook((void *)(**(DWORD **)ppD3Ddev9 + 92), extCreateTexture9, (void **)&pCreateTexture9, "CreateTexture(D9)");
	}
#ifdef DXWNDDISABLEDHOOKS
	SetHook((void *)(**(DWORD **)ppD3Ddev9 + 120), extUpdateSurface, (void **)&pUpdateSurface, "UpdateSurface(D9)");
	SetHook((void *)(**(DWORD **)ppD3Ddev9 + 124), extUpdateTexture, (void **)&pUpdateTexture, "UpdateTexture(D9)");
#endif

	//SetHook((void *)(**(DWORD **)ppD3Ddev9 + 112), extCreateRenderTarget9, (void **)&pCreateRenderTarget9, "CreateRenderTarget(D9)");
	SetHook((void *)(**(DWORD **)ppD3Ddev9 + 164), extBeginScene9, (void **)&pBeginScene9, "BeginScene(D9)");
	SetHook((void *)(**(DWORD **)ppD3Ddev9 + 168), extEndScene9, (void **)&pEndScene9, "EndScene(D9)");
	//SetHook((void *)(**(DWORD **)ppD3Ddev9 +188), extSetViewport, (void **)&pSetViewport, "SetViewport(D9)");
	//SetHook((void *)(**(DWORD **)ppD3Ddev9 +192), extGetViewport, (void **)&pGetViewport, "GetViewport(D9)");
	if((dxw.dwFlags2 & WIREFRAME) || (dxw.dwFlags4 & DISABLEFOGGING) || (dxw.dwFlags4 & ZBUFFERALWAYS)){
		SetHook((void *)(**(DWORD **)ppD3Ddev9 + 228), extSetRenderState9, (void **)&pSetRenderState9, "SetRenderState(D9)");
		SetHook((void *)(**(DWORD **)ppD3Ddev9 + 232), extGetRenderState9, (void **)&pGetRenderState9, "GetRenderState(D9)");
		if(dxw.dwFlags2 & WIREFRAME) (*pSetRenderState9)((void *)*ppD3Ddev9, D3DRS_FILLMODE, D3DFILL_WIREFRAME);
		if(dxw.dwFlags4 & DISABLEFOGGING) (*pSetRenderState9)((void *)*ppD3Ddev9, D3DRS_FOGENABLE, FALSE);
		if(dxw.dwFlags4 & ZBUFFERALWAYS) (*pSetRenderState9)((void *)*ppD3Ddev9, D3DRS_ZFUNC, D3DCMP_ALWAYS);
		//if(1) (*pSetRenderState9)((void *)*ppD3Ddev9, D3DRS_SPECULARENABLE, TRUE);
	}
	SetHook((void *)(**(DWORD **)ppD3Ddev9 + 240), extBeginStateBlock9, (void **)&pBeginStateBlock9, "BeginStateBlock(D9)");
	SetHook((void *)(**(DWORD **)ppD3Ddev9 + 244), extEndStateBlock9, (void **)&pEndStateBlock9, "EndStateBlock(D9)");
	if((dxw.dwFlags5 & TEXTUREMASK) || (dxw.dwFlags4 & NOTEXTURES)){
		SetHook((void *)(**(DWORD **)ppD3Ddev9 + 260), extSetTexture9, (void **)&pSetTexture9, "SetTexture(D9)");
	}
}

// WIP
void HookD3DTexture8(void** ppD3Dtex8)
{
	OutTraceD3D("Device hook for IDirect3DTexture8 interface\n");
	SetHook((void *)(**(DWORD **)ppD3Dtex8 + 64), extLockRect8, (void **)&pLockRect8, "LockRect(D8)");
	SetHook((void *)(**(DWORD **)ppD3Dtex8 + 68), extUnlockRect8, (void **)&pUnlockRect8, "UnlockRect(D8)");
}

void HookD3DTexture9(void** ppD3Dtex9)
{
	OutTraceD3D("Device hook for IDirect3DTexture9 interface\n");
	SetHook((void *)(**(DWORD **)ppD3Dtex9 + 76), extLockRect9, (void **)&pLockRect9, "LockRect(T9)");
	SetHook((void *)(**(DWORD **)ppD3Dtex9 + 80), extUnlockRect9, (void **)&pUnlockRect9, "UnlockRect(T9)");
}

void HookDirect3D8(void *lpd3d)
{
	SetHook((void *)(*(DWORD *)lpd3d +  0), extQueryInterfaceD3D8, (void **)&pQueryInterfaceD3D8, "QueryInterface(D8)");
	SetHook((void *)(*(DWORD *)lpd3d + 16), extGetAdapterCount8, (void **)&pGetAdapterCount8, "GetAdapterCount(D8)");
	SetHook((void *)(*(DWORD *)lpd3d + 20), extGetAdapterIdentifier8, (void **)&pGetAdapterIdentifier8, "GetAdapterIdentifier(D8)");
	SetHook((void *)(*(DWORD *)lpd3d + 28), extEnumAdapterModes8, (void **)&pEnumAdapterModes8, "EnumAdapterModes(D8)");
	SetHook((void *)(*(DWORD *)lpd3d + 32), extGetAdapterDisplayMode8, (void **)&pGetAdapterDisplayMode8, "GetAdapterDisplayMode(D8)");
	SetHook((void *)(*(DWORD *)lpd3d + 36), extCheckDeviceType8, (void **)&pCheckDeviceType8, "CheckDeviceType(D8)");
	SetHook((void *)(*(DWORD *)lpd3d + 52), extD3DGetDeviceCaps8, (void **)&pD3DGetDeviceCaps8, "GetDeviceCaps(D8)");
	SetHook((void *)(*(DWORD *)lpd3d + 56), extGetAdapterMonitor8, (void **)&pGetAdapterMonitor8, "GetAdapterMonitor(D8)");
	SetHook((void *)(*(DWORD *)lpd3d + 60), extCreateDevice8, (void **)&pCreateDevice8, "CreateDevice(D8)");
}

BOOL WINAPI extDisableD3DSpy(void)
{
	if(TRUE) return FALSE;
	return (*pDisableD3DSpy)();
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
	SetHook((void *)(*(DWORD *)lpd3d + 36), extCheckDeviceType9, (void **)&pCheckDeviceType9, "CheckDeviceType(D9)");
	SetHook((void *)(*(DWORD *)lpd3d + 56), extD3DGetDeviceCaps9, (void **)&pD3DGetDeviceCaps9, "GetDeviceCaps(D9)");
	SetHook((void *)(*(DWORD *)lpd3d + 60), extGetAdapterMonitor9, (void **)&pGetAdapterMonitor9, "GetAdapterMonitor(D9)");
	SetHook((void *)(*(DWORD *)lpd3d + 64), extCreateDevice9, (void **)&pCreateDevice9, "CreateDevice(D9)");
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
		OutTraceD3D("GetAdapterCount: HIDEMULTIMONITOR count=1\n");
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
		OutTraceD3D("GetAdapterCount: HIDEMULTIMONITOR count=1\n");
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
	//if(pIdentifier){
	//	OutTraceD3D("\tDriver=%s\n", pIdentifier->Driver);
	//	OutTraceD3D("\tDescription=%s\n", pIdentifier->Description);
	//	OutTraceD3D("\tDeviceName=%s\n", pIdentifier->DeviceName);
	//	OutTraceD3D("\tVersion=%x\n", pIdentifier->DriverVersion);
	//}
	return res;
}

HRESULT WINAPI extGetAdapterIdentifier9(void *pd3dd, UINT Adapter, DWORD Flags, D3DADAPTER_IDENTIFIER9 *pIdentifier)
{
	HRESULT res;
	OutTraceD3D("GetAdapterIdentifier(9): Adapter=%d flags=%x\n", Adapter, Flags);
	res=pGetAdapterIdentifier9(pd3dd, Adapter, Flags, pIdentifier);
	OutTraceD3D("GetAdapterIdentifier(9): ret=%x\n", res);
	if(pIdentifier){
		OutTraceD3D("\tDriver=%s\n", pIdentifier->Driver);
		OutTraceD3D("\tDescription=%s\n", pIdentifier->Description);
		OutTraceD3D("\tDeviceName=%s\n", pIdentifier->DeviceName);
		OutTraceD3D("\tVersion=%x\n", pIdentifier->DriverVersion);
		OutTraceD3D("\tVendorId=%x\n", pIdentifier->VendorId);
		OutTraceD3D("\tDeviceId=%x\n", pIdentifier->DeviceId);
		OutTraceD3D("\tSubSysId=%x\n", pIdentifier->SubSysId);
		OutTraceD3D("\tWHQLLevel=%x\n", pIdentifier->WHQLLevel);
	}
	return res;
}

static char *ExplainSwapEffect(DWORD f)
{
	char *s;
	switch(f){
		case D3DSWAPEFFECT_DISCARD: s="DISCARD"; break;
		case D3DSWAPEFFECT_FLIP:	s="FLIP"; break;
		case D3DSWAPEFFECT_COPY:	s="COPY"; break;
		case D3DSWAPEFFECT_OVERLAY: s="OVERLAY"; break;
		case D3DSWAPEFFECT_FLIPEX:	s="FLIPEX"; break;
		default:					s="unknown"; break;
	}
	return s;
}

static HRESULT WINAPI extReset(int dwD3DVersion, void *pd3dd, D3DPRESENT_PARAMETERS* pPresParam)
{
	HRESULT res;
	DWORD param[64], *tmp;
	D3DDISPLAYMODE mode;
	DWORD Windowed;
	void *pD3D;
	Reset_Type pReset;

	memcpy(param, pPresParam, (dwD3DVersion == 9)?56:52);
	dxw.SetScreenSize(param[0], param[1]);

	if(IsTraceD3D){
		tmp = param;
		DWORD SwapEffect;
		OutTrace("D3D%d::Reset\n", dwD3DVersion);
		OutTrace("    BackBufferWidth = %i\n", *(tmp ++));
		OutTrace("    BackBufferHeight = %i\n", *(tmp ++));
		OutTrace("    BackBufferFormat = %i\n", *(tmp ++));
		OutTrace("    BackBufferCount = %i\n", *(tmp ++));
		OutTrace("    MultiSampleType = %i\n", *(tmp ++));
		if(dwD3DVersion == 9) OutTrace("    MultiSampleQuality = %i\n", *(tmp ++));
		SwapEffect = *(tmp ++);
		OutTrace("    SwapEffect = 0x%x(%s)\n", SwapEffect, ExplainSwapEffect(SwapEffect));
		OutTrace("    hDeviceWindow = 0x%x\n", *(tmp ++));
		OutTrace("    Windowed = %i\n", *(tmp ++));
		OutTrace("    EnableAutoDepthStencil = %i\n", *(tmp ++));
		OutTrace("    AutoDepthStencilFormat = %i\n", *(tmp ++));
		OutTrace("    Flags = 0x%x\n", *(tmp ++));
		OutTrace("    FullScreen_RefreshRateInHz = %i\n", *(tmp ++));
		OutTrace("    PresentationInterval = 0x%x\n", *(tmp ++));
	}

	Windowed = (dwD3DVersion == 8) ? param[7] : param[8];
	pReset = (dwD3DVersion == 8) ? pReset8 : pReset9;
	if(dxw.Windowize){
		if(Windowed) {
			// do not attempt to reset when returning to windowed mode: it is useless (you are windowed already)
			// and the code below (GetDirect3D, ...) doesn't work.
			OutTraceD3D("SKIPPED!\n");
			return DD_OK;
		}

		if (dwD3DVersion == 9){
			res=(*pGetDirect3D9)(pd3dd, &pD3D);
			if(res) {
				OutTraceD3D("GetDirect3D FAILED! %x\n", res);
				return(DD_OK);
			}
			OutTraceD3D("GetDirect3D pd3d=%x\n", pD3D);
			res=(*pGetAdapterDisplayMode9)(pD3D, 0, &mode);
			if(res) {
				OutTraceD3D("GetAdapterDisplayMode FAILED! %x\n", res);
				return(DD_OK);
			}
			if(dxw.dwFlags6 & FORCESWAPEFFECT) param[6] = dxw.SwapEffect;			//Swap effect;
			dwD3DSwapEffect = param[6];
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
				OutTraceD3D("GetDirect3D FAILED! %x\n", res);
				return(DD_OK);
			}
			OutTraceD3D("GetDirect3D pd3d=%x\n", pD3D);
			res=(*pGetAdapterDisplayMode8)(pD3D, 0, &mode);
			if(res) {
				OutTraceD3D("GetAdapterDisplayMode FAILED! %x\n", res);
				return(DD_OK);
			}
			if(dxw.dwFlags6 & FORCESWAPEFFECT) param[5] = dxw.SwapEffect;			//Swap effect;
			dwD3DSwapEffect = param[5];
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
		if(res){
			OutTraceD3D("switching to mode=%x\n", mode.Format);
			param[2] = mode.Format; // first attempt: current screen mode
			res = (*pReset)(pd3dd, (D3DPRESENT_PARAMETERS *)param);
		}
		if(res){
			OutTraceD3D("switching to mode=D3DFMT_UNKNOWN\n");
			param[2] = D3DFMT_UNKNOWN; // second attempt: unknown, good for windowed mode
			res = (*pReset)(pd3dd, (D3DPRESENT_PARAMETERS *)param);
		}
		if(res == D3DERR_INVALIDCALL){
			OutTraceD3D("FAILED! D3DERR_INVALIDCALL\n", res);
			return D3DERR_INVALIDCALL;
		}
		if(res){
			OutTraceD3D("FAILED! %x\n", res);
			return res;
		}
		OutTraceD3D("SUCCESS!\n");
		(dwD3DVersion == 8) ? HookD3DDevice8(&pd3dd) : HookD3DDevice9(&pd3dd);
	}
	else{
		OutTraceD3D("SKIPPED!\n");
		res=D3D_OK;
	}

	dxw.SetScreenSize(pPresParam->BackBufferWidth, pPresParam->BackBufferHeight);
	dxw.SetVSyncDelays(mode.RefreshRate);

	GetHookInfo()->IsFullScreen = dxw.IsFullScreen();
	GetHookInfo()->DXVersion=(short)dwD3DVersion;
	GetHookInfo()->Height=(short)dxw.GetScreenHeight();
	GetHookInfo()->Width=(short)dxw.GetScreenWidth();
	GetHookInfo()->ColorDepth=(short)dxw.VirtualPixelFormat.dwRGBBitCount;
	
	return D3D_OK;
}

HRESULT WINAPI extReset8(void *pd3dd, D3DPRESENT_PARAMETERS* pPresParam)
{ return extReset(8, pd3dd, pPresParam); }
HRESULT WINAPI extReset9(void *pd3dd, D3DPRESENT_PARAMETERS* pPresParam)
{ return extReset(9, pd3dd, pPresParam); }

static HRESULT WINAPI extPresent(Present_Type pPresent, void *pd3dd, CONST RECT *pSourceRect, CONST RECT *pDestRect, HWND hDestWindowOverride, CONST RGNDATA *pDirtyRegion)
{
	HRESULT res;
	RECT RemappedSrcRect, RemappedDstRect;
	if(IsDebug){
		char sSourceRect[81];
		char sDestRect[81];
		if (pSourceRect) sprintf_s(sSourceRect, 80, "(%d,%d)-(%d,%d)", pSourceRect->left, pSourceRect->top, pSourceRect->right, pSourceRect->bottom);
		else strcpy(sSourceRect, "(NULL)");
		if (pDestRect) sprintf_s(sDestRect, 80, "(%d,%d)-(%d,%d)", pDestRect->left, pDestRect->top, pDestRect->right, pDestRect->bottom);
		else strcpy(sDestRect, "(NULL)");
		OutTraceB("Present: SourceRect=%s DestRect=%s hDestWndOverride=%x\n", sSourceRect, sDestRect, hDestWindowOverride);
	}

	// frame counter handling....
	if (dxw.HandleFPS()) return D3D_OK;
	if (dxw.dwFlags1 & SAVELOAD) dxw.VSyncWait();

	if(dxw.dwFlags2 & FULLRECTBLT) pSourceRect = pDestRect = NULL;
	if(dxw.Windowize){
		// v2.03.15 - fix target RECT region
		if ((dwD3DSwapEffect == D3DSWAPEFFECT_COPY) && (dxw.dwFlags2 & KEEPASPECTRATIO)) {
			RemappedDstRect=dxw.MapClientRect((LPRECT)pDestRect);
			pDestRect = &RemappedDstRect;
			OutTraceB("Present: FIXED DestRect=(%d,%d)-(%d,%d)\n", RemappedDstRect.left, RemappedDstRect.top, RemappedDstRect.right, RemappedDstRect.bottom);
		}
		// in case of NOD3DRESET, remap source rect. Unfortunately, this doesn't work in fullscreen mode ....
		if((dxw.dwFlags4 & NOD3DRESET) && (pSourceRect == NULL)){ 
			RemappedSrcRect = dxw.GetScreenRect();
			pSourceRect = &RemappedSrcRect;
			OutTraceB("Present: NOD3DRESET FIXED SourceRect=(%d,%d)-(%d,%d)\n", RemappedSrcRect.left, RemappedSrcRect.top, RemappedSrcRect.right, RemappedSrcRect.bottom);
		}
	}

	res=(*pPresent)(pd3dd, pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion);
	if(res) OutTraceE("Present: err=%x(%s)\n", res, ExplainDDError(res));
	dxw.ShowOverlay();
	return res;
}

HRESULT WINAPI extPresent8(void *pd3dd, CONST RECT *pSourceRect, CONST RECT *pDestRect, HWND hDestWindowOverride, CONST RGNDATA *pDirtyRegion)
{ return extPresent(pPresent8, pd3dd, pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion); }
HRESULT WINAPI extPresent9(void *pd3dd, CONST RECT *pSourceRect, CONST RECT *pDestRect, HWND hDestWindowOverride, CONST RGNDATA *pDirtyRegion)
{ return extPresent(pPresent9, pd3dd, pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion); }

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
	OutTraceD3D("GetDisplayMode(8): fixed size=(%dx%d)\n", pMode->Width, pMode->Height);
	dxw.SetVSyncDelays(pMode->RefreshRate);
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
	OutTraceD3D("GetDisplayMode(9): fixed size=(%dx%d)\n", pMode->Width, pMode->Height);
	dxw.SetVSyncDelays(pMode->RefreshRate);
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
	OutTraceD3D("GetAdapterDisplayMode(8): fixed size=(%dx%d)\n", pMode->Width, pMode->Height);
	dxw.SetVSyncDelays(pMode->RefreshRate);
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
	OutTraceD3D("GetAdapterDisplayMode(9): fixed size=(%dx%d)\n", pMode->Width, pMode->Height);
	dxw.SetVSyncDelays(pMode->RefreshRate);
	return res;
}

HWND FixD3DWindowFrame(HWND hFocusWin)
{
	extern void HookWindowProc(HWND);
	HWND hRetWnd = hFocusWin;

	if(!(dxw.dwFlags3 & NOWINDOWMOVE)) 
		AdjustWindowFrame(dxw.GethWnd(), dxw.GetScreenWidth(), dxw.GetScreenHeight());

	if(dxw.dwFlags3 & FIXD3DFRAME){
		char ClassName[81];
		RECT workarea;
		HWND hChildWin;
		GetClassName(dxw.GethWnd(), ClassName, 80);
		(*pGetClientRect)(dxw.GethWnd(), &workarea);
		if (dxw.dwFlags2 & KEEPASPECTRATIO) dxw.FixWorkarea(&workarea);
		hChildWin=(*pCreateWindowExA)(
			0, ClassName, "child", 
			WS_CHILD|WS_VISIBLE, 
			//GetSystemMetrics(SM_CXSIZEFRAME), GetSystemMetrics(SM_CYSIZEFRAME)+GetSystemMetrics(SM_CYCAPTION), 
			workarea.left, workarea.top, workarea.right-workarea.left, workarea.bottom-workarea.top, 
			//workarea.left+20, workarea.top+20, workarea.right-workarea.left-40, workarea.bottom-workarea.top-40, 
			dxw.GethWnd(), NULL, NULL, NULL);
		if (hChildWin) 
			OutTraceD3D("CreateDevice: ChildWin=%x pos=(%d,%d) size=(%d,%d)\n", 
				hChildWin, workarea.left, workarea.top, workarea.right-workarea.left, workarea.bottom-workarea.top);
		else
			OutTraceE("CreateDevice: CreateWindowEx ERROR err=%d\n", GetLastError());
		dxw.SethWnd(hChildWin, dxw.GethWnd());
		HookWindowProc(hChildWin);
		hRetWnd = hChildWin;
	}

	return hRetWnd;
}

static HRESULT WINAPI extCreateDevice(void *lpd3d, UINT adapter, D3DDEVTYPE devicetype,
	HWND hfocuswindow, DWORD behaviorflags, D3DPRESENT_PARAMETERS *ppresentparam, void **ppd3dd, int dwD3DVersion)
{
	HRESULT res;
	DWORD param[64], *tmp;
	D3DDISPLAYMODE mode;

	OutTraceD3D("CreateDevice(%d): lpd3d=%x adapter=%x hFocusWnd=%x behavior=%x, size=(%d,%d)\n",
		dwD3DVersion, lpd3d, adapter, hfocuswindow, behaviorflags, ppresentparam->BackBufferWidth, ppresentparam->BackBufferHeight);

	memcpy(param, ppresentparam, (dwD3DVersion == 9)?56:52);
	dxw.SethWnd(hfocuswindow);
	dxw.SetScreenSize(param[0], param[1]);

	if(dxw.Windowize) hfocuswindow=FixD3DWindowFrame(hfocuswindow);

	if(IsTraceD3D){
		tmp = param;
		DWORD SwapEffect;
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
		SwapEffect = *(tmp ++);
		OutTrace("    SwapEffect = 0x%x(%s)\n", SwapEffect, ExplainSwapEffect(SwapEffect));
		OutTrace("    hDeviceWindow = 0x%x\n", *(tmp ++));
		OutTrace("    Windowed = %i\n", *(tmp ++));
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
	OutTraceD3D("    Current Format = 0x%x\n", mode.Format);
	OutTraceD3D("    Current ScreenSize = (%dx%d)\n", mode.Width, mode.Height);
	OutTraceD3D("    Current Refresh Rate = %d\n", mode.RefreshRate);

	if((dxw.dwFlags4 & NOD3DRESET) && dxw.Windowize){ 
		RECT desktop;
		// Get a handle to the desktop window
		const HWND hDesktop = (*pGetDesktopWindow)();
		// Get the size of screen to the variable desktop
		(*pGetWindowRect)(hDesktop, &desktop);
		// The top left corner will have coordinates (0,0)
		// and the bottom right corner will have coordinates
		// (horizontal, vertical)
		param[0] = desktop.right;
		param[1] = desktop.bottom;			
	}

	if(dwD3DVersion == 9){
		if(dxw.Windowize){
			if(dxw.dwFlags6 & FORCESWAPEFFECT) param[6] = dxw.SwapEffect;			//Swap effect;
			dwD3DSwapEffect = param[6];
			param[7] = 0;			//hDeviceWindow
			dxw.SetFullScreen(~param[8]?TRUE:FALSE); 
			param[8] = 1;			//Windowed
			//param[11] = D3DPRESENTFLAG_DEVICECLIP;			//Flags;
			param[12] = 0;			//FullScreen_RefreshRateInHz;
			param[13] = D3DPRESENT_INTERVAL_DEFAULT;	//PresentationInterval
		}
		res = (*pCreateDevice9)(lpd3d, 0, devicetype, hfocuswindow, behaviorflags, param, ppd3dd);
		if(res){
			OutTraceD3D("switching to mode=%x\n", mode.Format);
			param[2] = mode.Format; // first attempt: current screen mode
			res = (*pCreateDevice9)(lpd3d, 0, devicetype, hfocuswindow, behaviorflags, param, ppd3dd);
		}
		if(res){
			OutTraceD3D("switching to mode=D3DFMT_UNKNOWN\n");
			param[2] = D3DFMT_UNKNOWN; // second attempt: unknown, good for windowed mode
			res = (*pCreateDevice9)(lpd3d, 0, devicetype, hfocuswindow, behaviorflags, param, ppd3dd);
		}
	}
	else{
		if(dxw.Windowize){
			if(dxw.dwFlags6 & FORCESWAPEFFECT) param[5] = dxw.SwapEffect;			//Swap effect;
			dwD3DSwapEffect = param[5];
			param[6] = 0;			//hDeviceWindow
			dxw.SetFullScreen(~param[7]?TRUE:FALSE); 
			param[7] = 1;			//Windowed
			//param[10] = D3DPRESENTFLAG_DEVICECLIP;			//Flags;
			param[11] = 0;			//FullScreen_RefreshRateInHz;
			param[12] = D3DPRESENT_INTERVAL_DEFAULT;	//PresentationInterval
		}
		res = (*pCreateDevice8)(lpd3d, 0, devicetype, hfocuswindow, behaviorflags, param, ppd3dd);
		if(res){
			OutTraceD3D("switching to mode=%x\n", mode.Format);
			param[2] = mode.Format; // first attempt: current screen mode
			res = (*pCreateDevice8)(lpd3d, 0, devicetype, hfocuswindow, behaviorflags, param, ppd3dd);
		}
		if(res){
			OutTraceD3D("switching to mode=D3DFMT_UNKNOWN\n");
			param[2] = D3DFMT_UNKNOWN; // second attempt: unknown, good for windowed mode
			res = (*pCreateDevice8)(lpd3d, 0, devicetype, hfocuswindow, behaviorflags, param, ppd3dd);
		}
	}

	if(res){
		OutTraceD3D("FAILED! %x\n", res);
		return res;
	}
	OutTraceD3D("SUCCESS! device=%x\n", *ppd3dd);
	lpD3DActiveDevice = *ppd3dd;

	if(dwD3DVersion == 8){ 
		HookD3DDevice8(ppd3dd);
	}
	else {
		HookD3DDevice9(ppd3dd);
	}

	dxw.SetVSyncDelays(mode.RefreshRate);

	GetHookInfo()->IsFullScreen = dxw.IsFullScreen();
	GetHookInfo()->DXVersion=(short)dwD3DVersion;
	GetHookInfo()->Height=(short)dxw.GetScreenHeight();
	GetHookInfo()->Width=(short)dxw.GetScreenWidth();
	GetHookInfo()->ColorDepth=(short)dxw.VirtualPixelFormat.dwRGBBitCount;
	return DD_OK;
}

HRESULT WINAPI extCreateDevice8(void *lpd3d, UINT adapter, D3DDEVTYPE dt, HWND hfw, DWORD bf, D3DPRESENT_PARAMETERS *ppp, void **ppd3dd)
{ return extCreateDevice(lpd3d, adapter, dt, hfw, bf, ppp, ppd3dd, 8); }
HRESULT WINAPI extCreateDevice9(void *lpd3d, UINT adapter, D3DDEVTYPE dt, HWND hfw, DWORD bf, D3DPRESENT_PARAMETERS *ppp, void **ppd3dd)
{ return extCreateDevice(lpd3d, adapter, dt, hfw, bf, ppp, ppd3dd, 9); }


HRESULT WINAPI extCreateDeviceEx(void *lpd3d, UINT adapter, D3DDEVTYPE devicetype,
	HWND hfocuswindow, DWORD behaviorflags, D3DPRESENT_PARAMETERS *ppresentparam, D3DDISPLAYMODEEX *pFullscreenDisplayMode, void **ppd3dd)
{
	HRESULT res;
	DWORD param[64], *tmp;
	D3DDISPLAYMODE mode;

	OutTraceD3D("CreateDeviceEx: D3DVersion=%d lpd3d=%x adapter=%x hFocusWnd=%x behavior=%x, size=(%d,%d)\n",
		dwD3DVersion, lpd3d, adapter, hfocuswindow, behaviorflags, ppresentparam->BackBufferWidth, ppresentparam->BackBufferHeight);

	memcpy(param, ppresentparam, 56);
	dxw.SethWnd(hfocuswindow);
	dxw.SetScreenSize(param[0], param[1]);

	if(dxw.Windowize) hfocuswindow=FixD3DWindowFrame(hfocuswindow);

	if(IsTraceD3D){
	    tmp = param;
		DWORD SwapEffect;
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
		SwapEffect = *(tmp ++);
		OutTrace("    SwapEffect = 0x%x(%s)\n", SwapEffect, ExplainSwapEffect(SwapEffect));
		OutTrace("    hDeviceWindow = 0x%x\n", *(tmp ++));
		OutTrace("    Windowed = %i\n", *(tmp ++));
		OutTrace("    EnableAutoDepthStencil = %i\n", *(tmp ++));
		OutTrace("    AutoDepthStencilFormat = %i\n", *(tmp ++));
		OutTrace("    Flags = 0x%x\n", *(tmp ++));
		OutTrace("    FullScreen_RefreshRateInHz = %i\n", *(tmp ++));
		OutTrace("    PresentationInterval = 0x%x\n", *(tmp ++));
	}

	//((LPDIRECT3D9)lpd3d)->GetAdapterDisplayMode(0, &mode);
	(*pGetAdapterDisplayMode9)(lpd3d, 0, &mode);
	OutTraceD3D("    Current Format = 0x%x\n", mode.Format);
	OutTraceD3D("    Current ScreenSize = (%dx%d)\n", mode.Width, mode.Height);
	OutTraceD3D("    Current Refresh Rate = %d\n", mode.RefreshRate);

	if((dxw.dwFlags4 & NOD3DRESET) && dxw.Windowize){ 
		RECT desktop;
		// Get a handle to the desktop window
		const HWND hDesktop = (*pGetDesktopWindow)();
		// Get the size of screen to the variable desktop
		(*pGetWindowRect)(hDesktop, &desktop);
		// The top left corner will have coordinates (0,0)
		// and the bottom right corner will have coordinates
		// (horizontal, vertical)
		param[0] = desktop.right;
		param[1] = desktop.bottom;			
	}

	if(dxw.Windowize){
		if(dxw.dwFlags6 & FORCESWAPEFFECT) param[6] = dxw.SwapEffect;			//Swap effect;
		dwD3DSwapEffect = param[6];
		//param[7] = 0;			//hDeviceWindow
		param[7] = (DWORD)dxw.GethWnd();			//hDeviceWindow
		dxw.SetFullScreen(~param[8]?TRUE:FALSE); 
		param[8] = 1;			//Windowed
		param[12] = 0;			//FullScreen_RefreshRateInHz;
		param[13] = D3DPRESENT_INTERVAL_DEFAULT;	//PresentationInterval
	}

	res = (*pCreateDeviceEx)(lpd3d, 0, devicetype, hfocuswindow, behaviorflags, param, pFullscreenDisplayMode, ppd3dd);
	if(res){
		OutTraceD3D("switching to mode=%x\n", mode.Format);
		param[2] = mode.Format; // first attempt: current screen mode
		res = (*pCreateDeviceEx)(lpd3d, 0, devicetype, hfocuswindow, behaviorflags, param, pFullscreenDisplayMode, ppd3dd);
	}
	if(res){
		OutTraceD3D("switching to mode=D3DFMT_UNKNOWN\n");
		param[2] = D3DFMT_UNKNOWN; // second attempt: unknown, good for windowed mode
		res = (*pCreateDeviceEx)(lpd3d, 0, devicetype, hfocuswindow, behaviorflags, param, pFullscreenDisplayMode, ppd3dd);
	}
	if(res){
		OutTraceD3D("FAILED! %x\n", res);
		return res;
	}
	OutTraceD3D("SUCCESS!\n"); 
	lpD3DActiveDevice = *ppd3dd;

	HookD3DDevice9(ppd3dd);

	dxw.SetVSyncDelays(mode.RefreshRate);

	GetHookInfo()->IsFullScreen = dxw.IsFullScreen();
	GetHookInfo()->DXVersion=(short)dwD3DVersion;
	GetHookInfo()->Height=(short)dxw.GetScreenHeight();
	GetHookInfo()->Width=(short)dxw.GetScreenWidth();
	GetHookInfo()->ColorDepth=(short)dxw.VirtualPixelFormat.dwRGBBitCount;
	
	return DD_OK;
}

extern char *ExplainRenderstateValue(DWORD Value);

static HRESULT WINAPI extSetRenderState(SetRenderState_Type pSetRenderState, void *pd3dd, D3DRENDERSTATETYPE State, DWORD Value) 
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

HRESULT WINAPI extSetRenderState8(void *pd3dd, D3DRENDERSTATETYPE State, DWORD Value) 
{ return extSetRenderState(pSetRenderState8, pd3dd, State, Value); }
HRESULT WINAPI extSetRenderState9(void *pd3dd, D3DRENDERSTATETYPE State, DWORD Value) 
{ return extSetRenderState(pSetRenderState9, pd3dd, State, Value); }

static HRESULT WINAPI extGetRenderState(GetRenderState_Type pGetRenderState, void *pd3dd, D3DRENDERSTATETYPE State, DWORD Value) 
{
	return (*pGetRenderState)(pd3dd, State, Value);
}

HRESULT WINAPI extGetRenderState8(void *pd3dd, D3DRENDERSTATETYPE State, DWORD Value) 
{ return extGetRenderState(pSetRenderState8, pd3dd, State, Value); }
HRESULT WINAPI extGetRenderState9(void *pd3dd, D3DRENDERSTATETYPE State, DWORD Value) 
{ return extGetRenderState(pSetRenderState9, pd3dd, State, Value); }

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
	OutTraceD3D("SetViewport: remapped pos=(%d,%d) size=(%d,%d)\n", 
		pViewport->X, pViewport->Y, pViewport->Width, pViewport->Height);
	return (*pSetViewport)(pd3dd, pViewport);
}

static HRESULT WINAPI extCreateAdditionalSwapChain(int dwD3DVersion, void *lpd3dd, D3DPRESENT_PARAMETERS *pPresentationParameters, IDirect3DSwapChain9 **ppSwapChain)
{
	HRESULT res, res2;
	DWORD param[64], *tmp;
	D3DDISPLAYMODE mode;
	CreateAdditionalSwapChain_Type pCreateAdditionalSwapChain;
	Reset_Type pReset;
	GetAdapterDisplayMode_Type pGetAdapterDisplayMode;

	OutTraceD3D("CreateAdditionalSwapChain(%d): d3d=%x\n", dwD3DVersion, lpd3dd);

	pCreateAdditionalSwapChain = (dwD3DVersion == 8) ? pCreateAdditionalSwapChain8 : pCreateAdditionalSwapChain9;
	pReset = (dwD3DVersion == 8) ? pReset8 : pReset9;
	pGetAdapterDisplayMode = (dwD3DVersion == 8) ? pGetAdapterDisplayMode8 : pGetAdapterDisplayMode9;

	memcpy(param, pPresentationParameters, (dwD3DVersion == 9)?56:52);
	dxw.SetScreenSize(param[0], param[1]);
	if(dxw.Windowize) FixD3DWindowFrame(dxw.GethWnd());

	if(IsTraceD3D){
	    tmp = param;
		DWORD SwapEffect;
		OutTrace("D3D%d::CreateAdditionalSwapChain\n", dwD3DVersion);
		OutTrace("    BackBufferWidth = %i\n", *(tmp ++));
		OutTrace("    BackBufferHeight = %i\n", *(tmp ++));
		OutTrace("    BackBufferFormat = %i\n", *(tmp ++));
		OutTrace("    BackBufferCount = %i\n", *(tmp ++));
		OutTrace("    MultiSampleType = %i\n", *(tmp ++));
		if(dwD3DVersion == 9) OutTrace("    MultiSampleQuality = %i\n", *(tmp ++));
		SwapEffect = *(tmp ++);
		OutTrace("    SwapEffect = 0x%x(%s)\n", SwapEffect, ExplainSwapEffect(SwapEffect));
		OutTrace("    hDeviceWindow = 0x%x\n", *(tmp ++));
		OutTrace("    Windowed = %i\n", *(tmp ++));
		OutTrace("    EnableAutoDepthStencil = %i\n", *(tmp ++));
		OutTrace("    AutoDepthStencilFormat = %i\n", *(tmp ++));
		OutTrace("    Flags = 0x%x\n", *(tmp ++));
		OutTrace("    FullScreen_RefreshRateInHz = %i\n", *(tmp ++));
		OutTrace("    PresentationInterval = 0x%x\n", *(tmp ++));
	}

	res2=(*pGetAdapterDisplayMode)(lpd3dd, D3DADAPTER_DEFAULT, &mode);

	if(res2==DD_OK){
	OutTraceD3D("    Current Format = 0x%x\n", mode.Format);
	OutTraceD3D("    Current ScreenSize = (%dx%d)\n", mode.Width, mode.Height);
	OutTraceD3D("    Current Refresh Rate = %d\n", mode.RefreshRate);
	}
	else
		OutTraceE("CreateAdditionalSwapChain: GetAdapterDisplayMode err=%x(%s)\n", res2, ExplainDDError(res2));

	if(dxw.Windowize){
		if(dwD3DVersion == 9){
			if(dxw.dwFlags6 & FORCESWAPEFFECT) param[6] = dxw.SwapEffect;			//Swap effect;
			dwD3DSwapEffect = param[6];
			param[7] = 0;			//hDeviceWindow
			dxw.SetFullScreen(~param[8]?TRUE:FALSE); 
			param[8] = 1;			//Windowed
			param[12] = 0;			//FullScreen_RefreshRateInHz;
			param[13] = D3DPRESENT_INTERVAL_DEFAULT;	//PresentationInterval
		}
		else{
			if(dxw.dwFlags6 & FORCESWAPEFFECT) param[5] = dxw.SwapEffect;			//Swap effect;
			dwD3DSwapEffect = param[5];
			param[6] = 0;			//hDeviceWindow
			dxw.SetFullScreen(~param[7]?TRUE:FALSE); 
			param[7] = 1;			//Windowed
			param[11] = 0;			//FullScreen_RefreshRateInHz;
			param[12] = D3DPRESENT_INTERVAL_DEFAULT;	//PresentationInterval
		}
	}

	if(res2 == D3DERR_DEVICELOST){
		res2 = (*pReset)(lpd3dd, (D3DPRESENT_PARAMETERS *)param);
		if(res2 != DD_OK) OutTraceE("CreateAdditionalSwapChain: Reset err=%x(%s)\n", res2, ExplainDDError(res2));
	}

	res=(*pCreateAdditionalSwapChain)(lpd3dd, (D3DPRESENT_PARAMETERS *)param, ppSwapChain);
	if(res && (res2==DD_OK)){
		OutTraceD3D("switching to mode=%x\n", mode.Format);
		param[2] = mode.Format; // first attempt: current screen mode
		res=(*pCreateAdditionalSwapChain)(lpd3dd, (D3DPRESENT_PARAMETERS *)param, ppSwapChain);
	}
	if(res){
		OutTraceD3D("switching to mode=D3DFMT_UNKNOWN\n");
		param[2] = D3DFMT_UNKNOWN; // second attempt: unknown, good for windowed mode
		res=(*pCreateAdditionalSwapChain)(lpd3dd, (D3DPRESENT_PARAMETERS *)param, ppSwapChain);
	}
	//if(res){
	//	OutTraceD3D("switching to size=0,0\n");
	//	param[0] = param[1] = 0; // third attempt: unknown, good for windowed mode
	//	res=(*pCreateAdditionalSwapChain)(lpd3dd, (D3DPRESENT_PARAMETERS *)param, ppSwapChain);
	//}
	if(res) {
		OutTraceE("CreateAdditionalSwapChain ERROR: res=%x(%s)\n", res, ExplainDDError(res));
	}

	(dwD3DVersion == 9) ? HookD3DDevice9(&lpd3dd) : HookD3DDevice8(&lpd3dd);
	dxw.SetVSyncDelays(mode.RefreshRate);
	return res;
}

HRESULT WINAPI extCreateAdditionalSwapChain8(void *lpd3dd, D3DPRESENT_PARAMETERS *pPresentationParameters, IDirect3DSwapChain9 **ppSwapChain)
{ return extCreateAdditionalSwapChain(8, lpd3dd, pPresentationParameters, ppSwapChain); }
HRESULT WINAPI extCreateAdditionalSwapChain9(void *lpd3dd, D3DPRESENT_PARAMETERS *pPresentationParameters, IDirect3DSwapChain9 **ppSwapChain)
{ return extCreateAdditionalSwapChain(9, lpd3dd, pPresentationParameters, ppSwapChain); }

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
	//D3D10_VIEWPORT ViewPort;
	OutTraceD3D("D3D10CreateDevice: DriverType=%x Flags=%x SDKVersion=%x\n", DriverType, Flags, SDKVersion);
	//return 0x887a0004;
	res=(*pD3D10CreateDevice)(pAdapter, DriverType, Software, Flags, SDKVersion, ppDevice);
	if(res){
		OutTraceE("D3D10CreateDevice: ret=%x(%s)\n", res, ExplainDDError(res));
		return res;
	}
	// useless ....
	//SetHook((void *)(*(DWORD *)*ppDevice + 120), extRSSetViewports10, (void **)&pRSSetViewports10, "RSSetViewports(D10)");

	// useless ....
	//if(dxw.Windowize){
	//	ViewPort.TopLeftX=dxw.iPosX;
	//	ViewPort.TopLeftY=dxw.iPosY;
	//	ViewPort.Width=dxw.iSizX;
	//	ViewPort.Height=dxw.iSizY;
	//	ViewPort.MinDepth=1.0;
	//	ViewPort.MaxDepth=1.0;
	//	(*pRSSetViewports10)((void *)*ppDevice, 1, (D3D10_VIEWPORT *)&ViewPort);
	//}

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
	SetHook((void *)(*(DWORD *)*ppDevice + 100), extRSSetViewports10, (void **)&pRSSetViewports10, "RSSetViewports(D10)");
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
	SetHook((void *)(*(DWORD *)ppImmediateContext + 148), extRSSetViewports11, (void **)&pRSSetViewports11, "RSSetViewports(D11)");
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

void WINAPI extRSSetViewports10(void *This, UINT NumViewports, D3D10_VIEWPORT *pViewports)
{
	OutTraceD3D("RSSetViewports: NumViewports=%d\n", NumViewports);

	if(dxw.Windowize && (NumViewports==1)){
		pViewports->TopLeftX=dxw.iPosX;
		pViewports->TopLeftY=dxw.iPosY;
		pViewports->Width=dxw.iSizX;
		pViewports->Height=dxw.iSizY;
	}

	(*pRSSetViewports10)(This, NumViewports, pViewports);
}

void WINAPI extRSSetViewports11(ID3D11DeviceContext *This, UINT NumViewports, D3D11_VIEWPORT *pViewports)
{
	OutTraceD3D("RSSetViewports: NumViewports=%d\n", NumViewports);

	if(dxw.Windowize && (NumViewports==1)){
		pViewports->TopLeftX=dxw.iPosX;
		pViewports->TopLeftY=dxw.iPosY;
		pViewports->Width=dxw.iSizX;
		pViewports->Height=dxw.iSizY;
	}

	(*pRSSetViewports11)(This, NumViewports, pViewports);
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
		OutTraceD3D("CheckFullScreen: return OK\n");
		res=0;
	}
	else{
		res=(*pCheckFullScreen)();
		OutTraceD3D("CheckFullScreen: return %x\n", res);
	}
	return res;
}

static void WINAPI extSetGammaRamp(SetGammaRamp_Type pSetGammaRamp, void *lpdd3dd, UINT iSwapChain, DWORD Flags, D3DGAMMARAMP *pRamp)
{
	OutTraceD3D("SetGammaRamp: dd3dd=%x SwapChain=%d flags=%x\n", lpdd3dd, iSwapChain, Flags);
	if (dxw.dwFlags2 & DISABLEGAMMARAMP) return;
	(*pSetGammaRamp)(lpdd3dd, iSwapChain, Flags, pRamp);
}

void WINAPI extSetGammaRamp8(void *lpdd3dd, UINT iSwapChain, DWORD Flags, D3DGAMMARAMP *pRamp)
{ return extSetGammaRamp(pSetGammaRamp8, lpdd3dd, iSwapChain, Flags, pRamp); }
void WINAPI extSetGammaRamp9(void *lpdd3dd, UINT iSwapChain, DWORD Flags, D3DGAMMARAMP *pRamp)
{ return extSetGammaRamp(pSetGammaRamp9, lpdd3dd, iSwapChain, Flags, pRamp); }

static void WINAPI extGetGammaRamp(GetGammaRamp_Type pGetGammaRamp, void *lpdd3dd, UINT iSwapChain, D3DGAMMARAMP *pRamp)
{
	OutTraceD3D("GetGammaRamp: dd3dd=%x SwapChain=%d\n", lpdd3dd, iSwapChain);
	(*pGetGammaRamp)(lpdd3dd, iSwapChain, pRamp);
}

void WINAPI extGetGammaRamp8(void *lpdd3dd, UINT iSwapChain, D3DGAMMARAMP *pRamp)
{ return extGetGammaRamp(pGetGammaRamp8, lpdd3dd, iSwapChain, pRamp); }
void WINAPI extGetGammaRamp9(void *lpdd3dd, UINT iSwapChain, D3DGAMMARAMP *pRamp)
{ return extGetGammaRamp(pGetGammaRamp9, lpdd3dd, iSwapChain, pRamp); }

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
	OutTraceB("Device::BeginScene(8): d3dd=%x\n", lpdd3dd);
	res=(*pBeginScene8)(lpdd3dd);
	if (res) OutTraceE("Device::BeginScene(8) ERROR: err=%x\n", res);
	return res;
}

ULONG WINAPI extBeginScene9(void *lpdd3dd)
{
	ULONG res;
	OutTraceB("Device::BeginScene(9): d3dd=%x\n", lpdd3dd);
	res=(*pBeginScene9)(lpdd3dd);
	if (res) OutTraceE("Device::BeginScene(9) ERROR: err=%x\n", res);
	return res;
}

ULONG WINAPI extEndScene8(void *lpdd3dd)
{
	ULONG res;
	OutTraceB("Device::EndScene(8): d3dd=%x\n", lpdd3dd);
	res=(*pEndScene8)(lpdd3dd);
	if (res) OutTraceE("Device::EndScene(8) ERROR: err=%x\n", res);
	return res;
}

ULONG WINAPI extEndScene9(void *lpdd3dd)
{
	ULONG res;
	OutTraceB("Device::EndScene(9): d3dd=%x\n", lpdd3dd);
	res=(*pEndScene9)(lpdd3dd);
	if (res) OutTraceE("Device::EndScene(9) ERROR: err=%x\n", res);
	return res;
}

ULONG WINAPI extSetTexture8(void *lpd3dd, DWORD Stage, void* pTexture)
{
	ULONG ret;
	if (dxw.dwFlags4 & NOTEXTURES) {
		(*pSetTexture8)(lpd3dd, Stage, NULL);
		OutTraceD3D("Device::SetTexture(8): d3dd=%x stage=%x\n", lpd3dd, Stage);
		return DD_OK;
	}
	ret = (*pSetTexture8)(lpd3dd, Stage, pTexture);
	OutTraceD3D("Device::SetTexture(8): d3dd=%x stage=%x texture=%x ret=%x\n", lpd3dd, Stage, pTexture, ret);
	if(pTexture) D3D8TextureHandling((void *)pTexture, Stage);
	return ret;
}

ULONG WINAPI extSetTexture9(void *lpd3dd, DWORD Stage, void* pTexture)
{
	ULONG ret;
	if (dxw.dwFlags4 & NOTEXTURES) {
		(*pSetTexture9)(lpd3dd, Stage, NULL);
		OutTraceD3D("Device::SetTexture(9): d3dd=%x stage=%x\n", lpd3dd, Stage);
		return DD_OK;
	}
	ret = (*pSetTexture9)(lpd3dd, Stage, pTexture);
	OutTraceD3D("Device::SetTexture(9): d3dd=%x stage=%x texture=%x ret=%x\n", lpd3dd, Stage, pTexture, ret);
	if(pTexture) D3D9TextureHandling((void *)pTexture, (int)Stage);
	return ret;
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

void WINAPI voidDebugSetLevel(void) 
{
	OutTraceD3D("DebugSetLevel: SUPPRESS\n");
}

void WINAPI voidDebugSetMute(void) 
{
	OutTraceD3D("DebugSetMute: SUPPRESS\n");
}

BOOL  WINAPI voidDisableD3DSpy(void)
{
	OutTraceD3D("DisableD3DSpy: SUPPRESS\n");
	return FALSE;
}

HRESULT WINAPI voidDirect3DShaderValidatorCreate9(void) 
{
	OutTraceD3D("Direct3DShaderValidatorCreate9: SUPPRESS\n");
	return 0;
}

static HRESULT WINAPI extCheckDeviceType(CheckDeviceType_Type pCheckDeviceType, void *lpd3d, UINT Adapter, D3DDEVTYPE DevType, D3DFORMAT AdapterFormat, D3DFORMAT BackBufferFormat, BOOL bWindowed)
{
	HRESULT res;
	OutTraceD3D("CheckDeviceType: d3d=%x adapter=%d windowed=%x\n", lpd3d, Adapter, bWindowed);
	if(dxw.Windowize) bWindowed = TRUE;
	res = (*pCheckDeviceType)(lpd3d, Adapter, DevType, AdapterFormat, BackBufferFormat, bWindowed);
	OutTraceD3D("CheckDeviceType: res=%x\n", res);
	return res;
}

HRESULT WINAPI extCheckDeviceType8(void *lpd3d, UINT Adapter, D3DDEVTYPE DevType, D3DFORMAT AdapterFormat, D3DFORMAT BackBufferFormat, BOOL bWindowed)
{ return extCheckDeviceType(pCheckDeviceType8, lpd3d, Adapter, DevType, AdapterFormat, BackBufferFormat, bWindowed); }
HRESULT WINAPI extCheckDeviceType9(void *lpd3d, UINT Adapter, D3DDEVTYPE DevType, D3DFORMAT AdapterFormat, D3DFORMAT BackBufferFormat, BOOL bWindowed)
{ return extCheckDeviceType(pCheckDeviceType9, lpd3d, Adapter, DevType, AdapterFormat, BackBufferFormat, bWindowed); }

static char *ExplainD3D9DeviceType(D3DDEVTYPE DeviceType)
{
	char *s;
	switch(DeviceType){
		case D3DDEVTYPE_HAL: s="HAL"; break;
		case D3DDEVTYPE_NULLREF: s="NULLREF"; break;
		case D3DDEVTYPE_REF: s="REF"; break;
		case D3DDEVTYPE_SW: s="SW"; break;
		default: s="unknown"; break;
	}
	return s;
}

HRESULT WINAPI extD3DGetDeviceCaps(void *lpd3d, UINT Adapter, D3DDEVTYPE DeviceType, D3DCAPS9* pCaps, int version)
{
	HRESULT res;
	OutTraceD3D("GetDeviceCaps(%d): d3d=%x adapter=%d devtype=%x(%s)\n", version, lpd3d, Adapter, DeviceType, ExplainD3D9DeviceType(DeviceType));
	
	if(version == 8)
		res=(*pD3DGetDeviceCaps8)(lpd3d, Adapter, DeviceType, (D3DCAPS8 *)pCaps);
	else
		res=(*pD3DGetDeviceCaps9)(lpd3d, Adapter, DeviceType, pCaps);

	if(res){
		OutTraceE("GetDeviceCaps: ERROR: err=%x\n", res);
	}
	else{
		if(IsDebug){
			OutTrace("GetDeviceCaps:\n\t"
				"DeviceType=%x(%s)\n\t"
				"Caps=%x Caps2=%x Caps3=%x\n\t"
				"PresentationIntervals=%x\n\t"
				"CursorCaps=%x DevCaps=%x\n\t"
				"PrimitiveMiscCaps=%x\n\t"
				"RasterCaps=%x\n\t"
				"ZCmpCaps=%x\n\t"
				"SrcBlendCaps=%x\n\t"
				"DestBlendCaps=%x\n\t"
				"AlphaCmpCaps=%x\n\t"
				"ShadeCaps=%x\n\t"
				"TextureCaps=%x\n\t",
				pCaps->DeviceType, ExplainD3D9DeviceType(pCaps->DeviceType), 
				pCaps->Caps, pCaps->Caps2, pCaps->Caps3, 
				pCaps->PresentationIntervals,
				pCaps->CursorCaps, pCaps->DevCaps,
				pCaps->PrimitiveMiscCaps, 
				pCaps->RasterCaps, 
				pCaps->ZCmpCaps, 
				pCaps->SrcBlendCaps, 
				pCaps->DestBlendCaps, 
				pCaps->AlphaCmpCaps, 				
				pCaps->ShadeCaps, 
				pCaps->TextureCaps);
			OutTrace(
				"TextureFilterCaps=%x\n\t"
				"CubeTextureFilterCaps=%x\n\t"
				"VolumeTextureFilterCaps=%x\n\t"
				"TextureAddressCaps=%x\n\t"
				"VolumeTextureAddressCaps=%x\n\t"
				"LineCaps=%x\n\t"
				"StencilCaps=%x\n\t"
				"FVFCaps=%x\n\t"
				"TextureOpCaps=%x\n\t"
				"VertexProcessingCaps=%x\n\t",
				pCaps->TextureFilterCaps, 
				pCaps->CubeTextureFilterCaps, 
				pCaps->VolumeTextureFilterCaps, 
				pCaps->TextureAddressCaps, 
				pCaps->VolumeTextureAddressCaps, 
				pCaps->LineCaps, 
				pCaps->StencilCaps, 
				pCaps->FVFCaps, 
				pCaps->TextureOpCaps, 
				pCaps->VertexProcessingCaps);
			OutTrace(
				"MaxTexture(Width x Height)=(%dx%d)\n\t"
				"MaxVolumeExtent=%d\n\t"
				"MaxTextureRepeat=%d\n\t"
				"MaxTextureAspectRatio=%d\n\t"
				"MaxAnisotropy=%d\n\t"
				"MaxActiveLights=%d\n\t"
				"MaxUserClipPlanes=%x\n\t"
				"MaxUserClipPlanes=%x\n\t"
				"MaxVertexW=%f\n\t"
				"GuardBandLeft=%f\n\t"
				"GuardBandTop=%f\n\t"
				"GuardBandRight=%f\n\t"
				"GuardBandBottom=%f\n\t"
				"ExtentsAdjust=%f\n\t"
				"MaxPointSize=%f\n\t"
				"MaxTextureBlendStages=%d\n\t"
				"MaxSimultaneousTextures=%x\n\t"
				"MaxVertexBlendMatrices=%x\n\t"
				"MaxVertexBlendMatrixIndex=%x\n\t"
				"MaxPrimitiveCount=%d\n\t"
				"MaxVertexIndex=%x\n\t"
				"MaxStreams=%x\n\t"
				"MaxStreamStride=%x\n\t"
				"VertexShaderVersion=%x\n\t"
				"MaxVertexShaderConst=%d\n\t"
				"PixelShaderVersion=%x\n\t"
				"PixelShader1xMaxValue=%f\n\t",
				pCaps->MaxTextureWidth, pCaps->MaxTextureHeight, 
				pCaps->MaxVolumeExtent, 
				pCaps->MaxTextureRepeat, 
				pCaps->MaxTextureAspectRatio, 
				pCaps->MaxAnisotropy,
				pCaps->MaxActiveLights, 
				pCaps->MaxUserClipPlanes, 
				pCaps->MaxUserClipPlanes,
				pCaps->MaxVertexW, 
				pCaps->GuardBandLeft, 
				pCaps->GuardBandTop, 
				pCaps->GuardBandRight, 
				pCaps->GuardBandBottom, 
				pCaps->ExtentsAdjust, 
				pCaps->MaxPointSize,
				pCaps->MaxTextureBlendStages, 
				pCaps->MaxSimultaneousTextures, 
				pCaps->MaxVertexBlendMatrices, 
				pCaps->MaxVertexBlendMatrixIndex,
				pCaps->MaxPrimitiveCount, 
				pCaps->MaxVertexIndex, 
				pCaps->MaxStreams, 
				pCaps->MaxStreamStride, 
				pCaps->VertexShaderVersion,
				pCaps->MaxVertexShaderConst, 
				pCaps->PixelShaderVersion, 
				pCaps->PixelShader1xMaxValue);
			if(version == 9) OutTrace(
				"DevCaps2=%x Reserved5=%x\n\t"
				"MaxNpatchTessellationLevel=%f\n\t"
				"MasterAdapterOrdinal=%i\n\t"
				"AdapterOrdinalInGroup=%i\n\t"
				"NumberOfAdaptersInGroup=%i\n\t"
				"DeclTypes=%x\n\t"
				"NumSimultaneousRTs=%x\n\t"
				"StretchRectFilterCaps=%x\n\t"
				"VertexTextureFilterCaps=%x\n\t"
				"MaxVShaderInstructionsExecuted=%x\n\t"
				"MaxPShaderInstructionsExecuted=%x\n\t"
				"MaxVertexShader30InstructionSlots=%x\n\t"
				"MaxPixelShader30InstructionSlots=%x\n",
				pCaps->DevCaps2, pCaps->Reserved5,
				pCaps->MaxNpatchTessellationLevel,
				pCaps->MasterAdapterOrdinal,
				pCaps->AdapterOrdinalInGroup,
				pCaps->NumberOfAdaptersInGroup,
				pCaps->DeclTypes,
				pCaps->NumSimultaneousRTs,
				pCaps->StretchRectFilterCaps,
				pCaps->VertexTextureFilterCaps,
				pCaps->MaxVShaderInstructionsExecuted,
				pCaps->MaxPShaderInstructionsExecuted,
				pCaps->MaxVertexShader30InstructionSlots,
				pCaps->MaxPixelShader30InstructionSlots);
		}
	}
	return res;
}

HRESULT WINAPI extD3DGetDeviceCaps8(void *lpd3d, UINT Adapter, D3DDEVTYPE DeviceType, D3DCAPS8 *pCaps)
{ return extD3DGetDeviceCaps(lpd3d, Adapter, DeviceType, (D3DCAPS9 *)pCaps, 8); }
HRESULT WINAPI extD3DGetDeviceCaps9(void *lpd3d, UINT Adapter, D3DDEVTYPE DeviceType, D3DCAPS9 *pCaps)
{ return extD3DGetDeviceCaps(lpd3d, Adapter, DeviceType, pCaps, 9); }

static HMONITOR WINAPI extGetAdapterMonitor(GetAdapterMonitor_Type pGetAdapterMonitor, void *lpd3d, UINT Adapter)
{
	OutTraceD3D("GetAdapterMonitor: d3d=%x adapter=%d\n", lpd3d, Adapter);
	if((dxw.dwFlags2 & HIDEMULTIMONITOR) && (Adapter != D3DADAPTER_DEFAULT)){
		OutTraceD3D("GetAdapterMonitor: HIDE adapter=%d\n", Adapter);
		return NULL;
	}
	return (*pGetAdapterMonitor)(lpd3d, Adapter);
}

HMONITOR WINAPI extGetAdapterMonitor8(void *lpd3d, UINT Adapter)
{ return extGetAdapterMonitor(pGetAdapterMonitor8, lpd3d, Adapter); }
HMONITOR WINAPI extGetAdapterMonitor9(void *lpd3d, UINT Adapter)
{ return extGetAdapterMonitor(pGetAdapterMonitor9, lpd3d, Adapter); }


static UINT WINAPI extGetAvailableTextureMem(GetAvailableTextureMem_Type pGetAvailableTextureMem, void *lpd3dd)
{
	const UINT TextureMemoryLimit = 1024 * 1024 * 1024; // 1GB
	// const DWORD dwMaxMem = 0x70000000; = 1.8G
	UINT AvailableTextureMem = (*pGetAvailableTextureMem)(lpd3dd);
	OutTraceD3D("GetAvailableTextureMem: lpd3dd=%x AvailableTextureMem=%u(%dMB)\n", lpd3dd, AvailableTextureMem, AvailableTextureMem>>20);
	if((dxw.dwFlags2 & LIMITRESOURCES) && (AvailableTextureMem > TextureMemoryLimit)){
		OutTraceDW("GetAvailableTextureMem: LIMIT AvailableTextureMem=%u->%u\n", AvailableTextureMem, TextureMemoryLimit);
		AvailableTextureMem = TextureMemoryLimit;
	}
	if(dxw.dwFlags5 & STRESSRESOURCES) AvailableTextureMem = 0xFFFFFFFF;
	return AvailableTextureMem;
}

UINT WINAPI extGetAvailableTextureMem8(void *lpd3dd)
{ return extGetAvailableTextureMem(pGetAvailableTextureMem8, lpd3dd); }
UINT WINAPI extGetAvailableTextureMem9(void *lpd3dd)
{ return extGetAvailableTextureMem(pGetAvailableTextureMem9, lpd3dd); }

static HRESULT WINAPI extTestCooperativeLevel(TestCooperativeLevel_Type pTestCooperativeLevel, void *lpd3dd)
{
	HRESULT res;
	res = (*pTestCooperativeLevel)(lpd3dd);
	OutTraceB("TestCooperativeLevel: d3dd=%x res=%x\n", lpd3dd, res);
	return res;
}

HRESULT WINAPI extTestCooperativeLevel8(void *lpd3dd)
{ return extTestCooperativeLevel(pTestCooperativeLevel8, lpd3dd); }
HRESULT WINAPI extTestCooperativeLevel9(void *lpd3dd)
{ return extTestCooperativeLevel(pTestCooperativeLevel9, lpd3dd); }

HRESULT WINAPI extGetSwapChain(void *lpd3dd, UINT iSwapChain, IDirect3DSwapChain9** pSwapChain)
{
	HRESULT res;
	OutTraceD3D("GetSwapChain: d3dd=%x SwapChain=%d\n", lpd3dd, iSwapChain);
	res = (*pGetSwapChain)(lpd3dd, iSwapChain, pSwapChain);
	if(res) OutTraceE("GetSwapChain ERROR: res=%x(%s)\n", res, ExplainDDError(res));
	return res;
}

UINT WINAPI extGetNumberOfSwapChains(void *lpd3dd)
{
	UINT res;
	res = (*pGetNumberOfSwapChains)(lpd3dd);
	OutTraceD3D("GetNumberOfSwapChains: d3dd=%x res=%d\n", lpd3dd, res);
	return res;
}

HRESULT WINAPI extBeginStateBlock8(void *lpd3dd)
{
	HRESULT res;
	OutTraceD3D("BeginStateBlock(8): d3dd=%x\n", lpd3dd);
	res = (*pBeginStateBlock8)(lpd3dd);
	HookD3DDevice8(&lpd3dd);
	return res;
}

HRESULT WINAPI extBeginStateBlock9(void *lpd3dd)
{
	// v2.02.78: D3DDevice9::BeginStateBlock refreshes the COM D3DDevice9 method pointers, so that
	// you need to hook the device object again. This operation fixes the switch to fullscreen mode
	// in "Freedom Force vs. the Third Reich".
	HRESULT res;
	OutTraceD3D("BeginStateBlock(9): d3dd=%x\n", lpd3dd);
	res = (*pBeginStateBlock9)(lpd3dd);
	HookD3DDevice9(&lpd3dd);
	return res;
}

HRESULT WINAPI extEndStateBlock8(void *lpd3dd, DWORD *pToken)
{
	HRESULT res;
	OutTraceD3D("EndStateBlock(8): d3dd=%x\n", lpd3dd);
	res = (*pEndStateBlock8)(lpd3dd, pToken);
	return res;
}

HRESULT WINAPI extEndStateBlock9(void *lpd3dd, IDirect3DStateBlock9** ppSB)
{
	HRESULT res;
	OutTraceD3D("EndStateBlock(9): d3dd=%x\n", lpd3dd);
	res = (*pEndStateBlock9)(lpd3dd, ppSB);
	return res;
}

HRESULT WINAPI extCreateTexture8(void *lpd3dd, UINT Width, UINT Height, UINT Levels, 
								 DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, 
								 void **ppTexture)
{
	HRESULT res;
	OutTraceD3D("CreateTexture(8): lpd3dd=%x size=(%dx%d) levels=%d usage=%x format=%d pool=%x\n",
		lpd3dd, Width, Height, Levels, Usage, Format, Pool);
	res=(*pCreateTexture8)(lpd3dd, Width, Height, Levels, Usage, Format, Pool, ppTexture);
	if (res) 
		OutTraceE("CreateTexture(8) ERROR: err=%x\n", res);
	else {
		OutTraceD3D("CreateTexture(8): lpTexture=%x\n", *ppTexture);
		HookD3DTexture8(ppTexture);
	}
	return res;
}

HRESULT WINAPI extCreateTexture9(void *lpd3dd, UINT Width, UINT Height, UINT Levels, 
								 DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, 
								 void **ppTexture, HANDLE *pSharedHandle)
{
	HRESULT res;
	OutTraceD3D("CreateTexture(9): lpd3dd=%x size=(%dx%d) levels=%d usage=%x format=%d pool=%x\n",
		lpd3dd, Width, Height, Levels, Usage, Format, Pool, ppTexture, pSharedHandle);
	res=(*pCreateTexture9)(lpd3dd, Width, Height, Levels, Usage, Format, Pool, ppTexture, pSharedHandle);
	if (res) 
		OutTraceE("CreateTexture(9) ERROR: err=%x\n", res);
	else {
		OutTraceD3D("CreateTexture(9): lpTexture=%x\n", *ppTexture);
		HookD3DTexture9(ppTexture);
	}
	return res;
}

static HRESULT WINAPI extLockRect(LockRect_Type pLockRect, void *lpd3dtex, UINT Level, D3DLOCKED_RECT *pLockedRect, CONST RECT *pRect, DWORD Flags)
{
	HRESULT res;
	if(IsTraceD3D){
		char sRect[81];
		if(pRect) 
			sprintf_s(sRect, 80, "(%d,%d)-(%d,%d)", pRect->left, pRect->top, pRect->right, pRect->bottom);
		else 
			strcpy_s(sRect, 80, "NULL");
		OutTrace("Texture::LockRect: lpd3dtex=%x level=%d rect=%s flags=%x\n", lpd3dtex, Level, sRect, Flags);
	}
	res=(*pLockRect)(lpd3dtex, Level, pLockedRect, pRect, Flags);
	if (res) OutTraceE("Device::LockRect ERROR: err=%x\n", res);
	else {
		OutTraceD3D("Device::LockRect: pBits=%x pitch=%d\n", pLockedRect->pBits, pLockedRect->Pitch);
	}
	return res;
}

HRESULT WINAPI extLockRect8(void *lpd3dtex, UINT Level, D3DLOCKED_RECT *pLockedRect, CONST RECT *pRect, DWORD Flags)
{ return extLockRect(pLockRect8, lpd3dtex, Level, pLockedRect, pRect, Flags); }
HRESULT WINAPI extLockRect9(void *lpd3dtex, UINT Level, D3DLOCKED_RECT *pLockedRect, CONST RECT *pRect, DWORD Flags)
{ return extLockRect(pLockRect9, lpd3dtex, Level, pLockedRect, pRect, Flags); }

typedef void (*TextureHandling_Type)(void *, int);

static HRESULT WINAPI extUnlockRect(UnlockRect_Type pUnlockRect, void *lpd3dtex, UINT Level, TextureHandling_Type TextureHandling)
{
	HRESULT res;
	OutTraceD3D("Texture::UnlockRect: lpd3dtex=%x level=%d\n", lpd3dtex, Level);
	res=(*pUnlockRect)(lpd3dtex, Level);

	if(TextureHandling) TextureHandling(lpd3dtex, Level);
	return res;
}

HRESULT WINAPI extUnlockRect8(void *lpd3dtex, UINT Level)
{ return extUnlockRect(pUnlockRect8, lpd3dtex, Level, D3D8TextureHandling); }
HRESULT WINAPI extUnlockRect9(void *lpd3dtex, UINT Level)
{ return extUnlockRect(pUnlockRect9, lpd3dtex, Level, D3D9TextureHandling); }

void RestoreD3DSurfaces(BOOL bFullScreen)
{
	//DWORD param[64];
	//HRESULT res;

	// a well programmed D3D game should provide to Reset by itself, so the best thing to do is NOTHING
	return;

	//if(lpD3DActiveDevice){
	//	OutTrace("RestoreD3DSurfaces(%d): reset device %x fullscreen=%x\n", dwD3DVersion, lpD3DActiveDevice, bFullScreen);
	//	RECT Desktop;
	//	memcpy(param, gActiveDeviceParams, sizeof(param));
	//	if(bFullScreen){
	//		(*pGetClientRect)(0, &Desktop);
	//		param[0] = Desktop.right;			// BackBufferWidth
	//		param[1] = Desktop.bottom;			// BackBufferHeight
	//	}
	//	res=(*pReset)(lpD3DActiveDevice, (D3DPRESENT_PARAMETERS *)param);
	//	if(res != DD_OK) OutTraceE("RestoreD3DSurfaces: Reset size=(%dx%d) err=%x(%s)\n", 
	//		param[0], param[1], res, ExplainDDError(res));
	//}
}
