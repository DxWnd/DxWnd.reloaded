#define _CRT_SECURE_NO_WARNINGS
#define INITGUID

#include <windows.h>
#include <ddrawex.h>
#include "dxwnd.h"
#include "dxhook.h"
#include "ddrawi.h"
#include "dxwcore.hpp"
#include "stdio.h" 
#include "hddraw.h"
#include "dxhelper.h"
#include "syslibs.h"

/*
Using DirectDrawEx

This article provides a brief overview of DirectDrawEx and how it extends the functionality of a DirectDraw object as described in the Microsoft DirectX® SDK.

Contents of this article:
    What Is DirectDrawEx?
    Advantages of Using DirectDrawEx
    Creating DirectDraw Objects and Surfaces with DirectDrawEx
    Distinctions Between DirectDraw and DirectDrawEx 

What Is DirectDrawEx?

DirectDrawEx is a dynamic-link library (DLL) that embellishes current functionality of DirectDraw, enhancing 
existing features and providing new functionality. 
DirectDrawEx also exposes new interfaces that applications can use when you include the ddrawex.h header file.
To create a DirectDraw object that can use the extended features provided by DirectDrawEx, you must create the 
object by using the IDirectDrawFactory interface. A DirectDraw object created with the IDirectDrawFactory 
interface will support the IDirectDraw3 interface, aggregation of DirectDraw surfaces, data exchange, and palette 
mapping, in addition to the features of DirectDraw objects described in the DirectX SDK.

Advantages of Using DirectDrawEx

The primary advantage of creating a DirectDraw object through the IDirectDrawFactory interface is that it exposes 
the IDirectDraw3 interface. The IDirectDraw3 interface inherits all the functionality of the IDirectDraw and the 
IDirectDraw2 interfaces and provides a new method that can retrieve a pointer to an IDirectDrawSurface interface, 
given a handle to a device context.
To obtain the IDirectDraw3 interface, you must call the IDirectDrawFactory::CreateDirectDraw method to create the 
DirectDraw object and expose the IUnknown and IDirectDraw interfaces. Applications can then call QueryInterface to 
obtain a pointer to the IDirectDraw3 interface. To view sample code that demonstrates this, see Creating DirectDraw 
Objects and Surfaces with DirectDrawEx.
Another advantage of using DirectDrawEx over using DirectDraw is that you can now aggregate inner objects with outer 
objects by using the IDirectDraw3::CreateSurface method. Formerly, IDirectDraw::CreateSurface and 
IDirectDraw2::CreateSurface did not provide COM aggregation features. 
For a thorough description of how IDirectDraw3 implements aggregation see, IDirectDraw3::CreateSurface.

Finally, DirectDrawEx now also provides the DDSCAPS_DATAEXCHANGE flag for the DDSCAPS structure's dwcaps member. 
Setting this flag in conjunction with the DDSCAPS_OWNDC flag enables applications to call the 
IDirectDrawSurface::GetDC method to lock the device context for as long they require, without holding a lock on the surface.

Creating DirectDraw Objects and Surfaces with DirectDrawEx

The following sample code demonstrates how to create a DirectDraw object by using DirectDrawEx, and get a pointer to 
the IDirectDraw3 interface. The code shows how to create and call DirectDraw objects.

#include ddrawex.h

void CreateDDEx()
{
	//Declarations
	HRESULT 	hr;
	IDirectDraw 	*pDD;    
	IDirectDraw3 	*pDD3; 
	IDirectDrawFactory *pDDF;

	//Initialize COM library	
	CoInitialize(NULL);


	//Create a DirectDrawFactory object and get  
	//an IDirectDrawFactory interface pointer
	CoCreateInstance(CLSID_DirectDrawFactory, NULL, CLSCTX_INPROC_SERVER, 
							IID_IDirectDrawFactory, (void **)&pDDF);

	//Call the IDirectDrawFactory::CreateDirectDraw method to create the 
	//DirectDraw surface, set the cooperative level, and get the address 
	//of an IDirectDraw interface pointer
	hr = (pDDF->CreateDirectDraw(NULL, GetDesktopWindow(), DDSCL_NORMAL, 
				NULL, NULL, &pDD));

	if (hr !=DD_OK) {//error checking
	}
	
	//Now query for the new IDirectDraw3 interface, and release the old one.
	hr =(pDD->QueryInterface(IID_IDirectDraw3, (LPVOID*)&pDD3));
	
	if (hr !=S_OK) {//error checking
	}
	
	//Release IDirectDraw
	pDD->Release();
	pDD= NULL;	

	//Initialize the DDSURFACEDESC structure for the primary surface
	ZeroMemory(&ddsd, sizeof(ddsd));
      ddsd.dwSize = sizeof(ddsd);    
	ddsd.dwFlags = DDSD_CAPS;
      ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE; 
      hr = pDD3->CreateSurface(&ddsd, &pPrimarySurface, NULL);
	

	//Do whatever you need to do in your application here with your 
	//DirectDraw surface

	//Release IDirectDraw3, IDirectDrawFactory, and the DirectDraw surface
	pDD3->Release();
	pDDF->Release();
	pPrimarySurface->Release();    

	//Close the COM library
	CoUninitialize();
}

Distinctions Between DirectDraw and DirectDrawEx
One important distinction to note between DirectDrawEx and DirectDraw is that applications that have created 
multiple DirectDrawSurfaces through a DirectDrawEx surface must release every DirectDraw surface.
Also, calling the GetDDInterface method from any surface created under DirectDrawEx will return a pointer to 
the IUnknown interface instead of a pointer to an IDirectDraw interface. Applications must use the 
IUnknown::QueryInterface method to retrieve the IDirectDraw, IDirectDraw2, or IDirectDraw3 interfaces.
Finally, DirectDrawEx does not currently support blitting between surfaces created by DirectDrawEx and surfaces
created by DirectDraw. Applications should blit only between similar surfaces.

© 1997 Microsoft Corporation. All rights reserved. Terms of Use. 
*/

// from ddrawex.h
//DECLARE_INTERFACE_(IDirectDrawFactory, IUnknown)
//{
//    /*** IUnknown methods ***/
//    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR * ppvObj) PURE;
//    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
//    STDMETHOD_(ULONG,Release) (THIS) PURE;
//    /*** IDirectDrawFactory methods ***/
//    STDMETHOD(CreateDirectDraw) (THIS_ GUID * pGUID, HWND hWnd, DWORD dwCoopLevelFlags, DWORD dwReserved, IUnknown *pUnkOuter, IDirectDraw **ppDirectDraw) PURE;
//    STDMETHOD(DirectDrawEnumerate) (THIS_ LPDDENUMCALLBACK lpCallback, LPVOID lpContext) PURE;
//};

// DirectDrawEx API (undocumented)
HRESULT WINAPI extDllCanUnloadNow(void);
HRESULT WINAPI extDllGetClassObject(REFCLSID, REFIID, void **);
HRESULT WINAPI extDllRegisterServer(void);
HRESULT WINAPI extDllUnregisterServer(void);

typedef HRESULT (WINAPI *DllCanUnloadNow_Type)(void);
typedef HRESULT (WINAPI *DllGetClassObject_Type)(REFCLSID, REFIID, void **);
typedef HRESULT (WINAPI *DllRegisterServer_Type)(void);
typedef HRESULT (WINAPI *DllUnregisterServer_Type)(void);

DllCanUnloadNow_Type pDllCanUnloadNow;
DllGetClassObject_Type pDllGetClassObject;
DllRegisterServer_Type pDllRegisterServer;
DllUnregisterServer_Type pDllUnregisterServer;

// DirectDrawEx class objects
HRESULT WINAPI extCreateDirectDraw(void *, GUID *, HWND, DWORD, DWORD, IUnknown *, IDirectDraw **);
HRESULT WINAPI extDirectDrawEnumerateEX(void *, LPDDENUMCALLBACK, LPVOID);

typedef HRESULT (WINAPI *CreateDirectDrawEX_Type)(void *, GUID *, HWND, DWORD, DWORD, IUnknown *, IDirectDraw **);
typedef HRESULT (WINAPI *DirectDrawEnumerateEX_Type)(void *, LPDDENUMCALLBACK, LPVOID);

CreateDirectDrawEX_Type pCreateDirectDrawEX = NULL;
DirectDrawEnumerateEX_Type pDirectDrawEnumerateEX = NULL;

// static functions

static char *sGUID(GUID *pGUID)
{
	static char sGUIDBuffer[81];
	switch ((DWORD)pGUID){
		case 0: return "(NULL)"; break;
		case DDCREATE_EMULATIONONLY: return "DDCREATE_EMULATIONONLY"; break;
		case DDCREATE_HARDWAREONLY:  return "DDCREATE_HARDWAREONLY"; break;
		default: sprintf(sGUIDBuffer, "%x.%x.%x.%x", pGUID->Data1, pGUID->Data2, pGUID->Data3, pGUID->Data4); return sGUIDBuffer; break;
	}
}

// Library hook

static HookEntryEx_Type ddexHooks[]={
	{HOOK_HOT_CANDIDATE, 0x01, "DllCanUnloadNow", (FARPROC)NULL, (FARPROC *)&pDllCanUnloadNow, (FARPROC)extDllCanUnloadNow},
	{HOOK_HOT_CANDIDATE, 0x02, "DllGetClassObject", (FARPROC)NULL, (FARPROC *)&pDllGetClassObject, (FARPROC)extDllGetClassObject},
	{HOOK_HOT_CANDIDATE, 0x03, "DllRegisterServer", (FARPROC)NULL, (FARPROC *)&pDllRegisterServer, (FARPROC)extDllRegisterServer},
	{HOOK_HOT_CANDIDATE, 0x04, "DllUnregisterServer", (FARPROC)NULL, (FARPROC *)&pDllUnregisterServer, (FARPROC)extDllUnregisterServer},
	{HOOK_IAT_CANDIDATE, 0, 0, NULL, 0, 0} // terminator
};

void HookDirectDrawFactoryLib(HMODULE module)
{
	HookLibraryEx(module, ddexHooks, "ddrawex.dll");
}

// Objects Hook

void HookDirectDrawFactory(void *obj)
{
	//MessageBox(0,"Hooking IID_DirectDrawFactory object", "DxWnd", MB_OK);
	OutTrace("Hooking IID_DirectDrawFactory object\n");
	//SetHook((void *)(**(DWORD **)obj      ), extQueryInterfaceD1, (void **)&pQueryInterfaceD1, "QueryInterface(D1)");
	//SetHook((void *)(**(DWORD **)obj +   8), extReleaseD1, (void **)&pReleaseD1, "Release(D1)");
	SetHook((void *)(**(DWORD **)obj +  12), extCreateDirectDraw, (void **)&pCreateDirectDrawEX, "CreateDirectDraw(ex)");
	SetHook((void *)(**(DWORD **)obj +  16), extDirectDrawEnumerateEX, (void **)&pDirectDrawEnumerateEX, "DirectDrawEnumerate(ex)");
}

// API wrappers

HRESULT WINAPI extDllCanUnloadNow(void)
{
	HRESULT res;
	OutTraceDW("ddrawex#DllCanUnloadNow\n");
	res = (*pDllCanUnloadNow)();
	if(res) OutTraceE("ddrawex#DllCanUnloadNow ERROR: res=%x\n", res);
	return res;
}

HRESULT WINAPI extDllGetClassObject(REFCLSID rclsid, REFIID riid, void **out)
{
	HRESULT res;
	OutTraceDW("ddrawex#DllGetClassObject: clsid=%x refiid=%x\n", rclsid, riid);
	res = (*pDllGetClassObject)(rclsid, riid, out);
	if(res) OutTraceE("ddrawex#DllGetClassObject ERROR: res=%x\n", res);
	return res;
}

HRESULT WINAPI extDllRegisterServer(void)
{
	HRESULT res;
	OutTraceDW("ddrawex#DllRegisterServer\n");
	res = (*pDllRegisterServer)();
	if(res) OutTraceE("ddrawex#DllRegisterServer ERROR: res=%x\n", res);
	return res;
}

HRESULT WINAPI extDllUnregisterServer(void)
{
	HRESULT res;
	OutTraceDW("ddrawex#DllUnregisterServer\n");
	res = (*pDllUnregisterServer)();
	if(res) OutTraceE("ddrawex#DllUnregisterServer ERROR: res=%x\n", res);
	return res;
}

// COM bject wrappers

HRESULT WINAPI extCreateDirectDraw(void *ddf, GUID *pGUID, HWND hWnd, DWORD dwCoopLevelFlags, DWORD dwReserved, IUnknown *pUnkOuter, IDirectDraw **ppDirectDraw)
{
	HRESULT res;
	if(IsTraceDW){
		OutTrace("CreateDirectDraw(EX): factory=%x guid=%s hwnd=%x coopflags=%x(%s)\n", 
			ddf, sGUID(pGUID), hWnd, dwCoopLevelFlags, ExplainCoopFlags(dwCoopLevelFlags));
	}

	res = (*pCreateDirectDrawEX)(ddf, pGUID, hWnd, dwCoopLevelFlags, dwReserved, pUnkOuter, ppDirectDraw);
	if(res){
		OutTraceE("CreateDirectDraw(EX) ERROR: res=%x\n");
	}
	else {
		// CreateDirectDraw can load an unreferences ddraw.dll module, so it's time now to hook it.
		extern DirectDrawCreate_Type pDirectDrawCreate;
		if(pDirectDrawCreate == NULL){
			HINSTANCE hinst;
			hinst=(*pLoadLibraryA)("ddraw.dll");
			HookDirectDraw(hinst, 1);
			FreeLibrary(hinst);
		}
		OutTraceDW("CreateDirectDraw(EX): guid=%s pDirectDraw=%x\n", sGUID(pGUID), *ppDirectDraw);
	}

	return res;
}

HRESULT WINAPI extDirectDrawEnumerateEX(void *ddf, LPDDENUMCALLBACK lpCallback, LPVOID lpContext)
{
	HRESULT res;
	OutTraceDW("DirectDrawEnumerate(EX): factory=%x\n", ddf);
	res = (*pDirectDrawEnumerateEX)(ddf, lpCallback, lpContext);
	if(res) OutTraceE("DirectDrawEnumerate(EX) ERROR: res=%x\n");
	return res;
}
