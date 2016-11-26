#include <dxdiag.h>

typedef HRESULT (WINAPI *DirectDrawCreate_Type)(GUID *, LPDIRECTDRAW *, IUnknown *);
typedef HRESULT (WINAPI *DirectDrawCreateEx_Type)(GUID *, LPDIRECTDRAW *, REFIID, IUnknown *);
typedef HRESULT (WINAPI *DirectDrawEnumerateA_Type)(LPDDENUMCALLBACKA, LPVOID);
typedef HRESULT (WINAPI *DirectDrawEnumerateExA_Type)(LPDDENUMCALLBACKEXA, LPVOID, DWORD);
typedef HRESULT (WINAPI *DirectDrawEnumerateW_Type)(LPDDENUMCALLBACKW, LPVOID);
typedef HRESULT (WINAPI *DirectDrawEnumerateExW_Type)(LPDDENUMCALLBACKEXW, LPVOID, DWORD);
typedef HRESULT (WINAPI *DirectDrawCreateClipper_Type)(DWORD, LPDIRECTDRAWCLIPPER *, IUnknown *);
typedef void	(WINAPI *HandleDDThreadLock_Type)(void);

typedef HDC (WINAPI *GDIGetDC_Type)(HWND);
typedef int (WINAPI *GDIReleaseDC_Type)(HWND, HDC);

// COM generic types
typedef HRESULT (WINAPI *QueryInterface_Type)(void *, REFIID, LPVOID *);

// DirectDraw
//    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR * ppvObj) PURE;
typedef ULONG (WINAPI *AddRefD_Type)(LPDIRECTDRAW);
typedef ULONG (WINAPI *ReleaseD_Type)(LPDIRECTDRAW);
    /*** IDirectDraw methods ***/
typedef ULONG (WINAPI *Compact_Type)(LPDIRECTDRAW);
typedef HRESULT (WINAPI *CreateClipper_Type)(LPDIRECTDRAW, DWORD, LPDIRECTDRAWCLIPPER FAR* , IUnknown FAR*);
typedef HRESULT (WINAPI *CreatePalette_Type)(LPDIRECTDRAW, DWORD, LPPALETTEENTRY, LPDIRECTDRAWPALETTE *, IUnknown *);
typedef HRESULT (WINAPI *CreateSurface1_Type)(LPDIRECTDRAW, DDSURFACEDESC *, LPDIRECTDRAWSURFACE *, void *);
typedef HRESULT (WINAPI *CreateSurface2_Type)(LPDIRECTDRAW, DDSURFACEDESC2 *, LPDIRECTDRAWSURFACE *, void *);
typedef HRESULT (WINAPI *CreateSurface_Type)(LPDIRECTDRAW, DDSURFACEDESC2 *, LPDIRECTDRAWSURFACE *, void *);
typedef HRESULT (WINAPI *EnumDisplayModes1_Type)(LPDIRECTDRAW, DWORD, LPDDSURFACEDESC, LPVOID, LPDDENUMMODESCALLBACK);
typedef HRESULT (WINAPI *EnumDisplayModes4_Type)(LPDIRECTDRAW, DWORD, LPDDSURFACEDESC2, LPVOID, LPDDENUMMODESCALLBACK2);
typedef HRESULT (WINAPI *EnumSurfaces1_Type)(LPDIRECTDRAW, DWORD, LPDDSURFACEDESC, LPVOID, LPDDENUMSURFACESCALLBACK);
typedef HRESULT (WINAPI *EnumSurfaces4_Type)(LPDIRECTDRAW, DWORD, LPDDSURFACEDESC2, LPVOID, LPDDENUMSURFACESCALLBACK2);
typedef HRESULT (WINAPI *FlipToGDISurface_Type)(LPDIRECTDRAW);
typedef HRESULT (WINAPI *GetCapsD_Type)(LPDIRECTDRAW, LPDDCAPS, LPDDCAPS);
typedef HRESULT (WINAPI *GetDisplayMode_Type)(LPDIRECTDRAW, LPDDSURFACEDESC);
typedef HRESULT (WINAPI *GetDisplayMode4_Type)(LPDIRECTDRAW, LPDDSURFACEDESC2);
typedef HRESULT (WINAPI *GetFourCCCodes_Type)(LPDIRECTDRAW, LPDWORD, LPDWORD);
typedef HRESULT (WINAPI *GetGDISurface_Type)(LPDIRECTDRAW, LPDIRECTDRAWSURFACE *);
typedef HRESULT (WINAPI *GetMonitorFrequency_Type)(LPDIRECTDRAW, LPDWORD);
typedef HRESULT (WINAPI *GetScanLine_Type)(LPDIRECTDRAW, LPDWORD);
typedef HRESULT (WINAPI *GetVerticalBlankStatus_Type)(LPDIRECTDRAW, LPBOOL);
typedef HRESULT (WINAPI *Initialize_Type)(LPDIRECTDRAW, GUID FAR *);
typedef HRESULT (WINAPI *RestoreDisplayMode_Type)(LPDIRECTDRAW);
typedef HRESULT (WINAPI *SetCooperativeLevel_Type)(void *, HWND, DWORD);
typedef HRESULT (WINAPI *SetDisplayMode2_Type)(LPDIRECTDRAW, DWORD, DWORD, DWORD, DWORD, DWORD);
typedef HRESULT (WINAPI *SetDisplayMode1_Type)(LPDIRECTDRAW, DWORD, DWORD, DWORD);
typedef HRESULT (WINAPI *WaitForVerticalBlank_Type)(LPDIRECTDRAW, DWORD, HANDLE);
typedef HRESULT (WINAPI *GetAvailableVidMem_Type)(LPDIRECTDRAW, LPDDSCAPS, LPDWORD, LPDWORD);
typedef HRESULT (WINAPI *GetAvailableVidMem4_Type)(LPDIRECTDRAW, LPDDSCAPS2, LPDWORD, LPDWORD);
typedef HRESULT (WINAPI *GetSurfaceFromDC_Type)(LPDIRECTDRAW, HDC, LPDIRECTDRAWSURFACE*);
typedef HRESULT (WINAPI *RestoreAllSurfaces_Type)(LPDIRECTDRAW); 
typedef HRESULT (WINAPI *TestCooperativeLevel_Type)(LPDIRECTDRAW);
typedef HRESULT (WINAPI *GetDeviceIdentifier_Type)(LPDIRECTDRAW, LPDDDEVICEIDENTIFIER, DWORD);
typedef HRESULT (WINAPI *GetScanLine_Type)(LPDIRECTDRAW, LPDWORD);
//    STDMETHOD(StartModeTest)(THIS_ LPSIZE, DWORD, DWORD ) PURE;
//    STDMETHOD(EvaluateMode)(THIS_ DWORD, DWORD * ) PURE;

// DirectDrawSurface
typedef ULONG (WINAPI *AddRefS_Type)(LPDIRECTDRAWSURFACE);
typedef HRESULT (WINAPI *ReleaseS_Type)(LPDIRECTDRAWSURFACE);
typedef HRESULT (WINAPI *AddAttachedSurface_Type)(LPDIRECTDRAWSURFACE, LPDIRECTDRAWSURFACE);
typedef HRESULT (WINAPI *AddOverlayDirtyRect_Type)(LPDIRECTDRAWSURFACE, LPRECT);
typedef HRESULT (WINAPI *Blt_Type)(LPDIRECTDRAWSURFACE, LPRECT, LPDIRECTDRAWSURFACE, LPRECT, DWORD, LPDDBLTFX);
typedef HRESULT (WINAPI *BltBatch_Type)(LPDIRECTDRAWSURFACE, LPDDBLTBATCH, DWORD, DWORD);
typedef HRESULT (WINAPI *BltFast_Type)(LPDIRECTDRAWSURFACE, DWORD, DWORD, LPDIRECTDRAWSURFACE, LPRECT, DWORD);
typedef HRESULT (WINAPI *DeleteAttachedSurface_Type)(LPDIRECTDRAWSURFACE,  DWORD, LPDIRECTDRAWSURFACE);
typedef HRESULT (WINAPI *EnumAttachedSurfaces_Type)(LPDIRECTDRAWSURFACE, LPVOID, LPDDENUMSURFACESCALLBACK);
typedef HRESULT (WINAPI *EnumOverlayZOrders_Type)(LPDIRECTDRAWSURFACE, DWORD, LPVOID, LPDDENUMSURFACESCALLBACK);
typedef HRESULT (WINAPI *Flip_Type)(LPDIRECTDRAWSURFACE, LPDIRECTDRAWSURFACE, DWORD);
typedef HRESULT (WINAPI *GetAttachedSurface_Type)(LPDIRECTDRAWSURFACE, DDSCAPS *, LPDIRECTDRAWSURFACE *);
typedef HRESULT (WINAPI *GetBltStatus_Type)(LPDIRECTDRAWSURFACE, DWORD);
typedef HRESULT (WINAPI *GetCapsS_Type)(LPDIRECTDRAWSURFACE, LPDDSCAPS);
typedef HRESULT (WINAPI *GetCaps2S_Type)(LPDIRECTDRAWSURFACE, LPDDSCAPS2);
typedef HRESULT (WINAPI *GetClipper_Type)(LPDIRECTDRAWSURFACE, LPDIRECTDRAWCLIPPER FAR*);
typedef HRESULT (WINAPI *GetColorKey_Type)(LPDIRECTDRAWSURFACE, DWORD, LPDDCOLORKEY);
typedef HRESULT (WINAPI *GetDC_Type) (LPDIRECTDRAWSURFACE, HDC FAR *);
typedef HRESULT (WINAPI *GetFlipStatus_Type)(LPDIRECTDRAWSURFACE, DWORD);
typedef HRESULT (WINAPI *GetOverlayPosition_Type)(LPDIRECTDRAWSURFACE, LPLONG, LPLONG);
typedef HRESULT (WINAPI *GetPalette_Type)(LPDIRECTDRAWSURFACE, LPDIRECTDRAWPALETTE *);
typedef HRESULT (WINAPI *GetPixelFormat_Type)(LPDIRECTDRAWSURFACE, LPDDPIXELFORMAT);
typedef HRESULT (WINAPI *GetSurfaceDesc_Type)(LPDIRECTDRAWSURFACE, LPDDSURFACEDESC);
typedef HRESULT (WINAPI *GetSurfaceDesc2_Type)(LPDIRECTDRAWSURFACE2, LPDDSURFACEDESC2);
typedef HRESULT (WINAPI *InitializeC_Type)(LPDIRECTDRAWCLIPPER, LPDIRECTDRAW, DWORD);
typedef HRESULT (WINAPI *IsLost_Type)(LPDIRECTDRAWSURFACE);
typedef HRESULT (WINAPI *Lock_Type)(LPDIRECTDRAWSURFACE, LPRECT, LPDDSURFACEDESC, DWORD, HANDLE);
typedef HRESULT (WINAPI *ReleaseDC_Type)(LPDIRECTDRAWSURFACE, HDC);
typedef HRESULT (WINAPI *Restore_Type)(LPDIRECTDRAWSURFACE);
typedef HRESULT (WINAPI *SetClipper_Type)(LPDIRECTDRAWSURFACE, LPDIRECTDRAWCLIPPER);
typedef HRESULT (WINAPI *SetColorKey_Type)(LPDIRECTDRAWSURFACE, DWORD,  LPDDCOLORKEY);
typedef HRESULT (WINAPI *SetOverlayPosition_Type)(LPDIRECTDRAWSURFACE, LONG, LONG);
typedef HRESULT (WINAPI *SetPalette_Type)(LPDIRECTDRAWSURFACE, LPDIRECTDRAWPALETTE);
typedef HRESULT (WINAPI *Unlock1_Type)(LPDIRECTDRAWSURFACE, LPVOID);
typedef HRESULT (WINAPI *Unlock4_Type)(LPDIRECTDRAWSURFACE, LPRECT);
typedef HRESULT (WINAPI *UpdateOverlay_Type)(LPDIRECTDRAWSURFACE, LPRECT, LPDIRECTDRAWSURFACE, LPRECT, DWORD, LPDDOVERLAYFX);
typedef HRESULT (WINAPI *UpdateOverlayDisplay_Type) (LPDIRECTDRAWSURFACE, DWORD);
typedef HRESULT (WINAPI *UpdateOverlayZOrder_Type)(LPDIRECTDRAWSURFACE, DWORD, LPDIRECTDRAWSURFACE);
typedef HRESULT (WINAPI *SetSurfaceDesc_Type)(LPDIRECTDRAWSURFACE, LPDDSURFACEDESC, DWORD);

// DirectDrawClipper
typedef HRESULT (WINAPI *AddRefC_Type)(LPDIRECTDRAWCLIPPER);
typedef HRESULT (WINAPI *ReleaseC_Type)(LPDIRECTDRAWCLIPPER);
typedef HRESULT (WINAPI *GetClipList_Type)(LPDIRECTDRAWCLIPPER, LPRECT, LPRGNDATA, LPDWORD);
typedef HRESULT (WINAPI *GetHWnd_Type)(LPDIRECTDRAWCLIPPER, HWND FAR *);
typedef HRESULT (WINAPI *IsClipListChanged_Type)(LPDIRECTDRAWCLIPPER, BOOL FAR *);
typedef HRESULT (WINAPI *SetClipList_Type)(LPDIRECTDRAWCLIPPER, LPRGNDATA, DWORD);
typedef HRESULT (WINAPI *SetHWnd_Type)(LPDIRECTDRAWCLIPPER, DWORD, HWND);

// DirectDraw Palette 
    /*** IUnknown methods ***/
typedef HRESULT (WINAPI *QueryInterfaceP_Type)(void *, REFIID, LPVOID *);
typedef HRESULT (WINAPI *AddRefP_Type)(LPDIRECTDRAWPALETTE);
typedef HRESULT (WINAPI *ReleaseP_Type)(LPDIRECTDRAWPALETTE);
    /*** IDirectDrawPalette methods ***/
typedef HRESULT (WINAPI *GetCapsP_Type)(LPDIRECTDRAWPALETTE, LPDWORD);
typedef HRESULT (WINAPI *GetEntries_Type)(LPDIRECTDRAWPALETTE, DWORD, DWORD, DWORD, LPPALETTEENTRY);
//    STDMETHOD(Initialize)(THIS_ LPDIRECTDRAW, DWORD, LPPALETTEENTRY) PURE;
typedef HRESULT (WINAPI *SetEntries_Type)(LPDIRECTDRAWPALETTE, DWORD, DWORD, DWORD, LPPALETTEENTRY);

// 
// GammaRamp
typedef HRESULT (WINAPI *GammaRamp_Type)(LPDIRECTDRAWSURFACE, DWORD, LPDDGAMMARAMP);

// extern procedures

extern void HookTexture(LPVOID *, int);
extern int Set_dwSize_From_Surface();

extern void SetBltTransformations(int);
extern HRESULT WINAPI PrimaryBlt(int, Blt_Type, LPDIRECTDRAWSURFACE, LPRECT, LPDIRECTDRAWSURFACE, LPRECT, LPDDBLTFX);
extern HRESULT WINAPI PrimaryNoBlt(int, Blt_Type, LPDIRECTDRAWSURFACE, LPRECT, LPDIRECTDRAWSURFACE, LPRECT, LPDDBLTFX);
extern HRESULT WINAPI PrimaryStretchBlt(int, Blt_Type, LPDIRECTDRAWSURFACE, LPRECT, LPDIRECTDRAWSURFACE, LPRECT, LPDDBLTFX);
extern HRESULT WINAPI PrimaryFastBlt(int, Blt_Type, LPDIRECTDRAWSURFACE, LPRECT, LPDIRECTDRAWSURFACE, LPRECT, LPDDBLTFX);
extern HRESULT WINAPI PrimaryBilinearBlt(int, Blt_Type, LPDIRECTDRAWSURFACE, LPRECT, LPDIRECTDRAWSURFACE, LPRECT, LPDDBLTFX);

typedef HRESULT (WINAPI *EmuBlt_Type)(int, Blt_Type, LPDIRECTDRAWSURFACE, LPRECT, LPDIRECTDRAWSURFACE, LPRECT, DWORD, LPVOID);
typedef HRESULT (WINAPI *RevBlt_Type)(int, Blt_Type, LPDIRECTDRAWSURFACE, LPRECT, LPDIRECTDRAWSURFACE, LPRECT);
typedef HRESULT (WINAPI *PrimaryBlt_Type)(int, Blt_Type, LPDIRECTDRAWSURFACE, LPRECT, LPDIRECTDRAWSURFACE, LPRECT, LPDDBLTFX);

extern EmuBlt_Type pEmuBlt;
extern RevBlt_Type pRevBlt;
