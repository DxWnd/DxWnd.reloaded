#define DIRECTINPUT_VERSION 0x800
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

typedef HRESULT (WINAPI *QueryInterface_Type)(void *, REFIID, LPVOID *);
typedef HRESULT (WINAPI *DirectInputCreate_Type)(HINSTANCE, DWORD, LPDIRECTINPUT *, LPUNKNOWN);
typedef HRESULT (WINAPI *DirectInputCreateEx_Type)(HINSTANCE, DWORD, REFIID, LPVOID *, LPUNKNOWN);
typedef HRESULT (WINAPI *DICreateDevice_Type)(LPDIRECTINPUT, REFGUID, LPDIRECTINPUTDEVICE *, LPUNKNOWN);
typedef HRESULT (WINAPI *DICreateDeviceEx_Type)(LPDIRECTINPUT, REFGUID, REFIID, LPVOID *, LPUNKNOWN);
typedef HRESULT (WINAPI *GetDeviceData_Type)(LPDIRECTINPUTDEVICE, DWORD, LPVOID, LPDWORD, DWORD);
typedef HRESULT (WINAPI *GetDeviceState_Type)(LPDIRECTINPUTDEVICE, DWORD, LPDIMOUSESTATE);
typedef HRESULT (WINAPI *DISetCooperativeLevel_Type)(LPDIRECTINPUTDEVICE, HWND, DWORD);
typedef HRESULT (WINAPI *SetDataFormat_Type)(LPDIRECTINPUTDEVICE, LPCDIDATAFORMAT);
typedef HRESULT (WINAPI *DIEnumDevices_Type)(void *, DWORD, LPDIENUMDEVICESCALLBACK, LPVOID, DWORD);
typedef HRESULT (WINAPI *Acquire_Type)(LPDIRECTINPUTDEVICE);
typedef HRESULT (WINAPI *Unacquire_Type)(LPDIRECTINPUTDEVICE);
typedef HRESULT (WINAPI *DirectInput8Create_Type)(HINSTANCE, DWORD, REFIID, LPVOID *, LPUNKNOWN);

HRESULT WINAPI extDirectInputCreateA(HINSTANCE, DWORD, LPDIRECTINPUT *, LPUNKNOWN);
HRESULT WINAPI extDirectInputCreateW(HINSTANCE, DWORD, LPDIRECTINPUT *, LPUNKNOWN);
HRESULT WINAPI extDirectInputCreateEx(HINSTANCE, DWORD, REFIID, LPVOID *, LPUNKNOWN);
HRESULT WINAPI extDirectInput8Create(HINSTANCE, DWORD, REFIID, LPVOID *, LPUNKNOWN);
HRESULT WINAPI extDICreateDevice(LPDIRECTINPUT, REFGUID, LPDIRECTINPUTDEVICE *, LPUNKNOWN);
HRESULT WINAPI extDICreateDeviceEx(LPDIRECTINPUT, REFGUID, REFIID, LPVOID *, LPUNKNOWN);
HRESULT WINAPI extGetDeviceData(LPDIRECTINPUTDEVICE, DWORD, LPVOID, LPDWORD, DWORD);
HRESULT WINAPI extGetDeviceState(LPDIRECTINPUTDEVICE, DWORD, LPDIMOUSESTATE);
HRESULT WINAPI extDISetCooperativeLevel(LPDIRECTINPUTDEVICE, HWND, DWORD);
HRESULT WINAPI extSetDataFormat(LPDIRECTINPUTDEVICE, LPCDIDATAFORMAT);
HRESULT WINAPI extDIQueryInterface(void *, REFIID, LPVOID *);
HRESULT WINAPI extDIEnumDevices(void *, DWORD, LPDIENUMDEVICESCALLBACK, LPVOID, DWORD);
HRESULT WINAPI extAcquire(LPDIRECTINPUTDEVICE);
HRESULT WINAPI extUnacquire(LPDIRECTINPUTDEVICE);
HRESULT WINAPI extDirectInput8Create(HINSTANCE, DWORD, REFIID, LPVOID *, LPUNKNOWN);

DirectInputCreate_Type pDirectInputCreateA = NULL;
DirectInputCreate_Type pDirectInputCreateW = NULL;
DirectInputCreateEx_Type pDirectInputCreateEx = NULL;
DICreateDevice_Type pDICreateDevice = NULL;
DICreateDeviceEx_Type pDICreateDeviceEx = NULL;
GetDeviceData_Type pGetDeviceData = NULL;
GetDeviceState_Type pGetDeviceState = NULL;
DISetCooperativeLevel_Type pDISetCooperativeLevel = NULL;
SetDataFormat_Type pSetDataFormat = NULL;
QueryInterface_Type pDIQueryInterface = NULL;
DIEnumDevices_Type pDIEnumDevices = NULL;
Acquire_Type pAcquire = NULL;
Unacquire_Type pUnacquire = NULL;
DirectInput8Create_Type pDirectInput8Create = NULL;

static HookEntry_Type diHooks[]={
	{HOOK_HOT_CANDIDATE, "DirectInputCreateA", (FARPROC)NULL, (FARPROC *)&pDirectInputCreateA, (FARPROC)extDirectInputCreateA},
	{HOOK_HOT_CANDIDATE, "DirectInputCreateW", (FARPROC)NULL, (FARPROC *)&pDirectInputCreateW, (FARPROC)extDirectInputCreateW},
	{HOOK_HOT_CANDIDATE, "DirectInputCreateEx", (FARPROC)NULL, (FARPROC *)&pDirectInputCreateEx, (FARPROC)extDirectInputCreateEx},
	{HOOK_IAT_CANDIDATE, 0, NULL, 0, 0} // terminator
};

static HookEntry_Type di8Hooks[]={
	{HOOK_HOT_CANDIDATE, "DirectInput8Create", (FARPROC)NULL, (FARPROC *)&pDirectInput8Create, (FARPROC)extDirectInput8Create},
	{HOOK_IAT_CANDIDATE, 0, NULL, 0, 0} // terminator
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

	HookLibrary(module, diHooks, "dinput.dll");
	if(!pDirectInputCreateA && !pDirectInputCreateW && !pDirectInputCreateEx){
		hinst = LoadLibrary("dinput.dll");
		pDirectInputCreateA = (DirectInputCreate_Type)GetProcAddress(hinst, "DirectInputCreateA");
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

	HookLibrary(module, di8Hooks, "dinput8.dll");
	if(!pDirectInput8Create){
		hinst = LoadLibrary("dinput8.dll");
		pDirectInput8Create = (DirectInput8Create_Type)GetProcAddress(hinst, "DirectInput8Create");
		if(pDirectInput8Create)
			if(!extDirectInput8Create(GetModuleHandle(0), DIRECTINPUT_VERSION,
				di8, (LPVOID *)&lpdi, 0)) lpdi->Release();
	}
}

FARPROC Remap_DInput_ProcAddress(LPCSTR proc, HMODULE hModule)
{
	FARPROC addr;
	if (addr=RemapLibrary(proc, hModule, diHooks)) return addr;
	return NULL;
}

FARPROC Remap_DInput8_ProcAddress(LPCSTR proc, HMODULE hModule)
{
	FARPROC addr;
	if (addr=RemapLibrary(proc, hModule, di8Hooks)) return addr;
	return NULL;
}

HRESULT WINAPI extDirectInputCreate(HINSTANCE hinst,
	DWORD dwversion, LPDIRECTINPUT *lplpdi, LPUNKNOWN pu, DirectInputCreate_Type pDirectInputCreate, char *apiname)
{
	HRESULT res;

	OutTraceDW("%s: dwVersion=%x\n", apiname, dwversion);

	res = (*pDirectInputCreate)(hinst, dwversion, lplpdi, pu);
	if(res) {
		OutTraceE("%s: ERROR err=%x(%s)\n", apiname, res, ExplainDDError(res));
		return res;
	}
	SetHook((void *)(**(DWORD **)lplpdi), extDIQueryInterface, (void **)&pDIQueryInterface, "QueryInterface(I)");
	SetHook((void *)(**(DWORD **)lplpdi + 12), extDICreateDevice, (void **)&pDICreateDevice, "CreateDevice(I)");
	SetHook((void *)(**(DWORD **)lplpdi + 16), extDIEnumDevices, (void **)&pDIEnumDevices, "EnumDevices(I)");
	return 0;
}

HRESULT WINAPI extDirectInputCreateA(HINSTANCE hinst, DWORD dwversion, LPDIRECTINPUT *lplpdi, LPUNKNOWN pu)
{
	return extDirectInputCreate(hinst, dwversion, lplpdi, pu, pDirectInputCreateA, "DirectInputCreateA");
}

HRESULT WINAPI extDirectInputCreateW(HINSTANCE hinst, DWORD dwversion, LPDIRECTINPUT *lplpdi, LPUNKNOWN pu)
{
	return extDirectInputCreate(hinst, dwversion, lplpdi, pu, pDirectInputCreateW, "DirectInputCreateW");
}

HRESULT WINAPI extDirectInputCreateEx(HINSTANCE hinst, DWORD dwversion, REFIID riidltf, LPVOID *ppvout, LPUNKNOWN pu)
{
	HRESULT res;

	OutTraceDW("DirectInputCreateEx: dwVersion=%x REFIID=%x\n",
		dwversion, riidltf.Data1);

	res = (*pDirectInputCreateEx)(hinst, dwversion, riidltf, ppvout, pu);
	if(res) {
		OutTraceE("DirectInputCreateEx: ERROR err=%x(%s)\n", res, ExplainDDError(res));
		return res;
	}
	SetHook((void *)(**(DWORD **)ppvout + 12), extDICreateDevice, (void **)&pDICreateDevice, "CreateDevice(I7)");
	SetHook((void *)(**(DWORD **)ppvout + 16), extDIEnumDevices, (void **)&pDIEnumDevices, "EnumDevices(I7)");
	if(dwversion >= 700)
		SetHook((void *)(**(DWORD **)ppvout + 36), extDICreateDeviceEx, (void **)&pDICreateDeviceEx, "CreateDeviceEx(I7)");
	return 0;
}

HRESULT WINAPI extDIQueryInterface(void * lpdi, REFIID riid, LPVOID *obp)
{
	HRESULT res;

	OutTraceDW("QueryInterface(I): REFIID=%x\n",
		riid.Data1);

	res = (*pDIQueryInterface)(lpdi, riid, obp);
	if(res) return res;

	switch(riid.Data1){
	case 0x89521360:		//DirectInputA
	case 0x89521361:		//DirectInputW
		SetHook((void *)(**(DWORD **)obp + 12), extDICreateDevice, (void **)&pDICreateDevice, "CreateDevice(I)");
		SetHook((void *)(**(DWORD **)obp + 16), extDIEnumDevices, (void **)&pDIEnumDevices, "EnumDevices(I)");
		break;
	case 0x5944E662:		//DirectInput2A
	case 0x5944E663:		//DirectInput2W
		SetHook((void *)(**(DWORD **)obp + 12), extDICreateDevice, (void **)&pDICreateDevice, "CreateDevice(I2)");
		SetHook((void *)(**(DWORD **)obp + 16), extDIEnumDevices, (void **)&pDIEnumDevices, "EnumDevices(I2)");
		break;
	case 0x9A4CB684:		//IDirectInput7A
	case 0x9A4CB685:		//IDirectInput7W
		SetHook((void *)(**(DWORD **)obp + 12), extDICreateDevice, (void **)&pDICreateDevice, "CreateDevice(I7)");
		SetHook((void *)(**(DWORD **)obp + 16), extDIEnumDevices, (void **)&pDIEnumDevices, "EnumDevices(I7)");
		SetHook((void *)(**(DWORD **)obp + 36), extDICreateDeviceEx, (void **)&pDICreateDeviceEx, "CreateDeviceEx(I7)");
		break;
	}
	return 0;
}

HRESULT WINAPI extDirectInput8Create(HINSTANCE hinst, DWORD dwversion, REFIID riidltf, LPVOID *ppvout, LPUNKNOWN pu)
{
	HRESULT res;

	OutTraceDW("DirectInput8Create: dwVersion=%x REFIID=%x\n",
		dwversion, riidltf.Data1);

	//res = (*pDirectInputCreateEx)(hinst, dwversion, riidltf, ppvout, pu);
	res = (*pDirectInput8Create)(hinst, dwversion, riidltf, ppvout, pu);
	if(res) {
		OutTraceE("DirectInput8Create: ERROR res=%x\n", res);
		return res;
	}
	OutTraceDW("DirectInput8Create: di=%x\n", *ppvout);
	SetHook((void *)(**(DWORD **)ppvout + 12), extDICreateDevice, (void **)&pDICreateDevice, "CreateDevice(I8)");
	SetHook((void *)(**(DWORD **)ppvout + 16), extDIEnumDevices, (void **)&pDIEnumDevices, "EnumDevices(I8)");
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

HRESULT WINAPI extDICreateDevice(LPDIRECTINPUT lpdi, REFGUID rguid, LPDIRECTINPUTDEVICE *lplpdid, LPUNKNOWN pu)
{
	HRESULT res;

	OutTraceDW("CreateDevice(I): REFGUID=%x(%s)\n", rguid.Data1, sDeviceType(rguid));

	res = (*pDICreateDevice)(lpdi, rguid, lplpdid, pu);
	if(res) {
		OutTraceE("CreateDevice(I): ERROR res=%x\n", res);
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

HRESULT WINAPI extDICreateDeviceEx(LPDIRECTINPUT lpdi, REFGUID rguid,
	REFIID riid, LPVOID *pvout, LPUNKNOWN pu)
{
	HRESULT res;

	OutTraceDW("CreateDeviceEx(I): GUID=%x(%s) REFIID=%x\n", rguid.Data1, sDeviceType(rguid), riid.Data1);

	res = (*pDICreateDeviceEx)(lpdi, rguid, riid, pvout, pu);
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

HRESULT WINAPI extGetDeviceData(LPDIRECTINPUTDEVICE lpdid, DWORD cbdata, LPVOID rgdod, LPDWORD pdwinout, DWORD dwflags)
{
	HRESULT res;
	BYTE *tmp;
	unsigned int i;
	POINT p;

	OutTraceDW("GetDeviceData(I): did=%x(%s) cbdata=%i rgdod=%x, inout=%d flags=%x\n", 
		lpdid, sDevice(lpdid), cbdata, rgdod, *pdwinout, dwflags);

	res = (*pGetDeviceData)(lpdid, cbdata, rgdod, pdwinout, dwflags);
	switch(res){
		case DI_OK:
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
		if(dxw.bDInputAbs){
			GetMousePosition((int *)&p.x, (int *)&p.y);
			for(i = 0; i < *pdwinout; i ++){
				if(((LPDIDEVICEOBJECTDATA)tmp)->dwOfs == DIMOFS_X)((LPDIDEVICEOBJECTDATA)tmp)->dwData = p.x;
				if(((LPDIDEVICEOBJECTDATA)tmp)->dwOfs == DIMOFS_Y)((LPDIDEVICEOBJECTDATA)tmp)->dwData = p.y;
				tmp += cbdata;
			}
			OutTraceB("GetDeviceData(I): ABS mousedata=(%d,%d)\n", p.x, p.y);
		}
		else{
			for(i = 0; i < *pdwinout; i ++){
				if(((LPDIDEVICEOBJECTDATA)tmp)->dwOfs == DIMOFS_X) OutTraceB("GetDeviceData(I): REL mousedata X=%d\n", ((LPDIDEVICEOBJECTDATA)tmp)->dwData);
				if(((LPDIDEVICEOBJECTDATA)tmp)->dwOfs == DIMOFS_Y) OutTraceB("GetDeviceData(I): REL mousedata Y=%d\n", ((LPDIDEVICEOBJECTDATA)tmp)->dwData);
				tmp += cbdata;
			}
		}
	}
	return DI_OK;
}

HRESULT WINAPI extGetDeviceState(LPDIRECTINPUTDEVICE lpdid, DWORD cbdata, LPDIMOUSESTATE lpvdata)
{
	HRESULT res;
	POINT p = {0, 0};

	OutTraceDW("GetDeviceState(I): did=%x(%s) cbData=%i,%i\n", lpdid, sDevice(lpdid), cbdata, dxw.bActive);

	res = (*pGetDeviceState)(lpdid, cbdata, lpvdata);

	if ((res == DIERR_INPUTLOST) && RECOVERINPUTLOST){
		OutTraceE("GetDeviceState(I) recovering DIERR_INPUTLOST\n"); 
		res = (*pDISetCooperativeLevel)(lpdid, dxw.GethWnd(), DISCL_NONEXCLUSIVE | DISCL_FOREGROUND);
		if(res) OutTraceE("GetDeviceState(I): SetCooperativeLevel ERROR: err=%x(%s)\n", res, ExplainDDError(res)); 
		res = (*pAcquire)(lpdid);
		if(res) OutTraceE("GetDeviceState(I): Acquire ERROR: err=%x(%s)\n", res, ExplainDDError(res)); 
		if(dxw.dwFlags1 & CLIPCURSOR) dxw.SetClipCursor();
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
				RECT WinRect = dxw.GetMainWindow();
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
	//dwflags = DISCL_NONEXCLUSIVE | DISCL_BACKGROUND;

	dwflags = DISCL_NONEXCLUSIVE | DISCL_FOREGROUND;
	hwnd=dxw.GethWnd();
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

	res = (*(((CallbackArg *)arg)->cb))(dev, ((CallbackArg *)arg)->arg);
	OutTraceDW("EnumDevices: CALLBACK ret=%x\n", res);
	return res;
}

HRESULT WINAPI extDIEnumDevices(void *lpdi, DWORD dwDevType, LPDIENUMDEVICESCALLBACK lpCallback, LPVOID pvRef, DWORD dwFlags)
{
	HRESULT res;
	CallbackArg Arg;
	OutTraceDW("EnumDevices(I): di=%x DevType=%x CallBack=%x Ref=%x Flags=%x\n", lpdi, dwDevType, lpCallback, pvRef, dwFlags);
	Arg.cb= lpCallback;
	Arg.arg=pvRef;
	res=(*pDIEnumDevices)( lpdi, dwDevType, (LPDIENUMDEVICESCALLBACK)extDeviceProxy, &Arg, dwFlags); // V2.02.80 fix
	//res=(*pDIEnumDevices)( lpdi, dwDevType, lpCallback, pvRef, dwFlags);
	OutTraceDW("EnumDevices(I): res=%x\n", res);
	return res;
}

HRESULT WINAPI extAcquire(LPDIRECTINPUTDEVICE lpdid)
{
	HRESULT res;
	res = (*pAcquire)(lpdid);
	OutTrace("Acquire(I): lpdid=%x(%s) res=%x(%s)\n", lpdid, sDevice(lpdid), res, ExplainDDError(res));
	return res;
}

HRESULT WINAPI extUnacquire(LPDIRECTINPUTDEVICE lpdid)
{
	HRESULT res;
	res = (*pUnacquire)(lpdid);
	OutTrace("Unacquire(I): lpdid=%x(%s) res=%x(%s)\n", lpdid, sDevice(lpdid), res, ExplainDDError(res));
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
