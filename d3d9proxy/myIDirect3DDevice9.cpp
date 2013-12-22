#include "StdAfx.h"
#include "myIDirect3DDevice9.h"
#include "stdio.h"

#define trace if(log) fprintf

extern FILE *log;

myIDirect3DDevice9::myIDirect3DDevice9(IDirect3DDevice9* pOriginal)
{
    m_pIDirect3DDevice9 = pOriginal; // store the pointer to original object
}

myIDirect3DDevice9::~myIDirect3DDevice9(void)
{
}

HRESULT myIDirect3DDevice9::QueryInterface (REFIID riid, void** ppvObj)
{
	trace(log, "IDirect3DDevice9::QueryInterface\n");
    // check if original dll can provide interface. then send *our* address
	*ppvObj = NULL;

	HRESULT hRes = m_pIDirect3DDevice9->QueryInterface(riid, ppvObj); 

	if (hRes == NOERROR)
	{
		*ppvObj = this;
	}
	
	return hRes;
}

ULONG myIDirect3DDevice9::AddRef(void)
{
	trace(log, "IDirect3DDevice9::AddRef\n");
    return(m_pIDirect3DDevice9->AddRef());
}

ULONG myIDirect3DDevice9::Release(void)
{
	trace(log, "IDirect3DDevice9::Release\n");
	// ATTENTION: This is a booby-trap ! Watch out !
	// If we create our own sprites, surfaces, etc. (thus increasing the ref counter
	// by external action), we need to delete that objects before calling the original
	// Release function	
	
	// global var
	extern myIDirect3DDevice9* gl_pmyIDirect3DDevice9;

    // release/delete own objects
    // .....
	
	// Calling original function now
	ULONG count = m_pIDirect3DDevice9->Release();
		
    // now, the Original Object has deleted itself, so do we here
	if(count == 0){
		gl_pmyIDirect3DDevice9 = NULL;
		delete(this);  // destructor will be called automatically
	}

	return (count);
}

HRESULT myIDirect3DDevice9::TestCooperativeLevel(void)
{
	trace(log, "IDirect3DDevice9::TestCooperativeLevel\n");
    return(m_pIDirect3DDevice9->TestCooperativeLevel());
}

UINT myIDirect3DDevice9::GetAvailableTextureMem(void)
{
	trace(log, "IDirect3DDevice9::GetAvailableTextureMem\n");
    return(m_pIDirect3DDevice9->GetAvailableTextureMem());
}

HRESULT myIDirect3DDevice9::EvictManagedResources(void)
{
	trace(log, "IDirect3DDevice9::EvictManagedResources\n");
    return(m_pIDirect3DDevice9->EvictManagedResources());
}

HRESULT myIDirect3DDevice9::GetDirect3D(IDirect3D9** ppD3D9)
{
	trace(log, "IDirect3DDevice9::GetDirect3D\n");
    return(m_pIDirect3DDevice9->GetDirect3D(ppD3D9));
}

HRESULT myIDirect3DDevice9::GetDeviceCaps(D3DCAPS9* pCaps)
{
	trace(log, "IDirect3DDevice9::GetDeviceCaps\n");
    return(m_pIDirect3DDevice9->GetDeviceCaps(pCaps));
}

HRESULT myIDirect3DDevice9::GetDisplayMode(UINT iSwapChain,D3DDISPLAYMODE* pMode)
{
	trace(log, "IDirect3DDevice9::GetDisplayMode\n");
    return(m_pIDirect3DDevice9->GetDisplayMode(iSwapChain, pMode));
}

HRESULT myIDirect3DDevice9::GetCreationParameters(D3DDEVICE_CREATION_PARAMETERS *pParameters)
{
	trace(log, "IDirect3DDevice9::GetCreationParameters\n");
    return(m_pIDirect3DDevice9->GetCreationParameters(pParameters));
}

HRESULT myIDirect3DDevice9::SetCursorProperties(UINT XHotSpot,UINT YHotSpot,IDirect3DSurface9* pCursorBitmap)
{
	trace(log, "IDirect3DDevice9::SetCursorProperties\n");
    return(m_pIDirect3DDevice9->SetCursorProperties(XHotSpot,YHotSpot,pCursorBitmap));
}

void    myIDirect3DDevice9::SetCursorPosition(int X,int Y,DWORD Flags)
{
	trace(log, "IDirect3DDevice9::SetCursorPosition\n");
    return(m_pIDirect3DDevice9->SetCursorPosition(X,Y,Flags));
}

BOOL    myIDirect3DDevice9::ShowCursor(BOOL bShow)
{
	trace(log, "IDirect3DDevice9::ShowCursor\n");
    return(m_pIDirect3DDevice9->ShowCursor(bShow));
}

HRESULT myIDirect3DDevice9::CreateAdditionalSwapChain(D3DPRESENT_PARAMETERS* pPresentationParameters,IDirect3DSwapChain9** pSwapChain)  
{
	trace(log, "IDirect3DDevice9::CreateAdditionalSwapChain\n");
    return(m_pIDirect3DDevice9->CreateAdditionalSwapChain(pPresentationParameters,pSwapChain));
}

HRESULT myIDirect3DDevice9::GetSwapChain(UINT iSwapChain,IDirect3DSwapChain9** pSwapChain)
{
	trace(log, "IDirect3DDevice9::GetSwapChain\n");
    return(m_pIDirect3DDevice9->GetSwapChain(iSwapChain,pSwapChain));
}

UINT    myIDirect3DDevice9::GetNumberOfSwapChains(void)
{
	trace(log, "IDirect3DDevice9::GetNumberOfSwapChains\n");
    return(m_pIDirect3DDevice9->GetNumberOfSwapChains());
}

HRESULT myIDirect3DDevice9::Reset(D3DPRESENT_PARAMETERS* pPresentationParameters)
{
	trace(log, "IDirect3DDevice9::Reset\n");
    return(m_pIDirect3DDevice9->Reset(pPresentationParameters));
}

HRESULT myIDirect3DDevice9::Present(CONST RECT* pSourceRect,CONST RECT* pDestRect,HWND hDestWindowOverride,CONST RGNDATA* pDirtyRegion)
{
	trace(log, "IDirect3DDevice9::Present\n");
    // we may want to draw own things here before flipping surfaces
    // ... draw own stuff ...
	// this->ShowWeAreHere();
    
    // call original routine
	HRESULT hres = m_pIDirect3DDevice9->Present( pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion);

	return (hres);
}

HRESULT myIDirect3DDevice9::GetBackBuffer(UINT iSwapChain,UINT iBackBuffer,D3DBACKBUFFER_TYPE Type,IDirect3DSurface9** ppBackBuffer)
{
	trace(log, "IDirect3DDevice9::GetBackBuffer\n");
    return(m_pIDirect3DDevice9->GetBackBuffer(iSwapChain,iBackBuffer,Type,ppBackBuffer));
}

HRESULT myIDirect3DDevice9::GetRasterStatus(UINT iSwapChain,D3DRASTER_STATUS* pRasterStatus)
{
	trace(log, "IDirect3DDevice9::GetRasterStatus\n");
    return(m_pIDirect3DDevice9->GetRasterStatus(iSwapChain,pRasterStatus));
}

HRESULT myIDirect3DDevice9::SetDialogBoxMode(BOOL bEnableDialogs)
{
	trace(log, "IDirect3DDevice9::SetDialogBoxMode\n");
    return(m_pIDirect3DDevice9->SetDialogBoxMode(bEnableDialogs));
}

void    myIDirect3DDevice9::SetGammaRamp(UINT iSwapChain,DWORD Flags,CONST D3DGAMMARAMP* pRamp)
{
	trace(log, "IDirect3DDevice9::SetGammaRamp\n");
    return(m_pIDirect3DDevice9->SetGammaRamp(iSwapChain,Flags,pRamp));
}

void    myIDirect3DDevice9::GetGammaRamp(UINT iSwapChain,D3DGAMMARAMP* pRamp)
{
	trace(log, "IDirect3DDevice9::GetGammaRamp\n");
    return(m_pIDirect3DDevice9->GetGammaRamp(iSwapChain,pRamp));
}

HRESULT myIDirect3DDevice9::CreateTexture(UINT Width,UINT Height,UINT Levels,DWORD Usage,D3DFORMAT Format,D3DPOOL Pool,IDirect3DTexture9** ppTexture,HANDLE* pSharedHandle)
{
	trace(log, "IDirect3DDevice9::CreateTexture\n");
    return(m_pIDirect3DDevice9->CreateTexture(Width,Height,Levels,Usage,Format,Pool,ppTexture,pSharedHandle));
}

HRESULT myIDirect3DDevice9::CreateVolumeTexture(UINT Width,UINT Height,UINT Depth,UINT Levels,DWORD Usage,D3DFORMAT Format,D3DPOOL Pool,IDirect3DVolumeTexture9** ppVolumeTexture,HANDLE* pSharedHandle)
{
	trace(log, "IDirect3DDevice9::CreateVolumeTexture\n");
    return(m_pIDirect3DDevice9->CreateVolumeTexture(Width,Height,Depth,Levels,Usage,Format,Pool,ppVolumeTexture,pSharedHandle));
}

HRESULT myIDirect3DDevice9::CreateCubeTexture(UINT EdgeLength,UINT Levels,DWORD Usage,D3DFORMAT Format,D3DPOOL Pool,IDirect3DCubeTexture9** ppCubeTexture,HANDLE* pSharedHandle)
{
	trace(log, "IDirect3DDevice9::CreateCubeTexture\n");
    return(m_pIDirect3DDevice9->CreateCubeTexture(EdgeLength,Levels,Usage,Format,Pool,ppCubeTexture,pSharedHandle));
}

HRESULT myIDirect3DDevice9::CreateVertexBuffer(UINT Length,DWORD Usage,DWORD FVF,D3DPOOL Pool,IDirect3DVertexBuffer9** ppVertexBuffer,HANDLE* pSharedHandle)
{
	trace(log, "IDirect3DDevice9::CreateVertexBuffer\n");
    return(m_pIDirect3DDevice9->CreateVertexBuffer(Length,Usage,FVF,Pool,ppVertexBuffer,pSharedHandle));
}

HRESULT myIDirect3DDevice9::CreateIndexBuffer(UINT Length,DWORD Usage,D3DFORMAT Format,D3DPOOL Pool,IDirect3DIndexBuffer9** ppIndexBuffer,HANDLE* pSharedHandle)
{
	trace(log, "IDirect3DDevice9::CreateIndexBuffer\n");
    return(m_pIDirect3DDevice9->CreateIndexBuffer(Length,Usage,Format,Pool,ppIndexBuffer,pSharedHandle));
}

HRESULT myIDirect3DDevice9::CreateRenderTarget(UINT Width,UINT Height,D3DFORMAT Format,D3DMULTISAMPLE_TYPE MultiSample,DWORD MultisampleQuality,BOOL Lockable,IDirect3DSurface9** ppSurface,HANDLE* pSharedHandle)
{
	trace(log, "IDirect3DDevice9::CreateRenderTarget\n");
    return(m_pIDirect3DDevice9->CreateRenderTarget(Width,Height,Format,MultiSample,MultisampleQuality,Lockable,ppSurface,pSharedHandle));
}

HRESULT myIDirect3DDevice9::CreateDepthStencilSurface(UINT Width,UINT Height,D3DFORMAT Format,D3DMULTISAMPLE_TYPE MultiSample,DWORD MultisampleQuality,BOOL Discard,IDirect3DSurface9** ppSurface,HANDLE* pSharedHandle)
{
	trace(log, "IDirect3DDevice9::CreateDepthStencilSurface\n");
    return(m_pIDirect3DDevice9->CreateDepthStencilSurface(Width,Height,Format,MultiSample,MultisampleQuality,Discard,ppSurface,pSharedHandle));
}

HRESULT myIDirect3DDevice9::UpdateSurface(IDirect3DSurface9* pSourceSurface,CONST RECT* pSourceRect,IDirect3DSurface9* pDestinationSurface,CONST POINT* pDestPoint)
{
	trace(log, "IDirect3DDevice9::UpdateSurface\n");
    return(m_pIDirect3DDevice9->UpdateSurface(pSourceSurface,pSourceRect,pDestinationSurface,pDestPoint));
}

HRESULT myIDirect3DDevice9::UpdateTexture(IDirect3DBaseTexture9* pSourceTexture,IDirect3DBaseTexture9* pDestinationTexture)
{
	trace(log, "IDirect3DDevice9::UpdateTexture\n");
    return(m_pIDirect3DDevice9->UpdateTexture(pSourceTexture,pDestinationTexture));
}

HRESULT myIDirect3DDevice9::GetRenderTargetData(IDirect3DSurface9* pRenderTarget,IDirect3DSurface9* pDestSurface)
{
	trace(log, "IDirect3DDevice9::GetRenderTargetData\n");
    return(m_pIDirect3DDevice9->GetRenderTargetData(pRenderTarget,pDestSurface));
}

HRESULT myIDirect3DDevice9::GetFrontBufferData(UINT iSwapChain,IDirect3DSurface9* pDestSurface)
{
	trace(log, "IDirect3DDevice9::GetFrontBufferData\n");
    return(m_pIDirect3DDevice9->GetFrontBufferData(iSwapChain,pDestSurface));
}

HRESULT myIDirect3DDevice9::StretchRect(IDirect3DSurface9* pSourceSurface,CONST RECT* pSourceRect,IDirect3DSurface9* pDestSurface,CONST RECT* pDestRect,D3DTEXTUREFILTERTYPE Filter)
{
	trace(log, "IDirect3DDevice9::StretchRect\n");
    return(m_pIDirect3DDevice9->StretchRect(pSourceSurface,pSourceRect,pDestSurface,pDestRect,Filter));
}

HRESULT myIDirect3DDevice9::ColorFill(IDirect3DSurface9* pSurface,CONST RECT* pRect,D3DCOLOR color)
{
	trace(log, "IDirect3DDevice9::ColorFill\n");
    return(m_pIDirect3DDevice9->ColorFill(pSurface,pRect,color));
}

HRESULT myIDirect3DDevice9::CreateOffscreenPlainSurface(UINT Width,UINT Height,D3DFORMAT Format,D3DPOOL Pool,IDirect3DSurface9** ppSurface,HANDLE* pSharedHandle)
{
	trace(log, "IDirect3DDevice9::CreateOffscreenPlainSurface\n");
    return(m_pIDirect3DDevice9->CreateOffscreenPlainSurface(Width,Height,Format,Pool,ppSurface,pSharedHandle));
}

HRESULT myIDirect3DDevice9::SetRenderTarget(DWORD RenderTargetIndex,IDirect3DSurface9* pRenderTarget)
{
	trace(log, "IDirect3DDevice9::SetRenderTarget\n");
    return(m_pIDirect3DDevice9->SetRenderTarget(RenderTargetIndex,pRenderTarget));
}

HRESULT myIDirect3DDevice9::GetRenderTarget(DWORD RenderTargetIndex,IDirect3DSurface9** ppRenderTarget)
{
	trace(log, "IDirect3DDevice9::GetRenderTarget\n");
    return(m_pIDirect3DDevice9->GetRenderTarget(RenderTargetIndex,ppRenderTarget));
}

HRESULT myIDirect3DDevice9::SetDepthStencilSurface(IDirect3DSurface9* pNewZStencil)
{
	trace(log, "IDirect3DDevice9::SetDepthStencilSurface\n");
    return(m_pIDirect3DDevice9->SetDepthStencilSurface(pNewZStencil));
}

HRESULT myIDirect3DDevice9::GetDepthStencilSurface(IDirect3DSurface9** ppZStencilSurface)
{
	trace(log, "IDirect3DDevice9::GetDepthStencilSurface\n");
    return(m_pIDirect3DDevice9->GetDepthStencilSurface(ppZStencilSurface));
}

HRESULT myIDirect3DDevice9::BeginScene(void)
{
	trace(log, "IDirect3DDevice9::BeginScene\n");
    return(m_pIDirect3DDevice9->BeginScene());
}

HRESULT myIDirect3DDevice9::EndScene(void)
{
	trace(log, "IDirect3DDevice9::EndScene\n");
    return(m_pIDirect3DDevice9->EndScene());
}

HRESULT myIDirect3DDevice9::Clear(DWORD Count,CONST D3DRECT* pRects,DWORD Flags,D3DCOLOR Color,float Z,DWORD Stencil)
{
	trace(log, "IDirect3DDevice9::Clear\n");
    return(m_pIDirect3DDevice9->Clear(Count,pRects,Flags,Color,Z,Stencil));
}

HRESULT myIDirect3DDevice9::SetTransform(D3DTRANSFORMSTATETYPE State,CONST D3DMATRIX* pMatrix)
{
	trace(log, "IDirect3DDevice9::SetTransform\n");
    return(m_pIDirect3DDevice9->SetTransform(State,pMatrix));
}

HRESULT myIDirect3DDevice9::GetTransform(D3DTRANSFORMSTATETYPE State,D3DMATRIX* pMatrix)
{
	trace(log, "IDirect3DDevice9::GetTransform\n");
    return(m_pIDirect3DDevice9->GetTransform(State,pMatrix));
}

HRESULT myIDirect3DDevice9::MultiplyTransform(D3DTRANSFORMSTATETYPE State,CONST D3DMATRIX* pMatrix)
{
	trace(log, "IDirect3DDevice9::MultiplyTransform\n");
    return(m_pIDirect3DDevice9->MultiplyTransform(State,pMatrix));
}

HRESULT myIDirect3DDevice9::SetViewport(CONST D3DVIEWPORT9* pViewport)
{
	trace(log, "IDirect3DDevice9::SetViewport\n");
    return(m_pIDirect3DDevice9->SetViewport(pViewport));
}

HRESULT myIDirect3DDevice9::GetViewport(D3DVIEWPORT9* pViewport)
{
	trace(log, "IDirect3DDevice9::GetViewport\n");
    return(m_pIDirect3DDevice9->GetViewport(pViewport));
}

HRESULT myIDirect3DDevice9::SetMaterial(CONST D3DMATERIAL9* pMaterial)
{
	trace(log, "IDirect3DDevice9::SetMaterial\n");
    return(m_pIDirect3DDevice9->SetMaterial(pMaterial));
}

HRESULT myIDirect3DDevice9::GetMaterial(D3DMATERIAL9* pMaterial)
{
	trace(log, "IDirect3DDevice9::GetMaterial\n");
    return(m_pIDirect3DDevice9->GetMaterial(pMaterial));
}

HRESULT myIDirect3DDevice9::SetLight(DWORD Index,CONST D3DLIGHT9* pLight)
{
	trace(log, "IDirect3DDevice9::SetLight\n");
    return(m_pIDirect3DDevice9->SetLight(Index,pLight));
}

HRESULT myIDirect3DDevice9::GetLight(DWORD Index,D3DLIGHT9* pLight)
{
	trace(log, "IDirect3DDevice9::GetLight\n");
    return(m_pIDirect3DDevice9->GetLight(Index,pLight));
}

HRESULT myIDirect3DDevice9::LightEnable(DWORD Index,BOOL Enable)
{
	trace(log, "IDirect3DDevice9::LightEnable\n");
    return(m_pIDirect3DDevice9->LightEnable(Index,Enable));
}

HRESULT myIDirect3DDevice9::GetLightEnable(DWORD Index,BOOL* pEnable)
{
	trace(log, "IDirect3DDevice9::GetLightEnable\n");
    return(m_pIDirect3DDevice9->GetLightEnable(Index, pEnable));
}

HRESULT myIDirect3DDevice9::SetClipPlane(DWORD Index,CONST float* pPlane)
{
	trace(log, "IDirect3DDevice9::SetClipPlane\n");
    return(m_pIDirect3DDevice9->SetClipPlane(Index, pPlane));
}

HRESULT myIDirect3DDevice9::GetClipPlane(DWORD Index,float* pPlane)
{
	trace(log, "IDirect3DDevice9::GetClipPlane\n");
    return(m_pIDirect3DDevice9->GetClipPlane(Index,pPlane));
}

HRESULT myIDirect3DDevice9::SetRenderState(D3DRENDERSTATETYPE State,DWORD Value)
{
	//trace(log, "IDirect3DDevice9::SetRenderState\n");
    return(m_pIDirect3DDevice9->SetRenderState(State, Value));
}

HRESULT myIDirect3DDevice9::GetRenderState(D3DRENDERSTATETYPE State,DWORD* pValue)
{
	trace(log, "IDirect3DDevice9::GetRenderState\n");
    return(m_pIDirect3DDevice9->GetRenderState(State, pValue));
}

HRESULT myIDirect3DDevice9::CreateStateBlock(D3DSTATEBLOCKTYPE Type,IDirect3DStateBlock9** ppSB)
{
	trace(log, "IDirect3DDevice9::CreateStateBlock\n");
    return(m_pIDirect3DDevice9->CreateStateBlock(Type,ppSB));
}

HRESULT myIDirect3DDevice9::BeginStateBlock(void)
{
	trace(log, "IDirect3DDevice9::BeginStateBlock\n");
    return(m_pIDirect3DDevice9->BeginStateBlock());
}

HRESULT myIDirect3DDevice9::EndStateBlock(IDirect3DStateBlock9** ppSB)
{
	trace(log, "IDirect3DDevice9::EndStateBlock\n");
    return(m_pIDirect3DDevice9->EndStateBlock(ppSB));
}

HRESULT myIDirect3DDevice9::SetClipStatus(CONST D3DCLIPSTATUS9* pClipStatus)
{
	trace(log, "IDirect3DDevice9::SetClipStatus\n");
    return(m_pIDirect3DDevice9->SetClipStatus(pClipStatus));
}

HRESULT myIDirect3DDevice9::GetClipStatus(D3DCLIPSTATUS9* pClipStatus)
{
	trace(log, "IDirect3DDevice9::GetClipStatus\n");
    return(m_pIDirect3DDevice9->GetClipStatus( pClipStatus));
}

HRESULT myIDirect3DDevice9::GetTexture(DWORD Stage,IDirect3DBaseTexture9** ppTexture)
{
	trace(log, "IDirect3DDevice9::GetTexture\n");
    return(m_pIDirect3DDevice9->GetTexture(Stage,ppTexture));
}

HRESULT myIDirect3DDevice9::SetTexture(DWORD Stage,IDirect3DBaseTexture9* pTexture)
{
	trace(log, "IDirect3DDevice9::SetTexture\n");
    return(m_pIDirect3DDevice9->SetTexture(Stage,pTexture));
}

HRESULT myIDirect3DDevice9::GetTextureStageState(DWORD Stage,D3DTEXTURESTAGESTATETYPE Type,DWORD* pValue)
{
	trace(log, "IDirect3DDevice9::GetTextureStageState\n");
    return(m_pIDirect3DDevice9->GetTextureStageState(Stage,Type, pValue));
}

HRESULT myIDirect3DDevice9::SetTextureStageState(DWORD Stage,D3DTEXTURESTAGESTATETYPE Type,DWORD Value)
{
	trace(log, "IDirect3DDevice9::SetTextureStageState\n");
    return(m_pIDirect3DDevice9->SetTextureStageState(Stage,Type,Value));
}

HRESULT myIDirect3DDevice9::GetSamplerState(DWORD Sampler,D3DSAMPLERSTATETYPE Type,DWORD* pValue)
{
	trace(log, "IDirect3DDevice9::GetSamplerState\n");
    return(m_pIDirect3DDevice9->GetSamplerState(Sampler,Type, pValue));
}

HRESULT myIDirect3DDevice9::SetSamplerState(DWORD Sampler,D3DSAMPLERSTATETYPE Type,DWORD Value)
{
	trace(log, "IDirect3DDevice9::SetSamplerState\n");
    return(m_pIDirect3DDevice9->SetSamplerState(Sampler,Type,Value));
}

HRESULT myIDirect3DDevice9::ValidateDevice(DWORD* pNumPasses)
{
	trace(log, "IDirect3DDevice9::ValidateDevice\n");
    return(m_pIDirect3DDevice9->ValidateDevice( pNumPasses));
}

HRESULT myIDirect3DDevice9::SetPaletteEntries(UINT PaletteNumber,CONST PALETTEENTRY* pEntries)
{
	trace(log, "IDirect3DDevice9::SetPaletteEntries\n");
    return(m_pIDirect3DDevice9->SetPaletteEntries(PaletteNumber, pEntries));
}

HRESULT myIDirect3DDevice9::GetPaletteEntries(UINT PaletteNumber,PALETTEENTRY* pEntries)
{
	trace(log, "IDirect3DDevice9::GetPaletteEntries\n");
    return(m_pIDirect3DDevice9->GetPaletteEntries(PaletteNumber, pEntries));
}

HRESULT myIDirect3DDevice9::SetCurrentTexturePalette(UINT PaletteNumber)
{
	trace(log, "IDirect3DDevice9::SetCurrentTexturePalette\n");
    return(m_pIDirect3DDevice9->SetCurrentTexturePalette(PaletteNumber));
}

HRESULT myIDirect3DDevice9::GetCurrentTexturePalette(UINT *PaletteNumber)
{
	trace(log, "IDirect3DDevice9::GetCurrentTexturePalette\n");
    return(m_pIDirect3DDevice9->GetCurrentTexturePalette(PaletteNumber));
}

HRESULT myIDirect3DDevice9::SetScissorRect(CONST RECT* pRect)
{
	trace(log, "IDirect3DDevice9::SetScissorRect\n");
    return(m_pIDirect3DDevice9->SetScissorRect( pRect));
}

HRESULT myIDirect3DDevice9::GetScissorRect( RECT* pRect)
{
	trace(log, "IDirect3DDevice9::GetScissorRect\n");
    return(m_pIDirect3DDevice9->GetScissorRect( pRect));
}

HRESULT myIDirect3DDevice9::SetSoftwareVertexProcessing(BOOL bSoftware)
{
	trace(log, "IDirect3DDevice9::SetSoftwareVertexProcessing\n");
    return(m_pIDirect3DDevice9->SetSoftwareVertexProcessing(bSoftware));
}

BOOL    myIDirect3DDevice9::GetSoftwareVertexProcessing(void)
{
	trace(log, "IDirect3DDevice9::GetSoftwareVertexProcessing\n");
    return(m_pIDirect3DDevice9->GetSoftwareVertexProcessing());
}

HRESULT myIDirect3DDevice9::SetNPatchMode(float nSegments)
{
	trace(log, "IDirect3DDevice9::SetNPatchMode\n");
    return(m_pIDirect3DDevice9->SetNPatchMode(nSegments));
}

float   myIDirect3DDevice9::GetNPatchMode(void)
{
	trace(log, "IDirect3DDevice9::GetNPatchMode\n");
    return(m_pIDirect3DDevice9->GetNPatchMode());
}

HRESULT myIDirect3DDevice9::DrawPrimitive(D3DPRIMITIVETYPE PrimitiveType,UINT StartVertex,UINT PrimitiveCount)
{
	trace(log, "IDirect3DDevice9::DrawPrimitive\n");
    return(m_pIDirect3DDevice9->DrawPrimitive(PrimitiveType,StartVertex,PrimitiveCount));
}

HRESULT myIDirect3DDevice9::DrawIndexedPrimitive(D3DPRIMITIVETYPE PrimitiveType,INT BaseVertexIndex,UINT MinVertexIndex,UINT NumVertices,UINT startIndex,UINT primCount)
{
	trace(log, "IDirect3DDevice9::DrawIndexedPrimitive\n");
    return(m_pIDirect3DDevice9->DrawIndexedPrimitive(PrimitiveType,BaseVertexIndex,MinVertexIndex,NumVertices,startIndex,primCount));
}

HRESULT myIDirect3DDevice9::DrawPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType,UINT PrimitiveCount,CONST void* pVertexStreamZeroData,UINT VertexStreamZeroStride)
{
	trace(log, "IDirect3DDevice9::DrawPrimitiveUP\n");
    return(m_pIDirect3DDevice9->DrawPrimitiveUP(PrimitiveType,PrimitiveCount,pVertexStreamZeroData,VertexStreamZeroStride));
}

HRESULT myIDirect3DDevice9::DrawIndexedPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType,UINT MinVertexIndex,UINT NumVertices,UINT PrimitiveCount,CONST void* pIndexData,D3DFORMAT IndexDataFormat,CONST void* pVertexStreamZeroData,UINT VertexStreamZeroStride)
{
	trace(log, "IDirect3DDevice9::DrawIndexedPrimitiveUP\n");
    return(m_pIDirect3DDevice9->DrawIndexedPrimitiveUP(PrimitiveType,MinVertexIndex,NumVertices,PrimitiveCount, pIndexData, IndexDataFormat, pVertexStreamZeroData,VertexStreamZeroStride));
}

HRESULT myIDirect3DDevice9::ProcessVertices(UINT SrcStartIndex,UINT DestIndex,UINT VertexCount,IDirect3DVertexBuffer9* pDestBuffer,IDirect3DVertexDeclaration9* pVertexDecl,DWORD Flags)
{
	trace(log, "IDirect3DDevice9::ProcessVertices\n");
    return(m_pIDirect3DDevice9->ProcessVertices( SrcStartIndex, DestIndex, VertexCount, pDestBuffer, pVertexDecl, Flags));
}

HRESULT myIDirect3DDevice9::CreateVertexDeclaration(CONST D3DVERTEXELEMENT9* pVertexElements,IDirect3DVertexDeclaration9** ppDecl)
{
	trace(log, "IDirect3DDevice9::CreateVertexDeclaration\n");
    return(m_pIDirect3DDevice9->CreateVertexDeclaration( pVertexElements,ppDecl));
}

HRESULT myIDirect3DDevice9::SetVertexDeclaration(IDirect3DVertexDeclaration9* pDecl)
{
	trace(log, "IDirect3DDevice9::SetVertexDeclaration\n");
    return(m_pIDirect3DDevice9->SetVertexDeclaration(pDecl));
}

HRESULT myIDirect3DDevice9::GetVertexDeclaration(IDirect3DVertexDeclaration9** ppDecl)
{
	trace(log, "IDirect3DDevice9::GetVertexDeclaration\n");
    return(m_pIDirect3DDevice9->GetVertexDeclaration(ppDecl));
}

HRESULT myIDirect3DDevice9::SetFVF(DWORD FVF)
{
	trace(log, "IDirect3DDevice9::SetFVF\n");
    return(m_pIDirect3DDevice9->SetFVF(FVF));
}

HRESULT myIDirect3DDevice9::GetFVF(DWORD* pFVF)
{
	trace(log, "IDirect3DDevice9::GetFVF\n");
    return(m_pIDirect3DDevice9->GetFVF(pFVF));
}

HRESULT myIDirect3DDevice9::CreateVertexShader(CONST DWORD* pFunction,IDirect3DVertexShader9** ppShader)
{
	trace(log, "IDirect3DDevice9::CreateVertexShader\n");
    return(m_pIDirect3DDevice9->CreateVertexShader(pFunction,ppShader));
}

HRESULT myIDirect3DDevice9::SetVertexShader(IDirect3DVertexShader9* pShader)
{
	trace(log, "IDirect3DDevice9::SetVertexShader\n");
    return(m_pIDirect3DDevice9->SetVertexShader(pShader));
}

HRESULT myIDirect3DDevice9::GetVertexShader(IDirect3DVertexShader9** ppShader)
{
	trace(log, "IDirect3DDevice9::GetVertexShader\n");
    return(m_pIDirect3DDevice9->GetVertexShader(ppShader));
}

HRESULT myIDirect3DDevice9::SetVertexShaderConstantF(UINT StartRegister,CONST float* pConstantData,UINT Vector4fCount)
{
	trace(log, "IDirect3DDevice9::SetVertexShaderConstantF\n");
    return(m_pIDirect3DDevice9->SetVertexShaderConstantF(StartRegister,pConstantData, Vector4fCount));
}

HRESULT myIDirect3DDevice9::GetVertexShaderConstantF(UINT StartRegister,float* pConstantData,UINT Vector4fCount)
{
	trace(log, "IDirect3DDevice9::GetVertexShaderConstantF\n");
    return(m_pIDirect3DDevice9->GetVertexShaderConstantF(StartRegister,pConstantData,Vector4fCount));
}

HRESULT myIDirect3DDevice9::SetVertexShaderConstantI(UINT StartRegister,CONST int* pConstantData,UINT Vector4iCount)
{
	trace(log, "IDirect3DDevice9::SetVertexShaderConstantI\n");
    return(m_pIDirect3DDevice9->SetVertexShaderConstantI(StartRegister,pConstantData,Vector4iCount));
}

HRESULT myIDirect3DDevice9::GetVertexShaderConstantI(UINT StartRegister,int* pConstantData,UINT Vector4iCount)
{
	trace(log, "IDirect3DDevice9::GetVertexShaderConstantI\n");
    return(m_pIDirect3DDevice9->GetVertexShaderConstantI(StartRegister,pConstantData,Vector4iCount));
}

HRESULT myIDirect3DDevice9::SetVertexShaderConstantB(UINT StartRegister,CONST BOOL* pConstantData,UINT  BoolCount)
{
	trace(log, "IDirect3DDevice9::SetVertexShaderConstantB\n");
    return(m_pIDirect3DDevice9->SetVertexShaderConstantB(StartRegister,pConstantData,BoolCount));
}

HRESULT myIDirect3DDevice9::GetVertexShaderConstantB(UINT StartRegister,BOOL* pConstantData,UINT BoolCount)
{
	trace(log, "IDirect3DDevice9::GetVertexShaderConstantB\n");
    return(m_pIDirect3DDevice9->GetVertexShaderConstantB(StartRegister,pConstantData,BoolCount));
}

HRESULT myIDirect3DDevice9::SetStreamSource(UINT StreamNumber,IDirect3DVertexBuffer9* pStreamData,UINT OffsetInBytes,UINT Stride)
{
	trace(log, "IDirect3DDevice9::SetStreamSource\n");
    return(m_pIDirect3DDevice9->SetStreamSource(StreamNumber,pStreamData,OffsetInBytes,Stride));
}

HRESULT myIDirect3DDevice9::GetStreamSource(UINT StreamNumber,IDirect3DVertexBuffer9** ppStreamData,UINT* OffsetInBytes,UINT* pStride)
{
	trace(log, "IDirect3DDevice9::GetStreamSource\n");
    return(m_pIDirect3DDevice9->GetStreamSource(StreamNumber,ppStreamData,OffsetInBytes,pStride));
}

HRESULT myIDirect3DDevice9::SetStreamSourceFreq(UINT StreamNumber,UINT Divider)
{
	trace(log, "IDirect3DDevice9::SetStreamSourceFreq\n");
    return(m_pIDirect3DDevice9->SetStreamSourceFreq(StreamNumber,Divider));
}

HRESULT myIDirect3DDevice9::GetStreamSourceFreq(UINT StreamNumber,UINT* Divider)
{
	trace(log, "IDirect3DDevice9::GetStreamSourceFreq\n");
    return(m_pIDirect3DDevice9->GetStreamSourceFreq(StreamNumber,Divider));
}

HRESULT myIDirect3DDevice9::SetIndices(IDirect3DIndexBuffer9* pIndexData)
{
	trace(log, "IDirect3DDevice9::SetIndices\n");
    return(m_pIDirect3DDevice9->SetIndices(pIndexData));
}

HRESULT myIDirect3DDevice9::GetIndices(IDirect3DIndexBuffer9** ppIndexData)
{
	trace(log, "IDirect3DDevice9::GetIndices\n");
    return(m_pIDirect3DDevice9->GetIndices(ppIndexData));
}

HRESULT myIDirect3DDevice9::CreatePixelShader(CONST DWORD* pFunction,IDirect3DPixelShader9** ppShader)
{
	trace(log, "IDirect3DDevice9::CreatePixelShader\n");
    return(m_pIDirect3DDevice9->CreatePixelShader(pFunction,ppShader));
}

HRESULT myIDirect3DDevice9::SetPixelShader(IDirect3DPixelShader9* pShader)
{
	trace(log, "IDirect3DDevice9::SetPixelShader\n");
    return(m_pIDirect3DDevice9->SetPixelShader(pShader));
}

HRESULT myIDirect3DDevice9::GetPixelShader(IDirect3DPixelShader9** ppShader)
{
	trace(log, "IDirect3DDevice9::GetPixelShader\n");
    return(m_pIDirect3DDevice9->GetPixelShader(ppShader));
}

HRESULT myIDirect3DDevice9::SetPixelShaderConstantF(UINT StartRegister,CONST float* pConstantData,UINT Vector4fCount)
{
	trace(log, "IDirect3DDevice9::SetPixelShaderConstantF\n");
    return(m_pIDirect3DDevice9->SetPixelShaderConstantF(StartRegister,pConstantData,Vector4fCount));
}

HRESULT myIDirect3DDevice9::GetPixelShaderConstantF(UINT StartRegister,float* pConstantData,UINT Vector4fCount)
{
	trace(log, "IDirect3DDevice9::GetPixelShaderConstantF\n");
    return(m_pIDirect3DDevice9->GetPixelShaderConstantF(StartRegister,pConstantData,Vector4fCount));
}

HRESULT myIDirect3DDevice9::SetPixelShaderConstantI(UINT StartRegister,CONST int* pConstantData,UINT Vector4iCount)
{
	trace(log, "IDirect3DDevice9::SetPixelShaderConstantI\n");
    return(m_pIDirect3DDevice9->SetPixelShaderConstantI(StartRegister,pConstantData,Vector4iCount));
}

HRESULT myIDirect3DDevice9::GetPixelShaderConstantI(UINT StartRegister,int* pConstantData,UINT Vector4iCount)
{
	trace(log, "IDirect3DDevice9::GetPixelShaderConstantI\n");
    return(m_pIDirect3DDevice9->GetPixelShaderConstantI(StartRegister,pConstantData,Vector4iCount));
}

HRESULT myIDirect3DDevice9::SetPixelShaderConstantB(UINT StartRegister,CONST BOOL* pConstantData,UINT  BoolCount)
{
	trace(log, "IDirect3DDevice9::SetPixelShaderConstantB\n");
    return(m_pIDirect3DDevice9->SetPixelShaderConstantB(StartRegister,pConstantData,BoolCount));
}

HRESULT myIDirect3DDevice9::GetPixelShaderConstantB(UINT StartRegister,BOOL* pConstantData,UINT BoolCount)
{
	trace(log, "IDirect3DDevice9::GetPixelShaderConstantB\n");
    return(m_pIDirect3DDevice9->GetPixelShaderConstantB(StartRegister,pConstantData,BoolCount));
}

HRESULT myIDirect3DDevice9::DrawRectPatch(UINT Handle,CONST float* pNumSegs,CONST D3DRECTPATCH_INFO* pRectPatchInfo)
{
	trace(log, "IDirect3DDevice9::DrawRectPatch\n");
    return(m_pIDirect3DDevice9->DrawRectPatch(Handle,pNumSegs, pRectPatchInfo));
}

HRESULT myIDirect3DDevice9::DrawTriPatch(UINT Handle,CONST float* pNumSegs,CONST D3DTRIPATCH_INFO* pTriPatchInfo)
{
	trace(log, "IDirect3DDevice9::DrawTriPatch\n");
    return(m_pIDirect3DDevice9->DrawTriPatch(Handle, pNumSegs, pTriPatchInfo));
}

HRESULT myIDirect3DDevice9::DeletePatch(UINT Handle)
{
	trace(log, "IDirect3DDevice9::DeletePatch\n");
    return(m_pIDirect3DDevice9->DeletePatch(Handle));
}

HRESULT myIDirect3DDevice9::CreateQuery(D3DQUERYTYPE Type,IDirect3DQuery9** ppQuery)
{
	trace(log, "IDirect3DDevice9::CreateQuery\n");
    return(m_pIDirect3DDevice9->CreateQuery(Type,ppQuery));
}

// This is our test function
void myIDirect3DDevice9::ShowWeAreHere(void)
{
	D3DRECT rec = {1,1,50,50};
	m_pIDirect3DDevice9->Clear(1, &rec, D3DCLEAR_TARGET, D3DCOLOR_ARGB(255,255,255,0),0 ,0);
}


