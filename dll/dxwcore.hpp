#include <windows.h>
#include "syslibs.h"

#define DDSQLEN 0x10

class dxwCore
{
// Construction/destruction
public:
    dxwCore();
    virtual ~dxwCore();

// Operations
public: // methods
	void InitTarget(TARGETMAP *);
	void SethWnd(HWND hwnd);
	void SethWnd(HWND hwnd, HWND hwndfps) {hWnd=hwnd; hWndFPS=hwndfps;}
	void InitWindowPos(int, int, int, int);
	HWND GethWnd(void) {return hWnd;}
	void SetScreenSize(void) {dwScreenWidth=800; dwScreenHeight=600;}
	void SetScreenSize(int x, int y) {if(x)dwScreenWidth=x; if(y)dwScreenHeight=y;}
	DWORD GetScreenWidth(void) {return dwScreenWidth;}
	DWORD GetScreenHeight(void) {return dwScreenHeight;}
	void SetFullScreen(BOOL);
	void SetFullScreen(BOOL, int);
	BOOL IsFullScreen();
	BOOL IsDesktop(HWND);
	BOOL IsRealDesktop(HWND);
	POINT FixCursorPos(POINT);
	void FixNCHITCursorPos(LPPOINT);
	void SetClipCursor(void);
	void EraseClipCursor(void);
	RECT MapWindowRect(LPRECT lpRect);
	RECT MapWindowRect(void);
	void MapClient(LPPOINT);
	void MapClient(LPRECT);
	void MapClient(int *, int *, int *, int *);
	void MapClient(int *, int *);
	void UnmapClient(LPPOINT);
	void UnmapClient(int *, int *);
	void MapWindow(LPPOINT);
	void MapWindow(LPRECT);
	void MapWindow(int *, int *, int *, int *);
	RECT GetScreenRect(void);
	RECT GetWindowRect(RECT);
	RECT GetClientRect(RECT);
	POINT AddCoordinates(POINT, POINT);
	POINT SubCoordinates(POINT, POINT);
	POINT ClientOffset(HWND);
	void ScreenRefresh(void);
	BOOL HandleFPS(void);
	DWORD GetTickCount(void);
	void MarkPrimarySurface(LPDIRECTDRAWSURFACE);
	void UnmarkPrimarySurface(LPDIRECTDRAWSURFACE);
	BOOL IsAPrimarySurface(LPDIRECTDRAWSURFACE);
	LPDIRECTDRAWSURFACE GetPrimarySurface(void);
	void SetPrimarySurface(void);
	void ResetPrimarySurface(void);
	void GetSystemTime(LPSYSTEMTIME lpSystemTime);
	void GetSystemTimeAsFileTime(LPFILETIME lpSystemTimeAsFileTime);
	DWORD StretchTime(DWORD);
	void ShowFPS(LPDIRECTDRAWSURFACE);
	void ShowFPS(void);
	void ShowFPS(HDC);
	char *GetTSCaption(int);
	char *GetTSCaption(void);
	void DoSlow(int);
	void ShowBanner(HWND);
	POINT ScreenToClient(POINT);
	int GetDLLIndex(char *);
	void FixStyle(char *, HWND, WPARAM, LPARAM);
	HDC AcquireEmulatedDC(HWND);
	HDC AcquireEmulatedDC(HDC);
	BOOL ReleaseEmulatedDC(HWND);
	BOOL IsVirtual(HDC);
	void ResetEmulatedDC();
	int VirtualOffsetX, VirtualOffsetY;

public: // simple data variables
	DDPIXELFORMAT ActualPixelFormat;
	DDPIXELFORMAT VirtualPixelFormat;
	DWORD dwPrimarySurfaceCaps;
	DWORD dwBackBufferCount;
	DWORD dwDDVersion;
	DWORD dwTargetDDVersion;
	DWORD dwMaxDDVersion;
	DWORD dwFlags1;
	DWORD dwFlags2;
	DWORD dwFlags3;
	DWORD dwFlags4;
	DWORD dwTFlags;
	HWND hParentWnd;
	HWND hChildWnd;
	BOOL bActive;
	BOOL bDInputAbs;
	DWORD MaxFPS;
	char *gsModules;
	int TimeShift;
	LPDIRECTDRAWSURFACE lpDDSPrimHDC;
	short iPosX;
	short iPosY;
	short iSizX;
	short iSizY;
	//BOOL IsWithinDDraw; // flag to avoid double intervention at ddraw & GDI level
	BOOL IsGDIPalette;
	char *CustomOpenGLLib;
    WORD palVersion;
    WORD palNumEntries;
    PALETTEENTRY palPalEntry[256];
	short FakeVersionId;
	int Coordinates;

// Implementation
protected:
	DWORD dwScreenWidth;
	DWORD dwScreenHeight;
	BOOL FullScreen;
	HWND hWnd, hWndFPS;
	DWORD PrimSurfaces[DDSQLEN+1];
	HDC VirtualHDC;
	HBITMAP VirtualPic;
};

extern dxwCore dxw;

typedef enum {
	SYSLIBIDX_KERNEL32 = 0,
	SYSLIBIDX_USER32,
	SYSLIBIDX_GDI32,
	SYSLIBIDX_IMELIB,
	SYSLIBIDX_ADVAPI32,
	SYSLIBIDX_OLE32,
	SYSLIBIDX_DIRECTDRAW,
	SYSLIBIDX_DIRECT3D8,
	SYSLIBIDX_DIRECT3D9,
	SYSLIBIDX_DIRECT3D10,
	SYSLIBIDX_DIRECT3D10_1,
	SYSLIBIDX_DIRECT2D11,
	SYSLIBIDX_OPENGL,
	SYSLIBIDX_MSVFW,
	SYSLIBIDX_SMACK,
	SYSLIBIDX_VERSION,
	SYSLIBIDX_DPLAYX,
	SYSLIBIDX_DSOUND,
	SYSLIBIDX_WINMM,
	SYSLIBIDX_IMM32,
	SYSLIBIDX_WSOCK,
	SYSLIBIDX_DINPUT,
	SYSLIBIDX_DINPUT8,
	SYSLIBIDX_SHFOLDER,
	SYSLIBIDX_SHELL32,
	SYSLIBIDX_WS2_32,
	SYSLIBIDX_TAPI32,
	SYSLIBIDX_NETAPI32,
	SYSLIBIDX_WINTRUST,
	SYSLIBIDX_MAX } 
enum_syslibraries;
