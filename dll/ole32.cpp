#include <dxdiag.h>
#include "dxwnd.h"
#include "dxwcore.hpp"
#include "syslibs.h"
#include "dxhook.h"
#include "dxhelper.h"

extern void HookModule(HMODULE, int);

static HookEntry_Type Hooks[]={
	{"CoCreateInstance", NULL, (FARPROC *)&pCoCreateInstance, (FARPROC)extCoCreateInstance},
	{"CoCreateInstanceEx", NULL, (FARPROC *)&pCoCreateInstanceEx, (FARPROC)extCoCreateInstanceEx}, 
	{"CoInitialize", NULL, (FARPROC *)&pCoInitialize, (FARPROC)extCoInitialize}, 
	{0, NULL, 0, 0} // terminator
};

extern HRESULT WINAPI extDirectDrawCreate(GUID FAR *, LPDIRECTDRAW FAR *, IUnknown FAR *);
extern HRESULT WINAPI extDirectDrawCreateEx(GUID FAR *, LPDIRECTDRAW FAR *, REFIID, IUnknown FAR *);

void HookOle32(HMODULE module)
{
	HookLibrary(module, Hooks, "ole32.dll");
}

FARPROC Remap_ole32_ProcAddress(LPCSTR proc, HMODULE hModule)
{
	FARPROC addr;
	if (addr=RemapLibrary(proc, hModule, Hooks)) return addr;
	return NULL;
}

// so far, there are 4 additional libraries that could be loaded by meand of a CoCreateInstance call....

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
				OutTraceD("CoCreateInstance: hooked module=%s hmodule=%x\n", AddedModules[i].ModuleName, hModule);
				//CloseHandle(hModule); // do NOT close this handle
			}
		}
	}
}

// -------------------------------------------------------------------------------------
// Ole32 CoCreateInstance handling: you can create DirectDraw objects through it!
// utilized so far in a single game: Axiz & Allies
// -------------------------------------------------------------------------------------

static void HookDDSession(LPDIRECTDRAW *, int);
static HRESULT STDAPICALLTYPE myCoCreateInstance(REFCLSID rclsid, LPUNKNOWN pUnkOuter, DWORD dwClsContext, REFIID riid, LPVOID FAR* ppv)
{
	HRESULT res;

	res=(*pCoCreateInstance)(rclsid, pUnkOuter, dwClsContext, riid, ppv);
	if(res) 
		OutTraceE("CoCreateInstance: ERROR res=%x\n", res);
	else
		OutTraceD("CoCreateInstance: ppv=%x->%x\n", *ppv, *(DWORD *)*ppv);

	if (*(DWORD *)&rclsid==*(DWORD *)&CLSID_DirectDraw){
		LPDIRECTDRAW lpOldDDraw;
		OutTraceD("CoCreateInstance: CLSID_DirectDraw object\n");
		switch (*(DWORD *)&riid){
		case 0x6C14DB80:
			OutTraceD("DirectDrawCreate: IID_DirectDraw RIID\n");
			res=extDirectDrawCreate(NULL, (LPDIRECTDRAW *)&ppv, 0);
			if(res)OutTraceD("DirectDrawCreate: res=%x(%s)\n", res, ExplainDDError(res));
			break;
		case 0xB3A6F3E0:
			OutTraceD("DirectDrawCreate: IID_DirectDraw2 RIID\n");
			res=extDirectDrawCreate(NULL, &lpOldDDraw, 0);
			if(res)OutTraceD("DirectDrawCreate: res=%x(%s)\n", res, ExplainDDError(res));
			res=lpOldDDraw->QueryInterface(IID_IDirectDraw2, (LPVOID *)&ppv);
			if(res)OutTraceD("QueryInterface: res=%x(%s)\n", res, ExplainDDError(res));
			lpOldDDraw->Release();
			break;
		case 0x9c59509a:
			OutTraceD("DirectDrawCreate: IID_DirectDraw4 RIID\n");
			res=extDirectDrawCreate(NULL, &lpOldDDraw, 0);
			if(res)OutTraceD("DirectDrawCreate: res=%x(%s)\n", res, ExplainDDError(res));
			res=lpOldDDraw->QueryInterface(IID_IDirectDraw4, (LPVOID *)&ppv);
			if(res)OutTraceD("QueryInterface: res=%x(%s)\n", res, ExplainDDError(res));
			lpOldDDraw->Release();
		case 0x15e65ec0:
			OutTraceD("CoCreateInstance: IID_DirectDraw7 RIID\n");
			res=extDirectDrawCreateEx(NULL, (LPDIRECTDRAW *)&ppv, IID_IDirectDraw7, 0);
			if(res)OutTraceD("DirectDrawCreateEx: res=%x(%s)\n", res, ExplainDDError(res));
			break;
		case 0xe436ebb3:
			break;
		}
	}
	else
	if (*(DWORD *)&rclsid==*(DWORD *)&CLSID_DxDiagProvider) res=HookDxDiag(riid, ppv);

	HookAdditionalModules();
	return res;
}

HRESULT STDAPICALLTYPE extCoCreateInstance(REFCLSID rclsid, LPUNKNOWN pUnkOuter, DWORD dwClsContext, REFIID riid, LPVOID FAR* ppv)
{
	HRESULT res;
	OutTraceD("CoCreateInstance: rclsid=%x UnkOuter=%x ClsContext=%x refiid=%x(%s)\n",
		rclsid, pUnkOuter, dwClsContext, riid.Data1, ExplainGUID((GUID *)&riid));

	res=(*pCoCreateInstance)(rclsid, pUnkOuter, dwClsContext, riid, ppv);
	if(res) 
		OutTraceE("CoCreateInstance: ERROR res=%x\n", res);
	else
		OutTraceD("CoCreateInstance: ppv=%x->%x\n", *ppv, *(DWORD *)*ppv);

	if (*(DWORD *)&rclsid==*(DWORD *)&CLSID_DirectDraw){
		LPDIRECTDRAW lpOldDDraw;
		OutTraceD("CoCreateInstance: CLSID_DirectDraw object\n");
		switch (*(DWORD *)&riid){
		case 0x6C14DB80:
			OutTraceD("DirectDrawCreate: IID_DirectDraw RIID\n");
			res=extDirectDrawCreate(NULL, (LPDIRECTDRAW *)&ppv, 0);
			if(res)OutTraceD("DirectDrawCreate: res=%x(%s)\n", res, ExplainDDError(res));
			break;
		case 0xB3A6F3E0:
			OutTraceD("DirectDrawCreate: IID_DirectDraw2 RIID\n");
			res=extDirectDrawCreate(NULL, &lpOldDDraw, 0);
			if(res)OutTraceD("DirectDrawCreate: res=%x(%s)\n", res, ExplainDDError(res));
			res=lpOldDDraw->QueryInterface(IID_IDirectDraw2, (LPVOID *)&ppv);
			if(res)OutTraceD("QueryInterface: res=%x(%s)\n", res, ExplainDDError(res));
			lpOldDDraw->Release();
			break;
		case 0x9c59509a:
			OutTraceD("DirectDrawCreate: IID_DirectDraw4 RIID\n");
			res=extDirectDrawCreate(NULL, &lpOldDDraw, 0);
			if(res)OutTraceD("DirectDrawCreate: res=%x(%s)\n", res, ExplainDDError(res));
			res=lpOldDDraw->QueryInterface(IID_IDirectDraw4, (LPVOID *)&ppv);
			if(res)OutTraceD("QueryInterface: res=%x(%s)\n", res, ExplainDDError(res));
			lpOldDDraw->Release();
		case 0x15e65ec0:
			OutTraceD("CoCreateInstance: IID_DirectDraw7 RIID\n");
			res=extDirectDrawCreateEx(NULL, (LPDIRECTDRAW *)&ppv, IID_IDirectDraw7, 0);
			if(res)OutTraceD("DirectDrawCreateEx: res=%x(%s)\n", res, ExplainDDError(res));
			break;
		case 0xe436ebb3:
			break;
		}
	}
	else
	if (*(DWORD *)&rclsid==*(DWORD *)&CLSID_DxDiagProvider) res=HookDxDiag(riid, ppv);

	HookAdditionalModules();
	return res;
}

HRESULT STDAPICALLTYPE extCoCreateInstanceEx(REFCLSID rclsid, IUnknown *punkOuter, DWORD dwClsCtx, COSERVERINFO *pServerInfo, DWORD dwCount, MULTI_QI *pResults)
{
	HRESULT res;
	DWORD i;

	OutTraceD("CoCreateInstanceEx: rclsid=%x UnkOuter=%x ClsContext=%x Count=%d\n",
		rclsid, punkOuter, dwClsCtx, dwCount);

	res=(*pCoCreateInstanceEx)(rclsid, punkOuter, dwClsCtx, pServerInfo, dwCount, pResults);
	if(res) {
		OutTraceE("CoCreateInstanceEx: ERROR res=%x\n", res);
		return res;
	}

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
			OutTraceD("CoCreateInstanceEx: CLSID_FilterGraph RIID=%x\n", *(DWORD *)&riid);
			qlib=(*pLoadLibraryA)("quartz.dll");
			OutTraceD("CoCreateInstanceEx: quartz lib handle=%x\n", qlib);
			HookModule(qlib, 0);
		}

		if (*(DWORD *)&rclsid==*(DWORD *)&CLSID_DirectDraw){
			LPDIRECTDRAW lpOldDDraw;
			OutTraceD("CoCreateInstanceEx: CLSID_DirectDraw object\n");
			switch (*(DWORD *)&riid){
			case 0x6C14DB80:
				OutTraceD("DirectDrawCreateEx: IID_DirectDraw RIID\n");
				res=extDirectDrawCreate(NULL, (LPDIRECTDRAW *)&ppv, 0);
				if(res)OutTraceD("DirectDrawCreateEx: res=%x(%s)\n", res, ExplainDDError(res));
				break;
			case 0xB3A6F3E0:
				OutTraceD("DirectDrawCreateEx: IID_DirectDraw2 RIID\n");
				res=extDirectDrawCreate(NULL, &lpOldDDraw, 0);
				if(res)OutTraceD("DirectDrawCreateEx: res=%x(%s)\n", res, ExplainDDError(res));
				res=lpOldDDraw->QueryInterface(IID_IDirectDraw2, (LPVOID *)&ppv);
				if(res)OutTraceD("QueryInterfaceEx: res=%x(%s)\n", res, ExplainDDError(res));
				lpOldDDraw->Release();
				break;
			case 0x9c59509a:
				OutTraceD("DirectDrawCreateEx: IID_DirectDraw4 RIID\n");
				res=extDirectDrawCreate(NULL, &lpOldDDraw, 0);
				if(res)OutTraceD("DirectDrawCreateEx: res=%x(%s)\n", res, ExplainDDError(res));
				res=lpOldDDraw->QueryInterface(IID_IDirectDraw4, (LPVOID *)&ppv);
				if(res)OutTraceD("QueryInterfaceEx: res=%x(%s)\n", res, ExplainDDError(res));
				lpOldDDraw->Release();
			case 0x15e65ec0:
				OutTraceD("CoCreateInstanceEx: IID_DirectDraw7 RIID\n");
				res=extDirectDrawCreateEx(NULL, (LPDIRECTDRAW *)&ppv, IID_IDirectDraw7, 0);
				if(res)OutTraceD("DirectDrawCreateEx: res=%x(%s)\n", res, ExplainDDError(res));
				break;
			case 0xe436ebb3:
				break;
			}
		}
		else
		if (*(DWORD *)&rclsid==*(DWORD *)&CLSID_DxDiagProvider) res=HookDxDiag(riid, ppv);
	}

	HookAdditionalModules();
	return res;
}

HRESULT WINAPI extCoInitialize(LPVOID pvReserved)
{
	HRESULT res;
	OutTraceD("CoInitialize: Reserved=%x\n", pvReserved);
	res=(*pCoInitialize)(pvReserved);
	return res;
}
