#include <windows.h>
#include <d3d.h>
#include "dxwnd.h"
#include "dxwcore.hpp"
#include "dxhook.h"
#include "syslibs.h"
#include "dxhelper.h"

// exported API

DWORD gD3DVersion; // quick & dirty ....
//#undef OutTraceDW
//#define OutTraceDW OutTrace

typedef HRESULT (WINAPI *Direct3DCreateDevice_Type)(GUID FAR *, LPDIRECT3D, LPDIRECTDRAWSURFACE, LPDIRECT3D *, LPUNKNOWN);
typedef HRESULT (WINAPI *Direct3DCreate_Type)(UINT, LPDIRECT3D *, LPUNKNOWN);

Direct3DCreateDevice_Type pDirect3DCreateDevice = NULL;
Direct3DCreate_Type pDirect3DCreate = NULL;

HRESULT WINAPI extDirect3DCreateDevice(GUID FAR *, LPDIRECT3D, LPDIRECTDRAWSURFACE, LPDIRECT3D *, LPUNKNOWN);
HRESULT WINAPI extDirect3DCreate(UINT, LPDIRECT3D *, LPUNKNOWN);

// IDirect3D interfaces

typedef HRESULT (WINAPI *Initialize_Type)(void *);
typedef HRESULT (WINAPI *EnumDevices_Type)(void *, LPD3DENUMDEVICESCALLBACK, LPVOID);
typedef HRESULT (WINAPI *CreateLight_Type)(void *, LPDIRECT3DLIGHT *, IUnknown *);
typedef HRESULT (WINAPI *CreateMaterial_Type)(void *, LPDIRECT3DMATERIAL *, IUnknown *);
typedef HRESULT (WINAPI *CreateViewport_Type)(void *, LPDIRECT3DVIEWPORT *, IUnknown *);
typedef HRESULT (WINAPI *FindDevice_Type)(void *, LPD3DFINDDEVICESEARCH, LPD3DFINDDEVICERESULT);
typedef HRESULT (WINAPI *CreateDevice2_Type)(void *, REFCLSID, LPDIRECTDRAWSURFACE, LPDIRECT3DDEVICE2 *);
typedef HRESULT (WINAPI *CreateDevice3_Type)(void *, REFCLSID, LPDIRECTDRAWSURFACE4, LPDIRECT3DDEVICE3 *, LPUNKNOWN);

typedef HRESULT (WINAPI *InitializeVP_Type)(void *, LPDIRECT3D);
typedef HRESULT (WINAPI *SetViewport_Type)(void *, LPD3DVIEWPORT);
typedef HRESULT (WINAPI *GetViewport_Type)(void *, LPD3DVIEWPORT);
typedef HRESULT (WINAPI *QueryInterfaceD3_Type)(void *, REFIID, LPVOID *);

typedef HRESULT (WINAPI *D3DInitialize_Type)(void *, LPDIRECT3D , LPGUID, LPD3DDEVICEDESC);
typedef HRESULT (WINAPI *D3DGetCaps_Type)(void *, LPD3DDEVICEDESC ,LPD3DDEVICEDESC);

typedef HRESULT (WINAPI *SetRenderState3_Type)(void *, D3DRENDERSTATETYPE, DWORD);
typedef HRESULT (WINAPI *Scene3_Type)(void *);
typedef HRESULT (WINAPI *GetCaps3_Type)(void *, LPD3DDEVICEDESC, LPD3DDEVICEDESC);
typedef HRESULT (WINAPI *SetLightState_Type)(void *, D3DLIGHTSTATETYPE, DWORD);

Initialize_Type pInitialize = NULL;
EnumDevices_Type pEnumDevices = NULL;
CreateLight_Type pCreateLight = NULL;
CreateMaterial_Type pCreateMaterial = NULL;
CreateViewport_Type pCreateViewport = NULL;
FindDevice_Type pFindDevice = NULL;
CreateDevice2_Type pCreateDevice2 = NULL;
CreateDevice3_Type pCreateDevice3 = NULL;

InitializeVP_Type pInitializeVP = NULL;
SetViewport_Type pSetViewport = NULL;
GetViewport_Type pGetViewport = NULL;
QueryInterfaceD3_Type pQueryInterfaceD3 = NULL;

SetRenderState3_Type pSetRenderState2 = NULL;
SetRenderState3_Type pSetRenderState3 = NULL;
Scene3_Type pBeginScene1 = NULL;
Scene3_Type pBeginScene2 = NULL;
Scene3_Type pBeginScene3 = NULL;
Scene3_Type pEndScene1 = NULL;
Scene3_Type pEndScene2 = NULL;
Scene3_Type pEndScene3 = NULL;

GetCaps3_Type pGetCaps3 = NULL;

SetLightState_Type pSetLightState3 = NULL;

D3DInitialize_Type pD3DInitialize = NULL;
D3DGetCaps_Type pD3DGetCaps = NULL;

HRESULT WINAPI extInitialize(void *);
HRESULT WINAPI extEnumDevices(void *, LPD3DENUMDEVICESCALLBACK, LPVOID);
HRESULT WINAPI extCreateLight(void *, LPDIRECT3DLIGHT *, IUnknown *);
HRESULT WINAPI extCreateMaterial(void *, LPDIRECT3DMATERIAL *, IUnknown *);
HRESULT WINAPI extCreateViewport(void *, LPDIRECT3DVIEWPORT *, IUnknown *);
HRESULT WINAPI extFindDevice(void *, LPD3DFINDDEVICESEARCH, LPD3DFINDDEVICERESULT);
HRESULT WINAPI extCreateDevice2(void *, REFCLSID, LPDIRECTDRAWSURFACE, LPDIRECT3DDEVICE2 *);
HRESULT WINAPI extCreateDevice3(void *, REFCLSID, LPDIRECTDRAWSURFACE4, LPDIRECT3DDEVICE3 *, LPUNKNOWN);

HRESULT WINAPI extInitializeVP(void *, LPDIRECT3D);
HRESULT WINAPI extSetViewport(void *, LPD3DVIEWPORT);
HRESULT WINAPI extGetViewport(void *, LPD3DVIEWPORT);
HRESULT WINAPI extQueryInterfaceD3(void *, REFIID, LPVOID *);

HRESULT WINAPI extD3DInitialize(void *, LPDIRECT3D , LPGUID, LPD3DDEVICEDESC);
HRESULT WINAPI extD3DGetCaps(void *, LPD3DDEVICEDESC ,LPD3DDEVICEDESC);

HRESULT WINAPI extSetRenderState2(void *, D3DRENDERSTATETYPE, DWORD);
HRESULT WINAPI extBeginScene1(void *);
HRESULT WINAPI extEndScene1(void *);
HRESULT WINAPI extBeginScene2(void *);
HRESULT WINAPI extEndScene2(void *);
HRESULT WINAPI extSetRenderState3(void *, D3DRENDERSTATETYPE, DWORD);
HRESULT WINAPI extBeginScene3(void *);
HRESULT WINAPI extEndScene3(void *);
HRESULT WINAPI extGetCaps3(void *, LPD3DDEVICEDESC, LPD3DDEVICEDESC);
HRESULT WINAPI extSetLightState3(void *d3dd, D3DLIGHTSTATETYPE d3dls, DWORD t);

extern char *ExplainDDError(DWORD);

int HookDirect3D7(HMODULE module, int version){
	void *tmp;
	HINSTANCE hinst;
	LPDIRECT3D lpd3d=NULL;
	HRESULT res;

	gD3DVersion = version; 

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
		OutTraceDW("GetProcAddress: hooking proc=%s at addr=%x\n", ProcToString(proc), pDirect3DCreate);
		return (FARPROC)extDirect3DCreate;
	}
	if (!strcmp(proc,"Direct3DCreateDevice") && !pDirect3DCreateDevice){
		pDirect3DCreateDevice=(Direct3DCreateDevice_Type)(*pGetProcAddress)(hModule, proc);
		OutTraceDW("GetProcAddress: hooking proc=%s at addr=%x\n", ProcToString(proc), pDirect3DCreateDevice);
		return (FARPROC)extDirect3DCreateDevice;
	}
	// NULL -> keep the original call address
	return NULL;
}

HRESULT WINAPI extDirect3DCreateDevice(GUID FAR *lpGUID, LPDIRECT3D lpd3ddevice, LPDIRECTDRAWSURFACE surf, LPDIRECT3D *lplpd3ddevice, LPUNKNOWN pUnkOuter)
{
	HRESULT res;

	OutTraceD3D("Direct3DCreateDevice: guid=%x d3ddevice=%x dds=%x UnkOuter=%x\n",
		lpGUID, lpd3ddevice, surf, pUnkOuter);
	res=(*pDirect3DCreateDevice)(lpGUID, lpd3ddevice, surf, lplpd3ddevice, pUnkOuter);
	if(res) OutTraceE("Direct3DCreateDevice ERROR: err=%x(%s) at %d\n", res, ExplainDDError(res), __LINE__);
	else OutTraceD3D("Direct3DCreateDevice: d3ddevice=%x\n", *lplpd3ddevice);
	return res;
}

void HookDirect3DSession(LPDIRECTDRAW *lplpdd, int dxversion)
{
	OutTraceDW("HookDirect3DSession: d3d=%x d3dversion=%d\n", *lplpdd, dxversion);

	gD3DVersion = dxversion;

	switch(dxversion){
	case 1:
		SetHook((void *)(**(DWORD **)lplpdd +   0), extQueryInterfaceD3, (void **)&pQueryInterfaceD3, "QueryInterface(D3S)");
		SetHook((void *)(**(DWORD **)lplpdd +  12), extInitialize, (void **)&pInitialize, "Initialize");
		SetHook((void *)(**(DWORD **)lplpdd +  16), extEnumDevices, (void **)&pEnumDevices, "EnumDevices");
		SetHook((void *)(**(DWORD **)lplpdd +  20), extCreateLight, (void **)&pCreateLight, "CreateLight");
		SetHook((void *)(**(DWORD **)lplpdd +  24), extCreateMaterial, (void **)&pCreateMaterial, "CreateMaterial");
		SetHook((void *)(**(DWORD **)lplpdd +  28), extCreateViewport, (void **)&pCreateViewport, "CreateViewport");
		SetHook((void *)(**(DWORD **)lplpdd +  32), extFindDevice, (void **)&pFindDevice, "FindDevice");	
		break;
	case 5:
		SetHook((void *)(**(DWORD **)lplpdd +   0), extQueryInterfaceD3, (void **)&pQueryInterfaceD3, "QueryInterface(D3S)");
		SetHook((void *)(**(DWORD **)lplpdd +  12), extEnumDevices, (void **)&pEnumDevices, "EnumDevices");
		SetHook((void *)(**(DWORD **)lplpdd +  16), extCreateLight, (void **)&pCreateLight, "CreateLight");
		SetHook((void *)(**(DWORD **)lplpdd +  20), extCreateMaterial, (void **)&pCreateMaterial, "CreateMaterial");
		SetHook((void *)(**(DWORD **)lplpdd +  24), extCreateViewport, (void **)&pCreateViewport, "CreateViewport");
		SetHook((void *)(**(DWORD **)lplpdd +  28), extFindDevice, (void **)&pFindDevice, "FindDevice");
		SetHook((void *)(**(DWORD **)lplpdd +  32), extCreateDevice2, (void **)&pCreateDevice2, "CreateDevice(D3D2)");
		break;
	case 6:
		SetHook((void *)(**(DWORD **)lplpdd +   0), extQueryInterfaceD3, (void **)&pQueryInterfaceD3, "QueryInterface(D3S)");
		SetHook((void *)(**(DWORD **)lplpdd +  12), extEnumDevices, (void **)&pEnumDevices, "EnumDevices");
		SetHook((void *)(**(DWORD **)lplpdd +  16), extCreateLight, (void **)&pCreateLight, "CreateLight");
		SetHook((void *)(**(DWORD **)lplpdd +  20), extCreateMaterial, (void **)&pCreateMaterial, "CreateMaterial");
		SetHook((void *)(**(DWORD **)lplpdd +  24), extCreateViewport, (void **)&pCreateViewport, "CreateViewport");
		SetHook((void *)(**(DWORD **)lplpdd +  28), extFindDevice, (void **)&pFindDevice, "FindDevice");
		SetHook((void *)(**(DWORD **)lplpdd +  32), extCreateDevice3, (void **)&pCreateDevice3, "CreateDevice(D3D3)");
		break;
	}
} 


void HookDirect3DDevice(void **lpd3ddev, int dxversion)
{
	OutTraceDW("HookDirect3DDevice: d3ddev=%x d3dversion=%d\n", lpd3ddev, dxversion);

	gD3DVersion = dxversion;

	switch(dxversion){
	case 1:
		//SetHook((void *)(**(DWORD **)lpd3ddev +  16), extGetCaps1, (void **)&pGetCaps1, "GetCaps(1)");
		SetHook((void *)(**(DWORD **)lpd3ddev +  76), extBeginScene1, (void **)&pBeginScene1, "BeginScene(1)");
		SetHook((void *)(**(DWORD **)lpd3ddev +  80), extEndScene1, (void **)&pEndScene1, "EndScene(1)");
		break;
	case 2:
		//SetHook((void *)(**(DWORD **)lpd3ddev +  12), extGetCaps2, (void **)&pGetCaps2, "GetCaps(2)");
		SetHook((void *)(**(DWORD **)lpd3ddev +  40), extBeginScene2, (void **)&pBeginScene2, "BeginScene(2)");
		SetHook((void *)(**(DWORD **)lpd3ddev +  44), extEndScene2, (void **)&pEndScene2, "EndScene(2)");
		SetHook((void *)(**(DWORD **)lpd3ddev +  92), extSetRenderState2, (void **)&pSetRenderState2, "SetRenderState(2)");
		if(pSetRenderState2){
			if(dxw.dwFlags2 & WIREFRAME)(*pSetRenderState2)(*lpd3ddev, D3DRENDERSTATE_FILLMODE, D3DFILL_WIREFRAME); 		
			if(dxw.dwFlags4 & DISABLEFOGGING) (*pSetRenderState2)(*lpd3ddev, D3DRENDERSTATE_FOGENABLE, FALSE); 
		}		
		break;
	case 3:
		SetHook((void *)(**(DWORD **)lpd3ddev +  12), extGetCaps3, (void **)&pGetCaps3, "GetCaps(3)");
		SetHook((void *)(**(DWORD **)lpd3ddev +  36), extBeginScene3, (void **)&pBeginScene3, "BeginScene(3)");
		SetHook((void *)(**(DWORD **)lpd3ddev +  40), extEndScene3, (void **)&pEndScene3, "EndScene(3)");
		SetHook((void *)(**(DWORD **)lpd3ddev +  88), extSetRenderState3, (void **)&pSetRenderState3, "SetRenderState(3)");
		SetHook((void *)(**(DWORD **)lpd3ddev +  96), extSetLightState3, (void **)&pSetLightState3, "SetLightState(3)");
		if(pSetRenderState3){
			if(dxw.dwFlags2 & WIREFRAME)(*pSetRenderState3)(*lpd3ddev, D3DRENDERSTATE_FILLMODE, D3DFILL_WIREFRAME); 		
			if(dxw.dwFlags4 & DISABLEFOGGING) (*pSetRenderState3)(*lpd3ddev, D3DRENDERSTATE_FOGENABLE, FALSE); 
		}		
		break;
	}
} 

HRESULT WINAPI extDirect3DCreate(UINT SDKVersion, LPDIRECT3D *lplpd3d, LPUNKNOWN pUnkOuter)
{
	HRESULT res;

	OutTraceD3D("Direct3DCreate: SDKVersion=%x UnkOuter=%x\n", SDKVersion, pUnkOuter);
	res=(*pDirect3DCreate)(SDKVersion, lplpd3d, pUnkOuter);

	if(res) {
		OutTraceE("Direct3DCreate ERROR: err=%x(%s) at %d\n", res, ExplainDDError(res), __LINE__);
		return res;
	}

	HookDirect3DSession((LPDIRECTDRAW *)lplpd3d, SDKVersion);
	OutTraceD3D("Direct3DCreate: d3d=%x\n", *lplpd3d);
	return res;
}

void HookViewport(LPDIRECT3DVIEWPORT *lpViewport, int dxversion)
{
	OutTraceDW("HookViewport: Viewport=%x d3dversion=%d\n", *lpViewport, dxversion);

 	switch(dxversion){
	case 1:
	case 5:
	case 6:
	case 7:
		SetHook((void *)(**(DWORD **)lpViewport +  12), extInitializeVP, (void **)&pInitializeVP, "Initialize(VP)");
		SetHook((void *)(**(DWORD **)lpViewport +  16), extGetViewport, (void **)&pGetViewport, "GetViewport");
		SetHook((void *)(**(DWORD **)lpViewport +  20), extSetViewport, (void **)&pSetViewport, "SetViewport");
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
		case 0x6aae1ec1: d3dversion=5; break;
		case 0xbb223240: d3dversion=6; break;
		case 0xf5049e77: d3dversion=7; break;
	}
	if(d3dversion) OutTraceDW("QueryInterface(D3): hooking version=%d\n", d3dversion);
	switch(d3dversion){
	case 1:
		SetHook((void *)(**(DWORD **)ppvObj +  12), extInitialize, (void **)&pInitialize, "Initialize");
		SetHook((void *)(**(DWORD **)ppvObj +  16), extEnumDevices, (void **)&pEnumDevices, "EnumDevices");
		SetHook((void *)(**(DWORD **)ppvObj +  20), extCreateLight, (void **)&pCreateLight, "CreateLight");
		SetHook((void *)(**(DWORD **)ppvObj +  24), extCreateMaterial, (void **)&pCreateMaterial, "CreateMaterial");
		SetHook((void *)(**(DWORD **)ppvObj +  28), extCreateViewport, (void **)&pCreateViewport, "CreateViewport");
		SetHook((void *)(**(DWORD **)ppvObj +  32), extFindDevice, (void **)&pFindDevice, "FindDevice");	
		break;
	case 5:
	case 6:
		SetHook((void *)(**(DWORD **)ppvObj +  12), extEnumDevices, (void **)&pEnumDevices, "EnumDevices");
		SetHook((void *)(**(DWORD **)ppvObj +  16), extCreateLight, (void **)&pCreateLight, "CreateLight");
		SetHook((void *)(**(DWORD **)ppvObj +  20), extCreateMaterial, (void **)&pCreateMaterial, "CreateMaterial");
		SetHook((void *)(**(DWORD **)ppvObj +  24), extCreateViewport, (void **)&pCreateViewport, "CreateViewport");
		SetHook((void *)(**(DWORD **)ppvObj +  28), extFindDevice, (void **)&pFindDevice, "FindDevice");
		break;
	}
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

static void DumpD3DDevideDesc(LPD3DDEVICEDESC d3, char *label)
{
	if(IsTraceD3D){
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
		OutTrace("\n");
	}
}

HRESULT WINAPI extDeviceProxy(GUID FAR *lpGuid, LPSTR lpDeviceDescription, LPSTR lpDeviceName, LPD3DDEVICEDESC lpd3ddd1, LPD3DDEVICEDESC lpd3ddd2, LPVOID arg)
{
	HRESULT res;
	OutTraceDW("EnumDevices: CALLBACK GUID=%x(%s) DeviceDescription=\"%s\", DeviceName=\"%s\", arg=%x\n", lpGuid->Data1, ExplainGUID(lpGuid), lpDeviceDescription, lpDeviceName, ((CallbackArg *)arg)->arg);
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
	OutTraceDW("EnumDevices: CALLBACK ret=%x\n", res);
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
	return res;
}

HRESULT WINAPI extCreateViewport(void *lpd3d, LPDIRECT3DVIEWPORT *lpViewport, IUnknown *p0)
{
	HRESULT res;

	OutTraceD3D("CreateViewport: d3d=%x\n", lpd3d);
	res=(*pCreateViewport)(lpd3d, lpViewport, p0);
	if(res) OutTraceE("CreateViewport ERROR: err=%x(%s) at %d\n", res, ExplainDDError(res), __LINE__);
	else OutTraceD3D("CreateViewport: Viewport=%x\n", *lpViewport);
	HookViewport(lpViewport, gD3DVersion);
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
		lpvp, vpd, vpd->dwSize, vpd->dwX, vpd->dwY, vpd->dwHeight, vpd->dwWidth, vpd->dvScaleX, vpd->dvScaleY, 
		vpd->dvMaxX, vpd->dvMaxY, vpd->dvMaxZ, vpd->dvMinZ);
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
	if(res) OutTraceE("GetViewport ERROR: err=%x(%s) at %d\n", res, ExplainDDError(res), __LINE__);
	else OutTraceD3D("GetViewport: OK size=%d pos=(%d,%d) dim=(%dx%d) scale=(%fx%f) maxXYZ=(%f,%f,%f) minZ=%f\n",
		vpd->dwSize, vpd->dwX, vpd->dwY, vpd->dwHeight, vpd->dwWidth, vpd->dvScaleX, vpd->dvScaleY, 
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

	OutTraceD3D("CreateDevice(D3D2): d3d=%x GUID=%x(%s) lpdds=%x\n", lpd3d, Guid.Data1, ExplainGUID((GUID *)&Guid), lpdds);
	res=(*pCreateDevice2)(lpd3d, Guid, lpdds, lplpd3dd);
	if(res) {
		OutTraceE("CreateDevice(D3D2) ERROR: err=%x(%s) at %d\n", res, ExplainDDError(res), __LINE__);
		return res;
	}
	else 
		OutTraceD3D("CreateDevice(D3D2): lpd3dd=%x\n", lpd3d, *lplpd3dd);

	HookDirect3DDevice((void **)lplpd3dd, 2);
	return res;
}

const GUID IID_IDirect3DRampDevice;
const GUID IID_IDirect3DRGBDevice;

HRESULT WINAPI extCreateDevice3(void *lpd3d, REFCLSID Guid, LPDIRECTDRAWSURFACE4 lpdds, LPDIRECT3DDEVICE3 *lplpd3dd, LPUNKNOWN unk)
{
	HRESULT res;
	GUID IID_IDirect3DRGBDevice = {0xA4665C60,0x2673,0x11CF,0xA3,0x1A,0x00,0xAA,0x00,0xB9,0x33,0x56};
	GUID IID_IDirect3DRampDevice = {0xF2086B20,0x259F,0x11CF,0xA3,0x1A,0x00,0xAA,0x00,0xB9,0x33,0x56};
	GUID IID_IDirect3DMMXDevice = {0x881949a1,0xd6f3,0x11d0,0x89,0xab,0x00,0xa0,0xc9,0x05,0x41,0x29};

	OutTraceD3D("CreateDevice(D3D3): d3d=%x GUID=%x(%s) lpdds=%x\n", lpd3d, Guid.Data1, ExplainGUID((GUID *)&Guid), lpdds);
	res=(*pCreateDevice3)(lpd3d, Guid, lpdds, lplpd3dd, unk);
	//res=(*pCreateDevice3)(lpd3d, IID_IDirect3DMMXDevice, lpdds, lplpd3dd, unk);
	if(res) {
		OutTraceE("CreateDevice(D3D3) ERROR: err=%x(%s) at %d\n", res, ExplainDDError(res), __LINE__);
		//if(1) {
		//	GUID IID_IDirect3DRGBDevice = {0xA4665C60,0x2673,0x11CF,0xA3,0x1A,0x00,0xAA,0x00,0xB9,0x33,0x56};
		//	GUID IID_IDirect3DRampDevice = {0xF2086B20,0x259F,0x11CF,0xA3,0x1A,0x00,0xAA,0x00,0xB9,0x33,0x56};
		//	GUID IID_IDirect3DMMXDevice = {0x881949a1,0xd6f3,0x11d0,0x89,0xab,0x00,0xa0,0xc9,0x05,0x41,0x29};
		//	res=(*pCreateDevice3)(lpd3d, IID_IDirect3DRGBDevice, lpdds, lplpd3dd, unk);
		//	if(res) OutTraceE("CreateDevice(IID_IDirect3DRGBDevice) ERROR: err=%x(%s) at %d\n", res, ExplainDDError(res), __LINE__);
		//	res=(*pCreateDevice3)(lpd3d, IID_IDirect3DRampDevice, lpdds, lplpd3dd, unk);
		//	if(res) OutTraceE("CreateDevice(IID_IDirect3DRampDevice) ERROR: err=%x(%s) at %d\n", res, ExplainDDError(res), __LINE__);
		//	res=(*pCreateDevice3)(lpd3d, IID_IDirect3DMMXDevice, lpdds, lplpd3dd, unk);
		//	if(res) OutTraceE("CreateDevice(IID_IDirect3DMMXDevice) ERROR: err=%x(%s) at %d\n", res, ExplainDDError(res), __LINE__);
		//}
		return res;
	}

	HookDirect3DDevice((void **)lplpd3dd, 3);

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

#if 0
//HRESULT WINAPI extSwapTextureHandles(void *d3dd, LPDIRECT3DTEXTURE,LPDIRECT3DTEXTURE)
//HRESULT WINAPI extCreateExecuteBuffer(void *d3dd, LPD3DEXECUTEBUFFERDESC,LPDIRECT3DEXECUTEBUFFER*,IUnknown*)
//HRESULT WINAPI extGetStats(void *d3dd, LPD3DSTATS)
HRESULT WINAPI extExecute(void *d3dd, LPDIRECT3DEXECUTEBUFFER lpd3dEB, LPDIRECT3DVIEWPORT lpd3dVP, DWORD dwFlags)
HRESULT WINAPI extAddViewport(void *d3dd, LPDIRECT3DVIEWPORT)
HRESULT WINAPI extDeleteViewport(void *d3dd,HIS_ LPDIRECT3DVIEWPORT)
HRESULT WINAPI extNextViewport(void *d3dd, LPDIRECT3DVIEWPORT,LPDIRECT3DVIEWPORT*,DWORD)
HRESULT WINAPI extPick(void *d3dd, LPDIRECT3DEXECUTEBUFFER,LPDIRECT3DVIEWPORT,DWORD,LPD3DRECT)
HRESULT WINAPI extGetPickRecords(void *d3dd, LPDWORD,LPD3DPICKRECORD)
HRESULT WINAPI extEnumTextureFormats(void *d3dd, LPD3DENUMTEXTUREFORMATSCALLBACK,LPVOID)
HRESULT WINAPI extCreateMatrix(void *d3dd, LPD3DMATRIXHANDLE)
HRESULT WINAPI extSetMatrix(void *d3dd, D3DMATRIXHANDLE,const LPD3DMATRIX)
HRESULT WINAPI extGetMatrix(void *d3dd, D3DMATRIXHANDLE,LPD3DMATRIX)
HRESULT WINAPI extDeleteMatrix(void *d3dd, D3DMATRIXHANDLE)
HRESULT WINAPI extBeginScene(void *d3dd)
HRESULT WINAPI extEndScene(void *d3dd)
HRESULT WINAPI extGetDirect3D(void *d3dd, LPDIRECT3D*)
#endif

char *ExplainRenderstateValue(DWORD Value)
{
	char *p;
	switch(Value){
	case D3DCMP_NEVER               : p="D3DCMP_NEVER"; break;
	case D3DCMP_LESS                : p="D3DCMP_LESS"; break;
	case D3DCMP_EQUAL               : p="D3DCMP_EQUAL"; break;
	case D3DCMP_LESSEQUAL           : p="D3DCMP_LESSEQUAL"; break;
	case D3DCMP_GREATER             : p="D3DCMP_GREATER"; break;
	case D3DCMP_NOTEQUAL            : p="D3DCMP_NOTEQUAL"; break;
	case D3DCMP_GREATEREQUAL        : p="D3DCMP_GREATEREQUAL"; break;
	case D3DCMP_ALWAYS              : p="D3DCMP_ALWAYS"; break;
	default							: p="???"; break;
	}
	return p;
}

HRESULT WINAPI extSetRenderState2(void *d3dd, D3DRENDERSTATETYPE State, DWORD Value)
{
	HRESULT res;
	OutTraceD3D("SetRenderState(2): d3dd=%x State=%x(%s) Value=%x\n", d3dd, State, ExplainD3DRenderState(State), Value);
	if((dxw.dwFlags4 & ZBUFFERALWAYS) && (State == D3DRENDERSTATE_ZFUNC)) {
		DWORD OldValue;
		OldValue = Value;
		Value = D3DCMP_ALWAYS;
		OutTraceDW("SetRenderState: FIXED State=ZFUNC Value=%s->D3DCMP_ALWAYS\n", ExplainRenderstateValue(OldValue));
	}
	if((dxw.dwFlags2 & WIREFRAME) && (State == D3DRENDERSTATE_FILLMODE)){
		Value = D3DFILL_WIREFRAME;
		OutTraceDW("SetRenderState: FIXED State=FILLMODE Value=D3DFILL_WIREFRAME\n");
	}
	res=(*pSetRenderState2)(d3dd, State, Value);
	if(res) OutTraceE("SetRenderState(2): res=%x(%s)\n", res, ExplainDDError(res));
	return res;
}

HRESULT WINAPI extSetRenderState3(void *d3dd, D3DRENDERSTATETYPE State, DWORD Value)
{
	HRESULT res;
	OutTraceD3D("SetRenderState(3): d3dd=%x State=%x(%s) Value=%x\n", d3dd, State, ExplainD3DRenderState(State), Value);
	if((dxw.dwFlags4 & ZBUFFERALWAYS) && (State == D3DRENDERSTATE_ZFUNC)) {
		DWORD OldValue;
		OldValue = Value;
		Value = D3DCMP_ALWAYS;
		OutTraceDW("SetRenderState: FIXED State=ZFUNC Value=%s->D3DCMP_ALWAYS\n", ExplainRenderstateValue(OldValue));
	}
	if((dxw.dwFlags2 & WIREFRAME) && (State == D3DRENDERSTATE_FILLMODE)){
		Value = D3DFILL_WIREFRAME;
		OutTraceDW("SetRenderState: FIXED State=FILLMODE Value=D3DFILL_WIREFRAME\n");
	}
	if((dxw.dwFlags4 & DISABLEFOGGING) && (State == D3DRENDERSTATE_FOGENABLE)){
		OutTraceDW("SetRenderState: FIXED State=FOGENABLE Value=FALSE\n");
		Value = FALSE;
	}
	res=(*pSetRenderState3)(d3dd, State, Value);
	if(res) OutTraceE("SetRenderState(3): res=%x(%s)\n", res, ExplainDDError(res));
	return res;
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
	if(dxw.dwFlags4 & ZBUFFERCLEAN){
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
			OutTraceDW("d3dRect=(%d,%d)-(%d,%d)\n", d3dRect.x1, d3dRect.y1, d3dRect.x2, d3dRect.y2);
			vp->Clear(1, &d3dRect, D3DCLEAR_ZBUFFER);	
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
	if(dxw.dwFlags4 & (ZBUFFERCLEAN|ZBUFFER0CLEAN)){
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
			OutTraceDW("d3dRect=(%d,%d)-(%d,%d)\n", d3dRect.x1, d3dRect.y1, d3dRect.x2, d3dRect.y2);
			if(dxw.dwFlags4 & ZBUFFERCLEAN )vp->Clear2(1, &d3dRect, D3DCLEAR_ZBUFFER, 0, 1.0, 0);	
			if(dxw.dwFlags4 & ZBUFFER0CLEAN)vp->Clear2(1, &d3dRect, D3DCLEAR_ZBUFFER, 0, 0.0, 0);	
		}
	}
	res=(*pBeginScene3)(d3dd);
	if(res) OutTraceE("BeginScene(3): res=%x(%s)\n", res, ExplainDDError(res));
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
