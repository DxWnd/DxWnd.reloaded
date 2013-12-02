/* ===================================================================

syslibs.h: contains typedef definition and extern pointer declarations
for all system calls hooked by DxWnd:
GDI32.dll
Kernel32.dll
ole32.dll
user32.dll.dll

====================================================================*/

/* ===================================================================
type definitions
====================================================================*/

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
typedef int		(WINAPI *GetDeviceCaps_Type)(HDC, int);
typedef BOOL	(WINAPI *GetDeviceGammaRamp_Type)(HDC, LPVOID);
typedef BOOL	(WINAPI *GetSystemPaletteEntries_Type)(HDC, UINT, UINT, LPPALETTEENTRY);
typedef BOOL	(WINAPI *PatBlt_Type)(HDC, int, int, int, int, DWORD);
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
typedef BOOL	(WINAPI *TextOut_Type)(HDC, int, int, LPCTSTR, int);

// Kernel32.dll:
typedef BOOL	(WINAPI *GetDiskFreeSpaceA_Type)(LPCSTR, LPDWORD, LPDWORD, LPDWORD, LPDWORD);
typedef void	(WINAPI *GetLocalTime_Type)(LPSYSTEMTIME);
typedef FARPROC (WINAPI *GetProcAddress_Type)(HMODULE, LPCSTR);
typedef void	(WINAPI *GetSystemTime_Type)(LPSYSTEMTIME);
typedef void	(WINAPI *GetSystemTimeAsFileTime_Type)(LPFILETIME);
typedef DWORD	(WINAPI *GetTickCount_Type)(void);
typedef DWORD	(WINAPI *GetVersion_Type)(void);
typedef BOOL	(WINAPI *GetVersionEx_Type)(LPOSVERSIONINFO);
typedef HMODULE (WINAPI *LoadLibraryA_Type)(LPCTSTR);
typedef HMODULE (WINAPI *LoadLibraryExA_Type)(LPCTSTR, HANDLE, DWORD);
typedef HMODULE (WINAPI *LoadLibraryW_Type)(LPCWSTR);
typedef HMODULE (WINAPI *LoadLibraryExW_Type)(LPCWSTR, HANDLE, DWORD);
typedef LPTOP_LEVEL_EXCEPTION_FILTER 
				(WINAPI *SetUnhandledExceptionFilter_Type)(LPTOP_LEVEL_EXCEPTION_FILTER);
typedef void	(WINAPI *Sleep_Type)(DWORD);
typedef DWORD	(WINAPI *SleepEx_Type)(DWORD, BOOL);

// ole32.dll:
typedef HRESULT (STDAPICALLTYPE *CoCreateInstance_Type)(REFCLSID rclsid, LPUNKNOWN pUnkOuter, DWORD dwClsContext, REFIID riid, LPVOID FAR* ppv);

// user32.dll:
typedef HDC		(WINAPI *BeginPaint_Type)(HWND, LPPAINTSTRUCT);
typedef LRESULT (WINAPI *CallWindowProc_Type)(WNDPROC, HWND, UINT, WPARAM, LPARAM);
typedef LONG	(WINAPI *ChangeDisplaySettings_Type)(DEVMODE *, DWORD);
typedef LONG	(WINAPI *ChangeDisplaySettingsEx_Type)(LPCTSTR, DEVMODE *, HWND, DWORD, LPVOID);
typedef BOOL	(WINAPI *ClientToScreen_Type)(HWND, LPPOINT);
typedef BOOL	(WINAPI *ClipCursor_Type)(const RECT *);
typedef HWND	(WINAPI *CreateDialogIndirectParam_Type)(HINSTANCE, LPCDLGTEMPLATE, HWND, DLGPROC, LPARAM);
typedef HWND	(WINAPI *CreateDialogParam_Type)(HINSTANCE, LPCTSTR, HWND, DLGPROC, LPARAM);
typedef HWND	(WINAPI *CreateWindowExA_Type)(DWORD, LPCTSTR, LPCTSTR, DWORD, int, int, int, int, HWND, HMENU, HINSTANCE, LPVOID);
typedef LRESULT (WINAPI *DefWindowProc_Type)(HWND, UINT, WPARAM, LPARAM);
typedef BOOL	(WINAPI *EndPaint_Type)(HWND, const PAINTSTRUCT *);
typedef LONG	(WINAPI *EnumDisplaySettings_Type)(LPCTSTR, DWORD, LPDEVMODEA);
typedef int		(WINAPI *FillRect_Type)(HDC, const RECT *, HBRUSH);
typedef BOOL	(WINAPI *GetClientRect_Type)(HWND, LPRECT);
typedef BOOL	(WINAPI *GetClipCursor_Type)(LPRECT);
typedef BOOL	(WINAPI *GetCursorPos_Type)(LPPOINT);
typedef HDC		(WINAPI *GDIGetDC_Type)(HWND);
typedef HWND	(WINAPI *GetDesktopWindow_Type)(void);
typedef BOOL	(WINAPI *GetMessage_Type)(LPMSG, HWND, UINT, UINT);
typedef int		(WINAPI *GetSystemMetrics_Type)(int);
typedef LONG	(WINAPI *GetWindowLong_Type)(HWND, int);
typedef BOOL	(WINAPI *GetWindowRect_Type)(HWND, LPRECT);
typedef BOOL	(WINAPI *InvalidateRect_Type)(HWND, CONST RECT *, BOOL);
typedef int		(WINAPI *MapWindowPoints_Type)(HWND, HWND, LPPOINT, UINT);
typedef BOOL	(WINAPI *MoveWindow_Type)(HWND, int, int, int, int, BOOL);
typedef BOOL	(WINAPI *PeekMessage_Type)(LPMSG, HWND, UINT, UINT, UINT);
typedef ATOM	(WINAPI *RegisterClassExA_Type)(WNDCLASSEX *);
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

// Winmm.dll:
typedef DWORD	(WINAPI *timeGetTime_Type)(void);

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
DXWEXTERN GetDeviceCaps_Type pGDIGetDeviceCaps DXWINITIALIZED;
DXWEXTERN GetDeviceGammaRamp_Type pGDIGetDeviceGammaRamp DXWINITIALIZED;
DXWEXTERN GetSystemPaletteEntries_Type pGDIGetSystemPaletteEntries DXWINITIALIZED;
DXWEXTERN PatBlt_Type pGDIPatBlt DXWINITIALIZED;
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
DXWEXTERN TextOut_Type pGDITextOutA DXWINITIALIZED;

// Kernel32.dll:
DXWEXTERN GetDiskFreeSpaceA_Type pGetDiskFreeSpaceA DXWINITIALIZED;
DXWEXTERN GetLocalTime_Type pGetLocalTime DXWINITIALIZED;
DXWEXTERN GetProcAddress_Type pGetProcAddress DXWINITIALIZED;
DXWEXTERN GetSystemTime_Type pGetSystemTime DXWINITIALIZED;
DXWEXTERN GetSystemTimeAsFileTime_Type pGetSystemTimeAsFileTime DXWINITIALIZED;
DXWEXTERN GetTickCount_Type pGetTickCount DXWINITIALIZED;
DXWEXTERN GetVersion_Type pGetVersion DXWINITIALIZED;
DXWEXTERN GetVersionEx_Type pGetVersionEx DXWINITIALIZED;
DXWEXTERN LoadLibraryA_Type pLoadLibraryA DXWINITIALIZED;
DXWEXTERN LoadLibraryExA_Type pLoadLibraryExA DXWINITIALIZED;
DXWEXTERN LoadLibraryW_Type pLoadLibraryW DXWINITIALIZED;
DXWEXTERN LoadLibraryExW_Type pLoadLibraryExW DXWINITIALIZED;
DXWEXTERN SetUnhandledExceptionFilter_Type pSetUnhandledExceptionFilter DXWINITIALIZED;
DXWEXTERN Sleep_Type pSleep DXWINITIALIZED;
DXWEXTERN SleepEx_Type pSleepEx DXWINITIALIZED;

// ole32.dll:
DXWEXTERN CoCreateInstance_Type pCoCreateInstance DXWINITIALIZED;

// user32.dll:
DXWEXTERN BeginPaint_Type pBeginPaint DXWINITIALIZED;
DXWEXTERN CallWindowProc_Type pCallWindowProc DXWINITIALIZED;
DXWEXTERN ChangeDisplaySettings_Type pChangeDisplaySettings DXWINITIALIZED;
DXWEXTERN ChangeDisplaySettingsEx_Type pChangeDisplaySettingsEx DXWINITIALIZED;
DXWEXTERN ClientToScreen_Type pClientToScreen DXWINITIALIZED;
DXWEXTERN ClipCursor_Type pClipCursor DXWINITIALIZED;
DXWEXTERN CreateDialogIndirectParam_Type pCreateDialogIndirectParam DXWINITIALIZED;
DXWEXTERN CreateDialogParam_Type pCreateDialogParam DXWINITIALIZED;
DXWEXTERN CreateWindowExA_Type pCreateWindowExA DXWINITIALIZED;
DXWEXTERN DefWindowProc_Type pDefWindowProc DXWINITIALIZED;
DXWEXTERN EndPaint_Type pEndPaint DXWINITIALIZED;
DXWEXTERN EnumDisplaySettings_Type pEnumDisplaySettings DXWINITIALIZED;
DXWEXTERN FillRect_Type pFillRect DXWINITIALIZED;
DXWEXTERN GetClientRect_Type pGetClientRect DXWINITIALIZED;
DXWEXTERN GetClipCursor_Type pGetClipCursor DXWINITIALIZED;
DXWEXTERN GetCursorPos_Type pGetCursorPos DXWINITIALIZED;
DXWEXTERN GDIGetDC_Type pGDIGetDC DXWINITIALIZED;
DXWEXTERN GetDesktopWindow_Type pGetDesktopWindow DXWINITIALIZED;
DXWEXTERN GetMessage_Type pGetMessage DXWINITIALIZED;
DXWEXTERN GetSystemMetrics_Type pGetSystemMetrics DXWINITIALIZED;
DXWEXTERN GDIGetDC_Type pGDIGetWindowDC DXWINITIALIZED;
DXWEXTERN GetWindowLong_Type pGetWindowLong DXWINITIALIZED;
DXWEXTERN GetWindowRect_Type pGetWindowRect DXWINITIALIZED;
DXWEXTERN InvalidateRect_Type pInvalidateRect DXWINITIALIZED;
DXWEXTERN MapWindowPoints_Type pMapWindowPoints DXWINITIALIZED;
DXWEXTERN MoveWindow_Type pMoveWindow DXWINITIALIZED;
DXWEXTERN PeekMessage_Type pPeekMessage DXWINITIALIZED;
DXWEXTERN RegisterClassExA_Type pRegisterClassExA DXWINITIALIZED;
DXWEXTERN GDIReleaseDC_Type pGDIReleaseDC DXWINITIALIZED;
DXWEXTERN ScreenToClient_Type pScreenToClient DXWINITIALIZED;
DXWEXTERN SendMessage_Type pSendMessage DXWINITIALIZED;
DXWEXTERN SetCursor_Type pSetCursor DXWINITIALIZED;
DXWEXTERN SetCursorPos_Type pSetCursorPos DXWINITIALIZED;
DXWEXTERN SetTimer_Type pSetTimer DXWINITIALIZED;
DXWEXTERN SetWindowLong_Type pSetWindowLong DXWINITIALIZED;
DXWEXTERN SetWindowPos_Type pSetWindowPos DXWINITIALIZED;
DXWEXTERN ShowCursor_Type pShowCursor DXWINITIALIZED;
DXWEXTERN ShowWindow_Type pShowWindow DXWINITIALIZED;

// Winmm.dll:
DXWEXTERN timeGetTime_Type ptimeGetTime DXWINITIALIZED;

/* ===================================================================
hook procedures (possibly more tnan one per each API...)
====================================================================*/

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
extern HDC WINAPI extDDCreateCompatibleDC(HDC);
extern HDC WINAPI extGDICreateDC(LPSTR, LPSTR, LPSTR, CONST DEVMODE *);
extern HDC WINAPI extDDCreateDC(LPSTR, LPSTR, LPSTR, CONST DEVMODE *);
extern HFONT WINAPI extCreateFont(int, int, int, int, int, DWORD, DWORD, DWORD, DWORD, DWORD, DWORD, DWORD, DWORD, LPCTSTR);
extern HFONT WINAPI extCreateFontIndirect(const LOGFONT*);
extern HPALETTE WINAPI extGDICreatePalette(CONST LOGPALETTE *);
extern HDWP WINAPI extDeferWindowPos(HDWP, HWND, HWND, int, int, int, int, UINT);
extern BOOL WINAPI extGDIDeleteDC(HDC);
extern BOOL WINAPI extDDDeleteDC(HDC);
extern int WINAPI extGetDeviceCaps(HDC, int);
extern BOOL WINAPI extGetDeviceGammaRamp(HDC, LPVOID);
extern UINT WINAPI extGetSystemPaletteEntries(HDC, UINT, UINT, LPPALETTEENTRY);
extern BOOL WINAPI extGDIPatBlt(HDC, int, int, int, int, DWORD);
extern BOOL WINAPI extDDBitBlt(HDC, int, int, int, int, HDC, int, int, DWORD);
extern UINT WINAPI extRealizePalette(HDC);
extern BOOL WINAPI extRectangle(HDC, int, int, int, int);
extern BOOL WINAPI extGDIRestoreDC(HDC, int);
extern int WINAPI extGDISaveDC(HDC);
extern BOOL WINAPI extScaleWindowExtEx(HDC, int, int, int, int, LPSIZE);
extern HPALETTE WINAPI extSelectPalette(HDC, HPALETTE, BOOL);
extern COLORREF WINAPI extSetBkColor(HDC, COLORREF);
extern BOOL WINAPI extSetDeviceGammaRamp(HDC, LPVOID);
extern COLORREF WINAPI extSetTextColor(HDC, COLORREF);
extern BOOL WINAPI extGDIStretchBlt(HDC, int, int, int, int, HDC, int, int, int, int, DWORD);
extern BOOL WINAPI extDDStretchBlt(HDC, int, int, int, int, HDC, int, int, int, int, DWORD);
extern BOOL WINAPI extTextOutA(HDC, int, int, LPCTSTR, int);

// Kernel32.dll:
extern BOOL WINAPI extGetDiskFreeSpaceA(LPCSTR, LPDWORD, LPDWORD, LPDWORD, LPDWORD);
extern void WINAPI extGetLocalTime(LPSYSTEMTIME);
extern FARPROC WINAPI extGetProcAddress(HMODULE, LPCSTR);
extern void WINAPI extGetSystemTime(LPSYSTEMTIME);
extern void WINAPI extGetSystemTimeAsFileTime(LPFILETIME);
extern DWORD WINAPI extGetTickCount(void);
extern DWORD WINAPI extGetVersion(void);
extern BOOL WINAPI extGetVersionEx(LPOSVERSIONINFO);
extern HMODULE WINAPI extLoadLibraryA(LPCTSTR);
extern HMODULE WINAPI extLoadLibraryExA(LPCTSTR, HANDLE, DWORD);
extern HMODULE WINAPI extLoadLibraryW(LPCWSTR);
extern HMODULE WINAPI extLoadLibraryExW(LPCWSTR, HANDLE, DWORD);
extern LPTOP_LEVEL_EXCEPTION_FILTER WINAPI extSetUnhandledExceptionFilter(LPTOP_LEVEL_EXCEPTION_FILTER);
extern void WINAPI extSleep(DWORD);
extern DWORD WINAPI extSleepEx(DWORD, BOOL);

// ole32.dll:
extern HRESULT STDAPICALLTYPE extCoCreateInstance(REFCLSID, LPUNKNOWN, DWORD, REFIID, LPVOID FAR*);

// user32.dll:
extern HDC WINAPI extBeginPaint(HWND, LPPAINTSTRUCT);
extern LRESULT WINAPI extCallWindowProc(WNDPROC, HWND, UINT, WPARAM, LPARAM);
extern LONG WINAPI extChangeDisplaySettings(DEVMODE *, DWORD);
extern LONG WINAPI extChangeDisplaySettingsEx(LPCTSTR, DEVMODE *, HWND, DWORD, LPVOID);
extern BOOL WINAPI extClientToScreen(HWND, LPPOINT);
extern BOOL WINAPI extClipCursor(LPRECT);
extern HWND WINAPI extCreateDialogIndirectParam(HINSTANCE, LPCDLGTEMPLATE, HWND, DLGPROC, LPARAM);
extern HWND WINAPI extCreateDialogParam(HINSTANCE, LPCSTR, HWND, DLGPROC, LPARAM);
extern HWND WINAPI extCreateWindowExA(DWORD, LPCTSTR, LPCTSTR, DWORD, int, int, int, int, HWND, HMENU, HINSTANCE, LPVOID);
extern LRESULT WINAPI extDefWindowProc(HWND, UINT, WPARAM, LPARAM);
extern BOOL WINAPI extEndPaint(HWND, const PAINTSTRUCT *);
extern LONG WINAPI extEnumDisplaySettings(LPCTSTR, DWORD, DEVMODE *);
extern int WINAPI extFillRect(HDC, const RECT *, HBRUSH);
extern BOOL WINAPI extGetClientRect(HWND, LPRECT);
extern BOOL WINAPI extGetClipCursor(LPRECT);
extern BOOL WINAPI extGetCursorPos(LPPOINT);
extern HDC WINAPI extGDIGetDC(HWND);
extern HDC WINAPI extDDGetDC(HWND);
extern HWND WINAPI extGetDesktopWindow(void);
extern BOOL WINAPI extGetMessage(LPMSG, HWND, UINT, UINT);
extern int WINAPI extGetSystemMetrics(int);
extern HDC WINAPI extGDIGetWindowDC(HWND);
extern HDC WINAPI extDDGetWindowDC(HWND);
extern LONG WINAPI extGetWindowLong(HWND, int);
extern BOOL WINAPI extGetWindowRect(HWND, LPRECT);
extern BOOL WINAPI extInvalidateRect(HWND, RECT *, BOOL);
extern BOOL WINAPI extDDInvalidateRect(HWND, RECT *, BOOL);
extern int WINAPI extMapWindowPoints(HWND, HWND, LPPOINT, UINT);
extern BOOL WINAPI extMoveWindow(HWND, int, int, int, int, BOOL);
extern BOOL WINAPI extPeekMessage(LPMSG, HWND, UINT, UINT, UINT);
extern ATOM WINAPI extRegisterClassExA(WNDCLASSEXA *);
extern int WINAPI extDDReleaseDC(HWND, HDC);
extern int WINAPI extGDIReleaseDC(HWND, HDC);
extern BOOL WINAPI extScreenToClient(HWND, LPPOINT);
extern LRESULT WINAPI extSendMessage(HWND, UINT, WPARAM, LPARAM);
extern HCURSOR WINAPI extSetCursor(HCURSOR);
extern BOOL WINAPI extSetCursorPos(int, int);
extern UINT_PTR WINAPI extSetTimer(HWND, UINT_PTR, UINT, TIMERPROC);
extern LONG WINAPI extSetWindowLong(HWND, int, LONG);
extern BOOL WINAPI extSetWindowPos(HWND, HWND, int, int, int, int, UINT);
extern int WINAPI extShowCursor(BOOL);
extern BOOL WINAPI extShowWindow(HWND, int);

// Winmm.dll:
extern DWORD WINAPI exttimeGetTime(void);

/* eof */