#include <windows.h>
#include <dxdiag.h>
#include "dxwnd.h"
#include "dxwcore.hpp"

// COM generic types
typedef HRESULT (WINAPI *QueryInterface_Type)(void *, REFIID, LPVOID *);

// DxDiag IDxDiagProvider
typedef HRESULT (WINAPI *Initialize_Type)                   (void *, DXDIAG_INIT_PARAMS *);
typedef HRESULT (WINAPI *GetRootContainer_Type)             (void *, IDxDiagContainer **);
// DxDiag IDxDiagContainer
typedef HRESULT (WINAPI *GetNumberOfChildContainers_Type)   (void *, DWORD *);
typedef HRESULT (WINAPI *EnumChildContainerNames_Type)      (void *, DWORD, LPWSTR, DWORD);
typedef HRESULT (WINAPI *GetChildContainer_Type)            (void *, LPCWSTR, IDxDiagContainer **);  
typedef HRESULT (WINAPI *GetNumberOfProps_Type)             (void *, DWORD *);
typedef HRESULT (WINAPI *EnumPropNames_Type)                (void *, DWORD, LPWSTR, DWORD);
typedef HRESULT (WINAPI *GetProp_Type)                      (void *, LPCWSTR, VARIANT *);

typedef HRESULT (WINAPI *Initialize_Type)(void *, DXDIAG_INIT_PARAMS *);
typedef HRESULT (WINAPI *GetRootContainer_Type)(void *, IDxDiagContainer **);

HRESULT WINAPI extInitializeDD(void *, DXDIAG_INIT_PARAMS *);
HRESULT WINAPI extGetRootContainer(void *, IDxDiagContainer **);
HRESULT WINAPI extGetNumberOfChildContainers(void *, DWORD *);
HRESULT WINAPI extQueryInterfaceDD(void *, REFIID, LPVOID *);
HRESULT WINAPI extGetProp(void *, LPCWSTR, VARIANT *);

Initialize_Type pInitializeDD;
GetRootContainer_Type pGetRootContainer;
GetNumberOfChildContainers_Type pGetNumberOfChildContainers;
EnumChildContainerNames_Type pEnumChildContainerNames;
GetChildContainer_Type pGetChildContainer;  
GetNumberOfProps_Type pGetNumberOfProps;
EnumPropNames_Type pEnumPropNames;
GetProp_Type pGetProp;
QueryInterface_Type pQueryInterfaceDD;

HRESULT HookDxDiag(REFIID riid, LPVOID FAR* ppv)
{
	HMODULE dxdlib;

	OutTraceD("CoCreateInstance: CLSID_DxDiagProvider object\n");
	dxdlib=(*pLoadLibraryA)("dxdiagn.dll");
	OutTraceD("CoCreateInstance: dxdiagn lib handle=%x\n", dxdlib);
	extern void HookModule(HMODULE, int);
	HookModule(dxdlib, 0);

	switch (*(DWORD *)&riid){
	case 0x9C6B4CB0:
		OutTraceD("CoCreateInstance: IID_DxDiagProvider RIID\n");
		// IID_DxDiagProvider::QueryInterface
		SetHook((void *)(**(DWORD **)ppv), extQueryInterfaceDD, (void **)&pQueryInterfaceDD, "QueryInterface(DxDiag)");
		// IID_DxDiagProvider::Initialize
		SetHook((void *)(**(DWORD **)ppv + 12), extInitializeDD, (void **)&pInitializeDD, "Initialize(DxDiag)");
		// IID_DxDiagProvider::Initialize
		SetHook((void *)(**(DWORD **)ppv + 16), extGetRootContainer, (void **)&pGetRootContainer, "GetRootContainer(DxDiag)");
		break;
	case 0x7D0F462F:
		OutTraceD("CoCreateInstance: IID_IDxDiagContainer RIID\n");
		break;
	}

	return DD_OK;
}


HRESULT WINAPI extInitializeDD(void *th, DXDIAG_INIT_PARAMS *pParams)
{
	HRESULT res;
	OutTraceD("DxDiag::Initialize Params=%x\n", pParams);
	res=(*pInitializeDD)(th, pParams);
	OutTraceD("DxDiag::Initialize res=%x\n", res);
	return res;
}

HRESULT WINAPI extGetRootContainer(void *th, IDxDiagContainer **ppInstance)
{
	HRESULT res;
	OutTraceD("DxDiag::GetRootContainer pInstance=%x\n", *ppInstance);
	res=(*pGetRootContainer)(th, ppInstance);
	// IID_IDxDiagContainer::GetNumberOfChildContainers
	SetHook((void *)(**(DWORD **)ppInstance + 12), extGetNumberOfChildContainers, (void **)&pGetNumberOfChildContainers, "GetNumberOfChildContainers(DxDiag)");
	// IID_IDxDiagContainer::GetProp
	SetHook((void *)(**(DWORD **)ppInstance + 32), extGetProp, (void **)&pGetProp, "GetProp(DxDiag)");
	OutTraceD("DxDiag::GetRootContainer res=%x\n", res);
	return res;
}

HRESULT WINAPI extGetNumberOfChildContainers(void *th, DWORD *pdwCount)
{
	HRESULT res;
	OutTraceD("DxDiag::GetNumberOfChildContainers\n");
	res=(*pGetNumberOfChildContainers)(th, pdwCount);
	OutTraceD("DxDiag::GetNumberOfChildContainers res=%x Count=%d\n", res, *pdwCount);
	return res;
}

HRESULT WINAPI extQueryInterfaceDD(void *th, REFIID riid, LPVOID *ppvObj)
{
	HRESULT res;
	OutTraceD("DxDiag::QueryInterface ref=%x\n");
	res=(*pQueryInterfaceDD)(th, riid, ppvObj);
	OutTraceD("DxDiag::QueryInterface res=%x\n", res);
	return res;
}

HRESULT WINAPI extGetProp(void *th, LPCWSTR pwszPropName, VARIANT *pvarProp)
{
	HRESULT res;
	OutTraceD("DxDiag::GetProp PropName=%ls\n", pwszPropName);
	res=(*pGetProp)(th, pwszPropName, pvarProp);
	if(res)
		OutTraceE("DxDiag::GetProp ERROR res=%x\n", res);

	if (!wcsncmp(L"dwDirectXVersionMajor", pwszPropName, sizeof(L"dwDirectXVersionMajor"))){
		OutTraceD("DxDiag::GetProp DirectXVersionMajor=%d\n", *pvarProp);
		//*(DWORD *)pvarProp=10;
		//OutTraceD("DxDiag::GetProp fixed DirectXVersionMajor=%d\n", *pvarProp);
	}
	if (!wcsncmp(L"dwDirectXVersionMinor", pwszPropName, sizeof(L"dwDirectXVersionMinor"))){
		OutTraceD("DxDiag::GetProp DirectXVersionMinor=%d\n", *pvarProp);
		//*(DWORD *)pvarProp=10;
		//OutTraceD("DxDiag::GetProp fixed dwDirectXVersionMinor=%d\n", *pvarProp);
	}
	if (!wcsncmp(L"szDirectXVersionLetter", pwszPropName, sizeof(L"szDirectXVersionLetter"))){
		OutTraceD("DxDiag::GetProp DirectXVersionLetter=%d\n", *pvarProp);
		//*(DWORD *)pvarProp=9;
		//OutTraceD("DxDiag::GetProp fixed szDirectXVersionLetter=%d\n", *pvarProp);
	}
	return res;
}

/*
    STDMETHOD(GetNumberOfChildContainers)   (THIS_ DWORD *pdwCount) PURE;
    STDMETHOD(EnumChildContainerNames)      (THIS_ DWORD dwIndex, LPWSTR pwszContainer, DWORD cchContainer) PURE;
    STDMETHOD(GetChildContainer)            (THIS_ LPCWSTR pwszContainer, IDxDiagContainer **ppInstance) PURE;  
    STDMETHOD(GetNumberOfProps)             (THIS_ DWORD *pdwCount) PURE;
    STDMETHOD(EnumPropNames)                (THIS_ DWORD dwIndex, LPWSTR pwszPropName, DWORD cchPropName) PURE;
    STDMETHOD(GetProp)                      (THIS_ LPCWSTR pwszPropName, VARIANT *pvarProp) PURE;
*/