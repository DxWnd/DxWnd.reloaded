#define DIRECTINPUT_VERSION 0x800

#include <windows.h>
#include <dinput.h>
#include "dxwnd.h"
#include "dxwcore.hpp"
#include "syslibs.h"

typedef HRESULT (WINAPI *QueryInterface_Type)(void *, REFIID, LPVOID *);
typedef HRESULT (WINAPI *DirectInputCreate_Type)(HINSTANCE, DWORD, LPDIRECTINPUT *, LPUNKNOWN);
typedef HRESULT (WINAPI *DirectInputCreateEx_Type)(HINSTANCE, DWORD, REFIID, LPVOID *, LPUNKNOWN);
typedef HRESULT (WINAPI *DICreateDevice_Type)(LPDIRECTINPUT, REFGUID, LPDIRECTINPUTDEVICE *, LPUNKNOWN);
typedef HRESULT (WINAPI *DICreateDeviceEx_Type)(LPDIRECTINPUT, REFGUID, REFIID, LPVOID *, LPUNKNOWN);
typedef HRESULT (WINAPI *GetDeviceData_Type)(LPDIRECTINPUTDEVICE, DWORD, LPVOID, LPDWORD, DWORD);
typedef HRESULT (WINAPI *GetDeviceState_Type)(LPDIRECTINPUTDEVICE, DWORD, LPDIMOUSESTATE);
typedef HRESULT (WINAPI *DISetCooperativeLevel_Type)(LPDIRECTINPUTDEVICE, HWND, DWORD);
typedef HRESULT (WINAPI *SetDataFormat_Type)(LPDIRECTINPUTDEVICE, LPCDIDATAFORMAT);

HRESULT WINAPI extDirectInputCreate(HINSTANCE, DWORD, LPDIRECTINPUT *, LPUNKNOWN);
HRESULT WINAPI extDirectInputCreateEx(HINSTANCE, DWORD, REFIID, LPVOID *, LPUNKNOWN);
HRESULT WINAPI extDirectInput8Create(HINSTANCE, DWORD, REFIID, LPVOID *, LPUNKNOWN);
HRESULT WINAPI extDICreateDevice(LPDIRECTINPUT, REFGUID, LPDIRECTINPUTDEVICE *, LPUNKNOWN);
HRESULT WINAPI extDICreateDeviceEx(LPDIRECTINPUT, REFGUID, REFIID, LPVOID *, LPUNKNOWN);
HRESULT WINAPI extGetDeviceData(LPDIRECTINPUTDEVICE, DWORD, LPVOID, LPDWORD, DWORD);
HRESULT WINAPI extGetDeviceState(LPDIRECTINPUTDEVICE, DWORD, LPDIMOUSESTATE);
HRESULT WINAPI extDISetCooperativeLevel(LPDIRECTINPUTDEVICE, HWND, DWORD);
HRESULT WINAPI extSetDataFormat(LPDIRECTINPUTDEVICE, LPCDIDATAFORMAT);
HRESULT WINAPI extQueryInterfaceI(void *, REFIID, LPVOID *);
void GetMousePosition(int *, int *);
void InitPosition(int, int, int, int, int, int);
	
DirectInputCreate_Type pDirectInputCreate = 0;
DirectInputCreateEx_Type pDirectInputCreateEx = 0;
DICreateDevice_Type pDICreateDevice = 0;
DICreateDeviceEx_Type pDICreateDeviceEx = 0;
GetDeviceData_Type pGetDeviceData;
GetDeviceState_Type pGetDeviceState;
DISetCooperativeLevel_Type pDISetCooperativeLevel;
SetDataFormat_Type pSetDataFormat;
QueryInterface_Type pQueryInterfaceI;

int iCursorX;
int iCursorY;
int iCursorXBuf;
int iCursorYBuf;
int iCurMinX;
int iCurMinY;
int iCurMaxX;
int iCurMaxY;

int HookDirectInput(HMODULE module, int version)
{
	HINSTANCE hinst;
	void *tmp;
	LPDIRECTINPUT lpdi;
	const GUID di7 = {0x9A4CB684,0x236D,0x11D3,0x8E,0x9D,0x00,0xC0,0x4F,0x68,0x44,0xAE};
	const GUID di8 = {0xBF798030,0x483A,0x4DA2,0xAA,0x99,0x5D,0x64,0xED,0x36,0x97,0x00};

	tmp = HookAPI(module, "dinput.dll", NULL, "DirectInputCreateA", extDirectInputCreate);
	if(tmp) pDirectInputCreate = (DirectInputCreate_Type)tmp;
	tmp = HookAPI(module, "dinput.dll", NULL, "DirectInputCreateW", extDirectInputCreate);
	if(tmp) pDirectInputCreate = (DirectInputCreate_Type)tmp;
	tmp = HookAPI(module, "dinput.dll", NULL, "DirectInputCreateEx", extDirectInputCreateEx);
	if(tmp) pDirectInputCreateEx = (DirectInputCreateEx_Type)tmp;
	tmp = HookAPI(module, "dinput8.dll", NULL, "DirectInput8Create", extDirectInput8Create);
	if(tmp) pDirectInputCreateEx = (DirectInputCreateEx_Type)tmp;
	if(!pDirectInputCreate && !pDirectInputCreateEx){
		if(version < 8){
			hinst = LoadLibrary("dinput.dll");
			pDirectInputCreate =
				(DirectInputCreate_Type)GetProcAddress(hinst, "DirectInputCreateA");
			if(pDirectInputCreate)
				if(!extDirectInputCreate(GetModuleHandle(0), DIRECTINPUT_VERSION,
					&lpdi, 0)) lpdi->Release();
			pDirectInputCreateEx =
				(DirectInputCreateEx_Type)GetProcAddress(hinst, "DirectInputCreateEx");
			if(pDirectInputCreateEx)
				if(!extDirectInputCreateEx(GetModuleHandle(0), DIRECTINPUT_VERSION,
					di7, (void **)&lpdi, 0)) lpdi->Release();
		}
		else{
			hinst = LoadLibrary("dinput8.dll");
			pDirectInputCreateEx =
				(DirectInputCreateEx_Type)GetProcAddress(hinst, "DirectInput8Create");
			if(pDirectInputCreateEx)
				if(!extDirectInputCreateEx(GetModuleHandle(0), DIRECTINPUT_VERSION,
					di8, (void **)&lpdi, 0)) lpdi->Release();
		}
	}
	if(pDirectInputCreate || pDirectInputCreateEx) return 1;
	return 0;
}

HRESULT WINAPI extDirectInputCreate(HINSTANCE hinst,
	DWORD dwversion, LPDIRECTINPUT *lplpdi, LPUNKNOWN pu)
{
	HRESULT res;

	OutTraceD("DirectInputCreate: dwVersion = %x\n",
		dwversion);

	res = (*pDirectInputCreate)(hinst, dwversion, lplpdi, pu);
	if(res) return res;
	SetHook((void *)(**(DWORD **)lplpdi + 12), extDICreateDevice, (void **)&pDICreateDevice, "CreateDevice(I)");
	SetHook((void *)(**(DWORD **)lplpdi), extQueryInterfaceI, (void **)&pQueryInterfaceI, "QueryInterface(I)");
	return 0;
}

HRESULT WINAPI extDirectInputCreateEx(HINSTANCE hinst,
	DWORD dwversion, REFIID riidltf, LPVOID *ppvout, LPUNKNOWN pu)
{
	HRESULT res;

	OutTraceD("DirectInputCreateEx: dwVersion = %x REFIID = %x\n",
		dwversion, riidltf.Data1);

	res = (*pDirectInputCreateEx)(hinst, dwversion, riidltf, ppvout, pu);
	if(res) return res;
	SetHook((void *)(**(DWORD **)ppvout + 12), extDICreateDevice, (void **)&pDICreateDevice, "CreateDevice(I)");
	SetHook((void *)(**(DWORD **)ppvout + 36), extDICreateDeviceEx, (void **)pDICreateDeviceEx, "DICreateDeviceEx(I)");
	return 0;
}

HRESULT WINAPI extQueryInterfaceI(void * lpdi, REFIID riid, LPVOID *obp)
{
	HRESULT res;

	OutTraceD("lpDI->QueryInterface: REFIID = %x\n",
		riid.Data1);

	res = (*pQueryInterfaceI)(lpdi, riid, obp);
	if(res) return res;

	switch(riid.Data1){
	case 0x5944E662:		//DirectInput2A
	case 0x5944E663:		//DirectInput2W
		SetHook((void *)(**(DWORD **)obp + 12), extDICreateDevice, (void **)pDICreateDevice, "CreateDevice(I)");
		break;
	}
	return 0;
}

HRESULT WINAPI extDirectInput8Create(HINSTANCE hinst,
	DWORD dwversion, REFIID riidltf, LPVOID *ppvout, LPUNKNOWN pu)
{
	HRESULT res;

	OutTraceD("DirectInput8Create: dwVersion = %x REFIID = %x\n",
		dwversion, riidltf.Data1);

	res = (*pDirectInputCreateEx)(hinst, dwversion, riidltf, ppvout, pu);
	if(res) return res;
	SetHook((void *)(**(DWORD **)ppvout + 12), extDICreateDevice, (void **)&pDICreateDevice, "CreateDevice(I8)");
	return 0;
}

HRESULT WINAPI extDICreateDevice(LPDIRECTINPUT lpdi, REFGUID rguid,
	LPDIRECTINPUTDEVICE *lplpdid, LPUNKNOWN pu)
{
	HRESULT res;

	OutTraceD("lpDI->CreateDevice: REFGUID = %x\n",
		rguid.Data1);

	res = (*pDICreateDevice)(lpdi, rguid, lplpdid, pu);
	if(res) return res;
	SetHook((void *)(**(DWORD **)lplpdid + 36), extGetDeviceState, (void **)&pGetDeviceState, "GetDeviceState(I)");
	SetHook((void *)(**(DWORD **)lplpdid + 40), extGetDeviceData, (void **)&pGetDeviceData, "GetDeviceData(I)");
	SetHook((void *)(**(DWORD **)lplpdid + 44), extSetDataFormat, (void **)&pSetDataFormat, "SetDataFormat(I)");
	SetHook((void *)(**(DWORD **)lplpdid + 52), extDISetCooperativeLevel, (void **)&pDISetCooperativeLevel, "SetCooperativeLevel(I)");
	return 0;
}

HRESULT WINAPI extDICreateDeviceEx(LPDIRECTINPUT lpdi, REFGUID rguid,
	REFIID riid, LPVOID *pvout, LPUNKNOWN pu)
{
	HRESULT res;

	OutTraceD("lpDI->CreateDeviceEx: GUID = %x REFIID = %x\n",
		rguid.Data1, riid.Data1);

	res = (*pDICreateDeviceEx)(lpdi, rguid, riid, pvout, pu);
	if(res) return res;
	SetHook((void *)(**(DWORD **)pvout + 36), extGetDeviceState, (void **)&pGetDeviceState, "GetDeviceState(I)");
	SetHook((void *)(**(DWORD **)pvout + 40), extGetDeviceData, (void **)&pGetDeviceData, "GetDeviceData(I)");
	SetHook((void *)(**(DWORD **)pvout + 44), extSetDataFormat, (void **)&pSetDataFormat, "SetDataFormat(I)");
	SetHook((void *)(**(DWORD **)pvout + 52), extDISetCooperativeLevel, (void **)&pDISetCooperativeLevel, "SetCooperativeLevel(I)");
	return 0;
}

HRESULT WINAPI extGetDeviceData(LPDIRECTINPUTDEVICE lpdid, DWORD cbdata, LPVOID rgdod, LPDWORD pdwinout, DWORD dwflags)
{
	HRESULT res;
	BYTE *tmp;
	unsigned int i;
	POINT p;

	OutTraceD("GetDeviceData cbdata:%i\n", cbdata);

	res = (*pGetDeviceData)(lpdid, cbdata, rgdod, pdwinout, dwflags);
	if(res) return res;

	if(!dxw.bActive) *pdwinout = 0;
	GetMousePosition((int *)&p.x, (int *)&p.y);
	if(cbdata == 20 || cbdata == 24){
		tmp = (BYTE *)rgdod;
		for(i = 0; i < *pdwinout; i ++){
			if(((LPDIDEVICEOBJECTDATA)tmp)->dwOfs == DIMOFS_X){
				((LPDIDEVICEOBJECTDATA)tmp)->dwData = p.x;
				if(!dxw.bDInputAbs){
					if(p.x < iCurMinX) p.x = iCurMinX;
					if(p.x > iCurMaxX) p.x = iCurMaxX;
					((LPDIDEVICEOBJECTDATA)tmp)->dwData = p.x - iCursorXBuf;
					iCursorXBuf = p.x;
				}
			}
			if(((LPDIDEVICEOBJECTDATA)tmp)->dwOfs == DIMOFS_Y){
				((LPDIDEVICEOBJECTDATA)tmp)->dwData = p.y;
				if(!dxw.bDInputAbs){
					if(p.y < iCurMinY) p.y = iCurMinY;
					if(p.y > iCurMaxY) p.y = iCurMaxY;
					((LPDIDEVICEOBJECTDATA)tmp)->dwData = p.y - iCursorYBuf;
					iCursorYBuf = p.y;
				}
			}
			tmp += cbdata;
		}
		OutTraceD("DEBUG: directinput mousedata=(%d,%d)\n", p.x, p.y);
	}
	return 0;
}

HRESULT WINAPI extGetDeviceState(LPDIRECTINPUTDEVICE lpdid, DWORD cbdata, LPDIMOUSESTATE lpvdata)
{
	HRESULT res;
	POINT p = {0, 0};

	OutTraceD("GetDeviceState cbData:%i %i\n", cbdata, dxw.bActive);

	res = (*pGetDeviceState)(lpdid, cbdata, lpvdata);
	if(res) return res;
	if(cbdata == sizeof(DIMOUSESTATE) || cbdata == sizeof(DIMOUSESTATE2)){
		GetMousePosition((int *)&p.x, (int *)&p.y);
		lpvdata->lX = p.x;
		lpvdata->lY = p.y;
		if(!dxw.bDInputAbs){
			if(p.x < iCurMinX) p.x = iCurMinX;
			if(p.x > iCurMaxX) p.x = iCurMaxX;
			if(p.y < iCurMinY) p.y = iCurMinY;
			if(p.y > iCurMaxY) p.y = iCurMaxY;
			lpvdata->lX = p.x - iCursorX;
			lpvdata->lY = p.y - iCursorY;
			iCursorX = p.x;
			iCursorY = p.y;
		}
		if(!dxw.bActive){
			lpvdata->lZ = 0;
			*(DWORD *)lpvdata->rgbButtons = 0;
		}
		OutTraceD("DEBUG: directinput mousestate=(%d,%d)\n", p.x, p.y);
	}
	
	if(cbdata == 256 && !dxw.bActive) ZeroMemory(lpvdata, 256);
	return 0;
}
	
HRESULT WINAPI extSetDataFormat(LPDIRECTINPUTDEVICE lpdid, LPCDIDATAFORMAT lpdf)
{
	OutTraceD("SetDataFormat: flags = 0x%x\n", lpdf->dwFlags);

	if(lpdf->dwFlags & DIDF_ABSAXIS) dxw.bDInputAbs = 1;
	if(lpdf->dwFlags & DIDF_RELAXIS) dxw.bDInputAbs = 0;
	return (*pSetDataFormat)(lpdid, lpdf);
}

HRESULT WINAPI extDISetCooperativeLevel(LPDIRECTINPUTDEVICE lpdid, HWND hwnd, DWORD dwflags)
{
	OutTraceD("lpDI->SetCooperativeLevel\n");

	dwflags = DISCL_NONEXCLUSIVE | DISCL_BACKGROUND;
	return (*pDISetCooperativeLevel)(lpdid, hwnd, dwflags);
}

// Simplified version, taking in proper account the GetCursorPos API hooking & coordinate processing
void GetMousePosition(int *x, int *y)
{
	POINT p;
	//GetCursorPos(&p);
	extern BOOL WINAPI extGetCursorPos(LPPOINT);
	extGetCursorPos(&p);
	*x = p.x;
	*y = p.y;
	OutTraceD("GetMousePosition: x,y=(%d,%d)\n", *x, *y);
}

void InitPosition(int x, int y, int minx, int miny, int maxx, int maxy)
{
	iCursorX = x;
	iCursorY = y;
	iCursorXBuf = x;
	iCursorYBuf = y;
	iCurMinX = minx;
	iCurMinY = miny;
	iCurMaxX = maxx;
	iCurMaxY = maxy;
}
