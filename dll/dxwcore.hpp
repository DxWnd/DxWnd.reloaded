#include <windows.h>

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
	void ScreenRefresh(void);
	BOOL HandleFPS();

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

// Implementation
protected:
	DWORD dwScreenWidth;
	DWORD dwScreenHeight;
	BOOL FullScreen;
	HWND hWnd;
};

extern dxwCore dxw;
