#include <dxdiag.h>
#include <dsound.h>
#include "dxwnd.h"
#include "dxwcore.hpp"
#include "syslibs.h"
#include "dxhook.h"
#include "dxhelper.h"

extern void HookModule(HMODULE, int);
extern void HookDirectSoundObj(LPDIRECTSOUND *);

static BOOL bRecursedHook = FALSE;

static HookEntryEx_Type Hooks[]={
	{HOOK_HOT_CANDIDATE, 0, "CoCreateInstance", NULL, (FARPROC *)&pCoCreateInstance, (FARPROC)extCoCreateInstance},
	{HOOK_HOT_CANDIDATE, 0, "CoCreateInstanceEx", NULL, (FARPROC *)&pCoCreateInstanceEx, (FARPROC)extCoCreateInstanceEx}, 
	//{HOOK_HOT_CANDIDATE, 0, "CoInitialize", NULL, (FARPROC *)&pCoInitialize, (FARPROC)extCoInitialize}, 
	//{HOOK_HOT_CANDIDATE, 0, "CoUninitialize", NULL, (FARPROC *)&pCoUninitialize, (FARPROC)extCoUninitialize}, 
	{HOOK_IAT_CANDIDATE, 0, 0, NULL, 0, 0} // terminator
};

extern HRESULT WINAPI extDirectDrawCreate(GUID FAR *, LPDIRECTDRAW FAR *, IUnknown FAR *);
extern HRESULT WINAPI extDirectDrawCreateEx(GUID FAR *, LPDIRECTDRAW FAR *, REFIID, IUnknown FAR *);

void HookOle32(HMODULE module)
{
	HookLibraryEx(module, Hooks, "ole32.dll");
}

FARPROC Remap_ole32_ProcAddress(LPCSTR proc, HMODULE hModule)
{
	FARPROC addr;
	if (addr=RemapLibraryEx(proc, hModule, Hooks)) return addr;
	return NULL;
}

// so far, there are 4 additional libraries that could be loaded by means of a CoCreateInstance call....

#define ADDITIONAL_MODULE_COUNT 4
struct {
	BOOL Hooked;
	char *ModuleName;
} AddedModules[ADDITIONAL_MODULE_COUNT]=
{
	{FALSE, "quartz"},
	{FALSE, "ddrawex"},
	{FALSE, "amstream"},
	{FALSE, "dplayx"}
};

static void HookAdditionalModules()
{
	for(int i=0; i<ADDITIONAL_MODULE_COUNT; i++){
		if(!AddedModules[i].Hooked){ // if the module is not hooked yet...
			HMODULE hModule;
			hModule=GetModuleHandle(AddedModules[i].ModuleName);
			if(hModule){ // --- and now it exists ...
				HookModule(hModule, 0); // ... then hook it and ..
				AddedModules[i].Hooked=TRUE; // .. mark it as already hooked
				OutTraceDW("CoCreateInstance: hooked module=%s hmodule=%x\n", AddedModules[i].ModuleName, hModule);
				//CloseHandle(hModule); // do NOT close this handle
			}
		}
	}
}

// -------------------------------------------------------------------------------------
// Ole32 CoCreateInstance handling: you can create DirectDraw objects through it!
// utilized so far in a single game: "Axis & Allies"
// another one: "Crusaders of Might and Magic" ....
// -------------------------------------------------------------------------------------

extern void HookDDSession(LPDIRECTDRAW *, int); 
#if 0
typedef HRESULT (WINAPI *Initialize_Type)(LPDIRECTDRAW, GUID FAR *);
extern Initialize_Type pInitialize;
HRESULT WINAPI extInitialize1(LPDIRECTDRAW lpdd, GUID FAR *lpguid) { OutTrace("1\n"); HookDDSession(&lpdd, 1); return DD_OK; }
HRESULT WINAPI extInitialize2(LPDIRECTDRAW lpdd, GUID FAR *lpguid) { OutTrace("2\n"); HookDDSession(&lpdd, 2); return DD_OK; }
HRESULT WINAPI extInitialize4(LPDIRECTDRAW lpdd, GUID FAR *lpguid) { OutTrace("4\n"); HookDDSession(&lpdd, 4); return DD_OK; }
HRESULT WINAPI extInitialize7(LPDIRECTDRAW lpdd, GUID FAR *lpguid) { OutTrace("7\n"); HookDDSession(&lpdd, 7); return DD_OK; }

void HookDDSessionInitialize(LPDIRECTDRAW *lplpdd, int dxversion, Initialize_Type extInitialize)
{
	OutTraceDW("Hooking directdraw Init dd=%x dxversion=%d pInit=%x\n", 
		*lplpdd, dxversion, extInitialize);
	SetHook((void *)(**(DWORD **)lplpdd + 72), extInitialize, (void **)&pInitialize, "Initialize(D)");
}
#endif

HRESULT STDAPICALLTYPE extCoCreateInstance(REFCLSID rclsid, LPUNKNOWN pUnkOuter, DWORD dwClsContext, REFIID riid, LPVOID FAR* ppv)
{
	HRESULT res;
	OutTraceDW("CoCreateInstance: rclsid=%x UnkOuter=%x ClsContext=%x refiid=%x(%s)\n",
		rclsid.Data1, pUnkOuter, dwClsContext, riid.Data1, ExplainGUID((GUID *)&riid));

	bRecursedHook = TRUE;
	res=(*pCoCreateInstance)(rclsid, pUnkOuter, dwClsContext, riid, ppv);
	bRecursedHook = FALSE;
	if(res) {
		OutTraceE("CoCreateInstance: ERROR res=%x\n", res);
		return res;
	}
	
	OutTraceDW("CoCreateInstance: ppv=%x->%x\n", *ppv, *(DWORD *)*ppv);

	switch (*(DWORD *)&rclsid) {
		case 0xD7B70EE0: // CLSID_DirectDraw:
			// v2.03.18: fixed
			OutTraceDW("CoCreateInstance: CLSID_DirectDraw object\n");
			switch (*(DWORD *)&riid){
				LPDIRECTDRAW lpOldDDraw;
				case 0x6C14DB80:
					// must go through DirectDrawCreate: needed for "Darius Gaiden"
					OutTraceDW("CoCreateInstance: IID_DirectDraw RIID\n");
					res=extDirectDrawCreate(NULL, &lpOldDDraw, 0);
					if(res)OutTraceDW("DirectDrawCreate: res=%x(%s)\n", res, ExplainDDError(res));
					res=lpOldDDraw->QueryInterface(IID_IDirectDraw, (LPVOID *)ppv);
					if(res)OutTraceDW("QueryInterface: res=%x(%s)\n", res, ExplainDDError(res));
					lpOldDDraw->Release();
					break;
				case 0xB3A6F3E0:
					OutTraceDW("CoCreateInstance: IID_DirectDraw2 RIID\n");
					res=extDirectDrawCreate(NULL, &lpOldDDraw, 0);
					if(res)OutTraceDW("DirectDrawCreate: res=%x(%s)\n", res, ExplainDDError(res));
					res=lpOldDDraw->QueryInterface(IID_IDirectDraw2, (LPVOID *)ppv);
					if(res)OutTraceDW("QueryInterface: res=%x(%s)\n", res, ExplainDDError(res));
					lpOldDDraw->Release();
					break;
				case 0x9C59509A:
					OutTraceDW("CoCreateInstance: IID_DirectDraw4 RIID\n");
					res=extDirectDrawCreate(NULL, &lpOldDDraw, 0);
					if(res)OutTraceDW("DirectDrawCreate: res=%x(%s)\n", res, ExplainDDError(res));
					res=lpOldDDraw->QueryInterface(IID_IDirectDraw4, (LPVOID *)ppv);
					if(res)OutTraceDW("QueryInterface: res=%x(%s)\n", res, ExplainDDError(res));
					lpOldDDraw->Release();
					break;
				case 0x15E65EC0:
					OutTraceDW("CoCreateInstance: IID_DirectDraw7 RIID\n");
					res=extDirectDrawCreateEx(NULL, (LPDIRECTDRAW *)ppv, IID_IDirectDraw7, 0);
					if(res)OutTraceDW("DirectDrawCreateEx: res=%x(%s)\n", res, ExplainDDError(res));
					break;
				case 0xE436EBB3:
					break;
			}
			break;
		case 0xA65B8071: // CLSID_DxDiagProvider
			OutTraceDW("CoCreateInstance: CLSID_DxDiagProvider object\n");
			HookDxDiag(riid, ppv);
			break;
		case 0x47d4d946: // CLSID_DirectSound
			OutTraceDW("CoCreateInstance: CLSID_DirectSound object\n");
			HookDirectSoundObj((LPDIRECTSOUND *)ppv);
			break;
		default:
			break;
	}

	HookAdditionalModules();
	OutTraceDW("CoCreateInstance: res=%x ppv=%x->%x\n", res, *ppv, *(DWORD *)*ppv);
	return S_OK;
}

HRESULT STDAPICALLTYPE extCoCreateInstanceEx(REFCLSID rclsid, IUnknown *punkOuter, DWORD dwClsCtx, COSERVERINFO *pServerInfo, DWORD dwCount, MULTI_QI *pResults)
{
	HRESULT res;
	DWORD i;

	OutTraceDW("CoCreateInstanceEx: rclsid=%x UnkOuter=%x ClsContext=%x Count=%d\n",
		rclsid.Data1, punkOuter, dwClsCtx, dwCount);

	res=(*pCoCreateInstanceEx)(rclsid, punkOuter, dwClsCtx, pServerInfo, dwCount, pResults);
	if(res) {
		OutTraceE("CoCreateInstanceEx: ERROR res=%x\n", res);
		return res;
	}
	if(bRecursedHook) return res;

	for(i=0; i<dwCount; i++){
		LPVOID FAR* ppv;
		IID riid;

		if(pResults[i].hr) continue;

		riid=*pResults[i].pIID;
		ppv=(LPVOID *)pResults[i].pItf;

		// CLSID e436ebb3 implies loading quartz.dll to play movies through dshow:
		// quartz.dll must be hooked.
		if (*(DWORD *)&rclsid==0xe436ebb3){
			HMODULE qlib;
			OutTraceDW("CoCreateInstanceEx: CLSID_FilterGraph RIID=%x\n", *(DWORD *)&riid);
			qlib=(*pLoadLibraryA)("quartz.dll");
			OutTraceDW("CoCreateInstanceEx: quartz lib handle=%x\n", qlib);
			HookModule(qlib, 0);
		}

		if (*(DWORD *)&rclsid==*(DWORD *)&CLSID_DirectDraw){
			extern void HookDDSession(LPDIRECTDRAW *, int); 
			OutTraceDW("CoCreateInstance: CLSID_DirectDraw object\n");
			switch (*(DWORD *)&riid){
				LPDIRECTDRAW lpOldDDraw;
				case 0x6C14DB80:
					OutTraceDW("CoCreateInstance: IID_DirectDraw RIID\n");
					HookDDSession((LPDIRECTDRAW *)ppv, 1);
					break;
				case 0xB3A6F3E0:
					OutTraceDW("CoCreateInstance: IID_DirectDraw2 RIID\n");
					res=extDirectDrawCreate(NULL, &lpOldDDraw, 0);
					if(res)OutTraceDW("DirectDrawCreate: res=%x(%s)\n", res, ExplainDDError(res));
					res=lpOldDDraw->QueryInterface(IID_IDirectDraw2, (LPVOID *)ppv);
					if(res)OutTraceDW("QueryInterface: res=%x(%s)\n", res, ExplainDDError(res));
					lpOldDDraw->Release();
					break;
				case 0x9C59509A:
					OutTraceDW("CoCreateInstance: IID_DirectDraw4 RIID\n");
					res=extDirectDrawCreate(NULL, &lpOldDDraw, 0);
					if(res)OutTraceDW("DirectDrawCreate: res=%x(%s)\n", res, ExplainDDError(res));
					res=lpOldDDraw->QueryInterface(IID_IDirectDraw4, (LPVOID *)ppv);
					if(res)OutTraceDW("QueryInterface: res=%x(%s)\n", res, ExplainDDError(res));
					lpOldDDraw->Release();
					break;
				case 0x15E65EC0:
					OutTraceDW("CoCreateInstance: IID_DirectDraw7 RIID\n");
					res=extDirectDrawCreateEx(NULL, (LPDIRECTDRAW *)ppv, IID_IDirectDraw7, 0);
					if(res)OutTraceDW("DirectDrawCreateEx: res=%x(%s)\n", res, ExplainDDError(res));
					break;
				case 0xE436EBB3:
					break;
			}
		}
		else {
			if (*(DWORD *)&rclsid==*(DWORD *)&CLSID_DxDiagProvider) res=HookDxDiag(riid, ppv);
		}
	}

	HookAdditionalModules();
	OutTraceDW("CoCreateInstanceEx: res=%x\n", res);
	return res;
}

HRESULT WINAPI extCoInitialize(LPVOID pvReserved)
{
	HRESULT res;
	OutTraceDW("CoInitialize: Reserved=%x\n", pvReserved);
	res=(*pCoInitialize)(pvReserved);
	return res;
}

void WINAPI extCoUninitialize(void)
{
	OutTraceDW("CoUninitialize\n");
	(*pCoUninitialize)();
}