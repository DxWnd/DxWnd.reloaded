#define DIRECTINPUT_VERSION 0x700
#define DIRECTINPUT8_VERSION 0x800
#define  _CRT_SECURE_NO_WARNINGS

#define RECOVERINPUTLOST TRUE // to become a flag?

#include <windows.h>
#include <dinput.h>
#include "dxwnd.h"
#include "dxhook.h"
#include "dxwcore.hpp"
#include "syslibs.h"
#include "dxhelper.h"

#ifndef DIDEVTYPE_MOUSE
#define DIDEVTYPE_DEVICE        1
#define DIDEVTYPE_MOUSE         2
#define DIDEVTYPE_KEYBOARD      3
#define DIDEVTYPE_JOYSTICK      4
#endif

extern BOOL WINAPI extGetCursorPos(LPPOINT);

#ifndef OLDDIRECTINPUTVERSION
#define OLDDIRECTINPUTVERSION 0x8007047e
#endif

/*
DEFINE_GUID(IID_IDirectInputA,     0x89521360,0xAA8A,0x11CF,0xBF,0xC7,0x44,0x45,0x53,0x54,0x00,0x00);
DEFINE_GUID(IID_IDirectInputW,     0x89521361,0xAA8A,0x11CF,0xBF,0xC7,0x44,0x45,0x53,0x54,0x00,0x00);
DEFINE_GUID(IID_IDirectInput2A,    0x5944E662,0xAA8A,0x11CF,0xBF,0xC7,0x44,0x45,0x53,0x54,0x00,0x00);
DEFINE_GUID(IID_IDirectInput2W,    0x5944E663,0xAA8A,0x11CF,0xBF,0xC7,0x44,0x45,0x53,0x54,0x00,0x00);
DEFINE_GUID(IID_IDirectInput7A,    0x9A4CB684,0x236D,0x11D3,0x8E,0x9D,0x00,0xC0,0x4F,0x68,0x44,0xAE);
DEFINE_GUID(IID_IDirectInput7W,    0x9A4CB685,0x236D,0x11D3,0x8E,0x9D,0x00,0xC0,0x4F,0x68,0x44,0xAE);
DEFINE_GUID(IID_IDirectInput8A,    0xBF798030,0x483A,0x4DA2,0xAA,0x99,0x5D,0x64,0xED,0x36,0x97,0x00);
DEFINE_GUID(IID_IDirectInput8W,    0xBF798031,0x483A,0x4DA2,0xAA,0x99,0x5D,0x64,0xED,0x36,0x97,0x00);
DEFINE_GUID(IID_IDirectInputDeviceA, 0x5944E680,0xC92E,0x11CF,0xBF,0xC7,0x44,0x45,0x53,0x54,0x00,0x00);
DEFINE_GUID(IID_IDirectInputDeviceW, 0x5944E681,0xC92E,0x11CF,0xBF,0xC7,0x44,0x45,0x53,0x54,0x00,0x00);
DEFINE_GUID(IID_IDirectInputDevice2A,0x5944E682,0xC92E,0x11CF,0xBF,0xC7,0x44,0x45,0x53,0x54,0x00,0x00);
DEFINE_GUID(IID_IDirectInputDevice2W,0x5944E683,0xC92E,0x11CF,0xBF,0xC7,0x44,0x45,0x53,0x54,0x00,0x00);
DEFINE_GUID(IID_IDirectInputDevice7A,0x57D7C6BC,0x2356,0x11D3,0x8E,0x9D,0x00,0xC0,0x4F,0x68,0x44,0xAE);
DEFINE_GUID(IID_IDirectInputDevice7W,0x57D7C6BD,0x2356,0x11D3,0x8E,0x9D,0x00,0xC0,0x4F,0x68,0x44,0xAE);
DEFINE_GUID(IID_IDirectInputDevice8A,0x54D41080,0xDC15,0x4833,0xA4,0x1B,0x74,0x8F,0x73,0xA3,0x81,0x79);
DEFINE_GUID(IID_IDirectInputDevice8W,0x54D41081,0xDC15,0x4833,0xA4,0x1B,0x74,0x8F,0x73,0xA3,0x81,0x79);
DEFINE_GUID(IID_IDirectInputEffect,  0xE7E1F7C0,0x88D2,0x11D0,0x9A,0xD0,0x00,0xA0,0xC9,0xA0,0x6E,0x35);
*/

typedef HRESULT (WINAPI *QueryInterface_Type)(void *, REFIID, LPVOID *);
typedef HRESULT (WINAPI *DirectInputCreateA_Type)(HINSTANCE, DWORD, LPDIRECTINPUTA *, LPUNKNOWN);
typedef HRESULT (WINAPI *DirectInputCreateW_Type)(HINSTANCE, DWORD, LPDIRECTINPUTW *, LPUNKNOWN);
typedef HRESULT (WINAPI *DirectInputCreateEx_Type)(HINSTANCE, DWORD, REFIID, LPVOID *, LPUNKNOWN);
typedef HRESULT (WINAPI *DICreateDeviceA_Type)(LPDIRECTINPUTA, REFGUID, LPDIRECTINPUTDEVICEA *, LPUNKNOWN);
typedef HRESULT (WINAPI *DICreateDeviceW_Type)(LPDIRECTINPUTW, REFGUID, LPDIRECTINPUTDEVICEW *, LPUNKNOWN);
typedef HRESULT (WINAPI *DICreateDeviceEx_Type)(LPDIRECTINPUT, REFGUID, REFIID, LPVOID *, LPUNKNOWN);
typedef HRESULT (WINAPI *GetDeviceData_Type)(LPDIRECTINPUTDEVICE, DWORD, LPVOID, LPDWORD, DWORD);
typedef HRESULT (WINAPI *GetDeviceState_Type)(LPDIRECTINPUTDEVICE, DWORD, LPDIMOUSESTATE);
typedef HRESULT (WINAPI *DISetCooperativeLevel_Type)(LPDIRECTINPUTDEVICE, HWND, DWORD);
typedef HRESULT (WINAPI *SetDataFormat_Type)(LPDIRECTINPUTDEVICE, LPCDIDATAFORMAT);
typedef HRESULT (WINAPI *DIEnumDevicesA_Type)(void *, DWORD, LPDIENUMDEVICESCALLBACKA, LPVOID, DWORD);
typedef HRESULT (WINAPI *DIEnumDevicesW_Type)(void *, DWORD, LPDIENUMDEVICESCALLBACKW, LPVOID, DWORD);
typedef HRESULT (WINAPI *Acquire_Type)(LPDIRECTINPUTDEVICE);
typedef HRESULT (WINAPI *Unacquire_Type)(LPDIRECTINPUTDEVICE);
typedef HRESULT (WINAPI *DirectInput8Create_Type)(HINSTANCE, DWORD, REFIID, LPVOID *, LPUNKNOWN);
//typedef HRESULT (WINAPI *DIFindDeviceA8_Type)(void *, REFGUID, LPCSTR, LPGUID);
//typedef HRESULT (WINAPI *EnumDevicesBySemantics_Type)(void *, LPCTSTR, LPDIACTIONFORMAT, LPDIENUMDEVICESBYSEMANTICSCB, LPVOID, DWORD);
 
HRESULT WINAPI extDirectInputCreateA(HINSTANCE, DWORD, LPDIRECTINPUTA *, LPUNKNOWN);
HRESULT WINAPI extDirectInputCreateW(HINSTANCE, DWORD, LPDIRECTINPUTW *, LPUNKNOWN);
HRESULT WINAPI extDirectInputCreateEx(HINSTANCE, DWORD, REFIID, LPVOID *, LPUNKNOWN);
HRESULT WINAPI extDirectInput8Create(HINSTANCE, DWORD, REFIID, LPVOID *, LPUNKNOWN);
HRESULT WINAPI extDICreateDeviceA1(LPDIRECTINPUTA, REFGUID, LPDIRECTINPUTDEVICEA *, LPUNKNOWN);
HRESULT WINAPI extDICreateDeviceA2(LPDIRECTINPUTA, REFGUID, LPDIRECTINPUTDEVICEA *, LPUNKNOWN);
HRESULT WINAPI extDICreateDeviceA7(LPDIRECTINPUTA, REFGUID, LPDIRECTINPUTDEVICEA *, LPUNKNOWN);
HRESULT WINAPI extDICreateDeviceA8(LPDIRECTINPUTA, REFGUID, LPDIRECTINPUTDEVICEA *, LPUNKNOWN);
HRESULT WINAPI extDICreateDeviceW1(LPDIRECTINPUTW, REFGUID, LPDIRECTINPUTDEVICEW *, LPUNKNOWN);
HRESULT WINAPI extDICreateDeviceW2(LPDIRECTINPUTW, REFGUID, LPDIRECTINPUTDEVICEW *, LPUNKNOWN);
HRESULT WINAPI extDICreateDeviceW7(LPDIRECTINPUTW, REFGUID, LPDIRECTINPUTDEVICEW *, LPUNKNOWN);
HRESULT WINAPI extDICreateDeviceW8(LPDIRECTINPUTW, REFGUID, LPDIRECTINPUTDEVICEW *, LPUNKNOWN);
HRESULT WINAPI extDICreateDeviceExA(LPDIRECTINPUTA, REFGUID, REFIID, LPVOID *, LPUNKNOWN);
HRESULT WINAPI extDICreateDeviceExW(LPDIRECTINPUTW, REFGUID, REFIID, LPVOID *, LPUNKNOWN);
HRESULT WINAPI extGetDeviceData(LPDIRECTINPUTDEVICE, DWORD, LPVOID, LPDWORD, DWORD);
HRESULT WINAPI extGetDeviceState(LPDIRECTINPUTDEVICE, DWORD, LPDIMOUSESTATE);
HRESULT WINAPI extDISetCooperativeLevel(LPDIRECTINPUTDEVICE, HWND, DWORD);
HRESULT WINAPI extSetDataFormat(LPDIRECTINPUTDEVICE, LPCDIDATAFORMAT);
HRESULT WINAPI extDIQueryInterface(void *, REFIID, LPVOID *);
HRESULT WINAPI extDIEnumDevicesA1(void *, DWORD, LPDIENUMDEVICESCALLBACKA, LPVOID, DWORD);
HRESULT WINAPI extDIEnumDevicesA2(void *, DWORD, LPDIENUMDEVICESCALLBACKA, LPVOID, DWORD);
HRESULT WINAPI extDIEnumDevicesA7(void *, DWORD, LPDIENUMDEVICESCALLBACKA, LPVOID, DWORD);
HRESULT WINAPI extDIEnumDevicesA8(void *, DWORD, LPDIENUMDEVICESCALLBACKA, LPVOID, DWORD);
HRESULT WINAPI extDIEnumDevicesW1(void *, DWORD, LPDIENUMDEVICESCALLBACKW, LPVOID, DWORD);
HRESULT WINAPI extDIEnumDevicesW2(void *, DWORD, LPDIENUMDEVICESCALLBACKW, LPVOID, DWORD);
HRESULT WINAPI extDIEnumDevicesW7(void *, DWORD, LPDIENUMDEVICESCALLBACKW, LPVOID, DWORD);
HRESULT WINAPI extDIEnumDevicesW8(void *, DWORD, LPDIENUMDEVICESCALLBACKW, LPVOID, DWORD);
HRESULT WINAPI extAcquire(LPDIRECTINPUTDEVICE);
HRESULT WINAPI extUnacquire(LPDIRECTINPUTDEVICE);
HRESULT WINAPI extDirectInput8Create(HINSTANCE, DWORD, REFIID, LPVOID *, LPUNKNOWN);
//HRESULT WINAPI extDIFindDeviceA8(void *, REFGUID, LPCSTR, LPGUID);
//HRESULT WINAPI extEnumDevicesBySemantics(void *, LPCTSTR, LPDIACTIONFORMAT, LPDIENUMDEVICESBYSEMANTICSCB, LPVOID, DWORD);

DirectInputCreateA_Type pDirectInputCreateA = NULL;
DirectInputCreateW_Type pDirectInputCreateW = NULL;
DirectInputCreateEx_Type pDirectInputCreateEx;
DICreateDeviceA_Type pDICreateDeviceA1, pDICreateDeviceA2, pDICreateDeviceA7, pDICreateDeviceA8;
DICreateDeviceW_Type pDICreateDeviceW1, pDICreateDeviceW2, pDICreateDeviceW7, pDICreateDeviceW8;
DICreateDeviceEx_Type pDICreateDeviceExA, pDICreateDeviceExW;
//DICreateDeviceEx_Type pDICreateDeviceEx;
GetDeviceData_Type pGetDeviceData = NULL;
GetDeviceState_Type pGetDeviceState = NULL;
DISetCooperativeLevel_Type pDISetCooperativeLevel = NULL;
SetDataFormat_Type pSetDataFormat = NULL;
QueryInterface_Type pDIQueryInterface = NULL;
DIEnumDevicesA_Type pDIEnumDevicesA1, pDIEnumDevicesA2, pDIEnumDevicesA7, pDIEnumDevicesA8;
DIEnumDevicesW_Type pDIEnumDevicesW1, pDIEnumDevicesW2, pDIEnumDevicesW7, pDIEnumDevicesW8;
Acquire_Type pAcquire = NULL;
Unacquire_Type pUnacquire = NULL;
DirectInput8Create_Type pDirectInput8Create = NULL;
//DIFindDeviceA8_Type pDIFindDeviceA8 = NULL;
//EnumDevicesBySemantics_Type pEnumDevicesBySemantics = NULL;

static HookEntryEx_Type diHooks[]={
	{HOOK_HOT_CANDIDATE, 0, "DirectInputCreateA", (FARPROC)NULL, (FARPROC *)&pDirectInputCreateA, (FARPROC)extDirectInputCreateA},
	{HOOK_HOT_CANDIDATE, 0, "DirectInputCreateW", (FARPROC)NULL, (FARPROC *)&pDirectInputCreateW, (FARPROC)extDirectInputCreateW},
	{HOOK_HOT_CANDIDATE, 0, "DirectInputCreateEx", (FARPROC)NULL, (FARPROC *)&pDirectInputCreateEx, (FARPROC)extDirectInputCreateEx},
	{HOOK_IAT_CANDIDATE, 0, 0, NULL, 0, 0} // terminator
};

static HookEntryEx_Type di8Hooks[]={
	{HOOK_HOT_CANDIDATE, 0, "DirectInput8Create", (FARPROC)NULL, (FARPROC *)&pDirectInput8Create, (FARPROC)extDirectInput8Create},
	{HOOK_IAT_CANDIDATE, 0, 0, NULL, 0, 0} // terminator
};

void GetMousePosition(int *, int *);
void InitPosition(int, int, int, int, int, int);

int iCursorX;
int iCursorY;
int iCursorXBuf;
int iCursorYBuf;
int iCurMinX;
int iCurMinY;
int iCurMaxX;
int iCurMaxY;

LPDIRECTINPUTDEVICE lpDIDDevice  = NULL;
LPDIRECTINPUTDEVICE lpDIDKeyboard = NULL;
LPDIRECTINPUTDEVICE lpDIDSysMouse = NULL;
LPDIRECTINPUTDEVICE lpDIDJoystick = NULL;

static char *sDevice(LPDIRECTINPUTDEVICE lpdid)
{
	char *ret;
	ret = NULL;
	if(lpdid==lpDIDDevice) ret = "Device";
	if(lpdid==lpDIDKeyboard) ret = "Keyboard";
	if(lpdid==lpDIDSysMouse) ret = "Mouse";
	if(lpdid==lpDIDJoystick) ret = "Joystick";
	if (ret) 
		return ret;
	else
		return (lpdid ? "unknown" : "NULL");
}

void HookDirectInput(HMODULE module)
{
	const GUID di7 = {0x9A4CB684,0x236D,0x11D3,0x8E,0x9D,0x00,0xC0,0x4F,0x68,0x44,0xAE};
	HINSTANCE hinst;
	LPDIRECTINPUT lpdi;

	if(!(dxw.dwFlags1 & HOOKDI)) return;

	HookLibraryEx(module, diHooks, "dinput.dll");
	if(!pDirectInputCreateA && !pDirectInputCreateW && !pDirectInputCreateEx){
		hinst = LoadLibrary("dinput.dll");
		if(!hinst) {
			OutTraceE("LoadLibrary dinput.dll ERROR err=%d at %d\n", GetLastError(), __LINE__);
			return;
		}
		pDirectInputCreateA = (DirectInputCreateA_Type)GetProcAddress(hinst, "DirectInputCreateA");
		if(pDirectInputCreateA)
			if(!extDirectInputCreateA(GetModuleHandle(0), DIRECTINPUT_VERSION,
				&lpdi, 0)) lpdi->Release();
		pDirectInputCreateEx = (DirectInputCreateEx_Type)GetProcAddress(hinst, "DirectInputCreateEx");
		if(pDirectInputCreateEx)
			if(!extDirectInputCreateEx(GetModuleHandle(0), DIRECTINPUT_VERSION,
				di7, (void **)&lpdi, 0)) lpdi->Release();
	}
}

void HookDirectInput8(HMODULE module)
{
	const GUID di8 = {0xBF798030,0x483A,0x4DA2,0xAA,0x99,0x5D,0x64,0xED,0x36,0x97,0x00};
	HINSTANCE hinst;
	LPDIRECTINPUT lpdi;

	if(!(dxw.dwFlags1 & HOOKDI8)) return;

	HookLibraryEx(module, di8Hooks, "dinput8.dll");
	if(!pDirectInput8Create){
		hinst = LoadLibrary("dinput8.dll");
		if(!hinst) {
			OutTraceE("LoadLibrary dinput8.dll ERROR err=%d at %d\n", GetLastError(), __LINE__);
			return;
		}
		pDirectInput8Create = (DirectInput8Create_Type)GetProcAddress(hinst, "DirectInput8Create");
		if(pDirectInput8Create)
			if(!extDirectInput8Create(GetModuleHandle(0), DIRECTINPUT8_VERSION,
				di8, (LPVOID *)&lpdi, 0)) lpdi->Release();
	}
}

FARPROC Remap_DInput_ProcAddress(LPCSTR proc, HMODULE hModule)
{
	FARPROC addr;
	if ((dxw.dwFlags1 & HOOKDI) && (addr=RemapLibraryEx(proc, hModule, diHooks))) return addr;
	return NULL;
}

FARPROC Remap_DInput8_ProcAddress(LPCSTR proc, HMODULE hModule)
{
	FARPROC addr;
	if ((dxw.dwFlags1 & HOOKDI8) && (addr=RemapLibraryEx(proc, hModule, di8Hooks))) return addr;
	return NULL;
}

const GUID FAR IID_IDirectInput8A = { 0xBF798030,0x483A,0x4DA2,0xAA,0x99,0x5D,0x64,0xED,0x36,0x97,0x00 };
const GUID FAR IID_IDirectInput8W = { 0xBF798031,0x483A,0x4DA2,0xAA,0x99,0x5D,0x64,0xED,0x36,0x97,0x00 };
#define OLDDIVERSION FALSE

HRESULT WINAPI extDirectInputCreateA(HINSTANCE hinst, DWORD dwversion, LPDIRECTINPUT *lplpdi, LPUNKNOWN pu)
{
	HRESULT res;

	OutTraceDW("DirectInputCreateA(%x)\n", dwversion);

	res = (*pDirectInputCreateA)(hinst, dwversion, lplpdi, pu);
	if(res) {
		if ((res == OLDDIRECTINPUTVERSION) && (dwversion == 0x800) && OLDDIVERSION){
			OutTraceE("DirectInputCreateA: ERROR err=OLDDIRECTINPUTVERSION version=800 try to call DirectInput8 interface\n");
			if(!pDirectInput8Create) {
				HMODULE hDI8;
				hDI8 = (*pLoadLibraryA)("dinput8.dll");
				pDirectInput8Create = (DirectInput8Create_Type)(*pGetProcAddress)(hDI8, "DirectInput8Create");
			}
			if(pDirectInput8Create){
				res = extDirectInput8Create(hinst, dwversion, IID_IDirectInput8A, (LPVOID *)lplpdi, pu);
				return res;
			}
		}
		OutTraceE("DirectInputCreateA: ERROR err=%x(%s)\n", res, ExplainDDError(res));
		return res;
	}
	SetHook((void *)(**(DWORD **)lplpdi), extDIQueryInterface, (void **)&pDIQueryInterface, "QueryInterface(I)");
	SetHook((void *)(**(DWORD **)lplpdi + 12), extDICreateDeviceA1, (void **)&pDICreateDeviceA1, "CreateDevice(I)");
	SetHook((void *)(**(DWORD **)lplpdi + 16), extDIEnumDevicesA1, (void **)&pDIEnumDevicesA1, "EnumDevices(I)");
	return 0;
}

HRESULT WINAPI extDirectInputCreateW(HINSTANCE hinst, DWORD dwversion, LPDIRECTINPUTW *lplpdi, LPUNKNOWN pu)
{
	HRESULT res;

	OutTraceDW("DirectInputCreateW(%x)\n", dwversion);

	res = (*pDirectInputCreateW)(hinst, dwversion, lplpdi, pu);
	if(res) {
		if ((res == OLDDIRECTINPUTVERSION) && (dwversion == 0x800) && OLDDIVERSION){
			OutTraceE("DirectInputCreateA: ERROR err=OLDDIRECTINPUTVERSION version=800 try to call DirectInput8 interface\n");
			if(!pDirectInput8Create) {
				HMODULE hDI8;
				hDI8 = (*pLoadLibraryA)("dinput8.dll");
				pDirectInput8Create = (DirectInput8Create_Type)(*pGetProcAddress)(hDI8, "DirectInput8Create");
			}
			if(pDirectInput8Create){
				res = extDirectInput8Create(hinst, dwversion, IID_IDirectInput8W, (LPVOID *)lplpdi, pu);
				return res;
			}
		}
		OutTraceE("DirectInputCreateW: ERROR err=%x(%s)\n", res, ExplainDDError(res));
		return res;
	}
	SetHook((void *)(**(DWORD **)lplpdi), extDIQueryInterface, (void **)&pDIQueryInterface, "QueryInterface(I)");
	SetHook((void *)(**(DWORD **)lplpdi + 12), extDICreateDeviceW1, (void **)&pDICreateDeviceW1, "CreateDevice(IW1)");
	SetHook((void *)(**(DWORD **)lplpdi + 16), extDIEnumDevicesW1, (void **)&pDIEnumDevicesW1, "EnumDevices(IW1)");
	return 0;
}

HRESULT WINAPI extDirectInputCreateEx(HINSTANCE hinst, DWORD dwversion, REFIID riidltf, LPVOID *ppvout, LPUNKNOWN pu)
{
	HRESULT res;

	OutTraceDW("DirectInputCreateEx: dwVersion=%x REFIID=%x(%s)\n",
		dwversion, riidltf.Data1, ExplainGUID((GUID *)&riidltf));

	res = (*pDirectInputCreateEx)(hinst, dwversion, riidltf, ppvout, pu);
	if(res) {
		if ((res == OLDDIRECTINPUTVERSION) && (dwversion == 0x800) && OLDDIVERSION){
			OutTraceE("DirectInputCreateA: ERROR err=OLDDIRECTINPUTVERSION version=800 try to call DirectInput8 interface\n");
			if(!pDirectInput8Create) {
				HMODULE hDI8;
				hDI8 = (*pLoadLibraryA)("dinput8.dll");
				pDirectInput8Create = (DirectInput8Create_Type)(*pGetProcAddress)(hDI8, "DirectInput8Create");
			}
			if(pDirectInput8Create){
				switch (riidltf.Data1){
					case 0x9A4CB684: res = extDirectInput8Create(hinst, dwversion, IID_IDirectInput8A, ppvout, pu); break;
					case 0x9A4CB685: res = extDirectInput8Create(hinst, dwversion, IID_IDirectInput8W, ppvout, pu); break;
				}
				return res;
			}
		}
		OutTraceE("DirectInputCreateEx: ERROR err=%x(%s)\n", res, ExplainDDError(res));
		return res;
	}
	switch (riidltf.Data1){
		case 0x9A4CB684: 
			SetHook((void *)(**(DWORD **)ppvout + 12), extDICreateDeviceA7, (void **)&pDICreateDeviceA7, "CreateDevice(IA7)");
			SetHook((void *)(**(DWORD **)ppvout + 16), extDIEnumDevicesA7, (void **)&pDIEnumDevicesA7, "EnumDevices(IA7)");
			if(dwversion >= 700)
				SetHook((void *)(**(DWORD **)ppvout + 36), extDICreateDeviceExA, (void **)&pDICreateDeviceExA, "CreateDeviceEx(IA7)");
			break;
		case 0x9A4CB685:
				SetHook((void *)(**(DWORD **)ppvout + 12), extDICreateDeviceW7, (void **)&pDICreateDeviceA7, "CreateDevice(IA7)");
				SetHook((void *)(**(DWORD **)ppvout + 16), extDIEnumDevicesW7, (void **)&pDIEnumDevicesA7, "EnumDevices(IA7)");
				if(dwversion >= 700)
					SetHook((void *)(**(DWORD **)ppvout + 36), extDICreateDeviceExW, (void **)&pDICreateDeviceExA, "CreateDeviceEx(IA7)");
		break;
	}
	return res;
}

HRESULT WINAPI extDIQueryInterface(void * lpdi, REFIID riid, LPVOID *obp)
{
	HRESULT res;

	OutTraceDW("QueryInterface(I): REFIID=%x(%s)\n",
		riid.Data1, ExplainGUID((GUID *)&riid));

	res = (*pDIQueryInterface)(lpdi, riid, obp);
	if(res) return res;

	switch(riid.Data1){
	case 0x89521360:		//DirectInputA
		SetHook((void *)(**(DWORD **)obp + 12), extDICreateDeviceA1, (void **)&pDICreateDeviceA1, "CreateDevice(IA1)");
		SetHook((void *)(**(DWORD **)obp + 16), extDIEnumDevicesA1, (void **)&pDIEnumDevicesA1, "EnumDevices(IA1)");
		break;
	case 0x89521361:		//DirectInputW
		SetHook((void *)(**(DWORD **)obp + 12), extDICreateDeviceW1, (void **)&pDICreateDeviceW1, "CreateDevice(IW1)");
		SetHook((void *)(**(DWORD **)obp + 16), extDIEnumDevicesW1, (void **)&pDIEnumDevicesW1, "EnumDevices(IW1)");
		break;
	case 0x5944E662:		//DirectInput2A
		SetHook((void *)(**(DWORD **)obp + 12), extDICreateDeviceA2, (void **)&pDICreateDeviceA2, "CreateDevice(IA2)");
		SetHook((void *)(**(DWORD **)obp + 16), extDIEnumDevicesA2, (void **)&pDIEnumDevicesA2, "EnumDevices(IA2)");
		break;
	case 0x5944E663:		//DirectInput2W
		SetHook((void *)(**(DWORD **)obp + 12), extDICreateDeviceW2, (void **)&pDICreateDeviceW2, "CreateDevice(IW2)");
		SetHook((void *)(**(DWORD **)obp + 16), extDIEnumDevicesW2, (void **)&pDIEnumDevicesW2, "EnumDevices(IW2)");
		break;
	case 0x9A4CB684:		//IDirectInput7A
		SetHook((void *)(**(DWORD **)obp + 12), extDICreateDeviceA7, (void **)&pDICreateDeviceA7, "CreateDevice(IA7)");
		SetHook((void *)(**(DWORD **)obp + 16), extDIEnumDevicesA7, (void **)&pDIEnumDevicesA7, "EnumDevices(IA7)");
		SetHook((void *)(**(DWORD **)obp + 36), extDICreateDeviceExA, (void **)&pDICreateDeviceExA, "CreateDeviceEx(IA7)");
	case 0x9A4CB685:		//IDirectInput7W
		SetHook((void *)(**(DWORD **)obp + 12), extDICreateDeviceW7, (void **)&pDICreateDeviceW7, "CreateDevice(IW7)");
		SetHook((void *)(**(DWORD **)obp + 16), extDIEnumDevicesW7, (void **)&pDIEnumDevicesW7, "EnumDevices(IW7)");
		SetHook((void *)(**(DWORD **)obp + 36), extDICreateDeviceExW, (void **)&pDICreateDeviceExW, "CreateDeviceEx(IW7)");
		break;
	}
	return 0;
}

HRESULT WINAPI extDirectInput8Create(HINSTANCE hinst, DWORD dwversion, REFIID riidltf, LPVOID *ppvout, LPUNKNOWN pu)
{
	HRESULT res;

	OutTraceDW("DirectInput8Create: dwVersion=%x REFIID=%x(%s)\n",
		dwversion, riidltf.Data1, ExplainGUID((GUID *)&riidltf));

	res = (*pDirectInput8Create)(hinst, dwversion, riidltf, ppvout, pu);
	if(res) {
		OutTraceE("DirectInput8Create: ERROR res=%x\n", res);
		return res;
	}
	OutTraceDW("DirectInput8Create: di=%x\n", *ppvout);
	switch(riidltf.Data1){
		case 0xBF798030:
			SetHook((void *)(**(DWORD **)ppvout + 12), extDICreateDeviceA8, (void **)&pDICreateDeviceA8, "CreateDevice(IA8)");
			SetHook((void *)(**(DWORD **)ppvout + 16), extDIEnumDevicesA8, (void **)&pDIEnumDevicesA8, "EnumDevices(IA8)");
			//SetHook((void *)(**(DWORD **)ppvout + 32), extDIFindDeviceA8, (void **)&pDIFindDeviceA8, "FindDevice(IA8)");
			break;
		case 0xBF798031:
			SetHook((void *)(**(DWORD **)ppvout + 12), extDICreateDeviceW8, (void **)&pDICreateDeviceW8, "CreateDevice(IW8)");
			SetHook((void *)(**(DWORD **)ppvout + 16), extDIEnumDevicesW8, (void **)&pDIEnumDevicesW8, "EnumDevices(IW8)");
			//SetHook((void *)(**(DWORD **)ppvout + 32), extDIFindDeviceW8, (void **)&pDIFindDeviceW8, "FindDevice(IW8)");
			break;
		default:
			MessageBox(0, "Bad DirectInput REFIID", "warning", 0);
			break;
	}
	//SetHook((void *)(**(DWORD **)ppvout + 36), extEnumDevicesBySemantics, (void **)&pEnumDevicesBySemantics, "EnumDevicesBySemantics(I8)");
	return 0;
}

static char *sDeviceType(REFGUID rguid)
{
	char *devtype;
	switch(rguid.Data1){
		case 0x6F1D2B60: devtype = "SysMouse"; break;
		case 0x6F1D2B61: devtype = "SysKeyboard"; break;
		case 0x6F1D2B70: devtype = "Joystick"; break;
		case 0x6F1D2B80: devtype = "SysMouseEm"; break;
		case 0x6F1D2B81: devtype = "SysMouseEm2"; break;
		case 0x6F1D2B82: devtype = "SysKeyboardEm"; break;
		case 0x6F1D2B83: devtype = "SysKeyboardEm2"; break;
		default: devtype = "Unknown"; break;
	}
	return devtype;
}

static int iDeviceType(REFGUID rguid)
{
	int devtype;
	switch(rguid.Data1){
		case 0x6F1D2B60: devtype = DIDEVTYPE_MOUSE; break;
		case 0x6F1D2B61: devtype = DIDEVTYPE_KEYBOARD; break;
		case 0x6F1D2B70: devtype = DIDEVTYPE_JOYSTICK; break;
		default: devtype = 0; break;
	}
	return devtype;
}

HRESULT WINAPI extDICreateDeviceA(DICreateDeviceA_Type pDICreateDevice, LPDIRECTINPUT lpdi, REFGUID rguid, LPDIRECTINPUTDEVICEA *lplpdid, LPUNKNOWN pu)
{
	HRESULT res;

	OutTraceDW("CreateDeviceA(I): REFGUID=%x(%s)\n", rguid.Data1, sDeviceType(rguid));

	res = (*pDICreateDevice)(lpdi, rguid, lplpdid, pu);
	if(res) {
		OutTraceE("CreateDeviceA(I): ERROR res=%x\n", res);
		return res;
	}

	OutTraceDW("CreateDevice(I): did=%x\n", *lplpdid);
	SetHook((void *)(**(DWORD **)lplpdid + 28), extAcquire, (void **)&pAcquire, "Acquire(I)");
	SetHook((void *)(**(DWORD **)lplpdid + 32), extUnacquire, (void **)&pUnacquire, "Unacquire(I)");
	SetHook((void *)(**(DWORD **)lplpdid + 36), extGetDeviceState, (void **)&pGetDeviceState, "GetDeviceState(I)");
	SetHook((void *)(**(DWORD **)lplpdid + 40), extGetDeviceData, (void **)&pGetDeviceData, "GetDeviceData(I)");
	SetHook((void *)(**(DWORD **)lplpdid + 44), extSetDataFormat, (void **)&pSetDataFormat, "SetDataFormat(I)");
	SetHook((void *)(**(DWORD **)lplpdid + 52), extDISetCooperativeLevel, (void **)&pDISetCooperativeLevel, "SetCooperativeLevel(I)");

	switch(iDeviceType(rguid)){
		case DIDEVTYPE_DEVICE:	 lpDIDDevice   = *lplpdid; break;
		case DIDEVTYPE_MOUSE:	 lpDIDSysMouse = *lplpdid; break;
		case DIDEVTYPE_KEYBOARD: lpDIDKeyboard = *lplpdid; break;
		case DIDEVTYPE_JOYSTICK: lpDIDJoystick = *lplpdid; break;
	}
	return DI_OK;
}

HRESULT WINAPI extDICreateDeviceA1(LPDIRECTINPUT lpdi, REFGUID rguid, LPDIRECTINPUTDEVICEA *lplpdid, LPUNKNOWN pu)
{ return extDICreateDeviceA(pDICreateDeviceA1, lpdi, rguid, lplpdid, pu); }
HRESULT WINAPI extDICreateDeviceA2(LPDIRECTINPUT lpdi, REFGUID rguid, LPDIRECTINPUTDEVICEA *lplpdid, LPUNKNOWN pu)
{ return extDICreateDeviceA(pDICreateDeviceA2, lpdi, rguid, lplpdid, pu); }
HRESULT WINAPI extDICreateDeviceA7(LPDIRECTINPUT lpdi, REFGUID rguid, LPDIRECTINPUTDEVICEA *lplpdid, LPUNKNOWN pu)
{ return extDICreateDeviceA(pDICreateDeviceA7, lpdi, rguid, lplpdid, pu); }
HRESULT WINAPI extDICreateDeviceA8(LPDIRECTINPUT lpdi, REFGUID rguid, LPDIRECTINPUTDEVICEA *lplpdid, LPUNKNOWN pu)
{ return extDICreateDeviceA(pDICreateDeviceA8, lpdi, rguid, lplpdid, pu); }

HRESULT WINAPI extDICreateDeviceW(DICreateDeviceW_Type pDICreateDevice, LPDIRECTINPUTW lpdi, REFGUID rguid, LPDIRECTINPUTDEVICEW *lplpdid, LPUNKNOWN pu)
{
	HRESULT res;

	OutTraceDW("CreateDeviceW(I): REFGUID=%x(%s)\n", rguid.Data1, sDeviceType(rguid));

	res = (*pDICreateDevice)(lpdi, rguid, lplpdid, pu);
	if(res) {
		OutTraceE("CreateDeviceW(I): ERROR res=%x\n", res);
		return res;
	}

	OutTraceDW("CreateDevice(I): did=%x\n", *lplpdid);
	SetHook((void *)(**(DWORD **)lplpdid + 28), extAcquire, (void **)&pAcquire, "Acquire(I)");
	SetHook((void *)(**(DWORD **)lplpdid + 32), extUnacquire, (void **)&pUnacquire, "Unacquire(I)");
	SetHook((void *)(**(DWORD **)lplpdid + 36), extGetDeviceState, (void **)&pGetDeviceState, "GetDeviceState(I)");
	SetHook((void *)(**(DWORD **)lplpdid + 40), extGetDeviceData, (void **)&pGetDeviceData, "GetDeviceData(I)");
	SetHook((void *)(**(DWORD **)lplpdid + 44), extSetDataFormat, (void **)&pSetDataFormat, "SetDataFormat(I)");
	SetHook((void *)(**(DWORD **)lplpdid + 52), extDISetCooperativeLevel, (void **)&pDISetCooperativeLevel, "SetCooperativeLevel(I)");

	switch(iDeviceType(rguid)){
		case DIDEVTYPE_DEVICE:	 lpDIDDevice   = (LPDIRECTINPUTDEVICE)*lplpdid; break;
		case DIDEVTYPE_MOUSE:	 lpDIDSysMouse = (LPDIRECTINPUTDEVICE)*lplpdid; break;
		case DIDEVTYPE_KEYBOARD: lpDIDKeyboard = (LPDIRECTINPUTDEVICE)*lplpdid; break;
		case DIDEVTYPE_JOYSTICK: lpDIDJoystick = (LPDIRECTINPUTDEVICE)*lplpdid; break;
	}
	return DI_OK;
}

HRESULT WINAPI extDICreateDeviceW1(LPDIRECTINPUTW lpdi, REFGUID rguid, LPDIRECTINPUTDEVICEW *lplpdid, LPUNKNOWN pu)
{ return extDICreateDeviceW(pDICreateDeviceW1, lpdi, rguid, lplpdid, pu); }
HRESULT WINAPI extDICreateDeviceW2(LPDIRECTINPUTW lpdi, REFGUID rguid, LPDIRECTINPUTDEVICEW *lplpdid, LPUNKNOWN pu)
{ return extDICreateDeviceW(pDICreateDeviceW2, lpdi, rguid, lplpdid, pu); }
HRESULT WINAPI extDICreateDeviceW7(LPDIRECTINPUTW lpdi, REFGUID rguid, LPDIRECTINPUTDEVICEW *lplpdid, LPUNKNOWN pu)
{ return extDICreateDeviceW(pDICreateDeviceW7, lpdi, rguid, lplpdid, pu); }
HRESULT WINAPI extDICreateDeviceW8(LPDIRECTINPUTW lpdi, REFGUID rguid, LPDIRECTINPUTDEVICEW *lplpdid, LPUNKNOWN pu)
{ return extDICreateDeviceW(pDICreateDeviceW8, lpdi, rguid, lplpdid, pu); }

HRESULT WINAPI extDICreateDeviceEx(LPDIRECTINPUT lpdi, REFGUID rguid,
	REFIID riid, LPVOID *pvout, LPUNKNOWN pu)
{
	HRESULT res;

	OutTraceDW("CreateDeviceEx(I): GUID=%x(%s) REFIID=%x\n", rguid.Data1, sDeviceType(rguid), riid.Data1);

	res = (*pDICreateDeviceExA)(lpdi, rguid, riid, pvout, pu);
	if(res) {
		OutTraceE("CreateDeviceEx(I): ERROR res=%x\n", res);
		return res;
	}
	OutTraceDW("CreateDeviceEx(I): did=%x\n", *pvout);
	SetHook((void *)(**(DWORD **)pvout + 28), extAcquire, (void **)&pAcquire, "Acquire(I)");
	SetHook((void *)(**(DWORD **)pvout + 32), extUnacquire, (void **)&pUnacquire, "Unacquire(I)");
	SetHook((void *)(**(DWORD **)pvout + 36), extGetDeviceState, (void **)&pGetDeviceState, "GetDeviceState(I)");
	SetHook((void *)(**(DWORD **)pvout + 40), extGetDeviceData, (void **)&pGetDeviceData, "GetDeviceData(I)");
	SetHook((void *)(**(DWORD **)pvout + 44), extSetDataFormat, (void **)&pSetDataFormat, "SetDataFormat(I)");
	SetHook((void *)(**(DWORD **)pvout + 52), extDISetCooperativeLevel, (void **)&pDISetCooperativeLevel, "SetCooperativeLevel(I)");

	switch(iDeviceType(rguid)){
		case DIDEVTYPE_DEVICE:	 lpDIDDevice   = *(LPDIRECTINPUTDEVICE *)pvout; break;
		case DIDEVTYPE_MOUSE:	 lpDIDSysMouse = *(LPDIRECTINPUTDEVICE *)pvout; break;
		case DIDEVTYPE_KEYBOARD: lpDIDKeyboard = *(LPDIRECTINPUTDEVICE *)pvout; break;
		case DIDEVTYPE_JOYSTICK: lpDIDJoystick = *(LPDIRECTINPUTDEVICE *)pvout; break;
	}
	return DI_OK;
}

// to do
HRESULT WINAPI extDICreateDeviceExA(LPDIRECTINPUTA lpdi, REFGUID rguid, REFIID riid, LPVOID *pvout, LPUNKNOWN pu)
{ return extDICreateDeviceEx(lpdi, rguid, riid, pvout, pu); }
HRESULT WINAPI extDICreateDeviceExW(LPDIRECTINPUTW lpdi, REFGUID rguid, REFIID riid, LPVOID *pvout, LPUNKNOWN pu)
{ return extDICreateDeviceEx((LPDIRECTINPUT)lpdi, rguid, riid, pvout, pu); }


/* from MSDN:
cbObjectData
    Size of the DIDEVICEOBJECTDATA structure, in bytes. 
rgdod
    Array of DIDEVICEOBJECTDATA structures to receive the buffered data. 
	The number of elements in this array must be equal to the value of the pdwInOut parameter. 
	If this parameter is NULL, the buffered data is not stored anywhere, but all other side effects take place. 
pdwInOut
    On entry, the number of elements in the array pointed to by the rgdod parameter. 
	On exit, the number of elements actually obtained. 
dwFlags
    Flags that control the manner in which data is obtained. 
	This value can be 0 or the following flag.

    DIGDD_PEEK
        Do not remove the items from the buffer. A subsequent IDirectInputDevice8::GetDeviceData call will read the same data. 
		Normally, data is removed from the buffer after it is read.
*/

/* Mind the following scenarios!
Your application can query for the number of elements in the device buffer by setting the rgdod parameter to NULL, setting pdwInOut to INFINITE 
and setting dwFlags to DIGDD_PEEK. The following code example illustrates how this can be done.

dwItems = INFINITE; 
hres = idirectinputdevice9_GetDeviceData( 
            pdid, 
            sizeof(DIDEVICEOBJECTDATA), 
            NULL, 
            &dwItems, 
            DIGDD_PEEK); 
if (SUCCEEDED(hres)) { 
    // dwItems = Number of elements in buffer. 
    if (hres == DI_BUFFEROVERFLOW) { 
        // Buffer overflow occurred; not all data 
        //   was successfully captured. 
    } 
} 

To query about whether a buffer overflow has occurred, set the rgdod parameter to NULL and the pdwInOut parameter to 0. 
The following code example illustrates how this can be done.

dwItems = 0; 
hres = idirectinputdevice9_GetDeviceData( 
            pdid, 
            sizeof(DIDEVICEOBJECTDATA), 
            NULL, 
            &dwItems, 
            0); 
if (hres == DI_BUFFEROVERFLOW) { 
    // Buffer overflow occurred. 
} 
*/

HRESULT WINAPI extGetDeviceData(LPDIRECTINPUTDEVICE lpdid, DWORD cbdata, LPVOID rgdod, LPDWORD pdwinout, DWORD dwflags)
{
	HRESULT res;
	BYTE *tmp;
	unsigned int i;
	POINT p;

	OutTraceDW("GetDeviceData(I): did=%x(%s) cbdata=%i rgdod=%x, inout=%d flags=%x\n", 
		lpdid, sDevice(lpdid), cbdata, rgdod, *pdwinout, dwflags);

	res = (*pGetDeviceData)(lpdid, cbdata, rgdod, pdwinout, dwflags);

	if (((res == DIERR_INPUTLOST) || (res == DIERR_NOTACQUIRED)) && RECOVERINPUTLOST){
		OutTraceE("GetDeviceState(I) recovering DIERR_INPUTLOST\n"); 
		if(lpdid == lpDIDSysMouse) {
			res = (*pDISetCooperativeLevel)(lpdid, dxw.GethWnd(), DISCL_NONEXCLUSIVE | DISCL_FOREGROUND);
			if(res) OutTraceE("GetDeviceState(I): SetCooperativeLevel ERROR: err=%x(%s)\n", res, ExplainDDError(res)); 
			if(dxw.dwFlags1 & CLIPCURSOR) dxw.SetClipCursor();
		}
		res = (*pAcquire)(lpdid);
		if(res) OutTraceE("GetDeviceState(I): Acquire ERROR: err=%x(%s)\n", res, ExplainDDError(res)); 
		res = (*pGetDeviceData)(lpdid, cbdata, rgdod, pdwinout, dwflags);
	}

	switch(res){
		case DI_OK:
		case DI_BUFFEROVERFLOW:
			break;
		case DIERR_NOTACQUIRED: 
		case DIERR_INPUTLOST: 
			OutTraceB("GetDeviceData(I): handling err=%s\n", ExplainDDError(res));
			*pdwinout = 0; // to avoid crashes
			break;
		default:
			OutTraceE("GetDeviceData(I) ERROR: err=%x(%s)\n", res, ExplainDDError(res)); 
			return res;
			break;
	}

	if(!dxw.bActive) {
		*pdwinout = 0;
		return DI_OK;
	}

	if(lpdid == lpDIDSysMouse){
		if(dxw.dwFlags4 & RELEASEMOUSE) {
			POINT curr;
			RECT client;
			extern GetCursorPos_Type pGetCursorPos;
			extern GetClientRect_Type pGetClientRect;
			extern ScreenToClient_Type pScreenToClient;
			(*pGetCursorPos)(&curr);
			(*pScreenToClient)(dxw.GethWnd(), &curr);
			(*pGetClientRect)(dxw.GethWnd(), &client);
			if ((curr.x < client.left) || (curr.y < client.top) || (curr.x > client.right) || (curr.y > client.bottom)){
				*pdwinout = 0;
				return DI_OK;
			}
		}

		tmp = (BYTE *)rgdod;
		if(!tmp) return res;

		if(dxw.bDInputAbs){
			GetMousePosition((int *)&p.x, (int *)&p.y);
			for(i = 0; (i < *pdwinout) && ((LPDIDEVICEOBJECTDATA)tmp)->dwOfs; i ++){
				if(((LPDIDEVICEOBJECTDATA)tmp)->dwOfs == DIMOFS_X)((LPDIDEVICEOBJECTDATA)tmp)->dwData = p.x;
				if(((LPDIDEVICEOBJECTDATA)tmp)->dwOfs == DIMOFS_Y)((LPDIDEVICEOBJECTDATA)tmp)->dwData = p.y;
				tmp += cbdata;
			}
			OutTraceB("GetDeviceData(I): ABS mousedata=(%d,%d)\n", p.x, p.y);
		}
		else{
			for(i = 0; (i < *pdwinout) && ((LPDIDEVICEOBJECTDATA)tmp)->dwOfs; i ++){
				if(((LPDIDEVICEOBJECTDATA)tmp)->dwOfs == DIMOFS_X) OutTraceB("GetDeviceData(I): REL mousedata X=%d\n", ((LPDIDEVICEOBJECTDATA)tmp)->dwData);
				if(((LPDIDEVICEOBJECTDATA)tmp)->dwOfs == DIMOFS_Y) OutTraceB("GetDeviceData(I): REL mousedata Y=%d\n", ((LPDIDEVICEOBJECTDATA)tmp)->dwData);
	 			tmp += cbdata;
			}
		}
	}
	return res;
}

HRESULT WINAPI extGetDeviceState(LPDIRECTINPUTDEVICE lpdid, DWORD cbdata, LPDIMOUSESTATE lpvdata)
{
	HRESULT res; 
	POINT p = {0, 0};

	OutTraceB("GetDeviceState(I): did=%x(%s) cbData=%i,%i\n", lpdid, sDevice(lpdid), cbdata, dxw.bActive);

	res = (*pGetDeviceState)(lpdid, cbdata, lpvdata);

	if (((res == DIERR_INPUTLOST) || (res == DIERR_NOTACQUIRED)) && RECOVERINPUTLOST){
		OutTraceE("GetDeviceState(I) recovering DIERR_INPUTLOST\n"); 
		if(lpdid == lpDIDSysMouse) {
			res = (*pDISetCooperativeLevel)(lpdid, dxw.GethWnd(), DISCL_NONEXCLUSIVE | DISCL_FOREGROUND);
			if(res) OutTraceE("GetDeviceState(I): SetCooperativeLevel ERROR: err=%x(%s)\n", res, ExplainDDError(res)); 
			if(dxw.dwFlags1 & CLIPCURSOR) dxw.SetClipCursor();
		}
		res = (*pAcquire)(lpdid);
		if(res) OutTraceE("GetDeviceState(I): Acquire ERROR: err=%x(%s)\n", res, ExplainDDError(res)); 
		res = (*pGetDeviceState)(lpdid, cbdata, lpvdata);
	}

	switch(res){
		case DI_OK:
			break;
		case DIERR_NOTACQUIRED: 
		case DIERR_INPUTLOST: 
			OutTraceB("GetDeviceState(I): handling err=%s\n", ExplainDDError(res));
			break;
		default:
			OutTraceE("GetDeviceState(I) ERROR: err=%x(%s)\n", res, ExplainDDError(res)); 
			return res;
			break;
	}

	if(	cbdata == sizeof(DIMOUSESTATE) || cbdata == sizeof(DIMOUSESTATE2)){

		if(!dxw.bActive){
			lpvdata->lZ = 0;
			*(DWORD *)lpvdata->rgbButtons = 0;
			OutTraceB("GetDeviceState(I): DEBUG cleared mousestate=(%d,%d)\n", p.x, p.y);
			return DI_OK;
		}

		if(dxw.bDInputAbs){ // absolute position
			POINT p;
			extGetCursorPos(&p);
			lpvdata->lX = p.x;
			lpvdata->lY = p.y;
			OutTraceDW("GetMousePosition(I): x,y=(%d,%d)\n", p.x, p.y);
		}
		else { // relative position
			if(dxw.dwFlags6 & EMULATERELMOUSE){
				int iMaxX, iMaxY, iMinX, iMinY;
				RECT WinRect = dxw.GetUnmappedScreenRect();
				iMinX = WinRect.left;
				iMaxX = WinRect.right;
				iMinY = WinRect.top;
				iMaxY = WinRect.bottom;
				iCursorX = (iMaxX+iMinX)/2;
				iCursorY = (iMaxY+iMinY)/2;
				OutTraceB("GetDeviceState(I): RELATIVE clip=(%d,%d)-(%d,%d) pos=(%d,%d)\n", iMinX, iMinY, iMaxX, iMaxY, iCursorX, iCursorY);
				(*pGetCursorPos)(&p);
				lpvdata->lX = p.x - iCursorX;
				lpvdata->lY = p.y - iCursorY;
				(*pSetCursorPos)(iCursorX, iCursorY);
			}
		}
	}

	// SysKeybd device
	if(cbdata == 256 && !dxw.bActive) {
		ZeroMemory(lpvdata, 256);
		OutTraceB("GetDeviceState(I): DEBUG cleared syskeybdstate\n");
	}
	return DI_OK;
}

static char *didftype(DWORD c)
{
	static char eb[256];
	unsigned int l;
	strcpy(eb,"DIDFT_");
	switch (c & 0x00000003){
		case DIDFT_RELAXIS: strcat(eb, "RELAXIS+"); break;
		case DIDFT_ABSAXIS: strcat(eb, "ABSAXIS+"); break;
		case DIDFT_AXIS: strcat(eb, "AXIS+"); break;
	}
	switch (c & 0x0000000C){
		case DIDFT_PSHBUTTON: strcat(eb, "PSHBUTTON+"); break;
		case DIDFT_TGLBUTTON: strcat(eb, "TGLBUTTON+"); break;
		case DIDFT_BUTTON: strcat(eb, "BUTTON+"); break;
	}
	if (c & DIDFT_POV) strcat(eb, "POV+");
	if (c & DIDFT_COLLECTION) strcat(eb, "COLLECTION+");
	if (c & DIDFT_NODATA) strcat(eb, "NODATA+");
	if (c & DIDFT_FFACTUATOR) strcat(eb, "FFACTUATOR+");
	if (c & DIDFT_FFEFFECTTRIGGER) strcat(eb, "FFEFFECTTRIGGER+");
	if (c & DIDFT_VENDORDEFINED) strcat(eb, "VENDORDEFINED+");
	if (c & DIDFT_OUTPUT) strcat(eb, "OUTPUT+");
	if (c & DIDFT_ALIAS) strcat(eb, "ALIAS+");
	if (c & DIDFT_OPTIONAL) strcat(eb, "OPTIONAL+");
	l=strlen(eb);
	if (l>strlen("DIDFT_")) eb[l-1]=0; // delete last '+' if any
	else eb[0]=0;
	return(eb);
}
static char *ExplainDataFormatFlags(DWORD f)
{
	char *s;
	s="unknown";
	switch(f){
		case DIDF_ABSAXIS: s="DIDF_ABSAXIS";
		case DIDF_RELAXIS: s="DIDF_RELAXIS";
	}
	return s;
}

HRESULT WINAPI extSetDataFormat(LPDIRECTINPUTDEVICE lpdid, LPCDIDATAFORMAT lpdf)
{
	OutTraceDW("SetDataFormat(I): did=%x(%s) lpdf=%x size=%d objsize=%d flags=0x%x(%s) datasize=%d numobjects=%d\n", 
		lpdid, sDevice(lpdid), lpdf, lpdf->dwSize, lpdf->dwObjSize, lpdf->dwFlags, ExplainDataFormatFlags(lpdf->dwFlags), lpdf->dwDataSize, lpdf->dwNumObjs);
	if(IsDebug){
		DIOBJECTDATAFORMAT *df;
		df = lpdf->rgodf;
		for(DWORD i=0; i<lpdf->dwNumObjs; i++){
			OutTrace("SetDataFormat(I): DataFormat[%d] ofs=%x flags=%x type=%x(%s)\n", 
				i, df[i].dwOfs, df[i].dwFlags, df[i].dwType, didftype(df[i].dwType));
		}
	}

	if(lpdid == lpDIDSysMouse){
		if(lpdf->dwFlags & DIDF_ABSAXIS) dxw.bDInputAbs = 1;
		if(lpdf->dwFlags & DIDF_RELAXIS) dxw.bDInputAbs = 0;
	}
	return (*pSetDataFormat)(lpdid, lpdf);
}

HRESULT WINAPI extDISetCooperativeLevel(LPDIRECTINPUTDEVICE lpdid, HWND hwnd, DWORD dwflags)
{
	HRESULT res;

	OutTraceDW("SetCooperativeLevel(I): did=%x(%s) hwnd=%x flags=%x(%s)\n", 
		lpdid, sDevice(lpdid), hwnd, dwflags, ExplainDICooperativeFlags(dwflags));

	if(dxw.IsRealDesktop(hwnd)) hwnd=dxw.GethWnd();

	if(lpdid == lpDIDSysMouse) dwflags = (DISCL_NONEXCLUSIVE | DISCL_FOREGROUND);
	// v2.03.84: SHAREDKEYBOARD option to prevent situations like "Planet of the Apes"
	// not processing the PrintScreen syskeys.
	if((lpdid == lpDIDKeyboard) && (dxw.dwFlags7 & SHAREDKEYBOARD))
		dwflags = (DISCL_NONEXCLUSIVE | DISCL_FOREGROUND);

	res = (*pDISetCooperativeLevel)(lpdid, hwnd, dwflags);
	if(res != DD_OK){
		OutTraceE("SetCooperativeLevel(I) ERROR: err=%x(%s)\n", res, ExplainDDError(res)); 
	}

	return res;
}

// Simplified version, taking in proper account the GetCursorPos API hooking & coordinate processing
void GetMousePosition(int *x, int *y)
{
	POINT p;

	extern BOOL WINAPI extGetCursorPos(LPPOINT);
	extGetCursorPos(&p);
	*x = p.x;
	*y = p.y;
	OutTraceDW("GetMousePosition(I): x,y=(%d,%d)\n", *x, *y);
}

typedef struct {
	LPDIENUMDEVICESCALLBACK cb;
	LPVOID arg;
} CallbackArg;

HRESULT WINAPI extDeviceProxy(LPCDIDEVICEINSTANCE dev, LPVOID arg)
{
	HRESULT res;
	char *p;
	switch (dev->dwSize) {
		case sizeof(DIDEVICEINSTANCEA): p="ASCII"; break;
		case sizeof(DIDEVICEINSTANCEW): p="WIDECHAR"; break;
		default: p="UNKNOWN"; break;
	}
	OutTraceDW("EnumDevices(I): CALLBACK size=%d(%s) GUID=(%x.%x.%x.%x) type=%x InstanceName=\"%s\", ProductName=\"%s\"\n", 
		dev->dwSize, p, dev->guidInstance.Data1, dev->guidInstance.Data2, dev->guidInstance.Data3, dev->guidInstance.Data4, 
		dev->dwDevType, dev->tszInstanceName, dev->tszProductName);

	if((dxw.dwFlags7 & SKIPDEVTYPEHID) && (dev->dwDevType & DIDEVTYPE_HID)) {
		OutTraceDW("EnumDevices(I): skip HID device devtype=%x\n", dev->dwDevType);
		return TRUE; // skip DIDEVTYPE_HID
	}
	
	res = (*(((CallbackArg *)arg)->cb))(dev, ((CallbackArg *)arg)->arg);
	OutTraceDW("EnumDevices: CALLBACK ret=%x\n", res);
	return res;
}

HRESULT WINAPI extDIEnumDevicesA(DIEnumDevicesA_Type pDIEnumDevices, void *lpdi, DWORD dwDevType, LPDIENUMDEVICESCALLBACKA lpCallback, LPVOID pvRef, DWORD dwFlags)
{
	HRESULT res;
	CallbackArg Arg;
	OutTraceDW("EnumDevicesA(I): di=%x DevType=%x CallBack=%x Ref=%x Flags=%x\n", lpdi, dwDevType, lpCallback, pvRef, dwFlags);
	Arg.cb= lpCallback;
	Arg.arg=pvRef;
	res=(*pDIEnumDevices)( lpdi, dwDevType, (LPDIENUMDEVICESCALLBACKA)extDeviceProxy, &Arg, dwFlags); // V2.02.80 fix
	//res=(*pDIEnumDevices)( lpdi, dwDevType, lpCallback, pvRef, dwFlags);
	OutTraceDW("EnumDevicesA(I): res=%x\n", res);
	return res;
}

HRESULT WINAPI extDIEnumDevicesA1(void *lpdi, DWORD dwDevType, LPDIENUMDEVICESCALLBACK lpCallback, LPVOID pvRef, DWORD dwFlags)
{ return extDIEnumDevicesA(pDIEnumDevicesA1, lpdi, dwDevType, lpCallback, pvRef, dwFlags); }
HRESULT WINAPI extDIEnumDevicesA2(void *lpdi, DWORD dwDevType, LPDIENUMDEVICESCALLBACK lpCallback, LPVOID pvRef, DWORD dwFlags)
{ return extDIEnumDevicesA(pDIEnumDevicesA2, lpdi, dwDevType, lpCallback, pvRef, dwFlags); }
HRESULT WINAPI extDIEnumDevicesA7(void *lpdi, DWORD dwDevType, LPDIENUMDEVICESCALLBACK lpCallback, LPVOID pvRef, DWORD dwFlags)
{ return extDIEnumDevicesA(pDIEnumDevicesA7, lpdi, dwDevType, lpCallback, pvRef, dwFlags); }
HRESULT WINAPI extDIEnumDevicesA8(void *lpdi, DWORD dwDevType, LPDIENUMDEVICESCALLBACK lpCallback, LPVOID pvRef, DWORD dwFlags)
{ return extDIEnumDevicesA(pDIEnumDevicesA8, lpdi, dwDevType, lpCallback, pvRef, dwFlags); }

HRESULT WINAPI extDIEnumDevicesW(DIEnumDevicesW_Type pDIEnumDevices, void *lpdi, DWORD dwDevType, LPDIENUMDEVICESCALLBACKW lpCallback, LPVOID pvRef, DWORD dwFlags)
{
	HRESULT res;
	CallbackArg Arg;
	OutTraceDW("EnumDevicesW(I): di=%x DevType=%x CallBack=%x Ref=%x Flags=%x\n", lpdi, dwDevType, lpCallback, pvRef, dwFlags);
	Arg.cb= (LPDIENUMDEVICESCALLBACKA)lpCallback;
	Arg.arg=pvRef;
	res=(*pDIEnumDevices)( lpdi, dwDevType, (LPDIENUMDEVICESCALLBACKW)extDeviceProxy, &Arg, dwFlags); // V2.02.80 fix
	//res=(*pDIEnumDevices)( lpdi, dwDevType, lpCallback, pvRef, dwFlags);
	OutTraceDW("EnumDevicesW(I): res=%x\n", res);
	return res;
}

HRESULT WINAPI extDIEnumDevicesW1(void *lpdi, DWORD dwDevType, LPDIENUMDEVICESCALLBACKW lpCallback, LPVOID pvRef, DWORD dwFlags)
{ return extDIEnumDevicesW(pDIEnumDevicesW1, lpdi, dwDevType, lpCallback, pvRef, dwFlags); }
HRESULT WINAPI extDIEnumDevicesW2(void *lpdi, DWORD dwDevType, LPDIENUMDEVICESCALLBACKW lpCallback, LPVOID pvRef, DWORD dwFlags)
{ return extDIEnumDevicesW(pDIEnumDevicesW2, lpdi, dwDevType, lpCallback, pvRef, dwFlags); }
HRESULT WINAPI extDIEnumDevicesW7(void *lpdi, DWORD dwDevType, LPDIENUMDEVICESCALLBACKW lpCallback, LPVOID pvRef, DWORD dwFlags)
{ return extDIEnumDevicesW(pDIEnumDevicesW7, lpdi, dwDevType, lpCallback, pvRef, dwFlags); }
HRESULT WINAPI extDIEnumDevicesW8(void *lpdi, DWORD dwDevType, LPDIENUMDEVICESCALLBACKW lpCallback, LPVOID pvRef, DWORD dwFlags)
{ return extDIEnumDevicesW(pDIEnumDevicesW8, lpdi, dwDevType, lpCallback, pvRef, dwFlags); }

HRESULT WINAPI extAcquire(LPDIRECTINPUTDEVICE lpdid)
{
	HRESULT res;
	res = (*pAcquire)(lpdid);
	OutTrace("Acquire(I): lpdid=%x(%s) res=%x(%s)\n", lpdid, sDevice(lpdid), res, ExplainDDError(res));
	if((dxw.dwFlags7 & SUPPRESSDIERRORS) && (res == DIERR_OTHERAPPHASPRIO)) res = DI_OK;
	return res;
}

HRESULT WINAPI extUnacquire(LPDIRECTINPUTDEVICE lpdid)
{
	HRESULT res;
	res = (*pUnacquire)(lpdid);
	OutTrace("Unacquire(I): lpdid=%x(%s) res=%x(%s)\n", lpdid, sDevice(lpdid), res, ExplainDDError(res));
	if((dxw.dwFlags7 & SUPPRESSDIERRORS) && (res == DIERR_OTHERAPPHASPRIO)) res = DI_OK;
	return res;
}	

void ToggleAcquiredDevices(BOOL flag)
{
	if(flag && pAcquire){
		if(lpDIDSysMouse) (*pAcquire)(lpDIDSysMouse);
		if(lpDIDKeyboard) (*pAcquire)(lpDIDKeyboard);
		if(lpDIDJoystick) (*pAcquire)(lpDIDJoystick);
		if (dxw.dwFlags1 & CLIPCURSOR) dxw.SetClipCursor();
	}
	if(!flag && pUnacquire){
		if(lpDIDSysMouse) (*pUnacquire)(lpDIDSysMouse);
		if(lpDIDKeyboard) (*pUnacquire)(lpDIDKeyboard);
		if(lpDIDJoystick) (*pUnacquire)(lpDIDJoystick);
		if (dxw.dwFlags1 & CLIPCURSOR) dxw.EraseClipCursor();
	}
}

//HRESULT WINAPI extDIFindDeviceA8(void *di, REFGUID rguidClass, LPCSTR ptszName, LPGUID pguidInstance)
//{
//	HRESULT res;
//	MessageBox(0, "calling FindDevice", "debug", 0);
//	res=(*pDIFindDeviceA8)(di, rguidClass, ptszName, pguidInstance);
//	return res;
//}

//HRESULT WINAPI extEnumDevicesBySemantics(void *di, LPCTSTR ptszUserName, LPDIACTIONFORMAT lpdiActionFormat, LPDIENUMDEVICESBYSEMANTICSCB lpCallback, LPVOID pvRef, DWORD dwFlags)
//{
//	HRESULT res;
//	MessageBox(0, "calling EnumDevicesBySemantics", "debug", 0);
//	res=(*pEnumDevicesBySemantics)(di, ptszUserName, lpdiActionFormat, lpCallback, pvRef, dwFlags);
//	return res;
//}

//BOOL WINAPI extDIEnumDevicesBySemanticsCallback(LPCDIDEVICEINSTANCE lpddi, LPDIRECTINPUTDEVICE8 lpdid,  DWORD dwFlags, DWORD dwRemaining, LPVOID pvRef)
//{
//	BOOL res;
//	MessageBox(0, "calling EnumDevicesBySemanticsCallback", "debug", 0);
//	res=(*pDIFindDeviceA8)(di, rguidClass, ptszName, pguidInstance);
//	return res;
//}
