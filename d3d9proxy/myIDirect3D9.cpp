// myIDirect3D9.cpp

#include "StdAfx.h"
#include "stdio.h"

#define trace if(log) fprintf

extern FILE *log;

myIDirect3D9::myIDirect3D9(IDirect3D9 *pOriginal)
{
    m_pIDirect3D9 = pOriginal;
}

myIDirect3D9::~myIDirect3D9(void)
{
}

HRESULT  __stdcall myIDirect3D9::QueryInterface(REFIID riid, void** ppvObj)
{
    *ppvObj = NULL;

	// call this to increase AddRef at original object
	// and to check if such an interface is there

	HRESULT hRes = m_pIDirect3D9->QueryInterface(riid, ppvObj); 

	if (hRes == NOERROR) // if OK, send our "fake" address
	{
		*ppvObj = this;
	}
	
	return hRes;
}

ULONG    __stdcall myIDirect3D9::AddRef(void)
{
    return(m_pIDirect3D9->AddRef());
}

ULONG    __stdcall myIDirect3D9::Release(void)
{
    extern myIDirect3D9* gl_pmyIDirect3D9;

	// call original routine
	ULONG count = m_pIDirect3D9->Release();
	
    // in case no further Ref is there, the Original Object has deleted itself
	// so do we here
	if (count == 0){
		gl_pmyIDirect3D9 = NULL;
  	    delete(this); 
	}

	return(count);
}

HRESULT __stdcall myIDirect3D9::RegisterSoftwareDevice(void* pInitializeFunction)
{
	trace(log, "IDirect3D9::RegisterSoftwareDevice\n");
    return(m_pIDirect3D9->RegisterSoftwareDevice(pInitializeFunction));
}

UINT __stdcall myIDirect3D9::GetAdapterCount(void)
{
	trace(log, "IDirect3D9::GetAdapterCount\n");
    return(m_pIDirect3D9->GetAdapterCount());
}

HRESULT __stdcall myIDirect3D9::GetAdapterIdentifier(UINT Adapter,DWORD Flags,D3DADAPTER_IDENTIFIER9* pIdentifier)
{
	trace(log, "IDirect3D9::GetAdapterIdentifier\n");
    return(m_pIDirect3D9->GetAdapterIdentifier(Adapter,Flags,pIdentifier));
}

UINT __stdcall myIDirect3D9::GetAdapterModeCount(UINT Adapter, D3DFORMAT Format)
{
	trace(log, "IDirect3D9::GetAdapterModeCount\n");
    return(m_pIDirect3D9->GetAdapterModeCount(Adapter, Format));
}

HRESULT __stdcall myIDirect3D9::EnumAdapterModes(UINT Adapter,D3DFORMAT Format,UINT Mode,D3DDISPLAYMODE* pMode)
{
	trace(log, "IDirect3D9::EnumAdapterModes\n");
    return(m_pIDirect3D9->EnumAdapterModes(Adapter,Format,Mode,pMode));
}

HRESULT __stdcall myIDirect3D9::GetAdapterDisplayMode( UINT Adapter,D3DDISPLAYMODE* pMode)
{
	trace(log, "IDirect3D9::GetAdapterDisplayMode\n");
    return(m_pIDirect3D9->GetAdapterDisplayMode(Adapter,pMode));
}

HRESULT __stdcall myIDirect3D9::CheckDeviceType(UINT iAdapter,D3DDEVTYPE DevType,D3DFORMAT DisplayFormat,D3DFORMAT BackBufferFormat,BOOL bWindowed)
{
	trace(log, "IDirect3D9::CheckDeviceType\n");
    return(m_pIDirect3D9->CheckDeviceType(iAdapter,DevType,DisplayFormat,BackBufferFormat,bWindowed));
}

HRESULT __stdcall myIDirect3D9::CheckDeviceFormat(UINT Adapter,D3DDEVTYPE DeviceType,D3DFORMAT AdapterFormat,DWORD Usage,D3DRESOURCETYPE RType,D3DFORMAT CheckFormat)
{
	trace(log, "IDirect3D9::CheckDeviceFormat\n");
    return(m_pIDirect3D9->CheckDeviceFormat(Adapter,DeviceType,AdapterFormat,Usage,RType,CheckFormat));
}

HRESULT __stdcall myIDirect3D9::CheckDeviceMultiSampleType(UINT Adapter,D3DDEVTYPE DeviceType,D3DFORMAT SurfaceFormat,BOOL Windowed,D3DMULTISAMPLE_TYPE MultiSampleType,DWORD* pQualityLevels)
{
	trace(log, "IDirect3D9::CheckDeviceMultiSampleType\n");
    return(m_pIDirect3D9->CheckDeviceMultiSampleType(Adapter,DeviceType,SurfaceFormat,Windowed,MultiSampleType,pQualityLevels));
}

HRESULT __stdcall myIDirect3D9::CheckDepthStencilMatch(UINT Adapter,D3DDEVTYPE DeviceType,D3DFORMAT AdapterFormat,D3DFORMAT RenderTargetFormat,D3DFORMAT DepthStencilFormat)
{
	trace(log, "IDirect3D9::CheckDepthStencilMatch\n");
    return(m_pIDirect3D9->CheckDepthStencilMatch(Adapter,DeviceType,AdapterFormat,RenderTargetFormat,DepthStencilFormat));
}

HRESULT __stdcall myIDirect3D9::CheckDeviceFormatConversion(UINT Adapter,D3DDEVTYPE DeviceType,D3DFORMAT SourceFormat,D3DFORMAT TargetFormat)
{
	trace(log, "IDirect3D9::CheckDeviceFormatConversion\n");
    return(m_pIDirect3D9->CheckDeviceFormatConversion(Adapter,DeviceType,SourceFormat,TargetFormat));
}

HRESULT __stdcall myIDirect3D9::GetDeviceCaps(UINT Adapter,D3DDEVTYPE DeviceType,D3DCAPS9* pCaps)
{
	trace(log, "IDirect3D9::GetDeviceCaps\n");
    return(m_pIDirect3D9->GetDeviceCaps(Adapter,DeviceType,pCaps));
}

HMONITOR __stdcall myIDirect3D9::GetAdapterMonitor(UINT Adapter)
{
	trace(log, "IDirect3D9::GetAdapterMonitor\n");
    return(m_pIDirect3D9->GetAdapterMonitor(Adapter));
}

HRESULT __stdcall myIDirect3D9::CreateDevice(UINT Adapter,D3DDEVTYPE DeviceType,HWND hFocusWindow,DWORD BehaviorFlags,D3DPRESENT_PARAMETERS* pPresentationParameters,IDirect3DDevice9** ppReturnedDeviceInterface)
{
	trace(log, "IDirect3D9::CreateDevice\n");
    // global var
	extern myIDirect3DDevice9* gl_pmyIDirect3DDevice9;

	// we intercept this call and provide our own "fake" Device Object
	HRESULT hres = m_pIDirect3D9->CreateDevice( Adapter, DeviceType, hFocusWindow, BehaviorFlags, pPresentationParameters, ppReturnedDeviceInterface);
    
	// Create our own Device object and store it in global pointer
	// note: the object will delete itself once Ref count is zero (similar to COM objects)
	gl_pmyIDirect3DDevice9 = new myIDirect3DDevice9(*ppReturnedDeviceInterface);
	
	// store our pointer (the fake one) for returning it to the calling progam
	*ppReturnedDeviceInterface = gl_pmyIDirect3DDevice9;

	return(hres); 
}
  
