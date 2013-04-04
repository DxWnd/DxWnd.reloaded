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
	void SethWnd(HWND hwnd) {hWnd=hwnd;}
	void InitWindowPos(int, int, int, int);
	HWND GethWnd(void) {return hWnd;}
	void SetScreenSize(void) {dwScreenWidth=800; dwScreenHeight=600;}
	void SetScreenSize(int x, int y) {if(x)dwScreenWidth=x; if(y)dwScreenHeight=y;}
	DWORD GetScreenWidth(void) {return dwScreenWidth;}
	DWORD GetScreenHeight(void) {return dwScreenHeight;}
	RECT GetScreenRect(void);
	void MapRect(int *, int *, int *, int *);
	void SetFullScreen(BOOL fs) {FullScreen=fs;}
	BOOL IsFullScreen() {return FullScreen;}
	POINT FixCursorPos(POINT);
	POINT FixCursorPos(HWND, POINT);
	void FixNCHITCursorPos(LPPOINT);
	void SetClipCursor(void);
	void EraseClipCursor(void);
	RECT MapWindowRect(LPRECT lpRect);
	RECT MapWindowRect(void);
	void MapPoint(LPPOINT);
	void UnmapPoint(LPPOINT);
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
	void ShowFPS(HDC);
	char *GetTSCaption(int);
	char *GetTSCaption(void);
	void DoSlow(int);
	void ShowBanner(HWND);
	POINT ScreenToClient(POINT);

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

// Implementation
protected:
	DWORD dwScreenWidth;
	DWORD dwScreenHeight;
	BOOL FullScreen;
	HWND hWnd;
	DWORD PrimSurfaces[DDSQLEN+1];
};

extern dxwCore dxw;
