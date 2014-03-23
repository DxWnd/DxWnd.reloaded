/* ===================================================================

syslibs.h: contains typedef definition and extern pointer declarations
for all system calls hooked by DxWnd:
GDI32.dll
Kernel32.dll
ole32.dll
user32.dll.dll

====================================================================*/

#include "Mmsystem.h"

/* ===================================================================
type definitions
====================================================================*/

// advapi32.dll

typedef LONG	(WINAPI *RegCloseKey_Type)(HKEY);
typedef LONG	(WINAPI *RegCreateKey_Type)(HKEY, LPCTSTR, PHKEY);
typedef LONG	(WINAPI *RegCreateKeyEx_Type)(HKEY, LPCTSTR, DWORD, LPTSTR, DWORD, REGSAM, LPSECURITY_ATTRIBUTES, PHKEY, LPDWORD);
typedef LONG	(WINAPI *RegOpenKeyEx_Type)(HKEY, LPCTSTR, DWORD, REGSAM, PHKEY);
typedef LONG	(WINAPI *RegQueryValueEx_Type)(HKEY, LPCTSTR, LPDWORD, LPDWORD, LPBYTE, LPDWORD);
typedef LONG	(WINAPI *RegSetValueEx_Type)(HKEY, LPCTSTR, DWORD, DWORD, const BYTE *, DWORD);

// ImmLib.dll:

typedef BOOL	(WINAPI *ImmNotifyIME_Type)(HIMC, DWORD, DWORD, DWORD);
typedef HIMC	(WINAPI *ImmCreateContext_Type)(void);
typedef BOOL	(WINAPI *ImmDestroyContext_Type)(HIMC);
typedef BOOL	(WINAPI *ImmSetOpenStatus_Type)(HIMC, BOOL);
typedef HIMC	(WINAPI *ImmAssociateContext_Type)(HWND, HIMC);
typedef BOOL	(WINAPI *ImmSetCompositionWindow_Type)(HIMC, LPCOMPOSITIONFORM);
typedef BOOL	(WINAPI *ImmSetCompositionString_Type)(HIMC, DWORD, LPVOID, DWORD, LPVOID, DWORD);
typedef BOOL	(WINAPI *ImmGetOpenStatus_Type)(HIMC);

// GDI32.dll:
typedef BOOL	(WINAPI *BitBlt_Type)(HDC, int, int, int, int, HDC, int, int, DWORD);
typedef HDC		(WINAPI *CreateCompatibleDC_Type)(HDC);
typedef HDC		(WINAPI *CreateDC_Type)(LPCSTR, LPCSTR, LPCSTR, const DEVMODE *);
typedef HFONT	(WINAPI *CreateFont_Type)(int, int, int, int, int, DWORD, DWORD, DWORD, DWORD, DWORD, DWORD, DWORD, DWORD, LPCTSTR);
typedef HFONT	(WINAPI *CreateFontIndirect_Type)(const LOGFONT*);
typedef HPALETTE(WINAPI *GDICreatePalette_Type)(CONST LOGPALETTE *);
typedef HDWP	(WINAPI *DeferWindowPos_Type)(HDWP, HWND, HWND, int, int, int, int, UINT);
typedef BOOL	(WINAPI *DeleteDC_Type)(HDC);
typedef int		(WINAPI *GetClipBox_Type)(HDC, LPRECT);
typedef int		(WINAPI *GetRegionBox_Type)(HDC, LPRECT);
typedef int		(WINAPI *GetDeviceCaps_Type)(HDC, int);
typedef BOOL	(WINAPI *GetDeviceGammaRamp_Type)(HDC, LPVOID);
typedef BOOL	(WINAPI *GetSystemPaletteEntries_Type)(HDC, UINT, UINT, LPPALETTEENTRY);
typedef BOOL	(WINAPI *PatBlt_Type)(HDC, int, int, int, int, DWORD);
typedef BOOL	(WINAPI *Polyline_Type)(HDC, const POINT *, int);
typedef UINT	(WINAPI *RealizePalette_Type)(HDC);
typedef BOOL	(WINAPI *Rectangle_Type)(HDC, int, int, int, int);
typedef BOOL	(WINAPI *RestoreDC_Type)(HDC, int);
typedef int		(WINAPI *SaveDC_Type)(HDC);
typedef BOOL	(WINAPI *ScaleWindowExtEx_Type)(HDC, int, int, int, int, LPSIZE);
typedef HPALETTE(WINAPI *SelectPalette_Type)(HDC, HPALETTE, BOOL);
typedef COLORREF (WINAPI *SetBkColor_Type)(HDC, COLORREF);
typedef BOOL	(WINAPI *SetDeviceGammaRamp_Type)(HDC, LPVOID);
typedef COLORREF(WINAPI *SetTextColor_Type)(HDC, COLORREF);
typedef BOOL	(WINAPI *StretchBlt_Type)(HDC, int, int, int, int, HDC, int, int, int, int, DWORD);
typedef int		(WINAPI *StretchDIBits_Type)(HDC, int, int, int, int, int, int, int, int, const VOID *, const BITMAPINFO *, UINT, DWORD);
typedef BOOL	(WINAPI *TextOut_Type)(HDC, int, int, LPCTSTR, int);
typedef BOOL	(WINAPI *PolyBezierTo_Type)(HDC, const POINT *, DWORD);
typedef BOOL	(WINAPI *PolylineTo_Type)(HDC, const POINT *, DWORD);
typedef BOOL	(WINAPI *PolyDraw_Type)(HDC, const POINT *, const BYTE *, int);
typedef BOOL	(WINAPI *MoveToEx_Type)(HDC, int, int, LPPOINT);
typedef BOOL	(WINAPI *ArcTo_Type)(HDC, int, int, int, int, int, int, int, int);
typedef BOOL	(WINAPI *LineTo_Type)(HDC, int, int);
typedef int		(WINAPI *SetDIBitsToDevice_Type)(HDC, int, int, DWORD, DWORD, int, int, UINT, UINT, const VOID *, const BITMAPINFO *, UINT);
typedef HBITMAP (WINAPI *CreateCompatibleBitmap_Type)(HDC, int, int);
typedef COLORREF(WINAPI *SetPixel_Type)(HDC, int, int, COLORREF);
typedef BOOL	(WINAPI *Ellipse_Type)(HDC, int, int, int, int);
typedef BOOL	(WINAPI *Polygon_Type)(HDC, const POINT *, int);
typedef BOOL	(WINAPI *Arc_Type)(HDC, int, int, int, int, int, int, int, int);
typedef HRGN	(WINAPI *CreateEllipticRgn_Type)(int, int, int, int);
typedef HRGN	(WINAPI *CreateEllipticRgnIndirect_Type)(const RECT *);
typedef HRGN	(WINAPI *CreateRectRgn_Type)(int, int, int, int);
typedef HRGN	(WINAPI *CreateRectRgnIndirect_Type)(const RECT *);
typedef HRGN	(WINAPI *CreatePolygonRgn_Type)(const POINT *, int, int);
typedef int		(WINAPI *DrawText_Type)(HDC, LPCTSTR, int, LPRECT, UINT);
typedef int		(WINAPI *DrawTextEx_Type)(HDC, LPTSTR, int, LPRECT, UINT, LPDRAWTEXTPARAMS);
typedef BOOL	(WINAPI *MaskBlt_Type)(HDC, int, int, int, int, HDC, int, int, HBITMAP, int, int, DWORD);
typedef BOOL	(WINAPI *SetViewportOrgEx_Type)(HDC, int, int, LPPOINT);
typedef BOOL	(WINAPI *SetViewportExtEx_Type)(HDC, int, int, LPSIZE);
typedef BOOL	(WINAPI *GetViewportOrgEx_Type)(HDC, LPPOINT);
typedef BOOL	(WINAPI *GetWindowOrgEx_Type)(HDC, LPPOINT);
typedef BOOL	(WINAPI *SetWindowOrgEx_Type)(HDC, int, int, LPPOINT);
typedef BOOL	(WINAPI *GetCurrentPositionEx_Type)(HDC, LPPOINT);
typedef BOOL	(WINAPI *AnimatePalette_Type)(HPALETTE, UINT, UINT, const PALETTEENTRY *);
typedef UINT	(WINAPI *SetSystemPaletteUse_Type)(HDC, UINT);
typedef BOOL	(WINAPI *GDISetPixelFormat_Type)(HDC, int, const PIXELFORMATDESCRIPTOR *);
typedef BOOL	(WINAPI *GDIGetPixelFormat_Type)(HDC);
typedef int		(WINAPI *ChoosePixelFormat_Type)(HDC, const PIXELFORMATDESCRIPTOR *);
typedef int		(WINAPI *DescribePixelFormat_Type)(HDC, int, UINT, LPPIXELFORMATDESCRIPTOR);
typedef DWORD	(WINAPI *GetObjectType_Type)(HGDIOBJ);


// Kernel32.dll:
typedef BOOL	(WINAPI *GetDiskFreeSpaceA_Type)(LPCSTR, LPDWORD, LPDWORD, LPDWORD, LPDWORD);
typedef UINT	(WINAPI *GetDriveType_Type)(LPCSTR);
typedef void	(WINAPI *GetLocalTime_Type)(LPSYSTEMTIME);
typedef FARPROC (WINAPI *GetProcAddress_Type)(HMODULE, LPCSTR);
typedef void	(WINAPI *GetSystemTime_Type)(LPSYSTEMTIME);
typedef void	(WINAPI *GetSystemTimeAsFileTime_Type)(LPFILETIME);
typedef DWORD	(WINAPI *GetTickCount_Type)(void);
typedef DWORD	(WINAPI *GetVersion_Type)(void);
typedef BOOL	(WINAPI *GetVersionExA_Type)(LPOSVERSIONINFOA);
typedef BOOL	(WINAPI *GetVersionExW_Type)(LPOSVERSIONINFOW);
typedef void	(WINAPI *GlobalMemoryStatus_Type)(LPMEMORYSTATUS);
typedef HMODULE (WINAPI *LoadLibraryA_Type)(LPCTSTR);
typedef HMODULE (WINAPI *LoadLibraryExA_Type)(LPCTSTR, HANDLE, DWORD);
typedef HMODULE (WINAPI *LoadLibraryW_Type)(LPCWSTR);
typedef HMODULE (WINAPI *LoadLibraryExW_Type)(LPCWSTR, HANDLE, DWORD);
typedef BOOL	(WINAPI *ReadFile_Type)(HANDLE, LPVOID, DWORD, LPDWORD, LPOVERLAPPED);
typedef LPTOP_LEVEL_EXCEPTION_FILTER 
				(WINAPI *SetUnhandledExceptionFilter_Type)(LPTOP_LEVEL_EXCEPTION_FILTER);
typedef void	(WINAPI *Sleep_Type)(DWORD);
typedef DWORD	(WINAPI *SleepEx_Type)(DWORD, BOOL);
typedef BOOL	(WINAPI *CreateScalableFontResourceA_Type)(DWORD, LPCTSTR, LPCTSTR, LPCTSTR);
typedef int		(WINAPI *AddFontResourceA_Type)(LPCTSTR);
typedef BOOL	(WINAPI *CreateScalableFontResourceW_Type)(DWORD, LPCWSTR, LPCWSTR, LPCWSTR);
typedef int		(WINAPI *AddFontResourceW_Type)(LPCWSTR);
typedef HANDLE  (WINAPI *CreateFile_Type)(LPCTSTR, DWORD, DWORD, LPSECURITY_ATTRIBUTES, DWORD, DWORD, HANDLE);
typedef DWORD	(WINAPI *SetFilePointer_Type)(HANDLE, LONG, PLONG, DWORD);
typedef BOOL	(WINAPI *CloseHandle_Type)(HANDLE);
typedef BOOL	(WINAPI *QueryPerformanceFrequency_Type)(LARGE_INTEGER *);
typedef BOOL	(WINAPI *QueryPerformanceCounter_Type)(LARGE_INTEGER *);
typedef BOOL	(WINAPI *QueryPerformanceFrequency_Type)(LARGE_INTEGER *);

// ole32.dll:
typedef HRESULT (STDAPICALLTYPE *CoCreateInstance_Type)(REFCLSID, LPUNKNOWN, DWORD, REFIID, LPVOID FAR*);
typedef HRESULT (STDAPICALLTYPE *CoCreateInstanceEx_Type)(REFCLSID, IUnknown *, DWORD, COSERVERINFO *, DWORD, MULTI_QI *);
typedef HRESULT (STDAPICALLTYPE *CoInitialize_Type)(LPVOID);

// user32.dll:
typedef HDC		(WINAPI *BeginPaint_Type)(HWND, LPPAINTSTRUCT);
typedef LRESULT (WINAPI *CallWindowProc_Type)(WNDPROC, HWND, UINT, WPARAM, LPARAM);
typedef LONG	(WINAPI *ChangeDisplaySettingsA_Type)(DEVMODEA *, DWORD);
typedef LONG	(WINAPI *ChangeDisplaySettingsExA_Type)(LPCTSTR, DEVMODEA *, HWND, DWORD, LPVOID);
typedef LONG	(WINAPI *ChangeDisplaySettingsW_Type)(DEVMODEW *, DWORD);
typedef LONG	(WINAPI *ChangeDisplaySettingsExW_Type)(LPCTSTR, DEVMODEW *, HWND, DWORD, LPVOID);
typedef BOOL	(WINAPI *ClientToScreen_Type)(HWND, LPPOINT);
typedef BOOL	(WINAPI *ClipCursor_Type)(const RECT *);
typedef HWND	(WINAPI *CreateDialogIndirectParam_Type)(HINSTANCE, LPCDLGTEMPLATE, HWND, DLGPROC, LPARAM);
typedef HWND	(WINAPI *CreateDialogParam_Type)(HINSTANCE, LPCTSTR, HWND, DLGPROC, LPARAM);
typedef HWND	(WINAPI *CreateWindowExA_Type)(DWORD, LPCTSTR, LPCTSTR, DWORD, int, int, int, int, HWND, HMENU, HINSTANCE, LPVOID);
typedef HWND	(WINAPI *CreateWindowExW_Type)(DWORD, LPCWSTR, LPCWSTR, DWORD, int, int, int, int, HWND, HMENU, HINSTANCE, LPVOID);
typedef LRESULT (WINAPI *DefWindowProc_Type)(HWND, UINT, WPARAM, LPARAM);
typedef BOOL	(WINAPI *EndPaint_Type)(HWND, const PAINTSTRUCT *);
typedef LONG	(WINAPI *EnumDisplaySettings_Type)(LPCTSTR, DWORD, LPDEVMODEA);
typedef int		(WINAPI *FillRect_Type)(HDC, const RECT *, HBRUSH);
typedef int		(WINAPI *FrameRect_Type)(HDC, const RECT *, HBRUSH);
typedef BOOL	(WINAPI *GetClientRect_Type)(HWND, LPRECT);
typedef BOOL	(WINAPI *GetClipCursor_Type)(LPRECT);
typedef BOOL	(WINAPI *GetCursorPos_Type)(LPPOINT);
typedef HDC		(WINAPI *GDIGetDC_Type)(HWND);
typedef HWND	(WINAPI *GetDesktopWindow_Type)(void);
typedef BOOL	(WINAPI *GetMessage_Type)(LPMSG, HWND, UINT, UINT);
typedef int		(WINAPI *GetSystemMetrics_Type)(int);
typedef HWND	(WINAPI *GetTopWindow_Type)(HWND);
typedef LONG	(WINAPI *GetWindowLong_Type)(HWND, int);
typedef BOOL	(WINAPI *GetWindowRect_Type)(HWND, LPRECT);
typedef BOOL	(WINAPI *InvalidateRect_Type)(HWND, CONST RECT *, BOOL);
typedef int		(WINAPI *MapWindowPoints_Type)(HWND, HWND, LPPOINT, UINT);
typedef BOOL	(WINAPI *MoveWindow_Type)(HWND, int, int, int, int, BOOL);
typedef BOOL	(WINAPI *PeekMessage_Type)(LPMSG, HWND, UINT, UINT, UINT);
typedef ATOM	(WINAPI *RegisterClassExA_Type)(WNDCLASSEX *);
typedef ATOM	(WINAPI *RegisterClassA_Type)(WNDCLASS *);
typedef int		(WINAPI *GDIReleaseDC_Type)(HWND, HDC);
typedef BOOL	(WINAPI *ScreenToClient_Type)(HWND, LPPOINT);
typedef LRESULT (WINAPI *SendMessage_Type)(HWND, UINT, WPARAM, LPARAM);
typedef HCURSOR (WINAPI *SetCursor_Type)(HCURSOR);
typedef BOOL	(WINAPI *SetCursorPos_Type)(int, int);
typedef UINT_PTR(WINAPI *SetTimer_Type)(HWND, UINT_PTR, UINT, TIMERPROC);
typedef LONG	(WINAPI *SetWindowLong_Type)(HWND, int, LONG);
typedef BOOL	(WINAPI *SetWindowPos_Type)(HWND, HWND, int, int, int, int, UINT);
typedef int		(WINAPI *ShowCursor_Type)(BOOL);
typedef BOOL	(WINAPI *ShowWindow_Type)(HWND, int);
typedef LONG	(WINAPI *TabbedTextOutA_Type)(HDC, int, int, LPCTSTR, int, int, const LPINT, int);
typedef BOOL	(WINAPI *DestroyWindow_Type)(HWND);
typedef BOOL	(WINAPI *CloseWindow_Type)(HWND);
typedef BOOL	(WINAPI *SetSysColors_Type)(int, const INT *, const COLORREF *);
typedef HDC		(WINAPI *GetDCEx_Type)(HWND, HRGN, DWORD);
typedef BOOL	(WINAPI *UpdateWindow_Type)(HWND);
typedef BOOL	(WINAPI *GetWindowPlacement_Type)(HWND, WINDOWPLACEMENT *);
typedef BOOL	(WINAPI *SetWindowPlacement_Type)(HWND, WINDOWPLACEMENT *);
typedef HWND	(WINAPI *SetCapture_Type)(HWND);
typedef HWND	(WINAPI *GetForegroundWindow_Type)(void);
typedef HWND	(WINAPI *GetActiveWindow_Type)(void);
typedef BOOL	(WINAPI *IsWindowVisible_Type)(HWND);
typedef BOOL	(WINAPI *SystemParametersInfo_Type)(UINT, UINT, PVOID, UINT);
typedef BOOL	(WINAPI *KillTimer_Type)(HWND, UINT_PTR);
typedef BOOL	(WINAPI *GetUpdateRect_Type)(HWND, LPRECT, BOOL);

// Winmm.dll:
typedef MCIERROR(WINAPI *mciSendCommand_Type)(MCIDEVICEID, UINT, DWORD_PTR, DWORD_PTR);
typedef DWORD	(WINAPI *timeGetTime_Type)(void);
typedef MMRESULT(WINAPI *timeKillEvent_Type)(UINT);
typedef MMRESULT(WINAPI *timeSetEvent_Type)(UINT, UINT, LPTIMECALLBACK, DWORD_PTR, UINT);

/* ===================================================================
hooked APIs real pointers
====================================================================*/

#ifdef DXWDECLARATIONS 
#undef DXWEXTERN
#undef DXWINITIALIZED
#define DXWEXTERN
#define DXWINITIALIZED = NULL
#else
#undef DXWEXTERN
#undef DXWINITIALIZED
#define DXWEXTERN extern
#define DXWINITIALIZED
#endif

// advapi32.dll

DXWEXTERN RegCloseKey_Type pRegCloseKey DXWINITIALIZED;
DXWEXTERN RegCreateKey_Type pRegCreateKey DXWINITIALIZED;
DXWEXTERN RegCreateKeyEx_Type pRegCreateKeyEx DXWINITIALIZED;
DXWEXTERN RegOpenKeyEx_Type pRegOpenKeyEx DXWINITIALIZED;
DXWEXTERN RegQueryValueEx_Type pRegQueryValueEx DXWINITIALIZED;
DXWEXTERN RegSetValueEx_Type pRegSetValueEx DXWINITIALIZED;

// ImmLib.dll

DXWEXTERN ImmNotifyIME_Type pImmNotifyIME DXWINITIALIZED;
DXWEXTERN ImmCreateContext_Type pImmCreateContext DXWINITIALIZED;
DXWEXTERN ImmDestroyContext_Type pImmDestroyContext DXWINITIALIZED;
DXWEXTERN ImmSetOpenStatus_Type pImmSetOpenStatus DXWINITIALIZED;
DXWEXTERN ImmAssociateContext_Type pImmAssociateContext DXWINITIALIZED;
DXWEXTERN ImmSetCompositionWindow_Type pImmSetCompositionWindow DXWINITIALIZED;
DXWEXTERN ImmSetCompositionString_Type pImmSetCompositionString DXWINITIALIZED;
DXWEXTERN ImmGetOpenStatus_Type pImmGetOpenStatus DXWINITIALIZED;

// GDI32.dll:
DXWEXTERN BitBlt_Type pGDIBitBlt DXWINITIALIZED;
DXWEXTERN CreateCompatibleDC_Type pGDICreateCompatibleDC DXWINITIALIZED;
DXWEXTERN CreateDC_Type pGDICreateDC DXWINITIALIZED;
DXWEXTERN CreateFont_Type pGDICreateFont DXWINITIALIZED;
DXWEXTERN CreateFontIndirect_Type pGDICreateFontIndirect DXWINITIALIZED;
DXWEXTERN GDICreatePalette_Type pGDICreatePalette DXWINITIALIZED;
DXWEXTERN DeferWindowPos_Type pGDIDeferWindowPos DXWINITIALIZED;
DXWEXTERN DeleteDC_Type pGDIDeleteDC DXWINITIALIZED;
DXWEXTERN GetClipBox_Type pGDIGetClipBox DXWINITIALIZED;
DXWEXTERN GetRegionBox_Type pGDIGetRegionBox DXWINITIALIZED;
DXWEXTERN GetDeviceCaps_Type pGDIGetDeviceCaps DXWINITIALIZED;
DXWEXTERN GetDeviceGammaRamp_Type pGDIGetDeviceGammaRamp DXWINITIALIZED;
DXWEXTERN GetSystemPaletteEntries_Type pGDIGetSystemPaletteEntries DXWINITIALIZED;
DXWEXTERN PatBlt_Type pGDIPatBlt DXWINITIALIZED;
DXWEXTERN Polyline_Type pPolyline DXWINITIALIZED;
DXWEXTERN RealizePalette_Type pGDIRealizePalette DXWINITIALIZED;
DXWEXTERN Rectangle_Type pGDIRectangle DXWINITIALIZED;
DXWEXTERN RestoreDC_Type pGDIRestoreDC DXWINITIALIZED;
DXWEXTERN SaveDC_Type pGDISaveDC DXWINITIALIZED;
DXWEXTERN ScaleWindowExtEx_Type pGDIScaleWindowExtEx DXWINITIALIZED;
DXWEXTERN SelectPalette_Type pGDISelectPalette DXWINITIALIZED;
DXWEXTERN SetBkColor_Type pGDISetBkColor DXWINITIALIZED;
DXWEXTERN SetDeviceGammaRamp_Type pGDISetDeviceGammaRamp DXWINITIALIZED;
DXWEXTERN SetTextColor_Type pGDISetTextColor DXWINITIALIZED;
DXWEXTERN StretchBlt_Type pGDIStretchBlt DXWINITIALIZED;
DXWEXTERN StretchDIBits_Type pStretchDIBits DXWINITIALIZED;
DXWEXTERN TextOut_Type pGDITextOutA DXWINITIALIZED;
DXWEXTERN LineTo_Type pLineTo DXWINITIALIZED;
DXWEXTERN ArcTo_Type pArcTo DXWINITIALIZED;
DXWEXTERN MoveToEx_Type pMoveToEx DXWINITIALIZED;
DXWEXTERN PolyDraw_Type pPolyDraw DXWINITIALIZED;
DXWEXTERN PolylineTo_Type pPolylineTo DXWINITIALIZED;
DXWEXTERN PolyBezierTo_Type pPolyBezierTo DXWINITIALIZED;
DXWEXTERN SetDIBitsToDevice_Type pSetDIBitsToDevice DXWINITIALIZED;
DXWEXTERN CreateCompatibleBitmap_Type pCreateCompatibleBitmap DXWINITIALIZED;
DXWEXTERN SetPixel_Type pSetPixel DXWINITIALIZED;
DXWEXTERN Ellipse_Type pEllipse DXWINITIALIZED;
DXWEXTERN Polygon_Type pPolygon DXWINITIALIZED;
DXWEXTERN Arc_Type pArc DXWINITIALIZED;
DXWEXTERN CreateEllipticRgn_Type pCreateEllipticRgn DXWINITIALIZED;
DXWEXTERN CreateEllipticRgnIndirect_Type pCreateEllipticRgnIndirect DXWINITIALIZED;
DXWEXTERN CreateRectRgn_Type pCreateRectRgn DXWINITIALIZED;
DXWEXTERN CreateRectRgnIndirect_Type pCreateRectRgnIndirect DXWINITIALIZED;
DXWEXTERN CreatePolygonRgn_Type pCreatePolygonRgn DXWINITIALIZED;
DXWEXTERN DrawText_Type pDrawText DXWINITIALIZED;
DXWEXTERN DrawTextEx_Type pDrawTextEx DXWINITIALIZED;
DXWEXTERN MaskBlt_Type pMaskBlt DXWINITIALIZED;
DXWEXTERN SetViewportOrgEx_Type pSetViewportOrgEx DXWINITIALIZED;
DXWEXTERN SetViewportExtEx_Type pSetViewportExtEx DXWINITIALIZED;
DXWEXTERN GetViewportOrgEx_Type pGetViewportOrgEx DXWINITIALIZED;
DXWEXTERN GetWindowOrgEx_Type pGetWindowOrgEx DXWINITIALIZED;
DXWEXTERN SetWindowOrgEx_Type pSetWindowOrgEx DXWINITIALIZED;
DXWEXTERN GetCurrentPositionEx_Type pGetCurrentPositionEx DXWINITIALIZED;
DXWEXTERN CreateScalableFontResourceA_Type pCreateScalableFontResourceA DXWINITIALIZED;
DXWEXTERN AddFontResourceA_Type pAddFontResourceA DXWINITIALIZED;
DXWEXTERN CreateScalableFontResourceW_Type pCreateScalableFontResourceW DXWINITIALIZED;
DXWEXTERN AddFontResourceW_Type pAddFontResourceW DXWINITIALIZED;
DXWEXTERN AnimatePalette_Type pAnimatePalette DXWINITIALIZED;
DXWEXTERN SetSystemPaletteUse_Type pSetSystemPaletteUse DXWINITIALIZED;
DXWEXTERN GDISetPixelFormat_Type pGDISetPixelFormat DXWINITIALIZED;
DXWEXTERN GDIGetPixelFormat_Type pGDIGetPixelFormat DXWINITIALIZED;
DXWEXTERN ChoosePixelFormat_Type pChoosePixelFormat DXWINITIALIZED;
DXWEXTERN DescribePixelFormat_Type pDescribePixelFormat DXWINITIALIZED;
DXWEXTERN GetObjectType_Type pGetObjectType DXWINITIALIZED;

// Kernel32.dll:
DXWEXTERN GetDiskFreeSpaceA_Type pGetDiskFreeSpaceA DXWINITIALIZED;
DXWEXTERN GetDriveType_Type pGetDriveType DXWINITIALIZED;
DXWEXTERN GetLocalTime_Type pGetLocalTime DXWINITIALIZED;
DXWEXTERN GetProcAddress_Type pGetProcAddress DXWINITIALIZED;
DXWEXTERN GetSystemTime_Type pGetSystemTime DXWINITIALIZED;
DXWEXTERN GetSystemTimeAsFileTime_Type pGetSystemTimeAsFileTime DXWINITIALIZED;
DXWEXTERN GetTickCount_Type pGetTickCount DXWINITIALIZED;
DXWEXTERN GetVersion_Type pGetVersion DXWINITIALIZED;
DXWEXTERN GetVersionExA_Type pGetVersionExA DXWINITIALIZED;
DXWEXTERN GetVersionExW_Type pGetVersionExW DXWINITIALIZED;
DXWEXTERN GlobalMemoryStatus_Type pGlobalMemoryStatus DXWINITIALIZED;
DXWEXTERN LoadLibraryA_Type pLoadLibraryA DXWINITIALIZED;
DXWEXTERN LoadLibraryExA_Type pLoadLibraryExA DXWINITIALIZED;
DXWEXTERN LoadLibraryW_Type pLoadLibraryW DXWINITIALIZED;
DXWEXTERN LoadLibraryExW_Type pLoadLibraryExW DXWINITIALIZED;
DXWEXTERN ReadFile_Type pReadFile DXWINITIALIZED;
DXWEXTERN SetUnhandledExceptionFilter_Type pSetUnhandledExceptionFilter DXWINITIALIZED;
DXWEXTERN Sleep_Type pSleep DXWINITIALIZED;
DXWEXTERN SleepEx_Type pSleepEx DXWINITIALIZED;
DXWEXTERN CreateFile_Type pCreateFile DXWINITIALIZED;
DXWEXTERN SetFilePointer_Type pSetFilePointer DXWINITIALIZED;
DXWEXTERN CloseHandle_Type pCloseHandle DXWINITIALIZED;
DXWEXTERN QueryPerformanceFrequency_Type pQueryPerformanceFrequency DXWINITIALIZED;
DXWEXTERN QueryPerformanceCounter_Type pQueryPerformanceCounter DXWINITIALIZED;

// ole32.dll:
DXWEXTERN CoCreateInstance_Type pCoCreateInstance DXWINITIALIZED;
DXWEXTERN CoCreateInstanceEx_Type pCoCreateInstanceEx  DXWINITIALIZED;
DXWEXTERN CoInitialize_Type pCoInitialize  DXWINITIALIZED;

// user32.dll:
DXWEXTERN BeginPaint_Type pBeginPaint DXWINITIALIZED;
DXWEXTERN CallWindowProc_Type pCallWindowProc DXWINITIALIZED;
DXWEXTERN ChangeDisplaySettingsA_Type pChangeDisplaySettingsA DXWINITIALIZED;
DXWEXTERN ChangeDisplaySettingsExA_Type pChangeDisplaySettingsExA DXWINITIALIZED;
DXWEXTERN ChangeDisplaySettingsW_Type pChangeDisplaySettingsW DXWINITIALIZED;
DXWEXTERN ChangeDisplaySettingsExW_Type pChangeDisplaySettingsExW DXWINITIALIZED;
DXWEXTERN ClientToScreen_Type pClientToScreen DXWINITIALIZED;
DXWEXTERN ClipCursor_Type pClipCursor DXWINITIALIZED;
DXWEXTERN CreateDialogIndirectParam_Type pCreateDialogIndirectParam DXWINITIALIZED;
DXWEXTERN CreateDialogParam_Type pCreateDialogParam DXWINITIALIZED;
DXWEXTERN CreateWindowExA_Type pCreateWindowExA DXWINITIALIZED;
DXWEXTERN CreateWindowExW_Type pCreateWindowExW DXWINITIALIZED;
DXWEXTERN DefWindowProc_Type pDefWindowProc DXWINITIALIZED;
DXWEXTERN EndPaint_Type pEndPaint DXWINITIALIZED;
DXWEXTERN EnumDisplaySettings_Type pEnumDisplaySettings DXWINITIALIZED;
DXWEXTERN FillRect_Type pFillRect DXWINITIALIZED;
DXWEXTERN FrameRect_Type pFrameRect DXWINITIALIZED;
DXWEXTERN GetClientRect_Type pGetClientRect DXWINITIALIZED;
DXWEXTERN GetClipCursor_Type pGetClipCursor DXWINITIALIZED;
DXWEXTERN GetCursorPos_Type pGetCursorPos DXWINITIALIZED;
DXWEXTERN GDIGetDC_Type pGDIGetDC DXWINITIALIZED;
DXWEXTERN GetDesktopWindow_Type pGetDesktopWindow DXWINITIALIZED;
DXWEXTERN GetSystemMetrics_Type pGetSystemMetrics DXWINITIALIZED;
DXWEXTERN GetTopWindow_Type pGetTopWindow DXWINITIALIZED;
DXWEXTERN GDIGetDC_Type pGDIGetWindowDC DXWINITIALIZED;
DXWEXTERN GetWindowLong_Type pGetWindowLong DXWINITIALIZED;
DXWEXTERN GetWindowRect_Type pGetWindowRect DXWINITIALIZED;
DXWEXTERN InvalidateRect_Type pInvalidateRect DXWINITIALIZED;
DXWEXTERN MapWindowPoints_Type pMapWindowPoints DXWINITIALIZED;
DXWEXTERN MoveWindow_Type pMoveWindow DXWINITIALIZED;
DXWEXTERN PeekMessage_Type pPeekMessage DXWINITIALIZED;
DXWEXTERN RegisterClassExA_Type pRegisterClassExA DXWINITIALIZED;
DXWEXTERN RegisterClassA_Type pRegisterClassA DXWINITIALIZED;
DXWEXTERN GDIReleaseDC_Type pGDIReleaseDC DXWINITIALIZED;
DXWEXTERN ScreenToClient_Type pScreenToClient DXWINITIALIZED;
DXWEXTERN SendMessage_Type pSendMessageA DXWINITIALIZED;
DXWEXTERN SendMessage_Type pSendMessageW DXWINITIALIZED;
DXWEXTERN SetCursor_Type pSetCursor DXWINITIALIZED;
DXWEXTERN SetCursorPos_Type pSetCursorPos DXWINITIALIZED;
DXWEXTERN SetTimer_Type pSetTimer DXWINITIALIZED;
DXWEXTERN SetWindowLong_Type pSetWindowLong DXWINITIALIZED;
DXWEXTERN SetWindowPos_Type pSetWindowPos DXWINITIALIZED;
DXWEXTERN ShowCursor_Type pShowCursor DXWINITIALIZED;
DXWEXTERN ShowWindow_Type pShowWindow DXWINITIALIZED;
DXWEXTERN TabbedTextOutA_Type pTabbedTextOutA DXWINITIALIZED;
DXWEXTERN DestroyWindow_Type pDestroyWindow DXWINITIALIZED;
DXWEXTERN CloseWindow_Type pCloseWindow DXWINITIALIZED;
DXWEXTERN SetSysColors_Type pSetSysColors DXWINITIALIZED;
DXWEXTERN GetDCEx_Type pGDIGetDCEx DXWINITIALIZED;
DXWEXTERN UpdateWindow_Type pUpdateWindow DXWINITIALIZED;
DXWEXTERN GetWindowPlacement_Type pGetWindowPlacement DXWINITIALIZED;
DXWEXTERN SetWindowPlacement_Type pSetWindowPlacement DXWINITIALIZED;
DXWEXTERN SetCapture_Type pSetCapture DXWINITIALIZED;
DXWEXTERN GetForegroundWindow_Type pGetForegroundWindow DXWINITIALIZED;
DXWEXTERN GetActiveWindow_Type pGetActiveWindow DXWINITIALIZED;
DXWEXTERN IsWindowVisible_Type pIsWindowVisible DXWINITIALIZED;
DXWEXTERN SystemParametersInfo_Type pSystemParametersInfoA DXWINITIALIZED;
DXWEXTERN KillTimer_Type pKillTimer DXWINITIALIZED;
DXWEXTERN GetUpdateRect_Type pGetUpdateRect DXWINITIALIZED;

// Winmm.dll:
DXWEXTERN mciSendCommand_Type pmciSendCommand DXWINITIALIZED;
DXWEXTERN timeGetTime_Type ptimeGetTime DXWINITIALIZED;
DXWEXTERN timeKillEvent_Type ptimeKillEvent DXWINITIALIZED;
DXWEXTERN timeSetEvent_Type ptimeSetEvent DXWINITIALIZED;

/* ===================================================================
hook procedures (possibly more tnan one per each API...)
====================================================================*/

// advapi32.dll

extern LONG	WINAPI extRegCloseKey(HKEY);
extern LONG	WINAPI extRegCreateKey(HKEY, LPCTSTR, PHKEY);
extern LONG	WINAPI extRegCreateKeyEx(HKEY, LPCTSTR, DWORD, LPTSTR, DWORD, REGSAM, LPSECURITY_ATTRIBUTES, PHKEY, LPDWORD);
extern LONG	WINAPI extRegOpenKeyEx(HKEY, LPCTSTR, DWORD, REGSAM, PHKEY);
extern LONG	WINAPI extRegQueryValueEx(HKEY, LPCTSTR, LPDWORD, LPDWORD, LPBYTE, LPDWORD);
extern LONG	WINAPI extRegSetValueEx(HKEY, LPCTSTR, DWORD, DWORD, const BYTE *, DWORD);

// ImmLib.dll

extern BOOL WINAPI extImmNotifyIME(HIMC, DWORD, DWORD, DWORD);
extern HIMC	WINAPI extImmCreateContext(void);
extern BOOL	WINAPI extImmDestroyContext(HIMC);
extern BOOL	WINAPI extImmSetOpenStatus(HIMC, BOOL);
extern HIMC WINAPI extImmAssociateContext(HWND, HIMC);
extern BOOL WINAPI extImmSetCompositionWindow(HIMC, LPCOMPOSITIONFORM);
extern BOOL WINAPI extImmSetCompositionString(HIMC, DWORD, LPVOID, DWORD, LPVOID, DWORD);
extern BOOL WINAPI extImmGetOpenStatus(HIMC);

// GDI32.dll:
extern BOOL WINAPI extGDIBitBlt(HDC, int, int, int, int, HDC, int, int, DWORD);
extern HDC WINAPI extGDICreateCompatibleDC(HDC);
extern HDC WINAPI extEMUCreateCompatibleDC(HDC);
extern HDC WINAPI extDDCreateCompatibleDC(HDC);
extern HDC WINAPI extGDICreateDC(LPSTR, LPSTR, LPSTR, CONST DEVMODE *);
extern HDC WINAPI extDDCreateDC(LPSTR, LPSTR, LPSTR, CONST DEVMODE *);
extern HFONT WINAPI extCreateFont(int, int, int, int, int, DWORD, DWORD, DWORD, DWORD, DWORD, DWORD, DWORD, DWORD, LPCTSTR);
extern HFONT WINAPI extCreateFontIndirect(const LOGFONT*);
extern HPALETTE WINAPI extGDICreatePalette(CONST LOGPALETTE *);
extern HDWP WINAPI extDeferWindowPos(HDWP, HWND, HWND, int, int, int, int, UINT);
extern BOOL WINAPI extGDIDeleteDC(HDC);
extern BOOL WINAPI extDDDeleteDC(HDC);
extern int WINAPI extGetClipBox(HDC, LPRECT);
extern int WINAPI extGetRegionBox(HDC, LPRECT);
extern int WINAPI extGetDeviceCaps(HDC, int);
extern BOOL WINAPI extGetDeviceGammaRamp(HDC, LPVOID);
extern UINT WINAPI extGetSystemPaletteEntries(HDC, UINT, UINT, LPPALETTEENTRY);
extern BOOL WINAPI extGDIPatBlt(HDC, int, int, int, int, DWORD);
extern BOOL WINAPI extPolyline(HDC, const POINT *, int);
extern BOOL WINAPI extDDBitBlt(HDC, int, int, int, int, HDC, int, int, DWORD);
extern UINT WINAPI extRealizePalette(HDC);
extern BOOL WINAPI extRectangle(HDC, int, int, int, int);
extern BOOL WINAPI extGDIRestoreDC(HDC, int);
extern int WINAPI extGDISaveDC(HDC);
extern BOOL WINAPI extScaleWindowExtEx(HDC, int, int, int, int, LPSIZE);
extern HPALETTE WINAPI extSelectPalette(HDC, HPALETTE, BOOL);
extern BOOL WINAPI extSetDeviceGammaRamp(HDC, LPVOID);
extern BOOL WINAPI extGDIStretchBlt(HDC, int, int, int, int, HDC, int, int, int, int, DWORD);
extern int WINAPI extStretchDIBits(HDC, int, int, int, int, int, int, int, int, const VOID *, const BITMAPINFO *, UINT, DWORD);
extern BOOL WINAPI extDDStretchBlt(HDC, int, int, int, int, HDC, int, int, int, int, DWORD);
extern BOOL WINAPI extTextOutA(HDC, int, int, LPCTSTR, int);
extern BOOL WINAPI extPolyBezierTo(HDC, const POINT *, DWORD);
extern BOOL WINAPI extPolylineTo(HDC, const POINT *, DWORD);
extern BOOL WINAPI extPolyDraw(HDC, const POINT *, const BYTE *, int);
extern BOOL WINAPI extMoveToEx(HDC, int, int, LPPOINT);
extern BOOL WINAPI extArcTo(HDC, int, int, int, int, int, int, int, int);
extern BOOL WINAPI extLineTo(HDC, int, int);
extern int WINAPI extSetDIBitsToDevice(HDC, int, int, DWORD, DWORD, int, int, UINT, UINT, const VOID *, const BITMAPINFO *, UINT);
extern HBITMAP WINAPI extCreateCompatibleBitmap(HDC, int, int);
extern COLORREF WINAPI extSetPixel(HDC, int, int, COLORREF);
extern BOOL WINAPI extEllipse(HDC, int, int, int, int);
extern BOOL WINAPI extPolygon(HDC, const POINT *, int);
extern BOOL WINAPI extArc(HDC, int, int, int, int, int, int, int, int);
extern HRGN WINAPI extCreateEllipticRgn(int, int, int, int);
extern HRGN WINAPI extCreateEllipticRgnIndirect(const RECT *);
extern HRGN WINAPI extCreateRectRgn(int, int, int, int);
extern HRGN WINAPI extCreateRectRgnIndirect(const RECT *);
extern HRGN WINAPI extCreatePolygonRgn(const POINT *, int, int);
extern int WINAPI extDrawText(HDC, LPCTSTR, int, LPRECT, UINT);
extern int WINAPI extDrawTextEx(HDC, LPTSTR, int, LPRECT, UINT, LPDRAWTEXTPARAMS);
extern BOOL WINAPI extMaskBlt(HDC, int, int, int, int, HDC, int, int, HBITMAP, int, int, DWORD);
extern BOOL WINAPI extSetViewportOrgEx(HDC, int, int, LPPOINT);
extern BOOL WINAPI extSetViewportExtEx(HDC, int, int, LPSIZE);
extern BOOL WINAPI extGetViewportOrgEx(HDC, LPPOINT);
extern BOOL WINAPI extGetWindowOrgEx(HDC, LPPOINT);
extern BOOL WINAPI extSetWindowOrgEx(HDC, int, int, LPPOINT);
extern BOOL WINAPI extGetCurrentPositionEx(HDC, LPPOINT);
extern BOOL WINAPI extCreateScalableFontResourceA(DWORD, LPCTSTR, LPCTSTR, LPCTSTR);
extern int WINAPI extAddFontResourceA(LPCTSTR);
extern BOOL WINAPI extCreateScalableFontResourceW(DWORD, LPCWSTR, LPCWSTR, LPCWSTR);
extern int WINAPI extAddFontResourceW(LPCWSTR);
extern BOOL WINAPI extAnimatePalette(HPALETTE, UINT, UINT, const PALETTEENTRY *);
extern UINT WINAPI extSetSystemPaletteUse(HDC, UINT);
extern BOOL WINAPI extGDISetPixelFormat(HDC, int, const PIXELFORMATDESCRIPTOR *);
extern int WINAPI extGDIGetPixelFormat(HDC);
extern int WINAPI extChoosePixelFormat(HDC, const PIXELFORMATDESCRIPTOR *);
extern int WINAPI extDescribePixelFormat(HDC, int, UINT, LPPIXELFORMATDESCRIPTOR);
extern DWORD WINAPI extGetObjectType(HGDIOBJ);

// Kernel32.dll:
extern BOOL WINAPI extGetDiskFreeSpaceA(LPCSTR, LPDWORD, LPDWORD, LPDWORD, LPDWORD);
extern UINT WINAPI extGetDriveType(LPCSTR);
extern void WINAPI extGetLocalTime(LPSYSTEMTIME);
extern FARPROC WINAPI extGetProcAddress(HMODULE, LPCSTR);
extern void WINAPI extGetSystemTime(LPSYSTEMTIME);
extern void WINAPI extGetSystemTimeAsFileTime(LPFILETIME);
extern DWORD WINAPI extGetTickCount(void);
extern DWORD WINAPI extGetVersion(void);
extern BOOL WINAPI extGetVersionExA(LPOSVERSIONINFOA);
extern BOOL WINAPI extGetVersionExW(LPOSVERSIONINFOW);
extern void WINAPI extGlobalMemoryStatus(LPMEMORYSTATUS);
extern int WINAPI extIsDebuggerPresent(void);
extern HMODULE WINAPI extLoadLibraryA(LPCTSTR);
extern HMODULE WINAPI extLoadLibraryExA(LPCTSTR, HANDLE, DWORD);
extern HMODULE WINAPI extLoadLibraryW(LPCWSTR);
extern HMODULE WINAPI extLoadLibraryExW(LPCWSTR, HANDLE, DWORD);
extern BOOL WINAPI extReadFile(HANDLE, LPVOID, DWORD, LPDWORD, LPOVERLAPPED);
extern LPTOP_LEVEL_EXCEPTION_FILTER WINAPI extSetUnhandledExceptionFilter(LPTOP_LEVEL_EXCEPTION_FILTER);
extern void WINAPI extSleep(DWORD);
extern DWORD WINAPI extSleepEx(DWORD, BOOL);
extern HANDLE WINAPI extCreateFile(LPCTSTR, DWORD, DWORD, LPSECURITY_ATTRIBUTES, DWORD, DWORD, HANDLE);
extern DWORD WINAPI extSetFilePointer(HANDLE, LONG, PLONG, DWORD);
extern BOOL WINAPI extCloseHandle(HANDLE);
extern BOOL WINAPI extCreateProcessA(LPCTSTR, LPTSTR, LPSECURITY_ATTRIBUTES, LPSECURITY_ATTRIBUTES, BOOL, DWORD, LPVOID, LPCTSTR, LPSTARTUPINFO, LPPROCESS_INFORMATION);
extern BOOL WINAPI extQueryPerformanceFrequency(LARGE_INTEGER *);
extern BOOL WINAPI extQueryPerformanceCounter(LARGE_INTEGER *);

// ole32.dll:
extern HRESULT STDAPICALLTYPE extCoCreateInstance(REFCLSID, LPUNKNOWN, DWORD, REFIID, LPVOID FAR*);
extern HRESULT STDAPICALLTYPE extCoCreateInstanceEx(REFCLSID, IUnknown *, DWORD, COSERVERINFO *, DWORD, MULTI_QI *);
extern HRESULT STDAPICALLTYPE extCoInitialize(LPVOID);

// user32.dll:
extern HDC WINAPI extBeginPaint(HWND, LPPAINTSTRUCT);
extern HDC WINAPI extDDBeginPaint(HWND, LPPAINTSTRUCT);
extern LRESULT WINAPI extCallWindowProc(WNDPROC, HWND, UINT, WPARAM, LPARAM);
extern LONG WINAPI extChangeDisplaySettingsA(DEVMODEA *, DWORD);
extern LONG WINAPI extChangeDisplaySettingsExA(LPCTSTR, DEVMODEA *, HWND, DWORD, LPVOID);
extern LONG WINAPI extChangeDisplaySettingsW(DEVMODEW *, DWORD);
extern LONG WINAPI extChangeDisplaySettingsExW(LPCTSTR, DEVMODEW *, HWND, DWORD, LPVOID);
extern BOOL WINAPI extClientToScreen(HWND, LPPOINT);
extern BOOL WINAPI extClipCursor(LPRECT);
extern HWND WINAPI extCreateDialogIndirectParam(HINSTANCE, LPCDLGTEMPLATE, HWND, DLGPROC, LPARAM);
extern HWND WINAPI extCreateDialogParam(HINSTANCE, LPCSTR, HWND, DLGPROC, LPARAM);
extern HWND WINAPI extCreateWindowExA(DWORD, LPCTSTR, LPCTSTR, DWORD, int, int, int, int, HWND, HMENU, HINSTANCE, LPVOID);
extern HWND WINAPI extCreateWindowExW(DWORD, LPCWSTR, LPCWSTR, DWORD, int, int, int, int, HWND, HMENU, HINSTANCE, LPVOID);
extern LRESULT WINAPI extDefWindowProc(HWND, UINT, WPARAM, LPARAM);
extern BOOL WINAPI extEndPaint(HWND, const PAINTSTRUCT *);
extern BOOL WINAPI extDDEndPaint(HWND, const PAINTSTRUCT *);
extern LONG WINAPI extEnumDisplaySettings(LPCTSTR, DWORD, DEVMODE *);
extern int WINAPI extFillRect(HDC, const RECT *, HBRUSH);
extern int WINAPI extFrameRect(HDC, const RECT *, HBRUSH);
extern BOOL WINAPI extGetClientRect(HWND, LPRECT);
extern BOOL WINAPI extGetClipCursor(LPRECT);
extern BOOL WINAPI extGetCursorPos(LPPOINT);
extern HDC WINAPI extGDIGetDC(HWND);
extern HDC WINAPI extDDGetDC(HWND);
extern HWND WINAPI extGetDesktopWindow(void);
extern int WINAPI extGetSystemMetrics(int);
extern HWND WINAPI extGetTopWindow(HWND);
extern HDC WINAPI extGDIGetWindowDC(HWND);
extern HDC WINAPI extDDGetWindowDC(HWND);
extern LONG WINAPI extGetWindowLong(HWND, int);
extern BOOL WINAPI extGetWindowRect(HWND, LPRECT);
extern BOOL WINAPI extInvalidateRect(HWND, RECT *, BOOL);
//extern BOOL WINAPI extDDInvalidateRect(HWND, RECT *, BOOL);
extern int WINAPI extMapWindowPoints(HWND, HWND, LPPOINT, UINT);
extern BOOL WINAPI extMoveWindow(HWND, int, int, int, int, BOOL);
extern BOOL WINAPI extPeekMessage(LPMSG, HWND, UINT, UINT, UINT);
extern ATOM WINAPI extRegisterClassExA(WNDCLASSEXA *);
extern ATOM WINAPI extRegisterClassA(WNDCLASSA *);
extern int WINAPI extDDReleaseDC(HWND, HDC);
extern int WINAPI extGDIReleaseDC(HWND, HDC);
extern BOOL WINAPI extScreenToClient(HWND, LPPOINT);
extern LRESULT WINAPI extSendMessageA(HWND, UINT, WPARAM, LPARAM);
extern LRESULT WINAPI extSendMessageW(HWND, UINT, WPARAM, LPARAM);
extern HCURSOR WINAPI extSetCursor(HCURSOR);
extern BOOL WINAPI extSetCursorPos(int, int);
extern UINT_PTR WINAPI extSetTimer(HWND, UINT_PTR, UINT, TIMERPROC);
extern LONG WINAPI extSetWindowLong(HWND, int, LONG);
extern BOOL WINAPI extSetWindowPos(HWND, HWND, int, int, int, int, UINT);
extern int WINAPI extShowCursor(BOOL);
extern BOOL WINAPI extShowWindow(HWND, int);
extern LONG WINAPI extTabbedTextOutA(HDC, int, int, LPCTSTR, int, int, const LPINT, int);
extern BOOL WINAPI extDestroyWindow(HWND);
extern BOOL WINAPI extCloseWindow(HWND);
extern BOOL WINAPI extSetSysColors(int, const INT *, const COLORREF *);
extern HDC WINAPI extGDIGetDCEx(HWND, HRGN, DWORD);
extern BOOL WINAPI extUpdateWindow(HWND);
extern BOOL WINAPI extGetWindowPlacement(HWND, WINDOWPLACEMENT *);
extern BOOL WINAPI extSetWindowPlacement(HWND, WINDOWPLACEMENT *);
extern HWND WINAPI extSetCapture(HWND);
extern HWND WINAPI extGetForegroundWindow(void);
extern HWND WINAPI extGetActiveWindow(void);
extern BOOL WINAPI extIsWindowVisible(HWND);
extern BOOL WINAPI extSystemParametersInfoA(UINT, UINT, PVOID, UINT);
extern BOOL WINAPI extKillTimer(HWND, UINT_PTR);
extern BOOL WINAPI extGetUpdateRect(HWND, LPRECT, BOOL);

// Winmm.dll:
extern MCIERROR WINAPI extmciSendCommand(MCIDEVICEID, UINT, DWORD_PTR, DWORD_PTR);
extern DWORD WINAPI exttimeGetTime(void);
extern MMRESULT WINAPI exttimeSetEvent(UINT, UINT, LPTIMECALLBACK, DWORD_PTR, UINT);
extern MMRESULT WINAPI exttimeKillEvent(UINT);

// extern function declaration

extern void	HookKernel32Init();
extern void	HookUser32Init();
extern void	HookGDI32Init();

/* eof */