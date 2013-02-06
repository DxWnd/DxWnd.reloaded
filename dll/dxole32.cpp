#include <dxdiag.h>
#include "dxwnd.h"
#include "dxwcore.hpp"
#include "syslibs.h"
#include "dxhook.h"
#include "dxhelper.h"

extern HRESULT WINAPI extDirectDrawCreate(GUID FAR *, LPDIRECTDRAW FAR *, IUnknown FAR *);
extern HRESULT WINAPI extDirectDrawCreateEx(GUID FAR *, LPDIRECTDRAW FAR *, REFIID, IUnknown FAR *);

FARPROC Remap_ole32_ProcAddress(LPCSTR proc, HMODULE hModule)
{
	if (!strcmp(proc,"CoCreateInstance")){
		pCoCreateInstance=(CoCreateInstance_Type)(*pGetProcAddress)(hModule, proc);
		OutTraceD("GetProcAddress: hooking proc=%s at addr=%x\n", ProcToString(proc), pCoCreateInstance);
		return (FARPROC)extCoCreateInstance;
	}
	return NULL;
}

// -------------------------------------------------------------------------------------
// Ole32 CoCreateInstance handling: you can create DirectDraw objects through it!
// utilized so far in a single game: Axiz & Allies
// -------------------------------------------------------------------------------------

static void HookDDSession(LPDIRECTDRAW *, int);
//CoCreateInstance_Type pCoCreateInstance=NULL;

HRESULT STDAPICALLTYPE extCoCreateInstance(REFCLSID rclsid, LPUNKNOWN pUnkOuter, DWORD dwClsContext, REFIID riid, LPVOID FAR* ppv)
{
	HRESULT res;
	OutTraceD("CoCreateInstance: rclsid=%x UnkOuter=%x ClsContext=%x refiid=%x\n",
		rclsid, pUnkOuter, dwClsContext, riid);

	// CLSID e436ebb3 implies loading quartz.dll to play movies through dshow:
	// quartz.dll must be hooked.
	if (*(DWORD *)&rclsid==0xe436ebb3){
		HMODULE qlib;
		OutTraceD("CoCreateInstance: CLSID_FilterGraph RIID=%x\n", *(DWORD *)&riid);
		qlib=(*pLoadLibraryA)("quartz.dll");
		OutTraceD("CoCreateInstance: quartz lib handle=%x\n", qlib);
		extern void HookSysLibs(HMODULE);
		HookSysLibs(qlib);
	}

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

	return res;
}

int HookOle32(HMODULE module, int version)
{
	// used by Axis & Allies ....
	void *tmp;
	//return 0;
	OutTraceD("HookOle32 version=%d\n", version); //GHO
	tmp = HookAPI(module, "ole32.dll", NULL, "CoCreateInstance", extCoCreateInstance);
	if(tmp) pCoCreateInstance = (CoCreateInstance_Type)tmp;
	return 0;
}
