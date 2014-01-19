#include <windows.h>
#include "syslibs.h"

#define DDSQLEN 0x10

typedef struct {
	DWORD dwTimerType;
	union{
		struct {
			UINT uTimerId;
			UINT uDelay;
			UINT uResolution;
			LPTIMECALLBACK lpTimeProc;
			DWORD_PTR dwUser; 
			UINT fuEvent;
		};
		struct {
			HWND hWnd;
			UINT_PTR nIDEvent;
			UINT uElapse;
			TIMERPROC lpTimerFunc;
		};
	} t;
} TimerEvent_Type;

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
	void SetScreenSize(void);
	void SetScreenSize(int x, int y);
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
	void FixWorkarea(LPRECT);
	POINT FixMessagePt(HWND, POINT);
	RECT GetScreenRect(void);
	RECT GetUnmappedScreenRect();
	RECT GetWindowRect(RECT);
	RECT GetClientRect(RECT);
	POINT AddCoordinates(POINT, POINT);
	POINT SubCoordinates(POINT, POINT);
	POINT ClientOffset(HWND);
	void ScreenRefresh(void);
	BOOL HandleFPS(void);
	DWORD GetTickCount(void);
	void MarkPrimarySurface(LPDIRECTDRAWSURFACE);
	BOOL IsAPrimarySurface(LPDIRECTDRAWSURFACE);
	LPDIRECTDRAWSURFACE GetPrimarySurface(void);
	void MarkBackBufferSurface(LPDIRECTDRAWSURFACE);
	BOOL IsABackBufferSurface(LPDIRECTDRAWSURFACE);
	LPDIRECTDRAWSURFACE GetBackBufferSurface(void);
	void MarkRegularSurface(LPDIRECTDRAWSURFACE);
	void SetPrimarySurface(void);
	void ResetPrimarySurface(void);
	void GetSystemTime(LPSYSTEMTIME);
	void GetSystemTimeAsFileTime(LPFILETIME);
	DWORD StretchTime(DWORD);
	DWORD StretchCounter(DWORD);
	LARGE_INTEGER dxwCore::StretchCounter(LARGE_INTEGER);
	void ShowOverlay();
	void ShowOverlay(HDC);
	void ShowOverlay(LPDIRECTDRAWSURFACE);
	char *GetTSCaption(void);
	char *GetTSCaption(int);
	void DoSlow(int);
	void ShowBanner(HWND);
	POINT ScreenToClient(POINT);
	int GetDLLIndex(char *);
	void FixStyle(char *, HWND, WPARAM, LPARAM);
	void FixWindowFrame(HWND);
	HDC AcquireEmulatedDC(HWND);
	HDC AcquireEmulatedDC(HDC);
	HDC AcquireEmulatedDC(LPDIRECTDRAWSURFACE);
	BOOL ReleaseEmulatedDC(HWND);
	BOOL IsVirtual(HDC);
	void ResetEmulatedDC();
	int VirtualOffsetX, VirtualOffsetY;
	void DumpPalette(DWORD, LPPALETTEENTRY);
	void PushTimer(UINT, UINT, UINT, LPTIMECALLBACK, DWORD_PTR, UINT);
	void PushTimer(HWND, UINT_PTR, UINT, TIMERPROC);
	void PopTimer(UINT);
	void PopTimer(HWND, UINT_PTR);
	void RenewTimers();
	BOOL CheckScreenResolution(unsigned int, unsigned int);

public: // simple data variables
	DDPIXELFORMAT ActualPixelFormat;
	DDPIXELFORMAT VirtualPixelFormat;
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
	char *CustomOpenGLLib;
    WORD palVersion;
    WORD palNumEntries;
    PALETTEENTRY palPalEntry[256];
	short FakeVersionId;
	short Coordinates;
	short MaxScreenRes;
	HDC RealHDC;
	HDC VirtualHDC;

// Implementation
protected:
	DWORD dwScreenWidth;
	DWORD dwScreenHeight;
	BOOL FullScreen;
	HWND hWnd, hWndFPS;
	DWORD PrimSurfaces[DDSQLEN+1];
	DWORD BackSurfaces[DDSQLEN+1];
	HBITMAP VirtualPic;

private:
	void UnmarkPrimarySurface(LPDIRECTDRAWSURFACE);
	void UnmarkBackBufferSurface(LPDIRECTDRAWSURFACE);
	BOOL MustShowOverlay;
	void ShowFPS(HDC);
	void ShowTimeStretching(HDC);
	TimerEvent_Type TimerEvent;
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
	SYSLIBIDX_DIRECT3D11,
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
	SYSLIBIDX_DIRECT3D,
	SYSLIBIDX_DIRECT3D700,
//	SYSLIBIDX_GLIDE,
//	SYSLIBIDX_GLIDE2X,
//	SYSLIBIDX_GLIDE3X,
	SYSLIBIDX_MAX } 
enum_syslibraries;

