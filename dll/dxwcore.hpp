#include <windows.h>
#include "syslibs.h"

#define DDSQLEN 0x20

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

typedef struct {
	LPDIRECTDRAWSURFACE lpdds;
	USHORT	uRef;
	USHORT	uRole;
	USHORT	uVersion;
} SurfaceDB_Type;

typedef struct {
	LPDIRECTDRAWSURFACE lpdds;
	DWORD	dwCaps;
} CapsDB_Type;

typedef enum {
	SURFACE_ROLE_PRIMARY = 1,
	SURFACE_ROLE_BACKBUFFER
} Enum_Surface_Role_Type;

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
	BOOL IsToRemap(HDC);
	BOOL IsDesktop(HWND);
	BOOL IsRealDesktop(HWND);
	POINT FixCursorPos(POINT);
	void FixNCHITCursorPos(LPPOINT);
	void SetClipCursor(void);
	void EraseClipCursor(void);
	BOOL IsValidMainWindow();
	RECT MapWindowRect(LPRECT);
	RECT MapWindowRect(void);
	RECT MapClientRect(LPRECT);
	void MapClient(LPPOINT);
	void MapClient(LPRECT);
	void MapClient(int *, int *, int *, int *);
	void MapClient(int *, int *);
	//void MapClient(float *, float *);
	void UnmapClient(LPPOINT);
	void UnmapClient(int *, int *);
	void UnmapClient(int *, int *, int *, int *);
	void UnmapClient(LPRECT);
	void MapWindow(LPPOINT);
	void MapWindow(LPRECT);
	void MapWindow(int *, int *, int *, int *);
	void UnmapWindow(LPRECT);
	void UnmapWindow(LPPOINT);
	void FixWorkarea(LPRECT);
	void UpdateDesktopCoordinates();
	RECT GetScreenRect(void);
	RECT GetUnmappedScreenRect();
	RECT GetWindowRect(RECT);
	RECT GetClientRect(RECT);
	POINT GetFrameOffset();
	POINT AddCoordinates(POINT, POINT);
	RECT AddCoordinates(RECT, POINT);
	POINT SubCoordinates(POINT, POINT);
	POINT ClientOffset(HWND);
	void AutoScale();
	void ScreenRefresh(void);
	BOOL HandleFPS(void);
	BOOL ishWndFPS(HWND);
	DWORD GetTickCount(void);
	void GetSystemTime(LPSYSTEMTIME);
	void GetSystemTimeAsFileTime(LPFILETIME);
	DWORD StretchTime(DWORD);
	DWORD StretchCounter(DWORD);
	LARGE_INTEGER StretchCounter(LARGE_INTEGER);
	void ShowOverlay();
	void ShowOverlay(HDC);
	void ShowOverlay(HDC, int, int);
	void ShowOverlay(LPDIRECTDRAWSURFACE);
	char *GetTSCaption(void);
	char *GetTSCaption(int);
	void DoSlow(int);
	void ShowBanner(HWND);
	void HideDesktop(HWND);
	POINT ScreenToClient(POINT);
	int GetDLLIndex(char *);
	void PushDLL(char *, int);
	void FixStyle(char *, HWND, WPARAM, LPARAM);
	void FixWindowFrame(HWND);
	DWORD FixWinStyle(DWORD);
	DWORD FixWinExStyle(DWORD);
	HDC AcquireEmulatedDC(HWND);
	BOOL ReleaseEmulatedDC(HWND);
	BOOL IsVirtual(HDC);
	void ResetEmulatedDC();
	int VirtualOffsetX, VirtualOffsetY, VirtualExtentX, VirtualExtentY;
	void DumpPalette(DWORD, LPPALETTEENTRY);
	void PushTimer(UINT, UINT, UINT, LPTIMECALLBACK, DWORD_PTR, UINT);
	void PushTimer(HWND, UINT_PTR, UINT, TIMERPROC);
	void PopTimer(UINT);
	void PopTimer(HWND, UINT_PTR);
	void RenewTimers();
	BOOL CheckScreenResolution(unsigned int, unsigned int);
	LARGE_INTEGER StretchLargeCounter(LARGE_INTEGER);
	UINT MapKeysConfig(UINT, LPARAM, WPARAM);
	void MapKeysInit();
	void SetVSyncDelays(UINT);
	void VSyncWait();
	void VSyncWaitLine(DWORD);
	void DumpDesktopStatus();
	void ToggleFreezedTime();
	void GetMonitorWorkarea(LPRECT, BOOL);
	void CalculateWindowPos(HWND, DWORD, DWORD, LPWINDOWPOS);
	void MessagePump(void);

public: // simple data variables
	int MonitorId;
	BOOL Windowize;
	BOOL IsVisible;
	BOOL IsEmulated;
	DDPIXELFORMAT ActualPixelFormat;
	DDPIXELFORMAT VirtualPixelFormat;
	DWORD dwDDVersion;
	DWORD dwTargetDDVersion;
	DWORD dwMaxDDVersion;
	DWORD MaxDdrawInterface;
	DWORD dwFlags1;
	DWORD dwFlags2;
	DWORD dwFlags3;
	DWORD dwFlags4;
	DWORD dwFlags5;
	DWORD dwFlags6;
	DWORD dwFlags7;
	DWORD dwFlags8;
	DWORD dwTFlags;
	HWND hParentWnd;
	HWND hChildWnd;
	BOOL bActive;
	BOOL bDInputAbs;
	DWORD MaxFPS;
	DWORD SwapEffect;
	char *gsModules;
	int TimeShift;
	long iPosX;
	long iPosY;
	long iSizX;
	long iSizY;
	long iSiz0X;
	long iSiz0Y;
	long iMaxW;
	long iMaxH;
	int iRatioX;
	int iRatioY;
	char *CustomOpenGLLib;
    WORD palVersion;
    WORD palNumEntries;
    PALETTEENTRY palPalEntry[256];
	short FakeVersionId;
	short Coordinates;
	short MaxScreenRes;
	short SlowRatio;
	short ScanLine;
	HDC RealHDC;
	HDC VirtualHDC;
	int GDIEmulationMode;
	BOOL TimeFreeze;
	RECT VirtualDesktop;
	BOOL bHintActive;
	BOOL bAutoScale;
	DWORD dwRealScreenWidth;
	DWORD dwRealScreenHeight;

// Implementation
protected:
	LONG dwScreenWidth;
	LONG dwScreenHeight;
	BOOL FullScreen;
	HWND hWnd, hWndFPS;
	HBITMAP VirtualPic;
	RECT VirtualPicRect;

private:
	BOOL MustShowOverlay;
	void ShowFPS(HDC, int, int);
	void ShowTimeStretching(HDC, int, int);
	TimerEvent_Type TimerEvent;
	DWORD gdwRefreshRate;
	#define MAXREFRESHDELAYCOUNT 20
	int iRefreshDelays[MAXREFRESHDELAYCOUNT];
	int iRefreshDelayCount;
};

class dxwSStack
{
public:
    dxwSStack();
    virtual ~dxwSStack();

public:
	char *ExplainSurfaceRole(LPDIRECTDRAWSURFACE);
	void ClearSurfaceList();
	void PushPrimarySurface(LPDIRECTDRAWSURFACE, int);
	BOOL IsAPrimarySurface(LPDIRECTDRAWSURFACE);
	void UnrefSurface(LPDIRECTDRAWSURFACE);
	LPDIRECTDRAWSURFACE GetPrimarySurface(void);
	void PushBackBufferSurface(LPDIRECTDRAWSURFACE, int);
	BOOL IsABackBufferSurface(LPDIRECTDRAWSURFACE);
	LPDIRECTDRAWSURFACE GetBackBufferSurface(void);
	void PopSurface(LPDIRECTDRAWSURFACE);

protected:
	SurfaceDB_Type SurfaceDB[DDSQLEN+1];
	LPDIRECTDRAWSURFACE lpDDSPrimary;
	LPDIRECTDRAWSURFACE lpDDSBackBuffer;

private:
	void PushSurface(LPDIRECTDRAWSURFACE, USHORT, USHORT);
	LPDIRECTDRAWSURFACE GetSurfaceByRole(USHORT);
	void SetSurfaceEntry(LPDIRECTDRAWSURFACE, USHORT, USHORT);
};

class dxwCapsDB
{
public:
    dxwCapsDB();
    virtual ~dxwCapsDB();

public:
	// void ClearCapsDB();
	void PushCaps(LPDIRECTDRAWSURFACE, DWORD);
	DWORD GetCaps(LPDIRECTDRAWSURFACE);

protected:
	CapsDB_Type CapsDB[DDSQLEN+1];
};

typedef struct {
	HWND hwnd;
	WNDPROC wndproc;
	int w;
	int h;
} wndstack_entry;

class dxwWStack
{
public:
    dxwWStack();
    virtual ~dxwWStack();

public:
	void Put(HWND, WNDPROC, int, int);
	void PutProc(HWND, WNDPROC);
	void PutSize(HWND, int, int);
	BOOL GetSize(HWND, int *, int *);
	WNDPROC GetProc(HWND);

private:
	wndstack_entry *WhndStack;
	int WhndTOS;
	int WhndSize;
};

extern dxwCore dxw;
extern dxwSStack dxwss;
extern dxwWStack dxwws;
extern dxwCapsDB dxwcdb;

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
	SYSLIBIDX_DSOUND,
	SYSLIBIDX_WINMM,
	SYSLIBIDX_IMM32,
	SYSLIBIDX_DINPUT,
	SYSLIBIDX_DINPUT8,
	SYSLIBIDX_WINTRUST,
	SYSLIBIDX_DIRECT3D,
	SYSLIBIDX_DIRECT3D700,
	SYSLIBIDX_IMAGEHLP,
	SYSLIBIDX_COMDLG32,
	SYSLIBIDX_COMCTL32,
	SYSLIBIDX_AVIFIL32,
	SYSLIBIDX_GLIDE,
	SYSLIBIDX_GLIDE2,
	SYSLIBIDX_GLIDE3,
	SYSLIBIDX_SMACKW32,
	SYSLIBIDX_WING32,
	FREE6,
	SYSLIBIDX_MAX 
} 
enum_syslibraries;

#ifdef SYSLIBNAMES_DEFINES
char *SysNames[]={
	"kernel32",
	"USER32",
	"GDI32",
	"imelib",
	"ADVAPI32",
	"ole32",
	"ddraw",
	"d3d8",
	"d3d9",
	"d3d10",
	"d3d10_1",
	"d3d11",
	"opengl32",
	"msvfw32",
	"dsound",
	"winmm",
	"imm32",
	"dinput",
	"dinput8",
	"wintrust",
	"d3dim",
	"d3dim700",
	"imagehlp",
	"comdlg32",
	"comctl32",
	"AVIFIL32",
	"_", "_", "_", "_", "_", "_", // 6 free slots 
	NULL
};	
#else
extern char *SysNames[];
#endif 

enum {
	HINT_HINT = 0,
	HINT_DDRAW,		HINT_D3D8,		HINT_D3D9,		HINT_D3D10,
	HINT_D3D11,		HINT_OPENGL,	HINT_DSOUND,	HINT_DINPUT,
	HINT_DINPUT8,	HINT_MOVIES,	HINT_D3D,		HINT_IHLP,
	HINT_FAKEOS,	HINT_OBFUSCATED,HINT_SAFEDISC,	HINT_SECUROM,
	HINT_LIMITMEM,	HINT_HOOKUPDATE,
	HINT_LAST
};

enum {
	DXVK_NONE=0,
	DXVK_CLIPTOGGLE,
	DXVK_REFRESH,
	DXVK_LOGTOGGLE,
	DXVK_PLOCKTOGGLE,
	DXVK_FPSTOGGLE,
	DXVK_TIMEFAST,
	DXVK_TIMESLOW,
	DXVK_TIMETOGGLE,
	DXVK_ALTF4,
	DXVK_PRINTSCREEN,
	DXVK_CORNERIZE,
	DXVK_FREEZETIME,
	DXVK_FULLSCREEN,
	DXVK_FAKEWORKAREA,
	DXVK_FAKEDESKTOP,
	DXVK_SIZE
};

enum {
	GDIMODE_NONE = 0,
	GDIMODE_STRETCHED,
	GDIMODE_EMULATED,
	GDIMODE_SHAREDDC
};

typedef HRESULT (WINAPI *ColorConversion_Type)(int, LPDIRECTDRAWSURFACE, RECT, LPDIRECTDRAWSURFACE *);
extern ColorConversion_Type pColorConversion;
extern void ToggleAcquiredDevices(BOOL);
extern GetWindowLong_Type pGetWindowLong;
extern SetWindowLong_Type pSetWindowLong;